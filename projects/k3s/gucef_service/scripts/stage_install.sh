#!/bin/bash
# Copyright (C) 1998 - 2026.  Dinand Vanvelzen. Licensed under the Apache License, Version 2.0.
#
# Stages the /opt/<service> tree of an image from one or more GUCEF build directories and sets the RPATHs. Runs in the
# build stage of the image, where patchelf is installed.
#
# usage: stage_install.sh <destination> --program <name> [--bootstrap <file>] <build dir>...
#   destination        the tree to make, for example /out/opt/example-service; must not exist
#   --program name     the service executable, which must be in a bin/ of the build output
#   --bootstrap file   the bootstrap file of the image; it replaces config/<name>_bootstrap.ini of the build
#   build dir          a CMake build directory with bin/ and lib/ (one per target)
#
# Result:
#   bin/       programs (test applications and debug bootstrap files left out), check_libs.sh, links to config/ files
#   lib/       shared libraries
#   plugins/   plugin libraries (lib<kind>plugin<NAME>.so*, recognised by name); a plugin a library links stays in
#              plugins/ only, so the dynamic loader and the plugin loader load one object
#   data/      module data of the build directories
#   config/    bootstrap and default settings files (bin/ links to them: programs look next to themselves), site/ for the
#              ConfigMap mount; "Path"="$MODULEDIR$" of every bootstrap file points at ../plugins
# RPATH (a RUNPATH serves only the file's own direct dependencies, so each file names every directory of its own):
#   programs   $ORIGIN/../lib:$ORIGIN/../plugins
#   libraries  $ORIGIN:$ORIGIN/../plugins
#   plugins    $ORIGIN/../lib:$ORIGIN
# Environment: PATCHELF, the patchelf program (default patchelf; tests give a stand-in).
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=lib.sh
source "$SCRIPT_DIR/lib.sh"

usage() {
    echo "usage: stage_install.sh <destination> --program <name> [--bootstrap <file>] <build dir>..." >&2
    exit 2
}

DEST="${1:-}"
[ -n "$DEST" ] || usage
shift
PROGRAM=""
BOOTSTRAP=""
BUILDS=()
while [ $# -gt 0 ]; do
    case "$1" in
        --program) PROGRAM="${2:-}"; [ -n "$PROGRAM" ] || usage; shift ;;
        --bootstrap) BOOTSTRAP="${2:-}"; [ -n "$BOOTSTRAP" ] || usage; shift ;;
        -*) echo "stage_install: unknown option $1" >&2; usage ;;
        *) BUILDS+=( "$1" ) ;;
    esac
    shift
done
[ -n "$PROGRAM" ] || usage
[ ${#BUILDS[@]} -gt 0 ] || { echo "stage_install: no build directory given" >&2; exit 2; }
[ ! -e "$DEST" ] || { echo "stage_install: $DEST exists already" >&2; exit 2; }
[ -z "$BOOTSTRAP" ] || [ -f "$BOOTSTRAP" ] || gs_fail "the bootstrap file $BOOTSTRAP does not exist"
PATCHELF="${PATCHELF:-patchelf}"
command -v "$PATCHELF" > /dev/null || { echo "stage_install: $PATCHELF is not installed" >&2; exit 2; }

PLUGIN_PATTERN='^lib[A-Za-z0-9]*plugin[A-Za-z0-9_]+\.so(\..*)?$'

# plugins are also the libraries the bootstrap files name ("Filename" and "AltFilename"), whatever their name pattern
NAMED_PLUGINS=" "
for ini in "$BOOTSTRAP" "${BUILDS[@]/%//bin}"; do
    for file in $( [ -d "$ini" ] && ls "$ini"/*_bootstrap.ini 2> /dev/null; [ -f "$ini" ] && echo "$ini" ); do
        for name in $( awk -F'"' '$1 == "" && ( $2 == "Filename" || $2 == "AltFilename" ) { print $4 }' "$file" ); do
            NAMED_PLUGINS="$NAMED_PLUGINS$name "
        done
    done
done
is_plugin() {
    local stem="${1#lib}"
    stem="${stem%%.so*}"
    [[ "$1" =~ $PLUGIN_PATTERN ]] || [[ "$NAMED_PLUGINS" == *" $stem "* ]]
}

copy_new() {
    if [ ! -e "$2" ] && [ ! -L "$2" ]; then
        cp -a "$1" "$2"
    fi
}

mkdir -p "$DEST/bin" "$DEST/lib" "$DEST/plugins" "$DEST/data" "$DEST/config/site"

for build in "${BUILDS[@]}"; do
    [ -d "$build/bin" ] && [ -d "$build/lib" ] || gs_fail "$build has no bin/ and lib/"
    for entry in "$build"/bin/*; do
        [ -e "$entry" ] || [ -L "$entry" ] || continue
        name="$(basename "$entry")"
        case "$name" in
            *_TestApp*|*_bootstrap_d.ini|*.a) ;;
            *_bootstrap.ini|*_DefaultConfig.json) copy_new "$entry" "$DEST/config/$name" ;;
            *.so|*.so.*)
                if is_plugin "$name"; then copy_new "$entry" "$DEST/plugins/$name"; else copy_new "$entry" "$DEST/lib/$name"; fi ;;
            *) copy_new "$entry" "$DEST/bin/$name" ;;
        esac
    done
    for entry in "$build"/lib/*; do
        [ -e "$entry" ] || [ -L "$entry" ] || continue
        name="$(basename "$entry")"
        case "$name" in
            *.a) continue ;;
        esac
        if is_plugin "$name"; then
            copy_new "$entry" "$DEST/plugins/$name"
        else
            copy_new "$entry" "$DEST/lib/$name"
        fi
    done
    if [ -d "$build/data" ]; then
        cp -af "$build"/data/. "$DEST/data/"
    fi
done

[ -e "$DEST/bin/$PROGRAM" ] || gs_fail "bin/$PROGRAM is not in the build output of: ${BUILDS[*]}"

if [ -n "$BOOTSTRAP" ]; then
    rm -f "$DEST/config/${PROGRAM}_bootstrap.ini"
    cp "$BOOTSTRAP" "$DEST/config/${PROGRAM}_bootstrap.ini"
fi
for entry in "$DEST"/config/*_bootstrap.ini; do
    [ -f "$entry" ] || continue
    sed -i 's|^"Path"="\$MODULEDIR\$"|"Path"="$MODULEDIR$/../plugins"|' "$entry"
done
for entry in "$DEST"/config/*_bootstrap.ini "$DEST"/config/*_DefaultConfig.json; do
    [ -f "$entry" ] || continue
    ln -sf "../config/$(basename "$entry")" "$DEST/bin/$(basename "$entry")"
done

cp "$SCRIPT_DIR/check_libs.sh" "$SCRIPT_DIR/lib.sh" "$DEST/bin/"
chmod 0755 "$DEST/bin/check_libs.sh"

set_rpath() {
    local dir="$1" rpath="$2" file
    while IFS= read -r -d '' file; do
        if gs_is_elf "$file"; then
            "$PATCHELF" --set-rpath "$rpath" "$file"
        fi
    done < <(find "$dir" -type f -print0)
}
set_rpath "$DEST/bin" '$ORIGIN/../lib:$ORIGIN/../plugins'
set_rpath "$DEST/lib" '$ORIGIN:$ORIGIN/../plugins'
set_rpath "$DEST/plugins" '$ORIGIN/../lib:$ORIGIN'

# not writable by the service user; its data lives on /data
chmod -R go-w,a+rX "$DEST"
echo "stage_install: staged $DEST from ${BUILDS[*]}"
