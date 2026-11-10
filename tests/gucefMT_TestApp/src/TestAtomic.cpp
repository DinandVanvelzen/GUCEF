/*
 *  gucefMT_TestApp: GUCEF test application for the MT module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <limits>
#include <stdio.h>

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

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAtomic.h"

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const UInt32 ThreadCount = 4;
const UInt32 ThreadWaitLimitInMs = 120000;

const MT::TMemoryOrder AllOrders[ 6 ] = { MT::MEMORDER_RELAXED, MT::MEMORDER_CONSUME, MT::MEMORDER_ACQUIRE, MT::MEMORDER_RELEASE, MT::MEMORDER_ACQ_REL, MT::MEMORDER_SEQ_CST };

enum ETestEnum { TESTENUM_NONE = 0, TESTENUM_FIRST = 1, TESTENUM_SECOND = 2 };

/*-------------------------------------------------------------------------*/

/** Runs a function of the test on its own thread. The thread stops after the function returns */
class CAtomicTestThread : public MT::CActiveObject
{
    public:

    typedef void ( *TFunction )( void* argument );

    CAtomicTestThread( TFunction function, void* argument )
        : MT::CActiveObject()
        , m_function( function )
        , m_argument( argument )
        , m_done( false )
    {}

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        return true;
    }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
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

/** Starts 'count' threads on the function (argument is the pointer to a SThreadArg array element each) and waits for all of them */
struct SThreadArg
{
    UInt32 index;
    void* shared;
};

bool
RunThreads( CAtomicTestThread::TFunction function, void* shared, UInt32 count )
{
    GUCEF::vector< SThreadArg > args( count );
    GUCEF::vector< CAtomicTestThread* > threads;
    for ( UInt32 i=0; i<count; ++i )
    {
        args[ i ].index = i;
        args[ i ].shared = shared;
    }
    bool allStarted = true;
    for ( UInt32 i=0; i<count; ++i )
    {
        threads.push_back( GUCEF_NEW CAtomicTestThread( function, &args[ i ] ) );
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
        // the function has returned, now let the thread finish winding down before its object goes away
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

/** Releases the threads at the same moment to get real contention */
template< typename Tag >
struct SStartGate
{
    MT::CTAtomic< UInt32, Tag > arrived;
    MT::CTAtomic< UInt32, Tag > open;
    SStartGate( void ) : arrived( 0 ), open( 0 ) {}

    void Wait( UInt32 threadCount )
    {
        arrived.FetchAdd( 1 );
        if ( arrived.Load() == threadCount )
            open.Store( 1 );
        UInt32 spins = 0;
        while ( 0 == open.Load( MT::MEMORDER_ACQUIRE ) && spins < 200000000 )
        {
            MT::AtomicCpuRelax();
            ++spins;
        }
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      SINGLE THREADED OPERATIONS                                         //
//                                                                         //
//-------------------------------------------------------------------------*/

template< typename T, typename Tag >
void
CheckIntegerOperations( void )
{
    typedef MT::CTAtomic< T, Tag > TAtomic;

    const T maxValue = std::numeric_limits< T >::max();
    const T wrapped = std::numeric_limits< T >::is_signed ? std::numeric_limits< T >::min() : static_cast< T >( 0 );

    TAtomic a( 5 );
    ASSERT_TRUE( a.Load() == 5 );
    ASSERT_TRUE( TAtomic().Load() == 0 );

    a.Store( 7 );
    ASSERT_TRUE( a.Load() == 7 );
    ASSERT_TRUE( a.Exchange( 9 ) == 7 );
    ASSERT_TRUE( a.Load() == 9 );

    T expected = 3;
    ASSERT_FALSE( a.CompareExchangeStrong( expected, 11 ) );
    ASSERT_TRUE( expected == 9 );                               // the failure reports what was found
    ASSERT_TRUE( a.Load() == 9 );
    ASSERT_TRUE( a.CompareExchangeStrong( expected, 11 ) );
    ASSERT_TRUE( a.Load() == 11 );

    expected = 11;
    UInt32 attempts = 0;
    while ( !a.CompareExchangeWeak( expected, 12 ) && attempts < 1000 )
    {
        expected = 11;                                          // spurious failure may or may not have touched it
        ++attempts;
    }
    ASSERT_TRUE( a.Load() == 12 );

    ASSERT_TRUE( a.FetchAdd( 3 ) == 12 );  ASSERT_TRUE( a.Load() == 15 );
    ASSERT_TRUE( a.FetchSub( 5 ) == 15 );  ASSERT_TRUE( a.Load() == 10 );
    ASSERT_TRUE( a.FetchAnd( 6 ) == 10 );  ASSERT_TRUE( a.Load() == 2 );
    ASSERT_TRUE( a.FetchOr( 9 ) == 2 );    ASSERT_TRUE( a.Load() == 11 );
    ASSERT_TRUE( a.FetchXor( 3 ) == 11 );  ASSERT_TRUE( a.Load() == 8 );

    a.Store( maxValue );
    ASSERT_TRUE( a.FetchAdd( 1 ) == maxValue );
    ASSERT_TRUE( a.Load() == wrapped );                         // wraps like the unsigned type does, no trap
    ASSERT_TRUE( a.FetchSub( 1 ) == wrapped );
    ASSERT_TRUE( a.Load() == maxValue );

    a.Store( 1 );
    ASSERT_TRUE( ++a == 2 );
    ASSERT_TRUE( a++ == 2 );  ASSERT_TRUE( a == 3 );
    ASSERT_TRUE( --a == 2 );
    ASSERT_TRUE( a-- == 2 );  ASSERT_TRUE( a == 1 );
    ASSERT_TRUE( ( a += 4 ) == 5 );
    ASSERT_TRUE( ( a -= 2 ) == 3 );
    ASSERT_TRUE( ( a |= 8 ) == 11 );
    ASSERT_TRUE( ( a &= 10 ) == 10 );
    ASSERT_TRUE( ( a ^= 3 ) == 9 );
    ASSERT_TRUE( ( a = 4 ) == 4 );
    T plain = a;
    ASSERT_TRUE( plain == 4 );

    if ( std::numeric_limits< T >::is_signed )
    {
        a.Store( static_cast< T >( -1 ) );
        ASSERT_TRUE( a.Load() == static_cast< T >( -1 ) );
        ASSERT_TRUE( a.FetchAdd( 1 ) == static_cast< T >( -1 ) );
        ASSERT_TRUE( a.Load() == 0 );
        a.Store( static_cast< T >( -5 ) );
        ASSERT_TRUE( a.FetchAdd( 2 ) == static_cast< T >( -5 ) );
        ASSERT_TRUE( a.Load() == static_cast< T >( -3 ) );
    }
}

/*-------------------------------------------------------------------------*/

template< typename Tag >
void
CheckAllIntegerWidths( void )
{
    CheckIntegerOperations< UInt8, Tag >();
    CheckIntegerOperations< Int8, Tag >();
    CheckIntegerOperations< UInt16, Tag >();
    CheckIntegerOperations< Int16, Tag >();
    CheckIntegerOperations< UInt32, Tag >();
    CheckIntegerOperations< Int32, Tag >();
    CheckIntegerOperations< UInt64, Tag >();
    CheckIntegerOperations< Int64, Tag >();
}

/*-------------------------------------------------------------------------*/

/** Every memory order on every operation, including the combinations that are not meaningful and get strengthened */
template< typename T, typename Tag >
void
CheckMemoryOrders( void )
{
    MT::CTAtomic< T, Tag > a( 0 );
    for ( UInt32 o=0; o<6; ++o )
    {
        const MT::TMemoryOrder order = AllOrders[ o ];

        a.Store( 10, order );
        ASSERT_TRUE( a.Load( order ) == 10 );
        ASSERT_TRUE( a.Exchange( 20, order ) == 10 );
        ASSERT_TRUE( a.FetchAdd( 5, order ) == 20 );
        ASSERT_TRUE( a.FetchSub( 5, order ) == 25 );
        ASSERT_TRUE( a.FetchOr( 1, order ) == 20 );
        ASSERT_TRUE( a.FetchAnd( 0x1F, order ) == 21 );
        ASSERT_TRUE( a.FetchXor( 1, order ) == 21 );
        ASSERT_TRUE( a.Load() == 20 );

        for ( UInt32 f=0; f<6; ++f )
        {
            T expected = 20;
            ASSERT_TRUE( a.CompareExchangeStrong( expected, 21, order, AllOrders[ f ] ) );
            expected = 99;
            ASSERT_FALSE( a.CompareExchangeStrong( expected, 5, order, AllOrders[ f ] ) );
            ASSERT_TRUE( expected == 21 );
            expected = 21;
            UInt32 attempts = 0;
            while ( !a.CompareExchangeWeak( expected, 20, order, AllOrders[ f ] ) && attempts < 1000 ) { expected = 21; ++attempts; }
            ASSERT_TRUE( a.Load() == 20 );
        }
    }
}

/*-------------------------------------------------------------------------*/

template< typename Tag >
void
CheckPointerBoolAndEnum( void )
{
    // pointers step in elements of the pointed to type
    Int64 elements[ 8 ];
    MT::CTAtomic< Int64*, Tag > p( elements );
    ASSERT_TRUE( p.Load() == elements );
    ASSERT_TRUE( p.FetchAdd( 3 ) == elements );
    ASSERT_TRUE( p.Load() == elements + 3 );
    ASSERT_TRUE( ++p == elements + 4 );
    ASSERT_TRUE( ( p -= 2 ) == elements + 2 );
    ASSERT_TRUE( p.FetchSub( 1 ) == elements + 2 );
    ASSERT_TRUE( p.Load() == elements + 1 );
    Int64* expected = elements;
    ASSERT_FALSE( p.CompareExchangeStrong( expected, elements + 7 ) );
    ASSERT_TRUE( expected == elements + 1 );
    ASSERT_TRUE( p.CompareExchangeStrong( expected, elements + 7 ) );
    ASSERT_TRUE( p.Load() == elements + 7 );
    ASSERT_TRUE( p.Exchange( GUCEF_NULL ) == elements + 7 );
    ASSERT_TRUE( p.Load() == GUCEF_NULL );
    ASSERT_TRUE( ( MT::CTAtomic< Int64*, Tag >().Load() == GUCEF_NULL ) );

    // void pointers step in bytes
    MT::CTAtomic< void*, Tag > raw( elements );
    ASSERT_TRUE( raw.FetchAdd( 8 ) == static_cast< void* >( elements ) );
    ASSERT_TRUE( raw.Load() == static_cast< void* >( elements + 1 ) );

    // bool
    MT::CTAtomic< bool, Tag > flag( false );
    ASSERT_FALSE( flag.Load() );
    flag.Store( true );
    ASSERT_TRUE( flag.Exchange( false ) );
    ASSERT_FALSE( flag.Load() );
    bool expectedFlag = false;
    ASSERT_TRUE( flag.CompareExchangeStrong( expectedFlag, true ) );
    ASSERT_TRUE( flag.Load() );
    expectedFlag = false;
    ASSERT_FALSE( flag.CompareExchangeStrong( expectedFlag, false ) );
    ASSERT_TRUE( expectedFlag );

    // enumeration
    MT::CTAtomic< ETestEnum, Tag > state( TESTENUM_NONE );
    ASSERT_TRUE( state.Load() == TESTENUM_NONE );
    state.Store( TESTENUM_FIRST );
    ETestEnum expectedState = TESTENUM_FIRST;
    ASSERT_TRUE( state.CompareExchangeStrong( expectedState, TESTENUM_SECOND ) );
    ASSERT_TRUE( state.Load() == TESTENUM_SECOND );
    ASSERT_TRUE( state.Exchange( TESTENUM_NONE ) == TESTENUM_SECOND );
}

/*-------------------------------------------------------------------------*/

template< typename Tag >
void
CheckFencesAndLockFreedom( bool expectLockFree )
{
    MT::AtomicThreadFence< Tag >( MT::MEMORDER_ACQUIRE );
    MT::AtomicThreadFence< Tag >( MT::MEMORDER_RELEASE );
    MT::AtomicThreadFence< Tag >( MT::MEMORDER_SEQ_CST );
    MT::AtomicThreadFence();
    MT::AtomicSignalFence();
    MT::AtomicCpuRelax();

    ASSERT_TRUE( (MT::CTAtomic< UInt32, Tag >().IsLockFree() == expectLockFree) );
    ASSERT_TRUE( (MT::CTAtomic< UInt8, Tag >().IsLockFree() == expectLockFree) );
    ASSERT_TRUE( (MT::CTAtomic< UInt16, Tag >().IsLockFree() == expectLockFree) );
    ASSERT_TRUE( (MT::CTAtomic< void*, Tag >().IsLockFree() == expectLockFree) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      MULTI THREADED                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 CounterIterations = 100000;

template< typename Tag >
struct SCounters
{
    SStartGate< Tag > gate;
    MT::CTAtomic< UInt32, Tag > fetchAdd;
    MT::CTAtomic< UInt64, Tag > casLoop;
    MT::CTAtomic< UInt16, Tag > narrow;
    MT::CTAtomic< UInt8, Tag > tiny;
    MT::CTAtomic< UInt32, Tag > bits;
    MT::CTAtomic< Int64, Tag > signedSum;
    SCounters( void ) : gate(), fetchAdd( 0 ), casLoop( 0 ), narrow( 0 ), tiny( 0 ), bits( 0 ), signedSum( 0 ) {}
};

template< typename Tag >
void
CounterWorker( void* argument )
{
    SThreadArg* arg = static_cast< SThreadArg* >( argument );
    SCounters< Tag >& c = *static_cast< SCounters< Tag >* >( arg->shared );
    c.gate.Wait( ThreadCount );

    for ( UInt32 i=0; i<CounterIterations; ++i )
    {
        c.fetchAdd.FetchAdd( 1, MT::MEMORDER_RELAXED );

        UInt64 e = c.casLoop.Load( MT::MEMORDER_RELAXED );
        while ( !c.casLoop.CompareExchangeWeak( e, e + 2, MT::MEMORDER_ACQ_REL, MT::MEMORDER_RELAXED ) ) {}

        c.signedSum.FetchAdd( ( 0 == arg->index % 2 ) ? 3 : -2 );
    }
    for ( UInt32 i=0; i<10000; ++i )
        c.narrow.FetchAdd( 1 );
    for ( UInt32 i=0; i<50; ++i )
        c.tiny.FetchAdd( 1 );

    // each thread sets, clears and finally sets its own bit, without disturbing the bits of the others
    const UInt32 myBit = static_cast< UInt32 >( 1 ) << arg->index;
    for ( UInt32 i=0; i<1000; ++i )
    {
        c.bits.FetchOr( myBit );
        c.bits.FetchAnd( ~myBit );
    }
    c.bits.FetchOr( myBit );
}

template< typename Tag >
void
CheckCountersAcrossThreads( void )
{
    SCounters< Tag > counters;
    ASSERT_TRUE( RunThreads( &CounterWorker< Tag >, &counters, ThreadCount ) );
    ASSERT_TRUE( counters.fetchAdd.Load() == ThreadCount * CounterIterations );
    ASSERT_TRUE( counters.casLoop.Load() == static_cast< UInt64 >( ThreadCount ) * CounterIterations * 2 );
    ASSERT_TRUE( counters.narrow.Load() == ThreadCount * 10000 );
    ASSERT_TRUE( counters.tiny.Load() == ThreadCount * 50 );
    ASSERT_TRUE( counters.bits.Load() == ( ( static_cast< UInt32 >( 1 ) << ThreadCount ) - 1 ) );
    // threads 0 and 2 add 3, threads 1 and 3 subtract 2
    ASSERT_TRUE( counters.signedSum.Load() == static_cast< Int64 >( CounterIterations ) * ( 2 * 3 - 2 * 2 ) );
}

/*-------------------------------------------------------------------------*/

/** A spin lock made from an atomic exchange protects a plain counter: only correct when acquire / release really order memory */
template< typename Tag >
struct SSpinLockShared
{
    SStartGate< Tag > gate;
    MT::CTAtomic< UInt32, Tag > lock;
    volatile UInt64 protectedCounter;
    SSpinLockShared( void ) : gate(), lock( 0 ), protectedCounter( 0 ) {}
};

template< typename Tag >
void
SpinLockWorker( void* argument )
{
    SThreadArg* arg = static_cast< SThreadArg* >( argument );
    SSpinLockShared< Tag >& s = *static_cast< SSpinLockShared< Tag >* >( arg->shared );
    s.gate.Wait( ThreadCount );

    for ( UInt32 i=0; i<20000; ++i )
    {
        while ( 0 != s.lock.Exchange( 1, MT::MEMORDER_ACQUIRE ) )
            MT::AtomicCpuRelax();
        const UInt64 value = s.protectedCounter;    // plain read, modify, write: any lack of mutual exclusion loses updates
        s.protectedCounter = value + 1;
        s.lock.Store( 0, MT::MEMORDER_RELEASE );
    }
}

template< typename Tag >
void
CheckSpinLockAcrossThreads( void )
{
    SSpinLockShared< Tag > shared;
    ASSERT_TRUE( RunThreads( &SpinLockWorker< Tag >, &shared, ThreadCount ) );
    ASSERT_TRUE( shared.protectedCounter == static_cast< UInt64 >( ThreadCount ) * 20000 );
}

/*-------------------------------------------------------------------------*/

/** One thread fills a buffer and publishes how much is valid with a release store, another reads it with acquire loads */
const UInt32 PublishCount = 200000;

template< typename Tag >
struct SPublishShared
{
    volatile UInt32 data[ PublishCount ];
    MT::CTAtomic< UInt32, Tag > published;
    volatile UInt32 failures;
    volatile UInt32 consumed;
    SPublishShared( void ) : published( 0 ), failures( 0 ), consumed( 0 ) {}
};

template< typename Tag >
void
PublishWorker( void* argument )
{
    SThreadArg* arg = static_cast< SThreadArg* >( argument );
    SPublishShared< Tag >& s = *static_cast< SPublishShared< Tag >* >( arg->shared );

    if ( 0 == arg->index )
    {
        for ( UInt32 i=0; i<PublishCount; ++i )
        {
            s.data[ i ] = i * 3 + 1;
            s.published.Store( i + 1, MT::MEMORDER_RELEASE );
        }
    }
    else
    {
        UInt32 checked = 0;
        UInt32 spins = 0;
        while ( checked < PublishCount && spins < 400000000 )
        {
            const UInt32 available = s.published.Load( MT::MEMORDER_ACQUIRE );
            if ( available == checked )
            {
                MT::AtomicCpuRelax();
                ++spins;
                continue;
            }
            for ( ; checked < available; ++checked )
                if ( s.data[ checked ] != checked * 3 + 1 )
                    s.failures = s.failures + 1;
        }
        s.consumed = checked;
    }
}

template< typename Tag >
void
CheckPublicationAcrossThreads( void )
{
    SPublishShared< Tag >* shared = GUCEF_NEW SPublishShared< Tag >();
    ASSERT_TRUE( RunThreads( &PublishWorker< Tag >, shared, 2 ) );
    ASSERT_TRUE( shared->consumed == PublishCount );
    ASSERT_TRUE( shared->failures == 0 );
    GUCEF_DELETE shared;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      SUITE PER BACKEND                                                  //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The name of a test case of the suite of a backend, in the given buffer */
const char*
CaseName( char* buffer, size_t bufferSize, const char* backendName, const char* title )
{
    snprintf( buffer, bufferSize, "Atomic [%s]: %s", backendName, title );
    return buffer;
}

template< typename Tag >
void
PerformBackendTests( const char* backendName, bool expectLockFree )
{
    char name[ 128 ];

    {
        CaseName( name, sizeof( name ), backendName, "Operations on all integer widths" );
        GUCEF_TESTFW_TESTCASE( name )
        try
        {
            CheckAllIntegerWidths< Tag >();
        }
        catch( ... )
        {
            ERRORHERE;
        }
        GUCEF_TESTFW_TESTCASE_END
    }

    {
        CaseName( name, sizeof( name ), backendName, "Every memory order" );
        GUCEF_TESTFW_TESTCASE( name )
        try
        {
            CheckMemoryOrders< UInt8, Tag >();
            CheckMemoryOrders< UInt16, Tag >();
            CheckMemoryOrders< UInt32, Tag >();
            CheckMemoryOrders< UInt64, Tag >();
        }
        catch( ... )
        {
            ERRORHERE;
        }
        GUCEF_TESTFW_TESTCASE_END
    }

    {
        CaseName( name, sizeof( name ), backendName, "Pointers, bool and enumerations" );
        GUCEF_TESTFW_TESTCASE( name )
        try
        {
            CheckPointerBoolAndEnum< Tag >();
        }
        catch( ... )
        {
            ERRORHERE;
        }
        GUCEF_TESTFW_TESTCASE_END
    }

    {
        CaseName( name, sizeof( name ), backendName, "Fences and lock freedom" );
        GUCEF_TESTFW_TESTCASE( name )
        try
        {
            CheckFencesAndLockFreedom< Tag >( expectLockFree );
        }
        catch( ... )
        {
            ERRORHERE;
        }
        GUCEF_TESTFW_TESTCASE_END
    }

    {
        CaseName( name, sizeof( name ), backendName, "No lost updates across threads" );
        GUCEF_TESTFW_TESTCASE( name )
        try
        {
            CheckCountersAcrossThreads< Tag >();
        }
        catch( ... )
        {
            ERRORHERE;
        }
        GUCEF_TESTFW_TESTCASE_END
    }

    {
        CaseName( name, sizeof( name ), backendName, "Spin lock gives mutual exclusion" );
        GUCEF_TESTFW_TESTCASE( name )
        try
        {
            CheckSpinLockAcrossThreads< Tag >();
        }
        catch( ... )
        {
            ERRORHERE;
        }
        GUCEF_TESTFW_TESTCASE_END
    }

    {
        CaseName( name, sizeof( name ), backendName, "Release / acquire publication" );
        GUCEF_TESTFW_TESTCASE( name )
        try
        {
            CheckPublicationAcrossThreads< Tag >();
        }
        catch( ... )
        {
            ERRORHERE;
        }
        GUCEF_TESTFW_TESTCASE_END
    }
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAtomicTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Atomic TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefMT.Atomic" );

    PerformBackendTests< void >( "default", true );

    #ifdef GUCEF_MT_ATOMIC_HAVE_STD
    PerformBackendTests< MT::SAtomicBackendStd >( "std", true );
    #endif
    #ifdef GUCEF_MT_ATOMIC_HAVE_GCC_ATOMIC
    PerformBackendTests< MT::SAtomicBackendGccAtomic >( "gcc atomic", true );
    #endif
    #ifdef GUCEF_MT_ATOMIC_HAVE_GCC_SYNC
    PerformBackendTests< MT::SAtomicBackendGccSync >( "gcc sync", true );
    #endif
    #ifdef GUCEF_MT_ATOMIC_HAVE_MSVC
    PerformBackendTests< MT::SAtomicBackendMsvc >( "msvc", true );
    #endif
    PerformBackendTests< MT::SAtomicBackendLock >( "lock", false );
}

/*-------------------------------------------------------------------------*/
