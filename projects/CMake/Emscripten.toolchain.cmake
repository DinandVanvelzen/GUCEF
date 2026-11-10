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

# What an executable is turned into: ".html" gives a page that can be opened in a browser,
# ".js" gives a script that can be run with node, which is what the test applications use.
set(GUCEF_EMSCRIPTEN_EXECUTABLE_SUFFIX ".html" CACHE STRING "Suffix of executables: .html or .js")
set(CMAKE_EXECUTABLE_SUFFIX "${GUCEF_EMSCRIPTEN_EXECUTABLE_SUFFIX}" CACHE STRING "Executable suffix" FORCE)

# Threads in the browser need SharedArrayBuffer, which a page only gets when it is served with the
# COOP and COEP headers set to cross-origin isolation. Because that restricts how the page can be hosted
# threads are an explicit choice. Without them everything runs on the browser main thread.
option(GUCEF_EMSCRIPTEN_PTHREADS "Build with pthread support (requires cross-origin isolated hosting)" OFF)

# For compile flags
# GUCEF uses C++ exceptions, for example to signal lock timeouts, and Emscripten turns them into aborts unless asked not to
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -fexceptions")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -fexceptions")
if(GUCEF_EMSCRIPTEN_PTHREADS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    set(GUCEF_EMSCRIPTEN_LINK_PTHREAD_FLAGS "-pthread -s PTHREAD_POOL_SIZE=4")
else()
    set(GUCEF_EMSCRIPTEN_LINK_PTHREAD_FLAGS "")
endif()

# Executables that are scripts for node (the test applications) get the real file system and environment of the machine
# they run on, so that they can read and write files, find their data and report their results
if(GUCEF_EMSCRIPTEN_EXECUTABLE_SUFFIX STREQUAL ".js")
    set(GUCEF_EMSCRIPTEN_LINK_NODE_FLAGS "-s NODERAWFS=1 -s ENVIRONMENT=node")
else()
    set(GUCEF_EMSCRIPTEN_LINK_NODE_FLAGS "")
endif()

# For link flags (only applied during final linking)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fexceptions -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 ${GUCEF_EMSCRIPTEN_LINK_PTHREAD_FLAGS} ${GUCEF_EMSCRIPTEN_LINK_NODE_FLAGS}" CACHE STRING "" FORCE)
