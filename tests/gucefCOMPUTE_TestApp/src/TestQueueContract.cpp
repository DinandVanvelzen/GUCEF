/*
 *  gucefCOMPUTE_TestApp: GUCEF test application for the Compute module
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
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

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_COMPUTE_H
#include "gucefCOMPUTE.h"
#define GUCEF_COMPUTE_H
#endif /* GUCEF_COMPUTE_H ? */

#ifndef GUCEF_COMPUTE_TESTAPP_TESTCOMPUTESUPPORT_H
#include "TestComputeSupport.h"
#define GUCEF_COMPUTE_TESTAPP_TESTCOMPUTESUPPORT_H
#endif /* GUCEF_COMPUTE_TESTAPP_TESTCOMPUTESUPPORT_H ? */

#include "TestQueueContract.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::COMPUTE;
using namespace GUCEF::COMPUTE::TEST;

/*-------------------------------------------------------------------------*/

namespace {

/** Runs a list synchronously on a queue */
class CExecuteBody : public CTestThreadBody
{
    public:
    CExecuteBody( const TComputeQueuePtr& queue, const CComputeCommandList& list ) : CTestThreadBody(), result( COMPUTERESULT_BACKEND_ERROR ), m_queue( queue ), m_list( list ) {}
    virtual void Run( void ) GUCEF_VIRTUAL_OVERRIDE { result = m_queue->Execute( m_list ); }
    TComputeResult result;
    private:
    TComputeQueuePtr m_queue;
    CComputeCommandList m_list;
};

UInt64
NowInMs( void )
{
    return static_cast< UInt64 >( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() ) );
}

}

/*-------------------------------------------------------------------------*/

void
PerformQueueContractTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "QueueContract" );

    GUCEF_TESTFW_TESTCASE( "QueueContract: a submission the backend refuses takes no timeline value, nothing waits for it" )
    try
    {
        TComputeDevicePtr device = CreateCpuTestDevice();
        TComputeDevicePtr deferredDevice = CTestDeferredDevice::Create();
        CTestDeferredQueue& queue = static_cast< CTestDeferredDevice* >( deferredDevice.GetPointerAlways() )->GetDeferredQueue();
        CComputeCommandList empty;
        CComputeFence fence;
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == queue.Submit( empty, fence ) );
        ASSERT_TRUE( 1 == fence.GetValue() );

        queue.failNextSubmit.Store( 1 );
        CComputeFence refused;
        ASSERT_TRUE( COMPUTERESULT_BACKEND_ERROR == queue.Submit( empty, refused ) );
        ASSERT_TRUE( 1 == queue.GetLastSubmittedFence().GetValue() );
        queue.failNextSubmit.Store( 1 );
        ASSERT_TRUE( COMPUTERESULT_BACKEND_ERROR == queue.Execute( empty ) );
        ASSERT_TRUE( 1 == queue.GetLastSubmittedFence().GetValue() );
        queue.failNextSubmit.Store( 1 );
        ASSERT_TRUE( COMPUTERESULT_BACKEND_ERROR == ComputeResultFromFuture( queue.ExecuteAsync( empty ) ) );
        ASSERT_TRUE( 1 == queue.GetLastSubmittedFence().GetValue() );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == queue.WaitIdle( 1000 ) );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == queue.Execute( empty ) );
        ASSERT_TRUE( 2 == queue.GetLastSubmittedFence().GetValue() );

        // a refused submission leaves the buffers it would use as they were: they map
        TComputeBufferPtr buffer = MakeBuffer( deferredDevice, 16 );
        CComputeCommandList fill;
        fill.Fill( buffer, 0, 16, 3 );
        queue.failNextSubmit.Store( 1 );
        ASSERT_TRUE( COMPUTERESULT_BACKEND_ERROR == queue.Execute( fill ) );
        ASSERT_FALSE( buffer->IsInUseByWorkInFlight() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "QueueContract: fences are host-visible completion, the host copies of a backend run before the timeline advances" )
    try
    {
        TComputeDevicePtr device = CTestDeferredDevice::Create();
        CTestDeferredQueue& queue = static_cast< CTestDeferredDevice* >( device.GetPointerAlways() )->GetDeferredQueue();
        TComputeBufferPtr buffer = MakeBuffer( device, 16 );
        UInt32 host[ 4 ] = { 0, 0, 0, 0 };
        CComputeCommandList list;
        list.Fill( buffer, 0, 16, 9 );
        list.ReadToHost( buffer, 0, host, 16 );

        CComputeFence fence;
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == queue.Submit( list, fence ) );
        // done on the device, not yet on the host: not signalled
        ASSERT_TRUE( 1 == queue.GetDeviceCompletedValue() );
        ASSERT_TRUE( 0 == queue.GetCompletedValue() || 9 == host[ 0 ] );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == fence.Wait( 10000 ) );
        ASSERT_TRUE( fence.IsSignalled() );
        ASSERT_TRUE( 9 == host[ 0 ] && 9 == host[ 3 ] );
        ASSERT_TRUE( 1 == queue.hostTransfers.Load() );

        // synchronous: the data is on the host when Execute returns
        memset( host, 0, sizeof( host ) );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == queue.Execute( list ) );
        ASSERT_TRUE( 9 == host[ 0 ] && 9 == host[ 3 ] );

        // asynchronous: the data is on the host when the future completes, the completion task did the copies
        memset( host, 0, sizeof( host ) );
        CORE::CFutureResult future = queue.ExecuteAsync( list );
        ASSERT_TRUE( AwaitFuture( future, 10000 ) );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == ComputeResultFromFuture( future ) );
        ASSERT_TRUE( 9 == host[ 0 ] && 9 == host[ 3 ] );
        ASSERT_TRUE( 3 == queue.hostTransfers.Load() );
        ASSERT_TRUE( 3 == queue.GetCompletedValue() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "QueueContract: a synchronous Execute that waits does not block ExecuteAsync of another thread, order holds" )
    try
    {
        TComputeDevicePtr device = CreateCpuTestDevice();
        TComputeQueuePtr queue = device->GetDefaultQueue();
        TComputeBufferPtr flag = MakeBuffer( device, 16 );
        CComputeCommandList gate;
        ASSERT_TRUE( RecordGate( gate, device, flag ) );
        UInt32 seen = 0;
        CComputeCommandList after;
        UInt32 seven = 7;
        after.ReadToHost( flag, 0, &seen, 4 );
        after.WriteFromHost( flag, 8, &seven, 4 );

        CloseGate();
        CExecuteBody execute( queue, gate );
        CTestThread thread;
        ASSERT_TRUE( thread.Start( &execute ) );
        ASSERT_TRUE( WaitUntilGateEntered( 10000 ) );

        const UInt64 start = NowInMs();
        CORE::CFutureResult future = queue->ExecuteAsync( after );
        const UInt64 tookMs = NowInMs() - start;
        ASSERT_TRUE( tookMs < 500 );
        MT::PrecisionDelay( 20 );
        ASSERT_FALSE( future.IsReady() );       // in order: after the synchronous work at the gate
        OpenGate();
        ASSERT_TRUE( thread.Join( 10000 ) );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == execute.result );
        ASSERT_TRUE( AwaitFuture( future, 10000 ) );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == ComputeResultFromFuture( future ) );
        ASSERT_TRUE( 1 == seen );
    }
    catch( ... )
    {
        OpenGate();
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "QueueContract: cooperative completion: pending work completes in Update() or in a wait on the calling thread, in order" )
    try
    {
        CComputeDeviceParams params;
        params.SetCooperativeCompletion( true );
        TComputeDevicePtr device;
        ASSERT_TRUE( CComputeGlobal::Instance()->GetBackendRegistry().CreateDevice( "cpu", params, device ) );
        TComputeQueuePtr queue = device->GetDefaultQueue();
        ASSERT_TRUE( queue->IsCooperative() );
        TComputeBufferPtr buffer = MakeBuffer( device, 16 );

        UInt32 first = 0;
        CComputeCommandList fillOne;
        fillOne.Fill( buffer, 0, 16, 1 );
        fillOne.ReadToHost( buffer, 0, &first, 4 );
        CORE::CFutureResult future = queue->ExecuteAsync( fillOne );
        MT::PrecisionDelay( 30 );
        ASSERT_FALSE( future.IsReady() );
        ASSERT_TRUE( 1 == queue->GetPendingWorkCount() );
        ASSERT_TRUE( 1 <= CComputeGlobal::Instance()->Update() );
        ASSERT_TRUE( future.IsReady() );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == ComputeResultFromFuture( future ) );
        ASSERT_TRUE( 1 == first );
        ASSERT_TRUE( 0 == queue->GetPendingWorkCount() );
        ASSERT_TRUE( 0 == CComputeGlobal::Instance()->Update() );

        // a fence wait runs the pending work on the waiting thread
        UInt32 second = 0;
        CComputeCommandList fillTwo;
        fillTwo.Fill( buffer, 0, 16, 2 );
        fillTwo.ReadToHost( buffer, 0, &second, 4 );
        CComputeFence fence;
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == queue->Submit( fillTwo, fence ) );
        ASSERT_FALSE( fence.IsSignalled() );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == fence.Wait() );
        ASSERT_TRUE( 2 == second );

        // a synchronous call runs the pending work before it, in order
        CComputeCommandList fillThree;
        fillThree.Fill( buffer, 0, 16, 3 );
        CORE::CFutureResult three = queue->ExecuteAsync( fillThree );
        UInt32 last = 0;
        CComputeCommandList read;
        read.ReadToHost( buffer, 0, &last, 4 );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == queue->Execute( read ) );
        ASSERT_TRUE( 3 == last );
        ASSERT_TRUE( three.IsReady() && COMPUTERESULT_SUCCESS == ComputeResultFromFuture( three ) );

        // released with work pending: it fails
        CComputeCommandList fillFour;
        fillFour.Fill( buffer, 0, 16, 4 );
        CORE::CFutureResult four = queue->ExecuteAsync( fillFour );
        fillFour.Clear();
        buffer.Unlink();
        queue.Unlink();
        device.Unlink();
        // the pending work holds the buffer and so the device: Update() completes it, then the device goes
        CComputeGlobal::Instance()->Update();
        ASSERT_TRUE( four.IsReady() && COMPUTERESULT_SUCCESS == ComputeResultFromFuture( four ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "QueueContract: when its pool stops the completion task, pending work fails, the queue closes, no value is skipped, the device is released" )
    try
    {
        // the devices of earlier tests are all gone (none leaks)
        ASSERT_TRUE( WaitForDeviceCount( 0, 5000 ) );
        const UInt32 devicesBefore = 0;
        TComputeDevicePtr device = CreateCpuTestDevice();
        TComputeQueuePtr queue = device->GetDefaultQueue();
        TComputeBufferPtr flag = MakeBuffer( device, 16 );
        CComputeCommandList gate;
        ASSERT_TRUE( RecordGate( gate, device, flag ) );

        CloseGate();
        CORE::CFutureResult running = queue->ExecuteAsync( gate );
        ASSERT_TRUE( WaitUntilGateEntered( 10000 ) );
        CORE::CFutureResult pending = queue->ExecuteAsync( gate );
        CComputeFence pendingFence;
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == queue->Submit( gate, pendingFence ) );

        // what the application does when it shuts down; new work stays possible for the other tests
        CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool( CComputeCompletionTaskConsumer::ThreadPoolName );
        ASSERT_TRUE( !pool.IsNULL() );
        pool->RequestAllTasksToStop( false, true, 0 );
        OpenGate();

        ASSERT_TRUE( AwaitFuture( running, 10000 ) );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == ComputeResultFromFuture( running ) );
        ASSERT_TRUE( AwaitFuture( pending, 10000 ) );
        ASSERT_TRUE( COMPUTERESULT_BACKEND_ERROR == ComputeResultFromFuture( pending ) );
        ASSERT_TRUE( COMPUTERESULT_BACKEND_ERROR == pendingFence.Wait( 10000 ) );
        ASSERT_TRUE( queue->IsClosed() );
        ASSERT_TRUE( 1 == queue->GetCompletedValue() );
        ASSERT_TRUE( 0 == queue->GetPendingWorkCount() );

        // closed for good: refused, nothing hangs, no later value is reported as completed
        ASSERT_TRUE( COMPUTERESULT_BACKEND_ERROR == queue->WaitIdle( 1000 ) );
        ASSERT_TRUE( COMPUTERESULT_BACKEND_ERROR == queue->Execute( gate ) );
        CComputeFence refused;
        ASSERT_TRUE( COMPUTERESULT_BACKEND_ERROR == queue->Submit( gate, refused ) );
        ASSERT_TRUE( COMPUTERESULT_BACKEND_ERROR == ComputeResultFromFuture( queue->ExecuteAsync( gate ) ) );
        ASSERT_TRUE( 1 == queue->GetCompletedValue() );

        // nothing pending keeps the device: released, it goes
        pendingFence = CComputeFence();
        gate.Clear();
        flag.Unlink();
        queue.Unlink();
        device.Unlink();
        ASSERT_TRUE( WaitForDeviceCount( devicesBefore, 5000 ) );

        // a new device gets a new completion task
        TComputeDevicePtr fresh = CreateCpuTestDevice();
        CComputeCommandList empty;
        CORE::CFutureResult works = fresh->GetDefaultQueue()->ExecuteAsync( empty );
        ASSERT_TRUE( AwaitFuture( works, 10000 ) && COMPUTERESULT_SUCCESS == ComputeResultFromFuture( works ) );
    }
    catch( ... )
    {
        OpenGate();
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
