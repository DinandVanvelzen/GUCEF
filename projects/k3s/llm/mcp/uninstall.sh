#!/bin/bash
# ---------------------------------------------------------------------------
# Removes the MCP service gucef-mcp from this machine: the service, its units,
# the installed versions in /opt/gucef-mcp and /etc/gucef-mcp, and its
# registration with Claude Code of the service user. /var/lib/gucef-mcp (the
# token, the build, the logs) is kept unless --purge is passed.
#
# Usage: sudo ./uninstall.sh [--purge]
# The cluster side (namespace gucef-mcp, on a server):
#   kubectl delete namespace gucef-mcp
# ---------------------------------------------------------------------------

STATE_DIR="/var/lib/gucef-mcp"
SERVICE="gucef-mcp"
UNIT_DIR="/etc/systemd/system"

PURGE=false
[ "$1" = "--purge" ] && PURGE=true

if [ "$(id -u)" -ne 0 ]; then
    echo "ERROR: run with sudo."
    exit 1
fi

SERVICE_USER="$(sed -n 's/^User=//p' "${UNIT_DIR}/${SERVICE}.service" 2>/dev/null)"
systemctl disable --now "${SERVICE}-tls.path" "${SERVICE}.service" > /dev/null 2>&1
rm -f "${UNIT_DIR}/${SERVICE}.service" "${UNIT_DIR}/${SERVICE}-tls.path" "${UNIT_DIR}/${SERVICE}-tls.service"
systemctl daemon-reload
rm -rf /opt/gucef-mcp /etc/gucef-mcp
echo "Removed the service ${SERVICE}."

if [ -n "${SERVICE_USER}" ]; then
    CLAUDE_BIN="$(sudo -u "${SERVICE_USER}" -H bash -lc 'command -v claude' 2>/dev/null)"
    if [ -n "${CLAUDE_BIN}" ] && sudo -u "${SERVICE_USER}" -H "${CLAUDE_BIN}" mcp get local-llm 2>/dev/null | grep -q '127.0.0.1'; then
        sudo -u "${SERVICE_USER}" -H "${CLAUDE_BIN}" mcp remove local-llm --scope user > /dev/null 2>&1 && \
            echo "Removed local-llm from Claude Code of ${SERVICE_USER}."
    fi
    # the claude command install.sh made for a user whose Claude Code is an editor extension
    CLAUDE_WRAPPER="$(getent passwd "${SERVICE_USER}" | cut -d: -f6)/.local/bin/claude"
    if [ -f "${CLAUDE_WRAPPER}" ] && grep -q '^# gucef-mcp: claude command' "${CLAUDE_WRAPPER}"; then
        rm -f "${CLAUDE_WRAPPER}"
        echo "Removed ${CLAUDE_WRAPPER}."
    fi
fi

if [ "${PURGE}" = "true" ]; then
    rm -rf "${STATE_DIR}"
    echo "Removed ${STATE_DIR}."
else
    echo "Kept ${STATE_DIR} (token, build, logs). --purge removes it."
fi
