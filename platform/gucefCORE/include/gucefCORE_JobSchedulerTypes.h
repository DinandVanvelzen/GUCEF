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

#ifndef GUCEF_CORE_JOBSCHEDULERTYPES_H
#define GUCEF_CORE_JOBSCHEDULERTYPES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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

/**
 *  Value types of the job scheduler (CJobScheduler), a facility of the task manager.
 *  The numbers of the enumerators are the numbers of the matching enums of the contract gucef.drnutils.scheduler 1.0,
 *  so they are never renumbered. Numbers that are skipped are reserved for a later version.
 */

/** When a job becomes due */
enum EJobTriggerKind
{
    JOBTRIGGER_UNSPECIFIED      = 0 ,
    JOBTRIGGER_SCHEDULE         = 1 ,   /**< a CSchedule (cron and/or date time range), matched in UTC only */
    JOBTRIGGER_INTERVAL         = 2 ,   /**< a fixed period on the monotonic clock, between run starts or run ends */
    JOBTRIGGER_ONCE_AT          = 3 ,   /**< one UTC instant, the job ends after it */
    JOBTRIGGER_MANUAL           = 4 ,   /**< never due by itself: run-now and API only */
    JOBTRIGGER_WORK_SOURCE      = 5     /**< a CIJobWorkSource decides what is due */
};
typedef enum EJobTriggerKind TJobTriggerKind;

/** What the period of an INTERVAL trigger is measured from */
enum EJobIntervalFrom
{
    JOBINTERVALFROM_START       = 1 ,
    JOBINTERVALFROM_END         = 2
};
typedef enum EJobIntervalFrom TJobIntervalFrom;

/**
 *  Where a run of a job executes. Threads are globally impactful, which is why the scheduler is a facility of the
 *  task manager: every run is a task of the task manager, on a pool or on the main thread
 */
enum EJobExecutionTarget
{
    JOBEXECUTIONTARGET_UNSPECIFIED  = 0 ,
    JOBEXECUTIONTARGET_POOL         = 1 ,   /**< a task on a named thread pool (default: the scheduler's run pool) */
    JOBEXECUTIONTARGET_MAIN_THREAD  = 2     /**< a task carried out by the thread of the scheduler's main pulse generator: short, cooperative work only */
};
typedef enum EJobExecutionTarget TJobExecutionTarget;

/** State of a job, computed from its definition, its runs, its lane and its availability */
enum EJobState
{
    JOBSTATE_UNSPECIFIED        = 0 ,
    JOBSTATE_SCHEDULED          = 1 ,   /**< waiting for its next due time */
    JOBSTATE_QUEUED             = 2 ,
    JOBSTATE_WAITING            = 3 ,   /**< lane busy, throttle, hold or backoff: SJobStatus::waitingFor says which and until when */
    JOBSTATE_RUNNING            = 4 ,
    /* 5 is reserved for PAUSED (pause and resume are deferred) */
    JOBSTATE_DISABLED           = 6 ,
    JOBSTATE_UNAVAILABLE        = 7 ,   /**< the job or its lane was reported unavailable, with a reason */
    JOBSTATE_ENDED              = 8     /**< no further occurrence */
};
typedef enum EJobState TJobState;

/** State of one run */
enum EJobRunState
{
    JOBRUNSTATE_UNSPECIFIED     = 0 ,
    JOBRUNSTATE_QUEUED          = 1 ,
    JOBRUNSTATE_WAITING         = 2 ,
    JOBRUNSTATE_RUNNING         = 3 ,
    JOBRUNSTATE_SUCCEEDED       = 4 ,   /**< end state: the task ended TASKSTATUS_FINISHED */
    JOBRUNSTATE_FAILED          = 5 ,   /**< end state: the task ended in an error status or was killed */
    JOBRUNSTATE_CANCELLED       = 6 ,   /**< end state */
    JOBRUNSTATE_TIMED_OUT       = 7 ,   /**< end state: stopped after the job's timeout */
    JOBRUNSTATE_SKIPPED         = 8     /**< end state: missed by policy, or concurrency SKIP_IF_RUNNING */
    /* 9, 10, 11 are reserved for the park states (pause and resume are deferred) */
};
typedef enum EJobRunState TJobRunState;

/** Why a run exists */
enum EJobRunCause
{
    JOBRUNCAUSE_UNSPECIFIED     = 0 ,
    JOBRUNCAUSE_SCHEDULE        = 1 ,
    JOBRUNCAUSE_CATCH_UP        = 2 ,   /**< a missed occurrence, per the missed run policy */
    JOBRUNCAUSE_RUN_NOW         = 3 ,
    JOBRUNCAUSE_RETRY           = 4 ,
    JOBRUNCAUSE_WORK_ITEM       = 5 ,   /**< a work item of a CIJobWorkSource */
    JOBRUNCAUSE_API             = 6 ,
    JOBRUNCAUSE_HOLD            = 7     /**< not a run: the use of a lane by a hold holder, reported to the throttle policy */
};
typedef enum EJobRunCause TJobRunCause;

/** What happens when a job becomes due while a run of it is still going */
enum EJobConcurrency
{
    JOBCONCURRENCY_SKIP_IF_RUNNING  = 1 ,
    JOBCONCURRENCY_QUEUE_ONE        = 2 ,   /**< at most one pending run */
    JOBCONCURRENCY_PARALLEL         = 3     /**< up to the job's maximum of parallel runs */
};
typedef enum EJobConcurrency TJobConcurrency;

/** What happens with occurrences that were missed (process down, clock jump, lateness above the maximum) */
enum EJobMissedRunPolicy
{
    JOBMISSEDRUNPOLICY_SKIP         = 1 ,
    JOBMISSEDRUNPOLICY_RUN_ONCE     = 2 ,   /**< coalesced into one run, as CScheduleTrigger */
    JOBMISSEDRUNPOLICY_RUN_EACH     = 3     /**< one run each, up to the job's maximum of catch up runs */
};
typedef enum EJobMissedRunPolicy TJobMissedRunPolicy;

/** Control actions on a scheduler (local or remote) */
enum EJobControlAction
{
    JOBCONTROLACTION_UNSPECIFIED    = 0 ,
    /* 1 and 2 are reserved for PAUSE and RESUME (deferred) */
    JOBCONTROLACTION_RUN_NOW        = 3 ,
    JOBCONTROLACTION_CANCEL_RUN     = 4 ,
    JOBCONTROLACTION_ENABLE         = 5 ,
    JOBCONTROLACTION_DISABLE        = 6 ,
    JOBCONTROLACTION_SET_PRIORITY   = 7 ,
    JOBCONTROLACTION_SET_LANE_MODE  = 8 ,
    JOBCONTROLACTION_CUSTOM         = 9 ,   /**< a named action declared by a work source or the service */
    JOBCONTROLACTION_REMOVE_JOB     = 10    /**< removes a job that was submitted as data (never a job defined in code) */
};
typedef enum EJobControlAction TJobControlAction;

/** What a control action applies to */
enum EJobControlTargetKind
{
    JOBCONTROLTARGET_UNSPECIFIED    = 0 ,
    JOBCONTROLTARGET_JOB            = 1 ,
    JOBCONTROLTARGET_LANE           = 2 ,
    JOBCONTROLTARGET_RUN            = 3 ,
    JOBCONTROLTARGET_HOST           = 4
};
typedef enum EJobControlTargetKind TJobControlTargetKind;

/** Outcome of a control action or a submission */
enum EJobControlStatus
{
    JOBCONTROLSTATUS_OK                     = 0 ,
    JOBCONTROLSTATUS_NOT_FOUND              = 1 ,
    JOBCONTROLSTATUS_INVALID_ARGUMENT       = 2 ,
    JOBCONTROLSTATUS_FAILED_PRECONDITION    = 3 ,   /**< host starting or draining, job disabled for run-now, task type not submittable */
    JOBCONTROLSTATUS_ABORTED                = 4 ,   /**< the expected revision does not match */
    JOBCONTROLSTATUS_UNIMPLEMENTED          = 5 ,   /**< a custom action nobody declares */
    JOBCONTROLSTATUS_RESOURCE_EXHAUSTED     = 6 ,   /**< a bound was reached (submitted jobs, payload size) */
    JOBCONTROLSTATUS_INTERNAL               = 7
};
typedef enum EJobControlStatus TJobControlStatus;

/** Life cycle of a scheduler */
enum EJobSchedulerHostState
{
    JOBSCHEDULERSTATE_CREATED   = 0 ,
    JOBSCHEDULERSTATE_STARTING  = 1 ,
    JOBSCHEDULERSTATE_RUNNING   = 2 ,
    JOBSCHEDULERSTATE_DRAINING  = 3 ,
    JOBSCHEDULERSTATE_STOPPED   = 4
};
typedef enum EJobSchedulerHostState TJobSchedulerHostState;

/** Kind of a change of the scheduler model; the numbers equal the contract's ChangeKind */
enum EJobSchedulerChangeKind
{
    JOBSCHEDULERCHANGE_UNSPECIFIED  = 0 ,
    JOBSCHEDULERCHANGE_JOB          = 1 ,
    JOBSCHEDULERCHANGE_LANE         = 2 ,
    JOBSCHEDULERCHANGE_RUN          = 3 ,
    JOBSCHEDULERCHANGE_HOST         = 6
};
typedef enum EJobSchedulerChangeKind TJobSchedulerChangeKind;

/** Where a job definition came from */
enum EJobOrigin
{
    JOBORIGIN_CODE          = 1 ,   /**< added by the service in code (AddJob, AddWorkSourceJob) */
    JOBORIGIN_SUBMITTED     = 2     /**< submitted as data (SubmitJob, from settings, RPC or DRN) for an allowlisted task type */
};
typedef enum EJobOrigin TJobOrigin;

/** Flags of a job definition */
enum EJobFlags
{
    JOBFLAG_NONE                = 0 ,
    JOBFLAG_HIDDEN              = 1 ,   /**< never listed remotely */
    JOBFLAG_SENSITIVE           = 2 ,   /**< notes and messages are redacted before they leave the process */
    JOBFLAG_EXPOSE_PAYLOAD      = 4     /**< the payload may be shown to remote viewers */
};

typedef GUCEF::set< Int32 > TJobControlActionSet;     /**< TJobControlAction values */

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP const char* JobTriggerKindToString( TJobTriggerKind kind );
GUCEF_CORE_PUBLIC_CPP const char* JobExecutionTargetToString( TJobExecutionTarget target );
GUCEF_CORE_PUBLIC_CPP const char* JobStateToString( TJobState state );
GUCEF_CORE_PUBLIC_CPP const char* JobRunStateToString( TJobRunState state );
GUCEF_CORE_PUBLIC_CPP const char* JobRunCauseToString( TJobRunCause cause );
GUCEF_CORE_PUBLIC_CPP const char* JobControlActionToString( TJobControlAction action );
GUCEF_CORE_PUBLIC_CPP const char* JobControlStatusToString( TJobControlStatus status );
GUCEF_CORE_PUBLIC_CPP const char* JobSchedulerHostStateToString( TJobSchedulerHostState state );

/** Whether the run state is an end state (SUCCEEDED, FAILED, CANCELLED, TIMED_OUT, SKIPPED) */
GUCEF_CORE_PUBLIC_CPP bool JobRunStateIsAnEndState( TJobRunState state );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_JOBSCHEDULERTYPES_H ? */
