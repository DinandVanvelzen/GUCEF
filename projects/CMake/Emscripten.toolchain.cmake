# EmscriptenWrapper.cmake

if(NOT DEFINED EMSDK)
    if(DEFINED ENV{EMSDK})
        set(EMSDK $ENV{EMSDK})
    else()
        message(FATAL_ERROR "EMSDK environment variable not set. Run emsdk_env.bat first.")
    endif()
endif()

# Tell CMake we are cross-compiling
set(CMAKE_SYSTEM_NAME Emscripten)

# Include the official toolchain
include("${EMSDK}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake")

# Extra defaults
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(EMSCRIPTEN 1)
set(CMAKE_EXECUTABLE_SUFFIX ".html" CACHE STRING "Executable suffix" FORCE)
# For compile flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
# For link flags (only applied during final linking)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s WASM=1 -s ALLOW_MEMORY_GROWTH=1" CACHE STRING "" FORCE)
