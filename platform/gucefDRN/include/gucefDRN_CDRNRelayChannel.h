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

#ifndef GUCEF_DRN_CDRNRELAYCHANNEL_H
#define GUCEF_DRN_CDRNRELAYCHANNEL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_CIDRNCHANNEL_H
#include "gucefDRN_CIDRNChannel.h"
#define GUCEF_DRN_CIDRNCHANNEL_H
#endif /* GUCEF_DRN_CIDRNCHANNEL_H ? */

#ifndef GUCEF_DRN_CDRNCONNECTOR_H
#include "gucefDRN_CDRNConnector.h"
#define GUCEF_DRN_CDRNCONNECTOR_H
#endif /* GUCEF_DRN_CDRNCONNECTOR_H ? */

#ifndef GUCEF_DRN_CDRNENDPOINTLIST_H
#include "gucefDRN_CDRNEndpointList.h"
#define GUCEF_DRN_CDRNENDPOINTLIST_H
#endif /* GUCEF_DRN_CDRNENDPOINTLIST_H ? */

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
class CDRNRelayClient;
class CDRNRelayBinding;

/**
 *  A channel whose packets travel as FORWARD messages over a link to a relay service. To the peer link on top of it
 *  it is a datagram channel like any other: the link does its own reliability end to end.
 *  Endpoint: relay://<relay host>:<relay port>/<realm>/<peer id>?via=<channel type to the relay, default udp>, with
 *  relayPolicy (the relay's policy hash: the link to the relay runs under it) and relayKey (pins the relay's key).
 *  Made by the node for relay endpoints (the channel needs the node's relay client), not by the channel factory.
 */
class GUCEF_DRN_EXPORT_CPP CDRNRelayChannel : public CIDRNChannel
{
    public:

    explicit CDRNRelayChannel( CDRNRelayClient& client );

    virtual ~CDRNRelayChannel();

    virtual bool Connect( const CDRNEndpoint& remote ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SendPacket( const void* data, UInt32 size ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CDRNChannelCapabilities& GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CDRNEndpoint GetRemoteEndpoint( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CString GetChannelType( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void SetPulseGenerator( CORE::PulseGeneratorPtr pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::PulseGeneratorPtr GetPulseGenerator( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The id of the other end in the realm */
    UInt32 GetRemotePeerId( void ) const;

    private:

    friend class CDRNRelayClient;
    friend class CDRNRelayBinding;

    CDRNRelayChannel( const CDRNRelayChannel& src );
    CDRNRelayChannel& operator=( const CDRNRelayChannel& src );

    void OnBindingReady( UInt32 maxPacketSize );
    void OnBindingGone( void );
    void OnPacket( const UInt8* data, UInt32 size );
    void Tick( void );

    CDRNRelayClient* m_client;
    CDRNRelayBinding* m_binding;
    CDRNChannelCapabilities m_capabilities;
    CORE::PulseGeneratorPtr m_pulseGenerator;
    CDRNEndpoint m_remote;
    UInt32 m_remotePeerId;
    bool m_connected;
    bool m_announceConnected;
    bool m_announceDisconnected;
    CORE::CDynamicBuffer m_rxView;
    CDRNPacketEventData m_rxEventData;
};

/*-------------------------------------------------------------------------*/

/** A link to a relay service with a binding in one realm, and the relay channels that use it */
class GUCEF_DRN_EXPORT_CPP CDRNRelayBinding
{
    public:

    enum TBindingState
    {
        BINDINGSTATE_CONNECTING ,
        BINDINGSTATE_BINDING    ,
        BINDINGSTATE_BOUND      ,
        BINDINGSTATE_FAILED
    };

    CORE::CString relayHost;
    UInt16 relayPort;
    CORE::CString via;              /**< channel type to the relay */
    CORE::CString realm;
    CORE::CString token;
    CORE::CString relayPolicy;          /**< hex of the relay's policy hash, empty: the node's policy */
    CORE::CString relayKey;             /**< hex of the relay's static key, empty: not pinned */
    UInt32 requestedPeerId;
    UInt32 boundPeerId;
    bool advertise;                 /**< the node offers its relay endpoint as one of its own */
    TBindingState state;
    CORE::CString failure;
    CDRNConnectorPtr connector;
    CDRNPeerLinkPtr link;
    UInt32 maxPacketSize;

    CDRNRelayBinding( void );

    /** relay://host:port/realm/<bound peer id>?via=... */
    CDRNEndpoint GetEndpoint( void ) const;

    private:

    friend class CDRNRelayClient;
    friend class CDRNRelayChannel;

    typedef GUCEF::vector< CDRNRelayChannel* > TChannelVector;

    TChannelVector m_channelsByPeerId;
    TChannelVector m_waiting;           /**< connecting channels until the binding is bound */
    UInt8 m_header[ 8 ];
};

/*-------------------------------------------------------------------------*/

/**
 *  The node's side of relaying: bindings at relay services, the relay channels over them. Packets that arrive for a
 *  peer id without a channel are a new connection: the client makes an accepting relay channel for it and hands it
 *  to the node as a listener would. Run by the node, on its thread.
 */
class GUCEF_DRN_EXPORT_CPP CDRNRelayClient
{
    public:

    explicit CDRNRelayClient( CDRNNode& node );

    ~CDRNRelayClient();

    /**
     *  Binds at the relay service reached at the given endpoint (udp://host:port, tcp://...) in a realm, as the
     *  given peer id or (0) one the relay hands out. Once bound, the relay endpoint is one of the node's endpoints.
     */
    bool Bind( const CDRNEndpoint& relayService, const CORE::CString& realm, UInt32 requestedPeerId, const CORE::CString& token );

    /** The relay endpoints of the bindings that are bound and offered */
    void GetBoundEndpoints( CDRNEndpointList& endpoints ) const;

    bool IsBound( const CDRNEndpoint& relayService, const CORE::CString& realm ) const;

    /** A channel to a relay endpoint */
    CIDRNChannelPtr CreateChannel( const CDRNEndpoint& remote );

    /** RELAY messages from the links to relays, false if the link is not one of those */
    bool OnLinkMessage( CDRNPeerLink* link, const CDRNLinkMessage& message );

    void OnLinkClosed( CDRNPeerLink* link );

    void Tick( UInt64 nodeNow );

    void Shutdown( void );

    // used by the channels

    bool Forward( CDRNRelayBinding& binding, UInt32 toPeerId, const void* data, UInt32 size );
    void Detach( CDRNRelayChannel& channel );

    private:

    friend class CDRNRelayChannel;

    typedef GUCEF::vector< CDRNRelayBinding* > TBindingVector;
    typedef GUCEF::vector< CDRNRelayChannel* > TChannelVector;

    CDRNRelayClient( const CDRNRelayClient& src );
    CDRNRelayClient& operator=( const CDRNRelayClient& src );

    CDRNRelayBinding* FindBinding( const CORE::CString& host, UInt16 port, const CORE::CString& via, const CORE::CString& realm ) const;
    CDRNRelayBinding* AddBinding( const CORE::CString& host, UInt16 port, const CORE::CString& via, const CORE::CString& realm, UInt32 requestedPeerId,
                                  const CORE::CString& token, bool advertise, const CORE::CString& relayPolicy, const CORE::CString& relayKey );
    CDRNRelayBinding* FindBindingByLink( const CDRNPeerLink* link ) const;
    void SendBind( CDRNRelayBinding& binding );
    void OnBindReply( CDRNRelayBinding& binding, CDRNWireReader& reader );
    void OnForward( CDRNRelayBinding& binding, CDRNWireReader& reader );
    void FailBinding( CDRNRelayBinding& binding, const CORE::CString& reason );
    void AttachChannel( CDRNRelayBinding& binding, CDRNRelayChannel& channel );

    CDRNNode* m_node;
    TBindingVector m_bindings;
    TChannelVector m_channels;      /**< every relay channel, for the ticks */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNRELAYCHANNEL_H ? */
