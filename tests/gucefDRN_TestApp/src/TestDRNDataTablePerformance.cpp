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

#include <time.h>

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
#include "TestDRNGridBenchmark.h"
#include "TestDRNTableScanThread.h"
#include "TestDRNAllocationCounter.h"
#include "TestDRNDataTablePerformance.h"

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

/** Keys of the grid of the benchmark */
static const UInt32 BenchmarkKeyCount = 100000;

/** Repetitions of every measurement, the median is kept */
static const UInt32 BenchmarkRepetitions = 5;

/** Pairs of every measurement of two grids against each other, the median of the pairs is kept (odd: a middle pair) */
static const UInt32 BenchmarkPairedRounds = 9;

/**
 *  The bounds of the guard.
 *  A table scanning the whole grid without a pause competes for the lock of the node with the operations measured:
 *  that is wall time, the operations wait for the lock. They measured at most about 240 percent of the same
 *  operations without the scan in the runs the bound was set from (the reads of the publisher, which take the lock
 *  themselves, are the worst; the work of the delta stays within about 110 percent). The runs with and without the scan
 *  are measured in pairs right after each other, so both see the same machine; on a machine busy with other work the
 *  scanning thread is also put aside while it holds the lock, which the operations then wait out: the bound grows with
 *  how much the machine stretched the wall time of the runs without the scan beyond their CPU time.
 *  Tables that are attached and idle, and a policy that allows public items while no item is public, cost nothing:
 *  CPU time of the measuring thread (the operations, and the delta's work inside a fixed number of pulses, see
 *  BenchmarkDeltaMinPulses), against runs without measured in pairs with them, the median of the pairs
 *  (CGridBenchmark::MeasurePaired(), MeasurePairedWithLoad()); 97 to 103 percent measured, also on a machine busy with
 *  a parallel build, the bound leaves room for the noise of the caches. The wall times are logged; other processes stretch them by any amount.
 */
static const UInt32 ScanningBoundPercent = 300;
static const UInt32 IdleBoundPercent = 130;

/** Bound of the allocations of a scan of 20000 rows: per batch, never per row */
static const UInt32 ScanAllocationBound = 400;

/*-------------------------------------------------------------------------*/

/** Percent of the wall times of a result compared to a reference, the worst of the operations, for the log */
static UInt32
WorstPercent( const CGridBenchmarkResult& result, const CGridBenchmarkResult& reference )
{
    UInt32 worst = 0;
    const UInt64 numbers[ 6 ][ 2 ] = { { result.setValueNs, reference.setValueNs }, { result.setValueByIndexNs, reference.setValueByIndexNs },
                                       { result.getValueByIndexNs, reference.getValueByIndexNs }, { result.replicaGetValueByIndexNs, reference.replicaGetValueByIndexNs },
                                       { result.deltaNsPerKey, reference.deltaNsPerKey }, { result.deltaWorkNsPerKey, reference.deltaWorkNsPerKey } };
    for ( UInt32 i=0; i<6; ++i )
    {
        const UInt32 percent = numbers[ i ][ 1 ] > 0 ? (UInt32) ( numbers[ i ][ 0 ] * 100 / numbers[ i ][ 1 ] ) : 0;
        if ( percent > worst )
            worst = percent;
    }
    return worst;
}

/*-------------------------------------------------------------------------*/

/**
 *  Tables attached to both grids of a benchmark and idle (nobody scans, nobody listens) for the measured loop: made
 *  before it, let go of after it, so that runs with and without them are measured in pairs
 */
class CIdleTablesLoad : public CGridBenchmarkLoad
{
    public:

    CIdleTablesLoad( CGridBenchmark& benchmark, const DRN::CDRNDataGridTableSettings& replicaSettings )
        : CGridBenchmarkLoad()
        , observedByGrid( 0 )
        , m_benchmark( benchmark )
        , m_replicaSettings( replicaSettings )
        , m_publisherTable( GUCEF_NULL )
        , m_replicaTable( GUCEF_NULL )
    {}

    virtual ~CIdleTablesLoad()
    {
        End();
    }

    virtual void Begin( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_publisherTable = GUCEF_NEW DRN::CDRNDataGridTable( m_benchmark.GetPublisherSession(), *m_benchmark.GetPublisherGrid() );
        m_replicaTable = GUCEF_NEW DRN::CDRNDataGridTable( m_benchmark.GetReplicaSession(), *m_benchmark.GetReplicaGrid(), m_replicaSettings );
    }

    virtual void End( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        // an idle table has nothing at the grid
        if ( GUCEF_NULL != m_publisherTable )
            observedByGrid += m_benchmark.GetPublisherGrid()->GetSubscriptionCountForObserver( &m_publisherTable->AsObserver() );
        if ( GUCEF_NULL != m_replicaTable )
            observedByGrid += m_benchmark.GetReplicaGrid()->GetSubscriptionCountForObserver( &m_replicaTable->AsObserver() );
        GUCEF_DELETE m_publisherTable;
        m_publisherTable = GUCEF_NULL;
        GUCEF_DELETE m_replicaTable;
        m_replicaTable = GUCEF_NULL;
    }

    UInt32 observedByGrid;          /**< subscriptions of the tables at their grids, over all runs: 0 */

    private:

    CGridBenchmark& m_benchmark;
    DRN::CDRNDataGridTableSettings m_replicaSettings;
    DRN::CDRNDataGridTable* m_publisherTable;
    DRN::CDRNDataGridTable* m_replicaTable;
};

/*-------------------------------------------------------------------------*/

/** Grid settings of a hot path grid, optionally with declared types */
static DRN::CDRNDataGridSettings
HotGrid( const CORE::CString& name, bool declareTypes )
{
    DRN::CDRNDataGridSettings settings;
    settings.name = name;
    settings.authority = DRN::DRN_AUTHORITY_OWNER;
    settings.sendRateHz = 0;
    settings.defaultQuantisation = DRN::CDRNQuantisation( 3, 0 );
    if ( declareTypes )
    {
        settings.keyType = CORE::DATATABLE_TYPE_UINT32;
        settings.valueType = CORE::DATATABLE_TYPE_FLOAT32;
    }
    return settings;
}

/*-------------------------------------------------------------------------*/

/**
 *  The allocation free check of the hot path of the grid (see the replication tests), with tables attached to the
 *  grid on both members when asked to. The tables are idle: nobody listens to them.
 */
static bool
HotPathAllocations( bool withTables, UInt32& allocations, UInt32& updates, UInt32& subscriptionsAtGrid, bool allowPublicItems = false )
{
    CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, withTables ? "no-alloc-table" : "no-alloc-plain", "mem" );
    group.nodeSettings.securityPolicy.allowPublicItems = allowPublicItems;
    if ( !group.Start( 2 ) )
        return false;

    DRN::CDRNDataGrid* grid = group[ 0 ].CreateGrid( HotGrid( "hot", withTables ) );
    if ( GUCEF_NULL == grid )
        return false;
    GUCEF::vector< UInt32 > indices;
    for ( UInt32 i=0; i<16; ++i )
    {
        indices.push_back( grid->GetOrAddKeyIndex( CORE::CVariant( i ) ) );
        grid->SetValueByIndex( indices.back(), CORE::CVariant( (Float32) 0 ) );
    }
    group[ 1 ].Subscribe( "hot" );
    CGridCondition subscribed;
    subscribed.Expect( group[ 1 ], "hot", 1, CORE::CVariant( (UInt32) 15 ), CORE::CVariant( (Float32) 0 ) );
    if ( !PumpUntil( subscribed, 10000 ) )
        return false;
    DRN::CDRNDataGrid* replica = group[ 1 ].FindGrid( "hot" );
    replica->SetNotifyMode( DRN::DRN_NOTIFY_PER_PACKET );
    CItemRecorder recorder;
    recorder.SubscribeTo( replica );

    DRN::CDRNDataGridTableSettings replicaTableSettings;
    replicaTableSettings.keyType = CORE::DATATABLE_TYPE_UINT32;
    replicaTableSettings.valueType = CORE::DATATABLE_TYPE_FLOAT32;
    DRN::CDRNDataGridTable* publisherTable = GUCEF_NULL;
    DRN::CDRNDataGridTable* replicaTable = GUCEF_NULL;
    if ( withTables )
    {
        publisherTable = GUCEF_NEW DRN::CDRNDataGridTable( group[ 0 ], *grid );
        replicaTable = GUCEF_NEW DRN::CDRNDataGridTable( group[ 1 ], *replica, replicaTableSettings );
    }

    CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    allocations = 0;
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
    updates = recorder.updates;
    subscriptionsAtGrid = 0;
    if ( withTables )
    {
        subscriptionsAtGrid = grid->GetSubscriptionCountForObserver( &publisherTable->AsObserver() ) + replica->GetSubscriptionCountForObserver( &replicaTable->AsObserver() );
        GUCEF_DELETE publisherTable;
        GUCEF_DELETE replicaTable;
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDRNDataTablePerformanceTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefDRN.DataTablePerformance" );

    GUCEF_TESTFW_TESTCASE( "Performance: SetValue, GetValueByIndex and delta encoding on a 100,000 key grid without a table" )
    try
    {
        CGridBenchmark benchmark;
        REQUIRE_TRUE( benchmark.Setup( BenchmarkKeyCount, "bench-baseline" ) );
        CGridBenchmarkResult result;
        REQUIRE_TRUE( benchmark.Measure( GUCEF_NULL, BenchmarkRepetitions, result ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Grid benchmark without a table: " << result.ToString() );
        ASSERT_TRUE( result.setValueNs > 0 && result.setValueByIndexNs > 0 && result.getValueByIndexNs > 0 && result.deltaNsPerKey > 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Performance: With a table attached to the grid on both members the hot path still does not allocate, and the grid has no observer of the table" )
    try
    {
        UInt32 plainAllocations = 0;
        UInt32 plainUpdates = 0;
        UInt32 plainSubscriptions = 0;
        UInt32 tableAllocations = 0;
        UInt32 tableUpdates = 0;
        UInt32 tableSubscriptions = 0;
        REQUIRE_TRUE( HotPathAllocations( false, plainAllocations, plainUpdates, plainSubscriptions ) );
        REQUIRE_TRUE( HotPathAllocations( true, tableAllocations, tableUpdates, tableSubscriptions ) );
        ASSERT_TRUE( plainUpdates > 100 && tableUpdates > 100 );
        ASSERT_TRUE( 0 == tableSubscriptions );
        if ( IsAllocationCountingAvailable() )
        {
            ASSERT_TRUE( 0 == plainAllocations );
            ASSERT_TRUE( 0 == tableAllocations );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Grid hot path allocations in 200 frames: " << plainAllocations << " without tables, " << tableAllocations << " with a table on each member" );
            if ( 0 != tableAllocations )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Grid hot path allocations with tables:\n" << GetCountedAllocationStacks() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    // the values were still handed over, only the allocation count could not be checked
    GUCEF_TESTFW_SKIP_IF( !IsAllocationCountingAvailable(), GetAllocationCountingUnavailableReason() );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Performance: A scan allocates per batch and not per row" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "scan-alloc", "mem" );
        REQUIRE_TRUE( group.Start( 1 ) );
        DRN::CDRNDataGridSettings settings = HotGrid( "rows", true );
        settings.valueType = CORE::DATATABLE_TYPE_UINT32;
        settings.capacity = 40000;
        DRN::CDRNDataGrid* grid = group[ 0 ].CreateGrid( settings );
        REQUIRE_TRUE( GUCEF_NULL != grid );
        DRN::CDRNDataGridTable table( group[ 0 ], *grid );

        const UInt32 rows = 20000;
        UInt32 allocationsFor[ 2 ] = { 0, 0 };
        UInt32 rowsFor[ 2 ] = { rows, 2 * rows };
        for ( UInt32 round=0; round<2; ++round )
        {
            for ( UInt32 i=0; i<rowsFor[ round ]; ++i )
                grid->SetValue( CORE::CVariant( i ), CORE::CVariant( i ) );
            for ( UInt32 pass=0; pass<2; ++pass )
            {
                // the first pass warms the pool and the vectors up
                if ( 1 == pass )
                    StartAllocationCounting();
                CORE::CDataTableScanPtr scan;
                REQUIRE_TRUE( CORE::DATATABLE_STATUS_OK == table.Scan( CORE::CDataTableQuery(), scan ) );
                UInt32 seen = 0;
                CORE::CDataTableBatchPtr batch;
                while ( CORE::DATATABLE_STATUS_OK == scan->NextBatch( batch ) )
                    seen += batch->GetSelectedRowCount();
                scan.Unlink();
                batch.Reset();
                if ( 1 == pass )
                    allocationsFor[ round ] = StopAllocationCounting();
                ASSERT_TRUE( rowsFor[ round ] == seen );
            }
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Scan allocations: " << allocationsFor[ 0 ] << " for " << rowsFor[ 0 ] << " rows, " << allocationsFor[ 1 ] << " for " << rowsFor[ 1 ] << " rows, bound " << ScanAllocationBound );
        if ( IsAllocationCountingAvailable() )
        {
            ASSERT_TRUE( allocationsFor[ 0 ] <= ScanAllocationBound && allocationsFor[ 1 ] <= ScanAllocationBound );
            // twice the rows, not twice the allocations
            ASSERT_TRUE( allocationsFor[ 1 ] <= 2 * allocationsFor[ 0 ] + 64 );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_SKIP_IF( !IsAllocationCountingAvailable(), GetAllocationCountingUnavailableReason() );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Performance: A reliable 20,000 key grid snapshotted to a subscriber over tcp: messages, packets, bytes and time" )
    try
    {
        // the message budget of the replicator on a stream channel decides how many messages a snapshot takes
        static const UInt32 KeyCount = 20000;
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "tcp-snapshot", "tcp" );
        REQUIRE_TRUE( group.Start( 2 ) );
        DRN::CDRNDataGridSettings settings;
        settings.name = "pages";
        settings.reliable = true;
        settings.snapshotOnSubscribe = true;
        DRN::CDRNDataGrid* grid = group[ 0 ].CreateGrid( settings );
        REQUIRE_TRUE( GUCEF_NULL != grid );
        CORE::CString value60( "0123456789012345678901234567890123456789012345678901234567" );    // 58 characters, a 60 byte value
        for ( UInt32 k=0; k<KeyCount; ++k )
            grid->SetValue( CORE::CVariant( k ), CORE::CVariant( value60 ) );

        DRN::CDRNPeerLinkPtr link = group[ 0 ].GetLinkToMember( group[ 1 ].GetLocalPeerId() );
        REQUIRE_TRUE( !link.IsNULL() );
        DRN::CDRNLinkStats before = link->GetStats();
        UInt64 start = DRN::CDRNClock::NowInMicroseconds();
        group[ 1 ].Subscribe( "pages" );
        CGridCondition arrived;
        arrived.Expect( group[ 1 ], "pages", 1, CORE::CVariant( (UInt32) ( KeyCount - 1 ) ), CORE::CVariant( value60 ) );
        REQUIRE_TRUE( PumpUntil( arrived, 60000 ) );
        UInt64 elapsedUs = DRN::CDRNClock::NowInMicroseconds() - start;
        DRN::CDRNLinkStats after = link->GetStats();
        DRN::CDRNDataGrid* replica = group[ 1 ].FindGrid( "pages" );
        REQUIRE_TRUE( GUCEF_NULL != replica );
        ASSERT_TRUE( KeyCount == replica->GetKeyCount() );

        UInt64 messages = after.messagesSent - before.messagesSent;
        UInt64 packets = after.packetsSent - before.packetsSent;
        UInt64 bytes = after.bytesSent - before.bytesSent;
        UInt32 messageBudget = group[ 0 ].GetReplicator().GetMessageBudget( group[ 1 ].GetLocalPeerId() );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Snapshot of " << KeyCount << " keys of 60 bytes over tcp: " << messages << " messages, " << packets << " packets, " << bytes << " bytes, "
                   << ( elapsedUs / 1000 ) << " ms, message budget " << messageBudget << " bytes" );

        // on a stream channel the replicator's cap of 16 KB holds, not a datagram budget: a few hundred keys per message
        ASSERT_TRUE( 16 * 1024 == messageBudget );
        ASSERT_TRUE( messages <= KeyCount / 100 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Performance: A policy that allows public items costs nothing while no item is public: the grid benchmark and the allocation free hot path" )
    try
    {
        // both grids exist side by side and every operation is measured on both in pairs, right after each other: the
        // pairs see the same machine, a machine busy with other work slows both runs of a pair alike, and the median of
        // the pairs leaves out the pairs that a burst of other work fell in. The grids are idle while the other one is
        // measured: their pulses on the shared pulse generator are a small, equal part of the work of both
        CGridBenchmarkResult sealed;
        CGridBenchmarkResult allowing;
        UInt32 percent = 0;
        {
            CGridBenchmark sealedBenchmark;
            REQUIRE_TRUE( sealedBenchmark.Setup( BenchmarkKeyCount, "bench-sealed" ) );
            CGridBenchmark allowingBenchmark;
            REQUIRE_TRUE( allowingBenchmark.Setup( BenchmarkKeyCount, "bench-public-allowed", false, true ) );
            REQUIRE_TRUE( CGridBenchmark::MeasurePaired( sealedBenchmark, allowingBenchmark, BenchmarkPairedRounds, sealed, allowing, percent ) );
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Grid benchmark, allowPublicItems false: " << sealed.ToString() );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Grid benchmark, allowPublicItems true:  " << allowing.ToString() );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Worst operation with public items allowed against the sealed run of the same pairs: " << percent << "% of CPU time, median of "
                   << BenchmarkPairedRounds << " pairs (bound " << IdleBoundPercent << "%), " << WorstPercent( allowing, sealed ) << "% of wall time of the medians" );
        if ( !IsSanitizerBuild() )
            ASSERT_TRUE( percent <= IdleBoundPercent );

        UInt32 allocations = 0;
        UInt32 updates = 0;
        UInt32 subscriptions = 0;
        REQUIRE_TRUE( HotPathAllocations( false, allocations, updates, subscriptions, true ) );
        ASSERT_TRUE( updates > 100 );
        if ( IsAllocationCountingAvailable() )
        {
            ASSERT_TRUE( 0 == allocations );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Grid hot path allocations in 200 frames with public items allowed: " << allocations );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Performance: A reliable 100,000 key grid of 60 byte values snapshotted to four subscribers: CPU and packets per MB, CONFIDENTIAL against PUBLIC mac and none, udp and tcp" )
    try
    {
        // a sanitizer build is many times slower: a fifth of the keys there, the figures of such a run say nothing anyway
        const UInt32 KeyCount = IsSanitizerBuild() ? 20000 : 100000;
        const char* channels[] = { "mem", "tcp" };
        const char* modeNames[] = { "CONFIDENTIAL", "PUBLIC mac", "PUBLIC none" };
        CORE::CString value60( "0123456789012345678901234567890123456789012345678901234567" );
        for ( UInt32 c=0; c<2; ++c )
        {
            for ( UInt32 mode=0; mode<3; ++mode )
            {
                CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "public-snapshot", channels[ c ] );
                group.nodeSettings.securityPolicy.allowPublicItems = 0 != mode;
                group.nodeSettings.securityPolicy.publicItemProtection = 2 == mode ? DRN::DRN_PUBLICPROTECTION_NONE : DRN::DRN_PUBLICPROTECTION_MAC;
                REQUIRE_TRUE( group.Start( 5 ) );
                DRN::CDRNDataGridSettings settings;
                settings.name = "quotes";
                settings.reliable = true;
                settings.capacity = KeyCount;
                settings.protection = 0 != mode ? DRN::DRN_PROTECTION_PUBLIC : DRN::DRN_PROTECTION_CONFIDENTIAL;
                DRN::CDRNDataGrid* grid = group[ 0 ].CreateGrid( settings );
                REQUIRE_TRUE( GUCEF_NULL != grid );
                for ( UInt32 k=0; k<KeyCount; ++k )
                    grid->SetValue( CORE::CVariant( k ), CORE::CVariant( value60 ) );

                UInt64 packetsBefore = 0;
                UInt64 bytesBefore = 0;
                for ( UInt32 m=1; m<5; ++m )
                {
                    DRN::CDRNPeerLinkPtr link = group[ 0 ].GetLinkToMember( group[ m ].GetLocalPeerId() );
                    REQUIRE_TRUE( !link.IsNULL() );
                    packetsBefore += link->GetStats().packetsSent;
                    bytesBefore += link->GetStats().bytesSent;
                }
                clock_t cpuStart = clock();
                UInt64 start = DRN::CDRNClock::NowInMicroseconds();
                CGridCondition arrived;
                for ( UInt32 m=1; m<5; ++m )
                {
                    group[ m ].Subscribe( "quotes" );
                    arrived.Expect( group[ m ], "quotes", group[ 0 ].GetLocalPeerId(), CORE::CVariant( (UInt32) ( KeyCount - 1 ) ), CORE::CVariant( value60 ) );
                }
                REQUIRE_TRUE( PumpUntil( arrived, 120000 ) );
                UInt64 elapsedUs = DRN::CDRNClock::NowInMicroseconds() - start;
                Float64 cpuMs = (Float64) ( clock() - cpuStart ) * 1000.0 / CLOCKS_PER_SEC;
                UInt64 packets = 0;
                UInt64 bytes = 0;
                for ( UInt32 m=1; m<5; ++m )
                {
                    DRN::CDRNPeerLinkPtr link = group[ 0 ].GetLinkToMember( group[ m ].GetLocalPeerId() );
                    packets += link->GetStats().packetsSent;
                    bytes += link->GetStats().bytesSent;
                }
                packets -= packetsBefore;
                bytes -= bytesBefore;
                Float64 megabytes = (Float64) bytes / ( 1024.0 * 1024.0 );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Public snapshot over " << channels[ c ] << ", " << modeNames[ mode ] << ": " << bytes << " bytes, " << packets << " packets, " << ( elapsedUs / 1000 ) <<
                           " ms, process CPU " << cpuMs << " ms, " << ( megabytes > 0 ? cpuMs / megabytes : 0.0 ) << " ms CPU per MB, " << ( megabytes > 0 ? packets / megabytes : 0.0 ) << " packets per MB" );
                for ( UInt32 m=1; m<5; ++m )
                {
                    DRN::CDRNDataGrid* replica = group[ m ].FindGrid( "quotes" );
                    ASSERT_TRUE( GUCEF_NULL != replica && KeyCount == replica->GetKeyCount() && settings.protection == replica->GetProtectionClass() );
                }
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Performance: Declared types change nothing of what the grid sends" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "declared", "mem" );
        REQUIRE_TRUE( group.Start( 2 ) );
        DRN::CDRNDataGrid* typed = group[ 0 ].CreateGrid( HotGrid( "grid-a", true ) );
        DRN::CDRNDataGrid* plain = group[ 0 ].CreateGrid( HotGrid( "grid-b", false ) );
        REQUIRE_TRUE( GUCEF_NULL != typed && GUCEF_NULL != plain );
        for ( UInt32 round=1; round<=20; ++round )
        {
            for ( UInt32 k=0; k<200; ++k )
            {
                typed->SetValue( CORE::CVariant( k ), CORE::CVariant( (Float32) ( round + k * 0.25f ) ) );
                plain->SetValue( CORE::CVariant( k ), CORE::CVariant( (Float32) ( round + k * 0.25f ) ) );
            }
        }
        group[ 1 ].Subscribe( "grid-a" );
        group[ 1 ].Subscribe( "grid-b" );
        CGridCondition arrived;
        arrived.Expect( group[ 1 ], "grid-a", 1, CORE::CVariant( (UInt32) 199 ), CORE::CVariant( (Float32) ( 20 + 199 * 0.25f ) ) );
        arrived.Expect( group[ 1 ], "grid-b", 1, CORE::CVariant( (UInt32) 199 ), CORE::CVariant( (Float32) ( 20 + 199 * 0.25f ) ) );
        REQUIRE_TRUE( PumpUntil( arrived, 10000 ) );
        PumpFor( 50 );
        ASSERT_TRUE( typed->GetBytesSent() > 0 );
        ASSERT_TRUE( typed->GetBytesSent() == plain->GetBytesSent() && typed->GetDeltasSent() == plain->GetDeltasSent() );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Bytes sent for the same writes: " << typed->GetBytesSent() << " with declared types, " << plain->GetBytesSent() << " without" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Performance: SetValue, GetValueByIndex and delta encoding on a 100,000 key grid stay within the recorded bound with a table attached and with a table scanning in the background" )
    try
    {
        CGridBenchmark benchmark;
        REQUIRE_TRUE( benchmark.Setup( BenchmarkKeyCount, "bench-tables", true ) );
        DRN::CDRNDataGridTableSettings replicaSettings;
        replicaSettings.keyType = CORE::DATATABLE_TYPE_UINT32;
        replicaSettings.valueType = CORE::DATATABLE_TYPE_UINT32;
        CGridBenchmarkResult none;
        CGridBenchmarkResult idle;
        CGridBenchmarkResult scanReference;
        CGridBenchmarkResult scanning;
        UInt32 idlePercent = 0;
        UInt32 scanningPercent = 0;
        UInt32 busyPercent = 100;

        // tables attached and idle (nobody scans, nobody listens) against no table, every operation in pairs right after
        // each other: a machine busy with other work slows both runs of a pair alike, the median of the pairs is kept
        {
            CIdleTablesLoad idleTables( benchmark, replicaSettings );
            REQUIRE_TRUE( benchmark.MeasurePairedWithLoad( idleTables, BenchmarkPairedRounds, none, idle, idlePercent ) );
            ASSERT_TRUE( 0 == idleTables.observedByGrid );
        }

        {
            DRN::CDRNDataGridTable publisherTable( benchmark.GetPublisherSession(), *benchmark.GetPublisherGrid() );
            DRN::CDRNDataGridTable replicaTable( benchmark.GetReplicaSession(), *benchmark.GetReplicaGrid(), replicaSettings );

            // a thread scans both tables over and over next to the measured loops, each run paired with one without
            CTableScanThread scanner( false );
            scanner.AddTable( &publisherTable );
            scanner.AddTable( &replicaTable );
            REQUIRE_TRUE( benchmark.MeasureAgainstReference( scanner, BenchmarkRepetitions, scanReference, scanning, scanningPercent, busyPercent ) );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "The background scans: " << scanner.scans.Load() << " scans, " << scanner.rowsSeen.Load() << " rows, " << scanner.failures.Load() << " failed" );
            ASSERT_TRUE( 0 == scanner.failures.Load() );
            ASSERT_TRUE( scanner.scans.Load() > 0 );
        }

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Grid benchmark, no table, paired:        " << none.ToString() );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Grid benchmark, tables attached, idle:   " << idle.ToString() );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Grid benchmark, tables idle, paired:     " << scanReference.ToString() );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Grid benchmark, tables scanning, paired: " << scanning.ToString() );

        // idle tables against the runs without a table they were paired with, in CPU time; the scan against the runs it
        // was paired with, in wall time, its bound as far beyond the recorded one as the machine was busy
        const UInt32 scanningBound = (UInt32) ( (UInt64) ScanningBoundPercent * busyPercent / 100 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Worst operation against the paired runs without a table: " << idlePercent << "% of CPU time with idle tables, median of " <<
                   BenchmarkPairedRounds << " pairs (bound " << IdleBoundPercent << "%, " << WorstPercent( idle, none ) << "% of wall time of the medians); " << scanningPercent << "% of wall time while scanning against the paired runs (bound " <<
                   scanningBound << "%: " << ScanningBoundPercent << "% at " << busyPercent << "% wall of CPU time)" );

        // the numbers of a build with a sanitizer say nothing about the code
        if ( !IsSanitizerBuild() )
        {
            ASSERT_TRUE( idlePercent <= IdleBoundPercent );
            ASSERT_TRUE( scanningPercent <= scanningBound );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
