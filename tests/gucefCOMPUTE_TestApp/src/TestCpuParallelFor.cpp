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

#include <stdlib.h>

#include <string.h>

#include <new>

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

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

#include "TestCpuParallelFor.h"

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      ALLOCATION COUNTING                                                //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  Counts the calls of operator new of every thread while armed: the workers allocate on their own threads.
 *  The replacement is for the whole test application and forwards to malloc; the nothrow forms are replaced as well,
 *  so that no memory of the replacement reaches the operator delete of a sanitizer runtime or the other way around.
 */

static MT::CTAtomic< UInt32 > g_countAllocations( 0 );
static MT::CTAtomic< UInt32 > g_allocationCount( 0 );

static void
CountAllocation( void )
{
    if ( 0 != g_countAllocations.Load( MT::MEMORDER_RELAXED ) )
        g_allocationCount.FetchAdd( 1, MT::MEMORDER_RELAXED );
}

#if defined( __cplusplus ) && ( __cplusplus >= 201103L )
  #define COMPUTETEST_NEW_SPEC
  #define COMPUTETEST_DELETE_SPEC noexcept
#else
  #define COMPUTETEST_NEW_SPEC throw( std::bad_alloc )
  #define COMPUTETEST_DELETE_SPEC throw()
#endif

void* operator new( std::size_t size ) COMPUTETEST_NEW_SPEC
{
    CountAllocation();
    void* memory = malloc( 0 == size ? 1 : size );
    if ( GUCEF_NULL == memory )
        throw std::bad_alloc();
    return memory;
}

void* operator new[]( std::size_t size ) COMPUTETEST_NEW_SPEC
{
    CountAllocation();
    void* memory = malloc( 0 == size ? 1 : size );
    if ( GUCEF_NULL == memory )
        throw std::bad_alloc();
    return memory;
}

void* operator new( std::size_t size, const std::nothrow_t& ) COMPUTETEST_DELETE_SPEC
{
    CountAllocation();
    return malloc( 0 == size ? 1 : size );
}

void* operator new[]( std::size_t size, const std::nothrow_t& ) COMPUTETEST_DELETE_SPEC
{
    CountAllocation();
    return malloc( 0 == size ? 1 : size );
}

void operator delete( void* memory, const std::nothrow_t& ) COMPUTETEST_DELETE_SPEC { free( memory ); }
void operator delete[]( void* memory, const std::nothrow_t& ) COMPUTETEST_DELETE_SPEC { free( memory ); }
void operator delete( void* memory ) COMPUTETEST_DELETE_SPEC { free( memory ); }
void operator delete[]( void* memory ) COMPUTETEST_DELETE_SPEC { free( memory ); }
#if defined( __cplusplus ) && ( __cplusplus >= 201402L )
void operator delete( void* memory, std::size_t ) COMPUTETEST_DELETE_SPEC { free( memory ); }
void operator delete[]( void* memory, std::size_t ) COMPUTETEST_DELETE_SPEC { free( memory ); }
#endif

/*-------------------------------------------------------------------------*/

namespace {

void
StartCounting( void )
{
    g_allocationCount.Store( 0 );
    g_countAllocations.Store( 1 );
}

UInt32
StopCounting( void )
{
    g_countAllocations.Store( 0 );
    return g_allocationCount.Load();
}

/** Workers start in the background: waits until the number of running workers is as expected */
bool
WaitForRunningWorkers( CCpuParallelFor& parallelFor, UInt32 expected )
{
    for ( UInt32 waited=0; waited<5000; ++waited )
    {
        if ( parallelFor.GetRunningWorkerCount() == expected )
            return true;
        MT::PrecisionDelay( 1 );
    }
    return parallelFor.GetRunningWorkerCount() == expected;
}

/** Counts how often each chunk ran and which threads ran chunks */
class CCountingBody : public CCpuParallelFor::CBody
{
    public:

    explicit CCountingBody( UInt32 chunkCount )
        : CCpuParallelFor::CBody()
        , runs( chunkCount, 0 )
        , threads( chunkCount, 0 )
        , m_lock()
        , m_delayInMs( 0 )
    {
    }

    void SetDelay( UInt32 delayInMs ) { m_delayInMs = delayInMs; }

    virtual void ExecuteChunk( UInt32 chunkIndex, void* scratch, UInt32 scratchSizeInBytes ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( m_delayInMs > 0 )
            MT::PrecisionDelay( m_delayInMs );
        MT::CScopeMutex lock( m_lock );
        ++runs[ chunkIndex ];
        threads[ chunkIndex ] = MT::GetCurrentThreadID();
    }

    UInt32 DistinctThreads( void ) const
    {
        GUCEF::set< TThreadId > distinct;
        for ( size_t i=0; i<threads.size(); ++i )
            distinct.insert( threads[ i ] );
        return static_cast< UInt32 >( distinct.size() );
    }

    bool EachChunkOnce( void ) const
    {
        for ( size_t i=0; i<runs.size(); ++i )
            if ( 1 != runs[ i ] )
                return false;
        return true;
    }

    GUCEF::vector< UInt32 > runs;
    GUCEF::vector< TThreadId > threads;

    private:

    MT::CMutex m_lock;
    UInt32 m_delayInMs;
};

/** Writes into its own slot per chunk without a lock: what a real kernel does */
class CSlotBody : public CCpuParallelFor::CBody
{
    public:

    CSlotBody( UInt32* slots, UInt32 chunkCount ) : CCpuParallelFor::CBody(), m_slots( slots ), m_chunkCount( chunkCount ) {}

    virtual void ExecuteChunk( UInt32 chunkIndex, void* scratch, UInt32 scratchSizeInBytes ) GUCEF_VIRTUAL_OVERRIDE
    {
        // uses the scratch, which is the calling thread's own for the chunk
        UInt32* local = static_cast< UInt32* >( scratch );
        local[ 0 ] = chunkIndex * 3 + 1;
        m_slots[ chunkIndex ] = local[ 0 ];
    }

    private:

    UInt32* m_slots;
    UInt32 m_chunkCount;
};

/** Each chunk runs a nested parallel-for of its own */
class CNestedBody : public CCpuParallelFor::CBody
{
    public:

    CNestedBody( CCpuParallelFor& parallelFor, UInt32 innerChunks )
        : CCpuParallelFor::CBody()
        , total( 0 )
        , failures( 0 )
        , m_parallelFor( parallelFor )
        , m_innerChunks( innerChunks )
    {
    }

    class CInnerBody : public CCpuParallelFor::CBody
    {
        public:
        explicit CInnerBody( MT::CTAtomic< UInt32 >& total ) : CCpuParallelFor::CBody(), m_total( total ) {}
        virtual void ExecuteChunk( UInt32 chunkIndex, void* scratch, UInt32 scratchSizeInBytes ) GUCEF_VIRTUAL_OVERRIDE
        {
            m_total.FetchAdd( chunkIndex + 1 );
        }
        private:
        MT::CTAtomic< UInt32 >& m_total;
    };

    virtual void ExecuteChunk( UInt32 chunkIndex, void* scratch, UInt32 scratchSizeInBytes ) GUCEF_VIRTUAL_OVERRIDE
    {
        CInnerBody inner( total );
        if ( COMPUTERESULT_SUCCESS != m_parallelFor.Run( inner, m_innerChunks, 64 ) )
            failures.FetchAdd( 1 );
    }

    MT::CTAtomic< UInt32 > total;
    MT::CTAtomic< UInt32 > failures;

    private:

    CCpuParallelFor& m_parallelFor;
    UInt32 m_innerChunks;
};

/** Throws in one chunk */
class CThrowingBody : public CCpuParallelFor::CBody
{
    public:
    CThrowingBody( void ) : CCpuParallelFor::CBody(), ran( 0 ) {}
    virtual void ExecuteChunk( UInt32 chunkIndex, void* scratch, UInt32 scratchSizeInBytes ) GUCEF_VIRTUAL_OVERRIDE
    {
        ran.FetchAdd( 1 );
        if ( 3 == chunkIndex )
            throw 3;
    }
    MT::CTAtomic< UInt32 > ran;
};

/** Reduces random data on the CPU device; the per-group partials and the total */
bool
ReduceOnDevice( TComputeDevicePtr device, const GUCEF::vector< Float32 >& data, GUCEF::vector< Float32 >& partials, Float32& total, TComputeExecutionPolicy policy )
{
    const UInt32 count = static_cast< UInt32 >( data.size() );
    const UInt32 groups = ( count + CCpuTestKernels::GroupSize - 1 ) / CCpuTestKernels::GroupSize;
    TComputeKernelPtr reduce;
    if ( !device->CreateKernel( CCpuTestKernels::ReduceSumKernelId, reduce ) )
        return false;
    TComputeBufferPtr in = MakeBuffer( device, count * 4 );
    TComputeBufferPtr out = MakeBuffer( device, groups * 4 );
    CComputeArguments args;
    args.SetBuffer( 0, in );
    args.SetBuffer( 1, out );
    partials.assign( groups, 0.0f );
    CComputeCommandList list;
    list.WriteFromHost( in, 0, &data[ 0 ], count * 4 );
    list.Dispatch( reduce, args, CComputeDispatchSize::ForItems( count ) );
    list.ReadToHost( out, 0, &partials[ 0 ], groups * 4 );
    if ( COMPUTERESULT_SUCCESS != device->GetDefaultQueue()->Execute( list, policy ) )
        return false;
    total = 0.0f;
    for ( UInt32 g=0; g<groups; ++g )
        total += partials[ g ];
    return true;
}

/** The outer run of the worker-count test: the chunk that lands on a worker starts a nested run while the count changes */
MT::CTAtomic< UInt32 > g_workerInChunk( 0 );
MT::CTAtomic< UInt32 > g_countChanging( 0 );
MT::CTAtomic< TThreadId > g_runnerThread( 0 );

class CNestedDuringChangeBody : public CCpuParallelFor::CBody
{
    public:

    explicit CNestedDuringChangeBody( CCpuParallelFor& parallelFor ) : CCpuParallelFor::CBody(), nested( 0 ), m_parallelFor( parallelFor ) {}

    virtual void ExecuteChunk( UInt32 chunkIndex, void* scratch, UInt32 scratchSizeInBytes ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( MT::GetCurrentThreadID() == g_runnerThread.Load() )
        {
            // the runner keeps its chunk until a worker took the other one
            for ( UInt32 waited=0; waited<5000 && 0 == g_workerInChunk.Load(); ++waited )
                MT::PrecisionDelay( 1 );
            return;
        }
        g_workerInChunk.Store( 1 );
        while ( 0 == g_countChanging.Load() )
            MT::PrecisionDelay( 1 );
        MT::PrecisionDelay( 20 );
        CCountingInner inner;
        if ( COMPUTERESULT_SUCCESS == m_parallelFor.Run( inner, 16 ) )
            nested.FetchAdd( 1 );
    }

    class CCountingInner : public CCpuParallelFor::CBody
    {
        public:
        virtual void ExecuteChunk( UInt32 chunkIndex, void* scratch, UInt32 scratchSizeInBytes ) GUCEF_VIRTUAL_OVERRIDE {}
    };

    MT::CTAtomic< UInt32 > nested;

    private:

    CCpuParallelFor& m_parallelFor;
};

class COuterRunBody : public CTestThreadBody
{
    public:
    explicit COuterRunBody( CCpuParallelFor& parallelFor ) : CTestThreadBody(), result( COMPUTERESULT_BACKEND_ERROR ), body( parallelFor ), m_parallelFor( parallelFor ) {}
    virtual void Run( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        g_runnerThread.Store( MT::GetCurrentThreadID() );
        result = m_parallelFor.Run( body, 2 );
    }
    TComputeResult result;
    CNestedDuringChangeBody body;
    private:
    CCpuParallelFor& m_parallelFor;
};

/** Hash dispatches on a shared device from a test thread */
class CSubmitterBody : public CTestThreadBody
{
    public:

    CSubmitterBody( const TComputeDevicePtr& device, UInt32 seed, bool async )
        : CTestThreadBody(), failures( 0 ), m_device( device ), m_seed( seed ), m_async( async )
    {
    }

    virtual void Run( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        TComputeKernelPtr hash;
        if ( !m_device->CreateKernel( CCpuTestKernels::HashKernelId, hash ) )
        {
            failures.FetchAdd( 1 );
            return;
        }
        const UInt32 count = 20000;
        GUCEF::vector< UInt32 > input( count, 0 );
        GUCEF::vector< UInt32 > output( count, 0 );
        UInt32 state = m_seed;
        for ( UInt32 round=0; round<20; ++round )
        {
            for ( UInt32 i=0; i<count; ++i )
            {
                state ^= state << 13; state ^= state >> 17; state ^= state << 5;
                input[ i ] = state;
            }
            CComputeHostBatch::SHostBinding bindings[ 2 ] =
            {
                CComputeHostBatch::SHostBinding::Input( 0, &input[ 0 ], count * 4 ),
                CComputeHostBatch::SHostBinding::Output( 1, &output[ 0 ], count * 4 )
            };
            CCpuTestKernels::SHashConstants constants = { 0, 0, m_seed, 0 };
            CComputeHostBatch batch( m_device, 2 );
            TComputeResult result = COMPUTERESULT_SUCCESS;
            if ( m_async )
            {
                CORE::CFutureResult future = batch.RunAsync( hash, bindings, 2, &constants, sizeof( constants ), CComputeDispatchSize::ForItems( count ) );
                if ( !AwaitFuture( future, 30000 ) )
                    result = COMPUTERESULT_TIMEOUT;
                else
                    result = ComputeResultFromFuture( future );
            }
            else
            {
                result = batch.Run( hash, bindings, 2, &constants, sizeof( constants ), CComputeDispatchSize::ForItems( count ) );
            }
            if ( COMPUTERESULT_SUCCESS != result )
            {
                failures.FetchAdd( 1 );
                continue;
            }
            for ( UInt32 i=0; i<count; ++i )
            {
                if ( output[ i ] != CCpuTestKernels::Hash( input[ i ] ^ m_seed ) )
                {
                    failures.FetchAdd( 1 );
                    break;
                }
            }
        }
    }

    MT::CTAtomic< UInt32 > failures;

    private:

    TComputeDevicePtr m_device;
    UInt32 m_seed;
    bool m_async;
};

/** Direct parallel-for runs from a test thread */
class CParallelRunnerBody : public CTestThreadBody
{
    public:

    explicit CParallelRunnerBody( CCpuParallelFor& parallelFor ) : CTestThreadBody(), failures( 0 ), m_parallelFor( parallelFor ) {}

    virtual void Run( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        GUCEF::vector< UInt32 > slots( 512, 0 );
        for ( UInt32 round=0; round<50; ++round )
        {
            slots.assign( 512, 0 );
            CSlotBody body( &slots[ 0 ], 512 );
            if ( COMPUTERESULT_SUCCESS != m_parallelFor.Run( body, 512, 16 ) )
                failures.FetchAdd( 1 );
            for ( UInt32 i=0; i<512; ++i )
                if ( slots[ i ] != i * 3 + 1 )
                {
                    failures.FetchAdd( 1 );
                    break;
                }
        }
    }

    MT::CTAtomic< UInt32 > failures;

    private:

    CCpuParallelFor& m_parallelFor;
};

}

/*-------------------------------------------------------------------------*/

void
PerformCpuParallelForTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "CpuParallelFor" );

    CCpuParallelFor& parallelFor = CComputeGlobal::Instance()->GetCpuParallelFor();
    const UInt32 defaultWorkers = CCpuParallelFor::GetDefaultWorkerCount();

    GUCEF_TESTFW_TESTCASE( "CpuParallelFor: every chunk runs exactly once, also with no chunks and one chunk" )
    try
    {
        CCountingBody none( 0 );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == parallelFor.Run( none, 0 ) );

        CCountingBody one( 1 );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == parallelFor.Run( one, 1 ) );
        ASSERT_TRUE( one.EachChunkOnce() );
        ASSERT_TRUE( one.threads[ 0 ] == MT::GetCurrentThreadID() );

        CCountingBody many( 10007 );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == parallelFor.Run( many, 10007, 128 ) );
        ASSERT_TRUE( many.EachChunkOnce() );
        ASSERT_TRUE( parallelFor.GetWorkerCount() == defaultWorkers );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CpuParallelFor: the workers take part in a run, calling-thread-only keeps every chunk on the caller" )
    try
    {
        CCountingBody forkJoin( 64 );
        forkJoin.SetDelay( 2 );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == parallelFor.Run( forkJoin, 64 ) );
        ASSERT_TRUE( forkJoin.EachChunkOnce() );
        if ( defaultWorkers > 0 )
        {
            ASSERT_TRUE( forkJoin.DistinctThreads() > 1 );
            ASSERT_TRUE( WaitForRunningWorkers( parallelFor, defaultWorkers ) );
        }

        CCountingBody callerOnly( 64 );
        callerOnly.SetDelay( 1 );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == parallelFor.Run( callerOnly, 64, 0, EXECUTIONPOLICY_CALLING_THREAD_ONLY ) );
        ASSERT_TRUE( callerOnly.EachChunkOnce() );
        ASSERT_TRUE( 1 == callerOnly.DistinctThreads() );
        ASSERT_TRUE( callerOnly.threads[ 0 ] == MT::GetCurrentThreadID() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CpuParallelFor: a reduction is bit-identical for 1, 2 and N workers and on the calling thread only" )
    try
    {
        const UInt32 count = 1000003;
        GUCEF::vector< Float32 > data( count, 0.0f );
        UInt32 state = 2026;
        for ( UInt32 i=0; i<count; ++i )
        {
            state ^= state << 13; state ^= state >> 17; state ^= state << 5;
            data[ i ] = static_cast< Float32 >( state % 2000001 ) * 1.0e-3f - 1000.0f;
        }
        TComputeDevicePtr device = CreateCpuTestDevice();

        GUCEF::vector< Float32 > reference;
        Float32 referenceTotal = 0.0f;
        ASSERT_TRUE( ReduceOnDevice( device, data, reference, referenceTotal, EXECUTIONPOLICY_CALLING_THREAD_ONLY ) );

        const UInt32 workerCounts[ 3 ] = { 1, 2, defaultWorkers > 2 ? defaultWorkers : 3 };
        for ( UInt32 w=0; w<3; ++w )
        {
            parallelFor.SetWorkerCount( workerCounts[ w ] );
            ASSERT_TRUE( parallelFor.GetWorkerCount() == workerCounts[ w ] );
            GUCEF::vector< Float32 > partials;
            Float32 total = 0.0f;
            ASSERT_TRUE( ReduceOnDevice( device, data, partials, total, EXECUTIONPOLICY_DEFAULT ) );
            ASSERT_TRUE( partials.size() == reference.size() );
            ASSERT_TRUE( 0 == memcmp( &partials[ 0 ], &reference[ 0 ], reference.size() * sizeof( Float32 ) ) );
            ASSERT_TRUE( 0 == memcmp( &total, &referenceTotal, sizeof( Float32 ) ) );
            ASSERT_TRUE( WaitForRunningWorkers( parallelFor, workerCounts[ w ] ) );
        }
        parallelFor.SetWorkerCount( 0 );
        ASSERT_TRUE( parallelFor.GetWorkerCount() == defaultWorkers );

        // a Float64 sum of the data agrees with the reduction within the declared tolerance of the kernel
        Float64 exact = 0.0;
        for ( UInt32 i=0; i<count; ++i )
            exact += data[ i ];
        CComputeTolerance tolerance( 0, 0.0, 1.0e-3 );
        ASSERT_TRUE( tolerance.IsWithin( exact, static_cast< Float64 >( referenceTotal ) ) );
    }
    catch( ... )
    {
        parallelFor.SetWorkerCount( 0 );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CpuParallelFor: runs nested in chunks complete without deadlock, also with one worker" )
    try
    {
        const UInt32 workerCounts[ 3 ] = { 1, 2, 0 };
        for ( UInt32 w=0; w<3; ++w )
        {
            parallelFor.SetWorkerCount( workerCounts[ w ] );
            CNestedBody nested( parallelFor, 32 );
            ASSERT_TRUE( COMPUTERESULT_SUCCESS == parallelFor.Run( nested, 48, 64 ) );
            ASSERT_TRUE( 0 == nested.failures.Load() );
            ASSERT_TRUE( 48 * ( 32 * 33 / 2 ) == nested.total.Load() );
        }
        parallelFor.SetWorkerCount( 0 );
    }
    catch( ... )
    {
        parallelFor.SetWorkerCount( 0 );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CpuParallelFor: no allocation per chunk once the run bookkeeping exists" )
    try
    {
        GUCEF::vector< UInt32 > slots( 20000, 0 );
        CSlotBody warmUp( &slots[ 0 ], 20000 );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == parallelFor.Run( warmUp, 20000, 64 ) );

        CSlotBody body( &slots[ 0 ], 20000 );
        StartCounting();
        const TComputeResult result = parallelFor.Run( body, 20000, 64 );
        const UInt32 allocations = StopCounting();
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == result );
        // a worker that took no chunk of the warm-up grows its scratch area once (counted: it is aligned memory through
        // operator new), and other threads of the process may allocate meanwhile; one allocation per chunk would be 20000
        ASSERT_TRUE( allocations <= parallelFor.GetWorkerCount() + 8 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CpuParallelFor test: " << allocations << " allocations during a run of 20000 chunks" );

        // and the counter counts, also the aligned memory of buffers and scratch areas
        StartCounting();
        void* probe = ::operator new( 16 );
        const UInt32 counted = StopCounting();
        ::operator delete( probe );
        ASSERT_TRUE( counted >= 1 );
        StartCounting();
        void* aligned = AlignedAllocate( 1000, 64 );
        const UInt32 alignedCounted = StopCounting();
        AlignedFree( aligned );
        ASSERT_TRUE( 1 == alignedCounted );

        // a scratch area larger than any before grows once, later runs of that size do not allocate
        CSlotBody bigScratch( &slots[ 0 ], 20000 );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == parallelFor.Run( bigScratch, 20000, 64 * 1024 ) );
        StartCounting();
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == parallelFor.Run( bigScratch, 20000, 64 * 1024 ) );
        const UInt32 scratchAllocations = StopCounting();
        ASSERT_TRUE( scratchAllocations <= 2 * ( parallelFor.GetWorkerCount() + 1 ) );
    }
    catch( ... )
    {
        StopCounting();
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CpuParallelFor: the worker count changes while a worker chunk starts a nested run, without deadlock" )
    try
    {
        parallelFor.SetWorkerCount( 2 );
        g_workerInChunk.Store( 0 );
        g_countChanging.Store( 0 );
        COuterRunBody outer( parallelFor );
        CTestThread thread;
        ASSERT_TRUE( thread.Start( &outer ) );
        bool workerTookChunk = false;
        for ( UInt32 waited=0; waited<5000 && !workerTookChunk; ++waited )
        {
            workerTookChunk = 0 != g_workerInChunk.Load();
            if ( !workerTookChunk )
                MT::PrecisionDelay( 1 );
        }
        ASSERT_TRUE( workerTookChunk );
        g_countChanging.Store( 1 );
        parallelFor.SetWorkerCount( 3 );        // stops the worker that is in the chunk, which needs the worker lock for its nested run
        ASSERT_TRUE( thread.Join( 20000 ) );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == outer.result );
        ASSERT_TRUE( 1 == outer.body.nested.Load() );
        ASSERT_TRUE( 3 == parallelFor.GetWorkerCount() );
        parallelFor.SetWorkerCount( 0 );
    }
    catch( ... )
    {
        g_countChanging.Store( 1 );
        parallelFor.SetWorkerCount( 0 );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CpuParallelFor: a throwing chunk fails the run, the other chunks still run" )
    try
    {
        CThrowingBody body;
        ASSERT_TRUE( COMPUTERESULT_BACKEND_ERROR == parallelFor.Run( body, 100 ) );
        ASSERT_TRUE( 100 == body.ran.Load() );
        CThrowingBody callerOnly;
        ASSERT_TRUE( COMPUTERESULT_BACKEND_ERROR == parallelFor.Run( callerOnly, 10, 0, EXECUTIONPOLICY_CALLING_THREAD_ONLY ) );
        ASSERT_TRUE( 10 == callerOnly.ran.Load() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CpuParallelFor: concurrent submitters, direct runs and dispatches, sync and async, all give correct results" )
    try
    {
        TComputeDevicePtr device = CreateCpuTestDevice();
        CParallelRunnerBody runnerA( parallelFor );
        CParallelRunnerBody runnerB( parallelFor );
        CSubmitterBody syncA( device, 11, false );
        CSubmitterBody syncB( device, 22, false );
        CSubmitterBody asyncA( device, 33, true );
        CSubmitterBody asyncB( device, 44, true );
        CTestThreadBody* bodies[ 6 ] = { &runnerA, &runnerB, &syncA, &syncB, &asyncA, &asyncB };
        ASSERT_TRUE( RunOnTestThreads( bodies, 6, 120000 ) );
        ASSERT_TRUE( 0 == runnerA.failures.Load() && 0 == runnerB.failures.Load() );
        ASSERT_TRUE( 0 == syncA.failures.Load() && 0 == syncB.failures.Load() );
        ASSERT_TRUE( 0 == asyncA.failures.Load() && 0 == asyncB.failures.Load() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CpuParallelFor: the worker count follows the settings, 0 is the logical CPUs minus one" )
    try
    {
        CComputeSettings settings = CComputeGlobal::Instance()->GetSettings();
        settings.SetMaxCpuWorkerCount( 3 );
        CComputeGlobal::Instance()->ApplySettings( settings );
        ASSERT_TRUE( 3 == parallelFor.GetWorkerCount() );
        CCountingBody body( 200 );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == parallelFor.Run( body, 200 ) );
        ASSERT_TRUE( body.EachChunkOnce() );
        ASSERT_TRUE( WaitForRunningWorkers( parallelFor, 3 ) );

        settings.SetMaxCpuWorkerCount( 0 );
        CComputeGlobal::Instance()->ApplySettings( settings );
        ASSERT_TRUE( defaultWorkers == parallelFor.GetWorkerCount() );

        parallelFor.StopWorkers();
        ASSERT_TRUE( 0 == parallelFor.GetRunningWorkerCount() );
        CCountingBody restart( 50 );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == parallelFor.Run( restart, 50 ) );
        ASSERT_TRUE( restart.EachChunkOnce() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
