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

/**
 *  The bounds of the guard, in percent of the slower of two runs without a table. A table scanning the whole grid
 *  without a pause competes for the lock of the node with the operations measured: they measured at most about 240
 *  percent in the runs the bound was set from (the reads of the publisher, which take the lock themselves, are the
 *  worst; the delta encoding stays within about 110 percent). Tables that are attached and idle measured the same as
 *  no table (97 to 100 percent), the bound only leaves room for the noise of the machine.
 */
static const UInt32 ScanningBoundPercent = 300;
static const UInt32 IdleBoundPercent = 130;

/** Bound of the allocations of a scan of 20000 rows: per batch, never per row */
static const UInt32 ScanAllocationBound = 400;

/*-------------------------------------------------------------------------*/

/** Percent of the numbers of a result compared to a reference, the worst of the operations */
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
HotPathAllocations( bool withTables, UInt32& allocations, UInt32& updates, UInt32& subscriptionsAtGrid )
{
    CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, withTables ? "no-alloc-table" : "no-alloc-plain", "mem" );
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
        CGridBenchmarkResult scanning;
        CGridBenchmarkResult noneAgain;

        // no table: the run the others are held to
        REQUIRE_TRUE( benchmark.Measure( GUCEF_NULL, BenchmarkRepetitions, none ) );

        {
            // tables attached and idle: nobody scans, nobody listens
            DRN::CDRNDataGridTable publisherTable( benchmark.GetPublisherSession(), *benchmark.GetPublisherGrid() );
            DRN::CDRNDataGridTable replicaTable( benchmark.GetReplicaSession(), *benchmark.GetReplicaGrid(), replicaSettings );
            REQUIRE_TRUE( benchmark.Measure( GUCEF_NULL, BenchmarkRepetitions, idle ) );
            ASSERT_TRUE( 0 == benchmark.GetPublisherGrid()->GetSubscriptionCountForObserver( &publisherTable.AsObserver() ) );
            ASSERT_TRUE( 0 == benchmark.GetReplicaGrid()->GetSubscriptionCountForObserver( &replicaTable.AsObserver() ) );

            // a thread scans both tables over and over next to the measured loops
            CTableScanThread scanner( false );
            scanner.AddTable( &publisherTable );
            scanner.AddTable( &replicaTable );
            REQUIRE_TRUE( benchmark.Measure( &scanner, BenchmarkRepetitions, scanning ) );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "The background scans: " << scanner.scans.Load() << " scans, " << scanner.rowsSeen.Load() << " rows, " << scanner.failures.Load() << " failed" );
            ASSERT_TRUE( 0 == scanner.failures.Load() );
            ASSERT_TRUE( scanner.scans.Load() > 0 );
        }

        // the run without a table once more, to see how far the machine drifted between the runs
        REQUIRE_TRUE( benchmark.Measure( GUCEF_NULL, BenchmarkRepetitions, noneAgain ) );

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Grid benchmark, no table:                " << none.ToString() );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Grid benchmark, tables attached, idle:   " << idle.ToString() );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Grid benchmark, tables scanning:         " << scanning.ToString() );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Grid benchmark, no table, once more:     " << noneAgain.ToString() );

        // the reference is the slower of the two runs without a table
        CGridBenchmarkResult reference = none;
        if ( noneAgain.setValueNs > reference.setValueNs ) reference.setValueNs = noneAgain.setValueNs;
        if ( noneAgain.setValueByIndexNs > reference.setValueByIndexNs ) reference.setValueByIndexNs = noneAgain.setValueByIndexNs;
        if ( noneAgain.getValueByIndexNs > reference.getValueByIndexNs ) reference.getValueByIndexNs = noneAgain.getValueByIndexNs;
        if ( noneAgain.replicaGetValueByIndexNs > reference.replicaGetValueByIndexNs ) reference.replicaGetValueByIndexNs = noneAgain.replicaGetValueByIndexNs;
        if ( noneAgain.deltaNsPerKey > reference.deltaNsPerKey ) reference.deltaNsPerKey = noneAgain.deltaNsPerKey;
        if ( noneAgain.deltaWorkNsPerKey > reference.deltaWorkNsPerKey ) reference.deltaWorkNsPerKey = noneAgain.deltaWorkNsPerKey;
        const UInt32 idlePercent = WorstPercent( idle, reference );
        const UInt32 scanningPercent = WorstPercent( scanning, reference );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Worst operation against the slower run without a table: " << idlePercent << "% with idle tables (bound " << IdleBoundPercent << "%), " << scanningPercent << "% while scanning (bound " << ScanningBoundPercent << "%)" );

        // the numbers of a build with a sanitizer say nothing about the code
        if ( !IsSanitizerBuild() )
        {
            ASSERT_TRUE( idlePercent <= IdleBoundPercent );
            ASSERT_TRUE( scanningPercent <= ScanningBoundPercent );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
