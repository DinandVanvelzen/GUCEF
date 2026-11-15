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

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_DRNUTILS_H
#include "gucefDRNUTILS.h"
#define GUCEF_DRNUTILS_H
#endif /* GUCEF_DRNUTILS_H ? */

#ifndef GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSRPCWORLD_H
#include "TestDrnUtilsRpcWorld.h"
#define GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSRPCWORLD_H
#endif /* GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSRPCWORLD_H ? */

#ifndef GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSFAKETASKMANAGERSOURCE_H
#include "TestDrnUtilsFakeTaskManagerSource.h"
#define GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSFAKETASKMANAGERSOURCE_H
#endif /* GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSFAKETASKMANAGERSOURCE_H ? */

#include "TestDrnUtilsTaskManagerHost.h"

using namespace GUCEF;
using namespace DRNUTILSTEST;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)
#define REQUIRE_TRUE(t)  do { if ( !(t) ) { ::GUCEF::TESTFW::CTestContext::Instance().RecordFailure( #t, __FILE__, __LINE__ ); throw std::exception(); } } while ( 0 )

namespace {

const char* T_GETINFO = "gucef.drnutils.tasks.TaskManager/GetInfo";
const char* T_LISTPOOLS = "gucef.drnutils.tasks.TaskManager/ListPools";
const char* T_LISTTHREADS = "gucef.drnutils.tasks.TaskManager/ListThreads";
const char* T_LISTTASKS = "gucef.drnutils.tasks.TaskManager/ListTasks";
const char* T_WATCH = "gucef.drnutils.tasks.TaskManager/Watch";
const char* T_CONTROL = "gucef.drnutils.tasks.TaskManager/Control";
const char* T_LISTJOBS = "gucef.drnutils.tasks.TaskManager/ListScheduledJobs";

/*-------------------------------------------------------------------------*/

class CTaskAuditSink : public DRNUTILS::CIDrnUtilsAuditSink
{
    public:
    GUCEF::vector< DRNUTILS::SDrnUtilsAuditRecord > records;
    virtual void OnAuditRecord( const DRNUTILS::SDrnUtilsAuditRecord& record ) GUCEF_VIRTUAL_OVERRIDE { records.push_back( record ); }
};

/*-------------------------------------------------------------------------*/

class CTaskWorld
{
    public:
    CDrnUtilsRpcWorld rpc;
    CFakeTaskManagerSource source;
    DRNUTILS::CTaskManagerHostSettings settings;
    DRNUTILS::CTaskManagerHost* host;
    CTaskAuditSink audit;

    CTaskWorld( void ) : rpc(), source(), settings(), host( GUCEF_NULL ), audit()
    {
        settings.SetExposure( DRNUTILS::DRNUTILS_EXPOSURE_CONTROL );
        CORE::CString::StringSet controllable;
        controllable.insert( "work" );
        controllable.insert( "gucef.rpc" );
        settings.SetControllablePools( controllable );
        CORE::CString::StringSet cancellable;
        cancellable.insert( "cancellable.type" );
        settings.SetCancellableTaskTypes( cancellable );
        settings.SetMinAllowedWorkers( 1 );
        settings.SetMaxAllowedThreads( 8 );
        settings.SetSampleIntervalMs( 1000 );
        settings.SetTaskSampleIntervalMs( 1000 );
        settings.SetViewingGraceMs( 300 );
        source.AddPool( "work", 2 );
        source.AddPool( "gucef.rpc", 1 );
        source.AddPool( "other", 1 );
        source.AddTask( 11, "work", "cancellable.type", 501 );
        source.AddTask( 12, "work", "other.type", 502 );
        source.AddTask( 13, "other", "cancellable.type", 503 );
        CORE::CScheduledJobInfo job;
        job.SetSchedulerName( "node1" );
        job.SetJobId( "market-data" );
        job.SetTitle( "Market data" );
        job.SetExecutionTarget( CORE::JOBEXECUTIONTARGET_MAIN_THREAD );
        job.SetState( CORE::JOBSTATE_SCHEDULED );
        source.scheduledJobs.push_back( job );
    }

    ~CTaskWorld()
    {
        rpc.Disconnect();
        GUCEF_DELETE host;
    }

    bool Start( void )
    {
        host = GUCEF_NEW DRNUTILS::CTaskManagerHost( source, settings, rpc.pulseGenerator );
        host->SetAuditSink( &audit );
        CORE::CString error;
        return host->Expose( rpc.services, error );
    }

    bool Connect( const char* roles, const CORE::CString& transport = "loopback", const CORE::CString& subject = "drn:alice" )
    {
        return rpc.Connect( CDrnUtilsRpcWorld::Principal( subject, roles, transport ), true, false );
    }

    COMCORE::CRpcCallResult Control( const CORE::CString& commandId, DRNUTILS::TDrnUtilsTaskControlAction action, const CORE::CString& pool,
                                     UInt32 taskId, UInt32 minWorkers = 0, Int32 maxThreads = 0, bool dryRun = false )
    {
        COMCORE::CRpcMessage request = rpc.NewRequest( T_CONTROL );
        request.SetString( 1, commandId );
        request.SetEnum( 2, (Int32) action );
        request.SetString( 3, pool );
        request.SetUInt32( 4, taskId );
        request.SetUInt32( 5, minWorkers );
        request.SetInt32( 6, maxThreads );
        request.SetBool( 8, dryRun );
        return rpc.Call( T_CONTROL, request );
    }
};

}; /* anonymous namespace */

/*-------------------------------------------------------------------------*/

void
PerformDrnUtilsTaskManagerHostTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "DrnUtilsTaskManagerHost" );

    GUCEF_TESTFW_TESTCASE( "RU-T9 pools, threads, tasks and scheduled jobs over loopback; rates from the counter deltas; capabilities and controllable pools per viewer" )
    try
    {
        CTaskWorld w;
        REQUIRE_TRUE( w.Start() );
        REQUIRE_TRUE( w.Connect( "tasks.view" ) );
        COMCORE::CRpcCallResult info = w.rpc.Call( T_GETINFO, w.rpc.NewRequest( T_GETINFO ) );
        REQUIRE_TRUE( info.status.IsOk() );
        ASSERT_TRUE( "node1" == info.response.GetString( 1 ) && 77 == info.response.GetUInt64( 2 ) );
        ASSERT_TRUE( 3 == info.response.GetCount( 4 ) );     // taskTimes, threadNames, scheduledJobs: no control capabilities for a viewer
        ASSERT_TRUE( 0 == info.response.GetCount( 5 ) );

        COMCORE::CRpcCallResult pools = w.rpc.Call( T_LISTPOOLS, w.rpc.NewRequest( T_LISTPOOLS ) );
        REQUIRE_TRUE( pools.status.IsOk() && 3 == pools.response.GetCount( 1 ) );
        w.source.AddFinished( "work", 20, 2000 );
        PumpFor( 1100 );
        pools = w.rpc.Call( T_LISTPOOLS, w.rpc.NewRequest( T_LISTPOOLS ) );
        REQUIRE_TRUE( pools.status.IsOk() );
        bool rated = false;
        for ( UInt32 p=0; p<pools.response.GetCount( 1 ); ++p )
        {
            const COMCORE::CRpcMessage* row = pools.response.GetMessageAt( 1, p );
            if ( "work" == row->GetString( 1 ) )
            {
                rated = row->GetFloat64( 9 ) > 5.0 && row->GetFloat64( 9 ) < 25.0 && 20 == row->GetUInt64( 16 ) && 100.0 == row->GetFloat64( 13 );
                ASSERT_TRUE( 2 == row->GetUInt32( 2 ) && 4 == row->GetInt32( 4 ) );
            }
        }
        ASSERT_TRUE( rated );

        COMCORE::CRpcMessage threads = w.rpc.NewRequest( T_LISTTHREADS );
        threads.SetString( 1, "work" );
        COMCORE::CRpcCallResult threadList = w.rpc.Call( T_LISTTHREADS, threads );
        ASSERT_TRUE( threadList.status.IsOk() && 2 == threadList.response.GetCount( 1 ) );
        ASSERT_TRUE( "work#1" == threadList.response.GetMessageAt( 1, 0 )->GetString( 2 ) );

        COMCORE::CRpcMessage tasks = w.rpc.NewRequest( T_LISTTASKS );
        tasks.SetString( 1, "work" );
        COMCORE::CRpcCallResult taskList = w.rpc.Call( T_LISTTASKS, tasks );
        REQUIRE_TRUE( taskList.status.IsOk() && 2 == taskList.response.GetCount( 1 ) );
        ASSERT_FALSE( taskList.response.GetBool( 2 ) );
        tasks.SetString( 1, "" );
        tasks.SetUInt32( 4, 2 );
        taskList = w.rpc.Call( T_LISTTASKS, tasks );
        ASSERT_TRUE( taskList.status.IsOk() && 2 == taskList.response.GetCount( 1 ) && taskList.response.GetBool( 2 ) );

        COMCORE::CRpcCallResult jobs = w.rpc.Call( T_LISTJOBS, w.rpc.NewRequest( T_LISTJOBS ) );
        REQUIRE_TRUE( jobs.status.IsOk() && 1 == jobs.response.GetCount( 1 ) );
        ASSERT_TRUE( "node1/market-data" == jobs.response.GetMessageAt( 1, 0 )->GetString( 1 ) );
        ASSERT_TRUE( CORE::JOBEXECUTIONTARGET_MAIN_THREAD == jobs.response.GetMessageAt( 1, 0 )->GetEnum( 6 ) );

        // without tasks.view nothing; with tasks.control the capabilities and the controllable pools (never a protected one)
        REQUIRE_TRUE( w.Connect( "nobody" ) );
        ASSERT_TRUE( COMCORE::RPC_STATUS_PERMISSION_DENIED == w.rpc.Call( T_LISTPOOLS, w.rpc.NewRequest( T_LISTPOOLS ) ).status.code );
        REQUIRE_TRUE( w.Connect( "tasks.view,tasks.control" ) );
        info = w.rpc.Call( T_GETINFO, w.rpc.NewRequest( T_GETINFO ) );
        REQUIRE_TRUE( info.status.IsOk() );
        ASSERT_TRUE( 6 == info.response.GetCount( 4 ) );
        REQUIRE_TRUE( 1 == info.response.GetCount( 5 ) );
        CORE::CVariant pool;
        info.response.GetAt( 5, 0, pool );
        ASSERT_TRUE( "work" == pool.AsString() );
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

    GUCEF_TESTFW_TESTCASE( "RU-T9 control: resize within bounds, refused outside them and on protected pools; cancel and stop of allowed task types only; roles, transport, idempotency, audit, dry run" )
    try
    {
        CTaskWorld w;
        REQUIRE_TRUE( w.Start() );
        REQUIRE_TRUE( w.Connect( "tasks.view,tasks.control" ) );
        ASSERT_TRUE( w.Control( "c1", DRNUTILS::DRNUTILS_TASKACTION_RESIZE_POOL, "work", 0, 2, 6 ).status.IsOk() );
        REQUIRE_TRUE( 1 == w.source.resizes.size() );
        ASSERT_TRUE( "work" == w.source.resizes[ 0 ].pool && 2 == w.source.resizes[ 0 ].minWorkers && 6 == w.source.resizes[ 0 ].maxThreads );
        ASSERT_TRUE( COMCORE::RPC_STATUS_INVALID_ARGUMENT == w.Control( "c2", DRNUTILS::DRNUTILS_TASKACTION_RESIZE_POOL, "work", 0, 0, 6 ).status.code );
        ASSERT_TRUE( COMCORE::RPC_STATUS_INVALID_ARGUMENT == w.Control( "c3", DRNUTILS::DRNUTILS_TASKACTION_RESIZE_POOL, "work", 0, 2, 9 ).status.code );
        ASSERT_TRUE( COMCORE::RPC_STATUS_INVALID_ARGUMENT == w.Control( "c4", DRNUTILS::DRNUTILS_TASKACTION_RESIZE_POOL, "work", 0, 2, -1 ).status.code );
        ASSERT_TRUE( COMCORE::RPC_STATUS_PERMISSION_DENIED == w.Control( "c5", DRNUTILS::DRNUTILS_TASKACTION_RESIZE_POOL, "gucef.rpc", 0, 1, 2 ).status.code );
        ASSERT_TRUE( COMCORE::RPC_STATUS_PERMISSION_DENIED == w.Control( "c6", DRNUTILS::DRNUTILS_TASKACTION_RESIZE_POOL, "other", 0, 1, 2 ).status.code );
        ASSERT_TRUE( COMCORE::RPC_STATUS_PERMISSION_DENIED == w.Control( "c7", DRNUTILS::DRNUTILS_TASKACTION_RESIZE_POOL, "gucef.drnutils.sampler", 0, 1, 2 ).status.code );
        ASSERT_TRUE( 1 == w.source.resizes.size() );

        ASSERT_TRUE( w.Control( "c8", DRNUTILS::DRNUTILS_TASKACTION_CANCEL_TASK, "", 11 ).status.IsOk() );
        ASSERT_TRUE( 1 == w.source.cancelled.size() && 11 == w.source.cancelled[ 0 ] );
        ASSERT_TRUE( COMCORE::RPC_STATUS_PERMISSION_DENIED == w.Control( "c9", DRNUTILS::DRNUTILS_TASKACTION_CANCEL_TASK, "", 12 ).status.code );
        ASSERT_TRUE( COMCORE::RPC_STATUS_PERMISSION_DENIED == w.Control( "c10", DRNUTILS::DRNUTILS_TASKACTION_CANCEL_TASK, "", 13 ).status.code );
        ASSERT_TRUE( COMCORE::RPC_STATUS_NOT_FOUND == w.Control( "c11", DRNUTILS::DRNUTILS_TASKACTION_STOP_TASK, "", 99 ).status.code );
        ASSERT_TRUE( w.Control( "c12", DRNUTILS::DRNUTILS_TASKACTION_STOP_TASK, "", 11 ).status.IsOk() );
        ASSERT_TRUE( 1 == w.source.stopped.size() );
        ASSERT_TRUE( COMCORE::RPC_STATUS_UNIMPLEMENTED == w.Control( "c13", (DRNUTILS::TDrnUtilsTaskControlAction) 4, "work", 11 ).status.code );

        // replay: the stored outcome, nothing applied twice
        COMCORE::CRpcCallResult replay = w.Control( "c8", DRNUTILS::DRNUTILS_TASKACTION_CANCEL_TASK, "", 11 );
        ASSERT_TRUE( replay.status.IsOk() && replay.response.GetBool( 1 ) );
        ASSERT_TRUE( 1 == w.source.cancelled.size() );
        // dry run: checked, not applied
        ASSERT_TRUE( w.Control( "", DRNUTILS::DRNUTILS_TASKACTION_RESIZE_POOL, "work", 0, 3, 6, true ).status.IsOk() );
        ASSERT_TRUE( 1 == w.source.resizes.size() );

        // audit: every command, the resize arguments as digest
        REQUIRE_TRUE( w.audit.records.size() >= 14 );
        ASSERT_TRUE( "gucef.drnutils.tasks" == w.audit.records[ 0 ].contract && "RESIZE_POOL" == w.audit.records[ 0 ].action );
        ASSERT_TRUE( "POOL" == w.audit.records[ 0 ].targetKind && "work" == w.audit.records[ 0 ].targetId && 64 == w.audit.records[ 0 ].argumentsDigest.Length() );
        ASSERT_TRUE( "cancellable.type" == w.audit.records[ 7 ].taskTypeName );

        // a viewer cannot, an insecure transport cannot
        REQUIRE_TRUE( w.Connect( "tasks.view" ) );
        ASSERT_TRUE( COMCORE::RPC_STATUS_PERMISSION_DENIED == w.Control( "v1", DRNUTILS::DRNUTILS_TASKACTION_CANCEL_TASK, "", 11 ).status.code );
        REQUIRE_TRUE( w.Connect( "tasks.view,tasks.control", "tcp" ) );
        ASSERT_TRUE( COMCORE::RPC_STATUS_PERMISSION_DENIED == w.Control( "t1", DRNUTILS::DRNUTILS_TASKACTION_CANCEL_TASK, "", 11 ).status.code );
        ASSERT_TRUE( 1 == w.source.cancelled.size() );
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

    GUCEF_TESTFW_TESTCASE( "RU-T4 hidden jobs never leave the process: not in ListScheduledJobs nor in Watch samples, a task of a hidden job's run is listed without scheduler, job and run (B-4)" )
    try
    {
        CTaskWorld w;
        CORE::CScheduledJobInfo hidden;
        hidden.SetSchedulerName( "node1" );
        hidden.SetJobId( "secret-job" );
        hidden.SetTitle( "Secret" );
        hidden.SetFlags( CORE::JOBFLAG_HIDDEN );
        w.source.scheduledJobs.push_back( hidden );
        w.source.AddTask( 21, "work", "other.type", 521 );
        w.source.AddTask( 22, "work", "other.type", 522 );
        {
            MT::CScopeMutex scope( w.source.lock );
            w.source.tasks[ 21 ].SetSchedulerName( "node1" );
            w.source.tasks[ 21 ].SetScheduledJobId( "secret-job" );
            w.source.tasks[ 21 ].SetScheduledRunId( 7 );
            w.source.tasks[ 22 ].SetSchedulerName( "node1" );
            w.source.tasks[ 22 ].SetScheduledJobId( "market-data" );
            w.source.tasks[ 22 ].SetScheduledRunId( 8 );
        }
        REQUIRE_TRUE( w.Start() );
        REQUIRE_TRUE( w.Connect( "tasks.view" ) );

        COMCORE::CRpcCallResult jobs = w.rpc.Call( T_LISTJOBS, w.rpc.NewRequest( T_LISTJOBS ) );
        REQUIRE_TRUE( jobs.status.IsOk() );
        ASSERT_TRUE( 1 == jobs.response.GetCount( 1 ) );
        for ( UInt32 j=0; j<jobs.response.GetCount( 1 ); ++j )
            ASSERT_TRUE( "secret-job" != jobs.response.GetMessageAt( 1, j )->GetString( 3 ) );

        COMCORE::CRpcMessage tasks = w.rpc.NewRequest( T_LISTTASKS );
        tasks.SetString( 1, "work" );
        COMCORE::CRpcCallResult taskList = w.rpc.Call( T_LISTTASKS, tasks );
        REQUIRE_TRUE( taskList.status.IsOk() );
        UInt32 seen = 0;
        for ( UInt32 t=0; t<taskList.response.GetCount( 1 ); ++t )
        {
            const COMCORE::CRpcMessage* row = taskList.response.GetMessageAt( 1, t );
            if ( 21 == row->GetUInt32( 1 ) )
            {
                ++seen;
                ASSERT_TRUE( row->GetString( 17 ).IsNULLOrEmpty() && row->GetString( 18 ).IsNULLOrEmpty() && 0 == row->GetUInt64( 19 ) );
            }
            if ( 22 == row->GetUInt32( 1 ) )
            {
                ++seen;
                ASSERT_TRUE( "node1" == row->GetString( 17 ) && "market-data" == row->GetString( 18 ) && 8 == row->GetUInt64( 19 ) );
            }
        }
        ASSERT_TRUE( 2 == seen );

        COMCORE::CRpcMessage request = w.rpc.NewRequest( T_WATCH );
        request.SetUInt32( 1, 1000 );
        request.SetBool( 3, true );
        request.SetBool( 4, true );
        COMCORE::CIRpcStreamPtr stream = w.rpc.OpenStream( T_WATCH, request );
        REQUIRE_TRUE( !stream.IsNULL() );
        CStreamRecorder recorder;
        recorder.Watch( stream );
        CCounterAtLeast first( recorder.count, 1 );
        REQUIRE_TRUE( PumpUntil( first, 5000 ) );
        const COMCORE::CRpcMessage& sample = recorder.messages.back();
        ASSERT_TRUE( 1 == sample.GetCount( 6 ) && "node1/market-data" == sample.GetMessageAt( 6, 0 )->GetString( 1 ) );
        for ( UInt32 t=0; t<sample.GetCount( 4 ); ++t )
            ASSERT_TRUE( "secret-job" != sample.GetMessageAt( 4, t )->GetString( 18 ) );
        stream->Cancel( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_CANCELLED, "viewer closed" ) );
        PumpFor( 100 );
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

    GUCEF_TESTFW_TESTCASE( "Watch: a viewer without credit holds one sample at the host; the latest sample wins, older ones are replaced, and it arrives as soon as the viewer grants credit" )
    try
    {
        CTaskWorld w;
        w.settings.SetMinSampleIntervalMs( 50 );
        w.settings.SetSampleIntervalMs( 50 );
        REQUIRE_TRUE( w.Start() );
        REQUIRE_TRUE( w.Connect( "tasks.view" ) );

        COMCORE::CRpcMessage request = w.rpc.NewRequest( T_WATCH );
        request.SetUInt32( 1, 50 );
        COMCORE::CIRpcStreamPtr stream = w.rpc.OpenStream( T_WATCH, request );
        REQUIRE_TRUE( !stream.IsNULL() );
        stream->SetManualCredit( true );
        CStreamRecorder recorder;
        recorder.Watch( stream );

        // the initial credit of Watch (16) is used up, then the host keeps the latest sample only
        CCounterAtLeast initial( recorder.count, 16 );
        REQUIRE_TRUE( PumpUntil( initial, 10000 ) );
        PumpFor( 1200 );
        ASSERT_TRUE( 16 == recorder.count );
        UInt32 streams = 0;
        UInt64 written = 0;
        UInt64 replaced = 0;
        w.host->GetWatchStatistics( streams, written, replaced );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Watch latest sample: " << written << " written, " << replaced << " replaced while the viewer had no credit" );
        ASSERT_TRUE( 1 == streams && 16 == written );
        ASSERT_TRUE( replaced >= 5 );
        Int64 lastBefore = recorder.messages.back().GetTimestamp( 1 ) / 1000;

        // one message of credit: the newest sample comes, not the oldest held back
        UInt64 grantedAtUtcMs = CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
        stream->GrantCredit( 1 );
        CCounterAtLeast one( recorder.count, 17 );
        REQUIRE_TRUE( PumpUntil( one, 5000 ) );
        PumpFor( 300 );
        ASSERT_TRUE( 17 == recorder.count );
        Int64 latest = recorder.messages.back().GetTimestamp( 1 ) / 1000;
        ASSERT_TRUE( latest - lastBefore >= 1000 );
        ASSERT_TRUE( latest + 500 >= (Int64) grantedAtUtcMs );
        w.host->GetWatchStatistics( streams, written, replaced );
        ASSERT_TRUE( 17 == written );

        stream->Cancel( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_CANCELLED, "viewer closed" ) );
        PumpFor( 100 );
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

    GUCEF_TESTFW_TESTCASE( "Viewing mode: the sampler is idle while nobody watches, samples at most once per second on its own pool while a Watch is open, and stops after the grace period" )
    try
    {
        CTaskWorld w;
        REQUIRE_TRUE( w.Start() );
        REQUIRE_TRUE( w.Connect( "tasks.view" ) );
        PumpFor( 1500 );
        ASSERT_TRUE( 0 == w.source.GetCounterCalls() );
        ASSERT_FALSE( w.host->IsInViewingMode() );

        COMCORE::CRpcMessage request = w.rpc.NewRequest( T_WATCH );
        request.SetUInt32( 1, 200 );      // clamped to 1000
        request.SetBool( 2, true );
        request.SetBool( 3, true );
        request.SetBool( 4, true );
        COMCORE::CIRpcStreamPtr stream = w.rpc.OpenStream( T_WATCH, request );
        REQUIRE_TRUE( !stream.IsNULL() );
        CStreamRecorder recorder;
        recorder.Watch( stream );
        PumpFor( 2600 );
        ASSERT_TRUE( w.host->IsInViewingMode() );
        UInt32 calls = w.source.GetCounterCalls();
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Viewing mode: " << calls << " samples and " << recorder.count << " messages in 2.6 s" );
        ASSERT_TRUE( calls >= 2 && calls <= 4 );
        ASSERT_TRUE( recorder.count >= 2 && recorder.count <= 4 );
        REQUIRE_TRUE( !recorder.messages.empty() );
        const COMCORE::CRpcMessage& sample = recorder.messages.back();
        ASSERT_TRUE( 3 == sample.GetCount( 2 ) && 3 == sample.GetCount( 3 ) && sample.GetCount( 4 ) >= 3 && 1 == sample.GetCount( 6 ) );

        // a task that went away shows as ended (the last ended ones seen)
        w.source.RemoveTask( 12 );
        PumpFor( 1200 );
        bool sawEnded = false;
        const COMCORE::CRpcMessage& later = recorder.messages.back();
        for ( UInt32 t=0; t<later.GetCount( 4 ); ++t )
            if ( 12 == later.GetMessageAt( 4, t )->GetUInt32( 1 ) && "ENDED" == later.GetMessageAt( 4, t )->GetString( 4 ) )
                sawEnded = true;
        ASSERT_TRUE( sawEnded );

        stream->Cancel( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_CANCELLED, "viewer closed" ) );
        PumpFor( 150 );
        ASSERT_TRUE( w.host->IsInViewingMode() );      // within the grace period
        PumpFor( 400 );
        ASSERT_FALSE( w.host->IsInViewingMode() );
        calls = w.source.GetCounterCalls();
        PumpFor( 2000 );
        ASSERT_TRUE( calls == w.source.GetCounterCalls() );
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

    GUCEF_TESTFW_TESTCASE( "Client facade: CTaskManagerClient info, a resize command, and viewing mode through StartViewing / StopViewing" )
    try
    {
        CTaskWorld w;
        REQUIRE_TRUE( w.Start() );
        REQUIRE_TRUE( w.Connect( "tasks.view,tasks.control" ) );
        DRNUTILS::CTaskManagerClient client( w.rpc.pulseGenerator );
        class CRecorder : public CORE::CObserver
        {
            public:
            UInt64 infos;
            UInt64 commands;
            UInt64 viewing;
            DRNUTILS::SDrnUtilsCommandOutcome last;
            DRNUTILS::STaskManagerHostInfo lastInfo;
            CRecorder( void ) : infos( 0 ), commands( 0 ), viewing( 0 ), last(), lastInfo() {}
            virtual ~CRecorder() { SignalUpcomingObserverDestruction(); }
            virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
            {
                if ( DRNUTILS::CTaskManagerClient::InfoReceivedEvent == eventid )
                {
                    ++infos;
                    lastInfo = static_cast< DRNUTILS::CTaskManagerClient::TInfoReceivedEventData* >( eventdata )->GetData();
                }
                if ( DRNUTILS::CTaskManagerClient::ViewingChangedEvent == eventid ) ++viewing;
                if ( DRNUTILS::CTaskManagerClient::CommandCompletedEvent == eventid )
                {
                    ++commands;
                    last = static_cast< DRNUTILS::CTaskManagerClient::TCommandCompletedEventData* >( eventdata )->GetData();
                }
            }
        };
        CRecorder recorder;
        client.Subscribe( &recorder );
        // C-3: StopViewing() without a viewing is safe and changes nothing
        client.StopViewing();
        ASSERT_FALSE( client.IsViewing() );
        ASSERT_TRUE( 0 == recorder.viewing );
        client.SetEndpoint( w.rpc.client );
        ASSERT_TRUE( client.IsConnected() );
        // C-3: valid right after SetEndpoint (nothing asked yet: the empty info)
        ASSERT_TRUE( client.GetLastInfo().hostName.IsNULLOrEmpty() && client.GetLastInfo().capabilities.empty() );
        client.StopViewing();
        ASSERT_TRUE( 0 == recorder.viewing );
        REQUIRE_TRUE( client.RequestInfo() );
        CCounterAtLeast info( recorder.infos, 1 );
        REQUIRE_TRUE( PumpUntil( info, 3000 ) );
        ASSERT_TRUE( client.GetLastInfo().capabilities.find( "resizePool" ) != client.GetLastInfo().capabilities.end() );
        // C-3: InfoReceivedEvent carries the whole host info
        ASSERT_TRUE( "node1" == recorder.lastInfo.hostName && recorder.lastInfo.hostName == client.GetLastInfo().hostName );
        ASSERT_TRUE( recorder.lastInfo.hostInstanceId == client.GetLastInfo().hostInstanceId && 77 == recorder.lastInfo.hostInstanceId );
        ASSERT_TRUE( recorder.lastInfo.capabilities == client.GetLastInfo().capabilities && recorder.lastInfo.controllablePools == client.GetLastInfo().controllablePools );
        ASSERT_TRUE( 1 == recorder.lastInfo.controllablePools.size() );
        CORE::CString resizeId = client.ResizePool( "work", 2, 4, "more" );
        ASSERT_FALSE( resizeId.IsNULLOrEmpty() );
        CCounterAtLeast command( recorder.commands, 1 );
        REQUIRE_TRUE( PumpUntil( command, 3000 ) );
        ASSERT_TRUE( DRNUTILS::DRNUTILS_STATUS_OK == recorder.last.status );
        // C-3: the id of the command is the one CommandCompletedEvent carries
        ASSERT_TRUE( resizeId == recorder.last.commandId );
        ASSERT_TRUE( 1 == w.source.resizes.size() );
        client.CancelTask( 12, "not allowed" );
        CCounterAtLeast refused( recorder.commands, 2 );
        REQUIRE_TRUE( PumpUntil( refused, 3000 ) );
        ASSERT_TRUE( DRNUTILS::DRNUTILS_STATUS_PERMISSION_DENIED == recorder.last.status );

        REQUIRE_TRUE( client.StartViewing( 1000, false, true ) );
        ASSERT_TRUE( client.IsViewing() );
        PumpFor( 300 );
        ASSERT_TRUE( w.host->IsInViewingMode() );
        client.StopViewing();
        ASSERT_FALSE( client.IsViewing() );
        PumpFor( 700 );
        ASSERT_FALSE( w.host->IsInViewingMode() );
        ASSERT_TRUE( 2 == recorder.viewing );
        client.Unsubscribe( &recorder );
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
