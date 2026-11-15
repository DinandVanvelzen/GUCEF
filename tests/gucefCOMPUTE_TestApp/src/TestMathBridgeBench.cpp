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

#ifndef GUCEF_MATH_CMATHGLOBAL_H
#include "gucefMATH_CMathGlobal.h"
#define GUCEF_MATH_CMATHGLOBAL_H
#endif /* GUCEF_MATH_CMATHGLOBAL_H ? */

#ifndef GUCEF_COMPUTE_H
#include "gucefCOMPUTE.h"
#define GUCEF_COMPUTE_H
#endif /* GUCEF_COMPUTE_H ? */

#include "TestMathBridgeHarness.h"
#include "TestMathBridge.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::COMPUTE;
using namespace GUCEF::COMPUTE::TEST;

/*-------------------------------------------------------------------------*/

namespace {

const UInt32 MaxKernels = 64;

/** The size of the table of the minimum counts: every MATH kernel */
enum { MATHBATCHKERNEL_MATH_COUNT_BENCH = 64 };

/** The most bytes of arrays one benchmark run touches */
const UInt64 MaxBenchBytes = 256ULL * 1024 * 1024;

/** Which tier a measurement runs: the flags that select it */
struct STierRun
{
    const char* name;
    UInt32 flags;
};

const STierRun g_tiers[ 3 ] =
{
    { "scalar",    MATH::MATHBATCH_FORCE_SCALAR },
    { "simd",      MATH::MATHBATCH_NO_MULTICORE },
    { "multicore", 0                            }
};

/** Seconds per call of the best of a few runs, in nanoseconds per item */
Float64
MeasureNsPerItem( const SMathKernelCase& kc, UInt32 count, UInt32 flags, const SMathCall& call )
{
    // enough repetitions that the shortest one lasts a measurable time
    const UInt32 repetitions = count < 4096 ? 400 : ( count < 262144 ? 20 : 3 );
    Float64 best = 0.0;
    for ( UInt32 pass=0; pass<5; ++pass )
    {
        const UInt64 start = MT::PrecisionTickCount();
        for ( UInt32 r=0; r<repetitions; ++r )
            kc.run( MATH::CMathGlobal::Instance()->GetBatchDispatcher(), flags, call );
        const Float64 ns = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) * 1000000.0;
        const Float64 perCall = ns / repetitions;
        if ( 0 == pass || perCall < best )
            best = perCall;
    }
    return best / count;
}

}

/*-------------------------------------------------------------------------*/

void
PerformMathBridgeBenchmarks( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "MathBridgeBenchmarks" );

    GUCEF_TESTFW_TESTCASE( "MathBridgeBenchmarks: multi-core crossover per kernel (report only, GUCEF_TEST_COMPUTE_BENCH=1)" )
    GUCEF_TESTFW_SKIP_IF( GUCEF_TESTFW_GETENV( "GUCEF_TEST_COMPUTE_BENCH" ) != "1", "GUCEF_TEST_COMPUTE_BENCH=1 runs the benchmarks" );
    try
    {

        CComputeGlobal* compute = CComputeGlobal::Instance();
        const CComputeSettings originalSettings = compute->GetSettings();
        // the minimum counts the cost model registers, to put beside the measured crossovers
        UInt32 modelMinCount[ MATHBATCHKERNEL_MATH_COUNT_BENCH ];
        for ( UInt32 k=0; k<(UInt32) MATH::MATHBATCHKERNEL_MATH_COUNT; ++k )
            modelMinCount[ k ] = 0;
        UnloadMathBridgePlugin();
        SetMathBridgeSettings( PROVIDERTIER_NONE );
        GUCEF_TESTFW_ASSERT_TRUE( LoadMathBridgePlugin() );
        for ( UInt32 k=0; k<(UInt32) MATH::MATHBATCHKERNEL_MATH_COUNT; ++k )
        {
            MATH::SMathBatchProvider provider;
            if ( FindMathProvider( (MATH::TMathBatchKernel) k, MATH::MATHBATCHTIER_MULTICORE, provider ) )
                modelMinCount[ k ] = provider.minCount;
        }

        UnloadMathBridgePlugin();
        SetMathBridgeSettings( PROVIDERTIER_MULTICORE );
        GUCEF_TESTFW_ASSERT_TRUE( LoadMathBridgePlugin() );

        MATH::TMathBatchKernel kernels[ MaxKernels ];
        const UInt32 kernelCount = GetMathMultiCoreKernels( kernels, MaxKernels );
        const MATH::CMathBatchDispatcher& dispatcher = MATH::CMathGlobal::Instance()->GetBatchDispatcher();
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "MathBridgeBenchmarks: " << kernelCount << " kernels, " << compute->GetCpuParallelFor().GetWorkerCount() << " workers, columns are ns per item" );

        // the fork-join latency: calls of 2 and of 64 chunks of one item each, nothing but the hand-over to the workers
        {
            const SMathKernelCase* kc = FindMathKernelCase( MATH::MATHBATCHKERNEL_DOT_PRODUCTS3_F32 );
            const COMPUTEBRIDGEMATH::SMathBridgeProviderContext* context = FindMathProviderContext( MATH::MATHBATCHKERNEL_DOT_PRODUCTS3_F32, MATH::MATHBATCHTIER_MULTICORE );
            if ( GUCEF_NULL != kc && GUCEF_NULL != context )
            {
                const UInt32 chunkCounts[ 3 ] = { 2, 8, 64 };
                for ( UInt32 c=0; c<3; ++c )
                {
                    const UInt32 chunks = chunkCounts[ c ];
                    context->diagnostics->grainOverrideItems.Store( 1 );
                    CTestBuffer a, b, out;
                    a.Resize( chunks * 12, 0, 0 );
                    b.Resize( chunks * 12, 0, 0 );
                    out.Resize( chunks * 4, 0, 0 );
                    SMathCall call;
                    call.in0 = a.Data(); call.in0Stride = 12; call.in1 = b.Data(); call.in1Stride = 12; call.out = out.Data(); call.outStride = 4; call.count = chunks;
                    const Float64 perItem = MeasureNsPerItem( *kc, chunks, 0, call );
                    context->diagnostics->grainOverrideItems.Store( 0 );
                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "MathBridgeBenchmarks: fork-join over " << chunks << " one item chunks: " << perItem * chunks << " ns per call" );
                }
            }
        }

        for ( UInt32 i=0; i<kernelCount; ++i )
        {
            const SMathKernelCase* kc = FindMathKernelCase( kernels[ i ] );
            if ( GUCEF_NULL == kc )
                continue;
            const UInt32 bytesPerItem = kc->in0Bytes + kc->in1Bytes + kc->outBytes;
            const MATH::TFloat32Matrix4D matrix = MakeTestMatrix( 0 );

            const CString kernelName = dispatcher.GetKernelId( kernels[ i ] );
            Float64 simdTimes[ 24 ];
            Float64 multiTimes[ 24 ];
            UInt32 sizes[ 24 ];
            UInt32 measured = 0;
            for ( UInt32 exponent=8; exponent<=24; ++exponent )
            {
                const UInt32 count = 1u << exponent;
                if ( static_cast< UInt64 >( count ) * bytesPerItem > MaxBenchBytes )
                    break;
                CTestBuffer in0, in1, out;
                in0.Resize( static_cast< UInt64 >( count ) * kc->in0Bytes, 0, 0 );
                kc->fill( 0, in0.Data(), count, kc->in0Bytes, 11 );
                if ( 0 != kc->in1Bytes )
                {
                    in1.Resize( static_cast< UInt64 >( count ) * kc->in1Bytes, 0, 0 );
                    kc->fill( 1, in1.Data(), count, kc->in1Bytes, 12 );
                }
                out.Resize( static_cast< UInt64 >( count ) * kc->outBytes, 0, 0 );
                SMathCall call;
                call.in0 = in0.Data(); call.in0Stride = kc->in0Bytes; call.in1 = in1.Data(); call.in1Stride = kc->in1Bytes;
                call.out = out.Data(); call.outStride = kc->outBytes; call.count = count; call.matrix = &matrix;

                Float64 times[ 3 ];
                for ( UInt32 t=0; t<3; ++t )
                    times[ t ] = MeasureNsPerItem( *kc, count, g_tiers[ t ].flags, call );
                sizes[ measured ] = count;
                simdTimes[ measured ] = times[ 1 ];
                multiTimes[ measured ] = times[ 2 ];
                ++measured;
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "MathBridgeBenchmarks: " << kernelName << " count " << count << " ns/item: scalar " << times[ 0 ] << " simd " << times[ 1 ] << " multicore " << times[ 2 ] );
            }

            // the crossover: the smallest size from which multi-core stays faster than SIMD
            UInt32 crossover = 0;
            for ( UInt32 m=measured; m>0; --m )
            {
                if ( multiTimes[ m - 1 ] < simdTimes[ m - 1 ] )
                    crossover = sizes[ m - 1 ];
                else
                    break;
            }
            const COMPUTEBRIDGEMATH::SMathBridgeProviderContext* context = FindMathProviderContext( kernels[ i ], MATH::MATHBATCHTIER_MULTICORE );
            Float64 largest = 0.0;
            if ( measured > 0 )
                largest = simdTimes[ measured - 1 ] / multiTimes[ measured - 1 ];
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "MathBridgeBenchmarks: " << kernelName << " crossover from " << crossover << " items, speedup " << largest << " at " << ( measured > 0 ? sizes[ measured - 1 ] : 0 ) <<
                              " items; cost model minimum count " << modelMinCount[ kernels[ i ] ] << ", " << ( GUCEF_NULL != context ? context->info->simdNsPerItem : 0.0 ) << " ns/item single thread, grain " << ( GUCEF_NULL != context ? context->info->grainItems : 0 ) <<
                              ", speedup bound " << ( GUCEF_NULL != context ? context->info->maxMultiCoreSpeedup : 0.0 ) );
        }

        UnloadMathBridgePlugin();
        ClearMathBridgeSettings();
        compute->ApplySettings( originalSettings );
    }
    catch ( ... )
    {
        GUCEF_TESTFW_ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
