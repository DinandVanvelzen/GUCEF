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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef GUCEF_PLATFORM_H
#include "gucef_platform.h"
#define GUCEF_PLATFORM_H
#endif /* GUCEF_PLATFORM_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
#include <unistd.h>
#endif

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_THREADNAME_H
#include "gucefMT_ThreadName.h"
#define GUCEF_MT_THREADNAME_H
#endif /* GUCEF_MT_THREADNAME_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_CHILD_PROCESS_H
#include "gucef_test_child_process.h"
#define GUCEF_TEST_CHILD_PROCESS_H
#endif /* GUCEF_TEST_CHILD_PROCESS_H ? */

#include "TestJobSchedulerSupport.h"
#include "TestTaskManagerCounters.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace CORETEST;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Counts the scheduler registry events of the task manager */
class CTmRegistryEvents : public CORE::CObserver
{
    public:

    CTmRegistryEvents( void ) : created(), unregistered() {}
    virtual ~CTmRegistryEvents() GUCEF_VIRTUAL_OVERRIDE { UnsubscribeAllFromObserver(); }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( CORE::CTaskManager::JobSchedulerCreatedEvent == eventId && GUCEF_NULL != eventData )
            created.push_back( static_cast< CORE::CTaskManager::JobSchedulerCreatedEventData* >( eventData )->GetData() );
        if ( CORE::CTaskManager::JobSchedulerUnregisteredEvent == eventId && GUCEF_NULL != eventData )
            unregistered.push_back( static_cast< CORE::CTaskManager::JobSchedulerUnregisteredEventData* >( eventData )->GetData() );
    }

    CORE::CString::StringVector created;
    CORE::CString::StringVector unregistered;
};

/*-------------------------------------------------------------------------*/

/** The store of the shutdown child: notes whether the run pool of the scheduler was still there when it saved */
class CShutdownStateStore : public CORE::CIJobSchedulerStateStore
{
    public:

    CShutdownStateStore( const CORE::CString& runPoolName ) : saves( 0 ), poolPresentAtSave( false ), m_runPoolName( runPoolName ) {}
    virtual ~CShutdownStateStore() GUCEF_VIRTUAL_OVERRIDE {}

    virtual bool LoadState( CORE::CDataNode& state, CORE::CString& error ) GUCEF_VIRTUAL_OVERRIDE { state.Clear(); return true; }

    virtual bool SaveState( const CORE::CDataNode& state, CORE::CString& error ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++saves;
        poolPresentAtSave = !CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool( m_runPoolName ).IsNULL();
        return true;
    }

    UInt32 saves;
    bool poolPresentAtSave;

    private:

    CORE::CString m_runPoolName;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static CORE::CFutureResult
QueueJsTask( CORE::ThreadPoolPtr pool, const CORE::CString& value, bool fail, UInt32 blockMs = 0 )
{
    CJsTestTaskData data;
    data.value = value;
    data.fail = fail;
    data.blockMs = blockMs;
    return pool->QueueTask( CJsTestTaskData::TaskType, &data );
}

/*-------------------------------------------------------------------------*/

static bool
WaitForCount( const CORE::CString& value, UInt32 count, UInt32 timeoutMs )
{
    for ( UInt32 waited=0; waited<timeoutMs; waited += 2 )
    {
        if ( CountJsRuns( value ) >= count )
            return true;
        MT::ThreadDelay( 2 );
    }
    return CountJsRuns( value ) >= count;
}

/*-------------------------------------------------------------------------*/

int
RunJobSchedulerShutdownChild( void )
{
    RegisterJsTestTaskType();
    CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
    CORE::CJobSchedulerSettings settings = MakeJsSettings( "shutdown-child" );
    CShutdownStateStore store( settings.GetRunPoolName() );
    CORE::JobSchedulerPtr scheduler = taskManager.GetOrCreateJobScheduler( settings, CORE::CPulseGenerator::CreateSharedObj() );
    if ( scheduler.IsNULL() )
        return 2;
    scheduler->SetStateStore( &store );
    CORE::CString error;
    if ( !scheduler->AddJob( MakeJsJob( "endless", CORE::CJobTrigger::Manual(), "endless", 0 ), error ) || !scheduler->Start( error ) )
        return 3;
    CORE::SJobControlCommand command;
    command.action = CORE::JOBCONTROLACTION_RUN_NOW;
    command.targetKind = CORE::JOBCONTROLTARGET_JOB;
    command.targetId = "endless";
    CORE::SJobControlResult result;
    if ( CORE::JOBCONTROLSTATUS_OK != scheduler->Control( command, result ) )
        return 4;
    if ( !WaitForCount( "endless", 1, 5000 ) )
        return 5;

    taskManager.Shutdown();

    CORE::SJobRunRecord run;
    if ( !scheduler->GetRun( result.runId, run ) || CORE::JOBRUNSTATE_CANCELLED != run.state )
        return 6;
    if ( CORE::JOBSCHEDULERSTATE_STOPPED != scheduler->GetHostState() )
        return 7;
    if ( 0 == store.saves || !store.poolPresentAtSave )
        return 8;
    if ( !taskManager.GetJobScheduler( "shutdown-child" ).IsNULL() )
        return 9;
    scheduler.Unlink();
    return 0;
}

/*-------------------------------------------------------------------------*/

static void
WaitWhileTheTaskEndsAtExit( void )
{
    // Registered before the first use of the process wide monotonic clock, so it runs
    // after that clock's static would have been destroyed and before the application
    // stop that gucefCORE registers with atexit(): the pool thread ends its task in
    // this window.
    MT::ThreadDelay( 3000 );
}

/*-------------------------------------------------------------------------*/

int
RunPoolTaskEndsAtExitChild( void )
{
    atexit( WaitWhileTheTaskEndsAtExit );

    RegisterJsTestTaskType();
    CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
    CORE::ThreadPoolPtr pool = taskManager.GetOrCreateThreadPool( "tm.exit.child" );
    if ( pool.IsNULL() )
        return 2;
    pool->SetDesiredMaxTotalNrOfThreads( 1 );

    // the first cycle keeps its thread for 1 s, then the task ends on the pool thread
    CORE::CFutureResult future = QueueJsTask( pool, "exit", false, 1000 );
    for ( UInt32 waited=0; waited<5000 && 0 == pool->GetRunningTaskCount(); waited += 2 )
        MT::ThreadDelay( 2 );
    if ( 0 == pool->GetRunningTaskCount() )
        return 3;

    // return while the task runs: no RequestAllThreadsToStop(), as a library user may do
    return 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformCounterTests( void )
{
    GUCEF_TESTFW_TESTCASE( "TM-T1: the counters equal the queued, started, finished, failed and cancelled tasks after 10 000 tasks over 4 pools" )
    {
        CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
        ClearJsRunLog();
        const char* names[ 3 ] = { "tm.t1.a", "tm.t1.b", "tm.t1.c" };
        bool allEqual = true;
        for ( UInt32 p=0; p<3; ++p )
        {
            // carried out on this thread: 2400 finish, 300 fail at their start, 300 are cancelled while queued
            CORE::ThreadPoolPtr pool = taskManager.GetOrCreateThreadPool( names[ p ] );
            pool->SetDesiredMaxTotalNrOfThreads( 0 );
            for ( UInt32 i=0; i<3000; ++i )
            {
                bool cancel = ( i % 10 ) == 1;
                bool fail = ( i % 10 ) == 2;
                CORE::CFutureResult future = QueueJsTask( pool, "t1", fail );
                if ( cancel )
                    pool->RequestTaskCancellation( future.GetTaskId(), false );
                if ( 0 == ( i % 100 ) )
                    pool->CarryOutQueuedTasksIfAny( 100 );
            }
            while ( pool->CarryOutQueuedTasksIfAny( 100 ) > 0 ) {}

            CORE::CThreadPoolCounters counters;
            pool->GetCounters( counters );
            allEqual = allEqual && 3000 == counters.GetQueuedTotal() && 2700 == counters.GetStartedTotal() &&
                       2400 == counters.GetFinishedTotal() && 300 == counters.GetFailedTotal() &&
                       300 == counters.GetCancelledTotal() && 0 == counters.GetKilledTotal();
            if ( !allEqual )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "TM-T1 " << names[ p ] << ": queued " << counters.GetQueuedTotal() << " started " << counters.GetStartedTotal() << " finished " << counters.GetFinishedTotal() << " failed " << counters.GetFailedTotal() << " cancelled " << counters.GetCancelledTotal() );
        }
        ASSERT_TRUE( allEqual );

        // the fourth pool with worker threads: 1000 tasks
        CORE::ThreadPoolPtr workers = taskManager.GetOrCreateThreadPool( "tm.t1.workers" );
        workers->SetDesiredMaxTotalNrOfThreads( 2 );
        workers->SetDesiredMinNrOfWorkerThreads( 2 );
        for ( UInt32 i=0; i<1000; ++i )
            QueueJsTask( workers, "t1w", false );
        ASSERT_TRUE( WaitForCount( "t1w", 1000, 60000 ) );
        CORE::CThreadPoolCounters workerCounters;
        bool settled = false;
        for ( UInt32 i=0; i<2500 && !settled; ++i )
        {
            workers->GetCounters( workerCounters );
            settled = 1000 == workerCounters.GetFinishedTotal();
            if ( !settled )
                MT::ThreadDelay( 2 );
        }
        ASSERT_TRUE( 1000 == workerCounters.GetQueuedTotal() && 1000 == workerCounters.GetStartedTotal() && 1000 == workerCounters.GetFinishedTotal() );
        ASSERT_TRUE( 0 == workers->GetRunningTaskCount() );

        // the same through the task manager, without a task walk, and in the pool info
        CORE::CTaskManager::TThreadPoolCounterMap all;
        ASSERT_TRUE( taskManager.GetPoolCounters( all ) );
        UInt64 queued = 0, finished = 0;
        for ( UInt32 p=0; p<3; ++p )
        {
            queued += all[ names[ p ] ].GetQueuedTotal();
            finished += all[ names[ p ] ].GetFinishedTotal();
        }
        queued += all[ "tm.t1.workers" ].GetQueuedTotal();
        finished += all[ "tm.t1.workers" ].GetFinishedTotal();
        ASSERT_TRUE( 10000 == queued );
        ASSERT_TRUE( 8200 == finished );
        CORE::CThreadPoolInfo info;
        ASSERT_TRUE( taskManager.GetThreadPoolInfo( "tm.t1.a", info ) );
        ASSERT_TRUE( 300 == info.GetCounters().GetCancelledTotal() && 0 == info.GetRunningTaskCount() );

        workers->SetDesiredMinNrOfWorkerThreads( 0 );
        workers->SetDesiredMaxTotalNrOfThreads( 0 );
        workers->RequestAllThreadsToStop( true, false, 5000 );
        for ( UInt32 p=0; p<3; ++p )
            taskManager.UnregisterThreadPool( names[ p ] );
        taskManager.UnregisterThreadPool( "tm.t1.workers" );
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "TM-T2: the queue wait and the run time of a task are within tolerance" )
    {
        CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
        CORE::ThreadPoolPtr pool = taskManager.GetOrCreateThreadPool( "tm.t2" );
        pool->SetDesiredMaxTotalNrOfThreads( 0 );
        CORE::CFutureResult future = QueueJsTask( pool, "t2", false, 50 );
        CORE::CTaskInfo queuedInfo;
        ASSERT_TRUE( pool->GetTaskInfo( future.GetTaskId(), queuedInfo, false ) );
        ASSERT_TRUE( 0 != queuedInfo.GetQueuedAtMs() && 0 == queuedInfo.GetStartedAtMs() );
        ASSERT_TRUE( queuedInfo.GetThreadPoolName() == "tm.t2" );
        MT::ThreadDelay( 100 );
        pool->CarryOutQueuedTasksIfAny( 1 );
        CORE::CThreadPoolCounters counters;
        pool->GetCounters( counters );
        ASSERT_TRUE( counters.GetCumulativeQueueWaitMs() >= 95 && counters.GetCumulativeQueueWaitMs() < 1000 );
        ASSERT_TRUE( counters.GetCumulativeRunMs() >= 45 && counters.GetCumulativeRunMs() < 1000 );
        taskManager.UnregisterThreadPool( pool );

        // a running task: its start, and the thread that is busy with it since then
        CORE::ThreadPoolPtr workers = taskManager.GetOrCreateThreadPool( "tm.t2.run" );
        workers->SetDesiredMaxTotalNrOfThreads( 1 );
        workers->SetDesiredMinNrOfWorkerThreads( 1 );
        ClearJsRunLog();
        CORE::CFutureResult running = QueueJsTask( workers, "t2run", false, 400 );
        ASSERT_TRUE( WaitForCount( "t2run", 1, 5000 ) );
        CORE::CTaskInfo taskInfo;
        ASSERT_TRUE( workers->GetTaskInfo( running.GetTaskId(), taskInfo, false ) );
        ASSERT_TRUE( 0 != taskInfo.GetStartedAtMs() && taskInfo.GetStartedAtMs() >= taskInfo.GetQueuedAtMs() );
        TJsRunLog log;
        GetJsRunLog( log );
        CORE::CThreadInfo threadInfo;
        CORE::CString poolName;
        ASSERT_TRUE( 1 == log.size() && taskManager.GetThreadInfo( (TSystemThreadId) log[ 0 ].systemThreadId, threadInfo, poolName ) );
        ASSERT_TRUE( threadInfo.GetCurrentTaskId() == running.GetTaskId() );
        ASSERT_TRUE( threadInfo.GetBusySinceMs() == taskInfo.GetStartedAtMs() );
        ASSERT_TRUE( threadInfo.GetCurrentTaskTypeName() == CJsTestTaskData::TaskType );
        ASSERT_TRUE( threadInfo.GetThreadPoolName() == "tm.t2.run" );
        running.GetResult( 10000 );
        workers->SetDesiredMinNrOfWorkerThreads( 0 );
        workers->SetDesiredMaxTotalNrOfThreads( 0 );
        workers->RequestAllThreadsToStop( true, false, 5000 );
        taskManager.UnregisterThreadPool( workers );
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "TM-T3: pool threads are named <pool>#<n> in the thread info and at the level of the operating system" )
    {
        CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
        CORE::ThreadPoolPtr pool = taskManager.GetOrCreateThreadPool( "tm.names" );
        pool->SetDesiredMaxTotalNrOfThreads( 2 );
        pool->SetDesiredMinNrOfWorkerThreads( 2 );
        CORE::CThreadPool::TThreadInfoMap threads;
        UInt32 named = 0;
        for ( UInt32 i=0; i<2500 && named < 2; ++i )
        {
            pool->GetAllThreadInfo( threads );
            named = 0;
            for ( CORE::CThreadPool::TThreadInfoMap::iterator t = threads.begin(); t != threads.end(); ++t )
                if ( (*t).second.GetThreadName().HasSubstr( "tm.names#" ) == 0 )
                    ++named;
            if ( named < 2 )
                MT::ThreadDelay( 2 );
        }
        ASSERT_TRUE( 2 == named );
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
        for ( CORE::CThreadPool::TThreadInfoMap::iterator t = threads.begin(); t != threads.end(); ++t )
        {
            char path[ 128 ];
            sprintf( path, "/proc/self/task/%llu/comm", (unsigned long long) (*t).first );
            FILE* file = fopen( path, "r" );
            ASSERT_TRUE( GUCEF_NULL != file );
            if ( GUCEF_NULL != file )
            {
                char comm[ 64 ] = { 0 };
                if ( GUCEF_NULL != fgets( comm, sizeof( comm ), file ) )
                {
                    size_t length = strlen( comm );
                    if ( length > 0 && '\n' == comm[ length-1 ] )
                        comm[ length-1 ] = '\0';
                }
                fclose( file );
                ASSERT_TRUE( (*t).second.GetThreadName() == comm );
            }
        }
        #endif
        ASSERT_TRUE( MT::SetCurrentThreadName( "a-name-longer-than-fifteen" ) || GUCEF_PLATFORM != GUCEF_PLATFORM_LINUX );
        char current[ 64 ];
        if ( MT::GetCurrentThreadName( current, sizeof( current ) ) )
        {
            // the kernel of Linux keeps 15 bytes of a thread name, macOS keeps 63
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MACOS )
            ASSERT_TRUE( 0 == strcmp( current, "a-name-longer-than-fifteen" ) );
            #else
            ASSERT_TRUE( 0 == strcmp( current, "a-name-longer-t" ) );
            #endif
        }
        MT::SetCurrentThreadName( "gucefCORE_TestA" );
        pool->SetDesiredMinNrOfWorkerThreads( 0 );
        pool->SetDesiredMaxTotalNrOfThreads( 0 );
        pool->RequestAllThreadsToStop( true, false, 5000 );
        taskManager.UnregisterThreadPool( pool );
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "The thread pool info keeps its queued task count through a data node" )
    {
        CORE::CThreadPoolInfo info;
        info.SetThreadPoolName( "roundtrip" );
        info.SetQueuedTaskCount( 7 );
        info.SetRunningTaskCount( 3 );
        CORE::CThreadPoolCounters counters;
        counters.SetFinishedTotal( 11 );
        info.SetCounters( counters );
        CORE::CDataNode node;
        CORE::CDataNodeSerializableSettings settings;
        ASSERT_TRUE( info.Serialize( node, settings ) );
        CORE::CThreadPoolInfo back;
        ASSERT_TRUE( back.Deserialize( node, settings ) );
        ASSERT_TRUE( 7 == back.GetQueuedTaskCount() );
        ASSERT_TRUE( 3 == back.GetRunningTaskCount() );
        ASSERT_TRUE( 11 == back.GetCounters().GetFinishedTotal() );
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

static void
PerformOverviewTests( void )
{
    GUCEF_TESTFW_TESTCASE( "TM-T4: the task manager overview shows the scheduled jobs, which run on the main thread, and the run of a task" )
    {
        CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
        CORE::CTaskManagerInfo before;
        ASSERT_TRUE( taskManager.GetInfo( before ) );

        CORE::CJobSchedulerSettings settings = MakeJsSettings( "tm-t4-overview" );
        CORE::JobSchedulerPtr scheduler = MakeJsScheduler( settings, GUCEF_NULL );
        CORE::CString error;
        ASSERT_TRUE( scheduler->AddJob( MakeJsJob( "pooled", CORE::CJobTrigger::Manual(), "t4", 0 ), error ) );
        CORE::CJobDefinition mainJob = MakeJsJob( "on-main", CORE::CJobTrigger::Manual(), "t4main" );
        mainJob.SetExecutionTarget( CORE::JOBEXECUTIONTARGET_MAIN_THREAD );
        ASSERT_TRUE( scheduler->AddJob( mainJob, error ) );
        ASSERT_TRUE( scheduler->Start( error ) );

        CORE::TScheduledJobInfoVector jobs;
        ASSERT_TRUE( taskManager.GetAllScheduledJobInfo( jobs ) );
        UInt32 found = 0;
        for ( size_t i=0; i<jobs.size(); ++i )
        {
            if ( jobs[ i ].GetSchedulerName() != "tm-t4-overview" )
                continue;
            ++found;
            if ( jobs[ i ].GetJobId() == "on-main" )
                ASSERT_TRUE( CORE::JOBEXECUTIONTARGET_MAIN_THREAD == jobs[ i ].GetExecutionTarget() && jobs[ i ].GetThreadPoolName() == settings.GetMainThreadPoolName() );
            else
                ASSERT_TRUE( CORE::JOBEXECUTIONTARGET_POOL == jobs[ i ].GetExecutionTarget() && jobs[ i ].GetThreadPoolName() == settings.GetRunPoolName() );
        }
        ASSERT_TRUE( 2 == found );

        CORE::CTaskManagerInfo after;
        ASSERT_TRUE( taskManager.GetInfo( after ) );
        ASSERT_TRUE( after.GetJobSchedulerCount() == before.GetJobSchedulerCount() + 1 );
        ASSERT_TRUE( after.GetScheduledJobCount() == before.GetScheduledJobCount() + 2 );
        ASSERT_TRUE( after.GetMainThreadScheduledJobCount() == before.GetMainThreadScheduledJobCount() + 1 );
        CORE::CThreadPoolInfo runPoolInfo;
        ASSERT_TRUE( taskManager.GetThreadPoolInfo( settings.GetRunPoolName(), runPoolInfo ) );
        ASSERT_TRUE( 1 == runPoolInfo.GetScheduledJobCount() );
        CORE::CTaskManager::TThreadPoolInfoMap pools;
        ASSERT_TRUE( taskManager.GetAllThreadPoolInfo( pools ) );
        ASSERT_TRUE( 1 == pools[ settings.GetMainThreadPoolName() ].GetScheduledJobCount() );

        // the task of a run names its scheduler, job and run
        CORE::SJobControlCommand command;
        command.action = CORE::JOBCONTROLACTION_RUN_NOW;
        command.targetKind = CORE::JOBCONTROLTARGET_JOB;
        command.targetId = "pooled";
        CORE::SJobControlResult result;
        ASSERT_TRUE( CORE::JOBCONTROLSTATUS_OK == scheduler->Control( command, result ) );
        CORE::SJobRunRecord run;
        for ( UInt32 i=0; i<2500; ++i )
        {
            if ( scheduler->GetRun( result.runId, run ) && 0 != run.taskId )
                break;
            MT::ThreadDelay( 2 );
        }
        ASSERT_TRUE( 0 != run.taskId );
        CORE::CTaskInfo taskInfo;
        CORE::CString poolName;
        ASSERT_TRUE( taskManager.GetTaskInfo( run.taskId, taskInfo, poolName, false ) );
        ASSERT_TRUE( taskInfo.GetSchedulerName() == "tm-t4-overview" );
        ASSERT_TRUE( taskInfo.GetScheduledJobId() == "pooled" );
        ASSERT_TRUE( taskInfo.GetScheduledRunId() == result.runId );
        ASSERT_TRUE( poolName == settings.GetRunPoolName() );
        CORE::CDataNode taskNode;
        CORE::CDataNodeSerializableSettings serializerSettings;
        ASSERT_TRUE( taskInfo.Serialize( taskNode, serializerSettings ) );
        CORE::CTaskInfo taskInfoBack;
        ASSERT_TRUE( taskInfoBack.Deserialize( taskNode, serializerSettings ) && taskInfoBack.GetScheduledRunId() == result.runId );
        DropJsScheduler( scheduler );
    }
    GUCEF_TESTFW_TESTCASE_END;
    GUCEF_TESTFW_TESTCASE( "TM-T4: the overview carries the flags of a job; a hidden job is shown in the process and marked hidden" )
    {
        CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
        CORE::JobSchedulerPtr scheduler = MakeJsScheduler( MakeJsSettings( "tm-t4-hidden" ), GUCEF_NULL );
        CORE::CString error;
        CORE::CJobDefinition hidden = MakeJsJob( "hidden", CORE::CJobTrigger::Manual(), "t4hidden" );
        hidden.SetFlags( CORE::JOBFLAG_HIDDEN | CORE::JOBFLAG_SENSITIVE );
        ASSERT_TRUE( scheduler->AddJob( hidden, error ) );
        ASSERT_TRUE( scheduler->AddJob( MakeJsJob( "shown", CORE::CJobTrigger::Manual(), "t4shown" ), error ) );

        CORE::TScheduledJobInfoVector jobs;
        ASSERT_TRUE( taskManager.GetAllScheduledJobInfo( jobs ) );
        UInt32 hiddenFound = 0, shownFound = 0;
        for ( size_t i=0; i<jobs.size(); ++i )
        {
            if ( jobs[ i ].GetSchedulerName() != "tm-t4-hidden" )
                continue;
            if ( jobs[ i ].GetJobId() == "hidden" )
            {
                ++hiddenFound;
                ASSERT_TRUE( jobs[ i ].IsHidden() && ( CORE::JOBFLAG_HIDDEN | CORE::JOBFLAG_SENSITIVE ) == jobs[ i ].GetFlags() );

                // the flags survive a data node
                CORE::CDataNode node;
                CORE::CDataNodeSerializableSettings serializerSettings;
                ASSERT_TRUE( jobs[ i ].Serialize( node, serializerSettings ) );
                CORE::CScheduledJobInfo back;
                ASSERT_TRUE( back.Deserialize( node, serializerSettings ) && back.IsHidden() && back.GetFlags() == jobs[ i ].GetFlags() );
            }
            else
            {
                ++shownFound;
                ASSERT_TRUE( !jobs[ i ].IsHidden() && CORE::JOBFLAG_NONE == jobs[ i ].GetFlags() );
            }
        }
        ASSERT_TRUE( 1 == hiddenFound && 1 == shownFound );
        DropJsScheduler( scheduler );
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

static void
PerformRegistryTests( void )
{
    GUCEF_TESTFW_TESTCASE( "JS-T14: the task manager makes one scheduler per name, finds it and tells about it" )
    {
        CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
        CTmRegistryEvents events;
        taskManager.Subscribe( &events );

        CORE::CJobSchedulerSettings settings = MakeJsSettings( "js-t14-registry" );
        CORE::JobSchedulerPtr first = taskManager.GetOrCreateJobScheduler( settings, CORE::CPulseGenerator::CreateSharedObj() );
        CORE::CJobSchedulerSettings other = settings;
        other.SetMaxSleepMs( 1 );
        CORE::JobSchedulerPtr second = taskManager.GetOrCreateJobScheduler( other, CORE::CPulseGenerator::CreateSharedObj() );
        ASSERT_TRUE( !first.IsNULL() && first == second );
        ASSERT_TRUE( 900000 == second->GetSettings().GetMaxSleepMs() );
        ASSERT_TRUE( taskManager.GetJobScheduler( "js-t14-registry" ) == first );
        ASSERT_TRUE( taskManager.GetJobScheduler( "nope" ).IsNULL() );
        CORE::CString::StringSet names;
        taskManager.GetAllJobSchedulerNames( names );
        ASSERT_TRUE( names.find( "js-t14-registry" ) != names.end() );
        CORE::CTaskManager::JobSchedulerVector all;
        taskManager.GetAllJobSchedulers( all );
        ASSERT_TRUE( !all.empty() );
        CORE::CJobSchedulerSettings noName;
        ASSERT_TRUE( taskManager.GetOrCreateJobScheduler( noName, CORE::PulseGeneratorPtr() ).IsNULL() );

        CORE::CString error;
        ASSERT_TRUE( first->Start( error ) );
        ASSERT_TRUE( taskManager.UnregisterJobScheduler( "js-t14-registry" ) );
        ASSERT_FALSE( taskManager.UnregisterJobScheduler( "js-t14-registry" ) );
        ASSERT_TRUE( CORE::JOBSCHEDULERSTATE_STOPPED == first->GetHostState() );
        ASSERT_TRUE( taskManager.GetJobScheduler( "js-t14-registry" ).IsNULL() );
        first.Unlink();
        second.Unlink();

        // the events are mailed to the task manager's pulse generator
        CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        bool created = false, unregistered = false;
        for ( size_t i=0; i<events.created.size(); ++i )
            created = created || events.created[ i ] == "js-t14-registry";
        for ( size_t i=0; i<events.unregistered.size(); ++i )
            unregistered = unregistered || events.unregistered[ i ] == "js-t14-registry";
        ASSERT_TRUE( created );
        ASSERT_TRUE( unregistered );
        taskManager.Unsubscribe( &events );
    }
    GUCEF_TESTFW_TESTCASE_END;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    GUCEF_TESTFW_TESTCASE( "TM-T11: a pool task that ends while the process exits (exit handlers and static destructors) ends the process cleanly" )
    {
        char exePath[ 4096 ];
        ssize_t exePathLength = readlink( "/proc/self/exe", exePath, sizeof( exePath ) - 1 );
        ASSERT_TRUE( exePathLength > 0 );
        if ( exePathLength > 0 )
        {
            exePath[ exePathLength ] = '\0';
            CORE::CProcess::TStringVector args;
            args.push_back( GUCEF_CORE_TESTAPP_POOL_TASK_ENDS_AT_EXIT_ARG );
            CORE::CProcess child;
            child.SetIsolateFromParent( true );
            ASSERT_TRUE( child.Launch( exePath, args, CORE::CString(), TESTFW::ChildEnvironmentWithOwnExitCode() ) );
            Int32 exitCode = -1;
            const bool ended = child.WaitForExit( 60000, exitCode );
            if ( !ended )
                child.Terminate( false );
            ASSERT_TRUE( ended );
            if ( 0 != exitCode )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "TM-T11: the child exited with " << exitCode );
            ASSERT_TRUE( 0 == exitCode );
        }
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JS-T14: CTaskManager::Shutdown() stops a running scheduler, with its state saved, before its pools stop" )
    {
        char exePath[ 4096 ];
        ssize_t exePathLength = readlink( "/proc/self/exe", exePath, sizeof( exePath ) - 1 );
        ASSERT_TRUE( exePathLength > 0 );
        if ( exePathLength > 0 )
        {
            exePath[ exePathLength ] = '\0';
            CORE::CProcess::TStringVector args;
            args.push_back( GUCEF_CORE_TESTAPP_SCHEDULER_SHUTDOWN_ARG );
            CORE::CProcess child;
            child.SetIsolateFromParent( true );
            ASSERT_TRUE( child.Launch( exePath, args, CORE::CString(), TESTFW::ChildEnvironmentWithOwnExitCode() ) );
            Int32 exitCode = -1;
            const bool ended = child.WaitForExit( 120000, exitCode );
            if ( !ended )
                child.Terminate( false );
            ASSERT_TRUE( ended );
            ASSERT_TRUE( 0 == exitCode );
        }
    }
    GUCEF_TESTFW_TESTCASE_END;
    #endif
}

/*-------------------------------------------------------------------------*/

void
PerformTaskManagerCounterTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "TaskManagerCounters" );

    RegisterJsTestTaskType();
    PerformCounterTests();
    PerformOverviewTests();
    PerformRegistryTests();
    UnregisterJsTestTaskType();
}

/*-------------------------------------------------------------------------*/
