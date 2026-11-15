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

#ifndef GUCEF_CORE_CJOBSTATUS_H
#define GUCEF_CORE_CJOBSTATUS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_JOBSCHEDULERTYPES_H
#include "gucefCORE_JobSchedulerTypes.h"
#define GUCEF_CORE_JOBSCHEDULERTYPES_H
#endif /* GUCEF_CORE_JOBSCHEDULERTYPES_H ? */

#ifndef GUCEF_CORE_CJOBCUSTOMACTION_H
#include "gucefCORE_CJobCustomAction.h"
#define GUCEF_CORE_CJOBCUSTOMACTION_H
#endif /* GUCEF_CORE_CJOBCUSTOMACTION_H ? */

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
 *  How one job stands, as a row of a CJobSchedulerSnapshot. A copy, taken under the scheduler lock.
 */
struct GUCEF_CORE_PUBLIC_CPP SJobStatus
{
    CString jobId;
    CString title;
    CString description;
    CString taskTypeName;
    CString laneName;
    TJobState state;
    UInt32 priority;
    TJobTriggerKind triggerKind;
    CString triggerText;                /**< CJobTrigger::GetDescription() */
    TJobExecutionTarget executionTarget;
    CString threadPoolName;             /**< POOL: the pool of the runs (the run pool when the definition names none); MAIN_THREAD: the main thread pool */
    TJobOrigin origin;
    CString submittedBy;
    Int64 nextDueUtcMs;                 /**< 0 = none */
    CString waitingFor;                 /**< WAITING: lane, throttle limit name, hold, backoff */
    Int64 waitingUntilUtcMs;
    CString unavailableReason;
    UInt64 lastRunId;
    TJobRunState lastRunState;
    Int64 lastRunEndedUtcMs;
    UInt64 runCount;
    UInt64 failureCount;
    UInt32 consecutiveFailures;
    UInt32 activeRunCount;
    UInt32 flags;                       /**< EJobFlags */
    bool enabled;
    bool hasWorkSource;
    TJobControlActionSet remoteActions;
    CString::StringSet remoteCustomActions;
    TJobCustomActionVector customActions;   /**< declared by the job's work source */
    CDataNode payload;                  /**< only filled when the job has JOBFLAG_EXPOSE_PAYLOAD */
    UInt64 revision;

    SJobStatus( void );
};

/*-------------------------------------------------------------------------*/

typedef GUCEF::vector< SJobStatus > TJobStatusVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CJOBSTATUS_H ? */
