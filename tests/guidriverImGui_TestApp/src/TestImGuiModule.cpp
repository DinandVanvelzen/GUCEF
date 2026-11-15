/*
 *  guidriverImGui_TestApp: tests of the gucefGUI driver library of Dear ImGui
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

#include <string.h>

#include "imgui.h"
#include "imgui_internal.h"

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "guidriverImGui_TestSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMGUITEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUIDRIVERIMGUI;

namespace {

/** A window that Dear ImGui wants to save the place of */
void
BuildMovedWindow( CImGuiContext& context, UInt32 deltaInMs, void* userData )
{
    Int32* frame = static_cast< Int32* >( userData );
    ImGui::SetNextWindowPos( ImVec2( 20.0f + (float) ( *frame ), 30.0f ), ImGuiCond_Always );
    ImGui::SetNextWindowSize( ImVec2( 120.0f, 80.0f ), ImGuiCond_Always );
    ImGui::Begin( "Saved" );
    ImGui::Text( "frame %d", *frame );
    ImGui::End();
    ++( *frame );
}

}

/*-------------------------------------------------------------------------*/

void
PerformImGuiModuleTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "Module" );

    GUCEF_TESTFW_TESTCASE( "Module: the plugin, the test and the imgui module have the same version and data layout, 32-bit indices and characters" )
    try
    {
        ASSERT_TRUE( ImGui::DebugCheckVersionAndDataLayout( IMGUI_VERSION, sizeof( ImGuiIO ), sizeof( ImGuiStyle ), sizeof( ImVec2 ), sizeof( ImVec4 ), sizeof( ImDrawVert ), sizeof( ImDrawIdx ) ) );
        ASSERT_TRUE( 4 == sizeof( ImDrawIdx ) );
        ASSERT_TRUE( 4 == sizeof( ImWchar ) );
        ASSERT_TRUE( 19300 == IMGUI_VERSION_NUM );
        ASSERT_TRUE( "1.93.0" == GetTestDriver().GetDriverProperty( "ImGuiVersion" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Module: the driver reports what it can do, ImmediateMode included, and no native layouts, backend hints or present filters" )
    try
    {
        CTestImGuiDriver& driver = GetTestDriver();
        const char* expected[] = { "Render", "CaptureFrame", "BitmapFonts", "VectorFonts", "DrawSurface", "DynamicImages", "PresentScaling", "ImmediateMode" };
        for ( UInt32 i=0; i<sizeof( expected )/sizeof( expected[ 0 ] ); ++i )
            ASSERT_TRUE( driver.HasCapability( expected[ i ] ) );
        ASSERT_FALSE( driver.HasCapability( "NativeLayouts" ) );
        ASSERT_FALSE( driver.HasCapability( "BackendHints" ) );
        ASSERT_FALSE( driver.HasCapability( "PresentFilters" ) );
        ASSERT_TRUE( GUI::CGUIDriver::InterfaceVersion == driver.GetInterfaceVersion() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Module: Dear ImGui allocates through the GUCEF allocator while a context exists and frees all of it with the context" )
    try
    {
        CImGuiGlobal& global = *CImGuiGlobal::Instance();
        const UInt64 allocationsBefore = global.GetAllocationCount();
        const UInt64 outstandingBefore = global.GetAllocationCount() - global.GetFreeCount();
        {
            GUI::TGuiContextPtr contextPtr = CreateImGuiContext();
            CImGuiContext* context = AsImGui( contextPtr );
            ASSERT_TRUE( GUCEF_NULL != context );
            if ( GUCEF_NULL != context )
            {
                ASSERT_TRUE( global.IsAllocatorInstalled() );
                for ( UInt32 i=0; i<3; ++i )
                    context->RenderFrame( 16 );
                ASSERT_TRUE( global.GetAllocationCount() > allocationsBefore );
            }
        }
        ASSERT_TRUE( global.GetAllocationCount() - global.GetFreeCount() == outstandingBefore );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Module: a failed assertion of Dear ImGui is logged and counted, and does not stop the program when the test says so" )
    try
    {
        CImGuiGlobal& global = *CImGuiGlobal::Instance();
        const bool stop = global.GetStopOnAssertion();
        global.SetStopOnAssertion( false );
        const UInt32 before = global.GetAssertionCount();
        volatile int two = 2;
        IM_ASSERT( 1 == two );
        ASSERT_TRUE( before + 1 == global.GetAssertionCount() );
        ASSERT_TRUE( global.GetLastAssertion().HasSubstr( "1 == two" ) >= 0 );
        global.SetStopOnAssertion( stop );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Module: Dear ImGui writes no file, its settings are given to the application instead" )
    try
    {
        CString iniPath = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( iniPath, "imgui.ini" );
        CString logPath = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( logPath, "imgui_log.txt" );
        const bool iniBefore = CORE::FileExists( iniPath );

        GUI::TGuiContextPtr contextPtr = CreateImGuiContext();
        CImGuiContext* context = AsImGui( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            ASSERT_TRUE( GUCEF_NULL == context->GetImGuiContext()->IO.IniFilename );
            Int32 frame = 0;
            CTestFrameBuilder builder( &BuildMovedWindow, &frame );
            ASSERT_TRUE( builder.Attach( contextPtr ) );
            CEventCounter counter;
            context->Subscribe( &counter, CImGuiContext::IniSettingsChangedEvent );
            for ( UInt32 i=0; i<4; ++i )
                context->RenderFrame( 3000 );
            ASSERT_TRUE( counter.Count( CImGuiContext::IniSettingsChangedEvent ) > 0 );
            CString settings;
            ASSERT_TRUE( context->GetIniSettings( settings ) );
            ASSERT_TRUE( settings.HasSubstr( "[Window][Saved]" ) >= 0 );
        }
        contextPtr.Unlink();
        ASSERT_TRUE( iniBefore == CORE::FileExists( iniPath ) );
        ASSERT_FALSE( CORE::FileExists( logPath ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Module: the configuration section ImGui sets what new contexts start with" )
    try
    {
        CTestImGuiDriver& driver = GetTestDriver();
        const CImGuiDriver::TSettings original = driver.GetSettings();

        CORE::CDataNode config( "ImGui" );
        config.SetAttribute( "style", "Light" );
        config.SetAttribute( "uiScale", "2" );
        config.SetAttribute( "fontSize", "20" );
        config.SetAttribute( "keyboardNavigation", "false" );
        ASSERT_TRUE( driver.LoadConfig( config ) );
        ASSERT_TRUE( "Light" == driver.GetSettings().style );
        ASSERT_TRUE( 20.0f == driver.GetSettings().fontSize );

        GUI::TGuiContextPtr contextPtr = CreateImGuiContext();
        CImGuiContext* context = AsImGui( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            ASSERT_TRUE( "Light" == context->GetStyle() );
            ASSERT_TRUE( 2.0f == context->GetUiScale() );
            ASSERT_TRUE( 0 == ( context->GetImGuiContext()->IO.ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard ) );
            ASSERT_TRUE( 2.0f == context->GetImGuiContext()->Style.FontScaleDpi );
            ASSERT_TRUE( 20.0f == context->GetImGuiContext()->Style.FontSizeBase );
        }

        CORE::CDataNode saved;
        ASSERT_TRUE( driver.SaveConfig( saved ) );
        ASSERT_TRUE( GUCEF_NULL != saved.FindChild( "ImGui" ) );
        driver.SetSettings( original );
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

}; /* namespace IMGUITEST */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
