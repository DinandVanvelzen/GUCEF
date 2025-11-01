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
    , m_lastTask()
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
    , m_lastTask()
    , m_chainIsHealthy( !threadPool.IsNULL() )
    , m_startRightAwayOnSubmit( false )
    , m_chainHasBeenSubmitted( false )
{GUCEF_TRACE;

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

    if ( !m_state->m_lastTask.IsNULL() )
    {
        if ( !m_state->m_chainHasBeenSubmitted )
            CTask::BreakApartTaskChain( m_state->m_lastTask );
        m_state->m_lastTask.Unlink();
        m_state->m_chainIsHealthy = true;
        m_state->m_startRightAwayOnSubmit = false;
        m_state->m_chainHasBeenSubmitted = false;
    }    
}

/*-------------------------------------------------------------------------*/

CASync&
CASync::SetLastTaskStatus( TTaskStatus taskStatus )
{GUCEF_TRACE;

    if ( m_state->m_lastTask.IsNULL() )
    {
        m_state->m_lastTask = CTask::CreateSharedObjWithParam( taskStatus );
    }
    else
    {
        m_state->m_lastTask->SetTaskStatus( taskStatus );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CTaskPtr
CASync::GetLastTask( void ) const
{GUCEF_TRACE;

    return m_state->m_lastTask;
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
    if ( m_state->m_lastTask.IsNULL() )
    {
        GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "ASync: Calling ThenCallback() as the initial call is invalid" );
        CTask::BreakApartTaskChain( m_state->m_lastTask );
        m_state->m_lastTask.Unlink();
        m_state->m_chainIsHealthy = false;

        GUCEF_DELETE taskData;
        taskData = GUCEF_NULL;

        m_state->m_lastTask = CTask::CreateSharedObjWithParam( TTaskStatus::TASKSTATUS_TASK_CHAINING_FAILED );
        return *this;
    }

    // the task object will take ownership of the task data going forward
    CTaskPtr newLastTask = m_state->m_threadPool->PrepTaskObj( CGenericCallbackTaskConsumer::TaskType ,
                                                               taskData                               ,
                                                               GUCEF_NULL                             ,
                                                               true                                   );
    if ( !newLastTask->IsTaskInErrorState() )
    {
        if ( m_state->m_lastTask->SetNextTask( newLastTask ) )
        {
            // Update our own reference to the tail end of the chain for further additions
            GUCEF_DEBUG_LOG( LOGLEVEL_BELOW_NORMAL, "ASync: Will chain task " + ToString( newLastTask->GetTaskId() ) +
                " to execute after task " + ToString( m_state->m_lastTask->GetTaskId() ) );

            m_state->m_lastTask = newLastTask;
        }
        else
        {
            GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "ASync: Failed setting ThenCallback() next task in chain" );
            m_state->m_lastTask->SetTaskStatus( TTaskStatus::TASKSTATUS_TASK_CHAINING_FAILED );
        }
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

    if ( !m_state->m_lastTask.IsNULL() )
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
    CTaskPtr newChainTask = m_state->m_threadPool->PrepTaskObj( CGenericCallbackTaskConsumer::TaskType ,
                                                                taskData                               ,
                                                                GUCEF_NULL                             ,
                                                                true                                   );

    m_state->m_lastTask = newChainTask;

    return *this;
}

/*-------------------------------------------------------------------------*/

CFutureResult
CASync::Submit( void )
{GUCEF_TRACE;

    if ( !m_state->m_chainIsHealthy || m_state->m_threadPool.IsNULL() )
    {
        // There was an error during the formulation of the chain
        // as such we want the future result to reflect the earliest error that occurred
        // we will not actually submit the chain, it has 'no future'
        if ( !m_state->m_lastTask.IsNULL() )
            return m_state->m_lastTask->GetFirstErrorStateTask();
        else
            return CTask::CreateSharedObjWithParam( TTaskStatus::TASKSTATUS_RESOURCE_NOT_AVAILABLE );
    }

    if ( !m_state->m_chainHasBeenSubmitted )
    {
        // this chain has not been submitted yet (not a redundant Submit() call)
        // We will attempt to submit the task now

        CTaskPtr firstTask = m_state->m_lastTask->GetFirstTaskInChain();
        if ( m_state->m_startRightAwayOnSubmit )
        {
            // Use the StartTask() set of functions to force a start of the task chain right away
            CFutureResult future = m_state->m_threadPool->StartTask( firstTask );
            if ( future.HasAFuture() )
            {
                m_state->m_chainHasBeenSubmitted = true;
                return CFutureResult( m_state->m_lastTask );
            }
            else
            {
                // There was an error starting the task chain
                return future;
            }
        }
        else
        {
            // Use the QueueTask() set of functions to queue a start to the task chain        
            CFutureResult future = m_state->m_threadPool->QueueTask( firstTask );
            if ( future.HasAFuture() )
            {
                m_state->m_chainHasBeenSubmitted = true;
                return CFutureResult( m_state->m_lastTask );
            }
            else
            {
                // There was an error starting the task chain
                return future;
            }
        }
    }
    else
    {
        // Chain has already been submitted, return the existing future result
        return CFutureResult( m_state->m_lastTask );
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
