#!/bin/bash
# Copyright (C) 1998 - 2026.  Dinand Vanvelzen. Licensed under the Apache License, Version 2.0.
#
# Exit of AssetStoreService with a plugin loaded: the program must unload its plugins itself before it returns, in
# every mode. Left to the exit handlers, a plugin is unloaded after the function local statics it made are destroyed
# (they were made after the globals of the framework, so they go first); the stores of the private plugins keep their
# identifier in such a static, so their unload found nothing to remove, the providers stayed in the registry and were
# released after their code was unloaded: the admin mode died with SIGSEGV at exit.
#
# The plugin used is gucefCORE_TestPlugin, which with GUCEF_TESTPLUGIN_UNLOAD_NEEDS_STATIC=1 makes a static of that kind
# when it loads and exits with code 97 when it is unloaded after the static is gone. Not run by the kit tests of
# projects/k3s/gucef_service: it needs a debug build of the program and of the test plugin, and starts the service.
#
# usage: test_exit_with_plugins.sh <build directory of AssetStoreService> <directory with libgucefCORE_TestPlugin_d.so>
#   the build directory holds bin/AssetStoreService and lib/ (common/bin/CBNINJA_AssetStoreService_svc)
# Exit code 0 when every check passes, 1 otherwise. The scratch directory is removed; every process started is stopped.
set -u

BUILD="${1:?usage: test_exit_with_plugins.sh <build directory> <test plugin directory>}"
PLUGIN_DIR="${2:?usage: test_exit_with_plugins.sh <build directory> <test plugin directory>}"
PROGRAM="$BUILD/bin/AssetStoreService"
[ -x "$PROGRAM" ] || { echo "no program at $PROGRAM"; exit 1; }
[ -f "$PLUGIN_DIR/libgucefCORE_TestPlugin_d.so" ] || { echo "no libgucefCORE_TestPlugin_d.so in $PLUGIN_DIR"; exit 1; }

SCRATCH="$(mktemp -d "${TMPDIR:-/tmp}/assetstore-exit-test.XXXXXX")"
SERVICE_PID=""
cleanup() {
    [ -z "$SERVICE_PID" ] || kill "$SERVICE_PID" 2> /dev/null
    [ -z "$SERVICE_PID" ] || wait "$SERVICE_PID" 2> /dev/null
    rm -rf "$SCRATCH"
}
trap cleanup EXIT

export GUCEF_CONSOLE_LOGGER=none
export LD_LIBRARY_PATH="$BUILD/lib:$BUILD/bin"

# three free ports
free_port() {
    local port
    for port in $(shuf -i 47000-48999 -n 50); do
        ss -ltn | grep -q ":$port " || { echo "$port"; return 0; }
    done
    return 1
}
HEALTH="$(free_port)"; ADMIN="$(free_port)"; SESSION="$(free_port)"; PAIRING="$(free_port)"

DATA="$SCRATCH/data"
mkdir -p "$DATA"
cat > "$SCRATCH/site.json" <<EOF
{
    "Service": { "displayName": "Exit test", "dataDir": "$DATA", "drainSeconds": 5, "healthPort": $HEALTH, "healthBindAddress": "127.0.0.1", "adminPort": $ADMIN },
    "RemoteHost": { "serviceName": "gucef.assetstore.exittest", "drnSessionName": "assetstore-exittest", "sessionPort": $SESSION, "pairingPort": $PAIRING,
                    "sessionEndpoints": [ "tcp://127.0.0.1:$SESSION" ], "pairingEndpoints": [ "tcp://127.0.0.1:$PAIRING" ], "lanDiscovery": false },
    "AssetStoreHost": { "roles": "owner" },
    "Database": { "type": "vfstables", "directory": "$DATA/catalogue" },
    "Storage": { "locations": [ { "id": "main", "name": "Main", "path": "$DATA/library/main", "vfsRoot": "assetstore/main" } ], "defaultLocationId": "main" },
    "Credentials": { "directory": "$DATA/credentials" },
    "ConverterHook": { "threads": 1 }
}
EOF

# the bootstrap file of the build, its plugins from the lib directory, and the test plugin
BOOTSTRAP="$SCRATCH/bootstrap.ini"
sed "s#\$MODULEDIR\$#$BUILD/lib#" "$BUILD/bin/AssetStoreService_bootstrap_d.ini" > "$BOOTSTRAP"
cat >> "$BOOTSTRAP" <<EOF

[Bootstrap\PluginGroup\PluginMetaData]
"Description"="the test plugin of gucefCORE"
"Path"="$PLUGIN_DIR"
"Filename"="gucefCORE_TestPlugin_d"
"Type"="GucefGenericPlugin"
"LoadFailAllowed"="false"
EOF

passed=0
failed=0
expect_exit() {
    local what="$1" want="$2"
    shift 2
    "$@" > "$SCRATCH/out.txt" 2> "$SCRATCH/err.txt"
    local got=$?
    if [ "$got" -eq "$want" ]; then
        passed=$((passed + 1)); echo "  ok    $what"
    else
        failed=$((failed + 1)); echo "  FAIL  $what: exit code $got, expected $want"; head -c 400 "$SCRATCH/err.txt"
    fi
}

echo "AssetStoreService exit with a plugin whose unload needs its static"
"$PROGRAM" --console --config "$SCRATCH/site.json" --bootstrap "$BOOTSTRAP" > "$SCRATCH/service.out" 2>&1 &
SERVICE_PID=$!
ready=0
for attempt in $(seq 1 60); do
    if "$PROGRAM" --config "$SCRATCH/site.json" --bootstrap "$BOOTSTRAP" --admin status > /dev/null 2>&1; then ready=1; break; fi
    kill -0 "$SERVICE_PID" 2> /dev/null || break
    sleep 1
done
if [ "$ready" -ne 1 ]; then echo "  FAIL  the service did not answer its admin endpoint"; exit 1; fi

export GUCEF_TESTPLUGIN_UNLOAD_NEEDS_STATIC=1
expect_exit "--admin status exits with 0" 0 "$PROGRAM" --config "$SCRATCH/site.json" --bootstrap "$BOOTSTRAP" --admin status
expect_exit "--admin devices exits with 0" 0 "$PROGRAM" --config "$SCRATCH/site.json" --bootstrap "$BOOTSTRAP" --admin devices
expect_exit "a settings file that is refused exits with 3" 3 "$PROGRAM" --config "$SCRATCH/nonexistent.json" --bootstrap "$BOOTSTRAP"
expect_exit "--help exits with 0" 0 "$PROGRAM" --help
unset GUCEF_TESTPLUGIN_UNLOAD_NEEDS_STATIC

# the service itself: drained by the admin command, exits in order
"$PROGRAM" --config "$SCRATCH/site.json" --bootstrap "$BOOTSTRAP" --admin drain > /dev/null 2>&1
wait "$SERVICE_PID"
service_exit=$?
SERVICE_PID=""
if [ "$service_exit" -eq 0 ]; then passed=$((passed + 1)); echo "  ok    the service drained by the admin command exits with 0"
else failed=$((failed + 1)); echo "  FAIL  the service exited with $service_exit"; fi

echo
echo "passed: $passed, failed: $failed"
[ "$failed" -eq 0 ]
