/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#include <algorithm>
#include <stdio.h>

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_CIMONOTONICCLOCK_H
#include "gucefCORE_CIMonotonicClock.h"
#define GUCEF_CORE_CIMONOTONICCLOCK_H
#endif /* GUCEF_CORE_CIMONOTONICCLOCK_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAllocationCounter.h"
#include "TestTaskPoolHotPath.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static MT::CTAtomic< UInt32 > g_hotPathTasksRun( 0 );

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** a task that is done in its first cycle: what is measured is the task framework around it */
class CHotPathEmptyTask : public CORE::CTaskConsumer
{
    public:

    static const CORE::CString TaskType;

    CHotPathEmptyTask( void ) : CORE::CTaskConsumer() {}
    virtual ~CHotPathEmptyTask() GUCEF_VIRTUAL_OVERRIDE {}

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return TaskType; }
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { return TaskType; }

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE
    {
        g_hotPathTasksRun.FetchAdd( 1, MT::MEMORDER_RELAXED );
        return true;
    }

    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CTaskConsumer::OnTaskEnded( task, wasForced );
    }
};

const CORE::CString CHotPathEmptyTask::TaskType = "HotPathEmptyTask";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static UInt64
NowInNs( void )
{
    return (UInt64) ( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() ) * 1000000.0 );
}

/*-------------------------------------------------------------------------*/

static Float64
Median( GUCEF::vector< Float64 >& values )
{
    if ( values.empty() )
        return 0.0;
    std::sort( values.begin(), values.end() );
    return values[ values.size() / 2 ];
}

/*-------------------------------------------------------------------------*/

/**
 *  Queues 'count' empty tasks on a pool without threads and carries them out on this thread: the queue, the hand
 *  over, the run and the clean up of a task without any thread switch, so the figure is the framework's own cost.
 *  Nanoseconds per task
 */
static Float64
MeasureCarriedOutOnCallingThread( CORE::ThreadPoolPtr pool, UInt32 count )
{
    UInt64 start = NowInNs();
    for ( UInt32 i=0; i<count; ++i )
        pool->QueueTask( CHotPathEmptyTask::TaskType );
    UInt32 done = 0;
    while ( done < count )
    {
        UInt32 carried = pool->CarryOutQueuedTasksIfAny( count - done );
        if ( 0 == carried )
            break;
        done += carried;
    }
    UInt64 end = NowInNs();
    return done == count ? (Float64) ( end - start ) / count : -1.0;
}

/*-------------------------------------------------------------------------*/

/** Queues 'count' empty tasks on a pool with worker threads and waits until all ran. Tasks per second */
static Float64
MeasureWorkerThroughput( CORE::ThreadPoolPtr pool, UInt32 count )
{
    g_hotPathTasksRun.Store( 0 );
    UInt64 start = NowInNs();
    for ( UInt32 i=0; i<count; ++i )
        pool->QueueTask( CHotPathEmptyTask::TaskType );
    UInt64 deadline = start + 60000000000ULL;
    while ( g_hotPathTasksRun.Load() < count && NowInNs() < deadline )
        MT::ThreadDelay( 1 );
    UInt64 end = NowInNs();
    if ( g_hotPathTasksRun.Load() < count )
        return -1.0;
    return count / ( (Float64) ( end - start ) / 1000000000.0 );
}

/*-------------------------------------------------------------------------*/

/**
 *  What the per pool counters and the task time stamps add per task, measured on their own: one relaxed atomic add
 *  at each of the three transitions (queued, started, ended), two cumulative time adds and the three monotonic clock
 *  reads of the time stamps. Nanoseconds per task
 */
static Float64
MeasureCounterOperationsAlone( UInt32 count )
{
    MT::CTAtomic< UInt64 > queued( 0 ), started( 0 ), finished( 0 ), waitMs( 0 ), runMs( 0 );
    const CORE::CIMonotonicClock& clock = CORE::CSystemMonotonicClock::Instance();
    UInt64 sink = 0;
    UInt64 begin = NowInNs();
    for ( UInt32 i=0; i<count; ++i )
    {
        UInt64 queuedAt = clock.GetMonotonicTimeInMs();
        queued.FetchAdd( 1, MT::MEMORDER_RELAXED );
        UInt64 startedAt = clock.GetMonotonicTimeInMs();
        started.FetchAdd( 1, MT::MEMORDER_RELAXED );
        waitMs.FetchAdd( startedAt - queuedAt, MT::MEMORDER_RELAXED );
        UInt64 endedAt = clock.GetMonotonicTimeInMs();
        finished.FetchAdd( 1, MT::MEMORDER_RELAXED );
        runMs.FetchAdd( endedAt - startedAt, MT::MEMORDER_RELAXED );
        sink += endedAt;
    }
    UInt64 end = NowInNs();
    if ( 0 == sink + finished.Load() )
        return -1.0;
    return (Float64) ( end - begin ) / count;
}

/*-------------------------------------------------------------------------*/

void
PerformTaskPoolHotPathTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "TaskPoolHotPath" );

    CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
    typedef CORE::CTFactory< CORE::CTaskConsumer, CHotPathEmptyTask, MT::CMutex > TEmptyFactory;
    TEmptyFactory emptyFactory;
    taskManager.RegisterTaskConsumerFactory( CHotPathEmptyTask::TaskType, &emptyFactory );

    CORE::CString report;

    GUCEF_TESTFW_TESTCASE( "TM-T8: thread pool hot path figures (empty tasks)" )
    {
        // a pool that never gets a thread: its tasks are carried out by the caller
        CORE::ThreadPoolPtr callerPool = taskManager.GetOrCreateThreadPool( "test.hotpath.caller" );
        ASSERT_TRUE( !callerPool.IsNULL() );
        callerPool->SetDesiredMaxTotalNrOfThreads( 0 );

        // warm up: task objects and consumers are reused from the free lists afterwards
        MeasureCarriedOutOnCallingThread( callerPool, 2000 );

        GUCEF::vector< Float64 > callerNs;
        for ( UInt32 round=0; round<7; ++round )
            callerNs.push_back( MeasureCarriedOutOnCallingThread( callerPool, 20000 ) );
        Float64 callerMedianNs = Median( callerNs );
        ASSERT_TRUE( callerMedianNs > 0.0 );

        // allocations per task after warm-up, carried out on this thread
        Float64 allocationsPerTask = -1.0;
        if ( CORETEST::IsAllocationCountingAvailable() )
        {
            CORETEST::StartAllocationCounting();
            MeasureCarriedOutOnCallingThread( callerPool, 1000 );
            allocationsPerTask = CORETEST::StopAllocationCounting() / 1000.0;
        }

        // worker threads: two of them take the tasks queued by this thread
        CORE::ThreadPoolPtr workerPool = taskManager.GetOrCreateThreadPool( "test.hotpath.workers" );
        ASSERT_TRUE( !workerPool.IsNULL() );
        workerPool->SetDesiredMaxTotalNrOfThreads( 2 );
        workerPool->SetDesiredMinNrOfWorkerThreads( 2 );
        // a worker thread takes one task per thread cycle and waits its minimal cycle delta between cycles when the
        // queue is not woken: this figure follows the delegator's cycle pacing, not the per task cost
        MeasureWorkerThroughput( workerPool, 100 );
        GUCEF::vector< Float64 > workerRates;
        for ( UInt32 round=0; round<3; ++round )
            workerRates.push_back( MeasureWorkerThroughput( workerPool, 400 ) );
        Float64 workerMedianRate = Median( workerRates );
        ASSERT_TRUE( workerMedianRate > 0.0 );

        GUCEF::vector< Float64 > aloneNs;
        for ( UInt32 round=0; round<7; ++round )
            aloneNs.push_back( MeasureCounterOperationsAlone( 1000000 ) );
        Float64 aloneMedianNs = Median( aloneNs );

        char line[ 512 ];
        sprintf( line, "carriedOutOnCallingThreadNsPerTask=%.1f workerTasksPerSecond=%.0f allocationsPerTask=%.2f counterOpsNsPerTask=%.2f counterOpsShareOfCallingThreadPath=%.4f%%",
                 callerMedianNs, workerMedianRate, allocationsPerTask, aloneMedianNs, 100.0 * aloneMedianNs / callerMedianNs );
        report = line;
        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "TaskPoolHotPath: " << report );

        CORE::CString reportPath = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( reportPath, "TaskPoolHotPath_Benchmark.txt" );
        FILE* reportFile = fopen( reportPath.C_String(), "ab" );
        if ( GUCEF_NULL != reportFile )
        {
            fprintf( reportFile, "%s\n", line );
            fclose( reportFile );
        }

        // the counters and time stamps on their own must stay a small part of the cheapest task path
        ASSERT_TRUE( aloneMedianNs < callerMedianNs * 0.02 );

        workerPool->SetDesiredMinNrOfWorkerThreads( 0 );
        workerPool->SetDesiredMaxTotalNrOfThreads( 0 );
        // a pool with threads is unregistered once they stopped (its threads reference it)
        workerPool->RequestAllThreadsToStop( true, false, 5000 );
        taskManager.UnregisterThreadPool( workerPool );
        taskManager.UnregisterThreadPool( callerPool );
    }
    GUCEF_TESTFW_TESTCASE_END;

    taskManager.UnregisterTaskConsumerFactory( CHotPathEmptyTask::TaskType );
}

/*-------------------------------------------------------------------------*/
