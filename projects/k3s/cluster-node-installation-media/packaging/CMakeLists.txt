#-------------------------------------------------------------------
# Makes one package of the k3s home lab with CPack (DEB, RPM), from files
# that are built and tested already (see ../make-media.sh):
#
#   gucef-k3s-scripts  the scripts of projects/k3s a node and a machine run:
#                      agent-node, nvidia, host-node, llm/mcp, lablink;
#                      in /usr/share/gucef-k3s
#   gucef-mcp          the MCP server gucefMcpLocalLlm, in /usr/lib/gucef-mcp
#   gucef-lablink      gucefLabLink, in /usr/lib/gucef-lablink
#
# The program packages hold the files; their service is set up by the
# install.sh of the scripts (llm/mcp/install.sh, lablink/install.sh), which
# use the package files when there is no checkout. A package upgrade brings a
# service that is set up to the new files; removing a package removes its
# service.
#
# Variables:
#   GUCEF_PACKAGE          one of the names above
#   GUCEF_PACKAGE_VERSION  e.g. 1.2450.202611121530
#   GUCEF_PACKAGE_SOURCE   gucef-k3s-scripts: projects/k3s; others: the staged
#                          files (bin/, lib/)
#   GUCEF_PACKAGE_DEB_LIBRARIES  program packages: the DEB packages of the
#                                system libraries the programs link against
#   GUCEF_PACKAGE_RPM_LIBRARIES  the same as RPM requirements (sonames)
#   GUCEF_PACKAGE_ARCH     program packages: amd64 or arm64; default: the
#                          architecture of this machine
#   GUCEF_PACKAGE_OUTPUT   where the packages go:
#                          <package>-<version>-<amd64|arm64|all>.deb and .rpm
#-------------------------------------------------------------------

cmake_minimum_required( VERSION 3.20 )
project( gucef_k3s_package NONE )

foreach( variable GUCEF_PACKAGE GUCEF_PACKAGE_VERSION GUCEF_PACKAGE_SOURCE GUCEF_PACKAGE_OUTPUT )
  if( NOT ${variable} )
    message( FATAL_ERROR "${variable} is not set" )
  endif()
endforeach()

set( CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE )
set( SCRIPTS_DIR "/usr/share/gucef-k3s" )
set( SCRIPTS_PACKAGE "gucef-k3s-scripts" )
set( VERSION "${GUCEF_PACKAGE_VERSION}" )

if( GUCEF_PACKAGE STREQUAL "gucef-k3s-scripts" )

  set( SUMMARY "GUCEF k3s home lab: scripts to join a node and set up its services" )
  set( DESCRIPTION "The scripts of projects/k3s that a machine of the home lab runs: joining the k3s cluster as an agent node (agent-node), NVIDIA host preparation (nvidia), the MCP service (llm/mcp) and the lab link service (lablink)." )
  set( DEB_ARCH "all" )
  set( RPM_ARCH "noarch" )
  set( DEB_DEPENDS "bash, curl, sudo, systemd" )
  set( RPM_REQUIRES "bash, curl, sudo, systemd" )
  set( FILE_ARCH "all" )
  foreach( directory agent-node nvidia lablink llm/mcp )
    get_filename_component( parent "${directory}" DIRECTORY )
    install( DIRECTORY "${GUCEF_PACKAGE_SOURCE}/${directory}"
             DESTINATION "share/gucef-k3s/${parent}"
             FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
             DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
             PATTERN "*.sh" PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
             PATTERN "config.env" EXCLUDE
             PATTERN ".gitignore" EXCLUDE
             PATTERN "*.old" EXCLUDE )
  endforeach()
  install( PROGRAMS "${GUCEF_PACKAGE_SOURCE}/host-node/set_inotify_limits.sh" DESTINATION "share/gucef-k3s/host-node" )

else()

  if( GUCEF_PACKAGE STREQUAL "gucef-mcp" )
    set( SUMMARY "GUCEF MCP server: local LLM tools for Claude sessions" )
    set( DESCRIPTION "gucefMcpLocalLlm, the MCP server of the service gucef-mcp. Set it up with: sudo ${SCRIPTS_DIR}/llm/mcp/install.sh" )
    set( SETUP_SCRIPT "${SCRIPTS_DIR}/llm/mcp/install.sh" )
    set( UNINSTALL_SCRIPT "${SCRIPTS_DIR}/llm/mcp/uninstall.sh" )
    set( DEB_EXTRA_DEPENDS ", libssl3 | libssl3t64" )
    set( RPM_EXTRA_REQUIRES ", libssl.so.3()(64bit)" )
  elseif( GUCEF_PACKAGE STREQUAL "gucef-lablink" )
    set( SUMMARY "GUCEF lab link: machines of the home network find, join and use the k3s cluster" )
    set( DESCRIPTION "gucefLabLink, the program of the service gucef-lablink. Set it up with: sudo ${SCRIPTS_DIR}/lablink/install.sh" )
    set( SETUP_SCRIPT "${SCRIPTS_DIR}/lablink/install.sh" )
    set( UNINSTALL_SCRIPT "${SCRIPTS_DIR}/lablink/uninstall.sh" )
    set( DEB_EXTRA_DEPENDS ", python3, iproute2, gucef-mcp (= ${VERSION})" )
    set( RPM_EXTRA_REQUIRES ", python3, iproute, gucef-mcp = ${VERSION}" )
  else()
    message( FATAL_ERROR "Unknown package ${GUCEF_PACKAGE}" )
  endif()

  set( DEB_ARCH "${GUCEF_PACKAGE_ARCH}" )
  if( NOT DEB_ARCH )
    execute_process( COMMAND dpkg --print-architecture OUTPUT_VARIABLE DEB_ARCH OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET )
  endif()
  if( DEB_ARCH STREQUAL "amd64" )
    set( RPM_ARCH "x86_64" )
  elseif( DEB_ARCH STREQUAL "arm64" )
    set( RPM_ARCH "aarch64" )
  else()
    message( FATAL_ERROR "Unknown architecture ${DEB_ARCH}: amd64 or arm64" )
  endif()
  set( FILE_ARCH "${DEB_ARCH}" )

  foreach( variable GUCEF_PACKAGE_DEB_LIBRARIES GUCEF_PACKAGE_RPM_LIBRARIES )
    if( NOT ${variable} )
      message( FATAL_ERROR "${variable} is not set" )
    endif()
  endforeach()
  set( DEB_DEPENDS "${GUCEF_PACKAGE_DEB_LIBRARIES}, ${SCRIPTS_PACKAGE} (= ${VERSION})${DEB_EXTRA_DEPENDS}" )
  set( RPM_REQUIRES "${GUCEF_PACKAGE_RPM_LIBRARIES}, ${SCRIPTS_PACKAGE} = ${VERSION}${RPM_EXTRA_REQUIRES}" )

  install( DIRECTORY "${GUCEF_PACKAGE_SOURCE}/" DESTINATION "lib/${GUCEF_PACKAGE}" USE_SOURCE_PERMISSIONS )

  # the same scripts serve dpkg (postinst configure, prerm remove) and rpm (%post 1|2, %preun 0)
  set( CONTROL_DIR "${CMAKE_CURRENT_BINARY_DIR}/control" )
  configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/postinst.in" "${CONTROL_DIR}/postinst" @ONLY
                  FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE )
  configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/prerm.in" "${CONTROL_DIR}/prerm" @ONLY
                  FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE )
  set( CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CONTROL_DIR}/postinst;${CONTROL_DIR}/prerm" )
  set( CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${CONTROL_DIR}/postinst" )
  set( CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE "${CONTROL_DIR}/prerm" )

endif()

#-------------------------------------------------------------------
# CPack
#-------------------------------------------------------------------
set( CPACK_PACKAGE_NAME "${GUCEF_PACKAGE}" )
set( CPACK_PACKAGE_VERSION "${VERSION}" )
set( CPACK_PACKAGE_VENDOR "GUCEF" )
set( CPACK_PACKAGE_CONTACT "Dinand Vanvelzen" )
set( CPACK_PACKAGE_DESCRIPTION_SUMMARY "${SUMMARY}" )
set( CPACK_PACKAGE_DESCRIPTION "${DESCRIPTION}" )
set( CPACK_PACKAGING_INSTALL_PREFIX "/usr" )
set( CPACK_PACKAGE_DIRECTORY "${GUCEF_PACKAGE_OUTPUT}" )
set( CPACK_STRIP_FILES FALSE )

set( CPACK_DEBIAN_FILE_NAME "${GUCEF_PACKAGE}-${VERSION}-${FILE_ARCH}.deb" )
set( CPACK_DEBIAN_PACKAGE_ARCHITECTURE "${DEB_ARCH}" )
set( CPACK_DEBIAN_PACKAGE_DEPENDS "${DEB_DEPENDS}" )
set( CPACK_DEBIAN_PACKAGE_SECTION "admin" )
set( CPACK_DEBIAN_PACKAGE_PRIORITY "optional" )
set( CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/LiberatorUSA/GUCEF" )
set( CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION TRUE )

set( CPACK_RPM_FILE_NAME "${GUCEF_PACKAGE}-${VERSION}-${FILE_ARCH}.rpm" )
set( CPACK_RPM_PACKAGE_ARCHITECTURE "${RPM_ARCH}" )
set( CPACK_RPM_PACKAGE_RELEASE "1" )
set( CPACK_RPM_PACKAGE_LICENSE "Apache-2.0" )
set( CPACK_RPM_PACKAGE_GROUP "System Environment/Daemons" )
set( CPACK_RPM_PACKAGE_URL "https://github.com/LiberatorUSA/GUCEF" )
set( CPACK_RPM_PACKAGE_REQUIRES "${RPM_REQUIRES}" )
# the requirements are the ones above: the libraries of the package are its own, not for other packages
set( CPACK_RPM_PACKAGE_AUTOREQPROV "no" )
set( CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/usr/lib;/usr/share" )
# the files are stripped already; build-id links of the libraries both program packages hold would conflict
set( CPACK_RPM_SPEC_MORE_DEFINE "%define _build_id_links none
%define debug_package %{nil}
%define __os_install_post %{nil}" )

set( CPACK_GENERATOR "DEB" )
find_program( RPMBUILD_PROGRAM rpmbuild )
if( RPMBUILD_PROGRAM )
  list( APPEND CPACK_GENERATOR "RPM" )
endif()

include( CPack )
