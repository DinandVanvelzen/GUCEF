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

#ifndef GUCEF_COMPUTE_CCPUCOMPUTEQUEUE_H
#define GUCEF_COMPUTE_CCPUCOMPUTEQUEUE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMPUTE_CCOMPUTEQUEUE_H
#include "gucefCOMPUTE_CComputeQueue.h"
#define GUCEF_COMPUTE_CCOMPUTEQUEUE_H
#endif /* GUCEF_COMPUTE_CCOMPUTEQUEUE_H ? */

#ifndef GUCEF_COMPUTE_CCPUPARALLELFOR_H
#include "gucefCOMPUTE_CCpuParallelFor.h"
#define GUCEF_COMPUTE_CCPUPARALLELFOR_H
#endif /* GUCEF_COMPUTE_CCPUPARALLELFOR_H ? */

#ifndef GUCEF_COMPUTE_CCPUCOMPUTEFENCE_H
#include "gucefCOMPUTE_CCpuComputeFence.h"
#define GUCEF_COMPUTE_CCPUCOMPUTEFENCE_H
#endif /* GUCEF_COMPUTE_CCPUCOMPUTEFENCE_H ? */

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

/**
 *  A queue of the CPU device. Synchronous work (Execute()) runs on the calling thread after the work submitted
 *  before it completed; the dispatches of a list are fork-join through CCpuParallelFor with the calling thread taking
 *  part, or on the calling thread only with EXECUTIONPOLICY_CALLING_THREAD_ONLY. Asynchronous work (Submit(),
 *  ExecuteAsync()) runs on the completion task of the queue, which is the queue worker of the CPU backend: it runs
 *  each list through the same function and then completes its future. In cooperative mode it runs in
 *  CComputeGlobal::Update(), or on a thread that waits for it. The order is that of the values of the timeline, also
 *  across threads: each submission waits until the one before it completed.
 *  A kernel must not use the queue that runs it: that would wait for itself.
 */
class GUCEF_COMPUTE_PUBLIC_CPP CCpuComputeQueue : public CComputeQueue
{
    public:

    CCpuComputeQueue( CComputeDevice* device, UInt32 queueIndex, TComputeExecutionPolicy policy, bool cooperative, const TCpuParallelForPtr& parallelFor );

    virtual ~CCpuComputeQueue() GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt64 GetCompletedValue( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual TComputeResult WaitForValue( UInt64 value, UInt32 timeoutInMs ) GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual TComputeResult OnSubmit( const CComputeCommandList& list, UInt64 fenceValue ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TComputeResult OnExecute( const CComputeCommandList& list, UInt64 fenceValue, TComputeExecutionPolicy policy ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TComputeResult ProcessPendingWork( CComputePendingWork& work, UInt32 timeoutInMs, CString& errorText ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnDeviceLost( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnQueueClosed( TComputeResult reason ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    /** Executes the commands of the list in order on this thread (and the CPU workers for dispatches) */
    TComputeResult ExecuteCommands( const CComputeCommandList& list, TComputeExecutionPolicy policy, CString& errorText );

    TComputeResult ExecuteDispatch( const CComputeCommandList::SCommand& command, TComputeExecutionPolicy policy );

    CCpuComputeQueue( const CCpuComputeQueue& src );            /**< not implemented */
    CCpuComputeQueue& operator=( const CCpuComputeQueue& src ); /**< not implemented */

    private:

    TCpuParallelForPtr m_parallelFor;      /**< held, so dispatches in flight outlive an unload of the module global */
    CCpuComputeFence m_timeline;
    CComputeCommandList::SCommand m_commandScratch;     /**< one command read back at a time, guarded by the execution order */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCPUCOMPUTEQUEUE_H ? */
