#-------------------------------------------------------------------
# This file is part of the CMake build system for GUCEF
#     (Galaxy Unlimited Framework)
# For the latest info, see http://www.VanvelzenSoftware.com/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

######################################################################
# OIS MODULE BUILD
# Standalone CMake root which builds, installs and packages the single module OIS.
# The modules it depends on are found as installed CMake packages, see CMAKE_PREFIX_PATH,
# for example as provided by Conan.
######################################################################

cmake_minimum_required(VERSION 3.20)

# The source tree is located relative to this file. The GUCEF_HOME environment variable is not used here
# since a module build has to use the sources it was generated for, which may be an exported copy
if ( NOT DEFINED GUCEF_SOURCE_DIR )
  get_filename_component( GUCEF_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../" ABSOLUTE )
endif()
set( GUCEF_HOME_DIR "${GUCEF_SOURCE_DIR}" )
if ( NOT DEFINED GUCEF_BINARY_DIR )
  set( GUCEF_BINARY_DIR "${CMAKE_BINARY_DIR}" )
endif()

message(STATUS "Source dir: ${GUCEF_SOURCE_DIR}")
message(STATUS "Binary dir: ${GUCEF_BINARY_DIR}")

project( OIS )

# Unless an install prefix is given, install next to the build output like the target builds do
if ( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )
  set( CMAKE_INSTALL_PREFIX "${GUCEF_BINARY_DIR}/dist" CACHE PATH
          "Install path prefix, prepended onto install directories." FORCE )
endif()

# A module build installs its headers and its CMake package so that dependents can be built against it
set( GUCEF_INSTALL_DEVELOPMENT_FILES ON )

# Shared build setup: platform detection, compiler settings, output dirs and build options
include( "${GUCEF_SOURCE_DIR}/projects/CMake/GUCEFBuildSetup.cmake" )

##################################################################
# Now setup the module
##################################################################

# Find the dependencies, add the module and install its package
include( OIS_ModuleRoot.cmake )
