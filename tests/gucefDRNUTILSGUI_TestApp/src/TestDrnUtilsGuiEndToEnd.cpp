/*
 *  gucefDRNUTILSGUI_TestApp: GUCEF test application for the gucefDRNUTILSGUI module
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

#include <exception>

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_UNIQUE_H
#include "gucef_test_unique.h"
#define GUCEF_TEST_UNIQUE_H
#endif /* GUCEF_TEST_UNIQUE_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_CJOBSCHEDULER_H
#include "gucefCORE_CJobScheduler.h"
#define GUCEF_CORE_CJOBSCHEDULER_H
#endif /* GUCEF_CORE_CJOBSCHEDULER_H ? */

#ifndef GUCEF_COMCORE_CRPCENDPOINT_H
#include "gucefCOMCORE_CRpcEndpoint.h"
#define GUCEF_COMCORE_CRPCENDPOINT_H
#endif /* GUCEF_COMCORE_CRPCENDPOINT_H ? */

#ifndef GUCEF_COMCORE_CIRPCTRANSPORTLISTENER_H
#include "gucefCOMCORE_CIRpcTransportListener.h"
#define GUCEF_COMCORE_CIRPCTRANSPORTLISTENER_H
#endif /* GUCEF_COMCORE_CIRPCTRANSPORTLISTENER_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#ifndef GUCEF_DRN_CDRNSESSION_H
#include "gucefDRN_CDRNSession.h"
#define GUCEF_DRN_CDRNSESSION_H
#endif /* GUCEF_DRN_CDRNSESSION_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#ifndef GUCEF_DRN_CDRNGLOBAL_H
#include "gucefDRN_CDRNGlobal.h"
#define GUCEF_DRN_CDRNGLOBAL_H
#endif /* GUCEF_DRN_CDRNGLOBAL_H ? */

#ifndef GUCEF_DRN_CDRNSESSIONREGISTRY_H
#include "gucefDRN_CDRNSessionRegistry.h"
#define GUCEF_DRN_CDRNSESSIONREGISTRY_H
#endif /* GUCEF_DRN_CDRNSESSIONREGISTRY_H ? */

#ifndef GUCEF_DRN_CDRNRPCTRANSPORT_H
#include "gucefDRN_CDRNRpcTransport.h"
#define GUCEF_DRN_CDRNRPCTRANSPORT_H
#endif /* GUCEF_DRN_CDRNRPCTRANSPORT_H ? */

#ifndef GUCEF_DRN_CDRNRPCTRANSPORTLISTENER_H
#include "gucefDRN_CDRNRpcTransportListener.h"
#define GUCEF_DRN_CDRNRPCTRANSPORTLISTENER_H
#endif /* GUCEF_DRN_CDRNRPCTRANSPORTLISTENER_H ? */

#include "TestRpcLossyTransport.h"
#include "TestDrnUtilsGuiFixtures.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)
#define REQUIRE_TRUE(t) do { if ( !(t) ) { ::GUCEF::TESTFW::CTestContext::Instance().RecordFailure( #t, __FILE__, __LINE__ ); throw std::exception(); } } while ( 0 )

namespace {

const char* E2E_HOST_NAME = "e2e-node";
const char* E2E_SESSION_NAME = "drnutils-gui-e2e";

/** A task of one short cycle: the work of the jobs of the real scheduler */
class CE2ETask : public CORE::CTaskConsumer
{
    public:
    static const char* TaskType;
    CE2ETask( void ) : CORE::CTaskConsumer() {}
    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return TaskType; }
    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE
    {
        MT::PrecisionDelay( 10 );
        return true;
    }
    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CTaskConsumer::OnTaskEnded( task, wasForced );
    }
};
const char* CE2ETask::TaskType = "drnutilsgui.test.e2e";
typedef CORE::CTFactory< CORE::CTaskConsumer, CE2ETask, MT::CMutex > TE2ETaskFactory;

/*-------------------------------------------------------------------------*/

/** A real CJobScheduler of the task manager with two jobs: "quotes" (manual, run-now allowed remotely) and "rebuild" (every hour) */
class CE2EScheduler
{
    public:
    TE2ETaskFactory factory;
    CORE::JobSchedulerPtr scheduler;
    bool registered;

    CE2EScheduler( void ) : factory(), scheduler(), registered( false ) {}
    ~CE2EScheduler() { Stop(); }

    bool Start( void )
    {
        CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
        taskManager.RegisterTaskConsumerFactory( CE2ETask::TaskType, &factory );
        registered = true;
        CORE::CJobSchedulerSettings settings;
        settings.SetHostName( E2E_HOST_NAME );
        settings.SetDispatcherPoolName( "drnutilsgui.e2e.dispatch" );
        settings.SetRunPoolName( "drnutilsgui.e2e.run" );
        settings.SetMainThreadPoolName( "drnutilsgui.e2e.main" );
        scheduler = taskManager.GetOrCreateJobScheduler( settings, CORE::CCoreGlobal::Instance()->GetPulseGenerator() );
        if ( scheduler.IsNULL() )
            return false;
        CORE::TJobControlActionSet remote;
        remote.insert( CORE::JOBCONTROLACTION_RUN_NOW );
        CORE::CJobDefinition quotes;
        quotes.SetId( "quotes" );
        quotes.SetTitle( "Fetch quotes" );
        quotes.SetDescription( "Fetches the quotes of the watch list" );
        quotes.SetTaskTypeName( CE2ETask::TaskType );
        quotes.SetTrigger( CORE::CJobTrigger::Manual() );
        quotes.SetRemoteActions( remote );
        CORE::CJobDefinition rebuild;
        rebuild.SetId( "rebuild" );
        rebuild.SetTitle( "Rebuild index" );
        rebuild.SetTaskTypeName( CE2ETask::TaskType );
        rebuild.SetTrigger( CORE::CJobTrigger::Interval( 3600000 ) );
        CORE::CString error;
        return scheduler->AddJob( quotes, error ) && scheduler->AddJob( rebuild, error ) && scheduler->Start( error );
    }

    void Stop( void )
    {
        CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
        if ( !scheduler.IsNULL() )
        {
            scheduler->Stop();
            taskManager.UnregisterJobScheduler( E2E_HOST_NAME );
            scheduler.Unlink();
        }
        if ( registered )
            taskManager.UnregisterTaskConsumerFactory( CE2ETask::TaskType );
        registered = false;
    }
};

/*-------------------------------------------------------------------------*/

/** The serving side over drn: an RPC endpoint per accepted transport, the server sees the given roles */
class CE2EServer : public CORE::CObserver
{
    public:
    COMCORE::CRpcServiceSetPtr services;
    CORE::CString::StringSet roles;
    CORE::CTSharedPtr< DRN::CDRNRpcTransportListener, MT::CMutex > listener;
    GUCEF::vector< COMCORE::CRpcEndpointPtr > endpoints;
    GUCEF::vector< COMCORE::CIRpcTransportPtr > transports;

    CE2EServer( void ) : CORE::CObserver(), services( COMCORE::CRpcServiceSet::Create() ), roles(), listener(), endpoints(), transports() {}
    virtual ~CE2EServer() { Stop(); SignalUpcomingObserverDestruction(); }

    bool Start( DRN::CDRNSession& session )
    {
        listener = CORE::CTSharedPtr< DRN::CDRNRpcTransportListener, MT::CMutex >( GUCEF_NEW DRN::CDRNRpcTransportListener( session ) );
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
        COMCORE::CRpcTransportAcceptedEventData* data = static_cast< COMCORE::CRpcTransportAcceptedEventData* >( eventdata );
        CRpcLossyTransport* wire = GUCEF_NULL;
        COMCORE::CIRpcTransportPtr transport = CRpcLossyTransport::Wrap( data->transport, &wire );
        COMCORE::CRpcPrincipal principal = data->transport->GetPeerPrincipal();
        principal.roles = roles;
        principal.isAuthenticated = true;
        wire->hasPrincipalOverride = true;
        wire->principalOverride = principal;
        COMCORE::CRpcEndpointSettings settings;
        settings.nodeName = "e2e-host";
        COMCORE::CRpcEndpointPtr endpoint = COMCORE::CRpcEndpoint::Create( CORE::CCoreGlobal::Instance()->GetPulseGenerator(), settings );
        endpoint->SetServiceSet( services );
        endpoint->Start( transport, false );
        endpoints.push_back( endpoint );
        transports.push_back( transport );
    }
};

/*-------------------------------------------------------------------------*/

/** Two members of a DRN session on nodes of their own (loopback tcp, encrypted links), pumped by this thread */
class CE2EGroup
{
    public:
    DRN::CDRNNode* nodes[ 2 ];
    DRN::CDRNSession* sessions[ 2 ];
    DRN::CDRNSession* registered;

    CE2EGroup( void ) : registered( GUCEF_NULL )
    {
        nodes[ 0 ] = nodes[ 1 ] = GUCEF_NULL;
        sessions[ 0 ] = sessions[ 1 ] = GUCEF_NULL;
    }

    ~CE2EGroup()
    {
        Register( GUCEF_NULL );
        for ( UInt32 i=0; i<2; ++i )
        {
            if ( GUCEF_NULL != nodes[ i ] )
                nodes[ i ]->Stop();
            GUCEF_DELETE nodes[ i ];
            nodes[ i ] = GUCEF_NULL;
        }
    }

    bool Start( void )
    {
        DRN::CDRNEndpoint hostEndpoint;
        for ( UInt32 i=0; i<2; ++i )
        {
            DRN::CDRNNodeSettings nodeSettings;
            nodeSettings.echoIntervalMs = 50;
            nodeSettings.pulseIntervalMs = 1;
            nodes[ i ] = GUCEF_NEW DRN::CDRNNode();
            nodes[ i ]->SetSettings( nodeSettings );
            sessions[ i ] = nodes[ i ]->CreateSession();
            UInt16 port = TESTFW::FindFreeTcpAndUdpPort( 0 );
            DRN::CDRNEndpoint endpoint( DRN::CDRNEndpoint::SchemeForChannelType( "tcp" ) + "://127.0.0.1:" + CORE::UInt32ToString( port ) );
            if ( !nodes[ i ]->Listen( endpoint ) )
                return false;
            DRN::CDRNSessionSettings sessionSettings;
            sessionSettings.sessionName = E2E_SESSION_NAME;
            if ( 0 == i )
            {
                hostEndpoint = endpoint;
                if ( !sessions[ 0 ]->Host( sessionSettings ) )
                    return false;
            }
            else
            if ( !sessions[ 1 ]->Join( DRN::CDRNEndpointList( hostEndpoint ), sessionSettings ) )
            {
                return false;
            }
        }
        for ( UInt32 waited=0; waited<15000; waited += 5 )
        {
            if ( sessions[ 0 ]->IsActive() && sessions[ 1 ]->IsActive() && 2 == sessions[ 0 ]->GetMemberCount() && 2 == sessions[ 1 ]->GetMemberCount() )
                return true;
            PumpFor( 5 );
        }
        return false;
    }

    /** The session the registry of gucefDRN finds by the session name (the host publishes there, the viewer binds there) */
    bool Register( DRN::CDRNSession* session )
    {
        DRN::CDRNSessionRegistry& registry = DRN::CDRNGlobal::Instance()->GetSessionRegistry();
        if ( GUCEF_NULL != registered )
            registry.Unregister( *registered );
        registered = session;
        return GUCEF_NULL == session || registry.Register( *session );
    }
};

/*-------------------------------------------------------------------------*/

/** The text of a cell of the row with the key, "(no such row)" without such a row */
CORE::CString
CellOf( DRNUTILSGUI::CTableGridBinding& binding, const CORE::CString& keyText, const CORE::CString& column )
{
    for ( UInt32 row=0; row<binding.GetRowCount(); ++row )
    {
        CORE::CString key;
        if ( binding.GetCellText( row, binding.GetKeyColumn(), key ) && key == keyText )
        {
            CORE::CString text;
            binding.GetCellText( row, column, text );
            return text;
        }
    }
    return "(no such row)";
}

/*-------------------------------------------------------------------------*/

/** The first run of the job in the runs grid, "(no such row)" without one */
CORE::CString
StateOfRunOfJob( DRNUTILSGUI::CTableGridBinding& runs, const CORE::CString& jobId )
{
    for ( UInt32 row=0; row<runs.GetRowCount(); ++row )
    {
        CORE::CString job;
        if ( runs.GetCellText( row, "jobId", job ) && job == jobId )
        {
            CORE::CString state;
            runs.GetCellText( row, "state", state );
            return state;
        }
    }
    return "(no such row)";
}

/*-------------------------------------------------------------------------*/

/** Pumps until the form's grids show the jobs, at most the time */
bool
PumpUntilJobRows( DRNUTILSGUI::CSchedulerForm& form, GUI::CGUIContext* context, UInt32 rows, UInt32 timeoutMs )
{
    for ( UInt32 waited=0; waited<timeoutMs; waited += 10 )
    {
        if ( form.GetJobsBinding().GetRowCount() >= rows )
        {
            context->FlushChanges();
            return true;
        }
        PumpFor( 10 );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/** Pumps until the cell of the row with the key holds the text, at most the time */
bool
PumpUntilCellHas( DRNUTILSGUI::CTableGridBinding& binding, const CORE::CString& keyText, const CORE::CString& column, const CORE::CString& part, UInt32 timeoutMs )
{
    for ( UInt32 waited=0; waited<timeoutMs; waited += 10 )
    {
        if ( CellOf( binding, keyText, column ).HasSubstr( part ) >= 0 )
            return true;
        PumpFor( 10 );
    }
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "End to end: the cell " << column << " of " << keyText << " shows \"" << CellOf( binding, keyText, column ) << "\", not \"" << part << "\"" );
    return false;
}

/*-------------------------------------------------------------------------*/

/** Pumps until the first run of the job shows the state, at most the time */
bool
PumpUntilRunState( DRNUTILSGUI::CSchedulerForm& form, const CORE::CString& jobId, const CORE::CString& state, UInt32 timeoutMs )
{
    for ( UInt32 waited=0; waited<timeoutMs; waited += 10 )
    {
        if ( state == StateOfRunOfJob( form.GetRunsBinding(), jobId ) )
            return true;
        PumpFor( 10 );
    }
    return false;
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformEndToEndTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "DrnUtilsGui.EndToEnd" );

    GUCEF_TESTFW_TESTCASE( "End to end, local: the scheduler form bound to CSchedulerSnapshotTable of a real CJobScheduler shows its jobs and lanes and follows a run" )
    try
    {
        CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        TThreadId previousDriver = pulseGenerator->GetPulseDriverThreadId();
        pulseGenerator->SetPulseDriverThreadId( MT::GetCurrentThreadID() );
        CE2EScheduler scheduler;
        bool started = scheduler.Start();
        ASSERT_TRUE( started );
        if ( started )
        {
            GUI::TGuiContextPtr contextPtr = CreateFormContext();
            GUI::CGUIContext* context = ContextOf( contextPtr );
            REQUIRE_TRUE( GUCEF_NULL != context );
            DRNUTILS::CJobSchedulerSource source( scheduler.scheduler, pulseGenerator );
            DRNUTILS::CSchedulerTableSet tables;
            CORE::CString error;
            REQUIRE_TRUE( DRNUTILS::CDrnUtilsTableBinder::OpenLocalSchedulerTables( source, pulseGenerator, tables, error ) );
            {
                DRNUTILSGUI::CSchedulerForm form( context );
                DRNUTILSGUI::CTimeDisplaySettings display;
                display.SetDisplayTimeZone( "utc" );
                form.SetTimeDisplay( display );
                form.SetTables( tables );
                REQUIRE_TRUE( form.Open() );
                ASSERT_TRUE( PumpUntilJobRows( form, context, 2, 5000 ) );
                ASSERT_TRUE( 2 == form.GetJobsBinding().GetRowCount() && 1 == form.GetLanesBinding().GetRowCount() );
                ASSERT_TRUE( "Fetch quotes" == CellOf( form.GetJobsBinding(), "quotes", "title" ) );
                ASSERT_TRUE( "Rebuild index" == CellOf( form.GetJobsBinding(), "rebuild", "title" ) );
                ASSERT_TRUE( PumpUntilCellHas( form.GetJobsBinding(), "rebuild", "nextDueUtc", " UTC", 5000 ) );
                ASSERT_TRUE( "POOL" == CellOf( form.GetJobsBinding(), "quotes", "executionTarget" ) );
                ASSERT_TRUE( "default" == CellOf( form.GetLanesBinding(), "default", "lane" ) );
                GUI::CGridView* jobsGrid = form.GetWidgetAs< GUI::CGridView >( "JobsGrid" );
                ASSERT_TRUE( GUCEF_NULL != jobsGrid && 2 == jobsGrid->GetRowCount() );

                // a run of the scheduler shows up in the runs grid and ends there
                CORE::SJobControlCommand runNow;
                runNow.action = CORE::JOBCONTROLACTION_RUN_NOW;
                runNow.targetKind = CORE::JOBCONTROLTARGET_JOB;
                runNow.targetId = "quotes";
                CORE::SJobControlResult result;
                ASSERT_TRUE( CORE::JOBCONTROLSTATUS_OK == scheduler.scheduler->Control( runNow, result ) );
                ASSERT_TRUE( PumpUntilRunState( form, "quotes", "SUCCEEDED", 10000 ) );
                form.Close();
            }
            tables.Clear();
        }
        scheduler.Stop();
        pulseGenerator->SetPulseDriverThreadId( previousDriver );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "End to end over drn: a real CJobScheduler, the DRNUTILS host, an encrypted DRN link, the client facade and the scheduler form on the replicas (svc/jobs, svc/lanes, svc/runs) through the table binder: the job rows, the job detail, run now from the form" )
    try
    {
        CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        TThreadId previousDriver = pulseGenerator->GetPulseDriverThreadId();
        pulseGenerator->SetPulseDriverThreadId( MT::GetCurrentThreadID() );
        if ( !DRN::CDRNCrypto::IsKeyAgreementAvailable() )
            LoadTestPlugin( "codecspluginMONOCYPHER", "test" );
        REQUIRE_TRUE( DRN::CDRNCrypto::IsKeyAgreementAvailable() && DRN::CDRNCrypto::IsSignatureAvailable() );
        CE2EScheduler scheduler;
        bool started = scheduler.Start();
        ASSERT_TRUE( started );
        if ( started )
        {
            CE2EGroup group;
            REQUIRE_TRUE( group.Start() );

            // the host: the scheduler exposed for control and published as svc/jobs, svc/lanes, svc/runs
            DRNUTILS::CJobSchedulerSource source( scheduler.scheduler, pulseGenerator );
            DRNUTILS::CSchedulerHostSettings hostSettings;
            hostSettings.SetExposure( DRNUTILS::DRNUTILS_EXPOSURE_CONTROL );
            CORE::TJobControlActionSet allowed;
            allowed.insert( CORE::JOBCONTROLACTION_RUN_NOW );
            hostSettings.SetAllowedActions( allowed );
            hostSettings.SetTablePrefix( "svc" );
            DRNUTILS::CSchedulerHost host( source, hostSettings, pulseGenerator );
            CE2EServer server;
            server.roles.insert( "scheduler.view" );
            server.roles.insert( "scheduler.control" );
            CORE::CString error;
            REQUIRE_TRUE( host.Expose( server.services, error ) );
            REQUIRE_TRUE( group.Register( group.sessions[ 0 ] ) );
            REQUIRE_TRUE( host.PublishTables( E2E_SESSION_NAME, error ) );
            REQUIRE_TRUE( server.Start( *group.sessions[ 0 ] ) );

            // the viewer: an RPC endpoint over the drn link, the client facade on it
            COMCORE::CRpcEndpointSettings viewerSettings;
            viewerSettings.nodeName = "e2e-viewer";
            COMCORE::CRpcEndpointPtr viewer = COMCORE::CRpcEndpoint::Create( pulseGenerator, viewerSettings );
            viewer->AddWantedContract( "gucef.drnutils.scheduler", 1 );
            CORE::CTSharedPtr< DRN::CDRNRpcTransport, MT::CMutex > drn( GUCEF_NEW DRN::CDRNRpcTransport( *group.sessions[ 1 ], group.sessions[ 0 ]->GetLocalPeerId() ) );
            REQUIRE_TRUE( drn->Connect( CORE::CUri() ) );
            COMCORE::CIRpcTransportPtr transport;
            transport = drn;
            REQUIRE_TRUE( viewer->Start( transport, true ) );
            bool bound = false;
            for ( UInt32 waited=0; waited<10000 && !bound; waited += 5 )
            {
                bound = viewer->IsContractBound( "gucef.drnutils.scheduler", 1 );
                if ( !bound )
                    PumpFor( 5 );
            }
            REQUIRE_TRUE( bound );
            DRNUTILS::CSchedulerClient client( pulseGenerator );
            client.SetEndpoint( viewer );

            // the replicas of the viewer's member through the table binder
            bool announced = false;
            for ( UInt32 waited=0; waited<10000 && !announced; waited += 5 )
            {
                announced = GUCEF_NULL != group.sessions[ 1 ]->FindGrid( "svc/runs" ) && GUCEF_NULL != group.sessions[ 1 ]->FindGrid( "svc/jobs" ) &&
                            GUCEF_NULL != group.sessions[ 1 ]->FindGrid( "svc/lanes" );
                if ( !announced )
                    PumpFor( 5 );
            }
            REQUIRE_TRUE( announced );
            REQUIRE_TRUE( group.Register( group.sessions[ 1 ] ) );
            DRNUTILS::CSchedulerTableSet tables;
            REQUIRE_TRUE( DRNUTILS::CDrnUtilsTableBinder::OpenSchedulerTables( E2E_SESSION_NAME, "svc", tables, error ) );
            REQUIRE_TRUE( tables.IsComplete() );

            GUI::TGuiContextPtr contextPtr = CreateFormContext();
            GUI::CGUIContext* context = ContextOf( contextPtr );
            REQUIRE_TRUE( GUCEF_NULL != context );
            {
                DRNUTILSGUI::CSchedulerForm form( context );
                DRNUTILSGUI::CTimeDisplaySettings display;
                display.SetDisplayTimeZone( "utc" );
                form.SetTimeDisplay( display );
                form.SetTables( tables );
                form.SetClient( &client );
                REQUIRE_TRUE( form.Open() );

                // the job rows arrive on the replica and show in the grid
                ASSERT_TRUE( PumpUntilJobRows( form, context, 2, 15000 ) );
                ASSERT_TRUE( 2 == form.GetJobsBinding().GetRowCount() );
                ASSERT_TRUE( "Fetch quotes" == CellOf( form.GetJobsBinding(), "quotes", "title" ) );
                ASSERT_TRUE( "Rebuild index" == CellOf( form.GetJobsBinding(), "rebuild", "title" ) );
                ASSERT_TRUE( CE2ETask::TaskType == CellOf( form.GetJobsBinding(), "quotes", "taskType" ) );
                ASSERT_TRUE( PumpUntilCellHas( form.GetJobsBinding(), "rebuild", "nextDueUtc", " UTC", 5000 ) );
                GUI::CGridView* jobsGrid = form.GetWidgetAs< GUI::CGridView >( "JobsGrid" );
                ASSERT_TRUE( GUCEF_NULL != jobsGrid && 2 == jobsGrid->GetRowCount() );
                for ( UInt32 waited=0; waited<5000 && form.GetHostInfo().hostName.IsNULLOrEmpty(); waited += 10 )
                    PumpFor( 10 );
                ASSERT_TRUE( E2E_HOST_NAME == form.GetHostInfo().hostName );
                ASSERT_TRUE( form.GetStatusText().HasSubstr( E2E_HOST_NAME ) >= 0 && form.GetStatusText().HasSubstr( ", connected" ) >= 0 );

                // the detail of the selected job comes from Scheduler/GetJob (C-2)
                form.SelectJob( "quotes" );
                DRNUTILS::SSchedulerJobDetailAnswer detail;
                bool haveDetail = false;
                for ( UInt32 waited=0; waited<5000 && !haveDetail; waited += 10 )
                {
                    haveDetail = form.GetSelectedJobDetail( detail );
                    if ( !haveDetail )
                        PumpFor( 10 );
                }
                REQUIRE_TRUE( haveDetail );
                ASSERT_TRUE( DRNUTILS::DRNUTILS_STATUS_OK == detail.status && "quotes" == detail.detail.job.jobId );
                ASSERT_TRUE( "Fetches the quotes of the watch list" == detail.detail.job.description );
                ASSERT_TRUE( form.GetJobDetailsText().HasSubstr( "Allowed actions: RUN_NOW" ) >= 0 );

                // run now from the form: the command goes over the link, the run arrives in the runs replica
                ASSERT_TRUE( form.IsActionEnabled( CORE::JOBCONTROLACTION_RUN_NOW ) );
                ASSERT_TRUE( form.RunSelectedJobNow( "end to end" ) );
                for ( UInt32 waited=0; waited<10000 && form.GetLastCommandText().HasSubstr( "done" ) < 0; waited += 10 )
                    PumpFor( 10 );
                if ( form.GetLastCommandText().HasSubstr( "done" ) < 0 )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "End to end over drn: the run-now ended with \"" << form.GetLastCommandText() << "\"" );
                ASSERT_TRUE( form.GetLastCommandText().HasSubstr( "done" ) >= 0 );
                ASSERT_TRUE( PumpUntilRunState( form, "quotes", "SUCCEEDED", 15000 ) );
                context->FlushChanges();
                WriteShot( CaptureFrame( context ), "SchedulerForm_EndToEnd" );
                form.SetClient( GUCEF_NULL );
                form.Close();
            }
            tables.Clear();
            group.Register( group.sessions[ 0 ] );
            client.SetEndpoint( COMCORE::CRpcEndpointPtr() );
            viewer->Close( false );
            PumpFor( 50 );
            viewer.Unlink();
            drn.Unlink();
            server.Stop();
            host.StopPublishing();
            host.Withdraw();
            PumpFor( 50 );
        }
        scheduler.Stop();
        pulseGenerator->SetPulseDriverThreadId( previousDriver );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/
