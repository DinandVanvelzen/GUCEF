/*
 *  guidriverImGuiOgre_TestApp: tests of the gucefGUI driver ImGuiOgre
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

#include <Ogre.h>

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "guidriverImGuiOgre_TestSupport.h"
#include "TestImGuiOgreScenes.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMGUIOGRETEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUIDRIVERIMGUI;
using namespace GUIDRIVERIMGUIOGRE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformImGuiOgreContextTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Context" );

    GUCEF_TESTFW_TESTCASE( "Context: the plugin registers the driver ImGuiOgre, which refuses windows that are not Ogre windows and makes overlay contexts on Ogre windows" )
    try
    {
        GUI::CGUIManager& manager = GUI::CGuiGlobal::Instance()->GetGuiManager();
        GUI::CGUIDriver* driver = manager.GetGuiDriver( "ImGuiOgre" );
        ASSERT_TRUE( GUCEF_NULL != driver );
        if ( GUCEF_NULL == driver )
            GUCEF_TESTFW_SKIP( "a precondition of the test failed, see the assertion before" );
        ASSERT_TRUE( "ImGuiOgre" == driver->GetDriverName() );
        ASSERT_TRUE( driver->HasCapability( "Render" ) && driver->HasCapability( "CaptureFrame" ) && driver->HasCapability( "ImmediateMode" ) );
        ASSERT_TRUE( driver->HasCapability( "VectorFonts" ) && driver->HasCapability( "PresentScaling" ) && driver->HasCapability( "DynamicImages" ) );
        ASSERT_FALSE( driver->HasCapability( "PresentFilters" ) || driver->HasCapability( "NativeLayouts" ) );

        // a window of another kind: refused with an error log entry
        if ( !LoadFreeTypeFontPlugin() )
            GUCEF_TESTFW_SKIP( "the FreeType font plugin is not there" );
        CLogTextCounter counter( "needs a window of the window backend Ogre" );
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &counter );
        GUI::TWindowContextPtr other( GUCEF_NEW CNonOgreWindow() );
        ASSERT_TRUE( manager.CreateGUIContext( "ImGuiOgre", other ).IsNULL() );
        ASSERT_TRUE( manager.CreateGUIContext( "ImGuiOgre", GUI::TWindowContextPtr() ).IsNULL() );
        CORE::CCoreGlobal::Instance()->GetLogManager().FlushLogs();
        CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( &counter );
        ASSERT_TRUE( 2 == counter.m_count );

        if ( TESTWINDOWMODE_NONE == GetTestWindowMode() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        GUI::TWindowContextPtr window = GetMainTestWindow();
        ASSERT_TRUE( !window.IsNULL() );
        GUIDRIVEROGRE::COgreWindowContext* ogreWindow = AsOgreWindow( window );
        ASSERT_TRUE( GUCEF_NULL != ogreWindow );
        if ( GUCEF_NULL == ogreWindow )
            GUCEF_TESTFW_SKIP( "a precondition of the test failed, see the assertion before" );
        const UInt32 overlays = ogreWindow->GetOverlayCount();
        {
            GUI::TGuiContextPtr contextPtr = CreateImGuiOgreContext();
            CImGuiOgreContext* context = static_cast< CImGuiOgreContext* >( AsImGui( contextPtr ) );
            ASSERT_TRUE( GUCEF_NULL != context );
            if ( GUCEF_NULL != context )
            {
                ASSERT_TRUE( context->IsReady() && context->IsRendererReady() );
                ASSERT_TRUE( CImGuiOgreContext::ClassTypeName == context->GetClassTypeName() );
                ASSERT_TRUE( overlays + 1 == ogreWindow->GetOverlayCount() );
                ASSERT_TRUE( context->GetLayer() == context->GetOverlayOrder() );
                ASSERT_TRUE( !driver->GetDriverProperty( "Renderer" ).IsNULLOrEmpty() );
                CORE::CString rendererName;
                {
                    CImGuiContextScope scope( context->GetImGuiContext() );
                    rendererName = ImGui::GetIO().BackendRendererName;
                    ASSERT_TRUE( 0 != ( ImGui::GetIO().BackendFlags & ImGuiBackendFlags_RendererHasTextures ) );
                }
                ASSERT_TRUE( "gucefGUI Ogre" == rendererName );
                ASSERT_TRUE( 1 == driver->GetContextCount() );
            }
        }
        // the context unregisters from its window when it goes
        ASSERT_TRUE( overlays == ogreWindow->GetOverlayCount() );
        ASSERT_TRUE( 0 == driver->GetContextCount() );
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

/*-------------------------------------------------------------------------*/

void
PerformImGuiOgreTearDownTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "TearDown" );

    GUCEF_TESTFW_TESTCASE( "TearDown: the window, the plugins and the Root go in the order of an application, and the driver is gone" )
    try
    {
        TearDownOgreForTests();
        ASSERT_TRUE( GUCEF_NULL == GUI::CGuiGlobal::Instance()->GetGuiManager().GetGuiDriver( "ImGuiOgre" ) );
        ASSERT_TRUE( GUCEF_NULL == Ogre::Root::getSingletonPtr() );
        // Dear ImGui holds nothing of ours once the last context is gone
        ASSERT_TRUE( !GUIDRIVERIMGUI::CImGuiGlobal::HasInstance() || 0 == GUIDRIVERIMGUI::CImGuiGlobal::Instance()->GetContextCount() );
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

}; /* namespace IMGUIOGRETEST */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
