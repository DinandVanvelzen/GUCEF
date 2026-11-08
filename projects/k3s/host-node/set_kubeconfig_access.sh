#!/bin/bash
# ---------------------------------------------------------------------------
# Gives a group read access to the k3s admin kubeconfig, so its members can
# use kubectl and helm without sudo.
#
# The kubeconfig grants full cluster admin rights; it is readable by root and
# the group only (mode 0640), not by all users.
#
# - creates the group if it does not exist
# - adds the user to the group (default: the user who invoked sudo)
# - sets write-kubeconfig-mode and write-kubeconfig-group in the k3s config
#   file, so k3s keeps these whenever it rewrites the kubeconfig
# - applies the group and mode to the existing kubeconfig; no k3s restart needed
#
# Idempotent. Must run as root.
# Environment:
#   K3S_KUBECONFIG_GROUP  group name (default k3s-admin)
#   K3S_ADMIN_USER        user to add to the group (default: $SUDO_USER)
#   K3S_CONFIG_FILE       k3s config file (default /etc/rancher/k3s/config.yaml)
#   KUBECONFIG_PATH       kubeconfig (default /etc/rancher/k3s/k3s.yaml)
# ---------------------------------------------------------------------------

GROUP="${K3S_KUBECONFIG_GROUP:-k3s-admin}"
ADMIN_USER="${K3S_ADMIN_USER:-${SUDO_USER}}"
CONFIG_FILE="${K3S_CONFIG_FILE:-/etc/rancher/k3s/config.yaml}"
KUBECONFIG_PATH="${KUBECONFIG_PATH:-/etc/rancher/k3s/k3s.yaml}"
MODE="0640"

if [ "$(id -u)" -ne 0 ]; then
    echo "ERROR: must run as root (use sudo)."
    exit 1
fi

if ! getent group "${GROUP}" > /dev/null; then
    echo "Creating group ${GROUP}"
    groupadd --system "${GROUP}" || exit 1
fi

if [ -n "${ADMIN_USER}" ] && [ "${ADMIN_USER}" != "root" ]; then
    if id -nG "${ADMIN_USER}" | tr ' ' '\n' | grep -qx "${GROUP}"; then
        echo "User ${ADMIN_USER} is already in group ${GROUP}"
    else
        echo "Adding user ${ADMIN_USER} to group ${GROUP}"
        usermod -aG "${GROUP}" "${ADMIN_USER}" || exit 1
        echo "NOTE: ${ADMIN_USER} must log out and in again (or run 'newgrp ${GROUP}') for the group to take effect."
    fi
fi

# Sets key $1 to value $2 in the k3s config file
set_config_key() {
    if grep -qE "^$1:" "${CONFIG_FILE}"; then
        sed -i -E "s|^$1:.*|$1: $2|" "${CONFIG_FILE}"
    else
        echo "$1: $2" >> "${CONFIG_FILE}"
    fi
}

mkdir -p "$(dirname "${CONFIG_FILE}")"
touch "${CONFIG_FILE}"
set_config_key "write-kubeconfig-mode" "\"${MODE}\""
set_config_key "write-kubeconfig-group" "\"${GROUP}\""
echo "k3s config ${CONFIG_FILE}: write-kubeconfig-mode ${MODE}, write-kubeconfig-group ${GROUP}"

if [ -f "${KUBECONFIG_PATH}" ]; then
    chgrp "${GROUP}" "${KUBECONFIG_PATH}" && chmod "${MODE}" "${KUBECONFIG_PATH}" || exit 1
    ls -l "${KUBECONFIG_PATH}"
fi
