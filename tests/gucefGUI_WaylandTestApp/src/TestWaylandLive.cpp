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

#include <stdlib.h>
#include <unistd.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

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

#include "TestWaylandCommon.h"
#include "TestWaylandLive.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::PLUGINGLUE::WAYLAND;
using namespace GUCEF::GUIDRIVERWAYLAND;
using namespace GUCEF::TESTWAYLAND;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Pumps the connection and the window by hand, as the pulses of the application do, until the condition holds or the time is up */
bool
RunUntil( CWaylandConnection& connection, CWaylandWindowContext& window, UInt32 timeoutInMs, UInt32 minimumFrames, Int32 wantedWidth )
{
    const UInt32 start = CORE::GUCEFGetTickCount();
    const UInt32 framesAtStart = window.GetFramesDrawn();
    while ( (UInt32) ( CORE::GUCEFGetTickCount() - start ) < timeoutInMs )
    {
        if ( !connection.Pump() )
            return false;
        window.Update( CORE::GUCEFGetTickCount() );
        if ( window.GetFramesDrawn() - framesAtStart >= minimumFrames && ( wantedWidth <= 0 || (Int32) window.GetWidth() == wantedWidth ) )
            return true;
        ::usleep( 4000 );
    }
    return false;
}

}

/*-------------------------------------------------------------------------*/

void
PerformWaylandLiveTests( bool live )
{GUCEF_TRACE;

    if ( !live )
        return;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Wayland live TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "WaylandLive" );

    GUCEF_TESTFW_TESTCASE( "Test: A window on a compositor is configured, drawn into with EGL, resized and closed" )
        try
        {
            CWaylandWindowManagerImp manager;
            ASSERT_TRUE( manager.IsAvailable() );
            if ( !manager.IsAvailable() )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Wayland live: not available: " << manager.GetAvailabilityDescription() );
            }
            else
            {
                CORE::CString problem;
                CWaylandConnection* connection = CWaylandConnection::Acquire( &problem );
                ASSERT_TRUE( GUCEF_NULL != connection );
                if ( GUCEF_NULL != connection )
                {
                    connection->SetPumpOnPulse( false );
                    ASSERT_TRUE( connection->HasGlobal( "xdg_wm_base" ) );

                    CWaylandWindowContext window( *connection, GetWaylandGuiRuntime() );
                    GUI::CVideoSettings settings;
                    settings.SetResolution( 640, 480, 32 );
                    settings.SetVSyncState( true );
                    settings.SetFrequency( 60 );
                    SWaylandWindowParams params;
                    params.appId = "gucef-wayland-live-test";
                    ASSERT_TRUE( window.Initialize( "GUCEF Wayland live test", settings, params ) );

                    ASSERT_TRUE( window.IsConfigured() );
                    ASSERT_TRUE( window.GetWidth() > 0 && window.GetHeight() > 0 );
                    ASSERT_TRUE( RunUntil( *connection, window, 5000, 3, 0 ) );
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Wayland live: decorations " << window.GetProperty( "DECORATIONS" ) << ", scale " << window.GetProperty( "SCALE" )
                        << ", size " << window.GetWidth() << " x " << window.GetHeight() );

                    // a floating window takes the size of the settings by drawing at that size
                    settings.SetResolution( 800, 600, 32 );
                    ASSERT_TRUE( window.ApplyVideoSettings( settings ) );
                    const bool resized = RunUntil( *connection, window, 5000, 2, (Int32) CWaylandScale::LogicalToBuffer( 800, window.GetScale120() ) );
                    ASSERT_TRUE( resized );
                    ASSERT_TRUE( 800 == window.GetLogicalWidth() && 600 == window.GetLogicalHeight() );

                    window.Shutdown();
                    CWaylandConnection::Release( connection );
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Wayland live: ok" );
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A compositor that offers xdg-activation gives the launcher of the browser a token for the window" )
        try
        {
            CWaylandWindowManagerImp manager;
            ASSERT_TRUE( manager.IsAvailable() );
            CORE::CString problem;
            CWaylandConnection* connection = manager.IsAvailable() ? CWaylandConnection::Acquire( &problem ) : GUCEF_NULL;
            ASSERT_TRUE( GUCEF_NULL != connection );
            if ( GUCEF_NULL != connection )
            {
                connection->SetPumpOnPulse( false );
                CWaylandWindowContext window( *connection, GetWaylandGuiRuntime() );
                GUI::CVideoSettings settings;
                settings.SetResolution( 640, 480, 32 );
                settings.SetVSyncState( true );
                settings.SetFrequency( 60 );
                SWaylandWindowParams params;
                params.appId = "gucef-wayland-live-test";
                ASSERT_TRUE( window.Initialize( "GUCEF Wayland live test", settings, params ) );
                ASSERT_TRUE( RunUntil( *connection, window, 5000, 2, 0 ) );

                CORE::CIActivationTokenProvider* provider = CORE::GetActivationTokenProvider();
                ASSERT_TRUE( GUCEF_NULL != provider );
                if ( !connection->HasGlobal( "xdg_activation_v1" ) )
                {
                    // a compositor without it: the launcher opens the browser without a token
                    CORE::CString token;
                    CORE::TActivationTokenKind kind = CORE::ACTIVATIONTOKEN_STARTUP_ID;
                    ASSERT_FALSE( GUCEF_NULL != provider && provider->GetActivationToken( token, kind, 200 ) );
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Wayland live: the compositor offers no xdg_activation_v1, no token" );
                }
                else
                if ( GUCEF_NULL != provider )
                {
                    // a headless compositor has no input devices: the token is made without a serial, the compositor still answers it
                    CORE::CString token;
                    CORE::TActivationTokenKind kind = CORE::ACTIVATIONTOKEN_STARTUP_ID;
                    ASSERT_TRUE( provider->GetActivationToken( token, kind, 1000 ) );
                    ASSERT_TRUE( !token.IsNULLOrEmpty() && CORE::ACTIVATIONTOKEN_XDG_ACTIVATION == kind );
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Wayland live: a token was made (not logged)" );
                }
                window.Shutdown();
                ASSERT_TRUE( GUCEF_NULL == CORE::GetActivationTokenProvider() );
                CWaylandConnection::Release( connection );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
