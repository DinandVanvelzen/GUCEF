#!/bin/bash
# Exports the Sealed Secrets controller key(s) to <mount>/cluster-keys/. Runs as root (the share is mounted root-only).
# Without these keys every committed sealed-secret.yaml is undecryptable after a rebuild.
source "$(dirname "${BASH_SOURCE[0]}")/lib.sh"
BACKUP_MOUNT_OVERRIDE="${BACKUP_MOUNT}"
bk_load_config
[ -n "${BACKUP_MOUNT_OVERRIDE}" ] && BACKUP_MOUNT="${BACKUP_MOUNT_OVERRIDE}"
mountpoint -q "${BACKUP_MOUNT}" || bk_fail "${BACKUP_MOUNT} is not mounted. Run setup-nas.sh first."
[ "$(id -u)" -eq 0 ] || bk_fail "run as root (sudo): the backup share is root-only."

DEST="${BACKUP_MOUNT}/cluster-keys"
mkdir -p "${DEST}" && chmod 700 "${DEST}"
TMP=$(mktemp "${DEST}/.key.XXXXXX") && chmod 600 "${TMP}"
kubectl get secret -n kube-system -l sealedsecrets.bitnami.com/sealed-secrets-key -o yaml > "${TMP}" \
    || { rm -f "${TMP}"; bk_fail "could not read the sealed-secrets key from the cluster"; }
grep -q 'tls.key' "${TMP}" || { rm -f "${TMP}"; bk_fail "no sealed-secrets key found in kube-system"; }
STAMP=$(date +%Y%m%d)
mv "${TMP}" "${DEST}/sealed-secrets-key-${STAMP}.yaml"
cp "${DEST}/sealed-secrets-key-${STAMP}.yaml" "${DEST}/sealed-secrets-key-latest.yaml"
# keep the 30 newest dated copies
ls -1t "${DEST}"/sealed-secrets-key-2*.yaml 2>/dev/null | tail -n +31 | xargs -r rm -f
echo "Sealed-secrets key saved to ${DEST}/sealed-secrets-key-${STAMP}.yaml"
