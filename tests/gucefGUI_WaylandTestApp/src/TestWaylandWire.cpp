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

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdexcept>
#include <sys/resource.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/mman.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_H
#include "pluginglueWAYLAND.h"
#define GUCEF_PLUGINGLUE_WAYLAND_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_H ? */

#ifndef GUCEF_INPUT_WAYLAND_CWAYLANDINPUTCONTEXT_H
#include "inputdriverWAYLAND_CWaylandInputContext.h"
#define GUCEF_INPUT_WAYLAND_CWAYLANDINPUTCONTEXT_H
#endif /* GUCEF_INPUT_WAYLAND_CWAYLANDINPUTCONTEXT_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_CEVENTGUARD_H
#include "pluginglueWAYLAND_CEventGuard.h"
#define GUCEF_PLUGINGLUE_WAYLAND_CEVENTGUARD_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_CEVENTGUARD_H ? */

#include "TestWaylandCommon.h"
#include "TestWaylandWire.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::PLUGINGLUE::WAYLAND;
using namespace GUCEF::TESTWAYLAND;
using namespace GUCEF::INPUT::WAYLAND;

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE SERVER OF THE TEST                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

struct wl_client;
struct wl_resource;
struct wl_global;
struct wl_event_loop;

/** libwayland-server as far as the test uses it */
struct SServerApi
{
    struct wl_display* ( *display_create )( void );
    void ( *display_destroy )( struct wl_display* );
    int ( *display_add_socket_fd )( struct wl_display*, int fd );
    struct wl_event_loop* ( *display_get_event_loop )( struct wl_display* );
    void ( *display_flush_clients )( struct wl_display* );
    int ( *event_loop_dispatch )( struct wl_event_loop*, int timeout );
    struct wl_global* ( *global_create )( struct wl_display*, const struct wl_interface*, int version, void* data, void ( *bind )( struct wl_client*, void*, uint32_t, uint32_t ) );
    struct wl_resource* ( *resource_create )( struct wl_client*, const struct wl_interface*, int version, uint32_t id );
    void ( *resource_set_implementation )( struct wl_resource*, const void* implementation, void* data, void ( *destroy )( struct wl_resource* ) );
    void ( *resource_post_event )( struct wl_resource*, uint32_t opcode, ... );
    void* ( *resource_get_user_data )( struct wl_resource* );
    uint32_t ( *resource_get_id )( struct wl_resource* );
    int ( *resource_get_version )( struct wl_resource* );
};

SServerApi g_server;

/** What the server does when the test asks: the server thread is the only one that touches libwayland-server */
enum EServerAction
{
    ACTION_NONE = 0 ,
    ACTION_PING ,
    ACTION_POINTER ,        /**< enter, motion, button, axis and frame on the surface of the client */
    ACTION_SCRIPT ,         /**< the events of g_script on the pointer, in order */
    ACTION_STOP
};

/** One event of the pointer the server sends: the opcode and up to four 32 bit arguments as they go on the wire. A surface argument is written as SCRIPT_SURFACE */
struct SScriptedEvent
{
    uint32_t opcode;
    int count;
    uint32_t value[ 4 ];
};

const uint32_t SCRIPT_SURFACE = 0xFFFFFFFFu;
const int MaxScriptedEvents = 256;

struct SServerState
{
    pthread_t thread;
    pthread_mutex_t lock;
    struct wl_display* display;
    struct wl_event_loop* loop;
    int action;
    int seatGlobalVersion;
    int scriptCount;
    bool scriptDone;
    SScriptedEvent script[ MaxScriptedEvents ];
    struct wl_resource* seat;
    struct wl_resource* pointer;
    struct wl_resource* keyboard;
    struct wl_resource* surface;
    uint32_t pongSerial;
    bool gotPong;
    uint32_t cursorSerial;
    bool gotSetCursor;
    bool gotCommit;
    char* keymap;
    bool running;
    int listenFd;
};

SServerState g_state;

/** Apart from the state: that is reset with memset, which must not touch a string */
CORE::CString g_socketPath;

// --- the requests the server understands

void ServerNothing( struct wl_client*, struct wl_resource* ) {}
void ServerCreateSurfaceDone( struct wl_client*, struct wl_resource* ) {}

void
SurfaceDestroy( struct wl_client*, struct wl_resource* )
{
}

void SurfaceAttach( struct wl_client*, struct wl_resource*, struct wl_resource*, int32_t, int32_t ) {}
void SurfaceDamage( struct wl_client*, struct wl_resource*, int32_t, int32_t, int32_t, int32_t ) {}
void SurfaceFrame( struct wl_client*, struct wl_resource*, uint32_t ) {}
void SurfaceSetRegion( struct wl_client*, struct wl_resource*, struct wl_resource* ) {}
void SurfaceCommit( struct wl_client*, struct wl_resource* ) { g_state.gotCommit = true; }
void SurfaceInt( struct wl_client*, struct wl_resource*, int32_t ) {}
void SurfaceOffset( struct wl_client*, struct wl_resource*, int32_t, int32_t ) {}

void ( *const g_surfaceImpl[] )( void ) = {
    (void ( * )( void )) SurfaceDestroy, (void ( * )( void )) SurfaceAttach, (void ( * )( void )) SurfaceDamage, (void ( * )( void )) SurfaceFrame,
    (void ( * )( void )) SurfaceSetRegion, (void ( * )( void )) SurfaceSetRegion, (void ( * )( void )) SurfaceCommit, (void ( * )( void )) SurfaceInt,
    (void ( * )( void )) SurfaceInt, (void ( * )( void )) SurfaceDamage, (void ( * )( void )) SurfaceOffset };

void
CompositorCreateSurface( struct wl_client* client, struct wl_resource* compositor, uint32_t id )
{
    struct wl_resource* surface = g_server.resource_create( client, &gucef_wl_surface_interface, 6, id );
    g_server.resource_set_implementation( surface, g_surfaceImpl, GUCEF_NULL, GUCEF_NULL );
    g_state.surface = surface;
}

void
CompositorCreateRegion( struct wl_client*, struct wl_resource*, uint32_t )
{
}

void ( *const g_compositorImpl[] )( void ) = { (void ( * )( void )) CompositorCreateSurface, (void ( * )( void )) CompositorCreateRegion };

void
PointerSetCursor( struct wl_client*, struct wl_resource*, uint32_t serial, struct wl_resource*, int32_t, int32_t )
{
    g_state.cursorSerial = serial;
    g_state.gotSetCursor = true;
}

void PointerRelease( struct wl_client*, struct wl_resource* ) {}

void ( *const g_pointerImpl[] )( void ) = { (void ( * )( void )) PointerSetCursor, (void ( * )( void )) PointerRelease };
void ( *const g_keyboardImpl[] )( void ) = { (void ( * )( void )) PointerRelease };
void ( *const g_outputImpl[] )( void ) = { (void ( * )( void )) PointerRelease };

void
SeatGetPointer( struct wl_client* client, struct wl_resource*, uint32_t id )
{
    g_state.pointer = g_server.resource_create( client, &gucef_wl_pointer_interface, g_server.resource_get_version( g_state.seat ), id );
    g_server.resource_set_implementation( g_state.pointer, g_pointerImpl, GUCEF_NULL, GUCEF_NULL );
}

void
SeatGetKeyboard( struct wl_client* client, struct wl_resource*, uint32_t id )
{
    g_state.keyboard = g_server.resource_create( client, &gucef_wl_keyboard_interface, 9, id );
    g_server.resource_set_implementation( g_state.keyboard, g_keyboardImpl, GUCEF_NULL, GUCEF_NULL );

    // the keymap travels as a file descriptor, then the repeat rate
    const size_t size = strlen( g_state.keymap ) + 1;
    char path[] = "/tmp/gucef-wire-keymapXXXXXX";
    int fd = ::mkstemp( path );
    if ( fd >= 0 )
    {
        ::unlink( path );
        if ( (ssize_t) size == ::write( fd, g_state.keymap, size ) )
            g_server.resource_post_event( g_state.keyboard, GUCEF_WL_WL_KEYBOARD_KEYMAP_EVENT_OPCODE, (uint32_t) GUCEF_WL_WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1, fd, (uint32_t) size );
        ::close( fd );
    }
    g_server.resource_post_event( g_state.keyboard, GUCEF_WL_WL_KEYBOARD_REPEAT_INFO_EVENT_OPCODE, (int32_t) 33, (int32_t) 250 );
}

void SeatGetTouch( struct wl_client*, struct wl_resource*, uint32_t ) {}

void ( *const g_seatImpl[] )( void ) = { (void ( * )( void )) SeatGetPointer, (void ( * )( void )) SeatGetKeyboard, (void ( * )( void )) SeatGetTouch, (void ( * )( void )) PointerRelease };

void WmBaseCreatePositioner( struct wl_client*, struct wl_resource*, uint32_t ) {}
void WmBaseGetXdgSurface( struct wl_client*, struct wl_resource*, uint32_t, struct wl_resource* ) {}

void
WmBasePong( struct wl_client*, struct wl_resource*, uint32_t serial )
{
    g_state.pongSerial = serial;
    g_state.gotPong = true;
}

void ( *const g_wmBaseImpl[] )( void ) = { (void ( * )( void )) ServerNothing, (void ( * )( void )) WmBaseCreatePositioner, (void ( * )( void )) WmBaseGetXdgSurface, (void ( * )( void )) WmBasePong };
struct wl_resource* g_wmBase = GUCEF_NULL;

// --- xdg-activation: the compositor answers a committed token with a done event

struct SActivationServer
{
    bool offered;
    bool answers;
    int commits;
    bool gotSerial;
    uint32_t serial;
    bool surfaceIsOurs;
    char appId[ 64 ];
};

SActivationServer g_activation;

void ActivationGetToken( struct wl_client* client, struct wl_resource*, uint32_t id );

void
TokenSetSerial( struct wl_client*, struct wl_resource*, uint32_t serial, struct wl_resource* )
{
    g_activation.gotSerial = true;
    g_activation.serial = serial;
}

void
TokenSetAppId( struct wl_client*, struct wl_resource*, const char* appId )
{
    strncpy( g_activation.appId, GUCEF_NULL != appId ? appId : "", sizeof( g_activation.appId ) - 1 );
}

void
TokenSetSurface( struct wl_client*, struct wl_resource*, struct wl_resource* surface )
{
    g_activation.surfaceIsOurs = surface == g_state.surface;
}

void
TokenCommit( struct wl_client*, struct wl_resource* token )
{
    ++g_activation.commits;
    if ( g_activation.answers )
        g_server.resource_post_event( token, GUCEF_WL_XDG_ACTIVATION_TOKEN_V1_DONE_EVENT_OPCODE, "wire-activation-token" );
}

void ( *const g_tokenImpl[] )( void ) = { (void ( * )( void )) TokenSetSerial, (void ( * )( void )) TokenSetAppId, (void ( * )( void )) TokenSetSurface, (void ( * )( void )) TokenCommit, (void ( * )( void )) ServerNothing };

void
ActivationGetToken( struct wl_client* client, struct wl_resource*, uint32_t id )
{
    struct wl_resource* token = g_server.resource_create( client, &gucef_xdg_activation_token_v1_interface, 1, id );
    g_server.resource_set_implementation( token, g_tokenImpl, GUCEF_NULL, GUCEF_NULL );
}

void ActivationActivate( struct wl_client*, struct wl_resource*, const char*, struct wl_resource* ) {}

void ( *const g_activationImpl[] )( void ) = { (void ( * )( void )) ServerNothing, (void ( * )( void )) ActivationGetToken, (void ( * )( void )) ActivationActivate };

void
BindActivation( struct wl_client* client, void*, uint32_t version, uint32_t id )
{
    struct wl_resource* resource = g_server.resource_create( client, &gucef_xdg_activation_v1_interface, (int) version, id );
    g_server.resource_set_implementation( resource, g_activationImpl, GUCEF_NULL, GUCEF_NULL );
}

// --- binding the globals

void
BindCompositor( struct wl_client* client, void*, uint32_t version, uint32_t id )
{
    struct wl_resource* resource = g_server.resource_create( client, &gucef_wl_compositor_interface, (int) version, id );
    g_server.resource_set_implementation( resource, g_compositorImpl, GUCEF_NULL, GUCEF_NULL );
}

void
BindWmBase( struct wl_client* client, void*, uint32_t version, uint32_t id )
{
    g_wmBase = g_server.resource_create( client, &gucef_xdg_wm_base_interface, (int) version, id );
    g_server.resource_set_implementation( g_wmBase, g_wmBaseImpl, GUCEF_NULL, GUCEF_NULL );
}

void
BindSeat( struct wl_client* client, void*, uint32_t version, uint32_t id )
{
    g_state.seat = g_server.resource_create( client, &gucef_wl_seat_interface, (int) version, id );
    g_server.resource_set_implementation( g_state.seat, g_seatImpl, GUCEF_NULL, GUCEF_NULL );
    g_server.resource_post_event( g_state.seat, GUCEF_WL_WL_SEAT_CAPABILITIES_EVENT_OPCODE, (uint32_t) 3 );
}

void
BindOutput( struct wl_client* client, void*, uint32_t version, uint32_t id )
{
    struct wl_resource* output = g_server.resource_create( client, &gucef_wl_output_interface, (int) version, id );
    g_server.resource_set_implementation( output, g_outputImpl, GUCEF_NULL, GUCEF_NULL );
    g_server.resource_post_event( output, GUCEF_WL_WL_OUTPUT_GEOMETRY_EVENT_OPCODE, (int32_t) 0, (int32_t) 0, (int32_t) 600, (int32_t) 340, (int32_t) 0, "Wire", "Test", (int32_t) 0 );
    g_server.resource_post_event( output, GUCEF_WL_WL_OUTPUT_MODE_EVENT_OPCODE, (uint32_t) 3, (int32_t) 1920, (int32_t) 1080, (int32_t) 144000 );
    g_server.resource_post_event( output, GUCEF_WL_WL_OUTPUT_SCALE_EVENT_OPCODE, (int32_t) 2 );
    g_server.resource_post_event( output, GUCEF_WL_WL_OUTPUT_DONE_EVENT_OPCODE );
}

// --- the thread

void*
ServerMain( void* )
{
    while ( true )
    {
        int action = ACTION_NONE;
        pthread_mutex_lock( &g_state.lock );
        action = g_state.action;
        g_state.action = ACTION_NONE;
        pthread_mutex_unlock( &g_state.lock );

        if ( ACTION_STOP == action )
            break;
        if ( ACTION_PING == action && GUCEF_NULL != g_wmBase )
            g_server.resource_post_event( g_wmBase, GUCEF_WL_XDG_WM_BASE_PING_EVENT_OPCODE, (uint32_t) 4711 );
        if ( ACTION_SCRIPT == action && GUCEF_NULL != g_state.pointer )
        {
            for ( int i=0; i<g_state.scriptCount; ++i )
            {
                const SScriptedEvent& event = g_state.script[ i ];
                const uint32_t* v = event.value;
                struct wl_resource* r = g_state.pointer;
                #define ARG( n ) ( SCRIPT_SURFACE == v[ n ] ? (void*) g_state.surface : (void*) (size_t) v[ n ] )
                // every argument is passed in a slot of its own, as the variable arguments of libwayland-server read them
                switch ( event.count )
                {
                    case 0: g_server.resource_post_event( r, event.opcode ); break;
                    case 1: g_server.resource_post_event( r, event.opcode, ARG( 0 ) ); break;
                    case 2: g_server.resource_post_event( r, event.opcode, ARG( 0 ), ARG( 1 ) ); break;
                    case 3: g_server.resource_post_event( r, event.opcode, ARG( 0 ), ARG( 1 ), ARG( 2 ) ); break;
                    default: g_server.resource_post_event( r, event.opcode, ARG( 0 ), ARG( 1 ), ARG( 2 ), ARG( 3 ) ); break;
                }
                #undef ARG
            }
            pthread_mutex_lock( &g_state.lock );
            g_state.scriptDone = true;
            pthread_mutex_unlock( &g_state.lock );
        }
        if ( ACTION_POINTER == action && GUCEF_NULL != g_state.pointer && GUCEF_NULL != g_state.surface )
        {
            g_server.resource_post_event( g_state.pointer, GUCEF_WL_WL_POINTER_ENTER_EVENT_OPCODE, (uint32_t) 900, g_state.surface, (int32_t) 2688, (int32_t) 5184 );      // 10.5, 20.25
            g_server.resource_post_event( g_state.pointer, GUCEF_WL_WL_POINTER_MOTION_EVENT_OPCODE, (uint32_t) 1, (int32_t) 5120, (int32_t) 7680 );                          // 20, 30
            g_server.resource_post_event( g_state.pointer, GUCEF_WL_WL_POINTER_BUTTON_EVENT_OPCODE, (uint32_t) 901, (uint32_t) 2, (uint32_t) 0x110, (uint32_t) 1 );
            g_server.resource_post_event( g_state.pointer, GUCEF_WL_WL_POINTER_AXIS_VALUE120_EVENT_OPCODE, (uint32_t) 0, (int32_t) 120 );
            g_server.resource_post_event( g_state.pointer, GUCEF_WL_WL_POINTER_FRAME_EVENT_OPCODE );
        }
        g_server.event_loop_dispatch( g_state.loop, 5 );
        g_server.display_flush_clients( g_state.display );
    }
    return GUCEF_NULL;
}

void
PostAction( int action )
{
    pthread_mutex_lock( &g_state.lock );
    g_state.action = action;
    pthread_mutex_unlock( &g_state.lock );
}

bool
LoadServer( void )
{
    void* lib = CWaylandRuntime::LoadLibraryOnce( "libwayland-server.so.0" );
    if ( GUCEF_NULL == lib )
        return false;
    #define LOAD( member, symbol ) g_server.member = (__typeof__( g_server.member )) CWaylandRuntime::GetSymbol( lib, symbol ); if ( GUCEF_NULL == g_server.member ) return false;
    LOAD( display_create, "wl_display_create" )
    LOAD( display_destroy, "wl_display_destroy" )
    LOAD( display_add_socket_fd, "wl_display_add_socket_fd" )
    LOAD( display_get_event_loop, "wl_display_get_event_loop" )
    LOAD( display_flush_clients, "wl_display_flush_clients" )
    LOAD( event_loop_dispatch, "wl_event_loop_dispatch" )
    LOAD( global_create, "wl_global_create" )
    LOAD( resource_create, "wl_resource_create" )
    LOAD( resource_set_implementation, "wl_resource_set_implementation" )
    LOAD( resource_post_event, "wl_resource_post_event" )
    LOAD( resource_get_user_data, "wl_resource_get_user_data" )
    LOAD( resource_get_id, "wl_resource_get_id" )
    LOAD( resource_get_version, "wl_resource_get_version" )
    #undef LOAD
    return true;
}

/** Starts the server on a socket in a directory of its own; false when it cannot (no directory, no library) */
bool
StartServer( const CORE::CString& directory, int seatGlobalVersion = 9 )
{
    memset( &g_state, 0, sizeof( g_state ) );
    g_state.seatGlobalVersion = seatGlobalVersion;
    g_wmBase = GUCEF_NULL;
    if ( !LoadServer() )
        return false;
    pthread_mutex_init( &g_state.lock, GUCEF_NULL );
    g_state.keymap = const_cast< char* >( GetTestKeymap() );
    g_socketPath = CORE::CombinePath( directory, "wire-socket" );

    g_state.listenFd = ::socket( AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0 );
    if ( g_state.listenFd < 0 )
        return false;
    struct sockaddr_un address;
    memset( &address, 0, sizeof( address ) );
    address.sun_family = AF_UNIX;
    strncpy( address.sun_path, g_socketPath.C_String(), sizeof( address.sun_path ) - 1 );
    ::unlink( address.sun_path );
    if ( 0 != ::bind( g_state.listenFd, (struct sockaddr*) &address, sizeof( address ) ) || 0 != ::listen( g_state.listenFd, 4 ) )
        return false;

    g_state.display = g_server.display_create();
    if ( GUCEF_NULL == g_state.display || 0 != g_server.display_add_socket_fd( g_state.display, g_state.listenFd ) )
        return false;
    g_state.loop = g_server.display_get_event_loop( g_state.display );
    g_server.global_create( g_state.display, &gucef_wl_compositor_interface, 6, GUCEF_NULL, BindCompositor );
    g_server.global_create( g_state.display, &gucef_xdg_wm_base_interface, 6, GUCEF_NULL, BindWmBase );
    g_server.global_create( g_state.display, &gucef_wl_seat_interface, g_state.seatGlobalVersion, GUCEF_NULL, BindSeat );
    g_server.global_create( g_state.display, &gucef_wl_output_interface, 4, GUCEF_NULL, BindOutput );
    if ( g_activation.offered )
        g_server.global_create( g_state.display, &gucef_xdg_activation_v1_interface, 1, GUCEF_NULL, BindActivation );
    g_state.running = 0 == pthread_create( &g_state.thread, GUCEF_NULL, ServerMain, GUCEF_NULL );
    return g_state.running;
}

void
StopServer( void )
{
    if ( g_state.running )
    {
        PostAction( ACTION_STOP );
        pthread_join( g_state.thread, GUCEF_NULL );
        g_state.running = false;
    }
    if ( GUCEF_NULL != g_state.display )
        g_server.display_destroy( g_state.display );      // closes the socket it was given
    g_state.display = GUCEF_NULL;
    if ( !g_socketPath.IsNULLOrEmpty() )
        ::unlink( g_socketPath.C_String() );
}

/** Pumps the connection until the condition holds (or two seconds are over) */
template< typename Condition >
bool
PumpUntil( CWaylandConnection& connection, Condition condition )
{
    for ( int i=0; i<2000; ++i )
    {
        connection.Pump();
        if ( condition() )
            return true;
        ::usleep( 1000 );
    }
    return false;
}

struct SPingDone { bool operator()() const { return g_state.gotPong; } };

UInt64
MonotonicTimeInMs( void )
{
    struct timespec now;
    ::clock_gettime( CLOCK_MONOTONIC, &now );
    return (UInt64) now.tv_sec * 1000 + (UInt64) now.tv_nsec / 1000000;
}

class CWireRecorder : public IWaylandSeatListener
{
    public:

    GUCEF::vector< CORE::CString > events;
    UInt32 keymapSize;
    CORE::CString keymapStart;

    CWireRecorder( void ) : events(), keymapSize( 0 ), keymapStart() {}

    virtual void OnSeatCapabilities( UInt32 capabilities ) { events.push_back( "caps " + CORE::UInt32ToString( capabilities ) ); }
    virtual void OnPointerEnter( UInt32 serial, struct wl_surface*, wl_fixed_t x, wl_fixed_t y ) { events.push_back( "enter " + CORE::UInt32ToString( serial ) + " " + CORE::Int32ToString( x ) + "," + CORE::Int32ToString( y ) ); }
    virtual void OnPointerMotion( UInt32 time, wl_fixed_t x, wl_fixed_t y ) { events.push_back( "motion " + CORE::Int32ToString( x ) + "," + CORE::Int32ToString( y ) ); }
    virtual void OnPointerButton( UInt32 serial, UInt32, UInt32 button, UInt32 state ) { events.push_back( "button " + CORE::UInt32ToString( serial ) + " " + CORE::UInt32ToString( button ) + " " + CORE::UInt32ToString( state ) ); }
    virtual void OnPointerAxisValue120( UInt32 axis, Int32 value120 ) { events.push_back( "value120 " + CORE::UInt32ToString( axis ) + " " + CORE::Int32ToString( value120 ) ); }
    virtual void OnPointerFrame( void ) { events.push_back( "frame" ); }
    virtual void OnKeyboardKeymap( UInt32 format, const char* data, UInt32 size ) { events.push_back( "keymap " + CORE::UInt32ToString( format ) ); keymapSize = size; keymapStart = CORE::CString( data, 11 ); }
    virtual void OnKeyboardRepeatInfo( Int32 rate, Int32 delay ) { events.push_back( "repeat " + CORE::Int32ToString( rate ) + " " + CORE::Int32ToString( delay ) ); }

    bool Has( const char* text ) const
    {
        for ( UInt32 i=0; i<events.size(); ++i )
            if ( events[ i ] == text )
                return true;
        return false;
    }
};

struct SFrameSeen
{
    CWireRecorder* recorder;
    bool operator()() const { return recorder->Has( "frame" ); }
};

struct SScriptDone { bool operator()() const { return g_state.scriptDone; } };

// --- the events of the pointer, as a script for the server

void ScriptClear( void ) { g_state.scriptCount = 0; g_state.scriptDone = false; }

void
ScriptAdd( uint32_t opcode, int count = 0, uint32_t a = 0, uint32_t b = 0, uint32_t c = 0, uint32_t d = 0 )
{
    if ( g_state.scriptCount >= MaxScriptedEvents )
        return;
    SScriptedEvent& event = g_state.script[ g_state.scriptCount++ ];
    event.opcode = opcode;
    event.count = count;
    event.value[ 0 ] = a; event.value[ 1 ] = b; event.value[ 2 ] = c; event.value[ 3 ] = d;
}

/** The surface units of a number of the wire (24.8 fixed point) */
uint32_t Fx( double value ) { return (uint32_t) (int32_t) ( value * 256.0 ); }

void ScriptEnter( double x, double y ) { ScriptAdd( GUCEF_WL_WL_POINTER_ENTER_EVENT_OPCODE, 4, 700, SCRIPT_SURFACE, Fx( x ), Fx( y ) ); }
void ScriptSource( uint32_t source ) { ScriptAdd( GUCEF_WL_WL_POINTER_AXIS_SOURCE_EVENT_OPCODE, 1, source ); }
void ScriptAxis( uint32_t axis, double value ) { ScriptAdd( GUCEF_WL_WL_POINTER_AXIS_EVENT_OPCODE, 3, 5, axis, Fx( value ) ); }
void ScriptValue120( uint32_t axis, int32_t value ) { ScriptAdd( GUCEF_WL_WL_POINTER_AXIS_VALUE120_EVENT_OPCODE, 2, axis, (uint32_t) value ); }
void ScriptDiscrete( uint32_t axis, int32_t value ) { ScriptAdd( GUCEF_WL_WL_POINTER_AXIS_DISCRETE_EVENT_OPCODE, 2, axis, (uint32_t) value ); }
void ScriptDirection( uint32_t axis, uint32_t direction ) { ScriptAdd( GUCEF_WL_WL_POINTER_AXIS_RELATIVE_DIRECTION_EVENT_OPCODE, 2, axis, direction ); }
void ScriptStop( uint32_t axis ) { ScriptAdd( GUCEF_WL_WL_POINTER_AXIS_STOP_EVENT_OPCODE, 2, 9, axis ); }
void ScriptFrame( void ) { ScriptAdd( GUCEF_WL_WL_POINTER_FRAME_EVENT_OPCODE ); }

/** What mutter sends for one notch of a wheel: source, the distance in surface units, the 120ths, the direction, frame */
void
ScriptMutterNotch( uint32_t axis, int notches, bool inverted = false )
{
    ScriptSource( GUCEF_WL_WL_POINTER_AXIS_SOURCE_WHEEL );
    ScriptAxis( axis, 15.0 * notches );
    ScriptValue120( axis, 120 * notches );
    ScriptDirection( axis, inverted ? GUCEF_WL_WL_POINTER_AXIS_RELATIVE_DIRECTION_INVERTED : GUCEF_WL_WL_POINTER_AXIS_RELATIVE_DIRECTION_IDENTICAL );
    ScriptFrame();
}

/** Every event of the pointer interface that exists at a version, in the order of the protocol, with arguments made from the signature (a surface where an object is asked for); returns how many */
int
ScriptEveryPointerEvent( UInt32 version, bool withLeave )
{
    const struct wl_interface& pointer = gucef_wl_pointer_interface;
    int sent = 0;
    for ( int event=0; event<pointer.event_count; ++event )
    {
        if ( !withLeave && GUCEF_WL_WL_POINTER_LEAVE_EVENT_OPCODE == event )
            continue;
        const char* signature = pointer.events[ event ].signature;
        UInt32 since = 0;
        while ( *signature >= '0' && *signature <= '9' )
            since = since * 10 + (UInt32) ( *signature++ - '0' );
        if ( since > version )
            continue;
        uint32_t values[ 4 ] = { 0, 0, 0, 0 };
        int count = 0;
        for ( ; '\0' != *signature && count < 4; ++signature, ++count )
        {
            switch ( *signature )
            {
                case 'o': values[ count ] = SCRIPT_SURFACE; break;
                case 'f': values[ count ] = Fx( 3.5 ); break;
                default: values[ count ] = 1; break;
            }
        }
        ScriptAdd( (uint32_t) event, count, values[ 0 ], values[ 1 ], values[ 2 ], values[ 3 ] );
        ++sent;
    }
    return sent;
}

/** A callback as libwayland calls them, that throws */
void
GuardedCallbackThatThrows( int kind )
{
    GUCEF_WL_GUARD_BEGIN
    if ( 0 == kind )
        throw std::runtime_error( "a failure in the handler" );
    if ( 1 == kind )
        throw 7;
    GUCEF_WL_GUARD_END
}

/** Reads what a child process wrote to the pipe until it ends */
CORE::CString
ReadAll( int fd )
{
    CORE::CString text;
    char buffer[ 512 ];
    ssize_t got = 0;
    while ( ( got = ::read( fd, buffer, sizeof( buffer ) - 1 ) ) > 0 )
    {
        buffer[ got ] = '\0';
        text += buffer;
    }
    return text;
}

/** Runs 'what' in a child process whose stderr is a pipe: 0 an event handler (guard) that aborts, 1 libwayland dispatching with no handler that gets a segmentation fault, 2 nothing at all that aborts */
bool
RunFatalChild( int what, CORE::CString& output, int& status )
{
    int channel[ 2 ];
    if ( 0 != ::pipe( channel ) )
        return false;
    const pid_t child = ::fork();
    if ( child < 0 )
        return false;
    if ( 0 == child )
    {
        ::close( channel[ 0 ] );
        ::dup2( channel[ 1 ], STDERR_FILENO );
        ::close( channel[ 1 ] );
        // a child that dies does not write a core file of the test process
        struct rlimit noCore;
        noCore.rlim_cur = 0;
        noCore.rlim_max = 0;
        ::setrlimit( RLIMIT_CORE, &noCore );
        CEventGuard::InstallFatalSignalReporter();
        if ( 0 == what )
        {
            CEventGuard guard( "PointerAxisRelativeDirection" );
            ::abort();
        }
        if ( 1 == what )
        {
            {
                CEventGuard handled( "Inner" );
            }
            CDispatchPhase phase;
            ::raise( SIGSEGV );
        }
        if ( 3 == what )
        {
            // a bad access of memory, outside of any handler of ours
            volatile int* nothing = GUCEF_NULL;
            *nothing = 1;
        }
        if ( 4 == what )
        {
            // the reporter taken away again: the signal then ends the process as it would without
            CEventGuard::RemoveFatalSignalReporter();
            ::abort();
        }
        ::abort();
        ::_exit( 99 );
    }
    ::close( channel[ 1 ] );
    output = ReadAll( channel[ 0 ] );
    ::close( channel[ 0 ] );
    status = 0;
    ::waitpid( child, &status, 0 );
    return true;
}

/** Asks for an activation token inside the handler of a button, as a login does that starts with a click, and writes down the order of what it saw */
class CNestedTokenAsker : public IWaylandSeatListener
{
    public:

    CWaylandActivationTokenProvider* provider;
    GUCEF::vector< CORE::CString > events;
    CORE::CString token;
    bool gotToken;

    CNestedTokenAsker( void ) : provider( GUCEF_NULL ), events(), token(), gotToken( false ) {}

    virtual void OnPointerButton( UInt32, UInt32, UInt32, UInt32 )
    {
        events.push_back( "button-begin" );
        CORE::TActivationTokenKind kind = CORE::ACTIVATIONTOKEN_STARTUP_ID;
        gotToken = provider->GetActivationToken( token, kind, 1000 ) && CORE::ACTIVATIONTOKEN_XDG_ACTIVATION == kind;
        events.push_back( "button-end" );
    }

    virtual void OnPointerMotion( UInt32, wl_fixed_t, wl_fixed_t ) { events.push_back( "motion" ); }

    bool Has( const char* text ) const
    {
        for ( UInt32 i=0; i<events.size(); ++i )
            if ( events[ i ] == text )
                return true;
        return false;
    }
};

struct SNestedDone
{
    CNestedTokenAsker* asker;
    bool operator()() const { return asker->Has( "motion" ); }
};

/** The server, a connection to it and an input context of the driver for a surface of the connection, all real (libwayland on both sides) */
struct SWireRig
{
    CORE::CString directory;
    CWaylandConnection* connection;
    CInputRecorder recorder;
    CWaylandInputContext* context;
    struct wl_surface* surface;
    bool started;

    SWireRig( void ) : directory(), connection( GUCEF_NULL ), recorder(), context( GUCEF_NULL ), surface( GUCEF_NULL ), started( false ) {}

    ~SWireRig()
    {
        delete context;
        if ( GUCEF_NULL != connection )
        {
            if ( GUCEF_NULL != surface )
                gucef_wl_surface_destroy( CWaylandRuntime::GetClientApi(), surface );
            connection->Disconnect();
            delete connection;
        }
        if ( started )
        {
            StopServer();
            CORE::DeleteDir( directory, true );
        }
    }

    bool Start( int seatGlobalVersion )
    {
        const SWaylandApi* clientApi = CWaylandRuntime::GetClientApi();
        directory = CORE::CombinePath( CORE::TempDir(), "gucef-wayland-wire-test" );
        CORE::DeleteDir( directory, true );
        if ( !CORE::CreateDirs( directory ) || !StartServer( directory, seatGlobalVersion ) )
            return false;
        started = true;
        connection = new CWaylandConnection( *clientApi, g_socketPath.C_String() );
        connection->SetPumpOnPulse( false );
        if ( !connection->Connect() )
            return false;
        surface = gucef_wl_compositor_create_surface( clientApi, connection->GetCompositor() );
        gucef_wl_surface_commit( clientApi, surface );
        if ( GUCEF_NULL == surface || !connection->Roundtrip() )
            return false;
        const UInt32 surfaceId = connection->RegisterSurface( surface );
        context = new CWaylandInputContext( *connection, surfaceId, recorder.callbacks, GUCEF_NULL, false );
        return true;
    }

    /** Lets the server send the script and pumps until all of it has been dispatched by the real library */
    bool Play( void )
    {
        PostAction( ACTION_SCRIPT );
        if ( !PumpUntil( *connection, SScriptDone() ) )
            return false;
        return connection->Roundtrip();
    }
};

}

/*-------------------------------------------------------------------------*/

void
PerformWaylandWireTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Wayland wire TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "WaylandWire" );

    const SWaylandApi* clientApi = CWaylandRuntime::GetClientApi();
    GUCEF_TESTFW_TESTCASE( "Test: The connection works with the real libwayland-client: globals, a seat with a keymap in a file descriptor, fixed point positions, pings and axis frames" )
        GUCEF_TESTFW_SKIP_IF( GUCEF_NULL == clientApi, "libwayland-client.so.0 (1.19 or newer) is not installed" );
        GUCEF_TESTFW_SKIP_IF( GUCEF_NULL == CWaylandRuntime::LoadLibraryOnce( "libwayland-server.so.0" ), "libwayland-server.so.0 is not installed" );
        try
        {
            const CORE::CString directory = CORE::CombinePath( CORE::TempDir(), "gucef-wayland-wire-test" );
            CORE::DeleteDir( directory, true );
            ASSERT_TRUE( CORE::CreateDirs( directory ) );
            ASSERT_TRUE( StartServer( directory ) );

            {
                // the socket is named by its path: nothing of the session of the person running this is looked at
                CWaylandConnection connection( *clientApi, g_socketPath.C_String() );
                connection.SetPumpOnPulse( false );
                CORE::CString problem;
                ASSERT_TRUE( connection.Connect( &problem ) );
                ASSERT_TRUE( connection.IsHealthy() );
                ASSERT_TRUE( GUCEF_NULL != connection.GetCompositor() && GUCEF_NULL != connection.GetWmBase() && GUCEF_NULL != connection.GetSeat() );
                ASSERT_TRUE( 4 == connection.GetGlobals().size() );
                ASSERT_TRUE( 3 == connection.GetSeatCapabilities() );
                ASSERT_TRUE( GUCEF_NULL != connection.GetPointer() && GUCEF_NULL != connection.GetKeyboard() );

                // the monitor of the server
                ASSERT_TRUE( 1 == connection.GetOutputCount() );
                const SWaylandOutput* output = connection.GetOutput( 0 );
                ASSERT_TRUE( GUCEF_NULL != output && output->done && 2 == output->scale );
                ASSERT_TRUE( GUCEF_NULL != output->GetCurrentMode() && 1920 == output->GetCurrentMode()->width && 144000 == output->GetCurrentMode()->refreshInMilliHertz );
                ASSERT_TRUE( "Wire" == output->make || 0 == strncmp( output->make.C_String(), "Wire", 4 ) );

                // the keymap and the repeat rate came in the roundtrips of connecting, the keymap through a file descriptor
                CWireRecorder recorder;
                connection.AddSeatListener( &recorder );
                ASSERT_TRUE( recorder.Has( "caps 3" ) && recorder.Has( "keymap 1" ) && recorder.Has( "repeat 33 250" ) );
                ASSERT_TRUE( recorder.keymapSize == strlen( GetTestKeymap() ) + 1 );
                ASSERT_TRUE( 0 == strncmp( recorder.keymapStart.C_String(), "xkb_keymap ", 11 ) );

                // a ping travels to the client and the pong comes back
                PostAction( ACTION_PING );
                ASSERT_TRUE( PumpUntil( connection, SPingDone() ) );
                ASSERT_TRUE( 4711 == g_state.pongSerial );

                // a surface of the client, then the events of a pointer on it
                struct wl_surface* surface = gucef_wl_compositor_create_surface( clientApi, connection.GetCompositor() );
                ASSERT_TRUE( GUCEF_NULL != surface );
                gucef_wl_surface_commit( clientApi, surface );
                ASSERT_TRUE( connection.Roundtrip() );
                ASSERT_TRUE( g_state.gotCommit );
                connection.RegisterSurface( surface );

                PostAction( ACTION_POINTER );
                SFrameSeen seen;
                seen.recorder = &recorder;
                ASSERT_TRUE( PumpUntil( connection, seen ) );
                ASSERT_TRUE( recorder.Has( "enter 900 2688,5184" ) );          // 10.5 and 20.25 as 24.8 fixed point
                ASSERT_TRUE( recorder.Has( "motion 5120,7680" ) );
                ASSERT_TRUE( recorder.Has( "button 901 272 1" ) );
                ASSERT_TRUE( recorder.Has( "value120 0 120" ) );
                ASSERT_TRUE( (void*) surface == (void*) connection.GetPointerFocus() );
                ASSERT_TRUE( 900 == connection.GetPointerEnterSerial() );

                // the cursor is set again by the connection at the enter, with the serial of that enter (no shape manager: a theme)
                connection.RemoveSeatListener( &recorder );
                gucef_wl_surface_destroy( clientApi, surface );
                connection.Disconnect();
                ASSERT_FALSE( connection.IsConnected() );
            }
            StopServer();
            CORE::DeleteDir( directory, true );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;
        }
        catch ( ... )
        {
            StopServer();
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The wheel of a seat at the newest version (value120, relative direction, frames) goes through the real libwayland and the input context" )
        GUCEF_TESTFW_SKIP_IF( GUCEF_NULL == clientApi, "libwayland-client.so.0 (1.19 or newer) is not installed" );
        GUCEF_TESTFW_SKIP_IF( GUCEF_NULL == CWaylandRuntime::LoadLibraryOnce( "libwayland-server.so.0" ), "libwayland-server.so.0 is not installed" );
        try
        {
            SWireRig rig;
            ASSERT_TRUE( rig.Start( 9 ) );
            ASSERT_TRUE( 9 == rig.connection->GetSeatVersion() );

            // one notch down as mutter sends it: nothing may be left over for the next
            ScriptClear();
            ScriptEnter( 100.0, 50.0 );
            ScriptMutterNotch( 0, 1 );
            ASSERT_TRUE( rig.Play() );
            ASSERT_TRUE( rig.context->IsPointerInside() );
            ASSERT_TRUE( rig.recorder.Has( "var 0 -1 -1" ) );
            ASSERT_TRUE( 1 == rig.recorder.Count( "var " ) );

            // a quick series of notches in both directions, then the other axis
            rig.recorder.Clear();
            ScriptClear();
            for ( int i=0; i<10; ++i )
                ScriptMutterNotch( 0, 1 );
            for ( int i=0; i<10; ++i )
                ScriptMutterNotch( 0, -1, true );
            ScriptMutterNotch( 1, 1 );
            ScriptMutterNotch( 1, -2 );
            ASSERT_TRUE( rig.Play() );
            ASSERT_TRUE( 22 == rig.recorder.Count( "var " ) );
            ASSERT_TRUE( rig.recorder.Has( "var 1 1 +1" ) );
            ASSERT_TRUE( rig.recorder.Has( "var 1 -1 -2" ) );
            ASSERT_TRUE( -1 == rig.context->GetWheel() );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A compositor with a newer seat than the code knows gets version 9, and every event of the pointer at that version goes through libwayland to a function" )
        GUCEF_TESTFW_SKIP_IF( GUCEF_NULL == clientApi, "libwayland-client.so.0 (1.19 or newer) is not installed" );
        GUCEF_TESTFW_SKIP_IF( GUCEF_NULL == CWaylandRuntime::LoadLibraryOnce( "libwayland-server.so.0" ), "libwayland-server.so.0 is not installed" );
        try
        {
            CLogCapture errors( CORE::LOG_ERROR );
            CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &errors );
            CEventGuard::ResetExceptionLog();
            {
                SWireRig rig;
                ASSERT_TRUE( rig.Start( 11 ) );
                ASSERT_TRUE( 9 == rig.connection->GetSeatVersion() );
                ASSERT_TRUE( 9 == clientApi->proxy_get_version( (struct wl_proxy*) rig.connection->GetPointer() ) );

                // every event of the interface that exists at the version of the pointer, in the order of the protocol (the pointer enters and leaves at once)
                ScriptClear();
                ASSERT_TRUE( 11 == ScriptEveryPointerEvent( 9, true ) );
                ASSERT_TRUE( rig.Play() );
                ASSERT_TRUE( rig.connection->IsHealthy() );
                ASSERT_FALSE( rig.context->IsPointerInside() );

                // and again with the pointer staying in the window
                ScriptClear();
                ASSERT_TRUE( 10 == ScriptEveryPointerEvent( 9, false ) );
                ASSERT_TRUE( rig.Play() );
                ASSERT_TRUE( rig.connection->IsHealthy() );
                ASSERT_TRUE( 1 == rig.context->GetAxisAccumulator().GetRelativeDirection( 1 ) );
            }
            CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( &errors );
            ASSERT_FALSE( errors.Has( "has no function for every event" ) );
            ASSERT_TRUE( 0 == CEventGuard::GetExceptionCount() );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Touch pad scrolling and wheels of compositors of every seat version go through libwayland to the input context" )
        GUCEF_TESTFW_SKIP_IF( GUCEF_NULL == clientApi, "libwayland-client.so.0 (1.19 or newer) is not installed" );
        GUCEF_TESTFW_SKIP_IF( GUCEF_NULL == CWaylandRuntime::LoadLibraryOnce( "libwayland-server.so.0" ), "libwayland-server.so.0 is not installed" );
        try
        {
            // a touch pad on a version 9 seat: fingers, the distance in many frames, the direction with each, the stop (and frame) at the end, in both axes
            {
                SWireRig rig;
                ASSERT_TRUE( rig.Start( 9 ) );
                ScriptClear();
                ScriptEnter( 40.0, 30.0 );
                for ( int i=0; i<20; ++i )
                {
                    ScriptSource( GUCEF_WL_WL_POINTER_AXIS_SOURCE_FINGER );
                    ScriptAxis( 0, 3.75 );
                    ScriptAxis( 1, -1.25 );
                    ScriptDirection( 0, GUCEF_WL_WL_POINTER_AXIS_RELATIVE_DIRECTION_INVERTED );
                    ScriptFrame();
                }
                ScriptSource( GUCEF_WL_WL_POINTER_AXIS_SOURCE_FINGER );
                ScriptStop( 0 );
                ScriptStop( 1 );
                ScriptFrame();
                ASSERT_TRUE( rig.Play() );
                ASSERT_TRUE( -5 == rig.context->GetWheel() );                       // 75 units down: five notches
                ASSERT_TRUE( -1 == rig.context->GetHorizontalWheel() );             // 25 units to the left: one notch, and the rest is dropped at the stop
            }

            // continuous source, frames, and the end of the scroll
            {
                SWireRig rig;
                ASSERT_TRUE( rig.Start( 9 ) );
                ScriptClear();
                ScriptEnter( 40.0, 30.0 );
                ScriptSource( GUCEF_WL_WL_POINTER_AXIS_SOURCE_CONTINUOUS );
                ScriptAxis( 0, -45.0 );
                ScriptFrame();
                ScriptStop( 0 );
                ScriptFrame();
                ASSERT_TRUE( rig.Play() );
                ASSERT_TRUE( 3 == rig.context->GetWheel() );
            }

            // compositors with older seats: discrete steps (version 5 to 7), and no frames at all before version 5
            const int versions[] = { 7, 5, 4, 3 };
            for ( size_t v=0; v<sizeof( versions ) / sizeof( versions[ 0 ] ); ++v )
            {
                SWireRig rig;
                ASSERT_TRUE( rig.Start( versions[ v ] ) );
                ASSERT_TRUE( (UInt32) versions[ v ] == rig.connection->GetSeatVersion() );
                ScriptClear();
                ScriptEnter( 40.0, 30.0 );
                if ( versions[ v ] >= 5 )
                {
                    ScriptSource( GUCEF_WL_WL_POINTER_AXIS_SOURCE_WHEEL );
                    ScriptAxis( 0, 15.0 );
                    ScriptDiscrete( 0, 1 );
                    ScriptFrame();
                }
                else
                    ScriptAxis( 0, 15.0 );
                ASSERT_TRUE( rig.Play() );
                ASSERT_TRUE( 1 == rig.recorder.Count( "var" ) && rig.recorder.Has( "var 0 -1 -1" ) );
            }
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An activation token is made with the real libwayland: the request carries the serial of the last button, the answer comes on a queue of its own, also while the click is still being dispatched" )
        GUCEF_TESTFW_SKIP_IF( GUCEF_NULL == clientApi, "libwayland-client.so.0 (1.19 or newer) is not installed" );
        GUCEF_TESTFW_SKIP_IF( GUCEF_NULL == CWaylandRuntime::LoadLibraryOnce( "libwayland-server.so.0" ), "libwayland-server.so.0 is not installed" );
        try
        {
            memset( &g_activation, 0, sizeof( g_activation ) );
            g_activation.offered = true;
            g_activation.answers = true;
            {
                SWireRig rig;
                ASSERT_TRUE( rig.Start( 9 ) );
                ASSERT_TRUE( GUCEF_NULL != rig.connection->GetActivationManager() );
                const UInt32 surfaceId = rig.connection->RegisterSurface( rig.surface );
                CWaylandActivationTokenProvider provider( *rig.connection, surfaceId, "redux" );
                ASSERT_TRUE( provider.IsSupported() );

                // a click of the user: the serial of the button is what the token is made from, not the one of the enter before it
                ScriptClear();
                ScriptEnter( 10.0, 10.0 );
                ScriptAdd( GUCEF_WL_WL_POINTER_BUTTON_EVENT_OPCODE, 4, 901, 5, 0x110, 1 );
                ScriptFrame();
                ASSERT_TRUE( rig.Play() );
                ASSERT_TRUE( 901 == rig.connection->GetLastActivationSerial() );

                CORE::CString token;
                CORE::TActivationTokenKind kind = CORE::ACTIVATIONTOKEN_STARTUP_ID;
                ASSERT_TRUE( provider.GetActivationToken( token, kind, 1000 ) );
                ASSERT_TRUE( "wire-activation-token" == token && CORE::ACTIVATIONTOKEN_XDG_ACTIVATION == kind );
                ASSERT_TRUE( 1 == g_activation.commits && g_activation.gotSerial && 901 == g_activation.serial );
                ASSERT_TRUE( 0 == strcmp( g_activation.appId, "redux" ) && g_activation.surfaceIsOurs );

                // asked from within the handler of a button: the motion that follows it on the wire is not dispatched before the handler is done
                CNestedTokenAsker asker;
                asker.provider = &provider;
                rig.connection->AddSeatListener( &asker );
                ScriptClear();
                ScriptAdd( GUCEF_WL_WL_POINTER_BUTTON_EVENT_OPCODE, 4, 902, 6, 0x110, 0 );
                ScriptAdd( GUCEF_WL_WL_POINTER_MOTION_EVENT_OPCODE, 3, 7, Fx( 20.0 ), Fx( 30.0 ) );
                ScriptFrame();
                PostAction( ACTION_SCRIPT );
                SNestedDone nestedDone;
                nestedDone.asker = &asker;
                ASSERT_TRUE( PumpUntil( *rig.connection, nestedDone ) );
                rig.connection->RemoveSeatListener( &asker );
                ASSERT_TRUE( asker.gotToken && "wire-activation-token" == asker.token );
                ASSERT_TRUE( 3 == asker.events.size() && "button-begin" == asker.events[ 0 ] && "button-end" == asker.events[ 1 ] && "motion" == asker.events[ 2 ] );
                ASSERT_TRUE( 2 == g_activation.commits && 902 == g_activation.serial );

                // a compositor that never answers: the provider gives up at its time and the connection goes on working
                g_activation.answers = false;
                const UInt64 before = MonotonicTimeInMs();
                ASSERT_FALSE( provider.GetActivationToken( token, kind, 60 ) );
                const UInt64 waited = MonotonicTimeInMs() - before;
                ASSERT_TRUE( token.IsNULLOrEmpty() && waited >= 50 && waited < 1500 );
                PostAction( ACTION_PING );
                g_state.gotPong = false;
                ASSERT_TRUE( PumpUntil( *rig.connection, SPingDone() ) );
                ASSERT_TRUE( rig.connection->IsHealthy() );
            }
            memset( &g_activation, 0, sizeof( g_activation ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;
        }
        catch ( ... )
        {
            memset( &g_activation, 0, sizeof( g_activation ) );
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An exception in an event handler stops at the boundary of libwayland and is logged once for each handler" )
        try
        {
            CLogCapture errors( CORE::LOG_ERROR );
            CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &errors );
            CEventGuard::ResetExceptionLog();
            ASSERT_TRUE( GUCEF_NULL == CEventGuard::GetCurrentEvent() );
            GuardedCallbackThatThrows( 0 );
            GuardedCallbackThatThrows( 0 );
            GuardedCallbackThatThrows( 1 );
            GuardedCallbackThatThrows( 2 );
            CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( &errors );
            ASSERT_TRUE( 3 == CEventGuard::GetExceptionCount() );
            ASSERT_TRUE( 1 == errors.Count( "a failure in the handler" ) && 2 >= errors.Count( "GuardedCallbackThatThrows" ) );
            ASSERT_TRUE( GUCEF_NULL == CEventGuard::GetCurrentEvent() );
            ASSERT_TRUE( 0 == strcmp( "GuardedCallbackThatThrows", CEventGuard::GetLastEvent() ) );
            CEventGuard::ResetExceptionLog();
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A fatal signal names the event handler that was running on stderr and then ends the process as it would have" )
        try
        {
            // the line itself
            char line[ 400 ];
            {
                CEventGuard guard( "PointerFrame" );
                const UInt32 length = CEventGuard::FormatFatalSignalMessage( 6, line, sizeof( line ) );
                ASSERT_TRUE( length == strlen( line ) );
                ASSERT_TRUE( 0 == strcmp( line, "Wayland driver: fatal signal 6 in PointerFrame\n" ) );
                // too small a buffer cuts the text but ends it
                char tiny[ 12 ];
                ASSERT_TRUE( 11 == CEventGuard::FormatFatalSignalMessage( 11, tiny, sizeof( tiny ) ) );
                ASSERT_TRUE( 0 == strncmp( tiny, "Wayland dri", 11 ) );
            }
            {
                CDispatchPhase phase;
                ASSERT_TRUE( CEventGuard::IsDispatching() );
                CEventGuard::FormatFatalSignalMessage( 11, line, sizeof( line ) );
                ASSERT_TRUE( GUCEF_NULL != strstr( line, "fatal signal 11 while libwayland dispatched events, outside of a handler of ours" ) );
                ASSERT_TRUE( GUCEF_NULL != strstr( line, "last event handled: " ) );
            }
            ASSERT_FALSE( CEventGuard::IsDispatching() );

            // a process that aborts inside an event handler says so, and still dies of the signal
            CORE::CString output;
            int status = 0;
            ASSERT_TRUE( RunFatalChild( 0, output, status ) );
            ASSERT_TRUE( output.HasSubstr( "Wayland driver: fatal signal 6 in PointerAxisRelativeDirection" ) >= 0 );
            ASSERT_TRUE( WIFSIGNALED( status ) && SIGABRT == WTERMSIG( status ) );

            // libwayland dispatching and no handler of ours: the line says the failing event is the one after the last handled
            ASSERT_TRUE( RunFatalChild( 1, output, status ) );
            ASSERT_TRUE( output.HasSubstr( "Wayland driver: fatal signal 11 while libwayland dispatched events" ) >= 0 );
            ASSERT_TRUE( output.HasSubstr( "last event handled: Inner" ) >= 0 );
            ASSERT_TRUE( WIFSIGNALED( status ) || WIFEXITED( status ) );

            ASSERT_TRUE( RunFatalChild( 2, output, status ) );
            ASSERT_TRUE( output.HasSubstr( "Wayland driver: fatal signal 6 outside of the handling of Wayland events" ) >= 0 );
            ASSERT_TRUE( WIFSIGNALED( status ) && SIGABRT == WTERMSIG( status ) );

            // the report says where: the address that was accessed and the functions that were running (the signal is for the whole process, whoever caused it)
            ASSERT_TRUE( RunFatalChild( 3, output, status ) );
            ASSERT_TRUE( output.HasSubstr( "fatal signal 11 outside of the handling of Wayland events" ) >= 0 );
            ASSERT_TRUE( output.HasSubstr( "Address accessed: 0x0" ) >= 0 );
#if defined( __GLIBC__ )
            ASSERT_TRUE( output.HasSubstr( "RunFatalChild" ) >= 0 || output.HasSubstr( "gucefGUI_WaylandTestApp" ) >= 0 );
#endif
            ASSERT_TRUE( WIFSIGNALED( status ) || WIFEXITED( status ) );       // a sanitizer ends such a process by itself

            // the handler is taken away on request, and then the signal says nothing of ours
            ASSERT_TRUE( RunFatalChild( 4, output, status ) );
            ASSERT_TRUE( output.HasSubstr( "Wayland driver" ) < 0 );
            ASSERT_TRUE( WIFSIGNALED( status ) && SIGABRT == WTERMSIG( status ) );

            // the reporter can be installed and removed, and a handler that somebody else put on top of it is left alone
            const bool wasInstalled = CEventGuard::IsFatalSignalReporterInstalled();
            ASSERT_TRUE( CEventGuard::InstallFatalSignalReporter() && CEventGuard::IsFatalSignalReporterInstalled() );
            ASSERT_TRUE( CEventGuard::RemoveFatalSignalReporter() && !CEventGuard::IsFatalSignalReporterInstalled() );
            ASSERT_FALSE( CEventGuard::RemoveFatalSignalReporter() );
            if ( wasInstalled )
                CEventGuard::InstallFatalSignalReporter();
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: No listener that the code gave to a compositor, fake or real, lacked a function for an event of its version" )
        try
        {
            // the fakes of all the tests before this one are gone: each told, when it went, whether it had rejected a listener it was not meant to
            ASSERT_TRUE( 0 == CFakeWayland::g_unexpectedRejectedListeners );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A connection to a socket nobody listens on fails with a reason" )
        GUCEF_TESTFW_SKIP_IF( GUCEF_NULL == clientApi, "libwayland-client.so.0 (1.19 or newer) is not installed" );
        try
        {
            const CORE::CString nothing = CORE::CombinePath( CORE::TempDir(), "gucef-wayland-no-such-socket" );
            CWaylandConnection connection( *clientApi, nothing.C_String() );
            connection.SetPumpOnPulse( false );
            CORE::CString problem;
            ASSERT_FALSE( connection.Connect( &problem ) );
            ASSERT_FALSE( problem.IsNULLOrEmpty() );
            ASSERT_FALSE( connection.IsConnected() );
        }
        catch ( const TESTFW::STestSkipRequest& )
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
