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

## CI scratch repository (`scratch-repo.sh`)

The matrix CI (`.forgejo/workflows/gucef-ci.yml`) also runs on unpublished work: a snapshot is pushed as a branch
`scratch/<machine>-<utc>-<sha>` to a normal writable repository and started with `workflow_dispatch` (inputs `force`,
`toolchains`, `cppStandards`, `products`, `mode` = `build` or `test`). The pull mirror of the published repository refuses
pushes, so the scratch repository sits next to it. `./scratch-repo.sh` (idempotent) sets it up:

- Bot user `ci-bot` (`FORGEJO_CI_BOT_USER`) with a random password, stored as `FORGEJO_CI_BOT_PASSWORD` in `secrets.env` and
  `~/.config/gucef-k3s/forgejo-secrets.env`. It is never printed.
- Private repository `ci/GUCEF` (`FORGEJO_CI_OWNER`/`FORGEJO_CI_REPO`; the owner is created as a private organization when
  missing), Actions enabled, `ci-bot` collaborator with write access.
- `--seed <git url or local repository>` pushes all branches and tags into it, once, while it is empty. Actions are off
  during that push, so it starts no CI run.
- `--report-token` gives `ci-bot` a new token with scope `write:package` (earlier ones of this script are deleted) and stores
  it as the Actions secret `CI_REPORT_TOKEN` of the scratch repository. Without that secret the cells still build and write
  their reports, they only skip the upload. Run it once after the first setup, and again to rotate the token.
- `--gateway-token FILE` gives `ci-bot` an access token for the CI gateway (`gucefMcpCi --mode gateway`, see
  [../ci/mcp/README.md](../ci/mcp/README.md)) and writes it to FILE with mode 0600; the directory of FILE must exist.
  Scopes: `write:repository` (push and delete scratch branches, dispatch and cancel runs), `read:package` and
  `write:package` (read and delete the report packages of `ci-bot`). When FILE exists and is not empty nothing is done.
  With `--rotate` a new token is made and written even then, after which the earlier tokens of this option (named
  `ci-gateway-*`) are deleted; without it they stay valid. `ci/mcp/install.sh --role gateway` runs this option when the
  gateway has no token yet.
- `--prune [DAYS]` deletes branches `scratch/*` whose head commit is older than DAYS days (default `FORGEJO_CI_PRUNE_DAYS`, 14).
- `--dry-run` prints the planned API calls and git commands without touching the cluster.

Example: `./scratch-repo.sh --seed https://github.com/<owner>/GUCEF.git --report-token`, later `./scratch-repo.sh --prune`.
Tokens are never printed; the gateway token exists only in FILE.

Reports: each matrix cell (`projects/CI/scripts/ci_cell.sh`) writes `ci-report/<cell>/report.json` (per product: status,
phase, exit code, seconds, log tail; in test mode per test application the failed test cases) and `<product>.log.gz`, and
uploads them with `CI_REPORT_TOKEN` to the generic package registry of `ci-bot`: package `ci-reports`, version
`<run id>-<cell id>`, for example `http://<host>:32001/api/packages/ci-bot/generic/ci-reports/<run id>-<cell id>/report.json`.
The upload is one function in `ci_cell.sh` (`upload_report`) and is still to be confirmed on the live Forgejo; upload
failures are warnings. Test applications listed in `projects/CI/matrix/test-skip.txt` are not run.
