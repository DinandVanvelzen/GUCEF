# Forgejo

Forgejo on k3s using the official chart `oci://code.forgejo.org/forgejo-helm/forgejo` (version in `config.env`, default 17.3.2).

- Namespace `forgejo`, release `forgejo`.
- Data: PVC `forgejo-data-pvc` on storage class `longhorn-raid`.
- Database: `forgejo` database and role in `my-postgres` (namespace `postgresql`).
- HTTP LoadBalancer port 32001 (`FORGEJO_HTTP_PORT`), SSH LoadBalancer port 32002 (`FORGEJO_SSH_PORT`). Git over HTTP uses the web port.
- Registration disabled, repositories private by default, push-to-create enabled for users and orgs, Actions enabled.
- Organizations from `FORGEJO_ORGS` (default none; set in the gitignored `config.env`) are created private.

## Setup

Part of the main k3s setup (`../setup.sh`). To run it on its own, the components it depends on
must be installed first: Sealed Secrets (`../sealed-secrets/setup.sh`) and PostgreSQL
(`../postgresql/setup.sh`). `./setup.sh` checks for them and names the script to run when one is missing.

1. `./setup.sh` (idempotent). With no configuration it auto-detects the node address and the storage
   class, generates the admin and database passwords, and prints the next steps when it finishes.
2. Passwords are written to `secrets.env` and copied to `~/.config/gucef-k3s/forgejo-secrets.env`.
3. Optional: `cp config.env.example config.env` and uncomment settings (host name, ports, organizations).
   `config.env`, `secrets.env` and `sealed-secret.yaml` are gitignored.

## Secret keys (`forgejo-credentials`)

`DB_PASSWORD`, `username`, `password` (chart admin secret format), `GITHUB_TOKEN`.

## Uninstall

`./uninstall.sh` removes the helm release and keeps the PVC and database.
`./uninstall.sh --wipe-data` also deletes the PVC and the database.

## Workstation helpers

- `workstation/push-local-repos.sh <org> <dir>...` adds the remote `forgejo` (SSH `ssh://git@$FORGEJO_HOST:32002/<org>/<repo>.git`, or HTTP `http://$FORGEJO_HOST:32001/<org>/<repo>.git` when `FORGEJO_TOKEN` is set), pushes all branches and tags, verifies branch heads with `git ls-remote`, prints OK/FAIL per repo.
- `workstation/make-bundles.sh <outdir> <dir>...` writes and verifies `<repo>.bundle` per repo.

The SSH public key of the pushing user must be added to the Forgejo user (Settings, SSH/GPG Keys) before pushing over SSH.

## Actions runner (`runner/`)

`runner/setup.sh` installs a Forgejo Actions runner (`forgejo-runner` plus a privileged Docker-in-Docker sidecar) in the `forgejo` namespace. The top-level `setup.sh` runs it when `CI_ENGINE` is `forgejo` or `all` (see `../config.env.example`).

- Labels default to `gucef-linux` and `docker`, both running the image `FORGEJO_RUNNER_JOB_IMAGE` (default `node:20-bookworm`). Override all labels with `FORGEJO_RUNNER_LABELS` in `config.env`. Workflows select a runner with `runs-on: <label>`.
- Registration is automatic: the token comes from `forgejo actions generate-runner-token` and is stored in the secret `forgejo-runner-registration`. Changing the labels re-registers the runner.
- The Docker daemon MTU is derived from the pod network MTU minus 50. With the default Docker MTU of 1500 on a 1450 network, image pulls stall.
- A NetworkPolicy denies all ingress to the runner pod: the daemon listens on tcp 2375 without authentication.
- Job images are cached on the PVC `forgejo-runner-data-pvc`. `runner/uninstall.sh --wipe-data` deletes it.
- Other settings in `config.env`: `FORGEJO_RUNNER_CAPACITY` (default 2), `FORGEJO_RUNNER_STORAGE_SIZE`, `FORGEJO_RUNNER_VERSION`, `FORGEJO_RUNNER_MTU`.
