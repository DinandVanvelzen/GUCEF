#!/bin/sh
set -e

PREFIX=${PREFIX:-/usr/local}
BIN_DIR="$PREFIX/bin"
UNIT_DIR=${UNIT_DIR:-/etc/systemd/system}

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
# Compute repo root (two levels above the project root) and place artifacts under repo-root/common
REPO_ROOT="$(cd "${PROJECT_ROOT}/../.." && pwd)"
TARGET_DIR=${TARGET_DIR:-"${REPO_ROOT}/common/bin/k3s-discover"}

echo "Stopping k3s-discover service"
systemctl stop k3s-discover || true
systemctl disable k3s-discover || true

echo "Removing service unit"
rm -f "${UNIT_DIR}/k3s-discover.service"
systemctl daemon-reload || true

echo "Removing binaries"
rm -f "${BIN_DIR}/k3s-discover"
rm -f "${BIN_DIR}/k3s-discover-listener"
rm -f "${BIN_DIR}/k3s-discover-join.sh"

# Optionally remove build artifacts from the common bin target directory
if [ -d "${TARGET_DIR}" ]; then
	echo "Removing build artifacts at ${TARGET_DIR}"
	rm -rf "${TARGET_DIR}"
fi

echo "Uninstalled"
