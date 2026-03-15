#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SECRETS_FILE="${SCRIPT_DIR}/secrets.env"
OUTPUT_FILE="${SCRIPT_DIR}/sealed-secret.yaml"
NAMESPACE="postgresql"
SECRET_NAME="postgresql-credentials"
KUBECONFIG_PATH="/etc/rancher/k3s/k3s.yaml"

if [ ! -f "${SECRETS_FILE}" ]; then
    echo "ERROR: ${SECRETS_FILE} not found."
    echo "Copy secrets.env.example to secrets.env and fill in your values first."
    exit 1
fi

# Load secrets (strip Windows carriage returns in case file was edited on Windows)
set -a
source <(sed 's/\r//' "${SECRETS_FILE}")
set +a

if [ -z "${POSTGRES_PASSWORD}" ] || [ -z "${IMMICH_DB_PASSWORD}" ]; then
    echo "ERROR: POSTGRES_PASSWORD and IMMICH_DB_PASSWORD must both be set in secrets.env"
    exit 1
fi

echo "Generating sealed secret for namespace: ${NAMESPACE}..."

kubectl create secret generic "${SECRET_NAME}" \
    --namespace "${NAMESPACE}" \
    --from-literal=POSTGRES_PASSWORD="${POSTGRES_PASSWORD}" \
    --from-literal=IMMICH_DB_PASSWORD="${IMMICH_DB_PASSWORD}" \
    --dry-run=client -o yaml \
| ~/.local/bin/kubeseal \
    --kubeconfig "${KUBECONFIG_PATH}" \
    --controller-name=sealed-secrets \
    --controller-namespace=kube-system \
    --format yaml \
> "${OUTPUT_FILE}"

echo "Sealed secret written to ${OUTPUT_FILE}"
echo "This file is safe to commit to git."
