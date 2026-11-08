/*
 *  gucefWEB_TestApp: GUCEF test application for the WEB module
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

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CSERVICEHEALTHREGISTRY_H
#include "gucefCORE_CServiceHealthRegistry.h"
#define GUCEF_CORE_CSERVICEHEALTHREGISTRY_H
#endif /* GUCEF_CORE_CSERVICEHEALTHREGISTRY_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_COMCORE_H
#include "gucefCOMCORE.h"
#define GUCEF_COMCORE_H
#endif /* GUCEF_COMCORE_H ? */

#ifndef GUCEF_WEB_H
#include "gucefWEB.h"
#define GUCEF_WEB_H
#endif /* GUCEF_WEB_H ? */

#ifndef GUCEF_WEB_CSERVICEHEALTHHTTPENDPOINTS_H
#include "gucefWEB_CServiceHealthHttpEndpoints.h"
#define GUCEF_WEB_CSERVICEHEALTHHTTPENDPOINTS_H
#endif /* GUCEF_WEB_CSERVICEHEALTHHTTPENDPOINTS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_WEB_TESTAPP_TESTHTTPRAWCLIENT_H
#include "TestHttpRawClient.h"
#define GUCEF_WEB_TESTAPP_TESTHTTPRAWCLIENT_H
#endif /* GUCEF_WEB_TESTAPP_TESTHTTPRAWCLIENT_H ? */

#include "TestServiceHealthHttpEndpoints.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** A readiness check whose answer the test sets; it can throw or block */
class CHealthEndpointTestCheck : public CORE::CIServiceHealthCheck
{
    public:

    explicit CHealthEndpointTestCheck( const char* name )
        : CORE::CIServiceHealthCheck()
        , state( CORE::HEALTH_PASS )
        , detail()
        , throws( false )
        , blockMs( 0 )
        , entered( false )
        , m_name( name )
    {
    }

    virtual const CORE::CString& GetName( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_name; }

    virtual CORE::TServiceHealthState Check( CORE::CString& outDetail ) GUCEF_VIRTUAL_OVERRIDE
    {
        entered.Store( true );
        if ( blockMs > 0 )
            MT::PrecisionDelay( blockMs );
        if ( throws )
            throw CORE::CString( "boom" );
        outDetail = detail;
        return state;
    }

    CORE::TServiceHealthState state;
    CORE::CString detail;
    bool throws;
    UInt32 blockMs;
    MT::CTAtomic< bool > entered;

    private:

    CORE::CString m_name;
};

/*-------------------------------------------------------------------------*/

/** Evaluates the checks of a registry once on a thread of the task manager, as a pulse thread of its own would */
class CHealthEvaluationTask : public CORE::CTaskConsumer
{
    public:

    explicit CHealthEvaluationTask( CORE::CServiceHealthRegistry& registry ) : CORE::CTaskConsumer(), done( false ), m_registry( registry ) {}

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return "TestServiceHealthHttpEndpoints::CHealthEvaluationTask"; }

    virtual bool OnTaskCycle( CORE::CTaskPtr ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_registry.EvaluateChecks();
        done.Store( true );
        return true;
    }

    virtual void OnTaskEnded( CORE::CTaskPtr, bool ) GUCEF_VIRTUAL_OVERRIDE {}

    MT::CTAtomic< bool > done;

    private:

    CORE::CServiceHealthRegistry& m_registry;
};

typedef CORE::CTSharedPtr< CHealthEvaluationTask, MT::CMutex > THealthEvaluationTaskPtr;

/*-------------------------------------------------------------------------*/

/** A HTTP server on the loopback interface (or the given address) and the application's pulse generator, with the default router */
class CHealthTestServer
{
    public:

    explicit CHealthTestServer( const char* bindAddress = "127.0.0.1" )
        : m_server( GUCEF_NULL )
    {
        WEB::CHttpServerSettings settings;
        settings.bindAddress = bindAddress;
        m_server = GUCEF_NEW WEB::CHTTPServer( CORE::CCoreGlobal::Instance()->GetPulseGenerator(), GUCEF_NULL, &settings );
    }

    ~CHealthTestServer()
    {
        m_server->Close();
        GUCEF_DELETE m_server;
    }

    bool Start( void ) { return m_server->ListenOnPort( 0 ); }
    UInt16 GetPort( void ) const { return m_server->GetBoundPort(); }
    WEB::CHTTPServer& GetServer( void ) { return *m_server; }

    private:

    WEB::CHTTPServer* m_server;
};

/*-------------------------------------------------------------------------*/

/** One request on a connection of its own; false when no complete response came */
bool
HealthRequest( UInt16 port, const char* method, const CORE::CString& path, STestHttpResponse& response )
{
    CTestHttpRawClient client;
    if ( !client.Connect( port ) )
        return false;
    if ( !client.Send( CORE::CString( method ) + " " + path + " HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n" ) )
        return false;
    if ( !client.WaitForResponses( 1, 5000 ) )
        return false;
    TTestHttpResponseVector responses;
    if ( 1 != client.GetResponses( responses ) )
        return false;
    response = responses[ 0 ];
    return true;
}

/*-------------------------------------------------------------------------*/

/** The status of a GET, 0 when there was no response */
Int32
HealthStatus( UInt16 port, const CORE::CString& path, CORE::CString* body = GUCEF_NULL )
{
    STestHttpResponse response;
    if ( !HealthRequest( port, "GET", path, response ) )
        return 0;
    if ( GUCEF_NULL != body )
        *body = response.body;
    return response.status;
}

/*-------------------------------------------------------------------------*/

bool
BodyHas( const CORE::CString& body, const CORE::CString& text )
{
    if ( body.HasSubstr( text ) >= 0 )
        return true;
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ServiceHealthHttpEndpoints test: \"" << text << "\" is not in " << body );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
WaitForFlag( const MT::CTAtomic< bool >& flag, UInt32 timeoutInMs )
{
    UInt32 waited = 0;
    while ( !flag.Load() && waited < timeoutInMs )
    {
        TestHttpPumpFor( 2 );
        waited += 3;
    }
    return flag.Load();
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformServiceHealthHttpEndpointsTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING SERVICE HEALTH HTTP ENDPOINT TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "ServiceHealthHttpEndpoints" );

    GUCEF_TESTFW_TESTCASE( "Test: The probes answer 503 before the start and 200 after it, with the state in the body" )
        try
        {
            CORE::PulseGeneratorPtr registryPulse = CORE::CPulseGenerator::CreateSharedObj();
            CORE::CServiceHealthRegistry registry( registryPulse );
            CHealthEndpointTestCheck database( "database" );
            ASSERT_TRUE( registry.RegisterReadinessCheck( &database ) );

            CHealthTestServer server;
            WEB::CServiceHealthHttpEndpoints endpoints( registry );
            ASSERT_TRUE( endpoints.RegisterWith( server.GetServer() ) && endpoints.IsRegistered() );
            ASSERT_TRUE( server.Start() );
            const UInt16 port = server.GetPort();

            // nothing evaluated, nothing started
            CORE::CString body;
            ASSERT_TRUE( 503 == HealthStatus( port, "/health/live", &body ) );
            ASSERT_TRUE( BodyHas( body, "\"status\":\"fail\"" ) && BodyHas( body, "\"stale\":true" ) );
            ASSERT_TRUE( 503 == HealthStatus( port, "/health/ready" ) );
            ASSERT_TRUE( 503 == HealthStatus( port, "/health/started" ) );

            // alive, not started: live passes, the others do not
            registry.EvaluateChecks();
            ASSERT_TRUE( 200 == HealthStatus( port, "/health/live", &body ) );
            ASSERT_TRUE( BodyHas( body, "{\"status\":\"pass\",\"checks\":[]}" ) );
            ASSERT_TRUE( 503 == HealthStatus( port, "/health/ready" ) );
            ASSERT_TRUE( 503 == HealthStatus( port, "/health/started" ) );

            registry.SetStarted( true );
            ASSERT_TRUE( 200 == HealthStatus( port, "/health/started" ) );
            ASSERT_TRUE( 200 == HealthStatus( port, "/health/ready", &body ) );
            ASSERT_TRUE( BodyHas( body, "\"status\":\"pass\"" ) && BodyHas( body, "\"name\":\"database\"" ) && BodyHas( body, "\"since\":\"" ) );

            // WARN passes, FAIL does not
            database.state = CORE::HEALTH_WARN;
            database.detail = "slow";
            registry.EvaluateChecks();
            ASSERT_TRUE( 200 == HealthStatus( port, "/health/ready", &body ) );
            ASSERT_TRUE( BodyHas( body, "\"status\":\"warn\"" ) && BodyHas( body, "\"detail\":\"slow\"" ) );
            database.state = CORE::HEALTH_FAIL;
            database.detail = "connection refused";
            registry.EvaluateChecks();
            ASSERT_TRUE( 503 == HealthStatus( port, "/health/ready", &body ) );
            ASSERT_TRUE( BodyHas( body, "\"detail\":\"connection refused\"" ) );
            // liveness never depends on a check
            ASSERT_TRUE( 200 == HealthStatus( port, "/health/live" ) );

            endpoints.Unregister();
            ASSERT_FALSE( endpoints.IsRegistered() );
            registry.UnregisterReadinessCheck( &database );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: HEAD gets the status without a body, other methods 405, /info the info as JSON, a JSON content type and no caching" )
        try
        {
            CORE::PulseGeneratorPtr registryPulse = CORE::CPulseGenerator::CreateSharedObj();
            CORE::CServiceHealthRegistry registry( registryPulse );
            registry.EvaluateChecks();
            registry.SetStarted( true );
            CORE::CValueList info;
            info.Set( "version", CORE::CVariant( CORE::CString( "1.2.3" ) ) );
            info.Set( "note", CORE::CVariant( CORE::CString( "a \"quoted\" word" ) ) );
            registry.SetInfo( info );

            CHealthTestServer server;
            WEB::CServiceHealthHttpEndpoints endpoints( registry );
            ASSERT_TRUE( endpoints.RegisterWith( server.GetServer() ) );
            ASSERT_TRUE( server.Start() );
            const UInt16 port = server.GetPort();

            STestHttpResponse response;
            ASSERT_TRUE( HealthRequest( server.GetPort(), "HEAD", "/health/ready", response ) );
            ASSERT_TRUE( 200 == response.status && response.body.IsNULLOrEmpty() );
            registry.SetStarted( false );
            ASSERT_TRUE( HealthRequest( server.GetPort(), "HEAD", "/health/ready", response ) );
            ASSERT_TRUE( 503 == response.status && response.body.IsNULLOrEmpty() );

            ASSERT_TRUE( HealthRequest( port, "GET", "/health/live", response ) );
            ASSERT_TRUE( response.Header( "Content-Type" ).HasSubstr( "application/json" ) >= 0 );
            ASSERT_TRUE( response.Header( "Cache-Control" ).HasSubstr( "no-store" ) >= 0 );

            ASSERT_TRUE( HealthRequest( port, "POST", "/health/live", response ) );
            ASSERT_TRUE( 405 == response.status );

            CORE::CString body;
            ASSERT_TRUE( 200 == HealthStatus( port, "/info", &body ) );
            ASSERT_TRUE( BodyHas( body, "\"version\":\"1.2.3\"" ) && BodyHas( body, "\"note\":\"a \\\"quoted\\\" word\"" ) );
            CORE::CValueList single;
            single.Set( "note", CORE::CVariant( CORE::CString( "a \"quoted\" word" ) ) );
            ASSERT_TRUE( "{\"note\":\"a \\\"quoted\\\" word\"}" == WEB::CServiceHealthHttpEndpoints::BuildInfoBody( single ) );

            // what is not one of the paths goes to the router as before
            ASSERT_TRUE( 200 != HealthStatus( port, "/health/live/x" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A check that throws fails readiness, and the details are redacted before they are written" )
        try
        {
            CORE::PulseGeneratorPtr registryPulse = CORE::CPulseGenerator::CreateSharedObj();
            CORE::CServiceHealthRegistry registry( registryPulse );
            CHealthEndpointTestCheck thrower( "thrower" );
            thrower.throws = true;
            CHealthEndpointTestCheck leaky( "leaky" );
            leaky.state = CORE::HEALTH_WARN;
            leaky.detail = "retrying https://db.local/v1?key=SECRETKEY123 with Authorization: Bearer abcdef";
            ASSERT_TRUE( registry.RegisterReadinessCheck( &thrower ) && registry.RegisterReadinessCheck( &leaky ) );
            registry.SetStarted( true );
            registry.EvaluateChecks();

            CHealthTestServer server;
            WEB::CServiceHealthHttpEndpoints endpoints( registry );
            ASSERT_TRUE( endpoints.RegisterWith( server.GetServer() ) );
            ASSERT_TRUE( server.Start() );

            CORE::CString body;
            ASSERT_TRUE( 503 == HealthStatus( server.GetPort(), "/health/ready", &body ) );
            ASSERT_TRUE( BodyHas( body, "\"detail\":\"the check threw an exception\"" ) );
            ASSERT_TRUE( body.HasSubstr( "SECRETKEY123" ) < 0 && body.HasSubstr( "abcdef" ) < 0 );
            ASSERT_TRUE( BodyHas( body, "key=***" ) );

            registry.UnregisterReadinessCheck( &thrower );
            registry.UnregisterReadinessCheck( &leaky );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A server that listens on every address leaves the checks out of the health bodies by default, the status codes stay" )
        try
        {
            CORE::PulseGeneratorPtr registryPulse = CORE::CPulseGenerator::CreateSharedObj();
            CORE::CServiceHealthRegistry registry( registryPulse );
            CHealthEndpointTestCheck failing( "databaseConnection" );
            failing.state = CORE::HEALTH_FAIL;
            failing.detail = "cannot reach db.internal:5432";
            ASSERT_TRUE( registry.RegisterReadinessCheck( &failing ) );
            registry.SetStarted( true );
            registry.EvaluateChecks();

            // an empty bind address listens on every interface: the clients need not be of this machine
            CHealthTestServer server( "" );
            WEB::CServiceHealthHttpEndpoints endpoints( registry );
            ASSERT_TRUE( endpoints.RegisterWith( server.GetServer() ) );
            ASSERT_TRUE( server.Start() );

            CORE::CString body;
            ASSERT_TRUE( 503 == HealthStatus( server.GetPort(), "/health/ready", &body ) );
            ASSERT_TRUE( BodyHas( body, "\"status\":\"fail\"" ) );
            ASSERT_TRUE( body.HasSubstr( "databaseConnection" ) < 0 );
            ASSERT_TRUE( body.HasSubstr( "db.internal" ) < 0 );
            ASSERT_TRUE( body.HasSubstr( "\"checks\"" ) < 0 );
            ASSERT_TRUE( 200 == HealthStatus( server.GetPort(), "/health/live", &body ) );
            ASSERT_TRUE( BodyHas( body, "\"status\":\"pass\"" ) );

            registry.UnregisterReadinessCheck( &failing );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The check details setting decides per bind address: automatic only on loopback, included and omitted everywhere" )
        try
        {
            typedef WEB::CServiceHealthHttpEndpoints TEndpoints;
            ASSERT_TRUE( TEndpoints::IncludesCheckDetails( TEndpoints::CHECKDETAILS_AUTOMATIC, "127.0.0.1" ) );
            ASSERT_TRUE( TEndpoints::IncludesCheckDetails( TEndpoints::CHECKDETAILS_AUTOMATIC, "127.10.20.30" ) );
            ASSERT_FALSE( TEndpoints::IncludesCheckDetails( TEndpoints::CHECKDETAILS_AUTOMATIC, "" ) );
            ASSERT_FALSE( TEndpoints::IncludesCheckDetails( TEndpoints::CHECKDETAILS_AUTOMATIC, "0.0.0.0" ) );
            ASSERT_FALSE( TEndpoints::IncludesCheckDetails( TEndpoints::CHECKDETAILS_AUTOMATIC, "192.168.1.5" ) );
            ASSERT_FALSE( TEndpoints::IncludesCheckDetails( TEndpoints::CHECKDETAILS_AUTOMATIC, "127.0.0" ) );
            ASSERT_FALSE( TEndpoints::IncludesCheckDetails( TEndpoints::CHECKDETAILS_AUTOMATIC, "127.0.0.1.5" ) );
            ASSERT_FALSE( TEndpoints::IncludesCheckDetails( TEndpoints::CHECKDETAILS_AUTOMATIC, "127.0.0.256" ) );
            ASSERT_FALSE( TEndpoints::IncludesCheckDetails( TEndpoints::CHECKDETAILS_AUTOMATIC, "localhost" ) );
            ASSERT_TRUE( TEndpoints::IncludesCheckDetails( TEndpoints::CHECKDETAILS_INCLUDED, "" ) );
            ASSERT_FALSE( TEndpoints::IncludesCheckDetails( TEndpoints::CHECKDETAILS_OMITTED, "127.0.0.1" ) );

            CORE::PulseGeneratorPtr registryPulse = CORE::CPulseGenerator::CreateSharedObj();
            CORE::CServiceHealthRegistry registry( registryPulse );
            CHealthEndpointTestCheck database( "database" );
            database.state = CORE::HEALTH_WARN;
            database.detail = "slow";
            ASSERT_TRUE( registry.RegisterReadinessCheck( &database ) );
            registry.SetStarted( true );
            registry.EvaluateChecks();
            CORE::CString body;

            {
                // every interface, but the deployment asks for the details
                CHealthTestServer server( "0.0.0.0" );
                TEndpoints endpoints( registry );
                ASSERT_TRUE( TEndpoints::CHECKDETAILS_AUTOMATIC == endpoints.GetCheckDetails() );
                endpoints.SetCheckDetails( TEndpoints::CHECKDETAILS_INCLUDED );
                ASSERT_TRUE( endpoints.RegisterWith( server.GetServer() ) );
                ASSERT_TRUE( server.Start() );
                ASSERT_TRUE( 200 == HealthStatus( server.GetPort(), "/health/ready", &body ) );
                ASSERT_TRUE( BodyHas( body, "\"name\":\"database\"" ) && BodyHas( body, "\"detail\":\"slow\"" ) );
            }
            {
                // loopback, but the details are never wanted
                CHealthTestServer server;
                TEndpoints endpoints( registry );
                endpoints.SetCheckDetails( TEndpoints::CHECKDETAILS_OMITTED );
                ASSERT_TRUE( endpoints.RegisterWith( server.GetServer() ) );
                ASSERT_TRUE( server.Start() );
                ASSERT_TRUE( 200 == HealthStatus( server.GetPort(), "/health/ready", &body ) );
                ASSERT_TRUE( BodyHas( body, "\"status\":\"warn\"" ) );
                ASSERT_TRUE( body.HasSubstr( "database" ) < 0 && body.HasSubstr( "\"checks\"" ) < 0 );
            }
            ASSERT_TRUE( TEndpoints::BuildHealthBody( CORE::CServiceHealthRegistry::SHealthReport(), false ).HasSubstr( "\"checks\"" ) < 0 );

            registry.UnregisterReadinessCheck( &database );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A check that blocks the thread of the registry fails live and ready within the staleness window while the server keeps answering" )
        try
        {
            CORE::PulseGeneratorPtr registryPulse = CORE::CPulseGenerator::CreateSharedObj();
            CORE::CServiceHealthRegistry registry( registryPulse );
            registry.SetStalenessInMs( 300 );
            CHealthEndpointTestCheck slow( "slow" );
            ASSERT_TRUE( registry.RegisterReadinessCheck( &slow ) );
            registry.SetStarted( true );
            registry.EvaluateChecks();

            CHealthTestServer server;
            WEB::CServiceHealthHttpEndpoints endpoints( registry );
            ASSERT_TRUE( endpoints.RegisterWith( server.GetServer() ) );
            ASSERT_TRUE( server.Start() );
            const UInt16 port = server.GetPort();
            ASSERT_TRUE( 200 == HealthStatus( port, "/health/ready" ) );

            // the next evaluation hangs for 2 s in the check, on a thread of its own
            slow.blockMs = 2000;
            CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( "TestServiceHealthHttpEndpoints" );
            THealthEvaluationTaskPtr task( GUCEF_NEW CHealthEvaluationTask( registry ) );
            CORE::CFutureResult started = pool->StartTaskWithConsumer( task );
            ASSERT_FALSE( started.HasNoFuture() );
            ASSERT_TRUE( WaitForFlag( slow.entered, 5000 ) );

            // the probes go to 503 once the cache is older than 300 ms, well before the check returns
            const UInt64 startTicks = MT::PrecisionTickCount();
            const Float64 ticksPerMs = MT::PrecisionTimerResolution() / 1000.0;
            Int32 readyStatus = 0;
            while ( ( MT::PrecisionTickCount() - startTicks ) / ticksPerMs < 1500 )
            {
                readyStatus = HealthStatus( port, "/health/ready" );
                if ( 503 == readyStatus )
                    break;
                TestHttpPumpFor( 20 );
            }
            ASSERT_TRUE( 503 == readyStatus );
            ASSERT_FALSE( task->done.Load() );
            CORE::CString body;
            ASSERT_TRUE( 503 == HealthStatus( port, "/health/live", &body ) );
            ASSERT_TRUE( BodyHas( body, "\"stale\":true" ) );

            // once the check returns the probes pass again
            ASSERT_TRUE( WaitForFlag( task->done, 5000 ) );
            ASSERT_TRUE( 200 == HealthStatus( port, "/health/live" ) );
            ASSERT_TRUE( 200 == HealthStatus( port, "/health/ready" ) );
            registry.UnregisterReadinessCheck( &slow );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The paths can be changed or left out, and a path that is taken leaves nothing registered" )
        try
        {
            CORE::PulseGeneratorPtr registryPulse = CORE::CPulseGenerator::CreateSharedObj();
            CORE::CServiceHealthRegistry registry( registryPulse );
            registry.EvaluateChecks();
            registry.SetStarted( true );

            CHealthTestServer server;
            WEB::CServiceHealthHttpEndpoints endpoints( registry );
            endpoints.SetPaths( "/k8s/live", "/k8s/ready", CORE::CString::Empty, CORE::CString::Empty );
            ASSERT_TRUE( endpoints.RegisterWith( server.GetServer() ) );
            ASSERT_FALSE( endpoints.RegisterWith( server.GetServer() ) );
            ASSERT_TRUE( server.Start() );
            ASSERT_TRUE( 200 == HealthStatus( server.GetPort(), "/k8s/live" ) );
            ASSERT_TRUE( 200 == HealthStatus( server.GetPort(), "/k8s/ready" ) );
            ASSERT_TRUE( 200 != HealthStatus( server.GetPort(), "/health/live" ) );

            // a second set on the same server: /k8s/ready is taken, so none of its paths stays registered
            WEB::CServiceHealthHttpEndpoints second( registry );
            second.SetPaths( "/other/live", "/k8s/ready", CORE::CString::Empty, CORE::CString::Empty );
            ASSERT_FALSE( second.RegisterWith( server.GetServer() ) );
            ASSERT_FALSE( second.IsRegistered() );
            ASSERT_TRUE( 200 != HealthStatus( server.GetPort(), "/other/live" ) );
            // and the first set still answers
            ASSERT_TRUE( 200 == HealthStatus( server.GetPort(), "/k8s/ready" ) );

            // the registry of the process is the default
            WEB::CServiceHealthHttpEndpoints processEndpoints;
            ASSERT_TRUE( processEndpoints.RegisterWith( server.GetServer() ) );
            CORE::CServiceHealthRegistry& processRegistry = CORE::CCoreGlobal::Instance()->GetServiceHealthRegistry();
            processRegistry.EvaluateChecks();
            processRegistry.SetStarted( true );
            ASSERT_TRUE( 200 == HealthStatus( server.GetPort(), "/health/started" ) );
            processRegistry.SetStarted( false );
            ASSERT_TRUE( 503 == HealthStatus( server.GetPort(), "/health/started" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
