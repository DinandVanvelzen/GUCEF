# NAS backups

Backs up the cluster to an SMB share on a NAS:

| What | How | Where on the share |
|---|---|---|
| Every Longhorn volume | Longhorn backup target + recurring jobs (daily, weekly) | `longhorn/` |
| Sealed Secrets key | nightly host timer `k3s-backup-keys` | `cluster-keys/` |
| PostgreSQL (all databases) | CronJob `pg-dumpall` | `postgresql/` |
| Forgejo repositories | CronJob `forgejo-repo-mirror` (plain bare repos) | `forgejo/repos/<owner>/<repo>.git` |

The live data stays on local disks; the share only holds backups.

## Setup

1. On the NAS create a share (default name `k3s-backup`) and an account with read-write access to it.
2. `cp config.env.example config.env` and set `BACKUP_NAS_HOST` (name or IP of the NAS).
3. `sudo ./setup-nas.sh` asks for the account once, mounts the share at `/media/backup` (fstab, root-only) and installs the key-export timer.
4. `./setup.sh` seals the account, sets the Longhorn backup target, creates the recurring jobs and the CronJobs.

The account is kept in `secrets.env` (gitignored) and in `~/.config/gucef-k3s/backup-secrets.env`. Keep a copy outside the cluster.
Every node needs `cifs-utils` (`apt install cifs-utils`) for Longhorn to mount the share.

## Recovery after a node wipe

1. Install k3s, then `sudo ./setup-nas.sh` (mounts the share).
2. Restore the Sealed Secrets key before the controller is installed:
   `kubectl apply -f /media/backup/cluster-keys/sealed-secrets-key-latest.yaml`, then run `../sealed-secrets/setup.sh`.
3. Install Longhorn, then `./setup.sh` (backup target).
4. `./restore-volumes.sh --list`, then `./restore-volumes.sh --all` (or `--only <pvc>,<pvc>`).
5. Run the component setup scripts again; they reuse the restored PVCs.
6. PostgreSQL dumps (`postgresql/*.sql.gz`) are a second copy for database-level recovery:
   `gunzip -c <file> | psql -h my-postgres -U postgres`.

Longhorn backups are crash-consistent; the PostgreSQL dumps are the consistent copy of the databases.
