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

#include <string.h>

#include "gucefDRNUTILS_CRemoteSession.h"

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

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

CRemoteSession::CRemoteSession( void )
    : CORE::CTSharedObjCreator< CRemoteSession, MT::CMutex >( this )
    , m_sessionId()
    , m_deviceId()
    , m_userId()
    , m_deviceKind()
    , m_clientKind()
    , m_clientVersion()
    , m_capabilities()
    , m_state( REMOTESESSION_ACTIVE )
    , m_openedUtcMs( 0 )
    , m_suspendedAtMs( 0 )
    , m_lastUsedMs( 0 )
    , m_hasBinding( false )
    , m_peerId( 0 )
    , m_attached()
    , m_ring()
    , m_eventsCall()
    , m_nextToSend( 0 )
{GUCEF_TRACE;

    memset( m_resumeSecret, 0, sizeof( m_resumeSecret ) );
    memset( m_rpcSessionId, 0, sizeof( m_rpcSessionId ) );
}

/*-------------------------------------------------------------------------*/

CRemoteSession::~CRemoteSession()
{GUCEF_TRACE;

    memset( m_resumeSecret, 0, sizeof( m_resumeSecret ) );
}

/*-------------------------------------------------------------------------*/

const CORE::CString& CRemoteSession::GetSessionId( void ) const { return m_sessionId; }
const CORE::CString& CRemoteSession::GetDeviceId( void ) const { return m_deviceId; }
const CORE::CString& CRemoteSession::GetUserId( void ) const { return m_userId; }
const CORE::CString& CRemoteSession::GetDeviceKind( void ) const { return m_deviceKind; }
const CORE::CString& CRemoteSession::GetClientKind( void ) const { return m_clientKind; }
const CORE::CString& CRemoteSession::GetClientVersion( void ) const { return m_clientVersion; }
const CORE::CString::StringSet& CRemoteSession::GetCapabilities( void ) const { return m_capabilities; }
TRemoteSessionState CRemoteSession::GetState( void ) const { return m_state; }
UInt64 CRemoteSession::GetOpenedUtcMs( void ) const { return m_openedUtcMs; }
UInt64 CRemoteSession::GetSuspendedAtMs( void ) const { return m_suspendedAtMs; }
bool CRemoteSession::HasBinding( void ) const { return m_hasBinding; }
UInt32 CRemoteSession::GetAttachedCount( void ) const { return (UInt32) m_attached.size(); }
UInt32 CRemoteSession::GetPeerId( void ) const { return m_peerId; }
const CRemoteSessionEventRing& CRemoteSession::GetEventRing( void ) const { return m_ring; }

/*-------------------------------------------------------------------------*/

bool
CRemoteSession::IsResumeSecret( const void* secret, UInt32 size ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == secret || ResumeSecretSize != size )
        return false;
    return CORE::CSha256::ConstantTimeEquals( secret, m_resumeSecret, ResumeSecretSize );
}

/*-------------------------------------------------------------------------*/

bool
CRemoteSession::IsBoundTo( const UInt8* rpcSessionId ) const
{GUCEF_TRACE;

    return m_hasBinding && GUCEF_NULL != rpcSessionId && 0 == memcmp( m_rpcSessionId, rpcSessionId, RpcSessionIdSize );
}

/*-------------------------------------------------------------------------*/

bool
CRemoteSession::IsAttached( const UInt8* rpcSessionId ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == rpcSessionId )
        return false;
    CORE::CString key = ToKey( rpcSessionId );
    for ( UInt32 i=0; i<m_attached.size(); ++i )
    {
        if ( m_attached[ i ] == key )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteSession::ToKey( const UInt8* rpcSessionId )
{GUCEF_TRACE;

    return DRN::CDRNCrypto::ToHex( rpcSessionId, RpcSessionIdSize );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
