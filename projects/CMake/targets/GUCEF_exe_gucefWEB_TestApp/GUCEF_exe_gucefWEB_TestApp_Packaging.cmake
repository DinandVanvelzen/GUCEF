
#------------------------------------------------------------------------------
# CPACK
include(InstallRequiredSystemLibraries)

set(CPACK_MONOLITHIC_INSTALL 1)
set(CPACK_PACKAGE_VENDOR "Vanvelzen LLC")
set(CPACK_PACKAGE_NAME gucefWEB_TestApp )
set(CPACK_PACKAGE_CONTACT "Dinand Vanvelzen <dinand@vanvelzen.us>")
set(HOMEPAGE "https://github.com/LiberatorUSA/GUCEF")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "This is a description")
#set(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/readme.md")
#set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/license.md")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_EXT_ENABLE_STAGING 1)

set(CPACK_PACKAGE_INSTALL_DIRECTORY "gucefWEB_TestApp")
set(CPACK_PACKAGE_DIRECTORY "${GUCEF_BINARY_DIR}/packages")
set(CPACK_WARN_ON_ABSOLUTE_INSTALL_DESTINATION TRUE)
set(CPACK_PACKAGE_EXECUTABLES "GUCEF" "GUCEF")
set(CPACK_CREATE_DESKTOP_LINKS "GUCEF")
set(CPACK_STRIP_FILES FALSE )

if(WIN32 AND NOT UNIX)
    #--------------------------------------------------------------------------
    # Windows specific
    set(CPACK_GENERATOR "STGZ;ZIP;External")
    message(STATUS "Package generation - Windows")
    message(STATUS "   + STGZ                                 YES ")
    message(STATUS "   + ZIP                                  YES ")
    message(STATUS "   + External                             YES ")

    # NSIS windows installer
    find_program(NSIS_PATH nsis PATH_SUFFIXES nsis)
    if(NSIS_PATH)
        set(CPACK_GENERATOR "${CPACK_GENERATOR};NSIS")
        message(STATUS "   + NSIS                                 YES ")
        # Note: There is a bug in NSI that does not handle full unix paths properly. Make
        # sure there is at least one set of four (4) backlasshes.
        set(CPACK_NSIS_DISPLAY_NAME ${CPACK_PACKAGE_NAME})
        # Icon of the installer
#        set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}\\\\exampleApp.ico")
        # set(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.my-project-home-page.org")
        # set(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.my-personal-home-page.com")
        set(CPACK_NSIS_CONTACT "${CPACK_PACKAGE_CONTACT}")
        set(CPACK_NSIS_MODIFY_PATH ON)
    else()
        message(STATUS "   + NSIS                                 NO ")
    endif()

#    set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}\\\\exampleApp.png")

    # Configure file with right path, place the result to PROJECT_BINARY_DIR.
    # When ${PROJECT_BINARY_DIR}/exampleApp.icon.rc is added to an executable
    # it will have icon specified in exampleApp.icon.in.rc
#    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/exampleApp.icon.in.rc
#        ${PROJECT_BINARY_DIR}/exampleApp.icon.rc)

elseif(APPLE)
    #--------------------------------------------------------------------------
    # Apple specific
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_PACKAGING_INSTALL_PREFIX "/")

    set(MACOSX_BUNDLE_BUNDLE_NAME ${CPACK_PACKAGE_NAME})
    set(MACOSX_BUNDLE_BUNDLE_GUI_IDENTIFIER "com.exampleApp.exampleApp")
    set(MACOSX_BUNDLE_ICON_FILE ${PROJECT_SOURCE_DIR}/packaging/ exampleApp.icns)
    set(MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/packaging/MacOSXBundleInfo.plist.in)
    set(MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
    
    set_source_files_properties(${PROJECT_SOURCE_DIR}/packaging/exampleApp.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    
    set(CPACK_DMG_VOLUME_NAME "exampleApp")
    set(CPACK_DMG_DS_STORE_SETUP_SCRIPT "${PROJECT_SOURCE_DIR}/packaging/DS_Store.scpt") 
    set(CPACK_DMG_BACKGROUND_IMAGE "${PROJECT_SOURCE_DIR}/packaging/dmg_background.png")
    set(CPACK_OSX_PACKAGE_VERSION "10.6") #min package version
    
else()
    #-----------------------------------------------------------------------------
    # Linux specific
    set(CPACK_GENERATOR "DEB;TBZ2;TXZ;External")
    message(STATUS "Package generation - UNIX")
    message(STATUS "   + DEB                                  YES ")
    message(STATUS "   + TBZ2                                 YES ")
    message(STATUS "   + TXZ                                  YES ")
    message(STATUS "   + External                             YES ")

    find_program(RPMBUILD_PATH rpmbuild)
    if(RPMBUILD_PATH)
        message(STATUS "   + RPM                                  YES ")
        set(CPACK_GENERATOR "${CPACK_GENERATOR};RPM")
        set(CPACK_RPM_PACKAGE_LICENSE "MIT")
        # set(CPACK_RPM_PACKAGE_REQUIRES "gtkmm30")
        # exclude folders which clash with default ones
        set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST
            ${CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST}
            /usr
            /usr/bin
            /usr/share
            /usr/share/applications
            /usr/share/doc
            /usr/share/icons
            /usr/share/icons/hicolor
            /usr/share/icons/hicolor/256x256
            /usr/share/icons/hicolor/256x256/apps
            /usr/share/icons/gnome
            /usr/share/icons/gnome/256x256
            /usr/share/icons/gnome/256x256/apps)
    else()
        message(STATUS "   + RPM                                  NO ")
    endif()

    IF(NOT CPACK_DEBIAN_PACKAGE_ARCHITECTURE)
      FIND_PROGRAM(DPKG_CMD dpkg)
      IF(NOT DPKG_CMD)
        MESSAGE(STATUS "Can not find dpkg in your path, default to amd64.")
        SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE amd64)
      ENDIF(NOT DPKG_CMD)
        EXECUTE_PROCESS(COMMAND "${DPKG_CMD}" --print-architecture
            OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    ENDIF(NOT CPACK_DEBIAN_PACKAGE_ARCHITECTURE)

    set(CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION TRUE)
    set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "${HOMEPAGE}")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Dinand Vanvelzen")
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "DEB test description")
    set(CPACK_DEBIAN_COMPRESSION_TYPE "gzip")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
    # set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgtkmm-3.0")



    # Icon and app shortcut for Linux systems
    # Note: .desktop file must have same name as executable
#    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/exampleApp.desktop
#        DESTINATION share/applications/
#        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
#        )
#    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/exampleApp.png
#        DESTINATION share/icons/hicolor/256x256/apps/
#        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
#        )
#    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/exampleApp.png
#        DESTINATION share/icons/gnome/256x256/apps/
#        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
#        )
    # License file
#    install(FILES ${PROJECT_SOURCE_DIR}/license.md
#        DESTINATION share/doc/${PROJECT_NAME}/
#        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
#        RENAME copyright)
    # set package icon
#    set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/exampleApp.png")
endif()

#------------------------------------------------------------------------------
# and include CPack, so we get target for packages
include(CPack)
