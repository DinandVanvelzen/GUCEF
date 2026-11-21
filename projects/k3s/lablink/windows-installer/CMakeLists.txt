# ---------------------------------------------------------------------------
# The Windows installer of gucefLabLink: a CPack project that packs a Windows
# build of gucefLabLink (gucefLabLink.exe and the DLLs next to it) together
# with install.ps1 and uninstall.ps1 of projects/k3s/lablink into one setup
# program. Run as an administrator, the setup program installs the files and
# lets install.ps1 register and start the Windows service gucefLabLink; the
# uninstaller (Apps and features) runs uninstall.ps1. The service logic is that
# of the scripts, there is no second copy of it here.
#
# Usage:
#   cmake -S projects/k3s/lablink/windows-installer -B <dir> -DLABLINK_BIN_DIR=<bin directory of the build>
#   cpack --config <dir>/CPackConfig.cmake
# or projects\k3s\lablink\build-installer.ps1, which builds first. The setup
# program is made with NSIS (makensis in the PATH or installed in the default
# place); without it a ZIP with the same files is made, which is installed with
# scripts\install.ps1 -Prebuilt bin from an elevated PowerShell.
#
# Variables:
#   LABLINK_BIN_DIR  directory with gucefLabLink.exe and its DLLs (required)
#   LABLINK_VERSION  version of the package, default: that of the program
#                    (AgentVersion of tools/gucefLabLink) or 1.0.0
# ---------------------------------------------------------------------------

cmake_minimum_required( VERSION 3.15 )
project( gucefLabLinkWindowsInstaller NONE )

set( LABLINK_BIN_DIR "" CACHE PATH "Directory with gucefLabLink.exe and the DLLs of a Windows build" )
set( LABLINK_VERSION "" CACHE STRING "Version of the package, default: that of the program" )

if( NOT LABLINK_BIN_DIR OR NOT EXISTS "${LABLINK_BIN_DIR}/gucefLabLink.exe" )
    message( FATAL_ERROR "LABLINK_BIN_DIR must be the directory with gucefLabLink.exe of a Windows build: \"${LABLINK_BIN_DIR}\"" )
endif()

get_filename_component( K3S_LABLINK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE )
foreach( script install.ps1 uninstall.ps1 README.md config.env.example )
    if( NOT EXISTS "${K3S_LABLINK_DIR}/${script}" )
        message( FATAL_ERROR "${K3S_LABLINK_DIR}/${script} is missing" )
    endif()
endforeach()

# the version of the program: AgentVersion = "gucefLabLink 1.1"
if( NOT LABLINK_VERSION )
    set( LABLINK_VERSION "1.0.0" )
    set( protocolSource "${CMAKE_CURRENT_SOURCE_DIR}/../../../../tools/gucefLabLink/src/gucefLabLink_Protocol.cpp" )
    if( EXISTS "${protocolSource}" )
        file( STRINGS "${protocolSource}" agentVersionLine REGEX "AgentVersion[ ]*=" LIMIT_COUNT 1 )
        if( agentVersionLine MATCHES "\"gucefLabLink ([0-9]+)\\.([0-9]+)" )
            set( LABLINK_VERSION "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.0" )
        endif()
    endif()
endif()
string( REGEX MATCH "^([0-9]+)\\.([0-9]+)\\.([0-9]+)" versionOk "${LABLINK_VERSION}" )
if( NOT versionOk )
    message( FATAL_ERROR "LABLINK_VERSION is major.minor.patch: \"${LABLINK_VERSION}\"" )
endif()

# the files: the program and its DLLs (the gucef libraries and the plugins it loads), not a test application
file( GLOB lablinkDlls "${LABLINK_BIN_DIR}/*.dll" )
if( NOT lablinkDlls )
    message( FATAL_ERROR "There are no DLLs next to ${LABLINK_BIN_DIR}/gucefLabLink.exe: build with BUILD_SHARED_LIBS=ON." )
endif()
foreach( plugin codecspluginMONOCYPHER dstorepluginJSONPARSER tablecodecpluginPARQUET dbpluginVFSTABLES )
    file( GLOB found "${LABLINK_BIN_DIR}/${plugin}.dll" )
    if( NOT found )
        message( WARNING "${plugin}.dll is not in ${LABLINK_BIN_DIR}: the service will lack what that plugin does." )
    endif()
endforeach()

install( FILES "${LABLINK_BIN_DIR}/gucefLabLink.exe" ${lablinkDlls} DESTINATION bin )
install( FILES
    "${K3S_LABLINK_DIR}/install.ps1"
    "${K3S_LABLINK_DIR}/uninstall.ps1"
    "${K3S_LABLINK_DIR}/README.md"
    "${K3S_LABLINK_DIR}/config.env.example"
    DESTINATION scripts )

# ---------------------------------------------------------------------------
# CPack
# ---------------------------------------------------------------------------
set( CPACK_PACKAGE_NAME "gucefLabLink" )
set( CPACK_PACKAGE_VENDOR "Vanvelzen LLC" )
set( CPACK_PACKAGE_CONTACT "Dinand Vanvelzen <dinand@vanvelzen.us>" )
set( CPACK_PACKAGE_DESCRIPTION_SUMMARY "Links this machine to the k3s cluster of the home network" )
set( CPACK_PACKAGE_VERSION "${LABLINK_VERSION}" )
set( CPACK_PACKAGE_FILE_NAME "gucefLabLink-${LABLINK_VERSION}-setup" )
set( CPACK_PACKAGE_INSTALL_DIRECTORY "gucef-lablink-setup" )
set( CPACK_PACKAGE_INSTALL_REGISTRY_KEY "gucefLabLink" )
set( CPACK_MONOLITHIC_INSTALL ON )
set( CPACK_STRIP_FILES FALSE )

set( programFiles86 "ProgramFiles(x86)" )     # the parentheses cannot be written in $ENV{} directly
find_program( MAKENSIS_EXECUTABLE makensis
    HINTS "$ENV{ProgramFiles}/NSIS" "$ENV{${programFiles86}}/NSIS" "$ENV{ProgramW6432}/NSIS" )
if( MAKENSIS_EXECUTABLE )
    set( CPACK_GENERATOR "NSIS" )
    set( CPACK_NSIS_EXECUTABLE "${MAKENSIS_EXECUTABLE}" )
    message( STATUS "Package generation: NSIS setup program (${MAKENSIS_EXECUTABLE})" )
else()
    set( CPACK_GENERATOR "ZIP" )
    message( STATUS "Package generation: makensis not found, a ZIP is made instead of the setup program (install NSIS: https://nsis.sourceforge.io)" )
endif()

# the setup program asks for administrator rights itself (RequestExecutionLevel admin in the NSIS template)
set( CPACK_NSIS_PACKAGE_NAME "GUCEF lab link" )
set( CPACK_NSIS_DISPLAY_NAME "GUCEF lab link" )
set( CPACK_NSIS_CONTACT "${CPACK_PACKAGE_CONTACT}" )
set( CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64" )
set( CPACK_NSIS_MODIFY_PATH OFF )
set( CPACK_NSIS_IGNORE_LICENSE_PAGE ON )
set( CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL OFF )

# The NSIS code is in files next to this one, so that it is read as NSIS and not as CMake strings. CPack writes the
# value of a variable into its configuration file between quotes without escaping anything, and reads that file as
# CMake code again: the backslashes, quotes and dollar signs of the NSIS code are escaped here for that.
foreach( part DEFINES:defines EXTRA_INSTALL_COMMANDS:install EXTRA_UNINSTALL_COMMANDS:uninstall )
    string( REPLACE ":" ";" partInfo "${part}" )
    list( GET partInfo 0 partVariable )
    list( GET partInfo 1 partFile )
    file( READ "${CMAKE_CURRENT_SOURCE_DIR}/lablink-${partFile}.nsh" nsisCode )
    string( REPLACE "\\" "\\\\" nsisCode "${nsisCode}" )
    string( REPLACE "\"" "\\\"" nsisCode "${nsisCode}" )
    string( REPLACE "$" "\\$" nsisCode "${nsisCode}" )
    set( CPACK_NSIS_${partVariable} "${nsisCode}" )
endforeach()

include( CPack )
