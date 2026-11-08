#!/bin/bash
# ---------------------------------------------------------------------------
# Builds, tests and installs the MCP server gucefMcpLocalLlm on this machine
# and runs it as the systemd service gucef-mcp, for every Claude session that
# needs it: local ones over http://127.0.0.1:MCP_PORT/mcp, sessions on other
# machines over https://<this machine>:MCP_TLS_PORT/mcp. Idempotent: a re-run
# builds what changed, and installs and restarts only when the result differs.
#
#   - build: a Release build of the checkout this script is in, in
#     /var/lib/gucef-mcp/build, as the user the service runs as
#   - test: the test application of the server; a failure keeps the version
#     that is installed
#   - install: the server, its libraries and the coding agent client, stripped,
#     in /opt/gucef-mcp/<version>; /opt/gucef-mcp/current points to the one in
#     use. A running session keeps the version it started with until the restart
#   - service: gucef-mcp, as that user, from boot. The bearer token clients send
#     is the lab MCP token, /var/lib/gucef-mcp/lab-token, which llm/mcp/cluster.sh
#     makes and its DaemonSet delivers: the same on every node. Until it is there
#     the service makes and uses /var/lib/gucef-mcp/token of its own
#   - HTTPS: with the certificate of this node, which llm/mcp/cluster.sh on a
#     server makes and its DaemonSet copies to /var/lib/gucef-mcp/tls. Until it
#     is there only the local listener runs; the service starts HTTPS once it is
#   - LLM servers: /var/lib/gucef-mcp/endpoints, kept up to date by the same
#     DaemonSet; the server reads it again when it changes
#   - Claude Code of the user: the coding agent and the registration need a
#     claude command. When the user has Claude Code only as an editor extension
#     (VS Code and forks), a wrapper ~/.local/bin/claude runs the newest one.
#     The local server is registered as local-llm (--scope user)
#
# Without a checkout around this script, as in the package gucef-k3s-scripts,
# it installs the built files of the package gucef-mcp (/usr/lib/gucef-mcp)
# instead of building, and its settings are /etc/gucef-mcp/settings.env.
#
# Usage: sudo ./install.sh [--user NAME] [--skip-tests] [--prebuilt DIR] [--upgrade]
#   --user NAME     the user the service runs as; default: the user that ran sudo.
#                   Its files are the ones the tools work on
#   --skip-tests    install without running the tests
#   --prebuilt DIR  install the built files in DIR (bin/, lib/) instead of building
#   --upgrade       only when the service is installed already: install the new
#                   files for the user it runs as (used by the package)
# Settings: config.env next to this script (see config.env.example)
# ---------------------------------------------------------------------------

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GUCEF_ROOT="$(cd "${SCRIPT_DIR}/../../../.." && pwd)"
STATE_DIR="/var/lib/gucef-mcp"
BUILD_DIR="${STATE_DIR}/build"
INSTALL_ROOT="/opt/gucef-mcp"
CONFIG_DIR="/etc/gucef-mcp"
UNIT_DIR="/etc/systemd/system"
SERVICE="gucef-mcp"
PACKAGE_DIR="/usr/lib/gucef-mcp"
KEEP_VERSIONS=3

SERVICE_USER="${SUDO_USER:-}"
SKIP_TESTS=false
PREBUILT=""
UPGRADE=false
while [ $# -gt 0 ]; do
    case "$1" in
        --user) SERVICE_USER="$2"; shift ;;
        --skip-tests) SKIP_TESTS=true ;;
        --prebuilt) PREBUILT="$2"; shift ;;
        --upgrade) UPGRADE=true ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
    shift
done

if [ "$(id -u)" -ne 0 ]; then
    echo "ERROR: run with sudo: it installs a systemd service."
    exit 1
fi
if [ "${UPGRADE}" = "true" ]; then
    SERVICE_USER="$(sed -n 's/^User=//p' "${UNIT_DIR}/${SERVICE}.service" 2>/dev/null)"
    [ -n "${SERVICE_USER}" ] || exit 0
fi
SETTINGS_FILE="${SCRIPT_DIR}/config.env"
if [ ! -d "${GUCEF_ROOT}/projects/CMake/targets/GUCEF_exe_gucefMcpLocalLlm_TestApp" ]; then
    PREBUILT="${PREBUILT:-${PACKAGE_DIR}}"
    SETTINGS_FILE="${CONFIG_DIR}/settings.env"
fi
if [ -n "${PREBUILT}" ] && [ ! -x "${PREBUILT}/bin/gucefMcpLocalLlm" ]; then
    echo "ERROR: ${PREBUILT}/bin/gucefMcpLocalLlm is not there: install the package gucef-mcp, or run this from a checkout."
    exit 1
fi
if [ -z "${SERVICE_USER}" ] || [ "${SERVICE_USER}" = "root" ]; then
    echo "ERROR: the service runs as a user, not root. Run with sudo from that user, or pass --user NAME."
    exit 1
fi
if ! id "${SERVICE_USER}" > /dev/null 2>&1; then
    echo "ERROR: there is no user ${SERVICE_USER}."
    exit 1
fi
SERVICE_GROUP="$(id -gn "${SERVICE_USER}")"
SERVICE_HOME="$(getent passwd "${SERVICE_USER}" | cut -d: -f6)"
as_user() { sudo -u "${SERVICE_USER}" -H "$@"; }

if [ ! -f "${SETTINGS_FILE}" ]; then
    install -d -m 755 "$(dirname "${SETTINGS_FILE}")"
    cp "${SCRIPT_DIR}/config.env.example" "${SETTINGS_FILE}"
    chown "${SERVICE_USER}:${SERVICE_GROUP}" "${SETTINGS_FILE}"
    echo "Created ${SETTINGS_FILE} (all settings commented out, defaults apply)."
fi
# shellcheck disable=SC1090
. "${SETTINGS_FILE}"
MCP_PORT="${MCP_PORT:-8790}"
MCP_TLS_PORT="${MCP_TLS_PORT:-8791}"
MCP_ALLOW_COMMANDS="${MCP_ALLOW_COMMANDS:-cmake --build *|ctest *|ninja *|make *}"
MCP_MODEL="${MCP_MODEL:-}"
MCP_REGISTER_CLAUDE="${MCP_REGISTER_CLAUDE:-true}"
MCP_CLAUDE_NAME="${MCP_CLAUDE_NAME:-local-llm}"

# ---------------------------------------------------------------------------
# The claude command of the user. The coding agent (local-agent.sh) runs it,
# and the registration below needs it. A user who has Claude Code only as an
# editor extension (VS Code and its forks keep a native binary inside the
# extension folder) has no such command, so a small wrapper in ~/.local/bin,
# which is on the PATH of the service, runs the newest one of those. A claude
# command that exists already is left alone.
# ---------------------------------------------------------------------------
CLAUDE_WRAPPER="${SERVICE_HOME}/.local/bin/claude"
CLAUDE_WRAPPER_MARK="# gucef-mcp: claude command"
if ! as_user bash -lc 'command -v claude' > /dev/null 2>&1 && [ ! -e "${CLAUDE_WRAPPER}" ]; then
    if compgen -G "${SERVICE_HOME}/.vscode*/extensions/anthropic.claude-code-*/resources/native-binary/claude" > /dev/null \
            || compgen -G "${SERVICE_HOME}/.cursor*/extensions/anthropic.claude-code-*/resources/native-binary/claude" > /dev/null \
            || compgen -G "${SERVICE_HOME}/.windsurf*/extensions/anthropic.claude-code-*/resources/native-binary/claude" > /dev/null; then
        install -d -m 755 -o "${SERVICE_USER}" -g "${SERVICE_GROUP}" "${SERVICE_HOME}/.local" "${SERVICE_HOME}/.local/bin"
        cat > "${CLAUDE_WRAPPER}" <<'WRAPPEREOF'
#!/bin/bash
# gucef-mcp: claude command
# Runs the Claude Code binary of the newest editor extension installed for this user, so that
# tools that need a "claude" command (the coding agent of the service gucef-mcp) find one.
# Written by llm/mcp/install.sh; delete this file if you install Claude Code another way.
BIN="$(ls -d "${HOME}"/.vscode*/extensions/anthropic.claude-code-*/resources/native-binary/claude \
              "${HOME}"/.cursor*/extensions/anthropic.claude-code-*/resources/native-binary/claude \
              "${HOME}"/.windsurf*/extensions/anthropic.claude-code-*/resources/native-binary/claude 2>/dev/null \
        | awk -F/ '{ v = $0; sub(/.*anthropic\.claude-code-/, "", v); sub(/\/.*/, "", v); print v "\t" $0 }' \
        | sort -V | tail -n 1 | cut -f2)"
[ -x "${BIN}" ] || { echo "ERROR: no Claude Code binary found in the editor extensions of ${HOME}" >&2; exit 127; }
exec "${BIN}" "$@"
WRAPPEREOF
        chown "${SERVICE_USER}:${SERVICE_GROUP}" "${CLAUDE_WRAPPER}"
        chmod 755 "${CLAUDE_WRAPPER}"
        echo "Created ${CLAUDE_WRAPPER}: Claude Code of ${SERVICE_USER} is an editor extension, this runs it as 'claude'."
    fi
fi

install -d -m 750 -o "${SERVICE_USER}" -g "${SERVICE_GROUP}" "${STATE_DIR}" "${STATE_DIR}/logs"
install -d -m 755 "${INSTALL_ROOT}" "${CONFIG_DIR}"
STAGE="$(mktemp -d "${INSTALL_ROOT}/.stage.XXXXXX")"
trap 'rm -rf "${STAGE}"' EXIT

if [ -n "${PREBUILT}" ]; then
    cp -a "${PREBUILT}/." "${STAGE}/" || exit 1
    SKIP_TESTS=true
else
    # -----------------------------------------------------------------------
    # Build tools
    # -----------------------------------------------------------------------
    MISSING_PACKAGES=()
    command -v cmake > /dev/null 2>&1 || MISSING_PACKAGES+=(cmake)
    command -v ninja > /dev/null 2>&1 || MISSING_PACKAGES+=(ninja-build)
    command -v g++ > /dev/null 2>&1 || MISSING_PACKAGES+=(build-essential)
    command -v openssl > /dev/null 2>&1 || MISSING_PACKAGES+=(openssl)
    command -v curl > /dev/null 2>&1 || MISSING_PACKAGES+=(curl)
    command -v strip > /dev/null 2>&1 || MISSING_PACKAGES+=(binutils)
    [ -e /usr/lib/x86_64-linux-gnu/libssl.so.3 ] || ls /usr/lib/*/libssl.so.3 > /dev/null 2>&1 || MISSING_PACKAGES+=(libssl3)
    if [ ${#MISSING_PACKAGES[@]} -gt 0 ]; then
        if command -v apt-get > /dev/null 2>&1; then
            echo "Installing ${MISSING_PACKAGES[*]}"
            apt-get update -q && DEBIAN_FRONTEND=noninteractive apt-get install -y -q "${MISSING_PACKAGES[@]}" || exit 1
        else
            echo "ERROR: install these first: ${MISSING_PACKAGES[*]}"
            exit 1
        fi
    fi

    # -----------------------------------------------------------------------
    # Build
    # -----------------------------------------------------------------------
    install -d -m 755 -o "${SERVICE_USER}" -g "${SERVICE_GROUP}" "${BUILD_DIR}"
    echo "Building gucefMcpLocalLlm from ${GUCEF_ROOT}"
    # relocatable: the installed copy finds its libraries next to itself, never in the build tree
    # the generated CMake files of GUCEF ask for CMake 2.6; CMake 4 refuses that without a policy minimum
    as_user cmake -S "${GUCEF_ROOT}/projects/CMake/targets/GUCEF_exe_gucefMcpLocalLlm_TestApp" -B "${BUILD_DIR}" -G Ninja \
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON '-DCMAKE_INSTALL_RPATH=$ORIGIN/../lib' \
        > "${STATE_DIR}/logs/build.log" 2>&1 &&
    as_user cmake --build "${BUILD_DIR}" >> "${STATE_DIR}/logs/build.log" 2>&1
    if [ $? -ne 0 ]; then
        grep -E 'error|Error' "${STATE_DIR}/logs/build.log" | head -20
        echo "ERROR: the build failed, see ${STATE_DIR}/logs/build.log. The installed version stays."
        exit 1
    fi

    # the files of an installation: the server, the libraries it and its JSON plugin load from the build, the agent client
    "${GUCEF_ROOT}/projects/k3s/cluster-node-installation-media/stage.sh" \
        --doc "${SCRIPT_DIR}/README.md" --doc "${GUCEF_ROOT}/tools/gucefMcpLocalLlm/README.md:gucefMcpLocalLlm.md" \
        "${BUILD_DIR}" "${STAGE}" gucefMcpLocalLlm libdstorepluginJSONPARSER.so || exit 1
    install -m 755 "${GUCEF_ROOT}/projects/k3s/llm/clients/local-agent.sh" "${STAGE}/bin/"
fi
chmod -R a+rX,go-w "${STAGE}"
VERSION="$(cd "${STAGE}" && find . -type f -print0 | sort -z | xargs -0 sha256sum | sha256sum | cut -c1-12)"
CURRENT_VERSION="$(basename "$(readlink "${INSTALL_ROOT}/current" 2>/dev/null)")"

if [ "${VERSION}" = "${CURRENT_VERSION}" ]; then
    echo "Version ${VERSION} is installed already."
else
    if [ "${SKIP_TESTS}" != "true" ]; then
        echo "Testing version ${VERSION}"
        # no console logger: without it the test application opens a terminal window of the desktop
        TEST_ENV=(GUCEF_CONSOLE_LOGGER=none LOCAL_AGENT_SCRIPT="${GUCEF_ROOT}/projects/k3s/llm/clients/local-agent.sh"
                  PATH="${SERVICE_HOME}/.local/bin:${SERVICE_HOME}/.opencode/bin:${SERVICE_HOME}/bin:${PATH}")
        FIRST_ENDPOINT="$(grep -v '^#' "${STATE_DIR}/endpoints" 2>/dev/null | sed 's/^ephemeral //' | awk 'NF {print $1; exit}')"
        [ -n "${FIRST_ENDPOINT}" ] && TEST_ENV+=(LOCAL_LLM_URL="${FIRST_ENDPOINT}")
        if ! ( cd "${BUILD_DIR}/bin" && as_user env "${TEST_ENV[@]}" ./gucefMcpLocalLlm_TestApp > "${STATE_DIR}/logs/test.log" 2>&1 ); then
            python3 - "${BUILD_DIR}/bin/gucefMcpLocalLlm_TestApp_Results.xml" <<'PYEOF' 2>/dev/null
import sys, xml.etree.ElementTree as ET
for case in ET.parse(sys.argv[1]).getroot().iter("testcase"):
    for result in case:
        if result.tag == "failure":
            print("  failed: " + case.get("name", ""))
PYEOF
            echo "ERROR: the tests failed, see ${STATE_DIR}/logs/test.log. The installed version stays."
            exit 1
        fi
        echo "Tests passed."
    fi
    mv "${STAGE}" "${INSTALL_ROOT}/${VERSION}"
    trap - EXIT
    ln -sfn "${VERSION}" "${INSTALL_ROOT}/current.new"
    mv -T "${INSTALL_ROOT}/current.new" "${INSTALL_ROOT}/current"
    echo "Installed version ${VERSION} in ${INSTALL_ROOT}/${VERSION}"
    # the newest versions stay, for sessions that started on them and to go back
    ls -1dt "${INSTALL_ROOT}"/*/ 2>/dev/null | grep -v '/current/$' | tail -n +$(( KEEP_VERSIONS + 1 )) | while read -r old; do
        [ "$(basename "${old}")" != "${VERSION}" ] && rm -rf "${old}"
    done
fi

# ---------------------------------------------------------------------------
# Service
# ---------------------------------------------------------------------------
CONFIG_FILE="${CONFIG_DIR}/config.env"
NEW_CONFIG="$(cat <<EOF
# written by llm/mcp/install.sh from ${SCRIPT_DIR}/config.env
MCP_PORT="${MCP_PORT}"
MCP_TLS_PORT="${MCP_TLS_PORT}"
MCP_ALLOW_COMMANDS="${MCP_ALLOW_COMMANDS}"
MCP_MODEL="${MCP_MODEL}"
EOF
)"
CONFIG_CHANGED=false
if [ "$(cat "${CONFIG_FILE}" 2>/dev/null)" != "${NEW_CONFIG}" ]; then
    echo "${NEW_CONFIG}" > "${CONFIG_FILE}"
    CONFIG_CHANGED=true
fi

cat > "${INSTALL_ROOT}/run.sh" <<'EOF'
#!/bin/bash
# Starts the MCP server of the service gucef-mcp, written by llm/mcp/install.sh
. /etc/gucef-mcp/config.env
STATE_DIR=/var/lib/gucef-mcp
TLS_DIR="${STATE_DIR}/tls"
# the coding agent and the build tools of the user
export PATH="${HOME}/.local/bin:${HOME}/.opencode/bin:${HOME}/bin:${PATH}"
[ -s "${STATE_DIR}/llm-api-key" ] && LOCAL_LLM_API_KEY="$(cat "${STATE_DIR}/llm-api-key")" && export LOCAL_LLM_API_KEY

# the lab MCP token of the cluster, the same on every node; a token of this node until it is there
TOKEN_FILE="${STATE_DIR}/token"
if [ -s "${STATE_DIR}/lab-token" ]; then
    TOKEN_FILE="${STATE_DIR}/lab-token"
    rm -f "${STATE_DIR}/own-token"
else
    touch "${STATE_DIR}/own-token"
fi
ARGS=(--listen "127.0.0.1:${MCP_PORT}" --token-file "${TOKEN_FILE}" --endpoints-file "${STATE_DIR}/endpoints"
      --agent-script /opt/gucef-mcp/current/bin/local-agent.sh --log-dir "${STATE_DIR}/logs/commands")
IFS='|' read -r -a COMMANDS <<< "${MCP_ALLOW_COMMANDS}"
for command in "${COMMANDS[@]}"; do
    [ -n "${command}" ] && ARGS+=(--allow-command "${command}")
done
[ -n "${MCP_MODEL}" ] && ARGS+=(--model "${MCP_MODEL}")

if [ -s "${TLS_DIR}/tls.crt" ] && [ -s "${TLS_DIR}/tls.key" ]; then
    ARGS+=(--listen-tls "0.0.0.0:${MCP_TLS_PORT}" --tls-cert "${TLS_DIR}/tls.crt" --tls-key "${TLS_DIR}/tls.key")
    # the names clients on other machines use for this one
    HOST="$(hostname)"
    for name in "${HOST}" "${HOST,,}" "$(hostname -f 2>/dev/null)" $(hostname -I 2>/dev/null); do
        [ -n "${name}" ] && ARGS+=(--allowed-host "${name}")
    done
    rm -f "${STATE_DIR}/no-tls"
else
    touch "${STATE_DIR}/no-tls"
fi
exec /opt/gucef-mcp/current/bin/gucefMcpLocalLlm "${ARGS[@]}"
EOF
chmod 755 "${INSTALL_ROOT}/run.sh"

UNIT_CHANGED=false
write_unit() {
    local file="$1" content="$2"
    if [ "$(cat "${file}" 2>/dev/null)" != "${content}" ]; then
        echo "${content}" > "${file}"
        UNIT_CHANGED=true
    fi
}
write_unit "${UNIT_DIR}/${SERVICE}.service" "$(cat <<EOF
[Unit]
Description=GUCEF MCP server: local LLM tools for Claude sessions
After=network-online.target
Wants=network-online.target

[Service]
User=${SERVICE_USER}
Group=${SERVICE_GROUP}
WorkingDirectory=${SERVICE_HOME}
ExecStart=${INSTALL_ROOT}/run.sh
Restart=on-failure
RestartSec=5
KillSignal=SIGTERM
TimeoutStopSec=30

[Install]
WantedBy=multi-user.target
EOF
)"
# HTTPS starts once the certificate of the node is there, the lab MCP token is used once it is there
write_unit "${UNIT_DIR}/${SERVICE}-tls.path" "$(cat <<EOF
[Unit]
Description=Restarts gucef-mcp for the certificate of the node and the lab MCP token once they are there

[Path]
PathChanged=${STATE_DIR}/tls/tls.key
PathChanged=${STATE_DIR}/lab-token
Unit=${SERVICE}-tls.service

[Install]
WantedBy=multi-user.target
EOF
)"
write_unit "${UNIT_DIR}/${SERVICE}-tls.service" "$(cat <<EOF
[Unit]
Description=Restarts gucef-mcp when it runs without the certificate or the lab MCP token and they are there

[Service]
Type=oneshot
ExecStart=/bin/sh -c 'if { [ -e ${STATE_DIR}/no-tls ] && [ -s ${STATE_DIR}/tls/tls.key ]; } || { [ -e ${STATE_DIR}/own-token ] && [ -s ${STATE_DIR}/lab-token ]; }; then systemctl try-restart ${SERVICE}.service; fi'
EOF
)"

systemctl enable --quiet "${SERVICE}.service" "${SERVICE}-tls.path"
[ "${UNIT_CHANGED}" = "true" ] && systemctl daemon-reload
# a path unit watches the paths it had when it started: it starts again so that it watches those of its definition
systemctl stop "${SERVICE}-tls.service" > /dev/null 2>&1
systemctl restart "${SERVICE}-tls.path"
# a service that runs with a token of its own while the lab MCP token is there, or without HTTPS while the
# certificate is there, starts again to take them
STALE=false
{ [ -e "${STATE_DIR}/own-token" ] && [ -s "${STATE_DIR}/lab-token" ]; } && STALE=true
{ [ -e "${STATE_DIR}/no-tls" ] && [ -s "${STATE_DIR}/tls/tls.key" ]; } && STALE=true
if ! systemctl is-active --quiet "${SERVICE}.service"; then
    systemctl start "${SERVICE}.service"
elif [ "${VERSION}" != "${CURRENT_VERSION}" ] || [ "${CONFIG_CHANGED}" = "true" ] || [ "${UNIT_CHANGED}" = "true" ] || [ "${STALE}" = "true" ]; then
    echo "Restarting ${SERVICE}: work that runs on it now is ended."
    systemctl restart "${SERVICE}.service"
fi

# ready when the local listener answers
for _ in $(seq 1 60); do
    { [ -s "${STATE_DIR}/token" ] || [ -s "${STATE_DIR}/lab-token" ]; } && curl -s -o /dev/null -m 2 "http://127.0.0.1:${MCP_PORT}/mcp" && break
    sleep 1
done
TOKEN="$(cat "${STATE_DIR}/lab-token" 2>/dev/null)"
[ -n "${TOKEN}" ] || TOKEN="$(cat "${STATE_DIR}/token" 2>/dev/null)"
INIT='{"jsonrpc":"2.0","id":1,"method":"initialize","params":{"protocolVersion":"2025-11-25","capabilities":{},"clientInfo":{"name":"install.sh","version":"1"}}}'
if [ -n "${TOKEN}" ] && curl -s -m 30 -H "Authorization: Bearer ${TOKEN}" -H "Content-Type: application/json" \
        -H "Accept: application/json, text/event-stream" -d "${INIT}" "http://127.0.0.1:${MCP_PORT}/mcp" | grep -q '"result"'; then
    echo "${SERVICE} answers on http://127.0.0.1:${MCP_PORT}/mcp"
else
    echo "ERROR: ${SERVICE} does not answer. See: journalctl -u ${SERVICE} -n 50"
    exit 1
fi

# ---------------------------------------------------------------------------
# Claude Code of the user
# ---------------------------------------------------------------------------
CLAUDE_BIN="$(as_user bash -lc 'command -v claude' 2>/dev/null)"
LOCAL_URL="http://127.0.0.1:${MCP_PORT}/mcp"
if [ "${MCP_REGISTER_CLAUDE}" = "true" ] && [ -n "${CLAUDE_BIN}" ]; then
    # registered again each time: the token may have changed
    as_user "${CLAUDE_BIN}" mcp remove "${MCP_CLAUDE_NAME}" --scope user > /dev/null 2>&1
    if as_user "${CLAUDE_BIN}" mcp add --transport http "${MCP_CLAUDE_NAME}" "${LOCAL_URL}" --scope user \
            --header "Authorization: Bearer ${TOKEN}" > /dev/null; then
        echo "Registered ${MCP_CLAUDE_NAME} with Claude Code of ${SERVICE_USER}; sessions started from now on have it."
    else
        echo "WARNING: registering with Claude Code failed."
    fi
fi

HOST="$(hostname)"
echo ""
echo "gucef-mcp ${VERSION} runs as ${SERVICE_USER}."
if [ "${MCP_REGISTER_CLAUDE}" != "true" ] || [ -z "${CLAUDE_BIN}" ]; then
    echo "  Local Claude Code:"
    echo "    claude mcp add --transport http ${MCP_CLAUDE_NAME} ${LOCAL_URL} --scope user --header \"Authorization: Bearer \$(sudo cat ${STATE_DIR}/token)\""
fi
if [ -s "${STATE_DIR}/tls/tls.crt" ]; then
    echo "  From another machine (it trusts ${STATE_DIR}/tls/ca.crt of this one, copied there):"
    echo "    export NODE_EXTRA_CA_CERTS=/path/to/gucef-mcp-ca.crt    (before starting claude)"
    echo "    claude mcp add --transport http ${HOST,,}-llm https://${HOST}:${MCP_TLS_PORT}/mcp --scope user --header \"Authorization: Bearer <token of ${STATE_DIR}/token>\""
else
    echo "  HTTPS for other machines starts once this node's certificate is there: run llm/mcp/cluster.sh on a server"
    echo "  (llm/setup.sh does) after this machine joined the cluster."
fi
