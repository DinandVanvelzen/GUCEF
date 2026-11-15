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

#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

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

#ifndef GUCEF_CORE_CIJOBTHROTTLEPOLICY_H
#include "gucefCORE_CIJobThrottlePolicy.h"
#define GUCEF_CORE_CIJOBTHROTTLEPOLICY_H
#endif /* GUCEF_CORE_CIJOBTHROTTLEPOLICY_H ? */

#ifndef GUCEF_CORE_CIJOBWORKSOURCE_H
#include "gucefCORE_CIJobWorkSource.h"
#define GUCEF_CORE_CIJOBWORKSOURCE_H
#endif /* GUCEF_CORE_CIJOBWORKSOURCE_H ? */

#ifndef GUCEF_CORE_CIJOBWORKITEMSINK_H
#include "gucefCORE_CIJobWorkItemSink.h"
#define GUCEF_CORE_CIJOBWORKITEMSINK_H
#endif /* GUCEF_CORE_CIJOBWORKITEMSINK_H ? */

#ifndef GUCEF_CORE_CIJOBPAYLOADVALIDATOR_H
#include "gucefCORE_CIJobPayloadValidator.h"
#define GUCEF_CORE_CIJOBPAYLOADVALIDATOR_H
#endif /* GUCEF_CORE_CIJOBPAYLOADVALIDATOR_H ? */

#ifndef GUCEF_CORE_CJOBSCHEDULERLANEHOLDSCOPE_H
#include "gucefCORE_CJobSchedulerLaneHoldScope.h"
#define GUCEF_CORE_CJOBSCHEDULERLANEHOLDSCOPE_H
#endif /* GUCEF_CORE_CJOBSCHEDULERLANEHOLDSCOPE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestJobSchedulerSupport.h"
#include "TestJobScheduler.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace CORETEST;

typedef CORE::CJobSchedulerTestAccess TAccess;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A throttle policy that counts the calls and refuses while told so */
class CJsCountingThrottle : public CORE::CIJobThrottlePolicy
{
    public:

    CJsCountingThrottle( void ) : checks( 0 ), started( 0 ), finished( 0 ), runNows( 0 ), refuse( false ), waitMs( 5000 ), holdCost( 0 ) {}
    virtual ~CJsCountingThrottle() GUCEF_VIRTUAL_OVERRIDE {}

    virtual void Check( const CORE::SJobPermitRequest& request, Int64 nowUtcMs, UInt64 nowMonotonicMs, CORE::SJobPermitAnswer& answer ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++checks;
        answer.granted = !refuse;
        answer.waitMs = refuse ? waitMs : 0;
        answer.bindingLimit = refuse ? "testLimit" : "";
    }
    virtual void OnStarted( const CORE::SJobPermitRequest& request, Int64 nowUtcMs, UInt64 nowMonotonicMs ) GUCEF_VIRTUAL_OVERRIDE { ++started; }
    virtual void OnFinished( const CORE::SJobPermitRequest& request, UInt32 actualCost, CORE::TJobRunState outcome, UInt64 retryAfterMs, Int64 nowUtcMs ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++finished;
        if ( CORE::JOBRUNCAUSE_HOLD == request.cause )
            holdCost += actualCost;
    }
    virtual void OnRunNow( Int64 nowUtcMs ) GUCEF_VIRTUAL_OVERRIDE { ++runNows; refuse = false; }
    virtual void GetStatus( CORE::CDataNode& status, Int64 nowUtcMs ) const GUCEF_VIRTUAL_OVERRIDE { status.SetAttribute( "checks", checks ); }

    UInt32 checks;
    UInt32 started;
    UInt32 finished;
    UInt32 runNows;
    bool refuse;
    UInt64 waitMs;
    UInt32 holdCost;
};

/*-------------------------------------------------------------------------*/

/** A work source that hands out the items it is given, on lanes and priorities of their own */
class CJsWorkSource : public CORE::CIJobWorkSource
{
    public:

    CJsWorkSource( void ) : evaluations( 0 ), nextWakeUtcMs( 0 ), runNowLanes(), m_lock(), m_pending(), m_finished() {}
    virtual ~CJsWorkSource() GUCEF_VIRTUAL_OVERRIDE {}

    void AddItem( const CORE::SJobWorkItem& item ) { MT::CScopeMutex lock( m_lock ); m_pending.push_back( item ); }

    virtual void Evaluate( Int64 nowUtcMs, UInt64 nowMonotonicMs, CORE::CIJobWorkItemSink& dispatch, Int64& nextWake ) GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::TJobWorkItemVector items;
        {
            MT::CScopeMutex lock( m_lock );
            ++evaluations;
            items.swap( m_pending );
            nextWake = nextWakeUtcMs;
        }
        for ( CORE::TJobWorkItemVector::iterator i = items.begin(); i != items.end(); ++i )
            dispatch.Dispatch( (*i) );
    }

    virtual void OnItemFinished( const CORE::SJobWorkItem& item, const CORE::SJobRunRecord& run, Int64 nowUtcMs ) GUCEF_VIRTUAL_OVERRIDE
    {
        MT::CScopeMutex lock( m_lock );
        m_finished.push_back( std::pair< CORE::CString, CORE::TJobRunState >( item.key, run.state ) );
    }

    virtual void OnRunNow( const CORE::CString& laneName, Int64 nowUtcMs ) GUCEF_VIRTUAL_OVERRIDE { MT::CScopeMutex lock( m_lock ); runNowLanes.push_back( laneName ); }

    virtual void GetStatus( CORE::CDataNode& status, Int64 nowUtcMs, bool includeQueue ) const GUCEF_VIRTUAL_OVERRIDE
    {
        MT::CScopeMutex lock( m_lock );
        status.SetNodeType( GUCEF_DATATYPE_OBJECT );
        status.SetAttribute( "evaluations", evaluations );
        status.SetAttribute( "finished", (UInt32) m_finished.size() );
        status.SetAttribute( "nowUtcMs", nowUtcMs );
        if ( includeQueue )
        {
            CORE::CDataNode* queue = status.AddChild( "queue", GUCEF_DATATYPE_ARRAY );
            for ( size_t i=0; i<m_pending.size(); ++i )
                queue->AddValueAsChild( m_pending[ i ].key );
        }
    }

    UInt32 FinishedCount( void ) const { MT::CScopeMutex lock( m_lock ); return (UInt32) m_finished.size(); }
    CORE::TJobRunState FinishedState( const CORE::CString& key ) const
    {
        MT::CScopeMutex lock( m_lock );
        for ( size_t i=0; i<m_finished.size(); ++i )
            if ( m_finished[ i ].first == key )
                return m_finished[ i ].second;
        return CORE::JOBRUNSTATE_UNSPECIFIED;
    }

    UInt32 evaluations;
    Int64 nextWakeUtcMs;
    CORE::CString::StringVector runNowLanes;

    private:

    mutable MT::CMutex m_lock;
    CORE::TJobWorkItemVector m_pending;
    GUCEF::vector< std::pair< CORE::CString, CORE::TJobRunState > > m_finished;
};

/*-------------------------------------------------------------------------*/

/** Refuses payloads whose value is "evil" */
class CJsPayloadValidator : public CORE::CIJobPayloadValidator
{
    public:

    virtual bool ValidatePayload( const CORE::CString& taskTypeName, const CORE::CDataNode& payload, CORE::CString& reason ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( payload.GetAttributeValueOrChildValueByName( "value" ).AsString( CORE::CString::Empty, true ) == "evil" )
        {
            reason = "evil payloads are refused";
            return false;
        }
        return true;
    }
};

/*-------------------------------------------------------------------------*/

/** Counts the events of a scheduler and the thread they arrive on */
class CJsEventCounter : public CORE::CObserver
{
    public:

    CJsEventCounter( void ) : jobChanges( 0 ), runChanges( 0 ), hostChanges( 0 ), laneChanges( 0 ), otherThread( false ), m_threadId( MT::GetCurrentThreadID() ) {}
    virtual ~CJsEventCounter() GUCEF_VIRTUAL_OVERRIDE { UnsubscribeAllFromObserver(); }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( MT::GetCurrentThreadID() != m_threadId )
            otherThread = true;
        if ( CORE::CJobScheduler::JobChangedEvent == eventId ) ++jobChanges;
        else if ( CORE::CJobScheduler::RunChangedEvent == eventId ) ++runChanges;
        else if ( CORE::CJobScheduler::HostStateChangedEvent == eventId ) ++hostChanges;
        else if ( CORE::CJobScheduler::LaneChangedEvent == eventId ) ++laneChanges;
    }

    UInt32 jobChanges;
    UInt32 runChanges;
    UInt32 hostChanges;
    UInt32 laneChanges;
    bool otherThread;

    private:

    TThreadId m_threadId;
};

/*-------------------------------------------------------------------------*/

/** Plays the application's main thread: pumps a pulse generator until it is stopped */
class CJsPulseThread : public MT::CActiveObject
{
    public:

    explicit CJsPulseThread( CORE::PulseGeneratorPtr pulse ) : MT::CActiveObject(), m_pulse( pulse ), m_systemThreadId( 0 ) {}
    virtual ~CJsPulseThread() { Deactivate( false, true ); }

    UInt64 GetSystemThreadId( void ) const { return m_systemThreadId.Load(); }
    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { m_systemThreadId.Store( (UInt64) MT::GetCurrentThreadSystemID() ); return true; }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_pulse->ForceDirectPulse();
        return false;
    }

    private:

    CORE::PulseGeneratorPtr m_pulse;
    MT::CTAtomic< UInt64 > m_systemThreadId;
};

/*-------------------------------------------------------------------------*/

/** Issues control commands and snapshots from a thread of its own while the dispatcher works */
class CJsControlThread : public MT::CActiveObject
{
    public:

    CJsControlThread( CORE::CJobScheduler* scheduler, const CORE::CString& jobId )
        : MT::CActiveObject(), m_scheduler( scheduler ), m_jobId( jobId ), m_commands( 0 ), m_ok( 0 ), m_rounds( 0 ) {}
    virtual ~CJsControlThread() { Deactivate( false, true ); }

    UInt32 GetOk( void ) const { return m_ok.Load(); }
    UInt32 GetCommands( void ) const { return m_commands.Load(); }
    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::SJobControlCommand command;
        command.action = CORE::JOBCONTROLACTION_RUN_NOW;
        command.targetKind = CORE::JOBCONTROLTARGET_JOB;
        command.targetId = m_jobId;
        command.requestedBy = "thread";
        CORE::SJobControlResult result;
        if ( CORE::JOBCONTROLSTATUS_OK == m_scheduler->Control( command, result ) )
            m_ok.FetchAdd( 1 );
        m_commands.FetchAdd( 1 );
        CORE::CJobSchedulerSnapshot snapshot;
        m_scheduler->GetSnapshot( snapshot );
        CORE::TScheduledJobInfoVector info;
        m_scheduler->GetScheduledJobInfo( info );
        ++m_rounds;
        return m_rounds >= 25;
    }

    private:

    CORE::CJobScheduler* m_scheduler;
    CORE::CString m_jobId;
    MT::CTAtomic< UInt32 > m_commands;
    MT::CTAtomic< UInt32 > m_ok;
    UInt32 m_rounds;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static CORE::CJobTrigger
Cron( const char* expression )
{
    return CORE::CJobTrigger::Schedule( CORE::CSchedule( CORE::CCronSchedule( expression ) ) );
}

/*-------------------------------------------------------------------------*/

static CORE::TJobControlStatus
RunNow( CORE::CJobScheduler& scheduler, const CORE::CString& jobId, CORE::SJobControlResult& result )
{
    CORE::SJobControlCommand command;
    command.action = CORE::JOBCONTROLACTION_RUN_NOW;
    command.targetKind = CORE::JOBCONTROLTARGET_JOB;
    command.targetId = jobId;
    command.requestedBy = "tester";
    return scheduler.Control( command, result );
}

/*-------------------------------------------------------------------------*/

static CORE::TJobControlStatus
SimpleControl( CORE::CJobScheduler& scheduler, CORE::TJobControlAction action, CORE::TJobControlTargetKind kind, const CORE::CString& target )
{
    CORE::SJobControlCommand command;
    command.action = action;
    command.targetKind = kind;
    command.targetId = target;
    CORE::SJobControlResult result;
    return scheduler.Control( command, result );
}

/*-------------------------------------------------------------------------*/

static UInt32
CountRunsWithCause( CORE::CJobScheduler& scheduler, const CORE::CString& jobId, CORE::TJobRunCause cause, bool excludeSkipped = true )
{
    CORE::TJobRunRecordVector runs;
    GetJobRuns( scheduler, jobId, runs );
    UInt32 count = 0;
    for ( size_t i=0; i<runs.size(); ++i )
        if ( runs[ i ].cause == cause && ( !excludeSkipped || CORE::JOBRUNSTATE_SKIPPED != runs[ i ].state ) )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

static UInt32
CountExecutedRuns( CORE::CJobScheduler& scheduler, const CORE::CString& jobId )
{
    CORE::TJobRunRecordVector runs;
    GetJobRuns( scheduler, jobId, runs );
    UInt32 count = 0;
    for ( size_t i=0; i<runs.size(); ++i )
        if ( CORE::JOBRUNSTATE_SKIPPED != runs[ i ].state )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

/** A run of the job in the given state, the newest; false when none */
static bool
FindRun( CORE::CJobScheduler& scheduler, const CORE::CString& jobId, CORE::TJobRunState state, CORE::SJobRunRecord& run )
{
    CORE::TJobRunRecordVector runs;
    GetJobRuns( scheduler, jobId, runs );
    for ( size_t i=0; i<runs.size(); ++i )
    {
        if ( runs[ i ].state == state )
        {
            run = runs[ i ];
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/** Waits (bounded) until the run is in the state; real time */
static bool
WaitForRunState( CORE::CJobScheduler& scheduler, UInt64 runId, CORE::TJobRunState state, UInt32 timeoutMs, bool manualCycles )
{
    for ( UInt32 waited=0; waited<timeoutMs; waited += 2 )
    {
        if ( manualCycles )
            TAccess::Cycle( scheduler );
        CORE::SJobRunRecord run;
        if ( scheduler.GetRun( runId, run ) && run.state == state )
            return true;
        MT::ThreadDelay( 2 );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

/** Voluntary context switches of a thread of this process (Linux), -1 when it cannot be read */
static Int64
VoluntaryContextSwitches( UInt64 systemThreadId )
{
    char path[ 128 ];
    sprintf( path, "/proc/self/task/%llu/status", (unsigned long long) systemThreadId );
    FILE* file = fopen( path, "r" );
    if ( GUCEF_NULL == file )
        return -1;
    char line[ 256 ];
    Int64 switches = -1;
    while ( GUCEF_NULL != fgets( line, sizeof( line ), file ) )
    {
        unsigned long long value = 0;
        if ( 1 == sscanf( line, "voluntary_ctxt_switches: %llu", &value ) )
        {
            switches = (Int64) value;
            break;
        }
    }
    fclose( file );
    return switches;
}

#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformTriggerTests( void )
{
    GUCEF_TESTFW_TESTCASE( "JS-T1: a cron trigger is due at its next occurrence in UTC" )
    {
        CManualClocks clocks( JsUtc( 0, 3 ) );
        CORE::JobSchedulerPtr scheduler = MakeJsScheduler( MakeJsSettings( "js-t1-cron" ), &clocks );
        CORE::CString error;
        ASSERT_TRUE( scheduler->AddJob( MakeJsJob( "cron", Cron( "*/10 * * * *" ), "cron" ), error ) );
        ASSERT_TRUE( TAccess::StartManual( *scheduler, error ) );

        Int64 delay = TAccess::Cycle( *scheduler );
        ASSERT_TRUE( 7 * 60000 == delay );
        ASSERT_TRUE( JsUtc( 0, 10 ) == GetJobStatus( *scheduler, "cron" ).nextDueUtcMs );
        clocks.Advance( delay );
        TAccess::Cycle( *scheduler );
        ASSERT_TRUE( CycleUntilIdle( *scheduler ) );
        CORE::SJobRunRecord run;
        ASSERT_TRUE( FindRun( *scheduler, "cron", CORE::JOBRUNSTATE_SUCCEEDED, run ) );
        ASSERT_TRUE( JsUtc( 0, 10 ) == run.scheduledUtcMs );
        ASSERT_TRUE( CORE::JOBRUNCAUSE_SCHEDULE == run.cause );
        ASSERT_TRUE( JsUtc( 0, 20 ) == GetJobStatus( *scheduler, "cron" ).nextDueUtcMs );
        DropJsScheduler( scheduler );
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JS-T1: interval, once-at and manual triggers fire at the right instants" )
    {
        CManualClocks clocks( JsUtc( 1 ) );
        CORE::CJobSchedulerSettings settings = MakeJsSettings( "js-t1-kinds" );
        settings.SetMaxSleepMs( 900000 );
        CORE::JobSchedulerPtr scheduler = MakeJsScheduler( settings, &clocks );
        CORE::CString error;
        ASSERT_TRUE( scheduler->AddJob( MakeJsJob( "interval", CORE::CJobTrigger::Interval( 60000 ), "interval" ), error ) );
        ASSERT_TRUE( scheduler->AddJob( MakeJsJob( "once", CORE::CJobTrigger::OnceAt( JsUtc( 1, 0, 5 ) ), "once" ), error ) );
        ASSERT_TRUE( scheduler->AddJob( MakeJsJob( "manual", CORE::CJobTrigger::Manual(), "manual" ), error ) );
        ASSERT_TRUE( TAccess::StartManual( *scheduler, error ) );

        ASSERT_TRUE( 5000 == TAccess::Cycle( *scheduler ) );
        clocks.Advance( 5000 );
        TAccess::Cycle( *scheduler );
        ASSERT_TRUE( CycleUntilIdle( *scheduler ) );
        ASSERT_TRUE( 1 == CountJobRunsInState( *scheduler, "once", CORE::JOBRUNSTATE_SUCCEEDED ) );
        ASSERT_TRUE( CORE::JOBSTATE_ENDED == GetJobStatus( *scheduler, "once" ).state );

        Int64 delay = TAccess::Cycle( *scheduler );
        ASSERT_TRUE( 55000 == delay );
        clocks.Advance( delay );
        TAccess::Cycle( *scheduler );
        ASSERT_TRUE( CycleUntilIdle( *scheduler ) );
        ASSERT_TRUE( 1 == CountJobRunsInState( *scheduler, "interval", CORE::JOBRUNSTATE_SUCCEEDED ) );
        ASSERT_TRUE( 60000 == TAccess::Cycle( *scheduler ) );
        ASSERT_TRUE( 0 == CountExecutedRuns( *scheduler, "manual" ) );
        ASSERT_TRUE( CORE::JOBSTATE_SCHEDULED == GetJobStatus( *scheduler, "manual" ).state );
        ASSERT_TRUE( 0 == GetJobStatus( *scheduler, "manual" ).nextDueUtcMs );
        DropJsScheduler( scheduler );
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JS-T1: the first planning of a job's due instant is a change of the job (new revision, in the changes after the previous revision)" )
    {
        CManualClocks clocks( JsUtc( 2 ) );
        CORE::JobSchedulerPtr scheduler = MakeJsScheduler( MakeJsSettings( "js-t1-plan-change" ), &clocks );
        CORE::CString error;
        ASSERT_TRUE( scheduler->AddJob( MakeJsJob( "interval", CORE::CJobTrigger::Interval( 3600000 ), "interval" ), error ) );
        ASSERT_TRUE( TAccess::StartManual( *scheduler, error ) );
        CORE::CJobSchedulerSnapshot before;
        scheduler->GetSnapshot( before, 0 );
        const CORE::SJobStatus* planned = before.FindJob( "interval" );
        ASSERT_TRUE( GUCEF_NULL != planned && 0 == planned->nextDueUtcMs );
        const UInt64 revisionBefore = before.GetRevision();
        const UInt64 jobRevisionBefore = GUCEF_NULL != planned ? planned->revision : 0;

        TAccess::Cycle( *scheduler );
        CORE::CJobSchedulerSnapshot changes;
        scheduler->GetSnapshot( changes, revisionBefore );
        const CORE::SJobStatus* changed = changes.FindJob( "interval" );
        ASSERT_TRUE( GUCEF_NULL != changed );
        if ( GUCEF_NULL != changed )
        {
            ASSERT_TRUE( JsUtc( 3 ) == changed->nextDueUtcMs );
            ASSERT_TRUE( changed->revision > jobRevisionBefore );
        }
        DropJsScheduler( scheduler );
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JS-T1: 1000 jobs over 10 due instants give one timed dispatcher wake per distinct instant" )
    {
        CManualClocks clocks( JsUtc( 2, 0, 30 ) );
        CORE::CJobSchedulerSettings settings = MakeJsSettings( "js-t1-many" );
        settings.SetMaxSleepMs( 0 );
        settings.SetDefaultLaneMaxConcurrent( 1000 );
        CORE::JobSchedulerPtr scheduler = MakeJsScheduler( settings, &clocks );
        CORE::ThreadPoolPtr runPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( settings.GetRunPoolName() );
        runPool->SetDesiredMinNrOfWorkerThreads( 4 );
        CORE::CString error;
        bool allAdded = true;
        for ( UInt32 i=0; i<1000; ++i )
        {
            char cron[ 32 ];
            sprintf( cron, "%u * * * *", (unsigned) ( 1 + ( i % 10 ) ) );
            char id[ 32 ];
            sprintf( id, "job-%04u", (unsigned) i );
            allAdded = scheduler->AddJob( MakeJsJob( id, Cron( cron ), "many" ), error ) && allAdded;
        }
        ASSERT_TRUE( allAdded );
        ClearJsRunLog();
        ASSERT_TRUE( TAccess::StartManual( *scheduler, error ) );

        // every timed wake lands on a due instant: the minutes 1..10 of the hour
        UInt32 timedWakes = 0;
        bool everyWakeOnAnInstant = true;
        Int64 delay = TAccess::Cycle( *scheduler );
        while ( timedWakes < 10 && delay > 0 )
        {
            clocks.Advance( delay );
            ++timedWakes;
            Int64 now = clocks.utc.GetUtcTimeInMs();
            everyWakeOnAnInstant = everyWakeOnAnInstant && ( 0 == ( now % 60000 ) ) && now == JsUtc( 2, timedWakes );
            TAccess::Cycle( *scheduler );
            CycleUntilIdle( *scheduler, 30000 );
            delay = TAccess::Cycle( *scheduler );
        }
        ASSERT_TRUE( 10 == timedWakes );
        ASSERT_TRUE( everyWakeOnAnInstant );
        ASSERT_TRUE( 1000 == CountJsRuns( "many" ) );
        // the next timed wake is the next hour's minute 1
        ASSERT_TRUE( JsUtc( 3, 1 ) - clocks.utc.GetUtcTimeInMs() == delay );
        runPool->SetDesiredMinNrOfWorkerThreads( 0 );
        DropJsScheduler( scheduler );
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

/** Runs a scheduler with an hourly job to 10:00, keeps the state, and starts another on it at 13:30 */
static UInt32
MissedRunsAfterRestart( CORE::TJobMissedRunPolicy policy, const char* name )
{
    CMemoryJobStateStore store;
    CORE::CString error;
    {
        CManualClocks clocks( JsUtc( 9, 59, 30 ) );
        CORE::JobSchedulerPtr scheduler = MakeJsScheduler( MakeJsSettings( CORE::CString( name ) + "-a" ), &clocks );
        CORE::CJobDefinition job = MakeJsJob( "hourly", Cron( "0 * * * *" ), name );
        job.SetMissedRunPolicy( policy );
        job.SetMaxCatchUpRuns( 3 );
        scheduler->AddJob( job, error );
        scheduler->SetStateStore( &store );
        TAccess::StartManual( *scheduler, error );
        clocks.Advance( TAccess::Cycle( *scheduler ) );
        TAccess::Cycle( *scheduler );
        CycleUntilIdle( *scheduler );
        DropJsScheduler( scheduler );
    }

    CManualClocks clocks( JsUtc( 13, 30 ) );
    CORE::JobSchedulerPtr scheduler = MakeJsScheduler( MakeJsSettings( CORE::CString( name ) + "-b" ), &clocks );
    CORE::CJobDefinition job = MakeJsJob( "hourly", Cron( "0 * * * *" ), name );
    job.SetMissedRunPolicy( policy );
    job.SetMaxCatchUpRuns( 3 );
    scheduler->AddJob( job, error );
    scheduler->SetStateStore( &store );
    TAccess::StartManual( *scheduler, error );
    TAccess::Cycle( *scheduler );
    CycleUntilIdle( *scheduler );
    UInt32 catchUps = CountRunsWithCause( *scheduler, "hourly", CORE::JOBRUNCAUSE_CATCH_UP );
    UInt32 executed = CountExecutedRuns( *scheduler, "hourly" );
    DropJsScheduler( scheduler );
    return catchUps == executed ? catchUps : 1000 + executed;
}

/*-------------------------------------------------------------------------*/

static void
PerformMissedRunTests( void )
{
    GUCEF_TESTFW_TESTCASE( "JS-T2: three occurrences missed across a restart give 0 / 1 / 3 runs for SKIP / RUN_ONCE / RUN_EACH" )
    {
        ASSERT_TRUE( 0 == MissedRunsAfterRestart( CORE::JOBMISSEDRUNPOLICY_SKIP, "js-t2-skip" ) );
        ASSERT_TRUE( 1 == MissedRunsAfterRestart( CORE::JOBMISSEDRUNPOLICY_RUN_ONCE, "js-t2-once" ) );
        ASSERT_TRUE( 3 == MissedRunsAfterRestart( CORE::JOBMISSEDRUNPOLICY_RUN_EACH, "js-t2-each" ) );
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JS-T2: an occurrence found later than maxLatenessMs counts as missed" )
    {
        CORE::CString error;
        for ( UInt32 variant=0; variant<2; ++variant )
        {
            CManualClocks clocks( JsUtc( 4, 0, 30 ) );
            CORE::JobSchedulerPtr scheduler = MakeJsScheduler( MakeJsSettings( 0 == variant ? "js-t2-late" : "js-t2-lenient" ), &clocks );
            CORE::CJobDefinition job = MakeJsJob( "minutely", Cron( "* * * * *" ), "late" );
            job.SetMissedRunPolicy( CORE::JOBMISSEDRUNPOLICY_SKIP );
            job.SetMaxLatenessMs( 0 == variant ? 1000 : 0 );
            scheduler->AddJob( job, error );
            TAccess::StartManual( *scheduler, error );
            TAccess::Cycle( *scheduler );
            clocks.Advance( 30000 + 5000 );      // the dispatcher gets to 04:01 five seconds late
            TAccess::Cycle( *scheduler );
            CycleUntilIdle( *scheduler );
            ASSERT_TRUE( ( 0 == variant ? 0u : 1u ) == CountExecutedRuns( *scheduler, "minutely" ) );
            DropJsScheduler( scheduler );
        }
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

static void
PerformClockTests( void )
{
    GUCEF_TESTFW_TESTCASE( "JS-T3: a clock jump back runs nothing twice, a jump forward follows the missed run policy" )
    {
        CManualClocks clocks( JsUtc( 5, 0, 30 ) );
        CORE::JobSchedulerPtr scheduler = MakeJsScheduler( MakeJsSettings( "js-t3-jump" ), &clocks );
        CORE::CString error;
        CORE::CJobDefinition job = MakeJsJob( "minutely", Cron( "* * * * *" ), "jump" );
        job.SetMissedRunPolicy( CORE::JOBMISSEDRUNPOLICY_RUN_ONCE );
        scheduler->AddJob( job, error );
        TAccess::StartManual( *scheduler, error );
        TAccess::Cycle( *scheduler );
        clocks.Advance( 30000 );                                 // 05:01:00
        TAccess::Cycle( *scheduler );
        CycleUntilIdle( *scheduler );
        ASSERT_TRUE( 1 == CountExecutedRuns( *scheduler, "minutely" ) );

        // the wall clock goes back 20 s (the monotonic clock goes on 1 s)
        clocks.utc.Set( JsUtc( 5, 0, 40 ) );
        clocks.monotonic.Advance( 1000 );
        TAccess::Cycle( *scheduler );
        ASSERT_TRUE( 1 == TAccess::GetClockJumpCount( *scheduler ) );
        clocks.Advance( 20000 );                                 // 05:01:00 again
        TAccess::Cycle( *scheduler );
        CycleUntilIdle( *scheduler );
        ASSERT_TRUE( 1 == CountExecutedRuns( *scheduler, "minutely" ) );
        clocks.Advance( 60000 );                                 // 05:02:00
        TAccess::Cycle( *scheduler );
        CycleUntilIdle( *scheduler );
        ASSERT_TRUE( 2 == CountExecutedRuns( *scheduler, "minutely" ) );

        // forward 3.5 minutes at once: 05:03, 05:04, 05:05 coalesced into one run (RUN_ONCE)
        clocks.utc.Advance( 210000 );
        clocks.monotonic.Advance( 1000 );
        TAccess::Cycle( *scheduler );
        CycleUntilIdle( *scheduler );
        ASSERT_TRUE( 2 == TAccess::GetClockJumpCount( *scheduler ) );
        ASSERT_TRUE( 3 == CountExecutedRuns( *scheduler, "minutely" ) );
        ASSERT_TRUE( JsUtc( 5, 6 ) == GetJobStatus( *scheduler, "minutely" ).nextDueUtcMs );
        DropJsScheduler( scheduler );
    }
    GUCEF_TESTFW_TESTCASE_END;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    GUCEF_TESTFW_TESTCASE( "JS-T3: schedules match in UTC whatever the process time zone" )
    {
        const char* previous = getenv( "TZ" );
        CORE::CString previousTz = GUCEF_NULL != previous ? previous : "";
        Int64 dueInZone[ 2 ] = { 0, 0 };
        const char* zones[ 2 ] = { "UTC", "America/New_York" };
        for ( UInt32 z=0; z<2; ++z )
        {
            setenv( "TZ", zones[ z ], 1 );
            tzset();
            CManualClocks clocks( JsUtc( 24 * 70 ) );            // in March, around the daylight saving change
            CORE::JobSchedulerPtr scheduler = MakeJsScheduler( MakeJsSettings( 0 == z ? "js-t3-utc" : "js-t3-ny" ), &clocks );
            CORE::CString error;
            scheduler->AddJob( MakeJsJob( "daily", Cron( "0 2 * * *" ), "tz" ), error );
            TAccess::StartManual( *scheduler, error );
            TAccess::Cycle( *scheduler );
            dueInZone[ z ] = GetJobStatus( *scheduler, "daily" ).nextDueUtcMs;
            DropJsScheduler( scheduler );
        }
        if ( previousTz.IsNULLOrEmpty() )
            unsetenv( "TZ" );
        else
            setenv( "TZ", previousTz.C_String(), 1 );
        tzset();
        ASSERT_TRUE( JsUtc( 24 * 70 + 2 ) == dueInZone[ 0 ] );
        ASSERT_TRUE( dueInZone[ 0 ] == dueInZone[ 1 ] );
    }
    GUCEF_TESTFW_TESTCASE_END;
    #endif
}

/*-------------------------------------------------------------------------*/

static void
PerformConcurrencyTests( void )
{
    GUCEF_TESTFW_TESTCASE( "JS-T4: a lane with one place starts the highest priority first, the other waits for the lane" )
    {
        CManualClocks clocks( JsUtc( 6 ) );
        CORE::JobSchedulerPtr scheduler = MakeJsScheduler( MakeJsSettings( "js-t4-lane" ), &clocks );
        CORE::CString error;
        CORE::CJobLaneDefinition lane;
        lane.SetName( "single" );
        lane.SetMaxConcurrent( 1 );
        ASSERT_TRUE( scheduler->AddLane( lane, error ) );
        CORE::CJobDefinition low = MakeJsJob( "low", CORE::CJobTrigger::Manual(), "low", 0 );
        low.SetLaneName( "single" );
        low.SetPriority( 2 );
        CORE::CJobDefinition high = MakeJsJob( "high", CORE::CJobTrigger::Manual(), "high", 0 );
        high.SetLaneName( "single" );
        high.SetPriority( 0 );
        ASSERT_TRUE( scheduler->AddJob( low, error ) && scheduler->AddJob( high, error ) );
        TAccess::StartManual( *scheduler, error );
        TAccess::Cycle( *scheduler );

        CORE::SJobControlResult lowResult, highResult;
        ASSERT_TRUE( CORE::JOBCONTROLSTATUS_OK == RunNow( *scheduler, "low", lowResult ) );
        ASSERT_TRUE( CORE::JOBCONTROLSTATUS_OK == RunNow( *scheduler, "high", highResult ) );
        TAccess::Cycle( *scheduler );
        CORE::SJobRunRecord lowRun, highRun;
        scheduler->GetRun( lowResult.runId, lowRun );
        scheduler->GetRun( highResult.runId, highRun );
        ASSERT_TRUE( CORE::JOBRUNSTATE_RUNNING == highRun.state );
        ASSERT_TRUE( CORE::JOBRUNSTATE_WAITING == lowRun.state );
        ASSERT_TRUE( GetJobStatus( *scheduler, "low" ).waitingFor == "lane:single" );
        ASSERT_TRUE( CORE::JOBSTATE_WAITING == GetJobStatus( *scheduler, "low" ).state );

        // the high one ends (cancelled): the low one gets the lane
        CORE::SJobControlCommand cancel;
        cancel.action = CORE::JOBCONTROLACTION_CANCEL_RUN;
        cancel.targetKind = CORE::JOBCONTROLTARGET_RUN;
        cancel.targetId = CORE::ToString( highResult.runId );
        CORE::SJobControlResult cancelResult;
        ASSERT_TRUE( CORE::JOBCONTROLSTATUS_OK == scheduler->Control( cancel, cancelResult ) );
        ASSERT_TRUE( WaitForRunState( *scheduler, highResult.runId, CORE::JOBRUNSTATE_CANCELLED, 5000, true ) );
        ASSERT_TRUE( WaitForRunState( *scheduler, lowResult.runId, CORE::JOBRUNSTATE_RUNNING, 5000, true ) );
        cancel.targetId = CORE::ToString( lowResult.runId );
        scheduler->Control( cancel, cancelResult );
        ASSERT_TRUE( CycleUntilIdle( *scheduler ) );
        DropJsScheduler( scheduler );
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JS-T4: SKIP_IF_RUNNING, QUEUE_ONE and PARALLEL when a job is due while its runs go on" )
    {
        CManualClocks clocks( JsUtc( 7 ) );
        CORE::JobSchedulerPtr scheduler = MakeJsScheduler( MakeJsSettings( "js-t4-concurrency" ), &clocks );
        CORE::CString error;
        CORE::CJobDefinition skip = MakeJsJob( "skip", CORE::CJobTrigger::Interval( 1000 ), "c", 0 );
        CORE::CJobDefinition queue = MakeJsJob( "queue", CORE::CJobTrigger::Interval( 1000 ), "c", 0 );
        queue.SetConcurrency( CORE::JOBCONCURRENCY_QUEUE_ONE );
        CORE::CJobDefinition parallel = MakeJsJob( "parallel", CORE::CJobTrigger::Interval( 1000 ), "c", 0 );
        parallel.SetConcurrency( CORE::JOBCONCURRENCY_PARALLEL );
        parallel.SetMaxParallelRuns( 2 );
        scheduler->AddJob( skip, error );
        scheduler->AddJob( queue, error );
        scheduler->AddJob( parallel, error );
        TAccess::StartManual( *scheduler, error );
        TAccess::Cycle( *scheduler );
        for ( UInt32 i=0; i<3; ++i )
        {
            clocks.Advance( 1000 );
            TAccess::Cycle( *scheduler );
        }
        // the runs never end by themselves (cycles 0): three due instants each
        CORE::SJobStatus skipStatus = GetJobStatus( *scheduler, "skip" );
        CORE::SJobStatus queueStatus = GetJobStatus( *scheduler, "queue" );
        CORE::SJobStatus parallelStatus = GetJobStatus( *scheduler, "parallel" );
        ASSERT_TRUE( 1 == skipStatus.activeRunCount );
        ASSERT_TRUE( 2 == CountJobRunsInState( *scheduler, "skip", CORE::JOBRUNSTATE_SKIPPED ) );
        ASSERT_TRUE( 2 == queueStatus.activeRunCount );        // one running, one pending
        ASSERT_TRUE( 1 == CountJobRunsInState( *scheduler, "queue", CORE::JOBRUNSTATE_SKIPPED ) );
        ASSERT_TRUE( 2 == parallelStatus.activeRunCount );
        ASSERT_TRUE( 2 == CountJobRunsInState( *scheduler, "parallel", CORE::JOBRUNSTATE_RUNNING ) );
        ASSERT_TRUE( 1 == CountJobRunsInState( *scheduler, "parallel", CORE::JOBRUNSTATE_SKIPPED ) );
        DropJsScheduler( scheduler );
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

static void
PerformControlTests( void )
{
    GUCEF_TESTFW_TESTCASE( "JS-T5: run-now honours the lane throttle, a disabled job refuses it, cancel of a queued run" )
    {
        CManualClocks clocks( JsUtc( 8 ) );
        CORE::JobSchedulerPtr scheduler = MakeJsScheduler( MakeJsSettings( "js-t5-control" ), &clocks );
        CORE::CString error;
        CJsCountingThrottle throttle;
        throttle.refuse = true;
        CORE::CJobLaneDefinition lane;
        lane.SetName( "api" );
        lane.SetThrottlePolicy( &throttle );
        scheduler->AddLane( lane, error );
        CORE::CJobDefinition job = MakeJsJob( "fetch", CORE::CJobTrigger::Manual(), "fetch" );
        job.SetLaneName( "api" );
        scheduler->AddJob( job, error );
        TAccess::StartManual( *scheduler, error );
        TAccess::Cycle( *scheduler );

        CORE::SJobControlResult result;
        ASSERT_TRUE( CORE::JOBCONTROLSTATUS_OK == RunNow( *scheduler, "fetch", result ) );
        Int64 delay = TAccess::Cycle( *scheduler );
        CORE::SJobRunRecord run;
        scheduler->GetRun( result.runId, run );
        ASSERT_TRUE( CORE::JOBRUNSTATE_WAITING == run.state );
        ASSERT_TRUE( GetJobStatus( *scheduler, "fetch" ).waitingFor == "throttle:testLimit" );
        ASSERT_TRUE( 5000 == delay );
        ASSERT_TRUE( throttle.checks >= 1 );

        // cancel of the waiting run: it ends at once, never started
        CORE::SJobControlCommand cancel;
        cancel.action = CORE::JOBCONTROLACTION_CANCEL_RUN;
        cancel.targetKind = CORE::JOBCONTROLTARGET_RUN;
        cancel.targetId = CORE::ToString( result.runId );
        CORE::SJobControlResult cancelResult;
        ASSERT_TRUE( CORE::JOBCONTROLSTATUS_OK == scheduler->Control( cancel, cancelResult ) );
        scheduler->GetRun( result.runId, run );
        ASSERT_TRUE( CORE::JOBRUNSTATE_CANCELLED == run.state );
        ASSERT_TRUE( 0 == run.taskId );

        // run-now of the lane: the policy clears its own wait (OnRunNow), the next run goes
        CORE::SJobControlCommand laneRunNow;
        laneRunNow.action = CORE::JOBCONTROLACTION_RUN_NOW;
        laneRunNow.targetKind = CORE::JOBCONTROLTARGET_LANE;
        laneRunNow.targetId = "api";
        ASSERT_TRUE( CORE::JOBCONTROLSTATUS_OK == scheduler->Control( laneRunNow, cancelResult ) );
        ASSERT_TRUE( 1 == throttle.runNows );
        ASSERT_TRUE( CORE::JOBCONTROLSTATUS_OK == RunNow( *scheduler, "fetch", result ) );
        ASSERT_TRUE( WaitForRunState( *scheduler, result.runId, CORE::JOBRUNSTATE_SUCCEEDED, 5000, true ) );
        ASSERT_TRUE( 1 == throttle.started );
        ASSERT_TRUE( 1 == throttle.finished );

        // disabled: run-now refused
        ASSERT_TRUE( CORE::JOBCONTROLSTATUS_OK == SimpleControl( *scheduler, CORE::JOBCONTROLACTION_DISABLE, CORE::JOBCONTROLTARGET_JOB, "fetch" ) );
        ASSERT_TRUE( CORE::JOBSTATE_DISABLED == GetJobStatus( *scheduler, "fetch" ).state );
        ASSERT_TRUE( CORE::JOBCONTROLSTATUS_FAILED_PRECONDITION == RunNow( *scheduler, "fetch", result ) );
        ASSERT_TRUE( CORE::JOBCONTROLSTATUS_NOT_FOUND == RunNow( *scheduler, "nothing", result ) );
        ASSERT_TRUE( CORE::JOBCONTROLSTATUS_UNIMPLEMENTED == SimpleControl( *scheduler, (CORE::TJobControlAction) 1, CORE::JOBCONTROLTARGET_JOB, "fetch" ) );
        ASSERT_TRUE( CycleUntilIdle( *scheduler ) );
        DropJsScheduler( scheduler );
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JS-T5: cancel of a running run, set priority, enable and disable are kept across a restart" )
    {
        CMemoryJobStateStore store;
        CORE::CString error;
        {
            CManualClocks clocks( JsUtc( 9 ) );
            CORE::JobSchedulerPtr scheduler = MakeJsScheduler( MakeJsSettings( "js-t5-persist-a" ), &clocks );
            scheduler->SetStateStore( &store );
            scheduler->AddJob( MakeJsJob( "long", CORE::CJobTrigger::Manual(), "long", 0 ), error );
            scheduler->AddJob( MakeJsJob( "other", CORE::CJobTrigger::Manual(), "other" ), error );
            TAccess::StartManual( *scheduler, error );
            TAccess::Cycle( *scheduler );
            CORE::SJobControlResult result;
            RunNow( *scheduler, "long", result );
            ASSERT_TRUE( WaitForRunState( *scheduler, result.runId, CORE::JOBRUNSTATE_RUNNING, 5000, true ) );
            CORE::SJobControlCommand cancel;
            cancel.action = CORE::JOBCONTROLACTION_CANCEL_RUN;
            cancel.targetKind = CORE::JOBCONTROLTARGET_RUN;
            cancel.targetId = CORE::ToString( result.runId );
            CORE::SJobControlResult cancelResult;
            ASSERT_TRUE( CORE::JOBCONTROLSTATUS_OK == scheduler->Control( cancel, cancelResult ) );
            ASSERT_TRUE( WaitForRunState( *scheduler, result.runId, CORE::JOBRUNSTATE_CANCELLED, 5000, true ) );

            CORE::SJobControlCommand priority;
            priority.action = CORE::JOBCONTROLACTION_SET_PRIORITY;
            priority.targetKind = CORE::JOBCONTROLTARGET_JOB;
            priority.targetId = "other";
            priority.hasPriority = true;
            priority.priority = 3;
            ASSERT_TRUE( CORE::JOBCONTROLSTATUS_OK == scheduler->Control( priority, cancelResult ) );
            priority.priority = 9;
            ASSERT_TRUE( CORE::JOBCONTROLSTATUS_INVALID_ARGUMENT == scheduler->Control( priority, cancelResult ) );
            ASSERT_TRUE( CORE::JOBCONTROLSTATUS_OK == SimpleControl( *scheduler, CORE::JOBCONTROLACTION_DISABLE, CORE::JOBCONTROLTARGET_JOB, "long" ) );
            DropJsScheduler( scheduler );
        }
        ASSERT_TRUE( store.GetSaveCount() >= 1 );

        CManualClocks clocks( JsUtc( 10 ) );
        CORE::JobSchedulerPtr scheduler = MakeJsScheduler( MakeJsSettings( "js-t5-persist-b" ), &clocks );
        scheduler->SetStateStore( &store );
        scheduler->AddJob( MakeJsJob( "long", CORE::CJobTrigger::Manual(), "long", 0 ), error );
        scheduler->AddJob( MakeJsJob( "other", CORE::CJobTrigger::Manual(), "other" ), error );
        TAccess::StartManual( *scheduler, error );
        TAccess::Cycle( *scheduler );
        ASSERT_TRUE( CORE::JOBSTATE_DISABLED == GetJobStatus( *scheduler, "long" ).state );
        ASSERT_TRUE( 3 == GetJobStatus( *scheduler, "other" ).priority );
        ASSERT_TRUE( CORE::JOBCONTROLSTATUS_OK == SimpleControl( *scheduler, CORE::JOBCONTROLACTION_ENABLE, CORE::JOBCONTROLTARGET_JOB, "long" ) );
        ASSERT_TRUE( CORE::JOBSTATE_SCHEDULED == GetJobStatus( *scheduler, "long" ).state );
        DropJsScheduler( scheduler );
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JS-T5: a lane hold waits for the runs in flight and tells the throttle policy what the holder spent" )
    {
        CManualClocks clocks( JsUtc( 11 ) );
        CORE::JobSchedulerPtr scheduler = MakeJsScheduler( MakeJsSettings( "js-t5-hold" ), &clocks );
        CORE::CString error;
        CJsCountingThrottle throttle;
        CORE::CJobLaneDefinition lane;
        lane.SetName( "held" );
        lane.SetThrottlePolicy( &throttle );
        scheduler->AddLane( lane, error );
        CORE::CJobDefinition job = MakeJsJob( "busy", CORE::CJobTrigger::Manual(), "busy", 0 );
        job.SetLaneName( "held" );
        scheduler->AddJob( job, error );
        TAccess::StartManual( *scheduler, error );
        TAccess::Cycle( *scheduler );
        CORE::SJobControlResult result;
        RunNow( *scheduler, "busy", result );
        TAccess::Cycle( *scheduler );
        CORE::CString refusal;
        ASSERT_FALSE( scheduler->TryHoldLane( "held", "cli", refusal ) );
        SimpleControl( *scheduler, CORE::JOBCONTROLACTION_CANCEL_RUN, CORE::JOBCONTROLTARGET_JOB, "busy" );
        ASSERT_TRUE( CycleUntilIdle( *scheduler ) );
        {
            CORE::CJobSchedulerLaneHoldScope hold( *scheduler, "held", "cli" );
            ASSERT_TRUE( hold.IsHeld() );
            hold.SetActualCost( 7 );
            RunNow( *scheduler, "busy", result );
            TAccess::Cycle( *scheduler );
            ASSERT_TRUE( GetJobStatus( *scheduler, "busy" ).waitingFor == "hold:cli" );
        }
        ASSERT_TRUE( 7 == throttle.holdCost );
        ASSERT_TRUE( WaitForRunState( *scheduler, result.runId, CORE::JOBRUNSTATE_RUNNING, 5000, true ) );
        SimpleControl( *scheduler, CORE::JOBCONTROLACTION_CANCEL_RUN, CORE::JOBCONTROLTARGET_JOB, "busy" );
        ASSERT_TRUE( CycleUntilIdle( *scheduler ) );
        DropJsScheduler( scheduler );
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

static void
PerformRetryTests( void )
{
    GUCEF_TESTFW_TESTCASE( "JS-T6: a failed run is retried after a doubling backoff within the jitter bounds" )
    {
        CManualClocks clocks( JsUtc( 12 ) );
        CORE::JobSchedulerPtr scheduler = MakeJsScheduler( MakeJsSettings( "js-t6-retry" ), &clocks );
        CORE::CString error;
        CORE::CJobDefinition job = MakeJsJob( "flaky", CORE::CJobTrigger::Manual(), "flaky", 1, 0, true );
        CORE::CJobRetryPolicy retry;
        retry.SetMaxAttempts( 3 );
        retry.SetBackoffInitialMs( 1000 );
        retry.SetBackoffMaxMs( 4000 );
        retry.SetJitterPercent( 20 );
        job.SetRetryPolicy( retry );
        scheduler->AddJob( job, error );
        TAccess::StartManual( *scheduler, error );
        TAccess::Cycle( *scheduler );
        CORE::SJobControlResult result;
        RunNow( *scheduler, "flaky", result );

        Int64 expectedLow[ 2 ] = { 800, 1600 };
        Int64 expectedHigh[ 2 ] = { 1200, 2400 };
        for ( UInt32 attempt=1; attempt<=2; ++attempt )
        {
            // the attempt fails, the next one waits for its backoff
            bool waiting = false;
            for ( UInt32 i=0; i<2000 && !waiting; ++i )
            {
                TAccess::Cycle( *scheduler );
                waiting = GetJobStatus( *scheduler, "flaky" ).waitingFor == "backoff";
                if ( !waiting )
                    MT::ThreadDelay( 2 );
            }
            ASSERT_TRUE( waiting );
            Int64 waitMs = GetJobStatus( *scheduler, "flaky" ).waitingUntilUtcMs - clocks.utc.GetUtcTimeInMs();
            ASSERT_TRUE( waitMs >= expectedLow[ attempt-1 ] && waitMs <= expectedHigh[ attempt-1 ] );
            clocks.Advance( waitMs );
        }
        ASSERT_TRUE( CycleUntilIdle( *scheduler ) );
        CORE::TJobRunRecordVector runs;
        GetJobRuns( *scheduler, "flaky", runs );
        ASSERT_TRUE( 3 == runs.size() );
        ASSERT_TRUE( 3 == CountJobRunsInState( *scheduler, "flaky", CORE::JOBRUNSTATE_FAILED ) );
        ASSERT_TRUE( 2 == CountRunsWithCause( *scheduler, "flaky", CORE::JOBRUNCAUSE_RETRY ) );
        ASSERT_TRUE( 3 == runs[ 0 ].attempt );
        ASSERT_TRUE( 3 == GetJobStatus( *scheduler, "flaky" ).consecutiveFailures );
        DropJsScheduler( scheduler );
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JS-T6: a run that passes its timeout is cancelled cooperatively and ends TIMED_OUT" )
    {
        CManualClocks clocks( JsUtc( 13 ) );
        CORE::JobSchedulerPtr scheduler = MakeJsScheduler( MakeJsSettings( "js-t6-timeout" ), &clocks );
        CORE::CString error;
        CORE::CJobDefinition job = MakeJsJob( "slow", CORE::CJobTrigger::Manual(), "slow", 0 );
        job.SetTimeoutMs( 30000 );
        scheduler->AddJob( job, error );
        TAccess::StartManual( *scheduler, error );
        TAccess::Cycle( *scheduler );
        CORE::SJobControlResult result;
        RunNow( *scheduler, "slow", result );
        ASSERT_TRUE( WaitForRunState( *scheduler, result.runId, CORE::JOBRUNSTATE_RUNNING, 5000, true ) );
        ASSERT_TRUE( 30000 == TAccess::Cycle( *scheduler ) );
        clocks.Advance( 30000 );
        ASSERT_TRUE( WaitForRunState( *scheduler, result.runId, CORE::JOBRUNSTATE_TIMED_OUT, 5000, true ) );
        DropJsScheduler( scheduler );
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

static void
PerformWorkSourceTests( void )
{
    GUCEF_TESTFW_TESTCASE( "JS-T8: work source items are dispatched by lane and priority, every outcome comes back, its next wake is honoured" )
    {
        CManualClocks clocks( JsUtc( 14 ) );
        CORE::JobSchedulerPtr scheduler = MakeJsScheduler( MakeJsSettings( "js-t8-source" ), &clocks );
        CORE::CString error;
        CORE::CJobLaneDefinition lane;
        lane.SetName( "provider" );
        lane.SetMaxConcurrent( 1 );
        scheduler->AddLane( lane, error );
        CJsWorkSource source;
        source.nextWakeUtcMs = JsUtc( 14, 0, 42 );
        CORE::CJobDefinition job = MakeJsJob( "planner", CORE::CJobTrigger::WorkSource(), "planner" );
        job.SetLaneName( "provider" );
        ASSERT_TRUE( scheduler->AddWorkSourceJob( job, &source, error ) );

        const char* keys[ 3 ] = { "late", "urgent", "failing" };
        UInt32 priorities[ 3 ] = { 3, 0, 1 };
        for ( UInt32 i=0; i<3; ++i )
        {
            CORE::SJobWorkItem item;
            item.key = keys[ i ];
            item.publicLabel = CORE::CString( "item " ) + keys[ i ];
            item.priority = priorities[ i ];
            item.estimatedCost = 1;
            item.payload.SetNodeType( GUCEF_DATATYPE_OBJECT );
            item.payload.SetAttribute( "value", CORE::CString( "item-" ) + keys[ i ] );
            item.payload.SetAttribute( "fail", 2 == i );
            source.AddItem( item );
        }
        // a duplicate key is refused while the first is pending
        CORE::SJobWorkItem duplicate;
        duplicate.key = "late";
        duplicate.payload.SetAttribute( "value", "dup" );
        source.AddItem( duplicate );

        ClearJsRunLog();
        TAccess::StartManual( *scheduler, error );
        Int64 delay = TAccess::Cycle( *scheduler );
        ASSERT_TRUE( 1 == source.evaluations );
        ASSERT_TRUE( 42000 == delay );
        ASSERT_TRUE( CycleUntilIdle( *scheduler ) );
        ASSERT_TRUE( 3 == source.FinishedCount() );
        ASSERT_TRUE( CORE::JOBRUNSTATE_SUCCEEDED == source.FinishedState( "urgent" ) );
        ASSERT_TRUE( CORE::JOBRUNSTATE_SUCCEEDED == source.FinishedState( "late" ) );
        ASSERT_TRUE( CORE::JOBRUNSTATE_FAILED == source.FinishedState( "failing" ) );
        TJsRunLog log;
        GetJsRunLog( log );
        ASSERT_TRUE( 3 == log.size() );
        if ( 3 == log.size() )
        {
            // one at a time on the lane, in priority order
            ASSERT_TRUE( log[ 0 ].value == "item-urgent" );
            ASSERT_TRUE( log[ 1 ].value == "item-failing" );
            ASSERT_TRUE( log[ 2 ].value == "item-late" );
        }
        CORE::SJobControlCommand laneRunNow;
        laneRunNow.action = CORE::JOBCONTROLACTION_RUN_NOW;
        laneRunNow.targetKind = CORE::JOBCONTROLTARGET_LANE;
        laneRunNow.targetId = "provider";
        CORE::SJobControlResult result;
        ASSERT_TRUE( CORE::JOBCONTROLSTATUS_OK == scheduler->Control( laneRunNow, result ) );
        ASSERT_TRUE( 1 == source.runNowLanes.size() );
        DropJsScheduler( scheduler );
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JS-T8: the status of a job's work source is read through the scheduler (GetJob with the source status)" )
    {
        CManualClocks clocks( JsUtc( 9 ) );
        CORE::JobSchedulerPtr scheduler = MakeJsScheduler( MakeJsSettings( "js-t8-status" ), &clocks );
        CORE::CString error;
        CJsWorkSource source;
        ASSERT_TRUE( scheduler->AddWorkSourceJob( MakeJsJob( "planner", CORE::CJobTrigger::WorkSource(), "planner" ), &source, error ) );
        ASSERT_TRUE( scheduler->AddJob( MakeJsJob( "plain", CORE::CJobTrigger::Manual(), "plain" ), error ) );
        CORE::SJobWorkItem item;
        item.key = "waiting";
        item.payload.SetNodeType( GUCEF_DATATYPE_OBJECT );
        item.payload.SetAttribute( "value", "item-waiting" );
        source.AddItem( item );

        // before any evaluation: the queue of the source is in the status when asked for
        CORE::CDataNode status;
        ASSERT_TRUE( scheduler->GetWorkSourceStatus( "planner", status, true ) );
        ASSERT_TRUE( 0 == status.GetAttributeValueOrChildValueByName( "evaluations" ).AsUInt32( 99 ) );
        ASSERT_TRUE( JsUtc( 9 ) == status.GetAttributeValueOrChildValueByName( "nowUtcMs" ).AsInt64( 0 ) );
        const CORE::CDataNode* queue = status.FindChild( "queue" );
        ASSERT_TRUE( GUCEF_NULL != queue && 1 == queue->GetNrOfChildNodes() );
        ASSERT_TRUE( scheduler->GetWorkSourceStatus( "planner", status, false ) );
        ASSERT_TRUE( GUCEF_NULL == status.FindChild( "queue" ) );

        ClearJsRunLog();
        TAccess::StartManual( *scheduler, error );
        TAccess::Cycle( *scheduler );
        ASSERT_TRUE( CycleUntilIdle( *scheduler ) );
        ASSERT_TRUE( scheduler->GetWorkSourceStatus( "planner", status, false ) );
        ASSERT_TRUE( 1 <= status.GetAttributeValueOrChildValueByName( "evaluations" ).AsUInt32( 0 ) );
        ASSERT_TRUE( 1 == status.GetAttributeValueOrChildValueByName( "finished" ).AsUInt32( 0 ) );

        // a job without a work source and an unknown job have none
        ASSERT_FALSE( scheduler->GetWorkSourceStatus( "plain", status, true ) );
        ASSERT_TRUE( 0 == status.GetNrOfChildNodes() && 0 == status.GetAttCount() );
        ASSERT_FALSE( scheduler->GetWorkSourceStatus( "nobody", status, true ) );
        DropJsScheduler( scheduler );
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

static void
PerformStateStoreTests( void )
{
    GUCEF_TESTFW_TESTCASE( "JS-T9: the state save is debounced on the monotonic clock, also when the calendar clock goes back" )
    {
        CManualClocks clocks( JsUtc( 15 ) );
        CMemoryJobStateStore store;
        CORE::CJobSchedulerSettings settings = MakeJsSettings( "js-t9-debounce" );
        settings.SetStateSaveDebounceMs( 2000 );
        CORE::JobSchedulerPtr scheduler = MakeJsScheduler( settings, &clocks );
        scheduler->SetStateStore( &store );
        CORE::CString error;
        scheduler->AddJob( MakeJsJob( "job", CORE::CJobTrigger::Manual(), "job" ), error );
        TAccess::StartManual( *scheduler, error );
        TAccess::Cycle( *scheduler );
        ASSERT_TRUE( 1 == store.GetLoadCount() );
        UInt32 savesBefore = store.GetSaveCount();
        SimpleControl( *scheduler, CORE::JOBCONTROLACTION_DISABLE, CORE::JOBCONTROLTARGET_JOB, "job" );
        ASSERT_TRUE( 2000 == TAccess::Cycle( *scheduler ) );
        clocks.monotonic.Advance( 1999 );
        clocks.utc.Advance( -86400000 );                     // the calendar clock goes back a day
        TAccess::Cycle( *scheduler );
        ASSERT_TRUE( savesBefore == store.GetSaveCount() );
        clocks.monotonic.Advance( 1 );
        TAccess::Cycle( *scheduler );
        ASSERT_TRUE( savesBefore + 1 == store.GetSaveCount() );
        DropJsScheduler( scheduler );
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JS-T9: a failed save is tried again after the larger of the debounce and 5 s without another change" )
    {
        CManualClocks clocks( JsUtc( 16 ) );
        CMemoryJobStateStore store;
        store.SetFailSaves( 1 );
        CORE::CJobSchedulerSettings settings = MakeJsSettings( "js-t9-retry" );
        settings.SetStateSaveDebounceMs( 1000 );
        CORE::JobSchedulerPtr scheduler = MakeJsScheduler( settings, &clocks );
        scheduler->SetStateStore( &store );
        CORE::CString error;
        scheduler->AddJob( MakeJsJob( "job", CORE::CJobTrigger::Manual(), "job" ), error );
        TAccess::StartManual( *scheduler, error );
        TAccess::Cycle( *scheduler );
        SimpleControl( *scheduler, CORE::JOBCONTROLACTION_DISABLE, CORE::JOBCONTROLTARGET_JOB, "job" );
        TAccess::Cycle( *scheduler );
        clocks.Advance( 1000 );
        Int64 delay = TAccess::Cycle( *scheduler );            // the save fails
        ASSERT_TRUE( 0 == store.GetSaveCount() );
        ASSERT_TRUE( 5000 == delay );
        clocks.Advance( 5000 );
        TAccess::Cycle( *scheduler );
        ASSERT_TRUE( 1 == store.GetSaveCount() );
        DropJsScheduler( scheduler );
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

static void
PerformIdleTests( void )
{
    GUCEF_TESTFW_TESTCASE( "JS-T10: nothing due for 24 simulated hours with maxSleepMs 15 min gives 96 dispatcher wakes" )
    {
        CManualClocks clocks( JsUtc( 24 ) );
        CORE::CJobSchedulerSettings settings = MakeJsSettings( "js-t10-idle" );
        settings.SetMaxSleepMs( 900000 );
        CORE::JobSchedulerPtr scheduler = MakeJsScheduler( settings, &clocks );
        CORE::CString error;
        scheduler->AddJob( MakeJsJob( "manual", CORE::CJobTrigger::Manual(), "idle" ), error );
        TAccess::StartManual( *scheduler, error );
        Int64 delay = TAccess::Cycle( *scheduler );
        UInt32 wakes = 0;
        Int64 end = JsUtc( 48 );
        while ( clocks.utc.GetUtcTimeInMs() + delay <= end && delay > 0 )
        {
            clocks.Advance( delay );
            delay = TAccess::Cycle( *scheduler );
            ++wakes;
        }
        ASSERT_TRUE( 96 == wakes );
        DropJsScheduler( scheduler );
    }
    GUCEF_TESTFW_TESTCASE_END;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    GUCEF_TESTFW_TESTCASE( "JS-T10: the dispatcher thread makes no voluntary context switch in 5 s with nothing due" )
    {
        CORE::CJobSchedulerSettings settings = MakeJsSettings( "js-t10-real" );
        settings.SetDispatcherPoolName( "test.js.dispatch.idle" );
        settings.SetMaxSleepMs( 900000 );
        CORE::JobSchedulerPtr scheduler = MakeJsScheduler( settings, GUCEF_NULL );
        CORE::CString error;
        scheduler->AddJob( MakeJsJob( "manual", CORE::CJobTrigger::Manual(), "idle" ), error );
        ASSERT_TRUE( scheduler->Start( error ) );
        // the dispatcher made its first cycle and went to sleep
        UInt64 threadId = 0;
        for ( UInt32 i=0; i<2000 && 0 == threadId; ++i )
        {
            threadId = TAccess::GetDispatcherSystemThreadId( *scheduler );
            MT::ThreadDelay( 1 );
        }
        ASSERT_TRUE( 0 != threadId );
        MT::ThreadDelay( 200 );
        UInt64 cyclesBefore = TAccess::GetCycleCount( *scheduler );
        Int64 switchesBefore = VoluntaryContextSwitches( threadId );
        MT::ThreadDelay( 5000 );
        Int64 switchesAfter = VoluntaryContextSwitches( threadId );
        ASSERT_TRUE( switchesBefore >= 0 );
        ASSERT_TRUE( switchesAfter == switchesBefore );
        ASSERT_TRUE( cyclesBefore == TAccess::GetCycleCount( *scheduler ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "JobScheduler idle test: voluntary context switches of the dispatcher thread in 5 s: " << ( switchesAfter - switchesBefore ) );

        // a wake from another thread is taken at once
        scheduler->RequestDispatch();
        bool woke = false;
        for ( UInt32 i=0; i<1000 && !woke; ++i )
        {
            woke = TAccess::GetCycleCount( *scheduler ) > cyclesBefore;
            MT::ThreadDelay( 1 );
        }
        ASSERT_TRUE( woke );
        DropJsScheduler( scheduler );
    }
    GUCEF_TESTFW_TESTCASE_END;
    #endif
}

/*-------------------------------------------------------------------------*/

static void
PerformThreadTests( void )
{
    GUCEF_TESTFW_TESTCASE( "JS-T11: runs on the run pool, the dispatcher on its own thread, events on the main pulse generator's thread" )
    {
        CORE::PulseGeneratorPtr mainPulse = CORE::CPulseGenerator::CreateSharedObj();
        CORE::CJobSchedulerSettings settings = MakeJsSettings( "js-t11-threads" );
        CORE::JobSchedulerPtr scheduler = MakeJsScheduler( settings, GUCEF_NULL, mainPulse );
        CJsEventCounter events;
        scheduler->Subscribe( &events );
        CORE::CString error;
        scheduler->AddJob( MakeJsJob( "work", CORE::CJobTrigger::Manual(), "threads" ), error );
        ClearJsRunLog();
        ASSERT_TRUE( scheduler->Start( error ) );

        CORE::SJobControlResult result;
        RunNow( *scheduler, "work", result );
        ASSERT_TRUE( WaitForRunState( *scheduler, result.runId, CORE::JOBRUNSTATE_SUCCEEDED, 5000, false ) );
        TJsRunLog log;
        GetJsRunLog( log );
        ASSERT_TRUE( 1 == log.size() );
        UInt64 dispatcherThread = TAccess::GetDispatcherSystemThreadId( *scheduler );
        if ( 1 == log.size() )
        {
            ASSERT_TRUE( log[ 0 ].systemThreadId != (UInt64) MT::GetCurrentThreadSystemID() );
            ASSERT_TRUE( log[ 0 ].systemThreadId != dispatcherThread );
            CORE::CThreadInfo threadInfo;
            CORE::CString poolName;
            // the thread that ran it belongs to the run pool
            ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadInfo( (TSystemThreadId) log[ 0 ].systemThreadId, threadInfo, poolName ) );
            ASSERT_TRUE( poolName == settings.GetRunPoolName() );
        }
        ASSERT_TRUE( dispatcherThread != (UInt64) MT::GetCurrentThreadSystemID() );

        // events: none until the main pulse generator pulses, then on its thread (this one)
        ASSERT_TRUE( 0 == events.runChanges );
        mainPulse->ForceDirectPulse();
        ASSERT_TRUE( events.runChanges > 0 );
        ASSERT_TRUE( events.jobChanges > 0 );
        ASSERT_TRUE( events.hostChanges > 0 );
        ASSERT_FALSE( events.otherThread );
        DropJsScheduler( scheduler );
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JS-T11: control from 8 threads during dispatch" )
    {
        CORE::CJobSchedulerSettings settings = MakeJsSettings( "js-t11-control" );
        settings.SetDefaultLaneMaxConcurrent( 8 );
        CORE::JobSchedulerPtr scheduler = MakeJsScheduler( settings, GUCEF_NULL );
        CORE::CString error;
        CORE::CJobDefinition job = MakeJsJob( "busy", CORE::CJobTrigger::Manual(), "eight" );
        job.SetConcurrency( CORE::JOBCONCURRENCY_PARALLEL );
        job.SetMaxParallelRuns( 8 );
        scheduler->AddJob( job, error );
        ClearJsRunLog();
        ASSERT_TRUE( scheduler->Start( error ) );

        CJsControlThread* threads[ 8 ];
        for ( UInt32 i=0; i<8; ++i )
        {
            threads[ i ] = GUCEF_NEW CJsControlThread( scheduler.GetPointerAlways(), "busy" );
            threads[ i ]->Activate();
        }
        UInt32 ok = 0;
        for ( UInt32 i=0; i<8; ++i )
        {
            threads[ i ]->WaitForThreadToFinish( 30000 );
            ok += threads[ i ]->GetOk();
            GUCEF_DELETE threads[ i ];
        }
        ASSERT_TRUE( 200 == ok );
        ASSERT_TRUE( WaitUntilIdle( *scheduler, 60000 ) );
        ASSERT_TRUE( 200 == CountJsRuns( "eight" ) );
        CORE::CJobSchedulerSnapshot snapshot;
        scheduler->GetSnapshot( snapshot );
        ASSERT_TRUE( 200 == snapshot.GetTotalRunsSucceeded() );
        DropJsScheduler( scheduler );
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

static void
PerformStopTests( void )
{
    GUCEF_TESTFW_TESTCASE( "JS-T12: stop with runs in flight: cooperative runs end within the stop timeout, the state is saved" )
    {
        CMemoryJobStateStore store;
        CORE::CJobSchedulerSettings settings = MakeJsSettings( "js-t12-stop" );
        settings.SetStopTimeoutMs( 5000 );
        CORE::JobSchedulerPtr scheduler = MakeJsScheduler( settings, GUCEF_NULL );
        scheduler->SetStateStore( &store );
        CORE::CString error;
        CORE::CJobDefinition job = MakeJsJob( "endless", CORE::CJobTrigger::Manual(), "stop", 0 );
        job.SetConcurrency( CORE::JOBCONCURRENCY_PARALLEL );
        job.SetMaxParallelRuns( 3 );
        scheduler->AddJob( job, error );
        ASSERT_TRUE( scheduler->Start( error ) );
        CORE::SJobControlResult results[ 3 ];
        for ( UInt32 i=0; i<3; ++i )
            RunNow( *scheduler, "endless", results[ i ] );
        for ( UInt32 i=0; i<3; ++i )
            ASSERT_TRUE( WaitForRunState( *scheduler, results[ i ].runId, CORE::JOBRUNSTATE_RUNNING, 5000, false ) );

        UInt64 start = CORE::CSystemMonotonicClock::Instance().GetMonotonicTimeInMs();
        scheduler->Stop();
        UInt64 stopMs = CORE::CSystemMonotonicClock::Instance().GetMonotonicTimeInMs() - start;
        ASSERT_TRUE( stopMs < 5000 );
        ASSERT_TRUE( CORE::JOBSCHEDULERSTATE_STOPPED == scheduler->GetHostState() );
        ASSERT_TRUE( scheduler->IsIdle() );
        ASSERT_TRUE( 3 == CountJobRunsInState( *scheduler, "endless", CORE::JOBRUNSTATE_CANCELLED ) );
        ASSERT_TRUE( store.GetSaveCount() >= 1 );
        ASSERT_FALSE( TAccess::IsDispatcherActive( *scheduler ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "JobScheduler stop test: stopped with 3 runs in flight in " << stopMs << " ms" );
        DropJsScheduler( scheduler );
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

static void
PerformMainThreadTests( void )
{
    GUCEF_TESTFW_TESTCASE( "JS-T15: a pool without threads keeps queued tasks for CarryOutQueuedTasksIfAny()" )
    {
        CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
        CORE::ThreadPoolPtr pool = taskManager.GetOrCreateThreadPool( "test.js.zero-threads" );
        pool->SetDesiredMaxTotalNrOfThreads( 0 );
        ClearJsRunLog();
        CJsTestTaskData data;
        data.value = "zero";
        CORE::CFutureResult future = pool->QueueTask( CJsTestTaskData::TaskType, &data );
        ASSERT_TRUE( future.HasAFuture() );
        MT::ThreadDelay( 200 );
        ASSERT_TRUE( 1 == pool->GetNrOfQueuedTasks() );
        ASSERT_TRUE( 0 == pool->GetActiveNrOfThreads() );
        ASSERT_TRUE( 0 == CountJsRuns( "zero" ) );
        ASSERT_TRUE( 1 == pool->CarryOutQueuedTasksIfAny( 5 ) );
        TJsRunLog log;
        GetJsRunLog( log );
        ASSERT_TRUE( 1 == log.size() && log[ 0 ].systemThreadId == (UInt64) MT::GetCurrentThreadSystemID() );
        taskManager.UnregisterThreadPool( pool );
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JS-T15: (characterisation) a pool with the default sizes starts a worker thread for a queued task" )
    {
        CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
        CORE::ThreadPoolPtr pool = taskManager.GetOrCreateThreadPool( "test.js.default-sizes" );
        ClearJsRunLog();
        CJsTestTaskData data;
        data.value = "default";
        pool->QueueTask( CJsTestTaskData::TaskType, &data );
        bool ran = false;
        for ( UInt32 i=0; i<2500 && !ran; ++i )
        {
            ran = 1 == CountJsRuns( "default" );
            MT::ThreadDelay( 2 );
        }
        ASSERT_TRUE( ran );
        pool->SetDesiredMinNrOfWorkerThreads( 0 );
        pool->SetDesiredMaxTotalNrOfThreads( 0 );
        // a pool with threads is unregistered once they stopped (its threads reference it)
        pool->RequestAllThreadsToStop( true, false, 5000 );
        taskManager.UnregisterThreadPool( pool );
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JS-T15: MAIN_THREAD runs are carried out by the main pulse generator's thread, the dispatcher never waits for them" )
    {
        CORE::PulseGeneratorPtr mainPulse = CORE::CPulseGenerator::CreateSharedObj();
        CORE::CJobSchedulerSettings settings = MakeJsSettings( "js-t15-main" );
        CORE::JobSchedulerPtr scheduler = MakeJsScheduler( settings, GUCEF_NULL, mainPulse );
        CORE::CString error;
        CORE::CJobDefinition mainJob = MakeJsJob( "on-main", CORE::CJobTrigger::Manual(), "main", 1, 4000 );
        mainJob.SetExecutionTarget( CORE::JOBEXECUTIONTARGET_MAIN_THREAD );
        ASSERT_TRUE( scheduler->AddJob( mainJob, error ) );
        ASSERT_TRUE( scheduler->AddJob( MakeJsJob( "on-pool", CORE::CJobTrigger::Manual(), "pool" ), error ) );
        ClearJsRunLog();
        ASSERT_TRUE( scheduler->Start( error ) );

        // posted, not carried out by anybody while the main pulse generator does not pulse
        CORE::SJobControlResult mainResult, poolResult;
        ASSERT_TRUE( CORE::JOBCONTROLSTATUS_OK == RunNow( *scheduler, "on-main", mainResult ) );
        ASSERT_TRUE( WaitForRunState( *scheduler, mainResult.runId, CORE::JOBRUNSTATE_RUNNING, 5000, false ) );
        CORE::ThreadPoolPtr mainPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool( settings.GetMainThreadPoolName() );
        bool queued = false;
        for ( UInt32 i=0; i<2500 && !queued; ++i )
        {
            queued = !mainPool.IsNULL() && 1 == mainPool->GetNrOfQueuedTasks();
            if ( !queued )
                MT::ThreadDelay( 2 );
        }
        ASSERT_TRUE( queued );
        ASSERT_TRUE( !mainPool.IsNULL() && 0 == mainPool->GetActiveNrOfThreads() );
        ASSERT_TRUE( 0 == CountJsRuns( "main" ) );

        // the main thread of this test: a thread that pumps the main pulse generator; the run blocks it for 4 s
        CJsPulseThread mainThread( mainPulse );
        mainThread.Activate();
        bool mainStarted = false;
        for ( UInt32 i=0; i<5000 && !mainStarted; ++i )
        {
            mainStarted = 1 == CountJsRuns( "main" );
            if ( !mainStarted )
                MT::ThreadDelay( 1 );
        }
        ASSERT_TRUE( mainStarted );

        // while the main thread is blocked by its run, a pool run is dispatched and ends: the dispatcher does not wait
        ASSERT_TRUE( CORE::JOBCONTROLSTATUS_OK == RunNow( *scheduler, "on-pool", poolResult ) );
        ASSERT_TRUE( WaitForRunState( *scheduler, poolResult.runId, CORE::JOBRUNSTATE_SUCCEEDED, 10000, false ) );
        TJsRunLog log;
        GetJsRunLog( log );
        bool mainStillRunning = false;
        for ( size_t i=0; i<log.size(); ++i )
            if ( log[ i ].value == "main" )
                mainStillRunning = 0 == log[ i ].endedRealMs;
        ASSERT_TRUE( mainStillRunning );

        ASSERT_TRUE( WaitForRunState( *scheduler, mainResult.runId, CORE::JOBRUNSTATE_SUCCEEDED, 15000, false ) );
        mainThread.Deactivate( false, true );
        GetJsRunLog( log );
        for ( size_t i=0; i<log.size(); ++i )
        {
            if ( log[ i ].value == "main" )
                ASSERT_TRUE( log[ i ].systemThreadId == mainThread.GetSystemThreadId() );
            if ( log[ i ].value == "pool" )
                ASSERT_TRUE( log[ i ].systemThreadId != mainThread.GetSystemThreadId() );
        }
        CORE::SJobRunRecord mainRun;
        scheduler->GetRun( mainResult.runId, mainRun );
        ASSERT_TRUE( mainRun.threadPoolName == settings.GetMainThreadPoolName() );
        ASSERT_TRUE( CORE::JOBEXECUTIONTARGET_MAIN_THREAD == mainRun.executionTarget );
        DropJsScheduler( scheduler );
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

static void
PerformSubmitTests( void )
{
    GUCEF_TESTFW_TESTCASE( "JS-T16: a job is a task type and a payload: the task data factory gets the deserialized payload" )
    {
        CManualClocks clocks( JsUtc( 17 ) );
        CORE::JobSchedulerPtr scheduler = MakeJsScheduler( MakeJsSettings( "js-t16-payload" ), &clocks );
        CORE::CString error;
        ASSERT_TRUE( scheduler->AddJob( MakeJsJob( "payload", CORE::CJobTrigger::Manual(), "hello payload" ), error ) );
        CORE::CJobDefinition unknownType = MakeJsJob( "unknown", CORE::CJobTrigger::Manual(), "x" );
        unknownType.SetTaskTypeName( "NoSuchTaskType" );
        ASSERT_FALSE( scheduler->AddJob( unknownType, error ) );
        CORE::CJobDefinition badId = MakeJsJob( "Bad Id", CORE::CJobTrigger::Manual(), "x" );
        ASSERT_FALSE( scheduler->AddJob( badId, error ) );
        ClearJsRunLog();
        TAccess::StartManual( *scheduler, error );
        TAccess::Cycle( *scheduler );
        CORE::SJobControlResult result;
        RunNow( *scheduler, "payload", result );
        ASSERT_TRUE( WaitForRunState( *scheduler, result.runId, CORE::JOBRUNSTATE_SUCCEEDED, 5000, true ) );
        TJsRunLog log;
        GetJsRunLog( log );
        ASSERT_TRUE( 1 == log.size() && log[ 0 ].hadTaskData && log[ 0 ].value == "hello payload" );
        CORE::SJobRunRecord run;
        scheduler->GetRun( result.runId, run );
        ASSERT_TRUE( run.taskTypeName == CJsTestTaskData::TaskType );
        ASSERT_TRUE( run.requestedBy == "tester" );
        ASSERT_TRUE( 0 != run.taskId );
        DropJsScheduler( scheduler );
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JS-T16: SubmitJob checks the allowlist, the payload bound, the validator, the job count and never replaces a job defined in code" )
    {
        CManualClocks clocks( JsUtc( 18 ) );
        CMemoryJobStateStore store;
        CJsPayloadValidator validator;
        CORE::CJobSchedulerSettings settings = MakeJsSettings( "js-t16-submit-a" );
        settings.SetMaxSubmitPayloadBytes( 256 );
        settings.SetMaxSubmittedJobs( 2 );
        CORE::JobSchedulerPtr scheduler = MakeJsScheduler( settings, &clocks );
        scheduler->SetStateStore( &store );
        CORE::CString error;
        ASSERT_TRUE( scheduler->AddJob( MakeJsJob( "code-job", CORE::CJobTrigger::Manual(), "code" ), error ) );
        TAccess::StartManual( *scheduler, error );
        TAccess::Cycle( *scheduler );

        CORE::SJobSubmission submission;
        submission.definition = MakeJsJob( "submitted-1", CORE::CJobTrigger::Manual(), "data" );
        submission.submittedBy = "operator";
        CORE::SJobControlResult result;
        ASSERT_TRUE( CORE::JOBCONTROLSTATUS_FAILED_PRECONDITION == scheduler->SubmitJob( submission, result ) );

        ASSERT_TRUE( scheduler->RegisterSubmittableTaskType( CJsTestTaskData::TaskType, &validator ) );
        ASSERT_FALSE( scheduler->RegisterSubmittableTaskType( "NoSuchTaskType" ) );

        CORE::SJobSubmission big = submission;
        CORE::CString longText;
        for ( UInt32 i=0; i<64; ++i )
            longText += "0123456789";
        big.definition = MakeJsJob( "submitted-big", CORE::CJobTrigger::Manual(), longText );
        ASSERT_TRUE( CORE::JOBCONTROLSTATUS_RESOURCE_EXHAUSTED == scheduler->SubmitJob( big, result ) );

        CORE::SJobSubmission evil = submission;
        evil.definition = MakeJsJob( "submitted-evil", CORE::CJobTrigger::Manual(), "evil" );
        ASSERT_TRUE( CORE::JOBCONTROLSTATUS_INVALID_ARGUMENT == scheduler->SubmitJob( evil, result ) );

        CORE::SJobSubmission notTaskData = submission;
        notTaskData.definition = MakeJsJob( "submitted-garbage", CORE::CJobTrigger::Manual(), "x" );
        CORE::CDataNode garbage;
        garbage.SetAttribute( "other", "thing" );
        notTaskData.definition.SetPayload( garbage );
        ASSERT_TRUE( CORE::JOBCONTROLSTATUS_INVALID_ARGUMENT == scheduler->SubmitJob( notTaskData, result ) );

        CORE::SJobSubmission replaceCode = submission;
        replaceCode.definition = MakeJsJob( "code-job", CORE::CJobTrigger::Manual(), "taken over" );
        replaceCode.replaceExisting = true;
        ASSERT_TRUE( CORE::JOBCONTROLSTATUS_FAILED_PRECONDITION == scheduler->SubmitJob( replaceCode, result ) );
        CORE::CJobDefinition codeJob;
        ASSERT_TRUE( scheduler->GetJobDefinition( "code-job", codeJob ) && CORE::JOBORIGIN_CODE == codeJob.GetOrigin() );

        CORE::SJobSubmission dry = submission;
        dry.dryRun = true;
        ASSERT_TRUE( CORE::JOBCONTROLSTATUS_OK == scheduler->SubmitJob( dry, result ) );
        ASSERT_FALSE( scheduler->GetJobDefinition( "submitted-1", codeJob ) );

        ASSERT_TRUE( CORE::JOBCONTROLSTATUS_OK == scheduler->SubmitJob( submission, result ) );
        CORE::SJobSubmission second = submission;
        second.definition = MakeJsJob( "submitted-2", CORE::CJobTrigger::Interval( 60000 ), "data2" );
        ASSERT_TRUE( CORE::JOBCONTROLSTATUS_OK == scheduler->SubmitJob( second, result ) );
        CORE::SJobSubmission third = submission;
        third.definition = MakeJsJob( "submitted-3", CORE::CJobTrigger::Manual(), "data3" );
        ASSERT_TRUE( CORE::JOBCONTROLSTATUS_RESOURCE_EXHAUSTED == scheduler->SubmitJob( third, result ) );
        ASSERT_TRUE( CORE::JOBCONTROLSTATUS_FAILED_PRECONDITION == scheduler->SubmitJob( submission, result ) );

        CORE::SJobStatus status = GetJobStatus( *scheduler, "submitted-1" );
        ASSERT_TRUE( CORE::JOBORIGIN_SUBMITTED == status.origin && status.submittedBy == "operator" );
        ASSERT_TRUE( CORE::JOBCONTROLSTATUS_FAILED_PRECONDITION == SimpleControl( *scheduler, CORE::JOBCONTROLACTION_REMOVE_JOB, CORE::JOBCONTROLTARGET_JOB, "code-job" ) );
        ASSERT_TRUE( CORE::JOBCONTROLSTATUS_OK == SimpleControl( *scheduler, CORE::JOBCONTROLACTION_REMOVE_JOB, CORE::JOBCONTROLTARGET_JOB, "submitted-2" ) );
        ASSERT_TRUE( CORE::JOBCONTROLSTATUS_OK == scheduler->SubmitJob( third, result ) );
        DropJsScheduler( scheduler );

        // a restart on the same state: the submitted jobs come back for the submittable task type
        CORE::CJobSchedulerSettings settingsB = settings;
        settingsB.SetHostName( "js-t16-submit-b" );
        scheduler = MakeJsScheduler( settingsB, &clocks );
        scheduler->SetStateStore( &store );
        ASSERT_TRUE( scheduler->RegisterSubmittableTaskType( CJsTestTaskData::TaskType, &validator ) );
        TAccess::StartManual( *scheduler, error );
        TAccess::Cycle( *scheduler );
        CORE::CJobDefinition restored;
        ASSERT_TRUE( scheduler->GetJobDefinition( "submitted-1", restored ) );
        ASSERT_TRUE( CORE::JOBORIGIN_SUBMITTED == restored.GetOrigin() && restored.GetSubmittedBy() == "operator" );
        ASSERT_TRUE( scheduler->GetJobDefinition( "submitted-3", restored ) );
        ASSERT_FALSE( scheduler->GetJobDefinition( "submitted-2", restored ) );
        ClearJsRunLog();
        ASSERT_TRUE( CORE::JOBCONTROLSTATUS_OK == RunNow( *scheduler, "submitted-1", result ) );
        ASSERT_TRUE( WaitForRunState( *scheduler, result.runId, CORE::JOBRUNSTATE_SUCCEEDED, 5000, true ) );
        ASSERT_TRUE( 1 == CountJsRuns( "data" ) );
        DropJsScheduler( scheduler );
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JS-T16: RunNowAsync completes its future with the final run record" )
    {
        CORE::CJobSchedulerSettings settings = MakeJsSettings( "js-t16-async" );
        CORE::JobSchedulerPtr scheduler = MakeJsScheduler( settings, GUCEF_NULL );
        CORE::CString error;
        scheduler->AddJob( MakeJsJob( "async", CORE::CJobTrigger::Manual(), "async" ), error );
        ASSERT_TRUE( scheduler->Start( error ) );
        CORE::CFutureResult future = scheduler->RunNowAsync( "async", "waiter" );
        ASSERT_TRUE( future.HasAFuture() );
        CORE::CTaskPtr resultTask = future.GetResult( 10000 );
        ASSERT_TRUE( !resultTask.IsNULL() );
        if ( !resultTask.IsNULL() )
        {
            const CORE::TCloneableJobRunRecord* record = static_cast< const CORE::TCloneableJobRunRecord* >( resultTask->GetTaskData() );
            ASSERT_TRUE( GUCEF_NULL != record );
            if ( GUCEF_NULL != record )
            {
                ASSERT_TRUE( CORE::JOBRUNSTATE_SUCCEEDED == record->GetData().state );
                ASSERT_TRUE( record->GetData().requestedBy == "waiter" );
            }
        }
        CORE::CFutureResult refused = scheduler->RunNowAsync( "no-such-job", "waiter" );
        ASSERT_TRUE( refused.HasNoFuture() );
        DropJsScheduler( scheduler );
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

static void
PerformBoundedMemoryTests( void )
{
    GUCEF_TESTFW_TESTCASE( "JS-T13: 1 000 000 ended runs leave the run history at its bounds" )
    {
        CORE::CJobRunHistory history( 1000 );
        CORE::SJobRunRecord run;
        run.jobId = "job";
        for ( UInt32 i=1; i<=1000000; ++i )
        {
            run.runId = i;
            run.revision = i;
            history.Add( run );
        }
        ASSERT_TRUE( 1000 == history.GetCount() );
        CORE::TJobRunRecordVector newest;
        history.GetNewestFirst( newest, 0, 3 );
        ASSERT_TRUE( 3 == newest.size() && 1000000 == newest[ 0 ].runId && 999998 == newest[ 2 ].runId );
        CORE::SJobRunRecord found;
        ASSERT_TRUE( history.Find( 999001, found ) );
        ASSERT_FALSE( history.Find( 999000, found ) );
        history.SetMaximum( 10 );
        newest.clear();
        history.GetNewestFirst( newest );
        ASSERT_TRUE( 10 == newest.size() && 1000000 == newest[ 0 ].runId && 999991 == newest[ 9 ].runId );
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JS-T13: 3000 runs through the engine keep the history and the per job history at their bounds" )
    {
        CManualClocks clocks( JsUtc( 19 ) );
        CORE::CJobSchedulerSettings settings = MakeJsSettings( "js-t13-engine" );
        settings.SetMaxRunHistory( 100 );
        settings.SetPerJobHistory( 5 );
        settings.SetDefaultLaneMaxConcurrent( 64 );
        CORE::JobSchedulerPtr scheduler = MakeJsScheduler( settings, &clocks );
        CORE::ThreadPoolPtr runPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( settings.GetRunPoolName() );
        runPool->SetDesiredMinNrOfWorkerThreads( 4 );
        CORE::CString error;
        CORE::CJobDefinition job = MakeJsJob( "repeat", CORE::CJobTrigger::Manual(), "bounded" );
        job.SetConcurrency( CORE::JOBCONCURRENCY_PARALLEL );
        job.SetMaxParallelRuns( 64 );
        scheduler->AddJob( job, error );
        TAccess::StartManual( *scheduler, error );
        TAccess::Cycle( *scheduler );
        CORE::SJobControlResult result;
        for ( UInt32 round=0; round<30; ++round )
        {
            for ( UInt32 i=0; i<100; ++i )
                RunNow( *scheduler, "repeat", result );
            CycleUntilIdle( *scheduler, 60000 );
        }
        CORE::CJobSchedulerSnapshot snapshot;
        scheduler->GetSnapshot( snapshot );
        ASSERT_TRUE( 3000 == snapshot.GetTotalRunsSucceeded() );
        ASSERT_TRUE( 100 == TAccess::GetHistoryCount( *scheduler ) );
        ASSERT_TRUE( 5 == TAccess::GetJobHistoryCount( *scheduler, "repeat" ) );
        ASSERT_TRUE( snapshot.GetRuns().size() <= 100 );
        // the events wait for a pulse that never comes in this test: coalesced, they stay bounded
        ASSERT_TRUE( TAccess::GetPendingChangeCount( *scheduler ) <= 100 + 64 + 8 );
        runPool->SetDesiredMinNrOfWorkerThreads( 0 );
        DropJsScheduler( scheduler );
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

static void
PerformValueTypeTests( void )
{
    GUCEF_TESTFW_TESTCASE( "Job definitions, triggers and settings survive a round trip through a data node" )
    {
        CORE::CJobDefinition job = MakeJsJob( "round.trip-1", Cron( "15 3 * * 1" ), "trip" );
        job.SetTitle( "Round trip" );
        job.SetLaneName( "api" );
        job.SetExecutionTarget( CORE::JOBEXECUTIONTARGET_MAIN_THREAD );
        job.SetPriority( 2 );
        job.SetConcurrency( CORE::JOBCONCURRENCY_PARALLEL );
        job.SetMaxParallelRuns( 4 );
        job.SetMissedRunPolicy( CORE::JOBMISSEDRUNPOLICY_RUN_EACH );
        job.SetTimeoutMs( 1234 );
        job.SetFlags( CORE::JOBFLAG_SENSITIVE );
        CORE::TJobControlActionSet actions;
        actions.insert( CORE::JOBCONTROLACTION_RUN_NOW );
        job.SetRemoteActions( actions );
        CORE::CDataNode node;
        CORE::CDataNodeSerializableSettings serializerSettings;
        ASSERT_TRUE( job.Serialize( node, serializerSettings ) );
        CORE::CJobDefinition back;
        ASSERT_TRUE( back.Deserialize( node, serializerSettings ) );
        ASSERT_TRUE( back.GetId() == job.GetId() && back.GetTitle() == "Round trip" );
        ASSERT_TRUE( back.GetTrigger() == job.GetTrigger() );
        ASSERT_TRUE( CORE::JOBEXECUTIONTARGET_MAIN_THREAD == back.GetExecutionTarget() );
        ASSERT_TRUE( 2 == back.GetPriority() && 4 == back.GetMaxParallelRuns() && 1234 == back.GetTimeoutMs() );
        ASSERT_TRUE( CORE::JOBMISSEDRUNPOLICY_RUN_EACH == back.GetMissedRunPolicy() );
        ASSERT_TRUE( 1 == back.GetRemoteActions().size() );
        ASSERT_TRUE( back.GetPayload().GetAttributeValueOrChildValueByName( "value" ).AsString( CORE::CString::Empty, true ) == "trip" );

        CORE::CJobTrigger interval = CORE::CJobTrigger::Interval( 5000, CORE::JOBINTERVALFROM_END );
        interval.SetHasAlignment( true );
        interval.SetAlignToUtcMs( 777 );
        CORE::CDataNode triggerNode;
        ASSERT_TRUE( interval.Serialize( triggerNode, serializerSettings ) );
        CORE::CJobTrigger intervalBack;
        ASSERT_TRUE( intervalBack.Deserialize( triggerNode, serializerSettings ) && intervalBack == interval );
        ASSERT_TRUE( interval.GetDescription() == "every 5000 ms after the end of the run, aligned to 1970-01-01T00:00:00.777Z" || !interval.GetDescription().IsNULLOrEmpty() );
        ASSERT_TRUE( Cron( "0 2 * * *" ).GetDescription() == "cron 0 2 * * * UTC" );

        CORE::CJobSchedulerSettings settings = MakeJsSettings( "round-trip" );
        settings.SetMaxSleepMs( 4242 );
        CORE::CDataNode settingsNode;
        ASSERT_TRUE( settings.SaveConfig( settingsNode ) );
        CORE::CJobSchedulerSettings settingsBack;
        ASSERT_TRUE( settingsBack.LoadConfig( settingsNode ) );
        ASSERT_TRUE( settingsBack.GetHostName() == "round-trip" && 4242 == settingsBack.GetMaxSleepMs() && settingsBack.GetRunPoolName() == settings.GetRunPoolName() );

        CORE::CString error;
        CORE::CJobDefinition invalid;
        ASSERT_FALSE( invalid.IsValid( error ) );
        invalid.SetId( "ok" );
        invalid.SetTaskTypeName( "T" );
        invalid.SetTrigger( CORE::CJobTrigger::Interval( 0 ) );
        ASSERT_FALSE( invalid.IsValid( error ) );
        invalid.SetTrigger( CORE::CJobTrigger::Manual() );
        ASSERT_TRUE( invalid.IsValid( error ) );
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

void
PerformJobSchedulerTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "JobScheduler" );

    RegisterJsTestTaskType();

    PerformValueTypeTests();
    PerformTriggerTests();
    PerformMissedRunTests();
    PerformClockTests();
    PerformConcurrencyTests();
    PerformControlTests();
    PerformRetryTests();
    PerformWorkSourceTests();
    PerformStateStoreTests();
    PerformIdleTests();
    PerformThreadTests();
    PerformStopTests();
    PerformMainThreadTests();
    PerformSubmitTests();
    PerformBoundedMemoryTests();

    UnregisterJsTestTaskType();
}

/*-------------------------------------------------------------------------*/
