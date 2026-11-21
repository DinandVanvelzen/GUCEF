# CI MCP server `gucefMcpCi`

`gucefMcpCi` ([../../../../tools/gucefMcpCi/README.md](../../../../tools/gucefMcpCi/README.md)) lets a Claude session hand
the build matrix of a repository to the lab CI: the server sends the unpublished work of a checkout as a scratch
snapshot, starts a run of the full matrix (or a filtered part of it) and answers status, report and failure details in
a short, bounded form. `install.sh` installs it in one of two roles.

| Role | Where | What it installs |
|---|---|---|
| `client` | any machine of the lab (default when the machine is no k3s server) | the program in `/opt/gucef-mcp-ci/<version>` (`current` points to the one in use) and its registration with Claude Code of the user as the stdio server `ci` |
| `gateway` | the k3s server node that runs the lablink hub (default when `/etc/systemd/system/k3s.service` exists) | everything of the client role, and the systemd service `gucef-ci-gateway`, which serves the gateway API on `https://<host>:8792` and holds the Forgejo credentials |

On a client, Claude Code starts the program itself (`gucefMcpCi --stdio --mode auto`); no service runs. In `auto` mode
the program works as a proxy of the gateway with the gateway URL, the gateway token of the machine and the authority of
the lab that lablink writes to `/var/lib/gucef-mcp/ci` once the hub approved the machine. A client holds no Forgejo
credentials and needs a route to the gateway only.

## Setup

1. **The lablink hub** on the k3s server node: `sudo projects/k3s/lablink/install.sh` (also part of `projects/k3s/setup.sh`).
   It sets up the MCP certificates of the nodes (`llm/mcp/cluster.sh`), whose node certificate the gateway uses.
2. **Forgejo, its runner and the scratch repository**: `projects/k3s/setup.sh` with `GIT_SERVER=forgejo` and `CI_ENGINE=forgejo`
   or `all`, then once `projects/k3s/forgejo/scratch-repo.sh --seed <published repository> --report-token`
   (see [../../forgejo/README.md](../../forgejo/README.md)).
3. **The gateway**: `sudo projects/k3s/ci/mcp/install.sh --role gateway` on the k3s server node. `projects/k3s/setup.sh` runs
   it by itself when Forgejo CI is selected (`CI_GATEWAY=auto`, the default). Then edit `/etc/gucef-ci/ci.json` (made from
   `projects/k3s/lablink/ci.json.example`) when the lab has other CI ids, URLs or remotes, and run the script again.
4. **The clients**: `sudo projects/k3s/ci/mcp/install.sh` on each machine, from the user whose Claude Code gets the tools.
   The machine must run lablink and be approved on the hub: `sudo gucefLabLink --approve <name>` there. A machine that
   linked with the lab secret alone, or whose lablink runs with `LAB_CI=false` (`--no-ci`), receives no CI configuration.
   The configuration arrives within the refresh time of lablink (10 minutes by default).

Usage: `sudo ./install.sh [--role gateway|client] [--prebuilt DIR] [--skip-tests] [--upgrade] [--dry-run]`

| Option | Meaning |
|---|---|
| `--role ROLE` | `gateway` or `client`; default `CI_ROLE` of `config.env`, else detected |
| `--prebuilt DIR` | installs the built files of DIR (`bin/gucefMcpCi`, `lib/`) instead of building |
| `--skip-tests` | installs without running `gucefMcpCi_TestApp` |
| `--upgrade` | only when it is installed already: installs the new files and restarts the gateway, everything else stays (for a package) |
| `--dry-run` | prints what would be done and changes nothing |

`install.sh` builds the checkout it is in as the user that ran `sudo` (Release, relocatable, in `/var/cache/gucef-mcp-ci/build`):
the program with the plugins it loads (`dstorepluginJSONPARSER`, `codecspluginZLIB`) from `GUCEF_exe_gucefMcpCi`, and the
tests from `GUCEF_exe_gucefMcpCi_TestApp`, which run against the program that is installed. A failing build or test keeps
the installed version. It is idempotent: it installs, registers and restarts only when the result or the settings
differ. Without a checkout around the script it installs the files of `/usr/lib/gucef-mcp-ci` and reads its settings from
`/etc/gucef-ci/settings.env`.

`uninstall.sh [--purge]` removes the service and its units, the installed versions, the build and the registration with
Claude Code. Without `--purge` the registry `/etc/gucef-ci/ci.json`, the state `/var/lib/gucef-ci`, the system user and the
CI settings of the lablink hub stay; `--purge` removes them as well.

## Settings

`config.env` next to the script ([config.env.example](config.env.example)), copied from the example on the first run:

| Setting | Default | Meaning |
|---|---|---|
| `CI_ROLE` | detected | `gateway` or `client` |
| `CI_MCP_NAME` | `ci` | the name of the server in Claude Code |
| `CI_REGISTER_CLAUDE` | `true` | register the server with Claude Code of the user |
| `CI_GATEWAY_URL` | empty | client without lablink: the gateway; gateway: the URL the hub announces, default `https://<host name>:8792` |
| `CI_GATEWAY_TOKEN_FILE` | empty | client without lablink: the gateway token of the machine, owner-only |
| `CI_CA_FILE` | empty | client without lablink: the authority of the gateway certificate |
| `CI_GATEWAY_PORT` | `8792` | gateway: the HTTPS port |
| `CI_GATEWAY_USER` | `gucef-ci` | gateway: the system user of the service |
| `CI_ALLOW` | the networks of the machine | gateway: the networks the port is opened to in the firewall |
| `CI_DEPLOY_MACHINES` | unset | gateway: the approved machines (names or machine ids, comma separated) whose gateway token may also deploy; set as `LAB_CI_DEPLOY_MACHINES` of the hub. Unset: the setting of the hub stays; set but empty: no machine may deploy |

## What the gateway role sets up

- The system user `gucef-ci` (no login, home `/var/lib/gucef-ci`), made when missing.
- The CI registry `/etc/gucef-ci/ci.json`, made from `projects/k3s/lablink/ci.json.example` only when it does not exist. The
  gateway (`--ci-config`) and the lablink hub (`LAB_CI_REGISTRY`) use the same file.
- The Forgejo token of the bot user `ci-bot`, `/var/lib/gucef-ci/forgejo.token`, made with
  `projects/k3s/forgejo/scratch-repo.sh --gateway-token` (run as the user that ran `sudo`) when it is missing. When it cannot
  be made, the service is installed and stays disabled, and the script says what to do.
- The certificate of the node: `llm/mcp/cluster.sh` keeps it in `/var/lib/gucef-mcp/tls`, which `gucef-ci` cannot read; it is
  copied to `/var/lib/gucef-ci/tls`, and again by `gucef-ci-gateway-tls.path` when it is renewed, which then restarts the
  gateway. Until the certificate is there the gateway waits; it starts by itself once it arrives.
- The settings of the lablink hub (in `projects/k3s/lablink/config.env`, or `/etc/gucef-lablink/settings.env` without a
  checkout), marked `# set by ci/mcp/install.sh`: `LAB_CI_REGISTRY`, `LAB_CI_GATEWAY_URL`, `LAB_CI_GATEWAY_TOKENS`
  (`/var/lib/gucef-ci/tokens/gateway-tokens`) and `LAB_CI_GATEWAY_USER` (`gucef-ci`). The running hub gets the same options in
  `/etc/gucef-lablink/args.env` and is restarted, only when they changed. A gateway URL set for the hub before stays,
  unless `CI_GATEWAY_URL` names one.
- 8792/tcp open to the lab networks (`host-node/open_firewall_ports.sh ci-gateway`), when ufw or firewalld is active.
- Deployments, when the kubeconfig `/var/lib/gucef-ci/kubeconfig` exists (see [Deployments](#deployments)) and `kubectl` is
  installed (else `k3s kubectl`, through the wrapper `/opt/gucef-mcp-ci/kubectl`): the gateway gets `--kubeconfig` and
  `--kubectl`, the kubeconfig is given to `gucef-ci` with mode 0600, and its digest is kept in `gateway.env`, so that a
  changed kubeconfig restarts the gateway on the next run. `--allow-deploy` is not given: which machine may deploy is the
  scope of its gateway token. When the kubeconfig or kubectl is missing the install goes on and says what to do; the
  gateway then builds and does not deploy.
- The service `gucef-ci-gateway`: `gucefMcpCi --mode gateway --listen-tls 0.0.0.0:8792` with the node certificate, the
  registry, the Forgejo token and the token hashes of the hub, and `--allowed-host` for the host name, its lower case form,
  the fully qualified name and the addresses of the machine. It runs as `gucef-ci` without capabilities
  (`CapabilityBoundingSet=` empty), with `NoNewPrivileges`, `ProtectSystem=strict` (writable: `/var/lib/gucef-ci`, also its
  home `HOME=/var/lib/gucef-ci`, where kubectl keeps its cache in `.kube`), `ProtectHome`, `PrivateTmp`, and restarts on
  failure.
- `git` must be installed: the gateway takes the snapshots in with it. The script warns when it is missing.

## Deployments

The gateway deploys only Deployments that the `deploy` blocks of `/etc/gucef-ci/ci.json` name, with a kubeconfig of
restricted ServiceAccounts and never with the admin kubeconfig. The owner of the cluster sets them up once, and again
whenever the deploy targets change, with [../deployer/setup.sh](../deployer/setup.sh) (admin `kubectl`, no `sudo`):

```bash
projects/k3s/ci/deployer/setup.sh --dry-run          # the manifests and the actions, nothing is changed
projects/k3s/ci/deployer/setup.sh                    # applies them and writes the kubeconfig
sudo install -m 0600 -o gucef-ci -g gucef-ci ...     # the one command it prints, when it cannot write the file itself
projects/k3s/ci/deployer/setup.sh --verify           # kubectl auth can-i with the kubeconfig of the gateway
sudo projects/k3s/ci/mcp/install.sh --role gateway   # passes the kubeconfig to the gateway
```

For every namespace of a deploy target it applies the ServiceAccount, Role and RoleBinding `gucef-deployer` and the Secret
`gucef-deployer-token` (a token that does not expire), labelled `app.kubernetes.io/managed-by: gucef-ci-deployer`. The Role
allows `get` and `patch` on the named Deployments only, `list` and `watch` on the Deployments of the namespace (Kubernetes
cannot limit these two by name), and `get`, `list`, `watch` on ReplicaSets and pods, which `kubectl rollout status` and
`rollout undo` need. It cannot delete or create anything, nor read Secrets. The kubeconfig holds one context per
namespace, named exactly like the namespace; the gateway selects it per target (`kubectl --context <namespace>`).
`setup.sh --remove` deletes the objects and the kubeconfig.

Which machine may deploy is decided by the hub: every gateway token has the scope `build`, the tokens of the machines of
`CI_DEPLOY_MACHINES` (`LAB_CI_DEPLOY_MACHINES` of the hub, for example the workstation) also `deploy`. `sudo gucefLabLink
--list` shows `ci-deploy` after those machines.

## How a model uses the tools

Claude Code offers the tools of `ci` in every session started after the registration. The model calls
`ci_resolve_repo` with the path of a checkout to learn its CI id, `ci_submit` once to send the snapshot and start the run
(the full matrix unless it filters), `ci_status` with `wait_seconds` until the run is finished, `ci_report` for what failed,
and `ci_job_details` only for failed jobs. `ci_cancel` stops a run, `ci_cleanup` removes old scratch branches and reports.
Only repositories that the registry knows are built.

## Files and ports

| Path or port | Role | Content |
|---|---|---|
| `/opt/gucef-mcp-ci/<version>`, `current` | both | the program and its libraries; `current` points to the one in use |
| `/var/cache/gucef-mcp-ci` | both | the build and its logs (`logs/build.log`, `logs/test.log`) |
| `/etc/gucef-ci/install.env` | both | role, user and name of the installation, for `--upgrade` and `uninstall.sh` |
| `/var/lib/gucef-mcp/ci` | client | `ci.json`, `gateway.token`, `ca.crt`, written by lablink for the user of Claude Code |
| `/etc/gucef-ci/ci.json` | gateway | the CI registry of the lab, also served by the lablink hub |
| `/etc/gucef-ci/gateway.env` | gateway | the port, kubectl and the kubeconfig of the deployments, and the digests of the registry and the kubeconfig the service was started with |
| `/var/lib/gucef-ci/forgejo.token` | gateway | the Forgejo token of `ci-bot`, owner `gucef-ci`, mode 0600 |
| `/var/lib/gucef-ci/tokens/gateway-tokens` | gateway | the SHA-256 of each gateway token, written by the hub, owner `gucef-ci`, mode 0600 |
| `/var/lib/gucef-ci/tls` | gateway | the copy of the node certificate and key |
| `/var/lib/gucef-ci/kubeconfig` | gateway | the restricted ServiceAccounts of the deployments (`ci/deployer/setup.sh`), owner `gucef-ci`, mode 0600 |
| `/var/lib/gucef-ci/.kube` | gateway | the cache of kubectl |
| `/opt/gucef-mcp-ci/kubectl` | gateway | runs `k3s kubectl`, only when there is no `kubectl` |
| `/var/lib/gucef-ci/work`, `logs` | gateway | bare repositories per CI id, uploads, logs of jobs |
| 8792/tcp | gateway | the gateway API (HTTPS, bearer token) |

## Troubleshooting

| Symptom | Cause and remedy |
|---|---|
| The tools of `ci` say that no gateway or token is configured | `/var/lib/gucef-mcp/ci` is missing or incomplete: approve the machine on the hub (`sudo gucefLabLink --approve <name>`), check that lablink runs without `--no-ci`, and wait for its next visit (`journalctl -u gucef-lablink -f`) |
| A token file "cannot be used" | lablink gives the files to `LAB_CLAUDE_USER`; the server reads owner-only files of its own user only. Set `LAB_CLAUDE_USER` to the user of Claude Code |
| The gateway cannot be reached | `systemctl status gucef-ci-gateway`, `journalctl -u gucef-ci-gateway -n 50`; the firewall must let 8792/tcp in; the client must trust the lab authority (`ca.crt`) |
| `gucef-ci-gateway` is disabled | no Forgejo token: see the end of the output of `install.sh`, then run it again |
| The gateway does not start | no certificate in `/var/lib/gucef-ci/tls` yet: run `llm/mcp/cluster.sh` on the server; the gateway starts once the certificate is copied |
| Every request of a machine is refused | the hub took its gateway token away (`--revoke`) or never made one; `sudo gucefLabLink --list` shows `ci-token` after machines that have one |
| The gateway does not deploy | the summary of `install.sh` says why (`Deployments: off: ...`); the machine needs the scope `deploy` (`ci-deploy` in `sudo gucefLabLink --list`, set with `CI_DEPLOY_MACHINES`); `projects/k3s/ci/deployer/setup.sh --verify` checks the ServiceAccounts |
| A changed `ci.json` has no effect on the gateway | run `install.sh --role gateway` again (it restarts the gateway when the registry changed), or `sudo systemctl restart gucef-ci-gateway` |
