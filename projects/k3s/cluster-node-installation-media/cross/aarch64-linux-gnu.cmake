# CMake toolchain for arm64 (aarch64) Linux with the Ubuntu/Debian cross
# compilers (crossbuild-essential-arm64) and arm64 multiarch libraries.
set( CMAKE_SYSTEM_NAME Linux )
set( CMAKE_SYSTEM_PROCESSOR aarch64 )
set( CMAKE_C_COMPILER aarch64-linux-gnu-gcc )
set( CMAKE_CXX_COMPILER aarch64-linux-gnu-g++ )
set( CMAKE_LIBRARY_ARCHITECTURE aarch64-linux-gnu )
set( CMAKE_FIND_ROOT_PATH /usr/aarch64-linux-gnu )
set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )
set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH )
set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH )
