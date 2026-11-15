#-------------------------------------------------------------------
# This file is part of the CMake build system for GUCEF
#     (Galaxy Unlimited Framework)
# For the latest info, see http://www.VanvelzenSoftware.com/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

######################################################################
# GUCEF_exe_gucefMcpEcho BUILD SYSTEM
# Welcome to the CMake build system for GUCEF_exe_gucefMcpEcho.
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



# project() sets <name>_SOURCE_DIR and <name>_BINARY_DIR, which for the project named GUCEF are the GUCEF dirs:
# keep the dirs determined above
set( GUCEF_SOURCE_DIR_OF_TREE "${GUCEF_SOURCE_DIR}" )
set( GUCEF_BINARY_DIR_OF_TREE "${GUCEF_BINARY_DIR}" )
project( GUCEF_exe_gucefMcpEcho )
set( GUCEF_SOURCE_DIR "${GUCEF_SOURCE_DIR_OF_TREE}" )
set( GUCEF_BINARY_DIR "${GUCEF_BINARY_DIR_OF_TREE}" )

# Shared build setup: platform detection, compiler settings, output dirs and build options
include( "${GUCEF_SOURCE_DIR}/projects/CMake/GUCEFBuildSetup.cmake" )

##################################################################
# Now setup targets
##################################################################

# Add all relevant modules for the target
include( GUCEF_exe_gucefMcpEcho_ModuleDirs.cmake )

# Provide CPack packaging target
include( GUCEF_exe_gucefMcpEcho_Packaging.cmake )

##################################################################
# Add documentation capability if feasible
##################################################################

# Add documentation generation if Doxygen is available
option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" ON)

if(BUILD_DOCUMENTATION)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        # Add a documentation target that generates docs for the entire solution
        set(DOXYGEN_INPUT_DIR "${GUCEF_SOURCE_DIR}")
        set(DOXYGEN_OUTPUT_DIR "${GUCEF_BINARY_DIR}/docs/doxygen")
        set(DOXYGEN_CONFIG_IN "${GUCEF_SOURCE_DIR}/projects/CMake/Templates/Doxyfile.in")
        set(DOXYGEN_CONFIG_OUT "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")
        
        # Create docs directory
        file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})
        
        # Configure the Doxyfile
        configure_file(${DOXYGEN_CONFIG_IN} ${DOXYGEN_CONFIG_OUT} @ONLY)

        # Add the main documentation target
        add_custom_target(DOCS
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_CONFIG_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen for GUCEF_exe_gucefMcpEcho"
            VERBATIM
        )

        # Set target properties to organize in IDE
        set_target_properties(DOCS PROPERTIES
            FOLDER "Documentation"
        )
        
        message(STATUS "Documentation target 'DOCS' will be available for GUCEF_exe_gucefMcpEcho")
    else()
        message(WARNING "Doxygen not found: Documentation will not be available")
    endif()
endif()
