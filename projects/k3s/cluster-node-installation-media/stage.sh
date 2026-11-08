#!/bin/bash
# ---------------------------------------------------------------------------
# Copies a program of a relocatable GUCEF build (built with
# -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON -DCMAKE_INSTALL_RPATH=$ORIGIN/../lib) to
# a directory, with the libraries it needs: STAGE_DIR/bin/PROGRAM,
# STAGE_DIR/lib/ with the given libraries (plugins the program loads), every
# library of the build these link against and the names they are linked by,
# and the given documents in STAGE_DIR. The result is stripped and readable
# for everyone.
#
# Usage: stage.sh [--doc FILE[:NAME]]... BUILD_DIR STAGE_DIR PROGRAM [LIBRARY.so ...]
#   --doc FILE[:NAME]  copies FILE to STAGE_DIR/NAME (default: its own name)
#
# Builds for another architecture: OBJDUMP and STRIP name the tools for it
# (e.g. aarch64-linux-gnu-objdump); default objdump and strip.
# ---------------------------------------------------------------------------

OBJDUMP="${OBJDUMP:-objdump}"
STRIP="${STRIP:-strip}"

DOCS=()
while [ "$1" = "--doc" ]; do
    DOCS+=("$2")
    shift 2
done
if [ $# -lt 3 ]; then
    echo "Usage: stage.sh [--doc FILE[:NAME]]... BUILD_DIR STAGE_DIR PROGRAM [LIBRARY.so ...]"
    exit 1
fi
BUILD_DIR="$1"
STAGE_DIR="$2"
PROGRAM="$3"
shift 3

mkdir -p "${STAGE_DIR}/bin" "${STAGE_DIR}/lib" || exit 1
cp "${BUILD_DIR}/bin/${PROGRAM}" "${STAGE_DIR}/bin/" || exit 1
QUEUE=("${BUILD_DIR}/bin/${PROGRAM}")
for library in "$@"; do
    cp "${BUILD_DIR}/lib/${library}" "${STAGE_DIR}/lib/" || exit 1
    QUEUE+=("${BUILD_DIR}/lib/${library}")
done

# the libraries of the build the files link against, and the libraries those link against
declare -A SEEN
while [ ${#QUEUE[@]} -gt 0 ]; do
    file="${QUEUE[0]}"
    QUEUE=("${QUEUE[@]:1}")
    for soname in $("${OBJDUMP}" -p "${file}" 2>/dev/null | awk '/NEEDED/ {print $2}'); do
        [ -n "${SEEN[${soname}]}" ] && continue
        SEEN[${soname}]=1
        [ -e "${BUILD_DIR}/lib/${soname}" ] || continue
        cp -L "${BUILD_DIR}/lib/${soname}" "${STAGE_DIR}/lib/${soname}" || exit 1
        QUEUE+=("${BUILD_DIR}/lib/${soname}")
        # the name the build links against points to the versioned file
        for link in "${BUILD_DIR}/lib/"*.so; do
            [ "$(readlink "${link}")" = "${soname}" ] && ln -sf "${soname}" "${STAGE_DIR}/lib/$(basename "${link}")"
        done
    done
done

for doc in "${DOCS[@]}"; do
    source="${doc%%:*}"
    name="$(basename "${source}")"
    [ "${doc}" != "${source}" ] && name="${doc#*:}"
    install -m 644 "${source}" "${STAGE_DIR}/${name}" || exit 1
done

"${STRIP}" --strip-unneeded "${STAGE_DIR}/bin/${PROGRAM}" "${STAGE_DIR}"/lib/*.so* 2> /dev/null
chmod -R a+rX,go-w "${STAGE_DIR}"
exit 0
