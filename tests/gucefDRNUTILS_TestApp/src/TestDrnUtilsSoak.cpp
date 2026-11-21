/*
 *  gucefDRNUTILS_TestApp: GUCEF test application for gucefDRNUTILS
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

#ifndef GUCEF_PLATFORM_H
#include "gucef_platform.h"
#define GUCEF_PLATFORM_H
#endif /* GUCEF_PLATFORM_H ? */

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
#include <dirent.h>
#define GUCEF_DRNUTILS_TEST_HAS_PROCFS
#endif

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_CJOBSCHEDULER_H
#include "gucefCORE_CJobScheduler.h"
#define GUCEF_CORE_CJOBSCHEDULER_H
#endif /* GUCEF_CORE_CJOBSCHEDULER_H ? */

#ifndef GUCEF_CORE_CIJOBWORKSOURCE_H
#include "gucefCORE_CIJobWorkSource.h"
#define GUCEF_CORE_CIJOBWORKSOURCE_H
#endif /* GUCEF_CORE_CIJOBWORKSOURCE_H ? */

#ifndef GUCEF_CORE_CIJOBWORKITEMSINK_H
#include "gucefCORE_CIJobWorkItemSink.h"
#define GUCEF_CORE_CIJOBWORKITEMSINK_H
#endif /* GUCEF_CORE_CIJOBWORKITEMSINK_H ? */

#ifndef GUCEF_DRNUTILS_H
#include "gucefDRNUTILS.h"
#define GUCEF_DRNUTILS_H
#endif /* GUCEF_DRNUTILS_H ? */

#ifndef GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSRPCWORLD_H
#include "TestDrnUtilsRpcWorld.h"
#define GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSRPCWORLD_H
#endif /* GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSRPCWORLD_H ? */

#ifndef GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSFAKESCHEDULERSOURCE_H
#include "TestDrnUtilsFakeSchedulerSource.h"
#define GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSFAKESCHEDULERSOURCE_H
#endif /* GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSFAKESCHEDULERSOURCE_H ? */

#ifndef GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSFAKETASKMANAGERSOURCE_H
#include "TestDrnUtilsFakeTaskManagerSource.h"
#define GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSFAKETASKMANAGERSOURCE_H
#endif /* GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSFAKETASKMANAGERSOURCE_H ? */

#include "TestDrnUtilsSoak.h"

using namespace GUCEF;
using namespace DRNUTILSTEST;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)
#define REQUIRE_TRUE(t)  do { if ( !(t) ) { ::GUCEF::TESTFW::CTestContext::Instance().RecordFailure( #t, __FILE__, __LINE__ ); throw std::exception(); } } while ( 0 )

namespace {

/** Resident set size in KiB, threads and open file descriptors of this process (Linux /proc; 0 elsewhere) */
struct SProcessFigures
{
    UInt64 rssKiB;
    UInt32 threads;
    UInt32 fds;
};

SProcessFigures
ReadFigures( void )
{
    SProcessFigures figures;
    figures.rssKiB = 0;
    figures.threads = 0;
    figures.fds = 0;
    #ifdef GUCEF_DRNUTILS_TEST_HAS_PROCFS
    FILE* status = fopen( "/proc/self/status", "r" );
    if ( GUCEF_NULL != status )
    {
        char line[ 256 ];
        while ( GUCEF_NULL != fgets( line, sizeof( line ), status ) )
        {
            if ( 0 == strncmp( line, "VmRSS:", 6 ) )
                figures.rssKiB = (UInt64) atol( line + 6 );
            else
            if ( 0 == strncmp( line, "Threads:", 8 ) )
                figures.threads = (UInt32) atol( line + 8 );
        }
        fclose( status );
    }
    DIR* fds = opendir( "/proc/self/fd" );
    if ( GUCEF_NULL != fds )
    {
        while ( GUCEF_NULL != readdir( fds ) )
            ++figures.fds;
        closedir( fds );
    }
    #endif /* GUCEF_DRNUTILS_TEST_HAS_PROCFS ? */
    return figures;
}

/*-------------------------------------------------------------------------*/

/** A task of one cycle, for the real scheduler */
class CDrnUtilsTestTask : public CORE::CTaskConsumer
{
    public:
    static const char* TaskType;
    CDrnUtilsTestTask( void ) : CORE::CTaskConsumer() {}
    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return TaskType; }
    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE
    {
        MT::PrecisionDelay( 50 );
        return true;
    }
    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CTaskConsumer::OnTaskEnded( task, wasForced );
    }
};
const char* CDrnUtilsTestTask::TaskType = "drnutils.test.task";
typedef CORE::CTFactory< CORE::CTaskConsumer, CDrnUtilsTestTask, MT::CMutex > TTestTaskFactory;

/*-------------------------------------------------------------------------*/

/** A work source with nothing to do, whose status the views show (GetJob with includeSourceStatus) */
class CDrnUtilsStatusWorkSource : public CORE::CIJobWorkSource
{
    public:
    CDrnUtilsStatusWorkSource( void ) : CORE::CIJobWorkSource(), statusCalls( 0 ) {}
    virtual void Evaluate( Int64 nowUtcMs, UInt64 nowMonotonicMs, CORE::CIJobWorkItemSink& dispatch, Int64& nextWakeUtcMs ) GUCEF_VIRTUAL_OVERRIDE { nextWakeUtcMs = 0; }
    virtual void OnItemFinished( const CORE::SJobWorkItem& item, const CORE::SJobRunRecord& run, Int64 nowUtcMs ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual void GetStatus( CORE::CDataNode& status, Int64 nowUtcMs, bool includeQueue ) const GUCEF_VIRTUAL_OVERRIDE
    {
        ++statusCalls;
        status.SetNodeType( GUCEF_DATATYPE_OBJECT );
        status.SetAttribute( "plannerState", "idle" );
        status.SetAttribute( "pendingSymbols", 3 );
    }
    mutable MT::CTAtomic< UInt32 > statusCalls;
};

}; /* anonymous namespace */

/*-------------------------------------------------------------------------*/

void
PerformDrnUtilsSoakTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "DrnUtilsSoak" );

    GUCEF_TESTFW_TESTCASE( "Adapters: CJobSchedulerSource over the real CJobScheduler of the task manager serves lists, RUN_NOW with waitForRun and watch changes over loopback" )
    try
    {
        CDrnUtilsRpcWorld rpc;
        CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
        TTestTaskFactory factory;
        taskManager.RegisterTaskConsumerFactory( CDrnUtilsTestTask::TaskType, &factory );
        {
            CORE::CJobSchedulerSettings settings;
            settings.SetHostName( "drnutils-real" );
            settings.SetDispatcherPoolName( "drnutils.test.dispatch" );
            settings.SetRunPoolName( "drnutils.test.run" );
            settings.SetMainThreadPoolName( "drnutils.test.main" );
            CORE::JobSchedulerPtr scheduler = taskManager.GetOrCreateJobScheduler( settings, rpc.pulseGenerator );
            REQUIRE_TRUE( !scheduler.IsNULL() );
            CORE::CJobDefinition job;
            job.SetId( "real-job" );
            job.SetTitle( "A real job" );
            job.SetTaskTypeName( CDrnUtilsTestTask::TaskType );
            job.SetTrigger( CORE::CJobTrigger::Manual() );
            CORE::TJobControlActionSet remote;
            remote.insert( CORE::JOBCONTROLACTION_RUN_NOW );
            job.SetRemoteActions( remote );
            CORE::CString error;
            REQUIRE_TRUE( scheduler->AddJob( job, error ) );
            CDrnUtilsStatusWorkSource workSource;
            CORE::CJobDefinition plannerJob;
            plannerJob.SetId( "planner-job" );
            plannerJob.SetTitle( "A planner" );
            plannerJob.SetTaskTypeName( CDrnUtilsTestTask::TaskType );
            plannerJob.SetTrigger( CORE::CJobTrigger::WorkSource() );
            REQUIRE_TRUE( scheduler->AddWorkSourceJob( plannerJob, &workSource, error ) );
            REQUIRE_TRUE( scheduler->Start( error ) );
            {
                DRNUTILS::CJobSchedulerSource source( scheduler, rpc.pulseGenerator );
                ASSERT_TRUE( "drnutils-real" == source.GetHostName() );
                DRNUTILS::CSchedulerHostSettings hostSettings;
                hostSettings.SetExposure( DRNUTILS::DRNUTILS_EXPOSURE_CONTROL );
                CORE::TJobControlActionSet allowed;
                allowed.insert( CORE::JOBCONTROLACTION_RUN_NOW );
                hostSettings.SetAllowedActions( allowed );
                DRNUTILS::CSchedulerHost host( source, hostSettings, rpc.pulseGenerator );
                REQUIRE_TRUE( host.Expose( rpc.services, error ) );
                REQUIRE_TRUE( rpc.Connect( CDrnUtilsRpcWorld::Principal( "drn:alice", "scheduler.view,scheduler.control" ), false, true ) );
                PumpFor( 300 );
                COMCORE::CRpcCallResult jobs = rpc.Call( "gucef.drnutils.scheduler.Scheduler/ListJobs", rpc.NewRequest( "gucef.drnutils.scheduler.Scheduler/ListJobs" ) );
                REQUIRE_TRUE( jobs.status.IsOk() );
                REQUIRE_TRUE( 2 == jobs.response.GetCount( 1 ) );
                const COMCORE::CRpcMessage* realRow = "real-job" == jobs.response.GetMessageAt( 1, 0 )->GetString( 1 ) ? jobs.response.GetMessageAt( 1, 0 ) : jobs.response.GetMessageAt( 1, 1 );
                ASSERT_TRUE( "real-job" == realRow->GetString( 1 ) );
                ASSERT_TRUE( CDrnUtilsTestTask::TaskType == realRow->GetString( 20 ) );

                COMCORE::CIRpcStreamPtr stream = rpc.OpenStream( "gucef.drnutils.scheduler.Scheduler/Watch", rpc.NewRequest( "gucef.drnutils.scheduler.Scheduler/Watch" ) );
                CStreamRecorder recorder;
                recorder.Watch( stream );
                CCounterAtLeast snapshot( recorder.count, 2 );
                REQUIRE_TRUE( PumpUntil( snapshot, 5000 ) );
                UInt64 before = recorder.count;

                COMCORE::CRpcMessage request = rpc.NewRequest( "gucef.drnutils.scheduler.Scheduler/Control" );
                request.SetString( 1, "real-run-1" );
                request.SetEnum( 2, CORE::JOBCONTROLACTION_RUN_NOW );
                request.SetEnum( 3, CORE::JOBCONTROLTARGET_JOB );
                request.SetString( 4, "real-job" );
                request.SetBool( 13, true );
                COMCORE::CRpcCallResult run = rpc.Call( "gucef.drnutils.scheduler.Scheduler/Control", request, COMCORE::CRpcCallOptions(), 15000 );
                if ( !run.status.IsOk() )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "real scheduler run-now: " << run.status.ToString() );
                REQUIRE_TRUE( run.status.IsOk() );
                ASSERT_TRUE( run.wasAccepted );
                REQUIRE_TRUE( GUCEF_NULL != run.response.GetMessage( 5 ) );
                ASSERT_TRUE( CORE::JOBRUNSTATE_SUCCEEDED == run.response.GetMessage( 5 )->GetEnum( 5 ) );
                ASSERT_TRUE( "drn:alice" == run.response.GetMessage( 5 )->GetString( 14 ) );
                ASSERT_TRUE( recorder.count > before );

                DRNUTILS::SSchedulerJobDetail detail;
                ASSERT_TRUE( source.GetJobDetail( "real-job", 5, false, detail ) && 1 == detail.recentRuns.size() );

                // B-3: GetJob with includeSourceStatus answers the status of the job's work source
                COMCORE::CRpcMessage getJob = rpc.NewRequest( "gucef.drnutils.scheduler.Scheduler/GetJob" );
                getJob.SetString( 1, "planner-job" );
                getJob.SetBool( 3, true );
                COMCORE::CRpcCallResult jobDetail = rpc.Call( "gucef.drnutils.scheduler.Scheduler/GetJob", getJob );
                REQUIRE_TRUE( jobDetail.status.IsOk() );
                CORE::CDataNode sourceStatus;
                ASSERT_TRUE( jobDetail.response.GetDataNode( 3, sourceStatus ) );
                ASSERT_TRUE( "idle" == sourceStatus.GetAttributeValueOrChildValueByName( "plannerState" ).AsString() );
                ASSERT_TRUE( 3 == sourceStatus.GetAttributeValueOrChildValueByName( "pendingSymbols" ).AsInt32() );
                UInt32 statusCalls = workSource.statusCalls;
                getJob.SetBool( 3, false );
                jobDetail = rpc.Call( "gucef.drnutils.scheduler.Scheduler/GetJob", getJob );
                REQUIRE_TRUE( jobDetail.status.IsOk() );
                sourceStatus.Clear();
                jobDetail.response.GetDataNode( 3, sourceStatus );
                ASSERT_TRUE( 0 == sourceStatus.GetAttCount() && 0 == sourceStatus.GetNrOfChildNodes() );
                ASSERT_TRUE( statusCalls == workSource.statusCalls );
                ASSERT_TRUE( source.GetJobDetail( "planner-job", 5, true, detail ) && "idle" == detail.sourceStatus.GetAttributeValueOrChildValueByName( "plannerState" ).AsString() );
                ASSERT_TRUE( source.GetJobDetail( "real-job", 5, true, detail ) && 0 == detail.sourceStatus.GetAttCount() && 0 == detail.sourceStatus.GetNrOfChildNodes() );
                DRNUTILS::TSchedulerActionInfoVector actions;
                source.GetDeclaredActions( actions );
                ASSERT_TRUE( actions.size() >= 6 );
                stream->Cancel( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_CANCELLED, "done" ) );
                rpc.Disconnect();
            }
            scheduler->Stop();
            taskManager.UnregisterJobScheduler( "drnutils-real" );
        }
        taskManager.UnregisterTaskConsumerFactory( CDrnUtilsTestTask::TaskType );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Adapters: CTaskManagerSource lists the pools, threads and tasks of the process task manager and resizes a pool" )
    try
    {
        CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
        CORE::ThreadPoolPtr pool = taskManager.GetOrCreateThreadPool( "drnutils.test.resize", true );
        REQUIRE_TRUE( !pool.IsNULL() );
        DRNUTILS::CTaskManagerSource source( "this-node" );
        ASSERT_TRUE( 0 != source.GetHostInstanceId() );
        ASSERT_TRUE( source.GetUtcTimeInMs() > 1767225600000LL );
        CORE::CTaskManager::TThreadPoolInfoMap pools;
        ASSERT_TRUE( source.GetAllThreadPoolInfo( pools ) && pools.find( "drnutils.test.resize" ) != pools.end() );
        CORE::CTaskManager::TThreadPoolCounterMap counters;
        ASSERT_TRUE( source.GetPoolCounters( counters ) );
        CORE::CTaskManager::TThreadInfoMap threads;
        ASSERT_TRUE( source.GetAllThreadInfo( threads ) );
        CORE::CTaskManager::TTaskInfoMap tasks;
        ASSERT_TRUE( source.GetAllTaskInfo( tasks ) );
        CORE::CString error;
        ASSERT_TRUE( source.ResizePool( "drnutils.test.resize", 1, 3, error ) );
        ASSERT_FALSE( source.ResizePool( "no-such-pool", 1, 3, error ) );
        ASSERT_FALSE( source.CancelTask( 999999, error ) );
        CORE::CString poolName, taskType;
        ASSERT_FALSE( source.GetTaskPoolAndType( 999999, poolName, taskType ) );
        pool->SetDesiredMinNrOfWorkerThreads( 0 );
        pool->SetDesiredMaxTotalNrOfThreads( 0 );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RU-T14 soak: viewer churn, watch streams, viewing mode and command load keep memory, threads and file descriptors bounded (GUCEF_TEST_SOAK_MINUTES for a long run)" )
    try
    {
        UInt32 minutes = 0;
        const char* soakEnv = getenv( "GUCEF_TEST_SOAK_MINUTES" );
        if ( GUCEF_NULL != soakEnv )
            minutes = (UInt32) atoi( soakEnv );
        UInt64 durationMs = 0 == minutes ? 20000 : (UInt64) minutes * 60000;
        // which parts of a round run (bit mask, for finding where memory goes): 1 RPC sessions, 2 scheduler watch,
        // 4 task manager watch (viewing mode and the sampler), 8 commands and changes; default all
        UInt32 parts = 15;
        const char* partsEnv = getenv( "GUCEF_DRNUTILS_SOAK_PARTS" );
        if ( GUCEF_NULL != partsEnv )
            parts = (UInt32) atoi( partsEnv );

        CDrnUtilsRpcWorld rpc;
        CFakeSchedulerSource source( rpc.pulseGenerator );
        for ( UInt32 i=0; i<50; ++i )
            source.AddJob( "job" + CORE::ToString( i ) );
        DRNUTILS::CSchedulerHostSettings settings;
        settings.SetExposure( DRNUTILS::DRNUTILS_EXPOSURE_CONTROL );
        CORE::TJobControlActionSet allowed;
        allowed.insert( CORE::JOBCONTROLACTION_ENABLE );
        allowed.insert( CORE::JOBCONTROLACTION_RUN_NOW );
        settings.SetAllowedActions( allowed );
        settings.SetMaxControlCommandsPerMinutePerSubject( 0 );
        settings.SetMaxCommands( 256 );
        settings.SetMaxCommandsPerSubject( 64 );
        settings.SetJournalSize( 512 );
        DRNUTILS::CSchedulerHost host( source, settings, rpc.pulseGenerator );
        class CNullSink : public DRNUTILS::CIDrnUtilsAuditSink { public: UInt64 n; CNullSink() : n( 0 ) {} virtual void OnAuditRecord( const DRNUTILS::SDrnUtilsAuditRecord& ) GUCEF_VIRTUAL_OVERRIDE { ++n; } };
        CNullSink sink;
        host.SetAuditSink( &sink );
        CFakeTaskManagerSource tasks;
        tasks.AddPool( "work", 2 );
        tasks.AddTask( 1, "work", "t", 9 );
        DRNUTILS::CTaskManagerHostSettings taskSettings;
        taskSettings.SetExposure( DRNUTILS::DRNUTILS_EXPOSURE_VIEW );
        taskSettings.SetViewingGraceMs( 100 );
        DRNUTILS::CTaskManagerHost taskHost( tasks, taskSettings, rpc.pulseGenerator );
        CORE::CString error;
        REQUIRE_TRUE( host.Expose( rpc.services, error ) && taskHost.Expose( rpc.services, error ) );

        SProcessFigures baseline;
        SProcessFigures afterWarmUp;
        UInt64 start = MT::PrecisionTickCount();
        UInt64 ticksPerMs = MT::PrecisionTimerResolution() / 1000;
        UInt32 round = 0;
        UInt64 commands = 0;
        while ( ( MT::PrecisionTickCount() - start ) / ticksPerMs < durationMs )
        {
            COMCORE::CIRpcStreamPtr watch;
            COMCORE::CIRpcStreamPtr view;
            CStreamRecorder recorder;
            if ( 0 != ( parts & 1 ) )
            {
                REQUIRE_TRUE( rpc.Connect( CDrnUtilsRpcWorld::Principal( "drn:soak" + CORE::ToString( round % 7 ), "scheduler.view,scheduler.control,tasks.view" ), true, true ) );
                if ( 0 != ( parts & 2 ) )
                {
                    watch = rpc.OpenStream( "gucef.drnutils.scheduler.Scheduler/Watch", rpc.NewRequest( "gucef.drnutils.scheduler.Scheduler/Watch" ) );
                    recorder.Watch( watch );
                }
                if ( 0 != ( parts & 4 ) )
                {
                    COMCORE::CRpcMessage viewRequest = rpc.NewRequest( "gucef.drnutils.tasks.TaskManager/Watch" );
                    viewRequest.SetBool( 3, true );
                    view = rpc.OpenStream( "gucef.drnutils.tasks.TaskManager/Watch", viewRequest );
                }
            }
            for ( UInt32 c=0; c<100 && 0 != ( parts & 8 ); ++c )
            {
                CORE::SJobControlCommand command;
                command.action = CORE::JOBCONTROLACTION_ENABLE;
                command.targetKind = CORE::JOBCONTROLTARGET_JOB;
                command.targetId = "job" + CORE::ToString( c % 50 );
                CORE::SJobControlResult result;
                bool replayed = false;
                COMCORE::CRpcPrincipal principal( "local", "local:soak" + CORE::ToString( c % 13 ), true );
                principal.roles.insert( "scheduler.control" );
                host.Control( principal, "cmd-" + CORE::ToString( commands++ ), command, result, replayed );
                source.Touch( "job" + CORE::ToString( c % 50 ), 0 == c % 10 );
            }
            PumpFor( 150 );
            recorder.messages.clear();
            if ( !watch.IsNULL() )
                watch->Cancel( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_CANCELLED, "churn" ) );
            if ( !view.IsNULL() )
                view->Cancel( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_CANCELLED, "churn" ) );
            watch.Unlink();
            view.Unlink();
            PumpFor( 200 );
            if ( 0 != ( parts & 1 ) )
                rpc.Disconnect();
            ++round;
            if ( 3 == round )
                baseline = ReadFigures();
            if ( 10 == round )
                afterWarmUp = ReadFigures();
        }
        PumpFor( 300 );
        SProcessFigures end = ReadFigures();
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RU-T14 soak: " << round << " rounds, " << commands << " commands, audit " << sink.n << "; RSS KiB " << baseline.rssKiB << " / " << afterWarmUp.rssKiB << " / " << end.rssKiB
                   << ", threads " << baseline.threads << " / " << end.threads << ", fds " << baseline.fds << " / " << end.fds );
        REQUIRE_TRUE( round >= 12 );
        ASSERT_TRUE( commands == sink.n );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RU-T14 soak parts " << parts << ": RSS growth after warm-up " << ( (Int64) end.rssKiB - (Int64) afterWarmUp.rssKiB ) << " KiB over " << ( round - 10 ) << " rounds" );
        // after the warm-up rounds the figures stay put: no growth with the rounds (bounded caches, journal, streams)
        #if defined( __SANITIZE_ADDRESS__ )
        // AddressSanitizer keeps freed memory in quarantine: the resident size says nothing about leaks there (LeakSanitizer does)
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RU-T14 soak: the resident size is not checked under AddressSanitizer" );
        #else
        ASSERT_TRUE( end.rssKiB <= afterWarmUp.rssKiB + 4096 );
        #endif
        ASSERT_TRUE( end.threads <= baseline.threads + 1 );
        ASSERT_TRUE( end.fds <= baseline.fds + 2 );
        ASSERT_FALSE( taskHost.IsInViewingMode() );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/
