/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACTBUILDER_H
#include "gucefCOMCORE_CRpcContractBuilder.h"
#define GUCEF_COMCORE_CRPCCONTRACTBUILDER_H
#endif /* GUCEF_COMCORE_CRPCCONTRACTBUILDER_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACT_H
#include "gucefCOMCORE_CRpcContract.h"
#define GUCEF_COMCORE_CRPCCONTRACT_H
#endif /* GUCEF_COMCORE_CRPCCONTRACT_H ? */

#include "gucefDRNUTILS_CTaskManagerContracts.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Reserved numbers and names of an enum or a type: what the frozen 1.0 contract keeps for the deferred items, through
 *  the reserved entries of CRpcContractBuilder (comma separated lists)
 */
static void
AddReserved( COMCORE::CRpcContractBuilder& builder, const char* listName, const char* itemName, const char* numbers, const char* names )
{GUCEF_TRACE;

    const bool isEnum = 0 == strcmp( listName, "enums" );
    CORE::CString::StringVector numberList = CORE::CString( numbers ).ParseElements( ',', false );
    for ( UInt32 n=0; n<numberList.size(); ++n )
    {
        if ( isEnum )
            builder.AddReservedEnumNumber( itemName, CORE::StringToInt32( numberList[ n ] ) );
        else
            builder.AddReservedFieldNumber( itemName, CORE::StringToUInt32( numberList[ n ] ) );
    }
    CORE::CString::StringVector nameList = CORE::CString( names ).ParseElements( ',', false );
    for ( UInt32 n=0; n<nameList.size(); ++n )
    {
        if ( isEnum )
            builder.AddReservedEnumName( itemName, nameList[ n ] );
        else
            builder.AddReservedFieldName( itemName, nameList[ n ] );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* CTaskManagerContracts::ContractName = "gucef.drnutils.tasks";
const char* CTaskManagerContracts::ServiceName = "TaskManager";

/*-------------------------------------------------------------------------*/

COMCORE::CRpcContractPtr
CTaskManagerContracts::GetContract( void )
{GUCEF_TRACE;

    CORE::CDataNode source;
    GetContractSource( source );
    CORE::CString error;
    COMCORE::CRpcContractPtr contract = COMCORE::CRpcContract::LoadFromDataNode( source, error );
    if ( contract.IsNULL() )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CTaskManagerContracts: building the contract " << ContractName << " failed: " << error );
    return contract;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerContracts::GetContractSource( CORE::CDataNode& result )
{GUCEF_TRACE;

    COMCORE::CRpcContractBuilder builder( "gucef.drnutils.tasks", 1, 0 );
    builder.SetDoc( "View and control of the task manager of a node: thread pools, threads, tasks and the scheduled jobs of its job schedulers. Task data is never exposed. All times are UTC milliseconds" );
    builder.AddEnum( "TaskControlAction" );
    builder.AddEnumValue( "TaskControlAction", 0, "TASK_CONTROL_ACTION_UNSPECIFIED" );
    builder.AddEnumValue( "TaskControlAction", 1, "RESIZE_POOL" );
    builder.AddEnumValue( "TaskControlAction", 2, "CANCEL_TASK" );
    builder.AddEnumValue( "TaskControlAction", 3, "STOP_TASK" );
    builder.AddEnum( "ExecutionTarget" );
    builder.AddEnumValue( "ExecutionTarget", 0, "EXECUTION_TARGET_UNSPECIFIED" );
    builder.AddEnumValue( "ExecutionTarget", 1, "POOL" );
    builder.AddEnumValue( "ExecutionTarget", 2, "MAIN_THREAD" );
    builder.AddType( "Empty" );
    builder.AddType( "TaskManagerInfo" );
    builder.AddField( "TaskManagerInfo", 1, "hostName", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "TaskManagerInfo", 2, "hostInstanceId", "uint64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "TaskManagerInfo", 3, "serverTimeUtc", "timestamp", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "TaskManagerInfo", 4, "capabilities", "string", COMCORE::RPC_CARDINALITY_REPEATED );
    builder.AddField( "TaskManagerInfo", 5, "controllablePools", "string", COMCORE::RPC_CARDINALITY_REPEATED );
    builder.AddField( "TaskManagerInfo", 6, "tableNames", "map<string,string>", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddType( "PoolRow" );
    builder.AddField( "PoolRow", 1, "name", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "PoolRow", 2, "activeWorkers", "uint32", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "PoolRow", 3, "desiredMinWorkers", "uint32", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "PoolRow", 4, "desiredMaxThreads", "int32", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "PoolRow", 5, "dedicatedThreads", "uint32", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "PoolRow", 6, "queued", "uint32", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "PoolRow", 7, "running", "uint32", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "PoolRow", 8, "startedPerSecond", "float64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "PoolRow", 9, "finishedPerSecond", "float64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "PoolRow", 10, "failedPerSecond", "float64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "PoolRow", 11, "cancelledPerSecond", "float64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "PoolRow", 12, "avgQueueWaitMs", "float64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "PoolRow", 13, "avgRunMs", "float64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "PoolRow", 14, "utilisationPercent", "float32", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "PoolRow", 15, "startedTotal", "uint64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "PoolRow", 16, "finishedTotal", "uint64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "PoolRow", 17, "failedTotal", "uint64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "PoolRow", 21, "scheduledJobs", "uint32", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "PoolRow", 22, "cancelledTotal", "uint64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddType( "PoolList" );
    builder.AddField( "PoolList", 1, "pools", "PoolRow", COMCORE::RPC_CARDINALITY_REPEATED );
    builder.AddField( "PoolList", 2, "sampleUtc", "timestamp", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddType( "ListThreadsRequest" );
    builder.AddField( "ListThreadsRequest", 1, "pool", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddType( "ThreadRow" );
    builder.AddField( "ThreadRow", 1, "systemThreadId", "uint64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ThreadRow", 2, "name", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ThreadRow", 3, "pool", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ThreadRow", 4, "status", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ThreadRow", 5, "taskId", "uint32", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ThreadRow", 6, "taskType", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ThreadRow", 7, "busySinceUtc", "timestamp", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ThreadRow", 8, "external", "bool", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddType( "ThreadList" );
    builder.AddField( "ThreadList", 1, "threads", "ThreadRow", COMCORE::RPC_CARDINALITY_REPEATED );
    builder.AddType( "ListTasksRequest" );
    builder.AddField( "ListTasksRequest", 1, "pool", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ListTasksRequest", 2, "includeQueued", "bool", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ListTasksRequest", 3, "includeEnded", "bool", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ListTasksRequest", 4, "maxRows", "uint32", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddType( "TaskRow" );
    builder.AddField( "TaskRow", 1, "taskId", "uint32", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "TaskRow", 2, "pool", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "TaskRow", 3, "taskType", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "TaskRow", 4, "status", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "TaskRow", 5, "extraInfo", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "TaskRow", 6, "queuedUtc", "timestamp", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "TaskRow", 7, "startedUtc", "timestamp", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "TaskRow", 8, "runningForMs", "uint64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "TaskRow", 9, "systemThreadId", "uint64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "TaskRow", 17, "schedulerName", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "TaskRow", 18, "jobId", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "TaskRow", 19, "runId", "uint64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddType( "TaskList" );
    builder.AddField( "TaskList", 1, "tasks", "TaskRow", COMCORE::RPC_CARDINALITY_REPEATED );
    builder.AddField( "TaskList", 2, "truncated", "bool", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddType( "ScheduledJobRow" );
    builder.AddField( "ScheduledJobRow", 1, "jobKey", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ScheduledJobRow", 2, "schedulerName", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ScheduledJobRow", 3, "jobId", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ScheduledJobRow", 4, "title", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ScheduledJobRow", 5, "taskType", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ScheduledJobRow", 6, "executionTarget", "ExecutionTarget", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ScheduledJobRow", 7, "pool", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ScheduledJobRow", 8, "lane", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ScheduledJobRow", 9, "state", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ScheduledJobRow", 10, "nextDueUtc", "timestamp", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ScheduledJobRow", 11, "activeRuns", "uint32", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ScheduledJobRow", 12, "lastRunState", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ScheduledJobRow", 13, "lastRunEndedUtc", "timestamp", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ScheduledJobRow", 14, "origin", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddType( "ScheduledJobList" );
    builder.AddField( "ScheduledJobList", 1, "jobs", "ScheduledJobRow", COMCORE::RPC_CARDINALITY_REPEATED );
    builder.AddType( "WatchRequest" );
    builder.AddField( "WatchRequest", 1, "sampleIntervalMs", "uint32", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "WatchRequest", 2, "includeThreads", "bool", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "WatchRequest", 3, "includeTasks", "bool", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "WatchRequest", 4, "includeScheduledJobs", "bool", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddType( "Sample" );
    builder.AddField( "Sample", 1, "sampleUtc", "timestamp", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "Sample", 2, "pools", "PoolRow", COMCORE::RPC_CARDINALITY_REPEATED );
    builder.AddField( "Sample", 3, "threads", "ThreadRow", COMCORE::RPC_CARDINALITY_REPEATED );
    builder.AddField( "Sample", 4, "tasks", "TaskRow", COMCORE::RPC_CARDINALITY_REPEATED );
    builder.AddField( "Sample", 5, "truncated", "bool", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "Sample", 6, "scheduledJobs", "ScheduledJobRow", COMCORE::RPC_CARDINALITY_REPEATED );
    builder.AddType( "ControlRequest" );
    builder.AddField( "ControlRequest", 1, "commandId", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ControlRequest", 2, "action", "TaskControlAction", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ControlRequest", 3, "pool", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ControlRequest", 4, "taskId", "uint32", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ControlRequest", 5, "minWorkers", "uint32", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ControlRequest", 6, "maxThreads", "int32", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ControlRequest", 7, "reason", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ControlRequest", 8, "dryRun", "bool", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddType( "ControlOutcome" );
    builder.AddField( "ControlOutcome", 1, "replayed", "bool", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ControlOutcome", 2, "messages", "string", COMCORE::RPC_CARDINALITY_REPEATED );
    builder.AddService( "TaskManager", 1 );
    builder.AddMethod( "TaskManager", 1, "GetInfo", COMCORE::RPC_METHODKIND_UNARY, "Empty", "TaskManagerInfo", GUCEF_RPC_METHODFLAG_IDEMPOTENT, 10000 );
    builder.AddMethod( "TaskManager", 2, "ListPools", COMCORE::RPC_METHODKIND_UNARY, "Empty", "PoolList", GUCEF_RPC_METHODFLAG_IDEMPOTENT, 10000 );
    builder.AddMethod( "TaskManager", 3, "ListThreads", COMCORE::RPC_METHODKIND_UNARY, "ListThreadsRequest", "ThreadList", GUCEF_RPC_METHODFLAG_IDEMPOTENT, 10000 );
    builder.AddMethod( "TaskManager", 4, "ListTasks", COMCORE::RPC_METHODKIND_UNARY, "ListTasksRequest", "TaskList", GUCEF_RPC_METHODFLAG_IDEMPOTENT, 10000 );
    builder.AddMethod( "TaskManager", 5, "Watch", COMCORE::RPC_METHODKIND_SERVER_STREAM, "WatchRequest", "Sample", 0, 0 );
    builder.SetMethodInitialCredit( "TaskManager", "Watch", 16 );
    builder.AddMethod( "TaskManager", 6, "Control", COMCORE::RPC_METHODKIND_UNARY, "ControlRequest", "ControlOutcome", 0, 30000 );
    builder.AddMethod( "TaskManager", 7, "ListScheduledJobs", COMCORE::RPC_METHODKIND_UNARY, "Empty", "ScheduledJobList", GUCEF_RPC_METHODFLAG_IDEMPOTENT, 10000 );
    AddReserved( builder, "enums", "TaskControlAction", "4,5,6,7", "PAUSE_TASK,RESUME_TASK,PAUSE_POOL,RESUME_POOL" );
    AddReserved( builder, "types", "PoolRow", "18,19,20", "dispatchPaused,parkedTasks,parkOverdueTasks" );
    AddReserved( builder, "types", "ThreadRow", "9", "parkedInPlace" );
    AddReserved( builder, "types", "TaskRow", "10,11,12,13,14,15,16", "parkState,parkSupport,parkRequestedUtc,parkedSinceUtc,parkOverdue,parkReason,threadReleased" );
    AddReserved( builder, "types", "ControlRequest", "9", "parkRunning" );
    AddReserved( builder, "types", "ControlOutcome", "3", "parkState" );
    result = builder.GetDataNode();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
