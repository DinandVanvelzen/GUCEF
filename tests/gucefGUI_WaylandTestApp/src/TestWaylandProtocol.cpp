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

#include <stddef.h>
#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_H
#include "pluginglueWAYLAND.h"
#define GUCEF_PLUGINGLUE_WAYLAND_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_H ? */

#ifndef GUCEF_GUIDRIVERWAYLAND_CDECORATIONPOLICY_H
#include "guidriverWAYLAND_CDecorationPolicy.h"
#define GUCEF_GUIDRIVERWAYLAND_CDECORATIONPOLICY_H
#endif /* GUCEF_GUIDRIVERWAYLAND_CDECORATIONPOLICY_H ? */

#ifndef GUCEF_GUIDRIVERWAYLAND_CXDGCONFIGURESTATE_H
#include "guidriverWAYLAND_CXdgConfigureState.h"
#define GUCEF_GUIDRIVERWAYLAND_CXDGCONFIGURESTATE_H
#endif /* GUCEF_GUIDRIVERWAYLAND_CXDGCONFIGURESTATE_H ? */

#ifndef GUCEF_GUIDRIVERWAYLAND_CWAYLANDFRAMEPACER_H
#include "guidriverWAYLAND_CWaylandFramePacer.h"
#define GUCEF_GUIDRIVERWAYLAND_CWAYLANDFRAMEPACER_H
#endif /* GUCEF_GUIDRIVERWAYLAND_CWAYLANDFRAMEPACER_H ? */

#include "TestWaylandCommon.h"
#include "TestWaylandProtocol.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::PLUGINGLUE::WAYLAND;
using namespace GUCEF::GUIDRIVERWAYLAND;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The names of the events of an interface in order are the slots of its listener structure: a mismatch would call the wrong function */
static bool
EventsAre( const struct wl_interface& iface, const char* const* names, int count )
{
    if ( iface.event_count != count )
        return false;
    for ( int i=0; i<count; ++i )
        if ( 0 != strcmp( iface.events[ i ].name, names[ i ] ) )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
RequestIs( const struct wl_interface& iface, int opcode, const char* name, const char* signature )
{
    return opcode < iface.method_count && 0 == strcmp( iface.methods[ opcode ].name, name ) && 0 == strcmp( iface.methods[ opcode ].signature, signature );
}

/*-------------------------------------------------------------------------*/

void
PerformWaylandProtocolTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Wayland protocol and logic TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "WaylandProtocol" );

    GUCEF_TESTFW_TESTCASE( "Test: The interface tables carry the names, versions and wire signatures of the protocols" )
        try
        {
            ASSERT_TRUE( 0 == strcmp( gucef_xdg_wm_base_interface.name, "xdg_wm_base" ) );
            ASSERT_TRUE( 7 == gucef_xdg_wm_base_interface.version );
            ASSERT_TRUE( RequestIs( gucef_xdg_wm_base_interface, 2, "get_xdg_surface", "no" ) );
            ASSERT_TRUE( RequestIs( gucef_xdg_wm_base_interface, 3, "pong", "u" ) );
            ASSERT_TRUE( RequestIs( gucef_xdg_surface_interface, 4, "ack_configure", "u" ) );
            ASSERT_TRUE( RequestIs( gucef_xdg_surface_interface, 3, "set_window_geometry", "iiii" ) );
            ASSERT_TRUE( RequestIs( gucef_xdg_toplevel_interface, 2, "set_title", "s" ) );
            ASSERT_TRUE( RequestIs( gucef_xdg_toplevel_interface, 3, "set_app_id", "s" ) );
            ASSERT_TRUE( RequestIs( gucef_xdg_toplevel_interface, 11, "set_fullscreen", "?o" ) );
            ASSERT_TRUE( RequestIs( gucef_xdg_toplevel_interface, 5, "move", "ou" ) );
            ASSERT_TRUE( RequestIs( gucef_xdg_toplevel_interface, 6, "resize", "ouu" ) );
            ASSERT_TRUE( 0 == strcmp( gucef_xdg_toplevel_interface.events[ 0 ].signature, "iia" ) );
            ASSERT_TRUE( 0 == strcmp( gucef_xdg_toplevel_interface.events[ 2 ].signature, "4ii" ) );       // configure_bounds, since version 4
            ASSERT_TRUE( 0 == strcmp( gucef_xdg_toplevel_interface.events[ 3 ].signature, "5a" ) );        // wm_capabilities, since version 5

            // the registry: bind takes a name, the name of the interface, its version and the new object
            ASSERT_TRUE( RequestIs( gucef_wl_registry_interface, 0, "bind", "usun" ) );
            ASSERT_TRUE( RequestIs( gucef_wl_compositor_interface, 0, "create_surface", "n" ) );
            ASSERT_TRUE( RequestIs( gucef_wl_surface_interface, 6, "commit", "" ) );
            ASSERT_TRUE( RequestIs( gucef_wl_surface_interface, 8, "set_buffer_scale", "3i" ) );
            ASSERT_TRUE( RequestIs( gucef_wl_surface_interface, 9, "damage_buffer", "4iiii" ) );
            ASSERT_TRUE( RequestIs( gucef_wl_pointer_interface, 0, "set_cursor", "u?oii" ) );
            ASSERT_TRUE( RequestIs( gucef_wp_viewport_interface, 2, "set_destination", "ii" ) );
            ASSERT_TRUE( RequestIs( gucef_wp_viewport_interface, 1, "set_source", "ffff" ) );
            ASSERT_TRUE( RequestIs( gucef_zxdg_toplevel_decoration_v1_interface, 1, "set_mode", "u" ) );
            ASSERT_TRUE( RequestIs( gucef_wp_cursor_shape_device_v1_interface, 1, "set_shape", "uu" ) );

            // xdg-activation: a global with one version, a token that is made from a serial and a seat, an application id and a surface
            ASSERT_TRUE( 0 == strcmp( gucef_xdg_activation_v1_interface.name, "xdg_activation_v1" ) && 1 == gucef_xdg_activation_v1_interface.version );
            ASSERT_TRUE( RequestIs( gucef_xdg_activation_v1_interface, 1, "get_activation_token", "n" ) );
            ASSERT_TRUE( RequestIs( gucef_xdg_activation_v1_interface, 2, "activate", "so" ) );
            ASSERT_TRUE( 0 == strcmp( gucef_xdg_activation_token_v1_interface.name, "xdg_activation_token_v1" ) && 1 == gucef_xdg_activation_token_v1_interface.version );
            ASSERT_TRUE( RequestIs( gucef_xdg_activation_token_v1_interface, 0, "set_serial", "uo" ) );
            ASSERT_TRUE( RequestIs( gucef_xdg_activation_token_v1_interface, 1, "set_app_id", "s" ) );
            ASSERT_TRUE( RequestIs( gucef_xdg_activation_token_v1_interface, 2, "set_surface", "o" ) );
            ASSERT_TRUE( RequestIs( gucef_xdg_activation_token_v1_interface, 3, "commit", "" ) );
            ASSERT_TRUE( RequestIs( gucef_xdg_activation_token_v1_interface, 4, "destroy", "" ) );
            ASSERT_TRUE( 1 == gucef_xdg_activation_token_v1_interface.event_count && 0 == strcmp( gucef_xdg_activation_token_v1_interface.events[ 0 ].name, "done" ) && 0 == strcmp( gucef_xdg_activation_token_v1_interface.events[ 0 ].signature, "s" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The order of the events in the tables is the order of the slots of the listener structures" )
        try
        {
            const char* pointer[] = { "enter", "leave", "motion", "button", "axis", "frame", "axis_source", "axis_stop", "axis_discrete", "axis_value120", "axis_relative_direction" };
            ASSERT_TRUE( gucef_wl_pointer_interface.event_count >= 11 );
            for ( int i=0; i<11; ++i )
                ASSERT_TRUE( 0 == strcmp( gucef_wl_pointer_interface.events[ i ].name, pointer[ i ] ) );
            ASSERT_TRUE( offsetof( struct gucef_wl_pointer_listener, axis_value120 ) == 9 * sizeof( void* ) );
            ASSERT_TRUE( offsetof( struct gucef_wl_pointer_listener, frame ) == 5 * sizeof( void* ) );

            const char* keyboard[] = { "keymap", "enter", "leave", "key", "modifiers", "repeat_info" };
            ASSERT_TRUE( EventsAre( gucef_wl_keyboard_interface, keyboard, 6 ) );
            const char* seat[] = { "capabilities", "name" };
            ASSERT_TRUE( EventsAre( gucef_wl_seat_interface, seat, 2 ) );
            const char* output[] = { "geometry", "mode", "done", "scale", "name", "description" };
            ASSERT_TRUE( EventsAre( gucef_wl_output_interface, output, 6 ) );
            const char* surface[] = { "enter", "leave", "preferred_buffer_scale", "preferred_buffer_transform" };
            ASSERT_TRUE( EventsAre( gucef_wl_surface_interface, surface, 4 ) );
            const char* toplevel[] = { "configure", "close", "configure_bounds", "wm_capabilities" };
            ASSERT_TRUE( EventsAre( gucef_xdg_toplevel_interface, toplevel, 4 ) );
            const char* xdgSurface[] = { "configure" };
            ASSERT_TRUE( EventsAre( gucef_xdg_surface_interface, xdgSurface, 1 ) );
            const char* wmBase[] = { "ping" };
            ASSERT_TRUE( EventsAre( gucef_xdg_wm_base_interface, wmBase, 1 ) );
            const char* fractional[] = { "preferred_scale" };
            ASSERT_TRUE( EventsAre( gucef_wp_fractional_scale_v1_interface, fractional, 1 ) );
            const char* decoration[] = { "configure" };
            ASSERT_TRUE( EventsAre( gucef_zxdg_toplevel_decoration_v1_interface, decoration, 1 ) );
            const char* xdgOutput[] = { "logical_position", "logical_size", "done", "name", "description" };
            ASSERT_TRUE( EventsAre( gucef_zxdg_output_v1_interface, xdgOutput, 5 ) );
            const char* activationToken[] = { "done" };
            ASSERT_TRUE( EventsAre( gucef_xdg_activation_token_v1_interface, activationToken, 1 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A listener structure has a slot for every event of its interface, and the versions the code binds are covered" )
        try
        {
            struct SListenerRow
            {
                const char* name;
                const struct wl_interface* iface;
                size_t slots;               // sizeof the listener structure in functions
                int eventCountOfHeader;     // what the generated header says
                int versionOfHeader;
                UInt32 boundVersion;        // the newest version the code binds (the version of the object a listener is added to)
            };
            #define LISTENER_ROW( n, N, bound ) { #n, &gucef_##n##_interface, sizeof( struct gucef_##n##_listener ) / sizeof( void* ), GUCEF_WL_##N##_EVENT_COUNT, GUCEF_WL_##N##_VERSION, bound }
            const SListenerRow rows[] = {
                LISTENER_ROW( wl_registry, WL_REGISTRY, 1 ),
                LISTENER_ROW( wl_callback, WL_CALLBACK, 1 ),
                LISTENER_ROW( wl_shm, WL_SHM, 1 ),
                LISTENER_ROW( wl_buffer, WL_BUFFER, 1 ),
                LISTENER_ROW( wl_surface, WL_SURFACE, CWaylandConnection::MaxCompositorVersion ),
                LISTENER_ROW( wl_seat, WL_SEAT, CWaylandConnection::MaxSeatVersion ),
                LISTENER_ROW( wl_pointer, WL_POINTER, CWaylandConnection::MaxSeatVersion ),
                LISTENER_ROW( wl_keyboard, WL_KEYBOARD, CWaylandConnection::MaxSeatVersion ),
                LISTENER_ROW( wl_touch, WL_TOUCH, CWaylandConnection::MaxSeatVersion ),
                LISTENER_ROW( wl_output, WL_OUTPUT, CWaylandConnection::MaxOutputVersion ),
                LISTENER_ROW( xdg_wm_base, XDG_WM_BASE, CWaylandConnection::MaxWmBaseVersion ),
                LISTENER_ROW( xdg_surface, XDG_SURFACE, CWaylandConnection::MaxWmBaseVersion ),
                LISTENER_ROW( xdg_toplevel, XDG_TOPLEVEL, CWaylandConnection::MaxWmBaseVersion ),
                LISTENER_ROW( xdg_popup, XDG_POPUP, CWaylandConnection::MaxWmBaseVersion ),
                LISTENER_ROW( wp_fractional_scale_v1, WP_FRACTIONAL_SCALE_V1, 1 ),
                LISTENER_ROW( zxdg_toplevel_decoration_v1, ZXDG_TOPLEVEL_DECORATION_V1, 1 ),
                LISTENER_ROW( zxdg_output_v1, ZXDG_OUTPUT_V1, CWaylandConnection::MaxOutputManagerVersion ),
                LISTENER_ROW( xdg_activation_token_v1, XDG_ACTIVATION_TOKEN_V1, 1 )
            };
            #undef LISTENER_ROW
            for ( size_t i=0; i<sizeof( rows ) / sizeof( rows[ 0 ] ); ++i )
            {
                const SListenerRow& row = rows[ i ];
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Listener of " << row.name << ": " << (UInt32) row.slots << " slots, " << row.iface->event_count << " events" );
                // a listener shorter than the events makes libwayland call whatever follows the structure
                ASSERT_TRUE( (int) row.slots == row.iface->event_count );
                ASSERT_TRUE( row.eventCountOfHeader == row.iface->event_count );
                ASSERT_TRUE( row.versionOfHeader == row.iface->version );
                ASSERT_TRUE( (UInt32) row.iface->version >= row.boundVersion );

                // filled in full it covers every event of the version the code binds, with one function less it does not when the last event is of that version
                void* functions[ 16 ];
                for ( int e=0; e<16; ++e )
                    functions[ e ] = (void*) &functions;
                ASSERT_TRUE( 0 != gucef_wl_listener_is_complete( row.iface, (unsigned int) row.slots, functions, row.boundVersion ) );
                ASSERT_TRUE( 0 != gucef_wl_listener_is_complete( row.iface, (unsigned int) row.slots, functions, row.iface->version ) );
                for ( int hole=0; hole<row.iface->event_count; ++hole )
                {
                    void* holey[ 16 ];
                    memcpy( holey, functions, sizeof( holey ) );
                    holey[ hole ] = GUCEF_NULL;
                    const char* signature = row.iface->events[ hole ].signature;
                    UInt32 since = 0;
                    while ( *signature >= '0' && *signature <= '9' )
                        since = since * 10 + (UInt32) ( *signature++ - '0' );
                    const bool needed = since <= row.boundVersion;
                    ASSERT_TRUE( needed == ( 0 == gucef_wl_listener_is_complete( row.iface, (unsigned int) row.slots, holey, row.boundVersion ) ) );
                }
                ASSERT_TRUE( 0 == gucef_wl_listener_is_complete( row.iface, (unsigned int) row.slots - 1, functions, row.iface->version ) );
            }
            ASSERT_TRUE( 0 == gucef_wl_listener_is_complete( GUCEF_NULL, 1, GUCEF_NULL, 1 ) );

            // the seat is bound at 9 and the pointer of a seat at 9 has events up to relative direction (10): the one that was missing
            ASSERT_TRUE( 9 == CWaylandConnection::MaxSeatVersion );
            ASSERT_TRUE( GUCEF_WL_WL_POINTER_AXIS_RELATIVE_DIRECTION_EVENT_SINCE_VERSION <= CWaylandConnection::MaxSeatVersion );
            ASSERT_TRUE( GUCEF_WL_WL_POINTER_AXIS_RELATIVE_DIRECTION_EVENT_OPCODE == (int) ( offsetof( struct gucef_wl_pointer_listener, axis_relative_direction ) / sizeof( void* ) ) );
            ASSERT_TRUE( GUCEF_WL_WL_POINTER_AXIS_VALUE120_EVENT_OPCODE == (int) ( offsetof( struct gucef_wl_pointer_listener, axis_value120 ) / sizeof( void* ) ) );
            ASSERT_TRUE( 0 == strcmp( gucef_wl_pointer_interface.events[ GUCEF_WL_WL_POINTER_AXIS_RELATIVE_DIRECTION_EVENT_OPCODE ].signature, "9uu" ) );
            ASSERT_TRUE( 0 == strcmp( gucef_wl_pointer_interface.events[ GUCEF_WL_WL_POINTER_AXIS_VALUE120_EVENT_OPCODE ].signature, "8ui" ) );
            ASSERT_TRUE( 0 == strcmp( gucef_wl_pointer_interface.events[ GUCEF_WL_WL_POINTER_AXIS_EVENT_OPCODE ].signature, "uuf" ) );
            ASSERT_TRUE( 0 == strcmp( gucef_wl_pointer_interface.events[ GUCEF_WL_WL_POINTER_AXIS_DISCRETE_EVENT_OPCODE ].signature, "5ui" ) );
            ASSERT_TRUE( 0 == strcmp( gucef_wl_pointer_interface.events[ GUCEF_WL_WL_POINTER_AXIS_STOP_EVENT_OPCODE ].signature, "5uu" ) );
            ASSERT_TRUE( 0 == strcmp( gucef_wl_pointer_interface.events[ GUCEF_WL_WL_POINTER_AXIS_SOURCE_EVENT_OPCODE ].signature, "5u" ) );
            ASSERT_TRUE( 0 == strcmp( gucef_wl_pointer_interface.events[ GUCEF_WL_WL_POINTER_FRAME_EVENT_OPCODE ].signature, "5" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The opcodes of the requests are those of the protocols" )
        try
        {
            ASSERT_TRUE( 3 == GUCEF_WL_XDG_WM_BASE_PONG_OPCODE );
            ASSERT_TRUE( 4 == GUCEF_WL_XDG_SURFACE_ACK_CONFIGURE_OPCODE );
            ASSERT_TRUE( 2 == GUCEF_WL_XDG_TOPLEVEL_SET_TITLE_OPCODE );
            ASSERT_TRUE( 6 == GUCEF_WL_WL_SURFACE_COMMIT_OPCODE );
            ASSERT_TRUE( 3 == GUCEF_WL_WL_SURFACE_FRAME_OPCODE );
            ASSERT_TRUE( 0 == GUCEF_WL_WL_POINTER_SET_CURSOR_OPCODE );
            ASSERT_TRUE( 0 == GUCEF_WL_WL_SEAT_GET_POINTER_OPCODE );
            ASSERT_TRUE( 1 == GUCEF_WL_WL_SEAT_GET_KEYBOARD_OPCODE );
            ASSERT_TRUE( 1 == GUCEF_WL_XDG_SURFACE_GET_TOPLEVEL_OPCODE );
            ASSERT_TRUE( 1 == GUCEF_WL_XDG_ACTIVATION_V1_GET_ACTIVATION_TOKEN_OPCODE );
            ASSERT_TRUE( 0 == GUCEF_WL_XDG_ACTIVATION_TOKEN_V1_SET_SERIAL_OPCODE );
            ASSERT_TRUE( 3 == GUCEF_WL_XDG_ACTIVATION_TOKEN_V1_COMMIT_OPCODE );
            ASSERT_TRUE( 0 == GUCEF_WL_XDG_ACTIVATION_TOKEN_V1_DONE_EVENT_OPCODE );
            // the values of the enums the code compares with
            ASSERT_TRUE( 4 == GUCEF_WL_XDG_TOPLEVEL_STATE_ACTIVATED );
            ASSERT_TRUE( 2 == GUCEF_WL_XDG_TOPLEVEL_STATE_FULLSCREEN );
            ASSERT_TRUE( 1 == GUCEF_WL_WL_POINTER_BUTTON_STATE_PRESSED );
            ASSERT_TRUE( 0 == GUCEF_WL_WL_POINTER_AXIS_VERTICAL_SCROLL );
            ASSERT_TRUE( 1 == GUCEF_WL_WL_POINTER_AXIS_HORIZONTAL_SCROLL );
            ASSERT_TRUE( 1 == GUCEF_WL_WL_KEYBOARD_KEY_STATE_PRESSED );
            ASSERT_TRUE( 1 == GUCEF_WL_WL_SEAT_CAPABILITY_POINTER );
            ASSERT_TRUE( 2 == GUCEF_WL_WL_SEAT_CAPABILITY_KEYBOARD );
            ASSERT_TRUE( 2 == GUCEF_WL_ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Scale arithmetic rounds the way the fractional scale protocol says and never goes under 1" )
        try
        {
            // a scale of 1.25 is 150, 1.5 is 180, 2 is 240
            ASSERT_TRUE( 1600 == CWaylandScale::LogicalToBuffer( 1280, 150 ) );
            ASSERT_TRUE( 1251 == CWaylandScale::LogicalToBuffer( 1001, 150 ) );       // 1251.25
            ASSERT_TRUE( 1254 == CWaylandScale::LogicalToBuffer( 1003, 150 ) );       // 1253.75
            ASSERT_TRUE( 8 == CWaylandScale::LogicalToBuffer( 5, 180 ) );             // 7.5: halves go up
            ASSERT_TRUE( 1280 == CWaylandScale::LogicalToBuffer( 1280, 120 ) );
            ASSERT_TRUE( 1280 == CWaylandScale::LogicalToBuffer( 1280, 0 ) );         // no scale is 1
            ASSERT_TRUE( 2560 == CWaylandScale::LogicalToBuffer( 1280, 240 ) );
            ASSERT_TRUE( 1280 == CWaylandScale::BufferToLogical( 1600, 150 ) );
            ASSERT_TRUE( 1001 == CWaylandScale::BufferToLogical( 1251, 150 ) );       // 1000.8 rounded up
            ASSERT_TRUE( 3 == CWaylandScale::BufferToLogical( 5, 240 ) );
            ASSERT_TRUE( 120 == CWaylandScale::FromWholeScale( 1 ) );
            ASSERT_TRUE( 240 == CWaylandScale::FromWholeScale( 2 ) );
            ASSERT_TRUE( 120 == CWaylandScale::FromWholeScale( 0 ) );
            ASSERT_TRUE( 120 == CWaylandScale::FromWholeScale( -3 ) );
            ASSERT_TRUE( CWaylandScale::IsWholeScale( 240 ) );
            ASSERT_FALSE( CWaylandScale::IsWholeScale( 180 ) );
            ASSERT_TRUE( "1" == CWaylandScale::ToString( 120 ) );
            ASSERT_TRUE( "1.5" == CWaylandScale::ToString( 180 ) );
            ASSERT_TRUE( "1.25" == CWaylandScale::ToString( 150 ) );
            ASSERT_TRUE( "2" == CWaylandScale::ToString( 240 ) );
            ASSERT_TRUE( "1.083" == CWaylandScale::ToString( 130 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Pointer coordinates are scaled to buffer pixels, negative ones too" )
        try
        {
            ASSERT_TRUE( 100 == CWaylandScale::PointerToBuffer( gucef_wl_fixed_from_int( 100 ), 120 ) );
            ASSERT_TRUE( 200 == CWaylandScale::PointerToBuffer( gucef_wl_fixed_from_int( 100 ), 240 ) );
            ASSERT_TRUE( 150 == CWaylandScale::PointerToBuffer( gucef_wl_fixed_from_int( 100 ), 180 ) );
            // 10.5 units at 1.5 is 15.75 pixels
            ASSERT_TRUE( 16 == CWaylandScale::PointerToBuffer( 10 * 256 + 128, 180 ) );
            ASSERT_TRUE( 0 == CWaylandScale::PointerToBuffer( 0, 180 ) );
            // a pointer dragged to the left of the window: -10.25 units is -10 pixels at scale 1 and -15.375 at 1.5
            ASSERT_TRUE( -10 == CWaylandScale::PointerToBuffer( -( 10 * 256 + 64 ), 120 ) );
            ASSERT_TRUE( -15 == CWaylandScale::PointerToBuffer( -( 10 * 256 + 64 ), 180 ) );
            ASSERT_TRUE( 0 == CWaylandScale::PointerToBuffer( -128, 120 ) );       // -0.5 rounds up like every half
            ASSERT_TRUE( -1 == CWaylandScale::PointerToBuffer( -129, 120 ) );
            ASSERT_TRUE( 3839 == CWaylandScale::PointerToBuffer( gucef_wl_fixed_from_int( 3839 ), 120 ) );
            ASSERT_TRUE( 100 == CWaylandScale::PointerToBuffer( gucef_wl_fixed_from_int( 100 ), 0 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The scale of a surface comes from the fractional scale, else the preferred whole scale, else the monitors it is on" )
        try
        {
            CWaylandScaleTracker tracker;
            ASSERT_TRUE( 120 == tracker.GetScale120() );
            ASSERT_TRUE( CWaylandScaleTracker::SCALESOURCE_DEFAULT == tracker.GetSource() );

            int monitorA = 0, monitorB = 0;
            tracker.OnOutputEntered( &monitorA, 2 );
            ASSERT_TRUE( 240 == tracker.GetScale120() );
            ASSERT_TRUE( CWaylandScaleTracker::SCALESOURCE_OUTPUTS == tracker.GetSource() );
            tracker.OnOutputEntered( &monitorB, 3 );
            ASSERT_TRUE( 360 == tracker.GetScale120() );                    // the largest of the monitors the window is on
            tracker.OnOutputLeft( &monitorB );
            ASSERT_TRUE( 240 == tracker.GetScale120() );
            tracker.OnOutputScaleChanged( &monitorA, 1 );
            ASSERT_TRUE( 120 == tracker.GetScale120() );
            tracker.OnOutputLeft( &monitorA );
            ASSERT_TRUE( CWaylandScaleTracker::SCALESOURCE_DEFAULT == tracker.GetSource() );

            tracker.OnOutputEntered( &monitorA, 2 );
            tracker.OnPreferredBufferScale( 3 );
            ASSERT_TRUE( 360 == tracker.GetScale120() );                    // the compositor's own preference wins over the monitors
            ASSERT_TRUE( CWaylandScaleTracker::SCALESOURCE_PREFERRED == tracker.GetSource() );
            ASSERT_FALSE( tracker.UsesViewport() );
            ASSERT_TRUE( 3 == tracker.GetWholeBufferScale() );

            // a fractional scale only counts when a viewport can apply it
            tracker.OnFractionalScale( 150 );
            ASSERT_TRUE( 360 == tracker.GetScale120() );
            tracker.SetFractionalCapable( true );
            ASSERT_TRUE( 150 == tracker.GetScale120() );
            ASSERT_TRUE( CWaylandScaleTracker::SCALESOURCE_FRACTIONAL == tracker.GetSource() );
            ASSERT_TRUE( tracker.UsesViewport() );
            ASSERT_TRUE( 1 == tracker.GetWholeBufferScale() );              // the viewport does the scaling

            // capable but the compositor said nothing fractional (yet): the whole scales are used with the viewport
            CWaylandScaleTracker waiting;
            waiting.SetFractionalCapable( true );
            waiting.OnPreferredBufferScale( 2 );
            ASSERT_TRUE( 240 == waiting.GetScale120() );
            ASSERT_TRUE( waiting.UsesViewport() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The socket of the compositor is found as libwayland finds it" )
        try
        {
            ASSERT_TRUE( "/run/user/1000/wayland-1" == CWaylandAvailability::ResolveSocketPath( "wayland-1", "/run/user/1000" ) );
            ASSERT_TRUE( "/run/user/1000/wayland-1" == CWaylandAvailability::ResolveSocketPath( "wayland-1", "/run/user/1000/" ) );
            ASSERT_TRUE( "/run/user/1000/wayland-0" == CWaylandAvailability::ResolveSocketPath( GUCEF_NULL, "/run/user/1000" ) );
            ASSERT_TRUE( "/run/user/1000/wayland-0" == CWaylandAvailability::ResolveSocketPath( "", "/run/user/1000" ) );
            ASSERT_TRUE( "/tmp/my-socket" == CWaylandAvailability::ResolveSocketPath( "/tmp/my-socket", GUCEF_NULL ) );
            ASSERT_TRUE( CWaylandAvailability::ResolveSocketPath( "wayland-1", GUCEF_NULL ).IsNULLOrEmpty() );
            ASSERT_TRUE( CWaylandAvailability::ResolveSocketPath( "wayland-1", "" ).IsNULLOrEmpty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Wayland is available when the session is there, the compositor accepts the connection and offers what a window needs" )
        try
        {
            TWaylandGlobalList globals;
            SWaylandGlobal compositor;
            compositor.name = 1; compositor.interfaceName = "wl_compositor"; compositor.version = 6;
            SWaylandGlobal wmBase;
            wmBase.name = 2; wmBase.interfaceName = "xdg_wm_base"; wmBase.version = 5;
            SWaylandGlobal seat;
            seat.name = 3; seat.interfaceName = "wl_seat"; seat.version = 9;
            globals.push_back( compositor );
            globals.push_back( wmBase );

            SWaylandAvailability result = CWaylandAvailability::Decide( true, true, true, globals, false );
            ASSERT_TRUE( result.available );
            ASSERT_TRUE( result.reason.IsNULLOrEmpty() );

            // input needs a seat as well
            result = CWaylandAvailability::Decide( true, true, true, globals, true );
            ASSERT_FALSE( result.available );
            ASSERT_TRUE( result.reason.HasSubstr( "wl_seat" ) >= 0 );
            globals.push_back( seat );
            ASSERT_TRUE( CWaylandAvailability::Decide( true, true, true, globals, true ).available );

            // the first thing that fails is the reason
            result = CWaylandAvailability::Decide( false, true, true, globals, false );
            ASSERT_FALSE( result.available );
            ASSERT_TRUE( result.reason.HasSubstr( "no Wayland session" ) >= 0 );
            result = CWaylandAvailability::Decide( true, false, true, globals, false );
            ASSERT_FALSE( result.available );
            ASSERT_TRUE( result.reason.HasSubstr( "libwayland-client" ) >= 0 );
            result = CWaylandAvailability::Decide( true, true, false, globals, false );
            ASSERT_FALSE( result.available );

            // an old compositor interface and a missing xdg_wm_base
            TWaylandGlobalList old;
            compositor.version = 3;
            old.push_back( compositor );
            old.push_back( wmBase );
            CORE::CString missing;
            ASSERT_FALSE( CWaylandAvailability::HasRequiredGlobals( old, false, missing ) );
            ASSERT_TRUE( 0 == strncmp( missing.C_String(), "wl_compositor", 13 ) );
            TWaylandGlobalList noShell;
            compositor.version = 6;
            noShell.push_back( compositor );
            ASSERT_FALSE( CWaylandAvailability::HasRequiredGlobals( noShell, false, missing ) );
            ASSERT_TRUE( "xdg_wm_base" == missing );
            ASSERT_TRUE( CWaylandAvailability::HasGlobal( globals, "wl_seat", 9 ) );
            ASSERT_FALSE( CWaylandAvailability::HasGlobal( globals, "wl_seat", 10 ) );
            ASSERT_FALSE( CWaylandAvailability::HasGlobal( globals, "wp_viewporter" ) );
            ASSERT_TRUE( 9 == CWaylandAvailability::GetGlobalVersion( globals, "wl_seat" ) );
            ASSERT_TRUE( 0 == CWaylandAvailability::GetGlobalVersion( globals, "wp_viewporter" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Cursor shapes map to the shapes of the protocol and to the names of cursor themes" )
        try
        {
            ASSERT_TRUE( 1 == CWaylandCursor::ToProtocolShape( CWaylandCursor::CURSORSHAPE_DEFAULT ) );
            ASSERT_TRUE( 9 == CWaylandCursor::ToProtocolShape( CWaylandCursor::CURSORSHAPE_TEXT ) );
            ASSERT_TRUE( 4 == CWaylandCursor::ToProtocolShape( CWaylandCursor::CURSORSHAPE_POINTER ) );
            ASSERT_TRUE( 8 == CWaylandCursor::ToProtocolShape( CWaylandCursor::CURSORSHAPE_CROSSHAIR ) );
            ASSERT_TRUE( 6 == CWaylandCursor::ToProtocolShape( CWaylandCursor::CURSORSHAPE_WAIT ) );
            ASSERT_TRUE( 26 == CWaylandCursor::ToProtocolShape( CWaylandCursor::CURSORSHAPE_EW_RESIZE ) );
            ASSERT_TRUE( 27 == CWaylandCursor::ToProtocolShape( CWaylandCursor::CURSORSHAPE_NS_RESIZE ) );
            ASSERT_TRUE( 0 == strcmp( "text", CWaylandCursor::ToThemeName( CWaylandCursor::CURSORSHAPE_TEXT ) ) );
            ASSERT_TRUE( 0 == strcmp( "xterm", CWaylandCursor::ToLegacyThemeName( CWaylandCursor::CURSORSHAPE_TEXT ) ) );
            ASSERT_TRUE( 0 == strcmp( "left_ptr", CWaylandCursor::ToLegacyThemeName( CWaylandCursor::CURSORSHAPE_DEFAULT ) ) );
            for ( int shape=0; shape<CWaylandCursor::CURSORSHAPE_COUNT; ++shape )
            {
                ASSERT_TRUE( CWaylandCursor::ToProtocolShape( (CWaylandCursor::ECursorShape) shape ) > 0 );
                ASSERT_TRUE( GUCEF_NULL != CWaylandCursor::ToThemeName( (CWaylandCursor::ECursorShape) shape ) );
                ASSERT_TRUE( GUCEF_NULL != CWaylandCursor::ToLegacyThemeName( (CWaylandCursor::ECursorShape) shape ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Decorations are the compositor's, else libdecor's, else none, unless the user says otherwise" )
        try
        {
            // auto
            ASSERT_TRUE( DECORATION_SERVER == CDecorationPolicy::Choose( DECORATIONPREFERENCE_AUTO, true, true ) );
            ASSERT_TRUE( DECORATION_SERVER == CDecorationPolicy::Choose( DECORATIONPREFERENCE_AUTO, true, false ) );
            ASSERT_TRUE( DECORATION_LIBDECOR == CDecorationPolicy::Choose( DECORATIONPREFERENCE_AUTO, false, true ) );
            ASSERT_TRUE( DECORATION_NONE == CDecorationPolicy::Choose( DECORATIONPREFERENCE_AUTO, false, false ) );
            // the user
            ASSERT_TRUE( DECORATION_NONE == CDecorationPolicy::Choose( DECORATIONPREFERENCE_NONE, true, true ) );
            ASSERT_TRUE( DECORATION_LIBDECOR == CDecorationPolicy::Choose( DECORATIONPREFERENCE_LIBDECOR, true, true ) );
            ASSERT_TRUE( DECORATION_SERVER == CDecorationPolicy::Choose( DECORATIONPREFERENCE_LIBDECOR, true, false ) );       // not there: the next best
            ASSERT_TRUE( DECORATION_SERVER == CDecorationPolicy::Choose( DECORATIONPREFERENCE_SERVER, true, true ) );
            ASSERT_TRUE( DECORATION_LIBDECOR == CDecorationPolicy::Choose( DECORATIONPREFERENCE_SERVER, false, true ) );       // not offered: the next best
            ASSERT_TRUE( DECORATION_NONE == CDecorationPolicy::Choose( DECORATIONPREFERENCE_SERVER, false, false ) );

            ASSERT_TRUE( DECORATION_SERVER == CDecorationPolicy::Effective( DECORATION_SERVER, false ) );
            ASSERT_TRUE( DECORATION_NONE == CDecorationPolicy::Effective( DECORATION_SERVER, true ) );
            ASSERT_TRUE( DECORATION_LIBDECOR == CDecorationPolicy::Effective( DECORATION_LIBDECOR, true ) );
            ASSERT_TRUE( DECORATIONPREFERENCE_NONE == CDecorationPolicy::ParsePreference( "None" ) );
            ASSERT_TRUE( DECORATIONPREFERENCE_SERVER == CDecorationPolicy::ParsePreference( "server" ) );
            ASSERT_TRUE( DECORATIONPREFERENCE_LIBDECOR == CDecorationPolicy::ParsePreference( "LIBDECOR" ) );
            ASSERT_TRUE( DECORATIONPREFERENCE_AUTO == CDecorationPolicy::ParsePreference( "auto" ) );
            ASSERT_TRUE( DECORATIONPREFERENCE_AUTO == CDecorationPolicy::ParsePreference( "" ) );
            ASSERT_TRUE( 0 == strcmp( "libdecor", CDecorationPolicy::ToString( DECORATION_LIBDECOR ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: With vertical sync a frame is drawn when the compositor showed the one before" )
        try
        {
            CWaylandFramePacer pacer;
            pacer.SetVerticalSync( true );
            ASSERT_FALSE( pacer.ShouldDraw( 0 ) );                          // not configured: nothing may be drawn
            pacer.SetConfigured( true );
            ASSERT_TRUE( pacer.ShouldDraw( 0 ) );                           // the first frame
            ASSERT_TRUE( pacer.WantsFrameCallback() );
            pacer.OnFrameCallbackRequested( 0 );
            pacer.OnFrameDrawn( 0 );
            ASSERT_FALSE( pacer.WantsFrameCallback() );
            ASSERT_TRUE( pacer.IsCallbackPending() );
            ASSERT_FALSE( pacer.ShouldDraw( 5 ) );
            ASSERT_FALSE( pacer.ShouldDraw( 500 ) );
            pacer.OnFrameCallbackDone();
            ASSERT_TRUE( pacer.ShouldDraw( 501 ) );
            ASSERT_TRUE( 1 == pacer.GetFramesDrawn() );

            // a callback that never comes: a frame after the timeout, and then the wait starts over
            pacer.OnFrameCallbackRequested( 600 );
            pacer.OnFrameDrawn( 600 );
            ASSERT_FALSE( pacer.ShouldDraw( 1599 ) );
            ASSERT_TRUE( pacer.ShouldDraw( 1600 ) );
            pacer.OnFrameDrawn( 1600 );
            ASSERT_FALSE( pacer.ShouldDraw( 1601 ) );
            ASSERT_FALSE( pacer.ShouldDraw( 2599 ) );
            ASSERT_TRUE( pacer.ShouldDraw( 2600 ) );

            // something that must be shown is drawn at once
            pacer.RequestRedraw();
            ASSERT_TRUE( pacer.ShouldDraw( 1700 ) );
            pacer.OnFrameDrawn( 1700 );
            ASSERT_FALSE( pacer.ShouldDraw( 1701 ) );

            // the tick count wrapped around
            CWaylandFramePacer wrapped;
            wrapped.SetConfigured( true );
            wrapped.OnFrameDrawn( 0xFFFFFFF0u );
            wrapped.OnFrameCallbackRequested( 0xFFFFFFF0u );
            ASSERT_FALSE( wrapped.ShouldDraw( 100 ) );
            ASSERT_TRUE( wrapped.ShouldDraw( 0xFFFFFFF0u + 1000u ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Without vertical sync frames come at the pace of the refresh rate of the settings and ask for no callbacks" )
        try
        {
            CWaylandFramePacer pacer;
            pacer.SetVerticalSync( false );
            pacer.SetFrequency( 50 );
            pacer.SetConfigured( true );
            ASSERT_TRUE( pacer.ShouldDraw( 1000 ) );
            ASSERT_FALSE( pacer.WantsFrameCallback() );
            pacer.OnFrameDrawn( 1000 );
            ASSERT_FALSE( pacer.ShouldDraw( 1019 ) );
            ASSERT_TRUE( pacer.ShouldDraw( 1020 ) );
            pacer.OnFrameDrawn( 1020 );
            pacer.SetFrequency( 0 );
            ASSERT_FALSE( pacer.ShouldDraw( 1035 ) );
            ASSERT_TRUE( pacer.ShouldDraw( 1036 ) );        // no rate: 60 a second
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A configure ends a series of events and gives the size and the states to apply" )
        try
        {
            CXdgConfigureState state;
            state.SetRequestedSize( 1280, 720 );
            ASSERT_FALSE( state.IsConfigured() );
            ASSERT_TRUE( 1280 == state.GetWidth() );

            // the first configure of a compositor that leaves the size to us
            SConfigureResult result;
            state.OnToplevelConfigure( 0, 0, 0 );
            state.OnSurfaceConfigure( 7, result );
            ASSERT_TRUE( 7 == result.serial );
            ASSERT_TRUE( 1280 == result.width && 720 == result.height );
            ASSERT_TRUE( result.sizeChanged && result.firstConfigure );
            ASSERT_TRUE( state.IsConfigured() );

            // the user resizes the window
            state.OnToplevelConfigure( 1000, 600, TOPLEVELSTATE_RESIZING | TOPLEVELSTATE_ACTIVATED );
            state.OnSurfaceConfigure( 8, result );
            ASSERT_TRUE( 1000 == result.width && 600 == result.height );
            ASSERT_TRUE( result.sizeChanged && !result.firstConfigure );
            ASSERT_TRUE( 0 != ( result.changedStates & TOPLEVELSTATE_ACTIVATED ) );
            ASSERT_TRUE( state.IsActivated() && state.IsFloating() );

            // maximised: the size of the compositor; no change of size when it repeats
            state.OnToplevelConfigure( 1920, 1080, TOPLEVELSTATE_MAXIMIZED | TOPLEVELSTATE_ACTIVATED );
            state.OnSurfaceConfigure( 9, result );
            ASSERT_TRUE( 1920 == result.width && 1080 == result.height );
            ASSERT_FALSE( state.IsFloating() );
            state.OnToplevelConfigure( 1920, 1080, TOPLEVELSTATE_MAXIMIZED | TOPLEVELSTATE_ACTIVATED );
            state.OnSurfaceConfigure( 10, result );
            ASSERT_FALSE( result.sizeChanged );
            ASSERT_TRUE( 0 == result.changedStates );

            // un-maximised with the size left to us: back to the size of the floating window
            state.OnToplevelConfigure( 0, 0, TOPLEVELSTATE_ACTIVATED );
            state.OnSurfaceConfigure( 11, result );
            ASSERT_TRUE( 1000 == result.width && 600 == result.height );
            ASSERT_TRUE( state.IsFloating() );

            // several toplevel configures before the surface one: the last wins
            state.OnToplevelConfigure( 800, 500, TOPLEVELSTATE_ACTIVATED );
            state.OnToplevelConfigure( 810, 510, TOPLEVELSTATE_ACTIVATED | TOPLEVELSTATE_RESIZING );
            state.OnSurfaceConfigure( 12, result );
            ASSERT_TRUE( 810 == result.width && 510 == result.height );
            ASSERT_TRUE( 0 != ( result.states & TOPLEVELSTATE_RESIZING ) );

            // full screen with an unspecified size keeps the size the window has
            state.OnToplevelConfigure( 0, 0, TOPLEVELSTATE_FULLSCREEN | TOPLEVELSTATE_ACTIVATED );
            state.OnSurfaceConfigure( 13, result );
            ASSERT_TRUE( 810 == result.width && 510 == result.height );
            ASSERT_TRUE( 0 != ( result.states & TOPLEVELSTATE_FULLSCREEN ) );

            // an xdg_surface configure with no toplevel configure before it still has to be acknowledged, with what is
            ASSERT_FALSE( state.OnSurfaceConfigure( 14, result ) );
            ASSERT_TRUE( 14 == result.serial );
            ASSERT_TRUE( 810 == result.width );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The states of a toplevel come from the numbers of the protocol and from libdecor" )
        try
        {
            const UInt32 protocol[] = { GUCEF_WL_XDG_TOPLEVEL_STATE_MAXIMIZED, GUCEF_WL_XDG_TOPLEVEL_STATE_ACTIVATED, GUCEF_WL_XDG_TOPLEVEL_STATE_TILED_LEFT, 77 };
            UInt32 bits = CXdgConfigureState::StatesFromProtocol( protocol, 4 );
            ASSERT_TRUE( bits == ( TOPLEVELSTATE_MAXIMIZED | TOPLEVELSTATE_ACTIVATED | TOPLEVELSTATE_TILED_LEFT ) );
            ASSERT_TRUE( 0 == CXdgConfigureState::StatesFromProtocol( GUCEF_NULL, 0 ) );
            // LIBDECOR_WINDOW_STATE_ACTIVE 1, MAXIMIZED 2, FULLSCREEN 4, TILED_LEFT 8, SUSPENDED 128
            bits = CXdgConfigureState::StatesFromLibdecor( 1 | 4 | 128 );
            ASSERT_TRUE( bits == ( TOPLEVELSTATE_ACTIVATED | TOPLEVELSTATE_FULLSCREEN | TOPLEVELSTATE_SUSPENDED ) );

            // the window manager can say what it cannot do
            CXdgConfigureState state;
            ASSERT_TRUE( state.CanFullscreen() && state.CanMaximize() );
            const UInt32 caps[] = { 2 };    // maximize
            state.OnCapabilities( caps, 1 );
            ASSERT_TRUE( state.CanMaximize() );
            ASSERT_FALSE( state.CanFullscreen() );
            state.OnToplevelBounds( 3840, 1040 );
            ASSERT_TRUE( 3840 == state.GetBoundsWidth() && 1040 == state.GetBoundsHeight() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The application sets the size of a floating window, not of one the compositor decides on" )
        try
        {
            CXdgConfigureState state;
            state.SetRequestedSize( 1280, 720 );
            SConfigureResult result;
            state.OnToplevelConfigure( 0, 0, TOPLEVELSTATE_ACTIVATED );
            state.OnSurfaceConfigure( 1, result );
            ASSERT_TRUE( state.ApplyClientSize( 1024, 768 ) );
            ASSERT_TRUE( 1024 == state.GetWidth() && 768 == state.GetHeight() );
            ASSERT_FALSE( state.ApplyClientSize( 1024, 768 ) );
            ASSERT_FALSE( state.ApplyClientSize( 0, 768 ) );

            state.OnToplevelConfigure( 3840, 1080, TOPLEVELSTATE_MAXIMIZED );
            state.OnSurfaceConfigure( 2, result );
            ASSERT_FALSE( state.ApplyClientSize( 640, 480 ) );
            ASSERT_TRUE( 3840 == state.GetWidth() );
            // leaving the maximised state brings back the size the application set last while floating
            state.OnToplevelConfigure( 0, 0, 0 );
            state.OnSurfaceConfigure( 3, result );
            ASSERT_TRUE( 1024 == result.width && 768 == result.height );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
