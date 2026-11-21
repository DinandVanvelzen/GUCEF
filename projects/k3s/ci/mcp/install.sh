#!/bin/bash
# ---------------------------------------------------------------------------
# Builds, tests and installs the CI MCP server gucefMcpCi on this machine (see
# tools/gucefMcpCi/README.md and README.md next to this script), in one of two
# roles:
#
#   client   any machine of the lab (the default when this is no k3s server):
#            the program in /opt/gucef-mcp-ci/<version>, /opt/gucef-mcp-ci/current
#            points to the one in use, and the registration with Claude Code of
#            the user that ran sudo as the stdio server CI_MCP_NAME (default ci):
#              gucefMcpCi --stdio --mode auto
#            Claude Code starts the program itself; no service runs. In auto mode
#            it takes the gateway URL, its token and the authority of the lab from
#            /var/lib/gucef-mcp/ci, which lablink writes once the hub approved the
#            machine. Without lablink, CI_GATEWAY_URL, CI_GATEWAY_TOKEN_FILE and
#            CI_CA_FILE of config.env are passed as options.
#   gateway  the k3s server node, next to the lablink hub (the default when
#            /etc/systemd/system/k3s.service exists): everything of the client
#            role, and the systemd service gucef-ci-gateway, which serves the
#            gateway API on https://<this machine>:8792 for the proxies of the lab:
#              - the system user gucef-ci, /var/lib/gucef-ci and /etc/gucef-ci
#              - the CI registry /etc/gucef-ci/ci.json, made from
#                projects/k3s/lablink/ci.json.example when missing, never replaced;
#                the lablink hub serves the same file (LAB_CI_REGISTRY)
#              - the hashes of the gateway tokens the hub makes for the approved
#                machines: /var/lib/gucef-ci/tokens/gateway-tokens
#                (LAB_CI_GATEWAY_TOKENS, LAB_CI_GATEWAY_USER of the hub)
#              - the Forgejo token of the bot user ci-bot,
#                /var/lib/gucef-ci/forgejo.token, made by
#                projects/k3s/forgejo/scratch-repo.sh --gateway-token when missing
#              - the certificate of this node (llm/mcp/cluster.sh), copied to
#                /var/lib/gucef-ci/tls and again when it is renewed
#                (gucef-ci-gateway-tls.path)
#              - 8792/tcp open to the lab networks (host-node/open_firewall_ports.sh)
#              - deployments: when /var/lib/gucef-ci/kubeconfig exists (made by
#                projects/k3s/ci/deployer/setup.sh, the restricted ServiceAccounts)
#                and kubectl (or k3s) is installed, the gateway gets --kubeconfig and
#                --kubectl; which machine may deploy is the scope of its gateway
#                token (CI_DEPLOY_MACHINES, LAB_CI_DEPLOY_MACHINES of the hub)
#
# Idempotent: a re-run builds what changed, and installs, registers and
# restarts only when the result or the settings differ.
#
# Without a checkout around this script, it installs the built files of
# /usr/lib/gucef-mcp-ci instead of building, and its settings are
# /etc/gucef-ci/settings.env.
#
# Usage: sudo ./install.sh [--role gateway|client] [--prebuilt DIR] [--skip-tests] [--upgrade] [--dry-run]
#   --role ROLE     gateway or client; default CI_ROLE of config.env, else detected
#   --prebuilt DIR  install the built files in DIR (bin/, lib/) instead of building
#   --skip-tests    install without running the tests
#   --upgrade       only when it is installed already: install the new files and
#                   restart the gateway, keep everything else (used by a package)
#   --dry-run       print what would be done and change nothing
# Settings: config.env next to this script (see config.env.example)
# ---------------------------------------------------------------------------

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
K3S_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"
GUCEF_ROOT="$(cd "${K3S_DIR}/../.." && pwd)"
INSTALL_ROOT="/opt/gucef-mcp-ci"
CACHE_DIR="/var/cache/gucef-mcp-ci"
CONFIG_DIR="/etc/gucef-ci"
STATE_DIR="/var/lib/gucef-ci"
UNIT_DIR="/etc/systemd/system"
PACKAGE_DIR="/usr/lib/gucef-mcp-ci"
MCP_DIR="/var/lib/gucef-mcp"
LABLINK_CONFIG_DIR="/etc/gucef-lablink"
KUBECONFIG_FILE="/etc/rancher/k3s/k3s.yaml"
SERVICE="gucef-ci-gateway"
LABLINK_SERVICE="gucef-lablink"
KEEP_VERSIONS=3
SETTING_MARK="# set by ci/mcp/install.sh"

BUILD_DIR="${CACHE_DIR}/build"
LOG_DIR="${CACHE_DIR}/logs"
REGISTRY_FILE="${CONFIG_DIR}/ci.json"
INSTALL_RECORD="${CONFIG_DIR}/install.env"
GATEWAY_ENV="${CONFIG_DIR}/gateway.env"
TLS_DIR="${STATE_DIR}/tls"
TOKENS_DIR="${STATE_DIR}/tokens"
GATEWAY_TOKENS_FILE="${TOKENS_DIR}/gateway-tokens"
FORGEJO_TOKEN_FILE="${STATE_DIR}/forgejo.token"
DEPLOY_KUBECONFIG="${STATE_DIR}/kubeconfig"
KUBECTL_WRAPPER="${INSTALL_ROOT}/kubectl"
RUN_SCRIPT="${INSTALL_ROOT}/gateway-run.sh"
TLS_SYNC_SCRIPT="${INSTALL_ROOT}/gateway-tls-sync.sh"
CI_PROGRAM="${INSTALL_ROOT}/current/bin/gucefMcpCi"

usage()
{
    echo "Usage: sudo $0 [--role gateway|client] [--prebuilt DIR] [--skip-tests] [--upgrade] [--dry-run]"
    exit "${1:-1}"
}

ROLE=""
PREBUILT=""
SKIP_TESTS=false
UPGRADE=false
DRY_RUN=false
while [ $# -gt 0 ]; do
    case "$1" in
        --role) [ $# -ge 2 ] || usage; ROLE="$2"; shift ;;
        --prebuilt) [ $# -ge 2 ] || usage; PREBUILT="$2"; shift ;;
        --skip-tests) SKIP_TESTS=true ;;
        --upgrade) UPGRADE=true ;;
        --dry-run) DRY_RUN=true ;;
        -h|--help) usage 0 ;;
        *) echo "Unknown option: $1"; usage ;;
    esac
    shift
done

dry() { [ "${DRY_RUN}" = "true" ]; }
# runs a command that changes the system; in a dry run it is only printed
run()
{
    if dry; then
        printf 'DRY-RUN:'
        printf ' %q' "$@"
        printf '\n'
        return 0
    fi
    "$@"
}

if ! dry && [ "$(id -u)" -ne 0 ]; then
    echo "ERROR: run with sudo: it installs into /opt and /etc, and a systemd service in the gateway role."
    exit 1
fi
dry && echo "DRY-RUN: nothing is changed; checks that need root may be incomplete without sudo."

# an upgrade keeps the role, the user and the registration of the installation
if [ "${UPGRADE}" = "true" ]; then
    if [ ! -L "${INSTALL_ROOT}/current" ] || [ ! -f "${INSTALL_RECORD}" ]; then
        exit 0
    fi
    CI_INSTALLED_ROLE=""
    # shellcheck disable=SC1090
    . "${INSTALL_RECORD}"
    ROLE="${CI_INSTALLED_ROLE}"
fi

SETTINGS_FILE="${SCRIPT_DIR}/config.env"
CHECKOUT=true
if [ ! -d "${GUCEF_ROOT}/projects/CMake/targets/GUCEF_exe_gucefMcpCi_TestApp" ]; then
    CHECKOUT=false
    PREBUILT="${PREBUILT:-${PACKAGE_DIR}}"
    SETTINGS_FILE="${CONFIG_DIR}/settings.env"
fi
if [ -n "${PREBUILT}" ] && [ ! -x "${PREBUILT}/bin/gucefMcpCi" ]; then
    echo "ERROR: ${PREBUILT}/bin/gucefMcpCi is not there: pass --prebuilt with a built gucefMcpCi, or run this from a checkout."
    exit 1
fi

BUILD_USER="${SUDO_USER:-root}"
CLAUDE_USER="${SUDO_USER:-}"
[ "${UPGRADE}" = "true" ] && CLAUDE_USER="${CI_INSTALLED_USER:-}"
[ "${CLAUDE_USER}" = "root" ] && CLAUDE_USER=""
as_builder() { if [ "${BUILD_USER}" = "root" ]; then "$@"; else sudo -u "${BUILD_USER}" -H "$@"; fi; }
as_claude_user() { sudo -u "${CLAUDE_USER}" -H "$@"; }

if [ ! -f "${SETTINGS_FILE}" ]; then
    if dry; then
        echo "DRY-RUN: create ${SETTINGS_FILE} from ${SCRIPT_DIR}/config.env.example"
    else
        install -d -m 755 "$(dirname "${SETTINGS_FILE}")"
        cp "${SCRIPT_DIR}/config.env.example" "${SETTINGS_FILE}"
        [ "${BUILD_USER}" != "root" ] && [ "${CHECKOUT}" = "true" ] && chown "${BUILD_USER}" "${SETTINGS_FILE}"
        echo "Created ${SETTINGS_FILE} (all settings commented out, defaults apply)."
    fi
fi
if [ -f "${SETTINGS_FILE}" ]; then
    # shellcheck disable=SC1090
    . <(sed 's/\r$//' "${SETTINGS_FILE}")
fi
CI_ROLE="${CI_ROLE:-}"
CI_MCP_NAME="${CI_MCP_NAME:-ci}"
CI_GATEWAY_URL="${CI_GATEWAY_URL:-}"
CI_GATEWAY_TOKEN_FILE="${CI_GATEWAY_TOKEN_FILE:-}"
CI_CA_FILE="${CI_CA_FILE:-}"
CI_GATEWAY_PORT="${CI_GATEWAY_PORT:-8792}"
CI_GATEWAY_USER="${CI_GATEWAY_USER:-gucef-ci}"
CI_REGISTER_CLAUDE="${CI_REGISTER_CLAUDE:-true}"
CI_ALLOW="${CI_ALLOW:-}"
# unset: the deploy machines of the hub stay as its settings have them; set (also empty): they become this list
CI_DEPLOY_MACHINES_SET=false
if [ -n "${CI_DEPLOY_MACHINES+set}" ]; then
    CI_DEPLOY_MACHINES_SET=true
    CI_DEPLOY_MACHINES="$(printf '%s' "${CI_DEPLOY_MACHINES}" | tr -d '[:space:]')"
fi
[ "${UPGRADE}" = "true" ] && CI_MCP_NAME="${CI_INSTALLED_NAME:-${CI_MCP_NAME}}" && CI_GATEWAY_USER="${CI_INSTALLED_GATEWAY_USER:-${CI_GATEWAY_USER}}"

[ -n "${ROLE}" ] || ROLE="${CI_ROLE}"
if [ -z "${ROLE}" ]; then
    if [ -f "${UNIT_DIR}/k3s.service" ]; then ROLE="gateway"; else ROLE="client"; fi
fi
case "${ROLE}" in
    gateway|client) ;;
    *) echo "ERROR: the role is gateway or client, not ${ROLE}."; exit 1 ;;
esac
if ! [[ "${CI_MCP_NAME}" =~ ^[A-Za-z0-9_-]+$ ]]; then
    echo "ERROR: CI_MCP_NAME=${CI_MCP_NAME}: letters, digits, - and _ only."
    exit 1
fi
if ! [[ "${CI_GATEWAY_PORT}" =~ ^[0-9]+$ ]] || [ "${CI_GATEWAY_PORT}" -lt 1 ] || [ "${CI_GATEWAY_PORT}" -gt 65535 ]; then
    echo "ERROR: CI_GATEWAY_PORT=${CI_GATEWAY_PORT} is no port."
    exit 1
fi
if ! [[ "${CI_GATEWAY_USER}" =~ ^[a-z_][a-z0-9_-]*$ ]] || [ "${CI_GATEWAY_USER}" = "root" ]; then
    echo "ERROR: CI_GATEWAY_USER=${CI_GATEWAY_USER} is no name for a system user (and not root)."
    exit 1
fi
if [ "${CI_DEPLOY_MACHINES_SET}" = "true" ] && ! [[ "${CI_DEPLOY_MACHINES}" =~ ^[A-Za-z0-9._:,-]*$ ]]; then
    echo "ERROR: CI_DEPLOY_MACHINES=${CI_DEPLOY_MACHINES}: machine names or ids separated by commas only."
    exit 1
fi
if [ -n "${CI_GATEWAY_URL}" ] && ! [[ "${CI_GATEWAY_URL}" =~ ^https?://[A-Za-z0-9._:-]+(/[^[:space:]\"\']*)?$ ]]; then
    echo "ERROR: CI_GATEWAY_URL=${CI_GATEWAY_URL} is no URL of the form https://host:port."
    exit 1
fi
echo "Role: ${ROLE}"
# the registration of the previous install, which goes when the name or the user changed
PREVIOUS_NAME="$(sed -n 's/^CI_INSTALLED_NAME="\(.*\)"$/\1/p' "${INSTALL_RECORD}" 2>/dev/null)"
PREVIOUS_USER="$(sed -n 's/^CI_INSTALLED_USER="\(.*\)"$/\1/p' "${INSTALL_RECORD}" 2>/dev/null)"

# ---------------------------------------------------------------------------
# Helpers for files: written when their content, mode or owner differ
# ---------------------------------------------------------------------------
FILE_CHANGED=false
# put_file FILE MODE OWNER:GROUP CONTENT
put_file()
{
    local file="$1" mode="$2" owner="$3" content="$4" temporary
    FILE_CHANGED=false
    if [ -f "${file}" ] && [ "$(cat "${file}" 2>/dev/null)" = "${content}" ] && \
       [ "$(stat -c '%a %U:%G' "${file}" 2>/dev/null)" = "${mode} ${owner}" ]; then
        return 0
    fi
    FILE_CHANGED=true
    if dry; then
        echo "DRY-RUN: write ${file} (mode ${mode}, owner ${owner})"
        return 0
    fi
    temporary="$(mktemp "${file}.XXXXXX")" || return 1
    if printf '%s\n' "${content}" > "${temporary}" && chmod "${mode}" "${temporary}" && chown "${owner}" "${temporary}" && \
       mv -f "${temporary}" "${file}"; then
        return 0
    fi
    rm -f "${temporary}"
    return 1
}
# ensure_dir DIR MODE OWNER GROUP
ensure_dir()
{
    local dir="$1" mode="$2" owner="$3" group="$4"
    [ -d "${dir}" ] && [ ! -L "${dir}" ] && [ "$(stat -c '%a %U:%G' "${dir}" 2>/dev/null)" = "${mode} ${owner}:${group}" ] && return 0
    run install -d -m "${mode}" -o "${owner}" -g "${group}" "${dir}"
}

TOKEN_WORK=""
STAGE=""
cleanup()
{
    [ -n "${STAGE}" ] && rm -rf "${STAGE}"
    [ -n "${TOKEN_WORK}" ] && rm -rf "${TOKEN_WORK}"
}
trap cleanup EXIT

# ---------------------------------------------------------------------------
# Tools the program uses at run time
# ---------------------------------------------------------------------------
if ! command -v git > /dev/null 2>&1; then
    if [ "${ROLE}" = "gateway" ]; then
        echo "WARNING: git is not installed. The gateway needs it for every snapshot (git bundle, fetch, push): install git."
    else
        echo "WARNING: git is not installed. gucefMcpCi needs it to make snapshots of a repository: install git."
    fi
fi

# ---------------------------------------------------------------------------
# Build, test, install the files
# ---------------------------------------------------------------------------
run install -d -m 755 "${INSTALL_ROOT}"
VERSION=""
CURRENT_VERSION="$(basename "$(readlink "${INSTALL_ROOT}/current" 2>/dev/null)" 2>/dev/null)"
if dry; then
    if [ -n "${PREBUILT}" ]; then
        echo "DRY-RUN: copy ${PREBUILT} to a new version in ${INSTALL_ROOT} when it differs from the installed one (${CURRENT_VERSION:-none})"
    else
        echo "DRY-RUN: as ${BUILD_USER}: cmake -S ${GUCEF_ROOT}/projects/CMake/targets/GUCEF_exe_gucefMcpCi -B ${BUILD_DIR}/app (Release, relocatable)"
        echo "DRY-RUN: as ${BUILD_USER}: cmake --build ${BUILD_DIR}/app --target gucefMcpCi dstorepluginJSONPARSER codecspluginZLIB"
        if [ "${SKIP_TESTS}" != "true" ]; then
            echo "DRY-RUN: as ${BUILD_USER}: cmake -S ${GUCEF_ROOT}/projects/CMake/targets/GUCEF_exe_gucefMcpCi_TestApp -B ${BUILD_DIR}/test, --target gucefMcpCi_TestApp and the plugins"
            echo "DRY-RUN: as ${BUILD_USER}: run gucefMcpCi_TestApp with the gucefMcpCi of ${BUILD_DIR}/app"
        fi
        echo "DRY-RUN: stage gucefMcpCi and its libraries (stage.sh), install as a new version in ${INSTALL_ROOT} when it differs from the installed one (${CURRENT_VERSION:-none})"
    fi
    VERSION="${CURRENT_VERSION}"
else
    STAGE="$(mktemp -d "${INSTALL_ROOT}/.stage.XXXXXX")" || exit 1
    if [ -n "${PREBUILT}" ]; then
        cp -a "${PREBUILT}/." "${STAGE}/" || exit 1
        SKIP_TESTS=true
    else
        MISSING_PACKAGES=()
        command -v cmake > /dev/null 2>&1 || MISSING_PACKAGES+=(cmake)
        command -v ninja > /dev/null 2>&1 || MISSING_PACKAGES+=(ninja-build)
        command -v g++ > /dev/null 2>&1 || MISSING_PACKAGES+=(build-essential)
        command -v python3 > /dev/null 2>&1 || MISSING_PACKAGES+=(python3)
        command -v strip > /dev/null 2>&1 || MISSING_PACKAGES+=(binutils)
        if [ ${#MISSING_PACKAGES[@]} -gt 0 ]; then
            if command -v apt-get > /dev/null 2>&1; then
                echo "Installing ${MISSING_PACKAGES[*]}"
                apt-get update -q && DEBIAN_FRONTEND=noninteractive apt-get install -y -q "${MISSING_PACKAGES[@]}" || exit 1
            else
                echo "ERROR: install these first: ${MISSING_PACKAGES[*]}"
                exit 1
            fi
        fi

        install -d -m 755 "${CACHE_DIR}" "${LOG_DIR}"
        install -d -m 755 -o "${BUILD_USER}" "${BUILD_DIR}"
        echo "Building gucefMcpCi from ${GUCEF_ROOT}"
        # relocatable: the installed copy finds its libraries next to itself, never in the build tree
        # the generated CMake files of GUCEF ask for CMake 2.6; CMake 4 refuses that without a policy minimum
        PLUGINS="dstorepluginJSONPARSER codecspluginZLIB"
        CMAKE_ARGS=(-G Ninja -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON '-DCMAKE_INSTALL_RPATH=$ORIGIN/../lib')
        # the program and the tests are two projects of GUCEF: the target of the one is unknown in the other
        BUILD_OK=false
        if as_builder cmake -S "${GUCEF_ROOT}/projects/CMake/targets/GUCEF_exe_gucefMcpCi" -B "${BUILD_DIR}/app" "${CMAKE_ARGS[@]}" \
                > "${LOG_DIR}/build.log" 2>&1 &&
            as_builder cmake --build "${BUILD_DIR}/app" --target gucefMcpCi ${PLUGINS} >> "${LOG_DIR}/build.log" 2>&1; then
            BUILD_OK=true
            if [ "${SKIP_TESTS}" != "true" ]; then
                BUILD_OK=false
                # the tests start the program next to them: the one that is installed
                as_builder cmake -S "${GUCEF_ROOT}/projects/CMake/targets/GUCEF_exe_gucefMcpCi_TestApp" -B "${BUILD_DIR}/test" "${CMAKE_ARGS[@]}" \
                    >> "${LOG_DIR}/build.log" 2>&1 &&
                as_builder cmake --build "${BUILD_DIR}/test" --target gucefMcpCi_TestApp ${PLUGINS} >> "${LOG_DIR}/build.log" 2>&1 &&
                as_builder cp "${BUILD_DIR}/app/bin/gucefMcpCi" "${BUILD_DIR}/test/bin/gucefMcpCi" && BUILD_OK=true
            fi
        fi
        if [ "${BUILD_OK}" != "true" ]; then
            grep -E 'error' "${LOG_DIR}/build.log" | head -20
            echo "ERROR: the build failed, see ${LOG_DIR}/build.log. The installed version stays."
            exit 1
        fi
        # the program, the plugins it loads (JSON codec, gzip codec for the logs of jobs) and the libraries they need
        "${K3S_DIR}/cluster-node-installation-media/stage.sh" \
            --doc "${SCRIPT_DIR}/README.md" --doc "${GUCEF_ROOT}/tools/gucefMcpCi/README.md:gucefMcpCi.md" \
            "${BUILD_DIR}/app" "${STAGE}" gucefMcpCi libdstorepluginJSONPARSER.so libcodecspluginZLIB.so || exit 1
    fi
    chmod -R a+rX,go-w "${STAGE}"
    VERSION="$(cd "${STAGE}" && find . -type f -print0 | sort -z | xargs -0 sha256sum | sha256sum | cut -c1-12)"

    if [ "${VERSION}" = "${CURRENT_VERSION}" ]; then
        echo "Version ${VERSION} is installed already."
    else
        if [ "${SKIP_TESTS}" != "true" ]; then
            echo "Testing version ${VERSION}"
            # no console logger: without it the test application opens a terminal window of the desktop
            if ! ( cd "${BUILD_DIR}/test/bin" && as_builder env GUCEF_CONSOLE_LOGGER=none ./gucefMcpCi_TestApp > "${LOG_DIR}/test.log" 2>&1 ); then
                python3 - "${BUILD_DIR}/test/bin/gucefMcpCi_TestApp_Results.xml" <<'PYEOF' 2>/dev/null
import sys, xml.etree.ElementTree as ET
for case in ET.parse(sys.argv[1]).getroot().iter("testcase"):
    for result in case:
        if result.tag == "failure":
            print("  failed: " + case.get("name", ""))
PYEOF
                echo "ERROR: the tests failed, see ${LOG_DIR}/test.log. The installed version stays."
                exit 1
            fi
            echo "Tests passed."
        fi
        mv "${STAGE}" "${INSTALL_ROOT}/${VERSION}" || exit 1
        STAGE=""
        ln -sfn "${VERSION}" "${INSTALL_ROOT}/current.new"
        mv -T "${INSTALL_ROOT}/current.new" "${INSTALL_ROOT}/current"
        echo "Installed version ${VERSION} in ${INSTALL_ROOT}/${VERSION}"
        # the newest versions stay, for sessions that started on them and to go back
        ls -1dt "${INSTALL_ROOT}"/*/ 2>/dev/null | grep -v '/current/$' | tail -n +$(( KEEP_VERSIONS + 1 )) | while read -r old; do
            [ "$(basename "${old}")" != "${VERSION}" ] && rm -rf "${old}"
        done
    fi
fi
VERSION_CHANGED=false
[ "${VERSION}" != "${CURRENT_VERSION}" ] && VERSION_CHANGED=true

# ---------------------------------------------------------------------------
# Gateway: user, directories, registry, certificate, Forgejo token, lablink hub, service
# ---------------------------------------------------------------------------
GATEWAY_URL=""
GATEWAY_STATE=""
FORGEJO_PROBLEM=""
LABLINK_NOTE=""
DEPLOY_STATE=""
if [ "${ROLE}" = "gateway" ] && [ "${UPGRADE}" = "true" ]; then
    # an upgrade only brings the new files to the service; everything else stays as the last install left it
    if [ "${VERSION_CHANGED}" = "true" ]; then
        run systemctl try-restart "${SERVICE}.service"
        GATEWAY_STATE="restarted when it ran"
    else
        GATEWAY_STATE="unchanged"
    fi
elif [ "${ROLE}" = "gateway" ]; then
    GATEWAY_URL="${CI_GATEWAY_URL:-https://$(hostname | tr '[:upper:]' '[:lower:]'):${CI_GATEWAY_PORT}}"

    # -----------------------------------------------------------------------
    # The system user and the directories
    # -----------------------------------------------------------------------
    if ! id "${CI_GATEWAY_USER}" > /dev/null 2>&1; then
        NOLOGIN="/usr/sbin/nologin"
        [ -x "${NOLOGIN}" ] || NOLOGIN="/sbin/nologin"
        run useradd --system --user-group --home-dir "${STATE_DIR}" --no-create-home --shell "${NOLOGIN}" \
            --comment "GUCEF CI gateway" "${CI_GATEWAY_USER}" || exit 1
        dry || echo "Created the system user ${CI_GATEWAY_USER}."
    fi
    GATEWAY_GROUP="$(id -gn "${CI_GATEWAY_USER}" 2>/dev/null)"
    GATEWAY_GROUP="${GATEWAY_GROUP:-${CI_GATEWAY_USER}}"
    # the directories the service user can enter but not change belong to root: the hub (root) writes the token
    # hashes, this script the certificate, and nothing there can be replaced by a link of the service user
    ensure_dir "${CONFIG_DIR}" 755 root root || exit 1
    ensure_dir "${STATE_DIR}" 755 root root || exit 1
    ensure_dir "${STATE_DIR}/work" 750 "${CI_GATEWAY_USER}" "${GATEWAY_GROUP}" || exit 1
    ensure_dir "${STATE_DIR}/logs" 750 "${CI_GATEWAY_USER}" "${GATEWAY_GROUP}" || exit 1
    # the home of the service user: kubectl keeps its cache in .kube
    ensure_dir "${STATE_DIR}/.kube" 700 "${CI_GATEWAY_USER}" "${GATEWAY_GROUP}" || exit 1
    ensure_dir "${TLS_DIR}" 750 root "${GATEWAY_GROUP}" || exit 1
    ensure_dir "${TOKENS_DIR}" 750 root "${GATEWAY_GROUP}" || exit 1

    # -----------------------------------------------------------------------
    # The CI registry: made once from the example, then the file of the administrator
    # -----------------------------------------------------------------------
    REGISTRY_SEEDED=false
    if [ ! -f "${REGISTRY_FILE}" ]; then
        run install -m 644 -o root -g root "${K3S_DIR}/lablink/ci.json.example" "${REGISTRY_FILE}" || exit 1
        REGISTRY_SEEDED=true
    fi
    REGISTRY_SHA256="$(sha256sum "${REGISTRY_FILE}" 2>/dev/null | cut -c1-64)"

    # -----------------------------------------------------------------------
    # The certificate of this node, for the service user, and again when it is renewed
    # -----------------------------------------------------------------------
    put_file "${TLS_SYNC_SCRIPT}" 755 root:root "$(cat <<EOF
#!/bin/bash
# Copies the certificate of this node, which llm/mcp/cluster.sh keeps in ${MCP_DIR}/tls, for the CI gateway: the
# service user ${CI_GATEWAY_USER} cannot read it there. Written by ci/mcp/install.sh; run by ${SERVICE}-tls.service.
# Usage: gateway-tls-sync.sh [--restart]
#   without --restart: exit 0 when nothing changed, 3 when a file was copied, 1 on an error
#   --restart:         restarts ${SERVICE} when a file was copied (and starts it when it waits for the certificate)
SOURCE_DIR="${MCP_DIR}/tls"
TARGET_DIR="${TLS_DIR}"
OWNER="${CI_GATEWAY_USER}"
GROUP="${GATEWAY_GROUP}"
CHANGED=false
put()
{
    local source="\$1" target="\$2" mode="\$3"
    [ -s "\${source}" ] || return 0
    if cmp -s "\${source}" "\${target}" && [ "\$(stat -c '%a %U' "\${target}" 2>/dev/null)" = "\${mode} \${OWNER}" ]; then
        return 0
    fi
    install -m "\${mode}" -o "\${OWNER}" -g "\${GROUP}" "\${source}" "\${target}.new" && mv -f "\${target}.new" "\${target}" || return 1
    CHANGED=true
}
put "\${SOURCE_DIR}/tls.crt" "\${TARGET_DIR}/tls.crt" 644 || exit 1
put "\${SOURCE_DIR}/tls.key" "\${TARGET_DIR}/tls.key" 600 || exit 1
if [ "\${1:-}" = "--restart" ]; then
    if [ "\${CHANGED}" = "true" ]; then
        systemctl try-restart ${SERVICE}.service
    fi
    if systemctl is-enabled --quiet ${SERVICE}.service && ! systemctl is-active --quiet ${SERVICE}.service; then
        systemctl start ${SERVICE}.service
    fi
    exit 0
fi
[ "\${CHANGED}" = "true" ] && exit 3
exit 0
EOF
)" || exit 1
    TLS_CHANGED=false
    if dry; then
        if [ -s "${MCP_DIR}/tls/tls.key" ]; then
            echo "DRY-RUN: copy the certificate of this node from ${MCP_DIR}/tls to ${TLS_DIR} when it differs"
        else
            echo "DRY-RUN: no certificate of this node (yet) in ${MCP_DIR}/tls (or it cannot be seen without sudo)"
        fi
    else
        "${TLS_SYNC_SCRIPT}"
        case $? in
            0) ;;
            3) TLS_CHANGED=true; echo "Copied the certificate of this node to ${TLS_DIR}." ;;
            *) echo "WARNING: the certificate of this node could not be copied to ${TLS_DIR}." ;;
        esac
    fi

    # -----------------------------------------------------------------------
    # The Forgejo token of the bot user: made by forgejo/scratch-repo.sh when missing
    # -----------------------------------------------------------------------
    TOKEN_CHANGED=false
    if [ -s "${FORGEJO_TOKEN_FILE}" ]; then
        if [ "$(stat -c '%a %U' "${FORGEJO_TOKEN_FILE}")" != "600 ${CI_GATEWAY_USER}" ]; then
            run chown "${CI_GATEWAY_USER}:${GATEWAY_GROUP}" "${FORGEJO_TOKEN_FILE}" && run chmod 600 "${FORGEJO_TOKEN_FILE}" && TOKEN_CHANGED=true
        fi
    else
        SCRATCH_REPO="${K3S_DIR}/forgejo/scratch-repo.sh"
        if [ ! -x "${SCRATCH_REPO}" ]; then
            FORGEJO_PROBLEM="${SCRATCH_REPO} is not there"
        elif ! command -v kubectl > /dev/null 2>&1 || [ ! -f "${KUBECONFIG_FILE}" ]; then
            FORGEJO_PROBLEM="kubectl or ${KUBECONFIG_FILE} is missing, so Forgejo cannot be reached from here"
        elif dry; then
            echo "DRY-RUN: as ${BUILD_USER}: ${SCRATCH_REPO} --gateway-token <private temporary file>, then install it as ${FORGEJO_TOKEN_FILE} (owner ${CI_GATEWAY_USER}, mode 0600)"
        else
            echo ""
            echo "The Forgejo token of the gateway (forgejo/scratch-repo.sh --gateway-token)"
            install -d -m 755 "${CACHE_DIR}"
            TOKEN_WORK="$(mktemp -d "${CACHE_DIR}/token.XXXXXX")" || exit 1
            [ "${BUILD_USER}" != "root" ] && chown "${BUILD_USER}" "${TOKEN_WORK}"
            if as_builder "${SCRATCH_REPO}" --gateway-token "${TOKEN_WORK}/forgejo.token" && [ -s "${TOKEN_WORK}/forgejo.token" ]; then
                if install -m 600 -o "${CI_GATEWAY_USER}" -g "${GATEWAY_GROUP}" "${TOKEN_WORK}/forgejo.token" "${FORGEJO_TOKEN_FILE}"; then
                    TOKEN_CHANGED=true
                    echo "Stored the Forgejo token of the gateway in ${FORGEJO_TOKEN_FILE}."
                else
                    FORGEJO_PROBLEM="${FORGEJO_TOKEN_FILE} could not be written"
                fi
            else
                FORGEJO_PROBLEM="forgejo/scratch-repo.sh could not make the token, see above"
            fi
            rm -rf "${TOKEN_WORK}"
            TOKEN_WORK=""
            echo ""
        fi
    fi

    # -----------------------------------------------------------------------
    # Deployments: kubectl and the kubeconfig of the restricted ServiceAccounts (ci/deployer/setup.sh). What is
    # missing is reported and the install goes on: the gateway then builds but does not deploy
    # -----------------------------------------------------------------------
    GATEWAY_KUBECTL=""
    if command -v kubectl > /dev/null 2>&1; then
        GATEWAY_KUBECTL="$(command -v kubectl)"
    elif command -v k3s > /dev/null 2>&1; then
        # one program for the gateway to start: kubectl of k3s through a wrapper
        put_file "${KUBECTL_WRAPPER}" 755 root:root "$(cat <<EOF
#!/bin/sh
# kubectl of k3s for the CI gateway, written by ci/mcp/install.sh
exec $(command -v k3s) kubectl "\$@"
EOF
)" || exit 1
        GATEWAY_KUBECTL="${KUBECTL_WRAPPER}"
    fi
    DEPLOY_KUBECONFIG_SHA256=""
    DEPLOY_KUBECTL=""
    DEPLOY_KUBECONFIG_USED=""
    if [ -L "${DEPLOY_KUBECONFIG}" ]; then
        DEPLOY_STATE="off: ${DEPLOY_KUBECONFIG} is a link; put the file itself there (projects/k3s/ci/deployer/setup.sh)"
    elif [ ! -f "${DEPLOY_KUBECONFIG}" ]; then
        DEPLOY_STATE="off: there is no ${DEPLOY_KUBECONFIG}. As the owner of the cluster (admin kubectl, no sudo) run
    projects/k3s/ci/deployer/setup.sh, put the kubeconfig in place as it says, then run this script again"
    elif [ -z "${GATEWAY_KUBECTL}" ]; then
        DEPLOY_STATE="off: neither kubectl nor k3s is installed on this machine"
    else
        # the gateway reads it as an owner-only file of its own user
        if [ "$(stat -c '%a %U' "${DEPLOY_KUBECONFIG}" 2>/dev/null)" != "600 ${CI_GATEWAY_USER}" ]; then
            { run chown "${CI_GATEWAY_USER}:${GATEWAY_GROUP}" "${DEPLOY_KUBECONFIG}" && run chmod 600 "${DEPLOY_KUBECONFIG}"; } || exit 1
        fi
        DEPLOY_KUBECONFIG_SHA256="$(sha256sum "${DEPLOY_KUBECONFIG}" 2>/dev/null | cut -c1-64)"
        DEPLOY_KUBECTL="${GATEWAY_KUBECTL}"
        DEPLOY_KUBECONFIG_USED="${DEPLOY_KUBECONFIG}"
        DEPLOY_STATE="on: ${DEPLOY_KUBECONFIG} with ${GATEWAY_KUBECTL}; a machine deploys when its gateway token has the scope deploy"
    fi

    # -----------------------------------------------------------------------
    # The settings of the service, the start script and the units
    # -----------------------------------------------------------------------
    put_file "${GATEWAY_ENV}" 644 root:root "$(cat <<EOF
# written by ci/mcp/install.sh from ${SETTINGS_FILE}
CI_GATEWAY_PORT="${CI_GATEWAY_PORT}"
# the registry the gateway was (re)started with: a change restarts it on the next run of install.sh
CI_REGISTRY_SHA256="${REGISTRY_SHA256}"
# deployments (ci/deployer/setup.sh): kubectl and the kubeconfig of the restricted ServiceAccounts, empty when off;
# a changed kubeconfig restarts the gateway on the next run of install.sh
CI_KUBECTL="${DEPLOY_KUBECTL}"
CI_KUBECONFIG="${DEPLOY_KUBECONFIG_USED}"
CI_KUBECONFIG_SHA256="${DEPLOY_KUBECONFIG_SHA256}"
EOF
)" || exit 1
    CONFIG_CHANGED="${FILE_CHANGED}"

    put_file "${RUN_SCRIPT}" 755 root:root "$(cat <<EOF
#!/bin/bash
# Starts the CI gateway of the service ${SERVICE}, written by ci/mcp/install.sh
. ${GATEWAY_ENV}
ARGS=(--mode gateway --listen-tls "0.0.0.0:\${CI_GATEWAY_PORT}" --tls-cert ${TLS_DIR}/tls.crt --tls-key ${TLS_DIR}/tls.key
      --ci-config ${REGISTRY_FILE} --forgejo-token-file ${FORGEJO_TOKEN_FILE}
      --gateway-tokens-file ${GATEWAY_TOKENS_FILE} --lablink-ci-dir none
      --work-dir ${STATE_DIR}/work --log-dir ${STATE_DIR}/logs)
# deployments: the kubeconfig of the restricted ServiceAccounts, one context per namespace. --allow-deploy is not
# given: which machine may deploy is the scope of its gateway token in the tokens file of the hub
if [ -n "\${CI_KUBECONFIG:-}" ] && [ -n "\${CI_KUBECTL:-}" ] && [ -r "\${CI_KUBECONFIG}" ]; then
    export KUBECONFIG="\${CI_KUBECONFIG}"
    ARGS+=(--kubeconfig "\${CI_KUBECONFIG}" --kubectl "\${CI_KUBECTL}")
fi
# the names clients on other machines use for this one
HOST="\$(hostname)"
for name in "\${HOST}" "\${HOST,,}" "\$(hostname -f 2>/dev/null)" \$(hostname -I 2>/dev/null); do
    [ -n "\${name}" ] && ARGS+=(--allowed-host "\${name}")
done
exec ${CI_PROGRAM} "\${ARGS[@]}"
EOF
)" || exit 1
    [ "${FILE_CHANGED}" = "true" ] && CONFIG_CHANGED=true

    UNIT_CHANGED=false
    write_unit()
    {
        put_file "$1" 644 root:root "$2" || exit 1
        [ "${FILE_CHANGED}" = "true" ] && UNIT_CHANGED=true
    }
    write_unit "${UNIT_DIR}/${SERVICE}.service" "$(cat <<EOF
[Unit]
Description=GUCEF CI gateway: the lab CI for the gucefMcpCi proxies of the lab machines
After=network-online.target
Wants=network-online.target
# it waits for the certificate of the node (${SERVICE}-tls.path starts it) and the Forgejo token
ConditionPathExists=${TLS_DIR}/tls.key
ConditionPathExists=${FORGEJO_TOKEN_FILE}

[Service]
User=${CI_GATEWAY_USER}
Group=${GATEWAY_GROUP}
WorkingDirectory=${STATE_DIR}/work
# kubectl keeps its cache in the home of the service user
Environment=HOME=${STATE_DIR}
ExecStart=${RUN_SCRIPT}
Restart=on-failure
RestartSec=10
KillSignal=SIGTERM
TimeoutStopSec=30
LimitNOFILE=8192
UMask=0077
CapabilityBoundingSet=
AmbientCapabilities=
NoNewPrivileges=yes
ProtectSystem=strict
ReadWritePaths=${STATE_DIR}
ProtectHome=yes
PrivateTmp=yes
ProtectKernelTunables=yes
ProtectKernelModules=yes
ProtectControlGroups=yes
RestrictSUIDSGID=yes
LockPersonality=yes

[Install]
WantedBy=multi-user.target
EOF
)"
    write_unit "${UNIT_DIR}/${SERVICE}-tls.path" "$(cat <<EOF
[Unit]
Description=Copies the renewed certificate of the node for the CI gateway and restarts it

[Path]
PathChanged=${MCP_DIR}/tls/tls.key
PathChanged=${MCP_DIR}/tls/tls.crt
Unit=${SERVICE}-tls.service

[Install]
WantedBy=multi-user.target
EOF
)"
    write_unit "${UNIT_DIR}/${SERVICE}-tls.service" "$(cat <<EOF
[Unit]
Description=Copies the certificate of the node for the CI gateway and restarts it when it changed

[Service]
Type=oneshot
ExecStart=${TLS_SYNC_SCRIPT} --restart
EOF
)"

    # -----------------------------------------------------------------------
    # Firewall: the gateway port for the lab networks
    # -----------------------------------------------------------------------
    FIREWALL="${K3S_DIR}/host-node/open_firewall_ports.sh"
    if [ -x "${FIREWALL}" ]; then
        # shellcheck disable=SC2086
        run "${FIREWALL}" ci-gateway ${CI_ALLOW} || echo "WARNING: the firewall could not be changed; open ${CI_GATEWAY_PORT}/tcp to the lab networks by hand."
        [ "${CI_GATEWAY_PORT}" != "8792" ] && echo "WARNING: the firewall role ci-gateway opens 8792/tcp; open ${CI_GATEWAY_PORT}/tcp by hand."
    fi

    # -----------------------------------------------------------------------
    # The lablink hub: the same registry, the gateway URL, the token hashes for the service user
    # -----------------------------------------------------------------------
    if [ "${CHECKOUT}" = "true" ] && [ -d "${GUCEF_ROOT}/projects/CMake/targets/GUCEF_exe_gucefLabLink_TestApp" ]; then
        LABLINK_SETTINGS="${K3S_DIR}/lablink/config.env"
    else
        LABLINK_SETTINGS="${LABLINK_CONFIG_DIR}/settings.env"
    fi
    if [ ! -f "${LABLINK_SETTINGS}" ]; then
        if dry; then
            echo "DRY-RUN: create ${LABLINK_SETTINGS} from ${K3S_DIR}/lablink/config.env.example"
        else
            install -d -m 755 "$(dirname "${LABLINK_SETTINGS}")"
            cp "${K3S_DIR}/lablink/config.env.example" "${LABLINK_SETTINGS}" || exit 1
            [ "${BUILD_USER}" != "root" ] && [ "${LABLINK_SETTINGS}" = "${K3S_DIR}/lablink/config.env" ] && chown "${BUILD_USER}" "${LABLINK_SETTINGS}"
            echo "Created ${LABLINK_SETTINGS} (from config.env.example)."
        fi
    fi
    # the value a setting has in the lablink settings, as lablink/install.sh reads them
    lablink_setting()
    {
        local name="$1"
        [ -f "${LABLINK_SETTINGS}" ] || return 0
        # without carriage returns: a settings file edited on Windows keeps working
    ( set +u; . <(sed 's/\r$//' "${LABLINK_SETTINGS}") > /dev/null 2>&1; printf '%s' "${!name:-}" )
    }
    LABLINK_SETTINGS_CHANGED=false
    # set_lablink_setting NAME VALUE: replaces the active lines of NAME by one marked line, below its commented
    # example when there is one; the line endings and the rest of the file stay as they are
    set_lablink_setting()
    {
        local name="$1" value="$2" current
        current="$(lablink_setting "${name}")"
        [ "${current}" = "${value}" ] && return 0
        LABLINK_SETTINGS_CHANGED=true
        if [ -n "${current}" ]; then
            echo "lablink: ${name} was ${current}, it becomes ${value}"
        fi
        if dry; then
            echo "DRY-RUN: set ${name}=\"${value}\" in ${LABLINK_SETTINGS}"
            return 0
        fi
        python3 -I - "${LABLINK_SETTINGS}" "${name}" "${value}" "${SETTING_MARK}" <<'PYEOF' || { echo "ERROR: ${name} could not be set in ${LABLINK_SETTINGS}."; exit 1; }
import re, sys
path, name, value, mark = sys.argv[1:5]
with open(path, "r", newline="") as f:
    lines = f.read().splitlines(True)
eol = "\r\n" if any(l.endswith("\r\n") for l in lines) else "\n"
active = re.compile(r"^\s*(export\s+)?" + re.escape(name) + r"=")
example = re.compile(r"^\s*#\s*" + re.escape(name) + r"=")
kept = [l for l in lines if not active.match(l)]
if kept and not kept[-1].endswith(("\n", "\r")):
    kept[-1] += eol
line = name + '="' + value + '"   ' + mark + eol
at = next((i for i, l in enumerate(kept) if example.match(l)), None)
if at is None:
    kept.append(line)
else:
    kept.insert(at + 1, line)
with open(path, "w", newline="") as f:
    f.write("".join(kept))
PYEOF
    }
    CURRENT_REGISTRY="$(lablink_setting LAB_CI_REGISTRY)"
    if [ -n "${CURRENT_REGISTRY}" ] && [ "${CURRENT_REGISTRY}" != "${REGISTRY_FILE}" ]; then
        echo "WARNING: the lablink hub served the CI registry ${CURRENT_REGISTRY}; the hub and the gateway now share ${REGISTRY_FILE}."
        echo "         Copy what that file defines into ${REGISTRY_FILE} when it held CI ids the example lacks."
    fi
    # a gateway URL set for the hub stays, unless CI_GATEWAY_URL names one
    if [ -z "${CI_GATEWAY_URL}" ]; then
        CURRENT_URL="$(lablink_setting LAB_CI_GATEWAY_URL)"
        [ -n "${CURRENT_URL}" ] && GATEWAY_URL="${CURRENT_URL}"
    fi
    set_lablink_setting LAB_CI_REGISTRY "${REGISTRY_FILE}"
    set_lablink_setting LAB_CI_GATEWAY_URL "${GATEWAY_URL}"
    set_lablink_setting LAB_CI_GATEWAY_TOKENS "${GATEWAY_TOKENS_FILE}"
    set_lablink_setting LAB_CI_GATEWAY_USER "${CI_GATEWAY_USER}"
    # the machines whose gateway token may also deploy: set only when CI_DEPLOY_MACHINES is (also when empty)
    [ "${CI_DEPLOY_MACHINES_SET}" = "true" ] && set_lablink_setting LAB_CI_DEPLOY_MACHINES "${CI_DEPLOY_MACHINES}"
    HUB_DEPLOY_MACHINES="$(lablink_setting LAB_CI_DEPLOY_MACHINES)"
    [ "${CI_DEPLOY_MACHINES_SET}" = "true" ] && HUB_DEPLOY_MACHINES="${CI_DEPLOY_MACHINES}"
    HUB_DEPLOY_MACHINES="$(printf '%s' "${HUB_DEPLOY_MACHINES}" | tr -d '[:space:]')"
    [ "${LABLINK_SETTINGS_CHANGED}" = "true" ] && ! dry && echo "Set the CI settings of the lablink hub in ${LABLINK_SETTINGS}."

    # the options of the running hub, as lablink/install.sh writes them from those settings: they are the last of
    # the cluster role, so the other options stay as they are
    ARGS_FILE="${LABLINK_CONFIG_DIR}/args.env"
    if [ ! -f "${ARGS_FILE}" ]; then
        LABLINK_NOTE="the lablink hub is not installed here: install it (sudo projects/k3s/lablink/install.sh); it then serves the CI configuration with these settings"
    elif ! grep -q -- '--role cluster' "${ARGS_FILE}"; then
        LABLINK_NOTE="lablink runs here in the machine role, not as the hub: approved machines get no CI configuration from this machine"
    else
        ARGS_LINE="$(cat "${ARGS_FILE}")"
        ARGS_TEXT="${ARGS_LINE#LABLINK_ARGS=\"}"
        ARGS_TEXT="${ARGS_TEXT%\"}"
        read -r -a ARGS_WORDS <<< "${ARGS_TEXT}"
        NEW_WORDS=()
        SKIP_VALUE=false
        for word in "${ARGS_WORDS[@]}"; do
            if [ "${SKIP_VALUE}" = "true" ]; then SKIP_VALUE=false; continue; fi
            case "${word}" in
                --ci-registry|--ci-gateway-url|--ci-gateway-tokens|--ci-gateway-user|--ci-deploy-machines) SKIP_VALUE=true; continue ;;
            esac
            NEW_WORDS+=("${word}")
        done
        # as lablink/install.sh writes them from its settings
        NEW_WORDS+=(--ci-registry "${REGISTRY_FILE}" --ci-gateway-url "${GATEWAY_URL}"
                    --ci-gateway-tokens "${GATEWAY_TOKENS_FILE}" --ci-gateway-user "${CI_GATEWAY_USER}")
        [ -n "${HUB_DEPLOY_MACHINES}" ] && NEW_WORDS+=(--ci-deploy-machines "${HUB_DEPLOY_MACHINES}")
        NEW_ARGS_LINE="LABLINK_ARGS=\"${NEW_WORDS[*]}\""
        if [ "${NEW_ARGS_LINE}" != "${ARGS_LINE}" ]; then
            if dry; then
                echo "DRY-RUN: write the CI options into ${ARGS_FILE} and restart ${LABLINK_SERVICE}"
            else
                printf '%s\n' "${NEW_ARGS_LINE}" > "${ARGS_FILE}"
                systemctl try-restart "${LABLINK_SERVICE}.service"
                echo "Restarted ${LABLINK_SERVICE} with the CI settings."
            fi
        fi
    fi

    # -----------------------------------------------------------------------
    # The service
    # -----------------------------------------------------------------------
    if dry; then
        echo "DRY-RUN: systemctl daemon-reload (when a unit changed), enable ${SERVICE}-tls.path and ${SERVICE}.service, (re)start ${SERVICE} when something changed"
        if [ ! -s "${FORGEJO_TOKEN_FILE}" ]; then
            GATEWAY_STATE="installed, disabled until the Forgejo token is there"
        else
            GATEWAY_STATE="enabled"
        fi
    else
        [ "${UNIT_CHANGED}" = "true" ] && systemctl daemon-reload
        systemctl enable --quiet "${SERVICE}-tls.path"
        # a path unit watches the paths it had when it started: it starts again so that it watches those of its definition
        if [ "${UNIT_CHANGED}" = "true" ] || ! systemctl is-active --quiet "${SERVICE}-tls.path"; then
            systemctl stop "${SERVICE}-tls.service" > /dev/null 2>&1
            systemctl restart "${SERVICE}-tls.path"
        fi
        if [ ! -s "${FORGEJO_TOKEN_FILE}" ]; then
            if systemctl is-enabled --quiet "${SERVICE}.service" || systemctl is-active --quiet "${SERVICE}.service"; then
                systemctl disable --now --quiet "${SERVICE}.service"
            fi
            GATEWAY_STATE="installed, disabled until the Forgejo token is there"
        else
            systemctl is-enabled --quiet "${SERVICE}.service" || systemctl enable --quiet "${SERVICE}.service"
            if [ ! -s "${TLS_DIR}/tls.key" ]; then
                GATEWAY_STATE="enabled, waits for the certificate of this node (llm/mcp/cluster.sh); it starts by itself once it is there"
            else
                if ! systemctl is-active --quiet "${SERVICE}.service"; then
                    systemctl start "${SERVICE}.service"
                elif [ "${VERSION_CHANGED}" = "true" ] || [ "${CONFIG_CHANGED}" = "true" ] || [ "${UNIT_CHANGED}" = "true" ] || \
                     [ "${TLS_CHANGED}" = "true" ] || [ "${TOKEN_CHANGED}" = "true" ]; then
                    echo "Restarting ${SERVICE}: uploads and waiting requests of this moment are ended."
                    systemctl restart "${SERVICE}.service"
                fi
                GATEWAY_STATE="runs"
                sleep 2
                if ! systemctl is-active --quiet "${SERVICE}.service"; then
                    GATEWAY_STATE="does not run: journalctl -u ${SERVICE} -n 50"
                elif command -v curl > /dev/null 2>&1 && [ -s "${MCP_DIR}/tls/ca.crt" ]; then
                    # without a token the gateway answers 401: it listens and its certificate is valid
                    CODE="000"
                    for _ in $(seq 1 10); do
                        CODE="$(curl -s -o /dev/null -w '%{http_code}' -m 5 --cacert "${MCP_DIR}/tls/ca.crt" \
                                "https://localhost:${CI_GATEWAY_PORT}/v1/ci" 2>/dev/null)"
                        [ "${CODE}" != "000" ] && break
                        sleep 1
                    done
                    if [ "${CODE}" = "000" ]; then
                        GATEWAY_STATE="runs, but https://localhost:${CI_GATEWAY_PORT} does not answer: journalctl -u ${SERVICE} -n 50"
                    else
                        GATEWAY_STATE="runs, answers on https://localhost:${CI_GATEWAY_PORT} (HTTP ${CODE} without a token)"
                    fi
                fi
            fi
        fi
    fi
fi

# ---------------------------------------------------------------------------
# What this installation is, for --upgrade and uninstall.sh
# ---------------------------------------------------------------------------
[ -d "${CONFIG_DIR}" ] || run install -d -m 755 "${CONFIG_DIR}"
if [ "${UPGRADE}" != "true" ]; then
    put_file "${INSTALL_RECORD}" 644 root:root "$(cat <<EOF
# written by ci/mcp/install.sh
CI_INSTALLED_ROLE="${ROLE}"
CI_INSTALLED_USER="${CLAUDE_USER}"
CI_INSTALLED_NAME="${CI_MCP_NAME}"
CI_INSTALLED_GATEWAY_USER="${CI_GATEWAY_USER}"
EOF
)" || exit 1
fi

# ---------------------------------------------------------------------------
# Claude Code of the user: the stdio server, started by Claude Code itself
# ---------------------------------------------------------------------------
REGISTER_ARGS=(--stdio --mode auto)
# on the gateway machine CI_GATEWAY_URL is the URL the hub announces; it is a client setting only with a token
if [ -n "${CI_GATEWAY_URL}" ] && { [ "${ROLE}" = "client" ] || [ -n "${CI_GATEWAY_TOKEN_FILE}" ]; }; then
    REGISTER_ARGS+=(--gateway-url "${CI_GATEWAY_URL}")
fi
[ -n "${CI_GATEWAY_TOKEN_FILE}" ] && REGISTER_ARGS+=(--gateway-token-file "${CI_GATEWAY_TOKEN_FILE}")
[ -n "${CI_CA_FILE}" ] && REGISTER_ARGS+=(--ca-file "${CI_CA_FILE}")
MANUAL_REGISTRATION="claude mcp add --scope user ${CI_MCP_NAME} -- ${CI_PROGRAM} ${REGISTER_ARGS[*]}"
# the registration of the previous install goes when the name, the user or CI_REGISTER_CLAUDE changed, and only when
# it is the program of this installation
if [ "${UPGRADE}" != "true" ] && [ -n "${PREVIOUS_NAME}" ] && [ -n "${PREVIOUS_USER}" ] && \
   { [ "${PREVIOUS_NAME}" != "${CI_MCP_NAME}" ] || [ "${PREVIOUS_USER}" != "${CLAUDE_USER}" ] || [ "${CI_REGISTER_CLAUDE}" != "true" ]; } && \
   id "${PREVIOUS_USER}" > /dev/null 2>&1; then
    PREVIOUS_HOME="$(getent passwd "${PREVIOUS_USER}" | cut -d: -f6)"
    PREVIOUS_CLAUDE="$(sudo -u "${PREVIOUS_USER}" -H bash -lc 'command -v claude' 2>/dev/null)"
    if [ -n "${PREVIOUS_CLAUDE}" ] && python3 -I - "${PREVIOUS_HOME}/.claude.json" "${PREVIOUS_NAME}" "${INSTALL_ROOT}/" <<'PYEOF' 2>/dev/null; then
import json, sys
path, name, prefix = sys.argv[1:4]
entry = json.load(open(path)).get("mcpServers", {}).get(name)
sys.exit(0 if isinstance(entry, dict) and str(entry.get("command", "")).startswith(prefix) else 1)
PYEOF
        if dry; then
            echo "DRY-RUN: as ${PREVIOUS_USER}: claude mcp remove ${PREVIOUS_NAME} --scope user   (the earlier registration)"
        elif sudo -u "${PREVIOUS_USER}" -H "${PREVIOUS_CLAUDE}" mcp remove "${PREVIOUS_NAME}" --scope user > /dev/null 2>&1; then
            echo "Removed the earlier registration ${PREVIOUS_NAME} from Claude Code of ${PREVIOUS_USER}."
        fi
    fi
fi
REGISTERED=""
if [ "${UPGRADE}" = "true" ]; then
    :   # the registration names the program through current, which now points to the new version
elif [ "${CI_REGISTER_CLAUDE}" != "true" ]; then
    REGISTERED="not registered (CI_REGISTER_CLAUDE=${CI_REGISTER_CLAUDE})"
elif [ -z "${CLAUDE_USER}" ]; then
    REGISTERED="not registered: run this with sudo from the user whose Claude Code gets the tools"
elif ! id "${CLAUDE_USER}" > /dev/null 2>&1; then
    REGISTERED="not registered: there is no user ${CLAUDE_USER}"
else
    CLAUDE_GROUP="$(id -gn "${CLAUDE_USER}")"
    CLAUDE_HOME="$(getent passwd "${CLAUDE_USER}" | cut -d: -f6)"
    # the claude command of the user; a user who has Claude Code only as an editor extension gets the wrapper
    # llm/mcp/install.sh also makes (the same mark, so that llm/mcp/uninstall.sh knows it)
    CLAUDE_WRAPPER="${CLAUDE_HOME}/.local/bin/claude"
    if ! as_claude_user bash -lc 'command -v claude' > /dev/null 2>&1 && [ ! -e "${CLAUDE_WRAPPER}" ]; then
        if compgen -G "${CLAUDE_HOME}/.vscode*/extensions/anthropic.claude-code-*/resources/native-binary/claude" > /dev/null \
                || compgen -G "${CLAUDE_HOME}/.cursor*/extensions/anthropic.claude-code-*/resources/native-binary/claude" > /dev/null \
                || compgen -G "${CLAUDE_HOME}/.windsurf*/extensions/anthropic.claude-code-*/resources/native-binary/claude" > /dev/null; then
            if dry; then
                echo "DRY-RUN: create ${CLAUDE_WRAPPER}: Claude Code of ${CLAUDE_USER} is an editor extension"
            else
                install -d -m 755 -o "${CLAUDE_USER}" -g "${CLAUDE_GROUP}" "${CLAUDE_HOME}/.local" "${CLAUDE_HOME}/.local/bin"
                cat > "${CLAUDE_WRAPPER}" <<'WRAPPEREOF'
#!/bin/bash
# gucef-mcp: claude command
# Runs the Claude Code binary of the newest editor extension installed for this user, so that
# tools that need a "claude" command (the coding agent of the service gucef-mcp) find one.
# Written by llm/mcp/install.sh or ci/mcp/install.sh; delete this file if you install Claude Code another way.
BIN="$(ls -d "${HOME}"/.vscode*/extensions/anthropic.claude-code-*/resources/native-binary/claude \
              "${HOME}"/.cursor*/extensions/anthropic.claude-code-*/resources/native-binary/claude \
              "${HOME}"/.windsurf*/extensions/anthropic.claude-code-*/resources/native-binary/claude 2>/dev/null \
        | awk -F/ '{ v = $0; sub(/.*anthropic\.claude-code-/, "", v); sub(/\/.*/, "", v); print v "\t" $0 }' \
        | sort -V | tail -n 1 | cut -f2)"
[ -x "${BIN}" ] || { echo "ERROR: no Claude Code binary found in the editor extensions of ${HOME}" >&2; exit 127; }
exec "${BIN}" "$@"
WRAPPEREOF
                chown "${CLAUDE_USER}:${CLAUDE_GROUP}" "${CLAUDE_WRAPPER}"
                chmod 755 "${CLAUDE_WRAPPER}"
                echo "Created ${CLAUDE_WRAPPER}: Claude Code of ${CLAUDE_USER} is an editor extension, this runs it as 'claude'."
            fi
        fi
    fi
    CLAUDE_BIN="$(as_claude_user bash -lc 'command -v claude' 2>/dev/null)"
    if [ -z "${CLAUDE_BIN}" ] && [ -x "${CLAUDE_WRAPPER}" ]; then
        CLAUDE_BIN="${CLAUDE_WRAPPER}"
    fi
    if [ -z "${CLAUDE_BIN}" ]; then
        REGISTERED="not registered: ${CLAUDE_USER} has no claude command"
    else
        # the user scope of Claude Code is the object mcpServers of ~/.claude.json: registered again only when it differs
        if python3 -I - "${CLAUDE_HOME}/.claude.json" "${CI_MCP_NAME}" "${CI_PROGRAM}" "${REGISTER_ARGS[@]}" <<'PYEOF' 2>/dev/null; then
import json, sys
path, name, command, args = sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4:]
entry = json.load(open(path)).get("mcpServers", {}).get(name)
same = isinstance(entry, dict) and entry.get("type", "stdio") == "stdio" and entry.get("command") == command \
       and list(entry.get("args", [])) == args and not entry.get("env")
sys.exit(0 if same else 1)
PYEOF
            REGISTERED="registered as ${CI_MCP_NAME} with Claude Code of ${CLAUDE_USER} (unchanged)"
        elif dry; then
            echo "DRY-RUN: as ${CLAUDE_USER}: claude mcp remove ${CI_MCP_NAME} --scope user; ${MANUAL_REGISTRATION}"
            REGISTERED="would be registered as ${CI_MCP_NAME} with Claude Code of ${CLAUDE_USER}"
        else
            as_claude_user "${CLAUDE_BIN}" mcp remove "${CI_MCP_NAME}" --scope user > /dev/null 2>&1
            if as_claude_user "${CLAUDE_BIN}" mcp add --scope user "${CI_MCP_NAME}" -- "${CI_PROGRAM}" "${REGISTER_ARGS[@]}" > /dev/null; then
                REGISTERED="registered as ${CI_MCP_NAME} with Claude Code of ${CLAUDE_USER}; sessions started from now on have it"
            else
                REGISTERED="WARNING: registering with Claude Code of ${CLAUDE_USER} failed"
            fi
        fi
    fi
fi

# ---------------------------------------------------------------------------
# What auto mode finds on this machine
# ---------------------------------------------------------------------------
CI_NOTES=()
LABLINK_ARGS_FILE="${LABLINK_CONFIG_DIR}/args.env"
if [ "${UPGRADE}" = "true" ]; then
    :
elif [ "${ROLE}" = "client" ]; then
    if [ -f "${LABLINK_ARGS_FILE}" ] && grep -q -- '--no-ci' "${LABLINK_ARGS_FILE}"; then
        CI_NOTES+=("lablink runs with --no-ci (LAB_CI=false): it does not ask the hub for the CI configuration. Set LAB_CI=true in the lablink settings and run lablink/install.sh again.")
    fi
    LABLINK_CLAUDE_USER=""
    [ -f "${LABLINK_ARGS_FILE}" ] && LABLINK_CLAUDE_USER="$(sed -n 's/.*--claude-user \([^ "]*\).*/\1/p' "${LABLINK_ARGS_FILE}")"
    if [ -n "${LABLINK_CLAUDE_USER}" ] && [ -n "${CLAUDE_USER}" ] && [ "${LABLINK_CLAUDE_USER}" != "${CLAUDE_USER}" ]; then
        CI_NOTES+=("lablink gives the files of ${MCP_DIR}/ci to ${LABLINK_CLAUDE_USER}, but gucefMcpCi runs as ${CLAUDE_USER} and reads owner-only files of its own user only. Set LAB_CLAUDE_USER=${CLAUDE_USER} in the lablink settings.")
    fi
    if [ ! -d "${MCP_DIR}/ci" ] && [ -z "${CI_GATEWAY_URL}${CI_GATEWAY_TOKEN_FILE}" ]; then
        if [ -f "${LABLINK_ARGS_FILE}" ]; then
            CI_NOTES+=("There is no CI configuration from lablink in ${MCP_DIR}/ci yet. The hub hands it out to approved machines only:"
                       "  on the hub: sudo gucefLabLink --approve $(uname -n | tr '[:upper:]' '[:lower:]')   (a machine linked with the lab secret alone gets none)"
                       "  it arrives within the refresh time of lablink (10 minutes); until then the tools of ${CI_MCP_NAME} say what is missing.")
        else
            CI_NOTES+=("There is no CI configuration in ${MCP_DIR}/ci: lablink is not installed here. Install it (projects/k3s/lablink/install.sh,"
                       "  without --secret so that the hub approves this machine), or set CI_GATEWAY_URL, CI_GATEWAY_TOKEN_FILE and CI_CA_FILE in ${SETTINGS_FILE}.")
        fi
    fi
else
    if [ -z "${CI_GATEWAY_TOKEN_FILE}" ]; then
        CI_NOTES+=("On this machine the hub hands itself no CI configuration: ${CI_MCP_NAME} offers its tools here only with CI_GATEWAY_URL and"
                   "  CI_GATEWAY_TOKEN_FILE (a token file of ${CLAUDE_USER:-the user}, mode 0600) in ${SETTINGS_FILE}. The other machines use the gateway.")
    fi
fi

# ---------------------------------------------------------------------------
# Summary
# ---------------------------------------------------------------------------
echo ""
echo "gucefMcpCi ${VERSION:-(not built in a dry run)} in ${INSTALL_ROOT}/current, role ${ROLE}."
[ -n "${REGISTERED}" ] && echo "  Claude Code: ${REGISTERED}"
if [ "${REGISTERED#registered}" = "${REGISTERED}" ] && [ "${UPGRADE}" != "true" ]; then
    echo "    by hand, as that user: ${MANUAL_REGISTRATION}"
fi
if [ "${ROLE}" = "gateway" ] && [ "${UPGRADE}" = "true" ]; then
    echo "  Gateway ${SERVICE}: ${GATEWAY_STATE}"
elif [ "${ROLE}" = "gateway" ]; then
    echo "  Gateway ${SERVICE}: ${GATEWAY_STATE}"
    echo "  Gateway URL the lablink hub announces: ${GATEWAY_URL}"
    echo "  CI registry (hub and gateway): ${REGISTRY_FILE}"
    if [ "${REGISTRY_SEEDED:-false}" = "true" ]; then
        echo "    made from projects/k3s/lablink/ci.json.example: edit it (CI ids, Forgejo URL, scratch repository, remotes),"
        echo "    then run this script again, which restarts the gateway; the hub reads it again by itself."
    fi
    echo "  Deployments: ${DEPLOY_STATE}"
    [ -n "${HUB_DEPLOY_MACHINES:-}" ] && echo "  Machines whose gateway token may also deploy (hub): ${HUB_DEPLOY_MACHINES}"
    [ -n "${LABLINK_NOTE}" ] && echo "WARNING: ${LABLINK_NOTE}."
    if [ -n "${FORGEJO_PROBLEM}" ]; then
        echo "WARNING: there is no Forgejo token for the gateway: ${FORGEJO_PROBLEM}."
        echo "  The service is installed and stays disabled until the token is there. To finish:"
        echo "    1. Forgejo with its CI runner: projects/k3s/setup.sh with GIT_SERVER=forgejo and CI_ENGINE=forgejo or all"
        echo "    2. once: projects/k3s/forgejo/scratch-repo.sh --seed <published repository> --report-token"
        echo "    3. again: sudo ${SCRIPT_DIR}/install.sh --role gateway   (makes the token with scratch-repo.sh --gateway-token)"
        echo "  Or put a token of ci-bot with the scopes write:repository, read:package and write:package in"
        echo "  ${FORGEJO_TOKEN_FILE} (owner ${CI_GATEWAY_USER}, mode 0600) and run step 3."
    fi
fi
for note in "${CI_NOTES[@]}"; do
    echo "  ${note}"
done
dry && echo "Dry run: nothing was changed."
exit 0
