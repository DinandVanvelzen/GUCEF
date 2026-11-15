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

#ifndef GUCEF_CORE_CJOBSCHEDULER_H
#define GUCEF_CORE_CJOBSCHEDULER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_CORE_CIMONOTONICCLOCK_H
#include "gucefCORE_CIMonotonicClock.h"
#define GUCEF_CORE_CIMONOTONICCLOCK_H
#endif /* GUCEF_CORE_CIMONOTONICCLOCK_H ? */

#ifndef GUCEF_CORE_CSYSTEMUTCCLOCK_H
#include "gucefCORE_CSystemUtcClock.h"
#define GUCEF_CORE_CSYSTEMUTCCLOCK_H
#endif /* GUCEF_CORE_CSYSTEMUTCCLOCK_H ? */

#ifndef GUCEF_CORE_JOBSCHEDULERTYPES_H
#include "gucefCORE_JobSchedulerTypes.h"
#define GUCEF_CORE_JOBSCHEDULERTYPES_H
#endif /* GUCEF_CORE_JOBSCHEDULERTYPES_H ? */

#ifndef GUCEF_CORE_CJOBDEFINITION_H
#include "gucefCORE_CJobDefinition.h"
#define GUCEF_CORE_CJOBDEFINITION_H
#endif /* GUCEF_CORE_CJOBDEFINITION_H ? */

#ifndef GUCEF_CORE_CJOBLANEDEFINITION_H
#include "gucefCORE_CJobLaneDefinition.h"
#define GUCEF_CORE_CJOBLANEDEFINITION_H
#endif /* GUCEF_CORE_CJOBLANEDEFINITION_H ? */

#ifndef GUCEF_CORE_CJOBSCHEDULERSETTINGS_H
#include "gucefCORE_CJobSchedulerSettings.h"
#define GUCEF_CORE_CJOBSCHEDULERSETTINGS_H
#endif /* GUCEF_CORE_CJOBSCHEDULERSETTINGS_H ? */

#ifndef GUCEF_CORE_CJOBSCHEDULERSNAPSHOT_H
#include "gucefCORE_CJobSchedulerSnapshot.h"
#define GUCEF_CORE_CJOBSCHEDULERSNAPSHOT_H
#endif /* GUCEF_CORE_CJOBSCHEDULERSNAPSHOT_H ? */

#ifndef GUCEF_CORE_CJOBCONTROLCOMMAND_H
#include "gucefCORE_CJobControlCommand.h"
#define GUCEF_CORE_CJOBCONTROLCOMMAND_H
#endif /* GUCEF_CORE_CJOBCONTROLCOMMAND_H ? */

#ifndef GUCEF_CORE_CJOBCONTROLRESULT_H
#include "gucefCORE_CJobControlResult.h"
#define GUCEF_CORE_CJOBCONTROLRESULT_H
#endif /* GUCEF_CORE_CJOBCONTROLRESULT_H ? */

#ifndef GUCEF_CORE_CJOBSUBMISSION_H
#include "gucefCORE_CJobSubmission.h"
#define GUCEF_CORE_CJOBSUBMISSION_H
#endif /* GUCEF_CORE_CJOBSUBMISSION_H ? */

#ifndef GUCEF_CORE_CJOBSCHEDULERCHANGE_H
#include "gucefCORE_CJobSchedulerChange.h"
#define GUCEF_CORE_CJOBSCHEDULERCHANGE_H
#endif /* GUCEF_CORE_CJOBSCHEDULERCHANGE_H ? */

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_CORE_CJOBWORKITEM_H
#include "gucefCORE_CJobWorkItem.h"
#define GUCEF_CORE_CJOBWORKITEM_H
#endif /* GUCEF_CORE_CJOBWORKITEM_H ? */

#ifndef GUCEF_CORE_CJOBRUNHISTORY_H
#include "gucefCORE_CJobRunHistory.h"
#define GUCEF_CORE_CJOBRUNHISTORY_H
#endif /* GUCEF_CORE_CJOBRUNHISTORY_H ? */

#ifndef GUCEF_CORE_CJOBSCHEDULERRUNLINK_H
#include "gucefCORE_CJobSchedulerRunLink.h"
#define GUCEF_CORE_CJOBSCHEDULERRUNLINK_H
#endif /* GUCEF_CORE_CJOBSCHEDULERRUNLINK_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_CSCHEDULEDJOBINFO_H
#include "gucefCORE_CScheduledJobInfo.h"
#define GUCEF_CORE_CSCHEDULEDJOBINFO_H
#endif /* GUCEF_CORE_CSCHEDULEDJOBINFO_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CTaskManager;
class CIJobWorkSource;
class CIJobSchedulerStateStore;
class CIJobPayloadValidator;

/*-------------------------------------------------------------------------*/

/**
 *  A job scheduler: a facility of the task manager. Threads are globally impactful and the task manager is the global
 *  overview of such things, so a scheduler is created, owned and found through CTaskManager
 *  (CTaskManager::GetOrCreateJobScheduler()) and every run is a task of the task manager.
 *
 *  The scheduler adds no work abstraction beside tasks. A job (CJobDefinition) names a task type whose task consumer
 *  factory is registered with the task manager, plus a payload the task data factory of that type deserializes. A run
 *  is queued with the existing mechanisms (CThreadPool::QueueTask() with the task data, the scheduler observing the
 *  task) on:
 *    - POOL: the pool the job names, or the run pool of the settings;
 *    - MAIN_THREAD: the main thread pool of the settings, a pool without threads of its own, whose queued tasks the
 *      thread of the main pulse generator carries out with CThreadPool::CarryOutQueuedTasksIfAny() (at most
 *      MaxMainThreadRunsPerPulse per pulse). The dispatcher only queues the task and mails a wake to the main pulse
 *      generator; it never waits for the main thread and never runs task code itself. Main thread jobs must be short
 *      and cooperative: a long one stalls the application's main thread.
 *  The outcome of a run is the end status of its task (TASKSTATUS_FINISHED = SUCCEEDED, CANCELLED = CANCELLED, stopped
 *  after the job's timeout = TIMED_OUT, error statuses and KILLED = FAILED); the note of a run is the task's status extra
 *  info. Cancelling a run is CThreadPool::RequestTaskCancellation() of its task.
 *  Value-driven work (a planner that decides what is due) is a CIJobWorkSource; its items are (task type, payload)
 *  pairs too.
 *
 *  Jobs can also be submitted as data (SubmitJob()): only for task types registered as submittable on this scheduler
 *  (RegisterSubmittableTaskType(), with an optional payload validator); none are by default.
 *
 *  Threads: one dispatcher task on the dispatcher pool sleeps until the next due instant (at most MaxSleepMs). Control()
 *  and the views run on the calling thread, bookkeeping only, under the scheduler lock (a leaf lock: nothing is called
 *  under it that takes another lock, except throttle policies under their contract). Events are mailed to the main pulse
 *  generator (CTSGNotifier); subscribe per event with CTEventHandlerFunctor. The task manager lock and the scheduler
 *  lock are never held together.
 *
 *  Not in this version: pause and resume of jobs, lanes and runs (deferred, with task parking).
 */
class GUCEF_CORE_PUBLIC_CPP CJobScheduler : public CTSGNotifier ,
                                             public CTSharedPtrCreator< CJobScheduler, MT::CMutex >
{
    public:

    static const CString ClassTypeName;

    static const CEvent JobChangedEvent;            /**< TJobSchedulerChangeEventData, kind JOB */
    static const CEvent LaneChangedEvent;           /**< TJobSchedulerChangeEventData, kind LANE */
    static const CEvent RunChangedEvent;            /**< TJobSchedulerChangeEventData, kind RUN; progress at most once per ProgressEventMinIntervalMs per run */
    static const CEvent HostStateChangedEvent;      /**< TJobSchedulerChangeEventData, kind HOST */

    static void RegisterEvents( void );

    typedef CTCloneableObj< SJobSchedulerChange >                                       TJobSchedulerChangeEventData;
    typedef CTSharedPtrCreator< CJobScheduler, MT::CMutex >::TBasicSharedPtrType        JobSchedulerPtr;

    /** Waits for the dispatcher and the runs (Stop()) */
    virtual ~CJobScheduler() GUCEF_VIRTUAL_OVERRIDE;

    /** The registry key of the task manager (settings HostName) */
    const CString& GetName( void ) const;

    const CJobSchedulerSettings& GetSettings( void ) const;

    /** The application's main pulse generator: events are mailed to it and MAIN_THREAD runs are carried out on its thread */
    PulseGeneratorPtr GetMainPulseGenerator( void ) const;

    TJobSchedulerHostState GetHostState( void ) const;

    /** Before Start(); not owned. NULL restores the system clocks */
    void SetClocks( const CIUtcClock* utcClock, const CIMonotonicClock* monotonicClock );

    /** Before Start(); not owned. NULL = no persistence */
    void SetStateStore( CIJobSchedulerStateStore* store );

    bool AddLane( const CJobLaneDefinition& lane, CString& error );

    bool RemoveLane( const CString& laneName );

    /** The service reports a lane (un)available, with the reason shown in the views */
    bool SetLaneAvailability( const CString& laneName, bool available, const CString& reason );

    /** Adds a job defined in code. Its task type must have a task consumer factory registered with the task manager */
    bool AddJob( const CJobDefinition& job, CString& error );

    /** Adds a job with trigger WORK_SOURCE; the source is not owned and must outlive the job */
    bool AddWorkSourceJob( const CJobDefinition& job, CIJobWorkSource* source, CString& error );

    bool RemoveJob( const CString& jobId );

    /** The service reports a job (un)available, with the reason shown in the views */
    bool SetJobAvailability( const CString& jobId, bool available, const CString& reason );

    bool GetJobDefinition( const CString& jobId, CJobDefinition& job ) const;

    void GetJobIds( CString::StringSet& jobIds ) const;

    /**
     *  Allows jobs of the task type to be submitted as data (SubmitJob()), with an optional payload validator (not owned).
     *  The task type must have a task consumer factory registered with the task manager. Nothing is submittable by default
     */
    bool RegisterSubmittableTaskType( const CString& taskTypeName, const CIJobPayloadValidator* validator = GUCEF_NULL );

    /** Jobs already submitted for the task type stay until removed */
    bool UnregisterSubmittableTaskType( const CString& taskTypeName );

    void GetSubmittableTaskTypes( CString::StringSet& taskTypeNames ) const;

    /**
     *  Defines (or with replaceExisting replaces) a job from data, on the calling thread: the task type must be
     *  submittable here, the payload within MaxSubmitPayloadBytes, accepted by the validator of the task type and
     *  deserializable by the task data factory of the task type; at most MaxSubmittedJobs. FAILED_PRECONDITION for a task
     *  type that is not submittable, RESOURCE_EXHAUSTED for the bounds, INVALID_ARGUMENT for a bad definition or payload.
     *  Submitted jobs are kept with the state (state store)
     */
    TJobControlStatus SubmitJob( const SJobSubmission& submission, SJobControlResult& result );

    /** No I/O on the calling thread: the first dispatcher cycle loads the state */
    bool Start( CString& error );

    /** No new run; the running runs' tasks are asked to cancel; returns at once */
    void BeginDrain( void );

    bool IsIdle( void ) const;

    /** Ends the dispatcher, waits for runs up to StopTimeoutMs, saves the state */
    void Stop( void );

    /** Wakes the dispatcher for a cycle, from any thread (a work source has new work); takes a leaf lock only */
    void RequestDispatch( void );

    /** Applies a control command on the calling thread (bookkeeping under the scheduler lock) and wakes the dispatcher */
    TJobControlStatus Control( const SJobControlCommand& command, SJobControlResult& result );

    /** Exclusive use of a lane by an outside party once its runs in flight have ended; see CJobSchedulerLaneHoldScope */
    bool TryHoldLane( const CString& laneName, const CString& holder, CString& refusal );

    /** Ends a hold; the throttle policy is told the cost of what the holder did (cause HOLD) */
    void ReleaseLaneHold( const CString& laneName, UInt32 actualCost );

    /** A copy of the model in one step; with changedAfterRevision only the rows changed after it */
    void GetSnapshot( CJobSchedulerSnapshot& snapshot, UInt64 changedAfterRevision = 0 ) const;

    bool GetRun( UInt64 runId, SJobRunRecord& run ) const;

    /**
     *  The status of the work source of a job with trigger WORK_SOURCE (CIJobWorkSource::GetStatus(), for GetJob with
     *  includeSourceStatus). The source is called on the calling thread outside the scheduler lock (it keeps its own
     *  leaf lock). False when there is no such job or the job has no work source
     */
    bool GetWorkSourceStatus( const CString& jobId, CDataNode& status, bool includeQueue = false ) const;

    UInt64 GetRevision( void ) const;

    UInt64 GetHostInstanceId( void ) const;

    /** The jobs as the task manager overview shows them (CTaskManager::GetAllScheduledJobInfo()) */
    void GetScheduledJobInfo( TScheduledJobInfoVector& info ) const;

    /**
     *  Async form of run-now: queues a run with cause RUN_NOW and returns a future that the scheduler completes when the
     *  run ends, with a TCloneableJobRunRecord as result data. A blocking "run and wait" is the caller's wait on the
     *  future, never a pump of another thread's pulse generator
     */
    CFutureResult RunNowAsync( const CString& jobId, const CString& requestedBy );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    friend class CTaskManager;
    friend class CJobSchedulerRunLink;
    friend class CJobSchedulerTask;
    friend class CJobSchedulerTestAccess;   /**< tests: dispatch cycles on the test thread, dispatcher figures */

    /** Only the task manager makes schedulers (CTaskManager::GetOrCreateJobScheduler()) */
    CJobScheduler( CTaskManager& taskManager                     ,
                   const PulseGeneratorPtr& mainPulseGenerator   ,
                   const CJobSchedulerSettings& settings         );

    CJobScheduler( void );                                      /**< not implemented */
    CJobScheduler( const CJobScheduler& src );                  /**< not implemented */
    CJobScheduler& operator=( const CJobScheduler& src );       /**< not implemented */

    private:

    /** The pulses of the main pulse generator: carries out queued MAIN_THREAD runs on that thread */
    virtual void OnPumpedNotify( CNotifier* notifier                 ,
                                 const CEvent& eventid               ,
                                 CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef GUCEF::vector< SJobSchedulerChange >    TChangeVector;
    typedef GUCEF::vector< UInt64 >                 TUInt64Vector;

    struct SJobEntry
    {
        CJobDefinition definition;
        CIJobWorkSource* workSource;            /**< WORK_SOURCE: not owned */
        bool enabled;
        bool priorityOverridden;
        bool available;
        CString unavailableReason;
        bool ended;                             /**< ONCE_AT: its occurrence was handled */
        bool planned;                           /**< SCHEDULE, INTERVAL: due instants are set up (first cycle or the add after it) */
        Int64 lastOccurrenceUtcMs;              /**< SCHEDULE, ONCE_AT: the last occurrence handled (persisted) */
        Int64 nextDueUtcMs;                     /**< SCHEDULE, ONCE_AT: the next occurrence; INTERVAL: for the views; 0 = none */
        UInt64 nextDueMonotonicMs;              /**< INTERVAL: the next due instant; 0 = waiting for the end of a run */
        Int64 sourceNextWakeUtcMs;              /**< WORK_SOURCE: when the source wants to be evaluated again; 0 = at the next cycle only */
        UInt32 pendingRunCount;                 /**< queued or waiting */
        UInt32 runningRunCount;
        UInt64 runCount;
        UInt64 failureCount;
        UInt32 consecutiveFailures;
        UInt64 lastRunId;
        TJobRunState lastRunState;
        Int64 lastRunEndedUtcMs;
        CString waitingFor;
        Int64 waitingUntilUtcMs;
        UInt64 revision;
        CJobRunHistory recentRuns;              /**< the last PerJobHistory ended runs */

        SJobEntry( void );
    };
    typedef GUCEF::map< CString, SJobEntry > TJobMap;

    struct SLaneEntry
    {
        CJobLaneDefinition definition;
        CString mode;
        bool modeOverridden;
        bool available;
        CString unavailableReason;
        UInt32 inFlight;
        CString heldBy;                         /**< non empty while an outside party holds the lane */
        Int64 heldSinceUtcMs;
        CString waitingFor;
        Int64 waitingUntilUtcMs;
        UInt64 revision;

        SLaneEntry( void );
    };
    typedef GUCEF::map< CString, SLaneEntry > TLaneMap;

    struct SRunEntry
    {
        SJobRunRecord record;
        UInt32 priority;
        UInt64 sequence;                        /**< enqueue order */
        UInt64 notBeforeMonotonicMs;            /**< retry backoff; 0 = none */
        UInt64 startedMonotonicMs;              /**< when the task was handed to the task manager; 0 = not yet */
        UInt64 timeoutMs;
        bool taskQueued;
        bool cancelRequested;
        bool cancelSent;
        bool timeoutRequested;
        CString laneName;                       /**< the effective lane */
        UInt32 estimatedCost;
        bool hasWorkItem;
        SJobWorkItem workItem;
        CFutureResultSource* future;            /**< RunNowAsync(): owned, handed on to a retry and completed when the run ends */

        SRunEntry( void );
    };
    typedef GUCEF::map< UInt64, SRunEntry > TRunMap;

    struct SEndedTask
    {
        UInt64 runId;
        UInt32 taskId;
        TTaskStatus status;
        CString statusExtra;
    };
    typedef GUCEF::vector< SEndedTask > TEndedTaskVector;

    struct SStartOrder
    {
        UInt64 runId;
        CString jobId;
        CString taskTypeName;
        CDataNode payload;
        CICloneable* taskData;                  /**< a work item's ready task data: a clone, owned by the order */
        TJobExecutionTarget executionTarget;
        CString threadPoolName;
    };
    typedef GUCEF::vector< SStartOrder > TStartOrderVector;

    struct STaskAction
    {
        CString threadPoolName;
        UInt32 taskId;
    };
    typedef GUCEF::vector< STaskAction > TTaskActionVector;

    struct SItemFinished
    {
        CIJobWorkSource* source;
        SJobWorkItem item;
        SJobRunRecord run;
    };
    typedef GUCEF::vector< SItemFinished > TItemFinishedVector;

    struct SFutureCompletion
    {
        CFutureResultSource* future;            /**< owned until completed */
        SJobRunRecord run;
    };
    typedef GUCEF::vector< SFutureCompletion > TFutureCompletionVector;

    struct SSubmittableTaskType
    {
        const CIJobPayloadValidator* validator;
    };
    typedef GUCEF::map< CString, SSubmittableTaskType > TSubmittableTaskTypeMap;

    /** What one locked step collected to do after the lock is released */
    struct SDeferred
    {
        TTaskActionVector cancellations;
        TItemFinishedVector itemsFinished;
        TFutureCompletionVector futures;
        bool wakeMainThread;
        bool wakeDispatcher;
        bool postChanges;

        SDeferred( void );
    };

    typedef GUCEF::map< CString, SJobSchedulerChange >  TChangeMap;      /**< by kind and key */
    typedef GUCEF::map< UInt64, SJobSchedulerChange >   TRunChangeMap;   /**< by run id */

    class CWorkItemSink;
    friend class CWorkItemSink;

    // engine internals, all under m_lock unless the comment says otherwise
    static const CString DefaultLaneName;

    void QueueChange( const SJobSchedulerChange& change, SDeferred& deferred );
    void DeliverPendingChanges( void );                 /**< main pulse generator's thread, no lock held */

    void OnRunTaskEnded( UInt64 runId, UInt32 taskId, TTaskStatus endStatus, const CString& statusExtra );
    Int64 DispatchCycle( void );                        /**< dispatcher thread; ms until the next cycle, -1 = stop */
    void OnDispatcherStarted( const PulseGeneratorPtr& dispatcherPulse );
    void OnDispatcherEnded( void );
    bool StartImpl( bool withDispatcherTask, CString& error );
    void LoadStateOnDispatcher( void );                 /**< no lock held when called */
    void ApplyLoadedJobState( SJobEntry& job, const CDataNode& jobState );
    bool BuildState( CDataNode& state ) const;
    bool SaveStateNow( void );                          /**< no lock held when called */
    void MarkStateDirty( UInt64 nowMonotonicMs );
    UInt64 NextRevision( void );
    const CString& EffectiveLaneName( const CJobDefinition& job ) const;
    CString EffectivePoolName( const CJobDefinition& job ) const;
    SLaneEntry* FindLane( const CString& laneName );
    TJobState ComputeJobState( const SJobEntry& job ) const;
    void TouchJob( SJobEntry& job, SDeferred& deferred );
    void TouchLane( SLaneEntry& lane, SDeferred& deferred );
    void TouchRun( SJobRunRecord& run, SDeferred& deferred );
    void TouchHost( SDeferred& deferred );
    void PlanJob( SJobEntry& job, Int64 nowUtcMs, UInt64 nowMonotonicMs );
    void EvaluateDueJobs( Int64 nowUtcMs, UInt64 nowMonotonicMs, SDeferred& deferred );
    UInt64 QueueRun( SJobEntry& job, TJobRunCause cause, Int64 scheduledUtcMs, UInt32 priority, const CString& requestedBy,
                     const SJobWorkItem* workItem, SDeferred& deferred, bool applyConcurrency );
    void RecordSkippedRun( SJobEntry& job, TJobRunCause cause, Int64 scheduledUtcMs, const CString& reason, SDeferred& deferred );
    void SelectRunsToStart( Int64 nowUtcMs, UInt64 nowMonotonicMs, TStartOrderVector& orders, SDeferred& deferred );
    void ProcessEndedTasks( Int64 nowUtcMs, UInt64 nowMonotonicMs, SDeferred& deferred );
    void EndRun( UInt64 runId, TJobRunState state, const CString& note, const CString& message, Int64 nowUtcMs, UInt64 nowMonotonicMs, SDeferred& deferred );
    void CheckTimeouts( UInt64 nowMonotonicMs, SDeferred& deferred );
    Int64 ComputeNextWakeDelayMs( Int64 nowUtcMs, UInt64 nowMonotonicMs ) const;
    void StartOrders( TStartOrderVector& orders );      /**< no lock held when called */
    void CompleteDeferred( SDeferred& deferred );       /**< no lock held when called */
    void CancelAllActiveRuns( const CString& reason, SDeferred& deferred );
    TJobControlStatus ControlLocked( const SJobControlCommand& command, SJobControlResult& result, SDeferred& deferred,
                                     CIJobWorkSource*& customSource, CString& runNowLane );
    bool AddJobImpl( const CJobDefinition& job, CIJobWorkSource* source, bool fromState, CString& error );
    void FillJobStatus( const SJobEntry& job, SJobStatus& status ) const;
    void FillLaneStatus( const SLaneEntry& lane, SJobLaneStatus& status, Int64 nowUtcMs ) const;
    void RequestMainThreadPulse( void );

    private:

    CTaskManager* m_taskManager;
    PulseGeneratorPtr m_mainPulseGenerator;
    CJobSchedulerSettings m_settings;
    CSystemUtcClock m_systemUtcClock;
    const CIUtcClock* m_utcClock;
    const CIMonotonicClock* m_monotonicClock;
    CIJobSchedulerStateStore* m_stateStore;
    UInt64 m_hostInstanceId;
    UInt64 m_revision;
    TJobSchedulerHostState m_hostState;
    MT::CMutex m_lock;

    CJobSchedulerRunLinkPtr m_runLink;
    TJobMap m_jobs;
    TLaneMap m_lanes;
    TRunMap m_activeRuns;
    TEndedTaskVector m_endedTasks;
    CJobRunHistory m_history;                   /**< ended runs, at most MaxRunHistory */
    TSubmittableTaskTypeMap m_submittable;
    CDataNode m_loadedState;                    /**< the kept state, for jobs added after it was loaded */
    bool m_stateLoaded;
    bool m_stateDirty;
    UInt64 m_stateSaveDueMonotonicMs;
    UInt64 m_nextRunId;
    UInt64 m_nextSequence;
    Int64 m_startUtcMs;
    UInt64 m_startMonotonicMs;
    Int64 m_lastCycleUtcMs;
    UInt64 m_lastCycleMonotonicMs;
    bool m_stopRequested;
    bool m_dispatcherActive;
    bool m_manualDispatch;                      /**< tests: cycles run on the test thread, no dispatcher task */
    UInt32 m_dispatcherTaskId;
    CString m_dispatcherPoolName;
    UInt64 m_cycleCount;
    UInt64 m_clockJumpCount;
    UInt64 m_lastPlannedDelayMs;
    UInt64 m_dispatcherSystemThreadId;
    UInt64 m_totalRunsStarted;
    UInt64 m_totalRunsSucceeded;
    UInt64 m_totalRunsFailed;
    UInt64 m_totalRunsCancelled;
    UInt64 m_totalRunsTimedOut;
    UInt64 m_totalRunsSkipped;
    UInt32 m_submittedJobCount;

    TChangeMap m_pendingChanges;
    TRunChangeMap m_pendingRunChanges;
    bool m_changesPosted;
    MT::CTAtomic< UInt32 > m_changesPendingHint;   /**< read on every pulse of the main pulse generator without the lock */

    MT::CMutex m_wakeLock;                      /**< leaf: the dispatcher's pulse generator only */
    PulseGeneratorPtr m_dispatcherPulse;
    MT::CTAtomic< UInt32 > m_mainThreadRunsQueued;
};

/*-------------------------------------------------------------------------*/

typedef CJobScheduler::JobSchedulerPtr  JobSchedulerPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CJOBSCHEDULER_H ? */
