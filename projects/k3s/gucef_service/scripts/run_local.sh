#!/bin/bash
# Copyright (C) 1998 - 2026.  Dinand Vanvelzen. Licensed under the Apache License, Version 2.0.
#
# Starts the image of a GUCEF service on this machine with docker, with the restrictions of the k3s Deployment: user
# 10001, read-only root file system, no capabilities, no privilege escalation, /tmp a tmpfs, the data directory at /data,
# the site settings mounted read-only. For tests and for running a service without a cluster.
#
# usage: run_local.sh --env <service.env> <data directory> [--tag TAG] [--name NAME] [--foreground] [--config FILE]
#                     [--bind ADDRESS] [--uid UID[:GID]] [--dry-run]
#   data directory   made when missing; becomes /data. One process writes it: do not start a second container on it
#   --tag TAG        default IMAGE_TAG of the service file
#   --name NAME      container name, default SERVICE_NAME
#   --foreground     run in the foreground (Ctrl-C stops the service in order); default: in the background
#   --config FILE    the site settings file; default SITE_CONFIG_SOURCE of the service file
#   --bind ADDRESS   the host address the session and pairing ports are published on, default 127.0.0.1 (0.0.0.0 for
#                    the LAN). The health port is always published on 127.0.0.1 only; the admin port never (use
#                    docker exec, as kubectl exec in the cluster)
#   --uid UID[:GID]  the user in the container; default 10001:10001 when the data directory belongs to 10001, otherwise
#                    its owner (docker cannot give the directory to 10001 without root)
#   --dry-run        print the docker command; run nothing
# The stop timeout is TERMINATION_GRACE_SECONDS of the service file, as in the cluster.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=lib.sh
source "$SCRIPT_DIR/lib.sh"

usage() {
    sed -n '2,/^set -euo pipefail/p' "${BASH_SOURCE[0]}" | sed '$d' | sed 's/^# \{0,1\}//'
}

ENV_FILE=""
DATA=""
TAG=""
NAME=""
FOREGROUND=false
CONFIG=""
BIND="127.0.0.1"
RUN_UID=""
DRY_RUN=false
while [ $# -gt 0 ]; do
    case "$1" in
        --env) ENV_FILE="${2:?--env needs a file}"; shift ;;
        --tag) TAG="${2:?--tag needs a value}"; shift ;;
        --name) NAME="${2:?--name needs a value}"; shift ;;
        --foreground) FOREGROUND=true ;;
        --config) CONFIG="${2:?--config needs a file}"; shift ;;
        --bind) BIND="${2:?--bind needs an address}"; shift ;;
        --uid) RUN_UID="${2:?--uid needs a value}"; shift ;;
        --dry-run) DRY_RUN=true ;;
        -h|--help) usage; exit 0 ;;
        -*) echo "run_local: unknown option $1" >&2; usage >&2; exit 2 ;;
        *) [ -z "$DATA" ] || { echo "run_local: more than one data directory given" >&2; exit 2; }; DATA="$1" ;;
    esac
    shift
done
[ -n "$ENV_FILE" ] || { echo "run_local: --env is required" >&2; exit 2; }
[ -n "$DATA" ] || { echo "run_local: the data directory is required" >&2; exit 2; }
gs_load_env "$ENV_FILE"
gs_require SERVICE_NAME SERVICE_PROGRAM IMAGE_NAME IMAGE_TAG SESSION_PORT PAIRING_PORT HEALTH_PORT SITE_CONFIG_NAME TERMINATION_GRACE_SECONDS TMP_SIZE
IMAGE="$IMAGE_NAME:${TAG:-$IMAGE_TAG}"
NAME="${NAME:-$SERVICE_NAME}"
CONFIG="${CONFIG:-$(gs_env_path "${SITE_CONFIG_SOURCE:-}")}"
[ -f "$CONFIG" ] || gs_fail "the site settings file $CONFIG does not exist"
CONFIG="$(cd "$(dirname "$CONFIG")" && pwd)/$(basename "$CONFIG")"

case "$TMP_SIZE" in
    *Gi) TMPFS_SIZE="${TMP_SIZE%Gi}g" ;;
    *Mi) TMPFS_SIZE="${TMP_SIZE%Mi}m" ;;
    *) gs_fail "TMP_SIZE '$TMP_SIZE' is not <n>Mi or <n>Gi" ;;
esac

mkdir -p "$DATA"
DATA="$(cd "$DATA" && pwd)"
if [ -z "$RUN_UID" ]; then
    owner="$(stat -c '%u:%g' "$DATA")"
    if [ "${owner%%:*}" = "10001" ]; then RUN_UID="10001:10001"; else RUN_UID="$owner"; fi
fi

COMMAND=( docker run --name "$NAME" --init
          --user "$RUN_UID" --read-only --cap-drop ALL --security-opt no-new-privileges
          --tmpfs "/tmp:rw,nosuid,nodev,size=$TMPFS_SIZE"
          --stop-timeout "$TERMINATION_GRACE_SECONDS" --stop-signal SIGTERM
          -e GUCEF_CONSOLE_LOGGER=stderr
          -v "$DATA:/data"
          -v "$CONFIG:/opt/$SERVICE_NAME/config/site/$SITE_CONFIG_NAME:ro"
          -p "$BIND:$SESSION_PORT:$SESSION_PORT" -p "$BIND:$PAIRING_PORT:$PAIRING_PORT"
          -p "127.0.0.1:$HEALTH_PORT:$HEALTH_PORT" )
$FOREGROUND && COMMAND+=( --rm -it ) || COMMAND+=( -d )
COMMAND+=( "$IMAGE" )
# shellcheck disable=SC2206
COMMAND+=( ${CONTAINER_ARGS:-} )

if $DRY_RUN; then
    printf '%q ' "${COMMAND[@]}"
    echo
    exit 0
fi
command -v docker > /dev/null || gs_fail "docker is not installed"
docker image inspect "$IMAGE" > /dev/null 2>&1 || gs_fail "the image $IMAGE is not in docker; build it with build_image.sh --env $ENV_FILE --tag ${TAG:-$IMAGE_TAG} --no-import"
if docker container inspect "$NAME" > /dev/null 2>&1; then
    [ "$(docker container inspect -f '{{.State.Running}}' "$NAME")" != "true" ] || gs_fail "the container $NAME is running; stop it with: docker stop $NAME"
    docker rm "$NAME" > /dev/null
fi
"${COMMAND[@]}"
$FOREGROUND || echo "run_local: started $NAME; logs: docker logs -f $NAME; stop (drains): docker stop $NAME"
