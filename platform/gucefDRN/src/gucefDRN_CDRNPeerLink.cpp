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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_DRN_CDRNCLOCK_H
#include "gucefDRN_CDRNClock.h"
#define GUCEF_DRN_CDRNCLOCK_H
#endif /* GUCEF_DRN_CDRNCLOCK_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#ifndef GUCEF_DRN_CIDRNBANLIST_H
#include "gucefDRN_CIDRNBanList.h"
#define GUCEF_DRN_CIDRNBANLIST_H
#endif /* GUCEF_DRN_CIDRNBANLIST_H ? */

#ifndef GUCEF_DRN_CDRNSESSION_H
#include "gucefDRN_CDRNSession.h"
#define GUCEF_DRN_CDRNSESSION_H
#endif /* GUCEF_DRN_CDRNSESSION_H ? */

#include "gucefDRN_CDRNPeerLink.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CDRNPeerLink::LinkOperationalEvent = "GUCEF::DRN::CDRNPeerLink::LinkOperationalEvent";
const CORE::CEvent CDRNPeerLink::LinkClosedEvent = "GUCEF::DRN::CDRNPeerLink::LinkClosedEvent";
const CORE::CEvent CDRNPeerLink::LinkRefusedEvent = "GUCEF::DRN::CDRNPeerLink::LinkRefusedEvent";
const CORE::CEvent CDRNPeerLink::MessageReceivedEvent = "GUCEF::DRN::CDRNPeerLink::MessageReceivedEvent";
const CORE::CEvent CDRNPeerLink::LatencyUpdateEvent = "GUCEF::DRN::CDRNPeerLink::LatencyUpdateEvent";
const CORE::CEvent CDRNPeerLink::PeerLimitExceededEvent = "GUCEF::DRN::CDRNPeerLink::PeerLimitExceededEvent";
const CORE::CEvent CDRNPeerLink::ClockAnomalyEvent = "GUCEF::DRN::CDRNPeerLink::ClockAnomalyEvent";
const CORE::CEvent CDRNPeerLink::TrafficAnomalyEvent = "GUCEF::DRN::CDRNPeerLink::TrafficAnomalyEvent";

/** RTO before the first RTT sample */
static const UInt64 InitialRetransmitTimeoutUs = 200000;
static const UInt64 MinRetransmitTimeoutUs = 50000;

/** What the transcript hash and the keys of handshake 2 are labelled with */
static const char TranscriptLabel[] = "DRN handshake 2";
static const char KeysLabel[] = "DRN 2 keys ";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNLinkSettings::CDRNLinkSettings( void )
    : nodeId()
    , serviceName( "drn" )
    , handshakeTimeoutMs( 5000 )
    , linkTimeoutMs( 10000 )
    , echoIntervalMs( 250 )
    , latencyUpdateIntervalMs( 1000 )
    , ackDelayMs( 5 )
    , closeGraceMs( 100 )
    , firstSequence( 1 )
    , linkId( 0 )
    , limits()
    , anomalies()
    , banList( GUCEF_NULL )
    , requireEncryption( false )
    , requireTokenPolicyHash( false )
    , ciphers()
    , haveStaticKey( false )
    , rekeyPackets( 0 )
    , rekeyBytes( 0 )
{GUCEF_TRACE;

    memset( policyHash, 0, sizeof( policyHash ) );
    memset( staticPrivateKey, 0, sizeof( staticPrivateKey ) );
    memset( staticPublicKey, 0, sizeof( staticPublicKey ) );
}

/*-------------------------------------------------------------------------*/

CDRNLinkClosedInfo::CDRNLinkClosedInfo( void )
    : reason( DRN_LINKREASON_NONE )
    , text()
    , wasOperational( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNLinkMessage::CDRNLinkMessage( void )
    : lane( 0 )
    , msgClass( 0 )
    , opcode( 0 )
    , payload()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::RegisterEvents( void )
{GUCEF_TRACE;

    LinkOperationalEvent.Initialize();
    LinkClosedEvent.Initialize();
    LinkRefusedEvent.Initialize();
    MessageReceivedEvent.Initialize();
    LatencyUpdateEvent.Initialize();
    PeerLimitExceededEvent.Initialize();
    ClockAnomalyEvent.Initialize();
    TrafficAnomalyEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CDRNPeerLink::CDRNPeerLink( void )
    : CORE::CObservingNotifier()
    , CORE::CTSharedObjCreator< CDRNPeerLink, MT::CMutex >( this )
    , CIDRNLinkMessageSink()
    , m_state( LINKSTATE_IDLE )
    , m_initiator( false )
    , m_channel()
    , m_channelReaper()
    , m_authenticator()
    , m_settings()
    , m_reliability()
    , m_ownMeter()
    , m_meter( &m_ownMeter )
    , m_remoteEndpoint()
    , m_linkEpoch( CDRNClock::NowInMicroseconds() )
    , m_remoteLinkEpoch( 0 )
    , m_stateDeadline( 0 )
    , m_closeDeadline( 0 )
    , m_lastReceiveTime( 0 )
    , m_lastSendTime( 0 )
    , m_lastLatencyEventTime( 0 )
    , m_closeReason( DRN_LINKREASON_NONE )
    , m_closeText()
    , m_wasOperational( false )
    , m_remoteNodeId()
    , m_linkId( 0 )
    , m_authResult()
    , m_session( GUCEF_NULL )
    , m_sessionPeerId( 0 )
    , m_rxMessage()
    , m_txScratch()
    , m_cryptBuffer()
    , m_monitor()
    , m_deferred()
    , m_deferredHead( 0 )
    , m_deferredBytes( 0 )
    , m_deferredScratch()
    , m_deferredProtected()
    , m_protection()
    , m_haveEphemeral( false )
    , m_helloPayload()
    , m_challengePayload()
    , m_havePinnedKey( false )
    , m_havePeerStaticKey( false )
    , m_offeredCiphers()
    , m_currentPacketProtected( false )
    , m_protectBuffer()
    , m_authFailures( 0 )
    , m_droppedPlainPackets( 0 )
{GUCEF_TRACE;

    RegisterEvents();
    memset( m_nonce, 0, sizeof( m_nonce ) );
    memset( m_ephemeralPrivate, 0, sizeof( m_ephemeralPrivate ) );
    memset( m_ephemeralPublic, 0, sizeof( m_ephemeralPublic ) );
    memset( m_transcriptHash, 0, sizeof( m_transcriptHash ) );
    memset( m_pinnedKey, 0, sizeof( m_pinnedKey ) );
    memset( m_peerStaticKey, 0, sizeof( m_peerStaticKey ) );
}

/*-------------------------------------------------------------------------*/

CDRNPeerLink::~CDRNPeerLink()
{GUCEF_TRACE;

    SignalUpcomingObserverDestruction();
    WipeHandshakeSecrets();
    m_protection.Clear();
    memset( m_settings.staticPrivateKey, 0, sizeof( m_settings.staticPrivateKey ) );
    if ( !m_channel.IsNULL() )
    {
        UnsubscribeFrom( m_channel.GetPointerAlways() );
        if ( !m_channelReaper.IsNULL() )
        {
            // the channel is closed and destroyed on the thread that runs it
            m_channelReaper->Release( m_channel );
        }
        else
        {
            m_channel->Close();
            m_channel.Unlink();
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CDRNPeerLink::Start( const CIDRNChannelPtr& channel, const CDRNLinkSettings& settings, const CIDRNAuthenticatorPtr& authenticator )
{GUCEF_TRACE;

    if ( channel.IsNULL() || LINKSTATE_IDLE != m_state )
        return false;

    m_channel = channel;
    m_settings = settings;
    m_authenticator = authenticator;

    // the datagram budget always keeps room for the tag of packet protection
    const CDRNChannelCapabilities& capabilities = m_channel->GetCapabilities();
    UInt32 datagramBudget = 0;
    if ( capabilities.isDatagram )
        datagramBudget = capabilities.maxPacketSize > DRN_PACKET_PROTECTION_OVERHEAD ? capabilities.maxPacketSize - DRN_PACKET_PROTECTION_OVERHEAD : capabilities.maxPacketSize;
    m_reliability.Reset( capabilities.isReliable, datagramBudget, m_settings.firstSequence );
    m_protection.Clear();
    m_protection.SetRekeyLimits( m_settings.rekeyPackets, m_settings.rekeyBytes );
    WipeHandshakeSecrets();
    m_havePeerStaticKey = false;
    m_currentPacketProtected = false;

    // until the handshake is done the peer gets nothing but the handshake: no fragments, no reliable lane state,
    // small packets on stream channels
    m_reliability.SetPreAuthRestricted( true );
    m_reliability.SetReceiveLimits( m_settings.limits.maxMessageSize, m_settings.limits.maxFragmentGroups, m_settings.limits.maxFragmentBytes );
    if ( 0 != m_settings.limits.maxPreAuthPacketSize )
        m_channel->SetMaxReceivePacketSize( m_settings.limits.maxPreAuthPacketSize );

    CIDRNLatencyMeter* channelMeter = m_channel->GetLatencyMeter();
    m_meter = GUCEF_NULL != channelMeter ? channelMeter : &m_ownMeter;
    m_meter->Reset();

    UInt64 now = CDRNClock::NowInMicroseconds();
    m_linkEpoch = now;
    m_lastReceiveTime = now;
    m_lastSendTime = now;
    m_lastLatencyEventTime = now;
    m_stateDeadline = now + (UInt64) m_settings.handshakeTimeoutMs * 1000;
    m_monitor.Reset( m_settings.limits, m_settings.anomalies, now );
    m_deferred.clear();
    m_deferredProtected.clear();
    m_deferredHead = 0;
    m_deferredBytes = 0;

    TEventCallback packetCallback( this, &CDRNPeerLink::OnChannelPacketReceived );
    TEventCallback connectedCallback( this, &CDRNPeerLink::OnChannelConnectedEvent );
    TEventCallback disconnectedCallback( this, &CDRNPeerLink::OnChannelDisconnectedEvent );
    TEventCallback errorCallback( this, &CDRNPeerLink::OnChannelErrorEvent );
    SubscribeTo( m_channel.GetPointerAlways(), CIDRNChannel::PacketReceivedEvent, packetCallback );
    SubscribeTo( m_channel.GetPointerAlways(), CIDRNChannel::ChannelConnectedEvent, connectedCallback );
    SubscribeTo( m_channel.GetPointerAlways(), CIDRNChannel::ChannelDisconnectedEvent, disconnectedCallback );
    SubscribeTo( m_channel.GetPointerAlways(), CIDRNChannel::ChannelErrorEvent, errorCallback );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPeerLink::StartConnect( const CIDRNChannelPtr& channel              ,
                            const CDRNEndpoint& remote                  ,
                            const CDRNLinkSettings& settings            ,
                            const CIDRNAuthenticatorPtr& authenticator  )
{GUCEF_TRACE;

    if ( !Start( channel, settings, authenticator ) )
        return false;

    m_initiator = true;
    m_remoteEndpoint = remote;
    SetState( LINKSTATE_CONNECTING );

    // a "key" parameter of the endpoint pins the acceptor's static key
    m_havePinnedKey = false;
    const CORE::CVariant& pinned = remote.GetParams().GetValueAlways( "key" );
    if ( pinned.IsInitialized() && !pinned.AsString( CORE::CString::Empty, true ).IsNULLOrEmpty() )
    {
        if ( !CDRNCrypto::FromHex( pinned.AsString( CORE::CString::Empty, true ), m_pinnedKey, DRN_HANDSHAKE_KEY_SIZE ) )
        {
            CloseNow( DRN_LINKREASON_KEY_MISMATCH, "the pinned key of " + remote.ToString() + " is not a key" );
            return false;
        }
        m_havePinnedKey = true;
    }
    if ( m_settings.requireEncryption && !CanEncrypt() )
    {
        CloseNow( DRN_LINKREASON_POLICY, "the security policy requires encryption and none of its ciphers or no key agreement is available" );
        return false;
    }

    if ( m_channel->IsConnected() )
    {
        OnChannelConnected();
        return true;
    }
    if ( !m_channel->Connect( remote ) )
    {
        CloseNow( DRN_LINKREASON_CONNECT_FAILED, "the channel could not connect to " + remote.ToString() );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPeerLink::StartAccept( const CIDRNChannelPtr& channel              ,
                           const CDRNLinkSettings& settings            ,
                           const CIDRNAuthenticatorPtr& authenticator  )
{GUCEF_TRACE;

    if ( !Start( channel, settings, authenticator ) )
        return false;

    m_initiator = false;
    m_linkId = settings.linkId;
    m_remoteEndpoint = channel->GetRemoteEndpoint();
    SetState( LINKSTATE_AWAITING_HELLO );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::SetState( TLinkState state )
{GUCEF_TRACE;
    m_state = state;
    if ( LINKSTATE_OPERATIONAL == state && !m_wasOperational )
    {
        m_wasOperational = true;

        // the peer authenticated: the full receive side and the stream packet size of an operational link
        m_reliability.SetPreAuthRestricted( false );
        if ( !m_channel.IsNULL() )
            m_channel->SetMaxReceivePacketSize( DRN_MAX_STREAM_PACKET_SIZE );
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::SendHello( void )
{GUCEF_TRACE;

    // an ephemeral key and the ciphers on offer when this side can encrypt
    m_offeredCiphers.clear();
    WipeHandshakeSecrets();
    if ( CanEncrypt() && CDRNCrypto::GenerateKeyAgreementKeyPair( m_ephemeralPrivate, m_ephemeralPublic ) )
    {
        m_haveEphemeral = true;
        for ( UInt32 i=0; i<m_settings.ciphers.size(); ++i )
        {
            CORE::CString cipher = m_settings.ciphers[ i ].Lowercase();
            if ( CDRNCrypto::IsCipherAvailable( cipher ) )
                m_offeredCiphers.push_back( cipher );
        }
    }

    m_helloPayload.SetDataSize( 0 );
    CDRNWireWriter writer( m_helloPayload );
    writer.WriteUInt8( DRN_PROTOCOL_VERSION_MAJOR );
    writer.WriteUInt8( DRN_PROTOCOL_VERSION_MINOR );
    writer.WriteVarUInt32( DRN_FEATURE_NONE );
    writer.WriteString( m_settings.nodeId );
    writer.WriteString( m_settings.serviceName );
    writer.WriteVarUInt64( m_linkEpoch );
    writer.WriteSizedBytes( m_settings.policyHash, DRN_HANDSHAKE_HASH_SIZE );
    writer.WriteSizedBytes( m_ephemeralPublic, m_haveEphemeral ? DRN_HANDSHAKE_KEY_SIZE : 0 );
    writer.WriteVarUInt32( (UInt32) m_offeredCiphers.size() );
    for ( UInt32 i=0; i<m_offeredCiphers.size(); ++i )
        writer.WriteString( m_offeredCiphers[ i ] );
    m_reliability.QueueMessage( DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_LINK, DRN_LINK_HELLO, m_helloPayload.GetConstBufferPtr(), writer.GetOffset(), GUCEF_NULL, 0 );
}

/*-------------------------------------------------------------------------*/

bool
CDRNPeerLink::CanEncrypt( void ) const
{GUCEF_TRACE;

    if ( !CDRNCrypto::IsKeyAgreementAvailable() )
        return false;
    for ( UInt32 i=0; i<m_settings.ciphers.size(); ++i )
    {
        if ( CDRNCrypto::IsCipherAvailable( m_settings.ciphers[ i ] ) )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::WipeHandshakeSecrets( void )
{GUCEF_TRACE;

    memset( m_ephemeralPrivate, 0, sizeof( m_ephemeralPrivate ) );
    memset( m_ephemeralPublic, 0, sizeof( m_ephemeralPublic ) );
    m_haveEphemeral = false;
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::ComputeTranscriptHash( void )
{GUCEF_TRACE;

    // the label, then HELLO and CHALLENGE as they were sent, each with its size in front
    CORE::CSha256 sha;
    sha.Update( TranscriptLabel, (UInt32) sizeof( TranscriptLabel ) - 1 );
    UInt32 sizes[ 2 ] = { m_helloPayload.GetDataSize(), m_challengePayload.GetDataSize() };
    const CORE::CDynamicBuffer* parts[ 2 ] = { &m_helloPayload, &m_challengePayload };
    for ( UInt32 i=0; i<2; ++i )
    {
        UInt8 sizeBytes[ 4 ] = { (UInt8) ( sizes[ i ] >> 24 ), (UInt8) ( sizes[ i ] >> 16 ), (UInt8) ( sizes[ i ] >> 8 ), (UInt8) sizes[ i ] };
        sha.Update( sizeBytes, 4 );
        sha.Update( parts[ i ]->GetConstBufferPtr(), sizes[ i ] );
    }
    sha.Final( m_transcriptHash );
}

/*-------------------------------------------------------------------------*/

bool
CDRNPeerLink::DeriveKeys( const UInt8* sharedEphemeral, const UInt8* sharedStatic, const CORE::CString& cipher )
{GUCEF_TRACE;

    // keys = HKDF( salt = transcript hash, DH(e,e) || DH(e,s), "DRN 2 keys <cipher>" ): the cipher, the policy and
    // every byte of HELLO and CHALLENGE are bound in, so anything changed on the way gives other keys
    UInt8 ikm[ 2 * DRN_HANDSHAKE_KEY_SIZE ];
    memcpy( ikm, sharedEphemeral, DRN_HANDSHAKE_KEY_SIZE );
    memcpy( ikm + DRN_HANDSHAKE_KEY_SIZE, sharedStatic, DRN_HANDSHAKE_KEY_SIZE );
    CORE::CString info = CORE::CString( KeysLabel ) + cipher;
    UInt32 infoSize = info.ByteSize() > 0 ? info.ByteSize() - 1 : 0;
    UInt8 okm[ 2 * CDRNPacketProtection::KeySize ];
    bool ok = CDRNCrypto::Hkdf( m_transcriptHash, DRN_HANDSHAKE_HASH_SIZE, ikm, sizeof( ikm ), info.C_String(), infoSize, okm, sizeof( okm ) );
    if ( ok )
    {
        const UInt8* initiatorToAcceptor = okm;
        const UInt8* acceptorToInitiator = okm + CDRNPacketProtection::KeySize;
        ok = m_initiator ? m_protection.Setup( cipher, initiatorToAcceptor, acceptorToInitiator, 0 )
                         : m_protection.Setup( cipher, acceptorToInitiator, initiatorToAcceptor, 1 );
    }
    memset( ikm, 0, sizeof( ikm ) );
    memset( okm, 0, sizeof( okm ) );
    if ( ok )
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DRNPeerLink(" << CORE::PointerToString( this ) << "): Protected with " << cipher );
    return ok;
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnChannelConnected( void )
{GUCEF_TRACE;

    if ( !m_initiator || LINKSTATE_CONNECTING != m_state )
        return;

    SendHello();
    SetState( LINKSTATE_HELLO_SENT );
    m_stateDeadline = CDRNClock::NowInMicroseconds() + (UInt64) m_settings.handshakeTimeoutMs * 1000;
    FlushImp( false );
}

/*-------------------------------------------------------------------------*/

bool
CDRNPeerLink::Send( UInt8 lane, UInt8 msgClass, UInt32 opcode, const void* payload, UInt32 payloadSize )
{GUCEF_TRACE;

    return SendGather( lane, msgClass, opcode, payload, payloadSize, GUCEF_NULL, 0 );
}

/*-------------------------------------------------------------------------*/

bool
CDRNPeerLink::Send( UInt8 lane, UInt8 msgClass, UInt32 opcode, const CORE::CDynamicBuffer& payload )
{GUCEF_TRACE;

    return SendGather( lane, msgClass, opcode, payload.GetConstBufferPtr(), payload.GetDataSize(), GUCEF_NULL, 0 );
}

/*-------------------------------------------------------------------------*/

bool
CDRNPeerLink::SendGather( UInt8 lane, UInt8 msgClass, UInt32 opcode, const void* part1, UInt32 size1, const void* part2, UInt32 size2 )
{GUCEF_TRACE;

    if ( LINKSTATE_OPERATIONAL != m_state )
        return false;
    if ( DRN_MSGCLASS_LINK == msgClass )
        return false;   // the link's own messages are not for others to send
    if ( 0 != m_settings.limits.maxMessageSize && (UInt64) size1 + size2 > (UInt64) m_settings.limits.maxMessageSize )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNPeerLink(" << CORE::PointerToString( this ) << "): A message of " << ( (UInt64) size1 + size2 ) <<
            " bytes is larger than the security policy allows (" << m_settings.limits.maxMessageSize << ")" );
        return false;
    }
    if ( !m_reliability.QueueMessage( lane, msgClass, opcode, part1, size1, part2, size2 ) )
        return false;
    if ( GUCEF_NULL != m_session )
        m_session->OnLinkTraffic( *this, true, lane, msgClass, opcode, part1, size1, part2, size2 );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::Flush( void )
{GUCEF_TRACE;

    FlushImp( false );
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::FlushImp( bool forceAck )
{GUCEF_TRACE;

    if ( LINKSTATE_CLOSED == m_state || m_channel.IsNULL() || !m_channel->IsConnected() )
        return;

    UInt64 now = CDRNClock::NowInMicroseconds();
    UInt64 linkNow = now - m_linkEpoch;
    bool force = forceAck;
    const UInt8* packet = GUCEF_NULL;
    UInt32 packetSize = 0;
    while ( m_reliability.BuildPacket( linkNow, force, packet, packetSize ) )
    {
        force = false;
        m_meter->OnPacketSent( m_reliability.GetNextSequence() - 1, linkNow, packetSize );

        const void* toSend = packet;
        UInt32 toSendSize = packetSize;
        if ( m_protection.IsSendEnabled() )
        {
            if ( !m_protection.Protect( packet, packetSize, m_protectBuffer ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNPeerLink(" << CORE::PointerToString( this ) << "): Failed to protect a packet" );
                continue;
            }
            toSend = m_protectBuffer.GetConstBufferPtr();
            toSendSize = m_protectBuffer.GetDataSize();
        }
        if ( !m_channel->SendPacket( toSend, toSendSize ) )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "DRNPeerLink(" << CORE::PointerToString( this ) << "): The channel did not take a packet of " << toSendSize << " bytes" );
        }
        m_lastSendTime = now;
        if ( LINKSTATE_CLOSED == m_state || m_channel.IsNULL() )
            return;
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::Close( TDRNLinkReason reason, const CORE::CString& text )
{GUCEF_TRACE;

    if ( LINKSTATE_CLOSED == m_state || LINKSTATE_CLOSING == m_state )
        return;

    if ( m_channel.IsNULL() || !m_channel->IsConnected() || LINKSTATE_IDLE == m_state || LINKSTATE_CONNECTING == m_state )
    {
        CloseNow( reason, text );
        return;
    }

    m_txScratch.SetDataSize( 0 );
    CDRNWireWriter writer( m_txScratch );
    writer.WriteVarUInt32( (UInt32) reason );
    writer.WriteString( text );
    m_reliability.QueueMessage( DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_LINK, DRN_LINK_CLOSE, m_txScratch.GetConstBufferPtr(), writer.GetOffset(), GUCEF_NULL, 0 );

    m_closeReason = reason;
    m_closeText = text;
    SetState( LINKSTATE_CLOSING );
    m_closeDeadline = CDRNClock::NowInMicroseconds() + (UInt64) m_settings.closeGraceMs * 1000;
    FlushImp( false );
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::Refuse( TDRNLinkReason reason, const CORE::CString& text )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DRNPeerLink(" << CORE::PointerToString( this ) << "): Refusing " << m_remoteEndpoint.ToString() << ": " << text );

    m_txScratch.SetDataSize( 0 );
    CDRNWireWriter writer( m_txScratch );
    writer.WriteVarUInt32( (UInt32) reason );
    writer.WriteString( text );
    m_reliability.QueueMessage( DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_LINK, DRN_LINK_REFUSE, m_txScratch.GetConstBufferPtr(), writer.GetOffset(), GUCEF_NULL, 0 );

    m_closeReason = reason;
    m_closeText = text;
    SetState( LINKSTATE_CLOSING );
    m_closeDeadline = CDRNClock::NowInMicroseconds() + (UInt64) m_settings.closeGraceMs * 1000;
    FlushImp( false );

    CDRNLinkClosedEventData eventData;
    eventData.GetData().reason = reason;
    eventData.GetData().text = text;
    eventData.GetData().wasOperational = false;
    NotifyObservers( LinkRefusedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::CloseNow( TDRNLinkReason reason, const CORE::CString& text )
{GUCEF_TRACE;

    if ( LINKSTATE_CLOSED == m_state )
        return;

    bool wasOperational = m_wasOperational;
    SetState( LINKSTATE_CLOSED );
    m_closeReason = reason;
    m_closeText = text;
    m_deferred.clear();
    m_deferredProtected.clear();
    m_deferredHead = 0;
    m_deferredBytes = 0;

    if ( !m_channel.IsNULL() )
    {
        UnsubscribeFrom( m_channel.GetPointerAlways() );
        m_channel->Close();
    }

    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DRNPeerLink(" << CORE::PointerToString( this ) << "): Closed, reason " << (UInt32) reason << " " << text );

    CDRNLinkClosedEventData eventData;
    eventData.GetData().reason = reason;
    eventData.GetData().text = text;
    eventData.GetData().wasOperational = wasOperational;
    NotifyObservers( LinkClosedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNPeerLink::GetRetransmitTimeout( void ) const
{GUCEF_TRACE;
    CDRNLatencyInfo info;
    m_meter->GetLatencyInfo( info );
    if ( 0 == info.sampleCount )
        return InitialRetransmitTimeoutUs;
    UInt64 rto = 2 * (UInt64) info.smoothedRtt;
    return rto < MinRetransmitTimeoutUs ? MinRetransmitTimeoutUs : rto;
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::Tick( UInt64 nodeNow )
{GUCEF_TRACE;

    if ( LINKSTATE_CLOSED == m_state || LINKSTATE_IDLE == m_state )
        return;

    if ( LINKSTATE_CLOSING == m_state )
    {
        if ( nodeNow >= m_closeDeadline )
        {
            CloseNow( m_closeReason, m_closeText );
            return;
        }
        m_reliability.CheckRetransmits( nodeNow - m_linkEpoch, GetRetransmitTimeout() );
        FlushImp( false );
        return;
    }

    if ( m_deferredHead < m_deferred.size() )
    {
        ProcessDeferredPackets( nodeNow );
        if ( LINKSTATE_CLOSED == m_state || LINKSTATE_CLOSING == m_state )
            return;
    }
    if ( m_monitor.IsOverLimitsSustained( nodeNow ) )
    {
        Close( DRN_LINKREASON_LIMIT_EXCEEDED, "over the receive limits for " + CORE::UInt32ToString( m_settings.limits.sustainedLimitSeconds ) + " seconds" );
        return;
    }

    if ( LINKSTATE_OPERATIONAL != m_state )
    {
        if ( nodeNow >= m_stateDeadline )
        {
            CloseNow( LINKSTATE_CONNECTING == m_state ? DRN_LINKREASON_CONNECT_FAILED : DRN_LINKREASON_HANDSHAKE_TIMEOUT, "handshake timeout" );
            return;
        }
    }
    else
    if ( nodeNow > m_lastReceiveTime && nodeNow - m_lastReceiveTime > (UInt64) m_settings.linkTimeoutMs * 1000 )
    {
        CloseNow( DRN_LINKREASON_LINK_TIMEOUT, "nothing received for too long" );
        return;
    }

    UInt64 linkNow = nodeNow - m_linkEpoch;
    m_reliability.CheckRetransmits( linkNow, GetRetransmitTimeout() );

    if ( LINKSTATE_OPERATIONAL == m_state )
    {
        if ( nodeNow - m_lastSendTime >= (UInt64) m_settings.echoIntervalMs * 1000 && !m_reliability.HasQueuedOutput() )
            m_reliability.QueueMessage( DRN_LANE_UNRELIABLE, DRN_MSGCLASS_LINK, DRN_LINK_ECHO, GUCEF_NULL, 0, GUCEF_NULL, 0 );

        if ( nodeNow - m_lastLatencyEventTime >= (UInt64) m_settings.latencyUpdateIntervalMs * 1000 )
        {
            m_lastLatencyEventTime = nodeNow;
            CDRNLatencyInfoEventData eventData;
            m_meter->GetLatencyInfo( eventData.GetData() );
            if ( eventData.GetData().sampleCount > 0 )
            {
                NotifyObservers( LatencyUpdateEvent, &eventData );
                if ( LINKSTATE_CLOSED == m_state )
                    return;
            }
        }
    }

    bool ackDue = m_reliability.IsAckOwed() && linkNow - m_reliability.GetAckOwedSince() >= (UInt64) m_settings.ackDelayMs * 1000;
    FlushImp( ackDue );
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnPacket( const CORE::CDynamicBuffer& packetBuffer )
{GUCEF_TRACE;

    if ( LINKSTATE_CLOSED == m_state || LINKSTATE_IDLE == m_state )
        return;

    const UInt8* data = static_cast< const UInt8* >( packetBuffer.GetConstBufferPtr() );
    UInt32 size = packetBuffer.GetDataSize();
    bool wasProtected = false;
    if ( CDRNPacketProtection::IsProtectedPacket( data, size ) )
    {
        if ( !m_protection.IsReady() || !m_protection.Unprotect( data, size, m_cryptBuffer ) )
        {
            // forged, changed or of another connection: dropped; a stream that carries one is broken
            ++m_authFailures;
            if ( m_reliability.IsTransportReliable() && m_protection.IsReady() )
                CloseNow( DRN_LINKREASON_PROTOCOL_ERROR, "a packet failed authentication" );
            return;
        }
        data = static_cast< const UInt8* >( m_cryptBuffer.GetConstBufferPtr() );
        size = m_cryptBuffer.GetDataSize();
        wasProtected = true;

        // the acceptor protects what it sends once the initiator showed that it has the keys
        if ( !m_initiator && !m_protection.IsSendEnabled() )
            m_protection.SetSendEnabled( true );
    }
    else
    if ( m_protection.IsReady() && LINKSTATE_OPERATIONAL == m_state )
    {
        // once protected, a plain packet can only be forged
        ++m_droppedPlainPackets;
        return;
    }

    UInt64 now = CDRNClock::NowInMicroseconds();
    m_lastReceiveTime = now;

    CDRNLinkAnomaly traffic;
    if ( m_monitor.OnPacketArrival( now, size, traffic ) )
    {
        ReportAnomaly( TrafficAnomalyEvent, traffic );
        if ( LINKSTATE_CLOSED == m_state )
            return;
    }

    // packets held back for the receive limits go first, in the order they came
    if ( m_deferredHead < m_deferred.size() )
    {
        DeferPacket( data, size, wasProtected );
        return;
    }

    TDRNLinkAnomalyKind exceeded = DRN_ANOMALY_NONE;
    if ( !m_monitor.MayReceive( now, exceeded ) )
    {
        // datagram channels: dropped, the reliable messages come again as nothing of the packet is acknowledged;
        // stream channels: held back until the peer is within its limits again
        m_reliability.NoteRateLimitedPacket();
        Float64 threshold = DRN_LIMIT_MESSAGES_PER_SECOND == exceeded ? (Float64) m_settings.limits.maxMessagesPerSecond : (Float64) m_settings.limits.maxBytesPerSecond;
        ReportLimit( exceeded, threshold, threshold, CDRNLinkAnomaly::KindToString( exceeded ) + " limit reached", now );
        if ( LINKSTATE_CLOSED == m_state )
            return;
        if ( m_reliability.IsTransportReliable() )
        {
            DeferPacket( data, size, wasProtected );
        }
        else
        {
            m_monitor.OnPacketDropped();
        }
        return;
    }

    m_currentPacketProtected = wasProtected;
    ProcessReceivedPacket( data, size, now );
    m_currentPacketProtected = false;
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::DeferPacket( const UInt8* data, UInt32 size, bool wasProtected )
{GUCEF_TRACE;

    // at most a second's worth, and at least room for the largest message
    UInt64 cap = 0 != m_settings.limits.maxBytesPerSecond ? (UInt64) m_settings.limits.maxBytesPerSecond : (UInt64) DRN_MAX_STREAM_PACKET_SIZE;
    UInt64 minimumCap = (UInt64) m_settings.limits.maxMessageSize + DRN_MAX_PACKET_HEADER_SIZE + DRN_MAX_MESSAGE_HEADER_SIZE;
    if ( cap < minimumCap )
        cap = minimumCap;
    if ( m_deferredBytes + size > cap )
    {
        UInt64 now = CDRNClock::NowInMicroseconds();
        ReportLimit( DRN_LIMIT_DEFERRED_BYTES, (Float64) ( m_deferredBytes + size ), (Float64) cap, "more held back for the receive limits than allowed", now );
        if ( LINKSTATE_CLOSED != m_state )
            Close( DRN_LINKREASON_LIMIT_EXCEEDED, "the peer sends far over its receive limits" );
        return;
    }

    if ( m_deferredHead > 0 && m_deferredHead >= m_deferred.size() )
    {
        m_deferred.clear();
        m_deferredProtected.clear();
        m_deferredHead = 0;
    }
    m_deferred.push_back( CORE::CDynamicBuffer() );
    m_deferred.back().CopyFrom( 0, size, data );
    m_deferredProtected.push_back( wasProtected ? 1 : 0 );
    m_deferredBytes += size;
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::ProcessDeferredPackets( UInt64 now )
{GUCEF_TRACE;

    while ( m_deferredHead < m_deferred.size() && LINKSTATE_CLOSED != m_state )
    {
        TDRNLinkAnomalyKind exceeded = DRN_ANOMALY_NONE;
        if ( !m_monitor.MayReceive( now, exceeded ) )
            return;

        m_deferredScratch.Swap( m_deferred[ m_deferredHead ] );
        m_currentPacketProtected = m_deferredHead < m_deferredProtected.size() && 0 != m_deferredProtected[ m_deferredHead ];
        ++m_deferredHead;
        m_deferredBytes -= m_deferredScratch.GetDataSize();
        ProcessReceivedPacket( static_cast< const UInt8* >( m_deferredScratch.GetConstBufferPtr() ), m_deferredScratch.GetDataSize(), now );
        m_currentPacketProtected = false;
    }
    if ( m_deferredHead >= m_deferred.size() )
    {
        m_deferred.clear();
        m_deferredProtected.clear();
        m_deferredHead = 0;
        m_deferredBytes = 0;
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::ProcessReceivedPacket( const UInt8* data, UInt32 size, UInt64 now )
{GUCEF_TRACE;

    UInt64 linkNow = now - m_linkEpoch;

    CDRNLatencyInfo info;
    m_meter->GetLatencyInfo( info );
    UInt64 lossDelay = info.sampleCount > 0 ? (UInt64) info.smoothedRtt + 2000 : MinRetransmitTimeoutUs;

    CDRNPacketHeader header;
    UInt32 lostBefore = 0;
    UInt64 messagesBefore = m_reliability.GetStats().messagesReceived;
    CDRNLinkReliability::TPacketResult result = m_reliability.ProcessPacket( data, size, linkNow, lossDelay, *this, header, lostBefore );
    if ( LINKSTATE_CLOSED == m_state )
        return;
    m_monitor.OnReceived( size, (UInt32) ( m_reliability.GetStats().messagesReceived - messagesBefore ) );

    switch ( result )
    {
        case CDRNLinkReliability::PACKET_MALFORMED:
        {
            if ( m_reliability.IsTransportReliable() )
            {
                CloseNow( DRN_LINKREASON_PROTOCOL_ERROR, "malformed packet" );
                return;
            }
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "DRNPeerLink(" << CORE::PointerToString( this ) << "): Dropped a malformed packet" );
            return;
        }
        case CDRNLinkReliability::PACKET_REFUSED:
        {
            // a stream does not send it again: a peer that sends more than the handshake before it authenticated is done
            if ( m_reliability.IsTransportReliable() )
            {
                CloseNow( DRN_LINKREASON_PROTOCOL_ERROR, "more than the handshake before the link was operational" );
                return;
            }
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "DRNPeerLink(" << CORE::PointerToString( this ) << "): Dropped a packet with more than the handshake before the link is operational" );
            return;
        }
        case CDRNLinkReliability::PACKET_LIMIT_EXCEEDED:
        {
            TDRNLinkAnomalyKind kind = DRN_LIMIT_MESSAGE_SIZE;
            Float64 threshold = (Float64) m_settings.limits.maxMessageSize;
            switch ( m_reliability.GetLimitViolation() )
            {
                case CDRNLinkReliability::LIMIT_FRAGMENT_GROUPS:
                {
                    kind = DRN_LIMIT_FRAGMENT_GROUPS;
                    threshold = (Float64) m_settings.limits.maxFragmentGroups;
                    break;
                }
                case CDRNLinkReliability::LIMIT_FRAGMENT_BYTES:
                {
                    kind = DRN_LIMIT_FRAGMENT_BYTES;
                    threshold = (Float64) m_settings.limits.maxFragmentBytes;
                    break;
                }
                default:
                    break;
            }
            CORE::CString text = CDRNLinkAnomaly::KindToString( kind ) + " limit exceeded";
            ReportLimit( kind, threshold, threshold, text, now );
            if ( LINKSTATE_CLOSED != m_state )
                Close( DRN_LINKREASON_LIMIT_EXCEEDED, text );
            return;
        }
        case CDRNLinkReliability::PACKET_ACCEPTED:
        {
            m_meter->OnPacketReceived( header, linkNow, lostBefore );
            CDRNLinkAnomaly clock;
            if ( m_monitor.OnPacketTimed( now, linkNow, header.sendTime, clock ) )
                ReportAnomaly( ClockAnomalyEvent, clock );
            return;
        }
        default:
            return;
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::ReportLimit( TDRNLinkAnomalyKind kind, Float64 measured, Float64 threshold, const CORE::CString& text, UInt64 now )
{GUCEF_TRACE;

    if ( !m_monitor.OnLimitHit( kind, now ) )
        return;

    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DRNPeerLink(" << CORE::PointerToString( this ) << "): Peer " << m_remoteEndpoint.ToString() << ": " << text );
    CDRNLinkAnomalyEventData eventData;
    CDRNLinkAnomaly& anomaly = eventData.GetData();
    anomaly.kind = kind;
    anomaly.measured = measured;
    anomaly.threshold = threshold;
    anomaly.nodeTime = now;
    anomaly.text = text;
    NotifyObservers( PeerLimitExceededEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::ReportAnomaly( const CORE::CEvent& eventid, const CDRNLinkAnomaly& anomaly )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRNPeerLink(" << CORE::PointerToString( this ) << "): Peer " << m_remoteEndpoint.ToString() << ": " << anomaly.text );
    CDRNLinkAnomalyEventData eventData( anomaly );
    NotifyObservers( eventid, &eventData );
}

/*-------------------------------------------------------------------------*/

bool
CDRNPeerLink::OnLinkMessage( const CDRNMessageView& message )
{GUCEF_TRACE;

    if ( DRN_MSGCLASS_LINK == message.msgClass )
        return HandleLinkMessage( message );

    if ( LINKSTATE_OPERATIONAL != m_state )
        return LINKSTATE_CLOSED != m_state;

    if ( GUCEF_NULL != m_session )
        m_session->OnLinkTraffic( *this, false, message.lane, message.msgClass, message.opcode, message.payload, message.payloadSize, GUCEF_NULL, 0 );

    CDRNLinkMessage& rx = m_rxMessage.GetData();
    rx.lane = message.lane;
    rx.msgClass = message.msgClass;
    rx.opcode = message.opcode;
    rx.payload.LinkTo( message.payload, message.payloadSize );
    NotifyObservers( MessageReceivedEvent, &m_rxMessage );
    return LINKSTATE_CLOSED != m_state;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPeerLink::HandleLinkMessage( const CDRNMessageView& message )
{GUCEF_TRACE;

    CDRNWireReader reader( message.payload, message.payloadSize );
    switch ( message.opcode )
    {
        case DRN_LINK_HELLO:
        {
            if ( m_initiator || LINKSTATE_AWAITING_HELLO != m_state )
                return true;

            UInt8 major = 0, minor = 0;
            UInt32 features = 0;
            CORE::CString service;
            const UInt8* policyHash = GUCEF_NULL;
            UInt32 policyHashSize = 0;
            const UInt8* peerEphemeral = GUCEF_NULL;
            UInt32 peerEphemeralSize = 0;
            UInt32 offerCount = 0;
            if ( !reader.ReadUInt8( major ) || !reader.ReadUInt8( minor ) || !reader.ReadVarUInt32( features ) )
            {
                Refuse( DRN_LINKREASON_PROTOCOL_ERROR, "malformed HELLO" );
                return false;
            }
            if ( DRN_PROTOCOL_VERSION_MAJOR != major )
            {
                Refuse( DRN_LINKREASON_PROTOCOL_VERSION, "protocol version " + CORE::UInt32ToString( major ) + " is not supported" );
                return false;
            }
            if ( !reader.ReadString( m_remoteNodeId ) || !reader.ReadString( service ) || !reader.ReadVarUInt64( m_remoteLinkEpoch ) ||
                 !reader.ReadSizedBytes( policyHash, policyHashSize ) || !reader.ReadSizedBytes( peerEphemeral, peerEphemeralSize ) ||
                 !reader.ReadVarUInt32( offerCount ) || offerCount > 32 )
            {
                Refuse( DRN_LINKREASON_PROTOCOL_ERROR, "malformed HELLO" );
                return false;
            }
            CORE::CString::StringVector offer;
            for ( UInt32 i=0; i<offerCount; ++i )
            {
                CORE::CString cipher;
                if ( !reader.ReadString( cipher ) )
                {
                    Refuse( DRN_LINKREASON_PROTOCOL_ERROR, "malformed HELLO" );
                    return false;
                }
                offer.push_back( cipher.Lowercase() );
            }
            if ( service != m_settings.serviceName )
            {
                Refuse( DRN_LINKREASON_SERVICE_MISMATCH, "service \"" + service + "\" is not offered here" );
                return false;
            }
            if ( DRN_HANDSHAKE_HASH_SIZE != policyHashSize || !CDRNCrypto::ConstantTimeEquals( policyHash, m_settings.policyHash, DRN_HANDSHAKE_HASH_SIZE ) )
            {
                Refuse( DRN_LINKREASON_POLICY, "the security policies differ" );
                return false;
            }
            m_helloPayload.SetDataSize( 0 );
            m_helloPayload.CopyFrom( 0, message.payloadSize, message.payload );

            // the first of our ciphers the peer offers too and that we have a codec for
            CORE::CString chosen;
            if ( m_settings.haveStaticKey && DRN_HANDSHAKE_KEY_SIZE == peerEphemeralSize && CDRNCrypto::IsKeyAgreementAvailable() )
            {
                for ( UInt32 i=0; i<m_settings.ciphers.size() && chosen.IsNULLOrEmpty(); ++i )
                {
                    CORE::CString cipher = m_settings.ciphers[ i ].Lowercase();
                    for ( UInt32 n=0; n<offer.size(); ++n )
                    {
                        if ( offer[ n ] == cipher && CDRNCrypto::IsCipherAvailable( cipher ) )
                        {
                            chosen = cipher;
                            break;
                        }
                    }
                }
            }
            if ( chosen.IsNULLOrEmpty() && m_settings.requireEncryption )
            {
                Refuse( DRN_LINKREASON_POLICY, 0 == peerEphemeralSize ? CORE::CString( "the security policy requires encryption and the peer offers none" )
                                                                      : CORE::CString( "no cipher in common" ) );
                return false;
            }

            if ( !CDRNCrypto::RandomBytes( m_nonce, DRN_CHALLENGE_NONCE_SIZE ) ||
                 ( !chosen.IsNULLOrEmpty() && !CDRNCrypto::GenerateKeyAgreementKeyPair( m_ephemeralPrivate, m_ephemeralPublic ) ) )
            {
                // a predictable challenge or key would make every proof replayable: no link without randomness
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "DRNPeerLink(" << CORE::PointerToString( this ) << "): No randomness or key agreement for the challenge, refusing the link" );
                WipeHandshakeSecrets();
                Refuse( DRN_LINKREASON_LOCAL_ERROR, "the accepting side cannot make a challenge" );
                return false;
            }
            m_haveEphemeral = !chosen.IsNULLOrEmpty();
            CORE::CString method = m_authenticator.IsNULL() ? CORE::CString( "anonymous" ) : m_authenticator->GetMethodName();

            m_challengePayload.SetDataSize( 0 );
            CDRNWireWriter writer( m_challengePayload );
            writer.WriteSizedBytes( m_nonce, DRN_CHALLENGE_NONCE_SIZE );
            writer.WriteString( m_settings.nodeId );
            writer.WriteUInt8( DRN_PROTOCOL_VERSION_MINOR );
            writer.WriteString( method );
            writer.WriteSizedBytes( m_settings.policyHash, DRN_HANDSHAKE_HASH_SIZE );
            writer.WriteSizedBytes( m_ephemeralPublic, m_haveEphemeral ? DRN_HANDSHAKE_KEY_SIZE : 0 );
            writer.WriteSizedBytes( m_settings.staticPublicKey, m_haveEphemeral ? DRN_HANDSHAKE_KEY_SIZE : 0 );
            writer.WriteString( chosen );
            ComputeTranscriptHash();

            if ( m_haveEphemeral )
            {
                // DH(e,e) and DH(s,e): only the holder of the static key gets these keys
                UInt8 sharedEphemeral[ DRN_HANDSHAKE_KEY_SIZE ];
                UInt8 sharedStatic[ DRN_HANDSHAKE_KEY_SIZE ];
                bool ok = CDRNCrypto::ComputeSharedSecret( m_ephemeralPrivate, peerEphemeral, sharedEphemeral ) &&
                          CDRNCrypto::ComputeSharedSecret( m_settings.staticPrivateKey, peerEphemeral, sharedStatic ) &&
                          DeriveKeys( sharedEphemeral, sharedStatic, chosen );
                memset( sharedEphemeral, 0, sizeof( sharedEphemeral ) );
                memset( sharedStatic, 0, sizeof( sharedStatic ) );
                memset( m_ephemeralPrivate, 0, sizeof( m_ephemeralPrivate ) );
                if ( !ok )
                {
                    Refuse( DRN_LINKREASON_KEY_MISMATCH, "the keys could not be agreed" );
                    return false;
                }
            }

            // CHALLENGE goes out in the clear; this side protects what it sends once a protected packet came in
            m_reliability.QueueMessage( DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_LINK, DRN_LINK_CHALLENGE, m_challengePayload.GetConstBufferPtr(), writer.GetOffset(), GUCEF_NULL, 0 );
            SetState( LINKSTATE_CHALLENGE_SENT );
            m_stateDeadline = CDRNClock::NowInMicroseconds() + (UInt64) m_settings.handshakeTimeoutMs * 1000;
            FlushImp( false );
            return LINKSTATE_CLOSED != m_state;
        }
        case DRN_LINK_CHALLENGE:
        {
            if ( !m_initiator || LINKSTATE_HELLO_SENT != m_state )
                return true;

            const UInt8* nonce = GUCEF_NULL;
            UInt32 nonceSize = 0;
            UInt8 minor = 0;
            CORE::CString method;
            const UInt8* policyHash = GUCEF_NULL;
            UInt32 policyHashSize = 0;
            const UInt8* peerEphemeral = GUCEF_NULL;
            UInt32 peerEphemeralSize = 0;
            const UInt8* peerStatic = GUCEF_NULL;
            UInt32 peerStaticSize = 0;
            CORE::CString chosen;
            if ( !reader.ReadSizedBytes( nonce, nonceSize ) || DRN_CHALLENGE_NONCE_SIZE != nonceSize ||
                 !reader.ReadString( m_remoteNodeId ) || !reader.ReadUInt8( minor ) || !reader.ReadString( method ) ||
                 !reader.ReadSizedBytes( policyHash, policyHashSize ) || !reader.ReadSizedBytes( peerEphemeral, peerEphemeralSize ) ||
                 !reader.ReadSizedBytes( peerStatic, peerStaticSize ) || !reader.ReadString( chosen ) )
            {
                CloseNow( DRN_LINKREASON_PROTOCOL_ERROR, "malformed CHALLENGE" );
                return false;
            }
            memcpy( m_nonce, nonce, DRN_CHALLENGE_NONCE_SIZE );
            if ( DRN_HANDSHAKE_HASH_SIZE != policyHashSize || !CDRNCrypto::ConstantTimeEquals( policyHash, m_settings.policyHash, DRN_HANDSHAKE_HASH_SIZE ) )
            {
                CloseNow( DRN_LINKREASON_POLICY, "the security policies differ" );
                return false;
            }
            m_challengePayload.SetDataSize( 0 );
            m_challengePayload.CopyFrom( 0, message.payloadSize, message.payload );
            ComputeTranscriptHash();

            if ( chosen.IsNULLOrEmpty() )
            {
                WipeHandshakeSecrets();
                if ( m_settings.requireEncryption )
                {
                    CloseNow( DRN_LINKREASON_POLICY, "the security policy requires encryption and the acceptor offers none" );
                    return false;
                }
                if ( m_havePinnedKey )
                {
                    CloseNow( DRN_LINKREASON_KEY_MISMATCH, "the acceptor does not prove the pinned key" );
                    return false;
                }
            }
            else
            {
                // a cipher we did not offer means HELLO was changed on the way
                bool offered = false;
                for ( UInt32 i=0; i<m_offeredCiphers.size() && !offered; ++i )
                    offered = m_offeredCiphers[ i ] == chosen;
                if ( !offered || !m_haveEphemeral || DRN_HANDSHAKE_KEY_SIZE != peerEphemeralSize || DRN_HANDSHAKE_KEY_SIZE != peerStaticSize )
                {
                    WipeHandshakeSecrets();
                    CloseNow( DRN_LINKREASON_POLICY, "the acceptor chose a cipher that was not offered" );
                    return false;
                }
                if ( m_havePinnedKey && !CDRNCrypto::ConstantTimeEquals( peerStatic, m_pinnedKey, DRN_HANDSHAKE_KEY_SIZE ) )
                {
                    WipeHandshakeSecrets();
                    CloseNow( DRN_LINKREASON_KEY_MISMATCH, "the acceptor's key is not the pinned key" );
                    return false;
                }
                memcpy( m_peerStaticKey, peerStatic, DRN_HANDSHAKE_KEY_SIZE );
                m_havePeerStaticKey = true;

                UInt8 sharedEphemeral[ DRN_HANDSHAKE_KEY_SIZE ];
                UInt8 sharedStatic[ DRN_HANDSHAKE_KEY_SIZE ];
                bool ok = CDRNCrypto::ComputeSharedSecret( m_ephemeralPrivate, peerEphemeral, sharedEphemeral ) &&
                          CDRNCrypto::ComputeSharedSecret( m_ephemeralPrivate, peerStatic, sharedStatic ) &&
                          DeriveKeys( sharedEphemeral, sharedStatic, chosen );
                memset( sharedEphemeral, 0, sizeof( sharedEphemeral ) );
                memset( sharedStatic, 0, sizeof( sharedStatic ) );
                WipeHandshakeSecrets();
                if ( !ok )
                {
                    CloseNow( DRN_LINKREASON_KEY_MISMATCH, "the keys could not be agreed" );
                    return false;
                }
                // from AUTH on everything this side sends is protected
                m_protection.SetSendEnabled( true );
            }

            CDRNAuthContext context;
            context.serviceName = m_settings.serviceName;
            context.initiatorNodeId = m_settings.nodeId;
            context.acceptorNodeId = m_remoteNodeId;
            memcpy( context.nonce, m_nonce, DRN_CHALLENGE_NONCE_SIZE );
            memcpy( context.transcriptHash, m_transcriptHash, DRN_HANDSHAKE_HASH_SIZE );
            memcpy( context.policyHash, m_settings.policyHash, DRN_HANDSHAKE_HASH_SIZE );
            context.requirePolicyHash = m_settings.requireTokenPolicyHash;

            CORE::CString ourMethod = m_authenticator.IsNULL() ? CORE::CString( "anonymous" ) : m_authenticator->GetMethodName();
            CORE::CDynamicBuffer proof;
            if ( !m_authenticator.IsNULL() && !m_authenticator->CreateProof( context, proof ) )
                proof.SetDataSize( 0 );

            m_txScratch.SetDataSize( 0 );
            CDRNWireWriter writer( m_txScratch );
            writer.WriteString( ourMethod );
            writer.WriteSizedBytes( proof.GetConstBufferPtr(), proof.GetDataSize() );
            m_reliability.QueueMessage( DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_LINK, DRN_LINK_AUTH, m_txScratch.GetConstBufferPtr(), writer.GetOffset(), GUCEF_NULL, 0 );

            SetState( LINKSTATE_AUTH_SENT );
            m_stateDeadline = CDRNClock::NowInMicroseconds() + (UInt64) m_settings.handshakeTimeoutMs * 1000;
            FlushImp( false );
            return LINKSTATE_CLOSED != m_state;
        }
        case DRN_LINK_AUTH:
        {
            if ( m_initiator || LINKSTATE_CHALLENGE_SENT != m_state )
                return true;
            if ( m_protection.IsReady() && !m_currentPacketProtected )
                return true;    // with agreed keys AUTH only counts protected: a plain one was not made with them

            CORE::CString method;
            const UInt8* proof = GUCEF_NULL;
            UInt32 proofSize = 0;
            if ( !reader.ReadString( method ) || !reader.ReadSizedBytes( proof, proofSize ) )
            {
                Refuse( DRN_LINKREASON_PROTOCOL_ERROR, "malformed AUTH" );
                return false;
            }

            CORE::CString ourMethod = m_authenticator.IsNULL() ? CORE::CString( "anonymous" ) : m_authenticator->GetMethodName();
            if ( method != ourMethod )
            {
                Refuse( DRN_LINKREASON_AUTH_METHOD, "authentication method \"" + ourMethod + "\" is required" );
                return false;
            }

            CDRNAuthContext context;
            context.serviceName = m_settings.serviceName;
            context.initiatorNodeId = m_remoteNodeId;
            context.acceptorNodeId = m_settings.nodeId;
            memcpy( context.nonce, m_nonce, DRN_CHALLENGE_NONCE_SIZE );
            memcpy( context.transcriptHash, m_transcriptHash, DRN_HANDSHAKE_HASH_SIZE );
            memcpy( context.policyHash, m_settings.policyHash, DRN_HANDSHAKE_HASH_SIZE );
            context.requirePolicyHash = m_settings.requireTokenPolicyHash;

            m_authResult = CDRNAuthResult();
            if ( !m_authenticator.IsNULL() && !m_authenticator->VerifyProof( context, proof, proofSize, m_authResult ) )
            {
                Refuse( DRN_LINKREASON_AUTH_FAILED, m_authResult.refusalText.IsNULLOrEmpty() ? CORE::CString( "authentication failed" ) : m_authResult.refusalText );
                return false;
            }

            CDRNBanEntry ban;
            bool banned = false;
            if ( GUCEF_NULL != m_settings.banList )
            {
                banned = m_settings.banList->IsUserBanned( m_authResult.userId, &ban );
                if ( !banned && m_authResult.publicKey.GetDataSize() > 0 )
                    banned = m_settings.banList->IsPublicKeyBanned( static_cast< const UInt8* >( m_authResult.publicKey.GetConstBufferPtr() ), m_authResult.publicKey.GetDataSize(), &ban );
                for ( size_t a=0; !banned && a<m_authResult.accounts.size(); ++a )
                    banned = m_settings.banList->IsAccountBanned( m_authResult.accounts[ a ], &ban );
            }
            if ( banned )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRNPeerLink(" << CORE::PointerToString( this ) << "): User \"" << m_authResult.userId << "\" is banned" );
                Refuse( DRN_LINKREASON_BANNED, ban.reason.IsNULLOrEmpty() ? CORE::CString( "banned" ) : "banned: " + ban.reason );
                return false;
            }

            // WELCOME is the first message this side protects
            m_txScratch.SetDataSize( 0 );
            CDRNWireWriter writer( m_txScratch );
            writer.WriteVarUInt32( m_linkId );
            writer.WriteVarUInt64( m_linkEpoch );
            m_reliability.QueueMessage( DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_LINK, DRN_LINK_WELCOME, m_txScratch.GetConstBufferPtr(), writer.GetOffset(), GUCEF_NULL, 0 );
            SetState( LINKSTATE_OPERATIONAL );
            FlushImp( false );
            if ( LINKSTATE_CLOSED == m_state )
                return false;

            NotifyObservers( LinkOperationalEvent );
            return LINKSTATE_CLOSED != m_state;
        }
        case DRN_LINK_WELCOME:
        {
            if ( !m_initiator || LINKSTATE_AUTH_SENT != m_state )
                return true;
            if ( m_protection.IsReady() && !m_currentPacketProtected )
                return true;    // only the acceptor that holds the keys can send WELCOME

            if ( !reader.ReadVarUInt32( m_linkId ) || !reader.ReadVarUInt64( m_remoteLinkEpoch ) )
            {
                CloseNow( DRN_LINKREASON_PROTOCOL_ERROR, "malformed WELCOME" );
                return false;
            }
            SetState( LINKSTATE_OPERATIONAL );
            NotifyObservers( LinkOperationalEvent );
            return LINKSTATE_CLOSED != m_state;
        }
        case DRN_LINK_REFUSE:
        {
            UInt32 reason = 0;
            CORE::CString text;
            reader.ReadVarUInt32( reason );
            reader.ReadString( text );

            CDRNLinkClosedEventData eventData;
            eventData.GetData().reason = (TDRNLinkReason) reason;
            eventData.GetData().text = text;
            eventData.GetData().wasOperational = false;
            NotifyObservers( LinkRefusedEvent, &eventData );
            CloseNow( DRN_LINKREASON_REFUSED_BY_PEER, text );
            return false;
        }
        case DRN_LINK_CLOSE:
        {
            UInt32 reason = 0;
            CORE::CString text;
            reader.ReadVarUInt32( reason );
            reader.ReadString( text );
            CloseNow( DRN_LINKREASON_REMOTE_CLOSED, text );
            return false;
        }
        case DRN_LINK_ECHO:
        default:
        {
            return true;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnNotify( CORE::CNotifier* notifier                 ,
                        const CORE::CEvent& eventid               ,
                        CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    if ( m_channel.IsNULL() || notifier != m_channel.GetPointerAlways() )
        return;

    if ( CORE::CNotifier::DestructionEvent == eventid )
        OnChannelLost();
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnChannelPacketReceived( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_channel.IsNULL() || notifier != m_channel.GetPointerAlways() )
        return;
    if ( GUCEF_NULL != eventData )
        OnPacket( static_cast< const CDRNPacketEventData* >( eventData )->GetData() );     // const: no private copy
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnChannelConnectedEvent( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_channel.IsNULL() || notifier != m_channel.GetPointerAlways() )
        return;
    OnChannelConnected();
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnChannelDisconnectedEvent( CORE::CNotifier* notifier    ,
                                          const CORE::CEvent& eventId  ,
                                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_channel.IsNULL() || notifier != m_channel.GetPointerAlways() )
        return;
    OnChannelLost();
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnChannelErrorEvent( CORE::CNotifier* notifier    ,
                                   const CORE::CEvent& eventId  ,
                                   CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_channel.IsNULL() || notifier != m_channel.GetPointerAlways() )
        return;

    CORE::CString text = GUCEF_NULL != eventData ? static_cast< CDRNChannelErrorEventData* >( eventData )->GetData() : CORE::CString( "channel error" );
    if ( LINKSTATE_CONNECTING == m_state )
        CloseNow( DRN_LINKREASON_CONNECT_FAILED, text );
    else
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "DRNPeerLink(" << CORE::PointerToString( this ) << "): Channel error: " << text );
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnChannelLost( void )
{GUCEF_TRACE;

    if ( LINKSTATE_CLOSING == m_state )
        CloseNow( m_closeReason, m_closeText );
    else
    if ( LINKSTATE_CONNECTING == m_state )
        CloseNow( DRN_LINKREASON_CONNECT_FAILED, "the channel did not connect" );
    else
        CloseNow( DRN_LINKREASON_CHANNEL_LOST, "the channel was disconnected" );
}

/*-------------------------------------------------------------------------*/

CDRNPeerLink::TLinkState
CDRNPeerLink::GetState( void ) const
{GUCEF_TRACE;

    return m_state;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPeerLink::IsOperational( void ) const
{GUCEF_TRACE;

    return LINKSTATE_OPERATIONAL == m_state;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPeerLink::IsClosed( void ) const
{GUCEF_TRACE;

    return LINKSTATE_CLOSED == m_state;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNPeerLink::GetLastReceiveTime( void ) const
{GUCEF_TRACE;

    return m_lastReceiveTime;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPeerLink::IsInitiator( void ) const
{GUCEF_TRACE;

    return m_initiator;
}

/*-------------------------------------------------------------------------*/

TDRNLinkReason
CDRNPeerLink::GetCloseReason( void ) const
{GUCEF_TRACE;

    return m_closeReason;
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::GetLatencyInfo( CDRNLatencyInfo& info ) const
{GUCEF_TRACE;

    m_meter->GetLatencyInfo( info );
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNPeerLink::GetLinkTime( void ) const
{GUCEF_TRACE;

    return CDRNClock::NowInMicroseconds() - m_linkEpoch;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNPeerLink::GetLocalLinkEpoch( void ) const
{GUCEF_TRACE;

    return m_linkEpoch;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNPeerLink::GetRemoteLinkEpoch( void ) const
{GUCEF_TRACE;

    return m_remoteLinkEpoch;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNPeerLink::GetEstimatedRemoteTime( UInt64 localNodeTime ) const
{GUCEF_TRACE;

    CDRNLatencyInfo info;
    m_meter->GetLatencyInfo( info );
    Int64 remoteLinkTime = (Int64) localNodeTime - (Int64) m_linkEpoch + info.clockOffset;
    Int64 remoteNodeTime = remoteLinkTime + (Int64) m_remoteLinkEpoch;
    return remoteNodeTime > 0 ? (UInt64) remoteNodeTime : 0;
}

/*-------------------------------------------------------------------------*/

const CDRNLinkStats&
CDRNPeerLink::GetStats( void ) const
{GUCEF_TRACE;

    return m_reliability.GetStats();
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNPeerLink::GetNextSequence( void ) const
{GUCEF_TRACE;

    return m_reliability.GetNextSequence();
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNPeerLink::GetUnackedCount( void ) const
{GUCEF_TRACE;

    return m_reliability.GetUnackedCount();
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNPeerLink::GetMaxUnfragmentedPayload( void ) const
{GUCEF_TRACE;

    return m_reliability.GetMaxUnfragmentedPayload();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNPeerLink::GetRemoteNodeId( void ) const
{GUCEF_TRACE;

    return m_remoteNodeId;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNPeerLink::GetLinkId( void ) const
{GUCEF_TRACE;

    return m_linkId;
}

/*-------------------------------------------------------------------------*/

const CDRNAuthResult&
CDRNPeerLink::GetAuthResult( void ) const
{GUCEF_TRACE;

    return m_authResult;
}

/*-------------------------------------------------------------------------*/

CIDRNChannelPtr
CDRNPeerLink::GetChannel( void ) const
{GUCEF_TRACE;

    return m_channel;
}

/*-------------------------------------------------------------------------*/

CDRNEndpoint
CDRNPeerLink::GetRemoteEndpoint( void ) const
{GUCEF_TRACE;

    return m_remoteEndpoint;
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::GetTelemetry( CORE::CDataNode& telemetry ) const
{GUCEF_TRACE;

    telemetry.SetAttribute( "linkId", m_linkId );
    telemetry.SetAttribute( "remoteNodeId", m_remoteNodeId );
    telemetry.SetAttribute( "remoteEndpoint", m_remoteEndpoint.ToString() );
    telemetry.SetAttribute( "userId", m_authResult.userId );
    telemetry.SetAttribute( "operational", LINKSTATE_OPERATIONAL == m_state );

    const CDRNLinkStats& stats = m_reliability.GetStats();
    telemetry.SetAttribute( "packetsSent", stats.packetsSent );
    telemetry.SetAttribute( "packetsReceived", stats.packetsReceived );
    telemetry.SetAttribute( "bytesSent", stats.bytesSent );
    telemetry.SetAttribute( "bytesReceived", stats.bytesReceived );
    telemetry.SetAttribute( "messagesSent", stats.messagesSent );
    telemetry.SetAttribute( "messagesReceived", stats.messagesReceived );
    telemetry.SetAttribute( "packetsLost", stats.packetsLost );
    telemetry.SetAttribute( "duplicatePackets", stats.duplicatePackets );
    telemetry.SetAttribute( "tooOldPackets", stats.tooOldPackets );
    telemetry.SetAttribute( "staleMessages", stats.staleMessages );
    telemetry.SetAttribute( "duplicateMessages", stats.duplicateMessages );
    telemetry.SetAttribute( "messagesResent", stats.messagesResent );
    telemetry.SetAttribute( "preAuthRefusedPackets", stats.preAuthRefusedPackets );
    telemetry.SetAttribute( "rateLimitedPackets", stats.rateLimitedPackets );
    telemetry.SetAttribute( "openFragmentGroups", m_reliability.GetOpenFragmentGroups() );
    telemetry.SetAttribute( "openFragmentBytes", m_reliability.GetOpenFragmentBytes() );
    telemetry.SetAttribute( "deferredPackets", (UInt32) ( m_deferred.size() - m_deferredHead ) );
    telemetry.SetAttribute( "deferredBytes", m_deferredBytes );

    CDRNLatencyInfo info;
    m_meter->GetLatencyInfo( info );
    telemetry.SetAttribute( "rttUs", info.smoothedRtt );
    telemetry.SetAttribute( "rttJitterUs", info.rttVariance );
    telemetry.SetAttribute( "minRttUs", info.minRtt );
    telemetry.SetAttribute( "lossRatio", CORE::CVariant( info.lossRatio ) );
    telemetry.SetAttribute( "clockOffsetUs", info.clockOffset );

    m_monitor.GetTelemetry( telemetry );
}

/*-------------------------------------------------------------------------*/

const CDRNLinkMonitor&
CDRNPeerLink::GetMonitor( void ) const
{GUCEF_TRACE;

    return m_monitor;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNPeerLink::GetDeferredPacketCount( void ) const
{GUCEF_TRACE;

    return (UInt32) ( m_deferred.size() - m_deferredHead );
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::SetChannelReaper( const CDRNChannelReaperPtr& reaper )
{GUCEF_TRACE;

    m_channelReaper = reaper;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPeerLink::IsProtected( void ) const
{GUCEF_TRACE;

    return m_protection.IsReady();
}

/*-------------------------------------------------------------------------*/

const CDRNPacketProtection&
CDRNPeerLink::GetProtection( void ) const
{GUCEF_TRACE;

    return m_protection;
}

/*-------------------------------------------------------------------------*/

const UInt8*
CDRNPeerLink::GetPeerStaticKey( void ) const
{GUCEF_TRACE;

    return m_havePeerStaticKey ? m_peerStaticKey : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const UInt8*
CDRNPeerLink::GetTranscriptHash( void ) const
{GUCEF_TRACE;

    return m_transcriptHash;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNPeerLink::GetAuthenticationFailureCount( void ) const
{GUCEF_TRACE;

    return m_authFailures;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNPeerLink::GetDroppedPlainPacketCount( void ) const
{GUCEF_TRACE;

    return m_droppedPlainPackets;
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::SetSessionBinding( CDRNSession* session, UInt32 peerId )
{GUCEF_TRACE;

    m_session = session;
    m_sessionPeerId = peerId;
}

/*-------------------------------------------------------------------------*/

CDRNSession*
CDRNPeerLink::GetSession( void ) const
{GUCEF_TRACE;

    return m_session;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNPeerLink::GetSessionPeerId( void ) const
{GUCEF_TRACE;

    return m_sessionPeerId;
}

/*-------------------------------------------------------------------------*/

CDRNPeerLinkPtr
CDRNPeerLink::GetSharedPtr( void ) const
{GUCEF_TRACE;

    return CreateSharedPtr();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNPeerLink::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRN::CDRNPeerLink";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
