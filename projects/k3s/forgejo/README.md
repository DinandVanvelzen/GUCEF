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
- `runner/setup.sh --dry-run` prints the manifests on standard output (the steps on standard error) and changes nothing; check them with `runner/setup.sh --dry-run | kubectl apply --dry-run=client -f -`.

## Image build runner (`runner/setup.sh --oci`)

A second runner, `forgejo-runner-oci`, builds container images and pushes them to the container registry of Forgejo. It is
isolated from `forgejo-runner`: its own Deployment, ConfigMap `forgejo-runner-oci-config`, PVC `forgejo-runner-oci-data-pvc`,
registration secret `forgejo-runner-oci-registration` and NetworkPolicy `forgejo-runner-oci-deny-ingress` (all ingress
denied). `runner/setup.sh` without `--oci` and the resources of `forgejo-runner` are unchanged by it; the matrix cells keep
running on `gucef-linux` and never reach the builder.

- Label `gucef-oci:host` (`FORGEJO_RUNNER_OCI_LABELS`, only `<label>:host` labels): the jobs run in host mode, as processes
  in the runner container. The runner has no container daemon. Changing the labels registers the runner again, as for
  `forgejo-runner`. Capacity `FORGEJO_RUNNER_OCI_CAPACITY` (default 1).
- A BuildKit daemon container (`FORGEJO_BUILDKIT_IMAGE`, default `moby/buildkit:<FORGEJO_BUILDKIT_VERSION>-rootless`, version
  pinned in `config.env.example`) listens on `127.0.0.1:1234` and on the socket `/buildkit-socket/buildkitd.sock` in an
  emptyDir that the runner container mounts as well. Its `buildkitd.toml` (from the ConfigMap) lists the registry
  `FORGEJO_REGISTRY_HOST` (default: the Forgejo host and `FORGEJO_HTTP_PORT`) as plain HTTP. Its cache is on the PVC.
- Name resolution: the registry answers an unauthenticated request with `Www-Authenticate: Bearer
  realm="http://worker.lan:32001/v2/token"` (the host of the Forgejo ROOT_URL), so BuildKit fetches its token from
  `worker.lan`. The pod gets `hostAliases` that resolve the names of `FORGEJO_REGISTRY_ALIASES` (default `worker.lan`) to
  the address of `FORGEJO_REGISTRY_HOST` when that is an IPv4 address; with a host name as `FORGEJO_REGISTRY_HOST` no
  aliases are written and the cluster DNS resolves the names. The nodes resolve `worker.lan` themselves and need nothing.
- The jobs find `buildctl` on their `PATH` (`/opt/oci-tools/bin`, also in `BUILDCTL`), and `BUILDKIT_HOST` and `REGISTRY_HOST`
  in the environment of the runner. An init container copies the static `buildctl` binary from the BuildKit image into an
  emptyDir, since the runner image has none. A job that does not inherit the runner environment sets
  `BUILDKIT_HOST=unix:///buildkit-socket/buildkitd.sock` itself.
- Job tools: the runner image is assumed to be Alpine based (it is built on the Forgejo Alpine image and has the busybox tools
  and `apk`). The entry point installs `FORGEJO_RUNNER_OCI_PACKAGES` (default `bash git curl python3 nodejs coreutils`) with
  `apk add` at every pod start, so the container runs as root (unprivileged, no privilege escalation) and needs the Alpine
  package mirrors at start. Actions written in JavaScript (`actions/checkout`) need `nodejs`. A job that needs more tools
  installs them with `apk add` itself.
- Registration: the secret `forgejo-runner-oci-registration` takes the token of `forgejo-runner-registration` when that
  exists (generating a new instance token can invalidate the earlier one, which `forgejo-runner` needs to register again);
  else a token is generated. The token passes through pipes only.
- `runner/setup.sh --oci --dry-run` prints the manifests; `runner/uninstall.sh --oci [--wipe-data]` removes the runner.

Security notes:

- Rootless BuildKit (`FORGEJO_BUILDKIT_MODE=rootless`, the default) runs as uid 1000 in user namespaces. It needs a node that
  allows unprivileged user namespaces: on Ubuntu 23.10 and later `kernel.apparmor_restrict_unprivileged_userns` is 1 by
  default and rootless BuildKit fails there. The daemon container runs with seccomp and AppArmor `Unconfined` (both filters
  block the mounts it makes inside its namespaces) and with `--oci-worker-no-process-sandbox` (build steps share the PID
  namespace of the daemon), as the upstream rootless example does. Bind the runner to a node that allows them with
  `FORGEJO_RUNNER_OCI_NODE` (in this cluster: worker-x299 with Ubuntu 22.04 allows them, workstationznix with
  Ubuntu 26.04 restricts them; both read from `/proc/sys/kernel`, not tested with a running daemon).
- `FORGEJO_BUILDKIT_MODE=privileged` is the fallback: a privileged daemon container (`moby/buildkit:<version>`), as the
  Docker-in-Docker sidecar of `forgejo-runner` is. A job can then reach a privileged daemon through the socket.
- Every job of this runner can use the builder and the registry credentials it is given: only image jobs are routed here
  (`runs-on: gucef-oci`); the matrix cells never run on it. The pod gets no Kubernetes API token.

## Container registry on the nodes (`registry/`)

The container registry of Forgejo (`http://<host>:32001/v2/`, plain HTTP) holds the images the CI builds, below the owner
`ci`: `<host>:32001/ci/<image>:<tag>`, with the tags `sha-<12 character commit>`, `scratch-<run id>` and `candidate`.
containerd on the nodes pulls them with a pull token (scope `read:package`), without a pull Secret per namespace.

- `sudo registry/setup-node.sh --token-file FILE [--registry HOST:PORT] [--user NAME] [--dry-run]`, once per existing node:
  writes `/etc/rancher/k3s/registries.yaml` (mirror endpoint `http://HOST:PORT`, `configs` auth with the user and the token
  of FILE; root, mode 0600) and restarts `k3s` on a server node or `k3s-agent` on an agent node, only when the file changed
  (running pods keep running; on the server node the API is away for the restart). An existing file with other entries
  is merged and its other entries stay (python3 with the yaml module; comments of the file are not kept); without that
  module an existing file with other content is refused and left alone. The token is never printed. Defaults:
  `FORGEJO_REGISTRY_HOST`, else `FORGEJO_HOST:FORGEJO_HTTP_PORT` of `config.env`; user `ci-bot`. `--file PATH` writes
  another file (tests); `--no-restart` only writes.
- New agent nodes: `agent-node/join.sh --registry-settings FILE` (or the environment `REGISTRY_HOST`, `REGISTRY_TOKEN`) writes
  the same file next to its `config.yaml.d/50-gucef-agent.yaml` and restarts the agent when it changed.
- The pull token: `./scratch-repo.sh --registry-token FILE` (see below). After `--rotate` every node needs
  `registry/setup-node.sh` again.
- `registry/push-base-image.sh --token-file FILE [--minideb TAG] [--dry-run]` pushes the base images of the image builds
  once from a machine that has them in docker: `vanvelzen/github-gucef-dev-base:latest` as `ci/gucef-dev-base:latest`, and
  with `--minideb TAG` `bitnami/minideb:TAG` as `ci/minideb:TAG`. The token needs `write:package` for the packages of `ci`.
  The docker daemon must list the registry in `insecure-registries`; the script checks it and says what to change.
- Images are built and pushed with `projects/k3s/gucef_service/scripts/build_image.sh --builder buildkit --registry
  <host>:32001/ci --push` (its README, "Builder and registry"); a service file with `IMAGE_REGISTRY` makes the manifests pull
  them.

## CI scratch repository (`scratch-repo.sh`)

The matrix CI (`.forgejo/workflows/gucef-ci.yml`) also runs on unpublished work: a snapshot is pushed as a branch
`scratch/<machine>-<utc>-<sha>` to a normal writable repository and started with `workflow_dispatch` (inputs `force`,
`toolchains`, `cppStandards`, `products`, `mode` = `build` or `test`). The pull mirror of the published repository refuses
pushes, so the scratch repository sits next to it. `./scratch-repo.sh` (idempotent) sets it up:

- Bot user `ci-bot` (`FORGEJO_CI_BOT_USER`) with a random password, stored as `FORGEJO_CI_BOT_PASSWORD` in `secrets.env` and
  `~/.config/gucef-k3s/forgejo-secrets.env`. It is never printed.
- Private repository `ci/GUCEF` (`FORGEJO_CI_OWNER`/`FORGEJO_CI_REPO`; the owner is created as a private organization when
  missing), Actions enabled, `ci-bot` collaborator with write access.
- Team `publishers` of the owner `ci` (permission `write`, all repositories, units `repo.code`, `repo.releases`,
  `repo.packages`) with `ci-bot` as member, created when missing; an existing team is kept as it is (a warning when it
  cannot write packages). Forgejo derives the package rights of an organization from team membership, as Gitea does
  (`GetOrgUserMaxAuthorizeLevel` and the package unit of the teams); a repository collaborator gets no access to the
  packages of the organization, so without the team `ci-bot` can neither push nor pull the container images of `ci`.
  This rule is taken from the Gitea semantics and verified only on the live system. When the team or the membership
  cannot be established the script warns and goes on; `--report-token`, `--registry-token` and `--gateway-token` repeat
  the warning.
- `--seed <git url or local repository>` pushes all branches and tags into it, once, while it is empty. Actions are off
  during that push, so it starts no CI run.
- `--report-token` gives `ci-bot` a new token with scope `write:package` (earlier ones of this script are deleted) and stores
  it as the Actions secrets `CI_REPORT_TOKEN` and `CI_REGISTRY_TOKEN` of the scratch repository (one token, two names: the
  report upload and the image push). Without that secret the cells still build and write
  their reports, they only skip the upload. Run it once after the first setup, and again to rotate the token.
- `--gateway-token FILE` gives `ci-bot` an access token for the CI gateway (`gucefMcpCi --mode gateway`, see
  [../ci/mcp/README.md](../ci/mcp/README.md)) and writes it to FILE with mode 0600; the directory of FILE must exist.
  Scopes: `write:repository` (push and delete scratch branches, dispatch and cancel runs), `read:package` and
  `write:package` (read and delete the report packages of `ci-bot`). When FILE exists and is not empty nothing is done.
  With `--rotate` a new token is made and written even then, after which the earlier tokens of this option (named
  `ci-gateway-*`) are deleted; without it they stay valid. `ci/mcp/install.sh --role gateway` runs this option when the
  gateway has no token yet.
- `--registry-token FILE` gives `ci-bot` an access token with the single scope `read:package` and writes it to FILE with mode
  0600 (the directory must exist): the pull token of the nodes (`registry/setup-node.sh --token-file FILE`). Nothing is done
  when FILE exists and is not empty; `--rotate` makes a new one and then deletes the earlier tokens of this option (named
  `ci-registry-*`). A rotated token must be written to every node again.
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
