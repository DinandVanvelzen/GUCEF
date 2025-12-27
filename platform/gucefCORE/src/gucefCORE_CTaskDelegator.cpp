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
#include "gucefMT_dvmtoswrap.h"         /* OS wrappers for threading functionality */
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#include "gucefCORE_CTaskDelegator.h"

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

const CEvent CTaskDelegator::ThreadKilledEvent = "GUCEF::CORE::CTaskDelegator::ThreadKilledEvent";
const CEvent CTaskDelegator::ThreadStartedEvent = "GUCEF::CORE::CTaskDelegator::ThreadStartedEvent";
const CEvent CTaskDelegator::ThreadPausedEvent = "GUCEF::CORE::CTaskDelegator::ThreadPausedEvent";
const CEvent CTaskDelegator::ThreadResumedEvent = "GUCEF::CORE::CTaskDelegator::ThreadResumedEvent";
const CEvent CTaskDelegator::ThreadFinishedEvent = "GUCEF::CORE::CTaskDelegator::ThreadFinishedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CTaskDelegator::RegisterEvents( void )
{GUCEF_TRACE;

    ThreadKilledEvent.Initialize();
    ThreadStartedEvent.Initialize();
    ThreadPausedEvent.Initialize();
    ThreadResumedEvent.Initialize();
    ThreadFinishedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CTaskDelegator::CTaskDelegator( const ThreadPoolPtr& threadPool )
    : MT::CActiveObject()
    , CNotifier()
    , CTSharedPtrCreator< CTaskDelegator, MT::CMutex >( this )
    , CIPulseGeneratorDriver()
    , m_pulseGenerator()
    , m_threadPool( threadPool )
    , m_consumerBusy( false )
    , m_sendRegularPulses( false )
    , m_immediatePulseTickets( 0 )
    , m_immediatePulseTicketMax( 1 )
    , m_taskRequestedCycleDelayInMs( 0 )
{GUCEF_TRACE;

    RegisterEvents();

    GUCEF_ASSERT( !m_threadPool.IsNULL() );

    m_pulseGenerator = CPulseGenerator::CreateSharedObj();
    m_pulseGenerator->SetPulseGeneratorDriver( this );
}

/*-------------------------------------------------------------------------*/

CTaskDelegator::CTaskDelegator( const ThreadPoolPtr& threadPool ,
                                const CTaskPtr& task            )
    : MT::CActiveObject()
    , CNotifier()
    , CTSharedPtrCreator< CTaskDelegator, MT::CMutex >( this )
    , CIPulseGeneratorDriver()
    , m_pulseGenerator()
    , m_task( task )
    , m_threadPool( threadPool )
    , m_consumerBusy( false )
    , m_sendRegularPulses( false )
    , m_immediatePulseTickets( 0 )
    , m_immediatePulseTicketMax( 1 )
    , m_taskRequestedCycleDelayInMs( 0 )
{GUCEF_TRACE;

    RegisterEvents();

    // asserts to ensure we have the required objects for this internal implementation
    GUCEF_ASSERT( !m_threadPool.IsNULL() );
    GUCEF_ASSERT( !m_task.IsNULL() );

    m_pulseGenerator = CPulseGenerator::CreateSharedObj();
    m_pulseGenerator->SetPulseGeneratorDriver( this );

    // If we are being handed the task already we can already establish the bi-directional link
    // this delegator is going to be the one to execute this task
    // This means the task is now assigned to the thread which is represented by this delegator
    CTaskConsumerPtr taskConsumer = m_task->GetTaskConsumer();
    GUCEF_ASSERT( !taskConsumer.IsNULL() );
    taskConsumer->SetTaskDelegator( CreateBasicSharedPtr() );
}

/*-------------------------------------------------------------------------*/

CTaskDelegator::~CTaskDelegator()
{GUCEF_TRACE;

    if ( !NotifyObservers( DestructionEvent ) ) 
        return;

    UnsubscribeAllFromNotifier();
    m_pulseGenerator->SetPulseGeneratorDriver( GUCEF_NULL );
    m_pulseGenerator.Unlink();
    m_threadPool.Unlink();
    
    TaskCleanup( m_task );
    m_task.Unlink();
}

/*-------------------------------------------------------------------------*/

ThreadPoolPtr
CTaskDelegator::GetThreadPool( void )
{GUCEF_TRACE;

    return m_threadPool;
}

/*-------------------------------------------------------------------------*/

PulseGeneratorPtr
CTaskDelegator::GetPulseGenerator( void )
{GUCEF_TRACE;

    return m_pulseGenerator;
}

/*-------------------------------------------------------------------------*/

void
CTaskDelegator::RequestImmediatePulse( CPulseGenerator& pulseGenerator )
{GUCEF_TRACE;

    if ( &pulseGenerator == m_pulseGenerator.GetPointerAlways() )
    {
        m_immediatePulseTickets = m_immediatePulseTicketMax;
    }
}

/*-------------------------------------------------------------------------*/

void
CTaskDelegator::RequestPulsesPerImmediatePulseRequest( CPulseGenerator& pulseGenerator                     ,
                                                       const Int32 requestedPulsesPerImmediatePulseRequest )
{GUCEF_TRACE;

    if ( &pulseGenerator == m_pulseGenerator.GetPointerAlways() )
    {
        if ( requestedPulsesPerImmediatePulseRequest > 1 )
            m_immediatePulseTicketMax = requestedPulsesPerImmediatePulseRequest;
        else
            m_immediatePulseTicketMax = 1;
        m_sendRegularPulses = true;
    }
}

/*-------------------------------------------------------------------------*/

void
CTaskDelegator::RequestPeriodicPulses( CPulseGenerator& pulseGenerator    ,
                                       const UInt32 pulseDeltaInMilliSecs )

{GUCEF_TRACE;

    if ( &pulseGenerator == m_pulseGenerator.GetPointerAlways() )
    {
        m_delayInMilliSecs = pulseDeltaInMilliSecs;
        m_sendRegularPulses = true;
    }
}

/*-------------------------------------------------------------------------*/

void
CTaskDelegator::RequestPulseInterval( CPulseGenerator& pulseGenerator    ,
                                      const UInt32 pulseDeltaInMilliSecs )
{GUCEF_TRACE;

    if ( &pulseGenerator == m_pulseGenerator.GetPointerAlways() )
    {
        m_delayInMilliSecs = pulseDeltaInMilliSecs;
        m_sendRegularPulses = true;
    }
}

/*-------------------------------------------------------------------------*/

void
CTaskDelegator::RequestStopOfPeriodicUpdates( CPulseGenerator& pulseGenerator )
{GUCEF_TRACE;

    if ( &pulseGenerator == m_pulseGenerator.GetPointerAlways() )
    {
        m_sendRegularPulses = false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CTaskDelegator::OnThreadStart( void* taskData )
{GUCEF_TRACE;

    m_pulseGenerator->SetPulseDriverThreadId( GetThreadID() );
    ThreadStartedEventData id( GetThreadID() );
    NotifyObservers( ThreadStartedEvent, &id );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CTaskDelegator::OnThreadStarted( void* taskData )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CTaskDelegator::TaskCleanup( CTaskPtr task )
{GUCEF_TRACE;

    if ( !task.IsNULL() )
    {
        try
        {
            // decouple the bi-directional link with the delegator
            // important to avoid memory leaks
            CTaskConsumerPtr taskConsumer = task->GetTaskConsumer();
            if ( !taskConsumer.IsNULL() ) 
                taskConsumer->SetTaskDelegator( CTaskDelegatorPtr() );

            if ( !m_threadPool.IsNULL() )
            {
                m_threadPool->TaskCleanup( task );
            }
            else
            {
                // We should not get here but just in case
                ThreadPoolPtr threadPool = task->GetThreadPool();
                if ( !threadPool.IsNULL() )
                {
                    threadPool->TaskCleanup( task );
                }
            }
        }
        catch ( const timeout_exception& )
        {
            GUCEF_EXCEPTION_LOG( LOGLEVEL_NORMAL, "TaskDelegator:TaskCleanup: caught timeout_exception while attempting to clean up task of type " + task->GetTaskType() +
                " with id " + ToString( task->GetTaskId() ) );
        }
        catch ( const std::exception& e )
        {
            GUCEF_EXCEPTION_LOG( LOGLEVEL_NORMAL, "TaskDelegator:TaskCleanup: caught std exception while attempting to process task of type " + task->GetTaskType() +
                " with id " + ToString( task->GetTaskId() ) + " - std what = " + CString( e.what() ) );
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CTaskDelegator::OnThreadCycle( void* )
{GUCEF_TRACE;

    if ( ( !m_threadPool.IsNULL() ) &&
         ( m_threadPool->GetQueuedTask( m_task ) ) &&
         !m_task.IsNULL()                           )
    {
        try
        {
            bool attemptTimedOut = false;
            do
            {
                attemptTimedOut = false;
                try
                {
                    ProcessTask( m_task );
                }
                catch ( const timeout_exception& )
                {
                    GUCEF_EXCEPTION_LOG( LOGLEVEL_NORMAL, "TaskDelegator:OnThreadCycle: caught timeout_exception while attempting to process task of type " + m_task->GetTaskType() +
                        " with id " + ToString( m_task->GetTaskId() ) );
                    attemptTimedOut = true;
                }
            }
            while ( attemptTimedOut );
        }
        catch ( const std::exception& e )
        {
            CString errorMessage = "TaskDelegator:OnThreadCycle: caught std exception while attempting to process task of type " + m_task->GetTaskType() +
                " with id " + ToString( m_task->GetTaskId() ) + " - std what = " + CString( e.what() );
            GUCEF_EXCEPTION_LOG( LOGLEVEL_NORMAL, errorMessage );
            m_task->SetTaskStatus( TASKSTATUS_EXCEPTION_DURING_RUN );
            m_task->SetTaskStatusExtraInfo( errorMessage );
        }

        TaskCleanup( m_task );
        m_task.Unlink();
    }

    // Return false, this is an infinite task worker thread
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CTaskDelegator::ProcessTask( CTaskPtr task )
{GUCEF_TRACE;

    bool returnStatus = false;
    bool taskWasFinished = false;

    // Create local refs for the invocation duration to avoid external interference
    PulseGeneratorPtr pulseGenerator = m_pulseGenerator;
    ThreadPoolPtr threadPool = m_threadPool;
    if GUCEF_PREDICT_FALSE( pulseGenerator.IsNULL() || threadPool.IsNULL() || task.IsNULL()  )
        return false;

    // At this point we should have a valid task which has a valid task consumer
    CTaskConsumerPtr taskConsumer = task->GetTaskConsumer();
    if GUCEF_PREDICT_FALSE( taskConsumer.IsNULL() )
        return false;

    // first establish the bi-directional link
    // this delegator is going to be the one to execute this task
    // This means the task is now assigned to the thread which is represented by this delegator
    taskConsumer->SetTaskDelegator( CreateBasicSharedPtr() );

    // optionally the task has associated task data which we can use to pass to the consumer
    // just keep a pointer here
    CICloneable* taskData = task->GetTaskData();

    // Now that the consumer is linked to the delegator wait until we are
    // given the signal that we are ready to begin operations
    while ( taskConsumer->GetIsInPhasedSetup() )
    {
        if ( !IsDeactivationRequested() )
        {
            pulseGenerator->WaitTillNextPulseWindow( 25 );
        }
        else
        {
            break;
        }
    }
    
    if ( !IsDeactivationRequested() )
    {
        // Now we go through the execution sequence within a cycle as if this
        // where a thread sequence
        m_consumerBusy = true;

        // Notify the thread pool directly
        task->SetTaskStatus( TASKSTATUS_STARTUP );
        m_threadPool->OnTaskStartup( task );
        if ( taskConsumer->OnTaskStart( task ) )
        {
            task->SetTaskStatus( TASKSTATUS_RUNNING );
            taskConsumer->OnTaskStarted( task );

            // cycle the task as long as it is not "done"
            while ( !IsDeactivationRequested() )
            {
                try
                {
                    // Perform a cycle directly and ask the task if we are done
                    if ( taskConsumer->OnTaskCycle( task ) )
                    {
                        // Task says we are done
                        taskWasFinished = true;
                        break;
                    }

                    if ( m_sendRegularPulses || m_immediatePulseTickets > 0 )
                        SendDriverPulse( *pulseGenerator.GetPointerAlways() );

                    if ( m_immediatePulseTickets > 0 )
                    {
                        --m_immediatePulseTickets;
                    }
                    else
                    {
                        // Per cycle we allow external deferment of the next cycle which is distinct from the minimum cycle delta
                        // this allows others or the task logic itself to throttle and 'back off' on a per cycle basis as needed
                        if ( m_taskRequestedCycleDelayInMs > 0 && m_taskRequestedCycleDelayInMs > m_minimalCycleDeltaInMilliSecs )
                        {
                            UInt32 timeLeftToWait = m_taskRequestedCycleDelayInMs; 
                            while ( !IsDeactivationRequested() && timeLeftToWait >= m_minimalCycleDeltaInMilliSecs )
                            {
                                UInt32 elapsedWaitTimeInMilliSecs = 0;
                                pulseGenerator->WaitTillNextPulseWindow( m_minimalCycleDeltaInMilliSecs, timeLeftToWait, &elapsedWaitTimeInMilliSecs  );

                                Int64 timeLeftToWaitResult = ((Int64)timeLeftToWait) - elapsedWaitTimeInMilliSecs;
                                if ( timeLeftToWaitResult < m_minimalCycleDeltaInMilliSecs )
                                    timeLeftToWait = 0;
                                else
                                    timeLeftToWait = (UInt32) timeLeftToWaitResult;

                                // While we don't trigger OnTaskCycle for an extended period, we do still send out out driver pulses
                                // This is because pulses are more rudimentary on not per se tied to task cycles explicitly
                                // for example timer implementations may depend on it for additional processing time management
                                if ( m_sendRegularPulses )
                                    SendDriverPulse( *pulseGenerator.GetPointerAlways() );

                                // Any immediate pulse requests cause us to bust out of the extended cycle delay as it counts as an override
                                if ( m_immediatePulseTickets > 0 )
                                    break;
                            }
                            m_taskRequestedCycleDelayInMs = 0;
                        }
                        else
                        {
                            pulseGenerator->WaitTillNextPulseWindow( m_minimalCycleDeltaInMilliSecs );
                        }
                    }
                }
                catch ( const timeout_exception& )
                {                
                    GUCEF_EXCEPTION_LOG( LOGLEVEL_NORMAL, "TaskDelegator: caught timeout_exception while attempting to carry out a cycle for task of type " + task->GetTaskType() + " with id " + ToString( task->GetTaskId() ) );

                    // if we timed out whatever async thing we were trying to do, 
                    // just try again next round
                }
            }

            taskConsumer->OnTaskEnding( task, false );
            task->SetTaskStatus( taskWasFinished ? TASKSTATUS_FINISHED : TASKSTATUS_STOPPED );
            taskConsumer->OnTaskEnded( task, false );            

            if ( !threadPool.IsNULL() )
            {
                // Notify the thread pool directly
                threadPool->OnTaskFinished( task );
            }
            returnStatus = true;
        }
        else
        {
            task->SetTaskStatus( TASKSTATUS_STARTUP_FAILED );
            taskConsumer->OnTaskStartupFailed( task );

            if ( !threadPool.IsNULL() )
            {
                // Notify the thread pool directly
                threadPool->OnTaskStartupFailed( task );
            }
        }
        m_consumerBusy = false;    
    }
    else
    {
        task->SetTaskStatus( TASKSTATUS_STOPPED );
        
        // We never even got to start the consumer's work
        // not an error, just an efficiency thing so we still return success
        if ( !threadPool.IsNULL() )
        {
            // Notify the thread pool directly
            threadPool->OnTaskStopped( task );
        }
        returnStatus = true;
    }
    return returnStatus;
}

/*-------------------------------------------------------------------------*/

void
CTaskDelegator::RequestTaskCycleDelayInMs( UInt32 requestedDelayInMs )
{GUCEF_TRACE;

    m_taskRequestedCycleDelayInMs = requestedDelayInMs;
}

/*-------------------------------------------------------------------------*/

void
CTaskDelegator::OnThreadPausedForcibly( void* taskdata )
{GUCEF_TRACE;

    ThreadPausedEventData id( GetThreadID() );
    if ( !NotifyObservers( ThreadPausedEvent, &id ) ) return;

    CTaskPtr task = m_task;
    if ( !task.IsNULL() )
    {
        CTaskConsumerPtr taskConsumer = task->GetTaskConsumer();
        if ( !taskConsumer.IsNULL() )
        {
            task->SetTaskStatus( TASKSTATUS_PAUSED );
            taskConsumer->OnTaskPaused( task, true );

            ThreadPoolPtr threadPool = m_threadPool;
            if ( !threadPool.IsNULL() )
            {
                // Notify the thread pool directly
                threadPool->OnTaskPaused( task );
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CTaskDelegator::OnThreadResumed( void* taskdata )
{GUCEF_TRACE;

    ThreadResumedEventData id( GetThreadID() );
    if ( !NotifyObservers( ThreadResumedEvent, &id ) ) return;

    CTaskPtr task = m_task;
    if ( !task.IsNULL() )
    {
        CTaskConsumerPtr taskConsumer = task->GetTaskConsumer();
        if ( !taskConsumer.IsNULL() )
        {        
            ThreadPoolPtr threadPool = m_threadPool;
            if ( !threadPool.IsNULL() )
            {
                // Notify the thread pool directly
                threadPool->OnTaskResumed( task );
            }

            task->SetTaskStatus( TASKSTATUS_RESUMED );
            taskConsumer->OnTaskResumed( task );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CTaskDelegator::OnThreadEnding( void* taskdata    ,
                                bool willBeForced )
{GUCEF_TRACE;

    // This is invoked from a different thread than the thread represented by the TaskDelegator
    CTaskPtr task = m_task;
    if ( !task.IsNULL() )
    {
        CTaskConsumerPtr taskConsumer = task->GetTaskConsumer();
        if ( !taskConsumer.IsNULL() )
        {
            taskConsumer->OnTaskEnding( task, willBeForced );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CTaskDelegator::OnThreadEnded( void* taskdata ,
                               bool forced    )
{GUCEF_TRACE;

    // if we get here and the m_consumerBusy flag is set then the task was killed
    // and we did not finish whatever the consumer was doing gracefully
    // We should give the consumer a chance to cleanup
    CTaskPtr task = m_task;
    if ( !task.IsNULL() )
    {
        CTaskConsumerPtr taskConsumer = task->GetTaskConsumer();
        if ( !taskConsumer.IsNULL() )
        {
            TTaskStatus taskStatusBeforeEnd = taskConsumer->GetTaskStatus();
            if ( !( TASKSTATUS_STOPPED == taskStatusBeforeEnd || 
                    TASKSTATUS_FINISHED == taskStatusBeforeEnd ) )
            {
                taskConsumer->SetTaskStatus( TASKSTATUS_STOPPED );
            }
        
            taskConsumer->OnTaskEnded( task, m_consumerBusy );

            if ( !m_threadPool.IsNULL() )
                m_threadPool->RemoveConsumer( task->GetTaskConsumer().GetPointerAlways() );
            taskConsumer->SetTaskDelegator( CTaskDelegatorPtr() );
        }
    }

    if ( forced )
    {
        ThreadFinishedEventData id( GetThreadID() );
        NotifyObservers( ThreadKilledEvent, &id );
    }
    else
    {
        ThreadFinishedEventData id( GetThreadID() );
        NotifyObservers( ThreadFinishedEvent, &id );
    }
}

/*-------------------------------------------------------------------------*/

CTaskConsumerPtr
CTaskDelegator::GetTaskConsumer( void ) const
{GUCEF_TRACE;

    CTaskPtr task = m_task;
    if ( !task.IsNULL() )
        return task->GetTaskConsumer();
    return CTaskConsumerPtr();
}

/*-------------------------------------------------------------------------*/

bool 
CTaskDelegator::HasTaskData( UInt32 taskId ) const
{GUCEF_TRACE;

    CTaskPtr task = m_task;
    if ( !task.IsNULL() )
    {    
        if ( taskId == task->GetTaskId() )
            return GUCEF_NULL != task->GetTaskData();
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CTaskPtr
CTaskDelegator::GetCurrentTask( void ) const
{GUCEF_TRACE;

    return m_task;
}

/*-------------------------------------------------------------------------*/

CTask::TIntegerTypeUsedForTaskId
CTaskDelegator::GetCurrentTaskId( void ) const
{GUCEF_TRACE;

    CTaskPtr task = m_task;
    if ( !task.IsNULL() )
    {    
        return task->GetTaskId();
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

bool
CTaskDelegator::GetThreadInfo( CThreadInfo& info ) const
{GUCEF_TRACE;

    info.SetThreadId( GetThreadID() );
    info.SetThreadStatus( GetThreadStatus() );
        
    UInt64 affinityMask = 0;
    UInt32 affinityMaskSize = sizeof(affinityMask);
    if ( GetCpuAffinityMask( affinityMaskSize, &affinityMask, affinityMaskSize ) )
    {
        info.SetThreadAffinityMask( affinityMask );
    }
    else
    {
        info.SetThreadAffinityMask( 0 );
    }

    UInt32 threadCpuId = 0;
    if ( GetCpuAffinityByCpuId( threadCpuId ) )
    {
        info.SetThreadAffinityByCpuId( threadCpuId );
    }
    else
    {
        info.SetThreadAffinityByCpuId( 0 );
    }

    return true;
}

/*-------------------------------------------------------------------------*/

const MT::CILockable*
CTaskDelegator::AsLockable( void ) const
{GUCEF_TRACE;

    return static_cast< const MT::CActiveObject* >( this );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CTaskDelegator::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return MT::CActiveObject::Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CTaskDelegator::Unlock( void ) const
{GUCEF_TRACE;

    return MT::CActiveObject::Unlock();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
