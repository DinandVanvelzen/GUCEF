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

#include <string.h>

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
#include "TestDRNAllocationCounter.h"
#include "TestDRNReplication.h"

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

static UInt64
NowMs( void )
{
    return DRN::CDRNClock::NowInMicroseconds() / 1000;
}

/*-------------------------------------------------------------------------*/

/** A session with its members, each on a node of its own, all on loopback */
class CGroup
{
    public:

    GUCEF::vector< DRN::CDRNNode* > nodes;
    GUCEF::vector< DRN::CDRNSession* > sessions;
    GUCEF::vector< DRN::CDRNEndpoint > endpoints;
    DRN::CDRNNodeSettings nodeSettings;
    DRN::CDRNSessionSettings sessionSettings;
    CORE::CString channelType;
    CORE::CString params;

    CGroup( DRN::TDRNTopology topology, const CORE::CString& name, const CORE::CString& channel = "tcp", const CORE::CString& channelParams = CORE::CString::Empty )
        : nodes(), sessions(), endpoints(), nodeSettings(), sessionSettings(), channelType( channel ), params( channelParams )
    {
        nodeSettings.echoIntervalMs = 50;
        nodeSettings.pulseIntervalMs = 1;
        sessionSettings.sessionName = name;
        sessionSettings.topology = topology;
    }

    ~CGroup()
    {
        Stop();
    }

    DRN::CDRNSession& operator[]( UInt32 index ) { return *sessions[ index ]; }

    UInt32 GetSize( void ) const { return (UInt32) sessions.size(); }

    bool Start( UInt32 count )
    {
        if ( !AddNode() || !sessions[ 0 ]->Host( sessionSettings ) )
            return false;
        for ( UInt32 i=1; i<count; ++i )
            if ( !AddMember() )
                return false;
        return true;
    }

    bool AddMember( void )
    {
        if ( !AddNode() )
            return false;
        DRN::CDRNSessionSettings joinSettings;
        joinSettings.sessionName = sessionSettings.sessionName;
        if ( !sessions.back()->Join( DRN::CDRNEndpointList( endpoints[ 0 ] ), joinSettings ) )
            return false;
        return WaitForMembership();
    }

    bool WaitForMembership( void )
    {
        CMembership condition( *this );
        return PumpUntil( condition, 15000 );
    }

    void Stop( void )
    {
        for ( UInt32 i=0; i<nodes.size(); ++i )
        {
            nodes[ i ]->Stop();
            GUCEF_DELETE nodes[ i ];
        }
        nodes.clear();
        sessions.clear();
        endpoints.clear();
    }

    private:

    class CMembership : public CTestCondition
    {
        public:
        CGroup& m_group;
        explicit CMembership( CGroup& group ) : m_group( group ) {}
        virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE
        {
            UInt32 count = m_group.GetSize();
            for ( UInt32 i=0; i<count; ++i )
            {
                DRN::CDRNSession& session = *m_group.sessions[ i ];
                if ( !session.IsActive() || session.GetMemberCount() != count )
                    return false;
                if ( DRN::DRN_TOPOLOGY_PEER_TO_PEER == m_group.sessionSettings.topology && session.GetDirectLinkCount() != count-1 )
                    return false;
            }
            return true;
        }
    };

    bool AddNode( void )
    {
        DRN::CDRNNode* node = GUCEF_NEW DRN::CDRNNode();
        node->SetSettings( nodeSettings );
        nodes.push_back( node );
        endpoints.push_back( LoopbackEndpoint( channelType, NextTestPort(), params ) );
        sessions.push_back( node->CreateSession() );
        return node->Listen( endpoints.back() );
    }
};

/*-------------------------------------------------------------------------*/

/** Records what a grid or stream tells the application, can vote on its own */
class CItemRecorder : public CORE::CObserver
{
    public:

    enum TAutoVote { VOTE_NOT, VOTE_YES, VOTE_NO };

    UInt32 keyChanged;
    UInt32 updates;
    UInt32 updatedKeys;
    UInt32 snapshots;
    UInt32 removed;
    UInt32 proposeAccepted;
    UInt32 proposeRefused;
    UInt32 lastProposeOutcome;
    CORE::CString lastRefusal;
    UInt32 votesStarted;
    UInt32 votesEnded;
    UInt32 lastVoteOutcome;
    UInt32 entriesEvents;
    TAutoVote autoVote;

    CItemRecorder( void )
        : keyChanged( 0 ), updates( 0 ), updatedKeys( 0 ), snapshots( 0 ), removed( 0 ), proposeAccepted( 0 ), proposeRefused( 0 )
        , lastProposeOutcome( 0 ), lastRefusal(), votesStarted( 0 ), votesEnded( 0 ), lastVoteOutcome( 0 ), entriesEvents( 0 ), autoVote( VOTE_NOT )
    {}

    virtual ~CItemRecorder() { SignalUpcomingObserverDestruction(); }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( DRN::CDRNDataGrid::GridKeyChangedEvent == eventid )
            ++keyChanged;
        else
        if ( DRN::CDRNDataGrid::GridUpdatedEvent == eventid )
        {
            ++updates;
            updatedKeys += (UInt32) static_cast< DRN::CDRNGridUpdatedEventData* >( eventdata )->GetData().size();
        }
        else
        if ( DRN::CDRNDataItem::SnapshotCompleteEvent == eventid )
            ++snapshots;
        else
        if ( DRN::CDRNDataItem::ItemRemovedEvent == eventid )
            ++removed;
        else
        if ( DRN::CDRNDataGrid::ProposeResultEvent == eventid )
        {
            const DRN::CDRNProposeResult& result = static_cast< DRN::CDRNProposeResultEventData* >( eventdata )->GetData();
            if ( result.accepted )
                ++proposeAccepted;
            else
            {
                ++proposeRefused;
                lastRefusal = result.reason;
            }
            lastProposeOutcome = result.voteOutcome;
        }
        else
        if ( DRN::CDRNDataGrid::VoteStartedEvent == eventid )
        {
            ++votesStarted;
            if ( VOTE_NOT != autoVote )
            {
                const DRN::CDRNVoteInfo& info = static_cast< DRN::CDRNVoteEventData* >( eventdata )->GetData();
                static_cast< DRN::CDRNDataGrid* >( notifier )->CastVote( info.voteId, VOTE_YES == autoVote );
            }
        }
        else
        if ( DRN::CDRNDataGrid::VoteEndedEvent == eventid )
        {
            ++votesEnded;
            lastVoteOutcome = static_cast< DRN::CDRNVoteEventData* >( eventdata )->GetData().outcome;
        }
        else
        if ( DRN::CDRNDataStream::EntriesAvailableEvent == eventid )
            ++entriesEvents;
    }
};

/*-------------------------------------------------------------------------*/

/** Waits for grid values (or their absence) on several members */
class CGridCondition : public CTestCondition
{
    public:

    class CCheck
    {
        public:
        DRN::CDRNSession* session;
        CORE::CString gridName;
        UInt32 owner;
        CORE::CVariant key;
        CORE::CVariant expected;
        bool missing;
    };

    GUCEF::vector< CCheck > checks;

    void Expect( DRN::CDRNSession& session, const CORE::CString& gridName, UInt32 owner, const CORE::CVariant& key, const CORE::CVariant& expected )
    {
        CCheck check;
        check.session = &session;
        check.gridName = gridName;
        check.owner = owner;
        check.key = key;
        check.expected = expected;
        check.missing = false;
        checks.push_back( check );
    }

    void ExpectMissing( DRN::CDRNSession& session, const CORE::CString& gridName, UInt32 owner, const CORE::CVariant& key )
    {
        CCheck check;
        check.session = &session;
        check.gridName = gridName;
        check.owner = owner;
        check.key = key;
        check.missing = true;
        checks.push_back( check );
    }

    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        for ( UInt32 i=0; i<checks.size(); ++i )
        {
            const CCheck& check = checks[ i ];
            DRN::CDRNDataGrid* grid = check.session->FindGrid( check.gridName, check.owner );
            if ( GUCEF_NULL == grid || !grid->IsSnapshotComplete() )
                return false;
            CORE::CVariant value;
            bool has = grid->GetValue( check.key, value );
            if ( check.missing ? has : ( !has || !DRN::CDRNVariantCodec::IsSame( value, check.expected ) ) )
                return false;
        }
        return true;
    }
};

/*-------------------------------------------------------------------------*/

/** A grid as the application of a member sees it: "key=value,..." sorted */
static CORE::CString
DescribeGrid( DRN::CDRNSession& session, const CORE::CString& gridName )
{
    DRN::CDRNDataGrid* grid = session.FindGrid( gridName );
    if ( GUCEF_NULL == grid )
        return gridName + " none";
    GUCEF::vector< CORE::CVariant > keys;
    grid->GetKeys( keys );
    TStringVector entries;
    for ( UInt32 i=0; i<keys.size(); ++i )
    {
        CORE::CVariant value;
        grid->GetValue( keys[ i ], value );
        entries.push_back( keys[ i ].AsString() + "=" + value.AsString() );
    }
    std::sort( entries.begin(), entries.end() );
    CORE::CString result = gridName + " [";
    for ( UInt32 i=0; i<entries.size(); ++i )
        result += ( i > 0 ? "," : "" ) + entries[ i ];
    return result + "]";
}

/*-------------------------------------------------------------------------*/

/**
 *  The grid scenario for a topology: a member publishes an OWNER grid, the host a HOST grid, the others subscribe,
 *  values change and a key goes, a late member joins and catches up, the publisher leaves.
 */
static CORE::CString
RunGridScenario( DRN::TDRNTopology topology, CORE::CString& failedStep, bool ownThread = false )
{
    CGroup group( topology, "grids-" + CORE::UInt32ToString( (UInt32) topology ) );
    group.nodeSettings.ownThread = ownThread;
    CORE::CString result;
    do
    {
        if ( !group.Start( 3 ) )
        {
            failedStep = "start";
            break;
        }

        DRN::CDRNDataGridSettings ownerSettings;
        ownerSettings.name = "players";
        ownerSettings.authority = DRN::DRN_AUTHORITY_OWNER;
        ownerSettings.sendRateHz = 0;
        DRN::CDRNDataGrid* players = group[ 1 ].CreateGrid( ownerSettings );
        if ( GUCEF_NULL == players )
        {
            failedStep = "create players";
            break;
        }
        players->SetValue( "hp", CORE::CVariant( (Int32) 100 ) );
        players->SetValue( "name", CORE::CVariant( "two" ) );
        group[ 0 ].Subscribe( "players" );
        group[ 2 ].Subscribe( "players" );

        CGridCondition first;
        first.Expect( group[ 0 ], "players", 2, "hp", CORE::CVariant( (Int32) 100 ) );
        first.Expect( group[ 0 ], "players", 2, "name", CORE::CVariant( "two" ) );
        first.Expect( group[ 2 ], "players", 2, "hp", CORE::CVariant( (Int32) 100 ) );
        first.Expect( group[ 2 ], "players", 2, "name", CORE::CVariant( "two" ) );
        if ( !PumpUntil( first, 10000 ) )
        {
            failedStep = "first values";
            break;
        }

        players->SetValue( "hp", CORE::CVariant( (Int32) 90 ) );
        players->SetValue( "mana", CORE::CVariant( (Int32) 5 ) );
        players->RemoveKey( "name" );
        CGridCondition second;
        for ( UInt32 i=0; i<3; i+=2 )
        {
            second.Expect( group[ i ], "players", 2, "hp", CORE::CVariant( (Int32) 90 ) );
            second.Expect( group[ i ], "players", 2, "mana", CORE::CVariant( (Int32) 5 ) );
            second.ExpectMissing( group[ i ], "players", 2, "name" );
        }
        if ( !PumpUntil( second, 10000 ) )
        {
            failedStep = "changed values";
            break;
        }

        DRN::CDRNDataGridSettings hostSettings;
        hostSettings.name = "world";
        hostSettings.authority = DRN::DRN_AUTHORITY_HOST;
        DRN::CDRNDataGrid* world = group[ 0 ].CreateGrid( hostSettings );
        if ( GUCEF_NULL == world || GUCEF_NULL != group[ 1 ].CreateGrid( hostSettings ) )
        {
            failedStep = "create world";
            break;
        }
        world->SetValue( "round", CORE::CVariant( (Int32) 1 ) );
        group[ 1 ].Subscribe( "world" );
        group[ 2 ].Subscribe( "world" );

        // a member that joins late catches up
        if ( !group.AddMember() )
        {
            failedStep = "late join";
            break;
        }
        group[ 3 ].Subscribe( "players" );
        group[ 3 ].Subscribe( "world" );
        CGridCondition third;
        for ( UInt32 i=1; i<4; ++i )
            third.Expect( group[ i ], "world", 1, "round", CORE::CVariant( (Int32) 1 ) );
        third.Expect( group[ 3 ], "players", 2, "hp", CORE::CVariant( (Int32) 90 ) );
        third.Expect( group[ 3 ], "players", 2, "mana", CORE::CVariant( (Int32) 5 ) );
        third.ExpectMissing( group[ 3 ], "players", 2, "name" );
        if ( !PumpUntil( third, 10000 ) )
        {
            failedStep = "late member";
            break;
        }

        for ( UInt32 i=0; i<4; ++i )
            result += "member " + CORE::UInt32ToString( group[ i ].GetLocalPeerId() ) + " " + DescribeGrid( group[ i ], "players" ) + " " + DescribeGrid( group[ i ], "world" ) + "\n";

        // the publisher leaves: its grid goes away everywhere
        group[ 1 ].Leave();
        bool gone = false;
        UInt64 start = NowMs();
        while ( !gone && NowMs() - start < 10000 )
        {
            PumpFor( 5 );
            gone = GUCEF_NULL == group[ 0 ].FindGrid( "players" ) && GUCEF_NULL == group[ 2 ].FindGrid( "players" ) && GUCEF_NULL == group[ 3 ].FindGrid( "players" );
        }
        if ( !gone )
        {
            failedStep = "publisher left";
            break;
        }
        result += "after leaving: " + DescribeGrid( group[ 3 ], "players" ) + " " + DescribeGrid( group[ 3 ], "world" ) + "\n";
    }
    while ( false );
    return result;
}

/*-------------------------------------------------------------------------*/

/** Writes a stream entry: its number, a text and a float */
static bool
AppendNumberedEntry( DRN::CDRNDataStream& stream, UInt32 number, const CORE::CString& text )
{
    CORE::CVariantStream& entry = stream.GetWriteStream();
    entry << number << text << (Float32) ( number * 0.5f );
    entry.WriteSegmentEnd();
    if ( stream.CommitEntry() )
        return true;
    entry.Clear();
    return false;
}

/*-------------------------------------------------------------------------*/

/** Reads a stream entry written by AppendNumberedEntry */
static bool
ReadNumberedEntry( DRN::CDRNDataStream& stream, UInt32& number, CORE::CString& text )
{
    CORE::CVariantStream entry;
    if ( !stream.ReadNextEntry( entry ) )
        return false;
    Float32 half = 0;
    entry >> number >> text >> half;
    return entry.IsValid() && half == number * 0.5f;
}

/*-------------------------------------------------------------------------*/

class CEntryCountCondition : public CTestCondition
{
    public:
    DRN::CDRNSession& m_session;
    CORE::CString m_name;
    UInt32 m_count;
    CEntryCountCondition( DRN::CDRNSession& session, const CORE::CString& name, UInt32 count ) : m_session( session ), m_name( name ), m_count( count ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        DRN::CDRNDataStream* stream = m_session.FindStream( m_name );
        return GUCEF_NULL != stream && stream->GetEntryCount() >= m_count;
    }
};

/*-------------------------------------------------------------------------*/

/** Refuses writes of one key */
class CRefusingValidator : public DRN::CIDRNWriteValidator
{
    public:
    virtual bool ValidateWrite( UInt32 /* proposerPeerId */, const DRN::CDRNDataGrid& /* grid */, const CORE::CVariant& key, const CORE::CVariant& /* value */, CORE::CString& refusalReason ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( key.AsString() == "forbidden" )
        {
            refusalReason = "not today";
            return false;
        }
        return true;
    }
};

/*-------------------------------------------------------------------------*/

/** Answers incoming bulk transfers the way the test wants and counts how transfers end */
class CBulkRecorder : public CORE::CObserver
{
    public:

    enum TAnswer { ANSWER_MEMORY, ANSWER_FILE, ANSWER_REJECT, ANSWER_NONE };

    TAnswer answer;
    CORE::CString targetPath;
    DRN::CDRNBulkTransfer* incoming;
    UInt32 offers;
    UInt32 completed;
    UInt32 failed;
    UInt32 progress;
    CORE::CString lastFailure;

    CBulkRecorder( void )
        : answer( ANSWER_MEMORY ), targetPath(), incoming( GUCEF_NULL ), offers( 0 ), completed( 0 ), failed( 0 ), progress( 0 ), lastFailure()
    {}

    virtual ~CBulkRecorder() { SignalUpcomingObserverDestruction(); }

    void Watch( DRN::CDRNBulkTransfer* transfer )
    {
        if ( GUCEF_NULL != transfer )
            SubscribeTo( transfer );
    }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( DRN::CDRNBulkTransferManager::IncomingTransferEvent == eventid )
        {
            ++offers;
            const DRN::CDRNBulkTransferInfo& info = static_cast< DRN::CDRNBulkTransferEventData* >( eventdata )->GetData();
            incoming = static_cast< DRN::CDRNBulkTransferManager* >( notifier )->GetIncoming( info.peerId, info.transferId );
            Watch( incoming );
            if ( GUCEF_NULL == incoming )
                return;
            if ( ANSWER_MEMORY == answer )
                incoming->Accept();
            else
            if ( ANSWER_FILE == answer )
                incoming->Accept( targetPath );
            else
            if ( ANSWER_REJECT == answer )
                incoming->Reject( "not wanted" );
        }
        else
        if ( DRN::CDRNBulkTransfer::CompletedEvent == eventid )
            ++completed;
        else
        if ( DRN::CDRNBulkTransfer::FailedEvent == eventid )
        {
            ++failed;
            lastFailure = static_cast< CORE::TCloneableString* >( eventdata )->GetData();
        }
        else
        if ( DRN::CDRNBulkTransfer::ProgressEvent == eventid )
            ++progress;
    }
};

/*-------------------------------------------------------------------------*/

static void
FillPattern( CORE::CDynamicBuffer& buffer, UInt32 size, UInt32 seed )
{
    buffer.SetDataSize( 0 );
    buffer.SetBufferSize( size, false );
    UInt8* bytes = static_cast< UInt8* >( buffer.GetBufferPtr() );
    UInt32 state = seed * 2654435761u + 1;
    for ( UInt32 i=0; i<size; ++i )
    {
        state = state * 1103515245u + 12345u;
        bytes[ i ] = (UInt8) ( state >> 16 );
    }
    buffer.SetDataSize( size );
}

/*-------------------------------------------------------------------------*/

static bool
WriteTestFile( const CORE::CString& path, const CORE::CDynamicBuffer& data, UInt32 size )
{
    CORE::CFileAccess file( path, "wb" );
    if ( !file.Opened() )
        return false;
    bool ok = size == file.Write( data.GetConstBufferPtr(), 1, size );
    file.Close();
    return ok;
}

/*-------------------------------------------------------------------------*/

static bool
ReadTestFile( const CORE::CString& path, CORE::CDynamicBuffer& data )
{
    CORE::CFileAccess file( path, "rb" );
    if ( !file.Opened() )
        return false;
    UInt32 size = (UInt32) file.GetSize();
    data.SetDataSize( 0 );
    data.SetBufferSize( size, false );
    bool ok = size == file.Read( data.GetBufferPtr(), 1, size );
    data.SetDataSize( size );
    file.Close();
    return ok;
}

/*-------------------------------------------------------------------------*/

static bool
SameBytes( const CORE::CDynamicBuffer& a, const CORE::CDynamicBuffer& b )
{
    return a.GetDataSize() == b.GetDataSize() && 0 == memcmp( a.GetConstBufferPtr(), b.GetConstBufferPtr(), a.GetDataSize() );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDRNReplicationTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefDRN.Replication" );

    GUCEF_TESTFW_TESTCASE( "Grids: CLIENT_SERVER and PEER_TO_PEER give the same values, a late member catches up, a leaving publisher's grid goes" )
    try
    {
        CORE::CString failedCs;
        CORE::CString clientServer = RunGridScenario( DRN::DRN_TOPOLOGY_CLIENT_SERVER, failedCs );
        CORE::CString failedP2p;
        CORE::CString peerToPeer = RunGridScenario( DRN::DRN_TOPOLOGY_PEER_TO_PEER, failedP2p );
        ASSERT_TRUE( failedCs.IsNULLOrEmpty() );
        ASSERT_TRUE( failedP2p.IsNULLOrEmpty() );
        ASSERT_FALSE( clientServer.IsNULLOrEmpty() );
        ASSERT_TRUE( clientServer == peerToPeer );
        ASSERT_TRUE( clientServer.HasSubstr( "member 4 players [hp=90,mana=5] world [round=1]", true ) >= 0 );
        ASSERT_TRUE( clientServer.HasSubstr( "after leaving: players none world [round=1]", true ) >= 0 );
        if ( !failedCs.IsNULLOrEmpty() || !failedP2p.IsNULLOrEmpty() || clientServer != peerToPeer )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Grid scenario, failed step \"" << failedCs << "\" / \"" << failedP2p << "\"\nCLIENT_SERVER:\n" << clientServer << "PEER_TO_PEER:\n" << peerToPeer );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Grids: nodes on threads of their own give the application the same values" )
    try
    {
        CORE::CString failedOwn;
        CORE::CString ownThread = RunGridScenario( DRN::DRN_TOPOLOGY_CLIENT_SERVER, failedOwn, true );
        CORE::CString failedPulsed;
        CORE::CString pulsed = RunGridScenario( DRN::DRN_TOPOLOGY_CLIENT_SERVER, failedPulsed, false );
        ASSERT_TRUE( failedOwn.IsNULLOrEmpty() && failedPulsed.IsNULLOrEmpty() );
        ASSERT_FALSE( ownThread.IsNULLOrEmpty() );
        ASSERT_TRUE( ownThread == pulsed );
        if ( !failedOwn.IsNULLOrEmpty() || ownThread != pulsed )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Grid scenario on own threads, failed step \"" << failedOwn << "\"\nOWN THREAD:\n" << ownThread << "PULSED:\n" << pulsed );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Grids: 1000 updates of one key over a lossy channel arrive as the newest value, with fewer sends than updates" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "lossy-grid", "lossy-udp", "?loss=0.2&reorder=0.1&dup=0.05" );
        REQUIRE_TRUE( group.Start( 2 ) );

        DRN::CDRNDataGridSettings settings;
        settings.name = "counter";
        settings.authority = DRN::DRN_AUTHORITY_OWNER;
        settings.sendRateHz = 30;
        DRN::CDRNDataGrid* grid = group[ 0 ].CreateGrid( settings );
        REQUIRE_TRUE( GUCEF_NULL != grid );
        grid->SetValue( "v", CORE::CVariant( (UInt32) 0 ) );
        group[ 1 ].Subscribe( "counter" );
        CGridCondition subscribed;
        subscribed.Expect( group[ 1 ], "counter", 1, "v", CORE::CVariant( (UInt32) 0 ) );
        REQUIRE_TRUE( PumpUntil( subscribed, 15000 ) );

        UInt32 index = grid->GetKeyIndex( "v" );
        UInt64 deltasBefore = grid->GetDeltasSent();
        for ( UInt32 i=1; i<=1000; ++i )
        {
            grid->SetValueByIndex( index, CORE::CVariant( i ) );
            PumpFor( 1 );
        }
        CGridCondition newest;
        newest.Expect( group[ 1 ], "counter", 1, "v", CORE::CVariant( (UInt32) 1000 ) );
        ASSERT_TRUE( PumpUntil( newest, 15000 ) );
        UInt64 deltas = grid->GetDeltasSent() - deltasBefore;
        ASSERT_TRUE( deltas > 0 && deltas < 1000 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Lossy grid: 1000 updates went out in " << deltas << " deltas" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Grids: 64 players at 60 Hz stay within the byte budget and the subscriber gets the final positions" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "arena", "udp" );
        REQUIRE_TRUE( group.Start( 2 ) );

        const UInt32 playerCount = 64;
        const UInt32 budget = 1200;
        DRN::CDRNDataGridSettings settings;
        settings.name = "positions";
        settings.authority = DRN::DRN_AUTHORITY_OWNER;
        settings.sendRateHz = 60;
        settings.capacity = playerCount * 3;
        settings.maxBytesPerTick = budget;
        settings.defaultQuantisation = DRN::CDRNQuantisation( 2, 0 );
        DRN::CDRNDataGrid* grid = group[ 0 ].CreateGrid( settings );
        REQUIRE_TRUE( GUCEF_NULL != grid );
        GUCEF::vector< UInt32 > indices;
        for ( UInt32 i=0; i<playerCount*3; ++i )
            indices.push_back( grid->GetOrAddKeyIndex( CORE::CVariant( i ) ) );
        group[ 1 ].Subscribe( "positions" );
        CGridCondition subscribed;
        subscribed.ExpectMissing( group[ 1 ], "positions", 1, CORE::CVariant( (UInt32) 0 ) );
        REQUIRE_TRUE( PumpUntil( subscribed, 10000 ) );

        UInt64 bytesBefore = grid->GetBytesSent();
        UInt64 start = NowMs();
        UInt32 frame = 0;
        while ( NowMs() - start < 2000 )
        {
            ++frame;
            for ( UInt32 i=0; i<playerCount*3; ++i )
                grid->SetValueByIndex( indices[ i ], CORE::CVariant( (Float32) ( i + frame * 0.01f ) ) );
            UInt64 frameStart = NowMs();
            while ( NowMs() - frameStart < 16 )
                PumpFor( 1 );
        }
        UInt64 elapsed = NowMs() - start;
        UInt64 bytes = grid->GetBytesSent() - bytesBefore;
        UInt64 allowed = ( elapsed * 60 / 1000 + 2 ) * budget;
        ASSERT_TRUE( bytes > 0 && bytes <= allowed );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "64 players at 60 Hz: " << frame << " frames in " << elapsed << " ms, " << bytes << " bytes (" << ( bytes * 1000 / ( elapsed > 0 ? elapsed : 1 ) ) << " bytes/s), allowed " << allowed );

        // the newest positions arrive once the writes stop
        DRN::CDRNDataGrid* replica = group[ 1 ].FindGrid( "positions" );
        REQUIRE_TRUE( GUCEF_NULL != replica );
        bool converged = false;
        UInt64 waitStart = NowMs();
        while ( !converged && NowMs() - waitStart < 5000 )
        {
            PumpFor( 10 );
            converged = true;
            for ( UInt32 i=0; i<playerCount*3 && converged; ++i )
            {
                CORE::CVariant value;
                Float32 expected = (Float32) ( i + frame * 0.01f );
                converged = replica->GetValue( CORE::CVariant( i ), value ) && value.AsFloat32() > expected - 0.006f && value.AsFloat32() < expected + 0.006f;
            }
        }
        ASSERT_TRUE( converged );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Grids: On loopback a change reaches the subscriber within a tick plus transport" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "latency", "udp" );
        group.nodeSettings.pulseIntervalMs = 5;
        REQUIRE_TRUE( group.Start( 2 ) );

        DRN::CDRNDataGridSettings settings;
        settings.name = "ping";
        settings.authority = DRN::DRN_AUTHORITY_OWNER;
        settings.sendRateHz = 0;
        DRN::CDRNDataGrid* grid = group[ 0 ].CreateGrid( settings );
        REQUIRE_TRUE( GUCEF_NULL != grid );
        grid->SetValue( "t", CORE::CVariant( (UInt32) 0 ) );
        group[ 1 ].Subscribe( "ping" );
        CGridCondition subscribed;
        subscribed.Expect( group[ 1 ], "ping", 1, "t", CORE::CVariant( (UInt32) 0 ) );
        REQUIRE_TRUE( PumpUntil( subscribed, 10000 ) );
        DRN::CDRNDataGrid* replica = group[ 1 ].FindGrid( "ping" );

        CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        GUCEF::vector< UInt64 > samples;
        for ( UInt32 i=1; i<=21; ++i )
        {
            UInt64 sent = DRN::CDRNClock::NowInMicroseconds();
            grid->SetValue( "t", CORE::CVariant( i ) );
            CORE::CVariant value;
            while ( DRN::CDRNClock::NowInMicroseconds() - sent < 1000000 )
            {
                pulseGenerator->ForceDirectPulse();
                if ( replica->GetValue( "t", value ) && value.AsUInt32() == i )
                    break;
                MT::ThreadDelay( 0 );
            }
            samples.push_back( DRN::CDRNClock::NowInMicroseconds() - sent );
            PumpFor( 3 );
        }
        std::sort( samples.begin(), samples.end() );
        UInt64 median = samples[ samples.size() / 2 ];
        // a tick to send, transport, at most a tick to the end of the receiving tick
        ASSERT_TRUE( median <= 2 * 5000 + 5000 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Grid change to subscriber on loopback: median " << median << " us, best " << samples.front() << " us, worst " << samples.back() << " us" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Grids: Writing, sending, receiving and handing over numeric values does not allocate" )
    try
    {
        // in-process channels: the measurement covers DRN and the GUCEF event system, not a socket layer
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "no-alloc", "mem" );
        REQUIRE_TRUE( group.Start( 2 ) );

        DRN::CDRNDataGridSettings settings;
        settings.name = "hot";
        settings.authority = DRN::DRN_AUTHORITY_OWNER;
        settings.sendRateHz = 0;
        settings.defaultQuantisation = DRN::CDRNQuantisation( 3, 0 );
        DRN::CDRNDataGrid* grid = group[ 0 ].CreateGrid( settings );
        REQUIRE_TRUE( GUCEF_NULL != grid );
        GUCEF::vector< UInt32 > indices;
        for ( UInt32 i=0; i<16; ++i )
        {
            indices.push_back( grid->GetOrAddKeyIndex( CORE::CVariant( i ) ) );
            grid->SetValueByIndex( indices.back(), CORE::CVariant( (Float32) 0 ) );
        }
        group[ 1 ].Subscribe( "hot" );
        CGridCondition subscribed;
        subscribed.Expect( group[ 1 ], "hot", 1, CORE::CVariant( (UInt32) 15 ), CORE::CVariant( (Float32) 0 ) );
        REQUIRE_TRUE( PumpUntil( subscribed, 10000 ) );
        DRN::CDRNDataGrid* replica = group[ 1 ].FindGrid( "hot" );
        replica->SetNotifyMode( DRN::DRN_NOTIFY_PER_PACKET );
        CItemRecorder recorder;
        recorder.SubscribeTo( replica );

        CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        UInt32 allocations = 0;
        for ( UInt32 round=0; round<2; ++round )
        {
            // the first round warms up: buffers, queues and vectors grow to their size
            if ( 1 == round )
                StartAllocationCounting();
            for ( UInt32 frame=1; frame<=200; ++frame )
            {
                for ( UInt32 i=0; i<16; ++i )
                    grid->SetValueByIndex( indices[ i ], CORE::CVariant( (Float32) ( frame + i * 0.5f ) ) );
                for ( UInt32 p=0; p<3; ++p )
                {
                    pulseGenerator->ForceDirectPulse();
                    MT::ThreadDelay( 1 );
                }
            }
            if ( 1 == round )
                allocations = StopAllocationCounting();
        }
        ASSERT_TRUE( recorder.updates > 100 );
        if ( IsAllocationCountingAvailable() )
        {
            ASSERT_TRUE( 0 == allocations );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Grid hot path: " << allocations << " allocations in 200 frames, " << recorder.updates << " updates handed over" );
            if ( 0 != allocations )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Grid hot path allocations:\n" << GetCountedAllocationStacks() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    // the values were still handed over, only the allocation count could not be checked
    GUCEF_TESTFW_SKIP_IF( !IsAllocationCountingAvailable(), GetAllocationCountingUnavailableReason() );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Grids: The data node dump restores the values, settings round trip through a data node" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "dump", "tcp" );
        REQUIRE_TRUE( group.Start( 1 ) );

        DRN::CDRNDataGridSettings settings;
        settings.name = "source";
        settings.authority = DRN::DRN_AUTHORITY_PROPOSAL_VOTE;
        settings.reliable = true;
        settings.reliableKeys.push_back( CORE::CVariant( "score" ) );
        settings.sendRateHz = 33;
        settings.maxBytesPerTick = 777;
        settings.deltaAgainstAck = false;
        settings.capacity = 12;
        settings.notify = DRN::DRN_NOTIFY_POLL;
        settings.priority = 2.5f;
        settings.snapshotOnSubscribe = false;
        settings.SetQuantisation( CORE::CVariant( "x" ), DRN::CDRNQuantisation( 2, 0 ) );
        settings.votePolicy.rule = DRN::CDRNVotePolicy::VOTERULE_QUORUM;
        settings.votePolicy.quorumFraction = 0.75f;
        settings.votePolicy.timeoutMs = 1234;
        settings.votePolicy.proposerVoteCounts = true;

        CORE::CDataNode settingsNode;
        ASSERT_TRUE( settings.SaveConfig( settingsNode ) );
        DRN::CDRNDataGridSettings loaded;
        ASSERT_TRUE( loaded.LoadConfig( settingsNode ) );
        ASSERT_TRUE( loaded.name == "source" && loaded.authority == DRN::DRN_AUTHORITY_PROPOSAL_VOTE && loaded.reliable );
        ASSERT_TRUE( loaded.IsReliableKey( CORE::CVariant( "score" ) ) && !loaded.IsReliableKey( CORE::CVariant( "x" ) ) );
        ASSERT_TRUE( loaded.sendRateHz == 33 && loaded.maxBytesPerTick == 777 && !loaded.deltaAgainstAck && loaded.capacity == 12 );
        ASSERT_TRUE( loaded.notify == DRN::DRN_NOTIFY_POLL && loaded.priority == 2.5f && !loaded.snapshotOnSubscribe );
        ASSERT_TRUE( 2 == loaded.GetQuantisation( CORE::CVariant( "x" ) ).floatPrecision && -1 == loaded.GetQuantisation( CORE::CVariant( "y" ) ).floatPrecision );
        ASSERT_TRUE( loaded.votePolicy.rule == DRN::CDRNVotePolicy::VOTERULE_QUORUM && loaded.votePolicy.quorumFraction == 0.75f );
        ASSERT_TRUE( loaded.votePolicy.timeoutMs == 1234 && loaded.votePolicy.proposerVoteCounts );

        DRN::CDRNDataStreamSettings streamSettings;
        streamSettings.name = "log";
        streamSettings.backlogLimit = 17;
        streamSettings.retainedHistory = 9;
        streamSettings.priority = 0.5f;
        streamSettings.notify = DRN::DRN_NOTIFY_PER_CHANGE;
        CORE::CDataNode streamNode;
        ASSERT_TRUE( streamSettings.SaveConfig( streamNode ) );
        DRN::CDRNDataStreamSettings streamLoaded;
        ASSERT_TRUE( streamLoaded.LoadConfig( streamNode ) );
        ASSERT_TRUE( streamLoaded.name == "log" && streamLoaded.backlogLimit == 17 && streamLoaded.retainedHistory == 9 );
        ASSERT_TRUE( streamLoaded.priority == 0.5f && streamLoaded.notify == DRN::DRN_NOTIFY_PER_CHANGE );

        // the grid's dump
        DRN::CDRNDataGridSettings sourceSettings;
        sourceSettings.name = "source";
        DRN::CDRNDataGrid* source = group[ 0 ].CreateGrid( sourceSettings );
        REQUIRE_TRUE( GUCEF_NULL != source );
        source->SetValue( "name", CORE::CVariant( "arena" ) );
        source->SetValue( CORE::CVariant( (Int32) 7 ), CORE::CVariant( (Float64) 2.25 ) );
        source->SetValue( "gone", CORE::CVariant( (Int32) 1 ) );
        source->RemoveKey( "gone" );
        source->SetValue( "name", CORE::CVariant( "arena 2" ) );
        CORE::CDataNode dump;
        CORE::CDataNodeSerializableSettings serializeSettings;
        ASSERT_TRUE( source->Serialize( dump, serializeSettings ) );
        ASSERT_TRUE( GUCEF_NULL != dump.FindChild( "keys" ) && 3 == dump.FindChild( "keys" )->FindChildrenOfType( "entry" ).size() );
        ASSERT_TRUE( 2 == source->GetRevision( source->GetKeyIndex( "name" ) ) );

        DRN::CDRNDataGridSettings copySettings;
        copySettings.name = "copy";
        DRN::CDRNDataGrid* copy = group[ 0 ].CreateGrid( copySettings );
        REQUIRE_TRUE( GUCEF_NULL != copy );
        ASSERT_TRUE( copy->Deserialize( dump, serializeSettings ) );
        CORE::CVariant value;
        ASSERT_TRUE( copy->GetValue( "name", value ) && value.AsString() == "arena 2" );
        ASSERT_TRUE( copy->GetValue( CORE::CVariant( (Int32) 7 ), value ) && value.AsFloat64() == 2.25 );
        ASSERT_FALSE( copy->HasKey( "gone" ) );
        ASSERT_TRUE( 2 == copy->GetKeyCount() );

        // the stream's dump
        DRN::CDRNDataStreamSettings logSettings;
        logSettings.name = "log";
        logSettings.retainedHistory = 4;
        DRN::CDRNDataStream* log = group[ 0 ].CreateStream( logSettings );
        REQUIRE_TRUE( GUCEF_NULL != log );
        for ( UInt32 i=0; i<6; ++i )
            ASSERT_TRUE( AppendNumberedEntry( *log, i, "line" ) );
        PumpFor( 20 );
        CORE::CDataNode logDump;
        ASSERT_TRUE( log->Serialize( logDump, serializeSettings ) );
        ASSERT_TRUE( GUCEF_NULL != logDump.FindChild( "entries" ) && 4 == logDump.FindChild( "entries" )->FindChildrenOfType( "entry" ).size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Streams: Multi-variant entries arrive in order and counted, retained history reaches a late subscriber, the backlog limit holds the sender back" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "streams", "udp" );
        REQUIRE_TRUE( group.Start( 3 ) );

        DRN::CDRNDataStreamSettings settings;
        settings.name = "chat";
        settings.retainedHistory = 5;
        settings.backlogLimit = 8;
        DRN::CDRNDataStream* chat = group[ 1 ].CreateStream( settings );
        REQUIRE_TRUE( GUCEF_NULL != chat );
        for ( UInt32 i=0; i<10; ++i )
            ASSERT_TRUE( AppendNumberedEntry( *chat, i, "old" ) );

        // a late subscriber, through the server: the last 5
        group[ 2 ].Subscribe( "chat" );
        CEntryCountCondition history( group[ 2 ], "chat", 5 );
        REQUIRE_TRUE( PumpUntil( history, 10000 ) );
        DRN::CDRNDataStream* received = group[ 2 ].FindStream( "chat" );
        REQUIRE_TRUE( GUCEF_NULL != received );
        CItemRecorder recorder;
        recorder.SubscribeTo( received );
        PumpFor( 50 );
        ASSERT_TRUE( 5 == received->GetEntryCount() && received->IsSnapshotComplete() );
        for ( UInt32 i=5; i<10; ++i )
        {
            UInt32 number = 0;
            CORE::CString text;
            ASSERT_TRUE( ReadNumberedEntry( *received, number, text ) && number == i && text == "old" );
        }

        // 200 entries, read as they come: every one, in order
        UInt32 appended = 0;
        UInt32 read = 0;
        bool inOrder = true;
        UInt64 start = NowMs();
        while ( read < 200 && NowMs() - start < 20000 )
        {
            while ( appended < 200 && chat->CanAppend() )
            {
                if ( !AppendNumberedEntry( *chat, 100 + appended, "new" ) )
                    break;
                ++appended;
            }
            PumpFor( 2 );
            UInt32 number = 0;
            CORE::CString text;
            while ( ReadNumberedEntry( *received, number, text ) )
            {
                if ( number != 100 + read || text != "new" )
                    inOrder = false;
                ++read;
            }
        }
        ASSERT_TRUE( 200 == read && inOrder );
        ASSERT_TRUE( recorder.entriesEvents > 0 );

        // the subscriber stops reading: the publisher is held back
        UInt32 heldBackAfter = 0;
        for ( UInt32 i=0; i<100 && 0 == heldBackAfter; ++i )
        {
            if ( !chat->CanAppend() )
                heldBackAfter = i;
            else
                AppendNumberedEntry( *chat, 1000 + i, "unread" );
            PumpFor( 5 );
        }
        ASSERT_TRUE( heldBackAfter > 0 && heldBackAfter <= 3 * settings.backlogLimit + 1 );
        ASSERT_TRUE( received->GetEntryCount() <= settings.backlogLimit );

        // reading lets everything through, in order
        UInt32 number = 0;
        CORE::CString text;
        UInt32 unread = 0;
        bool unreadInOrder = true;
        start = NowMs();
        while ( unread < heldBackAfter && NowMs() - start < 10000 )
        {
            PumpFor( 2 );
            while ( ReadNumberedEntry( *received, number, text ) )
            {
                if ( number != 1000 + unread )
                    unreadInOrder = false;
                ++unread;
            }
        }
        ASSERT_TRUE( unread == heldBackAfter && unreadInOrder );
        ASSERT_TRUE( chat->CanAppend() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Authority: proposals accepted and refused, an owner's grid refuses the writes of others" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "authority", "tcp" );
        REQUIRE_TRUE( group.Start( 3 ) );

        DRN::CDRNDataGridSettings rulesSettings;
        rulesSettings.name = "rules";
        rulesSettings.authority = DRN::DRN_AUTHORITY_HOST;
        DRN::CDRNDataGrid* rules = group[ 0 ].CreateGrid( rulesSettings );
        REQUIRE_TRUE( GUCEF_NULL != rules );
        CRefusingValidator validator;
        rules->SetWriteValidator( &validator );
        rules->SetValue( "gravity", CORE::CVariant( (Int32) 10 ) );

        group[ 1 ].Subscribe( "rules" );
        CGridCondition subscribed;
        subscribed.Expect( group[ 1 ], "rules", 1, "gravity", CORE::CVariant( (Int32) 10 ) );
        REQUIRE_TRUE( PumpUntil( subscribed, 10000 ) );
        DRN::CDRNDataGrid* replica = group[ 1 ].FindGrid( "rules" );
        CItemRecorder recorder;
        recorder.SubscribeTo( replica );

        ASSERT_TRUE( replica->SetValue( "speed", CORE::CVariant( (Int32) 5 ) ) );
        ASSERT_TRUE( PumpUntil( recorder.proposeAccepted, 1 ) );
        CGridCondition applied;
        applied.Expect( group[ 0 ], "rules", 1, "speed", CORE::CVariant( (Int32) 5 ) );
        applied.Expect( group[ 1 ], "rules", 1, "speed", CORE::CVariant( (Int32) 5 ) );
        ASSERT_TRUE( PumpUntil( applied, 10000 ) );
        ASSERT_TRUE( 2 == rules->GetLastWriter( rules->GetKeyIndex( "speed" ) ) );

        ASSERT_TRUE( replica->SetValue( "forbidden", CORE::CVariant( (Int32) 1 ) ) );
        ASSERT_TRUE( PumpUntil( recorder.proposeRefused, 1 ) );
        ASSERT_TRUE( recorder.lastRefusal == "not today" );
        ASSERT_FALSE( rules->HasKey( "forbidden" ) );

        // an owner's grid
        DRN::CDRNDataGridSettings mineSettings;
        mineSettings.name = "mine";
        mineSettings.authority = DRN::DRN_AUTHORITY_OWNER;
        DRN::CDRNDataGrid* mine = group[ 1 ].CreateGrid( mineSettings );
        REQUIRE_TRUE( GUCEF_NULL != mine );
        mine->SetValue( "gold", CORE::CVariant( (Int32) 3 ) );
        group[ 2 ].Subscribe( "mine" );
        CGridCondition mineSubscribed;
        mineSubscribed.Expect( group[ 2 ], "mine", 2, "gold", CORE::CVariant( (Int32) 3 ) );
        REQUIRE_TRUE( PumpUntil( mineSubscribed, 10000 ) );
        DRN::CDRNDataGrid* theirs = group[ 2 ].FindGrid( "mine" );
        ASSERT_FALSE( theirs->SetValue( "gold", CORE::CVariant( (Int32) 1000 ) ) );
        DRN::CDRNDataGrid* atServer = group[ 0 ].FindGrid( "mine" );
        ASSERT_TRUE( GUCEF_NULL != atServer && !atServer->SetValue( "gold", CORE::CVariant( (Int32) 1000 ) ) );
        PumpFor( 100 );
        CORE::CVariant gold;
        ASSERT_TRUE( mine->GetValue( "gold", gold ) && 3 == gold.AsInt32() );
        ASSERT_TRUE( theirs->GetValue( "gold", gold ) && 3 == gold.AsInt32() );

        // the owner of a grid with OWNER authority is the only one, a non-sequencer cannot create HOST grids
        DRN::CDRNDataGridSettings notAllowed;
        notAllowed.name = "takeover";
        notAllowed.authority = DRN::DRN_AUTHORITY_HOST;
        ASSERT_TRUE( GUCEF_NULL == group[ 1 ].CreateGrid( notAllowed ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Authority: votes pass, fail and time out, the sequencer's own writes apply directly" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_PEER_TO_PEER, "votes", "tcp" );
        REQUIRE_TRUE( group.Start( 4 ) );

        DRN::CDRNDataGridSettings settings;
        settings.name = "ballot";
        settings.authority = DRN::DRN_AUTHORITY_PROPOSAL_VOTE;
        settings.votePolicy.rule = DRN::CDRNVotePolicy::VOTERULE_MAJORITY;
        settings.votePolicy.timeoutMs = 400;
        DRN::CDRNDataGrid* ballot = group[ 0 ].CreateGrid( settings );
        REQUIRE_TRUE( GUCEF_NULL != ballot );
        ballot->SetValue( "map", CORE::CVariant( "office" ) );

        CItemRecorder recorders[ 4 ];
        CGridCondition subscribed;
        for ( UInt32 i=1; i<4; ++i )
        {
            group[ i ].Subscribe( "ballot" );
            subscribed.Expect( group[ i ], "ballot", 1, "map", CORE::CVariant( "office" ) );
        }
        REQUIRE_TRUE( PumpUntil( subscribed, 10000 ) );
        for ( UInt32 i=1; i<4; ++i )
            recorders[ i ].SubscribeTo( group[ i ].FindGrid( "ballot" ) );
        DRN::CDRNDataGrid* proposer = group[ 1 ].FindGrid( "ballot" );

        // both voters say yes
        recorders[ 2 ].autoVote = CItemRecorder::VOTE_YES;
        recorders[ 3 ].autoVote = CItemRecorder::VOTE_YES;
        ASSERT_TRUE( proposer->SetValue( "map", CORE::CVariant( "dust" ) ) );
        ASSERT_TRUE( PumpUntil( recorders[ 1 ].proposeAccepted, 1 ) );
        ASSERT_TRUE( DRN::CDRNVotePolicy::VOTEOUTCOME_PASSED == recorders[ 1 ].lastProposeOutcome );
        CGridCondition passed;
        for ( UInt32 i=0; i<4; ++i )
            passed.Expect( group[ i ], "ballot", 1, "map", CORE::CVariant( "dust" ) );
        ASSERT_TRUE( PumpUntil( passed, 10000 ) );
        ASSERT_TRUE( 1 == recorders[ 2 ].votesStarted && 1 == recorders[ 3 ].votesStarted && 0 == recorders[ 1 ].votesStarted );
        ASSERT_TRUE( PumpUntil( recorders[ 2 ].votesEnded, 1 ) && DRN::CDRNVotePolicy::VOTEOUTCOME_PASSED == recorders[ 2 ].lastVoteOutcome );

        // both say no
        recorders[ 2 ].autoVote = CItemRecorder::VOTE_NO;
        recorders[ 3 ].autoVote = CItemRecorder::VOTE_NO;
        ASSERT_TRUE( proposer->SetValue( "map", CORE::CVariant( "nuke" ) ) );
        ASSERT_TRUE( PumpUntil( recorders[ 1 ].proposeRefused, 1 ) );
        ASSERT_TRUE( DRN::CDRNVotePolicy::VOTEOUTCOME_FAILED == recorders[ 1 ].lastProposeOutcome );

        // nobody votes
        recorders[ 2 ].autoVote = CItemRecorder::VOTE_NOT;
        recorders[ 3 ].autoVote = CItemRecorder::VOTE_NOT;
        UInt64 proposed = NowMs();
        ASSERT_TRUE( proposer->SetValue( "map", CORE::CVariant( "inferno" ) ) );
        ASSERT_TRUE( PumpUntil( recorders[ 1 ].proposeRefused, 2, 5000 ) );
        ASSERT_TRUE( DRN::CDRNVotePolicy::VOTEOUTCOME_TIMEOUT == recorders[ 1 ].lastProposeOutcome );
        ASSERT_TRUE( NowMs() - proposed >= 350 );
        PumpFor( 50 );
        CORE::CVariant map;
        ASSERT_TRUE( ballot->GetValue( "map", map ) && map.AsString() == "dust" );

        // the boss case
        ASSERT_TRUE( ballot->SetValue( "map", CORE::CVariant( "train" ) ) );
        CGridCondition boss;
        for ( UInt32 i=1; i<4; ++i )
            boss.Expect( group[ i ], "ballot", 1, "map", CORE::CVariant( "train" ) );
        ASSERT_TRUE( PumpUntil( boss, 10000 ) );
        ASSERT_TRUE( 3 == recorders[ 2 ].votesStarted && 3 == recorders[ 3 ].votesStarted );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Grids: A gap in the reliable deltas makes the replica ask for the whole state again, unsubscribing stops the updates" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "resync", "tcp" );
        REQUIRE_TRUE( group.Start( 2 ) );

        DRN::CDRNDataGridSettings settings;
        settings.name = "gappy";
        settings.authority = DRN::DRN_AUTHORITY_OWNER;
        settings.reliable = true;
        DRN::CDRNDataGrid* grid = group[ 0 ].CreateGrid( settings );
        REQUIRE_TRUE( GUCEF_NULL != grid );
        grid->SetValue( "a", CORE::CVariant( (Int32) 1 ) );
        group[ 1 ].Subscribe( "gappy" );
        CGridCondition subscribed;
        subscribed.Expect( group[ 1 ], "gappy", 1, "a", CORE::CVariant( (Int32) 1 ) );
        REQUIRE_TRUE( PumpUntil( subscribed, 10000 ) );
        DRN::CDRNDataGrid* replica = group[ 1 ].FindGrid( "gappy" );
        CItemRecorder recorder;
        recorder.SubscribeTo( replica );
        ASSERT_TRUE( 0 == replica->GetResyncCount() );

        // a reliable delta whose predecessor is not the last one the replica got
        CORE::CDynamicBuffer crafted;
        DRN::CDRNWireWriter writer( crafted );
        writer.WriteVarUInt32( 1 );                         // owner
        writer.WriteVarUInt32( grid->GetItemId() );
        writer.WriteVarUInt32( 0x70000000 );                // sequence: newer than any the subscription started with
        writer.WriteUInt8( 0x05 );                          // reliable, with revisions
        writer.WriteVarUInt32( 12345 );                     // not the previous reliable delta
        writer.WriteVarUInt64( 0 );
        writer.WriteUInt16( 0 );
        DRN::CDRNPeerLinkPtr link = group[ 0 ].GetLinkToMember( 2 );
        REQUIRE_TRUE( !link.IsNULL() );
        ASSERT_TRUE( link->Send( DRN::DRN_LANE_RELIABLE_ORDERED, DRN::DRN_MSGCLASS_REPL, DRN::DRN_REPL_GRID_DELTA, crafted.GetConstBufferPtr(), writer.GetOffset() ) );
        grid->SetValue( "a", CORE::CVariant( (Int32) 2 ) );
        ASSERT_TRUE( PumpUntil( recorder.snapshots, 1, 10000 ) );
        ASSERT_TRUE( 1 == replica->GetResyncCount() );
        CGridCondition resynced;
        resynced.Expect( group[ 1 ], "gappy", 1, "a", CORE::CVariant( (Int32) 2 ) );
        ASSERT_TRUE( PumpUntil( resynced, 10000 ) );
        grid->SetValue( "a", CORE::CVariant( (Int32) 3 ) );
        CGridCondition after;
        after.Expect( group[ 1 ], "gappy", 1, "a", CORE::CVariant( (Int32) 3 ) );
        ASSERT_TRUE( PumpUntil( after, 10000 ) );

        // unsubscribing
        ASSERT_TRUE( 1 == grid->GetSubscriberCount() );
        ASSERT_TRUE( group[ 1 ].Unsubscribe( "gappy" ) );
        bool unsubscribed = false;
        for ( UInt32 i=0; i<1000 && !unsubscribed; ++i )
        {
            PumpFor( 5 );
            unsubscribed = 0 == grid->GetSubscriberCount();
        }
        ASSERT_TRUE( unsubscribed );
        grid->SetValue( "a", CORE::CVariant( (Int32) 4 ) );
        PumpFor( 200 );
        CORE::CVariant value;
        ASSERT_TRUE( replica->GetValue( "a", value ) && 3 == value.AsInt32() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Sessions: The sequencer publishes the latency of every member in a grid" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "measured", "udp" );
        group.sessionSettings.publishLatency = true;
        group.sessionSettings.latencyPublishIntervalMs = 100;
        REQUIRE_TRUE( group.Start( 3 ) );
        group[ 1 ].Subscribe( DRN::CDRNSession::LatencyGridName );
        bool published = false;
        UInt64 start = NowMs();
        while ( !published && NowMs() - start < 10000 )
        {
            PumpFor( 10 );
            DRN::CDRNDataGrid* latency = group[ 1 ].FindGrid( DRN::CDRNSession::LatencyGridName );
            CORE::CVariant rtt2;
            CORE::CVariant rtt3;
            published = GUCEF_NULL != latency && latency->GetValue( CORE::CVariant( (UInt32) 2 ), rtt2 ) && latency->GetValue( CORE::CVariant( (UInt32) 3 ), rtt3 ) &&
                        rtt2.AsUInt32() > 0 && rtt2.AsUInt32() < 100000 && rtt3.AsUInt32() > 0;
        }
        ASSERT_TRUE( published );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Bulk transfers: a blob through the server and a file arrive with their hash checked, a partial file resumes, a wrong part fails" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "bulk", "udp" );
        REQUIRE_TRUE( group.Start( 3 ) );
        CBulkRecorder receiver2;
        CBulkRecorder receiver3;
        receiver2.SubscribeTo( &group[ 1 ].GetBulkTransfers() );
        receiver3.SubscribeTo( &group[ 2 ].GetBulkTransfers() );

        // a blob from one client to the other, through the server, into memory
        CORE::CDynamicBuffer blob;
        FillPattern( blob, 200000, 7 );
        CBulkRecorder sender2;
        DRN::CDRNBulkTransfer* blobOut = group[ 1 ].GetBulkTransfers().OfferData( 3, "blob", blob );
        REQUIRE_TRUE( GUCEF_NULL != blobOut );
        sender2.Watch( blobOut );
        ASSERT_TRUE( PumpUntil( receiver3.completed, 1, 20000 ) );
        ASSERT_TRUE( PumpUntil( sender2.completed, 1, 10000 ) );
        REQUIRE_TRUE( GUCEF_NULL != receiver3.incoming );
        ASSERT_TRUE( SameBytes( receiver3.incoming->GetReceivedData(), blob ) );
        ASSERT_TRUE( receiver3.incoming->GetInfo().name == "blob" && 200000 == receiver3.incoming->GetInfo().size );
        ASSERT_TRUE( 0 == memcmp( receiver3.incoming->GetSha256(), blobOut->GetSha256(), CORE::CSha256::DigestSize ) );
        ASSERT_TRUE( receiver3.progress > 0 && sender2.progress > 0 );

        // a file whose first part is there already: the rest is sent
        CORE::CString workDir = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::CString sourcePath = CORE::CombinePath( workDir, "drn_bulk_source.bin" );
        CORE::CString targetPath = CORE::CombinePath( workDir, "drn_bulk_target.bin" );
        CORE::CDynamicBuffer source;
        FillPattern( source, 100000, 11 );
        REQUIRE_TRUE( WriteTestFile( sourcePath, source, 100000 ) );
        REQUIRE_TRUE( WriteTestFile( targetPath, source, 40000 ) );
        receiver2.answer = CBulkRecorder::ANSWER_FILE;
        receiver2.targetPath = targetPath;
        CBulkRecorder sender1;
        DRN::CDRNBulkTransfer* fileOut = group[ 0 ].GetBulkTransfers().OfferFile( 2, "level.pak", sourcePath );
        REQUIRE_TRUE( GUCEF_NULL != fileOut );
        sender1.Watch( fileOut );
        ASSERT_TRUE( PumpUntil( receiver2.completed, 1, 20000 ) );
        ASSERT_TRUE( PumpUntil( sender1.completed, 1, 10000 ) );
        ASSERT_TRUE( 40000 == fileOut->GetResumeOffset() );
        CORE::CDynamicBuffer arrived;
        ASSERT_TRUE( ReadTestFile( targetPath, arrived ) && SameBytes( arrived, source ) );

        // a first part that is not the start of the file: the hash tells
        CORE::CDynamicBuffer other;
        FillPattern( other, 40000, 99 );
        REQUIRE_TRUE( WriteTestFile( targetPath, other, 40000 ) );
        DRN::CDRNBulkTransfer* wrongOut = group[ 0 ].GetBulkTransfers().OfferFile( 2, "level.pak", sourcePath );
        REQUIRE_TRUE( GUCEF_NULL != wrongOut );
        sender1.Watch( wrongOut );
        ASSERT_TRUE( PumpUntil( receiver2.failed, 1, 20000 ) );
        ASSERT_TRUE( PumpUntil( sender1.failed, 1, 10000 ) );
        ASSERT_TRUE( DRN::CDRNBulkTransfer::TRANSFERSTATE_FAILED == wrongOut->GetState() );

        // a refusal
        receiver3.answer = CBulkRecorder::ANSWER_REJECT;
        CBulkRecorder sender1b;
        DRN::CDRNBulkTransfer* refusedOut = group[ 0 ].GetBulkTransfers().OfferData( 3, "unwanted", blob );
        REQUIRE_TRUE( GUCEF_NULL != refusedOut );
        sender1b.Watch( refusedOut );
        ASSERT_TRUE( PumpUntil( sender1b.failed, 1, 10000 ) );
        ASSERT_TRUE( sender1b.lastFailure.HasSubstr( "not wanted", true ) >= 0 );
        ASSERT_TRUE( group[ 0 ].GetBulkTransfers().Release( refusedOut ) );

        CORE::DeleteFile( sourcePath );
        CORE::DeleteFile( targetPath );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
