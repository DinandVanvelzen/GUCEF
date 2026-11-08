#!/bin/bash
# ---------------------------------------------------------------------------
# Builds, tests and installs gucefLabLink on this machine and runs it as the
# systemd service gucef-lablink (see tools/gucefLabLink/README.md):
#
#   - on a k3s server node it announces the cluster on the local network and
#     answers the machines that link to it; the first run makes the lab secret
#     and prints it
#   - on any other machine it finds the cluster, links to it (with the lab
#     secret, or after approval on the cluster), joins it as an ephemeral node
#     (Linux, agent-node/join.sh) and sets up Claude Code of the user for the MCP
#     services of the nodes
#
# Idempotent: a re-run builds what changed, and installs and restarts only when
# the result or the settings differ.
#
# Without a checkout around this script, as in the package gucef-k3s-scripts,
# it installs the built files of the package gucef-lablink
# (/usr/lib/gucef-lablink) instead of building, and its settings are
# /etc/gucef-lablink/settings.env.
#
# Usage: sudo ./install.sh [--secret SECRET] [--skip-tests] [--prebuilt DIR] [--upgrade]
#   --secret SECRET  a machine: the lab secret the cluster printed; without it
#                    the machine asks the cluster for access and waits until it
#                    is approved there: sudo gucefLabLink --approve <name>
#   --skip-tests     install without running the tests
#   --prebuilt DIR   install the built files in DIR (bin/, lib/) instead of building
#   --upgrade        only when the service is installed already: install the new
#                    files and keep its settings (used by the package)
# Settings: config.env next to this script (see config.env.example)
# ---------------------------------------------------------------------------

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
K3S_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
GUCEF_ROOT="$(cd "${K3S_DIR}/../.." && pwd)"
STATE_DIR="/var/lib/gucef-lablink"
BUILD_DIR="${STATE_DIR}/build"
INSTALL_ROOT="/opt/gucef-lablink"
CONFIG_DIR="/etc/gucef-lablink"
SERVICE="gucef-lablink"
PACKAGE_DIR="/usr/lib/gucef-lablink"
KEEP_VERSIONS=3

SECRET=""
SKIP_TESTS=false
PREBUILT=""
UPGRADE=false
while [ $# -gt 0 ]; do
    case "$1" in
        --secret) SECRET="$2"; shift ;;
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
if [ "${UPGRADE}" = "true" ] && { [ ! -f "/etc/systemd/system/${SERVICE}.service" ] || [ ! -f "${CONFIG_DIR}/args.env" ]; }; then
    exit 0
fi
BUILD_USER="${SUDO_USER:-root}"
as_builder() { if [ "${BUILD_USER}" = "root" ]; then "$@"; else sudo -u "${BUILD_USER}" -H "$@"; fi; }

SETTINGS_FILE="${SCRIPT_DIR}/config.env"
if [ ! -d "${GUCEF_ROOT}/projects/CMake/targets/GUCEF_exe_gucefLabLink_TestApp" ]; then
    PREBUILT="${PREBUILT:-${PACKAGE_DIR}}"
    SETTINGS_FILE="${CONFIG_DIR}/settings.env"
fi
if [ -n "${PREBUILT}" ] && [ ! -x "${PREBUILT}/bin/gucefLabLink" ]; then
    echo "ERROR: ${PREBUILT}/bin/gucefLabLink is not there: install the package gucef-lablink, or run this from a checkout."
    exit 1
fi
if [ ! -f "${SETTINGS_FILE}" ]; then
    install -d -m 755 "$(dirname "${SETTINGS_FILE}")"
    cp "${SCRIPT_DIR}/config.env.example" "${SETTINGS_FILE}"
    [ "${BUILD_USER}" != "root" ] && [ -z "${PREBUILT}" ] && chown "${BUILD_USER}" "${SETTINGS_FILE}"
    echo "Created ${SETTINGS_FILE} (all settings commented out, defaults apply)."
fi
# shellcheck disable=SC1090
. "${SETTINGS_FILE}"

ROLE="${LAB_ROLE:-}"
# an upgrade keeps the role and the settings the service runs with
[ "${UPGRADE}" = "true" ] && ROLE="$(sed -n 's/.*--role \([a-z]*\).*/\1/p' "${CONFIG_DIR}/args.env")"
if [ -z "${ROLE}" ]; then
    if [ -f /etc/systemd/system/k3s.service ]; then ROLE="cluster"; else ROLE="machine"; fi
fi
# the networks of this machine, when no ranges are given
if [ -z "${LAB_ALLOW}" ]; then
    # not the networks of containers, the cluster and virtual machines
    LAB_ALLOW="$(ip -o -4 addr show scope global 2>/dev/null | awk '$2 !~ /^(docker|br-|cni|flannel|veth|virbr|tailscale|lxc|kube)/ {print $4}' | \
        while read -r cidr; do python3 -c 'import ipaddress,sys; print(ipaddress.ip_interface(sys.argv[1]).network)' "${cidr}"; done | \
        sort -u | tr '\n' ' ')"
fi
LAB_JOIN="${LAB_JOIN:-true}"
LAB_PERMANENT="${LAB_PERMANENT:-false}"
LAB_CLAUDE_USER="${LAB_CLAUDE_USER:-${SUDO_USER}}"
[ "${LAB_CLAUDE_USER}" = "root" ] && LAB_CLAUDE_USER=""

install -d -m 700 "${STATE_DIR}"
install -d -m 755 "${STATE_DIR}/logs" "${INSTALL_ROOT}" "${CONFIG_DIR}"
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

    # -----------------------------------------------------------------------
    # Build
    # -----------------------------------------------------------------------
    install -d -m 755 -o "${BUILD_USER}" "${BUILD_DIR}"
    echo "Building gucefLabLink from ${GUCEF_ROOT}"
    # relocatable: the installed copy finds its libraries next to itself, never in the build tree
    as_builder cmake -S "${GUCEF_ROOT}/projects/CMake/targets/GUCEF_exe_gucefLabLink_TestApp" -B "${BUILD_DIR}" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON '-DCMAKE_INSTALL_RPATH=$ORIGIN/../lib' \
        > "${STATE_DIR}/logs/build.log" 2>&1 &&
    as_builder cmake --build "${BUILD_DIR}" --target gucefLabLink gucefLabLink_TestApp codecspluginMONOCYPHER >> "${STATE_DIR}/logs/build.log" 2>&1
    if [ $? -ne 0 ]; then
        grep -E 'error' "${STATE_DIR}/logs/build.log" | head -20
        echo "ERROR: the build failed, see ${STATE_DIR}/logs/build.log. The installed version stays."
        exit 1
    fi
    "${K3S_DIR}/cluster-node-installation-media/stage.sh" \
        --doc "${SCRIPT_DIR}/README.md" --doc "${GUCEF_ROOT}/tools/gucefLabLink/README.md:gucefLabLink.md" \
        "${BUILD_DIR}" "${STAGE}" gucefLabLink libcodecspluginMONOCYPHER.so || exit 1
fi
chmod -R a+rX,go-w "${STAGE}"
VERSION="$(cd "${STAGE}" && find . -type f -print0 | sort -z | xargs -0 sha256sum | sha256sum | cut -c1-12)"
CURRENT_VERSION="$(basename "$(readlink "${INSTALL_ROOT}/current" 2>/dev/null)")"

if [ "${VERSION}" = "${CURRENT_VERSION}" ]; then
    echo "Version ${VERSION} is installed already."
else
    if [ "${SKIP_TESTS}" != "true" ]; then
        echo "Testing version ${VERSION}"
        if ! ( cd "${BUILD_DIR}/bin" && as_builder ./gucefLabLink_TestApp > "${STATE_DIR}/logs/test.log" 2>&1 ); then
            python3 - "${BUILD_DIR}/bin/gucefLabLink_TestApp_Results.xml" <<'PYEOF' 2>/dev/null
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
    ls -1dt "${INSTALL_ROOT}"/*/ 2>/dev/null | grep -v '/current/$' | tail -n +$(( KEEP_VERSIONS + 1 )) | while read -r old; do
        [ "$(basename "${old}")" != "${VERSION}" ] && rm -rf "${old}"
    done
fi
ln -sf "${INSTALL_ROOT}/current/bin/gucefLabLink" /usr/local/bin/gucefLabLink

# ---------------------------------------------------------------------------
# The lab secret
# ---------------------------------------------------------------------------
LABLINK="${INSTALL_ROOT}/current/bin/gucefLabLink"
SECRET_SHOWN=""
if [ "${UPGRADE}" = "true" ]; then
    :
elif [ "${ROLE}" = "cluster" ]; then
    "${LABLINK}" --state-dir "${STATE_DIR}" --init > "${STATE_DIR}/init.txt" 2>/dev/null
    SECRET_SHOWN="$(sed -n 3p "${STATE_DIR}/init.txt" | tr -d ' ')"
    rm -f "${STATE_DIR}/init.txt"
elif [ -n "${SECRET}" ]; then
    "${LABLINK}" --state-dir "${STATE_DIR}" --set-secret "${SECRET}" > /dev/null || exit 1
fi

# ---------------------------------------------------------------------------
# Service
# ---------------------------------------------------------------------------
ARGS="--role ${ROLE} --state-dir ${STATE_DIR}"
for range in ${LAB_ALLOW}; do ARGS="${ARGS} --allow ${range}"; done
[ "${LAB_MDNS:-true}" = "true" ] || ARGS="${ARGS} --no-mdns"
[ "${LAB_BROADCAST:-true}" = "true" ] || ARGS="${ARGS} --no-broadcast"
if [ "${ROLE}" = "cluster" ]; then
    ARGS="${ARGS} --cluster-name ${LAB_CLUSTER_NAME:-$(uname -n | tr '[:upper:]' '[:lower:]')}"
    ARGS="${ARGS} --join-command ${K3S_DIR}/agent-node/join-command.sh"
else
    [ "${LAB_JOIN}" = "true" ] && ARGS="${ARGS} --join-script ${K3S_DIR}/agent-node/join.sh"
    [ "${LAB_PERMANENT}" = "true" ] && ARGS="${ARGS} --permanent"
    if [ -n "${LAB_CLAUDE_USER}" ]; then
        ARGS="${ARGS} --claude-user ${LAB_CLAUDE_USER} --local-node $(uname -n | tr '[:upper:]' '[:lower:]')"
    fi
fi

CONFIG_CHANGED=false
NEW_CONFIG="LABLINK_ARGS=\"${ARGS}\""
[ "${UPGRADE}" = "true" ] && NEW_CONFIG="$(cat "${CONFIG_DIR}/args.env")"
if [ "$(cat "${CONFIG_DIR}/args.env" 2>/dev/null)" != "${NEW_CONFIG}" ]; then
    echo "${NEW_CONFIG}" > "${CONFIG_DIR}/args.env"
    CONFIG_CHANGED=true
fi

UNIT="$(cat <<EOF
[Unit]
Description=GUCEF lab link: the cluster announces itself, machines of the home network link to it
After=network-online.target
Wants=network-online.target

[Service]
EnvironmentFile=${CONFIG_DIR}/args.env
ExecStart=${INSTALL_ROOT}/current/bin/gucefLabLink \$LABLINK_ARGS
Restart=on-failure
RestartSec=10
KillSignal=SIGTERM
TimeoutStopSec=30

[Install]
WantedBy=multi-user.target
EOF
)"
UNIT_CHANGED=false
if [ "$(cat "/etc/systemd/system/${SERVICE}.service" 2>/dev/null)" != "${UNIT}" ]; then
    echo "${UNIT}" > "/etc/systemd/system/${SERVICE}.service"
    systemctl daemon-reload
    UNIT_CHANGED=true
fi
systemctl enable --quiet "${SERVICE}.service"
if ! systemctl is-active --quiet "${SERVICE}.service"; then
    systemctl start "${SERVICE}.service"
elif [ "${VERSION}" != "${CURRENT_VERSION}" ] || [ "${CONFIG_CHANGED}" = "true" ] || [ "${UNIT_CHANGED}" = "true" ]; then
    systemctl restart "${SERVICE}.service"
fi
sleep 2
if ! systemctl is-active --quiet "${SERVICE}.service"; then
    echo "ERROR: ${SERVICE} does not run. See: journalctl -u ${SERVICE} -n 50"
    exit 1
fi

echo ""
echo "gucef-lablink ${VERSION} runs as ${ROLE}, for ${LAB_ALLOW:-every address}."
if [ "${ROLE}" = "cluster" ]; then
    if [ -n "${SECRET_SHOWN}" ]; then
        echo "  The lab secret, for machines that link without approval:"
        echo "    ${SECRET_SHOWN}"
        echo "  On such a machine, from projects/k3s/lablink of a checkout of this repository:"
        echo "    sudo ./install.sh --secret <the secret>"
        echo "  or from an install medium (cluster-node-installation-media/make-media.sh):  sudo bash <medium>/gucef-lab/install.sh"
        echo "  Machines without it wait for approval:  sudo gucefLabLink --list  /  sudo gucefLabLink --approve <name>"
        echo "  Open 47810/tcp and 47811/tcp, and 5353/udp (mDNS) and 47812/udp (broadcast) when a firewall is active."
    fi
else
    echo "  It finds the cluster within a minute; follow it with: journalctl -u ${SERVICE} -f"
    [ -z "${SECRET}" ] && [ ! -s "${STATE_DIR}/secret" ] && \
        echo "  Without the lab secret it waits for approval on the cluster: sudo gucefLabLink --approve $(uname -n | tr '[:upper:]' '[:lower:]')"
fi
exit 0
