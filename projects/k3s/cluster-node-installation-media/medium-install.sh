#!/bin/bash
# ---------------------------------------------------------------------------
# Installs the home lab services on this machine from this medium (made by
# projects/k3s/cluster-node-installation-media/make-media.sh) and sets them up: the packages
# gucef-k3s-scripts, gucef-mcp and gucef-lablink, then the service
# gucef-lablink. The machine then finds the cluster, joins it as a node and
# registers the MCP services of the nodes with Claude Code of the user that
# ran sudo. Re-running it installs newer packages and changes only what
# differs.
#
# Usage: sudo bash install.sh [--secret SECRET] [--approval] [--allow-downgrade]
#   --secret SECRET    the lab secret; default: lab-secret on this medium, or
#                      asked for
#   --approval         without the lab secret: the machine waits until it is
#                      approved on the cluster (sudo gucefLabLink --approve NAME)
#   --allow-downgrade  install this medium also when the machine has newer
#                      packages; without it an older medium is refused
# ---------------------------------------------------------------------------

MEDIUM_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SCRIPTS_DIR="/usr/share/gucef-k3s"
SETTINGS_FILE="/etc/gucef-lablink/settings.env"

SECRET=""
APPROVAL=false
ALLOW_DOWNGRADE=false
while [ $# -gt 0 ]; do
    case "$1" in
        --secret) SECRET="$2"; shift ;;
        --approval) APPROVAL=true ;;
        --allow-downgrade) ALLOW_DOWNGRADE=true ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
    shift
done

if [ "$(uname -s)" != "Linux" ]; then
    echo "ERROR: this medium installs on Linux."
    exit 1
fi
if [ "$(id -u)" -ne 0 ]; then
    echo "ERROR: run with sudo: sudo bash $0"
    exit 1
fi
if [ ! -d /run/systemd/system ]; then
    echo "ERROR: the services need systemd."
    exit 1
fi

MEDIUM_VERSION="$(tr -d '[:space:]' < "${MEDIUM_DIR}/VERSION" 2>/dev/null)"
echo "GUCEF lab install medium ${MEDIUM_VERSION:-(without VERSION)} in ${MEDIUM_DIR}"

if command -v sha256sum > /dev/null 2>&1 && [ -f "${MEDIUM_DIR}/SHA256SUMS" ]; then
    if ! ( cd "${MEDIUM_DIR}" && sha256sum --quiet -c SHA256SUMS ); then
        echo "ERROR: files of the medium differ from SHA256SUMS; make the medium again."
        exit 1
    fi
fi

# ---------------------------------------------------------------------------
# Packages
# ---------------------------------------------------------------------------
# a copy outside the medium: package managers read it as another user, and the medium may be gone later
PACKAGES="$(mktemp -d)"
trap 'rm -rf "${PACKAGES}"' EXIT
chmod 755 "${PACKAGES}"
FILES=()
case "$(uname -m)" in
    x86_64) ARCH=amd64 ;;
    aarch64|arm64) ARCH=arm64 ;;
    *) ARCH="$(uname -m)" ;;
esac
if command -v apt-get > /dev/null 2>&1; then
    EXTENSION=deb
    # the architecture of the packages, e.g. armhf for a 32-bit system on a 64-bit board
    ARCH="$(dpkg --print-architecture)"
else
    EXTENSION=rpm
fi
# the packages of this medium's version; files of another medium in the same directory are left out
for package in gucef-k3s-scripts gucef-mcp gucef-lablink; do
    if [ -n "${MEDIUM_VERSION}" ]; then
        candidates="$(ls -1 "${MEDIUM_DIR}"/linux/"${package}-${MEDIUM_VERSION}"-{"${ARCH}",all}."${EXTENSION}" 2>/dev/null)"
    else
        candidates="$(ls -1 "${MEDIUM_DIR}"/linux/"${package}"-[0-9]*-{"${ARCH}",all}."${EXTENSION}" 2>/dev/null | sort -V | tail -1)"
    fi
    file="$(echo "${candidates}" | head -1)"
    [ -f "${file}" ] && cp "${file}" "${PACKAGES}/" && FILES+=("${PACKAGES}/$(basename "${file}")")
done
OTHERS="$(ls -1 "${MEDIUM_DIR}"/linux/ 2>/dev/null | grep -v -- "-${MEDIUM_VERSION:-none}-" | tr '\n' ' ')"
if [ -n "${MEDIUM_VERSION}" ] && [ -n "${OTHERS// /}" ]; then
    echo "NOTE: ${MEDIUM_DIR}/linux holds packages of another medium, they are left out: ${OTHERS}"
fi
KIND="${EXTENSION^^} (${ARCH})"
chmod 644 "${PACKAGES}"/* 2> /dev/null
if [ ${#FILES[@]} -lt 3 ]; then
    echo "ERROR: this medium has no ${KIND} packages for this machine:"
    ls -1 "${MEDIUM_DIR}/linux" 2> /dev/null | sed 's/^/  /'
    exit 1
fi

# an older medium does not replace newer packages by accident
INSTALLED_VERSION=""
if command -v dpkg-query > /dev/null 2>&1; then
    INSTALLED_VERSION="$(dpkg-query -W -f='${Version}' gucef-lablink 2>/dev/null)"
elif command -v rpm > /dev/null 2>&1; then
    INSTALLED_VERSION="$(rpm -q --qf '%{VERSION}' gucef-lablink 2>/dev/null | grep -v 'not installed')"
fi
if [ -n "${MEDIUM_VERSION}" ] && [ -n "${INSTALLED_VERSION}" ] && [ "${INSTALLED_VERSION}" != "${MEDIUM_VERSION}" ] &&
   [ "$(printf '%s\n%s\n' "${INSTALLED_VERSION}" "${MEDIUM_VERSION}" | sort -V | tail -1)" = "${INSTALLED_VERSION}" ]; then
    if [ "${ALLOW_DOWNGRADE}" != "true" ]; then
        echo "ERROR: this machine has version ${INSTALLED_VERSION}, newer than this medium (${MEDIUM_VERSION})."
        echo "       Use the newest medium, or --allow-downgrade to go back to this one."
        exit 1
    fi
    echo "Going back from ${INSTALLED_VERSION} to ${MEDIUM_VERSION} (--allow-downgrade)."
fi

echo "Installing ${FILES[*]##*/}"
if command -v apt-get > /dev/null 2>&1; then
    DOWNGRADE_OPTION=()
    [ "${ALLOW_DOWNGRADE}" = "true" ] && DOWNGRADE_OPTION=(--allow-downgrades)
    DEBIAN_FRONTEND=noninteractive apt-get install -y -q "${DOWNGRADE_OPTION[@]}" "${FILES[@]}"
elif command -v dnf > /dev/null 2>&1; then
    dnf install -y "${FILES[@]}"
elif command -v zypper > /dev/null 2>&1; then
    zypper --non-interactive install --allow-unsigned-rpm "${FILES[@]}"
elif command -v yum > /dev/null 2>&1; then
    yum install -y "${FILES[@]}"
else
    echo "ERROR: no apt-get, dnf, zypper or yum to install the packages with."
    false
fi
if [ $? -ne 0 ]; then
    echo "ERROR: installing the packages failed, see above."
    exit 1
fi

# ---------------------------------------------------------------------------
# Settings and the lab secret
# ---------------------------------------------------------------------------
if [ -f "${MEDIUM_DIR}/lablink.env" ] && [ ! -f "${SETTINGS_FILE}" ]; then
    install -d -m 755 "$(dirname "${SETTINGS_FILE}")"
    install -m 644 "${MEDIUM_DIR}/lablink.env" "${SETTINGS_FILE}"
    echo "Settings of the lab: ${SETTINGS_FILE}"
fi

if [ -z "${SECRET}" ] && [ "${APPROVAL}" != "true" ]; then
    if [ -s "${MEDIUM_DIR}/lab-secret" ]; then
        SECRET="$(tr -d '[:space:]' < "${MEDIUM_DIR}/lab-secret")"
    elif [ -s /var/lib/gucef-lablink/secret ]; then
        :
    elif [ -t 0 ]; then
        echo ""
        echo "The lab secret links this machine without approval (the cluster shows it at the end of its install.sh, or: sudo cat /var/lib/gucef-lablink/secret)."
        read -r -s -p "Lab secret (Enter: wait for approval on the cluster instead): " SECRET
        echo ""
    fi
fi

ARGS=()
[ -n "${SECRET}" ] && ARGS+=(--secret "${SECRET}")
"${SCRIPTS_DIR}/lablink/install.sh" "${ARGS[@]}" || exit 1

if [ -z "${SUDO_USER}" ] || [ "${SUDO_USER}" = "root" ]; then
    echo ""
    echo "NOTE: run as root, not with sudo from a user: no Claude Code is set up. Set LAB_CLAUDE_USER in ${SETTINGS_FILE}"
    echo "      and run sudo ${SCRIPTS_DIR}/lablink/install.sh again."
fi
exit 0
