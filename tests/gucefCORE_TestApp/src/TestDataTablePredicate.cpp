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
#include <limits>

#ifndef GUCEF_CORE_CDATATABLEFILTER_H
#include "gucefCORE_CDataTableFilter.h"
#include "gucefCORE_CDataTableMerge.h"
#define GUCEF_CORE_CDATATABLEFILTER_H
#endif /* GUCEF_CORE_CDATATABLEFILTER_H ? */

#ifndef GUCEF_CORE_CDATATABLEPARTITIONSPEC_H
#include "gucefCORE_CDataTablePartitionSpec.h"
#define GUCEF_CORE_CDATATABLEPARTITIONSPEC_H
#endif /* GUCEF_CORE_CDATATABLEPARTITIONSPEC_H ? */

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

#include "TestDataTableScanSupport.h"
#include "TestDataTablePredicate.h"

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

/*-------------------------------------------------------------------------*/

/** A schema for resolution tests: i8 int8, i32 int32, u8 uint8, f32 float32, f64 float64, dec decimal(9,2), d date, ts timestamp(us), s utf8, bin binary, nn int64 not nullable */
static CDataTableSchemaPtr
MakeResolveSchema( void )
{
    CDataTableSchema schema;
    schema.AddColumn( CDataTableColumnDef( "i8", DATATABLE_TYPE_INT8 ) );
    schema.AddColumn( CDataTableColumnDef( "i32", DATATABLE_TYPE_INT32 ) );
    schema.AddColumn( CDataTableColumnDef( "u8", DATATABLE_TYPE_UINT8 ) );
    schema.AddColumn( CDataTableColumnDef( "f32", DATATABLE_TYPE_FLOAT32 ) );
    schema.AddColumn( CDataTableColumnDef( "f64", DATATABLE_TYPE_FLOAT64 ) );
    schema.AddColumn( CDataTableColumnDef::Decimal( "dec", 9, 2 ) );
    schema.AddColumn( CDataTableColumnDef( "d", DATATABLE_TYPE_DATE ) );
    schema.AddColumn( CDataTableColumnDef::Timestamp( "ts", DATATABLE_TIMEUNIT_MICROSECONDS, true ) );
    schema.AddColumn( CDataTableColumnDef( "s", DATATABLE_TYPE_UTF8 ) );
    schema.AddColumn( CDataTableColumnDef( "bin", DATATABLE_TYPE_BINARY ) );
    schema.AddColumn( CDataTableColumnDef( "nn", DATATABLE_TYPE_INT64, false ) );
    schema.AddColumn( CDataTableColumnDef( "b", DATATABLE_TYPE_BOOL ) );
    return CreateDataTableSchemaPtr( schema );
}

/** Resolves a query of one predicate; false when resolution failed */
static bool
ResolveOne( const CDataTableSchemaPtr& schema, const CDataTablePredicate& predicate, CDataTableResolvedPredicate& resolved, TDataTableStatus& status )
{
    CDataTableQuery query;
    query.Where( predicate );
    CDataTableResolvedQuery result;
    result.schema = schema;
    CString error;
    const bool ok = query.Resolve( *schema, result, error );
    status = result.status;
    if ( ok && 1 == result.predicates.size() )
        resolved = result.predicates[ 0 ];
    return ok;
}

static CVariant
DecimalLiteral( const char* text )
{
    return CDataTableValues::Decimal( CDecimal::Parse( text ) );
}

/** Statistics of a unit: min / max as given, exact, nullCount of valueCount */
static CDataTableColumnStats
MakeStats( const CVariant& minValue, const CVariant& maxValue, UInt64 nullCount, UInt64 valueCount )
{
    CDataTableColumnStats stats;
    stats.hasMinMax = true;
    stats.minValue = minValue;
    stats.maxValue = maxValue;
    stats.minExact = true;
    stats.maxExact = true;
    stats.hasNullCount = true;
    stats.nullCount = nullCount;
    stats.valueCount = valueCount;
    return stats;
}

/** Whether the unit may match, for a predicate on column of the resolve schema */
static bool
StatsMayMatch( const CDataTableSchemaPtr& schema, const CDataTablePredicate& predicate, const CDataTableColumnStats& stats )
{
    CDataTableResolvedPredicate resolved;
    TDataTableStatus status = DATATABLE_STATUS_OK;
    if ( !ResolveOne( schema, predicate, resolved, status ) )
        return true;
    return CDataTablePruner::MayMatch( resolved, schema->GetColumn( resolved.columnIndex ), stats );
}

/** A batch of one dictionary encoded UTF8 column over the given dictionary and indices; row i null when nulls[ i ] */
static CDataTableBatchPtr
MakeDictionaryBatch( CDataTableBufferPool& pool, const CDataTableSchemaPtr& schema, const char** dictionary, UInt32 dictionarySize,
                     const UInt32* indices, const bool* nulls, UInt32 rowCount )
{
    // the dictionary: a batch with one variable column
    CDataTableSchema dictionarySchema;
    dictionarySchema.AddColumn( CDataTableColumnDef( "s", DATATABLE_TYPE_UTF8 ) );
    CDataTableBatchBuilder builder( CreateDataTableSchemaPtr( dictionarySchema ), pool );
    for ( UInt32 i=0; i<dictionarySize; ++i )
        builder.GetColumn( 0 ).AppendUtf8( dictionary[ i ], (UInt32) strlen( dictionary[ i ] ) );
    CDataTableBatchPtr dictionaryBatch;
    CString error;
    builder.Build( dictionaryBatch, error );

    CDataTableBufferPtr indexBuffer;
    CDataTableBufferPtr validityBuffer;
    pool.Acquire( rowCount * 4, indexBuffer );
    pool.Acquire( ( rowCount + 7 ) / 8, validityBuffer );
    UInt32* indexValues = static_cast< UInt32* >( indexBuffer->GetData() );
    UInt8* validity = static_cast< UInt8* >( validityBuffer->GetData() );
    memset( validity, 0, ( rowCount + 7 ) / 8 );
    UInt32 nullCount = 0;
    for ( UInt32 r=0; r<rowCount; ++r )
    {
        indexValues[ r ] = nulls[ r ] ? 0 : indices[ r ];
        if ( nulls[ r ] )
            ++nullCount;
        else
            validity[ r >> 3 ] = (UInt8) ( validity[ r >> 3 ] | ( 1u << ( r & 7u ) ) );
    }

    CDataTableBatchPtr batch;
    pool.AcquireBatch( batch );
    CDataTableBatch* writable = batch.GetWritable();
    writable->SetColumnCount( 1 );
    writable->SetSchema( schema );
    writable->SetRowCount( rowCount );
    writable->SetSchemaColumnIndex( 0, (UInt32) schema->FindColumn( "s" ) );
    CDataTableColumnBatch& column = writable->GetMutableColumn( 0 );
    column.SetType( DATATABLE_TYPE_UTF8 );
    column.SetDictionaryValues( rowCount, indexValues, indexBuffer, dictionaryBatch, 0 );
    column.SetValidity( validity, nullCount, validityBuffer );
    return batch;
}

/** The selected rows of a batch after the filter of the query, or GUCEF_UINT32MAX entries on failure */
static GUCEF::vector< UInt32 >
FilterRows( const CDataTableSchemaPtr& schema, const CDataTableQuery& query, CDataTableBatchPtr batch, CDataTableBufferPool& pool )
{
    GUCEF::vector< UInt32 > rows;
    CDataTableResolvedQuery resolved;
    resolved.schema = schema;
    CString error;
    if ( !query.Resolve( *schema, resolved, error ) )
    {
        rows.push_back( GUCEF_UINT32MAX );
        return rows;
    }
    CDataTableSchema::TColumnIndexVector columns;
    for ( UInt32 c=0; c<batch->GetColumnCount(); ++c )
        columns.push_back( batch->GetSchemaColumnIndex( c ) );
    CDataTableFilter filter;
    UInt32 selected = 0;
    if ( DATATABLE_STATUS_OK != filter.Prepare( resolved, columns, error ) ||
         DATATABLE_STATUS_OK != filter.Apply( *batch.GetWritable(), pool, selected ) )
    {
        rows.push_back( GUCEF_UINT32MAX );
        return rows;
    }
    const UInt32* selection = batch->GetSelection();
    for ( UInt32 i=0; i<selected; ++i )
        rows.push_back( GUCEF_NULL == selection ? i : selection[ i ] );
    return rows;
}

/*-------------------------------------------------------------------------*/

typedef GUCEF::vector< CVariant > TVariantVector;

/** A literal: from the domain when one is given, otherwise from the small domains of RandomCellValue */
static CVariant
RandomLiteral( CORETEST::CTestRandom& random, const CDataTableColumnDef& column, const TVariantVector* domain )
{
    if ( GUCEF_NULL != domain && !domain->empty() )
        return (*domain)[ random.Below( (UInt32) domain->size() ) ];
    return CORETEST::RandomCellValue( random, column, 0 );
}

/** A random predicate on a column with literals taken from the same small domains as the data, or from the given domain */
static CDataTablePredicate
RandomPredicate( CORETEST::CTestRandom& random, const CDataTableColumnDef& column, const TVariantVector* domain = GUCEF_NULL )
{
    const UInt32 kind = random.Below( 12 );
    CVariant a = RandomLiteral( random, column, domain );
    CVariant b = RandomLiteral( random, column, domain );
    switch ( kind )
    {
        case 0:  { return CDataTablePredicate( column.name, DATATABLE_PREDICATE_EQUALS, a ); }
        case 1:  { return CDataTablePredicate( column.name, DATATABLE_PREDICATE_NOT_EQUALS, a ); }
        case 2:  { return CDataTablePredicate( column.name, DATATABLE_PREDICATE_LESS, a ); }
        case 3:  { return CDataTablePredicate( column.name, DATATABLE_PREDICATE_LESS_OR_EQUAL, a ); }
        case 4:  { return CDataTablePredicate( column.name, DATATABLE_PREDICATE_GREATER, a ); }
        case 5:  { return CDataTablePredicate( column.name, DATATABLE_PREDICATE_GREATER_OR_EQUAL, a ); }
        case 6:  { return CDataTablePredicate( column.name, a, random.Chance( 50 ), b, random.Chance( 50 ) ); }
        case 7:  { return CDataTablePredicate( column.name, random.Chance( 50 ) ? a : CVariant(), true, b, false ); }
        case 8:
        {
            CVariant::VariantVector values;
            const UInt32 count = 1 + random.Below( 4 );
            for ( UInt32 i=0; i<count; ++i )
                values.push_back( RandomLiteral( random, column, domain ) );
            return CDataTablePredicate( column.name, values );
        }
        case 9:  { return CDataTablePredicate( column.name, DATATABLE_PREDICATE_IS_NULL ); }
        case 10: { return CDataTablePredicate( column.name, DATATABLE_PREDICATE_IS_NOT_NULL ); }
        default: { return CDataTablePredicate( column.name, DATATABLE_PREDICATE_EQUALS, a ); }
    }
}

/*-------------------------------------------------------------------------*/

/** The special float cells: NaN, both infinities and both zeros, in the width of the column */
static CVariant
SpecialFloat( CORETEST::CTestRandom& random, const CDataTableColumnDef& column )
{
    const UInt32 pick = random.Below( 5 );
    if ( DATATABLE_TYPE_FLOAT32 == column.type )
    {
        switch ( pick )
        {
            case 0:  { return CVariant( std::numeric_limits< Float32 >::quiet_NaN() ); }
            case 1:  { return CVariant( std::numeric_limits< Float32 >::infinity() ); }
            case 2:  { return CVariant( -std::numeric_limits< Float32 >::infinity() ); }
            case 3:  { return CVariant( (Float32) -0.0f ); }
            default: { return CVariant( (Float32) 0.0f ); }
        }
    }
    switch ( pick )
    {
        case 0:  { return CVariant( std::numeric_limits< Float64 >::quiet_NaN() ); }
        case 1:  { return CVariant( std::numeric_limits< Float64 >::infinity() ); }
        case 2:  { return CVariant( -std::numeric_limits< Float64 >::infinity() ); }
        case 3:  { return CVariant( (Float64) -0.0 ); }
        default: { return CVariant( (Float64) 0.0 ); }
    }
}

/**
 *  Random rows in which every run of rowGroupRows rows (a row group of an unsorted table) holds one or two values per
 *  column, with now and then a null or a special value (NaN, infinities, zeros of both signs for floats), so row groups
 *  whose minimum equals their maximum are the common case. domains gets, per column, the values to take literals from.
 */
static CDataTableBatchPtr
MakeNarrowBatch( const CDataTableSchemaPtr& schema, CDataTableBufferPool& pool, CORETEST::CTestRandom& random,
                 UInt32 rowCount, UInt32 rowGroupRows, GUCEF::vector< TVariantVector >& domains )
{
    const UInt32 columnCount = schema->GetColumnCount();
    domains.assign( columnCount, TVariantVector() );
    for ( UInt32 c=0; c<columnCount; ++c )
    {
        const CDataTableColumnDef& column = schema->GetColumn( c );
        if ( DATATABLE_TYPE_FLOAT32 == column.type || DATATABLE_TYPE_FLOAT64 == column.type )
            for ( UInt32 i=0; i<5; ++i )
                domains[ c ].push_back( SpecialFloat( random, column ) );
    }

    CDataTableBatchBuilder builder( schema, pool );
    TVariantVector cells( columnCount );
    TVariantVector bases( 2 * columnCount );
    static const UInt32 specialPercents[] = { 0, 0, 1, 2, 10 };
    UInt32 specialPercent = 0;
    UInt32 nullPercent = 0;
    bool nanOnly = false;
    for ( UInt32 r=0; r<rowCount; ++r )
    {
        if ( 0 == r % rowGroupRows )
        {
            specialPercent = specialPercents[ random.Below( sizeof( specialPercents ) / sizeof( specialPercents[ 0 ] ) ) ];
            nullPercent = random.Chance( 50 ) ? 0 : 3;
            nanOnly = random.Chance( 60 );
            for ( UInt32 c=0; c<columnCount; ++c )
            {
                const CDataTableColumnDef& column = schema->GetColumn( c );
                bases[ 2 * c ] = CORETEST::RandomCellValue( random, column, 0 );
                bases[ 2 * c + 1 ] = random.Chance( 40 ) ? CORETEST::RandomCellValue( random, column, 0 ) : bases[ 2 * c ];
                domains[ c ].push_back( bases[ 2 * c ] );
                domains[ c ].push_back( bases[ 2 * c + 1 ] );
            }
        }
        for ( UInt32 c=0; c<columnCount; ++c )
        {
            const CDataTableColumnDef& column = schema->GetColumn( c );
            const bool isFloat = DATATABLE_TYPE_FLOAT32 == column.type || DATATABLE_TYPE_FLOAT64 == column.type;
            if ( 0 == c )
                cells[ c ] = CVariant( (Int64) r );
            else
            if ( column.nullable && random.Chance( nullPercent ) )
                cells[ c ] = CVariant();
            else
            if ( random.Chance( specialPercent ) )
            {
                if ( !isFloat )
                    cells[ c ] = CORETEST::RandomCellValue( random, column, 0 );
                else
                if ( nanOnly )
                    cells[ c ] = DATATABLE_TYPE_FLOAT32 == column.type ? CVariant( std::numeric_limits< Float32 >::quiet_NaN() ) :
                                                                         CVariant( std::numeric_limits< Float64 >::quiet_NaN() );
                else
                    cells[ c ] = SpecialFloat( random, column );
            }
            else
                cells[ c ] = bases[ 2 * c + random.Below( 2 ) ];
        }
        builder.AppendRow( &cells[ 0 ], columnCount );
    }
    CDataTableBatchPtr batch;
    CString error;
    builder.Build( batch, error );
    return batch;
}

/** One property round: a table of random rows, random queries, the scan compared with the brute force */
static bool
RunPruningProperty( UInt64 seed, bool sortOnInt64, UInt32 queries, UInt64& rowGroupsPruned, CString& failure, bool narrow = false )
{
    CDataTableBufferPool pool;
    CDataTableSchemaPtr allTypes = CORETEST::MakeAllTypesSchema();
    CDataTableDefinition definition;
    definition.name = "property";
    definition.schema.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, false ) );
    for ( UInt32 c=0; c<allTypes->GetColumnCount(); ++c )
        definition.schema.AddColumn( allTypes->GetColumn( c ) );
    if ( sortOnInt64 )
    {
        CString::StringVector sortKey;
        sortKey.push_back( "i64" );
        definition.schema.SetSortKey( sortKey );
    }
    definition.storage.rowGroupRowCount = 97;
    definition.storage.pageRowCount = 97;
    CMemoryDataTable table( definition, &pool );
    CDataTableSchemaPtr schema = table.GetSchema();

    CORETEST::CTestRandom random( seed );
    GUCEF::vector< TVariantVector > domains;
    CDataTableBatchPtr rows = narrow ? MakeNarrowBatch( schema, pool, random, 1500, 97, domains )
                                     : CORETEST::MakeRandomBatch( schema, pool, random, 1500, 0, 20 );
    CDataTableWriteResult writeResult;
    if ( DATATABLE_STATUS_OK != table.Write( *rows, DATATABLE_WRITE_INSERT, writeResult ) )
    {
        failure = "Writing the rows failed: " + table.GetLastErrorMessage();
        return false;
    }

    // all rows in table order, as cells per column
    CDataTableScanPtr scan;
    table.Scan( CDataTableQuery().SetBatchRowCount( 1024 ), scan );
    GUCEF::vector< CDataTableBatchPtr > batches;
    CDataTableBatchPtr batch;
    while ( DATATABLE_STATUS_OK == scan->NextBatch( batch ) )
        batches.push_back( batch );

    for ( UInt32 q=0; q<queries; ++q )
    {
        CDataTableQuery query;
        query.SetBatchRowCount( 1024 );
        const UInt32 predicateCount = narrow && random.Chance( 60 ) ? 1 : 1 + random.Below( 3 );
        for ( UInt32 p=0; p<predicateCount; ++p )
        {
            UInt32 column = 1 + random.Below( schema->GetColumnCount() - 1 );
            if ( narrow && random.Chance( 40 ) )
                column = (UInt32) schema->FindColumn( random.Chance( 50 ) ? "f32" : "f64" );
            query.Where( RandomPredicate( random, schema->GetColumn( column ), narrow ? &domains[ column ] : GUCEF_NULL ) );
        }
        if ( random.Chance( 30 ) )
            query.Select( "id" ).Select( "s" );

        // the brute force over all rows
        CString::StringVector expected;
        for ( UInt32 b=0; b<(UInt32) batches.size(); ++b )
        {
            const CDataTableBatch& all = *batches[ b ];
            for ( UInt32 r=0; r<all.GetRowCount(); ++r )
            {
                bool matches = true;
                for ( UInt32 p=0; p<(UInt32) query.GetPredicates().size() && matches; ++p )
                {
                    const CDataTablePredicate& predicate = query.GetPredicates()[ p ];
                    const UInt32 column = (UInt32) schema->FindColumn( predicate.columnName );
                    CVariant cell;
                    all.GetValue( r, column, cell, false );
                    matches = CORETEST::BruteForceMatches( schema->GetColumn( column ), cell, predicate );
                }
                if ( matches )
                    expected.push_back( CORETEST::CellText( all, r, 0 ) );
            }
        }

        CDataTableScanPtr filtered;
        if ( DATATABLE_STATUS_OK != table.Scan( query, filtered ) )
        {
            failure = "Scan failed: " + table.GetLastErrorMessage();
            return false;
        }
        CString::StringVector actual;
        CDataTableBatchPtr result;
        while ( DATATABLE_STATUS_OK == filtered->NextBatch( result ) )
        {
            const UInt32* selection = result->GetSelection();
            for ( UInt32 i=0; i<result->GetSelectedRowCount(); ++i )
                actual.push_back( CORETEST::CellText( *result, GUCEF_NULL == selection ? i : selection[ i ], 0 ) );
        }
        CDataTableScanStats stats;
        filtered->GetStats( stats );
        rowGroupsPruned += stats.rowGroupsPrunedByStatistics;
        if ( actual != expected )
        {
            failure = "Seed " + ToString( seed ) + " query " + ToString( q ) + ": the scan gave " + ToString( (UInt32) actual.size() ) +
                      " rows, the brute force " + ToString( (UInt32) expected.size() );
            for ( UInt32 p=0; p<(UInt32) query.GetPredicates().size(); ++p )
                failure += "; " + query.GetPredicates()[ p ].columnName + " " + DataTablePredicateOpToString( query.GetPredicates()[ p ].op ) +
                           " " + query.GetPredicates()[ p ].low.AsString() + " / " + query.GetPredicates()[ p ].high.AsString();
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDataTablePredicateTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "DataTablePredicate" );

    GUCEF_TESTFW_TESTCASE( "Test: Resolve maps names to columns, keeps the projection order and refuses unknown columns" )
        try
        {
            CDataTableSchemaPtr schema = MakeResolveSchema();
            CDataTableResolvedQuery resolved;
            CString error;
            ASSERT_TRUE( CDataTableQuery().Select( "s" ).Select( 1 ).Resolve( *schema, resolved, error ) );
            ASSERT_TRUE( 2 == resolved.projection.size() && 8 == resolved.projection[ 0 ] && 1 == resolved.projection[ 1 ] );
            ASSERT_TRUE( !resolved.schema.IsNULL() );

            CDataTableResolvedQuery all;
            ASSERT_TRUE( CDataTableQuery().Resolve( *schema, all, error ) );
            ASSERT_TRUE( schema->GetColumnCount() == all.projection.size() );

            CDataTableResolvedQuery unknown;
            ASSERT_FALSE( CDataTableQuery().Select( "nope" ).Resolve( *schema, unknown, error ) );
            ASSERT_TRUE( DATATABLE_STATUS_NOT_FOUND == unknown.status );
            CDataTableResolvedQuery unknownPredicate;
            ASSERT_FALSE( CDataTableQuery().WhereEquals( "nope", CVariant( (Int64) 1 ) ).Resolve( *schema, unknownPredicate, error ) );
            ASSERT_TRUE( DATATABLE_STATUS_NOT_FOUND == unknownPredicate.status );
            CDataTableResolvedQuery badIndex;
            ASSERT_FALSE( CDataTableQuery().Select( 99 ).Resolve( *schema, badIndex, error ) );
            CDataTableResolvedQuery noOperand;
            ASSERT_FALSE( CDataTableQuery().Where( CDataTablePredicate( "i8", DATATABLE_PREDICATE_LESS ) ).Resolve( *schema, noOperand, error ) );
            ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == noOperand.status );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: integer literals out of the column range and inexact bounds keep the predicate's meaning" )
        try
        {
            CDataTableSchemaPtr schema = MakeResolveSchema();
            CDataTableResolvedPredicate r;
            TDataTableStatus status = DATATABLE_STATUS_OK;

            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "i8", DATATABLE_PREDICATE_EQUALS, CVariant( (Int64) 100 ) ), r, status ) );
            ASSERT_TRUE( !r.matchesNothing && 100 == r.low.AsInt64() );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "i8", DATATABLE_PREDICATE_EQUALS, CVariant( (Int64) 1000 ) ), r, status ) );
            ASSERT_TRUE( r.matchesNothing );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "i8", DATATABLE_PREDICATE_NOT_EQUALS, CVariant( (Int64) -1000 ) ), r, status ) );
            ASSERT_TRUE( r.matchesAllNonNull && !r.matchesNothing );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "i8", DATATABLE_PREDICATE_GREATER, CVariant( (Int64) 1000 ) ), r, status ) );
            ASSERT_TRUE( r.matchesNothing );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "i8", DATATABLE_PREDICATE_LESS, CVariant( (Int64) 1000 ) ), r, status ) );
            ASSERT_TRUE( r.matchesAllNonNull && !r.hasHigh );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "u8", DATATABLE_PREDICATE_GREATER_OR_EQUAL, CVariant( (Int64) -5 ) ), r, status ) );
            ASSERT_TRUE( r.matchesAllNonNull );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "u8", DATATABLE_PREDICATE_LESS, CVariant( (Int64) -5 ) ), r, status ) );
            ASSERT_TRUE( r.matchesNothing );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "i8", DATATABLE_PREDICATE_GREATER, CVariant( (UInt64) 18446744073709551615ULL ) ), r, status ) );
            ASSERT_TRUE( r.matchesNothing );

            // x < 2.5 on an integer column is x <= 2, x >= 2.5 is x >= 3, x = 2.5 matches nothing, x != 2.5 every non-null row
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "i32", DATATABLE_PREDICATE_LESS, CVariant( (Float64) 2.5 ) ), r, status ) );
            ASSERT_TRUE( r.hasHigh && r.highInclusive && 2 == r.high.AsInt64() );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "i32", DATATABLE_PREDICATE_GREATER_OR_EQUAL, CVariant( (Float64) 2.5 ) ), r, status ) );
            ASSERT_TRUE( r.hasLow && r.lowInclusive && 3 == r.low.AsInt64() );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "i32", DATATABLE_PREDICATE_GREATER, CVariant( (Float64) -2.5 ) ), r, status ) );
            ASSERT_TRUE( r.hasLow && r.lowInclusive && -2 == r.low.AsInt64() );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "i32", DATATABLE_PREDICATE_EQUALS, CVariant( (Float64) 2.5 ) ), r, status ) );
            ASSERT_TRUE( r.matchesNothing );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "i32", DATATABLE_PREDICATE_NOT_EQUALS, CVariant( (Float64) 2.5 ) ), r, status ) );
            ASSERT_TRUE( r.matchesAllNonNull );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "i32", DATATABLE_PREDICATE_EQUALS, CVariant( (Float64) 7.0 ) ), r, status ) );
            ASSERT_TRUE( !r.matchesNothing && 7 == r.low.AsInt64() );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "i32", DATATABLE_PREDICATE_LESS_OR_EQUAL, DecimalLiteral( "-3.5" ) ), r, status ) );
            ASSERT_TRUE( r.hasHigh && r.highInclusive && -4 == r.high.AsInt64() );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "i32", DATATABLE_PREDICATE_EQUALS, CVariant( std::numeric_limits< Float64 >::quiet_NaN() ) ), r, status ) );
            ASSERT_TRUE( r.matchesNothing );

            // unrelated literal types
            ASSERT_FALSE( ResolveOne( schema, CDataTablePredicate( "i32", DATATABLE_PREDICATE_EQUALS, CVariant( "12" ) ), r, status ) );
            ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == status );
            ASSERT_FALSE( ResolveOne( schema, CDataTablePredicate( "i32", DATATABLE_PREDICATE_EQUALS, CDataTableValues::Date( 3 ) ), r, status ) );
            ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == status );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: decimal, float, date, timestamp, text and binary literals resolve to the native form" )
        try
        {
            CDataTableSchemaPtr schema = MakeResolveSchema();
            CDataTableResolvedPredicate r;
            TDataTableStatus status = DATATABLE_STATUS_OK;

            // decimal(9,2): x >= 1.005 is x >= 1.01, x < 1.005 is x <= 1.00, mantissas at scale 2
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "dec", DATATABLE_PREDICATE_GREATER_OR_EQUAL, DecimalLiteral( "1.005" ) ), r, status ) );
            ASSERT_TRUE( r.hasLow && r.lowInclusive && 101 == r.low.AsInt64() );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "dec", DATATABLE_PREDICATE_LESS, DecimalLiteral( "1.005" ) ), r, status ) );
            ASSERT_TRUE( r.hasHigh && r.highInclusive && 100 == r.high.AsInt64() );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "dec", DATATABLE_PREDICATE_EQUALS, DecimalLiteral( "1.50000" ) ), r, status ) );
            ASSERT_TRUE( !r.matchesNothing && 150 == r.low.AsInt64() );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "dec", DATATABLE_PREDICATE_EQUALS, CVariant( (Int64) 3 ) ), r, status ) );
            ASSERT_TRUE( 300 == r.low.AsInt64() );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "dec", DATATABLE_PREDICATE_GREATER, DecimalLiteral( "9999999.98" ) ), r, status ) );
            ASSERT_TRUE( !r.matchesNothing );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "dec", DATATABLE_PREDICATE_GREATER, DecimalLiteral( "1000000000" ) ), r, status ) );
            ASSERT_TRUE( r.matchesNothing );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "dec", DATATABLE_PREDICATE_GREATER, CVariant( (Float64) 0.125 ) ), r, status ) );
            ASSERT_TRUE( r.hasLow && r.lowInclusive && 13 == r.low.AsInt64() );

            // floats: an integer beyond 2^53 is refused; a float64 literal on a float32 column rounds to the next float32
            ASSERT_FALSE( ResolveOne( schema, CDataTablePredicate( "f64", DATATABLE_PREDICATE_EQUALS, CVariant( (Int64) 9007199254740993LL ) ), r, status ) );
            ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == status );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "f64", DATATABLE_PREDICATE_EQUALS, CVariant( (Int64) 9007199254740992LL ) ), r, status ) );
            ASSERT_TRUE( 9007199254740992.0 == r.low.AsFloat64() );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "f32", DATATABLE_PREDICATE_EQUALS, CVariant( (Float64) 0.1 ) ), r, status ) );
            ASSERT_TRUE( r.matchesNothing );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "f32", DATATABLE_PREDICATE_GREATER, CVariant( (Float64) 0.1 ) ), r, status ) );
            ASSERT_TRUE( r.hasLow && r.lowInclusive && r.low.AsFloat64() > 0.1 && (Float64) (Float32) r.low.AsFloat64() == r.low.AsFloat64() );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "f32", DATATABLE_PREDICATE_LESS_OR_EQUAL, CVariant( (Float64) 0.1 ) ), r, status ) );
            ASSERT_TRUE( r.hasHigh && r.high.AsFloat64() < 0.1 );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "f64", DATATABLE_PREDICATE_GREATER, DecimalLiteral( "0.1" ) ), r, status ) );
            ASSERT_TRUE( r.hasLow && r.low.AsFloat64() >= 0.1 );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "f64", DATATABLE_PREDICATE_EQUALS, CVariant( std::numeric_limits< Float64 >::quiet_NaN() ) ), r, status ) );
            ASSERT_TRUE( r.matchesNothing );

            // dates: a timestamp in the day is rounded per bound
            const Int64 noonOfDay3 = 3LL * 86400000000LL + 43200000000LL;
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "d", DATATABLE_PREDICATE_EQUALS, CDataTableValues::Date( -400 ) ), r, status ) );
            ASSERT_TRUE( -400 == r.low.AsInt64() );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "d", DATATABLE_PREDICATE_GREATER_OR_EQUAL, CDataTableValues::Timestamp( noonOfDay3, DATATABLE_TIMEUNIT_MICROSECONDS ) ), r, status ) );
            ASSERT_TRUE( r.hasLow && 4 == r.low.AsInt64() );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "d", DATATABLE_PREDICATE_LESS, CDataTableValues::Timestamp( noonOfDay3, DATATABLE_TIMEUNIT_MICROSECONDS ) ), r, status ) );
            ASSERT_TRUE( r.hasHigh && r.highInclusive && 3 == r.high.AsInt64() );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "d", DATATABLE_PREDICATE_GREATER, CDataTableValues::Timestamp( -1, DATATABLE_TIMEUNIT_MILLISECONDS ) ), r, status ) );
            ASSERT_TRUE( r.hasLow && 0 == r.low.AsInt64() );

            // timestamps (us): ms widen exactly, ns round per bound, dates are midnight
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "ts", DATATABLE_PREDICATE_EQUALS, CDataTableValues::Timestamp( 1500, DATATABLE_TIMEUNIT_MILLISECONDS ) ), r, status ) );
            ASSERT_TRUE( 1500000 == r.low.AsInt64() );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "ts", DATATABLE_PREDICATE_GREATER_OR_EQUAL, CDataTableValues::Timestamp( 1500, DATATABLE_TIMEUNIT_NANOSECONDS ) ), r, status ) );
            ASSERT_TRUE( r.hasLow && 2 == r.low.AsInt64() );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "ts", DATATABLE_PREDICATE_LESS, CDataTableValues::Timestamp( -1500, DATATABLE_TIMEUNIT_NANOSECONDS ) ), r, status ) );
            ASSERT_TRUE( r.hasHigh && r.highInclusive && -2 == r.high.AsInt64() );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "ts", DATATABLE_PREDICATE_EQUALS, CDataTableValues::Timestamp( 1500, DATATABLE_TIMEUNIT_NANOSECONDS ) ), r, status ) );
            ASSERT_TRUE( r.matchesNothing );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "ts", DATATABLE_PREDICATE_EQUALS, CDataTableValues::Date( -1 ) ), r, status ) );
            ASSERT_TRUE( -86400000000LL == r.low.AsInt64() );

            // text and binary are bytes
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "s", DATATABLE_PREDICATE_EQUALS, CVariant( "XNYS" ) ), r, status ) );
            UInt32 length = 0;
            const char* bytes = CDataTablePruner::GetNativeBytes( r.low, length );
            ASSERT_TRUE( 4 == length && 0 == memcmp( bytes, "XNYS", 4 ) );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "bin", DATATABLE_PREDICATE_EQUALS, CVariant( "ab" ) ), r, status ) );
            bytes = CDataTablePruner::GetNativeBytes( r.low, length );
            ASSERT_TRUE( 2 == length && GUCEF_DATATYPE_BINARY_BLOB == r.low.GetTypeId() );
            ASSERT_FALSE( ResolveOne( schema, CDataTablePredicate( "s", DATATABLE_PREDICATE_EQUALS, CVariant( (Int64) 4 ) ), r, status ) );

            // booleans: true / false, and 0 / 1
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "b", DATATABLE_PREDICATE_EQUALS, CVariant( (Int64) 1 ) ), r, status ) );
            ASSERT_TRUE( 1 == r.low.AsInt64() );
            ASSERT_FALSE( ResolveOne( schema, CDataTablePredicate( "b", DATATABLE_PREDICATE_EQUALS, CVariant( (Int64) 2 ) ), r, status ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: IN sets are sorted without duplicates, ranges that cross match nothing, null tests on non-nullable columns" )
        try
        {
            CDataTableSchemaPtr schema = MakeResolveSchema();
            CDataTableResolvedPredicate r;
            TDataTableStatus status = DATATABLE_STATUS_OK;

            CVariant::VariantVector values;
            values.push_back( CVariant( (Int64) 5 ) );
            values.push_back( CVariant( (Int64) -3 ) );
            values.push_back( CVariant( (Int64) 5 ) );
            values.push_back( CVariant( (Float64) 2.5 ) );
            values.push_back( CVariant( (Int64) 1000 ) );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "i8", values ), r, status ) );
            ASSERT_TRUE( 2 == r.values.size() && -3 == r.values[ 0 ].AsInt64() && 5 == r.values[ 1 ].AsInt64() );

            CVariant::VariantVector none;
            none.push_back( CVariant( (Float64) 0.5 ) );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "i8", none ), r, status ) );
            ASSERT_TRUE( r.matchesNothing );

            CVariant::VariantVector words;
            words.push_back( CVariant( "b" ) );
            words.push_back( CVariant( "a" ) );
            words.push_back( CVariant( "b" ) );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "s", words ), r, status ) );
            ASSERT_TRUE( 2 == r.values.size() && "a" == r.values[ 0 ].AsString() );

            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "i32", CVariant( (Int64) 5 ), true, CVariant( (Int64) 4 ), true ), r, status ) );
            ASSERT_TRUE( r.matchesNothing );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "i32", CVariant( (Int64) 5 ), true, CVariant( (Int64) 5 ), false ), r, status ) );
            ASSERT_TRUE( r.matchesNothing );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "i32", CVariant( (Int64) 5 ), true, CVariant( (Int64) 5 ), true ), r, status ) );
            ASSERT_TRUE( !r.matchesNothing );
            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "i32", CVariant(), true, CVariant( (Int64) 5 ), true ), r, status ) );
            ASSERT_TRUE( !r.hasLow && r.hasHigh );

            ASSERT_TRUE( ResolveOne( schema, CDataTablePredicate( "nn", DATATABLE_PREDICATE_IS_NULL ), r, status ) );
            ASSERT_TRUE( r.matchesNothing );

            CDataTableQuery query;
            query.WhereEquals( "i8", CVariant( (Int64) 1000 ) ).WhereEquals( "i32", CVariant( (Int64) 1 ) );
            CDataTableResolvedQuery resolved;
            CString error;
            ASSERT_TRUE( query.Resolve( *schema, resolved, error ) );
            ASSERT_TRUE( resolved.matchesNothing );
            CDataTableResolvedQuery::TResolvedPredicateVector onI32;
            resolved.GetPredicatesOnColumn( 1, onI32 );
            ASSERT_TRUE( 1 == onI32.size() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: the statistics pruning rules of every operator" )
        try
        {
            CDataTableSchemaPtr schema = MakeResolveSchema();
            const CDataTableColumnStats stats = MakeStats( CVariant( (Int64) 10 ), CVariant( (Int64) 20 ), 2, 10 );
            const CDataTableColumnStats allNull = MakeStats( CVariant( (Int64) 0 ), CVariant( (Int64) 0 ), 10, 10 );
            CDataTableColumnStats noNulls = stats;
            noNulls.nullCount = 0;
            CDataTableColumnStats single = MakeStats( CVariant( (Int64) 15 ), CVariant( (Int64) 15 ), 0, 10 );
            CDataTableColumnStats none;

            // EQUALS
            ASSERT_FALSE( StatsMayMatch( schema, CDataTablePredicate( "i32", DATATABLE_PREDICATE_EQUALS, CVariant( (Int64) 9 ) ), stats ) );
            ASSERT_TRUE( StatsMayMatch( schema, CDataTablePredicate( "i32", DATATABLE_PREDICATE_EQUALS, CVariant( (Int64) 10 ) ), stats ) );
            ASSERT_TRUE( StatsMayMatch( schema, CDataTablePredicate( "i32", DATATABLE_PREDICATE_EQUALS, CVariant( (Int64) 20 ) ), stats ) );
            ASSERT_FALSE( StatsMayMatch( schema, CDataTablePredicate( "i32", DATATABLE_PREDICATE_EQUALS, CVariant( (Int64) 21 ) ), stats ) );
            ASSERT_FALSE( StatsMayMatch( schema, CDataTablePredicate( "i32", DATATABLE_PREDICATE_EQUALS, CVariant( (Int64) 0 ) ), allNull ) );
            ASSERT_TRUE( StatsMayMatch( schema, CDataTablePredicate( "i32", DATATABLE_PREDICATE_EQUALS, CVariant( (Int64) 0 ) ), none ) );

            // NOT_EQUALS: only min == max == v prunes, and only with exact bounds
            ASSERT_FALSE( StatsMayMatch( schema, CDataTablePredicate( "i32", DATATABLE_PREDICATE_NOT_EQUALS, CVariant( (Int64) 15 ) ), single ) );
            ASSERT_TRUE( StatsMayMatch( schema, CDataTablePredicate( "i32", DATATABLE_PREDICATE_NOT_EQUALS, CVariant( (Int64) 14 ) ), single ) );
            CDataTableColumnStats inexact = single;
            inexact.minExact = false;
            ASSERT_TRUE( StatsMayMatch( schema, CDataTablePredicate( "i32", DATATABLE_PREDICATE_NOT_EQUALS, CVariant( (Int64) 15 ) ), inexact ) );

            // ranges with their inclusive flags
            ASSERT_FALSE( StatsMayMatch( schema, CDataTablePredicate( "i32", DATATABLE_PREDICATE_LESS, CVariant( (Int64) 10 ) ), stats ) );
            ASSERT_TRUE( StatsMayMatch( schema, CDataTablePredicate( "i32", DATATABLE_PREDICATE_LESS_OR_EQUAL, CVariant( (Int64) 10 ) ), stats ) );
            ASSERT_FALSE( StatsMayMatch( schema, CDataTablePredicate( "i32", DATATABLE_PREDICATE_GREATER, CVariant( (Int64) 20 ) ), stats ) );
            ASSERT_TRUE( StatsMayMatch( schema, CDataTablePredicate( "i32", DATATABLE_PREDICATE_GREATER_OR_EQUAL, CVariant( (Int64) 20 ) ), stats ) );
            ASSERT_FALSE( StatsMayMatch( schema, CDataTablePredicate( "i32", CVariant( (Int64) 21 ), true, CVariant( (Int64) 30 ), true ), stats ) );
            ASSERT_FALSE( StatsMayMatch( schema, CDataTablePredicate( "i32", CVariant( (Int64) 0 ), true, CVariant( (Int64) 10 ), false ), stats ) );
            ASSERT_TRUE( StatsMayMatch( schema, CDataTablePredicate( "i32", CVariant( (Int64) 0 ), true, CVariant( (Int64) 10 ), true ), stats ) );
            ASSERT_TRUE( StatsMayMatch( schema, CDataTablePredicate( "i32", CVariant( (Int64) 12 ), true, CVariant( (Int64) 13 ), true ), stats ) );

            // IN: a member within [ min, max ]
            CVariant::VariantVector outside;
            outside.push_back( CVariant( (Int64) 1 ) );
            outside.push_back( CVariant( (Int64) 25 ) );
            ASSERT_FALSE( StatsMayMatch( schema, CDataTablePredicate( "i32", outside ), stats ) );
            CVariant::VariantVector inside( outside );
            inside.push_back( CVariant( (Int64) 20 ) );
            ASSERT_TRUE( StatsMayMatch( schema, CDataTablePredicate( "i32", inside ), stats ) );

            // null tests
            ASSERT_FALSE( StatsMayMatch( schema, CDataTablePredicate( "i32", DATATABLE_PREDICATE_IS_NULL ), noNulls ) );
            ASSERT_TRUE( StatsMayMatch( schema, CDataTablePredicate( "i32", DATATABLE_PREDICATE_IS_NULL ), stats ) );
            ASSERT_FALSE( StatsMayMatch( schema, CDataTablePredicate( "i32", DATATABLE_PREDICATE_IS_NOT_NULL ), allNull ) );
            ASSERT_TRUE( StatsMayMatch( schema, CDataTablePredicate( "i32", DATATABLE_PREDICATE_IS_NULL ), allNull ) );

            // unsigned statistics compare unsigned, text bytewise
            const CDataTableColumnStats unsignedStats = MakeStats( CVariant( (UInt64) 5 ), CVariant( (UInt64) 200 ), 0, 10 );
            ASSERT_TRUE( StatsMayMatch( schema, CDataTablePredicate( "u8", DATATABLE_PREDICATE_GREATER, CVariant( (Int64) 199 ) ), unsignedStats ) );
            ASSERT_FALSE( StatsMayMatch( schema, CDataTablePredicate( "u8", DATATABLE_PREDICATE_GREATER, CVariant( (Int64) 200 ) ), unsignedStats ) );
            const CDataTableColumnStats textStats = MakeStats( CVariant( CUtf8String( "KO" ) ), CVariant( CUtf8String( "XNAS" ) ), 0, 10 );
            ASSERT_FALSE( StatsMayMatch( schema, CDataTablePredicate( "s", DATATABLE_PREDICATE_EQUALS, CVariant( "XNYS" ) ), textStats ) );
            ASSERT_TRUE( StatsMayMatch( schema, CDataTablePredicate( "s", DATATABLE_PREDICATE_EQUALS, CVariant( "XNA" ) ), textStats ) );
            ASSERT_FALSE( StatsMayMatch( schema, CDataTablePredicate( "s", DATATABLE_PREDICATE_LESS, CVariant( "KO" ) ), textStats ) );

            // dates as date variants in the statistics
            const CDataTableColumnStats dateStats = MakeStats( CDataTableValues::Date( -10 ), CDataTableValues::Date( 10 ), 0, 10 );
            ASSERT_FALSE( StatsMayMatch( schema, CDataTablePredicate( "d", DATATABLE_PREDICATE_GREATER, CDataTableValues::Date( 10 ) ), dateStats ) );
            ASSERT_TRUE( StatsMayMatch( schema, CDataTablePredicate( "d", DATATABLE_PREDICATE_LESS, CDataTableValues::Date( -9 ) ), dateStats ) );

            // booleans
            const CDataTableColumnStats onlyFalse = MakeStats( CVariant( false ), CVariant( false ), 0, 10 );
            ASSERT_FALSE( StatsMayMatch( schema, CDataTablePredicate( "b", DATATABLE_PREDICATE_EQUALS, CVariant( true ) ), onlyFalse ) );
            ASSERT_TRUE( StatsMayMatch( schema, CDataTablePredicate( "b", DATATABLE_PREDICATE_EQUALS, CVariant( false ) ), onlyFalse ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: float statistics with NaN are not used, -0.0 and +0.0 bounds are equal, decimal statistics of a lower scale are rescaled" )
        try
        {
            CDataTableSchemaPtr schema = MakeResolveSchema();
            const Float64 nan = std::numeric_limits< Float64 >::quiet_NaN();
            const CDataTableColumnStats nanStats = MakeStats( CVariant( nan ), CVariant( (Float64) 1.0 ), 0, 10 );
            ASSERT_TRUE( StatsMayMatch( schema, CDataTablePredicate( "f64", DATATABLE_PREDICATE_GREATER, CVariant( (Float64) 100.0 ) ), nanStats ) );
            const CDataTableColumnStats zeros = MakeStats( CVariant( (Float64) -0.0 ), CVariant( (Float64) 0.0 ), 0, 10 );
            ASSERT_TRUE( StatsMayMatch( schema, CDataTablePredicate( "f64", DATATABLE_PREDICATE_EQUALS, CVariant( (Float64) 0.0 ) ), zeros ) );
            ASSERT_TRUE( StatsMayMatch( schema, CDataTablePredicate( "f64", DATATABLE_PREDICATE_EQUALS, CVariant( (Float64) -0.0 ) ), zeros ) );
            ASSERT_FALSE( StatsMayMatch( schema, CDataTablePredicate( "f64", DATATABLE_PREDICATE_GREATER, CVariant( (Float64) 0.0 ) ), zeros ) );
            const CDataTableColumnStats positive = MakeStats( CVariant( (Float64) 0.0 ), CVariant( (Float64) 3.0 ), 0, 10 );
            ASSERT_TRUE( StatsMayMatch( schema, CDataTablePredicate( "f64", DATATABLE_PREDICATE_LESS_OR_EQUAL, CVariant( (Float64) -0.0 ) ), positive ) );

            // file statistics of decimal(9,1) read for a table decimal(9,2): 1.5 at scale 1 is mantissa 15, 150 at scale 2
            CDataTableResolvedPredicate r;
            TDataTableStatus status = DATATABLE_STATUS_OK;
            ResolveOne( schema, CDataTablePredicate( "dec", DATATABLE_PREDICATE_GREATER, DecimalLiteral( "1.49" ) ), r, status );
            const CDataTableColumnStats fileStats = MakeStats( CVariant( (Int64) 10 ), CVariant( (Int64) 15 ), 0, 10 );
            const CDataTableColumnDef fileColumn = CDataTableColumnDef::Decimal( "dec", 9, 1 );
            ASSERT_TRUE( CDataTablePruner::MayMatch( r, schema->GetColumn( 5 ), fileStats, &fileColumn ) );
            ResolveOne( schema, CDataTablePredicate( "dec", DATATABLE_PREDICATE_GREATER, DecimalLiteral( "1.5" ) ), r, status );
            ASSERT_FALSE( CDataTablePruner::MayMatch( r, schema->GetColumn( 5 ), fileStats, &fileColumn ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: row group pruning maps table columns to file columns and prunes on columns a file lacks" )
        try
        {
            CDataTableSchemaPtr schema = MakeResolveSchema();
            CDataTableResolvedQuery resolved;
            resolved.schema = schema;
            CString error;
            ASSERT_TRUE( CDataTableQuery().WhereEquals( "i32", CVariant( (Int64) 30 ) ).Resolve( *schema, resolved, error ) );

            CDataTableRowGroupInfo info;
            info.rowCount = 10;
            info.columnStats.resize( 2 );
            info.columnStats[ 1 ] = MakeStats( CVariant( (Int64) 10 ), CVariant( (Int64) 20 ), 0, 10 );
            CDataTableSchema::TColumnIndexVector mapping( schema->GetColumnCount(), GUCEF_UINT32MAX );
            mapping[ 1 ] = 1;
            ASSERT_FALSE( CDataTablePruner::RowGroupMayMatch( resolved, info, mapping ) );
            info.columnStats[ 1 ] = MakeStats( CVariant( (Int64) 10 ), CVariant( (Int64) 40 ), 0, 10 );
            ASSERT_TRUE( CDataTablePruner::RowGroupMayMatch( resolved, info, mapping ) );

            // the file lacks i32: all nulls there, only IS_NULL may match
            mapping[ 1 ] = GUCEF_UINT32MAX;
            ASSERT_FALSE( CDataTablePruner::RowGroupMayMatch( resolved, info, mapping ) );
            CDataTableResolvedQuery isNull;
            isNull.schema = schema;
            CDataTableQuery().WhereNull( "i32", true ).Resolve( *schema, isNull, error );
            ASSERT_TRUE( CDataTablePruner::RowGroupMayMatch( isNull, info, mapping ) );

            // an empty row group never matches
            info.rowCount = 0;
            ASSERT_FALSE( CDataTablePruner::RowGroupMayMatch( isNull, info, CDataTableSchema::TColumnIndexVector() ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: MayMatch prunes partitions on identity values and on the periods of year, month and day fields" )
        try
        {
            CDataTableSchema schema;
            schema.AddColumn( CDataTableColumnDef( "mic", DATATABLE_TYPE_UTF8, false ) );
            schema.AddColumn( CDataTableColumnDef( "bucket", DATATABLE_TYPE_INT32, false ) );
            schema.AddColumn( CDataTableColumnDef( "date", DATATABLE_TYPE_DATE, false ) );
            schema.AddColumn( CDataTableColumnDef::Timestamp( "ts", DATATABLE_TIMEUNIT_MICROSECONDS, true, false ) );
            CDataTableSchemaPtr schemaPtr = CreateDataTableSchemaPtr( schema );
            CDataTablePartitionSpec spec;
            ASSERT_TRUE( spec.AddField( "mic", "mic", DATATABLE_PARTITION_TRANSFORM_IDENTITY ) );
            ASSERT_TRUE( spec.AddField( "bucket", "bucket", DATATABLE_PARTITION_TRANSFORM_IDENTITY ) );
            ASSERT_TRUE( spec.AddField( "year", "date", DATATABLE_PARTITION_TRANSFORM_YEAR ) );
            ASSERT_TRUE( spec.AddField( "month", "ts", DATATABLE_PARTITION_TRANSFORM_MONTH ) );
            ASSERT_TRUE( spec.SetPathTemplate( "{mic}/{bucket}/{year}/{month}.dtt" ) );

            // the keys as ParseSegment gives them: identity values as text, periods as their first day
            CValueList keys;
            ASSERT_TRUE( spec.ParseSegment( 0, "XNYS", keys ) );
            ASSERT_TRUE( spec.ParseSegment( 1, "17", keys ) );
            ASSERT_TRUE( spec.ParseSegment( 2, "2020", keys ) );
            ASSERT_TRUE( spec.ParseSegment( 3, "2021-02.dtt", keys ) );

            Int32 jan2020 = 0, dec2020 = 0, feb2021 = 0, mar2021 = 0;
            CDataTableValues::DaysFromCivil( 2020, 1, 1, jan2020 );
            CDataTableValues::DaysFromCivil( 2020, 12, 31, dec2020 );
            CDataTableValues::DaysFromCivil( 2021, 2, 1, feb2021 );
            CDataTableValues::DaysFromCivil( 2021, 3, 1, mar2021 );

            struct SCase { CDataTablePredicate predicate; bool mayMatch; };
            const Int64 usPerDay = 86400000000LL;
            SCase cases[] =
            {
                { CDataTablePredicate( "mic", DATATABLE_PREDICATE_EQUALS, CVariant( "XNYS" ) ), true },
                { CDataTablePredicate( "mic", DATATABLE_PREDICATE_EQUALS, CVariant( "XNAS" ) ), false },
                { CDataTablePredicate( "bucket", DATATABLE_PREDICATE_GREATER, CVariant( (Int64) 17 ) ), false },
                { CDataTablePredicate( "bucket", DATATABLE_PREDICATE_LESS_OR_EQUAL, CVariant( (Int64) 17 ) ), true },
                { CDataTablePredicate( "date", DATATABLE_PREDICATE_GREATER_OR_EQUAL, CDataTableValues::Date( dec2020 ) ), true },
                { CDataTablePredicate( "date", DATATABLE_PREDICATE_GREATER, CDataTableValues::Date( dec2020 ) ), false },
                { CDataTablePredicate( "date", DATATABLE_PREDICATE_LESS, CDataTableValues::Date( jan2020 ) ), false },
                { CDataTablePredicate( "date", DATATABLE_PREDICATE_NOT_EQUALS, CDataTableValues::Date( jan2020 ) ), true },
                { CDataTablePredicate( "ts", DATATABLE_PREDICATE_GREATER_OR_EQUAL, CDataTableValues::Timestamp( (Int64) mar2021 * usPerDay, DATATABLE_TIMEUNIT_MICROSECONDS ) ), false },
                { CDataTablePredicate( "ts", DATATABLE_PREDICATE_LESS, CDataTableValues::Timestamp( (Int64) mar2021 * usPerDay, DATATABLE_TIMEUNIT_MICROSECONDS ) ), true },
                { CDataTablePredicate( "ts", DATATABLE_PREDICATE_LESS, CDataTableValues::Timestamp( (Int64) feb2021 * usPerDay, DATATABLE_TIMEUNIT_MICROSECONDS ) ), false },
                { CDataTablePredicate( "ts", DATATABLE_PREDICATE_LESS_OR_EQUAL, CDataTableValues::Timestamp( (Int64) feb2021 * usPerDay, DATATABLE_TIMEUNIT_MICROSECONDS ) ), true },
                { CDataTablePredicate( "ts", DATATABLE_PREDICATE_EQUALS, CDataTableValues::Date( feb2021 + 27 ) ), true },
            };
            for ( UInt32 i=0; i<sizeof( cases ) / sizeof( cases[ 0 ] ); ++i )
            {
                CDataTableResolvedQuery resolved;
                resolved.schema = schemaPtr;
                CString error;
                ASSERT_TRUE( CDataTableQuery().Where( cases[ i ].predicate ).Resolve( schema, resolved, error ) );
                const bool mayMatch = spec.MayMatch( keys, resolved );
                if ( mayMatch != cases[ i ].mayMatch )
                    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "MayMatch case " << i << " gave " << mayMatch );
                ASSERT_TRUE( mayMatch == cases[ i ].mayMatch );
            }

            // keys a partition does not have never prune; a query matching nothing always does
            CDataTableResolvedQuery resolved;
            resolved.schema = schemaPtr;
            CString error;
            CDataTableQuery().WhereEquals( "mic", CVariant( "XNAS" ) ).Resolve( schema, resolved, error );
            ASSERT_TRUE( spec.MayMatch( CValueList(), resolved ) );
            CDataTableResolvedQuery nothing;
            nothing.schema = schemaPtr;
            CDataTableQuery().WhereEquals( "bucket", CVariant( (Float64) 0.5 ) ).Resolve( schema, nothing, error );
            ASSERT_FALSE( spec.MayMatch( CValueList(), nothing ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: the filter selects matching rows for every storage kind and leaves no selection when all rows match" )
        try
        {
            CDataTableBufferPool pool;
            CDataTableSchemaPtr schema = CORETEST::MakeAllTypesSchema();
            CDataTableBatchBuilder builder( schema, pool );
            const UInt32 b = 0, f32 = 9, f64 = 10, dec = 11, d = 12, s = 16, bin = 17;
            for ( UInt32 r=0; r<20; ++r )
            {
                for ( UInt32 c=0; c<schema->GetColumnCount(); ++c )
                {
                    if ( 3 == r % 5 )
                        builder.GetColumn( c ).AppendNull();
                    else
                    if ( c == b )   builder.GetColumn( c ).AppendBool( 0 == r % 2 );
                    else
                    if ( c == f32 ) builder.GetColumn( c ).AppendFloat64( (Float64) r * 0.5 );
                    else
                    if ( c == f64 ) builder.GetColumn( c ).AppendFloat64( 7 == r ? std::numeric_limits< Float64 >::quiet_NaN() : (Float64) r );
                    else
                    if ( c == dec ) builder.GetColumn( c ).AppendDecimalMantissa( (Int64) r * 10000 );
                    else
                    if ( c == d )   builder.GetColumn( c ).AppendDate( (Int32) r - 10 );
                    else
                    if ( DATATABLE_TYPE_TIMESTAMP == schema->GetColumn( c ).type ) builder.GetColumn( c ).AppendTimestamp( (Int64) r * 1000 );
                    else
                    if ( c == s )   { CString text = "t" + ToString( r % 4 ); builder.GetColumn( c ).AppendString( text ); }
                    else
                    if ( c == bin ) { UInt8 bytes[ 2 ] = { (UInt8) ( r % 3 ), 0 }; builder.GetColumn( c ).AppendBinary( bytes, 2 ); }
                    else
                    if ( DataTableLogicalTypeIsUnsigned( schema->GetColumn( c ).type ) ) builder.GetColumn( c ).AppendUInt64( r );
                    else            builder.GetColumn( c ).AppendInt64( r );
                }
            }
            CDataTableBatchPtr batch;
            CString error;
            ASSERT_TRUE( DATATABLE_STATUS_OK == builder.Build( batch, error ) );

            // rows 3, 8, 13 and 18 are null in every column
            GUCEF::vector< UInt32 > rows = FilterRows( schema, CDataTableQuery().WhereRange( "i64", CVariant( (Int64) 5 ), true, CVariant( (Int64) 9 ), false ), batch, pool );
            ASSERT_TRUE( 3 == rows.size() && 5 == rows[ 0 ] && 7 == rows[ 2 ] );
            batch.GetWritable()->SetSelection( GUCEF_NULL, 0, CDataTableBufferPtr() );
            rows = FilterRows( schema, CDataTableQuery().WhereEquals( "b", CVariant( true ) ), batch, pool );
            ASSERT_TRUE( 8 == rows.size() && 0 == rows[ 0 ] && 2 == rows[ 1 ] );
            batch.GetWritable()->SetSelection( GUCEF_NULL, 0, CDataTableBufferPtr() );
            rows = FilterRows( schema, CDataTableQuery().Where( CDataTablePredicate( "u16", DATATABLE_PREDICATE_GREATER, CVariant( (Int64) 16 ) ) ), batch, pool );
            ASSERT_TRUE( 2 == rows.size() && 17 == rows[ 0 ] && 19 == rows[ 1 ] );
            batch.GetWritable()->SetSelection( GUCEF_NULL, 0, CDataTableBufferPtr() );
            rows = FilterRows( schema, CDataTableQuery().Where( CDataTablePredicate( "f32", DATATABLE_PREDICATE_LESS_OR_EQUAL, CVariant( (Float64) 1.0 ) ) ), batch, pool );
            ASSERT_TRUE( 3 == rows.size() && 2 == rows[ 2 ] );
            batch.GetWritable()->SetSelection( GUCEF_NULL, 0, CDataTableBufferPtr() );
            rows = FilterRows( schema, CDataTableQuery().Where( CDataTablePredicate( "f64", DATATABLE_PREDICATE_NOT_EQUALS, CVariant( (Float64) 0.0 ) ) ), batch, pool );
            ASSERT_TRUE( 15 == rows.size() );   // the NaN row differs from 0.0, the nulls do not match
            batch.GetWritable()->SetSelection( GUCEF_NULL, 0, CDataTableBufferPtr() );
            rows = FilterRows( schema, CDataTableQuery().Where( CDataTablePredicate( "f64", DATATABLE_PREDICATE_GREATER, CVariant( (Float64) 5.0 ) ) ), batch, pool );
            ASSERT_TRUE( 10 == rows.size() );   // 6, 9..12, 14..17, 19: NaN is no match
            batch.GetWritable()->SetSelection( GUCEF_NULL, 0, CDataTableBufferPtr() );
            rows = FilterRows( schema, CDataTableQuery().WhereEquals( "dec", DecimalLiteral( "2.0000" ) ), batch, pool );
            ASSERT_TRUE( 1 == rows.size() && 2 == rows[ 0 ] );
            batch.GetWritable()->SetSelection( GUCEF_NULL, 0, CDataTableBufferPtr() );
            rows = FilterRows( schema, CDataTableQuery().Where( CDataTablePredicate( "d", DATATABLE_PREDICATE_LESS, CDataTableValues::Date( -8 ) ) ), batch, pool );
            ASSERT_TRUE( 2 == rows.size() && 1 == rows[ 1 ] );
            batch.GetWritable()->SetSelection( GUCEF_NULL, 0, CDataTableBufferPtr() );
            rows = FilterRows( schema, CDataTableQuery().Where( CDataTablePredicate( "ts", DATATABLE_PREDICATE_GREATER_OR_EQUAL, CDataTableValues::Timestamp( 19, DATATABLE_TIMEUNIT_MILLISECONDS ) ) ), batch, pool );
            ASSERT_TRUE( 1 == rows.size() && 19 == rows[ 0 ] );
            batch.GetWritable()->SetSelection( GUCEF_NULL, 0, CDataTableBufferPtr() );
            CVariant::VariantVector words;
            words.push_back( CVariant( "t1" ) );
            words.push_back( CVariant( "t2" ) );
            rows = FilterRows( schema, CDataTableQuery().WhereIn( "s", words ), batch, pool );
            ASSERT_TRUE( 8 == rows.size() && 1 == rows[ 0 ] && 2 == rows[ 1 ] );
            batch.GetWritable()->SetSelection( GUCEF_NULL, 0, CDataTableBufferPtr() );
            UInt8 one[ 2 ] = { 1, 0 };
            CVariant blob;
            blob.Set( one, 2, GUCEF_DATATYPE_BINARY_BLOB, false );
            rows = FilterRows( schema, CDataTableQuery().WhereEquals( "bin", blob ), batch, pool );
            ASSERT_TRUE( 6 == rows.size() && 1 == rows[ 0 ] );
            batch.GetWritable()->SetSelection( GUCEF_NULL, 0, CDataTableBufferPtr() );

            // conjunctions narrow; null tests; everything matching leaves no selection
            rows = FilterRows( schema, CDataTableQuery().WhereNull( "i64", true ), batch, pool );
            ASSERT_TRUE( 4 == rows.size() && 3 == rows[ 0 ] );
            batch.GetWritable()->SetSelection( GUCEF_NULL, 0, CDataTableBufferPtr() );
            rows = FilterRows( schema, CDataTableQuery().WhereNull( "i64", false ).WhereRange( "i64", CVariant( (Int64) 0 ), true, CVariant( (Int64) 4 ), true ), batch, pool );
            ASSERT_TRUE( 4 == rows.size() );
            batch.GetWritable()->SetSelection( GUCEF_NULL, 0, CDataTableBufferPtr() );
            rows = FilterRows( schema, CDataTableQuery().Where( CDataTablePredicate( "i64", DATATABLE_PREDICATE_LESS, CVariant( (Int64) 1000 ) ) ), batch, pool );
            ASSERT_TRUE( 16 == rows.size() && GUCEF_NULL != batch->GetSelection() );
            batch.GetWritable()->SetSelection( GUCEF_NULL, 0, CDataTableBufferPtr() );

            CDataTableSchema noNulls;
            noNulls.AddColumn( CDataTableColumnDef( "v", DATATABLE_TYPE_INT32, false ) );
            CDataTableSchemaPtr noNullsPtr = CreateDataTableSchemaPtr( noNulls );
            CDataTableBatchBuilder plain( noNullsPtr, pool );
            for ( Int32 i=0; i<10; ++i )
                plain.GetColumn( 0 ).AppendInt64( i );
            CDataTableBatchPtr plainBatch;
            plain.Build( plainBatch, error );
            rows = FilterRows( noNullsPtr, CDataTableQuery().Where( CDataTablePredicate( "v", DATATABLE_PREDICATE_GREATER_OR_EQUAL, CVariant( (Int64) 0 ) ) ), plainBatch, pool );
            ASSERT_TRUE( 10 == rows.size() && GUCEF_NULL == plainBatch->GetSelection() );
            rows = FilterRows( noNullsPtr, CDataTableQuery().WhereNull( "v", false ), plainBatch, pool );
            ASSERT_TRUE( 10 == rows.size() && GUCEF_NULL == plainBatch->GetSelection() );

            // a guaranteed predicate is not tested again
            plainBatch.GetWritable()->SetGuaranteedPredicates( 1 );
            rows = FilterRows( noNullsPtr, CDataTableQuery().Where( CDataTablePredicate( "v", DATATABLE_PREDICATE_GREATER, CVariant( (Int64) 100 ) ) ), plainBatch, pool );
            ASSERT_TRUE( 10 == rows.size() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: dictionary columns are filtered through a bitmap over the dictionary" )
        try
        {
            CDataTableBufferPool pool;
            CDataTableSchemaPtr schema = MakeResolveSchema();
            const char* dictionary[] = { "AAPL", "KO", "MSFT", "XOM" };
            const UInt32 indices[] = { 1, 0, 3, 1, 2, 2, 0, 1, 3, 1 };
            const bool nulls[] = { false, false, false, true, false, false, false, false, false, false };
            CDataTableBatchPtr batch = MakeDictionaryBatch( pool, schema, dictionary, 4, indices, nulls, 10 );
            CString error;
            ASSERT_TRUE( batch->GetColumn( 0 ).Validate( error ) );

            GUCEF::vector< UInt32 > rows = FilterRows( schema, CDataTableQuery().WhereEquals( "s", CVariant( "KO" ) ), batch, pool );
            ASSERT_TRUE( 3 == rows.size() && 0 == rows[ 0 ] && 7 == rows[ 1 ] && 9 == rows[ 2 ] );
            batch = MakeDictionaryBatch( pool, schema, dictionary, 4, indices, nulls, 10 );
            rows = FilterRows( schema, CDataTableQuery().Where( CDataTablePredicate( "s", DATATABLE_PREDICATE_GREATER_OR_EQUAL, CVariant( "MSFT" ) ) ), batch, pool );
            ASSERT_TRUE( 4 == rows.size() && 2 == rows[ 0 ] && 4 == rows[ 1 ] );
            batch = MakeDictionaryBatch( pool, schema, dictionary, 4, indices, nulls, 10 );
            rows = FilterRows( schema, CDataTableQuery().WhereEquals( "s", CVariant( "IBM" ) ), batch, pool );
            ASSERT_TRUE( rows.empty() );
            batch = MakeDictionaryBatch( pool, schema, dictionary, 4, indices, nulls, 10 );
            rows = FilterRows( schema, CDataTableQuery().Where( CDataTablePredicate( "s", DATATABLE_PREDICATE_NOT_EQUALS, CVariant( "KO" ) ) ), batch, pool );
            ASSERT_TRUE( 6 == rows.size() );

            // one filter over two batches of one dictionary, then one of another dictionary at maybe the same address
            CDataTableResolvedQuery resolved;
            resolved.schema = schema;
            ASSERT_TRUE( CDataTableQuery().WhereEquals( "s", CVariant( "XOM" ) ).Resolve( *schema, resolved, error ) );
            CDataTableSchema::TColumnIndexVector columns;
            columns.push_back( (UInt32) schema->FindColumn( "s" ) );
            CDataTableFilter filter;
            ASSERT_TRUE( DATATABLE_STATUS_OK == filter.Prepare( resolved, columns, error ) );
            UInt32 selected = 0;
            CDataTableBatchPtr first = MakeDictionaryBatch( pool, schema, dictionary, 4, indices, nulls, 10 );
            ASSERT_TRUE( DATATABLE_STATUS_OK == filter.Apply( *first.GetWritable(), pool, selected ) && 2 == selected );
            first.Reset();
            const char* other[] = { "XOM", "KO" };
            const UInt32 otherIndices[] = { 0, 1, 0, 0, 1, 1, 1, 1, 1, 1 };
            CDataTableBatchPtr second = MakeDictionaryBatch( pool, schema, other, 2, otherIndices, nulls, 10 );
            ASSERT_TRUE( DATATABLE_STATUS_OK == filter.Apply( *second.GetWritable(), pool, selected ) && 2 == selected );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: NOT_EQUALS on a float column returns the NaN rows of a row group whose other values all equal the literal" )
        try
        {
            // f64 { 5.0, NaN, 1.0, 2.0 } in row groups of two: the statistics of the first group are min = max = 5.0 (NaN is
            // left out of minimum and maximum), yet its NaN cell is not equal to 5.0 and matches
            for ( UInt32 width=0; width<2; ++width )
            {
                const bool is32 = 1 == width;
                const char* name = is32 ? "f32" : "f64";
                CDataTableBufferPool pool;
                CDataTableDefinition definition;
                definition.name = "nan";
                definition.schema.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, false ) );
                definition.schema.AddColumn( CDataTableColumnDef( name, is32 ? DATATABLE_TYPE_FLOAT32 : DATATABLE_TYPE_FLOAT64 ) );
                definition.storage.rowGroupRowCount = 2;
                definition.storage.pageRowCount = 2;
                CMemoryDataTable table( definition, &pool );
                CDataTableBatchBuilder builder( table.GetSchema(), pool );
                const Float64 nan = std::numeric_limits< Float64 >::quiet_NaN();
                const Float64 values[ 4 ] = { 5.0, nan, 1.0, 2.0 };
                for ( UInt32 r=0; r<4; ++r )
                {
                    CVariant cells[ 2 ];
                    cells[ 0 ] = CVariant( (Int64) r );
                    cells[ 1 ] = is32 ? CVariant( (Float32) values[ r ] ) : CVariant( values[ r ] );
                    ASSERT_TRUE( builder.AppendRow( cells, 2 ) );
                }
                CDataTableBatchPtr rows;
                CString error;
                ASSERT_TRUE( DATATABLE_STATUS_OK == builder.Build( rows, error ) );
                CDataTableWriteResult writeResult;
                ASSERT_TRUE( DATATABLE_STATUS_OK == table.Write( *rows, DATATABLE_WRITE_INSERT, writeResult ) );

                CDataTableScanPtr scan;
                ASSERT_TRUE( DATATABLE_STATUS_OK == table.Scan( CDataTableQuery().Where( CDataTablePredicate( name, DATATABLE_PREDICATE_NOT_EQUALS, CVariant( (Float64) 5.0 ) ) ), scan ) );
                UInt32 matched = 0;
                bool sawNaNRow = false;
                CDataTableBatchPtr batch;
                while ( DATATABLE_STATUS_OK == scan->NextBatch( batch ) )
                {
                    const UInt32* selection = batch->GetSelection();
                    for ( UInt32 i=0; i<batch->GetSelectedRowCount(); ++i )
                    {
                        ++matched;
                        sawNaNRow = sawNaNRow || 1 == batch->GetColumn( 0 ).GetInt64Data()[ GUCEF_NULL == selection ? i : selection[ i ] ];
                    }
                }
                ASSERT_TRUE( 3 == matched );
                ASSERT_TRUE( sawNaNRow );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: statistics of a float column that may hold NaN never prune NOT_EQUALS by min == max, and no other operator is affected" )
        try
        {
            CDataTableSchemaPtr schema = MakeResolveSchema();
            const CDataTablePredicate notFive( "f64", DATATABLE_PREDICATE_NOT_EQUALS, CVariant( (Float64) 5.0 ) );

            // statistics of another writer: min = max = 5.0 and nothing said about NaN
            CDataTableColumnStats external = MakeStats( CVariant( (Float64) 5.0 ), CVariant( (Float64) 5.0 ), 0, 4 );
            ASSERT_TRUE( StatsMayMatch( schema, notFive, external ) );

            // equal to the literal and a statement that there is no NaN: nothing in the unit differs, pruned
            CDataTableColumnStats noNaN = external;
            noNaN.hasNanCount = true;
            noNaN.nanCount = 0;
            ASSERT_FALSE( StatsMayMatch( schema, notFive, noNaN ) );

            // NaN counted: the NaN cells match
            CDataTableColumnStats withNaN = external;
            withNaN.hasNanCount = true;
            withNaN.nanCount = 1;
            ASSERT_TRUE( StatsMayMatch( schema, notFive, withNaN ) );

            // the other operators are decided by the bounds, NaN cells match none of them
            const CDataTablePredicate* others[ 5 ];
            const CDataTablePredicate equalsSix( "f64", DATATABLE_PREDICATE_EQUALS, CVariant( (Float64) 6.0 ) );
            const CDataTablePredicate greater( "f64", DATATABLE_PREDICATE_GREATER, CVariant( (Float64) 5.0 ) );
            const CDataTablePredicate less( "f64", DATATABLE_PREDICATE_LESS, CVariant( (Float64) 5.0 ) );
            CVariant::VariantVector members;
            members.push_back( CVariant( (Float64) 6.0 ) );
            const CDataTablePredicate in( "f64", members );
            const CDataTablePredicate range( "f64", CVariant( (Float64) 6.0 ), true, CVariant( (Float64) 7.0 ), true );
            others[ 0 ] = &equalsSix; others[ 1 ] = &greater; others[ 2 ] = &less; others[ 3 ] = &in; others[ 4 ] = &range;
            for ( UInt32 i=0; i<5; ++i )
            {
                ASSERT_FALSE( StatsMayMatch( schema, *others[ i ], external ) );
                ASSERT_FALSE( StatsMayMatch( schema, *others[ i ], withNaN ) );
            }

            // a float32 column and a NaN literal behave the same
            CDataTableColumnStats single32 = MakeStats( CVariant( (Float64) 5.0 ), CVariant( (Float64) 5.0 ), 0, 4 );
            ASSERT_TRUE( StatsMayMatch( schema, CDataTablePredicate( "f32", DATATABLE_PREDICATE_NOT_EQUALS, CVariant( (Float64) 5.0 ) ), single32 ) );
            ASSERT_TRUE( StatsMayMatch( schema, CDataTablePredicate( "f64", DATATABLE_PREDICATE_NOT_EQUALS, CVariant( std::numeric_limits< Float64 >::quiet_NaN() ) ), noNaN ) );

            // integers, text and the rest are untouched by the NaN rule
            CDataTableColumnStats ints = MakeStats( CVariant( (Int64) 5 ), CVariant( (Int64) 5 ), 0, 4 );
            ASSERT_FALSE( StatsMayMatch( schema, CDataTablePredicate( "i32", DATATABLE_PREDICATE_NOT_EQUALS, CVariant( (Int64) 5 ) ), ints ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: the statistics of a memory table row group count the NaN cells of float columns and leave them out of the bounds" )
        try
        {
            CDataTableBufferPool pool;
            CDataTableSchema schema;
            schema.AddColumn( CDataTableColumnDef( "f32", DATATABLE_TYPE_FLOAT32 ) );
            schema.AddColumn( CDataTableColumnDef( "f64", DATATABLE_TYPE_FLOAT64 ) );
            schema.AddColumn( CDataTableColumnDef( "i", DATATABLE_TYPE_INT32 ) );
            CDataTableSchemaPtr schemaPtr = CreateDataTableSchemaPtr( schema );
            CDataTableBatchBuilder builder( schemaPtr, pool );
            const Float64 nan = std::numeric_limits< Float64 >::quiet_NaN();
            CVariant rows[ 4 ][ 3 ] = {
                { CVariant( (Float32) 5.0f ), CVariant( (Float64) 5.0 ), CVariant( (Int64) 1 ) },
                { CVariant( std::numeric_limits< Float32 >::quiet_NaN() ), CVariant( nan ), CVariant( (Int64) 2 ) },
                { CVariant(), CVariant(), CVariant() },
                { CVariant( std::numeric_limits< Float32 >::quiet_NaN() ), CVariant( (Float64) 5.0 ), CVariant( (Int64) 3 ) } };
            for ( UInt32 r=0; r<4; ++r )
                ASSERT_TRUE( builder.AppendRow( rows[ r ], 3 ) );
            CDataTableBatchPtr batch;
            CString error;
            ASSERT_TRUE( DATATABLE_STATUS_OK == builder.Build( batch, error ) );
            CDataTableRowGroupInfo info;
            CDataTableMerge::ComputeStats( *batch, info );
            ASSERT_TRUE( 3 == info.columnStats.size() );
            ASSERT_TRUE( info.columnStats[ 0 ].hasNanCount && 2 == info.columnStats[ 0 ].nanCount );
            ASSERT_TRUE( info.columnStats[ 1 ].hasNanCount && 1 == info.columnStats[ 1 ].nanCount );
            ASSERT_TRUE( !info.columnStats[ 2 ].hasNanCount );
            ASSERT_TRUE( info.columnStats[ 0 ].hasMinMax && 5.0 == info.columnStats[ 0 ].minValue.AsFloat64() && 5.0 == info.columnStats[ 0 ].maxValue.AsFloat64() );
            ASSERT_TRUE( info.columnStats[ 1 ].hasMinMax && 5.0 == info.columnStats[ 1 ].minValue.AsFloat64() && 5.0 == info.columnStats[ 1 ].maxValue.AsFloat64() );
            ASSERT_TRUE( 1 == info.columnStats[ 0 ].nullCount );

            // the counts take part in the comparison of statistics
            CDataTableColumnStats copy = info.columnStats[ 1 ];
            ASSERT_TRUE( copy == info.columnStats[ 1 ] );
            copy.nanCount = 0;
            ASSERT_FALSE( copy == info.columnStats[ 1 ] );
            copy.Clear();
            ASSERT_FALSE( copy.hasNanCount );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: property: row groups of one or two values with nulls, NaN, infinities and zeros of both signs: a pruned scan returns exactly the rows of a brute force filter" )
        try
        {
            UInt64 pruned = 0;
            static const UInt64 seeds[] = { 5, 6, 7, 11, 99, 424242, 0xC0FFEEULL, 31337, 8080, 123 };
            for ( UInt32 s=0; s<sizeof( seeds ) / sizeof( seeds[ 0 ] ); ++s )
            {
                CString failure;
                const bool ok = RunPruningProperty( seeds[ s ], 0 != ( s & 1 ), 60, pruned, failure, true );
                if ( !ok )
                    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "Narrow pruning property: " << failure );
                ASSERT_TRUE( ok );
            }
            ASSERT_TRUE( pruned > 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: property: a pruned and filtered scan returns exactly the rows of a brute force filter (fixed seeds)" )
        try
        {
            UInt64 pruned = 0;
            static const UInt64 seeds[] = { 1, 2, 3, 42, 1234567, 987654321, 0xDEADBEEFULL, 77 };
            for ( UInt32 s=0; s<sizeof( seeds ) / sizeof( seeds[ 0 ] ); ++s )
            {
                CString failure;
                const bool ok = RunPruningProperty( seeds[ s ], 0 != ( s & 1 ), 40, pruned, failure );
                if ( !ok )
                    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "Pruning property: " << failure );
                ASSERT_TRUE( ok );
            }
            // the sorted tables give the statistics something to prune
            ASSERT_TRUE( pruned > 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
