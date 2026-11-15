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

#ifndef GUCEF_COMPUTE_CCOMPUTEQUEUE_H
#define GUCEF_COMPUTE_CCOMPUTEQUEUE_H

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

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTECOMMANDLIST_H
#include "gucefCOMPUTE_CComputeCommandList.h"
#define GUCEF_COMPUTE_CCOMPUTECOMMANDLIST_H
#endif /* GUCEF_COMPUTE_CCOMPUTECOMMANDLIST_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEFENCE_H
#include "gucefCOMPUTE_CComputeFence.h"
#define GUCEF_COMPUTE_CCOMPUTEFENCE_H
#endif /* GUCEF_COMPUTE_CCOMPUTEFENCE_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEPENDINGWORK_H
#include "gucefCOMPUTE_CComputePendingWork.h"
#define GUCEF_COMPUTE_CCOMPUTEPENDINGWORK_H
#endif /* GUCEF_COMPUTE_CCOMPUTEPENDINGWORK_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTECOMPLETIONTASKCONSUMER_H
#include "gucefCOMPUTE_CComputeCompletionTaskConsumer.h"
#define GUCEF_COMPUTE_CCOMPUTECOMPLETIONTASKCONSUMER_H
#endif /* GUCEF_COMPUTE_CCOMPUTECOMPLETIONTASKCONSUMER_H ? */

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

class CComputeDevice;
class CComputeGlobal;

/**
 *  An in-order queue of a device. Commands of one queue take effect in submission and recording order; ordering
 *  across queues exists only through CComputeCommandList::WaitFor(). Every submission gets the next value of the
 *  timeline of the queue; a fence is that value.
 *
 *  Fences are host-visible completion: once the timeline reaches a value (GetCompletedValue(), WaitForValue(),
 *  CComputeFence::IsSignalled() and Wait()), the work of that submission is done on the device and the data of its
 *  ReadToHost commands is in host memory. Backends whose native timeline signals before their staging copies to host
 *  memory are done keep the device timeline apart and advance the host-visible one with AdvanceHostTimeline(), which
 *  runs OnFinishHostTransfers() once per submission, in order, on whichever thread observes the completion first.
 *
 *  Synchronous and asynchronous execution (the GUCEF rule):
 *   - Execute(): the calling thread validates, translates and submits under the queue lock, then waits, without the
 *     lock, on the fence primitive of the backend itself. It never pumps a pulse generator of another thread.
 *     Backends that execute on the host (the CPU backend) execute the list on the calling thread at that point, once
 *     the submissions before it completed.
 *   - ExecuteAsync(): the calling thread validates, translates and submits, then hands a CFutureResultSource with the
 *     fence to the completion task of the queue (CComputeCompletionTaskConsumer, pool "ComputeCompletion"), which
 *     completes or fails the future when the fence signals. A failed future carries the TComputeResult name in its
 *     extra info (ComputeResultFromFuture()).
 *   - Submit(): as ExecuteAsync() without a future; the caller keeps the fence.
 *  All three use the same translation and submission of the backend. Submission is thread-safe: one lock per queue,
 *  held only while validating, translating and submitting.
 *
 *  Cooperative mode (CComputeDeviceParams::SetCooperativeCompletion(), a browser main thread, or a platform without
 *  threads): there is no completion task. Pending submissions stay with the queue and complete when the application
 *  calls CComputeGlobal::Update(), which processes them without waiting, or when a wait on the calling thread
 *  processes them (backends that execute on the host).
 *
 *  A queue closes for good when its completion task is stopped by its thread pool (application shutdown): pending
 *  submissions fail with COMPUTERESULT_BACKEND_ERROR and new work is refused with the same result. A queue of a lost
 *  device refuses work with COMPUTERESULT_DEVICE_LOST. A value of the timeline is never skipped: work that was not
 *  done fails, it is never reported as completed.
 *
 *  Backends derive from the queue and implement OnSubmit(), WaitForValue() and GetCompletedValue(); backends that
 *  execute on the host override OnExecute() and ProcessPendingWork() as well. Queues belong to their device; keep the
 *  device referenced while using its queues. A queue that outlives its device reports COMPUTERESULT_DEVICE_LOST.
 */
class GUCEF_COMPUTE_PUBLIC_CPP CComputeQueue : public CORE::CTSharedObjCreator< CComputeQueue, MT::CMutex >
{
    public:

    virtual ~CComputeQueue() GUCEF_VIRTUAL_OVERRIDE;

    /** The device of the queue; null once the device is gone */
    CComputeDevice* GetDevice( void ) const;

    UInt32 GetQueueIndex( void ) const;

    /** Submits the list and returns at once; 'fence' is signalled when the work completed */
    TComputeResult Submit( const CComputeCommandList& list, CComputeFence& fence );

    /** Runs the list and returns when it completed, with the execution policy of the queue */
    TComputeResult Execute( const CComputeCommandList& list );

    /** As Execute() with an execution policy for this call only */
    TComputeResult Execute( const CComputeCommandList& list, TComputeExecutionPolicy policy );

    /** Submits the list and returns a future that completes (or fails) when the work completed */
    CORE::CFutureResult ExecuteAsync( const CComputeCommandList& list );

    /** The fence of the last submission; a fence of value 0 when nothing was submitted */
    CComputeFence GetLastSubmittedFence( void );

    /** Waits on the calling thread until everything submitted so far completed */
    TComputeResult WaitIdle( UInt32 timeoutInMs = GUCEF_COMPUTE_INFINITE_TIMEOUT );

    /** The value of the last submission that completed host-visibly */
    virtual UInt64 GetCompletedValue( void ) const = 0;

    /** Waits on the calling thread until the timeline reaches 'value' host-visibly. COMPUTERESULT_SUCCESS, _TIMEOUT, _DEVICE_LOST or the result the queue closed with */
    virtual TComputeResult WaitForValue( UInt64 value, UInt32 timeoutInMs ) = 0;

    /** For synchronous calls without a policy of their own; only the CPU backend has a choice */
    void SetExecutionPolicy( TComputeExecutionPolicy policy );
    TComputeExecutionPolicy GetExecutionPolicy( void ) const;

    /** True when the device of the queue is lost or gone */
    bool IsLost( void ) const;

    /** True once the queue closed for good (see the class description) */
    bool IsClosed( void ) const;

    /** True when pending submissions complete through CComputeGlobal::Update() instead of a completion task */
    bool IsCooperative( void ) const;

    /** Submissions that are pending (with the completion task or, in cooperative mode, with the queue) */
    UInt32 GetPendingWorkCount( void ) const;

    protected:

    CComputeQueue( CComputeDevice* device, UInt32 queueIndex, TComputeExecutionPolicy policy, bool cooperative );

    /** Translates the list and submits it to the device with the given timeline value. Called with the queue lock held */
    virtual TComputeResult OnSubmit( const CComputeCommandList& list, UInt64 fenceValue ) = 0;

    /**
     *  Completes a synchronous call on the calling thread, after OnSubmit() succeeded and without the queue lock.
     *  The default waits with WaitForValue().
     */
    virtual TComputeResult OnExecute( const CComputeCommandList& list, UInt64 fenceValue, TComputeExecutionPolicy policy );

    /**
     *  Waits for a pending submission at most 'timeoutInMs', for the completion task or a cooperative drain, in order.
     *  COMPUTERESULT_TIMEOUT lets the caller come back later. The default waits with WaitForValue().
     */
    virtual TComputeResult ProcessPendingWork( CComputePendingWork& work, UInt32 timeoutInMs, CString& errorText );

    /** Called once when the device got lost: wake every waiter of the backend */
    virtual void OnDeviceLost( void );

    /** Called once when the queue closed: waiters of values that were not reached get 'reason' */
    virtual void OnQueueClosed( TComputeResult reason );

    /**
     *  The host side of a completed submission: copies from staging memory into the host memory of its ReadToHost
     *  commands. Called by AdvanceHostTimeline() once per submission, in order, before the host-visible value reaches
     *  it. The default does nothing (backends whose device writes host memory directly).
     */
    virtual TComputeResult OnFinishHostTransfers( UInt64 fenceValue, CString& errorText );

    /**
     *  Advances the host-visible timeline up to 'deviceCompletedValue' (what the native timeline reached), running
     *  OnFinishHostTransfers() for each value on the way. Serialised: the first thread that observes a completion does
     *  the transfers, the others wait for it. Returns the first failure of a transfer, the timeline advances anyway.
     */
    TComputeResult AdvanceHostTimeline( UInt64 deviceCompletedValue );

    /** The host-visible value that AdvanceHostTimeline() reached */
    UInt64 GetHostTimelineValue( void ) const;

    /**
     *  Cooperative mode: processes the pending submissions up to 'upToValue', in order, each with 'timeoutInMs'. Stops
     *  at the first one that is not done in time. Finished submissions go to 'finished': the caller deletes them when
     *  it no longer uses the queue, as deleting them may release the last reference to the device.
     */
    void DrainCooperativeWork( UInt64 upToValue, UInt32 timeoutInMs, TComputePendingWorkVector& finished );

    /** Fails the pending submissions, stops the completion task and detaches from the device. The device calls it */
    void ShutdownQueue( void );

    private:

    friend class CComputeDevice;
    friend class CComputeCompletionTaskConsumer;

    TComputeResult Validate( const CComputeCommandList& list ) const;
    TComputeResult ReserveBufferUses( const CComputeCommandList& list, UInt64 fenceValue, GUCEF::vector< UInt64 >& previous );
    void UndoBufferUses( const CComputeCommandList& list, const GUCEF::vector< UInt64 >& previous );
    TComputeResult SubmitLocked( const CComputeCommandList& list, UInt64& fenceValue );
    void HandOver( CComputePendingWork* work );
    bool EnsureCompletionTask( void );
    void NotifyDeviceLost( void );
    void CloseFromCompletionTask( TComputeResult reason );
    void Detach( void );
    static void DeleteFinished( TComputePendingWorkVector& finished );

    CComputeQueue( const CComputeQueue& src );            /**< not implemented */
    CComputeQueue& operator=( const CComputeQueue& src ); /**< not implemented */

    private:

    MT::CTAtomic< CComputeDevice* > m_device;
    UInt32 m_queueIndex;
    MT::CMutex m_submitLock;
    UInt64 m_lastSubmittedValue;                        /**< guarded by m_submitLock */
    MT::CTAtomic< UInt32 > m_policy;
    TComputeCompletionTaskConsumerPtr m_completion;     /**< guarded by m_submitLock */
    MT::CTAtomic< UInt32 > m_cooperative;
    MT::CTAtomic< UInt32 > m_closedResult;              /**< 0 while open, else the TComputeResult + 1 the queue closed with */
    MT::CMutex m_cooperativeLock;                       /**< recursive: guards the list and serialises draining */
    TComputePendingWorkVector m_cooperativeWork;        /**< guarded by m_cooperativeLock, oldest at m_cooperativeHead */
    UInt32 m_cooperativeHead;
    MT::CMutex m_hostTimelineLock;
    MT::CTAtomic< UInt64 > m_hostCompleted;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCOMPUTEQUEUE_H ? */
