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

#include "gucefCORE_JobSchedulerTypes.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char*
JobTriggerKindToString( TJobTriggerKind kind )
{GUCEF_TRACE;

    switch ( kind )
    {
        case JOBTRIGGER_SCHEDULE: return "SCHEDULE";
        case JOBTRIGGER_INTERVAL: return "INTERVAL";
        case JOBTRIGGER_ONCE_AT: return "ONCE_AT";
        case JOBTRIGGER_MANUAL: return "MANUAL";
        case JOBTRIGGER_WORK_SOURCE: return "WORK_SOURCE";
        default: return "UNSPECIFIED";
    }
}

/*-------------------------------------------------------------------------*/

const char*
JobExecutionTargetToString( TJobExecutionTarget target )
{GUCEF_TRACE;

    switch ( target )
    {
        case JOBEXECUTIONTARGET_POOL: return "POOL";
        case JOBEXECUTIONTARGET_MAIN_THREAD: return "MAIN_THREAD";
        default: return "UNSPECIFIED";
    }
}

/*-------------------------------------------------------------------------*/

const char*
JobStateToString( TJobState state )
{GUCEF_TRACE;

    switch ( state )
    {
        case JOBSTATE_SCHEDULED: return "SCHEDULED";
        case JOBSTATE_QUEUED: return "QUEUED";
        case JOBSTATE_WAITING: return "WAITING";
        case JOBSTATE_RUNNING: return "RUNNING";
        case JOBSTATE_DISABLED: return "DISABLED";
        case JOBSTATE_UNAVAILABLE: return "UNAVAILABLE";
        case JOBSTATE_ENDED: return "ENDED";
        default: return "UNSPECIFIED";
    }
}

/*-------------------------------------------------------------------------*/

const char*
JobRunStateToString( TJobRunState state )
{GUCEF_TRACE;

    switch ( state )
    {
        case JOBRUNSTATE_QUEUED: return "QUEUED";
        case JOBRUNSTATE_WAITING: return "WAITING";
        case JOBRUNSTATE_RUNNING: return "RUNNING";
        case JOBRUNSTATE_SUCCEEDED: return "SUCCEEDED";
        case JOBRUNSTATE_FAILED: return "FAILED";
        case JOBRUNSTATE_CANCELLED: return "CANCELLED";
        case JOBRUNSTATE_TIMED_OUT: return "TIMED_OUT";
        case JOBRUNSTATE_SKIPPED: return "SKIPPED";
        default: return "UNSPECIFIED";
    }
}

/*-------------------------------------------------------------------------*/

const char*
JobRunCauseToString( TJobRunCause cause )
{GUCEF_TRACE;

    switch ( cause )
    {
        case JOBRUNCAUSE_SCHEDULE: return "SCHEDULE";
        case JOBRUNCAUSE_CATCH_UP: return "CATCH_UP";
        case JOBRUNCAUSE_RUN_NOW: return "RUN_NOW";
        case JOBRUNCAUSE_RETRY: return "RETRY";
        case JOBRUNCAUSE_WORK_ITEM: return "WORK_ITEM";
        case JOBRUNCAUSE_API: return "API";
        case JOBRUNCAUSE_HOLD: return "HOLD";
        default: return "UNSPECIFIED";
    }
}

/*-------------------------------------------------------------------------*/

const char*
JobControlActionToString( TJobControlAction action )
{GUCEF_TRACE;

    switch ( action )
    {
        case JOBCONTROLACTION_RUN_NOW: return "RUN_NOW";
        case JOBCONTROLACTION_CANCEL_RUN: return "CANCEL_RUN";
        case JOBCONTROLACTION_ENABLE: return "ENABLE";
        case JOBCONTROLACTION_DISABLE: return "DISABLE";
        case JOBCONTROLACTION_SET_PRIORITY: return "SET_PRIORITY";
        case JOBCONTROLACTION_SET_LANE_MODE: return "SET_LANE_MODE";
        case JOBCONTROLACTION_CUSTOM: return "CUSTOM";
        case JOBCONTROLACTION_REMOVE_JOB: return "REMOVE_JOB";
        default: return "UNSPECIFIED";
    }
}

/*-------------------------------------------------------------------------*/

const char*
JobControlStatusToString( TJobControlStatus status )
{GUCEF_TRACE;

    switch ( status )
    {
        case JOBCONTROLSTATUS_OK: return "OK";
        case JOBCONTROLSTATUS_NOT_FOUND: return "NOT_FOUND";
        case JOBCONTROLSTATUS_INVALID_ARGUMENT: return "INVALID_ARGUMENT";
        case JOBCONTROLSTATUS_FAILED_PRECONDITION: return "FAILED_PRECONDITION";
        case JOBCONTROLSTATUS_ABORTED: return "ABORTED";
        case JOBCONTROLSTATUS_UNIMPLEMENTED: return "UNIMPLEMENTED";
        case JOBCONTROLSTATUS_RESOURCE_EXHAUSTED: return "RESOURCE_EXHAUSTED";
        default: return "INTERNAL";
    }
}

/*-------------------------------------------------------------------------*/

const char*
JobSchedulerHostStateToString( TJobSchedulerHostState state )
{GUCEF_TRACE;

    switch ( state )
    {
        case JOBSCHEDULERSTATE_STARTING: return "starting";
        case JOBSCHEDULERSTATE_RUNNING: return "running";
        case JOBSCHEDULERSTATE_DRAINING: return "draining";
        case JOBSCHEDULERSTATE_STOPPED: return "stopped";
        default: return "created";
    }
}

/*-------------------------------------------------------------------------*/

bool
JobRunStateIsAnEndState( TJobRunState state )
{GUCEF_TRACE;

    return state >= JOBRUNSTATE_SUCCEEDED && state <= JOBRUNSTATE_SKIPPED;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
