#!/bin/bash
# ---------------------------------------------------------------------------
# Removes the NVIDIA device plugin and the GPU node labels from the cluster.
# The host driver and container toolkit are left installed.
# ---------------------------------------------------------------------------

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
KUBECONFIG_PATH="${KUBECONFIG_PATH:-/etc/rancher/k3s/k3s.yaml}"
KUBECTL="kubectl --kubeconfig ${KUBECONFIG_PATH}"
NAMESPACE="nvidia-device-plugin"
RELEASE_NAME="nvidia-device-plugin"

# shellcheck source=node_detect.sh
. "${SCRIPT_DIR}/node_detect.sh"

helm uninstall "${RELEASE_NAME}" --namespace "${NAMESPACE}" --kubeconfig "${KUBECONFIG_PATH}" --ignore-not-found
${KUBECTL} delete namespace "${NAMESPACE}" --ignore-not-found

NODE_NAME="$(gucef_k8s_node_name "${KUBECONFIG_PATH}")"
if [ -n "${NODE_NAME}" ]; then
    ${KUBECTL} label node "${NODE_NAME}" nvidia.com/gpu.present- gucef.org/gpu- gucef.org/gpu-vram-mib- 2>/dev/null || true
fi
