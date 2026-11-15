#!/bin/bash
# Mounts the NAS backup share on this node and installs the nightly sealed-secrets key export.
# Run with sudo:  sudo ./setup-nas.sh      (idempotent)
source "$(dirname "${BASH_SOURCE[0]}")/lib.sh"
[ "$(id -u)" -eq 0 ] || bk_fail "run with sudo: sudo ./setup-nas.sh"

# Under sudo HOME is root's; use the invoking user's profile for the credentials copy.
if [ -n "${SUDO_USER}" ]; then HOME=$(getent passwd "${SUDO_USER}" | cut -d: -f6); BK_COPY_DIR="${HOME}/.config/gucef-k3s"; fi
bk_load_config
bk_require_nas_host
bk_ensure_secrets
[ -n "${SUDO_USER}" ] && chown -R "${SUDO_USER}": "${BK_COPY_DIR}" "${BK_DIR}/secrets.env" 2>/dev/null

command -v mount.cifs >/dev/null || { echo "Installing cifs-utils..."; apt-get install -y cifs-utils >/dev/null || bk_fail "could not install cifs-utils"; }

CRED=/etc/k3s-backup-nas-credentials
( umask 077; printf 'username=%s\npassword=%s\n' "${BACKUP_NAS_USER}" "${BACKUP_NAS_PASSWORD}" > "${CRED}" )
chmod 600 "${CRED}"

mkdir -p "${BACKUP_MOUNT}"
OPTS="credentials=${CRED},rw,uid=0,gid=0,file_mode=0600,dir_mode=0700,iocharset=utf8,_netdev,nofail"
LINE="//${BACKUP_NAS_HOST}/${BACKUP_NAS_SHARE} ${BACKUP_MOUNT} cifs ${OPTS} 0 0"
if grep -qF " ${BACKUP_MOUNT} cifs" /etc/fstab; then
    sed -i "\# ${BACKUP_MOUNT} cifs#c\\${LINE}" /etc/fstab
    echo "fstab entry updated."
else
    echo "${LINE}" >> /etc/fstab
    echo "fstab entry added."
fi
systemctl daemon-reload 2>/dev/null

mountpoint -q "${BACKUP_MOUNT}" && umount "${BACKUP_MOUNT}"
mount "${BACKUP_MOUNT}" || bk_fail "mount failed. Check host, share name and account (dmesg | tail)."
for d in longhorn forgejo postgresql cluster-keys; do mkdir -p "${BACKUP_MOUNT}/${d}"; done
T="${BACKUP_MOUNT}/.write-test"
echo ok > "${T}" && rm -f "${T}" || bk_fail "share is mounted but not writable: the account needs read-write permission on the share and NTFS folder."
echo "Mounted //${BACKUP_NAS_HOST}/${BACKUP_NAS_SHARE} at ${BACKUP_MOUNT} (read-write verified)."

# Nightly sealed-secrets key export (host timer, no container image needed)
if command -v kubectl >/dev/null && kubectl get ns kube-system >/dev/null 2>&1; then
    cat > /etc/systemd/system/k3s-backup-keys.service <<UNIT
[Unit]
Description=Export Sealed Secrets key to the NAS
RequiresMountsFor=${BACKUP_MOUNT}

[Service]
Type=oneshot
Environment=BACKUP_MOUNT=${BACKUP_MOUNT}
Environment=PATH=/usr/local/bin:/usr/bin:/bin
ExecStart=${BK_DIR}/backup-keys.sh
UNIT
    cat > /etc/systemd/system/k3s-backup-keys.timer <<UNIT
[Unit]
Description=Nightly Sealed Secrets key export

[Timer]
OnCalendar=*-*-* 01:00:00
Persistent=true

[Install]
WantedBy=timers.target
UNIT
    systemctl daemon-reload && systemctl enable --now k3s-backup-keys.timer >/dev/null
    systemctl start k3s-backup-keys.service && echo "Sealed-secrets key exported; nightly timer k3s-backup-keys.timer enabled." \
        || echo "WARNING: key export failed (journalctl -u k3s-backup-keys)."
fi
echo "Next: ./setup.sh"
