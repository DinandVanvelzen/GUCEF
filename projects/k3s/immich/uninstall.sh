#!/bin/bash

KUBECONFIG_PATH="/etc/rancher/k3s/k3s.yaml"
WIPE_DATA=false

for arg in "$@"; do
    case $arg in
        --wipe-data) WIPE_DATA=true ;;
    esac
done

helm --kubeconfig ${KUBECONFIG_PATH} ls --namespace immich
helm delete --namespace immich --kubeconfig ${KUBECONFIG_PATH} immich || true

# Always delete the ML cache PVC - it only contains downloaded model files
# which will be re-fetched on next startup
kubectl --kubeconfig ${KUBECONFIG_PATH} delete pvc immich-ml-cache-pvc -n immich --ignore-not-found

# Delete external media PVC/PV - safe, no data lives here (hostPath, read-only)
kubectl --kubeconfig ${KUBECONFIG_PATH} delete pvc immich-external-pvc -n immich --ignore-not-found
kubectl --kubeconfig ${KUBECONFIG_PATH} delete pv  immich-external-pvc --ignore-not-found

# Delete the sealed secret (will be re-applied on next setup.sh run)
kubectl --kubeconfig ${KUBECONFIG_PATH} delete sealedsecret immich-credentials -n immich --ignore-not-found

if [ "${WIPE_DATA}" = true ]; then
    echo "WARNING: --wipe-data specified. Deleting immich-library-pvc."
    echo "This will permanently delete all thumbnails, metadata, and ML embeddings."
    read -p "Are you sure? (yes/no): " confirm
    if [ "${confirm}" = "yes" ]; then
        kubectl --kubeconfig ${KUBECONFIG_PATH} delete pvc immich-library-pvc -n immich --ignore-not-found
        echo "Library PVC deleted."
    else
        echo "Aborted. Library PVC preserved."
    fi
else
    echo ""
    echo "Library PVC (immich-library-pvc) preserved."
    echo "Run with --wipe-data to also delete it (destructive - loses all thumbnails/metadata)."
fi
