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

#include "TestBackendAvailability.h"

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

bool
Contains( const CString::StringVector& names, const char* name )
{
    for ( size_t i=0; i<names.size(); ++i )
        if ( names[ i ] == name )
            return true;
    return false;
}

}

/*-------------------------------------------------------------------------*/

void
PerformBackendAvailabilityTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "BackendAvailability" );

    CComputeBackendRegistry& registry = CComputeGlobal::Instance()->GetBackendRegistry();
    const CComputeSettings originalSettings = CComputeGlobal::Instance()->GetSettings();

    GUCEF_TESTFW_TESTCASE( "BackendAvailability: a backend without its runtime is skipped once with a system log line, the CPU device serves" )
    try
    {
        CTestComputeBackend* backend = GUCEF_NEW CTestComputeBackend();
        TComputeBackendPtr backendPtr( backend );
        CEventCounter changes( &registry.AsNotifier(), CComputeBackendRegistry::BackendAvailabilityChangedEvent );
        CLogCapture capture;

        registry.SetForcedInitializeFailure( CTestComputeBackend::Name, true );
        ASSERT_TRUE( registry.RegisterBackend( backendPtr ) );
        ASSERT_FALSE( registry.RegisterBackend( backendPtr ) );
        ASSERT_TRUE( 0 == backend->initializeCalls.Load() );
        ASSERT_FALSE( backend->IsAvailable() );
        ASSERT_FALSE( registry.IsBackendAvailable( CTestComputeBackend::Name ) );
        CString::StringVector unavailable;
        registry.GetUnavailableBackendNames( unavailable );
        ASSERT_TRUE( Contains( unavailable, CTestComputeBackend::Name ) );
        ASSERT_FALSE( Contains( unavailable, "cpu" ) );
        ASSERT_TRUE( 1 == changes.GetCount() );
        ASSERT_TRUE( changes.GetLastText() == CTestComputeBackend::Name );
        ASSERT_TRUE( 1 == capture.CountOccurrences( "\"testgpu\" is not available" ) );

        TComputeBackendPtr found;
        ASSERT_TRUE( registry.TryFindBackend( "TestGPU", found ) );
        ASSERT_TRUE( found.GetPointerAlways() == backend );

        CComputeBackendRegistry::TComputeBackendPtrVector usable;
        registry.GetBackendsByPriority( usable );
        ASSERT_TRUE( 1 == usable.size() && usable[ 0 ]->GetName() == "cpu" );

        TComputeDevicePtr device;
        ASSERT_TRUE( registry.CreateBestDevice( CComputeDeviceRequirements(), device ) );
        ASSERT_TRUE( DEVICETYPE_CPU == device->GetInfo().GetDeviceType() );

        TComputeDeviceInfoVector devices;
        ASSERT_TRUE( registry.EnumerateAllDevices( devices ) );
        ASSERT_TRUE( 1 == devices.size() );

        // the runtime arrives: initialised again it is available and its discrete device wins
        registry.SetForcedInitializeFailure( CTestComputeBackend::Name, false );
        ASSERT_TRUE( registry.ReinitializeBackend( CTestComputeBackend::Name ) );
        ASSERT_TRUE( 1 == backend->initializeCalls.Load() );
        ASSERT_TRUE( registry.IsBackendAvailable( CTestComputeBackend::Name ) );
        ASSERT_TRUE( 2 == changes.GetCount() );
        ASSERT_TRUE( 1 == capture.CountOccurrences( "\"testgpu\" is not available" ) );

        registry.GetBackendsByPriority( usable );
        ASSERT_TRUE( 2 == usable.size() && usable[ 0 ]->GetName() == CTestComputeBackend::Name );
        ASSERT_TRUE( registry.EnumerateAllDevices( devices ) );
        ASSERT_TRUE( 2 == devices.size() && DEVICETYPE_DISCRETE_GPU == devices[ 0 ].GetDeviceType() );

        ASSERT_TRUE( registry.CreateBestDevice( CComputeDeviceRequirements(), device ) );
        ASSERT_TRUE( device->GetBackendName() == CTestComputeBackend::Name );
        ASSERT_TRUE( DEVICETYPE_DISCRETE_GPU == device->GetInfo().GetDeviceType() );
        ASSERT_TRUE( device->GetInfo().GetIdentifier() == "testgpu:10de:2c02:0" );

        // the test GPU runs the CPU artefacts
        TComputeKernelPtr fill;
        ASSERT_TRUE( device->CreateKernel( CCpuTestKernels::FillKernelId, fill ) );
        TComputeBufferPtr out = MakeBuffer( device, 16 );
        CCpuTestKernels::SFillConstants constants = { 0, 0, 1, 1 };
        CComputeArguments args;
        args.SetBuffer( 0, out );
        args.SetConstants( &constants, sizeof( constants ) );
        UInt32 values[ 4 ] = { 0, 0, 0, 0 };
        CComputeCommandList list;
        list.Dispatch( fill, args, CComputeDispatchSize::ForItems( 4 ) );
        list.ReadToHost( out, 0, values, 16 );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == device->GetDefaultQueue()->Execute( list ) );
        ASSERT_TRUE( 1 == values[ 0 ] && 4 == values[ 3 ] );
        device.Unlink();
        fill.Unlink();
        out.Unlink();

        // reinitialising without a change sends no event
        ASSERT_TRUE( registry.ReinitializeBackend( CTestComputeBackend::Name ) );
        ASSERT_TRUE( 1 == backend->shutdownCalls.Load() );
        ASSERT_TRUE( 2 == changes.GetCount() );
        ASSERT_FALSE( registry.ReinitializeBackend( "nosuchbackend" ) );

        // unregistered it shuts down and the CPU device serves again
        ASSERT_TRUE( registry.TryUnregister( "TESTGPU" ) );
        ASSERT_FALSE( registry.TryUnregister( CTestComputeBackend::Name ) );
        ASSERT_TRUE( 2 == backend->shutdownCalls.Load() );
        ASSERT_TRUE( 3 == changes.GetCount() );
        ASSERT_TRUE( registry.CreateBestDevice( CComputeDeviceRequirements(), device ) );
        ASSERT_TRUE( device->GetBackendName() == "cpu" );
    }
    catch( ... )
    {
        registry.SetForcedInitializeFailure( CTestComputeBackend::Name, false );
        registry.TryUnregister( CTestComputeBackend::Name );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BackendAvailability: settings disable backends, order them and prefer a device; requirements filter devices" )
    try
    {
        TComputeBackendPtr backendPtr( GUCEF_NEW CTestComputeBackend() );
        ASSERT_TRUE( registry.RegisterBackend( backendPtr ) );
        TComputeDevicePtr device;

        CComputeSettings settings( originalSettings );
        CString::StringSet disabled;
        disabled.insert( "TestGpu" );
        settings.SetDisabledBackends( disabled );
        CComputeGlobal::Instance()->ApplySettings( settings );
        ASSERT_FALSE( registry.IsBackendAvailable( CTestComputeBackend::Name ) );
        ASSERT_TRUE( registry.CreateBestDevice( CComputeDeviceRequirements(), device ) );
        ASSERT_TRUE( device->GetBackendName() == "cpu" );

        settings.SetDisabledBackends( CString::StringSet() );
        CString::StringSet enabled;
        enabled.insert( "cpu" );
        settings.SetEnabledBackends( enabled );
        CComputeGlobal::Instance()->ApplySettings( settings );
        ASSERT_FALSE( registry.IsBackendAvailable( CTestComputeBackend::Name ) );
        settings.SetEnabledBackends( CString::StringSet() );

        // the order of the settings beats the device type
        CString::StringVector order;
        order.push_back( "cpu" );
        order.push_back( "testgpu" );
        settings.SetBackendOrder( order );
        CComputeGlobal::Instance()->ApplySettings( settings );
        ASSERT_TRUE( registry.CreateBestDevice( CComputeDeviceRequirements(), device ) );
        ASSERT_TRUE( device->GetBackendName() == "cpu" );

        // the preferred device beats the order
        settings.SetPreferredDevice( "testgpu:10de:2c02:0" );
        CComputeGlobal::Instance()->ApplySettings( settings );
        ASSERT_TRUE( registry.CreateBestDevice( CComputeDeviceRequirements(), device ) );
        ASSERT_TRUE( device->GetBackendName() == CTestComputeBackend::Name );

        settings.SetPreferredDevice( "" );
        settings.SetBackendOrder( CString::StringVector() );
        CComputeGlobal::Instance()->ApplySettings( settings );

        // requirements: only a CPU device, no CPU device, a feature no device has, a backend
        CComputeDeviceRequirements cpuOnly;
        cpuOnly.AllowDeviceType( DEVICETYPE_CPU );
        ASSERT_TRUE( registry.CreateBestDevice( cpuOnly, device ) );
        ASSERT_TRUE( DEVICETYPE_CPU == device->GetInfo().GetDeviceType() );

        CComputeDeviceRequirements noCpu;
        noCpu.SetCpuDeviceAcceptable( false );
        ASSERT_TRUE( registry.CreateBestDevice( noCpu, device ) );
        ASSERT_TRUE( device->GetBackendName() == CTestComputeBackend::Name );

        CComputeDeviceRequirements rayQuery;
        rayQuery.SetRequiredFeatures( COMPUTEFEATURE_RAY_QUERY );
        TComputeResult result = COMPUTERESULT_SUCCESS;
        ASSERT_FALSE( registry.CreateBestDevice( rayQuery, device, &result ) );
        ASSERT_TRUE( device.IsNULL() );
        ASSERT_TRUE( COMPUTERESULT_UNSUPPORTED == result );

        CComputeDeviceRequirements onlyCpuBackend;
        onlyCpuBackend.AllowBackend( "CPU" );
        ASSERT_TRUE( registry.CreateBestDevice( onlyCpuBackend, device ) );
        ASSERT_TRUE( device->GetBackendName() == "cpu" );

        device.Unlink();
        ASSERT_TRUE( registry.TryUnregister( CTestComputeBackend::Name ) );

        // without the test backend, a requirement only it met cannot be met
        ASSERT_FALSE( registry.CreateBestDevice( noCpu, device, &result ) );
        ASSERT_TRUE( COMPUTERESULT_UNSUPPORTED == result );
        CComputeGlobal::Instance()->ApplySettings( originalSettings );
    }
    catch( ... )
    {
        registry.TryUnregister( CTestComputeBackend::Name );
        CComputeGlobal::Instance()->ApplySettings( originalSettings );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BackendAvailability: unregistering a backend whose device still exists marks the device lost first" )
    try
    {
        TComputeBackendPtr backendPtr( GUCEF_NEW CTestComputeBackend() );
        ASSERT_TRUE( registry.RegisterBackend( backendPtr ) );
        TComputeDevicePtr device;
        ASSERT_TRUE( registry.CreateDevice( CTestComputeBackend::Name, CComputeDeviceParams(), device ) );
        TComputeDevicePtr cpu = CreateCpuTestDevice();
        TComputeBufferPtr flag = MakeBuffer( device, 16 );
        CComputeCommandList gate;
        ASSERT_TRUE( RecordGate( gate, device, flag ) );
        CloseGate();
        CORE::CFutureResult running = device->GetDefaultQueue()->ExecuteAsync( gate );
        ASSERT_TRUE( WaitUntilGateEntered( 10000 ) );
        CORE::CFutureResult pending = device->GetDefaultQueue()->ExecuteAsync( gate );

        CLogCapture capture;
        ASSERT_TRUE( registry.TryUnregister( CTestComputeBackend::Name ) );
        ASSERT_TRUE( device->IsLost() );
        ASSERT_TRUE( device->GetLostReason() == "the backend was unregistered" );
        ASSERT_FALSE( cpu->IsLost() );
        ASSERT_TRUE( 1 == capture.CountOccurrences( "still exist while it is unregistered" ) );
        OpenGate();
        ASSERT_TRUE( AwaitFuture( running, 10000 ) && COMPUTERESULT_DEVICE_LOST == ComputeResultFromFuture( running ) );
        ASSERT_TRUE( AwaitFuture( pending, 10000 ) && COMPUTERESULT_DEVICE_LOST == ComputeResultFromFuture( pending ) );
        ASSERT_TRUE( COMPUTERESULT_DEVICE_LOST == device->GetDefaultQueue()->Execute( gate ) );
    }
    catch( ... )
    {
        OpenGate();
        registry.TryUnregister( CTestComputeBackend::Name );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BackendAvailability: kernels needing f64 run on a GPU only where the settings allow it or the device is fast at it" )
    try
    {
        TComputeBackendPtr backendPtr( GUCEF_NEW CTestComputeBackend() );
        ASSERT_TRUE( registry.RegisterBackend( backendPtr ) );
        CComputeKernelRegistry& kernels = CComputeGlobal::Instance()->GetKernelRegistry();
        TComputeKernelDeclPtr fillDecl;
        TComputeKernelArtefactPtr fillArtefact;
        ASSERT_TRUE( kernels.TryGetKernel( CCpuTestKernels::FillKernelId, fillDecl ) );
        ASSERT_TRUE( kernels.GetArtefact( CCpuTestKernels::FillKernelId, "cpu", fillArtefact ) );
        CComputeKernelDecl decl( *fillDecl );
        decl.SetKernelId( "TEST.Fill.F64" );
        decl.GetSignature().SetRequiredFeatures( COMPUTEFEATURE_FLOAT64 );
        static int owner = 0;
        ASSERT_TRUE( kernels.RegisterKernel( decl, &owner ) );
        ASSERT_TRUE( kernels.RegisterArtefact( "TEST.Fill.F64", *fillArtefact ) );

        TComputeDevicePtr gpu;
        ASSERT_TRUE( registry.CreateDevice( CTestComputeBackend::Name, CComputeDeviceParams(), gpu ) );
        // the test GPU reports fast f64 (it is the CPU)
        ASSERT_TRUE( gpu->IsKernelAvailable( "TEST.Fill.F64" ) );
        TComputeDevicePtr cpu = CreateCpuTestDevice();
        ASSERT_TRUE( cpu->IsKernelAvailable( "TEST.Fill.F64" ) );

        kernels.UnregisterModuleKernels( &owner );
        gpu.Unlink();
        registry.TryUnregister( CTestComputeBackend::Name );
    }
    catch( ... )
    {
        registry.TryUnregister( CTestComputeBackend::Name );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BackendAvailability: the CPU backend is built-in and stays when everything that came from a plugin is unregistered" )
    try
    {
        ASSERT_TRUE( registry.IsRegistered( "cpu" ) );
        ASSERT_TRUE( registry.IsBuiltIn( "cpu" ) );

        // a backend a plugin would add is not built-in, and goes when the plugins are unregistered
        ASSERT_TRUE( registry.RegisterBackend( TComputeBackendPtr( GUCEF_NEW CTestComputeBackend() ) ) );
        ASSERT_FALSE( registry.IsBuiltIn( CTestComputeBackend::Name ) );
        registry.UnregisterAllExceptBuiltIn();
        ASSERT_FALSE( registry.IsRegistered( CTestComputeBackend::Name ) );
        ASSERT_TRUE( registry.IsRegistered( "cpu" ) && registry.IsBuiltIn( "cpu" ) );

        // and the CPU backend still makes devices
        TComputeDevicePtr cpu = CreateCpuTestDevice();
        ASSERT_FALSE( cpu.IsNULL() );
    }
    catch( ... )
    {
        registry.TryUnregister( CTestComputeBackend::Name );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
