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

#ifndef GUCEF_DRNUTILS_CREMOTESESSIONCACHE_H
#define GUCEF_DRNUTILS_CREMOTESESSIONCACHE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESESSION_H
#include "gucefDRNUTILS_CRemoteSession.h"
#define GUCEF_DRNUTILS_CREMOTESESSION_H
#endif /* GUCEF_DRNUTILS_CREMOTESESSION_H ? */

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

/**
 *  The client sessions of a remote service host: ACTIVE while bound to a control channel, SUSPENDED for the resume grace
 *  after that went without Goodbye, then CLOSED and forgotten. Bounds: maxSessions over all states and
 *  maxSessionsPerDevice (a Hello beyond either evicts the oldest suspended session, of the device first, else it is
 *  refused), and a byte budget over the sessions (their event rings and what the extensions report) that evicts
 *  suspended sessions, the oldest suspension first. An active session is never evicted. In memory only: a restart of
 *  the service closes every session. Thread safe (one mutex): the DRN access policy and the RPC protocol threads ask it.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CRemoteSessionCache
{
    public:

    struct GUCEF_DRNUTILS_PUBLIC_CPP SLimits
    {
        UInt32 maxSessions;             /**< default 64 */
        UInt32 maxSessionsPerDevice;    /**< default 4 */
        UInt32 maxAttachedPerSession;   /**< bulk channels per session, default 2 */
        UInt64 budgetBytes;             /**< default 256 MiB, 0 = no budget */
        UInt64 resumeGraceMs;           /**< default 600000 */
        UInt32 eventRetentionEvents;    /**< per session, default 4096 */
        UInt64 eventRetentionBytes;     /**< per session, default 2 MiB */

        SLimits( void );
    };

    CRemoteSessionCache( void );
    ~CRemoteSessionCache();

    void SetLimits( const SLimits& limits );
    SLimits GetLimits( void ) const;

    /**
     *  A new ACTIVE session of the device with a new id and resume secret, bound to the RPC session. Makes room by
     *  evicting suspended sessions (returned in 'evicted', CLOSED already); NULL with the detail code in 'refusal'
     *  (SESSION_LIMIT, or NO_RANDOMNESS) when there is no room
     */
    CRemoteSessionPtr Open( const CORE::CString& deviceId, const CORE::CString& userId, const CORE::CString& deviceKind,
                            const UInt8* rpcSessionId, UInt32 peerId, UInt64 nowUtcMs, UInt64 nowMs,
                            TRemoteSessionVector& evicted, CORE::CString& refusal );

    CRemoteSessionPtr Find( const CORE::CString& sessionId ) const;

    /** The session whose control channel or one of whose attached channels is the RPC session; NULL for none */
    CRemoteSessionPtr FindByRpcSession( const UInt8* rpcSessionId, bool* isAttached = GUCEF_NULL ) const;

    /** Binds the session's control channel to an RPC session and member: ACTIVE */
    void Bind( const CRemoteSessionPtr& session, const UInt8* rpcSessionId, UInt32 peerId, UInt64 nowMs );

    /** Attaches an RPC session as a bulk channel; false (CHANNEL_LIMIT) beyond maxAttachedPerSession. Its index from 1 */
    bool Attach( const CRemoteSessionPtr& session, const UInt8* rpcSessionId, UInt32& channelIndex );

    /** Detaches an RPC session from whatever session it is attached to; true when it was */
    bool Detach( const UInt8* rpcSessionId );

    /** The control channel went: SUSPENDED from now, unbound, its attached channels detached */
    void Suspend( const CRemoteSessionPtr& session, UInt64 nowMs );

    /** CLOSED and forgotten */
    void Close( const CRemoteSessionPtr& session );

    /** Suspended sessions whose grace is over: CLOSED, forgotten, returned */
    void ExpireSuspended( UInt64 nowMs, TRemoteSessionVector& expired );

    /** Over the budget: suspended sessions, the oldest suspension first, CLOSED and forgotten until it fits */
    void EnforceBudget( UInt64 usedBytes, TRemoteSessionVector& evicted );

    void GetSessions( TRemoteSessionVector& sessions ) const;
    void GetSessionsOfDevice( const CORE::CString& deviceId, TRemoteSessionVector& sessions ) const;
    UInt32 GetCount( void ) const;
    UInt32 GetActiveCount( void ) const;
    UInt32 GetSuspendedCount( void ) const;
    UInt32 GetAttachedCount( void ) const;

    /** Closes every session (the host stops); returned for their extensions */
    void CloseAll( TRemoteSessionVector& closed );

    private:

    typedef GUCEF::map< CORE::CString, CRemoteSessionPtr > TSessionMap;

    CRemoteSessionCache( const CRemoteSessionCache& src );              /**< not implemented */
    CRemoteSessionCache& operator=( const CRemoteSessionCache& src );   /**< not implemented */

    bool EvictOldestSuspendedLocked( const CORE::CString& deviceId, TRemoteSessionVector& evicted );
    void ForgetLocked( const CRemoteSessionPtr& session );

    mutable MT::CMutex m_lock;
    SLimits m_limits;
    TSessionMap m_sessions;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CREMOTESESSIONCACHE_H ? */
