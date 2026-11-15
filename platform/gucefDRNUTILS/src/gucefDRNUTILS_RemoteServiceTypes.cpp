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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefDRNUTILS_RemoteServiceTypes.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char*
DeviceStatusToString( TDeviceStatus status )
{GUCEF_TRACE;

    switch ( status )
    {
        case DEVICESTATUS_PENDING_CONFIRMATION: return "PENDING_CONFIRMATION";
        case DEVICESTATUS_ACTIVE: return "ACTIVE";
        case DEVICESTATUS_REVOKED: return "REVOKED";
        default: return "UNKNOWN";
    }
}

/*-------------------------------------------------------------------------*/

bool
DeviceStatusFromString( const CORE::CString& text, TDeviceStatus& status )
{GUCEF_TRACE;

    CORE::CString upper = text.Trim( true ).Trim( false ).Uppercase();
    if ( upper == "PENDING_CONFIRMATION" )
        status = DEVICESTATUS_PENDING_CONFIRMATION;
    else if ( upper == "ACTIVE" )
        status = DEVICESTATUS_ACTIVE;
    else if ( upper == "REVOKED" )
        status = DEVICESTATUS_REVOKED;
    else
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

const char*
RemoteSessionStateToString( TRemoteSessionState state )
{GUCEF_TRACE;

    switch ( state )
    {
        case REMOTESESSION_ACTIVE: return "ACTIVE";
        case REMOTESESSION_SUSPENDED: return "SUSPENDED";
        case REMOTESESSION_CLOSED: return "CLOSED";
        default: return "UNKNOWN";
    }
}

/*-------------------------------------------------------------------------*/

const char*
RemoteConnectionStateToString( TRemoteConnectionState state )
{GUCEF_TRACE;

    switch ( state )
    {
        case REMOTECONNECTION_IDLE: return "IDLE";
        case REMOTECONNECTION_JOINING: return "JOINING";
        case REMOTECONNECTION_BINDING: return "BINDING";
        case REMOTECONNECTION_OPENING: return "OPENING";
        case REMOTECONNECTION_CONNECTED: return "CONNECTED";
        case REMOTECONNECTION_RECONNECTING: return "RECONNECTING";
        case REMOTECONNECTION_FAILED: return "FAILED";
        default: return "UNKNOWN";
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
