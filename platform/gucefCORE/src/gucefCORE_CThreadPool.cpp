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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#ifndef GUCEF_CORE_CSINGLETASKDELEGATOR_H
#include "gucefCORE_CSingleTaskDelegator.h"
#define GUCEF_CORE_CSINGLETASKDELEGATOR_H
#endif /* GUCEF_CORE_CSINGLETASKDELEGATOR_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefCORE_CThreadPool.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CThreadPool::ClassTypeName = "GUCEF::CORE::CThreadPool";

const CEvent CThreadPool::ThreadKilledEvent = "GUCEF::CORE::CThreadPool::ThreadKilledEvent";
const CEvent CThreadPool::ThreadStartedEvent = "GUCEF::CORE::CThreadPool::ThreadStartedEvent";
const CEvent CThreadPool::ThreadPausedEvent = "GUCEF::CORE::CThreadPool::ThreadPausedEvent";
const CEvent CThreadPool::ThreadResumedEvent = "GUCEF::CORE::CThreadPool::ThreadResumedEvent";
const CEvent CThreadPool::ThreadFinishedEvent = "GUCEF::CORE::CThreadPool::ThreadFinishedEvent";

const CEvent CThreadPool::TaskQueuedEvent = "GUCEF::CORE::CThreadPool::TaskQueuedEvent";
const CEvent CThreadPool::TaskStartupEvent = "GUCEF::CORE::CThreadPool::TaskStartupEvent";
const CEvent CThreadPool::TaskStartedEvent = "GUCEF::CORE::CThreadPool::TaskStartedEvent";
const CEvent CThreadPool::TaskStartupFailedEvent = "GUCEF::CORE::CThreadPool::TaskStartupFailedEvent";
const CEvent CThreadPool::TaskKilledEvent = "GUCEF::CORE::CThreadPool::TaskKilledEvent";
const CEvent CThreadPool::TaskStoppedEvent = "GUCEF::CORE::CThreadPool::TaskStoppedEvent";
const CEvent CThreadPool::TaskPausedEvent = "GUCEF::CORE::CThreadPool::TaskPausedEvent";
const CEvent CThreadPool::TaskResumedEvent = "GUCEF::CORE::CThreadPool::TaskResumedEvent";
const CEvent CThreadPool::TaskFinishedEvent = "GUCEF::CORE::CThreadPool::TaskFinishedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CThreadPool::RegisterEvents( void )
{GUCEF_TRACE;

    ThreadKilledEvent.Initialize();
    ThreadStartedEvent.Initialize();
    ThreadPausedEvent.Initialize();
    ThreadResumedEvent.Initialize();
    ThreadFinishedEvent.Initialize();

    TaskQueuedEvent.Initialize();
    TaskStartupEvent.Initialize();
    TaskStartedEvent.Initialize();
    TaskStartupFailedEvent.Initialize();
    TaskKilledEvent.Initialize();
    TaskStoppedEvent.Initialize();
    TaskPausedEvent.Initialize();
    TaskResumedEvent.Initialize();
    TaskFinishedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CThreadPool::CThreadPool( PulseGeneratorPtr threadPoolPulseContext ,
                          const CString& poolName                  )
    : CTSGNotifier( threadPoolPulseContext, true, false )
    , CTSharedPtrCreator< CThreadPool, MT::CMutex >( this )
    , m_poolName( poolName )
    , m_consumerFactory()
    , m_desiredMaxTotalNrOfThreads( -1 )
    , m_desiredMinNrOfWorkerThreads( 0 )
    , m_taskQueue()
    , m_inUseTaskObjs()
    , m_allTaskObjs( TTaskObjFreeList::REUSE_OBJECT_VIA_CLEAR_METHOD_IF_AVAILABLE )
    , m_freeTaskConsumers()
    , m_taskDedicatedDelegators()
    , m_taskGenericDelegators()
    , m_acceptNewWork( true )
    , m_allowAppThreadToWork( false )
{GUCEF_TRACE;

    RegisterEvents();

    CCoreGlobal::Instance()->GetApplication().Subscribe( &AsObserver(), CGUCEFApplication::AppShutdownEvent );
}

/*-------------------------------------------------------------------------*/

CThreadPool::~CThreadPool( void )
{GUCEF_TRACE;
    
    m_acceptNewWork = false;
    if ( !NotifyObservers( DestructionEvent ) )
        return;

    m_taskQueue.SetAcceptsNewMail( false );

    MT::CObjectScopeLock lock( this );

    // Cleanup tasks
    TTaskDelegatorSet::iterator i = m_taskGenericDelegators.begin();
    while ( i != m_taskGenericDelegators.end() )
    {
        // Kill the task
        TTaskDelegatorBasicPtr delegator = (*i);
        delegator->Deactivate( true, true );
        ++i;
    }
    m_taskGenericDelegators.clear();
    i = m_taskDedicatedDelegators.begin();
    while ( i != m_taskDedicatedDelegators.end() )
    {
        // Kill the task
        TTaskDelegatorBasicPtr delegator = (*i);
        delegator->Deactivate( true, true );
        ++i;
    }
    m_taskDedicatedDelegators.clear();
}

/*-------------------------------------------------------------------------*/

const CString&
CThreadPool::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

void
CThreadPool::SetAllowMainApplicationThreadToPickUpQueuedTasks( bool allowAppThreadToWork )
{GUCEF_TRACE;

    m_allowAppThreadToWork = allowAppThreadToWork;
    SetPropagatePulseEvent( m_allowAppThreadToWork );
}

/*-------------------------------------------------------------------------*/

bool
CThreadPool::GetAllowMainApplicationThreadToPickUpQueuedTasks( void ) const
{GUCEF_TRACE;

    return m_allowAppThreadToWork;
}

/*-------------------------------------------------------------------------*/

UInt32
CThreadPool::CarryOutQueuedTasksIfAny( UInt32 maxTasks )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<maxTasks; ++i )
    {
        CTaskPtr task;
        if ( GetQueuedTask( task ) )
        {
            CSingleTaskDelegator singleTaskExecutor( CreateSharedPtr(), task );
            singleTaskExecutor.ExecuteTaskFromCallingThread();
        }
        else
        {
            return i;
        }
    }
    return maxTasks;
}

/*-------------------------------------------------------------------------*/

void
CThreadPool::RemoveDelegator( CNotifier* notifier )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    TTaskDelegatorSet::iterator i = m_taskDedicatedDelegators.begin();
    while ( i != m_taskDedicatedDelegators.end() )
    {
        if ( static_cast< const CNotifier* >( (*i).GetPointerAlways() ) == notifier )
        {
            TTaskDelegatorBasicPtr delegator = (*i);
            m_taskDedicatedDelegators.erase( i );
            return;
        }
        ++i;
    }
    i = m_taskGenericDelegators.begin();
    while ( i != m_taskGenericDelegators.end() )
    {
        if ( static_cast< const CNotifier* >( (*i).GetPointerAlways() ) == notifier )
        {
            TTaskDelegatorBasicPtr delegator = (*i);
            m_taskGenericDelegators.erase( i );
            return;
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CThreadPool::OnPumpedNotify( CNotifier* notifier    ,
                             const CEvent& eventid  ,
                             CICloneable* eventdata )
{GUCEF_TRACE;

    if ( CPulseGenerator::PulseEvent == eventid )
    {
        if ( m_allowAppThreadToWork )
        {
            UInt32 tasksExecuted = CarryOutQueuedTasksIfAny( 1 );

            GUCEF_DEBUG_LOG( LOGLEVEL_BELOW_NORMAL, "ThreadPool(" + m_poolName + "): Executed " + 
                ToString( tasksExecuted ) + " tasks using the main application thread" );
        }
        return;
    }
    else
    if ( CGUCEFApplication::AppShutdownEvent == eventid )
    {
        MT::CObjectScopeLock lock( this );

        // Make sure we shut down all tasks
        EnforceDesiredNrOfThreads( 0, 0, true );
    }
    else
    if ( CTaskDelegator::ThreadKilledEvent == eventid )
    {
        NotifyObservers( ThreadKilledEvent, eventdata );
        RemoveDelegator( notifier );
    }
    else
    if ( CTaskDelegator::ThreadStartedEvent == eventid )
    {
        NotifyObservers( ThreadStartedEvent, eventdata );
    }
    else
    if ( CTaskDelegator::ThreadPausedEvent == eventid )
    {
        NotifyObservers( ThreadPausedEvent, eventdata );
    }
    else
    if ( CTaskDelegator::ThreadResumedEvent == eventid )
    {
        NotifyObservers( ThreadResumedEvent, eventdata );
    }
    else
    if ( CTaskDelegator::ThreadFinishedEvent == eventid )
    {
        NotifyObservers( ThreadFinishedEvent, eventdata );
        RemoveDelegator( notifier );
    }
    else
    if ( CTaskConsumer::TaskKilledEvent == eventid )
    {
        NotifyObservers( TaskKilledEvent, eventdata );
    }
    else
    if ( CTaskConsumer::TaskStartedEvent == eventid )
    {
        NotifyObservers( TaskKilledEvent );
    }
    else
    if ( CTaskConsumer::TaskStartupFailedEvent == eventid )
    {
        NotifyObservers( TaskStartupFailedEvent );
    }
    else
    if ( CTaskConsumer::TaskPausedEvent == eventid )
    {
        NotifyObservers( TaskPausedEvent );
    }
    else
    if ( CTaskConsumer::TaskResumedEvent == eventid )
    {
        NotifyObservers( TaskResumedEvent );
    }
    else
    if ( CTaskConsumer::TaskFinishedEvent == eventid )
    {
        NotifyObservers( TaskFinishedEvent );
    }
}

/*-------------------------------------------------------------------------*/

void
CThreadPool::RemoveConsumer( const CTaskConsumer* taskConsumer )
{GUCEF_TRACE;

    if ( GUCEF_NULL == taskConsumer )
        return;

    UInt32 queueItemsRemoved = 0;
    bool taskConsumerUnlinked = false;

    MT::CScopeMutex mailboxWriter( m_taskQueue.GetLock() );

    TTaskMailbox::iterator i = m_taskQueue.begin( mailboxWriter );
    while ( i != m_taskQueue.end() )
    {
        TTaskMailbox::Mail& mailElement = (*i);
        CTaskPtr& task = mailElement.eventid;

        // Note that in the mailbox elements only have a task consumer if one was provided
        // at the time the task was queued (as would be typical for an externally managed consumer)
        CTaskConsumerPtr consumer = task->GetTaskConsumer();
        if ( !consumer.IsNULL() && taskConsumer == consumer )
        {
            ++queueItemsRemoved;
            m_taskQueue.erase( mailboxWriter, i );
            i = m_taskQueue.begin( mailboxWriter );
            continue;
        }
        ++i;
    }

    mailboxWriter.EarlyUnlock();

    CTaskPtr consumerTask = taskConsumer->GetCurrentTask();
    if ( !consumerTask.IsNULL() )
    {
        CTask::TIntegerTypeUsedForTaskId taskId = consumerTask->GetTaskId();

        MT::CObjectScopeLock poolLock( this );

        TTaskId2TaskPtrMap::iterator n = m_inUseTaskObjs.find( taskId );
        if ( n != m_inUseTaskObjs.end() )
        {
            CTaskPtr task = (*n).second;
            if ( !task.IsNULL() )
            {
                // things could have changed by now so double check association
                CTaskConsumerPtr consumerOfTask = task->GetTaskConsumer();
                if ( consumerOfTask.GetPointerAlways() == taskConsumer )
                {
                    m_inUseTaskObjs.erase( n );

                    poolLock.EarlyUnlock();

                    // The task the consumer is referencing is currently being worked on
                    // We have to stop it
                    bool hasStopped = RequestTaskToStop( taskId, true, true );
                    if ( !hasStopped )
                        hasStopped = KillThreadWorkingOnTask( taskId );

                    task->SetTaskConsumer( CTaskConsumerPtr() );

                    taskConsumerUnlinked = true;
                }
            }
        }
        else
        {
            poolLock.EarlyUnlock();
        }
    }

    GUCEF_DEBUG_LOG( LOGLEVEL_BELOW_NORMAL, "ThreadPool(" + m_poolName + "): Removing references to consumer " + ToString( taskConsumer ) +
        ". This consumer had " + ToString( queueItemsRemoved ) + " queued work items. Unlinked consumer: " + ToString( taskConsumerUnlinked ) );
}

/*-------------------------------------------------------------------------*/

Int32
CThreadPool::EnforceDesiredNrOfThreads( Int32 desiredMaxTotalNrOfThreads   ,
                                        UInt32 desiredMinNrOfWorkerThreads ,
                                        bool gracefullEnforcement          )
{GUCEF_TRACE;

    // This is an internal function but still make sure that the TaskManager is locked
    // when this function is called. There are no locks here for efficiency!!!
    m_desiredMaxTotalNrOfThreads = desiredMaxTotalNrOfThreads;
    m_desiredMinNrOfWorkerThreads = desiredMinNrOfWorkerThreads;

    // Check if we need to do anything, calculate capacity
    UInt32 totalActiveNrOfThreads = (UInt32) m_taskGenericDelegators.size() + (UInt32) m_taskDedicatedDelegators.size();
    Int32 threadHeadroom = -1;
    Int32 missingWorkerThreads = 0;
    if ( m_desiredMaxTotalNrOfThreads >= 0 )
    {
        threadHeadroom = (Int32) m_desiredMaxTotalNrOfThreads - (Int32) totalActiveNrOfThreads;        
        if ( threadHeadroom > 0 )
        {
            missingWorkerThreads = (Int32) m_desiredMinNrOfWorkerThreads - (Int32) m_taskGenericDelegators.size();
            if ( missingWorkerThreads > threadHeadroom )
                missingWorkerThreads = threadHeadroom;
        }
    }
    else
    {
        missingWorkerThreads = (Int32) m_desiredMinNrOfWorkerThreads - (Int32) m_taskGenericDelegators.size();
    }

    if ( missingWorkerThreads > 0 )
    {
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Increasing the number of worker threads used for processing tasks to " +
            ToString( (UInt32) m_taskGenericDelegators.size() + (UInt32) missingWorkerThreads ) + ", adding " + ToString( missingWorkerThreads ) );

        for ( Int32 i=0; i<missingWorkerThreads; ++i )
        {
            // Just spawn a generic task delegator
            CTaskDelegator* rawDelegator = GUCEF_NEW CTaskDelegator( CreateSharedPtr() );
            CTaskDelegatorPtr delegator( rawDelegator->CreateSharedPtr() );
            SubscribeTo( delegator.GetPointerAlways() );
            m_taskGenericDelegators.insert( delegator );

            if ( delegator->Activate() )
            {
                GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Created worker thread with ID " + ToString( delegator->GetThreadID() ) );
            }
            else
            {
                m_taskGenericDelegators.erase( delegator );
                GUCEF_ERROR_LOG( LOGLEVEL_CRITICAL, "ThreadPool(" + m_poolName + "): Failed to create worker thread" );
            }
        }
    }
    else
    if ( threadHeadroom < 0 )
    {
        // Check the number of threads that are already asked to deactivate
        Int32 deactivatingCount = 0;
        TTaskDelegatorSet::iterator i = m_taskGenericDelegators.begin();
        while ( i != m_taskGenericDelegators.end() )
        {
            if ( (*i)->IsActive() )
            {
                if ( (*i)->IsDeactivationRequested() )
                {
                    ++deactivatingCount;
                }
            }
            ++i;
        }
        i = m_taskDedicatedDelegators.begin();
        while ( i != m_taskDedicatedDelegators.end() )
        {
            if ( (*i)->IsActive() )
            {
                if ( (*i)->IsDeactivationRequested() )
                {
                    ++deactivatingCount;
                }
            }
            ++i;
        }

        // Check if we need to do anything
        Int32 leftToBeDeactivated = -1 * ( threadHeadroom + deactivatingCount );
        if ( leftToBeDeactivated > 0 )
        {
            GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): There is an excess of " +
                    ToString( leftToBeDeactivated ) + " threads, taking into account " + ToString( deactivatingCount ) +
                    " threads which are already deactivating" );

            UInt32 workersAskedToShutDown = 0;
            TTaskDelegatorSet taskDelegators = m_taskGenericDelegators;
            TTaskDelegatorSet::iterator i = taskDelegators.begin();
            while ( leftToBeDeactivated > 0 && i != taskDelegators.end() )
            {
                const TTaskDelegatorBasicPtr& delegator = (*i);

                if ( delegator->IsActive() )
                {
                    // If the thread is not yet asked to deactivate we will do so now up
                    // to the number of thread we wish to deactivate
                    if ( !delegator->IsDeactivationRequested() )
                    {
                        // Ask thread to deactivate
                        UInt32 threadId = delegator->GetThreadID();
                        TTaskDelegatorBasicPtr delegatorRefCopy = (*i);
                        delegatorRefCopy->Deactivate( !gracefullEnforcement, false );

                        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Asked thread " + ToString( threadId ) + " to deactivate" );

                        --leftToBeDeactivated;
                        ++workersAskedToShutDown;
                    }
                }
                ++i;
            }

            if ( leftToBeDeactivated > 0 )
            {
                GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): After asking " + ToString( workersAskedToShutDown ) +
                        " worker threads to deactivate there is a remaining thread excess of " +
                        ToString( leftToBeDeactivated ) + " threads. Will ask " + ToString( leftToBeDeactivated ) + " dedicated threads to deactivate" );
            
                taskDelegators = m_taskDedicatedDelegators;
                TTaskDelegatorSet::iterator i = taskDelegators.begin();
                while ( leftToBeDeactivated > 0 && i != taskDelegators.end() )
                {
                    if ( (*i)->IsActive() )
                    {
                        // If the thread is not yet asked to deactivate we will do so now up
                        // to the number of thread we wish to deactivate
                        if ( !(*i)->IsDeactivationRequested() )
                        {
                            // Ask thread to deactivate
                            TTaskDelegatorBasicPtr delegator = (*i);
                            delegator->Deactivate( !gracefullEnforcement, false );
                            --leftToBeDeactivated;
                        }
                    }
                    ++i;
                }
            }
        }
    }
    // else: we don't have to do anything

    return threadHeadroom;
}

/*-------------------------------------------------------------------------*/

CTaskPtr
CThreadPool::PrepTaskObj( const CString& taskType        ,
                          CICloneable* taskData          ,
                          CObserver* taskObserver        ,
                          bool assumeOwnershipOfTaskData )
{GUCEF_TRACE;

    if ( !m_acceptNewWork )
        return CTask::CreateSharedObjWithParam( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

/*
    // If the caller wants to have a reference to the task consumer we will create it now
    // This does tend to lead to more consumers being created than strictly necessary since it
    // scales with the number of tasks queued, but it is the most convenient way to do it
    CTaskConsumerPtr taskConsumer;
    if ( GUCEF_NULL != outTask )
    {            
        if ( !GetOrCreateTaskConsumerOfType( taskType, taskConsumer ) )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Cannot queue task of type \"" + taskType + "\" because no suitable task consumer is available" );
            return TTaskStatus::TASKSTATUS_TASKTYPE_INVALID;
        }
        taskConsumer->Subscribe( taskObserver );

        // Assume queue success below for now, may have to undo if it fails
        taskConsumer->SetTaskStatus( TTaskStatus::TASKSTATUS_QUEUED );
    }
*/

    CTaskPtr task;
    if ( !GetOrCreateTaskObj( task ) || task.IsNULL() || task->GetTaskStatus() == TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED )
    {
        TaskCleanup( task );
        return CTask::CreateSharedObjWithParam( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );
    }

    if ( !task->Init( taskType                       ,
                      CTaskConsumerPtr()             ,
                      taskData                       ,
                      assumeOwnershipOfTaskData      ,
                      GUCEF_NULL                     ,
                      TTaskStatus::TASKSTATUS_SETUP  ,
                      CreateBasicSharedPtr()         ) )
    {
        task->SetTaskStatus( TTaskStatus::TASKSTATUS_SETUP_FAILED );
    }
    return task;
}

/*-------------------------------------------------------------------------*/

TTaskStatus
CThreadPool::ValidateTaskForIngress( CTaskPtr& task ) const
{GUCEF_TRACE;

    if GUCEF_PREDICT_FALSE( task.IsNULL() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Cannot queue task because it is NULL" );
        return TTaskStatus::TASKSTATUS_QUEUEING_FAILED;
    }
    if GUCEF_PREDICT_FALSE( task->IsTaskInErrorState() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Cannot queue task because it is in an error state" );
        return TTaskStatus::TASKSTATUS_QUEUEING_FAILED;
    }
    if GUCEF_PREDICT_FALSE( !m_acceptNewWork )
    {
        task->SetTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );
        return TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED ;
    }
    return TTaskStatus::TASKSTATUS_SETUP;
}

/*-------------------------------------------------------------------------*/

TTaskStatus
CThreadPool::ValidateTaskChainForIngress( const TTaskPtrVector& tasks ) const
{GUCEF_TRACE;

    if GUCEF_PREDICT_FALSE( tasks.empty() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Cannot queue task chain because the task list is empty" );
        return TTaskStatus::TASKSTATUS_QUEUEING_FAILED;
    }

    // Validate the task chain before accepting it
    TTaskPtrVector::const_iterator i = tasks.begin();
    while ( i != tasks.end() )
    {
        const CTaskPtr& task = (*i);
        if GUCEF_PREDICT_FALSE( task.IsNULL() )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Cannot queue task chain because one of the tasks is NULL" );
            return TTaskStatus::TASKSTATUS_TASK_CHAINING_FAILED;
        }
        if GUCEF_PREDICT_FALSE( task->IsTaskInErrorState() )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Cannot queue task chain because one of the tasks is in an error state" );
            return TTaskStatus::TASKSTATUS_TASK_CHAINING_FAILED;
        }
        ++i;
    }

    if GUCEF_PREDICT_FALSE( !m_acceptNewWork )
    {
        TTaskPtrVector::const_iterator i = tasks.begin();
        while ( i != tasks.end() )
        {
            CTaskPtr task = (*i);
            task->SetTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );
            ++i;
        }
        return TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED ;
    }

    return TTaskStatus::TASKSTATUS_SETUP;
}

/*-------------------------------------------------------------------------*/

CFutureResult
CThreadPool::QueueTask( CTaskPtr task )
{GUCEF_TRACE;

    TTaskStatus status = ValidateTaskForIngress( task );
    if ( TaskStatusIsAnError( status ) )
    {
        return status;
    }

    task->SetTaskStatus( TTaskStatus::TASKSTATUS_QUEUED );

    {
        MT::CObjectScopeLock lock( this );
        m_inUseTaskObjs[ task->GetTaskId() ] = task;
    }

    // Now that everything has been setup we can queue the first task in the chain
    // which will in turn trigger the rest of the chain as each task finishes
    // As soon as the task is added to the queue other threads may pick it up right away
    
    if ( m_taskQueue.AddMail( task ) )
    {
        // We don't want to queue a task that will never be picked up by anyone
        if ( 0 == GetActiveNrOfWorkerThreads() && !m_allowAppThreadToWork )
        {
            MT::CObjectScopeLock lock( this );
            if ( 0 == GetActiveNrOfWorkerThreads() && !m_allowAppThreadToWork )
            {
                EnforceDesiredNrOfThreads( m_desiredMaxTotalNrOfThreads, 1, true );
            }
        }
        return task;
    }
    else
    {
        // Since we failed to add the task to the queue we need to roll back everything

        task->SetTaskStatus( TTaskStatus::TASKSTATUS_QUEUEING_FAILED );
        {
            MT::CObjectScopeLock lock( this );
            m_inUseTaskObjs.erase( task->GetTaskId() );
        }

        return TTaskStatus::TASKSTATUS_QUEUEING_FAILED;
    }
}

/*-------------------------------------------------------------------------*/

CFutureResult
CThreadPool::QueueTaskChain( const TTaskPtrVector& tasks )
{GUCEF_TRACE;

    TTaskStatus status = ValidateTaskChainForIngress( tasks );
    if ( TaskStatusIsAnError( status ) )
    {
        return status;
    }

    CTaskPtr firstTask = tasks.front();
    firstTask->SetTaskStatus( TTaskStatus::TASKSTATUS_QUEUED );
    status = TTaskStatus::TASKSTATUS_QUEUED;

    {
        TTaskQueuedEventData eventData( firstTask->GetTaskId() );
        NotifyObserversFromThread( TaskQueuedEvent, &eventData );
    }
    {
        // We queue the tasks in the 'in-use' map which allows for lookup of said tasks via the task id
        // for chains we the subsequent taks are only added to the in-use map not the actual queue since
        // there is a required order of execution and we dont want other threads to pick up tasks out of order

        MT::CObjectScopeLock lock( this );

        m_inUseTaskObjs[ firstTask->GetTaskId() ] = firstTask;

        TTaskPtrVector::const_iterator i = tasks.begin();
        ++i;
        while ( i != tasks.end() )
        {
            CTaskPtr task = (*i);
            task->SetTaskStatus( TTaskStatus::TASKSTATUS_CHAIN_PREREQ_QUEUED );
            m_inUseTaskObjs[ task->GetTaskId() ] = task;

            TTaskQueuedEventData eventData( task->GetTaskId() );
            NotifyObserversFromThread( TaskQueuedEvent, &eventData );

            ++i;
        }
    }

    // Now that everything has been setup we can queue the first task in the chain
    // which will in turn trigger the rest of the chain as each task finishes
    // As soon as the task is added to the queue other threads may pick it up right away
    
    if ( m_taskQueue.AddMail( firstTask ) )
    {
        // We don't want to queue a task that will never be picked up by anyone
        if ( 0 == GetActiveNrOfWorkerThreads() && !m_allowAppThreadToWork )
        {
            MT::CObjectScopeLock lock( this );
            if ( 0 == GetActiveNrOfWorkerThreads() && !m_allowAppThreadToWork )
            {
                EnforceDesiredNrOfThreads( m_desiredMaxTotalNrOfThreads, 1, true );
            }
        }

        // We return the last task as that is the task one would wait for to finish the chain
        // any failure earlier in the chain will propagate to the last task as well
        CTaskPtr lastTask = tasks.back();
        return lastTask;
    }
    else
    {
        // Since we failed to add the first task to the queue we need to roll back everything

        firstTask->SetTaskStatus( TTaskStatus::TASKSTATUS_QUEUEING_FAILED );

        TTaskPtrVector::const_iterator i = tasks.begin();
        ++i;
        while ( i != tasks.end() )
        {
            CTaskPtr task = (*i);
            task->SetTaskStatus( TTaskStatus::TASKSTATUS_CHAIN_PREREQ_FAILED );
            ++i;
        }

        {
            MT::CObjectScopeLock lock( this );
            i = tasks.begin();
            while ( i != tasks.end() )
            {
                const CTaskPtr& task = (*i);
                m_inUseTaskObjs.erase( task->GetTaskId() );
                ++i;
            }            
        }

        return TTaskStatus::TASKSTATUS_QUEUEING_FAILED;
    }
}

/*-------------------------------------------------------------------------*/

CFutureResult
CThreadPool::QueueTask( const CString& taskType        ,
                        CICloneable* taskData          ,
                        CObserver* taskObserver        ,
                        bool assumeOwnershipOfTaskData )
{GUCEF_TRACE;

    CTaskPtr task = PrepTaskObj( taskType                  ,
                                 taskData                  ,
                                 taskObserver              ,
                                 assumeOwnershipOfTaskData );

    return QueueTask( task );
}

/*-------------------------------------------------------------------------*/

void
CThreadPool::SubscribeToTaskConsumerEvents( CTaskConsumerPtr& taskConsumer )
{GUCEF_TRACE;

    CORE::CNotifier* notifier = taskConsumer.GetPointerAlways();
    SubscribeTo( notifier, CTaskConsumer::TaskKilledEvent );
    SubscribeTo( notifier, CTaskConsumer::TaskStartedEvent );
    SubscribeTo( notifier, CTaskConsumer::TaskStartupFailedEvent );
    SubscribeTo( notifier, CTaskConsumer::TaskPausedEvent );
    SubscribeTo( notifier, CTaskConsumer::TaskResumedEvent );
    SubscribeTo( notifier, CTaskConsumer::TaskFinishedEvent );
}

/*-------------------------------------------------------------------------*/

void
CThreadPool::SetNrOfWorkerThreadsToLogicalCPUs( const UInt32 factor )
{GUCEF_TRACE;

    SetDesiredMinNrOfWorkerThreads( GetLogicalCPUCount() * factor );
}

/*-------------------------------------------------------------------------*/

void
CThreadPool::SetDesiredMaxTotalNrOfThreads( const Int32 nrOfThreads )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    m_desiredMaxTotalNrOfThreads = nrOfThreads;
    EnforceDesiredNrOfThreads( m_desiredMaxTotalNrOfThreads, m_desiredMinNrOfWorkerThreads, true );
}

/*-------------------------------------------------------------------------*/

Int32
CThreadPool::GetDesiredMaxTotalNrOfThreads( void ) const
{GUCEF_TRACE;

    return m_desiredMaxTotalNrOfThreads;
}

/*-------------------------------------------------------------------------*/

void
CThreadPool::SetDesiredMinNrOfWorkerThreads( const UInt32 nrOfThreads )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    m_desiredMinNrOfWorkerThreads = nrOfThreads;
    EnforceDesiredNrOfThreads( m_desiredMaxTotalNrOfThreads, m_desiredMinNrOfWorkerThreads, true );
}

/*-------------------------------------------------------------------------*/

UInt32
CThreadPool::GetDesiredMinNrOfWorkerThreads( void ) const
{GUCEF_TRACE;

    return m_desiredMinNrOfWorkerThreads;
}

/*-------------------------------------------------------------------------*/

UInt32
CThreadPool::GetActiveNrOfDedicatedThreads( void ) const
{GUCEF_TRACE;

    return (UInt32) m_taskDedicatedDelegators.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CThreadPool::GetActiveNrOfWorkerThreads( void ) const
{GUCEF_TRACE;

    return (UInt32) m_taskGenericDelegators.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CThreadPool::GetActiveNrOfThreads( void ) const
{GUCEF_TRACE;

    return (UInt32) m_taskDedicatedDelegators.size() + (UInt32) m_taskGenericDelegators.size();
}

/*-------------------------------------------------------------------------*/

UInt32 
CThreadPool::GetNrOfQueuedTasks( void ) const
{GUCEF_TRACE;

    return m_taskQueue.AmountOfMail();
}

/*-------------------------------------------------------------------------*/

void
CThreadPool::RegisterTaskConsumerFactory( const CString& taskType       ,
                                          TTaskConsumerFactory* factory )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    m_consumerFactory.RegisterConcreteFactory( taskType, factory );
    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): new consumer factory registered of type " + taskType );
}

/*-------------------------------------------------------------------------*/

void
CThreadPool::UnregisterTaskConsumerFactory( const CString& taskType )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    m_consumerFactory.UnregisterConcreteFactory( taskType );
    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): consumer factory unregistered of type " + taskType );
}

/*-------------------------------------------------------------------------*/

bool
CThreadPool::GetOrCreateTaskObj( CTaskPtr& taskObj )
{GUCEF_TRACE;

    // Free list concept
    // re-use a task object previously used if any are available, avoiding the setup costs
    taskObj = m_allTaskObjs.Acquire();
    if GUCEF_PREDICT_FALSE( taskObj.IsNULL() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "):GetOrCreateTaskObj: Failed to create a new task object" );
        return false;
    }

    if ( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED == taskObj->GetTaskStatus() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "):GetOrCreateTaskObj: Failed to create a new task object due to task internal resource limit" );
        return false;
    }

    GUCEF_DEBUG_LOG( LOGLEVEL_BELOW_NORMAL, "ThreadPool(" + m_poolName + "):GetOrCreateTaskObj: Obtained task object with task id " + ToString( taskObj->GetTaskId() ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CThreadPool::GetOrCreateTaskConsumerOfType( const CString& taskConsumerType ,
                                            CTaskConsumerPtr& taskConsumer  )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    // Free list concept
    // re-use a task consumer previously used if any are available
    TStringToTaskConsumerSetMap::iterator i = m_freeTaskConsumers.find( taskConsumerType );
    if ( i != m_freeTaskConsumers.end() )
    {
        CTaskConsumerPtrSet& freeConsumers = (*i).second;
        if ( !freeConsumers.empty() )
        {
            taskConsumer = *freeConsumers.begin();
            freeConsumers.erase( freeConsumers.begin() );
            return true;
        }
    }

    lock.EarlyUnlock();

    // Create a consumer for the given task type
    taskConsumer = m_consumerFactory.Create( taskConsumerType );
    if ( taskConsumer.IsNULL() )
    {
        // No pool local factory registered capable of handling the task type
        // We now check the global factory for the same
        taskConsumer = CCoreGlobal::Instance()->GetTaskManager().CreateTaskConsumer( taskConsumerType );
    }
    if ( !taskConsumer.IsNULL() )
    {
        taskConsumer->SetIsOwnedByThreadPool( true );
        taskConsumer->SetThreadPool( CreateBasicSharedPtr() );
        SubscribeToTaskConsumerEvents( taskConsumer );
    }
    return !taskConsumer.IsNULL();
}

/*-------------------------------------------------------------------------*/

bool
CThreadPool::GetQueuedTask( CTaskPtr& task )
{GUCEF_TRACE;

    CTaskPtr taskFromQueue;
    if ( m_taskQueue.GetMail( taskFromQueue ) )
    {
        // sanity check
        if GUCEF_PREDICT_FALSE( taskFromQueue.IsNULL() ) 
        {
            // Try the next queue item (if any)
            return GetQueuedTask( task );
        }

        // Its possible that the task consumer was created earlier
        // create one now if it was not. The latter is preferred since it allows for a smaller pool of consumers to be maintained
        // Downside is that it further abstracts the internals such that the caller wont have a reference to the task consumer
        // As such we leave it up to the caller wrt this trade-off
        CTaskConsumerPtr taskConsumer = taskFromQueue->GetTaskConsumer();
        if ( taskConsumer.IsNULL() )
        {
            // The task does not yet have a consumer, get one
            if ( !GetOrCreateTaskConsumerOfType( taskFromQueue->GetTaskType(), taskConsumer ) || taskConsumer.IsNULL() )
            {
                GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "):GetQueuedTask: Cannot provide task to thread since no suitable task consumer is available for type " + taskFromQueue->GetTaskType() );

                // Try the next queue item (if any)
                return GetQueuedTask( task );
            }
        }

        taskConsumer->SetCurrentTask( taskFromQueue );
        taskFromQueue->SetTaskConsumer( taskConsumer );

        task = taskFromQueue;

        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CFutureResult
CThreadPool::SetupSingularTask( CTaskConsumerPtr taskConsumer  ,
                                CICloneable* taskData          ,
                                bool assumeOwnershipOfTaskData )
{GUCEF_TRACE;

    return SetupSingularTaskImpl( taskConsumer, taskData, assumeOwnershipOfTaskData, GUCEF_NULL, false );
}

/*-------------------------------------------------------------------------*/

CFutureResult
CThreadPool::SetupSingularTaskImpl( CTaskPtr task )
{GUCEF_TRACE;

    // We will need a task object which represents the task to be executed
    // This variant of the function expects things to arrive more pre-baked
    if ( task.IsNULL() || TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED == task->GetTaskStatus() )
        return CTask::CreateSharedObjWithParam( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    // Sanity check: the task consumer should not already have a delegator assigned for this use-case
    CTaskConsumerPtr taskConsumer = task->GetTaskConsumer();
    if ( !taskConsumer.IsNULL() && !taskConsumer->GetTaskDelegator().IsNULL() )
    {
        UInt32 threadId = taskConsumer->GetTaskDelegator()->GetThreadID();
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Cannot setup task because the taskConsumer given already has a delegator (thread: " + ToString( threadId ) + ") assigned" );
        task->SetTaskStatus( TTaskStatus::TASKSTATUS_SETUP_FAILED );
        return task;
    }

    // When given a task for immediate execution we should be able to have or get a task consumer right away
    bool consumerWasCreatedByPool = true;
    if ( taskConsumer.IsNULL() )
    {
        CString taskType = task->GetTaskType();
        if ( taskType.IsNULLOrEmpty() )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Cannot setup task because it has no taskConsumer or task type" );
            return CTask::CreateSharedObjWithParam( TTaskStatus::TASKSTATUS_TASKTYPE_INVALID );
        }

        bool consumerObtained = GetOrCreateTaskConsumerOfType( taskType, taskConsumer );
        if ( !consumerObtained || taskConsumer.IsNULL() )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Cannot setup task because no taskConsumer could be obtained for task of type: " + taskType );
            return CTask::CreateSharedObjWithParam( TTaskStatus::TASKSTATUS_SETUP_FAILED );
        }

        // the task consumer was created by the pool so we already have a reference to it and are subscribed to its events
        task->SetTaskConsumer( taskConsumer );
    }
    else
    {
        consumerWasCreatedByPool = taskConsumer->IsOwnedByThreadPool();
        if ( !consumerWasCreatedByPool )
        {
            // If it was externally created the consumer may not be linked to the thread pool yet
            taskConsumer->SetThreadPool( CreateBasicSharedPtr() );

            // We listen for task consumer generic events
            SubscribeToTaskConsumerEvents( taskConsumer );
        }
    }

    // IMPORTANT: We set the flag to signal to the thread on startup that we wish to perform a phased setup
    //            This causes the consumer to be linked but no OnTaskStart or the like will not yet be invoked on the task consumer 
    taskConsumer->SetIsInPhasedSetup( true );

    MT::CObjectScopeLock lock( this );

    // Check if we are accepting new work, if not we refuse to do anything else
    if ( !m_acceptNewWork )
    {
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Refusing to setup task immediately of type \"" + task->GetTaskType() +
            "\" with id " + ToString( task->GetTaskId() ) + " because the thread pool is not accepting new work" );
        task->SetTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );
        return task;
    }

    // Now spawn a task delegator
    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Setting up task of type \"" + taskConsumer->GetType() + "\" with ID " + ToString( task->GetTaskId() )  );
    CTaskDelegatorPtr delegator( ( GUCEF_NEW CSingleTaskDelegator( CreateSharedPtr(), task ) )->CreateSharedPtr() );
    SubscribeTo( delegator.GetPointerAlways() );

    m_taskDedicatedDelegators.insert( delegator );
    m_inUseTaskObjs[ task->GetTaskId() ] = task;
    m_allTaskObjs.MarkActive( task );

    if ( delegator->Activate() )
    {
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Successfully activated dedicated delegator for task type \"" + taskConsumer->GetType() +
            "\" with task ID " + UInt32ToString( task->GetTaskId() ) + " and thread ID " + ToString( delegator->GetThreadID() )  );

        TThreadStartedEventData threadIdData( delegator->GetThreadID() );
        NotifyObserversFromThread( ThreadStartedEvent, &threadIdData );

        return task;
    }
    else
    {
        m_taskDedicatedDelegators.erase( delegator );
        m_inUseTaskObjs.erase( task->GetTaskId() );
        m_allTaskObjs.MarkDormant( task );

        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Failed to activate dedicated delegator for task type \"" + taskConsumer->GetType() +
            "\" with task ID " + ToString( task->GetTaskId() )  + " and thread ID " + ToString( delegator->GetThreadID() )  );

        task->SetTaskStatus( TTaskStatus::TASKSTATUS_STARTUP_FAILED );
        return task;
    }
}

/*-------------------------------------------------------------------------*/

CFutureResult
CThreadPool::SetupSingularTaskImpl( CTaskConsumerPtr taskConsumer  ,
                                    CICloneable* taskData          ,
                                    bool assumeOwnershipOfTaskData ,
                                    CDataNode* taskDataDom         ,
                                    bool consumerWasCreatedByPool  )
{GUCEF_TRACE;

    // We will need a task object which represents the task to be executed
    CTaskPtr task;
    if ( !GetOrCreateTaskObj( task ) || task.IsNULL() || TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED == task->GetTaskStatus() )
        return CTask::CreateSharedObjWithParam( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    if ( taskConsumer.IsNULL() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Cannot setup task because a nullptr is passed as the taskConsumer" );
        return CTask::CreateSharedObjWithParam( TTaskStatus::TASKSTATUS_SETUP_FAILED );
    }

    MT::CObjectScopeLock lock( this );

    // Check if we are accepting new work, if not we refuse to set up the task consumer
    if ( !m_acceptNewWork )
    {
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Refusing to setup task immediately of type \"" + taskConsumer->GetType() + "\" because the thread pool is not accepting new work" );
        task->SetTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );
        return task;
    }

    // Sanity check: the task consumer should not already have a delegator assigned for this use-case
    if ( !taskConsumer->GetTaskDelegator().IsNULL() )
    {
        UInt32 threadId = taskConsumer->GetTaskDelegator()->GetThreadID();
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Cannot setup task because the taskConsumer given already has a delegator (thread: " + ToString( threadId ) + ") assigned" );
        task->SetTaskStatus( TTaskStatus::TASKSTATUS_SETUP_FAILED );
        return task;
    }

    if ( !consumerWasCreatedByPool )
    {
        // Leverage the special friendship to internally set up the relationship / ownership
        // Keep in mind we support both task consumers who's life cycle is managed by the pool but also
        // externalized consumers which have an independent life cycle. As such we need to keep track of which is
        // which so we don't assume life cycle ownership when in fact we have none
        taskConsumer->SetIsOwnedByThreadPool( false );    
        taskConsumer->SetThreadPool( CreateBasicSharedPtr() );

        // We listen for task consumer generic events
        SubscribeToTaskConsumerEvents( taskConsumer );
    }
    // else: the task consumer was created by the pool so we already have a reference to it and are subscribed to its events

    // IMPORTANT: We set the flag to signal to the thread on startup that we wish to perform a phased setup
    //            This causes the consumer to be linked but no OnTaskStart or the like will not yet be invoked on the task consumer 
    taskConsumer->SetIsInPhasedSetup( true );

    if GUCEF_PREDICT_FALSE( !task->Init( taskConsumer->GetType()       ,
                                         taskConsumer                  ,
                                         taskData                      ,
                                         assumeOwnershipOfTaskData     ,
                                         taskDataDom                   ,
                                         TTaskStatus::TASKSTATUS_SETUP ,
                                         CreateBasicSharedPtr()        ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "):SetupSingularTaskImpl: Init failed for Task" );
        task->SetTaskStatus( TTaskStatus::TASKSTATUS_SETUP_FAILED );
        return task;
    }

    // Now spawn a task delegator
    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Setting up task of type \"" + taskConsumer->GetType() + "\" with ID " + ToString( task->GetTaskId() )  );
    CTaskDelegatorPtr delegator( ( GUCEF_NEW CSingleTaskDelegator( CreateSharedPtr(), task ) )->CreateSharedPtr() );
    SubscribeTo( delegator.GetPointerAlways() );
    m_taskDedicatedDelegators.insert( delegator );

    m_inUseTaskObjs[ task->GetTaskId() ] = task;
    m_allTaskObjs.MarkActive( task );

    if ( delegator->Activate() )
    {
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Successfully activated dedicated delegator for task type \"" + taskConsumer->GetType() +
            "\" with task ID " + UInt32ToString( task->GetTaskId() ) + " and thread ID " + ToString( delegator->GetThreadID() )  );
        
        TThreadStartedEventData threadIdData( delegator->GetThreadID() );
        NotifyObserversFromThread( ThreadStartedEvent, &threadIdData );

        return task;
    }
    else
    {
        m_taskDedicatedDelegators.erase( delegator );
        m_inUseTaskObjs.erase( task->GetTaskId() );
        m_allTaskObjs.MarkDormant( task );

        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Failed to activate dedicated delegator for task type \"" + taskConsumer->GetType() +
            "\" with task ID " + ToString( task->GetTaskId() )  + " and thread ID " + ToString( delegator->GetThreadID() )  );

        task->SetTaskStatus( TTaskStatus::TASKSTATUS_STARTUP_FAILED );
        return task;
    }
}

/*-------------------------------------------------------------------------*/

CFutureResult
CThreadPool::StartTaskChain( const TTaskPtrVector& tasks )
{GUCEF_TRACE;

    TTaskStatus status = ValidateTaskChainForIngress( tasks );
    if ( TaskStatusIsAnError( status ) )
    {
        return status;
    }

    CTaskPtr firstTask = tasks.front();

    CFutureResult future = SetupSingularTaskImpl( firstTask );
    if GUCEF_PREDICT_FALSE( future.HasNoFuture() )
    {
        TTaskPtrVector::const_iterator i = tasks.begin();
        ++i;
        while ( i != tasks.end() )
        {
            CTaskPtr task = (*i);
            task->SetTaskStatus( TTaskStatus::TASKSTATUS_CHAIN_PREREQ_FAILED );
            ++i;
        }

        return future;
    }

    {
        MT::CObjectScopeLock lock( this );

        // Since the first task was successfully setup we can now set up a reference to the rest of the chain
        TTaskPtrVector::const_iterator i = tasks.begin();
        ++i;
        while ( i != tasks.end() )
        {
            CTaskPtr task = (*i);
            firstTask->SetTaskStatus( TTaskStatus::TASKSTATUS_CHAIN_PREREQ_QUEUED );
            m_inUseTaskObjs[ task->GetTaskId() ] = task;
            ++i;
        }
    }

    // Check to see if setup has been performed yet
    // If it was not explicitly invoked yet we will just incorporate the setup step here
    CTaskConsumerPtr taskConsumer = firstTask->GetTaskConsumer();
    if ( taskConsumer->GetIsInPhasedSetup() )
    {
        // IMPORTANT: We remove the flag to signal to the delegator it should commence operations
        taskConsumer->SetIsInPhasedSetup( false );
    }

    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): First chain task of task type \"" + taskConsumer->GetType() +
            "\" with task ID " + ToString( taskConsumer->GetCurrentTaskId() ) + 
            ", linked to thread " + ToString( taskConsumer->GetDelegatorThreadId() ) + ", is instructed to commence work. There are "
            + ToString( tasks.size() ) + " tasks in the chain." );

    // We return the last task as that is the task one would wait for to finish the chain
    // any failure earlier in the chain will propagate to the last task as well
    CTaskPtr lastTask = tasks.back();
    return lastTask;
}

/*-------------------------------------------------------------------------*/

CFutureResult
CThreadPool::StartTask( CTaskPtr task )
{GUCEF_TRACE;

    TTaskStatus status = ValidateTaskForIngress( task );
    if ( TaskStatusIsAnError( status ) )
    {
        return status;
    }

    CFutureResult future = SetupSingularTaskImpl( task );
    if GUCEF_PREDICT_FALSE( future.HasNoFuture() )
        return future;

    // Check to see if setup has been performed yet
    // If it was not explicitly invoked yet we will just incorporate the setup step here
    CTaskConsumerPtr taskConsumer = task->GetTaskConsumer();
    if ( taskConsumer->GetIsInPhasedSetup() )
    {
        // IMPORTANT: We remove the flag to signal to the delegator it should commence operations
        taskConsumer->SetIsInPhasedSetup( false );
    }

    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Task of task type \"" + taskConsumer->GetType() +
            "\" with task ID " + ToString( taskConsumer->GetCurrentTaskId() ) + 
            ", linked to thread " + ToString( taskConsumer->GetDelegatorThreadId() ) + ", is instructed to commence work" );

    return task;
}

/*-------------------------------------------------------------------------*/

CFutureResult
CThreadPool::StartTaskWithConsumer( CTaskConsumerPtr taskConsumer  ,
                                    CICloneable* taskData          ,
                                    bool assumeOwnershipOfTaskData )
{GUCEF_TRACE;

    if ( taskConsumer.IsNULL() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Cannot start task because a nullptr is passed as the taskConsumer" );
        return CTask::CreateSharedObjWithParam( TTaskStatus::TASKSTATUS_UNDEFINED );
    }

    // Check to see if setup has been performed yet
    // If it was not explicitly invoked yet we will just incorporate the setup step here
    if ( !taskConsumer->GetIsInPhasedSetup() )
    {
        CFutureResult future = SetupSingularTask( taskConsumer, taskData, assumeOwnershipOfTaskData );
        if GUCEF_PREDICT_FALSE( future.HasNoFuture() )
            return future;
    }

    // IMPORTANT: We remove the flag to signal to the delegator it should commence operations
    taskConsumer->SetIsInPhasedSetup( false );
    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Task of task type \"" + taskConsumer->GetType() +
            "\" with task ID " + ToString( taskConsumer->GetCurrentTaskId() ) + 
            ", linked to thread " + ToString( taskConsumer->GetDelegatorThreadId() ) + ", is instructed to commence work" );
    return taskConsumer->GetCurrentTask();
}

/*-------------------------------------------------------------------------*/

CFutureResult
CThreadPool::StartTask( const CString& taskType        ,
                        CICloneable* taskData          ,
                        bool assumeOwnershipOfTaskData )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    if ( !m_acceptNewWork )
    {        
        return CTask::CreateSharedObjWithParam( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );
    }

    CTaskConsumerPtr taskConsumer;
    if ( !GetOrCreateTaskConsumerOfType( taskType, taskConsumer ) || taskConsumer.IsNULL() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "ThreadPool(" + m_poolName + "): Failed to start task of type \"" + taskType +
            " because no such task type is known" );

        return CTask::CreateSharedObjWithParam( TTaskStatus::TASKSTATUS_TASKTYPE_INVALID );
    }

    CFutureResult taskFuture = SetupSingularTaskImpl( taskConsumer, taskData, assumeOwnershipOfTaskData, GUCEF_NULL, true );
    if ( taskFuture.HasAFuture() )
    {
        // IMPORTANT: We remove the flag to signal to the delegator it should commence operations
        taskConsumer->SetIsInPhasedSetup( false );            
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Task of task type \"" + taskType +
                "\" with task ID " + ToString( taskConsumer->GetCurrentTaskId() ) + 
                ", linked to thread " + ToString( taskConsumer->GetDelegatorThreadId() ) + ", is instructed to commence work" );

        return taskFuture;
    }
    else
    {
        GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "ThreadPool(" + m_poolName + "): Failed to start task of type \"" + taskType +
            "\" with task ID " + UInt32ToString( taskConsumer->GetCurrentTaskId() )  + " because task setup failed" );
        return taskFuture;
    }
}

/*-------------------------------------------------------------------------*/

CFutureResult
CThreadPool::StartTask( const CString& taskType        ,
                        const CDataNode& taskData      ,
                        bool assumeOwnershipOfTaskData )
{GUCEF_TRACE;

    TAbstractTaskDataFactory::TProductPtr taskDataPtr = m_taskDataFactory.Create( taskType );
    if ( taskDataPtr.IsNULL() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "):StartTask: Task of type \"" + taskType + "\" cannot be created because no task data factory exists for the given type" );
        return CTask::CreateSharedObjWithParam( TTaskStatus::TASKSTATUS_TASKTYPE_INVALID );
    }

    CDataNodeSerializableSettings defaultSerializerSettings;
    if ( !taskDataPtr->Deserialize( taskData, defaultSerializerSettings ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "):StartTask: Task of type \"" + taskType + "\" cannot be created because deserialization of the task data failed" );
        return CTask::CreateSharedObjWithParam( TTaskStatus::TASKSTATUS_TASKDATA_INVALID );
    }

    // Now start the task with the data we constructed    
    return StartTask( taskType, taskDataPtr.GetPointerAlways(), assumeOwnershipOfTaskData );
}

/*-------------------------------------------------------------------------*/

bool 
CThreadPool::TaskOfTypeExists( const CString& taskType  , 
                               UInt32* taskIdIfExists   , 
                               UInt32* threadIdIfExists ,
                               TTaskStatus* taskStatus  ) const
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock readerLock( this );
    
    TTaskId2TaskPtrMap::const_iterator i = m_inUseTaskObjs.begin();
    while ( i != m_inUseTaskObjs.end() )
    {
        CTaskPtr task = (*i).second;

        readerLock.EarlyReaderUnlock();

        if ( !task.IsNULL() && taskType == task->GetTaskType() )
        {
            if ( GUCEF_NULL != taskIdIfExists )
                *taskIdIfExists = task->GetTaskId();    
            if ( GUCEF_NULL != threadIdIfExists )
                *threadIdIfExists = task->GetThreadId(); 
            if ( GUCEF_NULL != taskStatus )
                *taskStatus = task->GetTaskStatus(); 

            return true;
        }
        ++i;
    }

    if ( GUCEF_NULL != taskIdIfExists )
        *taskIdIfExists = 0;    
    if ( GUCEF_NULL != threadIdIfExists )
        *threadIdIfExists = 0; 
    if ( GUCEF_NULL != taskStatus )
        *taskStatus = TTaskStatus::TASKSTATUS_UNDEFINED; 

    return false;
}

/*-------------------------------------------------------------------------*/

CFutureResult
CThreadPool::StartTaskIfNoneExists( const CString& taskType        ,
                                    CICloneable* taskData          ,
                                    bool assumeOwnershipOfTaskData )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    // Check if a task of the type given already exists while we have the lock
    UInt32 taskIdIfExists = 0;   
    UInt32 threadIdIfExists = 0;
    TTaskStatus taskStatus = TTaskStatus::TASKSTATUS_UNDEFINED;
    if ( TaskOfTypeExists( taskType, &taskIdIfExists, &threadIdIfExists, &taskStatus ) )
    {
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "):StartTaskIfNoneExists: Task of type \"" + taskType + "\" with ID " +
                                            UInt32ToString( taskIdIfExists ) + " already exists and its using thread with ID " + UInt32ToString( threadIdIfExists ) );
        return CTask::CreateSharedObjWithParam( TTaskStatus::TASKSTATUS_TASK_WITH_TYPE_EXISTS );
    }


    // No such task exists, just create a new one    
    return StartTask( taskType, taskData, assumeOwnershipOfTaskData );
}

/*-------------------------------------------------------------------------*/

CFutureResult 
CThreadPool::StartTaskIfNoneExists( const CString& taskType   ,
                                    const CDataNode& taskData )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    // Check if a task of the type given already exists while we have the lock
    UInt32 taskIdIfExists = 0;   
    UInt32 threadIdIfExists = 0;
    TTaskStatus taskStatus = TTaskStatus::TASKSTATUS_UNDEFINED;
    if ( TaskOfTypeExists( taskType, &taskIdIfExists, &threadIdIfExists, &taskStatus ) )
    {
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "):StartTaskIfNoneExists: Task of type \"" + taskType + "\" with ID " +
                                            UInt32ToString( taskIdIfExists ) + " already exists and its using thread with ID " + UInt32ToString( threadIdIfExists ) );
        return CTask::CreateSharedObjWithParam( TTaskStatus::TASKSTATUS_TASK_WITH_TYPE_EXISTS );
    }

    // No such task exists, just create a new one
    // First we must construct the task data
    
    TAbstractTaskDataFactory::TProductPtr taskDataPtr = m_taskDataFactory.Create( taskType );
    if ( taskDataPtr.IsNULL() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "):StartTaskIfNoneExists: Task of type \"" + taskType + "\" cannot be created because no task data factory exists for the given type" );
        return CTask::CreateSharedObjWithParam( TTaskStatus::TASKSTATUS_TASKTYPE_INVALID );
    }

    CDataNodeSerializableSettings defaultSerializerSettings;
    if ( !taskDataPtr->Deserialize( taskData, defaultSerializerSettings ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "):StartTaskIfNoneExists: Task of type \"" + taskType + "\" cannot be created because deserialization of the task data failed" );
        return CTask::CreateSharedObjWithParam( TTaskStatus::TASKSTATUS_TASKDATA_INVALID );
    }

    // Now start the task with the data we constructed    
    return StartTask( taskType, taskDataPtr.GetPointerAlways() );
}

/*-------------------------------------------------------------------------*/

void
CThreadPool::TaskCleanup( CTaskPtr task )
{GUCEF_TRACE;

    if ( task.IsNULL() )
        return;

    // Make sure we update the bi-directional link, breaking it
    // Otherwise you will end up with a memory leak
    CTaskConsumerPtr taskConsumer = task->GetTaskConsumer();
    task->SetTaskConsumer( CTaskConsumerPtr() );
    if ( !taskConsumer.IsNULL() )
    {
        taskConsumer->SetCurrentTask( CTaskPtr() );
        taskConsumer->SetTaskDelegator( TTaskDelegatorBasicPtr() );
    }

    // Update our 'free lists' administration for the task    
    if ( task->IsTaskPartOfAChain() )
    {
        if ( task->IsLastTaskInAChain() )
        {
            // first locally gather all tasks in the chain
            // we need this local copy since we are about to break the chain apart
            CTask::TTaskPtrSet chainTasks;
            task->GetAllTasksInChain( chainTasks );

            MT::CObjectScopeLock lock( this );

            // now process the individual tasks
            CTask::TTaskPtrSet::iterator i = chainTasks.begin();
            while ( i != chainTasks.end() )
            {
                CTaskPtr chainedTask = (*i);
                m_inUseTaskObjs.erase( chainedTask->GetTaskId() );
                m_allTaskObjs.MarkDormant( chainedTask );
                ++i;
            }
            m_allTaskObjs.MarkDormant( task );
        }
        // else: for a chain its all or nothing, so we do not add it to the free list
    }
    else
    {
        // If there is only one task its always the last in the chain
        MT::CObjectScopeLock lock( this );

        m_inUseTaskObjs.erase( task->GetTaskId() );
        m_allTaskObjs.MarkDormant( task );
    }
    

    // Update our 'free lists' administration for the task consumer
    if ( !taskConsumer.IsNULL() && taskConsumer->IsOwnedByThreadPool() )
    {
        MT::CObjectScopeLock lock( this );
        m_freeTaskConsumers[ taskConsumer->GetType() ].insert( taskConsumer );
    }
}

/*-------------------------------------------------------------------------*/

void
CThreadPool::OnTaskStartup( CTaskPtr task )
{GUCEF_TRACE;

    if ( !task.IsNULL() )
    {
        TTaskStartupEventData eData( task->GetTaskId() ); 
        NotifyObserversFromThread( TaskStartupEvent, &eData ); 
    }
}

/*-------------------------------------------------------------------------*/

void
CThreadPool::OnTaskStarted( CTaskPtr task )
{GUCEF_TRACE;

    if ( !task.IsNULL() )
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Task with ID " + ToString( task->GetTaskId() ) + " started" );

        TTaskStartedEventData eData( task->GetTaskId() ); 
        NotifyObserversFromThread( TaskStartedEvent, &eData ); 
    }
}

/*-------------------------------------------------------------------------*/

void
CThreadPool::OnTaskStartupFailed( CTaskPtr task )
{GUCEF_TRACE;

    if ( !task.IsNULL() )
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Task with ID " + ToString( task->GetTaskId() ) + " failed startup" );

        TTaskStartupFailedEventData eData( task->GetTaskId() ); 
        NotifyObserversFromThread( TaskStartupFailedEvent, &eData ); 
    }
}

/*-------------------------------------------------------------------------*/

void
CThreadPool::OnTaskKilled( CTaskPtr task )
{GUCEF_TRACE;

    if ( !task.IsNULL() )
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Task with ID " + ToString( task->GetTaskId() ) + " was killed" );

        TTaskKilledEventData eData( task->GetTaskId() ); 
        NotifyObserversFromThread( TaskKilledEvent, &eData ); 
    }
}

/*-------------------------------------------------------------------------*/

void
CThreadPool::OnTaskStopped( CTaskPtr task )
{GUCEF_TRACE;

    if ( !task.IsNULL() )
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Task with ID " + ToString( task->GetTaskId() ) + " was stopped" );

        TTaskStoppedEventData eData( task->GetTaskId() ); 
        NotifyObserversFromThread( TaskStoppedEvent, &eData ); 
    }
}

/*-------------------------------------------------------------------------*/

void
CThreadPool::OnTaskPaused( CTaskPtr task )
{GUCEF_TRACE;

    if ( !task.IsNULL() )
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Task with ID " + ToString( task->GetTaskId() ) + " was paused" );

        TTaskPausedEventData eData( task->GetTaskId() ); 
        NotifyObserversFromThread( TaskPausedEvent, &eData ); 
    }
}

/*-------------------------------------------------------------------------*/

void
CThreadPool::OnTaskResumed( CTaskPtr task )
{GUCEF_TRACE;

    if ( !task.IsNULL() )
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Task with ID " + ToString( task->GetTaskId() ) + " has resumed" );

        TTaskResumedEventData eData( task->GetTaskId() ); 
        NotifyObserversFromThread( TaskResumedEvent, &eData ); 
    }
}

/*-------------------------------------------------------------------------*/

void
CThreadPool::OnTaskFinished( CTaskPtr task )
{GUCEF_TRACE;

    if ( !task.IsNULL() )
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Task with ID " + ToString( task->GetTaskId() ) + " has finished" );

        TTaskFinishedEventData eData( task->GetTaskId() ); 
        NotifyObserversFromThread( TaskFinishedEvent, &eData );

        if ( task->IsTaskPartOfAChain() )
        {
            CTaskPtr nextTask = task->GetNextTask();
            if ( !nextTask.IsNULL() )
            {
                CFutureResult future = QueueTask( nextTask );
                if ( future.HasNoFuture() )
                {
                    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "):OnTaskFinished: Failed to queue next task in chain (id=" +
                        ToString( nextTask->GetTaskId() ) + ") for task with ID " + ToString( task->GetTaskId() ) );
                }
                else
                {
                    GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "):OnTaskFinished: Queued next task in chain (id=" +
                        ToString( nextTask->GetTaskId() ) + ") for task with ID " + ToString( task->GetTaskId() ) );
                }
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

CThreadPool::TTaskDelegatorBasicPtr
CThreadPool::GetDelegatorForTaskId( TIntegerTypeUsedForTaskId taskID ,
                                    bool& taskExists                 ) const
{GUCEF_TRACE;

    taskExists = false;
    MT::CObjectScopeReadOnlyLock readLock( this );

    TTaskId2TaskPtrMap::const_iterator i = m_inUseTaskObjs.find( taskID );
    if ( i != m_inUseTaskObjs.end() )
    {
        CTaskPtr task = (*i).second;

        readLock.EarlyReaderUnlock();

        if ( !task.IsNULL() )
        {
            taskExists = true;
            CTaskConsumerPtr taskConsumer = task->GetTaskConsumer();
            if ( !taskConsumer.IsNULL() )
            {
                TTaskDelegatorBasicPtr delegator = taskConsumer->GetTaskDelegator();
                return delegator;
            }
        }
    }

    return CThreadPool::TTaskDelegatorBasicPtr();
}

/*-------------------------------------------------------------------------*/

bool
CThreadPool::PauseTask( const UInt32 taskID          ,
                        const bool force             ,
                        const bool okIfTaskIsUnknown )
{GUCEF_TRACE;

    bool taskExists = false;
    TTaskDelegatorBasicPtr delegator = GetDelegatorForTaskId( taskID, taskExists );
    if ( !delegator.IsNULL() )
    {
        delegator->Pause( force );                    

        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Paused task with ID " + ToString( taskID ) );
        TTaskPausedEventData eData( taskID );
        NotifyObserversFromThread( TaskPausedEvent, &eData );
        return true;
    }
    else
    {
        if ( !taskExists )
        {
            GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Cannot pause task with ID " + ToString( taskID ) + ". It is not known to this pool" );
            return okIfTaskIsUnknown;
        }
        return true;
    }
}

/*-------------------------------------------------------------------------*/

bool
CThreadPool::ResumeTask( const UInt32 taskID          ,
                         const bool okIfTaskIsUnknown )
{GUCEF_TRACE;

    bool taskExists = false;
    TTaskDelegatorBasicPtr delegator = GetDelegatorForTaskId( taskID, taskExists );
    if ( !delegator.IsNULL() )
    {
        delegator->Resume();

        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Resumed task with ID " + ToString( taskID ) );
        TTaskResumedEventData eData( taskID );
        NotifyObserversFromThread( TaskResumedEvent, &eData );
        return true;
    }
    else
    {
        if ( !taskExists )
        {
            GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Cannot resume task with ID " + ToString( taskID ) + ". It is not known to this pool" );
            return okIfTaskIsUnknown;
        }
        return true;
    }
}

/*-------------------------------------------------------------------------*/

bool
CThreadPool::RequestTaskToStop( const UInt32 taskID    ,
                                bool callerShouldWait  ,
                                bool okIfTaskIsUnknown )
{GUCEF_TRACE;

    bool taskExists = false;
    TTaskDelegatorBasicPtr delegator = GetDelegatorForTaskId( taskID, taskExists );
    if ( !delegator.IsNULL() )
    {
        delegator->Deactivate( false, callerShouldWait );

        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Requested task with ID " + ToString( taskID ) + " to stop" );
        TTaskResumedEventData eData( taskID );
        NotifyObserversFromThread( TaskResumedEvent, &eData );
        return true;
    }
    else
    {
        if ( taskExists )
        {
            GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): task with ID " + ToString( taskID ) + " is known but not active" );
            return true;
        }
        else
        {
            GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Cannot ask task with ID " + ToString( taskID ) + " to stop. It is not known to this pool" );
            return okIfTaskIsUnknown;
        }        
    }
}

/*-------------------------------------------------------------------------*/

bool
CThreadPool::RequestTaskToStop( CTaskConsumerPtr taskConsumer ,
                                bool callerShouldWait         )
{GUCEF_TRACE;

    if ( !taskConsumer.IsNULL() )
        return RequestTaskToStop( taskConsumer->GetCurrentTaskId(), callerShouldWait );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CThreadPool::RequestTaskCancellation( const UInt32 taskId    , 
                                      bool callerShouldWait  ,
                                      bool okIfTaskIsUnknown )
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock readLock( this );

    //TTaskId2TaskPtrMap::const_iterator i = m_inUseTaskObjs.find( taskId );
    //if ( i != m_inUseTaskObjs.end() )
    //{
    //    CTaskPtr primaryTask = (*i).second;
    //    if ( !primaryTask.IsNULL() )
    //    {
    //        CTask::TTaskPtrSet upcomingTasks;
    //        primaryTask->GetAllUpcomingTasksInChain( upcomingTasks );
    //        m_taskQueue.Delete( upcomingTasks );

    //        m_inUseTaskObjs.erase( i );
    //        m_taskQueue.Delete( task ); 
    //    }


    //    readLock.EarlyReaderUnlock();

    //    if ( !task.IsNULL() )
    //    {
    //        taskExists = true;
    //        CTaskConsumerPtr taskConsumer = task->GetTaskConsumer();
    //        if ( !taskConsumer.IsNULL() )
    //        {
    //            TTaskDelegatorBasicPtr delegator = taskConsumer->GetTaskDelegator();
    //            return delegator;
    //        }
    //    }
    //}

    return false;
}

/*-------------------------------------------------------------------------*/

CTaskPtr
CThreadPool::GetTaskObjById( TIntegerTypeUsedForTaskId taskId ) const
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock readerLock( this );

    TTaskId2TaskPtrMap::const_iterator i = m_inUseTaskObjs.find( taskId );
    if ( i != m_inUseTaskObjs.end() )
    {
        CTaskPtr task = (*i).second;
        readerLock.EarlyReaderUnlock();

        return task;
    }
    return CTaskPtr();
}

/*-------------------------------------------------------------------------*/

CThreadPool::TTaskDelegatorBasicPtr
CThreadPool::GetDelegatorForThreadId( const UInt32 threadId ) const
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock lock( this );

    TTaskDelegatorSet::const_iterator i = m_taskDedicatedDelegators.begin();
    while ( i != m_taskDedicatedDelegators.end() )
    {
        const TTaskDelegatorBasicPtr& delegator = (*i);
        if ( !delegator.IsNULL() && delegator->GetThreadID() == threadId )
        {
            return delegator;
        }
        ++i;
    }
    i = m_taskGenericDelegators.begin();
    while ( i != m_taskGenericDelegators.end() )
    {
        const TTaskDelegatorBasicPtr& delegator = (*i);
        if ( !delegator.IsNULL() && delegator->GetThreadID() == threadId )
        {
            return delegator;
        }
        ++i;
    }
    
    return TTaskDelegatorBasicPtr();
}

/*-------------------------------------------------------------------------*/

bool
CThreadPool::WaitForThreadToFinish( const UInt32 threadId ,
                                    Int32 timeoutInMs     )
{GUCEF_TRACE;

    TTaskDelegatorBasicPtr delegator = GetDelegatorForThreadId( threadId );
    if ( !delegator.IsNULL() )
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "):WaitForThreadToFinish: Waiting for thread with ID " + ToString( threadId ) + " to finish" );
        UInt32 waitResult = delegator->WaitForThreadToFinish( timeoutInMs );

        if ( GUCEF_THREAD_WAIT_OK == waitResult         || 
             GUCEF_THREAD_WAIT_ABANDONEND == waitResult )
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "):WaitForThreadToFinish: Successfully waited for thread with ID " + ToString( threadId ) + " to finish" );
            return true;
        }
        else
        if ( GUCEF_THREAD_WAIT_TIMEOUT == waitResult )
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "):WaitForThreadToFinish: Timeout occurred while waiting for thread with ID " + ToString( threadId ) + " to finish" );
            return false;
        }
        else
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "):WaitForThreadToFinish: Error occurred while waiting for thread with ID " + ToString( threadId ) + " to finish" );
            return false;
        }
    }

    // No such thread as such we consider it already finished
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CThreadPool::WaitForTaskToFinish( const UInt32 taskId ,
                                  Int32 timeoutInMs   )
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock readerLock( this );

    TTaskId2TaskPtrMap::iterator i = m_inUseTaskObjs.find( taskId );
    if ( i != m_inUseTaskObjs.end() )
    {
        CTaskPtr task = (*i).second;

        readerLock.EarlyReaderUnlock();

        if ( !task.IsNULL() )
        {
            UInt32 workerThreadId = task->GetThreadId();
            UInt32 callerThreadId = MT::GetCurrentTaskID();

            if ( workerThreadId != callerThreadId )
            {
                GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "):WaitForTaskToFinish: Waiting for task with ID " + ToString( taskId ) + " to finish" );

                MT::TLockStatus lockStatus = task->WaitForTaskToFinish( timeoutInMs );

                if ( MT::LOCKSTATUS_OPERATION_SUCCESS == lockStatus )
                {
                    GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "):WaitForTaskToFinish: Successfully waited for task with ID " + ToString( taskId ) + " to finish" );
                    return true;
                }
                else if ( MT::LOCKSTATUS_WAIT_TIMEOUT == lockStatus )
                {
                    GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "):WaitForTaskToFinish: Timeout occurred while waiting for task with ID " + ToString( taskId ) + " to finish" );
                    return false;
                }
                else
                {
                    GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "):WaitForTaskToFinish: Error occurred while waiting for task with ID " + ToString( taskId ) + " to finish" );
                    return false;
                }
            }
            else
            {
                GUCEF_ERROR_LOG( LOGLEVEL_CRITICAL, "ThreadPool(" + m_poolName + "):WaitForTaskToFinish: Task with ID " + ToString( taskId ) + 
                    " is requested to be waited on from thread " + ToString( callerThreadId ) + 
                    " which is also the thread running the task. This is not allowed to prevent deadlocks" );                    
                return false;
            }
        }
        else
        {
            // If a consumer does not have a delegator then it hasnt started yet
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "):WaitForTaskToFinish: No such Task with ID " + ToString( taskId ) );
            return true;
        }
    }
    else
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "):WaitForTaskToFinish: No such Task with ID " + ToString( taskId ) );
        return true;
    }
}

/*-------------------------------------------------------------------------*/

bool
CThreadPool::WaitForTaskToFinish( CTaskConsumerPtr taskConsumer ,
                                  Int32 timeoutInMs             )
{GUCEF_TRACE;

    if ( !taskConsumer.IsNULL() )
        return WaitForTaskToFinish( taskConsumer->GetCurrentTaskId(), timeoutInMs );
    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CThreadPool::WaitForAllTasksToFinish( Int32 timeoutInMs )
{GUCEF_TRACE;

    // first of all we cannot be done waiting on 'ALL' tasks if the queue still has work
    // as such we simply check if there is any mail left in the mailbox or if all remaining
    // mailbox content has been grabbed already for delivery
    
    MT::TLockStatus waitStatus = m_taskQueue.WaitForZeroMailInMailbox( timeoutInMs );
    if ( MT::LOCKSTATUS_OPERATION_SUCCESS == waitStatus )
    {
        // If there is no mail left waiting to be picked up we can go check the actual 'deliveries'
        // if our postal workers (threads) are finished with their tasks or not
        // This is a little more expensive than waiting for the mailbox to clear which is why we wait for that first

        TTaskIdVector taskIds;
        GetAllTaskIdsForTasksWithWork( taskIds, true );

        bool totalSuccess = true;
        TTaskIdVector::iterator i = taskIds.begin();
        while ( i != taskIds.end() )
        {
            totalSuccess = WaitForTaskToFinish( (*i), timeoutInMs ) && totalSuccess;
            ++i;
        }
        return totalSuccess;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CThreadPool::WaitForAllThreadsToFinish( Int32 timeoutInMs )
{GUCEF_TRACE;

    TThreadIdVector threadIds;
    GetAllCurrentThreadIds( threadIds );

    bool totalSuccess = true;
    TThreadIdVector::iterator i = threadIds.begin();
    while ( i != threadIds.end() )
    {
        totalSuccess = WaitForThreadToFinish( (*i), timeoutInMs ) && totalSuccess;
        ++i;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

void
CThreadPool::GetAllTaskIdsForTasksWithWork( TTaskIdVector& taskIds, bool includeQueued ) const
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock readLock( this );

    if ( includeQueued )
    {
        MT::CScopeMutex mailboxReader( m_taskQueue.GetLock() );

        TTaskMailbox::const_iterator i = m_taskQueue.begin( mailboxReader );
        while ( i != m_taskQueue.end() )
        {
            const TTaskMailbox::Mail& mailElement = (*i);
            const CTaskPtr& task = mailElement.eventid;
            taskIds.push_back( task->GetTaskId() );

            ++i;
        }
    }

    TTaskId2TaskPtrMap::const_iterator n = m_inUseTaskObjs.begin();
    while ( n != m_inUseTaskObjs.end() )
    {
        taskIds.push_back( (*n).second->GetTaskId() );
        ++n;
    }
}

/*-------------------------------------------------------------------------*/

void 
CThreadPool::GetAllCurrentThreadIds( TThreadIdVector& threadIds ) const
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock lock( this );

    TTaskDelegatorSet::const_iterator i = m_taskDedicatedDelegators.begin();
    while ( i != m_taskDedicatedDelegators.end() )
    {
        const TTaskDelegatorBasicPtr& delegator = (*i);
        if ( !delegator.IsNULL() )
        {
            threadIds.push_back( delegator->GetThreadID() );
        }
        ++i;
    }
    i = m_taskGenericDelegators.begin();
    while ( i != m_taskGenericDelegators.end() )
    {
        const TTaskDelegatorBasicPtr& delegator = (*i);
        if ( !delegator.IsNULL() )
        {
            threadIds.push_back( delegator->GetThreadID() );
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
CThreadPool::RequestAllTasksToStop( bool waitOnStop, bool acceptNewWork, Int32 timeoutInMs  )
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Requesting all tasks to stop" );

    // First signal to all tasks that we want them to stop doing work.
    // We don't wait yet for them to finish, we do that after signaling all of them and releasing the lock
    // Note that the administration we use to find all tasks is the same one that would be updated by the action
    // as such a 2 step process is needed

    MT::CObjectScopeReadOnlyLock readLock( this );

    m_acceptNewWork = acceptNewWork;

    TTaskId2TaskPtrMap::iterator i = m_inUseTaskObjs.begin();
    while ( i != m_inUseTaskObjs.end() )
    {
        CTaskPtr& task = (*i).second; 
        TTaskDelegatorBasicPtr delegator = task->GetDelegator();
        if ( !delegator.IsNULL() )
        {
            delegator->Deactivate( false, false );

            GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Requested task with ID " + ToString( task->GetTaskId() ) + " to stop" );
        }
        ++i;
    }

    readLock.EarlyReaderUnlock();

    // Now comes step 2: Waiting for all tasks to finish
    if ( waitOnStop )
    {
        // Note that this could be considered a snapshot in time when there is no work since its possible new work could come in
        // the relevance is up to the application usage of this functionality
        if ( !WaitForAllTasksToFinish( timeoutInMs ) )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "):RequestAllTasksToStop: Failed to wait for all tasks to finish" );
            return false;
        }
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "):RequestAllTasksToStop: Completed wait on all tasks to stop" );
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CThreadPool::RequestAllThreadsToStop( bool waitOnStop, bool acceptNewWork, Int32 timeoutInMs )
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Requesting all tasks to stop" );

    MT::CObjectScopeLock lock( this );
    m_acceptNewWork = acceptNewWork;
    Int32 threadDeltaNeeded = EnforceDesiredNrOfThreads( 0, 0, true );
    lock.EarlyUnlock();

    if ( 0 != threadDeltaNeeded && waitOnStop )
    {
        if ( !WaitForAllTasksToFinish( timeoutInMs ) )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "):RequestAllThreadsToStop: Failed to wait for all tasks to finish" );
            return false;
        }
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "):RequestAllThreadsToStop: Completed wait on all tasks to stop" );

        if ( !WaitForAllThreadsToFinish( timeoutInMs ) )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "):RequestAllThreadsToStop: Failed to wait for all threads to finish" );
            return false;
        }
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "):RequestAllThreadsToStop: Completed wait on all threads to stop" );
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CThreadPool::KillThreadWorkingOnTask( const UInt32 taskID )
{GUCEF_TRACE;

    bool taskExists = false;
    TTaskDelegatorBasicPtr delegator = GetDelegatorForTaskId( taskID, taskExists );
    if ( !delegator.IsNULL() )
    {                     
        UInt32 threadId = delegator->GetThreadID();

        delegator->Deactivate( true, true );

        GUCEF_SYSTEM_LOG( LOGLEVEL_VERY_IMPORTANT, "ThreadPool(" + m_poolName + "): Killing task with ID " + ToString( taskID ) + " to stop" );
        TTaskKilledEventData eData( taskID );
        NotifyObserversFromThread( TaskKilledEvent, &eData );
        return true;
    }
    else
    {
        if ( taskExists )
        {
            GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): task with ID " + ToString( taskID ) + " is known but not active" );
            return true;
        }
        else
        {
            GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ThreadPool(" + m_poolName + "): Cannot ask task with ID " + ToString( taskID ) + " to stop. It is not known to this pool" );
            return false;
        }        
    }
}

/*-------------------------------------------------------------------------*/

void
CThreadPool::GetAllRegisteredTaskConsumerFactoryTypes( CORE::CString::StringSet& taskTypes ) const
{GUCEF_TRACE;

    m_consumerFactory.ObtainKeySet( taskTypes );
}

/*-------------------------------------------------------------------------*/

void
CThreadPool::GetAllRegisteredTaskDataFactoryTypes( CORE::CString::StringSet& taskTypes ) const
{GUCEF_TRACE;

    m_taskDataFactory.ObtainKeySet( taskTypes );
}

/*-------------------------------------------------------------------------*/

void
CThreadPool::RegisterTaskDataFactory( const CString& taskType   ,
                                      TTaskDataFactory* factory )
{GUCEF_TRACE;

    m_taskDataFactory.RegisterConcreteFactory( taskType, factory );
}

/*-------------------------------------------------------------------------*/

void
CThreadPool::UnregisterTaskDataFactory( const CString& taskType )
{GUCEF_TRACE;

    m_taskDataFactory.UnregisterConcreteFactory( taskType );
}

/*-------------------------------------------------------------------------*/

bool 
CThreadPool::IsTaskOfTaskTypeExecutable( const CString& taskType ) const
{GUCEF_TRACE;

    return m_taskDataFactory.IsConstructible( taskType );
}

/*-------------------------------------------------------------------------*/

bool 
CThreadPool::IsCustomTaskDataForTaskTypeSerializable( const CString& taskType ) const
{GUCEF_TRACE;

    return m_taskDataFactory.IsConstructible( taskType );
}

/*-------------------------------------------------------------------------*/

CIDataNodeSerializableTaskDataBasicPtr
CThreadPool::CreateCustomTaskDataForTaskTypeIfAvailable( const CString& taskType ) const
{GUCEF_TRACE;

    return m_taskDataFactory.Create( taskType );
}

/*-------------------------------------------------------------------------*/

bool
CThreadPool::GetTaskIdForThreadId( const UInt32 threadId ,
                                   UInt32& taskId        ) const
{GUCEF_TRACE;

    taskId = 0;
    MT::CObjectScopeReadOnlyLock readerLock( this );

    TTaskDelegatorSet::const_iterator i = m_taskDedicatedDelegators.begin();
    while ( i != m_taskDedicatedDelegators.end() )
    {
        const TTaskDelegatorBasicPtr& delegator = (*i);
        if ( !delegator.IsNULL() )
        {
            if ( threadId == delegator->GetThreadID() )
            {
                taskId = delegator->GetCurrentTaskId();
                return true;
            }
        }
        ++i;
    }
    i = m_taskGenericDelegators.begin();
    while ( i != m_taskGenericDelegators.end() )
    {
        const TTaskDelegatorBasicPtr& delegator = (*i);
        if ( !delegator.IsNULL() )
        {
            if ( threadId == delegator->GetThreadID() )
            {
                taskId = delegator->GetCurrentTaskId();
                return true;
            }
        }
        ++i;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CThreadPool::GetThreadIdForTaskId( const UInt32 taskId ,
                                   UInt32& threadId    ) const
{GUCEF_TRACE;

    threadId = 0;
    MT::CObjectScopeReadOnlyLock readerLock( this );

    TTaskId2TaskPtrMap::const_iterator i = m_inUseTaskObjs.find( taskId );
    if ( i != m_inUseTaskObjs.end() )
    {
        CTaskPtr task = (*i).second;
        if ( !task.IsNULL() )
        {
            threadId = task->GetThreadId();
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CThreadPool::GetInfo( CThreadPoolInfo& info ) const
{GUCEF_TRACE;

    info.Clear();
    
    // Obtain an overall lock to get a coherent snapshot of thread pool info
    MT::CObjectScopeReadOnlyLock lock( this );

    info.SetActiveNrOfDedicatedThreads( (UInt32) m_taskDedicatedDelegators.size() );
    info.SetActiveNrOfWorkerThreads( (UInt32) m_taskGenericDelegators.size() );
    info.SetDesiredMaxNrOfThreads( m_desiredMaxTotalNrOfThreads );
    info.SetDesiredMinNrOfWorkerThreads( m_desiredMinNrOfWorkerThreads );
    info.SetAllowMainApplicationThreadToPickUpQueuedTasks( m_allowAppThreadToWork );
    m_consumerFactory.ObtainKeySet( info.GetTaskConsumerFactoryTypes() );
    m_taskDataFactory.ObtainKeySet( info.GetTaskDataFactoryTypes() );
    info.SetQueuedTaskCount( m_taskQueue.AmountOfMail() );
    info.SetThreadPoolName( m_poolName );

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CThreadPool::GetTaskInfo( UInt32 taskId                                                   , 
                          CTaskInfo& info                                                 , 
                          bool obtainTaskDataCopyIfPossible                               ,
                          const CDataNodeSerializableSettings* taskDataSerializerSettings ) const
{GUCEF_TRACE;

    info.Clear();
    MT::CObjectScopeReadOnlyLock lock( this );                                                   

    TTaskId2TaskPtrMap::const_iterator i = m_inUseTaskObjs.find( taskId );
    if ( i != m_inUseTaskObjs.end() )
    {
        CTaskPtr task = (*i).second;
        if ( !task.IsNULL() )
        {
            lock.EarlyReaderUnlock();
            
            info.SetTaskId( taskId );
            info.SetThreadId( task->GetThreadId() );
            info.SetTaskStatus( task->GetTaskStatus() );
            info.SetTaskTypeName( task->GetTaskType() );
            info.SetCustomTaskDataIsSerializable( IsCustomTaskDataForTaskTypeSerializable( info.GetTaskTypeName() ) );
            info.SetHasTaskData( task->HasTaskData() );
            if ( obtainTaskDataCopyIfPossible && info.GetHasTaskData() && info.GetCustomTaskDataIsSerializable() )
            {
                if ( GUCEF_NULL != taskDataSerializerSettings )
                {
                    task->GetSerializedTaskDataCopy( info.GetCustomTaskData(), *taskDataSerializerSettings );
                }
                else
                {
                    CDataNodeSerializableSettings defaultSerializableSettings;
                    task->GetSerializedTaskDataCopy( info.GetCustomTaskData(), defaultSerializableSettings );
                }
            }
            return true;
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CThreadPool::GetAllTaskInfo( TTaskInfoMap& info                                              ,
                             bool obtainTaskDataCopyIfPossible                               ,
                             const CDataNodeSerializableSettings* taskDataSerializerSettings ) const
{GUCEF_TRACE;

    info.clear();
    MT::CObjectScopeReadOnlyLock readerLock( this );

    bool totalSuccess = true;
    TTaskId2TaskPtrMap::const_iterator i = m_inUseTaskObjs.begin();
    while ( i != m_inUseTaskObjs.end() )
    {
        UInt32 taskId = (*i).first;
        CTaskInfo& taskInfo = info[ taskId ];

        totalSuccess = GetTaskInfo( taskId                       , 
                                    taskInfo                     , 
                                    obtainTaskDataCopyIfPossible , 
                                    taskDataSerializerSettings   ) && totalSuccess;
        ++i;
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CThreadPool::GetThreadInfo( UInt32 threadId, CThreadInfo& info ) const
{GUCEF_TRACE;

    info.Clear();
    MT::CObjectScopeReadOnlyLock lock( this );

    TTaskDelegatorSet::const_iterator i = m_taskDedicatedDelegators.begin();
    while ( i != m_taskDedicatedDelegators.end() )
    {
        if ( threadId == (*i)->GetThreadID() )
            return (*i)->GetThreadInfo( info );
        ++i;
    }
    i = m_taskGenericDelegators.begin();
    while ( i != m_taskGenericDelegators.end() )
    {
        if ( threadId == (*i)->GetThreadID() )
            return (*i)->GetThreadInfo( info );
        ++i;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CThreadPool::GetAllThreadInfo( TThreadInfoMap& info ) const
{GUCEF_TRACE;

    info.clear();
    MT::CObjectScopeReadOnlyLock lock( this );

    bool totalSuccess = true;
    TTaskDelegatorSet::const_iterator i = m_taskDedicatedDelegators.begin();
    while ( i != m_taskDedicatedDelegators.end() )
    {
        const TTaskDelegatorBasicPtr& delegator = (*i);
        if ( !delegator.IsNULL() )
        {
            UInt32 threadId = (*i)->GetThreadID();
            CThreadInfo& threadInfo = info[ threadId ];
            totalSuccess = delegator->GetThreadInfo( threadInfo ) && totalSuccess;
        }        
        ++i;
    }
    i = m_taskGenericDelegators.begin();
    while ( i != m_taskGenericDelegators.end() )
    {
        const TTaskDelegatorBasicPtr& delegator = (*i);
        if ( !delegator.IsNULL() )
        {
            UInt32 threadId = (*i)->GetThreadID();
            CThreadInfo& threadInfo = info[ threadId ];
            totalSuccess = delegator->GetThreadInfo( threadInfo ) && totalSuccess;
        }        
        ++i;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CThreadPool::GetSerializedTaskDataCopy( UInt32 taskId                                           ,
                                        CDataNode& domNode                                      ,
                                        const CDataNodeSerializableSettings& serializerSettings ) const
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock lock( this );

    TTaskId2TaskPtrMap::const_iterator i = m_inUseTaskObjs.find( taskId );
    if ( i != m_inUseTaskObjs.end() )
    {
        CTaskPtr task = (*i).second;

        lock.EarlyReaderUnlock();

        if ( !task.IsNULL() )
        {
            return task->GetSerializedTaskDataCopy( domNode, serializerSettings );
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

const CString& 
CThreadPool::GetThreadPoolName( void ) const
{GUCEF_TRACE;
    
    // no lock since this is only ever set in the constructor with no way to change it after the fact
    return m_poolName;  
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
