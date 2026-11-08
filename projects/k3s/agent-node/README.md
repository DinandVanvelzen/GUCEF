# Agent nodes

Scripts to add machines to the cluster as k3s agent nodes, including machines that are not always on, such as a
workstation with a GPU: an *ephemeral* node.

| Script | Runs on | What it does |
|---|---|---|
| `join-command.sh [--ephemeral] [--ttl 2h]` | a server node, no root needed | creates a bootstrap token and prints the `join.sh` command for the new machine |
| `join.sh --server URL --token T --version V [--ephemeral]` | the machine that joins, as root | prepares the host (inotify limits, NVIDIA driver and container toolkit), configures and installs the k3s agent |
| `join.sh --version V` | a node that already joined, as root | moves it to another k3s version, e.g. after `upgrade.sh` upgraded the servers |
| `leave.sh` | the agent node, as root | uninstalls the k3s agent |

`lablink/install.sh` does all of this without copying commands: machines find the cluster and join it themselves
(see [../lablink/README.md](../lablink/README.md)).

`join.sh` also installs the MCP service `gucef-mcp` (local LLM tools for Claude sessions, see
[../llm/mcp/README.md](../llm/mcp/README.md)) as the user that ran `sudo`; `--skip-mcp` leaves it out.

`join.sh` is idempotent: re-running it changes only what differs. The bootstrap token is only used for the first
join; afterwards the node authenticates with its own credentials.

## Joining a machine

On a server node:

```bash
./agent-node/join-command.sh --ephemeral
```

On the new machine, from `projects/k3s/agent-node` of a checkout of this repository, run the printed command:

```bash
sudo ./join.sh --server https://<server>:6443 --version <version> --token <token> --ephemeral
```

With an NVIDIA GPU, a driver change needs a reboot: the script then exits with code 10. Reboot and run the same
command again.

Then, on a server node:

```bash
kubectl get node <name>            # Ready
nvidia/setup.sh                    # once: lets the device plugin run on ephemeral nodes
llm/setup.sh --node <name>         # an LLM server on the GPU of the new node (see llm/README.md)
```

## Ephemeral nodes

`--ephemeral` marks the node:

- label `gucef.org/node-lifecycle=ephemeral`
- taint `gucef.org/ephemeral=true:NoSchedule`: only workloads that tolerate it run there. Longhorn, ServiceLB and other
  DaemonSets do not, so the node holds no Longhorn replicas and no LoadBalancer ports. Workloads on it keep their data
  in a directory on the node, and clients reach them on host ports of the node.

When the machine shuts down, the kubelet stops its pods cleanly (graceful node shutdown, `--shutdown-grace`). The node
then shows as NotReady until the machine is on again; its pods start again by themselves. Workloads that need to keep
running belong on nodes that are always on.

`upgrade.sh` leaves ephemeral nodes out. It lists those that run another version than the servers; on such a node, run
`sudo ./join.sh --version <server version>` when it is on. Kubernetes supports agents up to 3 minor versions behind the
servers.

Labels and taints apply when a node first registers. To change them later, use `kubectl label` / `kubectl taint` on a
server, or `leave.sh`, `kubectl delete node <name>` and join again.

## Network

The nodes reach each other on 6443/tcp (API server), 8472/udp (flannel VXLAN) and 10250/tcp (kubelet), plus the host
ports of workloads such as the LLM server, and 8791/tcp for the MCP service over HTTPS. `join.sh` opens the ports of a
node for the networks of the machine when ufw or firewalld is active ([../host-node/open_firewall_ports.sh](../host-node/open_firewall_ports.sh));
`lablink/install.sh` does it for the server. The certificate of the node's MCP service comes from the timer of
`llm/mcp/cluster.sh` on the server within 2 minutes.
