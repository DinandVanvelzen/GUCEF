/*
 *  gucefMATH_TestApp: GUCEF test application for the Math module
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
#if defined( __linux__ )
#include <unistd.h>
#endif

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_MATH_POINTS_H
#include "gucefMATH_points.h"
#define GUCEF_MATH_POINTS_H
#endif /* GUCEF_MATH_POINTS_H ? */

#ifndef GUCEF_MATH_CMATHBATCHDISPATCHER_H
#include "gucefMATH_CMathBatchDispatcher.h"
#define GUCEF_MATH_CMATHBATCHDISPATCHER_H
#endif /* GUCEF_MATH_CMATHBATCHDISPATCHER_H ? */

#ifndef GUCEF_MATH_BATCHFUNCTIONS_H
#include "gucefMATH_BatchFunctions.h"
#define GUCEF_MATH_BATCHFUNCTIONS_H
#endif /* GUCEF_MATH_BATCHFUNCTIONS_H ? */

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

#ifndef GUCEF_TEST_CHILD_PROCESS_H
#include "gucef_test_child_process.h"
#define GUCEF_TEST_CHILD_PROCESS_H
#endif /* GUCEF_TEST_CHILD_PROCESS_H ? */

#include "TestMathAllocationCounter.h"
#include "TestMathReference.h"
#include "TestBatchDispatcher.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::MATH;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** WebAssembly builds without pthreads run everything on one thread: tests that start threads are skipped there */
bool
SingleThreadedPlatform( void )
{
    #if defined( __EMSCRIPTEN__ ) && !defined( __EMSCRIPTEN_PTHREADS__ )
    return true;
    #else
    return false;
    #endif
}

const UInt64 DispatcherSeed = 0xD15BA7C40005ULL;
const TMathBatchKernel PointsKernel = MATHBATCHKERNEL_TRANSFORM_POINTS3_F32;
const bool BuildAllowsMulticore = GUCEF_MATH_BATCH_MAX_TIER_LEVEL >= 2;
const bool BuildAllowsAccelerator = GUCEF_MATH_BATCH_MAX_TIER_LEVEL >= 3;
const UInt32 ThreadWaitLimitInMs = 600000;

/** A provider for the tests: counts its calls, fails with 'status' or does the work of the scalar provider */
struct SFakeProvider
{
    MT::CTAtomic< UInt32 > calls;
    TMathStatus status;
    TMathTransformPoints3F32Function work;

    SFakeProvider( void ) : calls( 0 ), status( MATHSTATUS_OK ), work( GUCEF_NULL ) {}
};

TMathStatus
FakeTransformPoints( void* context, const SMathTransformPoints3F32Args& args )
{
    SFakeProvider* fake = static_cast< SFakeProvider* >( context );
    fake->calls.FetchAdd( 1 );
    if ( MATHSTATUS_OK != fake->status )
        return fake->status;
    return fake->work( GUCEF_NULL, args );
}

/*-------------------------------------------------------------------------*/

SMathBatchProvider
MakeProvider( TMathBatchTier tier, const char* name, SFakeProvider& fake, UInt32 flags, UInt32 minCount = 0 )
{
    SMathBatchProvider provider;
    provider.tier = tier;
    provider.name = name;
    provider.function = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathTransformPoints3F32Function >( &FakeTransformPoints ) );
    provider.minCount = minCount;
    provider.maxCount = 0xFFFFFFFF;
    provider.flags = flags;
    provider.alignment = 0;
    provider.requiredCpuFeatures = 0;
    provider.context = &fake;
    return provider;
}

/*-------------------------------------------------------------------------*/

/** The scalar provider function of TransformPoints3 of a dispatcher with the built-in providers */
TMathTransformPoints3F32Function
ScalarTransformPoints( const CMathBatchDispatcher& dispatcher )
{
    SMathBatchProvider scalar;
    dispatcher.GetProvider( PointsKernel, dispatcher.GetProviderCount( PointsKernel ) - 1, scalar );
    return reinterpret_cast< TMathTransformPoints3F32Function >( scalar.function );
}

/*-------------------------------------------------------------------------*/

/** Seeded points, a matrix and the expected result (computed with the value type) */
struct STransformCase
{
    TFloat32Matrix4D matrix;
    GUCEF::vector< TFloat32Point3D > in;
    GUCEF::vector< TFloat32Point3D > expected;

    STransformCase( UInt32 count, UInt64 seed )
        : matrix()
        , in( count )
        , expected( count )
    {
        MathReference::CSeededSequence sequence( seed );
        Float32 values[ 16 ];
        for ( UInt32 i=0; i<16; ++i )
        {
            const Float64 value = sequence.NextFloat64( -2.0, 2.0 );
            values[ i ] = (Float32) value;
        }
        matrix = TFloat32Matrix4D::FromColumnMajor( values );
        for ( UInt32 i=0; i<count; ++i )
        {
            const Float64 x = sequence.NextFloat64( -50.0, 50.0 );
            const Float64 y = sequence.NextFloat64( -50.0, 50.0 );
            const Float64 z = sequence.NextFloat64( -50.0, 50.0 );
            in[ i ] = TFloat32Point3D( (Float32) x, (Float32) y, (Float32) z );
            expected[ i ] = matrix.TransformPoint( in[ i ] );
        }
    }

    /** Runs the kernel through the dispatcher into 'out' and returns the status; true in 'correct' when out is the expected result */
    TMathStatus Run( const CMathBatchDispatcher& dispatcher, UInt32 flags, GUCEF::vector< TFloat32Point3D >& out, bool& correct ) const
    {
        const UInt32 count = (UInt32) in.size();
        out.assign( count, TFloat32Point3D() );
        SMathTransformPoints3F32Args args;
        args.matrix = &matrix;
        args.in = &in[ 0 ];
        args.inStrideBytes = (UInt32) sizeof( TFloat32Point3D );
        args.out = &out[ 0 ];
        args.outStrideBytes = (UInt32) sizeof( TFloat32Point3D );
        args.count = count;
        const TMathStatus status = dispatcher.Execute( PointsKernel, args, count, flags, args.in, args.out );
        correct = 0 == memcmp( &out[ 0 ], &expected[ 0 ], count * sizeof( TFloat32Point3D ) );
        return status;
    }
};

/*-------------------------------------------------------------------------*/

/** Runs a function of the test on its own thread (as in the atomic tests of gucefMT) */
class CDispatcherTestThread : public MT::CActiveObject
{
    public:

    typedef void ( *TFunction )( void* argument );

    CDispatcherTestThread( TFunction function, void* argument )
        : MT::CActiveObject()
        , m_function( function )
        , m_argument( argument )
        , m_done( false )
    {}

    virtual bool OnThreadStart( void* /* taskdata */ ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        return true;
    }

    virtual bool OnThreadCycle( void* /* taskdata */ ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        m_function( m_argument );
        m_done.Store( true, MT::MEMORDER_RELEASE );
        return true; // stop cycling
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    bool IsDone( void ) const { return m_done.Load( MT::MEMORDER_ACQUIRE ); }

    private:

    TFunction m_function;
    void* m_argument;
    MT::CTAtomic< bool > m_done;
};

/*-------------------------------------------------------------------------*/

/** Shared by the threads of the concurrency test */
struct SConcurrencyState
{
    const CMathBatchDispatcher* dispatcher;
    CMathBatchDispatcher* mutableDispatcher;
    const STransformCase* transformCase;
    SFakeProvider* multicore;
    SFakeProvider* accelerator;
    MT::CTAtomic< UInt32 > registrationDone;
    MT::CTAtomic< UInt32 > wrongResults;
    MT::CTAtomic< UInt32 > failedCalls;
    MT::CTAtomic< UInt32 > callCount;
    MT::CTAtomic< UInt32 > registrationFailures;
    UInt32 registrationCycles;

    SConcurrencyState( void )
        : dispatcher( GUCEF_NULL ), mutableDispatcher( GUCEF_NULL ), transformCase( GUCEF_NULL ), multicore( GUCEF_NULL ), accelerator( GUCEF_NULL )
        , registrationDone( 0 ), wrongResults( 0 ), failedCalls( 0 ), callCount( 0 ), registrationFailures( 0 ), registrationCycles( 0 )
    {}
};

void
CallerThread( void* argument )
{
    SConcurrencyState* state = static_cast< SConcurrencyState* >( argument );
    GUCEF::vector< TFloat32Point3D > out;
    UInt32 round = 0;
    // at least 200 calls, and on until the registering thread is done
    while ( round < 200 || 0 == state->registrationDone.Load( MT::MEMORDER_ACQUIRE ) )
    {
        const UInt32 flags = 0 == round % 3 ? MATHBATCH_REALTIME : ( 1 == round % 3 ? 0 : MATHBATCH_NO_ACCELERATOR );
        bool correct = false;
        const TMathStatus status = state->transformCase->Run( *state->dispatcher, flags, out, correct );
        if ( MATHSTATUS_OK != status )
            state->failedCalls.FetchAdd( 1 );
        if ( !correct )
            state->wrongResults.FetchAdd( 1 );
        state->callCount.FetchAdd( 1 );
        ++round;
    }
}

void
RegistrationThread( void* argument )
{
    SConcurrencyState* state = static_cast< SConcurrencyState* >( argument );
    const int ownerA = 0;
    const int ownerB = 0;
    for ( UInt32 i=0; i<state->registrationCycles; ++i )
    {
        // the accelerator fails every call (MAY_FAIL), so calls that reach it continue on the multi-core provider
        const SMathBatchProvider multicore = MakeProvider( MATHBATCHTIER_MULTICORE, "FakeMulticore", *state->multicore, MATHPROVIDER_BITEXACT | MATHPROVIDER_MAY_FAIL );
        const SMathBatchProvider accelerator = MakeProvider( MATHBATCHTIER_ACCELERATOR, "FakeAccelerator", *state->accelerator, MATHPROVIDER_MAY_FAIL );
        bool ok = state->mutableDispatcher->RegisterProvider( PointsKernel, multicore, &ownerA );
        ok = state->mutableDispatcher->RegisterProvider( PointsKernel, accelerator, &ownerB ) && ok;
        ok = 1 == state->mutableDispatcher->UnregisterProviders( &ownerB ) && ok;
        ok = 1 == state->mutableDispatcher->UnregisterProviders( &ownerA ) && ok;
        if ( !ok )
            state->registrationFailures.FetchAdd( 1 );
    }
    state->registrationDone.Store( 1, MT::MEMORDER_RELEASE );
}

/*-------------------------------------------------------------------------*/

/** Starts one thread per function and waits for all of them; false on a start failure or time-out */
bool
RunThreads( CDispatcherTestThread::TFunction* functions, UInt32 count, void* argument )
{
    GUCEF::vector< CDispatcherTestThread* > threads;
    bool allStarted = true;
    for ( UInt32 i=0; i<count; ++i )
    {
        threads.push_back( GUCEF_NEW CDispatcherTestThread( functions[ i ], argument ) );
        allStarted = threads.back()->Activate( GUCEF_NULL, 0, 0 ) && allStarted;
    }
    UInt32 waitedInMs = 0;
    bool allDone = false;
    while ( !allDone && waitedInMs < ThreadWaitLimitInMs )
    {
        allDone = true;
        for ( UInt32 i=0; i<count; ++i )
            allDone = threads[ i ]->IsDone() && allDone;
        if ( !allDone )
        {
            MT::PrecisionDelay( 2 );
            waitedInMs += 2;
        }
    }
    for ( UInt32 i=0; i<count; ++i )
    {
        UInt32 stopWaitInMs = 0;
        while ( threads[ i ]->IsDone() && threads[ i ]->IsActive() && stopWaitInMs < 5000 )
        {
            MT::PrecisionDelay( 1 );
            ++stopWaitInMs;
        }
        if ( threads[ i ]->IsDone() && !threads[ i ]->IsActive() )
            GUCEF_DELETE threads[ i ];
        // a thread that did not finish is leaked on purpose: deleting a running object is worse than the leak after a failed test
    }
    return allStarted && allDone;
}

/*-------------------------------------------------------------------------*/

/** A provider that stays inside until the unregistering thread has started UnregisterProviders, then a while longer */
struct SDrainState
{
    CMathBatchDispatcher* dispatcher;
    TMathTransformPoints3F32Function work;
    MT::CTAtomic< UInt32 > inside;
    MT::CTAtomic< UInt32 > unregisterStarted;
    MT::CTAtomic< UInt32 > left;
    MT::CTAtomic< UInt32 > leftWhenUnregisterReturned;
    MT::CTAtomic< UInt32 > removed;
    MT::CTAtomic< UInt32 > callStatus;
    const STransformCase* transformCase;

    SDrainState( void )
        : dispatcher( GUCEF_NULL ), work( GUCEF_NULL ), inside( 0 ), unregisterStarted( 0 ), left( 0 ), leftWhenUnregisterReturned( 0 )
        , removed( 0 ), callStatus( 0xFFFF ), transformCase( GUCEF_NULL )
    {}
};

TMathStatus
BlockingTransformPoints( void* context, const SMathTransformPoints3F32Args& args )
{
    SDrainState* state = static_cast< SDrainState* >( context );
    state->inside.Store( 1, MT::MEMORDER_RELEASE );
    UInt32 waited = 0;
    while ( 0 == state->unregisterStarted.Load( MT::MEMORDER_ACQUIRE ) && waited < 60000 )
    {
        MT::PrecisionDelay( 1 );
        ++waited;
    }
    MT::PrecisionDelay( 50 );       // the plugin code is still running while UnregisterProviders runs
    const TMathStatus status = state->work( GUCEF_NULL, args );
    state->left.Store( 1, MT::MEMORDER_RELEASE );
    return status;
}

void
DrainCallerThread( void* argument )
{
    SDrainState* state = static_cast< SDrainState* >( argument );
    GUCEF::vector< TFloat32Point3D > out;
    bool correct = false;
    const TMathStatus status = state->transformCase->Run( *state->dispatcher, 0, out, correct );
    state->callStatus.Store( correct ? (UInt32) status : 0xFFFE, MT::MEMORDER_RELEASE );
}

void
DrainUnregisterThread( void* argument )
{
    SDrainState* state = static_cast< SDrainState* >( argument );
    UInt32 waited = 0;
    while ( 0 == state->inside.Load( MT::MEMORDER_ACQUIRE ) && waited < 60000 )
    {
        MT::PrecisionDelay( 1 );
        ++waited;
    }
    state->unregisterStarted.Store( 1, MT::MEMORDER_RELEASE );
    state->removed.Store( state->dispatcher->UnregisterProviders( state ), MT::MEMORDER_RELEASE );
    state->leftWhenUnregisterReturned.Store( state->left.Load( MT::MEMORDER_ACQUIRE ), MT::MEMORDER_RELEASE );
}

/*-------------------------------------------------------------------------*/

/** The threads of the child process of the concurrent first batch call */
struct SFirstCallState
{
    MT::CTAtomic< UInt32 > arrived;
    MT::CTAtomic< UInt32 > wrong;
    UInt32 threadCount;

    SFirstCallState( void ) : arrived( 0 ), wrong( 0 ), threadCount( 0 ) {}
};

void
FirstCallThread( void* argument )
{
    SFirstCallState* state = static_cast< SFirstCallState* >( argument );
    const STransformCase transformCase( 64, DispatcherSeed + 77 );         // value types only: CMathGlobal is not touched yet
    GUCEF::vector< TFloat32Point3D > out( 64 );

    // all threads make the first batch call of the process at the same moment
    state->arrived.FetchAdd( 1 );
    UInt32 spins = 0;
    while ( state->arrived.Load() < state->threadCount && spins < 2000000000u )
    {
        MT::AtomicCpuRelax();
        ++spins;
    }
    TransformPoints( transformCase.matrix, &transformCase.in[ 0 ], &out[ 0 ], 64 );
    if ( 0 != memcmp( &out[ 0 ], &transformCase.expected[ 0 ], 64 * sizeof( TFloat32Point3D ) ) )
        state->wrong.FetchAdd( 1 );
}

/*-------------------------------------------------------------------------*/

/** A CPU feature bit this CPU does not report, 0 when it reports all of them */
UInt64
MissingCpuFeature( UInt64 features )
{
    for ( UInt32 bit=0; bit<64; ++bit )
    {
        const UInt64 feature = (UInt64) 1 << bit;
        if ( 0 != ( feature & GUCEF_CPU_ISA_ALL_KNOWN ) && 0 == ( feature & features ) )
            return feature;
    }
    return 0;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformBatchDispatcherTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "BatchDispatcher" );

    GUCEF_TESTFW_TESTCASE( "BatchDispatcher: the tiers equal those of gucefCOMPUTE, kernel ids and invalid registrations" )
    try
    {
        // the values that gucefCOMPUTE's TComputeProviderTier mirrors one to one
        ASSERT_TRUE( 0 == MATHBATCHTIER_SCALAR && 1 == MATHBATCHTIER_SIMD && 2 == MATHBATCHTIER_MULTICORE && 3 == MATHBATCHTIER_ACCELERATOR );
        ASSERT_TRUE( 256 == MATHBATCHKERNEL_EXTERNAL_FIRST && 511 == MATHBATCHKERNEL_EXTERNAL_LAST && 319 == MATHBATCHKERNEL_GEOMETRY_LAST );
        // the ranges reserved for other modules: GEOMETRY 256 to 319 and PHYSICS 320 to 383 follow each other without overlap,
        // lie inside the external range and above every MATH kernel id
        ASSERT_TRUE( 256 == MATHBATCHKERNEL_GEOMETRY_FIRST && 320 == MATHBATCHKERNEL_PHYSICS_FIRST && 383 == MATHBATCHKERNEL_PHYSICS_LAST );
        ASSERT_TRUE( MATHBATCHKERNEL_GEOMETRY_FIRST <= MATHBATCHKERNEL_GEOMETRY_LAST && MATHBATCHKERNEL_GEOMETRY_LAST < MATHBATCHKERNEL_PHYSICS_FIRST );
        ASSERT_TRUE( MATHBATCHKERNEL_PHYSICS_FIRST <= MATHBATCHKERNEL_PHYSICS_LAST && MATHBATCHKERNEL_PHYSICS_LAST <= MATHBATCHKERNEL_EXTERNAL_LAST );
        ASSERT_TRUE( MATHBATCHKERNEL_MATH_COUNT <= MATHBATCHKERNEL_GEOMETRY_FIRST && MATHBATCHKERNEL_GEOMETRY_FIRST >= MATHBATCHKERNEL_EXTERNAL_FIRST );
        ASSERT_TRUE( MATHBATCHKERNEL_PHYSICS_LAST < MATHBATCHKERNEL_SLOT_COUNT );

        CMathBatchDispatcher dispatcher;
        SFakeProvider fake;
        const int owner = 0;
        ASSERT_TRUE( 0 == dispatcher.GetProviderCount( PointsKernel ) );
        ASSERT_TRUE( GUCEF_NULL == dispatcher.SelectProvider( PointsKernel, 10, 0, GUCEF_NULL, GUCEF_NULL ) );
        ASSERT_TRUE( GUCEF_NULL == dispatcher.GetKernelId( MATHBATCHKERNEL_MATH_COUNT ) && GUCEF_NULL == dispatcher.GetKernelId( (TMathBatchKernel) 300 ) );
        ASSERT_TRUE( GUCEF_NULL == dispatcher.GetKernelId( (TMathBatchKernel) 600 ) );
        ASSERT_TRUE( 0 == strcmp( "MATH.NormalizeVectors3.F32", dispatcher.GetKernelId( MATHBATCHKERNEL_NORMALIZE_VECTORS3_F32 ) ) );

        SMathBatchProvider provider = MakeProvider( MATHBATCHTIER_MULTICORE, "Fake", fake, MATHPROVIDER_BITEXACT );
        ASSERT_FALSE( dispatcher.RegisterProvider( MATHBATCHKERNEL_MATH_COUNT, provider, &owner ) );        // unknown kernel
        ASSERT_FALSE( dispatcher.RegisterProvider( (TMathBatchKernel) 300, provider, &owner ) );            // external, not registered
        ASSERT_FALSE( dispatcher.RegisterProvider( PointsKernel, provider, GUCEF_NULL ) );                 // no owner
        SMathBatchProvider invalid = provider;
        invalid.function = GUCEF_NULL;
        ASSERT_FALSE( dispatcher.RegisterProvider( PointsKernel, invalid, &owner ) );
        invalid = provider;
        invalid.tier = (TMathBatchTier) 4;
        ASSERT_FALSE( dispatcher.RegisterProvider( PointsKernel, invalid, &owner ) );
        invalid = provider;
        invalid.minCount = 10;
        invalid.maxCount = 9;
        ASSERT_FALSE( dispatcher.RegisterProvider( PointsKernel, invalid, &owner ) );
        invalid = provider;
        invalid.flags |= MATHPROVIDER_REQUIRES_ALIGNED_INPUT;
        invalid.alignment = 24;
        ASSERT_FALSE( dispatcher.RegisterProvider( PointsKernel, invalid, &owner ) );
        const UInt64 missing = MissingCpuFeature( dispatcher.GetCpuIsaFeatures() );
        if ( 0 != missing )
        {
            invalid = provider;
            invalid.requiredCpuFeatures = missing;
            ASSERT_FALSE( dispatcher.RegisterProvider( PointsKernel, invalid, &owner ) );
        }
        // a full list
        for ( UInt32 i=0; i<CMathBatchDispatcher::MaxProvidersPerKernel; ++i )
            ASSERT_TRUE( dispatcher.RegisterProvider( PointsKernel, provider, &owner ) );
        ASSERT_FALSE( dispatcher.RegisterProvider( PointsKernel, provider, &owner ) );
        ASSERT_TRUE( CMathBatchDispatcher::MaxProvidersPerKernel == dispatcher.UnregisterProviders( &owner ) );
        ASSERT_TRUE( 0 == dispatcher.GetProviderCount( PointsKernel ) );
        ASSERT_TRUE( 0 == dispatcher.UnregisterProviders( GUCEF_NULL ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BatchDispatcher: the ids reserved for gucefGEOMETRY and gucefPHYSICS register at both ends of their ranges and do not collide" )
    try
    {
        CMathBatchDispatcher dispatcher;
        const int geometryOwner = 0;
        const int physicsOwner = 0;
        ASSERT_TRUE( dispatcher.RegisterExternalKernel( MATHBATCHKERNEL_GEOMETRY_FIRST, "GEOMETRY.TestFirst", false, &geometryOwner ) );
        ASSERT_TRUE( dispatcher.RegisterExternalKernel( MATHBATCHKERNEL_GEOMETRY_LAST, "GEOMETRY.TestLast", false, &geometryOwner ) );
        ASSERT_TRUE( dispatcher.RegisterExternalKernel( MATHBATCHKERNEL_PHYSICS_FIRST, "PHYSICS.TestFirst", false, &physicsOwner ) );
        ASSERT_TRUE( dispatcher.RegisterExternalKernel( MATHBATCHKERNEL_PHYSICS_LAST, "PHYSICS.TestLast", true, &physicsOwner ) );
        ASSERT_TRUE( 0 == strcmp( "GEOMETRY.TestLast", dispatcher.GetKernelId( MATHBATCHKERNEL_GEOMETRY_LAST ) ) );
        ASSERT_TRUE( 0 == strcmp( "PHYSICS.TestFirst", dispatcher.GetKernelId( MATHBATCHKERNEL_PHYSICS_FIRST ) ) );
        ASSERT_TRUE( dispatcher.IsKernelAlwaysDeterministic( MATHBATCHKERNEL_PHYSICS_LAST ) && !dispatcher.IsKernelAlwaysDeterministic( MATHBATCHKERNEL_PHYSICS_FIRST ) );
        // an id registered by one range cannot be taken again under a name of the other range
        ASSERT_FALSE( dispatcher.RegisterExternalKernel( MATHBATCHKERNEL_PHYSICS_FIRST, "GEOMETRY.TestLast", false, &physicsOwner ) );
        ASSERT_FALSE( dispatcher.RegisterExternalKernel( MATHBATCHKERNEL_GEOMETRY_LAST, "PHYSICS.TestFirst", false, &geometryOwner ) );
        // the ids just outside the reserved ranges still belong to the external range, MATH ids are never external
        ASSERT_TRUE( GUCEF_NULL == dispatcher.GetKernelId( (TMathBatchKernel) ( (UInt32) MATHBATCHKERNEL_PHYSICS_LAST + 1 ) ) );
        ASSERT_FALSE( dispatcher.RegisterExternalKernel( (TMathBatchKernel) ( (UInt32) MATHBATCHKERNEL_MATH_COUNT - 1 ), "PHYSICS.NotExternal", false, &physicsOwner ) );
        dispatcher.UnregisterProviders( &physicsOwner );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BatchDispatcher: realtime calls never reach the multi-core or accelerator tier, the call flags and caps select the tier" )
    try
    {
        CMathBatchDispatcher dispatcher;
        ASSERT_TRUE( dispatcher.RegisterBuiltInProviders() >= MATHBATCHKERNEL_MATH_COUNT );
        const UInt32 builtIn = dispatcher.GetProviderCount( PointsKernel );
        SFakeProvider multicore;
        SFakeProvider accelerator;
        multicore.work = ScalarTransformPoints( dispatcher );
        accelerator.work = multicore.work;
        const int owner = 0;
        ASSERT_TRUE( dispatcher.RegisterProvider( PointsKernel, MakeProvider( MATHBATCHTIER_ACCELERATOR, "FakeAccelerator", accelerator, MATHPROVIDER_BITEXACT ), &owner ) );
        ASSERT_TRUE( dispatcher.RegisterProvider( PointsKernel, MakeProvider( MATHBATCHTIER_MULTICORE, "FakeMulticore", multicore, MATHPROVIDER_BITEXACT ), &owner ) );
        ASSERT_TRUE( builtIn + 2 == dispatcher.GetProviderCount( PointsKernel ) );

        // the list is in descending tier
        SMathBatchProvider first;
        SMathBatchProvider second;
        ASSERT_TRUE( dispatcher.GetProvider( PointsKernel, 0, first ) && dispatcher.GetProvider( PointsKernel, 1, second ) );
        ASSERT_TRUE( MATHBATCHTIER_ACCELERATOR == first.tier && MATHBATCHTIER_MULTICORE == second.tier );

        const STransformCase transformCase( 1000, DispatcherSeed );
        GUCEF::vector< TFloat32Point3D > out;
        bool correct = false;

        // realtime: 1000 calls, none reaches the fakes
        for ( UInt32 i=0; i<1000; ++i )
        {
            ASSERT_TRUE( MATHSTATUS_OK == transformCase.Run( dispatcher, MATHBATCH_REALTIME, out, correct ) && correct );
            ASSERT_TRUE( MATHBATCHTIER_SIMD >= dispatcher.SelectProvider( PointsKernel, 1000, MATHBATCH_REALTIME, GUCEF_NULL, GUCEF_NULL )->tier );
        }
        ASSERT_TRUE( 0 == multicore.calls.Load() && 0 == accelerator.calls.Load() );
        ASSERT_TRUE( MATHSTATUS_OK == transformCase.Run( dispatcher, MATHBATCH_REALTIME | MATHBATCH_NO_MULTICORE | MATHBATCH_NO_ACCELERATOR, out, correct ) && correct );
        ASSERT_TRUE( 0 == multicore.calls.Load() && 0 == accelerator.calls.Load() );

        // no flags: the highest tier the build allows
        ASSERT_TRUE( MATHSTATUS_OK == transformCase.Run( dispatcher, 0, out, correct ) && correct );
        ASSERT_TRUE( ( BuildAllowsAccelerator ? 1u : 0u ) == accelerator.calls.Load() );
        ASSERT_TRUE( ( !BuildAllowsAccelerator && BuildAllowsMulticore ? 1u : 0u ) == multicore.calls.Load() );

        // NO_ACCELERATOR, NO_MULTICORE, FORCE_SCALAR
        const TMathBatchTier noAccelerator = dispatcher.SelectProvider( PointsKernel, 1000, MATHBATCH_NO_ACCELERATOR, GUCEF_NULL, GUCEF_NULL )->tier;
        ASSERT_TRUE( BuildAllowsMulticore ? MATHBATCHTIER_MULTICORE == noAccelerator : MATHBATCHTIER_SIMD >= noAccelerator );
        const TMathBatchTier noMulticore = dispatcher.SelectProvider( PointsKernel, 1000, MATHBATCH_NO_MULTICORE, GUCEF_NULL, GUCEF_NULL )->tier;
        ASSERT_TRUE( BuildAllowsAccelerator ? MATHBATCHTIER_ACCELERATOR == noMulticore : MATHBATCHTIER_SIMD >= noMulticore );
        ASSERT_TRUE( MATHBATCHTIER_SIMD >= dispatcher.SelectProvider( PointsKernel, 1000, MATHBATCH_NO_MULTICORE | MATHBATCH_NO_ACCELERATOR, GUCEF_NULL, GUCEF_NULL )->tier );
        ASSERT_TRUE( MATHBATCHTIER_SCALAR == dispatcher.SelectProvider( PointsKernel, 1000, MATHBATCH_FORCE_SCALAR, GUCEF_NULL, GUCEF_NULL )->tier );
        ASSERT_TRUE( MATHBATCHTIER_SCALAR == dispatcher.SelectProvider( PointsKernel, 1000, MATHBATCH_FORCE_SCALAR | MATHBATCH_REALTIME, GUCEF_NULL, GUCEF_NULL )->tier );

        // the application cap, the kernel cap and the build cap
        ASSERT_TRUE( (UInt32) dispatcher.GetMaxTier() == (UInt32) GUCEF_MATH_BATCH_MAX_TIER_LEVEL );
        dispatcher.SetMaxTier( MATHBATCHTIER_SIMD );
        ASSERT_TRUE( MATHBATCHTIER_SIMD >= dispatcher.GetMaxTier() && MATHBATCHTIER_SIMD >= dispatcher.GetMaxTier( PointsKernel ) );
        const UInt32 multicoreBefore = multicore.calls.Load();
        const UInt32 acceleratorBefore = accelerator.calls.Load();
        for ( UInt32 i=0; i<10; ++i )
            ASSERT_TRUE( MATHSTATUS_OK == transformCase.Run( dispatcher, 0, out, correct ) && correct );
        ASSERT_TRUE( multicoreBefore == multicore.calls.Load() && acceleratorBefore == accelerator.calls.Load() );
        dispatcher.SetMaxTier( MATHBATCHTIER_ACCELERATOR );
        dispatcher.SetMaxTier( PointsKernel, MATHBATCHTIER_MULTICORE );
        ASSERT_TRUE( MATHBATCHTIER_MULTICORE >= dispatcher.GetMaxTier( PointsKernel ) );
        ASSERT_TRUE( (UInt32) dispatcher.GetMaxTier( MATHBATCHKERNEL_DOT_PRODUCTS3_F32 ) == (UInt32) GUCEF_MATH_BATCH_MAX_TIER_LEVEL );
        const TMathBatchTier capped = dispatcher.SelectProvider( PointsKernel, 1000, 0, GUCEF_NULL, GUCEF_NULL )->tier;
        ASSERT_TRUE( BuildAllowsMulticore ? MATHBATCHTIER_MULTICORE == capped : MATHBATCHTIER_SIMD >= capped );
        dispatcher.SetMaxTier( PointsKernel, MATHBATCHTIER_SCALAR );
        ASSERT_TRUE( MATHBATCHTIER_SCALAR == dispatcher.SelectProvider( PointsKernel, 1000, 0, GUCEF_NULL, GUCEF_NULL )->tier );
        dispatcher.SetMaxTier( PointsKernel, MATHBATCHTIER_ACCELERATOR );

        // minCount: the accelerator only from 500 elements
        ASSERT_TRUE( 2 == dispatcher.UnregisterProviders( &owner ) && builtIn == dispatcher.GetProviderCount( PointsKernel ) );
        ASSERT_TRUE( dispatcher.RegisterProvider( PointsKernel, MakeProvider( MATHBATCHTIER_ACCELERATOR, "FakeAccelerator", accelerator, MATHPROVIDER_BITEXACT, 500 ), &owner ) );
        ASSERT_TRUE( MATHBATCHTIER_ACCELERATOR != dispatcher.SelectProvider( PointsKernel, 499, 0, GUCEF_NULL, GUCEF_NULL )->tier );
        if ( BuildAllowsAccelerator )
            ASSERT_TRUE( MATHBATCHTIER_ACCELERATOR == dispatcher.SelectProvider( PointsKernel, 500, 0, GUCEF_NULL, GUCEF_NULL )->tier );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BatchDispatcher: determinism, fixed-point style kernels, failing providers and alignment" )
    try
    {
        CMathBatchDispatcher dispatcher;
        dispatcher.RegisterBuiltInProviders();
        SFakeProvider inexact;
        SFakeProvider failing;
        SFakeProvider aligned;
        inexact.work = ScalarTransformPoints( dispatcher );
        failing.work = inexact.work;
        aligned.work = inexact.work;
        failing.status = MATHSTATUS_OUT_OF_MEMORY;
        const int owner = 0;
        const STransformCase transformCase( 333, DispatcherSeed + 1 );
        GUCEF::vector< TFloat32Point3D > out;
        bool correct = false;

        // a provider without BITEXACT is never used for deterministic calls
        ASSERT_TRUE( dispatcher.RegisterProvider( PointsKernel, MakeProvider( MATHBATCHTIER_MULTICORE, "FakeInexact", inexact, 0 ), &owner ) );
        for ( UInt32 i=0; i<20; ++i )
            ASSERT_TRUE( MATHSTATUS_OK == transformCase.Run( dispatcher, MATHBATCH_DETERMINISTIC, out, correct ) && correct );
        ASSERT_TRUE( 0 == inexact.calls.Load() );
        ASSERT_TRUE( MATHSTATUS_OK == transformCase.Run( dispatcher, 0, out, correct ) && correct );
        ASSERT_TRUE( ( BuildAllowsMulticore ? 1u : 0u ) == inexact.calls.Load() );

        // an external kernel that is always deterministic (as fixed-point kernels are): no BITEXACT, never called
        const TMathBatchKernel external = (TMathBatchKernel) 300;
        ASSERT_TRUE( dispatcher.RegisterExternalKernel( external, "TEST.TransformPoints3.FX32", true, &owner ) );
        ASSERT_TRUE( dispatcher.IsKernelAlwaysDeterministic( external ) && !dispatcher.IsKernelAlwaysDeterministic( PointsKernel ) );
        SFakeProvider externalScalar;
        externalScalar.work = inexact.work;
        SFakeProvider externalInexact;
        externalInexact.work = inexact.work;
        ASSERT_TRUE( dispatcher.RegisterProvider( external, MakeProvider( MATHBATCHTIER_SCALAR, "ExternalScalar", externalScalar, MATHPROVIDER_BITEXACT ), &owner ) );
        ASSERT_TRUE( dispatcher.RegisterProvider( external, MakeProvider( MATHBATCHTIER_SIMD, "ExternalInexact", externalInexact, 0 ), &owner ) );
        ASSERT_TRUE( 0 == strcmp( "ExternalScalar", dispatcher.SelectProvider( external, 10, 0, GUCEF_NULL, GUCEF_NULL )->name ) );
        ASSERT_TRUE( 0 == strcmp( "ExternalScalar", dispatcher.SelectProvider( external, 10, MATHBATCH_REALTIME, GUCEF_NULL, GUCEF_NULL )->name ) );

        // a MAY_FAIL provider that fails: the same call completes on the next tier with the correct result
        ASSERT_TRUE( dispatcher.RegisterProvider( PointsKernel, MakeProvider( MATHBATCHTIER_ACCELERATOR, "FakeFailing", failing, MATHPROVIDER_BITEXACT | MATHPROVIDER_MAY_FAIL ), &owner ) );
        const UInt32 inexactBefore = inexact.calls.Load();
        ASSERT_TRUE( MATHSTATUS_OK == transformCase.Run( dispatcher, 0, out, correct ) && correct );
        ASSERT_TRUE( ( BuildAllowsAccelerator ? 1u : 0u ) == failing.calls.Load() );
        ASSERT_TRUE( ( BuildAllowsMulticore ? 1u : 0u ) + inexactBefore == inexact.calls.Load() );
        ASSERT_TRUE( MATHSTATUS_OK == transformCase.Run( dispatcher, MATHBATCH_DETERMINISTIC, out, correct ) && correct );
        ASSERT_TRUE( ( BuildAllowsAccelerator ? 2u : 0u ) == failing.calls.Load() );

        // every provider fails: Execute reports the last failure (a dispatcher with only failing providers)
        CMathBatchDispatcher onlyFailing;
        ASSERT_TRUE( onlyFailing.RegisterProvider( PointsKernel, MakeProvider( MATHBATCHTIER_SCALAR, "FakeFailing", failing, MATHPROVIDER_BITEXACT | MATHPROVIDER_MAY_FAIL ), &owner ) );
        ASSERT_TRUE( MATHSTATUS_OUT_OF_MEMORY == transformCase.Run( onlyFailing, 0, out, correct ) );
        CMathBatchDispatcher empty;
        ASSERT_TRUE( MATHSTATUS_INVALID_OPERAND == transformCase.Run( empty, 0, out, correct ) );

        // a provider that requires 64 byte aligned pointers is skipped for other pointers
        SMathBatchProvider alignedProvider = MakeProvider( MATHBATCHTIER_ACCELERATOR, "FakeAligned", aligned, MATHPROVIDER_BITEXACT | MATHPROVIDER_REQUIRES_ALIGNED_INPUT );
        alignedProvider.alignment = 64;
        CMathBatchDispatcher alignment;
        alignment.RegisterBuiltInProviders();
        ASSERT_TRUE( alignment.RegisterProvider( PointsKernel, alignedProvider, &owner ) );
        const void* alignedPointer = reinterpret_cast< const void* >( (size_t) 4096 );
        const void* misalignedPointer = reinterpret_cast< const void* >( (size_t) 4096 + 4 );
        if ( BuildAllowsAccelerator )
            ASSERT_TRUE( 0 == strcmp( "FakeAligned", alignment.SelectProvider( PointsKernel, 10, 0, alignedPointer, alignedPointer )->name ) );
        ASSERT_TRUE( 0 != strcmp( "FakeAligned", alignment.SelectProvider( PointsKernel, 10, 0, misalignedPointer, alignedPointer )->name ) );
        ASSERT_TRUE( 0 != strcmp( "FakeAligned", alignment.SelectProvider( PointsKernel, 10, 0, alignedPointer, misalignedPointer )->name ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BatchDispatcher: UnregisterProviders removes exactly the providers of its owner, external kernels" )
    try
    {
        CMathBatchDispatcher dispatcher;
        const UInt32 builtIn = dispatcher.RegisterBuiltInProviders();
        SFakeProvider fake;
        fake.work = ScalarTransformPoints( dispatcher );
        const int ownerA = 0;
        const int ownerB = 0;
        const TMathBatchKernel dot = MATHBATCHKERNEL_DOT_PRODUCTS3_F32;
        const UInt32 pointsBefore = dispatcher.GetProviderCount( PointsKernel );
        const UInt32 dotBefore = dispatcher.GetProviderCount( dot );
        ASSERT_TRUE( dispatcher.RegisterProvider( PointsKernel, MakeProvider( MATHBATCHTIER_MULTICORE, "A1", fake, MATHPROVIDER_BITEXACT ), &ownerA ) );
        ASSERT_TRUE( dispatcher.RegisterProvider( PointsKernel, MakeProvider( MATHBATCHTIER_ACCELERATOR, "B1", fake, MATHPROVIDER_BITEXACT ), &ownerB ) );
        ASSERT_TRUE( dispatcher.RegisterProvider( dot, MakeProvider( MATHBATCHTIER_MULTICORE, "A2", fake, MATHPROVIDER_BITEXACT ), &ownerA ) );
        ASSERT_TRUE( dispatcher.RegisterProvider( PointsKernel, MakeProvider( MATHBATCHTIER_MULTICORE, "A3", fake, MATHPROVIDER_BITEXACT ), &ownerA ) );

        // within a tier the registration order holds
        SMathBatchProvider p;
        ASSERT_TRUE( dispatcher.GetProvider( PointsKernel, 1, p ) && 0 == strcmp( "A1", p.name ) );
        ASSERT_TRUE( dispatcher.GetProvider( PointsKernel, 2, p ) && 0 == strcmp( "A3", p.name ) );

        ASSERT_TRUE( 3 == dispatcher.UnregisterProviders( &ownerA ) );
        ASSERT_TRUE( pointsBefore + 1 == dispatcher.GetProviderCount( PointsKernel ) && dotBefore == dispatcher.GetProviderCount( dot ) );
        ASSERT_TRUE( dispatcher.GetProvider( PointsKernel, 0, p ) && 0 == strcmp( "B1", p.name ) );
        ASSERT_TRUE( 0 == dispatcher.UnregisterProviders( &ownerA ) );
        ASSERT_TRUE( 1 == dispatcher.UnregisterProviders( &ownerB ) );
        ASSERT_TRUE( pointsBefore == dispatcher.GetProviderCount( PointsKernel ) );
        // the built-in providers belong to the dispatcher
        ASSERT_TRUE( builtIn == dispatcher.UnregisterProviders( &dispatcher ) );
        ASSERT_TRUE( 0 == dispatcher.GetProviderCount( PointsKernel ) && GUCEF_NULL != dispatcher.GetKernelId( PointsKernel ) );

        // external kernels: range, unique ids, repeated registration, dropped with the last provider of their owner
        const TMathBatchKernel external = (TMathBatchKernel) 256;
        ASSERT_FALSE( dispatcher.RegisterExternalKernel( PointsKernel, "TEST.A", false, &ownerA ) );
        ASSERT_FALSE( dispatcher.RegisterExternalKernel( (TMathBatchKernel) 512, "TEST.A", false, &ownerA ) );
        ASSERT_FALSE( dispatcher.RegisterExternalKernel( external, "", false, &ownerA ) );
        ASSERT_FALSE( dispatcher.RegisterExternalKernel( external, GUCEF_NULL, false, &ownerA ) );
        ASSERT_FALSE( dispatcher.RegisterExternalKernel( external, "MATH.TransformPoints3.F32", false, &ownerA ) );
        const char* tooLong = "TEST.0123456789012345678901234567890123456789012345678901234567890123456789";
        ASSERT_FALSE( dispatcher.RegisterExternalKernel( external, tooLong, false, &ownerA ) );
        ASSERT_TRUE( dispatcher.RegisterExternalKernel( external, "TEST.TransformAabbs.F32", false, &ownerA ) );
        ASSERT_TRUE( dispatcher.RegisterExternalKernel( external, "TEST.TransformAabbs.F32", false, &ownerA ) );
        ASSERT_FALSE( dispatcher.RegisterExternalKernel( (TMathBatchKernel) 257, "TEST.TransformAabbs.F32", false, &ownerA ) );
        ASSERT_FALSE( dispatcher.RegisterExternalKernel( external, "TEST.Other.F32", false, &ownerA ) );
        ASSERT_TRUE( 0 == strcmp( "TEST.TransformAabbs.F32", dispatcher.GetKernelId( external ) ) );
        ASSERT_TRUE( dispatcher.RegisterProvider( external, MakeProvider( MATHBATCHTIER_SCALAR, "AS", fake, MATHPROVIDER_BITEXACT ), &ownerA ) );
        ASSERT_TRUE( dispatcher.RegisterProvider( external, MakeProvider( MATHBATCHTIER_SIMD, "BS", fake, MATHPROVIDER_BITEXACT ), &ownerB ) );
        ASSERT_TRUE( 1 == dispatcher.UnregisterProviders( &ownerA ) );
        ASSERT_TRUE( GUCEF_NULL != dispatcher.GetKernelId( external ) && 1 == dispatcher.GetProviderCount( external ) );       // B still serves it
        ASSERT_TRUE( 1 == dispatcher.UnregisterProviders( &ownerB ) );
        ASSERT_TRUE( 0 == dispatcher.UnregisterProviders( &ownerA ) );                                                          // the kernel goes with its owner
        ASSERT_TRUE( GUCEF_NULL == dispatcher.GetKernelId( external ) && 0 == dispatcher.GetProviderCount( external ) );
        ASSERT_TRUE( dispatcher.RegisterExternalKernel( (TMathBatchKernel) 257, "TEST.TransformAabbs.F32", false, &ownerB ) );  // the id is free again
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BatchDispatcher: selection and realtime calls do not allocate" )
    GUCEF_TESTFW_SKIP_IF( !MathAllocationCounter::IsAvailable(), "no allocation counter with a sanitizer that replaces the allocator (the RealtimeSanitizer build proves the same in a realtime context)" );
    try
    {
        CMathBatchDispatcher dispatcher;
        dispatcher.RegisterBuiltInProviders();
        const STransformCase transformCase( 100, DispatcherSeed + 2 );
        GUCEF::vector< TFloat32Point3D > out( 100 );
        SMathTransformPoints3F32Args args;
        args.matrix = &transformCase.matrix;
        args.in = &transformCase.in[ 0 ];
        args.inStrideBytes = 12;
        args.out = &out[ 0 ];
        args.outStrideBytes = 12;
        args.count = 100;
        MathAllocationCounter::StartCounting();
        MathAllocationCounter::AllocateProbe();
        ASSERT_TRUE( 1 == MathAllocationCounter::StopCounting() );
        MathAllocationCounter::StartCounting();
        const SMathBatchProvider* provider = dispatcher.SelectProvider( PointsKernel, 100, MATHBATCH_REALTIME, args.in, args.out );
        const TMathStatus status = dispatcher.Execute( PointsKernel, args, 100, MATHBATCH_REALTIME, args.in, args.out );
        const UInt32 allocations = MathAllocationCounter::StopCounting();
        ASSERT_TRUE( 0 == allocations && GUCEF_NULL != provider && MATHSTATUS_OK == status );
        ASSERT_TRUE( 0 == memcmp( &out[ 0 ], &transformCase.expected[ 0 ], 100 * sizeof( TFloat32Point3D ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BatchDispatcher: four threads call while a fifth registers and unregisters providers 10^4 times" )
    GUCEF_TESTFW_SKIP_IF( SingleThreadedPlatform(), "this platform build has no threads (WebAssembly without pthreads)" );
    try
    {
        CMathBatchDispatcher dispatcher;
        dispatcher.RegisterBuiltInProviders();
        SFakeProvider multicore;
        SFakeProvider accelerator;
        multicore.work = ScalarTransformPoints( dispatcher );
        accelerator.work = multicore.work;
        accelerator.status = MATHSTATUS_OUT_OF_MEMORY;
        const STransformCase transformCase( 64, DispatcherSeed + 3 );

        SConcurrencyState state;
        state.dispatcher = &dispatcher;
        state.mutableDispatcher = &dispatcher;
        state.transformCase = &transformCase;
        state.multicore = &multicore;
        state.accelerator = &accelerator;
        state.registrationCycles = 10000;

        CDispatcherTestThread::TFunction functions[ 5 ] = { &CallerThread, &CallerThread, &CallerThread, &CallerThread, &RegistrationThread };
        ASSERT_TRUE( RunThreads( functions, 5, &state ) );
        ASSERT_TRUE( 0 == state.registrationFailures.Load() );
        ASSERT_TRUE( 0 == state.wrongResults.Load() );
        ASSERT_TRUE( 0 == state.failedCalls.Load() );
        ASSERT_TRUE( 800 <= state.callCount.Load() );
        ASSERT_TRUE( dispatcher.GetProviderCount( PointsKernel ) == dispatcher.GetProviderCount( MATHBATCHKERNEL_TRANSFORM_VECTORS3_F32 ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "BatchDispatcher: " << state.callCount.Load() << " calls, multi-core provider called " << multicore.calls.Load()
                   << " times, failing accelerator " << accelerator.calls.Load() << " times during the registrations" );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BatchDispatcher: UnregisterProviders returns only after a call inside a provider of the owner has left it" )
    GUCEF_TESTFW_SKIP_IF( SingleThreadedPlatform(), "this platform build has no threads (WebAssembly without pthreads)" );
    try
    {
        CMathBatchDispatcher builtIn;
        builtIn.RegisterBuiltInProviders();
        CMathBatchDispatcher dispatcher;                          // only the blocking provider: every call reaches it
        const STransformCase transformCase( 32, DispatcherSeed + 5 );
        SDrainState state;
        state.dispatcher = &dispatcher;
        state.work = ScalarTransformPoints( builtIn );
        state.transformCase = &transformCase;
        SMathBatchProvider provider;
        provider.tier = MATHBATCHTIER_SCALAR;
        provider.name = "Blocking";
        provider.function = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathTransformPoints3F32Function >( &BlockingTransformPoints ) );
        provider.minCount = 0;
        provider.maxCount = 0xFFFFFFFF;
        provider.flags = MATHPROVIDER_BITEXACT;
        provider.alignment = 0;
        provider.requiredCpuFeatures = 0;
        provider.context = &state;
        ASSERT_TRUE( dispatcher.RegisterProvider( PointsKernel, provider, &state ) );

        CDispatcherTestThread::TFunction functions[ 2 ] = { &DrainCallerThread, &DrainUnregisterThread };
        ASSERT_TRUE( RunThreads( functions, 2, &state ) );
        ASSERT_TRUE( 1 == state.removed.Load() );
        ASSERT_TRUE( 1 == state.leftWhenUnregisterReturned.Load() );
        ASSERT_TRUE( (UInt32) MATHSTATUS_OK == state.callStatus.Load() );
        ASSERT_TRUE( 0 == dispatcher.GetProviderCount( PointsKernel ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BatchDispatcher: aligned-input providers see every pointer and stride of the call" )
    try
    {
        // the alignment bits of the argument structs include the second input and the strides
        const void* aligned = reinterpret_cast< const void* >( (size_t) 4096 );
        const void* misaligned = reinterpret_cast< const void* >( (size_t) 4100 );
        SMathMultiplyMatrices4F32Args multiply = { aligned, 64, misaligned, 64, const_cast< void* >( aligned ), 64, 2 };
        ASSERT_TRUE( 0 != ( MathBatchAlignmentBits( multiply ) & 15 ) );
        multiply.b = aligned;
        ASSERT_TRUE( 0 == ( MathBatchAlignmentBits( multiply ) & 15 ) );
        multiply.bStrideBytes = 68;
        ASSERT_TRUE( 0 != ( MathBatchAlignmentBits( multiply ) & 15 ) );
        SMathDotProducts3F32Args dot = { aligned, 16, misaligned, 16, const_cast< void* >( aligned ), 16, 2 };
        ASSERT_TRUE( 0 != ( MathBatchAlignmentBits( dot ) & 15 ) );
        dot.b = aligned;
        dot.aStrideBytes = 12;
        ASSERT_TRUE( 0 != ( MathBatchAlignmentBits( dot ) & 15 ) );

        // Execute: a provider that requires 16 byte alignment is skipped for a stride of 12, used for a stride of 16
        CMathBatchDispatcher builtIn;
        builtIn.RegisterBuiltInProviders();
        CMathBatchDispatcher dispatcher;
        SFakeProvider plain;
        SFakeProvider alignedFake;
        plain.work = ScalarTransformPoints( builtIn );
        alignedFake.work = plain.work;
        const int owner = 0;
        ASSERT_TRUE( dispatcher.RegisterProvider( PointsKernel, MakeProvider( MATHBATCHTIER_SCALAR, "Plain", plain, MATHPROVIDER_BITEXACT ), &owner ) );
        SMathBatchProvider alignedProvider = MakeProvider( MATHBATCHTIER_SIMD, "Aligned", alignedFake, MATHPROVIDER_BITEXACT | MATHPROVIDER_REQUIRES_ALIGNED_INPUT );
        alignedProvider.alignment = 16;
        ASSERT_TRUE( dispatcher.RegisterProvider( PointsKernel, alignedProvider, &owner ) );
        GUCEF::vector< Float32 > buffer( 4 * 64 + 16, 1.0f );
        Float32* base = &buffer[ 0 ];
        while ( 0 != ( (size_t) base & 15 ) )
            ++base;
        TFloat32Matrix4D m;
        SMathTransformPoints3F32Args packed = { &m, base, 12, base, 12, 8 };
        ASSERT_TRUE( MATHSTATUS_OK == dispatcher.Execute( PointsKernel, packed, 8, 0, packed.in, packed.out ) );
        ASSERT_TRUE( 1 == plain.calls.Load() && 0 == alignedFake.calls.Load() );
        SMathTransformPoints3F32Args padded = { &m, base, 16, base, 16, 8 };
        ASSERT_TRUE( MATHSTATUS_OK == dispatcher.Execute( PointsKernel, padded, 8, 0, padded.in, padded.out ) );
        ASSERT_TRUE( 1 == plain.calls.Load() && 1 == alignedFake.calls.Load() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BatchDispatcher: 16 threads that make the first batch call of a process together all get the right result" )
    #if !defined( __linux__ )
    GUCEF_TESTFW_SKIP( "the child process is started through /proc/self/exe (Linux)" );
    #else
    try
    {
        char exePath[ 4096 ];
        const ssize_t exePathLength = readlink( "/proc/self/exe", exePath, sizeof( exePath ) - 1 );
        ASSERT_TRUE( exePathLength > 0 );
        if ( exePathLength > 0 )
        {
            exePath[ exePathLength ] = '\0';
            UInt32 passed = 0;
            const UInt32 runs = 10;
            for ( UInt32 run=0; run<runs; ++run )
            {
                CORE::CProcess::TStringVector args;
                args.push_back( GUCEF_MATH_TESTAPP_CONCURRENT_FIRST_CALL_ARG );
                CORE::CProcess child;
                child.SetIsolateFromParent( true );
                ASSERT_TRUE( child.Launch( exePath, args, CORE::CString(), TESTFW::ChildEnvironmentWithOwnExitCode() ) );
                Int32 exitCode = -1;
                const bool ended = child.WaitForExit( 120000, exitCode );
                if ( !ended )
                    child.Terminate( false );
                if ( ended && 0 == exitCode )
                    ++passed;
                else
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "BatchDispatcher: the concurrent first call child " << run << " exited with " << exitCode );
            }
            ASSERT_TRUE( runs == passed );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    #endif
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

int
RunConcurrentFirstBatchCall( void )
{
    SFirstCallState state;
    state.threadCount = 16;
    CDispatcherTestThread::TFunction functions[ 16 ];
    for ( UInt32 i=0; i<16; ++i )
        functions[ i ] = &FirstCallThread;
    const bool ran = RunThreads( functions, 16, &state );
    return ran && 0 == state.wrong.Load() ? 0 : 1;
}

/*-------------------------------------------------------------------------*/
