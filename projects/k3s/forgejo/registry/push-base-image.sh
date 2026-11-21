#!/bin/bash
# Pushes base images that exist on this machine to the container registry of the cluster, once, so that image builds
# in the cluster (gucef_service/scripts/build_image.sh --builder buildkit) take them from there:
#   the GUCEF development image vanvelzen/github-gucef-dev-base:latest   -> <registry>/<owner>/gucef-dev-base:latest
#   with --minideb TAG: bitnami/minideb:TAG                              -> <registry>/<owner>/minideb:TAG
# The minideb tag stays the Debian release, because build_image.sh --runtime-base <registry>/<owner>/minideb uses the
# release of the development image as the tag.
#
# Usage: registry/push-base-image.sh --token-file FILE [--registry HOST:PORT] [--owner NAME] [--user NAME]
#                                    [--image IMAGE [--name NAME]] [--minideb TAG [--minideb-only]] [--dry-run]
#   --token-file FILE    a token with the scope write:package of a user who may write the packages of the owner; it is
#                        never printed and never on a command line
#   --registry HOST:PORT default FORGEJO_REGISTRY_HOST of ../config.env, else FORGEJO_HOST:FORGEJO_HTTP_PORT of it
#   --owner NAME         the owner (organization) of the images, default ci
#   --user NAME          the user of the token, default FORGEJO_CI_BOT_USER of ../config.env, else ci-bot
#   --image IMAGE        the local image to push (default vanvelzen/github-gucef-dev-base:latest)
#   --name NAME          its name below the owner (default gucef-dev-base for the default image, else the last path
#                        component of IMAGE without its tag); the tag of IMAGE is kept (latest when it has none)
#   --minideb TAG        also push bitnami/minideb:TAG (for example bookworm) as <owner>/minideb:TAG
#   --minideb-only       push only the minideb image
#   --dry-run            checks what it can without the registry and prints the commands; pushes nothing
#
# Tooling: docker (docker tag, then docker push with a private docker configuration that holds the login). The
# registry speaks plain HTTP, so the docker daemon must list it in "insecure-registries" of /etc/docker/daemon.json;
# the script checks that and says what to change, it never changes the daemon. Without docker, the script prints what
# to run with another tool. Nothing is downloaded: the images must exist in docker already.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FORGEJO_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
DEV_IMAGE_DEFAULT="vanvelzen/github-gucef-dev-base:latest"

usage() {
    sed -n '2,/^set -euo pipefail/p' "${BASH_SOURCE[0]}" | sed '$d' | sed 's/^# \{0,1\}//'
}
fail() {
    echo "push-base-image: $*" >&2
    exit 1
}
usage_error() {
    echo "push-base-image: $*" >&2
    exit 2
}

TOKEN_FILE=""
REGISTRY=""
OWNER="ci"
REGISTRY_USER=""
IMAGE="${DEV_IMAGE_DEFAULT}"
NAME=""
MINIDEB_TAG=""
MINIDEB_ONLY=false
DRY_RUN=false
while [ $# -gt 0 ]; do
    case "$1" in
        --token-file) TOKEN_FILE="${2:?--token-file needs a file}"; shift ;;
        --registry) REGISTRY="${2:?--registry needs HOST:PORT}"; shift ;;
        --owner) OWNER="${2:?--owner needs a name}"; shift ;;
        --user) REGISTRY_USER="${2:?--user needs a name}"; shift ;;
        --image) IMAGE="${2:?--image needs an image}"; shift ;;
        --name) NAME="${2:?--name needs a name}"; shift ;;
        --minideb) MINIDEB_TAG="${2:?--minideb needs a tag}"; shift ;;
        --minideb-only) MINIDEB_ONLY=true ;;
        --dry-run) DRY_RUN=true ;;
        -h|--help) usage; exit 0 ;;
        *) echo "push-base-image: unknown option $1" >&2; usage >&2; exit 2 ;;
    esac
    shift
done

config_value() {
    [ -f "${FORGEJO_DIR}/config.env" ] || return 0
    sed -n "s/\r\$//; s/^$1=//p" "${FORGEJO_DIR}/config.env" | tail -n 1 | sed 's/^"\(.*\)"$/\1/; s/^'"'"'\(.*\)'"'"'$/\1/'
}
if [ -z "${REGISTRY}" ]; then
    REGISTRY="$(config_value FORGEJO_REGISTRY_HOST)"
fi
if [ -z "${REGISTRY}" ] && [ -n "$(config_value FORGEJO_HOST)" ]; then
    port="$(config_value FORGEJO_HTTP_PORT)"
    REGISTRY="$(config_value FORGEJO_HOST):${port:-32001}"
fi
[ -n "${REGISTRY}" ] || usage_error "give --registry HOST:PORT (or FORGEJO_REGISTRY_HOST or FORGEJO_HOST in ${FORGEJO_DIR}/config.env)"
[[ "${REGISTRY}" =~ ^[A-Za-z0-9]([A-Za-z0-9.-]{0,252}[A-Za-z0-9])?:[0-9]{1,5}$ ]] || usage_error "--registry '${REGISTRY}' is not HOST:PORT"
[[ "${OWNER}" =~ ^[a-z0-9]+([._-][a-z0-9]+)*$ ]] || usage_error "--owner '${OWNER}' is not a lower case owner name"
REGISTRY_USER="${REGISTRY_USER:-$(config_value FORGEJO_CI_BOT_USER)}"
REGISTRY_USER="${REGISTRY_USER:-ci-bot}"
[[ "${REGISTRY_USER}" =~ ^[A-Za-z0-9][A-Za-z0-9._-]{0,63}$ ]] || usage_error "--user '${REGISTRY_USER}' is not a user name"
[ -n "${TOKEN_FILE}" ] || usage_error "--token-file FILE is required"
[ -f "${TOKEN_FILE}" ] && [ -r "${TOKEN_FILE}" ] || fail "the token file ${TOKEN_FILE} is not a readable file"
! ${MINIDEB_ONLY} || [ -n "${MINIDEB_TAG}" ] || usage_error "--minideb-only needs --minideb TAG"
[ -z "${MINIDEB_TAG}" ] || [[ "${MINIDEB_TAG}" =~ ^[A-Za-z0-9_][A-Za-z0-9_.-]{0,127}$ ]] || usage_error "--minideb '${MINIDEB_TAG}' is not a tag"

# the pairs <local image> <registry reference>
SOURCES=()
TARGETS=()
if ! ${MINIDEB_ONLY}; then
    [[ "${IMAGE}" =~ ^[a-z0-9]+([._/-][a-z0-9]+)*(:[A-Za-z0-9_][A-Za-z0-9_.-]{0,127})?$ ]] || usage_error "--image '${IMAGE}' is not an image name with an optional tag"
    tag="latest"
    repository="${IMAGE}"
    if [[ "${IMAGE##*/}" == *:* ]]; then
        tag="${IMAGE##*:}"
        repository="${IMAGE%:*}"
    fi
    if [ -z "${NAME}" ]; then
        if [ "${IMAGE}" = "${DEV_IMAGE_DEFAULT}" ]; then NAME="gucef-dev-base"; else NAME="${repository##*/}"; fi
    fi
    [[ "${NAME}" =~ ^[a-z0-9]+([._-][a-z0-9]+)*$ ]] || usage_error "--name '${NAME}' is not an image name"
    SOURCES+=( "${repository}:${tag}" )
    TARGETS+=( "${REGISTRY}/${OWNER}/${NAME}:${tag}" )
fi
if [ -n "${MINIDEB_TAG}" ]; then
    SOURCES+=( "bitnami/minideb:${MINIDEB_TAG}" )
    TARGETS+=( "${REGISTRY}/${OWNER}/minideb:${MINIDEB_TAG}" )
fi

if ! command -v docker > /dev/null 2>&1; then
    echo "push-base-image: docker is not installed on this machine; nothing was pushed. With skopeo, from the machine that has the images:" >&2
    for i in "${!SOURCES[@]}"; do
        echo "    skopeo copy --dest-tls-verify=false --dest-creds ${REGISTRY_USER}:<token of ${TOKEN_FILE}> docker-daemon:${SOURCES[$i]} docker://${TARGETS[$i]}" >&2
    done
    echo "  (or docker tag and docker push on a machine with docker, which this script does)" >&2
    exit 1
fi

# the images must be there; nothing is downloaded
missing=""
for source in "${SOURCES[@]}"; do
    docker image inspect "${source}" > /dev/null 2>&1 || missing="${missing} ${source}"
done
if [ -n "${missing}" ]; then
    ${DRY_RUN} || fail "not in docker:${missing} (the development image is built with projects/k3s/gucef_dev/create_github_gucef_dev_base_image.sh; minideb comes with build_image.sh --pull-base)"
    echo "push-base-image: (dry run) not in docker:${missing}"
fi

# the daemon must push to the registry over plain HTTP: the registry is listed in insecure-registries
INSECURE="$(docker info --format '{{range $name, $index := .RegistryConfig.IndexConfigs}}{{if not $index.Secure}}{{$name}}{{"\n"}}{{end}}{{end}}' 2> /dev/null || true)"
if ! grep -q -x -F "${REGISTRY}" <<< "${INSECURE}"; then
    message="the docker daemon does not list ${REGISTRY} in insecure-registries, so it would push over HTTPS and fail. Add it to /etc/docker/daemon.json, for example { \"insecure-registries\": [\"${REGISTRY}\"] } (merged with what the file holds), and restart docker (sudo systemctl restart docker); then run this again"
    ${DRY_RUN} || fail "${message}"
    echo "push-base-image: (dry run) ${message}"
fi

if ${DRY_RUN}; then
    for i in "${!SOURCES[@]}"; do
        echo "push-base-image: (dry run) docker tag ${SOURCES[$i]} ${TARGETS[$i]}"
        echo "push-base-image: (dry run) DOCKER_CONFIG=<private directory: config.json for ${REGISTRY_USER} on ${REGISTRY}> docker push ${TARGETS[$i]}"
    done
    exit 0
fi

# the login: a docker configuration in a private directory; the token passes through a pipe only
WORK="$(mktemp -d "${TMPDIR:-/tmp}/push-base-image.XXXXXX")"
trap 'rm -rf "${WORK}"' EXIT
token=""
IFS= read -r token < "${TOKEN_FILE}" || [ -n "${token}" ] || true
token="${token%$'\r'}"
[ -n "${token}" ] || fail "the token file holds no token on its first line"
auth="$(printf '%s:%s' "${REGISTRY_USER}" "${token}" | base64 -w0)"
( umask 077 && printf '{\n    "auths": {\n        "%s": { "auth": "%s" }\n    }\n}\n' "${REGISTRY}" "${auth}" > "${WORK}/config.json" )
unset token auth

for i in "${!SOURCES[@]}"; do
    docker tag "${SOURCES[$i]}" "${TARGETS[$i]}"
    DOCKER_CONFIG="${WORK}" docker push "${TARGETS[$i]}" || fail "docker push ${TARGETS[$i]} failed"
    digest="$(docker image inspect --format '{{range .RepoDigests}}{{println .}}{{end}}' "${TARGETS[$i]}" | grep -F "${TARGETS[$i]%:*}@" | sed 's/^.*@//' | head -n 1 || true)"
    echo "push-base-image: pushed ${SOURCES[$i]} as ${TARGETS[$i]}${digest:+ (${digest})}"
done
echo "push-base-image: for build_image.sh --builder buildkit:"
! ${MINIDEB_ONLY} && echo "    --dev-image ${TARGETS[0]}"
[ -z "${MINIDEB_TAG}" ] || echo "    --runtime-base ${REGISTRY}/${OWNER}/minideb"
