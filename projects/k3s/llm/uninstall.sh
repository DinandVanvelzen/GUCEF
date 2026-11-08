#!/bin/bash
# ---------------------------------------------------------------------------
# Removes an LLM server instance. Its model PVC (downloaded models) is kept
# unless --delete-models is passed. Model files in a directory on a node
# (LLM_HOST_MODEL_DIR) are never removed by this script.
#
# Usage: ./uninstall.sh [--node NODE | --instance NAME] [--delete-models]
#   (no options)   the main instance "llama-server"
#   --node NODE    the instance created with setup.sh --node NODE
#   --instance N   the instance created with setup.sh --instance N
# The namespace is removed with the last instance when --delete-models is passed.
# The MCP services stop listing the instance (mcp/cluster.sh).
# ---------------------------------------------------------------------------

KUBECONFIG_PATH="${KUBECONFIG_PATH:-/etc/rancher/k3s/k3s.yaml}"
KUBECTL="kubectl --kubeconfig ${KUBECONFIG_PATH}"
NAMESPACE="llm"

INSTANCE=""
DELETE_MODELS=false
while [ $# -gt 0 ]; do
    case "$1" in
        --node|--instance) INSTANCE="$2"; shift ;;
        --delete-models) DELETE_MODELS=true ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
    shift
done
APP_NAME="llama-server${INSTANCE:+-${INSTANCE}}"

${KUBECTL} delete deployment "${APP_NAME}" -n "${NAMESPACE}" --ignore-not-found
${KUBECTL} delete service "${APP_NAME}" -n "${NAMESPACE}" --ignore-not-found
${KUBECTL} delete secret "${APP_NAME}-secrets" -n "${NAMESPACE}" --ignore-not-found

if [ "${DELETE_MODELS}" = "true" ]; then
    ${KUBECTL} delete pvc "${APP_NAME}-models" -n "${NAMESPACE}" --ignore-not-found
    if [ -z "$(${KUBECTL} get deployments,pvc -n "${NAMESPACE}" -o name 2>/dev/null)" ]; then
        ${KUBECTL} delete namespace "${NAMESPACE}" --ignore-not-found
    fi
elif ${KUBECTL} get pvc "${APP_NAME}-models" -n "${NAMESPACE}" > /dev/null 2>&1; then
    echo "Kept PVC ${APP_NAME}-models in namespace ${NAMESPACE}. Use --delete-models to remove it."
fi

# the list of LLM servers of the MCP services without this instance
if [ -x "$(dirname "${BASH_SOURCE[0]}")/mcp/cluster.sh" ] && ${KUBECTL} get namespace gucef-mcp > /dev/null 2>&1; then
    "$(dirname "${BASH_SOURCE[0]}")/mcp/cluster.sh"
fi
