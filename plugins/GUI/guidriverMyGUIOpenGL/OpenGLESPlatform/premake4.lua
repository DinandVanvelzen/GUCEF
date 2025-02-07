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

-- Configuration for module: MyGUI.OpenGLESPlatform


configuration( { "ANDROID" } )
  project( "MyGUI.OpenGLESPlatform" )

configuration( {} )
  location( os.getenv( "PM4OUTPUTDIR" ) )

configuration( {} )
  targetdir( os.getenv( "PM4TARGETDIR" ) )

configuration( {} )
language( "C" )

configuration( { "ANDROID" } )
language( "C++" )

configuration( { "ANDROID" } )


configuration( { ANDROID } )
kind( "StaticLib" )
  
configuration( { ANDROID } )
links( { "MyGUI.Engine" } )
  links( { "GLESv1_CM", "MyGUI.Engine" } )


configuration( { "ANDROID" } )
    vpaths { ["Platform Headers"] = { "**.h", "**.hpp", "**.hxx" } }
    files( {
      "../../../../dependencies/MyGui/Common/FileSystemInfo/FileSystemInfo.h",
      "include/MyGUI_OpenGLDiagnostic.h",
      "include/MyGUI_OpenGLImageLoader.h",
      "include/MyGUI_OpenGLRTTexture.h",
      "include/MyGUI_OpenGLRenderManager.h",
      "include/MyGUI_OpenGLTexture.h",
      "include/MyGUI_OpenGLVertexBuffer.h"
    } )

    vpaths { ["Platform Source"] = { "**.c", "**.cpp", "**.cs", "**.asm" } }
    files( {
      "src/MyGUI_OpenGLRTTexture.cpp",
      "src/MyGUI_OpenGLRenderManager.cpp",
      "src/MyGUI_OpenGLTexture.cpp",
      "src/MyGUI_OpenGLVertexBuffer.cpp"
    } )


configuration( {} )
includedirs( { "../../../../dependencies/MyGui/MyGUIEngine/include", "../../../../dependencies/freetype/include", "../../../../dependencies/freetype/include/freetype", "../../../../dependencies/freetype/include/freetype/config", "../../../../dependencies/freetype/include/freetype/internal", "../../../../dependencies/freetype/include/freetype/internal/services", "../../../../dependencies/freetype/src", "../../../../dependencies/freetype/src/winfonts" } )

configuration( { "ANDROID" } )
includedirs( { "../../../../dependencies/MyGui/Common/FileSystemInfo", "include" } )
