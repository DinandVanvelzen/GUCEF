#!/bin/bash
# Copyright (C) 1998 - 2026.  Dinand Vanvelzen. Licensed under the Apache License, Version 2.0.
#
# Tests of the gucef_service scripts and templates on constructed trees: no docker, no cluster, no network, nothing
# downloaded. Needs bash, git, rsync, a C compiler (cc), ldd, awk and the coreutils.
#
# usage: tests/run_tests.sh
#
#   syntax     bash -n of every script; no tab in any file of the kit
#   env        the service file reader: quotes, comments, refused lines
#   discovery  DISCOVERY_MODE lan (the default, also for a service file without the key) and off: the component, the
#              host network on the named node, the ClusterIP Service, the Pod Security levels, the refusals, and the
#              kustomize build of both when kubectl is there (offline: kubectl kustomize reads files only)
#   render     the manifests of the example service file: every template rendered, no placeholder left, the site
#              settings inserted with their indentation, refusals (an unset or empty value, a bad name, a grace period
#              not above the drain, a non-empty output directory) leave nothing behind
#   staging    a program, a library and two plugins (one recognised by its name pattern, one only because the bootstrap
#              file names it) built with the local compiler in the layout of a build directory; stage_install.sh with a
#              stand-in patchelf (which records what it was asked); check_libs.sh passes, the staged program runs from
#              the staged tree alone, and check_libs.sh fails for a missing required plugin, a missing program, a
#              library that resolves outside the tree and a dangling plugin link; the plugin links of --plugin-links lib
#   context    make_context.sh on a constructed git checkout: tracked, untracked, ignored and deleted files, the state
#   image      build_image.sh --dry-run: the rendered Dockerfile, refusals before docker is ever called
#   local      run_local.sh --dry-run: the restrictions of the pod on the docker command line
#   storage    resolve_storage_class.sh: each friendly name and its candidates (the Longhorn class wins over the plain
#              one), auto in its order on several class lists (the owner's cluster, only local-path, only Longhorn
#              classes, both raid forms, none matching: refused with the classes found), an explicit name passed through
#              or refused, the list formats, KUBECTL as a command prefix with a stand-in kubectl and a stand-in ssh that
#              re-parses the arguments in a shell, shell characters in KUBECTL never evaluated; render_manifests.sh with
#              a class list, --query-cluster, --storage-class, and its refusals
#   components secret-file (rendered, enabled, mounted read-only with its mode, refusals) and the optional
#              credentials-key component
#   deploy     deploy.sh against the stand-in kubectl with the asset store's service folder: refusals (node label,
#              Secret, a claim of another class, no class), the order (namespace before the Secret check), the applied
#              manifests, the rollout wait, a second run, --dry-run, and the whole run through the stand-in ssh
#   local configuration  --local-dir: service.env.local overrides, site.json in the ConfigMap, patches and resources in
#              the kustomize build, the auto-detected service.env.local, the refusal inside a git work tree (a temporary
#              repository) and --allow-local-in-git, deploy.sh with it; components/host-share: rendered only with
#              HOST_SHARE_HOST_PATH, a node path with spaces, quotes and '-== ==-' through the YAML and kustomize, the
#              init container (service image, non-root, no capabilities) and HostToContainer in both containers, the
#              refusals, and the init container's script run against constructed mount tables (cifs, the node's disk
#              only, a name prefix, cifs over autofs, autofs alone, a directory the user cannot write)
#   assetstore projects/k3s/assetstore: rendered against the owner's cluster and only local-path, the kustomize build
#              (host network, node, probes, security, resources, the Secret mount, no Secret), and the agreement of its
#              service file, site settings, bootstrap file and the program's ModuleInfo (ports, drain, password file,
#              memory soft limit, plugin list)
#
# Exit code 0 when every check passes, 1 otherwise. KEEP_SCRATCH=1 keeps the scratch directory.
set -u

TESTS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
KIT_DIR="$(cd "$TESTS_DIR/.." && pwd)"
SCRIPTS="$KIT_DIR/scripts"
CC="${CC:-cc}"
SCRATCH="$(mktemp -d "${TMPDIR:-/tmp}/gucef-service-tests.XXXXXX")"
cleanup() {
    chmod -R u+rwX "$SCRATCH" 2> /dev/null
    if [ "${KEEP_SCRATCH:-0}" = "1" ]; then echo "scratch kept: $SCRATCH"; else rm -rf "$SCRATCH"; fi
}
trap cleanup EXIT

passed=0
failed=0
check() {
    local what="$1"
    shift
    if "$@" > /dev/null 2>&1; then
        passed=$((passed + 1)); echo "  ok    $what"
    else
        failed=$((failed + 1)); echo "  FAIL  $what"
    fi
}
fails() { ! "$@"; }
contains() { grep -q -F -- "$2" "$1"; }
not_exists() { [ ! -e "$1" ] && [ ! -L "$1" ]; }
exit_code_is() { local expected="$1"; shift; "$@" > /dev/null 2>&1; [ $? -eq "$expected" ]; }

# an example service with its files, in a directory of its own
make_service() {
    local dir="$1"
    mkdir -p "$dir"
    # an explicit class name: the base checks need no class list (the storage section tests the resolution)
    sed -e 's/^STORAGE_CLASS=.*/STORAGE_CLASS=example-class/' "$KIT_DIR/service.env.example" > "$dir/service.env"
    cat > "$dir/ExampleService_bootstrap_container.ini" <<'INI'

[Bootstrap\PluginGroup]
"GroupName"="bootstrap"
"LoadImmediately"="true"

[Bootstrap\PluginGroup\PluginMetaData]
"Description"="data storage codec: JSON"
"Path"="$MODULEDIR$"
"Filename"="dstorepluginJSONPARSER"
"Type"="GucefDataStoreCodecPlugin"
"LoadFailAllowed"="false"
INI
    printf '{\n    "ExampleService": {\n        "Service": { "dataDir": "/data" }\n    }\n}\n' > "$dir/ExampleService_site.json"
}

echo "syntax"
for script in "$SCRIPTS"/*.sh "$TESTS_DIR"/*.sh; do
    check "bash -n $(basename "$script")" bash -n "$script"
done
check "no tab in any file of the kit" fails grep -R -l -P '\t' "$KIT_DIR"
check "every script is executable" bash -c "for f in '$SCRIPTS'/*.sh; do [ \"\$(basename \$f)\" = lib.sh ] || [ -x \"\$f\" ] || exit 1; done"

echo "env"
cat > "$SCRATCH/good.env" <<'ENV'
# a comment
A_KEY=plain
QUOTED="two words"
SINGLE='$NOT_EXPANDED'
EMPTY=
ENV
check "the reader takes quotes off and expands nothing" bash -c "source '$SCRIPTS/lib.sh'; gs_load_env '$SCRATCH/good.env'; [ \"\$A_KEY\" = plain ] && [ \"\$QUOTED\" = 'two words' ] && [ \"\$SINGLE\" = '\$NOT_EXPANDED' ] && [ -z \"\$EMPTY\" ]"
printf 'lower=1\n' > "$SCRATCH/bad.env"
check "the reader refuses a key that is not upper case" fails bash -c "source '$SCRIPTS/lib.sh'; gs_load_env '$SCRATCH/bad.env'"
printf 'KEY value\n' > "$SCRATCH/bad2.env"
check "the reader refuses a line without =" fails bash -c "source '$SCRIPTS/lib.sh'; gs_load_env '$SCRATCH/bad2.env'"
check "gs_require names a missing key" fails bash -c "source '$SCRIPTS/lib.sh'; gs_require NOT_THERE_AT_ALL"

echo "render"
make_service "$SCRATCH/svc"
OUT="$SCRATCH/rendered"
# the example as shipped is in lan mode (the default) without a node name: refused, with a message that says what to set
check "the example as shipped (lan, no node name) is refused" fails "$SCRIPTS/render_manifests.sh" "$SCRATCH/svc/service.env" "$SCRATCH/r-shipped"
check "the refusal names DISCOVERY_NODE_NAME and the way out" bash -c "'$SCRIPTS/render_manifests.sh' '$SCRATCH/svc/service.env' '$SCRATCH/r-shipped2' 2>&1 | grep -q 'needs DISCOVERY_NODE_NAME' && '$SCRIPTS/render_manifests.sh' '$SCRATCH/svc/service.env' '$SCRATCH/r-shipped3' 2>&1 | grep -q 'DISCOVERY_MODE=off'"
check "and writes nothing" fails bash -c "ls -A '$SCRATCH/r-shipped' 2> /dev/null | grep -q ."
# the base templates first, in off mode
sed -i 's/^DISCOVERY_MODE=.*/DISCOVERY_MODE=off/' "$SCRATCH/svc/service.env"
check "the example service file renders in off mode" "$SCRIPTS/render_manifests.sh" "$SCRATCH/svc/service.env" "$OUT"
expected=$(( $(find "$KIT_DIR/k3s" -type f -name '*.yaml' | wc -l) - $(find "$KIT_DIR/k3s/components/lan-discovery" "$KIT_DIR/k3s/components/secret-file" "$KIT_DIR/k3s/components/host-share" -type f -name '*.yaml' | wc -l) ))
check "every template is rendered, without the lan-discovery, secret-file and host-share components ($expected files)" bash -c "[ \$(find '$OUT' -type f -name '*.yaml' | wc -l) -eq $expected ]"
check "no placeholder is left" fails grep -R -E '@[A-Z][A-Z0-9_:]*@' "$OUT"
check "the site settings are in the ConfigMap, indented" contains "$OUT/configmap.yaml" '            "Service": { "dataDir": "/data" }'
check "the ConfigMap key is the site settings file name" contains "$OUT/configmap.yaml" '  ExampleService.json: |'
check "the container arguments are a YAML sequence" contains "$OUT/deployment.yaml" 'args: [ "--console", "--data-dir", "/data", "--config", "/opt/example-service/config/site/ExampleService.json" ]'
check "the Service is a LoadBalancer with externalTrafficPolicy Local" bash -c "grep -q 'type: LoadBalancer' '$OUT/service.yaml' && grep -q 'externalTrafficPolicy: Local' '$OUT/service.yaml'"
check "the Service exposes neither the health nor the admin port" fails grep -E '^\s+- \{ name: (health|admin)' "$OUT/service.yaml"
check "the Deployment is Recreate with one replica" bash -c "grep -q 'type: Recreate' '$OUT/deployment.yaml' && grep -q 'replicas: 1' '$OUT/deployment.yaml'"
check "the Deployment runs read-only, non-root, without capabilities" bash -c "grep -q 'readOnlyRootFilesystem: true' '$OUT/deployment.yaml' && grep -q 'runAsNonRoot: true' '$OUT/deployment.yaml' && grep -q 'drop: \[ \"ALL\" \]' '$OUT/deployment.yaml'"
check "the namespace enforces the restricted standard" contains "$OUT/namespace.yaml" 'pod-security.kubernetes.io/enforce: restricted'
check "the credentials component mounts the Secret 0440 read-only" bash -c "grep -q 'defaultMode: 0440' '$OUT/components/credentials-key/kustomization.yaml' && grep -q 'readOnly: true' '$OUT/components/credentials-key/kustomization.yaml'"
check "no tab in the rendered manifests" fails grep -R -l -P '\t' "$OUT"
check "a non-empty output directory is refused" exit_code_is 2 "$SCRIPTS/render_manifests.sh" "$SCRATCH/svc/service.env" "$OUT"
grep -v '^DATA_SIZE=' "$SCRATCH/svc/service.env" > "$SCRATCH/svc/unset.env"
check "an unset variable refuses the rendering" fails "$SCRIPTS/render_manifests.sh" "$SCRATCH/svc/unset.env" "$SCRATCH/r-unset"
check "and leaves no file with a placeholder" fails bash -c "grep -R -l '@DATA_SIZE@' '$SCRATCH/r-unset' 2> /dev/null | grep -q ."
sed 's/^MEMORY_LIMIT=.*/MEMORY_LIMIT=/' "$SCRATCH/svc/service.env" > "$SCRATCH/svc/empty.env"
check "an empty value refuses the rendering" fails "$SCRIPTS/render_manifests.sh" "$SCRATCH/svc/empty.env" "$SCRATCH/r-empty"
sed 's/^SERVICE_NAME=.*/SERVICE_NAME=Not_A_Label/' "$SCRATCH/svc/service.env" > "$SCRATCH/svc/name.env"
check "a service name that is no DNS label is refused" fails "$SCRIPTS/render_manifests.sh" "$SCRATCH/svc/name.env" "$SCRATCH/r-name"
sed 's/^TERMINATION_GRACE_SECONDS=.*/TERMINATION_GRACE_SECONDS=10/' "$SCRATCH/svc/service.env" > "$SCRATCH/svc/grace.env"
check "a grace period not above the drain is refused" fails "$SCRIPTS/render_manifests.sh" "$SCRATCH/svc/grace.env" "$SCRATCH/r-grace"
sed 's/^SITE_CONFIG_SOURCE=.*/SITE_CONFIG_SOURCE=missing.json/' "$SCRATCH/svc/service.env" > "$SCRATCH/svc/site.env"
check "a missing site settings file is refused" fails "$SCRIPTS/render_manifests.sh" "$SCRATCH/svc/site.env" "$SCRATCH/r-site"

echo "discovery"
# off: restricted, LoadBalancer, no host network, the lan component left out
check "discovery off: the lan-discovery component is left out" not_exists "$OUT/components/lan-discovery"
check "discovery off: the kustomization names no component" fails grep -q '^components:' "$OUT/kustomization.yaml"
check "discovery off: no host network anywhere" fails grep -R -q 'hostNetwork' "$OUT"
check "discovery off: restricted is enforced, warned and audited" bash -c "grep -q 'enforce: restricted' '$OUT/namespace.yaml' && grep -q 'warn: restricted' '$OUT/namespace.yaml' && grep -q 'audit: restricted' '$OUT/namespace.yaml'"
# a service file from before the discovery keys is in lan mode (the default): refused without a node name, lan with one
grep -v '^DISCOVERY_' "$SCRATCH/svc/service.env" > "$SCRATCH/svc/old.env"
check "a service file without the discovery keys is lan by default and refused without a node name" fails "$SCRIPTS/render_manifests.sh" "$SCRATCH/svc/old.env" "$SCRATCH/r-old"
{ cat "$SCRATCH/svc/old.env"; echo 'DISCOVERY_NODE_NAME=worker'; } > "$SCRATCH/svc/old-node.env"
check "with only DISCOVERY_NODE_NAME added it renders in lan mode" "$SCRIPTS/render_manifests.sh" "$SCRATCH/svc/old-node.env" "$SCRATCH/r-old-node"
check "and enforces privileged with the lan component" bash -c "grep -q 'enforce: privileged' '$SCRATCH/r-old-node/namespace.yaml' && grep -q '^  - components/lan-discovery$' '$SCRATCH/r-old-node/kustomization.yaml'"
# lan
sed -e 's/^DISCOVERY_MODE=.*/DISCOVERY_MODE=lan/' -e 's/^DISCOVERY_NODE_NAME=.*/DISCOVERY_NODE_NAME=worker/' "$SCRATCH/svc/service.env" > "$SCRATCH/svc/lan.env"
LAN="$SCRATCH/r-lan"
check "discovery lan renders" "$SCRIPTS/render_manifests.sh" "$SCRATCH/svc/lan.env" "$LAN"
check "discovery lan: every template is rendered, without the secret-file and host-share components" bash -c "[ \$(find '$LAN' -type f -name '*.yaml' | wc -l) -eq $(( $(find "$KIT_DIR/k3s" -type f -name '*.yaml' | wc -l) - $(find "$KIT_DIR/k3s/components/secret-file" "$KIT_DIR/k3s/components/host-share" -type f -name '*.yaml' | wc -l) )) ]"
check "discovery lan: no placeholder is left" fails grep -R -E '@[A-Z][A-Z0-9_:]*@' "$LAN"
check "discovery lan: the kustomization enables the component" bash -c "grep -q '^components:$' '$LAN/kustomization.yaml' && grep -q '^  - components/lan-discovery$' '$LAN/kustomization.yaml'"
check "discovery lan: the credentials key component is still a commented line" contains "$LAN/kustomization.yaml" '#   - components/credentials-key'
check "discovery lan: host network with cluster DNS" bash -c "grep -q 'hostNetwork: true' '$LAN/components/lan-discovery/deployment-host-network.yaml' && grep -q 'dnsPolicy: ClusterFirstWithHostNet' '$LAN/components/lan-discovery/deployment-host-network.yaml'"
check "discovery lan: pinned to the node of the service file" contains "$LAN/components/lan-discovery/deployment-host-network.yaml" 'kubernetes.io/hostname: "worker"'
check "discovery lan: the discovery port is declared as UDP" contains "$LAN/components/lan-discovery/deployment-host-network.yaml" '- { name: discovery, containerPort: 47800, protocol: UDP }'
check "discovery lan: Recreate stays" contains "$LAN/components/lan-discovery/deployment-host-network.yaml" 'type: Recreate'
check "discovery lan: the Service becomes ClusterIP without externalTrafficPolicy" bash -c "grep -q 'type: ClusterIP' '$LAN/components/lan-discovery/service-cluster-ip.yaml' && grep -q 'externalTrafficPolicy: null' '$LAN/components/lan-discovery/service-cluster-ip.yaml'"
check "discovery lan: privileged is enforced, restricted still warned and audited" bash -c "grep -q 'enforce: privileged' '$LAN/namespace.yaml' && grep -q 'warn: restricted' '$LAN/namespace.yaml' && grep -q 'audit: restricted' '$LAN/namespace.yaml'"
check "discovery lan: the pod itself stays non-root, read-only, without capabilities" bash -c "grep -q 'runAsNonRoot: true' '$LAN/deployment.yaml' && grep -q 'readOnlyRootFilesystem: true' '$LAN/deployment.yaml' && grep -q 'drop: \[ \"ALL\" \]' '$LAN/deployment.yaml'"
check "discovery lan: no tab in the rendered manifests" fails grep -R -l -P '\t' "$LAN"
sed -e 's/^DISCOVERY_MODE=.*/DISCOVERY_MODE=lan/' "$SCRATCH/svc/service.env" > "$SCRATCH/svc/lan-nonode.env"
check "discovery lan without a node name is refused" fails "$SCRIPTS/render_manifests.sh" "$SCRATCH/svc/lan-nonode.env" "$SCRATCH/r-lan-nonode"
check "and writes nothing" fails bash -c "ls -A '$SCRATCH/r-lan-nonode' 2> /dev/null | grep -q ."
sed -e 's/^DISCOVERY_MODE=.*/DISCOVERY_MODE=wan/' "$SCRATCH/svc/service.env" > "$SCRATCH/svc/mode.env"
check "an unknown discovery mode is refused" fails "$SCRIPTS/render_manifests.sh" "$SCRATCH/svc/mode.env" "$SCRATCH/r-mode"
sed -e 's/^DISCOVERY_MODE=.*/DISCOVERY_MODE=lan/' -e 's/^DISCOVERY_NODE_NAME=.*/DISCOVERY_NODE_NAME=worker/' -e 's/^DISCOVERY_PORT=.*/DISCOVERY_PORT=70000/' "$SCRATCH/svc/service.env" > "$SCRATCH/svc/port.env"
check "a discovery port that is no port is refused" fails "$SCRIPTS/render_manifests.sh" "$SCRATCH/svc/port.env" "$SCRATCH/r-port"
sed -e 's/^DISCOVERY_MODE=.*/DISCOVERY_MODE=lan/' -e 's/^DISCOVERY_NODE_NAME=.*/DISCOVERY_NODE_NAME=Worker_1/' "$SCRATCH/svc/service.env" > "$SCRATCH/svc/node.env"
check "a node name that is no DNS name is refused" fails "$SCRIPTS/render_manifests.sh" "$SCRATCH/svc/node.env" "$SCRATCH/r-node"
# the kustomize build of both modes, offline (kubectl kustomize reads files only, it talks to no cluster)
if command -v kubectl > /dev/null 2>&1; then
    check "kustomize builds the off mode" bash -c "kubectl kustomize '$OUT' > '$SCRATCH/off.built.yaml'"
    check "the built off mode has a LoadBalancer and no host network" bash -c "grep -q 'type: LoadBalancer' '$SCRATCH/off.built.yaml' && ! grep -q 'hostNetwork' '$SCRATCH/off.built.yaml'"
    check "kustomize builds the lan mode" bash -c "kubectl kustomize '$LAN' > '$SCRATCH/lan.built.yaml'"
    check "the built lan mode: host network, both node selectors, ClusterIP, no externalTrafficPolicy" bash -c "grep -q 'hostNetwork: true' '$SCRATCH/lan.built.yaml' && grep -q 'kubernetes.io/hostname: worker' '$SCRATCH/lan.built.yaml' && grep -q 'gucef-service-host: \"true\"' '$SCRATCH/lan.built.yaml' && grep -q 'type: ClusterIP' '$SCRATCH/lan.built.yaml' && ! grep -q 'externalTrafficPolicy' '$SCRATCH/lan.built.yaml' && ! grep -q 'type: LoadBalancer' '$SCRATCH/lan.built.yaml'"
    check "the built lan mode keeps the TCP ports and adds the discovery port" bash -c "grep -q 'containerPort: 47400' '$SCRATCH/lan.built.yaml' && grep -q 'containerPort: 47800' '$SCRATCH/lan.built.yaml' && grep -q 'protocol: UDP' '$SCRATCH/lan.built.yaml'"
else
    echo "  skip  kustomize builds (no kubectl)"
fi

echo "storage"
RESOLVE="$SCRIPTS/resolve_storage_class.sh"
CL="$SCRATCH/classes"
mkdir -p "$CL"
# the simulated cluster of the owner and variations
printf 'storageclass.storage.k8s.io/%s\n' local-path longhorn longhorn-raid longhorn-mirror longhorn-jbod longhorn-static > "$CL/cluster"
printf 'storageclass.storage.k8s.io/%s\n' local-path raid jbod longhorn-raid longhorn-mirror longhorn-jbod > "$CL/both"
printf 'storageclass.storage.k8s.io/%s\n' raid longhorn-raid > "$CL/raid-plain-first"
printf 'storageclass.storage.k8s.io/%s\n' local-path > "$CL/local-only"
printf 'storageclass.storage.k8s.io/%s\n' longhorn longhorn-raid longhorn-static > "$CL/longhorn-only"
printf 'storageclass.storage.k8s.io/%s\n' longhorn longhorn-static > "$CL/no-candidate"
printf 'storageclass.storage.k8s.io/%s\n' local-path longhorn-jbod longhorn-mirror > "$CL/no-raid"
printf 'storageclass.storage.k8s.io/%s\n' local-path jbod > "$CL/host-node-jbod"
printf 'storageclass.storage.k8s.io/%s\n' raid jbod > "$CL/host-node-only"
printf '# a comment\n\nlocal-path\r\n  raid  \nstorageclass/longhorn-mirror\n' > "$CL/plain-names"
printf 'storageclass.storage.k8s.io/Not_A_Name\n' > "$CL/garbage"
: > "$CL/empty"
resolves_to() { local expected="$1"; shift; [ "$("$RESOLVE" "$@" 2> /dev/null)" = "$expected" ]; }
check "raid prefers the Longhorn class when both exist" resolves_to longhorn-raid --classes-file "$CL/both" raid
check "raid prefers the Longhorn class whatever the list order" resolves_to longhorn-raid --classes-file "$CL/raid-plain-first" raid
check "mirror prefers the Longhorn class" resolves_to longhorn-mirror --classes-file "$CL/both" mirror
check "jbod prefers the Longhorn class" resolves_to longhorn-jbod --classes-file "$CL/both" jbod
check "local-path is local-path" resolves_to local-path --classes-file "$CL/both" local-path
check "raid takes the host-node class when there is no Longhorn one" resolves_to raid --classes-file "$CL/host-node-only" raid
check "jbod takes the host-node class when there is no Longhorn one" resolves_to jbod --classes-file "$CL/host-node-jbod" jbod
check "auto on the owner's cluster is longhorn-raid" resolves_to longhorn-raid --classes-file "$CL/cluster" auto
check "auto without raid takes mirror" resolves_to longhorn-mirror --classes-file "$CL/no-raid" auto
check "auto with jbod and local-path takes jbod" resolves_to jbod --classes-file "$CL/host-node-jbod" auto
check "auto with only local-path takes local-path" resolves_to local-path --classes-file "$CL/local-only" auto
check "auto with only Longhorn classes takes longhorn-raid" resolves_to longhorn-raid --classes-file "$CL/longhorn-only" auto
check "auto with both raid forms takes longhorn-raid" resolves_to longhorn-raid --classes-file "$CL/both" auto
check "auto without any candidate is refused (no fallback to the default class)" fails "$RESOLVE" --classes-file "$CL/no-candidate" auto
check "and the refusal lists the classes found and the candidates tried" bash -c "'$RESOLVE' --classes-file '$CL/no-candidate' auto 2>&1 | grep -q 'classes found: longhorn longhorn-static' && '$RESOLVE' --classes-file '$CL/no-candidate' auto 2>&1 | grep -q 'raid(longhorn-raid,raid)'"
check "a friendly name without a candidate is refused" fails "$RESOLVE" --classes-file "$CL/local-only" mirror
check "and the refusal names its candidates" bash -c "'$RESOLVE' --classes-file '$CL/local-only' mirror 2>&1 | grep -q 'longhorn-mirror mirror'"
check "an explicit class name of the cluster is passed through" resolves_to longhorn-static --classes-file "$CL/cluster" longhorn-static
check "an explicit class name the cluster lacks is refused" fails "$RESOLVE" --classes-file "$CL/cluster" fast-ssd
check "plain names, comments, CRLF and blanks are read" resolves_to longhorn-mirror --classes-file "$CL/plain-names" mirror
check "a line that is no class name is refused" fails "$RESOLVE" --classes-file "$CL/garbage" auto
check "an empty list is refused" fails "$RESOLVE" --classes-file "$CL/empty" auto
check "the list from standard input" bash -c "[ \"\$('$RESOLVE' --classes-file - auto < '$CL/local-only' 2> /dev/null)\" = local-path ]"
check "a request that is no class name is refused" exit_code_is 1 "$RESOLVE" --classes-file "$CL/cluster" 'Bad Name'
check "only the class goes to standard output" bash -c "[ \"\$('$RESOLVE' --classes-file '$CL/cluster' auto 2> /dev/null | wc -l)\" -eq 1 ]"
# KUBECTL: a command prefix, split on white space, never evaluated; a fake kubectl and a fake ssh that re-parses its
# arguments in a shell as a real remote shell does
FAKE_BIN="$SCRATCH/fakebin"
export FAKE_K8S="$SCRATCH/fake-k8s"
mkdir -p "$FAKE_BIN" "$FAKE_K8S"
cat > "$FAKE_BIN/kubectl" <<'SH'
#!/bin/bash
# stand-in for kubectl: answers from the files in $FAKE_K8S and logs every call, one argument per line
{ echo "--- call"; printf '%s\n' "$@"; } >> "${FAKE_K8S:?}/calls.log"
args=" $* "
case "$args" in
    *" get storageclass -o name "*) [ -f "$FAKE_K8S/classes" ] && cat "$FAKE_K8S/classes"; exit 0 ;;
    *" get pvc "*) [ -f "$FAKE_K8S/pvc-class" ] && cat "$FAKE_K8S/pvc-class"; exit 0 ;;
    *" get node -l "*) [ -f "$FAKE_K8S/labelled" ] && cat "$FAKE_K8S/labelled"; exit 0 ;;
    *" get node "*) [ -f "$FAKE_K8S/images" ] && cat "$FAKE_K8S/images"; exit 0 ;;
    *" get namespace "*) [ -f "$FAKE_K8S/namespace" ] && echo "namespace/$(cat "$FAKE_K8S/namespace")"; exit 0 ;;
    *" get secret "*) [ -f "$FAKE_K8S/secret" ] && printf 'present'; exit 0 ;;
    *" apply -f - "*)
        count=$(( $(find "$FAKE_K8S" -name 'applied-*.yaml' | wc -l) + 1 ))
        cat > "$FAKE_K8S/applied-$count.yaml"
        grep -q '^kind: Namespace' "$FAKE_K8S/applied-$count.yaml" && sed -n 's/^  name: //p' "$FAKE_K8S/applied-$count.yaml" | head -n 1 > "$FAKE_K8S/namespace"
        exit 0 ;;
    *" rollout status "*) exit "${FAKE_ROLLOUT_EXIT:-0}" ;;
esac
echo "fake kubectl: not expected: $*" >&2
exit 9
SH
cat > "$FAKE_BIN/ssh" <<'SH'
#!/bin/bash
# stand-in for ssh: the remote shell joins the arguments and parses them again, as sshd does
host="$1"
shift
echo "$host" >> "${FAKE_K8S:?}/ssh-hosts"
PATH="$(dirname "$0"):$PATH" exec bash -c "$*"
SH
chmod +x "$FAKE_BIN/kubectl" "$FAKE_BIN/ssh"
cp "$CL/cluster" "$FAKE_K8S/classes"
check "KUBECTL: the list comes from the command prefix" bash -c "[ \"\$(KUBECTL='$FAKE_BIN/kubectl' '$RESOLVE' auto 2> /dev/null)\" = longhorn-raid ]"
check "KUBECTL: a prefix of several words through a remote shell (ssh host kubectl)" bash -c "[ \"\$(KUBECTL='$FAKE_BIN/ssh k3smaster kubectl' '$RESOLVE' auto 2> /dev/null)\" = longhorn-raid ] && grep -qx k3smaster '$FAKE_K8S/ssh-hosts'"
check "KUBECTL: shell characters in the value are words, never evaluated" bash -c "KUBECTL='$FAKE_BIN/kubectl ;touch $SCRATCH/pwned \$(touch $SCRATCH/pwned2)' '$RESOLVE' auto > /dev/null 2>&1; [ ! -e '$SCRATCH/pwned' ] && [ ! -e '$SCRATCH/pwned2' ]"
check "KUBECTL: a failing command is refused with its name" bash -c "KUBECTL=false '$RESOLVE' auto 2>&1 | grep -q \"'false get storageclass -o name' failed\""
# render_manifests and the storage class
make_service "$SCRATCH/st"
sed -i -e 's/^STORAGE_CLASS=.*/STORAGE_CLASS=auto/' -e 's/^DISCOVERY_MODE=.*/DISCOVERY_MODE=off/' "$SCRATCH/st/service.env"
check "render: auto without the cluster's classes is refused" fails "$SCRIPTS/render_manifests.sh" "$SCRATCH/st/service.env" "$SCRATCH/r-st-nolist"
check "and says how to give them" bash -c "'$SCRIPTS/render_manifests.sh' '$SCRATCH/st/service.env' '$SCRATCH/r-st-nolist2' 2>&1 | grep -q -- '--storage-classes-file'"
check "and writes nothing" fails bash -c "ls -A '$SCRATCH/r-st-nolist' 2> /dev/null | grep -q ."
grep -v '^STORAGE_CLASS=' "$SCRATCH/st/service.env" > "$SCRATCH/st/nokey.env"
check "render: a service file without STORAGE_CLASS is auto" bash -c "'$SCRIPTS/render_manifests.sh' --storage-classes-file '$CL/local-only' '$SCRATCH/st/nokey.env' '$SCRATCH/r-st-nokey' > /dev/null 2>&1 && grep -q 'storageClassName: local-path' '$SCRATCH/r-st-nokey/pvc.yaml'"
check "render: auto against the owner's cluster gives longhorn-raid" bash -c "'$SCRIPTS/render_manifests.sh' --storage-classes-file '$CL/cluster' '$SCRATCH/st/service.env' '$SCRATCH/r-st-auto' > /dev/null 2>&1 && grep -q 'storageClassName: longhorn-raid' '$SCRATCH/r-st-auto/pvc.yaml'"
check "render: the claim notes the request and the resolution" contains "$SCRATCH/r-st-auto/pvc.yaml" 'STORAGE_CLASS auto -> longhorn-raid'
check "render: no candidate refuses and writes nothing" bash -c "! '$SCRIPTS/render_manifests.sh' --storage-classes-file '$CL/no-candidate' '$SCRATCH/st/service.env' '$SCRATCH/r-st-none' > /dev/null 2>&1 && ! ls -A '$SCRATCH/r-st-none' 2> /dev/null | grep -q ."
check "render: --storage-class takes a resolved class as it is" bash -c "'$SCRIPTS/render_manifests.sh' --storage-class longhorn-jbod '$SCRATCH/st/service.env' '$SCRATCH/r-st-given' > /dev/null 2>&1 && grep -q 'storageClassName: longhorn-jbod' '$SCRATCH/r-st-given/pvc.yaml'"
check "render: --query-cluster asks KUBECTL" bash -c "KUBECTL='$FAKE_BIN/kubectl' '$SCRIPTS/render_manifests.sh' --query-cluster '$SCRATCH/st/service.env' '$SCRATCH/r-st-query' > /dev/null 2>&1 && grep -q 'storageClassName: longhorn-raid' '$SCRATCH/r-st-query/pvc.yaml'"
sed -i 's/^STORAGE_CLASS=.*/STORAGE_CLASS=fast-ssd/' "$SCRATCH/st/service.env"
check "render: an explicit name without a list is used as it is" bash -c "'$SCRIPTS/render_manifests.sh' '$SCRATCH/st/service.env' '$SCRATCH/r-st-name' > /dev/null 2>&1 && grep -q 'storageClassName: fast-ssd' '$SCRATCH/r-st-name/pvc.yaml'"
check "render: an explicit name the listed cluster lacks is refused" fails "$SCRIPTS/render_manifests.sh" --storage-classes-file "$CL/cluster" "$SCRATCH/st/service.env" "$SCRATCH/r-st-name2"
check "render: two class sources are a usage error" exit_code_is 2 "$SCRIPTS/render_manifests.sh" --storage-class a --storage-classes-file "$CL/cluster" "$SCRATCH/st/service.env" "$SCRATCH/r-st-two"
sed -i 's/^STORAGE_CLASS=.*/STORAGE_CLASS=Bad_Class/' "$SCRATCH/st/service.env"
check "render: a STORAGE_CLASS that is no name is refused" fails "$SCRIPTS/render_manifests.sh" "$SCRATCH/st/service.env" "$SCRATCH/r-st-bad"

echo "components"
make_service "$SCRATCH/sf"
sed -i -e 's/^DISCOVERY_MODE=.*/DISCOVERY_MODE=off/' -e 's/^SECRET_FILE_SECRET_NAME=.*/SECRET_FILE_SECRET_NAME=example-db/' "$SCRATCH/sf/service.env"
SF="$SCRATCH/r-sf"
check "secret-file: renders with SECRET_FILE_SECRET_NAME" "$SCRIPTS/render_manifests.sh" "$SCRATCH/sf/service.env" "$SF"
check "secret-file: enabled in the kustomization" bash -c "grep -q '^components:$' '$SF/kustomization.yaml' && grep -q '^  - components/secret-file$' '$SF/kustomization.yaml'"
check "secret-file: mounted read-only at its directory" bash -c "grep -q 'mountPath: /secrets/example-service-db' '$SF/components/secret-file/kustomization.yaml' && grep -q 'readOnly: true' '$SF/components/secret-file/kustomization.yaml'"
check "secret-file: only its key, mode 0400" bash -c "grep -q 'key: password' '$SF/components/secret-file/kustomization.yaml' && grep -q 'mode: 0400' '$SF/components/secret-file/kustomization.yaml' && grep -q 'defaultMode: 0400' '$SF/components/secret-file/kustomization.yaml'"
check "secret-file: its example is rendered, not in the kustomization" bash -c "[ -f '$SF/components/secret-file/secret.example.yaml' ] && ! grep -v '^ *#' '$SF/components/secret-file/kustomization.yaml' | grep -q 'secret.example' && ! grep -q '^  - .*secret.example' '$SF/kustomization.yaml'"
check "secret-file: the example creates the Secret from a pipe or a file" bash -c "grep -q -- '--from-file=password=/dev/stdin' '$SF/components/secret-file/secret.example.yaml' && grep -q 'create secret generic example-db' '$SF/components/secret-file/secret.example.yaml'"
check "secret-file: the credentials key component stays a commented line" contains "$SF/kustomization.yaml" '#   - components/credentials-key'
if command -v kubectl > /dev/null 2>&1; then
    check "secret-file: kustomize builds it" bash -c "kubectl kustomize '$SF' > '$SCRATCH/sf.built.yaml'"
    check "secret-file: the built Deployment mounts the Secret 0400 (256) read-only" bash -c "grep -q 'secretName: example-db' '$SCRATCH/sf.built.yaml' && grep -q 'defaultMode: 256' '$SCRATCH/sf.built.yaml' && grep -q 'mountPath: /secrets/example-service-db' '$SCRATCH/sf.built.yaml'"
    check "secret-file: the built manifests hold no Secret" fails grep -q '^kind: Secret' "$SCRATCH/sf.built.yaml"
fi
for bad in 'SECRET_FILE_KEY=pass-word' 'SECRET_FILE_MOUNT_DIR=/data/secret' 'SECRET_FILE_MOUNT_DIR=secrets/db' 'SECRET_FILE_MOUNT_DIR=/secrets/../etc' 'SECRET_FILE_MODE=0666' 'SECRET_FILE_MODE=rw' 'SECRET_FILE_SECRET_NAME=Bad_Name'; do
    sed "s|^${bad%%=*}=.*|$bad|" "$SCRATCH/sf/service.env" > "$SCRATCH/sf/bad.env"
    check "secret-file: $bad is refused" fails "$SCRIPTS/render_manifests.sh" "$SCRATCH/sf/bad.env" "$SCRATCH/r-sf-bad-$(echo "$bad" | tr -c 'A-Za-z0-9' _)"
done
sed 's/^SECRET_FILE_MODE=.*/SECRET_FILE_MODE=440/' "$SCRATCH/sf/service.env" > "$SCRATCH/sf/mode.env"
check "secret-file: a three digit mode becomes 0440" bash -c "'$SCRIPTS/render_manifests.sh' '$SCRATCH/sf/mode.env' '$SCRATCH/r-sf-mode' > /dev/null 2>&1 && grep -q 'defaultMode: 0440' '$SCRATCH/r-sf-mode/components/secret-file/kustomization.yaml'"
check "secret-file: off without SECRET_FILE_SECRET_NAME (the example)" bash -c "[ ! -e '$OUT/components/secret-file' ] && ! grep -v '^#' '$OUT/kustomization.yaml' | grep -q 'secret-file'"
sed -e 's/^CREDENTIALS_SECRET_NAME=.*/CREDENTIALS_SECRET_NAME=""/' "$SCRATCH/sf/service.env" | grep -v '^CREDENTIALS_KEY_' > "$SCRATCH/sf/nocred.env"
check "credentials-key: optional, renders without its keys" "$SCRIPTS/render_manifests.sh" "$SCRATCH/sf/nocred.env" "$SCRATCH/r-nocred"
check "credentials-key: then neither the component nor its example nor the commented line" bash -c "[ ! -e '$SCRATCH/r-nocred/components/credentials-key' ] && [ ! -e '$SCRATCH/r-nocred/secret.example.yaml' ] && ! grep -q '^#   - components/credentials-key' '$SCRATCH/r-nocred/kustomization.yaml'"
check "credentials-key: with its name set the keys are still required" bash -c "grep -v '^CREDENTIALS_KEY_FILE_ENV=' '$SCRATCH/sf/service.env' > '$SCRATCH/sf/credhalf.env'; ! '$SCRIPTS/render_manifests.sh' '$SCRATCH/sf/credhalf.env' '$SCRATCH/r-credhalf' > /dev/null 2>&1"
sed -e 's/^DISCOVERY_MODE=.*/DISCOVERY_MODE=lan/' -e 's/^DISCOVERY_NODE_NAME=.*/DISCOVERY_NODE_NAME=worker/' "$SCRATCH/sf/service.env" > "$SCRATCH/sf/lan.env"
check "both components: lan-discovery and secret-file enabled together" bash -c "'$SCRIPTS/render_manifests.sh' '$SCRATCH/sf/lan.env' '$SCRATCH/r-sf-lan' > /dev/null 2>&1 && grep -q '^  - components/lan-discovery$' '$SCRATCH/r-sf-lan/kustomization.yaml' && grep -q '^  - components/secret-file$' '$SCRATCH/r-sf-lan/kustomization.yaml'"

echo "staging"
if ! command -v "$CC" > /dev/null; then
    echo "  skip  no C compiler ($CC): the staging checks need one"
else
    B="$SCRATCH/build"
    mkdir -p "$B/bin" "$B/lib" "$B/src"
    cat > "$B/src/plugin.c" <<'C'
int plugin_value( void ) { return 40; }
C
    cat > "$B/src/named.c" <<'C'
int named_value( void ) { return 1; }
C
    cat > "$B/src/core.c" <<'C'
int plugin_value( void );
int core_value( void ) { return plugin_value() + 2; }
C
    cat > "$B/src/main.c" <<'C'
#include <stdio.h>
int core_value( void );
int main( void ) { if ( core_value() == 42 ) { puts( "staged program ok" ); return 0; } return 1; }
C
    # built with the RUNPATHs of the staged layout, so the stand-in patchelf has nothing to change
    "$CC" -shared -fPIC -o "$B/lib/libtestpluginECHO.so" "$B/src/plugin.c" -Wl,-rpath,'$ORIGIN/../lib:$ORIGIN' \
    && "$CC" -shared -fPIC -o "$B/lib/libnamedthing.so" "$B/src/named.c" -Wl,-rpath,'$ORIGIN/../lib:$ORIGIN' \
    && "$CC" -shared -fPIC -o "$B/lib/libcore.so" "$B/src/core.c" -L"$B/lib" -ltestpluginECHO -Wl,-rpath,'$ORIGIN:$ORIGIN/../plugins' \
    && "$CC" -o "$B/bin/TestService" "$B/src/main.c" -L"$B/lib" -lcore -ltestpluginECHO -Wl,-rpath,'$ORIGIN/../lib:$ORIGIN/../plugins' \
    || echo "  FAIL  building the test tree"
    cp "$B/bin/TestService" "$B/bin/TestService_TestApp"
    cp "$B/bin/TestService" "$B/bin/extratool"
    mkdir -p "$SCRATCH/extrasrc" "$SCRATCH/nolicencesrc"
    printf 'GPL text\n' > "$SCRATCH/extrasrc/COPYING"
    printf 'readme\n' > "$SCRATCH/extrasrc/README.txt"
    printf 'code\n' > "$SCRATCH/extrasrc/tool.c"
    printf 'code\n' > "$SCRATCH/nolicencesrc/tool.c"
    printf 'debug\n' > "$B/bin/TestService_bootstrap_d.ini"
    printf '[Bootstrap\\PluginGroup\\PluginMetaData]\n"Path"="$MODULEDIR$"\n"Filename"="buildonly"\n' > "$B/bin/TestService_bootstrap.ini"
    printf '{}\n' > "$B/bin/TestService_DefaultConfig.json"
    cat > "$SCRATCH/container.ini" <<'INI'
[Bootstrap\PluginGroup\PluginMetaData]
"Path"="$MODULEDIR$"
"Filename"="testpluginECHO"
"LoadFailAllowed"="false"

[Bootstrap\PluginGroup\PluginMetaData]
"Path"="$MODULEDIR$"
"Filename"="namedthing"
"LoadFailAllowed"="false"

;[Bootstrap\PluginGroup\PluginMetaData]
;"Filename"="commentedout"
;"LoadFailAllowed"="false"
INI
    cat > "$SCRATCH/patchelf" <<'SH'
#!/bin/bash
[ "$1" = "--set-rpath" ] && [ -f "$3" ] || exit 3
echo "$2 $3" >> "${PATCHELF_LOG:?}"
SH
    chmod +x "$SCRATCH/patchelf"
    export PATCHELF="$SCRATCH/patchelf" PATCHELF_LOG="$SCRATCH/patchelf.log"
    S="$SCRATCH/stage/opt/test-service"
    check "stage_install stages the build" "$SCRIPTS/stage_install.sh" "$S" --program TestService --bootstrap "$SCRATCH/container.ini" "$B"
    check "the program is in bin/" test -x "$S/bin/TestService"
    check "the test application is left out" not_exists "$S/bin/TestService_TestApp"
    check "the debug bootstrap file is left out" not_exists "$S/bin/TestService_bootstrap_d.ini"
    check "the plugin of the name pattern is in plugins/" test -f "$S/plugins/libtestpluginECHO.so"
    check "the plugin named only by the bootstrap file is in plugins/" test -f "$S/plugins/libnamedthing.so"
    check "the plugins are not in lib/ as well" bash -c "[ ! -e '$S/lib/libtestpluginECHO.so' ] && [ ! -e '$S/lib/libnamedthing.so' ]"
    check "the library is in lib/" test -f "$S/lib/libcore.so"
    check "the bootstrap file of the image replaced the build's" contains "$S/config/TestService_bootstrap.ini" '"Filename"="namedthing"'
    check "its plugin path points at plugins/" contains "$S/config/TestService_bootstrap.ini" '"Path"="$MODULEDIR$/../plugins"'
    check "bin/ links to the config files" bash -c "[ -L '$S/bin/TestService_bootstrap.ini' ] && [ -L '$S/bin/TestService_DefaultConfig.json' ]"
    check "programs get \$ORIGIN/../lib:\$ORIGIN/../plugins" contains "$PATCHELF_LOG" '$ORIGIN/../lib:$ORIGIN/../plugins '"$S/bin/TestService"
    check "libraries get \$ORIGIN:\$ORIGIN/../plugins" contains "$PATCHELF_LOG" '$ORIGIN:$ORIGIN/../plugins '"$S/lib/libcore.so"
    check "plugins get \$ORIGIN/../lib:\$ORIGIN" contains "$PATCHELF_LOG" '$ORIGIN/../lib:$ORIGIN '"$S/plugins/libnamedthing.so"
    check "a staged tree is not writable by others" fails bash -c "find '$S' \( -type f -o -type d \) -perm -o+w | grep -q ."
    check "an existing destination is refused" exit_code_is 2 "$SCRIPTS/stage_install.sh" "$S" --program TestService "$B"
    check "a program that is not built is refused" fails "$SCRIPTS/stage_install.sh" "$SCRATCH/stage2" --program Missing "$B"
    S2="$SCRATCH/stage-extra/opt/test-service"
    check "stage_install --extra-program stages the tool" "$SCRIPTS/stage_install.sh" "$S2" --program TestService --bootstrap "$SCRATCH/container.ini" --extra-program extratool="$SCRATCH/extrasrc" "$B"
    check "the extra program is in bin/ next to the program" test -x "$S2/bin/extratool"
    check "its licence and readme files are in licences/<directory name>/" bash -c "[ -f '$S2/licences/extrasrc/COPYING' ] && [ -f '$S2/licences/extrasrc/README.txt' ]"
    check "only licence files are copied, not its sources" not_exists "$S2/licences/extrasrc/tool.c"
    check "an extra program that is not built is refused" fails "$SCRIPTS/stage_install.sh" "$SCRATCH/stage-extra2" --program TestService --extra-program notbuilt="$SCRATCH/extrasrc" "$B"
    check "an extra program without a licence file is refused" fails "$SCRIPTS/stage_install.sh" "$SCRATCH/stage-extra3" --program TestService --extra-program extratool="$SCRATCH/nolicencesrc" "$B"
    check "an extra program with a missing source directory is refused" fails "$SCRIPTS/stage_install.sh" "$SCRATCH/stage-extra4" --program TestService --extra-program extratool="$SCRATCH/not-there" "$B"
    check "an extra program without a directory is a usage error" exit_code_is 2 "$SCRIPTS/stage_install.sh" "$SCRATCH/stage-extra5" --program TestService --extra-program extratool "$B"
    mv "$B" "$SCRATCH/build-moved"
    check "check_libs passes on the staged tree" "$S/bin/check_libs.sh" "$S" --program TestService
    check "check_libs passes on the tree with an extra program" "$S2/bin/check_libs.sh" "$S2" --program TestService
    check "the staged program runs from the staged tree alone" bash -c "env -u LD_LIBRARY_PATH '$S/bin/TestService' | grep -q 'staged program ok'"
    check "check_libs fails for a program that is not there" fails "$S/bin/check_libs.sh" "$S" --program Other
    cp -a "$S" "$SCRATCH/s-noplugin"
    rm -f "$SCRATCH/s-noplugin/plugins/libnamedthing.so"
    check "check_libs fails for a missing required plugin" fails "$SCRATCH/s-noplugin/bin/check_libs.sh" "$SCRATCH/s-noplugin"
    cp -a "$S" "$SCRATCH/s-outside"
    mkdir -p "$SCRATCH/elsewhere"
    cp "$SCRATCH/build-moved/lib/libtestpluginECHO.so" "$SCRATCH/elsewhere/libtestpluginECHO.so"
    "$CC" -shared -fPIC -o "$SCRATCH/s-outside/lib/libbad.so" "$SCRATCH/build-moved/src/core.c" -L"$SCRATCH/elsewhere" -ltestpluginECHO -Wl,-rpath,"$SCRATCH/elsewhere"
    check "check_libs fails for a library resolved outside the tree" bash -c "'$SCRATCH/s-outside/bin/check_libs.sh' '$SCRATCH/s-outside' 2>&1 | grep -q 'outside the image'"
    # --plugin-links lib: a program that loads its plugins as $MODULEDIR$/<name> finds them through lib/
    SL="$SCRATCH/stage-links/opt/test-service"
    check "stage_install --plugin-links lib stages the build" "$SCRIPTS/stage_install.sh" "$SL" --program TestService --bootstrap "$SCRATCH/container.ini" --plugin-links lib "$SCRATCH/build-moved"
    check "lib/ links each plugin to plugins/" bash -c "[ \"\$(readlink '$SL/lib/libtestpluginECHO.so')\" = ../plugins/libtestpluginECHO.so ] && [ \"\$(readlink '$SL/lib/libnamedthing.so')\" = ../plugins/libnamedthing.so ]"
    check "the plugins themselves stay in plugins/ only" bash -c "[ -f '$SL/plugins/libtestpluginECHO.so' ] && [ ! -L '$SL/plugins/libtestpluginECHO.so' ] && [ -z \"\$(find '$SL/lib' -type f -name 'libtestpluginECHO*')\" ]"
    check "bin/ gets no plugin links without being asked" not_exists "$SL/bin/libtestpluginECHO.so"
    check "check_libs passes on the tree with links" "$SL/bin/check_libs.sh" "$SL" --program TestService
    check "the staged program with links runs from the tree alone" bash -c "env -u LD_LIBRARY_PATH '$SL/bin/TestService' | grep -q 'staged program ok'"
    check "a plugin link directory other than bin and lib is refused" exit_code_is 2 "$SCRIPTS/stage_install.sh" "$SCRATCH/stage-links2" --program TestService --plugin-links etc "$SCRATCH/build-moved"
    cp -a "$SL" "$SCRATCH/s-dangling"
    rm -f "$SCRATCH/s-dangling/plugins/libnamedthing.so"
    check "check_libs fails for a dangling link" bash -c "'$SCRATCH/s-dangling/bin/check_libs.sh' '$SCRATCH/s-dangling' 2>&1 | grep -q 'dangling link: lib/libnamedthing.so'"
    cp -a "$SL" "$SCRATCH/s-outlink"
    ln -s "$SCRATCH/elsewhere/libtestpluginECHO.so" "$SCRATCH/s-outlink/lib/libelsewhere.so"
    check "check_libs fails for a link that leaves the tree" bash -c "'$SCRATCH/s-outlink/bin/check_libs.sh' '$SCRATCH/s-outlink' 2>&1 | grep -q 'link outside the image tree'"
    unset PATCHELF PATCHELF_LOG
fi

echo "context"
R="$SCRATCH/repo"
mkdir -p "$R/sub"
git -C "$SCRATCH" init -q repo
printf 'tracked\n' > "$R/tracked.txt"
printf 'gone\n' > "$R/deleted.txt"
printf 'ignored.txt\nbuild/\n' > "$R/.gitignore"
git -C "$R" add -A && git -C "$R" -c user.email=t@example.invalid -c user.name=test commit -q -m init
rm "$R/deleted.txt"
printf 'new\n' > "$R/sub/untracked.txt"
printf 'x\n' > "$R/ignored.txt"
mkdir -p "$R/build" && printf 'x\n' > "$R/build/out.o"
X="$SCRATCH/extra"
mkdir -p "$X" && git -C "$X" init -q && printf 'e\n' > "$X/e.txt" && git -C "$X" add -A && git -C "$X" -c user.email=t@example.invalid -c user.name=test commit -q -m e
C="$SCRATCH/context"
check "make_context makes the context" bash -c "GUCEF_HOME='$R' '$SCRIPTS/make_context.sh' '$C' 'Org/product=$X' > '$SCRATCH/context.log'"
check "tracked files are in it" test -f "$C/src/GUCEF/tracked.txt"
check "untracked, not ignored files are in it" test -f "$C/src/GUCEF/sub/untracked.txt"
check "ignored files are not" not_exists "$C/src/GUCEF/ignored.txt"
check "build directories are not" not_exists "$C/src/GUCEF/build"
check "files deleted in the working tree are not" not_exists "$C/src/GUCEF/deleted.txt"
check "the extra checkout is below its name" test -f "$C/src/Org/product/e.txt"
check "a checkout with changes is reported dirty" bash -c "tail -n 1 '$SCRATCH/context.log' | grep -qx dirty"
check "an existing context is refused" exit_code_is 2 env GUCEF_HOME="$R" "$SCRIPTS/make_context.sh" "$C"
check "an unusable extra name is refused" fails env GUCEF_HOME="$R" "$SCRIPTS/make_context.sh" "$SCRATCH/context2" "../up=$X"

echo "image"
G="$SCRATCH/gucef"
mkdir -p "$G/projects/CMake/targets/GUCEF_exe_ExampleService"
check "build_image --dry-run renders the Dockerfile" env GUCEF_HOME="$G" DRY_RUN_DOCKERFILE="$SCRATCH/Dockerfile" "$SCRIPTS/build_image.sh" --env "$SCRATCH/svc/service.env" --dry-run --tag dev
check "no placeholder is left in the Dockerfile" fails grep -E '@[A-Z][A-Z0-9_:]*@' "$SCRATCH/Dockerfile"
check "the entry point is the service program" contains "$SCRATCH/Dockerfile" 'ENTRYPOINT ["/opt/example-service/bin/ExampleService"]'
check "the image runs as 10001" contains "$SCRATCH/Dockerfile" 'USER 10001:10001'
check "the image build runs check_libs" contains "$SCRATCH/Dockerfile" '/opt/example-service/bin/check_libs.sh /opt/example-service --program ExampleService'
sed 's/^IMAGE_TARGETS=.*/IMAGE_TARGETS="GUCEF_exe_NotThere"/' "$SCRATCH/svc/service.env" > "$SCRATCH/svc/notarget.env"
check "a target without build files is refused before docker" bash -c "env GUCEF_HOME='$G' PATH=/usr/bin:/bin '$SCRIPTS/build_image.sh' --env '$SCRATCH/svc/notarget.env' 2>&1 | grep -q 'no build files for: GUCEF_exe_NotThere'"
check "an invalid tag is refused" exit_code_is 2 env GUCEF_HOME="$G" "$SCRIPTS/build_image.sh" --env "$SCRATCH/svc/service.env" --tag 'bad tag'
check "the service file is required" exit_code_is 2 "$SCRIPTS/build_image.sh"
sed 's/^BOOTSTRAP_INI=.*/BOOTSTRAP_INI=missing.ini/' "$SCRATCH/svc/service.env" > "$SCRATCH/svc/noboot.env"
check "a missing bootstrap file is refused" fails env GUCEF_HOME="$G" "$SCRIPTS/build_image.sh" --env "$SCRATCH/svc/noboot.env" --dry-run
check "the Dockerfile passes PLUGIN_LINKS to the staging" bash -c "grep -q '^ARG PLUGIN_LINKS=' '$SCRATCH/Dockerfile' && grep -q -- '--plugin-links \"\${PLUGIN_LINKS}\"' '$SCRATCH/Dockerfile'"
sed 's/^PLUGIN_LINKS=.*/PLUGIN_LINKS="lib etc"/' "$SCRATCH/svc/service.env" > "$SCRATCH/svc/links.env"
check "a PLUGIN_LINKS other than bin and lib is refused before docker" bash -c "env GUCEF_HOME='$G' '$SCRIPTS/build_image.sh' --env '$SCRATCH/svc/links.env' --dry-run 2>&1 | grep -q 'PLUGIN_LINKS takes bin and lib'"
check "the Dockerfile passes EXTRA_PROGRAMS to the staging" bash -c "grep -q '^ARG EXTRA_PROGRAMS=' '$SCRATCH/Dockerfile' && grep -q -F -- '--extra-program \${extra%%=*}=/src/\${TARGETS_CHECKOUT}/\${extra#*=}' '$SCRATCH/Dockerfile'"
sed 's|^EXTRA_PROGRAMS=.*|EXTRA_PROGRAMS="tool=some/dir"|' "$SCRATCH/svc/service.env" > "$SCRATCH/svc/extra.env"
check "EXTRA_PROGRAMS goes to docker as a build argument" bash -c "env GUCEF_HOME='$G' '$SCRIPTS/build_image.sh' --env '$SCRATCH/svc/extra.env' --dry-run --tag dev 2>&1 | grep -q -- '--build-arg EXTRA_PROGRAMS=tool=some/dir'"
sed 's|^EXTRA_PROGRAMS=.*|EXTRA_PROGRAMS="tool=../up"|' "$SCRATCH/svc/service.env" > "$SCRATCH/svc/extrabad.env"
check "an EXTRA_PROGRAMS entry that leaves the checkout is refused before docker" bash -c "env GUCEF_HOME='$G' '$SCRIPTS/build_image.sh' --env '$SCRATCH/svc/extrabad.env' --dry-run 2>&1 | grep -q 'EXTRA_PROGRAMS takes name=directory'"

echo "local"
D="$SCRATCH/data"
"$SCRIPTS/run_local.sh" --env "$SCRATCH/svc/service.env" "$D" --dry-run > "$SCRATCH/run.txt" 2>&1
for flag in '--read-only' '--cap-drop ALL' '--security-opt no-new-privileges' '/tmp:rw\,nosuid\,nodev\,size=512m' '--stop-timeout 45' '127.0.0.1:47480:47480' '127.0.0.1:47400:47400' 'GUCEF_CONSOLE_LOGGER=stderr'; do
    check "run_local: $flag" contains "$SCRATCH/run.txt" "$flag"
done
check "run_local publishes no admin port" fails contains "$SCRATCH/run.txt" '47403'
check "run_local mounts the site settings read-only" contains "$SCRATCH/run.txt" 'config/site/ExampleService.json:ro'
"$SCRIPTS/run_local.sh" --env "$SCRATCH/svc/service.env" "$D" --dry-run --bind 0.0.0.0 > "$SCRATCH/run2.txt" 2>&1
check "run_local --bind publishes the session port on the LAN, health stays on loopback" bash -c "grep -q '0.0.0.0:47400:47400' '$SCRATCH/run2.txt' && grep -q '127.0.0.1:47480:47480' '$SCRATCH/run2.txt'"
check "run_local needs the data directory" exit_code_is 2 "$SCRIPTS/run_local.sh" --env "$SCRATCH/svc/service.env"

echo "deploy"
# deploy.sh against the fake kubectl (and the fake ssh), with the asset store's service folder
ASSET_DIR="$(cd "$KIT_DIR/.." && pwd)/assetstore"
ASSET_ENV="$ASSET_DIR/service.env"
DEPLOY="$SCRIPTS/deploy.sh"
export DEPLOY_WORK_DIR="$SCRATCH/deploy-work"
if ! command -v kubectl > /dev/null 2>&1; then
    # without kubectl a stand-in build: the base resources concatenated (no components)
    printf '#!/bin/bash\nfor f in namespace pvc configmap deployment service; do cat "$1/$f.yaml"; echo ---; done\n' > "$FAKE_BIN/kustomize-standin"
    chmod +x "$FAKE_BIN/kustomize-standin"
    export KUSTOMIZE="$FAKE_BIN/kustomize-standin"
fi
fake_reset() {
    rm -rf "$FAKE_K8S"
    mkdir -p "$FAKE_K8S"
    cp "$CL/cluster" "$FAKE_K8S/classes"
}
applied_count() { find "$FAKE_K8S" -name 'applied-*.yaml' | wc -l; }
run_deploy() { KUBECTL="${DK:-$FAKE_BIN/kubectl}" "$DEPLOY" --env "$ASSET_ENV" "$@" > "$SCRATCH/deploy.log" 2>&1; }
# 1. no node with the label: refused before anything is applied
fake_reset
check "deploy: no labelled node is refused" fails run_deploy
check "deploy: and names the label command" contains "$SCRATCH/deploy.log" 'kubectl label node worker-x299 gucef-service-host=true'
check "deploy: and applies nothing" bash -c "[ \$(find '$FAKE_K8S' -name 'applied-*.yaml' | wc -l) -eq 0 ]"
# 2. the label on another node than DISCOVERY_NODE_NAME: refused
echo node/workstationznix > "$FAKE_K8S/labelled"
check "deploy: the label on another node than DISCOVERY_NODE_NAME is refused" fails run_deploy
# 3. the Secret missing: the namespace is made, then it stops with the command to create the Secret
fake_reset
printf 'node/worker-x299\nnode/workstationznix\n' > "$FAKE_K8S/labelled"
check "deploy: a missing Secret stops the deploy" fails run_deploy
check "deploy: the namespace is created first (the Secret goes into it)" bash -c "[ \$(find '$FAKE_K8S' -name 'applied-*.yaml' | wc -l) -eq 1 ] && grep -q '^kind: Namespace' '$FAKE_K8S/applied-1.yaml' && grep -q 'enforce: privileged' '$FAKE_K8S/applied-1.yaml'"
check "deploy: the message gives the create command from a pipe" bash -c "grep -q 'create secret generic assetstore-db --from-file=password=/dev/stdin' '$SCRATCH/deploy.log'"
check "deploy: the Secret check passes the template through unchanged and asks only for presence" bash -c "grep -qx 'go-template={{with.data.password}}present{{end}}' '$FAKE_K8S/calls.log'"
# 4. everything in place: applied and rolled out (applied-1 is the namespace of 3, applied-2 the namespace again,
#    applied-3 the built manifests)
touch "$FAKE_K8S/secret"
check "deploy: with the Secret it applies and rolls out" run_deploy
check "deploy: the built manifests applied: Namespace, claim, ConfigMap, Deployment, Service" bash -c "f='$FAKE_K8S/applied-3.yaml'; for k in Namespace PersistentVolumeClaim ConfigMap Deployment Service; do grep -q \"^kind: \$k\$\" \"\$f\" || exit 1; done"
check "deploy: no Secret is ever applied" fails grep -q '^kind: Secret' "$FAKE_K8S"/applied-*.yaml
check "deploy: the claim has the resolved class" contains "$FAKE_K8S/applied-3.yaml" 'storageClassName: longhorn-raid'
check "deploy: it waits for the rollout with the startup time" bash -c "grep -qx 'deployment/assetstore' '$FAKE_K8S/calls.log' && grep -qx -- '--timeout=420s' '$FAKE_K8S/calls.log'"
check "deploy: the missing image is a warning" contains "$SCRATCH/deploy.log" 'WARNING: docker.io/vanvelzen/asset-store-service:dev is not in the image list of worker-x299'
check "deploy: the rendered and built manifests are kept" bash -c "ls '$DEPLOY_WORK_DIR'/assetstore/*/built.yaml > /dev/null && ls '$DEPLOY_WORK_DIR'/assetstore/*/rendered/kustomization.yaml > /dev/null"
# 5. a second run: the same, no error (idempotent)
printf '[{"names":["docker.io/vanvelzen/asset-store-service:dev"],"sizeBytes":1}]' > "$FAKE_K8S/images"
echo longhorn-raid > "$FAKE_K8S/pvc-class"
check "deploy: a second run succeeds as well" run_deploy
check "deploy: and finds the image on the node" contains "$SCRATCH/deploy.log" 'the image docker.io/vanvelzen/asset-store-service:dev is on worker-x299'
# 6. an existing claim with another class: refused before anything is applied
fake_reset
printf 'node/worker-x299\n' > "$FAKE_K8S/labelled"
touch "$FAKE_K8S/secret"
echo local-path > "$FAKE_K8S/pvc-class"
check "deploy: a claim with another class than the resolution is refused" fails run_deploy
check "deploy: and says how to keep it" contains "$SCRATCH/deploy.log" 'Set STORAGE_CLASS=local-path'
check "deploy: and applies nothing" bash -c "[ \$(find '$FAKE_K8S' -name 'applied-*.yaml' | wc -l) -eq 0 ]"
# 7. no StorageClass candidate: refused before anything
rm -f "$FAKE_K8S/pvc-class"
cp "$CL/no-candidate" "$FAKE_K8S/classes"
check "deploy: no StorageClass candidate is refused" fails run_deploy
check "deploy: and applies nothing" bash -c "[ \$(find '$FAKE_K8S' -name 'applied-*.yaml' | wc -l) -eq 0 ]"
# 8. a rollout that does not complete
fake_reset
printf 'node/worker-x299\n' > "$FAKE_K8S/labelled"
touch "$FAKE_K8S/secret"
check "deploy: a rollout that does not complete fails" bash -c "FAKE_ROLLOUT_EXIT=1 KUBECTL='$FAKE_BIN/kubectl' '$DEPLOY' --env '$ASSET_ENV' > /dev/null 2>&1; [ \$? -eq 1 ]"
# 9. the dry run changes nothing
fake_reset
printf 'node/worker-x299\n' > "$FAKE_K8S/labelled"
check "deploy: --dry-run passes without the namespace and the Secret" run_deploy --dry-run
check "deploy: --dry-run applies nothing" bash -c "[ \$(find '$FAKE_K8S' -name 'applied-*.yaml' | wc -l) -eq 0 ] && ! grep -qx 'rollout' '$FAKE_K8S/calls.log'"
check "deploy: --dry-run reports the missing Secret" contains "$SCRATCH/deploy.log" '(dry run) the Secret assetstore/assetstore-db with the key password is missing'
check "deploy: --dry-run says the namespace would be created" contains "$SCRATCH/deploy.log" '(dry run) the namespace assetstore would be created'
# 10. through a remote shell: KUBECTL="ssh k3smaster kubectl"
fake_reset
printf 'node/worker-x299\n' > "$FAKE_K8S/labelled"
touch "$FAKE_K8S/secret"
check "deploy: through ssh k3smaster kubectl (arguments re-parsed by a shell)" bash -c "KUBECTL='$FAKE_BIN/ssh k3smaster kubectl' '$DEPLOY' --env '$ASSET_ENV' > '$SCRATCH/deploy-ssh.log' 2>&1"
check "deploy: through ssh the manifests arrive on standard input and the templates intact" bash -c "grep -q 'storageClassName: longhorn-raid' '$FAKE_K8S/applied-2.yaml' && grep -qx 'go-template={{with.data.password}}present{{end}}' '$FAKE_K8S/calls.log' && grep -qx 'jsonpath={.spec.storageClassName}' '$FAKE_K8S/calls.log' && [ \$(grep -c . '$FAKE_K8S/ssh-hosts') -ge 5 ]"
check "deploy: usage errors" bash -c "'$DEPLOY' > /dev/null 2>&1; [ \$? -eq 2 ] && '$DEPLOY' --env '$ASSET_ENV' --timeout 1m > /dev/null 2>&1; [ \$? -eq 2 ]"
unset DEPLOY_WORK_DIR

echo "local configuration"
# the local install configuration and the host-share component; the scratch directory must lie outside git
if [ -n "$(cd "$SCRATCH" && env -u GIT_DIR -u GIT_WORK_TREE git rev-parse --show-toplevel 2> /dev/null)" ]; then
    echo "  FAIL  the scratch directory $SCRATCH lies inside a git work tree: set TMPDIR to a directory outside git"
    failed=$((failed + 1))
fi
check "host-share: off without HOST_SHARE_HOST_PATH (no component, no hostPath)" bash -c "[ ! -e '$OUT/components/host-share' ] && ! grep -R -q 'hostPath' '$OUT' && ! grep -v '^#' '$LAN/kustomization.yaml' | grep -q 'host-share'"
make_service "$SCRATCH/lsvc"
sed -i -e 's/^DISCOVERY_MODE=.*/DISCOVERY_MODE=lan/' -e 's/^DISCOVERY_NODE_NAME=.*/DISCOVERY_NODE_NAME=from-repo/' -e 's/^SECRET_FILE_SECRET_NAME=.*/SECRET_FILE_SECRET_NAME=example-db/' "$SCRATCH/lsvc/service.env"
LD="$SCRATCH/local-site"
mkdir -p "$LD/patches" "$LD/resources"
# a node path with spaces, '-== ==-', double quotes and a backslash
cat > "$LD/service.env.local" <<'ENV'
# the site's values
HOST_SHARE_HOST_PATH='/mnt/nas/-== Assets ==-/Asset "Store" \x'
DISCOVERY_NODE_NAME=from-local
MEMORY_LIMIT=3Gi
ENV
printf '{\n    "ExampleService": { "Service": { "dataDir": "/data", "site": "local-site-marker" } }\n}\n' > "$LD/site.json"
cat > "$LD/patches/annotate.yaml" <<'YAML'
apiVersion: apps/v1
kind: Deployment
metadata:
  name: @SERVICE_NAME@
  namespace: @NAMESPACE@
  annotations:
    example.invalid/local-patch: "applied"
YAML
cat > "$LD/resources/extra.yaml" <<'YAML'
apiVersion: v1
kind: ConfigMap
metadata:
  name: local-extra
  namespace: @NAMESPACE@
data:
  note: "a resource of the local configuration"
YAML
L="$SCRATCH/r-local"
check "local: renders with --local-dir" "$SCRIPTS/render_manifests.sh" --local-dir "$LD" "$SCRATCH/lsvc/service.env" "$L"
check "local: the overrides of service.env.local win" bash -c "grep -q 'memory: 3Gi' '$L/deployment.yaml' && grep -q 'kubernetes.io/hostname: \"from-local\"' '$L/components/lan-discovery/deployment-host-network.yaml'"
check "local: site.json replaces the site settings in the ConfigMap" bash -c "grep -q 'local-site-marker' '$L/configmap.yaml' && ! grep -q '\"Service\": { \"dataDir\": \"/data\" }' '$L/configmap.yaml'"
check "local: the patch and the resource are rendered below local/" bash -c "grep -q 'namespace: gucef-example' '$L/local/patches/annotate.yaml' && grep -q 'namespace: gucef-example' '$L/local/resources/extra.yaml'"
check "local: the kustomization lists the resource and the patch" bash -c "grep -q '^  - local/resources/extra.yaml$' '$L/kustomization.yaml' && grep -q '^  - path: local/patches/annotate.yaml$' '$L/kustomization.yaml'"
check "host-share: rendered and enabled with HOST_SHARE_HOST_PATH" bash -c "[ -f '$L/components/host-share/deployment-host-share.yaml' ] && grep -q '^  - components/host-share$' '$L/kustomization.yaml'"
check "host-share: the node path is one YAML double-quoted scalar" contains "$L/components/host-share/deployment-host-share.yaml" 'path: "/mnt/nas/-== Assets ==-/Asset \"Store\" \\x"'
check "host-share: the namespace is privileged (hostPath)" contains "$L/namespace.yaml" 'enforce: privileged'
if command -v kubectl > /dev/null 2>&1; then
    LB="$SCRATCH/local.built.yaml"
    check "local: kustomize builds it" bash -c "kubectl kustomize '$L' > '$LB'"
    check "local: the patch is applied after the base and the components" bash -c "grep -q 'example.invalid/local-patch: applied' '$LB' && grep -q 'memory: 3Gi' '$LB'"
    check "local: the extra resource is in the build" bash -c "grep -q 'name: local-extra' '$LB'"
    check "host-share built: the path survives kustomize unchanged" contains "$LB" '/mnt/nas/-== Assets ==-/Asset "Store" \x'
    check "host-share built: a hostPath of type Directory" bash -c "grep -q 'hostPath:' '$LB' && grep -q 'type: Directory' '$LB'"
    check "host-share built: mounted at /share with HostToContainer in both containers" bash -c "[ \$(grep -c 'mountPropagation: HostToContainer' '$LB') -eq 2 ] && [ \$(grep -c 'mountPath: /share' '$LB') -eq 2 ]"
    check "host-share built: the init container of the service image, non-root, no capabilities" bash -c "awk '/initContainers:/,/nodeSelector:/' '$LB' > '$SCRATCH/init.yaml' && grep -q 'name: host-share-check' '$SCRATCH/init.yaml' && grep -q 'image: docker.io/vanvelzen/example-service:dev' '$SCRATCH/init.yaml' && grep -q 'runAsNonRoot: true' '$SCRATCH/init.yaml' && grep -q 'allowPrivilegeEscalation: false' '$SCRATCH/init.yaml' && grep -q -- '- ALL' '$SCRATCH/init.yaml' && grep -q 'readOnlyRootFilesystem: true' '$SCRATCH/init.yaml'"
    check "host-share built: the init container gets the accepted types and the node path" bash -c "grep -q -- '- cifs smb3 nfs nfs4' '$LB' && grep -q -- '- /share' '$LB'"
    check "host-share built: still host network, the secret file and no Secret" bash -c "grep -q 'hostNetwork: true' '$LB' && grep -q 'secretName: example-db' '$LB' && ! grep -q '^kind: Secret' '$LB'"
fi
# auto-detection: a service.env.local next to the service file (outside git) is used without --local-dir
make_service "$SCRATCH/autosvc"
sed -i -e 's/^DISCOVERY_MODE=.*/DISCOVERY_MODE=off/' "$SCRATCH/autosvc/service.env"
printf 'MEMORY_LIMIT=5Gi\n' > "$SCRATCH/autosvc/service.env.local"
check "local: a service.env.local next to the service file is used" bash -c "'$SCRIPTS/render_manifests.sh' '$SCRATCH/autosvc/service.env' '$SCRATCH/r-auto-local' > /dev/null 2>&1 && grep -q 'memory: 5Gi' '$SCRATCH/r-auto-local/deployment.yaml'"
# host-share in off mode raises the namespace to privileged, with a note
printf 'HOST_SHARE_HOST_PATH=/srv/share\n' >> "$SCRATCH/autosvc/service.env.local"
check "host-share: in off mode the namespace becomes privileged, with a note" bash -c "'$SCRIPTS/render_manifests.sh' '$SCRATCH/autosvc/service.env' '$SCRATCH/r-auto-hs' > '$SCRATCH/r-auto-hs.log' 2>&1 && grep -q 'Pod Security level privileged' '$SCRATCH/r-auto-hs.log' && grep -q 'enforce: privileged' '$SCRATCH/r-auto-hs/namespace.yaml'"
printf 'HOST_SHARE_SUBPATH="sub dir/library"\nHOST_SHARE_MOUNT_PATH=/library\nHOST_SHARE_FSTYPES=nfs4\n' >> "$SCRATCH/autosvc/service.env.local"
check "host-share: the subpath is joined to the node path; mount path and types as set" bash -c "'$SCRIPTS/render_manifests.sh' '$SCRATCH/autosvc/service.env' '$SCRATCH/r-auto-sub' > /dev/null 2>&1 && grep -q 'path: \"/srv/share/sub dir/library\"' '$SCRATCH/r-auto-sub/components/host-share/deployment-host-share.yaml' && grep -q 'mountPath: /library' '$SCRATCH/r-auto-sub/components/host-share/deployment-host-share.yaml' && grep -q -- '- \"nfs4\"' '$SCRATCH/r-auto-sub/components/host-share/deployment-host-share.yaml'"
rm -f "$SCRATCH/autosvc/service.env.local"
n=0
for bad in "HOST_SHARE_HOST_PATH=relative/share" "HOST_SHARE_HOST_PATH=/mnt/nas/../etc" "HOST_SHARE_HOST_PATH=/etc/share" "HOST_SHARE_HOST_PATH=/" "HOST_SHARE_HOST_PATH=/mnt/nas\$(printf '\t')x" "HOST_SHARE_SUBPATH=/abs" "HOST_SHARE_MOUNT_PATH=/data/share" "HOST_SHARE_MOUNT_PATH=share" "HOST_SHARE_MOUNT_PATH=/secrets/example-service-db/x" "HOST_SHARE_FSTYPES=CIFS!"; do
    n=$((n + 1))
    mkdir -p "$SCRATCH/bad-local-$n"
    { echo 'HOST_SHARE_HOST_PATH=/srv/share'; eval "printf '%s\n' \"$bad\""; } > "$SCRATCH/bad-local-$n/service.env.local"
    check "host-share: refused: $bad" fails "$SCRIPTS/render_manifests.sh" --local-dir "$SCRATCH/bad-local-$n" "$SCRATCH/lsvc/service.env" "$SCRATCH/r-bad-local-$n"
done
# a local directory inside a git work tree
GR="$SCRATCH/gitrepo"
mkdir -p "$GR/site"
git -C "$SCRATCH" init -q gitrepo
cp "$LD/service.env.local" "$GR/site/service.env.local"
check "local: a --local-dir inside a git work tree is refused" fails "$SCRIPTS/render_manifests.sh" --local-dir "$GR/site" "$SCRATCH/lsvc/service.env" "$SCRATCH/r-git"
check "local: and says why and what to do" bash -c "'$SCRIPTS/render_manifests.sh' --local-dir '$GR/site' '$SCRATCH/lsvc/service.env' '$SCRATCH/r-git2' 2>&1 | grep -q 'lies inside the git work tree' && '$SCRIPTS/render_manifests.sh' --local-dir '$GR/site' '$SCRATCH/lsvc/service.env' '$SCRATCH/r-git3' 2>&1 | grep -q -- '--allow-local-in-git'"
check "local: and writes nothing" fails bash -c "ls -A '$SCRATCH/r-git' 2> /dev/null | grep -q ."
check "local: --allow-local-in-git uses it with a note" bash -c "'$SCRIPTS/render_manifests.sh' --local-dir '$GR/site' --allow-local-in-git '$SCRATCH/lsvc/service.env' '$SCRATCH/r-git-allowed' > '$SCRATCH/r-git-allowed.log' 2>&1 && grep -q 'note:' '$SCRATCH/r-git-allowed.log' && [ -f '$SCRATCH/r-git-allowed/components/host-share/kustomization.yaml' ]"
cp -a "$SCRATCH/lsvc" "$GR/svc"
printf 'MEMORY_LIMIT=5Gi\n' > "$GR/svc/service.env.local"
check "local: a service.env.local next to a service file in git is refused as well" fails "$SCRIPTS/render_manifests.sh" "$GR/svc/service.env" "$SCRATCH/r-git-next"
check "local: deploy.sh refuses a local directory in git before it calls kubectl" bash -c "rm -f '$FAKE_K8S/calls.log'; ! KUBECTL='$FAKE_BIN/kubectl' '$SCRIPTS/deploy.sh' --env '$SCRATCH/lsvc/service.env' --local-dir '$GR/site' > /dev/null 2>&1 && [ ! -s '$FAKE_K8S/calls.log' ]"
# the init container's check, run here with constructed mount tables (the script as rendered)
sed -n '/^            - |$/,/^            - host-share-check$/p' "$L/components/host-share/deployment-host-share.yaml" | sed -e '1d' -e '$d' -e 's/^              //' > "$SCRATCH/host-share-check.sh"
NAS="$SCRATCH/nas mount"
SHARE="$NAS/-== Assets ==-/Store"
mkdir -p "$SHARE"
encode() { local p="$1"; p="${p//\\/\\134}"; p="${p// /\\040}"; printf '%s' "$p"; }
ROOTLINE="22 1 8:2 / / rw,relatime shared:1 - ext4 /dev/sda2 rw"
printf '%s\n%s\n' "$ROOTLINE" "88 22 0:61 / $(encode "$NAS") rw,relatime shared:50 - cifs //nas/share rw,vers=3.0" > "$SCRATCH/mi-cifs"
printf '%s\n' "$ROOTLINE" > "$SCRATCH/mi-none"
printf '%s\n%s\n' "$ROOTLINE" "88 22 0:61 / $(encode "${NAS% mount}") rw - cifs //nas/share rw" > "$SCRATCH/mi-prefix"
printf '%s\n%s\n%s\n' "$ROOTLINE" "87 22 0:60 / $(encode "$NAS") rw - autofs systemd-1 rw" "88 87 0:61 / $(encode "$NAS") rw - cifs //nas/share rw" > "$SCRATCH/mi-stacked"
printf '%s\n%s\n' "$ROOTLINE" "87 22 0:60 / $(encode "$NAS") rw - autofs systemd-1 rw" > "$SCRATCH/mi-autofs"
share_check() { HOST_SHARE_MOUNTINFO="$1" bash -c "$(cat "$SCRATCH/host-share-check.sh")" host-share-check "$SHARE" "cifs smb3 nfs nfs4" "/mnt/nas/-== Assets ==-/Store"; }
check "host-share check: the script is extracted" bash -c "[ -s '$SCRATCH/host-share-check.sh' ] && bash -n '$SCRATCH/host-share-check.sh'"
check "host-share check: a directory below a cifs mount passes (spaces and -== ==- in the path)" share_check "$SCRATCH/mi-cifs"
check "host-share check: and leaves no probe file" bash -c "[ -z \"\$(ls -A '$SHARE')\" ]"
check "host-share check: only the node's own disk is refused" fails share_check "$SCRATCH/mi-none"
check "host-share check: with a message that the share is not mounted" bash -c "HOST_SHARE_MOUNTINFO='$SCRATCH/mi-none' bash -c \"\$(cat '$SCRATCH/host-share-check.sh')\" x '$SHARE' 'cifs' n 2>&1 | grep -q 'not mounted on the node'"
check "host-share check: a mount at a mere name prefix does not count" fails share_check "$SCRATCH/mi-prefix"
check "host-share check: cifs on top of an automount passes" share_check "$SCRATCH/mi-stacked"
check "host-share check: an automount that is not mounted is refused" fails share_check "$SCRATCH/mi-autofs"
if [ "$(id -u)" != "0" ]; then
    chmod 555 "$SHARE"
    check "host-share check: a share the user cannot write is refused" bash -c "! HOST_SHARE_MOUNTINFO='$SCRATCH/mi-cifs' bash -c \"\$(cat '$SCRATCH/host-share-check.sh')\" x '$SHARE' 'cifs' n 2> '$SCRATCH/ro.txt' && grep -q 'not writable by uid' '$SCRATCH/ro.txt'"
    chmod 755 "$SHARE"
fi
# deploy.sh with the local configuration
fake_local_deploy() {
    rm -rf "$FAKE_K8S"; mkdir -p "$FAKE_K8S"; cp "$CL/cluster" "$FAKE_K8S/classes"
    printf 'storageclass.storage.k8s.io/example-class\n' >> "$FAKE_K8S/classes"
    printf 'node/from-local\n' > "$FAKE_K8S/labelled"; touch "$FAKE_K8S/secret"
    DEPLOY_WORK_DIR="$SCRATCH/deploy-local" KUBECTL="$FAKE_BIN/kubectl" "$SCRIPTS/deploy.sh" --env "$SCRATCH/lsvc/service.env" --local-dir "$LD" > "$SCRATCH/deploy-local.log" 2>&1
}
check "local: deploy.sh applies the local configuration" fake_local_deploy
check "local: the applied manifests hold the share, the init container and the local patch" bash -c "f='$FAKE_K8S/applied-2.yaml'; grep -q 'type: Directory' \"\$f\" && grep -q 'name: host-share-check' \"\$f\" && grep -q 'local-patch' \"\$f\" && grep -q 'deploy: the local install configuration' '$SCRATCH/deploy-local.log'"

echo "assetstore"
# the asset store's service folder: renders against the owner's cluster, and its files agree with each other
A="$SCRATCH/r-assetstore"
check "assetstore: renders against the owner's cluster" "$SCRIPTS/render_manifests.sh" --storage-classes-file "$CL/cluster" "$ASSET_ENV" "$A"
check "assetstore: the claim is longhorn-raid, 500Gi" bash -c "grep -q 'storageClassName: longhorn-raid' '$A/pvc.yaml' && grep -q 'storage: 500Gi' '$A/pvc.yaml'"
check "assetstore: renders on a cluster with only local-path" bash -c "'$SCRIPTS/render_manifests.sh' --storage-classes-file '$CL/local-only' '$ASSET_ENV' '$SCRATCH/r-assetstore-lp' > /dev/null 2>&1 && grep -q 'storageClassName: local-path' '$SCRATCH/r-assetstore-lp/pvc.yaml'"
check "assetstore: lan discovery and the secret file enabled, no credentials key component" bash -c "grep -q '^  - components/lan-discovery$' '$A/kustomization.yaml' && grep -q '^  - components/secret-file$' '$A/kustomization.yaml' && [ ! -e '$A/components/credentials-key' ]"
if command -v kubectl > /dev/null 2>&1; then
    AB="$SCRATCH/assetstore.built.yaml"
    check "assetstore: kustomize builds it" bash -c "kubectl kustomize '$A' > '$AB'"
    for expected in 'hostNetwork: true' 'dnsPolicy: ClusterFirstWithHostNet' 'kubernetes.io/hostname: worker-x299' 'gucef-service-host: "true"' 'type: Recreate' 'path: /health/started' 'path: /health/ready' 'path: /health/live' 'runAsNonRoot: true' 'runAsUser: 10001' 'readOnlyRootFilesystem: true' 'mountPath: /data' 'mountPath: /tmp' 'memory: 6Gi' 'memory: 1Gi' 'cpu: 250m' 'terminationGracePeriodSeconds: 45' 'imagePullPolicy: Never' 'image: docker.io/vanvelzen/asset-store-service:dev' 'type: ClusterIP' 'secretName: assetstore-db' 'mountPath: /secrets/assetstore-db' 'defaultMode: 256' 'enforce: privileged' 'containerPort: 47800'; do
        check "assetstore built: $expected" contains "$AB" "$expected"
    done
    check "assetstore built: no Secret, no LoadBalancer" bash -c "! grep -q '^kind: Secret' '$AB' && ! grep -q 'LoadBalancer' '$AB'"
    check "assetstore built: the probes use the health port" bash -c "[ \$(grep -c 'port: health' '$AB') -eq 3 ] && grep -q 'containerPort: 47480' '$AB'"
fi
SITE="$ASSET_DIR/AssetStoreService_site.json"
json_number() { grep -o "\"$1\": *[0-9]*" "$SITE" | head -n 1 | grep -o '[0-9]*$'; }
env_value() { sed -n "s/^$1=\"\{0,1\}\([^\"]*\)\"\{0,1\}\$/\1/p" "$ASSET_ENV"; }
check "assetstore: the password only as a file, at the secret file's path" bash -c "grep -q '\"passwordFile\": \"$(env_value SECRET_FILE_MOUNT_DIR)/$(env_value SECRET_FILE_KEY)\"' '$SITE' && ! grep -q '\"password\"' '$SITE' && ! grep -q 'passwordEnv' '$SITE'"
check "assetstore: drainSeconds is DRAIN_SECONDS" bash -c "[ '$(json_number drainSeconds)' = '$(env_value DRAIN_SECONDS)' ]"
check "assetstore: the ports of the settings are those of the service file" bash -c "[ '$(json_number healthPort)' = '$(env_value HEALTH_PORT)' ] && [ '$(json_number adminPort)' = '$(env_value ADMIN_PORT)' ] && [ '$(json_number sessionPort)' = '$(env_value SESSION_PORT)' ] && [ '$(json_number pairingPort)' = '$(env_value PAIRING_PORT)' ]"
check "assetstore: memorySoftLimitMB is below MEMORY_LIMIT" bash -c "limit='$(env_value MEMORY_LIMIT)'; case \"\$limit\" in *Gi) mb=\$(( \${limit%Gi} * 1024 )) ;; *Mi) mb=\${limit%Mi} ;; *) exit 1 ;; esac; [ '$(json_number memorySoftLimitMB)' -lt \"\$mb\" ]"
check "assetstore: the state is on /data (data, storage, credentials)" bash -c "grep -q '\"dataDir\": \"/data\"' '$SITE' && grep -q '\"path\": \"/data/' '$SITE' && grep -q '\"directory\": \"/data/credentials\"' '$SITE' && grep -q -- '--data-dir /data' '$ASSET_ENV'"
check "assetstore: the container arguments name the ConfigMap file" bash -c "grep -q -- \"--config /opt/$(env_value SERVICE_NAME)/config/site/$(env_value SITE_CONFIG_NAME)\" '$ASSET_ENV'"
modinfo="$(cd "$KIT_DIR/../../.." && pwd)/tools/AssetStoreService/ModuleInfo.xml"
extra_names=" $(env_value EXTRA_PROGRAMS | tr ' ' '\n' | sed 's/=.*//' | tr '\n' ' ')"
runtime_all="$(grep -o 'RuntimeDependency Name="[^"]*"' "$modinfo" 2> /dev/null | sed 's/.*="\(.*\)"/\1/' | awk '!seen[$0]++')"
plugins_moduleinfo="$(for n in $runtime_all; do case "$extra_names" in *" $n "*) ;; *) printf '%s ' "$n" ;; esac; done)"
plugins_ini="$(awk -F'"' '$1 == "" && $2 == "Filename" { print $4 }' "$ASSET_DIR/AssetStoreService_bootstrap_container.ini" | tr '\n' ' ')"
plugins_local="$(awk -F'"' '$1 == "" && $2 == "Filename" { print $4 }' "$(dirname "$modinfo")/config/AssetStoreService_bootstrap.ini" 2> /dev/null | tr '\n' ' ')"
check "assetstore: the plugins of the image are the RuntimeDependencies of the program, in order" bash -c "[ -n '$plugins_moduleinfo' ] && [ '$plugins_ini' = '$plugins_moduleinfo' ]"
check "assetstore: the bootstrap file of the program next to it names the same plugins" bash -c "[ '$plugins_local' = '$plugins_moduleinfo' ]"
check "assetstore: the site settings name no plugins (the bootstrap file loads them)" fails grep -q '"Plugins"' "$SITE"
check "assetstore: every plugin of the image is required" fails grep -q '"LoadFailAllowed"="true"' "$ASSET_DIR/AssetStoreService_bootstrap_container.ini"
TOOLS_DIR="$(cd "$KIT_DIR/../../.." && pwd)/tools"
ini_names() { awk -F'"' '$1 == "" && $2 == "Filename" { print $4 }' "$1" 2> /dev/null | tr '\n' ' '; }
ini_alts() { awk -F'"' '$1 == "" && $2 == "AltFilename" { print $4 }' "$1" 2> /dev/null | tr '\n' ' '; }
debug_ini="$(dirname "$modinfo")/config/AssetStoreService_bootstrap_d.ini"
plugins_debug="$(ini_names "$debug_ini" | sed 's/_d / /g')"
alts_debug="$(ini_alts "$debug_ini")"
check "assetstore: the debug bootstrap file of the program names the same plugins as _d files, with the release files as alternatives" bash -c "[ '$plugins_debug' = '$plugins_moduleinfo' ] && [ '$alts_debug' = '$plugins_moduleinfo' ]"
check "assetstore: the Unreal importer and the archive reader of the .mhpkg packs are in the ModuleInfo, the bootstrap file of the image and the one of the program" bash -c "for list in '$plugins_moduleinfo' '$plugins_ini' '$plugins_local'; do case \" \$list\" in *' assetpluginUNREAL '*' vfspluginLIBARCHIVE '*|*' vfspluginLIBARCHIVE '*' assetpluginUNREAL '*) ;; *) exit 1 ;; esac; done"
container_ini="$ASSET_DIR/AssetStoreService_bootstrap_container.ini"
check "assetstore: the image staging can rewrite the plugin paths: every plugin of the bootstrap file of the image has a \$MODULEDIR\$ path, a _d alternative and a type" bash -c "n=\$(grep -c '^\"Filename\"=' '$container_ini'); [ \"\$n\" -gt 0 ] && [ \"\$(grep -c '^\"Path\"=\"\$MODULEDIR\$\"\$' '$container_ini')\" = \"\$n\" ] && [ \"\$(grep -c '^\"AltFilename\"=.*_d\"\$' '$container_ini')\" = \"\$n\" ] && [ \"\$(grep -c '^\"Type\"=' '$container_ini')\" = \"\$n\" ]"
for host in AssetConverter AssetWorkbench AssetToolsMcp; do
    hostinfo="$TOOLS_DIR/$host/ModuleInfo.xml"
    check "importers: $host lists assetpluginUNREAL and vfspluginLIBARCHIVE as RuntimeDependencies" bash -c "grep -q 'RuntimeDependency Name=\"assetpluginUNREAL\"' '$hostinfo' && grep -q 'RuntimeDependency Name=\"vfspluginLIBARCHIVE\"' '$hostinfo'"
done
for host in AssetConverter AssetWorkbench; do
    for suffix in "" "_d"; do
        hostini="$TOOLS_DIR/$host/config/${host}_bootstrap$suffix.ini"
        check "importers: $host bootstrap$suffix.ini loads assetpluginUNREAL$suffix after the glTF, FBX and Unity importers and keeps it optional" bash -c "names=\" \$(awk -F'\"' '\$1 == \"\" && \$2 == \"Filename\" { print \$4 }' '$hostini' | tr '\n' ' ')\"; case \"\$names\" in *' assetpluginGLTF$suffix '*'assetpluginFBX$suffix '*'assetpluginUNITY$suffix '*'assetpluginUNREAL$suffix '*) ;; *) exit 1 ;; esac; awk '/^\"Filename\"=\"assetpluginUNREAL$suffix\"/ { f = 1 } f && /^\"LoadFailAllowed\"/ { print; exit }' '$hostini' | grep -q '\"true\"'"
    done
done
check "assetstore: no plugin links (the program loads its plugins through the bootstrap file, from plugins/)" bash -c "[ -z '$(env_value PLUGIN_LINKS)' ]"
check "assetstore: the service file ships ooz_extract from dependencies/ooz" bash -c "[ '$(env_value EXTRA_PROGRAMS)' = 'ooz_extract=dependencies/ooz' ]"
OOZ_DIR="$(cd "$KIT_DIR/../../.." && pwd)/dependencies/ooz"
check "assetstore: the licence files of the tool exist, so the image ships them under licences/ooz/" bash -c "[ -f '$OOZ_DIR/COPYING' ] && [ -f '$OOZ_DIR/README.txt' ]"
check "assetstore: ooz_extract is an executable runtime dependency of the program" bash -c "grep -q 'RuntimeDependency Name=\"ooz_extract\"' '$modinfo' && grep -q 'Type=\"Executable\"' '$OOZ_DIR/ModuleInfo.xml'"
check "assetstore: the dry run passes EXTRA_PROGRAMS to docker" bash -c "env GUCEF_HOME='$(cd "$KIT_DIR/../../.." && pwd)' '$SCRIPTS/build_image.sh' --env '$ASSET_ENV' --dry-run --tag dev 2>&1 | grep -q -- '--build-arg EXTRA_PROGRAMS=ooz_extract=dependencies/ooz'"
check "assetstore: no tab in its files" fails grep -R -l -P '\t' "$ASSET_DIR"
check "assetstore: the Dockerfile renders for it" env GUCEF_HOME="$(cd "$KIT_DIR/../../.." && pwd)" DRY_RUN_DOCKERFILE="$SCRATCH/assetstore.Dockerfile" "$SCRIPTS/build_image.sh" --env "$ASSET_ENV" --dry-run --tag dev

echo
echo "passed: $passed, failed: $failed"
[ "$failed" -eq 0 ]
