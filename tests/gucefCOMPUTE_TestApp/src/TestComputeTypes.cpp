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

#include <math.h>

#include <string.h>

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_COMPUTE_H
#include "gucefCOMPUTE.h"
#define GUCEF_COMPUTE_H
#endif /* GUCEF_COMPUTE_H ? */

#include "TestComputeTypes.h"

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

/*-------------------------------------------------------------------------*/

void
PerformComputeTypesTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "ComputeTypes" );

    GUCEF_TESTFW_TESTCASE( "ComputeTypes: results, device types, formats and tiers have names that read back" )
    try
    {
        bool roundTrip = true;
        for ( UInt32 r=COMPUTERESULT_SUCCESS; r<=COMPUTERESULT_BACKEND_ERROR; ++r )
        {
            TComputeResult parsed = COMPUTERESULT_SUCCESS;
            roundTrip = roundTrip && StringToComputeResult( ComputeResultToString( static_cast< TComputeResult >( r ) ), parsed ) && parsed == static_cast< TComputeResult >( r );
        }
        ASSERT_TRUE( roundTrip );
        ASSERT_TRUE( 0 == strcmp( "COMPUTERESULT_DEVICE_LOST", ComputeResultToString( COMPUTERESULT_DEVICE_LOST ) ) );
        ASSERT_TRUE( 0 == strcmp( "COMPUTERESULT_UNKNOWN", ComputeResultToString( static_cast< TComputeResult >( 999 ) ) ) );
        TComputeResult untouched = COMPUTERESULT_TIMEOUT;
        ASSERT_FALSE( StringToComputeResult( "nonsense", untouched ) );
        ASSERT_TRUE( COMPUTERESULT_TIMEOUT == untouched );
        ASSERT_TRUE( ComputeResultToFutureInfo( COMPUTERESULT_DEVICE_LOST, "gone" ) == "COMPUTERESULT_DEVICE_LOST: gone" );
        ASSERT_TRUE( ComputeResultToFutureInfo( COMPUTERESULT_TIMEOUT, "" ) == "COMPUTERESULT_TIMEOUT" );

        CORE::CFutureResultSource failed;
        failed.Fail( CORE::TASKSTATUS_STARTUP_FAILED, ComputeResultToFutureInfo( COMPUTERESULT_COMPILE_FAILED, "line 3: unknown identifier" ) );
        CString errorText;
        ASSERT_TRUE( COMPUTERESULT_COMPILE_FAILED == ComputeResultFromFuture( failed.GetFuture(), &errorText ) );
        ASSERT_TRUE( errorText == "line 3: unknown identifier" );
        CORE::CFutureResultSource done;
        done.Complete();
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == ComputeResultFromFuture( done.GetFuture() ) );
        CORE::CFutureResultSource running;
        ASSERT_TRUE( COMPUTERESULT_TIMEOUT == ComputeResultFromFuture( running.GetFuture() ) );
        running.Complete();
        CORE::CFutureResultSource foreign;
        foreign.Fail( CORE::TASKSTATUS_STARTUP_FAILED, "something else" );
        ASSERT_TRUE( COMPUTERESULT_BACKEND_ERROR == ComputeResultFromFuture( foreign.GetFuture(), &errorText ) );
        ASSERT_TRUE( errorText == "something else" );

        ASSERT_TRUE( 0 == strcmp( "discrete", ComputeDeviceTypeToString( DEVICETYPE_DISCRETE_GPU ) ) );
        ASSERT_TRUE( 0 == strcmp( "cpu", ComputeDeviceTypeToString( DEVICETYPE_CPU ) ) );
        ASSERT_TRUE( 0 == strcmp( "virtual", ComputeDeviceTypeToString( DEVICETYPE_VIRTUAL_GPU ) ) );

        bool formats = true;
        for ( UInt32 f=ARTEFACTFORMAT_UNKNOWN; f<=ARTEFACTFORMAT_OPENCL_C_SOURCE; ++f )
            formats = formats && static_cast< TComputeArtefactFormat >( f ) == StringToComputeArtefactFormat( ComputeArtefactFormatToString( static_cast< TComputeArtefactFormat >( f ) ) );
        ASSERT_TRUE( formats );
        ASSERT_TRUE( ARTEFACTFORMAT_SPIRV == StringToComputeArtefactFormat( "SPIRV" ) );
        ASSERT_TRUE( ARTEFACTFORMAT_UNKNOWN == StringToComputeArtefactFormat( "dxil" ) );
        ASSERT_TRUE( 0 == strcmp( "spv", ComputeArtefactFormatToFileExtension( ARTEFACTFORMAT_SPIRV ) ) );
        ASSERT_TRUE( 0 == strcmp( "", ComputeArtefactFormatToFileExtension( ARTEFACTFORMAT_CPU ) ) );

        TComputeProviderTier tier = PROVIDERTIER_SCALAR;
        ASSERT_TRUE( StringToComputeProviderTier( "Accelerator", tier ) && PROVIDERTIER_ACCELERATOR == tier );
        ASSERT_TRUE( StringToComputeProviderTier( "none", tier ) && PROVIDERTIER_NONE == tier );
        ASSERT_FALSE( StringToComputeProviderTier( "gpu", tier ) );
        ASSERT_TRUE( 0 == strcmp( "multicore", ComputeProviderTierToString( PROVIDERTIER_MULTICORE ) ) );

        ASSERT_TRUE( 4 == ComputeElementTypeSize( ELEMENTTYPE_FLOAT32 ) );
        ASSERT_TRUE( 8 == ComputeElementTypeSize( ELEMENTTYPE_UINT64 ) );
        ASSERT_TRUE( 2 == ComputeElementTypeSize( ELEMENTTYPE_FLOAT16 ) );
        ASSERT_TRUE( 0 == ComputeElementTypeSize( ELEMENTTYPE_UNKNOWN ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ComputeTypes: capabilities default to the portable limits, requirements judge devices" )
    try
    {
        CComputeDeviceCapabilities caps;
        ASSERT_TRUE( 256 == caps.GetMaxInvocationsPerGroup() );
        ASSERT_TRUE( 16 * 1024 == caps.GetMaxWorkgroupMemoryInBytes() );
        ASSERT_TRUE( 8 == caps.GetMaxStorageBuffersPerKernel() );
        ASSERT_TRUE( 128 == caps.GetMaxConstantsSizeInBytes() );
        ASSERT_TRUE( 65535 == caps.GetMaxGroupsPerDispatch( 0 ) && 0 == caps.GetMaxGroupsPerDispatch( 3 ) );
        ASSERT_TRUE( 64 == caps.GetMaxGroupSize( 2 ) );
        ASSERT_TRUE( static_cast< UInt64 >( 256 ) * 1024 * 1024 == caps.GetMaxBufferSizeInBytes() );
        ASSERT_TRUE( static_cast< UInt64 >( 128 ) * 1024 * 1024 == caps.GetMaxBindingSizeInBytes() );
        ASSERT_TRUE( 1 == caps.GetQueueCount() );
        ASSERT_TRUE( 0 == caps.GetMemoryBudgetInBytes() );
        ASSERT_TRUE( caps.HasFeatures( 0 ) );
        ASSERT_FALSE( caps.HasFeatures( COMPUTEFEATURE_FLOAT64 ) );
        caps.SetFeature( COMPUTEFEATURE_FLOAT64 | COMPUTEFEATURE_SUBGROUPS, true );
        ASSERT_TRUE( caps.HasFeatures( COMPUTEFEATURE_FLOAT64 ) && caps.HasFeatures( COMPUTEFEATURE_FLOAT64 | COMPUTEFEATURE_SUBGROUPS ) );
        caps.SetFeature( COMPUTEFEATURE_SUBGROUPS, false );
        ASSERT_TRUE( COMPUTEFEATURE_FLOAT64 == caps.GetFeatures() );
        caps.SetSubgroupSizeRange( 4, 128 );
        ASSERT_TRUE( 4 == caps.GetMinSubgroupSize() && 128 == caps.GetMaxSubgroupSize() );
        caps.SetQueueCount( 0 );
        ASSERT_TRUE( 1 == caps.GetQueueCount() );
        ASSERT_TRUE( 256 == CComputeDeviceCapabilities::PortableLimits().GetMaxInvocationsPerGroup() );

        CComputeDeviceInfo gpu;
        gpu.SetBackendName( "vulkan" );
        gpu.SetVendorId( 0x10DE );
        gpu.SetDeviceId( 0x2C02 );
        gpu.SetDeviceIndex( 1 );
        gpu.SetDeviceType( DEVICETYPE_DISCRETE_GPU );
        gpu.SetCapabilities( caps );
        ASSERT_TRUE( gpu.GetIdentifier() == "vulkan:10de:2c02:1" );

        CComputeDeviceInfo cpu;
        cpu.SetBackendName( "cpu" );
        cpu.SetDeviceType( DEVICETYPE_CPU );

        CComputeDeviceRequirements any;
        ASSERT_TRUE( any.IsMetBy( gpu ) && any.IsMetBy( cpu ) );
        CComputeDeviceRequirements f64;
        f64.SetRequiredFeatures( COMPUTEFEATURE_FLOAT64 );
        ASSERT_TRUE( f64.IsMetBy( gpu ) );
        ASSERT_FALSE( f64.IsMetBy( cpu ) );
        CComputeDeviceRequirements noCpu;
        noCpu.SetCpuDeviceAcceptable( false );
        ASSERT_FALSE( noCpu.IsMetBy( cpu ) );
        CComputeDeviceRequirements integratedOnly;
        integratedOnly.AllowDeviceType( DEVICETYPE_INTEGRATED_GPU );
        ASSERT_FALSE( integratedOnly.IsMetBy( gpu ) );
        CComputeDeviceRequirements vulkanOnly;
        vulkanOnly.AllowBackend( "Vulkan" );
        ASSERT_TRUE( vulkanOnly.IsMetBy( gpu ) );
        ASSERT_FALSE( vulkanOnly.IsMetBy( cpu ) );
        CComputeDeviceRequirements bigGroups;
        bigGroups.SetMinInvocationsPerGroup( 1024 );
        ASSERT_FALSE( bigGroups.IsMetBy( gpu ) );
        CComputeDeviceRequirements bigMemory;
        bigMemory.SetMinWorkgroupMemoryInBytes( 32 * 1024 );
        ASSERT_FALSE( bigMemory.IsMetBy( gpu ) );
        CComputeDeviceRequirements manyBuffers;
        manyBuffers.SetMinStorageBuffersPerKernel( 9 );
        ASSERT_FALSE( manyBuffers.IsMetBy( gpu ) );
        CComputeDeviceRequirements bigConstants;
        bigConstants.SetMinConstantsSizeInBytes( 256 );
        ASSERT_FALSE( bigConstants.IsMetBy( gpu ) );
        CComputeDeviceRequirements bigBuffers;
        bigBuffers.SetMinBufferSizeInBytes( static_cast< UInt64 >( 1 ) << 32 );
        ASSERT_FALSE( bigBuffers.IsMetBy( gpu ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ComputeTypes: signatures, dispatch sizes, arguments, declarations and artefacts" )
    try
    {
        CComputeKernelSignature signature;
        ASSERT_TRUE( 64 == signature.GetInvocationsPerGroup() );
        ASSERT_TRUE( 8 == signature.GetConstantsSizeInBytes() );
        ASSERT_TRUE( signature.AddBinding( 3, COMPUTEACCESS_WRITE, ELEMENTTYPE_FLOAT64 ) );
        ASSERT_TRUE( signature.AddBinding( 0, COMPUTEACCESS_READ, ELEMENTTYPE_UINT8 ) );
        ASSERT_FALSE( signature.AddBinding( 3, COMPUTEACCESS_READ, ELEMENTTYPE_UINT8 ) );
        ASSERT_FALSE( signature.AddBinding( 8, COMPUTEACCESS_READ, ELEMENTTYPE_UINT8 ) );
        ASSERT_TRUE( 2 == signature.GetBindingCount() );
        ASSERT_TRUE( 0x9u == signature.GetBoundSlotMask() );
        CComputeKernelSignature::SBinding binding;
        ASSERT_TRUE( signature.FindBinding( 3, binding ) && COMPUTEACCESS_WRITE == binding.access );
        ASSERT_FALSE( signature.FindBinding( 1, binding ) );
        ASSERT_TRUE( signature.GetBinding( 1, binding ) && 0 == binding.slot );
        ASSERT_FALSE( signature.GetBinding( 2, binding ) );
        ASSERT_TRUE( 8 == signature.GetOutputBytesPerItem() );
        ASSERT_FALSE( signature.SetConstantsSizeInBytes( 4 ) );
        ASSERT_FALSE( signature.SetConstantsSizeInBytes( 129 ) );
        ASSERT_TRUE( signature.SetConstantsSizeInBytes( 128 ) );
        ASSERT_FALSE( signature.SetWorkgroupSize( 0, 1, 1 ) );
        ASSERT_TRUE( signature.SetWorkgroupSize( 8, 8, 2 ) );
        ASSERT_TRUE( 128 == signature.GetInvocationsPerGroup() );
        ASSERT_TRUE( 2 == signature.GetWorkgroupSize( 2 ) && 1 == signature.GetWorkgroupSize( 3 ) );
        CComputeKernelSignature full;
        bool added = true;
        for ( UInt32 s=0; s<8; ++s )
            added = added && full.AddBinding( 7 - s, COMPUTEACCESS_READ, ELEMENTTYPE_UNKNOWN );
        ASSERT_TRUE( added );
        ASSERT_TRUE( 4 == full.GetOutputBytesPerItem() );

        CComputeKernelSignature groupOf256;
        groupOf256.SetWorkgroupSize( 256, 1, 1 );
        CComputeDispatchSize items = CComputeDispatchSize::ForItems( 1000 );
        ASSERT_TRUE( items.IsItems() );
        ASSERT_TRUE( 4 == items.GetGroupCount( 0, groupOf256 ) && 1 == items.GetGroupCount( 1, groupOf256 ) );
        ASSERT_TRUE( 4 == items.GetTotalGroupCount( groupOf256 ) );
        ASSERT_TRUE( 1000 == items.GetItemCount( groupOf256 ) );
        ASSERT_TRUE( 0 == CComputeDispatchSize::ForItems( 0 ).GetTotalGroupCount( groupOf256 ) );
        ASSERT_TRUE( 0 == CComputeDispatchSize().GetTotalGroupCount( groupOf256 ) );
        CComputeDispatchSize groups = CComputeDispatchSize::ForGroups( 3, 2, 2 );
        ASSERT_FALSE( groups.IsItems() );
        ASSERT_TRUE( 12 == groups.GetTotalGroupCount( groupOf256 ) );
        ASSERT_TRUE( 12 * 256 == groups.GetItemCount( groupOf256 ) );
        ASSERT_TRUE( 0 == groups.GetGroupCount( 3, groupOf256 ) );

        CComputeArguments args;
        ASSERT_TRUE( 0 == args.GetBoundSlotMask() );
        TComputeBufferPtr none;
        ASSERT_FALSE( args.SetBuffer( 0, none ) );
        UInt8 constants[ 129 ];
        memset( constants, 7, sizeof( constants ) );
        ASSERT_FALSE( args.SetConstants( constants, 129 ) );
        ASSERT_FALSE( args.SetConstants( GUCEF_NULL, 8 ) );
        ASSERT_TRUE( args.SetConstants( constants, 128 ) );
        ASSERT_TRUE( 128 == args.GetConstantsSizeInBytes() );
        ASSERT_TRUE( 7 == static_cast< const UInt8* >( args.GetConstants() )[ 127 ] );
        CComputeArguments copy( args );
        ASSERT_TRUE( 128 == copy.GetConstantsSizeInBytes() );
        args.Clear();
        ASSERT_TRUE( 0 == args.GetConstantsSizeInBytes() );
        ASSERT_TRUE( args.GetBinding( 9 ).buffer.IsNULL() );

        CComputeKernelDecl decl( "MATH.TransformPoints3.F32", 4 );
        ASSERT_TRUE( decl.IsValid() );
        ASSERT_TRUE( DETERMINISM_FLOAT_STRICT == decl.GetDeterminism() );
        decl.GetSignature().SetRequiredFeatures( COMPUTEFEATURE_SUBGROUPS );
        ASSERT_TRUE( COMPUTEFEATURE_SUBGROUPS == decl.GetRequiredFeatures() );
        decl.SetPreferredGrain( 16 );
        ASSERT_TRUE( 16 == decl.GetPreferredGrain() );
        decl.SetKernelId( "MATH.TransformPoints3" );
        ASSERT_FALSE( decl.IsValid() );
        ASSERT_FALSE( CComputeKernelDecl().IsValid() );

        CComputeKernelArtefact artefact;
        ASSERT_FALSE( artefact.IsValid() );
        static const UInt8 spirv[ 4 ] = { 0x03, 0x02, 0x23, 0x07 };
        artefact.SetBackendName( "Vulkan" );
        artefact.SetFormat( ARTEFACTFORMAT_SPIRV );
        artefact.SetEntryPoint( "main" );
        artefact.SetStaticData( spirv, sizeof( spirv ) );
        ASSERT_TRUE( artefact.IsValid() );
        ASSERT_TRUE( artefact.GetBackendName() == "vulkan" );
        ASSERT_TRUE( artefact.GetData() == spirv );
        CORE::CDynamicBuffer owned;
        owned.CopyFrom( 3, "abc" );
        artefact.SetOwnedData( owned );
        ASSERT_TRUE( artefact.HasOwnedData() && 3 == artefact.GetDataSizeInBytes() && artefact.GetData() != spirv );
        CComputeKernelArtefact assigned;
        assigned = artefact;
        ASSERT_TRUE( 0 == memcmp( assigned.GetData(), "abc", 3 ) );
        ASSERT_FALSE( CComputeKernelArtefact::ForCpu( GUCEF_NULL ).IsValid() );
        ASSERT_TRUE( CComputeKernelArtefact::ForCpu( GUCEF_NULL ).GetBackendName() == "cpu" );

        CComputeBufferDesc desc;
        ASSERT_TRUE( 0 == desc.GetSizeInBytes() && MEMORYLOCATION_DEVICE == desc.GetMemoryLocation() && !desc.IsHostVisible() );
        desc.SetUsage( BUFFERUSAGE_STORAGE | BUFFERUSAGE_COPY_DST );
        ASSERT_TRUE( desc.HasUsage( BUFFERUSAGE_COPY_DST ) && !desc.HasUsage( BUFFERUSAGE_COPY_SRC ) );
        desc.SetMemoryLocation( MEMORYLOCATION_READBACK );
        ASSERT_TRUE( desc.IsHostVisible() );

        CComputeDeviceParams params;
        ASSERT_TRUE( 1 == params.GetQueueCount() && 0 == params.GetDeviceIndex() );
        params.SetQueueCount( 0 );
        ASSERT_TRUE( 1 == params.GetQueueCount() );
        params.SetExecutionPolicy( EXECUTIONPOLICY_CALLING_THREAD_ONLY );
        ASSERT_TRUE( EXECUTIONPOLICY_CALLING_THREAD_ONLY == params.GetExecutionPolicy() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ComputeTypes: tolerances count ULPs like a Float64 reference and apply their statistic" )
    try
    {
        ASSERT_TRUE( 0 == CComputeTolerance::UlpDistance( 1.0f, 1.0f ) );
        ASSERT_TRUE( 0 == CComputeTolerance::UlpDistance( 0.0f, -0.0f ) );
        ASSERT_TRUE( 1 == CComputeTolerance::UlpDistance( 1.0f, nextafterf( 1.0f, 2.0f ) ) );
        ASSERT_TRUE( 2 == CComputeTolerance::UlpDistance( nextafterf( 0.0f, 1.0f ), nextafterf( 0.0f, -1.0f ) ) );
        ASSERT_TRUE( 1 == CComputeTolerance::UlpDistance( 1.0, nextafter( 1.0, 0.0 ) ) );
        const Float32 nan = sqrtf( -1.0f );
        ASSERT_TRUE( 0 == CComputeTolerance::UlpDistance( nan, nan ) );
        ASSERT_TRUE( 0 != CComputeTolerance::UlpDistance( nan, 1.0f ) );

        // independent reference: count the steps of nextafterf between two values
        Float32 a = 1.0f;
        UInt64 steps = 0;
        while ( a < 1.0001f )
        {
            a = nextafterf( a, 2.0f );
            ++steps;
        }
        ASSERT_TRUE( steps == CComputeTolerance::UlpDistance( 1.0f, a ) );

        CComputeTolerance exact;
        ASSERT_TRUE( exact.IsExact() );
        ASSERT_TRUE( exact.IsWithin( 2.5f, 2.5f ) );
        ASSERT_FALSE( exact.IsWithin( 2.5f, nextafterf( 2.5f, 3.0f ) ) );
        ASSERT_TRUE( exact.IsWithin( nan, nan ) );

        CComputeTolerance ulps( 2, 0.0, 0.0 );
        ASSERT_FALSE( ulps.IsExact() );
        ASSERT_TRUE( ulps.IsWithin( 1.0f, nextafterf( nextafterf( 1.0f, 2.0f ), 2.0f ) ) );
        ASSERT_FALSE( ulps.IsWithin( 1.0f, 1.001f ) );
        CComputeTolerance absolute( 0, 1.0e-3, 0.0 );
        ASSERT_TRUE( absolute.IsWithin( 0.0, 0.0005 ) );
        ASSERT_FALSE( absolute.IsWithin( 0.0, 0.002 ) );
        CComputeTolerance relative( 0, 0.0, 1.0e-3 );
        ASSERT_TRUE( relative.IsWithin( 1000.0, 1000.5 ) );
        ASSERT_FALSE( relative.IsWithin( 1000.0, 1002.0 ) );
        ASSERT_FALSE( relative.IsWithin( 0.0, 1.0e-9 ) );
        ASSERT_FALSE( absolute.IsWithin( 1.0f, sqrtf( -1.0f ) ) );

        Float32 expected[ 2000 ];
        Float32 actual[ 2000 ];
        for ( UInt32 i=0; i<2000; ++i )
        {
            expected[ i ] = static_cast< Float32 >( i ) * 0.5f;
            actual[ i ] = expected[ i ];
        }
        actual[ 10 ] = expected[ 10 ] + 1.0f;
        CComputeTolerance::SComparison comparison;
        ASSERT_FALSE( ulps.Compare( expected, actual, 2000, &comparison ) );
        ASSERT_TRUE( 1 == comparison.outsideCount && 10 == comparison.firstOutsideIndex && 2000 == comparison.count );
        ASSERT_TRUE( 1.0 == comparison.maxAbsoluteError );
        CComputeTolerance percentile( 2, 0.0, 0.0, CComputeTolerance::TOLERANCESTATISTIC_PERCENTILE_99_9 );
        ASSERT_TRUE( percentile.Compare( expected, actual, 2000 ) );
        actual[ 20 ] = -1.0f;
        actual[ 30 ] = -1.0f;
        ASSERT_FALSE( percentile.Compare( expected, actual, 2000, &comparison ) );
        ASSERT_TRUE( 3 == comparison.outsideCount );
        ASSERT_FALSE( ulps.Compare( static_cast< const Float32* >( GUCEF_NULL ), actual, 1 ) );
        const Float32* noFloats = GUCEF_NULL;
        ASSERT_TRUE( ulps.Compare( noFloats, noFloats, 0 ) );
        Float64 expected64[ 2 ] = { 1.0, 2.0 };
        Float64 actual64[ 2 ] = { 1.0, 2.0 };
        ASSERT_TRUE( exact.Compare( expected64, actual64, 2 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ComputeTypes: aligned memory is aligned, writable and freed" )
    try
    {
        ASSERT_TRUE( GUCEF_NULL == AlignedAllocate( 0, 64 ) );
        ASSERT_TRUE( GUCEF_NULL == AlignedAllocate( 16, 48 ) );
        const UInt32 alignments[ 4 ] = { 8, 16, 64, 4096 };
        bool ok = true;
        for ( UInt32 i=0; i<4; ++i )
        {
            void* memory = AlignedAllocate( 1000, alignments[ i ] );
            ok = ok && GUCEF_NULL != memory && 0 == ( reinterpret_cast< size_t >( memory ) % alignments[ i ] );
            if ( GUCEF_NULL != memory )
                memset( memory, 0xAB, 1000 );
            AlignedFree( memory );
        }
        ASSERT_TRUE( ok );
        AlignedFree( GUCEF_NULL );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ComputeTypes: settings survive a config round trip, loading keeps what the tree lacks" )
    try
    {
        CComputeSettings settings;
        ASSERT_TRUE( 0 == settings.GetMaxCpuWorkerCount() );
        ASSERT_FALSE( settings.GetAllowFloat64OnGpu() );
        ASSERT_FALSE( settings.GetValidation() );

        CString::StringSet disabled;
        disabled.insert( "OpenCL" );
        settings.SetDisabledBackends( disabled );
        CString::StringSet enabled;
        enabled.insert( "vulkan" );
        enabled.insert( "cpu" );
        settings.SetEnabledBackends( enabled );
        CString::StringVector order;
        order.push_back( "Vulkan" );
        order.push_back( "cuda" );
        order.push_back( "cpu" );
        settings.SetBackendOrder( order );
        settings.SetPreferredDevice( "vulkan:10de:2c02:0" );
        settings.SetMaxCpuWorkerCount( 6 );
        settings.SetKernelOverrideRoot( "kernels/override" );
        settings.SetValidation( true );
        settings.SetAllowFloat64OnGpu( true );
        CComputeSettings::SKernelProviderOverride providerOverride;
        providerOverride.forcedTier = PROVIDERTIER_MULTICORE;
        providerOverride.disabledTiers = ( 1u << PROVIDERTIER_ACCELERATOR ) | ( 1u << PROVIDERTIER_SIMD );
        providerOverride.minCount[ PROVIDERTIER_MULTICORE ] = 4096;
        settings.SetKernelProviderOverride( "MATH.TransformPoints3.F32", providerOverride );
        CComputeSettings::SKernelProviderOverride plain;
        settings.SetKernelProviderOverride( "MATH.DotProducts3.F32", plain );
        ASSERT_TRUE( settings.GetBackendOrder()[ 0 ] == "vulkan" );
        ASSERT_TRUE( providerOverride.IsTierDisabled( PROVIDERTIER_SIMD ) && !providerOverride.IsTierDisabled( PROVIDERTIER_SCALAR ) );

        CORE::CDataNode tree;
        ASSERT_TRUE( settings.SaveConfig( tree ) );
        ASSERT_TRUE( GUCEF_NULL != tree.FindChild( CComputeSettings::ConfigSectionName ) );

        CComputeSettings loaded;
        ASSERT_TRUE( loaded.LoadConfig( tree ) );
        ASSERT_TRUE( loaded.GetDisabledBackends() == settings.GetDisabledBackends() );
        ASSERT_TRUE( loaded.GetEnabledBackends() == settings.GetEnabledBackends() );
        ASSERT_TRUE( loaded.GetBackendOrder() == settings.GetBackendOrder() );
        ASSERT_TRUE( loaded.GetPreferredDevice() == "vulkan:10de:2c02:0" );
        ASSERT_TRUE( 6 == loaded.GetMaxCpuWorkerCount() );
        ASSERT_TRUE( loaded.GetKernelOverrideRoot() == "kernels/override" );
        ASSERT_TRUE( loaded.GetValidation() && loaded.GetAllowFloat64OnGpu() );
        CComputeSettings::SKernelProviderOverride readBack;
        ASSERT_TRUE( loaded.GetKernelProviderOverride( "MATH.TransformPoints3.F32", readBack ) );
        ASSERT_TRUE( readBack == providerOverride );
        ASSERT_TRUE( loaded.GetKernelProviderOverride( "MATH.DotProducts3.F32", readBack ) );
        ASSERT_TRUE( readBack == plain );
        ASSERT_TRUE( 2 == loaded.GetKernelProviderOverrides().size() );
        ASSERT_TRUE( loaded.RemoveKernelProviderOverride( "MATH.DotProducts3.F32" ) );
        ASSERT_FALSE( loaded.RemoveKernelProviderOverride( "MATH.DotProducts3.F32" ) );

        // the section itself loads too; a tree without the section is refused and changes nothing
        CComputeSettings fromSection;
        ASSERT_TRUE( fromSection.LoadConfig( *tree.FindChild( CComputeSettings::ConfigSectionName ) ) );
        ASSERT_TRUE( 6 == fromSection.GetMaxCpuWorkerCount() );
        CORE::CDataNode emptyTree;
        ASSERT_FALSE( fromSection.LoadConfig( emptyTree ) );
        ASSERT_TRUE( 6 == fromSection.GetMaxCpuWorkerCount() );

        // a partial section keeps the other values; unknown tiers are ignored
        CORE::CDataNode partial;
        CORE::CDataNode* section = partial.AddChild( CComputeSettings::ConfigSectionName );
        section->SetAttribute( "maxCpuWorkers", static_cast< UInt32 >( 2 ) );
        CORE::CDataNode* kernel = section->AddChild( "Kernel" );
        kernel->SetAttribute( "id", "MATH.TransformPoints3.F32" );
        kernel->SetAttribute( "forcedTier", "warp" );
        ASSERT_TRUE( loaded.LoadConfig( partial ) );
        ASSERT_TRUE( 2 == loaded.GetMaxCpuWorkerCount() );
        ASSERT_TRUE( loaded.GetPreferredDevice() == "vulkan:10de:2c02:0" );
        ASSERT_TRUE( loaded.GetKernelProviderOverride( "MATH.TransformPoints3.F32", readBack ) );
        ASSERT_TRUE( PROVIDERTIER_MULTICORE == readBack.forcedTier );

        CComputeSettings assigned;
        assigned = settings;
        ASSERT_TRUE( 6 == assigned.GetMaxCpuWorkerCount() );
        ASSERT_TRUE( assigned.GetClassTypeName() == "GUCEF::COMPUTE::CComputeSettings" );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
