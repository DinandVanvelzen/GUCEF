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

gs_is_elf() {
    [ "$(head -c 4 "$1" 2> /dev/null | od -An -tx1 | tr -d ' \n')" = "7f454c46" ]
}
