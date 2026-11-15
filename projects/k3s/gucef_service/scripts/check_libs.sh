#!/bin/bash
# Copyright (C) 1998 - 2026.  Dinand Vanvelzen. Licensed under the Apache License, Version 2.0.
#
# Checks with ldd that every ELF file below <root> (the programs, libraries and plugins of an image) finds its libraries
# inside <root> or in the base image, that every plugin the bootstrap files name is present, and that the programs exist.
#
# usage: check_libs.sh [root] [--program <name>]...
#   root             default: the directory above this script's bin/
#   --program name   a program that must be in bin/ with a bootstrap file config/<name>_bootstrap.ini
#
# ldd runs without LD_LIBRARY_PATH and LD_PRELOAD, so the RPATH of every file is what is tested. A library counts as
# found below <root>, /lib, /lib64, /usr/lib or /usr/lib64; one from any other directory (a build path left in an RPATH)
# is reported. A plugin of a bootstrap file is required when it says "LoadFailAllowed"="false" (missing: a failure),
# optional otherwise (missing: a note). Commented entries (';') are not read.
# Prints the libraries taken from the base image (what the runtime packages must provide).
# Exit code 0 when everything resolves, 1 with the problems otherwise, 2 for a usage error.
set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_ARG=""
PROGRAMS=()
while [ $# -gt 0 ]; do
    case "$1" in
        --program) [ -n "${2:-}" ] || { echo "check_libs: --program needs a name" >&2; exit 2; }; PROGRAMS+=( "$2" ); shift ;;
        -*) echo "check_libs: unknown option $1" >&2; exit 2 ;;
        *) [ -z "$ROOT_ARG" ] || { echo "check_libs: more than one root given" >&2; exit 2; }; ROOT_ARG="$1" ;;
    esac
    shift
done
ROOT_ARG="${ROOT_ARG:-$SCRIPT_DIR/..}"
[ -d "$ROOT_ARG" ] || { echo "check_libs: $ROOT_ARG is not a directory" >&2; exit 2; }
ROOT="$(cd "$ROOT_ARG" && pwd -P)"
unset LD_LIBRARY_PATH LD_PRELOAD

problems=""
external=""
checked=0

is_elf() {
    [ "$(head -c 4 "$1" 2> /dev/null | od -An -tx1 | tr -d ' \n')" = "7f454c46" ]
}

while IFS= read -r -d '' file; do
    is_elf "$file" || continue
    checked=$((checked + 1))
    output="$(ldd "$file" 2>&1)"
    while IFS= read -r line; do
        line="${line#"${line%%[![:space:]]*}"}"
        case "$line" in
            *"not found"*) problems="${problems}unresolved: ${file#"$ROOT"/}: ${line%% =>*}"$'\n'; continue ;;
            *"=> "*) path="${line#*=> }" ;;
            /*) path="$line" ;;
            *) continue ;;
        esac
        path="${path%% (*}"
        case "$path" in
            "$ROOT"/*) ;;
            /lib/*|/lib64/*|/usr/lib/*|/usr/lib64/*) external="${external}${path}"$'\n' ;;
            /*) problems="${problems}outside the image: ${file#"$ROOT"/}: ${path}"$'\n' ;;
        esac
    done <<< "$output"
done < <(find "$ROOT" -type f -print0)

plugin_count=0
check_bootstrap() {
    local bootstrap="$1" name alternative allowed
    while IFS='|' read -r name alternative allowed; do
        [ -n "$name" ] || continue
        plugin_count=$((plugin_count + 1))
        if ls "$ROOT"/plugins/lib"$name".so* > /dev/null 2>&1; then
            continue
        fi
        if [ -n "$alternative" ] && ls "$ROOT"/plugins/lib"$alternative".so* > /dev/null 2>&1; then
            continue
        fi
        if [ "$allowed" = "false" ]; then
            problems="${problems}required plugin missing: ${name} of ${bootstrap#"$ROOT"/} (plugins/lib${name}.so)"$'\n'
        else
            echo "check_libs: optional plugin of ${bootstrap#"$ROOT"/} not in the image: $name" >&2
        fi
    done < <(awk -F'"' '
        /^\[/ { if ( name != "" ) { print name "|" alt "|" allowed }; name = ""; alt = ""; allowed = "true"; next }
        $1 != "" { next }
        $2 == "Filename" { name = $4 }
        $2 == "AltFilename" { alt = $4 }
        $2 == "LoadFailAllowed" { allowed = $4 }
        END { if ( name != "" ) { print name "|" alt "|" allowed } }
    ' "$bootstrap")
}
for bootstrap in "$ROOT"/config/*_bootstrap.ini; do
    [ -f "$bootstrap" ] || continue
    check_bootstrap "$bootstrap"
done
for program in "${PROGRAMS[@]}"; do
    [ -x "$ROOT/bin/$program" ] || problems="${problems}program missing: bin/$program"$'\n'
    [ -f "$ROOT/config/${program}_bootstrap.ini" ] || problems="${problems}bootstrap file missing: config/${program}_bootstrap.ini"$'\n'
done

echo "check_libs: $checked ELF files and $plugin_count bootstrap plugins checked below $ROOT"
echo "check_libs: libraries taken from the base image:"
printf '%s' "$external" | sort -u | sed 's/^/    /'
if [ -n "$problems" ]; then
    echo "check_libs: FAILED" >&2
    printf '%s' "$problems" | sort -u | sed 's/^/    /' >&2
    exit 1
fi
echo "check_libs: ok"
