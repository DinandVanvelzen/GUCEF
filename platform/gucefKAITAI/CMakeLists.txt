#-------------------------------------------------------------------
# This file was automatically generated by ProjectGenerator
# which is tooling part the build system designed for GUCEF
#     (Galaxy Unlimited Framework)
# For the latest info, see http://www.VanvelzenSoftware.com/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------
#

# Configuration for module: gucefKAITAI

set( HEADER_FILES 
  includes/gucefKAITAI.h
  includes/gucefKAITAI_CGlobalKaitaiSchemaRegistry.h
  includes/gucefKAITAI_CKaitaiGlobal.h
  includes/gucefKAITAI_CKaitaiReader.h
  includes/gucefKAITAI_CKaitaiSchema.h
  includes/gucefKAITAI_CKaitaiSchemaEnumDefinition.h
  includes/gucefKAITAI_CKaitaiSchemaFieldDefinition.h
  includes/gucefKAITAI_CKaitaiSchemaRegistry.h
  includes/gucefKAITAI_CKaitaiSerializer.h
  includes/gucefKAITAI_CModule.h
  includes/gucefKAITAI_ETypes.h
  includes/gucefKAITAI_config.h
  includes/gucefKAITAI_macros.h
)

set( SOURCE_FILES 
  src/gucefKAITAI_CGlobalKaitaiSchemaRegistry.cpp
  src/gucefKAITAI_CKaitaiGlobal.cpp
  src/gucefKAITAI_CKaitaiReader.cpp
  src/gucefKAITAI_CKaitaiSchema.cpp
  src/gucefKAITAI_CKaitaiSchemaEnumDefinition.cpp
  src/gucefKAITAI_CKaitaiSchemaFieldDefinition.cpp
  src/gucefKAITAI_CKaitaiSchemaRegistry.cpp
  src/gucefKAITAI_CModule.cpp
)

# Make sure the PLATFORM_HEADER_FILES variable is always defined
 set( PLATFORM_HEADER_FILES "" )

# Make sure the PLATFORM_SOURCE_FILES variable is always defined
 set( PLATFORM_SOURCE_FILES "" )

# For ease of use make a variable that has all files for this module
set( ALL_FILES ${HEADER_FILES} ${SOURCE_FILES} ${PLATFORM_HEADER_FILES} ${PLATFORM_SOURCE_FILES} )

set( MODULE_NAME "gucefKAITAI" )
add_library( ${MODULE_NAME} ${ALL_FILES} )
add_dependencies( ${MODULE_NAME} gucefCORE gucefMT gucefVFS )
target_link_libraries( ${MODULE_NAME} gucefCORE gucefMT gucefVFS )
set_property( TARGET ${MODULE_NAME} APPEND_STRING PROPERTY COMPILE_DEFINITIONS "GUCEF_KAITAI_BUILD_MODULE" )

include_directories( ../../common/include ../gucefCORE/include ../gucefCORE/include/c_api ../gucefMT/include ../gucefVFS/include includes )

if ( ANDROID )
  include_directories( ../gucefCORE/include/android )
endif( ANDROID )

if ( LINUX32 )
  include_directories( ../gucefCORE/include/linux )
endif( LINUX32 )

if ( LINUX64 )
  include_directories( ../gucefCORE/include/linux )
endif( LINUX64 )

if ( WIN32 )
  include_directories( ../gucefCORE/include/mswin ../gucefMT/include/mswin )
endif( WIN32 )

if ( WIN64 )
  include_directories( ../gucefCORE/include/mswin ../gucefMT/include/mswin )
endif( WIN64 )



#******** START OF AUTOGENERATED TEMPLATED ADDITION CONTENT ********




if (ANDROID)
GUCEF_config_lib( ${MODULE_NAME} )
elseif (LINUX32)
GUCEF_config_lib( ${MODULE_NAME} )
elseif (LINUX64)
GUCEF_config_lib( ${MODULE_NAME} )
elseif (OSX)
GUCEF_config_lib( ${MODULE_NAME} )
elseif (WIN32)
GUCEF_config_lib( ${MODULE_NAME} )
elseif (WIN64)
GUCEF_config_lib( ${MODULE_NAME} )
else()
GUCEF_config_lib( ${MODULE_NAME} )
endif()



#******** END OF AUTOGENERATED TEMPLATED ADDITION CONTENT ********


