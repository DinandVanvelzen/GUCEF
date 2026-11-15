#!/bin/bash
# Usage: push-local-repos.sh <org> <dir>...
# Pushes all branches and tags of each local git repo to Forgejo as <org>/<repo>
# and verifies that every local branch head exists on the server.
# Environment:
#   FORGEJO_HOST       required: address or DNS name of the Forgejo server
#   FORGEJO_SSH_PORT   default 32002 (SSH mode)
#   FORGEJO_HTTP_PORT  default 32001 (HTTP mode)
# Transport: SSH by default (needs your SSH key added to your Forgejo user).
# HTTP mode is used when FORGEJO_TOKEN is set (an access token with write:repository,
# created under Settings > Applications); FORGEJO_USER is the user it belongs to.
# The token is passed to git per command and is not stored in the remote URL.

if [ $# -lt 2 ]; then
    echo "Usage: $0 <org> <dir>..."
    exit 2
fi

ORG="$1"
shift
HOST="${FORGEJO_HOST}"
if [ -z "${HOST}" ]; then
    echo "ERROR: set FORGEJO_HOST to the address of your Forgejo server, for example:"
    echo "  FORGEJO_HOST=myserver.lan $0 <org> <dir>..."
    exit 2
fi
SSH_PORT="${FORGEJO_SSH_PORT:-32002}"
HTTP_PORT="${FORGEJO_HTTP_PORT:-32001}"
GITN=(git)
if [ -n "${FORGEJO_TOKEN}" ]; then
    if [ -z "${FORGEJO_USER}" ]; then
        echo "ERROR: FORGEJO_TOKEN is set but FORGEJO_USER is not (the user the token belongs to)."
        exit 2
    fi
    AUTH="Authorization: Basic $(printf '%s:%s' "${FORGEJO_USER}" "${FORGEJO_TOKEN}" | base64 -w0)"
    GITN=(git -c "http.extraHeader=${AUTH}")
fi
REMOTE="forgejo"
RESULTS=()
FAILED=0

for DIR in "$@"; do
    DIR="${DIR%/}"
    NAME="$(basename "$(cd "${DIR}" 2>/dev/null && pwd)")"
    if ! git -C "${DIR}" rev-parse --git-dir >/dev/null 2>&1; then
        RESULTS+=("FAIL ${DIR} (not a git repository)")
        FAILED=1
        continue
    fi
    if [ -n "${FORGEJO_TOKEN}" ]; then
        URL="http://${HOST}:${HTTP_PORT}/${ORG}/${NAME}.git"
    else
        URL="ssh://git@${HOST}:${SSH_PORT}/${ORG}/${NAME}.git"
    fi

    if git -C "${DIR}" remote get-url "${REMOTE}" >/dev/null 2>&1; then
        git -C "${DIR}" remote set-url "${REMOTE}" "${URL}"
    else
        git -C "${DIR}" remote add "${REMOTE}" "${URL}"
    fi

    echo "=== ${NAME} -> ${URL}"
    if ! "${GITN[@]}" -C "${DIR}" push "${REMOTE}" --all; then
        RESULTS+=("FAIL ${NAME} (push --all)")
        FAILED=1
        continue
    fi
    if ! "${GITN[@]}" -C "${DIR}" push "${REMOTE}" --tags; then
        RESULTS+=("FAIL ${NAME} (push --tags)")
        FAILED=1
        continue
    fi

    REMOTE_REFS="$("${GITN[@]}" -C "${DIR}" ls-remote --heads "${REMOTE}")"
    if [ $? -ne 0 ]; then
        RESULTS+=("FAIL ${NAME} (ls-remote)")
        FAILED=1
        continue
    fi
    MISSING=""
    while read -r SHA REF; do
        if ! grep -q "^${SHA}[[:space:]]${REF}$" <<<"${REMOTE_REFS}"; then
            MISSING="${MISSING} ${REF#refs/heads/}"
        fi
    done < <(git -C "${DIR}" for-each-ref --format='%(objectname) %(refname)' refs/heads)

    if [ -n "${MISSING}" ]; then
        RESULTS+=("FAIL ${NAME} (branch head not on server:${MISSING})")
        FAILED=1
    else
        RESULTS+=("OK   ${NAME}")
    fi
done

echo
echo "Summary:"
for R in "${RESULTS[@]}"; do
    echo "  ${R}"
done
exit ${FAILED}
