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

#ifndef GUCEF_DRN_CDRNRPCSESSIONCOMPONENT_H
#define GUCEF_DRN_CDRNRPCSESSIONCOMPONENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_CIDRNSESSIONCOMPONENT_H
#include "gucefDRN_CIDRNSessionComponent.h"
#define GUCEF_DRN_CIDRNSESSIONCOMPONENT_H
#endif /* GUCEF_DRN_CIDRNSESSIONCOMPONENT_H ? */

#ifndef GUCEF_DRN_CDRNPEERLINK_H
#include "gucefDRN_CDRNPeerLink.h"
#define GUCEF_DRN_CDRNPEERLINK_H
#endif /* GUCEF_DRN_CDRNPEERLINK_H ? */

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
class CDRNRpcTransport;
class CDRNRpcTransportListener;

/**
 *  The DRN side of the "drn" RPC transport: the session component of DRN_MSGCLASS_RPC (gucefDRN_RpcTransport_Design.md
 *  4). It keeps the RPC channels of the session per member, hands each received frame to its CDRNRpcTransport, offers
 *  a frame of a channel a member opened to the session's CDRNRpcTransportListener, and tells the transports when a
 *  member leaves, the session closes or a link goes down.
 *
 *  One per session, made on first use by a transport or listener (Obtain()) and registered with the session; it lives
 *  as long as the session and goes with it (OnSessionDestroyed()). Everything runs under the node lock: the node
 *  thread holds it while it delivers, the transports and listeners take it for their calls.
 */
class GUCEF_DRN_EXPORT_CPP CDRNRpcSessionComponent : public CIDRNSessionComponent
{
    public:

    /**
     *  The RPC component of the session, made and registered when there is none.
     *  NULL when another component holds DRN_MSGCLASS_RPC in the session. Takes the node lock.
     */
    static CDRNRpcSessionComponent* Obtain( CDRNSession& session );

    CDRNSession& GetSession( void ) const;

    CDRNNode& GetNode( void ) const;

    /** A transport opens a channel to a member; returns the channel number (from 1). The transport sends the OPEN */
    UInt32 OpenChannel( UInt32 peerId, CDRNRpcTransport* transport );

    /** A channel the member opened, taken by a listener */
    void AddAcceptedChannel( UInt32 peerId, UInt32 channel, CDRNRpcTransport* transport );

    /** Forgets the transport's channel */
    void RemoveTransport( CDRNRpcTransport* transport );

    void AddListener( CDRNRpcTransportListener* listener );

    void RemoveListener( CDRNRpcTransportListener* listener );

    /** Channels a member opened to this side that are still open */
    UInt32 GetAcceptedChannelCount( UInt32 peerId ) const;

    /** True while a received frame is being delivered: the transports leave the flush of their link to the component */
    bool IsDelivering( void ) const;

    /** A transport sent over the link during a delivery: flushed when the delivery ends */
    void FlushAfterDelivery( CDRNPeerLink& link );

    virtual UInt8 GetMessageClass( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnSessionMessage( CDRNSession& session, CDRNPeerLink& link, UInt32 fromPeerId, const CDRNLinkMessage& message ) GUCEF_VIRTUAL_OVERRIDE;

    /** MAC packets carry the frames of PUBLIC methods; the RPC session judges each frame against its method */
    virtual bool MayAcceptPacketKind( const CDRNLinkMessage& message ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnSessionTick( CDRNSession& session, UInt64 nodeNow ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnMemberLeft( CDRNSession& session, UInt32 peerId ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnSessionClosed( CDRNSession& session ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnSessionDestroyed( CDRNSession& session ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    /** ( peerId, channel, opened by this side ) as one key */
    static UInt64 ChannelKey( UInt32 peerId, UInt32 channel, bool openedLocally );

    typedef GUCEF::map< UInt64, CDRNRpcTransport* > TChannelMap;
    typedef GUCEF::map< UInt32, UInt32 > TPeerCounterMap;
    typedef GUCEF::vector< CDRNRpcTransportListener* > TListenerVector;
    typedef GUCEF::vector< CDRNPeerLink* > TLinkVector;

    explicit CDRNRpcSessionComponent( CDRNSession& session );
    virtual ~CDRNRpcSessionComponent();
    CDRNRpcSessionComponent( const CDRNRpcSessionComponent& src );              /**< not implemented */
    CDRNRpcSessionComponent& operator=( const CDRNRpcSessionComponent& src );   /**< not implemented */

    void Accept( CDRNPeerLink& link, UInt32 fromPeerId, UInt32 channel );
    void SendClose( CDRNPeerLink& link, UInt32 channel, bool fromAcceptor );
    void EndDelivery( void );
    void DisconnectAll( UInt32 peerId, bool allPeers, Int32 statusCode, const char* why );

    CDRNSession* m_session;
    CDRNNode* m_node;
    TChannelMap m_channels;
    TPeerCounterMap m_nextChannel;
    TListenerVector m_listeners;
    UInt32 m_deliveryDepth;
    TLinkVector m_linksToFlush;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNRPCSESSIONCOMPONENT_H ? */
