#!/bin/sh
set -e

echo "Cross-build aarch64 (aarch64-unknown-linux-gnu) using 'cross' if available, otherwise try cargo with target"

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
# repo root is two levels above the project root
REPO_ROOT="$(cd "${PROJECT_ROOT}/../.." && pwd)"
TARGET_DIR=${TARGET_DIR:-"${REPO_ROOT}/common/bin/k3s-discover"}
export CARGO_TARGET_DIR="$TARGET_DIR"

if command -v cross >/dev/null 2>&1; then
  echo "Using cross (CARGO_TARGET_DIR=${CARGO_TARGET_DIR})"
  cross build --release --target aarch64-unknown-linux-gnu
else
  echo "'cross' not found. Ensure you have a suitable linker (e.g., aarch64-linux-gnu-gcc) and run:"
  echo "  rustup target add aarch64-unknown-linux-gnu"
  echo "Then: CARGO_TARGET_DIR=${CARGO_TARGET_DIR} cargo build --release --target aarch64-unknown-linux-gnu"
  CARGO_TARGET_DIR=${CARGO_TARGET_DIR} cargo build --release --target aarch64-unknown-linux-gnu
fi

echo "Build artifacts under ${CARGO_TARGET_DIR}/release/"
