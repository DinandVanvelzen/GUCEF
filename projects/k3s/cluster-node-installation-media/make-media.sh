#!/bin/bash
# ---------------------------------------------------------------------------
# Makes an install medium for the machines of the home lab: a directory, e.g.
# on a USB stick, with packages of the services and an install.sh that
# installs and sets them up. On a machine of the lab network:
#
#   sudo bash /media/<user>/<stick>/gucef-lab/install.sh
#
# and the machine finds the cluster, joins it as a node (Linux) and gets the
# MCP services of the nodes in Claude Code, as projects/k3s/lablink does from
# a checkout, without building there.
#
#   - build: Release builds of gucefMcpLocalLlm and gucefLabLink from this
#     checkout, relocatable, per architecture, in
#     ~/.cache/gucef-cluster-node-installation-media. The architecture of this
#     machine is built here; arm64 on an amd64 machine is cross compiled in a
#     container (docker, cross/arm64.Dockerfile)
#   - test: the test applications of both; a failure makes no medium. arm64
#     tests on an amd64 machine run when QEMU runs arm64 programs
#     (binfmt_misc, Ubuntu: sudo apt-get install qemu-user-static), otherwise
#     they are skipped
#   - packages (CPack, packaging/): gucef-k3s-scripts, and gucef-mcp and
#     gucef-lablink per architecture, as DEB, and as RPM when rpmbuild is
#     there (sudo apt-get install rpm)
#   - medium: TARGET/gucef-lab with linux/ (the packages), docs/ (the README
#     of each service and its program), install.sh, README.txt, SHA256SUMS;
#     lablink.env with the settings of lablink/config.env of this checkout
#     that apply to every machine (LAB_ALLOW and the like); with
#     --with-secret also the lab secret
#
# Usage: ./make-media.sh [TARGET] [--arch LIST] [--with-secret] [--skip-tests]
#   TARGET         the directory the medium goes in, e.g. the mount point of a
#                  USB stick; default: common/bin/cluster-node-installation-media
#                  of this checkout
#   --arch LIST    comma separated architectures: amd64, arm64; default
#                  amd64,arm64 (arm64: Raspberry Pi, Orange Pi 5, Banana Pi
#                  and other 64-bit ARM boards with a 64-bit OS)
#   --with-secret  put the lab secret of this cluster (sudo
#                  gucefLabLink --init made it) on the medium: machines link
#                  without approval. Whoever has the medium has the secret
#   --skip-tests   make the medium without running the tests
#
# Internal: --packages-for ARCH --work-dir DIR --version VERSION builds,
# tests and packages the programs for ARCH in the current environment.
# ---------------------------------------------------------------------------

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
K3S_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
GUCEF_ROOT="$(cd "${K3S_DIR}/../.." && pwd)"
TARGETS_DIR="${GUCEF_ROOT}/projects/CMake/targets"
LAB_STATE_DIR="/var/lib/gucef-lablink"

TARGET=""
ARCHES="amd64,arm64"
WITH_SECRET=false
SKIP_TESTS=false
PACKAGES_FOR=""
WORK_DIR=""
VERSION=""
while [ $# -gt 0 ]; do
    case "$1" in
        --arch) ARCHES="$2"; shift ;;
        --with-secret) WITH_SECRET=true ;;
        --skip-tests) SKIP_TESTS=true ;;
        --packages-for) PACKAGES_FOR="$2"; shift ;;
        --work-dir) WORK_DIR="$2"; shift ;;
        --version) VERSION="$2"; shift ;;
        -*) echo "Unknown option: $1"; exit 1 ;;
        *) TARGET="$1" ;;
    esac
    shift
done
TARGET="${TARGET:-${GUCEF_ROOT}/common/bin/cluster-node-installation-media}"

# builds run as the user, also under sudo
BUILD_USER="$(id -un 2>/dev/null)"
[ "$(id -u)" -eq 0 ] && [ -n "${SUDO_USER}" ] && BUILD_USER="${SUDO_USER}"
BUILD_UID="$(id -u "${BUILD_USER}" 2>/dev/null || id -u)"
as_builder() { if [ "$(id -u)" = "${BUILD_UID}" ]; then "$@"; else sudo -u "${BUILD_USER}" -H "$@"; fi; }
as_root() { if [ "$(id -u)" -eq 0 ]; then "$@"; else sudo "$@"; fi; }
BUILDER_HOME="$(getent passwd "${BUILD_USER}" 2>/dev/null | cut -d: -f6)"
BUILDER_HOME="${BUILDER_HOME:-${HOME}}"
WORK_DIR="${WORK_DIR:-${BUILDER_HOME}/.cache/gucef-cluster-node-installation-media}"
LOG_DIR="${WORK_DIR}/logs"
PACKAGES_DIR="${WORK_DIR}/packages"

HOST_ARCH="$(dpkg --print-architecture 2>/dev/null)"
if [ -z "${HOST_ARCH}" ]; then
    case "$(uname -m)" in x86_64) HOST_ARCH=amd64 ;; aarch64) HOST_ARCH=arm64 ;; *) HOST_ARCH="$(uname -m)" ;; esac
fi
multiarch_triplet() { case "$1" in amd64) echo x86_64-linux-gnu ;; arm64) echo aarch64-linux-gnu ;; esac; }

# ---------------------------------------------------------------------------
# The program packages of one architecture: build, test, stage, package
# ---------------------------------------------------------------------------
program_packages() {
    local arch="$1" build_root="${WORK_DIR}/build/$1" dist_dir="${WORK_DIR}/dist/$1"
    local cmake_args=() tools_prefix="" glibc
    if [ "${arch}" != "${HOST_ARCH}" ]; then
        tools_prefix="$(multiarch_triplet "${arch}")-"
        cmake_args=("-DCMAKE_TOOLCHAIN_FILE=${SCRIPT_DIR}/cross/$(multiarch_triplet "${arch}").cmake")
    fi
    export OBJDUMP="${tools_prefix}objdump" STRIP="${tools_prefix}strip"

    build() {
        local name="$1" project="$2"; shift 2
        echo "Building ${name} (${arch})"
        # relocatable: the installed copy finds its libraries next to itself
        as_builder cmake -S "${TARGETS_DIR}/${project}" -B "${build_root}/${name}" -G Ninja "${cmake_args[@]}" \
            -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON '-DCMAKE_INSTALL_RPATH=$ORIGIN/../lib' \
            > "${LOG_DIR}/build-${name}-${arch}.log" 2>&1 &&
        local targets=()
        [ $# -gt 0 ] && targets=(--target "$@")
        as_builder cmake --build "${build_root}/${name}" "${targets[@]}" >> "${LOG_DIR}/build-${name}-${arch}.log" 2>&1
        if [ $? -ne 0 ]; then
            grep -E 'error' "${LOG_DIR}/build-${name}-${arch}.log" | head -20
            echo "ERROR: the build of ${name} (${arch}) failed, see ${LOG_DIR}/build-${name}-${arch}.log"
            exit 1
        fi
    }

    run_test() {
        local name="$1" program="$2"; shift 2
        echo "Testing ${name} (${arch})"
        if ! ( cd "${build_root}/${name}/bin" && as_builder env "$@" "./${program}" > "${LOG_DIR}/test-${name}-${arch}.log" 2>&1 ); then
            python3 - "${build_root}/${name}/bin/${program}_Results.xml" <<'PYEOF' 2>/dev/null
import sys, xml.etree.ElementTree as ET
for case in ET.parse(sys.argv[1]).getroot().iter("testcase"):
    for result in case:
        if result.tag == "failure":
            print("  failed: " + case.get("name", ""))
            break
PYEOF
            echo "ERROR: the tests of ${name} (${arch}) failed, see ${LOG_DIR}/test-${name}-${arch}.log"
            exit 1
        fi
    }

    build gucef-mcp GUCEF_exe_gucefMcpLocalLlm_TestApp
    build gucef-lablink GUCEF_exe_gucefLabLink_TestApp gucefLabLink gucefLabLink_TestApp codecspluginMONOCYPHER
    if [ "${SKIP_TESTS}" != "true" ]; then
        run_test gucef-mcp gucefMcpLocalLlm_TestApp LOCAL_AGENT_SCRIPT="${K3S_DIR}/llm/clients/local-agent.sh" \
            LOCAL_LLM_URL= LOCAL_LLM_API_KEY= PATH="${BUILDER_HOME}/.local/bin:${BUILDER_HOME}/.opencode/bin:${BUILDER_HOME}/bin:${PATH}"
        run_test gucef-lablink gucefLabLink_TestApp
        echo "Tests passed (${arch})."
    fi

    as_builder rm -rf "${dist_dir}" "${WORK_DIR}/package/${arch}"
    as_builder "${SCRIPT_DIR}/stage.sh" \
        --doc "${K3S_DIR}/llm/mcp/README.md" --doc "${GUCEF_ROOT}/tools/gucefMcpLocalLlm/README.md:gucefMcpLocalLlm.md" \
        "${build_root}/gucef-mcp" "${dist_dir}/gucef-mcp" gucefMcpLocalLlm libdstorepluginJSONPARSER.so || exit 1
    as_builder install -m 755 "${K3S_DIR}/llm/clients/local-agent.sh" "${dist_dir}/gucef-mcp/bin/"
    as_builder "${SCRIPT_DIR}/stage.sh" \
        --doc "${K3S_DIR}/lablink/README.md" --doc "${GUCEF_ROOT}/tools/gucefLabLink/README.md:gucefLabLink.md" \
        "${build_root}/gucef-lablink" "${dist_dir}/gucef-lablink" gucefLabLink libcodecspluginMONOCYPHER.so || exit 1

    # the newest glibc the programs need, for the package dependencies
    glibc="$(find "${dist_dir}" -type f \( -path '*/bin/*' -o -name '*.so*' \) -exec "${OBJDUMP}" -T {} + 2>/dev/null | \
        grep -o 'GLIBC_[0-9.]*' | sed 's/GLIBC_//' | sort -uV | tail -1)"
    echo "${glibc}" > "${dist_dir}/glibc"

    # the system libraries of a staged program: "DEB-DEPENDS|RPM-REQUIRES", found from the libraries it links against
    system_dependencies() {
        local dir="$1" so path owner deb="" rpm="" own triplet
        own="$(ls "${dir}/lib")"
        triplet="$(multiarch_triplet "${arch}")"
        for so in $(find "${dir}" -type f \( -path '*/bin/*' -o -name '*.so*' \) -exec "${OBJDUMP}" -p {} + 2>/dev/null | awk '/NEEDED/ {print $2}' | sort -u); do
            echo "${own}" | grep -qx "${so}" && continue
            rpm="${rpm}, ${so}()(64bit)"
            [ "${so}" = "libc.so.6" ] && [ -n "${glibc}" ] && rpm="${rpm}, libc.so.6(GLIBC_${glibc})(64bit)"
            owner=""
            if command -v dpkg > /dev/null 2>&1; then
                for path in "/lib/${triplet}/${so}" "/usr/lib/${triplet}/${so}"; do
                    [ -e "${path}" ] || continue
                    for candidate in "${path}" "$(readlink -f "${path}")"; do
                        owner="$(dpkg -S "${candidate}" 2>/dev/null | head -1 | cut -d: -f1)"
                        [ -n "${owner}" ] && break 2
                    done
                done
            fi
            if [ -z "${owner}" ]; then
                echo "WARNING: no DEB package found for ${so} (${arch}); the DEB packages do not depend on it." >&2
                continue
            fi
            [ "${owner}" = "libc6" ] && [ -n "${glibc}" ] && owner="libc6 (>= ${glibc})"
            case ", ${deb}, " in *", ${owner}, "*) ;; *) deb="${deb}, ${owner}" ;; esac
        done
        echo "${deb#, }|${rpm#, }"
    }

    local name libraries
    for name in gucef-mcp gucef-lablink; do
        echo "Packaging ${name} ${VERSION} (${arch})"
        libraries="$(system_dependencies "${dist_dir}/${name}")"
        as_builder cmake -S "${SCRIPT_DIR}/packaging" -B "${WORK_DIR}/package/${arch}/${name}" \
            -DGUCEF_PACKAGE="${name}" -DGUCEF_PACKAGE_VERSION="${VERSION}" -DGUCEF_PACKAGE_SOURCE="${dist_dir}/${name}" \
            -DGUCEF_PACKAGE_ARCH="${arch}" \
            "-DGUCEF_PACKAGE_DEB_LIBRARIES=${libraries%%|*}" "-DGUCEF_PACKAGE_RPM_LIBRARIES=${libraries#*|}" \
            -DGUCEF_PACKAGE_OUTPUT="${PACKAGES_DIR}" > "${LOG_DIR}/package-${name}-${arch}.log" 2>&1 &&
        ( cd "${WORK_DIR}/package/${arch}/${name}" && as_builder cpack >> "${LOG_DIR}/package-${name}-${arch}.log" 2>&1 )
        if [ $? -ne 0 ]; then
            tail -20 "${LOG_DIR}/package-${name}-${arch}.log"
            echo "ERROR: packaging ${name} (${arch}) failed, see ${LOG_DIR}/package-${name}-${arch}.log"
            exit 1
        fi
    done
}

if [ -n "${PACKAGES_FOR}" ]; then
    [ -n "${VERSION}" ] || { echo "ERROR: --packages-for needs --version"; exit 1; }
    program_packages "${PACKAGES_FOR}"
    exit 0
fi

# ---------------------------------------------------------------------------
# Checks
# ---------------------------------------------------------------------------
MISSING=()
for tool in cmake cpack ninja g++ strip objdump sha256sum; do
    command -v "${tool}" > /dev/null 2>&1 || MISSING+=("${tool}")
done
if [ ${#MISSING[@]} -gt 0 ]; then
    echo "ERROR: these are needed: ${MISSING[*]} (Ubuntu: sudo apt-get install cmake ninja-build build-essential binutils)"
    exit 1
fi
command -v rpmbuild > /dev/null 2>&1 || echo "NOTE: rpmbuild is not there, so no RPM packages (Ubuntu: sudo apt-get install rpm)."

ARCH_LIST=()
for arch in ${ARCHES//,/ }; do
    case "${arch}" in
        amd64|arm64) ;;
        *) echo "ERROR: unknown architecture ${arch}: amd64 or arm64"; exit 1 ;;
    esac
    if [ "${arch}" != "${HOST_ARCH}" ]; then
        if [ ! -f "${SCRIPT_DIR}/cross/${arch}.Dockerfile" ]; then
            echo "ERROR: ${arch} cannot be built on this ${HOST_ARCH} machine; build it on a ${arch} machine (--arch ${arch})."
            exit 1
        fi
        if ! as_builder docker info > /dev/null 2>&1; then
            echo "ERROR: ${arch} is cross compiled in a container: docker is needed, usable by ${BUILD_USER}, or leave ${arch} out (--arch ${HOST_ARCH})."
            exit 1
        fi
    fi
    ARCH_LIST+=("${arch}")
done

LAB_SECRET=""
if [ "${WITH_SECRET}" = "true" ]; then
    LAB_SECRET="$(as_root cat "${LAB_STATE_DIR}/secret" 2>/dev/null | tr -d '[:space:]')"
    if [ -z "${LAB_SECRET}" ]; then
        echo "ERROR: there is no lab secret in ${LAB_STATE_DIR} on this machine: run --with-secret on the cluster (lablink/install.sh made it)."
        exit 1
    fi
fi

as_builder mkdir -p "${LOG_DIR}" || exit 1
as_builder rm -rf "${PACKAGES_DIR}" "${WORK_DIR}/package" "${WORK_DIR}/dist"
VERSION="1.$(git -C "${GUCEF_ROOT}" rev-list --count HEAD 2>/dev/null || echo 0).$(date -u +%Y%m%d%H%M)"

# ---------------------------------------------------------------------------
# Packages
# ---------------------------------------------------------------------------
echo "Packaging gucef-k3s-scripts ${VERSION}"
as_builder cmake -S "${SCRIPT_DIR}/packaging" -B "${WORK_DIR}/package/all/gucef-k3s-scripts" \
    -DGUCEF_PACKAGE=gucef-k3s-scripts -DGUCEF_PACKAGE_VERSION="${VERSION}" -DGUCEF_PACKAGE_SOURCE="${K3S_DIR}" \
    -DGUCEF_PACKAGE_OUTPUT="${PACKAGES_DIR}" > "${LOG_DIR}/package-gucef-k3s-scripts.log" 2>&1 &&
( cd "${WORK_DIR}/package/all/gucef-k3s-scripts" && as_builder cpack >> "${LOG_DIR}/package-gucef-k3s-scripts.log" 2>&1 )
if [ $? -ne 0 ]; then
    tail -20 "${LOG_DIR}/package-gucef-k3s-scripts.log"
    echo "ERROR: packaging gucef-k3s-scripts failed, see ${LOG_DIR}/package-gucef-k3s-scripts.log"
    exit 1
fi

UNTESTED=()
for arch in "${ARCH_LIST[@]}"; do
    if [ "${arch}" = "${HOST_ARCH}" ]; then
        ( program_packages "${arch}" ) || exit 1
        continue
    fi
    image="gucef-cross-${arch}"
    echo "Preparing the ${arch} build container"
    if ! as_builder docker build -q -t "${image}" -f "${SCRIPT_DIR}/cross/${arch}.Dockerfile" "${SCRIPT_DIR}/cross" > "${LOG_DIR}/container-${arch}.log" 2>&1; then
        tail -20 "${LOG_DIR}/container-${arch}.log"
        echo "ERROR: the ${arch} build container failed, see ${LOG_DIR}/container-${arch}.log"
        exit 1
    fi
    test_option=()
    if [ "${SKIP_TESTS}" = "true" ]; then
        test_option=(--skip-tests)
    elif ! grep -qs '^enabled' "/proc/sys/fs/binfmt_misc/qemu-$(multiarch_triplet "${arch}" | cut -d- -f1)"; then
        echo "NOTE: this machine does not run ${arch} programs (Ubuntu: sudo apt-get install qemu-user-static), so the ${arch} tests are skipped."
        test_option=(--skip-tests)
        UNTESTED+=("${arch}")
    fi
    # the checkout read-only and the work directory at the same paths as here
    as_builder docker run --rm -u "${BUILD_UID}:$(id -g "${BUILD_USER}")" -e HOME=/tmp \
        -v "${GUCEF_ROOT}:${GUCEF_ROOT}:ro" -v "${WORK_DIR}:${WORK_DIR}" "${image}" \
        bash "${SCRIPT_DIR}/make-media.sh" --packages-for "${arch}" --work-dir "${WORK_DIR}" --version "${VERSION}" "${test_option[@]}" \
        || exit 1
done

# ---------------------------------------------------------------------------
# Medium
# ---------------------------------------------------------------------------
MEDIUM_DIR="${TARGET}/gucef-lab"
if ! mkdir -p "${MEDIUM_DIR}/linux" "${MEDIUM_DIR}/docs/gucef-mcp" "${MEDIUM_DIR}/docs/gucef-lablink" 2> /dev/null; then
    echo "ERROR: cannot write ${MEDIUM_DIR}"
    exit 1
fi
rm -f "${MEDIUM_DIR}"/linux/*.deb "${MEDIUM_DIR}"/linux/*.rpm "${MEDIUM_DIR}/lab-secret" "${MEDIUM_DIR}/lablink.env"
cp "${PACKAGES_DIR}"/*.deb "${MEDIUM_DIR}/linux/" || exit 1
cp "${PACKAGES_DIR}"/*.rpm "${MEDIUM_DIR}/linux/" 2> /dev/null
cp "${SCRIPT_DIR}/medium-install.sh" "${MEDIUM_DIR}/install.sh"
cp "${SCRIPT_DIR}/medium-README.txt" "${MEDIUM_DIR}/README.txt"
cp "${K3S_DIR}/llm/mcp/README.md" "${MEDIUM_DIR}/docs/gucef-mcp/README.md"
cp "${GUCEF_ROOT}/tools/gucefMcpLocalLlm/README.md" "${MEDIUM_DIR}/docs/gucef-mcp/gucefMcpLocalLlm.md"
cp "${K3S_DIR}/lablink/README.md" "${MEDIUM_DIR}/docs/gucef-lablink/README.md"
cp "${GUCEF_ROOT}/tools/gucefLabLink/README.md" "${MEDIUM_DIR}/docs/gucef-lablink/gucefLabLink.md"
chmod 755 "${MEDIUM_DIR}/install.sh" 2> /dev/null

# the lab link settings of this checkout that apply to every machine; the role, name and user are each machine's own
if [ -f "${K3S_DIR}/lablink/config.env" ]; then
    grep -vE '^[[:space:]]*(LAB_ROLE|LAB_CLUSTER_NAME|LAB_CLAUDE_USER)=' "${K3S_DIR}/lablink/config.env" > "${MEDIUM_DIR}/lablink.env"
    grep -qE '^[[:space:]]*LAB_' "${MEDIUM_DIR}/lablink.env" || rm -f "${MEDIUM_DIR}/lablink.env"
fi
if [ -n "${LAB_SECRET}" ]; then
    ( umask 077 && echo "${LAB_SECRET}" > "${MEDIUM_DIR}/lab-secret" )
fi
( cd "${MEDIUM_DIR}" && find . -type f ! -name SHA256SUMS -printf '%P\n' | sort | xargs sha256sum > SHA256SUMS )
[ "$(id -u)" -eq 0 ] && [ "${BUILD_USER}" != "root" ] && chown -R "${BUILD_USER}" "${MEDIUM_DIR}" 2> /dev/null
sync

echo ""
echo "Install medium ${VERSION} in ${MEDIUM_DIR}:"
( cd "${MEDIUM_DIR}" && find . -type f -printf '  %P\n' | sort )
for arch in "${ARCH_LIST[@]}"; do
    echo "${arch}: built on glibc $(cat "${WORK_DIR}/dist/${arch}/glibc" 2>/dev/null): for Linux distributions with that glibc or newer."
done
[ ${#UNTESTED[@]} -gt 0 ] && echo "Not tested: ${UNTESTED[*]}."
echo "On a machine of the lab network:  sudo bash <where the medium is mounted>/gucef-lab/install.sh"
if [ -n "${LAB_SECRET}" ]; then
    echo "The medium holds the lab secret: machines link without approval. Keep it like a key; delete lab-secret from it when done."
else
    echo "Without the lab secret on it, install.sh asks for it; a machine without it waits for approval on the cluster."
fi
exit 0
