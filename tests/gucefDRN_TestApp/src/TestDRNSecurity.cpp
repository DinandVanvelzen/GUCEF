/*
 *  gucefDRN_TestApp: GUCEF test application for the DRN module
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
#include <exception>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_COMCORE_CTCPCLIENTSOCKET_H
#include "CTCPClientSocket.h"
#define GUCEF_COMCORE_CTCPCLIENTSOCKET_H
#endif /* GUCEF_COMCORE_CTCPCLIENTSOCKET_H ? */

#ifndef GUCEF_COMCORE_CUDPSOCKET_H
#include "CUDPSocket.h"
#define GUCEF_COMCORE_CUDPSOCKET_H
#endif /* GUCEF_COMCORE_CUDPSOCKET_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETCLIENT_H
#include "gucefWEB_CWebSocketClient.h"
#define GUCEF_WEB_CWEBSOCKETCLIENT_H
#endif /* GUCEF_WEB_CWEBSOCKETCLIENT_H ? */

#ifndef GUCEF_DRN_H
#include "gucefDRN.h"
#define GUCEF_DRN_H
#endif /* GUCEF_DRN_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestDRNTestUtils.h"
#include "TestDRNSecurity.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace DRNTEST;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)
#define REQUIRE_TRUE(t)  do { if ( !(t) ) { ::GUCEF::TESTFW::CTestContext::Instance().RecordFailure( #t, __FILE__, __LINE__ ); throw std::exception(); } } while ( 0 )

/** The hash of the default policy, computed independently from the documented canonical encoding */
static const char* DefaultPolicyHash = "c28b6ad2847cd5499be366e05652dc0170386bc495be5295b82a290fb5abd8d4";

/*-------------------------------------------------------------------------*/

static DRN::CDRNNodeSettings
SecurityNodeSettings( void )
{
    DRN::CDRNNodeSettings settings;
    settings.echoIntervalMs = 20;
    settings.handshakeTimeoutMs = 2000;
    settings.connectTimeoutMs = 2000;
    settings.pulseIntervalMs = 1;
    return settings;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
TestFilePath( const CORE::CString& name )
{
    CORE::CString path = CORE::RelativePath( "$CURWORKDIR$" );
    CORE::AppendToPath( path, name );
    return path;
}

/*-------------------------------------------------------------------------*/

/** A packet header written into the buffer, messages are added after it */
static void
StartPacket( CORE::CDynamicBuffer& packet, UInt32 sequence )
{
    packet.SetDataSize( 0 );
    DRN::CDRNWireWriter writer( packet );
    DRN::CDRNPacketHeader header;
    header.sequence = sequence;
    header.sendTime = 1000 * (UInt64) sequence;
    DRN::CDRNPacketCodec::WriteHeader( writer, header );
}

/*-------------------------------------------------------------------------*/

static void
AddMessage( CORE::CDynamicBuffer& packet, UInt8 msgClass, UInt32 opcode, UInt8 lane, UInt32 laneSequence, const void* payload, UInt32 payloadSize )
{
    DRN::CDRNWireWriter writer( packet, packet.GetDataSize() );
    DRN::CDRNPacketCodec::WriteMessage( writer, msgClass, opcode, lane, laneSequence, payload, payloadSize );
}

/*-------------------------------------------------------------------------*/

/** The payload of a FRAGMENT message with some bytes of an APP message */
static void
MakeFragment( CORE::CDynamicBuffer& payload, UInt32 group, UInt32 index, UInt32 count, UInt32 totalSize, UInt32 offset, UInt32 bytes )
{
    payload.SetDataSize( 0 );
    DRN::CDRNWireWriter writer( payload );
    writer.WriteVarUInt32( group );
    writer.WriteVarUInt32( index );
    writer.WriteVarUInt32( count );
    writer.WriteVarUInt32( totalSize );
    writer.WriteVarUInt32( offset );
    writer.WriteUInt8( DRN::DRN_MSGCLASS_APP );
    writer.WriteVarUInt32( 1 );
    for ( UInt32 i=0; i<bytes; ++i )
        writer.WriteUInt8( (UInt8) i );
}

/*-------------------------------------------------------------------------*/

/** A packet with one fragment of a message of totalSize bytes */
static void
MakeFragmentPacket( CORE::CDynamicBuffer& packet, UInt32 sequence, UInt8 lane, UInt32 laneSequence, UInt32 group, UInt32 totalSize )
{
    CORE::CDynamicBuffer payload;
    MakeFragment( payload, group, 0, ( totalSize + 999 ) / 1000, totalSize, 0, 100 );
    StartPacket( packet, sequence );
    AddMessage( packet, DRN::DRN_MSGCLASS_LINK, DRN::DRN_LINK_FRAGMENT, lane, laneSequence, payload.GetConstBufferPtr(), payload.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

/** Counts what the reliability layer delivers */
class CCountingSink : public DRN::CIDRNLinkMessageSink
{
    public:
    UInt32 count;
    UInt8 lastClass;
    CCountingSink( void ) : count( 0 ), lastClass( 0 ) {}
    virtual bool OnLinkMessage( const DRN::CDRNMessageView& message ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++count;
        lastClass = message.msgClass;
        return true;
    }
};

/*-------------------------------------------------------------------------*/

static DRN::CDRNLinkReliability::TPacketResult
Feed( DRN::CDRNLinkReliability& reliability, const CORE::CDynamicBuffer& packet, CCountingSink& sink )
{
    DRN::CDRNPacketHeader header;
    UInt32 lostBefore = 0;
    return reliability.ProcessPacket( static_cast< const UInt8* >( packet.GetConstBufferPtr() ), packet.GetDataSize(), 5000, 50000, sink, header, lostBefore );
}

/*-------------------------------------------------------------------------*/

/** Watches a connector */
class CSecurityConnectorRecorder : public CORE::CObserver
{
    public:
    CSecurityConnectorRecorder( void ) : connected( 0 ), failed( 0 ), failReason( DRN::DRN_LINKREASON_NONE ), failText() {}
    virtual ~CSecurityConnectorRecorder() { SignalUpcomingObserverDestruction(); }
    UInt32 connected;
    UInt32 failed;
    DRN::TDRNLinkReason failReason;
    CORE::CString failText;
    protected:
    virtual void OnNotify( CORE::CNotifier* /* notifier */, const CORE::CEvent& eventid, CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( DRN::CDRNConnector::ConnectedEvent == eventid )
            ++connected;
        else
        if ( DRN::CDRNConnector::ConnectFailedEvent == eventid )
        {
            if ( GUCEF_NULL != eventdata )
            {
                const DRN::CDRNLinkClosedInfo& info = static_cast< DRN::CDRNLinkClosedEventData* >( eventdata )->GetData();
                failReason = info.reason;
                failText = info.text;
            }
            ++failed;
        }
    }
};

/*-------------------------------------------------------------------------*/

/** Watches the peer reports of a node */
class CAnomalyRecorder : public CORE::CObserver
{
    public:
    CAnomalyRecorder( void ) : limitEvents( 0 ), clockEvents( 0 ), trafficEvents( 0 ), lastLimit(), lastClock(), lastTraffic() {}
    virtual ~CAnomalyRecorder() { SignalUpcomingObserverDestruction(); }
    UInt32 limitEvents;
    UInt32 clockEvents;
    UInt32 trafficEvents;
    DRN::CDRNLinkAnomaly lastLimit;
    DRN::CDRNLinkAnomaly lastClock;
    DRN::CDRNLinkAnomaly lastTraffic;
    protected:
    virtual void OnNotify( CORE::CNotifier* /* notifier */, const CORE::CEvent& eventid, CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( GUCEF_NULL == eventdata )
            return;
        if ( DRN::CDRNNode::PeerLimitExceededEvent == eventid )
        {
            lastLimit = static_cast< DRN::CDRNNodeLinkEventData* >( eventdata )->GetData().anomaly;
            ++limitEvents;
        }
        else
        if ( DRN::CDRNNode::ClockAnomalyEvent == eventid )
        {
            lastClock = static_cast< DRN::CDRNNodeLinkEventData* >( eventdata )->GetData().anomaly;
            ++clockEvents;
        }
        else
        if ( DRN::CDRNNode::TrafficAnomalyEvent == eventid )
        {
            lastTraffic = static_cast< DRN::CDRNNodeLinkEventData* >( eventdata )->GetData().anomaly;
            ++trafficEvents;
        }
    }
};

/*-------------------------------------------------------------------------*/

/** Keeps the channels a listener accepts */
class CAcceptRecorder : public CORE::CObserver
{
    public:
    CAcceptRecorder( void ) : channels() {}
    virtual ~CAcceptRecorder() { SignalUpcomingObserverDestruction(); }
    GUCEF::vector< DRN::CIDRNChannelPtr > channels;
    protected:
    virtual void OnNotify( CORE::CNotifier* /* notifier */, const CORE::CEvent& eventid, CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( DRN::CIDRNChannelListener::ChannelAcceptedEvent == eventid && GUCEF_NULL != eventdata )
            channels.push_back( static_cast< DRN::CDRNChannelEventData* >( eventdata )->GetData() );
    }
};

/*-------------------------------------------------------------------------*/

/** Keeps the datagrams a UDP socket receives */
class CDatagramRecorder : public CORE::CObserver
{
    public:
    CDatagramRecorder( void ) : datagrams(), count( 0 ) {}
    virtual ~CDatagramRecorder() { SignalUpcomingObserverDestruction(); }
    GUCEF::vector< CORE::CDynamicBuffer > datagrams;
    UInt32 count;
    protected:
    virtual void OnNotify( CORE::CNotifier* /* notifier */, const CORE::CEvent& eventid, CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( COMCORE::CUDPSocket::UDPPacketsRecievedEvent != eventid || GUCEF_NULL == eventdata )
            return;
        const COMCORE::CUDPSocket::TUdpPacketsRecievedEventData& packets = static_cast< COMCORE::CUDPSocket::UDPPacketsRecievedEventData* >( eventdata )->GetData();
        for ( UInt32 i=0; i<packets.packetsReceived && i<packets.packets.size(); ++i )
        {
            if ( packets.packets[ i ].dataBuffer.IsNULL() )
                continue;
            const CORE::CDynamicBuffer& data = packets.packets[ i ].dataBuffer.GetData();
            datagrams.push_back( CORE::CDynamicBuffer() );
            datagrams.back().CopyFrom( 0, data.GetDataSize(), data.GetConstBufferPtr() );
            ++count;
        }
    }
};

/*-------------------------------------------------------------------------*/

/** What a session tells the application, as far as these tests need it */
class CSecuritySessionRecorder : public CORE::CObserver
{
    public:
    CSecuritySessionRecorder( void ) : joined( 0 ), joinFailed( 0 ), appMessages( 0 ), localPeerId( 0 ) {}
    virtual ~CSecuritySessionRecorder() { SignalUpcomingObserverDestruction(); }
    UInt32 joined;
    UInt32 joinFailed;
    UInt32 appMessages;
    UInt32 localPeerId;
    protected:
    virtual void OnNotify( CORE::CNotifier* /* notifier */, const CORE::CEvent& eventid, CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( DRN::CDRNSession::JoinedEvent == eventid )
        {
            localPeerId = static_cast< CORE::TCloneableUInt32* >( eventdata )->GetData();
            ++joined;
        }
        else
        if ( DRN::CDRNSession::JoinFailedEvent == eventid )
            ++joinFailed;
        else
        if ( DRN::CDRNSession::AppMessageEvent == eventid )
            ++appMessages;
    }
};

/*-------------------------------------------------------------------------*/

/** A host and a member of one session over tcp */
class CSessionPair
{
    public:

    DRN::CDRNNode hostNode;
    DRN::CDRNNode memberNode;
    DRN::CDRNSession* host;
    DRN::CDRNSession* member;
    CSecuritySessionRecorder hostRecorder;
    CSecuritySessionRecorder memberRecorder;

    CSessionPair( void ) : hostNode(), memberNode(), host( GUCEF_NULL ), member( GUCEF_NULL ), hostRecorder(), memberRecorder() {}

    ~CSessionPair()
    {
        memberNode.Stop();
        hostNode.Stop();
    }

    bool Setup( const DRN::CDRNNodeSettings& hostSettings, const DRN::CDRNNodeSettings& memberSettings, const CORE::CString& sessionName )
    {
        if ( !hostNode.SetSettings( hostSettings ) || !memberNode.SetSettings( memberSettings ) )
            return false;
        DRN::CDRNEndpoint endpoint = LoopbackEndpoint( "tcp", NextTestPort() );
        if ( !hostNode.Listen( endpoint ) )
            return false;
        host = hostNode.CreateSession();
        member = memberNode.CreateSession();
        hostRecorder.SubscribeTo( host );
        memberRecorder.SubscribeTo( member );
        DRN::CDRNSessionSettings sessionSettings;
        sessionSettings.sessionName = sessionName;
        if ( !host->Host( sessionSettings ) )
            return false;
        if ( !member->Join( DRN::CDRNEndpointList( endpoint ), sessionSettings ) )
            return false;
        return PumpUntil( memberRecorder.joined, 1, 10000 );
    }
};

/*-------------------------------------------------------------------------*/

/** The node has the number of links */
class CLinkCountCondition : public CTestCondition
{
    public:
    CLinkCountCondition( DRN::CDRNNode& node, UInt32 expected ) : m_node( node ), m_expected( expected ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return m_node.GetLinkCount() == m_expected; }
    private:
    DRN::CDRNNode& m_node;
    UInt32 m_expected;
};

/*-------------------------------------------------------------------------*/

/** The UDP listener has the number of channels */
class CChannelCountCondition : public CTestCondition
{
    public:
    CChannelCountCondition( DRN::CDRNUdpChannelListener& listener, UInt32 expected ) : m_listener( listener ), m_expected( expected ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return m_listener.GetChannelCount() == m_expected; }
    private:
    DRN::CDRNUdpChannelListener& m_listener;
    UInt32 m_expected;
};

/*-------------------------------------------------------------------------*/

/** True if the bytes of the text occur in the buffer */
static bool
BufferContains( const CORE::CDynamicBuffer& buffer, const CORE::CString& text )
{
    UInt32 textSize = text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
    const char* data = static_cast< const char* >( buffer.GetConstBufferPtr() );
    UInt32 size = buffer.GetDataSize();
    if ( 0 == textSize || textSize > size || GUCEF_NULL == data )
        return false;
    for ( UInt32 i=0; i+textSize<=size; ++i )
    {
        if ( 0 == memcmp( data + i, text.C_String(), textSize ) )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/** Floods a link with messages until the peer closed it or the time is up */
static void
Flood( DRN::CDRNPeerLinkPtr& link, UInt8 lane, const UInt32& peerClosedCount, UInt32 timeoutInMs )
{
    CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    UInt64 start = DRN::CDRNClock::NowInMilliseconds();
    UInt8 payload[ 8 ] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    while ( 0 == peerClosedCount && DRN::CDRNClock::NowInMilliseconds() - start < timeoutInMs )
    {
        if ( link->IsOperational() )
        {
            for ( UInt32 i=0; i<20; ++i )
                link->Send( lane, DRN::DRN_MSGCLASS_APP, 1, payload, sizeof( payload ) );
            link->Flush();
        }
        pulseGenerator->ForceDirectPulse();
        MT::ThreadDelay( 1 );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDRNSecurityTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefDRN.Security" );

    GUCEF_TESTFW_TESTCASE( "Security: the policy round trips through a config and its hash is stable" )
    try
    {
        DRN::CDRNSecurityPolicy defaults;
        ASSERT_TRUE( defaults.requireEncryption && !defaults.requireSignedTokens );
        ASSERT_TRUE( defaults.GetPolicyHashString() == DefaultPolicyHash );

        DRN::CDRNSecurityPolicy policy;
        policy.requireEncryption = true;
        policy.ciphers.clear();
        policy.ciphers.push_back( "aes-256-gcm" );
        policy.ciphers.push_back( "chacha20-poly1305" );
        policy.allowedChannelTypes.insert( "udp" );
        policy.allowedChannelTypes.insert( "relay" );
        policy.relayOnly = true;
        policy.allowedAuthenticators.insert( "token" );
        policy.statelessCookies = false;
        policy.limits.maxMessagesPerSecond = 300;
        policy.limits.maxFragmentGroups = 7;
        policy.anomalies.clockDriftPercent = 3;
        policy.anomalies.burstPackets = 12;

        CORE::CDataNode cfg( "DRNSecurityPolicy" );
        ASSERT_TRUE( policy.SaveConfig( cfg ) );
        DRN::CDRNSecurityPolicy loaded;
        ASSERT_TRUE( loaded.LoadConfig( cfg ) );
        ASSERT_TRUE( loaded == policy );
        ASSERT_TRUE( loaded.GetPolicyHashString() == policy.GetPolicyHashString() );
        ASSERT_TRUE( loaded.limits.maxMessagesPerSecond == 300 && loaded.anomalies.burstPackets == 12 && !loaded.statelessCookies );
        ASSERT_TRUE( 2 == loaded.ciphers.size() && loaded.ciphers[ 0 ] == "aes-256-gcm" );

        // the same meaning gives the same hash: case and set order do not count, the cipher order does
        DRN::CDRNSecurityPolicy sameMeaning = policy;
        sameMeaning.allowedChannelTypes.clear();
        sameMeaning.allowedChannelTypes.insert( "RELAY" );
        sameMeaning.allowedChannelTypes.insert( "Udp" );
        ASSERT_TRUE( sameMeaning.GetPolicyHashString() == policy.GetPolicyHashString() );
        DRN::CDRNSecurityPolicy otherOrder = policy;
        otherOrder.ciphers[ 0 ] = "chacha20-poly1305";
        otherOrder.ciphers[ 1 ] = "aes-256-gcm";
        ASSERT_TRUE( otherOrder.GetPolicyHashString() != policy.GetPolicyHashString() );
        DRN::CDRNSecurityPolicy otherLimit = policy;
        otherLimit.limits.maxBytesPerSecond += 1;
        ASSERT_TRUE( otherLimit.GetPolicyHashString() != policy.GetPolicyHashString() );
        ASSERT_TRUE( otherLimit != policy );

        // carried in the node and the session settings
        DRN::CDRNNodeSettings nodeSettings;
        nodeSettings.securityPolicy = policy;
        CORE::CDataNode nodeCfg( "DRNNodeSettings" );
        ASSERT_TRUE( nodeSettings.SaveConfig( nodeCfg ) );
        DRN::CDRNNodeSettings nodeLoaded;
        ASSERT_TRUE( nodeLoaded.LoadConfig( nodeCfg ) );
        ASSERT_TRUE( nodeLoaded.securityPolicy == policy );
        DRN::CDRNSessionSettings sessionSettings;
        sessionSettings.securityPolicy = policy;
        CORE::CDataNode sessionCfg( "DRNSessionSettings" );
        ASSERT_TRUE( sessionSettings.SaveConfig( sessionCfg ) );
        DRN::CDRNSessionSettings sessionLoaded;
        ASSERT_TRUE( sessionLoaded.LoadConfig( sessionCfg ) );
        ASSERT_TRUE( sessionLoaded.securityPolicy.GetPolicyHashString() == policy.GetPolicyHashString() );

        // endpoints under relayOnly and the channel types
        ASSERT_TRUE( policy.IsEndpointAllowed( DRN::CDRNEndpoint( "relay://127.0.0.1:4000/realm/3" ) ) );
        ASSERT_FALSE( policy.IsEndpointAllowed( DRN::CDRNEndpoint( "udp://127.0.0.1:4000" ) ) );
        ASSERT_FALSE( policy.IsChannelTypeAllowed( "tcp" ) );
        ASSERT_TRUE( policy.IsAuthenticatorAllowed( "TOKEN" ) && !policy.IsAuthenticatorAllowed( "anonymous" ) );

        // weaker than a minimum
        {
            DRN::CDRNSecurityPolicy minimum;
            DRN::CDRNSecurityPolicy candidate;
            CORE::CString why;
            ASSERT_FALSE( candidate.IsWeakerThan( minimum, why ) );
            candidate.requireEncryption = false;
            ASSERT_TRUE( candidate.IsWeakerThan( minimum, why ) && why == "it does not require encryption" );
            candidate.requireEncryption = true;
            minimum.relayOnly = true;
            ASSERT_TRUE( candidate.IsWeakerThan( minimum, why ) && why == "it is not relay only" );
            candidate.relayOnly = true;
            candidate.ciphers.push_back( "rot13" );
            ASSERT_TRUE( candidate.IsWeakerThan( minimum, why ) && why == "it allows the cipher rot13" );
            candidate.ciphers.pop_back();
            minimum.allowedChannelTypes.insert( "udp" );
            ASSERT_TRUE( candidate.IsWeakerThan( minimum, why ) && why == "it allows the channel type any" );
            candidate.allowedChannelTypes.insert( "UDP" );
            ASSERT_FALSE( candidate.IsWeakerThan( minimum, why ) );
            candidate.limits.maxBytesPerSecond = 0;
            ASSERT_TRUE( candidate.IsWeakerThan( minimum, why ) && why.HasSubstr( "maxBytesPerSecond has no limit", true ) >= 0 );
            candidate.limits.maxBytesPerSecond = minimum.limits.maxBytesPerSecond / 2;
            ASSERT_FALSE( candidate.IsWeakerThan( minimum, why ) );
            minimum.requireSignedTokens = true;
            ASSERT_TRUE( candidate.IsWeakerThan( minimum, why ) && why == "it does not require signed tokens" );
            ASSERT_FALSE( minimum.IsWeakerThan( minimum, why ) );
        }

        // secrets of any size compare in constant time
        ASSERT_TRUE( DRN::CDRNCrypto::ConstantTimeEqualsHashed( "secret", 6, "secret", 6 ) );
        ASSERT_FALSE( DRN::CDRNCrypto::ConstantTimeEqualsHashed( "secret", 6, "secreT", 6 ) );
        ASSERT_FALSE( DRN::CDRNCrypto::ConstantTimeEqualsHashed( "secret", 6, "secret!", 7 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Security: a policy that cannot be honoured stops nodes, sessions and connectors with a clear reason" )
    try
    {
        // encryption with only a cipher nobody has
        DRN::CDRNNodeSettings settings = SecurityNodeSettings();
        settings.securityPolicy.requireEncryption = true;
        settings.securityPolicy.ciphers.clear();
        settings.securityPolicy.ciphers.push_back( "rot13" );
        CORE::CString whyNot;
        ASSERT_FALSE( settings.securityPolicy.CanBeHonoured( whyNot ) );
        ASSERT_TRUE( whyNot.HasSubstr( "encryption", true ) >= 0 );

        DRN::CDRNNode node;
        REQUIRE_TRUE( node.SetSettings( settings ) );
        ASSERT_FALSE( node.Start() );
        ASSERT_FALSE( node.Listen( LoopbackEndpoint( "tcp", NextTestPort() ) ) );
        DRN::CDRNSession* session = node.CreateSession();
        DRN::CDRNSessionSettings sessionSettings;
        sessionSettings.sessionName = "encrypted";
        ASSERT_FALSE( session->Host( sessionSettings ) );
        ASSERT_FALSE( session->Join( DRN::CDRNEndpointList( LoopbackEndpoint( "tcp", NextTestPort() ) ), sessionSettings ) );
        DRN::CDRNConnectorPtr connector = node.Connect( DRN::CDRNEndpointList( LoopbackEndpoint( "tcp", NextTestPort() ) ) );
        ASSERT_TRUE( connector->IsDone() && !connector->HasSucceeded() );
        connector.Unlink();

        // a session policy that requires it stops the session on a node that is fine otherwise
        DRN::CDRNNode plainNode;
        REQUIRE_TRUE( plainNode.SetSettings( SecurityNodeSettings() ) );
        DRN::CDRNSession* plainSession = plainNode.CreateSession();
        sessionSettings.securityPolicy.requireEncryption = true;
        sessionSettings.securityPolicy.ciphers.clear();
        sessionSettings.securityPolicy.ciphers.push_back( "rot13" );
        ASSERT_FALSE( plainSession->Host( sessionSettings ) );

        // channel types and authenticators the policy does not allow
        DRN::CDRNNodeSettings tcpOnly = SecurityNodeSettings();
        tcpOnly.securityPolicy.allowedChannelTypes.insert( "tcp" );
        DRN::CDRNNode tcpNode;
        REQUIRE_TRUE( tcpNode.SetSettings( tcpOnly ) );
        ASSERT_FALSE( tcpNode.Listen( LoopbackEndpoint( "udp", NextTestPort() ) ) );
        CSecurityConnectorRecorder recorder;
        DRN::CDRNConnectorPtr udpConnector = tcpNode.Connect( DRN::CDRNEndpointList( LoopbackEndpoint( "udp", NextTestPort() ) ) );
        recorder.SubscribeTo( udpConnector.GetPointerAlways() );
        ASSERT_TRUE( udpConnector->IsDone() && !udpConnector->HasSucceeded() );
        udpConnector.Unlink();

        DRN::CDRNNodeSettings tokenOnly = SecurityNodeSettings();
        tokenOnly.securityPolicy.allowedAuthenticators.insert( "token" );
        DRN::CDRNNode tokenNode;
        ASSERT_FALSE( tokenNode.SetSettings( tokenOnly ) );       // the settings say "anonymous"

        // relay only: a host reached directly is not joined
        DRN::CDRNSessionSettings relayOnlySettings;
        relayOnlySettings.sessionName = "hidden";
        relayOnlySettings.securityPolicy.relayOnly = true;
        DRN::CDRNSession* hiddenSession = plainNode.CreateSession();
        ASSERT_FALSE( hiddenSession->Join( DRN::CDRNEndpointList( LoopbackEndpoint( "tcp", NextTestPort() ) ), relayOnlySettings ) );

        node.Stop();
        plainNode.Stop();
        tcpNode.Stop();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Security: before the link is operational fragments and reliable lane state are refused" )
    try
    {
        CCountingSink sink;
        DRN::CDRNLinkReliability receiver;
        receiver.Reset( false, 1200, 1 );
        receiver.SetPreAuthRestricted( true );
        receiver.SetReceiveLimits( 16 * 1024 * 1024, 64, 32 * 1024 * 1024 );

        // a flood of fragments of 16 MB messages on both reliable lanes: nothing is kept
        CORE::CDynamicBuffer packet;
        for ( UInt32 i=0; i<200; ++i )
        {
            MakeFragmentPacket( packet, i + 1, (UInt8) ( i % 2 ), i / 2, i + 1, 16 * 1024 * 1024 );
            ASSERT_TRUE( DRN::CDRNLinkReliability::PACKET_REFUSED == Feed( receiver, packet, sink ) );
        }
        ASSERT_TRUE( 0 == receiver.GetOpenFragmentGroups() && 0 == receiver.GetOpenFragmentBytes() );
        ASSERT_TRUE( 200 == receiver.GetStats().preAuthRefusedPackets && 0 == receiver.GetStats().packetsReceived && 0 == sink.count );

        // other classes, lane 2, and lane 0 messages far ahead are refused as well
        UInt8 bytes[ 32 ];
        memset( bytes, 7, sizeof( bytes ) );
        StartPacket( packet, 300 );
        AddMessage( packet, DRN::DRN_MSGCLASS_APP, 1, DRN::DRN_LANE_RELIABLE_ORDERED, 0, bytes, sizeof( bytes ) );
        ASSERT_TRUE( DRN::CDRNLinkReliability::PACKET_REFUSED == Feed( receiver, packet, sink ) );
        StartPacket( packet, 301 );
        AddMessage( packet, DRN::DRN_MSGCLASS_LINK, DRN::DRN_LINK_ECHO, DRN::DRN_LANE_UNRELIABLE_SEQUENCED, 0, GUCEF_NULL, 0 );
        ASSERT_TRUE( DRN::CDRNLinkReliability::PACKET_REFUSED == Feed( receiver, packet, sink ) );
        StartPacket( packet, 302 );
        AddMessage( packet, DRN::DRN_MSGCLASS_LINK, DRN::DRN_LINK_HELLO, DRN::DRN_LANE_RELIABLE_ORDERED, 500, bytes, sizeof( bytes ) );
        ASSERT_TRUE( DRN::CDRNLinkReliability::PACKET_REFUSED == Feed( receiver, packet, sink ) );
        ASSERT_TRUE( 0 == sink.count );

        // the handshake itself goes through, and with WELCOME what follows it too
        StartPacket( packet, 303 );
        AddMessage( packet, DRN::DRN_MSGCLASS_LINK, DRN::DRN_LINK_HELLO, DRN::DRN_LANE_RELIABLE_ORDERED, 0, bytes, sizeof( bytes ) );
        ASSERT_TRUE( DRN::CDRNLinkReliability::PACKET_ACCEPTED == Feed( receiver, packet, sink ) );
        ASSERT_TRUE( 1 == sink.count );
        StartPacket( packet, 304 );
        AddMessage( packet, DRN::DRN_MSGCLASS_LINK, DRN::DRN_LINK_WELCOME, DRN::DRN_LANE_RELIABLE_ORDERED, 1, bytes, 4 );
        AddMessage( packet, DRN::DRN_MSGCLASS_APP, 1, DRN::DRN_LANE_RELIABLE_ORDERED, 2, bytes, sizeof( bytes ) );
        ASSERT_TRUE( DRN::CDRNLinkReliability::PACKET_ACCEPTED == Feed( receiver, packet, sink ) );
        ASSERT_TRUE( 3 == sink.count && DRN::DRN_MSGCLASS_APP == sink.lastClass );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Security: fragments are capped in groups, bytes and message size" )
    try
    {
        CCountingSink sink;
        CORE::CDynamicBuffer packet;

        DRN::CDRNLinkReliability groups;
        groups.Reset( false, 1200, 1 );
        groups.SetReceiveLimits( 1024 * 1024, 2, 2 * 1024 * 1024 );
        MakeFragmentPacket( packet, 1, DRN::DRN_LANE_RELIABLE_UNORDERED, 0, 1, 100000 );
        ASSERT_TRUE( DRN::CDRNLinkReliability::PACKET_ACCEPTED == Feed( groups, packet, sink ) );
        MakeFragmentPacket( packet, 2, DRN::DRN_LANE_RELIABLE_UNORDERED, 1, 2, 100000 );
        ASSERT_TRUE( DRN::CDRNLinkReliability::PACKET_ACCEPTED == Feed( groups, packet, sink ) );
        ASSERT_TRUE( 2 == groups.GetOpenFragmentGroups() && 200000 == groups.GetOpenFragmentBytes() );
        MakeFragmentPacket( packet, 3, DRN::DRN_LANE_RELIABLE_UNORDERED, 2, 3, 100000 );
        ASSERT_TRUE( DRN::CDRNLinkReliability::PACKET_LIMIT_EXCEEDED == Feed( groups, packet, sink ) );
        ASSERT_TRUE( DRN::CDRNLinkReliability::LIMIT_FRAGMENT_GROUPS == groups.GetLimitViolation() );
        ASSERT_TRUE( 2 == groups.GetOpenFragmentGroups() );

        DRN::CDRNLinkReliability bytes;
        bytes.Reset( false, 1200, 1 );
        bytes.SetReceiveLimits( 1024 * 1024, 8, 1024 * 1024 );
        MakeFragmentPacket( packet, 1, DRN::DRN_LANE_RELIABLE_UNORDERED, 0, 1, 600 * 1024 );
        ASSERT_TRUE( DRN::CDRNLinkReliability::PACKET_ACCEPTED == Feed( bytes, packet, sink ) );
        MakeFragmentPacket( packet, 2, DRN::DRN_LANE_RELIABLE_UNORDERED, 1, 2, 600 * 1024 );
        ASSERT_TRUE( DRN::CDRNLinkReliability::PACKET_LIMIT_EXCEEDED == Feed( bytes, packet, sink ) );
        ASSERT_TRUE( DRN::CDRNLinkReliability::LIMIT_FRAGMENT_BYTES == bytes.GetLimitViolation() );
        ASSERT_TRUE( 600 * 1024 == bytes.GetOpenFragmentBytes() );

        DRN::CDRNLinkReliability size;
        size.Reset( false, 1200, 1 );
        size.SetReceiveLimits( 1024 * 1024, 8, 4 * 1024 * 1024 );
        MakeFragmentPacket( packet, 1, DRN::DRN_LANE_RELIABLE_ORDERED, 0, 1, 2 * 1024 * 1024 );
        ASSERT_TRUE( DRN::CDRNLinkReliability::PACKET_LIMIT_EXCEEDED == Feed( size, packet, sink ) );
        ASSERT_TRUE( DRN::CDRNLinkReliability::LIMIT_MESSAGE_SIZE == size.GetLimitViolation() );
        ASSERT_TRUE( 0 == size.GetOpenFragmentGroups() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Security: a stream peer that sends more than the handshake, or a large packet, before it authenticated is closed" )
    try
    {
        DRN::CDRNNode node;
        REQUIRE_TRUE( node.SetSettings( SecurityNodeSettings() ) );
        CLinkRecorder recorder;
        recorder.WatchNode( node );
        UInt16 port = NextTestPort();
        REQUIRE_TRUE( node.Listen( LoopbackEndpoint( "tcp", port ) ) );

        // a fragment of a 16 MB message as the first packet
        COMCORE::CTCPClientSocket fragmenter( CORE::CCoreGlobal::Instance()->GetPulseGenerator(), false );
        REQUIRE_TRUE( fragmenter.ConnectTo( "127.0.0.1", port ) );
        CLinkCountCondition oneLink( node, 1 );
        REQUIRE_TRUE( PumpUntil( oneLink, 3000 ) );
        CORE::CDynamicBuffer packet;
        MakeFragmentPacket( packet, 1, DRN::DRN_LANE_RELIABLE_ORDERED, 0, 1, 16 * 1024 * 1024 );
        UInt8 frameHeader[ DRN::CDRNStreamFramer::MaxFrameHeaderSize ];
        UInt32 frameHeaderSize = DRN::CDRNStreamFramer::WriteFrameHeader( packet.GetDataSize(), frameHeader );
        ASSERT_TRUE( fragmenter.Send( frameHeader, frameHeaderSize ) );
        ASSERT_TRUE( fragmenter.Send( packet.GetConstBufferPtr(), packet.GetDataSize() ) );
        ASSERT_TRUE( PumpUntil( recorder.closedCount, 1, 3000 ) );
        ASSERT_TRUE( DRN::DRN_LINKREASON_PROTOCOL_ERROR == recorder.lastClosedReason );
        fragmenter.Close();

        // a frame that announces a packet of a megabyte: not collected before the peer authenticated
        COMCORE::CTCPClientSocket bigSender( CORE::CCoreGlobal::Instance()->GetPulseGenerator(), false );
        REQUIRE_TRUE( bigSender.ConnectTo( "127.0.0.1", port ) );
        PumpFor( 100 );
        frameHeaderSize = DRN::CDRNStreamFramer::WriteFrameHeader( 1024 * 1024, frameHeader );
        ASSERT_TRUE( bigSender.Send( frameHeader, frameHeaderSize ) );
        UInt8 filler[ 1024 ];
        memset( filler, 0xD1, sizeof( filler ) );
        ASSERT_TRUE( bigSender.Send( filler, sizeof( filler ) ) );
        ASSERT_TRUE( PumpUntil( recorder.closedCount, 2, 3000 ) );
        ASSERT_TRUE( 0 == recorder.operationalCount );
        PumpFor( 50 );
        ASSERT_TRUE( 0 == node.GetLinkCount() );
        bigSender.Close();
        node.Stop();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Security: a web socket peer that sends a large message before it authenticated is closed" )
    try
    {
        if ( !EnsureHashCodecPluginLoaded() )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "The web socket pre-auth test is skipped: no sha1 codec" );
        }
        else
        {
            DRN::CDRNNode node;
            REQUIRE_TRUE( node.SetSettings( SecurityNodeSettings() ) );
            CLinkRecorder recorder;
            recorder.WatchNode( node );
            UInt16 port = NextTestPort();
            REQUIRE_TRUE( node.Listen( LoopbackEndpoint( "websocket", port ) ) );

            WEB::CWebSocketClient client( CORE::CCoreGlobal::Instance()->GetPulseGenerator() );
            REQUIRE_TRUE( client.Connect( "ws://127.0.0.1:" + CORE::UInt32ToString( port ) + "/drn" ) );
            CLinkCountCondition oneLink( node, 1 );
            REQUIRE_TRUE( PumpUntil( oneLink, 3000 ) );
            PumpFor( 100 );

            // more than maxPreAuthPacketSize (16 KB) as one message: the connection refuses it and closes
            CORE::CDynamicBuffer large;
            large.SetDataSize( 100 * 1024 );
            memset( large.GetBufferPtr(), DRN::DRN_PROTOCOL_ID, large.GetDataSize() );
            ASSERT_TRUE( client.SendBinary( large.GetConstBufferPtr(), large.GetDataSize() ) );
            ASSERT_TRUE( PumpUntil( recorder.closedCount, 1, 3000 ) );
            ASSERT_TRUE( 0 == recorder.operationalCount );
            CLinkCountCondition noLink( node, 0 );
            ASSERT_TRUE( PumpUntil( noLink, 3000 ) );
            client.Disconnect();
            node.Stop();
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Security: handshakes are limited per node and per address" )
    try
    {
        // pending handshakes
        DRN::CDRNNodeSettings settings = SecurityNodeSettings();
        settings.handshakeTimeoutMs = 5000;
        settings.securityPolicy.limits.maxPendingHandshakes = 3;
        DRN::CDRNNode node;
        REQUIRE_TRUE( node.SetSettings( settings ) );
        UInt16 port = NextTestPort();
        REQUIRE_TRUE( node.Listen( LoopbackEndpoint( "tcp", port ) ) );
        GUCEF::vector< COMCORE::CTCPClientSocket* > silentClients;
        for ( UInt32 i=0; i<8; ++i )
        {
            COMCORE::CTCPClientSocket* client = GUCEF_NEW COMCORE::CTCPClientSocket( CORE::CCoreGlobal::Instance()->GetPulseGenerator(), false );
            silentClients.push_back( client );
            client->ConnectTo( "127.0.0.1", port );
        }
        UInt64 start = DRN::CDRNClock::NowInMilliseconds();
        while ( node.GetRefusedAcceptCount() < 5 && DRN::CDRNClock::NowInMilliseconds() - start < 3000 )
            PumpFor( 10 );
        ASSERT_TRUE( 3 == node.GetPendingHandshakeCount() );
        ASSERT_TRUE( 5 == node.GetRefusedAcceptCount() );
        for ( UInt32 i=0; i<silentClients.size(); ++i )
        {
            silentClients[ i ]->Close();
            GUCEF_DELETE silentClients[ i ];
        }
        silentClients.clear();
        node.Stop();

        // handshakes per address per minute
        DRN::CDRNNodeSettings rateSettings = SecurityNodeSettings();
        rateSettings.securityPolicy.limits.maxHandshakesPerAddressPerMinute = 4;
        DRN::CDRNNode rateNode;
        REQUIRE_TRUE( rateNode.SetSettings( rateSettings ) );
        UInt16 ratePort = NextTestPort();
        REQUIRE_TRUE( rateNode.Listen( LoopbackEndpoint( "tcp", ratePort ) ) );
        for ( UInt32 i=0; i<6; ++i )
        {
            COMCORE::CTCPClientSocket* client = GUCEF_NEW COMCORE::CTCPClientSocket( CORE::CCoreGlobal::Instance()->GetPulseGenerator(), false );
            silentClients.push_back( client );
            client->ConnectTo( "127.0.0.1", ratePort );
        }
        start = DRN::CDRNClock::NowInMilliseconds();
        while ( rateNode.GetRefusedAcceptCount() < 2 && DRN::CDRNClock::NowInMilliseconds() - start < 3000 )
            PumpFor( 10 );
        ASSERT_TRUE( 2 == rateNode.GetRefusedAcceptCount() );
        ASSERT_TRUE( 4 == rateNode.GetPendingHandshakeCount() );

        // the address used up its handshakes: a real peer from it is refused too
        DRN::CDRNNode peer;
        DRN::CDRNNodeSettings peerSettings = SecurityNodeSettings();
        peerSettings.handshakeTimeoutMs = 500;
        peerSettings.connectTimeoutMs = 1000;
        REQUIRE_TRUE( peer.SetSettings( peerSettings ) );
        CSecurityConnectorRecorder connectorRecorder;
        DRN::CDRNConnectorPtr connector = peer.Connect( DRN::CDRNEndpointList( LoopbackEndpoint( "tcp", ratePort ) ) );
        connectorRecorder.SubscribeTo( connector.GetPointerAlways() );
        ASSERT_TRUE( PumpUntil( connectorRecorder.failed, 1, 5000 ) );
        ASSERT_TRUE( 3 == rateNode.GetRefusedAcceptCount() );

        CORE::CDataNode telemetry;
        rateNode.GetTelemetry( telemetry );
        ASSERT_TRUE( 3 == telemetry.GetAttributeValueOrChildValueByName( "refusedForHandshakeRate" ).AsUInt32( 0, true ) );

        connector.Unlink();
        for ( UInt32 i=0; i<silentClients.size(); ++i )
        {
            silentClients[ i ]->Close();
            GUCEF_DELETE silentClients[ i ];
        }
        peer.Stop();
        rateNode.Stop();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Security: a UDP listener keeps nothing for an address until it echoes its cookie" )
    try
    {
        DRN::CDRNUdpChannelListener listener;
        listener.SetPulseGenerator( CORE::CCoreGlobal::Instance()->GetPulseGenerator() );
        CAcceptRecorder accepted;
        accepted.SubscribeTo( &listener );
        UInt16 port = NextTestPort();
        REQUIRE_TRUE( listener.Listen( LoopbackEndpoint( "udp", port ) ) );
        REQUIRE_TRUE( listener.IsUsingStatelessCookies() );
        COMCORE::CIPv4Address listenerAddress( "127.0.0.1", port );

        // a HELLO of an address that never sees the answer, as a spoofed source: answered with a cookie, nothing kept
        CORE::CDynamicBuffer hello;
        UInt8 helloBytes[ 32 ];
        memset( helloBytes, 1, sizeof( helloBytes ) );
        StartPacket( hello, 1 );
        AddMessage( hello, DRN::DRN_MSGCLASS_LINK, DRN::DRN_LINK_HELLO, DRN::DRN_LANE_RELIABLE_ORDERED, 0, helloBytes, sizeof( helloBytes ) );

        COMCORE::CUDPSocket source( CORE::CCoreGlobal::Instance()->GetPulseGenerator(), false );
        CDatagramRecorder sourceDatagrams;
        sourceDatagrams.SubscribeTo( &source );
        REQUIRE_TRUE( source.Open( (UInt16) 0 ) );
        for ( UInt32 i=0; i<20; ++i )
            source.SendPacketTo( listenerAddress, hello.GetConstBufferPtr(), (UInt16) hello.GetDataSize() );
        ASSERT_TRUE( PumpUntil( sourceDatagrams.count, 20, 3000 ) );
        ASSERT_TRUE( 20 == listener.GetCookieRetryCount() );
        ASSERT_TRUE( 0 == listener.GetChannelCount() && accepted.channels.empty() );
        const CORE::CDynamicBuffer& retry = sourceDatagrams.datagrams[ 0 ];
        REQUIRE_TRUE( DRN::DRN_UDP_COOKIE_SIZE == retry.GetDataSize() );
        ASSERT_TRUE( DRN::DRN_UDP_HELLO_RETRY == *static_cast< const UInt8* >( retry.GetConstBufferPtr() ) );
        ASSERT_TRUE( retry.GetDataSize() <= hello.GetDataSize() );

        // a made up cookie, and the cookie of another address, get nothing
        CORE::CDynamicBuffer echo;
        UInt8 forged[ DRN::DRN_UDP_COOKIE_SIZE ];
        memcpy( forged, retry.GetConstBufferPtr(), DRN::DRN_UDP_COOKIE_SIZE );
        forged[ 0 ] = DRN::DRN_UDP_COOKIE_ECHO;
        forged[ 10 ] ^= 0x5A;
        echo.CopyFrom( 0, sizeof( forged ), forged );
        echo.CopyFrom( sizeof( forged ), hello.GetDataSize(), hello.GetConstBufferPtr() );
        source.SendPacketTo( listenerAddress, echo.GetConstBufferPtr(), (UInt16) echo.GetDataSize() );

        UInt8 cookie[ DRN::DRN_UDP_COOKIE_SIZE ];
        memcpy( cookie, retry.GetConstBufferPtr(), DRN::DRN_UDP_COOKIE_SIZE );
        cookie[ 0 ] = DRN::DRN_UDP_COOKIE_ECHO;
        echo.SetDataSize( 0 );
        echo.CopyFrom( 0, sizeof( cookie ), cookie );
        echo.CopyFrom( sizeof( cookie ), hello.GetDataSize(), hello.GetConstBufferPtr() );
        COMCORE::CUDPSocket otherSource( CORE::CCoreGlobal::Instance()->GetPulseGenerator(), false );
        REQUIRE_TRUE( otherSource.Open( (UInt16) 0 ) );
        otherSource.SendPacketTo( listenerAddress, echo.GetConstBufferPtr(), (UInt16) echo.GetDataSize() );
        PumpFor( 100 );
        ASSERT_TRUE( 2 == listener.GetInvalidCookieCount() );
        ASSERT_TRUE( 0 == listener.GetChannelCount() && accepted.channels.empty() );

        // the address that got the cookie echoes it: now there is a channel, its packet handed over without the cookie
        source.SendPacketTo( listenerAddress, echo.GetConstBufferPtr(), (UInt16) echo.GetDataSize() );
        CChannelCountCondition oneChannel( listener, 1 );
        ASSERT_TRUE( PumpUntil( oneChannel, 3000 ) );
        ASSERT_TRUE( 1 == accepted.channels.size() );

        accepted.channels.clear();
        source.Close();
        otherSource.Close();
        listener.StopListening();

        // DRN links over udp do the cookie round on their own
        CLinkedPair pair;
        UInt16 pairPort = NextTestPort();
        REQUIRE_TRUE( pair.Setup( SecurityNodeSettings(), SecurityNodeSettings(), LoopbackEndpoint( "udp", pairPort ), LoopbackEndpoint( "udp", pairPort ) ) );
        ASSERT_TRUE( pair.linkA->IsOperational() && pair.linkB->IsOperational() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    const char* floodChannels[] = { "udp", "tcp" };
    for ( UInt32 c=0; c<2; ++c )
    {
        const CORE::CString channelType = floodChannels[ c ];
        const CORE::CString caseName = "Security: a peer over its receive limits is reported and, when it keeps on, closed, over " + channelType;
        GUCEF_TESTFW_TESTCASE( caseName.C_String() )
        try
        {
            DRN::CDRNNodeSettings settingsB = SecurityNodeSettings();
            settingsB.securityPolicy.limits.maxMessagesPerSecond = 200;
            settingsB.securityPolicy.limits.sustainedLimitSeconds = 1;
            CLinkedPair pair;
            CAnomalyRecorder anomalies;
            anomalies.SubscribeTo( &pair.nodeB );
            UInt16 port = NextTestPort();
            DRN::CDRNNodeSettings settingsA = settingsB;     // one policy for every node of a game
            settingsA.nodeId = DRN::CDRNNodeSettings::GenerateNodeId();
            REQUIRE_TRUE( pair.Setup( settingsA, settingsB, LoopbackEndpoint( channelType, port ), LoopbackEndpoint( channelType, port ) ) );

            UInt8 lane = "udp" == channelType ? (UInt8) DRN::DRN_LANE_UNRELIABLE : (UInt8) DRN::DRN_LANE_RELIABLE_ORDERED;
            Flood( pair.linkA, lane, pair.recorderB.closedCount, 8000 );
            ASSERT_TRUE( PumpUntil( pair.recorderB.closedCount, 1, 3000 ) );
            ASSERT_TRUE( DRN::DRN_LINKREASON_LIMIT_EXCEEDED == pair.recorderB.lastClosedReason );
            ASSERT_TRUE( anomalies.limitEvents >= 1 );
            ASSERT_TRUE( DRN::DRN_LIMIT_MESSAGES_PER_SECOND == anomalies.lastLimit.kind );
            ASSERT_TRUE( pair.linkB->GetMonitor().GetLimitHitCount( DRN::DRN_LIMIT_MESSAGES_PER_SECOND ) > 0 );
            ASSERT_TRUE( pair.linkB->GetStats().rateLimitedPackets > 0 );
            ASSERT_TRUE( PumpUntil( pair.recorderA.closedCount, 1, 3000 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        GUCEF_TESTFW_TESTCASE_END
    }

    GUCEF_TESTFW_TESTCASE( "Security: a ban list round trips through its file and its config, and keeps its changes" )
    try
    {
        UInt64 now = DRN::CDRNBanList::GetUnixTimeNow();
        const UInt8 key[ 4 ] = { 0x01, 0x02, 0xAB, 0xFF };
        const CORE::CString awkwardReason = "cheating\twith a\nline break and a \\ backslash";

        DRN::CDRNBanList bans;
        ASSERT_TRUE( bans.Ban( DRN::DRN_BAN_USER_ID, "eve", 0, awkwardReason ) );
        ASSERT_TRUE( bans.Ban( DRN::DRN_BAN_ADDRESS, "10.1.2.3", now + 3600, "flood" ) );
        ASSERT_TRUE( bans.Ban( DRN::DRN_BAN_PUBLIC_KEY, DRN::CDRNBanEntry::PublicKeyToString( key, sizeof( key ) ), 0, "key" ) );
        ASSERT_TRUE( bans.Ban( DRN::DRN_BAN_USER_ID, "old", now - 10, "expired" ) );
        ASSERT_TRUE( 3 == bans.GetCount() );
        ASSERT_TRUE( bans.IsUserBanned( "eve" ) && !bans.IsUserBanned( "old" ) && !bans.IsUserBanned( "alice" ) );
        ASSERT_TRUE( bans.IsAddressBanned( "10.1.2.3" ) && bans.IsPublicKeyBanned( key, sizeof( key ) ) );
        ASSERT_TRUE( "0102abff" == DRN::CDRNBanEntry::PublicKeyToString( key, sizeof( key ) ) );

        CORE::CString path = TestFilePath( "gucefDRN_TestApp_bans.txt" );
        ASSERT_TRUE( bans.SaveToFile( path ) );
        DRN::CDRNBanList loaded;
        ASSERT_TRUE( loaded.LoadFromFile( path ) );
        ASSERT_TRUE( 3 == loaded.GetCount() );
        DRN::CDRNBanEntry entry;
        ASSERT_TRUE( loaded.IsUserBanned( "eve", &entry ) && entry.reason == awkwardReason && 0 == entry.expiryUnixSeconds );
        ASSERT_TRUE( loaded.IsAddressBanned( "10.1.2.3", &entry ) && entry.expiryUnixSeconds == now + 3600 && entry.reason == "flood" );
        ASSERT_TRUE( loaded.IsPublicKeyBanned( key, sizeof( key ) ) );
        ASSERT_FALSE( loaded.IsUserBanned( "old" ) );

        CORE::CDataNode cfg( "DRNBanList" );
        ASSERT_TRUE( bans.SaveConfig( cfg ) );
        DRN::CDRNBanList fromConfig;
        ASSERT_TRUE( fromConfig.LoadConfig( cfg ) );
        ASSERT_TRUE( 3 == fromConfig.GetCount() && fromConfig.IsUserBanned( "eve", &entry ) && entry.reason == awkwardReason );

        // with a file every change is saved: a ban survives a restart, an unban too
        CORE::CString persistentPath = TestFilePath( "gucefDRN_TestApp_persistent_bans.txt" );
        CORE::DeleteFile( persistentPath );
        {
            DRN::CDRNBanList persistent;
            ASSERT_TRUE( persistent.LoadFromFile( persistentPath ) && 0 == persistent.GetCount() );
            persistent.SetFilePath( persistentPath );
            ASSERT_TRUE( persistent.Ban( DRN::DRN_BAN_USER_ID, "mallory", 0, "aimbot" ) );
            ASSERT_TRUE( persistent.Ban( DRN::DRN_BAN_USER_ID, "trent", 0, "spam" ) );
            ASSERT_TRUE( persistent.Unban( DRN::DRN_BAN_USER_ID, "trent" ) );
        }
        DRN::CDRNBanList restarted;
        ASSERT_TRUE( restarted.LoadFromFile( persistentPath ) );
        ASSERT_TRUE( restarted.IsUserBanned( "mallory", &entry ) && entry.reason == "aimbot" );
        ASSERT_FALSE( restarted.IsUserBanned( "trent" ) );

        CORE::DeleteFile( path );
        CORE::DeleteFile( persistentPath );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Security: a banned address is refused when its channel is accepted, a banned user at AUTH" )
    try
    {
        // the address
        DRN::CDRNBanList addressBans;
        addressBans.Ban( DRN::DRN_BAN_ADDRESS, "127.0.0.1", 0, "test" );
        DRN::CDRNNode host;
        REQUIRE_TRUE( host.SetSettings( SecurityNodeSettings() ) );
        host.SetBanList( &addressBans );
        UInt16 port = NextTestPort();
        REQUIRE_TRUE( host.Listen( LoopbackEndpoint( "tcp", port ) ) );
        DRN::CDRNNode client;
        DRN::CDRNNodeSettings clientSettings = SecurityNodeSettings();
        clientSettings.handshakeTimeoutMs = 500;
        clientSettings.connectTimeoutMs = 1000;
        REQUIRE_TRUE( client.SetSettings( clientSettings ) );
        CSecurityConnectorRecorder refused;
        DRN::CDRNConnectorPtr connector = client.Connect( DRN::CDRNEndpointList( LoopbackEndpoint( "tcp", port ) ) );
        refused.SubscribeTo( connector.GetPointerAlways() );
        ASSERT_TRUE( PumpUntil( refused.failed, 1, 5000 ) );
        ASSERT_TRUE( host.GetRefusedAcceptCount() >= 1 && 0 == host.GetLinkCount() );
        connector.Unlink();
        host.SetBanList( GUCEF_NULL );
        host.Stop();
        client.Stop();

        // the user
        DRN::CDRNBanList userBans;
        userBans.Ban( DRN::DRN_BAN_USER_ID, "mallory", 0, "cheating" );
        DRN::CDRNNodeSettings hostSettings = SecurityNodeSettings();
        hostSettings.auth.authenticatorType = "token";
        hostSettings.auth.SetSetting( "key", "lobby key" );
        DRN::CDRNNode tokenHost;
        REQUIRE_TRUE( tokenHost.SetSettings( hostSettings ) );
        tokenHost.SetBanList( &userBans );
        UInt16 tokenPort = NextTestPort();
        REQUIRE_TRUE( tokenHost.Listen( LoopbackEndpoint( "udp", tokenPort ) ) );

        const char* users[] = { "mallory", "alice" };
        for ( UInt32 u=0; u<2; ++u )
        {
            DRN::CDRNNodeSettings userSettings = SecurityNodeSettings();
            userSettings.auth.authenticatorType = "token";
            userSettings.auth.SetSetting( "token", DRN::CDRNTokenAuthenticator::IssueToken( "lobby key", "arena", users[ u ], "player", DRN::CDRNTokenAuthenticator::GetUnixTimeNow() + 600 ) );
            DRN::CDRNNode userNode;
            REQUIRE_TRUE( userNode.SetSettings( userSettings ) );
            CSecurityConnectorRecorder recorder;
            DRN::CDRNConnectorPtr userConnector = userNode.Connect( DRN::CDRNEndpointList( LoopbackEndpoint( "udp", tokenPort ) ) );
            recorder.SubscribeTo( userConnector.GetPointerAlways() );
            UInt32 done = 0;
            UInt64 start = DRN::CDRNClock::NowInMilliseconds();
            while ( 0 == recorder.connected + recorder.failed && DRN::CDRNClock::NowInMilliseconds() - start < 5000 )
                PumpFor( 5 );
            done = recorder.connected + recorder.failed;
            ASSERT_TRUE( 1 == done );
            if ( 0 == u )
            {
                ASSERT_TRUE( 1 == recorder.failed );
                ASSERT_TRUE( DRN::DRN_LINKREASON_BANNED == recorder.failReason );
                ASSERT_TRUE( recorder.failText == "banned: cheating" );
            }
            else
            {
                ASSERT_TRUE( 1 == recorder.connected );
            }
            userConnector.Unlink();
            userNode.Stop();
        }
        tokenHost.SetBanList( GUCEF_NULL );
        tokenHost.Stop();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Security: the access policy enforces the roles the authentication established" )
    try
    {
        DRN::CDRNNodeSettings hostSettings = SecurityNodeSettings();
        hostSettings.auth.authenticatorType = "token";
        hostSettings.auth.SetSetting( "key", "lobby key" );
        DRN::CDRNNode hostNode;
        REQUIRE_TRUE( hostNode.SetSettings( hostSettings ) );
        DRN::CDRNDefaultAccessPolicy policy;
        CORE::CString::StringSet joinRoles;
        joinRoles.insert( "player" );
        policy.SetJoinRoles( joinRoles );
        hostNode.SetAccessPolicy( &policy );
        DRN::CDRNEndpoint endpoint = LoopbackEndpoint( "tcp", NextTestPort() );
        REQUIRE_TRUE( hostNode.Listen( endpoint ) );
        DRN::CDRNSession* host = hostNode.CreateSession();
        DRN::CDRNSessionSettings sessionSettings;
        sessionSettings.sessionName = "arena";
        REQUIRE_TRUE( host->Host( sessionSettings ) );

        const char* roles[] = { "spectator", "player" };
        for ( UInt32 r=0; r<2; ++r )
        {
            DRN::CDRNNode memberNode;
            REQUIRE_TRUE( memberNode.SetSettings( SecurityNodeSettings() ) );
            DRN::CDRNSession* member = memberNode.CreateSession();
            CSecuritySessionRecorder recorder;
            recorder.SubscribeTo( member );
            DRN::CDRNAuthSettings credentials;
            credentials.authenticatorType = "token";
            credentials.SetSetting( "token", DRN::CDRNTokenAuthenticator::IssueToken( "lobby key", "arena", "user", roles[ r ], DRN::CDRNTokenAuthenticator::GetUnixTimeNow() + 600 ) );
            REQUIRE_TRUE( member->Join( DRN::CDRNEndpointList( endpoint ), sessionSettings, &credentials ) );
            UInt64 start = DRN::CDRNClock::NowInMilliseconds();
            while ( 0 == recorder.joined + recorder.joinFailed && DRN::CDRNClock::NowInMilliseconds() - start < 5000 )
                PumpFor( 5 );
            if ( 0 == r )
                ASSERT_TRUE( 1 == recorder.joinFailed && 0 == recorder.joined );
            else
                ASSERT_TRUE( 1 == recorder.joined && 0 == recorder.joinFailed );
            memberNode.Stop();
        }
        hostNode.SetAccessPolicy( GUCEF_NULL );
        hostNode.Stop();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Security: a peer whose clock runs fast is reported, an honest peer is not" )
    try
    {
        DRN::CDRNNodeSettings settingsB = SecurityNodeSettings();
        settingsB.securityPolicy.anomalies.clockDriftPercent = 10;
        settingsB.securityPolicy.anomalies.clockWindowMs = 200;
        settingsB.securityPolicy.anomalies.clockSustainMs = 600;
        // the warp rewrites the sendTime of the header, which packet protection would refuse: plain links
        settingsB.securityPolicy.requireEncryption = false;
        settingsB.securityPolicy.ciphers.clear();
        DRN::CDRNNodeSettings settingsA = settingsB;
        settingsA.nodeId = DRN::CDRNNodeSettings::GenerateNodeId();

        CLinkedPair honest;
        CAnomalyRecorder honestAnomalies;
        honestAnomalies.SubscribeTo( &honest.nodeB );
        UInt16 port = NextTestPort();
        REQUIRE_TRUE( honest.Setup( settingsA, settingsB, LoopbackEndpoint( "lossy-tcp", port ), LoopbackEndpoint( "lossy-tcp", port ) ) );
        PumpFor( 1500 );
        ASSERT_TRUE( 0 == honestAnomalies.clockEvents );

        CLinkedPair cheater;
        CAnomalyRecorder anomalies;
        anomalies.SubscribeTo( &cheater.nodeB );
        UInt16 cheatPort = NextTestPort();
        REQUIRE_TRUE( cheater.Setup( settingsA, settingsB, LoopbackEndpoint( "lossy-tcp", cheatPort ), LoopbackEndpoint( "lossy-tcp", cheatPort, "?warp=1.5" ) ) );
        ASSERT_TRUE( PumpUntil( anomalies.clockEvents, 1, 5000 ) );
        ASSERT_TRUE( DRN::DRN_ANOMALY_CLOCK_DRIFT == anomalies.lastClock.kind );
        ASSERT_TRUE( anomalies.lastClock.measured > 30.0 && anomalies.lastClock.measured < 70.0 );
        ASSERT_TRUE( 10.0 == anomalies.lastClock.threshold );
        ASSERT_TRUE( 1 <= cheater.linkB->GetMonitor().GetClockAnomalyCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Security: silence followed by a burst, a lag switch, is reported" )
    try
    {
        DRN::CDRNNodeSettings settingsB = SecurityNodeSettings();
        settingsB.securityPolicy.anomalies.silenceMs = 1000;
        settingsB.securityPolicy.anomalies.burstPackets = 8;
        settingsB.securityPolicy.anomalies.burstWindowMs = 100;
        DRN::CDRNNodeSettings settingsA = settingsB;
        settingsA.nodeId = DRN::CDRNNodeSettings::GenerateNodeId();
        CLinkedPair pair;
        CAnomalyRecorder anomalies;
        anomalies.SubscribeTo( &pair.nodeB );
        UInt16 port = NextTestPort();
        REQUIRE_TRUE( pair.Setup( settingsA, settingsB, LoopbackEndpoint( "lossy-tcp", port ), LoopbackEndpoint( "lossy-tcp", port, "?holdAfter=300&holdFor=1500" ) ) );
        ASSERT_TRUE( PumpUntil( anomalies.trafficEvents, 1, 5000 ) );
        ASSERT_TRUE( DRN::DRN_ANOMALY_BURST_AFTER_SILENCE == anomalies.lastTraffic.kind );
        ASSERT_TRUE( anomalies.lastTraffic.measured >= 1000.0 );
        ASSERT_TRUE( pair.linkB->IsOperational() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Security: the telemetry of a session holds the counters of every member" )
    try
    {
        CSessionPair pair;
        REQUIRE_TRUE( pair.Setup( SecurityNodeSettings(), SecurityNodeSettings(), "telemetry" ) );
        for ( UInt32 i=0; i<20; ++i )
            ASSERT_TRUE( pair.member->SendAppMessage( pair.member->GetSequencerPeerId(), DRN::DRN_LANE_RELIABLE_ORDERED, "telemetry", 9 ) );
        ASSERT_TRUE( PumpUntil( pair.hostRecorder.appMessages, 20, 5000 ) );
        PumpFor( 300 );

        CORE::CDataNode telemetry;
        pair.host->GetTelemetry( telemetry );
        ASSERT_TRUE( "telemetry" == telemetry.GetAttributeValueOrChildValueByName( "sessionName" ).AsString( CORE::CString::Empty, true ) );
        const CORE::CDataNode* memberNode = telemetry.FindChild( "Member" );
        REQUIRE_TRUE( GUCEF_NULL != memberNode );
        ASSERT_TRUE( pair.memberRecorder.localPeerId == memberNode->GetAttributeValueOrChildValueByName( "peerId" ).AsUInt32( 0, true ) );
        ASSERT_TRUE( memberNode->GetAttributeValueOrChildValueByName( "messagesReceived" ).AsUInt64( 0, true ) >= 20 );
        ASSERT_TRUE( memberNode->GetAttributeValueOrChildValueByName( "packetsArrived" ).AsUInt64( 0, true ) > 0 );
        ASSERT_TRUE( memberNode->GetAttributeValueOrChildValueByName( "meanIntervalUs" ).AsUInt64( 0, true ) > 0 );
        ASSERT_TRUE( memberNode->GetAttributeValueOrChildValueByName( "intervalJitterUs" ).IsInitialized() );
        ASSERT_TRUE( memberNode->GetAttributeValueOrChildValueByName( "rttUs" ).IsInitialized() );
        ASSERT_TRUE( memberNode->GetAttributeValueOrChildValueByName( "droppedForLimits" ).IsInitialized() );
        ASSERT_TRUE( 0 == memberNode->GetAttributeValueOrChildValueByName( "limitHits" ).AsUInt64( 1, true ) );

        CORE::CDataNode nodeTelemetry;
        pair.hostNode.GetTelemetry( nodeTelemetry );
        ASSERT_TRUE( pair.hostNode.GetSettings().securityPolicy.GetPolicyHashString() == nodeTelemetry.GetAttributeValueOrChildValueByName( "policyHash" ).AsString( CORE::CString::Empty, true ) );
        ASSERT_TRUE( GUCEF_NULL != nodeTelemetry.FindChild( "Link" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Security: a session recording round trips: what was sent and received, in order" )
    try
    {
        CSessionPair pair;
        REQUIRE_TRUE( pair.Setup( SecurityNodeSettings(), SecurityNodeSettings(), "recorded" ) );
        CORE::CString path = TestFilePath( "gucefDRN_TestApp_recording.drnrec" );
        DRN::CDRNSessionRecorder recorder;
        REQUIRE_TRUE( recorder.Start( *pair.host, path ) );
        ASSERT_TRUE( recorder.IsRecording() && pair.host->GetRecorder() == &recorder );

        for ( UInt32 i=0; i<5; ++i )
        {
            CORE::CString text = "to host " + CORE::UInt32ToString( i );
            ASSERT_TRUE( pair.member->SendAppMessage( pair.member->GetSequencerPeerId(), DRN::DRN_LANE_RELIABLE_ORDERED, text.C_String(), text.ByteSize() - 1 ) );
        }
        for ( UInt32 i=0; i<3; ++i )
        {
            CORE::CString text = "to member " + CORE::UInt32ToString( i );
            ASSERT_TRUE( pair.host->SendAppMessage( pair.memberRecorder.localPeerId, DRN::DRN_LANE_RELIABLE_ORDERED, text.C_String(), text.ByteSize() - 1 ) );
        }
        ASSERT_TRUE( PumpUntil( pair.hostRecorder.appMessages, 5, 5000 ) );
        ASSERT_TRUE( PumpUntil( pair.memberRecorder.appMessages, 3, 5000 ) );
        recorder.Stop();
        ASSERT_FALSE( recorder.IsRecording() );
        ASSERT_TRUE( GUCEF_NULL == pair.host->GetRecorder() );

        DRN::CDRNSessionRecordingReader reader;
        REQUIRE_TRUE( reader.Open( path ) );
        ASSERT_TRUE( "recorded" == reader.GetSessionName() );
        ASSERT_TRUE( 1 == reader.GetLocalPeerId() );
        UInt32 incoming = 0;
        UInt32 outgoing = 0;
        UInt64 lastTime = 0;
        bool ordered = true;
        bool payloadsOk = true;
        DRN::CDRNRecordedMessage message;
        while ( reader.ReadNext( message ) )
        {
            ordered = ordered && message.timeUs >= lastTime;
            lastTime = message.timeUs;
            if ( DRN::DRN_MSGCLASS_APP != message.msgClass )
                continue;
            CORE::CString expected = CORE::CString( message.outgoing ? "to member " : "to host " ) + CORE::UInt32ToString( message.outgoing ? outgoing : incoming );
            payloadsOk = payloadsOk && BufferContains( message.payload, expected );
            ASSERT_TRUE( message.peerId == pair.memberRecorder.localPeerId );
            if ( message.outgoing )
                ++outgoing;
            else
                ++incoming;
        }
        ASSERT_TRUE( reader.IsAtEnd() );
        ASSERT_TRUE( 5 == incoming && 3 == outgoing );
        ASSERT_TRUE( ordered && payloadsOk );
        ASSERT_TRUE( recorder.GetRecordCount() >= 8 );
        CORE::DeleteFile( path );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
