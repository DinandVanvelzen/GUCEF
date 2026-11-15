/*
 *  gucefGUI_TestApp: Application for testing gucefGUI
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

#include <stdlib.h>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "gucefGUI_TestCommon.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

/** Counts the redraw and size events of a window context */
class CWindowEventCounter : public CORE::CObserver
{
    public:

    CWindowEventCounter( void )
        : CORE::CObserver()
        , redraws( 0 )
        , sizes( 0 )
    {
    }

    virtual ~CWindowEventCounter()
    {
        UnsubscribeAllFromObserver();
    }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( GUI::CWindowContext::WindowContextRedrawEvent == eventId )
            ++redraws;
        else
        if ( GUI::CWindowContext::WindowContextSizeEvent == eventId )
            ++sizes;
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CWindowEventCounter";
        return classTypeName;
    }

    UInt32 redraws;
    UInt32 sizes;
};

/** Pulses the application (X11 events, window frames) until the condition is met or about a second passed */
template < typename TCondition >
bool
PulseUntil( TCondition condition )
{
    CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    for ( UInt32 i=0; i<100; ++i )
    {
        pulseGenerator->ForceDirectPulse();
        if ( condition() )
            return true;
        MT::PrecisionDelay( 10 );
    }
    return condition();
}

struct SHasRedraws
{
    const CWindowEventCounter& counter;
    explicit SHasRedraws( const CWindowEventCounter& c ) : counter( c ) {}
    bool operator()( void ) const { return counter.redraws > 0; }
};

struct SHasWidth
{
    const GUI::TWindowContextPtr& window;
    UInt32 width;
    SHasWidth( const GUI::TWindowContextPtr& w, UInt32 wd ) : window( w ), width( wd ) {}
    bool operator()( void ) const { return window->GetWidth() == width; }
};

/** Windows on the X server only when the test run asks for them (a private server such as Xvfb), never on a desktop by surprise */
bool
X11WindowsAllowed( void )
{
    const char* allowed = ::getenv( "GUCEF_TEST_X11_WINDOWS" );
    const char* display = ::getenv( "DISPLAY" );
    return GUCEF_NULL != allowed && '1' == allowed[ 0 ] && GUCEF_NULL != display && '\0' != display[ 0 ];
}

CORE::CValueList
GraphicsApiParams( const char* graphicsApi )
{
    CORE::CValueList params;
    params.Set( GUI::CWindowManagerBackend::GraphicsApiParameterName, graphicsApi );
    return params;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiGraphicsApiTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.GraphicsApi" );

    GUCEF_TESTFW_TESTCASE( "GraphicsApi: the window parameter is read in any letter case, and is empty when it is not given" )
    try
    {
        ASSERT_TRUE( GUI::CWindowManagerBackend::GetRequestedGraphicsApi( GUCEF_NULL ).IsNULLOrEmpty() );
        CORE::CValueList noApi;
        noApi.Set( "X", "10" );
        ASSERT_TRUE( GUI::CWindowManagerBackend::GetRequestedGraphicsApi( &noApi ).IsNULLOrEmpty() );
        CORE::CValueList none = GraphicsApiParams( "none" );
        ASSERT_TRUE( GUI::CWindowManagerBackend::GraphicsApiNone == GUI::CWindowManagerBackend::GetRequestedGraphicsApi( &none ) );
        CORE::CValueList gl = GraphicsApiParams( "Gl" );
        ASSERT_TRUE( GUI::CWindowManagerBackend::GraphicsApiGL == GUI::CWindowManagerBackend::GetRequestedGraphicsApi( &gl ) );
        CORE::CValueList other = GraphicsApiParams( "Vulkan" );
        ASSERT_TRUE( "Vulkan" == GUI::CWindowManagerBackend::GetRequestedGraphicsApi( &other ) );
        ASSERT_TRUE( "GraphicsApi" == GUI::CWindowManagerBackend::GraphicsApiParameterName );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

    GUI::CWindowManager& windows = GUI::CGuiGlobal::Instance()->GetWindowManager();

    GUCEF_TESTFW_TESTCASE( "GraphicsApi: the offscreen EGL backend makes GL windows and refuses windows without a GL context" )
    if ( windows.GetBackend( "EGL" ).IsNULL() )
        LoadTestPlugin( "guidriverEGL", "guidriver" );
    if ( windows.GetBackend( "EGL" ).IsNULL() )
        GUCEF_TESTFW_SKIP( "the EGL window backend is not there" );
    try
    {
        GUI::TWindowManagerBackendPtr backend = windows.GetBackend( "EGL" );
        ASSERT_TRUE( backend->IsGraphicsApiSupported( GUI::CWindowManagerBackend::GraphicsApiGL ) );
        ASSERT_FALSE( backend->IsGraphicsApiSupported( GUI::CWindowManagerBackend::GraphicsApiNone ) );

        CORE::CValueList none = GraphicsApiParams( "None" );
        GUI::TWindowContextPtr refused = backend->CreateWindowContext( "Test", 64, 48, false, none );
        ASSERT_TRUE( refused.IsNULL() );

        CORE::CValueList gl = GraphicsApiParams( "GL" );
        GUI::TWindowContextPtr window = backend->CreateWindowContext( "Test", 64, 48, false, gl );
        ASSERT_TRUE( !window.IsNULL() );
        if ( !window.IsNULL() )
        {
            ASSERT_TRUE( !window->GetProperty( "GL_GETPROCADDRESS" ).IsNULLOrEmpty() );
            backend->DestroyWindowContext( window );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GraphicsApi: an XWinGL window without a GL context has the native handles, events and sizing, and no GL properties" )
    if ( !X11WindowsAllowed() )
        GUCEF_TESTFW_SKIP( "X11 windows are made only when GUCEF_TEST_X11_WINDOWS=1 and DISPLAY name a server for tests (Xvfb)" );
    if ( windows.GetBackend( "XWinGL" ).IsNULL() )
        LoadTestPlugin( "guidriverXWinGL", "guidriver" );
    if ( windows.GetBackend( "XWinGL" ).IsNULL() )
        GUCEF_TESTFW_SKIP( "the XWinGL window backend is not there" );
    try
    {
        GUI::TWindowManagerBackendPtr backend = windows.GetBackend( "XWinGL" );
        ASSERT_TRUE( backend->IsGraphicsApiSupported( GUI::CWindowManagerBackend::GraphicsApiGL ) );
        ASSERT_TRUE( backend->IsGraphicsApiSupported( GUI::CWindowManagerBackend::GraphicsApiNone ) );
        ASSERT_FALSE( backend->IsGraphicsApiSupported( "Vulkan" ) );

        CORE::CValueList vulkan = GraphicsApiParams( "Vulkan" );
        GUI::TWindowContextPtr refused = backend->CreateWindowContext( "Test", 64, 48, false, vulkan );
        ASSERT_TRUE( refused.IsNULL() );

        CORE::CValueList none = GraphicsApiParams( "None" );
        GUI::TWindowContextPtr window = backend->CreateWindowContext( "Test", 200, 150, false, none );
        ASSERT_TRUE( !window.IsNULL() );
        if ( !window.IsNULL() )
        {
            ASSERT_TRUE( GUI::CWindowManagerBackend::GraphicsApiNone == window->GetProperty( "GRAPHICS_API" ) );
            ASSERT_TRUE( window->GetProperty( "GL_GETPROCADDRESS" ).IsNULLOrEmpty() );
            ASSERT_TRUE( window->GetProperty( "GL_API" ).IsNULLOrEmpty() );
            ASSERT_TRUE( window->GetProperty( "GL_FRAMEBUFFER" ).IsNULLOrEmpty() );
            ASSERT_TRUE( 0 != CORE::StringToUInt32( window->GetProperty( "WINDOW" ), 0 ) );
            ASSERT_TRUE( GUCEF_NULL != CORE::StringToPointer( window->GetProperty( "DISPLAY" ) ) );
            ASSERT_TRUE( 200 == window->GetWidth() && 150 == window->GetHeight() );

            // the window paces frames like every window: its redraw event comes, only nothing is cleared or swapped
            CWindowEventCounter counter;
            counter.SubscribeTo( window.GetPointerAlways() );
            ASSERT_TRUE( PulseUntil( SHasRedraws( counter ) ) );

            // sizing works as for a GL window
            GUI::CVideoSettings settings;
            settings.SetResolution( 320, 240, 24 );
            settings.SetFullscreenState( false );
            ASSERT_TRUE( backend->ApplyVideoSettings( window, settings ) );
            ASSERT_TRUE( PulseUntil( SHasWidth( window, 320 ) ) );
            ASSERT_TRUE( counter.sizes > 0 );
            counter.UnsubscribeAllFromObserver();
            backend->DestroyWindowContext( window );
        }

        // the GL window of the same backend reports its GL context as before
        GUI::TWindowContextPtr glWindow = backend->CreateWindowContext( "Test", 64, 48, false );
        ASSERT_TRUE( !glWindow.IsNULL() );
        if ( !glWindow.IsNULL() )
        {
            ASSERT_TRUE( GUI::CWindowManagerBackend::GraphicsApiGL == glWindow->GetProperty( "GRAPHICS_API" ) );
            ASSERT_TRUE( "GL" == glWindow->GetProperty( "GL_API" ) );
            ASSERT_TRUE( !glWindow->GetProperty( "GL_GETPROCADDRESS" ).IsNULLOrEmpty() );
            backend->DestroyWindowContext( glWindow );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    #endif
}

/*-------------------------------------------------------------------------*/
