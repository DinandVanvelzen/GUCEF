/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_DRN_CDRNRPCTRANSPORT_H
#define GUCEF_DRN_CDRNRPCTRANSPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_COMCORE_CIRPCTRANSPORT_H
#include "gucefCOMCORE_CIRpcTransport.h"
#define GUCEF_COMCORE_CIRPCTRANSPORT_H
#endif /* GUCEF_COMCORE_CIRPCTRANSPORT_H ? */

#ifndef GUCEF_DRN_CDRNPEERLINK_H
#include "gucefDRN_CDRNPeerLink.h"
#define GUCEF_DRN_CDRNPEERLINK_H
#endif /* GUCEF_DRN_CDRNPEERLINK_H ? */

#ifndef GUCEF_DRN_CDRNRPCTRANSPORTSETTINGS_H
#include "gucefDRN_CDRNRpcTransportSettings.h"
#define GUCEF_DRN_CDRNRPCTRANSPORTSETTINGS_H
#endif /* GUCEF_DRN_CDRNRPCTRANSPORTSETTINGS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CDRNSession;
class CDRNNode;
class CDRNRpcSessionComponent;

/**
 *  The RPC transport "drn" (gucefDRN_RpcTransport_Design.md): an RPC channel between two members of a DRN session,
 *  carried as DRN_MSGCLASS_RPC messages on lane 0 of their direct link.
 *
 *  Connect( "drn://<session name>/<peer id>" ) with the session registered in CDRNGlobal::GetSessionRegistry() (also
 *  through CRpcEndpoint::Connect()), or Connect( CORE::CUri() ) on a transport made for a session and member directly.
 *  Connect() opens the channel (DRN_RPC_OPCODE_OPEN): the member's listener accepts it or it is closed again. The member needs a direct, operational
 *  link of protocol 2.1 or later; without one Connect() fails at once (GetConnectStatus(): UNAVAILABLE). Transports of
 *  channels a member opens come from CDRNRpcTransportListener.
 *
 *  Threading (gucefCOMCORE_Rpc_Design.md 10.9): the protocol thread is the node's (CDRNNode::GetPulseGenerator()),
 *  SetPulseGenerator() returns false. Frames are received and every event is raised on it, synchronously, while the
 *  node lock is held (the transport is pulsed by the node's own pulse generator, CDRNNode::GetWorkPulseGenerator()):
 *  node lock, then the notifier's lock (Lock()), then the endpoint's lock, then this transport's leaf lock. Observers may subscribe from any thread. SendFrame() on the node
 *  thread sends at once; from another thread it queues under the leaf lock and the node sends it: a node thread of
 *  its own in a cycle that the call starts (CDRNNode::WakeOwnThread()), else the application's next pulse. Close() is followed by DisconnectedEvent on the node thread.
 *
 *  Protection: a PUBLIC frame goes in a MAC packet where the node's security policy allows public items, everything
 *  else in AEAD packets; FrameReceivedEvent reports PUBLIC for a frame that came in a MAC packet.
 *
 *  Principal: from the authentication of the member's link; subject "drn:<userId>" (or "drn-node:<node id>" without
 *  a user id), roles from the comma separated role of the authentication, attributes drn.session, drn.peerId,
 *  drn.nodeId, drn.publicKey (hex), drn.accounts, drn.protected, drn.cipher. Authenticated: on the accepting side when the
 *  node's authenticator is not anonymous; on the connecting side when the endpoint pinned the acceptor's static key
 *  (CDRNPeerLink::HasPinnedKey()), since without a pin the key and the node id are only the acceptor's own word.
 *  Shared pointers to a transport made by CDRNRpcTransportListener use the counter in the object
 *  (CTSharedObjCreator); one made by the transport factory is owned by the factory's pointer.
 */
class GUCEF_DRN_EXPORT_CPP CDRNRpcTransport : public COMCORE::CIRpcTransport ,
                                              public CORE::CTSharedObjCreator< CDRNRpcTransport, MT::CMutex >
{
    public:

    static const CORE::CString TransportType;       /**< "drn" */
    static const CORE::CString UriScheme;           /**< "drn" */

    /** For the transport factory: Connect() names the session and member */
    CDRNRpcTransport( void );

    /** A transport to a member of a session the caller holds; Connect( CORE::CUri() ) connects it */
    CDRNRpcTransport( CDRNSession& session, UInt32 peerId, const CDRNRpcTransportSettings& settings = CDRNRpcTransportSettings() );

    virtual ~CDRNRpcTransport() GUCEF_VIRTUAL_OVERRIDE;

    /** Before Connect() */
    void SetSettings( const CDRNRpcTransportSettings& settings );

    const CDRNRpcTransportSettings& GetSettings( void ) const;

    /** Why the last Connect() failed: UNAVAILABLE (no direct link, the member's node has no RPC), PERMISSION_DENIED, ... */
    COMCORE::CRpcStatus GetConnectStatus( void ) const;

    /** The member at the other end */
    UInt32 GetPeerId( void ) const;

    /** The channel number between the two members */
    UInt32 GetChannel( void ) const;

    virtual const CORE::CString& GetTransportType( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Connect( const CORE::CUri& remote ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SendFrame( const void* frame, UInt32 size, COMCORE::TRpcProtectionClass protection ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const COMCORE::CRpcTransportProperties& GetProperties( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const COMCORE::CRpcPrincipal& GetPeerPrincipal( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::PulseGeneratorPtr GetPulseGenerator( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CUri GetRemoteUri( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The node's thread is the protocol thread: always false */
    virtual bool SetPulseGenerator( const CORE::PulseGeneratorPtr& pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;

    /** The RPC session refused a frame for its protection class: counted and reported by the member link (ProtectionAnomalyEvent) */
    virtual void OnProtectionDowngradeRefused( COMCORE::TRpcProtectionClass required, COMCORE::TRpcProtectionClass arrivedWith ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  The notifier's lock: an endpoint subscribes on the thread that starts it while the node thread raises the
     *  events, which it does holding this lock. Lock order: node lock, this lock, the endpoint's lock, the leaf lock
     */
    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The principal and properties a transport over this link reports (also used by the listener's checks) */
    static void DescribeLink( const CDRNSession& session, UInt32 peerId, const CDRNPeerLink& link, const CDRNRpcTransportSettings& settings,
                              COMCORE::CRpcPrincipal& principal, COMCORE::CRpcTransportProperties& properties );

    private:

    friend class CDRNRpcSessionComponent;
    friend class CDRNRpcTransportListener;

    typedef CORE::CTEventHandlerFunctor< CDRNRpcTransport > TEventCallback;
    typedef GUCEF::vector< CORE::CDynamicBuffer* > TFrameVector;
    typedef GUCEF::vector< UInt8 > TProtectionVector;

    // called by the component, node lock held, on the node thread
    void Attach( CDRNRpcSessionComponent& component, const CDRNPeerLinkPtr& link, UInt32 peerId, UInt32 channel, bool openedLocally );
    void OnFrame( CDRNPeerLink& link, const CDRNLinkMessage& message );
    void OnLinkGone( Int32 statusCode, const CORE::CString& why );
    void OnComponentGone( void );
    bool IsOpenedLocally( void ) const;

    bool IsOnNodeThread( void ) const;
    bool ConnectTo( CDRNSession& session, UInt32 peerId );
    void Fail( Int32 statusCode, const CORE::CString& why );
    void SubscribeToPulses( CDRNNode& node );
    void Drain( void );
    bool SendNow( const UInt8* frame, UInt32 size, UInt8 protection );
    void FlushLink( void );
    void Detach( bool sendClose );
    void OnPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void RaiseDisconnected( void );

    CDRNRpcTransport( const CDRNRpcTransport& src );                  /**< not implemented */
    CDRNRpcTransport& operator=( const CDRNRpcTransport& src );       /**< not implemented */

    mutable MT::CMutex m_notifierLock;     /**< see Lock() */

    // the leaf lock: the queue and the state flags; nothing is called out while it is held
    mutable MT::CMutex m_lock;
    TFrameVector m_queue;
    TProtectionVector m_queueProtection;
    TFrameVector m_free;
    TFrameVector m_sending;                 /**< node thread only */
    TProtectionVector m_sendingProtection;  /**< node thread only */
    bool m_connected;
    bool m_closed;
    bool m_closeRequested;                  /**< Close() asked for, done on the node thread */
    bool m_disconnectPending;               /**< DisconnectedEvent still to raise */
    bool m_connectedPending;                /**< ConnectedEvent still to raise */
    bool m_disconnectRaised;
    COMCORE::CRpcStatus m_disconnectStatus;
    COMCORE::CRpcStatus m_connectStatus;

    // set by Connect() / Attach(), read on the node thread or under the node lock
    CDRNRpcTransportSettings m_settings;
    CDRNSession* m_session;
    CDRNNode* m_node;
    CDRNRpcSessionComponent* m_component;
    CDRNPeerLinkPtr m_link;
    UInt32 m_peerId;
    UInt32 m_channel;
    bool m_openedLocally;
    UInt32 m_sendOpcode;
    CORE::PulseGeneratorPtr m_pulseGenerator;
    CORE::CObserver m_pulseObserver;
    COMCORE::CRpcTransportProperties m_properties;
    COMCORE::CRpcPrincipal m_principal;
    CORE::CUri m_remoteUri;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNRPCTRANSPORT_H ? */
