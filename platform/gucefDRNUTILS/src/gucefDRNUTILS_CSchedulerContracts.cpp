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

#include "gucefDRNUTILS_CSchedulerContracts.h"

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

const char* CSchedulerContracts::ContractName = "gucef.drnutils.scheduler";
const char* CSchedulerContracts::ServiceName = "Scheduler";

/*-------------------------------------------------------------------------*/

COMCORE::CRpcContractPtr
CSchedulerContracts::GetContract( void )
{GUCEF_TRACE;

    CORE::CDataNode source;
    GetContractSource( source );
    CORE::CString error;
    COMCORE::CRpcContractPtr contract = COMCORE::CRpcContract::LoadFromDataNode( source, error );
    if ( contract.IsNULL() )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CSchedulerContracts: building the contract " << ContractName << " failed: " << error );
    return contract;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerContracts::GetContractSource( CORE::CDataNode& result )
{GUCEF_TRACE;

    COMCORE::CRpcContractBuilder builder( "gucef.drnutils.scheduler", 1, 0 );
    builder.SetDoc( "View and control of one job scheduler of a node, and submission of jobs as data for the task types the node allows. All times are UTC milliseconds" );
    builder.AddEnum( "JobState" );
    builder.AddEnumValue( "JobState", 0, "JOB_STATE_UNSPECIFIED" );
    builder.AddEnumValue( "JobState", 1, "SCHEDULED" );
    builder.AddEnumValue( "JobState", 2, "QUEUED" );
    builder.AddEnumValue( "JobState", 3, "WAITING" );
    builder.AddEnumValue( "JobState", 4, "RUNNING" );
    builder.AddEnumValue( "JobState", 6, "DISABLED" );
    builder.AddEnumValue( "JobState", 7, "UNAVAILABLE" );
    builder.AddEnumValue( "JobState", 8, "ENDED" );
    builder.AddEnum( "RunState" );
    builder.AddEnumValue( "RunState", 0, "RUN_STATE_UNSPECIFIED" );
    builder.AddEnumValue( "RunState", 1, "RUN_QUEUED" );
    builder.AddEnumValue( "RunState", 2, "RUN_WAITING" );
    builder.AddEnumValue( "RunState", 3, "RUN_RUNNING" );
    builder.AddEnumValue( "RunState", 4, "SUCCEEDED" );
    builder.AddEnumValue( "RunState", 5, "FAILED" );
    builder.AddEnumValue( "RunState", 6, "CANCELLED" );
    builder.AddEnumValue( "RunState", 7, "TIMED_OUT" );
    builder.AddEnumValue( "RunState", 8, "SKIPPED" );
    builder.AddEnum( "ControlAction" );
    builder.AddEnumValue( "ControlAction", 0, "CONTROL_ACTION_UNSPECIFIED" );
    builder.AddEnumValue( "ControlAction", 3, "RUN_NOW" );
    builder.AddEnumValue( "ControlAction", 4, "CANCEL_RUN" );
    builder.AddEnumValue( "ControlAction", 5, "ENABLE" );
    builder.AddEnumValue( "ControlAction", 6, "DISABLE" );
    builder.AddEnumValue( "ControlAction", 7, "SET_PRIORITY" );
    builder.AddEnumValue( "ControlAction", 8, "SET_LANE_MODE" );
    builder.AddEnumValue( "ControlAction", 9, "CUSTOM" );
    builder.AddEnumValue( "ControlAction", 10, "REMOVE_JOB" );
    builder.AddEnum( "TargetKind" );
    builder.AddEnumValue( "TargetKind", 0, "TARGET_KIND_UNSPECIFIED" );
    builder.AddEnumValue( "TargetKind", 1, "JOB" );
    builder.AddEnumValue( "TargetKind", 2, "LANE" );
    builder.AddEnumValue( "TargetKind", 3, "RUN" );
    builder.AddEnumValue( "TargetKind", 4, "HOST" );
    builder.AddEnum( "ChangeKind" );
    builder.AddEnumValue( "ChangeKind", 0, "CHANGE_KIND_UNSPECIFIED" );
    builder.AddEnumValue( "ChangeKind", 1, "JOB_CHANGED" );
    builder.AddEnumValue( "ChangeKind", 2, "LANE_CHANGED" );
    builder.AddEnumValue( "ChangeKind", 3, "RUN_CHANGED" );
    builder.AddEnumValue( "ChangeKind", 4, "RESET" );
    builder.AddEnumValue( "ChangeKind", 5, "SNAPSHOT_END" );
    builder.AddEnumValue( "ChangeKind", 6, "HOST_CHANGED" );
    builder.AddEnum( "ExecutionTarget" );
    builder.AddEnumValue( "ExecutionTarget", 0, "EXECUTION_TARGET_UNSPECIFIED" );
    builder.AddEnumValue( "ExecutionTarget", 1, "POOL" );
    builder.AddEnumValue( "ExecutionTarget", 2, "MAIN_THREAD" );
    builder.AddEnum( "TriggerKind" );
    builder.AddEnumValue( "TriggerKind", 0, "TRIGGER_KIND_UNSPECIFIED" );
    builder.AddEnumValue( "TriggerKind", 1, "SCHEDULE" );
    builder.AddEnumValue( "TriggerKind", 2, "INTERVAL" );
    builder.AddEnumValue( "TriggerKind", 3, "ONCE_AT" );
    builder.AddEnumValue( "TriggerKind", 4, "MANUAL" );
    builder.AddEnumValue( "TriggerKind", 5, "WORK_SOURCE" );
    builder.AddEnum( "IntervalFrom" );
    builder.AddEnumValue( "IntervalFrom", 0, "INTERVAL_FROM_UNSPECIFIED" );
    builder.AddEnumValue( "IntervalFrom", 1, "FROM_START" );
    builder.AddEnumValue( "IntervalFrom", 2, "FROM_END" );
    builder.AddEnum( "JobOrigin" );
    builder.AddEnumValue( "JobOrigin", 0, "JOB_ORIGIN_UNSPECIFIED" );
    builder.AddEnumValue( "JobOrigin", 1, "CODE" );
    builder.AddEnumValue( "JobOrigin", 2, "SUBMITTED" );
    builder.AddType( "Empty" );
    builder.AddType( "ActionInfo" );
    builder.AddField( "ActionInfo", 1, "action", "ControlAction", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ActionInfo", 2, "targetKind", "TargetKind", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ActionInfo", 3, "customName", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ActionInfo", 4, "title", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ActionInfo", 5, "argumentNames", "string", COMCORE::RPC_CARDINALITY_REPEATED );
    builder.AddType( "HostInfo" );
    builder.AddField( "HostInfo", 1, "hostName", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "HostInfo", 2, "hostInstanceId", "uint64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "HostInfo", 3, "revision", "uint64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "HostInfo", 4, "serverTimeUtc", "timestamp", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "HostInfo", 5, "state", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "HostInfo", 6, "allowedActions", "ActionInfo", COMCORE::RPC_CARDINALITY_REPEATED );
    builder.AddField( "HostInfo", 7, "tableNames", "map<string,string>", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "HostInfo", 8, "submittableTaskTypes", "string", COMCORE::RPC_CARDINALITY_REPEATED );
    builder.AddField( "HostInfo", 9, "maxSubmitPayloadBytes", "uint32", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddType( "JobRow" );
    builder.AddField( "JobRow", 1, "jobId", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "JobRow", 2, "title", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "JobRow", 3, "lane", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "JobRow", 4, "state", "JobState", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "JobRow", 5, "priority", "uint32", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "JobRow", 6, "trigger", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "JobRow", 7, "nextDueUtc", "timestamp", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "JobRow", 8, "waitingFor", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "JobRow", 9, "waitingUntilUtc", "timestamp", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "JobRow", 12, "lastRunId", "uint64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "JobRow", 13, "lastRunState", "RunState", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "JobRow", 14, "lastRunEndedUtc", "timestamp", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "JobRow", 15, "runCount", "uint64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "JobRow", 16, "failureCount", "uint64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "JobRow", 17, "consecutiveFailures", "uint32", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "JobRow", 18, "revision", "uint64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "JobRow", 19, "allowedActions", "ActionInfo", COMCORE::RPC_CARDINALITY_REPEATED );
    builder.AddField( "JobRow", 20, "taskType", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "JobRow", 21, "executionTarget", "ExecutionTarget", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "JobRow", 22, "threadPool", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "JobRow", 23, "origin", "JobOrigin", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "JobRow", 24, "submittedBy", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "JobRow", 25, "unavailableReason", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "JobRow", 26, "activeRuns", "uint32", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddType( "LaneRow" );
    builder.AddField( "LaneRow", 1, "lane", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "LaneRow", 2, "mode", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "LaneRow", 3, "state", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "LaneRow", 4, "inFlight", "uint32", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "LaneRow", 5, "maxConcurrent", "uint32", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "LaneRow", 6, "queuedByPriority", "uint32", COMCORE::RPC_CARDINALITY_REPEATED );
    builder.AddField( "LaneRow", 7, "waitingFor", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "LaneRow", 8, "waitingUntilUtc", "timestamp", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "LaneRow", 10, "unavailableReason", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "LaneRow", 11, "heldBy", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "LaneRow", 12, "throttle", "datanode", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "LaneRow", 13, "revision", "uint64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddType( "RunRow" );
    builder.AddField( "RunRow", 1, "runId", "uint64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "RunRow", 2, "jobId", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "RunRow", 3, "itemLabel", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "RunRow", 4, "cause", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "RunRow", 5, "state", "RunState", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "RunRow", 6, "scheduledUtc", "timestamp", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "RunRow", 7, "startedUtc", "timestamp", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "RunRow", 8, "endedUtc", "timestamp", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "RunRow", 9, "attempt", "uint32", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "RunRow", 10, "progressDone", "uint64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "RunRow", 11, "progressTotal", "uint64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "RunRow", 12, "note", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "RunRow", 13, "message", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "RunRow", 14, "requestedBy", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "RunRow", 15, "taskId", "uint32", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "RunRow", 16, "revision", "uint64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "RunRow", 19, "taskType", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "RunRow", 20, "executionTarget", "ExecutionTarget", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "RunRow", 21, "threadPool", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddType( "ListRequest" );
    builder.AddField( "ListRequest", 1, "jobId", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ListRequest", 2, "lane", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ListRequest", 3, "maxRows", "uint32", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ListRequest", 4, "pageToken", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ListRequest", 5, "includeEnded", "bool", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddType( "JobList" );
    builder.AddField( "JobList", 1, "jobs", "JobRow", COMCORE::RPC_CARDINALITY_REPEATED );
    builder.AddField( "JobList", 2, "nextPageToken", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "JobList", 3, "revision", "uint64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddType( "LaneList" );
    builder.AddField( "LaneList", 1, "lanes", "LaneRow", COMCORE::RPC_CARDINALITY_REPEATED );
    builder.AddField( "LaneList", 2, "revision", "uint64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddType( "RunList" );
    builder.AddField( "RunList", 1, "runs", "RunRow", COMCORE::RPC_CARDINALITY_REPEATED );
    builder.AddField( "RunList", 2, "nextPageToken", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "RunList", 3, "revision", "uint64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddType( "GetJobRequest" );
    builder.AddField( "GetJobRequest", 1, "jobId", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "GetJobRequest", 2, "recentRuns", "uint32", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "GetJobRequest", 3, "includeSourceStatus", "bool", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddType( "JobDetail" );
    builder.AddField( "JobDetail", 1, "job", "JobRow", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "JobDetail", 2, "recentRuns", "RunRow", COMCORE::RPC_CARDINALITY_REPEATED );
    builder.AddField( "JobDetail", 3, "sourceStatus", "datanode", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "JobDetail", 4, "description", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "JobDetail", 5, "payload", "datanode", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddType( "WatchRequest" );
    builder.AddField( "WatchRequest", 1, "hostInstanceId", "uint64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "WatchRequest", 2, "afterRevision", "uint64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "WatchRequest", 3, "includeRuns", "bool", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "WatchRequest", 4, "maxMessagesPerSecond", "uint32", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddType( "Change" );
    builder.AddField( "Change", 1, "kind", "ChangeKind", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "Change", 2, "revision", "uint64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "Change", 3, "job", "JobRow", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "Change", 4, "lane", "LaneRow", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "Change", 5, "run", "RunRow", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "Change", 6, "hostInstanceId", "uint64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "Change", 7, "removedKey", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddType( "ControlRequest" );
    builder.AddField( "ControlRequest", 1, "commandId", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ControlRequest", 2, "action", "ControlAction", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ControlRequest", 3, "targetKind", "TargetKind", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ControlRequest", 4, "targetId", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ControlRequest", 5, "customName", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ControlRequest", 6, "arguments", "map<string,string>", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ControlRequest", 8, "priority", "uint32", COMCORE::RPC_CARDINALITY_OPTIONAL );
    builder.AddField( "ControlRequest", 9, "mode", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ControlRequest", 10, "reason", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ControlRequest", 11, "expectedRevision", "uint64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ControlRequest", 12, "dryRun", "bool", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ControlRequest", 13, "waitForRun", "bool", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddType( "ControlOutcome" );
    builder.AddField( "ControlOutcome", 1, "replayed", "bool", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ControlOutcome", 2, "revision", "uint64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ControlOutcome", 3, "runId", "uint64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ControlOutcome", 4, "messages", "string", COMCORE::RPC_CARDINALITY_REPEATED );
    builder.AddField( "ControlOutcome", 5, "run", "RunRow", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "ControlOutcome", 6, "detail", "datanode", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddType( "TriggerSpec" );
    builder.AddField( "TriggerSpec", 1, "kind", "TriggerKind", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "TriggerSpec", 2, "cron", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "TriggerSpec", 3, "rangeStartUtc", "timestamp", COMCORE::RPC_CARDINALITY_OPTIONAL );
    builder.AddField( "TriggerSpec", 4, "rangeEndUtc", "timestamp", COMCORE::RPC_CARDINALITY_OPTIONAL );
    builder.AddField( "TriggerSpec", 5, "periodMs", "uint64", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "TriggerSpec", 6, "intervalFrom", "IntervalFrom", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "TriggerSpec", 7, "alignToUtc", "timestamp", COMCORE::RPC_CARDINALITY_OPTIONAL );
    builder.AddField( "TriggerSpec", 8, "onceAtUtc", "timestamp", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddType( "SubmitJobRequest" );
    builder.AddField( "SubmitJobRequest", 1, "commandId", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "SubmitJobRequest", 2, "jobId", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "SubmitJobRequest", 3, "title", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "SubmitJobRequest", 4, "taskType", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "SubmitJobRequest", 5, "payload", "datanode", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "SubmitJobRequest", 6, "trigger", "TriggerSpec", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "SubmitJobRequest", 7, "executionTarget", "ExecutionTarget", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "SubmitJobRequest", 8, "threadPool", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "SubmitJobRequest", 9, "lane", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "SubmitJobRequest", 10, "priority", "uint32", COMCORE::RPC_CARDINALITY_OPTIONAL );
    builder.AddField( "SubmitJobRequest", 11, "replaceExisting", "bool", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "SubmitJobRequest", 12, "dryRun", "bool", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "SubmitJobRequest", 13, "reason", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddField( "SubmitJobRequest", 14, "description", "string", COMCORE::RPC_CARDINALITY_SINGLE );
    builder.AddService( "Scheduler", 1 );
    builder.AddMethod( "Scheduler", 1, "GetInfo", COMCORE::RPC_METHODKIND_UNARY, "Empty", "HostInfo", GUCEF_RPC_METHODFLAG_IDEMPOTENT, 10000 );
    builder.AddMethod( "Scheduler", 2, "ListJobs", COMCORE::RPC_METHODKIND_UNARY, "ListRequest", "JobList", GUCEF_RPC_METHODFLAG_IDEMPOTENT, 10000 );
    builder.AddMethod( "Scheduler", 3, "GetJob", COMCORE::RPC_METHODKIND_UNARY, "GetJobRequest", "JobDetail", GUCEF_RPC_METHODFLAG_IDEMPOTENT, 10000 );
    builder.AddMethod( "Scheduler", 4, "ListLanes", COMCORE::RPC_METHODKIND_UNARY, "ListRequest", "LaneList", GUCEF_RPC_METHODFLAG_IDEMPOTENT, 10000 );
    builder.AddMethod( "Scheduler", 5, "ListRuns", COMCORE::RPC_METHODKIND_UNARY, "ListRequest", "RunList", GUCEF_RPC_METHODFLAG_IDEMPOTENT, 10000 );
    builder.AddMethod( "Scheduler", 6, "Watch", COMCORE::RPC_METHODKIND_SERVER_STREAM, "WatchRequest", "Change", 0, 0 );
    builder.SetMethodInitialCredit( "Scheduler", "Watch", 64 );
    builder.AddMethod( "Scheduler", 7, "Control", COMCORE::RPC_METHODKIND_UNARY, "ControlRequest", "ControlOutcome", GUCEF_RPC_METHODFLAG_MAYANSWERASYNC, 30000 );
    builder.AddMethod( "Scheduler", 8, "SubmitJob", COMCORE::RPC_METHODKIND_UNARY, "SubmitJobRequest", "ControlOutcome", GUCEF_RPC_METHODFLAG_SENSITIVE, 30000 );
    AddReserved( builder, "enums", "JobState", "5", "PAUSED" );
    AddReserved( builder, "enums", "RunState", "9,10,11", "RUN_PARKING,RUN_PARKED,RUN_RESUMING" );
    AddReserved( builder, "enums", "ControlAction", "1,2", "PAUSE,RESUME" );
    AddReserved( builder, "types", "JobRow", "10,11", "pausedUntilUtc,pausedBy" );
    AddReserved( builder, "types", "LaneRow", "9", "pausedUntilUtc" );
    AddReserved( builder, "types", "RunRow", "17,18", "parkedSinceUtc,parkReason" );
    AddReserved( builder, "types", "ControlRequest", "7,14", "pauseForSeconds,parkRunning" );
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
