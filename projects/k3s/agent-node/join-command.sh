#!/bin/bash
# ---------------------------------------------------------------------------
# Prints the command that joins another machine to this cluster as an agent
# node, with a new bootstrap token. Run on a k3s server node. Needs read
# access to the kubeconfig (see host-node/set_kubeconfig_access.sh), not root.
#
# Usage: ./join-command.sh [--ephemeral] [--ttl DURATION] [--values] [--node NAME]
#   --ephemeral  the machine is not always on (see README.md)
#   --node NAME  the node name the machine joins with: when there is no node of
#                that name, a node password k3s kept for the name (a join that
#                did not complete) is taken out, so the machine can join afresh
#   --values     prints server=, version=, token= and ephemeral= lines instead,
#                for programs (gucefLabLink)
#   --ttl        lifetime of the bootstrap token (default 2h). The token is
#                only used for the first join; afterwards the node uses its
#                own credentials.
#
# Environment:
#   K3S_JOIN_SERVER  server URL to join (default https://<InternalIP of this node>:6443)
#   K3S_BIN          k3s binary (default /usr/local/bin/k3s)
# ---------------------------------------------------------------------------

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
KUBECONFIG_PATH="${KUBECONFIG_PATH:-/etc/rancher/k3s/k3s.yaml}"
KUBECTL="kubectl --kubeconfig ${KUBECONFIG_PATH}"
K3S_BIN="${K3S_BIN:-/usr/local/bin/k3s}"

# shellcheck source=../nvidia/node_detect.sh
. "${SCRIPT_DIR}/../nvidia/node_detect.sh"

EPHEMERAL=false
TTL="2h"
VALUES=false
JOINING_NODE=""
while [ $# -gt 0 ]; do
    case "$1" in
        --ephemeral) EPHEMERAL=true ;;
        --ttl) TTL="$2"; shift ;;
        --values) VALUES=true ;;
        --node) JOINING_NODE="$2"; shift ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
    shift
done

if [ ! -r "${KUBECONFIG_PATH}" ]; then
    echo "ERROR: cannot read ${KUBECONFIG_PATH}. Run host-node/set_kubeconfig_access.sh, or use sudo."
    exit 1
fi
if [ ! -x "${K3S_BIN}" ]; then
    echo "ERROR: ${K3S_BIN} not found. Run on a k3s server node."
    exit 1
fi

SELF="$(gucef_k8s_node_name "${KUBECONFIG_PATH}")"
if [ -z "${SELF}" ]; then
    echo "ERROR: could not determine the Kubernetes node name of this machine."
    exit 1
fi
VERSION="$(${KUBECTL} get node "${SELF}" -o jsonpath='{.status.nodeInfo.kubeletVersion}')"
SERVER_URL="${K3S_JOIN_SERVER}"
if [ -z "${SERVER_URL}" ]; then
    SERVER_IP="$(${KUBECTL} get node "${SELF}" -o jsonpath='{.status.addresses[?(@.type=="InternalIP")].address}')"
    SERVER_URL="https://${SERVER_IP}:6443"
fi
if [ -z "${VERSION}" ] || [ "${SERVER_URL}" = "https://:6443" ]; then
    echo "ERROR: could not read the version or address of node ${SELF}."
    exit 1
fi

# a join of this name that did not complete leaves its node password behind; a new join would be refused with it
if [ -n "${JOINING_NODE}" ]; then
    if [[ ! "${JOINING_NODE}" =~ ^[a-z0-9]([-a-z0-9.]*[a-z0-9])?$ ]]; then
        echo "ERROR: ${JOINING_NODE} is no valid node name." >&2
        exit 1
    fi
    if ! ${KUBECTL} get node "${JOINING_NODE}" > /dev/null 2>&1 &&
       ${KUBECTL} get secret -n kube-system "${JOINING_NODE}.node-password.k3s" > /dev/null 2>&1; then
        ${KUBECTL} delete secret -n kube-system "${JOINING_NODE}.node-password.k3s" > /dev/null 2>&1 &&
            echo "Took out the node password of ${JOINING_NODE}, left by a join that did not complete." >&2
    fi
fi

TOKEN="$("${K3S_BIN}" token create --kubeconfig "${KUBECONFIG_PATH}" --ttl "${TTL}" \
    --description "agent join, created by join-command.sh" 2>/dev/null | tail -n 1)"
if [ -z "${TOKEN}" ]; then
    echo "ERROR: could not create a bootstrap token:"
    "${K3S_BIN}" token create --kubeconfig "${KUBECONFIG_PATH}" --ttl "${TTL}"
    exit 1
fi

if [ "${VALUES}" = "true" ]; then
    echo "server=${SERVER_URL}"
    echo "version=${VERSION}"
    echo "token=${TOKEN}"
    echo "ephemeral=${EPHEMERAL}"
    exit 0
fi

OPTIONS="--server ${SERVER_URL} --version ${VERSION} --token ${TOKEN}"
[ "${EPHEMERAL}" = "true" ] && OPTIONS="${OPTIONS} --ephemeral"

echo "Run on the machine that joins, from projects/k3s/agent-node of this repository"
echo "(the token is valid for ${TTL} and only needed for the first join):"
echo ""
echo "  sudo ./join.sh ${OPTIONS}"
echo ""
echo "A node that already joined moves to the version of this server with:"
echo ""
echo "  sudo ./join.sh --version ${VERSION}"
