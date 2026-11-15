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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#include "gucefDRNUTILS_CDrnUtilsHostSupport.h"

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

const char* CDrnUtilsHostSupport::SessionIdAttribute = "drnutils.sessionId";

/*-------------------------------------------------------------------------*/

UInt64
CDrnUtilsHostSupport::GetMonotonicMs( void )
{GUCEF_TRACE;

    UInt64 ticksPerMs = MT::PrecisionTimerResolution() / 1000;
    return MT::PrecisionTickCount() / ( ticksPerMs > 0 ? ticksPerMs : 1 );
}

/*-------------------------------------------------------------------------*/

Int64
CDrnUtilsHostSupport::GetUtcMs( void )
{GUCEF_TRACE;

    return (Int64) CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
}

/*-------------------------------------------------------------------------*/

bool
CDrnUtilsHostSupport::IsSecure( const COMCORE::CRpcPrincipal& principal )
{GUCEF_TRACE;

    if ( principal.transport == "loopback" || principal.transport == "local" )
        return true;
    return principal.attributes.GetValueAlways( principal.transport + ".protected" ).AsBool( false );
}

/*-------------------------------------------------------------------------*/

bool
CDrnUtilsHostSupport::HasAnyRole( const CORE::CString::StringSet& principalRoles, const CORE::CString::StringSet& roles, bool emptySetAllows )
{GUCEF_TRACE;

    if ( roles.empty() )
        return emptySetAllows;
    CORE::CString::StringSet::const_iterator i = roles.begin();
    while ( i != roles.end() )
    {
        if ( principalRoles.find( (*i) ) != principalRoles.end() )
            return true;
        ++i;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

TDrnUtilsCommandStatus
CDrnUtilsHostSupport::CheckCommandId( const CORE::CString& commandId, bool dryRun, CORE::CString& problem )
{GUCEF_TRACE;

    if ( commandId.IsNULLOrEmpty() && !dryRun )
    {
        problem = "a command id is required";
        return DRNUTILS_STATUS_INVALID_ARGUMENT;
    }
    if ( commandId.ByteSize() > MaxCommandIdBytes + 1 )
    {
        problem = "the command id is longer than " + CORE::ToString( MaxCommandIdBytes ) + " bytes";
        return DRNUTILS_STATUS_INVALID_ARGUMENT;
    }
    return DRNUTILS_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

bool
CDrnUtilsHostSupport::GetCommandId( const COMCORE::CRpcServerCall& call, const CORE::CString& fieldValue, CORE::CString& commandId, CORE::CString& problem )
{GUCEF_TRACE;

    CORE::CString metadataKey = call.GetMetadata().GetString( "idempotency-key" );
    if ( !fieldValue.IsNULLOrEmpty() && !metadataKey.IsNULLOrEmpty() && fieldValue != metadataKey )
    {
        problem = "the command id and the idempotency-key metadata differ";
        return false;
    }
    commandId = fieldValue.IsNULLOrEmpty() ? metadataKey : fieldValue;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDrnUtilsHostSupport::GetSessionKey( const COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    static const char* hexDigits = "0123456789abcdef";
    UInt8 sessionId[ GUCEF_RPC_SESSIONID_SIZE ];
    if ( !call.GetSessionId( sessionId ) )
        return CORE::CString::Empty;
    char text[ GUCEF_RPC_SESSIONID_SIZE * 2 + 1 ];
    for ( UInt32 i=0; i<GUCEF_RPC_SESSIONID_SIZE; ++i )
    {
        text[ i*2 ] = hexDigits[ ( sessionId[ i ] >> 4 ) & 0x0F ];
        text[ i*2+1 ] = hexDigits[ sessionId[ i ] & 0x0F ];
    }
    text[ GUCEF_RPC_SESSIONID_SIZE * 2 ] = '\0';
    return CORE::CString( text );
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcStatusCode
CDrnUtilsHostSupport::ToRpcStatusCode( TDrnUtilsCommandStatus status )
{GUCEF_TRACE;

    switch ( status )
    {
        case DRNUTILS_STATUS_OK: return COMCORE::RPC_STATUS_OK;
        case DRNUTILS_STATUS_NOT_FOUND: return COMCORE::RPC_STATUS_NOT_FOUND;
        case DRNUTILS_STATUS_INVALID_ARGUMENT: return COMCORE::RPC_STATUS_INVALID_ARGUMENT;
        case DRNUTILS_STATUS_FAILED_PRECONDITION: return COMCORE::RPC_STATUS_FAILED_PRECONDITION;
        case DRNUTILS_STATUS_ABORTED: return COMCORE::RPC_STATUS_ABORTED;
        case DRNUTILS_STATUS_UNIMPLEMENTED: return COMCORE::RPC_STATUS_UNIMPLEMENTED;
        case DRNUTILS_STATUS_RESOURCE_EXHAUSTED: return COMCORE::RPC_STATUS_RESOURCE_EXHAUSTED;
        case DRNUTILS_STATUS_PERMISSION_DENIED: return COMCORE::RPC_STATUS_PERMISSION_DENIED;
        case DRNUTILS_STATUS_UNAVAILABLE: return COMCORE::RPC_STATUS_UNAVAILABLE;
        default: return COMCORE::RPC_STATUS_INTERNAL;
    }
}

/*-------------------------------------------------------------------------*/

TDrnUtilsCommandStatus
CDrnUtilsHostSupport::FromRpcStatusCode( COMCORE::TRpcStatusCode code )
{GUCEF_TRACE;

    switch ( code )
    {
        case COMCORE::RPC_STATUS_OK: return DRNUTILS_STATUS_OK;
        case COMCORE::RPC_STATUS_NOT_FOUND: return DRNUTILS_STATUS_NOT_FOUND;
        case COMCORE::RPC_STATUS_INVALID_ARGUMENT: return DRNUTILS_STATUS_INVALID_ARGUMENT;
        case COMCORE::RPC_STATUS_FAILED_PRECONDITION: return DRNUTILS_STATUS_FAILED_PRECONDITION;
        case COMCORE::RPC_STATUS_ABORTED: return DRNUTILS_STATUS_ABORTED;
        case COMCORE::RPC_STATUS_UNIMPLEMENTED: return DRNUTILS_STATUS_UNIMPLEMENTED;
        case COMCORE::RPC_STATUS_RESOURCE_EXHAUSTED: return DRNUTILS_STATUS_RESOURCE_EXHAUSTED;
        case COMCORE::RPC_STATUS_PERMISSION_DENIED: return DRNUTILS_STATUS_PERMISSION_DENIED;
        case COMCORE::RPC_STATUS_UNAUTHENTICATED: return DRNUTILS_STATUS_PERMISSION_DENIED;
        case COMCORE::RPC_STATUS_UNAVAILABLE: return DRNUTILS_STATUS_UNAVAILABLE;
        case COMCORE::RPC_STATUS_DEADLINE_EXCEEDED: return DRNUTILS_STATUS_UNAVAILABLE;
        case COMCORE::RPC_STATUS_CANCELLED: return DRNUTILS_STATUS_UNAVAILABLE;
        default: return DRNUTILS_STATUS_INTERNAL;
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDrnUtilsHostSupport::JoinMessages( const CORE::CString::StringVector& messages )
{GUCEF_TRACE;

    CORE::CString joined;
    for ( UInt32 i=0; i<messages.size(); ++i )
    {
        if ( i > 0 )
            joined += "; ";
        joined += messages[ i ];
    }
    return joined;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
