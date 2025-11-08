#!/bin/sh
set -e

# Remove the in-tree Cargo 'target' directory if present to keep source tree clean.
PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LOCAL_TARGET="${PROJECT_ROOT}/target"

if [ -d "${LOCAL_TARGET}" ]; then
  echo "Removing local in-tree cargo target directory: ${LOCAL_TARGET}"
  rm -rf "${LOCAL_TARGET}"
else
  echo "No local in-tree target directory found at ${LOCAL_TARGET}"
fi

echo "Note: build artifacts will be placed under <repo-root>/common/bin/k3s-discover per .cargo/config.toml"
