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
 *  The market data store of the investment software (Investment_Architecture.md 7.3, section 14.3 of the data tables
 *  design) as a consumer of the backend: daily bars per market, symbol and year, intraday bars per exchange trading
 *  date, FX rates under a literal "FX" level, tickers escaped into path segments. Rows of several markets, symbols and
 *  years go in through appends and a correction; queries with symbol and date predicates must open only the files
 *  they need and give exactly the rows a brute force filter over everything written gives.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <algorithm>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef DBPLUGIN_VFSTABLES_CVFSDATATABLE_H
#include "dbpluginVFSTABLES_CVfsDataTable.h"
#define DBPLUGIN_VFSTABLES_CVFSDATATABLE_H
#endif /* DBPLUGIN_VFSTABLES_CVFSDATATABLE_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestVfsTablesSupport.h"
#include "TestVfsTablesMarketData.h"

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

/** A listing: its market (ISO 10383 MIC) and the ticker as the provider gives it */
struct SListing
{
    const char* mic;
    const char* symbol;
};

const SListing g_listings[] = {
    { "XNYS", "KO" }, { "XNYS", "BRK.B" }, { "XNYS", "BF-B" }, { "XNYS", "BRK/B" },
    { "XNAS", "AAPL" }, { "XNAS", "MSFT" },
    { "XTSE", "RY" }, { "XTSE", "SHOP" },
    { "XASX", "BHP" }
};
const UInt32 g_listingCount = (UInt32) ( sizeof( g_listings ) / sizeof( g_listings[ 0 ] ) );

void
AddColumns( CDataTableSchema& schema, bool intraday )
{
    schema.AddColumn( CDataTableColumnDef( "mic", DATATABLE_TYPE_UTF8, false ) );
    schema.AddColumn( CDataTableColumnDef( "symbol", DATATABLE_TYPE_UTF8, false ) );
    if ( intraday )
    {
        schema.AddColumn( CDataTableColumnDef( "tradingDate", DATATABLE_TYPE_DATE, false ) );
        schema.AddColumn( CDataTableColumnDef::Timestamp( "ts", DATATABLE_TIMEUNIT_MICROSECONDS, true, false ) );
    }
    else
    {
        schema.AddColumn( CDataTableColumnDef( "date", DATATABLE_TYPE_DATE, false ) );
    }
    schema.AddColumn( CDataTableColumnDef::Decimal( "open", 18, 8 ) );
    schema.AddColumn( CDataTableColumnDef::Decimal( "high", 18, 8 ) );
    schema.AddColumn( CDataTableColumnDef::Decimal( "low", 18, 8 ) );
    schema.AddColumn( CDataTableColumnDef::Decimal( "close", 18, 8 ) );
    schema.AddColumn( CDataTableColumnDef( "volume", DATATABLE_TYPE_INT64, true ) );
    schema.AddColumn( CDataTableColumnDef( "source", DATATABLE_TYPE_UTF8, true ) );
}

/** bars_1d: {mic}/{symbol}/bars_1d/{year}.parquet, a file per year */
CDataTableDefinition
MakeDailyBarsDefinition( void )
{
    CDataTableDefinition definition;
    definition.name = "bars_1d";
    definition.codecType = "parquet";
    AddColumns( definition.schema, false );
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

/** bars_1m: {mic}/{symbol}/bars_1m/{year}/{tradingDate}.parquet, a file per exchange trading date */
CDataTableDefinition
MakeMinuteBarsDefinition( bool gzip )
{
    CDataTableDefinition definition;
    definition.name = "bars_1m";
    definition.codecType = "parquet";
    AddColumns( definition.schema, true );
    CString::StringVector key;
    key.push_back( "mic" );
    key.push_back( "symbol" );
    key.push_back( "tradingDate" );
    key.push_back( "ts" );
    definition.schema.SetPrimaryKey( key );
    CString::StringVector sortKey;
    sortKey.push_back( "ts" );
    definition.schema.SetSortKey( sortKey );
    definition.partitioning.AddField( "mic", "mic", DATATABLE_PARTITION_TRANSFORM_IDENTITY );
    definition.partitioning.AddField( "symbol", "symbol", DATATABLE_PARTITION_TRANSFORM_IDENTITY );
    definition.partitioning.AddField( "year", "tradingDate", DATATABLE_PARTITION_TRANSFORM_YEAR );
    definition.partitioning.AddField( "tradingDate", "tradingDate", DATATABLE_PARTITION_TRANSFORM_DAY );
    definition.partitioning.SetPathTemplate( "{mic}/{symbol}/bars_1m/{year}/{tradingDate}.parquet" );
    if ( gzip )
        definition.storage.compression = "gzip";
    return definition;
}

/** fx_1d: FX/{pair}/fx_1d/{year}.parquet */
CDataTableDefinition
MakeFxDefinition( void )
{
    CDataTableDefinition definition;
    definition.name = "fx_1d";
    definition.codecType = "parquet";
    definition.schema.AddColumn( CDataTableColumnDef( "pair", DATATABLE_TYPE_UTF8, false ) );
    definition.schema.AddColumn( CDataTableColumnDef( "date", DATATABLE_TYPE_DATE, false ) );
    definition.schema.AddColumn( CDataTableColumnDef::Decimal( "rate", 18, 10 ) );
    definition.schema.AddColumn( CDataTableColumnDef( "source", DATATABLE_TYPE_UTF8, true ) );
    CString::StringVector key;
    key.push_back( "pair" );
    key.push_back( "date" );
    definition.schema.SetPrimaryKey( key );
    definition.partitioning.AddField( "pair", "pair", DATATABLE_PARTITION_TRANSFORM_IDENTITY );
    definition.partitioning.AddField( "year", "date", DATATABLE_PARTITION_TRANSFORM_YEAR );
    definition.partitioning.SetPathTemplate( "FX/{pair}/fx_1d/{year}.parquet" );
    return definition;
}

CVariant
Price( Int64 mantissa )
{
    CDecimal value;
    CDecimal::FromUnscaled( mantissa, 8, value );
    return CDataTableValues::Decimal( value );
}

Int32
DaysOf( Int32 year, UInt32 month, UInt32 day )
{
    Int32 days = 0;
    CDataTableValues::DaysFromCivil( year, month, day, days );
    return days;
}

/** A daily bar; prices follow from the listing and the day so a correction is told apart */
TRow
DailyBar( const SListing& listing, Int32 days, Int64 adjustment, const char* source )
{
    const Int64 base = ( 1000 + (Int64) ( listing.symbol[ 0 ] ) * 7 + days % 97 ) * 1000000LL + adjustment;
    TRow row;
    row.push_back( CVariant( listing.mic ) );
    row.push_back( CVariant( listing.symbol ) );
    row.push_back( CDataTableValues::Date( days ) );
    row.push_back( Price( base ) );
    row.push_back( Price( base + 2500000 ) );
    row.push_back( Price( base - 2500000 ) );
    row.push_back( Price( base + 1000000 ) );
    row.push_back( CVariant( (Int64) ( 100000 + days % 1000 ) ) );
    row.push_back( CVariant( source ) );
    return row;
}

/** A minute bar of a session: the trading date of the exchange, the bar time in UTC */
TRow
MinuteBar( const SListing& listing, Int32 tradingDate, Int64 ticks, UInt32 minute )
{
    const Int64 base = ( 500 + (Int64) minute ) * 1000000LL;
    TRow row;
    row.push_back( CVariant( listing.mic ) );
    row.push_back( CVariant( listing.symbol ) );
    row.push_back( CDataTableValues::Date( tradingDate ) );
    row.push_back( CDataTableValues::Timestamp( ticks, DATATABLE_TIMEUNIT_MICROSECONDS ) );
    row.push_back( Price( base ) );
    row.push_back( Price( base + 10000 ) );
    row.push_back( Price( base - 10000 ) );
    row.push_back( Price( base + 5000 ) );
    row.push_back( CVariant( (Int64) minute ) );
    row.push_back( CVariant( "feed" ) );
    return row;
}

bool
IsWeekday( Int32 days )
{
    // 1970-01-01 was a Thursday; 0 = Monday
    Int32 weekday = ( days + 3 ) % 7;
    if ( weekday < 0 )
        weekday += 7;
    return weekday < 5;
}

/** The scan and the brute force evaluation give the same rows; the scan statistics come back */
bool
SameRowsAsBruteForce( CIDataTable& table, const TRowVector& everything, const CDataTableQuery& query, CDataTableScanStats& stats, UInt32& rowCount )
{
    CString::StringVector scanned;
    CString::StringVector expected;
    if ( DATATABLE_STATUS_OK != ScanRows( table, query, scanned, &stats ) )
        return false;
    BruteForceRows( table.GetSchema(), everything, query, expected );
    rowCount = (UInt32) scanned.size();
    return scanned == expected;
}

TStorageStats
FreshStorageStats( CIDataTablePtr& table )
{
    TVfsTable* vfsTable = static_cast< TVfsTable* >( table.GetPointerAlways() );
    vfsTable->Refresh();
    vfsTable->GetStore()->ResetStats();
    return TStorageStats();
}

TStorageStats
StorageStatsOf( CIDataTablePtr& table )
{
    TStorageStats stats;
    static_cast< TVfsTable* >( table.GetPointerAlways() )->GetStorageStats( stats );
    return stats;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformVfsTablesMarketDataTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "VfsTablesMarketData" );

    GUCEF_TESTFW_TESTCASE( "Market data: daily bars per market, symbol and year; queries on symbols and dates open only their files and give the rows of a brute force filter" )
        try
        {
            const CString dir = MountFreshDirectory( "marketdata", "marketdata", true );
            COMCORE::CIDatabaseClientPtr client = OpenDatabase( MakeSettings( "marketdata", false ) );
            REQUIRE_TRUE( !client.IsNULL() );
            CIDataTablePtr bars;
            REQUIRE_TRUE( DATATABLE_STATUS_OK == client->CreateTable( MakeDailyBarsDefinition(), bars ) );

            // five years appended a year at a time, in two writes per year: what a provider refresh does
            TRowVector everything;
            UInt32 files = 0;
            for ( Int32 year=2018; year<=2022; ++year )
            {
                for ( UInt32 half=0; half<2; ++half )
                {
                    TRowVector rows;
                    const Int32 first = DaysOf( year, 0 == half ? 1 : 7, 1 );
                    const Int32 last = DaysOf( 0 == half ? year : year + 1, 0 == half ? 7 : 1, 1 );
                    for ( UInt32 l=0; l<g_listingCount; ++l )
                        for ( Int32 d=first; d<last; ++d )
                            if ( IsWeekday( d ) )
                                rows.push_back( DailyBar( g_listings[ l ], d, 0, "providerA" ) );
                    CDataTableWriteResult result;
                    CDataTableBatchPtr batch = MakeBatch( bars->GetSchema(), rows );
                    REQUIRE_TRUE( !batch.IsNULL() && DATATABLE_STATUS_OK == bars->Write( *batch, DATATABLE_WRITE_INSERT, result ) );
                    ASSERT_TRUE( g_listingCount == result.partitions.size() );
                    everything.insert( everything.end(), rows.begin(), rows.end() );
                }
                files += g_listingCount;
            }

            // a correction of one week of KO from another provider: an upsert by key
            {
                TRowVector corrections;
                for ( Int32 d=DaysOf( 2020, 3, 9 ); d<DaysOf( 2020, 3, 14 ); ++d )
                    corrections.push_back( DailyBar( g_listings[ 0 ], d, 7, "providerB" ) );
                CDataTableWriteResult result;
                CDataTableBatchPtr batch = MakeBatch( bars->GetSchema(), corrections );
                REQUIRE_TRUE( DATATABLE_STATUS_OK == bars->Write( *batch, DATATABLE_WRITE_UPSERT, result ) );
                ASSERT_TRUE( 1 == result.partitions.size() && 5 == result.partitions[ 0 ].rowsReplaced );
                for ( UInt32 i=0; i<(UInt32) everything.size(); ++i )
                {
                    const Int32 days = everything[ i ][ 2 ].AsInt32();
                    if ( everything[ i ][ 1 ].AsString() == "KO" && days >= DaysOf( 2020, 3, 9 ) && days < DaysOf( 2020, 3, 14 ) )
                        everything[ i ] = DailyBar( g_listings[ 0 ], days, 7, "providerB" );
                }
            }

            // the files are where the layout puts them, tickers escaped into one path segment each
            ASSERT_TRUE( FileExists( CombinePath( dir, "XNYS/KO/bars_1d/2018.parquet" ) ) );
            ASSERT_TRUE( FileExists( CombinePath( dir, "XNYS/BRK.B/bars_1d/2020.parquet" ) ) );
            ASSERT_TRUE( FileExists( CombinePath( dir, "XNYS/BF-B/bars_1d/2021.parquet" ) ) );
            ASSERT_TRUE( FileExists( CombinePath( dir, "XNYS/BRK%2FB/bars_1d/2022.parquet" ) ) );
            ASSERT_TRUE( FileExists( CombinePath( dir, "XASX/BHP/bars_1d/2019.parquet" ) ) );
            ASSERT_TRUE( files + 1 == CountFilesBelow( dir ) );     // and the definition

            CDataTableScanStats stats;
            UInt32 rows = 0;

            // one symbol over a season: the year directory of KO is listed, one file of five is opened
            FreshStorageStats( bars );
            CDataTableQuery season;
            season.WhereEquals( "mic", CVariant( "XNYS" ) ).WhereEquals( "symbol", CVariant( "KO" ) ).WhereRange( "date", Day( 2020, 3, 1 ), true, Day( 2020, 6, 30 ), true );
            ASSERT_TRUE( SameRowsAsBruteForce( *bars, everything, season, stats, rows ) );
            ASSERT_TRUE( rows > 80 && 1 == stats.filesOpened && 1 == stats.filesListed && 4 == stats.filesPruned );
            TStorageStats storage = StorageStatsOf( bars );
            ASSERT_TRUE( 1 == storage.directoryListings && 0 == storage.fileProbes && 1 == storage.footerReads );

            // the correction is what the store holds
            CDataTableQuery corrected;
            corrected.Select( "source" ).WhereEquals( "mic", CVariant( "XNYS" ) ).WhereEquals( "symbol", CVariant( "KO" ) ).WhereEquals( "source", CVariant( "providerB" ) );
            ASSERT_TRUE( SameRowsAsBruteForce( *bars, everything, corrected, stats, rows ) );
            ASSERT_TRUE( 5 == rows );

            // two symbols on two markets since 2022: four listings are built, two exist, each opens its 2022 file
            FreshStorageStats( bars );
            CVariant::VariantVector mics;
            mics.push_back( CVariant( "XNYS" ) );
            mics.push_back( CVariant( "XNAS" ) );
            CVariant::VariantVector symbols;
            symbols.push_back( CVariant( "KO" ) );
            symbols.push_back( CVariant( "AAPL" ) );
            CDataTableQuery recent;
            recent.WhereIn( "mic", mics ).WhereIn( "symbol", symbols ).Where( CDataTablePredicate( "date", DATATABLE_PREDICATE_GREATER_OR_EQUAL, Day( 2022, 1, 1 ) ) );
            ASSERT_TRUE( SameRowsAsBruteForce( *bars, everything, recent, stats, rows ) );
            ASSERT_TRUE( rows > 400 && 2 == stats.filesOpened && 2 == stats.filesListed );
            storage = StorageStatsOf( bars );
            ASSERT_TRUE( 4 == storage.directoryListings );          // the year directories of the four combinations, two of them absent

            // every symbol on one day: the markets and their symbols are listed, the year is built and probed
            FreshStorageStats( bars );
            CDataTableQuery crossSection;
            crossSection.WhereEquals( "date", Day( 2021, 6, 15 ) );
            ASSERT_TRUE( SameRowsAsBruteForce( *bars, everything, crossSection, stats, rows ) );
            ASSERT_TRUE( g_listingCount == rows && g_listingCount == stats.filesOpened );
            storage = StorageStatsOf( bars );
            ASSERT_TRUE( 5 == storage.directoryListings && g_listingCount == storage.fileProbes );

            // a ticker that needs escaping is found by its value
            CDataTableQuery slashed;
            slashed.WhereEquals( "symbol", CVariant( "BRK/B" ) ).WhereRange( "date", Day( 2019, 12, 30 ), true, Day( 2020, 1, 3 ), true );
            ASSERT_TRUE( SameRowsAsBruteForce( *bars, everything, slashed, stats, rows ) );
            ASSERT_TRUE( 5 == rows && 2 == stats.filesOpened );

            // the whole store, and a closing price filter that statistics cannot prune
            ASSERT_TRUE( SameRowsAsBruteForce( *bars, everything, CDataTableQuery(), stats, rows ) );
            ASSERT_TRUE( everything.size() == rows && files == stats.filesOpened );
            CDecimal threshold;
            CDecimal::FromUnscaled( 155, 1, threshold );
            CDataTableQuery expensive;
            expensive.Select( "symbol" ).Select( "date" ).Select( "close" ).Where( CDataTablePredicate( "close", DATATABLE_PREDICATE_GREATER, CDataTableValues::Decimal( threshold ) ) );
            ASSERT_TRUE( SameRowsAsBruteForce( *bars, everything, expensive, stats, rows ) );
            ASSERT_TRUE( rows > 0 && rows < everything.size() );

            bars.Unlink();
            client.Unlink();
            UnmountDirectory( "marketdata" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Market data: minute bars per exchange trading date, a session that starts on the previous UTC day included, and FX rates" )
        try
        {
            const CString dir = MountFreshDirectory( "intraday", "marketdata1m", true );
            COMCORE::CIDatabaseClientPtr client = OpenDatabase( MakeSettings( "marketdata1m", false ) );
            REQUIRE_TRUE( !client.IsNULL() );
            const bool gzip = !CCoreGlobal::Instance()->GetCodecRegistry().TryGetCodec( CoreCodecTypes::CompressionCodec, "gzip" ).IsNULL();
            CIDataTablePtr minutes;
            REQUIRE_TRUE( DATATABLE_STATUS_OK == client->CreateTable( MakeMinuteBarsDefinition( gzip ), minutes ) );

            // sessions of three listings over five trading dates: New York 14:30 to 21:00 UTC, Sydney from 23:00 UTC of
            // the day before (the exchange's trading date is the caller's to give: the backend knows no exchanges)
            static const UInt32 listingIndices[] = { 0, 4, 8 };
            static const Int32 tradingDays[] = { 1, 2, 3, 4, 7 };
            TRowVector everything;
            for ( UInt32 t=0; t<5; ++t )
            {
                TRowVector rows;
                const Int32 tradingDate = DaysOf( 2022, 11, 1 ) + tradingDays[ t ] - 1;
                for ( UInt32 l=0; l<3; ++l )
                {
                    const SListing& listing = g_listings[ listingIndices[ l ] ];
                    const bool sydney = 0 == strcmp( listing.mic, "XASX" );
                    const Int64 sessionStart = (Int64) ( sydney ? tradingDate - 1 : tradingDate ) * 86400000000LL + ( sydney ? 23 * 3600 : 14 * 3600 + 1800 ) * 1000000LL;
                    for ( UInt32 minute=0; minute<390; ++minute )
                        rows.push_back( MinuteBar( listing, tradingDate, sessionStart + (Int64) minute * 60000000LL, minute ) );
                }
                CDataTableWriteResult result;
                CDataTableBatchPtr batch = MakeBatch( minutes->GetSchema(), rows );
                REQUIRE_TRUE( !batch.IsNULL() && DATATABLE_STATUS_OK == minutes->Write( *batch, DATATABLE_WRITE_INSERT, result ) );
                ASSERT_TRUE( 3 == result.partitions.size() );
                everything.insert( everything.end(), rows.begin(), rows.end() );
            }
            ASSERT_TRUE( FileExists( CombinePath( dir, "XASX/BHP/bars_1m/2022/2022-11-03.parquet" ) ) );
            ASSERT_TRUE( FileExists( CombinePath( dir, "XNYS/KO/bars_1m/2022/2022-11-07.parquet" ) ) );
            ASSERT_FALSE( FileExists( CombinePath( dir, "XNYS/KO/bars_1m/2022/2022-11-05.parquet" ) ) );     // sparse: no session, no file

            CDataTableScanStats stats;
            UInt32 rows = 0;

            // one session: every level is fixed by the predicates, nothing is listed, one file is probed and opened;
            // the Sydney session of 2022-11-03 starts at 23:00 UTC on 2022-11-02 and is all in that one file
            FreshStorageStats( minutes );
            CDataTableQuery session;
            session.WhereEquals( "mic", CVariant( "XASX" ) ).WhereEquals( "symbol", CVariant( "BHP" ) ).WhereEquals( "tradingDate", Day( 2022, 11, 3 ) );
            ASSERT_TRUE( SameRowsAsBruteForce( *minutes, everything, session, stats, rows ) );
            ASSERT_TRUE( 390 == rows && 1 == stats.filesOpened );
            TStorageStats storage = StorageStatsOf( minutes );
            ASSERT_TRUE( 0 == storage.directoryListings && 1 == storage.fileProbes );
            CDataTableQuery previousUtcDay;
            previousUtcDay.WhereEquals( "mic", CVariant( "XASX" ) ).WhereEquals( "symbol", CVariant( "BHP" ) ).WhereEquals( "tradingDate", Day( 2022, 11, 3 ) )
                          .Where( CDataTablePredicate( "ts", DATATABLE_PREDICATE_LESS, CDataTableValues::Timestamp( (Int64) DaysOf( 2022, 11, 3 ) * 86400000000LL, DATATABLE_TIMEUNIT_MICROSECONDS ) ) );
            ASSERT_TRUE( SameRowsAsBruteForce( *minutes, everything, previousUtcDay, stats, rows ) );
            ASSERT_TRUE( 60 == rows );

            // a range of trading dates of one symbol: the dates of the year are listed, two of five are opened
            FreshStorageStats( minutes );
            CDataTableQuery twoDays;
            twoDays.WhereEquals( "mic", CVariant( "XNAS" ) ).WhereEquals( "symbol", CVariant( "AAPL" ) ).WhereRange( "tradingDate", Day( 2022, 11, 2 ), true, Day( 2022, 11, 3 ), true );
            ASSERT_TRUE( SameRowsAsBruteForce( *minutes, everything, twoDays, stats, rows ) );
            ASSERT_TRUE( 780 == rows && 2 == stats.filesOpened && 3 == stats.filesPruned );
            storage = StorageStatsOf( minutes );
            ASSERT_TRUE( 2 == storage.directoryListings );

            // the bars of a time window across every listing: no partition pruning on ts, the row groups' statistics decide
            CDataTableQuery window;
            window.WhereRange( "ts", CDataTableValues::Timestamp( (Int64) DaysOf( 2022, 11, 4 ) * 86400000000LL + 15LL * 3600000000LL, DATATABLE_TIMEUNIT_MICROSECONDS ), true,
                                     CDataTableValues::Timestamp( (Int64) DaysOf( 2022, 11, 4 ) * 86400000000LL + 16LL * 3600000000LL, DATATABLE_TIMEUNIT_MICROSECONDS ), false );
            ASSERT_TRUE( SameRowsAsBruteForce( *minutes, everything, window, stats, rows ) );
            ASSERT_TRUE( 120 == rows && 15 == stats.filesListed );
            minutes.Unlink();

            // FX: a literal "FX" level, a pair per directory, a file per year
            CIDataTablePtr fx;
            REQUIRE_TRUE( DATATABLE_STATUS_OK == client->CreateTable( MakeFxDefinition(), fx ) );
            TRowVector rates;
            static const char* const pairs[] = { "EURUSD", "USDCAD", "AUDUSD" };
            for ( UInt32 p=0; p<3; ++p )
            {
                for ( Int32 d=DaysOf( 2021, 12, 20 ); d<DaysOf( 2022, 1, 10 ); ++d )
                {
                    CDecimal rate;
                    CDecimal::FromUnscaled( 10000000000LL + (Int64) p * 1000000000LL + d % 50, 10, rate );
                    TRow row;
                    row.push_back( CVariant( pairs[ p ] ) );
                    row.push_back( CDataTableValues::Date( d ) );
                    row.push_back( CDataTableValues::Decimal( rate ) );
                    row.push_back( CVariant( "ecb" ) );
                    rates.push_back( row );
                }
            }
            CDataTableWriteResult result;
            CDataTableBatchPtr batch = MakeBatch( fx->GetSchema(), rates );
            REQUIRE_TRUE( DATATABLE_STATUS_OK == fx->Write( *batch, DATATABLE_WRITE_UPSERT, result ) );
            ASSERT_TRUE( 6 == result.partitions.size() );
            ASSERT_TRUE( FileExists( CombinePath( dir, "FX/EURUSD/fx_1d/2021.parquet" ) ) && FileExists( CombinePath( dir, "FX/AUDUSD/fx_1d/2022.parquet" ) ) );
            FreshStorageStats( fx );
            CDataTableQuery euro;
            euro.WhereEquals( "pair", CVariant( "EURUSD" ) ).WhereRange( "date", Day( 2021, 12, 30 ), true, Day( 2022, 1, 2 ), true );
            ASSERT_TRUE( SameRowsAsBruteForce( *fx, rates, euro, stats, rows ) );
            ASSERT_TRUE( 4 == rows && 2 == stats.filesOpened );
            storage = StorageStatsOf( fx );
            ASSERT_TRUE( 1 == storage.directoryListings );          // the years of EURUSD; "FX" and the pair are built

            // the tables share the root: each finds only its own files
            CString::StringSet names;
            ASSERT_TRUE( DATATABLE_STATUS_OK == client->ListTables( names ) && 2 == names.size() );
            fx.Unlink();
            client.Unlink();
            UnmountDirectory( "intraday" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
