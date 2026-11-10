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
    , m_cancelCurrentTask( false )
    , m_sendRegularPulses( false )
    , m_immediatePulseTickets( 0 )
    , m_immediatePulseTicketMax( 1 )
    , m_taskRequestedCycleDelayInMs( 0 )
    , m_requestedPulseDeltaInMs( 0 )
    , m_driverTaskId( 0 )
    , m_driverThreadEnded( false )
    , m_wakeUpSignal( CTaskManager::IsTaskDelegatorWakeUpAvailable() ? GUCEF_NEW MT::CSemaphore( 0 ) : GUCEF_NULL )
    , m_wakeUpPosted( false )
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
    , m_cancelCurrentTask( false )
    , m_sendRegularPulses( false )
    , m_immediatePulseTickets( 0 )
    , m_immediatePulseTicketMax( 1 )
    , m_taskRequestedCycleDelayInMs( 0 )
    , m_requestedPulseDeltaInMs( 0 )
    , m_driverTaskId( 0 )
    , m_driverThreadEnded( false )
    , m_wakeUpSignal( CTaskManager::IsTaskDelegatorWakeUpAvailable() ? GUCEF_NEW MT::CSemaphore( 0 ) : GUCEF_NULL )
    , m_wakeUpPosted( false )
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

    // no request can reach WakeUp() any more: the pulse generator let go of this driver
    GUCEF_DELETE m_wakeUpSignal;
    m_wakeUpSignal = GUCEF_NULL;
    
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
        m_immediatePulseTickets.Store( m_immediatePulseTicketMax.Load() );
        WakeUp();
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
            m_immediatePulseTicketMax.Store( requestedPulsesPerImmediatePulseRequest );
        else
            m_immediatePulseTicketMax.Store( 1 );
        m_sendRegularPulses.Store( true );
    }
}

/*-------------------------------------------------------------------------*/

void
CTaskDelegator::RequestPeriodicPulses( CPulseGenerator& pulseGenerator    ,
                                       const UInt32 pulseDeltaInMilliSecs )

{GUCEF_TRACE;

    if ( &pulseGenerator == m_pulseGenerator.GetPointerAlways() )
    {
        // the cycle delay is the delegator thread's own state, it takes the request at its next cycle
        m_requestedPulseDeltaInMs.Store( pulseDeltaInMilliSecs );
        m_sendRegularPulses.Store( true );
        WakeUp();
    }
}

/*-------------------------------------------------------------------------*/

void
CTaskDelegator::RequestPulseInterval( CPulseGenerator& pulseGenerator    ,
                                      const UInt32 pulseDeltaInMilliSecs )
{GUCEF_TRACE;

    if ( &pulseGenerator == m_pulseGenerator.GetPointerAlways() )
    {
        // the cycle delay is the delegator thread's own state, it takes the request at its next cycle
        m_requestedPulseDeltaInMs.Store( pulseDeltaInMilliSecs );
        m_sendRegularPulses.Store( true );
        WakeUp();
    }
}

/*-------------------------------------------------------------------------*/

void
CTaskDelegator::RequestStopOfPeriodicUpdates( CPulseGenerator& pulseGenerator )
{GUCEF_TRACE;

    if ( &pulseGenerator == m_pulseGenerator.GetPointerAlways() )
    {
        m_sendRegularPulses.Store( false );
    }
}

/*-------------------------------------------------------------------------*/

bool
CTaskDelegator::OnThreadStart( void* taskData )
{GUCEF_TRACE;

    // the id as MT::GetCurrentTaskID() gives it, on this thread: that is what the pulse generator's users compare with
    // (on Linux GetThreadID() is the kernel thread id, a different number)
    m_driverThreadEnded.Store( false );
    m_driverTaskId.Store( MT::GetCurrentTaskID() );
    m_pulseGenerator->SetPulseDriverThreadId( m_driverTaskId.Load() );
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
CTaskDelegator::CancelCurrentTask( UInt32 taskId )
{GUCEF_TRACE;

    if ( GetCurrentTaskId() == taskId )
    {
        m_cancelCurrentTask = true;
        WakeUp();   // the task ends now, not at the next cycle
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CTaskDelegator::IsStopOfCurrentTaskRequested( void ) const
{GUCEF_TRACE;

    return m_cancelCurrentTask || IsDeactivationRequested();
}

/*-------------------------------------------------------------------------*/

bool
CTaskDelegator::OnThreadCycle( void* )
{GUCEF_TRACE;

    ApplyRequestedPulseDelta();

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

    ApplyRequestedPulseDelta();

    // Create local refs for the invocation duration to avoid external interference
    PulseGeneratorPtr pulseGenerator = m_pulseGenerator;
    ThreadPoolPtr threadPool = m_threadPool;
    if GUCEF_PREDICT_FALSE( pulseGenerator.IsNULL() || threadPool.IsNULL() || task.IsNULL()  )
        return false;

    // A task which was asked to cancel before we got to it never starts
    m_cancelCurrentTask = task->IsCancellationRequested();

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
        if ( !IsStopOfCurrentTaskRequested() )
        {
            pulseGenerator->WaitTillNextPulseWindow( 25 );
        }
        else
        {
            break;
        }
    }
    
    if ( !IsStopOfCurrentTaskRequested() )
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
            while ( !IsStopOfCurrentTaskRequested() )
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

                    // A ticket is taken before the pulse, not after it: a request made while the pulse is delivered
                    // (by a handler of the pulse or by another thread meanwhile) hands out a ticket of its own, which
                    // is left for the next cycle. Only this thread takes tickets, others only hand them out
                    bool tookTicket = false;
                    Int32 tickets = m_immediatePulseTickets.Load();
                    while ( tickets > 0 && !tookTicket )
                        tookTicket = m_immediatePulseTickets.CompareExchangeStrong( tickets, tickets - 1 );

                    if ( tookTicket || m_sendRegularPulses.Load() )
                        SendDriverPulse( *pulseGenerator.GetPointerAlways() );

                    if ( m_immediatePulseTickets.Load() > 0 )
                    {
                        // tickets are left (more pulses per request, or a request made during the pulse): the next
                        // cycle comes without a wait
                    }
                    else
                    {
                        // Per cycle we allow external deferment of the next cycle which is distinct from the minimum cycle delta
                        // this allows others or the task logic itself to throttle and 'back off' on a per cycle basis as needed
                        UInt32 taskRequestedCycleDelayInMs = m_taskRequestedCycleDelayInMs.Load();
                        if ( taskRequestedCycleDelayInMs > 0 && taskRequestedCycleDelayInMs > m_minimalCycleDeltaInMilliSecs )
                        {
                            UInt32 timeLeftToWait = taskRequestedCycleDelayInMs;
                            while ( !IsStopOfCurrentTaskRequested() && timeLeftToWait >= m_minimalCycleDeltaInMilliSecs )
                            {
                                UInt32 elapsedWaitTimeInMilliSecs = 0;
                                WaitTillNextPulseWindowOrWakeUp( *pulseGenerator, m_minimalCycleDeltaInMilliSecs, timeLeftToWait, &elapsedWaitTimeInMilliSecs );

                                Int64 timeLeftToWaitResult = ((Int64)timeLeftToWait) - elapsedWaitTimeInMilliSecs;
                                if ( timeLeftToWaitResult < m_minimalCycleDeltaInMilliSecs )
                                    timeLeftToWait = 0;
                                else
                                    timeLeftToWait = (UInt32) timeLeftToWaitResult;

                                // While we don't trigger OnTaskCycle for an extended period, we do still send out out driver pulses
                                // This is because pulses are more rudimentary on not per se tied to task cycles explicitly
                                // for example timer implementations may depend on it for additional processing time management
                                if ( m_sendRegularPulses.Load() )
                                    SendDriverPulse( *pulseGenerator.GetPointerAlways() );

                                // Any immediate pulse requests cause us to bust out of the extended cycle delay as it counts as an override
                                if ( m_immediatePulseTickets.Load() > 0 )
                                    break;
                            }
                            m_taskRequestedCycleDelayInMs.Store( 0 );
                        }
                        else
                        {
                            // periodic pulses asked for at a shorter interval than the minimal cycle delta are given
                            // at that interval: the requests are made for it (a protocol thread waiting for frames)
                            UInt32 minimalCycleDelta = m_minimalCycleDeltaInMilliSecs;
                            UInt32 requestedPulseDelta = m_requestedPulseDeltaInMs.Load();
                            if ( m_sendRegularPulses.Load() && 0 != requestedPulseDelta && requestedPulseDelta < minimalCycleDelta )
                                minimalCycleDelta = requestedPulseDelta;
                            // a pulse request of another thread ends the wait: a dedicated protocol task answers a
                            // frame from another thread at once instead of at its next cycle
                            WaitTillNextPulseWindowOrWakeUp( *pulseGenerator, minimalCycleDelta, 1000, GUCEF_NULL );
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
            task->SetTaskStatus( taskWasFinished ? TASKSTATUS_FINISHED : ( task->IsCancellationRequested() ? TASKSTATUS_CANCELLED : TASKSTATUS_STOPPED ) );
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
        task->SetTaskStatus( task->IsCancellationRequested() ? TASKSTATUS_CANCELLED : TASKSTATUS_STOPPED );
        
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
CTaskDelegator::ApplyRequestedPulseDelta( void )
{GUCEF_TRACE;

    UInt32 requestedPulseDeltaInMs = m_requestedPulseDeltaInMs.Load();
    if ( 0 != requestedPulseDeltaInMs )
        m_delayInMilliSecs = requestedPulseDeltaInMs;
}

/*-------------------------------------------------------------------------*/

void
CTaskDelegator::WakeUp( void )
{GUCEF_TRACE;

    // without a working semaphore the delegator waits with a plain delay and takes the request at its next cycle
    if ( GUCEF_NULL == m_wakeUpSignal )
        return;
    if ( MT::GetCurrentTaskID() == m_driverTaskId.Load() )
        return;

    // one post at a time: requests that come while a post waits to be taken need none of their own, the delegator
    // thread looks at all requests after it took it, so the semaphore never counts more than 1
    bool notPosted = false;
    if ( m_wakeUpPosted.CompareExchangeStrong( notPosted, true ) )
    {
        if ( !m_wakeUpSignal->Post() )
            m_wakeUpPosted.Store( false );
    }
}

/*-------------------------------------------------------------------------*/

void
CTaskDelegator::WaitTillNextPulseWindowOrWakeUp( CPulseGenerator& pulseGenerator       ,
                                                 UInt32 forcedMinimalDeltaInMilliSecs  ,
                                                 UInt32 desiredMaximumDeltaInMilliSecs ,
                                                 UInt32* elapsedTimeInMilliSecs        )
{GUCEF_TRACE;

    UInt32 timeLeftToWaitInMs = pulseGenerator.GetTimeTillNextPulseWindowInMilliSecs( forcedMinimalDeltaInMilliSecs, desiredMaximumDeltaInMilliSecs );
    if ( 0 == timeLeftToWaitInMs )
        return; // Time's up already

    // without a working semaphore (CTaskManager::IsTaskDelegatorWakeUpAvailable()) the wait is the plain delay it was before
    Int32 waitResult = GUCEF_NULL != m_wakeUpSignal ? m_wakeUpSignal->WaitOrTimeout( timeLeftToWaitInMs ) : -1;
    if ( 0 == waitResult )
    {
        // taken: a request made from now on posts again
        m_wakeUpPosted.Store( false );
    }
    else
    if ( 0 > waitResult )
    {
        // without a working semaphore the wait is the plain delay it was before
        MT::PrecisionDelay( timeLeftToWaitInMs );
    }

    if ( GUCEF_NULL != elapsedTimeInMilliSecs )
    {
        *elapsedTimeInMilliSecs = (UInt32) pulseGenerator.GetActualPulseDeltaInMilliSecs();
    }
}

/*-------------------------------------------------------------------------*/

void
CTaskDelegator::RequestTaskCycleDelayInMs( UInt32 requestedDelayInMs )
{GUCEF_TRACE;

    m_taskRequestedCycleDelayInMs.Store( requestedDelayInMs );
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
            if ( !( TASKSTATUS_STOPPED == taskStatusBeforeEnd   ||
                    TASKSTATUS_CANCELLED == taskStatusBeforeEnd ||
                    TASKSTATUS_FINISHED == taskStatusBeforeEnd  ) )
            {
                taskConsumer->SetTaskStatus( task->IsCancellationRequested() ? TASKSTATUS_CANCELLED : TASKSTATUS_STOPPED );
            }
        
            taskConsumer->OnTaskEnded( task, m_consumerBusy );

            if ( !m_threadPool.IsNULL() )
                m_threadPool->RemoveConsumer( task->GetTaskConsumer().GetPointerAlways() );
            taskConsumer->SetTaskDelegator( CTaskDelegatorPtr() );
        }
    }

    // the thread is gone: a thread made later may get the same id, it must not pass for the driver of the pulse generator
    m_driverTaskId.Store( 0 );
    m_driverThreadEnded.Store( true );
    PulseGeneratorPtr pulseGenerator = m_pulseGenerator;
    if ( !pulseGenerator.IsNULL() )
        pulseGenerator->SetPulseDriverThreadId( 0 );

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

/*-------------------------------------------------------------------------*/

UInt32
CTaskDelegator::GetPulseDriverThreadId( void ) const
{GUCEF_TRACE;

    UInt32 taskId = m_driverTaskId.Load();
    if ( 0 != taskId )
        return taskId;
    // before the thread runs its id as the platform gives it; once it ended no thread
    return m_driverThreadEnded.Load() ? 0 : GetThreadID();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
