#!/bin/bash
# Creates the repositories listed in repos.conf on Forgejo and starts a sync of the mirrors.
# Idempotent: repositories that already exist are left alone, mirrors are re-synced.
# Usage: ./sync-repos.sh [--no-sync]

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
KUBECONFIG_PATH="${KUBECONFIG_PATH:-/etc/rancher/k3s/k3s.yaml}"
KC="kubectl --kubeconfig ${KUBECONFIG_PATH}"
NAMESPACE="forgejo"
RELEASE="forgejo"
CONF="${SCRIPT_DIR}/repos.conf"
DO_SYNC=true
[ "$1" = "--no-sync" ] && DO_SYNC=false

[ -f "${CONF}" ] || { echo "ERROR: ${CONF} not found. Copy repos.conf.example to repos.conf and edit it."; exit 1; }

set -a
[ -f "${SCRIPT_DIR}/config.env" ] && source <(sed 's/\r//' "${SCRIPT_DIR}/config.env")
[ -f "${SCRIPT_DIR}/secrets.env" ] && source <(sed 's/\r//' "${SCRIPT_DIR}/secrets.env")
set +a
FORGEJO_HTTP_PORT="${FORGEJO_HTTP_PORT:-32001}"
ADMIN_USER="${FORGEJO_ADMIN_USER:-forgejo-admin}"

fj() { ${KC} -n ${NAMESPACE} exec deploy/${RELEASE} -c forgejo -- forgejo "$@"; }

${KC} -n ${NAMESPACE} get deploy ${RELEASE} >/dev/null 2>&1 || { echo "ERROR: Forgejo is not installed. Run ./setup.sh first."; exit 1; }

TOKEN=$(fj admin user generate-access-token --username "${ADMIN_USER}" --token-name "sync-$(date +%s)" --scopes all --raw 2>/dev/null | tail -n 1)
[ -n "${TOKEN}" ] || { echo "ERROR: could not generate an admin API token."; exit 1; }

LOCAL_PORT=13001
${KC} -n ${NAMESPACE} port-forward svc/${RELEASE}-http ${LOCAL_PORT}:${FORGEJO_HTTP_PORT} >/dev/null 2>&1 &
PF_PID=$!
trap 'kill ${PF_PID} 2>/dev/null' EXIT
for _ in $(seq 1 20); do
    curl -s -o /dev/null "http://127.0.0.1:${LOCAL_PORT}/api/v1/version" && break
    sleep 1
done
API="http://127.0.0.1:${LOCAL_PORT}/api/v1"
api() { curl -s -H "Authorization: token ${TOKEN}" -H "Content-Type: application/json" "$@"; }
code() { curl -s -o /dev/null -w '%{http_code}' -H "Authorization: token ${TOKEN}" -H "Content-Type: application/json" "$@"; }

FAIL=0
while read -r OWNER NAME MODE URL INTERVAL; do
    case "${OWNER}" in ''|\#*) continue ;; esac
    [ -n "${NAME}" ] && [ -n "${MODE}" ] && [ -n "${URL}" ] || { echo "SKIP malformed line: ${OWNER} ${NAME}"; FAIL=1; continue; }
    INTERVAL="${INTERVAL:-10m}"

    if [ "$(code "${API}/repos/${OWNER}/${NAME}")" = "200" ]; then
        echo "${OWNER}/${NAME}: exists"
    else
        case "${MODE}" in
            pull)   MIRROR=true ;;
            import) MIRROR=false ;;
            push|bidirectional) echo "${OWNER}/${NAME}: mode ${MODE} is not implemented yet"; FAIL=1; continue ;;
            *) echo "${OWNER}/${NAME}: unknown mode ${MODE}"; FAIL=1; continue ;;
        esac
        BODY="{\"clone_addr\":\"${URL}\",\"repo_owner\":\"${OWNER}\",\"repo_name\":\"${NAME}\",\"service\":\"git\",\"mirror\":${MIRROR},\"mirror_interval\":\"${INTERVAL}\",\"private\":true"
        [ -n "${GITHUB_TOKEN}" ] && [[ "${URL}" == https://github.com/* ]] && BODY="${BODY},\"auth_token\":\"${GITHUB_TOKEN}\""
        BODY="${BODY}}"
        C=$(code -X POST -d "${BODY}" "${API}/repos/migrate")
        if [ "${C}" = "201" ]; then
            echo "${OWNER}/${NAME}: created (${MODE} of ${URL})"
        else
            echo "${OWNER}/${NAME}: creating failed (HTTP ${C})"
            FAIL=1; continue
        fi
    fi

    if [ "${MODE}" = "pull" ] && ${DO_SYNC}; then
        C=$(code -X POST "${API}/repos/${OWNER}/${NAME}/mirror-sync")
        echo "${OWNER}/${NAME}: sync requested (HTTP ${C})"
    fi
done < <(sed 's/\r//' "${CONF}")

exit ${FAIL}
