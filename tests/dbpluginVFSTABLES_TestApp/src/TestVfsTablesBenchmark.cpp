/*
 *  dbpluginVFSTABLES_TestApp: tests of the vfstables database backend
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

/*
 *  Throughput of the backend over a multi partition table of daily bars, reported in the log: writes, full scans
 *  (sync and async), a scan pruned to a few symbols and years, and the effect of the footer cache. With
 *  GUCEF_TEST_BENCHMARK=1 the table has 200 symbols over 20 years (about a million rows in 4000 files), otherwise
 *  20 symbols over 5 years. The numbers are reported, not judged; the counts of rows and files are checked.
 *
 *  Only with GUCEF_TEST_BENCHMARK=1: the history of one symbol read from venue day files (one file per trading day holding
 *  100 symbols, 5218 files by default or GUCEF_TEST_BENCHMARK_FILES), sync and async with 1 to 64 files in flight, with the
 *  footers cold and warm: the cost per small file of listing, opening, reading the footer and decoding.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdlib.h>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#ifndef GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#include "gucefCORE_CDataTableBatchBuilder.h"
#define GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#endif /* GUCEF_CORE_CDATATABLEBATCHBUILDER_H ? */

#ifndef DBPLUGIN_VFSTABLES_CVFSDATATABLE_H
#include "dbpluginVFSTABLES_CVfsDataTable.h"
#define DBPLUGIN_VFSTABLES_CVFSDATATABLE_H
#endif /* DBPLUGIN_VFSTABLES_CVFSDATATABLE_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#include "TestVfsTablesSupport.h"
#include "TestVfsTablesBenchmark.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::CORE;
using namespace VFSTABLESTEST;

namespace {

struct SRequirementFailed {};
#define REQUIRE_TRUE(t) { const bool requirementMet = ( t ); GUCEF_TESTFW_ASSERT_TRUE( requirementMet ); if ( !requirementMet ) throw SRequirementFailed(); }

typedef DBPLUGIN::VFSTABLES::CVfsDataTable      TVfsTable;
typedef DBPLUGIN::VFSTABLES::CVfsTablesStats    TStorageStats;

CDataTableDefinition
MakeBarsDefinition( void )
{
    CDataTableDefinition definition;
    definition.name = "bars_1d";
    definition.codecType = "parquet";
    definition.schema.AddColumn( CDataTableColumnDef( "mic", DATATABLE_TYPE_UTF8, false ) );
    definition.schema.AddColumn( CDataTableColumnDef( "symbol", DATATABLE_TYPE_UTF8, false ) );
    definition.schema.AddColumn( CDataTableColumnDef( "date", DATATABLE_TYPE_DATE, false ) );
    definition.schema.AddColumn( CDataTableColumnDef::Decimal( "open", 18, 8 ) );
    definition.schema.AddColumn( CDataTableColumnDef::Decimal( "high", 18, 8 ) );
    definition.schema.AddColumn( CDataTableColumnDef::Decimal( "low", 18, 8 ) );
    definition.schema.AddColumn( CDataTableColumnDef::Decimal( "close", 18, 8 ) );
    definition.schema.AddColumn( CDataTableColumnDef( "volume", DATATABLE_TYPE_INT64, true ) );
    CString::StringVector key;
    key.push_back( "mic" );
    key.push_back( "symbol" );
    key.push_back( "date" );
    definition.schema.SetPrimaryKey( key );
    CString::StringVector sortKey;
    sortKey.push_back( "date" );
    definition.schema.SetSortKey( sortKey );
    definition.partitioning.AddField( "mic", "mic", DATATABLE_PARTITION_TRANSFORM_IDENTITY );
    definition.partitioning.AddField( "symbol", "symbol", DATATABLE_PARTITION_TRANSFORM_IDENTITY );
    definition.partitioning.AddField( "year", "date", DATATABLE_PARTITION_TRANSFORM_YEAR );
    definition.partitioning.SetPathTemplate( "{mic}/{symbol}/bars_1d/{year}.parquet" );
    return definition;
}

Float64
SecondsSince( UInt64 startTicks )
{
    return MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - startTicks ) / 1000.0;
}

/** Pulls every batch of the scan, summing the close mantissas through the spans; rows seen and the scan statistics */
TDataTableStatus
DrainScan( CDataTableScan& scan, UInt32 closeColumn, UInt64& rowCount, Int64& checksum )
{
    rowCount = 0;
    checksum = 0;
    TDataTableStatus status = DATATABLE_STATUS_OK;
    while ( true )
    {
        CDataTableBatchPtr batch;
        status = scan.NextBatch( batch );
        if ( DATATABLE_STATUS_OK != status )
            break;
        const CDataTableColumnBatch& close = batch->GetColumn( closeColumn );
        const Int64* mantissas = close.GetInt64Data();
        const UInt32 rows = batch->GetRowCount();
        if ( GUCEF_NULL != mantissas )
            for ( UInt32 r=0; r<rows; ++r )
                checksum += mantissas[ r ];
        rowCount += batch->GetSelectedRowCount();
    }
    return DATATABLE_STATUS_END == status ? DATATABLE_STATUS_OK : status;
}

/** Daily bars of a venue: one file per venue and trading day holding every symbol of the day, sorted by symbol */
CDataTableDefinition
MakeVenueDayBarsDefinition( void )
{
    CDataTableDefinition definition;
    definition.name = "daily_bars";
    definition.codecType = "parquet";
    definition.schema.AddColumn( CDataTableColumnDef( "mic", DATATABLE_TYPE_UTF8, false ) );
    definition.schema.AddColumn( CDataTableColumnDef( "trading_date", DATATABLE_TYPE_DATE, false ) );
    definition.schema.AddColumn( CDataTableColumnDef( "symbol", DATATABLE_TYPE_UTF8, false ) );
    definition.schema.AddColumn( CDataTableColumnDef::Timestamp( "bar_start_timestamp_utc", DATATABLE_TIMEUNIT_MICROSECONDS, true, false ) );
    definition.schema.AddColumn( CDataTableColumnDef::Decimal( "open", 18, 8, false ) );
    definition.schema.AddColumn( CDataTableColumnDef::Decimal( "high", 18, 8, false ) );
    definition.schema.AddColumn( CDataTableColumnDef::Decimal( "low", 18, 8, false ) );
    definition.schema.AddColumn( CDataTableColumnDef::Decimal( "close", 18, 8, false ) );
    definition.schema.AddColumn( CDataTableColumnDef( "volume", DATATABLE_TYPE_INT64, false ) );
    definition.schema.AddColumn( CDataTableColumnDef( "source", DATATABLE_TYPE_UTF8, false ) );
    definition.schema.AddColumn( CDataTableColumnDef::Timestamp( "ingested_timestamp_utc", DATATABLE_TIMEUNIT_MICROSECONDS, true, false ) );
    CString::StringVector key;
    key.push_back( "mic" );
    key.push_back( "trading_date" );
    key.push_back( "symbol" );
    definition.schema.SetPrimaryKey( key );
    CString::StringVector sortKey;
    sortKey.push_back( "symbol" );
    definition.schema.SetSortKey( sortKey );
    definition.partitioning.AddField( "mic", "mic", DATATABLE_PARTITION_TRANSFORM_IDENTITY );
    definition.partitioning.AddField( "year", "trading_date", DATATABLE_PARTITION_TRANSFORM_YEAR );
    definition.partitioning.AddField( "tradingDate", "trading_date", DATATABLE_PARTITION_TRANSFORM_DAY );
    definition.partitioning.SetPathTemplate( "{mic}/{year}/{tradingDate}/daily_bars.parquet" );
    return definition;
}

/** One scan of the history of a symbol, timed: rows, files opened, footer cache hits */
struct SHistoryRead
{
    Float64 seconds;
    UInt64 rows;
    Int64 checksum;
    CDataTableScanStats stats;

    SHistoryRead( void ) : seconds( 0 ), rows( 0 ), checksum( 0 ), stats() {}
};

bool
ReadHistory( CIDataTable& table, const CDataTableQuery& query, bool async, UInt32 parallelism, ThreadPoolPtr pool, SHistoryRead& result )
{
    CDataTableQuery scanQuery( query );
    CDataTableScanPtr scan;
    const UInt64 start = MT::PrecisionTickCount();
    if ( async )
    {
        scanQuery.SetMaxParallelism( parallelism );
        table.ScanAsync( scanQuery, scan, pool );
        if ( scan.IsNULL() )
            return false;
    }
    else
    if ( DATATABLE_STATUS_OK != table.Scan( scanQuery, scan ) )
    {
        return false;
    }
    // close is the second projected column
    const bool ok = DATATABLE_STATUS_OK == DrainScan( *scan, 1, result.rows, result.checksum );
    result.seconds = SecondsSince( start );
    scan->GetStats( result.stats );
    return ok;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformVfsTablesBenchmarkTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "VfsTablesBenchmark" );

    GUCEF_TESTFW_TESTCASE( "Benchmark: writes and scans of daily bars over many partitions, full and pruned, with the footer cache cold and warm" )
        try
        {
            const char* benchmark = getenv( "GUCEF_TEST_BENCHMARK" );
            const bool large = GUCEF_NULL != benchmark && '1' == benchmark[ 0 ];
            const UInt32 symbolCount = large ? 200 : 20;
            const Int32 firstYear = large ? 2005 : 2018;
            const Int32 lastYear = 2024;
            static const char* const mics[] = { "XNYS", "XNAS", "XTSE", "XAMS" };

            MountFreshDirectory( "benchmark", "benchmarkdata", true );
            COMCORE::CIDatabaseClientPtr client = OpenDatabase( MakeSettings( "benchmarkdata", false ) );
            REQUIRE_TRUE( !client.IsNULL() );
            CIDataTablePtr bars;
            REQUIRE_TRUE( DATATABLE_STATUS_OK == client->CreateTable( MakeBarsDefinition(), bars ) );
            TVfsTable* vfsTable = static_cast< TVfsTable* >( bars.GetPointerAlways() );

            // a year of every symbol per write, built straight into a batch
            UInt64 rowsWritten = 0;
            UInt32 files = 0;
            const UInt64 writeStart = MT::PrecisionTickCount();
            for ( Int32 year=firstYear; year<=lastYear; ++year )
            {
                CDataTableBatchBuilder builder( bars->GetSchema(), CCoreGlobal::Instance()->GetDataTableBufferPool() );
                Int32 first = 0;
                Int32 last = 0;
                CDataTableValues::DaysFromCivil( year, 1, 1, first );
                CDataTableValues::DaysFromCivil( year + 1, 1, 1, last );
                for ( UInt32 s=0; s<symbolCount; ++s )
                {
                    const CString symbol = "S" + ToString( s );
                    for ( Int32 d=first; d<last; ++d )
                    {
                        if ( ( d + 3 ) % 7 >= 5 )
                            continue;
                        const Int64 price = 100000000000LL + (Int64) s * 1000000LL + d % 500;
                        builder.GetColumn( 0 ).AppendString( mics[ s % 4 ] );
                        builder.GetColumn( 1 ).AppendString( symbol );
                        builder.GetColumn( 2 ).AppendDate( d );
                        builder.GetColumn( 3 ).AppendDecimalMantissa( price );
                        builder.GetColumn( 4 ).AppendDecimalMantissa( price + 100 );
                        builder.GetColumn( 5 ).AppendDecimalMantissa( price - 100 );
                        builder.GetColumn( 6 ).AppendDecimalMantissa( price + 50 );
                        builder.GetColumn( 7 ).AppendInt64( 1000 + d % 77 );
                    }
                    ++files;
                }
                CDataTableBatchPtr batch;
                CString error;
                REQUIRE_TRUE( DATATABLE_STATUS_OK == builder.Build( batch, error ) );
                CDataTableWriteResult result;
                REQUIRE_TRUE( DATATABLE_STATUS_OK == bars->Write( *batch, DATATABLE_WRITE_INSERT, result ) );
                rowsWritten += result.GetTotalRowsWritten();
            }
            const Float64 writeSeconds = SecondsSince( writeStart );
            GUCEF_LOG( LOGLEVEL_NORMAL, "VfsTables benchmark: wrote " << rowsWritten << " rows into " << files << " partition files in " << writeSeconds <<
                       " s: " << (UInt64) ( rowsWritten / ( writeSeconds > 0 ? writeSeconds : 1e-9 ) ) << " rows/s" );

            CDataTableQuery closes;
            closes.Select( "close" );

            // cold: listings and footers read from storage
            vfsTable->Refresh();
            vfsTable->GetStore()->ResetStats();
            CDataTableScanPtr scan;
            UInt64 rows = 0;
            Int64 checksum = 0;
            UInt64 start = MT::PrecisionTickCount();
            REQUIRE_TRUE( DATATABLE_STATUS_OK == bars->Scan( closes, scan ) );
            REQUIRE_TRUE( DATATABLE_STATUS_OK == DrainScan( *scan, 0, rows, checksum ) );
            Float64 seconds = SecondsSince( start );
            CDataTableScanStats stats;
            scan->GetStats( stats );
            TStorageStats storage;
            vfsTable->GetStorageStats( storage );
            ASSERT_TRUE( rowsWritten == rows && files == stats.filesOpened );
            GUCEF_LOG( LOGLEVEL_NORMAL, "VfsTables benchmark: full scan, sync, cold: " << rows << " rows in " << seconds << " s: " << (UInt64) ( rows / seconds ) <<
                       " rows/s; files opened " << stats.filesOpened << ", listings " << storage.directoryListings << ", footer reads " << storage.footerReads );
            const Int64 coldChecksum = checksum;

            // warm: listings and footers cached
            vfsTable->GetStore()->ResetStats();
            start = MT::PrecisionTickCount();
            REQUIRE_TRUE( DATATABLE_STATUS_OK == bars->Scan( closes, scan ) );
            REQUIRE_TRUE( DATATABLE_STATUS_OK == DrainScan( *scan, 0, rows, checksum ) );
            seconds = SecondsSince( start );
            scan->GetStats( stats );
            vfsTable->GetStorageStats( storage );
            ASSERT_TRUE( rowsWritten == rows && coldChecksum == checksum && files == stats.footerCacheHits && 0 == storage.directoryListings );
            GUCEF_LOG( LOGLEVEL_NORMAL, "VfsTables benchmark: full scan, sync, warm: " << rows << " rows in " << seconds << " s: " << (UInt64) ( rows / seconds ) <<
                       " rows/s; footer cache hits " << stats.footerCacheHits << ", listings " << storage.directoryListings );

            // async, ordered, on the pool of the database
            start = MT::PrecisionTickCount();
            bars->ScanAsync( closes, scan );
            REQUIRE_TRUE( !scan.IsNULL() && DATATABLE_STATUS_OK == DrainScan( *scan, 0, rows, checksum ) );
            seconds = SecondsSince( start );
            ASSERT_TRUE( rowsWritten == rows && coldChecksum == checksum );
            GUCEF_LOG( LOGLEVEL_NORMAL, "VfsTables benchmark: full scan, async, warm: " << rows << " rows in " << seconds << " s: " << (UInt64) ( rows / seconds ) << " rows/s" );

            // every column
            start = MT::PrecisionTickCount();
            REQUIRE_TRUE( DATATABLE_STATUS_OK == bars->Scan( CDataTableQuery(), scan ) );
            REQUIRE_TRUE( DATATABLE_STATUS_OK == DrainScan( *scan, 6, rows, checksum ) );
            seconds = SecondsSince( start );
            ASSERT_TRUE( rowsWritten == rows && coldChecksum == checksum );
            GUCEF_LOG( LOGLEVEL_NORMAL, "VfsTables benchmark: full scan of every column, sync, warm: " << rows << " rows in " << seconds << " s: " << (UInt64) ( rows / seconds ) << " rows/s" );

            // every file opened and planned, every row group pruned on its statistics: the cost per file without decoding
            CDataTableQuery nothing;
            nothing.Select( "close" ).Where( CDataTablePredicate( "volume", DATATABLE_PREDICATE_GREATER, CVariant( (Int64) 1000000 ) ) );
            start = MT::PrecisionTickCount();
            REQUIRE_TRUE( DATATABLE_STATUS_OK == bars->Scan( nothing, scan ) );
            REQUIRE_TRUE( DATATABLE_STATUS_OK == DrainScan( *scan, 0, rows, checksum ) );
            seconds = SecondsSince( start );
            scan->GetStats( stats );
            ASSERT_TRUE( 0 == rows && files == stats.rowGroupsPrunedByStatistics );
            GUCEF_LOG( LOGLEVEL_NORMAL, "VfsTables benchmark: scan of every file with every row group pruned on statistics, warm: " << files << " files in " << seconds <<
                       " s: " << (UInt64) ( 1000000.0 * seconds / files ) << " us per file" );

            // the partitions listed with their file statistics: discovery and the opens through the VFS
            CDataTablePartitionInfo::TPartitionInfoVector partitions;
            start = MT::PrecisionTickCount();
            REQUIRE_TRUE( DATATABLE_STATUS_OK == bars->ListPartitions( CDataTableQuery(), true, partitions ) );
            seconds = SecondsSince( start );
            ASSERT_TRUE( files == partitions.size() );
            GUCEF_LOG( LOGLEVEL_NORMAL, "VfsTables benchmark: partitions listed with file statistics, warm: " << files << " files in " << seconds <<
                       " s: " << (UInt64) ( 1000000.0 * seconds / files ) << " us per file" );
            // the part of that the VFS takes: an existence check and an open for reading per file
            VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
            UInt32 opened = 0;
            start = MT::PrecisionTickCount();
            for ( UInt32 p=0; p<(UInt32) partitions.size(); ++p )
            {
                const CString path = "benchmarkdata/" + partitions[ p ].path;
                if ( vfs.FileExists( path ) && !vfs.GetFile( path, "rb", false ).IsNULL() )
                    ++opened;
            }
            seconds = SecondsSince( start );
            ASSERT_TRUE( files == opened );
            GUCEF_LOG( LOGLEVEL_NORMAL, "VfsTables benchmark: CVFS::FileExists and CVFS::GetFile alone: " << files << " files in " << seconds <<
                       " s: " << (UInt64) ( 1000000.0 * seconds / files ) << " us per file" );
            start = MT::PrecisionTickCount();
            REQUIRE_TRUE( DATATABLE_STATUS_OK == bars->ListPartitions( CDataTableQuery(), false, partitions ) );
            seconds = SecondsSince( start );
            GUCEF_LOG( LOGLEVEL_NORMAL, "VfsTables benchmark: partitions listed without opening them, warm: " << files << " files in " << seconds <<
                       " s: " << (UInt64) ( 1000000.0 * seconds / files ) << " us per file" );

            // pruned: four symbols over two years, cold
            vfsTable->Refresh();
            vfsTable->GetStore()->ResetStats();
            CVariant::VariantVector symbols;
            CVariant::VariantVector symbolMics;
            for ( UInt32 s=0; s<4; ++s )
            {
                symbols.push_back( CVariant( "S" + ToString( s * 3 ) ) );
                symbolMics.push_back( CVariant( mics[ ( s * 3 ) % 4 ] ) );
            }
            CDataTableQuery pruned;
            pruned.Select( "close" ).WhereIn( "mic", symbolMics ).WhereIn( "symbol", symbols ).WhereRange( "date", Day( 2020, 1, 1 ), true, Day( 2021, 12, 31 ), true );
            start = MT::PrecisionTickCount();
            REQUIRE_TRUE( DATATABLE_STATUS_OK == bars->Scan( pruned, scan ) );
            REQUIRE_TRUE( DATATABLE_STATUS_OK == DrainScan( *scan, 0, rows, checksum ) );
            seconds = SecondsSince( start );
            scan->GetStats( stats );
            vfsTable->GetStorageStats( storage );
            ASSERT_TRUE( 8 == stats.filesOpened && rows > 2000 );
            GUCEF_LOG( LOGLEVEL_NORMAL, "VfsTables benchmark: pruned scan (4 symbols, 2 years of " << files << " files), cold: " << rows << " rows in " << seconds <<
                       " s; files opened " << stats.filesOpened << ", pruned while listing " << stats.filesPruned << ", listings " << storage.directoryListings <<
                       ", probes " << storage.fileProbes );
            scan.Unlink();
            bars.Unlink();
            client.Unlink();
            UnmountDirectory( "benchmark" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Benchmark: the history of one symbol from thousands of small venue day files, sync and async with 1 to 64 files in flight, footers cold and warm (GUCEF_TEST_BENCHMARK=1)" )
        const char* enabled = getenv( "GUCEF_TEST_BENCHMARK" );
        GUCEF_TESTFW_SKIP_IF( GUCEF_NULL == enabled || '\0' == *enabled || '0' == *enabled, "GUCEF_TEST_BENCHMARK is not set" );
        try
        {
            // twenty years of trading days by default (5218 files of 100 symbols), GUCEF_TEST_BENCHMARK_FILES for another number
            UInt32 dayCount = 5218;
            const char* filesText = getenv( "GUCEF_TEST_BENCHMARK_FILES" );
            if ( GUCEF_NULL != filesText && 0 != StringToUInt32( filesText, 0 ) )
                dayCount = StringToUInt32( filesText, 0 );
            const UInt32 symbolCount = 100;
            const CString mic = "XNYS";

            MountFreshDirectory( "venuedays", "venuedaysdata", true );
            // the footers of every day file in the cache, like the market data store sets it
            COMCORE::CDatabaseClientSettings settings = MakeSettings( "venuedaysdata", false );
            settings.backendOptions.Set( "footerCacheBytes", CVariant( "268435456" ) );
            COMCORE::CIDatabaseClientPtr client = OpenDatabase( settings );
            REQUIRE_TRUE( !client.IsNULL() );
            CIDataTablePtr bars;
            REQUIRE_TRUE( DATATABLE_STATUS_OK == client->CreateTable( MakeVenueDayBarsDefinition(), bars ) );
            TVfsTable* vfsTable = static_cast< TVfsTable* >( bars.GetPointerAlways() );

            CString::StringVector symbols;
            for ( UInt32 s=0; s<symbolCount; ++s )
            {
                CString symbol = "S";
                if ( s < 10 )
                    symbol += "00";
                else
                if ( s < 100 )
                    symbol += "0";
                symbols.push_back( symbol + ToString( s ) );
            }

            // the weekdays from 2006-01-02 on, a year of days per write
            Int32 day = 0;
            CDataTableValues::DaysFromCivil( 2006, 1, 2, day );
            UInt32 daysWritten = 0;
            UInt64 rowsWritten = 0;
            const Int64 microsPerDay = 86400LL * 1000000LL;
            const UInt64 writeStart = MT::PrecisionTickCount();
            while ( daysWritten < dayCount )
            {
                CDataTableBatchBuilder builder( bars->GetSchema(), CCoreGlobal::Instance()->GetDataTableBufferPool() );
                for ( UInt32 d=0; d<261 && daysWritten<dayCount; ++day )
                {
                    if ( ( day + 3 ) % 7 >= 5 )
                        continue;
                    for ( UInt32 s=0; s<symbolCount; ++s )
                    {
                        const Int64 price = 100000000000LL + (Int64) s * 1000000LL + day % 500;
                        builder.GetColumn( 0 ).AppendString( mic );
                        builder.GetColumn( 1 ).AppendDate( day );
                        builder.GetColumn( 2 ).AppendString( symbols[ s ] );
                        builder.GetColumn( 3 ).AppendTimestamp( day * microsPerDay + 5 * 3600LL * 1000000LL );
                        builder.GetColumn( 4 ).AppendDecimalMantissa( price );
                        builder.GetColumn( 5 ).AppendDecimalMantissa( price + 100 );
                        builder.GetColumn( 6 ).AppendDecimalMantissa( price - 100 );
                        builder.GetColumn( 7 ).AppendDecimalMantissa( price + 50 );
                        builder.GetColumn( 8 ).AppendInt64( 1000000 + day % 77 + s );
                        builder.GetColumn( 9 ).AppendString( "benchmark" );
                        builder.GetColumn( 10 ).AppendTimestamp( day * microsPerDay + 23 * 3600LL * 1000000LL + s );
                    }
                    ++d;
                    ++daysWritten;
                }
                CDataTableBatchPtr batch;
                CString error;
                REQUIRE_TRUE( DATATABLE_STATUS_OK == builder.Build( batch, error ) );
                CDataTableWriteResult result;
                REQUIRE_TRUE( DATATABLE_STATUS_OK == bars->Write( *batch, DATATABLE_WRITE_INSERT, result ) );
                rowsWritten += result.GetTotalRowsWritten();
            }
            GUCEF_LOG( LOGLEVEL_NORMAL, "VfsTables venue day benchmark: wrote " << rowsWritten << " rows into " << daysWritten << " day files in " << SecondsSince( writeStart ) << " s" );

            // the history of one symbol: every column, like a read of a bar series
            CDataTableQuery history;
            history.Select( "trading_date" ).Select( "close" ).Select( "open" ).Select( "high" ).Select( "low" ).Select( "volume" ).Select( "bar_start_timestamp_utc" );
            history.WhereEquals( "symbol", CVariant( symbols[ 42 ] ) );

            ThreadPoolPtr pool = CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( "VfsTablesBenchmark.scan" );
            REQUIRE_TRUE( !pool.IsNULL() );
            pool->SetNrOfWorkerThreadsToLogicalCPUs( 1 );

            SHistoryRead reference;
            vfsTable->Refresh();
            REQUIRE_TRUE( ReadHistory( *bars, history, false, 0, pool, reference ) );
            ASSERT_TRUE( daysWritten == reference.rows && daysWritten == reference.stats.filesOpened );
            GUCEF_LOG( LOGLEVEL_NORMAL, "VfsTables venue day benchmark: sync, footers cold: " << reference.rows << " rows from " << reference.stats.filesOpened <<
                       " files in " << reference.seconds << " s: " << (UInt64) ( 1000000.0 * reference.seconds / daysWritten ) << " us per file" );
            SHistoryRead read;
            REQUIRE_TRUE( ReadHistory( *bars, history, false, 0, pool, read ) );
            ASSERT_TRUE( reference.rows == read.rows && reference.checksum == read.checksum && daysWritten == read.stats.footerCacheHits );
            GUCEF_LOG( LOGLEVEL_NORMAL, "VfsTables venue day benchmark: sync, footers warm: " << read.rows << " rows in " << read.seconds << " s: " <<
                       (UInt64) ( 1000000.0 * read.seconds / daysWritten ) << " us per file" );

            static const UInt32 parallelisms[] = { 1, 4, 8, 16, 64 };
            for ( UInt32 p=0; p<sizeof( parallelisms ) / sizeof( parallelisms[ 0 ] ); ++p )
            {
                vfsTable->Refresh();
                REQUIRE_TRUE( ReadHistory( *bars, history, true, parallelisms[ p ], pool, read ) );
                ASSERT_TRUE( reference.rows == read.rows && reference.checksum == read.checksum && daysWritten == read.stats.filesOpened );
                const Float64 coldSeconds = read.seconds;
                REQUIRE_TRUE( ReadHistory( *bars, history, true, parallelisms[ p ], pool, read ) );
                ASSERT_TRUE( reference.rows == read.rows && reference.checksum == read.checksum && daysWritten == read.stats.footerCacheHits );
                GUCEF_LOG( LOGLEVEL_NORMAL, "VfsTables venue day benchmark: async, parallelism " << parallelisms[ p ] << ": footers cold " << coldSeconds << " s (" <<
                           (UInt64) ( 1000000.0 * coldSeconds / daysWritten ) << " us per file), warm " << read.seconds << " s (" <<
                           (UInt64) ( 1000000.0 * read.seconds / daysWritten ) << " us per file)" );
            }

            bars.Unlink();
            client.Unlink();
            UnmountDirectory( "venuedays" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
