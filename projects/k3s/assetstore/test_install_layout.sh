#!/bin/bash
# Copyright (C) 1998 - 2026.  Dinand Vanvelzen. Licensed under the Apache License, Version 2.0.
#
# Starts AssetStoreService from the install layout of the image, as stage_install.sh makes it from a build directory
# with the bootstrap file of the image: the program in bin/, the plugins in plugins/ only (no links in bin/ or lib/),
# config/AssetStoreService_bootstrap.ini with every "Path" at ../plugins. The program has to find its plugins through
# that bootstrap file and reach READY with a vfstables catalogue; without one required plugin it has to exit with
# code 3. Runs in a network namespace of its own (unshare -rn, loopback and a veth pair), so the ports of the program
# are free and nothing leaves the machine. No docker, no cluster, nothing downloaded.
#
# usage: test_install_layout.sh <build dir> <scratch dir>
#   build dir     a CMake build directory of AssetStoreService with bin/ and lib/ (a debug build is fine: the bootstrap
#                 file of the image names the debug files as "AltFilename")
#   scratch dir   made (or emptied) by the test: the staged tree, the settings, the data and the logs
#
# Without patchelf (PATCHELF) a stand-in leaves the RPATHs of the build: the shared libraries then still come from the
# build directory, but every plugin has to come from plugins/ of the staged tree, which the test checks in the memory
# map of the process. Needs bash, unshare (user namespaces), ip, curl and the coreutils.
set -uo pipefail

KIT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
STAGE="$KIT_DIR/../gucef_service/scripts/stage_install.sh"

if [ "${1:-}" = "--in-namespace" ]; then
    # the part that runs inside the namespace: <tree> <settings> <work> <mode>
    TREE="$2"; SETTINGS="$3"; WORK="$4"; MODE="$5"
    ip link set lo up || exit 90
    ip link add v0 type veth peer name v1 && ip addr add 10.77.0.1/24 brd + dev v0 && ip addr add 10.77.0.2/24 brd + dev v1 || exit 91
    ip link set v0 up && ip link set v1 up || exit 92
    ip route add 224.0.0.0/4 dev v0 2> /dev/null
    if [ "$MODE" = "missing" ]; then
        GUCEF_CONSOLE_LOGGER=stderr timeout 60 "$TREE/bin/AssetStoreService" --config "$SETTINGS" --smoke-test > "$WORK/missing.log" 2>&1
        echo "exit $?"
        exit 0
    fi
    GUCEF_CONSOLE_LOGGER=stderr "$TREE/bin/AssetStoreService" --config "$SETTINGS" > "$WORK/service.log" 2>&1 &
    PID=$!
    ready=""
    for i in $(seq 1 150); do
        if [ "$(curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:47480/health/ready 2> /dev/null)" = "200" ]; then
            ready="after $i polls"
            break
        fi
        kill -0 "$PID" 2> /dev/null || break
        sleep 0.2
    done
    echo "ready ${ready:-never}"
    grep -o '/[^ ]*plugin[^ ]*\.so[^ ]*' "/proc/$PID/maps" 2> /dev/null | sort -u > "$WORK/plugins_mapped.txt"
    kill -TERM "$PID" 2> /dev/null
    wait "$PID"
    echo "exit $?"
    exit 0
fi

BUILD="${1:-}"; SCRATCH="${2:-}"
[ -n "$BUILD" ] && [ -n "$SCRATCH" ] || { echo "usage: test_install_layout.sh <build dir> <scratch dir>" >&2; exit 2; }
BUILD="$(cd "$BUILD" && pwd)" || exit 2
[ -x "$BUILD/bin/AssetStoreService" ] || { echo "test_install_layout: $BUILD/bin/AssetStoreService is not there" >&2; exit 2; }
rm -rf "$SCRATCH"
mkdir -p "$SCRATCH" || exit 2
SCRATCH="$(cd "$SCRATCH" && pwd)"
TREE="$SCRATCH/opt/assetstore"

passed=0
failed=0
check() {
    local what="$1"
    shift
    if "$@"; then
        passed=$((passed + 1)); echo "ok    $what"
    else
        failed=$((failed + 1)); echo "FAIL  $what"
    fi
}

# the tree of the image
if ! command -v "${PATCHELF:-patchelf}" > /dev/null; then
    printf '#!/bin/sh\nexit 0\n' > "$SCRATCH/patchelf"
    chmod 0755 "$SCRATCH/patchelf"
    export PATCHELF="$SCRATCH/patchelf"
    echo "note  no patchelf: the shared libraries keep the RPATHs of the build"
fi
check "stage_install.sh stages the tree with the bootstrap file of the image" \
    bash "$STAGE" "$TREE" --program AssetStoreService --bootstrap "$KIT_DIR/AssetStoreService_bootstrap_container.ini" "$BUILD"
check "the program's bootstrap file is the image's, with every Path at ../plugins" \
    bash -c "grep -q 'projects/k3s/assetstore' '$TREE/config/AssetStoreService_bootstrap.ini' && ! grep '^\"Path\"' '$TREE/config/AssetStoreService_bootstrap.ini' | grep -v -q '/../plugins\"'"
check "there is no debug bootstrap file in the tree" bash -c "! ls '$TREE'/bin/*_bootstrap_d.ini '$TREE'/config/*_bootstrap_d.ini 2> /dev/null | grep -q ."
check "the plugins are in plugins/ only: no plugin and no link to one in bin/ or lib/" \
    bash -c "! find '$TREE/bin' '$TREE/lib' -name '*plugin*' | grep -q . && ! find '$TREE/bin' '$TREE/lib' -type l -lname '*plugins/*' | grep -q . && ls '$TREE/plugins' | grep -q dstorepluginJSONPARSER"

# the settings: a vfstables catalogue and a library under the scratch folder
cat > "$SCRATCH/AssetStoreService.json" <<EOF
{
    "Service": { "displayName": "Asset store (install layout test)", "dataDir": "$SCRATCH/data", "healthBindAddress": "127.0.0.1", "drainSeconds": 5, "expectedBackSeconds": 5 },
    "RemoteHost": { "sessionEndpoints": [ "tcp://10.77.0.1:47400" ], "pairingEndpoints": [ "tcp://10.77.0.1:47401" ] },
    "Database": { "type": "vfstables", "directory": "$SCRATCH/catalogue" },
    "Storage": { "locations": [ { "id": "main", "name": "Main", "path": "$SCRATCH/library/main", "vfsRoot": "assetstore/main" } ] }
}
EOF
mkdir -p "$SCRATCH/catalogue" "$SCRATCH/library/main"

result="$(env -u DISPLAY unshare -rn bash "$KIT_DIR/test_install_layout.sh" --in-namespace "$TREE" "$SCRATCH/AssetStoreService.json" "$SCRATCH" ready)"
echo "$result" | sed 's/^/      /'
check "the program reaches READY from the tree" bash -c "echo '$result' | grep -q '^ready after'"
check "it stops in order on SIGTERM (exit code 0)" bash -c "echo '$result' | grep -q '^exit 0$'"
check "it loaded its plugins through the bootstrap file of the tree" grep -q "the plugins of $TREE/bin/AssetStoreService_bootstrap.ini are loaded" "$SCRATCH/service.log"
check "every plugin it mapped is one of plugins/ of the tree" \
    bash -c "[ -s '$SCRATCH/plugins_mapped.txt' ] && ! grep -v '^$TREE/plugins/' '$SCRATCH/plugins_mapped.txt' | grep -q . && [ \$(wc -l < '$SCRATCH/plugins_mapped.txt') -ge 9 ]"

# without a required plugin the program ends with exit code 3
mkdir -p "$SCRATCH/removed"
mv "$TREE"/plugins/libcodecspluginZLIB* "$SCRATCH/removed/"
result="$(env -u DISPLAY unshare -rn bash "$KIT_DIR/test_install_layout.sh" --in-namespace "$TREE" "$SCRATCH/AssetStoreService.json" "$SCRATCH" missing)"
check "without a required plugin it exits with code 3" bash -c "echo '$result' | grep -q '^exit 3$'"
check "and says that the bootstrap file failed" grep -q "a plugin it requires did not load" "$SCRATCH/missing.log"

echo
echo "passed: $passed, failed: $failed"
[ "$failed" -eq 0 ]
