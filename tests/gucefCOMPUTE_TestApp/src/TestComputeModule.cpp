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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

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

#include "TestComputeModule.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformComputeModuleTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "ComputeModule: The global singleton exists and is stable, Update does nothing yet" )
    try
    {
        COMPUTE::CComputeGlobal* global = COMPUTE::CComputeGlobal::Instance();
        ASSERT_TRUE( GUCEF_NULL != global );
        ASSERT_TRUE( global == COMPUTE::CComputeGlobal::Instance() );
        global->Update();
        ASSERT_TRUE( COMPUTE::ComputeModulePublicSpecProbe() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformComputeModuleUnloadTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "ComputeModuleUnload" );

    GUCEF_TESTFW_TESTCASE( "ComputeModuleUnload: unloading with a device held and work pending marks it lost, nothing is created again until the module loads" )
    try
    {
        using namespace GUCEF::COMPUTE;
        using namespace GUCEF::COMPUTE::TEST;

        TComputeDevicePtr device = CreateCpuTestDevice();
        TComputeQueuePtr queue = device->GetDefaultQueue();
        TComputeBufferPtr flag = MakeBuffer( device, 16 );
        CComputeCommandList gate;
        ASSERT_TRUE( RecordGate( gate, device, flag ) );
        CloseGate();
        CORE::CFutureResult running = queue->ExecuteAsync( gate );
        ASSERT_TRUE( WaitUntilGateEntered( 10000 ) );
        CORE::CFutureResult pending = queue->ExecuteAsync( gate );

        ASSERT_TRUE( ComputeModuleSimulateUnload() );
        ASSERT_TRUE( GUCEF_NULL == CComputeGlobal::InstanceIfCreated() );
        ASSERT_TRUE( GUCEF_NULL == CComputeGlobal::Instance() );      // not created again
        ASSERT_TRUE( device->IsLost() );
        ASSERT_TRUE( device->GetLostReason() == "the gucefCOMPUTE module was unloaded" );

        // the work in flight ends on the job system the queue holds, after the global is gone
        OpenGate();
        ASSERT_TRUE( AwaitFuture( running, 10000 ) && COMPUTERESULT_DEVICE_LOST == ComputeResultFromFuture( running ) );
        ASSERT_TRUE( AwaitFuture( pending, 10000 ) && COMPUTERESULT_DEVICE_LOST == ComputeResultFromFuture( pending ) );
        ASSERT_TRUE( COMPUTERESULT_DEVICE_LOST == queue->Execute( gate ) );
        TComputeKernelPtr kernel;
        ASSERT_FALSE( device->CreateKernel( GateKernelId, kernel ) );
        gate.Clear();
        flag.Unlink();
        queue.Unlink();
        device.Unlink();
        ASSERT_TRUE( GUCEF_NULL == CComputeGlobal::InstanceIfCreated() );

        ASSERT_TRUE( ComputeModuleSimulateLoad() );
        CComputeGlobal* global = CComputeGlobal::Instance();
        ASSERT_TRUE( GUCEF_NULL != global );
        if ( GUCEF_NULL != global )
        {
            ASSERT_TRUE( 0 == global->GetDeviceCount() );
            TComputeDevicePtr fresh = CreateCpuTestDevice();
            ASSERT_TRUE( !fresh.IsNULL() );
            CComputeCommandList empty;
            CORE::CFutureResult works = fresh->GetDefaultQueue()->ExecuteAsync( empty );
            ASSERT_TRUE( AwaitFuture( works, 10000 ) && COMPUTERESULT_SUCCESS == ComputeResultFromFuture( works ) );
        }
    }
    catch( ... )
    {
        GUCEF::COMPUTE::TEST::OpenGate();
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
