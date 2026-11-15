#!/bin/bash
# Copyright (C) 1998 - 2026.  Dinand Vanvelzen. Licensed under the Apache License, Version 2.0.
#
# Renders the k3s manifests of a GUCEF service from the templates of ../k3s and a service file into a directory that
# kubectl applies with -k. A placeholder @KEY@ takes the value of KEY of the service file; a line holding only
# @FILE:KEY@ takes the lines of the file KEY names (the site settings of the ConfigMap). An unset or empty value refuses
# the rendering (nothing is half written), except for keys that may be empty (EXTRA_CHECKOUTS, RUNTIME_EXTRA_PACKAGES,
# DOCKERFILE, CONTAINER_ARGS).
#
# usage: render_manifests.sh <service.env> <output directory>
#   output directory   made (must not exist, or be empty); then: kubectl apply -k <output directory>
#
# The credentials key Secret is not rendered with data: components/credentials-key mounts a Secret that is created in
# the cluster (README.md); it is enabled by uncommenting "components:" in the rendered kustomization.yaml.
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

ENV_FILE="${1:-}"
OUT="${2:-}"
[ -n "$ENV_FILE" ] && [ -n "$OUT" ] || { echo "usage: render_manifests.sh <service.env> <output directory>" >&2; exit 2; }
if [ -e "$OUT" ] && [ -n "$(ls -A "$OUT" 2> /dev/null)" ]; then
    echo "render_manifests: $OUT exists and is not empty" >&2
    exit 2
fi
gs_load_env "$ENV_FILE"
export GS_EMPTY_ALLOWED="EXTRA_CHECKOUTS RUNTIME_EXTRA_PACKAGES DOCKERFILE CONTAINER_ARGS"
# the default is lan, also for a service file from before the discovery mode
export DISCOVERY_MODE="${DISCOVERY_MODE:-lan}"
export DISCOVERY_PORT="${DISCOVERY_PORT:-47800}"
export DISCOVERY_NODE_NAME="${DISCOVERY_NODE_NAME:-}"
case "$DISCOVERY_MODE" in
    off)
        export POD_SECURITY_ENFORCE=restricted
        # the lan component is rendered for completeness and then left out: nothing of it may be required
        export GS_EMPTY_ALLOWED="$GS_EMPTY_ALLOWED DISCOVERY_NODE_NAME"
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
[[ "$SERVICE_NAME" =~ ^[a-z0-9]([-a-z0-9]{0,61}[a-z0-9])?$ ]] || gs_fail "SERVICE_NAME '$SERVICE_NAME' is not a DNS label"
[[ "$NAMESPACE" =~ ^[a-z0-9]([-a-z0-9]{0,61}[a-z0-9])?$ ]] || gs_fail "NAMESPACE '$NAMESPACE' is not a DNS label"
for port in SESSION_PORT PAIRING_PORT HEALTH_PORT ADMIN_PORT; do
    value="${!port:-}"
    [[ "$value" =~ ^[0-9]+$ ]] && [ "$value" -ge 1 ] && [ "$value" -le 65535 ] || gs_fail "$port '$value' is not a port"
done
[ "$TERMINATION_GRACE_SECONDS" -gt "$DRAIN_SECONDS" ] 2> /dev/null || gs_fail "TERMINATION_GRACE_SECONDS must be above DRAIN_SECONDS (the drain has to end before the kill)"
# the container arguments as a YAML flow sequence
CONTAINER_ARGS_YAML="["
first=true
for argument in ${CONTAINER_ARGS:-}; do
    $first || CONTAINER_ARGS_YAML="$CONTAINER_ARGS_YAML,"
    CONTAINER_ARGS_YAML="$CONTAINER_ARGS_YAML \"${argument//\"/\\\"}\""
    first=false
done
export CONTAINER_ARGS_YAML="$CONTAINER_ARGS_YAML ]"

mkdir -p "$OUT/components/credentials-key" "$OUT/components/lan-discovery"
rendered=0
while IFS= read -r -d '' template; do
    relative="${template#"$TEMPLATES"/}"
    gs_render "$template" "$OUT/$relative"
    rendered=$((rendered + 1))
done < <(find "$TEMPLATES" -type f -name '*.yaml' -print0)
if [ "$DISCOVERY_MODE" = "lan" ]; then
    # the component in the kustomization; the credentials key component stays a commented line below it
    sed -i -e 's|^# components:$|components:\n  - components/lan-discovery|' "$OUT/kustomization.yaml"
    grep -q '^  - components/lan-discovery$' "$OUT/kustomization.yaml" || gs_fail "the kustomization template has no '# components:' line for the lan-discovery component"
else
    rm -rf "$OUT/components/lan-discovery"
    rendered=$((rendered - $(find "$TEMPLATES/components/lan-discovery" -type f -name '*.yaml' | wc -l)))
fi
if grep -R -n -E '@[A-Z][A-Z0-9_:]*@' "$OUT" > /dev/null; then
    gs_fail "placeholders are left in $OUT: $(grep -R -l -E '@[A-Z][A-Z0-9_:]*@' "$OUT" | tr '\n' ' ')"
fi
echo "render_manifests: $rendered files in $OUT (discovery $DISCOVERY_MODE); apply with: kubectl apply -k $OUT"
