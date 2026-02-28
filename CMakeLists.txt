#-------------------------------------------------------------------
# This file is part of the CMake build system for GUCEF
#     (Galaxy Unlimited Framework)
# For the latest info, see http://www.VanvelzenSoftware.com/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

######################################################################
# GUCEF BUILD SYSTEM
# Welcome to the CMake build system for GUCEF.
# This is the main file where we prepare the general build environment
# and provide build configuration options.
######################################################################

cmake_minimum_required(VERSION 3.20)

# Set GUCEF_BUILD_TARGET to the folder name under projects/cmake/targets/
# e.g. via CMakePresets.json or: -DGUCEF_BUILD_TARGET=GUCEF_exe_gucefCORE_TestApp
if(NOT DEFINED GUCEF_BUILD_TARGET)
    message(FATAL_ERROR "GUCEF_BUILD_TARGET is not set. Select a CMake preset or pass -DGUCEF_BUILD_TARGET=<target> where <target> is a folder under projects/cmake/targets/")
endif()

project(GUCEF NONE)
add_subdirectory(
    "projects/cmake/targets/${GUCEF_BUILD_TARGET}"
    "${CMAKE_BINARY_DIR}/${GUCEF_BUILD_TARGET}"
)
