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

#ifndef GUCEF_DRNUTILS_CTASKMANAGERSAMPLE_H
#define GUCEF_DRNUTILS_CTASKMANAGERSAMPLE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRNUTILS_CITASKMANAGERSOURCE_H
#include "gucefDRNUTILS_CITaskManagerSource.h"
#define GUCEF_DRNUTILS_CITASKMANAGERSOURCE_H
#endif /* GUCEF_DRNUTILS_CITASKMANAGERSOURCE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Module-private: one sample of the task manager (design 5.1, 5.3): pools with rates from the counters of two
 *  samples, threads, tasks (in use plus the last ended ones seen) and the scheduled jobs. Never task data.
 *  Times: UTC milliseconds; the task manager's monotonic times are translated at the sample instant.
 */
class CTaskManagerSample
{
    public:

    struct SPool
    {
        CORE::CString name;
        UInt32 activeWorkers;
        UInt32 desiredMinWorkers;
        Int32 desiredMaxThreads;
        UInt32 dedicatedThreads;
        UInt32 queued;
        UInt32 running;
        Float64 startedPerSecond;
        Float64 finishedPerSecond;
        Float64 failedPerSecond;
        Float64 cancelledPerSecond;
        Float64 avgQueueWaitMs;
        Float64 avgRunMs;
        Float32 utilisationPercent;
        UInt64 startedTotal;
        UInt64 finishedTotal;
        UInt64 failedTotal;
        UInt64 cancelledTotal;
        UInt32 scheduledJobs;
        UInt64 cumulativeQueueWaitMs;   /**< for the next sample's averages, not published */
        UInt64 cumulativeRunMs;
        SPool( void );
    };
    struct SThread
    {
        UInt64 systemThreadId;
        CORE::CString name;
        CORE::CString pool;
        CORE::CString status;
        UInt32 taskId;
        CORE::CString taskType;
        Int64 busySinceUtcMs;
        bool external;
        SThread( void );
    };
    struct STask
    {
        UInt32 taskId;
        CORE::CString pool;
        CORE::CString taskType;
        CORE::CString status;
        CORE::CString extraInfo;
        Int64 queuedUtcMs;
        Int64 startedUtcMs;
        UInt64 runningForMs;
        UInt64 systemThreadId;
        CORE::CString schedulerName;
        CORE::CString jobId;
        UInt64 runId;
        bool ended;                     /**< seen in a previous sample, gone in this one */
        STask( void );
    };
    struct SScheduledJob
    {
        CORE::CString jobKey;
        CORE::CString schedulerName;
        CORE::CString jobId;
        CORE::CString title;
        CORE::CString taskType;
        CORE::CString executionTarget;
        Int32 executionTargetNumber;    /**< the contract's ExecutionTarget */
        CORE::CString pool;
        CORE::CString lane;
        CORE::CString state;
        Int64 nextDueUtcMs;
        UInt32 activeRuns;
        CORE::CString lastRunState;
        Int64 lastRunEndedUtcMs;
        CORE::CString origin;
        SScheduledJob( void );
    };
    typedef GUCEF::vector< SPool > TPoolVector;
    typedef GUCEF::vector< SThread > TThreadVector;
    typedef GUCEF::vector< STask > TTaskVector;
    typedef GUCEF::vector< SScheduledJob > TScheduledJobVector;

    struct SOptions
    {
        bool includeThreads;
        bool includeTasks;
        bool includeScheduledJobs;
        UInt32 maxThreadRows;           /**< 0 = no maximum */
        UInt32 maxTaskRows;             /**< 0 = no maximum */
        UInt32 finishedTaskRows;
        SOptions( void );
    };

    Int64 sampleUtcMs;
    UInt64 sampleMonotonicMs;
    bool truncated;
    bool hasThreads;
    bool hasTasks;
    bool hasScheduledJobs;
    TPoolVector pools;
    TThreadVector threads;
    TTaskVector tasks;
    TScheduledJobVector scheduledJobs;

    CTaskManagerSample( void );

    void Clear( void );

    /**
     *  Takes a sample from the source. With a previous sample the rates are the counter deltas over the elapsed time;
     *  the tasks of the last sample with tasks that are gone are kept as ended rows (at most finishedTaskRows).
     *  Threads and tasks are taken only when asked (the task walk is the costly part); the other parts are cheap.
     */
    void Take( const CITaskManagerSource& source, const CTaskManagerSample* previous, const CTaskManagerSample* previousWithTasks,
               const SOptions& options );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CTASKMANAGERSAMPLE_H ? */
