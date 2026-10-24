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

#ifndef GUCEF_DRN_CDRNSESSION_H
#define GUCEF_DRN_CDRNSESSION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_DRN_CDRNSESSIONSETTINGS_H
#include "gucefDRN_CDRNSessionSettings.h"
#define GUCEF_DRN_CDRNSESSIONSETTINGS_H
#endif /* GUCEF_DRN_CDRNSESSIONSETTINGS_H ? */

#ifndef GUCEF_DRN_CDRNCONNECTOR_H
#include "gucefDRN_CDRNConnector.h"
#define GUCEF_DRN_CDRNCONNECTOR_H
#endif /* GUCEF_DRN_CDRNCONNECTOR_H ? */

#ifndef GUCEF_DRN_CIDRNSESSIONCOMPONENT_H
#include "gucefDRN_CIDRNSessionComponent.h"
#define GUCEF_DRN_CIDRNSESSIONCOMPONENT_H
#endif /* GUCEF_DRN_CIDRNSESSIONCOMPONENT_H ? */

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

class CDRNNode;
class CDRNReplicator;
class CDRNBulkTransferManager;
class CDRNDataGrid;
class CDRNDataStream;
class CDRNDataGridSettings;
class CDRNDataStreamSettings;
class CDRNSessionRecorder;

/*-------------------------------------------------------------------------*/

/** A member of a session as the application sees it */
class GUCEF_DRN_EXPORT_CPP CDRNSessionMemberInfo
{
    public:

    UInt32 peerId;
    CORE::CString nodeId;
    CDRNEndpointList endpoints;
    bool isLocal;
    bool isSequencer;
    bool hasDirectLink;
    UInt32 reason;              /**< MemberLeftEvent: TDRNLeaveReason */

    CDRNSessionMemberInfo( void );
};

typedef CORE::CTCloneableObj< CDRNSessionMemberInfo >   CDRNSessionMemberEventData;
typedef GUCEF::vector< CDRNSessionMemberInfo >           TDRNSessionMemberInfoVector;

/*-------------------------------------------------------------------------*/

/** An application message, event data of AppMessageEvent. The payload is a copy once the event was mailed. */
class GUCEF_DRN_EXPORT_CPP CDRNAppMessage
{
    public:

    UInt32 fromPeer;
    UInt32 toPeer;              /**< 0 = to all */
    UInt8 lane;
    CORE::CDynamicBuffer payload;

    CDRNAppMessage( void );
};

typedef CORE::CTCloneableObj< CDRNAppMessage > CDRNAppMessageEventData;

/*-------------------------------------------------------------------------*/

/**
 *  A named scope of replication: one lobby, one game.
 *
 *  The topology is a setting: CLIENT_SERVER (clients link only to the server, which forwards) or PEER_TO_PEER
 *  (every member links to every member, the creator is the sequencer). The application uses the session the same
 *  way for both. Peer ids are given by the host / sequencer, which is peer 1; 0 means "none" or "all".
 *
 *  Joining: JOIN, answered by JOINED and MEMBER_LIST; the others learn about the new member with MEMBER_ADDED.
 *  In PEER_TO_PEER the host sends the joiner a CONNECT_TO for every other member and the joiner links to each.
 *  Until such a direct link exists, messages for that member travel through the sequencer.
 *
 *  Events are delivered on the application's pulse generator (see CDRNNode). Created by CDRNNode::CreateSession().
 */
class GUCEF_DRN_EXPORT_CPP CDRNSession : public CORE::CTSGNotifier
{
    public:

    static const CORE::CEvent JoinedEvent;             /**< CORE::TCloneableUInt32: the local peer id */
    static const CORE::CEvent JoinFailedEvent;         /**< CORE::TCloneableString: why */
    static const CORE::CEvent MemberJoinedEvent;       /**< CDRNSessionMemberEventData */
    static const CORE::CEvent MemberLeftEvent;         /**< CDRNSessionMemberEventData */
    static const CORE::CEvent ItemAnnouncedEvent;      /**< CDRNItemInfoEventData: another member published a grid or stream */
    static const CORE::CEvent ItemRemovedEvent;        /**< CDRNItemInfoEventData: an item of another member went away */
    static const CORE::CEvent SessionClosedEvent;      /**< CORE::TCloneableUInt32: TDRNLeaveReason */
    static const CORE::CEvent AppMessageEvent;         /**< CDRNAppMessageEventData */

    static void RegisterEvents( void );

    /** The grid in which the sequencer publishes the round trip time (UInt32 microseconds) per peer id (UInt32) */
    static const CORE::CString LatencyGridName;

    enum TSessionState
    {
        SESSIONSTATE_IDLE       ,
        SESSIONSTATE_JOINING    ,
        SESSIONSTATE_MEMBER     ,
        SESSIONSTATE_HOSTING    ,
        SESSIONSTATE_CLOSED
    };

    /** Starts the session with this node as its host / sequencer. The node should be listening. */
    bool Host( const CDRNSessionSettings& settings );

    /** Joins the session settings.sessionName hosted at one of the given endpoints, the host decides the topology */
    bool Join( const CDRNEndpointList& host, const CDRNSessionSettings& settings, const CDRNAuthSettings* credentials = GUCEF_NULL );

    /** Leaves the session; when the host leaves the session ends for everyone */
    void Leave( void );

    TSessionState GetState( void ) const;
    bool IsActive( void ) const;
    bool IsSequencer( void ) const;
    TDRNTopology GetTopology( void ) const;
    UInt32 GetLocalPeerId( void ) const;
    UInt32 GetSequencerPeerId( void ) const;
    const CDRNSessionSettings& GetSettings( void ) const;
    const CORE::CString& GetName( void ) const;

    /** The members including the local one, ordered by peer id */
    void GetMembers( TDRNSessionMemberInfoVector& members ) const;
    UInt32 GetMemberCount( void ) const;
    bool HasMember( UInt32 peerId ) const;

    /** Number of members this one has a direct link with */
    UInt32 GetDirectLinkCount( void ) const;

    /**
     *  Sends an application message to one member or (toPeer 0) to all others, on the given lane.
     *  Routed directly or through the sequencer, whatever the topology allows.
     */
    bool SendAppMessage( UInt32 toPeer, UInt8 lane, const void* data, UInt32 dataSize );

    bool SendAppMessage( UInt32 toPeer, UInt8 lane, const CORE::CDynamicBuffer& data );

    /** The sequencer's clock in microseconds since the session started, estimated on the other members */
    UInt64 GetSessionTime( void ) const;

    /** Publishes a grid of this member, see CDRNReplicator::CreateGrid() */
    CDRNDataGrid* CreateGrid( const CDRNDataGridSettings& settings );

    /** Publishes a stream of this member */
    CDRNDataStream* CreateStream( const CDRNDataStreamSettings& settings );

    /** The application wants the items of this name, of one owner or (0) of every owner */
    bool Subscribe( const CORE::CString& itemName, UInt32 ownerPeerId = 0 );

    bool Unsubscribe( const CORE::CString& itemName, UInt32 ownerPeerId = 0 );

    CDRNDataGrid* FindGrid( const CORE::CString& itemName, UInt32 ownerPeerId = 0 ) const;

    CDRNDataStream* FindStream( const CORE::CString& itemName, UInt32 ownerPeerId = 0 ) const;

    /** The grids and streams of the session */
    CDRNReplicator& GetReplicator( void ) const;

    /** Files and blobs sent to members */
    CDRNBulkTransferManager& GetBulkTransfers( void ) const;

    /** Called by the node's pulse: flushes what is due, checks the join timeout */
    void Tick( UInt64 nodeNow );

    /** Plugs a component in, not owned. One component per message class. */
    bool RegisterComponent( CIDRNSessionComponent* component );

    void UnregisterComponent( CIDRNSessionComponent* component );

    /** Sends a message over the direct link with a member, false if there is none */
    bool SendToMember( UInt32 peerId, UInt8 lane, UInt8 msgClass, UInt32 opcode, const void* part1, UInt32 size1, const void* part2 = GUCEF_NULL, UInt32 size2 = 0 );

    /** The direct link with a member, NULL if there is none */
    CDRNPeerLinkPtr GetLinkToMember( UInt32 peerId ) const;

    /** What the authentication of the member's direct link established, empty without a direct link */
    const CDRNAuthResult& GetMemberAuthResult( UInt32 peerId ) const;

    /**
     *  A snapshot of the counters per member with a direct link: a child "Member" (peerId, nodeId) per member with
     *  the link's telemetry (CDRNPeerLink::GetTelemetry())
     */
    void GetTelemetry( CORE::CDataNode& telemetry ) const;

    /** The recorder recording the session, NULL if none */
    CDRNSessionRecorder* GetRecorder( void ) const;

    CDRNNode& GetNode( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    friend class CDRNNode;
    friend class CDRNReplicator;
    friend class CDRNPeerLink;
    friend class CDRNSessionRecorder;

    class CMember
    {
        public:
        UInt32 peerId;
        CORE::CString nodeId;
        CORE::CString endpoints;
        CDRNPeerLinkPtr link;           /**< the direct link, if any */
        CDRNConnectorPtr connector;     /**< peer to peer: connecting to the member */
        bool known;                     /**< announced by the sequencer (or the sequencer itself) */
        bool isLocal;

        CMember( void );
    };

    class CInternalObserver : public CORE::CObserver
    {
        public:
        CDRNSession* m_session;
        CInternalObserver( void ) : CORE::CObserver(), m_session( GUCEF_NULL ) {}
        virtual ~CInternalObserver() { SignalUpcomingObserverDestruction(); }
        protected:
        virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;
    };
    friend class CInternalObserver;

    typedef GUCEF::vector< CMember* > TMemberVector;

    explicit CDRNSession( CDRNNode& node );
    virtual ~CDRNSession();
    CDRNSession( const CDRNSession& src );
    CDRNSession& operator=( const CDRNSession& src );

    // called by the node, on the node's thread
    void OnLinkMessage( CDRNPeerLink* link, const CDRNLinkMessage& message );
    void OnJoinRequest( CDRNPeerLink* link, const CDRNLinkMessage& message );
    void OnLinkClosed( CDRNPeerLink* link );
    void Shutdown( void );

    void OnInternalNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata );
    void OnConnectorDone( CDRNConnector* connector, bool succeeded );

    void HandleSessionMessage( CDRNPeerLink* link, const CDRNLinkMessage& message );
    void HandleAppMessage( CDRNPeerLink* link, const CDRNLinkMessage& message );
    void HandleJoined( CDRNPeerLink* link, CDRNWireReader& reader );
    void HandleMemberList( CDRNWireReader& reader );
    void HandleMemberAdded( CDRNWireReader& reader );
    void HandleMemberRemoved( CDRNWireReader& reader );
    void HandleConnectTo( CDRNWireReader& reader );
    void HandleLeave( CDRNPeerLink* link, CDRNWireReader& reader );

    CMember* GetMember( UInt32 peerId ) const;
    CMember* GetOrAddMember( UInt32 peerId );
    void RemoveMember( UInt32 peerId, UInt32 reason, bool announce );
    void BindLink( CDRNPeerLink* link, CMember* member );
    void SendJoinedReply( CDRNPeerLink* link, UInt32 result, const CORE::CString& reason, UInt32 peerId, UInt32 flags );
    void SendMemberList( CDRNPeerLink* link );
    void SendMemberAdded( CDRNPeerLink* link, const CMember& member );
    void SendMemberRemoved( CDRNPeerLink* link, UInt32 peerId, UInt32 reason );
    void SendConnectTo( CDRNPeerLink* link, const CMember& member );
    void SendJoin( CDRNPeerLink* link, UInt32 peerId, UInt32 flags );
    bool SendAppEnvelope( CDRNPeerLink* link, UInt32 fromPeer, UInt32 toPeer, UInt8 flags, UInt8 lane, const void* data, UInt32 dataSize );
    void DeliverAppMessage( UInt32 fromPeer, UInt32 toPeer, UInt8 lane, const UInt8* data, UInt32 dataSize );
    void CloseSession( UInt32 reason, bool notify );
    void FailJoin( const CORE::CString& reason );
    void Emit( const CORE::CEvent& eventid, CORE::CICloneable* eventData = GUCEF_NULL );
    void EmitMemberEvent( const CORE::CEvent& eventid, const CMember& member, UInt32 reason );
    CORE::CString GetAdvertisedEndpoints( void ) const;
    CORE::CString FilterEndpoints( const CORE::CString& endpoints ) const;
    bool IsLinkAllowed( const CDRNPeerLink& link ) const;
    void SetRecorder( CDRNSessionRecorder* recorder );

    /** Every message sent or received over a link of the session, for the recorder */
    void OnLinkTraffic( const CDRNPeerLink& link, bool outgoing, UInt8 lane, UInt8 msgClass, UInt32 opcode,
                        const void* part1, UInt32 size1, const void* part2, UInt32 size2 );
    CDRNPeerLink* GetSequencerLink( void ) const;
    void PublishLatency( UInt64 nodeNow );

    CDRNNode* m_node;
    CInternalObserver m_observer;
    CDRNSessionSettings m_settings;
    TSessionState m_state;
    TDRNTopology m_topology;
    UInt32 m_localPeerId;
    UInt32 m_sequencerPeerId;
    UInt32 m_nextPeerId;
    UInt64 m_sessionEpoch;              /**< the sequencer's node clock when the session started */
    UInt64 m_joinDeadline;
    CDRNConnectorPtr m_joinConnector;
    CDRNPeerLinkPtr m_sequencerLink;
    TMemberVector m_members;            /**< indexed by peer id */
    UInt32 m_memberCount;
    CIDRNSessionComponent* m_components[ DRN_MSGCLASS_COUNT ];
    CDRNReplicator* m_replicator;
    CDRNBulkTransferManager* m_bulkTransfers;
    CDRNDataGrid* m_latencyGrid;
    UInt64 m_nextLatencyPublish;
    CORE::CDynamicBuffer m_txBuffer;
    CDRNAppMessageEventData m_appMessageEvent;
    CDRNSessionRecorder* m_recorder;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNSESSION_H ? */
