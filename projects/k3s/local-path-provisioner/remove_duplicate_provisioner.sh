#!/bin/bash
# ---------------------------------------------------------------------------
# Removes an upstream rancher/local-path-provisioner installed in namespace
# "local-path-storage" alongside the k3s built-in one in kube-system.
# Both register provisioner "rancher.io/local-path" and share ClusterRole /
# ClusterRoleBinding names; the upstream copy cannot run alongside k3s's.
#
# Only the namespace is deleted. Do NOT "kubectl delete -f" the upstream
# manifest: it contains the shared "local-path" StorageClass and the shared
# ClusterRole/ClusterRoleBinding used by the k3s provisioner.
#
# Idempotent.
# ---------------------------------------------------------------------------

KUBECONFIG_PATH="${KUBECONFIG_PATH:-/etc/rancher/k3s/k3s.yaml}"
KUBECTL="kubectl --kubeconfig ${KUBECONFIG_PATH}"
DUPLICATE_NAMESPACE="local-path-storage"

if ! ${KUBECTL} get namespace "${DUPLICATE_NAMESPACE}" > /dev/null 2>&1; then
    echo "No duplicate local-path provisioner found (namespace ${DUPLICATE_NAMESPACE} does not exist)"
    exit 0
fi

# Only remove it if the k3s built-in provisioner is present to take over
if ! ${KUBECTL} get deployment local-path-provisioner -n kube-system > /dev/null 2>&1; then
    echo "WARNING: k3s built-in local-path provisioner (kube-system) not found."
    echo "Leaving namespace ${DUPLICATE_NAMESPACE} in place since it may be the only local-path provisioner."
    exit 0
fi

echo "Removing duplicate local-path provisioner namespace ${DUPLICATE_NAMESPACE}"
${KUBECTL} delete namespace "${DUPLICATE_NAMESPACE}" --wait=true --timeout=120s
