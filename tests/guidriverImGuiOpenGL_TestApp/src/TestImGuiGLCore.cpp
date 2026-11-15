/*
 *  guidriverImGuiOpenGL_TestApp: tests of the gucefGUI driver ImGui (OpenGL)
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "guidriverImGuiOpenGL_TestSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMGUIGLTEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGLCoreTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Core" );

    GUCEF_TESTFW_TESTCASE( "Core: the GLSL version follows the window: OpenGL 3.3 core, OpenGL ES 3, nothing older, no desktop OpenGL in a build for OpenGL ES" )
    try
    {
        CString glsl, reason;
        ASSERT_TRUE( GUIDRIVERIMGUIGL::CImGuiOpenGLContext::SelectGlslVersion( "GL", "3.3", false, glsl, reason ) );
        ASSERT_TRUE( "#version 330 core" == glsl && reason.IsNULLOrEmpty() );
        ASSERT_TRUE( GUIDRIVERIMGUIGL::CImGuiOpenGLContext::SelectGlslVersion( "GL", "4.6", false, glsl, reason ) );
        ASSERT_TRUE( "#version 330 core" == glsl );
        ASSERT_TRUE( GUIDRIVERIMGUIGL::CImGuiOpenGLContext::SelectGlslVersion( "GLES", "3.0", false, glsl, reason ) );
        ASSERT_TRUE( "#version 300 es" == glsl );
        ASSERT_TRUE( GUIDRIVERIMGUIGL::CImGuiOpenGLContext::SelectGlslVersion( "GLES", "3.0", true, glsl, reason ) );
        ASSERT_TRUE( "#version 300 es" == glsl );
        ASSERT_TRUE( GUIDRIVERIMGUIGL::CImGuiOpenGLContext::SelectGlslVersion( "GLES", "3.2", true, glsl, reason ) );
        ASSERT_TRUE( "#version 300 es" == glsl );

        // refused, with a reason and no version
        ASSERT_FALSE( GUIDRIVERIMGUIGL::CImGuiOpenGLContext::SelectGlslVersion( "GL", "3.2", false, glsl, reason ) );
        ASSERT_TRUE( glsl.IsNULLOrEmpty() && !reason.IsNULLOrEmpty() );
        ASSERT_FALSE( GUIDRIVERIMGUIGL::CImGuiOpenGLContext::SelectGlslVersion( "GL", "1.x", false, glsl, reason ) );
        ASSERT_FALSE( GUIDRIVERIMGUIGL::CImGuiOpenGLContext::SelectGlslVersion( "GL", "", false, glsl, reason ) );
        ASSERT_FALSE( GUIDRIVERIMGUIGL::CImGuiOpenGLContext::SelectGlslVersion( "GLES", "2.0", false, glsl, reason ) );
        ASSERT_FALSE( GUIDRIVERIMGUIGL::CImGuiOpenGLContext::SelectGlslVersion( "GL", "3.3", true, glsl, reason ) );
        ASSERT_FALSE( GUIDRIVERIMGUIGL::CImGuiOpenGLContext::SelectGlslVersion( "", "3.3", false, glsl, reason ) );
        ASSERT_FALSE( GUIDRIVERIMGUIGL::CImGuiOpenGLContext::SelectGlslVersion( "Ogre", "3.3", false, glsl, reason ) );

        // what this build was made for
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN || GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )
        ASSERT_TRUE( GUIDRIVERIMGUIGL::CImGuiGLFunctions::IsOpenGlEsBuild() );
        #else
        ASSERT_FALSE( GUIDRIVERIMGUIGL::CImGuiGLFunctions::IsOpenGlEsBuild() );
        #endif
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Core: the plugin registers the driver ImGui with the capabilities of an immediate-mode renderer that captures" )
    try
    {
        GUI::CGUIManager& manager = GUI::CGuiGlobal::Instance()->GetGuiManager();
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
        if ( GUCEF_NULL == manager.GetGuiDriver( "ImGui" ) )
            GUIDRIVERIMGUIGL::guidriverImGuiOpenGL_GUCEFPlugin_Load( 0, GUCEF_NULL );
        #else
        if ( GUCEF_NULL == manager.GetGuiDriver( "ImGui" ) )
            ASSERT_TRUE( LoadTestPlugin( "guidriverImGuiOpenGL", "guidriver" ) );
        #endif
        GUI::CGUIDriver* driver = manager.GetGuiDriver( "ImGui" );
        ASSERT_TRUE( GUCEF_NULL != driver );
        if ( GUCEF_NULL != driver )
        {
            ASSERT_TRUE( "ImGui" == driver->GetDriverName() );
            ASSERT_TRUE( driver->HasCapability( "Render" ) && driver->HasCapability( "CaptureFrame" ) && driver->HasCapability( "ImmediateMode" ) );
            ASSERT_TRUE( driver->HasCapability( "VectorFonts" ) && driver->HasCapability( "PresentScaling" ) && driver->HasCapability( "DynamicImages" ) );
            ASSERT_FALSE( driver->HasCapability( "PresentFilters" ) || driver->HasCapability( "NativeLayouts" ) );
            ASSERT_TRUE( GUI::CGUIDriver::InterfaceVersion == driver->GetInterfaceVersion() );
            ASSERT_TRUE( !driver->GetDriverProperty( "ImGuiVersion" ).IsNULLOrEmpty() );
        }
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Core: the driver refuses a window without a GL context and no window at all, with an error log entry and without a context" )
    try
    {
        if ( !LoadFreeTypeFontPlugin() )
            GUCEF_TESTFW_SKIP( "the FreeType font plugin is not there" );
        CLogTextCounter counter( "needs a window with an OpenGL context" );
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &counter );

        GUIDRIVERIMGUIGL::CImGuiOpenGLDriver& driver = GetOpenGLDriver();
        GUI::TWindowContextPtr noGl( GUCEF_NEW CNoGlWindow() );
        ASSERT_TRUE( driver.CreateGUIContext( noGl ).IsNULL() );
        ASSERT_TRUE( driver.CreateGUIContext( GUI::TWindowContextPtr() ).IsNULL() );
        ASSERT_TRUE( 0 == driver.GetContextCount() );

        CORE::CCoreGlobal::Instance()->GetLogManager().FlushLogs();
        CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( &counter );
        ASSERT_TRUE( 2 == counter.m_count );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMGUIGLTEST */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
