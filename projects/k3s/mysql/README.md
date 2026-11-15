# MySQL

MySQL 8.4 LTS (`mysql:8.4`) shared by the components that need MySQL, such as the IRC server (`../irc`).

- Namespace `mysql`, StatefulSet `mysql`, Service `mysql` (ClusterIP).
  In-cluster address: `mysql.mysql.svc.cluster.local:3306`. Not reachable from the LAN.
- Data: PVC `mysql-pvc`, storage class `longhorn-raid` when the cluster has it, otherwise the cluster default.
- Root password: Secret `mysql-credentials`, key `MYSQL_ROOT_PASSWORD`, from a sealed secret.

## Setup

Part of the main k3s setup (`../setup.sh`). Needs Sealed Secrets (`../sealed-secrets/setup.sh`).

1. `./setup.sh` (idempotent). Without `secrets.env` a random root password is generated, written to `secrets.env`
   and copied to `~/.config/gucef-k3s/mysql-secrets.env`.
2. Optional: `cp config.env.example config.env` to change the volume size or storage class.
   `config.env`, `secrets.env` and `sealed-secret.yaml` are gitignored.

MySQL keeps the root password it was first initialized with; changing `secrets.env` later does not change it.

## Databases for other components

```bash
DB_PASSWORD=<password> ./ensure-database.sh <database> <user>
```

Creates the database (utf8mb4) and the user when missing, sets the password and grants the user all rights on
that database. Safe to run on every setup.

## Uninstall

`./uninstall.sh` removes the StatefulSet and Service and keeps the PVC.
`./uninstall.sh --wipe-data` also deletes the PVC.
