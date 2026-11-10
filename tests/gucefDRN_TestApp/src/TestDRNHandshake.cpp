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

#ifndef GUCEF_COMCORE_CTCPSERVERSOCKET_H
#include "CTCPServerSocket.h"
#define GUCEF_COMCORE_CTCPSERVERSOCKET_H
#endif /* GUCEF_COMCORE_CTCPSERVERSOCKET_H ? */

#ifndef GUCEF_COMCORE_CTCPCLIENTSOCKET_H
#include "CTCPClientSocket.h"
#define GUCEF_COMCORE_CTCPCLIENTSOCKET_H
#endif /* GUCEF_COMCORE_CTCPCLIENTSOCKET_H ? */

#ifndef GUCEF_DRN_H
#include "gucefDRN.h"
#define GUCEF_DRN_H
#endif /* GUCEF_DRN_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestDRNTestUtils.h"
#include "TestDRNHandshake.h"

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

/*-------------------------------------------------------------------------*/

static DRN::CDRNNodeSettings
HandshakeNodeSettings( const CORE::CString& authenticatorType )
{
    DRN::CDRNNodeSettings settings;
    settings.echoIntervalMs = 50;
    settings.handshakeTimeoutMs = 2000;
    settings.pulseIntervalMs = 1;
    settings.auth.authenticatorType = authenticatorType;
    return settings;
}

/*-------------------------------------------------------------------------*/

/** Watches a connector */
class CConnectorRecorder : public CORE::CObserver
{
    public:
    CConnectorRecorder( void ) : connected( 0 ), failed( 0 ), failReason( DRN::DRN_LINKREASON_NONE ), failText() {}
    virtual ~CConnectorRecorder() { SignalUpcomingObserverDestruction(); }
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
            const DRN::CDRNLinkClosedInfo& info = static_cast< DRN::CDRNLinkClosedEventData* >( eventdata )->GetData();
            failReason = info.reason;
            failText = info.text;
            ++failed;
        }
    }
};

/*-------------------------------------------------------------------------*/

/** Links two nodes with the given settings and reports how it went */
class CHandshakeAttempt
{
    public:

    DRN::CDRNNode nodeA;
    DRN::CDRNNode nodeB;
    CLinkRecorder recorderA;
    CLinkRecorder recorderB;
    CConnectorRecorder connectorRecorder;
    DRN::CDRNConnectorPtr connector;

    bool Run( const DRN::CDRNNodeSettings& settingsA, const DRN::CDRNNodeSettings& settingsB, const CORE::CString& channelType )
    {
        if ( !nodeA.SetSettings( settingsA ) || !nodeB.SetSettings( settingsB ) )
            return false;
        recorderA.WatchNode( nodeA );
        recorderB.WatchNode( nodeB );
        UInt16 port = NextTestPort();
        if ( !nodeB.Listen( LoopbackEndpoint( channelType, port ) ) )
            return false;
        connector = nodeA.Connect( DRN::CDRNEndpointList( LoopbackEndpoint( channelType, port ) ) );
        connectorRecorder.SubscribeTo( connector.GetPointerAlways() );
        CDoneCondition condition( connectorRecorder );
        return PumpUntil( condition, 10000 );
    }

    ~CHandshakeAttempt()
    {
        recorderA.links.clear();
        recorderB.links.clear();
        connector.Unlink();
        nodeA.Stop();
        nodeB.Stop();
    }

    private:

    class CDoneCondition : public CTestCondition
    {
        public:
        explicit CDoneCondition( const CConnectorRecorder& recorder ) : m_recorder( recorder ) {}
        virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return m_recorder.connected + m_recorder.failed > 0; }
        private:
        const CConnectorRecorder& m_recorder;
    };
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDRNHandshakeTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefDRN.Handshake" );

    GUCEF_TESTFW_TESTCASE( "Handshake: anonymous links on tcp and udp" )
    try
    {
        const char* types[] = { "tcp", "udp" };
        for ( UInt32 t=0; t<2; ++t )
        {
            CHandshakeAttempt attempt;
            REQUIRE_TRUE( attempt.Run( HandshakeNodeSettings( "anonymous" ), HandshakeNodeSettings( "anonymous" ), types[ t ] ) );
            ASSERT_TRUE( 1 == attempt.connectorRecorder.connected );
            REQUIRE_TRUE( PumpUntil( attempt.recorderB.operationalCount, 1 ) );
            ASSERT_TRUE( 0 == attempt.recorderA.refusedCount && 0 == attempt.recorderB.refusedCount );
            ASSERT_TRUE( 0 != attempt.recorderB.links[ 0 ]->GetLinkId() );
            ASSERT_TRUE( attempt.recorderB.links[ 0 ]->GetLinkId() == attempt.recorderA.links[ 0 ]->GetLinkId() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Handshake: shared-secret with the same secret links" )
    try
    {
        DRN::CDRNNodeSettings settingsA = HandshakeNodeSettings( "shared-secret" );
        settingsA.auth.SetSetting( "secret", "lan password" );
        DRN::CDRNNodeSettings settingsB = settingsA;
        settingsB.nodeId = DRN::CDRNNodeSettings::GenerateNodeId();
        CHandshakeAttempt attempt;
        REQUIRE_TRUE( attempt.Run( settingsA, settingsB, "tcp" ) );
        ASSERT_TRUE( 1 == attempt.connectorRecorder.connected );
        REQUIRE_TRUE( PumpUntil( attempt.recorderB.operationalCount, 1 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Handshake: shared-secret with another secret is refused on both ends" )
    try
    {
        DRN::CDRNNodeSettings settingsA = HandshakeNodeSettings( "shared-secret" );
        settingsA.auth.SetSetting( "secret", "guess" );
        DRN::CDRNNodeSettings settingsB = HandshakeNodeSettings( "shared-secret" );
        settingsB.auth.SetSetting( "secret", "lan password" );
        CHandshakeAttempt attempt;
        REQUIRE_TRUE( attempt.Run( settingsA, settingsB, "udp" ) );
        ASSERT_TRUE( 1 == attempt.connectorRecorder.failed );
        ASSERT_TRUE( DRN::DRN_LINKREASON_AUTH_FAILED == attempt.connectorRecorder.failReason );
        ASSERT_TRUE( 1 == attempt.recorderA.refusedCount && DRN::DRN_LINKREASON_AUTH_FAILED == attempt.recorderA.lastRefusedReason );
        ASSERT_TRUE( 1 == attempt.recorderB.refusedCount && DRN::DRN_LINKREASON_AUTH_FAILED == attempt.recorderB.lastRefusedReason );
        ASSERT_TRUE( PumpUntil( attempt.recorderB.closedCount, 1, 3000 ) );
        ASSERT_TRUE( 0 == attempt.recorderA.operationalCount && 0 == attempt.recorderB.operationalCount );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Handshake: another authentication method is refused" )
    try
    {
        DRN::CDRNNodeSettings settingsB = HandshakeNodeSettings( "shared-secret" );
        settingsB.auth.SetSetting( "secret", "lan password" );
        CHandshakeAttempt attempt;
        REQUIRE_TRUE( attempt.Run( HandshakeNodeSettings( "anonymous" ), settingsB, "tcp" ) );
        ASSERT_TRUE( 1 == attempt.connectorRecorder.failed );
        ASSERT_TRUE( DRN::DRN_LINKREASON_AUTH_METHOD == attempt.connectorRecorder.failReason );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Handshake: another service is refused" )
    try
    {
        DRN::CDRNNodeSettings settingsA = HandshakeNodeSettings( "anonymous" );
        settingsA.serviceName = "chess";
        CHandshakeAttempt attempt;
        REQUIRE_TRUE( attempt.Run( settingsA, HandshakeNodeSettings( "anonymous" ), "tcp" ) );
        ASSERT_TRUE( 1 == attempt.connectorRecorder.failed );
        ASSERT_TRUE( DRN::DRN_LINKREASON_SERVICE_MISMATCH == attempt.connectorRecorder.failReason );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Handshake: token issued with the host's key links, the accepting side knows the user" )
    try
    {
        CORE::CString token = DRN::CDRNTokenAuthenticator::IssueToken( "lobby key", "arena", "alice", "player", DRN::CDRNTokenAuthenticator::GetUnixTimeNow() + 600 );
        DRN::CDRNNodeSettings settingsA = HandshakeNodeSettings( "token" );
        settingsA.auth.SetSetting( "token", token );
        DRN::CDRNNodeSettings settingsB = HandshakeNodeSettings( "token" );
        settingsB.auth.SetSetting( "key", "lobby key" );
        settingsB.auth.SetSetting( "expectedSession", "arena" );
        CHandshakeAttempt attempt;
        REQUIRE_TRUE( attempt.Run( settingsA, settingsB, "tcp" ) );
        ASSERT_TRUE( 1 == attempt.connectorRecorder.connected );
        REQUIRE_TRUE( PumpUntil( attempt.recorderB.operationalCount, 1 ) );
        const DRN::CDRNAuthResult& result = attempt.recorderB.links[ 0 ]->GetAuthResult();
        ASSERT_TRUE( result.userId == "alice" && result.role == "player" && result.sessionName == "arena" );

        // the token round trips
        CORE::CDynamicBuffer binary;
        binary.SetBufferSize( token.ByteSize(), false );
        UInt32 used = 0;
        ASSERT_TRUE( CORE::Base64Decode( token, binary.GetBufferPtr(), token.ByteSize(), used ) );
        CORE::CString session, user, role;
        UInt64 expiry = 0;
        ASSERT_TRUE( DRN::CDRNTokenAuthenticator::ParseToken( "lobby key", static_cast< const UInt8* >( binary.GetConstBufferPtr() ), used, session, user, role, expiry ) );
        ASSERT_FALSE( DRN::CDRNTokenAuthenticator::ParseToken( "other key", static_cast< const UInt8* >( binary.GetConstBufferPtr() ), used, session, user, role, expiry ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Handshake: an expired token, a token signed with another key and one for another session are refused" )
    try
    {
        CORE::CString tokens[ 3 ];
        tokens[ 0 ] = DRN::CDRNTokenAuthenticator::IssueToken( "lobby key", "arena", "bob", "player", DRN::CDRNTokenAuthenticator::GetUnixTimeNow() - 5 );
        tokens[ 1 ] = DRN::CDRNTokenAuthenticator::IssueToken( "forged key", "arena", "eve", "admin", DRN::CDRNTokenAuthenticator::GetUnixTimeNow() + 600 );
        tokens[ 2 ] = DRN::CDRNTokenAuthenticator::IssueToken( "lobby key", "other arena", "carol", "player", DRN::CDRNTokenAuthenticator::GetUnixTimeNow() + 600 );
        const char* expectedText[ 3 ] = { "token expired", "invalid token", "token is for another session" };
        for ( UInt32 i=0; i<3; ++i )
        {
            DRN::CDRNNodeSettings settingsA = HandshakeNodeSettings( "token" );
            settingsA.auth.SetSetting( "token", tokens[ i ] );
            DRN::CDRNNodeSettings settingsB = HandshakeNodeSettings( "token" );
            settingsB.auth.SetSetting( "key", "lobby key" );
            settingsB.auth.SetSetting( "expectedSession", "arena" );
            CHandshakeAttempt attempt;
            REQUIRE_TRUE( attempt.Run( settingsA, settingsB, "udp" ) );
            ASSERT_TRUE( 1 == attempt.connectorRecorder.failed );
            ASSERT_TRUE( DRN::DRN_LINKREASON_AUTH_FAILED == attempt.connectorRecorder.failReason );
            ASSERT_TRUE( attempt.connectorRecorder.failText == expectedText[ i ] );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Handshake: a peer that accepts the connection but never answers times out" )
    try
    {
        // a plain TCP server that takes the connection and stays silent
        UInt16 port = NextTestPort();
        COMCORE::CTCPServerSocket silentServer( CORE::CCoreGlobal::Instance()->GetPulseGenerator(), false );
        ASSERT_TRUE( silentServer.ListenOnPort( port ) );

        DRN::CDRNNodeSettings settings = HandshakeNodeSettings( "anonymous" );
        settings.handshakeTimeoutMs = 300;
        settings.connectTimeoutMs = 5000;
        DRN::CDRNNode node;
        ASSERT_TRUE( node.SetSettings( settings ) );
        CConnectorRecorder recorder;
        DRN::CDRNConnectorPtr connector = node.Connect( DRN::CDRNEndpointList( LoopbackEndpoint( "tcp", port ) ) );
        recorder.SubscribeTo( connector.GetPointerAlways() );
        UInt64 start = DRN::CDRNClock::NowInMilliseconds();
        ASSERT_TRUE( PumpUntil( recorder.failed, 1, 5000 ) );
        UInt64 took = DRN::CDRNClock::NowInMilliseconds() - start;
        ASSERT_TRUE( DRN::DRN_LINKREASON_HANDSHAKE_TIMEOUT == recorder.failReason );
        ASSERT_TRUE( took >= 250 && took < 2000 );
        connector.Unlink();
        node.Stop();
        silentServer.Close();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Handshake: a peer that connects but never says HELLO times out on the accepting side" )
    try
    {
        DRN::CDRNNodeSettings settings = HandshakeNodeSettings( "anonymous" );
        settings.handshakeTimeoutMs = 300;
        DRN::CDRNNode node;
        ASSERT_TRUE( node.SetSettings( settings ) );
        CLinkRecorder recorder;
        recorder.WatchNode( node );
        UInt16 port = NextTestPort();
        ASSERT_TRUE( node.Listen( LoopbackEndpoint( "tcp", port ) ) );

        COMCORE::CTCPClientSocket silentClient( CORE::CCoreGlobal::Instance()->GetPulseGenerator(), false );
        ASSERT_TRUE( silentClient.ConnectTo( "127.0.0.1", port ) );
        ASSERT_TRUE( PumpUntil( recorder.closedCount, 1, 3000 ) );
        ASSERT_TRUE( DRN::DRN_LINKREASON_HANDSHAKE_TIMEOUT == recorder.lastClosedReason );
        ASSERT_TRUE( 0 == recorder.operationalCount );
        PumpFor( 50 );
        ASSERT_TRUE( 0 == node.GetLinkCount() );
        silentClient.Close();
        node.Stop();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Handshake: a refused connector does not try further endpoints, nobody listening ends in a failure" )
    try
    {
        DRN::CDRNNodeSettings settingsB = HandshakeNodeSettings( "shared-secret" );
        settingsB.auth.SetSetting( "secret", "x" );
        DRN::CDRNNodeSettings settingsA = HandshakeNodeSettings( "shared-secret" );
        settingsA.auth.SetSetting( "secret", "y" );
        settingsA.connectTimeoutMs = 300;
        DRN::CDRNNode nodeA;
        DRN::CDRNNode nodeB;
        ASSERT_TRUE( nodeA.SetSettings( settingsA ) && nodeB.SetSettings( settingsB ) );
        UInt16 port = NextTestPort();
        ASSERT_TRUE( nodeB.Listen( LoopbackEndpoint( "tcp", port ) ) );

        DRN::CDRNEndpointList endpoints;
        endpoints.Add( LoopbackEndpoint( "tcp", port ) );
        endpoints.Add( LoopbackEndpoint( "udp", NextTestPort() ) );
        CConnectorRecorder recorder;
        DRN::CDRNConnectorPtr connector = nodeA.Connect( endpoints );
        recorder.SubscribeTo( connector.GetPointerAlways() );
        ASSERT_TRUE( PumpUntil( recorder.failed, 1, 5000 ) );
        ASSERT_TRUE( 1 == connector->GetAttemptCount() );

        CConnectorRecorder nobodyRecorder;
        DRN::CDRNConnectorPtr nobody = nodeA.Connect( DRN::CDRNEndpointList( LoopbackEndpoint( "tcp", NextTestPort() ) ) );
        nobodyRecorder.SubscribeTo( nobody.GetPointerAlways() );
        ASSERT_TRUE( PumpUntil( nobodyRecorder.failed, 1, 5000 ) );

        CConnectorRecorder unknownRecorder;
        DRN::CDRNConnectorPtr unknown = nodeA.Connect( DRN::CDRNEndpointList( DRN::CDRNEndpoint( "carrier-pigeon://loft:1" ) ) );
        ASSERT_TRUE( unknown->IsDone() && !unknown->HasSucceeded() );

        connector.Unlink();
        nobody.Unlink();
        unknown.Unlink();
        nodeA.Stop();
        nodeB.Stop();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
