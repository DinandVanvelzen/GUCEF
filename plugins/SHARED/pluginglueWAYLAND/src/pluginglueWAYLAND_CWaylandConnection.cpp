/*
 *  pluginglueWAYLAND: Library that owns the connection to the Wayland compositor for the plugins that share it
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

#include <errno.h>
#include <poll.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDRUNTIME_H
#include "pluginglueWAYLAND_CWaylandRuntime.h"
#define GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDRUNTIME_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDRUNTIME_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_CEVENTGUARD_H
#include "pluginglueWAYLAND_CEventGuard.h"
#define GUCEF_PLUGINGLUE_WAYLAND_CEVENTGUARD_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_CEVENTGUARD_H ? */

#include "pluginglueWAYLAND_CWaylandConnection.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PLUGINGLUE {
namespace WAYLAND {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The shared connection of the process and who uses it; made once and kept, the plugins that use it can unload after the statics of this library are gone */
struct SSharedConnection
{
    MT::CMutex lock;
    CWaylandConnection* connection;
    UInt32 users;
    GUCEF::vector< CWaylandConnection* > retired;

    SSharedConnection( void ) : lock(), connection( GUCEF_NULL ), users( 0 ), retired() {}
};

SSharedConnection&
Shared( void )
{
    static SSharedConnection* shared = new SSharedConnection();
    return *shared;
}

UInt32
MinVersion( UInt32 offered, UInt32 supported )
{
    return offered < supported ? offered : supported;
}

}

const CORE::CEvent CWaylandConnection::ConnectionLostEvent = "GUCEF::PLUGINGLUE::WAYLAND::CWaylandConnection::ConnectionLostEvent";
const CORE::CEvent CWaylandConnection::OutputsChangedEvent = "GUCEF::PLUGINGLUE::WAYLAND::CWaylandConnection::OutputsChangedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      CALLBACKS OF THE COMPOSITOR                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The functions the compositor's events call. They only hand over to the connection: 'data' is the connection (or the
 *  output for the events of a monitor).
 */
struct SWaylandConnectionCallbacks
{
    static CWaylandConnection* C( void* data ) { return static_cast< CWaylandConnection* >( data ); }

    // wl_registry

    static void RegistryGlobal( void* data, struct wl_registry*, uint32_t name, const char* interfaceName, uint32_t version )
    {
        GUCEF_WL_GUARD_BEGIN
        C( data )->OnGlobal( name, interfaceName, version );
        GUCEF_WL_GUARD_END
    }

    static void RegistryGlobalRemove( void* data, struct wl_registry*, uint32_t name )
    {
        GUCEF_WL_GUARD_BEGIN
        C( data )->OnGlobalRemoved( name );
        GUCEF_WL_GUARD_END
    }

    // xdg_wm_base

    static void WmBasePing( void* data, struct xdg_wm_base* wmBase, uint32_t serial )
    {
        GUCEF_WL_GUARD_BEGIN
        gucef_xdg_wm_base_pong( &C( data )->m_api, wmBase, serial );
        GUCEF_WL_GUARD_END
    }

    // wl_seat

    static void SeatCapabilities( void* data, struct wl_seat*, uint32_t capabilities )
    {
        GUCEF_WL_GUARD_BEGIN
        C( data )->OnSeatCapabilitiesEvent( capabilities );
        GUCEF_WL_GUARD_END
    }

    static void SeatName( void*, struct wl_seat*, const char* ) {}

    // wl_pointer

    static void PointerEnter( void* data, struct wl_pointer*, uint32_t serial, struct wl_surface* surface, wl_fixed_t x, wl_fixed_t y )
    {
        GUCEF_WL_GUARD_BEGIN
        CWaylandConnection* c = C( data );
        c->m_pointerEnterSerial = serial;
        c->m_lastInputSerial = serial;
        c->m_pointerFocus = surface;
        c->m_pointerX = x;
        c->m_pointerY = y;
        SWaylandSurfaceInfo* info = c->FindSurfaceByProxy( surface );
        if ( GUCEF_NULL != info )
            c->m_cursor.OnPointerEnter( serial, info->scale120 );
        CWaylandConnection::CDispatchScope dispatchScope_( *c );
        for ( UInt32 i=0; i<c->m_listeners.size(); ++i )
            if ( GUCEF_NULL != c->m_listeners[ i ] ) c->m_listeners[ i ]->OnPointerEnter( serial, surface, x, y );
        GUCEF_WL_GUARD_END
    }

    static void PointerLeave( void* data, struct wl_pointer*, uint32_t serial, struct wl_surface* surface )
    {
        GUCEF_WL_GUARD_BEGIN
        CWaylandConnection* c = C( data );
        if ( c->m_pointerFocus == surface )
            c->m_pointerFocus = GUCEF_NULL;
        c->m_cursor.OnPointerLeave();
        CWaylandConnection::CDispatchScope dispatchScope_( *c );
        for ( UInt32 i=0; i<c->m_listeners.size(); ++i )
            if ( GUCEF_NULL != c->m_listeners[ i ] ) c->m_listeners[ i ]->OnPointerLeave( serial, surface );
        GUCEF_WL_GUARD_END
    }

    static void PointerMotion( void* data, struct wl_pointer*, uint32_t time, wl_fixed_t x, wl_fixed_t y )
    {
        GUCEF_WL_GUARD_BEGIN
        CWaylandConnection* c = C( data );
        c->m_pointerX = x;
        c->m_pointerY = y;
        CWaylandConnection::CDispatchScope dispatchScope_( *c );
        for ( UInt32 i=0; i<c->m_listeners.size(); ++i )
            if ( GUCEF_NULL != c->m_listeners[ i ] ) c->m_listeners[ i ]->OnPointerMotion( time, x, y );
        GUCEF_WL_GUARD_END
    }

    static void PointerButton( void* data, struct wl_pointer*, uint32_t serial, uint32_t time, uint32_t button, uint32_t state )
    {
        GUCEF_WL_GUARD_BEGIN
        CWaylandConnection* c = C( data );
        c->m_lastInputSerial = serial;
        c->m_lastActivationSerial = serial;
        CWaylandConnection::CDispatchScope dispatchScope_( *c );
        for ( UInt32 i=0; i<c->m_listeners.size(); ++i )
            if ( GUCEF_NULL != c->m_listeners[ i ] ) c->m_listeners[ i ]->OnPointerButton( serial, time, button, state );
        GUCEF_WL_GUARD_END
    }

    static void PointerAxis( void* data, struct wl_pointer*, uint32_t time, uint32_t axis, wl_fixed_t value )
    {
        GUCEF_WL_GUARD_BEGIN
        CWaylandConnection* c = C( data );
        CWaylandConnection::CDispatchScope dispatchScope_( *c );
        for ( UInt32 i=0; i<c->m_listeners.size(); ++i )
            if ( GUCEF_NULL != c->m_listeners[ i ] ) c->m_listeners[ i ]->OnPointerAxis( time, axis, value );
        GUCEF_WL_GUARD_END
    }

    static void PointerFrame( void* data, struct wl_pointer* )
    {
        GUCEF_WL_GUARD_BEGIN
        CWaylandConnection* c = C( data );
        CWaylandConnection::CDispatchScope dispatchScope_( *c );
        for ( UInt32 i=0; i<c->m_listeners.size(); ++i )
            if ( GUCEF_NULL != c->m_listeners[ i ] ) c->m_listeners[ i ]->OnPointerFrame();
        GUCEF_WL_GUARD_END
    }

    static void PointerAxisSource( void* data, struct wl_pointer*, uint32_t source )
    {
        GUCEF_WL_GUARD_BEGIN
        CWaylandConnection* c = C( data );
        CWaylandConnection::CDispatchScope dispatchScope_( *c );
        for ( UInt32 i=0; i<c->m_listeners.size(); ++i )
            if ( GUCEF_NULL != c->m_listeners[ i ] ) c->m_listeners[ i ]->OnPointerAxisSource( source );
        GUCEF_WL_GUARD_END
    }

    static void PointerAxisStop( void* data, struct wl_pointer*, uint32_t time, uint32_t axis )
    {
        GUCEF_WL_GUARD_BEGIN
        CWaylandConnection* c = C( data );
        CWaylandConnection::CDispatchScope dispatchScope_( *c );
        for ( UInt32 i=0; i<c->m_listeners.size(); ++i )
            if ( GUCEF_NULL != c->m_listeners[ i ] ) c->m_listeners[ i ]->OnPointerAxisStop( time, axis );
        GUCEF_WL_GUARD_END
    }

    static void PointerAxisDiscrete( void* data, struct wl_pointer*, uint32_t axis, int32_t discrete )
    {
        GUCEF_WL_GUARD_BEGIN
        CWaylandConnection* c = C( data );
        CWaylandConnection::CDispatchScope dispatchScope_( *c );
        for ( UInt32 i=0; i<c->m_listeners.size(); ++i )
            if ( GUCEF_NULL != c->m_listeners[ i ] ) c->m_listeners[ i ]->OnPointerAxisDiscrete( axis, discrete );
        GUCEF_WL_GUARD_END
    }

    static void PointerAxisValue120( void* data, struct wl_pointer*, uint32_t axis, int32_t value120 )
    {
        GUCEF_WL_GUARD_BEGIN
        CWaylandConnection* c = C( data );
        CWaylandConnection::CDispatchScope dispatchScope_( *c );
        for ( UInt32 i=0; i<c->m_listeners.size(); ++i )
            if ( GUCEF_NULL != c->m_listeners[ i ] ) c->m_listeners[ i ]->OnPointerAxisValue120( axis, value120 );
        GUCEF_WL_GUARD_END
    }

    static void PointerAxisRelativeDirection( void* data, struct wl_pointer*, uint32_t axis, uint32_t direction )
    {
        CWaylandConnection* c = C( data );
        GUCEF_WL_GUARD_BEGIN
        CWaylandConnection::CDispatchScope dispatchScope_( *c );
        for ( UInt32 i=0; i<c->m_listeners.size(); ++i )
            if ( GUCEF_NULL != c->m_listeners[ i ] ) c->m_listeners[ i ]->OnPointerAxisRelativeDirection( axis, direction );
        GUCEF_WL_GUARD_END
    }

    // the pointer warps only for version 11 of the seat, which is not bound; the slot is filled so the listener has a function for every event
    static void PointerWarp( void*, struct wl_pointer*, wl_fixed_t, wl_fixed_t ) {}

    // wl_keyboard

    static void KeyboardKeymap( void* data, struct wl_keyboard*, uint32_t format, int32_t fd, uint32_t size )
    {
        GUCEF_WL_GUARD_BEGIN
        CWaylandConnection* c = C( data );
        c->m_keymap.clear();
        c->m_keymapFormat = format;
        if ( GUCEF_WL_WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1 == format && size > 0 )
        {
            void* mapped = ::mmap( GUCEF_NULL, size, PROT_READ, MAP_PRIVATE, fd, 0 );
            if ( MAP_FAILED != mapped )
            {
                const char* bytes = static_cast< const char* >( mapped );
                c->m_keymap.assign( bytes, bytes + size );
                ::munmap( mapped, size );
            }
            else
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWaylandConnection: The keymap of the keyboard could not be read" );
        }
        ::close( fd );

        CWaylandConnection::CDispatchScope dispatchScope_( *c );
        for ( UInt32 i=0; i<c->m_listeners.size(); ++i )
            if ( GUCEF_NULL != c->m_listeners[ i ] )
                c->m_listeners[ i ]->OnKeyboardKeymap( format, c->m_keymap.empty() ? GUCEF_NULL : &c->m_keymap[ 0 ], (UInt32) c->m_keymap.size() );
        GUCEF_WL_GUARD_END
    }

    static void KeyboardEnter( void* data, struct wl_keyboard*, uint32_t serial, struct wl_surface* surface, struct wl_array* keys )
    {
        GUCEF_WL_GUARD_BEGIN
        CWaylandConnection* c = C( data );
        c->m_lastInputSerial = serial;
        c->m_keyboardFocus = surface;
        const UInt32* pressed = GUCEF_NULL != keys ? static_cast< const UInt32* >( keys->data ) : GUCEF_NULL;
        const UInt32 count = GUCEF_NULL != keys ? (UInt32) ( keys->size / sizeof( UInt32 ) ) : 0;
        CWaylandConnection::CDispatchScope dispatchScope_( *c );
        for ( UInt32 i=0; i<c->m_listeners.size(); ++i )
            if ( GUCEF_NULL != c->m_listeners[ i ] ) c->m_listeners[ i ]->OnKeyboardEnter( serial, surface, pressed, count );
        GUCEF_WL_GUARD_END
    }

    static void KeyboardLeave( void* data, struct wl_keyboard*, uint32_t serial, struct wl_surface* surface )
    {
        GUCEF_WL_GUARD_BEGIN
        CWaylandConnection* c = C( data );
        if ( c->m_keyboardFocus == surface )
            c->m_keyboardFocus = GUCEF_NULL;
        CWaylandConnection::CDispatchScope dispatchScope_( *c );
        for ( UInt32 i=0; i<c->m_listeners.size(); ++i )
            if ( GUCEF_NULL != c->m_listeners[ i ] ) c->m_listeners[ i ]->OnKeyboardLeave( serial, surface );
        GUCEF_WL_GUARD_END
    }

    static void KeyboardKey( void* data, struct wl_keyboard*, uint32_t serial, uint32_t time, uint32_t key, uint32_t state )
    {
        GUCEF_WL_GUARD_BEGIN
        CWaylandConnection* c = C( data );
        c->m_lastInputSerial = serial;
        c->m_lastActivationSerial = serial;
        CWaylandConnection::CDispatchScope dispatchScope_( *c );
        for ( UInt32 i=0; i<c->m_listeners.size(); ++i )
            if ( GUCEF_NULL != c->m_listeners[ i ] ) c->m_listeners[ i ]->OnKeyboardKey( serial, time, key, state );
        GUCEF_WL_GUARD_END
    }

    static void KeyboardModifiers( void* data, struct wl_keyboard*, uint32_t serial, uint32_t depressed, uint32_t latched, uint32_t locked, uint32_t group )
    {
        GUCEF_WL_GUARD_BEGIN
        CWaylandConnection* c = C( data );
        CWaylandConnection::CDispatchScope dispatchScope_( *c );
        for ( UInt32 i=0; i<c->m_listeners.size(); ++i )
            if ( GUCEF_NULL != c->m_listeners[ i ] ) c->m_listeners[ i ]->OnKeyboardModifiers( serial, depressed, latched, locked, group );
        GUCEF_WL_GUARD_END
    }

    static void KeyboardRepeatInfo( void* data, struct wl_keyboard*, int32_t rate, int32_t delay )
    {
        GUCEF_WL_GUARD_BEGIN
        CWaylandConnection* c = C( data );
        c->m_hasRepeatInfo = true;
        c->m_repeatRate = rate;
        c->m_repeatDelay = delay;
        CWaylandConnection::CDispatchScope dispatchScope_( *c );
        for ( UInt32 i=0; i<c->m_listeners.size(); ++i )
            if ( GUCEF_NULL != c->m_listeners[ i ] ) c->m_listeners[ i ]->OnKeyboardRepeatInfo( rate, delay );
        GUCEF_WL_GUARD_END
    }

    // wl_output

    static SWaylandOutput* O( void* data ) { return static_cast< SWaylandOutput* >( data ); }

    static void OutputGeometry( void* data, struct wl_output*, int32_t x, int32_t y, int32_t physicalWidth, int32_t physicalHeight,
                                int32_t, const char* make, const char* model, int32_t transform )
    {
        GUCEF_WL_GUARD_BEGIN
        SWaylandOutput* o = O( data );
        o->x = x;
        o->y = y;
        o->physicalWidthInMm = physicalWidth;
        o->physicalHeightInMm = physicalHeight;
        o->make = GUCEF_NULL != make ? make : "";
        o->model = GUCEF_NULL != model ? model : "";
        o->transform = transform;
        GUCEF_WL_GUARD_END
    }

    static void OutputMode( void* data, struct wl_output*, uint32_t flags, int32_t width, int32_t height, int32_t refresh )
    {
        GUCEF_WL_GUARD_BEGIN
        SWaylandOutput* o = O( data );
        SWaylandOutputMode* mode = GUCEF_NULL;
        for ( UInt32 i=0; i<o->modes.size(); ++i )
            if ( o->modes[ i ].width == width && o->modes[ i ].height == height && o->modes[ i ].refreshInMilliHertz == refresh )
                mode = &o->modes[ i ];
        if ( GUCEF_NULL == mode )
        {
            o->modes.push_back( SWaylandOutputMode() );
            mode = &o->modes.back();
            mode->width = width;
            mode->height = height;
            mode->refreshInMilliHertz = refresh;
        }
        // one mode is the current one
        if ( 0 != ( flags & GUCEF_WL_WL_OUTPUT_MODE_CURRENT ) )
            for ( UInt32 i=0; i<o->modes.size(); ++i )
                o->modes[ i ].flags &= ~( (UInt32) GUCEF_WL_WL_OUTPUT_MODE_CURRENT );
        mode->flags = flags;
        GUCEF_WL_GUARD_END
    }

    static void OutputDone( void* data, struct wl_output* )
    {
        GUCEF_WL_GUARD_BEGIN
        SWaylandOutput* o = O( data );
        o->done = true;
        o->owner->NotifyObservers( CWaylandConnection::OutputsChangedEvent );
        GUCEF_WL_GUARD_END
    }

    static void OutputScale( void* data, struct wl_output*, int32_t factor )
    {
        GUCEF_WL_GUARD_BEGIN
        O( data )->scale = factor;
        GUCEF_WL_GUARD_END
    }

    static void OutputName( void* data, struct wl_output*, const char* name )
    {
        GUCEF_WL_GUARD_BEGIN
        O( data )->name = GUCEF_NULL != name ? name : "";
        GUCEF_WL_GUARD_END
    }

    static void OutputDescription( void* data, struct wl_output*, const char* description )
    {
        GUCEF_WL_GUARD_BEGIN
        O( data )->description = GUCEF_NULL != description ? description : "";
        GUCEF_WL_GUARD_END
    }

    // zxdg_output_v1

    static void XdgOutputPosition( void* data, struct zxdg_output_v1*, int32_t x, int32_t y )
    {
        GUCEF_WL_GUARD_BEGIN
        SWaylandOutput* o = O( data );
        o->logicalX = x;
        o->logicalY = y;
        GUCEF_WL_GUARD_END
    }

    static void XdgOutputSize( void* data, struct zxdg_output_v1*, int32_t width, int32_t height )
    {
        GUCEF_WL_GUARD_BEGIN
        SWaylandOutput* o = O( data );
        o->logicalWidth = width;
        o->logicalHeight = height;
        o->hasLogicalSize = true;
        GUCEF_WL_GUARD_END
    }

    static void XdgOutputDone( void*, struct zxdg_output_v1* ) {}
    static void XdgOutputName( void*, struct zxdg_output_v1*, const char* ) {}
    static void XdgOutputDescription( void*, struct zxdg_output_v1*, const char* ) {}
};

namespace {

const struct gucef_wl_registry_listener g_registryListener = { SWaylandConnectionCallbacks::RegistryGlobal, SWaylandConnectionCallbacks::RegistryGlobalRemove };
const struct gucef_xdg_wm_base_listener g_wmBaseListener = { SWaylandConnectionCallbacks::WmBasePing };
const struct gucef_wl_seat_listener g_seatListener = { SWaylandConnectionCallbacks::SeatCapabilities, SWaylandConnectionCallbacks::SeatName };

const struct gucef_wl_pointer_listener g_pointerListener = {
    SWaylandConnectionCallbacks::PointerEnter, SWaylandConnectionCallbacks::PointerLeave, SWaylandConnectionCallbacks::PointerMotion,
    SWaylandConnectionCallbacks::PointerButton, SWaylandConnectionCallbacks::PointerAxis, SWaylandConnectionCallbacks::PointerFrame,
    SWaylandConnectionCallbacks::PointerAxisSource, SWaylandConnectionCallbacks::PointerAxisStop, SWaylandConnectionCallbacks::PointerAxisDiscrete,
    SWaylandConnectionCallbacks::PointerAxisValue120, SWaylandConnectionCallbacks::PointerAxisRelativeDirection, SWaylandConnectionCallbacks::PointerWarp };

const struct gucef_wl_keyboard_listener g_keyboardListener = {
    SWaylandConnectionCallbacks::KeyboardKeymap, SWaylandConnectionCallbacks::KeyboardEnter, SWaylandConnectionCallbacks::KeyboardLeave,
    SWaylandConnectionCallbacks::KeyboardKey, SWaylandConnectionCallbacks::KeyboardModifiers, SWaylandConnectionCallbacks::KeyboardRepeatInfo };

const struct gucef_wl_output_listener g_outputListener = {
    SWaylandConnectionCallbacks::OutputGeometry, SWaylandConnectionCallbacks::OutputMode, SWaylandConnectionCallbacks::OutputDone,
    SWaylandConnectionCallbacks::OutputScale, SWaylandConnectionCallbacks::OutputName, SWaylandConnectionCallbacks::OutputDescription };

const struct gucef_zxdg_output_v1_listener g_xdgOutputListener = {
    SWaylandConnectionCallbacks::XdgOutputPosition, SWaylandConnectionCallbacks::XdgOutputSize, SWaylandConnectionCallbacks::XdgOutputDone,
    SWaylandConnectionCallbacks::XdgOutputName, SWaylandConnectionCallbacks::XdgOutputDescription };

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const SWaylandOutputMode*
SWaylandOutput::GetCurrentMode( void ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<modes.size(); ++i )
        if ( 0 != ( modes[ i ].flags & GUCEF_WL_WL_OUTPUT_MODE_CURRENT ) )
            return &modes[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CWaylandConnection::RegisterEvents( void )
{GUCEF_TRACE;

    ConnectionLostEvent.Initialize();
    OutputsChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CWaylandConnection::CWaylandConnection( const SWaylandApi& api, const char* displayName )
    : CORE::CObservingNotifier()
    , m_api( api )
    , m_displayName( GUCEF_NULL != displayName ? displayName : "" )
    , m_display( GUCEF_NULL )
    , m_registry( GUCEF_NULL )
    , m_globals()
    , m_pumpOnPulse( true )
    , m_pumping( false )
    , m_healthy( true )
    , m_pulsing( false )
    , m_compositor( GUCEF_NULL )
    , m_compositorVersion( 0 )
    , m_wmBase( GUCEF_NULL )
    , m_shm( GUCEF_NULL )
    , m_viewporter( GUCEF_NULL )
    , m_fractionalScaleManager( GUCEF_NULL )
    , m_decorationManager( GUCEF_NULL )
    , m_cursorShapeManager( GUCEF_NULL )
    , m_outputManager( GUCEF_NULL )
    , m_activationManager( GUCEF_NULL )
    , m_outputs()
    , m_seat( GUCEF_NULL )
    , m_seatVersion( 0 )
    , m_seatName( 0 )
    , m_seatCapabilities( 0 )
    , m_pointer( GUCEF_NULL )
    , m_keyboard( GUCEF_NULL )
    , m_lastInputSerial( 0 )
    , m_lastActivationSerial( 0 )
    , m_pointerEnterSerial( 0 )
    , m_pointerFocus( GUCEF_NULL )
    , m_keyboardFocus( GUCEF_NULL )
    , m_pointerX( 0 )
    , m_pointerY( 0 )
    , m_keymap()
    , m_keymapFormat( 0 )
    , m_hasRepeatInfo( false )
    , m_repeatRate( 0 )
    , m_repeatDelay( 0 )
    , m_listeners()
    , m_dispatchDepth( 0 )
    , m_compactListeners( false )
    , m_cursor( *this )
    , m_surfaces()
    , m_nextSurfaceId( 1 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWaylandConnection::~CWaylandConnection()
{GUCEF_TRACE;

    Disconnect();
    for ( TSurfaceMap::iterator i=m_surfaces.begin(); i!=m_surfaces.end(); ++i )
        delete (*i).second;
    m_surfaces.clear();
}

/*-------------------------------------------------------------------------*/

CWaylandConnection*
CWaylandConnection::Acquire( CORE::CString* problem )
{GUCEF_TRACE;

    SSharedConnection& shared = Shared();
    MT::CScopeMutex lock( shared.lock );

    // connections that were released while they dispatched go now that nobody can be in them
    for ( UInt32 i=0; i<shared.retired.size(); )
    {
        if ( !shared.retired[ i ]->m_pumping )
        {
            delete shared.retired[ i ];
            shared.retired.erase( shared.retired.begin() + i );
        }
        else
            ++i;
    }

    if ( GUCEF_NULL != shared.connection )
    {
        ++shared.users;
        return shared.connection;
    }

    const SWaylandApi* api = CWaylandRuntime::GetClientApi();
    if ( GUCEF_NULL == api )
    {
        if ( GUCEF_NULL != problem )
            *problem = "libwayland-client.so.0 (version 1.19 or newer) could not be loaded";
        return GUCEF_NULL;
    }
    if ( CWaylandAvailability::FindSessionSocket().IsNULLOrEmpty() )
    {
        if ( GUCEF_NULL != problem )
            *problem = "there is no Wayland session: WAYLAND_DISPLAY does not name a socket of the compositor";
        return GUCEF_NULL;
    }

    CWaylandConnection* connection = new CWaylandConnection( *api );
    if ( !connection->Connect( problem ) )
    {
        delete connection;
        return GUCEF_NULL;
    }
    shared.connection = connection;
    shared.users = 1;
    return shared.connection;
}

/*-------------------------------------------------------------------------*/

void
CWaylandConnection::Release( CWaylandConnection* connection )
{GUCEF_TRACE;

    SSharedConnection& shared = Shared();
    MT::CScopeMutex lock( shared.lock );
    if ( GUCEF_NULL == connection || connection != shared.connection )
        return;
    if ( shared.users > 0 )
        --shared.users;
    if ( 0 != shared.users )
        return;

    shared.connection = GUCEF_NULL;
    if ( connection->m_pumping )
    {
        // released from within an event of its own: it goes the next time it is safe
        connection->SetPumpOnPulse( false );
        shared.retired.push_back( connection );
    }
    else
        delete connection;
}

/*-------------------------------------------------------------------------*/

void
CWaylandConnection::Deinstance( void )
{GUCEF_TRACE;

    SSharedConnection& shared = Shared();
    MT::CScopeMutex lock( shared.lock );
    delete shared.connection;
    shared.connection = GUCEF_NULL;
    shared.users = 0;
    for ( UInt32 i=0; i<shared.retired.size(); ++i )
        delete shared.retired[ i ];
    shared.retired.clear();
}

/*-------------------------------------------------------------------------*/

void
CWaylandConnection::SetPumpOnPulse( bool pumpOnPulse )
{GUCEF_TRACE;

    m_pumpOnPulse = pumpOnPulse;
    if ( !pumpOnPulse && m_pulsing )
    {
        CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        pulseGenerator->RequestStopOfPeriodicUpdates( this );
        UnsubscribeFrom( pulseGenerator.GetPointerAlways() );
        m_pulsing = false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CWaylandConnection::Connect( CORE::CString* problem )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_display )
        return true;

    m_display = m_api.display_connect( m_displayName.IsNULLOrEmpty() ? GUCEF_NULL : m_displayName.C_String() );
    if ( GUCEF_NULL == m_display )
    {
        if ( GUCEF_NULL != problem )
            *problem = "the compositor did not accept the connection";
        return false;
    }
    m_healthy = true;

    // what a fatal signal in the events of the compositor says about itself is only wanted where there is a compositor
    CEventGuard::InstallFatalSignalReporter();
    CDispatchPhase dispatchPhase;

    // wl_display.get_registry is request 1 of the one object every connection starts with
    m_registry = (struct wl_registry*) m_api.proxy_marshal_flags( (struct wl_proxy*) m_display, 1, &gucef_wl_registry_interface, 1, 0, GUCEF_NULL );
    if ( GUCEF_NULL == m_registry )
    {
        if ( GUCEF_NULL != problem )
            *problem = "the registry of the compositor could not be asked for";
        Disconnect();
        return false;
    }
    gucef_wl_registry_add_listener( &m_api, m_registry, &g_registryListener, this );

    // the globals, then what binding them makes the compositor say: the monitors and the seat, then the keyboard that the seat made
    bool ok = m_api.display_roundtrip( m_display ) >= 0;
    for ( UInt32 i=0; ok && i<m_outputs.size(); ++i )
        AttachOutputExtension( *m_outputs[ i ] );
    ok = ok && m_api.display_roundtrip( m_display ) >= 0;
    ok = ok && ( GUCEF_NULL == m_keyboard || m_api.display_roundtrip( m_display ) >= 0 );
    if ( !ok )
    {
        if ( GUCEF_NULL != problem )
            *problem = "the compositor stopped answering while the connection was set up";
        Disconnect();
        return false;
    }

    if ( m_pumpOnPulse )
    {
        CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        SubscribeTo( pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent );
        pulseGenerator->RequestPeriodicPulses( this, PumpIntervalInMs );
        m_pulsing = true;
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CWaylandConnection: Connected to the compositor, " << (UInt32) m_globals.size() << " globals, " << (UInt32) m_outputs.size() << " monitors" );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWaylandConnection::Disconnect( void )
{GUCEF_TRACE;

    if ( m_pulsing )
        SetPumpOnPulse( false );

    if ( GUCEF_NULL == m_display )
        return;

    // listeners that are still there lose their devices
    ReleaseSeat();
    m_cursor.Shutdown();
    for ( UInt32 i=0; i<m_outputs.size(); ++i )
    {
        SWaylandOutput* output = m_outputs[ i ];
        if ( GUCEF_NULL != output->xdgOutput )
            gucef_zxdg_output_v1_destroy( &m_api, output->xdgOutput );
        if ( GUCEF_NULL != output->proxy )
        {
            if ( m_api.proxy_get_version( (struct wl_proxy*) output->proxy ) >= GUCEF_WL_WL_OUTPUT_RELEASE_SINCE_VERSION )
                gucef_wl_output_release( &m_api, output->proxy );
            else
                m_api.proxy_destroy( (struct wl_proxy*) output->proxy );
        }
        delete output;
    }
    m_outputs.clear();

    if ( GUCEF_NULL != m_activationManager ) gucef_xdg_activation_v1_destroy( &m_api, m_activationManager );
    if ( GUCEF_NULL != m_outputManager ) gucef_zxdg_output_manager_v1_destroy( &m_api, m_outputManager );
    if ( GUCEF_NULL != m_cursorShapeManager ) gucef_wp_cursor_shape_manager_v1_destroy( &m_api, m_cursorShapeManager );
    if ( GUCEF_NULL != m_decorationManager ) gucef_zxdg_decoration_manager_v1_destroy( &m_api, m_decorationManager );
    if ( GUCEF_NULL != m_fractionalScaleManager ) gucef_wp_fractional_scale_manager_v1_destroy( &m_api, m_fractionalScaleManager );
    if ( GUCEF_NULL != m_viewporter ) gucef_wp_viewporter_destroy( &m_api, m_viewporter );
    if ( GUCEF_NULL != m_wmBase ) gucef_xdg_wm_base_destroy( &m_api, m_wmBase );
    if ( GUCEF_NULL != m_shm ) m_api.proxy_destroy( (struct wl_proxy*) m_shm );
    if ( GUCEF_NULL != m_compositor ) m_api.proxy_destroy( (struct wl_proxy*) m_compositor );
    m_activationManager = GUCEF_NULL;
    m_outputManager = GUCEF_NULL;
    m_cursorShapeManager = GUCEF_NULL;
    m_decorationManager = GUCEF_NULL;
    m_fractionalScaleManager = GUCEF_NULL;
    m_viewporter = GUCEF_NULL;
    m_wmBase = GUCEF_NULL;
    m_shm = GUCEF_NULL;
    m_compositor = GUCEF_NULL;

    if ( GUCEF_NULL != m_registry )
        m_api.proxy_destroy( (struct wl_proxy*) m_registry );
    m_registry = GUCEF_NULL;
    m_api.display_flush( m_display );
    m_api.display_disconnect( m_display );
    m_display = GUCEF_NULL;
    m_globals.clear();
}

/*-------------------------------------------------------------------------*/

bool
CWaylandConnection::IsConnected( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_display;
}

/*-------------------------------------------------------------------------*/

bool
CWaylandConnection::IsHealthy( void ) const
{GUCEF_TRACE;

    return m_healthy && GUCEF_NULL != m_display;
}

/*-------------------------------------------------------------------------*/

const SWaylandApi&
CWaylandConnection::GetApi( void ) const
{GUCEF_TRACE;

    return m_api;
}

/*-------------------------------------------------------------------------*/

struct wl_display*
CWaylandConnection::GetDisplay( void ) const
{GUCEF_TRACE;

    return m_display;
}

/*-------------------------------------------------------------------------*/

void
CWaylandConnection::OnFatalError( void )
{GUCEF_TRACE;

    if ( !m_healthy )
        return;
    m_healthy = false;
    const int error = GUCEF_NULL != m_display ? m_api.display_get_error( m_display ) : 0;
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CWaylandConnection: The connection to the compositor is lost (error " << error << ")" );
    NotifyObservers( ConnectionLostEvent );
}

/*-------------------------------------------------------------------------*/

bool
CWaylandConnection::Pump( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_display || !m_healthy )
        return false;
    if ( m_pumping )
        return true;
    m_pumping = true;
    CDispatchPhase dispatchPhase;

    // events the library read while it waited for something else go first: reading is only allowed with nothing pending
    while ( 0 != m_api.display_prepare_read( m_display ) )
    {
        if ( m_api.display_dispatch_pending( m_display ) < 0 )
        {
            m_pumping = false;
            OnFatalError();
            return false;
        }
    }

    // requests out; a full socket (EAGAIN) is flushed on a later pulse
    if ( m_api.display_flush( m_display ) < 0 && EAGAIN != errno )
    {
        m_api.display_cancel_read( m_display );
        m_pumping = false;
        OnFatalError();
        return false;
    }

    struct pollfd pollInfo;
    pollInfo.fd = m_api.display_get_fd( m_display );
    pollInfo.events = POLLIN;
    pollInfo.revents = 0;
    const int polled = pollInfo.fd >= 0 ? ::poll( &pollInfo, 1, 0 ) : 0;
    if ( polled > 0 && 0 != ( pollInfo.revents & POLLIN ) )
    {
        if ( m_api.display_read_events( m_display ) < 0 )
        {
            m_pumping = false;
            OnFatalError();
            return false;
        }
    }
    else
    {
        m_api.display_cancel_read( m_display );
        if ( polled > 0 && 0 != ( pollInfo.revents & ( POLLERR | POLLHUP | POLLNVAL ) ) )
        {
            m_pumping = false;
            OnFatalError();
            return false;
        }
    }

    const bool dispatched = m_api.display_dispatch_pending( m_display ) >= 0;
    if ( dispatched )
        m_api.display_flush( m_display );      // what the handlers asked for goes out in the same pulse
    m_pumping = false;
    if ( !dispatched )
    {
        OnFatalError();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWaylandConnection::Roundtrip( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_display )
        return false;
    CDispatchPhase dispatchPhase;
    return m_api.display_roundtrip( m_display ) >= 0;
}

/*-------------------------------------------------------------------------*/

void
CWaylandConnection::OnNotify( CORE::CNotifier* notifier   ,
                              const CORE::CEvent& eventId ,
                              CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( CORE::CPulseGenerator::PulseEvent == eventId && m_pumpOnPulse )
        Pump();
}

/*-------------------------------------------------------------------------*/

void
CWaylandConnection::OnGlobal( UInt32 name, const char* interfaceName, UInt32 version )
{GUCEF_TRACE;

    SWaylandGlobal global;
    global.name = name;
    global.interfaceName = interfaceName;
    global.version = version;
    m_globals.push_back( global );
    BindGlobal( global );
}

/*-------------------------------------------------------------------------*/

void
CWaylandConnection::BindGlobal( const SWaylandGlobal& global )
{GUCEF_TRACE;

    const CORE::CString& name = global.interfaceName;
    if ( name == "wl_compositor" && GUCEF_NULL == m_compositor )
    {
        m_compositorVersion = MinVersion( global.version, CWaylandConnection::MaxCompositorVersion );
        m_compositor = (struct wl_compositor*) gucef_wl_registry_bind( &m_api, m_registry, global.name, &gucef_wl_compositor_interface, m_compositorVersion );
    }
    else
    if ( name == "xdg_wm_base" && GUCEF_NULL == m_wmBase )
    {
        m_wmBase = (struct xdg_wm_base*) gucef_wl_registry_bind( &m_api, m_registry, global.name, &gucef_xdg_wm_base_interface, MinVersion( global.version, CWaylandConnection::MaxWmBaseVersion ) );
        gucef_xdg_wm_base_add_listener( &m_api, m_wmBase, &g_wmBaseListener, this );
    }
    else
    if ( name == "wl_shm" && GUCEF_NULL == m_shm )
        m_shm = (struct wl_shm*) gucef_wl_registry_bind( &m_api, m_registry, global.name, &gucef_wl_shm_interface, 1 );
    else
    if ( name == "wp_viewporter" && GUCEF_NULL == m_viewporter )
        m_viewporter = (struct wp_viewporter*) gucef_wl_registry_bind( &m_api, m_registry, global.name, &gucef_wp_viewporter_interface, 1 );
    else
    if ( name == "wp_fractional_scale_manager_v1" && GUCEF_NULL == m_fractionalScaleManager )
        m_fractionalScaleManager = (struct wp_fractional_scale_manager_v1*) gucef_wl_registry_bind( &m_api, m_registry, global.name, &gucef_wp_fractional_scale_manager_v1_interface, 1 );
    else
    if ( name == "zxdg_decoration_manager_v1" && GUCEF_NULL == m_decorationManager )
        m_decorationManager = (struct zxdg_decoration_manager_v1*) gucef_wl_registry_bind( &m_api, m_registry, global.name, &gucef_zxdg_decoration_manager_v1_interface, 1 );
    else
    if ( name == "wp_cursor_shape_manager_v1" && GUCEF_NULL == m_cursorShapeManager )
        m_cursorShapeManager = (struct wp_cursor_shape_manager_v1*) gucef_wl_registry_bind( &m_api, m_registry, global.name, &gucef_wp_cursor_shape_manager_v1_interface, 1 );
    else
    if ( name == "xdg_activation_v1" && GUCEF_NULL == m_activationManager )
        m_activationManager = (struct xdg_activation_v1*) gucef_wl_registry_bind( &m_api, m_registry, global.name, &gucef_xdg_activation_v1_interface, 1 );
    else
    if ( name == "zxdg_output_manager_v1" && GUCEF_NULL == m_outputManager )
    {
        m_outputManager = (struct zxdg_output_manager_v1*) gucef_wl_registry_bind( &m_api, m_registry, global.name, &gucef_zxdg_output_manager_v1_interface, MinVersion( global.version, CWaylandConnection::MaxOutputManagerVersion ) );
        for ( UInt32 i=0; i<m_outputs.size(); ++i )
            AttachOutputExtension( *m_outputs[ i ] );
    }
    else
    if ( name == "wl_output" )
    {
        SWaylandOutput* output = new SWaylandOutput();
        output->globalName = global.name;
        output->owner = this;
        output->proxy = (struct wl_output*) gucef_wl_registry_bind( &m_api, m_registry, global.name, &gucef_wl_output_interface, MinVersion( global.version, CWaylandConnection::MaxOutputVersion ) );
        gucef_wl_output_add_listener( &m_api, output->proxy, &g_outputListener, output );
        m_outputs.push_back( output );
        AttachOutputExtension( *output );
    }
    else
    if ( name == "wl_seat" && GUCEF_NULL == m_seat )
    {
        m_seatName = global.name;
        m_seatVersion = MinVersion( global.version, CWaylandConnection::MaxSeatVersion );
        m_seat = (struct wl_seat*) gucef_wl_registry_bind( &m_api, m_registry, global.name, &gucef_wl_seat_interface, m_seatVersion );
        gucef_wl_seat_add_listener( &m_api, m_seat, &g_seatListener, this );
    }
}

/*-------------------------------------------------------------------------*/

void
CWaylandConnection::AttachOutputExtension( SWaylandOutput& output )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_outputManager || GUCEF_NULL != output.xdgOutput || GUCEF_NULL == output.proxy )
        return;
    output.xdgOutput = gucef_zxdg_output_manager_v1_get_xdg_output( &m_api, m_outputManager, output.proxy );
    gucef_zxdg_output_v1_add_listener( &m_api, output.xdgOutput, &g_xdgOutputListener, &output );
}

/*-------------------------------------------------------------------------*/

void
CWaylandConnection::OnGlobalRemoved( UInt32 name )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_globals.size(); ++i )
    {
        if ( m_globals[ i ].name == name )
        {
            m_globals.erase( m_globals.begin() + i );
            break;
        }
    }
    for ( UInt32 i=0; i<m_outputs.size(); ++i )
    {
        if ( m_outputs[ i ]->globalName == name )
        {
            RemoveOutput( i );
            NotifyObservers( OutputsChangedEvent );
            return;
        }
    }
    if ( name == m_seatName && GUCEF_NULL != m_seat )
    {
        ReleaseSeat();
        NotifyObservers( OutputsChangedEvent );
    }
}

/*-------------------------------------------------------------------------*/

void
CWaylandConnection::RemoveOutput( UInt32 index )
{GUCEF_TRACE;

    SWaylandOutput* output = m_outputs[ index ];
    m_outputs.erase( m_outputs.begin() + index );
    if ( GUCEF_NULL != output->xdgOutput )
        gucef_zxdg_output_v1_destroy( &m_api, output->xdgOutput );
    if ( GUCEF_NULL != output->proxy )
    {
        if ( m_api.proxy_get_version( (struct wl_proxy*) output->proxy ) >= GUCEF_WL_WL_OUTPUT_RELEASE_SINCE_VERSION )
            gucef_wl_output_release( &m_api, output->proxy );
        else
            m_api.proxy_destroy( (struct wl_proxy*) output->proxy );
    }
    delete output;
}

/*-------------------------------------------------------------------------*/

void
CWaylandConnection::OnSeatCapabilitiesEvent( UInt32 capabilities )
{GUCEF_TRACE;

    m_seatCapabilities = capabilities;

    const bool wantPointer = 0 != ( capabilities & GUCEF_WL_WL_SEAT_CAPABILITY_POINTER );
    if ( wantPointer && GUCEF_NULL == m_pointer )
    {
        m_pointer = gucef_wl_seat_get_pointer( &m_api, m_seat );
        gucef_wl_pointer_add_listener( &m_api, m_pointer, &g_pointerListener, this );
        m_cursor.OnPointerChanged( m_pointer );
    }
    else
    if ( !wantPointer && GUCEF_NULL != m_pointer )
        ReleasePointer();

    const bool wantKeyboard = 0 != ( capabilities & GUCEF_WL_WL_SEAT_CAPABILITY_KEYBOARD );
    if ( wantKeyboard && GUCEF_NULL == m_keyboard )
    {
        m_keyboard = gucef_wl_seat_get_keyboard( &m_api, m_seat );
        gucef_wl_keyboard_add_listener( &m_api, m_keyboard, &g_keyboardListener, this );
    }
    else
    if ( !wantKeyboard && GUCEF_NULL != m_keyboard )
        ReleaseKeyboard();

    CDispatchScope dispatchScope( *this );
    for ( UInt32 i=0; i<m_listeners.size(); ++i )
        if ( GUCEF_NULL != m_listeners[ i ] ) m_listeners[ i ]->OnSeatCapabilities( capabilities );
}

/*-------------------------------------------------------------------------*/

void
CWaylandConnection::ReleasePointer( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_pointer )
        return;
    m_cursor.OnPointerChanged( GUCEF_NULL );
    if ( m_api.proxy_get_version( (struct wl_proxy*) m_pointer ) >= GUCEF_WL_WL_POINTER_RELEASE_SINCE_VERSION )
        gucef_wl_pointer_release( &m_api, m_pointer );
    else
        m_api.proxy_destroy( (struct wl_proxy*) m_pointer );
    m_pointer = GUCEF_NULL;
    m_pointerFocus = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CWaylandConnection::ReleaseKeyboard( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_keyboard )
        return;
    if ( m_api.proxy_get_version( (struct wl_proxy*) m_keyboard ) >= GUCEF_WL_WL_KEYBOARD_RELEASE_SINCE_VERSION )
        gucef_wl_keyboard_release( &m_api, m_keyboard );
    else
        m_api.proxy_destroy( (struct wl_proxy*) m_keyboard );
    m_keyboard = GUCEF_NULL;
    m_keyboardFocus = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CWaylandConnection::ReleaseSeat( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_seat )
        return;
    const bool hadDevices = 0 != m_seatCapabilities;
    ReleasePointer();
    ReleaseKeyboard();
    if ( m_api.proxy_get_version( (struct wl_proxy*) m_seat ) >= GUCEF_WL_WL_SEAT_RELEASE_SINCE_VERSION )
        gucef_wl_seat_release( &m_api, m_seat );
    else
        m_api.proxy_destroy( (struct wl_proxy*) m_seat );
    m_seat = GUCEF_NULL;
    m_seatName = 0;
    m_seatCapabilities = 0;
    if ( hadDevices )
    {
        CDispatchScope dispatchScope( *this );
        for ( UInt32 i=0; i<m_listeners.size(); ++i )
            if ( GUCEF_NULL != m_listeners[ i ] ) m_listeners[ i ]->OnSeatCapabilities( 0 );
    }
}

/*-------------------------------------------------------------------------*/

void
CWaylandConnection::CompactListeners( void )
{GUCEF_TRACE;

    if ( 0 != m_dispatchDepth || !m_compactListeners )
        return;
    for ( UInt32 i=0; i<m_listeners.size(); )
    {
        if ( GUCEF_NULL == m_listeners[ i ] )
            m_listeners.erase( m_listeners.begin() + i );
        else
            ++i;
    }
    m_compactListeners = false;
}

/*-------------------------------------------------------------------------*/

void
CWaylandConnection::AddSeatListener( IWaylandSeatListener* listener )
{GUCEF_TRACE;

    if ( GUCEF_NULL == listener )
        return;
    for ( UInt32 i=0; i<m_listeners.size(); ++i )
        if ( m_listeners[ i ] == listener )
            return;
    m_listeners.push_back( listener );

    // what the seat already has is told to the newcomer: the devices first, then what a keyboard says once, then the focus
    if ( GUCEF_NULL != m_seat )
        listener->OnSeatCapabilities( m_seatCapabilities );
    if ( m_hasRepeatInfo )
        listener->OnKeyboardRepeatInfo( m_repeatRate, m_repeatDelay );
    if ( !m_keymap.empty() )
        listener->OnKeyboardKeymap( m_keymapFormat, &m_keymap[ 0 ], (UInt32) m_keymap.size() );
    if ( GUCEF_NULL != m_pointerFocus )
        listener->OnPointerEnter( m_pointerEnterSerial, m_pointerFocus, m_pointerX, m_pointerY );
    if ( GUCEF_NULL != m_keyboardFocus )
        listener->OnKeyboardEnter( m_lastInputSerial, m_keyboardFocus, GUCEF_NULL, 0 );
}

/*-------------------------------------------------------------------------*/

void
CWaylandConnection::RemoveSeatListener( IWaylandSeatListener* listener )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_listeners.size(); ++i )
    {
        if ( m_listeners[ i ] == listener )
        {
            if ( 0 != m_dispatchDepth )
            {
                m_listeners[ i ] = GUCEF_NULL;
                m_compactListeners = true;
            }
            else
                m_listeners.erase( m_listeners.begin() + i );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

CWaylandCursor&
CWaylandConnection::GetCursor( void )
{GUCEF_TRACE;

    return m_cursor;
}

/*-------------------------------------------------------------------------*/

UInt32
CWaylandConnection::RegisterSurface( struct wl_surface* surface )
{GUCEF_TRACE;

    SWaylandSurfaceInfo* info = new SWaylandSurfaceInfo();
    info->id = m_nextSurfaceId++;
    info->surface = surface;
    m_surfaces[ info->id ] = info;
    return info->id;
}

/*-------------------------------------------------------------------------*/

void
CWaylandConnection::UnregisterSurface( UInt32 id )
{GUCEF_TRACE;

    TSurfaceMap::iterator i = m_surfaces.find( id );
    if ( i == m_surfaces.end() )
        return;
    if ( m_pointerFocus == (*i).second->surface )
        m_cursor.OnPointerLeave();
    delete (*i).second;
    m_surfaces.erase( i );
}

/*-------------------------------------------------------------------------*/

SWaylandSurfaceInfo*
CWaylandConnection::FindSurface( UInt32 id )
{GUCEF_TRACE;

    TSurfaceMap::iterator i = m_surfaces.find( id );
    return i == m_surfaces.end() ? GUCEF_NULL : (*i).second;
}

/*-------------------------------------------------------------------------*/

SWaylandSurfaceInfo*
CWaylandConnection::FindSurfaceByProxy( const struct wl_surface* surface )
{GUCEF_TRACE;

    if ( GUCEF_NULL == surface )
        return GUCEF_NULL;
    for ( TSurfaceMap::iterator i=m_surfaces.begin(); i!=m_surfaces.end(); ++i )
        if ( (*i).second->surface == surface )
            return (*i).second;
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CWaylandConnection::SetSurfaceScale( UInt32 id, UInt32 scale120 )
{GUCEF_TRACE;

    SWaylandSurfaceInfo* info = FindSurface( id );
    if ( GUCEF_NULL != info )
        info->scale120 = scale120;
}

/*-------------------------------------------------------------------------*/

const TWaylandGlobalList&
CWaylandConnection::GetGlobals( void ) const
{GUCEF_TRACE;

    return m_globals;
}

/*-------------------------------------------------------------------------*/

bool
CWaylandConnection::HasGlobal( const char* interfaceName, UInt32 minimumVersion ) const
{GUCEF_TRACE;

    return CWaylandAvailability::HasGlobal( m_globals, interfaceName, minimumVersion );
}

/*-------------------------------------------------------------------------*/

struct wl_compositor* CWaylandConnection::GetCompositor( void ) const { return m_compositor; }
UInt32 CWaylandConnection::GetCompositorVersion( void ) const { return m_compositorVersion; }
struct xdg_wm_base* CWaylandConnection::GetWmBase( void ) const { return m_wmBase; }
struct wl_shm* CWaylandConnection::GetShm( void ) const { return m_shm; }
struct wp_viewporter* CWaylandConnection::GetViewporter( void ) const { return m_viewporter; }
struct wp_fractional_scale_manager_v1* CWaylandConnection::GetFractionalScaleManager( void ) const { return m_fractionalScaleManager; }
struct zxdg_decoration_manager_v1* CWaylandConnection::GetDecorationManager( void ) const { return m_decorationManager; }
struct wp_cursor_shape_manager_v1* CWaylandConnection::GetCursorShapeManager( void ) const { return m_cursorShapeManager; }
struct zxdg_output_manager_v1* CWaylandConnection::GetOutputManager( void ) const { return m_outputManager; }
struct xdg_activation_v1* CWaylandConnection::GetActivationManager( void ) const { return m_activationManager; }
struct wl_seat* CWaylandConnection::GetSeat( void ) const { return m_seat; }
UInt32 CWaylandConnection::GetSeatVersion( void ) const { return m_seatVersion; }
UInt32 CWaylandConnection::GetSeatCapabilities( void ) const { return m_seatCapabilities; }
struct wl_pointer* CWaylandConnection::GetPointer( void ) const { return m_pointer; }
struct wl_keyboard* CWaylandConnection::GetKeyboard( void ) const { return m_keyboard; }
UInt32 CWaylandConnection::GetLastInputSerial( void ) const { return m_lastInputSerial; }
UInt32 CWaylandConnection::GetLastActivationSerial( void ) const { return m_lastActivationSerial; }
UInt32 CWaylandConnection::GetPointerEnterSerial( void ) const { return m_pointerEnterSerial; }
struct wl_surface* CWaylandConnection::GetPointerFocus( void ) const { return m_pointerFocus; }
struct wl_surface* CWaylandConnection::GetKeyboardFocus( void ) const { return m_keyboardFocus; }

/*-------------------------------------------------------------------------*/

UInt32
CWaylandConnection::GetOutputCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_outputs.size();
}

/*-------------------------------------------------------------------------*/

const SWaylandOutput*
CWaylandConnection::GetOutput( UInt32 index ) const
{GUCEF_TRACE;

    return index < m_outputs.size() ? m_outputs[ index ] : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const SWaylandOutput*
CWaylandConnection::FindOutput( const struct wl_output* proxy ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_outputs.size(); ++i )
        if ( m_outputs[ i ]->proxy == proxy )
            return m_outputs[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CWaylandConnection::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CORE::CString classTypeName = "GUCEF::PLUGINGLUE::WAYLAND::CWaylandConnection";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WAYLAND */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
