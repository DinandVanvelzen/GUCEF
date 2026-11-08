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

    typedef TCloneableUInt32    ThreadKilledEventData;       /**< thread ID of the thread in question */
    typedef TCloneableUInt32    ThreadStartedEventData;      /**< thread ID of the thread in question */
    typedef TCloneableUInt32    ThreadPausedEventData;       /**< thread ID of the thread in question */
    typedef TCloneableUInt32    ThreadResumedEventData;      /**< thread ID of the thread in question */
    typedef TCloneableUInt32    ThreadFinishedEventData;     /**< thread ID of the thread in question */


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

    virtual UInt32 GetPulseDriverThreadId( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Takes the cycle delay asked for through the pulse requests, on the delegator thread that uses it */
    void ApplyRequestedPulseDelta( void );

    /**
     *  Ends a wait of the delegator thread between its cycles, for a pulse request made on another thread.
     *  The delegator thread itself looks at the requests before it waits, its own requests need no wake-up.
     */
    void WakeUp( void );

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
    MT::CTAtomic< UInt32 > m_requestedPulseDeltaInMs;       /**< the cycle delay asked for, applied on the delegator thread, 0 = none */
    MT::CTAtomic< UInt32 > m_driverTaskId;  /**< MT::GetCurrentTaskID() of the delegator thread, 0 before it runs and once it ended */
    MT::CTAtomic< bool > m_driverThreadEnded;   /**< set when the delegator thread ended, cleared when it starts */
    MT::CSemaphore* m_wakeUpSignal;         /**< posted to end the delegator thread's wait between cycles, counts at most 1; NULL without a working semaphore (CTaskManager::IsTaskDelegatorWakeUpAvailable()) */
    MT::CTAtomic< bool > m_wakeUpPosted;    /**< whether m_wakeUpSignal holds a post the delegator thread did not take yet */
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
