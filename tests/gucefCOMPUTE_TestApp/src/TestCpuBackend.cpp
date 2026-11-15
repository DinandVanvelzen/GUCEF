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

#include "TestCpuBackend.h"

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

/** A simple seeded generator for test data (xorshift32) */
UInt32
NextRandom( UInt32& state )
{
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

bool
Run( const TComputeDevicePtr& device, const CComputeCommandList& list )
{
    return COMPUTERESULT_SUCCESS == device->GetDefaultQueue()->Execute( list );
}

}

/*-------------------------------------------------------------------------*/

void
PerformCpuBackendTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "CpuBackend" );

    GUCEF_TESTFW_TESTCASE( "CpuBackend: the best device with default requirements is the CPU device with its capabilities" )
    try
    {
        CComputeBackendRegistry& registry = CComputeGlobal::Instance()->GetBackendRegistry();
        ASSERT_TRUE( registry.IsBackendAvailable( "cpu" ) );
        ASSERT_TRUE( registry.IsBackendAvailable( "CPU" ) );

        TComputeDevicePtr device;
        TComputeResult result = COMPUTERESULT_BACKEND_ERROR;
        ASSERT_TRUE( registry.CreateBestDevice( CComputeDeviceRequirements(), device, &result ) );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == result );
        ASSERT_TRUE( !device.IsNULL() );
        ASSERT_TRUE( device->GetBackendName() == "cpu" );
        ASSERT_TRUE( DEVICETYPE_CPU == device->GetInfo().GetDeviceType() );
        ASSERT_TRUE( 1 == device->GetQueueCount() );
        ASSERT_TRUE( !device->GetDefaultQueue().IsNULL() );
        ASSERT_TRUE( device->GetQueue( 1 ).IsNULL() );
        ASSERT_TRUE( device->GetCapabilities().HasFeatures( COMPUTEFEATURE_FLOAT64 | COMPUTEFEATURE_INT64 | COMPUTEFEATURE_UNIFIED_MEMORY | COMPUTEFEATURE_STRICT_FLOAT ) );
        ASSERT_FALSE( device->GetCapabilities().HasFeatures( COMPUTEFEATURE_RAY_QUERY ) );
        ASSERT_TRUE( 256 == device->GetCapabilities().GetMaxInvocationsPerGroup() );
        ASSERT_TRUE( device->GetInfo().GetIdentifier() == "cpu:0:0:0" );
        ASSERT_FALSE( device->IsLost() );

        CComputeDeviceParams native;
        native.GetBackendOptions().Set( "reportNativeLimits", "true" );
        native.SetQueueCount( 3 );
        TComputeDevicePtr nativeDevice;
        ASSERT_TRUE( registry.CreateDevice( "cpu", native, nativeDevice ) );
        ASSERT_TRUE( 1024 == nativeDevice->GetCapabilities().GetMaxInvocationsPerGroup() );
        ASSERT_TRUE( 3 == nativeDevice->GetQueueCount() );

        CComputeDeviceParams wrong;
        wrong.SetDeviceIdentifier( "cpu:0:0:7" );
        TComputeDevicePtr none;
        ASSERT_FALSE( registry.CreateDevice( "cpu", wrong, none, &result ) );
        ASSERT_TRUE( COMPUTERESULT_INVALID_ARGUMENT == result );
        ASSERT_TRUE( none.IsNULL() );
        ASSERT_FALSE( registry.CreateDevice( "nosuchbackend", CComputeDeviceParams(), none, &result ) );
        ASSERT_TRUE( COMPUTERESULT_UNSUPPORTED == result );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CpuBackend: buffers are zeroed, 64 byte aligned, mappable, and refuse invalid descriptions" )
    try
    {
        TComputeDevicePtr device = CreateCpuTestDevice();
        ASSERT_TRUE( !device.IsNULL() );

        TComputeResult result = COMPUTERESULT_SUCCESS;
        TComputeBufferPtr buffer;
        ASSERT_FALSE( device->CreateBuffer( CComputeBufferDesc( 0, BUFFERUSAGE_STORAGE, MEMORYLOCATION_UNIFIED ), buffer, &result ) );
        ASSERT_TRUE( COMPUTERESULT_INVALID_ARGUMENT == result );
        ASSERT_FALSE( device->CreateBuffer( CComputeBufferDesc( 16, 0, MEMORYLOCATION_UNIFIED ), buffer, &result ) );
        ASSERT_TRUE( COMPUTERESULT_INVALID_ARGUMENT == result );
        const UInt64 tooLarge = device->GetCapabilities().GetMaxBufferSizeInBytes() + 1;
        ASSERT_FALSE( device->CreateBuffer( CComputeBufferDesc( tooLarge, BUFFERUSAGE_STORAGE, MEMORYLOCATION_DEVICE ), buffer, &result ) );
        ASSERT_TRUE( COMPUTERESULT_INVALID_ARGUMENT == result );

        ASSERT_TRUE( device->CreateBuffer( CComputeBufferDesc( 1000, BUFFERUSAGE_STORAGE, MEMORYLOCATION_UNIFIED ), buffer, &result ) );
        ASSERT_TRUE( 1000 == buffer->GetSizeInBytes() );
        ASSERT_TRUE( buffer->GetDevice() == device.GetPointerAlways() );
        ASSERT_TRUE( buffer->IsMappable() );
        {
            CComputeBufferMapScope map( buffer, MAPACCESS_READ_WRITE );
            ASSERT_TRUE( map.IsMapped() );
            ASSERT_TRUE( COMPUTERESULT_SUCCESS == map.GetResult() );
            ASSERT_TRUE( 0 == ( reinterpret_cast< size_t >( map.GetData() ) % 64 ) );
            bool zeroed = true;
            for ( UInt32 i=0; i<1000; ++i )
                zeroed = zeroed && 0 == map.GetDataAs< UInt8 >()[ i ];
            ASSERT_TRUE( zeroed );
            map.GetDataAs< UInt8 >()[ 10 ] = 42;
            ASSERT_TRUE( buffer->IsMapped() );

            // a second map of a mapped buffer is refused, and so is a list that uses it
            void* again = GUCEF_NULL;
            ASSERT_TRUE( COMPUTERESULT_INVALID_ARGUMENT == buffer->Map( MAPACCESS_READ, again ) );
            ASSERT_TRUE( GUCEF_NULL == again );
            CComputeCommandList list;
            ASSERT_TRUE( COMPUTERESULT_SUCCESS == list.Fill( buffer, 0, 8, 1 ) );
            ASSERT_TRUE( COMPUTERESULT_INVALID_ARGUMENT == device->GetDefaultQueue()->Execute( list ) );

            ASSERT_TRUE( map.EarlyUnmap() );
            ASSERT_FALSE( map.EarlyUnmap() );
            ASSERT_FALSE( buffer->IsMapped() );
        }
        ASSERT_TRUE( COMPUTERESULT_INVALID_ARGUMENT == buffer->Unmap() );
        {
            CComputeBufferMapScope map( buffer, MAPACCESS_READ );
            ASSERT_TRUE( 42 == map.GetDataAs< UInt8 >()[ 10 ] );
        }
        ASSERT_FALSE( buffer->IsMapped() );

        // device local memory is not mappable, also on the CPU device
        TComputeBufferPtr local;
        ASSERT_TRUE( device->CreateBuffer( CComputeBufferDesc( 64, BUFFERUSAGE_STORAGE, MEMORYLOCATION_DEVICE ), local ) );
        CComputeBufferMapScope localMap( local, MAPACCESS_READ );
        ASSERT_FALSE( localMap.IsMapped() );
        ASSERT_TRUE( COMPUTERESULT_UNSUPPORTED == localMap.GetResult() );
        ASSERT_TRUE( GUCEF_NULL == localMap.GetData() );

        TComputeBufferPtr nullBuffer;
        CComputeBufferMapScope nullMap( nullBuffer, MAPACCESS_READ );
        ASSERT_FALSE( nullMap.IsMapped() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CpuBackend: command lists copy written data at recording, refuse invalid ranges and keep the first failure" )
    try
    {
        TComputeDevicePtr device = CreateCpuTestDevice();
        TComputeBufferPtr a = MakeBuffer( device, 64 );
        TComputeBufferPtr b = MakeBuffer( device, 64 );

        UInt32 source[ 4 ] = { 1, 2, 3, 4 };
        UInt32 result[ 4 ] = { 0, 0, 0, 0 };
        CComputeCommandList list;
        ASSERT_TRUE( list.IsEmpty() );
        ASSERT_TRUE( GUCEF_NULL == list.GetDevice() );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == list.WriteFromHost( a, 8, source, sizeof( source ) ) );
        source[ 0 ] = 99;     // the list has its own copy
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == list.Copy( a, 8, b, 0, sizeof( source ) ) );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == list.ReadToHost( b, 0, result, sizeof( result ) ) );
        ASSERT_TRUE( 3 == list.GetCommandCount() );
        ASSERT_TRUE( 2 == list.GetBufferCount() );
        ASSERT_TRUE( list.GetDevice() == device.GetPointerAlways() );
        ASSERT_TRUE( Run( device, list ) );
        ASSERT_TRUE( 1 == result[ 0 ] && 2 == result[ 1 ] && 3 == result[ 2 ] && 4 == result[ 3 ] );

        // the same list again, a list runs as often as wanted
        memset( result, 0, sizeof( result ) );
        ASSERT_TRUE( Run( device, list ) );
        ASSERT_TRUE( 1 == result[ 0 ] && 4 == result[ 3 ] );

        // a copy of the list records the same
        CComputeCommandList copy( list );
        memset( result, 0, sizeof( result ) );
        ASSERT_TRUE( copy.GetRecordingSizeInBytes() == list.GetRecordingSizeInBytes() );
        ASSERT_TRUE( Run( device, copy ) );
        ASSERT_TRUE( 2 == result[ 1 ] );

        CComputeCommandList bad;
        ASSERT_TRUE( COMPUTERESULT_INVALID_ARGUMENT == bad.WriteFromHost( a, 60, source, 8 ) );
        ASSERT_TRUE( COMPUTERESULT_INVALID_ARGUMENT == bad.ReadToHost( a, 0, GUCEF_NULL, 8 ) );
        ASSERT_TRUE( COMPUTERESULT_INVALID_ARGUMENT == bad.Fill( a, 2, 8, 0 ) );
        ASSERT_TRUE( COMPUTERESULT_INVALID_ARGUMENT == bad.Copy( a, 0, a, 4, 8 ) );        // overlapping in one buffer
        ASSERT_TRUE( COMPUTERESULT_INVALID_ARGUMENT == bad.Copy( a, 0, b, 60, 8 ) );
        ASSERT_TRUE( 0 == bad.GetCommandCount() );
        ASSERT_TRUE( COMPUTERESULT_INVALID_ARGUMENT == bad.GetRecordingResult() );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == bad.Fill( a, 0, 8, 7 ) );
        ASSERT_TRUE( COMPUTERESULT_INVALID_ARGUMENT == bad.GetRecordingResult() );
        ASSERT_TRUE( COMPUTERESULT_INVALID_ARGUMENT == device->GetDefaultQueue()->Execute( bad ) );
        bad.Clear();
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == bad.GetRecordingResult() );
        ASSERT_TRUE( bad.IsEmpty() );

        // a list for another device is refused by the queue
        TComputeDevicePtr other = CreateCpuTestDevice();
        CComputeCommandList foreign;
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == foreign.Fill( MakeBuffer( other, 16 ), 0, 16, 1 ) );
        ASSERT_TRUE( COMPUTERESULT_INVALID_ARGUMENT == device->GetDefaultQueue()->Execute( foreign ) );
        // and recording objects of two devices into one list fails
        ASSERT_TRUE( COMPUTERESULT_INVALID_ARGUMENT == foreign.Fill( a, 0, 16, 1 ) );

        // an empty list runs and completes
        CComputeCommandList empty;
        ASSERT_TRUE( Run( device, empty ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CpuBackend: commands take effect in recording order, fills repeat their pattern" )
    try
    {
        TComputeDevicePtr device = CreateCpuTestDevice();
        TComputeBufferPtr a = MakeBuffer( device, 32 );
        TComputeBufferPtr b = MakeBuffer( device, 32 );
        UInt32 fromA[ 8 ];
        UInt32 fromB[ 8 ];
        CComputeCommandList list;
        list.Fill( a, 0, 32, 0x11111111u );
        list.Copy( a, 0, b, 0, 32 );
        list.Fill( a, 8, 16, 0x22222222u );
        list.ReadToHost( a, 0, fromA, 32 );
        list.ReadToHost( b, 0, fromB, 32 );
        ASSERT_TRUE( Run( device, list ) );
        bool ok = true;
        for ( UInt32 i=0; i<8; ++i )
        {
            ok = ok && 0x11111111u == fromB[ i ];
            ok = ok && ( ( i >= 2 && i < 6 ) ? 0x22222222u : 0x11111111u ) == fromA[ i ];
        }
        ASSERT_TRUE( ok );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CpuBackend: test kernels give the host reference results, kernels are cached per device" )
    try
    {
        TComputeDevicePtr device = CreateCpuTestDevice();
        const UInt32 count = 100003;     // not a multiple of the group size

        TComputeKernelPtr fill;
        TComputeKernelPtr fillAgain;
        TComputeResult result = COMPUTERESULT_BACKEND_ERROR;
        ASSERT_TRUE( device->CreateKernel( CCpuTestKernels::FillKernelId, fill, &result ) );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == result );
        ASSERT_TRUE( device->CreateKernel( CCpuTestKernels::FillKernelId, fillAgain ) );
        ASSERT_TRUE( fill.GetPointerAlways() == fillAgain.GetPointerAlways() );
        ASSERT_TRUE( fill->IsBound() );
        ASSERT_TRUE( fill->GetDevice() == device.GetPointerAlways() );
        ASSERT_TRUE( fill->GetKernelId() == CCpuTestKernels::FillKernelId );
        ASSERT_TRUE( ARTEFACTFORMAT_CPU == fill->GetArtefact().GetFormat() );
        ASSERT_TRUE( device->IsKernelAvailable( CCpuTestKernels::HashKernelId ) );
        ASSERT_FALSE( device->IsKernelAvailable( "COMPUTE.NoSuchKernel.U32" ) );
        TComputeKernelPtr none;
        ASSERT_FALSE( device->CreateKernel( "COMPUTE.NoSuchKernel.U32", none, &result ) );
        ASSERT_TRUE( COMPUTERESULT_KERNEL_NOT_FOUND == result );

        // fill
        TComputeBufferPtr out = MakeBuffer( device, count * 4 );
        CCpuTestKernels::SFillConstants fillConstants = { 0, 0, 7, 3 };
        CComputeArguments args;
        ASSERT_TRUE( args.SetBuffer( 0, out ) );
        ASSERT_TRUE( args.SetConstants( &fillConstants, sizeof( fillConstants ) ) );
        GUCEF::vector< UInt32 > host( count, 0 );
        CComputeCommandList list;
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == list.Dispatch( fill, args, CComputeDispatchSize::ForItems( count ) ) );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == list.ReadToHost( out, 0, &host[ 0 ], count * 4 ) );
        ASSERT_TRUE( Run( device, list ) );
        bool ok = true;
        for ( UInt32 i=0; i<count; ++i )
            ok = ok && host[ i ] == 7 + i * 3;
        ASSERT_TRUE( ok );

        // hash: exact integer
        TComputeKernelPtr hash;
        ASSERT_TRUE( device->CreateKernel( CCpuTestKernels::HashKernelId, hash ) );
        GUCEF::vector< UInt32 > input( count, 0 );
        UInt32 seed = 12345;
        for ( UInt32 i=0; i<count; ++i )
            input[ i ] = NextRandom( seed );
        TComputeBufferPtr in = MakeBuffer( device, count * 4 );
        CCpuTestKernels::SHashConstants hashConstants = { 0, 0, 0xA5A5A5A5u, 0 };
        CComputeArguments hashArgs;
        hashArgs.SetBuffer( 0, in );
        hashArgs.SetBuffer( 1, out );
        hashArgs.SetConstants( &hashConstants, sizeof( hashConstants ) );
        list.Clear();
        list.WriteFromHost( in, 0, &input[ 0 ], count * 4 );
        list.Dispatch( hash, hashArgs, CComputeDispatchSize::ForItems( count ) );
        list.ReadToHost( out, 0, &host[ 0 ], count * 4 );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == list.GetRecordingResult() );
        ASSERT_TRUE( Run( device, list ) );
        ok = true;
        for ( UInt32 i=0; i<count; ++i )
            ok = ok && host[ i ] == CCpuTestKernels::Hash( input[ i ] ^ 0xA5A5A5A5u );
        ASSERT_TRUE( ok );

        // saxpy: strict float, exact against the same host arithmetic
        TComputeKernelPtr saxpy;
        ASSERT_TRUE( device->CreateKernel( CCpuTestKernels::SaxpyKernelId, saxpy ) );
        GUCEF::vector< Float32 > x( count, 0.0f );
        GUCEF::vector< Float32 > y( count, 0.0f );
        GUCEF::vector< Float32 > expected( count, 0.0f );
        GUCEF::vector< Float32 > actual( count, 0.0f );
        for ( UInt32 i=0; i<count; ++i )
        {
            x[ i ] = static_cast< Float32 >( NextRandom( seed ) % 10000 ) * 0.001f;
            y[ i ] = static_cast< Float32 >( NextRandom( seed ) % 10000 ) * 0.01f;
            volatile Float32 product = 1.5f * x[ i ];
            expected[ i ] = product + y[ i ];
        }
        TComputeBufferPtr xBuffer = MakeBuffer( device, count * 4 );
        TComputeBufferPtr yBuffer = MakeBuffer( device, count * 4 );
        CCpuTestKernels::SSaxpyConstants saxpyConstants = { 0, 0, 1.5f, 0 };
        CComputeArguments saxpyArgs;
        saxpyArgs.SetBuffer( 0, xBuffer );
        saxpyArgs.SetBuffer( 1, yBuffer );
        saxpyArgs.SetConstants( &saxpyConstants, sizeof( saxpyConstants ) );
        list.Clear();
        list.WriteFromHost( xBuffer, 0, &x[ 0 ], count * 4 );
        list.WriteFromHost( yBuffer, 0, &y[ 0 ], count * 4 );
        list.Dispatch( saxpy, saxpyArgs, CComputeDispatchSize::ForItems( count ) );
        list.ReadToHost( yBuffer, 0, &actual[ 0 ], count * 4 );
        ASSERT_TRUE( Run( device, list ) );
        CComputeTolerance::SComparison comparison;
        ASSERT_TRUE( CComputeTolerance().Compare( &expected[ 0 ], &actual[ 0 ], count, &comparison ) );
        ASSERT_TRUE( saxpy->GetDecl().GetTolerance().Compare( &expected[ 0 ], &actual[ 0 ], count ) );
        ASSERT_TRUE( 0 == comparison.outsideCount );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CpuBackend: dispatches check bindings, constants and ranges against the signature" )
    try
    {
        TComputeDevicePtr device = CreateCpuTestDevice();
        TComputeKernelPtr hash;
        ASSERT_TRUE( device->CreateKernel( CCpuTestKernels::HashKernelId, hash ) );
        TComputeBufferPtr a = MakeBuffer( device, 256 );
        TComputeBufferPtr b = MakeBuffer( device, 256 );
        CCpuTestKernels::SHashConstants constants = { 0, 0, 1, 0 };

        CComputeArguments missing;
        missing.SetBuffer( 0, a );
        missing.SetConstants( &constants, sizeof( constants ) );
        CComputeCommandList list;
        ASSERT_TRUE( COMPUTERESULT_INVALID_ARGUMENT == list.Dispatch( hash, missing, CComputeDispatchSize::ForItems( 64 ) ) );

        CComputeArguments extra( missing );
        extra.SetBuffer( 1, b );
        extra.SetBuffer( 2, b );
        ASSERT_TRUE( COMPUTERESULT_INVALID_ARGUMENT == list.Dispatch( hash, extra, CComputeDispatchSize::ForItems( 64 ) ) );

        CComputeArguments wrongConstants;
        wrongConstants.SetBuffer( 0, a );
        wrongConstants.SetBuffer( 1, b );
        wrongConstants.SetConstants( &constants, 12 );
        ASSERT_TRUE( COMPUTERESULT_INVALID_ARGUMENT == list.Dispatch( hash, wrongConstants, CComputeDispatchSize::ForItems( 64 ) ) );

        CComputeArguments outside;
        outside.SetBuffer( 0, a, 128, 256 );
        outside.SetBuffer( 1, b );
        outside.SetConstants( &constants, sizeof( constants ) );
        ASSERT_TRUE( COMPUTERESULT_INVALID_ARGUMENT == list.Dispatch( hash, outside, CComputeDispatchSize::ForItems( 64 ) ) );

        TComputeKernelPtr nullKernel;
        ASSERT_TRUE( COMPUTERESULT_INVALID_ARGUMENT == list.Dispatch( nullKernel, missing, CComputeDispatchSize::ForItems( 64 ) ) );
        ASSERT_TRUE( 0 == list.GetCommandCount() );

        // a kernel of another device
        TComputeDevicePtr other = CreateCpuTestDevice();
        TComputeKernelPtr otherHash;
        ASSERT_TRUE( other->CreateKernel( CCpuTestKernels::HashKernelId, otherHash ) );
        CComputeArguments good;
        good.SetBuffer( 0, a );
        good.SetBuffer( 1, b );
        good.SetConstants( &constants, sizeof( constants ) );
        CComputeCommandList otherList;
        ASSERT_TRUE( COMPUTERESULT_INVALID_ARGUMENT == otherList.Dispatch( otherHash, good, CComputeDispatchSize::ForItems( 64 ) ) );
        CComputeCommandList goodList;
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == goodList.Dispatch( hash, good, CComputeDispatchSize::ForItems( 64 ) ) );
        ASSERT_TRUE( 1 == goodList.GetCommandCount() );

        // no work at all is fine
        CComputeCommandList nothing;
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == nothing.Dispatch( hash, good, CComputeDispatchSize::ForItems( 0 ) ) );
        ASSERT_TRUE( Run( device, nothing ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CpuBackend: a prefix sum through workgroup memory, two scans and an offset pass, equals the host scan exactly" )
    try
    {
        TComputeDevicePtr device = CreateCpuTestDevice();
        const UInt32 groups = 200;
        const UInt32 count = groups * CCpuTestKernels::GroupSize - 17;
        TComputeKernelPtr scan;
        TComputeKernelPtr addOffsets;
        ASSERT_TRUE( device->CreateKernel( CCpuTestKernels::ScanGroupsKernelId, scan ) );
        ASSERT_TRUE( device->CreateKernel( CCpuTestKernels::AddGroupOffsetsKernelId, addOffsets ) );

        GUCEF::vector< UInt32 > input( count, 0 );
        UInt32 seed = 777;
        for ( UInt32 i=0; i<count; ++i )
            input[ i ] = NextRandom( seed ) % 1000;

        TComputeBufferPtr in = MakeBuffer( device, count * 4 );
        TComputeBufferPtr scanned = MakeBuffer( device, count * 4 );
        TComputeBufferPtr totals = MakeBuffer( device, groups * 4 );
        TComputeBufferPtr scannedTotals = MakeBuffer( device, groups * 4 );
        TComputeBufferPtr unusedTotals = MakeBuffer( device, 4 );

        CComputeArguments first;
        first.SetBuffer( 0, in );
        first.SetBuffer( 1, scanned );
        first.SetBuffer( 2, totals );
        CComputeArguments second;
        second.SetBuffer( 0, totals );
        second.SetBuffer( 1, scannedTotals );
        second.SetBuffer( 2, unusedTotals );
        CComputeArguments third;
        third.SetBuffer( 0, scanned );
        third.SetBuffer( 1, scannedTotals );

        GUCEF::vector< UInt32 > result( count, 0 );
        CComputeCommandList list;
        list.WriteFromHost( in, 0, &input[ 0 ], count * 4 );
        list.Dispatch( scan, first, CComputeDispatchSize::ForItems( count ) );
        list.Dispatch( scan, second, CComputeDispatchSize::ForItems( groups ) );
        list.Dispatch( addOffsets, third, CComputeDispatchSize::ForItems( count ) );
        list.ReadToHost( scanned, 0, &result[ 0 ], count * 4 );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == list.GetRecordingResult() );
        ASSERT_TRUE( Run( device, list ) );

        bool ok = true;
        UInt32 sum = 0;
        for ( UInt32 i=0; i<count; ++i )
        {
            sum += input[ i ];
            ok = ok && sum == result[ i ];
        }
        ASSERT_TRUE( ok );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CpuBackend: fences, submit, wait idle and the last submitted fence" )
    try
    {
        TComputeDevicePtr device = CreateCpuTestDevice();
        TComputeQueuePtr queue = device->GetDefaultQueue();
        ASSERT_TRUE( 0 == queue->GetQueueIndex() );
        ASSERT_TRUE( queue->GetDevice() == device.GetPointerAlways() );
        ASSERT_TRUE( 0 == queue->GetLastSubmittedFence().GetValue() );
        ASSERT_TRUE( queue->GetLastSubmittedFence().IsSignalled() );
        ASSERT_TRUE( CComputeFence().IsSignalled() );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == CComputeFence().Wait( 0 ) );
        ASSERT_FALSE( CComputeFence().HasQueue() );

        TComputeBufferPtr a = MakeBuffer( device, 16 );
        UInt32 value = 0;
        CComputeCommandList list;
        list.Fill( a, 0, 16, 5 );
        list.ReadToHost( a, 4, &value, 4 );
        CComputeFence fence;
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == queue->Submit( list, fence ) );
        ASSERT_TRUE( fence.HasQueue() );
        ASSERT_TRUE( 1 == fence.GetValue() );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == fence.Wait() );
        ASSERT_TRUE( fence.IsSignalled() );
        ASSERT_TRUE( 5 == value );
        ASSERT_TRUE( 1 == queue->GetLastSubmittedFence().GetValue() );

        CComputeFence fences[ 10 ];
        for ( UInt32 i=0; i<10; ++i )
            ASSERT_TRUE( COMPUTERESULT_SUCCESS == queue->Submit( list, fences[ i ] ) );
        ASSERT_TRUE( 11 == fences[ 9 ].GetValue() );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == queue->WaitIdle() );
        bool all = true;
        for ( UInt32 i=0; i<10; ++i )
            all = all && fences[ i ].IsSignalled();
        ASSERT_TRUE( all );
        ASSERT_TRUE( queue->GetCompletedValue() >= 11 );

        // a synchronous call takes a value of the timeline too
        ASSERT_TRUE( Run( device, list ) );
        ASSERT_TRUE( 12 == queue->GetLastSubmittedFence().GetValue() );
        ASSERT_TRUE( 12 == queue->GetCompletedValue() );

        // waiting with a timeout on work that waits at the gate
        TComputeBufferPtr flag = MakeBuffer( device, 4 );
        CComputeCommandList gate;
        ASSERT_TRUE( RecordGate( gate, device, flag ) );
        CloseGate();
        CComputeFence gated;
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == queue->Submit( gate, gated ) );
        ASSERT_TRUE( WaitUntilGateEntered( 10000 ) );
        ASSERT_TRUE( COMPUTERESULT_TIMEOUT == gated.Wait( 20 ) );
        ASSERT_FALSE( gated.IsSignalled() );
        ASSERT_TRUE( COMPUTERESULT_TIMEOUT == queue->WaitIdle( 5 ) );
        OpenGate();
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == gated.Wait( 10000 ) );
    }
    catch( ... )
    {
        OpenGate();
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CpuBackend: asynchronous execution completes every future once, in order with synchronous work" )
    try
    {
        TComputeDevicePtr device = CreateCpuTestDevice();
        TComputeQueuePtr queue = device->GetDefaultQueue();
        TComputeBufferPtr counter = MakeBuffer( device, 4 );
        TComputeKernelPtr fill;
        ASSERT_TRUE( device->CreateKernel( CCpuTestKernels::FillKernelId, fill ) );

        // 100 lists that each write their index; in order, so the last one wins and each read sees its own write
        const UInt32 lists = 100;
        UInt32 seen[ lists ];
        GUCEF::vector< CORE::CFutureResult > futures;
        for ( UInt32 i=0; i<lists; ++i )
        {
            seen[ i ] = 0xFFFFFFFFu;
            CComputeCommandList list;
            CCpuTestKernels::SFillConstants constants = { 0, 0, i, 0 };
            CComputeArguments args;
            args.SetBuffer( 0, counter );
            args.SetConstants( &constants, sizeof( constants ) );
            list.Dispatch( fill, args, CComputeDispatchSize::ForItems( 1 ) );
            list.ReadToHost( counter, 0, &seen[ i ], 4 );
            futures.push_back( queue->ExecuteAsync( list ) );
        }

        // synchronous work after them sees the result of all of them
        UInt32 last = 0;
        CComputeCommandList read;
        read.ReadToHost( counter, 0, &last, 4 );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == queue->Execute( read ) );
        ASSERT_TRUE( lists - 1 == last );

        bool allOk = true;
        for ( UInt32 i=0; i<lists; ++i )
        {
            allOk = allOk && AwaitFuture( futures[ i ], 10000 );
            allOk = allOk && COMPUTERESULT_SUCCESS == ComputeResultFromFuture( futures[ i ] );
            allOk = allOk && i == seen[ i ];
            allOk = allOk && CORE::TASKSTATUS_FINISHED == futures[ i ].GetResult( 0 )->GetTaskStatus();
        }
        ASSERT_TRUE( allOk );
        ASSERT_TRUE( 0 == queue->GetPendingWorkCount() );

        // a list that does not record well fails its future with the result in the extra info
        CComputeCommandList bad;
        bad.Fill( counter, 1, 4, 0 );
        CORE::CFutureResult failed = queue->ExecuteAsync( bad );
        ASSERT_TRUE( failed.IsReady() );
        CString errorText;
        ASSERT_TRUE( COMPUTERESULT_INVALID_ARGUMENT == ComputeResultFromFuture( failed, &errorText ) );
        ASSERT_TRUE( failed.HasNoFuture() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CpuBackend: work in flight keeps its buffers alive and refuses maps of them (deferred release)" )
    try
    {
        TComputeDevicePtr device = CreateCpuTestDevice();
        TComputeQueuePtr queue = device->GetDefaultQueue();
        TComputeBufferPtr flag = MakeBuffer( device, 4 );
        CComputeCommandList gate;
        ASSERT_TRUE( RecordGate( gate, device, flag ) );

        CloseGate();
        CORE::CFutureResult gated = queue->ExecuteAsync( gate );
        ASSERT_TRUE( WaitUntilGateEntered( 10000 ) );

        UInt32 values[ 4 ] = { 0, 0, 0, 0 };
        GUCEF::vector< CORE::CFutureResult > released;
        {
            TComputeBufferPtr temporary = MakeBuffer( device, 16 );
            CComputeCommandList list;
            list.Fill( temporary, 0, 16, 9 );
            list.ReadToHost( temporary, 0, values, 16 );
            released.push_back( queue->ExecuteAsync( list ) );

            // the gated work and this work have not run: the buffer is in use
            void* data = GUCEF_NULL;
            ASSERT_TRUE( temporary->IsInUseByWorkInFlight() );
            ASSERT_TRUE( COMPUTERESULT_INVALID_ARGUMENT == temporary->Map( MAPACCESS_READ, data ) );
            ASSERT_TRUE( flag->IsInUseByWorkInFlight() );
        }
        // the only references to the buffer are now those of the pending work
        OpenGate();
        ASSERT_TRUE( AwaitFuture( gated, 10000 ) );
        ASSERT_TRUE( 1 == released.size() && AwaitFuture( released[ 0 ], 10000 ) );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == ComputeResultFromFuture( released[ 0 ] ) );
        ASSERT_TRUE( 9 == values[ 0 ] && 9 == values[ 3 ] );

        ASSERT_FALSE( flag->IsInUseByWorkInFlight() );
        CComputeBufferMapScope map( flag, MAPACCESS_READ );
        ASSERT_TRUE( map.IsMapped() );
        ASSERT_TRUE( 1 == map.GetDataAs< UInt32 >()[ 0 ] );
    }
    catch( ... )
    {
        OpenGate();
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CpuBackend: a queue waits for the fence of another queue of the device" )
    try
    {
        TComputeDevicePtr device = CreateCpuTestDevice( 2 );
        ASSERT_TRUE( 2 == device->GetQueueCount() );
        TComputeQueuePtr first = device->GetQueue( 0 );
        TComputeQueuePtr second = device->GetQueue( 1 );
        TComputeBufferPtr flag = MakeBuffer( device, 4 );

        CComputeCommandList gate;
        ASSERT_TRUE( RecordGate( gate, device, flag ) );
        CloseGate();
        CComputeFence gated;
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == first->Submit( gate, gated ) );
        ASSERT_TRUE( WaitUntilGateEntered( 10000 ) );

        UInt32 seen = 0;
        CComputeCommandList after;
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == after.WaitFor( gated ) );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == after.ReadToHost( flag, 0, &seen, 4 ) );
        CORE::CFutureResult future = second->ExecuteAsync( after );
        MT::PrecisionDelay( 20 );
        ASSERT_FALSE( future.IsReady() );
        OpenGate();
        ASSERT_TRUE( AwaitFuture( future, 10000 ) );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == ComputeResultFromFuture( future ) );
        ASSERT_TRUE( 1 == seen );

        // a fence without a queue is no command
        CComputeCommandList nothing;
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == nothing.WaitFor( CComputeFence() ) );
        ASSERT_TRUE( nothing.IsEmpty() );
    }
    catch( ... )
    {
        OpenGate();
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CpuBackend: the host batch helper runs inputs, outputs, in-place and resident bindings without growing" )
    try
    {
        TComputeDevicePtr device = CreateCpuTestDevice();
        CComputeHostBatch batch( device, 2 );
        ASSERT_TRUE( 2 == batch.GetRingSize() );
        ASSERT_TRUE( batch.GetDevice().GetPointerAlways() == device.GetPointerAlways() );
        ASSERT_TRUE( 0 == batch.GetStagingSizeInBytes() );

        TComputeKernelPtr hash;
        TComputeKernelPtr saxpy;
        ASSERT_TRUE( device->CreateKernel( CCpuTestKernels::HashKernelId, hash ) );
        ASSERT_TRUE( device->CreateKernel( CCpuTestKernels::SaxpyKernelId, saxpy ) );

        const UInt32 count = 5000;
        GUCEF::vector< UInt32 > input( count, 0 );
        GUCEF::vector< UInt32 > output( count, 0 );
        UInt32 seed = 99;
        for ( UInt32 i=0; i<count; ++i )
            input[ i ] = NextRandom( seed );
        CCpuTestKernels::SHashConstants constants = { 0, 0, 0x1234u, 0 };
        CComputeHostBatch::SHostBinding bindings[ 2 ] =
        {
            CComputeHostBatch::SHostBinding::Input( 0, &input[ 0 ], count * 4 ),
            CComputeHostBatch::SHostBinding::Output( 1, &output[ 0 ], count * 4 )
        };
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == batch.Run( hash, bindings, 2, &constants, sizeof( constants ), CComputeDispatchSize::ForItems( count ) ) );
        bool ok = true;
        for ( UInt32 i=0; i<count; ++i )
            ok = ok && output[ i ] == CCpuTestKernels::Hash( input[ i ] ^ 0x1234u );
        ASSERT_TRUE( ok );

        // repeated calls of the same sizes reuse the staging buffers
        const UInt64 staging = batch.GetStagingSizeInBytes();
        ASSERT_TRUE( staging > 0 );
        for ( UInt32 r=0; r<10; ++r )
            ASSERT_TRUE( COMPUTERESULT_SUCCESS == batch.Run( hash, bindings, 2, &constants, sizeof( constants ), CComputeDispatchSize::ForItems( count ) ) );
        ASSERT_TRUE( staging == batch.GetStagingSizeInBytes() );

        // in place: y = a * x + y with y in and out of the same host memory
        GUCEF::vector< Float32 > x( count, 2.0f );
        GUCEF::vector< Float32 > y( count, 1.0f );
        CCpuTestKernels::SSaxpyConstants saxpyConstants = { 0, 0, 3.0f, 0 };
        CComputeHostBatch::SHostBinding inPlace[ 2 ] =
        {
            CComputeHostBatch::SHostBinding::Input( 0, &x[ 0 ], count * 4 ),
            CComputeHostBatch::SHostBinding::InOut( 1, &y[ 0 ], count * 4 )
        };
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == batch.Run( saxpy, inPlace, 2, &saxpyConstants, sizeof( saxpyConstants ), CComputeDispatchSize::ForItems( count ) ) );
        ok = true;
        for ( UInt32 i=0; i<count; ++i )
            ok = ok && 7.0f == y[ i ];
        ASSERT_TRUE( ok );

        // asynchronous, with a resident buffer as input
        TComputeBufferPtr resident = MakeBuffer( device, count * 4 );
        CComputeCommandList upload;
        upload.WriteFromHost( resident, 0, &input[ 0 ], count * 4 );
        ASSERT_TRUE( Run( device, upload ) );
        GUCEF::vector< UInt32 > asyncOutput( count, 0 );
        CComputeHostBatch::SHostBinding residentBindings[ 2 ] =
        {
            CComputeHostBatch::SHostBinding::Resident( 0, resident ),
            CComputeHostBatch::SHostBinding::Output( 1, &asyncOutput[ 0 ], count * 4 )
        };
        CORE::CFutureResult future = batch.RunAsync( hash, residentBindings, 2, &constants, sizeof( constants ), CComputeDispatchSize::ForItems( count ) );
        ASSERT_TRUE( AwaitFuture( future, 10000 ) );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == ComputeResultFromFuture( future ) );
        ASSERT_TRUE( output == asyncOutput );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == batch.WaitIdle() );

        // more asynchronous calls than slots wait for the oldest slot
        GUCEF::vector< CORE::CFutureResult > futures;
        GUCEF::vector< UInt32 > outputs( count * 6, 0 );
        for ( UInt32 r=0; r<6; ++r )
        {
            CComputeHostBatch::SHostBinding ring[ 2 ] =
            {
                CComputeHostBatch::SHostBinding::Input( 0, &input[ 0 ], count * 4 ),
                CComputeHostBatch::SHostBinding::Output( 1, &outputs[ r * count ], count * 4 )
            };
            futures.push_back( batch.RunAsync( hash, ring, 2, &constants, sizeof( constants ), CComputeDispatchSize::ForItems( count ) ) );
        }
        ok = true;
        for ( UInt32 r=0; r<6; ++r )
        {
            ok = ok && AwaitFuture( futures[ r ], 10000 ) && COMPUTERESULT_SUCCESS == ComputeResultFromFuture( futures[ r ] );
            ok = ok && 0 == memcmp( &outputs[ r * count ], &output[ 0 ], count * 4 );
        }
        ASSERT_TRUE( ok );

        // a missing binding fails, synchronously and asynchronously
        ASSERT_TRUE( COMPUTERESULT_INVALID_ARGUMENT == batch.Run( hash, bindings, 1, &constants, sizeof( constants ), CComputeDispatchSize::ForItems( count ) ) );
        CORE::CFutureResult failed = batch.RunAsync( hash, bindings, 1, &constants, sizeof( constants ), CComputeDispatchSize::ForItems( count ) );
        ASSERT_TRUE( COMPUTERESULT_INVALID_ARGUMENT == ComputeResultFromFuture( failed ) );
        TComputeKernelPtr none;
        ASSERT_TRUE( COMPUTERESULT_INVALID_ARGUMENT == batch.Run( none, bindings, 2, &constants, sizeof( constants ), CComputeDispatchSize::ForItems( count ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CpuBackend: a buffer still used on one queue cannot be mapped after a later use on another queue" )
    try
    {
        TComputeDevicePtr device = CreateCpuTestDevice( 2 );
        TComputeQueuePtr first = device->GetQueue( 0 );
        TComputeQueuePtr second = device->GetQueue( 1 );
        TComputeBufferPtr buffer = MakeBuffer( device, 16 );
        CComputeCommandList gate;
        ASSERT_TRUE( RecordGate( gate, device, buffer ) );
        UInt32 seven = 7;
        CComputeCommandList write;
        write.WriteFromHost( buffer, 8, &seven, 4 );

        CloseGate();
        CORE::CFutureResult gated = first->ExecuteAsync( gate );
        ASSERT_TRUE( WaitUntilGateEntered( 10000 ) );
        void* data = GUCEF_NULL;
        ASSERT_TRUE( COMPUTERESULT_INVALID_ARGUMENT == buffer->Map( MAPACCESS_READ_WRITE, data ) );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == second->Execute( write ) );
        // the later use on the second queue completed, the gate on the first still writes the buffer
        ASSERT_TRUE( COMPUTERESULT_INVALID_ARGUMENT == buffer->Map( MAPACCESS_READ_WRITE, data ) );
        ASSERT_TRUE( buffer->IsInUseByWorkInFlight() );
        OpenGate();
        ASSERT_TRUE( AwaitFuture( gated, 10000 ) );
        CComputeBufferMapScope map( buffer, MAPACCESS_READ );
        ASSERT_TRUE( map.IsMapped() );
        ASSERT_TRUE( 1 == map.GetDataAs< UInt32 >()[ 0 ] && 7 == map.GetDataAs< UInt32 >()[ 2 ] );
        // while mapped, no queue takes the buffer
        ASSERT_TRUE( COMPUTERESULT_INVALID_ARGUMENT == second->Execute( write ) );
        ASSERT_TRUE( COMPUTERESULT_INVALID_ARGUMENT == ComputeResultFromFuture( first->ExecuteAsync( write ) ) );
    }
    catch( ... )
    {
        OpenGate();
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CpuBackend: a kernel keeps its device, the device caches kernels weakly, a new device refuses an old kernel" )
    try
    {
        // the devices of earlier tests are all gone (none leaks)
        ASSERT_TRUE( WaitForDeviceCount( 0, 5000 ) );
        const UInt32 devicesBefore = 0;
        TComputeKernelPtr kernel;
        {
            TComputeDevicePtr first = CreateCpuTestDevice();
            ASSERT_TRUE( first->CreateKernel( CCpuTestKernels::FillKernelId, kernel ) );
            ASSERT_TRUE( 1 == first->GetKernelCacheSize() );
        }
        // the device lives on in the kernel
        ASSERT_TRUE( WaitForDeviceCount( devicesBefore + 1, 5000 ) );
        ASSERT_TRUE( GUCEF_NULL != kernel->GetDevice() );
        ASSERT_FALSE( kernel->GetDevice()->IsLost() );
        ASSERT_TRUE( 1 == kernel->GetDevice()->GetKernelCacheSize() );

        TComputeDevicePtr second = CreateCpuTestDevice();
        ASSERT_TRUE( second.GetPointerAlways() != kernel->GetDevice() );
        TComputeBufferPtr buffer = MakeBuffer( second, 16 );
        CCpuTestKernels::SFillConstants constants = { 0, 0, 1, 1 };
        CComputeArguments args;
        args.SetBuffer( 0, buffer );
        args.SetConstants( &constants, sizeof( constants ) );
        CComputeCommandList list;
        ASSERT_TRUE( COMPUTERESULT_INVALID_ARGUMENT == list.Dispatch( kernel, args, CComputeDispatchSize::ForItems( 4 ) ) );

        // the kernel goes, then its device; the cache never kept it
        CComputeDevice* old = kernel->GetDevice();
        TComputeDevicePtr oldPtr( old->CreateSharedPtr() );
        kernel.Unlink();
        ASSERT_TRUE( 0 == oldPtr->GetKernelCacheSize() );
        oldPtr.Unlink();
        ASSERT_TRUE( WaitForDeviceCount( devicesBefore + 1, 5000 ) );
        args.Clear();           // the arguments hold the buffer, the buffer holds the device
        second.Unlink();
        buffer.Unlink();
        ASSERT_TRUE( WaitForDeviceCount( devicesBefore, 5000 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CpuBackend: calling-thread-only execution gives the same results as fork-join" )
    try
    {
        TComputeDevicePtr device = CreateCpuTestDevice();
        TComputeQueuePtr queue = device->GetDefaultQueue();
        ASSERT_TRUE( EXECUTIONPOLICY_DEFAULT == queue->GetExecutionPolicy() );
        TComputeKernelPtr hash;
        ASSERT_TRUE( device->CreateKernel( CCpuTestKernels::HashKernelId, hash ) );
        const UInt32 count = 300000;
        GUCEF::vector< UInt32 > input( count, 0 );
        GUCEF::vector< UInt32 > forkJoin( count, 0 );
        GUCEF::vector< UInt32 > single( count, 1 );
        UInt32 seed = 4242;
        for ( UInt32 i=0; i<count; ++i )
            input[ i ] = NextRandom( seed );
        TComputeBufferPtr in = MakeBuffer( device, count * 4 );
        TComputeBufferPtr out = MakeBuffer( device, count * 4 );
        CCpuTestKernels::SHashConstants constants = { 0, 0, 77, 0 };
        CComputeArguments args;
        args.SetBuffer( 0, in );
        args.SetBuffer( 1, out );
        args.SetConstants( &constants, sizeof( constants ) );
        CComputeCommandList list;
        list.WriteFromHost( in, 0, &input[ 0 ], count * 4 );
        list.Dispatch( hash, args, CComputeDispatchSize::ForItems( count ) );
        list.ReadToHost( out, 0, &forkJoin[ 0 ], count * 4 );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == queue->Execute( list ) );

        CComputeCommandList again;
        again.Dispatch( hash, args, CComputeDispatchSize::ForItems( count ) );
        again.ReadToHost( out, 0, &single[ 0 ], count * 4 );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == queue->Execute( again, EXECUTIONPOLICY_CALLING_THREAD_ONLY ) );
        ASSERT_TRUE( forkJoin == single );

        queue->SetExecutionPolicy( EXECUTIONPOLICY_CALLING_THREAD_ONLY );
        ASSERT_TRUE( EXECUTIONPOLICY_CALLING_THREAD_ONLY == queue->GetExecutionPolicy() );
        single.assign( count, 2 );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == queue->Execute( again ) );
        ASSERT_TRUE( forkJoin == single );
        queue->SetExecutionPolicy( EXECUTIONPOLICY_DEFAULT );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
