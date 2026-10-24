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

#ifndef GUCEF_DRN_CDRNPEERLINK_H
#define GUCEF_DRN_CDRNPEERLINK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_DRN_CIDRNCHANNEL_H
#include "gucefDRN_CIDRNChannel.h"
#define GUCEF_DRN_CIDRNCHANNEL_H
#endif /* GUCEF_DRN_CIDRNCHANNEL_H ? */

#ifndef GUCEF_DRN_CDRNLINKRELIABILITY_H
#include "gucefDRN_CDRNLinkReliability.h"
#define GUCEF_DRN_CDRNLINKRELIABILITY_H
#endif /* GUCEF_DRN_CDRNLINKRELIABILITY_H ? */

#ifndef GUCEF_DRN_CDRNLATENCYMETER_H
#include "gucefDRN_CDRNLatencyMeter.h"
#define GUCEF_DRN_CDRNLATENCYMETER_H
#endif /* GUCEF_DRN_CDRNLATENCYMETER_H ? */

#ifndef GUCEF_DRN_CIDRNAUTHENTICATOR_H
#include "gucefDRN_CIDRNAuthenticator.h"
#define GUCEF_DRN_CIDRNAUTHENTICATOR_H
#endif /* GUCEF_DRN_CIDRNAUTHENTICATOR_H ? */

#ifndef GUCEF_DRN_CDRNLINKMONITOR_H
#include "gucefDRN_CDRNLinkMonitor.h"
#define GUCEF_DRN_CDRNLINKMONITOR_H
#endif /* GUCEF_DRN_CDRNLINKMONITOR_H ? */

#ifndef GUCEF_DRN_CDRNPACKETPROTECTION_H
#include "gucefDRN_CDRNPacketProtection.h"
#define GUCEF_DRN_CDRNPACKETPROTECTION_H
#endif /* GUCEF_DRN_CDRNPACKETPROTECTION_H ? */

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

class CIDRNBanList;
class CDRNSession;

/*-------------------------------------------------------------------------*/

/**
 *  The settings a link works with, filled by the node from its CDRNNodeSettings
 */
class GUCEF_DRN_EXPORT_CPP CDRNLinkSettings
{
    public:

    CORE::CString nodeId;
    CORE::CString serviceName;
    UInt32 handshakeTimeoutMs;          /**< per handshake step */
    UInt32 linkTimeoutMs;               /**< nothing received for this long closes the link */
    UInt32 echoIntervalMs;              /**< an ECHO goes out when nothing else was sent for this long */
    UInt32 latencyUpdateIntervalMs;     /**< interval of LatencyUpdateEvent */
    UInt32 ackDelayMs;                  /**< datagram channels: longest wait before an ack-only packet */
    UInt32 closeGraceMs;                /**< time for the last packets (CLOSE, REFUSE) to leave before the channel closes */
    UInt32 firstSequence;               /**< sequence of the first packet, never 0 */
    UInt32 linkId;                      /**< id the accepting node gives the link, sent in WELCOME */
    CDRNPolicyLimits limits;            /**< of the node's security policy */
    CDRNAnomalyThresholds anomalies;    /**< of the node's security policy */
    CIDRNBanList* banList;              /**< consulted when a peer authenticated, not owned, may be NULL */
    UInt8 policyHash[ DRN_HANDSHAKE_HASH_SIZE ];    /**< of the node's security policy, compared in the handshake */
    bool requireEncryption;             /**< of the node's security policy */
    bool requireTokenPolicyHash;        /**< of the node's security policy */
    CORE::CString::StringVector ciphers;    /**< of the node's security policy, most wanted first */
    bool haveStaticKey;                 /**< the node has a static x25519 key: it can accept encrypted links */
    UInt8 staticPrivateKey[ DRN_HANDSHAKE_KEY_SIZE ];
    UInt8 staticPublicKey[ DRN_HANDSHAKE_KEY_SIZE ];
    UInt64 rekeyPackets;                /**< packet protection rekeys after this many packets, 0 = 2^31 */
    UInt64 rekeyBytes;                  /**< ... or bytes, 0 = 64 GB */

    CDRNLinkSettings( void );
};

/*-------------------------------------------------------------------------*/

/** Event data of LinkClosedEvent and LinkRefusedEvent */
class GUCEF_DRN_EXPORT_CPP CDRNLinkClosedInfo
{
    public:

    TDRNLinkReason reason;
    CORE::CString text;
    bool wasOperational;

    CDRNLinkClosedInfo( void );
};

typedef CORE::CTCloneableObj< CDRNLinkClosedInfo > CDRNLinkClosedEventData;

/*-------------------------------------------------------------------------*/

/** Event data of MessageReceivedEvent: the payload is linked to the packet during the notification */
class GUCEF_DRN_EXPORT_CPP CDRNLinkMessage
{
    public:

    UInt8 lane;
    UInt8 msgClass;
    UInt32 opcode;
    CORE::CDynamicBuffer payload;

    CDRNLinkMessage( void );
};

typedef CORE::CTCloneableObj< CDRNLinkMessage > CDRNLinkMessageEventData;

/*-------------------------------------------------------------------------*/

/**
 *  One authenticated link over one channel.
 *
 *  Handshake (every step with a timeout): HELLO, CHALLENGE, AUTH, WELCOME or REFUSE. After WELCOME the link is
 *  operational (LinkOperationalEvent) and carries messages on the four lanes. On channels whose transport is not
 *  reliable the link does the reliability of the reliable lanes itself (CDRNLinkReliability). Latency is measured
 *  from the packet headers (CDRNLatencyMeter) unless the channel supplies a meter.
 *
 *  The link is driven by Tick() (the node calls it every pulse): timeouts, resends, echoes, latency events and
 *  flushing the messages queued with Send(). Events are sent from the thread that runs the node.
 *  A link is not deleted from within its own notifications.
 */
class GUCEF_DRN_EXPORT_CPP CDRNPeerLink : public CORE::CObservingNotifier                           ,
                                          public CORE::CTSharedObjCreator< CDRNPeerLink, MT::CMutex > ,
                                          private CIDRNLinkMessageSink
{
    public:

    static const CORE::CEvent LinkOperationalEvent;
    static const CORE::CEvent LinkClosedEvent;             /**< CDRNLinkClosedEventData */
    static const CORE::CEvent LinkRefusedEvent;            /**< CDRNLinkClosedEventData */
    static const CORE::CEvent MessageReceivedEvent;        /**< CDRNLinkMessageEventData */
    static const CORE::CEvent LatencyUpdateEvent;          /**< CDRNLatencyInfoEventData */
    static const CORE::CEvent PeerLimitExceededEvent;      /**< CDRNLinkAnomalyEventData: the peer went over a receive limit, what was over it was dropped or held back */
    static const CORE::CEvent ClockAnomalyEvent;           /**< CDRNLinkAnomalyEventData: the peer's clock runs faster or slower than ours */
    static const CORE::CEvent TrafficAnomalyEvent;         /**< CDRNLinkAnomalyEventData: silence then a burst, or a flood */

    static void RegisterEvents( void );

    enum TLinkState
    {
        LINKSTATE_IDLE              ,
        LINKSTATE_CONNECTING        ,   /**< connecting side: the channel is connecting */
        LINKSTATE_HELLO_SENT        ,   /**< connecting side: waiting for CHALLENGE */
        LINKSTATE_AUTH_SENT         ,   /**< connecting side: waiting for WELCOME or REFUSE */
        LINKSTATE_AWAITING_HELLO    ,   /**< accepting side */
        LINKSTATE_CHALLENGE_SENT    ,   /**< accepting side: waiting for AUTH */
        LINKSTATE_OPERATIONAL       ,
        LINKSTATE_CLOSING           ,   /**< the last packets are leaving */
        LINKSTATE_CLOSED
    };

    typedef CORE::CTSharedPtr< CDRNPeerLink, MT::CMutex > CDRNPeerLinkPtr;

    CDRNPeerLink( void );

    virtual ~CDRNPeerLink();

    /** Connecting side: connects the channel (if it is not connected yet) and starts the handshake */
    bool StartConnect( const CIDRNChannelPtr& channel              ,
                       const CDRNEndpoint& remote                  ,
                       const CDRNLinkSettings& settings            ,
                       const CIDRNAuthenticatorPtr& authenticator  );

    /** Accepting side: waits for the HELLO of the peer on an accepted channel */
    bool StartAccept( const CIDRNChannelPtr& channel              ,
                      const CDRNLinkSettings& settings            ,
                      const CIDRNAuthenticatorPtr& authenticator  );

    /** Queues a message, it goes out with the next flush */
    bool Send( UInt8 lane, UInt8 msgClass, UInt32 opcode, const void* payload, UInt32 payloadSize );

    bool Send( UInt8 lane, UInt8 msgClass, UInt32 opcode, const CORE::CDynamicBuffer& payload );

    /** Queues a message whose payload is part1 followed by part2, without joining them first */
    bool SendGather( UInt8 lane, UInt8 msgClass, UInt32 opcode, const void* part1, UInt32 size1, const void* part2, UInt32 size2 );

    /** Sends what is queued now */
    void Flush( void );

    /** Sends CLOSE and closes once it had time to leave, LinkClosedEvent follows */
    void Close( TDRNLinkReason reason = DRN_LINKREASON_CLOSED_BY_APP, const CORE::CString& text = CORE::CString::Empty );

    /** Timeouts, resends, echoes, latency events and the flush, nodeNow being CDRNClock time */
    void Tick( UInt64 nodeNow );

    TLinkState GetState( void ) const;
    bool IsOperational( void ) const;
    bool IsClosed( void ) const;
    bool IsInitiator( void ) const;
    TDRNLinkReason GetCloseReason( void ) const;

    void GetLatencyInfo( CDRNLatencyInfo& info ) const;

    /** Microseconds of the local link clock */
    UInt64 GetLinkTime( void ) const;

    /** When a packet of the peer arrived last, in microseconds of CDRNClock */
    UInt64 GetLastReceiveTime( void ) const;

    /** The node clock time at which the local link clock started */
    UInt64 GetLocalLinkEpoch( void ) const;

    /** The peer's node clock time at which its link clock started, as it told us */
    UInt64 GetRemoteLinkEpoch( void ) const;

    /** Estimates the peer's node clock for a local node clock time, using the clock offset of the link */
    UInt64 GetEstimatedRemoteTime( UInt64 localNodeTime ) const;

    const CDRNLinkStats& GetStats( void ) const;

    /** The sequence the next packet will carry */
    UInt32 GetNextSequence( void ) const;

    /** Reliable messages sent and not yet acknowledged (datagram channels) */
    UInt32 GetUnackedCount( void ) const;

    /** The largest message payload that goes out in one packet of the channel, without fragments */
    UInt32 GetMaxUnfragmentedPayload( void ) const;

    const CORE::CString& GetRemoteNodeId( void ) const;

    UInt32 GetLinkId( void ) const;

    const CDRNAuthResult& GetAuthResult( void ) const;

    CIDRNChannelPtr GetChannel( void ) const;

    CDRNEndpoint GetRemoteEndpoint( void ) const;

    /**
     *  The counters of what the peer sent, as attributes of the node: rates, arrival intervals and their jitter,
     *  drops, limit hits, anomalies, loss and round trip times
     */
    void GetTelemetry( CORE::CDataNode& telemetry ) const;

    /** The receive limits, telemetry counters and anomaly checks of the link */
    const CDRNLinkMonitor& GetMonitor( void ) const;

    /** Stream channels: packets held back because the peer went over a receive limit */
    UInt32 GetDeferredPacketCount( void ) const;

    /** The reaper that destroys the channel on the node's thread when the link goes away on another thread */
    void SetChannelReaper( const CDRNChannelReaperPtr& reaper );

    /** The handshake agreed on a cipher: packets are encrypted and authenticated (CDRNPacketProtection) */
    bool IsProtected( void ) const;

    const CDRNPacketProtection& GetProtection( void ) const;

    /** Connecting side: the static key the acceptor proved in the handshake, NULL when it did not */
    const UInt8* GetPeerStaticKey( void ) const;

    /** The hash over HELLO and CHALLENGE the keys and the AUTH proof are bound to */
    const UInt8* GetTranscriptHash( void ) const;

    /** Protected packets that did not authenticate, dropped */
    UInt64 GetAuthenticationFailureCount( void ) const;

    /** Plain packets dropped because the link is protected */
    UInt64 GetDroppedPlainPacketCount( void ) const;

    /** The session this link belongs to and the peer id of the other end in it, set by CDRNSession */
    void SetSessionBinding( CDRNSession* session, UInt32 peerId );
    CDRNSession* GetSession( void ) const;
    UInt32 GetSessionPeerId( void ) const;

    CDRNPeerLinkPtr GetSharedPtr( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventid               ,
                           CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CDRNPeerLink( const CDRNPeerLink& src );
    CDRNPeerLink& operator=( const CDRNPeerLink& src );

    virtual bool OnLinkMessage( const CDRNMessageView& message ) GUCEF_VIRTUAL_OVERRIDE;

    bool Start( const CIDRNChannelPtr& channel, const CDRNLinkSettings& settings, const CIDRNAuthenticatorPtr& authenticator );
    void OnChannelConnected( void );
    void OnPacket( const CORE::CDynamicBuffer& packet );
    bool HandleLinkMessage( const CDRNMessageView& message );
    void Refuse( TDRNLinkReason reason, const CORE::CString& text );
    void CloseNow( TDRNLinkReason reason, const CORE::CString& text );
    void SetState( TLinkState state );
    void FlushImp( bool forceAck );
    UInt64 GetRetransmitTimeout( void ) const;
    void SendHello( void );
    void ProcessReceivedPacket( const UInt8* data, UInt32 size, UInt64 now );
    void DeferPacket( const UInt8* data, UInt32 size, bool wasProtected );
    void ProcessDeferredPackets( UInt64 now );
    void ReportLimit( TDRNLinkAnomalyKind kind, Float64 measured, Float64 threshold, const CORE::CString& text, UInt64 now );
    void ReportAnomaly( const CORE::CEvent& eventid, const CDRNLinkAnomaly& anomaly );
    bool CanEncrypt( void ) const;
    void ComputeTranscriptHash( void );
    bool DeriveKeys( const UInt8* sharedEphemeral, const UInt8* sharedStatic, const CORE::CString& cipher );
    void WipeHandshakeSecrets( void );

    TLinkState m_state;
    bool m_initiator;
    CIDRNChannelPtr m_channel;
    CDRNChannelReaperPtr m_channelReaper;
    CIDRNAuthenticatorPtr m_authenticator;
    CDRNLinkSettings m_settings;
    CDRNLinkReliability m_reliability;
    CDRNLatencyMeter m_ownMeter;
    CIDRNLatencyMeter* m_meter;
    CDRNEndpoint m_remoteEndpoint;

    UInt64 m_linkEpoch;                 /**< node clock time at which the link clock started */
    UInt64 m_remoteLinkEpoch;
    UInt64 m_stateDeadline;             /**< node clock time a handshake step must be done by */
    UInt64 m_closeDeadline;
    UInt64 m_lastReceiveTime;           /**< node clock */
    UInt64 m_lastSendTime;              /**< node clock */
    UInt64 m_lastLatencyEventTime;      /**< node clock */
    TDRNLinkReason m_closeReason;
    CORE::CString m_closeText;
    bool m_wasOperational;

    CORE::CString m_remoteNodeId;
    UInt32 m_linkId;
    UInt8 m_nonce[ DRN_CHALLENGE_NONCE_SIZE ];
    CDRNAuthResult m_authResult;

    CDRNSession* m_session;
    UInt32 m_sessionPeerId;

    CDRNLinkMessageEventData m_rxMessage;
    CORE::CDynamicBuffer m_txScratch;
    CORE::CDynamicBuffer m_cryptBuffer;

    CDRNLinkMonitor m_monitor;
    GUCEF::vector< CORE::CDynamicBuffer > m_deferred;      /**< stream channels: packets held back for the receive limits */
    UInt32 m_deferredHead;
    UInt64 m_deferredBytes;
    CORE::CDynamicBuffer m_deferredScratch;
    GUCEF::vector< UInt8 > m_deferredProtected;

    // handshake 2 and packet protection
    CDRNPacketProtection m_protection;
    UInt8 m_ephemeralPrivate[ DRN_HANDSHAKE_KEY_SIZE ];
    UInt8 m_ephemeralPublic[ DRN_HANDSHAKE_KEY_SIZE ];
    bool m_haveEphemeral;
    CORE::CDynamicBuffer m_helloPayload;
    CORE::CDynamicBuffer m_challengePayload;
    UInt8 m_transcriptHash[ DRN_HANDSHAKE_HASH_SIZE ];
    UInt8 m_pinnedKey[ DRN_HANDSHAKE_KEY_SIZE ];
    bool m_havePinnedKey;
    UInt8 m_peerStaticKey[ DRN_HANDSHAKE_KEY_SIZE ];
    bool m_havePeerStaticKey;
    CORE::CString::StringVector m_offeredCiphers;
    bool m_currentPacketProtected;
    CORE::CDynamicBuffer m_protectBuffer;
    UInt64 m_authFailures;
    UInt64 m_droppedPlainPackets;
};

/*-------------------------------------------------------------------------*/

typedef CDRNPeerLink::CDRNPeerLinkPtr                   CDRNPeerLinkPtr;
typedef GUCEF::vector< CDRNPeerLinkPtr >                TDRNPeerLinkVector;
typedef CORE::CTCloneableObj< CDRNPeerLinkPtr >         CDRNPeerLinkEventData;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNPEERLINK_H ? */
