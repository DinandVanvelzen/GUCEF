#!/bin/bash
# ---------------------------------------------------------------------------
# Raises the inotify limits for a Kubernetes node.
# Each pod and many system services use inotify instances; the Ubuntu default
# of 128 instances per user is exhausted on a node running tens of pods, after
# which services fail with "Too many open files".
#
# Writes /etc/sysctl.d/90-gucef-k3s-inotify.conf and applies it immediately.
# Never lowers an existing higher value. Idempotent. Must run as root.
# ---------------------------------------------------------------------------

SYSCTL_FILE="/etc/sysctl.d/90-gucef-k3s-inotify.conf"
TARGET_MAX_USER_INSTANCES=8192
TARGET_MAX_USER_WATCHES=1048576

if [ "$(id -u)" -ne 0 ]; then
    echo "ERROR: must run as root (use sudo)."
    exit 1
fi

CURRENT_INSTANCES="$(sysctl -n fs.inotify.max_user_instances)"
CURRENT_WATCHES="$(sysctl -n fs.inotify.max_user_watches)"
[ "${CURRENT_INSTANCES}" -gt "${TARGET_MAX_USER_INSTANCES}" ] && TARGET_MAX_USER_INSTANCES="${CURRENT_INSTANCES}"
[ "${CURRENT_WATCHES}" -gt "${TARGET_MAX_USER_WATCHES}" ] && TARGET_MAX_USER_WATCHES="${CURRENT_WATCHES}"

cat > "${SYSCTL_FILE}" <<EOF
fs.inotify.max_user_instances = ${TARGET_MAX_USER_INSTANCES}
fs.inotify.max_user_watches = ${TARGET_MAX_USER_WATCHES}
EOF
sysctl -p "${SYSCTL_FILE}"
