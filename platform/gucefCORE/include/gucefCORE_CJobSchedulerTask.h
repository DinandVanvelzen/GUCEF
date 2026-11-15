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

#ifndef GUCEF_CORE_CJOBSCHEDULERTASK_H
#define GUCEF_CORE_CJOBSCHEDULERTASK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

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
 *  Engine internal: the dispatcher of a job scheduler, one task on the scheduler's dispatcher pool with a thread of
 *  its own (CThreadPool::StartTaskWithConsumer()). Its delegator's periodic pulses are stopped at the start, so between
 *  cycles the thread sleeps the whole delay the scheduler asks for (RequestTaskCycleDelayInMs()) and wakes early only
 *  when another thread asks its pulse generator for a pulse (CJobScheduler::RequestDispatch()). It never polls.
 *  The runs of the jobs are not carried out by this task: they are tasks of their own task types.
 */
class GUCEF_CORE_PRIVATE_CPP CJobSchedulerTask : public CTaskConsumer
{
    public:

    static const CString TaskType;

    explicit CJobSchedulerTask( CJobScheduler* scheduler );

    virtual ~CJobSchedulerTask() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskStart( CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnding( CTaskPtr task, bool willBeForced ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnded( CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CJobSchedulerTask( const CJobSchedulerTask& src );              /**< not implemented */
    CJobSchedulerTask& operator=( const CJobSchedulerTask& src );   /**< not implemented */

    private:

    CJobScheduler* m_scheduler;     /**< outlives the task: CJobScheduler::Stop() waits for it */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CJOBSCHEDULERTASK_H ? */
