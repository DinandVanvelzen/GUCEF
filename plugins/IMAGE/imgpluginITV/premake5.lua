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

-- Configuration for module: imgpluginITV


project( "imgpluginITV" )

configuration( {} )
  location( os.getenv( "PM5OUTPUTDIR" ) )

configuration( {} )
  targetdir( os.getenv( "PM5TARGETDIR" ) )

configuration( {} )
language( "C" )



configuration( {} )
kind( "SharedLib" )

configuration( {} )
links( { "gucefCORE", "gucefIMAGE", "gucefMT" } )


configuration( {} )
defines( { "GUCEF_CODECPLUGIN_BUILD_MODULE" } )


configuration( {} )
vpaths { ["Headers"] = { "**.h", "**.hpp", "**.hxx" } }
files( {
  "include/imgpluginITV.h"
 } )



configuration( {} )
vpaths { ["Source"] = { "**.c", "**.cpp", "**.cs", "**.asm" } }
files( {
  "src/imgpluginITV.c"
 } )


configuration( {} )
includedirs( { "../../../common/include", "../../../platform/gucefCORE/include", "../../../platform/gucefCORE/include/c_api", "../../../platform/gucefIMAGE/include", "../../../platform/gucefMT/include", "include" } )

configuration( { "ANDROID" } )
includedirs( { "../../../platform/gucefCORE/include/android" } )

configuration( { "LINUX32" } )
includedirs( { "../../../platform/gucefCORE/include/linux" } )

configuration( { "LINUX64" } )
includedirs( { "../../../platform/gucefCORE/include/linux" } )

configuration( { "WIN32" } )
includedirs( { "../../../platform/gucefCORE/include/mswin", "../../../platform/gucefMT/include/mswin" } )

configuration( { "WIN64" } )
includedirs( { "../../../platform/gucefCORE/include/mswin", "../../../platform/gucefMT/include/mswin" } )
