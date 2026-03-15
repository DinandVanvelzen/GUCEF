#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
KUBECONFIG_PATH="/etc/rancher/k3s/k3s.yaml"

# print the current services
kubectl --kubeconfig ${KUBECONFIG_PATH} get services -n postgresql

# Create namespace and PVC first
kubectl --kubeconfig ${KUBECONFIG_PATH} create namespace postgresql --dry-run=client -o yaml | kubectl --kubeconfig ${KUBECONFIG_PATH} apply -f -
kubectl --kubeconfig ${KUBECONFIG_PATH} apply -f - <<EOF
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: postgres-data-pvc
  namespace: postgresql
spec:
  accessModes:
    - ReadWriteOnce
  storageClassName: longhorn-raid
  resources:
    requests:
      storage: 50Gi
EOF

# Apply the sealed secret (credentials) before installing the chart
# Run create-sealed-secret.sh first if sealed-secret.yaml does not exist
if [ ! -f "${SCRIPT_DIR}/sealed-secret.yaml" ]; then
    echo "sealed-secret.yaml not found - running create-sealed-secret.sh..."
    bash "${SCRIPT_DIR}/create-sealed-secret.sh" || { echo "ERROR: Failed to create sealed secret. Ensure secrets.env exists."; exit 1; }
fi
kubectl --kubeconfig ${KUBECONFIG_PATH} apply -f "${SCRIPT_DIR}/sealed-secret.yaml" -n postgresql

# Add the repo that has the helm chart we want to use
helm repo add groundhog2k https://groundhog2k.github.io/helm-charts/ --kubeconfig ${KUBECONFIG_PATH} || true
helm repo update --kubeconfig ${KUBECONFIG_PATH}

# upgrade --install is idempotent: installs on first run, upgrades on subsequent runs
helm upgrade --install my-postgres groundhog2k/postgres \
    --version 1.5.1 \
    --namespace postgresql \
    --create-namespace \
    --kubeconfig ${KUBECONFIG_PATH} \
    --values="${SCRIPT_DIR}/values.yaml"

# Verify installation
kubectl --kubeconfig ${KUBECONFIG_PATH} --namespace postgresql get all
kubectl --kubeconfig ${KUBECONFIG_PATH} get services -n postgresql
