#!/bin/bash
# Copyright (C) 1998 - 2026.  Dinand Vanvelzen. Licensed under the Apache License, Version 2.0.
#
# Makes the docker build context of a GUCEF service image from checkouts: their tracked files and the untracked files
# git does not ignore (so work that is not committed yet is in the image), without files deleted in the working tree,
# build directories or ignored files.
#
# usage: make_context.sh <context directory> [<name>=<checkout path>]...
#   context directory   created; must not exist
#   name=path           a further checkout, copied to src/<name> (a name may hold '/': VanvelzenSoftware/product)
#
# Environment: GUCEF_HOME, the GUCEF checkout (default: the checkout this script is in), copied to src/GUCEF.
# Prints "dirty" on the last line when a checkout has uncommitted changes, "clean" otherwise.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=lib.sh
source "$SCRIPT_DIR/lib.sh"

GUCEF_HOME="${GUCEF_HOME:-$SCRIPT_DIR/../../../..}"
GUCEF_HOME="$(cd "$GUCEF_HOME" && pwd)"
CONTEXT="${1:-}"
[ -n "$CONTEXT" ] || { echo "usage: make_context.sh <context directory> [<name>=<checkout path>]..." >&2; exit 2; }
shift
[ ! -e "$CONTEXT" ] || { echo "make_context: $CONTEXT exists already" >&2; exit 2; }
command -v git > /dev/null || gs_fail "git is not installed"
command -v rsync > /dev/null || gs_fail "rsync is not installed"

state="clean"

copy_checkout() {
    local source="$1" destination="$2" list
    git -C "$source" rev-parse --is-inside-work-tree > /dev/null 2>&1 || gs_fail "$source is not a git checkout"
    mkdir -p "$destination"
    list="$(mktemp)"
    git -C "$source" ls-files -z --cached --others --exclude-standard --deduplicate > "$list"
    git -C "$source" ls-files -z --deleted | tr '\0' '\n' > "$list.deleted"
    if [ -s "$list.deleted" ]; then
        grep -z -v -x -F -f "$list.deleted" "$list" > "$list.kept" || true
        mv "$list.kept" "$list"
    fi
    rsync -a --from0 --files-from="$list" "$source/" "$destination/"
    rm -f "$list" "$list.deleted"
    if [ -n "$(git -C "$source" status --porcelain)" ]; then
        state="dirty"
    fi
}

mkdir -p "$CONTEXT/src"
copy_checkout "$GUCEF_HOME" "$CONTEXT/src/GUCEF"
for extra in "$@"; do
    [[ "$extra" == *=* ]] || gs_fail "'$extra' is not <name>=<checkout path>"
    name="${extra%%=*}"
    path="${extra#*=}"
    [[ "$name" =~ ^[A-Za-z0-9][A-Za-z0-9._/-]*$ ]] && [[ "$name" != *..* ]] || gs_fail "the name '$name' is not usable as a directory below src/"
    [ "$name" != "GUCEF" ] || gs_fail "the name GUCEF is the GUCEF checkout itself"
    copy_checkout "$(cd "$path" && pwd)" "$CONTEXT/src/$name"
done
echo "make_context: $CONTEXT"
echo "$state"
