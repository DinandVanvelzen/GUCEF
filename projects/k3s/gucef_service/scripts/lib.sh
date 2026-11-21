#!/bin/bash
# Copyright (C) 1998 - 2026.  Dinand Vanvelzen. Licensed under the Apache License, Version 2.0.
#
# Shared functions of the gucef_service scripts; sourced, not run.
#
#   gs_fail <message>             prints "<script>: <message>" to stderr and exits 1
#   gs_load_env <file>            reads a service.env (KEY=value lines, '#' comments) and exports every key; refuses a
#                                 line that is not KEY=value or a key that is not upper case letters, digits and '_'
#   gs_require <KEY>...           fails for every key that is unset or empty
#   gs_render <template> <out>    replaces @KEY@ with the value of the exported variable KEY and a line that holds only
#                                 @FILE:KEY@ (after indentation) with the lines of the file KEY names, each indented
#                                 like the marker; fails for a placeholder whose variable is not set (an empty value
#                                 is allowed only for keys listed in GS_EMPTY_ALLOWED)
#   gs_load_service <file> <dir> <allow>  the service file with the local install configuration of the site
#   gs_yaml_quote <value>         a value as a YAML double-quoted scalar
#   gs_storage_class_*            the friendly names of STORAGE_CLASS, their candidate classes and the auto order
#                                 (resolve_storage_class.sh)

GS_SCRIPT_NAME="$(basename "${0}")"
GS_SCRIPT_NAME="${GS_SCRIPT_NAME%.sh}"

gs_fail() {
    echo "${GS_SCRIPT_NAME}: $*" >&2
    exit 1
}

gs_load_env() {
    local file="$1" line key value number=0
    [ -f "$file" ] || gs_fail "the service file $file does not exist"
    while IFS= read -r line || [ -n "$line" ]; do
        number=$((number + 1))
        line="${line%$'\r'}"
        case "$line" in
            ''|'#'*) continue ;;
        esac
        if ! [[ "$line" =~ ^([A-Z][A-Z0-9_]*)=(.*)$ ]]; then
            gs_fail "$file:$number: not a KEY=value line: $line"
        fi
        key="${BASH_REMATCH[1]}"
        value="${BASH_REMATCH[2]}"
        # one pair of surrounding quotes is taken off; nothing is expanded
        if [[ "$value" =~ ^\"(.*)\"$ ]] || [[ "$value" =~ ^\'(.*)\'$ ]]; then
            value="${BASH_REMATCH[1]}"
        fi
        export "$key=$value"
    done < "$file"
    GS_ENV_DIR="$(cd "$(dirname "$file")" && pwd)"
    export GS_ENV_DIR
}

gs_require() {
    local key missing=""
    for key in "$@"; do
        if [ -z "${!key:-}" ]; then
            missing="$missing $key"
        fi
    done
    [ -z "$missing" ] || gs_fail "not set in the service file:$missing"
}

# a path of the service file: absolute as it is, relative to the directory of the service file otherwise
gs_env_path() {
    case "$1" in
        /*) echo "$1" ;;
        *) echo "${GS_ENV_DIR:-.}/$1" ;;
    esac
}

gs_render() {
    local template="$1" out="$2"
    [ -f "$template" ] || gs_fail "template $template does not exist"
    if ! awk -v emptyAllowed=" ${GS_EMPTY_ALLOWED:-} " -v envDir="${GS_ENV_DIR:-.}" '
        function fail( message ) {
            printf "%s:%d: %s\n", FILENAME, FNR, message > "/dev/stderr"
            failed = 1
        }
        {
            line = $0
            if ( match( line, /^[ ]*@FILE:[A-Z][A-Z0-9_]*@[ ]*$/ ) ) {
                indent = line
                sub( /@.*$/, "", indent )
                key = line
                sub( /^[ ]*@FILE:/, "", key )
                sub( /@[ ]*$/, "", key )
                if ( !( key in ENVIRON ) || ENVIRON[ key ] == "" ) { fail( "@FILE:" key "@: " key " is not set" ); next }
                path = ENVIRON[ key ]
                if ( substr( path, 1, 1 ) != "/" ) path = envDir "/" path
                count = 0
                while ( ( getline content < path ) > 0 ) { print indent content; ++count }
                close( path )
                if ( count == 0 ) fail( "@FILE:" key "@: " path " is missing or empty" )
                next
            }
            result = ""
            while ( match( line, /@[A-Z][A-Z0-9_]*@/ ) ) {
                key = substr( line, RSTART + 1, RLENGTH - 2 )
                if ( !( key in ENVIRON ) ) {
                    fail( "@" key "@: " key " is not set" )
                    value = ""
                } else {
                    value = ENVIRON[ key ]
                    if ( value == "" && index( emptyAllowed, " " key " " ) == 0 ) fail( "@" key "@: " key " is empty" )
                }
                result = result substr( line, 1, RSTART - 1 ) value
                line = substr( line, RSTART + RLENGTH )
            }
            print result line
        }
        END { exit failed }
    ' "$template" > "$out"; then
        rm -f "$out"
        gs_fail "rendering $template failed"
    fi
}

# ---- local install configuration (render_manifests.sh, deploy.sh) ----------------------------------------------------
# gs_load_service <service.env> <local dir or ""> <allow the local dir in git: true|false>
# Reads the service file, then the local install configuration of the site, which is kept out of every repository:
#   <local dir>/service.env.local   KEY=value lines that override the service file (a relative SITE_CONFIG_SOURCE,
#                                   BOOTSTRAP_INI or DOCKERFILE in it is relative to the local directory)
#   <local dir>/site.json           replaces the site settings file (SITE_CONFIG_SOURCE) as a whole
#   <local dir>/patches/*.yaml, <local dir>/resources/*.yaml   used by render_manifests.sh
# Without a local dir, a service.env.local next to the service file makes that directory the local dir. A local dir
# inside a git work tree is refused (the local configuration names hosts, paths and shares of the site) unless allowed.
# Sets GS_LOCAL_DIR (empty without one); GS_ENV_DIR stays the directory of the service file.
gs_load_service() {
    local env_file="$1" local_dir="$2" allow="$3" env_dir top key value
    gs_load_env "$env_file"
    env_dir="$GS_ENV_DIR"
    if [ -z "$local_dir" ] && [ -f "$env_dir/service.env.local" ]; then
        local_dir="$env_dir"
    fi
    GS_LOCAL_DIR=""
    if [ -n "$local_dir" ]; then
        [ -d "$local_dir" ] || gs_fail "the local directory $local_dir does not exist"
        local_dir="$(cd "$local_dir" && pwd -P)"
        top="$(cd "$local_dir" && env -u GIT_DIR -u GIT_WORK_TREE git rev-parse --show-toplevel 2> /dev/null || true)"
        if [ -n "$top" ]; then
            [ "$allow" = "true" ] || gs_fail "the local install configuration $local_dir lies inside the git work tree $top. It holds the values of one site (hosts, node paths, shares, overrides) and is kept out of every repository by convention: move it to a directory outside any repository and pass --local-dir, or pass --allow-local-in-git to use it there anyway"
            echo "$GS_SCRIPT_NAME: note: the local install configuration $local_dir lies inside the git work tree $top (--allow-local-in-git)" >&2
        fi
        if [ -f "$local_dir/service.env.local" ]; then
            gs_load_env "$local_dir/service.env.local"
            for key in SITE_CONFIG_SOURCE BOOTSTRAP_INI DOCKERFILE; do
                value="$(sed -n "s/^$key=//p" "$local_dir/service.env.local" | tail -n 1)"
                if [ -n "$value" ] && [ "${!key:0:1}" != "/" ] && [ -n "${!key}" ]; then
                    export "$key=$local_dir/${!key}"
                fi
            done
        fi
        if [ -f "$local_dir/site.json" ]; then
            export SITE_CONFIG_SOURCE="$local_dir/site.json"
        fi
        GS_LOCAL_DIR="$local_dir"
    fi
    GS_ENV_DIR="$env_dir"
    export GS_ENV_DIR GS_LOCAL_DIR
}

# a value as a YAML double-quoted scalar (backslash and double quote escaped; control characters are refused before)
gs_yaml_quote() {
    local value="$1"
    value="${value//\\/\\\\}"
    value="${value//\"/\\\"}"
    printf '"%s"' "$value"
}

# ---- storage classes (resolve_storage_class.sh, render_manifests.sh, deploy.sh) ----------------------------------------
# The friendly names of STORAGE_CLASS, the order in which auto tries them, and the StorageClass names each stands for,
# the Longhorn-managed class first: when a cluster has both, the Longhorn one wins.
GS_STORAGE_FRIENDLY_NAMES="raid mirror jbod local-path"
GS_STORAGE_AUTO_ORDER="raid mirror jbod local-path"

gs_storage_class_candidates() {
    case "$1" in
        raid) echo "longhorn-raid raid" ;;
        mirror) echo "longhorn-mirror mirror" ;;
        jbod) echo "longhorn-jbod jbod" ;;
        local-path) echo "local-path" ;;
        *) return 1 ;;
    esac
}

gs_storage_class_is_friendly() {
    case " $GS_STORAGE_FRIENDLY_NAMES " in
        *" $1 "*) return 0 ;;
    esac
    return 1
}

# auto, a friendly name, or something that can be a StorageClass name (a DNS subdomain)
gs_storage_class_request_valid() {
    [ "$1" = "auto" ] || gs_storage_class_is_friendly "$1" || [[ "$1" =~ ^[a-z0-9]([-a-z0-9.]{0,251}[a-z0-9])?$ ]]
}

gs_is_elf() {
    [ "$(head -c 4 "$1" 2> /dev/null | od -An -tx1 | tr -d ' \n')" = "7f454c46" ]
}
