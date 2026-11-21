#!/bin/bash
# Gives the containerd of this k3s node the container registry of the cluster (the Forgejo container registry, plain
# HTTP) with a pull token: /etc/rancher/k3s/registries.yaml, then a restart of k3s (server node) or k3s-agent (agent
# node) when the file changed. Running pods keep running. Idempotent: a second run changes nothing.
#
# Usage: sudo registry/setup-node.sh --token-file FILE [--registry HOST:PORT] [--user NAME] [--no-restart] [--dry-run]
#                                    [--file PATH]
#   --token-file FILE    the pull token of the registry (scratch-repo.sh --registry-token FILE: scope read:package);
#                        "-" reads it from standard input. It is never printed and never on a command line
#   --registry HOST:PORT the registry; default FORGEJO_REGISTRY_HOST of ../config.env, else FORGEJO_HOST:FORGEJO_HTTP_PORT
#                        of it (FORGEJO_HTTP_PORT default 32001)
#   --user NAME          the user the token belongs to; default FORGEJO_CI_BOT_USER of ../config.env, else ci-bot
#   --no-restart         write the file only; k3s reads it when it starts next (agent-node/join.sh restarts it itself)
#   --file PATH          the file to write instead of /etc/rancher/k3s/registries.yaml (for tests; no root needed)
#   --dry-run            prints what it would do, the content with the token replaced; writes and restarts nothing
#
# The file gets, for the registry: mirrors."HOST:PORT".endpoint ["http://HOST:PORT"] and configs."HOST:PORT".auth
# (username, password). An existing file with other entries is merged: its other mirrors and configs stay (python3 with
# the yaml module, package python3-yaml, is needed for that; without it an existing file with other content is refused
# and left alone). The file is root:root, mode 0600. A file that holds what it should already is not rewritten.
# Exit code 0 when done (or nothing to do), 1 on an error, 2 for a usage error.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FORGEJO_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
TARGET="/etc/rancher/k3s/registries.yaml"
HEADER="# Written by projects/k3s/forgejo/registry/setup-node.sh"

usage() {
    sed -n '2,/^set -euo pipefail/p' "${BASH_SOURCE[0]}" | sed '$d' | sed 's/^# \{0,1\}//'
}
fail() {
    echo "setup-node: $*" >&2
    exit 1
}
usage_error() {
    echo "setup-node: $*" >&2
    exit 2
}

TOKEN_FILE=""
REGISTRY=""
REGISTRY_USER=""
RESTART=true
DRY_RUN=false
FILE_GIVEN=false
while [ $# -gt 0 ]; do
    case "$1" in
        --token-file) TOKEN_FILE="${2:?--token-file needs a file}"; shift ;;
        --registry) REGISTRY="${2:?--registry needs HOST:PORT}"; shift ;;
        --user) REGISTRY_USER="${2:?--user needs a name}"; shift ;;
        --no-restart) RESTART=false ;;
        --file) TARGET="${2:?--file needs a path}"; FILE_GIVEN=true; shift ;;
        --dry-run) DRY_RUN=true ;;
        -h|--help) usage; exit 0 ;;
        *) echo "setup-node: unknown option $1" >&2; usage >&2; exit 2 ;;
    esac
    shift
done

# settings of ../config.env, read as KEY=value lines (the file is not run)
config_value() {
    [ -f "${FORGEJO_DIR}/config.env" ] || return 0
    sed -n "s/\r\$//; s/^$1=//p" "${FORGEJO_DIR}/config.env" | tail -n 1 | sed 's/^"\(.*\)"$/\1/; s/^'"'"'\(.*\)'"'"'$/\1/'
}
if [ -z "${REGISTRY}" ]; then
    REGISTRY="$(config_value FORGEJO_REGISTRY_HOST)"
fi
if [ -z "${REGISTRY}" ] && [ -n "$(config_value FORGEJO_HOST)" ]; then
    port="$(config_value FORGEJO_HTTP_PORT)"
    REGISTRY="$(config_value FORGEJO_HOST):${port:-32001}"
fi
[ -n "${REGISTRY}" ] || usage_error "give --registry HOST:PORT (or FORGEJO_REGISTRY_HOST or FORGEJO_HOST in ${FORGEJO_DIR}/config.env)"
[[ "${REGISTRY}" =~ ^[A-Za-z0-9]([A-Za-z0-9.-]{0,252}[A-Za-z0-9])?:[0-9]{1,5}$ ]] || usage_error "--registry '${REGISTRY}' is not HOST:PORT"
REGISTRY_USER="${REGISTRY_USER:-$(config_value FORGEJO_CI_BOT_USER)}"
REGISTRY_USER="${REGISTRY_USER:-ci-bot}"
[[ "${REGISTRY_USER}" =~ ^[A-Za-z0-9][A-Za-z0-9._-]{0,63}$ ]] || usage_error "--user '${REGISTRY_USER}' is not a user name"
[ -n "${TOKEN_FILE}" ] || usage_error "--token-file FILE is required (scratch-repo.sh --registry-token FILE makes it)"
if ! ${DRY_RUN} && ! ${FILE_GIVEN} && [ "$(id -u)" -ne 0 ]; then
    fail "must run as root (sudo): it writes ${TARGET} and restarts k3s"
fi

# the token: first line of the file, or of standard input
TOKEN=""
if [ "${TOKEN_FILE}" = "-" ]; then
    IFS= read -r TOKEN || [ -n "${TOKEN}" ] || true
else
    [ -f "${TOKEN_FILE}" ] && [ -r "${TOKEN_FILE}" ] || fail "the token file ${TOKEN_FILE} is not a readable file"
    if [ -n "$(find "${TOKEN_FILE}" -maxdepth 0 -perm /044 2> /dev/null)" ]; then
        echo "setup-node: warning: ${TOKEN_FILE} is readable by others than its owner; chmod 600 it" >&2
    fi
    IFS= read -r TOKEN < "${TOKEN_FILE}" || [ -n "${TOKEN}" ] || true
fi
TOKEN="${TOKEN%$'\r'}"
[ -n "${TOKEN}" ] || fail "the token file holds no token on its first line"
# a token is quoted in the YAML: only characters that need no escape
[[ "${TOKEN}" =~ ^[A-Za-z0-9._~+/=-]+$ ]] || fail "the token holds characters that are not expected in a token"

WORK="$(mktemp -d "${TMPDIR:-/tmp}/setup-node.XXXXXX")"
trap 'rm -rf "${WORK}"' EXIT

# the content of a file that holds only this registry; generated with the token or with a placeholder (for the output)
own_content() {
    local token="$1"
    printf '%s\n' "${HEADER}"
    printf 'mirrors:\n  "%s":\n    endpoint:\n      - "http://%s"\n' "${REGISTRY}" "${REGISTRY}"
    printf 'configs:\n  "%s":\n    auth:\n      username: "%s"\n      password: "%s"\n' "${REGISTRY}" "${REGISTRY_USER}" "${token}"
}
# merge into an existing file: python3 with yaml reads the token from standard input; writes <out> and prints
# "changed" or "unchanged"
MERGE_PY='
import copy, sys, yaml
existing_path, out_path, header, registry, user = sys.argv[1:6]
token = sys.stdin.readline().rstrip("\r\n")
with open(existing_path) as f:
    data = yaml.safe_load(f)
if data is None:
    data = {}
if not isinstance(data, dict):
    sys.exit("the existing file is not a YAML mapping")
merged = copy.deepcopy(data)
for key in ("mirrors", "configs"):
    if merged.get(key) is None:
        merged[key] = {}
    if not isinstance(merged[key], dict):
        sys.exit("%s of the existing file is not a mapping" % key)
mirror = merged["mirrors"].get(registry)
mirror = dict(mirror) if isinstance(mirror, dict) else {}
mirror["endpoint"] = ["http://" + registry]
merged["mirrors"][registry] = mirror
config = merged["configs"].get(registry)
config = dict(config) if isinstance(config, dict) else {}
config["auth"] = {"username": user, "password": token}
merged["configs"][registry] = config
with open(out_path, "w") as f:
    f.write(header + "\n")
    yaml.safe_dump(merged, f, default_flow_style=False, sort_keys=False)
print("unchanged" if merged == data else "changed")
'
have_python_yaml() {
    command -v python3 > /dev/null 2>&1 && python3 -c 'import yaml' > /dev/null 2>&1
}

NEW="${WORK}/registries.yaml"
SHOWN="${WORK}/registries.shown.yaml"
STATE=""
if [ -e "${TARGET}" ] && [ ! -r "${TARGET}" ]; then
    fail "${TARGET} exists and cannot be read by this user: run with sudo"
fi
( umask 077 && own_content "${TOKEN}" > "${NEW}" )
own_content "<token of ${TOKEN_FILE}>" > "${SHOWN}"
if [ ! -e "${TARGET}" ]; then
    STATE="new"
elif cmp -s "${NEW}" "${TARGET}"; then
    STATE="unchanged"
else
    have_python_yaml || fail "${TARGET} exists and holds other content; merging it needs python3 with the yaml module (python3-yaml). The file is left as it is: install python3-yaml, or add the registry by hand"
    STATE="$(printf '%s\n' "${TOKEN}" | ( umask 077 && python3 -c "${MERGE_PY}" "${TARGET}" "${NEW}" "${HEADER}" "${REGISTRY}" "${REGISTRY_USER}" ))" \
        || fail "${TARGET} could not be merged (above); it is left as it is"
    printf '%s\n' "placeholder" | python3 -c "${MERGE_PY}" "${TARGET}" "${SHOWN}" "${HEADER}" "${REGISTRY}" "${REGISTRY_USER}" > /dev/null \
        && sed -i 's/^\( *password: \)placeholder$/\1<token of '"$(printf '%s' "${TOKEN_FILE}" | sed 's/[\/&|]/\\&/g')"'>/' "${SHOWN}"
    [ "${STATE}" = "unchanged" ] || STATE="merged"
fi
TOKEN=""

# the unit of this node: k3s on a server node, k3s-agent on an agent node
UNIT=""
if command -v systemctl > /dev/null 2>&1; then
    if systemctl cat k3s.service > /dev/null 2>&1; then
        UNIT="k3s"
    elif systemctl cat k3s-agent.service > /dev/null 2>&1; then
        UNIT="k3s-agent"
    fi
fi

if ${DRY_RUN}; then
    case "${STATE}" in
        unchanged) echo "setup-node: (dry run) ${TARGET} holds the registry ${REGISTRY} already: nothing to write, nothing to restart" ;;
        *)
            echo "setup-node: (dry run) would write ${TARGET} (${STATE}; root:root, mode 0600):"
            sed 's/^/    /' "${SHOWN}"
            if ! ${RESTART}; then
                echo "setup-node: (dry run) --no-restart: k3s would read it at its next start"
            elif [ -n "${UNIT}" ]; then
                echo "setup-node: (dry run) would restart ${UNIT} (running pods keep running)"
            else
                echo "setup-node: (dry run) no k3s or k3s-agent unit on this machine: nothing would be restarted"
            fi ;;
    esac
    exit 0
fi

if [ "${STATE}" = "unchanged" ]; then
    # the content is right; the owner and the mode are made right as well
    chmod 600 "${TARGET}"
    [ "$(id -u)" -ne 0 ] || chown root:root "${TARGET}"
    echo "setup-node: ${TARGET} holds the registry ${REGISTRY} already; nothing restarted"
    exit 0
fi
mkdir -p "$(dirname "${TARGET}")"
TEMP="$(mktemp "$(dirname "${TARGET}")/.registries.yaml.XXXXXX")"
cat "${NEW}" > "${TEMP}"
chmod 600 "${TEMP}"
[ "$(id -u)" -ne 0 ] || chown root:root "${TEMP}"
mv -f "${TEMP}" "${TARGET}"
echo "setup-node: wrote ${TARGET} (${STATE}): registry ${REGISTRY} over http, pull user ${REGISTRY_USER}"
if ! ${RESTART}; then
    echo "setup-node: --no-restart: k3s reads it at its next start"
elif [ -n "${UNIT}" ]; then
    echo "setup-node: restarting ${UNIT} for the new registry configuration (running pods keep running)..."
    systemctl restart "${UNIT}" || fail "systemctl restart ${UNIT} failed; see journalctl -u ${UNIT}"
    echo "setup-node: ${UNIT} restarted"
else
    echo "setup-node: no k3s or k3s-agent unit on this machine: nothing restarted (k3s reads the file when it starts)"
fi
