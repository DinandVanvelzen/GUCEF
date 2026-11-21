/*
 *  vfspluginDRN: GUCEF VFS plugin presenting a share of another DRN node as a VFS archive
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

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGECONNECTION_H
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGECONNECTION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_DRN_CDRNSESSION_H
#include "gucefDRN_CDRNSession.h"
#define GUCEF_DRN_CDRNSESSION_H
#endif /* GUCEF_DRN_CDRNSESSION_H ? */

#ifndef GUCEF_DRN_CDRNNAMESPACECLIENT_H
#include "gucefDRN_CDRNNamespaceClient.h"
#define GUCEF_DRN_CDRNNAMESPACECLIENT_H
#endif /* GUCEF_DRN_CDRNNAMESPACECLIENT_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCEHANDLE_H
#include "gucefDRN_CDRNResourceHandle.h"
#define GUCEF_DRN_CDRNRESOURCEHANDLE_H
#endif /* GUCEF_DRN_CDRNRESOURCEHANDLE_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGERESULT_H
#include "vfspluginDRN_CDrnBridgeResult.h"
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGERESULT_H
#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGERESULT_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEMEMBERLINK_H
#include "vfspluginDRN_CDrnBridgeMemberLink.h"
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEMEMBERLINK_H
#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEMEMBERLINK_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace DRNBRIDGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What a bridge archive needs of the application: a DRN session and the member of that session that serves the
 *  shares. The application made the session (discovery, connecting and authentication are its affair, the bridge
 *  does none of them) and hands it over here, as a session pointer plus the member, so that the plugin does not
 *  depend on the service kit that may have made it (DRNUTILS::CRemoteServiceConnection::GetDrnSession() and
 *  GetServicePeerId() give both).
 *
 *  The member is either a fixed peer id, or a name that is resolved against the session each time it is used:
 *  "sequencer" for the host of a client-server session, or the node id of a member. Peer ids belong to one run of a
 *  session, so after the application left and joined the session again a named member is found again where a fixed
 *  peer id may be another member (or none).
 *
 *  All calls are blocking and are made on the calling thread. A call from inside work of the node (a handler it
 *  delivers, on the node's own thread or on the thread that pumps a node without one) is refused at once with
 *  DRN_NSRESULT_WRONG_THREAD, logged the first time, and nothing is pumped. The thread that pumps a node without a
 *  thread of its own (CDRNNode::IsPumpThread()), outside work of the node, works the node itself while its call waits.
 *  The calls may be made from several threads at once. Several archives (several shares) can use one connection.
 *
 *  The session has to outlive the archives that use the connection, and the calls under way: the connection notices
 *  that the session is gone (everything then answers "unavailable") but cannot save a call that is in flight from it.
 *
 *  The connection counts the calls it makes (GetStatCallCount() and the like), which tests and diagnostics use.
 *
 *  The namespace client, its request budget and the handle budget of the member are not the connection's: they belong to
 *  the member link (CDrnBridgeMemberLink) that the global hands out per (session, member), so every connection and every
 *  mount of the same member shares them. A call that the serving side refuses as BUSY (a limit of its own that the bridge
 *  could not know, or that others use up) is tried again a bounded number of times with a growing wait, when the caller asks
 *  for it (busyRetries and busyBackoffMs of CDrnBridgeSettings), before the result is handed back.
 */
class GUCEF_VFSPLUGIN_DRN_EXPORT_CPP CDrnBridgeConnection : public CORE::CObserver
{
    public:

    /** The member name that stands for the host of a client-server session */
    static const CORE::CString SequencerMemberName;

    CDrnBridgeConnection( DRN::CDRNSession& session, UInt32 peerId );

    CDrnBridgeConnection( DRN::CDRNSession& session, const CORE::CString& member );

    /**
     *  A connection to the member of a session that is named, not held: the session is looked up in the DRN session registry
     *  (CDRNGlobal::GetSessionRegistry()) by its name each time the connection is used. The session need not exist yet (every call then
     *  answers "cannot be reached" until it is registered), and when the session that was there is destroyed and another one of the same
     *  name is registered the connection follows it, without being made again. The form of "drn://<session name>/<member>/<share>".
     */
    CDrnBridgeConnection( const CORE::CString& sessionName, const CORE::CString& member );

    virtual ~CDrnBridgeConnection() GUCEF_VIRTUAL_OVERRIDE;

    /** The id this connection was registered under (CDrnBridgeGlobal::RegisterConnection()), empty when it was not */
    CORE::CString GetId( void ) const;

    void SetId( const CORE::CString& id );

    /**
     *  The session, NULL once it was destroyed. A connection that names its session finds it in the registry now: NULL while no
     *  session of the name is registered, and the session of the name that is registered now when it is another one than before
     */
    DRN::CDRNSession* GetSession( void ) const;

    /** Whether the session is found by its name in the registry each time (see the constructor that takes a name) */
    bool IsBoundByName( void ) const;

    /** The name of the session as it was when the connection was made, for messages */
    const CORE::CString& GetSessionName( void ) const;

    /** The member as it was given: a peer id as text, or the name */
    const CORE::CString& GetMemberDescription( void ) const;

    /** The peer id of the serving member now; false when the session is gone or the member is not in it (any more) */
    bool ResolvePeerId( UInt32& peerId ) const;

    /** The session exists, is active, the member is in it and there is an operational direct link to it */
    bool IsConnected( void ) const;

    /** Whether the calling thread works on the node of the session: blocking calls refuse there */
    bool IsOnNodeThread( void ) const;

    /** Namespace calls of the shares of the member, as CDRNNamespaceClient makes them, with the results in the form of the bridge */
    CDrnBridgeResult Stat( const CORE::CString& share, const CORE::CString::StringVector& paths, const CORE::CString::StringVector& knownRevisions,
                           DRN::CDRNNamespaceClient::TEntryVector& results, UInt32 timeoutInMs, UInt32 busyRetries = 0, UInt32 busyBackoffMs = 0 );

    CDrnBridgeResult List( const CORE::CString& share, const CORE::CString& dir, const CORE::CString& cursor, UInt32 maxEntries,
                           const CORE::CString& ifRevisionNot, DRN::CDRNNamespaceListPage& page, UInt32 timeoutInMs, UInt32 busyRetries = 0, UInt32 busyBackoffMs = 0 );

    CDrnBridgeResult ListAll( const CORE::CString& share, const CORE::CString& dir, DRN::CDRNNamespaceClient::TEntryVector& entries, CORE::CString& dirRevision,
                              bool& changedDuringListing, UInt32 maxTotalEntries, UInt32 pageEntries, UInt32 timeoutInMs, UInt32 busyRetries = 0, UInt32 busyBackoffMs = 0 );

    /**
     *  Opens a file of a share as a handle and waits until the serving side answered. 'options' carries the revision the file
     *  has to have (expectedRevision), empty for any. On failure the handle is NULL and the result says why.
     */
    CDrnBridgeResult OpenHandle( const CORE::CString& share, const CORE::CString& path, const DRN::CDRNResourceTransferOptions& options,
                                 UInt32 timeoutInMs, DRN::CDRNResourceHandlePtr& handle, UInt32 busyRetries = 0, UInt32 busyBackoffMs = 0 );

    /** Calls made through this connection since it was made */
    UInt64 GetStatCallCount( void ) const;
    UInt64 GetListCallCount( void ) const;
    UInt64 GetOpenCallCount( void ) const;

    /** Times a call of this connection that the serving side refused as BUSY was tried again */
    UInt64 GetBusyRetryCount( void ) const;

    /**
     *  The member link this connection uses: the one client, request budget and handle budget it shares with every other
     *  connection and mount of the member. NULL when the session is gone. Made, or found, when first asked.
     */
    CDrnBridgeMemberLinkPtr GetMemberLink( void );

    /**
     *  A handle pool (one per mount) takes part in the division of the member's handle limit for as long as it is registered;
     *  the connection holds the member link while a pool is registered, and lets go of it with the last one
     */
    void RegisterHandleUser( const CDrnBridgeHandlePool* user );
    void UnregisterHandleUser( const CDrnBridgeHandlePool* user );

    /**
     *  The handles the pool may hold open: its share of what the member announced, 0 when nothing is announced. Asks the
     *  member (once) when it was not asked yet and the link is up.
     */
    UInt32 GetHandleShare( const CDrnBridgeHandlePool* user, UInt32 timeoutInMs );

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CDrnBridgeMemberLink::TClientPtr TClientPtr;

    CDrnBridgeConnection( const CDrnBridgeConnection& src );                /**< not implemented */
    CDrnBridgeConnection& operator=( const CDrnBridgeConnection& src );     /**< not implemented */

    void Initialize( void );

    /** The session now: the one the connection holds, or for a connection by name the registered one, which is taken over when it is another one */
    DRN::CDRNSession* SyncSession( void ) const;

    /** The state of the session needed for a call, with the reason in 'result' when there is none: the call must not be made then */
    bool Prepare( DRN::CDRNSession*& session, UInt32& peerId, CDrnBridgeResult& result ) const;

    bool ResolvePeerIdOfSession( DRN::CDRNSession& session, UInt32& peerId ) const;

    /** The one client of the member (the member link's), made when there is none or the member is another peer now */
    TClientPtr ObtainClient( DRN::CDRNSession& session, UInt32 peerId, UInt32 timeoutInMs );

    /** Whether a result is a refusal of the serving side for a limit it keeps, which another try may get past */
    static bool IsBusy( const CDrnBridgeResult& result );

    /** Waits before a retry; on the thread that pumps the node the node is worked meanwhile */
    void BackOff( UInt32 attempt, UInt32 backoffMs );

    CDrnBridgeResult Unreachable( const CORE::CString& why ) const;

    CDrnBridgeResult WrongThread( void ) const;

    mutable MT::CMutex m_lock;
    DRN::CDRNSession* m_session;
    CORE::CString m_sessionName;
    UInt32 m_fixedPeerId;
    CORE::CString m_member;
    CORE::CString m_memberDescription;
    CORE::CString m_id;
    CDrnBridgeMemberLinkPtr m_link;
    bool m_byName;
    mutable MT::CMutex m_bindLock;      /**< taken to change the session of a connection by name, before m_lock */
    typedef GUCEF::vector< const CDrnBridgeHandlePool* > THandlePoolVector;
    THandlePoolVector m_handlePools;    /**< the pools registered as users of the member link, registered again with the link of a new session */
    mutable bool m_loggedWrongThread;
    MT::CTAtomic< UInt64 > m_statCalls;
    MT::CTAtomic< UInt64 > m_listCalls;
    MT::CTAtomic< UInt64 > m_openCalls;
    MT::CTAtomic< UInt64 > m_busyRetries;
};

typedef CORE::CTSharedPtr< CDrnBridgeConnection, MT::CMutex > CDrnBridgeConnectionPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNBRIDGE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGECONNECTION_H ? */
