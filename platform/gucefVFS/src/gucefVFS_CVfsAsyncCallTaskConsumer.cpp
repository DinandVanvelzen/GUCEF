/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
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

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_VFS_CVFSASYNCCALLTASKDATA_H
#include "gucefVFS_CVfsAsyncCallTaskData.h"
#define GUCEF_VFS_CVFSASYNCCALLTASKDATA_H
#endif /* GUCEF_VFS_CVFSASYNCCALLTASKDATA_H ? */

#ifndef GUCEF_VFS_CVFSTASKCANCELSOURCE_H
#include "gucefVFS_CVfsTaskCancelSource.h"
#define GUCEF_VFS_CVFSTASKCANCELSOURCE_H
#endif /* GUCEF_VFS_CVFSTASKCANCELSOURCE_H ? */

#include "gucefVFS_CVfsAsyncCallTaskConsumer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CVfsAsyncCallTaskConsumer::TaskType = "GUCEF::VFS::CVfsAsyncCallTaskConsumer";
const UInt32 CVfsAsyncCallTaskConsumer::DefaultWorkerThreads = 4;

/*-------------------------------------------------------------------------*/

CVfsAsyncCallTaskConsumer::CVfsAsyncCallTaskConsumer( void )
    : CORE::CTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsAsyncCallTaskConsumer::~CVfsAsyncCallTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CVfsAsyncCallTaskConsumer::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

bool
CVfsAsyncCallTaskConsumer::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( task.IsNULL() )
        return true;

    // the call is run once: whatever it delivers is in the result that its caller holds, the future ends after this returns
    CVfsAsyncCallTaskData* data = static_cast< CVfsAsyncCallTaskData* >( task->GetTaskData() );
    if ( GUCEF_NULL != data && !data->call.IsNULL() )
    {
        CVfsTaskCancelSource cancel( task, this );
        try
        {
            data->call->Run( cancel );
        }
        catch ( const std::exception& e )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VfsAsyncCallTaskConsumer: An asynchronous call threw: " << e.what() );
        }
        catch ( ... )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VfsAsyncCallTaskConsumer: An asynchronous call threw" );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CVfsAsyncCallTaskConsumer::OnTaskEnded( CORE::CTaskPtr task, bool /* wasForced */ )
{GUCEF_TRACE;

    // the call goes with the task data; whoever queued it holds its own reference to the result
    if ( task.IsNULL() )
        return;
    CVfsAsyncCallTaskData* data = static_cast< CVfsAsyncCallTaskData* >( task->GetTaskData() );
    if ( GUCEF_NULL != data )
        data->call.Unlink();
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CVfsAsyncCallTaskConsumer::Queue( const CORE::CString& threadPoolName, UInt32 workerThreads, const CIVfsAsyncCallPtr& call )
{GUCEF_TRACE;

    if ( call.IsNULL() )
        return CORE::CFutureResult( CORE::TASKSTATUS_QUEUEING_FAILED );

    try
    {
        CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( threadPoolName );
        if ( pool.IsNULL() )
            return CORE::CFutureResult( CORE::TASKSTATUS_QUEUEING_FAILED );
        if ( 0 == pool->GetDesiredMinNrOfWorkerThreads() )
            pool->SetDesiredMinNrOfWorkerThreads( workerThreads < 1 ? 1 : workerThreads );

        CVfsAsyncCallTaskData taskData( call );
        return pool->QueueTask( TaskType, &taskData );
    }
    catch ( const timeout_exception& )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "VfsAsyncCallTaskConsumer: Failed to queue a call on the pool \"" << threadPoolName << "\": timeout" );
    }
    catch ( const std::exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VfsAsyncCallTaskConsumer: Failed to queue a call on the pool \"" << threadPoolName << "\": " << e.what() );
    }
    return CORE::CFutureResult( CORE::TASKSTATUS_QUEUEING_FAILED );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
