#!/bin/bash
# Copyright (C) 1998 - 2026.  Dinand Vanvelzen. Licensed under the Apache License, Version 2.0.
#
# Stages the /opt/<service> tree of an image from one or more GUCEF build directories and sets the RPATHs. Runs in the
# build stage of the image, where patchelf is installed.
#
# usage: stage_install.sh <destination> --program <name> [--bootstrap <file>] [--plugin-links <dirs>] [--extra-program <name>=<source dir>]... <build dir>...
#   destination        the tree to make, for example /out/opt/example-service; must not exist
#   --program name     the service executable, which must be in a bin/ of the build output
#   --bootstrap file   the bootstrap file of the image; it replaces config/<name>_bootstrap.ini of the build
#   --plugin-links dirs  space separated, of bin and lib: a symbolic link there to every plugin of plugins/, for a
#                      program that loads its plugins as $MODULEDIR$/<name> (its own folder bin/, then lib/ next to it)
#                      instead of through a bootstrap file with "Path"; the loader opens the one file of plugins/
#                      through the link (glibc knows a loaded object by its file, so it is never loaded twice)
#   --extra-program name=dir  repeatable: a further program of the image, for example an external tool the service starts
#                      as a child process. bin/<name> must be in the build output (it is staged like any program); the
#                      licence and readme files (COPYING*, LICENSE*, LICENCE*, NOTICE*, README*) of the source directory
#                      <dir> are copied to licences/<last part of dir>/, and none there is an error (the licence of the
#                      tool has to ship with it)
#   build dir          a CMake build directory with bin/ and lib/ (one per target)
#
# Result:
#   bin/       programs (test applications and debug bootstrap files left out), check_libs.sh, links to config/ files
#   lib/       shared libraries
#   plugins/   plugin libraries (lib<kind>plugin<NAME>.so*, recognised by name); a plugin a library links stays in
#              plugins/ only, so the dynamic loader and the plugin loader load one object; with --plugin-links, bin/
#              or lib/ also hold a symbolic link to each
#   data/      module data of the build directories
#   licences/  with --extra-program: the licence files of each extra program, one folder per source directory
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
    echo "usage: stage_install.sh <destination> --program <name> [--bootstrap <file>] [--plugin-links <dirs>] [--extra-program <name>=<source dir>]... <build dir>..." >&2
    exit 2
}

DEST="${1:-}"
[ -n "$DEST" ] || usage
shift
PROGRAM=""
BOOTSTRAP=""
PLUGIN_LINKS=""
EXTRA_PROGRAMS=()
BUILDS=()
while [ $# -gt 0 ]; do
    case "$1" in
        --program) PROGRAM="${2:-}"; [ -n "$PROGRAM" ] || usage; shift ;;
        --bootstrap) BOOTSTRAP="${2:-}"; [ -n "$BOOTSTRAP" ] || usage; shift ;;
        --plugin-links) PLUGIN_LINKS="${2-}"; [ $# -ge 2 ] || usage; shift ;;
        --extra-program) [[ "${2:-}" == ?*=?* ]] || usage; EXTRA_PROGRAMS+=( "$2" ); shift ;;
        -*) echo "stage_install: unknown option $1" >&2; usage ;;
        *) BUILDS+=( "$1" ) ;;
    esac
    shift
done
[ -n "$PROGRAM" ] || usage
[ ${#BUILDS[@]} -gt 0 ] || { echo "stage_install: no build directory given" >&2; exit 2; }
[ ! -e "$DEST" ] || { echo "stage_install: $DEST exists already" >&2; exit 2; }
[ -z "$BOOTSTRAP" ] || [ -f "$BOOTSTRAP" ] || gs_fail "the bootstrap file $BOOTSTRAP does not exist"
for dir in $PLUGIN_LINKS; do
    case "$dir" in
        bin|lib) ;;
        *) echo "stage_install: --plugin-links takes bin and lib, not '$dir'" >&2; exit 2 ;;
    esac
done
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

# external tools the service starts as child processes: staged with the programs above, with their licence files
for extra in "${EXTRA_PROGRAMS[@]+"${EXTRA_PROGRAMS[@]}"}"; do
    extra_name="${extra%%=*}"
    extra_dir="${extra#*=}"
    [ -x "$DEST/bin/$extra_name" ] || gs_fail "the extra program bin/$extra_name is not in the build output of: ${BUILDS[*]}"
    [ -d "$extra_dir" ] || gs_fail "the source directory $extra_dir of the extra program $extra_name does not exist"
    licence_dir="$DEST/licences/$(basename "$extra_dir")"
    mkdir -p "$licence_dir"
    licence_count=0
    for licence in "$extra_dir"/COPYING* "$extra_dir"/LICENSE* "$extra_dir"/LICENCE* "$extra_dir"/NOTICE* "$extra_dir"/README*; do
        [ -f "$licence" ] || continue
        cp "$licence" "$licence_dir/"
        licence_count=$((licence_count + 1))
    done
    [ "$licence_count" -gt 0 ] || gs_fail "the extra program $extra_name has no licence file (COPYING*, LICENSE*, ...) in $extra_dir"
done

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

# links to the plugins for a program that loads them from its own folder (--plugin-links)
for dir in $PLUGIN_LINKS; do
    for entry in "$DEST"/plugins/*; do
        [ -e "$entry" ] || [ -L "$entry" ] || continue
        name="$(basename "$entry")"
        [ ! -e "$DEST/$dir/$name" ] && [ ! -L "$DEST/$dir/$name" ] || gs_fail "$dir/$name exists already; no link to plugins/$name is made"
        ln -s "../plugins/$name" "$DEST/$dir/$name"
    done
done

# not writable by the service user; its data lives on /data
chmod -R go-w,a+rX "$DEST"
echo "stage_install: staged $DEST from ${BUILDS[*]}"
