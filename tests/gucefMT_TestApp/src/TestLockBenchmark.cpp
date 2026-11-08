/*
 *  gucefMT_TestApp: GUCEF test application for the MT module
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

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_MUTEX_H
#include "gucefMT_mutex.h"
#define GUCEF_MT_MUTEX_H
#endif /* GUCEF_MT_MUTEX_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CREADWRITELOCK_H
#include "gucefMT_CReadWriteLock.h"
#define GUCEF_MT_CREADWRITELOCK_H
#endif /* GUCEF_MT_CREADWRITELOCK_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestLockBenchmark.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE

#define SKIP_UNLESS_BENCH() \
    GUCEF_TESTFW_SKIP_IF( GUCEF_TESTFW_GETENV( "GUCEF_TEST_MT_LOCK_BENCH" ) != "1", "GUCEF_TEST_MT_LOCK_BENCH is not 1" )

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const UInt32 RunCount = 9;                          /**< runs per measurement, the median is reported */
const UInt32 UncontendedPairsPerRun = 1000000;
const UInt32 ContendingThreadCount = 4;
const UInt32 ContendedPairsPerThread = 200000;
const Int32 ThreadWaitLimitInMs = 120000;

/** The kinds of lock and unlock pairs that are measured */
enum ELockKind
{
    LOCKKIND_CMUTEX = 0,
    LOCKKIND_C_MUTEX,
    LOCKKIND_C_MUTEX_REENTERED,     /**< one outer lock is held, the pair measured is the re-entry */
    LOCKKIND_RWLOCK_WRITER,
    LOCKKIND_RWLOCK_READER
};

/** The locks of one measurement */
struct SBenchLocks
{
    SBenchLocks( void )
        : cmutex()
        , mutex( MT::MutexCreate() )
        , rwLock( true )
        , counter( 0 )
    {}

    ~SBenchLocks()
    {
        if ( GUCEF_NULL != mutex )
            MT::MutexDestroy( mutex );
    }

    MT::CMutex cmutex;
    struct MT::SMutex* mutex;
    MT::CReadWriteLock rwLock;
    UInt64 counter;                 /**< changed under the lock, so the pair cannot be optimised away */
};

/*-------------------------------------------------------------------------*/

/** Runs 'pairs' lock and unlock pairs of the given kind, returns false if a lock or unlock failed */
bool
RunPairs( SBenchLocks& locks, ELockKind kind, UInt32 pairs )
{
    bool ok = true;
    switch ( kind )
    {
        case LOCKKIND_CMUTEX:
        {
            for ( UInt32 i=0; i<pairs; ++i )
            {
                ok = ( MT::LOCKSTATUS_OPERATION_SUCCESS == locks.cmutex.Lock() ) && ok;
                ++locks.counter;
                ok = ( MT::LOCKSTATUS_OPERATION_SUCCESS == locks.cmutex.Unlock() ) && ok;
            }
            break;
        }
        case LOCKKIND_C_MUTEX:
        case LOCKKIND_C_MUTEX_REENTERED:
        {
            for ( UInt32 i=0; i<pairs; ++i )
            {
                ok = ( GUCEF_MUTEX_OPERATION_SUCCESS == MT::MutexLock( locks.mutex, GUCEF_MUTEX_INFINITE_TIMEOUT ) ) && ok;
                ++locks.counter;
                ok = ( GUCEF_MUTEX_OPERATION_SUCCESS == MT::MutexUnlock( locks.mutex ) ) && ok;
            }
            break;
        }
        case LOCKKIND_RWLOCK_WRITER:
        {
            for ( UInt32 i=0; i<pairs; ++i )
            {
                ok = ( MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == locks.rwLock.WriterStart() ) && ok;
                ++locks.counter;
                ok = ( MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == locks.rwLock.WriterStop() ) && ok;
            }
            break;
        }
        case LOCKKIND_RWLOCK_READER:
        {
            // readers share the lock: they only read the counter
            volatile UInt64 sum = 0;
            for ( UInt32 i=0; i<pairs; ++i )
            {
                ok = ( MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == locks.rwLock.ReaderStart() ) && ok;
                sum = sum + locks.counter;
                ok = ( MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == locks.rwLock.ReaderStop() ) && ok;
            }
            break;
        }
    }
    return ok;
}

/*-------------------------------------------------------------------------*/

Float64
TicksToNs( UInt64 ticks )
{
    return (Float64) ticks * 1000000000.0 / (Float64) MT::PrecisionTimerResolution();
}

/*-------------------------------------------------------------------------*/

void
SortAscending( Float64* values, UInt32 count )
{
    for ( UInt32 i=1; i<count; ++i )
    {
        Float64 value = values[ i ];
        UInt32 n = i;
        while ( n > 0 && values[ n-1 ] > value )
        {
            values[ n ] = values[ n-1 ];
            --n;
        }
        values[ n ] = value;
    }
}

/*-------------------------------------------------------------------------*/

void
Report( const char* what, Float64* nsPerPair, UInt32 count )
{
    SortAscending( nsPerPair, count );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "LockBenchmark: " << what << ": median " << CORE::ToString( nsPerPair[ count / 2 ] ) <<
        " ns per lock and unlock pair (min " << CORE::ToString( nsPerPair[ 0 ] ) << ", max " << CORE::ToString( nsPerPair[ count-1 ] ) <<
        ", " << count << " runs)" );
}

/*-------------------------------------------------------------------------*/

void
MeasureUncontended( const char* what, ELockKind kind )
{
    SBenchLocks locks;
    bool outerLocked = false;
    if ( LOCKKIND_C_MUTEX_REENTERED == kind )
        outerLocked = GUCEF_MUTEX_OPERATION_SUCCESS == MT::MutexLock( locks.mutex, GUCEF_MUTEX_INFINITE_TIMEOUT );

    RunPairs( locks, kind, UncontendedPairsPerRun / 10 );    // warm up

    Float64 nsPerPair[ RunCount ];
    bool ok = true;
    for ( UInt32 run=0; run<RunCount; ++run )
    {
        UInt64 start = MT::PrecisionTickCount();
        ok = RunPairs( locks, kind, UncontendedPairsPerRun ) && ok;
        UInt64 end = MT::PrecisionTickCount();
        nsPerPair[ run ] = TicksToNs( end - start ) / (Float64) UncontendedPairsPerRun;
    }

    if ( outerLocked )
        MT::MutexUnlock( locks.mutex );
    if ( !ok )
        ERRORHERE;
    Report( what, nsPerPair, RunCount );
}

/*-------------------------------------------------------------------------*/

/** What the contending threads share */
struct SContention
{
    SContention( void )
        : locks( GUCEF_NULL )
        , kind( LOCKKIND_CMUTEX )
        , ready( 0 )
        , go( false )
        , failures( 0 )
    {}

    SBenchLocks* locks;
    ELockKind kind;
    MT::CTAtomic< UInt32 > ready;
    MT::CTAtomic< bool > go;
    MT::CTAtomic< UInt32 > failures;
};

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_CALLSPEC_PREFIX
ContendingThreadFunc( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    SContention* contention = static_cast< SContention* >( data );
    ++contention->ready;
    while ( !contention->go.Load( MT::MEMORDER_ACQUIRE ) )
    {
    }
    if ( !RunPairs( *contention->locks, contention->kind, ContendedPairsPerThread ) )
        ++contention->failures;
    return 0;
}

/*-------------------------------------------------------------------------*/

void
MeasureContended( const char* what, ELockKind kind )
{
    Float64 nsPerPair[ RunCount ];
    UInt32 runs = 0;
    bool ok = true;
    for ( UInt32 run=0; run<RunCount; ++run )
    {
        SBenchLocks locks;
        SContention contention;
        contention.locks = &locks;
        contention.kind = kind;

        MT::SThreadData* threads[ ContendingThreadCount ];
        UInt32 created = 0;
        for ( UInt32 i=0; i<ContendingThreadCount; ++i )
        {
            threads[ i ] = MT::ThreadDataReserve();
            if ( GUCEF_NULL == threads[ i ] )
                break;
            if ( 0 == MT::ThreadCreate( threads[ i ], &ContendingThreadFunc, &contention ) )
            {
                MT::ThreadDataCleanup( threads[ i ] );
                break;
            }
            ++created;
        }
        if ( created != ContendingThreadCount )
            ok = false;

        while ( contention.ready.Load() < created )
            MT::ThreadDelay( 1 );

        UInt64 start = MT::PrecisionTickCount();
        contention.go.Store( true, MT::MEMORDER_RELEASE );
        for ( UInt32 i=0; i<created; ++i )
        {
            if ( GUCEF_THREAD_WAIT_OK != MT::ThreadWait( threads[ i ], ThreadWaitLimitInMs ) )
                ok = false;
        }
        UInt64 end = MT::PrecisionTickCount();
        for ( UInt32 i=0; i<created; ++i )
            MT::ThreadDataCleanup( threads[ i ] );

        if ( 0 != contention.failures.Load() || created != ContendingThreadCount )
            ok = false;
        if ( created > 0 )
            nsPerPair[ runs++ ] = TicksToNs( end - start ) / (Float64) ( created * ContendedPairsPerThread );
    }

    if ( !ok )
        ERRORHERE;
    if ( runs > 0 )
        Report( what, nsPerPair, runs );
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformLockBenchmarks( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING lock BENCHMARKS (report only, GUCEF_TEST_MT_LOCK_BENCH=1)" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefMT.LockBenchmark" );

    GUCEF_TESTFW_TESTCASE( "LockBenchmark: Uncontended lock and unlock on one thread (report only, opt in)" )
    SKIP_UNLESS_BENCH();
    try
    {
        MeasureUncontended( "CMutex, uncontended", LOCKKIND_CMUTEX );
        MeasureUncontended( "C mutex, uncontended", LOCKKIND_C_MUTEX );
        MeasureUncontended( "C mutex re-entered (outer lock held), uncontended", LOCKKIND_C_MUTEX_REENTERED );
        MeasureUncontended( "CReadWriteLock writer, uncontended", LOCKKIND_RWLOCK_WRITER );
        MeasureUncontended( "CReadWriteLock reader, uncontended", LOCKKIND_RWLOCK_READER );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LockBenchmark: Lock and unlock contended by 4 threads (report only, opt in)" )
    SKIP_UNLESS_BENCH();
    try
    {
        MeasureContended( "CMutex, 4 threads", LOCKKIND_CMUTEX );
        MeasureContended( "C mutex, 4 threads", LOCKKIND_C_MUTEX );
        MeasureContended( "CReadWriteLock writer, 4 threads", LOCKKIND_RWLOCK_WRITER );
        MeasureContended( "CReadWriteLock reader, 4 threads", LOCKKIND_RWLOCK_READER );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
