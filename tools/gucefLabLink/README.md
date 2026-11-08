# gucefLabLink

Links the machines of a home network to its k3s cluster without manual steps. The cluster announces itself on the
local network; a machine that finds it, and is allowed to, joins it as a node (Linux) and sets up its Claude Code for
the MCP services of the nodes (any machine). Machines that are switched off drop out; when they are on again they are
back.

## Roles

One program, `gucefLabLink`, runs as a service on every machine. Its role follows from the machine:

| Role | Machine | What it does |
|---|---|---|
| cluster | a k3s server node | announces the cluster, answers the machines that link to it |
| machine | any other machine | finds the cluster, links to it, joins it as a node when allowed, configures Claude Code |

## Discovery

The cluster advertises the DNS-SD service `_gucef-lab._tcp` over mDNS (`<cluster>._gucef-lab._tcp.local`), and the
same entry with UDP broadcasts for networks where mDNS does not pass. The entry holds no secrets: the cluster name,
the endpoints of the two links below, and the public key of the cluster's link. Machines only use entries whose
address is in their allowed ranges.

## Links and trust

Every link is a gucefDRN link: X25519 key agreement and an AEAD cipher (XChaCha20-Poly1305, from the codec plugin
`codecspluginMONOCYPHER`), on Windows, Linux and macOS alike.

The cluster has two endpoints:

| Endpoint | Port | Who | Offers |
|---|---|---|---|
| lab | 47810/tcp | machines with a token | the service `gucef.lab.Lab` |
| access | 47811/tcp | any machine in the allowed ranges, anonymous | the service `gucef.lab.access.Access`: asking for access |

A machine gets onto the lab endpoint with a token (DRN token authenticator, HMAC, bound to the connection):

- **Lab secret.** A machine that has the lab secret makes its own short-lived tokens with the key derived from it.
  The secret is made by the cluster (`gucefLabLink --init`) and entered once on each machine
  (`gucefLabLink --set-secret`). It never travels: tokens are made from it.
- **Approval.** A machine without the secret asks on the access endpoint. The request waits until it is approved on
  the cluster (`gucefLabLink --approve NAME`); the machine then receives a token of its own, valid for a year, and
  keeps it. Taking the approval back (`--revoke NAME`) refuses the token.

The cluster proves itself to the machine: its answer to `Hello` holds an HMAC, made with the key derived from the lab
secret, over the public key of the link the machine is connected through. A machine with the secret checks it, so a
device that pretends to be the cluster gets nowhere. A machine that was approved keeps the public key of the cluster
it was approved by and only talks to that key afterwards.

Both sides check addresses: the cluster answers only machines in its allowed ranges, a machine only uses clusters in
its allowed ranges (`--allow 172.16.0.0/24 --allow 172.16.1.0/24`). Addresses on a LAN can be faked: the ranges limit
who can try, the token decides.

## Service `gucef.lab.Lab`

| Method | Gives |
|---|---|
| `Hello` | the cluster name, the k3s version, the proof of the cluster |
| `Join(name, os, arch, ephemeral)` | what `projects/k3s/agent-node/join.sh` needs: the server URL, the k3s version and a bootstrap token valid for one hour |
| `ClientConfig` | the certificate authority of the MCP services, the MCP token, and the MCP endpoint of every node |

## What a machine does with it

- **Linux, joining allowed** (`--join auto`): when it is not a k3s node yet, it runs `join.sh` with what `Join` gave,
  `--ephemeral` unless configured otherwise. `join.sh` prepares the host, joins, and installs the MCP service of the
  machine.
- **Claude Code** (`--claude-user NAME`): the authority goes to a file the user's Node.js trusts
  (`NODE_EXTRA_CA_CERTS`), and every node's MCP service is registered with Claude Code of the user as
  `<node>-llm`, with the MCP token. Nodes that are added or removed are registered or removed.

## Commands

| Command | Where | Does |
|---|---|---|
| `gucefLabLink --init` | cluster | makes the lab secret when there is none, prints it |
| `gucefLabLink --set-secret SECRET` | machine | keeps the lab secret |
| `gucefLabLink --list` | cluster | the machines that asked for access |
| `gucefLabLink --approve NAME\|ID` / `--revoke NAME\|ID` | cluster | approves a machine / refuses its token from then on |

Without a command it runs as the service; `--help` lists its options. `projects/k3s/lablink/install.sh` builds, tests
and installs it as the systemd service `gucef-lablink`; `projects/k3s/cluster-node-installation-media/make-media.sh` makes packages of it and an
install medium for machines without a checkout.

## Ports

| Port | Use |
|---|---|
| 47810/tcp | lab endpoint (cluster) |
| 47811/tcp | access endpoint (cluster) |
| 5353/udp | mDNS |
| 47812/udp | UDP broadcast discovery |

## Files

| Path | Machine | Holds |
|---|---|---|
| `/var/lib/gucef-lablink/secret` | any, root only | the lab secret |
| `/var/lib/gucef-lablink/identity` | machine | the machine id and, after approval, its token and the cluster key |
| `/var/lib/gucef-lablink/approvals` | cluster | approved and waiting machines |
| `/var/lib/gucef-lablink/lab.key` | cluster | the static key of the lab endpoint, which machines pin |
| `/var/lib/gucef-lablink/claude-registrations` | machine | what it registered with Claude Code, to change only what differs |

The cluster reads the MCP files the DaemonSet of `projects/k3s/llm/mcp/cluster.sh` delivers to `/var/lib/gucef-mcp`:
`tls/ca.crt`, `lab-token` and `mcp-nodes`.
