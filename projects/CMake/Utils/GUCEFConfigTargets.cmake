#-------------------------------------------------------------------
# This file is part of the CMake build system for GUCEF
#     (Galaxy Unlimited Framework)
# For the latest info, see http://www.VanvelzenSoftware.com/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure settings and install targets

if (WIN32)
  set(GUCEF_RELEASE_PATH "/Release")
  set(GUCEF_RELWDBG_PATH "/RelWithDebInfo")
  set(GUCEF_MINSIZE_PATH "/MinSizeRel")
  set(GUCEF_DEBUG_PATH "/Debug")
  set(GUCEF_LIB_RELEASE_PATH "/Release")
  set(GUCEF_LIB_RELWDBG_PATH "/RelWithDebInfo")
  set(GUCEF_LIB_MINSIZE_PATH "/MinSizeRel")
  set(GUCEF_LIB_DEBUG_PATH "/Debug")
  set(GUCEF_PLUGIN_PATH "/opt")
elseif (UNIX)
  set(GUCEF_RELEASE_PATH "")
  set(GUCEF_RELWDBG_PATH "")
  set(GUCEF_MINSIZE_PATH "")
  set(GUCEF_DEBUG_PATH "/debug")
  set(GUCEF_LIB_RELEASE_PATH "")
  set(GUCEF_LIB_RELWDBG_PATH "")
  set(GUCEF_LIB_MINSIZE_PATH "")
  set(GUCEF_LIB_DEBUG_PATH "")
  set(GUCEF_PLUGIN_PATH "/GUCEF")
  set(LINUX "1")
  option(GUCEF_USE_CLANG "build with clang" OFF) # OFF is the default
endif ()

# create vcproj.user file for Visual Studio to set debug working directory
function(GUCEF_create_vcproj_userfile TARGETNAME)
  if (MSVC)
    configure_file(
	  ${GUCEF_TEMPLATES_DIR}/VisualStudioUserFile.vcproj.user.in
	  ${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME}.vcproj.user
	  @ONLY
	)
  endif ()
endfunction(GUCEF_create_vcproj_userfile)

# install targets according to current build type
# Development files (headers, CMake package configuration) are only installed when asked for. By default an
# install holds what is needed to run the products. The generated module roots turn this on to build per module packages.
option(GUCEF_INSTALL_DEVELOPMENT_FILES "Install headers and CMake package configuration files" OFF)

function(GUCEF_install_target TARGETNAME SUFFIX)
  if (GUCEF_INSTALL_DEVELOPMENT_FILES)
    # A single rule which also adds the target to the export set of its package, see GUCEF_install_module_package()
    install(TARGETS ${TARGETNAME} EXPORT ${TARGETNAME}Targets
      RUNTIME DESTINATION "bin${SUFFIX}"
      LIBRARY DESTINATION "lib${SUFFIX}"
      ARCHIVE DESTINATION "lib${SUFFIX}"
      FRAMEWORK DESTINATION "bin"
    )
    return()
  endif ()
  install(TARGETS ${TARGETNAME}
    RUNTIME DESTINATION "bin${GUCEF_RELEASE_PATH}${SUFFIX}" CONFIGURATIONS Release None ""
    LIBRARY DESTINATION "lib${GUCEF_LIB_RELEASE_PATH}${SUFFIX}" CONFIGURATIONS Release None ""
    ARCHIVE DESTINATION "lib${GUCEF_LIB_RELEASE_PATH}${SUFFIX}" CONFIGURATIONS Release None ""
    FRAMEWORK DESTINATION "bin${GUCEF_RELEASE_PATH}" CONFIGURATIONS Release None ""
  )
  install(TARGETS ${TARGETNAME}
    RUNTIME DESTINATION "bin${GUCEF_RELWDBG_PATH}${SUFFIX}" CONFIGURATIONS RelWithDebInfo
    LIBRARY DESTINATION "lib${GUCEF_LIB_RELWDBG_PATH}${SUFFIX}" CONFIGURATIONS RelWithDebInfo
    ARCHIVE DESTINATION "lib${GUCEF_LIB_RELWDBG_PATH}${SUFFIX}" CONFIGURATIONS RelWithDebInfo
    FRAMEWORK DESTINATION "bin${GUCEF_RELWDBG_PATH}" CONFIGURATIONS RelWithDebInfo
  )
  install(TARGETS ${TARGETNAME}
    RUNTIME DESTINATION "bin${GUCEF_MINSIZE_PATH}${SUFFIX}" CONFIGURATIONS MinSizeRel
    LIBRARY DESTINATION "lib${GUCEF_LIB_MINSIZE_PATH}${SUFFIX}" CONFIGURATIONS MinSizeRel
    ARCHIVE DESTINATION "lib${GUCEF_LIB_MINSIZE_PATH}${SUFFIX}" CONFIGURATIONS MinSizeRel
    FRAMEWORK DESTINATION "bin${GUCEF_MINSIZE_PATH}" CONFIGURATIONS MinSizeRel
  )
  install(TARGETS ${TARGETNAME}
    RUNTIME DESTINATION "bin${GUCEF_DEBUG_PATH}${SUFFIX}" CONFIGURATIONS Debug
    LIBRARY DESTINATION "lib${GUCEF_LIB_DEBUG_PATH}${SUFFIX}" CONFIGURATIONS Debug
    ARCHIVE DESTINATION "lib${GUCEF_LIB_DEBUG_PATH}${SUFFIX}" CONFIGURATIONS Debug
    FRAMEWORK DESTINATION "bin${GUCEF_DEBUG_PATH}" CONFIGURATIONS Debug
  )
endfunction(GUCEF_install_target)

# setup common target settings
function(GUCEF_config_common TARGETNAME)
  set_target_properties(${TARGETNAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${GUCEF_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${GUCEF_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${GUCEF_BINARY_DIR}/bin
  )
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
  if(GUCEF_BUILD_PLATFORM_IPHONE)
    set_target_properties(${TARGETNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_THUMB_SUPPORT "NO")
    set_target_properties(${TARGETNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_UNROLL_LOOPS "YES")
    set_target_properties(${TARGETNAME} PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer")
    set_target_properties(${TARGETNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER "YES")
    set_target_properties(${TARGETNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_PREFIX_HEADER "${GUCEF_SOURCE_DIR}/GUCEFMain/include/GUCEFStableHeaders.h")
  endif(GUCEF_BUILD_PLATFORM_IPHONE)

  GUCEF_create_vcproj_userfile(${TARGETNAME})

  if (MSVC)
    add_definitions( "/D_CRT_SECURE_NO_WARNINGS /wd4251 /nologo" )
  endif()

endfunction(GUCEF_config_common)

# setup library build
function(GUCEF_config_lib LIBNAME)
  GUCEF_config_common(${LIBNAME})
  GUCEF_deploy_data_files(${LIBNAME})
  if (GUCEF_STATIC)
    # add static prefix, if compiling static version
    set_target_properties(${LIBNAME} PROPERTIES OUTPUT_NAME ${LIBNAME}Static)

    if(GUCEF_BUILD_PLATFORM_IPHONE)
      set_target_properties(${LIBNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_UNROLL_LOOPS "YES")
      set_target_properties(${LIBNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_THUMB_SUPPORT "NO")
      set_target_properties(${LIBNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER "YES")
      set_target_properties(${LIBNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_PREFIX_HEADER "${GUCEF_SOURCE_DIR}/GUCEFMain/include/GUCEFStableHeaders.h")
    endif(GUCEF_BUILD_PLATFORM_IPHONE)
  else (GUCEF_STATIC)
    if (CMAKE_COMPILER_IS_GNUCXX)
      # add GCC visibility flags to shared library build
      set_target_properties(${LIBNAME} PROPERTIES COMPILE_FLAGS "${GUCEF_GCC_VISIBILITY_FLAGS}")
	endif (CMAKE_COMPILER_IS_GNUCXX)

	# macOS builds produce regular libraries like every other platform, there is no framework bundle to assemble.
  endif (GUCEF_STATIC)
  GUCEF_install_target(${LIBNAME} "")

  if (GUCEF_INSTALL_PDB)
    # install debug pdb files
    if (GUCEF_STATIC)
	  install(FILES ${GUCEF_BINARY_DIR}/lib${GUCEF_LIB_DEBUG_PATH}/${LIBNAME}Static_d.pdb
	    DESTINATION lib${GUCEF_LIB_DEBUG_PATH}
		CONFIGURATIONS Debug
	  )
	  install(FILES ${GUCEF_BINARY_DIR}/lib${GUCEF_LIB_RELWDBG_PATH}/${LIBNAME}Static.pdb
	    DESTINATION lib${GUCEF_LIB_RELWDBG_PATH}
		CONFIGURATIONS RelWithDebInfo
	  )
	else ()
	  install(FILES ${GUCEF_BINARY_DIR}/bin${GUCEF_DEBUG_PATH}/${LIBNAME}_d.pdb
	    DESTINATION bin${GUCEF_DEBUG_PATH}
		CONFIGURATIONS Debug
	  )
	  install(FILES ${GUCEF_BINARY_DIR}/bin${GUCEF_RELWDBG_PATH}/${LIBNAME}.pdb
	    DESTINATION bin${GUCEF_RELWDBG_PATH}
		CONFIGURATIONS RelWithDebInfo
	  )
	endif ()
  endif ()
endfunction(GUCEF_config_lib)

# setup plugin build
function(GUCEF_config_plugin PLUGINNAME)
  GUCEF_config_common(${PLUGINNAME})
  GUCEF_deploy_data_files(${PLUGINNAME})
  if (GUCEF_STATIC)
    # add static prefix, if compiling static version
    set_target_properties(${PLUGINNAME} PROPERTIES OUTPUT_NAME ${PLUGINNAME}Static)

    if(GUCEF_BUILD_PLATFORM_IPHONE)
      set_target_properties(${PLUGINNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_THUMB_SUPPORT "NO")
      set_target_properties(${PLUGINNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_UNROLL_LOOPS "YES")
      set_target_properties(${PLUGINNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER "YES")
      set_target_properties(${PLUGINNAME} PROPERTIES XCODE_ATTRIBUTE_GCC_PREFIX_HEADER "${GUCEF_SOURCE_DIR}/GUCEFMain/include/GUCEFStableHeaders.h")
    endif(GUCEF_BUILD_PLATFORM_IPHONE)
  else (GUCEF_STATIC)
    if (CMAKE_COMPILER_IS_GNUCXX)
      # add GCC visibility flags to shared library build
      set_target_properties(${PLUGINNAME} PROPERTIES COMPILE_FLAGS "${GUCEF_GCC_VISIBILITY_FLAGS}")
      # disable "lib" prefix on Unix
      set_target_properties(${PLUGINNAME} PROPERTIES PREFIX "")
	endif (CMAKE_COMPILER_IS_GNUCXX)
  endif (GUCEF_STATIC)
  GUCEF_install_target(${PLUGINNAME} ${GUCEF_PLUGIN_PATH})

  if (GUCEF_INSTALL_PDB)
    # install debug pdb files
    if (GUCEF_STATIC)
	  install(FILES ${GUCEF_BINARY_DIR}/lib${GUCEF_LIB_DEBUG_PATH}/${PLUGINNAME}Static_d.pdb
	    DESTINATION lib${GUCEF_LIB_DEBUG_PATH}/opt
		CONFIGURATIONS Debug
	  )
	  install(FILES ${GUCEF_BINARY_DIR}/lib${GUCEF_LIB_RELWDBG_PATH}/${PLUGINNAME}Static.pdb
	    DESTINATION lib${GUCEF_LIB_RELWDBG_PATH}/opt
		CONFIGURATIONS RelWithDebInfo
	  )
	else ()
	  install(FILES ${GUCEF_BINARY_DIR}/bin${GUCEF_DEBUG_PATH}/${PLUGINNAME}_d.pdb
	    DESTINATION bin${GUCEF_DEBUG_PATH}
		CONFIGURATIONS Debug
	  )
	  install(FILES ${GUCEF_BINARY_DIR}/bin${GUCEF_RELWDBG_PATH}/${PLUGINNAME}.pdb
	    DESTINATION bin${GUCEF_RELWDBG_PATH}
		CONFIGURATIONS RelWithDebInfo
	  )
	endif ()
  endif ()
endfunction(GUCEF_config_plugin)

# setup GUCEF demo build
function(GUCEF_config_sample SAMPLENAME)
  # The PRODUCT_NAME target setting cannot contain underscores.  Just remove them
  # Known bug in Xcode CFBundleIdentifier processing rdar://6187020
  # Can cause an instant App Store rejection. Also, code signing will fail.
  #if (GUCEF_BUILD_PLATFORM_IPHONE)
#    string (REPLACE "_" "" SAMPLENAME ${SAMPLENAME})
  #endif()
  GUCEF_config_common(${SAMPLENAME})

  # set install RPATH for Unix systems
  if (UNIX AND GUCEF_FULL_RPATH)
    set_property(TARGET ${SAMPLENAME} APPEND PROPERTY
      INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)
    set_property(TARGET ${SAMPLENAME} PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)
  endif ()

  if (APPLE)
    # On OS X, create .app bundle
    set_property(TARGET ${SAMPLENAME} PROPERTY MACOSX_BUNDLE TRUE)

	if (GUCEF_BUILD_PLATFORM_IPHONE)
      set (GUCEF_SAMPLE_CONTENTS_PATH
        ${CMAKE_BINARY_DIR}/bin/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)/$(PRODUCT_NAME).app)
      add_custom_command(TARGET ${SAMPLENAME} POST_BUILD
        COMMAND cp ARGS ${CMAKE_BINARY_DIR}/bin/*.cfg ${GUCEF_SAMPLE_CONTENTS_PATH}/
	    COMMAND /Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp -exclude .DS_Store -exclude CVS -exclude .svn -exclude 'CMakeLists.txt' -resolve-src-symlinks ${GUCEF_SOURCE_DIR}/Samples/Media ${GUCEF_SAMPLE_CONTENTS_PATH}/
	    COMMAND /Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp -exclude .DS_Store -exclude CVS -exclude .svn -exclude 'CMakeLists.txt' -resolve-src-symlinks ${GUCEF_SOURCE_DIR}/Samples/Common/misc/Icon.png ${GUCEF_SAMPLE_CONTENTS_PATH}/
	    COMMAND /Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp -exclude .DS_Store -exclude CVS -exclude .svn -exclude 'CMakeLists.txt' -resolve-src-symlinks ${GUCEF_SOURCE_DIR}/Samples/Common/misc/Default.png ${GUCEF_SAMPLE_CONTENTS_PATH}/
      )
	else ()
      # also, symlink frameworks so .app is standalone
      # NOTE: $(CONFIGURATION) is not resolvable at CMake run time, it's only
      # valid at build time (hence parenthesis rather than braces)
      set (GUCEF_SAMPLE_CONTENTS_PATH
        ${CMAKE_BINARY_DIR}/bin/$(CONFIGURATION)/${SAMPLENAME}.app/Contents)
      add_custom_command(TARGET ${SAMPLENAME} POST_BUILD
        COMMAND mkdir ARGS -p ${GUCEF_SAMPLE_CONTENTS_PATH}/Frameworks
        COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/GUCEF.framework
          ${GUCEF_SAMPLE_CONTENTS_PATH}/Frameworks/
        COMMAND ln ARGS -s -f ${CMAKE_SOURCE_DIR}/Dependencies/Cg.framework
          ${GUCEF_SAMPLE_CONTENTS_PATH}/Frameworks/
        COMMAND ln ARGS -s -f ${CMAKE_SOURCE_DIR}/Dependencies/CEGUI.framework
          ${GUCEF_SAMPLE_CONTENTS_PATH}/Frameworks/
      )
      # now cfg files
      add_custom_command(TARGET ${SAMPLENAME} POST_BUILD
        COMMAND mkdir ARGS -p ${GUCEF_SAMPLE_CONTENTS_PATH}/Resources
        COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/bin/plugins.cfg
          ${GUCEF_SAMPLE_CONTENTS_PATH}/Resources/
        COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/bin/resources.cfg
          ${GUCEF_SAMPLE_CONTENTS_PATH}/Resources/
        COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/bin/media.cfg
          ${GUCEF_SAMPLE_CONTENTS_PATH}/Resources/
        COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/bin/quake3settings.cfg
          ${GUCEF_SAMPLE_CONTENTS_PATH}/Resources/
      )
    endif()
  endif (APPLE)

  if (GUCEF_INSTALL_SAMPLES)
    GUCEF_install_target(${SAMPLENAME} "")
    if (GUCEF_INSTALL_PDB)
      # install debug pdb files
      install(FILES ${GUCEF_BINARY_DIR}/bin${GUCEF_DEBUG_PATH}/${SAMPLENAME}.pdb
        DESTINATION bin${GUCEF_DEBUG_PATH} CONFIGURATIONS Debug
        )
      install(FILES ${GUCEF_BINARY_DIR}/bin${GUCEF_RELWDBG_PATH}/${SAMPLENAME}.pdb
        DESTINATION bin${GUCEF_RELWDBG_PATH} CONFIGURATIONS RelWithDebInfo
        )
    endif ()
  endif ()

endfunction(GUCEF_config_sample)

# The config files of an app or tool, <module dir>/config/<target>*.ini and <target>*.json, are copied next to its
# binary after each build: that is where the app looks for them ($MODULEDIR$). Other files in that dir, like examples
# in sub dirs, are left alone. A config file added later is picked up when CMake runs again
function(GUCEF_deploy_config_files TARGETNAME)
  file(GLOB _configFiles LIST_DIRECTORIES false
    "${CMAKE_CURRENT_SOURCE_DIR}/config/${TARGETNAME}*.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/config/${TARGETNAME}*.json"
    )
  if (_configFiles)
    add_custom_command(TARGET ${TARGETNAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_configFiles} $<TARGET_FILE_DIR:${TARGETNAME}>
      COMMENT "Copying the config files of ${TARGETNAME} next to it"
      VERBATIM
      )
  endif ()
endfunction(GUCEF_deploy_config_files)

# The data files of a module, <module dir>/data, are copied to <build dir>/data/<target>. Next to the bin dir is where
# the module finds them at run time: $MODULEDIR$/../data/<target>. The copy is a target of its own, <target>_DataFiles,
# that the module depends on and that depends on every data file: a change to a data file alone copies them again,
# without a rebuild of the module (a data file added or removed is found at build time, CONFIGURE_DEPENDS)
function(GUCEF_deploy_data_files TARGETNAME)
  if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/data")
    set_target_properties(${TARGETNAME} PROPERTIES GUCEF_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data")
    file(GLOB_RECURSE _dataFiles LIST_DIRECTORIES false CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/data/*")
    set(_stamp "${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME}_DataFiles.stamp")
    add_custom_command(OUTPUT "${_stamp}"
      COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/data" "${GUCEF_BINARY_DIR}/data/${TARGETNAME}"
      COMMAND ${CMAKE_COMMAND} -E touch "${_stamp}"
      DEPENDS ${_dataFiles}
      COMMENT "Copying the data files of ${TARGETNAME} to the data dir"
      VERBATIM
      )
    add_custom_target(${TARGETNAME}_DataFiles DEPENDS "${_stamp}")
    add_dependencies(${TARGETNAME} ${TARGETNAME}_DataFiles)
  endif ()
endfunction(GUCEF_deploy_data_files)

# Collects the data dirs of a target and of the targets it links, directly or through others
function(GUCEF_collect_data_dirs TARGETNAME VISITEDVAR DATADIRSVAR)
  set(_visited ${${VISITEDVAR}})
  set(_dataDirs ${${DATADIRSVAR}})
  if (TARGET ${TARGETNAME} AND NOT "${TARGETNAME}" IN_LIST _visited)
    list(APPEND _visited ${TARGETNAME})
    get_target_property(_dataDir ${TARGETNAME} GUCEF_DATA_DIR)
    if (_dataDir)
      list(APPEND _dataDirs "${_dataDir}@/data/${TARGETNAME}")
    endif ()
    get_target_property(_linked ${TARGETNAME} LINK_LIBRARIES)
    if (_linked)
      foreach(_dependency ${_linked})
        GUCEF_collect_data_dirs(${_dependency} _visited _dataDirs)
      endforeach ()
    endif ()
  endif ()
  set(${VISITEDVAR} ${_visited} PARENT_SCOPE)
  set(${DATADIRSVAR} ${_dataDirs} PARENT_SCOPE)
endfunction(GUCEF_collect_data_dirs)

# A web app has no files next to it: the data of the modules it links is preloaded into its virtual file system at
# /data/<module>. Runs once every target is known, at the end of the configuration
function(GUCEF_preload_data_files TARGETNAME)
  set(_visited "")
  set(_dataDirs "")
  GUCEF_collect_data_dirs(${TARGETNAME} _visited _dataDirs)
  foreach(_dataDir ${_dataDirs})
    target_link_options(${TARGETNAME} PRIVATE "SHELL:--preload-file ${_dataDir}")
  endforeach ()
endfunction(GUCEF_preload_data_files)

# The data files an app or tool needs: those of the modules it links, on the web preloaded into the app. A web build that
# node runs (GUCEF_EMSCRIPTEN_LINK_NODE_FLAGS, the real file system) reads them where they are copied, as a native one does
function(GUCEF_deploy_app_data_files TARGETNAME)
  GUCEF_deploy_data_files(${TARGETNAME})
  if (EMSCRIPTEN AND NOT GUCEF_EMSCRIPTEN_LINK_NODE_FLAGS)
    # the arguments of a deferred call are evaluated when it runs: the target name is put in now
    cmake_language(EVAL CODE "cmake_language(DEFER DIRECTORY [[${CMAKE_SOURCE_DIR}]] CALL GUCEF_preload_data_files [[${TARGETNAME}]])")
  endif ()
endfunction(GUCEF_deploy_app_data_files)

# setup GUCEF tool build
function(GUCEF_config_tool TOOLNAME)
  GUCEF_config_common(${TOOLNAME})
  GUCEF_deploy_config_files(${TOOLNAME})
  GUCEF_deploy_app_data_files(${TOOLNAME})

  # set install RPATH for Unix systems
  if (UNIX AND GUCEF_FULL_RPATH)
    set_property(TARGET ${TOOLNAME} APPEND PROPERTY
      INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)
    set_property(TARGET ${TOOLNAME} PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)
  endif ()

  if (GUCEF_INSTALL_TOOLS)
    GUCEF_install_target(${TOOLNAME} "")
    if (GUCEF_INSTALL_PDB)
      # install debug pdb files
      install(FILES ${GUCEF_BINARY_DIR}/bin${GUCEF_DEBUG_PATH}/${TOOLNAME}.pdb
        DESTINATION bin${GUCEF_DEBUG_PATH}
        CONFIGURATIONS Debug
        )
      install(FILES ${GUCEF_BINARY_DIR}/bin${GUCEF_RELWDBG_PATH}/${TOOLNAME}.pdb
        DESTINATION bin${GUCEF_RELWDBG_PATH}
        CONFIGURATIONS RelWithDebInfo
        )
    endif ()
  endif ()

endfunction(GUCEF_config_tool)

# setup GUCEF tool build
function(GUCEF_config_app APPNAME)
  GUCEF_config_common(${APPNAME})
  GUCEF_deploy_config_files(${APPNAME})
  GUCEF_deploy_app_data_files(${APPNAME})

  # set install RPATH for Unix systems
  if (UNIX AND GUCEF_FULL_RPATH)
    set_property(TARGET ${APPNAME} APPEND PROPERTY
      INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)
    set_property(TARGET ${APPNAME} PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)
  endif ()

    GUCEF_install_target(${APPNAME} "")
    if (GUCEF_INSTALL_PDB)
      # install debug pdb files
      install(FILES ${GUCEF_BINARY_DIR}/bin${GUCEF_DEBUG_PATH}/${APPNAME}.pdb
        DESTINATION bin${GUCEF_DEBUG_PATH}
        CONFIGURATIONS Debug
        )
      install(FILES ${GUCEF_BINARY_DIR}/bin${GUCEF_RELWDBG_PATH}/${APPNAME}.pdb
        DESTINATION bin${GUCEF_RELWDBG_PATH}
        CONFIGURATIONS RelWithDebInfo
        )
    endif ()

endfunction(GUCEF_config_app)

# Declares the include dirs a module offers to its dependents as usage requirements of its target, so they reach
# dependents in a build of the source tree and from an installed build alike. INCLUDE_DIRS are relative to BASE_DIR,
# which defaults to the current source dir, "." being BASE_DIR itself. With GUCEF_INSTALL_DEVELOPMENT_FILES the
# headers in these dirs are installed to include/<target>/<dir>, which is where the installed target points to.
function(GUCEF_module_usage TARGETNAME)
  cmake_parse_arguments(USAGE "" "BASE_DIR" "INCLUDE_DIRS" ${ARGN})
  if (NOT USAGE_BASE_DIR)
    set(USAGE_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
  endif ()
  foreach (INCLUDE_DIR ${USAGE_INCLUDE_DIRS})
    if (INCLUDE_DIR STREQUAL ".")
      set(SOURCE_INCLUDE_DIR "${USAGE_BASE_DIR}")
      set(INSTALL_INCLUDE_DIR "include/${TARGETNAME}")
    else ()
      set(SOURCE_INCLUDE_DIR "${USAGE_BASE_DIR}/${INCLUDE_DIR}")
      set(INSTALL_INCLUDE_DIR "include/${TARGETNAME}/${INCLUDE_DIR}")
    endif ()
    target_include_directories(${TARGETNAME} INTERFACE
      $<BUILD_INTERFACE:${SOURCE_INCLUDE_DIR}>
      $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>
    )
    if (GUCEF_INSTALL_DEVELOPMENT_FILES)
      install(DIRECTORY "${SOURCE_INCLUDE_DIR}/" DESTINATION "${INSTALL_INCLUDE_DIR}"
        FILES_MATCHING
          PATTERN "*.h" PATTERN "*.hh" PATTERN "*.hpp" PATTERN "*.hxx"
          PATTERN "*.inl" PATTERN "*.ipp" PATTERN "*.tcc" PATTERN "*.txx"
      )
    endif ()
  endforeach ()
endfunction(GUCEF_module_usage)

# Connects a module to dependencies of which it only uses headers. HEADER_MODULES (modules of type
# HeaderIncludeLocation) become PUBLIC usage requirements. INCLUDE_ONLY_MODULES, library modules it does not link,
# lend it their include dirs PRIVATE. Only dependencies which are targets in this build are connected: in a build of
# the source tree a module already gets these include dirs through its include_directories, in a build of a single
# module the targets come from the packages of its dependencies.
function(GUCEF_module_uses TARGETNAME)
  cmake_parse_arguments(USES "" "" "HEADER_MODULES;INCLUDE_ONLY_MODULES" ${ARGN})
  foreach (MODULE ${USES_HEADER_MODULES})
    if (TARGET ${MODULE})
      target_link_libraries(${TARGETNAME} PUBLIC ${MODULE})
    endif ()
  endforeach ()
  foreach (MODULE ${USES_INCLUDE_ONLY_MODULES})
    if (TARGET ${MODULE})
      target_include_directories(${TARGETNAME} PRIVATE $<TARGET_PROPERTY:${MODULE},INTERFACE_INCLUDE_DIRECTORIES>)
    endif ()
  endforeach ()
endfunction(GUCEF_module_uses)

# Installs the CMake package of a module: the export set of its target and a <target>Config.cmake which first finds
# the packages of the DEPENDENCIES. Used by the generated module roots together with GUCEF_INSTALL_DEVELOPMENT_FILES.
function(GUCEF_install_module_package TARGETNAME)
  cmake_parse_arguments(PACKAGE "" "" "DEPENDENCIES" ${ARGN})
  set(CONFIG_CONTENT "include(CMakeFindDependencyMacro)\n")
  foreach (DEPENDENCY ${PACKAGE_DEPENDENCIES})
    string(APPEND CONFIG_CONTENT "find_dependency(${DEPENDENCY} CONFIG)\n")
  endforeach ()
  string(APPEND CONFIG_CONTENT "include(\"\${CMAKE_CURRENT_LIST_DIR}/${TARGETNAME}Targets.cmake\")\n")
  set(CONFIG_FILE "${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME}Config.cmake")
  file(WRITE "${CONFIG_FILE}" "${CONFIG_CONTENT}")
  install(EXPORT ${TARGETNAME}Targets FILE ${TARGETNAME}Targets.cmake DESTINATION "lib/cmake/${TARGETNAME}")
  install(FILES "${CONFIG_FILE}" DESTINATION "lib/cmake/${TARGETNAME}")
endfunction(GUCEF_install_module_package)

# Sets RESULT_VAR to TRUE when CMAKE_CXX_STANDARD asks for a C++ standard below MIN_CPP_STANDARD, else to FALSE.
# Standards are given by their short name: 98, 03, 11, 14, 17, 20, 23 ... Without CMAKE_CXX_STANDARD the compiler
# default is used and the result is FALSE.
function(GUCEF_cpp_standard_is_below MIN_CPP_STANDARD RESULT_VAR)
  set(${RESULT_VAR} FALSE PARENT_SCOPE)
  if (NOT CMAKE_CXX_STANDARD OR NOT MIN_CPP_STANDARD)
    return()
  endif ()
  foreach (STANDARD_VAR CMAKE_CXX_STANDARD MIN_CPP_STANDARD)
    set(STANDARD_YEAR "${${STANDARD_VAR}}")
    if (STANDARD_YEAR GREATER_EQUAL 90)
      math(EXPR STANDARD_YEAR "1900 + ${STANDARD_YEAR}")
    else ()
      math(EXPR STANDARD_YEAR "2000 + ${STANDARD_YEAR}")
    endif ()
    set(${STANDARD_VAR}_YEAR ${STANDARD_YEAR})
  endforeach ()
  if (CMAKE_CXX_STANDARD_YEAR LESS MIN_CPP_STANDARD_YEAR)
    set(${RESULT_VAR} TRUE PARENT_SCOPE)
  endif ()
endfunction(GUCEF_cpp_standard_is_below)

# Fails the configuration when CMAKE_CXX_STANDARD asks for a C++ standard below MIN_CPP_STANDARD, the minimum C++ standard
# the module and the modules it depends on need.
# Used by the generated module roots: a build for a given C++ standard should not silently build at a higher one.
function(GUCEF_check_min_cpp_standard TARGETNAME MIN_CPP_STANDARD)
  GUCEF_cpp_standard_is_below("${MIN_CPP_STANDARD}" STANDARD_TOO_LOW)
  if (STANDARD_TOO_LOW)
    message(FATAL_ERROR "${TARGETNAME} needs at least C++${MIN_CPP_STANDARD} but CMAKE_CXX_STANDARD is ${CMAKE_CXX_STANDARD}")
  endif ()
endfunction(GUCEF_check_min_cpp_standard)

# Stands in for a module which is not built because CMAKE_CXX_STANDARD is below MIN_CPP_STANDARD, the minimum C++
# standard the module and the modules it depends on need. The GUCEF modules of one build share an ABI which depends on
# the C++ standard, so the module is not built with a raised standard. The empty target keeps dependencies on the
# module as a runtime plugin resolvable; modules linking it have the same or a higher minimum and are skipped as well.
function(GUCEF_add_skipped_module TARGETNAME MIN_CPP_STANDARD)
  message(STATUS "Skipping ${TARGETNAME}: it needs at least C++${MIN_CPP_STANDARD} and CMAKE_CXX_STANDARD is ${CMAKE_CXX_STANDARD}")
  add_custom_target(${TARGETNAME})
  set_property(GLOBAL APPEND PROPERTY GUCEF_SKIPPED_MODULES ${TARGETNAME})
endfunction(GUCEF_add_skipped_module)
