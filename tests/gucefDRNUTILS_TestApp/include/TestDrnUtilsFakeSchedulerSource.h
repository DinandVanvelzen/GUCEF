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

#ifndef GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSFAKESCHEDULERSOURCE_H
#define GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSFAKESCHEDULERSOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_DRNUTILS_CISCHEDULERSOURCE_H
#include "gucefDRNUTILS_CISchedulerSource.h"
#define GUCEF_DRNUTILS_CISCHEDULERSOURCE_H
#endif /* GUCEF_DRNUTILS_CISCHEDULERSOURCE_H ? */

#ifndef GUCEF_DRNUTILS_CISCHEDULERCUSTOMACTIONHANDLER_H
#include "gucefDRNUTILS_CISchedulerCustomActionHandler.h"
#define GUCEF_DRNUTILS_CISCHEDULERCUSTOMACTIONHANDLER_H
#endif /* GUCEF_DRNUTILS_CISCHEDULERCUSTOMACTIONHANDLER_H ? */

#ifndef GUCEF_DRNUTILS_CSCHEDULERHOST_H
#include "gucefDRNUTILS_CSchedulerHost.h"
#define GUCEF_DRNUTILS_CSCHEDULERHOST_H
#endif /* GUCEF_DRNUTILS_CSCHEDULERHOST_H ? */

namespace DRNUTILSTEST {

using namespace GUCEF;

/** Raises CISchedulerSource::ChangedEvent on the thread of its pulse generator */
class CFakeChangeNotifier : public CORE::CTSGNotifier
{
    public:
    explicit CFakeChangeNotifier( const CORE::PulseGeneratorPtr& pulseGenerator ) : CORE::CTSGNotifier( pulseGenerator, true, false ) {}
    virtual ~CFakeChangeNotifier() { SignalUpcomingDestruction(); }
    void Raise( UInt64 revision )
    {
        DRNUTILS::CISchedulerSource::TChangedEventData data( revision );
        NotifyObservers( DRNUTILS::CISchedulerSource::ChangedEvent, &data );
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  A scheduler as data, for the remote layer's tests (stream A's engine is not needed): jobs, lanes and runs with
 *  revisions, Apply() and Submit() recorded and carried out on the model, and probes for the lock rules (a probe that
 *  asks the host from another thread while Apply() or GetSnapshot() runs)
 */
class CFakeSchedulerSource : public DRNUTILS::CISchedulerSource
{
    public:

    CORE::CString hostName;
    UInt64 instanceId;
    UInt64 revision;
    CORE::TJobSchedulerHostState hostState;
    CORE::TJobStatusVector jobs;
    CORE::TJobLaneStatusVector lanes;
    CORE::TJobRunRecordVector runs;             /**< in creation order */
    CORE::CString::StringSet submittable;
    DRNUTILS::TSchedulerActionInfoVector declaredCustom;
    GUCEF::vector< CORE::SJobControlCommand > applied;   /**< the last 256 */
    GUCEF::vector< CORE::SJobSubmission > submitted;
    UInt32 snapshotCalls;
    UInt32 applyCalls;
    UInt64 nextRunId;
    bool raiseOnApply;                          /**< raise ChangedEvent from inside Apply() (the caller's thread) */
    DRNUTILS::CSchedulerHost* probeHost;        /**< asked from another thread while Apply() runs */
    UInt32 probeOk;
    UInt32 probeFailed;
    mutable MT::CMutex lock;
    CFakeChangeNotifier notifier;

    explicit CFakeSchedulerSource( const CORE::PulseGeneratorPtr& pulseGenerator, const CORE::CString& name = "node1" )
        : DRNUTILS::CISchedulerSource(), hostName( name ), instanceId( 1001 ), revision( 1 ), hostState( CORE::JOBSCHEDULERSTATE_RUNNING )
        , jobs(), lanes(), runs(), submittable(), declaredCustom(), applied(), submitted(), snapshotCalls( 0 ), applyCalls( 0 ), nextRunId( 1 )
        , raiseOnApply( false ), probeHost( GUCEF_NULL ), probeOk( 0 ), probeFailed( 0 ), lock(), notifier( pulseGenerator )
    {}

    virtual ~CFakeSchedulerSource() {}

    /** A job that allows every generic remote action */
    CORE::SJobStatus& AddJob( const CORE::CString& jobId, const CORE::CString& lane = "lane1", UInt32 flags = 0 )
    {
        MT::CScopeMutex scope( lock );
        CORE::SJobStatus job;
        job.jobId = jobId;
        job.title = "Job " + jobId;
        job.laneName = lane;
        job.taskTypeName = "test.task";
        job.state = CORE::JOBSTATE_SCHEDULED;
        job.triggerText = "every 60 s";
        job.executionTarget = CORE::JOBEXECUTIONTARGET_POOL;
        job.threadPoolName = "gucef.scheduler.run";
        job.origin = CORE::JOBORIGIN_CODE;
        job.flags = flags;
        job.enabled = true;
        for ( Int32 a=CORE::JOBCONTROLACTION_RUN_NOW; a<=CORE::JOBCONTROLACTION_SET_PRIORITY; ++a )
            job.remoteActions.insert( a );
        job.revision = ++revision;
        jobs.push_back( job );
        return jobs.back();
    }

    CORE::SJobLaneStatus& AddLane( const CORE::CString& name )
    {
        MT::CScopeMutex scope( lock );
        CORE::SJobLaneStatus lane;
        lane.laneName = name;
        lane.mode = "scheduled";
        lane.state = "available";
        lane.maxConcurrent = 2;
        lane.available = true;
        lane.queuedByPriority.resize( 4, 0 );
        lane.revision = ++revision;
        lanes.push_back( lane );
        return lanes.back();
    }

    CORE::SJobStatus* FindJob( const CORE::CString& jobId )
    {
        for ( UInt32 i=0; i<jobs.size(); ++i )
            if ( jobs[ i ].jobId == jobId )
                return &jobs[ i ];
        return GUCEF_NULL;
    }

    CORE::SJobRunRecord* FindRun( UInt64 runId )
    {
        for ( UInt32 i=0; i<runs.size(); ++i )
            if ( runs[ i ].runId == runId )
                return &runs[ i ];
        return GUCEF_NULL;
    }

    /** A change of the job (title changes), raised */
    void Touch( const CORE::CString& jobId, bool raise = true )
    {
        {
            MT::CScopeMutex scope( lock );
            CORE::SJobStatus* job = FindJob( jobId );
            if ( GUCEF_NULL == job )
                return;
            job->revision = ++revision;
            job->runCount += 1;
        }
        if ( raise )
            RaiseChanged();
    }

    void RemoveJob( const CORE::CString& jobId )
    {
        {
            MT::CScopeMutex scope( lock );
            for ( UInt32 i=0; i<jobs.size(); ++i )
                if ( jobs[ i ].jobId == jobId )
                {
                    jobs.erase( jobs.begin() + i );
                    break;
                }
            ++revision;
        }
        RaiseChanged();
    }

    UInt64 StartRun( const CORE::CString& jobId, const CORE::CString& requestedBy, const CORE::CString& itemKey = CORE::CString::Empty )
    {
        MT::CScopeMutex scope( lock );
        CORE::SJobRunRecord run;
        run.runId = nextRunId++;
        run.hostInstanceId = instanceId;
        run.jobId = jobId;
        run.itemKey = itemKey;
        run.itemLabel = itemKey.IsNULLOrEmpty() ? CORE::CString::Empty : CORE::CString( "item" );
        run.taskTypeName = "test.task";
        run.executionTarget = CORE::JOBEXECUTIONTARGET_POOL;
        run.threadPoolName = "gucef.scheduler.run";
        run.cause = CORE::JOBRUNCAUSE_RUN_NOW;
        run.state = CORE::JOBRUNSTATE_RUNNING;
        run.scheduledUtcMs = 1767225600000LL;
        run.startedUtcMs = 1767225600500LL;
        run.attempt = 1;
        run.note = "working on it";
        run.requestedBy = requestedBy;
        run.taskId = (UInt32) ( 100 + run.runId );
        run.revision = ++revision;
        runs.push_back( run );
        CORE::SJobStatus* job = FindJob( jobId );
        if ( GUCEF_NULL != job )
        {
            job->activeRunCount += 1;
            job->state = CORE::JOBSTATE_RUNNING;
            job->revision = ++revision;
        }
        return run.runId;
    }

    void EndRun( UInt64 runId, CORE::TJobRunState state, bool raise = true )
    {
        {
            MT::CScopeMutex scope( lock );
            CORE::SJobRunRecord* run = FindRun( runId );
            if ( GUCEF_NULL == run )
                return;
            run->state = state;
            run->endedUtcMs = 1767225601000LL;
            run->outcomeMessage = "done with secret detail";
            run->revision = ++revision;
            CORE::SJobStatus* job = FindJob( run->jobId );
            if ( GUCEF_NULL != job )
            {
                if ( job->activeRunCount > 0 )
                    job->activeRunCount -= 1;
                job->state = CORE::JOBSTATE_SCHEDULED;
                job->lastRunId = runId;
                job->lastRunState = state;
                job->lastRunEndedUtcMs = run->endedUtcMs;
                job->revision = ++revision;
            }
        }
        if ( raise )
            RaiseChanged();
    }

    /** A restart of the scheduler: a new instance id, revisions start over */
    void Restart( void )
    {
        {
            MT::CScopeMutex scope( lock );
            instanceId += 1;
            revision = 1;
            for ( UInt32 i=0; i<jobs.size(); ++i )
                jobs[ i ].revision = ++revision;
            for ( UInt32 i=0; i<lanes.size(); ++i )
                lanes[ i ].revision = ++revision;
            runs.clear();
        }
        RaiseChanged();
    }

    void RaiseChanged( void )
    {
        UInt64 current = 0;
        {
            MT::CScopeMutex scope( lock );
            current = revision;
        }
        notifier.Raise( current );
    }

    UInt32 GetApplyCalls( void ) const { MT::CScopeMutex scope( lock ); return applyCalls; }

    virtual const CORE::CString& GetHostName( void ) const GUCEF_VIRTUAL_OVERRIDE { return hostName; }
    virtual UInt64 GetHostInstanceId( void ) const GUCEF_VIRTUAL_OVERRIDE { MT::CScopeMutex scope( lock ); return instanceId; }

    virtual void GetSnapshot( CORE::CJobSchedulerSnapshot& snapshot, UInt64 changedAfterRevision ) const GUCEF_VIRTUAL_OVERRIDE
    {
        MT::CScopeMutex scope( lock );
        const_cast< CFakeSchedulerSource* >( this )->snapshotCalls++;
        snapshot.Clear();
        snapshot.SetHostName( hostName );
        snapshot.SetHostInstanceId( instanceId );
        snapshot.SetRevision( revision );
        snapshot.SetHostState( hostState );
        snapshot.SetServerTimeUtcMs( 1767225600000LL );
        snapshot.GetJobs() = jobs;
        snapshot.GetLanes() = lanes;
        // active runs first, then the ended ones newest first
        for ( UInt32 i=0; i<runs.size(); ++i )
            if ( !CORE::JobRunStateIsAnEndState( runs[ i ].state ) )
                snapshot.GetRuns().push_back( runs[ i ] );
        for ( UInt32 i=(UInt32)runs.size(); i>0; --i )
            if ( CORE::JobRunStateIsAnEndState( runs[ i-1 ].state ) )
                snapshot.GetRuns().push_back( runs[ i-1 ] );
    }

    virtual bool GetJobDetail( const CORE::CString& jobId, UInt32 recentRuns, bool includeSourceStatus, DRNUTILS::SSchedulerJobDetail& detail ) const GUCEF_VIRTUAL_OVERRIDE
    {
        MT::CScopeMutex scope( lock );
        CFakeSchedulerSource* self = const_cast< CFakeSchedulerSource* >( this );
        CORE::SJobStatus* job = self->FindJob( jobId );
        if ( GUCEF_NULL == job )
            return false;
        detail.job = *job;
        detail.job.description = "the description of " + jobId;
        for ( UInt32 i=(UInt32)runs.size(); i>0 && detail.recentRuns.size()<recentRuns; --i )
            if ( runs[ i-1 ].jobId == jobId )
                detail.recentRuns.push_back( runs[ i-1 ] );
        if ( includeSourceStatus )
            detail.sourceStatus.SetAttribute( "pending", "3" );
        return true;
    }

    virtual void GetDeclaredActions( DRNUTILS::TSchedulerActionInfoVector& actions ) const GUCEF_VIRTUAL_OVERRIDE
    {
        MT::CScopeMutex scope( lock );
        actions = declaredCustom;
    }

    virtual CORE::TJobControlStatus Apply( const CORE::SJobControlCommand& command, CORE::SJobControlResult& result ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( GUCEF_NULL != probeHost )
            Probe();
        CORE::TJobControlStatus status = CORE::JOBCONTROLSTATUS_OK;
        bool changed = false;
        {
            MT::CScopeMutex scope( lock );
            ++applyCalls;
            // the last commands only: a soak sends tens of thousands
            if ( applied.size() >= 256 )
                applied.erase( applied.begin() );
            applied.push_back( command );
            if ( command.dryRun )
                return CORE::JOBCONTROLSTATUS_OK;
            switch ( command.action )
            {
                case CORE::JOBCONTROLACTION_RUN_NOW:
                {
                    if ( GUCEF_NULL == FindJob( command.targetId ) )
                        return CORE::JOBCONTROLSTATUS_NOT_FOUND;
                    scope.EarlyUnlock();
                    result.runId = StartRun( command.targetId, command.requestedBy );
                    MT::CScopeMutex again( lock );
                    result.hasRun = true;
                    result.run = *FindRun( result.runId );
                    changed = true;
                    break;
                }
                case CORE::JOBCONTROLACTION_CANCEL_RUN:
                {
                    CORE::SJobRunRecord* run = FindRun( CORE::StringToUInt64( command.targetId ) );
                    if ( GUCEF_NULL == run )
                        return CORE::JOBCONTROLSTATUS_NOT_FOUND;
                    scope.EarlyUnlock();
                    EndRun( CORE::StringToUInt64( command.targetId ), CORE::JOBRUNSTATE_CANCELLED, false );
                    changed = true;
                    break;
                }
                case CORE::JOBCONTROLACTION_ENABLE:
                case CORE::JOBCONTROLACTION_DISABLE:
                case CORE::JOBCONTROLACTION_SET_PRIORITY:
                case CORE::JOBCONTROLACTION_REMOVE_JOB:
                {
                    CORE::SJobStatus* job = FindJob( command.targetId );
                    if ( GUCEF_NULL == job )
                        return CORE::JOBCONTROLSTATUS_NOT_FOUND;
                    if ( CORE::JOBCONTROLACTION_DISABLE == command.action )
                        job->state = CORE::JOBSTATE_DISABLED;
                    else if ( CORE::JOBCONTROLACTION_ENABLE == command.action )
                        job->state = CORE::JOBSTATE_SCHEDULED;
                    else if ( CORE::JOBCONTROLACTION_SET_PRIORITY == command.action )
                        job->priority = command.priority;
                    job->revision = ++revision;
                    if ( CORE::JOBCONTROLACTION_REMOVE_JOB == command.action )
                    {
                        for ( UInt32 i=0; i<jobs.size(); ++i )
                            if ( jobs[ i ].jobId == command.targetId )
                            {
                                jobs.erase( jobs.begin() + i );
                                break;
                            }
                    }
                    changed = true;
                    break;
                }
                case CORE::JOBCONTROLACTION_CUSTOM:
                {
                    result.detail.SetAttribute( "custom", command.customName );
                    result.messages.push_back( "custom applied" );
                    break;
                }
                default:
                    status = CORE::JOBCONTROLSTATUS_UNIMPLEMENTED;
            }
        }
        {
            MT::CScopeMutex scope( lock );
            result.revision = revision;
        }
        if ( changed && raiseOnApply )
            RaiseChanged();
        return status;
    }

    virtual void GetSubmittableTaskTypes( CORE::CString::StringSet& taskTypeNames ) const GUCEF_VIRTUAL_OVERRIDE
    {
        MT::CScopeMutex scope( lock );
        taskTypeNames = submittable;
    }

    virtual CORE::TJobControlStatus Submit( const CORE::SJobSubmission& submission, CORE::SJobControlResult& result ) GUCEF_VIRTUAL_OVERRIDE
    {
        {
            MT::CScopeMutex scope( lock );
            ++applyCalls;
            submitted.push_back( submission );
            if ( submission.dryRun )
                return CORE::JOBCONTROLSTATUS_OK;
        }
        CORE::SJobStatus& job = AddJob( submission.definition.GetId(), submission.definition.GetLaneName().IsNULLOrEmpty() ? CORE::CString( "lane1" ) : submission.definition.GetLaneName() );
        {
            MT::CScopeMutex scope( lock );
            job.origin = CORE::JOBORIGIN_SUBMITTED;
            job.submittedBy = submission.submittedBy;
            job.taskTypeName = submission.definition.GetTaskTypeName();
            job.remoteActions.insert( CORE::JOBCONTROLACTION_REMOVE_JOB );
            result.revision = revision;
        }
        return CORE::JOBCONTROLSTATUS_OK;
    }

    virtual CORE::CNotifier* GetChangeNotifier( void ) GUCEF_VIRTUAL_OVERRIDE { return &notifier; }

    private:

    /** Asks the host from another thread and waits for it: a lock of the host held around Apply() would stall it */
    void Probe( void );
};

/*-------------------------------------------------------------------------*/

/** Service level custom actions, recorded */
class CFakeCustomActionHandler : public DRNUTILS::CISchedulerCustomActionHandler
{
    public:
    UInt32 calls;
    CORE::CString::StringMap lastArguments;
    CFakeCustomActionHandler( void ) : calls( 0 ), lastArguments() {}
    virtual void GetCustomActions( DRNUTILS::TSchedulerActionInfoVector& actions ) const GUCEF_VIRTUAL_OVERRIDE
    {
        DRNUTILS::SSchedulerActionInfo info;
        info.action = CORE::JOBCONTROLACTION_CUSTOM;
        info.targetKind = CORE::JOBCONTROLTARGET_HOST;
        info.customName = "watchlist-add";
        info.title = "Add to the watch list";
        info.argumentNames.push_back( "symbol" );
        actions.push_back( info );
    }
    virtual CORE::TJobControlStatus ApplyCustomAction( const CORE::SJobControlCommand& command, CORE::CDataNode& detail, CORE::CString& message ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++calls;
        lastArguments = command.arguments;
        detail.SetAttribute( "added", "1" );
        message = "added to the watch list";
        return CORE::JOBCONTROLSTATUS_OK;
    }
};

}; /* namespace DRNUTILSTEST */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSFAKESCHEDULERSOURCE_H ? */
