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

-- Configuration for module: MsmqMetrics


configuration( { "WIN32" } )
  project( "MsmqMetrics" )

configuration( { "WIN64" } )
  project( "MsmqMetrics" )

configuration( {} )
  location( os.getenv( "PM4OUTPUTDIR" ) )

configuration( {} )
  targetdir( os.getenv( "PM4TARGETDIR" ) )

configuration( {} )
language( "C" )

configuration( { "WIN32" } )
language( "C++" )

configuration( { "WIN64" } )
language( "C++" )

configuration( { "WIN32" } )


configuration( { WIN32 } )
kind( "WindowedApp" )
configuration( { "WIN64" } )


configuration( { WIN64 } )
kind( "WindowedApp" )
  
configuration( { WIN32 } )
links( { "gucefCOM", "gucefCOMCORE", "gucefCORE", "gucefMT", "gucefVFS", "gucefWEB" } )
  links( { "Mqrt.lib", "gucefCOM", "gucefCOMCORE", "gucefCORE", "gucefMT", "gucefVFS", "gucefWEB" } )
  
configuration( { WIN64 } )
links( { "gucefCOM", "gucefCOMCORE", "gucefCORE", "gucefMT", "gucefVFS", "gucefWEB" } )
  links( { "Mqrt.lib", "gucefCOM", "gucefCOMCORE", "gucefCORE", "gucefMT", "gucefVFS", "gucefWEB" } )


configuration( { "WIN32" } )
    vpaths { ["Platform Headers"] = { "**.h", "**.hpp", "**.hxx" } }
    files( {
      "include/msmqMetrics.h"
    } )

    vpaths { ["Platform Source"] = { "**.c", "**.cpp", "**.cs", "**.asm" } }
    files( {
      "src/main.cpp",
      "src/msmqMetrics.cpp"
    } )



configuration( { "WIN64" } )
    vpaths { ["Platform Headers"] = { "**.h", "**.hpp", "**.hxx" } }
    files( {
      "include/msmqMetrics.h"
    } )

    vpaths { ["Platform Source"] = { "**.c", "**.cpp", "**.cs", "**.asm" } }
    files( {
      "src/main.cpp",
      "src/msmqMetrics.cpp"
    } )


configuration( {} )
includedirs( { "../../common/include", "../../platform/gucefCOM/include", "../../platform/gucefCOMCORE/include", "../../platform/gucefCORE/include", "../../platform/gucefCORE/include/c_api", "../../platform/gucefMT/include", "../../platform/gucefVFS/include", "../../platform/gucefWEB/include" } )

configuration( { "WIN32" } )
includedirs( { "../../platform/gucefCOMCORE/include/mswin", "../../platform/gucefCORE/include/mswin", "../../platform/gucefMT/include/mswin", "include" } )

configuration( { "WIN64" } )
includedirs( { "../../platform/gucefCOMCORE/include/mswin", "../../platform/gucefCORE/include/mswin", "../../platform/gucefMT/include/mswin", "include" } )
