#!/bin/bash
# ---------------------------------------------------------------------------
# Removes the k3s agent from this machine: stops its pods and uninstalls k3s
# (k3s-agent-uninstall.sh), including its container images and the data of
# its pods under /var/lib/rancher. Host directories that workloads mount
# (e.g. the LLM model directory, /var/lib/gucef) are kept.
#
# The node object stays in the cluster as NotReady; remove it on a server with:
#   kubectl delete node <name>
# The MCP service gucef-mcp stays; llm/mcp/uninstall.sh removes it.
#
# Usage: sudo ./leave.sh [--yes]
# ---------------------------------------------------------------------------

DROPIN_FILE="/etc/rancher/k3s/config.yaml.d/50-gucef-agent.yaml"
UNINSTALL_SCRIPT="/usr/local/bin/k3s-agent-uninstall.sh"

ASSUME_YES=false
[ "$1" = "--yes" ] && ASSUME_YES=true

if [ "$(id -u)" -ne 0 ]; then
    echo "ERROR: must run as root (use sudo)."
    exit 1
fi
if [ -f /etc/systemd/system/k3s.service ]; then
    echo "ERROR: this machine is a k3s server node, not an agent."
    exit 1
fi
if [ ! -x "${UNINSTALL_SCRIPT}" ]; then
    echo "No k3s agent installed. Nothing to do."
    exit 0
fi

NODE_NAME="$(sed -n 's/^node-name: *//p' "${DROPIN_FILE}" 2>/dev/null | tr -d '"')"
NODE_NAME="${NODE_NAME:-$(hostname | tr '[:upper:]' '[:lower:]')}"

if [ "${ASSUME_YES}" != "true" ]; then
    ANSWER="n"
    [ -t 0 ] && read -r -p "Remove the k3s agent (node ${NODE_NAME}) from this machine? [y/N] " ANSWER
    [[ "${ANSWER}" =~ ^[Yy] ]] || { echo "Aborted."; exit 1; }
fi

"${UNINSTALL_SCRIPT}" || exit 1
rm -f "${DROPIN_FILE}"
echo ""
echo "k3s agent removed. On a server node, remove the node object with:"
echo "  kubectl delete node ${NODE_NAME}"
