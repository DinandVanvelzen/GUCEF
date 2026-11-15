#!/bin/bash
# Copyright (C) 1998 - 2026.  Dinand Vanvelzen. Licensed under the Apache License, Version 2.0.
#
# Builds the container image of a GUCEF service with docker and imports it into k3s (docker save, then
# k3s ctr images import), after the pattern of projects/k3s/gucef_dev/create_github_gucef_dev_base_image.sh.
#
# usage: build_image.sh --env <service.env> [--tag TAG] [--dev-image IMAGE] [--no-import] [--save FILE] [--pull-base]
#                       [--dry-run]
#   --env FILE         the service file (service.env.example documents every key)
#   --tag TAG          the image tag (default: the short hashes of the checkouts joined with '-', with -dirty when one
#                      has uncommitted changes)
#   --dev-image IMAGE  the GUCEF development image of the build stage (default vanvelzen/github-gucef-dev-base:latest)
#   --no-import        build only; docker keeps the image, k3s does not get it
#   --save FILE        keep the docker save archive as FILE (for a k3s host that is not this machine:
#                      copy it there and run sudo k3s ctr images import FILE)
#   --pull-base        allow docker to download the runtime base image when it is missing; nothing else is downloaded
#                      by this script (the image build installs packages with install_packages)
#   --dry-run          check the service file and the targets, render the Dockerfile, print the docker command; run
#                      neither docker nor k3s
#
# The script refuses to start, before it calls docker, when a target of IMAGE_TARGETS has no build files below
# projects/CMake/targets of TARGETS_CHECKOUT, or when the bootstrap file is missing.
# Environment: GUCEF_HOME (default: the checkout this script is in); SERVICE_BUILD_SCRATCH, where the context and the
# archive are made (default $TMPDIR or /tmp; the context is a copy of the checkouts, use a disk with room for it).
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=lib.sh
source "$SCRIPT_DIR/lib.sh"
KIT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
GUCEF_HOME="${GUCEF_HOME:-$SCRIPT_DIR/../../../..}"
GUCEF_HOME="$(cd "$GUCEF_HOME" && pwd)"

usage() {
    sed -n '2,/^set -euo pipefail/p' "${BASH_SOURCE[0]}" | sed '$d' | sed 's/^# \{0,1\}//'
}

ENV_FILE=""
TAG=""
DEV_IMAGE="vanvelzen/github-gucef-dev-base:latest"
RUNTIME_BASE="bitnami/minideb"
DO_IMPORT=true
SAVE_FILE=""
PULL_BASE=false
DRY_RUN=false
while [ $# -gt 0 ]; do
    case "$1" in
        --env) ENV_FILE="${2:?--env needs a file}"; shift ;;
        --tag) TAG="${2:?--tag needs a value}"; shift ;;
        --dev-image) DEV_IMAGE="${2:?--dev-image needs a value}"; shift ;;
        --no-import) DO_IMPORT=false ;;
        --save) SAVE_FILE="${2:?--save needs a file}"; shift ;;
        --pull-base) PULL_BASE=true ;;
        --dry-run) DRY_RUN=true ;;
        -h|--help) usage; exit 0 ;;
        *) echo "build_image: unknown option $1" >&2; usage >&2; exit 2 ;;
    esac
    shift
done
[ -n "$ENV_FILE" ] || { echo "build_image: --env is required" >&2; usage >&2; exit 2; }
if [ -n "$TAG" ] && ! [[ "$TAG" =~ ^[A-Za-z0-9_][A-Za-z0-9_.-]{0,127}$ ]]; then
    echo "build_image: the tag '$TAG' is not valid: letters, digits, '_', '.' and '-', at most 128 characters" >&2
    exit 2
fi

gs_load_env "$ENV_FILE"
gs_require SERVICE_NAME SERVICE_PROGRAM IMAGE_NAME IMAGE_TARGETS TARGETS_CHECKOUT BOOTSTRAP_INI SESSION_PORT PAIRING_PORT HEALTH_PORT

# 1. the checkouts of the context and the targets
declare -A CHECKOUTS=( [GUCEF]="$GUCEF_HOME" )
EXTRA_ARGS=()
for extra in ${EXTRA_CHECKOUTS:-}; do
    [[ "$extra" == *=* ]] || gs_fail "EXTRA_CHECKOUTS: '$extra' is not <name>=<path>"
    path="$(gs_env_path "${extra#*=}")"
    [ -d "$path" ] || gs_fail "EXTRA_CHECKOUTS: $path does not exist"
    path="$(cd "$path" && pwd)"
    CHECKOUTS[${extra%%=*}]="$path"
    EXTRA_ARGS+=( "${extra%%=*}=$path" )
done
TARGETS_HOME="${CHECKOUTS[$TARGETS_CHECKOUT]:-}"
[ -n "$TARGETS_HOME" ] || gs_fail "TARGETS_CHECKOUT '$TARGETS_CHECKOUT' is neither GUCEF nor a name of EXTRA_CHECKOUTS"
missing=""
for target in $IMAGE_TARGETS; do
    [ -d "$TARGETS_HOME/projects/CMake/targets/$target" ] || missing="$missing $target"
done
if [ -n "$missing" ]; then
    echo "build_image: no build files for:$missing" >&2
    echo "build_image: expected below $TARGETS_HOME/projects/CMake/targets/; they are generated from the ModuleInfo.xml" >&2
    echo "build_image: of the program by ProjectGenerator (GUCEF CLAUDE.md, 'Project Files Are Generated')" >&2
    exit 1
fi
BOOTSTRAP_FILE="$(gs_env_path "$BOOTSTRAP_INI")"
[ -f "$BOOTSTRAP_FILE" ] || gs_fail "the bootstrap file $BOOTSTRAP_FILE does not exist"
DOCKERFILE_TEMPLATE="${DOCKERFILE:-}"
if [ -z "$DOCKERFILE_TEMPLATE" ]; then
    DOCKERFILE_TEMPLATE="$KIT_DIR/docker/gucef_service.dockerfile.template"
else
    DOCKERFILE_TEMPLATE="$(gs_env_path "$DOCKERFILE_TEMPLATE")"
fi

# 2. the tag
dirty=""
REVISION=""
for name in GUCEF ${EXTRA_CHECKOUTS:-}; do
    name="${name%%=*}"
    home="${CHECKOUTS[$name]}"
    hash="$(git -C "$home" rev-parse --short=9 HEAD 2> /dev/null || echo nogit)"
    REVISION="${REVISION:+$REVISION-}$hash"
    if [ -n "$(git -C "$home" status --porcelain 2> /dev/null)" ]; then
        dirty="-dirty"
    fi
done
REVISION="$REVISION$dirty"
TAG="${TAG:-$REVISION}"
IMAGE="$IMAGE_NAME:$TAG"

SCRATCH="$(mktemp -d "${SERVICE_BUILD_SCRATCH:-${TMPDIR:-/tmp}}/gucef-service-image.XXXXXX")"
trap 'rm -rf "$SCRATCH"' EXIT
gs_render "$DOCKERFILE_TEMPLATE" "$SCRATCH/Dockerfile"

BUILD_ARGS=( --build-arg "DEV_IMAGE=$DEV_IMAGE" --build-arg "IMAGE_TARGETS=$IMAGE_TARGETS"
             --build-arg "TARGETS_CHECKOUT=$TARGETS_CHECKOUT" --build-arg "REVISION=$REVISION" )

if $DRY_RUN; then
    echo "build_image: dry run for $IMAGE"
    echo "build_image: context: GUCEF=$GUCEF_HOME ${EXTRA_ARGS[*]:-}, service/bootstrap.ini from $BOOTSTRAP_FILE"
    echo "build_image: docker build -f <rendered $(basename "$DOCKERFILE_TEMPLATE")> ${BUILD_ARGS[*]} --build-arg MINIDEB_RELEASE=<of $DEV_IMAGE> --build-arg RUNTIME_PACKAGES=<base set + ${RUNTIME_EXTRA_PACKAGES:-}> -t $IMAGE <context>"
    cp "$SCRATCH/Dockerfile" "${DRY_RUN_DOCKERFILE:-/dev/null}"
    exit 0
fi

for tool in docker git rsync; do
    command -v "$tool" > /dev/null || gs_fail "$tool is not installed"
done
SUDO=""
if $DO_IMPORT; then
    command -v k3s > /dev/null || gs_fail "k3s is not installed on this machine; use --no-import and --save"
    [ "$(id -u)" = "0" ] || SUDO="sudo"
fi

# 3. the release and the libssl runtime package of the development image
docker image inspect "$DEV_IMAGE" > /dev/null 2>&1 || gs_fail "the development image $DEV_IMAGE is not in docker; build it with projects/k3s/gucef_dev/create_github_gucef_dev_base_image.sh"
MINIDEB_RELEASE="$(docker run --rm --entrypoint /bin/sh "$DEV_IMAGE" -c '. /etc/os-release; echo "$VERSION_CODENAME"' | tr -d '\r\n')"
[ -n "$MINIDEB_RELEASE" ] || gs_fail "cannot read the Debian release of $DEV_IMAGE"
SSL_PACKAGE="$(docker run --rm --entrypoint /bin/sh "$DEV_IMAGE" -c "dpkg-query -W -f='\${Package}\n' 'libssl3*' 2> /dev/null | head -n 1" | tr -d '\r\n')"
[ -n "$SSL_PACKAGE" ] || gs_fail "cannot find the libssl3 runtime package in $DEV_IMAGE"
# the C++ runtime, OpenSSL, the CA certificates, libX11 (libgucefCORE links it), and what the service adds
RUNTIME_PACKAGES="ca-certificates libstdc++6 $SSL_PACKAGE libx11-6 ${RUNTIME_EXTRA_PACKAGES:-}"
if ! docker image inspect "$RUNTIME_BASE:$MINIDEB_RELEASE" > /dev/null 2>&1; then
    $PULL_BASE || gs_fail "the runtime base image $RUNTIME_BASE:$MINIDEB_RELEASE is not in docker; run again with --pull-base to download it"
    docker pull "$RUNTIME_BASE:$MINIDEB_RELEASE"
fi

# 4. the context and the build
GUCEF_HOME="$GUCEF_HOME" "$SCRIPT_DIR/make_context.sh" "$SCRATCH/context" "${EXTRA_ARGS[@]}"
mkdir -p "$SCRATCH/context/service"
cp "$BOOTSTRAP_FILE" "$SCRATCH/context/service/bootstrap.ini"
docker build -f "$SCRATCH/Dockerfile" "${BUILD_ARGS[@]}" \
    --build-arg "MINIDEB_RELEASE=$MINIDEB_RELEASE" \
    --build-arg "RUNTIME_PACKAGES=$RUNTIME_PACKAGES" \
    -t "$IMAGE" "$SCRATCH/context"
rm -rf "$SCRATCH/context"

# 5. save and import
if $DO_IMPORT || [ -n "$SAVE_FILE" ]; then
    ARCHIVE="${SAVE_FILE:-$SCRATCH/$SERVICE_NAME-$TAG.tar}"
    docker save --output "$ARCHIVE" "$IMAGE"
    if $DO_IMPORT; then
        $SUDO k3s ctr images import "$ARCHIVE"
        $SUDO k3s ctr images ls | grep -F "$IMAGE_NAME:$TAG" || true
    fi
    [ -z "$SAVE_FILE" ] || echo "build_image: archive kept as $SAVE_FILE"
fi
echo "build_image: image $IMAGE (containerd: docker.io/$IMAGE)"
echo "build_image: set IMAGE_TAG=$TAG in the service file and render the manifests again (render_manifests.sh)"
