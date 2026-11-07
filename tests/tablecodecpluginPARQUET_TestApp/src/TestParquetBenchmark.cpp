/*
 *  tablecodecpluginPARQUET_TestApp: tests of the parquet table codec plugin
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
#include <stdlib.h>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_CFILEDATATABLE_H
#include "gucefCORE_CFileDataTable.h"
#define GUCEF_CORE_CFILEDATATABLE_H
#endif /* GUCEF_CORE_CFILEDATATABLE_H ? */

#ifndef GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#include "gucefCORE_CDataTableBatchBuilder.h"
#define GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#endif /* GUCEF_CORE_CDATATABLEBATCHBUILDER_H ? */

#ifndef GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#include "gucefCORE_CDataTableBufferPool.h"
#define GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#endif /* GUCEF_CORE_CDATATABLEBUFFERPOOL_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAllocationCounter.h"
#include "TestParquetSupport.h"
#include "TestParquetBenchmark.h"

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
using namespace PARQUETTEST;

namespace {

const UInt32 BenchmarkSymbols = 500;

struct SPassResult
{
    Float64 seconds;
    UInt64 rows;
    UInt64 valueBytes;          /**< bytes of the decoded values the pass touched */
    UInt32 allocations;
    UInt64 poolMisses;
    CDataTableScanStats stats;
    Float64 checksum;

    SPassResult( void ) : seconds( 0 ), rows( 0 ), valueBytes( 0 ), allocations( 0 ), poolMisses( 0 ), stats(), checksum( 0 ) {}
};

/** ts timestamp(us,utc) increasing, close float64 random walk, symbol utf8 of 500 values in random order */
TDataTableStatus
WriteBenchmarkFile( const CString& path, UInt64 rows, const char* compression, CString& error )
{
    CDataTableSchema schema;
    schema.AddColumn( CDataTableColumnDef::Timestamp( "ts", DATATABLE_TIMEUNIT_MICROSECONDS, true, false ) );
    schema.AddColumn( CDataTableColumnDef( "close", DATATABLE_TYPE_FLOAT64, false ) );
    schema.AddColumn( CDataTableColumnDef( "symbol", DATATABLE_TYPE_UTF8, false ) );
    const CDataTableSchemaPtr schemaPtr = CreateDataTableSchemaPtr( schema );
    CFileAccess* access = GUCEF_NEW CFileAccess( path, "wb" );
    IOAccessPtr output( access );
    if ( !access->IsValid() )
    {
        error = "cannot create " + path;
        return DATATABLE_STATUS_IO_ERROR;
    }
    CDataTableWriterOptions options;
    options.compression = compression;
    CITableWriterPtr writer;
    TDataTableStatus status = GetParquetCodec()->CreateWriter( output, schemaPtr, options, writer, error );
    if ( DATATABLE_STATUS_OK != status )
        return status;
    CTestRandom random( 20261104 );
    char symbols[ BenchmarkSymbols ][ 8 ];
    for ( UInt32 s=0; s<BenchmarkSymbols; ++s )
        snprintf( symbols[ s ], sizeof( symbols[ s ] ), "S%04u", s );
    Float64 close = 100.0;
    const Int64 firstTs = 1700000000000000LL;
    UInt64 written = 0;
    GUCEF::vector< Int64 > ts;
    GUCEF::vector< Float64 > closes;
    while ( written < rows && DATATABLE_STATUS_OK == status )
    {
        const UInt32 count = (UInt32) ( rows - written < 1048576 ? rows - written : 1048576 );
        CDataTableBatchBuilder builder( schemaPtr, CCoreGlobal::Instance()->GetDataTableBufferPool() );
        ts.resize( count );
        closes.resize( count );
        for ( UInt32 i=0; i<count; ++i )
        {
            ts[ i ] = firstTs + (Int64) ( written + i ) * 60000000LL;
            close += ( (Float64) (Int32) random.Below( 2001 ) - 1000.0 ) * 0.0001;
            closes[ i ] = close;
            const char* symbol = symbols[ random.Below( BenchmarkSymbols ) ];
            builder.GetColumn( 2 ).AppendUtf8( symbol, 5 );
        }
        builder.GetColumn( 0 ).AppendInt64s( &ts[ 0 ], count );
        builder.GetColumn( 1 ).AppendFloat64s( &closes[ 0 ], count );
        CDataTableBatchPtr batch;
        status = builder.Build( batch, error );
        if ( DATATABLE_STATUS_OK == status )
            status = writer->WriteBatch( *batch );
        written += count;
    }
    CDataTableFileSummary summary;
    if ( DATATABLE_STATUS_OK == status )
        status = writer->Finish( summary );
    if ( DATATABLE_STATUS_OK != status && error.IsNULLOrEmpty() )
        error = writer->GetLastErrorMessage();
    writer.Unlink();
    output->Close();
    return status;
}

/** One pass over the file: spans (Tier 2) of the selected columns, or cells (Tier 1) */
TDataTableStatus
RunPass( const CString& path, bool symbolOnly, bool cells, UInt64 totalRows, SPassResult& result, CString& error )
{
    result = SPassResult();
    CFileAccess* access = GUCEF_NEW CFileAccess( path, "rb" );
    IOAccessPtr input( access );
    CFileDataTable table( input, GetParquetCodec(), CValueList(), "benchmark" );
    TDataTableStatus status = table.Open();
    if ( DATATABLE_STATUS_OK != status )
    {
        error = table.GetLastErrorMessage();
        return status;
    }
    CDataTableQuery query;
    if ( symbolOnly )
        query.Select( "symbol" );
    else
        query.Select( "ts" ).Select( "close" );
    query.SetBatchRowCount( 16384 );
    CDataTableScanPtr scan;
    status = table.Scan( query, scan );
    if ( DATATABLE_STATUS_OK != status )
    {
        error = table.GetLastErrorMessage();
        return status;
    }

    bool counted = false;
    CDataTableBufferPoolStats before;
    CCoreGlobal::Instance()->GetDataTableBufferPool().GetStats( before );
    UInt64 histogram[ BenchmarkSymbols + 1 ];
    memset( histogram, 0, sizeof( histogram ) );
    Float64 sum = 0;
    Int64 hash = 0;
    CVariant cell;
    const bool counting = CORETEST::IsAllocationCountingAvailable();
    bool first = true;
    const UInt64 start = MT::PrecisionTickCount();
    for ( ;; )
    {
        CDataTableBatchPtr batch;
        status = scan->NextBatch( batch );
        if ( DATATABLE_STATUS_OK != status )
            break;
        if ( first && counting )
        {
            // the window runs from the first batch to the last
            CORETEST::StartAllocationCounting();
        }
        first = false;
        const UInt32 rows = batch->GetRowCount();
        result.rows += rows;
        if ( result.rows >= totalRows && counting && !counted )
        {
            // the window ends with the last batch: the call that ends the scan mails ScanFinishedEvent, a fixed cost per scan
            counted = true;
            result.allocations = CORETEST::StopAllocationCounting();
        }
        if ( symbolOnly )
        {
            const CDataTableColumnBatch& symbol = batch->GetColumn( 0 );
            const UInt32* indices = symbol.GetDictionaryIndices();
            if ( GUCEF_NULL != indices )
            {
                for ( UInt32 r=0; r<rows; ++r )
                    ++histogram[ indices[ r ] < BenchmarkSymbols ? indices[ r ] : BenchmarkSymbols ];
                result.valueBytes += (UInt64) rows * 4;
            }
            else
            {
                for ( UInt32 r=0; r<rows; ++r )
                {
                    UInt32 length = 0;
                    histogram[ (UInt8) symbol.GetVariableValue( r, length )[ 4 ] ] += length;
                }
                result.valueBytes += (UInt64) rows * 6;
            }
        }
        else
        if ( cells )
        {
            for ( UInt32 r=0; r<rows; ++r )
            {
                batch->GetValue( r, 0, cell, true );
                hash ^= cell.AsInt64();
                batch->GetValue( r, 1, cell, true );
                sum += cell.AsFloat64();
            }
            result.valueBytes += (UInt64) rows * 16;
        }
        else
        {
            const Int64* ts = batch->GetColumn( 0 ).GetInt64Data();
            const Float64* close = batch->GetColumn( 1 ).GetFloat64Data();
            for ( UInt32 r=0; r<rows; ++r )
            {
                hash ^= ts[ r ];
                sum += close[ r ];
            }
            result.valueBytes += (UInt64) rows * 16;
        }
    }
    result.seconds = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) / 1000.0;
    if ( counting && !first && !counted )
        result.allocations = CORETEST::StopAllocationCounting();
    if ( counting && !first )
    {
        if ( result.allocations > 0 )
            GUCEF_LOG( LOGLEVEL_NORMAL, "Parquet benchmark: where the allocations of the pass came from:\n" << CORETEST::GetCountedAllocationStacks() );
    }
    CDataTableBufferPoolStats after;
    CCoreGlobal::Instance()->GetDataTableBufferPool().GetStats( after );
    result.poolMisses = after.misses - before.misses;
    scan->GetStats( result.stats );
    result.checksum = sum + (Float64) ( hash & 0xFFFF ) + (Float64) histogram[ 7 ];
    if ( DATATABLE_STATUS_END == status )
        status = DATATABLE_STATUS_OK;
    if ( DATATABLE_STATUS_OK != status )
        error = scan->GetLastErrorMessage();
    return status;
}

void
Report( const char* name, const SPassResult& result )
{
    const Float64 seconds = result.seconds > 0 ? result.seconds : 1e-9;
    GUCEF_LOG( LOGLEVEL_NORMAL, "Parquet benchmark: " << name << ": " << result.rows << " rows in " << seconds << " s: " <<
               (UInt64) ( result.rows / seconds ) << " rows/s, " << (UInt64) ( result.valueBytes / seconds / 1000000.0 ) << " MB/s of values, " <<
               (UInt64) ( result.stats.bytesRead / seconds / 1000000.0 ) << " MB/s of file; heap allocations " << result.allocations <<
               ", pool misses " << result.poolMisses << ", pages in place " << result.stats.pagesInPlace << ", decoded " << result.stats.pagesDecoded <<
               ", alignment copies " << result.stats.alignmentCopies << " (checksum " << result.checksum << ")" );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformParquetBenchmarkTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "tablecodecpluginPARQUET.Benchmark" );

    GUCEF_TESTFW_TESTCASE( "Benchmark: decoding a file of ts, close and symbol: PLAIN pages in place, the dictionary column as indices, cells, and GZIP (GUCEF_TEST_BENCHMARK=1)" )
    GUCEF_TESTFW_REQUIRE_ENV( enabled, "GUCEF_TEST_BENCHMARK" );
    try
    {
        #ifdef GUCEF_DEBUG_MODE
        UInt64 rows = 1000000;
        #else
        UInt64 rows = 10000000;
        #endif
        const char* rowsText = getenv( "GUCEF_TEST_BENCHMARK_ROWS" );
        if ( GUCEF_NULL != rowsText && 0 != atoi( rowsText ) )
            rows = (UInt64) atoll( rowsText );
        const CString path = TestFilePath( "benchmark.parquet" );
        const CString gzipPath = TestFilePath( "benchmark_gzip.parquet" );
        CString error;
        UInt64 start = MT::PrecisionTickCount();
        ASSERT_TRUE( DATATABLE_STATUS_OK == WriteBenchmarkFile( path, rows, "none", error ) );
        const Float64 writeSeconds = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) / 1000.0;
        GUCEF_LOG( LOGLEVEL_NORMAL, "Parquet benchmark: wrote " << rows << " rows in " << writeSeconds << " s (" << (UInt64) ( rows / writeSeconds ) << " rows/s), " <<
                   FileSize( path ) << " bytes" );

        SPassResult warmUp, plain, dictionary, cells;
        ASSERT_TRUE( DATATABLE_STATUS_OK == RunPass( path, false, false, rows, warmUp, error ) );
        ASSERT_TRUE( DATATABLE_STATUS_OK == RunPass( path, true, false, rows, warmUp, error ) );
        ASSERT_TRUE( DATATABLE_STATUS_OK == RunPass( path, false, false, rows, plain, error ) );
        Report( "PLAIN ts + close, spans", plain );
        ASSERT_TRUE( DATATABLE_STATUS_OK == RunPass( path, true, false, rows, dictionary, error ) );
        Report( "dictionary symbol, indices", dictionary );
        ASSERT_TRUE( DATATABLE_STATUS_OK == RunPass( path, false, true, rows, cells, error ) );
        Report( "PLAIN ts + close, CVariant cells", cells );
        ASSERT_TRUE( rows == plain.rows && rows == dictionary.rows && rows == cells.rows );

        // every page of ts and close is used in place: one per 16384 rows of each row group, two columns
        const UInt64 fullGroups = rows / 1048576;
        const UInt64 lastGroupRows = rows % 1048576;
        const UInt64 pages = 2 * ( fullGroups * 64 + ( lastGroupRows + 16383 ) / 16384 );
        ASSERT_TRUE( pages == plain.stats.pagesInPlace && 0 == plain.stats.pagesDecoded && 0 == plain.stats.alignmentCopies );
        ASSERT_TRUE( 0 == plain.allocations && 0 == plain.poolMisses );
        ASSERT_TRUE( 0 == dictionary.poolMisses );

        if ( IsGzipAvailable() )
        {
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteBenchmarkFile( gzipPath, rows, "gzip", error ) );
            SPassResult gzip;
            ASSERT_TRUE( DATATABLE_STATUS_OK == RunPass( gzipPath, false, false, rows, gzip, error ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == RunPass( gzipPath, false, false, rows, gzip, error ) );
            Report( "GZIP ts + close, spans", gzip );
            GUCEF_LOG( LOGLEVEL_NORMAL, "Parquet benchmark: the GZIP file has " << FileSize( gzipPath ) << " bytes" );
        }
        DeleteFile( path );
        DeleteFile( gzipPath );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
