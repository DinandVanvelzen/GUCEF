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

#include <exception>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_DRN_H
#include "gucefDRN.h"
#define GUCEF_DRN_H
#endif /* GUCEF_DRN_H ? */

#ifndef GUCEF_COMCORE_CUDPSOCKET_H
#include "CUDPSocket.h"
#define GUCEF_COMCORE_CUDPSOCKET_H
#endif /* GUCEF_COMCORE_CUDPSOCKET_H ? */

#ifndef GUCEF_COM_CSTUNCLIENT_H
#include "gucefCOM_CStunClient.h"
#define GUCEF_COM_CSTUNCLIENT_H
#endif /* GUCEF_COM_CSTUNCLIENT_H ? */

#ifndef GUCEF_COM_NETWORKBYTEORDER_H
#include "gucefCOM_NetworkByteOrder.h"
#define GUCEF_COM_NETWORKBYTEORDER_H
#endif /* GUCEF_COM_NETWORKBYTEORDER_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestDRNTestUtils.h"
#include "TestDRNRelay.h"

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

static const UInt32 TestOpcode = 77;

/*-------------------------------------------------------------------------*/

static DRN::CDRNNodeSettings
RelayTestNodeSettings( void )
{
    DRN::CDRNNodeSettings settings;
    settings.echoIntervalMs = 20;
    settings.pulseIntervalMs = 1;
    return settings;
}

/*-------------------------------------------------------------------------*/

class CBoundCondition : public CTestCondition
{
    public:
    DRN::CDRNNode& m_node;
    DRN::CDRNEndpoint m_relay;
    CORE::CString m_realm;
    CBoundCondition( DRN::CDRNNode& node, const DRN::CDRNEndpoint& relay, const CORE::CString& realm ) : m_node( node ), m_relay( relay ), m_realm( realm ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return m_node.IsBoundAtRelay( m_relay, m_realm ); }
};

/*-------------------------------------------------------------------------*/

/** The link a recorder saw come up over a relay channel, NULL while there is none */
static DRN::CDRNPeerLinkPtr
FindRelayedLink( const CLinkRecorder& recorder )
{
    for ( UInt32 i=0; i<recorder.links.size(); ++i )
    {
        const DRN::CDRNPeerLinkPtr& link = recorder.links[ i ];
        if ( !link.IsNULL() && DRN::CDRNEndpoint::ChannelTypeRelay == link->GetRemoteEndpoint().GetChannelType() )
            return link;
    }
    return DRN::CDRNPeerLinkPtr();
}

/*-------------------------------------------------------------------------*/

class CRelayedLinkCondition : public CTestCondition
{
    public:
    const CLinkRecorder& m_recorder;
    explicit CRelayedLinkCondition( const CLinkRecorder& recorder ) : m_recorder( recorder ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return !FindRelayedLink( m_recorder ).IsNULL(); }
};

/*-------------------------------------------------------------------------*/

/** Application messages a recorder got, the relay's own messages on the link to the relay left out */
class CAppMessagesCondition : public CTestCondition
{
    public:
    const CLinkRecorder& m_recorder;
    UInt32 m_count;
    CAppMessagesCondition( const CLinkRecorder& recorder, UInt32 count ) : m_recorder( recorder ), m_count( count ) {}
    UInt32 Count( void ) const
    {
        UInt32 count = 0;
        for ( UInt32 i=0; i<m_recorder.messages.size(); ++i )
            if ( DRN::DRN_MSGCLASS_APP == m_recorder.messages[ i ].msgClass )
                ++count;
        return count;
    }
    const CRecordedMessage* Last( void ) const
    {
        for ( UInt32 i=(UInt32)m_recorder.messages.size(); i>0; --i )
            if ( DRN::DRN_MSGCLASS_APP == m_recorder.messages[ i-1 ].msgClass )
                return &m_recorder.messages[ i-1 ];
        return GUCEF_NULL;
    }
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return Count() >= m_count; }
};

class CSessionStateCondition : public CTestCondition
{
    public:
    CSessionStateCondition( DRN::CDRNSession& session ) : m_session( session ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return DRN::CDRNSession::SESSIONSTATE_JOINING != m_session.GetState(); }
    private:
    DRN::CDRNSession& m_session;
};

/*-------------------------------------------------------------------------*/

class CConnectorDoneCondition : public CTestCondition
{
    public:
    CConnectorDoneCondition( DRN::CDRNConnectorPtr& connector ) : m_connector( connector ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return m_connector->IsDone(); }
    private:
    DRN::CDRNConnectorPtr& m_connector;
};

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/

static const char* FakeExternalAddress = "203.0.113.7";

/**
 *  A NAT-PMP gateway on loopback: answers the external address and maps every port to the port + 10000
 *  (as the gateway of the NAT traversal tests of gucefCOM_TestApp, NAT-PMP only)
 */
class CFakeGateway : public CORE::CObserver
{
    public:

    UInt32 mapRequests;
    UInt32 addressRequests;

    explicit CFakeGateway( UInt16 port )
        : CORE::CObserver(), mapRequests( 0 ), addressRequests( 0 ), m_port( port ), m_socket( false )
    {
        SubscribeTo( &m_socket );
    }

    virtual ~CFakeGateway()
    {
        UnsubscribeAllFromObserver();
        m_socket.Close();
    }

    bool Start( void ) { return m_socket.Open( COMCORE::CIPv4Address( "127.0.0.1", m_port ) ); }

    protected:

    virtual void OnNotify( CORE::CNotifier* /* notifier */, const CORE::CEvent& eventId, CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( COMCORE::CUDPSocket::UDPPacketsRecievedEvent != eventId || GUCEF_NULL == eventData )
            return;
        const COMCORE::CUDPSocket::TUdpPacketsRecievedEventData& data = static_cast< COMCORE::CUDPSocket::UDPPacketsRecievedEventData* >( eventData )->GetData();
        for ( UInt32 i=0; i<data.packetsReceived; ++i )
            OnRequest( data.packets[ i ].sourceAddress, data.packets[ i ].dataBuffer.GetData() );
    }

    private:

    void OnRequest( const COMCORE::CIPv4Address& client, const CORE::CDynamicBuffer& request )
    {
        const UInt8* data = static_cast< const UInt8* >( request.GetConstBufferPtr() );
        UInt32 size = request.GetDataSize();
        if ( size < 2 )
            return;

        COMCORE::CIPv4Address external( FakeExternalAddress, 0 );
        UInt32 externalAddress = external.GetAddress();
        CORE::CDynamicBuffer response;
        COM::CNetworkByteWriter writer( response );
        if ( 2 == data[ 0 ] )
        {
            // a PCP request: this gateway only speaks NAT-PMP (RFC 6887 section 9)
            writer.WriteUInt8( 0 );
            writer.WriteUInt8( 0x80 | data[ 1 ] );
            writer.WriteUInt16( 1 );
            writer.WriteUInt32( 1234 );
        }
        else
        if ( 0 == data[ 0 ] && 0 == data[ 1 ] )
        {
            ++addressRequests;
            writer.WriteUInt8( 0 );
            writer.WriteUInt8( 128 );
            writer.WriteUInt16( 0 );
            writer.WriteUInt32( 1234 );
            writer.WriteBytes( &externalAddress, 4 );
        }
        else
        if ( 0 == data[ 0 ] && ( 1 == data[ 1 ] || 2 == data[ 1 ] ) && size >= 12 )
        {
            ++mapRequests;
            UInt16 internalPort = 0;
            UInt32 lifetime = 0;
            COM::CNetworkByteReader reader( data, size );
            reader.Skip( 4 );
            reader.ReadUInt16( internalPort );
            reader.Skip( 2 );
            reader.ReadUInt32( lifetime );
            writer.WriteUInt8( 0 );
            writer.WriteUInt8( (UInt8)( 128 + data[ 1 ] ) );
            writer.WriteUInt16( 0 );
            writer.WriteUInt32( 1234 );
            writer.WriteUInt16( internalPort );
            writer.WriteUInt16( (UInt16)( 0 == lifetime ? 0 : internalPort + 10000 ) );
            writer.WriteUInt32( lifetime );
        }
        if ( response.GetDataSize() > 0 )
            m_socket.SendPacketTo( client, response.GetConstBufferPtr(), (UInt16) response.GetDataSize() );
    }

    UInt16 m_port;
    COMCORE::CUDPSocket m_socket;
};

/*-------------------------------------------------------------------------*/

/** A STUN server on loopback: answers Binding requests with the address they came from (XOR-MAPPED-ADDRESS) */
class CFakeStunServer : public CORE::CObserver
{
    public:

    UInt32 requests;

    explicit CFakeStunServer( UInt16 port )
        : CORE::CObserver(), requests( 0 ), m_port( port ), m_socket( false )
    {
        SubscribeTo( &m_socket );
    }

    virtual ~CFakeStunServer()
    {
        UnsubscribeAllFromObserver();
        m_socket.Close();
    }

    bool Start( void ) { return m_socket.Open( COMCORE::CIPv4Address( "127.0.0.1", m_port ) ); }

    protected:

    virtual void OnNotify( CORE::CNotifier* /* notifier */, const CORE::CEvent& eventId, CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( COMCORE::CUDPSocket::UDPPacketsRecievedEvent != eventId || GUCEF_NULL == eventData )
            return;
        const COMCORE::CUDPSocket::TUdpPacketsRecievedEventData& data = static_cast< COMCORE::CUDPSocket::UDPPacketsRecievedEventData* >( eventData )->GetData();
        for ( UInt32 i=0; i<data.packetsReceived; ++i )
        {
            const CORE::CDynamicBuffer& request = data.packets[ i ].dataBuffer.GetData();
            if ( !COM::CStunClient::IsStunMessage( request.GetConstBufferPtr(), request.GetDataSize() ) )
                continue;
            ++requests;
            const UInt8* bytes = static_cast< const UInt8* >( request.GetConstBufferPtr() );
            const COMCORE::CIPv4Address& client = data.packets[ i ].sourceAddress;
            CORE::CDynamicBuffer response;
            COM::CNetworkByteWriter writer( response );
            writer.WriteUInt16( 0x0101 );
            writer.WriteUInt16( 12 );
            writer.WriteUInt32( COM::CStunClient::MagicCookie );
            writer.WriteBytes( bytes + 8, 12 );
            writer.WriteUInt16( 0x0020 );
            writer.WriteUInt16( 8 );
            writer.WriteUInt8( 0 );
            writer.WriteUInt8( 1 );
            writer.WriteUInt16( (UInt16)( client.GetPortInHostByteOrder() ^ ( COM::CStunClient::MagicCookie >> 16 ) ) );
            writer.WriteUInt32( client.GetAddressInHostByteOrder() ^ COM::CStunClient::MagicCookie );
            m_socket.SendPacketTo( client, response.GetConstBufferPtr(), (UInt16) response.GetDataSize() );
        }
    }

    private:

    UInt16 m_port;
    COMCORE::CUDPSocket m_socket;
};

/*-------------------------------------------------------------------------*/

class CReachabilityCondition : public CTestCondition
{
    public:
    DRN::CDRNReachabilityManager& m_manager;
    explicit CReachabilityCondition( DRN::CDRNReachabilityManager& manager ) : m_manager( manager ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return m_manager.IsComplete(); }
};

/*-------------------------------------------------------------------------*/

/** Counts the application messages a session got */
class CAppMessageCounter : public CORE::CObserver
{
    public:
    UInt32 joined;
    UInt32 messages;
    CAppMessageCounter( void ) : joined( 0 ), messages( 0 ) {}
    virtual ~CAppMessageCounter() { SignalUpcomingObserverDestruction(); }
    protected:
    virtual void OnNotify( CORE::CNotifier* /* notifier */, const CORE::CEvent& eventid, CORE::CICloneable* /* eventdata */ = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( DRN::CDRNSession::JoinedEvent == eventid )
            ++joined;
        else
        if ( DRN::CDRNSession::AppMessageEvent == eventid )
            ++messages;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDRNRelayTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefDRN.Relay" );

    GUCEF_TESTFW_TESTCASE( "Relay: two nodes that only know the relay link through it, every lane, the round trip and clock offset hold" )
    try
    {
        DRN::CDRNNodeSettings relaySettings = RelayTestNodeSettings();
        relaySettings.relayEnabled = true;
        DRN::CDRNNode relay;
        DRN::CDRNNode nodeA;
        DRN::CDRNNode nodeB;
        REQUIRE_TRUE( relay.SetSettings( relaySettings ) && nodeA.SetSettings( RelayTestNodeSettings() ) && nodeB.SetSettings( RelayTestNodeSettings() ) );
        DRN::CDRNEndpoint relayEndpoint = LoopbackEndpoint( "udp", NextTestPort() );
        REQUIRE_TRUE( relay.Listen( relayEndpoint ) );
        REQUIRE_TRUE( GUCEF_NULL != relay.GetRelayService() );

        // B is only reachable through the relay
        ASSERT_TRUE( nodeB.BindAtRelay( relayEndpoint, "arena" ) );
        CBoundCondition bound( nodeB, relayEndpoint, "arena" );
        REQUIRE_TRUE( PumpUntil( bound, 10000 ) );
        DRN::CDRNEndpointList relayEndpoints = nodeB.GetRelayEndpoints();
        REQUIRE_TRUE( 1 == relayEndpoints.GetCount() );
        ASSERT_TRUE( DRN::CDRNEndpoint::ChannelTypeRelay == relayEndpoints.Get( 0 ).GetChannelType() );
        ASSERT_TRUE( 1 == nodeB.GetListenEndpoints().GetCount() );

        CLinkRecorder recorderA;
        CLinkRecorder recorderB;
        recorderA.WatchNode( nodeA );
        recorderB.WatchNode( nodeB );
        DRN::CDRNConnectorPtr connector = nodeA.Connect( relayEndpoints );
        CRelayedLinkCondition relayedA( recorderA );
        CRelayedLinkCondition relayedB( recorderB );
        REQUIRE_TRUE( PumpUntil( relayedA, 10000 ) && PumpUntil( relayedB, 10000 ) );
        DRN::CDRNPeerLinkPtr linkA = FindRelayedLink( recorderA );
        DRN::CDRNPeerLinkPtr linkB = FindRelayedLink( recorderB );
        ASSERT_TRUE( linkA->GetRemoteNodeId() == nodeB.GetSettings().nodeId && linkB->GetRemoteNodeId() == nodeA.GetSettings().nodeId );
        ASSERT_TRUE( DRN::CDRNEndpoint::ChannelTypeRelay == linkA->GetRemoteEndpoint().GetChannelType() );
        // end to end: the relay forwards what it cannot read
        ASSERT_TRUE( linkA->IsProtected() && linkB->IsProtected() );

        // every lane, and a reliable message that has to be fragmented
        const UInt32 count = 50;
        CORE::CDynamicBuffer payload;
        for ( UInt32 i=0; i<count; ++i )
        {
            payload.SetDataSize( 0 );
            payload.CopyFrom( sizeof( i ), &i );
            for ( UInt8 lane=0; lane<DRN::DRN_LANE_COUNT; ++lane )
                ASSERT_TRUE( linkA->Send( lane, DRN::DRN_MSGCLASS_APP, TestOpcode, payload ) );
            PumpFor( 1 );
        }
        CORE::CDynamicBuffer big;
        big.SetDataSize( 0 );
        for ( UInt32 i=0; i<5000; ++i )
        {
            UInt8 b = (UInt8) i;
            big.CopyFrom( i, 1, &b );
        }
        ASSERT_TRUE( linkB->Send( DRN::DRN_LANE_RELIABLE_ORDERED, DRN::DRN_MSGCLASS_APP, TestOpcode, big ) );
        // the reliable lanes deliver all, the others whatever loopback did not lose
        CAppMessagesCondition atB( recorderB, 2 * count );
        CAppMessagesCondition atA( recorderA, 1 );
        ASSERT_TRUE( PumpUntil( atB, 10000 ) );
        ASSERT_TRUE( PumpUntil( atA, 10000 ) );
        ASSERT_TRUE( GUCEF_NULL != atA.Last() && 5000 == atA.Last()->payload.GetDataSize() );

        // the round trip through the relay on loopback, the clock offset of two nodes on the same clock
        PumpFor( 1200 );
        DRN::CDRNLatencyInfo info;
        linkA->GetLatencyInfo( info );
        ASSERT_TRUE( info.sampleCount > 0 && info.smoothedRtt > 0 && info.smoothedRtt < 20000 );
        DRN::CDRNLatencyInfo infoB;
        linkB->GetLatencyInfo( infoB );
        ASSERT_TRUE( infoB.sampleCount > 0 );
        Int64 remoteNow = (Int64) linkA->GetEstimatedRemoteTime( DRN::CDRNClock::NowInMicroseconds() );
        Int64 localNow = (Int64) DRN::CDRNClock::NowInMicroseconds();
        ASSERT_TRUE( remoteNow - localNow > -5000 && remoteNow - localNow < 5000 );
        ASSERT_TRUE( relay.GetRelayService()->GetForwardedBytes() > 5000 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Relay link: rtt " << info.smoothedRtt << " us, forwarded " << relay.GetRelayService()->GetForwardedBytes() << " bytes" );

        // closing through the relay
        linkA->Close();
        ASSERT_TRUE( PumpUntil( recorderB.closedCount, 1, 12000 ) );

        connector.Unlink();
        nodeA.Stop();
        nodeB.Stop();
        relay.Stop();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Relay: through a relay that delays what it sends the round trip and the clock offset stay within tolerance" )
    try
    {
        DRN::CDRNNodeSettings relaySettings = RelayTestNodeSettings();
        relaySettings.relayEnabled = true;
        DRN::CDRNNode relay;
        DRN::CDRNNode nodeA;
        DRN::CDRNNode nodeB;
        REQUIRE_TRUE( relay.SetSettings( relaySettings ) && nodeA.SetSettings( RelayTestNodeSettings() ) && nodeB.SetSettings( RelayTestNodeSettings() ) );

        // the relay's channels hold back what it sends by 10 ms: one delayed hop each way
        UInt16 relayPort = NextTestPort();
        REQUIRE_TRUE( relay.Listen( LoopbackEndpoint( "lossy-udp", relayPort, "?delay=10" ) ) );
        DRN::CDRNEndpoint relayService( "lossy-udp", "127.0.0.1:" + CORE::UInt32ToString( relayPort ) );
        ASSERT_TRUE( nodeB.BindAtRelay( relayService, "delayed" ) );
        CBoundCondition bound( nodeB, relayService, "delayed" );
        REQUIRE_TRUE( PumpUntil( bound, 10000 ) );

        CLinkRecorder recorderA;
        recorderA.WatchNode( nodeA );
        DRN::CDRNConnectorPtr connector = nodeA.Connect( nodeB.GetRelayEndpoints() );
        CRelayedLinkCondition relayedA( recorderA );
        REQUIRE_TRUE( PumpUntil( relayedA, 10000 ) );
        DRN::CDRNPeerLinkPtr linkA = FindRelayedLink( recorderA );
        PumpFor( 1500 );

        DRN::CDRNLatencyInfo info;
        linkA->GetLatencyInfo( info );
        Int64 remoteNow = (Int64) linkA->GetEstimatedRemoteTime( DRN::CDRNClock::NowInMicroseconds() );
        Int64 localNow = (Int64) DRN::CDRNClock::NowInMicroseconds();
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Delaying relay: rtt " << info.smoothedRtt << " us, min " << info.minRtt << " us, clock offset estimate " << ( remoteNow - localNow ) << " us" );
        ASSERT_TRUE( info.sampleCount > 0 && info.minRtt >= 19000 && info.smoothedRtt < 40000 );
        ASSERT_TRUE( remoteNow - localNow > -5000 && remoteNow - localNow < 5000 );

        connector.Unlink();
        nodeA.Stop();
        nodeB.Stop();
        relay.Stop();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Relay: a session hosted by a node that is only reachable through the relay" )
    try
    {
        DRN::CDRNNodeSettings relaySettings = RelayTestNodeSettings();
        relaySettings.relayEnabled = true;
        relaySettings.relayToken = "let-me-relay";
        DRN::CDRNNode relay;
        DRN::CDRNNode host;
        DRN::CDRNNode guest;
        DRN::CDRNNodeSettings memberSettings = RelayTestNodeSettings();
        memberSettings.relayToken = "let-me-relay";
        REQUIRE_TRUE( relay.SetSettings( relaySettings ) && host.SetSettings( memberSettings ) && guest.SetSettings( memberSettings ) );
        DRN::CDRNEndpoint relayEndpoint = LoopbackEndpoint( "tcp", NextTestPort() );
        REQUIRE_TRUE( relay.Listen( relayEndpoint ) );

        // a binding with another token is refused
        DRN::CDRNNode stranger;
        REQUIRE_TRUE( stranger.SetSettings( RelayTestNodeSettings() ) );
        const CORE::CString wrongToken = "guess";
        ASSERT_TRUE( stranger.BindAtRelay( relayEndpoint, "lobby", 0, &wrongToken ) );
        PumpFor( 300 );
        ASSERT_FALSE( stranger.IsBoundAtRelay( relayEndpoint, "lobby" ) );

        ASSERT_TRUE( host.BindAtRelay( relayEndpoint, "lobby", 40 ) );
        CBoundCondition bound( host, relayEndpoint, "lobby" );
        REQUIRE_TRUE( PumpUntil( bound, 10000 ) );
        ASSERT_TRUE( host.GetRelayEndpoints().Get( 0 ).GetPath() == "/lobby/40" );

        DRN::CDRNSession* hosted = host.CreateSession();
        DRN::CDRNSessionSettings settings;
        settings.sessionName = "relayed";
        REQUIRE_TRUE( hosted->Host( settings ) );
        CAppMessageCounter hostCounter;
        hostCounter.SubscribeTo( hosted );

        DRN::CDRNSession* joined = guest.CreateSession();
        CAppMessageCounter guestCounter;
        guestCounter.SubscribeTo( joined );
        REQUIRE_TRUE( joined->Join( host.GetListenEndpoints(), settings ) );
        REQUIRE_TRUE( PumpUntil( guestCounter.joined, 1, 10000 ) );
        ASSERT_TRUE( 2 == joined->GetMemberCount() );

        CORE::CString text = "through the relay";
        ASSERT_TRUE( joined->SendAppMessage( 1, DRN::DRN_LANE_RELIABLE_ORDERED, text.C_String(), text.Length() ) );
        ASSERT_TRUE( hosted->SendAppMessage( 0, DRN::DRN_LANE_UNRELIABLE, text.C_String(), text.Length() ) );
        ASSERT_TRUE( PumpUntil( hostCounter.messages, 1, 10000 ) && PumpUntil( guestCounter.messages, 1, 10000 ) );

        stranger.Stop();
        guest.Stop();
        host.Stop();
        relay.Stop();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    // A game host that stops and a new one for the same session: the peer id is free again once the relay saw the
    // link of the old binding close
    const char* rebindChannels[] = { "tcp", "udp" };
    for ( UInt32 c=0; c<2; ++c )
    {
        const CORE::CString channelType = rebindChannels[ c ];
        const CORE::CString caseName = "Relay: once the link of a binding closed its peer id can be bound again at once, over " + channelType;
        GUCEF_TESTFW_TESTCASE( caseName.C_String() )
        try
        {
            DRN::CDRNNodeSettings relaySettings = RelayTestNodeSettings();
            relaySettings.relayEnabled = true;
            relaySettings.relayToken = "let-me-relay";
            DRN::CDRNNodeSettings memberSettings = RelayTestNodeSettings();
            memberSettings.relayToken = "let-me-relay";
            DRN::CDRNNode relay;
            REQUIRE_TRUE( relay.SetSettings( relaySettings ) );
            DRN::CDRNEndpoint relayEndpoint = LoopbackEndpoint( channelType, NextTestPort() );
            REQUIRE_TRUE( relay.Listen( relayEndpoint ) );

            DRN::CDRNNode* oldHost = GUCEF_NEW DRN::CDRNNode();
            REQUIRE_TRUE( oldHost->SetSettings( memberSettings ) );
            ASSERT_TRUE( oldHost->BindAtRelay( relayEndpoint, "rebind", 1 ) );
            CBoundCondition oldBound( *oldHost, relayEndpoint, "rebind" );
            REQUIRE_TRUE( PumpUntil( oldBound, 10000 ) );

            // the old host goes away
            oldHost->Stop();
            GUCEF_DELETE oldHost;
            oldHost = GUCEF_NULL;
            PumpFor( 100 );

            DRN::CDRNNode newHost;
            REQUIRE_TRUE( newHost.SetSettings( memberSettings ) );
            ASSERT_TRUE( newHost.BindAtRelay( relayEndpoint, "rebind", 1 ) );
            CBoundCondition newBound( newHost, relayEndpoint, "rebind" );
            ASSERT_TRUE( PumpUntil( newBound, 2000 ) );
            if ( newHost.IsBoundAtRelay( relayEndpoint, "rebind" ) )
                ASSERT_TRUE( newHost.GetRelayEndpoints().Get( 0 ).GetPath() == "/rebind/1" );

            newHost.Stop();
            relay.Stop();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        GUCEF_TESTFW_TESTCASE_END
    }

    GUCEF_TESTFW_TESTCASE( "Relay: a binding whose link went silent is replaced by a new bind for its peer id" )
    try
    {
        // A host that is gone without closing its link, a process that was killed for instance: its link times out
        // only later, a new host with valid credentials gets the peer id once the old link has been silent a while
        DRN::CDRNNodeSettings relaySettings = RelayTestNodeSettings();
        relaySettings.relayEnabled = true;
        relaySettings.relayToken = "let-me-relay";
        DRN::CDRNNodeSettings memberSettings = RelayTestNodeSettings();
        memberSettings.relayToken = "let-me-relay";
        DRN::CDRNNode relay;
        REQUIRE_TRUE( relay.SetSettings( relaySettings ) );
        DRN::CDRNEndpoint relayEndpoint = LoopbackEndpoint( "udp", NextTestPort() );
        REQUIRE_TRUE( relay.Listen( relayEndpoint ) );

        // the old host runs on a pulse generator of its own: when that is no longer pulsed the host is silent
        CORE::PulseGeneratorPtr oldPulse = ( GUCEF_NEW CORE::CPulseGenerator() )->CreateSharedPtr();
        DRN::CDRNNode* oldHost = GUCEF_NEW DRN::CDRNNode( oldPulse );
        REQUIRE_TRUE( oldHost->SetSettings( memberSettings ) );
        ASSERT_TRUE( oldHost->BindAtRelay( relayEndpoint, "silent", 1 ) );
        for ( UInt32 waited=0; waited<10000 && !oldHost->IsBoundAtRelay( relayEndpoint, "silent" ); waited+=2 )
        {
            oldPulse->ForceDirectPulse();
            PumpFor( 2 );
        }
        REQUIRE_TRUE( oldHost->IsBoundAtRelay( relayEndpoint, "silent" ) );
        PumpFor( 1500 );

        DRN::CDRNNode newHost;
        REQUIRE_TRUE( newHost.SetSettings( memberSettings ) );
        ASSERT_TRUE( newHost.BindAtRelay( relayEndpoint, "silent", 1 ) );
        CBoundCondition newBound( newHost, relayEndpoint, "silent" );
        ASSERT_TRUE( PumpUntil( newBound, 2000 ) );

        newHost.Stop();
        GUCEF_DELETE oldHost;
        oldHost = GUCEF_NULL;
        relay.Stop();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Relay: an automatic peer id never takes a fixed one, a host binds its session id after a member bound automatically" )
    try
    {
        // A member reaching the host through the relay binds first with an id from the relay, then the host binds
        // as its session peer id 1
        DRN::CDRNNodeSettings relaySettings = RelayTestNodeSettings();
        relaySettings.relayEnabled = true;
        DRN::CDRNNode relay;
        DRN::CDRNNode member;
        DRN::CDRNNode host;
        REQUIRE_TRUE( relay.SetSettings( relaySettings ) && member.SetSettings( RelayTestNodeSettings() ) && host.SetSettings( RelayTestNodeSettings() ) );
        DRN::CDRNEndpoint relayEndpoint = LoopbackEndpoint( "udp", NextTestPort() );
        REQUIRE_TRUE( relay.Listen( relayEndpoint ) );

        ASSERT_TRUE( member.BindAtRelay( relayEndpoint, "mixed" ) );
        CBoundCondition memberBound( member, relayEndpoint, "mixed" );
        REQUIRE_TRUE( PumpUntil( memberBound, 10000 ) );
        PumpFor( 300 );

        ASSERT_TRUE( host.BindAtRelay( relayEndpoint, "mixed", 1 ) );
        CBoundCondition hostBound( host, relayEndpoint, "mixed" );
        ASSERT_TRUE( PumpUntil( hostBound, 5000 ) );
        if ( host.IsBoundAtRelay( relayEndpoint, "mixed" ) )
            ASSERT_TRUE( host.GetRelayEndpoints().Get( 0 ).GetPath() == "/mixed/1" );
        ASSERT_TRUE( member.GetRelayEndpoints().Get( 0 ).GetPath() != "/mixed/1" );

        host.Stop();
        member.Stop();
        relay.Stop();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Relay: the bytes per second of a realm are capped" )
    try
    {
        DRN::CDRNNodeSettings relaySettings = RelayTestNodeSettings();
        relaySettings.relayEnabled = true;
        relaySettings.relayMaxBytesPerSecondPerRealm = 20000;
        DRN::CDRNNode relay;
        DRN::CDRNNode nodeA;
        DRN::CDRNNode nodeB;
        REQUIRE_TRUE( relay.SetSettings( relaySettings ) && nodeA.SetSettings( RelayTestNodeSettings() ) && nodeB.SetSettings( RelayTestNodeSettings() ) );
        DRN::CDRNEndpoint relayEndpoint = LoopbackEndpoint( "udp", NextTestPort() );
        REQUIRE_TRUE( relay.Listen( relayEndpoint ) );
        ASSERT_TRUE( nodeB.BindAtRelay( relayEndpoint, "capped" ) );
        CBoundCondition bound( nodeB, relayEndpoint, "capped" );
        REQUIRE_TRUE( PumpUntil( bound, 10000 ) );

        CLinkRecorder recorderA;
        recorderA.WatchNode( nodeA );
        DRN::CDRNConnectorPtr connector = nodeA.Connect( nodeB.GetRelayEndpoints() );
        CRelayedLinkCondition relayedA( recorderA );
        REQUIRE_TRUE( PumpUntil( relayedA, 10000 ) );
        DRN::CDRNPeerLinkPtr linkA = FindRelayedLink( recorderA );

        // 1000 bytes per millisecond for half a second: far over 20000 bytes per second
        CORE::CDynamicBuffer payload;
        payload.SetDataSize( 0 );
        for ( UInt32 i=0; i<1000; ++i )
        {
            UInt8 b = (UInt8) i;
            payload.CopyFrom( i, 1, &b );
        }
        UInt64 forwardedBefore = relay.GetRelayService()->GetForwardedBytes();
        UInt64 start = DRN::CDRNClock::NowInMicroseconds();
        while ( DRN::CDRNClock::NowInMicroseconds() - start < 500000 )
        {
            linkA->Send( DRN::DRN_LANE_UNRELIABLE, DRN::DRN_MSGCLASS_APP, TestOpcode, payload );
            PumpFor( 1 );
        }
        UInt64 forwarded = relay.GetRelayService()->GetForwardedBytes() - forwardedBefore;
        ASSERT_TRUE( relay.GetRelayService()->GetDroppedBytes() > 0 );
        ASSERT_TRUE( forwarded <= 20000 + 20000 / 2 + 2000 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Relay cap: forwarded " << forwarded << " bytes in 0.5 s, dropped " << relay.GetRelayService()->GetDroppedBytes() );

        connector.Unlink();
        nodeA.Stop();
        nodeB.Stop();
        relay.Stop();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Relay: games with policies of their own share a relay, the link to it runs under the relay's policy, the links through it under the game's" )
    try
    {
        DRN::CDRNNodeSettings relaySettings = RelayTestNodeSettings();
        relaySettings.relayEnabled = true;
        relaySettings.securityPolicy.limits.maxMessagesPerSecond = 30000;
        DRN::CDRNNodeSettings gameOne = RelayTestNodeSettings();
        gameOne.securityPolicy.limits.maxMessagesPerSecond = 21000;
        DRN::CDRNNodeSettings gameTwo = RelayTestNodeSettings();
        gameTwo.securityPolicy.limits.maxMessagesPerSecond = 22000;
        REQUIRE_TRUE( gameOne.securityPolicy.GetPolicyHashString() != gameTwo.securityPolicy.GetPolicyHashString() );

        DRN::CDRNNode relay;
        REQUIRE_TRUE( relay.SetSettings( relaySettings ) );
        DRN::CDRNEndpoint relayEndpoint = LoopbackEndpoint( "udp", NextTestPort() );
        REQUIRE_TRUE( relay.Listen( relayEndpoint ) );
        DRN::CDRNEndpointList published = relay.GetPublishedEndpoints();
        REQUIRE_TRUE( 1 == published.GetCount() );
        ASSERT_TRUE( published.Get( 0 ).GetParams().GetValueAlways( "policy" ).AsString( CORE::CString::Empty, true ) == relaySettings.securityPolicy.GetPolicyHashString() );

        // without the relay's policy the game's own goes to the relay, which refuses it
        DRN::CDRNNode plainHost;
        REQUIRE_TRUE( plainHost.SetSettings( gameOne ) );
        ASSERT_TRUE( plainHost.BindAtRelay( relayEndpoint, "plain" ) );
        PumpFor( 500 );
        ASSERT_FALSE( plainHost.IsBoundAtRelay( relayEndpoint, "plain" ) );
        plainHost.Stop();

        // with it every game binds, whatever its own policy
        DRN::CDRNNode hostOne;
        DRN::CDRNNode hostTwo;
        REQUIRE_TRUE( hostOne.SetSettings( gameOne ) && hostTwo.SetSettings( gameTwo ) );
        ASSERT_TRUE( hostOne.BindAtRelay( published.Get( 0 ), "one", 1 ) );
        ASSERT_TRUE( hostTwo.BindAtRelay( published.Get( 0 ), "two", 1 ) );
        CBoundCondition boundOne( hostOne, published.Get( 0 ), "one" );
        CBoundCondition boundTwo( hostTwo, published.Get( 0 ), "two" );
        REQUIRE_TRUE( PumpUntil( boundOne, 10000 ) && PumpUntil( boundTwo, 10000 ) );
        DRN::CDRNEndpointList relayedOne = hostOne.GetRelayEndpoints();
        DRN::CDRNEndpointList relayedTwo = hostTwo.GetRelayEndpoints();
        REQUIRE_TRUE( 1 == relayedOne.GetCount() && 1 == relayedTwo.GetCount() );
        ASSERT_TRUE( relayedOne.Get( 0 ).GetParams().GetValueAlways( "relayPolicy" ).AsString( CORE::CString::Empty, true ) == relaySettings.securityPolicy.GetPolicyHashString() );
        ASSERT_TRUE( relayedOne.Get( 0 ).GetParams().GetValueAlways( "relayKey" ).AsString( CORE::CString::Empty, true ) == relay.GetPublicKeyString() );

        // a session of each game, its member joining through the relay
        DRN::CDRNSession* sessionOne = hostOne.CreateSession();
        DRN::CDRNSession* sessionTwo = hostTwo.CreateSession();
        DRN::CDRNSessionSettings settingsOne;
        settingsOne.sessionName = "one";
        settingsOne.securityPolicy = gameOne.securityPolicy;
        DRN::CDRNSessionSettings settingsTwo;
        settingsTwo.sessionName = "two";
        settingsTwo.securityPolicy = gameTwo.securityPolicy;
        REQUIRE_TRUE( sessionOne->Host( settingsOne ) && sessionTwo->Host( settingsTwo ) );

        DRN::CDRNNode memberOne;
        DRN::CDRNNode memberTwo;
        REQUIRE_TRUE( memberOne.SetSettings( gameOne ) && memberTwo.SetSettings( gameTwo ) );
        DRN::CDRNSession* joinOne = memberOne.CreateSession();
        DRN::CDRNSession* joinTwo = memberTwo.CreateSession();
        REQUIRE_TRUE( joinOne->Join( relayedOne, settingsOne ) && joinTwo->Join( relayedTwo, settingsTwo ) );
        CSessionStateCondition doneOne( *joinOne );
        CSessionStateCondition doneTwo( *joinTwo );
        REQUIRE_TRUE( PumpUntil( doneOne, 10000 ) && PumpUntil( doneTwo, 10000 ) );
        ASSERT_TRUE( DRN::CDRNSession::SESSIONSTATE_MEMBER == joinOne->GetState() && DRN::CDRNSession::SESSIONSTATE_MEMBER == joinTwo->GetState() );
        DRN::CDRNPeerLinkPtr hostLinkOne = joinOne->GetLinkToMember( joinOne->GetSequencerPeerId() );
        REQUIRE_TRUE( !hostLinkOne.IsNULL() );
        ASSERT_TRUE( hostLinkOne->IsProtected() && DRN::CDRNEndpoint::ChannelTypeRelay == hostLinkOne->GetRemoteEndpoint().GetChannelType() );
        hostLinkOne.Unlink();

        // a node of game two cannot reach the host of game one through the same relay: refused end to end
        DRN::CDRNNode intruder;
        REQUIRE_TRUE( intruder.SetSettings( gameTwo ) );
        DRN::CDRNConnectorPtr connector = intruder.Connect( relayedOne );
        CConnectorDoneCondition done( connector );
        REQUIRE_TRUE( PumpUntil( done, 10000 ) );
        ASSERT_FALSE( connector->HasSucceeded() );
        connector.Unlink();

        intruder.Stop();
        memberOne.Stop();
        memberTwo.Stop();
        hostOne.Stop();
        hostTwo.Stop();
        relay.Stop();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Reachability: the endpoint list has the LAN endpoints, the NAT-PMP mappings, the STUN reflexive address and the relay, in that order" )
    try
    {
        UInt16 gatewayPort = NextTestPort();
        UInt16 stunPort = NextTestPort();
        CFakeGateway gateway( gatewayPort );
        CFakeStunServer stunServer( stunPort );
        REQUIRE_TRUE( gateway.Start() && stunServer.Start() );

        DRN::CDRNNodeSettings relaySettings = RelayTestNodeSettings();
        relaySettings.relayEnabled = true;
        DRN::CDRNNode relay;
        DRN::CDRNNode node;
        DRN::CDRNNode peer;
        REQUIRE_TRUE( relay.SetSettings( relaySettings ) && node.SetSettings( RelayTestNodeSettings() ) && peer.SetSettings( RelayTestNodeSettings() ) );
        DRN::CDRNEndpoint relayEndpoint = LoopbackEndpoint( "udp", NextTestPort() );
        REQUIRE_TRUE( relay.Listen( relayEndpoint ) );
        UInt16 udpPort = NextTestPort();
        UInt16 tcpPort = NextTestPort();
        REQUIRE_TRUE( node.Listen( LoopbackEndpoint( "udp", udpPort ) ) && node.Listen( LoopbackEndpoint( "tcp", tcpPort ) ) );

        DRN::CDRNReachabilitySettings settings;
        CORE::CDataNode mapperConfig;
        mapperConfig.SetAttribute( "gateway", "127.0.0.1:" + CORE::UInt32ToString( gatewayPort ) );
        mapperConfig.SetAttribute( "usePcp", false );
        settings.AddPortMapper( "natpmp", mapperConfig );
        settings.stunServers.push_back( "127.0.0.1:" + CORE::UInt32ToString( stunPort ) );
        settings.relays.Add( relayEndpoint );
        settings.relayRealm = "reach";

        // the settings round trip through a data node
        CORE::CDataNode settingsNode;
        ASSERT_TRUE( settings.SaveConfig( settingsNode ) );
        DRN::CDRNReachabilitySettings loaded;
        ASSERT_TRUE( loaded.LoadConfig( settingsNode ) );
        ASSERT_TRUE( 1 == loaded.portMappers.size() && "natpmp" == loaded.portMappers[ 0 ].name && 1 == loaded.stunServers.size() );
        ASSERT_TRUE( 1 == loaded.relays.GetCount() && "reach" == loaded.relayRealm );

        DRN::CDRNReachabilityManager manager( node );
        ASSERT_TRUE( manager.Start( loaded ) );
        CReachabilityCondition complete( manager );
        REQUIRE_TRUE( PumpUntil( complete, 10000 ) );

        DRN::CDRNEndpointList endpoints = manager.GetEndpoints();
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Reachability: " << endpoints.ToString() );
        REQUIRE_TRUE( 5 == endpoints.GetCount() );
        ASSERT_TRUE( endpoints.Get( 0 ).ToString() == "udp://127.0.0.1:" + CORE::UInt32ToString( udpPort ) );
        ASSERT_TRUE( endpoints.Get( 1 ).ToString() == "tcp://127.0.0.1:" + CORE::UInt32ToString( tcpPort ) );
        ASSERT_TRUE( endpoints.Get( 2 ).ToString() == "udp://" + CORE::CString( FakeExternalAddress ) + ":" + CORE::UInt32ToString( udpPort + 10000 ) );
        ASSERT_TRUE( endpoints.Get( 3 ).ToString() == "tcp://" + CORE::CString( FakeExternalAddress ) + ":" + CORE::UInt32ToString( tcpPort + 10000 ) );
        ASSERT_TRUE( DRN::CDRNEndpoint::ChannelTypeRelay == endpoints.Get( 4 ).GetChannelType() );
        ASSERT_TRUE( 2 == gateway.mapRequests && stunServer.requests >= 1 );
        // on loopback the reflexive address is the LAN one: in the list once
        REQUIRE_TRUE( 1 == manager.GetReflexiveEndpoints().GetCount() );
        ASSERT_TRUE( manager.GetReflexiveEndpoints().Get( 0 ).ToString() == "udp://127.0.0.1:" + CORE::UInt32ToString( udpPort ) );

        // the list is what the node tells others, the sessions included
        ASSERT_TRUE( node.GetAdvertisedEndpoints().ToString() == endpoints.ToString() );
        DRN::CDRNSession* hosted = node.CreateSession();
        DRN::CDRNSessionSettings sessionSettings;
        sessionSettings.sessionName = "reachable";
        REQUIRE_TRUE( hosted->Host( sessionSettings ) );
        DRN::TDRNSessionMemberInfoVector members;
        hosted->GetMembers( members );
        REQUIRE_TRUE( 1 == members.size() );
        // with the node's public key on every endpoint, for pinning
        ASSERT_TRUE( members[ 0 ].endpoints.ToString() == node.GetPublishedEndpoints().ToString() );
        REQUIRE_TRUE( endpoints.GetCount() == members[ 0 ].endpoints.GetCount() && !node.GetPublicKeyString().IsNULLOrEmpty() );
        for ( UInt32 i=0; i<endpoints.GetCount(); ++i )
            ASSERT_TRUE( members[ 0 ].endpoints.Get( i ).GetParams().GetValueAlways( "key" ).AsString( CORE::CString::Empty, true ) == node.GetPublicKeyString() );

        // a connector tries the direct way before the relay, whatever the order of the list
        DRN::CDRNEndpointList relayFirst;
        relayFirst.Add( endpoints.Get( 4 ) );
        relayFirst.Add( endpoints.Get( 0 ) );
        CLinkRecorder recorder;
        recorder.WatchNode( peer );
        DRN::CDRNConnectorPtr connector = peer.Connect( relayFirst );
        REQUIRE_TRUE( PumpUntil( recorder.operationalCount, 1, 10000 ) );
        ASSERT_TRUE( connector->HasSucceeded() && DRN::CDRNEndpoint::ChannelTypeUdp == connector->GetLink()->GetRemoteEndpoint().GetChannelType() );

        manager.Stop();
        ASSERT_TRUE( node.GetAdvertisedEndpoints().ToString() == node.GetListenEndpoints().ToString() );
        connector.Unlink();
        peer.Stop();
        node.Stop();
        relay.Stop();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
