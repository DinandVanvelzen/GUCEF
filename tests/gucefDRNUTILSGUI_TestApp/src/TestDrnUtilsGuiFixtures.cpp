/*
 *  gucefDRNUTILSGUI_TestApp: GUCEF test application for the gucefDRNUTILSGUI module
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
//      INCLUDES                                                    //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#include "gucefCORE_CDataTableBatchBuilder.h"
#define GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#endif /* GUCEF_CORE_CDATATABLEBATCHBUILDER_H ? */

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#ifndef GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#include "gucefCORE_CDataTableBufferPool.h"
#define GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#endif /* GUCEF_CORE_CDATATABLEBUFFERPOOL_H ? */

#ifndef GUCEF_CORE_CDATATABLEQUERY_H
#include "gucefCORE_CDataTableQuery.h"
#define GUCEF_CORE_CDATATABLEQUERY_H
#endif /* GUCEF_CORE_CDATATABLEQUERY_H ? */

#include "TestDrnUtilsGuiFixtures.h"

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                   //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

CORE::CDataTableBufferPool&
TestBufferPool( void )
{
    static CORE::CDataTableBufferPool pool;
    return pool;
}

void
GetSchemaOf( ETestTableKind kind, CORE::CDataTableSchema& schema, CORE::CString& name )
{
    switch ( kind )
    {
        case TESTTABLE_JOBS: DRNUTILS::CJobsTableSchema::GetSchema( schema ); name = DRNUTILS::CJobsTableSchema::TableName; break;
        case TESTTABLE_LANES: DRNUTILS::CLanesTableSchema::GetSchema( schema ); name = DRNUTILS::CLanesTableSchema::TableName; break;
        case TESTTABLE_RUNS: DRNUTILS::CRunsTableSchema::GetSchema( schema ); name = DRNUTILS::CRunsTableSchema::TableName; break;
        case TESTTABLE_POOLS: DRNUTILS::CPoolsTableSchema::GetSchema( schema ); name = DRNUTILS::CPoolsTableSchema::TableName; break;
        case TESTTABLE_THREADS: DRNUTILS::CThreadsTableSchema::GetSchema( schema ); name = DRNUTILS::CThreadsTableSchema::TableName; break;
        case TESTTABLE_TASKS: DRNUTILS::CTasksTableSchema::GetSchema( schema ); name = DRNUTILS::CTasksTableSchema::TableName; break;
        default: DRNUTILS::CScheduledJobsTableSchema::GetSchema( schema ); name = DRNUTILS::CScheduledJobsTableSchema::TableName; break;
    }
}

/** The cell of the row for the column of the schema, in the type of the column; null when the row has nothing for it */
CORE::CVariant
CellOf( const CTestRow& row, const CORE::CDataTableColumnDef& column )
{
    CORE::CVariant cell;
    const CORE::CString name = column.name;
    const GUCEF::map< CORE::CString, CORE::CString >::const_iterator t = row.text.find( name );
    if ( t != row.text.end() )
        return CORE::ToUtf8String( t->second );
    const GUCEF::map< CORE::CString, Int64 >::const_iterator n = row.number.find( name );
    if ( n == row.number.end() )
        return cell;
    switch ( column.type )
    {
        case CORE::DATATABLE_TYPE_TIMESTAMP: return CORE::CDataTableValues::Timestamp( n->second, CORE::DATATABLE_TIMEUNIT_MILLISECONDS );
        case CORE::DATATABLE_TYPE_UINT32: return CORE::CVariant( (UInt32) n->second );
        case CORE::DATATABLE_TYPE_UINT64: return CORE::CVariant( (UInt64) n->second );
        case CORE::DATATABLE_TYPE_INT32: return CORE::CVariant( (Int32) n->second );
        case CORE::DATATABLE_TYPE_INT64: return CORE::CVariant( (Int64) n->second );
        case CORE::DATATABLE_TYPE_FLOAT32: return CORE::CVariant( (Float32) n->second );
        case CORE::DATATABLE_TYPE_FLOAT64: return CORE::CVariant( (Float64) n->second );
        case CORE::DATATABLE_TYPE_BOOL: return CORE::CVariant( 0 != n->second );
        default: return cell;
    }
}

}

/*-------------------------------------------------------------------------*/

CTestTable::CTestTable( const CORE::CDataTableDefinition& definition, CORE::CDataTableBufferPool* pool )
    : CORE::CMemoryDataTable( definition, pool )
    , m_hints()
{
}

/*-------------------------------------------------------------------------*/

void
CTestTable::RaiseRowsChanged( void )
{
    CORE::TDataTableRowsChangedEventData changed;
    NotifyObservers( CORE::CIDataTable::RowsChangedEvent, &changed );
}

/*-------------------------------------------------------------------------*/

void
CTestTable::SetHint( const CORE::CString& column, const CORE::CString& hint, const CORE::CVariant& value )
{
    m_hints[ column ][ hint ] = value;
}

/*-------------------------------------------------------------------------*/

bool
CTestTable::GetColumnHints( UInt32 columnIndex, CORE::CValueList& hints ) const
{
    if ( m_hints.empty() )
        return false;
    CORE::CDataTableSchemaPtr schema = GetSchema();
    if ( schema.IsNULL() || columnIndex >= schema->GetColumnCount() )
        return false;
    const THintMap::const_iterator column = m_hints.find( schema->GetColumn( columnIndex ).name );
    if ( column == m_hints.end() )
        return true;
    for ( GUCEF::map< CORE::CString, CORE::CVariant >::const_iterator i = column->second.begin(); i != column->second.end(); ++i )
        hints.Set( i->first, i->second );
    return true;
}

/*-------------------------------------------------------------------------*/

CTestTable*
MakeTestTable( ETestTableKind kind, CORE::CIDataTablePtr& table )
{
    CORE::CDataTableDefinition definition;
    GetSchemaOf( kind, definition.schema, definition.name );
    CTestTable* raw = GUCEF_NEW CTestTable( definition, &TestBufferPool() );
    table = CORE::CIDataTablePtr( raw );
    return raw->IsValid() ? raw : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
UpsertTestRows( CTestTable& table, const TTestRowVector& rows )
{
    CORE::CDataTableSchemaPtr schema = table.GetSchema();
    CORE::CDataTableBatchBuilder builder( schema, TestBufferPool() );
    const UInt32 columns = schema->GetColumnCount();
    for ( size_t r=0; r<rows.size(); ++r )
    {
        GUCEF::vector< CORE::CVariant > cells( columns );
        for ( UInt32 c=0; c<columns; ++c )
            cells[ c ] = CellOf( rows[ r ], schema->GetColumn( c ) );
        builder.AppendRow( &cells[ 0 ], columns );
    }
    CORE::CDataTableBatchPtr batch;
    CORE::CString error;
    CORE::CDataTableWriteResult result;
    return CORE::DATATABLE_STATUS_OK == builder.Build( batch, error ) && CORE::DATATABLE_STATUS_OK == table.Write( *batch, CORE::DATATABLE_WRITE_UPSERT, result );
}

/*-------------------------------------------------------------------------*/

bool
DeleteTestRows( CTestTable& table, const CORE::CString& column, const CORE::CString& text )
{
    CORE::CDataTableQuery query;
    query.WhereEquals( column, CORE::CVariant( text ) );
    CORE::CDataTableWriteResult result;
    return CORE::DATATABLE_STATUS_OK == table.DeleteRows( query, result );
}

/*-------------------------------------------------------------------------*/

DRNUTILS::SSchedulerHostInfo
MakeSchedulerInfo( const CORE::CString& hostName, bool allowControl )
{
    DRNUTILS::SSchedulerHostInfo info;
    info.hostName = hostName;
    info.hostInstanceId = 7;
    info.revision = 42;
    info.state = "RUNNING";
    if ( allowControl )
    {
        const CORE::TJobControlAction actions[] = { CORE::JOBCONTROLACTION_RUN_NOW, CORE::JOBCONTROLACTION_CANCEL_RUN, CORE::JOBCONTROLACTION_ENABLE, CORE::JOBCONTROLACTION_DISABLE };
        for ( UInt32 i=0; i<4; ++i )
        {
            DRNUTILS::SSchedulerActionInfo action;
            action.action = actions[ i ];
            action.targetKind = CORE::JOBCONTROLACTION_CANCEL_RUN == actions[ i ] ? CORE::JOBCONTROLTARGET_RUN : CORE::JOBCONTROLTARGET_JOB;
            action.title = CORE::JobControlActionToString( actions[ i ] );
            info.allowedActions.push_back( action );
        }
    }
    return info;
}

/*-------------------------------------------------------------------------*/

DRNUTILS::STaskManagerHostInfo
MakeTaskManagerInfo( const CORE::CString& hostName, bool allowControl )
{
    DRNUTILS::STaskManagerHostInfo info;
    info.hostName = hostName;
    info.hostInstanceId = 9;
    info.capabilities.insert( "taskTimes" );
    info.capabilities.insert( "scheduledJobs" );
    if ( allowControl )
    {
        info.capabilities.insert( "resizePool" );
        info.capabilities.insert( "cancelTask" );
        info.capabilities.insert( "stopTask" );
        info.controllablePools.insert( "workers" );
    }
    return info;
}

/*-------------------------------------------------------------------------*/

bool
STestSchedulerTables::Fill( void )
{
    jobs = MakeTestTable( TESTTABLE_JOBS, jobsPtr );
    lanes = MakeTestTable( TESTTABLE_LANES, lanesPtr );
    runs = MakeTestTable( TESTTABLE_RUNS, runsPtr );
    if ( GUCEF_NULL == jobs || GUCEF_NULL == lanes || GUCEF_NULL == runs )
        return false;

    TTestRowVector jobRows;
    jobRows.push_back( CTestRow().Text( "jobId", "job-1" ).Text( "title", "Fetch quotes" ).Text( "lane", "market" ).Text( "state", "SCHEDULED" ).Text( "executionTarget", "POOL" ).Text( "threadPool", "workers" ).Text( "taskType", "FetchTask" )
                                 .Num( "nextDueUtc", UtcMs( 2026, 3, 8, 6, 59, 59 ) ).Num( "runCount", 12 ).Num( "failureCount", 1 ).Num( "activeRuns", 0 ) );
    jobRows.push_back( CTestRow().Text( "jobId", "job-2" ).Text( "title", "Rebuild index" ).Text( "lane", "batch" ).Text( "state", "RUNNING" ).Text( "executionTarget", "MAIN_THREAD" ).Text( "taskType", "IndexTask" )
                                 .Num( "nextDueUtc", UtcMs( 2026, 3, 8, 7, 0, 0 ) ).Num( "runCount", 3 ).Num( "activeRuns", 1 ) );
    jobRows.push_back( CTestRow().Text( "jobId", "job-3" ).Text( "title", "Old report" ).Text( "lane", "batch" ).Text( "state", "DISABLED" ).Text( "executionTarget", "POOL" ).Text( "taskType", "ReportTask" )
                                 .Num( "nextDueUtc", UtcMs( 2026, 11, 1, 6, 0, 0 ) ).Num( "runCount", 0 ) );
    if ( !UpsertTestRows( *jobs, jobRows ) )
        return false;

    TTestRowVector laneRows;
    laneRows.push_back( CTestRow().Text( "lane", "market" ).Text( "mode", "NORMAL" ).Text( "state", "OPEN" ).Num( "inFlight", 0 ).Num( "maxConcurrent", 2 ) );
    if ( !UpsertTestRows( *lanes, laneRows ) )
        return false;

    TTestRowVector runRows;
    runRows.push_back( CTestRow().Num( "runId", 100 ).Text( "jobId", "job-1" ).Text( "state", "SUCCEEDED" ).Text( "cause", "SCHEDULE" ).Num( "startedUtc", UtcMs( 2026, 3, 8, 5, 0, 0 ) ).Num( "endedUtc", UtcMs( 2026, 3, 8, 5, 0, 4 ) ).Text( "taskType", "FetchTask" ) );
    runRows.push_back( CTestRow().Num( "runId", 101 ).Text( "jobId", "job-1" ).Text( "state", "FAILED" ).Text( "cause", "RUN_NOW" ).Num( "startedUtc", UtcMs( 2026, 3, 8, 6, 0, 0 ) ).Num( "endedUtc", UtcMs( 2026, 3, 8, 6, 0, 2 ) ).Text( "message", "timeout" ) );
    runRows.push_back( CTestRow().Num( "runId", 102 ).Text( "jobId", "job-1" ).Text( "state", "RUNNING" ).Text( "cause", "SCHEDULE" ).Num( "startedUtc", UtcMs( 2026, 3, 8, 6, 30, 0 ) ).Num( "progressDone", 12 ).Num( "progressTotal", 40 ) );
    runRows.push_back( CTestRow().Num( "runId", 200 ).Text( "jobId", "job-2" ).Text( "state", "RUNNING" ).Text( "cause", "SCHEDULE" ).Num( "startedUtc", UtcMs( 2026, 3, 8, 6, 40, 0 ) ) );
    return UpsertTestRows( *runs, runRows );
}

/*-------------------------------------------------------------------------*/

DRNUTILS::CSchedulerTableSet
STestSchedulerTables::GetSet( void ) const
{
    DRNUTILS::CSchedulerTableSet set;
    set.SetJobs( jobsPtr );
    set.SetLanes( lanesPtr );
    set.SetRuns( runsPtr );
    return set;
}

/*-------------------------------------------------------------------------*/

bool
STestTaskManagerTables::Fill( void )
{
    pools = MakeTestTable( TESTTABLE_POOLS, poolsPtr );
    threads = MakeTestTable( TESTTABLE_THREADS, threadsPtr );
    tasks = MakeTestTable( TESTTABLE_TASKS, tasksPtr );
    scheduledJobs = MakeTestTable( TESTTABLE_SCHEDULEDJOBS, scheduledJobsPtr );
    if ( GUCEF_NULL == pools || GUCEF_NULL == threads || GUCEF_NULL == tasks || GUCEF_NULL == scheduledJobs )
        return false;

    TTestRowVector poolRows;
    poolRows.push_back( CTestRow().Text( "name", "workers" ).Num( "activeWorkers", 4 ).Num( "desiredMinWorkers", 2 ).Num( "desiredMaxThreads", 8 ).Num( "queued", 3 ).Num( "running", 2 ) );
    poolRows.push_back( CTestRow().Text( "name", "gucef.scheduler.main" ).Num( "activeWorkers", 0 ).Num( "desiredMinWorkers", 0 ).Num( "desiredMaxThreads", 0 ).Num( "queued", 1 ).Num( "running", 0 ) );
    if ( !UpsertTestRows( *pools, poolRows ) )
        return false;

    TTestRowVector threadRows;
    threadRows.push_back( CTestRow().Num( "systemThreadId", 4711 ).Text( "name", "workers#1" ).Text( "pool", "workers" ).Text( "status", "BUSY" ).Num( "taskId", 55 ).Num( "busySinceUtc", UtcMs( 2026, 3, 8, 6, 59, 0 ) ) );
    if ( !UpsertTestRows( *threads, threadRows ) )
        return false;

    TTestRowVector taskRows;
    taskRows.push_back( CTestRow().Num( "taskId", 55 ).Text( "pool", "workers" ).Text( "taskType", "FetchTask" ).Text( "status", "RUNNING" ).Num( "queuedUtc", UtcMs( 2026, 3, 8, 6, 58, 59 ) ).Num( "startedUtc", UtcMs( 2026, 3, 8, 6, 59, 0 ) ).Text( "schedulerName", "main" ).Text( "jobId", "job-1" ) );
    taskRows.push_back( CTestRow().Num( "taskId", 56 ).Text( "pool", "workers" ).Text( "taskType", "FetchTask" ).Text( "status", "QUEUED" ) );
    if ( !UpsertTestRows( *tasks, taskRows ) )
        return false;

    TTestRowVector scheduledRows;
    scheduledRows.push_back( CTestRow().Text( "jobKey", "main/job-1" ).Text( "schedulerName", "main" ).Text( "jobId", "job-1" ).Text( "title", "Fetch quotes" ).Text( "taskType", "FetchTask" ).Text( "executionTarget", "POOL" ).Text( "pool", "workers" ).Text( "state", "SCHEDULED" ) );
    scheduledRows.push_back( CTestRow().Text( "jobKey", "main/job-2" ).Text( "schedulerName", "main" ).Text( "jobId", "job-2" ).Text( "title", "Rebuild index" ).Text( "taskType", "IndexTask" ).Text( "executionTarget", "MAIN_THREAD" ).Text( "state", "RUNNING" ) );
    return UpsertTestRows( *scheduledJobs, scheduledRows );
}

/*-------------------------------------------------------------------------*/

DRNUTILS::CTaskManagerTableSet
STestTaskManagerTables::GetSet( void ) const
{
    DRNUTILS::CTaskManagerTableSet set;
    set.SetPools( poolsPtr );
    set.SetThreads( threadsPtr );
    set.SetTasks( tasksPtr );
    set.SetScheduledJobs( scheduledJobsPtr );
    return set;
}

/*-------------------------------------------------------------------------*/
