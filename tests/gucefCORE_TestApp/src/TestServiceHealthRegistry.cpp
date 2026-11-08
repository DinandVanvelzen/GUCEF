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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

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

#ifndef GUCEF_CORE_CSERVICEHEALTHREGISTRY_H
#include "gucefCORE_CServiceHealthRegistry.h"
#define GUCEF_CORE_CSERVICEHEALTHREGISTRY_H
#endif /* GUCEF_CORE_CSERVICEHEALTHREGISTRY_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestServiceHealthRegistry.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::CORE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** A check whose answer the test sets; counts its calls. Can block for a while or throw */
class CTestHealthCheck : public CIServiceHealthCheck
{
    public:

    explicit CTestHealthCheck( const char* name )
        : CIServiceHealthCheck()
        , state( HEALTH_PASS )
        , detail()
        , throws( false )
        , blockMs( 0 )
        , calls( 0 )
        , entered( false )
        , finished( false )
        , m_name( name )
    {
    }

    virtual const CString& GetName( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_name; }

    virtual TServiceHealthState Check( CString& outDetail ) GUCEF_VIRTUAL_OVERRIDE
    {
        calls.FetchAdd( 1 );
        entered.Store( true );
        if ( blockMs > 0 )
            MT::PrecisionDelay( blockMs );
        finished.Store( true );
        if ( throws )
            throw CString( "boom" );
        outDetail = detail;
        return state;
    }

    TServiceHealthState state;
    CString detail;
    bool throws;
    UInt32 blockMs;
    MT::CTAtomic< UInt32 > calls;
    MT::CTAtomic< bool > entered;
    MT::CTAtomic< bool > finished;

    private:

    CString m_name;
};

/*-------------------------------------------------------------------------*/

/** Evaluates the checks of a registry once on a thread of the task manager */
class CEvaluateChecksTask : public CTaskConsumer
{
    public:

    explicit CEvaluateChecksTask( CServiceHealthRegistry& registry ) : CTaskConsumer(), done( false ), m_registry( registry ) {}

    virtual CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return "TestServiceHealthRegistry::CEvaluateChecksTask"; }

    virtual bool OnTaskCycle( CTaskPtr ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_registry.EvaluateChecks();
        done.Store( true );
        return true;
    }

    virtual void OnTaskEnded( CTaskPtr, bool ) GUCEF_VIRTUAL_OVERRIDE {}

    MT::CTAtomic< bool > done;

    private:

    CServiceHealthRegistry& m_registry;
};

typedef CTSharedPtr< CEvaluateChecksTask, MT::CMutex > TEvaluateChecksTaskPtr;

/*-------------------------------------------------------------------------*/

/** A check that unregisters a check (itself or another one) from within its own Check() */
class CUnregisteringHealthCheck : public CIServiceHealthCheck
{
    public:

    CUnregisteringHealthCheck( const char* name, CServiceHealthRegistry& registry )
        : CIServiceHealthCheck()
        , toUnregister( GUCEF_NULL )
        , state( HEALTH_FAIL )
        , detail( name )
        , calls( 0 )
        , m_registry( registry )
        , m_name( name )
    {
    }

    virtual const CString& GetName( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_name; }

    virtual TServiceHealthState Check( CString& outDetail ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++calls;
        if ( GUCEF_NULL != toUnregister )
            m_registry.UnregisterReadinessCheck( toUnregister );
        outDetail = detail;
        return state;
    }

    CIServiceHealthCheck* toUnregister;
    TServiceHealthState state;
    CString detail;
    UInt32 calls;

    private:

    CServiceHealthRegistry& m_registry;
    CString m_name;
};

/*-------------------------------------------------------------------------*/

bool
WaitForFlag( const MT::CTAtomic< bool >& flag, UInt32 timeoutInMs )
{
    UInt32 waited = 0;
    while ( !flag.Load() && waited < timeoutInMs )
    {
        MT::PrecisionDelay( 2 );
        waited += 2;
    }
    return flag.Load();
}

/*-------------------------------------------------------------------------*/

const CServiceHealthRegistry::SCheckOutcome*
FindOutcome( const CServiceHealthRegistry::SHealthReport& report, const char* name )
{
    for ( size_t i=0; i<report.checks.size(); ++i )
        if ( report.checks[ i ].name == name )
            return &report.checks[ i ];
    return GUCEF_NULL;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformServiceHealthRegistryTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( LOGLEVEL_NORMAL, "COMMENCING SERVICE HEALTH REGISTRY TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "ServiceHealthRegistry" );

    GUCEF_TESTFW_TESTCASE( "Test: Before anything is evaluated or started every probe fails; after that they pass" )
        try
        {
            PulseGeneratorPtr pulseGenerator = CPulseGenerator::CreateSharedObj();
            CServiceHealthRegistry registry( pulseGenerator );
            CServiceHealthRegistry::SHealthReport report;

            registry.GetLiveness( report );
            ASSERT_TRUE( HEALTH_FAIL == report.state && report.isStale && 0 == report.evaluatedAtUtcMs );
            registry.GetReadiness( report );
            ASSERT_FALSE( report.Passes() );
            registry.GetStartup( report );
            ASSERT_FALSE( report.Passes() );

            registry.EvaluateChecks();
            registry.GetLiveness( report );
            ASSERT_TRUE( HEALTH_PASS == report.state && !report.isStale && 0 != report.evaluatedAtUtcMs );
            // alive but not started: not ready
            registry.GetReadiness( report );
            ASSERT_TRUE( HEALTH_FAIL == report.state && !report.isStarted );

            registry.SetStarted( true );
            registry.GetStartup( report );
            ASSERT_TRUE( report.Passes() && report.isStarted );
            registry.GetReadiness( report );
            ASSERT_TRUE( HEALTH_PASS == report.state && report.checks.empty() );

            registry.SetStarted( false );
            registry.GetStartup( report );
            ASSERT_FALSE( report.Passes() );

            ASSERT_TRUE( "pass" == ServiceHealthStateToString( HEALTH_PASS ) && "warn" == ServiceHealthStateToString( HEALTH_WARN ) );
            ASSERT_TRUE( "fail" == ServiceHealthStateToString( HEALTH_FAIL ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Readiness is the worst of the cached check states, WARN passes, and a state keeps the time it began" )
        try
        {
            PulseGeneratorPtr pulseGenerator = CPulseGenerator::CreateSharedObj();
            CServiceHealthRegistry registry( pulseGenerator );
            CTestHealthCheck database( "database" );
            CTestHealthCheck lock( "lock" );
            ASSERT_TRUE( registry.RegisterReadinessCheck( &database ) );
            ASSERT_TRUE( registry.RegisterReadinessCheck( &lock ) );
            registry.SetStarted( true );

            // registered but not evaluated yet: failing
            CServiceHealthRegistry::SHealthReport report;
            registry.GetReadiness( report );
            ASSERT_FALSE( report.Passes() );
            ASSERT_TRUE( 2 == report.checks.size() );

            registry.EvaluateChecks();
            registry.GetReadiness( report );
            ASSERT_TRUE( HEALTH_PASS == report.state );
            const CServiceHealthRegistry::SCheckOutcome* outcome = FindOutcome( report, "database" );
            ASSERT_TRUE( GUCEF_NULL != outcome && HEALTH_PASS == outcome->state );
            const UInt64 passSince = GUCEF_NULL != outcome ? outcome->sinceUtcMs : 0;

            lock.state = HEALTH_WARN;
            lock.detail = "lease renews late";
            MT::PrecisionDelay( 20 );
            registry.EvaluateChecks();
            registry.GetReadiness( report );
            ASSERT_TRUE( HEALTH_WARN == report.state && report.Passes() );
            outcome = FindOutcome( report, "lock" );
            ASSERT_TRUE( GUCEF_NULL != outcome && HEALTH_WARN == outcome->state && "lease renews late" == outcome->detail );
            // the check that did not change keeps the time of its state
            outcome = FindOutcome( report, "database" );
            ASSERT_TRUE( GUCEF_NULL != outcome && passSince == outcome->sinceUtcMs );

            database.state = HEALTH_FAIL;
            database.detail = "connection refused";
            registry.EvaluateChecks();
            registry.GetReadiness( report );
            ASSERT_TRUE( HEALTH_FAIL == report.state );
            outcome = FindOutcome( report, "database" );
            ASSERT_TRUE( GUCEF_NULL != outcome && HEALTH_FAIL == outcome->state && outcome->sinceUtcMs >= passSince );

            // liveness never depends on the checks
            registry.GetLiveness( report );
            ASSERT_TRUE( report.Passes() );

            ASSERT_TRUE( registry.UnregisterReadinessCheck( &database ) );
            ASSERT_TRUE( registry.UnregisterReadinessCheck( &lock ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A check that throws is reported as failing and the others are still evaluated" )
        try
        {
            PulseGeneratorPtr pulseGenerator = CPulseGenerator::CreateSharedObj();
            CServiceHealthRegistry registry( pulseGenerator );
            CTestHealthCheck thrower( "thrower" );
            thrower.throws = true;
            CTestHealthCheck fine( "fine" );
            ASSERT_TRUE( registry.RegisterReadinessCheck( &thrower ) );
            ASSERT_TRUE( registry.RegisterReadinessCheck( &fine ) );
            registry.SetStarted( true );
            registry.EvaluateChecks();

            CServiceHealthRegistry::SHealthReport report;
            registry.GetReadiness( report );
            ASSERT_TRUE( HEALTH_FAIL == report.state );
            const CServiceHealthRegistry::SCheckOutcome* outcome = FindOutcome( report, "thrower" );
            ASSERT_TRUE( GUCEF_NULL != outcome && HEALTH_FAIL == outcome->state && "the check threw an exception" == outcome->detail );
            outcome = FindOutcome( report, "fine" );
            ASSERT_TRUE( GUCEF_NULL != outcome && HEALTH_PASS == outcome->state && 1 == fine.calls.Load() );

            registry.UnregisterReadinessCheck( &thrower );
            registry.UnregisterReadinessCheck( &fine );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The pulse evaluates the checks at the check interval and a pulse that stops makes liveness and readiness stale" )
        try
        {
            PulseGeneratorPtr pulseGenerator = CPulseGenerator::CreateSharedObj();
            CServiceHealthRegistry registry( pulseGenerator );
            registry.SetCheckIntervalInMs( 1000 );
            registry.SetStalenessInMs( 150 );
            CTestHealthCheck check( "check" );
            ASSERT_TRUE( registry.RegisterReadinessCheck( &check ) );
            registry.SetStarted( true );

            // the first pulse evaluates, the next ones within the interval do not
            pulseGenerator->ForceDirectPulse();
            pulseGenerator->ForceDirectPulse();
            pulseGenerator->ForceDirectPulse();
            ASSERT_TRUE( 1 == check.calls.Load() );
            CServiceHealthRegistry::SHealthReport report;
            registry.GetReadiness( report );
            ASSERT_TRUE( report.Passes() );

            // the pulse thread "hangs": nothing evaluates, the cache goes stale
            MT::PrecisionDelay( 300 );
            registry.GetLiveness( report );
            ASSERT_TRUE( HEALTH_FAIL == report.state && report.isStale );
            registry.GetReadiness( report );
            ASSERT_TRUE( HEALTH_FAIL == report.state && report.isStale );
            // startup is not about the pulse
            registry.GetStartup( report );
            ASSERT_TRUE( report.Passes() );

            // a short interval: the next pulse evaluates again and everything passes
            registry.SetCheckIntervalInMs( 10 );
            pulseGenerator->ForceDirectPulse();
            ASSERT_TRUE( 2 == check.calls.Load() );
            registry.GetLiveness( report );
            ASSERT_TRUE( report.Passes() && !report.isStale );
            registry.GetReadiness( report );
            ASSERT_TRUE( report.Passes() );

            // 0: never stale
            registry.SetStalenessInMs( 0 );
            MT::PrecisionDelay( 200 );
            registry.GetLiveness( report );
            ASSERT_TRUE( report.Passes() );

            ASSERT_TRUE( registry.UnregisterReadinessCheck( &check ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Registration refuses nothing and doubles, and the info is kept as given" )
        try
        {
            CServiceHealthRegistry registry;
            CTestHealthCheck check( "check" );
            ASSERT_FALSE( registry.RegisterReadinessCheck( GUCEF_NULL ) );
            ASSERT_TRUE( registry.RegisterReadinessCheck( &check ) );
            ASSERT_FALSE( registry.RegisterReadinessCheck( &check ) );
            ASSERT_TRUE( 1 == registry.GetReadinessCheckCount() );
            ASSERT_TRUE( registry.UnregisterReadinessCheck( &check ) );
            ASSERT_FALSE( registry.UnregisterReadinessCheck( &check ) );
            ASSERT_TRUE( 0 == registry.GetReadinessCheckCount() );

            CValueList info;
            info.Set( "version", CVariant( CString( "1.2.3" ) ) );
            info.Set( "host", CVariant( CString( "node-1" ) ) );
            registry.SetInfo( info );
            CValueList kept = registry.GetInfo();
            ASSERT_TRUE( 2 == kept.GetKeyCount() && "1.2.3" == kept.GetValueAlways( "version" ).AsString() );

            ASSERT_TRUE( CServiceHealthRegistry::DefaultCheckIntervalInMs == registry.GetCheckIntervalInMs() );
            ASSERT_TRUE( CServiceHealthRegistry::DefaultStalenessInMs == registry.GetStalenessInMs() );

            // the registry of the process is on the global pulse generator
            CServiceHealthRegistry& global = CCoreGlobal::Instance()->GetServiceHealthRegistry();
            ASSERT_TRUE( global.GetPulseGenerator() == CCoreGlobal::Instance()->GetPulseGenerator() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Unregistering a check while the checks run on another thread waits until they are done" )
        try
        {
            PulseGeneratorPtr pulseGenerator = CPulseGenerator::CreateSharedObj();
            CServiceHealthRegistry registry( pulseGenerator );
            CTestHealthCheck slow( "slow" );
            slow.blockMs = 300;
            ASSERT_TRUE( registry.RegisterReadinessCheck( &slow ) );
            registry.SetStarted( true );

            ThreadPoolPtr pool = CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( "TestServiceHealthRegistry" );
            TEvaluateChecksTaskPtr task( GUCEF_NEW CEvaluateChecksTask( registry ) );
            CFutureResult started = pool->StartTaskWithConsumer( task );
            ASSERT_FALSE( started.HasNoFuture() );
            ASSERT_TRUE( WaitForFlag( slow.entered, 5000 ) );

            // the cache is read while the check runs: not blocked by it
            CServiceHealthRegistry::SHealthReport report;
            registry.GetReadiness( report );
            ASSERT_FALSE( slow.finished.Load() );

            // the check is in use: unregistering waits for it
            ASSERT_TRUE( registry.UnregisterReadinessCheck( &slow ) );
            ASSERT_TRUE( slow.finished.Load() );
            ASSERT_TRUE( WaitForFlag( task->done, 5000 ) );
            ASSERT_TRUE( 0 == registry.GetReadinessCheckCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A check that unregisters itself from within its Check() leaves the outcomes of the other checks with them" )
        try
        {
            PulseGeneratorPtr none;
            CServiceHealthRegistry registry( none );
            CUnregisteringHealthCheck selfRemoving( "selfRemoving", registry );
            selfRemoving.toUnregister = &selfRemoving;
            selfRemoving.state = HEALTH_FAIL;
            CTestHealthCheck plain( "plain" );
            plain.state = HEALTH_PASS;
            plain.detail = "plain detail";
            ASSERT_TRUE( registry.RegisterReadinessCheck( &selfRemoving ) );
            ASSERT_TRUE( registry.RegisterReadinessCheck( &plain ) );
            registry.SetStarted( true );

            registry.EvaluateChecks();
            ASSERT_TRUE( 1 == registry.GetReadinessCheckCount() );

            CServiceHealthRegistry::SHealthReport report;
            registry.GetReadiness( report );
            ASSERT_TRUE( 1 == report.checks.size() );
            ASSERT_TRUE( GUCEF_NULL == FindOutcome( report, "selfRemoving" ) );
            const CServiceHealthRegistry::SCheckOutcome* plainOutcome = FindOutcome( report, "plain" );
            ASSERT_TRUE( GUCEF_NULL != plainOutcome );
            if ( GUCEF_NULL != plainOutcome )
            {
                // the outcome of the check that went must not land on the one that stays
                ASSERT_TRUE( HEALTH_PASS == plainOutcome->state );
                ASSERT_TRUE( plainOutcome->detail == "plain detail" );
            }
            ASSERT_TRUE( HEALTH_PASS == report.state );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A check that unregisters a later check from within its Check() keeps that one from being called" )
        try
        {
            PulseGeneratorPtr none;
            CServiceHealthRegistry registry( none );
            CUnregisteringHealthCheck first( "first", registry );
            first.state = HEALTH_PASS;
            CUnregisteringHealthCheck second( "second", registry );
            second.state = HEALTH_PASS;
            CTestHealthCheck third( "third" );
            third.state = HEALTH_WARN;
            third.detail = "third detail";
            first.toUnregister = &second;
            ASSERT_TRUE( registry.RegisterReadinessCheck( &first ) );
            ASSERT_TRUE( registry.RegisterReadinessCheck( &second ) );
            ASSERT_TRUE( registry.RegisterReadinessCheck( &third ) );
            registry.SetStarted( true );

            // a check that was unregistered may be gone already: it is not called any more
            registry.EvaluateChecks();
            ASSERT_TRUE( 1 == first.calls );
            ASSERT_TRUE( 0 == second.calls );
            ASSERT_TRUE( 1 == third.calls.Load() );
            ASSERT_TRUE( 2 == registry.GetReadinessCheckCount() );

            CServiceHealthRegistry::SHealthReport report;
            registry.GetReadiness( report );
            const CServiceHealthRegistry::SCheckOutcome* firstOutcome = FindOutcome( report, "first" );
            const CServiceHealthRegistry::SCheckOutcome* thirdOutcome = FindOutcome( report, "third" );
            ASSERT_TRUE( GUCEF_NULL != firstOutcome && HEALTH_PASS == firstOutcome->state && firstOutcome->detail == "first" );
            ASSERT_TRUE( GUCEF_NULL != thirdOutcome && HEALTH_WARN == thirdOutcome->state && thirdOutcome->detail == "third detail" );
            ASSERT_TRUE( GUCEF_NULL == FindOutcome( report, "second" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
