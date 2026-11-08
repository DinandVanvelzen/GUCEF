#!/bin/bash
# ---------------------------------------------------------------------------
# Joins this machine to a k3s cluster as an agent node, or brings an agent
# node that already joined to another k3s version. Idempotent: re-running it
# changes only what differs.
#
#   - host preparation: inotify limits, NVIDIA driver and container toolkit
#     when the machine has an NVIDIA GPU (nvidia/setup-host.sh)
#   - node labels for the GPU (gucef.org/gpu, gucef.org/gpu-vram-mib,
#     nvidia.com/gpu.present), set when the node registers
#   - kubelet: graceful shutdown (pods are stopped cleanly when the machine
#     shuts down) and resources reserved for the machine's own use
#   - k3s agent of the given version, started at boot
#   - firewall: the ports of a node, for the networks of this machine
#     (host-node/open_firewall_ports.sh, ufw or firewalld when active)
#   - the MCP service gucef-mcp (llm/mcp/install.sh), as the user that ran sudo:
#     local LLM tools for Claude sessions on this machine and, once the node has
#     its certificate from the cluster, for sessions on other machines
#
# --ephemeral: the machine is not always on, e.g. a workstation. The node gets
#   label gucef.org/node-lifecycle=ephemeral and taint
#   gucef.org/ephemeral=true:NoSchedule, so only workloads that tolerate the
#   taint run on it. Longhorn, ServiceLB and other DaemonSets without that
#   toleration stay off it. upgrade.sh leaves ephemeral nodes out; this script
#   brings them to the server's version.
#
# Usage (join-command.sh on a server prints the first command):
#   sudo ./join.sh --server URL --token TOKEN --version VERSION [--ephemeral]
#   sudo ./join.sh --version VERSION        already joined: change the k3s version
# Options:
#   --server URL              https://<server>:6443
#   --token TOKEN             join token; only needed for the first join
#   --version VERSION         k3s version of the servers, e.g. v1.37.2+k3s1
#   --ephemeral               see above; applies at the first join
#   --node-name NAME          default: host name in lower case
#   --system-reserved VALUE   kubelet system-reserved (default cpu=1,memory=2Gi)
#   --shutdown-grace SECONDS  time pods get to stop at machine shutdown (default 60)
#   --skip-gpu                leave the NVIDIA driver and toolkit alone
#   --skip-mcp                do not install the MCP service
#
# Exit codes: 0 done, 1 error, 10 reboot required (NVIDIA driver); re-run afterwards
# ---------------------------------------------------------------------------

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
K3S_CONFIG_DIR="/etc/rancher/k3s"
DROPIN_FILE="${K3S_CONFIG_DIR}/config.yaml.d/50-gucef-agent.yaml"
# kubelet settings that are no command line flags anymore: a configuration file in the drop-in directory k3s gives the kubelet
KUBELET_DROPIN_FILE="/var/lib/rancher/k3s/agent/etc/kubelet.conf.d/50-gucef-agent.conf"
# written once the agent runs: gucefLabLink takes the machine for a node from then on (a failed join is tried again)
JOINED_MARKER="${K3S_CONFIG_DIR}/gucef-node-joined"
AGENT_SERVICE="k3s-agent"
AGENT_ENV_FILE="/etc/systemd/system/${AGENT_SERVICE}.service.env"
K3S_BIN="/usr/local/bin/k3s"
INSTALL_URL="https://get.k3s.io"
KUBELET_HEALTHZ="http://127.0.0.1:10248/healthz"
EXIT_REBOOT_REQUIRED=10
EPHEMERAL_LABEL="gucef.org/node-lifecycle=ephemeral"
EPHEMERAL_TAINT="gucef.org/ephemeral=true:NoSchedule"

# shellcheck source=../nvidia/node_detect.sh
. "${SCRIPT_DIR}/../nvidia/node_detect.sh"

SERVER_URL=""
TOKEN=""
VERSION=""
EPHEMERAL=false
NODE_NAME=""
SYSTEM_RESERVED="cpu=1,memory=2Gi"
SHUTDOWN_GRACE=60
SKIP_GPU=false
SKIP_MCP=false
while [ $# -gt 0 ]; do
    case "$1" in
        --server) SERVER_URL="$2"; shift ;;
        --token) TOKEN="$2"; shift ;;
        --version) VERSION="$2"; shift ;;
        --ephemeral) EPHEMERAL=true ;;
        --node-name) NODE_NAME="$2"; shift ;;
        --system-reserved) SYSTEM_RESERVED="$2"; shift ;;
        --shutdown-grace) SHUTDOWN_GRACE="$2"; shift ;;
        --skip-gpu) SKIP_GPU=true ;;
        --skip-mcp) SKIP_MCP=true ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
    shift
done

if [ "$(id -u)" -ne 0 ]; then
    echo "ERROR: must run as root (use sudo)."
    exit 1
fi
if ! command -v systemctl > /dev/null 2>&1 || ! command -v curl > /dev/null 2>&1; then
    echo "ERROR: systemd and curl are required."
    exit 1
fi
if [ -f /etc/systemd/system/k3s.service ]; then
    echo "ERROR: this machine is a k3s server node. Agent nodes are other machines."
    exit 1
fi
if [[ ! "${VERSION}" =~ ^v[0-9]+\.[0-9]+\.[0-9]+ ]]; then
    echo "ERROR: --version is required, e.g. --version v1.37.2+k3s1 (join-command.sh on a server prints it)."
    exit 1
fi

# ---------------------------------------------------------------------------
# Existing agent
# ---------------------------------------------------------------------------
INSTALLED_VERSION=""
# A join that did not complete (no marker and no kubelet that runs) leaves an agent with an identity the cluster does
# not accept: it is removed, and with a token the machine joins afresh. A working node without the marker is kept
if [ -f "/etc/systemd/system/${AGENT_SERVICE}.service" ] && [ ! -f "${JOINED_MARKER}" ] && [ -n "${TOKEN}" ] &&
   [ "$(curl -s --max-time 2 "${KUBELET_HEALTHZ}")" != "ok" ]; then
    echo "A previous join of this machine did not complete: the k3s agent is removed and the machine joins afresh."
    [ -z "${NODE_NAME}" ] && NODE_NAME="$(sed -n 's/^node-name: *//p' "${DROPIN_FILE}" 2>/dev/null | tr -d '"')"
    if [ -x /usr/local/bin/k3s-agent-uninstall.sh ]; then
        /usr/local/bin/k3s-agent-uninstall.sh > /dev/null 2>&1 || echo "WARNING: k3s-agent-uninstall.sh reported a problem."
    fi
    rm -rf /etc/rancher/node "${DROPIN_FILE}" "${KUBELET_DROPIN_FILE}"
fi
if [ -x "${K3S_BIN}" ] && [ -f "/etc/systemd/system/${AGENT_SERVICE}.service" ]; then
    INSTALLED_VERSION="$("${K3S_BIN}" --version 2>/dev/null | awk 'NR == 1 {print $3}')"
    EXISTING_URL="$(sed -n 's/^K3S_URL=//p' "${AGENT_ENV_FILE}" 2>/dev/null | tr -d "'\"")"
    EXISTING_TOKEN="$(sed -n 's/^K3S_TOKEN=//p' "${AGENT_ENV_FILE}" 2>/dev/null | tr -d "'\"")"
    if [ -n "${SERVER_URL}" ] && [ -n "${EXISTING_URL}" ] && [ "${SERVER_URL}" != "${EXISTING_URL}" ]; then
        echo "ERROR: this node already joined ${EXISTING_URL}. Run ./leave.sh first to join another cluster."
        exit 1
    fi
    SERVER_URL="${SERVER_URL:-${EXISTING_URL}}"
    TOKEN="${TOKEN:-${EXISTING_TOKEN}}"
    echo "k3s agent ${INSTALLED_VERSION} installed, joined to ${SERVER_URL}"
fi
if [ -z "${SERVER_URL}" ] || [ -z "${TOKEN}" ]; then
    echo "ERROR: --server and --token are required for the first join (join-command.sh on a server prints them)."
    exit 1
fi
if [ "$(curl -sk --max-time 10 "${SERVER_URL}/ping")" != "pong" ]; then
    echo "ERROR: the k3s server at ${SERVER_URL} is not reachable from this machine."
    exit 1
fi

# The node name of a joined node is kept; a k3s node cannot be renamed
if [ -z "${NODE_NAME}" ] && [ -f "${DROPIN_FILE}" ]; then
    NODE_NAME="$(sed -n 's/^node-name: *//p' "${DROPIN_FILE}" | tr -d '"')"
fi
NODE_NAME="${NODE_NAME:-$(hostname | tr '[:upper:]' '[:lower:]')}"

# ---------------------------------------------------------------------------
# Host preparation
# ---------------------------------------------------------------------------
"${SCRIPT_DIR}/../host-node/set_inotify_limits.sh" > /dev/null || exit 1

HAS_GPU=false
if [ "${SKIP_GPU}" != "true" ] && gucef_has_nvidia_gpu; then
    # --restart-k3s: a running agent restarts to register the nvidia runtime; its pods keep running
    "${SCRIPT_DIR}/../nvidia/setup-host.sh" --restart-k3s
    RESULT=$?
    if [ "${RESULT}" -eq "${EXIT_REBOOT_REQUIRED}" ]; then
        echo ""
        echo "Reboot, then re-run this command to complete the join."
        exit "${EXIT_REBOOT_REQUIRED}"
    fi
    [ "${RESULT}" -eq 0 ] || exit 1
    if [ -n "$(gucef_loaded_nvidia_driver_version)" ] && command -v nvidia-container-runtime > /dev/null 2>&1; then
        HAS_GPU=true
    else
        echo "WARNING: NVIDIA driver or container toolkit not available; joining without GPU labels."
    fi
fi

# ---------------------------------------------------------------------------
# k3s agent configuration
# Labels and taints apply when the node registers; to change them on a node
# that already joined, use kubectl label / kubectl taint on a server.
# ---------------------------------------------------------------------------
LABELS=()
TAINTS=()
if [ "${EPHEMERAL}" = "true" ] || grep -q "${EPHEMERAL_LABEL}" "${DROPIN_FILE}" 2>/dev/null; then
    EPHEMERAL=true
    LABELS+=("${EPHEMERAL_LABEL}")
    TAINTS+=("${EPHEMERAL_TAINT}")
fi
# a machine whose lab link says LAB_LLM=false gets no LLM server of the cluster on its GPU
LAB_LLM_SETTING="$( for settings in /etc/gucef-lablink/settings.env "${SCRIPT_DIR}/../lablink/config.env"; do
    [ -f "${settings}" ] && ( . "${settings}"; echo "${LAB_LLM}" ); done | grep -v '^$' | head -1 )"
[ "${LAB_LLM_SETTING}" = "false" ] && LABELS+=("gucef.org/llm=false")
if [ "${HAS_GPU}" = "true" ]; then
    LABELS+=("nvidia.com/gpu.present=true" "gucef.org/gpu=nvidia")
    VRAM_MIB="$(gucef_nvidia_vram_mib)"
    [ -n "${VRAM_MIB}" ] && LABELS+=("gucef.org/gpu-vram-mib=${VRAM_MIB}")
fi

NEW_CONFIG="$(
    echo "# Written by projects/k3s/agent-node/join.sh"
    echo "node-name: \"${NODE_NAME}\""
    if [ "${#LABELS[@]}" -gt 0 ]; then
        echo "node-label:"
        printf '  - "%s"\n' "${LABELS[@]}"
    fi
    if [ "${#TAINTS[@]}" -gt 0 ]; then
        echo "node-taint:"
        printf '  - "%s"\n' "${TAINTS[@]}"
    fi
)"
NEW_KUBELET_CONFIG="$(
    echo "# Written by projects/k3s/agent-node/join.sh"
    echo "apiVersion: kubelet.config.k8s.io/v1beta1"
    echo "kind: KubeletConfiguration"
    echo "shutdownGracePeriod: ${SHUTDOWN_GRACE}s"
    echo "shutdownGracePeriodCriticalPods: $(( SHUTDOWN_GRACE / 4 ))s"
    echo "systemReserved:"
    echo "${SYSTEM_RESERVED}" | tr ',' '\n' | while IFS='=' read -r resource amount; do
        [ -n "${resource}" ] && echo "  ${resource}: \"${amount}\""
    done
)"
CONFIG_CHANGED=false
if [ "$(cat "${DROPIN_FILE}" 2>/dev/null)" != "${NEW_CONFIG}" ]; then
    mkdir -p "$(dirname "${DROPIN_FILE}")"
    echo "${NEW_CONFIG}" > "${DROPIN_FILE}"
    CONFIG_CHANGED=true
    echo "Wrote ${DROPIN_FILE}"
fi
if [ "$(cat "${KUBELET_DROPIN_FILE}" 2>/dev/null)" != "${NEW_KUBELET_CONFIG}" ]; then
    mkdir -p "$(dirname "${KUBELET_DROPIN_FILE}")"
    echo "${NEW_KUBELET_CONFIG}" > "${KUBELET_DROPIN_FILE}"
    CONFIG_CHANGED=true
    echo "Wrote ${KUBELET_DROPIN_FILE}"
fi

# ---------------------------------------------------------------------------
# k3s agent
# ---------------------------------------------------------------------------
if [ "${INSTALLED_VERSION}" != "${VERSION}" ]; then
    echo "Installing k3s agent ${VERSION}..."
    if ! curl -sfL "${INSTALL_URL}" | INSTALL_K3S_VERSION="${VERSION}" K3S_URL="${SERVER_URL}" K3S_TOKEN="${TOKEN}" \
            sh -s - agent; then
        rm -f "${JOINED_MARKER}"
        echo "ERROR: the k3s agent installation failed. The last lines of the agent:"
        journalctl -u k3s-agent -n 40 --no-pager -o cat 2>/dev/null | sed 's/^/  /'
        exit 1
    fi
elif [ "${CONFIG_CHANGED}" = "true" ]; then
    echo "Restarting the k3s agent for the new configuration (running pods keep running)..."
    systemctl restart "${AGENT_SERVICE}"
else
    systemctl enable --now "${AGENT_SERVICE}" > /dev/null 2>&1
    echo "k3s agent ${VERSION} already installed and configured."
fi

echo "Waiting for the kubelet..."
HEALTHY=false
for _ in $(seq 1 60); do
    if [ "$(curl -s --max-time 2 "${KUBELET_HEALTHZ}")" = "ok" ]; then
        HEALTHY=true
        break
    fi
    sleep 2
done
if [ "${HEALTHY}" != "true" ]; then
    rm -f "${JOINED_MARKER}"
    echo "ERROR: the kubelet did not become healthy. Recent k3s agent log:"
    journalctl -u "${AGENT_SERVICE}" -n 30 --no-pager
    exit 1
fi
echo "k3s agent ${VERSION} running as node ${NODE_NAME}."
echo "node=${NODE_NAME} server=${SERVER_URL} version=${VERSION}" > "${JOINED_MARKER}"

# the ports a node needs, for the networks of this machine, when a firewall is active
"${SCRIPT_DIR}/../host-node/open_firewall_ports.sh" node || echo "WARNING: the firewall could not be changed: open 8472/udp, 10250/tcp and 8791/tcp to the cluster."

MCP_INSTALLED=false
if [ "${SKIP_MCP}" != "true" ]; then
    echo ""
    if [ -n "${SUDO_USER}" ] && [ "${SUDO_USER}" != "root" ]; then
        if "${SCRIPT_DIR}/../llm/mcp/install.sh" --user "${SUDO_USER}"; then
            MCP_INSTALLED=true
        else
            echo "WARNING: the MCP service was not installed, see above. Run sudo ../llm/mcp/install.sh again later."
        fi
    else
        echo "The MCP service runs as a user: sudo ../llm/mcp/install.sh from that user."
    fi
fi

echo ""
echo "Next steps, on a server node:"
echo "  kubectl get node ${NODE_NAME}"
if [ "${HAS_GPU}" = "true" ]; then
    echo "  nvidia/setup.sh                      (once: the device plugin then also runs on ephemeral nodes)"
    echo "  llm/setup.sh --node ${NODE_NAME}     (an LLM server on this node's GPU; also gives the MCP"
    echo "                                        services of all nodes its address, and this node its certificate)"
elif [ "${MCP_INSTALLED}" = "true" ]; then
    echo "  llm/mcp/cluster.sh                   (the certificate and LLM servers for the MCP service of this node)"
fi
