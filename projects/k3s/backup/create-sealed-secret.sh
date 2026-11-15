#!/bin/bash
# Seals the NAS account into the Longhorn backup credential secret (longhorn-system/longhorn-backup-cifs).
source "$(dirname "${BASH_SOURCE[0]}")/lib.sh"
bk_load_config
bk_require_nas_host
bk_ensure_secrets

KUBESEAL_BIN="${KUBESEAL_BIN:-$(command -v kubeseal || echo "${HOME}/.local/bin/kubeseal")}"
[ -x "${KUBESEAL_BIN}" ] || bk_fail "kubeseal is not installed. Run ../sealed-secrets/setup.sh first."

kubectl create secret generic longhorn-backup-cifs \
    --namespace longhorn-system \
    --from-literal=CIFS_USERNAME="${BACKUP_NAS_USER}" \
    --from-literal=CIFS_PASSWORD="${BACKUP_NAS_PASSWORD}" \
    --dry-run=client -o yaml \
| "${KUBESEAL_BIN}" --kubeconfig "${KUBECONFIG_PATH}" \
    --controller-name=sealed-secrets --controller-namespace=kube-system --format yaml \
> "${BK_DIR}/sealed-secret.yaml" || bk_fail "sealing failed"
echo "Sealed secret written to ${BK_DIR}/sealed-secret.yaml (safe to commit)."
