#!/bin/bash
# ---------------------------------------------------------------------------
# Sets the number of Longhorn replicas per volume.
#
# Default 1: data redundancy is provided by the node's own disk setup
# (hardware/software RAID, mirrored disks), so Longhorn keeps no extra copies.
# The repo's own storage classes (longhorn-raid/-mirror/-jbod) also use 1.
#
# Changes:
#   - Longhorn setting "default-replica-count"
#   - ConfigMap "longhorn-storageclass", from which Longhorn manages the
#     "longhorn" StorageClass
#   - with --existing-volumes: volumes that want more replicas than the target.
#     On a multi-node cluster this deletes the surplus replicas.
#
# Options:
#   --check             report what would change, change nothing
#   --existing-volumes  also lower the replica count of existing volumes
#
# Must run after every (re)apply of the upstream longhorn.yaml, which resets
# the ConfigMap. Idempotent.
# Settings: config.env (see config.env.example)
# ---------------------------------------------------------------------------

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
KUBECONFIG_PATH="${KUBECONFIG_PATH:-/etc/rancher/k3s/k3s.yaml}"
KUBECTL="kubectl --kubeconfig ${KUBECONFIG_PATH}"
NAMESPACE="longhorn-system"

CHECK_ONLY=false
EXISTING_VOLUMES=false
for arg in "$@"; do
    case "${arg}" in
        --check) CHECK_ONLY=true ;;
        --existing-volumes) EXISTING_VOLUMES=true ;;
    esac
done

if [ -f "${SCRIPT_DIR}/config.env" ]; then
    # shellcheck disable=SC1091
    . "${SCRIPT_DIR}/config.env"
fi
LONGHORN_REPLICA_COUNT="${LONGHORN_REPLICA_COUNT:-1}"

if ! ${KUBECTL} get namespace "${NAMESPACE}" > /dev/null 2>&1; then
    echo "Longhorn is not installed (namespace ${NAMESPACE} not found). Nothing to do."
    exit 0
fi
echo "Target Longhorn replica count: ${LONGHORN_REPLICA_COUNT}"

# ---------------------------------------------------------------------------
# Global default
# ---------------------------------------------------------------------------
CURRENT_DEFAULT="$(${KUBECTL} -n "${NAMESPACE}" get settings.longhorn.io default-replica-count -o jsonpath='{.value}' 2>/dev/null)"
if [ "${CURRENT_DEFAULT}" = "${LONGHORN_REPLICA_COUNT}" ]; then
    echo "Setting default-replica-count is already ${LONGHORN_REPLICA_COUNT}"
elif [ "${CHECK_ONLY}" = "true" ]; then
    echo "CHECK: would change setting default-replica-count from ${CURRENT_DEFAULT:-unset} to ${LONGHORN_REPLICA_COUNT}"
else
    echo "Changing setting default-replica-count from ${CURRENT_DEFAULT:-unset} to ${LONGHORN_REPLICA_COUNT}"
    ${KUBECTL} -n "${NAMESPACE}" patch settings.longhorn.io default-replica-count \
        --type merge -p "{\"value\":\"${LONGHORN_REPLICA_COUNT}\"}" || exit 1
fi

# ---------------------------------------------------------------------------
# "longhorn" StorageClass (managed by Longhorn from ConfigMap longhorn-storageclass)
# ---------------------------------------------------------------------------
SC_YAML="$(${KUBECTL} -n "${NAMESPACE}" get configmap longhorn-storageclass -o jsonpath='{.data.storageclass\.yaml}' 2>/dev/null)"
if [ -z "${SC_YAML}" ]; then
    echo "WARNING: ConfigMap longhorn-storageclass not found; StorageClass 'longhorn' not changed."
else
    CURRENT_SC_REPLICAS="$(echo "${SC_YAML}" | grep -oE 'numberOfReplicas:[[:space:]]*"?[0-9]+' | grep -oE '[0-9]+$')"
    if [ "${CURRENT_SC_REPLICAS}" = "${LONGHORN_REPLICA_COUNT}" ]; then
        echo "StorageClass 'longhorn' configuration already uses ${LONGHORN_REPLICA_COUNT} replica(s)"
    elif [ "${CHECK_ONLY}" = "true" ]; then
        echo "CHECK: would change StorageClass 'longhorn' numberOfReplicas from ${CURRENT_SC_REPLICAS} to ${LONGHORN_REPLICA_COUNT}"
    else
        echo "Changing StorageClass 'longhorn' numberOfReplicas from ${CURRENT_SC_REPLICAS} to ${LONGHORN_REPLICA_COUNT}"
        NEW_SC_YAML="$(echo "${SC_YAML}" | sed -E "s/(numberOfReplicas:[[:space:]]*)\"?[0-9]+\"?/\1\"${LONGHORN_REPLICA_COUNT}\"/")"
        ${KUBECTL} -n "${NAMESPACE}" create configmap longhorn-storageclass \
            --from-literal=storageclass.yaml="${NEW_SC_YAML}" --dry-run=client -o yaml \
            | ${KUBECTL} apply -f - || exit 1
    fi

    if [ "${CHECK_ONLY}" != "true" ]; then
        # Longhorn recreates the StorageClass from the ConfigMap
        SC_REPLICAS=""
        for _ in $(seq 1 24); do
            SC_REPLICAS="$(${KUBECTL} get storageclass longhorn -o jsonpath='{.parameters.numberOfReplicas}' 2>/dev/null)"
            [ "${SC_REPLICAS}" = "${LONGHORN_REPLICA_COUNT}" ] && break
            sleep 5
        done
        if [ "${SC_REPLICAS}" = "${LONGHORN_REPLICA_COUNT}" ]; then
            echo "StorageClass 'longhorn' uses ${LONGHORN_REPLICA_COUNT} replica(s)"
        else
            echo "ERROR: StorageClass 'longhorn' still has numberOfReplicas=${SC_REPLICAS:-unknown}; Longhorn did not recreate it from the ConfigMap."
            exit 1
        fi
    fi
fi

# ---------------------------------------------------------------------------
# Existing volumes
# ---------------------------------------------------------------------------
VOLUMES_ABOVE_TARGET="$(${KUBECTL} -n "${NAMESPACE}" get volumes.longhorn.io \
    -o jsonpath='{range .items[*]}{.metadata.name}{" "}{.spec.numberOfReplicas}{" "}{.status.kubernetesStatus.namespace}/{.status.kubernetesStatus.pvcName}{"\n"}{end}' \
    | awk -v target="${LONGHORN_REPLICA_COUNT}" 'NF >= 2 && $2 > target')"
if [ -z "${VOLUMES_ABOVE_TARGET}" ]; then
    echo "No existing volumes use more than ${LONGHORN_REPLICA_COUNT} replica(s)"
    exit 0
fi

if [ "${EXISTING_VOLUMES}" != "true" ] || [ "${CHECK_ONLY}" = "true" ]; then
    echo "Existing volumes using more than ${LONGHORN_REPLICA_COUNT} replica(s) (volume, replicas, PVC):"
    echo "${VOLUMES_ABOVE_TARGET}" | sed 's/^/  /'
    if [ "${CHECK_ONLY}" = "true" ] && [ "${EXISTING_VOLUMES}" = "true" ]; then
        echo "CHECK: would lower these volumes to ${LONGHORN_REPLICA_COUNT} replica(s)"
    elif [ "${EXISTING_VOLUMES}" != "true" ]; then
        echo "Re-run with --existing-volumes to lower them to ${LONGHORN_REPLICA_COUNT} replica(s)."
    fi
    exit 0
fi

echo "${VOLUMES_ABOVE_TARGET}" | while read -r VOLUME REPLICAS PVC; do
    echo "Volume ${VOLUME} (${PVC}): ${REPLICAS} -> ${LONGHORN_REPLICA_COUNT} replica(s)"
    ${KUBECTL} -n "${NAMESPACE}" patch volumes.longhorn.io "${VOLUME}" \
        --type merge -p "{\"spec\":{\"numberOfReplicas\":${LONGHORN_REPLICA_COUNT}}}"
done
