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

#ifndef GUCEF_DRN_CDRNNODE_H
#define GUCEF_DRN_CDRNNODE_H

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

#ifndef GUCEF_DRN_CDRNNODESETTINGS_H
#include "gucefDRN_CDRNNodeSettings.h"
#define GUCEF_DRN_CDRNNODESETTINGS_H
#endif /* GUCEF_DRN_CDRNNODESETTINGS_H ? */

#ifndef GUCEF_DRN_CIDRNCHANNELLISTENER_H
#include "gucefDRN_CIDRNChannelListener.h"
#define GUCEF_DRN_CIDRNCHANNELLISTENER_H
#endif /* GUCEF_DRN_CIDRNCHANNELLISTENER_H ? */

#ifndef GUCEF_DRN_CDRNPEERLINK_H
#include "gucefDRN_CDRNPeerLink.h"
#define GUCEF_DRN_CDRNPEERLINK_H
#endif /* GUCEF_DRN_CDRNPEERLINK_H ? */

#ifndef GUCEF_DRN_CDRNCONNECTOR_H
#include "gucefDRN_CDRNConnector.h"
#define GUCEF_DRN_CDRNCONNECTOR_H
#endif /* GUCEF_DRN_CDRNCONNECTOR_H ? */

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
class CIDRNAccessPolicy;
class CIDRNBanList;
class CDRNNodeThread;
class CDRNRelayClient;
class CDRNRelayService;

/*-------------------------------------------------------------------------*/

/** Event data of the link events of the node */
class GUCEF_DRN_EXPORT_CPP CDRNNodeLinkInfo
{
    public:

    CDRNPeerLinkPtr link;
    CDRNLinkClosedInfo closedInfo;      /**< LinkClosedEvent, LinkRefusedEvent, ConnectFailedEvent */
    CDRNLatencyInfo latencyInfo;        /**< LatencyUpdateEvent */
    CDRNLinkAnomaly anomaly;            /**< PeerLimitExceededEvent, ClockAnomalyEvent, TrafficAnomalyEvent */

    CDRNNodeLinkInfo( void );
};

typedef CORE::CTCloneableObj< CDRNNodeLinkInfo > CDRNNodeLinkEventData;

/*-------------------------------------------------------------------------*/

/**
 *  A DRN node: owns the channel listeners, the links, the connectors and the sessions.
 *
 *  Everything runs on the pulse generator of the node: the global one by default, or with
 *  CDRNNodeSettings::ownThread a pulse generator of its own driven by a thread of its own. The node's events
 *  (and those of its sessions) reach the application on the application's pulse generator in both cases: with
 *  a thread of its own they are mailed over, as CORE::CTSGNotifier does. The node's own objects (links,
 *  channels, connectors) send their events from the node's thread; with ownThread observe the node and the
 *  sessions instead of the links. Calls into the node and its sessions are thread-safe: they take the node lock,
 *  which the node's thread holds while it works.
 */
class GUCEF_DRN_EXPORT_CPP CDRNNode : public CORE::CTSGNotifier
{
    public:

    static const CORE::CEvent LinkOperationalEvent;        /**< CDRNNodeLinkEventData */
    static const CORE::CEvent LinkClosedEvent;             /**< CDRNNodeLinkEventData */
    static const CORE::CEvent LinkRefusedEvent;            /**< CDRNNodeLinkEventData */
    static const CORE::CEvent LatencyUpdateEvent;          /**< CDRNNodeLinkEventData */
    static const CORE::CEvent RelayBoundEvent;             /**< CORE::TCloneableString: the relay endpoint of the node */
    static const CORE::CEvent RelayBindingLostEvent;       /**< CORE::TCloneableString: why */
    static const CORE::CEvent PeerLimitExceededEvent;      /**< CDRNNodeLinkEventData: see CDRNPeerLink */
    static const CORE::CEvent ClockAnomalyEvent;           /**< CDRNNodeLinkEventData: see CDRNPeerLink */
    static const CORE::CEvent TrafficAnomalyEvent;         /**< CDRNNodeLinkEventData: see CDRNPeerLink */

    static void RegisterEvents( void );

    typedef GUCEF::vector< CDRNSession* > TSessionVector;

    /** A node delivering its events on the global pulse generator */
    CDRNNode( void );

    /** A node delivering its events on the given pulse generator */
    explicit CDRNNode( const CORE::PulseGeneratorPtr& appPulseGenerator );

    virtual ~CDRNNode();

    /**
     *  Takes the settings. The threading choice can only change while the node has no listeners and no links.
     *  Fails if the authenticator is not one the security policy allows or the node id is empty.
     */
    bool SetSettings( const CDRNNodeSettings& settings );

    const CDRNNodeSettings& GetSettings( void ) const;

    /** Listens on the listen endpoints of the settings, fails if the security policy cannot be honoured */
    bool Start( void );

    /** Closes the sessions, the links and the listeners and stops the thread of the node */
    void Stop( void );

    bool Listen( const CDRNEndpoint& local );

    void StopListening( void );

    /** The endpoints the node listens on, followed by its relay endpoints */
    CDRNEndpointList GetListenEndpoints( void ) const;

    /**
     *  Binds at the relay service reached at relayService (udp://host:port for instance) in a realm, as the given id
     *  or one the relay hands out. Once bound (RelayBoundEvent) the relay endpoint is one of the node's endpoints and
     *  peers reach the node through the relay. The node's relayToken goes with the binding unless one is given.
     *  The link to the relay service runs under the policy the relay advertises in parameter "policy" of the endpoint
     *  (GetPublishedEndpoints() of the relay node), so a game with a policy of its own can use any relay; the links
     *  through the relay run end to end under the game's policy. Without the parameter the node's own policy is used.
     *  A "key" parameter pins the relay's static key. Both go into the node's relay endpoints (relayPolicy,
     *  relayKey) for the peers that connect through it.
     */
    bool BindAtRelay( const CDRNEndpoint& relayService, const CORE::CString& realm, UInt32 requestedId = 0, const CORE::CString* token = GUCEF_NULL );

    bool IsBoundAtRelay( const CDRNEndpoint& relayService, const CORE::CString& realm ) const;

    /** The relay endpoints the node is bound under */
    CDRNEndpointList GetRelayEndpoints( void ) const;

    /** The public half of the node's static x25519 key, false when the node has none (no key agreement codec) */
    bool GetPublicKey( UInt8* publicKey ) const;

    /** The public key in hex, empty when there is none */
    CORE::CString GetPublicKeyString( void ) const;

    /**
     *  The advertised endpoints with the node's public key as parameter "key" (a peer that connects to such an
     *  endpoint refuses a node that cannot prove that key: pinning) and the hash of its security policy as "policy"
     *  (what a node binding at this node's relay service links under, see BindAtRelay())
     */
    CDRNEndpointList GetPublishedEndpoints( void ) const;

    /** What the node tells others to reach it at (CDRNReachabilityManager sets it), empty = its listen endpoints */
    void SetAdvertisedEndpoints( const CDRNEndpointList& endpoints );

    /** The advertised endpoints, else the listen endpoints */
    CDRNEndpointList GetAdvertisedEndpoints( void ) const;

    /** The relay service of the node, NULL unless relayEnabled */
    CDRNRelayService* GetRelayService( void ) const;

    /** A channel to the endpoint: of the channel factory, or a relay channel for relay endpoints */
    CIDRNChannelPtr CreateChannel( const CDRNEndpoint& remote );

    /** Connects through a connector, the node's authenticator settings are used if credentials are not given */
    CDRNConnectorPtr Connect( const CDRNEndpointList& remote, const CDRNAuthSettings* credentials = GUCEF_NULL );

    void GetLinks( TDRNPeerLinkVector& links ) const;

    UInt32 GetLinkCount( bool operationalOnly = false ) const;

    /** Sessions are owned by the node */
    CDRNSession* CreateSession( void );

    void DestroySession( CDRNSession* session );

    const TSessionVector& GetSessions( void ) const;

    /** The hosted session with the given name, NULL if there is none */
    CDRNSession* FindSession( const CORE::CString& sessionName ) const;

    /** Not owned, NULL restores the default policy */
    void SetAccessPolicy( CIDRNAccessPolicy* policy );

    CIDRNAccessPolicy& GetAccessPolicy( void );

    /** Not owned, NULL for none. Consulted when a channel is accepted (address) and when a link authenticated (user id). */
    void SetBanList( CIDRNBanList* banList );

    CIDRNBanList* GetBanList( void ) const;

    /** Node counters: channels refused at accept, by reason, and the links in their handshake */
    void GetTelemetry( CORE::CDataNode& telemetry ) const;

    /** Accepted channels closed right away: policy, ban, pending handshakes, handshake rate */
    UInt32 GetRefusedAcceptCount( void ) const;

    /** Accepted links that did not finish their handshake yet */
    UInt32 GetPendingHandshakeCount( void ) const;

    /** The pulse generator the node works on */
    CORE::PulseGeneratorPtr GetPulseGenerator( void ) const;

    /** The pulse generator the application receives the events on */
    CORE::PulseGeneratorPtr GetAppPulseGenerator( void ) const;

    bool IsRunningOwnThread( void ) const;

    /** The node lock, held by the node's thread while it works */
    MT::CMutex& GetLock( void ) const;

    /** CDRNClock time in microseconds */
    UInt64 GetNodeTime( void ) const;

    const CDRNLinkSettings& GetLinkSettings( void ) const;

    /** Creates a link owned by the node, used by connectors */
    CDRNPeerLinkPtr CreateLink( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    friend class CDRNNodeThread;
    friend class CDRNSession;
    friend class CDRNRelayClient;
    friend class CDRNReachabilityManager;

    class CInternalObserver : public CORE::CObserver
    {
        public:
        CDRNNode* m_node;
        CInternalObserver( void ) : CORE::CObserver(), m_node( GUCEF_NULL ) {}
        virtual ~CInternalObserver() { SignalUpcomingObserverDestruction(); }
        protected:
        virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;
    };
    friend class CInternalObserver;

    typedef GUCEF::vector< CIDRNChannelListenerPtr > TListenerVector;
    typedef GUCEF::vector< CDRNConnectorPtr > TConnectorVector;

    class CHandshakeWindow
    {
        public:
        UInt64 windowStart;
        UInt32 count;
        CHandshakeWindow( void ) : windowStart( 0 ), count( 0 ) {}
    };
    typedef GUCEF::map< CORE::CString, CHandshakeWindow > THandshakeWindowMap;

    CDRNNode( const CDRNNode& src );
    CDRNNode& operator=( const CDRNNode& src );

    void Construct( void );
    void OnInternalNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata );
    void OnPulse( void );
    void OnChannelAccepted( const CIDRNChannelPtr& channel );
    void OnLinkMessage( CDRNPeerLink* link, const CDRNLinkMessage& message );
    void OnLinkClosed( CDRNPeerLink* link, const CDRNLinkClosedInfo& info );
    void OnRelayBindingChanged( const CDRNEndpoint& endpoint, bool bound, const CORE::CString& reason );
    bool MayAccept( const CIDRNChannelPtr& channel, UInt64 nodeNow );
    bool TakeHandshakeSlot( const CORE::CString& address, UInt64 nodeNow );
    void PurgeHandshakeWindows( UInt64 nodeNow );
    void ForwardLinkEvent( const CORE::CEvent& nodeEvent, CDRNPeerLink* link, CORE::CICloneable* eventdata );
    static CORE::CString GetAddressKey( const CDRNEndpoint& remote );
    void UpdateLinkSettings( void );
    CDRNConnectorPtr ConnectToService( const CDRNEndpointList& remote, const UInt8* servicePolicyHash );
    bool EnsureStaticKey( void );
    bool StartOwnThread( void );
    void StopOwnThread( void );
    void RunThreadCycle( void );

    CDRNNodeSettings m_settings;
    CDRNLinkSettings m_linkSettings;
    CIDRNAuthenticatorPtr m_authenticator;
    CDRNChannelReaperPtr m_channelReaper;
    CORE::PulseGeneratorPtr m_appPulseGenerator;
    CORE::PulseGeneratorPtr m_pulseGenerator;
    CDRNNodeThread* m_thread;
    CDRNRelayClient* m_relayClient;
    CDRNRelayService* m_relayService;
    CDRNEndpointList m_advertisedEndpoints;
    CInternalObserver m_observer;
    TListenerVector m_listeners;
    TDRNPeerLinkVector m_links;
    TDRNPeerLinkVector m_closedLinks;
    TConnectorVector m_connectors;
    TSessionVector m_sessions;
    CIDRNAccessPolicy* m_accessPolicy;
    CIDRNAccessPolicy* m_defaultAccessPolicy;
    UInt32 m_nextLinkId;
    bool m_stopping;
    CIDRNBanList* m_banList;
    bool m_haveStaticKey;
    UInt8 m_staticPrivateKey[ DRN_HANDSHAKE_KEY_SIZE ];
    UInt8 m_staticPublicKey[ DRN_HANDSHAKE_KEY_SIZE ];
    THandshakeWindowMap m_handshakeWindows;
    UInt64 m_nextWindowPurge;
    UInt32 m_refusedForPolicy;
    UInt32 m_refusedForBan;
    UInt32 m_refusedForPending;
    UInt32 m_refusedForRate;
    mutable MT::CMutex m_nodeLock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNNODE_H ? */
