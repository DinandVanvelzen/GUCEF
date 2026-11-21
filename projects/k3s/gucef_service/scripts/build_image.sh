#!/bin/bash
# Copyright (C) 1998 - 2026.  Dinand Vanvelzen. Licensed under the Apache License, Version 2.0.
#
# Builds the container image of a GUCEF service and delivers it: imported into k3s (docker save, then k3s ctr images
# import, after the pattern of projects/k3s/gucef_dev/create_github_gucef_dev_base_image.sh), or pushed to a registry.
#
# usage: build_image.sh --env <service.env> [--tag TAG] [--dev-image IMAGE] [--runtime-base REPOSITORY] [--no-import]
#                       [--save FILE] [--pull-base] [--builder auto|docker|buildkit]
#                       [--registry HOST[:PORT][/PATH] [--image-name NAME]
#                        [--push [--also-tag TAG]... [--digest-file FILE]]] [--dry-run]
#   --env FILE         the service file (service.env.example documents every key)
#   --tag TAG          the image tag (default: the short hashes of the checkouts joined with '-', with -dirty when one
#                      has uncommitted changes)
#   --dev-image IMAGE  the GUCEF development image of the build stage (default vanvelzen/github-gucef-dev-base:latest);
#                      with buildkit it may be a registry reference, such as 172.16.1.125:32001/ci/gucef-dev-base:latest
#   --runtime-base REPOSITORY  the repository of the runtime base image, used with the Debian release of the development
#                      image as its tag (default bitnami/minideb; for example 172.16.1.125:32001/ci/minideb)
#   --no-import        build only; docker keeps the image, k3s does not get it
#   --save FILE        keep the image as a docker archive FILE (for a k3s host that is not this machine: copy it there and
#                      run sudo k3s ctr images import FILE); with buildkit only without --push
#   --pull-base        allow docker to download the runtime base image when it is missing; nothing else is downloaded
#                      by docker for this script (the image build installs packages with install_packages). BuildKit
#                      resolves the base images in their registry at every build; the option makes no difference there
#   --builder ENGINE   the engine that builds the image:
#                        auto      (default) docker when it is installed, else buildkit when buildctl is installed
#                        docker    the local docker daemon (the local flow)
#                        buildkit  buildctl against the BuildKit daemon of BUILDKIT_HOST: no container daemon needed
#   --registry R       a registry and path, HOST[:PORT][/PATH]: the image becomes <R>/<name>:<tag>
#   --image-name NAME  the image name (below the registry). Default with --registry: IMAGE_REGISTRY_NAME of the service
#                      file, else the last path component of IMAGE_NAME (vanvelzen/asset-store-service gives
#                      <registry>/asset-store-service); without --registry: IMAGE_NAME
#   --push             push the image to the registry after the build (needs --registry); nothing is imported into k3s
#   --also-tag TAG     with --push, repeatable: push the same image under this tag as well (for example candidate)
#   --digest-file FILE with --push: write the digest of the pushed image manifest to FILE, as its first and only line
#                      sha256:<64 hex digits>
#   --dry-run          check the service file and the targets, render the Dockerfile, print the build command; run
#                      neither the builder nor k3s
#
# Delivery of the image:
#   docker, without --push    imported into k3s unless --no-import (the local flow); --save keeps the archive
#   docker, with --push       docker push of every tag; no import
#   buildkit, with --push     buildctl build --output type=image,name=<every tag>,push=true,registry.insecure=true
#   buildkit, without --push  needs --no-import (BuildKit never imports into k3s); --save FILE writes a docker archive,
#                             otherwise the build is a check only and its result stays in the BuildKit cache
#
# Registry login: REGISTRY_USER and REGISTRY_TOKEN_FILE (a file that holds the token, mode 0600). The script reads the
# token itself and writes a docker configuration file (config.json, mode 0600) into a private temporary directory, which
# DOCKER_CONFIG names for docker push and for buildctl (which also pulls registry base images with it). The token is never
# on a command line and never printed. Without REGISTRY_USER the builder uses the credentials it has already.
# The registry speaks plain HTTP: buildctl pushes with registry.insecure=true and its daemon lists the registry as http in
# buildkitd.toml (projects/k3s/forgejo/runner/setup.sh --oci); a docker daemon needs the registry in "insecure-registries"
# of its daemon.json.
#
# The script refuses to start, before it calls the builder, when a target of IMAGE_TARGETS has no build files below
# projects/CMake/targets of TARGETS_CHECKOUT, or when the bootstrap file is missing.
# Environment: GUCEF_HOME (default: the checkout this script is in); SERVICE_BUILD_SCRATCH, where the context and the
# archive are made (default $TMPDIR or /tmp; the context is a copy of the checkouts, use a disk with room for it);
# BUILDKIT_HOST, the address of the BuildKit daemon for buildctl (default of buildctl: unix:///run/buildkit/buildkitd.sock;
# tcp://127.0.0.1:1234 for example); REGISTRY_USER and REGISTRY_TOKEN_FILE, see above.
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
usage_error() {
    echo "build_image: $*" >&2
    exit 2
}
TAG_PATTERN='^[A-Za-z0-9_][A-Za-z0-9_.-]{0,127}$'

ENV_FILE=""
TAG=""
DEV_IMAGE="vanvelzen/github-gucef-dev-base:latest"
RUNTIME_BASE="bitnami/minideb"
RUNTIME_BASE_GIVEN=false
DO_IMPORT=true
SAVE_FILE=""
PULL_BASE=false
DRY_RUN=false
BUILDER="auto"
REGISTRY=""
NAME_GIVEN=""
PUSH=false
ALSO_TAGS=()
DIGEST_FILE=""
while [ $# -gt 0 ]; do
    case "$1" in
        --env) ENV_FILE="${2:?--env needs a file}"; shift ;;
        --tag) TAG="${2:?--tag needs a value}"; shift ;;
        --dev-image) DEV_IMAGE="${2:?--dev-image needs a value}"; shift ;;
        --runtime-base) RUNTIME_BASE="${2:?--runtime-base needs a repository}"; RUNTIME_BASE_GIVEN=true; shift ;;
        --no-import) DO_IMPORT=false ;;
        --save) SAVE_FILE="${2:?--save needs a file}"; shift ;;
        --pull-base) PULL_BASE=true ;;
        --builder) BUILDER="${2:?--builder needs auto, docker or buildkit}"; shift ;;
        --registry) REGISTRY="${2:?--registry needs HOST[:PORT][/PATH]}"; shift ;;
        --image-name) NAME_GIVEN="${2:?--image-name needs a name}"; shift ;;
        --push) PUSH=true ;;
        --also-tag) ALSO_TAGS+=( "${2:?--also-tag needs a tag}" ); shift ;;
        --digest-file) DIGEST_FILE="${2:?--digest-file needs a file}"; shift ;;
        --dry-run) DRY_RUN=true ;;
        -h|--help) usage; exit 0 ;;
        *) echo "build_image: unknown option $1" >&2; usage >&2; exit 2 ;;
    esac
    shift
done
[ -n "$ENV_FILE" ] || { echo "build_image: --env is required" >&2; usage >&2; exit 2; }
if [ -n "$TAG" ] && ! [[ "$TAG" =~ $TAG_PATTERN ]]; then
    echo "build_image: the tag '$TAG' is not valid: letters, digits, '_', '.' and '-', at most 128 characters" >&2
    exit 2
fi
for extra_tag in "${ALSO_TAGS[@]}"; do
    [[ "$extra_tag" =~ $TAG_PATTERN ]] || usage_error "the tag '$extra_tag' of --also-tag is not valid: letters, digits, '_', '.' and '-', at most 128 characters"
done
case "$BUILDER" in
    auto|docker|buildkit) ;;
    *) usage_error "--builder takes auto, docker or buildkit, not '$BUILDER'" ;;
esac
[ -z "$REGISTRY" ] || gs_registry_valid "$REGISTRY" || usage_error "--registry '$REGISTRY' is not HOST[:PORT][/PATH] (for example 172.16.1.125:32001/ci)"
! $PUSH || [ -n "$REGISTRY" ] || usage_error "--push needs --registry"
[ ${#ALSO_TAGS[@]} -eq 0 ] || $PUSH || usage_error "--also-tag needs --push"
[ -z "$DIGEST_FILE" ] || $PUSH || usage_error "--digest-file needs --push"
if [ -n "$DIGEST_FILE" ] && [ ! -d "$(dirname "$DIGEST_FILE")" ]; then
    usage_error "the directory of --digest-file $DIGEST_FILE does not exist"
fi
[[ "$RUNTIME_BASE" =~ ^[A-Za-z0-9][A-Za-z0-9.:/_-]*$ ]] && [[ "$RUNTIME_BASE" != *@* ]] || usage_error "--runtime-base '$RUNTIME_BASE' is not an image repository (without tag or digest)"

# the builder: auto takes what is installed; a dry run without either shows the docker command, as before
if [ "$BUILDER" = "auto" ]; then
    if command -v docker > /dev/null 2>&1; then
        BUILDER=docker
    elif command -v buildctl > /dev/null 2>&1; then
        BUILDER=buildkit
    elif $DRY_RUN; then
        BUILDER=docker
        echo "build_image: note: neither docker nor buildctl is installed; the dry run shows the docker command" >&2
    else
        gs_fail "--builder auto: neither docker nor buildctl (BuildKit) is installed; install one, or give --builder"
    fi
fi
if [ "$BUILDER" = "buildkit" ]; then
    if ! $PUSH && $DO_IMPORT; then
        usage_error "the buildkit builder does not import into k3s: give --push (with --registry), or --no-import (with --save FILE for an archive to import)"
    fi
    if $PUSH && [ -n "$SAVE_FILE" ]; then
        usage_error "with the buildkit builder --save and --push exclude each other (one output per build)"
    fi
fi
# a pushed image is pulled by the nodes: never imported
! $PUSH || DO_IMPORT=false

# the registry login: both or neither
REGISTRY_USER="${REGISTRY_USER:-}"
REGISTRY_TOKEN_FILE="${REGISTRY_TOKEN_FILE:-}"
if [ -n "$REGISTRY_USER" ] || [ -n "$REGISTRY_TOKEN_FILE" ]; then
    [ -n "$REGISTRY" ] || usage_error "REGISTRY_USER and REGISTRY_TOKEN_FILE need --registry"
    [ -n "$REGISTRY_USER" ] && [ -n "$REGISTRY_TOKEN_FILE" ] || usage_error "the registry login needs both REGISTRY_USER and REGISTRY_TOKEN_FILE"
    [[ "$REGISTRY_USER" =~ ^[A-Za-z0-9][A-Za-z0-9._-]{0,63}$ ]] || usage_error "REGISTRY_USER '$REGISTRY_USER' is not a user name (letters, digits, '.', '_', '-')"
    [ -f "$REGISTRY_TOKEN_FILE" ] && [ -r "$REGISTRY_TOKEN_FILE" ] || gs_fail "REGISTRY_TOKEN_FILE $REGISTRY_TOKEN_FILE is not a readable file"
    if [ -n "$(find "$REGISTRY_TOKEN_FILE" -maxdepth 0 -perm /044 2> /dev/null)" ]; then
        echo "build_image: warning: REGISTRY_TOKEN_FILE $REGISTRY_TOKEN_FILE is readable by others than its owner; chmod 600 it" >&2
    fi
fi

gs_load_env "$ENV_FILE"
gs_require SERVICE_NAME SERVICE_PROGRAM IMAGE_NAME IMAGE_TARGETS TARGETS_CHECKOUT BOOTSTRAP_INI
# the ports are optional: a service that listens on none (a client of other servers) exposes none
EXPOSE_PORTS=""
for port_key in SESSION_PORT PAIRING_PORT HEALTH_PORT; do
    port_value="${!port_key:-}"
    [ -z "$port_value" ] || EXPOSE_PORTS="${EXPOSE_PORTS:+$EXPOSE_PORTS }$port_value"
done
if [ -n "$EXPOSE_PORTS" ]; then
    EXPOSE_LINE="EXPOSE $EXPOSE_PORTS"
else
    EXPOSE_LINE="# no port is exposed"
fi
export EXPOSE_LINE
for dir in ${PLUGIN_LINKS:-}; do
    case "$dir" in
        bin|lib) ;;
        *) gs_fail "PLUGIN_LINKS takes bin and lib, not '$dir'" ;;
    esac
done
for extra in ${EXTRA_PROGRAMS:-}; do
    [[ "$extra" =~ ^[A-Za-z0-9][A-Za-z0-9._-]*=[A-Za-z0-9][A-Za-z0-9._/-]*$ ]] && [[ "$extra" != *..* ]] || gs_fail "EXTRA_PROGRAMS takes name=directory (relative to the targets checkout), not '$extra'"
done
NAME="$IMAGE_NAME"
[ -z "$REGISTRY" ] || NAME="$(gs_registry_image_name)"
NAME="${NAME_GIVEN:-$NAME}"
[[ "$NAME" =~ ^[a-z0-9]+([._-][a-z0-9]+)*(/[a-z0-9]+([._-][a-z0-9]+)*)*$ ]] || gs_fail "the image name '$NAME' is not an image repository name (lower case path components)"

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
REPOSITORY="${REGISTRY:+$REGISTRY/}$NAME"
IMAGE="$REPOSITORY:$TAG"
REFS=( "$IMAGE" )
for extra_tag in "${ALSO_TAGS[@]}"; do
    REFS+=( "$REPOSITORY:$extra_tag" )
done
REFS_CSV="$(IFS=,; echo "${REFS[*]}")"

SCRATCH="$(mktemp -d "${SERVICE_BUILD_SCRATCH:-${TMPDIR:-/tmp}}/gucef-service-image.XXXXXX")"
trap 'rm -rf "$SCRATCH"' EXIT
gs_render "$DOCKERFILE_TEMPLATE" "$SCRATCH/Dockerfile"

BUILD_ARGS=( --build-arg "DEV_IMAGE=$DEV_IMAGE" --build-arg "IMAGE_TARGETS=$IMAGE_TARGETS"
             --build-arg "TARGETS_CHECKOUT=$TARGETS_CHECKOUT" --build-arg "REVISION=$REVISION"
             --build-arg "PLUGIN_LINKS=${PLUGIN_LINKS:-}" --build-arg "EXTRA_PROGRAMS=${EXTRA_PROGRAMS:-}" )
# the runtime base goes to the build only when it is given, so the docker command of the local flow stays as it was
! $RUNTIME_BASE_GIVEN || BUILD_ARGS+=( --build-arg "RUNTIME_BASE=$RUNTIME_BASE" )
# the same build arguments as options of buildctl
BUILDKIT_ARGS=()
for (( i = 1; i < ${#BUILD_ARGS[@]}; i += 2 )); do
    BUILDKIT_ARGS+=( --opt "build-arg:${BUILD_ARGS[$i]}" )
done
# the image output of buildctl: every name, pushed; the registry speaks plain HTTP
if [ ${#REFS[@]} -gt 1 ]; then
    BUILDKIT_PUSH_OUTPUT="type=image,\"name=$REFS_CSV\",push=true,registry.insecure=true"
else
    BUILDKIT_PUSH_OUTPUT="type=image,name=$IMAGE,push=true,registry.insecure=true"
fi
AUTH_NOTE=""
[ -z "$REGISTRY_USER" ] || AUTH_NOTE="DOCKER_CONFIG=<private directory: config.json for $REGISTRY_USER on ${REGISTRY%%/*}> "

if $DRY_RUN; then
    echo "build_image: dry run for $IMAGE (builder $BUILDER)"
    echo "build_image: context: GUCEF=$GUCEF_HOME ${EXTRA_ARGS[*]:-}, service/bootstrap.ini from $BOOTSTRAP_FILE"
    if [ "$BUILDER" = "docker" ]; then
        TAG_ARGS=""
        for ref in "${REFS[@]}"; do TAG_ARGS="$TAG_ARGS -t $ref"; done
        echo "build_image: docker build -f <rendered $(basename "$DOCKERFILE_TEMPLATE")> ${BUILD_ARGS[*]} --build-arg MINIDEB_RELEASE=<of $DEV_IMAGE> --build-arg RUNTIME_PACKAGES=<base set + ${RUNTIME_EXTRA_PACKAGES:-}>$TAG_ARGS <context>"
        if $PUSH; then
            for ref in "${REFS[@]}"; do echo "build_image: ${AUTH_NOTE}docker push $ref"; done
        elif $DO_IMPORT; then
            echo "build_image: docker save, then k3s ctr images import (sudo)"
        fi
    else
        echo "build_image: ${AUTH_NOTE}buildctl build --progress=plain --frontend dockerfile.v0 --local context=<release probe> --local dockerfile=<release probe> --opt build-arg:DEV_IMAGE=$DEV_IMAGE --output type=local,dest=<release probe>/out"
        output="<none: a check build>"
        if $PUSH; then
            output="$BUILDKIT_PUSH_OUTPUT --metadata-file <metadata>"
        elif [ -n "$SAVE_FILE" ]; then
            output="type=docker,name=$IMAGE,dest=$SAVE_FILE"
        fi
        echo "build_image: ${AUTH_NOTE}buildctl build --progress=plain --frontend dockerfile.v0 --local context=<context> --local dockerfile=<rendered $(basename "$DOCKERFILE_TEMPLATE")> ${BUILDKIT_ARGS[*]} --opt build-arg:MINIDEB_RELEASE=<of $DEV_IMAGE> --opt build-arg:RUNTIME_PACKAGES=<base set + ${RUNTIME_EXTRA_PACKAGES:-}> --output $output"
        echo "build_image: BUILDKIT_HOST=${BUILDKIT_HOST:-<buildctl default>}"
    fi
    [ -z "$DIGEST_FILE" ] || echo "build_image: the digest of the pushed manifest goes to $DIGEST_FILE"
    cp "$SCRATCH/Dockerfile" "${DRY_RUN_DOCKERFILE:-/dev/null}"
    exit 0
fi

if [ "$BUILDER" = "docker" ]; then
    TOOLS="docker git rsync"
else
    TOOLS="buildctl git rsync"
fi
for tool in $TOOLS; do
    command -v "$tool" > /dev/null || gs_fail "$tool is not installed"
done
SUDO=""
if $DO_IMPORT; then
    command -v k3s > /dev/null || gs_fail "k3s is not installed on this machine; use --no-import and --save"
    [ "$(id -u)" = "0" ] || SUDO="sudo"
fi

# the registry login: a docker configuration file in a private directory, for docker push and buildctl
AUTH_ENV=()
if [ -n "$REGISTRY_USER" ]; then
    token=""
    IFS= read -r token < "$REGISTRY_TOKEN_FILE" || [ -n "$token" ] || true
    token="${token%$'\r'}"
    [ -n "$token" ] || gs_fail "REGISTRY_TOKEN_FILE $REGISTRY_TOKEN_FILE holds no token on its first line"
    mkdir -m 700 "$SCRATCH/registry-auth"
    # printf is a shell builtin: the token reaches base64 through a pipe, never through a command line
    auth="$(printf '%s:%s' "$REGISTRY_USER" "$token" | base64 -w0)"
    ( umask 077 && printf '{\n    "auths": {\n        "%s": { "auth": "%s" }\n    }\n}\n' "${REGISTRY%%/*}" "$auth" > "$SCRATCH/registry-auth/config.json" )
    unset token auth
    AUTH_ENV=( env "DOCKER_CONFIG=$SCRATCH/registry-auth" )
fi

# 3. the release and the libssl runtime package of the development image
if [ "$BUILDER" = "docker" ]; then
    docker image inspect "$DEV_IMAGE" > /dev/null 2>&1 || gs_fail "the development image $DEV_IMAGE is not in docker; build it with projects/k3s/gucef_dev/create_github_gucef_dev_base_image.sh"
    MINIDEB_RELEASE="$(docker run --rm --entrypoint /bin/sh "$DEV_IMAGE" -c '. /etc/os-release; echo "$VERSION_CODENAME"' | tr -d '\r\n')"
    [ -n "$MINIDEB_RELEASE" ] || gs_fail "cannot read the Debian release of $DEV_IMAGE"
    SSL_PACKAGE="$(docker run --rm --entrypoint /bin/sh "$DEV_IMAGE" -c "dpkg-query -W -f='\${Package}\n' 'libssl3*' 2> /dev/null | head -n 1" | tr -d '\r\n')"
    [ -n "$SSL_PACKAGE" ] || gs_fail "cannot find the libssl3 runtime package in $DEV_IMAGE"
else
    # BuildKit runs no container outside a build: a small build of its own reads both from the development image and
    # writes them to a local directory
    PROBE="$SCRATCH/release-probe"
    mkdir -p "$PROBE"
    cat > "$PROBE/Dockerfile" <<'PROBE'
ARG DEV_IMAGE
FROM ${DEV_IMAGE} AS probe
RUN mkdir -p /probe \
 && . /etc/os-release && echo "$VERSION_CODENAME" > /probe/release \
 && { dpkg-query -W -f='${Package}\n' 'libssl3*' 2> /dev/null | head -n 1; } > /probe/ssl
FROM scratch
COPY --from=probe /probe/ /
PROBE
    "${AUTH_ENV[@]}" buildctl build --progress=plain --frontend dockerfile.v0 --local "context=$PROBE" --local "dockerfile=$PROBE" \
        --opt "build-arg:DEV_IMAGE=$DEV_IMAGE" --output "type=local,dest=$PROBE/out" \
        || gs_fail "the BuildKit daemon (BUILDKIT_HOST=${BUILDKIT_HOST:-<buildctl default>}) could not read the development image $DEV_IMAGE"
    MINIDEB_RELEASE="$(tr -d '\r\n' < "$PROBE/out/release" 2> /dev/null || true)"
    [ -n "$MINIDEB_RELEASE" ] || gs_fail "cannot read the Debian release of $DEV_IMAGE"
    SSL_PACKAGE="$(tr -d '\r\n' < "$PROBE/out/ssl" 2> /dev/null || true)"
    [ -n "$SSL_PACKAGE" ] || gs_fail "cannot find the libssl3 runtime package in $DEV_IMAGE"
fi
# the C++ runtime, OpenSSL, the CA certificates, libX11 (libgucefCORE links it), and what the service adds
RUNTIME_PACKAGES="ca-certificates libstdc++6 $SSL_PACKAGE libx11-6 ${RUNTIME_EXTRA_PACKAGES:-}"
if [ "$BUILDER" = "docker" ] && ! docker image inspect "$RUNTIME_BASE:$MINIDEB_RELEASE" > /dev/null 2>&1; then
    $PULL_BASE || gs_fail "the runtime base image $RUNTIME_BASE:$MINIDEB_RELEASE is not in docker; run again with --pull-base to download it"
    docker pull "$RUNTIME_BASE:$MINIDEB_RELEASE"
fi

# 4. the context and the build
GUCEF_HOME="$GUCEF_HOME" "$SCRIPT_DIR/make_context.sh" "$SCRATCH/context" "${EXTRA_ARGS[@]}"
mkdir -p "$SCRATCH/context/service"
cp "$BOOTSTRAP_FILE" "$SCRATCH/context/service/bootstrap.ini"
DIGEST=""
if [ "$BUILDER" = "docker" ]; then
    TAG_ARGS=()
    for ref in "${REFS[@]}"; do TAG_ARGS+=( -t "$ref" ); done
    docker build -f "$SCRATCH/Dockerfile" "${BUILD_ARGS[@]}" \
        --build-arg "MINIDEB_RELEASE=$MINIDEB_RELEASE" \
        --build-arg "RUNTIME_PACKAGES=$RUNTIME_PACKAGES" \
        "${TAG_ARGS[@]}" "$SCRATCH/context"
    rm -rf "$SCRATCH/context"
    if $PUSH; then
        for ref in "${REFS[@]}"; do
            "${AUTH_ENV[@]}" docker push "$ref" || gs_fail "docker push $ref failed (a plain HTTP registry must be in insecure-registries of the docker daemon)"
        done
        DIGEST="$(docker image inspect --format '{{range .RepoDigests}}{{println .}}{{end}}' "$IMAGE" | grep -F "$REPOSITORY@sha256:" | sed 's/^.*@//' | grep -x 'sha256:[0-9a-f]\{64\}' | head -n 1 || true)"
    fi
else
    mkdir -p "$SCRATCH/dockerfile"
    cp "$SCRATCH/Dockerfile" "$SCRATCH/dockerfile/Dockerfile"
    OUTPUT=()
    if $PUSH; then
        OUTPUT=( --output "$BUILDKIT_PUSH_OUTPUT" --metadata-file "$SCRATCH/metadata.json" )
    elif [ -n "$SAVE_FILE" ]; then
        OUTPUT=( --output "type=docker,name=$IMAGE,dest=$SAVE_FILE" )
    fi
    "${AUTH_ENV[@]}" buildctl build --progress=plain --frontend dockerfile.v0 \
        --local "context=$SCRATCH/context" --local "dockerfile=$SCRATCH/dockerfile" \
        "${BUILDKIT_ARGS[@]}" \
        --opt "build-arg:MINIDEB_RELEASE=$MINIDEB_RELEASE" \
        --opt "build-arg:RUNTIME_PACKAGES=$RUNTIME_PACKAGES" \
        "${OUTPUT[@]}"
    rm -rf "$SCRATCH/context"
    if $PUSH; then
        DIGEST="$(grep -o '"containerimage.digest": *"sha256:[0-9a-f]\{64\}"' "$SCRATCH/metadata.json" 2> /dev/null | grep -o 'sha256:[0-9a-f]\{64\}' | head -n 1 || true)"
    fi
fi
if $PUSH; then
    [ -n "$DIGEST" ] || gs_fail "the image was pushed, but the digest of its manifest is not known"
    if [ -n "$DIGEST_FILE" ]; then
        printf '%s\n' "$DIGEST" > "$DIGEST_FILE.tmp.$$"
        mv "$DIGEST_FILE.tmp.$$" "$DIGEST_FILE"
    fi
    for ref in "${REFS[@]}"; do
        echo "build_image: pushed $ref"
    done
    echo "build_image: digest $DIGEST"
    echo "build_image: in the service file: IMAGE_REGISTRY=$REGISTRY, IMAGE_REGISTRY_NAME=$NAME (when it is not the last path component of IMAGE_NAME) and IMAGE_TAG=$DIGEST (or $TAG); then render the manifests again"
    exit 0
fi

# 5. save and import (docker); the archive of buildkit is written by the build
if [ "$BUILDER" = "docker" ] && { $DO_IMPORT || [ -n "$SAVE_FILE" ]; }; then
    ARCHIVE="${SAVE_FILE:-$SCRATCH/$SERVICE_NAME-$TAG.tar}"
    docker save --output "$ARCHIVE" "$IMAGE"
    if $DO_IMPORT; then
        $SUDO k3s ctr images import "$ARCHIVE"
        $SUDO k3s ctr images ls | grep -F "$REPOSITORY:$TAG" || true
    fi
fi
[ -z "$SAVE_FILE" ] || echo "build_image: archive kept as $SAVE_FILE"
if [ -n "$REGISTRY" ]; then
    echo "build_image: image $IMAGE (not pushed)"
else
    echo "build_image: image $IMAGE (containerd: docker.io/$IMAGE)"
fi
echo "build_image: set IMAGE_TAG=$TAG in the service file and render the manifests again (render_manifests.sh)"
