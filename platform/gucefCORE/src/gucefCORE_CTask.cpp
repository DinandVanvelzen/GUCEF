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
    , m_threadPool()
    , m_taskConsumer()
    , m_assumedOwnershipOfTaskData( false )
    , m_taskId()
    , m_taskStatus( taskStatus )
    , m_taskStatusExtraInfo()
    , m_chainTasks()
    , m_lock()
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

CTask::CTask( const CTask& src )
    : CNotifier( src )
    , CTSharedObjCreator< CTask, MT::CMutex >( src , this )
    , m_taskData( GUCEF_NULL )
    , m_serializedTaskData( src.m_serializedTaskData )
    , m_taskType( src.m_taskType )
    , m_threadPool( src.m_threadPool )
    , m_taskConsumer( src.m_taskConsumer )
    , m_assumedOwnershipOfTaskData( false )
    , m_taskId()
    , m_taskStatus( src.m_taskStatus )
    , m_taskStatusExtraInfo( src.m_taskStatusExtraInfo )
    , m_chainTasks( src.m_chainTasks )
    , m_lock()
{GUCEF_TRACE;

    SetTaskData( src.m_taskData, false );
}

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_RVALUE_REFERENCES_SUPPORTED

CTask::CTask( CTask&& src ) GUCEF_NOEXCEPT
    : CNotifier( GUCEF_MOVE( src ) )
    , CTSharedObjCreator< CTask, MT::CMutex >( GUCEF_MOVE( src ), this )
    , m_taskData( src.m_taskData )
    , m_serializedTaskData( GUCEF_MOVE( src.m_serializedTaskData ) )
    , m_taskType( GUCEF_MOVE( src.m_taskType ) )
    , m_threadPool( GUCEF_MOVE( src.m_threadPool ) )
    , m_taskConsumer( GUCEF_MOVE( src.m_taskConsumer ) )
    , m_assumedOwnershipOfTaskData( src.m_assumedOwnershipOfTaskData )
    , m_taskId( GUCEF_MOVE( src.m_taskId ) )
    , m_taskStatus( src.m_taskStatus )
    , m_taskStatusExtraInfo( GUCEF_MOVE( src.m_taskStatusExtraInfo ) )
    , m_chainTasks( GUCEF_MOVE( src.m_chainTasks ) )
    , m_lock( GUCEF_MOVE( src.m_lock ) )
{GUCEF_TRACE;

    // Leave the source in a valid state
    src.m_taskData = GUCEF_NULL;
    src.m_assumedOwnershipOfTaskData = false;
}
#endif

/*-------------------------------------------------------------------------*/

CTask::~CTask()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

void
CTask::SetTaskConsumer( CTaskConsumerPtr taskConsumer )
{GUCEF_TRACE;

    // shared pointers have their own assignment locks so copying them is thread safe
    // no need for extra locks here

    m_taskConsumer = taskConsumer;
}

/*-------------------------------------------------------------------------*/

CTaskConsumerPtr
CTask::GetTaskConsumer( void ) const
{GUCEF_TRACE;

    // shared pointers have their own assignment locks so copying them is thread safe
    // no need for extra locks here

    return m_taskConsumer;
}

/*-------------------------------------------------------------------------*/

const CString&
CTask::GetTaskType( void ) const
{GUCEF_TRACE;

    // for performance this is designed to be immutable after construction so no lock needed
    // obviously the task object itself needs to remain alive for this to be valid

    return m_taskType;
}

/*-------------------------------------------------------------------------*/

void
CTask::SetTaskData( CICloneable* taskData, bool assumeOwnershipOfTaskData )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

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

    MT::CScopeMutex lock( m_lock );
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

    MT::CScopeMutex lock( m_lock );
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

    MT::CScopeMutex lock( m_lock );
    return !m_chainTasks.empty();
}

/*-------------------------------------------------------------------------*/

bool
CTask::IsLastTaskInAChain( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    if ( !m_chainTasks.empty() )
    {
        return m_taskId == m_chainTasks.back();
    }
    else
    {
        return true;
    }
}

/*-------------------------------------------------------------------------*/

bool
CTask::GetAllTasksInChain( TTaskPtrVector& tasks ) const
{GUCEF_TRACE;

    tasks.clear();

    MT::CScopeMutex lock( m_lock );

    // first check if this is even a chained task
    // if not we can just return ourselves
    if ( m_chainTasks.empty() )
    {
        tasks.push_back( CreateBasicSharedPtr() );
        return true;
    }
    else
    {
        bool totalSuccess = true;
        tasks.reserve( m_chainTasks.size() );

        CThreadPoolPtr threadPool = GetThreadPool();  
        for ( UInt32 i=0; i<m_chainTasks.size(); ++i )
        {
            CTaskPtr task = threadPool->GetTaskObjById( m_chainTasks[ i ] );
            if ( !task.IsNULL() )
            {
                tasks.push_back( task );
            }
            else
            {
                totalSuccess = false;
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Task:GetAllTasksInChain: Failed to resolve chain task obj from id " + ToString( m_chainTasks[i] ) + " in chain for task with id " + ToString( m_taskId ) );
            }
        }
        return totalSuccess;
    }
}

/*-------------------------------------------------------------------------*/

bool
CTask::GetAllTasksInChain( TTaskPtrSet& taskSet ) const
{GUCEF_TRACE;

    TTaskPtrVector tasks;
    bool success = GetAllTasksInChain( tasks );
    for ( UInt32 i=0; i<tasks.size(); ++i )
    {
        taskSet.insert( tasks[ i ] );
    }
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CTask::GetAllUpcomingTasksInChain( TTaskPtrSet& taskSet ) const
{GUCEF_TRACE;

    taskSet.clear();

    MT::CScopeMutex lock( m_lock );

    // first check if this is even a chained task
    // if not we can just return ourselves
    if ( m_chainTasks.empty() )
    {
        // no chain, so no upcoming tasks
        return true;
    }
    else
    {
        bool totalSuccess = true;

        CThreadPoolPtr threadPool = GetThreadPool();  
        for ( UInt32 i=0; i<m_chainTasks.size(); ++i )
        {
            if ( m_taskId == m_chainTasks[ i ] )
            {
                for ( UInt32 n=i+1; n<m_chainTasks.size(); ++n )
                {
                    CTaskPtr task = threadPool->GetTaskObjById( m_chainTasks[ i ] );
                    if ( !task.IsNULL() )
                    {
                        taskSet.insert( task );
                    }
                    else
                    {
                        totalSuccess = false;
                        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Task:GetAllUpcomingTasksInChain: Failed to resolve chain task obj from id " + ToString( m_chainTasks[i] ) + " in chain for task with id " + ToString( m_taskId ) );
                    }
                }
                break;
            }
        }
        return totalSuccess;
    }
}

/*-------------------------------------------------------------------------*/

void
CTask::SetTaskStatus( TTaskStatus newStatus )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        m_taskStatus = newStatus;
    }

    CTaskPtr nextTask = GetNextTask();
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
            default:
            {
                // by default don't propagate the status
                break;
            }
        }        
    }
}

/*-------------------------------------------------------------------------*/

TTaskStatus
CTask::GetTaskStatus( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_taskStatus;
}

/*-------------------------------------------------------------------------*/

CString
CTask::GetTaskStatusString( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );    
    return TaskStatusToTaskStatusString( m_taskStatus );
}

/*-------------------------------------------------------------------------*/

void
CTask::SetTaskStatusExtraInfo( const CString& extraInfo )
{GUCEF_TRACE;
                                
    MT::CScopeMutex lock( m_lock );
    m_taskStatusExtraInfo = extraInfo;
}

/*-------------------------------------------------------------------------*/

CString
CTask::GetTaskStatusExtraInfo( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_taskStatusExtraInfo;
}

/*-------------------------------------------------------------------------*/

UInt32
CTask::GetThreadId( void ) const
{GUCEF_TRACE;

    // shared pointers have their own assignment locks so copying them is thread safe
    // no need for extra locks here

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

    // shared pointers have their own assignment locks so copying them is thread safe
    // no need for extra locks here

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
             TTaskStatus taskStatus          ,
             ThreadPoolPtr threadPool        )
{GUCEF_TRACE;

    // Sanity check
    if ( taskType.IsNULLOrEmpty() )
    {
        return false;
    }

    MT::CScopeMutex lock( m_lock );

    m_taskType = taskType;
    m_taskConsumer = taskConsumer;
    SetTaskData( taskData, assumedOwnershipOfTaskData );
    m_taskStatus = taskStatus;
    m_threadPool = threadPool; 

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

    MT::CScopeMutex lock( m_lock );

    SetTaskData( GUCEF_NULL, false );
    m_taskStatus = TTaskStatus::TASKSTATUS_UNDEFINED;
    m_taskStatusExtraInfo.Clear();
    m_serializedTaskData.Clear();
    m_serializedTaskData.SetNodeType( GUCEF_DATATYPE_UNKNOWN );
    m_chainTasks.clear();
    m_taskType.Clear();
    m_taskConsumer.Unlink();
    m_threadPool.Unlink();        
}
                                                                                                                                                                                                                                     
/*-------------------------------------------------------------------------*/

bool
CTask::GetSerializedTaskDataCopy( CDataNode& domNode                                      ,
                                  const CDataNodeSerializableSettings& serializerSettings ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

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

    MT::CScopeMutex lock( m_lock );
    return TaskStatusIsAnEndState( m_taskStatus );
}

/*-------------------------------------------------------------------------*/

bool
CTask::IsTaskInErrorState( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return TaskStatusIsAnError( m_taskStatus );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CTask::WaitForTaskToFinish( Int32 timeoutInMs ) const
{GUCEF_TRACE;

    UInt64 startTicks = MT::PrecisionTickCount();
    while ( !IsTaskInEndState() )
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

    MT::CScopeMutex lock( m_lock );

    if ( !m_chainTasks.empty() )
    {
        CThreadPoolPtr threadPool = GetThreadPool();
        if ( !threadPool.IsNULL() )
        {
            TIntegerTypeUsedForTaskId firstTaskId = m_chainTasks.front();
            if ( GetTaskId() != firstTaskId )
            {
                CTaskPtr task = threadPool->GetTaskObjById( firstTaskId );
                return task;
            }

            // This task is the first in the chain
            return CreateSharedPtr();
        }
        return CTaskPtr();
    }

    // this task is alone in its chain hence always the first
    return CreateSharedPtr();
}

/*-------------------------------------------------------------------------*/

CTaskPtr
CTask::GetLastTaskInChain( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    if ( !m_chainTasks.empty() )
    {
        CThreadPoolPtr threadPool = GetThreadPool();
        if ( !threadPool.IsNULL() )
        {
            TIntegerTypeUsedForTaskId lastTaskId = m_chainTasks.back();
            if ( GetTaskId() != lastTaskId )
            {
                CTaskPtr task = threadPool->GetTaskObjById( lastTaskId );
                return task;
            }

            // This task is the last in the chain
            return CreateSharedPtr();
        }
        return CTaskPtr();
    }

    // this task is alone in its chain hence always the last
    return CreateSharedPtr();
}

/*-------------------------------------------------------------------------*/

CTaskPtr
CTask::GetFirstErrorStateTask( void ) const
{GUCEF_TRACE;

    // shared pointers have their own assignment locks so copying them is thread safe
    // no need for extra locks here

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
CTask::UpdateTaskChainIds( const TTaskIdVector& taskIds )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_chainTasks = taskIds;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTask::ValidateTaskChainIdSequence( const TTaskIdVector& taskIds )
{GUCEF_TRACE;

    // Validate that all task ids only exist a single time in the chain
    // this is to prevent loops and duplicate tasks in a chain
    for ( UInt32 i=0; i<taskIds.size(); ++i )
    {
        for ( UInt32 n=i+1; n<taskIds.size(); ++n )
        {
            if ( taskIds[ i ] == taskIds[ n ] )
            {
                // duplicate found
                GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "Task:ValidateTaskChainIdSequence: Illegal attempt to create a task chain with duplicate task id " +
                    ToString( taskIds[ i ] ) );
                return false;
            }
        }
    }

    return true;
}

/*-------------------------------------------------------------------------*/

CTaskPtr
CTask::GetNextTask( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    if ( !m_chainTasks.empty() )
    {
        CThreadPoolPtr threadPool = GetThreadPool();
        if ( !threadPool.IsNULL() )
        {
            for ( UInt32 i=0; i<m_chainTasks.size(); ++i )
            {
                if ( m_taskId == m_chainTasks[ i ] )
                {
                    if ( i+1 < m_chainTasks.size() )
                    {
                        CTaskPtr nextTask = threadPool->GetTaskObjById( m_chainTasks[ i+1 ] );
                        return nextTask;
                    }
                    else
                    {
                        // this is already the last task in the chain
                        return CTaskPtr();
                    }
                    break;
                }
            }
        }
    }
    return CTaskPtr();
}

/*-------------------------------------------------------------------------*/

CTaskPtr
CTask::GetPriorTask( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    if ( !m_chainTasks.empty() )
    {
        CThreadPoolPtr threadPool = GetThreadPool();
        if ( !threadPool.IsNULL() )
        {
            for ( UInt32 i=0; i<m_chainTasks.size(); ++i )
            {
                if ( m_taskId == m_chainTasks[ i ] )
                {
                    if ( i > 0 )
                    {
                        CTaskPtr priorTask = threadPool->GetTaskObjById( m_chainTasks[ i-1 ] );
                        return priorTask;
                    }
                    else
                    {
                        // this is already the first task in the chain
                        return CTaskPtr();
                    }
                    break;
                }
            }
        }
    }
    return CTaskPtr();
}

/*-------------------------------------------------------------------------*/

CThreadPoolPtr
CTask::GetThreadPool( void ) const
{GUCEF_TRACE;

    // shared pointers have their own assignment locks so copying them is thread safe
    // no need for extra locks here

    ThreadPoolPtr pool = m_threadPool;
    if ( !pool.IsNULL() )
    {
        return pool;
    }

    CTaskConsumerPtr taskConsumer = m_taskConsumer;
    if ( !taskConsumer.IsNULL() )
    {
        return taskConsumer->GetThreadPool();
    }

    return CThreadPoolPtr();
}

/*-------------------------------------------------------------------------*/

bool
CTask::RequestCancellation( void ) const
{GUCEF_TRACE;

    // shared pointers have their own assignment locks so copying them is thread safe
    // no need for extra locks here

    TIntegerTypeUsedForTaskId taskId = GetTaskId();
    CThreadPoolPtr threadPool = GetThreadPool();
    if ( !threadPool.IsNULL() )
        return threadPool->RequestTaskCancellation( taskId, false, true );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CTask::operator==( const CTask& other ) const
{GUCEF_TRACE;

    return m_taskId == other.m_taskId;
}

/*-------------------------------------------------------------------------*/

bool
CTask::operator!=( const CTask& other ) const
{GUCEF_TRACE;

    return m_taskId != other.m_taskId;
}

/*-------------------------------------------------------------------------*/

bool
CTask::operator<( const CTask& other ) const
{GUCEF_TRACE;

    return m_taskId < other.m_taskId;
}

/*-------------------------------------------------------------------------*/

const MT::CILockable* 
CTask::AsLockable( void ) const
{GUCEF_TRACE;

    return &m_lock;
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CTask::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_lock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CTask::Unlock( void ) const
{GUCEF_TRACE;

    return m_lock.Unlock();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
