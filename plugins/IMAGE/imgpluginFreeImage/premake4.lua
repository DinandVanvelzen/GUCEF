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

-- Configuration for module: imgpluginFreeImage


configuration( { "LINUX32" } )
  project( "imgpluginFreeImage" )

configuration( { "LINUX64" } )
  project( "imgpluginFreeImage" )

configuration( { "WIN32" } )
  project( "imgpluginFreeImage" )

configuration( { "WIN64" } )
  project( "imgpluginFreeImage" )

configuration( {} )
  location( os.getenv( "PM4OUTPUTDIR" ) )

configuration( {} )
  targetdir( os.getenv( "PM4TARGETDIR" ) )

configuration( {} )
language( "C" )

configuration( { "LINUX32" } )


configuration( { LINUX32 } )
kind( "SharedLib" )
configuration( { "LINUX64" } )


configuration( { LINUX64 } )
kind( "SharedLib" )
configuration( { "WIN32" } )


configuration( { WIN32 } )
kind( "SharedLib" )
configuration( { "WIN64" } )


configuration( { WIN64 } )
kind( "SharedLib" )
  
configuration( { LINUX32 } )
links( { "FreeImage", "gucefCORE", "gucefIMAGE", "gucefMT" } )
  links( { "FreeImage" } )
  

configuration( { LINUX32 } )
defines( { "GUCEF_CODECPLUGIN_BUILD_MODULE" } )
  
configuration( { LINUX64 } )
links( { "FreeImage", "gucefCORE", "gucefIMAGE", "gucefMT" } )
  links( { "FreeImage" } )
  

configuration( { LINUX64 } )
defines( { "GUCEF_CODECPLUGIN_BUILD_MODULE" } )
  
configuration( { WIN32 } )
links( { "FreeImage", "gucefCORE", "gucefIMAGE", "gucefMT" } )
  links( { "FreeImage" } )
  

configuration( { WIN32 } )
defines( { "GUCEF_CODECPLUGIN_BUILD_MODULE" } )
  
configuration( { WIN64 } )
links( { "FreeImage", "gucefCORE", "gucefIMAGE", "gucefMT" } )
  links( { "FreeImage" } )
  

configuration( { WIN64 } )
defines( { "GUCEF_CODECPLUGIN_BUILD_MODULE" } )


configuration( { "LINUX32" } )
    vpaths { ["Platform Headers"] = { "**.h", "**.hpp", "**.hxx" } }
    files( {
      "include/imgpluginFreeImage.h"
    } )

    vpaths { ["Platform Source"] = { "**.c", "**.cpp", "**.cs", "**.asm" } }
    files( {
      "src/imgpluginFreeImage.c"
    } )



configuration( { "LINUX64" } )
    vpaths { ["Platform Headers"] = { "**.h", "**.hpp", "**.hxx" } }
    files( {
      "include/imgpluginFreeImage.h"
    } )

    vpaths { ["Platform Source"] = { "**.c", "**.cpp", "**.cs", "**.asm" } }
    files( {
      "src/imgpluginFreeImage.c"
    } )



configuration( { "WIN32" } )
    vpaths { ["Platform Headers"] = { "**.h", "**.hpp", "**.hxx" } }
    files( {
      "include/imgpluginFreeImage.h"
    } )

    vpaths { ["Platform Source"] = { "**.c", "**.cpp", "**.cs", "**.asm" } }
    files( {
      "src/imgpluginFreeImage.c"
    } )



configuration( { "WIN64" } )
    vpaths { ["Platform Headers"] = { "**.h", "**.hpp", "**.hxx" } }
    files( {
      "include/imgpluginFreeImage.h"
    } )

    vpaths { ["Platform Source"] = { "**.c", "**.cpp", "**.cs", "**.asm" } }
    files( {
      "src/imgpluginFreeImage.c"
    } )


configuration( {} )
includedirs( { "../../../common/include", "../../../platform/gucefCORE/include", "../../../platform/gucefCORE/include/c_api", "../../../platform/gucefIMAGE/include", "../../../platform/gucefMT/include" } )

configuration( { "LINUX32" } )
includedirs( { "../../../dependencies/FreeImage/Source", "../../../dependencies/FreeImage/Source/DeprecationManager", "../../../dependencies/FreeImage/Source/FreeImage", "../../../dependencies/FreeImage/Source/FreeImageToolkit", "../../../dependencies/FreeImage/Source/LibJPEG", "../../../dependencies/FreeImage/Source/LibJXR/common/include", "../../../dependencies/FreeImage/Source/LibJXR/image/decode", "../../../dependencies/FreeImage/Source/LibJXR/image/encode", "../../../dependencies/FreeImage/Source/LibJXR/image/sys", "../../../dependencies/FreeImage/Source/LibJXR/image/x86", "../../../dependencies/FreeImage/Source/LibJXR/jxrgluelib", "../../../dependencies/FreeImage/Source/LibOpenJPEG", "../../../dependencies/FreeImage/Source/LibPNG", "../../../dependencies/FreeImage/Source/LibRawLite/internal", "../../../dependencies/FreeImage/Source/LibRawLite/libraw", "../../../dependencies/FreeImage/Source/LibTIFF4", "../../../dependencies/FreeImage/Source/LibWebP/src/dec", "../../../dependencies/FreeImage/Source/LibWebP/src/dsp", "../../../dependencies/FreeImage/Source/LibWebP/src/enc", "../../../dependencies/FreeImage/Source/LibWebP/src/mux", "../../../dependencies/FreeImage/Source/LibWebP/src/utils", "../../../dependencies/FreeImage/Source/LibWebP/src/webp", "../../../dependencies/FreeImage/Source/Metadata", "../../../dependencies/FreeImage/Source/OpenEXR", "../../../dependencies/FreeImage/Source/OpenEXR/Half", "../../../dependencies/FreeImage/Source/OpenEXR/Iex", "../../../dependencies/FreeImage/Source/OpenEXR/IexMath", "../../../dependencies/FreeImage/Source/OpenEXR/IlmImf", "../../../dependencies/FreeImage/Source/OpenEXR/IlmThread", "../../../dependencies/FreeImage/Source/OpenEXR/Imath", "../../../dependencies/zlib", "../../../platform/gucefCORE/include/linux", "include" } )

configuration( { "LINUX64" } )
includedirs( { "../../../dependencies/FreeImage/Source", "../../../dependencies/FreeImage/Source/DeprecationManager", "../../../dependencies/FreeImage/Source/FreeImage", "../../../dependencies/FreeImage/Source/FreeImageToolkit", "../../../dependencies/FreeImage/Source/LibJPEG", "../../../dependencies/FreeImage/Source/LibJXR/common/include", "../../../dependencies/FreeImage/Source/LibJXR/image/decode", "../../../dependencies/FreeImage/Source/LibJXR/image/encode", "../../../dependencies/FreeImage/Source/LibJXR/image/sys", "../../../dependencies/FreeImage/Source/LibJXR/image/x86", "../../../dependencies/FreeImage/Source/LibJXR/jxrgluelib", "../../../dependencies/FreeImage/Source/LibOpenJPEG", "../../../dependencies/FreeImage/Source/LibPNG", "../../../dependencies/FreeImage/Source/LibRawLite/internal", "../../../dependencies/FreeImage/Source/LibRawLite/libraw", "../../../dependencies/FreeImage/Source/LibTIFF4", "../../../dependencies/FreeImage/Source/LibWebP/src/dec", "../../../dependencies/FreeImage/Source/LibWebP/src/dsp", "../../../dependencies/FreeImage/Source/LibWebP/src/enc", "../../../dependencies/FreeImage/Source/LibWebP/src/mux", "../../../dependencies/FreeImage/Source/LibWebP/src/utils", "../../../dependencies/FreeImage/Source/LibWebP/src/webp", "../../../dependencies/FreeImage/Source/Metadata", "../../../dependencies/FreeImage/Source/OpenEXR", "../../../dependencies/FreeImage/Source/OpenEXR/Half", "../../../dependencies/FreeImage/Source/OpenEXR/Iex", "../../../dependencies/FreeImage/Source/OpenEXR/IexMath", "../../../dependencies/FreeImage/Source/OpenEXR/IlmImf", "../../../dependencies/FreeImage/Source/OpenEXR/IlmThread", "../../../dependencies/FreeImage/Source/OpenEXR/Imath", "../../../dependencies/zlib", "../../../platform/gucefCORE/include/linux", "include" } )

configuration( { "WIN32" } )
includedirs( { "../../../dependencies/FreeImage/Source", "../../../dependencies/FreeImage/Source/DeprecationManager", "../../../dependencies/FreeImage/Source/FreeImage", "../../../dependencies/FreeImage/Source/FreeImageToolkit", "../../../dependencies/FreeImage/Source/LibJPEG", "../../../dependencies/FreeImage/Source/LibJXR/common/include", "../../../dependencies/FreeImage/Source/LibJXR/image/decode", "../../../dependencies/FreeImage/Source/LibJXR/image/encode", "../../../dependencies/FreeImage/Source/LibJXR/image/sys", "../../../dependencies/FreeImage/Source/LibJXR/image/x86", "../../../dependencies/FreeImage/Source/LibJXR/jxrgluelib", "../../../dependencies/FreeImage/Source/LibOpenJPEG", "../../../dependencies/FreeImage/Source/LibPNG", "../../../dependencies/FreeImage/Source/LibRawLite/internal", "../../../dependencies/FreeImage/Source/LibRawLite/libraw", "../../../dependencies/FreeImage/Source/LibTIFF4", "../../../dependencies/FreeImage/Source/LibWebP/src/dec", "../../../dependencies/FreeImage/Source/LibWebP/src/dsp", "../../../dependencies/FreeImage/Source/LibWebP/src/enc", "../../../dependencies/FreeImage/Source/LibWebP/src/mux", "../../../dependencies/FreeImage/Source/LibWebP/src/utils", "../../../dependencies/FreeImage/Source/LibWebP/src/webp", "../../../dependencies/FreeImage/Source/Metadata", "../../../dependencies/FreeImage/Source/OpenEXR", "../../../dependencies/FreeImage/Source/OpenEXR/Half", "../../../dependencies/FreeImage/Source/OpenEXR/Iex", "../../../dependencies/FreeImage/Source/OpenEXR/IexMath", "../../../dependencies/FreeImage/Source/OpenEXR/IlmImf", "../../../dependencies/FreeImage/Source/OpenEXR/IlmThread", "../../../dependencies/FreeImage/Source/OpenEXR/Imath", "../../../dependencies/zlib", "../../../platform/gucefCORE/include/mswin", "../../../platform/gucefMT/include/mswin", "include" } )

configuration( { "WIN64" } )
includedirs( { "../../../dependencies/FreeImage/Source", "../../../dependencies/FreeImage/Source/DeprecationManager", "../../../dependencies/FreeImage/Source/FreeImage", "../../../dependencies/FreeImage/Source/FreeImageToolkit", "../../../dependencies/FreeImage/Source/LibJPEG", "../../../dependencies/FreeImage/Source/LibJXR/common/include", "../../../dependencies/FreeImage/Source/LibJXR/image/decode", "../../../dependencies/FreeImage/Source/LibJXR/image/encode", "../../../dependencies/FreeImage/Source/LibJXR/image/sys", "../../../dependencies/FreeImage/Source/LibJXR/image/x86", "../../../dependencies/FreeImage/Source/LibJXR/jxrgluelib", "../../../dependencies/FreeImage/Source/LibOpenJPEG", "../../../dependencies/FreeImage/Source/LibPNG", "../../../dependencies/FreeImage/Source/LibRawLite/internal", "../../../dependencies/FreeImage/Source/LibRawLite/libraw", "../../../dependencies/FreeImage/Source/LibTIFF4", "../../../dependencies/FreeImage/Source/LibWebP/src/dec", "../../../dependencies/FreeImage/Source/LibWebP/src/dsp", "../../../dependencies/FreeImage/Source/LibWebP/src/enc", "../../../dependencies/FreeImage/Source/LibWebP/src/mux", "../../../dependencies/FreeImage/Source/LibWebP/src/utils", "../../../dependencies/FreeImage/Source/LibWebP/src/webp", "../../../dependencies/FreeImage/Source/Metadata", "../../../dependencies/FreeImage/Source/OpenEXR", "../../../dependencies/FreeImage/Source/OpenEXR/Half", "../../../dependencies/FreeImage/Source/OpenEXR/Iex", "../../../dependencies/FreeImage/Source/OpenEXR/IexMath", "../../../dependencies/FreeImage/Source/OpenEXR/IlmImf", "../../../dependencies/FreeImage/Source/OpenEXR/IlmThread", "../../../dependencies/FreeImage/Source/OpenEXR/Imath", "../../../dependencies/zlib", "../../../platform/gucefCORE/include/mswin", "../../../platform/gucefMT/include/mswin", "include" } )
