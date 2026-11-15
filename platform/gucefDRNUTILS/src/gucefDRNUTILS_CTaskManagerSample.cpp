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

#ifndef GUCEF_MT_THREADSTATUS_H
#include "gucefMT_ThreadStatus.h"
#define GUCEF_MT_THREADSTATUS_H
#endif /* GUCEF_MT_THREADSTATUS_H ? */

#ifndef GUCEF_CORE_TASKSTATUS_H
#include "gucefCORE_TaskStatus.h"
#define GUCEF_CORE_TASKSTATUS_H
#endif /* GUCEF_CORE_TASKSTATUS_H ? */

#ifndef GUCEF_DRNUTILS_CSCHEDULERROWCODEC_H
#include "gucefDRNUTILS_CSchedulerRowCodec.h"
#define GUCEF_DRNUTILS_CSCHEDULERROWCODEC_H
#endif /* GUCEF_DRNUTILS_CSCHEDULERROWCODEC_H ? */

#include "gucefDRNUTILS_CTaskManagerSample.h"

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

CTaskManagerSample::SPool::SPool( void )
    : name(), activeWorkers( 0 ), desiredMinWorkers( 0 ), desiredMaxThreads( 0 ), dedicatedThreads( 0 ), queued( 0 ), running( 0 )
    , startedPerSecond( 0 ), finishedPerSecond( 0 ), failedPerSecond( 0 ), cancelledPerSecond( 0 ), avgQueueWaitMs( 0 ), avgRunMs( 0 )
    , utilisationPercent( 0 ), startedTotal( 0 ), finishedTotal( 0 ), failedTotal( 0 ), cancelledTotal( 0 ), scheduledJobs( 0 )
    , cumulativeQueueWaitMs( 0 ), cumulativeRunMs( 0 )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CTaskManagerSample::SThread::SThread( void )
    : systemThreadId( 0 ), name(), pool(), status(), taskId( 0 ), taskType(), busySinceUtcMs( 0 ), external( false )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CTaskManagerSample::STask::STask( void )
    : taskId( 0 ), pool(), taskType(), status(), extraInfo(), queuedUtcMs( 0 ), startedUtcMs( 0 ), runningForMs( 0 ), systemThreadId( 0 )
    , schedulerName(), jobId(), runId( 0 ), ended( false )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CTaskManagerSample::SScheduledJob::SScheduledJob( void )
    : jobKey(), schedulerName(), jobId(), title(), taskType(), executionTarget(), executionTargetNumber( 0 ), pool(), lane(), state(), nextDueUtcMs( 0 )
    , activeRuns( 0 ), lastRunState(), lastRunEndedUtcMs( 0 ), origin()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CTaskManagerSample::SOptions::SOptions( void )
    : includeThreads( false ), includeTasks( false ), includeScheduledJobs( true ), maxThreadRows( 512 ), maxTaskRows( 512 ), finishedTaskRows( 64 )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CTaskManagerSample::CTaskManagerSample( void )
    : sampleUtcMs( 0 ), sampleMonotonicMs( 0 ), truncated( false ), hasThreads( false ), hasTasks( false ), hasScheduledJobs( false )
    , pools(), threads(), tasks(), scheduledJobs()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerSample::Clear( void )
{GUCEF_TRACE;

    sampleUtcMs = 0;
    sampleMonotonicMs = 0;
    truncated = false;
    hasThreads = false;
    hasTasks = false;
    hasScheduledJobs = false;
    pools.clear();
    threads.clear();
    tasks.clear();
    scheduledJobs.clear();
}

/*-------------------------------------------------------------------------*/

static Int64
MonotonicToUtc( UInt64 monotonicMs, UInt64 nowMonotonicMs, Int64 nowUtcMs )
{
    if ( 0 == monotonicMs )
        return 0;
    if ( monotonicMs >= nowMonotonicMs )
        return nowUtcMs;
    return nowUtcMs - (Int64) ( nowMonotonicMs - monotonicMs );
}

/*-------------------------------------------------------------------------*/

static Float64
PerSecond( UInt64 now, UInt64 before, UInt64 elapsedMs )
{
    if ( 0 == elapsedMs || now < before )
        return 0.0;
    return (Float64) ( now - before ) * 1000.0 / (Float64) elapsedMs;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerSample::Take( const CITaskManagerSource& source, const CTaskManagerSample* previous, const CTaskManagerSample* previousWithTasks,
                          const SOptions& options )
{GUCEF_TRACE;

    // keeps the capacity of the vectors: a sample reuses its buffers (5.3)
    pools.clear();
    threads.clear();
    tasks.clear();
    scheduledJobs.clear();
    truncated = false;
    sampleUtcMs = source.GetUtcTimeInMs();
    sampleMonotonicMs = source.GetMonotonicTimeInMs();

    CORE::CTaskManager::TThreadPoolInfoMap poolInfo;
    CORE::CTaskManager::TThreadPoolCounterMap counters;
    source.GetAllThreadPoolInfo( poolInfo );
    source.GetPoolCounters( counters );
    CORE::CTaskManager::TThreadPoolInfoMap::const_iterator p = poolInfo.begin();
    while ( p != poolInfo.end() )
    {
        const CORE::CThreadPoolInfo& info = (*p).second;
        SPool pool;
        pool.name = (*p).first;
        pool.activeWorkers = info.GetActiveNrOfWorkerThreads();
        pool.desiredMinWorkers = info.GetDesiredMinNrOfWorkerThreads();
        pool.desiredMaxThreads = info.GetDesiredMaxNrOfThreads();
        pool.dedicatedThreads = info.GetActiveNrOfDedicatedThreads();
        pool.queued = info.GetQueuedTaskCount();
        pool.running = info.GetRunningTaskCount();
        pool.scheduledJobs = info.GetScheduledJobCount();
        CORE::CTaskManager::TThreadPoolCounterMap::const_iterator c = counters.find( pool.name );
        const CORE::CThreadPoolCounters& counter = c != counters.end() ? (*c).second : info.GetCounters();
        pool.startedTotal = counter.GetStartedTotal();
        pool.finishedTotal = counter.GetFinishedTotal();
        pool.failedTotal = counter.GetFailedTotal();
        pool.cancelledTotal = counter.GetCancelledTotal();
        pool.cumulativeQueueWaitMs = counter.GetCumulativeQueueWaitMs();
        pool.cumulativeRunMs = counter.GetCumulativeRunMs();
        if ( GUCEF_NULL != previous && sampleMonotonicMs > previous->sampleMonotonicMs )
        {
            UInt64 elapsedMs = sampleMonotonicMs - previous->sampleMonotonicMs;
            for ( UInt32 i=0; i<previous->pools.size(); ++i )
            {
                const SPool& before = previous->pools[ i ];
                if ( before.name != pool.name )
                    continue;
                pool.startedPerSecond = PerSecond( pool.startedTotal, before.startedTotal, elapsedMs );
                pool.finishedPerSecond = PerSecond( pool.finishedTotal, before.finishedTotal, elapsedMs );
                pool.failedPerSecond = PerSecond( pool.failedTotal, before.failedTotal, elapsedMs );
                pool.cancelledPerSecond = PerSecond( pool.cancelledTotal, before.cancelledTotal, elapsedMs );
                UInt64 started = pool.startedTotal >= before.startedTotal ? pool.startedTotal - before.startedTotal : 0;
                UInt64 ended = ( pool.finishedTotal + pool.failedTotal + pool.cancelledTotal );
                UInt64 endedBefore = ( before.finishedTotal + before.failedTotal + before.cancelledTotal );
                ended = ended >= endedBefore ? ended - endedBefore : 0;
                UInt64 queueWait = pool.cumulativeQueueWaitMs >= before.cumulativeQueueWaitMs ? pool.cumulativeQueueWaitMs - before.cumulativeQueueWaitMs : 0;
                UInt64 runMs = pool.cumulativeRunMs >= before.cumulativeRunMs ? pool.cumulativeRunMs - before.cumulativeRunMs : 0;
                pool.avgQueueWaitMs = started > 0 ? (Float64) queueWait / (Float64) started : 0.0;
                pool.avgRunMs = ended > 0 ? (Float64) runMs / (Float64) ended : 0.0;
                UInt32 workers = pool.activeWorkers + pool.dedicatedThreads;
                if ( workers > 0 )
                {
                    Float64 utilisation = (Float64) runMs * 100.0 / ( (Float64) elapsedMs * (Float64) workers );
                    pool.utilisationPercent = (Float32) ( utilisation > 100.0 ? 100.0 : utilisation );
                }
                break;
            }
        }
        pools.push_back( pool );
        ++p;
    }

    if ( options.includeThreads )
    {
        hasThreads = true;
        CORE::CTaskManager::TThreadInfoMap threadInfo;
        source.GetAllThreadInfo( threadInfo );
        CORE::CTaskManager::TThreadInfoMap::const_iterator t = threadInfo.begin();
        while ( t != threadInfo.end() )
        {
            if ( options.maxThreadRows > 0 && threads.size() >= options.maxThreadRows )
            {
                truncated = true;
                break;
            }
            const CORE::CThreadInfo& info = (*t).second;
            SThread thread;
            thread.systemThreadId = (UInt64) info.GetThreadId();
            thread.name = CSchedulerRowCodec::Bound( info.GetThreadName(), CSchedulerRowCodec::MaxIdBytes );
            thread.pool = info.GetThreadPoolName();
            thread.status = MT::ThreadStatusToThreadStatusString( info.GetThreadStatus() );
            thread.taskId = info.GetCurrentTaskId();
            thread.taskType = info.GetCurrentTaskTypeName();
            thread.busySinceUtcMs = MonotonicToUtc( info.GetBusySinceMs(), sampleMonotonicMs, sampleUtcMs );
            thread.external = info.GetIsExternal();
            threads.push_back( thread );
            ++t;
        }
    }
    else
    {
        hasThreads = false;
    }

    // hidden jobs never leave the process (design 8.1): they are not listed, and a task of one of their runs is listed
    // without its scheduler, job and run
    CORE::TScheduledJobInfoVector jobs;
    if ( options.includeTasks || options.includeScheduledJobs )
        source.GetAllScheduledJobInfo( jobs );
    CORE::CString::StringSet hiddenJobKeys;
    for ( UInt32 i=0; i<jobs.size(); ++i )
        if ( jobs[ i ].IsHidden() )
            hiddenJobKeys.insert( jobs[ i ].GetSchedulerName() + "/" + jobs[ i ].GetJobId() );

    if ( options.includeTasks )
    {
        hasTasks = true;
        CORE::CTaskManager::TTaskInfoMap taskInfo;
        source.GetAllTaskInfo( taskInfo );
        CORE::CTaskManager::TTaskInfoMap::const_iterator t = taskInfo.begin();
        while ( t != taskInfo.end() )
        {
            if ( options.maxTaskRows > 0 && tasks.size() >= options.maxTaskRows )
            {
                truncated = true;
                break;
            }
            const CORE::CTaskInfo& info = (*t).second;
            STask task;
            task.taskId = info.GetTaskId();
            task.pool = info.GetThreadPoolName();
            task.taskType = info.GetTaskTypeName();
            task.status = CORE::TaskStatusToTaskStatusString( info.GetTaskStatus() );
            task.queuedUtcMs = MonotonicToUtc( info.GetQueuedAtMs(), sampleMonotonicMs, sampleUtcMs );
            task.startedUtcMs = MonotonicToUtc( info.GetStartedAtMs(), sampleMonotonicMs, sampleUtcMs );
            task.runningForMs = 0 != info.GetStartedAtMs() && sampleMonotonicMs > info.GetStartedAtMs() ? sampleMonotonicMs - info.GetStartedAtMs() : 0;
            task.systemThreadId = (UInt64) info.GetThreadId();
            task.schedulerName = info.GetSchedulerName();
            task.jobId = info.GetScheduledJobId();
            task.runId = info.GetScheduledRunId();
            if ( !hiddenJobKeys.empty() && !task.jobId.IsNULLOrEmpty() && hiddenJobKeys.find( task.schedulerName + "/" + task.jobId ) != hiddenJobKeys.end() )
            {
                task.schedulerName.Clear();
                task.jobId.Clear();
                task.runId = 0;
            }
            tasks.push_back( task );
            ++t;
        }
        // tasks of the previous sample that are gone: the last ended ones seen (tasks that start and end between two
        // samples are counted by the counters, not listed)
        if ( GUCEF_NULL != previousWithTasks && previousWithTasks->hasTasks && options.finishedTaskRows > 0 )
        {
            UInt32 kept = 0;
            for ( UInt32 i=0; i<previousWithTasks->tasks.size() && kept < options.finishedTaskRows; ++i )
            {
                const STask& before = previousWithTasks->tasks[ i ];
                if ( taskInfo.find( before.taskId ) != taskInfo.end() )
                    continue;
                STask ended( before );
                if ( !ended.ended )
                    ended.status = "ENDED";
                ended.ended = true;
                tasks.push_back( ended );
                ++kept;
            }
        }
    }
    else
    {
        hasTasks = false;
    }

    if ( options.includeScheduledJobs )
    {
        hasScheduledJobs = true;
        for ( UInt32 i=0; i<jobs.size(); ++i )
        {
            const CORE::CScheduledJobInfo& info = jobs[ i ];
            if ( info.IsHidden() )
                continue;
            SScheduledJob job;
            job.schedulerName = info.GetSchedulerName();
            job.jobId = info.GetJobId();
            job.jobKey = job.schedulerName + "/" + job.jobId;
            job.title = CSchedulerRowCodec::Bound( info.GetTitle(), CSchedulerRowCodec::MaxIdBytes );
            job.taskType = info.GetTaskTypeName();
            job.executionTarget = CORE::JobExecutionTargetToString( info.GetExecutionTarget() );
            job.executionTargetNumber = (Int32) info.GetExecutionTarget();
            job.pool = info.GetThreadPoolName();
            job.lane = info.GetLaneName();
            job.state = CORE::JobStateToString( info.GetState() );
            job.nextDueUtcMs = info.GetNextDueUtcMs();
            job.activeRuns = info.GetActiveRunCount();
            job.lastRunState = CORE::JobRunStateToString( info.GetLastRunState() );
            job.lastRunEndedUtcMs = info.GetLastRunEndedUtcMs();
            job.origin = CSchedulerRowCodec::JobOriginToString( info.GetOrigin() );
            scheduledJobs.push_back( job );
        }
    }
    else
    {
        hasScheduledJobs = false;
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
