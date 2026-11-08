#!/bin/bash
# ---------------------------------------------------------------------------
# Removes what install.sh of this medium put on this machine:
#
#   - the k3s agent, when the machine joined the cluster as a node
#     (agent-node/leave.sh: its pods stop, k3s is uninstalled; the cluster sees
#     the node as NotReady)
#   - the services gucef-lablink and gucef-mcp, their registrations with
#     Claude Code and the line in ~/.profile (removing the packages runs their
#     uninstall scripts)
#   - the packages gucef-lablink, gucef-mcp and gucef-k3s-scripts
#   - the firewall rules of the lab (ufw, firewalld)
#
# The lab secret and identity (/var/lib/gucef-lablink) and the state of the MCP
# service (/var/lib/gucef-mcp) stay, so installing again links without the
# secret; --purge removes them too, and the settings in /etc.
#
# Usage: sudo bash uninstall.sh [--purge] [--yes]
#   --purge  also the lab secret, identity, state and settings
#   --yes    do not ask
# ---------------------------------------------------------------------------

SCRIPTS_DIR="/usr/share/gucef-k3s"
PACKAGES=(gucef-lablink gucef-mcp gucef-k3s-scripts)

PURGE=false
ASSUME_YES=false
while [ $# -gt 0 ]; do
    case "$1" in
        --purge) PURGE=true ;;
        --yes) ASSUME_YES=true ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
    shift
done

if [ "$(id -u)" -ne 0 ]; then
    echo "ERROR: run with sudo: sudo bash $0"
    exit 1
fi
if [ -f /etc/systemd/system/k3s.service ]; then
    echo "ERROR: this machine is a k3s server node; this medium does not remove a cluster."
    exit 1
fi

if [ "${ASSUME_YES}" != "true" ]; then
    WHAT="the lab services and packages"
    [ -x /usr/local/bin/k3s-agent-uninstall.sh ] && WHAT="the k3s agent (this machine leaves the cluster), ${WHAT}"
    [ "${PURGE}" = "true" ] && WHAT="${WHAT}, the lab secret and all state"
    ANSWER="n"
    [ -t 0 ] && read -r -p "Remove ${WHAT}? [y/N] " ANSWER
    [[ "${ANSWER}" =~ ^[Yy] ]] || { echo "Nothing removed."; exit 1; }
fi

# the node first, while its scripts are there
if [ -x /usr/local/bin/k3s-agent-uninstall.sh ]; then
    if [ -x "${SCRIPTS_DIR}/agent-node/leave.sh" ]; then
        "${SCRIPTS_DIR}/agent-node/leave.sh" --yes || echo "WARNING: the k3s agent was not removed, see above."
    else
        /usr/local/bin/k3s-agent-uninstall.sh || echo "WARNING: the k3s agent was not removed, see above."
        rm -f /etc/rancher/k3s/config.yaml.d/50-gucef-agent.yaml /etc/rancher/k3s/gucef-node-joined
    fi
fi

# with --purge the services take their state along; without it the package removal runs the plain uninstall
if [ "${PURGE}" = "true" ]; then
    [ -x "${SCRIPTS_DIR}/lablink/uninstall.sh" ] && "${SCRIPTS_DIR}/lablink/uninstall.sh" --purge
    [ -x "${SCRIPTS_DIR}/llm/mcp/uninstall.sh" ] && "${SCRIPTS_DIR}/llm/mcp/uninstall.sh" --purge
fi
[ -x "${SCRIPTS_DIR}/host-node/open_firewall_ports.sh" ] && "${SCRIPTS_DIR}/host-node/open_firewall_ports.sh" close

INSTALLED=()
for package in "${PACKAGES[@]}"; do
    if command -v dpkg-query > /dev/null 2>&1; then
        dpkg-query -W -f='${Status}' "${package}" 2>/dev/null | grep -q "install ok installed" && INSTALLED+=("${package}")
    elif command -v rpm > /dev/null 2>&1; then
        rpm -q "${package}" > /dev/null 2>&1 && INSTALLED+=("${package}")
    fi
done
if [ ${#INSTALLED[@]} -gt 0 ]; then
    echo "Removing ${INSTALLED[*]}"
    if command -v apt-get > /dev/null 2>&1; then
        if [ "${PURGE}" = "true" ]; then
            DEBIAN_FRONTEND=noninteractive apt-get purge -y -q "${INSTALLED[@]}"
        else
            DEBIAN_FRONTEND=noninteractive apt-get remove -y -q "${INSTALLED[@]}"
        fi
    elif command -v dnf > /dev/null 2>&1; then
        dnf remove -y "${INSTALLED[@]}"
    elif command -v zypper > /dev/null 2>&1; then
        zypper --non-interactive remove "${INSTALLED[@]}"
    elif command -v yum > /dev/null 2>&1; then
        yum remove -y "${INSTALLED[@]}"
    fi || { echo "ERROR: removing the packages failed, see above."; exit 1; }
fi
[ "${PURGE}" = "true" ] && rm -rf /etc/gucef-lablink /etc/gucef-mcp /var/lib/gucef-lablink /var/lib/gucef-mcp /var/cache/gucef-lablink

echo ""
echo "The lab services are removed from this machine."
[ "${PURGE}" != "true" ] && echo "The lab secret and identity stay in /var/lib/gucef-lablink: installing again links this machine without the secret."
exit 0
