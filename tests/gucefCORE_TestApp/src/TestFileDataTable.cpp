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

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_CFILEDATATABLE_H
#include "gucefCORE_CFileDataTable.h"
#define GUCEF_CORE_CFILEDATATABLE_H
#endif /* GUCEF_CORE_CFILEDATATABLE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_UNIQUE_H
#include "gucef_test_unique.h"
#define GUCEF_TEST_UNIQUE_H
#endif /* GUCEF_TEST_UNIQUE_H ? */

#include "TestDataTableScanSupport.h"
#include "TestFileDataTable.h"

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

/** A fresh path in a directory of the test app under the temporary directory */
static CString
TestFilePath( const char* fileName )
{
    // one directory per process, removed at its end, so that two instances of this application do not share it
    static TESTFW::CScopedTestDir scopedDir( "gucefCORE_TestApp_FileDataTable" );
    const CString dir = scopedDir.GetPath();
    const CString path = CombinePath( dir, fileName );
    if ( FileExists( path ) )
        DeleteFile( path );
    return path;
}

static CFileDataTable::TTableCodecPtr
MakeTestCodec( void )
{
    // made as the registry hands codecs out: a typed shared pointer, held as the basic one
    CTSharedPtr< CITableCodec, MT::CMutex > codec( GUCEF_NEW CORETEST::CTestTableCodec() );
    return codec;
}

/** id int64 not null, name utf8, amount decimal(12,2); primary and sort key on id when asked */
static CDataTableDefinition
MakeFileDefinition( const char* name, bool keyed, UInt32 rowGroupRowCount )
{
    CDataTableDefinition definition;
    definition.name = name;
    definition.codecType = "dttest";
    definition.schema.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, false ) );
    definition.schema.AddColumn( CDataTableColumnDef( "name", DATATABLE_TYPE_UTF8 ) );
    definition.schema.AddColumn( CDataTableColumnDef::Decimal( "amount", 12, 2 ) );
    if ( keyed )
    {
        CString::StringVector key;
        key.push_back( "id" );
        definition.schema.SetPrimaryKey( key );
        definition.schema.SetSortKey( key );
    }
    definition.storage.rowGroupRowCount = rowGroupRowCount;
    if ( definition.storage.pageRowCount > rowGroupRowCount )
        definition.storage.pageRowCount = rowGroupRowCount;
    return definition;
}

/** Rows id first .. first + count - 1, name "r<id>" (null for every seventh), amount id.25 */
static CDataTableBatchPtr
MakeFileRows( const CDataTableSchemaPtr& schema, CDataTableBufferPool& pool, Int64 first, UInt32 count, const char* prefix = "r" )
{
    CDataTableBatchBuilder builder( schema, pool );
    for ( UInt32 i=0; i<count; ++i )
    {
        const Int64 id = first + (Int64) i;
        CVariant cells[ 3 ];
        cells[ 0 ] = id;
        if ( 0 != id % 7 )
            cells[ 1 ] = ToUtf8String( CString( prefix ) + ToString( id ) );
        CDecimal amount;
        CDecimal::FromUnscaled( id * 100 + 25, 2, amount );
        cells[ 2 ] = CDataTableValues::Decimal( amount );
        builder.AppendRow( cells, 3 );
    }
    CDataTableBatchPtr batch;
    CString error;
    builder.Build( batch, error );
    return batch;
}

static CString::StringVector
ReadRows( CIDataTable& table, const CDataTableQuery& query = CDataTableQuery() )
{
    CString::StringVector rows;
    CDataTableScanPtr scan;
    if ( DATATABLE_STATUS_OK != table.Scan( query, scan ) )
    {
        rows.push_back( "<scan failed: " + table.GetLastErrorMessage() + ">" );
        return rows;
    }
    if ( DATATABLE_STATUS_END != CORETEST::CollectScanRows( *scan, rows ) )
        rows.push_back( "<scan error: " + scan->GetLastErrorMessage() + ">" );
    return rows;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformFileDataTableTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "FileDataTable" );

    GUCEF_TESTFW_TESTCASE( "Test: a missing file is an empty table; the first write creates it" )
        try
        {
            CDataTableBufferPool pool;
            const CString path = TestFilePath( "created.dtt" );
            CFileDataTable table( path, MakeTestCodec(), CValueList(), MakeFileDefinition( "created", true, 64 ), &pool );
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Open() );
            ASSERT_FALSE( table.IsReadOnly() );
            ASSERT_TRUE( path == table.GetFilePath() && "created" == table.GetName() && "dttest" == table.GetDefinition().codecType );
            ASSERT_TRUE( ReadRows( table ).empty() );
            CDataTablePartitionInfo::TPartitionInfoVector partitions;
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.ListPartitions( CDataTableQuery(), true, partitions ) && partitions.empty() );

            CDataTableWriteResult result;
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Write( *MakeFileRows( table.GetSchema(), pool, 0, 100 ), DATATABLE_WRITE_INSERT, result ) );
            ASSERT_TRUE( FileExists( path ) && 100 == result.GetTotalRowsWritten() && path == result.partitions[ 0 ].path );
            CString::StringVector rows = ReadRows( table );
            ASSERT_TRUE( 100 == rows.size() && "0|<null>|0.25|" == rows[ 0 ] && "99|r99|99.25|" == rows[ 99 ] );
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.ListPartitions( CDataTableQuery(), true, partitions ) );
            ASSERT_TRUE( 1 == partitions.size() && 100 == partitions[ 0 ].rowCount && 2 == partitions[ 0 ].rowGroupCount && partitions[ 0 ].fileSize > 0 );
            DeleteFile( path );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: INSERT conflicts leave the file as it was, UPSERT replaces by key, REPLACE_PARTITIONS replaces the file" )
        try
        {
            CDataTableBufferPool pool;
            const CString path = TestFilePath( "keyed.dtt" );
            CFileDataTable table( path, MakeTestCodec(), CValueList(), MakeFileDefinition( "keyed", true, 32 ), &pool );
            CDataTableWriteResult result;
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Write( *MakeFileRows( table.GetSchema(), pool, 10, 50 ), DATATABLE_WRITE_INSERT, result ) );
            const UInt64 size = FileSize( path );
            const CString::StringVector before = ReadRows( table );

            ASSERT_TRUE( DATATABLE_STATUS_CONFLICT == table.Write( *MakeFileRows( table.GetSchema(), pool, 55, 10 ), DATATABLE_WRITE_INSERT, result ) );
            ASSERT_TRUE( size == FileSize( path ) && ReadRows( table ) == before );

            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Write( *MakeFileRows( table.GetSchema(), pool, 0, 20, "u" ), DATATABLE_WRITE_UPSERT, result ) );
            ASSERT_TRUE( 10 == result.partitions[ 0 ].rowsReplaced && 60 == result.partitions[ 0 ].rowsWritten );
            CString::StringVector rows = ReadRows( table );
            ASSERT_TRUE( 60 == rows.size() && "1|u1|1.25|" == rows[ 1 ] && "15|u15|15.25|" == rows[ 15 ] && "20|r20|20.25|" == rows[ 20 ] );

            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Write( *MakeFileRows( table.GetSchema(), pool, 500, 3 ), DATATABLE_WRITE_REPLACE_PARTITIONS, result ) );
            rows = ReadRows( table );
            ASSERT_TRUE( 3 == rows.size() && "500|r500|500.25|" == rows[ 0 ] );
            DeleteFile( path );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: without keys INSERT appends to the file; deletes rewrite it and delete it once empty" )
        try
        {
            CDataTableBufferPool pool;
            const CString path = TestFilePath( "append.dtt" );
            CFileDataTable table( path, MakeTestCodec(), CValueList(), MakeFileDefinition( "append", false, 1000 ), &pool );
            CDataTableWriteResult result;
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Write( *MakeFileRows( table.GetSchema(), pool, 100, 5 ), DATATABLE_WRITE_INSERT, result ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Write( *MakeFileRows( table.GetSchema(), pool, 1, 3 ), DATATABLE_WRITE_INSERT, result ) );
            CString::StringVector rows = ReadRows( table, CDataTableQuery().Select( "id" ) );
            ASSERT_TRUE( 8 == rows.size() && "100|" == rows[ 0 ] && "104|" == rows[ 4 ] && "1|" == rows[ 5 ] && "3|" == rows[ 7 ] );

            ASSERT_TRUE( DATATABLE_STATUS_OK == table.DeleteRows( CDataTableQuery().Where( CDataTablePredicate( "id", DATATABLE_PREDICATE_GREATER, CVariant( (Int64) 101 ) ) ), result ) );
            ASSERT_TRUE( 3 == result.partitions[ 0 ].rowsDeleted );
            rows = ReadRows( table, CDataTableQuery().Select( "id" ) );
            ASSERT_TRUE( 5 == rows.size() && "101|" == rows[ 1 ] );
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.DeleteRows( CDataTableQuery(), result ) );
            ASSERT_FALSE( FileExists( path ) );
            ASSERT_TRUE( ReadRows( table ).empty() );

            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Write( *MakeFileRows( table.GetSchema(), pool, 1, 3 ), DATATABLE_WRITE_INSERT, result ) );
            ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == table.DeletePartitions( CDataTableQuery().WhereEquals( "id", CVariant( (Int64) 1 ) ), result ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.DeletePartitions( CDataTableQuery(), result ) );
            ASSERT_FALSE( FileExists( path ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: a scan holding the file reads the rows it started with while a write replaces the file" )
        try
        {
            CDataTableBufferPool pool;
            const CString path = TestFilePath( "replaced.dtt" );
            CFileDataTable table( path, MakeTestCodec(), CValueList(), MakeFileDefinition( "replaced", true, 1024 ), &pool );
            CDataTableWriteResult result;
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Write( *MakeFileRows( table.GetSchema(), pool, 0, 5000 ), DATATABLE_WRITE_INSERT, result ) );

            CDataTableScanPtr old;
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Scan( CDataTableQuery().Select( "id" ).SetBatchRowCount( 1024 ), old ) );
            CDataTableBatchPtr batch;
            ASSERT_TRUE( DATATABLE_STATUS_OK == old->NextBatch( batch ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Write( *MakeFileRows( table.GetSchema(), pool, 9000, 10 ), DATATABLE_WRITE_REPLACE_PARTITIONS, result ) );
            UInt32 oldRows = batch->GetRowCount();
            while ( DATATABLE_STATUS_OK == old->NextBatch( batch ) )
                oldRows += batch->GetRowCount();
            ASSERT_TRUE( 5000 == oldRows );
            ASSERT_TRUE( 10 == ReadRows( table ).size() );
            DeleteFile( path );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Compact skips a file written in the layout of the table unless forced" )
        try
        {
            CDataTableBufferPool pool;
            const CString path = TestFilePath( "compact.dtt" );
            CFileDataTable table( path, MakeTestCodec(), CValueList(), MakeFileDefinition( "compact", true, 40 ), &pool );
            CDataTableWriteResult result;
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Write( *MakeFileRows( table.GetSchema(), pool, 0, 100 ), DATATABLE_WRITE_INSERT, result ) );
            result.Clear();
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Compact( CDataTableQuery(), false, result ) );
            ASSERT_TRUE( result.partitions.empty() );
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Compact( CDataTableQuery(), true, result ) );
            ASSERT_TRUE( 1 == result.partitions.size() && 100 == result.partitions[ 0 ].rowsWritten );

            // a file written in another layout is rewritten
            CFileDataTable other( path, MakeTestCodec(), CValueList(), MakeFileDefinition( "compact", true, 30 ), &pool );
            result.Clear();
            ASSERT_TRUE( DATATABLE_STATUS_OK == other.Compact( CDataTableQuery(), false, result ) );
            ASSERT_TRUE( 1 == result.partitions.size() );
            CDataTablePartitionInfo::TPartitionInfoVector partitions;
            other.ListPartitions( CDataTableQuery(), true, partitions );
            ASSERT_TRUE( 1 == partitions.size() && 4 == partitions[ 0 ].rowGroupCount );
            DeleteFile( path );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: a table on an input is read only and has the schema of the file" )
        try
        {
            CDataTableBufferPool pool;
            CDataTableSchemaPtr schema = MakeFileDefinition( "input", false, 100 ).CreateSchemaPtr();
            GUCEF::vector< CDataTableBatchPtr > batches;
            batches.push_back( MakeFileRows( schema, pool, 0, 250 ) );
            batches.push_back( MakeFileRows( schema, pool, 250, 250 ) );
            IOAccessPtr input;
            ASSERT_TRUE( DATATABLE_STATUS_OK == CORETEST::WriteTestTableFile( batches, schema, input ) );

            CFileDataTable table( input, MakeTestCodec(), CValueList(), "input", &pool );
            ASSERT_TRUE( table.IsReadOnly() );
            ASSERT_TRUE( !table.GetSchema().IsNULL() && 3 == table.GetSchema()->GetColumnCount() && "amount" == table.GetDefinition().schema.GetColumn( 2 ).name );
            CString::StringVector rows = ReadRows( table );
            ASSERT_TRUE( 500 == rows.size() && "499|r499|499.25|" == rows[ 499 ] );
            CDataTableWriteResult result;
            ASSERT_TRUE( DATATABLE_STATUS_READ_ONLY == table.Write( *batches[ 0 ], DATATABLE_WRITE_INSERT, result ) );
            ASSERT_TRUE( DATATABLE_STATUS_READ_ONLY == table.DeleteRows( CDataTableQuery(), result ) );
            CDataTablePartitionInfo::TPartitionInfoVector partitions;
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.ListPartitions( CDataTableQuery(), true, partitions ) );
            ASSERT_TRUE( 1 == partitions.size() && 500 == partitions[ 0 ].rowCount && 2 == partitions[ 0 ].rowGroupCount );

            // the spans of one row group are slices of one buffer read from the file
            CDataTableScanPtr scan;
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Scan( CDataTableQuery().Select( "id" ).SetBatchRowCount( 1024 ), scan ) );
            CDataTableBatchPtr first;
            ASSERT_TRUE( DATATABLE_STATUS_OK == scan->NextBatch( first ) );
            ASSERT_TRUE( 250 == first->GetRowCount() && GUCEF_NULL != first->GetColumn( 0 ).GetInt64Data() );
            CDataTableScanStats stats;
            scan->GetStats( stats );
            ASSERT_TRUE( 1 == stats.footerCacheHits && stats.pagesInPlace > 0 && stats.bytesRead > 0 );

            // a damaged input
            CDynamicBufferAccess* damaged = GUCEF_NEW CDynamicBufferAccess( GUCEF_NEW CDynamicBuffer(), true );
            IOAccessPtr damagedInput( damaged->CreateSharedPtr() );
            damagedInput->Write( "this is not a table file at all", 1, 31 );
            CFileDataTable broken( damagedInput, MakeTestCodec(), CValueList(), "broken", &pool );
            ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == broken.Open() );
            ASSERT_FALSE( broken.GetLastErrorMessage().IsNULLOrEmpty() );
            ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == broken.Scan( CDataTableQuery(), scan ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: the codec's row group statistics prune; spans of one row group share its buffer; async scans share the file" )
        try
        {
            CDataTableBufferPool pool;
            const CString path = TestFilePath( "stats.dtt" );
            CFileDataTable table( path, MakeTestCodec(), CValueList(), MakeFileDefinition( "stats", true, 2048 ), &pool );
            CDataTableWriteResult result;
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Write( *MakeFileRows( table.GetSchema(), pool, 0, 20480 ), DATATABLE_WRITE_INSERT, result ) );

            CDataTableQuery range;
            range.WhereRange( "id", CVariant( (Int64) 5000 ), true, CVariant( (Int64) 7000 ), false ).SetBatchRowCount( 1024 );
            CDataTableScanPtr scan;
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Scan( range, scan ) );
            CString::StringVector rows;
            ASSERT_TRUE( DATATABLE_STATUS_END == CORETEST::CollectScanRows( *scan, rows ) );
            CDataTableScanStats stats;
            scan->GetStats( stats );
            ASSERT_TRUE( 2000 == rows.size() && 10 == stats.rowGroupsTotal && 8 == stats.rowGroupsPrunedByStatistics );

            // without statistics in the metadata nothing is pruned, the rows are the same
            CValueList noStatistics;
            noStatistics.Set( "statistics", CVariant( false ) );
            CFileDataTable plain( path, MakeTestCodec(), noStatistics, MakeFileDefinition( "stats", true, 2048 ), &pool );
            ASSERT_TRUE( DATATABLE_STATUS_OK == plain.Scan( range, scan ) );
            CString::StringVector plainRows;
            CORETEST::CollectScanRows( *scan, plainRows );
            scan->GetStats( stats );
            ASSERT_TRUE( plainRows == rows && 0 == stats.rowGroupsPrunedByStatistics );

            // two batches of one row group are slices of one buffer
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Scan( CDataTableQuery().Select( "id" ).SetBatchRowCount( 1024 ), scan ) );
            CDataTableBatchPtr a, b;
            ASSERT_TRUE( DATATABLE_STATUS_OK == scan->NextBatch( a ) && DATATABLE_STATUS_OK == scan->NextBatch( b ) );
            ASSERT_TRUE( a->GetColumn( 0 ).GetInt64Data() + 1024 == b->GetColumn( 0 ).GetInt64Data() );

            // async with parallel readers on the one input
            ThreadPoolPtr threadPool = CORETEST::GetDataTableTestThreadPool( "DataTableFileTest", 4 );
            CFutureResult future = table.ScanAsync( CDataTableQuery().SetBatchRowCount( 1024 ).SetMaxParallelism( 4 ), scan, threadPool );
            CString::StringVector asyncRows;
            TDataTableStatus status = DATATABLE_STATUS_OK;
            CDataTableBatchPtr batch;
            while ( DATATABLE_STATUS_OK == ( status = scan->WaitForNextBatch( batch, 30000 ) ) )
                CORETEST::AppendBatchRows( *batch, asyncRows );
            ASSERT_TRUE( DATATABLE_STATUS_END == status );
            ASSERT_TRUE( ReadRows( table ) == asyncRows && 20480 == asyncRows.size() );
            scan.Unlink();
            DeleteFile( path );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
