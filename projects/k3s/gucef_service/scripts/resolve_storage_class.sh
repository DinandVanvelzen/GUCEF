#!/bin/bash
# Copyright (C) 1998 - 2026.  Dinand Vanvelzen. Licensed under the Apache License, Version 2.0.
#
# Resolves the STORAGE_CLASS of a service file (a friendly name, auto, or a StorageClass name) to a StorageClass the
# target cluster has, and prints that name on standard output (the reasoning goes to standard error).
#
# usage: resolve_storage_class.sh [--classes-file FILE] <request>
#   request              auto | raid | mirror | jbod | local-path | <StorageClass name>
#   --classes-file FILE  the StorageClasses of the cluster, one per line, as "kubectl get storageclass -o name" prints
#                        them ("storageclass.storage.k8s.io/<name>") or as plain names; '#' comment lines and empty
#                        lines are skipped. "-" reads standard input. Without this option the list is asked from the
#                        cluster: $KUBECTL get storageclass -o name
#
# Friendly names and their candidates, the Longhorn-managed class first (it wins when both exist):
#   raid        longhorn-raid, raid
#   mirror      longhorn-mirror, mirror
#   jbod        longhorn-jbod, jbod
#   local-path  local-path
# (raid and jbod without the prefix are the local-path based classes of projects/k3s/host-node/storage-classes.yaml;
#  the longhorn-* classes those of projects/k3s/longhorn.)
# auto tries raid, mirror, jbod, local-path in this order and takes the first friendly name of which the cluster has a
# candidate. Any other request is a StorageClass name and must be in the list as it is.
# Nothing found: exit 1 with a message that lists the classes found and the candidates tried; there is no fallback to
# the default class or to anything else.
#
# KUBECTL (default "kubectl") is a command prefix, for example "ssh k3smaster kubectl". It is split on white space into
# words (no quotes, no escapes, no shell expansion, never eval'd) and run with "get storageclass -o name" appended. It is
# executed: set it yourself in your environment; no script takes it from a service file. The appended arguments hold no
# white space or shell characters, so a remote shell (ssh) passes them on unchanged.
#
# Exit code 0 with the class on standard output; 1 when nothing matches or the list cannot be had; 2 for a usage error.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=lib.sh
source "$SCRIPT_DIR/lib.sh"

usage() {
    echo "usage: resolve_storage_class.sh [--classes-file FILE|-] <auto|raid|mirror|jbod|local-path|<StorageClass name>>" >&2
    exit 2
}

CLASSES_FILE=""
REQUEST=""
while [ $# -gt 0 ]; do
    case "$1" in
        --classes-file) [ -n "${2:-}" ] || usage; CLASSES_FILE="$2"; shift ;;
        -h|--help) usage ;;
        -*) echo "resolve_storage_class: unknown option $1" >&2; usage ;;
        *) [ -z "$REQUEST" ] || usage; REQUEST="$1" ;;
    esac
    shift
done
[ -n "$REQUEST" ] || usage
gs_storage_class_request_valid "$REQUEST" || gs_fail "'$REQUEST' is neither auto, a friendly name ($GS_STORAGE_FRIENDLY_NAMES) nor a StorageClass name"

# the list of the cluster, as plain names
if [ -z "$CLASSES_FILE" ]; then
    read -r -a KUBECTL_COMMAND <<< "${KUBECTL:-kubectl}"
    [ ${#KUBECTL_COMMAND[@]} -gt 0 ] || gs_fail "KUBECTL is empty"
    RAW="$("${KUBECTL_COMMAND[@]}" get storageclass -o name)" || gs_fail "'${KUBECTL:-kubectl} get storageclass -o name' failed; check KUBECTL and the cluster"
elif [ "$CLASSES_FILE" = "-" ]; then
    RAW="$(cat)"
else
    [ -f "$CLASSES_FILE" ] || gs_fail "the classes file $CLASSES_FILE does not exist"
    RAW="$(cat "$CLASSES_FILE")"
fi
FOUND=()
while IFS= read -r line; do
    line="${line%$'\r'}"
    line="${line#"${line%%[![:space:]]*}"}"
    line="${line%"${line##*[![:space:]]}"}"
    case "$line" in
        ''|'#'*) continue ;;
    esac
    name="${line#storageclass.storage.k8s.io/}"
    name="${name#storageclass/}"
    [[ "$name" =~ ^[a-z0-9]([-a-z0-9.]{0,251}[a-z0-9])?$ ]] || gs_fail "the class list holds a line that is no StorageClass name: $line"
    FOUND+=( "$name" )
done <<< "$RAW"
FOUND_TEXT="${FOUND[*]:-}"
[ ${#FOUND[@]} -gt 0 ] || gs_fail "the cluster has no StorageClass at all; nothing to resolve '$REQUEST' to"

has_class() {
    local wanted="$1" name
    for name in "${FOUND[@]}"; do
        [ "$name" = "$wanted" ] && return 0
    done
    return 1
}

# the first candidate of a friendly name that the cluster has
resolve_friendly() {
    local candidate
    for candidate in $(gs_storage_class_candidates "$1"); do
        if has_class "$candidate"; then
            echo "$candidate"
            return 0
        fi
    done
    return 1
}

tried=""
case "$REQUEST" in
    auto)
        for friendly in $GS_STORAGE_AUTO_ORDER; do
            tried="$tried $friendly($(gs_storage_class_candidates "$friendly" | tr ' ' ','))"
            if class="$(resolve_friendly "$friendly")"; then
                echo "resolve_storage_class: auto -> $friendly -> $class" >&2
                echo "$class"
                exit 0
            fi
        done
        gs_fail "STORAGE_CLASS=auto: the cluster has none of the candidates tried in order:$tried; classes found: $FOUND_TEXT. Set STORAGE_CLASS to one of the classes found, or create a candidate class"
        ;;
    *)
        if gs_storage_class_is_friendly "$REQUEST"; then
            if class="$(resolve_friendly "$REQUEST")"; then
                echo "resolve_storage_class: $REQUEST -> $class" >&2
                echo "$class"
                exit 0
            fi
            gs_fail "STORAGE_CLASS=$REQUEST: the cluster has none of its candidates ($(gs_storage_class_candidates "$REQUEST")); classes found: $FOUND_TEXT"
        fi
        if has_class "$REQUEST"; then
            echo "resolve_storage_class: $REQUEST (a StorageClass name, used as it is)" >&2
            echo "$REQUEST"
            exit 0
        fi
        gs_fail "STORAGE_CLASS=$REQUEST: the cluster has no StorageClass of that name; classes found: $FOUND_TEXT"
        ;;
esac
