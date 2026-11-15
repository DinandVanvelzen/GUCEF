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

#ifndef GUCEF_CORE_CJOBSCHEDULERRUNLINK_H
#define GUCEF_CORE_CJOBSCHEDULERRUNLINK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_TASKSTATUS_H
#include "gucefCORE_TaskStatus.h"
#define GUCEF_CORE_TASKSTATUS_H
#endif /* GUCEF_CORE_TASKSTATUS_H ? */

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

class CJobScheduler;

/*-------------------------------------------------------------------------*/

/**
 *  Engine internal: how the task of a run of a job scheduler tells the scheduler that it ended.
 *
 *  Every run task carries a reference to the link of its scheduler (CTask, set before the task is queued). When the
 *  task ends, the thread pool hands the end state to the link from the thread that ended the task, after the pool
 *  let go of its own locks and of the task object (CThreadPool::TaskCleanup()). The link passes it on to its scheduler
 *  as long as the scheduler exists: the scheduler detaches the link when it is destroyed, so a task that outlives its
 *  scheduler (a run that ignored the stop timeout) ends without calling into a destroyed object, and the last
 *  reference to the scheduler is never let go of on a pool thread.
 *
 *  Lock order: the link lock, then the scheduler lock (inside CJobScheduler::OnRunTaskEnded(), bookkeeping only).
 *  The scheduler never calls into the link while it holds its own lock.
 */
class GUCEF_CORE_PRIVATE_CPP CJobSchedulerRunLink : public CTSharedPtrCreator< CJobSchedulerRunLink, MT::CMutex >
{
    public:

    typedef CTSharedPtrCreator< CJobSchedulerRunLink, MT::CMutex >::TSharedPtrType     TRunLinkPtr;

    CJobSchedulerRunLink( CJobScheduler* scheduler, const CString& schedulerName );

    virtual ~CJobSchedulerRunLink() GUCEF_VIRTUAL_OVERRIDE;

    /** Host name of the scheduler, for the task info of its run tasks */
    const CString& GetSchedulerName( void ) const;

    /** From the thread that ended the task, no lock of the pool held */
    void OnRunTaskEnded( UInt64 runId                 ,
                         UInt32 taskId                ,
                         TTaskStatus endStatus        ,
                         const CString& statusExtra   );

    /** From the thread of the run's task: progress of the run; false when the scheduler or the run is gone */
    bool OnRunProgress( UInt64 runId, UInt64 done, UInt64 total, const CString& note );

    /** From the thread of the run's task: the result document of the run; false when the scheduler or the run is gone */
    bool OnRunResultDocument( UInt64 runId, const CString& documentPath );

    /** The scheduler goes: no calls into it from now on. Waits for a call in progress */
    void Detach( void );

    private:

    CJobSchedulerRunLink( const CJobSchedulerRunLink& src );                /**< not implemented */
    CJobSchedulerRunLink& operator=( const CJobSchedulerRunLink& src );     /**< not implemented */

    private:

    MT::CMutex m_lock;
    CJobScheduler* m_scheduler;
    CString m_schedulerName;
};

/*-------------------------------------------------------------------------*/

typedef CJobSchedulerRunLink::TRunLinkPtr   CJobSchedulerRunLinkPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CJOBSCHEDULERRUNLINK_H ? */
