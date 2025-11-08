#!/bin/sh
set -e

PREFIX=${PREFIX:-/usr/local}
BIN_DIR="$PREFIX/bin"
UNIT_DIR=${UNIT_DIR:-/etc/systemd/system}

# Resolve project root (k3s-discover) relative to this script
PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
# Compute repo root (two levels above the project root) and place artifacts under repo-root/common
REPO_ROOT="$(cd "${PROJECT_ROOT}/../.." && pwd)"
TARGET_DIR=${TARGET_DIR:-"${REPO_ROOT}/common/bin/k3s-discover"}

echo "Installing k3s-discover binaries to ${BIN_DIR} and service to ${UNIT_DIR}"
install -d -m 0755 "${BIN_DIR}"

# Binaries are expected at ${TARGET_DIR}/release/
if [ ! -f "${TARGET_DIR}/release/k3s-discover" ]; then
	echo "Error: expected binary not found: ${TARGET_DIR}/release/k3s-discover"
	echo "Build artifacts are placed out-of-source under <repo-root>/common/bin/k3s-discover by default."
	echo "Run 'cargo build --release' from the project directory or use 'sh deploy/cross-build-*.sh' to produce the binaries."
	exit 2
fi

if [ ! -f "${TARGET_DIR}/release/listener" ]; then
	echo "Warning: listener binary not found at ${TARGET_DIR}/release/listener. Continuing without listener installation."
else
	install -m 0755 "${TARGET_DIR}/release/listener" "${BIN_DIR}/k3s-discover-listener"
fi

install -m 0755 "${TARGET_DIR}/release/k3s-discover" "${BIN_DIR}/k3s-discover"
install -m 0755 "${PROJECT_ROOT}/deploy/k3s-discover-join.sh" "${BIN_DIR}/k3s-discover-join.sh"

install -d -m 0755 "${UNIT_DIR}"
install -m 644 "${PROJECT_ROOT}/deploy/k3s-discover.service" "${UNIT_DIR}/k3s-discover.service"

systemctl daemon-reload
systemctl enable --now k3s-discover || true

echo "Installed. Check logs with: journalctl -u k3s-discover -f"
