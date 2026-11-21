#!/bin/bash
# Copyright (C) 1998 - 2026.  Dinand Vanvelzen. Licensed under the Apache License, Version 2.0.
#
# Renders the k3s manifests of a GUCEF service from the templates of ../k3s and a service file into a directory that
# kubectl applies with -k. A placeholder @KEY@ takes the value of KEY of the service file; a line holding only
# @FILE:KEY@ takes the lines of the file KEY names (the site settings of the ConfigMap). An unset or empty value refuses
# the rendering (nothing is half written), except for keys that may be empty (EXTRA_CHECKOUTS, RUNTIME_EXTRA_PACKAGES,
# DOCKERFILE, CONTAINER_ARGS, PLUGIN_LINKS).
#
# usage: render_manifests.sh [--storage-classes-file FILE | --query-cluster | --storage-class NAME]
#                            [--local-dir DIR [--allow-local-in-git]] <service.env> <output directory>
#   output directory          made (must not exist, or be empty); then: kubectl apply -k <output directory>
#   --storage-classes-file F  the StorageClasses of the target cluster ("kubectl get storageclass -o name", "-" for
#                             standard input): STORAGE_CLASS is resolved against them (resolve_storage_class.sh)
#   --query-cluster           the same with the list asked from the cluster: $KUBECTL get storageclass -o name
#                             (KUBECTL, default kubectl, is a command prefix such as "ssh k3smaster kubectl")
#   --storage-class NAME      the class, already resolved (deploy.sh); STORAGE_CLASS of the service file is not used
#   --local-dir DIR           the local install configuration of the site (README.md, "Local install configuration"):
#                             service.env.local overrides keys of the service file, site.json replaces the site
#                             settings, patches/*.yaml are applied after everything else, resources/*.yaml are added.
#                             Default: the directory of the service file when it holds a service.env.local. Refused
#                             inside a git work tree unless --allow-local-in-git is given
# Without one of these, a STORAGE_CLASS that is a class name is used as it is (not checked against a cluster), and
# auto (the default) or a friendly name (raid, mirror, jbod, local-path) refuses the rendering: it needs the classes of
# the cluster. There is no fallback to any class.
#
# Optional components, each rendered and enabled only when the service file sets its key:
#   lan-discovery    DISCOVERY_MODE lan (the default), see below
#   secret-file      SECRET_FILE_SECRET_NAME: one key of a Secret as a read-only file for the program (a database
#                    password); enabled in the kustomization, the Secret is created in the cluster (README.md)
#   host-share       HOST_SHARE_HOST_PATH: a node-mounted network share at HOST_SHARE_MOUNT_PATH (default /share), with
#                    an init container that refuses a path that is not on a network file system (HOST_SHARE_FSTYPES)
#   credentials-key  CREDENTIALS_SECRET_NAME: the credential store key Secret; rendered with secret.example.yaml and
#                    left as a commented line of the kustomization, to enable by hand once the Secret exists
#
# DISCOVERY_MODE (default lan) chooses how the service is reached:
#   lan   (the default) also answers discovery on the local network: the component components/lan-discovery runs the
#         pod on the host network of the node DISCOVERY_NODE_NAME, which must then be set (the rendering refuses
#         without it, there is no fallback), with the discovery port DISCOVERY_PORT (default 47800, UDP), a ClusterIP
#         Service and the namespace at the "privileged" Pod Security level (README.md, "LAN discovery")
#   off   the restricted Pod Security standard and the LoadBalancer Service (servicelb): clients use the configured
#         endpoint (a DNS name or address and the session port) only
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=lib.sh
source "$SCRIPT_DIR/lib.sh"
TEMPLATES="$(cd "$SCRIPT_DIR/../k3s" && pwd)"

usage() {
    echo "usage: render_manifests.sh [--storage-classes-file FILE | --query-cluster | --storage-class NAME] [--local-dir DIR [--allow-local-in-git]] <service.env> <output directory>" >&2
    exit 2
}

ENV_FILE=""
OUT=""
CLASSES_FILE=""
QUERY_CLUSTER=false
GIVEN_CLASS=""
LOCAL_DIR=""
ALLOW_LOCAL_IN_GIT=false
while [ $# -gt 0 ]; do
    case "$1" in
        --storage-classes-file) [ -n "${2:-}" ] || usage; CLASSES_FILE="$2"; shift ;;
        --query-cluster) QUERY_CLUSTER=true ;;
        --local-dir) [ -n "${2:-}" ] || usage; LOCAL_DIR="$2"; shift ;;
        --allow-local-in-git) ALLOW_LOCAL_IN_GIT=true ;;
        --storage-class) [ -n "${2:-}" ] || usage; GIVEN_CLASS="$2"; shift ;;
        -h|--help) usage ;;
        -?*) echo "render_manifests: unknown option $1" >&2; usage ;;
        *)
            if [ -z "$ENV_FILE" ]; then ENV_FILE="$1"
            elif [ -z "$OUT" ]; then OUT="$1"
            else usage
            fi ;;
    esac
    shift
done
[ -n "$ENV_FILE" ] && [ -n "$OUT" ] || usage
sources=0
[ -z "$CLASSES_FILE" ] || sources=$((sources + 1))
! $QUERY_CLUSTER || sources=$((sources + 1))
[ -z "$GIVEN_CLASS" ] || sources=$((sources + 1))
[ "$sources" -le 1 ] || { echo "render_manifests: give at most one of --storage-classes-file, --query-cluster, --storage-class" >&2; exit 2; }
if [ -e "$OUT" ] && [ -n "$(ls -A "$OUT" 2> /dev/null)" ]; then
    echo "render_manifests: $OUT exists and is not empty" >&2
    exit 2
fi
gs_load_service "$ENV_FILE" "$LOCAL_DIR" "$ALLOW_LOCAL_IN_GIT"
export GS_EMPTY_ALLOWED="EXTRA_CHECKOUTS RUNTIME_EXTRA_PACKAGES DOCKERFILE CONTAINER_ARGS PLUGIN_LINKS HOST_SHARE_SUBPATH"
# the default is lan, also for a service file from before the discovery mode
export DISCOVERY_MODE="${DISCOVERY_MODE:-lan}"
export DISCOVERY_PORT="${DISCOVERY_PORT:-47800}"
export DISCOVERY_NODE_NAME="${DISCOVERY_NODE_NAME:-}"
case "$DISCOVERY_MODE" in
    off)
        export POD_SECURITY_ENFORCE=restricted
        ;;
    lan)
        export POD_SECURITY_ENFORCE=privileged
        [ -n "$DISCOVERY_NODE_NAME" ] || gs_fail "DISCOVERY_MODE=lan (the default) needs DISCOVERY_NODE_NAME: the name of the node whose addresses answer on the LAN (kubectl get nodes), for example the node of worker.lan; or set DISCOVERY_MODE=off to reach the service at its configured endpoint only"
        [[ "$DISCOVERY_NODE_NAME" =~ ^[a-z0-9]([-a-z0-9.]{0,251}[a-z0-9])?$ ]] || gs_fail "DISCOVERY_NODE_NAME '$DISCOVERY_NODE_NAME' is not a node name (kubectl get nodes)"
        ;;
    *)
        gs_fail "DISCOVERY_MODE '$DISCOVERY_MODE' is neither off nor lan"
        ;;
esac
[[ "$DISCOVERY_PORT" =~ ^[0-9]+$ ]] && [ "$DISCOVERY_PORT" -ge 1 ] && [ "$DISCOVERY_PORT" -le 65535 ] || gs_fail "DISCOVERY_PORT '$DISCOVERY_PORT' is not a port"
[[ "${SERVICE_NAME:-}" =~ ^[a-z0-9]([-a-z0-9]{0,61}[a-z0-9])?$ ]] || gs_fail "SERVICE_NAME '${SERVICE_NAME:-}' is not a DNS label"
[[ "${NAMESPACE:-}" =~ ^[a-z0-9]([-a-z0-9]{0,61}[a-z0-9])?$ ]] || gs_fail "NAMESPACE '${NAMESPACE:-}' is not a DNS label"
for port in SESSION_PORT PAIRING_PORT HEALTH_PORT ADMIN_PORT; do
    value="${!port:-}"
    [[ "$value" =~ ^[0-9]+$ ]] && [ "$value" -ge 1 ] && [ "$value" -le 65535 ] || gs_fail "$port '$value' is not a port"
done
[ "${TERMINATION_GRACE_SECONDS:-0}" -gt "${DRAIN_SECONDS:-}" ] 2> /dev/null || gs_fail "TERMINATION_GRACE_SECONDS must be above DRAIN_SECONDS (the drain has to end before the kill)"

# the optional components
COMPONENTS_ON=""
[ "$DISCOVERY_MODE" = "lan" ] && COMPONENTS_ON="lan-discovery"
SECRET_FILE_SECRET_NAME="${SECRET_FILE_SECRET_NAME:-}"
if [ -n "$SECRET_FILE_SECRET_NAME" ]; then
    export SECRET_FILE_MODE="${SECRET_FILE_MODE:-0400}"
    [[ "$SECRET_FILE_SECRET_NAME" =~ ^[a-z0-9]([-a-z0-9.]{0,251}[a-z0-9])?$ ]] || gs_fail "SECRET_FILE_SECRET_NAME '$SECRET_FILE_SECRET_NAME' is not a Secret name"
    # a plain identifier: deploy.sh checks for the key with a template that passes a remote shell (ssh) unchanged
    [[ "${SECRET_FILE_KEY:-}" =~ ^[A-Za-z_][A-Za-z0-9_]{0,62}$ ]] || gs_fail "SECRET_FILE_KEY '${SECRET_FILE_KEY:-}' is not a key name (letters, digits, '_')"
    mount="${SECRET_FILE_MOUNT_DIR:-}"
    [[ "$mount" =~ ^(/[A-Za-z0-9._-]+)+$ ]] && [[ "$mount" != *"/.."* ]] && [[ "$mount" != *"/./"* ]] || gs_fail "SECRET_FILE_MOUNT_DIR '$mount' is not an absolute directory path"
    case "$mount/" in
        /data/*|/tmp/*|/opt/*|/proc/*|/sys/*|/dev/*|/etc/*|/usr/*|/bin/*|/lib/*) gs_fail "SECRET_FILE_MOUNT_DIR '$mount' is below a directory of the image or of the pod's volumes; use a directory of its own, for example /secrets/$SERVICE_NAME" ;;
    esac
    [[ "$SECRET_FILE_MODE" =~ ^0?[0-7]{3}$ ]] || gs_fail "SECRET_FILE_MODE '$SECRET_FILE_MODE' is not an octal file mode such as 0400"
    [ "${#SECRET_FILE_MODE}" -eq 4 ] || SECRET_FILE_MODE="0$SECRET_FILE_MODE"
    (( (8#$SECRET_FILE_MODE & 8#022) == 0 )) || gs_fail "SECRET_FILE_MODE '$SECRET_FILE_MODE' makes the secret file writable"
    COMPONENTS_ON="${COMPONENTS_ON:+$COMPONENTS_ON }secret-file"
fi
HOST_SHARE_HOST_PATH="${HOST_SHARE_HOST_PATH:-}"
if [ -n "$HOST_SHARE_HOST_PATH" ]; then
    export HOST_SHARE_MOUNT_PATH="${HOST_SHARE_MOUNT_PATH:-/share}"
    export HOST_SHARE_FSTYPES="${HOST_SHARE_FSTYPES:-cifs smb3 nfs nfs4}"
    HOST_SHARE_SUBPATH="${HOST_SHARE_SUBPATH:-}"
    # the node path: any characters but control characters (spaces and '-== ==-' are fine), no . or .. component
    for value in "$HOST_SHARE_HOST_PATH" "$HOST_SHARE_SUBPATH"; do
        [[ "$value" != *[[:cntrl:]]* ]] || gs_fail "HOST_SHARE_HOST_PATH and HOST_SHARE_SUBPATH may hold no control characters"
        [[ "/$value/" != *"/../"* ]] && [[ "/$value/" != *"/./"* ]] || gs_fail "HOST_SHARE_HOST_PATH and HOST_SHARE_SUBPATH may hold no . or .. component: '$value'"
    done
    [ "${HOST_SHARE_HOST_PATH:0:1}" = "/" ] || gs_fail "HOST_SHARE_HOST_PATH '$HOST_SHARE_HOST_PATH' is not an absolute path on the node"
    [ -z "$HOST_SHARE_SUBPATH" ] || [ "${HOST_SHARE_SUBPATH:0:1}" != "/" ] || gs_fail "HOST_SHARE_SUBPATH '$HOST_SHARE_SUBPATH' is relative to HOST_SHARE_HOST_PATH"
    node_path="${HOST_SHARE_HOST_PATH%/}"
    [ -z "$HOST_SHARE_SUBPATH" ] || node_path="$node_path/${HOST_SHARE_SUBPATH%/}"
    case "$node_path/" in
        /|//|/proc/*|/sys/*|/dev/*|/etc/*|/boot/*|/run/*|/var/lib/kubelet/*|/var/lib/rancher/*|/usr/*|/bin/*|/sbin/*|/lib/*)
            gs_fail "HOST_SHARE_HOST_PATH '$node_path' is a directory of the node's system; give the directory of the share" ;;
    esac
    mount="$HOST_SHARE_MOUNT_PATH"
    [[ "$mount" =~ ^(/[A-Za-z0-9._-]+)+$ ]] && [[ "/$mount/" != *"/../"* ]] && [[ "/$mount/" != *"/./"* ]] || gs_fail "HOST_SHARE_MOUNT_PATH '$mount' is not an absolute directory path of letters, digits, '.', '_' and '-'"
    case "$mount/" in
        /data/*|/tmp/*|/opt/*|/proc/*|/sys/*|/dev/*|/etc/*|/usr/*|/bin/*|/lib/*) gs_fail "HOST_SHARE_MOUNT_PATH '$mount' is below a directory of the image or of the pod's volumes; use a directory of its own, such as /share" ;;
    esac
    if [ -n "${SECRET_FILE_MOUNT_DIR:-}" ]; then
        case "$mount/" in "${SECRET_FILE_MOUNT_DIR%/}/"*) gs_fail "HOST_SHARE_MOUNT_PATH '$mount' is below SECRET_FILE_MOUNT_DIR" ;; esac
        case "${SECRET_FILE_MOUNT_DIR%/}/" in "$mount/"*) gs_fail "SECRET_FILE_MOUNT_DIR is below HOST_SHARE_MOUNT_PATH '$mount'" ;; esac
    fi
    [[ "$HOST_SHARE_FSTYPES" =~ ^[a-z0-9.]+( [a-z0-9.]+)*$ ]] || gs_fail "HOST_SHARE_FSTYPES '$HOST_SHARE_FSTYPES' is not a space separated list of file system types (cifs smb3 nfs nfs4)"
    export HOST_SHARE_NODE_PATH_YAML="$(gs_yaml_quote "$node_path")"
    COMPONENTS_ON="${COMPONENTS_ON:+$COMPONENTS_ON }host-share"
    # a hostPath volume is admitted only at the "privileged" level (baseline and restricted refuse it)
    if [ "$POD_SECURITY_ENFORCE" != "privileged" ]; then
        echo "render_manifests: the namespace enforces the Pod Security level privileged (a hostPath volume, component host-share); restricted stays the warn and audit level" >&2
        export POD_SECURITY_ENFORCE=privileged
    fi
fi
CREDENTIALS_ON=false
[ -z "${CREDENTIALS_SECRET_NAME:-}" ] || CREDENTIALS_ON=true

# the StorageClass: resolved against the cluster's classes, given, or a class name used as it is
export STORAGE_CLASS_REQUEST="${STORAGE_CLASS:-auto}"
gs_storage_class_request_valid "$STORAGE_CLASS_REQUEST" || gs_fail "STORAGE_CLASS '$STORAGE_CLASS_REQUEST' is neither auto, a friendly name ($GS_STORAGE_FRIENDLY_NAMES) nor a StorageClass name"
if [ -n "$GIVEN_CLASS" ]; then
    [[ "$GIVEN_CLASS" =~ ^[a-z0-9]([-a-z0-9.]{0,251}[a-z0-9])?$ ]] || gs_fail "--storage-class '$GIVEN_CLASS' is not a StorageClass name"
    STORAGE_CLASS="$GIVEN_CLASS"
elif [ -n "$CLASSES_FILE" ]; then
    STORAGE_CLASS="$("$SCRIPT_DIR/resolve_storage_class.sh" --classes-file "$CLASSES_FILE" "$STORAGE_CLASS_REQUEST")" || gs_fail "STORAGE_CLASS $STORAGE_CLASS_REQUEST is not resolved (above); nothing is rendered"
elif $QUERY_CLUSTER; then
    STORAGE_CLASS="$("$SCRIPT_DIR/resolve_storage_class.sh" "$STORAGE_CLASS_REQUEST")" || gs_fail "STORAGE_CLASS $STORAGE_CLASS_REQUEST is not resolved (above); nothing is rendered"
elif [ "$STORAGE_CLASS_REQUEST" = "auto" ] || gs_storage_class_is_friendly "$STORAGE_CLASS_REQUEST"; then
    gs_fail "STORAGE_CLASS=$STORAGE_CLASS_REQUEST is resolved against the StorageClasses of the target cluster: give --storage-classes-file FILE (kubectl get storageclass -o name), --query-cluster (with KUBECTL) or --storage-class NAME"
else
    STORAGE_CLASS="$STORAGE_CLASS_REQUEST"
    echo "render_manifests: STORAGE_CLASS $STORAGE_CLASS is used as it is, not checked against a cluster" >&2
fi
export STORAGE_CLASS

# the container arguments as a YAML flow sequence
CONTAINER_ARGS_YAML="["
first=true
for argument in ${CONTAINER_ARGS:-}; do
    $first || CONTAINER_ARGS_YAML="$CONTAINER_ARGS_YAML,"
    CONTAINER_ARGS_YAML="$CONTAINER_ARGS_YAML \"${argument//\"/\\\"}\""
    first=false
done
export CONTAINER_ARGS_YAML="$CONTAINER_ARGS_YAML ]"

# whether a template belongs to a component (or example) that is off
template_off() {
    case "$1" in
        components/lan-discovery/*) [[ " $COMPONENTS_ON " != *" lan-discovery "* ]] ;;
        components/secret-file/*) [[ " $COMPONENTS_ON " != *" secret-file "* ]] ;;
        components/host-share/*) [[ " $COMPONENTS_ON " != *" host-share "* ]] ;;
        components/credentials-key/*|secret.example.yaml) ! $CREDENTIALS_ON ;;
        *) return 1 ;;
    esac
}

mkdir -p "$OUT"
rendered=0
while IFS= read -r -d '' template; do
    relative="${template#"$TEMPLATES"/}"
    template_off "$relative" && continue
    mkdir -p "$OUT/$(dirname "$relative")"
    gs_render "$template" "$OUT/$relative"
    rendered=$((rendered + 1))
done < <(find "$TEMPLATES" -type f -name '*.yaml' -print0 | sort -z)
if [ -n "$COMPONENTS_ON" ]; then
    # the enabled components in the kustomization; the credentials key component stays a commented line below them
    replacement="components:"
    for component in $COMPONENTS_ON; do
        replacement="$replacement\n  - components/$component"
    done
    sed -i -e "s|^# components:\$|$replacement|" "$OUT/kustomization.yaml"
    for component in $COMPONENTS_ON; do
        grep -q "^  - components/$component\$" "$OUT/kustomization.yaml" || gs_fail "the kustomization template has no '# components:' line for the component $component"
    done
fi
$CREDENTIALS_ON || sed -i -e '/^#   - components\/credentials-key$/d' "$OUT/kustomization.yaml"
# the local patches and resources: rendered like the templates, kept below local/ of the output (kustomize reads only
# files below the kustomization), patches after everything else
LOCAL_RESOURCES=""
LOCAL_PATCHES=""
if [ -n "$GS_LOCAL_DIR" ]; then
    for kind in resources patches; do
        [ -d "$GS_LOCAL_DIR/$kind" ] || continue
        while IFS= read -r -d '' file; do
            name="$(basename "$file")"
            [[ "$name" =~ ^[A-Za-z0-9._-]+\.yaml$ ]] || gs_fail "the local file $kind/$name: a name of letters, digits, '.', '_' and '-' ending in .yaml"
            mkdir -p "$OUT/local/$kind"
            gs_render "$file" "$OUT/local/$kind/$name"
            rendered=$((rendered + 1))
            if [ "$kind" = resources ]; then LOCAL_RESOURCES="$LOCAL_RESOURCES $name"; else LOCAL_PATCHES="$LOCAL_PATCHES $name"; fi
        done < <(find "$GS_LOCAL_DIR/$kind" -maxdepth 1 -type f -name '*.yaml' -print0 | sort -z)
    done
    for name in $LOCAL_RESOURCES; do
        sed -i -e "/^  - service.yaml\$/a\\  - local/resources/$name" "$OUT/kustomization.yaml"
    done
    grep -q '^  - service.yaml$' "$OUT/kustomization.yaml" || gs_fail "the kustomization template has no '  - service.yaml' line for the local resources"
    if [ -n "$LOCAL_PATCHES" ]; then
        {
            echo "# the local install configuration of the site, applied after the base and the components"
            echo "patches:"
            for name in $LOCAL_PATCHES; do
                echo "  - path: local/patches/$name"
            done
        } >> "$OUT/kustomization.yaml"
    fi
fi
if grep -R -n -E '@[A-Z][A-Z0-9_:]*@' "$OUT" > /dev/null; then
    gs_fail "placeholders are left in $OUT: $(grep -R -l -E '@[A-Z][A-Z0-9_:]*@' "$OUT" | tr '\n' ' ')"
fi
echo "render_manifests: $rendered files in $OUT (discovery $DISCOVERY_MODE, components: ${COMPONENTS_ON:-none}, storage class $STORAGE_CLASS from $STORAGE_CLASS_REQUEST${GS_LOCAL_DIR:+, local configuration $GS_LOCAL_DIR}); apply with: kubectl apply -k $OUT"
