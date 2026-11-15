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

#ifndef GUCEF_DRNUTILS_CSCHEDULERROWCODEC_H
#include "gucefDRNUTILS_CSchedulerRowCodec.h"
#define GUCEF_DRNUTILS_CSCHEDULERROWCODEC_H
#endif /* GUCEF_DRNUTILS_CSCHEDULERROWCODEC_H ? */

#include "gucefDRNUTILS_CTaskManagerRowCodec.h"

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

static void
SetTime( COMCORE::CRpcMessage& row, UInt32 field, Int64 utcMs )
{
    if ( 0 != utcMs )
        row.SetTimestamp( field, utcMs * 1000 );
}

/*-------------------------------------------------------------------------*/

static void
SetText( COMCORE::CRpcMessage& row, UInt32 field, const CORE::CString& text )
{
    if ( !text.IsNULLOrEmpty() )
        row.SetString( field, CSchedulerRowCodec::Bound( text, CSchedulerRowCodec::MaxIdBytes ) );
}

/*-------------------------------------------------------------------------*/

static CORE::CVariant
TextCell( const CORE::CString& text )
{
    if ( text.IsNULLOrEmpty() )
        return CORE::CVariant();
    return CORE::CVariant( CSchedulerRowCodec::Bound( text, CSchedulerRowCodec::MaxIdBytes ) );
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerRowCodec::PoolToMessage( const CTaskManagerSample::SPool& pool, COMCORE::CRpcMessage& row )
{GUCEF_TRACE;

    SetText( row, POOLROW_NAME, pool.name );
    row.SetUInt32( POOLROW_ACTIVEWORKERS, pool.activeWorkers );
    row.SetUInt32( POOLROW_DESIREDMIN, pool.desiredMinWorkers );
    row.SetInt32( POOLROW_DESIREDMAX, pool.desiredMaxThreads );
    row.SetUInt32( POOLROW_DEDICATED, pool.dedicatedThreads );
    row.SetUInt32( POOLROW_QUEUED, pool.queued );
    row.SetUInt32( POOLROW_RUNNING, pool.running );
    row.SetFloat64( POOLROW_STARTEDPS, pool.startedPerSecond );
    row.SetFloat64( POOLROW_FINISHEDPS, pool.finishedPerSecond );
    row.SetFloat64( POOLROW_FAILEDPS, pool.failedPerSecond );
    row.SetFloat64( POOLROW_CANCELLEDPS, pool.cancelledPerSecond );
    row.SetFloat64( POOLROW_AVGQUEUEWAIT, pool.avgQueueWaitMs );
    row.SetFloat64( POOLROW_AVGRUN, pool.avgRunMs );
    row.Set( POOLROW_UTILISATION, CORE::CVariant( pool.utilisationPercent ) );
    row.SetUInt64( POOLROW_STARTEDTOTAL, pool.startedTotal );
    row.SetUInt64( POOLROW_FINISHEDTOTAL, pool.finishedTotal );
    row.SetUInt64( POOLROW_FAILEDTOTAL, pool.failedTotal );
    row.SetUInt32( POOLROW_SCHEDULEDJOBS, pool.scheduledJobs );
    row.SetUInt64( POOLROW_CANCELLEDTOTAL, pool.cancelledTotal );
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerRowCodec::ThreadToMessage( const CTaskManagerSample::SThread& thread, COMCORE::CRpcMessage& row )
{GUCEF_TRACE;

    row.SetUInt64( THREADROW_ID, thread.systemThreadId );
    SetText( row, THREADROW_NAME, thread.name );
    SetText( row, THREADROW_POOL, thread.pool );
    SetText( row, THREADROW_STATUS, thread.status );
    row.SetUInt32( THREADROW_TASKID, thread.taskId );
    SetText( row, THREADROW_TASKTYPE, thread.taskType );
    SetTime( row, THREADROW_BUSYSINCE, thread.busySinceUtcMs );
    row.SetBool( THREADROW_EXTERNAL, thread.external );
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerRowCodec::TaskToMessage( const CTaskManagerSample::STask& task, COMCORE::CRpcMessage& row )
{GUCEF_TRACE;

    row.SetUInt32( TASKROW_ID, task.taskId );
    SetText( row, TASKROW_POOL, task.pool );
    SetText( row, TASKROW_TASKTYPE, task.taskType );
    SetText( row, TASKROW_STATUS, task.status );
    SetText( row, TASKROW_EXTRAINFO, task.extraInfo );
    SetTime( row, TASKROW_QUEUED, task.queuedUtcMs );
    SetTime( row, TASKROW_STARTED, task.startedUtcMs );
    row.SetUInt64( TASKROW_RUNNINGFOR, task.runningForMs );
    row.SetUInt64( TASKROW_THREADID, task.systemThreadId );
    SetText( row, TASKROW_SCHEDULER, task.schedulerName );
    SetText( row, TASKROW_JOBID, task.jobId );
    row.SetUInt64( TASKROW_RUNID, task.runId );
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerRowCodec::ScheduledJobToMessage( const CTaskManagerSample::SScheduledJob& job, COMCORE::CRpcMessage& row )
{GUCEF_TRACE;

    SetText( row, SJROW_KEY, job.jobKey );
    SetText( row, SJROW_SCHEDULER, job.schedulerName );
    SetText( row, SJROW_JOBID, job.jobId );
    SetText( row, SJROW_TITLE, job.title );
    SetText( row, SJROW_TASKTYPE, job.taskType );
    row.SetEnum( SJROW_EXECUTIONTARGET, job.executionTargetNumber );
    SetText( row, SJROW_POOL, job.pool );
    SetText( row, SJROW_LANE, job.lane );
    SetText( row, SJROW_STATE, job.state );
    SetTime( row, SJROW_NEXTDUE, job.nextDueUtcMs );
    row.SetUInt32( SJROW_ACTIVERUNS, job.activeRuns );
    SetText( row, SJROW_LASTRUNSTATE, job.lastRunState );
    SetTime( row, SJROW_LASTRUNENDED, job.lastRunEndedUtcMs );
    SetText( row, SJROW_ORIGIN, job.origin );
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerRowCodec::PoolFromMessage( const COMCORE::CRpcMessage& row, CTaskManagerSample::SPool& pool )
{GUCEF_TRACE;

    pool.name = row.GetString( POOLROW_NAME );
    pool.activeWorkers = row.GetUInt32( POOLROW_ACTIVEWORKERS );
    pool.desiredMinWorkers = row.GetUInt32( POOLROW_DESIREDMIN );
    pool.desiredMaxThreads = row.GetInt32( POOLROW_DESIREDMAX );
    pool.dedicatedThreads = row.GetUInt32( POOLROW_DEDICATED );
    pool.queued = row.GetUInt32( POOLROW_QUEUED );
    pool.running = row.GetUInt32( POOLROW_RUNNING );
    pool.startedPerSecond = row.GetFloat64( POOLROW_STARTEDPS );
    pool.finishedPerSecond = row.GetFloat64( POOLROW_FINISHEDPS );
    pool.failedPerSecond = row.GetFloat64( POOLROW_FAILEDPS );
    pool.cancelledPerSecond = row.GetFloat64( POOLROW_CANCELLEDPS );
    pool.avgQueueWaitMs = row.GetFloat64( POOLROW_AVGQUEUEWAIT );
    pool.avgRunMs = row.GetFloat64( POOLROW_AVGRUN );
    CORE::CVariant utilisation;
    row.Get( POOLROW_UTILISATION, utilisation );
    pool.utilisationPercent = utilisation.AsFloat32();
    pool.startedTotal = row.GetUInt64( POOLROW_STARTEDTOTAL );
    pool.finishedTotal = row.GetUInt64( POOLROW_FINISHEDTOTAL );
    pool.failedTotal = row.GetUInt64( POOLROW_FAILEDTOTAL );
    pool.scheduledJobs = row.GetUInt32( POOLROW_SCHEDULEDJOBS );
    pool.cancelledTotal = row.GetUInt64( POOLROW_CANCELLEDTOTAL );
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerRowCodec::TaskFromMessage( const COMCORE::CRpcMessage& row, CTaskManagerSample::STask& task )
{GUCEF_TRACE;

    task.taskId = row.GetUInt32( TASKROW_ID );
    task.pool = row.GetString( TASKROW_POOL );
    task.taskType = row.GetString( TASKROW_TASKTYPE );
    task.status = row.GetString( TASKROW_STATUS );
    task.extraInfo = row.GetString( TASKROW_EXTRAINFO );
    task.queuedUtcMs = row.GetTimestamp( TASKROW_QUEUED ) / 1000;
    task.startedUtcMs = row.GetTimestamp( TASKROW_STARTED ) / 1000;
    task.runningForMs = row.GetUInt64( TASKROW_RUNNINGFOR );
    task.systemThreadId = row.GetUInt64( TASKROW_THREADID );
    task.schedulerName = row.GetString( TASKROW_SCHEDULER );
    task.jobId = row.GetString( TASKROW_JOBID );
    task.runId = row.GetUInt64( TASKROW_RUNID );
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerRowCodec::PoolToCells( const CTaskManagerSample::SPool& pool, CORE::CVariant::VariantVector& cells )
{GUCEF_TRACE;

    cells.clear();
    cells.push_back( CORE::CVariant( pool.activeWorkers ) );
    cells.push_back( CORE::CVariant( pool.desiredMinWorkers ) );
    cells.push_back( CORE::CVariant( pool.desiredMaxThreads ) );
    cells.push_back( CORE::CVariant( pool.dedicatedThreads ) );
    cells.push_back( CORE::CVariant( pool.queued ) );
    cells.push_back( CORE::CVariant( pool.running ) );
    cells.push_back( CORE::CVariant( pool.startedPerSecond ) );
    cells.push_back( CORE::CVariant( pool.finishedPerSecond ) );
    cells.push_back( CORE::CVariant( pool.failedPerSecond ) );
    cells.push_back( CORE::CVariant( pool.cancelledPerSecond ) );
    cells.push_back( CORE::CVariant( pool.avgQueueWaitMs ) );
    cells.push_back( CORE::CVariant( pool.avgRunMs ) );
    cells.push_back( CORE::CVariant( pool.utilisationPercent ) );
    cells.push_back( CORE::CVariant( pool.startedTotal ) );
    cells.push_back( CORE::CVariant( pool.finishedTotal ) );
    cells.push_back( CORE::CVariant( pool.failedTotal ) );
    cells.push_back( CORE::CVariant( pool.cancelledTotal ) );
    cells.push_back( CORE::CVariant( pool.scheduledJobs ) );
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerRowCodec::ThreadToCells( const CTaskManagerSample::SThread& thread, CORE::CVariant::VariantVector& cells )
{GUCEF_TRACE;

    cells.clear();
    cells.push_back( TextCell( thread.name ) );
    cells.push_back( TextCell( thread.pool ) );
    cells.push_back( TextCell( thread.status ) );
    cells.push_back( CORE::CVariant( thread.taskId ) );
    cells.push_back( TextCell( thread.taskType ) );
    cells.push_back( CSchedulerRowCodec::TimestampCell( thread.busySinceUtcMs ) );
    cells.push_back( CORE::CVariant( thread.external ) );
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerRowCodec::TaskToCells( const CTaskManagerSample::STask& task, CORE::CVariant::VariantVector& cells )
{GUCEF_TRACE;

    cells.clear();
    cells.push_back( TextCell( task.pool ) );
    cells.push_back( TextCell( task.taskType ) );
    cells.push_back( TextCell( task.status ) );
    cells.push_back( TextCell( task.extraInfo ) );
    cells.push_back( CSchedulerRowCodec::TimestampCell( task.queuedUtcMs ) );
    cells.push_back( CSchedulerRowCodec::TimestampCell( task.startedUtcMs ) );
    cells.push_back( CORE::CVariant( task.runningForMs ) );
    cells.push_back( CORE::CVariant( task.systemThreadId ) );
    cells.push_back( TextCell( task.schedulerName ) );
    cells.push_back( TextCell( task.jobId ) );
    cells.push_back( CORE::CVariant( task.runId ) );
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerRowCodec::ScheduledJobToCells( const CTaskManagerSample::SScheduledJob& job, CORE::CVariant::VariantVector& cells )
{GUCEF_TRACE;

    cells.clear();
    cells.push_back( TextCell( job.schedulerName ) );
    cells.push_back( TextCell( job.jobId ) );
    cells.push_back( TextCell( job.title ) );
    cells.push_back( TextCell( job.taskType ) );
    cells.push_back( TextCell( job.executionTarget ) );
    cells.push_back( TextCell( job.pool ) );
    cells.push_back( TextCell( job.lane ) );
    cells.push_back( TextCell( job.state ) );
    cells.push_back( CSchedulerRowCodec::TimestampCell( job.nextDueUtcMs ) );
    cells.push_back( CORE::CVariant( job.activeRuns ) );
    cells.push_back( TextCell( job.lastRunState ) );
    cells.push_back( CSchedulerRowCodec::TimestampCell( job.lastRunEndedUtcMs ) );
    cells.push_back( TextCell( job.origin ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
