#!/bin/bash
# ---------------------------------------------------------------------------
# Makes Longhorn the only default StorageClass.
#
# The "local-path" StorageClass belongs to the k3s packaged addon "local-storage".
# k3s re-applies /var/lib/rancher/k3s/server/manifests/local-storage.yaml on every
# start, which resets its default annotation. A "local-storage.yaml.skip" file
# stops k3s from re-applying it; the deployed provisioner, StorageClass and RBAC
# remain and keep working.
# Consequence: k3s upgrades do not update the bundled local-path provisioner.
#
# Idempotent. Does not restart k3s.
# ---------------------------------------------------------------------------

KUBECONFIG_PATH="${KUBECONFIG_PATH:-/etc/rancher/k3s/k3s.yaml}"
K3S_MANIFESTS_DIR="/var/lib/rancher/k3s/server/manifests"
DEFAULT_ANNOTATION="storageclass.kubernetes.io/is-default-class"
KUBECTL="kubectl --kubeconfig ${KUBECONFIG_PATH}"

# Only switch the default if Longhorn actually provides its StorageClass
if ! ${KUBECTL} get storageclass longhorn > /dev/null 2>&1; then
    echo "WARNING: StorageClass 'longhorn' not found. Leaving 'local-path' as the default StorageClass."
    exit 1
fi

# 1. Stop k3s from re-applying its packaged local-storage manifest (server nodes only)
if sudo test -d "${K3S_MANIFESTS_DIR}"; then
    if ! sudo test -f "${K3S_MANIFESTS_DIR}/local-storage.yaml.skip"; then
        echo "Creating ${K3S_MANIFESTS_DIR}/local-storage.yaml.skip so k3s stops resetting the default StorageClass"
        sudo touch "${K3S_MANIFESTS_DIR}/local-storage.yaml.skip"
    else
        echo "k3s local-storage manifest is already marked as skipped"
    fi
else
    echo "WARNING: ${K3S_MANIFESTS_DIR} not found (not a k3s server node?). The local-path default may be reset by k3s on restart."
fi

# 2. local-path is not the default
if ${KUBECTL} get storageclass local-path > /dev/null 2>&1; then
    echo "Marking StorageClass 'local-path' as non-default"
    ${KUBECTL} patch storageclass local-path \
        -p "{\"metadata\":{\"annotations\":{\"${DEFAULT_ANNOTATION}\":\"false\"}}}"
fi

# 3. longhorn is the default
echo "Marking StorageClass 'longhorn' as the default"
${KUBECTL} patch storageclass longhorn \
    -p "{\"metadata\":{\"annotations\":{\"${DEFAULT_ANNOTATION}\":\"true\"}}}"

# 4. Verify there is exactly one default
DEFAULT_CLASSES=$(${KUBECTL} get storageclass \
    -o jsonpath="{range .items[?(@.metadata.annotations.storageclass\.kubernetes\.io/is-default-class=='true')]}{.metadata.name}{'\n'}{end}")
DEFAULT_COUNT=$(echo "${DEFAULT_CLASSES}" | grep -c .)

if [ "${DEFAULT_COUNT}" -ne 1 ]; then
    echo "ERROR: expected exactly 1 default StorageClass but found ${DEFAULT_COUNT}:"
    echo "${DEFAULT_CLASSES}"
    exit 1
fi
echo "Default StorageClass is now: ${DEFAULT_CLASSES}"
