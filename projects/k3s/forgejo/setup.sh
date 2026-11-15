#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
KUBECONFIG_PATH="/etc/rancher/k3s/k3s.yaml"
NAMESPACE="forgejo"
PG_NAMESPACE="postgresql"
PG_POD=""                  # empty: first pod in the PostgreSQL namespace
RELEASE="forgejo"

# Defaults, overridable through config.env
FORGEJO_HOST=""            # empty: the IP of the first node is used
FORGEJO_STORAGE_CLASS=""   # empty: longhorn-raid if present, else the cluster default
FORGEJO_HTTP_PORT="32001"
FORGEJO_SSH_PORT="32002"
FORGEJO_PVC_SIZE="100Gi"
FORGEJO_ADMIN_USER="forgejo-admin"
FORGEJO_ADMIN_EMAIL="admin@localhost.localdomain"
FORGEJO_ORGS=""
FORGEJO_CHART_VERSION="17.3.2"

if [ -f "${SCRIPT_DIR}/config.env" ]; then
    set -a
    source <(sed 's/\r//' "${SCRIPT_DIR}/config.env")
    set +a
fi

KC="kubectl --kubeconfig ${KUBECONFIG_PATH}"

source "${SCRIPT_DIR}/lib.sh"
fj_preflight
fj_detect_storage_class
fj_detect_host
echo "Storage class: ${FORGEJO_STORAGE_CLASS}"
echo "Host:          ${FORGEJO_HOST}$([ "${FORGEJO_HOST_AUTODETECTED}" = "true" ] && echo ' (auto-detected; set FORGEJO_HOST in config.env to use a DNS name)')"

# ---------------------------------------------------------------------------
# Namespace and PVC
# ---------------------------------------------------------------------------
${KC} create namespace ${NAMESPACE} --dry-run=client -o yaml | ${KC} apply -f -
${KC} apply -f - <<EOT
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: forgejo-data-pvc
  namespace: ${NAMESPACE}
spec:
  accessModes:
    - ReadWriteOnce
  storageClassName: ${FORGEJO_STORAGE_CLASS}
  resources:
    requests:
      storage: ${FORGEJO_PVC_SIZE}
EOT

# ---------------------------------------------------------------------------
# Sealed secret (DB password, admin credentials)
# ---------------------------------------------------------------------------
if [ ! -f "${SCRIPT_DIR}/sealed-secret.yaml" ]; then
    echo "sealed-secret.yaml not found - running create-sealed-secret.sh..."
    bash "${SCRIPT_DIR}/create-sealed-secret.sh" || { echo "ERROR: Failed to create sealed secret. Ensure secrets.env exists."; exit 1; }
fi
${KC} apply -f "${SCRIPT_DIR}/sealed-secret.yaml" -n ${NAMESPACE}

# The sealed-secrets controller needs a moment to produce the Secret
echo "Waiting for secret forgejo-credentials..."
for _ in $(seq 1 30); do
    ${KC} -n ${NAMESPACE} get secret forgejo-credentials >/dev/null 2>&1 && break
    sleep 2
done
${KC} -n ${NAMESPACE} get secret forgejo-credentials >/dev/null 2>&1 || { echo "ERROR: secret forgejo-credentials was not created."; exit 1; }

# ---------------------------------------------------------------------------
# Database and user in the existing my-postgres
# ---------------------------------------------------------------------------
PG_SUPER_PASS=$(${KC} -n ${PG_NAMESPACE} get secret postgresql-credentials -o jsonpath='{.data.POSTGRES_PASSWORD}' | base64 -d)
DB_PASS=$(${KC} -n ${NAMESPACE} get secret forgejo-credentials -o jsonpath='{.data.DB_PASSWORD}' | base64 -d)
if [ -z "${PG_SUPER_PASS}" ] || [ -z "${DB_PASS}" ]; then
    echo "ERROR: could not read the PostgreSQL superuser password or the forgejo DB password."
    exit 1
fi

psql_exec()
{
    ${KC} -n ${PG_NAMESPACE} exec -i ${PG_POD} -- env PGPASSWORD="${PG_SUPER_PASS}" psql -U postgres -v ON_ERROR_STOP=1 -tA "$@"
}

if [ "$(psql_exec -c "SELECT 1 FROM pg_roles WHERE rolname='forgejo'")" != "1" ]; then
    echo "Creating database role forgejo..."
    # Password passed through stdin as a psql variable to avoid quoting problems
    printf "CREATE ROLE forgejo LOGIN PASSWORD :'pw';\n" | ${KC} -n ${PG_NAMESPACE} exec -i ${PG_POD} -- env PGPASSWORD="${PG_SUPER_PASS}" psql -U postgres -v ON_ERROR_STOP=1 -v "pw=${DB_PASS}" || exit 1
else
    printf "ALTER ROLE forgejo PASSWORD :'pw';\n" | ${KC} -n ${PG_NAMESPACE} exec -i ${PG_POD} -- env PGPASSWORD="${PG_SUPER_PASS}" psql -U postgres -v ON_ERROR_STOP=1 -v "pw=${DB_PASS}" || exit 1
fi
if [ "$(psql_exec -c "SELECT 1 FROM pg_database WHERE datname='forgejo'")" != "1" ]; then
    echo "Creating database forgejo..."
    psql_exec -c "CREATE DATABASE forgejo OWNER forgejo ENCODING 'UTF8' TEMPLATE template0" || exit 1
fi

# ---------------------------------------------------------------------------
# Helm release
# ---------------------------------------------------------------------------
if [ "${FORGEJO_HTTP_PORT}" = "80" ]; then
    ROOT_URL="http://${FORGEJO_HOST}/"
else
    ROOT_URL="http://${FORGEJO_HOST}:${FORGEJO_HTTP_PORT}/"
fi

helm upgrade --install ${RELEASE} oci://code.forgejo.org/forgejo-helm/forgejo \
    --version "${FORGEJO_CHART_VERSION}" \
    --namespace ${NAMESPACE} \
    --kubeconfig ${KUBECONFIG_PATH} \
    --values "${SCRIPT_DIR}/values.yaml" \
    --set service.http.port="${FORGEJO_HTTP_PORT}" \
    --set service.ssh.port="${FORGEJO_SSH_PORT}" \
    --set-string gitea.config.server.DOMAIN="${FORGEJO_HOST}" \
    --set-string gitea.config.server.SSH_DOMAIN="${FORGEJO_HOST}" \
    --set-string gitea.config.server.ROOT_URL="${ROOT_URL}" \
    --set gitea.config.server.SSH_PORT="${FORGEJO_SSH_PORT}" \
    --set-string gitea.admin.email="${FORGEJO_ADMIN_EMAIL}" \
    --wait --timeout 10m || { echo "ERROR: helm install failed."; exit 1; }

${KC} -n ${NAMESPACE} rollout status deployment/${RELEASE} --timeout=10m || exit 1

# ---------------------------------------------------------------------------
# Admin user (the chart creates it from forgejo-credentials; this covers a miss)
# ---------------------------------------------------------------------------
fj()
{
    ${KC} -n ${NAMESPACE} exec deploy/${RELEASE} -c forgejo -- forgejo "$@"
}

ADMIN_USER=$(${KC} -n ${NAMESPACE} get secret forgejo-credentials -o jsonpath='{.data.username}' | base64 -d)
ADMIN_PASS=$(${KC} -n ${NAMESPACE} get secret forgejo-credentials -o jsonpath='{.data.password}' | base64 -d)
if ! fj admin user list 2>/dev/null | awk 'NR>1 {print $2}' | grep -qx "${ADMIN_USER}"; then
    echo "Creating admin user ${ADMIN_USER}..."
    fj admin user create --admin --username "${ADMIN_USER}" --password "${ADMIN_PASS}" \
        --email "${FORGEJO_ADMIN_EMAIL}" --must-change-password=false || exit 1
fi

# ---------------------------------------------------------------------------
# Organizations (private) through the REST API
# ---------------------------------------------------------------------------
TOKEN=$(fj admin user generate-access-token --username "${ADMIN_USER}" \
    --token-name "setup-$(date +%s)" --scopes "write:organization,read:organization" --raw 2>/dev/null | tail -n 1)
if [ -z "${TOKEN}" ]; then
    echo "ERROR: could not generate an admin API token."
    exit 1
fi

LOCAL_PORT=13000
${KC} -n ${NAMESPACE} port-forward svc/${RELEASE}-http ${LOCAL_PORT}:${FORGEJO_HTTP_PORT} >/dev/null 2>&1 &
PF_PID=$!
trap 'kill ${PF_PID} 2>/dev/null' EXIT
for _ in $(seq 1 20); do
    curl -s -o /dev/null "http://127.0.0.1:${LOCAL_PORT}/api/v1/version" && break
    sleep 1
done

ORG_FAIL=0
for ORG in ${FORGEJO_ORGS}; do
    CODE=$(curl -s -o /dev/null -w '%{http_code}' -H "Authorization: token ${TOKEN}" "http://127.0.0.1:${LOCAL_PORT}/api/v1/orgs/${ORG}")
    if [ "${CODE}" = "200" ]; then
        echo "Organization ${ORG} exists."
    else
        echo "Creating organization ${ORG} (private)..."
        CODE=$(curl -s -o /dev/null -w '%{http_code}' -X POST -H "Authorization: token ${TOKEN}" -H "Content-Type: application/json" \
            -d "{\"username\":\"${ORG}\",\"visibility\":\"private\"}" "http://127.0.0.1:${LOCAL_PORT}/api/v1/orgs")
        if [ "${CODE}" != "201" ]; then
            echo "ERROR: creating organization ${ORG} returned HTTP ${CODE}"
            ORG_FAIL=1
        fi
    fi
done

echo
echo "=========================================================================="
echo " Forgejo is running"
echo "=========================================================================="
echo " Web UI:     ${ROOT_URL}"
echo " Admin user: ${ADMIN_USER}"
echo " Password:   in ~/.config/gucef-k3s/forgejo-secrets.env (FORGEJO_ADMIN_PASSWORD)"
echo "             Save it in your password manager."
echo
echo " Next steps:"
echo "  1. Open the web UI and log in."
echo "  2. Add your SSH public key: user menu > Settings > SSH / GPG Keys."
echo "     (no key yet? run: ssh-keygen -t ed25519)"
echo "  3. Create a repository, or push an existing one directly:"
echo "       git remote add forgejo ssh://git@${FORGEJO_HOST}:${FORGEJO_SSH_PORT}/<owner>/<repo>.git"
echo "       git push forgejo --all && git push forgejo --tags"
echo "     Pushing to a repository that does not exist yet creates it (private)."
echo "  4. Many local repositories at once: workstation/push-local-repos.sh"
if [ "${FORGEJO_HOST_AUTODETECTED}" = "true" ]; then
    echo
    echo " Note: the address ${FORGEJO_HOST} was auto-detected. To use a DNS name instead,"
    echo " set FORGEJO_HOST in config.env and run ./setup.sh again; browsers show a"
    echo " warning when the URL they use differs from the configured one."
fi
echo "=========================================================================="
${KC} -n ${NAMESPACE} get svc
exit ${ORG_FAIL}
