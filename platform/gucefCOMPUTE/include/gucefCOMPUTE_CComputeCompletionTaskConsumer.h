/*
 *  gucefCOMPUTE: GUCEF module providing compute: devices, buffers and kernels over pluggable backends
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

#ifndef GUCEF_COMPUTE_CCOMPUTECOMPLETIONTASKCONSUMER_H
#define GUCEF_COMPUTE_CCOMPUTECOMPLETIONTASKCONSUMER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CSEMAPHORE_H
#include "gucefMT_CSemaphore.h"
#define GUCEF_MT_CSEMAPHORE_H
#endif /* GUCEF_MT_CSEMAPHORE_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEPENDINGWORK_H
#include "gucefCOMPUTE_CComputePendingWork.h"
#define GUCEF_COMPUTE_CCOMPUTEPENDINGWORK_H
#endif /* GUCEF_COMPUTE_CCOMPUTEPENDINGWORK_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CComputeQueue;
class CComputeCompletionTaskConsumer;

typedef CORE::CTSharedPtr< CComputeCompletionTaskConsumer, MT::CMutex > TComputeCompletionTaskConsumerPtr;

/*-------------------------------------------------------------------------*/

/**
 *  The completion task of a queue, shared by all backends: a dedicated task on the thread pool "ComputeCompletion"
 *  that takes the oldest pending submission, lets the queue wait for it (CComputeQueue::ProcessPendingWork()) in
 *  slices with a timeout, then completes or fails its future and releases its references (deferred release).
 *  On a lost device every pending submission fails with COMPUTERESULT_DEVICE_LOST. When its thread pool stops the task
 *  (application shutdown) the queue closes and every pending submission fails with COMPUTERESULT_BACKEND_ERROR.
 *  It never runs user callbacks and never touches command lists that are being recorded.
 *  The queue starts it on the first asynchronous submission and stops it when the queue shuts down.
 */
class GUCEF_COMPUTE_PUBLIC_CPP CComputeCompletionTaskConsumer : public CORE::CTaskConsumer
{
    public:

    static const CString TaskType;          /**< "GUCEF::COMPUTE::CComputeCompletionTaskConsumer" */
    static const CString ThreadPoolName;    /**< "ComputeCompletion" */

    /** How long the queue waits for one submission before the task checks for a stop request */
    static const UInt32 WaitSliceInMs;

    explicit CComputeCompletionTaskConsumer( CComputeQueue* queue );

    virtual ~CComputeCompletionTaskConsumer() GUCEF_VIRTUAL_OVERRIDE;

    /** Starts the dedicated task; 'self' is the shared pointer that owns this consumer. False when no thread could be had */
    bool Start( const TComputeCompletionTaskConsumerPtr& self );

    /**
     *  Asks the task to end and waits until it did. Submissions still pending are failed with 'result'.
     *  Safe to call when the task never started. Called on the task itself, which happens when finishing a submission
     *  releases the last reference to the device, it does not wait: the task ends when that submission is finished.
     */
    void Stop( TComputeResult result );

    /** Takes ownership of the work and wakes the task */
    void Enqueue( CComputePendingWork* work );

    /** Wakes the task, for example when the device got lost */
    void Wake( void );

    UInt32 GetPendingCount( void ) const;

    bool IsRunning( void ) const;

    /** True on the thread of the task while it runs its loop */
    bool IsCalledFromTask( void ) const;

    virtual CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    void RunLoop( void );
    CComputePendingWork* PeekOldest( void );
    void FinishOldest( TComputeResult result, const CString& errorText );
    void FailAll( TComputeResult result, const CString& errorText );

    CComputeCompletionTaskConsumer( const CComputeCompletionTaskConsumer& src );            /**< not implemented */
    CComputeCompletionTaskConsumer& operator=( const CComputeCompletionTaskConsumer& src ); /**< not implemented */

    private:

    CComputeQueue* m_queue;
    MT::CMutex m_pendingLock;
    GUCEF::vector< CComputePendingWork* > m_pending;    /**< oldest at m_pendingHead */
    UInt32 m_pendingHead;
    MT::CSemaphore m_wake;
    MT::CTAtomic< UInt32 > m_stopRequested;
    MT::CTAtomic< UInt32 > m_loopRunning;
    MT::CTAtomic< TThreadId > m_loopThread;
    bool m_started;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCOMPUTECOMPLETIONTASKCONSUMER_H ? */
