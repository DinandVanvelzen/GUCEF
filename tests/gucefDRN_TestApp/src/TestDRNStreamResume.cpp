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

#ifndef GUCEF_DRN_H
#include "gucefDRN.h"
#define GUCEF_DRN_H
#endif /* GUCEF_DRN_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestDRNTestUtils.h"
#include "TestDRNGroup.h"
#include "TestDRNAllocationCounter.h"
#include "TestDRNStreamResume.h"

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

namespace {

/*-------------------------------------------------------------------------*/

UInt64
NowMs( void )
{
    return DRN::CDRNClock::NowInMicroseconds() / 1000;
}

/*-------------------------------------------------------------------------*/

/** Writes a stream entry carrying its number, gives the sequence it got */
bool
AppendNumbered( DRN::CDRNDataStream& stream, UInt32 number, UInt64* seq = GUCEF_NULL )
{
    CORE::CVariantStream& entry = stream.GetWriteStream();
    entry << number << CORE::CString( "entry" );
    entry.WriteSegmentEnd();
    UInt64 entrySeq = 0;
    if ( stream.CommitEntry( entrySeq ) )
    {
        if ( GUCEF_NULL != seq )
            *seq = entrySeq;
        return true;
    }
    entry.Clear();
    return false;
}

/*-------------------------------------------------------------------------*/

/** Appends the entries numbered first to last, pumping while the backlog holds the publisher back */
bool
AppendRange( DRN::CDRNDataStream& stream, UInt32 first, UInt32 last )
{
    UInt64 start = NowMs();
    UInt32 number = first;
    while ( number <= last && NowMs() - start < 10000 )
    {
        if ( stream.CanAppend() && AppendNumbered( stream, number ) )
            ++number;
        else
            PumpFor( 2 );
    }
    return number > last;
}

/*-------------------------------------------------------------------------*/

class CReadEntry
{
    public:
    UInt64 seq;
    UInt32 number;
    CReadEntry( void ) : seq( 0 ), number( 0 ) {}
};
typedef GUCEF::vector< CReadEntry > TReadEntryVector;

/*-------------------------------------------------------------------------*/

/** Reads what arrives, with the sequences, until count entries were read or the time is up */
void
ReadEntries( DRN::CDRNDataStream& stream, UInt32 count, TReadEntryVector& entries, UInt32 timeoutInMs = 10000 )
{
    UInt64 start = NowMs();
    while ( entries.size() < count && NowMs() - start < timeoutInMs )
    {
        PumpFor( 2 );
        CORE::CVariantStream segment;
        CReadEntry read;
        while ( stream.ReadNextEntry( segment, read.seq ) )
        {
            CORE::CString text;
            segment >> read.number >> text;
            entries.push_back( read );
        }
    }
}

/*-------------------------------------------------------------------------*/

/** Every entry read carries its number as its sequence, starting at firstSeq */
bool
IsRun( const TReadEntryVector& entries, UInt32 from, UInt32 count, UInt64 firstSeq )
{
    if ( from + count > entries.size() )
        return false;
    for ( UInt32 i=0; i<count; ++i )
    {
        const CReadEntry& entry = entries[ from + i ];
        if ( entry.seq != firstSeq + i || entry.number != firstSeq + i )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Records the gaps of a stream, and the order of the gap and entry events ('G', 'E') */
class CGapRecorder : public CORE::CObserver
{
    public:

    GUCEF::vector< DRN::CDRNStreamGapInfo > gaps;
    CORE::CString order;

    CGapRecorder( void ) : CORE::CObserver(), gaps(), order() {}
    virtual ~CGapRecorder() { SignalUpcomingObserverDestruction(); }

    bool HasOnly( UInt64 fromSeq, UInt64 toSeq, DRN::TDRNStreamGapReason reason ) const
    {
        if ( 1 == gaps.size() && gaps[ 0 ].fromSeq == fromSeq && gaps[ 0 ].toSeq == toSeq && gaps[ 0 ].reason == reason )
            return true;
        for ( UInt32 i=0; i<gaps.size(); ++i )
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Gap recorded: " << gaps[ i ].fromSeq << " to " << gaps[ i ].toSeq << " " << DRN::CDRNStreamGapInfo::ReasonToString( gaps[ i ].reason ) );
        return false;
    }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( DRN::CDRNDataStream::GapEvent == eventid && GUCEF_NULL != eventdata )
        {
            gaps.push_back( static_cast< DRN::CDRNStreamGapEventData* >( eventdata )->GetData() );
            order += 'G';
        }
        else
        if ( DRN::CDRNDataStream::EntriesAvailableEvent == eventid )
        {
            order += 'E';
        }
    }
};

/*-------------------------------------------------------------------------*/

class CEntryCountReached : public CTestCondition
{
    public:
    DRN::CDRNDataStream& m_stream;
    UInt32 m_count;
    CEntryCountReached( DRN::CDRNDataStream& stream, UInt32 count ) : m_stream( stream ), m_count( count ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return m_stream.GetEntryCount() >= m_count; }
};

/*-------------------------------------------------------------------------*/

class CStreamAnnounced : public CTestCondition
{
    public:
    DRN::CDRNSession& m_session;
    CORE::CString m_name;
    CStreamAnnounced( DRN::CDRNSession& session, const CORE::CString& name ) : m_session( session ), m_name( name ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL != m_session.FindStream( m_name ); }
};

/*-------------------------------------------------------------------------*/

class CStreamFound : public CTestCondition
{
    public:
    DRN::CDRNSession& m_session;
    CORE::CString m_name;
    CStreamFound( DRN::CDRNSession& session, const CORE::CString& name ) : m_session( session ), m_name( name ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        DRN::CDRNDataStream* stream = m_session.FindStream( m_name );
        return GUCEF_NULL != stream && stream->IsSnapshotComplete();
    }
};

/*-------------------------------------------------------------------------*/

/** A member on a node of its own that joins the session of a group, and can leave again */
class CJoiner
{
    public:

    DRN::CDRNNode node;
    DRN::CDRNSession* session;

    CJoiner( void ) : node(), session( GUCEF_NULL ) {}
    ~CJoiner() { node.Stop(); }

    class CJoined : public CTestCondition
    {
        public:
        DRN::CDRNSession& m_session;
        explicit CJoined( DRN::CDRNSession& session ) : m_session( session ) {}
        virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return m_session.IsActive() && 0 != m_session.GetLocalPeerId(); }
    };

    bool Join( CGroup& group )
    {
        if ( !node.SetSettings( group.nodeSettings ) )
            return false;
        session = node.CreateSession();
        DRN::CDRNSessionSettings joinSettings;
        joinSettings.sessionName = group.sessionSettings.sessionName;
        if ( GUCEF_NULL == session || !session->Join( DRN::CDRNEndpointList( group.endpoints[ 0 ] ), joinSettings ) )
            return false;
        CJoined joined( *session );
        return PumpUntil( joined, 15000 );
    }

    void Leave( void )
    {
        node.Stop();
        PumpFor( 50 );
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  Test hook: hands the replica of a serving member a STREAM ENTRIES message as if it came from its upstream,
 *  with the numbered entries firstNumber to firstNumber + count - 1 at the sequence firstSeq.
 */
bool
InjectEntries( DRN::CDRNSession& server, UInt32 upstreamPeerId, const DRN::CDRNDataStream& replica, UInt64 firstSeq, UInt32 firstNumber, UInt16 count )
{
    MT::CScopeMutex lock( server.GetNode().GetLock() );
    DRN::CDRNPeerLinkPtr link = server.GetLinkToMember( upstreamPeerId );
    if ( link.IsNULL() )
        return false;

    DRN::CDRNLinkMessage message;
    message.lane = DRN::DRN_LANE_RELIABLE_ORDERED;
    message.msgClass = DRN::DRN_MSGCLASS_STREAM;
    message.opcode = DRN::DRN_STREAM_ENTRIES;
    DRN::CDRNWireWriter writer( message.payload );
    writer.WriteVarUInt32( replica.GetOwnerPeerId() );
    writer.WriteVarUInt32( replica.GetItemId() );
    writer.WriteVarUInt64( firstSeq );
    writer.WriteUInt16( count );
    for ( UInt32 i=0; i<count; ++i )
    {
        CORE::CVariantStream entry;
        entry << (UInt32) ( firstNumber + i ) << CORE::CString( "entry" );
        entry.WriteSegmentEnd();
        writer.WriteSizedBytes( entry.GetBuffer().GetConstBufferPtr(), entry.GetDataSize() );
    }
    server.GetReplicator().OnSessionMessage( server, *link, upstreamPeerId, message );
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  CLIENT_SERVER with three members: the client publishes, the third member subscribes through the server.
 *  After 'before' entries the server's replica gets a jump of 40 in its receive; returns what the third member reads.
 */
CORE::CString
RunServingReplicaJump( UInt32 before, TReadEntryVector& entries, CGapRecorder& gaps )
{
    CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "jump", "mem" );
    if ( !group.Start( 3 ) )
        return "start";

    DRN::CDRNDataStreamSettings settings;
    settings.name = "feed";
    settings.backlogLimit = 8;
    settings.retainedHistory = 8;
    DRN::CDRNDataStream* feed = group[ 1 ].CreateStream( settings );
    if ( GUCEF_NULL == feed )
        return "create";
    group[ 2 ].Subscribe( "feed" );
    CStreamFound found( group[ 2 ], "feed" );
    if ( !PumpUntil( found, 10000 ) )
        return "subscribe";
    DRN::CDRNDataStream* received = group[ 2 ].FindStream( "feed" );
    gaps.SubscribeTo( received );
    if ( !AppendRange( *feed, 1, before ) )
        return "append";
    ReadEntries( *received, before, entries );
    if ( entries.size() != before )
        return "before";
    gaps.order.Clear();

    DRN::CDRNDataStream* serving = group[ 0 ].FindStream( "feed" );
    if ( GUCEF_NULL == serving )
        return "serving replica";
    UInt64 jumpSeq = before + 1 + 40;
    if ( !InjectEntries( group[ 0 ], group[ 1 ].GetLocalPeerId(), *serving, jumpSeq, (UInt32) jumpSeq, 3 ) )
        return "inject";
    // not read until the gap was reported, so that the order of the events shows
    UInt64 start = NowMs();
    while ( gaps.gaps.empty() && NowMs() - start < 3000 )
        PumpFor( 2 );
    PumpFor( 20 );
    ReadEntries( *received, before + 3, entries, 3000 );
    PumpFor( 20 );
    gaps.UnsubscribeAllFromObserver();
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

/**
 *  Resume after the subscriber's session ended: the publisher (on the server, or on a client and served by the server)
 *  has 10 entries read by the first subscriber, then 'more' entries are appended, then a new session subscribes with
 *  fromSeq 11.
 */
CORE::CString
RunResume( bool throughServer, UInt32 more, TReadEntryVector& entries, CGapRecorder& gaps, UInt64& serverFirstRetained, UInt64& serverNext )
{
    CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, throughServer ? "resume-served" : "resume", "mem" );
    if ( !group.Start( throughServer ? 2 : 1 ) )
        return "start";
    DRN::CDRNSession& publisherSession = throughServer ? group[ 1 ] : group[ 0 ];

    DRN::CDRNDataStreamSettings settings;
    settings.name = "events";
    settings.backlogLimit = 8;
    settings.retainedHistory = 8;
    DRN::CDRNDataStream* events = publisherSession.CreateStream( settings );
    if ( GUCEF_NULL == events )
        return "create";
    UInt64 epoch = events->GetStreamEpoch();

    UInt64 lastRead = 0;
    {
        CJoiner first;
        if ( !first.Join( group ) )
            return "first join";
        first.session->Subscribe( "events" );
        CStreamFound found( *first.session, "events" );
        if ( !PumpUntil( found, 10000 ) )
            return "first subscribe";
        if ( !AppendRange( *events, 1, 10 ) )
            return "append 1";
        TReadEntryVector firstEntries;
        ReadEntries( *first.session->FindStream( "events" ), 10, firstEntries );
        if ( 10 != firstEntries.size() || !IsRun( firstEntries, 0, 10, 1 ) )
            return "first read";
        lastRead = firstEntries.back().seq;
        if ( first.session->FindStream( "events" )->GetStreamEpoch() != epoch )
            return "epoch";
        first.Leave();
    }

    // the subscriber is gone: nothing holds the publisher back
    PumpFor( 100 );
    if ( !AppendRange( *events, 11, 10 + more ) )
        return "append 2";
    PumpFor( 50 );

    CJoiner second;
    if ( !second.Join( group ) )
        return "second join";
    second.session->Subscribe( "events", 0, DRN::CDRNSubscribeOptions( lastRead + 1, epoch ) );
    CStreamAnnounced announced( *second.session, "events" );
    if ( !PumpUntil( announced, 10000 ) )
        return "second announce";
    DRN::CDRNDataStream* received = second.session->FindStream( "events" );
    gaps.SubscribeTo( received );
    CStreamFound found( *second.session, "events" );
    if ( !PumpUntil( found, 10000 ) )
        return "second subscribe";
    UInt32 expected = more > 8 ? 8 : more;
    ReadEntries( *received, expected, entries );
    PumpFor( 50 );
    ReadEntries( *received, expected + 1, entries, 50 );
    gaps.UnsubscribeAllFromObserver();

    if ( throughServer )
    {
        DRN::CDRNDataStream* serving = group[ 0 ].FindStream( "events" );
        if ( GUCEF_NULL == serving )
            return "serving replica";
        serverFirstRetained = serving->GetFirstRetainedSeq();
        serverNext = serving->GetNextSeq();
    }
    else
    {
        serverFirstRetained = events->GetFirstRetainedSeq();
        serverNext = events->GetNextSeq();
    }
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDRNStreamResumeTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING DRN STREAM RESUME TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefDRN.StreamResume" );

    GUCEF_TESTFW_TESTCASE( "Streams: Every entry has a sequence: AppendEntry gives it, ReadNextEntry and PeekEntry return it, firstSeq sets the start" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "sequences", "mem" );
        REQUIRE_TRUE( group.Start( 2 ) );

        DRN::CDRNDataStreamSettings settings;
        settings.name = "plain";
        DRN::CDRNDataStream* plain = group[ 0 ].CreateStream( settings );
        settings.name = "late";
        settings.firstSeq = 1000;
        DRN::CDRNDataStream* late = group[ 0 ].CreateStream( settings );
        REQUIRE_TRUE( GUCEF_NULL != plain && GUCEF_NULL != late );
        ASSERT_TRUE( 0 != plain->GetStreamEpoch() && 0 != late->GetStreamEpoch() && plain->GetStreamEpoch() != late->GetStreamEpoch() );
        ASSERT_TRUE( 1 == plain->GetFirstRetainedSeq() && 1 == plain->GetNextSeq() );
        ASSERT_TRUE( 1000 == late->GetNextSeq() );

        group[ 1 ].Subscribe( "plain" );
        group[ 1 ].Subscribe( "late" );
        CStreamFound plainFound( group[ 1 ], "plain" );
        CStreamFound lateFound( group[ 1 ], "late" );
        REQUIRE_TRUE( PumpUntil( plainFound, 10000 ) && PumpUntil( lateFound, 10000 ) );
        DRN::CDRNDataStream* plainReplica = group[ 1 ].FindStream( "plain" );
        DRN::CDRNDataStream* lateReplica = group[ 1 ].FindStream( "late" );
        ASSERT_TRUE( plainReplica->GetStreamEpoch() == plain->GetStreamEpoch() );

        for ( UInt32 i=1; i<=3; ++i )
        {
            UInt64 seq = 0;
            ASSERT_TRUE( AppendNumbered( *plain, i, &seq ) && seq == i );
            ASSERT_TRUE( AppendNumbered( *late, 999 + i, &seq ) && seq == 999 + i );
        }
        ASSERT_TRUE( 4 == plain->GetNextSeq() && 1003 == late->GetNextSeq() );

        CEntryCountReached plainCount( *plainReplica, 3 );
        CEntryCountReached lateCount( *lateReplica, 3 );
        REQUIRE_TRUE( PumpUntil( plainCount, 10000 ) && PumpUntil( lateCount, 10000 ) );
        CORE::CVariantStream segment;
        UInt64 seq = 0;
        ASSERT_TRUE( plainReplica->PeekEntry( 2, segment, seq ) && 3 == seq );
        TReadEntryVector plainEntries;
        TReadEntryVector lateEntries;
        ReadEntries( *plainReplica, 3, plainEntries );
        ReadEntries( *lateReplica, 3, lateEntries );
        ASSERT_TRUE( IsRun( plainEntries, 0, 3, 1 ) );
        ASSERT_TRUE( IsRun( lateEntries, 0, 3, 1000 ) );
        ASSERT_TRUE( 4 == plainReplica->GetNextExpectedSeq() && 1003 == lateReplica->GetNextExpectedSeq() );
        ASSERT_TRUE( 0 == plainReplica->GetGapCount() && 0 == lateReplica->GetGapCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Streams: Behind a serving replica whose receive jumps, the subscriber gets the true entries, never stale ring slots, and a TRIMMED gap" )
    try
    {
        TReadEntryVector entries;
        CGapRecorder gaps;
        CORE::CString failed = RunServingReplicaJump( 20, entries, gaps );
        ASSERT_TRUE( failed.IsNULLOrEmpty() );
        REQUIRE_TRUE( entries.size() >= 20 );
        ASSERT_TRUE( IsRun( entries, 0, 20, 1 ) );
        // after the jump: entries 61 to 63 with their true sequences, nothing else
        ASSERT_TRUE( 23 == entries.size() );
        ASSERT_TRUE( IsRun( entries, 20, 3, 61 ) );
        ASSERT_TRUE( gaps.HasOnly( 21, 60, DRN::DRN_STREAMGAP_TRIMMED ) );
        // the gap event comes before the entries event of the same tick
        ASSERT_TRUE( gaps.order.HasSubstr( "GE", true ) >= 0 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Serving replica jump: event order after the jump \"" << gaps.order << "\"" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Streams: Behind a serving replica whose receive jumps early, no ring slot that was never filled is sent" )
    try
    {
        TReadEntryVector entries;
        CGapRecorder gaps;
        CORE::CString failed = RunServingReplicaJump( 5, entries, gaps );
        ASSERT_TRUE( failed.IsNULLOrEmpty() );
        REQUIRE_TRUE( entries.size() >= 5 );
        ASSERT_TRUE( 8 == entries.size() );
        ASSERT_TRUE( IsRun( entries, 5, 3, 46 ) );
        ASSERT_TRUE( gaps.HasOnly( 6, 45, DRN::DRN_STREAMGAP_TRIMMED ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Streams: A new session that subscribes with fromSeq gets exactly the entries after the last one read, once, without a gap" )
    try
    {
        TReadEntryVector entries;
        CGapRecorder gaps;
        UInt64 firstRetained = 0;
        UInt64 next = 0;
        CORE::CString failed = RunResume( false, 5, entries, gaps, firstRetained, next );
        ASSERT_TRUE( failed.IsNULLOrEmpty() );
        ASSERT_TRUE( 5 == entries.size() && IsRun( entries, 0, 5, 11 ) );
        ASSERT_TRUE( gaps.gaps.empty() );
        ASSERT_TRUE( 16 == next && 8 == firstRetained );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Streams: Resuming beyond the retention raises a BEYOND_RETENTION gap with the exact range, then the retained entries" )
    try
    {
        TReadEntryVector entries;
        CGapRecorder gaps;
        UInt64 firstRetained = 0;
        UInt64 next = 0;
        CORE::CString failed = RunResume( false, 30, entries, gaps, firstRetained, next );
        ASSERT_TRUE( failed.IsNULLOrEmpty() );
        ASSERT_TRUE( 33 == firstRetained && 41 == next );
        ASSERT_TRUE( gaps.HasOnly( 11, 32, DRN::DRN_STREAMGAP_BEYOND_RETENTION ) );
        ASSERT_TRUE( 8 == entries.size() && IsRun( entries, 0, 8, 33 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Streams: A resubscription after the publisher trimmed raises the gap instead of moving the subscriber on silently" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "silent", "mem" );
        REQUIRE_TRUE( group.Start( 2 ) );
        DRN::CDRNDataStreamSettings settings;
        settings.name = "events";
        settings.backlogLimit = 8;
        settings.retainedHistory = 8;
        DRN::CDRNDataStream* events = group[ 0 ].CreateStream( settings );
        REQUIRE_TRUE( GUCEF_NULL != events );
        group[ 1 ].Subscribe( "events" );
        CStreamFound found( group[ 1 ], "events" );
        REQUIRE_TRUE( PumpUntil( found, 10000 ) );
        DRN::CDRNDataStream* received = group[ 1 ].FindStream( "events" );
        CGapRecorder gaps;
        gaps.SubscribeTo( received );
        REQUIRE_TRUE( AppendRange( *events, 1, 10 ) );
        TReadEntryVector entries;
        ReadEntries( *received, 10, entries );
        REQUIRE_TRUE( 10 == entries.size() );

        // away while the publisher goes on: it keeps the last 8 of 40
        group[ 1 ].Unsubscribe( "events" );
        PumpFor( 100 );
        REQUIRE_TRUE( AppendRange( *events, 11, 40 ) );
        group[ 1 ].Subscribe( "events" );
        ReadEntries( *received, 18, entries );
        REQUIRE_TRUE( 18 == entries.size() );

        // the application is told that 11 to 32 are gone, and the entries carry their sequences. Polled entries can be
        // read within the tick at whose end the gap is told: the event may come a pulse after the last entry was read
        UInt64 waitStart = NowMs();
        while ( gaps.gaps.empty() && NowMs() - waitStart < 2000 )
            PumpFor( 2 );
        ASSERT_TRUE( gaps.HasOnly( 11, 32, DRN::DRN_STREAMGAP_BEYOND_RETENTION ) );
        ASSERT_TRUE( IsRun( entries, 10, 8, 33 ) );
        ASSERT_TRUE( 1 == received->GetGapCount() && 22 == received->GetMissedEntryCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Streams: Resuming a recreated publisher reports EPOCH_CHANGED; with persisted firstSeq and epoch the resume continues without a gap" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "epoch", "mem" );
        REQUIRE_TRUE( group.Start( 1 ) );
        DRN::CDRNDataStreamSettings settings;
        settings.name = "log";
        settings.backlogLimit = 8;
        settings.retainedHistory = 16;
        DRN::CDRNDataStream* log = group[ 0 ].CreateStream( settings );
        REQUIRE_TRUE( GUCEF_NULL != log );
        UInt64 oldEpoch = log->GetStreamEpoch();
        REQUIRE_TRUE( AppendRange( *log, 1, 10 ) );
        UInt64 persistedNext = log->GetNextSeq();

        // a new publisher instance without persistence: another epoch, numbering from 1
        REQUIRE_TRUE( group[ 0 ].GetReplicator().RemoveItem( log ) );
        DRN::CDRNDataStream* restarted = group[ 0 ].CreateStream( settings );
        REQUIRE_TRUE( GUCEF_NULL != restarted );
        ASSERT_TRUE( restarted->GetStreamEpoch() != oldEpoch );
        REQUIRE_TRUE( AppendRange( *restarted, 1, 4 ) );
        {
            CJoiner joiner;
            REQUIRE_TRUE( joiner.Join( group ) );
            CGapRecorder gaps;
            joiner.session->Subscribe( "log", 0, DRN::CDRNSubscribeOptions( 8, oldEpoch ) );
            CStreamFound found( *joiner.session, "log" );
            REQUIRE_TRUE( PumpUntil( found, 10000 ) );
            DRN::CDRNDataStream* received = joiner.session->FindStream( "log" );
            gaps.SubscribeTo( received );
            TReadEntryVector entries;
            ReadEntries( *received, 4, entries );
            PumpFor( 20 );
            // reported, and received from the retained history of the new epoch
            ASSERT_TRUE( 1 == received->GetGapCount() );
            ASSERT_TRUE( 4 == entries.size() && IsRun( entries, 0, 4, 1 ) );
            joiner.Leave();
        }

        // a new publisher instance that persisted its numbering continues it
        REQUIRE_TRUE( group[ 0 ].GetReplicator().RemoveItem( restarted ) );
        settings.firstSeq = persistedNext;
        settings.epoch = oldEpoch;
        DRN::CDRNDataStream* continued = group[ 0 ].CreateStream( settings );
        REQUIRE_TRUE( GUCEF_NULL != continued );
        ASSERT_TRUE( continued->GetStreamEpoch() == oldEpoch && continued->GetNextSeq() == 11 );
        REQUIRE_TRUE( AppendRange( *continued, 11, 14 ) );
        {
            CJoiner joiner;
            REQUIRE_TRUE( joiner.Join( group ) );
            CGapRecorder gaps;
            joiner.session->Subscribe( "log", 0, DRN::CDRNSubscribeOptions( 11, oldEpoch ) );
            CStreamFound found( *joiner.session, "log" );
            REQUIRE_TRUE( PumpUntil( found, 10000 ) );
            DRN::CDRNDataStream* received = joiner.session->FindStream( "log" );
            gaps.SubscribeTo( received );
            TReadEntryVector entries;
            ReadEntries( *received, 4, entries );
            PumpFor( 20 );
            ASSERT_TRUE( 0 == received->GetGapCount() && gaps.gaps.empty() );
            ASSERT_TRUE( 4 == entries.size() && IsRun( entries, 0, 4, 11 ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Streams: Through a serving replica, resuming gives the same results, and the server's replica answers the range queries" )
    try
    {
        TReadEntryVector entries;
        CGapRecorder gaps;
        UInt64 firstRetained = 0;
        UInt64 next = 0;
        CORE::CString failed = RunResume( true, 5, entries, gaps, firstRetained, next );
        ASSERT_TRUE( failed.IsNULLOrEmpty() );
        ASSERT_TRUE( 5 == entries.size() && IsRun( entries, 0, 5, 11 ) );
        ASSERT_TRUE( gaps.gaps.empty() );
        ASSERT_TRUE( 16 == next && firstRetained >= 1 && firstRetained <= 11 );

        TReadEntryVector beyond;
        CGapRecorder beyondGaps;
        failed = RunResume( true, 30, beyond, beyondGaps, firstRetained, next );
        ASSERT_TRUE( failed.IsNULLOrEmpty() );
        ASSERT_TRUE( beyondGaps.HasOnly( 11, 32, DRN::DRN_STREAMGAP_BEYOND_RETENTION ) );
        ASSERT_TRUE( 8 == beyond.size() && IsRun( beyond, 0, 8, 33 ) );
        ASSERT_TRUE( 41 == next && 33 == firstRetained );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Streams: A subscription with fromSeq made before the stream is announced resumes from there" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "early", "mem" );
        REQUIRE_TRUE( group.Start( 2 ) );
        ASSERT_TRUE( group[ 1 ].Subscribe( "later", 0, DRN::CDRNSubscribeOptions( 11, 0 ) ) );
        PumpFor( 50 );

        DRN::CDRNDataStreamSettings settings;
        settings.name = "later";
        settings.backlogLimit = 32;
        settings.retainedHistory = 32;
        DRN::CDRNDataStream* later = group[ 0 ].CreateStream( settings );
        REQUIRE_TRUE( GUCEF_NULL != later );
        for ( UInt32 i=1; i<=15; ++i )
            REQUIRE_TRUE( AppendNumbered( *later, i ) );

        CStreamFound found( group[ 1 ], "later" );
        REQUIRE_TRUE( PumpUntil( found, 10000 ) );
        DRN::CDRNDataStream* received = group[ 1 ].FindStream( "later" );
        TReadEntryVector entries;
        ReadEntries( *received, 5, entries );
        PumpFor( 50 );
        ReadEntries( *received, 6, entries, 50 );
        ASSERT_TRUE( 5 == entries.size() && IsRun( entries, 0, 5, 11 ) );
        ASSERT_TRUE( 0 == received->GetGapCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Streams: SetResumeFromSeq makes a subscribed replica ask again from the given sequence" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "rewind", "mem" );
        REQUIRE_TRUE( group.Start( 2 ) );
        DRN::CDRNDataStreamSettings settings;
        settings.name = "tape";
        settings.backlogLimit = 32;
        settings.retainedHistory = 32;
        DRN::CDRNDataStream* tape = group[ 0 ].CreateStream( settings );
        REQUIRE_TRUE( GUCEF_NULL != tape );
        group[ 1 ].Subscribe( "tape" );
        CStreamFound found( group[ 1 ], "tape" );
        REQUIRE_TRUE( PumpUntil( found, 10000 ) );
        DRN::CDRNDataStream* received = group[ 1 ].FindStream( "tape" );
        REQUIRE_TRUE( AppendRange( *tape, 1, 12 ) );
        TReadEntryVector entries;
        ReadEntries( *received, 12, entries );
        REQUIRE_TRUE( 12 == entries.size() );

        received->SetResumeFromSeq( 5 );
        TReadEntryVector again;
        ReadEntries( *received, 8, again );
        PumpFor( 50 );
        ReadEntries( *received, 9, again, 50 );
        ASSERT_TRUE( 8 == again.size() && IsRun( again, 0, 8, 5 ) );
        ASSERT_TRUE( 13 == received->GetNextExpectedSeq() && 0 == received->GetGapCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Streams: Sequences on the wire: 32 bit encodings decode identically as 64 bit, values above 2^32 round trip" )
    try
    {
        // what a node of today writes (32 bit varints) is what the 64 bit reader reads, byte for byte
        static const UInt32 values32[] = { 0, 1, 127, 128, 300, 16383, 16384, 65535, 1000000, 0x7FFFFFFF, 0xFFFFFFFF };
        for ( UInt32 i=0; i<sizeof( values32 ) / sizeof( values32[ 0 ] ); ++i )
        {
            CORE::CDynamicBuffer old;
            DRN::CDRNWireWriter oldWriter( old );
            oldWriter.WriteVarUInt32( values32[ i ] );
            CORE::CDynamicBuffer wide;
            DRN::CDRNWireWriter wideWriter( wide );
            wideWriter.WriteVarUInt64( values32[ i ] );
            ASSERT_TRUE( oldWriter.GetOffset() == wideWriter.GetOffset() );
            ASSERT_TRUE( 0 == memcmp( old.GetConstBufferPtr(), wide.GetConstBufferPtr(), oldWriter.GetOffset() ) );
            DRN::CDRNWireReader reader( old.GetConstBufferPtr(), oldWriter.GetOffset() );
            UInt64 value = 0;
            ASSERT_TRUE( reader.ReadVarUInt64( value ) && value == values32[ i ] );
        }
        static const UInt64 values64[] = { 0x100000000ULL, 0x100000005ULL, 0x123456789ABCDEFULL, 0xFFFFFFFFFFFFFFFFULL };
        for ( UInt32 i=0; i<sizeof( values64 ) / sizeof( values64[ 0 ] ); ++i )
        {
            CORE::CDynamicBuffer buffer;
            DRN::CDRNWireWriter writer( buffer );
            writer.WriteVarUInt64( values64[ i ] );
            DRN::CDRNWireReader reader( buffer.GetConstBufferPtr(), writer.GetOffset() );
            UInt64 value = 0;
            ASSERT_TRUE( reader.ReadVarUInt64( value ) && value == values64[ i ] );
        }

        // a stream whose numbering is past 2^32 replicates with its true sequences
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "wide", "mem" );
        REQUIRE_TRUE( group.Start( 2 ) );
        DRN::CDRNDataStreamSettings settings;
        settings.name = "wide";
        settings.firstSeq = 0x100000000ULL - 2;
        DRN::CDRNDataStream* wide = group[ 0 ].CreateStream( settings );
        REQUIRE_TRUE( GUCEF_NULL != wide );
        group[ 1 ].Subscribe( "wide" );
        CStreamFound found( group[ 1 ], "wide" );
        REQUIRE_TRUE( PumpUntil( found, 10000 ) );
        DRN::CDRNDataStream* received = group[ 1 ].FindStream( "wide" );
        for ( UInt32 i=0; i<5; ++i )
            REQUIRE_TRUE( AppendNumbered( *wide, i ) );
        TReadEntryVector entries;
        ReadEntries( *received, 5, entries );
        REQUIRE_TRUE( 5 == entries.size() );
        for ( UInt32 i=0; i<5; ++i )
            ASSERT_TRUE( entries[ i ].seq == settings.firstSeq + i && entries[ i ].number == i );
        ASSERT_TRUE( received->GetNextExpectedSeq() == settings.firstSeq + 5 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Streams: A steady stream with sequences on every entry does not allocate after warm-up" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "stream-no-alloc", "mem" );
        REQUIRE_TRUE( group.Start( 2 ) );
        DRN::CDRNDataStreamSettings settings;
        settings.name = "steady";
        settings.backlogLimit = 64;
        settings.retainedHistory = 16;
        DRN::CDRNDataStream* steady = group[ 0 ].CreateStream( settings );
        REQUIRE_TRUE( GUCEF_NULL != steady );
        group[ 1 ].Subscribe( "steady" );
        CStreamFound found( group[ 1 ], "steady" );
        REQUIRE_TRUE( PumpUntil( found, 10000 ) );
        DRN::CDRNDataStream* received = group[ 1 ].FindStream( "steady" );

        CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        CORE::CVariantStream segment;
        segment << (UInt32) 0 << (Float32) 0;
        segment.WriteSegmentEnd();
        UInt32 allocations = 0;
        UInt64 lastSeq = 0;
        bool inOrder = true;
        UInt32 read = 0;
        for ( UInt32 round=0; round<7; ++round )
        {
            // the first rounds warm up: rings, queues and buffers grow to their size; the link's record of each
            // packet sent keeps its capacity once its ring (DRN_RECEIVE_WINDOW_SIZE packets) went round once
            if ( 6 == round )
                StartAllocationCounting();
            for ( UInt32 frame=0; frame<200; ++frame )
            {
                UInt64 seq = 0;
                steady->AppendEntry( segment, seq );
                for ( UInt32 p=0; p<2; ++p )
                {
                    pulseGenerator->ForceDirectPulse();
                    MT::ThreadDelay( 1 );
                }
                UInt64 readSeq = 0;
                while ( received->ReadNextEntry( segment, readSeq ) )
                {
                    if ( 0 != lastSeq && readSeq != lastSeq + 1 )
                        inOrder = false;
                    lastSeq = readSeq;
                    ++read;
                }
            }
            if ( 6 == round )
                allocations = StopAllocationCounting();
        }
        ASSERT_TRUE( read > 1200 && inOrder );
        if ( IsAllocationCountingAvailable() )
        {
            ASSERT_TRUE( 0 == allocations );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Stream hot path: " << allocations << " allocations in 200 frames, " << read << " entries read" );
            if ( 0 != allocations )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Stream hot path allocations:\n" << GetCountedAllocationStacks() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_SKIP_IF( !IsAllocationCountingAvailable(), GetAllocationCountingUnavailableReason() );
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
