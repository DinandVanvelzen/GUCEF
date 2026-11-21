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

#ifndef GUCEF_CORE_CTASKDELEGATOR_H
#define GUCEF_CORE_CTASKDELEGATOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_MT_CSEMAPHORE_H
#include "gucefMT_CSemaphore.h"
#define GUCEF_MT_CSEMAPHORE_H
#endif /* GUCEF_MT_CSEMAPHORE_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CNOTIFIER_H
#include "CNotifier.h"
#define GUCEF_CORE_CNOTIFIER_H
#endif /* GUCEF_CORE_CNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTNUMERICID_H
#include "CTNumericID.h"
#define GUCEF_CORE_CTNUMERICID_H
#endif /* GUCEF_CORE_CTNUMERICID_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_CIPULSEGENERATORDRIVER_H
#include "gucefCORE_CIPulseGeneratorDriver.h"
#define GUCEF_CORE_CIPULSEGENERATORDRIVER_H
#endif /* GUCEF_CORE_CIPULSEGENERATORDRIVER_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CIASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CTHREADINFO_H
#include "gucefCORE_CThreadlnfo.h"
#define GUCEF_CORE_CTHREADINFO_H
#endif /* GUCEF_CORE_CTHREADINFO_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

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

class CThreadPool;

/*-------------------------------------------------------------------------*/

/**
 *  This is an internally used class and is tightly coupled with the
 *  CTaskManager class. It should not be used directly.
 *
 *  This class implements an active object that delegates the actual logic to
 *  be threaded to a task consumer implementor. Thus separating thread lifecycle
 *  management from task lifecycle management. A delegator can execute as many
 *  tasks by invoking as many consumers as desired without having to (re)create
 *  a thread.
 */
class GUCEF_CORE_PRIVATE_CPP CTaskDelegator : public MT::CActiveObject      ,
                                              public CNotifier              ,
                                              public CTSharedPtrCreator< CTaskDelegator, MT::CMutex > ,
                                              private CIPulseGeneratorDriver
{
    public:

    static const CEvent ThreadKilledEvent;
    static const CEvent ThreadStartedEvent;
    static const CEvent ThreadPausedEvent;
    static const CEvent ThreadResumedEvent;
    static const CEvent ThreadFinishedEvent;

    typedef CTCloneableObj< TSystemThreadId >    ThreadKilledEventData;       /**< operating system id of the thread in question (MT::ThreadID()) */
    typedef CTCloneableObj< TSystemThreadId >    ThreadStartedEventData;      /**< operating system id of the thread in question (MT::ThreadID()) */
    typedef CTCloneableObj< TSystemThreadId >    ThreadPausedEventData;       /**< operating system id of the thread in question (MT::ThreadID()) */
    typedef CTCloneableObj< TSystemThreadId >    ThreadResumedEventData;      /**< operating system id of the thread in question (MT::ThreadID()) */
    typedef CTCloneableObj< TSystemThreadId >    ThreadFinishedEventData;     /**< operating system id of the thread in question (MT::ThreadID()) */


    static void RegisterEvents( void );

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual ~CTaskDelegator() GUCEF_VIRTUAL_OVERRIDE;

    public:

    typedef CTBasicSharedPtr< CThreadPool, MT::CMutex >     ThreadPoolPtr;
    
    CTaskConsumerPtr GetTaskConsumer( void ) const;

    PulseGeneratorPtr GetPulseGenerator( void );

    ThreadPoolPtr GetThreadPool( void );

    CTaskPtr GetCurrentTask( void ) const;

    /**
     *  Asks the delegator to stop working on the task with the given id, if that is the task it is carrying out now.
     *  Only the task ends: the consumer is given the chance to wrap up (OnTaskEnding / OnTaskEnded) and the task ends as
     *  cancelled, after which the thread of the delegator is free to pick up other work.
     *  @return true if the task is being carried out by this delegator
     */
    bool CancelCurrentTask( UInt32 taskId );

    UInt32 GetCurrentTaskId( void ) const;

    /**
     *  Whether the consumer was provided with any data or was expected to run without a specific handoff of 
     *  'work' data to the consumer.
     */
    bool HasTaskData( UInt32 taskId ) const;

    bool GetThreadInfo( CThreadInfo& info ) const;
    
    protected:
    friend class CThreadPool;
    friend class CTaskConsumer;

    CTaskDelegator( const ThreadPoolPtr& threadPool );

    /**
     *  Startup routine for the task. You should return true if startup succeeded and the task can commence
     *  cycling.
     */
    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Called after a successful call to OnTaskStart
     */
    virtual void OnThreadStarted( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Perform all your main task work in this function.
     *  It will be called repeatedly until true is returned indicating that the task has been completed.
     *  Thus for ongoing tasks you can write this function to take care of a single iteration of the task.
     */
    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE;

    /** Ends the wait between cycles (also the wait for a task) so that the thread sees the request to stop now */
    virtual void OnDeactivationRequested( void ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Last chance notification to descended classes of impending end of the tread
     *  If the 'willBeForced' flag is true the thread will be killed next
     *  Since this would be used in cases where the thread is misbehaving one should
     *  keep that in mind in the implementation of this callback
     */
    virtual void OnThreadEnding( void* taskdata    ,
                                 bool willBeForced ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnThreadPausedForcibly( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnThreadResumed( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  This is where all the cleanup should be done for task data
     *  Note that this member function will be called from within the spawned thread when ending gracefully
     *  but in the case of a forceful termination of the spawned thread this member function will be called
     *  from the thread that triggered the forceful termination.
     */
    virtual void OnThreadEnded( void* taskdata ,
                                bool forced    ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool ProcessTask( CTaskPtr task );

    /** whether the task being carried out has to stop: the thread is shutting down or the task itself was cancelled */
    bool IsStopOfCurrentTaskRequested( void ) const;

    virtual void TaskCleanup( CTaskPtr task );

    /**
     *  Requests that prior to the next task cycle a minimum delay of 'requestedDelayInMs' is observed
     *  Note that requests to expedite cycles, contradicting this request, take priority
     *  This functionality can be used to implement throttling a task and thus the owning thread
     */
    void RequestTaskCycleDelayInMs( UInt32 requestedDelayInMs );
    
    protected:

    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    PulseGeneratorPtr m_pulseGenerator;

    private:

    CTaskDelegator( const CTaskDelegator& src );
    CTaskDelegator& operator=( const CTaskDelegator& src );

    virtual void RequestImmediatePulse( CPulseGenerator& pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void RequestPulsesPerImmediatePulseRequest( CPulseGenerator& pulseGenerator                     ,
                                                        const Int32 requestedPulsesPerImmediatePulseRequest ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void RequestPeriodicPulses( CPulseGenerator& pulseGenerator    ,
                                        const UInt32 pulseDeltaInMilliSecs ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void RequestPulseInterval( CPulseGenerator& pulseGenerator    ,
                                       const UInt32 pulseDeltaInMilliSecs ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void RequestStopOfPeriodicUpdates( CPulseGenerator& pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TThreadId GetPulseDriverThreadId( void ) const GUCEF_VIRTUAL_OVERRIDE;
    /** The operating system's id of the delegator thread while it drives the pulses (as GetThreadID()), else 0 */
    virtual TSystemThreadId GetPulseDriverSystemThreadId( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Takes the cycle delay asked for through the pulse requests, on the delegator thread that uses it */
    void ApplyRequestedPulseDelta( void );

    /**
     *  The longest wait of a delegator without a task for a task to be queued (1000 ms): the pool wakes a waiting
     *  delegator when it queues one (WakeUpIfWaitingForTask()), so an idle worker thread does not look at the queue
     *  every cycle delay. Without a working semaphore the cycle delay remains the wait.
     */
    static const UInt32 IdleTaskWaitMs;

    /**
     *  Ends the wait of a delegator that has no task and waits for one to be queued; any thread. True when it was
     *  waiting: it is no longer counted as waiting, so the next task queued wakes another one.
     */
    bool WakeUpIfWaitingForTask( void );

    /**
     *  Ends a wait of the delegator thread between its cycles, for a pulse request made on another thread.
     *  The delegator thread itself looks at the requests before it waits, its own requests need no wake-up.
     */
    void WakeUp( void );

    /**
     *  On the delegator thread without a task: waits for one to be queued (see IdleTaskWaitMs); true with m_task set.
     *  Posts that are not for a task do not end the wait, a request to stop or to pause does. False without a working semaphore.
     */
    bool WaitForQueuedTask( void );

    /**
     *  Waits on the delegator thread until the next pulse time window of its pulse generator, as
     *  CPulseGenerator::WaitTillNextPulseWindow() does, or until a pulse request of another thread ends the wait
     *  (WakeUp()), whichever comes first.
     */
    void WaitTillNextPulseWindowOrWakeUp( CPulseGenerator& pulseGenerator          ,
                                          UInt32 forcedMinimalDeltaInMilliSecs     ,
                                          UInt32 desiredMaximumDeltaInMilliSecs    ,
                                          UInt32* elapsedTimeInMilliSecs           );

    protected:

    CTaskDelegator( const ThreadPoolPtr& threadPool ,
                    const CTaskPtr& task            );

    CTaskPtr m_task;

    private:

    ThreadPoolPtr m_threadPool;
    bool m_consumerBusy;
    MT::CTAtomic< bool > m_cancelCurrentTask;   /**< set when the task which is being carried out is asked to stop (from any thread), the thread itself carries on */
    MT::CTAtomic< bool > m_sendRegularPulses;               /**< pulse requests come from any thread, the delegator thread reads them */
    MT::CTAtomic< Int32 > m_immediatePulseTickets;
    MT::CTAtomic< Int32 > m_immediatePulseTicketMax;
    MT::CTAtomic< UInt32 > m_taskRequestedCycleDelayInMs;
    MT::CTAtomic< UInt32 > m_otherThreadPulseRequests;     /**< immediate pulse requests made on threads other than the delegator thread, counted (wraps); the delegator thread compares it before and after a task cycle */
    MT::CTAtomic< UInt32 > m_requestedPulseDeltaInMs;       /**< the cycle delay asked for, applied on the delegator thread, 0 = none */
    MT::CTAtomic< TThreadId > m_driverThreadId;    /**< MT::GetCurrentThreadID() of the delegator thread, 0 before it runs and once it ended */
    GUCEF_SYSTEM_THREAD_ID_CODE( MT::CTAtomic< TSystemThreadId > m_driverSystemThreadId; )   /**< display only, written with m_driverThreadId */
    MT::CSemaphore* m_wakeUpSignal;         /**< posted to end the delegator thread's wait between cycles, counts at most 1; NULL without a working semaphore (CTaskManager::IsTaskDelegatorWakeUpAvailable()) */
    MT::CTAtomic< bool > m_wakeUpPosted;    /**< whether m_wakeUpSignal holds a post the delegator thread did not take yet */
    MT::CTAtomic< bool > m_waitingForTask;  /**< set by the delegator thread while it has no task and waits for one, cleared by the thread pool that wakes it for a queued task */
    MT::CTAtomic< UInt32 > m_threadSequence;    /**< n of the thread name <pool>#<n>, set when the thread starts; 0 = no thread of its own (yet) */
    MT::CMutex m_currentTaskInfoLock;           /**< leaf: what GetThreadInfo() shows of the task being carried out, written by the delegator thread */
    UInt32 m_currentTaskInfoId;
    CString m_currentTaskInfoType;
    UInt64 m_currentTaskInfoBusySinceMs;

    void SetCurrentTaskInfo( const CTaskPtr& task );
    void ClearCurrentTaskInfo( void );
};

/*-------------------------------------------------------------------------*/

typedef CTSharedPtr< CTaskDelegator, MT::CMutex >   CTaskDelegatorPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTASKDELEGATOR_H ? */
