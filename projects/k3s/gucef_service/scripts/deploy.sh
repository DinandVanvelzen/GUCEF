#!/bin/bash
# Copyright (C) 1998 - 2026.  Dinand Vanvelzen. Licensed under the Apache License, Version 2.0.
#
# Deploys a GUCEF service to the cluster, in order and idempotently (a second run changes nothing that is in place):
#   1. resolves STORAGE_CLASS against the cluster's classes (resolve_storage_class.sh); refuses a class that differs
#      from the class of an existing claim (a claim's class cannot change)
#   2. renders the manifests (render_manifests.sh --storage-class) and builds them with kustomize on this machine
#   3. checks that a node carries the node selector label (and, in lan mode, that it is DISCOVERY_NODE_NAME)
#   4. creates or updates the namespace
#   5. checks the Secret of components/secret-file and its key (SECRET_FILE_SECRET_NAME): missing, it stops here with
#      the command to create it; the value is never read, printed or created by this script
#   6. warns when the image is not in the image list of the node (imagePullPolicy Never: it is imported by the owner);
#      an image of IMAGE_REGISTRY is pulled by the node (imagePullPolicy IfNotPresent) and not checked
#   7. applies the built manifests and waits for the rollout
# It never builds or imports the image and never creates a Secret.
#
# usage: deploy.sh --env <service.env> [--local-dir DIR [--allow-local-in-git]] [--out DIR] [--timeout SECONDS] [--dry-run]
#   --env FILE        the service file
#   --local-dir DIR   the local install configuration of the site (render_manifests.sh, README.md "Local install
#                     configuration"); default: the directory of the service file when it holds a service.env.local
#   --allow-local-in-git  use a local directory that lies inside a git work tree (refused otherwise)
#   --out DIR         where the rendered and built manifests are kept (must not exist or be empty); default a new
#                     directory below ${DEPLOY_WORK_DIR:-$HOME/.cache/gucef-deploy}/<service>/
#   --timeout SECONDS the wait for the rollout; default STARTUP_FAILURE_THRESHOLD x 10 + 120
#   --dry-run         steps 1 to 3 and the read-only checks of 5 and 6; creates and applies nothing
#
# Environment:
#   KUBECTL    the kubectl command prefix, default "kubectl"; for example "ssh k3smaster kubectl". Split on white space
#              into words (no quotes, no escapes, no expansion, never eval'd) and executed: set it yourself, it is never
#              read from a service file. Every argument this script adds is free of white space and shell characters,
#              so a remote shell passes it on unchanged; manifests go to "apply -f -" on standard input.
#   KUSTOMIZE  the local kustomize build command, default "kubectl kustomize" (offline: it reads files only); the built
#              file is what is applied, so the kubectl of KUBECTL needs no access to the rendered directory.
#   DEPLOY_WORK_DIR  see --out
# Exit code 0 when the rollout completed (or the dry run passed), 1 when a step refused or failed, 2 for a usage error.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=lib.sh
source "$SCRIPT_DIR/lib.sh"

usage() {
    sed -n '2,/^set -euo pipefail/p' "${BASH_SOURCE[0]}" | sed '$d' | sed 's/^# \{0,1\}//'
}

ENV_FILE=""
OUT=""
TIMEOUT=""
DRY_RUN=false
LOCAL_DIR=""
ALLOW_LOCAL_IN_GIT=false
while [ $# -gt 0 ]; do
    case "$1" in
        --env) ENV_FILE="${2:?--env needs a file}"; shift ;;
        --out) OUT="${2:?--out needs a directory}"; shift ;;
        --timeout) TIMEOUT="${2:?--timeout needs seconds}"; shift ;;
        --dry-run) DRY_RUN=true ;;
        --local-dir) LOCAL_DIR="${2:?--local-dir needs a directory}"; shift ;;
        --allow-local-in-git) ALLOW_LOCAL_IN_GIT=true ;;
        -h|--help) usage; exit 0 ;;
        *) echo "deploy: unknown argument $1" >&2; usage >&2; exit 2 ;;
    esac
    shift
done
[ -n "$ENV_FILE" ] || { echo "deploy: --env is required" >&2; exit 2; }
[ -z "$TIMEOUT" ] || [[ "$TIMEOUT" =~ ^[1-9][0-9]{0,5}$ ]] || { echo "deploy: --timeout takes seconds" >&2; exit 2; }
gs_load_service "$ENV_FILE" "$LOCAL_DIR" "$ALLOW_LOCAL_IN_GIT"
[ -z "$GS_LOCAL_DIR" ] || echo "deploy: the local install configuration $GS_LOCAL_DIR"
LOCAL_ARGS=()
[ -z "$GS_LOCAL_DIR" ] || LOCAL_ARGS=( --local-dir "$GS_LOCAL_DIR" )
! $ALLOW_LOCAL_IN_GIT || LOCAL_ARGS+=( --allow-local-in-git )
gs_require SERVICE_NAME NAMESPACE NODE_SELECTOR_KEY NODE_SELECTOR_VALUE IMAGE_NAME IMAGE_TAG
DISCOVERY_MODE="${DISCOVERY_MODE:-lan}"
TIMEOUT="${TIMEOUT:-$(( ${STARTUP_FAILURE_THRESHOLD:-30} * 10 + 120 ))}"

read -r -a K <<< "${KUBECTL:-kubectl}"
[ ${#K[@]} -gt 0 ] || gs_fail "KUBECTL is empty"
read -r -a KZ <<< "${KUSTOMIZE:-kubectl kustomize}"
[ ${#KZ[@]} -gt 0 ] || gs_fail "KUSTOMIZE is empty"
say() { echo "deploy: $*"; }

# the names this script passes to KUBECTL: plain names only (render_manifests.sh checks them again)
for key in SERVICE_NAME NAMESPACE; do
    [[ "${!key}" =~ ^[a-z0-9]([-a-z0-9]{0,61}[a-z0-9])?$ ]] || gs_fail "$key '${!key}' is not a DNS label"
done
[[ "$NODE_SELECTOR_KEY=$NODE_SELECTOR_VALUE" =~ ^[A-Za-z0-9./_-]+=[A-Za-z0-9._-]*$ ]] || gs_fail "NODE_SELECTOR_KEY=NODE_SELECTOR_VALUE '$NODE_SELECTOR_KEY=$NODE_SELECTOR_VALUE' is not a plain label"

# 1. the StorageClass
say "1. the StorageClass for STORAGE_CLASS=${STORAGE_CLASS:-auto} (${KUBECTL:-kubectl} get storageclass -o name)"
CLASS="$("$SCRIPT_DIR/resolve_storage_class.sh" "${STORAGE_CLASS:-auto}")" || gs_fail "the StorageClass is not resolved (above); nothing was changed"
EXISTING_CLASS="$("${K[@]}" -n "$NAMESPACE" get pvc "$SERVICE_NAME-data" --ignore-not-found -o 'jsonpath={.spec.storageClassName}')" || gs_fail "cannot read the claim $SERVICE_NAME-data of $NAMESPACE"
if [ -n "$EXISTING_CLASS" ] && [ "$EXISTING_CLASS" != "$CLASS" ]; then
    gs_fail "the claim $NAMESPACE/$SERVICE_NAME-data exists with the StorageClass $EXISTING_CLASS, and STORAGE_CLASS=${STORAGE_CLASS:-auto} resolves to $CLASS now; a claim's class cannot change. Set STORAGE_CLASS=$EXISTING_CLASS to keep the data where it is, or move the data and delete the claim first"
fi
say "   the claim uses $CLASS${EXISTING_CLASS:+ (as it does already)}"

# 2. render and build
if [ -z "$OUT" ]; then
    base="${DEPLOY_WORK_DIR:-$HOME/.cache/gucef-deploy}/$SERVICE_NAME"
    mkdir -p "$base"
    OUT="$(mktemp -d "$base/$(date -u +%Y%m%dT%H%M%SZ).XXXX")"
fi
say "2. rendering into $OUT"
"$SCRIPT_DIR/render_manifests.sh" --storage-class "$CLASS" "${LOCAL_ARGS[@]}" "$ENV_FILE" "$OUT/rendered" > /dev/null
"${KZ[@]}" "$OUT/rendered" > "$OUT/built.yaml" || gs_fail "'${KUSTOMIZE:-kubectl kustomize} $OUT/rendered' failed"
grep -q '^kind: Deployment$' "$OUT/built.yaml" || gs_fail "the built manifests in $OUT/built.yaml hold no Deployment"
if grep -q '^kind: Secret$' "$OUT/built.yaml"; then
    gs_fail "the built manifests hold a Secret; Secrets are created in the cluster, never applied from here"
fi

# 3. the node
say "3. the node with $NODE_SELECTOR_KEY=$NODE_SELECTOR_VALUE"
NODES="$("${K[@]}" get node -l "$NODE_SELECTOR_KEY=$NODE_SELECTOR_VALUE" -o name)" || gs_fail "cannot list the nodes"
LABEL_NODE="${DISCOVERY_NODE_NAME:-<node>}"
[ -n "$NODES" ] || gs_fail "no node carries the label $NODE_SELECTOR_KEY=$NODE_SELECTOR_VALUE; label the node that holds the image: kubectl label node $LABEL_NODE $NODE_SELECTOR_KEY=$NODE_SELECTOR_VALUE"
if [ "$DISCOVERY_MODE" = "lan" ]; then
    grep -qx "node/$DISCOVERY_NODE_NAME" <<< "$NODES" || gs_fail "the node $DISCOVERY_NODE_NAME (DISCOVERY_NODE_NAME) does not carry $NODE_SELECTOR_KEY=$NODE_SELECTOR_VALUE (labelled: $(tr '\n' ' ' <<< "$NODES")); kubectl label node $DISCOVERY_NODE_NAME $NODE_SELECTOR_KEY=$NODE_SELECTOR_VALUE"
fi
if [ "$DISCOVERY_MODE" = "lan" ]; then
    IMAGE_NODE="$DISCOVERY_NODE_NAME"
else
    IMAGE_NODE="$(head -n 1 <<< "$NODES")"
    IMAGE_NODE="${IMAGE_NODE#node/}"
fi

# 4. the namespace
NAMESPACE_EXISTS="$("${K[@]}" get namespace "$NAMESPACE" --ignore-not-found -o name)" || gs_fail "cannot read the namespace $NAMESPACE"
if $DRY_RUN; then
    if [ -n "$NAMESPACE_EXISTS" ]; then
        say "4. (dry run) the namespace $NAMESPACE exists"
    else
        say "4. (dry run) the namespace $NAMESPACE would be created"
    fi
else
    say "4. the namespace $NAMESPACE"
    "${K[@]}" apply -f - < "$OUT/rendered/namespace.yaml" || gs_fail "the namespace was not applied"
    NAMESPACE_EXISTS="namespace/$NAMESPACE"
fi

# 5. the Secret of components/secret-file: present with its key, else stop with how to make it
if [ -n "${SECRET_FILE_SECRET_NAME:-}" ]; then
    say "5. the Secret $SECRET_FILE_SECRET_NAME with the key $SECRET_FILE_KEY"
    state=""
    if [ -n "$NAMESPACE_EXISTS" ]; then
        # only "present" or nothing comes back: the value never leaves kubectl (no spaces or quotes: a remote shell
        # passes the template unchanged)
        state="$("${K[@]}" -n "$NAMESPACE" get secret "$SECRET_FILE_SECRET_NAME" --ignore-not-found -o "go-template={{with.data.$SECRET_FILE_KEY}}present{{end}}")" || gs_fail "cannot read whether the Secret $SECRET_FILE_SECRET_NAME exists"
    fi
    if [ "$state" != "present" ]; then
        message="the Secret $NAMESPACE/$SECRET_FILE_SECRET_NAME with the key $SECRET_FILE_KEY is missing. Create it from the value, never from a file of a repository, for example:
    ( set -a; . <env file>; printf '%s' \"\$<VARIABLE>\" ) | ${KUBECTL:-kubectl} -n $NAMESPACE create secret generic $SECRET_FILE_SECRET_NAME --from-file=$SECRET_FILE_KEY=/dev/stdin
  (or --from-file=$SECRET_FILE_KEY=<file holding only the value>), then run deploy.sh again"
        if $DRY_RUN; then
            say "   (dry run) $message"
        else
            gs_fail "$message"
        fi
    fi
else
    say "5. no secret file (SECRET_FILE_SECRET_NAME is not set)"
fi

# 6. the image on the node (the node lists a limited number of images: a warning, not a refusal); an image of a registry
#    is pulled by the node itself
if [ -n "${IMAGE_REGISTRY:-}" ]; then
    image_name="$(gs_registry_image_name)"
    say "6. the image $(gs_image_ref "$IMAGE_REGISTRY" "$image_name" "$IMAGE_TAG" || echo "$IMAGE_REGISTRY/$image_name ($IMAGE_TAG)") is pulled from the registry $IMAGE_REGISTRY (imagePullPolicy IfNotPresent; the node needs the registry in /etc/rancher/k3s/registries.yaml, projects/k3s/forgejo/registry/setup-node.sh)"
else
    IMAGES="$("${K[@]}" get node "$IMAGE_NODE" -o 'jsonpath={.status.images}' 2> /dev/null || true)"
    if grep -q -F "docker.io/$IMAGE_NAME:$IMAGE_TAG" <<< "$IMAGES"; then
        say "6. the image docker.io/$IMAGE_NAME:$IMAGE_TAG is on $IMAGE_NODE"
    else
        say "6. WARNING: docker.io/$IMAGE_NAME:$IMAGE_TAG is not in the image list of $IMAGE_NODE (imagePullPolicy Never): import it there with sudo k3s ctr images import <archive> (build_image.sh --save), or the pod stays in ErrImageNeverPull"
    fi
fi

# 7. apply and wait
if $DRY_RUN; then
    say "7. (dry run) would apply $OUT/built.yaml and wait up to ${TIMEOUT}s for deployment/$SERVICE_NAME"
    exit 0
fi
say "7. applying $OUT/built.yaml"
"${K[@]}" apply -f - < "$OUT/built.yaml" || gs_fail "the manifests were not applied completely"
say "   waiting up to ${TIMEOUT}s for deployment/$SERVICE_NAME"
"${K[@]}" -n "$NAMESPACE" rollout status "deployment/$SERVICE_NAME" "--timeout=${TIMEOUT}s" || gs_fail "the rollout did not complete; see: kubectl -n $NAMESPACE describe pod -l app.kubernetes.io/name=$SERVICE_NAME, kubectl -n $NAMESPACE logs deployment/$SERVICE_NAME"
say "done: $NAMESPACE/$SERVICE_NAME is rolled out (manifests in $OUT)"
