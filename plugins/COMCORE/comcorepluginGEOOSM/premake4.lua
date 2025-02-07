--------------------------------------------------------------------
-- This file was automatically generated by ProjectGenerator
-- which is tooling part the build system designed for GUCEF
--     (Galaxy Unlimited Framework)
-- For the latest info, see http://www.VanvelzenSoftware.com/
--
-- The contents of this file are placed in the public domain. Feel
-- free to make use of it in any way you like.
--------------------------------------------------------------------
--

-- Configuration for module: comcorepluginGEOOSM


project( "comcorepluginGEOOSM" )

configuration( {} )
  location( os.getenv( "PM4OUTPUTDIR" ) )

configuration( {} )
  targetdir( os.getenv( "PM4TARGETDIR" ) )

configuration( {} )
language( "C++" )



configuration( {} )
kind( "SharedLib" )

configuration( {} )
links( { "curl", "gucefCOMCORE", "gucefCORE", "gucefMT" } )
links( { "curl", "gucefCOMCORE", "gucefCORE", "gucefMT" } )


configuration( {} )
defines( { "COMCOREPLUGIN_GEOOSM_BUILD_PLUGIN_DLL" } )


configuration( {} )
vpaths { ["Headers"] = { "**.h", "**.hpp", "**.hxx" } }
files( {
  "include/comcorepluginGEOOSM.h",
  "include/comcorepluginGEOOSM_COsmGeoLocationLookupService.h",
  "include/comcorepluginGEOOSM_config.h",
  "include/comcorepluginGEOOSM_macros.h"
 } )



configuration( {} )
vpaths { ["Source"] = { "**.c", "**.cpp", "**.cs", "**.asm" } }
files( {
  "src/comcorepluginGEOOSM.cpp",
  "src/comcorepluginGEOOSM_COsmGeoLocationLookupService.cpp"
 } )


configuration( {} )
includedirs( { "../../../common/include", "../../../dependencies/curl/include", "../../../dependencies/curl/include/curl", "../../../dependencies/curl/lib", "../../../dependencies/curl/lib/vauth", "../../../dependencies/curl/lib/vquic", "../../../dependencies/curl/lib/vssh", "../../../dependencies/curl/lib/vtls", "../../../platform/gucefCOMCORE/include", "../../../platform/gucefCORE/include", "../../../platform/gucefCORE/include/c_api", "../../../platform/gucefMT/include", "include" } )

configuration( { "ANDROID" } )
includedirs( { "../../../platform/gucefCORE/include/android" } )

configuration( { "LINUX32" } )
includedirs( { "../../../platform/gucefCOMCORE/include/linux", "../../../platform/gucefCORE/include/linux" } )

configuration( { "LINUX64" } )
includedirs( { "../../../platform/gucefCOMCORE/include/linux", "../../../platform/gucefCORE/include/linux" } )

configuration( { "WIN32" } )
includedirs( { "../../../dependencies/curl/lib", "../../../platform/gucefCOMCORE/include/mswin", "../../../platform/gucefCORE/include/mswin", "../../../platform/gucefMT/include/mswin" } )

configuration( { "WIN64" } )
includedirs( { "../../../dependencies/curl/lib", "../../../platform/gucefCOMCORE/include/mswin", "../../../platform/gucefCORE/include/mswin", "../../../platform/gucefMT/include/mswin" } )
