#!/bin/bash
# Rebuilds Longhorn volumes, PersistentVolumes and PVCs from the backups on the NAS (disaster recovery after a node wipe).
#
#   ./restore-volumes.sh --list                 show what can be restored
#   ./restore-volumes.sh --all                  restore every backed-up volume
#   ./restore-volumes.sh --only forgejo-data-pvc,my-pvc    restore by PVC name (or Longhorn volume name)
#   add --dry-run to print the manifests instead of applying them
#
# Prerequisites: k3s, sealed-secrets (with the restored key), Longhorn and ./setup.sh (backup target) are in place.
# Afterwards re-run the component setup scripts: their PVC manifests match the restored PVCs.
source "$(dirname "${BASH_SOURCE[0]}")/lib.sh"
bk_load_config

MODE=""; ONLY=""; DRY=false
while [ $# -gt 0 ]; do
    case "$1" in
        --list) MODE=list ;;
        --all) MODE=all ;;
        --only) MODE=only; ONLY="$2"; shift ;;
        --dry-run) DRY=true ;;
        -h|--help) sed -n '2,12p' "$0"; exit 0 ;;
        *) bk_fail "unknown option $1 (see --help)" ;;
    esac
    shift
done
[ -n "${MODE}" ] || { sed -n '2,12p' "$0"; exit 1; }

LH=longhorn-system
echo -n "Waiting for Longhorn to read the backup target"
for i in $(seq 1 60); do
    N=$(kubectl -n ${LH} get backupvolumes.longhorn.io --no-headers 2>/dev/null | wc -l)
    [ "${N}" -gt 0 ] && break
    echo -n .; sleep 5
done
echo
[ "${N}" -gt 0 ] || bk_fail "no backups found. Is the backup target available? (kubectl -n ${LH} get backuptargets)"

# One JSON line per backup volume: volume, pvc, namespace, size, class, backup url
ROWS=$(kubectl -n ${LH} get backupvolumes.longhorn.io -o json | jq -c '.items[] | {
    volume: .metadata.name,
    size: (.status.size // "0"),
    last: (.status.lastBackupName // ""),
    class: (.status.storageClassName // ""),
    ks: ((.status.labels.KubernetesStatus // "{}") | fromjson? // {})
}')

[ "${MODE}" = list ] && printf '%-42s %-14s %-28s %-10s %s\n' VOLUME NAMESPACE PVC SIZE_GB CLASS
SELECTED=0
while read -r ROW; do
    [ -n "${ROW}" ] || continue
    VOL=$(jq -r .volume <<<"${ROW}"); NS=$(jq -r '.ks.namespace // ""' <<<"${ROW}"); PVC=$(jq -r '.ks.pvcName // ""' <<<"${ROW}")
    SIZE=$(jq -r .size <<<"${ROW}"); CLASS=$(jq -r .class <<<"${ROW}"); LAST=$(jq -r .last <<<"${ROW}")
    if [ "${MODE}" = list ]; then
        printf '%-42s %-14s %-28s %-10s %s\n' "${VOL}" "${NS:--}" "${PVC:--}" "$((SIZE / 1073741824))" "${CLASS:--}"
        continue
    fi
    if [ "${MODE}" = only ]; then
        case ",${ONLY}," in *",${PVC},"*|*",${VOL},"*) ;; *) continue ;; esac
    fi
    SELECTED=$((SELECTED + 1))
    [ -n "${NS}" ] && [ -n "${PVC}" ] || { echo "SKIP ${VOL}: backup has no PVC information (restore it by hand in the Longhorn UI)"; continue; }
    [ -n "${LAST}" ] || { echo "SKIP ${VOL}: no completed backup"; continue; }
    URL=$(kubectl -n ${LH} get backups.longhorn.io "${LAST}" -o jsonpath='{.status.url}')
    [ -n "${URL}" ] || { echo "SKIP ${VOL}: backup ${LAST} has no URL"; continue; }
    [ -z "${CLASS}" ] && CLASS=$(kubectl get sc -o json | jq -r '.items[] | select(.metadata.annotations["storageclass.kubernetes.io/is-default-class"]=="true") | .metadata.name' | head -n1)

    # Placement and replica settings come from the original storage class when it exists.
    REPL=2; DISKSEL="[]"; NODESEL="[]"
    if SCJ=$(kubectl get sc "${CLASS}" -o json 2>/dev/null); then
        REPL=$(jq -r '.parameters.numberOfReplicas // "2"' <<<"${SCJ}")
        DISKSEL=$(jq -c '(.parameters.diskSelector // "") | split(",") | map(select(length>0))' <<<"${SCJ}")
        NODESEL=$(jq -c '(.parameters.nodeSelector // "") | split(",") | map(select(length>0))' <<<"${SCJ}")
    fi
    MANIFEST=$(cat <<YAML
apiVersion: longhorn.io/v1beta2
kind: Volume
metadata: {name: ${VOL}, namespace: ${LH}}
spec:
  size: "${SIZE}"
  numberOfReplicas: ${REPL}
  frontend: blockdev
  accessMode: rwo
  fromBackup: "${URL}"
  diskSelector: ${DISKSEL}
  nodeSelector: ${NODESEL}
---
apiVersion: v1
kind: Namespace
metadata: {name: ${NS}}
---
apiVersion: v1
kind: PersistentVolume
metadata: {name: ${VOL}}
spec:
  capacity: {storage: "${SIZE}"}
  volumeMode: Filesystem
  accessModes: [ReadWriteOnce]
  persistentVolumeReclaimPolicy: Retain
  storageClassName: ${CLASS}
  claimRef: {namespace: ${NS}, name: ${PVC}}
  csi:
    driver: driver.longhorn.io
    fsType: ext4
    volumeHandle: ${VOL}
    volumeAttributes: {numberOfReplicas: "${REPL}", staleReplicaTimeout: "30"}
---
apiVersion: v1
kind: PersistentVolumeClaim
metadata: {name: ${PVC}, namespace: ${NS}}
spec:
  accessModes: [ReadWriteOnce]
  storageClassName: ${CLASS}
  volumeName: ${VOL}
  resources: {requests: {storage: "${SIZE}"}}
YAML
)
    if ${DRY}; then echo "# ---- ${NS}/${PVC} (${VOL})"; echo "${MANIFEST}"; continue; fi
    if kubectl -n ${LH} get volumes.longhorn.io "${VOL}" >/dev/null 2>&1; then
        echo "SKIP ${NS}/${PVC}: Longhorn volume ${VOL} already exists"; continue
    fi
    echo "Restoring ${NS}/${PVC} from ${LAST} ..."
    echo "${MANIFEST}" | kubectl apply -f - >/dev/null || { echo "FAILED to apply ${VOL}"; continue; }
done <<<"${ROWS}"

if [ "${MODE}" != list ] && ! ${DRY}; then
    [ "${SELECTED}" -gt 0 ] || bk_fail "no backup matched."
    echo "Volumes are restoring in the background. Progress:"
    echo "  kubectl -n ${LH} get volumes.longhorn.io"
    echo "Re-run each component's setup.sh once the volumes show 'detached'."
fi
