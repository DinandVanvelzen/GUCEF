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
#include <time.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_PROTOCOLS_H
#include "pluginglueWAYLAND_Protocols.h"
#define GUCEF_PLUGINGLUE_WAYLAND_PROTOCOLS_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_PROTOCOLS_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_CEVENTGUARD_H
#include "pluginglueWAYLAND_CEventGuard.h"
#define GUCEF_PLUGINGLUE_WAYLAND_CEVENTGUARD_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_CEVENTGUARD_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDCONNECTION_H
#include "pluginglueWAYLAND_CWaylandConnection.h"
#define GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDCONNECTION_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDCONNECTION_H ? */

#include "pluginglueWAYLAND_CWaylandActivation.h"

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** What the listener of the token writes into: lives on the stack of the request */
struct SActivationRequest
{
    bool done;
    CORE::CString token;

    SActivationRequest( void ) : done( false ), token() {}
};

void
TokenDone( void* data, struct xdg_activation_token_v1*, const char* token )
{
    GUCEF_WL_GUARD_BEGIN
    SActivationRequest* request = static_cast< SActivationRequest* >( data );
    request->token = GUCEF_NULL != token ? token : "";
    request->done = true;
    GUCEF_WL_GUARD_END
}

const struct gucef_xdg_activation_token_v1_listener g_tokenListener = { TokenDone };

UInt64
MonotonicTimeInMs( void )
{
    struct timespec now;
    ::clock_gettime( CLOCK_MONOTONIC, &now );
    return (UInt64) now.tv_sec * 1000 + (UInt64) now.tv_nsec / 1000000;
}

bool
HasQueueFunctions( const SWaylandApi& api )
{
    return GUCEF_NULL != api.display_create_queue && GUCEF_NULL != api.event_queue_destroy && GUCEF_NULL != api.proxy_set_queue &&
           GUCEF_NULL != api.display_dispatch_queue_pending && GUCEF_NULL != api.display_prepare_read_queue;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWaylandActivationTokenProvider::CWaylandActivationTokenProvider( CWaylandConnection& connection, UInt32 surfaceId, const CORE::CString& appId )
    : CORE::CIActivationTokenProvider()
    , m_connection( connection )
    , m_surfaceId( surfaceId )
    , m_appId( appId )
    , m_thread( ::pthread_self() )
    , m_busy( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWaylandActivationTokenProvider::~CWaylandActivationTokenProvider()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CWaylandActivationTokenProvider::IsSupported( void ) const
{GUCEF_TRACE;

    return m_connection.IsConnected() && m_connection.IsHealthy() && GUCEF_NULL != m_connection.GetActivationManager() && HasQueueFunctions( m_connection.GetApi() );
}

/*-------------------------------------------------------------------------*/

bool
CWaylandActivationTokenProvider::GetActivationToken( CORE::CString& token, CORE::TActivationTokenKind& kind, UInt32 maxWaitInMs )
{GUCEF_TRACE;

    token.Clear();
    kind = CORE::ACTIVATIONTOKEN_XDG_ACTIVATION;

    // the connection belongs to the thread that pumps it, and a request from within a request would wait on itself
    if ( 0 == maxWaitInMs || m_busy || 0 == ::pthread_equal( m_thread, ::pthread_self() ) )
        return false;
    if ( !IsSupported() )
        return false;
    const SWaylandSurfaceInfo* surface = m_connection.FindSurface( m_surfaceId );
    if ( GUCEF_NULL == surface || GUCEF_NULL == surface->surface )
        return false;

    m_busy = true;
    const SWaylandApi& api = m_connection.GetApi();
    struct wl_display* display = m_connection.GetDisplay();
    SActivationRequest request;
    bool failed = false;

    struct wl_event_queue* queue = api.display_create_queue( display );
    struct xdg_activation_token_v1* object = GUCEF_NULL != queue ? gucef_xdg_activation_v1_get_activation_token( &api, m_connection.GetActivationManager() ) : GUCEF_NULL;
    if ( GUCEF_NULL != object )
    {
        // the answer comes on the queue of its own: nothing is read into it before this, the request is not even sent yet
        api.proxy_set_queue( (struct wl_proxy*) object, queue );
        if ( 0 != gucef_xdg_activation_token_v1_add_listener( &api, object, &g_tokenListener, &request ) )
            failed = true;
        else
        {
            // the serial of the last button or key of the user, and the seat it came from, say what the user did
            UInt32 serial = m_connection.GetLastActivationSerial();
            if ( 0 == serial )
                serial = m_connection.GetLastInputSerial();
            if ( 0 != serial && GUCEF_NULL != m_connection.GetSeat() )
                gucef_xdg_activation_token_v1_set_serial( &api, object, serial, m_connection.GetSeat() );
            if ( !m_appId.IsNULLOrEmpty() )
                gucef_xdg_activation_token_v1_set_app_id( &api, object, m_appId.C_String() );
            gucef_xdg_activation_token_v1_set_surface( &api, object, surface->surface );
            gucef_xdg_activation_token_v1_commit( &api, object );
            if ( api.display_flush( display ) < 0 && EAGAIN != errno )
                failed = true;
        }

        const UInt64 deadline = MonotonicTimeInMs() + maxWaitInMs;
        while ( !failed && !request.done )
        {
            {
                CDispatchPhase dispatchPhase;
                if ( api.display_dispatch_queue_pending( display, queue ) < 0 )
                {
                    failed = true;
                    break;
                }
            }
            if ( request.done )
                break;
            const UInt64 now = MonotonicTimeInMs();
            if ( now >= deadline )
                break;

            // events of the queue that are waiting are dispatched first; else the socket is read, for the time that is left (10 ms at most, the state is looked at again then)
            if ( 0 != api.display_prepare_read_queue( display, queue ) )
                continue;
            if ( api.display_flush( display ) < 0 && EAGAIN != errno )
            {
                api.display_cancel_read( display );
                failed = true;
                break;
            }
            struct pollfd pollInfo;
            pollInfo.fd = api.display_get_fd( display );
            pollInfo.events = POLLIN;
            pollInfo.revents = 0;
            const UInt64 left = deadline - now;
            const int polled = pollInfo.fd >= 0 ? ::poll( &pollInfo, 1, (int) ( left < 10 ? left : 10 ) ) : 0;
            if ( polled > 0 && 0 != ( pollInfo.revents & POLLIN ) )
            {
                if ( api.display_read_events( display ) < 0 )
                    failed = true;
            }
            else
            {
                api.display_cancel_read( display );
                if ( polled > 0 && 0 != ( pollInfo.revents & ( POLLERR | POLLHUP | POLLNVAL ) ) )
                    failed = true;
            }
        }

        // an answer that comes after this is dropped by the library: the proxy is gone
        gucef_xdg_activation_token_v1_destroy( &api, object );
    }
    if ( GUCEF_NULL != queue )
        api.event_queue_destroy( queue );
    m_busy = false;

    if ( failed || !request.done || request.token.IsNULLOrEmpty() )
    {
        const char* reason = failed ? "the request failed" : ( request.done ? "the compositor gave an empty token" : "the compositor did not answer in time" );
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CWaylandActivationTokenProvider: No activation token: " << reason );
        request.token.Clear();
        return false;
    }
    token = request.token;
    request.token.Clear();
    return true;
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
