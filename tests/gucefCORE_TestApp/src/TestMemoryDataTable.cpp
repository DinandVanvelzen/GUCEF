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

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

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
#include "TestMemoryDataTable.h"

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

static const char* MemoryTablePoolName = "DataTableMemoryTest";

/*-------------------------------------------------------------------------*/

/** id int64 not null, name utf8, value float64; a primary key on id and a sort key on id when asked */
static CDataTableDefinition
MakeDefinition( const char* name, bool primaryKey, bool sortKey, UInt32 rowGroupRowCount = 1048576 )
{
    CDataTableDefinition definition;
    definition.name = name;
    definition.schema.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, false ) );
    definition.schema.AddColumn( CDataTableColumnDef( "name", DATATABLE_TYPE_UTF8 ) );
    definition.schema.AddColumn( CDataTableColumnDef( "value", DATATABLE_TYPE_FLOAT64 ) );
    CString::StringVector key;
    key.push_back( "id" );
    if ( primaryKey )
        definition.schema.SetPrimaryKey( key );
    if ( sortKey )
        definition.schema.SetSortKey( key );
    definition.storage.rowGroupRowCount = rowGroupRowCount;
    if ( definition.storage.pageRowCount > rowGroupRowCount )
        definition.storage.pageRowCount = rowGroupRowCount;
    return definition;
}

/** Rows of ( id, name, value ); a name of GUCEF_NULL is null */
static CDataTableBatchPtr
MakeRows( const CDataTableSchemaPtr& schema, CDataTableBufferPool& pool, const Int64* ids, const char* const* names, const Float64* values, UInt32 count )
{
    CDataTableBatchBuilder builder( schema, pool );
    for ( UInt32 i=0; i<count; ++i )
    {
        CVariant cells[ 3 ];
        cells[ 0 ] = ids[ i ];
        if ( GUCEF_NULL != names[ i ] )
            cells[ 1 ] = CUtf8String( names[ i ] );
        cells[ 2 ] = values[ i ];
        builder.AppendRow( cells, 3 );
    }
    CDataTableBatchPtr batch;
    CString error;
    builder.Build( batch, error );
    return batch;
}

/** Rows id 0 .. count - 1 with name "n<id>" and value id * 0.5 */
static CDataTableBatchPtr
MakeSequenceRows( const CDataTableSchemaPtr& schema, CDataTableBufferPool& pool, Int64 first, UInt32 count )
{
    CDataTableBatchBuilder builder( schema, pool );
    for ( UInt32 i=0; i<count; ++i )
    {
        CVariant cells[ 3 ];
        cells[ 0 ] = first + (Int64) i;
        cells[ 1 ] = ToUtf8String( "n" + ToString( first + (Int64) i ) );
        cells[ 2 ] = (Float64) ( first + (Int64) i ) * 0.5;
        builder.AppendRow( cells, 3 );
    }
    CDataTableBatchPtr batch;
    CString error;
    builder.Build( batch, error );
    return batch;
}

/** Every row of the table in scan order as "id|name|value|" */
static CString::StringVector
ReadAll( CIDataTable& table, const CDataTableQuery& query = CDataTableQuery() )
{
    CString::StringVector rows;
    CDataTableScanPtr scan;
    if ( DATATABLE_STATUS_OK != table.Scan( query, scan ) )
    {
        rows.push_back( "<scan failed: " + table.GetLastErrorMessage() + ">" );
        return rows;
    }
    CORETEST::CollectScanRows( *scan, rows );
    return rows;
}

/** The task status a future ended in, waiting at most the timeout */
static TTaskStatus
AwaitFuture( const CFutureResult& future, UInt32 timeoutMs, CTaskPtr& task )
{
    const UInt64 start = MT::PrecisionTickCount();
    while ( !future.IsReady() )
    {
        if ( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) > timeoutMs )
            return TTaskStatus::TASKSTATUS_UNDEFINED;
        MT::PrecisionDelay( 2 );
    }
    task = future.GetResult( 0 );
    return task.IsNULL() ? TTaskStatus::TASKSTATUS_UNDEFINED : task->GetTaskStatus();
}

/*-------------------------------------------------------------------------*/

/** Counts the partition events of a table */
class CTableEventCounter : public CObserver
{
    public:

    explicit CTableEventCounter( CIDataTable& table ) : CObserver(), written( 0 ), deleted( 0 ), lastRowCount( 0 )
    {
        TEventCallback onWritten( this, &CTableEventCounter::OnWritten );
        TEventCallback onDeleted( this, &CTableEventCounter::OnDeleted );
        SubscribeTo( &table, CIDataTable::PartitionWrittenEvent, onWritten );
        SubscribeTo( &table, CIDataTable::PartitionDeletedEvent, onDeleted );
    }

    UInt32 written;
    UInt32 deleted;
    UInt64 lastRowCount;

    private:

    typedef CTEventHandlerFunctor< CTableEventCounter > TEventCallback;

    void OnWritten( CNotifier*, const CEvent&, CICloneable* eventData )
    {
        ++written;
        const CDataTablePartitionInfo* info = static_cast< const CDataTablePartitionInfo* >( eventData );
        if ( GUCEF_NULL != info )
            lastRowCount = info->rowCount;
    }

    void OnDeleted( CNotifier*, const CEvent&, CICloneable* ) { ++deleted; }
};

/*-------------------------------------------------------------------------*/

/** Upserts rounds of rows into a table on a thread of its own while other threads scan it */
class CTableWriterThread : public MT::CActiveObject
{
    public:

    CTableWriterThread( CMemoryDataTable& table, CDataTableBufferPool& pool, UInt32 rounds )
        : MT::CActiveObject(), m_table( table ), m_pool( pool ), m_rounds( rounds ), failures( 0 ), done( 0 ) {}

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        for ( UInt32 r=0; r<m_rounds; ++r )
        {
            // the table grows by 500 rows a round, the first rows are rewritten each round
            CDataTableBatchPtr rows = MakeSequenceRows( m_table.GetSchema(), m_pool, (Int64) r * 500 - 250 < 0 ? 0 : (Int64) r * 500 - 250, r == 0 ? 500 : 750 );
            CDataTableWriteResult result;
            if ( DATATABLE_STATUS_OK != m_table.Write( *rows, DATATABLE_WRITE_UPSERT, result ) )
                failures.FetchAdd( 1 );
        }
        done.Store( 1 );
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    CMemoryDataTable& m_table;
    CDataTableBufferPool& m_pool;
    UInt32 m_rounds;
    MT::CTAtomic< UInt32 > failures;
    MT::CTAtomic< UInt32 > done;
};

/** Scans a table in a loop: every scan must see a whole snapshot, ids 0 .. n - 1 in order */
class CTableScannerThread : public MT::CActiveObject
{
    public:

    CTableScannerThread( CMemoryDataTable& table, MT::CTAtomic< UInt32 >& stop )
        : MT::CActiveObject(), m_table( table ), m_stop( stop ), scans( 0 ), failures( 0 ), done( 0 ) {}

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        while ( 0 == m_stop.Load() )
        {
            CDataTableScanPtr scan;
            if ( DATATABLE_STATUS_OK != m_table.Scan( CDataTableQuery().Select( "id" ), scan ) )
            {
                failures.FetchAdd( 1 );
                continue;
            }
            Int64 expected = 0;
            CDataTableBatchPtr batch;
            TDataTableStatus status = DATATABLE_STATUS_OK;
            while ( DATATABLE_STATUS_OK == ( status = scan->NextBatch( batch ) ) )
            {
                const Int64* ids = batch->GetColumn( 0 ).GetInt64Data();
                for ( UInt32 i=0; i<batch->GetRowCount(); ++i )
                    if ( ids[ i ] != expected++ )
                        failures.FetchAdd( 1 );
            }
            if ( DATATABLE_STATUS_END != status || 0 != expected % 250 )
                failures.FetchAdd( 1 );
            scans.FetchAdd( 1 );
        }
        done.Store( 1 );
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    CMemoryDataTable& m_table;
    MT::CTAtomic< UInt32 >& m_stop;
    MT::CTAtomic< UInt32 > scans;
    MT::CTAtomic< UInt32 > failures;
    MT::CTAtomic< UInt32 > done;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMemoryDataTableTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "MemoryDataTable" );

    ThreadPoolPtr threadPool = CORETEST::GetDataTableTestThreadPool( MemoryTablePoolName, 2 );

    GUCEF_TESTFW_TESTCASE( "Test: INSERT without keys appends batches; rows are checked against the schema" )
        try
        {
            CDataTableBufferPool pool;
            CMemoryDataTable table( MakeDefinition( "append", false, false, 4 ), &pool );
            ASSERT_TRUE( table.IsValid() && !table.IsReadOnly() && "append" == table.GetName() );
            const Int64 ids[] = { 5, 3, 9 };
            const char* names[] = { "five", GUCEF_NULL, "nine" };
            const Float64 values[] = { 0.5, 0.25, -1.0 };
            CDataTableWriteResult result;
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Write( *MakeRows( table.GetSchema(), pool, ids, names, values, 3 ), DATATABLE_WRITE_INSERT, result ) );
            ASSERT_TRUE( 1 == result.partitions.size() && 3 == result.GetTotalRowsWritten() && DATATABLE_STATUS_OK == result.GetOverallStatus() );
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Write( *MakeRows( table.GetSchema(), pool, ids, names, values, 3 ), DATATABLE_WRITE_INSERT, result ) );
            ASSERT_TRUE( 6 == table.GetRowCount() && 2 == table.GetBatchCount() );
            CString::StringVector rows = ReadAll( table );
            ASSERT_TRUE( 6 == rows.size() && "5|five|0.5|" == rows[ 0 ] && "3|<null>|0.25|" == rows[ 1 ] && rows[ 0 ] == rows[ 3 ] );

            // more rows than a row group holds are cut into row groups
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Write( *MakeSequenceRows( table.GetSchema(), pool, 100, 10 ), DATATABLE_WRITE_INSERT, result ) );
            ASSERT_TRUE( 16 == table.GetRowCount() && 5 == table.GetBatchCount() );

            // the rows of a projection: the missing nullable column is null; a selection writes the selected rows
            CDataTableSchema partial;
            partial.AddColumn( CDataTableColumnDef( "value", DATATABLE_TYPE_FLOAT64 ) );
            partial.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, false ) );
            CDataTableBatchBuilder builder( CreateDataTableSchemaPtr( partial ), pool );
            for ( Int64 i=0; i<4; ++i )
            {
                builder.GetColumn( 0 ).AppendFloat64( (Float64) i );
                builder.GetColumn( 1 ).AppendInt64( 1000 + i );
            }
            CDataTableBatchPtr projected;
            CString error;
            builder.Build( projected, error );
            CDataTableBufferPtr selection;
            pool.Acquire( 8, selection );
            static_cast< UInt32* >( selection->GetData() )[ 0 ] = 1;
            static_cast< UInt32* >( selection->GetData() )[ 1 ] = 3;
            projected.GetWritable()->SetSelection( static_cast< const UInt32* >( selection->GetData() ), 2, selection );
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Write( *projected, DATATABLE_WRITE_INSERT, result ) );
            rows = ReadAll( table, CDataTableQuery().Where( CDataTablePredicate( "id", DATATABLE_PREDICATE_GREATER_OR_EQUAL, CVariant( (Int64) 1000 ) ) ) );
            ASSERT_TRUE( 2 == rows.size() && "1001|<null>|1|" == rows[ 0 ] && "1003|<null>|3|" == rows[ 1 ] );

            // a missing column that is not nullable, and a column of another type, are refused
            CDataTableSchema noId;
            noId.AddColumn( CDataTableColumnDef( "name", DATATABLE_TYPE_UTF8 ) );
            CDataTableBatchBuilder noIdBuilder( CreateDataTableSchemaPtr( noId ), pool );
            noIdBuilder.GetColumn( 0 ).AppendString( "x" );
            CDataTableBatchPtr noIdRows;
            noIdBuilder.Build( noIdRows, error );
            ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == table.Write( *noIdRows, DATATABLE_WRITE_INSERT, result ) );
            ASSERT_FALSE( table.GetLastErrorMessage().IsNULLOrEmpty() );
            CDataTableSchema wrongType;
            wrongType.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT32, false ) );
            CDataTableBatchBuilder wrongBuilder( CreateDataTableSchemaPtr( wrongType ), pool );
            wrongBuilder.GetColumn( 0 ).AppendInt64( 1 );
            CDataTableBatchPtr wrongRows;
            wrongBuilder.Build( wrongRows, error );
            ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == table.Write( *wrongRows, DATATABLE_WRITE_INSERT, result ) );
            ASSERT_TRUE( 18 == table.GetRowCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: primary keys: INSERT conflicts leave the table unchanged, UPSERT replaces with the last row of a key" )
        try
        {
            CDataTableBufferPool pool;
            CMemoryDataTable table( MakeDefinition( "keyed", true, true, 3 ), &pool );
            const Int64 ids[] = { 5, 1, 3 };
            const char* names[] = { "five", "one", "three" };
            const Float64 values[] = { 5.0, 1.0, 3.0 };
            CDataTableWriteResult result;
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Write( *MakeRows( table.GetSchema(), pool, ids, names, values, 3 ), DATATABLE_WRITE_INSERT, result ) );
            CString::StringVector before = ReadAll( table );
            ASSERT_TRUE( 3 == before.size() && "1|one|1|" == before[ 0 ] && "5|five|5|" == before[ 2 ] );

            const Int64 clashIds[] = { 7, 3 };
            const char* clashNames[] = { "seven", "again" };
            const Float64 clashValues[] = { 7.0, 9.0 };
            ASSERT_TRUE( DATATABLE_STATUS_CONFLICT == table.Write( *MakeRows( table.GetSchema(), pool, clashIds, clashNames, clashValues, 2 ), DATATABLE_WRITE_INSERT, result ) );
            ASSERT_TRUE( ReadAll( table ) == before );
            const Int64 twiceIds[] = { 8, 8 };
            ASSERT_TRUE( DATATABLE_STATUS_CONFLICT == table.Write( *MakeRows( table.GetSchema(), pool, twiceIds, clashNames, clashValues, 2 ), DATATABLE_WRITE_INSERT, result ) );
            ASSERT_TRUE( ReadAll( table ) == before );

            const Int64 upsertIds[] = { 3, 2, 3 };
            const char* upsertNames[] = { "first", "two", "last" };
            const Float64 upsertValues[] = { 30.0, 2.0, 33.0 };
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Write( *MakeRows( table.GetSchema(), pool, upsertIds, upsertNames, upsertValues, 3 ), DATATABLE_WRITE_UPSERT, result ) );
            ASSERT_TRUE( 1 == result.partitions[ 0 ].rowsReplaced && 4 == result.partitions[ 0 ].rowsWritten );
            CString::StringVector after = ReadAll( table );
            ASSERT_TRUE( 4 == after.size() && "1|one|1|" == after[ 0 ] && "2|two|2|" == after[ 1 ] && "3|last|33|" == after[ 2 ] && "5|five|5|" == after[ 3 ] );
            ASSERT_TRUE( 2 == table.GetBatchCount() );

            CMemoryDataTable unkeyed( MakeDefinition( "unkeyed", false, false ), &pool );
            ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == unkeyed.Write( *MakeRows( unkeyed.GetSchema(), pool, ids, names, values, 3 ), DATATABLE_WRITE_UPSERT, result ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: rows are kept in sort key order, nulls last; REPLACE_PARTITIONS replaces every row" )
        try
        {
            CDataTableBufferPool pool;
            CDataTableDefinition definition = MakeDefinition( "sorted", false, false );
            CString::StringVector sortKey;
            sortKey.push_back( "value" );
            definition.schema.SetSortKey( sortKey );
            PulseGeneratorPtr pulseGenerator = CPulseGenerator::CreateSharedObj();
            CMemoryDataTable table( definition, &pool, pulseGenerator );
            const Int64 ids[] = { 1, 2, 3, 4 };
            const char* names[] = { "a", "b", "c", "d" };
            const Float64 values[] = { 3.0, -1.0, 2.0, 0.0 };
            CDataTableWriteResult result;
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Write( *MakeRows( table.GetSchema(), pool, ids, names, values, 4 ), DATATABLE_WRITE_INSERT, result ) );
            const Int64 moreIds[] = { 5 };
            const char* moreNames[] = { "e" };
            const Float64 moreValues[] = { 1.0 };
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Write( *MakeRows( table.GetSchema(), pool, moreIds, moreNames, moreValues, 1 ), DATATABLE_WRITE_INSERT, result ) );
            CString::StringVector rows = ReadAll( table, CDataTableQuery().Select( "id" ) );
            ASSERT_TRUE( 5 == rows.size() && "2|" == rows[ 0 ] && "4|" == rows[ 1 ] && "5|" == rows[ 2 ] && "3|" == rows[ 3 ] && "1|" == rows[ 4 ] );

            pulseGenerator->ForceDirectPulse();     // the events of the writes before go to no one
            CTableEventCounter events( table );
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Write( *MakeRows( table.GetSchema(), pool, moreIds, moreNames, moreValues, 1 ), DATATABLE_WRITE_REPLACE_PARTITIONS, result ) );
            pulseGenerator->ForceDirectPulse();
            ASSERT_TRUE( 1 == table.GetRowCount() && 1 == events.written && 1 == events.lastRowCount );
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Write( *MakeRows( table.GetSchema(), pool, moreIds, moreNames, moreValues, 0 ), DATATABLE_WRITE_REPLACE_PARTITIONS, result ) );
            pulseGenerator->ForceDirectPulse();
            ASSERT_TRUE( 0 == table.GetRowCount() && 1 == events.deleted );
            ASSERT_TRUE( ReadAll( table ).empty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: DeleteRows, DeletePartitions, ListPartitions, Compact and read only tables" )
        try
        {
            CDataTableBufferPool pool;
            CDataTableDefinition definition = MakeDefinition( "ops", false, false, 50 );
            CMemoryDataTable table( definition, &pool );
            CDataTableWriteResult result;
            for ( Int64 i=0; i<10; ++i )
                ASSERT_TRUE( DATATABLE_STATUS_OK == table.Write( *MakeSequenceRows( table.GetSchema(), pool, i * 20, 20 ), DATATABLE_WRITE_INSERT, result ) );
            ASSERT_TRUE( 200 == table.GetRowCount() && 10 == table.GetBatchCount() );

            ASSERT_TRUE( DATATABLE_STATUS_OK == table.DeleteRows( CDataTableQuery().WhereRange( "id", CVariant( (Int64) 10 ), true, CVariant( (Int64) 30 ), false ), result ) );
            ASSERT_TRUE( 20 == result.partitions[ 0 ].rowsDeleted && 180 == table.GetRowCount() );
            ASSERT_TRUE( ReadAll( table, CDataTableQuery().WhereRange( "id", CVariant( (Int64) 10 ), true, CVariant( (Int64) 30 ), false ) ).empty() );
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.DeleteRows( CDataTableQuery().WhereEquals( "id", CVariant( (Int64) 100000 ) ), result ) );
            ASSERT_TRUE( 0 == result.partitions[ 0 ].rowsDeleted && 180 == table.GetRowCount() );
            ASSERT_TRUE( DATATABLE_STATUS_NOT_FOUND == table.DeleteRows( CDataTableQuery().WhereEquals( "nope", CVariant( (Int64) 1 ) ), result ) );

            CDataTablePartitionInfo::TPartitionInfoVector partitions;
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.ListPartitions( CDataTableQuery(), true, partitions ) );
            ASSERT_TRUE( 1 == partitions.size() && 180 == partitions[ 0 ].rowCount && partitions[ 0 ].hasFileStats && partitions[ 0 ].fileSize > 0 );
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.ListPartitions( CDataTableQuery().WhereEquals( "id", CVariant( (Int64) 5000 ) ), false, partitions ) );
            ASSERT_TRUE( partitions.empty() );

            // compaction: the appended batches become canonical row groups, once
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Compact( CDataTableQuery(), false, result ) );
            ASSERT_TRUE( 4 == table.GetBatchCount() && 180 == table.GetRowCount() );
            result.Clear();
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Compact( CDataTableQuery(), false, result ) );
            ASSERT_TRUE( result.partitions.empty() );
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Compact( CDataTableQuery(), true, result ) );
            ASSERT_TRUE( 1 == result.partitions.size() );

            table.SetReadOnly( true );
            ASSERT_TRUE( table.IsReadOnly() );
            ASSERT_TRUE( DATATABLE_STATUS_READ_ONLY == table.Write( *MakeSequenceRows( table.GetSchema(), pool, 0, 1 ), DATATABLE_WRITE_INSERT, result ) );
            ASSERT_TRUE( DATATABLE_STATUS_READ_ONLY == table.DeleteRows( CDataTableQuery(), result ) );
            table.SetReadOnly( false );

            ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == table.DeletePartitions( CDataTableQuery().WhereEquals( "id", CVariant( (Int64) 1 ) ), result ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.DeletePartitions( CDataTableQuery(), result ) );
            ASSERT_TRUE( 0 == table.GetRowCount() && 180 == result.partitions[ 0 ].rowsDeleted );
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.ListPartitions( CDataTableQuery(), false, partitions ) );
            ASSERT_TRUE( partitions.empty() );

            // an invalid definition refuses every call
            CDataTableDefinition invalid;
            invalid.name = ".hidden";
            CMemoryDataTable refused( invalid, &pool );
            ASSERT_FALSE( refused.IsValid() );
            CDataTableScanPtr scan;
            ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == refused.Scan( CDataTableQuery(), scan ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: row group statistics made on insert let scans prune" )
        try
        {
            CDataTableBufferPool pool;
            CMemoryDataTable table( MakeDefinition( "pruned", true, true, 100 ), &pool );
            CDataTableWriteResult result;
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Write( *MakeSequenceRows( table.GetSchema(), pool, 0, 1000 ), DATATABLE_WRITE_UPSERT, result ) );
            CDataTableScanPtr scan;
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Scan( CDataTableQuery().WhereRange( "id", CVariant( (Int64) 250 ), true, CVariant( (Int64) 349 ), true ), scan ) );
            CString::StringVector rows;
            ASSERT_TRUE( DATATABLE_STATUS_END == CORETEST::CollectScanRows( *scan, rows ) );
            CDataTableScanStats stats;
            scan->GetStats( stats );
            ASSERT_TRUE( 100 == rows.size() && 10 == stats.rowGroupsTotal && 8 == stats.rowGroupsPrunedByStatistics && 200 == stats.rowsScanned );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: async writes, deletes, listings and compactions complete their futures with the results" )
        try
        {
            CDataTableBufferPool pool;
            CMemoryDataTable table( MakeDefinition( "async", true, true, 64 ), &pool );
            CTaskPtr task;

            CFutureResult written = table.WriteAsync( MakeSequenceRows( table.GetSchema(), pool, 0, 300 ), DATATABLE_WRITE_INSERT, threadPool );
            ASSERT_TRUE( TTaskStatus::TASKSTATUS_FINISHED == AwaitFuture( written, 10000, task ) );
            const CDataTableWriteResult* writeResult = static_cast< const CDataTableWriteResult* >( task->GetTaskData() );
            ASSERT_TRUE( GUCEF_NULL != writeResult && 300 == writeResult->GetTotalRowsWritten() );

            CFutureResult conflict = table.WriteAsync( MakeSequenceRows( table.GetSchema(), pool, 10, 5 ), DATATABLE_WRITE_INSERT, threadPool );
            ASSERT_TRUE( TTaskStatus::TASKSTATUS_STARTUP_FAILED == AwaitFuture( conflict, 10000, task ) );
            ASSERT_TRUE( task->GetTaskStatusExtraInfo().HasSubstr( "CONFLICT" ) >= 0 );

            CFutureResult deleted = table.DeleteRowsAsync( CDataTableQuery().Where( CDataTablePredicate( "id", DATATABLE_PREDICATE_LESS, CVariant( (Int64) 100 ) ) ), threadPool );
            ASSERT_TRUE( TTaskStatus::TASKSTATUS_FINISHED == AwaitFuture( deleted, 10000, task ) );
            const CDataTableWriteResult* deleteResult = static_cast< const CDataTableWriteResult* >( task->GetTaskData() );
            ASSERT_TRUE( GUCEF_NULL != deleteResult && 100 == deleteResult->partitions[ 0 ].rowsDeleted );

            CFutureResult listed = table.ListPartitionsAsync( CDataTableQuery(), true, ThreadPoolPtr() );
            ASSERT_TRUE( TTaskStatus::TASKSTATUS_FINISHED == AwaitFuture( listed, 10000, task ) );
            const CDataTablePartitionInfoList* list = static_cast< const CDataTablePartitionInfoList* >( task->GetTaskData() );
            ASSERT_TRUE( GUCEF_NULL != list && 1 == list->partitions.size() && 200 == list->partitions[ 0 ].rowCount );

            CFutureResult compacted = table.CompactAsync( CDataTableQuery(), true, threadPool );
            ASSERT_TRUE( TTaskStatus::TASKSTATUS_FINISHED == AwaitFuture( compacted, 10000, task ) );
            CFutureResult partitionsDeleted = table.DeletePartitionsAsync( CDataTableQuery(), threadPool );
            ASSERT_TRUE( TTaskStatus::TASKSTATUS_FINISHED == AwaitFuture( partitionsDeleted, 10000, task ) );
            ASSERT_TRUE( 0 == table.GetRowCount() );

            CDataTableScanPtr scan;
            CFutureResult scanned = table.ScanAsync( CDataTableQuery(), scan, threadPool );
            CDataTableBatchPtr batch;
            ASSERT_TRUE( DATATABLE_STATUS_END == scan->WaitForNextBatch( batch, 10000 ) );
            ASSERT_TRUE( TTaskStatus::TASKSTATUS_FINISHED == AwaitFuture( scanned, 10000, task ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: scans see whole snapshots while another thread writes" )
        try
        {
            CDataTableBufferPool pool;
            CMemoryDataTable table( MakeDefinition( "concurrent", true, true, 300 ), &pool );
            MT::CTAtomic< UInt32 > stop( 0 );
            CTableWriterThread* writer = GUCEF_NEW CTableWriterThread( table, pool, 40 );
            CTableScannerThread* scanners[ 2 ] = { GUCEF_NEW CTableScannerThread( table, stop ), GUCEF_NEW CTableScannerThread( table, stop ) };
            ASSERT_TRUE( scanners[ 0 ]->Activate() && scanners[ 1 ]->Activate() && writer->Activate() );
            UInt32 waited = 0;
            while ( 0 == writer->done.Load() && waited < 60000 )
            {
                MT::PrecisionDelay( 5 );
                waited += 5;
            }
            stop.Store( 1 );
            writer->Deactivate( false, true );
            for ( UInt32 i=0; i<2; ++i )
            {
                waited = 0;
                while ( 0 == scanners[ i ]->done.Load() && waited < 60000 )
                {
                    MT::PrecisionDelay( 5 );
                    waited += 5;
                }
                scanners[ i ]->Deactivate( false, true );
            }
            ASSERT_TRUE( 1 == writer->done.Load() && 0 == writer->failures.Load() );
            ASSERT_TRUE( 0 == scanners[ 0 ]->failures.Load() && 0 == scanners[ 1 ]->failures.Load() );
            ASSERT_TRUE( scanners[ 0 ]->scans.Load() > 0 && scanners[ 1 ]->scans.Load() > 0 );
            ASSERT_TRUE( 40 * 500 == table.GetRowCount() );
            GUCEF_DELETE writer;
            GUCEF_DELETE scanners[ 0 ];
            GUCEF_DELETE scanners[ 1 ];
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
