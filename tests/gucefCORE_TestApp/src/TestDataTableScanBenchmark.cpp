/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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
#include <stdlib.h>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CMEMORYDATATABLE_H
#include "gucefCORE_CMemoryDataTable.h"
#define GUCEF_CORE_CMEMORYDATATABLE_H
#endif /* GUCEF_CORE_CMEMORYDATATABLE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAllocationCounter.h"
#include "TestDataTableScanSupport.h"
#include "TestDataTableScanBenchmark.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::CORE;

static const UInt32 BenchmarkSymbols = 500;

/*-------------------------------------------------------------------------*/

/** What a pass computed, so the work cannot be optimized away and passes can be compared */
struct SBenchmarkResult
{
    UInt64 rows;
    Float64 closeSum;
    UInt64 tsXor;
    UInt64 symbolBytes;
    UInt32 histogram[ BenchmarkSymbols ];
    Float64 seconds;
    UInt32 allocations;
    UInt64 poolMisses;
};

static void
ClearResult( SBenchmarkResult& result )
{
    memset( &result, 0, sizeof( result ) );
}

/** A row group of rows: ts increasing from firstTs, close a random walk, symbol dictionary indices into the shared dictionary */
static CDataTableBatchPtr
MakeBenchmarkRowGroup( const CDataTableSchemaPtr& schema, CDataTableBufferPool& pool, const CDataTableBatchPtr& dictionary,
                       CORETEST::CTestRandom& random, Int64 firstTs, UInt32 rowCount, Float64& close )
{
    CDataTableBufferPtr ts;
    CDataTableBufferPtr closes;
    CDataTableBufferPtr indices;
    pool.Acquire( rowCount * 8, ts );
    pool.Acquire( rowCount * 8, closes );
    pool.Acquire( rowCount * 4, indices );
    Int64* tsValues = static_cast< Int64* >( ts->GetData() );
    Float64* closeValues = static_cast< Float64* >( closes->GetData() );
    UInt32* indexValues = static_cast< UInt32* >( indices->GetData() );
    for ( UInt32 i=0; i<rowCount; ++i )
    {
        tsValues[ i ] = firstTs + (Int64) i * 60000000LL;
        close += ( (Float64) (Int32) random.Below( 2001 ) - 1000.0 ) * 0.0001;
        closeValues[ i ] = close;
        indexValues[ i ] = random.Below( BenchmarkSymbols );
    }
    ts->SetDataSize( rowCount * 8 );
    closes->SetDataSize( rowCount * 8 );
    indices->SetDataSize( rowCount * 4 );

    CDataTableBatchPtr batch;
    pool.AcquireBatch( batch );
    CDataTableBatch* writable = batch.GetWritable();
    writable->SetColumnCount( 3 );
    writable->SetSchema( schema );
    writable->SetRowCount( rowCount );
    for ( UInt32 c=0; c<3; ++c )
    {
        writable->GetMutableColumn( c ).SetType( schema->GetColumn( c ) );
        writable->SetSchemaColumnIndex( c, c );
    }
    writable->GetMutableColumn( 0 ).SetFixedValues( rowCount, tsValues, ts );
    writable->GetMutableColumn( 1 ).SetFixedValues( rowCount, closeValues, closes );
    writable->GetMutableColumn( 2 ).SetDictionaryValues( rowCount, indexValues, indices, dictionary, 0 );
    return batch;
}

/** Tier 2: typed spans, the dictionary indices into a histogram */
static TDataTableStatus
ScanSpans( CDataTableScan& scan, SBenchmarkResult& result, bool countAllocations, UInt64 expectedRows )
{
    CDataTableBatchPtr batch;
    TDataTableStatus status = scan.NextBatch( batch );
    if ( countAllocations )
        CORETEST::StartAllocationCounting();
    const UInt64 start = MT::PrecisionTickCount();
    while ( DATATABLE_STATUS_OK == status )
    {
        const CDataTableBatch& rows = *batch;
        const UInt32 count = rows.GetRowCount();
        const Int64* ts = rows.GetColumn( 0 ).GetInt64Data();
        const Float64* close = rows.GetColumn( 1 ).GetFloat64Data();
        const UInt32* symbols = rows.GetColumn( 2 ).GetDictionaryIndices();
        Float64 sum = 0.0;
        UInt64 tsXor = 0;
        for ( UInt32 i=0; i<count; ++i )
        {
            sum += close[ i ];
            tsXor ^= (UInt64) ts[ i ];
            ++result.histogram[ symbols[ i ] ];
        }
        result.closeSum += sum;
        result.tsXor ^= tsXor;
        result.rows += count;
        if ( countAllocations && result.rows == expectedRows )
        {
            // the call that ends the scan raises ScanFinishedEvent, mailed with a copy of its data: once per scan
            result.allocations = CORETEST::StopAllocationCounting();
            countAllocations = false;
        }
        status = scan.NextBatch( batch );
    }
    result.seconds = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) / 1000.0;
    if ( countAllocations )
        result.allocations = CORETEST::StopAllocationCounting();
    return status;
}

/** Tier 1: every cell through one reused CVariant, text linked */
static TDataTableStatus
ScanVariants( CDataTableScan& scan, SBenchmarkResult& result, bool countAllocations, UInt64 expectedRows )
{
    CDataTableBatchPtr batch;
    TDataTableStatus status = scan.NextBatch( batch );
    CVariant cell;
    if ( countAllocations )
        CORETEST::StartAllocationCounting();
    const UInt64 start = MT::PrecisionTickCount();
    while ( DATATABLE_STATUS_OK == status )
    {
        const CDataTableBatch& rows = *batch;
        const UInt32 count = rows.GetRowCount();
        for ( UInt32 i=0; i<count; ++i )
        {
            rows.GetValue( i, 0, cell );
            result.tsXor ^= (UInt64) cell.AsInt64();
            rows.GetValue( i, 1, cell );
            result.closeSum += cell.AsFloat64();
            rows.GetValue( i, 2, cell );
            result.symbolBytes += cell.ByteSize();
        }
        result.rows += count;
        if ( countAllocations && result.rows == expectedRows )
        {
            // the call that ends the scan raises ScanFinishedEvent, mailed with a copy of its data: once per scan
            result.allocations = CORETEST::StopAllocationCounting();
            countAllocations = false;
        }
        status = scan.NextBatch( batch );
    }
    result.seconds = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) / 1000.0;
    if ( countAllocations )
        result.allocations = CORETEST::StopAllocationCounting();
    return status;
}

static Float64
RowsPerSecond( const SBenchmarkResult& result )
{
    return result.seconds > 0.0 ? (Float64) result.rows / result.seconds : 0.0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDataTableScanBenchmarkTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "DataTableScanBenchmark" );

    GUCEF_TESTFW_TESTCASE( "Test: benchmark: Int64, Float64 and dictionary text scanned through spans, CVariant cells and async" )
        const char* enabled = getenv( "GUCEF_TEST_BENCHMARK" );
        GUCEF_TESTFW_SKIP_IF( GUCEF_NULL == enabled || '\0' == *enabled || '0' == *enabled, "GUCEF_TEST_BENCHMARK is not set" );
        try
        {
            #ifdef GUCEF_DEBUG_MODE
            UInt64 rowCount = 1000000;
            #else
            UInt64 rowCount = 10000000;
            #endif
            const char* rowsText = getenv( "GUCEF_TEST_BENCHMARK_ROWS" );
            if ( GUCEF_NULL != rowsText && '\0' != *rowsText )
                rowCount = (UInt64) StringToUInt64( rowsText );
            const UInt32 rowGroupRows = 1048576;

            CDataTableBufferPool pool( 2048ULL * 1024 * 1024 );
            CDataTableDefinition definition;
            definition.name = "bars";
            definition.schema.AddColumn( CDataTableColumnDef::Timestamp( "ts", DATATABLE_TIMEUNIT_MICROSECONDS, true, false ) );
            definition.schema.AddColumn( CDataTableColumnDef( "close", DATATABLE_TYPE_FLOAT64, false ) );
            definition.schema.AddColumn( CDataTableColumnDef( "symbol", DATATABLE_TYPE_UTF8, false ) );
            definition.storage.rowGroupRowCount = rowGroupRows;
            CMemoryDataTable table( definition, &pool );
            CDataTableSchemaPtr schema = table.GetSchema();

            // the dictionary of symbols, shared by the row groups
            CDataTableSchema dictionarySchema;
            dictionarySchema.AddColumn( CDataTableColumnDef( "symbol", DATATABLE_TYPE_UTF8, false ) );
            CDataTableBatchBuilder dictionaryBuilder( CreateDataTableSchemaPtr( dictionarySchema ), pool );
            for ( UInt32 i=0; i<BenchmarkSymbols; ++i )
                dictionaryBuilder.GetColumn( 0 ).AppendString( "SYM" + ToString( i ) );
            CDataTableBatchPtr dictionary;
            CString error;
            ASSERT_TRUE( DATATABLE_STATUS_OK == dictionaryBuilder.Build( dictionary, error ) );

            CORETEST::CTestRandom random( 20261104 );
            Float64 close = 100.0;
            for ( UInt64 first=0; first<rowCount; first+=rowGroupRows )
            {
                const UInt32 rows = (UInt32) ( rowCount - first < rowGroupRows ? rowCount - first : rowGroupRows );
                CDataTableBatchPtr rowGroup = MakeBenchmarkRowGroup( schema, pool, dictionary, random, 1700000000000000LL + (Int64) first * 60000000LL, rows, close );
                CDataTableWriteResult result;
                ASSERT_TRUE( DATATABLE_STATUS_OK == table.Write( *rowGroup, DATATABLE_WRITE_INSERT, result ) );
            }
            ASSERT_TRUE( rowCount == table.GetRowCount() );

            // a memcpy of the same bytes, the bandwidth the spans pass is measured against
            const UInt64 bytes = rowCount * 20;
            CDynamicBuffer source( (UInt32) bytes, true );
            CDynamicBuffer target( (UInt32) bytes, true );
            source.SetDataSize( (UInt32) bytes );
            memset( source.GetBufferPtr(), 1, (size_t) bytes );
            UInt64 start = MT::PrecisionTickCount();
            memcpy( target.GetBufferPtr(), source.GetConstBufferPtr(), (size_t) bytes );
            const Float64 memcpySeconds = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) / 1000.0;
            const Float64 memcpyBandwidth = memcpySeconds > 0.0 ? bytes / memcpySeconds : 0.0;

            const bool counting = CORETEST::IsAllocationCountingAvailable();
            SBenchmarkResult warmup, spans, variants, async;
            ClearResult( warmup );
            ClearResult( spans );
            ClearResult( variants );
            ClearResult( async );

            CDataTableScanPtr scan;
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Scan( CDataTableQuery(), scan ) );
            ASSERT_TRUE( DATATABLE_STATUS_END == ScanSpans( *scan, warmup, false, rowCount ) );

            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Scan( CDataTableQuery(), scan ) );
            ASSERT_TRUE( DATATABLE_STATUS_END == ScanSpans( *scan, spans, counting, rowCount ) );
            CDataTableScanStats stats;
            scan->GetStats( stats );
            spans.poolMisses = stats.poolMisses;

            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Scan( CDataTableQuery(), scan ) );
            ASSERT_TRUE( DATATABLE_STATUS_END == ScanVariants( *scan, variants, counting, rowCount ) );
            scan->GetStats( stats );
            variants.poolMisses = stats.poolMisses;

            ThreadPoolPtr threadPool = CORETEST::GetDataTableTestThreadPool( "DataTableBenchmark", 4 );
            CFutureResult future = table.ScanAsync( CDataTableQuery().SetMaxParallelism( 4 ), scan, threadPool );
            ASSERT_TRUE( DATATABLE_STATUS_END == ScanSpans( *scan, async, counting, rowCount ) );
            scan->GetStats( stats );
            async.poolMisses = stats.poolMisses;
            scan.Unlink();

            ASSERT_TRUE( rowCount == spans.rows && rowCount == variants.rows && rowCount == async.rows );
            ASSERT_TRUE( spans.closeSum == warmup.closeSum && spans.tsXor == variants.tsXor && spans.tsXor == async.tsXor );
            ASSERT_TRUE( 0 == memcmp( spans.histogram, async.histogram, sizeof( spans.histogram ) ) );

            const Float64 spanBandwidth = spans.seconds > 0.0 ? bytes / spans.seconds : 0.0;
            GUCEF_LOG( LOGLEVEL_NORMAL, "DataTableScanBenchmark: " << rowCount << " rows (ts int64, close float64, symbol utf8 dictionary of " << BenchmarkSymbols << ")" );
            GUCEF_LOG( LOGLEVEL_NORMAL, "DataTableScanBenchmark: spans sync: " << (UInt64) RowsPerSecond( spans ) << " rows/s, " << ( spanBandwidth / 1048576.0 ) <<
                       " MiB/s, " << spans.allocations << " allocations, " << spans.poolMisses << " pool misses" );
            GUCEF_LOG( LOGLEVEL_NORMAL, "DataTableScanBenchmark: CVariant cells sync: " << (UInt64) RowsPerSecond( variants ) << " rows/s (" << (UInt64) ( RowsPerSecond( variants ) * 3 ) <<
                       " cells/s), " << variants.allocations << " allocations, " << variants.poolMisses << " pool misses" );
            GUCEF_LOG( LOGLEVEL_NORMAL, "DataTableScanBenchmark: spans async, 4 tasks: " << (UInt64) RowsPerSecond( async ) << " rows/s, " << async.allocations <<
                       " allocations on the consumer thread, " << async.poolMisses << " pool misses" );
            GUCEF_LOG( LOGLEVEL_NORMAL, "DataTableScanBenchmark: memcpy of the same bytes: " << ( memcpyBandwidth / 1048576.0 ) << " MiB/s" <<
                       ( counting ? "" : "; allocation counting not available: " ) << ( counting ? "" : CORETEST::GetAllocationCountingUnavailableReason() ) );

            // the gates: no heap allocation and no pool miss in a warm sync scan, through spans or cells
            if ( counting )
            {
                if ( 0 != spans.allocations || 0 != variants.allocations )
                    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "DataTableScanBenchmark: allocations:\n" << CORETEST::GetCountedAllocationStacks() );
                ASSERT_TRUE( 0 == spans.allocations && 0 == variants.allocations );
            }
            ASSERT_TRUE( 0 == spans.poolMisses && 0 == variants.poolMisses );
            #ifndef GUCEF_DEBUG_MODE
            ASSERT_TRUE( spanBandwidth >= 0.5 * memcpyBandwidth );
            #endif
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
