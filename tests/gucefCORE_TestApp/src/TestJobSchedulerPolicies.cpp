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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CSLIDINGWINDOWTHROTTLEPOLICY_H
#include "gucefCORE_CSlidingWindowThrottlePolicy.h"
#define GUCEF_CORE_CSLIDINGWINDOWTHROTTLEPOLICY_H
#endif /* GUCEF_CORE_CSLIDINGWINDOWTHROTTLEPOLICY_H ? */

#ifndef GUCEF_CORE_CDAILYBUDGETPACINGPOLICY_H
#include "gucefCORE_CDailyBudgetPacingPolicy.h"
#define GUCEF_CORE_CDAILYBUDGETPACINGPOLICY_H
#endif /* GUCEF_CORE_CDAILYBUDGETPACINGPOLICY_H ? */

#ifndef GUCEF_CORE_CJOBSCHEDULERFILESTATESTORE_H
#include "gucefCORE_CJobSchedulerFileStateStore.h"
#define GUCEF_CORE_CJOBSCHEDULERFILESTATESTORE_H
#endif /* GUCEF_CORE_CJOBSCHEDULERFILESTATESTORE_H ? */

#ifndef GUCEF_CORE_CJOBSCHEDULERHEALTHCHECK_H
#include "gucefCORE_CJobSchedulerHealthCheck.h"
#define GUCEF_CORE_CJOBSCHEDULERHEALTHCHECK_H
#endif /* GUCEF_CORE_CJOBSCHEDULERHEALTHCHECK_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestJsonDataNodeFidelity.h"
#include "TestJobSchedulerSupport.h"
#include "TestJobSchedulerPolicies.h"

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Simulates a lane that asks its policy whenever the policy says it is worth asking. The start times (ms after start) */
static void
SimulatePolicy( CORE::CIJobThrottlePolicy& policy, Int64 startUtcMs, UInt64 durationMs, UInt32 priority, GUCEF::vector< UInt64 >& starts )
{
    UInt64 elapsed = 0;
    UInt64 monotonicBase = 1000;
    starts.clear();
    while ( elapsed < durationMs )
    {
        CORE::SJobPermitRequest request;
        request.laneName = "sim";
        request.priority = priority;
        request.estimatedCost = 1;
        CORE::SJobPermitAnswer answer;
        policy.Check( request, startUtcMs + (Int64) elapsed, monotonicBase + elapsed, answer );
        if ( answer.granted )
        {
            policy.OnStarted( request, startUtcMs + (Int64) elapsed, monotonicBase + elapsed );
            starts.push_back( elapsed );
            continue;
        }
        elapsed += answer.waitMs > 0 ? answer.waitMs : 1;
    }
}

/*-------------------------------------------------------------------------*/

static UInt32
CountBefore( const GUCEF::vector< UInt64 >& starts, UInt64 limitMs )
{
    UInt32 count = 0;
    for ( size_t i=0; i<starts.size(); ++i )
        if ( starts[ i ] < limitMs )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

static UInt64
MaxGap( const GUCEF::vector< UInt64 >& starts )
{
    UInt64 gap = 0;
    for ( size_t i=1; i<starts.size(); ++i )
        if ( starts[ i ] - starts[ i-1 ] > gap )
            gap = starts[ i ] - starts[ i-1 ];
    return gap;
}

/*-------------------------------------------------------------------------*/

static UInt64
MinGap( const GUCEF::vector< UInt64 >& starts )
{
    UInt64 gap = 0xFFFFFFFFFFFFFFFFULL;
    for ( size_t i=1; i<starts.size(); ++i )
        if ( starts[ i ] - starts[ i-1 ] < gap )
            gap = starts[ i ] - starts[ i-1 ];
    return gap;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformThrottleTests( void )
{
    GUCEF_TESTFW_TESTCASE( "JS-T7: the sliding window policy holds per minute and per hour windows and the minimum interval" )
    {
        CORE::CSlidingWindowThrottlePolicy policy;
        policy.SetRequestsPerMinute( 5 );
        policy.SetRequestsPerHour( 100 );
        policy.SetMinimumIntervalMs( 1000 );
        GUCEF::vector< UInt64 > starts;
        SimulatePolicy( policy, JsUtc( 0 ), 3600000, 1, starts );
        ASSERT_TRUE( 100 == starts.size() );
        ASSERT_TRUE( MinGap( starts ) >= 1000 );
        // never more than 5 in any minute
        bool minuteHeld = true;
        for ( size_t i=5; i<starts.size(); ++i )
            minuteHeld = minuteHeld && starts[ i ] - starts[ i-5 ] >= 60000;
        ASSERT_TRUE( minuteHeld );
        CORE::CDataNode status;
        policy.GetStatus( status, JsUtc( 1 ) );
        ASSERT_TRUE( 100 == status.GetAttributeValueOrChildValueByName( "limitPerHour" ).AsUInt32( 0, true ) );
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JS-T7: the sliding window reserve keeps part of a limit for urgent work" )
    {
        CORE::CSlidingWindowThrottlePolicy policy;
        policy.SetRequestsPerMinute( 10 );
        policy.SetReservePercent( 30 );
        policy.SetReserveFromPriority( 2 );
        GUCEF::vector< UInt64 > lowStarts;
        SimulatePolicy( policy, JsUtc( 0 ), 1, 2, lowStarts );       // low priority: 7 of the 10
        ASSERT_TRUE( 7 == lowStarts.size() );
        CORE::SJobPermitRequest urgent;
        urgent.priority = 0;
        urgent.estimatedCost = 1;
        CORE::SJobPermitAnswer answer;
        policy.Check( urgent, JsUtc( 0 ), 1000, answer );
        ASSERT_TRUE( answer.granted );
        CORE::SJobPermitRequest low;
        low.priority = 3;
        low.estimatedCost = 1;
        policy.Check( low, JsUtc( 0 ), 1000, answer );
        ASSERT_FALSE( answer.granted );
        ASSERT_TRUE( answer.bindingLimit == "requestsPerMinute" );
        ASSERT_TRUE( answer.waitMs > 0 && answer.waitMs <= 60000 );

        // the windows hold across a restart: the kept starts come back
        CORE::CDataNode state;
        ASSERT_TRUE( policy.SaveState( state ) );
        CORE::CSlidingWindowThrottlePolicy restarted;
        restarted.SetRequestsPerMinute( 10 );
        restarted.SetReservePercent( 30 );
        CORE::CString error;
        ASSERT_TRUE( restarted.LoadState( state, error ) );
        restarted.Check( low, JsUtc( 0, 0, 10 ), 500, answer );
        ASSERT_FALSE( answer.granted );
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JS-T7: the daily budget pacing spends a day evenly (480/day: 20 in the first hour, every 180 s; 500 with a 20 % reserve: 400, 17 in the first hour, every 216 s)" )
    {
        CORE::CDailyBudgetPacingPolicy polite;
        polite.SetDailyCapacity( 480 );
        GUCEF::vector< UInt64 > starts;
        SimulatePolicy( polite, JsUtc( 0 ), 86400000 - 1, 1, starts );
        ASSERT_TRUE( 480 == starts.size() );
        ASSERT_TRUE( 20 == CountBefore( starts, 3600000 ) );
        ASSERT_TRUE( MinGap( starts ) >= 179000 && MaxGap( starts ) <= 181000 );

        CORE::CDailyBudgetPacingPolicy dayLimit;
        dayLimit.SetDailyCapacity( 500 );
        dayLimit.SetReservePercent( 20 );
        dayLimit.SetReserveFromPriority( 1 );
        SimulatePolicy( dayLimit, JsUtc( 24 ), 86400000 - 1, 1, starts );
        ASSERT_TRUE( 400 == starts.size() );
        ASSERT_TRUE( 17 == CountBefore( starts, 3600000 ) );
        ASSERT_TRUE( MinGap( starts ) >= 215000 && MaxGap( starts ) <= 217000 );

        // a calendar clock set back does not give the day's budget again (monotonic guard)
        CORE::SJobPermitRequest request;
        request.priority = 1;
        request.estimatedCost = 1;
        CORE::SJobPermitAnswer answer;
        dayLimit.Check( request, JsUtc( 24, 23, 59 ), 1000 + 86400000, answer );
        ASSERT_FALSE( answer.granted );
        ASSERT_TRUE( answer.bindingLimit == "dayBudget" );
        dayLimit.Check( request, JsUtc( 23 ), 1000 + 86400000 + 1000, answer );
        ASSERT_FALSE( answer.granted );
        // the next day starts the budget again
        dayLimit.Check( request, JsUtc( 48, 0, 1 ), 1000 + 86400000 + 2000, answer );
        ASSERT_TRUE( answer.granted );
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

static void
PerformFileStoreTests( void )
{
    CORE::CString directory = CORE::RelativePath( "$CURWORKDIR$" );
    CORE::CString path = CORE::CombinePath( directory, "jobscheduler_state_test.json" );
    CORE::DeleteFile( path );
    CORE::DeleteFile( path + ".bad" );

    GUCEF_TESTFW_TESTCASE( "JS-T9: the file state store reads what it wrote (json) and starts empty without a file" )
    {
        GUCEF_TESTFW_SKIP_IF( !JsonTestCodecAvailable(), "the json codec plugin is not available" );
        CORE::CJobSchedulerFileStateStore store( path );
        CORE::CDataNode state;
        CORE::CString error;
        ASSERT_TRUE( store.LoadState( state, error ) );
        ASSERT_FALSE( state.HasChildren() );
        state.SetName( "jobScheduler" );
        state.SetNodeType( GUCEF_DATATYPE_OBJECT );
        state.SetAttribute( "version", 1 );
        CORE::CDataNode* jobs = state.AddChild( "jobs", GUCEF_DATATYPE_OBJECT );
        CORE::CDataNode* job = jobs->AddChild( "nightly", GUCEF_DATATYPE_OBJECT );
        job->SetAttribute( "lastOccurrenceUtcMs", JsUtc( 2 ) );
        ASSERT_TRUE( store.SaveState( state, error ) );
        CORE::CDataNode loaded;
        ASSERT_TRUE( store.LoadState( loaded, error ) );
        const CORE::CDataNode* loadedJobs = loaded.FindChild( "jobs" );
        const CORE::CDataNode* loadedJob = GUCEF_NULL != loadedJobs ? loadedJobs->FindChild( "nightly" ) : GUCEF_NULL;
        ASSERT_TRUE( GUCEF_NULL != loadedJob && JsUtc( 2 ) == loadedJob->GetAttributeValueOrChildValueByName( "lastOccurrenceUtcMs" ).AsInt64( 0, true ) );
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JS-T9: an unreadable state file is kept as .bad and the scheduler starts from its defaults" )
    {
        GUCEF_TESTFW_SKIP_IF( !JsonTestCodecAvailable(), "the json codec plugin is not available" );
        const char garbage[] = "{ this is not json at all";
        ASSERT_TRUE( CORE::WriteFileAtomically( path, garbage, sizeof( garbage ) - 1 ) );
        CORE::CJobSchedulerFileStateStore store( path );
        CORE::CDataNode state;
        CORE::CString error;
        ASSERT_FALSE( store.LoadState( state, error ) );
        ASSERT_TRUE( CORE::FileExists( path + ".bad" ) );
        ASSERT_FALSE( CORE::FileExists( path ) );

        // a scheduler on that store: starts from its defaults, its next save writes a good file
        CORE::WriteFileAtomically( path, garbage, sizeof( garbage ) - 1 );
        CORE::DeleteFile( path + ".bad" );
        CManualClocks clocks( JsUtc( 20 ) );
        CORE::JobSchedulerPtr scheduler = MakeJsScheduler( MakeJsSettings( "js-t9-file" ), &clocks );
        scheduler->SetStateStore( &store );
        CORE::CString startError;
        scheduler->AddJob( MakeJsJob( "job", CORE::CJobTrigger::Manual(), "file" ), startError );
        CORE::CJobSchedulerTestAccess::StartManual( *scheduler, startError );
        CORE::CJobSchedulerTestAccess::Cycle( *scheduler );
        ASSERT_TRUE( CORE::FileExists( path + ".bad" ) );
        ASSERT_TRUE( CORE::JOBSTATE_SCHEDULED == GetJobStatus( *scheduler, "job" ).state );
        DropJsScheduler( scheduler );
        CORE::CDataNode afterStop;
        ASSERT_TRUE( store.LoadState( afterStop, error ) );
        ASSERT_TRUE( GUCEF_NULL != afterStop.FindChild( "jobs" ) );
    }
    GUCEF_TESTFW_TESTCASE_END;

    CORE::DeleteFile( path );
    CORE::DeleteFile( path + ".bad" );
}

/*-------------------------------------------------------------------------*/

static void
PerformHealthCheckTests( void )
{
    GUCEF_TESTFW_TESTCASE( "The scheduler health check passes, warns after consecutive failures and fails when stopped" )
    {
        CManualClocks clocks( JsUtc( 21 ) );
        CORE::JobSchedulerPtr scheduler = MakeJsScheduler( MakeJsSettings( "js-health" ), &clocks );
        CORE::CString error;
        scheduler->AddJob( MakeJsJob( "failing", CORE::CJobTrigger::Manual(), "health", 1, 0, true ), error );
        CORE::CJobSchedulerHealthCheck check( scheduler, 60000, 1 );
        CORE::CString detail;
        ASSERT_TRUE( CORE::HEALTH_FAIL == check.Check( detail ) );       // not started
        CORE::CJobSchedulerTestAccess::StartManual( *scheduler, error );
        CORE::CJobSchedulerTestAccess::Cycle( *scheduler );
        ASSERT_TRUE( CORE::HEALTH_PASS == check.Check( detail ) );
        for ( UInt32 i=0; i<2; ++i )
        {
            CORE::SJobControlCommand command;
            command.action = CORE::JOBCONTROLACTION_RUN_NOW;
            command.targetKind = CORE::JOBCONTROLTARGET_JOB;
            command.targetId = "failing";
            CORE::SJobControlResult result;
            scheduler->Control( command, result );
            CycleUntilIdle( *scheduler );
        }
        ASSERT_TRUE( CORE::HEALTH_WARN == check.Check( detail ) );
        ASSERT_TRUE( detail.HasSubstr( "failing" ) >= 0 );
        DropJsScheduler( scheduler );
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

void
PerformJobSchedulerPolicyTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "JobSchedulerPolicies" );

    RegisterJsTestTaskType();
    PerformThrottleTests();
    PerformFileStoreTests();
    PerformHealthCheckTests();
    UnregisterJsTestTaskType();
}

/*-------------------------------------------------------------------------*/
