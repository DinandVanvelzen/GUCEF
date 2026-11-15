/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_DRN_CDRNNODETASK_H
#define GUCEF_DRN_CDRNNODETASK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_MT_CSEMAPHORE_H
#include "gucefMT_CSemaphore.h"
#define GUCEF_MT_CSEMAPHORE_H
#endif /* GUCEF_MT_CSEMAPHORE_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CIPULSEGENERATORDRIVER_H
#include "gucefCORE_CIPulseGeneratorDriver.h"
#define GUCEF_CORE_CIPULSEGENERATORDRIVER_H
#endif /* GUCEF_CORE_CIPULSEGENERATORDRIVER_H ? */

#ifndef GUCEF_COMCORE_CSOCKETREADINESSWAITER_H
#include "gucefCOMCORE_CSocketReadinessWaiter.h"
#define GUCEF_COMCORE_CSOCKETREADINESSWAITER_H
#endif /* GUCEF_COMCORE_CSOCKETREADINESSWAITER_H ? */

#ifndef GUCEF_DRN_MACROS_H
#include "gucefDRN_macros.h"
#define GUCEF_DRN_MACROS_H
#endif /* GUCEF_DRN_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CDRNNode;

/**
 *  The dedicated task of a CDRNNode that runs on a thread of its own (CDRNNodeSettings::ownThread).
 *
 *  The task is set up through the task manager on a named thread pool, so the node's thread is known to the task
 *  manager like every other thread. The node keeps working on its own pulse generator (CDRNNode::GetWorkPulseGenerator()):
 *  the task drives it while it runs, every pulse under the node lock (CDRNNode::RunThreadCycle()), and hands it back to
 *  the driver of the node when it ends. The pulse generator of the task's delegator, which the delegator also pulses
 *  itself (before the task starts and after its cycles ended), never reaches the node's objects. Requests for an immediate pulse (busy sockets) make the next cycle start at
 *  once, as does CDRNNode::WakeOwnThread() during the wait between cycles. Otherwise the task keeps the periodic pulses
 *  its pulse generator asks for, as a CORE::CTaskDelegator does for the pulse generator of a task: it waits on the
 *  node's semaphore until the next pulse window of the requested interval (CPulseGenerator::GetTimeTillNextPulseWindowInMilliSecs()),
 *  at most IdleWaitMaxMs, and pulses no more often than that. A node without links asks for its idle interval
 *  (CDRNNodeSettings::idlePulseIntervalMs), so an idle node's thread sleeps; traffic wakes it through the socket
 *  readiness wake and WakeOwnThread() at once.
 *
 *  Used by CDRNNode only.
 */
class GUCEF_DRN_EXPORT_CPP CDRNNodeTask : public CORE::CTaskConsumer         ,
                                          public CORE::CIPulseGeneratorDriver
{
    public:

    static const CORE::CString TaskType;

    /** The longest wait between two cycles, also without periodic pulses: a stop asked by the task manager is seen within it */
    static const UInt32 IdleWaitMaxMs;

    explicit CDRNNodeTask( CDRNNode& node );

    virtual ~CDRNNodeTask() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Waits until the task started working for the node (or failed to start) for at most the given time.
     *  Returns true when the node now works on the task's thread.
     */
    bool WaitUntilStarted( UInt32 timeoutInMs );

    /**
     *  Waits until the task's thread is done with the node (it ended, or its start failed) for at most the given time.
     *  Returns true when the thread no longer touches the node.
     */
    bool WaitUntilEnded( UInt32 timeoutInMs );

    /** Whether the task works for the node: started and not ended */
    bool IsWorking( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual bool OnTaskStart( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskStartupFailed( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnding( CORE::CTaskPtr task, bool willBeForced ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE;

    // the pulse generator driver of the delegator's pulse generator while the task works

    virtual void RequestImmediatePulse( CORE::CPulseGenerator& pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void RequestPulsesPerImmediatePulseRequest( CORE::CPulseGenerator& pulseGenerator               ,
                                                        const Int32 requestedPulsesPerImmediatePulseRequest ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void RequestPeriodicPulses( CORE::CPulseGenerator& pulseGenerator ,
                                        const UInt32 pulseDeltaInMilliSecs    ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void RequestPulseInterval( CORE::CPulseGenerator& pulseGenerator ,
                                       const UInt32 pulseDeltaInMilliSecs    ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void RequestStopOfPeriodicUpdates( CORE::CPulseGenerator& pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;

    /** The task's thread as MT::GetCurrentThreadID() gives it there: what users of the pulse generator compare with */
    virtual TThreadId GetPulseDriverThreadId( void ) const GUCEF_VIRTUAL_OVERRIDE;
    /** The operating system's id of the task's thread, for display; 0 before it started */
    virtual TSystemThreadId GetPulseDriverSystemThreadId( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CDRNNodeTask( void );
    CDRNNodeTask( const CDRNNodeTask& src );
    CDRNNodeTask& operator=( const CDRNNodeTask& src );

    void ReleasePulseGenerator( void );
    void SetStartState( UInt32 state );
    bool WaitForStartState( UInt32 state, UInt32 timeoutInMs );

    enum TStartState
    {
        STARTSTATE_PENDING = 0,
        STARTSTATE_WORKING = 1,
        STARTSTATE_ENDED   = 2
    };

    CDRNNode* m_node;
    CORE::PulseGeneratorPtr m_pulseGenerator;
    CORE::CIPulseGeneratorDriver* m_previousDriver;
    MT::CTAtomic< UInt32 > m_startState;
    MT::CSemaphore m_startStateChanged;             /**< posted on every change of m_startState */
    MT::CTAtomic< UInt32 > m_immediatePulseRequested;
    MT::CTAtomic< UInt32 > m_sendRegularPulses;         /**< 1 while the pulse generator asks for periodic pulses */
    MT::CTAtomic< UInt32 > m_requestedPulseDeltaInMs;   /**< the interval it asks for, the wait between cycles follows it */
    MT::CTAtomic< TThreadId > m_driverTaskId;   /**< MT::GetCurrentThreadID() of the task's thread, 0 before it started */
    GUCEF_SYSTEM_THREAD_ID_CODE( MT::CTAtomic< TSystemThreadId > m_driverTaskSystemThreadId; )   /**< display only, recorded with m_driverTaskId */
    COMCORE::CSocketReadinessWaiterPtr m_readinessWaiter;   /**< wakes the node for the sockets on its pulse generator while the task drives it */
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CDRNNodeTask, MT::CMutex > CDRNNodeTaskPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNNODETASK_H ? */
