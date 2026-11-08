# MCP service `gucef-mcp`

Every node of the cluster runs the MCP server `gucefMcpLocalLlm` (see [../README.md](../README.md#claude-code-tools-mcp-server-gucefmcplocalllm))
as the systemd service `gucef-mcp`, for as long as the node is up. Any Claude session can use it:

| Client | URL |
|---|---|
| Claude Code on the node | `http://127.0.0.1:8790/mcp` |
| Claude Code on another machine | `https://<node>:8791/mcp` |

Both need the lab MCP token, `/var/lib/gucef-mcp/lab-token`, the same on every node (until it is there, a node
uses a token of its own, `/var/lib/gucef-mcp/token`). The tools work on the node: paths are paths
of that node and commands run there, as the user the service runs as. The server says so in its instructions.

## Setup

Nothing to do by hand on the nodes:

- `llm/setup.sh` (on a server) installs the service on the server and brings the cluster side up to date
- `agent-node/join.sh` installs the service on a machine that joins; `llm/setup.sh --node <name>` or
  `llm/mcp/cluster.sh` on a server then gives it its certificate and the LLM servers

The service runs as the user that ran `sudo`. To install or update it alone:

```bash
sudo llm/mcp/install.sh            # on the node, from the user the service runs as
sudo llm/mcp/cluster.sh            # on a server: certificates and LLM servers for all nodes
```

| Script | Runs on | What it does |
|---|---|---|
| `install.sh [--user NAME] [--skip-tests] [--prebuilt DIR] [--upgrade]` | every node, as root | builds the checkout it is in (`/var/lib/gucef-mcp/build`), runs the tests, installs the server stripped in `/opt/gucef-mcp/<version>`, sets up and (re)starts the service, registers it with Claude Code of the user as `local-llm`. Without a checkout (package `gucef-k3s-scripts`, see [../../cluster-node-installation-media/README.md](../../cluster-node-installation-media/README.md)) it installs the files of the package `gucef-mcp` instead of building |
| `uninstall.sh [--purge]` | the node, as root | removes the service; `--purge` also `/var/lib/gucef-mcp` |
| `cluster.sh` | a server, as root | the certificate authority, a certificate per node, the list of LLM servers, and the DaemonSet that copies them to each node |

`install.sh` is idempotent: it installs and restarts only when the build or the settings differ. A failing build or
test keeps the installed version. Restarting ends work that runs on the service at that moment. Settings:
`config.env` next to the script, or `/etc/gucef-mcp/settings.env` without a checkout ([config.env.example](config.env.example)): ports, the commands `run_and_digest` may
run, the preferred model, the Claude Code registration.

## What the cluster gives each node

`cluster.sh` keeps in namespace `gucef-mcp`, and DaemonSet `gucef-mcp-sync` copies to `/var/lib/gucef-mcp` of each
node (owned by the service user), within a minute of a change:

| File | From | Used for |
|---|---|---|
| `tls/tls.crt`, `tls/tls.key` | Secret `gucef-mcp-tls`: a certificate for the node's name, host name and address, valid 2 years, renewed by `cluster.sh` 60 days before it expires | HTTPS; the server reads them again when they change |
| `tls/ca.crt` | Secret `gucef-mcp-ca`: the authority, made once | what clients on other machines trust |
| `endpoints` | ConfigMap `gucef-mcp-endpoints`: the LoadBalancer address of each LLM instance, and node address and host port of instances on ephemeral nodes | the LLM servers; the server reads the file again when it changes |
| `llm-api-key` | `LLM_API_KEY` of `llm/config.env` | when the LLM servers require a key |
| `lab-token` | Secret `gucef-mcp-llm`: made once | the bearer token every node's MCP service accepts; the service restarts once to take it |
| `mcp-nodes` | ConfigMap `gucef-mcp-endpoints`: every node with its HTTPS endpoint | `gucefLabLink`, which registers them with Claude Code of the machines of the lab |

Until a node has its certificate the service listens on the local port only; it starts HTTPS by itself once the
certificate is there (`gucef-mcp-tls.path`). Namespace `gucef-mcp` holds the keys of all nodes: whoever can read
Secrets there, or is root on a node, can read them.

## Claude Code on another machine

`gucefLabLink` does this by itself on the machines of the lab: see [../../lablink/README.md](../../lablink/README.md).
By hand: copy `/var/lib/gucef-mcp/tls/ca.crt` and the lab token from a node. Node.js, and so Claude Code, takes additional
trusted authorities from `NODE_EXTRA_CA_CERTS`, which has to be set before `claude` starts (in the shell profile):

```bash
export NODE_EXTRA_CA_CERTS=$HOME/.config/gucef-mcp-ca.crt
claude mcp add --transport http worker-llm https://<node>:8791/mcp --scope user \
    --header "Authorization: Bearer <token>"
```

On Windows: `setx NODE_EXTRA_CA_CERTS C:\Users\<you>\.config\gucef-mcp-ca.crt`, then a new terminal.

## Operations

```bash
systemctl status gucef-mcp
journalctl -u gucef-mcp -f
sudo systemctl restart gucef-mcp
ls -l /opt/gucef-mcp            # installed versions, current -> the one in use
```

The ports must be reachable from the client machines: 8791/tcp (HTTPS). The local port listens on 127.0.0.1 only.
