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

#include <algorithm>

#include <exception>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_DRN_H
#include "gucefDRN.h"
#define GUCEF_DRN_H
#endif /* GUCEF_DRN_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestDRNTestUtils.h"
#include "TestDRNSessions.h"

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

typedef GUCEF::vector< CORE::CString > TStringVector;

/*-------------------------------------------------------------------------*/

/** Records what a session tells the application */
class CSessionRecorder : public CORE::CObserver
{
    public:

    CSessionRecorder( void )
        : joined( 0 ), joinFailed( 0 ), memberJoined( 0 ), memberLeft( 0 ), closed( 0 ), appMessages( 0 )
        , localPeerId( 0 ), closedReason( 0 ), appLog(), leftPeers(), eventThreadId( 0 ), wrongThreadEvents( 0 ), expectedThreadId( 0 )
    {}

    virtual ~CSessionRecorder() { SignalUpcomingObserverDestruction(); }

    UInt32 joined;
    UInt32 joinFailed;
    UInt32 memberJoined;
    UInt32 memberLeft;
    UInt32 closed;
    UInt32 appMessages;
    UInt32 localPeerId;
    UInt32 closedReason;
    TStringVector appLog;
    GUCEF::vector< UInt32 > leftPeers;
    UInt32 eventThreadId;
    UInt32 wrongThreadEvents;
    UInt32 expectedThreadId;

    protected:

    virtual void OnNotify( CORE::CNotifier* /* notifier */, const CORE::CEvent& eventid, CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !( DRN::CDRNSession::JoinedEvent == eventid || DRN::CDRNSession::JoinFailedEvent == eventid ||
                DRN::CDRNSession::MemberJoinedEvent == eventid || DRN::CDRNSession::MemberLeftEvent == eventid ||
                DRN::CDRNSession::SessionClosedEvent == eventid || DRN::CDRNSession::AppMessageEvent == eventid ) )
            return;

        eventThreadId = MT::GetCurrentTaskID();
        if ( 0 != expectedThreadId && eventThreadId != expectedThreadId )
            ++wrongThreadEvents;

        if ( DRN::CDRNSession::JoinedEvent == eventid )
        {
            localPeerId = static_cast< CORE::TCloneableUInt32* >( eventdata )->GetData();
            ++joined;
        }
        else
        if ( DRN::CDRNSession::JoinFailedEvent == eventid )
            ++joinFailed;
        else
        if ( DRN::CDRNSession::MemberJoinedEvent == eventid )
            ++memberJoined;
        else
        if ( DRN::CDRNSession::MemberLeftEvent == eventid )
        {
            leftPeers.push_back( static_cast< DRN::CDRNSessionMemberEventData* >( eventdata )->GetData().peerId );
            ++memberLeft;
        }
        else
        if ( DRN::CDRNSession::SessionClosedEvent == eventid )
        {
            closedReason = static_cast< CORE::TCloneableUInt32* >( eventdata )->GetData();
            ++closed;
        }
        else
        if ( DRN::CDRNSession::AppMessageEvent == eventid )
        {
            const DRN::CDRNAppMessage& message = static_cast< DRN::CDRNAppMessageEventData* >( eventdata )->GetData();
            CORE::CString text( static_cast< const char* >( message.payload.GetConstBufferPtr() ), message.payload.GetDataSize() );
            appLog.push_back( CORE::UInt32ToString( message.fromPeer ) + ">" + CORE::UInt32ToString( message.toPeer ) +
                              " lane " + CORE::UInt32ToString( message.lane ) + " " + text );
            ++appMessages;
        }
    }
};

/*-------------------------------------------------------------------------*/

/** All recorders reached a count */
class CAllCondition : public CTestCondition
{
    public:
    enum TWhat { MEMBERS, DIRECT_LINKS, APP_MESSAGES, MEMBER_LEFT, CLOSED };
    CAllCondition( GUCEF::vector< DRN::CDRNSession* >& sessions, GUCEF::vector< CSessionRecorder* >& recorders, TWhat what, UInt32 expected, UInt32 first, UInt32 last )
        : m_sessions( sessions ), m_recorders( recorders ), m_what( what ), m_expected( expected ), m_first( first ), m_last( last ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        for ( UInt32 i=m_first; i<=m_last; ++i )
        {
            UInt32 value = 0;
            switch ( m_what )
            {
                case MEMBERS: value = m_sessions[ i ]->GetMemberCount(); break;
                case DIRECT_LINKS: value = m_sessions[ i ]->GetDirectLinkCount(); break;
                case APP_MESSAGES: value = m_recorders[ i ]->appMessages; break;
                case MEMBER_LEFT: value = m_recorders[ i ]->memberLeft; break;
                case CLOSED: value = m_recorders[ i ]->closed; break;
            }
            if ( value < m_expected )
                return false;
        }
        return true;
    }
    private:
    GUCEF::vector< DRN::CDRNSession* >& m_sessions;
    GUCEF::vector< CSessionRecorder* >& m_recorders;
    TWhat m_what;
    UInt32 m_expected;
    UInt32 m_first;
    UInt32 m_last;
};

/*-------------------------------------------------------------------------*/

static CORE::CString
JoinSorted( TStringVector values )
{
    std::sort( values.begin(), values.end() );
    CORE::CString result;
    for ( UInt32 i=0; i<values.size(); ++i )
        result += ( i > 0 ? "|" : "" ) + values[ i ];
    return result;
}

/*-------------------------------------------------------------------------*/

/**
 *  The same scenario for every topology: N members join one after the other, every member sends a message to all
 *  and one to the next member, the last member leaves, then the host ends the session.
 *  @return a description of what the application of every member saw, empty when a step did not complete
 */
static CORE::CString
RunSessionScenario( DRN::TDRNTopology topology, UInt32 peerCount, const CORE::CString& channelType, bool ownThread, CORE::CString& failedStep )
{
    GUCEF::vector< DRN::CDRNNode* > nodes;
    GUCEF::vector< DRN::CDRNSession* > sessions;
    GUCEF::vector< CSessionRecorder* > recorders;
    GUCEF::vector< DRN::CDRNEndpoint > endpoints;
    CORE::CString result;
    UInt32 mainThreadId = MT::GetCurrentTaskID();
    CORE::CString sessionName = "arena-" + CORE::UInt32ToString( peerCount ) + "-" + CORE::UInt32ToString( (UInt32) topology );

    for ( UInt32 i=0; i<peerCount; ++i )
    {
        DRN::CDRNNodeSettings settings;
        settings.echoIntervalMs = 50;
        settings.pulseIntervalMs = 1;
        settings.ownThread = ownThread;
        DRN::CDRNNode* node = GUCEF_NEW DRN::CDRNNode();
        node->SetSettings( settings );
        nodes.push_back( node );
        endpoints.push_back( LoopbackEndpoint( channelType, NextTestPort() ) );
        if ( !node->Listen( endpoints.back() ) )
            failedStep = "listen";
        sessions.push_back( node->CreateSession() );
        CSessionRecorder* recorder = GUCEF_NEW CSessionRecorder();
        recorder->expectedThreadId = mainThreadId;
        recorder->SubscribeTo( sessions.back() );
        recorders.push_back( recorder );
    }

    do
    {
        if ( !failedStep.IsNULLOrEmpty() )
            break;

        DRN::CDRNSessionSettings hostSettings;
        hostSettings.sessionName = sessionName;
        hostSettings.topology = topology;
        if ( !sessions[ 0 ]->Host( hostSettings ) )
        {
            failedStep = "host";
            break;
        }

        DRN::CDRNSessionSettings joinSettings;
        joinSettings.sessionName = sessionName;
        for ( UInt32 i=1; i<peerCount; ++i )
        {
            if ( !sessions[ i ]->Join( DRN::CDRNEndpointList( endpoints[ 0 ] ), joinSettings ) || !PumpUntil( recorders[ i ]->joined, 1, 10000 ) )
            {
                failedStep = "join " + CORE::UInt32ToString( i );
                break;
            }
        }
        if ( !failedStep.IsNULLOrEmpty() )
            break;

        CAllCondition allMembers( sessions, recorders, CAllCondition::MEMBERS, peerCount, 0, peerCount-1 );
        if ( !PumpUntil( allMembers, 10000 ) )
        {
            failedStep = "membership";
            break;
        }
        if ( DRN::DRN_TOPOLOGY_PEER_TO_PEER == topology )
        {
            CAllCondition mesh( sessions, recorders, CAllCondition::DIRECT_LINKS, peerCount-1, 0, peerCount-1 );
            if ( !PumpUntil( mesh, 10000 ) )
            {
                failedStep = "mesh";
                break;
            }
        }

        // every member to all, on the reliable ordered lane and on the unreliable one
        for ( UInt32 i=0; i<peerCount; ++i )
        {
            CORE::CString text = "hello from " + CORE::UInt32ToString( sessions[ i ]->GetLocalPeerId() );
            if ( !sessions[ i ]->SendAppMessage( 0, DRN::DRN_LANE_RELIABLE_ORDERED, text.C_String(), text.Length() ) ||
                 !sessions[ i ]->SendAppMessage( 0, DRN::DRN_LANE_UNRELIABLE, text.C_String(), text.Length() ) )
                failedStep = "send to all";
        }
        CAllCondition toAll( sessions, recorders, CAllCondition::APP_MESSAGES, 2 * ( peerCount-1 ), 0, peerCount-1 );
        if ( !failedStep.IsNULLOrEmpty() || !PumpUntil( toAll, 10000 ) )
        {
            if ( failedStep.IsNULLOrEmpty() )
                failedStep = "receive to all";
            break;
        }

        // every member to the next one
        for ( UInt32 i=0; i<peerCount; ++i )
        {
            UInt32 to = sessions[ ( i + 1 ) % peerCount ]->GetLocalPeerId();
            CORE::CString text = "just for " + CORE::UInt32ToString( to );
            if ( !sessions[ i ]->SendAppMessage( to, DRN::DRN_LANE_RELIABLE_UNORDERED, text.C_String(), text.Length() ) )
                failedStep = "send to one";
        }
        CAllCondition toOne( sessions, recorders, CAllCondition::APP_MESSAGES, 2 * ( peerCount-1 ) + 1, 0, peerCount-1 );
        if ( !failedStep.IsNULLOrEmpty() || !PumpUntil( toOne, 10000 ) )
        {
            if ( failedStep.IsNULLOrEmpty() )
                failedStep = "receive to one";
            break;
        }
        PumpFor( 50 );

        // the last member leaves
        sessions[ peerCount-1 ]->Leave();
        CAllCondition left( sessions, recorders, CAllCondition::MEMBER_LEFT, 1, 0, peerCount-2 );
        if ( !PumpUntil( left, 10000 ) )
        {
            failedStep = "leave";
            break;
        }
        CAllCondition fewer( sessions, recorders, CAllCondition::MEMBERS, peerCount-1, 0, peerCount-2 );
        if ( !PumpUntil( fewer, 10000 ) )
        {
            failedStep = "member count after leave";
            break;
        }

        // the host ends the session
        sessions[ 0 ]->Leave();
        if ( peerCount > 2 )
        {
            CAllCondition ended( sessions, recorders, CAllCondition::CLOSED, 1, 1, peerCount-2 );
            if ( !PumpUntil( ended, 10000 ) )
            {
                failedStep = "session end";
                break;
            }
        }
        PumpFor( 50 );

        for ( UInt32 i=0; i<peerCount; ++i )
        {
            CSessionRecorder& recorder = *recorders[ i ];
            TStringVector left;
            for ( UInt32 n=0; n<recorder.leftPeers.size(); ++n )
                left.push_back( CORE::UInt32ToString( recorder.leftPeers[ n ] ) );
            result += "member " + CORE::UInt32ToString( 0 == i ? 1 : recorder.localPeerId ) +
                      " joined " + CORE::UInt32ToString( recorder.joined ) +
                      " saw " + CORE::UInt32ToString( recorder.memberJoined ) + " join" +
                      " [" + JoinSorted( recorder.appLog ) + "]" +
                      " left [" + JoinSorted( left ) + "]" +
                      " closed " + CORE::UInt32ToString( recorder.closed ) + " reason " + CORE::UInt32ToString( recorder.closedReason ) +
                      " wrong thread " + CORE::UInt32ToString( recorder.wrongThreadEvents ) + "\n";
        }
    }
    while ( false );

    for ( UInt32 i=0; i<peerCount; ++i )
    {
        GUCEF_DELETE recorders[ i ];
        nodes[ i ]->Stop();
        GUCEF_DELETE nodes[ i ];
    }
    return result;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDRNSessionTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefDRN.Sessions" );

    const UInt32 peerCounts[] = { 3, 8 };
    for ( UInt32 p=0; p<2; ++p )
    {
        UInt32 peerCount = peerCounts[ p ];
        const CORE::CString caseName = "Sessions: CLIENT_SERVER and PEER_TO_PEER with " + CORE::UInt32ToString( peerCount ) + " peers give the application the same results";
        GUCEF_TESTFW_TESTCASE( caseName.C_String() )
        try
        {
            CORE::CString failedCs;
            CORE::CString clientServer = RunSessionScenario( DRN::DRN_TOPOLOGY_CLIENT_SERVER, peerCount, "tcp", false, failedCs );
            ASSERT_TRUE( failedCs.IsNULLOrEmpty() );
            CORE::CString failedP2p;
            CORE::CString peerToPeer = RunSessionScenario( DRN::DRN_TOPOLOGY_PEER_TO_PEER, peerCount, "tcp", false, failedP2p );
            ASSERT_TRUE( failedP2p.IsNULLOrEmpty() );
            ASSERT_FALSE( clientServer.IsNULLOrEmpty() );
            ASSERT_TRUE( clientServer == peerToPeer );
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Session scenario with " << peerCount << " peers, as every member saw it:\n" << clientServer );
            if ( !failedCs.IsNULLOrEmpty() || !failedP2p.IsNULLOrEmpty() || clientServer != peerToPeer )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Session scenario, failed step \"" << failedCs << "\" / \"" << failedP2p << "\"\nCLIENT_SERVER:\n" << clientServer << "PEER_TO_PEER:\n" << peerToPeer );

            // what every member must have seen: the others' messages to all, the one meant for it
            for ( UInt32 i=0; i<peerCount; ++i )
            {
                UInt32 peerId = i + 1;
                UInt32 from = 0 == i ? peerCount : peerId - 1;
                CORE::CString expected = "just for " + CORE::UInt32ToString( peerId );
                ASSERT_TRUE( clientServer.HasSubstr( CORE::UInt32ToString( from ) + ">" + CORE::UInt32ToString( peerId ) + " lane 1 " + expected, true ) >= 0 );
                CORE::CString fromHost = "1>0 lane 0 hello from 1";
                ASSERT_TRUE( clientServer.HasSubstr( fromHost, true ) >= 0 );
            }
            ASSERT_TRUE( clientServer.HasSubstr( "wrong thread 1", true ) < 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        GUCEF_TESTFW_TESTCASE_END
    }

    GUCEF_TESTFW_TESTCASE( "Sessions: Joining a session that is not there fails, a full session refuses" )
    try
    {
        DRN::CDRNNode host;
        DRN::CDRNNode guest;
        DRN::CDRNNode third;
        DRN::CDRNEndpoint endpoint = LoopbackEndpoint( "tcp", NextTestPort() );
        ASSERT_TRUE( host.Listen( endpoint ) );
        DRN::CDRNSession* hosted = host.CreateSession();
        DRN::CDRNSessionSettings settings;
        settings.sessionName = "small";
        settings.maxMembers = 2;
        ASSERT_TRUE( hosted->Host( settings ) );

        DRN::CDRNSession* wrongName = guest.CreateSession();
        CSessionRecorder wrongNameRecorder;
        wrongNameRecorder.SubscribeTo( wrongName );
        DRN::CDRNSessionSettings wrong;
        wrong.sessionName = "nonexistent";
        ASSERT_TRUE( wrongName->Join( DRN::CDRNEndpointList( endpoint ), wrong ) );
        ASSERT_TRUE( PumpUntil( wrongNameRecorder.joinFailed, 1 ) );
        ASSERT_FALSE( wrongName->IsActive() );

        DRN::CDRNSession* first = guest.CreateSession();
        CSessionRecorder firstRecorder;
        firstRecorder.SubscribeTo( first );
        ASSERT_TRUE( first->Join( DRN::CDRNEndpointList( endpoint ), settings ) );
        REQUIRE_TRUE( PumpUntil( firstRecorder.joined, 1 ) );
        ASSERT_TRUE( 2 == firstRecorder.localPeerId && 2 == first->GetMemberCount() );
        ASSERT_FALSE( first->IsSequencer() );
        ASSERT_TRUE( hosted->IsSequencer() );

        DRN::CDRNSession* tooMany = third.CreateSession();
        CSessionRecorder tooManyRecorder;
        tooManyRecorder.SubscribeTo( tooMany );
        ASSERT_TRUE( tooMany->Join( DRN::CDRNEndpointList( endpoint ), settings ) );
        ASSERT_TRUE( PumpUntil( tooManyRecorder.joinFailed, 1 ) );
        ASSERT_TRUE( 2 == hosted->GetMemberCount() );

        third.Stop();
        guest.Stop();
        host.Stop();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Sessions: The session time of a member follows the host's clock" )
    try
    {
        DRN::CDRNNodeSettings settings;
        settings.echoIntervalMs = 20;
        settings.pulseIntervalMs = 1;
        DRN::CDRNNode host;
        DRN::CDRNNode member;
        ASSERT_TRUE( host.SetSettings( settings ) && member.SetSettings( settings ) );
        DRN::CDRNEndpoint endpoint = LoopbackEndpoint( "lossy-udp", NextTestPort(), "?delay=15" );
        ASSERT_TRUE( host.Listen( endpoint ) );

        DRN::CDRNSession* hosted = host.CreateSession();
        DRN::CDRNSessionSettings sessionSettings;
        sessionSettings.sessionName = "timed";
        ASSERT_TRUE( hosted->Host( sessionSettings ) );
        PumpFor( 200 );     // the session clock is running before the member joins

        DRN::CDRNSession* joined = member.CreateSession();
        CSessionRecorder recorder;
        recorder.SubscribeTo( joined );
        ASSERT_TRUE( joined->Join( DRN::CDRNEndpointList( endpoint ), sessionSettings ) );
        REQUIRE_TRUE( PumpUntil( recorder.joined, 1 ) );
        PumpFor( 1000 );

        UInt64 hostTime = hosted->GetSessionTime();
        UInt64 memberTime = joined->GetSessionTime();
        Int64 difference = (Int64) memberTime - (Int64) hostTime;
        ASSERT_TRUE( hostTime > 1100000 );
        ASSERT_TRUE( difference > -8000 && difference < 8000 );

        member.Stop();
        host.Stop();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Sessions: Nodes on threads of their own deliver the same results on the application's thread" )
    try
    {
        CORE::CString failedOwn;
        CORE::CString ownThread = RunSessionScenario( DRN::DRN_TOPOLOGY_PEER_TO_PEER, 3, "udp", true, failedOwn );
        ASSERT_TRUE( failedOwn.IsNULLOrEmpty() );
        CORE::CString failedPulsed;
        CORE::CString pulsed = RunSessionScenario( DRN::DRN_TOPOLOGY_PEER_TO_PEER, 3, "udp", false, failedPulsed );
        ASSERT_TRUE( failedPulsed.IsNULLOrEmpty() );
        ASSERT_FALSE( ownThread.IsNULLOrEmpty() );
        ASSERT_TRUE( ownThread == pulsed );
        ASSERT_TRUE( ownThread.HasSubstr( "wrong thread 0", true ) >= 0 );
        ASSERT_TRUE( ownThread.HasSubstr( "wrong thread 1", true ) < 0 );
        if ( !failedOwn.IsNULLOrEmpty() || ownThread != pulsed )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Own thread scenario, failed step \"" << failedOwn << "\" / \"" << failedPulsed << "\"\nOWN THREAD:\n" << ownThread << "PULSED:\n" << pulsed );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Sessions: A node on a thread of its own works while the application does not pulse" )
    try
    {
        DRN::CDRNNodeSettings settings;
        settings.ownThread = true;
        settings.echoIntervalMs = 20;
        DRN::CDRNNode nodeA;
        DRN::CDRNNode nodeB;
        ASSERT_TRUE( nodeA.SetSettings( settings ) && nodeB.SetSettings( settings ) );
        ASSERT_TRUE( nodeA.IsRunningOwnThread() && nodeB.IsRunningOwnThread() );
        DRN::CDRNEndpoint endpoint = LoopbackEndpoint( "tcp", NextTestPort() );
        ASSERT_TRUE( nodeB.Listen( endpoint ) );
        DRN::CDRNConnectorPtr connector = nodeA.Connect( DRN::CDRNEndpointList( endpoint ) );

        // no pulses on this thread: the link comes up on the nodes' threads
        bool linked = false;
        for ( UInt32 i=0; i<500 && !linked; ++i )
        {
            MT::ThreadDelay( 10 );
            linked = 1 == nodeA.GetLinkCount( true ) && 1 == nodeB.GetLinkCount( true );
        }
        ASSERT_TRUE( linked );

        // the threading cannot change with links around
        settings.ownThread = false;
        ASSERT_FALSE( nodeA.SetSettings( settings ) );

        connector.Unlink();
        nodeA.Stop();
        nodeB.Stop();
        ASSERT_FALSE( nodeA.IsRunningOwnThread() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
