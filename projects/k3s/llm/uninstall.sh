#!/bin/bash
# ---------------------------------------------------------------------------
# Removes the LLM server. The model PVC (downloaded models) is kept unless
# --delete-models is passed.
# ---------------------------------------------------------------------------

KUBECONFIG_PATH="${KUBECONFIG_PATH:-/etc/rancher/k3s/k3s.yaml}"
KUBECTL="kubectl --kubeconfig ${KUBECONFIG_PATH}"
NAMESPACE="llm"
APP_NAME="llama-server"

DELETE_MODELS=false
for arg in "$@"; do
    [ "${arg}" = "--delete-models" ] && DELETE_MODELS=true
done

${KUBECTL} delete deployment "${APP_NAME}" -n "${NAMESPACE}" --ignore-not-found
${KUBECTL} delete service "${APP_NAME}" -n "${NAMESPACE}" --ignore-not-found
${KUBECTL} delete secret "${APP_NAME}-secrets" -n "${NAMESPACE}" --ignore-not-found

if [ "${DELETE_MODELS}" = "true" ]; then
    ${KUBECTL} delete namespace "${NAMESPACE}" --ignore-not-found
else
    echo "Kept PVC ${APP_NAME}-models in namespace ${NAMESPACE}. Use --delete-models to remove it."
fi
