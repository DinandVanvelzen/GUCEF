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

#if defined( __linux__ ) && !defined( __ANDROID__ ) && ( defined( __LP64__ ) || defined( _LP64 ) )
  #include <pthread.h>
  #include <sys/mman.h>
  #include <time.h>
  #define GUCEF_MT_TESTAPP_COLLIDING_HANDLES_SUPPORTED
#endif

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

#include "TestThreadIdentity.h"

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

const UInt32 DistinctThreadCount = 8;
const Int32 ThreadWaitLimitInMs = 60000;

/** What a thread started with MT::ThreadCreate() records about its own identity */
struct SIdentityRecord
{
    SIdentityRecord( void )
        : firstId( 0 )
        , secondId( 0 )
        , recorded( false )
        , release( false )
    {}

    TThreadId firstId;          /**< written by the thread before recorded is set, read by the creator after */
    TThreadId secondId;
    MT::CTAtomic< bool > recorded;
    MT::CTAtomic< bool > release;   /**< the thread keeps running until this is set, so all threads are alive at once */
};

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_CALLSPEC_PREFIX
RecordIdentityThreadFunc( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    SIdentityRecord* record = static_cast< SIdentityRecord* >( data );
    record->firstId = MT::GetCurrentThreadID();
    record->secondId = MT::GetCurrentThreadID();
    record->recorded.Store( true, MT::MEMORDER_RELEASE );
    while ( !record->release.Load( MT::MEMORDER_ACQUIRE ) )
        MT::ThreadDelay( 1 );
    return 0;
}

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_MT_TESTAPP_COLLIDING_HANDLES_SUPPORTED

const UInt64 FourGiB = (UInt64) 1 << 32;
const size_t WorkerStackSize = 2 * 1024 * 1024;
const UInt32 CommandWaitLimitInMs = 30000;
const UInt32 ReaderWaitTimeoutInMs = 200;

/** What the main thread asks a worker thread to do, one command at a time */
enum EWorkerCommand
{
    WORKERCMD_NONE = 0,
    WORKERCMD_RECORD_IDS,
    WORKERCMD_MUTEX_LOCK,
    WORKERCMD_MUTEX_UNLOCK,
    WORKERCMD_CMUTEX_LOCK,
    WORKERCMD_CMUTEX_UNLOCK,
    WORKERCMD_RW_READER_START,
    WORKERCMD_RW_READER_START_SHORT_WAIT,
    WORKERCMD_RW_READER_STOP,
    WORKERCMD_RW_WRITER_START,
    WORKERCMD_RW_WRITER_STOP,
    WORKERCMD_QUIT
};

/** The locks the workers operate on, owned by the test case */
struct SSharedLocks
{
    SSharedLocks( void )
        : mutex( MT::MutexCreate() )
        , cmutex()
        , rwLock( true )
    {}

    ~SSharedLocks()
    {
        if ( GUCEF_NULL != mutex )
            MT::MutexDestroy( mutex );
    }

    struct MT::SMutex* mutex;
    MT::CMutex cmutex;
    MT::CReadWriteLock rwLock;      /**< writers overrule, so a reader that has to wait for a writer honours its timeout */
};

/** One worker thread: it runs the commands the main thread posts and records the results */
struct SWorker
{
    SWorker( void )
        : command( (Int32) WORKERCMD_NONE )
        , requestSeq( 0 )
        , doneSeq( 0 )
        , locks( GUCEF_NULL )
        , firstId( 0 )
        , secondId( 0 )
        , nativeHandle( 0 )
        , lastResult( 0 )
        , nextSeq( 0 )
    {}

    MT::CTAtomic< Int32 > command;      /**< written by the main thread before requestSeq */
    MT::CTAtomic< UInt32 > requestSeq;  /**< the main thread posts a command by raising this */
    MT::CTAtomic< UInt32 > doneSeq;     /**< the worker reports the command done by setting this to requestSeq */
    SSharedLocks* locks;

    // results, written by the worker before doneSeq and read by the main thread after it
    TThreadId firstId;
    TThreadId secondId;
    UInt64 nativeHandle;
    UInt32 lastResult;

    UInt32 nextSeq;                     /**< used by the main thread only */
};

/*-------------------------------------------------------------------------*/

void
ExecuteWorkerCommand( SWorker& worker, EWorkerCommand command )
{
    SSharedLocks& locks = *worker.locks;
    switch ( command )
    {
        case WORKERCMD_RECORD_IDS:
        {
            worker.firstId = MT::GetCurrentThreadID();
            worker.secondId = MT::GetCurrentThreadID();
            worker.nativeHandle = (UInt64) ::pthread_self();
            break;
        }
        case WORKERCMD_MUTEX_LOCK:              { worker.lastResult = MT::MutexLock( locks.mutex, GUCEF_MUTEX_INFINITE_TIMEOUT ); break; }
        case WORKERCMD_MUTEX_UNLOCK:            { worker.lastResult = MT::MutexUnlock( locks.mutex ); break; }
        case WORKERCMD_CMUTEX_LOCK:             { worker.lastResult = (UInt32) locks.cmutex.Lock(); break; }
        case WORKERCMD_CMUTEX_UNLOCK:           { worker.lastResult = (UInt32) locks.cmutex.Unlock(); break; }
        case WORKERCMD_RW_READER_START:         { worker.lastResult = (UInt32) locks.rwLock.ReaderStart(); break; }
        case WORKERCMD_RW_READER_START_SHORT_WAIT: { worker.lastResult = (UInt32) locks.rwLock.ReaderStart( ReaderWaitTimeoutInMs ); break; }
        case WORKERCMD_RW_READER_STOP:          { worker.lastResult = (UInt32) locks.rwLock.ReaderStop(); break; }
        case WORKERCMD_RW_WRITER_START:         { worker.lastResult = (UInt32) locks.rwLock.WriterStart(); break; }
        case WORKERCMD_RW_WRITER_STOP:          { worker.lastResult = (UInt32) locks.rwLock.WriterStop(); break; }
        default:                                { break; }
    }
}

/*-------------------------------------------------------------------------*/

void*
WorkerThreadMain( void* data )
{
    SWorker* worker = static_cast< SWorker* >( data );
    UInt32 handledSeq = 0;
    while ( true )
    {
        UInt32 seq = worker->requestSeq.Load( MT::MEMORDER_ACQUIRE );
        if ( seq != handledSeq )
        {
            EWorkerCommand command = (EWorkerCommand) worker->command.Load( MT::MEMORDER_RELAXED );
            ExecuteWorkerCommand( *worker, command );
            handledSeq = seq;
            worker->doneSeq.Store( seq, MT::MEMORDER_RELEASE );
            if ( WORKERCMD_QUIT == command )
                return GUCEF_NULL;
        }
        else
        {
            MT::ThreadDelay( 1 );
        }
    }
}

/*-------------------------------------------------------------------------*/

/**
 *  Two live threads whose native handles (pthread_t) differ by exactly 4 GiB.
 *  glibc keeps the thread descriptor, which pthread_t points to, at a fixed offset from the top of the stack the caller
 *  supplies, so two stacks of the same size placed exactly 4 GiB apart give two handles 4 GiB apart: a 32 bit
 *  truncation of those handles is the same number for both threads.
 *  The address range is reserved without backing memory; only the two stacks are made accessible.
 */
class CCollidingThreadPair
{
    public:

    CCollidingThreadPair( SSharedLocks& locks )
        : m_region( MAP_FAILED )
        , m_regionSize( (size_t) FourGiB + WorkerStackSize )
        , m_threadsCreated( 0 )
        , m_threadsStuck( false )
        , m_colliding( false )
        , m_skipReason( "not set up" )
    {
        m_workers[ 0 ].locks = &locks;
        m_workers[ 1 ].locks = &locks;

        m_region = ::mmap( GUCEF_NULL, m_regionSize, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0 );
        if ( MAP_FAILED == m_region )
        {
            m_skipReason = "a 4 GiB address range could not be reserved";
            return;
        }

        void* stacks[ 2 ];
        stacks[ 0 ] = m_region;
        stacks[ 1 ] = static_cast< char* >( m_region ) + FourGiB;
        for ( UInt32 i=0; i<2; ++i )
        {
            if ( 0 != ::mprotect( stacks[ i ], WorkerStackSize, PROT_READ | PROT_WRITE ) )
            {
                m_skipReason = "the thread stacks inside the reserved range could not be made accessible";
                return;
            }
        }

        for ( UInt32 i=0; i<2; ++i )
        {
            pthread_attr_t attr;
            if ( 0 != ::pthread_attr_init( &attr ) )
            {
                m_skipReason = "pthread_attr_init failed";
                return;
            }
            int result = ::pthread_attr_setstack( &attr, stacks[ i ], WorkerStackSize );
            if ( 0 == result )
                result = ::pthread_create( &m_threads[ i ], &attr, &WorkerThreadMain, &m_workers[ i ] );
            ::pthread_attr_destroy( &attr );
            if ( 0 != result )
            {
                m_skipReason = "a thread with a caller supplied stack could not be created";
                return;
            }
            ++m_threadsCreated;
        }

        if ( !Run( 0, WORKERCMD_RECORD_IDS ) || !Run( 1, WORKERCMD_RECORD_IDS ) )
        {
            m_skipReason = "the worker threads did not respond";
            return;
        }

        const UInt64 handleA = m_workers[ 0 ].nativeHandle;
        const UInt64 handleB = m_workers[ 1 ].nativeHandle;
        if ( handleA == handleB || ( handleA & 0xFFFFFFFF ) != ( handleB & 0xFFFFFFFF ) )
        {
            m_skipReason = "the C library did not place the two thread handles a multiple of 4 GiB apart";
            return;
        }
        m_colliding = true;
        m_skipReason = "";
    }

    ~CCollidingThreadPair()
    {
        for ( UInt32 i=0; i<m_threadsCreated; ++i )
        {
            if ( !Run( i, WORKERCMD_QUIT ) )
                m_threadsStuck = true;
        }
        for ( UInt32 i=0; i<m_threadsCreated; ++i )
        {
            struct timespec deadline;
            ::clock_gettime( CLOCK_REALTIME, &deadline );
            deadline.tv_sec += 10;
            if ( 0 != ::pthread_timedjoin_np( m_threads[ i ], GUCEF_NULL, &deadline ) )
                m_threadsStuck = true;
        }

        // a thread that is still running uses its stack inside the region: leave the region mapped then
        if ( MAP_FAILED != m_region && !m_threadsStuck )
            ::munmap( m_region, m_regionSize );
    }

    bool IsColliding( void ) const
    {
        return m_colliding;
    }

    const char* GetSkipReason( void ) const
    {
        return m_skipReason;
    }

    /**
     *  Has worker 'index' run the command and waits for it to finish
     *  @return false if the worker did not finish the command in time
     */
    bool Run( UInt32 index, EWorkerCommand command )
    {
        SWorker& worker = m_workers[ index ];
        const UInt32 seq = ++worker.nextSeq;
        worker.command.Store( (Int32) command, MT::MEMORDER_RELAXED );
        worker.requestSeq.Store( seq, MT::MEMORDER_RELEASE );
        for ( UInt32 waited=0; waited<CommandWaitLimitInMs; ++waited )
        {
            if ( seq == worker.doneSeq.Load( MT::MEMORDER_ACQUIRE ) )
                return true;
            MT::ThreadDelay( 1 );
        }
        return false;
    }

    /** Runs the command on worker 'index' and gives its result, or GUCEF_UINT32MAX if the worker did not finish it */
    UInt32 RunForResult( UInt32 index, EWorkerCommand command )
    {
        if ( !Run( index, command ) )
            return GUCEF_UINT32MAX;
        return m_workers[ index ].lastResult;
    }

    const SWorker& GetWorker( UInt32 index ) const
    {
        return m_workers[ index ];
    }

    private:

    CCollidingThreadPair( const CCollidingThreadPair& src );
    CCollidingThreadPair& operator=( const CCollidingThreadPair& src );

    void* m_region;
    size_t m_regionSize;
    pthread_t m_threads[ 2 ];
    SWorker m_workers[ 2 ];
    UInt32 m_threadsCreated;
    bool m_threadsStuck;
    bool m_colliding;
    const char* m_skipReason;
};

#endif /* GUCEF_MT_TESTAPP_COLLIDING_HANDLES_SUPPORTED ? */

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformThreadIdentityTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING ThreadIdentity TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefMT.ThreadIdentity" );

    GUCEF_TESTFW_TESTCASE( "GetCurrentThreadID is stable on one thread and differs between live threads" )
    try
    {
        const TThreadId mainId = MT::GetCurrentThreadID();
        ASSERT_TRUE( 0 != mainId );
        ASSERT_TRUE( mainId == MT::GetCurrentThreadID() );

        SIdentityRecord records[ DistinctThreadCount ];
        MT::SThreadData* threads[ DistinctThreadCount ];
        UInt32 created = 0;
        for ( UInt32 i=0; i<DistinctThreadCount; ++i )
        {
            threads[ i ] = MT::ThreadDataReserve();
            ASSERT_TRUE( GUCEF_NULL != threads[ i ] );
            if ( GUCEF_NULL == threads[ i ] )
                break;
            if ( 0 == MT::ThreadCreate( threads[ i ], &RecordIdentityThreadFunc, &records[ i ] ) )
            {
                ERRORHERE;
                MT::ThreadDataCleanup( threads[ i ] );
                break;
            }
            ++created;
        }

        for ( UInt32 i=0; i<created; ++i )
        {
            for ( UInt32 waited=0; waited<(UInt32)ThreadWaitLimitInMs && !records[ i ].recorded.Load( MT::MEMORDER_ACQUIRE ); ++waited )
                MT::ThreadDelay( 1 );
            ASSERT_TRUE( records[ i ].recorded.Load( MT::MEMORDER_ACQUIRE ) );
        }

        // all threads are alive at this point: no id may be reused
        for ( UInt32 i=0; i<created; ++i )
        {
            ASSERT_TRUE( 0 != records[ i ].firstId );
            ASSERT_TRUE( records[ i ].firstId == records[ i ].secondId );
            ASSERT_TRUE( records[ i ].firstId != mainId );
            for ( UInt32 n=i+1; n<created; ++n )
                ASSERT_TRUE( records[ i ].firstId != records[ n ].firstId );
        }

        for ( UInt32 i=0; i<created; ++i )
            records[ i ].release.Store( true, MT::MEMORDER_RELEASE );
        for ( UInt32 i=0; i<created; ++i )
        {
            ASSERT_TRUE( GUCEF_THREAD_WAIT_OK == MT::ThreadWait( threads[ i ], ThreadWaitLimitInMs ) );
            MT::ThreadDataCleanup( threads[ i ] );
        }
        ASSERT_TRUE( mainId == MT::GetCurrentThreadID() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    #ifdef GUCEF_MT_TESTAPP_COLLIDING_HANDLES_SUPPORTED

    GUCEF_TESTFW_TESTCASE( "Threads whose native handles are 4 GiB apart get different ids" )
    SSharedLocks locks;
    CCollidingThreadPair pair( locks );
    GUCEF_TESTFW_SKIP_IF( !pair.IsColliding(), pair.GetSkipReason() );
    try
    {
        const SWorker& a = pair.GetWorker( 0 );
        const SWorker& b = pair.GetWorker( 1 );
        ASSERT_TRUE( 0 != a.firstId );
        ASSERT_TRUE( 0 != b.firstId );
        ASSERT_TRUE( a.firstId == a.secondId );
        ASSERT_TRUE( b.firstId == b.secondId );
        ASSERT_TRUE( a.firstId != b.firstId );
        ASSERT_TRUE( a.firstId != MT::GetCurrentThreadID() );
        ASSERT_TRUE( b.firstId != MT::GetCurrentThreadID() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A mutex held by one of the 4 GiB apart threads is not treated as held by the other" )
    SSharedLocks locks;
    CCollidingThreadPair pair( locks );
    GUCEF_TESTFW_SKIP_IF( !pair.IsColliding(), pair.GetSkipReason() );
    try
    {
        // C level mutex: an unlock by a thread that does not hold it must leave the owner's bookkeeping alone
        ASSERT_TRUE( GUCEF_MUTEX_OPERATION_SUCCESS == pair.RunForResult( 0, WORKERCMD_MUTEX_LOCK ) );
        ASSERT_TRUE( 1 == MT::MutexLocked( locks.mutex ) );
        ASSERT_TRUE( GUCEF_MUTEX_OPERATION_FAILED == pair.RunForResult( 1, WORKERCMD_MUTEX_UNLOCK ) );
        ASSERT_TRUE( 1 == MT::MutexLocked( locks.mutex ) );
        ASSERT_TRUE( 0 == MT::MutexReentrancy( locks.mutex ) );
        ASSERT_TRUE( GUCEF_MUTEX_OPERATION_SUCCESS == pair.RunForResult( 0, WORKERCMD_MUTEX_UNLOCK ) );
        ASSERT_TRUE( 0 == MT::MutexLocked( locks.mutex ) );

        // CMutex: the recorded owner is the thread that took the lock, not the other one
        ASSERT_TRUE( (UInt32) MT::LOCKSTATUS_OPERATION_SUCCESS == pair.RunForResult( 0, WORKERCMD_CMUTEX_LOCK ) );
        ASSERT_TRUE( pair.GetWorker( 0 ).firstId == locks.cmutex.GetThreadIdOfThreadLastHoldingTheLock() );
        ASSERT_TRUE( pair.GetWorker( 1 ).firstId != locks.cmutex.GetThreadIdOfThreadLastHoldingTheLock() );
        ASSERT_TRUE( (UInt32) MT::LOCKSTATUS_OPERATION_FAILED == pair.RunForResult( 1, WORKERCMD_CMUTEX_UNLOCK ) );
        ASSERT_TRUE( locks.cmutex.IsLocked() );
        ASSERT_TRUE( (UInt32) MT::LOCKSTATUS_OPERATION_SUCCESS == pair.RunForResult( 0, WORKERCMD_CMUTEX_UNLOCK ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Re-entrant locking by one of the 4 GiB apart threads is not granted to the other" )
    SSharedLocks locks;
    CCollidingThreadPair pair( locks );
    GUCEF_TESTFW_SKIP_IF( !pair.IsColliding(), pair.GetSkipReason() );
    try
    {
        // C level mutex: the re-entrancy count belongs to the owner only
        ASSERT_TRUE( GUCEF_MUTEX_OPERATION_SUCCESS == pair.RunForResult( 0, WORKERCMD_MUTEX_LOCK ) );
        ASSERT_TRUE( GUCEF_MUTEX_OPERATION_SUCCESS == pair.RunForResult( 0, WORKERCMD_MUTEX_LOCK ) );
        ASSERT_TRUE( 1 == MT::MutexReentrancy( locks.mutex ) );
        ASSERT_TRUE( GUCEF_MUTEX_OPERATION_FAILED == pair.RunForResult( 1, WORKERCMD_MUTEX_UNLOCK ) );
        ASSERT_TRUE( 1 == MT::MutexReentrancy( locks.mutex ) );
        ASSERT_TRUE( 1 == MT::MutexLocked( locks.mutex ) );
        ASSERT_TRUE( GUCEF_MUTEX_OPERATION_SUCCESS == pair.RunForResult( 0, WORKERCMD_MUTEX_UNLOCK ) );
        ASSERT_TRUE( 0 == MT::MutexReentrancy( locks.mutex ) );
        ASSERT_TRUE( 1 == MT::MutexLocked( locks.mutex ) );
        ASSERT_TRUE( GUCEF_MUTEX_OPERATION_FAILED == pair.RunForResult( 1, WORKERCMD_MUTEX_UNLOCK ) );
        ASSERT_TRUE( 1 == MT::MutexLocked( locks.mutex ) );
        ASSERT_TRUE( GUCEF_MUTEX_OPERATION_SUCCESS == pair.RunForResult( 0, WORKERCMD_MUTEX_UNLOCK ) );
        ASSERT_TRUE( 0 == MT::MutexLocked( locks.mutex ) );

        // read/write lock: the writer itself may read and write again, the other thread has to wait
        ASSERT_TRUE( (UInt32) MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == pair.RunForResult( 0, WORKERCMD_RW_WRITER_START ) );
        ASSERT_TRUE( (UInt32) MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == pair.RunForResult( 0, WORKERCMD_RW_WRITER_START ) );
        ASSERT_TRUE( 1 == locks.rwLock.ActiveWriterCount() );
        ASSERT_TRUE( (UInt32) MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == pair.RunForResult( 0, WORKERCMD_RW_READER_START ) );
        ASSERT_TRUE( (UInt32) MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == pair.RunForResult( 0, WORKERCMD_RW_READER_STOP ) );

        const UInt32 otherReaderResult = pair.RunForResult( 1, WORKERCMD_RW_READER_START_SHORT_WAIT );
        ASSERT_TRUE( (UInt32) MT::CReadWriteLock::RWLOCK_WAIT_TIMEOUT == otherReaderResult );
        if ( (UInt32) MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == otherReaderResult )
            pair.Run( 1, WORKERCMD_RW_READER_STOP );
        ASSERT_TRUE( 0 == locks.rwLock.ActiveReaderCount() );
        ASSERT_TRUE( 1 == locks.rwLock.ActiveWriterCount() );

        ASSERT_TRUE( (UInt32) MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == pair.RunForResult( 0, WORKERCMD_RW_WRITER_STOP ) );
        ASSERT_TRUE( (UInt32) MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == pair.RunForResult( 0, WORKERCMD_RW_WRITER_STOP ) );
        ASSERT_TRUE( 0 == locks.rwLock.ActiveWriterCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Read/write lock reader bookkeeping keeps the 4 GiB apart threads apart" )
    SSharedLocks locks;
    CCollidingThreadPair pair( locks );
    GUCEF_TESTFW_SKIP_IF( !pair.IsColliding(), pair.GetSkipReason() );
    try
    {
        const TThreadId idA = pair.GetWorker( 0 ).firstId;
        const TThreadId idB = pair.GetWorker( 1 ).firstId;

        // the depth counts re-entries: 0 for a thread that holds one read lock and for a thread that holds none
        ASSERT_TRUE( (UInt32) MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == pair.RunForResult( 0, WORKERCMD_RW_READER_START ) );
        ASSERT_TRUE( (UInt32) MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == pair.RunForResult( 0, WORKERCMD_RW_READER_START ) );
        ASSERT_TRUE( 1 == locks.rwLock.ActiveReaderCount() );
        ASSERT_TRUE( 1 == locks.rwLock.ActiveReaderReentrancyDepth( idA ) );
        ASSERT_TRUE( 0 == locks.rwLock.ActiveReaderReentrancyDepth( idB ) );

        ASSERT_TRUE( (UInt32) MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == pair.RunForResult( 1, WORKERCMD_RW_READER_START ) );
        ASSERT_TRUE( 2 == locks.rwLock.ActiveReaderCount() );
        ASSERT_TRUE( 1 == locks.rwLock.ActiveReaderReentrancyDepth( idA ) );
        ASSERT_TRUE( 0 == locks.rwLock.ActiveReaderReentrancyDepth( idB ) );

        ASSERT_TRUE( (UInt32) MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == pair.RunForResult( 1, WORKERCMD_RW_READER_STOP ) );
        ASSERT_TRUE( 1 == locks.rwLock.ActiveReaderCount() );
        ASSERT_TRUE( 1 == locks.rwLock.ActiveReaderReentrancyDepth( idA ) );

        ASSERT_TRUE( (UInt32) MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == pair.RunForResult( 0, WORKERCMD_RW_READER_STOP ) );
        ASSERT_TRUE( 1 == locks.rwLock.ActiveReaderCount() );
        ASSERT_TRUE( 0 == locks.rwLock.ActiveReaderReentrancyDepth( idA ) );
        ASSERT_TRUE( (UInt32) MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == pair.RunForResult( 0, WORKERCMD_RW_READER_STOP ) );
        ASSERT_TRUE( 0 == locks.rwLock.ActiveReaderCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    #else

    GUCEF_TESTFW_TESTCASE( "Threads whose native handles are 4 GiB apart get different ids" )
    GUCEF_TESTFW_SKIP( "two thread handles 4 GiB apart are only arranged on 64 bit Linux with glibc" );
    GUCEF_TESTFW_TESTCASE_END

    #endif /* GUCEF_MT_TESTAPP_COLLIDING_HANDLES_SUPPORTED ? */
}

/*-------------------------------------------------------------------------*/
