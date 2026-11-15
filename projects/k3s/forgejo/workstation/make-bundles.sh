#!/bin/bash
# Usage: make-bundles.sh <outdir> <dir>...
# Creates <outdir>/<repo>.bundle with all refs for each git repo and verifies it.

if [ $# -lt 2 ]; then
    echo "Usage: $0 <outdir> <dir>..."
    exit 2
fi

OUTDIR="$1"
shift
mkdir -p "${OUTDIR}" || exit 1
FAILED=0

for DIR in "$@"; do
    DIR="${DIR%/}"
    if ! git -C "${DIR}" rev-parse --git-dir >/dev/null 2>&1; then
        echo "FAIL ${DIR} (not a git repository)"
        FAILED=1
        continue
    fi
    NAME="$(basename "$(cd "${DIR}" && pwd)")"
    BUNDLE="$(cd "${OUTDIR}" && pwd)/${NAME}.bundle"
    if git -C "${DIR}" bundle create "${BUNDLE}" --all && git -C "${DIR}" bundle verify "${BUNDLE}"; then
        echo "OK   ${NAME} -> ${BUNDLE}"
    else
        echo "FAIL ${NAME}"
        FAILED=1
    fi
done
exit ${FAILED}
