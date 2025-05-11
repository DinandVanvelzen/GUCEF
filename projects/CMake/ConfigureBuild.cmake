#-------------------------------------------------------------------
# This file is part of the CMake build system for GUCEF
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.VanvelzenSoftware.com/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

#######################################################################
# This file takes care of configuring GUCEF to build with the settings
# given in CMake. It creates the necessary config.h file and will 
# also prepare package files for pkg-config and CMake.
#######################################################################

if (GUCEF_BUILD_PLATFORM_IPHONE)
  set(GUCEF_SET_BUILD_PLATFORM_IPHONE 1)
  set(GUCEF_STATIC 1)
  set(GUCEF_STATIC_LIB 1)
endif()

# should we build static libs?
if (GUCEF_STATIC)
  set(GUCEF_LIB_TYPE STATIC)
else ()
  set(GUCEF_LIB_TYPE SHARED)
endif ()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# determine config values depending on build options 
set(GUCEF_SET_MEMTRACK_DEBUG 0)
set(GUCEF_SET_MEMTRACK_RELEASE 0)
set(GUCEF_STATIC_LIB 0)
if (GUCEF_CONFIG_CONTAINERS_USE_CUSTOM_ALLOCATOR)
  set(GUCEF_SET_CONTAINERS_USE_ALLOCATOR 1)
endif ()
if (GUCEF_CONFIG_STRING_USE_CUSTOM_ALLOCATOR)
  set(GUCEF_SET_STRING_USE_ALLOCATOR 1)
endif ()
if (GUCEF_CONFIG_MEMTRACK_DEBUG)
  set(GUCEF_SET_MEMTRACK_DEBUG 1)
endif()
if (GUCEF_CONFIG_MEMTRACK_RELEASE)
  set(GUCEF_SET_MEMTRACK_RELEASE 1)
endif()
if (GUCEF_STATIC)
  set(GUCEF_STATIC_LIB 1)
endif()
add_definitions(-DHAVE_GUCEF_BUILDSETTINGS_H)

if (GUCEF_TEST_BIG_ENDIAN)
  set(GUCEF_CONFIG_BIG_ENDIAN 1)
else ()
  set(GUCEF_CONFIG_LITTLE_ENDIAN 1)
endif ()

# Create the pkg-config package files on Unix systems
if (UNIX)
  set(GUCEF_LIB_SUFFIX "")
  set(GUCEF_PLUGIN_PREFIX "")
  set(GUCEF_PLUGIN_EXT ".so")
  if (GUCEF_STATIC)
    set(GUCEF_LIB_SUFFIX "${GUCEF_LIB_SUFFIX}Static")
    set(GUCEF_PLUGIN_PREFIX "lib")
    set(GUCEF_PLUGIN_EXT ".a")
  endif ()
  string(TOLOWER "${CMAKE_BUILD_TYPE}" GUCEF_BUILD_TYPE)
  if (GUCEF_BUILD_TYPE STREQUAL "debug")
    set(GUCEF_LIB_SUFFIX "${GUCEF_LIB_SUFFIX}_d")
  endif ()

  set(GUCEF_ADDITIONAL_LIBS "")
  set(GUCEF_CFLAGS "")
  set(GUCEF_PREFIX_PATH ${CMAKE_INSTALL_PREFIX})
  if (GUCEF_CONFIG_THREADS GREATER 0)
    set(GUCEF_CFLAGS "-pthread")
    set(GUCEF_ADDITIONAL_LIBS "${GUCEF_ADDITIONAL_LIBS} -lpthread")
  endif ()
  #if (GUCEF_STATIC)
  #  configure_file(${GUCEF_TEMPLATES_DIR}/GUCEFStatic.pc.in ${GUCEF_BINARY_DIR}/pkgconfig/GUCEF${GUCEF_LIB_SUFFIX}.pc @ONLY)
  #else ()
  #  configure_file(${GUCEF_TEMPLATES_DIR}/GUCEF.pc.in ${GUCEF_BINARY_DIR}/pkgconfig/GUCEF${GUCEF_LIB_SUFFIX}.pc @ONLY)
  #endif ()
  #install(FILES ${GUCEF_BINARY_DIR}/pkgconfig/GUCEF${GUCEF_LIB_SUFFIX}.pc DESTINATION lib/pkgconfig)

  # configure additional packages
  
endif ()

if (GUCEF_STANDALONE_BUILD)
  set(CMAKE_USE_RELATIVE_PATHS true)
  set(CMAKE_SUPPRESS_REGENERATION true)
endif()

if (MSVC)
  # Enable intrinsics on MSVC in debug mode
  # Not actually necessary in release mode since /O2 implies /Oi but can't easily add this per build type?
  add_definitions(/Oi)
  
  add_compile_options(-bigobj)
endif (MSVC)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if (CMAKE_CXX_COMPILER MATCHES "em\\+\\+")
        message(STATUS "Building with Emscripten")
        set(CMAKE_CXX_STANDARD 98)  # Explicitly set C++98 compatibility
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_SDL=2")
    endif()
endif()
