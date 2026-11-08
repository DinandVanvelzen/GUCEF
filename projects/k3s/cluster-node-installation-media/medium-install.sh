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
# Usage: sudo bash install.sh [--secret SECRET] [--approval]
#   --secret SECRET  the lab secret; default: lab-secret on this medium, or
#                    asked for
#   --approval       without the lab secret: the machine waits until it is
#                    approved on the cluster (sudo gucefLabLink --approve NAME)
# ---------------------------------------------------------------------------

MEDIUM_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SCRIPTS_DIR="/usr/share/gucef-k3s"
SETTINGS_FILE="/etc/gucef-lablink/settings.env"

SECRET=""
APPROVAL=false
while [ $# -gt 0 ]; do
    case "$1" in
        --secret) SECRET="$2"; shift ;;
        --approval) APPROVAL=true ;;
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
for file in "${MEDIUM_DIR}"/linux/*-"${ARCH}.${EXTENSION}" "${MEDIUM_DIR}"/linux/*-all."${EXTENSION}"; do
    [ -f "${file}" ] && cp "${file}" "${PACKAGES}/" && FILES+=("${PACKAGES}/$(basename "${file}")")
done
KIND="${EXTENSION^^} (${ARCH})"
chmod 644 "${PACKAGES}"/* 2> /dev/null
if [ ${#FILES[@]} -lt 3 ]; then
    echo "ERROR: this medium has no ${KIND} packages for this machine:"
    ls -1 "${MEDIUM_DIR}/linux" 2> /dev/null | sed 's/^/  /'
    exit 1
fi

echo "Installing ${FILES[*]##*/}"
if command -v apt-get > /dev/null 2>&1; then
    DEBIAN_FRONTEND=noninteractive apt-get install -y -q --allow-downgrades "${FILES[@]}"
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
        echo "The lab secret links this machine without approval (on the cluster: sudo gucefLabLink --init shows it)."
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
