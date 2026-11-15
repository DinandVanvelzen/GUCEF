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

#ifndef GUCEF_CORE_CJOBRUNRECORD_H
#define GUCEF_CORE_CJOBRUNRECORD_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_JOBSCHEDULERTYPES_H
#include "gucefCORE_JobSchedulerTypes.h"
#define GUCEF_CORE_JOBSCHEDULERTYPES_H
#endif /* GUCEF_CORE_JOBSCHEDULERTYPES_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

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
 *  One run of a job: one task of the task manager started for one occurrence (or one work item, one run-now) of a job.
 *  Fixed fields and bounded strings, so the run history needs no allocation per run after warm-up.
 */
struct GUCEF_CORE_PUBLIC_CPP SJobRunRecord
{
    static const UInt32 MaxNoteBytes = 256;
    static const UInt32 MaxOutcomeMessageBytes = 512;
    static const UInt32 MaxResultDocumentBytes = 1024;

    UInt64 runId;                       /**< per scheduler, increasing, never reused within the scheduler instance */
    UInt64 hostInstanceId;              /**< random at scheduler start; runId is unique together with it */
    CString jobId;
    CString itemKey;                    /**< work item key of a work source; private, never published */
    CString itemLabel;                  /**< what views may show of the work item; empty = withheld */
    CString taskTypeName;               /**< the task type of the run's task */
    TJobExecutionTarget executionTarget;
    CString threadPoolName;             /**< the pool the task was queued on (for MAIN_THREAD: the main thread pool) */
    TJobRunCause cause;
    Int64 scheduledUtcMs;
    Int64 queuedUtcMs;
    Int64 startedUtcMs;
    Int64 endedUtcMs;
    UInt32 attempt;
    TJobRunState state;
    UInt64 progressDone;                /**< 0 unless known: what the task reported (CTaskConsumer::ReportRunProgress()) */
    UInt64 progressTotal;               /**< 0 when unknown */
    CString note;                       /**< while running: the note of the last progress report; at the end the task's status extra info (CTask::GetTaskStatusExtraInfo()) when it has one; at most MaxNoteBytes */
    CString outcomeMessage;             /**< at most MaxOutcomeMessageBytes, through CRedactor::ForDisplay() for SENSITIVE jobs */
    CString requestedBy;                /**< principal subject for RUN_NOW, API and submitted jobs, empty otherwise */
    CString resultDocument;             /**< where the task wrote its result document (a VFS or file path, CTaskConsumer::ReportRunResultDocument()), empty when none; at most MaxResultDocumentBytes. The scheduler keeps the reference only, the document belongs to whoever reads it */
    UInt32 taskId;                      /**< the task manager task of the run, 0 while not started: links the two views */
    UInt32 actualCost;                  /**< reported to the throttle policy; the estimated cost unless a work source says otherwise */
    UInt64 revision;                    /**< scheduler revision of the last change of this record */

    SJobRunRecord( void );

    void Clear( void );
};

/*-------------------------------------------------------------------------*/

typedef GUCEF::vector< SJobRunRecord > TJobRunRecordVector;

/** Result data of the future of CJobScheduler::RunNowAsync(): the final record of the run */
typedef CTCloneableObj< SJobRunRecord > TCloneableJobRunRecord;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CJOBRUNRECORD_H ? */
