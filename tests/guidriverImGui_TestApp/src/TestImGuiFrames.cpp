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

/** What a frame builder saw */
struct SFrameRecord
{
    UInt32 builds;
    ::ImGuiContext* current;
    ::ImGuiContext* expected;
    bool currentWasRight;
    Float32 deltaTime;
    UInt32 eventDelta;
    ImVec2 displaySize;
    ImVec2 framebufferScale;
    bool building;

    SFrameRecord( void )
        : builds( 0 ), current( GUCEF_NULL ), expected( GUCEF_NULL ), currentWasRight( true ), deltaTime( 0.0f ), eventDelta( 0 ),
          displaySize( 0.0f, 0.0f ), framebufferScale( 0.0f, 0.0f ), building( false )
    {
    }
};

void
RecordFrame( CImGuiContext& context, UInt32 deltaInMs, void* userData )
{
    SFrameRecord* record = static_cast< SFrameRecord* >( userData );
    ++record->builds;
    record->current = ImGui::GetCurrentContext();
    if ( record->current != context.GetImGuiContext() )
        record->currentWasRight = false;
    ImGuiIO& io = ImGui::GetIO();
    record->deltaTime = io.DeltaTime;
    record->eventDelta = deltaInMs;
    record->displaySize = io.DisplaySize;
    record->framebufferScale = io.DisplayFramebufferScale;
    record->building = context.IsBuildingFrame();
    ImGui::Begin( "Record" );
    ImGui::Text( "Hello" );
    ImGui::End();
}

UInt32 g_destroyedBuilderCalls = 0;

void
CountCall( CImGuiContext& context, UInt32 deltaInMs, void* userData )
{
    ++g_destroyedBuilderCalls;
}

/** What a second thread was given */
struct SThreadResult
{
    bool contextMade;
    bool isImGuiThread;
    bool done;
};

UInt32 GUCEF_CALLSPEC_PREFIX
MakeContextOnThread( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    SThreadResult* result = static_cast< SThreadResult* >( data );
    result->isImGuiThread = CImGuiGlobal::Instance()->IsImGuiThread();
    GUI::TGuiContextPtr context = GetTestDriver().CreateGUIContext( GUI::TWindowContextPtr() );
    result->contextMade = !context.IsNULL();
    result->done = true;
    return 0;
}

}

/*-------------------------------------------------------------------------*/

void
PerformImGuiFrameTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "Frames" );

    GUCEF_TESTFW_TESTCASE( "Frames: BuildFrameEvent is notified once per frame with the context current, the previous context is current again afterwards" )
    try
    {
        GUI::TGuiContextPtr firstPtr = CreateImGuiContext();
        GUI::TGuiContextPtr secondPtr = CreateImGuiContext();
        CImGuiContext* first = AsImGui( firstPtr );
        CImGuiContext* second = AsImGui( secondPtr );
        ASSERT_TRUE( GUCEF_NULL != first && GUCEF_NULL != second );
        if ( GUCEF_NULL != first && GUCEF_NULL != second )
        {
            SFrameRecord record;
            CTestFrameBuilder builder( &RecordFrame, &record );
            ASSERT_TRUE( builder.Attach( firstPtr ) );
            ImGui::SetCurrentContext( second->GetImGuiContext() );
            for ( UInt32 i=0; i<3; ++i )
                first->RenderFrame( 16 );
            ASSERT_TRUE( 3 == record.builds );
            ASSERT_TRUE( 3 == builder.GetBuildCount() );
            ASSERT_TRUE( 3 == first->GetBuiltFrameCount() );
            ASSERT_TRUE( record.currentWasRight );
            ASSERT_TRUE( record.building );
            ASSERT_FALSE( first->IsBuildingFrame() );
            ASSERT_TRUE( ImGui::GetCurrentContext() == second->GetImGuiContext() );
            ImGui::SetCurrentContext( GUCEF_NULL );
            first->RenderFrame( 16 );
            ASSERT_TRUE( GUCEF_NULL == ImGui::GetCurrentContext() );

            // a hidden builder is not called
            builder.SetVisible( false );
            first->RenderFrame( 16 );
            ASSERT_TRUE( 4 == record.builds );
            ASSERT_TRUE( 5 == first->GetBuiltFrameCount() );
            ASSERT_TRUE( GUCEF_NULL != first->GetDrawData() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Frames: the time of a frame given to Dear ImGui is never zero, the event gives the frame time" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateImGuiContext();
        CImGuiContext* context = AsImGui( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            SFrameRecord record;
            CTestFrameBuilder builder( &RecordFrame, &record );
            builder.Attach( contextPtr );
            context->RenderFrame( 0 );
            ASSERT_TRUE( record.deltaTime > 0.0f );
            ASSERT_TRUE( 0 == record.eventDelta );
            context->RenderFrame( 25 );
            ASSERT_TRUE( record.deltaTime > 0.0249f && record.deltaTime < 0.0251f );
            ASSERT_TRUE( 25 == record.eventDelta );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Frames: Dear ImGui lays out in the logical size, the framebuffer scale is the present scale or without a window the render scale" )
    try
    {
        GUI::TWindowContextPtr window( GUCEF_NEW CRecordingWindow() );
        GUI::TGuiContextPtr contextPtr = CreateImGuiContext( window );
        CImGuiContext* context = AsImGui( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            SFrameRecord record;
            CTestFrameBuilder builder( &RecordFrame, &record );
            builder.Attach( contextPtr );
            context->SetLogicalResolution( 320, 240 );
            context->RenderFrame( 16 );
            ASSERT_TRUE( 320.0f == record.displaySize.x && 240.0f == record.displaySize.y );
            ASSERT_TRUE( 2.0f == record.framebufferScale.x && 2.0f == record.framebufferScale.y );

            // the present mapping a renderer applies: logical (0,0) at the present rectangle
            context->SetPresentMode( GUI::PRESENTMODE_INTEGER );
            context->SetLogicalResolution( 300, 200 );
            CImGuiContext::TDrawDataMapping mapping = context->GetPresentMapping();
            GUI::CRect present = context->GetPresentRect();
            ASSERT_TRUE( 2.0f == mapping.framebufferScale.x && 2.0f == mapping.framebufferScale.y );
            ASSERT_TRUE( -present.left / 2.0f == mapping.displayPos.x && -present.top / 2.0f == mapping.displayPos.y );
            ASSERT_TRUE( 320.0f == mapping.displaySize.x && 240.0f == mapping.displaySize.y );
        }
        contextPtr.Unlink();

        GUI::TGuiContextPtr headlessPtr = CreateImGuiContext();
        CImGuiContext* headless = AsImGui( headlessPtr );
        ASSERT_TRUE( GUCEF_NULL != headless );
        if ( GUCEF_NULL != headless )
        {
            SFrameRecord record;
            CTestFrameBuilder builder( &RecordFrame, &record );
            builder.Attach( headlessPtr );
            headless->SetRenderScale( 2 );
            headless->RenderFrame( 16 );
            ASSERT_TRUE( 640.0f == record.displaySize.x );
            ASSERT_TRUE( 2.0f == record.framebufferScale.x && 2.0f == record.framebufferScale.y );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Frames: a frame builder lets go of a context that goes, and a builder that goes is not called again" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateImGuiContext();
        CImGuiContext* context = AsImGui( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            CTestFrameBuilder survivor( &CountCall, GUCEF_NULL );
            ASSERT_TRUE( survivor.Attach( contextPtr ) );
            ASSERT_TRUE( survivor.IsAttached() );
            {
                CTestFrameBuilder shortLived( &CountCall, GUCEF_NULL );
                shortLived.Attach( contextPtr );
                context->RenderFrame( 16 );
                ASSERT_TRUE( 2 == g_destroyedBuilderCalls );
            }
            context->RenderFrame( 16 );
            ASSERT_TRUE( 3 == g_destroyedBuilderCalls );

            // the context goes: the builder knows, and lets go
            contextPtr.Unlink();
            ASSERT_FALSE( survivor.IsAttached() );
            ASSERT_TRUE( survivor.GetContext().IsNULL() );
            survivor.Detach();

            // a gucefGUI context of another driver is refused
            ASSERT_FALSE( survivor.Attach( GUI::TGuiContextPtr() ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Frames: while an ImGui context exists, a context made on another thread is refused" )
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) && !defined( __EMSCRIPTEN_PTHREADS__ )
    GUCEF_TESTFW_SKIP( "a web build without threads" );
    #endif
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateImGuiContext();
        ASSERT_TRUE( !contextPtr.IsNULL() );
        ASSERT_TRUE( CImGuiGlobal::Instance()->IsImGuiThread() );

        SThreadResult result;
        result.contextMade = true;
        result.isImGuiThread = true;
        result.done = false;
        MT::SThreadData* thread = MT::ThreadDataReserve();
        bool started = 0 != MT::ThreadCreate( thread, &MakeContextOnThread, &result );
        ASSERT_TRUE( started );
        if ( started )
            MT::ThreadWait( thread, 30000 );
        MT::ThreadDataCleanup( thread );
        ASSERT_TRUE( result.done );
        ASSERT_FALSE( result.contextMade );
        ASSERT_FALSE( result.isImGuiThread );
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
