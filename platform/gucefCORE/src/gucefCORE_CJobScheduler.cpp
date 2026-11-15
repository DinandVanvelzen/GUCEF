/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CIJOBWORKSOURCE_H
#include "gucefCORE_CIJobWorkSource.h"
#define GUCEF_CORE_CIJOBWORKSOURCE_H
#endif /* GUCEF_CORE_CIJOBWORKSOURCE_H ? */

#ifndef GUCEF_CORE_CIJOBWORKITEMSINK_H
#include "gucefCORE_CIJobWorkItemSink.h"
#define GUCEF_CORE_CIJOBWORKITEMSINK_H
#endif /* GUCEF_CORE_CIJOBWORKITEMSINK_H ? */

#ifndef GUCEF_CORE_CIJOBTHROTTLEPOLICY_H
#include "gucefCORE_CIJobThrottlePolicy.h"
#define GUCEF_CORE_CIJOBTHROTTLEPOLICY_H
#endif /* GUCEF_CORE_CIJOBTHROTTLEPOLICY_H ? */

#ifndef GUCEF_CORE_CIJOBSCHEDULERSTATESTORE_H
#include "gucefCORE_CIJobSchedulerStateStore.h"
#define GUCEF_CORE_CIJOBSCHEDULERSTATESTORE_H
#endif /* GUCEF_CORE_CIJOBSCHEDULERSTATESTORE_H ? */

#ifndef GUCEF_CORE_CIJOBPAYLOADVALIDATOR_H
#include "gucefCORE_CIJobPayloadValidator.h"
#define GUCEF_CORE_CIJOBPAYLOADVALIDATOR_H
#endif /* GUCEF_CORE_CIJOBPAYLOADVALIDATOR_H ? */

#ifndef GUCEF_CORE_CJOBSCHEDULERTASK_H
#include "gucefCORE_CJobSchedulerTask.h"
#define GUCEF_CORE_CJOBSCHEDULERTASK_H
#endif /* GUCEF_CORE_CJOBSCHEDULERTASK_H ? */

#ifndef GUCEF_CORE_CDATANODEBINARYSERIALIZER_H
#include "gucefCORE_CDataNodeBinarySerializer.h"
#define GUCEF_CORE_CDATANODEBINARYSERIALIZER_H
#endif /* GUCEF_CORE_CDATANODEBINARYSERIALIZER_H ? */

#ifndef GUCEF_CORE_CDATANODESERIALIZABLESETTINGS_H
#include "gucefCORE_CDataNodeSerializableSettings.h"
#define GUCEF_CORE_CDATANODESERIALIZABLESETTINGS_H
#endif /* GUCEF_CORE_CDATANODESERIALIZABLESETTINGS_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_CORE_CREDACTOR_H
#include "gucefCORE_CRedactor.h"
#define GUCEF_CORE_CREDACTOR_H
#endif /* GUCEF_CORE_CREDACTOR_H ? */

#include "gucefCORE_CJobScheduler.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CJobScheduler::ClassTypeName = "GUCEF::CORE::CJobScheduler";

const CEvent CJobScheduler::JobChangedEvent = "GUCEF::CORE::CJobScheduler::JobChangedEvent";
const CEvent CJobScheduler::LaneChangedEvent = "GUCEF::CORE::CJobScheduler::LaneChangedEvent";
const CEvent CJobScheduler::RunChangedEvent = "GUCEF::CORE::CJobScheduler::RunChangedEvent";
const CEvent CJobScheduler::HostStateChangedEvent = "GUCEF::CORE::CJobScheduler::HostStateChangedEvent";

const CString CJobScheduler::DefaultLaneName = "default";

/** A save that failed is tried again after the larger of this and the debounce */
static const UInt64 FailedStateSaveRetryMs = 5000;

/** The most occurrences of one schedule looked at one by one in one evaluation; beyond it only the latest counts */
static const UInt32 MaxOccurrencesPerEvaluation = 64;

/** Version of the kept state */
static const UInt32 StateFormatVersion = 1;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static CString
TruncateText( const CString& text, UInt32 maxBytes )
{GUCEF_TRACE;

    if ( text.Length() <= maxBytes )
        return text;
    return text.SubstrToIndex( maxBytes, true );
}

/*-------------------------------------------------------------------------*/

static TJobRunState
RunStateFromTaskStatus( TTaskStatus status, bool timeoutRequested )
{GUCEF_TRACE;

    switch ( status )
    {
        case TASKSTATUS_FINISHED:   return JOBRUNSTATE_SUCCEEDED;
        case TASKSTATUS_CANCELLED:
        case TASKSTATUS_STOPPED:    return timeoutRequested ? JOBRUNSTATE_TIMED_OUT : JOBRUNSTATE_CANCELLED;
        default:                    return JOBRUNSTATE_FAILED;
    }
}

/*-------------------------------------------------------------------------*/

static Int64
ToUtcMs( const CDateTime& dt )
{GUCEF_TRACE;

    return (Int64) dt.ToUnixEpochBasedTicksInMillisecs();
}

/*-------------------------------------------------------------------------*/

static CDateTime
FromUtcMs( Int64 utcMs )
{GUCEF_TRACE;

    CDateTime dt;
    dt.FromUnixEpochBasedTicksInMillisecs( (UInt64) ( utcMs < 0 ? 0 : utcMs ) );
    return dt;
}

/*-------------------------------------------------------------------------*/

/** The next occurrence of the schedule strictly after 'afterUtcMs'; 0 = none */
static Int64
NextOccurrenceAfter( const CSchedule& schedule, Int64 afterUtcMs )
{GUCEF_TRACE;

    CDateTime next;
    if ( !schedule.GetNextOccurrence( FromUtcMs( afterUtcMs ), next ) )
        return 0;
    Int64 nextUtcMs = ToUtcMs( next );
    if ( nextUtcMs <= afterUtcMs )
    {
        // a schedule that answers 'from' itself: look from just after it
        if ( !schedule.GetNextOccurrence( FromUtcMs( afterUtcMs + 1000 ), next ) )
            return 0;
        nextUtcMs = ToUtcMs( next );
        if ( nextUtcMs <= afterUtcMs )
            return 0;
    }
    return nextUtcMs;
}

/*-------------------------------------------------------------------------*/

static bool
ComputePayloadBytes( const CDataNode& payload, UInt32& bytes )
{GUCEF_TRACE;

    CDataNodeBinarySerializerOptions options;
    CDynamicBuffer buffer;
    bytes = 0;
    return CDataNodeBinarySerializer::Serialize( options, payload, 0, buffer, bytes );
}

/*-------------------------------------------------------------------------*/

static bool
PayloadIsEmpty( const CDataNode& payload )
{GUCEF_TRACE;

    return !payload.HasChildren() && payload.GetValue().IsNULLOrEmpty() && payload.AttributeBegin() == payload.AttributeEnd();
}

/*-------------------------------------------------------------------------*/

static void
SerializeRunRecord( const SJobRunRecord& run, CDataNode& node )
{GUCEF_TRACE;

    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    node.SetAttribute( "runId", run.runId );
    node.SetAttribute( "hostInstanceId", run.hostInstanceId );
    node.SetAttribute( "jobId", run.jobId );
    node.SetAttribute( "itemLabel", run.itemLabel );
    node.SetAttribute( "taskTypeName", run.taskTypeName );
    node.SetAttribute( "executionTarget", JobExecutionTargetToString( run.executionTarget ) );
    node.SetAttribute( "threadPoolName", run.threadPoolName );
    node.SetAttribute( "cause", (Int32) run.cause );
    node.SetAttribute( "scheduledUtcMs", run.scheduledUtcMs );
    node.SetAttribute( "queuedUtcMs", run.queuedUtcMs );
    node.SetAttribute( "startedUtcMs", run.startedUtcMs );
    node.SetAttribute( "endedUtcMs", run.endedUtcMs );
    node.SetAttribute( "attempt", run.attempt );
    node.SetAttribute( "state", (Int32) run.state );
    node.SetAttribute( "note", run.note );
    node.SetAttribute( "outcomeMessage", run.outcomeMessage );
    node.SetAttribute( "requestedBy", run.requestedBy );
    if ( 0 != run.progressDone || 0 != run.progressTotal )
    {
        node.SetAttribute( "progressDone", run.progressDone );
        node.SetAttribute( "progressTotal", run.progressTotal );
    }
    if ( !run.resultDocument.IsNULLOrEmpty() )
        node.SetAttribute( "resultDocument", run.resultDocument );
}

/*-------------------------------------------------------------------------*/

static void
DeserializeRunRecord( const CDataNode& node, SJobRunRecord& run )
{GUCEF_TRACE;

    run.Clear();
    run.runId = node.GetAttributeValueOrChildValueByName( "runId" ).AsUInt64( 0, true );
    run.hostInstanceId = node.GetAttributeValueOrChildValueByName( "hostInstanceId" ).AsUInt64( 0, true );
    run.jobId = node.GetAttributeValueOrChildValueByName( "jobId" ).AsString( CString::Empty, true );
    run.itemLabel = node.GetAttributeValueOrChildValueByName( "itemLabel" ).AsString( CString::Empty, true );
    run.taskTypeName = node.GetAttributeValueOrChildValueByName( "taskTypeName" ).AsString( CString::Empty, true );
    run.executionTarget = node.GetAttributeValueOrChildValueByName( "executionTarget" ).AsString( CString::Empty, true ).Equals( "MAIN_THREAD", false ) ? JOBEXECUTIONTARGET_MAIN_THREAD : JOBEXECUTIONTARGET_POOL;
    run.threadPoolName = node.GetAttributeValueOrChildValueByName( "threadPoolName" ).AsString( CString::Empty, true );
    run.cause = (TJobRunCause) node.GetAttributeValueOrChildValueByName( "cause" ).AsInt32( 0, true );
    run.scheduledUtcMs = node.GetAttributeValueOrChildValueByName( "scheduledUtcMs" ).AsInt64( 0, true );
    run.queuedUtcMs = node.GetAttributeValueOrChildValueByName( "queuedUtcMs" ).AsInt64( 0, true );
    run.startedUtcMs = node.GetAttributeValueOrChildValueByName( "startedUtcMs" ).AsInt64( 0, true );
    run.endedUtcMs = node.GetAttributeValueOrChildValueByName( "endedUtcMs" ).AsInt64( 0, true );
    run.attempt = node.GetAttributeValueOrChildValueByName( "attempt" ).AsUInt32( 1, true );
    run.state = (TJobRunState) node.GetAttributeValueOrChildValueByName( "state" ).AsInt32( 0, true );
    run.note = node.GetAttributeValueOrChildValueByName( "note" ).AsString( CString::Empty, true );
    run.outcomeMessage = node.GetAttributeValueOrChildValueByName( "outcomeMessage" ).AsString( CString::Empty, true );
    run.requestedBy = node.GetAttributeValueOrChildValueByName( "requestedBy" ).AsString( CString::Empty, true );
    run.progressDone = node.GetAttributeValueOrChildValueByName( "progressDone" ).AsUInt64( 0, true );
    run.progressTotal = node.GetAttributeValueOrChildValueByName( "progressTotal" ).AsUInt64( 0, true );
    run.resultDocument = node.GetAttributeValueOrChildValueByName( "resultDocument" ).AsString( CString::Empty, true );
}

/*-------------------------------------------------------------------------*/

/** Order of dispatch: priority, due time, enqueue order */
struct SPendingRunOrder
{
    UInt32 priority;
    Int64 scheduledUtcMs;
    UInt64 sequence;
    UInt64 runId;

    bool operator<( const SPendingRunOrder& other ) const
    {
        if ( priority != other.priority )
            return priority < other.priority;
        if ( scheduledUtcMs != other.scheduledUtcMs )
            return scheduledUtcMs < other.scheduledUtcMs;
        return sequence < other.sequence;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What a work source puts its due items in, during CIJobWorkSource::Evaluate() (outside the scheduler lock) */
class CJobScheduler::CWorkItemSink : public CIJobWorkItemSink
{
    public:

    CWorkItemSink( CJobScheduler& scheduler, const CString& jobId, Int64 nowUtcMs, SDeferred& deferred )
        : CIJobWorkItemSink()
        , m_scheduler( scheduler )
        , m_jobId( jobId )
        , m_nowUtcMs( nowUtcMs )
        , m_deferred( deferred )
    {GUCEF_TRACE;
    }

    virtual ~CWorkItemSink() GUCEF_VIRTUAL_OVERRIDE {}

    virtual bool Dispatch( const SJobWorkItem& item ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        MT::CScopeMutex lock( m_scheduler.m_lock );

        TJobMap::iterator j = m_scheduler.m_jobs.find( m_jobId );
        if ( j == m_scheduler.m_jobs.end() )
            return false;
        SJobEntry& job = (*j).second;
        if ( !job.enabled || JOBSCHEDULERSTATE_DRAINING == m_scheduler.m_hostState || JOBSCHEDULERSTATE_STOPPED == m_scheduler.m_hostState )
            return false;

        const CString& laneName = item.laneName.IsNULLOrEmpty() ? m_scheduler.EffectiveLaneName( job.definition ) : item.laneName;
        if ( GUCEF_NULL == m_scheduler.FindLane( laneName ) )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "JobScheduler(" << m_scheduler.GetName() << "): the work source of job " << m_jobId << " gave an item for the unknown lane " << laneName );
            return false;
        }

        // the key of an item is unique among the source's pending items
        for ( TRunMap::const_iterator r = m_scheduler.m_activeRuns.begin(); r != m_scheduler.m_activeRuns.end(); ++r )
        {
            const SRunEntry& run = (*r).second;
            if ( run.hasWorkItem && run.record.jobId == m_jobId && run.workItem.key == item.key )
                return false;
        }

        m_scheduler.QueueRun( job, JOBRUNCAUSE_WORK_ITEM, m_nowUtcMs, item.priority, CString::Empty, &item, m_deferred, false );
        return true;
    }

    private:

    CJobScheduler& m_scheduler;
    CString m_jobId;
    Int64 m_nowUtcMs;
    SDeferred& m_deferred;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CJobScheduler::SJobEntry::SJobEntry( void )
    : definition()
    , workSource( GUCEF_NULL )
    , enabled( true )
    , priorityOverridden( false )
    , available( true )
    , unavailableReason()
    , ended( false )
    , planned( false )
    , lastOccurrenceUtcMs( 0 )
    , nextDueUtcMs( 0 )
    , nextDueMonotonicMs( 0 )
    , sourceNextWakeUtcMs( 0 )
    , pendingRunCount( 0 )
    , runningRunCount( 0 )
    , runCount( 0 )
    , failureCount( 0 )
    , consecutiveFailures( 0 )
    , lastRunId( 0 )
    , lastRunState( JOBRUNSTATE_UNSPECIFIED )
    , lastRunEndedUtcMs( 0 )
    , waitingFor()
    , waitingUntilUtcMs( 0 )
    , revision( 0 )
    , recentRuns()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CJobScheduler::SLaneEntry::SLaneEntry( void )
    : definition()
    , mode()
    , modeOverridden( false )
    , available( true )
    , unavailableReason()
    , inFlight( 0 )
    , heldBy()
    , heldSinceUtcMs( 0 )
    , waitingFor()
    , waitingUntilUtcMs( 0 )
    , revision( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CJobScheduler::SRunEntry::SRunEntry( void )
    : record()
    , priority( 1 )
    , sequence( 0 )
    , notBeforeMonotonicMs( 0 )
    , startedMonotonicMs( 0 )
    , timeoutMs( 0 )
    , taskQueued( false )
    , cancelRequested( false )
    , cancelSent( false )
    , timeoutRequested( false )
    , laneName()
    , estimatedCost( 1 )
    , hasWorkItem( false )
    , workItem()
    , future( GUCEF_NULL )
    , lastProgressEventMonotonicMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CJobScheduler::SDeferred::SDeferred( void )
    : cancellations()
    , itemsFinished()
    , futures()
    , wakeMainThread( false )
    , wakeDispatcher( false )
    , postChanges( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::RegisterEvents( void )
{GUCEF_TRACE;

    JobChangedEvent.Initialize();
    LaneChangedEvent.Initialize();
    RunChangedEvent.Initialize();
    HostStateChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CJobScheduler::CJobScheduler( CTaskManager& taskManager                     ,
                              const PulseGeneratorPtr& mainPulseGenerator   ,
                              const CJobSchedulerSettings& settings         )
    : CTSGNotifier( mainPulseGenerator, true, false )
    , CTSharedPtrCreator< CJobScheduler, MT::CMutex >( this )
    , m_taskManager( &taskManager )
    , m_mainPulseGenerator( mainPulseGenerator )
    , m_settings( settings )
    , m_systemUtcClock()
    , m_utcClock( &m_systemUtcClock )
    , m_monotonicClock( &CSystemMonotonicClock::Instance() )
    , m_stateStore( GUCEF_NULL )
    , m_hostInstanceId( 0 )
    , m_revision( 0 )
    , m_hostState( JOBSCHEDULERSTATE_CREATED )
    , m_lock()
    , m_runLink()
    , m_jobs()
    , m_lanes()
    , m_activeRuns()
    , m_endedTasks()
    , m_history( settings.GetMaxRunHistory() )
    , m_submittable()
    , m_loadedState()
    , m_stateLoaded( false )
    , m_stateDirty( false )
    , m_stateSaveDueMonotonicMs( 0 )
    , m_nextRunId( 1 )
    , m_nextSequence( 1 )
    , m_startUtcMs( 0 )
    , m_startMonotonicMs( 0 )
    , m_lastCycleUtcMs( 0 )
    , m_lastCycleMonotonicMs( 0 )
    , m_stopRequested( false )
    , m_dispatcherActive( false )
    , m_manualDispatch( false )
    , m_dispatcherTaskId( 0 )
    , m_dispatcherPoolName()
    , m_cycleCount( 0 )
    , m_clockJumpCount( 0 )
    , m_lastPlannedDelayMs( 0 )
    , m_dispatcherSystemThreadId( 0 )
    , m_totalRunsStarted( 0 )
    , m_totalRunsSucceeded( 0 )
    , m_totalRunsFailed( 0 )
    , m_totalRunsCancelled( 0 )
    , m_totalRunsTimedOut( 0 )
    , m_totalRunsSkipped( 0 )
    , m_submittedJobCount( 0 )
    , m_pendingChanges()
    , m_pendingRunChanges()
    , m_changesPosted( false )
    , m_changesPendingHint( 0 )
    , m_wakeLock()
    , m_dispatcherPulse()
    , m_mainThreadRunsQueued( 0 )
{GUCEF_TRACE;

    RegisterEvents();

    // a random id per instance: run ids are unique together with it, across restarts of the host
    if ( !CRandom::GetRandomBytes( &m_hostInstanceId, sizeof( m_hostInstanceId ) ) || 0 == m_hostInstanceId )
        m_hostInstanceId = m_monotonicClock->GetMonotonicTimeInMs() ^ (UInt64) (size_t) this;

    m_runLink = ( GUCEF_NEW CJobSchedulerRunLink( this, settings.GetHostName() ) )->CreateSharedPtr();

    // the default lane: unthrottled, DefaultLaneMaxConcurrent at once
    SLaneEntry& lane = m_lanes[ DefaultLaneName ];
    lane.definition.SetName( DefaultLaneName );
    lane.definition.SetMaxConcurrent( settings.GetDefaultLaneMaxConcurrent() );
    lane.revision = NextRevision();

    // the pulses of the main pulse generator carry out the MAIN_THREAD runs on its thread
    if ( !mainPulseGenerator.IsNULL() )
        SetPropagatePulseEvent( true );
}

/*-------------------------------------------------------------------------*/

CJobScheduler::~CJobScheduler()
{GUCEF_TRACE;

    Stop();

    // a run task that outlived the stop no longer reaches this object
    if ( !m_runLink.IsNULL() )
        m_runLink->Detach();

    // runs that did not end within the stop timeout: their waiters are told
    for ( TRunMap::iterator r = m_activeRuns.begin(); r != m_activeRuns.end(); ++r )
    {
        if ( GUCEF_NULL != (*r).second.future )
        {
            (*r).second.future->Cancel();
            GUCEF_DELETE (*r).second.future;
            (*r).second.future = GUCEF_NULL;
        }
    }

    SetPropagatePulseEvent( false );
    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

const CString&
CJobScheduler::GetName( void ) const
{GUCEF_TRACE;

    return m_settings.GetHostName();
}

/*-------------------------------------------------------------------------*/

const CJobSchedulerSettings&
CJobScheduler::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

PulseGeneratorPtr
CJobScheduler::GetMainPulseGenerator( void ) const
{GUCEF_TRACE;

    return m_mainPulseGenerator;
}

/*-------------------------------------------------------------------------*/

TJobSchedulerHostState
CJobScheduler::GetHostState( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_hostState;
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::SetClocks( const CIUtcClock* utcClock, const CIMonotonicClock* monotonicClock )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_utcClock = GUCEF_NULL != utcClock ? utcClock : &m_systemUtcClock;
    m_monotonicClock = GUCEF_NULL != monotonicClock ? monotonicClock : &CSystemMonotonicClock::Instance();
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::SetStateStore( CIJobSchedulerStateStore* store )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_stateStore = store;
}

/*-------------------------------------------------------------------------*/

UInt64
CJobScheduler::NextRevision( void )
{GUCEF_TRACE;

    return ++m_revision;
}

/*-------------------------------------------------------------------------*/

const CString&
CJobScheduler::EffectiveLaneName( const CJobDefinition& job ) const
{GUCEF_TRACE;

    return job.GetLaneName().IsNULLOrEmpty() ? DefaultLaneName : job.GetLaneName();
}

/*-------------------------------------------------------------------------*/

CString
CJobScheduler::EffectivePoolName( const CJobDefinition& job ) const
{GUCEF_TRACE;

    if ( JOBEXECUTIONTARGET_MAIN_THREAD == job.GetExecutionTarget() )
        return m_settings.GetMainThreadPoolName();
    return job.GetThreadPoolName().IsNULLOrEmpty() ? m_settings.GetRunPoolName() : job.GetThreadPoolName();
}

/*-------------------------------------------------------------------------*/

CJobScheduler::SLaneEntry*
CJobScheduler::FindLane( const CString& laneName )
{GUCEF_TRACE;

    TLaneMap::iterator i = m_lanes.find( laneName );
    return i == m_lanes.end() ? GUCEF_NULL : &(*i).second;
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::MarkStateDirty( UInt64 nowMonotonicMs )
{GUCEF_TRACE;

    if ( !m_stateDirty )
    {
        m_stateDirty = true;
        m_stateSaveDueMonotonicMs = nowMonotonicMs + m_settings.GetStateSaveDebounceMs();
    }
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::QueueChange( const SJobSchedulerChange& change, SDeferred& deferred )
{GUCEF_TRACE;

    // changes are hints, coalesced per row until the main pulse generator's thread delivers them: what an observer
    // misses (a row that changed twice) the snapshot after the revision gives. Bounded: one entry per job and lane, and
    // at most as many runs as the history keeps
    if ( JOBSCHEDULERCHANGE_RUN == change.kind )
    {
        m_pendingRunChanges[ change.runId ] = change;
        UInt32 maxRunChanges = m_settings.GetMaxRunHistory() > 64 ? m_settings.GetMaxRunHistory() : 64;
        while ( m_pendingRunChanges.size() > maxRunChanges )
            m_pendingRunChanges.erase( m_pendingRunChanges.begin() );
    }
    else
    {
        CString key = ToString( (Int32) change.kind ) + ":" + change.key;
        m_pendingChanges[ key ] = change;
    }
    if ( !m_changesPosted )
    {
        m_changesPosted = true;
        m_changesPendingHint.Store( 1 );
        deferred.postChanges = true;
    }
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::TouchJob( SJobEntry& job, SDeferred& deferred )
{GUCEF_TRACE;

    job.revision = NextRevision();
    SJobSchedulerChange change;
    change.kind = JOBSCHEDULERCHANGE_JOB;
    change.key = job.definition.GetId();
    change.revision = job.revision;
    QueueChange( change, deferred );
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::TouchLane( SLaneEntry& lane, SDeferred& deferred )
{GUCEF_TRACE;

    lane.revision = NextRevision();
    SJobSchedulerChange change;
    change.kind = JOBSCHEDULERCHANGE_LANE;
    change.key = lane.definition.GetName();
    change.revision = lane.revision;
    QueueChange( change, deferred );
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::TouchRun( SJobRunRecord& run, SDeferred& deferred )
{GUCEF_TRACE;

    run.revision = NextRevision();
    SJobSchedulerChange change;
    change.kind = JOBSCHEDULERCHANGE_RUN;
    change.key = run.jobId;
    change.runId = run.runId;
    change.runState = run.state;
    change.revision = run.revision;
    QueueChange( change, deferred );
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::TouchHost( SDeferred& deferred )
{GUCEF_TRACE;

    SJobSchedulerChange change;
    change.kind = JOBSCHEDULERCHANGE_HOST;
    change.key = GetName();
    change.hostState = m_hostState;
    change.revision = NextRevision();
    QueueChange( change, deferred );
}

/*-------------------------------------------------------------------------*/

TJobState
CJobScheduler::ComputeJobState( const SJobEntry& job ) const
{GUCEF_TRACE;

    if ( !job.enabled )
        return JOBSTATE_DISABLED;
    if ( job.ended && 0 == job.pendingRunCount && 0 == job.runningRunCount )
        return JOBSTATE_ENDED;
    TLaneMap::const_iterator lane = m_lanes.find( EffectiveLaneName( job.definition ) );
    if ( !job.available || ( lane != m_lanes.end() && !(*lane).second.available ) )
        return JOBSTATE_UNAVAILABLE;
    if ( job.runningRunCount > 0 )
        return JOBSTATE_RUNNING;
    if ( job.pendingRunCount > 0 )
        return job.waitingFor.IsNULLOrEmpty() ? JOBSTATE_QUEUED : JOBSTATE_WAITING;
    return JOBSTATE_SCHEDULED;
}

/*-------------------------------------------------------------------------*/

bool
CJobScheduler::AddLane( const CJobLaneDefinition& lane, CString& error )
{GUCEF_TRACE;

    if ( lane.GetName().IsNULLOrEmpty() )
    {
        error = "a lane needs a name";
        return false;
    }
    if ( 0 == lane.GetMaxConcurrent() )
    {
        error = "a lane needs a maximum of runs at once above 0";
        return false;
    }

    SDeferred deferred;
    {
        MT::CScopeMutex lock( m_lock );
        if ( m_lanes.find( lane.GetName() ) != m_lanes.end() )
        {
            error = "the lane " + lane.GetName() + " exists already";
            return false;
        }
        SLaneEntry& entry = m_lanes[ lane.GetName() ];
        entry.definition = lane;
        entry.mode = lane.GetInitialMode();

        // the kept mode and throttle state, when the state was loaded before the lane was added
        if ( m_stateLoaded )
        {
            const CDataNode* lanesNode = m_loadedState.FindChild( "lanes" );
            const CDataNode* laneState = GUCEF_NULL != lanesNode ? lanesNode->FindChild( lane.GetName() ) : GUCEF_NULL;
            if ( GUCEF_NULL != laneState )
            {
                CString mode = laneState->GetAttributeValueOrChildValueByName( "mode" ).AsString( CString::Empty, true );
                if ( !mode.IsNULLOrEmpty() )
                {
                    entry.mode = mode;
                    entry.modeOverridden = true;
                }
                const CDataNode* throttleState = laneState->FindChild( "throttle" );
                if ( GUCEF_NULL != throttleState && GUCEF_NULL != lane.GetThrottlePolicy() )
                {
                    CString loadError;
                    if ( !lane.GetThrottlePolicy()->LoadState( *throttleState, loadError ) )
                        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "JobScheduler(" << GetName() << "): the kept throttle state of lane " << lane.GetName() << " is not used: " << loadError );
                }
            }
        }
        TouchLane( entry, deferred );
    }
    CompleteDeferred( deferred );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CJobScheduler::RemoveLane( const CString& laneName )
{GUCEF_TRACE;

    if ( DefaultLaneName == laneName )
        return false;

    MT::CScopeMutex lock( m_lock );
    TLaneMap::iterator i = m_lanes.find( laneName );
    if ( i == m_lanes.end() )
        return false;

    // a lane that jobs or runs use stays
    for ( TJobMap::const_iterator j = m_jobs.begin(); j != m_jobs.end(); ++j )
        if ( EffectiveLaneName( (*j).second.definition ) == laneName )
            return false;
    if ( (*i).second.inFlight > 0 )
        return false;
    m_lanes.erase( i );
    NextRevision();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CJobScheduler::SetLaneAvailability( const CString& laneName, bool available, const CString& reason )
{GUCEF_TRACE;

    SDeferred deferred;
    {
        MT::CScopeMutex lock( m_lock );
        SLaneEntry* lane = FindLane( laneName );
        if ( GUCEF_NULL == lane )
            return false;
        lane->available = available;
        lane->unavailableReason = available ? CString::Empty : reason;
        TouchLane( *lane, deferred );
        for ( TJobMap::iterator j = m_jobs.begin(); j != m_jobs.end(); ++j )
            if ( EffectiveLaneName( (*j).second.definition ) == laneName )
                TouchJob( (*j).second, deferred );
        deferred.wakeDispatcher = available;
    }
    CompleteDeferred( deferred );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CJobScheduler::AddJobImpl( const CJobDefinition& job, CIJobWorkSource* source, bool fromState, CString& error )
{GUCEF_TRACE;

    if ( !job.IsValid( error ) )
        return false;
    if ( JOBTRIGGER_WORK_SOURCE == job.GetTrigger().GetKind() && GUCEF_NULL == source )
    {
        error = "a job with trigger WORK_SOURCE is added with its work source (AddWorkSourceJob())";
        return false;
    }
    if ( JOBTRIGGER_WORK_SOURCE != job.GetTrigger().GetKind() && GUCEF_NULL != source )
    {
        error = "a job with a work source has trigger WORK_SOURCE";
        return false;
    }

    // the task type must be one the task manager can carry out on the pool of the runs: asked without the lock
    CString poolName = EffectivePoolName( job );
    if ( !m_taskManager->IsTaskOfTaskTypeExecutable( job.GetTaskTypeName(), CString::Empty ) )
    {
        error = "no task consumer factory is registered for the task type " + job.GetTaskTypeName();
        return false;
    }

    SDeferred deferred;
    {
        MT::CScopeMutex lock( m_lock );

        if ( GUCEF_NULL == FindLane( EffectiveLaneName( job ) ) )
        {
            error = "the lane " + EffectiveLaneName( job ) + " of the job is not known";
            return false;
        }

        TJobMap::iterator existing = m_jobs.find( job.GetId() );
        if ( existing != m_jobs.end() )
        {
            if ( JOBORIGIN_SUBMITTED != job.GetOrigin() || JOBORIGIN_SUBMITTED != (*existing).second.definition.GetOrigin() )
            {
                error = "a job with the id " + job.GetId() + " exists already";
                return false;
            }
            // a submitted job replaced by a submitted job: its runs stay theirs, the definition changes
            (*existing).second.definition = job;
            (*existing).second.planned = false;
            TouchJob( (*existing).second, deferred );
        }
        else
        {
            SJobEntry& entry = m_jobs[ job.GetId() ];
            entry.definition = job;
            entry.workSource = source;
            entry.recentRuns.SetMaximum( m_settings.GetPerJobHistory() );
            if ( JOBORIGIN_SUBMITTED == job.GetOrigin() )
                ++m_submittedJobCount;

            if ( m_stateLoaded )
            {
                const CDataNode* jobsNode = m_loadedState.FindChild( "jobs" );
                const CDataNode* jobState = GUCEF_NULL != jobsNode ? jobsNode->FindChild( job.GetId() ) : GUCEF_NULL;
                if ( GUCEF_NULL != jobState )
                    ApplyLoadedJobState( entry, *jobState );
            }
            TouchJob( entry, deferred );
        }
        if ( JOBORIGIN_SUBMITTED == job.GetOrigin() && !fromState )
            MarkStateDirty( m_monotonicClock->GetMonotonicTimeInMs() );
        deferred.wakeDispatcher = true;
    }
    CompleteDeferred( deferred );

    // the kept state of a source added after the state was loaded
    if ( GUCEF_NULL != source )
    {
        CDataNode sourceState;
        bool hasState = false;
        {
            MT::CScopeMutex lock( m_lock );
            if ( m_stateLoaded )
            {
                const CDataNode* jobsNode = m_loadedState.FindChild( "jobs" );
                const CDataNode* jobState = GUCEF_NULL != jobsNode ? jobsNode->FindChild( job.GetId() ) : GUCEF_NULL;
                const CDataNode* stateNode = GUCEF_NULL != jobState ? jobState->FindChild( "workSource" ) : GUCEF_NULL;
                if ( GUCEF_NULL != stateNode )
                {
                    sourceState = *stateNode;
                    hasState = true;
                }
            }
        }
        if ( hasState )
        {
            CString loadError;
            if ( !source->LoadState( sourceState, loadError ) )
                GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "JobScheduler(" << GetName() << "): the kept state of the work source of job " << job.GetId() << " is not used: " << loadError );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CJobScheduler::AddJob( const CJobDefinition& job, CString& error )
{GUCEF_TRACE;

    CJobDefinition definition( job );
    definition.SetOrigin( JOBORIGIN_CODE );
    definition.SetSubmittedBy( CString::Empty );
    definition.SetSubmittedAtUtcMs( 0 );
    return AddJobImpl( definition, GUCEF_NULL, false, error );
}

/*-------------------------------------------------------------------------*/

bool
CJobScheduler::AddWorkSourceJob( const CJobDefinition& job, CIJobWorkSource* source, CString& error )
{GUCEF_TRACE;

    if ( GUCEF_NULL == source )
    {
        error = "a work source job needs its work source";
        return false;
    }
    CJobDefinition definition( job );
    definition.SetOrigin( JOBORIGIN_CODE );
    if ( JOBTRIGGER_WORK_SOURCE != definition.GetTrigger().GetKind() )
        definition.SetTrigger( CJobTrigger::WorkSource() );
    return AddJobImpl( definition, source, false, error );
}

/*-------------------------------------------------------------------------*/

bool
CJobScheduler::RemoveJob( const CString& jobId )
{GUCEF_TRACE;

    SDeferred deferred;
    {
        MT::CScopeMutex lock( m_lock );
        TJobMap::iterator j = m_jobs.find( jobId );
        if ( j == m_jobs.end() )
            return false;

        // its runs that did not start never will; those that run are asked to cancel
        Int64 nowUtcMs = m_utcClock->GetUtcTimeInMs();
        UInt64 nowMonotonicMs = m_monotonicClock->GetMonotonicTimeInMs();
        TUInt64Vector runIds;
        for ( TRunMap::iterator r = m_activeRuns.begin(); r != m_activeRuns.end(); ++r )
            if ( (*r).second.record.jobId == jobId )
                runIds.push_back( (*r).first );
        for ( TUInt64Vector::iterator i = runIds.begin(); i != runIds.end(); ++i )
        {
            SRunEntry& run = m_activeRuns[ (*i) ];
            if ( !run.taskQueued )
                EndRun( (*i), JOBRUNSTATE_CANCELLED, CString::Empty, "the job was removed", nowUtcMs, nowMonotonicMs, deferred );
            else
                run.cancelRequested = true;
        }
        if ( JOBORIGIN_SUBMITTED == (*j).second.definition.GetOrigin() && m_submittedJobCount > 0 )
            --m_submittedJobCount;
        m_jobs.erase( jobId );
        NextRevision();
        MarkStateDirty( nowMonotonicMs );
        SJobSchedulerChange change;
        change.kind = JOBSCHEDULERCHANGE_JOB;
        change.key = jobId;
        change.revision = m_revision;
        QueueChange( change, deferred );
        deferred.wakeDispatcher = true;
    }
    CompleteDeferred( deferred );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CJobScheduler::SetJobAvailability( const CString& jobId, bool available, const CString& reason )
{GUCEF_TRACE;

    SDeferred deferred;
    {
        MT::CScopeMutex lock( m_lock );
        TJobMap::iterator j = m_jobs.find( jobId );
        if ( j == m_jobs.end() )
            return false;
        (*j).second.available = available;
        (*j).second.unavailableReason = available ? CString::Empty : reason;
        TouchJob( (*j).second, deferred );
        deferred.wakeDispatcher = available;
    }
    CompleteDeferred( deferred );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CJobScheduler::GetJobDefinition( const CString& jobId, CJobDefinition& job ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TJobMap::const_iterator j = m_jobs.find( jobId );
    if ( j == m_jobs.end() )
        return false;
    job = (*j).second.definition;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::GetJobIds( CString::StringSet& jobIds ) const
{GUCEF_TRACE;

    jobIds.clear();
    MT::CScopeMutex lock( m_lock );
    for ( TJobMap::const_iterator j = m_jobs.begin(); j != m_jobs.end(); ++j )
        jobIds.insert( (*j).first );
}

/*-------------------------------------------------------------------------*/

bool
CJobScheduler::RegisterSubmittableTaskType( const CString& taskTypeName, const CIJobPayloadValidator* validator )
{GUCEF_TRACE;

    if ( taskTypeName.IsNULLOrEmpty() )
        return false;
    if ( !m_taskManager->IsTaskOfTaskTypeExecutable( taskTypeName, CString::Empty ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "JobScheduler(" << GetName() << "): the task type " << taskTypeName << " cannot be made submittable: no task consumer factory is registered for it" );
        return false;
    }

    MT::CScopeMutex lock( m_lock );
    m_submittable[ taskTypeName ].validator = validator;
    NextRevision();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CJobScheduler::UnregisterSubmittableTaskType( const CString& taskTypeName )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( 0 == m_submittable.erase( taskTypeName ) )
        return false;
    NextRevision();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::GetSubmittableTaskTypes( CString::StringSet& taskTypeNames ) const
{GUCEF_TRACE;

    taskTypeNames.clear();
    MT::CScopeMutex lock( m_lock );
    for ( TSubmittableTaskTypeMap::const_iterator i = m_submittable.begin(); i != m_submittable.end(); ++i )
        taskTypeNames.insert( (*i).first );
}

/*-------------------------------------------------------------------------*/

TJobControlStatus
CJobScheduler::SubmitJob( const SJobSubmission& submission, SJobControlResult& result )
{GUCEF_TRACE;

    result = SJobControlResult();
    const CJobDefinition& definition = submission.definition;
    const CString& taskType = definition.GetTaskTypeName();

    // 1. the task type must be submittable here
    const CIJobPayloadValidator* validator = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        TSubmittableTaskTypeMap::const_iterator i = m_submittable.find( taskType );
        if ( i == m_submittable.end() )
        {
            result.messages.push_back( "the task type \"" + taskType + "\" cannot be submitted to this scheduler" );
            result.revision = m_revision;
            return JOBCONTROLSTATUS_FAILED_PRECONDITION;
        }
        validator = (*i).second.validator;
    }

    // 2. the bound of the payload, in its serialized form
    UInt32 payloadBytes = 0;
    if ( !ComputePayloadBytes( definition.GetPayload(), payloadBytes ) )
    {
        result.messages.push_back( "the payload cannot be serialized" );
        return JOBCONTROLSTATUS_INVALID_ARGUMENT;
    }
    if ( 0 != m_settings.GetMaxSubmitPayloadBytes() && payloadBytes > m_settings.GetMaxSubmitPayloadBytes() )
    {
        result.messages.push_back( "the payload is " + ToString( payloadBytes ) + " bytes, the most this scheduler takes is " + ToString( m_settings.GetMaxSubmitPayloadBytes() ) );
        return JOBCONTROLSTATUS_RESOURCE_EXHAUSTED;
    }

    // 3. the definition itself
    CString error;
    if ( !definition.IsValid( error ) )
    {
        result.messages.push_back( error );
        return JOBCONTROLSTATUS_INVALID_ARGUMENT;
    }
    if ( JOBTRIGGER_WORK_SOURCE == definition.GetTrigger().GetKind() )
    {
        result.messages.push_back( "a job submitted as data cannot have a work source" );
        return JOBCONTROLSTATUS_INVALID_ARGUMENT;
    }

    // 4. the validator of the task type, outside the lock
    if ( GUCEF_NULL != validator )
    {
        CString reason;
        if ( !validator->ValidatePayload( taskType, definition.GetPayload(), reason ) )
        {
            result.messages.push_back( reason.IsNULLOrEmpty() ? CString( "the payload was refused" ) : reason );
            return JOBCONTROLSTATUS_INVALID_ARGUMENT;
        }
    }

    // 5. the task data factory of the task type must understand the payload
    if ( !PayloadIsEmpty( definition.GetPayload() ) )
    {
        CIDataNodeSerializableTaskDataBasicPtr taskData = m_taskManager->CreateCustomTaskDataForTaskTypeIfAvailable( taskType, CString::Empty );
        CDataNodeSerializableSettings serializerSettings;
        if ( taskData.IsNULL() || !taskData->Deserialize( definition.GetPayload(), serializerSettings ) )
        {
            result.messages.push_back( "the payload is not task data the task type \"" + taskType + "\" understands" );
            return JOBCONTROLSTATUS_INVALID_ARGUMENT;
        }
    }

    // 6. the place it takes: a job defined in code is never replaced, the number of submitted jobs is bounded
    {
        MT::CScopeMutex lock( m_lock );
        TJobMap::const_iterator existing = m_jobs.find( definition.GetId() );
        if ( existing != m_jobs.end() )
        {
            if ( JOBORIGIN_SUBMITTED != (*existing).second.definition.GetOrigin() )
            {
                result.messages.push_back( "the job " + definition.GetId() + " is defined by the service and cannot be replaced" );
                return JOBCONTROLSTATUS_FAILED_PRECONDITION;
            }
            if ( !submission.replaceExisting )
            {
                result.messages.push_back( "a job with the id " + definition.GetId() + " was submitted already" );
                return JOBCONTROLSTATUS_FAILED_PRECONDITION;
            }
        }
        else
        if ( 0 != m_settings.GetMaxSubmittedJobs() && m_submittedJobCount >= m_settings.GetMaxSubmittedJobs() )
        {
            result.messages.push_back( "this scheduler holds the most submitted jobs it takes (" + ToString( m_settings.GetMaxSubmittedJobs() ) + ")" );
            return JOBCONTROLSTATUS_RESOURCE_EXHAUSTED;
        }
        if ( GUCEF_NULL == FindLane( EffectiveLaneName( definition ) ) )
        {
            result.messages.push_back( "the lane " + EffectiveLaneName( definition ) + " is not known" );
            return JOBCONTROLSTATUS_INVALID_ARGUMENT;
        }
    }

    if ( submission.dryRun )
    {
        result.messages.push_back( "the submission is valid (dry run, nothing changed)" );
        result.revision = GetRevision();
        return JOBCONTROLSTATUS_OK;
    }

    CJobDefinition submitted( definition );
    submitted.SetOrigin( JOBORIGIN_SUBMITTED );
    submitted.SetSubmittedBy( submission.submittedBy );
    submitted.SetSubmittedAtUtcMs( m_utcClock->GetUtcTimeInMs() );
    if ( !AddJobImpl( submitted, GUCEF_NULL, false, error ) )
    {
        result.messages.push_back( error );
        return JOBCONTROLSTATUS_INVALID_ARGUMENT;
    }
    result.messages.push_back( "job " + submitted.GetId() + " submitted" );
    result.revision = GetRevision();
    GUCEF_LOG( LOGLEVEL_NORMAL, "JobScheduler(" << GetName() << "): job " << submitted.GetId() << " of task type " << taskType << " submitted by " << submission.submittedBy );
    return JOBCONTROLSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

bool
CJobScheduler::Start( CString& error )
{GUCEF_TRACE;

    return StartImpl( true, error );
}

/*-------------------------------------------------------------------------*/

bool
CJobScheduler::StartImpl( bool withDispatcherTask, CString& error )
{GUCEF_TRACE;

    SDeferred deferred;
    {
        MT::CScopeMutex lock( m_lock );
        if ( JOBSCHEDULERSTATE_CREATED != m_hostState && JOBSCHEDULERSTATE_STOPPED != m_hostState )
        {
            error = "the scheduler is " + CString( JobSchedulerHostStateToString( m_hostState ) );
            return false;
        }
        m_hostState = JOBSCHEDULERSTATE_STARTING;
        m_stopRequested = false;
        m_manualDispatch = !withDispatcherTask;
        m_startUtcMs = m_utcClock->GetUtcTimeInMs();
        m_startMonotonicMs = m_monotonicClock->GetMonotonicTimeInMs();
        m_lastCycleUtcMs = m_startUtcMs;
        m_lastCycleMonotonicMs = m_startMonotonicMs;
        for ( TJobMap::iterator j = m_jobs.begin(); j != m_jobs.end(); ++j )
            (*j).second.planned = false;
        TouchHost( deferred );
    }
    CompleteDeferred( deferred );

    // the pools of the scheduler, through the task manager, without the scheduler lock
    ThreadPoolPtr mainThreadPool = m_taskManager->GetOrCreateThreadPool( m_settings.GetMainThreadPoolName(), m_mainPulseGenerator.IsNULL() ? CCoreGlobal::Instance()->GetPulseGenerator() : m_mainPulseGenerator, true );
    ThreadPoolPtr runPool = m_taskManager->GetOrCreateThreadPool( m_settings.GetRunPoolName(), true );
    if ( mainThreadPool.IsNULL() || runPool.IsNULL() )
    {
        error = "the thread pools of the scheduler cannot be made";
        MT::CScopeMutex lock( m_lock );
        m_hostState = JOBSCHEDULERSTATE_STOPPED;
        return false;
    }
    // the main thread pool never gets a thread of its own: its tasks wait for the main pulse generator's thread
    mainThreadPool->SetDesiredMaxTotalNrOfThreads( 0 );

    if ( !withDispatcherTask )
        return true;

    ThreadPoolPtr dispatcherPool = m_taskManager->GetOrCreateThreadPool( m_settings.GetDispatcherPoolName(), true );
    if ( dispatcherPool.IsNULL() )
    {
        error = "the dispatcher pool " + m_settings.GetDispatcherPoolName() + " cannot be made";
        MT::CScopeMutex lock( m_lock );
        m_hostState = JOBSCHEDULERSTATE_STOPPED;
        return false;
    }

    {
        MT::CScopeMutex lock( m_lock );
        m_dispatcherActive = true;
        m_dispatcherPoolName = m_settings.GetDispatcherPoolName();
    }

    // the dispatcher: a task with a thread of its own on the dispatcher pool, which sleeps until its next step
    CTSharedPtr< CJobSchedulerTask, MT::CMutex > dispatcher( GUCEF_NEW CJobSchedulerTask( this ) );
    CFutureResult started = dispatcherPool->StartTaskWithConsumer( dispatcher, GUCEF_NULL, false );
    if ( started.HasNoFuture() )
    {
        error = "the dispatcher of the scheduler cannot be started";
        MT::CScopeMutex lock( m_lock );
        m_dispatcherActive = false;
        m_hostState = JOBSCHEDULERSTATE_STOPPED;
        return false;
    }
    {
        MT::CScopeMutex lock( m_lock );
        m_dispatcherTaskId = started.GetTaskId();
    }
    GUCEF_LOG( LOGLEVEL_NORMAL, "JobScheduler(" << GetName() << "): started, the dispatcher runs on pool " << m_settings.GetDispatcherPoolName() );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::OnDispatcherStarted( const PulseGeneratorPtr& dispatcherPulse )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_wakeLock );
    m_dispatcherPulse = dispatcherPulse;
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::OnDispatcherEnded( void )
{GUCEF_TRACE;

    // let go of it outside the lock: the last reference may end the pulse generator
    PulseGeneratorPtr pulse;
    {
        MT::CScopeMutex lock( m_wakeLock );
        pulse = m_dispatcherPulse;
        m_dispatcherPulse.Unlink();
    }
    pulse.Unlink();

    MT::CScopeMutex lock( m_lock );
    m_dispatcherActive = false;
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::RequestDispatch( void )
{GUCEF_TRACE;

    PulseGeneratorPtr pulse;
    {
        MT::CScopeMutex lock( m_wakeLock );
        pulse = m_dispatcherPulse;
    }
    if ( !pulse.IsNULL() )
        pulse->RequestImmediatePulse();
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::RequestMainThreadPulse( void )
{GUCEF_TRACE;

    // a pulse request through the driver only: never a pulse on this thread. Without a driver (an application that
    // pumps its main pulse generator itself) the queued runs are carried out at its next pulse
    if ( !m_mainPulseGenerator.IsNULL() )
        m_mainPulseGenerator->RequestImmediatePulseThroughDriver();
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::OnPumpedNotify( CNotifier* notifier    ,
                               const CEvent& eventid  ,
                               CICloneable* eventdata )
{GUCEF_TRACE;

    if ( CPulseGenerator::PulseEvent == eventid )
    {
        // the MAIN_THREAD runs, carried out by this thread: at most MaxMainThreadRunsPerPulse per pulse
        if ( m_mainThreadRunsQueued.Load() > 0 )
        {
            ThreadPoolPtr mainThreadPool = m_taskManager->GetThreadPool( m_settings.GetMainThreadPoolName() );
            if ( !mainThreadPool.IsNULL() )
            {
                UInt32 maxRuns = m_settings.GetMaxMainThreadRunsPerPulse() > 0 ? m_settings.GetMaxMainThreadRunsPerPulse() : 1;
                mainThreadPool->CarryOutQueuedTasksIfAny( maxRuns );
                UInt32 left = mainThreadPool->GetNrOfQueuedTasks();
                m_mainThreadRunsQueued.Store( left );
                if ( left > 0 )
                    RequestMainThreadPulse();
            }
            else
            {
                m_mainThreadRunsQueued.Store( 0 );
            }
        }

        // the coalesced changes of the model, delivered on this thread
        if ( m_changesPendingHint.Load() > 0 )
            DeliverPendingChanges();
        return;
    }

    CTSGNotifier::OnPumpedNotify( notifier, eventid, eventdata );
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::DeliverPendingChanges( void )
{GUCEF_TRACE;

    // on the main pulse generator's thread: the coalesced changes, in revision order, no lock held while notifying
    TChangeVector changes;
    {
        MT::CScopeMutex lock( m_lock );
        m_changesPosted = false;
        m_changesPendingHint.Store( 0 );
        for ( TChangeMap::iterator i = m_pendingChanges.begin(); i != m_pendingChanges.end(); ++i )
            changes.push_back( (*i).second );
        for ( TRunChangeMap::iterator i = m_pendingRunChanges.begin(); i != m_pendingRunChanges.end(); ++i )
            changes.push_back( (*i).second );
        m_pendingChanges.clear();
        m_pendingRunChanges.clear();
    }

    // revision order
    for ( size_t i=1; i<changes.size(); ++i )
    {
        SJobSchedulerChange change = changes[ i ];
        size_t j = i;
        while ( j > 0 && changes[ j-1 ].revision > change.revision )
        {
            changes[ j ] = changes[ j-1 ];
            --j;
        }
        changes[ j ] = change;
    }

    for ( TChangeVector::iterator i = changes.begin(); i != changes.end(); ++i )
    {
        TJobSchedulerChangeEventData eventData( *i );
        switch ( (*i).kind )
        {
            case JOBSCHEDULERCHANGE_JOB:    { if ( !NotifyObservers( JobChangedEvent, &eventData ) ) return; break; }
            case JOBSCHEDULERCHANGE_LANE:   { if ( !NotifyObservers( LaneChangedEvent, &eventData ) ) return; break; }
            case JOBSCHEDULERCHANGE_RUN:    { if ( !NotifyObservers( RunChangedEvent, &eventData ) ) return; break; }
            case JOBSCHEDULERCHANGE_HOST:   { if ( !NotifyObservers( HostStateChangedEvent, &eventData ) ) return; break; }
            default:                        { break; }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::CompleteDeferred( SDeferred& deferred )
{GUCEF_TRACE;

    // no lock of the scheduler is held here

    // the changes wait for the next pulse of the main pulse generator, which is asked for through its driver
    if ( deferred.postChanges )
        deferred.wakeMainThread = true;

    for ( TTaskActionVector::iterator i = deferred.cancellations.begin(); i != deferred.cancellations.end(); ++i )
    {
        ThreadPoolPtr pool = m_taskManager->GetThreadPool( (*i).threadPoolName );
        if ( !pool.IsNULL() )
            pool->RequestTaskCancellation( (*i).taskId, false, true );
    }

    Int64 nowUtcMs = m_utcClock->GetUtcTimeInMs();
    for ( TItemFinishedVector::iterator i = deferred.itemsFinished.begin(); i != deferred.itemsFinished.end(); ++i )
    {
        if ( GUCEF_NULL != (*i).source )
            (*i).source->OnItemFinished( (*i).item, (*i).run, nowUtcMs );
    }

    for ( TFutureCompletionVector::iterator i = deferred.futures.begin(); i != deferred.futures.end(); ++i )
    {
        (*i).future->Complete( GUCEF_NEW TCloneableJobRunRecord( (*i).run ), true );
        GUCEF_DELETE (*i).future;
        (*i).future = GUCEF_NULL;
    }

    if ( deferred.wakeMainThread )
        RequestMainThreadPulse();

    if ( deferred.wakeDispatcher )
        RequestDispatch();

    deferred = SDeferred();
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::BeginDrain( void )
{GUCEF_TRACE;

    SDeferred deferred;
    {
        MT::CScopeMutex lock( m_lock );
        if ( JOBSCHEDULERSTATE_STOPPED == m_hostState || JOBSCHEDULERSTATE_CREATED == m_hostState || JOBSCHEDULERSTATE_DRAINING == m_hostState )
            return;
        m_hostState = JOBSCHEDULERSTATE_DRAINING;
        CancelAllActiveRuns( "the scheduler drains", deferred );
        TouchHost( deferred );
        deferred.wakeDispatcher = true;
    }
    CompleteDeferred( deferred );
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::CancelAllActiveRuns( const CString& reason, SDeferred& deferred )
{GUCEF_TRACE;

    Int64 nowUtcMs = m_utcClock->GetUtcTimeInMs();
    UInt64 nowMonotonicMs = m_monotonicClock->GetMonotonicTimeInMs();
    TUInt64Vector notStarted;
    for ( TRunMap::iterator r = m_activeRuns.begin(); r != m_activeRuns.end(); ++r )
    {
        SRunEntry& run = (*r).second;
        if ( !run.taskQueued )
        {
            notStarted.push_back( (*r).first );
        }
        else
        if ( !run.cancelSent )
        {
            run.cancelRequested = true;
            if ( 0 != run.record.taskId )
            {
                run.cancelSent = true;
                STaskAction action;
                action.threadPoolName = run.record.threadPoolName;
                action.taskId = run.record.taskId;
                deferred.cancellations.push_back( action );
            }
        }
    }
    for ( TUInt64Vector::iterator i = notStarted.begin(); i != notStarted.end(); ++i )
        EndRun( (*i), JOBRUNSTATE_CANCELLED, CString::Empty, reason, nowUtcMs, nowMonotonicMs, deferred );
}

/*-------------------------------------------------------------------------*/

bool
CJobScheduler::IsIdle( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_activeRuns.empty() && m_endedTasks.empty();
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::Stop( void )
{GUCEF_TRACE;

    UInt32 dispatcherTaskId = 0;
    CString dispatcherPoolName;
    {
        MT::CScopeMutex lock( m_lock );
        if ( JOBSCHEDULERSTATE_CREATED == m_hostState || JOBSCHEDULERSTATE_STOPPED == m_hostState )
            return;
        m_stopRequested = true;
        dispatcherTaskId = m_dispatcherActive ? m_dispatcherTaskId : 0;
        dispatcherPoolName = m_dispatcherPoolName;
    }

    // no new run, the runs are asked to cancel
    BeginDrain();

    // the dispatcher ends at its next cycle: woken, then waited for (never from its own thread)
    RequestDispatch();
    if ( 0 != dispatcherTaskId )
    {
        ThreadPoolPtr dispatcherPool = m_taskManager->GetThreadPool( dispatcherPoolName );
        if ( !dispatcherPool.IsNULL() )
        {
            UInt32 waitedMs = 0;
            while ( waitedMs < 30000 )
            {
                {
                    MT::CScopeMutex lock( m_lock );
                    if ( !m_dispatcherActive )
                        break;
                }
                RequestDispatch();
                MT::ThreadDelay( 2 );
                waitedMs += 2;
            }
        }
    }

    // the runs that are still going: cooperative ones end within StopTimeoutMs
    UInt64 stopTimeoutMs = m_settings.GetStopTimeoutMs();
    UInt64 waitStartMs = CSystemMonotonicClock::Instance().GetMonotonicTimeInMs();
    SDeferred deferred;
    while ( true )
    {
        bool idle = false;
        {
            MT::CScopeMutex lock( m_lock );
            ProcessEndedTasks( m_utcClock->GetUtcTimeInMs(), m_monotonicClock->GetMonotonicTimeInMs(), deferred );
            idle = m_activeRuns.empty();
        }
        CompleteDeferred( deferred );
        if ( idle )
            break;
        if ( CSystemMonotonicClock::Instance().GetMonotonicTimeInMs() - waitStartMs >= stopTimeoutMs )
        {
            MT::CScopeMutex lock( m_lock );
            GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "JobScheduler(" << GetName() << "): " << (UInt32) m_activeRuns.size() << " runs did not end within the stop timeout of " << stopTimeoutMs << " ms" );
            break;
        }
        MT::ThreadDelay( 2 );
    }

    // the kept state, before the pools end
    {
        MT::CScopeMutex lock( m_lock );
        if ( GUCEF_NULL != m_stateStore )
            m_stateDirty = true;
    }
    SaveStateNow();

    {
        MT::CScopeMutex lock( m_lock );
        m_hostState = JOBSCHEDULERSTATE_STOPPED;
        TouchHost( deferred );
    }
    CompleteDeferred( deferred );
    GUCEF_LOG( LOGLEVEL_NORMAL, "JobScheduler(" << GetName() << "): stopped" );
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::OnRunTaskEnded( UInt64 runId, UInt32 taskId, TTaskStatus endStatus, const CString& statusExtra )
{GUCEF_TRACE;

    // from the thread that ended the task: noted for the dispatcher, which does the bookkeeping
    {
        MT::CScopeMutex lock( m_lock );
        SEndedTask ended;
        ended.runId = runId;
        ended.taskId = taskId;
        ended.status = endStatus;
        ended.statusExtra = statusExtra;
        m_endedTasks.push_back( ended );
    }
    RequestDispatch();
}

/*-------------------------------------------------------------------------*/

bool
CJobScheduler::OnRunProgress( UInt64 runId, UInt64 done, UInt64 total, const CString& note )
{GUCEF_TRACE;

    // from the thread of the run's task: the record always gets the latest values, RunChangedEvent at most once per
    // ProgressEventMinIntervalMs (the end of the run raises one anyway)
    SDeferred deferred;
    {
        MT::CScopeMutex lock( m_lock );
        TRunMap::iterator r = m_activeRuns.find( runId );
        if ( r == m_activeRuns.end() )
            return false;
        SRunEntry& run = (*r).second;
        if ( JOBRUNSTATE_RUNNING != run.record.state && JOBRUNSTATE_QUEUED != run.record.state && JOBRUNSTATE_WAITING != run.record.state )
            return false;
        TJobMap::iterator j = m_jobs.find( run.record.jobId );
        bool sensitive = j != m_jobs.end() && 0 != ( (*j).second.definition.GetFlags() & JOBFLAG_SENSITIVE );
        run.record.progressDone = done;
        run.record.progressTotal = total;
        run.record.note = TruncateText( sensitive ? CRedactor::ForDisplay().RedactText( note ) : note, SJobRunRecord::MaxNoteBytes );
        const UInt64 nowMonotonicMs = m_monotonicClock->GetMonotonicTimeInMs();
        const UInt64 minIntervalMs = m_settings.GetProgressEventMinIntervalMs();
        if ( 0 == run.lastProgressEventMonotonicMs || nowMonotonicMs >= run.lastProgressEventMonotonicMs + minIntervalMs )
        {
            run.lastProgressEventMonotonicMs = nowMonotonicMs > 0 ? nowMonotonicMs : 1;
            TouchRun( run.record, deferred );
        }
    }
    CompleteDeferred( deferred );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CJobScheduler::OnRunResultDocument( UInt64 runId, const CString& documentPath )
{GUCEF_TRACE;

    if ( documentPath.Length() > SJobRunRecord::MaxResultDocumentBytes )
        return false;
    SDeferred deferred;
    {
        MT::CScopeMutex lock( m_lock );
        TRunMap::iterator r = m_activeRuns.find( runId );
        if ( r == m_activeRuns.end() )
            return false;
        SRunEntry& run = (*r).second;
        run.record.resultDocument = documentPath;
        TouchRun( run.record, deferred );
    }
    CompleteDeferred( deferred );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::ApplyLoadedJobState( SJobEntry& job, const CDataNode& jobState )
{GUCEF_TRACE;

    job.lastOccurrenceUtcMs = jobState.GetAttributeValueOrChildValueByName( "lastOccurrenceUtcMs" ).AsInt64( job.lastOccurrenceUtcMs, true );
    job.enabled = jobState.GetAttributeValueOrChildValueByName( "enabled" ).AsBool( job.enabled, true );
    job.ended = jobState.GetAttributeValueOrChildValueByName( "ended" ).AsBool( job.ended, true );
    job.consecutiveFailures = jobState.GetAttributeValueOrChildValueByName( "consecutiveFailures" ).AsUInt32( job.consecutiveFailures, true );
    job.runCount = jobState.GetAttributeValueOrChildValueByName( "runCount" ).AsUInt64( job.runCount, true );
    job.failureCount = jobState.GetAttributeValueOrChildValueByName( "failureCount" ).AsUInt64( job.failureCount, true );
    CVariant priority = jobState.GetAttributeValueOrChildValueByName( "priority" );
    if ( !priority.IsNULLOrEmpty() )
    {
        job.definition.SetPriority( priority.AsUInt32( job.definition.GetPriority(), true ) );
        job.priorityOverridden = true;
    }
    job.planned = false;
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::LoadStateOnDispatcher( void )
{GUCEF_TRACE;

    CIJobSchedulerStateStore* store = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        store = m_stateStore;
    }

    CDataNode state;
    bool loaded = false;
    if ( GUCEF_NULL != store )
    {
        CString error;
        if ( store->LoadState( state, error ) )
        {
            loaded = true;
        }
        else
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "JobScheduler(" << GetName() << "): the kept state cannot be loaded, the scheduler starts from its defaults: " << error );
        }
    }

    // the submitted jobs come back: only for task types that are still submittable here
    TJobDefinitionVector submitted;
    if ( loaded )
    {
        const CDataNode* submittedNode = state.FindChild( "submittedJobs" );
        if ( GUCEF_NULL != submittedNode )
        {
            CDataNodeSerializableSettings serializerSettings;
            CDataNode::const_iterator i = submittedNode->ConstBegin();
            while ( i != submittedNode->ConstEnd() )
            {
                CJobDefinition definition;
                if ( definition.Deserialize( *(*i), serializerSettings ) )
                    submitted.push_back( definition );
                ++i;
            }
        }
    }

    typedef GUCEF::vector< std::pair< CIJobWorkSource*, CDataNode > > TSourceStateVector;
    TSourceStateVector sourceStates;
    SDeferred deferred;
    {
        MT::CScopeMutex lock( m_lock );
        m_stateLoaded = true;
        if ( loaded )
        {
            m_loadedState = state;
            const CDataNode* jobsNode = state.FindChild( "jobs" );
            if ( GUCEF_NULL != jobsNode )
            {
                for ( TJobMap::iterator j = m_jobs.begin(); j != m_jobs.end(); ++j )
                {
                    const CDataNode* jobState = jobsNode->FindChild( (*j).first );
                    if ( GUCEF_NULL == jobState )
                        continue;
                    ApplyLoadedJobState( (*j).second, *jobState );
                    const CDataNode* sourceState = jobState->FindChild( "workSource" );
                    if ( GUCEF_NULL != sourceState && GUCEF_NULL != (*j).second.workSource )
                        sourceStates.push_back( std::pair< CIJobWorkSource*, CDataNode >( (*j).second.workSource, *sourceState ) );
                    TouchJob( (*j).second, deferred );
                }
            }
            const CDataNode* lanesNode = state.FindChild( "lanes" );
            if ( GUCEF_NULL != lanesNode )
            {
                for ( TLaneMap::iterator l = m_lanes.begin(); l != m_lanes.end(); ++l )
                {
                    const CDataNode* laneState = lanesNode->FindChild( (*l).first );
                    if ( GUCEF_NULL == laneState )
                        continue;
                    CString mode = laneState->GetAttributeValueOrChildValueByName( "mode" ).AsString( CString::Empty, true );
                    if ( !mode.IsNULLOrEmpty() )
                    {
                        (*l).second.mode = mode;
                        (*l).second.modeOverridden = true;
                    }
                    const CDataNode* throttleState = laneState->FindChild( "throttle" );
                    CIJobThrottlePolicy* policy = (*l).second.definition.GetThrottlePolicy();
                    if ( GUCEF_NULL != throttleState && GUCEF_NULL != policy )
                    {
                        // a throttle policy is bookkeeping only (its contract): called under the scheduler lock
                        CString error;
                        if ( !policy->LoadState( *throttleState, error ) )
                            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "JobScheduler(" << GetName() << "): the kept throttle state of lane " << (*l).first << " is not used: " << error );
                    }
                    TouchLane( (*l).second, deferred );
                }
            }
            const CDataNode* runsNode = state.FindChild( "runs" );
            if ( GUCEF_NULL != runsNode )
            {
                // oldest first, so the history ends with the newest
                TJobRunRecordVector runs;
                CDataNode::const_iterator i = runsNode->ConstBegin();
                while ( i != runsNode->ConstEnd() )
                {
                    SJobRunRecord run;
                    DeserializeRunRecord( *(*i), run );
                    runs.push_back( run );
                    ++i;
                }
                for ( TJobRunRecordVector::reverse_iterator r = runs.rbegin(); r != runs.rend(); ++r )
                {
                    (*r).revision = NextRevision();
                    m_history.Add( (*r) );
                    TJobMap::iterator j = m_jobs.find( (*r).jobId );
                    if ( j != m_jobs.end() )
                        (*j).second.recentRuns.Add( (*r) );
                }
            }
        }
    }
    CompleteDeferred( deferred );

    for ( TSourceStateVector::iterator i = sourceStates.begin(); i != sourceStates.end(); ++i )
    {
        CString error;
        if ( !(*i).first->LoadState( (*i).second, error ) )
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "JobScheduler(" << GetName() << "): the kept state of a work source is not used: " << error );
    }

    for ( TJobDefinitionVector::iterator i = submitted.begin(); i != submitted.end(); ++i )
    {
        bool submittable = false;
        {
            MT::CScopeMutex lock( m_lock );
            submittable = m_submittable.find( (*i).GetTaskTypeName() ) != m_submittable.end();
        }
        CString error;
        (*i).SetOrigin( JOBORIGIN_SUBMITTED );
        if ( !submittable )
        {
            GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "JobScheduler(" << GetName() << "): the kept submitted job " << (*i).GetId() << " is not restored: its task type " << (*i).GetTaskTypeName() << " is not submittable to this scheduler" );
            continue;
        }
        if ( !AddJobImpl( (*i), GUCEF_NULL, true, error ) )
            GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "JobScheduler(" << GetName() << "): the kept submitted job " << (*i).GetId() << " is not restored: " << error );
    }
}

/*-------------------------------------------------------------------------*/

bool
CJobScheduler::BuildState( CDataNode& state ) const
{GUCEF_TRACE;

    state.Clear();
    state.SetName( "jobScheduler" );
    state.SetNodeType( GUCEF_DATATYPE_OBJECT );
    state.SetAttribute( "version", StateFormatVersion );
    state.SetAttribute( "hostName", GetName() );

    CDataNode* jobsNode = state.AddChild( "jobs", GUCEF_DATATYPE_OBJECT );
    CDataNode* submittedNode = state.AddChild( "submittedJobs", GUCEF_DATATYPE_ARRAY );
    CDataNodeSerializableSettings serializerSettings;
    for ( TJobMap::const_iterator j = m_jobs.begin(); j != m_jobs.end(); ++j )
    {
        const SJobEntry& job = (*j).second;
        CDataNode* jobNode = jobsNode->AddChild( job.definition.GetId(), GUCEF_DATATYPE_OBJECT );
        jobNode->SetAttribute( "lastOccurrenceUtcMs", job.lastOccurrenceUtcMs );
        jobNode->SetAttribute( "enabled", job.enabled );
        jobNode->SetAttribute( "ended", job.ended );
        jobNode->SetAttribute( "consecutiveFailures", job.consecutiveFailures );
        jobNode->SetAttribute( "runCount", job.runCount );
        jobNode->SetAttribute( "failureCount", job.failureCount );
        if ( job.priorityOverridden )
            jobNode->SetAttribute( "priority", job.definition.GetPriority() );
        if ( JOBORIGIN_SUBMITTED == job.definition.GetOrigin() )
        {
            CDataNode* definitionNode = submittedNode->AddChild( job.definition.GetId(), GUCEF_DATATYPE_OBJECT );
            job.definition.Serialize( *definitionNode, serializerSettings );
        }
    }

    // the kept state of jobs that are not (yet) added stays
    const CDataNode* loadedJobs = m_loadedState.FindChild( "jobs" );
    if ( GUCEF_NULL != loadedJobs )
    {
        CDataNode::const_iterator i = loadedJobs->ConstBegin();
        while ( i != loadedJobs->ConstEnd() )
        {
            if ( GUCEF_NULL == jobsNode->FindChild( (*i)->GetName() ) )
                jobsNode->AddChild( *(*i) );
            ++i;
        }
    }

    CDataNode* lanesNode = state.AddChild( "lanes", GUCEF_DATATYPE_OBJECT );
    for ( TLaneMap::const_iterator l = m_lanes.begin(); l != m_lanes.end(); ++l )
    {
        const SLaneEntry& lane = (*l).second;
        CDataNode* laneNode = lanesNode->AddChild( lane.definition.GetName(), GUCEF_DATATYPE_OBJECT );
        if ( lane.modeOverridden )
            laneNode->SetAttribute( "mode", lane.mode );
        CIJobThrottlePolicy* policy = lane.definition.GetThrottlePolicy();
        if ( GUCEF_NULL != policy )
        {
            CDataNode throttleState;
            if ( policy->SaveState( throttleState ) )
            {
                throttleState.SetName( "throttle" );
                laneNode->AddChild( throttleState );
            }
        }
    }

    if ( m_settings.GetPersistRunHistory() > 0 )
    {
        CDataNode* runsNode = state.AddChild( "runs", GUCEF_DATATYPE_ARRAY );
        TJobRunRecordVector runs;
        m_history.GetNewestFirst( runs, 0, m_settings.GetPersistRunHistory() );
        for ( TJobRunRecordVector::const_iterator r = runs.begin(); r != runs.end(); ++r )
        {
            CDataNode* runNode = runsNode->AddChild( "run", GUCEF_DATATYPE_OBJECT );
            SerializeRunRecord( (*r), *runNode );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CJobScheduler::SaveStateNow( void )
{GUCEF_TRACE;

    // the state is built under the lock (with the throttle policies' state, bookkeeping only), the work sources add
    // theirs and the store writes it without the lock
    CIJobSchedulerStateStore* store = GUCEF_NULL;
    CDataNode state;
    typedef GUCEF::vector< std::pair< CString, CIJobWorkSource* > > TSourceVector;
    TSourceVector sources;
    {
        MT::CScopeMutex lock( m_lock );
        store = m_stateStore;
        if ( GUCEF_NULL == store || !m_stateDirty )
            return true;
        m_stateDirty = false;
        BuildState( state );
        for ( TJobMap::const_iterator j = m_jobs.begin(); j != m_jobs.end(); ++j )
            if ( GUCEF_NULL != (*j).second.workSource )
                sources.push_back( std::pair< CString, CIJobWorkSource* >( (*j).first, (*j).second.workSource ) );
    }

    CDataNode* jobsNode = state.FindChild( "jobs" );
    for ( TSourceVector::iterator i = sources.begin(); i != sources.end(); ++i )
    {
        CDataNode sourceState;
        if ( (*i).second->SaveState( sourceState ) && GUCEF_NULL != jobsNode )
        {
            CDataNode* jobNode = jobsNode->FindChild( (*i).first );
            if ( GUCEF_NULL != jobNode )
            {
                sourceState.SetName( "workSource" );
                jobNode->AddChild( sourceState );
            }
        }
    }

    CString error;
    if ( store->SaveState( state, error ) )
        return true;

    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "JobScheduler(" << GetName() << "): the state cannot be saved now, it is tried again: " << error );
    MT::CScopeMutex lock( m_lock );
    UInt64 retryMs = m_settings.GetStateSaveDebounceMs() > FailedStateSaveRetryMs ? m_settings.GetStateSaveDebounceMs() : FailedStateSaveRetryMs;
    m_stateDirty = true;
    m_stateSaveDueMonotonicMs = m_monotonicClock->GetMonotonicTimeInMs() + retryMs;
    return false;
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::PlanJob( SJobEntry& job, Int64 nowUtcMs, UInt64 nowMonotonicMs )
{GUCEF_TRACE;

    // the due instants of a job, at the first cycle after the start, after an add or after it was enabled again
    const CJobTrigger& trigger = job.definition.GetTrigger();
    switch ( trigger.GetKind() )
    {
        case JOBTRIGGER_SCHEDULE:
        {
            // a job without a kept occurrence starts at the first occurrence after the start: no catch up for it
            if ( 0 == job.lastOccurrenceUtcMs )
                job.lastOccurrenceUtcMs = nowUtcMs;
            job.nextDueUtcMs = NextOccurrenceAfter( trigger.GetSchedule(), job.lastOccurrenceUtcMs );
            break;
        }
        case JOBTRIGGER_INTERVAL:
        {
            UInt64 periodMs = trigger.GetPeriodMs();
            if ( trigger.GetHasAlignment() )
            {
                // the next instant align + k * period after now, in UTC, carried over to the monotonic clock
                Int64 align = trigger.GetAlignToUtcMs();
                Int64 sinceAlign = nowUtcMs - align;
                Int64 k = sinceAlign >= 0 ? ( sinceAlign / (Int64) periodMs ) + 1 : 0;
                Int64 nextUtcMs = align + k * (Int64) periodMs;
                while ( nextUtcMs <= nowUtcMs )
                    nextUtcMs += (Int64) periodMs;
                job.nextDueMonotonicMs = nowMonotonicMs + (UInt64) ( nextUtcMs - nowUtcMs );
            }
            else
            {
                job.nextDueMonotonicMs = nowMonotonicMs + periodMs;
            }
            job.nextDueUtcMs = nowUtcMs + (Int64) ( job.nextDueMonotonicMs - nowMonotonicMs );
            break;
        }
        case JOBTRIGGER_ONCE_AT:
        {
            job.nextDueUtcMs = job.ended ? 0 : trigger.GetOnceAtUtcMs();
            break;
        }
        default:
        {
            job.nextDueUtcMs = 0;
            break;
        }
    }
    job.planned = true;
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::RecordSkippedRun( SJobEntry& job, TJobRunCause cause, Int64 scheduledUtcMs, const CString& reason, SDeferred& deferred )
{GUCEF_TRACE;

    SJobRunRecord run;
    run.runId = m_nextRunId++;
    run.hostInstanceId = m_hostInstanceId;
    run.jobId = job.definition.GetId();
    run.taskTypeName = job.definition.GetTaskTypeName();
    run.executionTarget = job.definition.GetExecutionTarget();
    run.threadPoolName = EffectivePoolName( job.definition );
    run.cause = cause;
    run.scheduledUtcMs = scheduledUtcMs;
    run.queuedUtcMs = m_utcClock->GetUtcTimeInMs();
    run.endedUtcMs = run.queuedUtcMs;
    run.attempt = 1;
    run.state = JOBRUNSTATE_SKIPPED;
    run.outcomeMessage = TruncateText( reason, SJobRunRecord::MaxOutcomeMessageBytes );
    TouchRun( run, deferred );
    m_history.Add( run );
    job.recentRuns.Add( run );
    job.lastRunId = run.runId;
    job.lastRunState = run.state;
    job.lastRunEndedUtcMs = run.endedUtcMs;
    ++m_totalRunsSkipped;
    TouchJob( job, deferred );
}

/*-------------------------------------------------------------------------*/

UInt64
CJobScheduler::QueueRun( SJobEntry& job, TJobRunCause cause, Int64 scheduledUtcMs, UInt32 priority, const CString& requestedBy,
                         const SJobWorkItem* workItem, SDeferred& deferred, bool applyConcurrency )
{GUCEF_TRACE;

    if ( applyConcurrency )
    {
        UInt32 active = job.pendingRunCount + job.runningRunCount;
        switch ( job.definition.GetConcurrency() )
        {
            case JOBCONCURRENCY_SKIP_IF_RUNNING:
            {
                if ( active > 0 )
                {
                    RecordSkippedRun( job, cause, scheduledUtcMs, "a run of the job is still going (SKIP_IF_RUNNING)", deferred );
                    return 0;
                }
                break;
            }
            case JOBCONCURRENCY_QUEUE_ONE:
            {
                if ( job.pendingRunCount > 0 )
                {
                    RecordSkippedRun( job, cause, scheduledUtcMs, "a run of the job is pending already (QUEUE_ONE)", deferred );
                    return 0;
                }
                break;
            }
            case JOBCONCURRENCY_PARALLEL:
            default:
            {
                UInt32 maxParallel = job.definition.GetMaxParallelRuns() > 0 ? job.definition.GetMaxParallelRuns() : 1;
                if ( active >= maxParallel )
                {
                    RecordSkippedRun( job, cause, scheduledUtcMs, "the job has its most runs at once (PARALLEL)", deferred );
                    return 0;
                }
                break;
            }
        }
    }

    UInt64 runId = m_nextRunId++;
    SRunEntry& run = m_activeRuns[ runId ];
    run.record.runId = runId;
    run.record.hostInstanceId = m_hostInstanceId;
    run.record.jobId = job.definition.GetId();
    run.record.taskTypeName = job.definition.GetTaskTypeName();
    run.record.executionTarget = job.definition.GetExecutionTarget();
    run.record.threadPoolName = EffectivePoolName( job.definition );
    run.record.cause = cause;
    run.record.scheduledUtcMs = scheduledUtcMs;
    run.record.queuedUtcMs = m_utcClock->GetUtcTimeInMs();
    run.record.attempt = 1;
    run.record.state = JOBRUNSTATE_QUEUED;
    run.record.requestedBy = requestedBy;
    run.priority = priority > 3 ? 3 : priority;
    run.sequence = m_nextSequence++;
    run.timeoutMs = job.definition.GetTimeoutMs();
    run.laneName = EffectiveLaneName( job.definition );
    run.estimatedCost = 1;
    if ( GUCEF_NULL != workItem )
    {
        run.hasWorkItem = true;
        run.workItem = *workItem;
        if ( !workItem->laneName.IsNULLOrEmpty() )
            run.laneName = workItem->laneName;
        if ( !workItem->taskTypeName.IsNULLOrEmpty() )
            run.record.taskTypeName = workItem->taskTypeName;
        run.record.itemKey = workItem->key;
        run.record.itemLabel = workItem->publicLabel;
        run.estimatedCost = workItem->estimatedCost > 0 ? workItem->estimatedCost : 1;
    }
    run.record.actualCost = run.estimatedCost;
    TouchRun( run.record, deferred );

    ++job.pendingRunCount;
    TouchJob( job, deferred );
    return runId;
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::EvaluateDueJobs( Int64 nowUtcMs, UInt64 nowMonotonicMs, SDeferred& deferred )
{GUCEF_TRACE;

    for ( TJobMap::iterator j = m_jobs.begin(); j != m_jobs.end(); ++j )
    {
        SJobEntry& job = (*j).second;
        if ( !job.planned )
        {
            // a new due instant is a change of the job: views that follow the changes (GetSnapshot( revision ), the
            // change events) see it
            const Int64 dueBefore = job.nextDueUtcMs;
            PlanJob( job, nowUtcMs, nowMonotonicMs );
            if ( dueBefore != job.nextDueUtcMs )
                TouchJob( job, deferred );
        }
        if ( !job.enabled || job.ended )
            continue;

        const CJobTrigger& trigger = job.definition.GetTrigger();
        switch ( trigger.GetKind() )
        {
            case JOBTRIGGER_SCHEDULE:
            {
                if ( 0 == job.nextDueUtcMs || nowUtcMs < job.nextDueUtcMs )
                    break;

                // the occurrences in ( last handled, now ]: the latest one counts as on time unless it fell before the
                // start of this host or is later than the maximum lateness; the others were missed
                GUCEF::vector< Int64 > occurrences;
                Int64 cursor = job.lastOccurrenceUtcMs;
                Int64 next = job.nextDueUtcMs;
                bool more = false;
                while ( 0 != next && next <= nowUtcMs )
                {
                    if ( occurrences.size() >= MaxOccurrencesPerEvaluation )
                    {
                        more = true;
                        break;
                    }
                    occurrences.push_back( next );
                    cursor = next;
                    next = NextOccurrenceAfter( trigger.GetSchedule(), cursor );
                }
                if ( more )
                {
                    // a long gap: skip to the latest occurrence before now
                    CDateTime latest;
                    if ( trigger.GetSchedule().GetPreviousOccurrence( FromUtcMs( nowUtcMs + 1 ), latest ) && ToUtcMs( latest ) > occurrences.back() )
                        occurrences.push_back( ToUtcMs( latest ) );
                }
                if ( occurrences.empty() )
                    break;

                Int64 latestUtcMs = occurrences.back();
                bool latestOnTime = latestUtcMs >= m_startUtcMs &&
                                    ( 0 == job.definition.GetMaxLatenessMs() || (UInt64) ( nowUtcMs - latestUtcMs ) <= job.definition.GetMaxLatenessMs() );
                UInt32 missed = (UInt32) occurrences.size() - ( latestOnTime ? 1 : 0 );
                if ( more )
                    missed = MaxOccurrencesPerEvaluation;    // at least

                switch ( job.definition.GetMissedRunPolicy() )
                {
                    case JOBMISSEDRUNPOLICY_SKIP:
                    {
                        if ( missed > 0 )
                            GUCEF_LOG( LOGLEVEL_NORMAL, "JobScheduler(" << GetName() << "): job " << job.definition.GetId() << " missed " << missed << " occurrences, skipped by its policy" );
                        if ( latestOnTime )
                            QueueRun( job, JOBRUNCAUSE_SCHEDULE, latestUtcMs, job.definition.GetPriority(), CString::Empty, GUCEF_NULL, deferred, true );
                        break;
                    }
                    case JOBMISSEDRUNPOLICY_RUN_EACH:
                    {
                        UInt32 catchUps = missed < job.definition.GetMaxCatchUpRuns() ? missed : job.definition.GetMaxCatchUpRuns();
                        UInt32 missedEnd = latestOnTime ? (UInt32) occurrences.size() - 1 : (UInt32) occurrences.size();
                        UInt32 first = missedEnd >= catchUps ? missedEnd - catchUps : 0;
                        bool firstRun = true;
                        for ( UInt32 i=first; i<missedEnd; ++i )
                        {
                            // the runs of one evaluation are all queued; a job that is not PARALLEL runs them one after the other
                            QueueRun( job, JOBRUNCAUSE_CATCH_UP, occurrences[ i ], job.definition.GetPriority(), CString::Empty, GUCEF_NULL, deferred, firstRun );
                            firstRun = false;
                        }
                        if ( latestOnTime )
                            QueueRun( job, JOBRUNCAUSE_SCHEDULE, latestUtcMs, job.definition.GetPriority(), CString::Empty, GUCEF_NULL, deferred, firstRun );
                        break;
                    }
                    case JOBMISSEDRUNPOLICY_RUN_ONCE:
                    default:
                    {
                        // coalesced into one run
                        QueueRun( job, latestOnTime ? JOBRUNCAUSE_SCHEDULE : JOBRUNCAUSE_CATCH_UP, latestUtcMs, job.definition.GetPriority(), CString::Empty, GUCEF_NULL, deferred, true );
                        break;
                    }
                }

                job.lastOccurrenceUtcMs = latestUtcMs;
                job.nextDueUtcMs = NextOccurrenceAfter( trigger.GetSchedule(), latestUtcMs > nowUtcMs ? latestUtcMs : nowUtcMs );
                if ( 0 == job.nextDueUtcMs && trigger.GetSchedule().HasTimeRange() )
                    job.ended = true;
                MarkStateDirty( nowMonotonicMs );
                TouchJob( job, deferred );
                break;
            }
            case JOBTRIGGER_INTERVAL:
            {
                if ( 0 == job.nextDueMonotonicMs || nowMonotonicMs < job.nextDueMonotonicMs )
                    break;

                UInt64 periodMs = trigger.GetPeriodMs();
                UInt64 lateMs = nowMonotonicMs - job.nextDueMonotonicMs;
                UInt32 missed = (UInt32) ( lateMs / periodMs );
                Int64 dueUtcMs = nowUtcMs - (Int64) lateMs;
                if ( JOBMISSEDRUNPOLICY_RUN_EACH == job.definition.GetMissedRunPolicy() && missed > 0 )
                {
                    UInt32 catchUps = missed < job.definition.GetMaxCatchUpRuns() ? missed : job.definition.GetMaxCatchUpRuns();
                    for ( UInt32 i=0; i<catchUps; ++i )
                        QueueRun( job, JOBRUNCAUSE_CATCH_UP, dueUtcMs, job.definition.GetPriority(), CString::Empty, GUCEF_NULL, deferred, 0 == i );
                    QueueRun( job, JOBRUNCAUSE_SCHEDULE, dueUtcMs + (Int64) ( missed * periodMs ), job.definition.GetPriority(), CString::Empty, GUCEF_NULL, deferred, false );
                }
                else
                {
                    QueueRun( job, JOBRUNCAUSE_SCHEDULE, dueUtcMs, job.definition.GetPriority(), CString::Empty, GUCEF_NULL, deferred, true );
                }

                if ( JOBINTERVALFROM_END == trigger.GetIntervalFrom() )
                {
                    // the next due instant is set when the run ends
                    job.nextDueMonotonicMs = 0;
                    job.nextDueUtcMs = 0;
                }
                else
                {
                    job.nextDueMonotonicMs += ( (UInt64) missed + 1 ) * periodMs;
                    job.nextDueUtcMs = nowUtcMs + (Int64) ( job.nextDueMonotonicMs - nowMonotonicMs );
                }
                TouchJob( job, deferred );
                break;
            }
            case JOBTRIGGER_ONCE_AT:
            {
                Int64 onceAt = trigger.GetOnceAtUtcMs();
                if ( nowUtcMs < onceAt )
                    break;

                // an instant that passed while the host was down is a missed occurrence
                bool missed = onceAt < m_startUtcMs ||
                              ( 0 != job.definition.GetMaxLatenessMs() && (UInt64) ( nowUtcMs - onceAt ) > job.definition.GetMaxLatenessMs() );
                if ( !missed )
                    QueueRun( job, JOBRUNCAUSE_SCHEDULE, onceAt, job.definition.GetPriority(), CString::Empty, GUCEF_NULL, deferred, true );
                else
                if ( JOBMISSEDRUNPOLICY_SKIP == job.definition.GetMissedRunPolicy() )
                    RecordSkippedRun( job, JOBRUNCAUSE_CATCH_UP, onceAt, "the instant was missed (SKIP)", deferred );
                else
                    QueueRun( job, JOBRUNCAUSE_CATCH_UP, onceAt, job.definition.GetPriority(), CString::Empty, GUCEF_NULL, deferred, true );

                job.lastOccurrenceUtcMs = onceAt;
                job.ended = true;
                job.nextDueUtcMs = 0;
                MarkStateDirty( nowMonotonicMs );
                TouchJob( job, deferred );
                break;
            }
            default:
                break;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::SelectRunsToStart( Int64 nowUtcMs, UInt64 nowMonotonicMs, TStartOrderVector& orders, SDeferred& deferred )
{GUCEF_TRACE;

    // the pending runs in dispatch order
    GUCEF::vector< SPendingRunOrder > pending;
    for ( TRunMap::iterator r = m_activeRuns.begin(); r != m_activeRuns.end(); ++r )
    {
        SRunEntry& run = (*r).second;
        if ( run.taskQueued )
            continue;
        SPendingRunOrder order;
        order.priority = run.priority;
        order.scheduledUtcMs = run.record.scheduledUtcMs;
        order.sequence = run.sequence;
        order.runId = (*r).first;
        pending.push_back( order );
    }
    std::sort( pending.begin(), pending.end() );

    // what each job and lane waits for, set again below
    for ( TJobMap::iterator j = m_jobs.begin(); j != m_jobs.end(); ++j )
    {
        (*j).second.waitingFor.Clear();
        (*j).second.waitingUntilUtcMs = 0;
    }
    for ( TLaneMap::iterator l = m_lanes.begin(); l != m_lanes.end(); ++l )
    {
        if ( !(*l).second.waitingFor.IsNULLOrEmpty() )
        {
            (*l).second.waitingFor.Clear();
            (*l).second.waitingUntilUtcMs = 0;
            TouchLane( (*l).second, deferred );
        }
    }

    bool startDelayed = m_settings.GetStartDelayMs() > 0 && nowMonotonicMs < m_startMonotonicMs + m_settings.GetStartDelayMs();

    for ( GUCEF::vector< SPendingRunOrder >::iterator p = pending.begin(); p != pending.end(); ++p )
    {
        SRunEntry& run = m_activeRuns[ (*p).runId ];
        TJobMap::iterator j = m_jobs.find( run.record.jobId );
        if ( j == m_jobs.end() )
            continue;
        SJobEntry& job = (*j).second;
        SLaneEntry* lane = FindLane( run.laneName );

        CString waitingFor;
        Int64 waitingUntilUtcMs = 0;
        if ( startDelayed )
        {
            waitingFor = "startDelay";
            waitingUntilUtcMs = nowUtcMs + (Int64) ( m_startMonotonicMs + m_settings.GetStartDelayMs() - nowMonotonicMs );
        }
        else
        if ( GUCEF_NULL == lane )
            waitingFor = "lane " + run.laneName + " unknown";
        else
        if ( !job.available )
            waitingFor = "job unavailable";
        else
        if ( !lane->available )
            waitingFor = "lane unavailable";
        else
        if ( lane->mode.Equals( CJobLaneDefinition::DisabledModeName, false ) )
            waitingFor = "lane disabled";
        else
        if ( !lane->heldBy.IsNULLOrEmpty() )
            waitingFor = "hold:" + lane->heldBy;
        else
        if ( 0 != run.notBeforeMonotonicMs && nowMonotonicMs < run.notBeforeMonotonicMs )
        {
            waitingFor = "backoff";
            waitingUntilUtcMs = nowUtcMs + (Int64) ( run.notBeforeMonotonicMs - nowMonotonicMs );
        }
        else
        if ( JOBCONCURRENCY_PARALLEL != job.definition.GetConcurrency() && job.runningRunCount > 0 )
            waitingFor = "job running";
        else
        if ( JOBCONCURRENCY_PARALLEL == job.definition.GetConcurrency() && job.runningRunCount >= ( job.definition.GetMaxParallelRuns() > 0 ? job.definition.GetMaxParallelRuns() : 1 ) )
            waitingFor = "job running";
        else
        if ( lane->inFlight >= lane->definition.GetMaxConcurrent() )
            waitingFor = "lane:" + run.laneName;
        else
        {
            CIJobThrottlePolicy* policy = lane->definition.GetThrottlePolicy();
            SJobPermitRequest request;
            request.laneName = run.laneName;
            request.jobId = run.record.jobId;
            request.itemKey = run.record.itemKey;
            request.cause = run.record.cause;
            request.priority = run.priority;
            request.estimatedCost = run.estimatedCost;
            if ( GUCEF_NULL != policy )
            {
                // bookkeeping only, under the lock (the policy's contract)
                SJobPermitAnswer answer;
                policy->Check( request, nowUtcMs, nowMonotonicMs, answer );
                if ( !answer.granted )
                {
                    waitingFor = "throttle:" + answer.bindingLimit;
                    waitingUntilUtcMs = nowUtcMs + (Int64) answer.waitMs;
                    lane->waitingFor = waitingFor;
                    if ( 0 == lane->waitingUntilUtcMs || waitingUntilUtcMs < lane->waitingUntilUtcMs )
                        lane->waitingUntilUtcMs = waitingUntilUtcMs;
                    TouchLane( *lane, deferred );
                }
                else
                {
                    policy->OnStarted( request, nowUtcMs, nowMonotonicMs );
                }
            }

            if ( waitingFor.IsNULLOrEmpty() )
            {
                // granted: the run starts
                run.taskQueued = true;
                run.startedMonotonicMs = nowMonotonicMs;
                run.record.startedUtcMs = nowUtcMs;
                run.record.state = JOBRUNSTATE_RUNNING;
                ++lane->inFlight;
                TouchLane( *lane, deferred );
                if ( job.pendingRunCount > 0 )
                    --job.pendingRunCount;
                ++job.runningRunCount;
                ++m_totalRunsStarted;
                TouchRun( run.record, deferred );
                TouchJob( job, deferred );

                SStartOrder order;
                order.runId = run.record.runId;
                order.jobId = job.definition.GetId();
                order.taskTypeName = run.record.taskTypeName;
                order.taskData = GUCEF_NULL;
                if ( run.hasWorkItem )
                {
                    order.payload = run.workItem.payload;
                    if ( GUCEF_NULL != run.workItem.taskData )
                        order.taskData = run.workItem.taskData->Clone();
                }
                else
                {
                    order.payload = job.definition.GetPayload();
                }
                order.executionTarget = run.record.executionTarget;
                order.threadPoolName = run.record.threadPoolName;
                orders.push_back( order );
                continue;
            }
        }

        // the run waits
        if ( JOBRUNSTATE_WAITING != run.record.state )
        {
            run.record.state = JOBRUNSTATE_WAITING;
            TouchRun( run.record, deferred );
        }
        if ( job.waitingFor.IsNULLOrEmpty() )
        {
            job.waitingFor = waitingFor;
            job.waitingUntilUtcMs = waitingUntilUtcMs;
            TouchJob( job, deferred );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::StartOrders( TStartOrderVector& orders )
{GUCEF_TRACE;

    // no lock of the scheduler held: the task manager and its pools are called here (lock rule 8)
    CDataNodeSerializableSettings serializerSettings;
    for ( TStartOrderVector::iterator o = orders.begin(); o != orders.end(); ++o )
    {
        SStartOrder& order = (*o);
        CString failure;
        CFutureResult future( TASKSTATUS_UNDEFINED );
        bool queued = false;

        ThreadPoolPtr pool = m_taskManager->GetOrCreateThreadPool( order.threadPoolName, true );
        if ( pool.IsNULL() )
        {
            failure = "the thread pool " + order.threadPoolName + " cannot be made";
        }
        else
        if ( GUCEF_NULL != order.taskData )
        {
            future = pool->QueueScheduledRunTask( order.taskTypeName, order.taskData, true, m_runLink, order.jobId, order.runId );
            order.taskData = GUCEF_NULL;    // owned by the task now, or deleted by the pool when it was refused
            queued = !future.HasNoFuture();
        }
        else
        if ( !PayloadIsEmpty( order.payload ) )
        {
            // the task data factory of the run's pool, else any registered for the task type (the task manager's own included)
            CIDataNodeSerializableTaskDataBasicPtr taskData = m_taskManager->CreateCustomTaskDataForTaskTypeIfAvailable( order.taskTypeName, order.threadPoolName );
            if ( taskData.IsNULL() )
                taskData = m_taskManager->CreateCustomTaskDataForTaskTypeIfAvailable( order.taskTypeName, CString::Empty );
            if ( taskData.IsNULL() )
                failure = "no task data factory is registered for the task type " + order.taskTypeName;
            else
            if ( !taskData->Deserialize( order.payload, serializerSettings ) )
                failure = "the payload is not task data the task type " + order.taskTypeName + " understands";
            else
            {
                future = pool->QueueScheduledRunTask( order.taskTypeName, taskData.GetPointerAlways(), false, m_runLink, order.jobId, order.runId );
                queued = !future.HasNoFuture();
            }
        }
        else
        {
            future = pool->QueueScheduledRunTask( order.taskTypeName, GUCEF_NULL, false, m_runLink, order.jobId, order.runId );
            queued = !future.HasNoFuture();
        }

        if ( queued && JOBEXECUTIONTARGET_MAIN_THREAD == order.executionTarget )
        {
            // posted, never waited for: the main pulse generator's thread carries it out at its next pulse
            m_mainThreadRunsQueued.FetchAdd( 1 );
            RequestMainThreadPulse();
        }

        SDeferred deferred;
        {
            MT::CScopeMutex lock( m_lock );
            TRunMap::iterator r = m_activeRuns.find( order.runId );
            if ( r != m_activeRuns.end() )
            {
                SRunEntry& run = (*r).second;
                if ( queued )
                {
                    run.record.taskId = future.GetTaskId();
                    TouchRun( run.record, deferred );
                    if ( run.cancelRequested && !run.cancelSent )
                    {
                        run.cancelSent = true;
                        STaskAction action;
                        action.threadPoolName = order.threadPoolName;
                        action.taskId = run.record.taskId;
                        deferred.cancellations.push_back( action );
                    }
                }
                else
                {
                    if ( failure.IsNULLOrEmpty() )
                        failure = "the task manager refused the task (the pool accepts no new work or has no task object left)";
                    EndRun( order.runId, JOBRUNSTATE_FAILED, CString::Empty, failure, m_utcClock->GetUtcTimeInMs(), m_monotonicClock->GetMonotonicTimeInMs(), deferred );
                }
            }
        }
        CompleteDeferred( deferred );
        if ( !queued )
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "JobScheduler(" << GetName() << "): a run of job " << order.jobId << " cannot start: " << failure );
    }

    for ( TStartOrderVector::iterator o = orders.begin(); o != orders.end(); ++o )
    {
        GUCEF_DELETE (*o).taskData;
        (*o).taskData = GUCEF_NULL;
    }
    orders.clear();
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::EndRun( UInt64 runId, TJobRunState state, const CString& note, const CString& message, Int64 nowUtcMs, UInt64 nowMonotonicMs, SDeferred& deferred )
{GUCEF_TRACE;

    TRunMap::iterator r = m_activeRuns.find( runId );
    if ( r == m_activeRuns.end() )
        return;
    SRunEntry& run = (*r).second;
    TJobMap::iterator j = m_jobs.find( run.record.jobId );
    SJobEntry* job = j != m_jobs.end() ? &(*j).second : GUCEF_NULL;

    bool sensitive = GUCEF_NULL != job && 0 != ( job->definition.GetFlags() & JOBFLAG_SENSITIVE );
    run.record.state = state;
    run.record.endedUtcMs = nowUtcMs;
    // the task's status extra info is the note of the end; without one the note of the last progress report stays
    if ( !note.IsNULLOrEmpty() || 0 == run.lastProgressEventMonotonicMs )
        run.record.note = TruncateText( sensitive ? CRedactor::ForDisplay().RedactText( note ) : note, SJobRunRecord::MaxNoteBytes );
    run.record.outcomeMessage = TruncateText( sensitive ? CRedactor::ForDisplay().RedactText( message ) : message, SJobRunRecord::MaxOutcomeMessageBytes );

    // the lane and its throttle policy
    if ( run.taskQueued )
    {
        SLaneEntry* lane = FindLane( run.laneName );
        if ( GUCEF_NULL != lane )
        {
            if ( lane->inFlight > 0 )
                --lane->inFlight;
            CIJobThrottlePolicy* policy = lane->definition.GetThrottlePolicy();
            if ( GUCEF_NULL != policy )
            {
                SJobPermitRequest request;
                request.laneName = run.laneName;
                request.jobId = run.record.jobId;
                request.itemKey = run.record.itemKey;
                request.cause = run.record.cause;
                request.priority = run.priority;
                request.estimatedCost = run.estimatedCost;
                policy->OnFinished( request, run.record.actualCost, state, 0, nowUtcMs );
            }
            TouchLane( *lane, deferred );
        }
    }

    switch ( state )
    {
        case JOBRUNSTATE_SUCCEEDED:     { ++m_totalRunsSucceeded; break; }
        case JOBRUNSTATE_FAILED:        { ++m_totalRunsFailed; break; }
        case JOBRUNSTATE_CANCELLED:     { ++m_totalRunsCancelled; break; }
        case JOBRUNSTATE_TIMED_OUT:     { ++m_totalRunsTimedOut; break; }
        case JOBRUNSTATE_SKIPPED:       { ++m_totalRunsSkipped; break; }
        default:                        { break; }
    }

    if ( GUCEF_NULL != job )
    {
        if ( run.taskQueued )
        {
            if ( job->runningRunCount > 0 )
                --job->runningRunCount;
        }
        else
        if ( job->pendingRunCount > 0 )
        {
            --job->pendingRunCount;
        }

        if ( JOBRUNSTATE_CANCELLED != state && JOBRUNSTATE_SKIPPED != state )
        {
            ++job->runCount;
            if ( JOBRUNSTATE_SUCCEEDED == state )
                job->consecutiveFailures = 0;
            else
            {
                ++job->failureCount;
                ++job->consecutiveFailures;
            }
        }
        job->lastRunId = runId;
        job->lastRunState = state;
        job->lastRunEndedUtcMs = nowUtcMs;

        // the next due instant of an interval measured from the end of a run
        const CJobTrigger& trigger = job->definition.GetTrigger();
        if ( JOBTRIGGER_INTERVAL == trigger.GetKind() && JOBINTERVALFROM_END == trigger.GetIntervalFrom() &&
             0 == job->runningRunCount && 0 == job->pendingRunCount )
        {
            job->nextDueMonotonicMs = nowMonotonicMs + trigger.GetPeriodMs();
            job->nextDueUtcMs = nowUtcMs + (Int64) trigger.GetPeriodMs();
        }

        // a retry of a run that failed or timed out
        const CJobRetryPolicy& retry = job->definition.GetRetryPolicy();
        bool hostRunning = JOBSCHEDULERSTATE_RUNNING == m_hostState || JOBSCHEDULERSTATE_STARTING == m_hostState;
        if ( ( JOBRUNSTATE_FAILED == state || JOBRUNSTATE_TIMED_OUT == state ) && hostRunning && job->enabled &&
             run.record.attempt < retry.GetMaxAttempts() )
        {
            UInt64 backoffMs = retry.GetBackoffInitialMs();
            for ( UInt32 a=1; a<run.record.attempt && backoffMs < retry.GetBackoffMaxMs(); ++a )
                backoffMs *= 2;
            if ( 0 != retry.GetBackoffMaxMs() && backoffMs > retry.GetBackoffMaxMs() )
                backoffMs = retry.GetBackoffMaxMs();
            UInt32 jitterPercent = retry.GetJitterPercent() > 50 ? 50 : retry.GetJitterPercent();
            if ( jitterPercent > 0 && backoffMs > 0 )
            {
                UInt64 spread = backoffMs * jitterPercent / 100;
                UInt64 offset = (UInt64) CRandom::GetRandomUInt32( 0, (UInt32) ( 2 * spread > 0xFFFFFFFF ? 0xFFFFFFFF : 2 * spread ) );
                backoffMs = backoffMs - spread + offset;
            }

            const SJobWorkItem* item = run.hasWorkItem ? &run.workItem : GUCEF_NULL;
            UInt64 retryRunId = QueueRun( *job, JOBRUNCAUSE_RETRY, run.record.scheduledUtcMs, run.priority, run.record.requestedBy, item, deferred, false );
            TRunMap::iterator retryRun = m_activeRuns.find( retryRunId );
            if ( retryRun != m_activeRuns.end() )
            {
                (*retryRun).second.record.attempt = run.record.attempt + 1;
                (*retryRun).second.notBeforeMonotonicMs = nowMonotonicMs + backoffMs;
                (*retryRun).second.laneName = run.laneName;
                if ( GUCEF_NULL != run.future )
                {
                    // the future of run-now follows the retries: completed by the last attempt
                    (*retryRun).second.future = run.future;
                    run.future = GUCEF_NULL;
                }
            }
        }
        MarkStateDirty( nowMonotonicMs );
        TouchJob( *job, deferred );
    }

    TouchRun( run.record, deferred );
    m_history.Add( run.record );
    if ( GUCEF_NULL != job )
        job->recentRuns.Add( run.record );

    if ( run.hasWorkItem && GUCEF_NULL != job && GUCEF_NULL != job->workSource )
    {
        SItemFinished finished;
        finished.source = job->workSource;
        finished.item = run.workItem;
        finished.run = run.record;
        deferred.itemsFinished.push_back( finished );
    }
    if ( GUCEF_NULL != run.future )
    {
        SFutureCompletion completion;
        completion.future = run.future;
        completion.run = run.record;
        deferred.futures.push_back( completion );
        run.future = GUCEF_NULL;
    }

    m_activeRuns.erase( r );
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::ProcessEndedTasks( Int64 nowUtcMs, UInt64 nowMonotonicMs, SDeferred& deferred )
{GUCEF_TRACE;

    TEndedTaskVector ended;
    ended.swap( m_endedTasks );
    for ( TEndedTaskVector::iterator e = ended.begin(); e != ended.end(); ++e )
    {
        TRunMap::iterator r = m_activeRuns.find( (*e).runId );
        if ( r == m_activeRuns.end() )
            continue;
        SRunEntry& run = (*r).second;
        if ( 0 == run.record.taskId )
            run.record.taskId = (*e).taskId;
        TJobRunState state = RunStateFromTaskStatus( (*e).status, run.timeoutRequested );
        CString message;
        if ( JOBRUNSTATE_FAILED == state )
            message = (*e).statusExtra.IsNULLOrEmpty() ? CString( "the task ended with " ) + TaskStatusToTaskStatusString( (*e).status ) : (*e).statusExtra;
        else
        if ( JOBRUNSTATE_TIMED_OUT == state )
            message = "stopped after the timeout of " + ToString( run.timeoutMs ) + " ms";
        EndRun( (*e).runId, state, (*e).statusExtra, message, nowUtcMs, nowMonotonicMs, deferred );
    }
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::CheckTimeouts( UInt64 nowMonotonicMs, SDeferred& deferred )
{GUCEF_TRACE;

    for ( TRunMap::iterator r = m_activeRuns.begin(); r != m_activeRuns.end(); ++r )
    {
        SRunEntry& run = (*r).second;
        if ( !run.taskQueued || 0 == run.timeoutMs || run.timeoutRequested )
            continue;
        if ( nowMonotonicMs < run.startedMonotonicMs + run.timeoutMs )
            continue;

        // cooperative: the task's cancel flag; it ends as TIMED_OUT
        run.timeoutRequested = true;
        run.cancelRequested = true;
        if ( 0 != run.record.taskId && !run.cancelSent )
        {
            run.cancelSent = true;
            STaskAction action;
            action.threadPoolName = run.record.threadPoolName;
            action.taskId = run.record.taskId;
            deferred.cancellations.push_back( action );
        }
    }
}

/*-------------------------------------------------------------------------*/

Int64
CJobScheduler::ComputeNextWakeDelayMs( Int64 nowUtcMs, UInt64 nowMonotonicMs ) const
{GUCEF_TRACE;

    // the earliest of everything that can make the next cycle do something; at most MaxSleepMs
    UInt64 maxSleepMs = m_settings.GetMaxSleepMs();
    Int64 delayMs = 0 == maxSleepMs ? 0x7FFFFFFF : (Int64) maxSleepMs;

    if ( !m_endedTasks.empty() )
        return 0;

    for ( TJobMap::const_iterator j = m_jobs.begin(); j != m_jobs.end(); ++j )
    {
        const SJobEntry& job = (*j).second;
        if ( !job.enabled )
            continue;
        if ( !job.planned )
            return 0;
        switch ( job.definition.GetTrigger().GetKind() )
        {
            case JOBTRIGGER_SCHEDULE:
            case JOBTRIGGER_ONCE_AT:
            {
                if ( 0 != job.nextDueUtcMs && !job.ended )
                {
                    Int64 d = job.nextDueUtcMs - nowUtcMs;
                    if ( d < delayMs )
                        delayMs = d;
                }
                break;
            }
            case JOBTRIGGER_INTERVAL:
            {
                if ( 0 != job.nextDueMonotonicMs )
                {
                    Int64 d = job.nextDueMonotonicMs > nowMonotonicMs ? (Int64) ( job.nextDueMonotonicMs - nowMonotonicMs ) : 0;
                    if ( d < delayMs )
                        delayMs = d;
                }
                break;
            }
            case JOBTRIGGER_WORK_SOURCE:
            {
                if ( 0 != job.sourceNextWakeUtcMs )
                {
                    Int64 d = job.sourceNextWakeUtcMs - nowUtcMs;
                    if ( d < delayMs )
                        delayMs = d;
                }
                break;
            }
            default:
                break;
        }
        if ( 0 != job.waitingUntilUtcMs )
        {
            Int64 d = job.waitingUntilUtcMs - nowUtcMs;
            if ( d < delayMs )
                delayMs = d;
        }
    }

    for ( TRunMap::const_iterator r = m_activeRuns.begin(); r != m_activeRuns.end(); ++r )
    {
        const SRunEntry& run = (*r).second;
        if ( !run.taskQueued && 0 != run.notBeforeMonotonicMs )
        {
            Int64 d = run.notBeforeMonotonicMs > nowMonotonicMs ? (Int64) ( run.notBeforeMonotonicMs - nowMonotonicMs ) : 0;
            if ( d < delayMs )
                delayMs = d;
        }
        if ( run.taskQueued && 0 != run.timeoutMs && !run.timeoutRequested )
        {
            UInt64 at = run.startedMonotonicMs + run.timeoutMs;
            Int64 d = at > nowMonotonicMs ? (Int64) ( at - nowMonotonicMs ) : 0;
            if ( d < delayMs )
                delayMs = d;
        }
    }

    for ( TLaneMap::const_iterator l = m_lanes.begin(); l != m_lanes.end(); ++l )
    {
        if ( 0 != (*l).second.waitingUntilUtcMs )
        {
            Int64 d = (*l).second.waitingUntilUtcMs - nowUtcMs;
            if ( d < delayMs )
                delayMs = d;
        }
    }

    if ( m_settings.GetStartDelayMs() > 0 && nowMonotonicMs < m_startMonotonicMs + m_settings.GetStartDelayMs() && !m_activeRuns.empty() )
    {
        Int64 d = (Int64) ( m_startMonotonicMs + m_settings.GetStartDelayMs() - nowMonotonicMs );
        if ( d < delayMs )
            delayMs = d;
    }

    if ( m_stateDirty && GUCEF_NULL != m_stateStore )
    {
        Int64 d = m_stateSaveDueMonotonicMs > nowMonotonicMs ? (Int64) ( m_stateSaveDueMonotonicMs - nowMonotonicMs ) : 0;
        if ( d < delayMs )
            delayMs = d;
    }

    return delayMs < 0 ? 0 : delayMs;
}

/*-------------------------------------------------------------------------*/

Int64
CJobScheduler::DispatchCycle( void )
{GUCEF_TRACE;

    // the state is loaded by the first cycle, on the dispatcher's thread (no I/O on the thread of Start())
    bool firstCycle = false;
    {
        MT::CScopeMutex lock( m_lock );
        if ( m_stopRequested )
            return -1;
        firstCycle = !m_stateLoaded;
        ++m_cycleCount;
        m_dispatcherSystemThreadId = (UInt64) MT::GetCurrentThreadSystemID();
    }
    if ( firstCycle )
        LoadStateOnDispatcher();

    SDeferred deferred;
    TStartOrderVector orders;
    typedef GUCEF::vector< std::pair< CString, CIJobWorkSource* > > TSourceVector;
    TSourceVector sources;
    Int64 nowUtcMs = 0;
    UInt64 nowMonotonicMs = 0;
    {
        MT::CScopeMutex lock( m_lock );
        nowUtcMs = m_utcClock->GetUtcTimeInMs();
        nowMonotonicMs = m_monotonicClock->GetMonotonicTimeInMs();

        if ( JOBSCHEDULERSTATE_STARTING == m_hostState )
        {
            m_hostState = JOBSCHEDULERSTATE_RUNNING;
            TouchHost( deferred );
        }

        // a jump of the wall clock: elapsed wall time and elapsed monotonic time differ. Due instants are kept against
        // the last handled occurrence, so nothing runs twice when the clock goes back and missed occurrences are handled
        // by the policy when it goes forward; the next wake is planned again from the new time
        Int64 elapsedUtcMs = nowUtcMs - m_lastCycleUtcMs;
        Int64 elapsedMonotonicMs = (Int64) ( nowMonotonicMs - m_lastCycleMonotonicMs );
        Int64 jumpMs = elapsedUtcMs - elapsedMonotonicMs;
        if ( jumpMs < 0 )
            jumpMs = -jumpMs;
        if ( m_cycleCount > 1 && (UInt64) jumpMs > m_settings.GetClockJumpThresholdMs() )
        {
            ++m_clockJumpCount;
            GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "JobScheduler(" << GetName() << "): the wall clock jumped " << ( elapsedUtcMs - elapsedMonotonicMs ) << " ms; every job is planned again" );
            for ( TJobMap::iterator j = m_jobs.begin(); j != m_jobs.end(); ++j )
            {
                SJobEntry& job = (*j).second;
                if ( JOBTRIGGER_SCHEDULE == job.definition.GetTrigger().GetKind() && job.lastOccurrenceUtcMs > nowUtcMs )
                {
                    // back: the occurrences after now up to the last handled one are not repeated
                    const Int64 dueBefore = job.nextDueUtcMs;
                    job.nextDueUtcMs = NextOccurrenceAfter( job.definition.GetTrigger().GetSchedule(), job.lastOccurrenceUtcMs );
                    if ( dueBefore != job.nextDueUtcMs )
                        TouchJob( job, deferred );
                }
            }
        }
        m_lastCycleUtcMs = nowUtcMs;
        m_lastCycleMonotonicMs = nowMonotonicMs;

        ProcessEndedTasks( nowUtcMs, nowMonotonicMs, deferred );
        CheckTimeouts( nowMonotonicMs, deferred );

        bool dispatching = JOBSCHEDULERSTATE_RUNNING == m_hostState;
        if ( dispatching )
        {
            EvaluateDueJobs( nowUtcMs, nowMonotonicMs, deferred );
            for ( TJobMap::iterator j = m_jobs.begin(); j != m_jobs.end(); ++j )
            {
                SJobEntry& job = (*j).second;
                if ( GUCEF_NULL != job.workSource && job.enabled && job.available )
                    sources.push_back( std::pair< CString, CIJobWorkSource* >( (*j).first, job.workSource ) );
            }
        }
    }
    CompleteDeferred( deferred );

    // the work sources, outside the lock (they keep their own leaf lock); their items come back through the sink
    for ( TSourceVector::iterator s = sources.begin(); s != sources.end(); ++s )
    {
        Int64 nextWakeUtcMs = 0;
        CWorkItemSink sink( *this, (*s).first, nowUtcMs, deferred );
        (*s).second->Evaluate( nowUtcMs, nowMonotonicMs, sink, nextWakeUtcMs );
        MT::CScopeMutex lock( m_lock );
        TJobMap::iterator j = m_jobs.find( (*s).first );
        if ( j != m_jobs.end() )
            (*j).second.sourceNextWakeUtcMs = nextWakeUtcMs;
    }
    CompleteDeferred( deferred );

    {
        MT::CScopeMutex lock( m_lock );
        if ( JOBSCHEDULERSTATE_RUNNING == m_hostState )
            SelectRunsToStart( nowUtcMs, nowMonotonicMs, orders, deferred );
    }
    CompleteDeferred( deferred );
    StartOrders( orders );

    // the kept state, debounced on the monotonic clock
    bool saveDue = false;
    {
        MT::CScopeMutex lock( m_lock );
        saveDue = m_stateDirty && GUCEF_NULL != m_stateStore && nowMonotonicMs >= m_stateSaveDueMonotonicMs;
    }
    if ( saveDue )
        SaveStateNow();

    MT::CScopeMutex lock( m_lock );
    if ( m_stopRequested )
        return -1;
    Int64 delayMs = ComputeNextWakeDelayMs( m_utcClock->GetUtcTimeInMs(), m_monotonicClock->GetMonotonicTimeInMs() );
    m_lastPlannedDelayMs = (UInt64) delayMs;
    return delayMs;
}

/*-------------------------------------------------------------------------*/

TJobControlStatus
CJobScheduler::Control( const SJobControlCommand& command, SJobControlResult& result )
{GUCEF_TRACE;

    result = SJobControlResult();
    SDeferred deferred;
    CIJobWorkSource* customSource = GUCEF_NULL;
    CString runNowLane;
    TJobControlStatus status = JOBCONTROLSTATUS_OK;
    {
        MT::CScopeMutex lock( m_lock );
        status = ControlLocked( command, result, deferred, customSource, runNowLane );
        result.revision = m_revision;
    }
    CompleteDeferred( deferred );

    // what is carried out by others, without the lock
    if ( JOBCONTROLSTATUS_OK == status && !command.dryRun )
    {
        if ( JOBCONTROLACTION_CUSTOM == command.action && GUCEF_NULL != customSource )
        {
            CString message;
            status = customSource->ApplyCustomAction( command, result.detail, message );
            if ( !message.IsNULLOrEmpty() )
                result.messages.push_back( message );
        }
        if ( !runNowLane.IsNULLOrEmpty() )
        {
            // the work sources that use the lane clear their own waits
            typedef GUCEF::set< CIJobWorkSource* > TSourceSet;
            TSourceSet sources;
            {
                MT::CScopeMutex lock( m_lock );
                for ( TJobMap::iterator j = m_jobs.begin(); j != m_jobs.end(); ++j )
                    if ( GUCEF_NULL != (*j).second.workSource )
                        sources.insert( (*j).second.workSource );
            }
            Int64 nowUtcMs = m_utcClock->GetUtcTimeInMs();
            for ( TSourceSet::iterator s = sources.begin(); s != sources.end(); ++s )
                (*s)->OnRunNow( runNowLane, nowUtcMs );
        }
        RequestDispatch();
    }
    return status;
}

/*-------------------------------------------------------------------------*/

TJobControlStatus
CJobScheduler::ControlLocked( const SJobControlCommand& command, SJobControlResult& result, SDeferred& deferred,
                              CIJobWorkSource*& customSource, CString& runNowLane )
{GUCEF_TRACE;

    Int64 nowUtcMs = m_utcClock->GetUtcTimeInMs();
    UInt64 nowMonotonicMs = m_monotonicClock->GetMonotonicTimeInMs();

    switch ( command.action )
    {
        case JOBCONTROLACTION_RUN_NOW:
        {
            if ( JOBCONTROLTARGET_LANE == command.targetKind )
            {
                SLaneEntry* lane = FindLane( command.targetId );
                if ( GUCEF_NULL == lane )
                {
                    result.messages.push_back( "no lane " + command.targetId );
                    return JOBCONTROLSTATUS_NOT_FOUND;
                }
                if ( 0 != command.expectedRevision && command.expectedRevision != lane->revision )
                    return JOBCONTROLSTATUS_ABORTED;
                if ( command.dryRun )
                    return JOBCONTROLSTATUS_OK;
                if ( GUCEF_NULL != lane->definition.GetThrottlePolicy() )
                    lane->definition.GetThrottlePolicy()->OnRunNow( nowUtcMs );
                lane->waitingFor.Clear();
                lane->waitingUntilUtcMs = 0;
                TouchLane( *lane, deferred );
                runNowLane = lane->definition.GetName();
                result.messages.push_back( "the waits of lane " + runNowLane + " were cleared" );
                return JOBCONTROLSTATUS_OK;
            }

            TJobMap::iterator j = m_jobs.find( command.targetId );
            if ( JOBCONTROLTARGET_JOB != command.targetKind || j == m_jobs.end() )
            {
                result.messages.push_back( "no job " + command.targetId );
                return JOBCONTROLSTATUS_NOT_FOUND;
            }
            SJobEntry& job = (*j).second;
            if ( 0 != command.expectedRevision && command.expectedRevision != job.revision )
                return JOBCONTROLSTATUS_ABORTED;
            if ( !job.enabled )
            {
                result.messages.push_back( "the job " + command.targetId + " is disabled" );
                return JOBCONTROLSTATUS_FAILED_PRECONDITION;
            }
            if ( JOBSCHEDULERSTATE_DRAINING == m_hostState || JOBSCHEDULERSTATE_STOPPED == m_hostState )
            {
                result.messages.push_back( "the scheduler is " + CString( JobSchedulerHostStateToString( m_hostState ) ) );
                return JOBCONTROLSTATUS_FAILED_PRECONDITION;
            }
            if ( JOBTRIGGER_WORK_SOURCE == job.definition.GetTrigger().GetKind() )
            {
                result.messages.push_back( "the runs of a work source job are its items: run-now of its lane clears the waits" );
                return JOBCONTROLSTATUS_FAILED_PRECONDITION;
            }
            if ( command.hasPriority && command.priority > 3 )
            {
                result.messages.push_back( "a priority is 0 (highest) .. 3" );
                return JOBCONTROLSTATUS_INVALID_ARGUMENT;
            }
            if ( command.dryRun )
                return JOBCONTROLSTATUS_OK;

            // ignores the next due time, not the lane, the throttle or the job's one-at-a-time rule
            UInt32 priority = command.hasPriority ? command.priority : job.definition.GetPriority();
            UInt64 runId = QueueRun( job, JOBRUNCAUSE_RUN_NOW, nowUtcMs, priority, command.requestedBy, GUCEF_NULL, deferred, false );
            result.runId = runId;
            result.hasRun = true;
            result.run = m_activeRuns[ runId ].record;
            result.messages.push_back( "run " + ToString( runId ) + " of job " + command.targetId + " queued" );
            return JOBCONTROLSTATUS_OK;
        }
        case JOBCONTROLACTION_CANCEL_RUN:
        {
            TUInt64Vector runIds;
            if ( JOBCONTROLTARGET_RUN == command.targetKind )
            {
                UInt64 runId = StringToUInt64( command.targetId, 0 );
                TRunMap::iterator r = m_activeRuns.find( runId );
                if ( r == m_activeRuns.end() )
                {
                    SJobRunRecord ended;
                    if ( m_history.Find( runId, ended ) )
                    {
                        result.hasRun = true;
                        result.run = ended;
                        result.messages.push_back( "run " + command.targetId + " has ended already" );
                        return JOBCONTROLSTATUS_FAILED_PRECONDITION;
                    }
                    result.messages.push_back( "no run " + command.targetId );
                    return JOBCONTROLSTATUS_NOT_FOUND;
                }
                if ( 0 != command.expectedRevision && command.expectedRevision != (*r).second.record.revision )
                    return JOBCONTROLSTATUS_ABORTED;
                runIds.push_back( runId );
            }
            else
            if ( JOBCONTROLTARGET_JOB == command.targetKind )
            {
                if ( m_jobs.find( command.targetId ) == m_jobs.end() )
                {
                    result.messages.push_back( "no job " + command.targetId );
                    return JOBCONTROLSTATUS_NOT_FOUND;
                }
                for ( TRunMap::iterator r = m_activeRuns.begin(); r != m_activeRuns.end(); ++r )
                    if ( (*r).second.record.jobId == command.targetId )
                        runIds.push_back( (*r).first );
            }
            else
            {
                return JOBCONTROLSTATUS_INVALID_ARGUMENT;
            }
            if ( command.dryRun )
                return JOBCONTROLSTATUS_OK;

            for ( TUInt64Vector::iterator i = runIds.begin(); i != runIds.end(); ++i )
            {
                SRunEntry& run = m_activeRuns[ (*i) ];
                if ( !run.taskQueued )
                {
                    // never started: ends here
                    SJobRunRecord record = run.record;
                    EndRun( (*i), JOBRUNSTATE_CANCELLED, CString::Empty, "cancelled" + ( command.requestedBy.IsNULLOrEmpty() ? CString::Empty : CString( " by " + command.requestedBy ) ), nowUtcMs, nowMonotonicMs, deferred );
                    result.hasRun = true;
                    m_history.Find( (*i), result.run );
                }
                else
                {
                    // cooperative: the task's cancel flag through the pool (outside the lock); the run ends when the task does
                    run.cancelRequested = true;
                    if ( 0 != run.record.taskId && !run.cancelSent )
                    {
                        run.cancelSent = true;
                        STaskAction action;
                        action.threadPoolName = run.record.threadPoolName;
                        action.taskId = run.record.taskId;
                        deferred.cancellations.push_back( action );
                    }
                    result.hasRun = true;
                    result.run = run.record;
                }
            }
            result.messages.push_back( ToString( (UInt32) runIds.size() ) + " runs asked to cancel" );
            return JOBCONTROLSTATUS_OK;
        }
        case JOBCONTROLACTION_ENABLE:
        case JOBCONTROLACTION_DISABLE:
        case JOBCONTROLACTION_SET_PRIORITY:
        case JOBCONTROLACTION_REMOVE_JOB:
        {
            TJobMap::iterator j = m_jobs.find( command.targetId );
            if ( JOBCONTROLTARGET_JOB != command.targetKind || j == m_jobs.end() )
            {
                result.messages.push_back( "no job " + command.targetId );
                return JOBCONTROLSTATUS_NOT_FOUND;
            }
            SJobEntry& job = (*j).second;
            if ( 0 != command.expectedRevision && command.expectedRevision != job.revision )
                return JOBCONTROLSTATUS_ABORTED;

            if ( JOBCONTROLACTION_SET_PRIORITY == command.action )
            {
                if ( !command.hasPriority || command.priority > 3 )
                {
                    result.messages.push_back( "a priority is 0 (highest) .. 3" );
                    return JOBCONTROLSTATUS_INVALID_ARGUMENT;
                }
                if ( command.dryRun )
                    return JOBCONTROLSTATUS_OK;
                job.definition.SetPriority( command.priority );
                job.priorityOverridden = true;
                for ( TRunMap::iterator r = m_activeRuns.begin(); r != m_activeRuns.end(); ++r )
                    if ( (*r).second.record.jobId == job.definition.GetId() && !(*r).second.taskQueued )
                        (*r).second.priority = command.priority;
                MarkStateDirty( nowMonotonicMs );
                TouchJob( job, deferred );
                result.messages.push_back( "the priority of job " + command.targetId + " is " + ToString( command.priority ) );
                return JOBCONTROLSTATUS_OK;
            }

            if ( JOBCONTROLACTION_REMOVE_JOB == command.action )
            {
                if ( JOBORIGIN_SUBMITTED != job.definition.GetOrigin() )
                {
                    result.messages.push_back( "the job " + command.targetId + " is defined by the service and cannot be removed" );
                    return JOBCONTROLSTATUS_FAILED_PRECONDITION;
                }
                if ( command.dryRun )
                    return JOBCONTROLSTATUS_OK;
                TUInt64Vector runIds;
                for ( TRunMap::iterator r = m_activeRuns.begin(); r != m_activeRuns.end(); ++r )
                    if ( (*r).second.record.jobId == command.targetId )
                        runIds.push_back( (*r).first );
                for ( TUInt64Vector::iterator i = runIds.begin(); i != runIds.end(); ++i )
                {
                    SRunEntry& run = m_activeRuns[ (*i) ];
                    if ( !run.taskQueued )
                        EndRun( (*i), JOBRUNSTATE_CANCELLED, CString::Empty, "the job was removed", nowUtcMs, nowMonotonicMs, deferred );
                    else
                    if ( 0 != run.record.taskId && !run.cancelSent )
                    {
                        run.cancelRequested = true;
                        run.cancelSent = true;
                        STaskAction action;
                        action.threadPoolName = run.record.threadPoolName;
                        action.taskId = run.record.taskId;
                        deferred.cancellations.push_back( action );
                    }
                }
                if ( m_submittedJobCount > 0 )
                    --m_submittedJobCount;
                m_jobs.erase( j );
                NextRevision();
                SJobSchedulerChange change;
                change.kind = JOBSCHEDULERCHANGE_JOB;
                change.key = command.targetId;
                change.revision = m_revision;
                QueueChange( change, deferred );
                MarkStateDirty( nowMonotonicMs );
                result.messages.push_back( "job " + command.targetId + " removed" );
                return JOBCONTROLSTATUS_OK;
            }

            bool enable = JOBCONTROLACTION_ENABLE == command.action;
            if ( command.dryRun )
                return JOBCONTROLSTATUS_OK;
            if ( job.enabled != enable )
            {
                job.enabled = enable;
                if ( enable )
                {
                    // occurrences while disabled are not missed ones: the schedule continues from now
                    if ( JOBTRIGGER_SCHEDULE == job.definition.GetTrigger().GetKind() )
                        job.lastOccurrenceUtcMs = nowUtcMs;
                    job.planned = false;
                }
                else
                {
                    // the runs that did not start yet will not
                    TUInt64Vector runIds;
                    for ( TRunMap::iterator r = m_activeRuns.begin(); r != m_activeRuns.end(); ++r )
                        if ( (*r).second.record.jobId == command.targetId && !(*r).second.taskQueued )
                            runIds.push_back( (*r).first );
                    for ( TUInt64Vector::iterator i = runIds.begin(); i != runIds.end(); ++i )
                        EndRun( (*i), JOBRUNSTATE_SKIPPED, CString::Empty, "the job was disabled", nowUtcMs, nowMonotonicMs, deferred );
                }
                MarkStateDirty( nowMonotonicMs );
                TouchJob( job, deferred );
            }
            result.messages.push_back( "job " + command.targetId + ( enable ? " enabled" : " disabled" ) );
            return JOBCONTROLSTATUS_OK;
        }
        case JOBCONTROLACTION_SET_LANE_MODE:
        {
            SLaneEntry* lane = FindLane( command.targetId );
            if ( JOBCONTROLTARGET_LANE != command.targetKind || GUCEF_NULL == lane )
            {
                result.messages.push_back( "no lane " + command.targetId );
                return JOBCONTROLSTATUS_NOT_FOUND;
            }
            if ( 0 != command.expectedRevision && command.expectedRevision != lane->revision )
                return JOBCONTROLSTATUS_ABORTED;
            const CString::StringVector& modes = lane->definition.GetDeclaredModes();
            bool known = command.mode.Equals( CJobLaneDefinition::DisabledModeName, false ) || ( modes.empty() && !command.mode.IsNULLOrEmpty() );
            for ( CString::StringVector::const_iterator m = modes.begin(); m != modes.end() && !known; ++m )
                known = (*m) == command.mode;
            if ( !known )
            {
                result.messages.push_back( "the lane " + command.targetId + " has no mode " + command.mode );
                return JOBCONTROLSTATUS_INVALID_ARGUMENT;
            }
            if ( command.dryRun )
                return JOBCONTROLSTATUS_OK;
            lane->mode = command.mode;
            lane->modeOverridden = true;
            MarkStateDirty( nowMonotonicMs );
            TouchLane( *lane, deferred );
            result.messages.push_back( "the mode of lane " + command.targetId + " is " + command.mode );
            return JOBCONTROLSTATUS_OK;
        }
        case JOBCONTROLACTION_CUSTOM:
        {
            if ( JOBCONTROLTARGET_JOB == command.targetKind )
            {
                TJobMap::iterator j = m_jobs.find( command.targetId );
                if ( j == m_jobs.end() )
                {
                    result.messages.push_back( "no job " + command.targetId );
                    return JOBCONTROLSTATUS_NOT_FOUND;
                }
                if ( GUCEF_NULL != (*j).second.workSource )
                {
                    // the source checks its own declared actions; applied outside the lock
                    customSource = (*j).second.workSource;
                    return JOBCONTROLSTATUS_OK;
                }
            }
            result.messages.push_back( "nobody here declares the custom action " + command.customName );
            return JOBCONTROLSTATUS_UNIMPLEMENTED;
        }
        default:
        {
            result.messages.push_back( "the action " + CString( JobControlActionToString( command.action ) ) + " is not available" );
            return JOBCONTROLSTATUS_UNIMPLEMENTED;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CJobScheduler::TryHoldLane( const CString& laneName, const CString& holder, CString& refusal )
{GUCEF_TRACE;

    SDeferred deferred;
    {
        MT::CScopeMutex lock( m_lock );
        SLaneEntry* lane = FindLane( laneName );
        if ( GUCEF_NULL == lane )
        {
            refusal = "no lane " + laneName;
            return false;
        }
        if ( !lane->heldBy.IsNULLOrEmpty() )
        {
            refusal = "the lane is held by " + lane->heldBy;
            return false;
        }
        if ( lane->inFlight > 0 )
        {
            refusal = ToString( lane->inFlight ) + " runs of the lane are in flight";
            return false;
        }
        lane->heldBy = holder.IsNULLOrEmpty() ? CString( "unnamed" ) : holder;
        lane->heldSinceUtcMs = m_utcClock->GetUtcTimeInMs();
        TouchLane( *lane, deferred );
    }
    CompleteDeferred( deferred );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::ReleaseLaneHold( const CString& laneName, UInt32 actualCost )
{GUCEF_TRACE;

    SDeferred deferred;
    {
        MT::CScopeMutex lock( m_lock );
        SLaneEntry* lane = FindLane( laneName );
        if ( GUCEF_NULL == lane || lane->heldBy.IsNULLOrEmpty() )
            return;
        CIJobThrottlePolicy* policy = lane->definition.GetThrottlePolicy();
        if ( GUCEF_NULL != policy )
        {
            SJobPermitRequest request;
            request.laneName = laneName;
            request.cause = JOBRUNCAUSE_HOLD;
            request.estimatedCost = actualCost;
            policy->OnFinished( request, actualCost, JOBRUNSTATE_SUCCEEDED, 0, m_utcClock->GetUtcTimeInMs() );
        }
        lane->heldBy.Clear();
        lane->heldSinceUtcMs = 0;
        TouchLane( *lane, deferred );
        deferred.wakeDispatcher = true;
    }
    CompleteDeferred( deferred );
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::FillJobStatus( const SJobEntry& job, SJobStatus& status ) const
{GUCEF_TRACE;

    const CJobDefinition& definition = job.definition;
    status.jobId = definition.GetId();
    status.title = definition.GetTitle().IsNULLOrEmpty() ? definition.GetId() : definition.GetTitle();
    status.description = definition.GetDescription();
    status.taskTypeName = definition.GetTaskTypeName();
    status.laneName = EffectiveLaneName( definition );
    status.state = ComputeJobState( job );
    status.priority = definition.GetPriority();
    status.triggerKind = definition.GetTrigger().GetKind();
    status.triggerText = definition.GetTrigger().GetDescription();
    status.executionTarget = definition.GetExecutionTarget();
    status.threadPoolName = EffectivePoolName( definition );
    status.origin = definition.GetOrigin();
    status.submittedBy = definition.GetSubmittedBy();
    status.nextDueUtcMs = job.enabled && !job.ended ? job.nextDueUtcMs : 0;
    status.waitingFor = job.waitingFor;
    status.waitingUntilUtcMs = job.waitingUntilUtcMs;
    status.unavailableReason = job.unavailableReason;
    if ( status.unavailableReason.IsNULLOrEmpty() && JOBSTATE_UNAVAILABLE == status.state )
    {
        TLaneMap::const_iterator lane = m_lanes.find( status.laneName );
        if ( lane != m_lanes.end() )
            status.unavailableReason = (*lane).second.unavailableReason;
    }
    status.lastRunId = job.lastRunId;
    status.lastRunState = job.lastRunState;
    status.lastRunEndedUtcMs = job.lastRunEndedUtcMs;
    status.runCount = job.runCount;
    status.failureCount = job.failureCount;
    status.consecutiveFailures = job.consecutiveFailures;
    status.activeRunCount = job.pendingRunCount + job.runningRunCount;
    status.flags = definition.GetFlags();
    status.enabled = job.enabled;
    status.hasWorkSource = GUCEF_NULL != job.workSource;
    status.remoteActions = definition.GetRemoteActions();
    status.remoteCustomActions = definition.GetRemoteCustomActions();
    status.customActions.clear();
    if ( 0 != ( definition.GetFlags() & JOBFLAG_EXPOSE_PAYLOAD ) )
        status.payload = definition.GetPayload();
    status.revision = job.revision;
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::FillLaneStatus( const SLaneEntry& lane, SJobLaneStatus& status, Int64 nowUtcMs ) const
{GUCEF_TRACE;

    status.laneName = lane.definition.GetName();
    status.mode = lane.mode;
    status.declaredModes = lane.definition.GetDeclaredModes();
    status.inFlight = lane.inFlight;
    status.maxConcurrent = lane.definition.GetMaxConcurrent();
    status.queuedByPriority.assign( 4, 0 );
    for ( TRunMap::const_iterator r = m_activeRuns.begin(); r != m_activeRuns.end(); ++r )
    {
        const SRunEntry& run = (*r).second;
        if ( !run.taskQueued && run.laneName == status.laneName )
            ++status.queuedByPriority[ run.priority > 3 ? 3 : run.priority ];
    }
    status.waitingFor = lane.waitingFor;
    status.waitingUntilUtcMs = lane.waitingUntilUtcMs;
    status.available = lane.available;
    status.unavailableReason = lane.unavailableReason;
    status.heldBy = lane.heldBy;
    status.heldSinceUtcMs = lane.heldSinceUtcMs;
    if ( !lane.available )
        status.state = "unavailable";
    else
    if ( lane.mode.Equals( CJobLaneDefinition::DisabledModeName, false ) )
        status.state = "disabled";
    else
    if ( !lane.heldBy.IsNULLOrEmpty() )
        status.state = "held";
    else
    if ( !lane.waitingFor.IsNULLOrEmpty() )
        status.state = "waiting";
    else
        status.state = "available";
    CIJobThrottlePolicy* policy = lane.definition.GetThrottlePolicy();
    if ( GUCEF_NULL != policy )
        policy->GetStatus( status.throttleStatus, nowUtcMs );
    status.revision = lane.revision;
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::GetSnapshot( CJobSchedulerSnapshot& snapshot, UInt64 changedAfterRevision ) const
{GUCEF_TRACE;

    snapshot.Clear();
    MT::CScopeMutex lock( m_lock );

    Int64 nowUtcMs = m_utcClock->GetUtcTimeInMs();
    snapshot.SetHostName( GetName() );
    snapshot.SetHostInstanceId( m_hostInstanceId );
    snapshot.SetRevision( m_revision );
    snapshot.SetChangedAfterRevision( changedAfterRevision );
    snapshot.SetHostState( m_hostState );
    snapshot.SetServerTimeUtcMs( nowUtcMs );
    snapshot.SetTotalRunsStarted( m_totalRunsStarted );
    snapshot.SetTotalRunsSucceeded( m_totalRunsSucceeded );
    snapshot.SetTotalRunsFailed( m_totalRunsFailed );
    snapshot.SetTotalRunsCancelled( m_totalRunsCancelled );
    snapshot.SetTotalRunsTimedOut( m_totalRunsTimedOut );
    snapshot.SetTotalRunsSkipped( m_totalRunsSkipped );
    CString::StringSet submittable;
    for ( TSubmittableTaskTypeMap::const_iterator i = m_submittable.begin(); i != m_submittable.end(); ++i )
        submittable.insert( (*i).first );
    snapshot.SetSubmittableTaskTypes( submittable );

    for ( TJobMap::const_iterator j = m_jobs.begin(); j != m_jobs.end(); ++j )
    {
        if ( (*j).second.revision <= changedAfterRevision )
            continue;
        snapshot.GetJobs().push_back( SJobStatus() );
        FillJobStatus( (*j).second, snapshot.GetJobs().back() );
    }
    for ( TLaneMap::const_iterator l = m_lanes.begin(); l != m_lanes.end(); ++l )
    {
        if ( (*l).second.revision <= changedAfterRevision )
            continue;
        snapshot.GetLanes().push_back( SJobLaneStatus() );
        FillLaneStatus( (*l).second, snapshot.GetLanes().back(), nowUtcMs );
    }

    // active runs first, then the ended ones, newest first
    for ( TRunMap::const_reverse_iterator r = m_activeRuns.rbegin(); r != m_activeRuns.rend(); ++r )
        if ( (*r).second.record.revision > changedAfterRevision )
            snapshot.GetRuns().push_back( (*r).second.record );
    m_history.GetNewestFirst( snapshot.GetRuns(), changedAfterRevision );
}

/*-------------------------------------------------------------------------*/

bool
CJobScheduler::GetRun( UInt64 runId, SJobRunRecord& run ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TRunMap::const_iterator r = m_activeRuns.find( runId );
    if ( r != m_activeRuns.end() )
    {
        run = (*r).second.record;
        return true;
    }
    return m_history.Find( runId, run );
}

/*-------------------------------------------------------------------------*/

UInt64
CJobScheduler::GetRevision( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_revision;
}

/*-------------------------------------------------------------------------*/

UInt64
CJobScheduler::GetHostInstanceId( void ) const
{GUCEF_TRACE;

    return m_hostInstanceId;
}

/*-------------------------------------------------------------------------*/

void
CJobScheduler::GetScheduledJobInfo( TScheduledJobInfoVector& info ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( TJobMap::const_iterator j = m_jobs.begin(); j != m_jobs.end(); ++j )
    {
        const SJobEntry& job = (*j).second;
        info.push_back( CScheduledJobInfo() );
        CScheduledJobInfo& jobInfo = info.back();
        jobInfo.SetSchedulerName( GetName() );
        jobInfo.SetJobId( job.definition.GetId() );
        jobInfo.SetTitle( job.definition.GetTitle().IsNULLOrEmpty() ? job.definition.GetId() : job.definition.GetTitle() );
        jobInfo.SetTaskTypeName( job.definition.GetTaskTypeName() );
        jobInfo.SetLaneName( EffectiveLaneName( job.definition ) );
        jobInfo.SetState( ComputeJobState( job ) );
        jobInfo.SetExecutionTarget( job.definition.GetExecutionTarget() );
        jobInfo.SetThreadPoolName( EffectivePoolName( job.definition ) );
        jobInfo.SetOrigin( job.definition.GetOrigin() );
        jobInfo.SetNextDueUtcMs( job.enabled && !job.ended ? job.nextDueUtcMs : 0 );
        jobInfo.SetActiveRunCount( job.pendingRunCount + job.runningRunCount );
        jobInfo.SetLastRunId( job.lastRunId );
        jobInfo.SetLastRunState( job.lastRunState );
        jobInfo.SetLastRunEndedUtcMs( job.lastRunEndedUtcMs );
        jobInfo.SetFlags( job.definition.GetFlags() );
    }
}

/*-------------------------------------------------------------------------*/

bool
CJobScheduler::GetWorkSourceStatus( const CString& jobId, CDataNode& status, bool includeQueue ) const
{GUCEF_TRACE;

    status.Clear();
    CIJobWorkSource* source = GUCEF_NULL;
    Int64 nowUtcMs = 0;
    {
        MT::CScopeMutex lock( m_lock );
        TJobMap::const_iterator j = m_jobs.find( jobId );
        if ( j == m_jobs.end() || GUCEF_NULL == (*j).second.workSource )
            return false;
        source = (*j).second.workSource;
        nowUtcMs = m_utcClock->GetUtcTimeInMs();
    }
    // outside the lock, as Evaluate(): the source is not owned and outlives its job (AddWorkSourceJob())
    source->GetStatus( status, nowUtcMs, includeQueue );
    return true;
}

/*-------------------------------------------------------------------------*/

CFutureResult
CJobScheduler::RunNowAsync( const CString& jobId, const CString& requestedBy )
{GUCEF_TRACE;

    SJobControlCommand command;
    command.action = JOBCONTROLACTION_RUN_NOW;
    command.targetKind = JOBCONTROLTARGET_JOB;
    command.targetId = jobId;
    command.requestedBy = requestedBy;

    CFutureResultSource source( TASKSTATUS_QUEUED );
    SJobControlResult result;
    SDeferred deferred;
    TJobControlStatus status = JOBCONTROLSTATUS_OK;
    {
        MT::CScopeMutex lock( m_lock );
        CIJobWorkSource* customSource = GUCEF_NULL;
        CString runNowLane;
        status = ControlLocked( command, result, deferred, customSource, runNowLane );
        if ( JOBCONTROLSTATUS_OK == status && result.hasRun )
        {
            TRunMap::iterator r = m_activeRuns.find( result.runId );
            if ( r != m_activeRuns.end() )
                (*r).second.future = GUCEF_NEW CFutureResultSource( source );
        }
    }
    CompleteDeferred( deferred );

    if ( JOBCONTROLSTATUS_OK != status )
    {
        CString reason = result.messages.empty() ? CString( JobControlStatusToString( status ) ) : result.messages.front();
        source.Fail( JOBCONTROLSTATUS_NOT_FOUND == status ? TASKSTATUS_TASKTYPE_INVALID : TASKSTATUS_SETUP_FAILED, reason );
        return source.GetFuture();
    }
    RequestDispatch();
    return source.GetFuture();
}

/*-------------------------------------------------------------------------*/

const CString&
CJobScheduler::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
