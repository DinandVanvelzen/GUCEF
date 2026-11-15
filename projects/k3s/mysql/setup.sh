#!/bin/bash
# ---------------------------------------------------------------------------
# Installs MySQL 8.4 LTS in namespace mysql (StatefulSet mysql, Service mysql).
# Idempotent: re-running keeps the data volume and the root password.
# In-cluster address: mysql.mysql.svc.cluster.local:3306
# Components create their own database and user with ./ensure-database.sh.
# ---------------------------------------------------------------------------

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
KUBECONFIG_PATH="/etc/rancher/k3s/k3s.yaml"
NAMESPACE="mysql"

# Defaults, overridable through config.env
MYSQL_PVC_SIZE="10Gi"
MYSQL_STORAGE_CLASS=""     # empty: longhorn-raid if present, else the cluster default

if [ -f "${SCRIPT_DIR}/config.env" ]; then
    set -a
    source <(sed 's/\r//' "${SCRIPT_DIR}/config.env")
    set +a
fi

KC="kubectl --kubeconfig ${KUBECONFIG_PATH}"

fail()
{
    echo "ERROR: $1" >&2
    [ -n "$2" ] && echo "       $2" >&2
    exit 1
}

# ---------------------------------------------------------------------------
# Preflight
# ---------------------------------------------------------------------------
${KC} get nodes >/dev/null 2>&1 \
    || fail "Cannot talk to the cluster with ${KUBECONFIG_PATH}." "Is k3s running, and is your user in the k3s-admin group? See ../setup.sh."
${KC} -n kube-system get deployment sealed-secrets >/dev/null 2>&1 \
    || fail "The Sealed Secrets controller is not installed." "Run ../sealed-secrets/setup.sh first."

if [ -n "${MYSQL_STORAGE_CLASS}" ]; then
    ${KC} get storageclass "${MYSQL_STORAGE_CLASS}" >/dev/null 2>&1 \
        || fail "Storage class ${MYSQL_STORAGE_CLASS} (MYSQL_STORAGE_CLASS) does not exist."
elif ${KC} get storageclass longhorn-raid >/dev/null 2>&1; then
    MYSQL_STORAGE_CLASS="longhorn-raid"
else
    MYSQL_STORAGE_CLASS=$(${KC} get storageclass -o jsonpath='{range .items[?(@.metadata.annotations.storageclass\.kubernetes\.io/is-default-class=="true")]}{.metadata.name}{"\n"}{end}' | head -n 1)
    [ -n "${MYSQL_STORAGE_CLASS}" ] || fail "No storage class found." "Set MYSQL_STORAGE_CLASS in config.env."
fi
echo "Storage class: ${MYSQL_STORAGE_CLASS}"

# ---------------------------------------------------------------------------
# Namespace, credentials, PVC
# ---------------------------------------------------------------------------
${KC} create namespace ${NAMESPACE} --dry-run=client -o yaml | ${KC} apply -f -

if [ ! -f "${SCRIPT_DIR}/sealed-secret.yaml" ]; then
    echo "sealed-secret.yaml not found - running create-sealed-secret.sh..."
    bash "${SCRIPT_DIR}/create-sealed-secret.sh" || fail "Failed to create the sealed secret."
fi
${KC} apply -f "${SCRIPT_DIR}/sealed-secret.yaml" -n ${NAMESPACE}

echo "Waiting for secret mysql-credentials..."
for _ in $(seq 1 30); do
    ${KC} -n ${NAMESPACE} get secret mysql-credentials >/dev/null 2>&1 && break
    sleep 2
done
${KC} -n ${NAMESPACE} get secret mysql-credentials >/dev/null 2>&1 || fail "Secret mysql-credentials was not created." \
    "After a cluster rebuild, delete sealed-secret.yaml and run ./create-sealed-secret.sh again."

# An existing PVC keeps its storage class and size; the class of a bound PVC cannot change.
if ${KC} -n ${NAMESPACE} get pvc mysql-pvc >/dev/null 2>&1; then
    echo "PVC mysql-pvc exists; keeping it."
else
    sed -e "s|\${MYSQL_PVC_SIZE}|${MYSQL_PVC_SIZE}|" -e "s|\${MYSQL_STORAGE_CLASS}|${MYSQL_STORAGE_CLASS}|" \
        "${SCRIPT_DIR}/mysql-pvc.yaml" | ${KC} apply -f -
fi

# ---------------------------------------------------------------------------
# Server
# ---------------------------------------------------------------------------
${KC} apply -f "${SCRIPT_DIR}/mysql-statefulset.yaml"
${KC} apply -f "${SCRIPT_DIR}/mysql-service.yaml"

${KC} -n ${NAMESPACE} rollout status statefulset/mysql --timeout=10m || fail "MySQL did not become ready." \
    "Check: ${KC} -n ${NAMESPACE} logs statefulset/mysql"

echo
echo "MySQL is running: mysql.${NAMESPACE}.svc.cluster.local:3306 (in-cluster only)"
echo "Root password: ~/.config/gucef-k3s/mysql-secrets.env (MYSQL_ROOT_PASSWORD)"
${KC} -n ${NAMESPACE} get pods,svc
