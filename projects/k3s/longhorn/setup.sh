#!/bin/bash

#install prereq
kubectl apply -f https://raw.githubusercontent.com/longhorn/longhorn/v1.7.2/deploy/prerequisite/longhorn-iscsi-installation.yaml
sudo apt-get update;sudo apt-get install -y open-iscsi cryptsetup dmsetup
kubectl apply -f https://raw.githubusercontent.com/longhorn/longhorn/v1.7.2/deploy/prerequisite/longhorn-nfs-installation.yaml

# Install longhorn
kubectl apply -f https://raw.githubusercontent.com/longhorn/longhorn/v1.7.2/deploy/longhorn.yaml

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
KUBECONFIG_PATH="/etc/rancher/k3s/k3s.yaml"

# Wait for Longhorn to come up before making it the default StorageClass.
# If Longhorn does not come up, local-path stays the default (k3s out-of-the-box behaviour).
echo "Waiting for Longhorn manager to be ready..."
LONGHORN_READY=false
if kubectl --kubeconfig ${KUBECONFIG_PATH} -n longhorn-system rollout status daemonset/longhorn-manager --timeout=600s; then
    # The 'longhorn' StorageClass is created by Longhorn itself shortly after the manager starts
    for i in $(seq 1 30); do
        if kubectl --kubeconfig ${KUBECONFIG_PATH} get storageclass longhorn > /dev/null 2>&1; then
            LONGHORN_READY=true
            break
        fi
        sleep 5
    done
fi

if [ "${LONGHORN_READY}" = "true" ]; then
    # Replicas per volume (default 1, redundancy comes from the disk setup).
    # longhorn.yaml resets this, so it runs after every apply.
    bash "${SCRIPT_DIR}/set_replica_count.sh"
    # Longhorn is now the default; local-path is kept available as a non-default class
    bash "${SCRIPT_DIR}/../local-path-provisioner/switch_to_non_default.sh"
else
    echo "WARNING: Longhorn did not become ready. Leaving 'local-path' as the default StorageClass."
fi

# Now set up the local storage in Longhorn
. ./storage_setup.sh
