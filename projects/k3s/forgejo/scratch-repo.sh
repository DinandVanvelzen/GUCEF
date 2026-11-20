#!/bin/bash
# Sets up the writable CI scratch repository on Forgejo, or prunes old scratch branches from it.
# Idempotent: what exists is left alone.
#
# Usage:
#   ./scratch-repo.sh [--dry-run] [--seed <git url or local repository>] [--report-token] [--gateway-token FILE [--rotate]]
#   ./scratch-repo.sh [--dry-run] --prune [DAYS]
#
# Setup creates, when missing:
#   - the bot user FORGEJO_CI_BOT_USER (default ci-bot). Its random password is stored as FORGEJO_CI_BOT_PASSWORD in
#     secrets.env and in ~/.config/gucef-k3s/forgejo-secrets.env, the same places as the other Forgejo passwords.
#   - the owner FORGEJO_CI_OWNER (default ci) as a private organization, unless a user or organization of that name exists.
#   - the private repository FORGEJO_CI_OWNER/FORGEJO_CI_REPO (default ci/GUCEF), a normal writable repository next to
#     the pull mirror of the published repository.
#   - Actions enabled on it, and the bot user as collaborator with write access.
# --seed <source>   pushes all branches and tags of the source (a git URL, or a local repository) into the scratch
#                   repository, once: only while it is empty. Actions are off during the push, so the push to the
#                   default branch starts no CI run.
# --report-token    gives the bot user a new access token for the package registry (scope write:package, older
#                   tokens of this script are deleted) and stores it as the Actions secret CI_REPORT_TOKEN of the
#                   scratch repository: the matrix cells upload their reports with it to the packages of the bot user.
# --gateway-token FILE  gives the bot user an access token for the CI gateway (gucefMcpCi --mode gateway) and writes it
#                   to FILE, mode 0600, in a directory that exists. Scopes: write:repository (push and delete scratch
#                   branches, dispatch and cancel runs), read:package and write:package (read and delete the report
#                   packages of the bot user). Nothing is done when FILE exists and is not empty.
# --rotate          with --gateway-token: also when FILE exists, a new token is made and written, and then the earlier
#                   tokens of this script (named ci-gateway-*) are deleted. Without it earlier ones stay valid.
# --prune [DAYS]    deletes branches scratch/* whose head commit is older than DAYS days (default FORGEJO_CI_PRUNE_DAYS,
#                   else 14).
# --dry-run         prints the planned API calls and git commands and changes nothing: no cluster, no token, no network.
#
# Settings (config.env): FORGEJO_CI_OWNER, FORGEJO_CI_REPO, FORGEJO_CI_BOT_USER, FORGEJO_CI_BOT_EMAIL,
# FORGEJO_CI_PRUNE_DAYS, FORGEJO_HTTP_PORT, FORGEJO_ADMIN_USER. Needs kubectl access to the cluster, curl, git, python3.
# Passwords and tokens are never printed and never passed on a command line.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# Request bodies are piped into req, which must set CODE and BODY in this shell: run the last command of a
# pipeline in the current shell (pipes, so a body with a password never lands in a temporary file)
shopt -s lastpipe
KUBECONFIG_PATH="${KUBECONFIG_PATH:-/etc/rancher/k3s/k3s.yaml}"
KC="kubectl --kubeconfig ${KUBECONFIG_PATH}"
NAMESPACE="forgejo"
RELEASE="forgejo"
SECRETS_FILE="${SCRIPT_DIR}/secrets.env"
SECRETS_COPY="${HOME}/.config/gucef-k3s/forgejo-secrets.env"

DRY_RUN=false
SEED=""
REPORT_TOKEN=false
GATEWAY_TOKEN_FILE=""
ROTATE=false
PRUNE=false
PRUNE_DAYS=""
usage()
{
    echo "Usage: $0 [--dry-run] [--seed <git url or local repository>] [--report-token] [--gateway-token FILE [--rotate]]"
    echo "       $0 [--dry-run] --prune [DAYS]"
    exit 2
}
while [ $# -gt 0 ]; do
    case "$1" in
        --dry-run) DRY_RUN=true ;;
        --seed) [ -n "$2" ] || usage; SEED="$2"; shift ;;
        --report-token) REPORT_TOKEN=true ;;
        --gateway-token) [ -n "$2" ] || usage; GATEWAY_TOKEN_FILE="$2"; shift ;;
        --rotate) ROTATE=true ;;
        --prune)
            PRUNE=true
            if [[ "$2" =~ ^[0-9]+$ ]]; then PRUNE_DAYS="$2"; shift; fi
            ;;
        -h|--help) usage ;;
        *) echo "Unknown argument: $1"; usage ;;
    esac
    shift
done
if ${PRUNE} && { [ -n "${SEED}" ] || ${REPORT_TOKEN} || [ -n "${GATEWAY_TOKEN_FILE}" ]; }; then
    echo "--prune cannot be combined with --seed, --report-token or --gateway-token"
    usage
fi
if ${ROTATE} && [ -z "${GATEWAY_TOKEN_FILE}" ]; then
    echo "--rotate needs --gateway-token FILE"
    usage
fi
if [ -n "${GATEWAY_TOKEN_FILE}" ] && [ ! -d "$(dirname "${GATEWAY_TOKEN_FILE}")" ]; then
    echo "The directory of ${GATEWAY_TOKEN_FILE} does not exist."
    exit 1
fi

set -a
[ -f "${SCRIPT_DIR}/config.env" ] && source <(sed 's/\r//' "${SCRIPT_DIR}/config.env")
[ -f "${SECRETS_FILE}" ] && source <(sed 's/\r//' "${SECRETS_FILE}")
set +a
# Without secrets.env the bot password lives in the user-profile copy only
if [ -z "${FORGEJO_CI_BOT_PASSWORD:-}" ] && [ -f "${SECRETS_COPY}" ]; then
    FORGEJO_CI_BOT_PASSWORD=$(sed -n 's/\r$//; s/^FORGEJO_CI_BOT_PASSWORD=//p' "${SECRETS_COPY}" | tail -n 1)
fi
FORGEJO_HTTP_PORT="${FORGEJO_HTTP_PORT:-32001}"
ADMIN_USER="${FORGEJO_ADMIN_USER:-forgejo-admin}"
CI_OWNER="${FORGEJO_CI_OWNER:-ci}"
CI_REPO="${FORGEJO_CI_REPO:-GUCEF}"
BOT_USER="${FORGEJO_CI_BOT_USER:-ci-bot}"
BOT_EMAIL="${FORGEJO_CI_BOT_EMAIL:-${BOT_USER}@localhost.localdomain}"
PRUNE_DAYS="${PRUNE_DAYS:-${FORGEJO_CI_PRUNE_DAYS:-14}}"
REPO_PATH="${CI_OWNER}/${CI_REPO}"

source "${SCRIPT_DIR}/lib.sh"

for tool in curl git python3; do
    command -v "${tool}" >/dev/null 2>&1 || fj_fail "${tool} is not installed."
done

# Private working directory: header files with credentials, the seed clone. Removed on exit.
WORK="$(mktemp -d "${TMPDIR:-/var/tmp}/scratch-repo.XXXXXX")" || fj_fail "Cannot create a temporary directory."
chmod 700 "${WORK}"
ADMIN_HEADER="${WORK}/admin.header"
BOT_HEADER="${WORK}/bot.header"
PF_PID=""
ADMIN_TOKEN_NAME="scratch-repo-$(date +%s)"
cleanup()
{
    # The admin token of this run is deleted again (needs the admin password, basic auth only)
    if ! ${DRY_RUN} && [ -f "${ADMIN_HEADER}" ] && [ -n "${API:-}" ]; then
        local admin_pass
        admin_pass=$(${KC} -n ${NAMESPACE} get secret forgejo-credentials -o jsonpath='{.data.password}' 2>/dev/null | base64 -d)
        if [ -n "${admin_pass}" ]; then
            ( umask 077 && printf 'Authorization: Basic %s\n' "$(printf '%s:%s' "${ADMIN_USER}" "${admin_pass}" | base64 -w0)" > "${WORK}/admin-basic.header" )
            curl -s -o /dev/null -H @"${WORK}/admin-basic.header" -X DELETE "${API}/users/${ADMIN_USER}/tokens/${ADMIN_TOKEN_NAME}"
        fi
    fi
    [ -n "${PF_PID}" ] && kill "${PF_PID}" 2>/dev/null
    rm -rf "${WORK}"
}
trap cleanup EXIT

random_password()
{
    openssl rand -base64 64 | tr -dc 'A-Za-z0-9' | head -c 32
}

# ---------------------------------------------------------------------------
# API helpers. req METHOD PATH [HEADER_FILE] [DESCRIPTION]: the request body, if any, comes from stdin.
# Sets CODE (HTTP status) and BODY (response file). In dry-run mode the call is printed and CODE is 000.
# ---------------------------------------------------------------------------
RESP="${WORK}/response.json"
req()
{
    local method="$1" path="$2" header="${3:-${ADMIN_HEADER}}" description="${4:-}"
    BODY="${RESP}"
    : > "${RESP}"
    if ${DRY_RUN}; then
        printf 'DRY-RUN: %-6s /api/v1%s%s\n' "${method}" "${path}" "${description:+   (${description})}"
        # The body is consumed unseen: it can hold a password or a token
        [ "${method}" = "GET" ] || [ "${method}" = "DELETE" ] || cat > /dev/null
        CODE="000"
        return 0
    fi
    if [ "${method}" = "GET" ] || [ "${method}" = "DELETE" ]; then
        CODE=$(curl -s -o "${RESP}" -w '%{http_code}' -H @"${header}" -X "${method}" "${API}${path}" < /dev/null)
    else
        CODE=$(curl -s -o "${RESP}" -w '%{http_code}' -H @"${header}" -H "Content-Type: application/json" \
                    -X "${method}" --data-binary @- "${API}${path}")
    fi
}
# Body helpers: JSON built with python3 from arguments that hold no secrets, or from the environment for secrets
json_body()
{
    python3 -c 'import json,sys; a=sys.argv[1:]; print(json.dumps({a[i]: json.loads(a[i+1]) for i in range(0, len(a), 2)}))' "$@"
}
json_field()
{
    python3 -c 'import json,sys
try:
    v = json.load(open(sys.argv[1]))
    for k in sys.argv[2:]:
        v = v[k]
    print(json.dumps(v) if isinstance(v, (dict, list, bool)) or v is None else v)
except Exception:
    pass' "${BODY}" "$@"
}
expect()
{
    local what="$1"; shift
    ${DRY_RUN} && return 0
    local c
    for c in "$@"; do [ "${CODE}" = "${c}" ] && return 0; done
    fj_fail "${what} returned HTTP ${CODE}." "$(head -c 300 "${BODY}" 2>/dev/null)"
}

# ---------------------------------------------------------------------------
# Connection: admin token and a port-forward, as sync-repos.sh
# ---------------------------------------------------------------------------
LOCAL_PORT=13002
API="http://127.0.0.1:${LOCAL_PORT}/api/v1"
GIT_BASE="http://127.0.0.1:${LOCAL_PORT}"
if ${DRY_RUN}; then
    echo "DRY-RUN: no cluster access. Planned for ${REPO_PATH} (bot ${BOT_USER}) through a port-forward to svc/${RELEASE}-http:${FORGEJO_HTTP_PORT}:"
    echo "DRY-RUN: forgejo admin user generate-access-token --username ${ADMIN_USER} --token-name ${ADMIN_TOKEN_NAME} --scopes all (in the forgejo pod)"
else
    ${KC} -n ${NAMESPACE} get deploy ${RELEASE} >/dev/null 2>&1 || fj_fail "Forgejo is not installed." "Run ./setup.sh first."
    ADMIN_TOKEN=$(${KC} -n ${NAMESPACE} exec deploy/${RELEASE} -c forgejo -- forgejo admin user generate-access-token \
        --username "${ADMIN_USER}" --token-name "${ADMIN_TOKEN_NAME}" --scopes all --raw 2>/dev/null | tail -n 1 | tr -d '\r\n ')
    [ -n "${ADMIN_TOKEN}" ] || fj_fail "Could not generate an admin API token."
    ( umask 077 && printf 'Authorization: token %s\n' "${ADMIN_TOKEN}" > "${ADMIN_HEADER}" )
    ${KC} -n ${NAMESPACE} port-forward svc/${RELEASE}-http ${LOCAL_PORT}:${FORGEJO_HTTP_PORT} >/dev/null 2>&1 &
    PF_PID=$!
    for _ in $(seq 1 20); do
        curl -s -o /dev/null "${API}/version" && break
        sleep 1
    done
    curl -s -o /dev/null "${API}/version" || fj_fail "The port-forward to Forgejo did not come up."
fi

# ---------------------------------------------------------------------------
# Prune mode
# ---------------------------------------------------------------------------
if ${PRUNE}; then
    echo "Pruning scratch/* branches of ${REPO_PATH} older than ${PRUNE_DAYS} days"
    CUTOFF=$(( $(date +%s) - PRUNE_DAYS * 86400 ))
    : > "${WORK}/prune.txt"
    PAGE=1
    while :; do
        req GET "/repos/${REPO_PATH}/branches?page=${PAGE}&limit=50" "" "list branches, page by page"
        ${DRY_RUN} && break
        expect "Listing the branches" 200
        COUNT=$(python3 -c 'import json,sys; print(len(json.load(open(sys.argv[1]))))' "${BODY}")
        python3 - "${BODY}" "${CUTOFF}" >> "${WORK}/prune.txt" <<'PY'
import json, sys
from datetime import datetime
cutoff = int(sys.argv[2])
for b in json.load(open(sys.argv[1])):
    name = b.get("name", "")
    stamp = (b.get("commit") or {}).get("timestamp", "")
    if not name.startswith("scratch/") or not stamp:
        continue
    try:
        when = datetime.fromisoformat(stamp.replace("Z", "+00:00")).timestamp()
    except ValueError:
        continue
    if when < cutoff:
        print(name)
PY
        [ "${COUNT}" -lt 50 ] && break
        PAGE=$((PAGE + 1))
    done
    if ${DRY_RUN}; then
        echo "DRY-RUN: DELETE /api/v1/repos/${REPO_PATH}/branches/<name>   (each scratch/* branch whose head commit is older than ${PRUNE_DAYS} days)"
        exit 0
    fi
    DELETED=0; FAILED=0
    while read -r BRANCH; do
        [ -n "${BRANCH}" ] || continue
        ENCODED=$(python3 -c 'import sys,urllib.parse; print(urllib.parse.quote(sys.argv[1], safe="/"))' "${BRANCH}")
        req DELETE "/repos/${REPO_PATH}/branches/${ENCODED}"
        if [ "${CODE}" = "204" ]; then
            echo "  deleted ${BRANCH}"; DELETED=$((DELETED + 1))
        else
            echo "  deleting ${BRANCH} failed (HTTP ${CODE})"; FAILED=1
        fi
    done < "${WORK}/prune.txt"
    echo "Pruned ${DELETED} branch(es)."
    exit ${FAILED}
fi

# ---------------------------------------------------------------------------
# Bot user
# ---------------------------------------------------------------------------
store_bot_password()
{
    # Appends the password to secrets.env and to the user-profile copy, keeping both owner-only.
    # A missing secrets.env is not created: create-sealed-secret.sh would take it for a complete one.
    local file
    for file in "${SECRETS_FILE}" "${SECRETS_COPY}"; do
        if [ "${file}" = "${SECRETS_FILE}" ] && [ ! -f "${file}" ]; then
            echo "Note: ${SECRETS_FILE} does not exist, the bot password is stored in ${SECRETS_COPY} only"
            continue
        fi
        if [ "${file}" = "${SECRETS_COPY}" ]; then
            mkdir -p "$(dirname "${file}")" && chmod 700 "$(dirname "${file}")"
        fi
        ( umask 077 && touch "${file}" )
        if grep -q '^FORGEJO_CI_BOT_PASSWORD=' "${file}" 2>/dev/null; then
            sed -i "s|^FORGEJO_CI_BOT_PASSWORD=.*|FORGEJO_CI_BOT_PASSWORD=${FORGEJO_CI_BOT_PASSWORD}|" "${file}"
        else
            printf '\n# Password of the Forgejo CI bot user %s (scratch-repo.sh)\nFORGEJO_CI_BOT_PASSWORD=%s\n' \
                "${BOT_USER}" "${FORGEJO_CI_BOT_PASSWORD}" >> "${file}"
        fi
        chmod 600 "${file}"
    done
}

req GET "/users/${BOT_USER}" "" "does the bot user exist?"
if [ "${CODE}" = "200" ]; then
    echo "User ${BOT_USER}: exists"
    if [ -z "${FORGEJO_CI_BOT_PASSWORD}" ]; then
        # The password is not stored here: set a new one so this machine can act as the bot
        FORGEJO_CI_BOT_PASSWORD=$(random_password)
        store_bot_password
        P="${FORGEJO_CI_BOT_PASSWORD}" python3 -c 'import json,os,sys; print(json.dumps({"login_name": sys.argv[1], "source_id": 0, "password": os.environ["P"], "must_change_password": False}))' \
            "${BOT_USER}" | req PATCH "/admin/users/${BOT_USER}"
        expect "Setting the password of ${BOT_USER}" 200
        echo "User ${BOT_USER}: no stored password, a new one was set and stored as FORGEJO_CI_BOT_PASSWORD"
    fi
elif [ "${CODE}" = "404" ] || ${DRY_RUN}; then
    if ${DRY_RUN}; then
        echo "DRY-RUN: generate a random password, store FORGEJO_CI_BOT_PASSWORD in ${SECRETS_FILE} and ${SECRETS_COPY} (only when not stored yet)"
    elif [ -z "${FORGEJO_CI_BOT_PASSWORD}" ]; then
        FORGEJO_CI_BOT_PASSWORD=$(random_password)
        store_bot_password
    fi
    P="${FORGEJO_CI_BOT_PASSWORD}" python3 -c 'import json,os,sys; print(json.dumps({"username": sys.argv[1], "email": sys.argv[2], "password": os.environ.get("P", ""), "must_change_password": False, "send_notify": False, "visibility": "private"}))' \
        "${BOT_USER}" "${BOT_EMAIL}" | req POST "/admin/users" "" "create user ${BOT_USER}, body with the password from stdin"
    expect "Creating user ${BOT_USER}" 201
    ${DRY_RUN} || echo "User ${BOT_USER}: created, password stored as FORGEJO_CI_BOT_PASSWORD"
else
    fj_fail "Looking up user ${BOT_USER} returned HTTP ${CODE}."
fi
if ! ${DRY_RUN}; then
    ( umask 077 && printf 'Authorization: Basic %s\n' "$(printf '%s:%s' "${BOT_USER}" "${FORGEJO_CI_BOT_PASSWORD}" | base64 -w0)" > "${BOT_HEADER}" )
fi

# ---------------------------------------------------------------------------
# Owner and repository
# ---------------------------------------------------------------------------
OWNER_IS_ORG=true
req GET "/users/${CI_OWNER}" "" "does the owner exist (user or organization)?"
if [ "${CODE}" = "200" ]; then
    req GET "/orgs/${CI_OWNER}"
    [ "${CODE}" = "200" ] || OWNER_IS_ORG=false
    echo "Owner ${CI_OWNER}: exists ($(${OWNER_IS_ORG} && echo organization || echo user))"
elif [ "${CODE}" = "404" ] || ${DRY_RUN}; then
    json_body username "\"${CI_OWNER}\"" visibility '"private"' | req POST "/orgs" "" "create organization ${CI_OWNER}, private, when missing"
    expect "Creating organization ${CI_OWNER}" 201
    ${DRY_RUN} || echo "Owner ${CI_OWNER}: organization created (private)"
else
    fj_fail "Looking up ${CI_OWNER} returned HTTP ${CODE}."
fi

REPO_EMPTY=true
req GET "/repos/${REPO_PATH}" "" "does the scratch repository exist?"
if [ "${CODE}" = "200" ]; then
    [ "$(json_field mirror)" = "true" ] && fj_fail "${REPO_PATH} is a mirror and refuses pushes." \
        "Use another FORGEJO_CI_OWNER/FORGEJO_CI_REPO for the scratch repository; the mirror stays as it is."
    [ "$(json_field empty)" = "true" ] || REPO_EMPTY=false
    echo "Repository ${REPO_PATH}: exists$(${REPO_EMPTY} && echo ', empty')"
elif [ "${CODE}" = "404" ] || ${DRY_RUN}; then
    BODY_JSON=$(json_body name "\"${CI_REPO}\"" private true auto_init false \
                description '"CI scratch repository: snapshots on scratch/* branches, built by workflow_dispatch"')
    if ${OWNER_IS_ORG}; then
        echo "${BODY_JSON}" | req POST "/orgs/${CI_OWNER}/repos" "" "create repository ${CI_REPO}, private, empty"
    else
        echo "${BODY_JSON}" | req POST "/admin/users/${CI_OWNER}/repos" "" "create repository ${CI_REPO}, private, empty"
    fi
    expect "Creating repository ${REPO_PATH}" 201
    ${DRY_RUN} || echo "Repository ${REPO_PATH}: created (private)"
else
    fj_fail "Looking up ${REPO_PATH} returned HTTP ${CODE}."
fi

# ---------------------------------------------------------------------------
# Seeding (once, while empty). Actions are off during the push.
# ---------------------------------------------------------------------------
set_actions()
{
    json_body has_actions "$1" | req PATCH "/repos/${REPO_PATH}" "" "has_actions=$1"
    expect "Setting has_actions=$1 on ${REPO_PATH}" 200
}
if [ -n "${SEED}" ]; then
    if ! ${REPO_EMPTY}; then
        echo "Seed: ${REPO_PATH} is not empty, seeding skipped (it is done once)"
    else
        if git -C "${SEED}" rev-parse --git-dir >/dev/null 2>&1; then
            SOURCE_DIR="$(cd "${SEED}" && pwd)"
            CLONE_CMD=""
        else
            SOURCE_DIR="${WORK}/seed.git"
            CLONE_CMD="git clone --mirror --quiet ${SEED} ${SOURCE_DIR}"
        fi
        set_actions false
        if ${DRY_RUN}; then
            [ -n "${CLONE_CMD}" ] && echo "DRY-RUN: ${CLONE_CMD}"
            echo "DRY-RUN: git -C ${SOURCE_DIR} push ${GIT_BASE}/${REPO_PATH}.git 'refs/heads/*:refs/heads/*' 'refs/tags/*:refs/tags/*'   (admin token in an http.extraHeader from the environment)"
            echo "DRY-RUN: PATCH  /api/v1/repos/${REPO_PATH}   (default_branch master or main, whichever was pushed)"
        else
            if [ -n "${CLONE_CMD}" ]; then
                echo "Seed: cloning ${SEED}"
                git clone --mirror --quiet "${SEED}" "${SOURCE_DIR}" || fj_fail "Cloning ${SEED} failed."
            fi
            echo "Seed: pushing all branches and tags of ${SEED} to ${REPO_PATH}"
            # The credential reaches git through the environment, not the command line or a remote URL
            if ! GIT_CONFIG_COUNT=1 GIT_CONFIG_KEY_0="http.extraHeader" \
                 GIT_CONFIG_VALUE_0="Authorization: Basic $(printf '%s:%s' "${ADMIN_USER}" "${ADMIN_TOKEN}" | base64 -w0)" \
                 git -C "${SOURCE_DIR}" push --quiet "${GIT_BASE}/${REPO_PATH}.git" 'refs/heads/*:refs/heads/*' 'refs/tags/*:refs/tags/*'; then
                set_actions true
                fj_fail "Pushing the seed to ${REPO_PATH} failed."
            fi
            for DEFAULT in master main; do
                if git -C "${SOURCE_DIR}" show-ref --verify --quiet "refs/heads/${DEFAULT}"; then
                    json_body default_branch "\"${DEFAULT}\"" | req PATCH "/repos/${REPO_PATH}"
                    expect "Setting the default branch of ${REPO_PATH}" 200
                    break
                fi
            done
            echo "Seed: done"
        fi
    fi
elif ${REPO_EMPTY} && ! ${DRY_RUN}; then
    echo "Seed: ${REPO_PATH} is empty. Run again with --seed <git url or local repository> to fill it."
fi

set_actions true
${DRY_RUN} || echo "Repository ${REPO_PATH}: Actions enabled"

json_body permission '"write"' | req PUT "/repos/${REPO_PATH}/collaborators/${BOT_USER}" "" "${BOT_USER} collaborator with write access"
expect "Adding ${BOT_USER} as collaborator" 204
${DRY_RUN} || echo "Repository ${REPO_PATH}: ${BOT_USER} has write access"

# ---------------------------------------------------------------------------
# Report token: a package token of the bot user as Actions secret CI_REPORT_TOKEN
# ---------------------------------------------------------------------------
if ${REPORT_TOKEN}; then
    req GET "/users/${BOT_USER}/tokens" "${BOT_HEADER}" "as ${BOT_USER} (basic auth): list its tokens"
    expect "Listing the tokens of ${BOT_USER}" 200
    if ${DRY_RUN}; then
        echo "DRY-RUN: DELETE /api/v1/users/${BOT_USER}/tokens/<id>   (as ${BOT_USER}: each earlier token named ci-report-*)"
    else
        for TOKEN_ID in $(python3 -c 'import json,sys; print(" ".join(str(t["id"]) for t in json.load(open(sys.argv[1])) if t.get("name","").startswith("ci-report-")))' "${BODY}"); do
            req DELETE "/users/${BOT_USER}/tokens/${TOKEN_ID}" "${BOT_HEADER}"
            expect "Deleting an earlier report token" 204
        done
    fi
    json_body name "\"ci-report-$(date +%s)\"" scopes '["write:package"]' \
        | req POST "/users/${BOT_USER}/tokens" "${BOT_HEADER}" "as ${BOT_USER}: new token, scope write:package"
    expect "Creating the report token" 201
    if ${DRY_RUN}; then
        echo "DRY-RUN: PUT    /api/v1/repos/${REPO_PATH}/actions/secrets/CI_REPORT_TOKEN   (body with the new token from stdin)"
    else
        REPORT_TOKEN_VALUE="$(json_field sha1)"
        [ -n "${REPORT_TOKEN_VALUE}" ] || fj_fail "The new report token was not in the response."
        T="${REPORT_TOKEN_VALUE}" python3 -c 'import json,os; print(json.dumps({"data": os.environ["T"]}))' \
            | req PUT "/repos/${REPO_PATH}/actions/secrets/CI_REPORT_TOKEN"
        expect "Storing the Actions secret CI_REPORT_TOKEN" 201 204
        unset REPORT_TOKEN_VALUE
        echo "Repository ${REPO_PATH}: Actions secret CI_REPORT_TOKEN set (package token of ${BOT_USER})"
    fi
fi

# ---------------------------------------------------------------------------
# Gateway token: an access token of the bot user for the CI gateway, in a file of mode 0600, never printed.
# The new token is stored before earlier ones are deleted, so the gateway always has a valid one.
# ---------------------------------------------------------------------------
GATEWAY_TOKEN_SCOPES='["write:repository", "read:package", "write:package"]'
if [ -n "${GATEWAY_TOKEN_FILE}" ]; then
    if [ -s "${GATEWAY_TOKEN_FILE}" ] && ! ${ROTATE}; then
        echo "Gateway token: ${GATEWAY_TOKEN_FILE} exists, kept (--rotate makes a new one)"
    else
        req GET "/users/${BOT_USER}/tokens?limit=50" "${BOT_HEADER}" "as ${BOT_USER} (basic auth): list its tokens"
        expect "Listing the tokens of ${BOT_USER}" 200
        EARLIER_GATEWAY_TOKENS=""
        if ! ${DRY_RUN}; then
            EARLIER_GATEWAY_TOKENS=$(python3 -c 'import json,sys; print(" ".join(str(t["id"]) for t in json.load(open(sys.argv[1])) if t.get("name","").startswith("ci-gateway-")))' "${BODY}")
        fi
        json_body name "\"ci-gateway-$(date +%s)\"" scopes "${GATEWAY_TOKEN_SCOPES}" \
            | req POST "/users/${BOT_USER}/tokens" "${BOT_HEADER}" "as ${BOT_USER}: new token, scopes write:repository, read:package, write:package"
        expect "Creating the gateway token" 201
        if ${DRY_RUN}; then
            echo "DRY-RUN: write the new token to ${GATEWAY_TOKEN_FILE} (umask 077, install -m 600; it is never printed)"
            ${ROTATE} && echo "DRY-RUN: DELETE /api/v1/users/${BOT_USER}/tokens/<id>   (as ${BOT_USER}: each earlier token named ci-gateway-*, after the new one is stored)"
        else
            GATEWAY_TOKEN_VALUE="$(json_field sha1)"
            [ -n "${GATEWAY_TOKEN_VALUE}" ] || fj_fail "The new gateway token was not in the response."
            ( umask 077 && printf '%s\n' "${GATEWAY_TOKEN_VALUE}" > "${WORK}/gateway.token" )
            unset GATEWAY_TOKEN_VALUE
            : > "${RESP}"
            ( umask 077 && install -m 600 "${WORK}/gateway.token" "${GATEWAY_TOKEN_FILE}" ) || fj_fail "Writing ${GATEWAY_TOKEN_FILE} failed."
            rm -f "${WORK}/gateway.token"
            echo "Gateway token: written to ${GATEWAY_TOKEN_FILE} (mode 0600; scopes write:repository, read:package, write:package)"
            if ${ROTATE}; then
                for TOKEN_ID in ${EARLIER_GATEWAY_TOKENS}; do
                    req DELETE "/users/${BOT_USER}/tokens/${TOKEN_ID}" "${BOT_HEADER}"
                    expect "Deleting an earlier gateway token" 204
                done
                [ -n "${EARLIER_GATEWAY_TOKENS}" ] && echo "Gateway token: the earlier gateway tokens of ${BOT_USER} are deleted"
            elif [ -n "${EARLIER_GATEWAY_TOKENS}" ]; then
                echo "Gateway token: $(echo "${EARLIER_GATEWAY_TOKENS}" | wc -w) earlier gateway token(s) of ${BOT_USER} stay valid; --rotate deletes them"
            fi
        fi
    fi
fi

echo
echo "Scratch repository: ${REPO_PATH}   bot user: ${BOT_USER}   reports: packages of ${BOT_USER}, package ci-reports"
${DRY_RUN} && echo "Dry run: nothing was changed."
exit 0
