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

#ifndef GUCEF_DRNUTILS_CREMOTESESSION_H
#define GUCEF_DRNUTILS_CREMOTESESSION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_COMCORE_CRPCSERVERCALL_H
#include "gucefCOMCORE_CRpcServerCall.h"
#define GUCEF_COMCORE_CRPCSERVERCALL_H
#endif /* GUCEF_COMCORE_CRPCSERVERCALL_H ? */

#ifndef GUCEF_DRNUTILS_REMOTESERVICETYPES_H
#include "gucefDRNUTILS_RemoteServiceTypes.h"
#define GUCEF_DRNUTILS_REMOTESERVICETYPES_H
#endif /* GUCEF_DRNUTILS_REMOTESERVICETYPES_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESESSIONEVENTRING_H
#include "gucefDRNUTILS_CRemoteSessionEventRing.h"
#define GUCEF_DRNUTILS_CREMOTESESSIONEVENTRING_H
#endif /* GUCEF_DRNUTILS_CREMOTESESSIONEVENTRING_H ? */

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

class CRemoteSessionCache;
class CRemoteServiceHost;

/**
 *  One client session of a remote service host (gucefASSETSTORE_RemoteNetworking_Design.md 4): its id and resume secret,
 *  its device, the RPC session it is bound to (the control channel) and the RPC sessions attached to it (bulk
 *  channels), and its event ring with the open Session/Events call that receives it.
 *  Owned by CRemoteSessionCache, reached through CRemoteSessionPtr. Its state is changed by the cache and the host only,
 *  under the cache's lock (bindings and state) or the host's event lock (the ring and the events call); the getters
 *  give a snapshot and may be called from any thread.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CRemoteSession : public CORE::CTSharedObjCreator< CRemoteSession, MT::CMutex >
{
    public:

    static const UInt32 ResumeSecretSize = 32;
    static const UInt32 RpcSessionIdSize = 16;

    virtual ~CRemoteSession();

    const CORE::CString& GetSessionId( void ) const;
    const CORE::CString& GetDeviceId( void ) const;
    const CORE::CString& GetUserId( void ) const;
    const CORE::CString& GetDeviceKind( void ) const;
    const CORE::CString& GetClientKind( void ) const;
    const CORE::CString& GetClientVersion( void ) const;
    const CORE::CString::StringSet& GetCapabilities( void ) const;
    TRemoteSessionState GetState( void ) const;
    UInt64 GetOpenedUtcMs( void ) const;

    /** Monotonic milliseconds of the suspension; 0 while it is not suspended */
    UInt64 GetSuspendedAtMs( void ) const;

    /** Constant time comparison of a resume secret */
    bool IsResumeSecret( const void* secret, UInt32 size ) const;

    /** Whether the RPC session (RpcSessionIdSize bytes) is this session's control channel */
    bool IsBoundTo( const UInt8* rpcSessionId ) const;

    /** Whether the RPC session is one of the attached (bulk) channels */
    bool IsAttached( const UInt8* rpcSessionId ) const;

    bool HasBinding( void ) const;
    UInt32 GetAttachedCount( void ) const;

    /** The member of the control channel's link */
    UInt32 GetPeerId( void ) const;

    /** The event ring: read it under the host's event lock (CRemoteServiceHost::GetRingState() gives a snapshot) */
    const CRemoteSessionEventRing& GetEventRing( void ) const;

    private:

    friend class CRemoteSessionCache;
    friend class CRemoteServiceHost;
    friend class CORE::CTSharedObjCreator< CRemoteSession, MT::CMutex >;

    typedef GUCEF::vector< CORE::CString > TRpcSessionVector;     /**< hexadecimal RPC session ids */

    CRemoteSession( void );
    CRemoteSession( const CRemoteSession& src );                /**< not implemented */
    CRemoteSession& operator=( const CRemoteSession& src );     /**< not implemented */

    static CORE::CString ToKey( const UInt8* rpcSessionId );

    CORE::CString m_sessionId;
    CORE::CString m_deviceId;
    CORE::CString m_userId;
    CORE::CString m_deviceKind;
    CORE::CString m_clientKind;
    CORE::CString m_clientVersion;
    CORE::CString::StringSet m_capabilities;
    UInt8 m_resumeSecret[ ResumeSecretSize ];
    TRemoteSessionState m_state;
    UInt64 m_openedUtcMs;
    UInt64 m_suspendedAtMs;
    UInt64 m_lastUsedMs;
    UInt8 m_rpcSessionId[ RpcSessionIdSize ];
    bool m_hasBinding;
    UInt32 m_peerId;
    TRpcSessionVector m_attached;
    CRemoteSessionEventRing m_ring;
    COMCORE::CRpcServerCallPtr m_eventsCall;    /**< the open Session/Events call, NULL without one */
    UInt64 m_nextToSend;                        /**< the eventSeq the events call writes next */
};

typedef CORE::CTSharedPtr< CRemoteSession, MT::CMutex > CRemoteSessionPtr;
typedef GUCEF::vector< CRemoteSessionPtr > TRemoteSessionVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CREMOTESESSION_H ? */
