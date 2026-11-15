/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#ifndef GUCEF_DRNUTILS_REMOTESERVICETYPES_H
#define GUCEF_DRNUTILS_REMOTESERVICETYPES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_DRNUTILS_MACROS_H
#include "gucefDRNUTILS_macros.h"
#define GUCEF_DRNUTILS_MACROS_H
#endif /* GUCEF_DRNUTILS_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

enum EDeviceStatus
{
    DEVICESTATUS_PENDING_CONFIRMATION = 0 ,     /**< paired; the owner has not confirmed the key fingerprint yet */
    DEVICESTATUS_ACTIVE ,                       /**< may join and call within its roles */
    DEVICESTATUS_REVOKED                        /**< refused everywhere; kept for the audit and its bans */
};
typedef enum EDeviceStatus TDeviceStatus;

/** "PENDING_CONFIRMATION", "ACTIVE", "REVOKED" */
GUCEF_DRNUTILS_PUBLIC_CPP const char* DeviceStatusToString( TDeviceStatus status );
GUCEF_DRNUTILS_PUBLIC_CPP bool DeviceStatusFromString( const CORE::CString& text, TDeviceStatus& status );

/** The state of a client session of a remote service host (CRemoteSession) */
enum ERemoteSessionState
{
    REMOTESESSION_ACTIVE = 0 ,      /**< bound to an RPC session of its device */
    REMOTESESSION_SUSPENDED ,       /**< the RPC session went without Goodbye: kept for the resume grace */
    REMOTESESSION_CLOSED            /**< released; its id is unknown from now on */
};
typedef enum ERemoteSessionState TRemoteSessionState;

/** "ACTIVE", "SUSPENDED", "CLOSED" */
GUCEF_DRNUTILS_PUBLIC_CPP const char* RemoteSessionStateToString( TRemoteSessionState state );

/** The state of a client's connection to a remote service (CRemoteServiceConnection) */
enum ERemoteConnectionState
{
    REMOTECONNECTION_IDLE = 0 ,         /**< nothing started, or closed */
    REMOTECONNECTION_JOINING ,          /**< the DRN link and session are being set up */
    REMOTECONNECTION_BINDING ,          /**< the RPC session binds the contracts */
    REMOTECONNECTION_OPENING ,          /**< Session/Hello or Session/Resume is on its way */
    REMOTECONNECTION_CONNECTED ,        /**< the client session is active: calls and the event stream work */
    REMOTECONNECTION_RECONNECTING ,     /**< the link dropped; it is set up again (calls wait while the RPC session is suspended) */
    REMOTECONNECTION_FAILED             /**< the connection gave up (a permanent refusal or no reconnecting): GetLastError() says why */
};
typedef enum ERemoteConnectionState TRemoteConnectionState;

/** "IDLE", "JOINING", "BINDING", "OPENING", "CONNECTED", "RECONNECTING", "FAILED" */
GUCEF_DRNUTILS_PUBLIC_CPP const char* RemoteConnectionStateToString( TRemoteConnectionState state );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_REMOTESERVICETYPES_H ? */
