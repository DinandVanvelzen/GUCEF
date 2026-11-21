#!/bin/bash
# ---------------------------------------------------------------------------
# Builds gucefIrcHub from the checkout this script is in and makes the image
# gucef-irc-hub:<version> for k3s. Idempotent: a re-run builds what changed;
# the version is a hash of the built files, so unchanged files give the same
# image, which is not built or imported again.
#
#   - build: Release, relocatable, in ~/.cache/gucef-irc-hub/build
#   - image: the program, its libraries and the JSON plugin on the Ubuntu
#     release of this machine (the libraries of the build need its C++ runtime)
#   - k3s: there is no registry, so the image is imported into the containerd
#     of k3s on this machine (sudo k3s ctr images import). The hub runs on this
#     node (nodeSelector)
#
# Prints the image name on the last line of its output.
#
# Usage: build-image.sh [--force-import]
#   --force-import  import the image into k3s also when it was imported before
#                   (k3s was reinstalled, or its images were removed)
# ---------------------------------------------------------------------------

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GUCEF_ROOT="$(cd "${SCRIPT_DIR}/../../../.." && pwd)"
CACHE_DIR="${XDG_CACHE_HOME:-${HOME}/.cache}/gucef-irc-hub"
BUILD_DIR="${CACHE_DIR}/build"
CONTEXT_DIR="${CACHE_DIR}/image"
TARGET_DIR="${GUCEF_ROOT}/projects/CMake/targets/GUCEF_exe_gucefIrcHub"
FORCE_IMPORT=false
[ "$1" = "--force-import" ] && FORCE_IMPORT=true

fail()
{
    echo "ERROR: $1" >&2
    [ -n "$2" ] && echo "       $2" >&2
    exit 1
}

MISSING=()
command -v cmake >/dev/null 2>&1 || MISSING+=(cmake)
command -v ninja >/dev/null 2>&1 || MISSING+=(ninja-build)
command -v g++ >/dev/null 2>&1 || MISSING+=(build-essential)
command -v objdump >/dev/null 2>&1 || MISSING+=(binutils)
[ ${#MISSING[@]} -eq 0 ] || fail "Missing build tools." "sudo apt-get install -y ${MISSING[*]}"
command -v docker >/dev/null 2>&1 || fail "docker is not installed." "It builds the image of the hub."
docker info >/dev/null 2>&1 || fail "docker does not answer." "Is your user in the docker group? sudo usermod -aG docker \$USER, then log in again."
command -v k3s >/dev/null 2>&1 || fail "k3s is not installed on this machine." "Run this on the node the hub runs on (IRC_NODE)."
[ -d "${TARGET_DIR}" ] || fail "The build target ${TARGET_DIR} does not exist."

mkdir -p "${CACHE_DIR}"
echo "Building gucefIrcHub from ${GUCEF_ROOT} (log: ${CACHE_DIR}/build.log)"
# the generated CMake files of GUCEF ask for CMake 2.6; CMake 4 refuses that without a policy minimum
cmake -S "${TARGET_DIR}" -B "${BUILD_DIR}" -G Ninja \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON \
    -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON '-DCMAKE_INSTALL_RPATH=$ORIGIN/../lib' > "${CACHE_DIR}/build.log" 2>&1 \
    && cmake --build "${BUILD_DIR}" >> "${CACHE_DIR}/build.log" 2>&1
if [ $? -ne 0 ]; then
    grep -E ' error|Error [0-9]|FAILED' "${CACHE_DIR}/build.log" | head -20 >&2
    fail "The build failed, see ${CACHE_DIR}/build.log."
fi

rm -rf "${CONTEXT_DIR}"
mkdir -p "${CONTEXT_DIR}"
"${GUCEF_ROOT}/projects/k3s/cluster-node-installation-media/stage.sh" \
    --doc "${GUCEF_ROOT}/tools/gucefIrcHub/README.md:README.md" \
    "${BUILD_DIR}" "${CONTEXT_DIR}/stage" gucefIrcHub libdstorepluginJSONPARSER.so >/dev/null || fail "Staging the build failed."
cp "${SCRIPT_DIR}/hub.dockerfile" "${CONTEXT_DIR}/Dockerfile"

UBUNTU_VERSION="$(. /etc/os-release && echo "${VERSION_ID}")"
VERSION="$( (cd "${CONTEXT_DIR}" && find . -type f -print0 | sort -z | xargs -0 sha256sum; echo "${UBUNTU_VERSION}") | sha256sum | cut -c1-12)"
IMAGE="gucef-irc-hub:${VERSION}"

if ! docker image inspect "${IMAGE}" >/dev/null 2>&1; then
    echo "Building image ${IMAGE} (Ubuntu ${UBUNTU_VERSION})"
    docker build -q --build-arg UBUNTU_VERSION="${UBUNTU_VERSION}" -t "${IMAGE}" "${CONTEXT_DIR}" >/dev/null || fail "docker build failed."
    # older versions of the image are of no use to docker; k3s keeps its own copies
    docker images gucef-irc-hub --format '{{.Repository}}:{{.Tag}}' | grep -vx "${IMAGE}" | xargs -r docker rmi >/dev/null 2>&1
fi

MARKER="${CACHE_DIR}/imported-${VERSION}"
if [ ! -f "${MARKER}" ] || [ "${FORCE_IMPORT}" = "true" ]; then
    echo "Importing ${IMAGE} into k3s (sudo k3s ctr images import)"
    docker save "${IMAGE}" | sudo k3s ctr images import - >/dev/null || fail "Importing the image into k3s failed."
    rm -f "${CACHE_DIR}"/imported-*
    touch "${MARKER}"
fi
echo "${IMAGE}"
