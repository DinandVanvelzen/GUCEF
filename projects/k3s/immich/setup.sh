#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
KUBECONFIG_PATH="/etc/rancher/k3s/k3s.yaml"
EXTERNAL_MEDIA_HOST_PATH="/media/external"
EXTERNAL_PVC_NAME="immich-external-pvc"

# ---------------------------------------------------------------------------
# Namespace
# ---------------------------------------------------------------------------
kubectl --kubeconfig ${KUBECONFIG_PATH} create namespace immich --dry-run=client -o yaml \
    | kubectl --kubeconfig ${KUBECONFIG_PATH} apply -f -

# ---------------------------------------------------------------------------
# PVCs
# ---------------------------------------------------------------------------

# Main library: thumbnails, metadata, ML embeddings (originals stay on NAS)
kubectl --kubeconfig ${KUBECONFIG_PATH} apply -f - <<EOF
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: immich-library-pvc
  namespace: immich
spec:
  accessModes:
    - ReadWriteOnce
  storageClassName: longhorn-raid
  resources:
    requests:
      storage: 250Gi
EOF

# ML model cache: persistent so models survive pod restarts
kubectl --kubeconfig ${KUBECONFIG_PATH} apply -f - <<EOF
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: immich-ml-cache-pvc
  namespace: immich
spec:
  accessModes:
    - ReadWriteOnce
  storageClassName: longhorn-raid
  resources:
    requests:
      storage: 10Gi
EOF

# ---------------------------------------------------------------------------
# External media library (NAS mounts)
# Creates a hostPath PV + PVC if /media/external exists on this host.
# Others without a NAS can skip this by simply not creating /media/external.
# NAS mounts under /media/external are managed by setup-home.sh (gitignored).
# ---------------------------------------------------------------------------
EXTRA_VALUES_FLAG=""
HAS_EXTERNAL_MEDIA=false
if [ -d "${EXTERNAL_MEDIA_HOST_PATH}" ]; then
    echo "Found ${EXTERNAL_MEDIA_HOST_PATH} - creating hostPath PV/PVC for external media..."
    NODE_NAME=$(kubectl --kubeconfig ${KUBECONFIG_PATH} get nodes -o jsonpath='{.items[0].metadata.name}')
    kubectl --kubeconfig ${KUBECONFIG_PATH} apply -f - <<EOF
apiVersion: v1
kind: PersistentVolume
metadata:
  name: ${EXTERNAL_PVC_NAME}
spec:
  capacity:
    storage: 1Pi
  accessModes:
    - ReadOnlyMany
  persistentVolumeReclaimPolicy: Retain
  storageClassName: ""
  hostPath:
    path: ${EXTERNAL_MEDIA_HOST_PATH}
    type: Directory
  nodeAffinity:
    required:
      nodeSelectorTerms:
        - matchExpressions:
            - key: kubernetes.io/hostname
              operator: In
              values:
                - ${NODE_NAME}
EOF
    kubectl --kubeconfig ${KUBECONFIG_PATH} apply -f - <<EOF
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: ${EXTERNAL_PVC_NAME}
  namespace: immich
spec:
  accessModes:
    - ReadOnlyMany
  storageClassName: ""
  volumeName: ${EXTERNAL_PVC_NAME}
  resources:
    requests:
      storage: 1Pi
EOF

    # Generate the values overlay that mounts the external PVC into the server pod.
    # Must be nested under server: so only the server subchart sees it;
    # otherwise valkey also processes it and errors on the unknown controller.
    cat > "${SCRIPT_DIR}/values-external.yaml" <<EOF
server:
  persistence:
    external-media:
      enabled: true
      existingClaim: ${EXTERNAL_PVC_NAME}
      advancedMounts:
        main:
          main:
            - path: /media/external
              readOnly: true
EOF
    EXTRA_VALUES_FLAG="--values ${SCRIPT_DIR}/values-external.yaml"
    HAS_EXTERNAL_MEDIA=true
    echo "External media overlay written to values-external.yaml"
else
    echo "Skipping external media PV/PVC: ${EXTERNAL_MEDIA_HOST_PATH} not found on this host."
    echo "Run setup-home.sh first to mount the NAS, then re-run this script."
fi

# ---------------------------------------------------------------------------
# Sealed secret (DB credentials)
# ---------------------------------------------------------------------------
if [ ! -f "${SCRIPT_DIR}/sealed-secret.yaml" ]; then
    echo "sealed-secret.yaml not found - running create-sealed-secret.sh..."
    bash "${SCRIPT_DIR}/create-sealed-secret.sh" \
        || { echo "ERROR: Failed to create sealed secret. Ensure secrets.env exists."; exit 1; }
fi
kubectl --kubeconfig ${KUBECONFIG_PATH} apply -f "${SCRIPT_DIR}/sealed-secret.yaml" -n immich

# ---------------------------------------------------------------------------
# Auto-generate setup-home.sh if it doesn't exist yet
# (gitignored - intended to be personalised per installation)
# ---------------------------------------------------------------------------
if [ ! -f "${SCRIPT_DIR}/setup-home.sh" ]; then
    echo "Generating example setup-home.sh (all steps commented out)..."
    cat > "${SCRIPT_DIR}/setup-home.sh" << 'EXAMPLE'
#!/bin/bash
# setup-home.sh - personalise this file for your own NAS/mount layout
# This file is gitignored. It is auto-generated as a commented-out example
# the first time setup.sh runs. Edit it to match your actual NAS details.
#
# Convention: mount your NAS share(s) directly under /media/external/ so the
# sub-directories are visible inside the Immich pod as real paths (not symlinks).
# The directory /media/external must exist before running immich/setup.sh
# so the hostPath PV/PVC is created.
#
# Example using a CIFS/SMB Windows share:
#   sudo apt-get install -y cifs-utils
#   sudo mkdir -p /media/external/my-nas
#   # Add to /etc/fstab for persistence (_netdev = mount after network is up):
#   # //192.168.1.x/sharename /media/external/my-nas cifs guest,ro,uid=1000,gid=1000,iocharset=utf8,noperm,_netdev 0 0
#   sudo mount -t cifs //192.168.1.x/sharename /media/external/my-nas -o guest,ro,uid=$(id -u),gid=$(id -g),iocharset=utf8,noperm
#
# After running this script, run immich/setup.sh to create the hostPath PV/PVC.

# NAS_IP="192.168.1.x"
# NAS_SHARE="sharename"
# MOUNT_POINT="/media/external/my-nas"
# sudo apt-get install -y cifs-utils
# sudo mkdir -p "${MOUNT_POINT}"
# echo "//${NAS_IP}/${NAS_SHARE} ${MOUNT_POINT} cifs guest,ro,uid=$(id -u),gid=$(id -g),iocharset=utf8,noperm,_netdev 0 0" | sudo tee -a /etc/fstab
# sudo mount -t cifs "//${NAS_IP}/${NAS_SHARE}" "${MOUNT_POINT}" -o "guest,ro,uid=$(id -u),gid=$(id -g),iocharset=utf8,noperm"
EXAMPLE
    chmod +x "${SCRIPT_DIR}/setup-home.sh"
    echo "Created ${SCRIPT_DIR}/setup-home.sh - edit it for your NAS layout."
fi

# ---------------------------------------------------------------------------
# Helm install / upgrade
# ---------------------------------------------------------------------------
helm repo add immich https://immich-app.github.io/immich-charts --kubeconfig ${KUBECONFIG_PATH} || true
helm repo update --kubeconfig ${KUBECONFIG_PATH}

helm upgrade --install immich immich/immich \
    --namespace immich \
    --create-namespace \
    --kubeconfig ${KUBECONFIG_PATH} \
    --values "${SCRIPT_DIR}/values.yaml" \
    ${EXTRA_VALUES_FLAG}

# ---------------------------------------------------------------------------
# Wait for server and create default admin account
# The admin-sign-up endpoint is only available when no admin exists yet,
# so this is safe to re-run (returns 400 if admin already exists).
# ---------------------------------------------------------------------------
echo ""
echo "Waiting for immich-server to be ready..."
kubectl --kubeconfig ${KUBECONFIG_PATH} rollout status deployment/immich-server -n immich --timeout=180s

IMMICH_IP=$(kubectl --kubeconfig ${KUBECONFIG_PATH} get svc immich-server -n immich \
    -o jsonpath='{.status.loadBalancer.ingress[0].ip}')
IMMICH_URL="http://${IMMICH_IP}:2283"

echo "Creating default admin account (admin@immich.local / admin)..."
HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" \
    -X POST "${IMMICH_URL}/api/auth/admin-sign-up" \
    -H "Content-Type: application/json" \
    -d '{"email":"admin@immich.local","password":"admin","name":"Admin"}')

if [ "${HTTP_CODE}" = "201" ]; then
    echo "Admin account created."
    echo "  URL:      ${IMMICH_URL}"
    echo "  Email:    admin@immich.local"
    echo "  Password: admin"
    echo "  IMPORTANT: Change the password after first login!"
elif [ "${HTTP_CODE}" = "400" ]; then
    echo "Admin account already exists, skipping."
else
    echo "Warning: admin-sign-up returned HTTP ${HTTP_CODE}."
    echo "If this is a fresh install, open ${IMMICH_URL} to create an admin account manually."
fi

# ---------------------------------------------------------------------------
# Configure system settings via API
# ---------------------------------------------------------------------------
echo ""
echo "Configuring Immich system settings..."
ADMIN_TOKEN=$(curl -s -X POST "${IMMICH_URL}/api/auth/login" \
    -H "Content-Type: application/json" \
    -d '{"email":"admin@immich.local","password":"admin"}' \
    | python3 -c "import sys,json; d=json.load(sys.stdin); print(d.get('accessToken',''))" 2>/dev/null)

if [ -z "${ADMIN_TOKEN}" ]; then
    echo "Warning: Could not obtain API token - skipping system config (password may have been changed)."
else
    # Fetch current system config and apply desired overrides:
    #   map.enabled + reverseGeocoding.enabled : privacy-respecting map with GPS lookup
    #   newVersionCheck.enabled                : notify about new releases
    #   storageTemplate.enabled = false        : no template for uploads; using external libraries only
    CURRENT_CFG=$(curl -s "${IMMICH_URL}/api/system-config" \
        -H "Authorization: Bearer ${ADMIN_TOKEN}")

    UPDATED_CFG=$(echo "${CURRENT_CFG}" | python3 -c "
import sys, json
cfg = json.load(sys.stdin)
cfg['map']['enabled'] = True
cfg['reverseGeocoding']['enabled'] = True
cfg['newVersionCheck']['enabled'] = True
cfg['storageTemplate']['enabled'] = False
cfg['ffmpeg']['transcode'] = 'disabled'  # external library only; originals stay on NAS
print(json.dumps(cfg))
")

    HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" \
        -X PUT "${IMMICH_URL}/api/system-config" \
        -H "Authorization: Bearer ${ADMIN_TOKEN}" \
        -H "Content-Type: application/json" \
        -d "${UPDATED_CFG}")
    [ "${HTTP_CODE}" = "200" ] && echo "System config applied." \
        || echo "Warning: system config PUT returned HTTP ${HTTP_CODE}."

    # Enable Google Cast in admin user preferences
    HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" \
        -X PUT "${IMMICH_URL}/api/users/me/preferences" \
        -H "Authorization: Bearer ${ADMIN_TOKEN}" \
        -H "Content-Type: application/json" \
        -d '{"cast":{"gCastEnabled":true}}')
    [ "${HTTP_CODE}" = "200" ] && echo "User preferences applied (Google Cast enabled)." \
        || echo "Warning: user preferences PUT returned HTTP ${HTTP_CODE}."

    echo ""
    echo "Note: dark theme and language are browser-side preferences."
    echo "Set them once in the UI: top-right menu -> Account Settings."

    # -----------------------------------------------------------------------
    # External Libraries (only when /media/external was found)
    # Creates libraries for Family Photos and Family Videos if they don't
    # exist yet. Idempotent: skips any library whose name is already present.
    # After creating, triggers an initial scan so assets appear immediately.
    # -----------------------------------------------------------------------
    if [ "${HAS_EXTERNAL_MEDIA}" = "true" ]; then
        echo ""
        echo "Configuring External Libraries..."

        ADMIN_ID=$(curl -s "${IMMICH_URL}/api/users/me" \
            -H "Authorization: Bearer ${ADMIN_TOKEN}" \
            | python3 -c "import sys,json; print(json.load(sys.stdin)['id'])")

        EXISTING_NAMES=$(curl -s "${IMMICH_URL}/api/libraries" \
            -H "Authorization: Bearer ${ADMIN_TOKEN}" \
            | python3 -c "import sys,json; [print(l['name']) for l in json.load(sys.stdin)]")

        create_library() {
            local NAME="$1"
            local IMPORT_PATH="$2"
            if echo "${EXISTING_NAMES}" | grep -qxF "${NAME}"; then
                echo "  Library '${NAME}' already exists, skipping."
                return
            fi
            RESPONSE=$(curl -s -X POST "${IMMICH_URL}/api/libraries" \
                -H "Authorization: Bearer ${ADMIN_TOKEN}" \
                -H "Content-Type: application/json" \
                -d "$(python3 -c "import json; print(json.dumps({'name':'${NAME}','ownerId':'${ADMIN_ID}','importPaths':['${IMPORT_PATH}'],'exclusionPatterns':[]}))")")
            LIB_ID=$(echo "${RESPONSE}" | python3 -c "import sys,json; print(json.load(sys.stdin).get('id',''))" 2>/dev/null)
            if [ -n "${LIB_ID}" ]; then
                echo "  Created library '${NAME}' (id: ${LIB_ID})"
                # Trigger initial scan
                curl -s -o /dev/null -X POST "${IMMICH_URL}/api/libraries/${LIB_ID}/scan" \
                    -H "Authorization: Bearer ${ADMIN_TOKEN}" \
                    -H "Content-Type: application/json" \
                    -d '{"refreshModifiedFiles":false}'
                echo "  Scan triggered for '${NAME}'."
            else
                echo "  Warning: failed to create library '${NAME}': ${RESPONSE}"
            fi
        }

        create_library "Family Photos" "/media/external/mirror2/-== Pics ==-"
        create_library "Family Videos" "/media/external/mirror2/-== Video ==-"
    fi
fi

echo ""
echo "Immich deployed at ${IMMICH_URL}"
kubectl --kubeconfig ${KUBECONFIG_PATH} get all -n immich
