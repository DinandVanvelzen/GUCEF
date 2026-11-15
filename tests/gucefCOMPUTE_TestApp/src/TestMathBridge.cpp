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

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_MATH_CMATHGLOBAL_H
#include "gucefMATH_CMathGlobal.h"
#define GUCEF_MATH_CMATHGLOBAL_H
#endif /* GUCEF_MATH_CMATHGLOBAL_H ? */

#ifndef GUCEF_MATH_BATCHFUNCTIONS_H
#include "gucefMATH_BatchFunctions.h"
#define GUCEF_MATH_BATCHFUNCTIONS_H
#endif /* GUCEF_MATH_BATCHFUNCTIONS_H ? */

#ifndef GUCEF_COMPUTE_H
#include "gucefCOMPUTE.h"
#define GUCEF_COMPUTE_H
#endif /* GUCEF_COMPUTE_H ? */

#include "TestComputeSupport.h"
#include "TestMathBridgeHarness.h"
#include "TestMathBridge.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

// an assertion that the rest of the test case cannot do without: records it, then leaves the test case
struct SRequirementFailed {};
#define REQUIRE_TRUE(t) do { if ( !(t) ) { ASSERT_TRUE(t); throw SRequirementFailed(); } } while (0)

using namespace GUCEF;
using namespace GUCEF::COMPUTE;
using namespace GUCEF::COMPUTE::TEST;

/*-------------------------------------------------------------------------*/

namespace {

// the last count is the large one: 10^5, GUCEF_TEST_COMPUTE_MATH_LARGE replaces it (the repeated runs of the sanitizer builds use a smaller one)
UInt32 g_counts[] = { 0, 1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 1023, 100000 };
const UInt32 g_countCount = sizeof( g_counts ) / sizeof( g_counts[ 0 ] );

UInt32
LargeCount( void )
{
    return g_counts[ g_countCount - 1 ];
}

const UInt32 MaxKernels = 64;

MATH::CMathBatchDispatcher&
Dispatcher( void )
{
    return MATH::CMathGlobal::Instance()->GetBatchDispatcher();
}

/*-------------------------------------------------------------------------*/

/** The layout of the arrays of one call: strides in bytes, the offset of every array from its allocation, in place or not */
struct SLayout
{
    UInt32 stride[ 3 ];     // first input, second input, output; 0 for the packed element size
    UInt32 misalignBytes;
    bool inPlace;
    const char* name;
};

/*-------------------------------------------------------------------------*/

CString
Describe( const SMathKernelCase& kc, UInt32 count, const SLayout& layout, UInt32 grain, UInt32 seed )
{
    CString text = Dispatcher().GetKernelId( kc.kernel );
    text += " count=" + CORE::UInt32ToString( count ) + " layout=" + layout.name + " grain=" + CORE::UInt32ToString( grain ) + " seed=" + CORE::UInt32ToString( seed );
    return text;
}

/*-------------------------------------------------------------------------*/

/**
 *  Runs one call twice, once with the flags of the candidate and once forced to the scalar provider, on equal inputs,
 *  and compares everything the call may touch byte by byte: the arrays and the bytes between strided elements. In place
 *  both run on copies of the same array.
 */
bool
CompareWithScalar( const SMathKernelCase& kc, UInt32 count, const SLayout& layout, UInt32 seed, UInt32 matrixIndex, UInt32 candidateFlags, CString& failure )
{
    const UInt32 packedIn0 = kc.in0Bytes;
    const UInt32 packedIn1 = kc.in1Bytes;
    const UInt32 packedOut = kc.outBytes;
    const UInt32 in0Stride = 0 != layout.stride[ 0 ] ? layout.stride[ 0 ] : packedIn0;
    const UInt32 in1Stride = 0 != layout.stride[ 1 ] ? layout.stride[ 1 ] : packedIn1;
    const UInt32 outStride = 0 != layout.stride[ 2 ] ? layout.stride[ 2 ] : packedOut;
    const UInt32 elements = count > 0 ? count : 1;
    const MATH::TFloat32Matrix4D matrix = MakeTestMatrix( matrixIndex );

    // the first input also holds the output of an in place conversion, which is the wider of the two
    const bool convertInPlace = layout.inPlace && MATHCASESHAPE_CONVERT == kc.shape;
    const UInt32 in0Span = convertInPlace ? ( kc.in0Bytes > kc.outBytes ? kc.in0Bytes : kc.outBytes ) : in0Stride;

    CTestBuffer in0;
    CTestBuffer in1;
    in0.Resize( static_cast< UInt64 >( elements ) * in0Span, layout.misalignBytes, 0xA5 );
    if ( 0 != kc.in0Bytes )
        kc.fill( 0, in0.Data(), count, convertInPlace ? kc.in0Bytes : in0Stride, seed );
    if ( 0 != kc.in1Bytes )
    {
        in1.Resize( static_cast< UInt64 >( elements ) * in1Stride, layout.misalignBytes, 0xA5 );
        kc.fill( 1, in1.Data(), count, in1Stride, seed + 1 );
    }

    CTestBuffer refOut;
    CTestBuffer candOut;
    CTestBuffer refInPlace;
    CTestBuffer candInPlace;
    SMathCall refCall;
    SMathCall candCall;
    refCall.count = candCall.count = count;
    refCall.matrix = candCall.matrix = &matrix;
    refCall.in0Stride = candCall.in0Stride = convertInPlace ? 0 : in0Stride;
    refCall.in1Stride = candCall.in1Stride = in1Stride;
    refCall.outStride = candCall.outStride = outStride;
    refCall.in1 = candCall.in1 = in1.Data();

    const UInt8* refResult = GUCEF_NULL;
    const UInt8* candResult = GUCEF_NULL;
    UInt64 compareBytes = 0;
    if ( layout.inPlace )
    {
        refInPlace.Resize( in0.Size(), layout.misalignBytes, 0xA5 );
        candInPlace.Resize( in0.Size(), layout.misalignBytes, 0xA5 );
        memcpy( refInPlace.Data(), in0.Data(), static_cast< size_t >( in0.Size() ) );
        memcpy( candInPlace.Data(), in0.Data(), static_cast< size_t >( in0.Size() ) );
        refCall.in0 = refCall.out = refInPlace.Data();
        candCall.in0 = candCall.out = candInPlace.Data();
        refResult = refInPlace.Data();
        candResult = candInPlace.Data();
        compareBytes = in0.Size();
        // the halves of an in place conversion to Float16 fill the front of the array, the rest still holds floats
        if ( convertInPlace && kc.outBytes < kc.in0Bytes )
            compareBytes = static_cast< UInt64 >( count ) * kc.outBytes;
    }
    else
    {
        const UInt64 outSize = static_cast< UInt64 >( elements ) * outStride;
        refOut.Resize( outSize, layout.misalignBytes, 0xA5 );
        candOut.Resize( outSize, layout.misalignBytes, 0xA5 );
        refCall.in0 = candCall.in0 = in0.Data();
        refCall.out = refOut.Data();
        candCall.out = candOut.Data();
        refResult = refOut.Data();
        candResult = candOut.Data();
        compareBytes = outSize;
    }

    const MATH::TMathStatus refStatus = kc.run( Dispatcher(), MATH::MATHBATCH_FORCE_SCALAR, refCall );
    const MATH::TMathStatus candStatus = kc.run( Dispatcher(), candidateFlags, candCall );
    if ( MATH::MATHSTATUS_OK != refStatus || MATH::MATHSTATUS_OK != candStatus )
    {
        failure = "status of the call: scalar " + CORE::UInt32ToString( refStatus ) + ", candidate " + CORE::UInt32ToString( candStatus );
        return false;
    }

    if ( 0 != memcmp( refResult, candResult, static_cast< size_t >( compareBytes ) ) )
    {
        UInt64 first = 0;
        while ( first < compareBytes && refResult[ first ] == candResult[ first ] )
            ++first;
        failure = "first difference at byte " + CORE::UInt64ToString( first );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** The layouts a kernel is tested with: packed, misaligned by 4 bytes, in place, and with the strides of the design */
UInt32
MakeLayouts( const SMathKernelCase& kc, SLayout* layouts )
{
    UInt32 count = 0;
    SLayout packed = { { 0, 0, 0 }, 0, false, "packed" };
    layouts[ count++ ] = packed;
    SLayout misaligned = { { 0, 0, 0 }, 4, false, "misaligned4" };
    layouts[ count++ ] = misaligned;

    if ( MATHCASESHAPE_GENERATE == kc.shape )
        return count;

    if ( MATHCASESHAPE_CONVERT == kc.shape )
    {
        SLayout inPlace = { { 0, 0, 0 }, 0, true, "inplace" };
        layouts[ count++ ] = inPlace;
        SLayout inPlaceMisaligned = { { 0, 0, 0 }, 4, true, "inplace-misaligned4" };
        layouts[ count++ ] = inPlaceMisaligned;
        return count;
    }

    if ( kc.in0Bytes == kc.outBytes )
    {
        SLayout inPlace = { { 0, 0, 0 }, 0, true, "inplace" };
        layouts[ count++ ] = inPlace;
        SLayout inPlaceMisaligned = { { 0, 0, 0 }, 4, true, "inplace-misaligned4" };
        layouts[ count++ ] = inPlaceMisaligned;
        // in place with a stride: the same array is input and output, the other arrays are packed
        SLayout inPlaceStrided = { { kc.in0Bytes + 4, 0, kc.in0Bytes + 4 }, 0, true, "inplace-strided" };
        layouts[ count++ ] = inPlaceStrided;
    }

    // the strides of the design: 12, 16 and 32 bytes for the 12 byte elements; for the others the packed size, and larger ones
    static const UInt32 extra[ 3 ] = { 16, 32, 80 };
    for ( UInt32 e=0; e<3; ++e )
    {
        const UInt32 stride = extra[ e ];
        if ( stride < kc.in0Bytes || stride < kc.in1Bytes || stride < kc.outBytes )
            continue;
        static const char* names[ 3 ] = { "strided16", "strided32", "strided80" };
        SLayout strided = { { stride, 0 != kc.in1Bytes ? stride : 0, stride }, 0, false, names[ e ] };
        layouts[ count++ ] = strided;
    }
    // different strides for the arrays of one call, odd ones included: the strides need not be multiples of 4
    SLayout mixed = { { kc.in0Bytes + 6, kc.in1Bytes + 2, kc.outBytes + 10 }, 4, false, "strided-mixed" };
    if ( 0 == kc.in1Bytes )
        mixed.stride[ 1 ] = 0;
    layouts[ count++ ] = mixed;
    return count;
}

/*-------------------------------------------------------------------------*/

/** Calls that reached the MULTICORE provider of the kernel so far */
UInt32
MultiCoreCalls( MATH::TMathBatchKernel kernel )
{
    const COMPUTEBRIDGEMATH::SMathBridgeProviderContext* context = FindMathProviderContext( kernel, MATH::MATHBATCHTIER_MULTICORE );
    return GUCEF_NULL != context ? context->diagnostics->multiCoreCalls[ kernel ].Load() : 0;
}

UInt32
MultiCoreFailures( MATH::TMathBatchKernel kernel )
{
    const COMPUTEBRIDGEMATH::SMathBridgeProviderContext* context = FindMathProviderContext( kernel, MATH::MATHBATCHTIER_MULTICORE );
    return GUCEF_NULL != context ? context->diagnostics->multiCoreFailures[ kernel ].Load() : 0;
}

UInt32
AcceleratorCalls( MATH::TMathBatchKernel kernel )
{
    const COMPUTEBRIDGEMATH::SMathBridgeProviderContext* context = FindMathProviderContext( kernel, MATH::MATHBATCHTIER_ACCELERATOR );
    return GUCEF_NULL != context ? context->diagnostics->acceleratorCalls[ kernel ].Load() : 0;
}

UInt32
AcceleratorFailures( MATH::TMathBatchKernel kernel )
{
    const COMPUTEBRIDGEMATH::SMathBridgeProviderContext* context = FindMathProviderContext( kernel, MATH::MATHBATCHTIER_ACCELERATOR );
    return GUCEF_NULL != context ? context->diagnostics->acceleratorFailures[ kernel ].Load() : 0;
}

/** The diagnostics of the loaded plugin through any provider it registered */
COMPUTEBRIDGEMATH::SMathBridgeDiagnostics*
GetDiagnostics( void )
{
    MATH::TMathBatchKernel kernels[ MaxKernels ];
    const UInt32 count = GetMathMultiCoreKernels( kernels, MaxKernels );
    if ( 0 == count )
        return GUCEF_NULL;
    return FindMathProviderContext( kernels[ 0 ], MATH::MATHBATCHTIER_MULTICORE )->diagnostics;
}

/*-------------------------------------------------------------------------*/

/** Unloads, applies the settings, loads: the plugin registers its providers with the settings in effect at the load */
bool
ReloadBridge( TComputeProviderTier forcedTier, UInt32 minCountMultiCore = 0, UInt32 disabledTiers = 0 )
{
    UnloadMathBridgePlugin();
    SetMathBridgeSettings( forcedTier, minCountMultiCore, disabledTiers );
    return LoadMathBridgePlugin();
}

/*-------------------------------------------------------------------------*/

/** Every layout of every count of one kernel, the candidate flags as given: true when every call equals the scalar one */
bool
CompareKernel( const SMathKernelCase& kc, UInt32 candidateFlags, bool requireBridgeCalls, CString& failure )
{
    SLayout layouts[ 16 ];
    const UInt32 layoutCount = MakeLayouts( kc, layouts );
    COMPUTEBRIDGEMATH::SMathBridgeDiagnostics* diagnostics = GetDiagnostics();
    if ( GUCEF_NULL == diagnostics )
    {
        failure = "the bridge is not loaded";
        return false;
    }

    for ( UInt32 c=0; c<g_countCount; ++c )
    {
        const UInt32 count = g_counts[ c ];
        // small chunks put the boundaries of the chunks at odd places; the large counts are also run with the grain of the kernel
        UInt32 grains[ 3 ] = { 3, 0, 1001 };
        UInt32 grainCount = 2;
        if ( 1023 == count )
        {
            grainCount = 3;
        }
        else if ( count > 1023 )
        {
            grains[ 0 ] = 0;
            grains[ 1 ] = 1001;
        }
        for ( UInt32 g=0; g<grainCount; ++g )
        {
            diagnostics->grainOverrideItems.Store( grains[ g ] );
            for ( UInt32 l=0; l<layoutCount; ++l )
            {
                const UInt32 seed = 1000 + c * 31 + l;
                const UInt32 matrices = ( 1023 == count || 17 == count ) ? 2 : 1;
                for ( UInt32 m=0; m<matrices; ++m )
                {
                    const UInt32 before = MultiCoreCalls( kc.kernel );
                    if ( !CompareWithScalar( kc, count, layouts[ l ], seed, m, candidateFlags, failure ) )
                    {
                        // is it the multi-core tier, or already the SIMD tier of gucefMATH below it
                        CString simdFailure;
                        const bool simdEqual = CompareWithScalar( kc, count, layouts[ l ], seed, m, MATH::MATHBATCH_NO_MULTICORE | MATH::MATHBATCH_NO_ACCELERATOR, simdFailure );
                        failure = Describe( kc, count, layouts[ l ], grains[ g ], seed ) + " matrix " + CORE::UInt32ToString( m ) + ": " + failure +
                                  ( simdEqual ? " (the SIMD tier alone is equal to scalar)" : " (the SIMD tier alone differs from scalar too)" );
                        diagnostics->grainOverrideItems.Store( 0 );
                        return false;
                    }
                    if ( requireBridgeCalls && MultiCoreCalls( kc.kernel ) == before )
                    {
                        failure = Describe( kc, count, layouts[ l ], grains[ g ], seed ) + ": the call did not reach the MULTICORE provider";
                        diagnostics->grainOverrideItems.Store( 0 );
                        return false;
                    }
                }
            }
        }
    }
    diagnostics->grainOverrideItems.Store( 0 );
    return true;
}

}

namespace {

/** One call on packed arrays of test data, results not compared: for the cost of the call and for counters */
void
RunPackedCall( const SMathKernelCase& kc, UInt32 count, UInt32 flags, UInt32 seed )
{
    const MATH::TFloat32Matrix4D matrix = MakeTestMatrix( 0 );
    const UInt32 elements = count > 0 ? count : 1;
    CTestBuffer in0;
    CTestBuffer in1;
    CTestBuffer out;
    in0.Resize( static_cast< UInt64 >( elements ) * kc.in0Bytes, 0, 0 );
    if ( 0 != kc.in0Bytes )
        kc.fill( 0, in0.Data(), count, kc.in0Bytes, seed );
    if ( 0 != kc.in1Bytes )
    {
        in1.Resize( static_cast< UInt64 >( elements ) * kc.in1Bytes, 0, 0 );
        kc.fill( 1, in1.Data(), count, kc.in1Bytes, seed + 1 );
    }
    out.Resize( static_cast< UInt64 >( elements ) * kc.outBytes, 0, 0 );
    SMathCall call;
    call.in0 = in0.Data();
    call.in0Stride = kc.in0Bytes;
    call.in1 = in1.Data();
    call.in1Stride = kc.in1Bytes;
    call.out = out.Data();
    call.outStride = kc.outBytes;
    call.count = count;
    call.matrix = &matrix;
    kc.run( Dispatcher(), flags, call );
}

/*-------------------------------------------------------------------------*/

/** The CPU kernel of the kernel through a device and the host batch, against the scalar provider, on packed arrays */
bool
CompareDeviceKernel( TComputeDevicePtr device, CComputeHostBatch& batch, const SMathKernelCase& kc, UInt32 count, UInt32 seed, CString& failure, TComputeKernelPtr heldKernel = TComputeKernelPtr() )
{
    const CString kernelId = Dispatcher().GetKernelId( kc.kernel );
    TComputeKernelPtr kernel = heldKernel;
    if ( kernel.IsNULL() && !device->CreateKernel( kernelId, kernel ) )
    {
        failure = "the device cannot create kernel " + kernelId;
        return false;
    }

    const MATH::TFloat32Matrix4D matrix = MakeTestMatrix( 0 );
    CTestBuffer in0;
    CTestBuffer in1;
    CTestBuffer refOut;
    CTestBuffer devOut;
    in0.Resize( static_cast< UInt64 >( count ) * kc.in0Bytes, 0, 0 );
    if ( 0 != kc.in0Bytes )
        kc.fill( 0, in0.Data(), count, kc.in0Bytes, seed );
    if ( 0 != kc.in1Bytes )
    {
        in1.Resize( static_cast< UInt64 >( count ) * kc.in1Bytes, 0, 0 );
        kc.fill( 1, in1.Data(), count, kc.in1Bytes, seed + 1 );
    }
    refOut.Resize( static_cast< UInt64 >( count ) * kc.outBytes, 0, 0xA5 );
    devOut.Resize( static_cast< UInt64 >( count ) * kc.outBytes, 0, 0xA5 );

    SMathCall call;
    call.in0 = in0.Data();
    call.in0Stride = kc.in0Bytes;
    call.in1 = in1.Data();
    call.in1Stride = kc.in1Bytes;
    call.out = refOut.Data();
    call.outStride = kc.outBytes;
    call.count = count;
    call.matrix = &matrix;
    kc.run( Dispatcher(), MATH::MATHBATCH_FORCE_SCALAR, call );

    // the device gets the same arrays, its output goes to its own
    call.out = devOut.Data();
    SMathDeviceCall deviceCall;
    kc.makeDeviceCall( call, deviceCall );
    const TComputeResult result = batch.Run( kernel, deviceCall.bindings, deviceCall.bindingCount, deviceCall.constants, deviceCall.constantsSize, CComputeDispatchSize::ForItems( count ) );
    if ( COMPUTERESULT_SUCCESS != result )
    {
        failure = "the host batch run failed with result " + CORE::UInt32ToString( result );
        return false;
    }
    if ( 0 != memcmp( refOut.Data(), devOut.Data(), static_cast< size_t >( refOut.Size() ) ) )
    {
        failure = "the CPU kernel differs from the scalar provider";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Calls the batch functions from a thread of its own until told to stop, checking every result against the scalar one */
class CBatchCaller : public CTestThreadBody
{
    public:

    CBatchCaller( UInt32 seed, MT::CTAtomic< UInt32 >& stop, MT::CTAtomic< UInt32 >& mismatches, MT::CTAtomic< UInt32 >& calls )
        : m_seed( seed ), m_stop( stop ), m_mismatches( mismatches ), m_calls( calls )
    {
    }

    virtual void Run( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        try
        {
            const UInt32 count = 6000;
            const MATH::TFloat32Matrix4D matrix = MakeTestMatrix( 0 );
            CTestBuffer in;
            CTestBuffer expected;
            CTestBuffer out;
            in.Resize( count * 12, 0, 0 );
            expected.Resize( count * 12, 0, 0 );
            out.Resize( count * 12, 0, 0 );
            FillInput( in.Data() );
            MATH::TransformPoints( matrix, reinterpret_cast< const MATH::TFloat32Point3D* >( in.Data() ), reinterpret_cast< MATH::TFloat32Point3D* >( expected.Data() ), count, MATH::MATHBATCH_FORCE_SCALAR );

            while ( 0 == m_stop.Load() )
            {
                // every kind of call: default dispatch, the deterministic one and the one that stays on this thread
                const UInt32 flagSets[ 3 ] = { 0, MATH::MATHBATCH_DETERMINISTIC, MATH::MATHBATCH_NO_MULTICORE };
                for ( UInt32 f=0; f<3 && 0 == m_stop.Load(); ++f )
                {
                    memset( out.Data(), 0xA5, count * 12 );
                    MATH::TransformPoints( matrix, reinterpret_cast< const MATH::TFloat32Point3D* >( in.Data() ), reinterpret_cast< MATH::TFloat32Point3D* >( out.Data() ), count, flagSets[ f ] );
                    if ( 0 != memcmp( out.Data(), expected.Data(), count * 12 ) )
                        m_mismatches.FetchAdd( 1 );
                    m_calls.FetchAdd( 1 );
                }
            }
        }
        catch ( ... )
        {
            m_mismatches.FetchAdd( 1000000 );
        }
    }

    private:

    void FillInput( UInt8* data )
    {
        CTestRandom random( m_seed );
        for ( UInt32 i=0; i<6000 * 3; ++i )
        {
            const Float32 value = random.NextFloat( 100.0f );
            memcpy( data + i * sizeof( Float32 ), &value, sizeof( value ) );
        }
    }

    UInt32 m_seed;
    MT::CTAtomic< UInt32 >& m_stop;
    MT::CTAtomic< UInt32 >& m_mismatches;
    MT::CTAtomic< UInt32 >& m_calls;
};

/*-------------------------------------------------------------------------*/

void
PumpMainPulseGenerator( UInt32 times )
{
    for ( UInt32 i=0; i<times; ++i )
    {
        CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        MT::PrecisionDelay( 1 );
    }
}

bool
HasProvider( MATH::TMathBatchKernel kernel, MATH::TMathBatchTier tier )
{
    MATH::SMathBatchProvider provider;
    return FindMathProvider( kernel, tier, provider );
}

/** Whether any kernel has a provider of the tier */
UInt32
CountProviders( MATH::TMathBatchTier tier )
{
    UInt32 count = 0;
    for ( UInt32 k=0; k<(UInt32) MATH::MATHBATCHKERNEL_MATH_COUNT; ++k )
        count += HasProvider( (MATH::TMathBatchKernel) k, tier ) ? 1 : 0;
    return count;
}

}

/*-------------------------------------------------------------------------*/

void
PerformMathBridgeTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "MathBridge" );

    CComputeGlobal* compute = CComputeGlobal::Instance();
    const CComputeSettings originalSettings = compute->GetSettings();
    bool loaded = false;

    const ::std::string largeCount = GUCEF_TESTFW_GETENV( "GUCEF_TEST_COMPUTE_MATH_LARGE" );
    if ( !largeCount.empty() && CORE::StringToUInt32( largeCount.c_str() ) > 1023 )
        g_counts[ g_countCount - 1 ] = CORE::StringToUInt32( largeCount.c_str() );

    GUCEF_TESTFW_TESTCASE( "MathBridge: the plugin loads through the plugin control and registers every bridged kernel, its CPU artefact and a MULTICORE provider" )
    try
    {
        ASSERT_TRUE( 0 == CountProviders( MATH::MATHBATCHTIER_MULTICORE ) );
        const UInt32 kernelsBefore = compute->GetKernelRegistry().GetKernelCount();
        loaded = ReloadBridge( PROVIDERTIER_NONE );
        REQUIRE_TRUE( loaded );

        MATH::TMathBatchKernel kernels[ MaxKernels ];
        const UInt32 kernelCount = GetMathMultiCoreKernels( kernels, MaxKernels );
        ASSERT_TRUE( kernelCount >= 10 );
        ASSERT_TRUE( compute->GetKernelRegistry().GetKernelCount() == kernelsBefore + kernelCount );

        // every kernel that has a MULTICORE provider has a test case, and the other way around
        UInt32 withCase = 0;
        for ( UInt32 k=0; k<(UInt32) MATH::MATHBATCHKERNEL_MATH_COUNT; ++k )
            withCase += ( GUCEF_NULL != FindMathKernelCase( (MATH::TMathBatchKernel) k ) && Dispatcher().GetProviderCount( (MATH::TMathBatchKernel) k ) > 0 ) ? 1 : 0;
        ASSERT_TRUE( withCase == kernelCount );

        for ( UInt32 i=0; i<kernelCount; ++i )
        {
            const MATH::TMathBatchKernel kernel = kernels[ i ];
            ASSERT_TRUE( GUCEF_NULL != FindMathKernelCase( kernel ) );
            const CString kernelId = Dispatcher().GetKernelId( kernel );
            TComputeKernelDeclPtr decl;
            TComputeKernelArtefactPtr artefact;
            ASSERT_TRUE( compute->GetKernelRegistry().TryGetKernel( kernelId, decl ) );
            ASSERT_TRUE( compute->GetKernelRegistry().GetArtefact( kernelId, "cpu", artefact ) );
            ASSERT_TRUE( ARTEFACTFORMAT_CPU == artefact->GetFormat() && artefact->IsValid() );

            MATH::SMathBatchProvider provider;
            ASSERT_TRUE( FindMathProvider( kernel, MATH::MATHBATCHTIER_MULTICORE, provider ) );
            ASSERT_TRUE( 0 != ( provider.flags & MATH::MATHPROVIDER_BITEXACT ) );
            ASSERT_TRUE( 0 != ( provider.flags & MATH::MATHPROVIDER_MAY_FAIL ) );
            ASSERT_TRUE( 0 == ( provider.flags & MATH::MATHPROVIDER_REQUIRES_ALIGNED_INPUT ) );
            ASSERT_TRUE( provider.minCount <= provider.maxCount );

            // the declaration: the id is the one the dispatcher has, the determinism class maps to BITEXACT for the exact kernels
            ASSERT_TRUE( decl->GetKernelId() == kernelId );
            ASSERT_TRUE( decl->IsValid() );
            const SMathKernelCase* kc = FindMathKernelCase( kernel );
            ASSERT_TRUE( ( DETERMINISM_EXACT_INTEGER == decl->GetDeterminism() ) == ( MATHCASESHAPE_CONVERT == kc->shape || MATH::MATHBATCHKERNEL_TRANSFORM_POINTS3_FX32 == kernel || MATH::MATHBATCHKERNEL_MULTIPLY_MATRICES4_FX32 == kernel ) );
        }
        // MATH registered its own providers under another owner: the scalar and SIMD ones stay
        for ( UInt32 i=0; i<kernelCount; ++i )
            ASSERT_TRUE( HasProvider( kernels[ i ], MATH::MATHBATCHTIER_SCALAR ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "MathBridge: every bridged kernel at the forced MULTICORE tier is bit-identical with the scalar provider: counts 0 to 10^5, misaligned, in place, strided, small chunks" )
    try
    {
        REQUIRE_TRUE( ReloadBridge( PROVIDERTIER_MULTICORE ) );
        MATH::TMathBatchKernel kernels[ MaxKernels ];
        const UInt32 kernelCount = GetMathMultiCoreKernels( kernels, MaxKernels );
        REQUIRE_TRUE( kernelCount >= 10 );
        for ( UInt32 i=0; i<kernelCount; ++i )
        {
            const SMathKernelCase* kc = FindMathKernelCase( kernels[ i ] );
            ASSERT_TRUE( GUCEF_NULL != kc );
            if ( GUCEF_NULL == kc )
                continue;
            const UInt32 failuresBefore = MultiCoreFailures( kernels[ i ] );
            CString failure;
            const bool equal = CompareKernel( *kc, 0, true, failure );
            if ( !equal )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MathBridge: " << failure );
            ASSERT_TRUE( equal );

            // the calls that declined are the in place conversions only
            const UInt32 declined = MultiCoreFailures( kernels[ i ] ) - failuresBefore;
            if ( MATHCASESHAPE_CONVERT != kc->shape )
                ASSERT_TRUE( 0 == declined );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "MathBridge: the result is bit-identical for 1, 2 and N workers of the job system" )
    try
    {
        REQUIRE_TRUE( ReloadBridge( PROVIDERTIER_MULTICORE ) );
        MATH::TMathBatchKernel kernels[ MaxKernels ];
        const UInt32 kernelCount = GetMathMultiCoreKernels( kernels, MaxKernels );
        const UInt32 workerCounts[ 4 ] = { 1, 2, 3, 0 };
        CCpuParallelFor& parallelFor = compute->GetCpuParallelFor();
        for ( UInt32 w=0; w<4; ++w )
        {
            parallelFor.SetWorkerCount( workerCounts[ w ] );
            for ( UInt32 i=0; i<kernelCount; ++i )
            {
                const SMathKernelCase* kc = FindMathKernelCase( kernels[ i ] );
                ASSERT_TRUE( GUCEF_NULL != kc );
                if ( GUCEF_NULL == kc )
                    continue;
                SLayout packed = { { 0, 0, 0 }, 0, false, "packed" };
                CString failure;
                const bool equal = CompareWithScalar( *kc, LargeCount(), packed, 77 + w, 0, 0, failure );
                if ( !equal )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MathBridge: " << Describe( *kc, LargeCount(), packed, 0, 77 + w ) << " with " << workerCounts[ w ] << " workers: " << failure );
                ASSERT_TRUE( equal );
            }
        }
        parallelFor.SetWorkerCount( 0 );
    }
    catch ( ... )
    {
        compute->GetCpuParallelFor().SetWorkerCount( 0 );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "MathBridge: NO_MULTICORE, REALTIME, FORCE_SCALAR and NO_ACCELERATOR with REALTIME never reach the bridge; DETERMINISTIC and plain calls do" )
    try
    {
        REQUIRE_TRUE( ReloadBridge( PROVIDERTIER_MULTICORE ) );
        COMPUTEBRIDGEMATH::SMathBridgeDiagnostics* diagnostics = GetDiagnostics();
        REQUIRE_TRUE( GUCEF_NULL != diagnostics );
        diagnostics->grainOverrideItems.Store( 7 );
        MATH::TMathBatchKernel kernels[ MaxKernels ];
        const UInt32 kernelCount = GetMathMultiCoreKernels( kernels, MaxKernels );
        for ( UInt32 i=0; i<kernelCount; ++i )
        {
            const SMathKernelCase* kc = FindMathKernelCase( kernels[ i ] );
            ASSERT_TRUE( GUCEF_NULL != kc );
            if ( GUCEF_NULL == kc )
                continue;
            const UInt32 blocked[ 5 ] = { MATH::MATHBATCH_NO_MULTICORE, MATH::MATHBATCH_REALTIME, MATH::MATHBATCH_FORCE_SCALAR,
                                          MATH::MATHBATCH_REALTIME | MATH::MATHBATCH_DETERMINISTIC, MATH::MATHBATCH_NO_MULTICORE | MATH::MATHBATCH_DETERMINISTIC };
            for ( UInt32 b=0; b<5; ++b )
            {
                const UInt32 before = MultiCoreCalls( kernels[ i ] );
                RunPackedCall( *kc, 500, blocked[ b ], 5 );
                ASSERT_TRUE( before == MultiCoreCalls( kernels[ i ] ) );
            }
            const UInt32 allowed[ 3 ] = { 0, MATH::MATHBATCH_DETERMINISTIC, MATH::MATHBATCH_NO_ACCELERATOR };
            for ( UInt32 a=0; a<3; ++a )
            {
                const UInt32 before = MultiCoreCalls( kernels[ i ] );
                RunPackedCall( *kc, 500, allowed[ a ], 5 );
                ASSERT_TRUE( before + 1 == MultiCoreCalls( kernels[ i ] ) );
            }
        }
        diagnostics->grainOverrideItems.Store( 0 );

        // the caps of the application: a kernel capped at SIMD does not reach the bridge either
        const MATH::TMathBatchKernel capped = kernels[ 0 ];
        Dispatcher().SetMaxTier( capped, MATH::MATHBATCHTIER_SIMD );
        const UInt32 before = MultiCoreCalls( capped );
        RunPackedCall( *FindMathKernelCase( capped ), 500, 0, 5 );
        ASSERT_TRUE( before == MultiCoreCalls( capped ) );
        Dispatcher().SetMaxTier( capped, MATH::MATHBATCHTIER_ACCELERATOR );
        RunPackedCall( *FindMathKernelCase( capped ), 500, 0, 5 );
        ASSERT_TRUE( before + 1 == MultiCoreCalls( capped ) );
    }
    catch ( ... )
    {
        Dispatcher().SetMaxTier( MATH::MATHBATCHTIER_ACCELERATOR );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "MathBridge: a MULTICORE provider that fails leaves the output alone and the call completes on the next tier with the scalar result, in place too" )
    try
    {
        REQUIRE_TRUE( ReloadBridge( PROVIDERTIER_MULTICORE ) );
        COMPUTEBRIDGEMATH::SMathBridgeDiagnostics* diagnostics = GetDiagnostics();
        REQUIRE_TRUE( GUCEF_NULL != diagnostics );
        MATH::TMathBatchKernel kernels[ MaxKernels ];
        const UInt32 kernelCount = GetMathMultiCoreKernels( kernels, MaxKernels );
        for ( UInt32 i=0; i<kernelCount; ++i )
        {
            const SMathKernelCase* kc = FindMathKernelCase( kernels[ i ] );
            ASSERT_TRUE( GUCEF_NULL != kc );
            if ( GUCEF_NULL == kc )
                continue;
            SLayout layouts[ 16 ];
            const UInt32 layoutCount = MakeLayouts( *kc, layouts );
            for ( UInt32 l=0; l<layoutCount; ++l )
            {
                diagnostics->forcedMultiCoreFailures.Store( 1 );
                const UInt32 callsBefore = MultiCoreCalls( kernels[ i ] );
                const UInt32 failuresBefore = MultiCoreFailures( kernels[ i ] );
                CString failure;
                const bool equal = CompareWithScalar( *kc, 1000, layouts[ l ], 91 + l, 0, 0, failure );
                if ( !equal )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MathBridge: " << Describe( *kc, 1000, layouts[ l ], 0, 91 + l ) << ": " << failure );
                ASSERT_TRUE( equal );
                ASSERT_TRUE( 0 == diagnostics->forcedMultiCoreFailures.Load() );
                ASSERT_TRUE( callsBefore + 1 == MultiCoreCalls( kernels[ i ] ) );
                ASSERT_TRUE( failuresBefore + 1 == MultiCoreFailures( kernels[ i ] ) );
            }
        }
        diagnostics->forcedMultiCoreFailures.Store( 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "MathBridge: the threshold sends counts below the minimum of the cost model to SIMD and counts from it to the MULTICORE provider" )
    try
    {
        REQUIRE_TRUE( ReloadBridge( PROVIDERTIER_NONE ) );
        MATH::TMathBatchKernel kernels[ MaxKernels ];
        const UInt32 kernelCount = GetMathMultiCoreKernels( kernels, MaxKernels );
        REQUIRE_TRUE( kernelCount >= 10 );
        UInt32 testedAtThreshold = 0;
        for ( UInt32 i=0; i<kernelCount; ++i )
        {
            const SMathKernelCase* kc = FindMathKernelCase( kernels[ i ] );
            MATH::SMathBatchProvider provider;
            ASSERT_TRUE( GUCEF_NULL != kc && FindMathProvider( kernels[ i ], MATH::MATHBATCHTIER_MULTICORE, provider ) );
            if ( GUCEF_NULL == kc )
                continue;
            const COMPUTEBRIDGEMATH::SMathBridgeProviderContext* context = FindMathProviderContext( kernels[ i ], MATH::MATHBATCHTIER_MULTICORE );

            // at least two chunks: nothing runs side by side below that
            ASSERT_TRUE( provider.minCount >= 2 * context->info->grainItems );
            ASSERT_TRUE( provider.minCount < 4000000 );

            const UInt32 threshold = provider.minCount;
            const UInt32 before = MultiCoreCalls( kernels[ i ] );
            RunPackedCall( *kc, 1, 0, 5 );
            RunPackedCall( *kc, threshold / 2, 0, 5 );
            RunPackedCall( *kc, threshold - 1, 0, 5 );
            ASSERT_TRUE( before == MultiCoreCalls( kernels[ i ] ) );

            CString failure;
            SLayout packed = { { 0, 0, 0 }, 0, false, "packed" };
            ASSERT_TRUE( CompareWithScalar( *kc, threshold - 1, packed, 5, 0, 0, failure ) );
            ASSERT_TRUE( before == MultiCoreCalls( kernels[ i ] ) );
            ASSERT_TRUE( CompareWithScalar( *kc, threshold, packed, 5, 0, 0, failure ) );
            ASSERT_TRUE( before + 1 == MultiCoreCalls( kernels[ i ] ) );
            RunPackedCall( *kc, threshold * 2, 0, 5 );
            ASSERT_TRUE( before + 2 == MultiCoreCalls( kernels[ i ] ) );
            ++testedAtThreshold;
        }
        ASSERT_TRUE( testedAtThreshold == kernelCount );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "MathBridge: the settings replace the cost model: minimum count, disabled tier, a forced lower tier" )
    try
    {
        MATH::TMathBatchKernel kernels[ MaxKernels ];
        ASSERT_TRUE( ReloadBridge( PROVIDERTIER_NONE, 5000 ) );
        UInt32 kernelCount = GetMathMultiCoreKernels( kernels, MaxKernels );
        ASSERT_TRUE( kernelCount >= 10 );
        for ( UInt32 i=0; i<kernelCount; ++i )
        {
            MATH::SMathBatchProvider provider;
            ASSERT_TRUE( FindMathProvider( kernels[ i ], MATH::MATHBATCHTIER_MULTICORE, provider ) );
            ASSERT_TRUE( 5000 == provider.minCount );
        }

        ASSERT_TRUE( ReloadBridge( PROVIDERTIER_NONE, 0, 1 << PROVIDERTIER_MULTICORE ) );
        ASSERT_TRUE( 0 == GetMathMultiCoreKernels( kernels, MaxKernels ) );
        ASSERT_TRUE( 0 == CountProviders( MATH::MATHBATCHTIER_MULTICORE ) );

        ASSERT_TRUE( ReloadBridge( PROVIDERTIER_SIMD ) );
        ASSERT_TRUE( 0 == CountProviders( MATH::MATHBATCHTIER_MULTICORE ) );
        // the kernels are registered with gucefCOMPUTE all the same: they are still declared
        ASSERT_TRUE( compute->GetKernelRegistry().IsKernelRegistered( Dispatcher().GetKernelId( MATH::MATHBATCHKERNEL_TRANSFORM_POINTS3_F32 ) ) );

        ASSERT_TRUE( ReloadBridge( PROVIDERTIER_MULTICORE ) );
        kernelCount = GetMathMultiCoreKernels( kernels, MaxKernels );
        ASSERT_TRUE( kernelCount >= 10 );
        for ( UInt32 i=0; i<kernelCount; ++i )
        {
            MATH::SMathBatchProvider provider;
            ASSERT_TRUE( FindMathProvider( kernels[ i ], MATH::MATHBATCHTIER_MULTICORE, provider ) );
            ASSERT_TRUE( 0 == provider.minCount );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "MathBridge: the CPU kernel of every bridged kernel, run through a device, equals the scalar provider" )
    try
    {
        REQUIRE_TRUE( ReloadBridge( PROVIDERTIER_NONE ) );
        MATH::TMathBatchKernel kernels[ MaxKernels ];
        const UInt32 kernelCount = GetMathMultiCoreKernels( kernels, MaxKernels );
        REQUIRE_TRUE( kernelCount >= 10 );
        TComputeDevicePtr device = CreateCpuTestDevice();
        ASSERT_TRUE( !device.IsNULL() );
        CComputeHostBatch batch( device );
        const UInt32 deviceCounts[ 9 ] = { 1, 2, 3, 255, 256, 257, 1023, 4097, LargeCount() };
        for ( UInt32 i=0; i<kernelCount; ++i )
        {
            const SMathKernelCase* kc = FindMathKernelCase( kernels[ i ] );
            ASSERT_TRUE( GUCEF_NULL != kc );
            if ( GUCEF_NULL == kc )
                continue;
            for ( UInt32 c=0; c<9; ++c )
            {
                CString failure;
                const bool equal = CompareDeviceKernel( device, batch, *kc, deviceCounts[ c ], 300 + c, failure );
                if ( !equal )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MathBridge: " << Dispatcher().GetKernelId( kernels[ i ] ) << " count " << deviceCounts[ c ] << ": " << failure );
                ASSERT_TRUE( equal );
            }
        }
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == batch.WaitIdle() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "MathBridge: ACCELERATOR providers on a test device: packed and in place calls equal scalar, strided calls and a lost device fall back, DETERMINISTIC only for exact kernels" )
    try
    {
        CComputeBackendRegistry& registry = compute->GetBackendRegistry();
        REQUIRE_TRUE( ReloadBridge( PROVIDERTIER_ACCELERATOR ) );
        ASSERT_TRUE( 0 == CountProviders( MATH::MATHBATCHTIER_ACCELERATOR ) );
        COMPUTEBRIDGEMATH::SMathBridgeDiagnostics* diagnostics = GetDiagnostics();
        REQUIRE_TRUE( GUCEF_NULL != diagnostics );
        ASSERT_TRUE( GUCEF_NULL == diagnostics->acceleratorDevice.Load() );

        // a backend that offers a discrete device appears: the bridge hears about it and registers its providers
        TComputeBackendPtr backend( GUCEF_NEW CTestComputeBackend() );
        const UInt32 generationBefore = diagnostics->acceleratorGeneration.Load();
        ASSERT_TRUE( registry.RegisterBackend( backend ) );
        ASSERT_TRUE( diagnostics->acceleratorGeneration.Load() > generationBefore );
        ASSERT_TRUE( GUCEF_NULL != diagnostics->acceleratorDevice.Load() );

        MATH::TMathBatchKernel kernels[ MaxKernels ];
        const UInt32 kernelCount = GetMathMultiCoreKernels( kernels, MaxKernels );
        ASSERT_TRUE( kernelCount >= 10 );
        ASSERT_TRUE( kernelCount == CountProviders( MATH::MATHBATCHTIER_ACCELERATOR ) );
        for ( UInt32 i=0; i<kernelCount; ++i )
        {
            const SMathKernelCase* kc = FindMathKernelCase( kernels[ i ] );
            MATH::SMathBatchProvider provider;
            ASSERT_TRUE( GUCEF_NULL != kc && FindMathProvider( kernels[ i ], MATH::MATHBATCHTIER_ACCELERATOR, provider ) );
            if ( GUCEF_NULL == kc )
                continue;

            // may fail (strides, a lost device); bit-exact only for the kernels whose declaration is exact integer
            ASSERT_TRUE( 0 != ( provider.flags & MATH::MATHPROVIDER_MAY_FAIL ) );
            TComputeKernelDeclPtr decl;
            ASSERT_TRUE( compute->GetKernelRegistry().TryGetKernel( Dispatcher().GetKernelId( kernels[ i ] ), decl ) );
            const bool exact = DETERMINISM_EXACT_INTEGER == decl->GetDeterminism();
            ASSERT_TRUE( exact == ( 0 != ( provider.flags & MATH::MATHPROVIDER_BITEXACT ) ) );

            const COMPUTEBRIDGEMATH::SMathBridgeProviderContext* context = FindMathProviderContext( kernels[ i ], MATH::MATHBATCHTIER_ACCELERATOR );
            SLayout layouts[ 16 ];
            const UInt32 layoutCount = MakeLayouts( *kc, layouts );
            for ( UInt32 l=0; l<layoutCount; ++l )
            {
                const bool packedCall = ( 0 == layouts[ l ].stride[ 0 ] || kc->in0Bytes == layouts[ l ].stride[ 0 ] ) && ( 0 == layouts[ l ].stride[ 1 ] || kc->in1Bytes == layouts[ l ].stride[ 1 ] ) &&
                                        ( 0 == layouts[ l ].stride[ 2 ] || kc->outBytes == layouts[ l ].stride[ 2 ] );
                const UInt32 callsBefore = AcceleratorCalls( kernels[ i ] );
                const UInt32 failuresBefore = AcceleratorFailures( kernels[ i ] );
                CString failure;
                // the multi-core tier is out of the way: the call goes to the accelerator or, when that declines, to SIMD
                const bool equal = CompareWithScalar( *kc, 1000, layouts[ l ], 400 + l, 0, MATH::MATHBATCH_NO_MULTICORE, failure );
                if ( !equal )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MathBridge: accelerator " << Describe( *kc, 1000, layouts[ l ], 0, 400 + l ) << ": " << failure );
                ASSERT_TRUE( equal );
                ASSERT_TRUE( callsBefore + 1 == AcceleratorCalls( kernels[ i ] ) );
                // a call with strides is declined, a packed one is served
                const bool expectedFailures = ( packedCall ? 0 : 1 ) == AcceleratorFailures( kernels[ i ] ) - failuresBefore;
                if ( !expectedFailures )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MathBridge: accelerator " << Describe( *kc, 1000, layouts[ l ], 0, 400 + l ) << ": " << ( AcceleratorFailures( kernels[ i ] ) - failuresBefore ) << " declined calls" );
                ASSERT_TRUE( expectedFailures );
            }
            (void) context;

            // the call flags
            const UInt32 before = AcceleratorCalls( kernels[ i ] );
            RunPackedCall( *kc, 500, MATH::MATHBATCH_NO_ACCELERATOR, 7 );
            RunPackedCall( *kc, 500, MATH::MATHBATCH_REALTIME, 7 );
            RunPackedCall( *kc, 500, MATH::MATHBATCH_FORCE_SCALAR, 7 );
            ASSERT_TRUE( before == AcceleratorCalls( kernels[ i ] ) );
            RunPackedCall( *kc, 500, MATH::MATHBATCH_DETERMINISTIC | MATH::MATHBATCH_NO_MULTICORE, 7 );
            ASSERT_TRUE( before + ( exact ? 1 : 0 ) == AcceleratorCalls( kernels[ i ] ) );
        }

        // the device is lost: calls go on with the next tier at once, the providers go when the event arrives
        CComputeDevice* device = diagnostics->acceleratorDevice.Load();
        ASSERT_TRUE( GUCEF_NULL != device );
        const SMathKernelCase* kc = FindMathKernelCase( MATH::MATHBATCHKERNEL_TRANSFORM_POINTS3_F32 );
        SLayout packed = { { 0, 0, 0 }, 0, false, "packed" };
        ASSERT_TRUE( device->SimulateDeviceLoss( "the test takes the device away" ) );
        CString failure;
        ASSERT_TRUE( CompareWithScalar( *kc, 1000, packed, 9, 0, MATH::MATHBATCH_NO_MULTICORE, failure ) );
        for ( UInt32 waited=0; waited<2000 && CountProviders( MATH::MATHBATCHTIER_ACCELERATOR ) > 0; ++waited )
            PumpMainPulseGenerator( 1 );
        ASSERT_TRUE( 0 == CountProviders( MATH::MATHBATCHTIER_ACCELERATOR ) );
        ASSERT_TRUE( GUCEF_NULL == diagnostics->acceleratorDevice.Load() );
        ASSERT_TRUE( CompareWithScalar( *kc, 1000, packed, 10, 0, 0, failure ) );

        // the backend changes again (reinitialised): a new device is made and the providers come back
        registry.TryUnregister( CTestComputeBackend::Name );
        TComputeBackendPtr second( GUCEF_NEW CTestComputeBackend() );
        ASSERT_TRUE( registry.RegisterBackend( second ) );
        ASSERT_TRUE( kernelCount == CountProviders( MATH::MATHBATCHTIER_ACCELERATOR ) );
        ASSERT_TRUE( CompareWithScalar( *kc, 1000, packed, 11, 0, MATH::MATHBATCH_NO_MULTICORE, failure ) );

        // the plugin unloads with the device in use: nothing of it stays registered
        ASSERT_TRUE( UnloadMathBridgePlugin() );
        ASSERT_TRUE( 0 == CountProviders( MATH::MATHBATCHTIER_ACCELERATOR ) );
        registry.TryUnregister( CTestComputeBackend::Name );
        ASSERT_TRUE( ReloadBridge( PROVIDERTIER_NONE ) );
    }
    catch ( ... )
    {
        compute->GetBackendRegistry().TryUnregister( CTestComputeBackend::Name );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "MathBridge: unloading the plugin removes exactly its providers and kernels, other providers stay, and the dispatcher serves the calls from the tiers below" )
    try
    {
        REQUIRE_TRUE( ReloadBridge( PROVIDERTIER_MULTICORE ) );
        MATH::TMathBatchKernel kernels[ MaxKernels ];
        const UInt32 kernelCount = GetMathMultiCoreKernels( kernels, MaxKernels );
        REQUIRE_TRUE( kernelCount >= 10 );

        // a provider of somebody else, registered at the MULTICORE tier too
        char otherOwner = 0;
        MATH::SMathBatchProvider other;
        other.tier = MATH::MATHBATCHTIER_MULTICORE;
        other.name = "test provider of another owner";
        other.function = GUCEF_NULL;
        other.minCount = 0;
        other.maxCount = 0xFFFFFFFF;
        other.flags = MATH::MATHPROVIDER_BITEXACT | MATH::MATHPROVIDER_MAY_FAIL;
        other.alignment = 0;
        other.requiredCpuFeatures = 0;
        other.context = GUCEF_NULL;
        // a function is required: any with the right signature will do, it is never called (NO_MULTICORE below)
        struct SNeverCalled { static MATH::TMathStatus Run( void*, const MATH::SMathDotProducts3F32Args& ) { return MATH::MATHSTATUS_INVALID_OPERAND; } };
        other.function = reinterpret_cast< MATH::TMathBatchGenericFunction >( &SNeverCalled::Run );
        const UInt32 providersBefore = Dispatcher().GetProviderCount( MATH::MATHBATCHKERNEL_DOT_PRODUCTS3_F32 );
        ASSERT_TRUE( Dispatcher().RegisterProvider( MATH::MATHBATCHKERNEL_DOT_PRODUCTS3_F32, other, &otherOwner ) );
        ASSERT_TRUE( providersBefore + 1 == Dispatcher().GetProviderCount( MATH::MATHBATCHKERNEL_DOT_PRODUCTS3_F32 ) );

        const UInt32 registered = compute->GetKernelRegistry().GetKernelCount();
        ASSERT_TRUE( UnloadMathBridgePlugin() );
        ASSERT_TRUE( 1 == CountProviders( MATH::MATHBATCHTIER_MULTICORE ) );
        ASSERT_TRUE( HasProvider( MATH::MATHBATCHKERNEL_DOT_PRODUCTS3_F32, MATH::MATHBATCHTIER_MULTICORE ) );
        // the one of the bridge is gone, the one of the other owner is still there
        ASSERT_TRUE( providersBefore == Dispatcher().GetProviderCount( MATH::MATHBATCHKERNEL_DOT_PRODUCTS3_F32 ) );
        ASSERT_TRUE( compute->GetKernelRegistry().GetKernelCount() == registered - kernelCount );
        for ( UInt32 i=0; i<kernelCount; ++i )
            ASSERT_FALSE( compute->GetKernelRegistry().IsKernelRegistered( Dispatcher().GetKernelId( kernels[ i ] ) ) );

        // the calls that no provider of the other owner takes (NO_MULTICORE) are served by the tiers of gucefMATH
        const SMathKernelCase* kc = FindMathKernelCase( MATH::MATHBATCHKERNEL_TRANSFORM_POINTS3_F32 );
        SLayout packed = { { 0, 0, 0 }, 0, false, "packed" };
        CString failure;
        ASSERT_TRUE( CompareWithScalar( *kc, 1000, packed, 3, 0, 0, failure ) );

        ASSERT_TRUE( 1 == Dispatcher().UnregisterProviders( &otherOwner ) );
        ASSERT_TRUE( 0 == CountProviders( MATH::MATHBATCHTIER_MULTICORE ) );

        // and the plugin loads again where it was
        ASSERT_TRUE( LoadMathBridgePlugin() );
        ASSERT_TRUE( kernelCount == GetMathMultiCoreKernels( kernels, MaxKernels ) );
        ASSERT_TRUE( compute->GetKernelRegistry().GetKernelCount() == registered );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "MathBridge: the plugin unloads and loads again 12 times while 4 threads call batch functions: every result right, no call is inside the plugin when the unload returns" )
    try
    {
        ASSERT_TRUE( ReloadBridge( PROVIDERTIER_MULTICORE ) );
        MT::CTAtomic< UInt32 > stop( 0 );
        MT::CTAtomic< UInt32 > mismatches( 0 );
        MT::CTAtomic< UInt32 > calls( 0 );
        CBatchCaller callers[ 4 ] = { CBatchCaller( 1, stop, mismatches, calls ), CBatchCaller( 2, stop, mismatches, calls ),
                                      CBatchCaller( 3, stop, mismatches, calls ), CBatchCaller( 4, stop, mismatches, calls ) };
        CTestThread threads[ 4 ];
        for ( UInt32 t=0; t<4; ++t )
            ASSERT_TRUE( threads[ t ].Start( &callers[ t ] ) );

        UInt32 multiCoreCalls = 0;
        for ( UInt32 cycle=0; cycle<12; ++cycle )
        {
            MT::PrecisionDelay( 5 );
            COMPUTEBRIDGEMATH::SMathBridgeDiagnostics* diagnostics = GetDiagnostics();
            if ( GUCEF_NULL != diagnostics )
                multiCoreCalls += diagnostics->multiCoreCalls[ MATH::MATHBATCHKERNEL_TRANSFORM_POINTS3_F32 ].Load();
            ASSERT_TRUE( UnloadMathBridgePlugin() );
            ASSERT_TRUE( 0 == CountProviders( MATH::MATHBATCHTIER_MULTICORE ) );
            MT::PrecisionDelay( 3 );
            ASSERT_TRUE( LoadMathBridgePlugin() );
        }
        MT::PrecisionDelay( 10 );
        stop.Store( 1 );
        for ( UInt32 t=0; t<4; ++t )
            ASSERT_TRUE( threads[ t ].Join( 60000 ) );
        ASSERT_TRUE( 0 == mismatches.Load() );
        ASSERT_TRUE( calls.Load() > 0 );
        // the callers reached the multi-core tier while it was there
        ASSERT_TRUE( multiCoreCalls > 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "MathBridge: a CPU kernel that an application holds keeps working after the plugin unloaded, and the plugin loads again" )
    try
    {
        REQUIRE_TRUE( ReloadBridge( PROVIDERTIER_NONE ) );
        COMPUTEBRIDGEMATH::SMathBridgeDiagnostics* diagnostics = GetDiagnostics();
        REQUIRE_TRUE( GUCEF_NULL != diagnostics );
        const SMathKernelCase* kc = FindMathKernelCase( MATH::MATHBATCHKERNEL_TRANSFORM_POINTS3_F32 );
        const CString kernelId = Dispatcher().GetKernelId( MATH::MATHBATCHKERNEL_TRANSFORM_POINTS3_F32 );
        TComputeDevicePtr device = CreateCpuTestDevice();
        CComputeHostBatch batch( device );
        TComputeKernelPtr held;
        ASSERT_TRUE( device->CreateKernel( kernelId, held ) );
        const UInt32 live = diagnostics->liveKernels.Load();
        ASSERT_TRUE( live > 0 );

        // the plugin goes: its providers and kernels are removed, the kernel object of the application stays and runs
        ASSERT_TRUE( UnloadMathBridgePlugin() );
        ASSERT_TRUE( 0 == CountProviders( MATH::MATHBATCHTIER_MULTICORE ) );
        ASSERT_FALSE( compute->GetKernelRegistry().IsKernelRegistered( kernelId ) );
        ASSERT_TRUE( live == diagnostics->liveKernels.Load() );
        CString failure;
        const bool equal = CompareDeviceKernel( device, batch, *kc, 5000, 21, failure, held );
        if ( !equal )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MathBridge: " << failure );
        ASSERT_TRUE( equal );

        held.Unlink();
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == batch.WaitIdle() );
        ASSERT_TRUE( LoadMathBridgePlugin() );
        ASSERT_TRUE( HasProvider( MATH::MATHBATCHKERNEL_TRANSFORM_POINTS3_F32, MATH::MATHBATCHTIER_MULTICORE ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    UnloadMathBridgePlugin();
    ClearMathBridgeSettings();
    compute->ApplySettings( originalSettings );
    (void) loaded;
}

/*-------------------------------------------------------------------------*/
