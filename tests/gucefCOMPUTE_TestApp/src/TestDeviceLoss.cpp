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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

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

#include "TestDeviceLoss.h"

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

void
Pump( CORE::PulseGeneratorPtr pulseGenerator, UInt32 times )
{
    for ( UInt32 i=0; i<times; ++i )
    {
        pulseGenerator->ForceDirectPulse();
        MT::PrecisionDelay( 1 );
    }
}

}

/*-------------------------------------------------------------------------*/

void
PerformDeviceLossTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "DeviceLoss" );

    GUCEF_TESTFW_TESTCASE( "DeviceLoss: pending futures fail with DEVICE_LOST, one event, new work refused, other devices unaffected" )
    try
    {
        CORE::PulseGeneratorPtr pulseGenerator = CORE::CPulseGenerator::CreateSharedObj();
        TComputeDevicePtr device = CreateCpuTestDevice( 1, pulseGenerator );
        TComputeQueuePtr queue = device->GetDefaultQueue();
        CEventCounter lost( device.GetPointerAlways(), CComputeDevice::DeviceLostEvent );

        TComputeBufferPtr flag = MakeBuffer( device, 4 );
        TComputeBufferPtr data = MakeBuffer( device, 64 );
        CComputeCommandList gate;
        ASSERT_TRUE( RecordGate( gate, device, flag ) );
        CComputeCommandList fill;
        fill.Fill( data, 0, 64, 3 );

        CloseGate();
        CORE::CFutureResult inFlight = queue->ExecuteAsync( gate );
        ASSERT_TRUE( WaitUntilGateEntered( 10000 ) );
        GUCEF::vector< CORE::CFutureResult > pending;
        for ( UInt32 i=0; i<3; ++i )
            pending.push_back( queue->ExecuteAsync( fill ) );
        CComputeFence fence;
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == queue->Submit( fill, fence ) );
        ASSERT_FALSE( inFlight.IsReady() );

        ASSERT_TRUE( device->SimulateDeviceLoss( "simulated by the test" ) );
        ASSERT_FALSE( device->SimulateDeviceLoss( "a second time" ) );
        ASSERT_TRUE( device->IsLost() );
        ASSERT_TRUE( device->GetLostReason() == "simulated by the test" );
        ASSERT_TRUE( queue->IsLost() );
        OpenGate();

        ASSERT_TRUE( AwaitFuture( inFlight, 10000 ) );
        ASSERT_TRUE( COMPUTERESULT_DEVICE_LOST == ComputeResultFromFuture( inFlight ) );
        bool allLost = true;
        for ( size_t i=0; i<pending.size(); ++i )
            allLost = allLost && AwaitFuture( pending[ i ], 10000 ) && COMPUTERESULT_DEVICE_LOST == ComputeResultFromFuture( pending[ i ] );
        ASSERT_TRUE( allLost );
        ASSERT_TRUE( COMPUTERESULT_DEVICE_LOST == fence.Wait( 10000 ) );
        ASSERT_FALSE( fence.IsSignalled() );

        // the event reaches observers on the thread of the pulse generator, exactly once
        Pump( pulseGenerator, 20 );
        ASSERT_TRUE( 1 == lost.GetCount() );
        ASSERT_TRUE( lost.GetLastText() == "simulated by the test" );

        // new work is refused
        ASSERT_TRUE( COMPUTERESULT_DEVICE_LOST == queue->Execute( fill ) );
        CComputeFence refused;
        ASSERT_TRUE( COMPUTERESULT_DEVICE_LOST == queue->Submit( fill, refused ) );
        CORE::CFutureResult refusedFuture = queue->ExecuteAsync( fill );
        ASSERT_TRUE( COMPUTERESULT_DEVICE_LOST == ComputeResultFromFuture( refusedFuture ) );
        TComputeBufferPtr buffer;
        TComputeResult result = COMPUTERESULT_SUCCESS;
        ASSERT_FALSE( device->CreateBuffer( CComputeBufferDesc( 16, BUFFERUSAGE_STORAGE, MEMORYLOCATION_UNIFIED ), buffer, &result ) );
        ASSERT_TRUE( COMPUTERESULT_DEVICE_LOST == result );
        TComputeKernelPtr kernel;
        ASSERT_FALSE( device->CreateKernel( CCpuTestKernels::FillKernelId, kernel, &result ) );
        ASSERT_TRUE( COMPUTERESULT_DEVICE_LOST == result );
        ASSERT_FALSE( device->IsKernelAvailable( CCpuTestKernels::FillKernelId ) );
        void* mapped = GUCEF_NULL;
        ASSERT_TRUE( COMPUTERESULT_DEVICE_LOST == data->Map( MAPACCESS_READ, mapped ) );

        Pump( pulseGenerator, 5 );
        ASSERT_TRUE( 1 == lost.GetCount() );

        // another device works on
        TComputeDevicePtr other = CreateCpuTestDevice();
        TComputeBufferPtr otherData = MakeBuffer( other, 64 );
        UInt32 value = 0;
        CComputeCommandList otherList;
        otherList.Fill( otherData, 0, 64, 8 );
        otherList.ReadToHost( otherData, 60, &value, 4 );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == other->GetDefaultQueue()->Execute( otherList ) );
        ASSERT_TRUE( 8 == value );
    }
    catch( ... )
    {
        OpenGate();
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DeviceLoss: a device whose last references are held by pending work completes the work, then goes on the completion task" )
    try
    {
        TComputeDevicePtr device = CreateCpuTestDevice();
        TComputeBufferPtr data = MakeBuffer( device, 4096 );
        UInt32 values[ 1024 ];
        CComputeCommandList list;
        list.Fill( data, 0, 4096, 11 );
        list.ReadToHost( data, 0, values, sizeof( values ) );
        GUCEF::vector< CORE::CFutureResult > futures;
        for ( UInt32 i=0; i<20; ++i )
            futures.push_back( device->GetDefaultQueue()->ExecuteAsync( list ) );
        list.Clear();
        data.Unlink();
        device.Unlink();     // the pending work holds the buffer, the buffer the device: the last of them goes on the completion task

        bool all = true;
        for ( size_t i=0; i<futures.size(); ++i )
            all = all && AwaitFuture( futures[ i ], 10000 ) && COMPUTERESULT_SUCCESS == ComputeResultFromFuture( futures[ i ] );
        ASSERT_TRUE( all );
        ASSERT_TRUE( 11 == values[ 0 ] && 11 == values[ 1023 ] );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DeviceLoss: a device released by the caller with work pending waits for that work" )
    try
    {
        TComputeDevicePtr device = CreateCpuTestDevice();
        TComputeBufferPtr flag = MakeBuffer( device, 4 );
        UInt32 seen = 0;
        CComputeCommandList list;
        ASSERT_TRUE( RecordGate( list, device, flag ) );
        list.ReadToHost( flag, 0, &seen, 4 );
        CloseGate();
        CORE::CFutureResult future = device->GetDefaultQueue()->ExecuteAsync( list );
        ASSERT_TRUE( WaitUntilGateEntered( 10000 ) );
        list.Clear();
        flag.Unlink();
        OpenGate();
        // the pending work may or may not still hold the device here: either way the work completes
        device.Unlink();
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

    GUCEF_TESTFW_TESTCASE( "DeviceLoss: a queue that outlives its device reports the device lost" )
    try
    {
        TComputeQueuePtr queue;
        {
            TComputeDevicePtr device = CreateCpuTestDevice();
            queue = device->GetDefaultQueue();
        }
        ASSERT_TRUE( GUCEF_NULL == queue->GetDevice() );
        ASSERT_TRUE( queue->IsLost() );
        CComputeCommandList empty;
        ASSERT_TRUE( COMPUTERESULT_DEVICE_LOST == queue->Execute( empty ) );
        ASSERT_TRUE( COMPUTERESULT_DEVICE_LOST == ComputeResultFromFuture( queue->ExecuteAsync( empty ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
