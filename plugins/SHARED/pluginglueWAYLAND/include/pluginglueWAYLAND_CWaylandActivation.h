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


#ifndef GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDACTIVATION_H
#define GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDACTIVATION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <pthread.h>

#ifndef GUCEF_PLUGINGLUE_WAYLAND_MACROS_H
#include "pluginglueWAYLAND_macros.h"
#define GUCEF_PLUGINGLUE_WAYLAND_MACROS_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_MACROS_H ? */

#ifndef GUCEF_CORE_BROWSERLAUNCHER_H
#include "gucefCORE_BrowserLauncher.h"
#define GUCEF_CORE_BROWSERLAUNCHER_H
#endif /* GUCEF_CORE_BROWSERLAUNCHER_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CWaylandConnection;

/**
 *  Makes the activation token (xdg-activation-v1) that lets a program that is started from a window of this one come to the
 *  front: GetActivationToken() asks the compositor for a token from the serial of the last pointer button or key event of
 *  the seat, the window the user used and the application id, and waits for its answer for at most the time it is given.
 *  The launcher of gucefCORE (CIActivationTokenProvider) puts the token in the environment of the browser.
 *
 *  The request is answered on an event queue of its own: asking from inside the handler of a click, which is where a login
 *  starts, dispatches nothing else of the connection while it waits, and the other events wait for the dispatch that is going on.
 *  Only the thread the provider was made on may ask (the thread that pumps the connection); another thread is told there is
 *  no token. Without the global xdg_activation_v1, a window, a connection that works or the event queue functions of libwayland
 *  there is no token either. The provider never blocks longer than it is told and the token is never logged.
 */
class GUCEF_PLUGINGLUE_WAYLAND_EXPORT_CPP CWaylandActivationTokenProvider : public CORE::CIActivationTokenProvider
{
    public:

    /**
     *  @param connection   the connection to the compositor; has to outlive the provider
     *  @param surfaceId    the id of the window the user works in (CWaylandConnection::RegisterSurface()); the provider has no token while it is not registered
     *  @param appId        the application id of the window, as the compositor knows it; may be empty
     */
    CWaylandActivationTokenProvider( CWaylandConnection& connection, UInt32 surfaceId, const CORE::CString& appId );

    virtual ~CWaylandActivationTokenProvider();

    virtual bool GetActivationToken( CORE::CString& token, CORE::TActivationTokenKind& kind, UInt32 maxWaitInMs ) GUCEF_VIRTUAL_OVERRIDE;

    /** Whether the compositor offers xdg_activation_v1 and the library has what the provider needs; no token is made without */
    bool IsSupported( void ) const;

    private:

    CWaylandActivationTokenProvider( const CWaylandActivationTokenProvider& src );
    CWaylandActivationTokenProvider& operator=( const CWaylandActivationTokenProvider& src );

    private:

    CWaylandConnection& m_connection;
    UInt32 m_surfaceId;
    CORE::CString m_appId;
    pthread_t m_thread;
    bool m_busy;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WAYLAND */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDACTIVATION_H ? */
