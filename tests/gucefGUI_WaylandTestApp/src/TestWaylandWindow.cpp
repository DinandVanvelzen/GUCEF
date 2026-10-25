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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

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
#include "TestWaylandWindow.h"

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

/** A fake compositor, fake graphics and a connection to them: what a window needs around it */
struct SRig
{
    CFakeWayland fake;
    CFakeGraphics graphics;
    CWaylandConnection connection;

    explicit SRig( bool extensions = true, bool libdecor = false )
        : fake(), graphics(), connection( fake.GetApi() )
    {
        fake.OfferDefaultGlobals();
        if ( extensions )
        {
            fake.OfferGlobal( "wp_viewporter", 1 );
            fake.OfferGlobal( "wp_fractional_scale_manager_v1", 1 );
            fake.OfferGlobal( "zxdg_decoration_manager_v1", 1 );
            fake.OfferGlobal( "wp_cursor_shape_manager_v1", 1 );
            fake.OfferGlobal( "zxdg_output_manager_v1", 3 );
        }
        graphics.SetLibdecorAvailable( libdecor );
        connection.SetPumpOnPulse( false );
    }

    bool Connect( void ) { UseNoCursorTheme( connection ); return connection.Connect(); }
};

GUI::CVideoSettings
MakeSettings( bool vsync = true, bool fullscreen = false, UInt32 width = 1280, UInt32 height = 720 )
{
    GUI::CVideoSettings settings;
    settings.SetResolution( width, height, 32 );
    settings.SetVSyncState( vsync );
    settings.SetFullscreenState( fullscreen );
    settings.SetFrequency( 60 );
    return settings;
}

SWaylandWindowParams
MakeParams( const char* appId = "redux" )
{
    SWaylandWindowParams params;
    params.appId = appId;
    return params;
}

Int32
IndexOfPrefix( const CFakeWayland& fake, const char* prefix )
{
    const size_t length = strlen( prefix );
    for ( UInt32 i=0; i<fake.requests.size(); ++i )
        if ( 0 == strncmp( fake.requests[ i ].C_String(), prefix, length ) )
            return (Int32) i;
    return -1;
}

SFakeProxy*
FindFrameCallback( const CFakeWayland& fake )
{
    return fake.FindProxy( "wl_callback" );
}

}

/*-------------------------------------------------------------------------*/

namespace {

bool
ProviderOptions( GUI::CVideoOptions& options )
{
    GUI::CVideoOptions::TDisplayMode mode;
    mode.widthInPixels = 800; mode.heightInPixels = 600; mode.depthInBits = 24; mode.frequency = 75;
    options.AddDisplayMode( mode );
    return true;
}

bool
ProviderCurrent( GUI::CVideoOptions::TDisplayMode& mode )
{
    mode.widthInPixels = 800; mode.heightInPixels = 600; mode.depthInBits = 24; mode.frequency = 75;
    return true;
}

bool
ProviderNothing( GUI::CVideoOptions& )
{
    return false;
}

bool
ProviderNoCurrent( GUI::CVideoOptions::TDisplayMode& )
{
    return false;
}

}

/*-------------------------------------------------------------------------*/

void
PerformWaylandWindowTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Wayland window TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "WaylandWindow" );

    GUCEF_TESTFW_TESTCASE( "Test: A window is an xdg toplevel with a title, an app id and server side decorations, shown after the first configure" )
        try
        {
            SRig rig;
            ASSERT_TRUE( rig.Connect() );
            CWaylandWindowContext window( rig.connection, rig.graphics.GetRuntime() );
            ASSERT_TRUE( window.Initialize( "Redux: Echoes", MakeSettings(), MakeParams() ) );

            const Int32 getSurface = IndexOfPrefix( rig.fake, "xdg_wm_base.get_xdg_surface(" );
            const Int32 getToplevel = IndexOfPrefix( rig.fake, "xdg_surface.get_toplevel(" );
            const Int32 title = IndexOfPrefix( rig.fake, "xdg_toplevel.set_title(" );
            const Int32 appId = IndexOfPrefix( rig.fake, "xdg_toplevel.set_app_id(" );
            const Int32 decoration = IndexOfPrefix( rig.fake, "zxdg_decoration_manager_v1.get_toplevel_decoration(" );
            const Int32 mode = IndexOfPrefix( rig.fake, "zxdg_toplevel_decoration_v1.set_mode(" );
            const Int32 commit = IndexOfPrefix( rig.fake, "wl_surface.commit(" );
            const Int32 ack = IndexOfPrefix( rig.fake, "xdg_surface.ack_configure(" );
            ASSERT_TRUE( getSurface >= 0 && getToplevel > getSurface && title > getToplevel && appId > title );
            ASSERT_TRUE( decoration > appId && mode > decoration && commit > mode && ack > commit );
            ASSERT_TRUE( rig.fake.Requested( "xdg_toplevel.set_title(\"Redux: Echoes\")" ) );
            ASSERT_TRUE( rig.fake.Requested( "xdg_toplevel.set_app_id(\"redux\")" ) );
            ASSERT_TRUE( rig.fake.Requested( "zxdg_toplevel_decoration_v1.set_mode(2)" ) );
            ASSERT_TRUE( rig.fake.Requested( "xdg_surface.ack_configure(100)" ) );

            ASSERT_TRUE( window.IsConfigured() );
            ASSERT_TRUE( 1280 == window.GetWidth() && 720 == window.GetHeight() );
            ASSERT_TRUE( DECORATION_SERVER == window.GetDecorationMode() );
            ASSERT_TRUE( "server" == window.GetProperty( "DECORATIONS" ) );
            ASSERT_TRUE( "wayland" == window.GetProperty( "WINDOW_SYSTEM" ) );
            ASSERT_TRUE( "Redux: Echoes" == window.GetName() );
            ASSERT_FALSE( window.IsActive() );
            ASSERT_TRUE( 0 == rig.graphics.swapCount );                     // nothing is drawn until the pulses
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The window draws with EGL on the Wayland platform, with the buffer at the size of the window" )
        try
        {
            SRig rig;
            ASSERT_TRUE( rig.Connect() );
            CWaylandWindowContext window( rig.connection, rig.graphics.GetRuntime() );
            ASSERT_TRUE( window.Initialize( "Redux", MakeSettings(), MakeParams() ) );
            ASSERT_TRUE( 1 == rig.graphics.platformDisplayCalls );
            ASSERT_TRUE( 0x31D8 == rig.graphics.lastPlatform );
            ASSERT_TRUE( 1 == rig.graphics.windowCreateCount );
            ASSERT_TRUE( 1280 == rig.graphics.lastWindowWidth && 720 == rig.graphics.lastWindowHeight );
            ASSERT_TRUE( 1 == rig.graphics.createSurfaceCount );
            ASSERT_TRUE( 0 == rig.graphics.swapInterval );                  // the compositor paces the window, not the swap
            ASSERT_TRUE( 1280 == rig.graphics.lastViewportWidth && 720 == rig.graphics.lastViewportHeight );
            ASSERT_TRUE( "GL" == window.GetProperty( "GL_API" ) );
            ASSERT_TRUE( "3.3" == window.GetProperty( "GL_VERSION" ) );
            ASSERT_TRUE( "0" == window.GetProperty( "GL_FRAMEBUFFER" ) );
            ASSERT_FALSE( window.GetProperty( "GL_GETPROCADDRESS" ).IsNULLOrEmpty() );
            ASSERT_TRUE( "1" == window.GetProperty( "SCALE" ) );
            ASSERT_TRUE( "1280" == window.GetProperty( "LOGICAL_WIDTH" ) && "720" == window.GetProperty( "LOGICAL_HEIGHT" ) );

            // the surface can be found by its id: that is how the input driver finds the surface of the window
            const UInt32 id = CORE::StringToUInt32( window.GetProperty( "WINDOW" ), 0 );
            ASSERT_TRUE( 0 != id && id == window.GetID() );
            ASSERT_TRUE( GUCEF_NULL != rig.connection.FindSurface( id ) );
            ASSERT_TRUE( 120 == rig.connection.FindSurface( id )->scale120 );

            // the window is drawn into with an opaque region so the compositor need not blend it
            ASSERT_TRUE( rig.fake.Requested( "wl_region.add(0, 0, 1280, 720)" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A compositor that wants the client to draw the decorations leaves the window without" )
        try
        {
            SRig rig;
            rig.fake.decorationReply = GUCEF_WL_ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE;
            ASSERT_TRUE( rig.Connect() );
            CWaylandWindowContext window( rig.connection, rig.graphics.GetRuntime() );
            ASSERT_TRUE( window.Initialize( "Redux", MakeSettings(), MakeParams() ) );
            ASSERT_TRUE( DECORATION_NONE == window.GetDecorationMode() );
            ASSERT_TRUE( "none" == window.GetProperty( "DECORATIONS" ) );
            // and when the compositor changes its mind
            rig.fake.SendDecorationMode( GUCEF_WL_ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE );
            ASSERT_TRUE( DECORATION_SERVER == window.GetDecorationMode() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Where the compositor offers no decorations (GNOME) libdecor makes them" )
        try
        {
            SRig rig( false, true );
            ASSERT_TRUE( rig.Connect() );
            CWaylandWindowContext window( rig.connection, rig.graphics.GetRuntime() );
            CEventCounter counter;
            counter.Watch( window, GUI::CWindowContext::WindowContextSizeEvent );
            counter.Watch( window, GUI::CWindowContext::WindowContextActivateEvent );
            counter.Watch( window, GUI::CWindowContext::WindowContextCloseRequestEvent );
            ASSERT_TRUE( window.Initialize( "Redux", MakeSettings(), MakeParams() ) );

            ASSERT_TRUE( DECORATION_LIBDECOR == window.GetDecorationMode() );
            ASSERT_TRUE( 1 == rig.graphics.libdecorCreated );
            ASSERT_TRUE( "Redux" == rig.graphics.frameTitle && "redux" == rig.graphics.frameAppId );
            ASSERT_TRUE( rig.graphics.frameMapped );
            ASSERT_TRUE( 0 == rig.fake.CountRequestsWithPrefix( "xdg_wm_base.get_xdg_surface" ) );     // libdecor makes the toplevel
            ASSERT_TRUE( 0 == rig.fake.CountRequestsWithPrefix( "zxdg_decoration_manager_v1" ) );

            // the first configure came in the dispatch of libdecor and the window committed its size
            ASSERT_TRUE( window.IsConfigured() );
            ASSERT_TRUE( 1 == rig.graphics.commitCount );
            ASSERT_TRUE( 1280 == rig.graphics.lastCommitWidth && 720 == rig.graphics.lastCommitHeight );
            ASSERT_TRUE( 1280 == window.GetWidth() );
            ASSERT_TRUE( "libdecor" == window.GetProperty( "DECORATIONS" ) );

            // the user maximises the window: libdecor tells the content size and the state
            rig.graphics.configureWidth = 3840;
            rig.graphics.configureHeight = 1040;
            rig.graphics.configureState = 1 | 2;            // active, maximised
            rig.graphics.frameCallbacks.configure( rig.graphics.frame, (struct libdecor_configuration*) 0x8000, rig.graphics.frameUserData );
            ASSERT_TRUE( 2 == rig.graphics.commitCount );
            ASSERT_TRUE( 3840 == rig.graphics.lastCommitWidth && 1040 == rig.graphics.lastCommitHeight );
            ASSERT_TRUE( 3840 == window.GetWidth() && 1040 == window.GetHeight() );
            ASSERT_TRUE( window.IsMaximized() && window.IsActive() );
            ASSERT_TRUE( 3840 == rig.graphics.lastWindowWidth );
            ASSERT_TRUE( 1 == counter.Count( GUI::CWindowContext::WindowContextActivateEvent ) );
            ASSERT_TRUE( counter.Count( GUI::CWindowContext::WindowContextSizeEvent ) >= 1 );

            // the close button of the decorations
            rig.graphics.frameCallbacks.close( rig.graphics.frame, rig.graphics.frameUserData );
            ASSERT_TRUE( 1 == counter.Count( GUI::CWindowContext::WindowContextCloseRequestEvent ) );

            // libdecor gets its turn on every update, and full screen goes through it
            const UInt32 dispatches = rig.graphics.libdecorDispatches;
            window.Update( 100 );
            ASSERT_TRUE( rig.graphics.libdecorDispatches > dispatches );
            GUI::CVideoSettings fullscreen = MakeSettings( true, true );
            ASSERT_TRUE( window.ApplyVideoSettings( fullscreen ) );
            ASSERT_TRUE( rig.graphics.frameFullscreen );
            ASSERT_TRUE( window.ApplyVideoSettings( MakeSettings( true, false ) ) );
            ASSERT_FALSE( rig.graphics.frameFullscreen );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: When libdecor cannot make decorations the window is a plain toplevel without them" )
        try
        {
            SRig rig( false, true );
            rig.graphics.decorateFails = true;
            ASSERT_TRUE( rig.Connect() );
            CWaylandWindowContext window( rig.connection, rig.graphics.GetRuntime() );
            ASSERT_TRUE( window.Initialize( "Redux", MakeSettings(), MakeParams() ) );
            ASSERT_TRUE( DECORATION_NONE == window.GetDecorationMode() );
            ASSERT_TRUE( 1 == rig.fake.CountRequestsWithPrefix( "xdg_wm_base.get_xdg_surface" ) );
            ASSERT_TRUE( window.IsConfigured() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Without server decorations and without libdecor the window has none and works" )
        try
        {
            SRig rig( false, false );
            ASSERT_TRUE( rig.Connect() );
            CWaylandWindowContext window( rig.connection, rig.graphics.GetRuntime() );
            ASSERT_TRUE( window.Initialize( "Redux", MakeSettings(), MakeParams() ) );
            ASSERT_TRUE( DECORATION_NONE == window.GetDecorationMode() );
            ASSERT_TRUE( 0 == rig.graphics.libdecorCreated );
            ASSERT_TRUE( window.IsConfigured() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The user can ask for no decorations" )
        try
        {
            SRig rig( true, true );
            ASSERT_TRUE( rig.Connect() );
            CWaylandWindowContext window( rig.connection, rig.graphics.GetRuntime() );
            SWaylandWindowParams params = MakeParams();
            params.decorations = DECORATIONPREFERENCE_NONE;
            ASSERT_TRUE( window.Initialize( "Redux", MakeSettings(), params ) );
            ASSERT_TRUE( DECORATION_NONE == window.GetDecorationMode() );
            ASSERT_TRUE( 0 == rig.fake.CountRequestsWithPrefix( "zxdg_decoration_manager_v1" ) );
            ASSERT_TRUE( 0 == rig.graphics.libdecorCreated );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A configure is acknowledged and the window follows the size and the states the compositor gives" )
        try
        {
            SRig rig;
            ASSERT_TRUE( rig.Connect() );
            CWaylandWindowContext window( rig.connection, rig.graphics.GetRuntime() );
            CEventCounter counter;
            counter.Watch( window, GUI::CWindowContext::WindowContextSizeEvent );
            counter.Watch( window, GUI::CWindowContext::WindowContextActivateEvent );
            ASSERT_TRUE( window.Initialize( "Redux", MakeSettings(), MakeParams() ) );

            // activated: the first state changes of a window are not a change of the activation
            const UInt32 activated[] = { GUCEF_WL_XDG_TOPLEVEL_STATE_ACTIVATED };
            const UInt32 serial = rig.fake.Configure( 1280, 720, activated, 1 );
            ASSERT_TRUE( rig.fake.Requested( ( "xdg_surface.ack_configure(" + CORE::UInt32ToString( serial ) + ")" ).C_String() ) );
            ASSERT_TRUE( window.IsActive() );
            ASSERT_TRUE( 1 == counter.Count( GUI::CWindowContext::WindowContextActivateEvent ) );
            ASSERT_TRUE( 0 == counter.Count( GUI::CWindowContext::WindowContextSizeEvent ) );   // the size did not change

            // maximised
            const UInt32 maximized[] = { GUCEF_WL_XDG_TOPLEVEL_STATE_MAXIMIZED, GUCEF_WL_XDG_TOPLEVEL_STATE_ACTIVATED };
            rig.fake.Configure( 3840, 1040, maximized, 2 );
            ASSERT_TRUE( 3840 == window.GetWidth() && 1040 == window.GetHeight() );
            ASSERT_TRUE( window.IsMaximized() && !window.IsFullscreen() );
            ASSERT_TRUE( 3840 == rig.graphics.lastWindowWidth && 1040 == rig.graphics.lastWindowHeight );
            ASSERT_TRUE( 3840 == rig.graphics.lastViewportWidth );
            ASSERT_TRUE( 1 == counter.Count( GUI::CWindowContext::WindowContextSizeEvent ) );
            ASSERT_TRUE( 1 == counter.Count( GUI::CWindowContext::WindowContextActivateEvent ) );    // still active

            // the window loses the focus
            rig.fake.Configure( 3840, 1040, maximized, 1 );
            ASSERT_FALSE( window.IsActive() );
            ASSERT_TRUE( 2 == counter.Count( GUI::CWindowContext::WindowContextActivateEvent ) );

            // un-maximised: the size is left to us and the window goes back to the one it had
            rig.fake.Configure( 0, 0, activated, 1 );
            ASSERT_TRUE( 1280 == window.GetWidth() && 720 == window.GetHeight() );
            ASSERT_TRUE( 3 == counter.Count( GUI::CWindowContext::WindowContextActivateEvent ) );
            ASSERT_TRUE( 2 == counter.Count( GUI::CWindowContext::WindowContextSizeEvent ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The close button of the compositor is a request to close that the application decides on" )
        try
        {
            SRig rig;
            ASSERT_TRUE( rig.Connect() );
            CWaylandWindowContext window( rig.connection, rig.graphics.GetRuntime() );
            CEventCounter counter;
            counter.Watch( window, GUI::CWindowContext::WindowContextCloseRequestEvent );
            ASSERT_TRUE( window.Initialize( "Redux", MakeSettings(), MakeParams() ) );
            rig.fake.SendToplevelClose();
            ASSERT_TRUE( 1 == counter.Count( GUI::CWindowContext::WindowContextCloseRequestEvent ) );
            rig.fake.SendToplevelClose();       // the application said no and the user pressed it again
            ASSERT_TRUE( 2 == counter.Count( GUI::CWindowContext::WindowContextCloseRequestEvent ) );
            ASSERT_TRUE( window.IsConfigured() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A fractional scale gives a buffer larger than the surface and a viewport that maps it back" )
        try
        {
            SRig rig;
            rig.fake.preferredFractionalScale = 180;
            ASSERT_TRUE( rig.Connect() );
            CWaylandWindowContext window( rig.connection, rig.graphics.GetRuntime() );
            CEventCounter counter;
            counter.Watch( window, GUI::CWindowContext::WindowContextSizeEvent );
            ASSERT_TRUE( window.Initialize( "Redux", MakeSettings(), MakeParams() ) );

            // 1280 x 720 surface units at 1.5: the buffer is 1920 x 1080 pixels, which is what the GUI sees
            ASSERT_TRUE( 1920 == window.GetWidth() && 1080 == window.GetHeight() );
            ASSERT_TRUE( 1920 == rig.graphics.lastWindowWidth && 1080 == rig.graphics.lastWindowHeight );
            ASSERT_TRUE( rig.fake.Requested( "wp_viewport.set_destination(1280, 720)" ) );
            ASSERT_TRUE( 0 == rig.fake.CountRequestsWithPrefix( "wl_surface.set_buffer_scale" ) );     // the viewport does it
            ASSERT_TRUE( "1.5" == window.GetProperty( "SCALE" ) );
            ASSERT_TRUE( 1280 == window.GetLogicalWidth() && 720 == window.GetLogicalHeight() );
            ASSERT_TRUE( 180 == window.GetScale120() );
            ASSERT_TRUE( 180 == rig.connection.FindSurface( window.GetSurfaceId() )->scale120 );

            // the window is dragged to a monitor of 2.0
            rig.fake.SendFractionalScale( 240 );
            ASSERT_TRUE( 2560 == window.GetWidth() && 1440 == window.GetHeight() );
            ASSERT_TRUE( 2560 == rig.graphics.lastWindowWidth );
            ASSERT_TRUE( 2560 == rig.graphics.lastViewportWidth );
            ASSERT_TRUE( 1 == counter.Count( GUI::CWindowContext::WindowContextSizeEvent ) );
            ASSERT_TRUE( 240 == rig.connection.FindSurface( window.GetSurfaceId() )->scale120 );

            // and to a monitor of 1.25: the buffer rounds the way the protocol says
            rig.fake.SendFractionalScale( 150 );
            ASSERT_TRUE( 1600 == window.GetWidth() && 900 == window.GetHeight() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Whole scales use the buffer scale of the surface, the preference of the compositor first, then the monitor the window is on" )
        try
        {
            SRig rig( false );
            ASSERT_TRUE( rig.Connect() );
            CWaylandWindowContext window( rig.connection, rig.graphics.GetRuntime() );
            ASSERT_TRUE( window.Initialize( "Redux", MakeSettings(), MakeParams() ) );
            SFakeProxy* surface = rig.fake.FindProxy( "wl_surface" );
            SFakeProxy* monitor = rig.fake.FindProxy( "wl_output" );
            ASSERT_TRUE( GUCEF_NULL != surface && GUCEF_NULL != monitor );
            ASSERT_TRUE( 1280 == window.GetWidth() );

            // the window enters a monitor of scale 2
            rig.fake.SendOutputScale( monitor, 2 );
            rig.fake.SendSurfaceEnter( surface, monitor );
            ASSERT_TRUE( 2560 == window.GetWidth() && 1440 == window.GetHeight() );
            ASSERT_TRUE( rig.fake.Requested( "wl_surface.set_buffer_scale(2)" ) );
            ASSERT_TRUE( 2560 == rig.graphics.lastWindowWidth );
            ASSERT_TRUE( 240 == rig.connection.FindSurface( window.GetSurfaceId() )->scale120 );
            ASSERT_TRUE( "2" == window.GetProperty( "SCALE" ) );

            // the monitor changes its scale (the OutputsChanged event of the connection)
            rig.fake.SendOutputScale( monitor, 3 );
            rig.fake.SendOutputDone( monitor );
            ASSERT_TRUE( 3840 == window.GetWidth() );
            ASSERT_TRUE( rig.fake.Requested( "wl_surface.set_buffer_scale(3)" ) );

            // the compositor's own preference wins
            rig.fake.SendPreferredBufferScale( surface, 2 );
            ASSERT_TRUE( 2560 == window.GetWidth() );

            // and the window leaves the monitor with the preference gone: it stays at what the compositor prefers
            rig.fake.SendSurfaceLeave( surface, monitor );
            ASSERT_TRUE( 2560 == window.GetWidth() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: With vertical sync the window draws when the compositor showed the frame before and asks for the next callback before the swap" )
        try
        {
            SRig rig;
            ASSERT_TRUE( rig.Connect() );
            CWaylandWindowContext window( rig.connection, rig.graphics.GetRuntime() );
            CEventCounter counter;
            counter.Watch( window, GUI::CWindowContext::WindowContextRedrawEvent );
            ASSERT_TRUE( window.Initialize( "Redux", MakeSettings( true ), MakeParams() ) );

            window.Update( 1000 );
            ASSERT_TRUE( 1 == rig.graphics.swapCount );
            ASSERT_TRUE( 1 == rig.graphics.clearCount );
            ASSERT_TRUE( 1 == counter.Count( GUI::CWindowContext::WindowContextRedrawEvent ) );
            ASSERT_TRUE( 1 == rig.fake.CountRequestsWithPrefix( "wl_surface.frame(" ) );
            SFakeProxy* callback = FindFrameCallback( rig.fake );
            ASSERT_TRUE( GUCEF_NULL != callback );

            // the compositor has not shown it yet: nothing is drawn, however often the pulses come
            window.Update( 1004 );
            window.Update( 1008 );
            window.Update( 1500 );
            ASSERT_TRUE( 1 == rig.graphics.swapCount );

            // shown: the next frame, and the callback of the one before is let go
            rig.fake.SendFrameDone( callback );
            window.Update( 1510 );
            ASSERT_TRUE( 2 == rig.graphics.swapCount );
            ASSERT_TRUE( 2 == rig.fake.CountRequestsWithPrefix( "wl_surface.frame(" ) );
            ASSERT_TRUE( 2 == counter.Count( GUI::CWindowContext::WindowContextRedrawEvent ) );
            ASSERT_TRUE( callback->destroyed );

            // a compositor that does not answer: a frame after the timeout, once a second
            window.Update( 2509 );
            ASSERT_TRUE( 2 == rig.graphics.swapCount );
            window.Update( 2510 );
            ASSERT_TRUE( 3 == rig.graphics.swapCount );
            ASSERT_TRUE( 2 == rig.fake.CountRequestsWithPrefix( "wl_surface.frame(" ) );     // the first callback is still out: no second one
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Without vertical sync the window draws at the pace of the settings and asks for no frame callbacks" )
        try
        {
            SRig rig;
            ASSERT_TRUE( rig.Connect() );
            CWaylandWindowContext window( rig.connection, rig.graphics.GetRuntime() );
            GUI::CVideoSettings settings = MakeSettings( false );
            settings.SetFrequency( 50 );
            ASSERT_TRUE( window.Initialize( "Redux", settings, MakeParams() ) );
            window.Update( 5000 );
            window.Update( 5010 );
            window.Update( 5019 );
            ASSERT_TRUE( 1 == rig.graphics.swapCount );
            window.Update( 5020 );
            ASSERT_TRUE( 2 == rig.graphics.swapCount );
            ASSERT_TRUE( 0 == rig.fake.CountRequestsWithPrefix( "wl_surface.frame(" ) );

            // vertical sync is switched on with the settings
            ASSERT_TRUE( window.ApplyVideoSettings( MakeSettings( true ) ) );
            window.Update( 5040 );
            ASSERT_TRUE( 3 == rig.graphics.swapCount );
            ASSERT_TRUE( 1 == rig.fake.CountRequestsWithPrefix( "wl_surface.frame(" ) );
            window.Update( 5060 );
            ASSERT_TRUE( 3 == rig.graphics.swapCount );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Nothing is drawn before the compositor configured the window, and everything after" )
        try
        {
            SRig rig;
            rig.fake.autoConfigure = false;
            ASSERT_TRUE( rig.Connect() );
            CWaylandWindowContext window( rig.connection, rig.graphics.GetRuntime() );
            ASSERT_TRUE( window.Initialize( "Redux", MakeSettings(), MakeParams() ) );
            ASSERT_FALSE( window.IsConfigured() );
            ASSERT_TRUE( 1280 == window.GetWidth() );                        // the size that was asked for until the compositor says
            window.Update( 100 );
            window.Update( 200 );
            ASSERT_TRUE( 0 == rig.graphics.swapCount );
            ASSERT_TRUE( 0 == rig.fake.CountRequestsWithPrefix( "xdg_surface.ack_configure" ) );

            rig.fake.Configure( 1000, 700, GUCEF_NULL, 0 );
            ASSERT_TRUE( window.IsConfigured() );
            window.Update( 300 );
            ASSERT_TRUE( 1 == rig.graphics.swapCount );
            ASSERT_TRUE( 1000 == window.GetWidth() && 700 == window.GetHeight() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Full screen is a request that the compositor answers with a size, and the floating size follows the settings" )
        try
        {
            SRig rig;
            ASSERT_TRUE( rig.Connect() );
            CWaylandWindowContext window( rig.connection, rig.graphics.GetRuntime() );
            ASSERT_TRUE( window.Initialize( "Redux", MakeSettings(), MakeParams() ) );

            ASSERT_TRUE( window.ApplyVideoSettings( MakeSettings( true, true ) ) );
            ASSERT_TRUE( rig.fake.Requested( "xdg_toplevel.set_fullscreen(null)" ) );
            ASSERT_FALSE( window.IsFullscreen() );                           // not before the compositor says so
            ASSERT_TRUE( 1280 == window.GetWidth() );
            const UInt32 fullscreen[] = { GUCEF_WL_XDG_TOPLEVEL_STATE_FULLSCREEN, GUCEF_WL_XDG_TOPLEVEL_STATE_ACTIVATED };
            rig.fake.Configure( 3840, 1080, fullscreen, 2 );
            ASSERT_TRUE( window.IsFullscreen() );
            ASSERT_TRUE( 3840 == window.GetWidth() && 1080 == window.GetHeight() );
            ASSERT_TRUE( 3840 == rig.graphics.lastWindowWidth );

            // back to a window: the compositor leaves the size to us and we go back to the one we had
            ASSERT_TRUE( window.ApplyVideoSettings( MakeSettings( true, false ) ) );
            ASSERT_TRUE( rig.fake.Requested( "xdg_toplevel.unset_fullscreen()" ) );
            const UInt32 activated[] = { GUCEF_WL_XDG_TOPLEVEL_STATE_ACTIVATED };
            rig.fake.Configure( 0, 0, activated, 1 );
            ASSERT_FALSE( window.IsFullscreen() );
            ASSERT_TRUE( 1280 == window.GetWidth() && 720 == window.GetHeight() );

            // a floating window takes the size of the settings by drawing at that size
            ASSERT_TRUE( window.ApplyVideoSettings( MakeSettings( true, false, 1024, 768 ) ) );
            ASSERT_TRUE( 1024 == window.GetWidth() && 768 == window.GetHeight() );
            ASSERT_TRUE( 1024 == rig.graphics.lastWindowWidth && 768 == rig.graphics.lastWindowHeight );
            ASSERT_TRUE( 1024 == rig.graphics.lastViewportWidth );

            // and not while the compositor decides the size
            rig.fake.Configure( 3840, 1080, fullscreen, 2 );
            ASSERT_TRUE( window.ApplyVideoSettings( MakeSettings( true, true, 800, 600 ) ) );
            ASSERT_TRUE( 3840 == window.GetWidth() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A window made full screen asks for it before it is first shown" )
        try
        {
            SRig rig;
            ASSERT_TRUE( rig.Connect() );
            CWaylandWindowContext window( rig.connection, rig.graphics.GetRuntime() );
            ASSERT_TRUE( window.Initialize( "Redux", MakeSettings( true, true ), MakeParams() ) );
            const Int32 fullscreen = IndexOfPrefix( rig.fake, "xdg_toplevel.set_fullscreen(" );
            const Int32 commit = IndexOfPrefix( rig.fake, "wl_surface.commit(" );
            ASSERT_TRUE( fullscreen >= 0 && commit > fullscreen );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Minimum and maximum size go to the compositor, and the window can ask to be moved and resized" )
        try
        {
            SRig rig;
            ASSERT_TRUE( rig.Connect() );
            CWaylandWindowContext window( rig.connection, rig.graphics.GetRuntime() );
            SWaylandWindowParams params = MakeParams();
            params.minWidth = 320;
            params.minHeight = 200;
            params.maxWidth = 4000;
            params.maxHeight = 3000;
            ASSERT_TRUE( window.Initialize( "Redux", MakeSettings(), params ) );
            ASSERT_TRUE( rig.fake.Requested( "xdg_toplevel.set_min_size(320, 200)" ) );
            ASSERT_TRUE( rig.fake.Requested( "xdg_toplevel.set_max_size(4000, 3000)" ) );

            // a click gave the compositor's serial; the move uses that of the last event of the user
            SFakeProxy* surface = rig.fake.FindProxy( "wl_surface" );
            rig.fake.SendPointerEnter( 55, surface, 10.0, 10.0 );
            rig.fake.SendPointerButton( 56, 1, 0x110, 1 );
            ASSERT_TRUE( window.BeginInteractiveMove() );
            ASSERT_TRUE( 1 == rig.fake.CountRequestsWithPrefix( "xdg_toplevel.move(<wl_seat#" ) );
            ASSERT_TRUE( rig.fake.LastRequestWithPrefix( "xdg_toplevel.move(" ).HasSubstr( ", 56)" ) >= 0 );
            ASSERT_TRUE( window.BeginInteractiveResize( GUCEF_WL_XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT ) );
            ASSERT_TRUE( rig.fake.LastRequestWithPrefix( "xdg_toplevel.resize(" ).HasSubstr( ", 56, 10)" ) >= 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A window gives everything back in order when it goes" )
        try
        {
            SRig rig;
            rig.fake.preferredFractionalScale = 120;
            ASSERT_TRUE( rig.Connect() );
            UInt32 id = 0;
            {
                CWaylandWindowContext window( rig.connection, rig.graphics.GetRuntime() );
                ASSERT_TRUE( window.Initialize( "Redux", MakeSettings(), MakeParams() ) );
                id = window.GetSurfaceId();
                window.Update( 1000 );          // a frame callback is out
                rig.fake.ClearRequests();
                window.Shutdown();
                ASSERT_TRUE( GUCEF_NULL == rig.connection.FindSurface( id ) );
            }
            ASSERT_TRUE( 1 == rig.graphics.destroySurfaceCount );
            ASSERT_TRUE( 1 == rig.graphics.windowDestroyCount );
            const Int32 callback = IndexOfPrefix( rig.fake, "proxy_destroy(wl_callback)" );
            const Int32 decoration = IndexOfPrefix( rig.fake, "zxdg_toplevel_decoration_v1.destroy(" );
            const Int32 viewport = IndexOfPrefix( rig.fake, "wp_viewport.destroy(" );
            const Int32 toplevel = IndexOfPrefix( rig.fake, "xdg_toplevel.destroy(" );
            const Int32 xdgSurface = IndexOfPrefix( rig.fake, "xdg_surface.destroy(" );
            const Int32 surface = IndexOfPrefix( rig.fake, "wl_surface.destroy(" );
            ASSERT_TRUE( callback >= 0 && decoration >= 0 && viewport >= 0 && toplevel > decoration && xdgSurface > toplevel && surface > xdgSurface );
            ASSERT_TRUE( 0 == rig.fake.CountLiveProxies( "xdg_toplevel" ) );
            ASSERT_TRUE( 0 == rig.fake.CountLiveProxies( "xdg_surface" ) );
            ASSERT_TRUE( 0 == rig.fake.CountLiveProxies( "wp_fractional_scale_v1" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A lost connection asks the application to close the window, once" )
        try
        {
            SRig rig;
            ASSERT_TRUE( rig.Connect() );
            CWaylandWindowContext window( rig.connection, rig.graphics.GetRuntime() );
            CEventCounter counter;
            counter.Watch( window, GUI::CWindowContext::WindowContextCloseRequestEvent );
            ASSERT_TRUE( window.Initialize( "Redux", MakeSettings(), MakeParams() ) );
            rig.fake.flushFails = true;
            ASSERT_FALSE( rig.connection.Pump() );
            ASSERT_TRUE( 1 == counter.Count( GUI::CWindowContext::WindowContextCloseRequestEvent ) );
            window.Update( 10 );
            window.Update( 20 );
            ASSERT_TRUE( 1 == counter.Count( GUI::CWindowContext::WindowContextCloseRequestEvent ) );
            ASSERT_TRUE( 0 == rig.graphics.swapCount );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A window that cannot get EGL fails cleanly and leaves nothing behind" )
        try
        {
            SRig rig;
            rig.graphics.noConfig = true;
            ASSERT_TRUE( rig.Connect() );
            {
                CWaylandWindowContext window( rig.connection, rig.graphics.GetRuntime() );
                ASSERT_FALSE( window.Initialize( "Redux", MakeSettings(), MakeParams() ) );
            }
            ASSERT_TRUE( 0 == rig.fake.CountLiveProxies( "wl_surface" ) );
            ASSERT_TRUE( 0 == rig.fake.CountLiveProxies( "xdg_toplevel" ) );
            ASSERT_TRUE( GUCEF_NULL == rig.connection.FindSurface( 1 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A driver without desktop OpenGL 3.3 core gets OpenGL ES 3" )
        try
        {
            SRig rig;
            rig.graphics.contextCoreFails = true;
            ASSERT_TRUE( rig.Connect() );
            CWaylandWindowContext window( rig.connection, rig.graphics.GetRuntime() );
            ASSERT_TRUE( window.Initialize( "Redux", MakeSettings(), MakeParams() ) );
            ASSERT_TRUE( "GLES" == window.GetProperty( "GL_API" ) );
            ASSERT_TRUE( "3.0" == window.GetProperty( "GL_VERSION" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The window keeps the pointer image: hidden when the GUI draws its own, a shape when asked" )
        try
        {
            SRig rig;
            ASSERT_TRUE( rig.Connect() );
            CWaylandWindowContext window( rig.connection, rig.graphics.GetRuntime() );
            ASSERT_TRUE( window.Initialize( "Redux", MakeSettings(), MakeParams() ) );
            SFakeProxy* surface = rig.fake.FindProxy( "wl_surface" );
            rig.fake.SendPointerEnter( 61, surface, 1.0, 1.0 );
            rig.fake.ClearRequests();
            ASSERT_TRUE( window.SetSystemCursorVisible( false ) );
            ASSERT_TRUE( rig.fake.Requested( "wl_pointer.set_cursor(61, null, 0, 0)" ) );
            ASSERT_TRUE( window.SetSystemCursorVisible( true ) );
            ASSERT_TRUE( rig.fake.Requested( "wp_cursor_shape_device_v1.set_shape(61, 1)" ) );
            window.SetCursorShape( CWaylandCursor::CURSORSHAPE_CROSSHAIR );
            ASSERT_TRUE( rig.fake.Requested( "wp_cursor_shape_device_v1.set_shape(61, 8)" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The backend says whether windows can be made here, and makes them with the parameters of the application" )
        try
        {
            SRig rig;
            ASSERT_TRUE( rig.Connect() );
            CWaylandWindowManagerImp manager( &rig.connection, &rig.graphics.GetRuntime() );
            ASSERT_TRUE( manager.IsAvailable() );
            ASSERT_TRUE( manager.GetAvailabilityDescription().IsNULLOrEmpty() );

            CORE::CValueList params;
            params.Set( "APP_ID", "myapp" );
            params.Set( "DECORATIONS", "none" );
            params.Set( "MIN_WIDTH", "320" );
            params.Set( "MIN_HEIGHT", "200" );
            GUI::TWindowContextPtr window = manager.CreateWindowContext( "Redux", MakeSettings(), &params );
            ASSERT_FALSE( window.IsNULL() );
            ASSERT_TRUE( rig.fake.Requested( "xdg_toplevel.set_app_id(\"myapp\")" ) );
            ASSERT_TRUE( rig.fake.Requested( "xdg_toplevel.set_min_size(320, 200)" ) );
            ASSERT_TRUE( 0 == rig.fake.CountRequestsWithPrefix( "zxdg_decoration_manager_v1" ) );
            ASSERT_TRUE( 1280 == window->GetWidth() );
            ASSERT_TRUE( manager.ApplyVideoSettings( window, MakeSettings( true, false, 800, 600 ) ) );
            ASSERT_TRUE( 800 == window->GetWidth() );
            manager.DestroyWindowContext( window );
            ASSERT_TRUE( 0 == rig.fake.CountLiveProxies( "xdg_toplevel" ) );
            window.Unlink();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The backend is not available without what a window needs, and then makes no window" )
        try
        {
            {
                // a compositor without xdg_wm_base
                CFakeWayland fake;
                CFakeGraphics graphics;
                fake.OfferGlobal( "wl_compositor", 6 );
                fake.OfferGlobal( "wl_seat", 9 );
                CWaylandConnection connection( fake.GetApi() );
                connection.SetPumpOnPulse( false );
                ASSERT_TRUE( connection.Connect() );
                CWaylandWindowManagerImp manager( &connection, &graphics.GetRuntime() );
                ASSERT_FALSE( manager.IsAvailable() );
                ASSERT_TRUE( manager.GetAvailabilityDescription().HasSubstr( "xdg_wm_base" ) >= 0 );
                ASSERT_TRUE( manager.CreateWindowContext( "Redux", MakeSettings(), GUCEF_NULL ).IsNULL() );
                ASSERT_TRUE( 0 == fake.CountRequestsWithPrefix( "wl_compositor.create_surface" ) );
            }
            {
                // no EGL
                CFakeWayland fake;
                CFakeGraphics graphics;
                fake.OfferDefaultGlobals();
                CWaylandConnection connection( fake.GetApi() );
                connection.SetPumpOnPulse( false );
                ASSERT_TRUE( connection.Connect() );
                SWaylandGuiRuntime runtime = graphics.GetRuntime();
                runtime.hasEgl = false;
                CWaylandWindowManagerImp manager( &connection, &runtime );
                ASSERT_FALSE( manager.IsAvailable() );
                ASSERT_TRUE( manager.GetAvailabilityDescription().HasSubstr( "libEGL" ) >= 0 );
                SWaylandGuiRuntime noWindowGlue = graphics.GetRuntime();
                noWindowGlue.hasEglWindow = false;
                CWaylandWindowManagerImp second( &connection, &noWindowGlue );
                ASSERT_FALSE( second.IsAvailable() );
                ASSERT_TRUE( second.GetAvailabilityDescription().HasSubstr( "libwayland-egl" ) >= 0 );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The display modes are those of the primary monitor, the one at the origin, in the size a window gets" )
        try
        {
            SRig rig;
            rig.fake.output.x = 3840;
            rig.fake.output.width = 1920;
            rig.fake.output.height = 1080;
            rig.fake.output.refreshInMilliHertz = 59940;
            SFakeOutputSpec primary;
            primary.x = 0;
            primary.width = 3840;
            primary.height = 2160;
            primary.scale = 2;
            primary.logicalWidth = 1920;
            primary.logicalHeight = 1080;
            primary.refreshInMilliHertz = 143981;
            rig.fake.moreOutputs.push_back( primary );
            rig.fake.OfferGlobal( "wl_output", 4 );
            rig.fake.OfferGlobal( "zxdg_output_manager_v1", 3 );
            ASSERT_TRUE( rig.Connect() );
            ASSERT_TRUE( 2 == rig.connection.GetOutputCount() );

            const SWaylandOutput* chosen = CWaylandDisplayModes::SelectPrimaryOutput( rig.connection );
            ASSERT_TRUE( GUCEF_NULL != chosen );
            ASSERT_TRUE( 0 == chosen->x );
            GUI::CVideoOptions::TDisplayMode mode;
            ASSERT_TRUE( CWaylandDisplayModes::GetCurrentMode( rig.connection, mode ) );
            // the monitor of 3840 x 2160 at a scale of 2 is 1920 x 1080 for a window, at 144 frames a second
            ASSERT_TRUE( 1920 == mode.widthInPixels && 1080 == mode.heightInPixels && 24 == mode.depthInBits && 144 == mode.frequency );
            GUI::CVideoOptions options;
            ASSERT_TRUE( CWaylandDisplayModes::GetOptions( rig.connection, options ) );
            ASSERT_TRUE( 1 == options.GetDisplayModes().size() );
            ASSERT_TRUE( 1920 == options.GetDisplayModes()[ 0 ].widthInPixels );
            ASSERT_TRUE( 59 == CWaylandDisplayModes::FrequencyOf( 59000 ) );
            ASSERT_TRUE( 60 == CWaylandDisplayModes::FrequencyOf( 59940 ) );
            ASSERT_TRUE( 60 == CWaylandDisplayModes::FrequencyOf( 0 ) );
            ASSERT_TRUE( 165 == CWaylandDisplayModes::FrequencyOf( 164998 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Without the logical size the physical mode is the display mode, and a first monitor stands in when none is at the origin" )
        try
        {
            SRig rig( false );
            rig.fake.output.x = 100;
            rig.fake.output.width = 2560;
            rig.fake.output.height = 1440;
            rig.fake.output.refreshInMilliHertz = 165000;
            ASSERT_TRUE( rig.Connect() );
            GUI::CVideoOptions::TDisplayMode mode;
            ASSERT_TRUE( CWaylandDisplayModes::GetCurrentMode( rig.connection, mode ) );
            ASSERT_TRUE( 2560 == mode.widthInPixels && 1440 == mode.heightInPixels && 165 == mode.frequency );

            // no monitors at all
            CFakeWayland bare;
            bare.OfferGlobal( "wl_compositor", 6 );
            CWaylandConnection connection( bare.GetApi() );
            connection.SetPumpOnPulse( false );
            ASSERT_TRUE( connection.Connect() );
            ASSERT_FALSE( CWaylandDisplayModes::GetCurrentMode( connection, mode ) );
            GUI::CVideoOptions options;
            ASSERT_FALSE( CWaylandDisplayModes::GetOptions( connection, options ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CVideoOptions asks the source of the selected window system first and the platform when that has nothing" )
        try
        {
            const CORE::CString selectedBefore = GUI::CVideoOptions::GetSelectedDisplayModeSource();
            GUI::CVideoOptions::SDisplayModeSource source;
            source.retrieveOptions = ProviderOptions;
            source.retrieveCurrentMode = ProviderCurrent;
            GUI::CVideoOptions::RegisterDisplayModeSource( "TestSource", source );

            // registered but not selected: not asked
            GUI::CVideoOptions::SelectDisplayModeSource( "" );
            GUI::CVideoOptions::TDisplayMode mode;
            GUI::CVideoOptions::RetrieveCurrentDisplayModeFromOS( mode );
            ASSERT_FALSE( 800 == mode.widthInPixels && 600 == mode.heightInPixels && 75 == mode.frequency );

            GUI::CVideoOptions::SelectDisplayModeSource( "TestSource" );
            ASSERT_TRUE( "TestSource" == GUI::CVideoOptions::GetSelectedDisplayModeSource() );
            ASSERT_TRUE( GUI::CVideoOptions::RetrieveCurrentDisplayModeFromOS( mode ) );
            ASSERT_TRUE( 800 == mode.widthInPixels && 600 == mode.heightInPixels && 75 == mode.frequency );
            GUI::CVideoOptions options;
            ASSERT_TRUE( GUI::CVideoOptions::RetrieveOptionsFromOS( options ) );
            ASSERT_TRUE( 1 == options.GetDisplayModes().size() && 800 == options.GetDisplayModes()[ 0 ].widthInPixels );

            // a source with nothing leaves it to the platform
            source.retrieveOptions = ProviderNothing;
            source.retrieveCurrentMode = ProviderNoCurrent;
            GUI::CVideoOptions::RegisterDisplayModeSource( "TestSource", source );
            GUI::CVideoOptions::TDisplayMode platformMode;
            platformMode.widthInPixels = 1;
            GUI::CVideoOptions::RetrieveCurrentDisplayModeFromOS( platformMode );
            ASSERT_FALSE( 800 == platformMode.widthInPixels && 600 == platformMode.heightInPixels && 75 == platformMode.frequency );

            // a name nobody registered selects nothing
            GUI::CVideoOptions::UnregisterDisplayModeSource( "TestSource" );
            GUI::CVideoOptions::RetrieveCurrentDisplayModeFromOS( platformMode );
            ASSERT_FALSE( 800 == platformMode.widthInPixels && 75 == platformMode.frequency );
            GUI::CVideoOptions::SelectDisplayModeSource( selectedBefore );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
