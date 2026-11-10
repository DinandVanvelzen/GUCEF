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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_H
#include "pluginglueWAYLAND.h"
#define GUCEF_PLUGINGLUE_WAYLAND_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_H ? */

#include "TestWaylandCommon.h"
#include "TestWaylandConnection.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::PLUGINGLUE::WAYLAND;
using namespace GUCEF::TESTWAYLAND;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Writes the events of the seat down */
class CSeatRecorder : public IWaylandSeatListener
{
    public:

    GUCEF::vector< CORE::CString > events;
    CWaylandConnection* removeSelfOn;

    CSeatRecorder( void ) : events(), removeSelfOn( GUCEF_NULL ) {}

    virtual void OnSeatCapabilities( UInt32 capabilities ) { events.push_back( "caps " + CORE::UInt32ToString( capabilities ) ); }
    virtual void OnPointerEnter( UInt32 serial, struct wl_surface*, wl_fixed_t x, wl_fixed_t y ) { events.push_back( "enter " + CORE::UInt32ToString( serial ) + " " + CORE::Int32ToString( gucef_wl_fixed_to_int( x ) ) + "," + CORE::Int32ToString( gucef_wl_fixed_to_int( y ) ) ); }
    virtual void OnPointerLeave( UInt32 serial, struct wl_surface* ) { events.push_back( "leave " + CORE::UInt32ToString( serial ) ); }
    virtual void OnPointerMotion( UInt32 time, wl_fixed_t x, wl_fixed_t y )
    {
        events.push_back( "motion " + CORE::Int32ToString( gucef_wl_fixed_to_int( x ) ) + "," + CORE::Int32ToString( gucef_wl_fixed_to_int( y ) ) );
        if ( GUCEF_NULL != removeSelfOn )
            removeSelfOn->RemoveSeatListener( this );
    }
    virtual void OnPointerButton( UInt32 serial, UInt32 time, UInt32 button, UInt32 state ) { events.push_back( "button " + CORE::UInt32ToString( button ) + " " + CORE::UInt32ToString( state ) ); }
    virtual void OnPointerAxis( UInt32 time, UInt32 axis, wl_fixed_t value ) { events.push_back( "axis " + CORE::UInt32ToString( axis ) + " " + CORE::Int32ToString( gucef_wl_fixed_to_int( value ) ) ); }
    virtual void OnPointerAxisSource( UInt32 source ) { events.push_back( "axis_source " + CORE::UInt32ToString( source ) ); }
    virtual void OnPointerAxisStop( UInt32, UInt32 axis ) { events.push_back( "axis_stop " + CORE::UInt32ToString( axis ) ); }
    virtual void OnPointerAxisDiscrete( UInt32 axis, Int32 discrete ) { events.push_back( "axis_discrete " + CORE::UInt32ToString( axis ) + " " + CORE::Int32ToString( discrete ) ); }
    virtual void OnPointerAxisValue120( UInt32 axis, Int32 value120 ) { events.push_back( "axis_value120 " + CORE::UInt32ToString( axis ) + " " + CORE::Int32ToString( value120 ) ); }
    virtual void OnPointerFrame( void ) { events.push_back( "frame" ); }
    virtual void OnKeyboardKeymap( UInt32 format, const char* data, UInt32 size ) { events.push_back( "keymap " + CORE::UInt32ToString( format ) + " " + CORE::UInt32ToString( size ) + " " + CORE::CString( data, 9 ) ); }
    virtual void OnKeyboardEnter( UInt32 serial, struct wl_surface*, const UInt32*, UInt32 ) { events.push_back( "kenter " + CORE::UInt32ToString( serial ) ); }
    virtual void OnKeyboardLeave( UInt32 serial, struct wl_surface* ) { events.push_back( "kleave " + CORE::UInt32ToString( serial ) ); }
    virtual void OnKeyboardKey( UInt32 serial, UInt32, UInt32 key, UInt32 state ) { events.push_back( "key " + CORE::UInt32ToString( key ) + " " + CORE::UInt32ToString( state ) ); }
    virtual void OnKeyboardModifiers( UInt32, UInt32 depressed, UInt32, UInt32, UInt32 ) { events.push_back( "mods " + CORE::UInt32ToString( depressed ) ); }
    virtual void OnKeyboardRepeatInfo( Int32 rate, Int32 delay ) { events.push_back( "repeat " + CORE::Int32ToString( rate ) + " " + CORE::Int32ToString( delay ) ); }

    bool Has( const char* text ) const
    {
        for ( UInt32 i=0; i<events.size(); ++i )
            if ( events[ i ] == text )
                return true;
        return false;
    }
};

/** A cursor theme with one cursor of 24 x 24 pixels: the pictures of a theme need no files */
struct SFakeCursorTheme
{
    UInt32 loads;
    int lastSize;
    CORE::CString lastName;
    struct wl_cursor_image image;
    struct wl_cursor_image* images[ 1 ];
    struct wl_cursor cursor;
    char name[ 8 ];
    bool hasCursors;
};

SFakeCursorTheme g_theme;
SFakeProxy g_bufferProxy;

struct wl_cursor_theme*
T_load( const char*, int size, struct wl_shm* )
{
    ++g_theme.loads;
    g_theme.lastSize = size;
    return (struct wl_cursor_theme*) &g_theme;
}

void T_destroy( struct wl_cursor_theme* ) {}

struct wl_cursor*
T_getCursor( struct wl_cursor_theme*, const char* name )
{
    g_theme.lastName = name;
    if ( !g_theme.hasCursors )
        return GUCEF_NULL;
    return &g_theme.cursor;
}

struct wl_buffer* T_getBuffer( struct wl_cursor_image* ) { return (struct wl_buffer*) &g_bufferProxy; }

void
SetUpFakeTheme( SWaylandCursorApi& api )
{
    memset( &g_theme, 0, sizeof( g_theme ) );
    g_theme.image.width = 24;
    g_theme.image.height = 24;
    g_theme.image.hotspot_x = 4;
    g_theme.image.hotspot_y = 2;
    g_theme.images[ 0 ] = &g_theme.image;
    g_theme.cursor.image_count = 1;
    g_theme.cursor.images = g_theme.images;
    g_theme.cursor.name = g_theme.name;
    g_theme.hasCursors = true;
    g_bufferProxy.iface = &gucef_wl_buffer_interface;
    g_bufferProxy.id = 999;
    api.theme_load = T_load;
    api.theme_destroy = T_destroy;
    api.theme_get_cursor = T_getCursor;
    api.image_get_buffer = T_getBuffer;
}

void
OfferEverything( CFakeWayland& fake )
{
    fake.OfferDefaultGlobals();
    fake.OfferGlobal( "wp_viewporter", 1 );
    fake.OfferGlobal( "wp_fractional_scale_manager_v1", 1 );
    fake.OfferGlobal( "zxdg_decoration_manager_v1", 1 );
    fake.OfferGlobal( "wp_cursor_shape_manager_v1", 1 );
    fake.OfferGlobal( "zxdg_output_manager_v1", 3 );
}

void
OnSocketPing( void* )
{
    CFakeWayland::g_current->SendPing( 4242 );
}

}

/*-------------------------------------------------------------------------*/

void
PerformWaylandConnectionTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Wayland connection TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "WaylandConnection" );

    GUCEF_TESTFW_TESTCASE( "Test: Connecting binds the globals the plugins use at the versions they support and no more" )
        try
        {
            CFakeWayland fake;
            fake.OfferGlobal( "wl_compositor", 7 );
            fake.OfferGlobal( "xdg_wm_base", 7 );
            fake.OfferGlobal( "wl_shm", 1 );
            fake.OfferGlobal( "wl_seat", 10 );
            fake.OfferGlobal( "wl_output", 4 );
            fake.OfferGlobal( "wl_subcompositor", 1 );               // not used: not bound
            fake.OfferGlobal( "wp_viewporter", 1 );
            CWaylandConnection connection( fake.GetApi() );
            connection.SetPumpOnPulse( false );
            CORE::CString problem;
            ASSERT_TRUE( connection.Connect( &problem ) );
            ASSERT_TRUE( connection.IsConnected() && connection.IsHealthy() );
            ASSERT_TRUE( 1 == fake.connectCalls );

            ASSERT_TRUE( GUCEF_NULL != connection.GetCompositor() );
            ASSERT_TRUE( 6 == connection.GetCompositorVersion() );
            ASSERT_TRUE( GUCEF_NULL != connection.GetWmBase() );
            ASSERT_TRUE( GUCEF_NULL != connection.GetShm() );
            ASSERT_TRUE( GUCEF_NULL != connection.GetViewporter() );
            ASSERT_TRUE( GUCEF_NULL == connection.GetFractionalScaleManager() );
            ASSERT_TRUE( GUCEF_NULL == connection.GetDecorationManager() );
            ASSERT_TRUE( GUCEF_NULL == connection.GetCursorShapeManager() );
            ASSERT_TRUE( GUCEF_NULL == connection.GetOutputManager() );
            ASSERT_TRUE( 9 == connection.GetSeatVersion() );
            ASSERT_TRUE( fake.Requested( "wl_registry.bind(1, \"wl_compositor\", 6)" ) );
            ASSERT_TRUE( fake.Requested( "wl_registry.bind(2, \"xdg_wm_base\", 6)" ) );
            ASSERT_TRUE( fake.Requested( "wl_registry.bind(4, \"wl_seat\", 9)" ) );
            ASSERT_TRUE( fake.Requested( "wl_registry.bind(5, \"wl_output\", 4)" ) );
            ASSERT_TRUE( 0 == fake.CountRequestsWithPrefix( "wl_registry.bind(6" ) );

            ASSERT_TRUE( connection.HasGlobal( "wl_subcompositor" ) );
            ASSERT_TRUE( connection.HasGlobal( "wl_compositor", 7 ) );
            ASSERT_FALSE( connection.HasGlobal( "wl_compositor", 8 ) );
            ASSERT_TRUE( 7 == connection.GetGlobals().size() );

            connection.Disconnect();
            ASSERT_FALSE( connection.IsConnected() );
            ASSERT_TRUE( 1 == fake.disconnectCalls );
            ASSERT_TRUE( 0 == fake.CountLiveProxies( "wl_compositor" ) );
            ASSERT_TRUE( 0 == fake.CountLiveProxies( "xdg_wm_base" ) );
            ASSERT_TRUE( 0 == fake.CountLiveProxies( "wl_seat" ) );
            ASSERT_TRUE( 0 == fake.CountLiveProxies( "wl_pointer" ) );
            ASSERT_TRUE( 0 == fake.CountLiveProxies( "wl_output" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A compositor that does not accept the connection is a failure with a reason, not a crash" )
        try
        {
            CFakeWayland fake;
            fake.OfferDefaultGlobals();
            fake.connectFails = true;
            CWaylandConnection connection( fake.GetApi() );
            connection.SetPumpOnPulse( false );
            CORE::CString problem;
            ASSERT_FALSE( connection.Connect( &problem ) );
            ASSERT_FALSE( problem.IsNULLOrEmpty() );
            ASSERT_FALSE( connection.IsConnected() );
            ASSERT_FALSE( connection.Pump() );
            ASSERT_TRUE( GUCEF_NULL == connection.GetCompositor() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The extensions of the compositor are bound when it offers them" )
        try
        {
            CFakeWayland fake;
            OfferEverything( fake );
            CWaylandConnection connection( fake.GetApi() );
            connection.SetPumpOnPulse( false );
            ASSERT_TRUE( connection.Connect() );
            ASSERT_TRUE( GUCEF_NULL != connection.GetViewporter() );
            ASSERT_TRUE( GUCEF_NULL != connection.GetFractionalScaleManager() );
            ASSERT_TRUE( GUCEF_NULL != connection.GetDecorationManager() );
            ASSERT_TRUE( GUCEF_NULL != connection.GetCursorShapeManager() );
            ASSERT_TRUE( GUCEF_NULL != connection.GetOutputManager() );
            ASSERT_TRUE( fake.Requested( "wl_registry.bind(6, \"wp_viewporter\", 1)" ) );
            ASSERT_TRUE( fake.Requested( "wl_registry.bind(10, \"zxdg_output_manager_v1\", 3)" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Monitors are tracked with their modes, scale and logical size" )
        try
        {
            CFakeWayland fake;
            OfferEverything( fake );
            fake.output.logicalWidth = 3840;
            fake.output.logicalHeight = 1080;
            CWaylandConnection connection( fake.GetApi() );
            connection.SetPumpOnPulse( false );
            ASSERT_TRUE( connection.Connect() );
            ASSERT_TRUE( 1 == connection.GetOutputCount() );
            const SWaylandOutput* output = connection.GetOutput( 0 );
            ASSERT_TRUE( GUCEF_NULL != output );
            ASSERT_TRUE( output->done );
            ASSERT_TRUE( 1 == output->scale );
            ASSERT_TRUE( "DP-1" == output->name );
            ASSERT_TRUE( 1 == output->modes.size() );
            const SWaylandOutputMode* mode = output->GetCurrentMode();
            ASSERT_TRUE( GUCEF_NULL != mode );
            ASSERT_TRUE( 3840 == mode->width && 1080 == mode->height && 144000 == mode->refreshInMilliHertz );
            ASSERT_TRUE( output->hasLogicalSize );
            ASSERT_TRUE( 3840 == output->logicalWidth && 1080 == output->logicalHeight );
            ASSERT_TRUE( output == connection.FindOutput( output->proxy ) );

            // a monitor that came later
            CEventCounter counter;
            counter.Watch( connection, CWaylandConnection::OutputsChangedEvent );
            fake.OfferGlobalLater( "wl_output", 4 );
            ASSERT_TRUE( connection.Pump() );
            ASSERT_TRUE( 2 == connection.GetOutputCount() );
            ASSERT_TRUE( counter.Count( CWaylandConnection::OutputsChangedEvent ) >= 1 );

            // the scale of a monitor changes
            SFakeProxy* proxy = fake.FindProxy( "wl_output", 1 );
            ASSERT_TRUE( GUCEF_NULL != proxy );
            fake.SendOutputScale( proxy, 2 );
            fake.SendOutputDone( proxy );
            ASSERT_TRUE( 2 == connection.FindOutput( (struct wl_output*) proxy )->scale );

            // and one is unplugged
            const UInt32 before = counter.Count( CWaylandConnection::OutputsChangedEvent );
            fake.WithdrawGlobal( "wl_output" );
            ASSERT_TRUE( 1 == connection.GetOutputCount() );
            ASSERT_TRUE( counter.Count( CWaylandConnection::OutputsChangedEvent ) > before );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The seat gives a pointer and a keyboard as it has them, and they come and go" )
        try
        {
            CFakeWayland fake;
            fake.OfferDefaultGlobals();
            CWaylandConnection connection( fake.GetApi() );
            connection.SetPumpOnPulse( false );
            ASSERT_TRUE( connection.Connect() );
            ASSERT_TRUE( 3 == connection.GetSeatCapabilities() );
            ASSERT_TRUE( GUCEF_NULL != connection.GetPointer() );
            ASSERT_TRUE( GUCEF_NULL != connection.GetKeyboard() );

            CSeatRecorder recorder;
            connection.AddSeatListener( &recorder );
            ASSERT_TRUE( recorder.Has( "caps 3" ) );                        // told at once of what is there

            // the keyboard goes: released (version 3 and newer) and the listener told
            fake.SendSeatCapabilities( 1 );
            ASSERT_TRUE( GUCEF_NULL == connection.GetKeyboard() );
            ASSERT_TRUE( GUCEF_NULL != connection.GetPointer() );
            ASSERT_TRUE( fake.Requested( "wl_keyboard.release()" ) );
            ASSERT_TRUE( recorder.Has( "caps 1" ) );
            ASSERT_TRUE( 0 == fake.CountLiveProxies( "wl_keyboard" ) );

            // a keyboard is plugged in again
            fake.SendSeatCapabilities( 3 );
            ASSERT_TRUE( GUCEF_NULL != connection.GetKeyboard() );
            ASSERT_TRUE( 1 == fake.CountLiveProxies( "wl_keyboard" ) );

            // everything goes
            fake.SendSeatCapabilities( 0 );
            ASSERT_TRUE( GUCEF_NULL == connection.GetPointer() && GUCEF_NULL == connection.GetKeyboard() );
            ASSERT_TRUE( fake.Requested( "wl_pointer.release()" ) );
            connection.RemoveSeatListener( &recorder );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A seat without a pointer capability gets no pointer until it has one" )
        try
        {
            CFakeWayland fake;
            fake.OfferDefaultGlobals();
            fake.seatCapabilities = 2;
            CWaylandConnection connection( fake.GetApi() );
            connection.SetPumpOnPulse( false );
            ASSERT_TRUE( connection.Connect() );
            ASSERT_TRUE( GUCEF_NULL == connection.GetPointer() );
            ASSERT_TRUE( GUCEF_NULL != connection.GetKeyboard() );
            ASSERT_TRUE( 0 == fake.CountRequestsWithPrefix( "wl_seat.get_pointer" ) );
            fake.SendSeatCapabilities( 3 );
            ASSERT_TRUE( GUCEF_NULL != connection.GetPointer() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Events of the pointer and the keyboard reach every listener, and a listener that comes late hears what a keyboard said once" )
        try
        {
            CFakeWayland fake;
            fake.OfferDefaultGlobals();
            fake.keymapText = "xkb_keymap { dummy }";
            fake.repeatRate = 30;
            fake.repeatDelay = 500;
            CWaylandConnection connection( fake.GetApi() );
            connection.SetPumpOnPulse( false );
            ASSERT_TRUE( connection.Connect() );

            CSeatRecorder first;
            connection.AddSeatListener( &first );
            // the keymap and the repeat rate came while connecting: the first listener is told as it is added
            ASSERT_TRUE( first.Has( "repeat 30 500" ) );
            ASSERT_TRUE( first.Has( "keymap 1 21 xkb_keyma" ) );

            SFakeProxy* surface = fake.FindProxy( "wl_compositor" );        // any proxy will do as a surface to point at
            fake.SendPointerEnter( 11, surface, 10.0, 20.0 );
            fake.SendPointerMotion( 1, 30.0, 40.0 );
            fake.SendPointerButton( 12, 2, 0x110, 1 );
            fake.SendPointerAxisValue120( 0, 120 );
            fake.SendPointerAxisDiscrete( 1, -1 );
            fake.SendPointerAxisSource( 0 );
            fake.SendPointerAxisStop( 3, 0 );
            fake.SendPointerAxis( 4, 0, 15.0 );
            fake.SendPointerFrame();
            fake.SendKeyboardEnter( 13, surface );
            fake.SendKeyboardKey( 14, 5, 30, 1 );
            fake.SendKeyboardModifiers( 15, 1, 0, 0, 0 );
            fake.SendPointerLeave( 16, surface );
            fake.SendKeyboardLeave( 17, surface );
            ASSERT_TRUE( first.Has( "enter 11 10,20" ) && first.Has( "motion 30,40" ) && first.Has( "button 272 1" ) );
            ASSERT_TRUE( first.Has( "axis_value120 0 120" ) && first.Has( "axis_discrete 1 -1" ) && first.Has( "axis_source 0" ) );
            ASSERT_TRUE( first.Has( "axis_stop 0" ) && first.Has( "axis 0 15" ) && first.Has( "frame" ) );
            ASSERT_TRUE( first.Has( "kenter 13" ) && first.Has( "key 30 1" ) && first.Has( "mods 1" ) );
            ASSERT_TRUE( first.Has( "leave 16" ) && first.Has( "kleave 17" ) );
            ASSERT_TRUE( 14 == connection.GetLastInputSerial() );        // the last event of the user: the key

            CSeatRecorder late;
            connection.AddSeatListener( &late );
            ASSERT_TRUE( late.Has( "caps 3" ) && late.Has( "repeat 30 500" ) && late.Has( "keymap 1 21 xkb_keyma" ) );
            connection.RemoveSeatListener( &first );
            connection.RemoveSeatListener( &late );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A listener that is added while the pointer has the focus is told where it is" )
        try
        {
            CFakeWayland fake;
            fake.OfferDefaultGlobals();
            CWaylandConnection connection( fake.GetApi() );
            connection.SetPumpOnPulse( false );
            ASSERT_TRUE( connection.Connect() );
            SFakeProxy* surface = fake.FindProxy( "wl_compositor" );
            fake.SendPointerEnter( 21, surface, 7.0, 9.0 );
            fake.SendPointerMotion( 1, 11.0, 13.0 );
            ASSERT_TRUE( 21 == connection.GetPointerEnterSerial() );
            ASSERT_TRUE( (void*) surface == (void*) connection.GetPointerFocus() );
            CSeatRecorder late;
            connection.AddSeatListener( &late );
            ASSERT_TRUE( late.Has( "enter 21 11,13" ) );
            fake.SendKeyboardEnter( 22, surface );
            CSeatRecorder later;
            connection.AddSeatListener( &later );
            ASSERT_TRUE( later.Has( "kenter 22" ) );
            fake.SendPointerLeave( 23, surface );
            ASSERT_TRUE( GUCEF_NULL == connection.GetPointerFocus() );
            connection.RemoveSeatListener( &late );
            connection.RemoveSeatListener( &later );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A listener may remove itself from within an event" )
        try
        {
            CFakeWayland fake;
            fake.OfferDefaultGlobals();
            CWaylandConnection connection( fake.GetApi() );
            connection.SetPumpOnPulse( false );
            ASSERT_TRUE( connection.Connect() );
            CSeatRecorder leaving;
            CSeatRecorder staying;
            leaving.removeSelfOn = &connection;
            connection.AddSeatListener( &leaving );
            connection.AddSeatListener( &staying );
            fake.SendPointerMotion( 1, 1.0, 1.0 );
            fake.SendPointerMotion( 2, 2.0, 2.0 );
            ASSERT_TRUE( leaving.Has( "motion 1,1" ) && !leaving.Has( "motion 2,2" ) );
            ASSERT_TRUE( staying.Has( "motion 1,1" ) && staying.Has( "motion 2,2" ) );
            connection.RemoveSeatListener( &staying );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Pings of the compositor are answered when the connection is pumped" )
        try
        {
            CFakeWayland fake;
            fake.OfferDefaultGlobals();
            CWaylandConnection connection( fake.GetApi() );
            connection.SetPumpOnPulse( false );
            ASSERT_TRUE( connection.Connect() );
            fake.SendPing( 77 );
            ASSERT_TRUE( fake.Requested( "xdg_wm_base.pong(77)" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Pumping reads the socket only when it has data and pairs every prepare of a read with a read or a cancel" )
        try
        {
            CFakeWayland fake;
            fake.OfferDefaultGlobals();
            CWaylandConnection connection( fake.GetApi() );
            connection.SetPumpOnPulse( false );
            ASSERT_TRUE( connection.Connect() );
            fake.ClearRequests();

            // nothing on the socket: the read is cancelled, nothing blocks
            const UInt32 prepared = fake.prepareReadCalls;
            ASSERT_TRUE( connection.Pump() );
            ASSERT_TRUE( 1 == fake.prepareReadCalls - prepared );
            ASSERT_TRUE( 0 == fake.readEventsCalls );
            ASSERT_TRUE( 1 == fake.cancelReadCalls );
            ASSERT_TRUE( fake.flushCalls >= 2 );
            ASSERT_TRUE( 0 == fake.CountRequestsWithPrefix( "xdg_wm_base.pong" ) );

            // a ping on the socket: read, dispatched and answered in the same pump
            fake.PostFromSocket( OnSocketPing, GUCEF_NULL );
            ASSERT_TRUE( connection.Pump() );
            ASSERT_TRUE( 1 == fake.readEventsCalls );
            ASSERT_TRUE( 1 == fake.cancelReadCalls );
            ASSERT_TRUE( fake.Requested( "xdg_wm_base.pong(4242)" ) );

            // and the pump has nothing left to read
            ASSERT_TRUE( connection.Pump() );
            ASSERT_TRUE( 1 == fake.readEventsCalls );
            ASSERT_TRUE( 2 == fake.cancelReadCalls );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A broken connection is reported once and stops the pump" )
        try
        {
            CFakeWayland fake;
            fake.OfferDefaultGlobals();
            CWaylandConnection connection( fake.GetApi() );
            connection.SetPumpOnPulse( false );
            ASSERT_TRUE( connection.Connect() );
            CEventCounter counter;
            counter.Watch( connection, CWaylandConnection::ConnectionLostEvent );
            fake.flushFails = true;
            fake.errorCode = 32;
            ASSERT_FALSE( connection.Pump() );
            ASSERT_FALSE( connection.IsHealthy() );
            ASSERT_TRUE( 1 == counter.Count( CWaylandConnection::ConnectionLostEvent ) );
            ASSERT_FALSE( connection.Pump() );
            ASSERT_TRUE( 1 == counter.Count( CWaylandConnection::ConnectionLostEvent ) );
            // the pair of the failed flush: the read that was prepared is cancelled
            ASSERT_TRUE( fake.cancelReadCalls >= 1 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Surfaces of windows are registered under an id the input driver can find them by" )
        try
        {
            CFakeWayland fake;
            fake.OfferDefaultGlobals();
            CWaylandConnection connection( fake.GetApi() );
            connection.SetPumpOnPulse( false );
            ASSERT_TRUE( connection.Connect() );
            struct wl_surface* surfaceA = (struct wl_surface*) 0x10;
            struct wl_surface* surfaceB = (struct wl_surface*) 0x20;
            const UInt32 a = connection.RegisterSurface( surfaceA );
            const UInt32 b = connection.RegisterSurface( surfaceB );
            ASSERT_TRUE( 0 != a && 0 != b && a != b );
            ASSERT_TRUE( surfaceA == connection.FindSurface( a )->surface );
            ASSERT_TRUE( 120 == connection.FindSurface( a )->scale120 );
            connection.SetSurfaceScale( b, 180 );
            ASSERT_TRUE( 180 == connection.FindSurface( b )->scale120 );
            ASSERT_TRUE( connection.FindSurface( b ) == connection.FindSurfaceByProxy( surfaceB ) );
            ASSERT_TRUE( GUCEF_NULL == connection.FindSurfaceByProxy( (struct wl_surface*) 0x30 ) );
            connection.UnregisterSurface( a );
            ASSERT_TRUE( GUCEF_NULL == connection.FindSurface( a ) );
            ASSERT_TRUE( GUCEF_NULL != connection.FindSurface( b ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The cursor is set again each time the pointer enters a window, by shape when the compositor knows shapes" )
        try
        {
            CFakeWayland fake;
            OfferEverything( fake );
            CWaylandConnection connection( fake.GetApi() );
            connection.SetPumpOnPulse( false );
            ASSERT_TRUE( connection.Connect() );
            SFakeProxy* ours = fake.FindProxy( "wl_compositor" );           // stands for the surface of our window
            const UInt32 id = connection.RegisterSurface( (struct wl_surface*) ours );
            ASSERT_TRUE( 0 != id );
            ASSERT_TRUE( 1 == fake.CountLiveProxies( "wp_cursor_shape_device_v1" ) );
            fake.ClearRequests();

            fake.SendPointerEnter( 31, ours, 5.0, 5.0 );
            ASSERT_TRUE( fake.Requested( "wp_cursor_shape_device_v1.set_shape(31, 1)" ) );

            connection.GetCursor().SetShape( CWaylandCursor::CURSORSHAPE_TEXT );
            ASSERT_TRUE( fake.Requested( "wp_cursor_shape_device_v1.set_shape(31, 9)" ) );

            // hidden: the pointer gets no picture
            connection.GetCursor().SetVisible( false );
            ASSERT_TRUE( fake.Requested( "wl_pointer.set_cursor(31, null, 0, 0)" ) );
            connection.GetCursor().SetVisible( true );
            ASSERT_TRUE( 2 <= fake.CountRequestsWithPrefix( "wp_cursor_shape_device_v1.set_shape(31, 9)" ) );

            // away from the window nothing is set; entering again sets it with the new serial
            fake.SendPointerLeave( 32, ours );
            fake.ClearRequests();
            connection.GetCursor().SetShape( CWaylandCursor::CURSORSHAPE_POINTER );
            ASSERT_TRUE( 0 == fake.CountRequestsWithPrefix( "wp_cursor_shape_device_v1" ) );
            fake.SendPointerEnter( 33, ours, 5.0, 5.0 );
            ASSERT_TRUE( fake.Requested( "wp_cursor_shape_device_v1.set_shape(33, 4)" ) );

            // the pointer entering a surface that is not ours (the decorations of libdecor) is not ours to set
            SFakeProxy* foreign = fake.FindProxy( "xdg_wm_base" );
            fake.ClearRequests();
            fake.SendPointerEnter( 34, foreign, 1.0, 1.0 );
            ASSERT_TRUE( 0 == fake.CountRequestsWithPrefix( "wp_cursor_shape_device_v1" ) );
            connection.UnregisterSurface( id );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Without shapes the cursor is a picture of the cursor theme at the scale of the window" )
        try
        {
            CFakeWayland fake;
            fake.OfferDefaultGlobals();
            CWaylandConnection connection( fake.GetApi() );
            connection.SetPumpOnPulse( false );
            ASSERT_TRUE( connection.Connect() );
            SWaylandCursorApi cursorApi;
            SetUpFakeTheme( cursorApi );
            connection.GetCursor().SetCursorApi( cursorApi );
            SFakeProxy* ours = fake.FindProxy( "wl_compositor" );
            const UInt32 id = connection.RegisterSurface( (struct wl_surface*) ours );
            connection.SetSurfaceScale( id, 240 );
            fake.ClearRequests();

            fake.SendPointerEnter( 41, ours, 5.0, 5.0 );
            ASSERT_TRUE( 1 == g_theme.loads );
            ASSERT_TRUE( 48 == g_theme.lastSize );                          // 24 pixels at a scale of 2
            ASSERT_TRUE( "default" == g_theme.lastName );
            ASSERT_TRUE( fake.Requested( "wl_surface.set_buffer_scale(2)" ) );
            ASSERT_TRUE( fake.Requested( "wl_surface.damage_buffer(0, 0, 24, 24)" ) );
            ASSERT_TRUE( 1 == fake.CountRequestsWithPrefix( "wl_surface.commit" ) );
            const CORE::CString setCursor = "wl_pointer.set_cursor(41, <wl_surface#" + CORE::UInt32ToString( fake.FindProxy( "wl_surface" )->id ) + ">, 2, 1)";
            ASSERT_TRUE( fake.Requested( setCursor.C_String() ) );     // the hotspot in surface units

            // a theme that does not have the cursor: the older name, then the arrow
            g_theme.hasCursors = false;
            connection.GetCursor().SetShape( CWaylandCursor::CURSORSHAPE_TEXT );
            ASSERT_TRUE( "left_ptr" == g_theme.lastName );
            connection.UnregisterSurface( id );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
