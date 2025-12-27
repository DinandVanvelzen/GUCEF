/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCORE_CASync.h"

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

CASync::CASyncChainState::CASyncChainState( const CString& threadPoolName )
    : CTSharedObjCreator< CASyncChainState, MT::CNoLock >( this )
    , m_threadPool()
    , m_tasks()
    , m_chainIsHealthy( true )
    , m_startRightAwayOnSubmit( false )
    , m_chainHasBeenSubmitted( false )
{
    const CString& threadPoolNameToAskFor = threadPoolName.IsNULLOrEmpty() ? CTaskManager::DefaultThreadPoolName : threadPoolName; 
    m_threadPool = CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( threadPoolNameToAskFor );
    m_chainIsHealthy = !m_threadPool.IsNULL();
}

/*-------------------------------------------------------------------------*/

CASync::CASyncChainState::CASyncChainState( ThreadPoolPtr threadPool )
    : CTSharedObjCreator< CASyncChainState, MT::CNoLock >( this )
    , m_threadPool( threadPool )
    , m_tasks()
    , m_chainIsHealthy( !threadPool.IsNULL() )
    , m_startRightAwayOnSubmit( false )
    , m_chainHasBeenSubmitted( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CASync::CASyncChainState::~CASyncChainState()
{GUCEF_TRACE;

    m_threadPool.Unlink();
    m_tasks.clear();
}

/*-------------------------------------------------------------------------*/

CASync::CASync( const CString& threadPoolName )
    : m_state()
{GUCEF_TRACE;

    m_state = CASyncChainState::CreateSharedObjWithParam( threadPoolName );
}

/*-------------------------------------------------------------------------*/

CASync::CASync( ThreadPoolPtr threadPool )
    : m_state()
{GUCEF_TRACE;

    m_state = CASyncChainState::CreateSharedObjWithParam( threadPool );
}

/*-------------------------------------------------------------------------*/

CASync::CASync( const CASync& thisChain )
    : m_state( thisChain.m_state )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CASync::ClearChain( void )
{GUCEF_TRACE;

    if ( !m_state.IsNULL() )
    {
        m_state->m_tasks.clear();
        m_state->m_chainIsHealthy = true;
        m_state->m_startRightAwayOnSubmit = false;
        m_state->m_chainHasBeenSubmitted = false;
    }    
}

/*-------------------------------------------------------------------------*/

CASync&
CASync::SetLastTaskStatus( TTaskStatus taskStatus )
{GUCEF_TRACE;

    CTaskPtr lastTask = GetLastTask();
    if ( !lastTask.IsNULL() )
    {
        lastTask->SetTaskStatus( taskStatus );
    }
    else
    {
        m_state->m_tasks.push_back( CTask::CreateSharedObjWithParam( taskStatus ) );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CTaskPtr
CASync::GetFirstErrorStateTask( void ) const
{GUCEF_TRACE;
    
    if ( !m_state.IsNULL() )
    {
        if ( !m_state->m_tasks.empty() )
        {
            for ( UInt32 i=0; i<m_state->m_tasks.size(); ++i )
            {
                const CTaskPtr& task = m_state->m_tasks[ i ];
                if ( task->IsTaskInErrorState() )
                {
                    return task;
                }
            }
        }
    }
    return CTaskPtr();
}

/*-------------------------------------------------------------------------*/

CTaskPtr
CASync::GetFirstTask( void ) const
{GUCEF_TRACE;

    if ( !m_state.IsNULL() )
    {
        if ( !m_state->m_tasks.empty() )
            return m_state->m_tasks.front();
    }
    return CTaskPtr();
}

/*-------------------------------------------------------------------------*/

CTaskPtr
CASync::GetLastTask( void ) const
{GUCEF_TRACE;

    if ( !m_state.IsNULL() )
    {
        if ( !m_state->m_tasks.empty() )
            return m_state->m_tasks.back();
    }
    return CTaskPtr();
}

/*-------------------------------------------------------------------------*/

bool
CASync::GetTaskIdsInChain( TTaskIdVector& taskIds ) const
{GUCEF_TRACE;

    taskIds.clear();

    if ( !m_state.IsNULL() )
    {
        bool totalSuccess = true;
        if ( !m_state->m_tasks.empty() )
        {
            for ( size_t i=0; i<m_state->m_tasks.size(); ++i )
            {
                const CTaskPtr& task = m_state->m_tasks[ i ];
                if ( !task.IsNULL() )
                {
                    taskIds.push_back( task->GetTaskId() );
                }
                else
                {
                    totalSuccess = false;
                }
            }
        }
        return totalSuccess;
    }
    return false;    
}

/*-------------------------------------------------------------------------*/

bool
CASync::SetNextTask( CTaskPtr& nextTask )
{GUCEF_TRACE;

    if ( !nextTask.IsNULL() )
    {
        TTaskIdVector taskIds;
        if ( GetTaskIdsInChain( taskIds ) )
        {
            taskIds.push_back( nextTask->GetTaskId() );

            if ( !CTask::ValidateTaskChainIdSequence( taskIds ) )
            {
                GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "ASync: Detected invalid task chain sequence when trying to set next task with Id " + ToString( taskIds.back() )
                               + " chain starts with task " + ToString( taskIds.front() ) ); 
                return false;
            }

            m_state->m_tasks.push_back( nextTask );

            CASyncChainState::TTaskPtrVector::iterator i = m_state->m_tasks.begin();
            while ( i != m_state->m_tasks.end() )
            {
                CTaskPtr& task = (*i);
                task->UpdateTaskChainIds( taskIds );
                ++i;
            }

            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CASync::TASyncChainStatePtr
CASync::GetChainState( void ) const
{GUCEF_TRACE;

    return m_state;
}

/*-------------------------------------------------------------------------*/

CASync&
CASync::QueueCallbackCommonImpl( CICloneable* taskData )
{GUCEF_TRACE;

    return Queue( CGenericCallbackTaskConsumer::TaskType ,
                  taskData                               ,
                  true                                   );
}

/*-------------------------------------------------------------------------*/

CASync&
CASync::ThenCallbackCommonImpl( CICloneable* taskData )
{GUCEF_TRACE;

    // In the context of a 'then' there should always be a prior task already
    CTaskPtr lastTask = GetLastTask();
    if ( lastTask.IsNULL() )
    {
        GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "ASync: Calling ThenCallback() as the initial call is invalid" );
        m_state->m_tasks.clear();
        m_state->m_chainIsHealthy = false;

        GUCEF_DELETE taskData;
        taskData = GUCEF_NULL;

        m_state->m_tasks.push_back( CTask::CreateSharedObjWithParam( TTaskStatus::TASKSTATUS_TASK_CHAINING_FAILED ) );
        return *this;
    }

    // the task object will take ownership of the task data going forward
    CTaskPtr newLastTask = m_state->m_threadPool->PrepTaskObj( CGenericCallbackTaskConsumer::TaskType ,
                                                               taskData                               ,
                                                               GUCEF_NULL                             ,
                                                               true                                   );
    if ( !newLastTask->IsTaskInErrorState() )
    {
        if ( SetNextTask( newLastTask ) )
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_BELOW_NORMAL, "ASync: Will chain task " + ToString( newLastTask->GetTaskId() ) +
                " to execute after task " + ToString( lastTask->GetTaskId() ) );
        }
        else
        {
            GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "ASync: Failed setting ThenCallback() based next task in chain with id " + ToString( newLastTask->GetTaskId() ) );
            SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASK_CHAINING_FAILED );
            newLastTask->SetTaskStatus( TTaskStatus::TASKSTATUS_TASK_CHAINING_FAILED );

            GUCEF_DELETE taskData;
            taskData = GUCEF_NULL;
        }
    }
    else
    {
        GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "ASync: Failed preparing ThenCallback() task object: " + newLastTask->GetTaskStatusString() );
        SetLastTaskStatus( newLastTask->GetTaskStatus() );

        GUCEF_DELETE taskData;
        taskData = GUCEF_NULL;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CASync&
CASync::Queue( const CString& taskType        ,
               CICloneable* taskData          ,
               bool assumeOwnershipOfTaskData )
{GUCEF_TRACE;

    // Based on the context of the 'queue' function being called we pass the last 'startRightAwayOnSubmit' flag
    // to NOT start the task chain right away on submit, using the StartTask() set of functions
    // but rather to use the QueueTask() set of functions on the thread pool
    return StartChain( taskType                  ,
                       taskData                  ,
                       assumeOwnershipOfTaskData ,
                       false                     );
}

/*-------------------------------------------------------------------------*/

CASync&
CASync::Start( const CString& taskType        ,
               CICloneable* taskData          ,
               bool assumeOwnershipOfTaskData )
{GUCEF_TRACE;

    // Based on the context of the 'Start' function being called we pass the last 'startRightAwayOnSubmit' flag
    // to START the task chain RIGHT AWAY on submit, using the StartTask() set of functions
    // instead of the QueueTask() set of functions on the thread pool which defer until pool thread availability
    return StartChain( taskType                  ,
                       taskData                  ,
                       assumeOwnershipOfTaskData ,
                       true                      );
}

/*-------------------------------------------------------------------------*/

CASync&
CASync::StartChain( const CString& taskType        ,
                    CICloneable* taskData          ,
                    bool assumeOwnershipOfTaskData ,
                    bool startRightAwayOnSubmit    )
{GUCEF_TRACE;

    if GUCEF_PREDICT_FALSE( m_state.IsNULL() )
    {
        return *this;
    }

    CTaskPtr lastTask = GetLastTask();
    if ( !lastTask.IsNULL() )
    {
        if ( !m_state->m_chainHasBeenSubmitted )
            GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "ASync: resetting chain with new Queue() call" );
        ClearChain();
    }

    // Based on the context of the 'queue' function being called we set the flag
    // to NOT start the task chain right away on submit, using the StartTask() set of functions
    // but rather to use the QueueTask() set of functions on the thread pool
    m_state->m_startRightAwayOnSubmit = startRightAwayOnSubmit;

    // the task object will take ownership of the task data going forward
    CTaskPtr newChainTask = m_state->m_threadPool->PrepTaskObj( taskType   ,
                                                                taskData   ,
                                                                GUCEF_NULL ,
                                                                true       );

    m_state->m_tasks.push_back( newChainTask );

    GUCEF_DEBUG_LOG( LOGLEVEL_BELOW_NORMAL, "ASync: Starting a new chain with first task " + ToString( newChainTask->GetTaskId() ) +
        " with startRightAwayOnSubmit=" + ToString( startRightAwayOnSubmit ) );

    return *this;
}

/*-------------------------------------------------------------------------*/

CFutureResult
CASync::Submit( void )
{GUCEF_TRACE;

    if GUCEF_PREDICT_FALSE( m_state.IsNULL() )
    {
        return TTaskStatus::TASKSTATUS_RESOURCE_NOT_AVAILABLE;
    }
    if ( !m_state->m_chainIsHealthy || m_state->m_threadPool.IsNULL() )
    {
        // There was an error during the formulation of the chain
        // as such we want the future result to reflect the earliest error that occurred
        // we will not actually submit the chain, it has 'no future'

        if ( !m_state->m_tasks.empty() )
        {
            CTaskPtr errorTask = GetFirstErrorStateTask();
            if ( !errorTask.IsNULL() )
            {
                return errorTask;
            }
        }

        return TTaskStatus::TASKSTATUS_RESOURCE_NOT_AVAILABLE;
    }

    // this chain has not been submitted yet (not a redundant Submit() call)
    // We will attempt to submit the task now
    if ( !m_state->m_chainHasBeenSubmitted && !m_state->m_tasks.empty() )
    {        
        if ( m_state->m_startRightAwayOnSubmit )
        {
            // Use the StartTaskChain() set of functions to force a start of the task chain right away
            CFutureResult future = m_state->m_tasks.size() > 1 ?
                                   m_state->m_threadPool->StartTaskChain( m_state->m_tasks ) :
                                   m_state->m_threadPool->StartTask( m_state->m_tasks.front() );
            if ( future.HasAFuture() )
            {
                m_state->m_chainHasBeenSubmitted = true;
                GUCEF_DEBUG_LOG( LOGLEVEL_BELOW_NORMAL, "ASync: Chain has been submitted to threadpool for immediate start" );
                return GetLastTask();
            }
            else
            {
                // There was an error starting the task chain
                GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ASync: Chain failed to submit to threadpool for immediate start" );
                return future;
            }
        }
        else
        {
            // Use the QueueTaskChain() set of functions to queue a start to the task chain        
            CFutureResult future = m_state->m_tasks.size() > 1 ?
                                   m_state->m_threadPool->QueueTaskChain( m_state->m_tasks ) :
                                   m_state->m_threadPool->QueueTask( m_state->m_tasks.front() );
            if ( future.HasAFuture() )
            {
                m_state->m_chainHasBeenSubmitted = true;
                GUCEF_DEBUG_LOG( LOGLEVEL_BELOW_NORMAL, "ASync: Chain has been submitted to threadpool for queueing" );
                return GetLastTask();
            }
            else
            {
                // There was an error starting the task chain
                GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ASync: Chain failed to submit to threadpool for queueing" );
                return future;
            }
        }
    }
    else
    {
        // Chain has already been submitted, return the existing future result
        return GetLastTask();
    }
}

/*-------------------------------------------------------------------------*/

CASync::operator CFutureResult()
{GUCEF_TRACE;

    return Submit();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
