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
#   discovery  DISCOVERY_MODE off (the default, also for a service file without the key) and lan: the component, the
#              host network on the named node, the ClusterIP Service, the Pod Security levels, the refusals, and the
#              kustomize build of both when kubectl is there (offline: kubectl kustomize reads files only)
#   render     the manifests of the example service file: every template rendered, no placeholder left, the site
#              settings inserted with their indentation, refusals (an unset or empty value, a bad name, a grace period
#              not above the drain, a non-empty output directory) leave nothing behind
#   staging    a program, a library and two plugins (one recognised by its name pattern, one only because the bootstrap
#              file names it) built with the local compiler in the layout of a build directory; stage_install.sh with a
#              stand-in patchelf (which records what it was asked); check_libs.sh passes, the staged program runs from
#              the staged tree alone, and check_libs.sh fails for a missing required plugin, a missing program and a
#              library that resolves outside the tree
#   context    make_context.sh on a constructed git checkout: tracked, untracked, ignored and deleted files, the state
#   image      build_image.sh --dry-run: the rendered Dockerfile, refusals before docker is ever called
#   local      run_local.sh --dry-run: the restrictions of the pod on the docker command line
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
    cp "$KIT_DIR/service.env.example" "$dir/service.env"
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
expected=$(( $(find "$KIT_DIR/k3s" -type f -name '*.yaml' | wc -l) - $(find "$KIT_DIR/k3s/components/lan-discovery" -type f -name '*.yaml' | wc -l) ))
check "every template is rendered, without the lan-discovery component ($expected files)" bash -c "[ \$(find '$OUT' -type f -name '*.yaml' | wc -l) -eq $expected ]"
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
grep -v '^STORAGE_CLASS=' "$SCRATCH/svc/service.env" > "$SCRATCH/svc/unset.env"
check "an unset variable refuses the rendering" fails "$SCRIPTS/render_manifests.sh" "$SCRATCH/svc/unset.env" "$SCRATCH/r-unset"
check "and leaves no file with a placeholder" fails bash -c "grep -R -l '@STORAGE_CLASS@' '$SCRATCH/r-unset' 2> /dev/null | grep -q ."
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
check "discovery lan: every template is rendered" bash -c "[ \$(find '$LAN' -type f -name '*.yaml' | wc -l) -eq $(find "$KIT_DIR/k3s" -type f -name '*.yaml' | wc -l) ]"
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
    mv "$B" "$SCRATCH/build-moved"
    check "check_libs passes on the staged tree" "$S/bin/check_libs.sh" "$S" --program TestService
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

echo
echo "passed: $passed, failed: $failed"
[ "$failed" -eq 0 ]
