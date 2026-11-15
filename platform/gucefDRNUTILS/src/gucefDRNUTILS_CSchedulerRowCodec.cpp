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

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#include "gucefDRNUTILS_CSchedulerRowCodec.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* CSchedulerRowCodec::RedactedText = "[redacted]";

/*-------------------------------------------------------------------------*/

static Int64
MsToMicros( Int64 ms )
{
    return ms * 1000;
}

/*-------------------------------------------------------------------------*/

static Int64
MicrosToMs( Int64 micros )
{
    return micros / 1000;
}

/*-------------------------------------------------------------------------*/

static void
SetTime( COMCORE::CRpcMessage& row, UInt32 field, Int64 utcMs )
{
    if ( 0 != utcMs )
        row.SetTimestamp( field, MsToMicros( utcMs ) );
}

/*-------------------------------------------------------------------------*/

static void
SetText( COMCORE::CRpcMessage& row, UInt32 field, const CORE::CString& text, UInt32 maxBytes )
{
    if ( !text.IsNULLOrEmpty() )
        row.SetString( field, CSchedulerRowCodec::Bound( text, maxBytes ) );
}

/*-------------------------------------------------------------------------*/

static CORE::CVariant
TextCell( const CORE::CString& text, UInt32 maxBytes )
{
    if ( text.IsNULLOrEmpty() )
        return CORE::CVariant();
    return CORE::CVariant( CSchedulerRowCodec::Bound( text, maxBytes ) );
}

/*-------------------------------------------------------------------------*/

const char*
CSchedulerRowCodec::JobOriginToString( CORE::TJobOrigin origin )
{GUCEF_TRACE;

    switch ( origin )
    {
        case CORE::JOBORIGIN_CODE: return "CODE";
        case CORE::JOBORIGIN_SUBMITTED: return "SUBMITTED";
        default: return "UNSPECIFIED";
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSchedulerRowCodec::Bound( const CORE::CString& text, UInt32 maxBytes )
{GUCEF_TRACE;

    UInt32 bytes = text.ByteSize() > 0 ? text.ByteSize()-1 : 0;
    if ( bytes <= maxBytes )
        return text;
    // every code point takes at least one byte: cutting the excess in code points always fits
    return text.CutChars( bytes - maxBytes, false );
}

/*-------------------------------------------------------------------------*/

CORE::CVariant
CSchedulerRowCodec::TimestampCell( Int64 utcMs )
{GUCEF_TRACE;

    if ( 0 == utcMs )
        return CORE::CVariant();
    return CORE::CDataTableValues::Timestamp( utcMs, CORE::DATATABLE_TIMEUNIT_MILLISECONDS );
}

/*-------------------------------------------------------------------------*/

void
CSchedulerRowCodec::JobToMessage( const CORE::SJobStatus& job, COMCORE::CRpcMessage& row )
{GUCEF_TRACE;

    SetText( row, JOBROW_JOBID, job.jobId, MaxIdBytes );
    SetText( row, JOBROW_TITLE, job.title, MaxIdBytes );
    SetText( row, JOBROW_LANE, job.laneName, MaxIdBytes );
    row.SetEnum( JOBROW_STATE, (Int32) job.state );
    row.SetUInt32( JOBROW_PRIORITY, job.priority );
    SetText( row, JOBROW_TRIGGER, job.triggerText, MaxTextBytes );
    SetTime( row, JOBROW_NEXTDUE, job.nextDueUtcMs );
    SetText( row, JOBROW_WAITINGFOR, job.waitingFor, MaxIdBytes );
    SetTime( row, JOBROW_WAITINGUNTIL, job.waitingUntilUtcMs );
    row.SetUInt64( JOBROW_LASTRUNID, job.lastRunId );
    row.SetEnum( JOBROW_LASTRUNSTATE, (Int32) job.lastRunState );
    SetTime( row, JOBROW_LASTRUNENDED, job.lastRunEndedUtcMs );
    row.SetUInt64( JOBROW_RUNCOUNT, job.runCount );
    row.SetUInt64( JOBROW_FAILURECOUNT, job.failureCount );
    row.SetUInt32( JOBROW_CONSECUTIVEFAILURES, job.consecutiveFailures );
    row.SetUInt64( JOBROW_REVISION, job.revision );
    SetText( row, JOBROW_TASKTYPE, job.taskTypeName, MaxIdBytes );
    row.SetEnum( JOBROW_EXECUTIONTARGET, (Int32) job.executionTarget );
    SetText( row, JOBROW_THREADPOOL, job.threadPoolName, MaxIdBytes );
    row.SetEnum( JOBROW_ORIGIN, (Int32) job.origin );
    SetText( row, JOBROW_SUBMITTEDBY, job.submittedBy, MaxIdBytes );
    SetText( row, JOBROW_UNAVAILABLEREASON, job.unavailableReason, MaxTextBytes );
    row.SetUInt32( JOBROW_ACTIVERUNS, job.activeRunCount );
}

/*-------------------------------------------------------------------------*/

void
CSchedulerRowCodec::LaneToMessage( const CORE::SJobLaneStatus& lane, COMCORE::CRpcMessage& row )
{GUCEF_TRACE;

    SetText( row, LANEROW_LANE, lane.laneName, MaxIdBytes );
    SetText( row, LANEROW_MODE, lane.mode, MaxIdBytes );
    SetText( row, LANEROW_STATE, lane.state, MaxIdBytes );
    row.SetUInt32( LANEROW_INFLIGHT, lane.inFlight );
    row.SetUInt32( LANEROW_MAXCONCURRENT, lane.maxConcurrent );
    for ( UInt32 p=0; p<lane.queuedByPriority.size() && p<4; ++p )
        row.Add( LANEROW_QUEUEDBYPRIORITY, CORE::CVariant( lane.queuedByPriority[ p ] ) );
    SetText( row, LANEROW_WAITINGFOR, lane.waitingFor, MaxIdBytes );
    SetTime( row, LANEROW_WAITINGUNTIL, lane.waitingUntilUtcMs );
    SetText( row, LANEROW_UNAVAILABLEREASON, lane.unavailableReason, MaxTextBytes );
    SetText( row, LANEROW_HELDBY, lane.heldBy, MaxIdBytes );
    if ( lane.throttleStatus.GetNrOfChildNodes() > 0 || lane.throttleStatus.GetAttCount() > 0 )
        row.SetDataNode( LANEROW_THROTTLE, lane.throttleStatus );
    row.SetUInt64( LANEROW_REVISION, lane.revision );
}

/*-------------------------------------------------------------------------*/

void
CSchedulerRowCodec::RunToMessage( const CORE::SJobRunRecord& run, COMCORE::CRpcMessage& row )
{GUCEF_TRACE;

    row.SetUInt64( RUNROW_RUNID, run.runId );
    SetText( row, RUNROW_JOBID, run.jobId, MaxIdBytes );
    SetText( row, RUNROW_ITEMLABEL, run.itemLabel, MaxIdBytes );
    row.SetString( RUNROW_CAUSE, CORE::JobRunCauseToString( run.cause ) );
    row.SetEnum( RUNROW_STATE, (Int32) run.state );
    SetTime( row, RUNROW_SCHEDULED, run.scheduledUtcMs );
    SetTime( row, RUNROW_STARTED, run.startedUtcMs );
    SetTime( row, RUNROW_ENDED, run.endedUtcMs );
    row.SetUInt32( RUNROW_ATTEMPT, run.attempt );
    row.SetUInt64( RUNROW_PROGRESSDONE, run.progressDone );
    row.SetUInt64( RUNROW_PROGRESSTOTAL, run.progressTotal );
    SetText( row, RUNROW_NOTE, run.note, CORE::SJobRunRecord::MaxNoteBytes );
    SetText( row, RUNROW_MESSAGE, run.outcomeMessage, CORE::SJobRunRecord::MaxOutcomeMessageBytes );
    SetText( row, RUNROW_REQUESTEDBY, run.requestedBy, MaxIdBytes );
    row.SetUInt32( RUNROW_TASKID, run.taskId );
    row.SetUInt64( RUNROW_REVISION, run.revision );
    SetText( row, RUNROW_TASKTYPE, run.taskTypeName, MaxIdBytes );
    row.SetEnum( RUNROW_EXECUTIONTARGET, (Int32) run.executionTarget );
    SetText( row, RUNROW_THREADPOOL, run.threadPoolName, MaxIdBytes );
}

/*-------------------------------------------------------------------------*/

void
CSchedulerRowCodec::ActionToMessage( const SSchedulerActionInfo& action, COMCORE::CRpcMessage& row )
{GUCEF_TRACE;

    row.SetEnum( ACTIONINFO_ACTION, (Int32) action.action );
    row.SetEnum( ACTIONINFO_TARGETKIND, (Int32) action.targetKind );
    SetText( row, ACTIONINFO_CUSTOMNAME, action.customName, MaxIdBytes );
    SetText( row, ACTIONINFO_TITLE, action.title, MaxIdBytes );
    for ( UInt32 i=0; i<action.argumentNames.size(); ++i )
        row.Add( ACTIONINFO_ARGUMENTNAMES, CORE::CVariant( action.argumentNames[ i ] ) );
}

/*-------------------------------------------------------------------------*/

void
CSchedulerRowCodec::JobFromMessage( const COMCORE::CRpcMessage& row, CORE::SJobStatus& job )
{GUCEF_TRACE;

    job.jobId = row.GetString( JOBROW_JOBID );
    job.title = row.GetString( JOBROW_TITLE );
    job.laneName = row.GetString( JOBROW_LANE );
    job.state = (CORE::TJobState) row.GetEnum( JOBROW_STATE );
    job.priority = row.GetUInt32( JOBROW_PRIORITY );
    job.triggerText = row.GetString( JOBROW_TRIGGER );
    job.nextDueUtcMs = MicrosToMs( row.GetTimestamp( JOBROW_NEXTDUE ) );
    job.waitingFor = row.GetString( JOBROW_WAITINGFOR );
    job.waitingUntilUtcMs = MicrosToMs( row.GetTimestamp( JOBROW_WAITINGUNTIL ) );
    job.lastRunId = row.GetUInt64( JOBROW_LASTRUNID );
    job.lastRunState = (CORE::TJobRunState) row.GetEnum( JOBROW_LASTRUNSTATE );
    job.lastRunEndedUtcMs = MicrosToMs( row.GetTimestamp( JOBROW_LASTRUNENDED ) );
    job.runCount = row.GetUInt64( JOBROW_RUNCOUNT );
    job.failureCount = row.GetUInt64( JOBROW_FAILURECOUNT );
    job.consecutiveFailures = row.GetUInt32( JOBROW_CONSECUTIVEFAILURES );
    job.revision = row.GetUInt64( JOBROW_REVISION );
    job.taskTypeName = row.GetString( JOBROW_TASKTYPE );
    job.executionTarget = (CORE::TJobExecutionTarget) row.GetEnum( JOBROW_EXECUTIONTARGET );
    job.threadPoolName = row.GetString( JOBROW_THREADPOOL );
    job.origin = (CORE::TJobOrigin) row.GetEnum( JOBROW_ORIGIN );
    job.submittedBy = row.GetString( JOBROW_SUBMITTEDBY );
    job.unavailableReason = row.GetString( JOBROW_UNAVAILABLEREASON );
    job.activeRunCount = row.GetUInt32( JOBROW_ACTIVERUNS );
}

/*-------------------------------------------------------------------------*/

void
CSchedulerRowCodec::LaneFromMessage( const COMCORE::CRpcMessage& row, CORE::SJobLaneStatus& lane )
{GUCEF_TRACE;

    lane.laneName = row.GetString( LANEROW_LANE );
    lane.mode = row.GetString( LANEROW_MODE );
    lane.state = row.GetString( LANEROW_STATE );
    lane.inFlight = row.GetUInt32( LANEROW_INFLIGHT );
    lane.maxConcurrent = row.GetUInt32( LANEROW_MAXCONCURRENT );
    lane.queuedByPriority.clear();
    UInt32 count = row.GetCount( LANEROW_QUEUEDBYPRIORITY );
    for ( UInt32 p=0; p<count; ++p )
    {
        CORE::CVariant value;
        row.GetAt( LANEROW_QUEUEDBYPRIORITY, p, value );
        lane.queuedByPriority.push_back( value.AsUInt32() );
    }
    lane.waitingFor = row.GetString( LANEROW_WAITINGFOR );
    lane.waitingUntilUtcMs = MicrosToMs( row.GetTimestamp( LANEROW_WAITINGUNTIL ) );
    lane.unavailableReason = row.GetString( LANEROW_UNAVAILABLEREASON );
    lane.available = lane.unavailableReason.IsNULLOrEmpty();
    lane.heldBy = row.GetString( LANEROW_HELDBY );
    row.GetDataNode( LANEROW_THROTTLE, lane.throttleStatus );
    lane.revision = row.GetUInt64( LANEROW_REVISION );
}

/*-------------------------------------------------------------------------*/

void
CSchedulerRowCodec::RunFromMessage( const COMCORE::CRpcMessage& row, CORE::SJobRunRecord& run )
{GUCEF_TRACE;

    run.runId = row.GetUInt64( RUNROW_RUNID );
    run.jobId = row.GetString( RUNROW_JOBID );
    run.itemLabel = row.GetString( RUNROW_ITEMLABEL );
    CORE::CString cause = row.GetString( RUNROW_CAUSE );
    run.cause = CORE::JOBRUNCAUSE_UNSPECIFIED;
    for ( Int32 c=CORE::JOBRUNCAUSE_SCHEDULE; c<=CORE::JOBRUNCAUSE_HOLD; ++c )
    {
        if ( cause == CORE::JobRunCauseToString( (CORE::TJobRunCause) c ) )
            run.cause = (CORE::TJobRunCause) c;
    }
    run.state = (CORE::TJobRunState) row.GetEnum( RUNROW_STATE );
    run.scheduledUtcMs = MicrosToMs( row.GetTimestamp( RUNROW_SCHEDULED ) );
    run.startedUtcMs = MicrosToMs( row.GetTimestamp( RUNROW_STARTED ) );
    run.endedUtcMs = MicrosToMs( row.GetTimestamp( RUNROW_ENDED ) );
    run.attempt = row.GetUInt32( RUNROW_ATTEMPT );
    run.progressDone = row.GetUInt64( RUNROW_PROGRESSDONE );
    run.progressTotal = row.GetUInt64( RUNROW_PROGRESSTOTAL );
    run.note = row.GetString( RUNROW_NOTE );
    run.outcomeMessage = row.GetString( RUNROW_MESSAGE );
    run.requestedBy = row.GetString( RUNROW_REQUESTEDBY );
    run.taskId = row.GetUInt32( RUNROW_TASKID );
    run.revision = row.GetUInt64( RUNROW_REVISION );
    run.taskTypeName = row.GetString( RUNROW_TASKTYPE );
    run.executionTarget = (CORE::TJobExecutionTarget) row.GetEnum( RUNROW_EXECUTIONTARGET );
    run.threadPoolName = row.GetString( RUNROW_THREADPOOL );
}

/*-------------------------------------------------------------------------*/

void
CSchedulerRowCodec::ActionFromMessage( const COMCORE::CRpcMessage& row, SSchedulerActionInfo& action )
{GUCEF_TRACE;

    action.action = (CORE::TJobControlAction) row.GetEnum( ACTIONINFO_ACTION );
    action.targetKind = (CORE::TJobControlTargetKind) row.GetEnum( ACTIONINFO_TARGETKIND );
    action.customName = row.GetString( ACTIONINFO_CUSTOMNAME );
    action.title = row.GetString( ACTIONINFO_TITLE );
    action.argumentNames.clear();
    UInt32 count = row.GetCount( ACTIONINFO_ARGUMENTNAMES );
    for ( UInt32 i=0; i<count; ++i )
    {
        CORE::CVariant value;
        row.GetAt( ACTIONINFO_ARGUMENTNAMES, i, value );
        action.argumentNames.push_back( value.AsString() );
    }
}

/*-------------------------------------------------------------------------*/

void
CSchedulerRowCodec::JobToCells( const CORE::SJobStatus& job, CORE::CVariant::VariantVector& cells )
{GUCEF_TRACE;

    cells.clear();
    cells.push_back( TextCell( job.title, MaxIdBytes ) );
    cells.push_back( TextCell( job.laneName, MaxIdBytes ) );
    cells.push_back( CORE::CVariant( CORE::JobStateToString( job.state ) ) );
    cells.push_back( CORE::CVariant( job.priority ) );
    cells.push_back( TextCell( job.triggerText, MaxTextBytes ) );
    cells.push_back( TimestampCell( job.nextDueUtcMs ) );
    cells.push_back( TextCell( job.waitingFor, MaxIdBytes ) );
    cells.push_back( TimestampCell( job.waitingUntilUtcMs ) );
    cells.push_back( CORE::CVariant( job.lastRunId ) );
    cells.push_back( CORE::CVariant( CORE::JobRunStateToString( job.lastRunState ) ) );
    cells.push_back( TimestampCell( job.lastRunEndedUtcMs ) );
    cells.push_back( CORE::CVariant( job.runCount ) );
    cells.push_back( CORE::CVariant( job.failureCount ) );
    cells.push_back( CORE::CVariant( job.consecutiveFailures ) );
    cells.push_back( CORE::CVariant( job.revision ) );
    cells.push_back( TextCell( job.taskTypeName, MaxIdBytes ) );
    cells.push_back( CORE::CVariant( CORE::JobExecutionTargetToString( job.executionTarget ) ) );
    cells.push_back( TextCell( job.threadPoolName, MaxIdBytes ) );
    cells.push_back( CORE::CVariant( JobOriginToString( job.origin ) ) );
    cells.push_back( TextCell( job.submittedBy, MaxIdBytes ) );
    cells.push_back( TextCell( job.unavailableReason, MaxTextBytes ) );
    cells.push_back( CORE::CVariant( job.activeRunCount ) );
}

/*-------------------------------------------------------------------------*/

void
CSchedulerRowCodec::LaneToCells( const CORE::SJobLaneStatus& lane, CORE::CVariant::VariantVector& cells )
{GUCEF_TRACE;

    cells.clear();
    cells.push_back( TextCell( lane.mode, MaxIdBytes ) );
    cells.push_back( TextCell( lane.state, MaxIdBytes ) );
    cells.push_back( CORE::CVariant( lane.inFlight ) );
    cells.push_back( CORE::CVariant( lane.maxConcurrent ) );
    for ( UInt32 p=0; p<4; ++p )
        cells.push_back( CORE::CVariant( (UInt32) ( p < lane.queuedByPriority.size() ? lane.queuedByPriority[ p ] : 0 ) ) );
    cells.push_back( TextCell( lane.waitingFor, MaxIdBytes ) );
    cells.push_back( TimestampCell( lane.waitingUntilUtcMs ) );
    cells.push_back( TextCell( lane.unavailableReason, MaxTextBytes ) );
    cells.push_back( TextCell( lane.heldBy, MaxIdBytes ) );
    cells.push_back( CORE::CVariant( lane.revision ) );
}

/*-------------------------------------------------------------------------*/

void
CSchedulerRowCodec::RunToCells( const CORE::SJobRunRecord& run, CORE::CVariant::VariantVector& cells )
{GUCEF_TRACE;

    cells.clear();
    cells.push_back( TextCell( run.jobId, MaxIdBytes ) );
    cells.push_back( TextCell( run.itemLabel, MaxIdBytes ) );
    cells.push_back( CORE::CVariant( CORE::JobRunCauseToString( run.cause ) ) );
    cells.push_back( CORE::CVariant( CORE::JobRunStateToString( run.state ) ) );
    cells.push_back( TimestampCell( run.scheduledUtcMs ) );
    cells.push_back( TimestampCell( run.startedUtcMs ) );
    cells.push_back( TimestampCell( run.endedUtcMs ) );
    cells.push_back( CORE::CVariant( run.attempt ) );
    cells.push_back( CORE::CVariant( run.progressDone ) );
    cells.push_back( CORE::CVariant( run.progressTotal ) );
    cells.push_back( TextCell( run.note, CORE::SJobRunRecord::MaxNoteBytes ) );
    cells.push_back( TextCell( run.outcomeMessage, CORE::SJobRunRecord::MaxOutcomeMessageBytes ) );
    cells.push_back( TextCell( run.requestedBy, MaxIdBytes ) );
    cells.push_back( CORE::CVariant( run.taskId ) );
    cells.push_back( CORE::CVariant( run.revision ) );
    cells.push_back( TextCell( run.taskTypeName, MaxIdBytes ) );
    cells.push_back( CORE::CVariant( CORE::JobExecutionTargetToString( run.executionTarget ) ) );
    cells.push_back( TextCell( run.threadPoolName, MaxIdBytes ) );
}

/*-------------------------------------------------------------------------*/

void
CSchedulerRowCodec::FlattenThrottle( const CORE::CDataNode& throttleStatus, CORE::CString::StringMap& values )
{GUCEF_TRACE;

    values.clear();
    UInt32 attCount = throttleStatus.GetAttCount();
    for ( UInt32 a=0; a<attCount && values.size()<MaxThrottleColumns; ++a )
    {
        const CORE::CDataNode::TKeyValuePair* att = throttleStatus.GetAttribute( a );
        if ( GUCEF_NULL != att )
            values[ "throttle." + att->first ] = Bound( att->second.AsString(), MaxIdBytes );
    }
    CORE::CDataNode::const_iterator c = throttleStatus.ConstBegin();
    while ( c != throttleStatus.ConstEnd() && values.size()<MaxThrottleColumns )
    {
        const CORE::CDataNode& child = *(*c);
        if ( child.GetValue().IsInitialized() && !child.GetName().IsNULLOrEmpty() )
            values[ "throttle." + child.GetName() ] = Bound( child.GetValue().AsString(), MaxIdBytes );
        UInt32 childAtts = child.GetAttCount();
        for ( UInt32 a=0; a<childAtts && values.size()<MaxThrottleColumns; ++a )
        {
            const CORE::CDataNode::TKeyValuePair* att = child.GetAttribute( a );
            if ( GUCEF_NULL != att )
                values[ "throttle." + child.GetName() + "." + att->first ] = Bound( att->second.AsString(), MaxIdBytes );
        }
        ++c;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
