#!/bin/bash
# Builds one matrix cell: every product in CELL_PRODUCTS with the cell's C++ standard, build type and toolchain.
# Environment: CELL_ID CELL_PLATFORM CELL_COMPILER CELL_CPP_STANDARD CELL_BUILD_TYPE CELL_CMAKE_ARGS CELL_PRODUCTS
# Each product is a generated CMake project (projects/CMake/targets/<product>); a failed product does not stop the others.

set -u
REPO_ROOT="${GITHUB_WORKSPACE:-$(pwd)}"
cd "${REPO_ROOT}" || exit 1
PRODUCTS_FILE="${CI_MATRIX_DIR:-projects/CI/matrix}/products.json"

echo "Cell ${CELL_ID}: platform=${CELL_PLATFORM} compiler=${CELL_COMPILER} c++${CELL_CPP_STANDARD} ${CELL_BUILD_TYPE}"
echo "Products: ${CELL_PRODUCTS}"

# Debian based build images: install what the image lacks
need=""
command -v cmake >/dev/null || need="${need} cmake"
command -v ninja >/dev/null || need="${need} ninja-build"
[ "${CELL_COMPILER}" = "clang" ] && ! command -v clang++ >/dev/null && need="${need} clang"
if [ -n "${need}" ]; then
    echo "Installing:${need}"
    SUDO=""; [ "$(id -u)" -ne 0 ] && SUDO="sudo"
    ${SUDO} apt-get update -qq && ${SUDO} apt-get install -y -qq ${need} >/dev/null || { echo "Could not install${need}"; exit 1; }
fi

COMPILER_ARGS=""
case "${CELL_COMPILER}" in
    gcc)   COMPILER_ARGS="-DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++" ;;
    clang) COMPILER_ARGS="-DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++" ;;
esac
EXTRA_ARGS="$(eval echo "${CELL_CMAKE_ARGS}")"
GENERATOR=""; command -v ninja >/dev/null && GENERATOR="-G Ninja"

ok=0; failed=""
for product in ${CELL_PRODUCTS}; do
    project="projects/CMake/targets/${product}"
    if [ -f "${PRODUCTS_FILE}" ] && command -v python3 >/dev/null; then
        project="$(python3 -c "import json,sys; p=[x for x in json.load(open(sys.argv[1]))['products'] if x['name']==sys.argv[2]]; print(p[0]['project'] if p else '')" "${PRODUCTS_FILE}" "${product}")"
        [ -n "${project}" ] || project="projects/CMake/targets/${product}"
    fi
    if [ ! -f "${project}/CMakeLists.txt" ]; then
        echo "Skipping ${product}: no generated CMake project at ${project}"
        continue
    fi
    build_dir="common/bin/${CELL_ID}/${product}"
    echo "::group::${product}"
    if cmake -S "${project}" -B "${build_dir}" ${GENERATOR} -DCMAKE_BUILD_TYPE="${CELL_BUILD_TYPE}" \
             -DCMAKE_CXX_STANDARD="${CELL_CPP_STANDARD}" ${COMPILER_ARGS} ${EXTRA_ARGS} \
       && cmake --build "${build_dir}" --parallel; then
        ok=$((ok + 1))
    else
        failed="${failed} ${product}"
    fi
    echo "::endgroup::"
done

echo "Cell ${CELL_ID}: ${ok} built"
if [ -n "${failed}" ]; then
    echo "Cell ${CELL_ID}: FAILED:${failed}"
    exit 1
fi
