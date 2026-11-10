/*
 *  gucefGUI_WaylandTestApp: Tests of the Wayland window and input drivers against a fake compositor
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
#include <unistd.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_GUI_CVIDEOSETTINGS_H
#include "gucefGUI_CVideoSettings.h"
#define GUCEF_GUI_CVIDEOSETTINGS_H
#endif /* GUCEF_GUI_CVIDEOSETTINGS_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_H
#include "pluginglueWAYLAND.h"
#define GUCEF_PLUGINGLUE_WAYLAND_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_H ? */

#ifndef GUCEF_GUIDRIVERWAYLAND_H
#include "guidriverWAYLAND.h"
#define GUCEF_GUIDRIVERWAYLAND_H
#endif /* GUCEF_GUIDRIVERWAYLAND_H ? */

#ifndef GUCEF_INPUT_WAYLAND_CWAYLANDINPUTCONTEXT_H
#include "inputdriverWAYLAND_CWaylandInputContext.h"
#define GUCEF_INPUT_WAYLAND_CWAYLANDINPUTCONTEXT_H
#endif /* GUCEF_INPUT_WAYLAND_CWAYLANDINPUTCONTEXT_H ? */

#include "TestWaylandCommon.h"
#include "TestWaylandShutdown.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::PLUGINGLUE::WAYLAND;
using namespace GUCEF::GUIDRIVERWAYLAND;
using namespace GUCEF::INPUT::WAYLAND;
using namespace GUCEF::TESTWAYLAND;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The pulses of the application: the connection pumps, the windows draw and the input contexts repeat keys on them */
void
Pulses( UInt32 count )
{
    for ( UInt32 i=0; i<count; ++i )
    {
        CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        ::usleep( 500 );
    }
}

GUI::CVideoSettings
MakeSettings( void )
{
    GUI::CVideoSettings settings;
    settings.SetResolution( 1280, 720, 32 );
    settings.SetVSyncState( true );
    settings.SetFullscreenState( false );
    settings.SetFrequency( 60 );
    return settings;
}

void
ThunkClickPending( void* argument )
{
    CFakeWayland* fake = static_cast< CFakeWayland* >( argument );
    fake->SendPointerButton( 900, 5000, 0x110, 1 );
}

void
ThunkFramePending( void* argument )
{
    CFakeWayland* fake = static_cast< CFakeWayland* >( argument );
    fake->SendPointerFrame();
}

/**
 *  What the game has at the end of its life: a connection that pumps on the pulses of the application, a window with its GL
 *  context, and the input context of that window, which also lives on the pulses (the key repeat). The pieces are made
 *  in the order of the game (CGuiSetup::CreateWindowed) and taken away by the tests in its order (CGuiSetup::Destroy).
 */
struct SGameRig
{
    CFakeWayland fake;
    CFakeGraphics graphics;
    CWaylandConnection connection;
    CWaylandWindowContext* window;
    CWaylandInputContext* input;
    CInputRecorder recorder;
    SXkbApi xkb;
    bool hasXkb;
    bool connected;

    explicit SGameRig( bool libdecor = false )
        : fake(), graphics(), connection( fake.GetApi() ), window( GUCEF_NULL ), input( GUCEF_NULL ), recorder(), xkb(), hasXkb( false ), connected( false )
    {
        fake.OfferDefaultGlobals();
        fake.OfferGlobal( "wp_cursor_shape_manager_v1", 1 );
        fake.OfferGlobal( "wp_viewporter", 1 );
        fake.OfferGlobal( "wp_fractional_scale_manager_v1", 1 );
        fake.OfferGlobal( "zxdg_output_manager_v1", 3 );
        fake.OfferGlobal( "xdg_activation_v1", 1 );
        fake.keymapText = GetTestKeymap();
        hasXkb = LoadXkbApi( xkb );
        graphics.SetLibdecorAvailable( libdecor );
    }

    ~SGameRig()
    {
        delete input;
        delete window;
        connection.Disconnect();
    }

    bool Start( void )
    {
        UseNoCursorTheme( connection );
        connected = connection.Connect();
        if ( !connected )
            return false;
        window = new CWaylandWindowContext( connection, graphics.GetRuntime() );
        SWaylandWindowParams params;
        params.appId = "redux";
        if ( !window->Initialize( "Redux", MakeSettings(), params ) )
            return false;
        input = new CWaylandInputContext( connection, window->GetSurfaceId(), recorder.callbacks, hasXkb ? &xkb : GUCEF_NULL, true );
        return true;
    }

    SFakeProxy* Surface( void )
    {
        return (SFakeProxy*) connection.FindSurface( window->GetSurfaceId() )->surface;
    }

    /** What a user does in a few seconds with the pointer and the keyboard, as a compositor sends it */
    void UseTheWindow( void )
    {
        fake.SendPointerEnter( 10, Surface(), 100.0, 100.0 );
        fake.SendKeyboardEnter( 11, Surface() );
        for ( UInt32 i=0; i<5; ++i )
        {
            fake.SendPointerMotion( 100 + i, 100.0 + i, 100.0 );
            fake.SendPointerFrame();
        }
        fake.SendPointerButton( 12, 200, 0x110, 1 );
        fake.SendPointerButton( 13, 210, 0x110, 0 );
        fake.SendPointerAxisSource( GUCEF_WL_WL_POINTER_AXIS_SOURCE_WHEEL );
        fake.SendPointerAxis( 220, 0, 15.0 );
        fake.SendPointerAxisValue120( 0, 120 );
        fake.SendPointerFrame();
        fake.SendKeyboardKey( 14, 230, TESTKEY_A, 1 );
        Pulses( 5 );
        window->Update( 1000 );
        window->Update( 1016 );
    }
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformWaylandShutdownTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Wayland shutdown TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "WaylandShutdown" );

    GUCEF_TESTFW_TESTCASE( "Test: The game ends as it does at exit: the input context, then the window, then the connection, with the pulses going on all the time" )
        try
        {
            SGameRig rig;
            ASSERT_TRUE( rig.Start() );
            rig.UseTheWindow();
            ASSERT_TRUE( rig.recorder.Count( "pos" ) > 0 || rig.recorder.events.size() > 0 );

            // CGuiSetup::Destroy(): the input of the window, then the window
            delete rig.input;
            rig.input = GUCEF_NULL;
            Pulses( 5 );
            // events that arrive when nobody listens are of no use to anyone
            rig.fake.SendPointerMotion( 300, 1.0, 1.0 );
            rig.fake.SendPointerFrame();
            Pulses( 5 );
            delete rig.window;
            rig.window = GUCEF_NULL;
            Pulses( 5 );
            rig.fake.SendPointerFrame();
            rig.connection.Disconnect();
            ASSERT_FALSE( rig.connection.IsConnected() );
            Pulses( 20 );
            ASSERT_TRUE( GUCEF_NULL == CORE::GetActivationTokenProvider() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Events that were sent and not dispatched yet, and a frame callback that was asked for and not answered, when everything goes" )
        try
        {
            SGameRig rig;
            ASSERT_TRUE( rig.Start() );
            rig.UseTheWindow();
            rig.window->Update( 2000 );
            rig.window->Update( 2020 );
            ASSERT_TRUE( GUCEF_NULL != rig.fake.FindProxy( "wl_callback" ) );

            // a click and a frame that are on their way: nothing dispatches them
            rig.fake.Post( ThunkClickPending, &rig.fake );
            rig.fake.Post( ThunkFramePending, &rig.fake );
            delete rig.input;
            rig.input = GUCEF_NULL;
            delete rig.window;
            rig.window = GUCEF_NULL;
            rig.connection.Disconnect();
            Pulses( 10 );
            ASSERT_FALSE( rig.connection.IsConnected() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The connection goes first (the compositor is gone, the socket broke), then the input context and the window" )
        try
        {
            SGameRig rig;
            ASSERT_TRUE( rig.Start() );
            rig.UseTheWindow();
            rig.connection.Disconnect();
            Pulses( 5 );
            // the window and the input context outlive the connection: a pointer that was down is let go, nothing calls the dead connection
            delete rig.input;
            rig.input = GUCEF_NULL;
            Pulses( 5 );
            rig.window->Update( 3000 );
            delete rig.window;
            rig.window = GUCEF_NULL;
            Pulses( 5 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Every piece can be shut down twice, and the connection can be taken away twice" )
        try
        {
            SGameRig rig;
            ASSERT_TRUE( rig.Start() );
            rig.UseTheWindow();
            SFakeProxy* surface = rig.Surface();
            rig.window->Shutdown();
            rig.window->Shutdown();
            Pulses( 3 );
            rig.fake.SendPointerEnter( 50, surface, 1.0, 1.0 );
            rig.fake.SendPointerFrame();
            delete rig.input;
            rig.input = GUCEF_NULL;
            rig.connection.Disconnect();
            rig.connection.Disconnect();
            rig.window->Shutdown();
            delete rig.window;
            rig.window = GUCEF_NULL;
            Pulses( 5 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: With libdecor drawing the decorations (a GNOME session) the frame goes before the context of libdecor and both before the surface, in every order the game can end in" )
        try
        {
            for ( int order=0; order<3; ++order )
            {
                SGameRig rig( true );
                ASSERT_TRUE( rig.Start() );
                ASSERT_TRUE( rig.graphics.libdecorCreated > 0 );
                rig.UseTheWindow();
                if ( 0 == order )
                {
                    delete rig.input;
                    rig.input = GUCEF_NULL;
                    delete rig.window;
                    rig.window = GUCEF_NULL;
                    rig.connection.Disconnect();
                }
                else
                if ( 1 == order )
                {
                    rig.connection.Disconnect();
                    delete rig.input;
                    rig.input = GUCEF_NULL;
                    delete rig.window;
                    rig.window = GUCEF_NULL;
                }
                else
                {
                    rig.window->Shutdown();
                    rig.window->Shutdown();
                    delete rig.input;
                    rig.input = GUCEF_NULL;
                    delete rig.window;
                    rig.window = GUCEF_NULL;
                }
                Pulses( 10 );
                ASSERT_TRUE( rig.graphics.frameUnrefs <= 1 );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A key is held and the pointer is inside when the game ends: the repeat and the pulses of the input context do not outlive it" )
        try
        {
            SGameRig rig;
            ASSERT_TRUE( rig.Start() );
            rig.UseTheWindow();
            rig.fake.SendKeyboardKey( 20, 1000, TESTKEY_A, 1 );
            rig.fake.SendPointerButton( 21, 1000, 0x110, 1 );
            Pulses( 20 );
            delete rig.input;
            rig.input = GUCEF_NULL;
            // the pulses go on; whatever the input context subscribed to is gone with it
            Pulses( 50 );
            delete rig.window;
            rig.window = GUCEF_NULL;
            Pulses( 50 );
            ASSERT_TRUE( rig.connection.Pump() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL Wayland shutdown TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
