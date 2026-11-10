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

#include <algorithm>
#if ( defined( __GLIBC__ ) )
#include <malloc.h>
#endif

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

#ifndef GUCEF_CORE_CDATATABLEROWCURSOR_H
#include "gucefCORE_CDataTableRowCursor.h"
#define GUCEF_CORE_CDATATABLEROWCURSOR_H
#endif /* GUCEF_CORE_CDATATABLEROWCURSOR_H ? */

#include "TestAllocationCounter.h"
#include "TestDataTableScanSupport.h"
#include "TestDataTableScan.h"

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

static const char* ScanTestPoolName = "DataTableScanTest";
static const UInt32 ScanTestThreads = 6;
static const UInt32 ScanTestTimeoutMs = 60000;

/*-------------------------------------------------------------------------*/

/** id int64 not nullable, then a column of every type */
static CDataTableSchemaPtr
MakeIdAllTypesSchema( void )
{
    CDataTableSchemaPtr allTypes = CORETEST::MakeAllTypesSchema();
    CDataTableSchema schema;
    schema.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, false ) );
    for ( UInt32 c=0; c<allTypes->GetColumnCount(); ++c )
        schema.AddColumn( allTypes->GetColumn( c ) );
    return CreateDataTableSchemaPtr( schema );
}

/** Pulls an async scan to its end with a timeout; PENDING when it did not end in time */
static TDataTableStatus
DrainAsync( CDataTableScan& scan, CString::StringVector& rows, UInt32* batchCount = GUCEF_NULL )
{
    const UInt64 start = MT::PrecisionTickCount();
    while ( true )
    {
        CDataTableBatchPtr batch;
        const TDataTableStatus status = scan.WaitForNextBatch( batch, 1000 );
        if ( DATATABLE_STATUS_OK == status )
        {
            if ( GUCEF_NULL != batchCount )
                ++(*batchCount);
            CORETEST::AppendBatchRows( *batch, rows );
            continue;
        }
        if ( DATATABLE_STATUS_PENDING != status )
            return status;
        if ( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) > ScanTestTimeoutMs )
            return DATATABLE_STATUS_PENDING;
    }
}

/** The ids of rows whose text starts with the id */
static GUCEF::vector< Int64 >
IdsOf( const CString::StringVector& rows )
{
    GUCEF::vector< Int64 > ids;
    for ( UInt32 i=0; i<(UInt32) rows.size(); ++i )
        ids.push_back( StringToInt64( rows[ i ].SubstrToChar( '|', true ) ) );
    return ids;
}

/** True when the ids are first, first + 1, ... */
static bool
IsConsecutive( const GUCEF::vector< Int64 >& ids, Int64 first, UInt64 count )
{
    if ( ids.size() != count )
        return false;
    for ( UInt32 i=0; i<(UInt32) ids.size(); ++i )
        if ( ids[ i ] != first + (Int64) i )
            return false;
    return true;
}

/** Waits until the condition holds or the time passes */
static bool
WaitForReadersGone( CORETEST::STestSourceCounters& counters, UInt32 timeoutMs )
{
    const UInt64 start = MT::PrecisionTickCount();
    while ( 0 != counters.readersAlive.Load() )
    {
        if ( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) > timeoutMs )
            return false;
        MT::PrecisionDelay( 5 );
    }
    return true;
}

/** The task status the future of a scan ended in, TASKSTATUS_UNDEFINED when it did not end in time */
static TTaskStatus
WaitForFuture( const CFutureResult& future, UInt32 timeoutMs )
{
    const UInt64 start = MT::PrecisionTickCount();
    while ( !future.IsReady() )
    {
        if ( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) > timeoutMs )
            return TTaskStatus::TASKSTATUS_UNDEFINED;
        MT::PrecisionDelay( 2 );
    }
    CTaskPtr task = future.GetResult( 0 );
    return task.IsNULL() ? TTaskStatus::TASKSTATUS_UNDEFINED : task->GetTaskStatus();
}

/*-------------------------------------------------------------------------*/

/** Heap bytes in use by the process where the C library can say; -1 where it cannot or a sanitizer owns the allocator */
static Int64
HeapBytesInUse( void )
{
    #if ( defined( __GLIBC__ ) && ( __GLIBC__ > 2 || ( 2 == __GLIBC__ && __GLIBC_MINOR__ >= 33 ) ) )
    const struct mallinfo2 info = mallinfo2();
    const Int64 bytes = (Int64) info.uordblks + (Int64) info.hblkhd;
    return bytes > 0 ? bytes : -1;
    #else
    return -1;
    #endif
}

/** Counts the events of a scan, delivered when the test pumps the pulse generator */
class CScanEventCounter : public CObserver
{
    public:

    CScanEventCounter( CDataTableScan& scan )
        : CObserver()
        , available( 0 )
        , finished( 0 )
        , failed( 0 )
        , finishedRows( 0 )
        , error()
    {
        TEventCallback onAvailable( this, &CScanEventCounter::OnAvailable );
        TEventCallback onFinished( this, &CScanEventCounter::OnFinished );
        TEventCallback onFailed( this, &CScanEventCounter::OnFailed );
        SubscribeTo( &scan, CDataTableScan::ScanBatchesAvailableEvent, onAvailable );
        SubscribeTo( &scan, CDataTableScan::ScanFinishedEvent, onFinished );
        SubscribeTo( &scan, CDataTableScan::ScanFailedEvent, onFailed );
    }

    UInt32 available;
    UInt32 finished;
    UInt32 failed;
    UInt64 finishedRows;
    CString error;

    private:

    typedef CTEventHandlerFunctor< CScanEventCounter > TEventCallback;

    void OnAvailable( CNotifier* notifier, const CEvent& eventId, CICloneable* eventData ) { ++available; }

    void OnFinished( CNotifier* notifier, const CEvent& eventId, CICloneable* eventData )
    {
        ++finished;
        const CDataTableScanStats* stats = static_cast< const CDataTableScanStats* >( eventData );
        if ( GUCEF_NULL != stats )
            finishedRows = stats->rowsSelected;
    }

    void OnFailed( CNotifier* notifier, const CEvent& eventId, CICloneable* eventData )
    {
        ++failed;
        const TCloneableString* text = static_cast< const TCloneableString* >( eventData );
        if ( GUCEF_NULL != text )
            error = *text;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDataTableScanTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "DataTableScan" );

    ThreadPoolPtr threadPool = CORETEST::GetDataTableTestThreadPool( ScanTestPoolName, ScanTestThreads );

    GUCEF_TESTFW_TESTCASE( "Test: sync and async scans give the same rows for every column type and null pattern" )
        try
        {
            static const UInt32 nullPercents[] = { 0, 35, 100 };
            for ( UInt32 n=0; n<3; ++n )
            {
                CDataTableBufferPool pool;
                CORETEST::CTestFileSet files;
                ASSERT_TRUE( files.Make( MakeIdAllTypesSchema(), pool, 11 + n, 3, 4, 1100, nullPercents[ n ] ) );
                CORETEST::STestSourceCounters counters;

                CDataTableScanPtr sync = files.MakeScan( pool, counters );
                ASSERT_TRUE( DATATABLE_STATUS_OK == sync->Start( CDataTableQuery().SetBatchRowCount( 1024 ) ) );
                ASSERT_FALSE( sync->IsAsync() );
                CString::StringVector syncRows;
                ASSERT_TRUE( DATATABLE_STATUS_END == CORETEST::CollectScanRows( *sync, syncRows ) );
                ASSERT_TRUE( 3 * 4 * 1100 == syncRows.size() );
                ASSERT_TRUE( IsConsecutive( IdsOf( syncRows ), 0, 3 * 4 * 1100 ) );

                CDataTableScanPtr async = files.MakeScan( pool, counters );
                CFutureResult future = async->StartAsync( CDataTableQuery().SetBatchRowCount( 1024 ).SetMaxParallelism( 4 ), threadPool );
                ASSERT_TRUE( async->IsAsync() );
                CString::StringVector asyncRows;
                ASSERT_TRUE( DATATABLE_STATUS_END == DrainAsync( *async, asyncRows ) );
                ASSERT_TRUE( syncRows == asyncRows );
                ASSERT_TRUE( TTaskStatus::TASKSTATUS_FINISHED == WaitForFuture( future, 5000 ) );

                CDataTableScanPtr unordered = files.MakeScan( pool, counters );
                unordered->StartAsync( CDataTableQuery().SetBatchRowCount( 1024 ).SetOrderedDelivery( false ).SetMaxParallelism( 4 ), threadPool );
                CString::StringVector unorderedRows;
                ASSERT_TRUE( DATATABLE_STATUS_END == DrainAsync( *unordered, unorderedRows ) );
                std::sort( unorderedRows.begin(), unorderedRows.end() );
                std::sort( asyncRows.begin(), asyncRows.end() );
                ASSERT_TRUE( unorderedRows == asyncRows );

                CDataTableScanStats stats;
                sync->GetStats( stats );
                ASSERT_TRUE( 3 == stats.filesListed && 12 == stats.rowGroupsTotal && 3 * 4 * 1100 == stats.rowsSelected );
                ASSERT_TRUE( 1 == stats.footerCacheHits && 3 == stats.filesOpened );
                ASSERT_TRUE( stats.pagesInPlace > 0 && stats.bytesRead > 0 );

                sync.Unlink();
                async.Unlink();
                unordered.Unlink();
                ASSERT_TRUE( WaitForReadersGone( counters, 5000 ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: a row cursor iterates the selected rows of a scan, sync and async" )
        try
        {
            CDataTableBufferPool pool;
            CORETEST::CTestFileSet files;
            CDataTableSchema schema;
            schema.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, false ) );
            ASSERT_TRUE( files.Make( CreateDataTableSchemaPtr( schema ), pool, 7, 2, 3, 1500, 0 ) );
            for ( UInt32 async=0; async<2; ++async )
            {
                CORETEST::STestSourceCounters counters;
                CDataTableScanPtr scan = files.MakeScan( pool, counters );
                CDataTableQuery query;
                query.Where( CDataTablePredicate( "id", CVariant( (Int64) 1000 ), true, CVariant( (Int64) 5999 ), true ) ).SetBatchRowCount( 1024 );
                if ( 0 == async )
                    scan->Start( query );
                else
                    scan->StartAsync( query.SetMaxParallelism( 3 ), threadPool );
                const CDataTableBatchSourcePtr source( scan );
                CDataTableRowCursor cursor( source );
                Int64 expected = 1000;
                bool inOrder = true;
                TDataTableStatus status = DATATABLE_STATUS_OK;
                while ( DATATABLE_STATUS_OK == ( status = cursor.Next() ) )
                {
                    CVariant id;
                    inOrder = inOrder && cursor.GetValue( 0, id ) && expected++ == id.AsInt64();
                }
                ASSERT_TRUE( DATATABLE_STATUS_END == status && inOrder && 6000 == expected && 5000 == cursor.GetRowsVisited() );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: the projection is delivered in its order with the filter's columns dropped" )
        try
        {
            CDataTableBufferPool pool;
            CORETEST::CTestFileSet files;
            ASSERT_TRUE( files.Make( MakeIdAllTypesSchema(), pool, 5, 2, 2, 700, 10 ) );
            CORETEST::STestSourceCounters counters;
            CDataTableScanPtr scan = files.MakeScan( pool, counters );
            CDataTableQuery query;
            query.Select( "s" ).Select( "id" ).Select( "s" ).Where( CDataTablePredicate( "i32", DATATABLE_PREDICATE_GREATER_OR_EQUAL, CVariant( (Int64) 0 ) ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == scan->Start( query ) );
            CDataTableBatchPtr batch;
            UInt32 rows = 0;
            bool shapeOk = true;
            while ( DATATABLE_STATUS_OK == scan->NextBatch( batch ) )
            {
                shapeOk = shapeOk && 3 == batch->GetColumnCount() && 17 == batch->GetSchemaColumnIndex( 0 ) && 0 == batch->GetSchemaColumnIndex( 1 ) &&
                          17 == batch->GetSchemaColumnIndex( 2 ) && DATATABLE_TYPE_UTF8 == batch->GetColumn( 2 ).GetLogicalType();
                rows += batch->GetSelectedRowCount();
            }
            ASSERT_TRUE( shapeOk && rows > 0 && rows < 2800 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: ordered delivery with parallel decoding over many row groups and one to five credits" )
        try
        {
            CDataTableBufferPool pool;
            CORETEST::CTestFileSet files;
            CDataTableSchema schema;
            schema.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, false ) );
            schema.AddColumn( CDataTableColumnDef( "v", DATATABLE_TYPE_FLOAT64 ) );
            ASSERT_TRUE( files.Make( CreateDataTableSchemaPtr( schema ), pool, 3, 6, 10, 1500, 5 ) );
            static const UInt32 credits[] = { 1, 2, 3, 5 };
            for ( UInt32 c=0; c<4; ++c )
            {
                for ( UInt32 delay=0; delay<2; ++delay )
                {
                    CORETEST::STestSourceCounters counters;
                    CValueList options;
                    options.Set( "decodeDelayMs", CVariant( (Int64) delay ) );
                    CDataTableScanPtr scan = files.MakeScan( pool, counters, options );
                    CFutureResult future = scan->StartAsync( CDataTableQuery().Select( "id" ).SetBatchRowCount( 1024 ).SetMaxParallelism( 6 ).SetMaxBatchesInFlight( credits[ c ] ), threadPool );
                    CString::StringVector rows;
                    const TDataTableStatus status = DrainAsync( *scan, rows );
                    if ( DATATABLE_STATUS_END != status )
                        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "Ordered scan with " << credits[ c ] << " credits ended with " << DataTableStatusToString( status ) << ": " << scan->GetLastErrorMessage() );
                    ASSERT_TRUE( DATATABLE_STATUS_END == status );
                    ASSERT_TRUE( IsConsecutive( IdsOf( rows ), 0, 6 * 10 * 1500 ) );
                    ASSERT_TRUE( counters.maxConcurrentReads.Load() <= credits[ c ] );
                    ASSERT_TRUE( TTaskStatus::TASKSTATUS_FINISHED == WaitForFuture( future, 5000 ) );
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: credits bound the batches decoded ahead of a consumer that does not pull" )
        try
        {
            CDataTableBufferPool pool;
            CORETEST::CTestFileSet files;
            CDataTableSchema schema;
            schema.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, false ) );
            ASSERT_TRUE( files.Make( CreateDataTableSchemaPtr( schema ), pool, 9, 4, 6, 2048, 0 ) );
            for ( UInt32 ordered=0; ordered<2; ++ordered )
            {
                CORETEST::STestSourceCounters counters;
                CDataTableScanPtr scan = files.MakeScan( pool, counters );
                scan->StartAsync( CDataTableQuery().SetBatchRowCount( 1024 ).SetMaxParallelism( 4 ).SetMaxBatchesInFlight( 3 ).SetOrderedDelivery( 1 == ordered ), threadPool );
                MT::PrecisionDelay( 300 );
                const UInt32 decodedAhead = counters.batchesRead.Load();
                ASSERT_TRUE( decodedAhead > 0 && decodedAhead <= 3 );
                CString::StringVector rows;
                ASSERT_TRUE( DATATABLE_STATUS_END == DrainAsync( *scan, rows ) );
                ASSERT_TRUE( 4 * 6 * 2048 == rows.size() );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: a limit ends the scan after that many selected rows and cuts the last batch" )
        try
        {
            CDataTableBufferPool pool;
            CORETEST::CTestFileSet files;
            CDataTableSchema schema;
            schema.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, false ) );
            schema.AddColumn( CDataTableColumnDef( "s", DATATABLE_TYPE_UTF8 ) );
            ASSERT_TRUE( files.Make( CreateDataTableSchemaPtr( schema ), pool, 21, 3, 5, 1000, 20 ) );
            for ( UInt32 async=0; async<2; ++async )
            {
                CORETEST::STestSourceCounters counters;
                CDataTableScanPtr scan = files.MakeScan( pool, counters );
                CDataTableQuery query;
                query.SetLimit( 2500 ).SetBatchRowCount( 1024 ).Where( CDataTablePredicate( "id", DATATABLE_PREDICATE_GREATER_OR_EQUAL, CVariant( (Int64) 100 ) ) );
                CString::StringVector rows;
                UInt32 batches = 0;
                if ( 0 == async )
                {
                    ASSERT_TRUE( DATATABLE_STATUS_OK == scan->Start( query ) );
                    ASSERT_TRUE( DATATABLE_STATUS_END == CORETEST::CollectScanRows( *scan, rows, &batches ) );
                }
                else
                {
                    scan->StartAsync( query.SetMaxParallelism( 3 ), threadPool );
                    ASSERT_TRUE( DATATABLE_STATUS_END == DrainAsync( *scan, rows, &batches ) );
                }
                ASSERT_TRUE( IsConsecutive( IdsOf( rows ), 100, 2500 ) );
                CDataTableScanStats stats;
                scan->GetStats( stats );
                ASSERT_TRUE( 2500 == stats.rowsSelected && batches == stats.batchesDelivered );
                CDataTableBatchPtr batch;
                ASSERT_TRUE( DATATABLE_STATUS_END == scan->NextBatch( batch ) );
            }

            CORETEST::STestSourceCounters counters;
            CDataTableScanPtr none = files.MakeScan( pool, counters );
            ASSERT_TRUE( DATATABLE_STATUS_OK == none->Start( CDataTableQuery().SetLimit( 0 ) ) );
            CDataTableBatchPtr batch;
            ASSERT_TRUE( DATATABLE_STATUS_END == none->NextBatch( batch ) );
            CDataTableScanStats stats;
            none->GetStats( stats );
            ASSERT_TRUE( 0 == stats.filesListed );

            CDataTableScanPtr nothing = files.MakeScan( pool, counters );
            ASSERT_TRUE( DATATABLE_STATUS_OK == nothing->Start( CDataTableQuery().WhereEquals( "id", CVariant( (Float64) 0.5 ) ) ) );
            ASSERT_TRUE( DATATABLE_STATUS_END == nothing->NextBatch( batch ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: cancellation before the start, while planning, while decoding, through the future and after the end" )
        try
        {
            CDataTableBufferPool pool;
            CORETEST::CTestFileSet files;
            CDataTableSchema schema;
            schema.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, false ) );
            ASSERT_TRUE( files.Make( CreateDataTableSchemaPtr( schema ), pool, 31, 4, 4, 2048, 0 ) );
            CDataTableBatchPtr batch;

            // before the start: nothing happens, the scan runs
            {
                CORETEST::STestSourceCounters counters;
                CDataTableScanPtr scan = files.MakeScan( pool, counters );
                scan->Cancel();
                ASSERT_TRUE( DATATABLE_STATUS_OK == scan->Start( CDataTableQuery() ) );
                ASSERT_TRUE( DATATABLE_STATUS_OK == scan->NextBatch( batch ) );
            }

            // a sync scan between batches
            {
                CORETEST::STestSourceCounters counters;
                CDataTableScanPtr scan = files.MakeScan( pool, counters );
                scan->Start( CDataTableQuery() );
                ASSERT_TRUE( DATATABLE_STATUS_OK == scan->NextBatch( batch ) );
                scan->Cancel();
                ASSERT_TRUE( DATATABLE_STATUS_CANCELLED == scan->NextBatch( batch ) );
                scan.Unlink();
                ASSERT_TRUE( WaitForReadersGone( counters, 1000 ) );
            }

            // while the files are listed
            {
                CORETEST::STestSourceCounters counters;
                CValueList options;
                options.Set( "nextFileDelayMs", CVariant( (Int64) 300 ) );
                CDataTableScanPtr scan = files.MakeScan( pool, counters, options );
                CFutureResult future = scan->StartAsync( CDataTableQuery(), threadPool );
                MT::PrecisionDelay( 30 );
                const UInt64 start = MT::PrecisionTickCount();
                scan->Cancel();
                ASSERT_TRUE( DATATABLE_STATUS_CANCELLED == scan->NextBatch( batch ) );
                ASSERT_TRUE( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) < 100 );
                ASSERT_TRUE( TTaskStatus::TASKSTATUS_CANCELLED == WaitForFuture( future, 1000 ) );
            }

            // while decoding: running tasks stop at their next batch
            {
                CORETEST::STestSourceCounters counters;
                CValueList options;
                options.Set( "decodeDelayMs", CVariant( (Int64) 20 ) );
                CDataTableScanPtr scan = files.MakeScan( pool, counters, options );
                CFutureResult future = scan->StartAsync( CDataTableQuery().SetMaxParallelism( 4 ), threadPool );
                ASSERT_TRUE( DATATABLE_STATUS_OK == scan->WaitForNextBatch( batch, 5000 ) );
                const UInt64 start = MT::PrecisionTickCount();
                scan->Cancel();
                ASSERT_TRUE( DATATABLE_STATUS_CANCELLED == scan->NextBatch( batch ) );
                ASSERT_TRUE( TTaskStatus::TASKSTATUS_CANCELLED == WaitForFuture( future, 1000 ) );
                scan.Unlink();
                ASSERT_TRUE( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) < 1000 );
                ASSERT_TRUE( WaitForReadersGone( counters, 2000 ) );
                ASSERT_TRUE( counters.batchesRead.Load() < 4 * 4 * 2 );
            }

            // through the future
            {
                CORETEST::STestSourceCounters counters;
                CValueList options;
                options.Set( "decodeDelayMs", CVariant( (Int64) 10 ) );
                CDataTableScanPtr scan = files.MakeScan( pool, counters, options );
                CFutureResult future = scan->StartAsync( CDataTableQuery().SetMaxParallelism( 2 ), threadPool );
                future.RequestCancellation();
                CString::StringVector rows;
                ASSERT_TRUE( DATATABLE_STATUS_CANCELLED == DrainAsync( *scan, rows ) );
                ASSERT_TRUE( TTaskStatus::TASKSTATUS_CANCELLED == WaitForFuture( future, 1000 ) );
            }

            // after the end it changes nothing
            {
                CORETEST::STestSourceCounters counters;
                CDataTableScanPtr scan = files.MakeScan( pool, counters );
                CFutureResult future = scan->StartAsync( CDataTableQuery(), threadPool );
                CString::StringVector rows;
                ASSERT_TRUE( DATATABLE_STATUS_END == DrainAsync( *scan, rows ) );
                scan->Cancel();
                ASSERT_TRUE( DATATABLE_STATUS_END == scan->NextBatch( batch ) );
                ASSERT_TRUE( TTaskStatus::TASKSTATUS_FINISHED == WaitForFuture( future, 1000 ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: errors of readers and sources end the scan with their status and message, sync and async" )
        try
        {
            CDataTableBufferPool pool;
            CORETEST::CTestFileSet files;
            CDataTableSchema schema;
            schema.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, false ) );
            ASSERT_TRUE( files.Make( CreateDataTableSchemaPtr( schema ), pool, 41, 3, 4, 1000, 0 ) );

            struct SFault { const char* option; Int64 value; TDataTableStatus status; };
            const SFault faults[] =
            {
                { "failOpenRowGroup", 2, DATATABLE_STATUS_CORRUPT_DATA },
                { "failReadBatch", 1, DATATABLE_STATUS_CORRUPT_DATA },
                { "failNextFile", 1, DATATABLE_STATUS_IO_ERROR },
                { "failOpenFile", 2, DATATABLE_STATUS_IO_ERROR }
            };
            for ( UInt32 f=0; f<4; ++f )
            {
                for ( UInt32 async=0; async<2; ++async )
                {
                    CORETEST::STestSourceCounters counters;
                    CValueList options;
                    options.Set( faults[ f ].option, CVariant( faults[ f ].value ) );
                    CDataTableScanPtr scan = files.MakeScan( pool, counters, options );
                    CString::StringVector rows;
                    TDataTableStatus status = DATATABLE_STATUS_OK;
                    if ( 0 == async )
                    {
                        scan->Start( CDataTableQuery() );
                        status = CORETEST::CollectScanRows( *scan, rows );
                    }
                    else
                    {
                        CFutureResult future = scan->StartAsync( CDataTableQuery().SetMaxParallelism( 3 ), threadPool );
                        status = DrainAsync( *scan, rows );
                        ASSERT_TRUE( TTaskStatus::TASKSTATUS_STARTUP_FAILED == WaitForFuture( future, 2000 ) );
                    }
                    if ( faults[ f ].status != status )
                        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "Fault " << faults[ f ].option << " async " << async << " gave " << DataTableStatusToString( status ) );
                    ASSERT_TRUE( faults[ f ].status == status );
                    ASSERT_FALSE( scan->GetLastErrorMessage().IsNULLOrEmpty() );
                    if ( 0 == f )
                        ASSERT_TRUE( scan->GetLastErrorMessage().HasSubstr( "injected" ) >= 0 );
                    // the sync scan delivered what came before the fault, in order
                    if ( 0 == async && 0 == f )
                        ASSERT_TRUE( IsConsecutive( IdsOf( rows ), 0, 2000 ) );
                    CDataTableBatchPtr batch;
                    ASSERT_TRUE( faults[ f ].status == scan->NextBatch( batch ) );
                    scan.Unlink();
                    ASSERT_TRUE( WaitForReadersGone( counters, 2000 ) );
                }
            }

            // a file that does not fit the table schema
            CDataTableSchema other;
            other.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_UTF8, false ) );
            CORETEST::STestSourceCounters counters;
            CDataTableScanSourcePtr source( GUCEF_NEW CORETEST::CTestFileListSource( files.files, CValueList(), counters ) );
            CDataTableScan mismatched( source, CreateDataTableSchemaPtr( other ), pool );
            mismatched.Start( CDataTableQuery() );
            CDataTableBatchPtr batch;
            ASSERT_TRUE( DATATABLE_STATUS_SCHEMA_MISMATCH == mismatched.NextBatch( batch ) );

            // a query that does not fit refuses the start
            CDataTableScanPtr badQuery = files.MakeScan( pool, counters );
            ASSERT_TRUE( DATATABLE_STATUS_NOT_FOUND == badQuery->Start( CDataTableQuery().Select( "nope" ) ) );
            ASSERT_TRUE( DATATABLE_STATUS_NOT_FOUND == badQuery->NextBatch( batch ) );
            CDataTableScanPtr badAsync = files.MakeScan( pool, counters );
            CFutureResult future = badAsync->StartAsync( CDataTableQuery().WhereEquals( "id", CVariant( "x" ) ), threadPool );
            ASSERT_TRUE( future.IsReady() );
            ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == badAsync->NextBatch( batch ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: async scans open several small files at once and deliver them in listing order; the first failing file in listing order ends the scan" )
        try
        {
            CDataTableBufferPool pool;
            CORETEST::CTestFileSet files;
            CDataTableSchema schema;
            schema.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, false ) );
            const UInt32 fileCount = 48;
            const UInt32 rowsPerFile = 100;
            ASSERT_TRUE( files.Make( CreateDataTableSchemaPtr( schema ), pool, 53, fileCount, 1, rowsPerFile, 0 ) );

            // every even file is opened while planning (odd ones come with their metadata): 4 ms each
            for ( UInt32 parallelism=1; parallelism<=ScanTestThreads; parallelism+=ScanTestThreads-1 )
            {
                CORETEST::STestSourceCounters counters;
                CValueList options;
                options.Set( "openFileDelayMs", CVariant( (Int64) 4 ) );
                CDataTableScanPtr scan = files.MakeScan( pool, counters, options );
                CFutureResult future = scan->StartAsync( CDataTableQuery().SetMaxParallelism( parallelism ), threadPool );
                CString::StringVector rows;
                ASSERT_TRUE( DATATABLE_STATUS_END == DrainAsync( *scan, rows ) );
                ASSERT_TRUE( TTaskStatus::TASKSTATUS_FINISHED == WaitForFuture( future, 2000 ) );
                ASSERT_TRUE( IsConsecutive( IdsOf( rows ), 0, (UInt64) fileCount * rowsPerFile ) );
                CDataTableScanStats stats;
                scan->GetStats( stats );
                ASSERT_TRUE( fileCount == stats.filesListed && fileCount == stats.filesOpened && fileCount / 2 == stats.footerCacheHits );
                if ( 1 == parallelism )
                    ASSERT_TRUE( 1 == counters.maxConcurrentOpens.Load() );
                else
                    ASSERT_TRUE( counters.maxConcurrentOpens.Load() >= 2 && counters.maxConcurrentOpens.Load() <= parallelism );
                scan.Unlink();
                ASSERT_TRUE( WaitForReadersGone( counters, 2000 ) );
            }

            // file 6 fails at once while the files before it still open: the scan ends with its error, never one of a
            // later file, and what was delivered is the start of the rows in order
            for ( UInt32 round=0; round<10; ++round )
            {
                CORETEST::STestSourceCounters counters;
                CValueList options;
                options.Set( "openFileDelayMs", CVariant( (Int64) 3 ) );
                options.Set( "failOpenFile", CVariant( (Int64) 6 ) );
                CDataTableScanPtr scan = files.MakeScan( pool, counters, options );
                CFutureResult future = scan->StartAsync( CDataTableQuery().SetMaxParallelism( ScanTestThreads ), threadPool );
                CString::StringVector rows;
                ASSERT_TRUE( DATATABLE_STATUS_IO_ERROR == DrainAsync( *scan, rows ) );
                ASSERT_TRUE( TTaskStatus::TASKSTATUS_STARTUP_FAILED == WaitForFuture( future, 2000 ) );
                ASSERT_TRUE( scan->GetLastErrorMessage().HasSubstr( "\"file6\"" ) >= 0 );
                ASSERT_TRUE( rows.size() <= 6 * rowsPerFile && IsConsecutive( IdsOf( rows ), 0, rows.size() ) );
                scan.Unlink();
                ASSERT_TRUE( WaitForReadersGone( counters, 2000 ) );
            }

            // cancelled and destroyed while files are being opened: the tasks hand them back and release their readers
            for ( UInt32 round=0; round<5; ++round )
            {
                CORETEST::STestSourceCounters counters;
                CValueList options;
                options.Set( "openFileDelayMs", CVariant( (Int64) 20 ) );
                CDataTableScanPtr scan = files.MakeScan( pool, counters, options );
                CFutureResult future = scan->StartAsync( CDataTableQuery().SetMaxParallelism( ScanTestThreads ), threadPool );
                MT::PrecisionDelay( 5 * round );
                if ( 0 == round % 2 )
                {
                    scan->Cancel();
                    CDataTableBatchPtr batch;
                    ASSERT_TRUE( DATATABLE_STATUS_CANCELLED == scan->NextBatch( batch ) );
                    ASSERT_TRUE( TTaskStatus::TASKSTATUS_CANCELLED == WaitForFuture( future, 2000 ) );
                }
                scan.Unlink();
                ASSERT_TRUE( WaitForReadersGone( counters, 2000 ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: files lacking nullable columns read as nulls, narrower types are widened, a lower decimal scale rescaled" )
        try
        {
            CDataTableBufferPool pool;
            CDataTableSchema fileSchema;
            fileSchema.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, false ) );
            fileSchema.AddColumn( CDataTableColumnDef( "v", DATATABLE_TYPE_INT32 ) );
            fileSchema.AddColumn( CDataTableColumnDef( "f", DATATABLE_TYPE_FLOAT32 ) );
            fileSchema.AddColumn( CDataTableColumnDef::Decimal( "dec", 9, 1 ) );
            CORETEST::CTestFileSet files;
            ASSERT_TRUE( files.Make( CreateDataTableSchemaPtr( fileSchema ), pool, 51, 2, 3, 500, 25 ) );

            CDataTableSchema tableSchema;
            tableSchema.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, false ) );
            tableSchema.AddColumn( CDataTableColumnDef( "extra", DATATABLE_TYPE_UTF8 ) );
            tableSchema.AddColumn( CDataTableColumnDef( "v", DATATABLE_TYPE_INT64 ) );
            tableSchema.AddColumn( CDataTableColumnDef( "f", DATATABLE_TYPE_FLOAT64 ) );
            tableSchema.AddColumn( CDataTableColumnDef::Decimal( "dec", 10, 2 ) );
            CDataTableSchemaPtr table = CreateDataTableSchemaPtr( tableSchema );

            // the same rows read with the file schema, for the expected values
            CORETEST::STestSourceCounters counters;
            CDataTableScanPtr plain = files.MakeScan( pool, counters );
            plain->Start( CDataTableQuery() );
            GUCEF::vector< CDataTableBatchPtr > fileBatches;
            CDataTableBatchPtr batch;
            while ( DATATABLE_STATUS_OK == plain->NextBatch( batch ) )
                fileBatches.push_back( batch );

            CDataTableScanSourcePtr source( GUCEF_NEW CORETEST::CTestFileListSource( files.files, CValueList(), counters ) );
            CDataTableScan widened( source, table, pool );
            ASSERT_TRUE( DATATABLE_STATUS_OK == widened.Start( CDataTableQuery() ) );
            UInt32 index = 0;
            bool valuesOk = true;
            while ( DATATABLE_STATUS_OK == widened.NextBatch( batch ) && index < fileBatches.size() )
            {
                const CDataTableBatch& expected = *fileBatches[ index++ ];
                CString error;
                valuesOk = valuesOk && batch->GetRowCount() == expected.GetRowCount() && 5 == batch->GetColumnCount();
                valuesOk = valuesOk && batch->GetColumn( 1 ).Validate( error ) && batch->GetColumn( 2 ).Validate( error ) && batch->GetColumn( 4 ).Validate( error );
                valuesOk = valuesOk && GUCEF_NULL != batch->GetColumn( 2 ).GetInt64Data() && GUCEF_NULL != batch->GetColumn( 3 ).GetFloat64Data();
                for ( UInt32 r=0; r<batch->GetRowCount() && valuesOk; ++r )
                {
                    CVariant a, b;
                    batch->GetValue( r, 1, a );
                    valuesOk = a.IsNULL();
                    batch->GetValue( r, 2, a );
                    expected.GetValue( r, 1, b );
                    valuesOk = valuesOk && a.IsNULL() == b.IsNULL() && a.AsInt64() == b.AsInt64();
                    batch->GetValue( r, 3, a );
                    expected.GetValue( r, 2, b );
                    valuesOk = valuesOk && a.IsNULL() == b.IsNULL() && ( a.IsNULL() || a.AsFloat64() == b.AsFloat64() || a.AsFloat64() != a.AsFloat64() );
                    batch->GetValue( r, 4, a );
                    expected.GetValue( r, 3, b );
                    CDecimal da, db;
                    a.GetDecimal( da );
                    b.GetDecimal( db );
                    valuesOk = valuesOk && a.IsNULL() == b.IsNULL() && ( a.IsNULL() || ( 0 == da.Compare( db ) && 2 == da.GetScale() ) );
                }
            }
            ASSERT_TRUE( valuesOk && index == fileBatches.size() );

            // a predicate on the missing column prunes every row group, IS_NULL on it keeps every row
            CDataTableScanSourcePtr source2( GUCEF_NEW CORETEST::CTestFileListSource( files.files, CValueList(), counters ) );
            CDataTableScan pruned( source2, table, pool );
            pruned.Start( CDataTableQuery().WhereEquals( "extra", CVariant( "x" ) ) );
            ASSERT_TRUE( DATATABLE_STATUS_END == pruned.NextBatch( batch ) );
            CDataTableScanStats stats;
            pruned.GetStats( stats );
            ASSERT_TRUE( 6 == stats.rowGroupsPrunedByStatistics );
            CDataTableScanSourcePtr source3( GUCEF_NEW CORETEST::CTestFileListSource( files.files, CValueList(), counters ) );
            CDataTableScan nulls( source3, table, pool );
            nulls.Start( CDataTableQuery().WhereNull( "extra", true ).Where( CDataTablePredicate( "dec", DATATABLE_PREDICATE_GREATER, CDataTableValues::Decimal( CDecimal::Parse( "-1000000" ) ) ) ) );
            CString::StringVector rows;
            ASSERT_TRUE( DATATABLE_STATUS_END == CORETEST::CollectScanRows( nulls, rows ) );
            ASSERT_TRUE( rows.size() > 1000 && rows.size() < 3000 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: a scan destroyed while its tasks run cancels them; batches held stay valid" )
        try
        {
            CDataTableBufferPool pool;
            CORETEST::CTestFileSet files;
            CDataTableSchema schema;
            schema.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, false ) );
            schema.AddColumn( CDataTableColumnDef( "s", DATATABLE_TYPE_UTF8 ) );
            ASSERT_TRUE( files.Make( CreateDataTableSchemaPtr( schema ), pool, 61, 4, 6, 1500, 10 ) );
            for ( UInt32 round=0; round<5; ++round )
            {
                CORETEST::STestSourceCounters counters;
                CValueList options;
                options.Set( "decodeDelayMs", CVariant( (Int64) ( round % 3 ) ) );
                CDataTableBatchPtr held;
                {
                    CDataTableScanPtr scan = files.MakeScan( pool, counters, options );
                    scan->StartAsync( CDataTableQuery().SetMaxParallelism( 5 ).SetMaxBatchesInFlight( 4 ), threadPool );
                    if ( 0 != round )
                        ASSERT_TRUE( DATATABLE_STATUS_OK == scan->WaitForNextBatch( held, 5000 ) );
                }
                ASSERT_TRUE( WaitForReadersGone( counters, 5000 ) );
                if ( !held.IsNULL() )
                {
                    CString error;
                    ASSERT_TRUE( held->GetColumn( 0 ).Validate( error ) && held->GetColumn( 1 ).Validate( error ) );
                    CVariant id;
                    ASSERT_TRUE( held->GetValue( 0, 0, id ) && 0 == id.AsInt64() );
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: batches of a memory table stay valid after the scan and the table are gone" )
        try
        {
            CDataTableBufferPool pool;
            GUCEF::vector< CDataTableBatchPtr > held;
            {
                CDataTableDefinition definition;
                definition.name = "held";
                definition.schema = *MakeIdAllTypesSchema();
                definition.storage.rowGroupRowCount = 3000;
                definition.storage.pageRowCount = 3000;
                CMemoryDataTable table( definition, &pool );
                CORETEST::CTestRandom random( 71 );
                CDataTableBatchPtr rows = CORETEST::MakeRandomBatch( table.GetSchema(), pool, random, 10000, 0, 30 );
                CDataTableWriteResult result;
                ASSERT_TRUE( DATATABLE_STATUS_OK == table.Write( *rows, DATATABLE_WRITE_INSERT, result ) );
                rows.Reset();
                CDataTableScanPtr scan;
                ASSERT_TRUE( DATATABLE_STATUS_OK == table.Scan( CDataTableQuery().SetBatchRowCount( 2000 ), scan ) );
                CDataTableBatchPtr batch;
                while ( !scan.IsNULL() && DATATABLE_STATUS_OK == scan->NextBatch( batch ) )
                    held.push_back( batch );
            }
            ASSERT_TRUE( 7 == held.size() );
            UInt64 total = 0;
            bool valid = true;
            for ( UInt32 b=0; b<(UInt32) held.size(); ++b )
            {
                for ( UInt32 c=0; c<held[ b ]->GetColumnCount(); ++c )
                {
                    CString error;
                    valid = valid && held[ b ]->GetColumn( c ).Validate( error );
                }
                CVariant id;
                held[ b ]->GetValue( 0, 0, id );
                valid = valid && (Int64) total == id.AsInt64();
                total += held[ b ]->GetRowCount();
            }
            ASSERT_TRUE( valid && 10000 == total );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: the batches available event is coalesced; finished and failed events carry stats and the error" )
        try
        {
            CDataTableBufferPool pool;
            CORETEST::CTestFileSet files;
            CDataTableSchema schema;
            schema.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, false ) );
            ASSERT_TRUE( files.Make( CreateDataTableSchemaPtr( schema ), pool, 81, 3, 4, 2048, 0 ) );
            PulseGeneratorPtr pulseGenerator = CPulseGenerator::CreateSharedObj();

            CORETEST::STestSourceCounters counters;
            CDataTableScanPtr scan = files.MakeScan( pool, counters, CValueList(), pulseGenerator );
            CScanEventCounter events( *scan );
            scan->StartAsync( CDataTableQuery().SetBatchRowCount( 1024 ).SetMaxParallelism( 3 ), threadPool );
            UInt32 batches = 0;
            TDataTableStatus status = DATATABLE_STATUS_PENDING;
            const UInt64 start = MT::PrecisionTickCount();
            while ( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) < ScanTestTimeoutMs )
            {
                pulseGenerator->ForceDirectPulse();
                CDataTableBatchPtr batch;
                status = scan->TryNextBatch( batch );
                if ( DATATABLE_STATUS_OK == status )
                {
                    ++batches;
                    continue;
                }
                if ( DATATABLE_STATUS_PENDING != status )
                    break;
                MT::PrecisionDelay( 1 );
            }
            // events raised by tasks as they hand their last batches over may arrive after the end
            for ( UInt32 i=0; i<20 || ( 0 == events.finished && i<400 ); ++i )
            {
                pulseGenerator->ForceDirectPulse();
                MT::PrecisionDelay( 5 );
            }
            ASSERT_TRUE( DATATABLE_STATUS_END == status && 3 * 4 * 2 == batches );
            if ( events.available < 1 || events.available > batches )
                GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "Batches available events: " << events.available << " for " << batches << " batches" );
            ASSERT_TRUE( events.available >= 1 && events.available <= batches );
            ASSERT_TRUE( 1 == events.finished && 3 * 4 * 2048 == events.finishedRows && 0 == events.failed );

            CValueList options;
            options.Set( "failOpenRowGroup", CVariant( (Int64) 1 ) );
            CDataTableScanPtr failing = files.MakeScan( pool, counters, options, pulseGenerator );
            CScanEventCounter failEvents( *failing );
            failing->Start( CDataTableQuery() );
            CString::StringVector rows;
            ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == CORETEST::CollectScanRows( *failing, rows ) );
            for ( UInt32 i=0; i<20 && 0 == failEvents.failed; ++i )
            {
                pulseGenerator->ForceDirectPulse();
                MT::PrecisionDelay( 1 );
            }
            ASSERT_TRUE( 1 == failEvents.failed && failEvents.error.HasSubstr( "injected" ) >= 0 && 0 == failEvents.finished );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: scan events nobody pumps are released with the scan: no heap is retained, finished and failed, with and without a subscriber" )
        try
        {
            CDataTableBufferPool pool;
            CORETEST::CTestFileSet files;
            CDataTableSchema schema;
            schema.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, false ) );
            ASSERT_TRUE( files.Make( CreateDataTableSchemaPtr( schema ), pool, 81, 1, 2, 64, 0 ) );

            // nobody pumps this pulse generator: the events of the scans stay in the mailbox of their notifier until it goes
            PulseGeneratorPtr unpumped = CPulseGenerator::CreateSharedObj();
            CORETEST::STestSourceCounters counters;
            CValueList failOptions;
            failOptions.Set( "failOpenRowGroup", CVariant( (Int64) 1 ) );

            // where the heap cannot be measured (sanitizer builds) the rounds still run: the leak check of the sanitizer sees what is retained
            const bool measurable = HeapBytesInUse() >= 0;
            const UInt32 warmUpRounds = 100;
            const UInt32 measuredRounds = measurable ? 1500 : 100;
            Int64 afterWarmUp = 0;
            for ( UInt32 round=0; round<warmUpRounds+measuredRounds; ++round )
            {
                if ( warmUpRounds == round )
                    afterWarmUp = HeapBytesInUse();
                const bool subscribed = 0 != ( round & 1 );
                CString::StringVector rows;

                // finished: the event carries the statistics of the scan
                CDataTableScanPtr scan = files.MakeScan( pool, counters, CValueList(), unpumped );
                CScanEventCounter* events = subscribed ? new CScanEventCounter( *scan ) : GUCEF_NULL;
                scan->Start( CDataTableQuery() );
                ASSERT_TRUE( DATATABLE_STATUS_END == CORETEST::CollectScanRows( *scan, rows ) );
                delete events;
                scan.Unlink();

                // failed: the event carries the error text
                CDataTableScanPtr failing = files.MakeScan( pool, counters, failOptions, unpumped );
                CScanEventCounter* failEvents = subscribed ? new CScanEventCounter( *failing ) : GUCEF_NULL;
                failing->Start( CDataTableQuery() );
                ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == CORETEST::CollectScanRows( *failing, rows ) );
                delete failEvents;
                failing.Unlink();

                // destroyed before the end: the events raised so far are dropped with the scan
                CDataTableScanPtr abandoned = files.MakeScan( pool, counters, CValueList(), unpumped );
                CScanEventCounter* abandonedEvents = subscribed ? new CScanEventCounter( *abandoned ) : GUCEF_NULL;
                abandoned->Start( CDataTableQuery().SetBatchRowCount( 16 ) );
                CDataTableBatchPtr batch;
                abandoned->NextBatch( batch );
                batch.Reset();
                delete abandonedEvents;
                abandoned.Unlink();
            }
            const Int64 growth = measurable ? HeapBytesInUse() - afterWarmUp : 0;
            if ( growth > 256 * 1024 )
                GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "Heap use grew by " << growth << " bytes over " << measuredRounds << " rounds of unpumped scans" );
            ASSERT_TRUE( growth <= 256 * 1024 );

            // the same on pool threads: the events come from the tasks of the scan; no pump, no subscriber is needed for them to be raised
            for ( UInt32 round=0; round<50; ++round )
            {
                CString::StringVector rows;
                CDataTableScanPtr async = files.MakeScan( pool, counters, CValueList(), unpumped );
                CScanEventCounter* asyncEvents = 0 != ( round & 1 ) ? new CScanEventCounter( *async ) : GUCEF_NULL;
                async->StartAsync( CDataTableQuery().SetMaxParallelism( 2 ), threadPool );
                ASSERT_TRUE( DATATABLE_STATUS_END == DrainAsync( *async, rows ) );
                delete asyncEvents;
                async.Unlink();
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: a warm sync scan delivers its batches without a heap allocation and without pool misses" )
        GUCEF_TESTFW_SKIP_IF( !CORETEST::IsAllocationCountingAvailable(), CORETEST::GetAllocationCountingUnavailableReason() );
        try
        {
            CDataTableBufferPool pool;
            CDataTableDefinition definition;
            definition.name = "steady";
            definition.schema.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, false ) );
            definition.schema.AddColumn( CDataTableColumnDef( "close", DATATABLE_TYPE_FLOAT64 ) );
            definition.schema.AddColumn( CDataTableColumnDef( "symbol", DATATABLE_TYPE_UTF8 ) );
            definition.storage.rowGroupRowCount = 65536;
            CMemoryDataTable table( definition, &pool );
            CORETEST::CTestRandom random( 91 );
            CDataTableBatchPtr rows = CORETEST::MakeRandomBatch( table.GetSchema(), pool, random, 200000, 0, 5 );
            CDataTableWriteResult result;
            ASSERT_TRUE( DATATABLE_STATUS_OK == table.Write( *rows, DATATABLE_WRITE_INSERT, result ) );
            rows.Reset();

            // counted from the first batch to the last: the end of the scan raises ScanFinishedEvent, which is
            // mailed with a copy of its data to the thread of the pulse generator, once per scan
            UInt32 allocations = 0;
            UInt64 misses = 1;
            UInt32 batchCount = 0;
            for ( UInt32 round=0; round<3; ++round )
            {
                CDataTableScanPtr scan;
                table.Scan( CDataTableQuery().Where( CDataTablePredicate( "close", DATATABLE_PREDICATE_GREATER, CVariant( (Float64) 0.0 ) ) ), scan );
                CDataTableBatchPtr batch;
                ASSERT_TRUE( DATATABLE_STATUS_OK == scan->NextBatch( batch ) );
                if ( 2 == round )
                    CORETEST::StartAllocationCounting();
                Float64 sum = 0.0;
                UInt32 batches = 1;
                TDataTableStatus status = DATATABLE_STATUS_OK;
                while ( DATATABLE_STATUS_OK == status )
                {
                    const Float64* close = batch->GetColumn( 1 ).GetFloat64Data();
                    const UInt32* selection = batch->GetSelection();
                    for ( UInt32 i=0; i<batch->GetSelectedRowCount(); ++i )
                        sum += close[ GUCEF_NULL == selection ? i : selection[ i ] ];
                    if ( 2 == round && batches == batchCount )
                    {
                        allocations = CORETEST::StopAllocationCounting();
                        CDataTableScanStats stats;
                        scan->GetStats( stats );
                        misses = stats.poolMisses;
                    }
                    status = scan->NextBatch( batch );
                    if ( DATATABLE_STATUS_OK == status )
                        ++batches;
                }
                if ( 0 == round )
                    batchCount = batches;
                ASSERT_TRUE( DATATABLE_STATUS_END == status && sum > 0.0 && batches == batchCount && batchCount > 4 );
            }
            if ( allocations > 0 )
                GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "A warm scan allocated " << allocations << " times:\n" << CORETEST::GetCountedAllocationStacks() );
            ASSERT_TRUE( 0 == allocations );
            ASSERT_TRUE( 0 == misses );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
