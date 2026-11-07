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
#include <algorithm>
#include <limits>

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CTABLECODECREGISTRY_H
#include "gucefCORE_CTableCodecRegistry.h"
#define GUCEF_CORE_CTABLECODECREGISTRY_H
#endif /* GUCEF_CORE_CTABLECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CMEMORYDATATABLE_H
#include "gucefCORE_CMemoryDataTable.h"
#define GUCEF_CORE_CMEMORYDATATABLE_H
#endif /* GUCEF_CORE_CMEMORYDATATABLE_H ? */

#ifndef GUCEF_CORE_CDATATABLEMERGE_H
#include "gucefCORE_CDataTableMerge.h"
#define GUCEF_CORE_CDATATABLEMERGE_H
#endif /* GUCEF_CORE_CDATATABLEMERGE_H ? */

#ifndef GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#include "gucefCORE_CDataTableBufferPool.h"
#define GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#endif /* GUCEF_CORE_CDATATABLEBUFFERPOOL_H ? */

#ifndef GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#include "gucefCORE_CDataTableBatchBuilder.h"
#define GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#endif /* GUCEF_CORE_CDATATABLEBATCHBUILDER_H ? */

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

#ifndef TABLECODECPLUGIN_PARQUET_CPARQUETFILEMETADATA_H
#include "tablecodecpluginPARQUET_CParquetFileMetaData.h"
#define TABLECODECPLUGIN_PARQUET_CPARQUETFILEMETADATA_H
#endif /* TABLECODECPLUGIN_PARQUET_CPARQUETFILEMETADATA_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_CPARQUETTABLEWRITER_H
#include "tablecodecpluginPARQUET_CParquetTableWriter.h"
#define TABLECODECPLUGIN_PARQUET_CPARQUETTABLEWRITER_H
#endif /* TABLECODECPLUGIN_PARQUET_CPARQUETTABLEWRITER_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_CPARQUETTABLECODEC_H
#include "tablecodecpluginPARQUET_CParquetTableCodec.h"
#define TABLECODECPLUGIN_PARQUET_CPARQUETTABLECODEC_H
#endif /* TABLECODECPLUGIN_PARQUET_CPARQUETTABLECODEC_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_CPARQUETSCHEMAMAPPING_H
#include "tablecodecpluginPARQUET_CParquetSchemaMapping.h"
#define TABLECODECPLUGIN_PARQUET_CPARQUETSCHEMAMAPPING_H
#endif /* TABLECODECPLUGIN_PARQUET_CPARQUETSCHEMAMAPPING_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

/* the random cells and the brute force predicate rules of the gucefCORE data table tests */
#include "../../gucefCORE_TestApp/include/TestDataTableScanSupport.h"

#include "TestAllocationCounter.h"
#include "TestParquetSupport.h"
#include "TestParquetCodec.h"

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
using namespace GUCEF::TABLECODECPLUGIN::PARQUET;
using namespace PARQUETTEST;

namespace {

/** The parsed footer of a file held in memory */
const CParquetFileMetaData*
ParseFooter( const CDynamicBuffer& file, CITableFileMetaDataPtr& holder )
{
    IOAccessPtr input = MakeReadAccess( file );
    CString error;
    if ( DATATABLE_STATUS_OK != GetParquetCodec()->ReadMetaData( *input, CValueList(), holder, error ) )
        return GUCEF_NULL;
    return static_cast< const CParquetFileMetaData* >( holder.GetPointerAlways() );
}

/** Rows [ first, first + count ) of a full batch as a batch of their own */
CDataTableBatchPtr
SliceRows( const CDataTableBatchPtr& source, UInt32 first, UInt32 count )
{
    CDataTableBufferPool& pool = CCoreGlobal::Instance()->GetDataTableBufferPool();
    CDataTableBatchPtr slice;
    pool.AcquireBatch( slice );
    CDataTableBatch* writable = slice.GetWritable();
    writable->SetColumnCount( source->GetColumnCount() );
    writable->SetSchema( source->GetSchema() );
    writable->SetRowCount( count );
    for ( UInt32 c=0; c<source->GetColumnCount(); ++c )
    {
        writable->SetSchemaColumnIndex( c, c );
        writable->GetMutableColumn( c ).SetSlice( source->GetColumn( c ), first, count, pool );
    }
    return slice;
}

/** Statistics are the same: presence, bounds in the native order, counts */
bool
SameStats( TDataTableLogicalType type, const CDataTableColumnStats& expected, const CDataTableColumnStats& actual, CString& difference )
{
    if ( expected.hasMinMax != actual.hasMinMax )
    {
        difference = "min / max presence differs";
        return false;
    }
    if ( expected.hasMinMax &&
         ( 0 != CDataTablePruner::CompareNative( type, expected.minValue, actual.minValue ) || 0 != CDataTablePruner::CompareNative( type, expected.maxValue, actual.maxValue ) ) )
    {
        difference = "bounds differ: expected " + expected.minValue.AsString() + " .. " + expected.maxValue.AsString() + ", got " + actual.minValue.AsString() + " .. " + actual.maxValue.AsString();
        return false;
    }
    if ( !actual.hasNullCount || expected.nullCount != actual.nullCount || expected.valueCount != actual.valueCount )
    {
        difference = "counts differ";
        return false;
    }
    if ( expected.hasNanCount != actual.hasNanCount || ( expected.hasNanCount && expected.nanCount != actual.nanCount ) )
    {
        difference = "NaN counts differ";
        return false;
    }
    return true;
}

/** Where the bytes first occur in the file, -1 when they do not */
Int64
FindInFile( const CDynamicBuffer& file, const void* bytes, UInt32 length )
{
    const UInt8* data = static_cast< const UInt8* >( file.GetConstBufferPtr() );
    const UInt32 size = file.GetDataSize();
    for ( UInt32 i=0; i+length<=size; ++i )
    {
        if ( data[ i ] == *static_cast< const UInt8* >( bytes ) && 0 == memcmp( data + i, bytes, length ) )
            return i;
    }
    return -1;
}

/** A random predicate on a column with literals from the domains of CORETEST::RandomCellValue */
CDataTablePredicate
RandomPredicate( CORETEST::CTestRandom& random, const CDataTableColumnDef& column )
{
    const UInt32 kind = random.Below( 12 );
    CVariant a = CORETEST::RandomCellValue( random, column, 0 );
    CVariant b = CORETEST::RandomCellValue( random, column, 0 );
    switch ( kind )
    {
        case 0:  { return CDataTablePredicate( column.name, DATATABLE_PREDICATE_EQUALS, a ); }
        case 1:  { return CDataTablePredicate( column.name, DATATABLE_PREDICATE_NOT_EQUALS, a ); }
        case 2:  { return CDataTablePredicate( column.name, DATATABLE_PREDICATE_LESS, a ); }
        case 3:  { return CDataTablePredicate( column.name, DATATABLE_PREDICATE_LESS_OR_EQUAL, a ); }
        case 4:  { return CDataTablePredicate( column.name, DATATABLE_PREDICATE_GREATER, a ); }
        case 5:  { return CDataTablePredicate( column.name, DATATABLE_PREDICATE_GREATER_OR_EQUAL, a ); }
        case 6:  { return CDataTablePredicate( column.name, a, random.Chance( 50 ), b, random.Chance( 50 ) ); }
        case 8:
        {
            CVariant::VariantVector values;
            const UInt32 count = 1 + random.Below( 4 );
            for ( UInt32 i=0; i<count; ++i )
                values.push_back( CORETEST::RandomCellValue( random, column, 0 ) );
            return CDataTablePredicate( column.name, values );
        }
        case 9:  { return CDataTablePredicate( column.name, DATATABLE_PREDICATE_IS_NULL ); }
        case 10: { return CDataTablePredicate( column.name, DATATABLE_PREDICATE_IS_NOT_NULL ); }
        default: { return CDataTablePredicate( column.name, DATATABLE_PREDICATE_EQUALS, a ); }
    }
}

/** Every row a scan of the table delivers for the query, sorted, as text */
TDataTableStatus
ScanRows( CIDataTable& table, const CDataTableQuery& query, CString::StringVector& rows, CDataTableScanStats* stats = GUCEF_NULL )
{
    rows.clear();
    CDataTableScanPtr scan;
    TDataTableStatus status = table.Scan( query, scan );
    if ( DATATABLE_STATUS_OK != status )
        return status;
    status = CORETEST::CollectScanRows( *scan, rows );
    if ( GUCEF_NULL != stats )
        scan->GetStats( *stats );
    std::sort( rows.begin(), rows.end() );
    return DATATABLE_STATUS_END == status ? DATATABLE_STATUS_OK : status;
}

const char* const ModuleGroup = "ParquetModuleTest";

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformParquetCodecTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "tablecodecpluginPARQUET.Codec" );

    GUCEF_TESTFW_TESTCASE( "Codec: every type round trips, nullable and not, at 0, 35 and 100 % nulls, plain and dictionary, uncompressed and GZIP, batches crossing pages" )
    try
    {
        const TDataTableDictionaryMode modes[] = { DATATABLE_DICTIONARY_AUTO, DATATABLE_DICTIONARY_ALWAYS, DATATABLE_DICTIONARY_NEVER };
        const UInt32 nullPercents[] = { 0, 35, 100 };
        const char* compressions[] = { "none", "gzip" };
        UInt32 combinations = 0;
        for ( UInt32 nullable=0; nullable<2; ++nullable )
        {
            const CDataTableSchemaPtr schema = CreateDataTableSchemaPtr( MakeAllTypesSchema( 0 != nullable ) );
            for ( UInt32 n=0; n<3; ++n )
            {
                if ( 0 == nullable && 0 != nullPercents[ n ] )
                    continue;
                for ( UInt32 m=0; m<3; ++m )
                {
                    for ( UInt32 z=0; z<2; ++z )
                    {
                        if ( 1 == z && !IsGzipAvailable() )
                            continue;
                        CTestRandom random( 100 + n * 10 + m );
                        TBatchVector batches;
                        batches.push_back( MakeRandomBatch( schema, 2000, nullPercents[ n ], random, 0 == m ? 40 : 0 ) );
                        batches.push_back( MakeRandomBatch( schema, 1111, nullPercents[ n ], random, 0 == m ? 40 : 0 ) );
                        CDataTableWriterOptions options;
                        options.dictionary = modes[ m ];
                        options.compression = compressions[ z ];
                        options.pageRowCount = 700;
                        CDynamicBuffer file;
                        CString error;
                        const TDataTableStatus written = WriteParquet( schema, batches, options, file, error );
                        ASSERT_TRUE( DATATABLE_STATUS_OK == written );
                        if ( DATATABLE_STATUS_OK != written )
                        {
                            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "Parquet round trip: writing failed: " << error );
                            continue;
                        }
                        const UInt32 batchSizes[] = { 0, 1000, 333 };
                        for ( UInt32 b=0; b<3; ++b )
                        {
                            TBatchVector read;
                            const bool materialize = 2 == b;
                            ASSERT_TRUE( DATATABLE_STATUS_OK == ReadParquet( file, read, error, GUCEF_NULL, batchSizes[ b ], GUCEF_NULL, materialize ) );
                            CString difference;
                            const bool same = SameRows( batches, read, difference );
                            if ( !same )
                                GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "Parquet round trip: nullable " << nullable << " nulls " << nullPercents[ n ] << " mode " << m << " compression " << z << " batch " << batchSizes[ b ] << ": " << difference );
                            ASSERT_TRUE( same );
                            for ( size_t k=0; k<read.size(); ++k )
                            {
                                for ( UInt32 c=0; c<read[ k ]->GetColumnCount(); ++c )
                                {
                                    CString invalid;
                                    const bool valid = read[ k ]->GetColumn( c ).Validate( invalid );
                                    if ( !valid )
                                        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "Parquet round trip: column " << c << " not valid: " << invalid );
                                    ASSERT_TRUE( valid );
                                    if ( materialize )
                                        ASSERT_TRUE( DATATABLE_STORAGE_DICTIONARY != read[ k ]->GetColumn( c ).GetStorage() );
                                }
                            }
                        }
                        ++combinations;
                    }
                }
            }
        }
        ASSERT_TRUE( combinations >= 12 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: an empty file, one row, many row groups through flushes and size limits, a selection written, and the origins of the batches read" )
    try
    {
        const CDataTableSchemaPtr schema = CreateDataTableSchemaPtr( MakeAllTypesSchema( true ) );
        CDataTableWriterOptions options;
        CString error;

        // no rows at all, and a batch of none
        TBatchVector none;
        CDynamicBuffer file;
        CDataTableFileSummary summary;
        ASSERT_TRUE( DATATABLE_STATUS_OK == WriteParquet( schema, none, options, file, error, false, &summary ) );
        ASSERT_TRUE( 0 == summary.rowCount && 0 == summary.rowGroupCount && summary.byteCount == file.GetDataSize() );
        CITableFileMetaDataPtr holder;
        const CParquetFileMetaData* footer = ParseFooter( file, holder );
        ASSERT_TRUE( GUCEF_NULL != footer && 0 == footer->GetRowCount() && 0 == footer->GetRowGroupCount() );
        ASSERT_TRUE( GUCEF_NULL != footer && schema->GetColumnCount() == footer->GetSchema()->GetColumnCount() );
        CTestRandom random( 5 );
        none.push_back( MakeRandomBatch( schema, 0, 0, random ) );
        ASSERT_TRUE( DATATABLE_STATUS_OK == WriteParquet( schema, none, options, file, error ) );
        TBatchVector read;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadParquet( file, read, error ) && read.empty() );

        // one row
        TBatchVector one;
        one.push_back( MakeRandomBatch( schema, 1, 0, random ) );
        ASSERT_TRUE( DATATABLE_STATUS_OK == WriteParquet( schema, one, options, file, error ) );
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadParquet( file, read, error ) && 1 == read.size() );
        CString difference;
        ASSERT_TRUE( SameRows( one, read, difference ) );

        // row groups by row count: 3500 rows in groups of 1000
        TBatchVector many;
        many.push_back( MakeRandomBatch( schema, 3500, 20, random ) );
        options.rowGroupRowCount = 1000;
        options.pageRowCount = 300;
        ASSERT_TRUE( DATATABLE_STATUS_OK == WriteParquet( schema, many, options, file, error, false, &summary ) );
        ASSERT_TRUE( 4 == summary.rowGroupCount && 3500 == summary.rowCount );
        footer = ParseFooter( file, holder );
        ASSERT_TRUE( GUCEF_NULL != footer && 4 == footer->GetRowGroupCount() && 3500 == footer->GetRowCount() );
        CDataTableRowGroupInfo info;
        ASSERT_TRUE( GUCEF_NULL != footer && footer->GetRowGroupInfo( 3, info ) && 500 == info.rowCount && !footer->GetRowGroupInfo( 4, info ) );
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadParquet( file, read, error, GUCEF_NULL, 512 ) );
        ASSERT_TRUE( SameRows( many, read, difference ) );
        // origins: row group and first row within it
        UInt32 expectedGroup = 0;
        UInt64 expectedFirst = 0;
        for ( size_t i=0; i<read.size(); ++i )
        {
            const CDataTableBatchOrigin& origin = read[ i ]->GetOrigin();
            if ( origin.rowGroup != expectedGroup )
            {
                expectedGroup = origin.rowGroup;
                expectedFirst = 0;
            }
            ASSERT_TRUE( origin.firstRowInRowGroup == expectedFirst );
            expectedFirst += read[ i ]->GetRowCount();
        }
        ASSERT_TRUE( 3 == expectedGroup );

        // row groups by flushes: one per batch
        TBatchVector parts;
        parts.push_back( SliceRows( many[ 0 ], 0, 10 ) );
        parts.push_back( SliceRows( many[ 0 ], 10, 1 ) );
        parts.push_back( SliceRows( many[ 0 ], 11, 2000 ) );
        options.rowGroupRowCount = 1048576;
        ASSERT_TRUE( DATATABLE_STATUS_OK == WriteParquet( schema, parts, options, file, error, true, &summary ) );
        ASSERT_TRUE( 3 == summary.rowGroupCount && 2011 == summary.rowCount );
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadParquet( file, read, error ) && SameRows( parts, read, difference ) );

        // row groups by bytes
        options.rowGroupByteLimit = 64 * 1024;
        ASSERT_TRUE( DATATABLE_STATUS_OK == WriteParquet( schema, many, options, file, error, false, &summary ) );
        ASSERT_TRUE( summary.rowGroupCount > 4 );
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadParquet( file, read, error ) && SameRows( many, read, difference ) );
        options.rowGroupByteLimit = CDataTableWriterOptions().rowGroupByteLimit;

        // a selection: every third row
        CDataTableBufferPool& pool = CCoreGlobal::Instance()->GetDataTableBufferPool();
        CDataTableBatchPtr selected = SliceRows( many[ 0 ], 0, 3000 );
        CDataTableBufferPtr selectionBuffer;
        pool.Acquire( 1000 * 4, selectionBuffer );
        UInt32* selection = static_cast< UInt32* >( selectionBuffer->GetData() );
        for ( UInt32 i=0; i<1000; ++i )
            selection[ i ] = i * 3;
        selectionBuffer->SetDataSize( 4000 );
        selected.GetWritable()->SetSelection( selection, 1000, selectionBuffer );
        TBatchVector withSelection;
        withSelection.push_back( selected );
        ASSERT_TRUE( DATATABLE_STATUS_OK == WriteParquet( schema, withSelection, options, file, error, false, &summary ) );
        ASSERT_TRUE( 1000 == summary.rowCount );
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadParquet( file, read, error ) && 1000 == CountRows( read ) );
        bool same = true;
        for ( UInt32 i=0; i<1000 && same; ++i )
        {
            const UInt32 batchIndex = i / 16384;
            for ( UInt32 c=0; c<schema->GetColumnCount() && same; ++c )
            {
                CVariant expected, actual;
                many[ 0 ]->GetValue( i * 3, c, expected, false );
                read[ batchIndex ]->GetValue( i, c, actual, false );
                same = expected.IsNULL() == actual.IsNULL() && ( expected.IsNULL() || expected.AsString() == actual.AsString() || expected.IsFloat() );
            }
        }
        ASSERT_TRUE( same );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: the statistics of every row group and of the file equal brute force statistics; long text bounds are left out; NaN never is a bound" )
    try
    {
        const CDataTableSchemaPtr schema = CreateDataTableSchemaPtr( MakeAllTypesSchema( true ) );
        CTestRandom random( 77 );
        TBatchVector batches;
        batches.push_back( MakeRandomBatch( schema, 5000, 20, random ) );
        CDataTableWriterOptions options;
        options.rowGroupRowCount = 1700;
        options.pageRowCount = 400;
        CDynamicBuffer file;
        CString error;
        CDataTableFileSummary summary;
        ASSERT_TRUE( DATATABLE_STATUS_OK == WriteParquet( schema, batches, options, file, error, false, &summary ) );
        CITableFileMetaDataPtr holder;
        const CParquetFileMetaData* footer = ParseFooter( file, holder );
        ASSERT_TRUE( GUCEF_NULL != footer && 3 == footer->GetRowGroupCount() );
        UInt32 compared = 0;
        for ( UInt32 g=0; GUCEF_NULL != footer && g<footer->GetRowGroupCount(); ++g )
        {
            CDataTableRowGroupInfo info;
            ASSERT_TRUE( footer->GetRowGroupInfo( g, info ) );
            CDataTableRowGroupInfo expected;
            CDataTableMerge::ComputeStats( *SliceRows( batches[ 0 ], g * 1700, (UInt32) info.rowCount ), expected );
            for ( UInt32 c=0; c<schema->GetColumnCount(); ++c )
            {
                CString difference;
                const bool same = SameStats( schema->GetColumn( c ).type, expected.columnStats[ c ], info.columnStats[ c ], difference );
                if ( !same )
                    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "Parquet statistics: row group " << g << " column " << schema->GetColumn( c ).name << ": " << difference );
                ASSERT_TRUE( same );
                ASSERT_TRUE( !info.columnStats[ c ].hasMinMax || ( info.columnStats[ c ].minExact && info.columnStats[ c ].maxExact ) );
                ++compared;
            }
        }
        ASSERT_TRUE( 3 * schema->GetColumnCount() == compared );

        // the file summary
        CDataTableRowGroupInfo whole;
        CDataTableMerge::ComputeStats( *batches[ 0 ], whole );
        ASSERT_TRUE( summary.columnStats.size() == schema->GetColumnCount() );
        for ( UInt32 c=0; c<schema->GetColumnCount() && c<summary.columnStats.size(); ++c )
        {
            CString difference;
            const bool same = SameStats( schema->GetColumn( c ).type, whole.columnStats[ c ], summary.columnStats[ c ], difference );
            if ( !same )
                GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "Parquet summary statistics: column " << schema->GetColumn( c ).name << ": " << difference );
            ASSERT_TRUE( same );
        }

        // text longer than statisticsMaxValueBytes: no bounds for that chunk, the null count stays
        CDataTableSchema textSchema;
        textSchema.AddColumn( CDataTableColumnDef( "t", DATATABLE_TYPE_UTF8 ) );
        textSchema.AddColumn( CDataTableColumnDef( "f", DATATABLE_TYPE_FLOAT64 ) );
        const CDataTableSchemaPtr textSchemaPtr = CreateDataTableSchemaPtr( textSchema );
        CDataTableBatchBuilder builder( textSchemaPtr, CCoreGlobal::Instance()->GetDataTableBufferPool() );
        const CString longText( "0123456789012345678901234567890123456789012345678901234567890123456789" );
        builder.GetColumn( 0 ).AppendString( "short" );
        builder.GetColumn( 0 ).AppendString( longText );
        builder.GetColumn( 0 ).AppendNull();
        UInt64 nanBits = 0x7FF8000000000000ULL;
        Float64 nan = 0;
        memcpy( &nan, &nanBits, 8 );
        builder.GetColumn( 1 ).AppendFloat64( nan );
        builder.GetColumn( 1 ).AppendFloat64( -0.0 );
        builder.GetColumn( 1 ).AppendFloat64( 0.0 );
        CDataTableBatchPtr textBatch;
        ASSERT_TRUE( DATATABLE_STATUS_OK == builder.Build( textBatch, error ) );
        TBatchVector textBatches;
        textBatches.push_back( textBatch );
        ASSERT_TRUE( DATATABLE_STATUS_OK == WriteParquet( textSchemaPtr, textBatches, CDataTableWriterOptions(), file, error ) );
        footer = ParseFooter( file, holder );
        CDataTableRowGroupInfo textInfo;
        ASSERT_TRUE( GUCEF_NULL != footer && footer->GetRowGroupInfo( 0, textInfo ) );
        ASSERT_TRUE( !textInfo.columnStats[ 0 ].hasMinMax && textInfo.columnStats[ 0 ].hasNullCount && 1 == textInfo.columnStats[ 0 ].nullCount );
        ASSERT_TRUE( textInfo.columnStats[ 1 ].hasMinMax && 0.0 == textInfo.columnStats[ 1 ].minValue.AsFloat64() && 0.0 == textInfo.columnStats[ 1 ].maxValue.AsFloat64() );
        // the zero minimum is written as -0.0, the zero maximum as +0.0, NaN is counted
        const SParquetStatistics& raw = footer->GetParquetMetaData().rowGroups[ 0 ].columns[ 1 ].metaData.statistics;
        ASSERT_TRUE( raw.hasMinValue && 8 == raw.minValue.size && 0x80 == raw.minValue.data[ 7 ] && 0x00 == raw.maxValue.data[ 7 ] );
        ASSERT_TRUE( raw.hasNanCount && 1 == raw.nanCount );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: the NaN count of float columns is written and read, and NOT_EQUALS on a float column returns the NaN rows of a row group whose other values equal the literal" )
    try
    {
        // f64 { 5.0, NaN | 1.0, 2.0 } in row groups of two: min = max = 5.0 in the first, the NaN cell is not left out of the answer
        CDataTableDefinition definition;
        definition.name = "parquet_nan";
        definition.codecType = "parquet";
        definition.schema.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, false ) );
        definition.schema.AddColumn( CDataTableColumnDef( "f64", DATATABLE_TYPE_FLOAT64 ) );
        definition.schema.AddColumn( CDataTableColumnDef( "f32", DATATABLE_TYPE_FLOAT32 ) );
        definition.storage.rowGroupRowCount = 2;
        definition.storage.pageRowCount = 2;
        const CDataTableSchemaPtr schema = definition.CreateSchemaPtr();
        const CString path = TestFilePath( "nan.parquet" );
        CFileDataTable fileTable( path, GetParquetCodec(), CValueList(), definition );
        CMemoryDataTable memoryTable( definition );
        CDataTableBufferPool& pool = CCoreGlobal::Instance()->GetDataTableBufferPool();
        CDataTableBatchBuilder builder( schema, pool );
        const Float64 nan = std::numeric_limits< Float64 >::quiet_NaN();
        const Float64 values[ 4 ] = { 5.0, nan, 1.0, 2.0 };
        for ( UInt32 r=0; r<4; ++r )
        {
            CVariant cells[ 3 ];
            cells[ 0 ] = CVariant( (Int64) r );
            cells[ 1 ] = CVariant( values[ r ] );
            cells[ 2 ] = CVariant( (Float32) values[ r ] );
            ASSERT_TRUE( builder.AppendRow( cells, 3 ) );
        }
        CDataTableBatchPtr rows;
        CString error;
        ASSERT_TRUE( DATATABLE_STATUS_OK == builder.Build( rows, error ) );
        CDataTableWriteResult result;
        ASSERT_TRUE( DATATABLE_STATUS_OK == fileTable.Write( *rows, DATATABLE_WRITE_INSERT, result ) );
        ASSERT_TRUE( DATATABLE_STATUS_OK == memoryTable.Write( *rows, DATATABLE_WRITE_INSERT, result ) );

        for ( UInt32 c=1; c<=2; ++c )
        {
            const CString column = 1 == c ? "f64" : "f32";
            CDataTableQuery notFive;
            notFive.Where( CDataTablePredicate( column, DATATABLE_PREDICATE_NOT_EQUALS, 1 == c ? CVariant( (Float64) 5.0 ) : CVariant( (Float32) 5.0f ) ) );
            CString::StringVector fileRows, memoryRows;
            ASSERT_TRUE( DATATABLE_STATUS_OK == ScanRows( fileTable, notFive, fileRows ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == ScanRows( memoryTable, notFive, memoryRows ) );
            ASSERT_TRUE( 3 == fileRows.size() && fileRows == memoryRows );
        }

        // the writer states the NaN count per row group, the reader takes it over
        CDynamicBuffer file;
        TBatchVector batches;
        batches.push_back( rows );
        CDataTableWriterOptions options;
        options.rowGroupRowCount = 2;
        options.pageRowCount = 2;
        CDataTableFileSummary summary;
        const TDataTableStatus written = WriteParquet( schema, batches, options, file, error, false, &summary );
        if ( DATATABLE_STATUS_OK != written )
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "Parquet NaN count: writing failed: " << DataTableStatusToString( written ) << " " << error );
        ASSERT_TRUE( DATATABLE_STATUS_OK == written );
        CITableFileMetaDataPtr holder;
        const CParquetFileMetaData* footer = DATATABLE_STATUS_OK == written ? ParseFooter( file, holder ) : GUCEF_NULL;
        CDataTableRowGroupInfo first, second;
        const bool haveGroups = GUCEF_NULL != footer && 2 == footer->GetRowGroupCount() && footer->GetRowGroupInfo( 0, first ) && footer->GetRowGroupInfo( 1, second ) &&
                                3 == first.columnStats.size() && 3 == second.columnStats.size() && 3 == summary.columnStats.size();
        ASSERT_TRUE( haveGroups );
        if ( haveGroups )
        {
            ASSERT_TRUE( first.columnStats[ 1 ].hasNanCount && 1 == first.columnStats[ 1 ].nanCount );
            ASSERT_TRUE( second.columnStats[ 1 ].hasNanCount && 0 == second.columnStats[ 1 ].nanCount );
            ASSERT_TRUE( !first.columnStats[ 0 ].hasNanCount );
            ASSERT_TRUE( summary.columnStats[ 1 ].hasNanCount && 1 == summary.columnStats[ 1 ].nanCount );
            ASSERT_TRUE( summary.columnStats[ 2 ].hasNanCount && 1 == summary.columnStats[ 2 ].nanCount );
            ASSERT_TRUE( !summary.columnStats[ 0 ].hasNanCount );
        }
        DeleteFile( path );

        // statistics of another writer: min = max = 5.0 and no NaN count. Nothing is known about NaN, so the row group is
        // not pruned for NOT_EQUALS 5.0; a stated count of 0 prunes it, and a count beyond the rows is left out
        SParquetColumnInfo info;
        info.supported = true;
        info.physicalType = PARQUET_TYPE_DOUBLE;
        info.columnOrder = PARQUET_COLUMNORDER_TYPE_DEFINED;
        info.column = CDataTableColumnDef( "f64", DATATABLE_TYPE_FLOAT64 );
        UInt8 five[ 8 ];
        const Float64 fiveValue = 5.0;
        memcpy( five, &fiveValue, 8 );
        SParquetColumnChunk chunk;
        chunk.hasMetaData = true;
        chunk.metaData.hasStatistics = true;
        chunk.metaData.statistics.hasMinValue = chunk.metaData.statistics.hasMaxValue = true;
        chunk.metaData.statistics.minValue = chunk.metaData.statistics.maxValue = SParquetBytes( five, 8 );
        chunk.metaData.statistics.hasIsMinValueExact = chunk.metaData.statistics.hasIsMaxValueExact = true;
        chunk.metaData.statistics.isMinValueExact = chunk.metaData.statistics.isMaxValueExact = true;
        chunk.metaData.statistics.hasNullCount = true;
        chunk.metaData.statistics.nullCount = 0;

        CDataTableSchema oneColumnSchema;
        oneColumnSchema.AddColumn( info.column );
        const CDataTableSchemaPtr oneColumn = CreateDataTableSchemaPtr( oneColumnSchema );
        CDataTableResolvedQuery resolved;
        resolved.schema = oneColumn;
        ASSERT_TRUE( CDataTableQuery().Where( CDataTablePredicate( "f64", DATATABLE_PREDICATE_NOT_EQUALS, CVariant( (Float64) 5.0 ) ) ).Resolve( *oneColumn, resolved, error ) );
        ASSERT_TRUE( 1 == resolved.predicates.size() );

        CDataTableColumnStats external;
        CParquetSchemaMapping::BuildColumnStats( info, chunk, 4, external );
        ASSERT_TRUE( external.hasMinMax && !external.hasNanCount );
        ASSERT_TRUE( CDataTablePruner::MayMatch( resolved.predicates[ 0 ], info.column, external ) );

        chunk.metaData.statistics.hasNanCount = true;
        chunk.metaData.statistics.nanCount = 0;
        CDataTableColumnStats noNaN;
        CParquetSchemaMapping::BuildColumnStats( info, chunk, 4, noNaN );
        ASSERT_TRUE( noNaN.hasNanCount && 0 == noNaN.nanCount );
        ASSERT_FALSE( CDataTablePruner::MayMatch( resolved.predicates[ 0 ], info.column, noNaN ) );

        chunk.metaData.statistics.nanCount = 2;
        CDataTableColumnStats withNaN;
        CParquetSchemaMapping::BuildColumnStats( info, chunk, 4, withNaN );
        ASSERT_TRUE( withNaN.hasNanCount && 2 == withNaN.nanCount );
        ASSERT_TRUE( CDataTablePruner::MayMatch( resolved.predicates[ 0 ], info.column, withNaN ) );

        chunk.metaData.statistics.nanCount = 5;                  // more than the rows of the chunk: not believed
        CDataTableColumnStats absurd;
        CParquetSchemaMapping::BuildColumnStats( info, chunk, 4, absurd );
        ASSERT_TRUE( !absurd.hasNanCount );
        ASSERT_TRUE( CDataTablePruner::MayMatch( resolved.predicates[ 0 ], info.column, absurd ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: a dictionary that passes its byte limit continues PLAIN, and the encodings and encoding statistics of the chunk say so" )
    try
    {
        CDataTableSchema schema;
        schema.AddColumn( CDataTableColumnDef( "t", DATATABLE_TYPE_UTF8, false ) );
        const CDataTableSchemaPtr schemaPtr = CreateDataTableSchemaPtr( schema );
        CTestRandom random( 3 );
        TBatchVector batches;
        batches.push_back( MakeRandomBatch( schemaPtr, 3000, 0, random, 100000 ) );
        CDataTableWriterOptions options;
        options.dictionaryByteLimit = 2000;
        options.pageRowCount = 100;
        CDynamicBuffer file;
        CString error;
        ASSERT_TRUE( DATATABLE_STATUS_OK == WriteParquet( schemaPtr, batches, options, file, error ) );
        TBatchVector read;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadParquet( file, read, error, GUCEF_NULL, 250 ) );
        CString difference;
        ASSERT_TRUE( SameRows( batches, read, difference ) );
        // batches of one page each keep the form of their page
        TBatchVector perPage;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadParquet( file, perPage, error, GUCEF_NULL, 100 ) );
        ASSERT_TRUE( SameRows( batches, perPage, difference ) );
        CITableFileMetaDataPtr holder;
        const CParquetFileMetaData* footer = ParseFooter( file, holder );
        ASSERT_TRUE( GUCEF_NULL != footer );
        if ( GUCEF_NULL != footer )
        {
            const SParquetColumnMetaData& meta = footer->GetParquetMetaData().rowGroups[ 0 ].columns[ 0 ].metaData;
            bool plain = false, dictionary = false;
            for ( size_t i=0; i<meta.encodings.size(); ++i )
            {
                plain = plain || PARQUET_ENCODING_PLAIN == meta.encodings[ i ];
                dictionary = dictionary || PARQUET_ENCODING_RLE_DICTIONARY == meta.encodings[ i ];
            }
            ASSERT_TRUE( plain && dictionary && meta.hasDictionaryPageOffset && meta.hasEncodingStats );
            Int32 plainPages = 0, dictionaryPages = 0;
            for ( size_t i=0; i<meta.encodingStats.size(); ++i )
            {
                if ( PARQUET_PAGE_DATA_PAGE == meta.encodingStats[ i ].pageType && PARQUET_ENCODING_PLAIN == meta.encodingStats[ i ].encoding ) plainPages += meta.encodingStats[ i ].count;
                if ( PARQUET_PAGE_DATA_PAGE == meta.encodingStats[ i ].pageType && PARQUET_ENCODING_RLE_DICTIONARY == meta.encodingStats[ i ].encoding ) dictionaryPages += meta.encodingStats[ i ].count;
            }
            ASSERT_TRUE( plainPages > 0 && dictionaryPages > 0 && 30 == plainPages + dictionaryPages );
        }
        // the batches of dictionary pages carry dictionary storage, the others variable storage
        bool sawDictionary = false, sawVariable = false;
        for ( size_t i=0; i<perPage.size(); ++i )
        {
            sawDictionary = sawDictionary || DATATABLE_STORAGE_DICTIONARY == perPage[ i ]->GetColumn( 0 ).GetStorage();
            sawVariable = sawVariable || DATATABLE_STORAGE_VARIABLE == perPage[ i ]->GetColumn( 0 ).GetStorage();
        }
        ASSERT_TRUE( sawDictionary && sawVariable );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: a projection decodes only its columns, in its order, and reads less of the file" )
    try
    {
        const CDataTableSchemaPtr schema = CreateDataTableSchemaPtr( MakeAllTypesSchema( true ) );
        CTestRandom random( 21 );
        TBatchVector batches;
        batches.push_back( MakeRandomBatch( schema, 4000, 10, random ) );
        CDynamicBuffer file;
        CString error;
        ASSERT_TRUE( DATATABLE_STATUS_OK == WriteParquet( schema, batches, CDataTableWriterOptions(), file, error ) );
        CValueList noCoalescing;
        noCoalescing.Set( "ioCoalesceGapBytes", CVariant( "0" ) );
        CDataTableScanStats full, part;
        TBatchVector read;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadParquet( file, read, error, GUCEF_NULL, 0, &full, false, &noCoalescing ) );
        CDataTableSchema::TColumnIndexVector projection;
        projection.push_back( 17 );     // text
        projection.push_back( 4 );      // i64
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadParquet( file, read, error, &projection, 0, &part, false, &noCoalescing ) );
        ASSERT_TRUE( !read.empty() && 2 == read[ 0 ]->GetColumnCount() && 17 == read[ 0 ]->GetSchemaColumnIndex( 0 ) && 4 == read[ 0 ]->GetSchemaColumnIndex( 1 ) );
        CString difference;
        ASSERT_TRUE( SameRows( batches, read, difference, &projection ) );
        ASSERT_TRUE( part.bytesRead * 4 < full.bytesRead );
        ASSERT_TRUE( part.pagesDecoded + part.pagesInPlace < full.pagesDecoded + full.pagesInPlace );

        // a column twice, or one the file does not have, is refused
        projection.push_back( 17 );
        ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == ReadParquet( file, read, error, &projection ) );
        projection.clear();
        projection.push_back( 99 );
        ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == ReadParquet( file, read, error, &projection ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: PLAIN fixed width pages are used in place, the spans are views of the file bytes as read, and a warm read of batches makes no heap allocation" )
    try
    {
        CDataTableSchema schema;
        schema.AddColumn( CDataTableColumnDef( "i64", DATATABLE_TYPE_INT64, false ) );
        schema.AddColumn( CDataTableColumnDef( "f64", DATATABLE_TYPE_FLOAT64, false ) );
        schema.AddColumn( CDataTableColumnDef( "f32", DATATABLE_TYPE_FLOAT32, false ) );
        schema.AddColumn( CDataTableColumnDef( "date", DATATABLE_TYPE_DATE, false ) );
        schema.AddColumn( CDataTableColumnDef::Timestamp( "ts", DATATABLE_TIMEUNIT_MICROSECONDS, true, true ) );     // nullable, no nulls
        const CDataTableSchemaPtr schemaPtr = CreateDataTableSchemaPtr( schema );
        CTestRandom random( 8 );
        TBatchVector batches;
        batches.push_back( MakeRandomBatch( schemaPtr, 4 * 16384, 0, random ) );
        CDynamicBuffer file;
        CString error;
        ASSERT_TRUE( DATATABLE_STATUS_OK == WriteParquet( schemaPtr, batches, CDataTableWriterOptions(), file, error ) );

        IOAccessPtr input = MakeReadAccess( file );
        CITableFileMetaDataPtr metaData;
        ASSERT_TRUE( DATATABLE_STATUS_OK == GetParquetCodec()->ReadMetaData( *input, CValueList(), metaData, error ) );
        CITableReaderPtr reader;
        ASSERT_TRUE( DATATABLE_STATUS_OK == GetParquetCodec()->CreateReader( input, metaData, CValueList(), reader, error ) );
        CDataTableScanStats stats;
        CDataTableReadRequest request;
        request.batchRowCount = 16384;
        request.stats = &stats;
        ASSERT_TRUE( DATATABLE_STATUS_OK == reader->OpenRowGroup( 0, request ) );
        TBatchVector read;
        for ( ;; )
        {
            CDataTableBatchPtr batch;
            if ( DATATABLE_STATUS_OK != reader->ReadNextBatch( batch ) )
                break;
            read.push_back( batch );
        }
        ASSERT_TRUE( 4 == read.size() && 20 == stats.pagesInPlace && 0 == stats.pagesDecoded && 0 == stats.alignmentCopies );
        CString difference;
        ASSERT_TRUE( SameRows( batches, read, difference ) );

        // the span of each page holds the bytes of the file at the page values, and the spans of the pages lie as far
        // apart as the pages lie in the file: views of one copy of the file bytes, not copies per page
        for ( UInt32 c=0; c<5 && 4 == read.size(); ++c )
        {
            const UInt32 width = DataTableLogicalTypeByteWidth( schema.GetColumn( c ).type );
            const UInt8* first = static_cast< const UInt8* >( read[ 0 ]->GetColumn( c ).GetFixedData() );
            const Int64 firstAt = FindInFile( file, first, 64 );
            ASSERT_TRUE( firstAt > 0 && 0 == firstAt % 8 && 0 == ( (size_t) first ) % width );
            for ( UInt32 k=1; k<4; ++k )
            {
                const UInt8* span = static_cast< const UInt8* >( read[ k ]->GetColumn( c ).GetFixedData() );
                const Int64 at = FindInFile( file, span, 64 );
                ASSERT_TRUE( at > firstAt && (Int64) ( span - first ) == at - firstAt );
                ASSERT_TRUE( 0 == memcmp( span, static_cast< const UInt8* >( file.GetConstBufferPtr() ) + at, 16384 * width ) );
            }
        }

        // warm: the same row group again; reading its batches makes no heap allocation and takes no new pool memory
        read.clear();
        CDataTableBufferPoolStats before, after;
        CCoreGlobal::Instance()->GetDataTableBufferPool().GetStats( before );
        ASSERT_TRUE( DATATABLE_STATUS_OK == reader->OpenRowGroup( 0, request ) );
        UInt32 allocations = 0;
        UInt32 batchCount = 0;
        const bool counting = CORETEST::IsAllocationCountingAvailable();
        if ( counting )
            CORETEST::StartAllocationCounting();
        for ( ;; )
        {
            CDataTableBatchPtr batch;
            if ( DATATABLE_STATUS_OK != reader->ReadNextBatch( batch ) )
                break;
            ++batchCount;
        }
        if ( counting )
            allocations = CORETEST::StopAllocationCounting();
        CCoreGlobal::Instance()->GetDataTableBufferPool().GetStats( after );
        ASSERT_TRUE( 4 == batchCount );
        ASSERT_TRUE( 0 == allocations );
        ASSERT_TRUE( after.misses == before.misses );
        if ( !counting )
            GUCEF_LOG( LOGLEVEL_NORMAL, "Parquet in place: heap allocations not counted: " << CORETEST::GetAllocationCountingUnavailableReason() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: GZIP pages decompress into pooled pages used in place; a codec that is not registered and an unknown one are UNSUPPORTED, named" )
    GUCEF_TESTFW_SKIP_IF( !IsGzipAvailable(), "codecspluginZLIB is not available" );
    try
    {
        CDataTableSchema schema;
        schema.AddColumn( CDataTableColumnDef( "i64", DATATABLE_TYPE_INT64, false ) );
        schema.AddColumn( CDataTableColumnDef( "s", DATATABLE_TYPE_UTF8, true ) );
        const CDataTableSchemaPtr schemaPtr = CreateDataTableSchemaPtr( schema );
        CTestRandom random( 31 );
        TBatchVector batches;
        batches.push_back( MakeRandomBatch( schemaPtr, 40000, 10, random, 30 ) );
        CDataTableWriterOptions options;
        options.compression = "gzip";
        options.compressionLevel = 6;
        CDynamicBuffer file;
        CString error;
        ASSERT_TRUE( DATATABLE_STATUS_OK == WriteParquet( schemaPtr, batches, options, file, error ) );
        CDynamicBuffer plainFile;
        ASSERT_TRUE( DATATABLE_STATUS_OK == WriteParquet( schemaPtr, batches, CDataTableWriterOptions(), plainFile, error ) );
        ASSERT_TRUE( file.GetDataSize() < plainFile.GetDataSize() );
        CDataTableScanStats stats;
        TBatchVector read;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadParquet( file, read, error, GUCEF_NULL, 16384, &stats ) );
        CString difference;
        ASSERT_TRUE( SameRows( batches, read, difference ) );
        ASSERT_TRUE( 3 == stats.pagesInPlace );
        CITableFileMetaDataPtr holder;
        const CParquetFileMetaData* footer = ParseFooter( file, holder );
        ASSERT_TRUE( GUCEF_NULL != footer && PARQUET_COMPRESSION_GZIP == footer->GetParquetMetaData().rowGroups[ 0 ].columns[ 0 ].metaData.codec );

        options.compression = "snappy";
        CITableWriterPtr writer;
        CDynamicBuffer unused;
        ASSERT_TRUE( DATATABLE_STATUS_UNSUPPORTED == WriteParquet( schemaPtr, batches, options, unused, error ) );
        ASSERT_TRUE( error.HasSubstr( "snappy" ) >= 0 );
        options.compression = "lz77";
        ASSERT_TRUE( DATATABLE_STATUS_UNSUPPORTED == WriteParquet( schemaPtr, batches, options, unused, error ) );
        ASSERT_TRUE( error.HasSubstr( "lz77" ) >= 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: through CFileDataTable scans, row groups are pruned on statistics and dictionaries, and random conjunctions select the rows a memory table selects" )
    try
    {
        CDataTableDefinition definition;
        definition.name = "parquet_property";
        definition.codecType = "parquet";
        definition.schema.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, false ) );
        const CDataTableSchemaPtr allTypes = CORETEST::MakeAllTypesSchema();
        for ( UInt32 c=0; c<allTypes->GetColumnCount(); ++c )
            definition.schema.AddColumn( allTypes->GetColumn( c ) );
        CString::StringVector sortKey;
        sortKey.push_back( "id" );
        definition.schema.SetSortKey( sortKey );
        definition.storage.rowGroupRowCount = 97;
        definition.storage.pageRowCount = 40;
        const CDataTableSchemaPtr schema = definition.CreateSchemaPtr();

        const CString path = TestFilePath( "property.parquet" );
        CFileDataTable fileTable( path, GetParquetCodec(), CValueList(), definition );
        CMemoryDataTable memoryTable( definition );
        ASSERT_TRUE( memoryTable.IsValid() );
        CORETEST::CTestRandom random( 4242 );
        CDataTableBufferPool& pool = CCoreGlobal::Instance()->GetDataTableBufferPool();
        const CDataTableBatchPtr rows = CORETEST::MakeRandomBatch( schema, pool, random, 1000, 0, 15 );
        CDataTableWriteResult result;
        ASSERT_TRUE( DATATABLE_STATUS_OK == fileTable.Write( *rows, DATATABLE_WRITE_INSERT, result ) );
        ASSERT_TRUE( DATATABLE_STATUS_OK == memoryTable.Write( *rows, DATATABLE_WRITE_INSERT, result ) );

        // a range on the sorted id: row groups wholly outside are pruned on their statistics
        CDataTableQuery range;
        range.WhereRange( "id", CVariant( (Int64) 200 ), true, CVariant( (Int64) 299 ), true );
        CString::StringVector fileRows, memoryRows;
        CDataTableScanStats stats;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ScanRows( fileTable, range, fileRows, &stats ) );
        ASSERT_TRUE( DATATABLE_STATUS_OK == ScanRows( memoryTable, range, memoryRows ) );
        ASSERT_TRUE( 100 == fileRows.size() && fileRows == memoryRows );
        ASSERT_TRUE( 11 == stats.rowGroupsTotal && 9 == stats.rowGroupsPrunedByStatistics );

        // random conjunctions of one to three predicates over every column
        UInt64 prunedByStatistics = 0;
        UInt32 mismatches = 0;
        for ( UInt32 q=0; q<300; ++q )
        {
            CDataTableQuery query;
            const UInt32 count = 1 + random.Below( 3 );
            for ( UInt32 p=0; p<count; ++p )
                query.Where( RandomPredicate( random, schema->GetColumn( random.Below( schema->GetColumnCount() ) ) ) );
            CDataTableScanStats queryStats;
            const TDataTableStatus fileStatus = ScanRows( fileTable, query, fileRows, &queryStats );
            const TDataTableStatus memoryStatus = ScanRows( memoryTable, query, memoryRows );
            if ( fileStatus != memoryStatus || fileRows != memoryRows )
            {
                ++mismatches;
                GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "Parquet property: query " << q << ": parquet " << DataTableStatusToString( fileStatus ) << " " << (UInt32) fileRows.size() <<
                                 " rows, memory " << DataTableStatusToString( memoryStatus ) << " " << (UInt32) memoryRows.size() << " rows" );
            }
            prunedByStatistics += queryStats.rowGroupsPrunedByStatistics;
        }
        ASSERT_TRUE( 0 == mismatches );
        ASSERT_TRUE( prunedByStatistics > 0 );

        // dictionaries: every row group holds two symbols, the literal lies between them and in no row group
        CDataTableDefinition symbols;
        symbols.name = "parquet_symbols";
        symbols.codecType = "parquet";
        symbols.schema.AddColumn( CDataTableColumnDef( "sym", DATATABLE_TYPE_UTF8, false ) );
        symbols.schema.AddColumn( CDataTableColumnDef( "v", DATATABLE_TYPE_INT64, false ) );
        symbols.storage.rowGroupRowCount = 50;
        symbols.storage.pageRowCount = 25;
        const CDataTableSchemaPtr symbolSchema = symbols.CreateSchemaPtr();
        CDataTableBatchBuilder builder( symbolSchema, pool );
        const char* const names[] = { "AA", "AC", "BA", "BC", "CA", "CC" };
        for ( UInt32 r=0; r<150; ++r )
        {
            const char* name = names[ ( r / 50 ) * 2 + ( r % 2 ) ];
            builder.GetColumn( 0 ).AppendUtf8( name, 2 );
            builder.GetColumn( 1 ).AppendInt64( r );
        }
        CDataTableBatchPtr symbolRows;
        CString error;
        ASSERT_TRUE( DATATABLE_STATUS_OK == builder.Build( symbolRows, error ) );
        const CString symbolPath = TestFilePath( "symbols.parquet" );
        CFileDataTable symbolTable( symbolPath, GetParquetCodec(), CValueList(), symbols );
        const TDataTableStatus symbolsWritten = symbolTable.Write( *symbolRows, DATATABLE_WRITE_INSERT, result );
        if ( DATATABLE_STATUS_OK != symbolsWritten )
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "Parquet symbols: " << symbolTable.GetLastErrorMessage() );
        ASSERT_TRUE( DATATABLE_STATUS_OK == symbolsWritten );
        CDataTableQuery absent;
        absent.WhereEquals( "sym", CVariant( "AB" ) );
        ASSERT_TRUE( DATATABLE_STATUS_OK == ScanRows( symbolTable, absent, fileRows, &stats ) );
        ASSERT_TRUE( fileRows.empty() && 3 == stats.rowGroupsTotal && 1 == stats.rowGroupsPrunedByDictionary && 2 == stats.rowGroupsPrunedByStatistics );
        CDataTableQuery present;
        CVariant::VariantVector members;
        members.push_back( CVariant( "AB" ) );
        members.push_back( CVariant( "BC" ) );
        present.WhereIn( "sym", members );
        ASSERT_TRUE( DATATABLE_STATUS_OK == ScanRows( symbolTable, present, fileRows, &stats ) );
        ASSERT_TRUE( 25 == fileRows.size() && 1 == stats.rowGroupsPrunedByDictionary );
        DeleteFile( path );
        DeleteFile( symbolPath );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: the table schema rides in the file: descriptions, encoding hints, keys and the schema version return; sorted rows list their sort columns" )
    GUCEF_TESTFW_SKIP_IF( !IsJsonAvailable(), "no JSON codec is registered" );
    try
    {
        CDataTableDefinition definition;
        definition.name = "described";
        definition.codecType = "parquet";
        CDataTableColumnDef mic( "mic", DATATABLE_TYPE_UTF8, false );
        mic.description = "market identifier (ISO 10383)";
        CDataTableColumnDef day( "date", DATATABLE_TYPE_DATE, false );
        CDataTableColumnDef close = CDataTableColumnDef::Decimal( "close", 18, 6 );
        close.encodingHint = DATATABLE_ENCODING_DICTIONARY;
        close.description = "closing price";
        definition.schema.AddColumn( mic );
        definition.schema.AddColumn( day );
        definition.schema.AddColumn( close );
        CString::StringVector key;
        key.push_back( "mic" );
        key.push_back( "date" );
        definition.schema.SetPrimaryKey( key );
        CString::StringVector sortKey;
        sortKey.push_back( "date" );
        definition.schema.SetSortKey( sortKey );
        definition.schema.SetSchemaVersion( 3 );
        const CDataTableSchemaPtr schema = definition.CreateSchemaPtr();

        CDataTableBatchBuilder builder( schema, CCoreGlobal::Instance()->GetDataTableBufferPool() );
        for ( UInt32 r=0; r<20; ++r )
        {
            builder.GetColumn( 0 ).AppendUtf8( "XNYS", 4 );
            builder.GetColumn( 1 ).AppendDate( (Int32) ( 20000 - r ) );     // given out of order: the table sorts
            builder.GetColumn( 2 ).AppendDecimalMantissa( 1000000 + r );
        }
        CDataTableBatchPtr rows;
        CString error;
        ASSERT_TRUE( DATATABLE_STATUS_OK == builder.Build( rows, error ) );
        const CString path = TestFilePath( "described.parquet" );
        {
            CFileDataTable table( path, GetParquetCodec(), CValueList(), definition );
            CDataTableWriteResult result;
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Write( *rows, DATATABLE_WRITE_INSERT, result ) );
        }
        CDynamicBuffer file;
        ASSERT_TRUE( file.LoadContentFromFile( path ) );
        CITableFileMetaDataPtr holder;
        const CParquetFileMetaData* footer = ParseFooter( file, holder );
        ASSERT_TRUE( GUCEF_NULL != footer );
        if ( GUCEF_NULL != footer )
        {
            const CDataTableSchema& fileSchema = *footer->GetSchema();
            ASSERT_TRUE( fileSchema == definition.schema );
            ASSERT_TRUE( "closing price" == fileSchema.GetColumn( 2 ).description && DATATABLE_ENCODING_DICTIONARY == fileSchema.GetColumn( 2 ).encodingHint );
            ASSERT_TRUE( 2 == fileSchema.GetPrimaryKey().size() && 1 == fileSchema.GetSortKey().size() && 3 == fileSchema.GetSchemaVersion() );
            CValueList keyValues;
            footer->GetKeyValueMetaData( keyValues );
            ASSERT_TRUE( keyValues.HasKey( CParquetFileMetaData::TableSchemaKey ) && keyValues.HasKey( CFileDataTable::LayoutMetaDataKey ) );
            CDataTableRowGroupInfo info;
            ASSERT_TRUE( footer->GetRowGroupInfo( 0, info ) && 1 == info.sortedOn.size() && 1 == info.sortedOn[ 0 ] );
            // close asked for dictionary encoding
            const SParquetColumnMetaData& closeMeta = footer->GetParquetMetaData().rowGroups[ 0 ].columns[ 2 ].metaData;
            ASSERT_TRUE( closeMeta.hasDictionaryPageOffset );
            ASSERT_TRUE( 0 == strcmp( CParquetTableWriter::CreatedBy, footer->GetParquetMetaData().createdBy.ToString().C_String() ) );
        }

        // rows that are not sorted on the sort key: no sorting columns are claimed
        TBatchVector unsorted;
        unsorted.push_back( rows );
        ASSERT_TRUE( DATATABLE_STATUS_OK == WriteParquet( schema, unsorted, CDataTableWriterOptions(), file, error ) );
        footer = ParseFooter( file, holder );
        CDataTableRowGroupInfo info;
        ASSERT_TRUE( GUCEF_NULL != footer && footer->GetRowGroupInfo( 0, info ) && info.sortedOn.empty() );
        DeleteFile( path );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: the table schema in a footer is parsed once per text: files of one table share it, another schema is parsed anew, the cache keeps at most MaxEntries texts" )
    GUCEF_TESTFW_SKIP_IF( !IsJsonAvailable(), "no JSON codec is registered" );
    try
    {
        CDataTableSchema first;
        CDataTableColumnDef mic( "mic", DATATABLE_TYPE_UTF8, false );
        mic.description = "market identifier";
        first.AddColumn( mic );
        first.AddColumn( CDataTableColumnDef::Decimal( "close", 18, 6 ) );
        CString::StringVector key;
        key.push_back( "mic" );
        first.SetPrimaryKey( key );
        first.SetSchemaVersion( 2 );
        // the same columns with another description and version: the text differs, so must the schema read
        CDataTableSchema second;
        mic.description = "venue";
        second.AddColumn( mic );
        second.AddColumn( CDataTableColumnDef::Decimal( "close", 18, 6 ) );
        second.SetSchemaVersion( 3 );

        const CDataTableSchemaPtr schemas[] = { CreateDataTableSchemaPtr( first ), CreateDataTableSchemaPtr( second ) };
        CDynamicBuffer files[ 4 ];
        for ( UInt32 f=0; f<4; ++f )
        {
            const CDataTableSchemaPtr& schema = schemas[ f / 3 ];       // three files of the first schema, one of the second
            CDataTableBatchBuilder builder( schema, CCoreGlobal::Instance()->GetDataTableBufferPool() );
            for ( UInt32 r=0; r<=f; ++r )
            {
                builder.GetColumn( 0 ).AppendUtf8( "XNYS", 4 );
                builder.GetColumn( 1 ).AppendDecimalMantissa( 1000 * f + r );
            }
            CDataTableBatchPtr rows;
            CString error;
            ASSERT_TRUE( DATATABLE_STATUS_OK == builder.Build( rows, error ) );
            TBatchVector batches;
            batches.push_back( rows );
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteParquet( schema, batches, CDataTableWriterOptions(), files[ f ], error ) );
        }

        // a codec of its own: its cache starts empty
        TABLECODECPLUGIN::PARQUET::CParquetTableCodec codec;
        const TABLECODECPLUGIN::PARQUET::CParquetTableSchemaCache& cache = codec.GetTableSchemaCache();
        CITableFileMetaDataPtr footers[ 4 ];
        for ( UInt32 f=0; f<4; ++f )
        {
            IOAccessPtr input = MakeReadAccess( files[ f ] );
            CString error;
            ASSERT_TRUE( DATATABLE_STATUS_OK == codec.ReadMetaData( *input, CValueList(), footers[ f ], error ) );
            ASSERT_TRUE( !footers[ f ].IsNULL() && f + 1 == footers[ f ]->GetRowCount() );
        }
        ASSERT_TRUE( 2 == cache.GetMissCount() && 2 == cache.GetHitCount() && 2 == cache.GetEntryCount() );
        for ( UInt32 f=0; f<4; ++f )
        {
            const CDataTableSchema& read = *footers[ f ]->GetSchema();
            ASSERT_TRUE( read == *schemas[ f / 3 ] );
            ASSERT_TRUE( ( f < 3 ? "market identifier" : "venue" ) == read.GetColumn( 0 ).description );
            ASSERT_TRUE( ( f < 3 ? 2u : 3u ) == read.GetSchemaVersion() && ( f < 3 ? 1u : 0u ) == read.GetPrimaryKey().size() );
        }

        // a reader made without metadata reads the footer through the cache too
        CITableReaderPtr reader;
        CString error;
        ASSERT_TRUE( DATATABLE_STATUS_OK == codec.CreateReader( MakeReadAccess( files[ 1 ] ), CITableFileMetaDataPtr(), CValueList(), reader, error ) );
        ASSERT_TRUE( !reader.IsNULL() && *reader->GetMetaData()->GetSchema() == first && 3 == cache.GetHitCount() );

        // bounded: the oldest text goes first
        TABLECODECPLUGIN::PARQUET::CParquetTableSchemaCache bounded;
        const UInt32 maxEntries = TABLECODECPLUGIN::PARQUET::CParquetTableSchemaCache::MaxEntries;
        for ( UInt32 i=0; i<maxEntries+3; ++i )
            bounded.Put( "text " + ToString( i ), schemas[ i % 2 ] );
        CDataTableSchemaPtr found;
        ASSERT_TRUE( maxEntries == bounded.GetEntryCount() );
        ASSERT_FALSE( bounded.Find( "text 0", found ) || bounded.Find( "text 2", found ) );
        ASSERT_TRUE( bounded.Find( "text 3", found ) && found.GetPointerAlways() == schemas[ 1 ].GetPointerAlways() );
        ASSERT_TRUE( bounded.Find( "text " + ToString( maxEntries + 2 ), found ) && found.GetPointerAlways() == schemas[ 0 ].GetPointerAlways() );
        bounded.Put( "text 3", schemas[ 0 ] );
        ASSERT_TRUE( maxEntries == bounded.GetEntryCount() && bounded.Find( "text 3", found ) && found.GetPointerAlways() == schemas[ 0 ].GetPointerAlways() );
        bounded.Put( "no schema", CDataTableSchemaPtr() );
        ASSERT_FALSE( bounded.Find( "no schema", found ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: writer misuse is refused with a status: another shape, a null in a required column, after Finish, after Abort; text is checked for UTF-8 on request" )
    try
    {
        CDataTableSchema schema;
        schema.AddColumn( CDataTableColumnDef( "a", DATATABLE_TYPE_INT64, false ) );
        schema.AddColumn( CDataTableColumnDef( "t", DATATABLE_TYPE_UTF8, true ) );
        const CDataTableSchemaPtr schemaPtr = CreateDataTableSchemaPtr( schema );
        CDataTableSchema other;
        other.AddColumn( CDataTableColumnDef( "a", DATATABLE_TYPE_INT32, true ) );
        other.AddColumn( CDataTableColumnDef( "t", DATATABLE_TYPE_UTF8, true ) );
        const CDataTableSchemaPtr otherPtr = CreateDataTableSchemaPtr( other );

        CDynamicBuffer file;
        IOAccessPtr output( GUCEF_NEW CDynamicBufferAccess( file ) );
        CITableWriterPtr writer;
        CString error;
        ASSERT_TRUE( DATATABLE_STATUS_OK == GetParquetCodec()->CreateWriter( output, schemaPtr, CDataTableWriterOptions(), writer, error ) );
        CTestRandom random( 1 );
        CDataTableBatchPtr wrongShape = MakeRandomBatch( otherPtr, 10, 0, random );
        ASSERT_TRUE( DATATABLE_STATUS_SCHEMA_MISMATCH == writer->WriteBatch( *wrongShape ) );
        ASSERT_FALSE( writer->GetLastErrorMessage().IsNULLOrEmpty() );

        // a null in column a, which does not take null
        CDataTableBatchBuilder builder( otherPtr, CCoreGlobal::Instance()->GetDataTableBufferPool() );
        builder.GetColumn( 0 ).AppendNull();
        builder.GetColumn( 1 ).AppendUtf8( "x", 1 );
        CDataTableBatchPtr withNull;
        builder.Build( withNull, error );
        CDataTableSchema nullable;
        nullable.AddColumn( CDataTableColumnDef( "a", DATATABLE_TYPE_INT64, true ) );
        nullable.AddColumn( CDataTableColumnDef( "t", DATATABLE_TYPE_UTF8, true ) );
        CDataTableBatchBuilder builder64( CreateDataTableSchemaPtr( nullable ), CCoreGlobal::Instance()->GetDataTableBufferPool() );
        builder64.GetColumn( 0 ).AppendNull();
        builder64.GetColumn( 1 ).AppendUtf8( "x", 1 );
        builder64.Build( withNull, error );
        writer.Unlink();
        ASSERT_TRUE( DATATABLE_STATUS_OK == GetParquetCodec()->CreateWriter( output, schemaPtr, CDataTableWriterOptions(), writer, error ) );
        ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == writer->WriteBatch( *withNull ) );

        // after Finish and after Abort nothing more is taken
        writer.Unlink();
        file.Clear( true );
        ASSERT_TRUE( DATATABLE_STATUS_OK == GetParquetCodec()->CreateWriter( output, schemaPtr, CDataTableWriterOptions(), writer, error ) );
        CDataTableBatchPtr good = MakeRandomBatch( schemaPtr, 10, 0, random );
        CDataTableFileSummary summary;
        ASSERT_TRUE( DATATABLE_STATUS_OK == writer->WriteBatch( *good ) && DATATABLE_STATUS_OK == writer->Finish( summary ) );
        ASSERT_TRUE( DATATABLE_STATUS_OK != writer->WriteBatch( *good ) );
        ASSERT_TRUE( DATATABLE_STATUS_OK != writer->Finish( summary ) );
        writer.Unlink();
        ASSERT_TRUE( DATATABLE_STATUS_OK == GetParquetCodec()->CreateWriter( output, schemaPtr, CDataTableWriterOptions(), writer, error ) );
        writer->Abort();
        ASSERT_TRUE( DATATABLE_STATUS_OK != writer->WriteBatch( *good ) );

        // a column the codec cannot write
        CDataTableSchema unsupported;
        unsupported.AddColumn( CDataTableColumnDef( "x", DATATABLE_TYPE_UNSUPPORTED ) );
        writer.Unlink();
        ASSERT_TRUE( DATATABLE_STATUS_OK != GetParquetCodec()->CreateWriter( output, CreateDataTableSchemaPtr( unsupported ), CDataTableWriterOptions(), writer, error ) );

        // UTF-8: not checked by default, checked with validateUtf8
        CDataTableBatchBuilder textBuilder( schemaPtr, CCoreGlobal::Instance()->GetDataTableBufferPool() );
        textBuilder.GetColumn( 0 ).AppendInt64( 1 );
        textBuilder.GetColumn( 1 ).AppendUtf8( "\xC0\xAF", 2 );    // an overlong form of '/'
        CDataTableBatchPtr badText;
        textBuilder.Build( badText, error );
        TBatchVector textBatches;
        textBatches.push_back( badText );
        CDataTableWriterOptions plain;
        plain.dictionary = DATATABLE_DICTIONARY_NEVER;
        ASSERT_TRUE( DATATABLE_STATUS_OK == WriteParquet( schemaPtr, textBatches, plain, file, error ) );
        TBatchVector read;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadParquet( file, read, error ) );
        CValueList validate;
        validate.Set( "validateUtf8", CVariant( "true" ) );
        ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == ReadParquet( file, read, error, GUCEF_NULL, 0, GUCEF_NULL, false, &validate ) );
        validate.Set( "maxPageBytes", CVariant( "lots" ) );
        ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == ReadParquet( file, read, error, GUCEF_NULL, 0, GUCEF_NULL, false, &validate ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: the plugin module registers the parquet codec through the table codec plugin manager and takes it out at unload" )
    try
    {
        CPluginControl& pluginControl = CCoreGlobal::Instance()->GetPluginControl();
        CTableCodecRegistry& registry = CCoreGlobal::Instance()->GetTableCodecRegistry();
        CDataNode config( "Config" );
        CDataNode* group = config.AddChild( "PluginGroup" );
        group->SetAttribute( "GroupName", ModuleGroup );
        group->SetAttribute( "LoadImmediately", "true" );
        CDataNode* block = group->AddChild( "PluginMetaData" );
        block->SetAttribute( "Description", "parquet table codec plugin" );
        block->SetAttribute( "Path", "$MODULEDIR$" );
        #ifdef GUCEF_DEBUG_MODE
        block->SetAttribute( "Filename", "tablecodecpluginPARQUET_d" );
        block->SetAttribute( "AltFilename", "tablecodecpluginPARQUET" );
        #else
        block->SetAttribute( "Filename", "tablecodecpluginPARQUET" );
        block->SetAttribute( "AltFilename", "tablecodecpluginPARQUET_d" );
        #endif
        block->SetAttribute( "Type", "GucefTableCodecPlugin" );
        ASSERT_TRUE( pluginControl.LoadConfig( config ) );
        CTableCodecRegistry::TTableCodecPtr codec;
        ASSERT_TRUE( registry.TryLookup( "parquet", codec ) && !codec.IsNULL() );
        CTableCodecRegistry::TTableCodecPtr byExtension;
        ASSERT_TRUE( registry.TryGetCodecForFileExtension( ".PARQUET", byExtension ) && byExtension.GetPointer() == codec.GetPointer() );
        if ( !codec.IsNULL() )
        {
            ASSERT_TRUE( "parquet" == codec->GetTypeName() && 0 != ( codec->GetCapabilities() & GUCEF_TABLECODEC_CAPABILITY_IN_PLACE_PAGES ) );
            // the codec of the module reads what the linked codec wrote
            const CDataTableSchemaPtr schema = CreateDataTableSchemaPtr( MakeAllTypesSchema( true ) );
            CTestRandom random( 2 );
            TBatchVector batches;
            batches.push_back( MakeRandomBatch( schema, 100, 10, random ) );
            CDynamicBuffer file;
            CString error;
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteParquet( schema, batches, CDataTableWriterOptions(), file, error ) );
            IOAccessPtr input = MakeReadAccess( file );
            CITableFileMetaDataPtr metaData;
            ASSERT_TRUE( DATATABLE_STATUS_OK == codec->ReadMetaData( *input, CValueList(), metaData, error ) && 100 == metaData->GetRowCount() );

            // the guard of the module's metadata passes the source check on (API 1.1): the file it was read from verifies,
            // the same file with one footer byte changed and another file do not
            if ( !metaData.IsNULL() )
            {
                ASSERT_TRUE( metaData->CanVerifySource() && metaData->VerifySource( *input ) );
                CDynamicBuffer changed( file );
                static_cast< UInt8* >( changed.GetBufferPtr() )[ changed.GetDataSize() - 20 ] ^= 0x01;
                IOAccessPtr changedInput = MakeReadAccess( changed );
                ASSERT_FALSE( metaData->VerifySource( *changedInput ) );
                TBatchVector otherBatches;
                otherBatches.push_back( MakeRandomBatch( schema, 100, 10, random ) );
                CDynamicBuffer other;
                ASSERT_TRUE( DATATABLE_STATUS_OK == WriteParquet( schema, otherBatches, CDataTableWriterOptions(), other, error ) );
                IOAccessPtr otherInput = MakeReadAccess( other );
                ASSERT_FALSE( metaData->VerifySource( *otherInput ) );
            }
        }
        codec.Unlink();
        byExtension.Unlink();
        pluginControl.UnloadPluginGroup( ModuleGroup );
        pluginControl.ProcessDeferredModuleUnloads( false );
        pluginControl.ClearMetaDataFromGroup( ModuleGroup );
        ASSERT_FALSE( registry.IsRegistered( "parquet" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
