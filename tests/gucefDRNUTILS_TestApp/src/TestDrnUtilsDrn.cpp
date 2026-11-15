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

#include <signal.h>

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

#ifndef GUCEF_COMCORE_CIRPCTRANSPORTLISTENER_H
#include "gucefCOMCORE_CIRpcTransportListener.h"
#define GUCEF_COMCORE_CIRPCTRANSPORTLISTENER_H
#endif /* GUCEF_COMCORE_CIRPCTRANSPORTLISTENER_H ? */

#ifndef GUCEF_DRN_CDRNRPCTRANSPORT_H
#include "gucefDRN_CDRNRpcTransport.h"
#define GUCEF_DRN_CDRNRPCTRANSPORT_H
#endif /* GUCEF_DRN_CDRNRPCTRANSPORT_H ? */

#ifndef GUCEF_DRN_CDRNRPCTRANSPORTLISTENER_H
#include "gucefDRN_CDRNRpcTransportListener.h"
#define GUCEF_DRN_CDRNRPCTRANSPORTLISTENER_H
#endif /* GUCEF_DRN_CDRNRPCTRANSPORTLISTENER_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_CJOBSCHEDULER_H
#include "gucefCORE_CJobScheduler.h"
#define GUCEF_CORE_CJOBSCHEDULER_H
#endif /* GUCEF_CORE_CJOBSCHEDULER_H ? */

#ifndef GUCEF_CORE_CJOBSCHEDULERFILESTATESTORE_H
#include "gucefCORE_CJobSchedulerFileStateStore.h"
#define GUCEF_CORE_CJOBSCHEDULERFILESTATESTORE_H
#endif /* GUCEF_CORE_CJOBSCHEDULERFILESTATESTORE_H ? */

#ifndef GUCEF_DRNUTILS_H
#include "gucefDRNUTILS.h"
#define GUCEF_DRNUTILS_H
#endif /* GUCEF_DRNUTILS_H ? */

#ifndef GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSRPCWORLD_H
#include "TestDrnUtilsRpcWorld.h"
#define GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSRPCWORLD_H
#endif /* GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSRPCWORLD_H ? */

#ifndef GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSDRNGROUP_H
#include "TestDrnUtilsDrnGroup.h"
#define GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSDRNGROUP_H
#endif /* GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSDRNGROUP_H ? */

#ifndef GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSFAKESCHEDULERSOURCE_H
#include "TestDrnUtilsFakeSchedulerSource.h"
#define GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSFAKESCHEDULERSOURCE_H
#endif /* GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSFAKESCHEDULERSOURCE_H ? */

#include "TestRpcSupport.h"
#include "TestDrnUtilsDrn.h"

using namespace GUCEF;
using namespace DRNUTILSTEST;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)
#define REQUIRE_TRUE(t)  do { if ( !(t) ) { ::GUCEF::TESTFW::CTestContext::Instance().RecordFailure( #t, __FILE__, __LINE__ ); throw std::exception(); } } while ( 0 )

namespace {

const char* D_GETINFO = "gucef.drnutils.scheduler.Scheduler/GetInfo";
const char* D_LISTJOBS = "gucef.drnutils.scheduler.Scheduler/ListJobs";
const char* D_WATCH = "gucef.drnutils.scheduler.Scheduler/Watch";
const char* D_CONTROL = "gucef.drnutils.scheduler.Scheduler/Control";

typedef CORE::CTSharedPtr< DRN::CDRNRpcTransport, MT::CMutex >          TDrnTransportPtr;
typedef CORE::CTSharedPtr< DRN::CDRNRpcTransportListener, MT::CMutex >  TDrnListenerPtr;

volatile sig_atomic_t g_childStop = 0;

/*-------------------------------------------------------------------------*/

UInt64
NowMs( void )
{
    UInt64 ticksPerMs = MT::PrecisionTimerResolution() / 1000;
    return MT::PrecisionTickCount() / ( ticksPerMs > 0 ? ticksPerMs : 1 );
}

extern "C" void
OnChildSignal( int )
{
    g_childStop = 1;
}

/*-------------------------------------------------------------------------*/

/**
 *  The serving side over drn: accepts RPC transports of a session and starts an endpoint on the shared service set.
 *  The principal the server sees is the drn transport's (subject, authentication, drn.protected), with the roles an
 *  authenticator of the service would give (the tests run anonymous links, which carry no role)
 */
class CDrnServer : public CORE::CObserver
{
    public:
    COMCORE::CRpcServiceSetPtr services;
    CORE::CString::StringSet roles;
    TDrnListenerPtr listener;
    GUCEF::vector< COMCORE::CRpcEndpointPtr > endpoints;
    GUCEF::vector< COMCORE::CIRpcTransportPtr > transports;
    COMCORE::CRpcPrincipal lastPrincipal;
    UInt32 accepted;

    CDrnServer( void ) : CORE::CObserver(), services( COMCORE::CRpcServiceSet::Create() ), roles(), listener(), endpoints(), transports(), lastPrincipal(), accepted( 0 ) {}

    virtual ~CDrnServer()
    {
        Stop();
        SignalUpcomingObserverDestruction();
    }

    bool Start( DRN::CDRNSession& session, const DRN::CDRNRpcTransportSettings& transportSettings = DRN::CDRNRpcTransportSettings() )
    {
        listener = TDrnListenerPtr( GUCEF_NEW DRN::CDRNRpcTransportListener( session, transportSettings ) );
        listener->Subscribe( this, COMCORE::CIRpcTransportListener::TransportAcceptedEvent );
        return listener->Listen( CORE::CUri() );
    }

    void Stop( void )
    {
        for ( UInt32 i=0; i<endpoints.size(); ++i )
            endpoints[ i ]->Close( false );
        endpoints.clear();
        transports.clear();
        if ( !listener.IsNULL() )
        {
            listener->Unsubscribe( this );
            listener->StopListening();
        }
        listener.Unlink();
    }

    protected:
    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( COMCORE::CIRpcTransportListener::TransportAcceptedEvent != eventid || GUCEF_NULL == eventdata )
            return;
        ++accepted;
        COMCORE::CRpcTransportAcceptedEventData* data = static_cast< COMCORE::CRpcTransportAcceptedEventData* >( eventdata );
        CRpcLossyTransport* wire = GUCEF_NULL;
        COMCORE::CIRpcTransportPtr transport = CRpcLossyTransport::Wrap( data->transport, &wire );
        COMCORE::CRpcPrincipal principal = data->transport->GetPeerPrincipal();
        principal.roles = roles;
        principal.isAuthenticated = true;
        wire->hasPrincipalOverride = true;
        wire->principalOverride = principal;
        lastPrincipal = principal;
        COMCORE::CRpcEndpointSettings settings;
        settings.nodeName = "drn-node";
        COMCORE::CRpcEndpointPtr endpoint = COMCORE::CRpcEndpoint::Create( CORE::CCoreGlobal::Instance()->GetPulseGenerator(), settings );
        endpoint->SetServiceSet( services );
        endpoint->Start( transport, false );
        endpoints.push_back( endpoint );
        transports.push_back( transport );
    }
};

/*-------------------------------------------------------------------------*/

/** The calling side over drn */
class CDrnClient
{
    public:
    COMCORE::CRpcEndpointPtr endpoint;
    TDrnTransportPtr drn;

    COMCORE::CRpcStatus connectStatus;

    bool Connect( DRN::CDRNSession& session, UInt32 hostPeerId, const DRN::CDRNRpcTransportSettings& transportSettings = DRN::CDRNRpcTransportSettings() )
    {
        COMCORE::CRpcEndpointSettings settings;
        settings.nodeName = "drn-viewer";
        endpoint = COMCORE::CRpcEndpoint::Create( CORE::CCoreGlobal::Instance()->GetPulseGenerator(), settings );
        endpoint->AddWantedContract( "gucef.drnutils.scheduler", 1 );
        drn = TDrnTransportPtr( GUCEF_NEW DRN::CDRNRpcTransport( session, hostPeerId, transportSettings ) );
        if ( !drn->Connect( CORE::CUri() ) )
        {
            connectStatus = drn->GetConnectStatus();
            return false;
        }
        COMCORE::CIRpcTransportPtr transport;
        transport = drn;
        if ( !endpoint->Start( transport, true ) )
            return false;
        class CBound : public CCondition
        {
            public:
            COMCORE::CRpcEndpointPtr& e;
            explicit CBound( COMCORE::CRpcEndpointPtr& ep ) : e( ep ) {}
            virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return e->IsContractBound( "gucef.drnutils.scheduler", 1 ); }
        };
        CBound bound( endpoint );
        return PumpUntil( bound, 10000 );
    }

    void Close( void )
    {
        if ( !endpoint.IsNULL() )
            endpoint->Close( false );
        PumpFor( 50 );
        endpoint.Unlink();
        drn.Unlink();
    }

    COMCORE::CRpcMessage NewRequest( const char* path )
    {
        COMCORE::CRpcMethodRef ref;
        COMCORE::CRpcStatus status;
        if ( !endpoint->ResolveMethod( path, ref, status ) )
            return COMCORE::CRpcMessage();
        return endpoint->NewRequest( ref );
    }

    COMCORE::CRpcCallResult Call( const char* path, const COMCORE::CRpcMessage& request, UInt32 timeoutMs = 10000 )
    {
        COMCORE::CRpcCallResult result;
        CORE::CFutureResult future = endpoint->CallAsync( path, request );
        class CReady : public CCondition
        {
            public:
            CORE::CFutureResult& f;
            explicit CReady( CORE::CFutureResult& fr ) : f( fr ) {}
            virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return f.IsReady() || f.HasNoFuture(); }
        };
        CReady ready( future );
        if ( !PumpUntil( ready, timeoutMs ) )
        {
            result.status.Set( COMCORE::RPC_STATUS_DEADLINE_EXCEEDED, "test timeout" );
            return result;
        }
        COMCORE::CRpcEndpoint::TryGetCallResult( future, result );
        return result;
    }

    COMCORE::CRpcCallResult Control( const CORE::CString& commandId, CORE::TJobControlAction action, const CORE::CString& target, bool waitForRun )
    {
        COMCORE::CRpcMessage request = NewRequest( D_CONTROL );
        request.SetString( 1, commandId );
        request.SetEnum( 2, (Int32) action );
        request.SetEnum( 3, CORE::JOBCONTROLACTION_CANCEL_RUN == action ? CORE::JOBCONTROLTARGET_RUN : CORE::JOBCONTROLTARGET_JOB );
        request.SetString( 4, target );
        request.SetBool( 13, waitForRun );
        return Call( D_CONTROL, request, 15000 );
    }
};

/*-------------------------------------------------------------------------*/

DRNUTILS::CSchedulerHostSettings
ControlSettings( void )
{
    DRNUTILS::CSchedulerHostSettings settings;
    settings.SetExposure( DRNUTILS::DRNUTILS_EXPOSURE_CONTROL );
    CORE::TJobControlActionSet allowed;
    allowed.insert( CORE::JOBCONTROLACTION_RUN_NOW );
    allowed.insert( CORE::JOBCONTROLACTION_CANCEL_RUN );
    allowed.insert( CORE::JOBCONTROLACTION_ENABLE );
    allowed.insert( CORE::JOBCONTROLACTION_DISABLE );
    settings.SetAllowedActions( allowed );
    settings.SetTablePrefix( "svc" );
    return settings;
}

/*-------------------------------------------------------------------------*/

/** Ends the runs of a fake source that are older than the given time, as a real scheduler's tasks would */
void
EndOldRuns( CFakeSchedulerSource& source, GUCEF::map< UInt64, UInt64 >& startedAt, UInt64 nowMs, UInt64 afterMs )
{
    GUCEF::vector< UInt64 > due;
    {
        MT::CScopeMutex lock( source.lock );
        for ( UInt32 r=0; r<source.runs.size(); ++r )
        {
            if ( CORE::JobRunStateIsAnEndState( source.runs[ r ].state ) )
                continue;
            UInt64 runId = source.runs[ r ].runId;
            if ( startedAt.find( runId ) == startedAt.end() )
                startedAt[ runId ] = nowMs;
            else
            if ( nowMs - startedAt[ runId ] >= afterMs )
                due.push_back( runId );
        }
    }
    for ( UInt32 i=0; i<due.size(); ++i )
        source.EndRun( due[ i ], CORE::JOBRUNSTATE_SUCCEEDED );
}

/*-------------------------------------------------------------------------*/

/** A task of one short cycle for the real scheduler of the child (RU-T12) */
class CDrnUtilsChildTask : public CORE::CTaskConsumer
{
    public:
    static const char* TaskType;
    CDrnUtilsChildTask( void ) : CORE::CTaskConsumer() {}
    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return TaskType; }
    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE
    {
        MT::PrecisionDelay( 20 );
        return true;
    }
    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CTaskConsumer::OnTaskEnded( task, wasForced );
    }
};
const char* CDrnUtilsChildTask::TaskType = "drnutils.test.childtask";
typedef CORE::CTFactory< CORE::CTaskConsumer, CDrnUtilsChildTask, MT::CMutex > TChildTaskFactory;

/** The node lock is free at some moment within the time (a node thread that waits under it for a lock this thread holds keeps it) */
bool
NodeLockFreeWithin( DRN::CDRNNode& node, UInt32 timeoutMs )
{
    UInt64 start = NowMs();
    while ( NowMs() - start < timeoutMs )
    {
        if ( !node.GetLock().IsLocked() )
            return true;
        MT::PrecisionDelay( 1 );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/**
 *  Lock order of design 9.1 (drn transport design T14): the test thread holds the notification lock of the global
 *  pulse generator, as a pulse does, while the server's node thread delivers Control frames and the end of a watch
 *  stream. The node threads may not wait for that lock under their node lock: both node locks are free within 2 s
 */
void
RunNotificationLockOrderCase( CDrnUtilsGroup& group, CDrnServer& server, CDrnClient& client )
{
    CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    REQUIRE_TRUE( !server.endpoints.empty() );
    COMCORE::CRpcEndpointPtr serverEndpoint = server.endpoints.back();

    COMCORE::CRpcMessage watchRequest = client.NewRequest( D_WATCH );
    COMCORE::CRpcMethodRef ref;
    COMCORE::CRpcStatus status;
    REQUIRE_TRUE( client.endpoint->ResolveMethod( D_WATCH, ref, status ) );
    COMCORE::CIRpcStreamPtr stream = client.endpoint->OpenStream( ref, &watchRequest );
    REQUIRE_TRUE( !stream.IsNULL() );
    CStreamRecorder recorder;
    recorder.Watch( stream );
    CCounterAtLeast snapshot( recorder.count, 3 );
    REQUIRE_TRUE( PumpUntil( snapshot, 10000 ) );

    GUCEF::vector< CORE::CFutureResult > futures;
    bool delivered = false;
    bool serverNodeFree = false;
    bool clientNodeFree = false;
    {
        CORE::CNotifierScopeLock pulseLock( pulseGenerator.GetPointerAlways() );
        UInt64 deliveredSeq = serverEndpoint->GetDeliveredSeq();
        for ( UInt32 i=0; i<3; ++i )
        {
            COMCORE::CRpcMessage request = client.NewRequest( D_CONTROL );
            request.SetString( 1, "lock-order-" + CORE::ToString( i ) );
            request.SetEnum( 2, (Int32) CORE::JOBCONTROLACTION_RUN_NOW );
            request.SetEnum( 3, (Int32) CORE::JOBCONTROLTARGET_JOB );
            request.SetString( 4, "j2" );
            futures.push_back( client.endpoint->CallAsync( D_CONTROL, request ) );
        }
        stream->Cancel( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_CANCELLED, "viewer closed under the pulse lock" ) );
        UInt64 start = NowMs();
        while ( serverEndpoint->GetDeliveredSeq() == deliveredSeq && NowMs() - start < 5000 )
            MT::PrecisionDelay( 1 );
        delivered = serverEndpoint->GetDeliveredSeq() != deliveredSeq;
        MT::PrecisionDelay( 100 );
        serverNodeFree = NodeLockFreeWithin( *group.nodes[ 0 ], 2000 );
        clientNodeFree = NodeLockFreeWithin( *group.nodes[ 1 ], 2000 );
    }
    ASSERT_TRUE( delivered );
    ASSERT_TRUE( serverNodeFree );
    ASSERT_TRUE( clientNodeFree );

    // with the lock let go the commands are answered (applied: the link is protected) and the stream ends
    for ( UInt32 i=0; i<futures.size(); ++i )
    {
        class CReady : public CCondition
        {
            public:
            CORE::CFutureResult& f;
            explicit CReady( CORE::CFutureResult& fr ) : f( fr ) {}
            virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return f.IsReady() || f.HasNoFuture(); }
        };
        CReady ready( futures[ i ] );
        ASSERT_TRUE( PumpUntil( ready, 10000 ) );
        COMCORE::CRpcCallResult result;
        ASSERT_TRUE( COMCORE::CRpcEndpoint::TryGetCallResult( futures[ i ], result ) && result.status.IsOk() );
    }
    PumpFor( 100 );
}

/*-------------------------------------------------------------------------*/

/** RU-T2 .. RU-T8 essentials over a drn link */
void
RunOverDrn( CDrnClient& client, CFakeSchedulerSource* localSource, bool protectedLink )
{
    COMCORE::CRpcCallResult info = client.Call( D_GETINFO, client.NewRequest( D_GETINFO ) );
    REQUIRE_TRUE( info.status.IsOk() );
    ASSERT_TRUE( info.response.GetCount( 7 ) == 3 );
    COMCORE::CRpcCallResult jobs = client.Call( D_LISTJOBS, client.NewRequest( D_LISTJOBS ) );
    REQUIRE_TRUE( jobs.status.IsOk() );
    ASSERT_TRUE( jobs.response.GetCount( 1 ) >= 2 );

    COMCORE::CRpcMessage watchRequest = client.NewRequest( D_WATCH );
    COMCORE::CRpcMethodRef ref;
    COMCORE::CRpcStatus status;
    REQUIRE_TRUE( client.endpoint->ResolveMethod( D_WATCH, ref, status ) );
    COMCORE::CIRpcStreamPtr stream = client.endpoint->OpenStream( ref, &watchRequest );
    REQUIRE_TRUE( !stream.IsNULL() );
    CStreamRecorder recorder;
    recorder.Watch( stream );
    CCounterAtLeast snapshot( recorder.count, 3 );
    REQUIRE_TRUE( PumpUntil( snapshot, 10000 ) );
    ASSERT_TRUE( 4 == recorder.messages[ 0 ].GetEnum( 1 ) );

    COMCORE::CRpcCallResult run = client.Control( "drn-run-1", CORE::JOBCONTROLACTION_RUN_NOW, "j1", true );
    if ( protectedLink )
    {
        // ACCEPTED, then the RESULT once the run ended
        ASSERT_TRUE( run.status.IsOk() && run.wasAccepted );
        ASSERT_TRUE( GUCEF_NULL != run.response.GetMessage( 5 ) && CORE::JOBRUNSTATE_SUCCEEDED == run.response.GetMessage( 5 )->GetEnum( 5 ) );
        // the change of the job reached the open watch stream
        UInt64 before = recorder.count;
        if ( GUCEF_NULL != localSource )
            localSource->Touch( "j1" );
        CCounterAtLeast change( recorder.count, before + 1 );
        ASSERT_TRUE( PumpUntil( change, 10000 ) );
    }
    else
    {
        // a link without protection is not a secure transport: no control over it
        ASSERT_TRUE( COMCORE::RPC_STATUS_PERMISSION_DENIED == run.status.code );
    }
    stream->Cancel( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_CANCELLED, "done" ) );
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------*/

int
RunDrnUtilsChildHost( const char* portText )
{GUCEF_TRACE;

    signal( SIGTERM, OnChildSignal );
    signal( SIGINT, OnChildSignal );
    CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    pulseGenerator->SetPulseDriverThreadId( MT::GetCurrentThreadID() );
    int exitCode = 0;
    EnsureCryptoCodecPluginLoaded();
    const char* statePath = getenv( "GUCEF_DRNUTILS_TEST_CHILD_STATE" );
    // the state file of the scheduler is JSON: the codec is a plugin
    if ( !RpcTestJsonCodecAvailable() )
        return 6;
    CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
    TChildTaskFactory taskFactory;
    taskManager.RegisterTaskConsumerFactory( CDrnUtilsChildTask::TaskType, &taskFactory );
    {
        DRN::CDRNNodeSettings nodeSettings;
        nodeSettings.echoIntervalMs = 50;
        nodeSettings.pulseIntervalMs = 1;
        DRN::CDRNNode node;
        node.SetSettings( nodeSettings );
        DRN::CDRNSession* session = node.CreateSession();
        DRN::CDRNEndpoint endpoint( DRN::CDRNEndpoint::SchemeForChannelType( "tcp" ) + "://127.0.0.1:" + CORE::CString( portText ) );
        DRN::CDRNSessionSettings sessionSettings;
        sessionSettings.sessionName = "drnutils-child";
        if ( !node.Listen( endpoint ) || !session->Host( sessionSettings ) )
            return 2;
        DRN::CDRNGlobal::Instance()->GetSessionRegistry().Register( *session );

        // a real scheduler with its state in a file: j1 may be run remotely, j2 not, ticker runs every 300 ms
        CORE::CJobSchedulerFileStateStore store( GUCEF_NULL != statePath ? CORE::CString( statePath ) : CORE::CString::Empty );
        CORE::CJobSchedulerSettings schedulerSettings;
        schedulerSettings.SetHostName( "child-node" );
        schedulerSettings.SetDispatcherPoolName( "drnutils.child.dispatch" );
        schedulerSettings.SetRunPoolName( "drnutils.child.run" );
        schedulerSettings.SetMainThreadPoolName( "drnutils.child.main" );
        CORE::JobSchedulerPtr scheduler = taskManager.GetOrCreateJobScheduler( schedulerSettings, pulseGenerator );
        if ( scheduler.IsNULL() )
            return 3;
        scheduler->SetStateStore( &store );
        CORE::TJobControlActionSet remote;
        remote.insert( CORE::JOBCONTROLACTION_RUN_NOW );
        CORE::CJobDefinition j1;
        j1.SetId( "j1" );
        j1.SetTaskTypeName( CDrnUtilsChildTask::TaskType );
        j1.SetTrigger( CORE::CJobTrigger::Manual() );
        j1.SetRemoteActions( remote );
        CORE::CJobDefinition j2;
        j2.SetId( "j2" );
        j2.SetTaskTypeName( CDrnUtilsChildTask::TaskType );
        j2.SetTrigger( CORE::CJobTrigger::Manual() );
        CORE::CJobDefinition ticker;
        ticker.SetId( "ticker" );
        ticker.SetTaskTypeName( CDrnUtilsChildTask::TaskType );
        ticker.SetTrigger( CORE::CJobTrigger::Interval( 300 ) );
        CORE::CString error;
        if ( !scheduler->AddJob( j1, error ) || !scheduler->AddJob( j2, error ) || !scheduler->AddJob( ticker, error ) || !scheduler->Start( error ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DrnUtils child host: the scheduler did not start: " << error );
            return 4;
        }

        DRNUTILS::CJobSchedulerSource source( scheduler, pulseGenerator );
        DRNUTILS::CSchedulerHostSettings settings = ControlSettings();
        DRNUTILS::CSchedulerHost host( source, settings, pulseGenerator );
        CDrnServer server;
        server.roles.insert( "scheduler.view" );
        server.roles.insert( "scheduler.control" );
        if ( !host.Expose( server.services, error ) || !host.PublishTables( "drnutils-child", error ) || !server.Start( *session ) )
            return 5;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DrnUtils child host: serving on port " << portText );

        while ( 0 == g_childStop )
        {
            pulseGenerator->ForceDirectPulse();
            MT::PrecisionDelay( 1 );
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DrnUtils child host: stopping on request" );
        server.Stop();
        host.StopPublishing();
        host.Withdraw();
        // the scheduler saves its state as it stops
        scheduler->Stop();
        taskManager.UnregisterJobScheduler( "child-node" );
        scheduler.Unlink();
        DRN::CDRNGlobal::Instance()->GetSessionRegistry().Unregister( *session );
        node.Stop();
    }
    taskManager.UnregisterTaskConsumerFactory( CDrnUtilsChildTask::TaskType );
    return exitCode;
}

/*-------------------------------------------------------------------------*/

void
PerformDrnUtilsDrnTests( const char* exePath )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "DrnUtilsDrn" );

    GUCEF_TESTFW_TESTCASE( "RU-T10 two members in one process over drn (nodes pumped, nodes on threads of their own): info, lists, watch, RUN_NOW with waitForRun over an encrypted link; the lock order of 9.1 under the notification lock" )
    try
    {
        CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        TThreadId previousDriver = pulseGenerator->GetPulseDriverThreadId();
        pulseGenerator->SetPulseDriverThreadId( MT::GetCurrentThreadID() );
        for ( UInt32 threaded=0; threaded<2; ++threaded )
        {
            CDrnUtilsGroup group( 0 == threaded ? "drnutils-rpc" : "drnutils-rpc-threads" );
            group.nodeSettings.ownThread = 1 == threaded;
            REQUIRE_TRUE( group.Start( 2 ) );
            CFakeSchedulerSource source( pulseGenerator );
            source.AddLane( "lane1" );
            source.AddJob( "j1" );
            source.AddJob( "j2" );
            DRNUTILS::CSchedulerHost host( source, ControlSettings(), pulseGenerator );
            CDrnServer server;
            server.roles.insert( "scheduler.view" );
            server.roles.insert( "scheduler.control" );
            CORE::CString error;
            REQUIRE_TRUE( host.Expose( server.services, error ) );
            REQUIRE_TRUE( host.PublishTables( group.name, error ) );
            REQUIRE_TRUE( server.Start( group[ 0 ] ) );
            CDrnClient client;
            REQUIRE_TRUE( client.Connect( group[ 1 ], group[ 0 ].GetLocalPeerId() ) );
            bool protectedLink = server.lastPrincipal.attributes.GetValueAlways( "drn.protected" ).AsBool( false );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RU-T10: node threads " << threaded << ", link protected " << protectedLink << ", principal " << server.lastPrincipal.subject );
            ASSERT_TRUE( "drn" == server.lastPrincipal.transport );
            // the default security policy encrypts every link (the crypto codecs are loaded)
            ASSERT_TRUE( protectedLink );

            // the runs of the fake end after 300 ms, as tasks would
            class CRunEnder : public CORE::CObserver
            {
                public:
                CFakeSchedulerSource& s;
                GUCEF::map< UInt64, UInt64 > startedAt;
                CRunEnder( CFakeSchedulerSource& source ) : s( source ), startedAt() {}
                virtual ~CRunEnder() { SignalUpcomingObserverDestruction(); }
                virtual void OnNotify( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ) GUCEF_VIRTUAL_OVERRIDE
                {
                    EndOldRuns( s, startedAt, NowMs(), 300 );
                }
            };
            CRunEnder ender( source );
            CORE::CTimer timer( pulseGenerator, 50 );
            timer.Subscribe( &ender );
            timer.SetEnabled( true );
            RunOverDrn( client, &source, protectedLink );
            if ( 1 == threaded )
                RunNotificationLockOrderCase( group, server, client );
            timer.SetEnabled( false );
            timer.Unsubscribe( &ender );
            client.Close();
            server.Stop();
            host.StopPublishing();
            host.Withdraw();
            PumpFor( 50 );
        }
        pulseGenerator->SetPulseDriverThreadId( previousDriver );
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

    GUCEF_TESTFW_TESTCASE( "RU-T4 over drn: a member link without packet protection is refused by the drn transport; allowed by its settings, the views work and every control and submission is refused (not a secure transport)" )
    try
    {
        CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        TThreadId previousDriver = pulseGenerator->GetPulseDriverThreadId();
        pulseGenerator->SetPulseDriverThreadId( MT::GetCurrentThreadID() );
        {
            CDrnUtilsGroup group( "drnutils-rpc-plain" );
            group.nodeSettings.securityPolicy.requireEncryption = false;
            group.nodeSettings.securityPolicy.ciphers.clear();
            REQUIRE_TRUE( group.Start( 2 ) );
            DRN::CDRNPeerLinkPtr link = group[ 1 ].GetLinkToMember( group[ 0 ].GetLocalPeerId() );
            REQUIRE_TRUE( !link.IsNULL() );
            ASSERT_FALSE( link->IsProtected() );

            CFakeSchedulerSource source( pulseGenerator );
            source.AddLane( "lane1" );
            source.AddJob( "j1" );
            source.AddJob( "j2" );
            DRNUTILS::CSchedulerHostSettings hostSettings = ControlSettings();
            CORE::CString::StringSet submittable;
            submittable.insert( "fake.task" );
            hostSettings.SetSubmittableTaskTypes( submittable );
            DRNUTILS::CSchedulerHost host( source, hostSettings, pulseGenerator );
            CDrnServer server;
            server.roles.insert( "scheduler.view" );
            server.roles.insert( "scheduler.control" );
            server.roles.insert( "scheduler.submit" );
            CORE::CString error;
            REQUIRE_TRUE( host.Expose( server.services, error ) );
            REQUIRE_TRUE( host.PublishTables( group.name, error ) );

            // the drn transport refuses a link without protection by default
            DRN::CDRNRpcTransportSettings allowPlain;
            allowPlain.requireProtectedLink = false;
            REQUIRE_TRUE( server.Start( group[ 0 ], allowPlain ) );
            {
                CDrnClient refused;
                ASSERT_FALSE( refused.Connect( group[ 1 ], group[ 0 ].GetLocalPeerId() ) );
                ASSERT_TRUE( COMCORE::RPC_STATUS_PERMISSION_DENIED == refused.connectStatus.code );
                refused.Close();
            }

            // allowed on both sides: the server sees drn.protected = false
            CDrnClient client;
            REQUIRE_TRUE( client.Connect( group[ 1 ], group[ 0 ].GetLocalPeerId(), allowPlain ) );
            ASSERT_TRUE( "drn" == server.lastPrincipal.transport );
            ASSERT_FALSE( server.lastPrincipal.attributes.GetValueAlways( "drn.protected" ).AsBool( true ) );
            RunOverDrn( client, &source, false );

            UInt32 appliedBefore = (UInt32) source.applied.size();
            COMCORE::CRpcCallResult disable = client.Control( "plain-disable", CORE::JOBCONTROLACTION_DISABLE, "j2", false );
            ASSERT_TRUE( COMCORE::RPC_STATUS_PERMISSION_DENIED == disable.status.code );
            COMCORE::CRpcMessage submit = client.NewRequest( "gucef.drnutils.scheduler.Scheduler/SubmitJob" );
            submit.SetString( 1, "plain-submit" );
            submit.SetString( 2, "submitted-1" );
            submit.SetString( 4, "fake.task" );
            COMCORE::CRpcCallResult submitted = client.Call( "gucef.drnutils.scheduler.Scheduler/SubmitJob", submit );
            ASSERT_TRUE( COMCORE::RPC_STATUS_PERMISSION_DENIED == submitted.status.code );
            ASSERT_TRUE( appliedBefore == (UInt32) source.applied.size() );
            client.Close();
            server.Stop();
            host.StopPublishing();
            host.Withdraw();
            PumpFor( 50 );
        }
        pulseGenerator->SetPulseDriverThreadId( previousDriver );
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

    GUCEF_TESTFW_TESTCASE( "RU-T12 two processes: a child hosts a real CJobScheduler over drn; the parent views, runs now (success and a refused job told apart), watches; the link is killed, the changes made while away arrive after the reconnect without RESET; the child exits 0 on SIGTERM with its state saved in its state file" )
    try
    {
        GUCEF_TESTFW_SKIP_IF( GUCEF_NULL == exePath || '\0' == *exePath, "the path of the test application is not known" );
        CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        TThreadId previousDriver = pulseGenerator->GetPulseDriverThreadId();
        pulseGenerator->SetPulseDriverThreadId( MT::GetCurrentThreadID() );
        {
            UInt16 port = TESTFW::FindFreeTcpAndUdpPort( 0 );
            REQUIRE_TRUE( 0 != port );
            CORE::CString statePath = CORE::RelativePath( "$CURWORKDIR$" );
            CORE::AppendToPath( statePath, "gucefDRNUTILS_TestApp_ChildState.json" );
            CORE::DeleteFile( statePath );
            CORE::CProcess child;
            CORE::CProcess::TStringMap environment;
            environment[ "GUCEF_DRNUTILS_TEST_CHILD" ] = CORE::UInt32ToString( port );
            environment[ "GUCEF_DRNUTILS_TEST_CHILD_STATE" ] = statePath;
            CORE::CProcess::TStringVector args;
            REQUIRE_TRUE( child.Launch( exePath, args, CORE::CString(), environment ) );
            PumpFor( 1500 );
            ASSERT_TRUE( child.IsRunning() );

            DRN::CDRNEndpoint hostEndpoint( DRN::CDRNEndpoint::SchemeForChannelType( "tcp" ) + "://127.0.0.1:" + CORE::UInt32ToString( port ) );
            UInt64 cursor = 0;
            UInt64 instance = 0;
            UInt32 succeededRunNows = 0;
            EnsureCryptoCodecPluginLoaded();
            for ( UInt32 round=0; round<2; ++round )
            {
                // a node of its own per round: the first one is stopped without closing anything (the link is killed)
                DRN::CDRNNodeSettings nodeSettings;
                nodeSettings.echoIntervalMs = 50;
                nodeSettings.pulseIntervalMs = 1;
                DRN::CDRNNode node;
                node.SetSettings( nodeSettings );
                DRN::CDRNSession* session = node.CreateSession();
                DRN::CDRNSessionSettings joinSettings;
                joinSettings.sessionName = "drnutils-child";
                bool joined = false;
                for ( UInt32 attempt=0; attempt<20 && !joined; ++attempt )
                {
                    joined = session->Join( DRN::CDRNEndpointList( hostEndpoint ), joinSettings );
                    if ( !joined )
                        PumpFor( 250 );
                }
                REQUIRE_TRUE( joined );
                class CActive : public CCondition
                {
                    public:
                    DRN::CDRNSession& s;
                    explicit CActive( DRN::CDRNSession& session ) : s( session ) {}
                    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return s.IsActive() && s.GetMemberCount() >= 2; }
                };
                CActive active( *session );
                REQUIRE_TRUE( PumpUntil( active, 15000 ) );
                DRN::CDRNPeerLinkPtr link = session->GetLinkToMember( session->GetSequencerPeerId() );
                ASSERT_TRUE( !link.IsNULL() && link->IsProtected() );
                CDrnClient client;
                REQUIRE_TRUE( client.Connect( *session, session->GetSequencerPeerId() ) );
                COMCORE::CRpcCallResult info = client.Call( D_GETINFO, client.NewRequest( D_GETINFO ) );
                REQUIRE_TRUE( info.status.IsOk() );
                ASSERT_TRUE( "child-node" == info.response.GetString( 1 ) );
                if ( 1 == round )
                    ASSERT_TRUE( instance == info.response.GetUInt64( 2 ) );
                instance = info.response.GetUInt64( 2 );
                const UInt64 revisionAtConnect = info.response.GetUInt64( 3 );

                COMCORE::CRpcMessage watchRequest = client.NewRequest( D_WATCH );
                if ( 0 != cursor )
                {
                    watchRequest.SetUInt64( 1, instance );
                    watchRequest.SetUInt64( 2, cursor );
                }
                COMCORE::CRpcMethodRef ref;
                COMCORE::CRpcStatus status;
                REQUIRE_TRUE( client.endpoint->ResolveMethod( D_WATCH, ref, status ) );
                COMCORE::CIRpcStreamPtr stream = client.endpoint->OpenStream( ref, &watchRequest );
                REQUIRE_TRUE( !stream.IsNULL() );
                CStreamRecorder recorder;
                recorder.Watch( stream );
                CCounterAtLeast some( recorder.count, 1 );
                REQUIRE_TRUE( PumpUntil( some, 10000 ) );

                // run-now of j1 succeeds; j2 does not allow it remotely: PERMISSION_DENIED, told apart from success
                COMCORE::CRpcCallResult run = client.Control( "child-run-" + CORE::ToString( round ), CORE::JOBCONTROLACTION_RUN_NOW, "j1", true );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RU-T12 round " << round << ": run-now of j1 " << run.status.ToString() );
                ASSERT_TRUE( run.status.IsOk() && run.wasAccepted && GUCEF_NULL != run.response.GetMessage( 5 ) );
                if ( run.status.IsOk() && GUCEF_NULL != run.response.GetMessage( 5 ) && CORE::JOBRUNSTATE_SUCCEEDED == run.response.GetMessage( 5 )->GetEnum( 5 ) )
                    ++succeededRunNows;
                COMCORE::CRpcCallResult refused = client.Control( "child-refused-" + CORE::ToString( round ), CORE::JOBCONTROLACTION_RUN_NOW, "j2", true );
                ASSERT_TRUE( COMCORE::RPC_STATUS_PERMISSION_DENIED == refused.status.code && !refused.wasAccepted );
                PumpFor( 500 );

                if ( 0 == round )
                {
                    ASSERT_TRUE( 4 == recorder.messages[ 0 ].GetEnum( 1 ) );
                }
                else
                {
                    // resumed from the cursor of the previous round: no RESET, and the changes made while the link was
                    // down (the ticker kept running) arrive first, revisions after the cursor up to the one at connect
                    bool reset = false;
                    bool whileAway = false;
                    for ( UInt32 m=0; m<recorder.messages.size(); ++m )
                    {
                        reset = reset || 4 == recorder.messages[ m ].GetEnum( 1 );
                        UInt64 revision = recorder.messages[ m ].GetUInt64( 2 );
                        whileAway = whileAway || ( revision > cursor && revision <= revisionAtConnect );
                    }
                    ASSERT_FALSE( reset );
                    ASSERT_TRUE( revisionAtConnect > cursor );
                    ASSERT_TRUE( whileAway );
                }
                for ( UInt32 m=0; m<recorder.messages.size(); ++m )
                    if ( recorder.messages[ m ].GetUInt64( 2 ) > cursor )
                        cursor = recorder.messages[ m ].GetUInt64( 2 );
                if ( 0 == round )
                {
                    // kill the link: the node goes without closing the stream, the endpoint or the session
                    node.Stop();
                    PumpFor( 1500 );
                    stream.Unlink();
                    client.Close();
                }
                else
                {
                    stream->Cancel( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_CANCELLED, "round done" ) );
                    client.Close();
                    node.Stop();
                }
                PumpFor( 200 );
            }
            ASSERT_TRUE( 2 == succeededRunNows );
            ASSERT_TRUE( child.Terminate( true, 10000 ) );
            Int32 exitCode = -1;
            ASSERT_TRUE( child.WaitForExit( 10000, exitCode ) );
            ASSERT_TRUE( 0 == exitCode );

            // the state the child saved as its scheduler stopped
            REQUIRE_TRUE( RpcTestJsonCodecAvailable() );
            CORE::CJobSchedulerFileStateStore store( statePath );
            CORE::CDataNode state;
            CORE::CString error;
            ASSERT_TRUE( store.LoadState( state, error ) );
            const CORE::CDataNode* jobs = state.FindChild( "jobs" );
            REQUIRE_TRUE( GUCEF_NULL != jobs );
            const CORE::CDataNode* j1 = jobs->FindChild( "j1" );
            const CORE::CDataNode* ticker = jobs->FindChild( "ticker" );
            ASSERT_TRUE( GUCEF_NULL != j1 && 2 <= j1->GetAttributeValueOrChildValueByName( "runCount" ).AsUInt32( 0 ) );
            ASSERT_TRUE( GUCEF_NULL != ticker && 3 <= ticker->GetAttributeValueOrChildValueByName( "runCount" ).AsUInt32( 0 ) );
            ASSERT_TRUE( "child-node" == state.GetAttributeValueOrChildValueByName( "hostName" ).AsString() );
        }
        pulseGenerator->SetPulseDriverThreadId( previousDriver );
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
