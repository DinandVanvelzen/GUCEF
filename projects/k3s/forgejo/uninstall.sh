#!/bin/bash
# Removes the Forgejo helm release.
# The PVC (forgejo-data-pvc) and the forgejo database are kept unless --wipe-data is given.

KUBECONFIG_PATH="/etc/rancher/k3s/k3s.yaml"
NAMESPACE="forgejo"
WIPE_DATA=false

for arg in "$@"; do
    case "${arg}" in
        --wipe-data) WIPE_DATA=true ;;
        *) echo "Usage: $0 [--wipe-data]"; exit 1 ;;
    esac
done

if [ "${WIPE_DATA}" = true ]; then
    echo "WARNING: --wipe-data deletes the PVC forgejo-data-pvc and the forgejo database. All repositories are lost."
    read -r -p "Type 'wipe' to continue: " CONFIRM
    if [ "${CONFIRM}" != "wipe" ]; then
        echo "Aborted."
        exit 1
    fi
fi

helm uninstall forgejo --namespace "${NAMESPACE}" --kubeconfig "${KUBECONFIG_PATH}" || true

if [ "${WIPE_DATA}" = true ]; then
    kubectl --kubeconfig "${KUBECONFIG_PATH}" -n "${NAMESPACE}" delete pvc forgejo-data-pvc --ignore-not-found
    PGPOD=$(kubectl --kubeconfig "${KUBECONFIG_PATH}" -n postgresql get pod -o jsonpath='{.items[0].metadata.name}')
    PGPASS=$(kubectl --kubeconfig "${KUBECONFIG_PATH}" -n postgresql get secret postgresql-credentials -o jsonpath='{.data.POSTGRES_PASSWORD}' | base64 -d)
    kubectl --kubeconfig "${KUBECONFIG_PATH}" -n postgresql exec "${PGPOD}" -- env PGPASSWORD="${PGPASS}" psql -U postgres -c "DROP DATABASE IF EXISTS forgejo;"
    kubectl --kubeconfig "${KUBECONFIG_PATH}" -n postgresql exec "${PGPOD}" -- env PGPASSWORD="${PGPASS}" psql -U postgres -c "DROP ROLE IF EXISTS forgejo;"
else
    echo "Kept PVC forgejo-data-pvc and database forgejo."
fi
