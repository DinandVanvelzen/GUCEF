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
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "TestFakeWayland.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace TESTWAYLAND {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CFakeWayland* CFakeWayland::g_current = GUCEF_NULL;

namespace {

wl_fixed_t
ToFixed( double value )
{
    return (wl_fixed_t) ( value * 256.0 + ( value >= 0.0 ? 0.5 : -0.5 ) );
}

bool
IsInterface( const SFakeProxy* proxy, const char* name )
{
    return GUCEF_NULL != proxy && GUCEF_NULL != proxy->iface && 0 == strcmp( proxy->iface->name, name );
}

// the function table: every function goes to the fake that is current

struct wl_display* F_connect( const char* ) { return CFakeWayland::g_current->Connect(); }
void F_disconnect( struct wl_display* ) { CFakeWayland::g_current->Disconnect(); }
int F_getFd( struct wl_display* ) { return CFakeWayland::g_current->GetFd(); }
int
F_flush( struct wl_display* )
{
    ++CFakeWayland::g_current->flushCalls;
    if ( CFakeWayland::g_current->flushFails )
    {
        errno = EPIPE;
        return -1;
    }
    return 0;
}

int F_dispatchPending( struct wl_display* ) { return CFakeWayland::g_current->DispatchPending(); }
int F_prepareRead( struct wl_display* ) { return CFakeWayland::g_current->PrepareRead(); }
int F_readEvents( struct wl_display* ) { return CFakeWayland::g_current->ReadEvents(); }
void F_cancelRead( struct wl_display* ) { ++CFakeWayland::g_current->cancelReadCalls; }
int F_roundtrip( struct wl_display* ) { return CFakeWayland::g_current->Roundtrip(); }
int F_getError( struct wl_display* ) { return CFakeWayland::g_current->errorCode; }

struct wl_proxy*
F_marshal( struct wl_proxy* proxy, uint32_t opcode, const struct wl_interface* iface, uint32_t version, uint32_t flags, ... )
{
    va_list args;
    va_start( args, flags );
    struct wl_proxy* result = CFakeWayland::g_current->Marshal( proxy, opcode, iface, version, flags, args );
    va_end( args );
    return result;
}

int
F_addListener( struct wl_proxy* proxy, void ( **implementation )( void ), void* data )
{
    SFakeProxy* fake = (SFakeProxy*) proxy;
    fake->listener = implementation;
    fake->data = data;
    return 0;
}

void
F_listenerRejected( const char* interfaceName )
{
    ++CFakeWayland::g_current->rejectedListenerCount;
    CFakeWayland::g_current->lastRejectedListener = GUCEF_NULL != interfaceName ? interfaceName : "";
}

void
F_destroy( struct wl_proxy* proxy )
{
    SFakeProxy* fake = (SFakeProxy*) proxy;
    fake->destroyed = true;
    CORE::CString text = "proxy_destroy(";
    text += GUCEF_NULL != fake->iface ? fake->iface->name : "wl_display";
    text += ")";
    CFakeWayland::g_current->requests.push_back( text );
}

struct wl_event_queue*
F_createQueue( struct wl_display* )
{
    ++CFakeWayland::g_current->queuesCreated;
    return (struct wl_event_queue*) new int( 7 );
}

void
F_destroyQueue( struct wl_event_queue* queue )
{
    ++CFakeWayland::g_current->queuesDestroyed;
    delete (int*) queue;
}

void F_setQueue( struct wl_proxy* proxy, struct wl_event_queue* queue ) { ( (SFakeProxy*) proxy )->queue = queue; }
int F_dispatchQueuePending( struct wl_display*, struct wl_event_queue* queue ) { return CFakeWayland::g_current->DispatchQueuePending( queue ); }
int F_prepareReadQueue( struct wl_display*, struct wl_event_queue* queue ) { return CFakeWayland::g_current->PrepareReadQueue( queue ); }

uint32_t F_getVersion( struct wl_proxy* proxy ) { return ( (SFakeProxy*) proxy )->version; }
void F_setUserData( struct wl_proxy* proxy, void* data ) { ( (SFakeProxy*) proxy )->data = data; }
void* F_getUserData( struct wl_proxy* proxy ) { return ( (SFakeProxy*) proxy )->data; }
uint32_t F_getId( struct wl_proxy* proxy ) { return ( (SFakeProxy*) proxy )->id; }

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

UInt32 CFakeWayland::g_unexpectedRejectedListeners = 0;

/*-------------------------------------------------------------------------*/

CFakeWayland::CFakeWayland( void )
    : seatCapabilities( GUCEF_WL_WL_SEAT_CAPABILITY_POINTER | GUCEF_WL_WL_SEAT_CAPABILITY_KEYBOARD )
    , repeatRate( 25 )
    , repeatDelay( 400 )
    , preferredFractionalScale( 0 )
    , decorationReply( GUCEF_WL_ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE )
    , firstConfigureWidth( 0 )
    , firstConfigureHeight( 0 )
    , firstConfigureStateCount( 0 )
    , autoConfigure( true )
    , output()
    , moreOutputs()
    , keymapText( GUCEF_NULL )
    , activationToken( "fake-activation-token-0001" )
    , activationAnswers( true )
    , activationFromSocket( false )
    , requests()
    , prepareReadCalls( 0 )
    , readEventsCalls( 0 )
    , cancelReadCalls( 0 )
    , flushCalls( 0 )
    , rejectedListenerCount( 0 )
    , lastRejectedListener()
    , rejectionsExpected( false )
    , roundtripCalls( 0 )
    , queuesCreated( 0 )
    , queuesDestroyed( 0 )
    , disconnectCalls( 0 )
    , connectCalls( 0 )
    , flushFails( false )
    , connectFails( false )
    , errorCode( 0 )
    , m_previous( g_current )
    , m_proxies()
    , m_queue()
    , m_socketQueue()
    , m_display()
    , m_registry( GUCEF_NULL )
    , m_globals()
    , m_nextGlobalName( 1 )
    , m_nextId( 2 )
    , m_nextSerial( 100 )
    , m_outputsBound( 0 )
    , m_connected( false )
{GUCEF_TRACE;

    memset( &m_api, 0, sizeof( m_api ) );
    memset( firstConfigureStates, 0, sizeof( firstConfigureStates ) );
    m_pipe[ 0 ] = -1;
    m_pipe[ 1 ] = -1;
    m_api.display_connect = F_connect;
    m_api.display_disconnect = F_disconnect;
    m_api.display_get_fd = F_getFd;
    m_api.display_flush = F_flush;
    m_api.display_dispatch_pending = F_dispatchPending;
    m_api.display_prepare_read = F_prepareRead;
    m_api.display_read_events = F_readEvents;
    m_api.display_cancel_read = F_cancelRead;
    m_api.display_roundtrip = F_roundtrip;
    m_api.display_get_error = F_getError;
    m_api.proxy_marshal_flags = F_marshal;
    m_api.proxy_add_listener = F_addListener;
    m_api.proxy_destroy = F_destroy;
    m_api.proxy_get_version = F_getVersion;
    m_api.proxy_set_user_data = F_setUserData;
    m_api.proxy_get_user_data = F_getUserData;
    m_api.proxy_get_id = F_getId;
    m_api.display_create_queue = F_createQueue;
    m_api.event_queue_destroy = F_destroyQueue;
    m_api.proxy_set_queue = F_setQueue;
    m_api.display_dispatch_queue_pending = F_dispatchQueuePending;
    m_api.display_prepare_read_queue = F_prepareReadQueue;
    m_api.listener_rejected = F_listenerRejected;
    m_display.id = 1;
    g_current = this;
}

/*-------------------------------------------------------------------------*/

void
CFakeWayland::RemoveQueueFunctions( void )
{GUCEF_TRACE;

    m_api.display_create_queue = GUCEF_NULL;
    m_api.event_queue_destroy = GUCEF_NULL;
    m_api.proxy_set_queue = GUCEF_NULL;
    m_api.display_dispatch_queue_pending = GUCEF_NULL;
    m_api.display_prepare_read_queue = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CFakeWayland::~CFakeWayland()
{GUCEF_TRACE;

    if ( rejectedListenerCount > 0 && !rejectionsExpected )
        ++g_unexpectedRejectedListeners;

    for ( UInt32 i=0; i<m_proxies.size(); ++i )
        delete m_proxies[ i ];
    if ( m_pipe[ 0 ] >= 0 ) ::close( m_pipe[ 0 ] );
    if ( m_pipe[ 1 ] >= 0 ) ::close( m_pipe[ 1 ] );
    if ( g_current == this )
        g_current = m_previous;
}

/*-------------------------------------------------------------------------*/

const SWaylandApi&
CFakeWayland::GetApi( void ) const
{GUCEF_TRACE;

    return m_api;
}

/*-------------------------------------------------------------------------*/

void
CFakeWayland::OfferGlobal( const char* interfaceName, UInt32 version )
{GUCEF_TRACE;

    SGlobal global;
    global.name = m_nextGlobalName++;
    global.interfaceName = interfaceName;
    global.version = version;
    global.announced = false;
    m_globals.push_back( global );
}

/*-------------------------------------------------------------------------*/

void
CFakeWayland::OfferDefaultGlobals( void )
{GUCEF_TRACE;

    OfferGlobal( "wl_compositor", 6 );
    OfferGlobal( "xdg_wm_base", 6 );
    OfferGlobal( "wl_shm", 1 );
    OfferGlobal( "wl_seat", 9 );
    OfferGlobal( "wl_output", 4 );
}

/*-------------------------------------------------------------------------*/

void
CFakeWayland::OfferGlobalLater( const char* interfaceName, UInt32 version )
{GUCEF_TRACE;

    OfferGlobal( interfaceName, version );
}

/*-------------------------------------------------------------------------*/

void
CFakeWayland::WithdrawGlobal( const char* interfaceName )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_globals.size(); ++i )
    {
        if ( m_globals[ i ].interfaceName == interfaceName )
        {
            const UInt32 name = m_globals[ i ].name;
            m_globals.erase( m_globals.begin() + i );
            if ( GUCEF_NULL != m_registry && GUCEF_NULL != m_registry->listener )
            {
                const struct gucef_wl_registry_listener* l = (const struct gucef_wl_registry_listener*) m_registry->listener;
                l->global_remove( m_registry->data, (struct wl_registry*) m_registry, name );
            }
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CFakeWayland::Requested( const char* text ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<requests.size(); ++i )
        if ( requests[ i ] == text )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CFakeWayland::CountRequestsWithPrefix( const char* prefix ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    const size_t length = strlen( prefix );
    for ( UInt32 i=0; i<requests.size(); ++i )
        if ( 0 == strncmp( requests[ i ].C_String(), prefix, length ) )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeWayland::LastRequestWithPrefix( const char* prefix ) const
{GUCEF_TRACE;

    const size_t length = strlen( prefix );
    for ( UInt32 i=(UInt32) requests.size(); i>0; --i )
        if ( 0 == strncmp( requests[ i-1 ].C_String(), prefix, length ) )
            return requests[ i-1 ];
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
CFakeWayland::ClearRequests( void )
{GUCEF_TRACE;

    requests.clear();
}

/*-------------------------------------------------------------------------*/

SFakeProxy*
CFakeWayland::NewProxy( const struct wl_interface* iface, UInt32 version )
{GUCEF_TRACE;

    SFakeProxy* proxy = new SFakeProxy();
    proxy->iface = iface;
    proxy->version = version;
    proxy->id = m_nextId++;
    m_proxies.push_back( proxy );
    return proxy;
}

/*-------------------------------------------------------------------------*/

SFakeProxy*
CFakeWayland::FindProxy( const char* interfaceName, UInt32 index ) const
{GUCEF_TRACE;

    UInt32 seen = 0;
    for ( UInt32 i=0; i<m_proxies.size(); ++i )
    {
        if ( !m_proxies[ i ]->destroyed && IsInterface( m_proxies[ i ], interfaceName ) )
        {
            if ( seen == index )
                return m_proxies[ i ];
            ++seen;
        }
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt32
CFakeWayland::CountLiveProxies( const char* interfaceName ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( UInt32 i=0; i<m_proxies.size(); ++i )
        if ( !m_proxies[ i ]->destroyed && IsInterface( m_proxies[ i ], interfaceName ) )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

template< typename L >
L*
CFakeWayland::ListenerOf( const char* interfaceName, UInt32 index ) const
{
    SFakeProxy* proxy = FindProxy( interfaceName, index );
    return GUCEF_NULL != proxy ? (L*) proxy->listener : GUCEF_NULL;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE FUNCTIONS OF THE TABLE                                         //
//                                                                         //
//-------------------------------------------------------------------------*/

struct wl_display*
CFakeWayland::Connect( void )
{GUCEF_TRACE;

    ++connectCalls;
    if ( connectFails )
        return GUCEF_NULL;
    if ( m_pipe[ 0 ] < 0 )
        if ( 0 != ::pipe2( m_pipe, O_NONBLOCK | O_CLOEXEC ) )
            return GUCEF_NULL;
    m_connected = true;
    return (struct wl_display*) &m_display;
}

/*-------------------------------------------------------------------------*/

void
CFakeWayland::Disconnect( void )
{GUCEF_TRACE;

    ++disconnectCalls;
    m_connected = false;
}

/*-------------------------------------------------------------------------*/

int
CFakeWayland::GetFd( void ) const
{GUCEF_TRACE;

    return m_pipe[ 0 ];
}

/*-------------------------------------------------------------------------*/

void
CFakeWayland::Post( TFakeThunk thunk, void* argument )
{GUCEF_TRACE;

    SQueued queued;
    queued.thunk = thunk;
    queued.argument = argument;
    queued.queue = GUCEF_NULL;
    m_queue.push_back( queued );
}

/*-------------------------------------------------------------------------*/

void
CFakeWayland::PostToQueue( TFakeThunk thunk, void* argument, void* queue, bool fromSocket )
{GUCEF_TRACE;

    SQueued queued;
    queued.thunk = thunk;
    queued.argument = argument;
    queued.queue = queue;
    if ( !fromSocket )
    {
        m_queue.push_back( queued );
        return;
    }
    m_socketQueue.push_back( queued );
    const char byte = 1;
    if ( m_pipe[ 1 ] >= 0 )
        (void) ::write( m_pipe[ 1 ], &byte, 1 );
}

/*-------------------------------------------------------------------------*/

void
CFakeWayland::PostFromSocket( TFakeThunk thunk, void* argument )
{GUCEF_TRACE;

    SQueued queued;
    queued.thunk = thunk;
    queued.argument = argument;
    queued.queue = GUCEF_NULL;
    m_socketQueue.push_back( queued );
    const char byte = 1;
    if ( m_pipe[ 1 ] >= 0 )
        (void) ::write( m_pipe[ 1 ], &byte, 1 );
}

/*-------------------------------------------------------------------------*/

void
CFakeWayland::DrainQueue( void )
{GUCEF_TRACE;

    DrainQueueOf( GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

void
CFakeWayland::DrainQueueOf( void* queue )
{GUCEF_TRACE;

    // what a handler adds to the queue is dispatched in the same call, as libwayland does
    while ( true )
    {
        UInt32 found = (UInt32) m_queue.size();
        for ( UInt32 i=0; i<m_queue.size(); ++i )
        {
            if ( m_queue[ i ].queue == queue )
            {
                found = i;
                break;
            }
        }
        if ( found == m_queue.size() )
            break;
        SQueued queued = m_queue[ found ];
        m_queue.erase( m_queue.begin() + found );
        queued.thunk( queued.argument );
    }
}

/*-------------------------------------------------------------------------*/

bool
CFakeWayland::HasQueued( void* queue ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_queue.size(); ++i )
        if ( m_queue[ i ].queue == queue )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

int
CFakeWayland::DispatchQueuePending( void* queue )
{GUCEF_TRACE;

    DrainQueueOf( queue );
    return 0;
}

/*-------------------------------------------------------------------------*/

int
CFakeWayland::PrepareReadQueue( void* queue )
{GUCEF_TRACE;

    if ( HasQueued( queue ) )
    {
        errno = EAGAIN;
        return -1;
    }
    ++prepareReadCalls;
    return 0;
}

/*-------------------------------------------------------------------------*/

void
CFakeWayland::DeliverGlobals( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_registry || GUCEF_NULL == m_registry->listener )
        return;
    const struct gucef_wl_registry_listener* l = (const struct gucef_wl_registry_listener*) m_registry->listener;
    for ( UInt32 i=0; i<m_globals.size(); ++i )
    {
        if ( m_globals[ i ].announced )
            continue;
        m_globals[ i ].announced = true;
        l->global( m_registry->data, (struct wl_registry*) m_registry, m_globals[ i ].name, m_globals[ i ].interfaceName.C_String(), m_globals[ i ].version );
    }
}

/*-------------------------------------------------------------------------*/

int
CFakeWayland::Roundtrip( void )
{GUCEF_TRACE;

    ++roundtripCalls;
    DeliverGlobals();
    DrainQueue();
    // globals announced by what the events did
    DeliverGlobals();
    DrainQueue();
    return 0;
}

/*-------------------------------------------------------------------------*/

int
CFakeWayland::DispatchPending( void )
{GUCEF_TRACE;

    DeliverGlobals();
    DrainQueue();
    return 0;
}

/*-------------------------------------------------------------------------*/

int
CFakeWayland::PrepareRead( void )
{GUCEF_TRACE;

    ++prepareReadCalls;
    if ( HasQueued( GUCEF_NULL ) )
    {
        --prepareReadCalls;
        errno = EAGAIN;
        return -1;
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

int
CFakeWayland::ReadEvents( void )
{GUCEF_TRACE;

    ++readEventsCalls;
    char buffer[ 64 ];
    while ( ::read( m_pipe[ 0 ], buffer, sizeof( buffer ) ) > 0 ) {}
    for ( UInt32 i=0; i<m_socketQueue.size(); ++i )
        m_queue.push_back( m_socketQueue[ i ] );
    m_socketQueue.clear();
    return 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE COMPOSITOR'S ANSWERS                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

void
ThunkSeatInit( void* argument )
{
    SFakeProxy* seat = (SFakeProxy*) argument;
    const struct gucef_wl_seat_listener* l = (const struct gucef_wl_seat_listener*) seat->listener;
    if ( GUCEF_NULL != l )
        l->capabilities( seat->data, (struct wl_seat*) seat, CFakeWayland::g_current->seatCapabilities );
}

void
ThunkOutputInit( void* argument )
{
    SFakeProxy* proxy = (SFakeProxy*) argument;
    CFakeWayland* fake = CFakeWayland::g_current;
    const struct gucef_wl_output_listener* l = (const struct gucef_wl_output_listener*) proxy->listener;
    if ( GUCEF_NULL == l )
        return;
    const SFakeOutputSpec& spec = proxy->outputIndex > 0 && proxy->outputIndex <= fake->moreOutputs.size() ? fake->moreOutputs[ proxy->outputIndex - 1 ] : fake->output;
    l->geometry( proxy->data, (struct wl_output*) proxy, spec.x, spec.y, 1000, 300, 0, "Fake", "Monitor", 0 );
    l->mode( proxy->data, (struct wl_output*) proxy, GUCEF_WL_WL_OUTPUT_MODE_CURRENT | GUCEF_WL_WL_OUTPUT_MODE_PREFERRED, spec.width, spec.height, spec.refreshInMilliHertz );
    l->scale( proxy->data, (struct wl_output*) proxy, spec.scale );
    if ( proxy->version >= 4 )
        l->name( proxy->data, (struct wl_output*) proxy, spec.name );
    l->done( proxy->data, (struct wl_output*) proxy );
}

void
ThunkXdgOutputInit( void* argument )
{
    SFakeProxy* proxy = (SFakeProxy*) argument;
    CFakeWayland* fake = CFakeWayland::g_current;
    const struct gucef_zxdg_output_v1_listener* l = (const struct gucef_zxdg_output_v1_listener*) proxy->listener;
    if ( GUCEF_NULL == l || GUCEF_NULL == proxy->related )
        return;
    const SFakeProxy* outputProxy = proxy->related;
    const SFakeOutputSpec& spec = outputProxy->outputIndex > 0 && outputProxy->outputIndex <= fake->moreOutputs.size() ? fake->moreOutputs[ outputProxy->outputIndex - 1 ] : fake->output;
    if ( 0 == spec.logicalWidth )
        return;
    l->logical_position( proxy->data, (struct zxdg_output_v1*) proxy, spec.x, spec.y );
    l->logical_size( proxy->data, (struct zxdg_output_v1*) proxy, spec.logicalWidth, spec.logicalHeight );
}

void
ThunkKeyboardInit( void* argument )
{
    CFakeWayland* fake = CFakeWayland::g_current;
    if ( GUCEF_NULL != fake->keymapText )
        fake->SendKeyboardKeymap( fake->keymapText );
    if ( ( (SFakeProxy*) argument )->version >= 4 )
        fake->SendKeyboardRepeatInfo( fake->repeatRate, fake->repeatDelay );
}

void
ThunkFractionalInit( void* argument )
{
    SFakeProxy* proxy = (SFakeProxy*) argument;
    const struct gucef_wp_fractional_scale_v1_listener* l = (const struct gucef_wp_fractional_scale_v1_listener*) proxy->listener;
    if ( GUCEF_NULL != l && 0 != CFakeWayland::g_current->preferredFractionalScale )
        l->preferred_scale( proxy->data, (struct wp_fractional_scale_v1*) proxy, CFakeWayland::g_current->preferredFractionalScale );
}

void
ThunkDecorationInit( void* argument )
{
    SFakeProxy* proxy = (SFakeProxy*) argument;
    const struct gucef_zxdg_toplevel_decoration_v1_listener* l = (const struct gucef_zxdg_toplevel_decoration_v1_listener*) proxy->listener;
    if ( GUCEF_NULL != l )
        l->configure( proxy->data, (struct zxdg_toplevel_decoration_v1*) proxy, CFakeWayland::g_current->decorationReply );
}

void
ThunkActivationDone( void* argument )
{
    SFakeProxy* proxy = (SFakeProxy*) argument;
    // a proxy that was destroyed gets no events: libwayland drops them
    if ( proxy->destroyed )
        return;
    const struct gucef_xdg_activation_token_v1_listener* l = (const struct gucef_xdg_activation_token_v1_listener*) proxy->listener;
    if ( GUCEF_NULL != l )
        l->done( proxy->data, (struct xdg_activation_token_v1*) proxy, CFakeWayland::g_current->activationToken.C_String() );
}

void
ThunkFirstConfigure( void* )
{
    CFakeWayland* fake = CFakeWayland::g_current;
    fake->Configure( fake->firstConfigureWidth, fake->firstConfigureHeight, fake->firstConfigureStates, fake->firstConfigureStateCount );
}

}

/*-------------------------------------------------------------------------*/

void
CFakeWayland::OnBound( SFakeProxy* proxy )
{GUCEF_TRACE;

    if ( IsInterface( proxy, "wl_seat" ) )
        Post( ThunkSeatInit, proxy );
    else
    if ( IsInterface( proxy, "wl_output" ) )
    {
        proxy->outputIndex = m_outputsBound++;
        Post( ThunkOutputInit, proxy );
    }
}

/*-------------------------------------------------------------------------*/

void
CFakeWayland::OnRequest( SFakeProxy* proxy, const char* name, SFakeProxy* created, const GUCEF::vector< SFakeProxy* >& objectArgs )
{GUCEF_TRACE;

    const char* iface = GUCEF_NULL != proxy->iface ? proxy->iface->name : "wl_display";
    if ( 0 == strcmp( iface, "wl_display" ) && 0 == strcmp( name, "get_registry" ) )
        m_registry = created;
    else
    if ( 0 == strcmp( iface, "wl_registry" ) && 0 == strcmp( name, "bind" ) )
        OnBound( created );
    else
    if ( 0 == strcmp( iface, "xdg_wm_base" ) && 0 == strcmp( name, "get_xdg_surface" ) && !objectArgs.empty() )
        created->related = objectArgs[ 0 ];
    else
    if ( 0 == strcmp( iface, "xdg_surface" ) && 0 == strcmp( name, "get_toplevel" ) )
        created->related = proxy;
    else
    if ( 0 == strcmp( iface, "wl_surface" ) && 0 == strcmp( name, "commit" ) && autoConfigure )
    {
        for ( UInt32 i=0; i<m_proxies.size(); ++i )
        {
            SFakeProxy* candidate = m_proxies[ i ];
            if ( !candidate->destroyed && IsInterface( candidate, "xdg_surface" ) && candidate->related == proxy && !candidate->configured )
            {
                candidate->configured = true;
                Post( ThunkFirstConfigure, GUCEF_NULL );
            }
        }
    }
    else
    if ( 0 == strcmp( iface, "wl_seat" ) && 0 == strcmp( name, "get_keyboard" ) )
        Post( ThunkKeyboardInit, created );
    else
    if ( 0 == strcmp( iface, "wp_fractional_scale_manager_v1" ) && 0 == strcmp( name, "get_fractional_scale" ) )
        Post( ThunkFractionalInit, created );
    else
    if ( 0 == strcmp( iface, "zxdg_decoration_manager_v1" ) && 0 == strcmp( name, "get_toplevel_decoration" ) )
        Post( ThunkDecorationInit, created );
    else
    if ( 0 == strcmp( iface, "xdg_activation_token_v1" ) && 0 == strcmp( name, "commit" ) )
    {
        if ( activationAnswers )
            PostToQueue( ThunkActivationDone, proxy, proxy->queue, activationFromSocket );
    }
    else
    if ( 0 == strcmp( iface, "zxdg_output_manager_v1" ) && 0 == strcmp( name, "get_xdg_output" ) )
    {
        if ( !objectArgs.empty() )
            created->related = objectArgs[ 0 ];
        Post( ThunkXdgOutputInit, created );
    }
}

/*-------------------------------------------------------------------------*/

struct wl_proxy*
CFakeWayland::Marshal( struct wl_proxy* wlProxy, uint32_t opcode, const struct wl_interface* iface, uint32_t version, uint32_t flags, va_list args )
{GUCEF_TRACE;

    SFakeProxy* proxy = (SFakeProxy*) wlProxy;
    const struct wl_message* message = GUCEF_NULL;
    if ( GUCEF_NULL != proxy->iface && (int) opcode < proxy->iface->method_count )
        message = &proxy->iface->methods[ opcode ];
    const char* signature = GUCEF_NULL != message ? message->signature : "n";
    const char* name = GUCEF_NULL != message ? message->name : ( 1 == opcode ? "get_registry" : "sync" );
    const char* ifaceName = GUCEF_NULL != proxy->iface ? proxy->iface->name : "wl_display";

    CORE::CString text = ifaceName;
    text += '.';
    text += name;
    text += '(';
    bool first = true;
    bool hasNewId = false;
    GUCEF::vector< SFakeProxy* > objects;
    for ( const char* c=signature; '\0' != *c; ++c )
    {
        if ( ( *c >= '0' && *c <= '9' ) || '?' == *c )
            continue;
        if ( 'n' == *c )
        {
            (void) va_arg( args, void* );
            hasNewId = true;
            continue;
        }
        if ( !first )
            text += ", ";
        first = false;
        switch ( *c )
        {
            case 'i' : text += CORE::Int32ToString( va_arg( args, int ) ); break;
            case 'f' : text += CORE::Int32ToString( va_arg( args, int ) ); break;
            case 'u' : text += CORE::UInt32ToString( va_arg( args, unsigned int ) ); break;
            case 'h' : text += "fd"; (void) va_arg( args, int ); break;
            case 'a' : text += "array"; (void) va_arg( args, void* ); break;
            case 's' :
            {
                const char* string = va_arg( args, const char* );
                text += '"';
                text += GUCEF_NULL != string ? string : "";
                text += '"';
                break;
            }
            case 'o' :
            {
                SFakeProxy* object = (SFakeProxy*) va_arg( args, void* );
                objects.push_back( object );
                if ( GUCEF_NULL == object )
                    text += "null";
                else
                {
                    text += '<';
                    text += GUCEF_NULL != object->iface ? object->iface->name : "wl_display";
                    text += '#';
                    text += CORE::UInt32ToString( object->id );
                    text += '>';
                }
                break;
            }
            default : break;
        }
    }
    text += ')';
    requests.push_back( text );

    SFakeProxy* created = GUCEF_NULL;
    if ( hasNewId && GUCEF_NULL != iface )
        created = NewProxy( iface, version );
    if ( 0 != ( flags & GUCEF_WL_MARSHAL_FLAG_DESTROY ) )
        proxy->destroyed = true;
    OnRequest( proxy, name, created, objects );
    return (struct wl_proxy*) created;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EVENTS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define FAKE_LISTENER( type, interfaceName, variable, proxyVariable ) \
    SFakeProxy* proxyVariable = FindProxy( interfaceName ); \
    const struct type* variable = GUCEF_NULL != proxyVariable ? (const struct type*) proxyVariable->listener : GUCEF_NULL; \
    if ( GUCEF_NULL == variable ) return;

void
CFakeWayland::SendSeatCapabilities( UInt32 capabilities )
{
    FAKE_LISTENER( gucef_wl_seat_listener, "wl_seat", l, p );
    l->capabilities( p->data, (struct wl_seat*) p, capabilities );
}

void
CFakeWayland::SendPointerEnter( UInt32 serial, SFakeProxy* surface, double x, double y )
{
    FAKE_LISTENER( gucef_wl_pointer_listener, "wl_pointer", l, p );
    l->enter( p->data, (struct wl_pointer*) p, serial, (struct wl_surface*) surface, ToFixed( x ), ToFixed( y ) );
}

void
CFakeWayland::SendPointerLeave( UInt32 serial, SFakeProxy* surface )
{
    FAKE_LISTENER( gucef_wl_pointer_listener, "wl_pointer", l, p );
    l->leave( p->data, (struct wl_pointer*) p, serial, (struct wl_surface*) surface );
}

void
CFakeWayland::SendPointerMotion( UInt32 time, double x, double y )
{
    FAKE_LISTENER( gucef_wl_pointer_listener, "wl_pointer", l, p );
    l->motion( p->data, (struct wl_pointer*) p, time, ToFixed( x ), ToFixed( y ) );
}

void
CFakeWayland::SendPointerButton( UInt32 serial, UInt32 time, UInt32 button, UInt32 state )
{
    FAKE_LISTENER( gucef_wl_pointer_listener, "wl_pointer", l, p );
    l->button( p->data, (struct wl_pointer*) p, serial, time, button, state );
}

void
CFakeWayland::SendPointerAxis( UInt32 time, UInt32 axis, double value )
{
    FAKE_LISTENER( gucef_wl_pointer_listener, "wl_pointer", l, p );
    l->axis( p->data, (struct wl_pointer*) p, time, axis, ToFixed( value ) );
}

void
CFakeWayland::SendPointerAxisSource( UInt32 source )
{
    FAKE_LISTENER( gucef_wl_pointer_listener, "wl_pointer", l, p );
    l->axis_source( p->data, (struct wl_pointer*) p, source );
}

void
CFakeWayland::SendPointerAxisStop( UInt32 time, UInt32 axis )
{
    FAKE_LISTENER( gucef_wl_pointer_listener, "wl_pointer", l, p );
    l->axis_stop( p->data, (struct wl_pointer*) p, time, axis );
}

void
CFakeWayland::SendPointerAxisDiscrete( UInt32 axis, Int32 discrete )
{
    FAKE_LISTENER( gucef_wl_pointer_listener, "wl_pointer", l, p );
    l->axis_discrete( p->data, (struct wl_pointer*) p, axis, discrete );
}

void
CFakeWayland::SendPointerAxisValue120( UInt32 axis, Int32 value120 )
{
    FAKE_LISTENER( gucef_wl_pointer_listener, "wl_pointer", l, p );
    l->axis_value120( p->data, (struct wl_pointer*) p, axis, value120 );
}

void
CFakeWayland::SendPointerAxisRelativeDirection( UInt32 axis, UInt32 direction )
{
    FAKE_LISTENER( gucef_wl_pointer_listener, "wl_pointer", l, p );
    l->axis_relative_direction( p->data, (struct wl_pointer*) p, axis, direction );
}

void
CFakeWayland::SendPointerFrame( void )
{
    FAKE_LISTENER( gucef_wl_pointer_listener, "wl_pointer", l, p );
    l->frame( p->data, (struct wl_pointer*) p );
}

void
CFakeWayland::SendKeyboardKeymap( const char* text )
{
    FAKE_LISTENER( gucef_wl_keyboard_listener, "wl_keyboard", l, p );
    const size_t size = strlen( text ) + 1;
    int fd = ::memfd_create( "fake-keymap", 0 );
    if ( fd < 0 ) return;
    if ( (ssize_t) size != ::write( fd, text, size ) ) { ::close( fd ); return; }
    ::lseek( fd, 0, SEEK_SET );
    // the connection takes the file descriptor and closes it
    l->keymap( p->data, (struct wl_keyboard*) p, GUCEF_WL_WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1, fd, (uint32_t) size );
}

void
CFakeWayland::SendKeyboardEnter( UInt32 serial, SFakeProxy* surface )
{
    FAKE_LISTENER( gucef_wl_keyboard_listener, "wl_keyboard", l, p );
    struct wl_array keys;
    memset( &keys, 0, sizeof( keys ) );
    l->enter( p->data, (struct wl_keyboard*) p, serial, (struct wl_surface*) surface, &keys );
}

void
CFakeWayland::SendKeyboardLeave( UInt32 serial, SFakeProxy* surface )
{
    FAKE_LISTENER( gucef_wl_keyboard_listener, "wl_keyboard", l, p );
    l->leave( p->data, (struct wl_keyboard*) p, serial, (struct wl_surface*) surface );
}

void
CFakeWayland::SendKeyboardKey( UInt32 serial, UInt32 time, UInt32 evdevKey, UInt32 state )
{
    FAKE_LISTENER( gucef_wl_keyboard_listener, "wl_keyboard", l, p );
    l->key( p->data, (struct wl_keyboard*) p, serial, time, evdevKey, state );
}

void
CFakeWayland::SendKeyboardModifiers( UInt32 serial, UInt32 depressed, UInt32 latched, UInt32 locked, UInt32 group )
{
    FAKE_LISTENER( gucef_wl_keyboard_listener, "wl_keyboard", l, p );
    l->modifiers( p->data, (struct wl_keyboard*) p, serial, depressed, latched, locked, group );
}

void
CFakeWayland::SendKeyboardRepeatInfo( Int32 rate, Int32 delay )
{
    FAKE_LISTENER( gucef_wl_keyboard_listener, "wl_keyboard", l, p );
    l->repeat_info( p->data, (struct wl_keyboard*) p, rate, delay );
}

void
CFakeWayland::SendSurfaceEnter( SFakeProxy* surface, SFakeProxy* outputProxy )
{
    const struct gucef_wl_surface_listener* l = (const struct gucef_wl_surface_listener*) surface->listener;
    if ( GUCEF_NULL != l ) l->enter( surface->data, (struct wl_surface*) surface, (struct wl_output*) outputProxy );
}

void
CFakeWayland::SendSurfaceLeave( SFakeProxy* surface, SFakeProxy* outputProxy )
{
    const struct gucef_wl_surface_listener* l = (const struct gucef_wl_surface_listener*) surface->listener;
    if ( GUCEF_NULL != l ) l->leave( surface->data, (struct wl_surface*) surface, (struct wl_output*) outputProxy );
}

void
CFakeWayland::SendPreferredBufferScale( SFakeProxy* surface, Int32 factor )
{
    const struct gucef_wl_surface_listener* l = (const struct gucef_wl_surface_listener*) surface->listener;
    if ( GUCEF_NULL != l ) l->preferred_buffer_scale( surface->data, (struct wl_surface*) surface, factor );
}

void
CFakeWayland::SendFractionalScale( UInt32 scale120 )
{
    FAKE_LISTENER( gucef_wp_fractional_scale_v1_listener, "wp_fractional_scale_v1", l, p );
    l->preferred_scale( p->data, (struct wp_fractional_scale_v1*) p, scale120 );
}

void
CFakeWayland::SendOutputScale( SFakeProxy* outputProxy, Int32 scale )
{
    const struct gucef_wl_output_listener* l = (const struct gucef_wl_output_listener*) outputProxy->listener;
    if ( GUCEF_NULL != l ) l->scale( outputProxy->data, (struct wl_output*) outputProxy, scale );
}

void
CFakeWayland::SendOutputDone( SFakeProxy* outputProxy )
{
    const struct gucef_wl_output_listener* l = (const struct gucef_wl_output_listener*) outputProxy->listener;
    if ( GUCEF_NULL != l ) l->done( outputProxy->data, (struct wl_output*) outputProxy );
}

void
CFakeWayland::SendToplevelConfigure( Int32 width, Int32 height, const UInt32* states, UInt32 stateCount )
{
    FAKE_LISTENER( gucef_xdg_toplevel_listener, "xdg_toplevel", l, p );
    struct wl_array array;
    array.size = stateCount * sizeof( UInt32 );
    array.alloc = array.size;
    array.data = const_cast< UInt32* >( states );
    l->configure( p->data, (struct xdg_toplevel*) p, width, height, &array );
}

void
CFakeWayland::SendXdgSurfaceConfigure( UInt32 serial )
{
    FAKE_LISTENER( gucef_xdg_surface_listener, "xdg_surface", l, p );
    l->configure( p->data, (struct xdg_surface*) p, serial );
}

UInt32
CFakeWayland::Configure( Int32 width, Int32 height, const UInt32* states, UInt32 stateCount )
{
    SendToplevelConfigure( width, height, states, stateCount );
    const UInt32 serial = m_nextSerial++;
    SendXdgSurfaceConfigure( serial );
    return serial;
}

void
CFakeWayland::SendToplevelClose( void )
{
    FAKE_LISTENER( gucef_xdg_toplevel_listener, "xdg_toplevel", l, p );
    l->close( p->data, (struct xdg_toplevel*) p );
}

void
CFakeWayland::SendPing( UInt32 serial )
{
    FAKE_LISTENER( gucef_xdg_wm_base_listener, "xdg_wm_base", l, p );
    l->ping( p->data, (struct xdg_wm_base*) p, serial );
}

void
CFakeWayland::SendFrameDone( SFakeProxy* callback )
{
    const struct gucef_wl_callback_listener* l = (const struct gucef_wl_callback_listener*) callback->listener;
    if ( GUCEF_NULL != l ) l->done( callback->data, (struct wl_callback*) callback, 0 );
}

void
CFakeWayland::SendDecorationMode( UInt32 mode )
{
    FAKE_LISTENER( gucef_zxdg_toplevel_decoration_v1_listener, "zxdg_toplevel_decoration_v1", l, p );
    l->configure( p->data, (struct zxdg_toplevel_decoration_v1*) p, mode );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EGL AND LIBDECOR                                                   //
//                                                                         //
//-------------------------------------------------------------------------*/

CFakeGraphics* CFakeGraphics::g_current = GUCEF_NULL;

namespace {

struct SFakeState
{
    int width;
    int height;
};

SFakeState g_state;

void G_glViewport( int, int, int width, int height ) { CFakeGraphics::g_current->lastViewportWidth = width; CFakeGraphics::g_current->lastViewportHeight = height; }
void G_glClearColor( float, float, float, float ) {}
void G_glClear( unsigned int ) { ++CFakeGraphics::g_current->clearCount; }

void*
E_getProcAddress( const char* name )
{
    if ( 0 == strcmp( name, "glViewport" ) ) return (void*) &G_glViewport;
    if ( 0 == strcmp( name, "glClearColor" ) ) return (void*) &G_glClearColor;
    if ( 0 == strcmp( name, "glClear" ) ) return (void*) &G_glClear;
    return GUCEF_NULL;
}

void*
E_getPlatformDisplay( unsigned int platform, void*, const intptr_t* )
{
    ++CFakeGraphics::g_current->platformDisplayCalls;
    CFakeGraphics::g_current->lastPlatform = platform;
    return (void*) 0x1000;
}

unsigned int E_initialize( void*, int* major, int* minor ) { *major = 1; *minor = 5; return 1; }
unsigned int E_terminate( void* ) { return 1; }
unsigned int E_bindApi( unsigned int ) { return 1; }

unsigned int
E_chooseConfig( void*, const int*, void** configs, int, int* count )
{
    if ( CFakeGraphics::g_current->noConfig )
    {
        *count = 0;
        return 1;
    }
    configs[ 0 ] = (void*) 0x2000;
    *count = 1;
    return 1;
}

void*
E_createContext( void*, void*, void*, const int* attributes )
{
    if ( CFakeGraphics::g_current->contextCoreFails )
        for ( int i=0; GUIDRIVERWAYLAND::GUCEF_EGL_NONE != attributes[ i ]; i += 2 )
            if ( GUIDRIVERWAYLAND::GUCEF_EGL_CONTEXT_OPENGL_PROFILE_MASK == attributes[ i ] )
                return GUCEF_NULL;
    return (void*) 0x3000;
}

unsigned int E_destroyContext( void*, void* ) { return 1; }
void* E_createWindowSurface( void*, void*, void*, const int* ) { ++CFakeGraphics::g_current->createSurfaceCount; return (void*) 0x4000; }
unsigned int E_destroySurface( void*, void* ) { ++CFakeGraphics::g_current->destroySurfaceCount; return 1; }
unsigned int E_makeCurrent( void*, void*, void*, void* ) { ++CFakeGraphics::g_current->makeCurrentCount; return 1; }
unsigned int E_swapBuffers( void*, void* ) { ++CFakeGraphics::g_current->swapCount; return 1; }
unsigned int E_swapInterval( void*, int interval ) { CFakeGraphics::g_current->swapInterval = interval; return 1; }
int E_getError( void ) { return 0x3000; }

struct wl_egl_window*
W_create( struct wl_surface*, int width, int height )
{
    CFakeGraphics* g = CFakeGraphics::g_current;
    ++g->windowCreateCount;
    g->lastWindowWidth = width;
    g->lastWindowHeight = height;
    return (struct wl_egl_window*) 0x5000;
}

void W_destroy( struct wl_egl_window* ) { ++CFakeGraphics::g_current->windowDestroyCount; }

void
W_resize( struct wl_egl_window*, int width, int height, int, int )
{
    CFakeGraphics* g = CFakeGraphics::g_current;
    ++g->resizeCount;
    g->lastWindowWidth = width;
    g->lastWindowHeight = height;
}

struct libdecor* D_create( struct wl_display*, const GUIDRIVERWAYLAND::SLibdecorInterface* ) { ++CFakeGraphics::g_current->libdecorCreated; return (struct libdecor*) 0x6000; }
void D_unref( struct libdecor* ) {}

int
D_dispatch( struct libdecor*, int )
{
    CFakeGraphics* g = CFakeGraphics::g_current;
    ++g->libdecorDispatches;

    // the compositor configures the window once it was mapped
    if ( g->frameMapped && !g->configureDelivered && g->autoConfigure && GUCEF_NULL != g->frameCallbacks.configure )
    {
        g->configureDelivered = true;
        g->frameCallbacks.configure( g->frame, (struct libdecor_configuration*) 0x8000, g->frameUserData );
    }
    return 0;
}

struct libdecor_frame*
D_decorate( struct libdecor*, struct wl_surface*, const GUIDRIVERWAYLAND::SLibdecorFrameInterface* iface, void* userData )
{
    CFakeGraphics* g = CFakeGraphics::g_current;
    if ( g->decorateFails )
        return GUCEF_NULL;
    g->frameCallbacks = *iface;
    g->frameUserData = userData;
    g->frame = (struct libdecor_frame*) 0x7000;
    return g->frame;
}

void D_frameUnref( struct libdecor_frame* ) { ++CFakeGraphics::g_current->frameUnrefs; }
void D_setTitle( struct libdecor_frame*, const char* title ) { CFakeGraphics::g_current->frameTitle = title; }
void D_setAppId( struct libdecor_frame*, const char* appId ) { CFakeGraphics::g_current->frameAppId = appId; }
void D_setMin( struct libdecor_frame*, int, int ) {}
void D_setMax( struct libdecor_frame*, int, int ) {}
void D_setFullscreen( struct libdecor_frame*, struct wl_output* ) { CFakeGraphics::g_current->frameFullscreen = true; }
void D_unsetFullscreen( struct libdecor_frame* ) { CFakeGraphics::g_current->frameFullscreen = false; }
void D_setMaximized( struct libdecor_frame* ) {}
void D_unsetMaximized( struct libdecor_frame* ) {}
void D_setVisibility( struct libdecor_frame*, bool ) {}
void D_map( struct libdecor_frame* ) { CFakeGraphics::g_current->frameMapped = true; }

void
D_commit( struct libdecor_frame*, struct libdecor_state* state, struct libdecor_configuration* )
{
    CFakeGraphics* g = CFakeGraphics::g_current;
    SFakeState* s = (SFakeState*) state;
    g->lastCommitWidth = s->width;
    g->lastCommitHeight = s->height;
    ++g->commitCount;
}

bool
D_getContentSize( struct libdecor_configuration*, struct libdecor_frame*, int* width, int* height )
{
    CFakeGraphics* g = CFakeGraphics::g_current;
    if ( g->configureWidth <= 0 )
        return false;
    *width = g->configureWidth;
    *height = g->configureHeight;
    return true;
}

bool
D_getWindowState( struct libdecor_configuration*, int* state )
{
    *state = CFakeGraphics::g_current->configureState;
    return true;
}

struct libdecor_state*
D_stateNew( int width, int height )
{
    SFakeState* state = new SFakeState();
    state->width = width;
    state->height = height;
    return (struct libdecor_state*) state;
}

void D_stateFree( struct libdecor_state* state ) { delete (SFakeState*) state; }

}

/*-------------------------------------------------------------------------*/

CFakeGraphics::CFakeGraphics( void )
    : swapCount( 0 ), createSurfaceCount( 0 ), destroySurfaceCount( 0 ), makeCurrentCount( 0 ), platformDisplayCalls( 0 ), lastPlatform( 0 )
    , windowCreateCount( 0 ), windowDestroyCount( 0 ), resizeCount( 0 ), lastWindowWidth( 0 ), lastWindowHeight( 0 )
    , lastViewportWidth( 0 ), lastViewportHeight( 0 ), clearCount( 0 ), contextCoreFails( false ), noConfig( false ), swapInterval( -1 )
    , libdecorCreated( 0 ), libdecorDispatches( 0 ), frameUnrefs( 0 ), frameTitle(), frameAppId(), frameMapped( false ), frameFullscreen( false )
    , lastCommitWidth( 0 ), lastCommitHeight( 0 ), commitCount( 0 ), decorateFails( false ), configureWidth( 0 ), configureHeight( 0 ), configureState( 0 )
    , frameUserData( GUCEF_NULL ), frame( GUCEF_NULL ), configureDelivered( false ), autoConfigure( true ), m_previous( g_current ), m_runtime()
{GUCEF_TRACE;

    memset( &frameCallbacks, 0, sizeof( frameCallbacks ) );
    GUIDRIVERWAYLAND::SEglApi& egl = m_runtime.egl;
    egl.getProcAddress = E_getProcAddress;
    egl.getPlatformDisplay = E_getPlatformDisplay;
    egl.getPlatformDisplayEXT = GUCEF_NULL;
    egl.getDisplay = GUCEF_NULL;
    egl.initialize = E_initialize;
    egl.terminate = E_terminate;
    egl.bindApi = E_bindApi;
    egl.chooseConfig = E_chooseConfig;
    egl.createContext = E_createContext;
    egl.destroyContext = E_destroyContext;
    egl.createWindowSurface = E_createWindowSurface;
    egl.destroySurface = E_destroySurface;
    egl.makeCurrent = E_makeCurrent;
    egl.swapBuffers = E_swapBuffers;
    egl.swapInterval = E_swapInterval;
    egl.getError = E_getError;
    m_runtime.hasEgl = true;

    m_runtime.eglWindow.window_create = W_create;
    m_runtime.eglWindow.window_destroy = W_destroy;
    m_runtime.eglWindow.window_resize = W_resize;
    m_runtime.eglWindow.window_get_attached_size = GUCEF_NULL;
    m_runtime.hasEglWindow = true;

    GUIDRIVERWAYLAND::SLibdecorApi& decor = m_runtime.libdecor;
    decor.create = D_create;
    decor.unref = D_unref;
    decor.dispatch = D_dispatch;
    decor.decorate = D_decorate;
    decor.frame_unref = D_frameUnref;
    decor.frame_set_title = D_setTitle;
    decor.frame_set_app_id = D_setAppId;
    decor.frame_set_min_content_size = D_setMin;
    decor.frame_set_max_content_size = D_setMax;
    decor.frame_set_fullscreen = D_setFullscreen;
    decor.frame_unset_fullscreen = D_unsetFullscreen;
    decor.frame_set_maximized = D_setMaximized;
    decor.frame_unset_maximized = D_unsetMaximized;
    decor.frame_set_visibility = D_setVisibility;
    decor.frame_map = D_map;
    decor.frame_commit = D_commit;
    decor.configuration_get_content_size = D_getContentSize;
    decor.configuration_get_window_state = D_getWindowState;
    decor.state_new = D_stateNew;
    decor.state_free = D_stateFree;
    m_runtime.hasLibdecor = false;
    g_current = this;
}

/*-------------------------------------------------------------------------*/

CFakeGraphics::~CFakeGraphics()
{GUCEF_TRACE;

    if ( g_current == this )
        g_current = m_previous;
}

/*-------------------------------------------------------------------------*/

const GUIDRIVERWAYLAND::SWaylandGuiRuntime&
CFakeGraphics::GetRuntime( void ) const
{GUCEF_TRACE;

    return m_runtime;
}

/*-------------------------------------------------------------------------*/

void
CFakeGraphics::SetLibdecorAvailable( bool available )
{GUCEF_TRACE;

    m_runtime.hasLibdecor = available;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} // namespace TESTWAYLAND
} // namespace GUCEF

/*-------------------------------------------------------------------------*/
