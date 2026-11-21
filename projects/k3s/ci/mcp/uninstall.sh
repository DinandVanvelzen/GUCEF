#!/bin/bash
# ---------------------------------------------------------------------------
# Removes gucefMcpCi from this machine: the service gucef-ci-gateway and its
# units, the installed versions in /opt/gucef-mcp-ci, the build in
# /var/cache/gucef-mcp-ci, and the registration with Claude Code of the user
# install.sh registered it for. Kept unless --purge is passed: the CI registry
# /etc/gucef-ci/ci.json, the state of the gateway in /var/lib/gucef-ci (the
# Forgejo token, the token hashes of the hub, the bare repositories), the
# system user gucef-ci and the CI settings install.sh set for the lablink hub.
#
# Usage: sudo ./uninstall.sh [--purge]
#   --purge  also removes /etc/gucef-ci and /var/lib/gucef-ci, the system user,
#            and the CI settings of the lablink hub that install.sh set (marked
#            "# set by ci/mcp/install.sh"); the hub then serves no CI
#            configuration. The Forgejo token stays valid on Forgejo until it is
#            deleted there (scratch-repo.sh --gateway-token FILE --rotate makes a
#            new one and deletes the earlier ones).
# ---------------------------------------------------------------------------

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
K3S_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"
GUCEF_ROOT="$(cd "${K3S_DIR}/../.." && pwd)"
INSTALL_ROOT="/opt/gucef-mcp-ci"
CACHE_DIR="/var/cache/gucef-mcp-ci"
CONFIG_DIR="/etc/gucef-ci"
STATE_DIR="/var/lib/gucef-ci"
UNIT_DIR="/etc/systemd/system"
LABLINK_CONFIG_DIR="/etc/gucef-lablink"
SERVICE="gucef-ci-gateway"
LABLINK_SERVICE="gucef-lablink"
SETTING_MARK="# set by ci/mcp/install.sh"
INSTALL_RECORD="${CONFIG_DIR}/install.env"

PURGE=false
case "${1:-}" in
    "") ;;
    --purge) PURGE=true ;;
    *) echo "Usage: sudo $0 [--purge]"; exit 1 ;;
esac

if [ "$(id -u)" -ne 0 ]; then
    echo "ERROR: run with sudo."
    exit 1
fi

CI_INSTALLED_USER=""
CI_INSTALLED_NAME="ci"
CI_INSTALLED_GATEWAY_USER="gucef-ci"
# shellcheck disable=SC1090
[ -f "${INSTALL_RECORD}" ] && . "${INSTALL_RECORD}"

# ---------------------------------------------------------------------------
# The service and its units
# ---------------------------------------------------------------------------
UNITS=("${UNIT_DIR}/${SERVICE}.service" "${UNIT_DIR}/${SERVICE}-tls.path" "${UNIT_DIR}/${SERVICE}-tls.service")
HAD_UNITS=false
for unit in "${UNITS[@]}"; do
    [ -e "${unit}" ] && HAD_UNITS=true
done
if [ "${HAD_UNITS}" = "true" ]; then
    systemctl disable --now "${SERVICE}-tls.path" "${SERVICE}.service" > /dev/null 2>&1
    systemctl stop "${SERVICE}-tls.service" > /dev/null 2>&1
    rm -f "${UNITS[@]}"
    systemctl daemon-reload
    echo "Removed the service ${SERVICE}."
fi

# ---------------------------------------------------------------------------
# The registration with Claude Code: only when it is the program of this installation
# ---------------------------------------------------------------------------
if [ -n "${CI_INSTALLED_USER}" ] && id "${CI_INSTALLED_USER}" > /dev/null 2>&1; then
    CLAUDE_HOME="$(getent passwd "${CI_INSTALLED_USER}" | cut -d: -f6)"
    CLAUDE_BIN="$(sudo -u "${CI_INSTALLED_USER}" -H bash -lc 'command -v claude' 2>/dev/null)"
    if [ -n "${CLAUDE_BIN}" ] && python3 -I - "${CLAUDE_HOME}/.claude.json" "${CI_INSTALLED_NAME}" "${INSTALL_ROOT}/" <<'PYEOF' 2>/dev/null; then
import json, sys
path, name, prefix = sys.argv[1:4]
entry = json.load(open(path)).get("mcpServers", {}).get(name)
sys.exit(0 if isinstance(entry, dict) and str(entry.get("command", "")).startswith(prefix) else 1)
PYEOF
        sudo -u "${CI_INSTALLED_USER}" -H "${CLAUDE_BIN}" mcp remove "${CI_INSTALLED_NAME}" --scope user > /dev/null 2>&1 && \
            echo "Removed ${CI_INSTALLED_NAME} from Claude Code of ${CI_INSTALLED_USER}."
    fi
    # the claude command wrapper (# gucef-mcp: claude command) stays: llm/mcp/install.sh shares it and its uninstall.sh removes it
fi

rm -rf "${INSTALL_ROOT}" "${CACHE_DIR}"
# install.env stays until --purge: it names the system user --purge removes
rm -f "${CONFIG_DIR}/gateway.env"
echo "Removed ${INSTALL_ROOT} and ${CACHE_DIR}."

if [ "${PURGE}" != "true" ]; then
    [ -d "${CONFIG_DIR}" ] && echo "Kept ${CONFIG_DIR} (the CI registry ci.json, install.env). --purge removes it."
    [ -d "${STATE_DIR}" ] && echo "Kept ${STATE_DIR} (Forgejo token, token hashes of the hub, bare repositories). --purge removes it."
    id "${CI_INSTALLED_GATEWAY_USER}" > /dev/null 2>&1 && echo "Kept the system user ${CI_INSTALLED_GATEWAY_USER}. --purge removes it."
    if [ "${HAD_UNITS}" = "true" ]; then
        echo "The lablink hub keeps announcing the gateway; run install.sh --role gateway again, or uninstall.sh --purge."
        echo "The firewall rule for 8792/tcp stays; host-node/open_firewall_ports.sh close removes all rules of the lab."
    fi
    exit 0
fi

# ---------------------------------------------------------------------------
# --purge: state, configuration, system user, the CI settings of the lablink hub
# ---------------------------------------------------------------------------
rm -rf "${CONFIG_DIR}" "${STATE_DIR}"
echo "Removed ${CONFIG_DIR} and ${STATE_DIR}."

GATEWAY_USER="${CI_INSTALLED_GATEWAY_USER}"
if [ -n "${GATEWAY_USER}" ] && [ "${GATEWAY_USER}" != "root" ] && id "${GATEWAY_USER}" > /dev/null 2>&1; then
    # only the system user install.sh made: its home is the state directory
    if [ "$(getent passwd "${GATEWAY_USER}" | cut -d: -f6)" = "${STATE_DIR}" ]; then
        userdel "${GATEWAY_USER}" && echo "Removed the system user ${GATEWAY_USER}."
    else
        echo "Kept the user ${GATEWAY_USER}: install.sh did not make it."
    fi
fi

# the settings install.sh marked, in the settings file lablink/install.sh reads
if [ -d "${GUCEF_ROOT}/projects/CMake/targets/GUCEF_exe_gucefLabLink_TestApp" ]; then
    LABLINK_SETTINGS="${K3S_DIR}/lablink/config.env"
else
    LABLINK_SETTINGS="${LABLINK_CONFIG_DIR}/settings.env"
fi
if [ -f "${LABLINK_SETTINGS}" ] && grep -qF -- "${SETTING_MARK}" "${LABLINK_SETTINGS}"; then
    python3 -I - "${LABLINK_SETTINGS}" "${SETTING_MARK}" <<'PYEOF' && echo "Removed the CI settings of the lablink hub from ${LABLINK_SETTINGS}."
import sys
path, mark = sys.argv[1:3]
with open(path, "r", newline="") as f:
    lines = f.read().splitlines(True)
with open(path, "w", newline="") as f:
    f.write("".join(l for l in lines if not l.rstrip("\r\n").endswith(mark)))
PYEOF
fi
# the CI options of the running hub, again as lablink/install.sh writes them from the settings that are left: they are
# the last options of the cluster role
lablink_setting()
{
    local name="$1"
    [ -f "${LABLINK_SETTINGS}" ] || return 0
    # without carriage returns: a settings file edited on Windows keeps working
    ( set +u; . <(sed 's/\r$//' "${LABLINK_SETTINGS}") > /dev/null 2>&1; printf '%s' "${!name:-}" )
}
ARGS_FILE="${LABLINK_CONFIG_DIR}/args.env"
if [ -f "${ARGS_FILE}" ] && grep -q -- '--role cluster' "${ARGS_FILE}"; then
    ARGS_LINE="$(cat "${ARGS_FILE}")"
    ARGS_TEXT="${ARGS_LINE#LABLINK_ARGS=\"}"
    ARGS_TEXT="${ARGS_TEXT%\"}"
    read -r -a ARGS_WORDS <<< "${ARGS_TEXT}"
    NEW_WORDS=()
    SKIP_VALUE=false
    for word in "${ARGS_WORDS[@]}"; do
        if [ "${SKIP_VALUE}" = "true" ]; then SKIP_VALUE=false; continue; fi
        case "${word}" in
            --ci-registry|--ci-gateway-url|--ci-gateway-tokens|--ci-gateway-user) SKIP_VALUE=true; continue ;;
        esac
        NEW_WORDS+=("${word}")
    done
    for pair in LAB_CI_REGISTRY:--ci-registry LAB_CI_GATEWAY_URL:--ci-gateway-url LAB_CI_GATEWAY_TOKENS:--ci-gateway-tokens \
                LAB_CI_GATEWAY_USER:--ci-gateway-user; do
        value="$(lablink_setting "${pair%%:*}")"
        [ -n "${value}" ] && NEW_WORDS+=("${pair#*:}" "${value}")
    done
    NEW_ARGS_LINE="LABLINK_ARGS=\"${NEW_WORDS[*]}\""
    if [ "${NEW_ARGS_LINE}" != "${ARGS_LINE}" ]; then
        printf '%s\n' "${NEW_ARGS_LINE}" > "${ARGS_FILE}"
        systemctl try-restart "${LABLINK_SERVICE}.service"
        echo "Restarted ${LABLINK_SERVICE} without the CI settings of the gateway."
    fi
fi
echo "The Forgejo token of the gateway stays valid on Forgejo until it is deleted there (tokens ci-gateway-* of ci-bot)."
exit 0
