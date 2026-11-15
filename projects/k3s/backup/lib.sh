#!/bin/bash
# Shared helpers for the backup scripts. Sourced, not executed.

BK_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BK_COPY_DIR="${HOME}/.config/gucef-k3s"
KUBECONFIG_PATH="${KUBECONFIG_PATH:-/etc/rancher/k3s/k3s.yaml}"
export KUBECONFIG="${KUBECONFIG_PATH}"

bk_fail()
{
    echo "ERROR: $*" >&2
    exit 1
}

# Loads config.env and secrets.env (carriage returns stripped) and applies defaults.
bk_load_config()
{
    set -a
    [ -f "${BK_DIR}/config.env" ] && source <(sed 's/\r//' "${BK_DIR}/config.env")
    [ -f "${BK_DIR}/secrets.env" ] && source <(sed 's/\r//' "${BK_DIR}/secrets.env")
    set +a
    BACKUP_NAS_SHARE="${BACKUP_NAS_SHARE:-k3s-backup}"
    BACKUP_MOUNT="${BACKUP_MOUNT:-/media/backup}"
    BACKUP_DAILY_CRON="${BACKUP_DAILY_CRON:-0 2 * * *}"
    BACKUP_DAILY_RETAIN="${BACKUP_DAILY_RETAIN:-14}"
    BACKUP_WEEKLY_CRON="${BACKUP_WEEKLY_CRON:-0 3 * * 0}"
    BACKUP_WEEKLY_RETAIN="${BACKUP_WEEKLY_RETAIN:-8}"
    BACKUP_PG_CRON="${BACKUP_PG_CRON:-30 1 * * *}"
    BACKUP_PG_RETAIN="${BACKUP_PG_RETAIN:-14}"
    BACKUP_FORGEJO_CRON="${BACKUP_FORGEJO_CRON:-0 4 * * *}"
    if [ -z "${BACKUP_NODE}" ]; then
        BACKUP_NODE=$(kubectl get nodes -o jsonpath='{.items[?(@.metadata.name=="'"$(hostname | tr 'A-Z' 'a-z')"'")].metadata.name}' 2>/dev/null)
        [ -z "${BACKUP_NODE}" ] && BACKUP_NODE=$(kubectl get nodes -l node-role.kubernetes.io/control-plane -o jsonpath='{.items[0].metadata.name}' 2>/dev/null)
    fi
}

bk_require_nas_host()
{
    [ -n "${BACKUP_NAS_HOST}" ] || bk_fail "BACKUP_NAS_HOST is not set. Copy config.env.example to config.env and set BACKUP_NAS_HOST (host name or IP of the NAS)."
}

# Makes sure secrets.env exists: prompts for the NAS account when it is missing, keeps a copy in the user profile.
bk_ensure_secrets()
{
    if [ ! -f "${BK_DIR}/secrets.env" ]; then
        [ -t 0 ] || bk_fail "${BK_DIR}/secrets.env not found and no terminal to ask for the NAS account. Copy secrets.env.example to secrets.env and fill it in."
        echo "Account of the '${BACKUP_NAS_SHARE}' share on ${BACKUP_NAS_HOST}:"
        read -r -p "  User name: " u
        read -r -s -p "  Password (not shown): " p
        echo
        [ -n "${u}" ] && [ -n "${p}" ] || bk_fail "user name and password are both required."
        ( umask 077; printf 'BACKUP_NAS_USER=%s\nBACKUP_NAS_PASSWORD=%s\n' "${u}" "${p}" > "${BK_DIR}/secrets.env" )
    fi
    mkdir -p "${BK_COPY_DIR}" && chmod 700 "${BK_COPY_DIR}"
    cp "${BK_DIR}/secrets.env" "${BK_COPY_DIR}/backup-secrets.env" && chmod 600 "${BK_COPY_DIR}/backup-secrets.env"
    bk_load_config
    [ -n "${BACKUP_NAS_USER}" ] && [ -n "${BACKUP_NAS_PASSWORD}" ] || bk_fail "BACKUP_NAS_USER and BACKUP_NAS_PASSWORD must both be set in secrets.env"
}
