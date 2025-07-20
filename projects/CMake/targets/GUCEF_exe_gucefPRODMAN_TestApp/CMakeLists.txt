#-------------------------------------------------------------------
# This file is part of the CMake build system for GUCEF
#     (Galaxy Unlimited Framework)
# For the latest info, see http://www.VanvelzenSoftware.com/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

######################################################################
# GUCEF_exe_gucefPRODMAN_TestApp BUILD SYSTEM
# Welcome to the CMake build system for GUCEF_exe_gucefPRODMAN_TestApp.
# This is the main file where we prepare the general build environment
# and provide build configuration options.
######################################################################

cmake_minimum_required(VERSION 2.6)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)
cmake_policy(SET CMP0003 NEW)

if ( DEFINED ENV{GUCEF_HOME} )
  set( GUCEF_HOME_DIR "$ENV{GUCEF_HOME}" )
  STRING(REGEX REPLACE "\\\\" "/" GUCEF_HOME_DIR ${GUCEF_HOME_DIR})
  message(STATUS "Setting CMake variable GUCEF_HOME_DIR based on env var GUCEF_HOME to ${GUCEF_HOME_DIR}")
endif()
if ( DEFINED ENV{GUCEF_SOURCE_DIR} )
  set( GUCEF_SOURCE_DIR "$ENV{GUCEF_SOURCE_DIR}" )
  STRING(REGEX REPLACE "\\\\" "/" GUCEF_SOURCE_DIR ${GUCEF_SOURCE_DIR})
  message(STATUS "Setting CMake variable GUCEF_SOURCE_DIR based on env var GUCEF_SOURCE_DIR to ${GUCEF_SOURCE_DIR}")
endif()

if ( NOT DEFINED GUCEF_HOME_DIR )
  set( GUCEF_HOME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../" )
  message(STATUS "GUCEF_HOME_DIR not defined, defaulting to CMake file relative dir: ${GUCEF_HOME_DIR}")
endif()
if ( NOT DEFINED GUCEF_SOURCE_DIR )
  set( GUCEF_SOURCE_DIR ${GUCEF_HOME_DIR} )
  message(STATUS "GUCEF_SOURCE_DIR not defined, defaulting to GUCEF_HOME_DIR: ${GUCEF_HOME_DIR}")
endif()
if ( NOT DEFINED GUCEF_BINARY_DIR )
  set( GUCEF_BINARY_DIR "${CMAKE_BINARY_DIR}" )
endif()

set( CMAKE_INSTALL_PREFIX "${GUCEF_BINARY_DIR}/dist" CACHE PATH
        "Install path prefix, prepended onto install directories." FORCE )

message(STATUS "Source dir: ${GUCEF_SOURCE_DIR}")
message(STATUS "Binary dir: ${GUCEF_BINARY_DIR}")
message(STATUS "CMake Binary dir: ${CMAKE_BINARY_DIR}")

if ( NOT DEFINED GUCEF_SYSTEM_NAME )
  if ( DEFINED CMAKE_SYSTEM_NAME )
    set( GUCEF_SYSTEM_NAME ${CMAKE_SYSTEM_NAME} )
  else()
    set( GUCEF_SYSTEM_NAME ${CMAKE_HOST_SYSTEM_NAME} )
  endif()
endif()

if ( GUCEF_SYSTEM_NAME STREQUAL "Linux" )
  if ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    set( LINUX64 TRUE )
    set( GUCEF_TARGET_PLATFORM "LINUX64" )
    message(STATUS "Determined that pointer size is 8 bytes so this is a 64bit system")
  else ()
    set( LINUX32 TRUE )
    set( GUCEF_TARGET_PLATFORM "LINUX32" )
    message(STATUS "Determined that pointer size is 4 bytes so this is a 32bit system")
  endif()
endif()

message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}" )
message(STATUS "GUCEF_TARGET_PLATFORM: ${GUCEF_TARGET_PLATFORM}" )


project( GUCEF_exe_gucefPRODMAN_TestApp )

# Include necessary submodules
set(CMAKE_MODULE_PATH
  "${GUCEF_SOURCE_DIR}/projects/CMake"
  "${GUCEF_SOURCE_DIR}/projects/CMake/Utils"
  "${GUCEF_SOURCE_DIR}/projects/CMake/Packages"
)
include(CMakeDependentOption)
include(MacroLogFeature)
include(GUCEFConfigTargets)
include(PreprocessorUtils)
set(GUCEF_TEMPLATES_DIR "${GUCEF_SOURCE_DIR}/projects/CMake/Templates")
set(GUCEF_WORK_DIR ${GUCEF_BINARY_DIR})


#####################################################################
# Set up the basic build environment
#####################################################################

if (CMAKE_BUILD_TYPE STREQUAL "")
  # CMake defaults to leaving CMAKE_BUILD_TYPE empty. This screws up
  # differentiation between debug and release builds.
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: None (CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif ()

# determine GUCEF_exe_gucefPRODMAN_TestApp version numbers
include(GUCEFGetVersion)
GUCEF_get_version(${GUCEF_SOURCE_DIR}/common/include/gucef_prerequisites.h)
message(STATUS "Configuring GUCEF_exe_gucefPRODMAN_TestApp ${GUCEF_VERSION}")

if (NOT APPLE)
  # Create debug libraries with _d postfix
  set(CMAKE_DEBUG_POSTFIX "_d")
endif ()

# Set compiler specific build flags
if (CMAKE_COMPILER_IS_GNUCXX)
  add_definitions(-msse)
endif ()
if (MSVC)
  add_definitions(/fp:fast)
endif ()

if (CMAKE_COMPILER_IS_GNUCXX)
  # Test for GCC visibility
  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag(-fvisibility=hidden GUCEF_GCC_VISIBILITY)
  if (GUCEF_GCC_VISIBILITY)
    # determine gcc version
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion
      OUTPUT_VARIABLE GUCEF_GCC_VERSION)
    message(STATUS "Detected g++ ${GUCEF_GCC_VERSION}")
    message(STATUS "Enabling GCC visibility flags")
    set(GUCEF_GCC_VISIBILITY_FLAGS "-DGUCEF_GCC_VISIBILITY -fvisibility=hidden")

    # check if we can safely add -fvisibility-inlines-hidden
    string(TOLOWER "${CMAKE_BUILD_TYPE}" GUCEF_BUILD_TYPE)
    if (GUCEF_BUILD_TYPE STREQUAL "debug" AND GUCEF_GCC_VERSION VERSION_LESS "4.2")
      message(STATUS "Skipping -fvisibility-inlines-hidden due to possible bug in g++ < 4.2")
    else ()
      set(GUCEF_GCC_VISIBILITY_FLAGS "${GUCEF_GCC_VISIBILITY_FLAGS} -fvisibility-inlines-hidden")
    endif ()
  endif (GUCEF_GCC_VISIBILITY)
endif (CMAKE_COMPILER_IS_GNUCXX)

# determine system endianess
include(TestBigEndian)
test_big_endian(GUCEF_TEST_BIG_ENDIAN)

# Add common include path
include_directories("${GUCEF_SOURCE_DIR}/common/include")

# Specify build paths
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${GUCEF_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${GUCEF_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${GUCEF_BINARY_DIR}/bin")

# Set up iPhone overrides.
if (GUCEF_BUILD_PLATFORM_IPHONE)
  # Force gcc <= 4.2 on iPhone
  include(CMakeForceCompiler)
  CMAKE_FORCE_C_COMPILER(gcc-4.2 GNU)
  CMAKE_FORCE_CXX_COMPILER(gcc-4.2 GNU)

  # Set build variables
  set(CMAKE_OSX_SYSROOT iphoneos3.0)
  set(CMAKE_OSX_ARCHITECTURES "$(ARCHS_UNIVERSAL_IPHONE_OS)")
  set(CMAKE_EXE_LINKER_FLAGS "-framework Foundation -framework CoreGraphics -framework QuartzCore -framework UIKit ${GUCEF_SOURCE_DIR}/iPhoneDependencies/lib/release/libboost_date_time.a ${GUCEF_SOURCE_DIR}/iPhoneDependencies/lib/release/libboost_thread.a")
  set(XCODE_ATTRIBUTE_SDKROOT iphoneos3.0)
  set(GUCEF_BUILD_RENDERSYSTEM_GLES TRUE CACHE BOOL "Forcing OpenGL ES RenderSystem for iPhone" FORCE)
  set(GUCEF_STATIC TRUE CACHE BOOL "Forcing static build for iPhone" FORCE)
  set(GUCEF_SET_USE_BOOST TRUE CACHE BOOL "Forcing use of Boost libraries for iPhone" FORCE)
  set(GUCEF_USE_BOOST TRUE CACHE BOOL "Forcing use of Boost libraries for iPhone" FORCE)
  set(Boost_FOUND TRUE CACHE BOOL "Forcing use of Boost libraries for iPhone" FORCE)
  set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.yourcompany.\${PRODUCT_NAME:rfc1034identifier}")

  add_definitions(-fno-regmove)
  remove_definitions(-msse)
elseif (APPLE AND NOT GUCEF_BUILD_PLATFORM_IPHONE)
  # Force gcc <= 4.0 on Mac OS X because 4.2 is not supported prior to Mac OS X 10.5
  include(CMakeForceCompiler)
  CMAKE_FORCE_C_COMPILER(gcc-4.0 GNU)
  CMAKE_FORCE_CXX_COMPILER(gcc-4.0 GNU)

  # Set 10.4 as the base SDK by default
  set(XCODE_ATTRIBUTE_SDKROOT macosx10.4)
  
endif ()

######################################################################
# Provide user options to customise the build process
######################################################################

# Customise what to build
option(GUCEF_STATIC "Static build" FALSE)
set(GUCEF_CONFIG_ALLOCATOR 2 CACHE STRING
"Specify the memory allocator to use. Possible values:
  1 - Standard allocator
  2 - nedmalloc
  3 - User-provided allocator"
)
option(GUCEF_CONFIG_MEMTRACK_DEBUG "Enable GUCEF's memory tracker in debug mode" FALSE)
option(GUCEF_CONFIG_MEMTRACK_RELEASE "Enable GUCEF's memory tracker in release mode" FALSE)


###################################################################
# configure global build settings based on selected build options
###################################################################

include(ConfigureBuild)

##################################################################
# Now setup targets
##################################################################

include( GUCEF_exe_gucefPRODMAN_TestApp_ModuleDirs.cmake )

# Provide CPack packaging target
include( GUCEF_exe_gucefPRODMAN_TestApp_Packaging.cmake )
