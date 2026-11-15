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

#include "gucefDRNUTILS_CRemoteSessionCache.h"

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_DRNUTILS_CDEVICEPAIRINGCRYPTO_H
#include "gucefDRNUTILS_CDevicePairingCrypto.h"
#define GUCEF_DRNUTILS_CDEVICEPAIRINGCRYPTO_H
#endif /* GUCEF_DRNUTILS_CDEVICEPAIRINGCRYPTO_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESERVICEDETAILCODES_H
#include "gucefDRNUTILS_CRemoteServiceDetailCodes.h"
#define GUCEF_DRNUTILS_CREMOTESERVICEDETAILCODES_H
#endif /* GUCEF_DRNUTILS_CREMOTESERVICEDETAILCODES_H ? */

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

CRemoteSessionCache::SLimits::SLimits( void )
    : maxSessions( 64 )
    , maxSessionsPerDevice( 4 )
    , maxAttachedPerSession( 2 )
    , budgetBytes( 256 * 1024 * 1024 )
    , resumeGraceMs( 600000 )
    , eventRetentionEvents( 4096 )
    , eventRetentionBytes( 2 * 1024 * 1024 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRemoteSessionCache::CRemoteSessionCache( void )
    : m_lock()
    , m_limits()
    , m_sessions()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRemoteSessionCache::~CRemoteSessionCache()
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_sessions.clear();
}

/*-------------------------------------------------------------------------*/

void
CRemoteSessionCache::SetLimits( const SLimits& limits )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_limits = limits;
}

/*-------------------------------------------------------------------------*/

CRemoteSessionCache::SLimits
CRemoteSessionCache::GetLimits( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_limits;
}

/*-------------------------------------------------------------------------*/

CRemoteSessionPtr
CRemoteSessionCache::Open( const CORE::CString& deviceId, const CORE::CString& userId, const CORE::CString& deviceKind,
                           const UInt8* rpcSessionId, UInt32 peerId, UInt64 nowUtcMs, UInt64 nowMs,
                           TRemoteSessionVector& evicted, CORE::CString& refusal )
{GUCEF_TRACE;

    CRemoteSessionPtr session = CRemoteSession::CreateSharedObj();
    if ( session.IsNULL() || !CDevicePairingCrypto::GenerateSessionId( session->m_sessionId ) ||
         !CDevicePairingCrypto::RandomBytes( session->m_resumeSecret, CRemoteSession::ResumeSecretSize ) )
    {
        refusal = "NO_RANDOMNESS";
        return CRemoteSessionPtr();
    }
    session->m_deviceId = deviceId;
    session->m_userId = userId;
    session->m_deviceKind = deviceKind;
    session->m_openedUtcMs = nowUtcMs;
    session->m_lastUsedMs = nowMs;
    session->m_state = REMOTESESSION_ACTIVE;
    session->m_peerId = peerId;
    if ( GUCEF_NULL != rpcSessionId )
    {
        memcpy( session->m_rpcSessionId, rpcSessionId, CRemoteSession::RpcSessionIdSize );
        session->m_hasBinding = true;
    }

    MT::CScopeMutex lock( m_lock );
    session->m_ring.SetLimits( m_limits.eventRetentionEvents, m_limits.eventRetentionBytes );

    // per device first: a further Hello of a device takes the place of its oldest suspended session
    UInt32 ofDevice = 0;
    for ( TSessionMap::const_iterator i = m_sessions.begin(); i != m_sessions.end(); ++i )
    {
        if ( (*i).second->m_deviceId == deviceId )
            ++ofDevice;
    }
    if ( 0 != m_limits.maxSessionsPerDevice && ofDevice >= m_limits.maxSessionsPerDevice && !EvictOldestSuspendedLocked( deviceId, evicted ) )
    {
        refusal = CRemoteServiceDetailCodes::SessionLimit;
        return CRemoteSessionPtr();
    }
    if ( 0 != m_limits.maxSessions && m_sessions.size() >= m_limits.maxSessions && !EvictOldestSuspendedLocked( CORE::CString::Empty, evicted ) )
    {
        refusal = CRemoteServiceDetailCodes::SessionLimit;
        return CRemoteSessionPtr();
    }
    m_sessions[ session->m_sessionId ] = session;
    return session;
}

/*-------------------------------------------------------------------------*/

CRemoteSessionPtr
CRemoteSessionCache::Find( const CORE::CString& sessionId ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TSessionMap::const_iterator i = m_sessions.find( sessionId );
    if ( i == m_sessions.end() )
        return CRemoteSessionPtr();
    return (*i).second;
}

/*-------------------------------------------------------------------------*/

CRemoteSessionPtr
CRemoteSessionCache::FindByRpcSession( const UInt8* rpcSessionId, bool* isAttached ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != isAttached )
        *isAttached = false;
    if ( GUCEF_NULL == rpcSessionId )
        return CRemoteSessionPtr();
    MT::CScopeMutex lock( m_lock );
    for ( TSessionMap::const_iterator i = m_sessions.begin(); i != m_sessions.end(); ++i )
    {
        if ( (*i).second->IsBoundTo( rpcSessionId ) )
            return (*i).second;
    }
    for ( TSessionMap::const_iterator i = m_sessions.begin(); i != m_sessions.end(); ++i )
    {
        if ( (*i).second->IsAttached( rpcSessionId ) )
        {
            if ( GUCEF_NULL != isAttached )
                *isAttached = true;
            return (*i).second;
        }
    }
    return CRemoteSessionPtr();
}

/*-------------------------------------------------------------------------*/

void
CRemoteSessionCache::Bind( const CRemoteSessionPtr& sessionRef, const UInt8* rpcSessionId, UInt32 peerId, UInt64 nowMs )
{GUCEF_TRACE;

    CRemoteSessionPtr session( sessionRef );

    if ( session.IsNULL() || GUCEF_NULL == rpcSessionId )
        return;
    MT::CScopeMutex lock( m_lock );
    memcpy( session->m_rpcSessionId, rpcSessionId, CRemoteSession::RpcSessionIdSize );
    session->m_hasBinding = true;
    session->m_peerId = peerId;
    session->m_state = REMOTESESSION_ACTIVE;
    session->m_suspendedAtMs = 0;
    session->m_lastUsedMs = nowMs;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteSessionCache::Attach( const CRemoteSessionPtr& sessionRef, const UInt8* rpcSessionId, UInt32& channelIndex )
{GUCEF_TRACE;

    CRemoteSessionPtr session( sessionRef );

    channelIndex = 0;
    if ( session.IsNULL() || GUCEF_NULL == rpcSessionId )
        return false;
    MT::CScopeMutex lock( m_lock );
    CORE::CString key = CRemoteSession::ToKey( rpcSessionId );
    for ( UInt32 i=0; i<session->m_attached.size(); ++i )
    {
        if ( session->m_attached[ i ] == key )
        {
            channelIndex = i+1;
            return true;
        }
    }
    if ( 0 != m_limits.maxAttachedPerSession && session->m_attached.size() >= m_limits.maxAttachedPerSession )
        return false;
    // an RPC session is attached to one client session at a time
    for ( TSessionMap::iterator s = m_sessions.begin(); s != m_sessions.end(); ++s )
    {
        CRemoteSession::TRpcSessionVector& attached = (*s).second->m_attached;
        for ( UInt32 i=0; i<attached.size(); ++i )
        {
            if ( attached[ i ] == key )
            {
                attached.erase( attached.begin() + i );
                break;
            }
        }
    }
    session->m_attached.push_back( key );
    channelIndex = (UInt32) session->m_attached.size();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteSessionCache::Detach( const UInt8* rpcSessionId )
{GUCEF_TRACE;

    if ( GUCEF_NULL == rpcSessionId )
        return false;
    CORE::CString key = CRemoteSession::ToKey( rpcSessionId );
    MT::CScopeMutex lock( m_lock );
    for ( TSessionMap::iterator s = m_sessions.begin(); s != m_sessions.end(); ++s )
    {
        CRemoteSession::TRpcSessionVector& attached = (*s).second->m_attached;
        for ( UInt32 i=0; i<attached.size(); ++i )
        {
            if ( attached[ i ] == key )
            {
                attached.erase( attached.begin() + i );
                return true;
            }
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CRemoteSessionCache::Suspend( const CRemoteSessionPtr& sessionRef, UInt64 nowMs )
{GUCEF_TRACE;

    CRemoteSessionPtr session( sessionRef );

    if ( session.IsNULL() )
        return;
    MT::CScopeMutex lock( m_lock );
    if ( REMOTESESSION_ACTIVE != session->m_state )
        return;
    session->m_state = REMOTESESSION_SUSPENDED;
    session->m_suspendedAtMs = 0 == nowMs ? 1 : nowMs;
    session->m_hasBinding = false;
    session->m_peerId = 0;
    session->m_attached.clear();
}

/*-------------------------------------------------------------------------*/

void
CRemoteSessionCache::Close( const CRemoteSessionPtr& session )
{GUCEF_TRACE;

    if ( session.IsNULL() )
        return;
    MT::CScopeMutex lock( m_lock );
    ForgetLocked( session );
}

/*-------------------------------------------------------------------------*/

void
CRemoteSessionCache::ForgetLocked( const CRemoteSessionPtr& sessionRef )
{GUCEF_TRACE;

    CRemoteSessionPtr session( sessionRef );

    session->m_state = REMOTESESSION_CLOSED;
    session->m_hasBinding = false;
    session->m_attached.clear();
    m_sessions.erase( session->m_sessionId );
}

/*-------------------------------------------------------------------------*/

bool
CRemoteSessionCache::EvictOldestSuspendedLocked( const CORE::CString& deviceId, TRemoteSessionVector& evicted )
{GUCEF_TRACE;

    CRemoteSessionPtr oldest;
    for ( TSessionMap::const_iterator i = m_sessions.begin(); i != m_sessions.end(); ++i )
    {
        const CRemoteSessionPtr& session = (*i).second;
        if ( REMOTESESSION_SUSPENDED != session->m_state )
            continue;
        if ( !deviceId.IsNULLOrEmpty() && session->m_deviceId != deviceId )
            continue;
        if ( oldest.IsNULL() || session->m_suspendedAtMs < oldest->m_suspendedAtMs )
            oldest = session;
    }
    if ( oldest.IsNULL() )
        return false;
    ForgetLocked( oldest );
    evicted.push_back( oldest );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRemoteSessionCache::ExpireSuspended( UInt64 nowMs, TRemoteSessionVector& expired )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TRemoteSessionVector due;
    for ( TSessionMap::const_iterator i = m_sessions.begin(); i != m_sessions.end(); ++i )
    {
        const CRemoteSessionPtr& session = (*i).second;
        if ( REMOTESESSION_SUSPENDED == session->m_state && nowMs >= session->m_suspendedAtMs + m_limits.resumeGraceMs )
            due.push_back( session );
    }
    for ( UInt32 i=0; i<due.size(); ++i )
    {
        ForgetLocked( due[ i ] );
        expired.push_back( due[ i ] );
    }
}

/*-------------------------------------------------------------------------*/

void
CRemoteSessionCache::EnforceBudget( UInt64 usedBytes, TRemoteSessionVector& evicted )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( 0 == m_limits.budgetBytes )
        return;
    while ( usedBytes > m_limits.budgetBytes )
    {
        size_t before = evicted.size();
        if ( !EvictOldestSuspendedLocked( CORE::CString::Empty, evicted ) || evicted.size() == before )
            break;
        UInt64 gone = evicted.back()->m_ring.GetRetainedBytes();
        usedBytes = usedBytes > gone ? usedBytes - gone : 0;
    }
}

/*-------------------------------------------------------------------------*/

void
CRemoteSessionCache::GetSessions( TRemoteSessionVector& sessions ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( TSessionMap::const_iterator i = m_sessions.begin(); i != m_sessions.end(); ++i )
        sessions.push_back( (*i).second );
}

/*-------------------------------------------------------------------------*/

void
CRemoteSessionCache::GetSessionsOfDevice( const CORE::CString& deviceId, TRemoteSessionVector& sessions ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( TSessionMap::const_iterator i = m_sessions.begin(); i != m_sessions.end(); ++i )
    {
        if ( (*i).second->m_deviceId == deviceId )
            sessions.push_back( (*i).second );
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CRemoteSessionCache::GetCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_sessions.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CRemoteSessionCache::GetActiveCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    UInt32 count = 0;
    for ( TSessionMap::const_iterator i = m_sessions.begin(); i != m_sessions.end(); ++i )
    {
        if ( REMOTESESSION_ACTIVE == (*i).second->m_state )
            ++count;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CRemoteSessionCache::GetSuspendedCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    UInt32 count = 0;
    for ( TSessionMap::const_iterator i = m_sessions.begin(); i != m_sessions.end(); ++i )
    {
        if ( REMOTESESSION_SUSPENDED == (*i).second->m_state )
            ++count;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CRemoteSessionCache::GetAttachedCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    UInt32 count = 0;
    for ( TSessionMap::const_iterator i = m_sessions.begin(); i != m_sessions.end(); ++i )
        count += (UInt32) (*i).second->m_attached.size();
    return count;
}

/*-------------------------------------------------------------------------*/

void
CRemoteSessionCache::CloseAll( TRemoteSessionVector& closed )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( TSessionMap::iterator i = m_sessions.begin(); i != m_sessions.end(); ++i )
    {
        (*i).second->m_state = REMOTESESSION_CLOSED;
        (*i).second->m_hasBinding = false;
        (*i).second->m_attached.clear();
        closed.push_back( (*i).second );
    }
    m_sessions.clear();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
