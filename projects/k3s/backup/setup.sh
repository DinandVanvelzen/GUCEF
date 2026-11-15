#!/bin/bash
# Configures Longhorn backups to the NAS (backup target + recurring jobs) and the app-consistent dump CronJobs.
# Prerequisites: sealed-secrets and longhorn installed; the share created on the NAS; ./setup-nas.sh run once (CronJobs only).
source "$(dirname "${BASH_SOURCE[0]}")/lib.sh"
bk_load_config
bk_require_nas_host
kubectl get ns longhorn-system >/dev/null 2>&1 || bk_fail "Longhorn is not installed. Run ../longhorn/setup.sh first."
[ -f "${BK_DIR}/sealed-secret.yaml" ] || "${BK_DIR}/create-sealed-secret.sh" || exit 1

echo "=== Longhorn backup target ==="
kubectl apply -f "${BK_DIR}/sealed-secret.yaml" || bk_fail "could not apply the sealed secret"
for i in $(seq 1 30); do kubectl -n longhorn-system get secret longhorn-backup-cifs >/dev/null 2>&1 && break; sleep 2; done
kubectl -n longhorn-system get secret longhorn-backup-cifs >/dev/null 2>&1 || bk_fail "sealed-secrets did not produce longhorn-backup-cifs (key mismatch? delete sealed-secret.yaml and re-run)"

URL="cifs://${BACKUP_NAS_HOST}/${BACKUP_NAS_SHARE}/longhorn"
kubectl -n longhorn-system patch backuptargets.longhorn.io default --type merge \
    -p "{\"spec\":{\"backupTargetURL\":\"${URL}\",\"credentialSecret\":\"longhorn-backup-cifs\",\"pollInterval\":\"5m0s\"}}" >/dev/null \
    || bk_fail "could not set the backup target"

echo "=== Recurring jobs (group 'default': every volume without its own jobs) ==="
kubectl apply -f - <<YAML
apiVersion: longhorn.io/v1beta2
kind: RecurringJob
metadata: {name: backup-daily, namespace: longhorn-system}
spec: {name: backup-daily, groups: [default], task: backup, cron: "${BACKUP_DAILY_CRON}", retain: ${BACKUP_DAILY_RETAIN}, concurrency: 1}
---
apiVersion: longhorn.io/v1beta2
kind: RecurringJob
metadata: {name: backup-weekly, namespace: longhorn-system}
spec: {name: backup-weekly, groups: [default], task: backup, cron: "${BACKUP_WEEKLY_CRON}", retain: ${BACKUP_WEEKLY_RETAIN}, concurrency: 1}
---
apiVersion: longhorn.io/v1beta2
kind: RecurringJob
metadata: {name: snapshot-cleanup, namespace: longhorn-system}
spec: {name: snapshot-cleanup, groups: [default], task: snapshot-cleanup, cron: "0 5 * * *", retain: 0, concurrency: 1}
YAML

echo -n "Waiting for the backup target to become available"
OK=false
for i in $(seq 1 30); do
    [ "$(kubectl -n longhorn-system get backuptargets.longhorn.io default -o jsonpath='{.status.available}' 2>/dev/null)" = "true" ] && { OK=true; break; }
    echo -n .; sleep 5
done
echo
if ! ${OK}; then
    kubectl -n longhorn-system get backuptargets.longhorn.io default -o jsonpath='{.status.conditions}' ; echo
    echo "WARNING: backup target not available yet. Common causes: wrong account, share name, or cifs-utils missing on a node (apt install cifs-utils on every node)."
fi

echo "=== CronJobs (need the share mounted at ${BACKUP_MOUNT} on node ${BACKUP_NODE}) ==="
if ! mountpoint -q "${BACKUP_MOUNT}"; then
    echo "Skipped: ${BACKUP_MOUNT} is not mounted here. Run: sudo ./setup-nas.sh   then ./setup.sh again."
else
    render() { sed -e "s|@NODE@|${BACKUP_NODE}|g" -e "s|@MOUNT@|${BACKUP_MOUNT}|g" -e "s|@PG_CRON@|${BACKUP_PG_CRON}|g" \
                   -e "s|@PG_RETAIN@|${BACKUP_PG_RETAIN}|g" -e "s|@FORGEJO_CRON@|${BACKUP_FORGEJO_CRON}|g" "$1"; }
    if kubectl -n postgresql get pod my-postgres-0 >/dev/null 2>&1; then
        render "${BK_DIR}/cronjobs/pg-dumpall.yaml" | kubectl apply -f - && echo "pg-dumpall CronJob applied."
    fi
    if kubectl -n forgejo get deploy forgejo >/dev/null 2>&1; then
        render "${BK_DIR}/cronjobs/forgejo-repo-mirror.yaml" | kubectl apply -f - && echo "forgejo-repo-mirror CronJob applied."
    fi
fi

echo ""
echo "Backup target: ${URL}"
echo "Check:  kubectl -n longhorn-system get backuptargets,recurringjobs"
echo "Restore after a node wipe: ./restore-volumes.sh --list"
