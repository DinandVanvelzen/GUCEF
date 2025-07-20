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

#include "gucefCORE_CTask.h"

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


CTask::CTask( TTaskStatus taskStatus )
    : CNotifier()
    , CTSharedObjCreator< CTask, MT::CMutex >( this )
    , m_taskData( GUCEF_NULL )
    , m_serializedTaskData()
    , m_taskType()
    , m_taskConsumer()
    , m_assumedOwnershipOfTaskData( false )
    , m_taskId()
    , m_taskStatus( taskStatus )
    , m_nextTask()
    , m_priorTask()
{GUCEF_TRACE;

    // Obtain a globally unique task id
    // Essentially the 'handle' by which the task can be referenced
    if ( !CCoreGlobal::Instance()->GetTaskManager().TryGetGlobalTaskId( m_taskId ) )
    {
        // Set status to an error but only if the input was not already an error condition
        // in which case the task object is just a conveyance for error info
        if ( !IsTaskInErrorState() )
            m_taskStatus = TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED;
    }

    m_serializedTaskData.SetNodeType( GUCEF_DATATYPE_UNKNOWN );
}

/*-------------------------------------------------------------------------*/

CTask::~CTask()
{GUCEF_TRACE;

    m_serializedTaskData.Clear();
}

/*-------------------------------------------------------------------------*/

void
CTask::SetTaskConsumer( CTaskConsumerPtr taskConsumer )
{GUCEF_TRACE;

    m_taskConsumer = taskConsumer;
}

/*-------------------------------------------------------------------------*/

CTaskConsumerPtr
CTask::GetTaskConsumer( void ) const
{GUCEF_TRACE;
                 
    return m_taskConsumer;
}

/*-------------------------------------------------------------------------*/

const CString&
CTask::GetTaskType( void ) const
{GUCEF_TRACE;

    return m_taskType;
}

/*-------------------------------------------------------------------------*/

void
CTask::SetTaskData( CICloneable* taskData, bool assumeOwnershipOfTaskData )
{GUCEF_TRACE;

    // First clean up what we had before
    if ( GUCEF_NULL != m_taskData && m_assumedOwnershipOfTaskData )
    {
        GUCEF_DELETE m_taskData;
        m_taskData = GUCEF_NULL;
        m_assumedOwnershipOfTaskData = false;
    }

    if ( !assumeOwnershipOfTaskData && GUCEF_NULL != taskData )
    {
        m_taskData = taskData = taskData->Clone();
    }
    else
    {
        m_taskData = taskData;
    }
    m_assumedOwnershipOfTaskData = assumeOwnershipOfTaskData;
}

/*-------------------------------------------------------------------------*/

CICloneable*
CTask::GetTaskData( void ) const
{GUCEF_TRACE;

    return m_taskData;
}

/*-------------------------------------------------------------------------*/

bool
CTask::HasTaskData( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_taskData;
}

/*-------------------------------------------------------------------------*/

CTask::TIntegerTypeUsedForTaskId
CTask::GetTaskId( void ) const
{GUCEF_TRACE;

    return m_taskId;
}

/*-------------------------------------------------------------------------*/

bool
CTask::HadAssumedOwnershipOfTaskData( void ) const
{GUCEF_TRACE;

    return m_assumedOwnershipOfTaskData;
}

/*-------------------------------------------------------------------------*/

bool
CTask::IsTaskPartOfAChain( void ) const
{GUCEF_TRACE;

    return !m_nextTask.IsNULL() || !m_priorTask.IsNULL();
}

/*-------------------------------------------------------------------------*/

bool
CTask::IsLastTaskInAChain( void ) const
{GUCEF_TRACE;

    if ( IsTaskPartOfAChain() )
        return m_nextTask.IsNULL() && !m_priorTask.IsNULL();
    else
        return true;
}

/*-------------------------------------------------------------------------*/

void
CTask::SetTaskStatus( TTaskStatus newStatus )
{GUCEF_TRACE;

    m_taskStatus = newStatus;

    CTaskPtr nextTask = m_nextTask;
    if ( !nextTask.IsNULL() )
    {
        // Propagate the status change to the next task in the chain
        // Downstream chain states are really just for informational purposes
        // However we want them to have a chain-like feel when viewed in any sort of task manager
        switch ( newStatus )
        {
            case TTaskStatus::TASKSTATUS_SETUP:
            {
                if ( !nextTask->IsTaskInErrorState() )
                    nextTask->SetTaskStatus( newStatus );
                break;
            }
            case TTaskStatus::TASKSTATUS_STARTUP:
            case TTaskStatus::TASKSTATUS_RUNNING:
            {
                if ( !nextTask->IsTaskInErrorState() )
                    nextTask->SetTaskStatus( TTaskStatus::TASKSTATUS_QUEUED );
                break;
            }
            case TTaskStatus::TASKSTATUS_STOPPED:
            case TTaskStatus::TASKSTATUS_KILLED:
            {
                if ( !nextTask->IsTaskInErrorState() )
                    nextTask->SetTaskStatus( newStatus );
                break;
            }
        }        
    }
}

/*-------------------------------------------------------------------------*/

TTaskStatus
CTask::GetTaskStatus( void ) const
{GUCEF_TRACE;

    return m_taskStatus;
}

/*-------------------------------------------------------------------------*/

CString
CTask::GetTaskStatusString( void ) const
{GUCEF_TRACE;

    return TaskStatusToTaskStatusString( m_taskStatus );
}

/*-------------------------------------------------------------------------*/

UInt32
CTask::GetThreadId( void ) const
{GUCEF_TRACE;

    CTaskConsumerPtr taskConsumer = m_taskConsumer;
    if ( !taskConsumer.IsNULL() )
    {
        return taskConsumer->GetDelegatorThreadId();
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

CTask::TTaskDelegatorBasicPtr
CTask::GetDelegator( void ) const
{GUCEF_TRACE;

    CTaskConsumerPtr taskConsumer = m_taskConsumer;
    if ( !taskConsumer.IsNULL() )
    {
        return taskConsumer->GetTaskDelegator();
    }
    return TTaskDelegatorBasicPtr();
}

/*-------------------------------------------------------------------------*/

bool
CTask::Init( const CString& taskType         ,
             CTaskConsumerPtr taskConsumer   ,
             CICloneable* taskData           ,
             bool assumedOwnershipOfTaskData ,
             CDataNode* serializedTaskData   ,
             TTaskStatus taskStatus          )
{GUCEF_TRACE;

    // Sanity check
    if ( taskType.IsNULLOrEmpty() )
    {
        return false;
    }

    m_taskType = taskType;
    m_taskConsumer = taskConsumer;
    SetTaskData( taskData, assumedOwnershipOfTaskData );
    m_taskStatus = taskStatus;

    if ( GUCEF_NULL != serializedTaskData )
    {
        // create a deep copy of the DOM tree
        m_serializedTaskData.Clear();
        m_serializedTaskData = *serializedTaskData;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

void
CTask::Clear( void )
{GUCEF_TRACE;

    m_taskType.Clear();
    m_taskConsumer.Unlink();
    m_taskData = GUCEF_NULL;
    m_assumedOwnershipOfTaskData = false;
    m_taskStatus = TTaskStatus::TASKSTATUS_UNDEFINED;
    m_serializedTaskData.Clear();
    m_serializedTaskData.SetNodeType( GUCEF_DATATYPE_UNKNOWN );
}

/*-------------------------------------------------------------------------*/

bool
CTask::GetSerializedTaskDataCopy( CDataNode& domNode                                      ,
                                  const CDataNodeSerializableSettings& serializerSettings ) const
{GUCEF_TRACE;

    if ( m_serializedTaskData.GetNodeType() != GUCEF_DATATYPE_UNKNOWN )
    {
        // Deep copy the original task data dom
        domNode = m_serializedTaskData;
        return true;
    }

    // It is also possible that while we were handed the instantiated data object instead of a serialized payload
    // that the data object is in fact serializable
    if ( GUCEF_NULL != m_taskData )
    {
        if ( CCoreGlobal::Instance()->GetTaskManager().IsCustomTaskDataForTaskTypeSerializable( m_taskType ) )
        {
            const CIDataNodeSerializable* serializableTaskData = static_cast< const CIDataNodeSerializable* >( m_taskData );
            if ( serializableTaskData->Serialize( domNode, serializerSettings ) )
            {
                return true;
            }
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CTask::IsTaskInEndState( void ) const
{GUCEF_TRACE;

    return TaskStatusIsAnEndState( m_taskStatus );
}

/*-------------------------------------------------------------------------*/

bool
CTask::IsTaskInErrorState( void ) const
{GUCEF_TRACE;

    return TaskStatusIsAnError( m_taskStatus );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CTask::WaitForTaskToFinish( Int32 timeoutInMs ) const
{GUCEF_TRACE;

    UInt64 startTicks = MT::PrecisionTickCount();
    while ( !m_taskConsumer.IsNULL() && !IsTaskInEndState() )
    {
        MT::PrecisionDelay( 10 );

        UInt64 ticksNow = MT::PrecisionTickCount();
        if ( timeoutInMs > 0 )
            if ( MT::PrecisionTimerTicksToMs( ticksNow - startTicks ) >= timeoutInMs )
                return MT::TLockStatus::LOCKSTATUS_WAIT_TIMEOUT;
    }
    return MT::TLockStatus::LOCKSTATUS_OPERATION_SUCCESS;
}

/*-------------------------------------------------------------------------*/

CTaskPtr
CTask::GetFirstTaskInChain( void ) const
{GUCEF_TRACE;

    if ( m_priorTask.IsNULL() )
        return CreateSharedPtr(); // return this as a shared pointer

    CTaskPtr rootTask = CreateSharedPtr();
    while ( !rootTask.IsNULL() && !rootTask->m_priorTask.IsNULL() )
    {
        rootTask = rootTask->m_priorTask;
    }

    return rootTask;
}

/*-------------------------------------------------------------------------*/

CTaskPtr
CTask::GetLastTaskInChain( void ) const
{GUCEF_TRACE;

    if ( m_nextTask.IsNULL() )
        return CreateSharedPtr(); // return this as a shared pointer

    CTaskPtr finalTask = CreateSharedPtr();
    while ( !finalTask.IsNULL() && !finalTask->m_nextTask.IsNULL() )
    {
        finalTask = finalTask->m_nextTask;
    }

    return finalTask;
}

/*-------------------------------------------------------------------------*/

void
CTask::BreakApartTaskChain( CTaskPtr task )
{GUCEF_TRACE;

    // Break the double linked list
    // This should only leave the individual references outstanding which helps advance towards deletion

    if ( task.IsNULL() )
        return;

    task = task->GetFirstTaskInChain();

    while ( !task->m_nextTask.IsNULL() )
    {
        CTaskPtr nextTask = task->m_nextTask;
        task->m_nextTask.Unlink();
        nextTask->m_priorTask.Unlink();
        task = nextTask;
    }
}

/*-------------------------------------------------------------------------*/

CTaskPtr
CTask::GetFirstErrorStateTask( void ) const
{GUCEF_TRACE;

    CTaskPtr task = GetFirstTaskInChain();
    if ( task.IsNULL() )
        return CTask::CreateSharedObjWithParam( TTaskStatus::TASKSTATUS_RESOURCE_NOT_AVAILABLE );

    while ( !task->IsTaskInErrorState() )
    {
        CTaskPtr nextTask = task->GetNextTask();
        if ( !nextTask.IsNULL() )
            task = nextTask;
        else
            return task;
    }
    return task;
}

/*-------------------------------------------------------------------------*/

bool
CTask::SetNextTask( CTaskPtr nextTask )
{GUCEF_TRACE;

    if ( nextTask.IsNULL() )
    {
        if ( !m_nextTask.IsNULL() && !m_nextTask->m_nextTask.IsNULL() )
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "Task:SetNextTask: Illegal attempt to break chain since next task (id=" +
                ToString( m_nextTask->GetTaskId() ) + ") has a 'next' task (id=" +
                ToString( m_nextTask->m_nextTask->GetTaskId() ) + ")" );
            return false;
        }

        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "Task:SetNextTask: Removing last link in the chain (id=" +
            ToString( m_nextTask->GetTaskId() ) + ") which was linked to this task (id=" +
            ToString( GetTaskId() ) + ")" );

        m_nextTask->m_priorTask.Unlink();
        m_nextTask.Unlink();
        return true;
    }

    // Sanity check on the next task being eligible to be next in a chain
    if ( !nextTask->m_priorTask.IsNULL() )
    {
        // the task is already part of a chain, it has a prior segment
        // you are not allowed to fork chains
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "Task:SetNextTask: Illegal attempt to add a task as 'next' (id=" +
            ToString( nextTask->GetTaskId() ) + ") which already has 'prior' tasks itself (id=" +
            ToString( nextTask->m_priorTask->GetTaskId() ) + ")" );
        return false;
    }

    if ( !nextTask->m_nextTask.IsNULL() )
    {
        // Currently not allowed to add more than 1 segment at a time
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "Task:SetNextTask: Unsupported attempt to add a task as 'next' (id=" +
            ToString( nextTask->GetTaskId() ) + ") which already has 'next' tasks itself" );
        return false;
    }

    // Validate that adding the new task to the chain does not create a loop
    CTaskPtr task = GetFirstTaskInChain();
    while ( !task.IsNULL() )
    {
        if ( task->m_taskId == nextTask->m_taskId )
        {
            // not allowed as this creates a loop
            // all tasks in the chain must be uniquely linked
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "Task:SetNextTask: Illegal attempt to add a task as 'next' (id=" +
                ToString( nextTask->GetTaskId() ) + ") even though there is already such a task in the chain" );
            return false;
        }

        task = task->m_nextTask;
    }

    GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "Task:SetNextTask: Set next link in the chain (id=" +
        ToString( nextTask->GetTaskId() ) + ") linked to this task (id=" +
        ToString( GetTaskId() ) + ")" );

    m_nextTask = nextTask;
    m_nextTask->m_priorTask = CreateSharedPtr();
    return true;
}

/*-------------------------------------------------------------------------*/

CTaskPtr
CTask::GetNextTask( void ) const
{GUCEF_TRACE;

    return m_nextTask;
}

/*-------------------------------------------------------------------------*/

CTaskPtr
CTask::GetPriorTask( void ) const
{GUCEF_TRACE;

    return m_priorTask;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
