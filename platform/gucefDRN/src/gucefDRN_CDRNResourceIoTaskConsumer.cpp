/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCEIOTASKDATA_H
#include "gucefDRN_CDRNResourceIoTaskData.h"
#define GUCEF_DRN_CDRNRESOURCEIOTASKDATA_H
#endif /* GUCEF_DRN_CDRNRESOURCEIOTASKDATA_H ? */

#include "gucefDRN_CDRNResourceIoTaskConsumer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CDRNResourceIoTaskConsumer::TaskType = "GUCEF::DRN::CDRNResourceIoTaskConsumer";

/*-------------------------------------------------------------------------*/

CDRNResourceIoTaskConsumer::CDRNResourceIoTaskConsumer( void )
    : CORE::CTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNResourceIoTaskConsumer::~CDRNResourceIoTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNResourceIoTaskConsumer::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceIoTaskConsumer::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( task.IsNULL() )
        return true;
    CDRNResourceIoTaskData* data = static_cast< CDRNResourceIoTaskData* >( task->GetTaskData() );
    if ( GUCEF_NULL != data && !data->job.IsNULL() )
        data->job->Execute();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceIoTaskConsumer::OnTaskEnded( CORE::CTaskPtr task, bool /* wasForced */ )
{GUCEF_TRACE;

    // the job goes with the task data; whoever queued it holds its own reference to the results
    if ( task.IsNULL() )
        return;
    CDRNResourceIoTaskData* data = static_cast< CDRNResourceIoTaskData* >( task->GetTaskData() );
    if ( GUCEF_NULL != data )
    {
        // the delegator set the end state of the task before it calls this: the future of the job is ready, a node that
        // is woken now finds the result. A wake from inside the job could be taken by a cycle that finds it not ready
        if ( !data->job.IsNULL() )
            data->job->SignalCompleted();
        data->job.Unlink();
    }
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CDRNResourceIoTaskConsumer::Queue( const CORE::CString& threadPoolName, const CIDRNResourceIoJobPtr& job )
{GUCEF_TRACE;

    return Queue( threadPoolName, job, CORE::PulseGeneratorPtr() );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CDRNResourceIoTaskConsumer::Queue( const CORE::CString& threadPoolName, const CIDRNResourceIoJobPtr& job, const CORE::PulseGeneratorPtr& wakeOnCompletion )
{GUCEF_TRACE;

    // before the job is queued: the pool thread reads it once the job ended
    CIDRNResourceIoJobPtr queuedJob( job );
    if ( !queuedJob.IsNULL() )
        queuedJob->SetCompletionPulseGenerator( wakeOnCompletion );

    const CORE::CString& poolName = threadPoolName.IsNULLOrEmpty() ? CORE::CTaskManager::DefaultThreadPoolName : threadPoolName;
    CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( poolName );
    if ( threadPool.IsNULL() )
        return CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
    CDRNResourceIoTaskData taskData( job );
    return threadPool->QueueTask( TaskType, &taskData );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
