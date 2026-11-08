#!/bin/bash
# ---------------------------------------------------------------------------
# Removes the service gucef-lablink from this machine: the service, its unit,
# the installed versions in /opt/gucef-lablink, /etc/gucef-lablink, and what it
# set up for Claude Code of the user: the MCP services it registered, the
# certificate authority in ~/.config/gucef-lab and its line in ~/.profile.
# /var/lib/gucef-lablink (the lab secret, the identity of the machine, the
# approvals of a cluster) is kept unless --purge is passed. A node that joined
# stays a node: agent-node/leave.sh takes it out of the cluster.
#
# Usage: sudo ./uninstall.sh [--purge]
# ---------------------------------------------------------------------------

STATE_DIR="/var/lib/gucef-lablink"
CONFIG_DIR="/etc/gucef-lablink"
SERVICE="gucef-lablink"
UNIT_FILE="/etc/systemd/system/${SERVICE}.service"

PURGE=false
[ "$1" = "--purge" ] && PURGE=true

if [ "$(id -u)" -ne 0 ]; then
    echo "ERROR: run with sudo."
    exit 1
fi

CLAUDE_USER="$(sed -n 's/.*--claude-user \([^ "]*\).*/\1/p' "${CONFIG_DIR}/args.env" 2>/dev/null)"
systemctl disable --now "${SERVICE}.service" > /dev/null 2>&1
rm -f "${UNIT_FILE}"
systemctl daemon-reload
rm -rf /opt/gucef-lablink "${CONFIG_DIR}"
[ "$(readlink /usr/local/bin/gucefLabLink)" = "/opt/gucef-lablink/current/bin/gucefLabLink" ] && rm -f /usr/local/bin/gucefLabLink
echo "Removed the service ${SERVICE}."

if [ -n "${CLAUDE_USER}" ] && id "${CLAUDE_USER}" > /dev/null 2>&1; then
    HOME_DIR="$(getent passwd "${CLAUDE_USER}" | cut -d: -f6)"
    CLAUDE_BIN="$(sudo -u "${CLAUDE_USER}" -H bash -lc 'command -v claude' 2>/dev/null)"
    if [ -n "${CLAUDE_BIN}" ] && [ -s "${STATE_DIR}/claude-registrations" ]; then
        while read -r name _; do
            [ -n "${name}" ] && sudo -u "${CLAUDE_USER}" -H "${CLAUDE_BIN}" mcp remove "${name}" --scope user > /dev/null 2>&1 && \
                echo "Removed ${name} from Claude Code of ${CLAUDE_USER}."
        done < "${STATE_DIR}/claude-registrations"
    fi
    rm -f "${STATE_DIR}/claude-registrations"
    if [ -f "${HOME_DIR}/.profile" ] && grep -q '# gucef-lab' "${HOME_DIR}/.profile"; then
        sed -i '/# gucef-lab/d' "${HOME_DIR}/.profile"
        echo "Removed NODE_EXTRA_CA_CERTS from ${HOME_DIR}/.profile."
    fi
    rm -rf "${HOME_DIR}/.config/gucef-lab"
fi

if [ "${PURGE}" = "true" ]; then
    rm -rf "${STATE_DIR}"
    echo "Removed ${STATE_DIR}."
else
    echo "Kept ${STATE_DIR} (lab secret, identity, approvals). --purge removes it."
fi
exit 0
