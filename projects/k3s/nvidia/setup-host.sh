#!/bin/bash
# ---------------------------------------------------------------------------
# Host level NVIDIA GPU preparation for k3s:
#   - NVIDIA driver (installed/upgraded on Ubuntu when missing or too old)
#   - NVIDIA container toolkit (k3s detects it and registers the "nvidia" runtime)
#
# Must run as root. Run before k3s is installed, or pass --restart-k3s to let an
# already running k3s pick up a newly installed container toolkit.
#
# --check  report what would be changed, change nothing (root not required)
#
# Exit codes:
#    0  done, or nothing to do (no NVIDIA GPU / unsupported distro)
#    1  error
#   10  reboot required to load the new driver; re-run afterwards
#
# Settings: config.env (see config.env.example)
# ---------------------------------------------------------------------------

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EXIT_REBOOT_REQUIRED=10
K3S_CONTAINERD_CONFIG="/var/lib/rancher/k3s/agent/etc/containerd/config.toml"
TOOLKIT_KEYRING="/usr/share/keyrings/nvidia-container-toolkit-keyring.gpg"
TOOLKIT_SOURCES_LIST="/etc/apt/sources.list.d/nvidia-container-toolkit.list"
PACKAGE_BACKUP_DIR="/var/backups/gucef-k3s"

RESTART_K3S=false
CHECK_ONLY=false
for arg in "$@"; do
    case "${arg}" in
        --restart-k3s) RESTART_K3S=true ;;
        --check) CHECK_ONLY=true ;;
    esac
done

# shellcheck source=node_detect.sh
. "${SCRIPT_DIR}/node_detect.sh"

if [ -f "${SCRIPT_DIR}/config.env" ]; then
    # shellcheck disable=SC1091
    . "${SCRIPT_DIR}/config.env"
fi
NVIDIA_MIN_DRIVER_VERSION="${NVIDIA_MIN_DRIVER_VERSION:-570}"
NVIDIA_DRIVER_PACKAGE="${NVIDIA_DRIVER_PACKAGE:-}"

# ---------------------------------------------------------------------------
# Detection
# ---------------------------------------------------------------------------
if ! gucef_has_nvidia_gpu; then
    OTHER_GPUS="$(gucef_describe_other_gpus | sort -u | tr '\n' ' ')"
    if [ -n "${OTHER_GPUS}" ]; then
        echo "Found: ${OTHER_GPUS}- only NVIDIA GPUs are supported by this setup. Skipping GPU host setup."
    else
        echo "No NVIDIA GPU found. Skipping GPU host setup."
    fi
    exit 0
fi
echo "NVIDIA GPU found."

if [ "${CHECK_ONLY}" != "true" ] && [ "$(id -u)" -ne 0 ]; then
    echo "ERROR: must run as root (use sudo)."
    exit 1
fi

if [ ! -r /etc/os-release ]; then
    echo "Cannot determine the Linux distribution. Install the NVIDIA driver and nvidia-container-toolkit manually."
    exit 0
fi
# shellcheck disable=SC1091
. /etc/os-release
IS_UBUNTU=false
IS_DEBIAN_FAMILY=false
case " ${ID} ${ID_LIKE} " in
    *" ubuntu "*) IS_UBUNTU=true; IS_DEBIAN_FAMILY=true ;;
    *" debian "*) IS_DEBIAN_FAMILY=true ;;
esac
if [ "${IS_DEBIAN_FAMILY}" != "true" ]; then
    echo "Distribution '${ID}' is not supported for automated setup."
    echo "Install the NVIDIA driver (>= ${NVIDIA_MIN_DRIVER_VERSION}) and nvidia-container-toolkit manually:"
    echo "  https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html"
    exit 0
fi

# ---------------------------------------------------------------------------
# Driver
# ---------------------------------------------------------------------------
REBOOT_REQUIRED=false
NEW_BRANCH=""

# Highest installed nvidia-driver-<branch> package branch, or empty
installed_driver_branch() {
    dpkg-query -W -f='${Package} ${Status}\n' 'nvidia-driver-*' 2>/dev/null \
        | awk '$NF == "installed" {print $1}' \
        | grep -oE '^nvidia-driver-[0-9]+' | grep -oE '[0-9]+$' \
        | sort -n | tail -n 1
}

# Installed packages belonging to the given driver branch (excluding the container toolkit)
driver_branch_packages() {
    dpkg-query -W -f='${Package}:${Architecture} ${Status}\n' 2>/dev/null \
        | awk '$NF == "installed" {print $1}' \
        | grep -E "^(nvidia-|libnvidia-|xserver-xorg-video-nvidia-)(.*-)?${1}(-server)?(-open)?:" \
        | grep -vE '^(nvidia-container-|libnvidia-container)'
}

# Packages removed earlier that still have configuration files left behind
residual_config_packages() {
    dpkg -l 2>/dev/null | awk '$1 == "rc" && $2 ~ /nvidia/ {print $2}'
}

toolkit_installed() {
    command -v nvidia-ctk > /dev/null 2>&1 && command -v nvidia-container-runtime > /dev/null 2>&1
}

LOADED_VERSION="$(gucef_loaded_nvidia_driver_version)"
LOADED_MAJOR="${LOADED_VERSION%%.*}"
INSTALLED_BRANCH="$(installed_driver_branch)"
echo "Loaded driver: ${LOADED_VERSION:-none}, installed driver package branch: ${INSTALLED_BRANCH:-none}, required: >= ${NVIDIA_MIN_DRIVER_VERSION}"

if [ -n "${LOADED_MAJOR}" ] && [ "${LOADED_MAJOR}" -ge "${NVIDIA_MIN_DRIVER_VERSION}" ]; then
    echo "Loaded NVIDIA driver is recent enough."
elif [ -n "${INSTALLED_BRANCH}" ] && [ "${INSTALLED_BRANCH}" -ge "${NVIDIA_MIN_DRIVER_VERSION}" ]; then
    echo "NVIDIA driver ${INSTALLED_BRANCH} is installed but not loaded yet."
    REBOOT_REQUIRED=true
elif [ "${IS_UBUNTU}" != "true" ]; then
    echo "Automated driver installation is only supported on Ubuntu."
    echo "Install NVIDIA driver >= ${NVIDIA_MIN_DRIVER_VERSION} manually, reboot, and re-run this script."
    exit 0
else
    if ! command -v ubuntu-drivers > /dev/null 2>&1; then
        if [ "${CHECK_ONLY}" = "true" ]; then
            echo "CHECK: would install ubuntu-drivers-common to determine the recommended driver."
            exit 0
        fi
        apt-get update
        apt-get install -y ubuntu-drivers-common
    fi

    if [ -z "${NVIDIA_DRIVER_PACKAGE}" ]; then
        NVIDIA_DRIVER_PACKAGE="$(ubuntu-drivers devices 2>/dev/null | awk '/nvidia-driver-.*recommended/ {print $3; exit}')"
    fi
    if [ -z "${NVIDIA_DRIVER_PACKAGE}" ]; then
        echo "ERROR: no recommended NVIDIA driver package found. Set NVIDIA_DRIVER_PACKAGE in config.env."
        exit 1
    fi
    NEW_BRANCH="$(echo "${NVIDIA_DRIVER_PACKAGE}" | grep -oE '[0-9]+' | head -n 1)"
    if [ -z "${NEW_BRANCH}" ] || [ "${NEW_BRANCH}" -lt "${NVIDIA_MIN_DRIVER_VERSION}" ]; then
        echo "ERROR: driver package '${NVIDIA_DRIVER_PACKAGE}' is older than the required ${NVIDIA_MIN_DRIVER_VERSION}."
        exit 1
    fi

    if [ "${CHECK_ONLY}" = "true" ]; then
        echo "CHECK: would install linux-headers-$(uname -r) (if missing) and ${NVIDIA_DRIVER_PACKAGE}"
        if [ -n "${INSTALLED_BRANCH}" ]; then
            echo "CHECK: would purge driver branch ${INSTALLED_BRANCH} packages:"
            driver_branch_packages "${INSTALLED_BRANCH}" | sed 's/^/         /'
        fi
        RC_LIST="$(residual_config_packages)"
        if [ -n "${RC_LIST}" ]; then
            echo "CHECK: would purge residual configuration of:"
            echo "${RC_LIST}" | sed 's/^/         /'
        fi
        if mokutil --sb-state 2>/dev/null | grep -qi 'enabled'; then
            echo "CHECK: Secure Boot is enabled; MOK enrollment will be needed at reboot."
        fi
    fi
fi

if [ "${CHECK_ONLY}" = "true" ]; then
    if toolkit_installed; then
        echo "CHECK: NVIDIA container toolkit already installed."
    else
        echo "CHECK: would install nvidia-container-toolkit from the NVIDIA apt repository."
    fi
    if [ "${REBOOT_REQUIRED}" = "true" ] || [ -n "${NEW_BRANCH}" ]; then
        echo "CHECK: a reboot would be required afterwards."
    fi
    exit 0
fi

if [ -n "${NEW_BRANCH}" ]; then
    echo "Installing NVIDIA driver package ${NVIDIA_DRIVER_PACKAGE}"

    # Record the currently installed NVIDIA packages so they can be restored
    mkdir -p "${PACKAGE_BACKUP_DIR}"
    BACKUP_FILE="${PACKAGE_BACKUP_DIR}/nvidia-packages-$(date +%Y%m%d-%H%M%S).txt"
    dpkg-query -W -f='${Package}:${Architecture}=${Version} ${Status}\n' 2>/dev/null \
        | awk '$NF == "installed" {print $1}' \
        | grep -E '^(nvidia-|libnvidia-|xserver-xorg-video-nvidia-)' \
        | grep -vE '^(nvidia-container-|libnvidia-container)' > "${BACKUP_FILE}"
    echo "Installed NVIDIA packages recorded in ${BACKUP_FILE}"

    # Kernel headers are needed to build the kernel module (DKMS)
    apt-get update
    apt-get install -y "linux-headers-$(uname -r)"

    # Remove the old driver branch packages
    if [ -n "${INSTALLED_BRANCH}" ]; then
        OLD_BRANCH_PACKAGES="$(driver_branch_packages "${INSTALLED_BRANCH}")"
        if [ -n "${OLD_BRANCH_PACKAGES}" ]; then
            echo "Removing NVIDIA driver branch ${INSTALLED_BRANCH} packages:"
            echo "${OLD_BRANCH_PACKAGES}"
            # shellcheck disable=SC2086
            apt-get purge -y ${OLD_BRANCH_PACKAGES}
        fi
    fi
    RC_PACKAGES="$(residual_config_packages)"
    if [ -n "${RC_PACKAGES}" ]; then
        # shellcheck disable=SC2086
        apt-get purge -y ${RC_PACKAGES}
    fi

    if ! apt-get install -y "${NVIDIA_DRIVER_PACKAGE}"; then
        echo "ERROR: installing ${NVIDIA_DRIVER_PACKAGE} failed. Restoring the previous NVIDIA packages from ${BACKUP_FILE}"
        if [ -s "${BACKUP_FILE}" ]; then
            # shellcheck disable=SC2046
            apt-get install -y $(cat "${BACKUP_FILE}")
        fi
        exit 1
    fi

    if command -v mokutil > /dev/null 2>&1 && mokutil --sb-state 2>/dev/null | grep -qi 'enabled'; then
        echo "NOTE: Secure Boot is enabled. Enroll the module signing key (MOK) when prompted during the reboot."
    fi
    REBOOT_REQUIRED=true
fi

# ---------------------------------------------------------------------------
# Container toolkit
# ---------------------------------------------------------------------------
TOOLKIT_NEWLY_INSTALLED=false
if toolkit_installed; then
    echo "NVIDIA container toolkit is already installed."
else
    echo "Installing NVIDIA container toolkit"
    apt-get install -y curl gnupg ca-certificates
    curl -fsSL https://nvidia.github.io/libnvidia-container/gpgkey \
        | gpg --dearmor --yes -o "${TOOLKIT_KEYRING}"
    curl -fsSL https://nvidia.github.io/libnvidia-container/stable/deb/nvidia-container-toolkit.list \
        | sed "s#deb https://#deb [signed-by=${TOOLKIT_KEYRING}] https://#g" \
        > "${TOOLKIT_SOURCES_LIST}"
    apt-get update
    if ! apt-get install -y nvidia-container-toolkit; then
        echo "ERROR: installing nvidia-container-toolkit failed."
        exit 1
    fi
    TOOLKIT_NEWLY_INSTALLED=true
fi

# ---------------------------------------------------------------------------
# k3s runtime registration
# k3s registers the "nvidia" containerd runtime when it starts and finds nvidia-container-runtime.
# ---------------------------------------------------------------------------
K3S_SERVICE=""
if systemctl is-active --quiet k3s; then
    K3S_SERVICE="k3s"
elif systemctl is-active --quiet k3s-agent; then
    K3S_SERVICE="k3s-agent"
fi

if [ "${REBOOT_REQUIRED}" = "true" ]; then
    echo ""
    echo "REBOOT REQUIRED to load the NVIDIA driver. k3s registers the nvidia runtime on its next start."
    echo "Reboot, then re-run this script to verify."
    exit ${EXIT_REBOOT_REQUIRED}
fi

if [ -n "${K3S_SERVICE}" ]; then
    if [ "${TOOLKIT_NEWLY_INSTALLED}" = "true" ]; then
        if [ "${RESTART_K3S}" = "true" ]; then
            echo "Restarting ${K3S_SERVICE} so it registers the nvidia runtime (running pods are not stopped)"
            systemctl restart "${K3S_SERVICE}"
            sleep 15
        else
            echo "k3s must be restarted to register the nvidia runtime. Re-run with --restart-k3s, or restart k3s yourself."
            exit 0
        fi
    fi
    if grep -q 'nvidia' "${K3S_CONTAINERD_CONFIG}" 2>/dev/null; then
        echo "k3s containerd has the nvidia runtime registered."
    else
        echo "WARNING: nvidia runtime not found in ${K3S_CONTAINERD_CONFIG}. Restart k3s and check again."
    fi
fi

if command -v nvidia-smi > /dev/null 2>&1; then
    nvidia-smi --query-gpu=name,driver_version,memory.total --format=csv
fi
echo "NVIDIA host setup complete."
exit 0
