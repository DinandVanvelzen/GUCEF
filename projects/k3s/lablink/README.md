# Lab link

Machines of the home network find the k3s cluster themselves, join it as nodes and set up Claude Code for the MCP
services of the nodes. The service is `gucef-lablink`, the program `tools/gucefLabLink` (how it works, and its trust
model: [tools/gucefLabLink/README.md](../../../tools/gucefLabLink/README.md)).

## Setup

On a k3s server node, once:

```bash
sudo lablink/install.sh
```

That sets up the whole cluster side: the firewall ports, the MCP service of the node, the certificates, lab MCP token
and node list of the MCP services (kept current by a timer as nodes come and go), and the lab link. It shows the lab
secret. On each other machine, from `projects/k3s/lablink` of a checkout of this repository:

```bash
sudo ./install.sh --secret <the secret>
```

That is all: the machine finds the cluster within a minute, joins it as an ephemeral node (Linux; see
[../agent-node/README.md](../agent-node/README.md)), which also installs its own MCP service, and registers the MCP
service of every node with Claude Code of the user that ran `sudo`. Machines that are switched off drop out and come
back when they are on.

Or without a checkout, from an install medium such as a USB stick, which `cluster-node-installation-media/make-media.sh` makes (see
[../cluster-node-installation-media/README.md](../cluster-node-installation-media/README.md)):

```bash
sudo bash /media/$USER/<stick>/gucef-lab/install.sh
```

Without `--secret` a machine asks the cluster for access and waits. On the cluster:

```bash
sudo gucefLabLink --list
sudo gucefLabLink --approve <name>
sudo gucefLabLink --revoke <name>        # its token is refused from then on
```

The certificates and the list of MCP services come from `llm/mcp/cluster.sh`; its timer `gucef-mcp-cluster` runs it
within 2 minutes after a machine joined, so the new node's MCP service gets its certificate and is in the list.

## Settings

`config.env` next to `install.sh`, or `/etc/gucef-lablink/settings.env` when installed from the packages
([config.env.example](config.env.example)):

| Setting | Default |
|---|---|
| `LAB_ALLOW` | the networks of the machine's interfaces (not those of containers): who may link, and which clusters a machine uses |
| `LAB_ROLE` | `cluster` on a k3s server node, `machine` elsewhere |
| `LAB_JOIN` | `true`: a Linux machine joins as a node |
| `LAB_PERMANENT` | `false`: it joins as an ephemeral node |
| `LAB_LLM` | `true`: a machine with an NVIDIA GPU gets an LLM server of the cluster on it when it joins |
| `LAB_CLAUDE_USER` | the user that ran `sudo` |
| `LAB_MDNS`, `LAB_BROADCAST` | `true` |
| `LAB_CLUSTER_NAME` | the host name |

Ports: 47810/tcp and 47811/tcp on the cluster, 5353/udp (mDNS) and 47812/udp (broadcast) everywhere, and those of k3s
and the MCP service on nodes. `install.sh` opens them for the lab networks (`LAB_ALLOW`) when ufw or firewalld is active
([../host-node/open_firewall_ports.sh](../host-node/open_firewall_ports.sh)).

```bash
systemctl status gucef-lablink
journalctl -u gucef-lablink -f
sudo lablink/uninstall.sh [--purge]      # removes the service; --purge also the secret and the identity
```

Windows and macOS machines follow (phase 2 and 3: Claude Code setup, and a node in WSL2 on Windows).
