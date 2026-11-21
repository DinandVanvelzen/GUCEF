/*
 *  gucefMT_TestApp: GUCEF test application for the MT module
 *  Copyright (C) 2002 - 2022.  Dinand Vanvelzen
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

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_MUTEX_H
#include "gucefMT_mutex.h"
#define GUCEF_MT_MUTEX_H
#endif /* GUCEF_MT_MUTEX_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CREADWRITELOCK_H
#include "gucefMT_CReadWriteLock.h"
#define GUCEF_MT_CREADWRITELOCK_H
#endif /* GUCEF_MT_CREADWRITELOCK_H ? */

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

#ifndef GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H
#include "gucefMT_CObjectScopeReadOnlyLock.h"
#define GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H
#endif /* GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestMutex.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*
 *  All Lock() calls in test code MUST use this bounded timeout.
 *  If a broken mutex never returns, the test fails with LOCKSTATUS_WAIT_TIMEOUT
 *  instead of hanging the test process forever.
 *  5 seconds is far more than any correct lock acquisition should ever take.
 */
#define MUTEX_TEST_LOCK_TIMEOUT_IN_MS   5000

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Active object used by the multi-threaded mutex tests.
 *  The thread tries to acquire a mutex and records whether it succeeded
 *  within the allotted window.
 */
class CMutexTestThread : public MT::CActiveObject
{
    public:

    CMutexTestThread( MT::CMutex& mutex )
        : MT::CActiveObject()
        , m_mutex( mutex )
        , m_threadStarted( false )
        , m_lockAcquired( false )
        , m_lockReleased( false )
        , m_done( false )
    {}

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        m_threadStarted = true;
        return true;
    }

    virtual void OnThreadStarted( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
    }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( !m_done )
        {
            // Block until the mutex is available, bounded so we never hang forever
            MT::TLockStatus lockStatus = m_mutex.Lock( MUTEX_TEST_LOCK_TIMEOUT_IN_MS );
            if ( MT::LOCKSTATUS_OPERATION_SUCCESS == lockStatus )
            {
                m_lockAcquired = true;
                MT::PrecisionDelay( 5 );
                m_mutex.Unlock();
                m_lockReleased = true;
            }
            m_done = true;
        }
        return true; // stop cycling
    }

    virtual void OnThreadEnding( void* taskdata, bool willBeForced ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    bool WasThreadStarted( void ) const { return m_threadStarted; }
    bool WasLockAcquired( void ) const  { return m_lockAcquired; }
    bool WasLockReleased( void ) const  { return m_lockReleased; }
    bool IsDone( void ) const           { return m_done; }

    private:

    MT::CMutex& m_mutex;
    volatile bool m_threadStarted;
    volatile bool m_lockAcquired;
    volatile bool m_lockReleased;
    volatile bool m_done;
};

/**
 *  Active object which samples IsLocked() of a mutex from its own thread
 */
class CMutexIsLockedProbeThread : public MT::CActiveObject
{
    public:

    CMutexIsLockedProbeThread( MT::CMutex& mutex )
        : MT::CActiveObject()
        , m_mutex( mutex )
        , m_sawLocked( false )
    {}

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        return true;
    }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        m_sawLocked = m_mutex.IsLocked();
        return true; // stop cycling
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    bool SawLocked( void ) const { return m_sawLocked; }

    /** returns whether the thread finished within the given time */
    bool WaitForCompletion( UInt32 timeoutInMs )
    {
        UInt32 waitedInMs = 0;
        while ( IsActive() && waitedInMs < timeoutInMs )
        {
            MT::PrecisionDelay( 5 );
            waitedInMs += 5;
        }
        return !IsActive();
    }

    private:

    MT::CMutex& m_mutex;
    volatile bool m_sawLocked;
};

/**
 *  Active object which locks a mutex and keeps holding it until told to release it.
 *  Used to have a lock owned by a thread other than the one calling Unlock().
 */
class CMutexHolderThread : public MT::CActiveObject
{
    public:

    CMutexHolderThread( MT::CMutex& mutex )
        : MT::CActiveObject()
        , m_mutex( mutex )
        , m_holding( false )
        , m_release( false )
    {}

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        return true;
    }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( MT::LOCKSTATUS_OPERATION_SUCCESS == m_mutex.Lock( MUTEX_TEST_LOCK_TIMEOUT_IN_MS ) )
        {
            m_holding = true;
            MT::UInt32 waitedInMs = 0;
            while ( !m_release && waitedInMs < MUTEX_TEST_LOCK_TIMEOUT_IN_MS )
            {
                MT::PrecisionDelay( 5 );
                waitedInMs += 5;
            }
            m_mutex.Unlock();
            m_holding = false;
        }
        return true; // stop cycling
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    bool WaitUntilHolding( MT::UInt32 timeoutInMs )
    {
        MT::UInt32 waitedInMs = 0;
        while ( !m_holding && waitedInMs < timeoutInMs )
        {
            MT::PrecisionDelay( 5 );
            waitedInMs += 5;
        }
        return m_holding;
    }

    void Release( void ) { m_release = true; }

    private:

    MT::CMutex& m_mutex;
    volatile bool m_holding;
    volatile bool m_release;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  pthreads reports failure as a positive errno value. An unlock that the OS
 *  refused (unlocked mutex, or a mutex owned by another thread) must be
 *  reported as a failure and must leave the mutex state untouched.
 */
static void
PerformFailedUnlockIsReportedTest( void )
{
    GUCEF_TESTFW_TESTCASE( "Mutex: Unlock of a mutex that is not locked reports failure" )
    try
    {
        MT::CMutex mutex;

        // Never locked
        ASSERT_TRUE( MT::LOCKSTATUS_OPERATION_FAILED == mutex.Unlock() );
        ASSERT_FALSE( mutex.IsLocked() );

        // Balanced lock/unlock, then one unlock too many
        ASSERT_TRUE( MT::LOCKSTATUS_OPERATION_SUCCESS == mutex.Lock( MUTEX_TEST_LOCK_TIMEOUT_IN_MS ) );
        ASSERT_TRUE( MT::LOCKSTATUS_OPERATION_SUCCESS == mutex.Unlock() );
        ASSERT_TRUE( MT::LOCKSTATUS_OPERATION_FAILED == mutex.Unlock() );

        // The failed unlocks must not have corrupted the mutex
        ASSERT_TRUE( MT::LOCKSTATUS_OPERATION_SUCCESS == mutex.Lock( MUTEX_TEST_LOCK_TIMEOUT_IN_MS ) );
        ASSERT_TRUE( MT::LOCKSTATUS_OPERATION_SUCCESS == mutex.Unlock() );
        ASSERT_FALSE( mutex.IsLocked() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mutex: Unlock from a thread that does not own the lock reports failure" )
    try
    {
        MT::CMutex mutex;
        CMutexHolderThread holder( mutex );
        ASSERT_TRUE( holder.Activate( GUCEF_NULL, true ) );
        ASSERT_TRUE( holder.WaitUntilHolding( MUTEX_TEST_LOCK_TIMEOUT_IN_MS ) );

        // We are not the owner, the unlock must be refused and reported as such
        ASSERT_TRUE( MT::LOCKSTATUS_OPERATION_FAILED == mutex.Unlock() );

        // The owner still holds it
        ASSERT_TRUE( mutex.IsLocked() );

        holder.Release();
        MT::UInt32 waitResult = holder.WaitForThreadToFinish( 5000 );
        ASSERT_TRUE( GUCEF_THREAD_WAIT_OK == waitResult || GUCEF_THREAD_WAIT_ABANDONEND == waitResult );
        ASSERT_FALSE( mutex.IsLocked() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mutex: C mutex unlock of an unlocked mutex reports failure" )
    try
    {
        struct MT::SMutex* mutex = MT::MutexCreate();
        ASSERT_TRUE( GUCEF_NULL != mutex );
        if ( GUCEF_NULL != mutex )
        {
            ASSERT_TRUE( GUCEF_MUTEX_OPERATION_FAILED == MT::MutexUnlock( mutex ) );
            ASSERT_TRUE( GUCEF_MUTEX_OPERATION_SUCCESS == MT::MutexLock( mutex, MUTEX_TEST_LOCK_TIMEOUT_IN_MS ) );
            ASSERT_TRUE( GUCEF_MUTEX_OPERATION_SUCCESS == MT::MutexUnlock( mutex ) );
            ASSERT_TRUE( GUCEF_MUTEX_OPERATION_FAILED == MT::MutexUnlock( mutex ) );
            ASSERT_TRUE( 0 == MT::MutexLocked( mutex ) );
            MT::MutexDestroy( mutex );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformBasicMutexTests( void )
{
    GUCEF_TESTFW_TESTCASE( "Mutex: Basic single-threaded lock and unlock" )
    try
    {
        MT::CMutex mutex;

        // Initial state: should not be locked
        ASSERT_FALSE( mutex.IsLocked() );

        // Lock — bounded timeout: if broken, times out and fails the test rather than hanging
        MT::TLockStatus lockResult = mutex.Lock( MUTEX_TEST_LOCK_TIMEOUT_IN_MS );
        ASSERT_TRUE( MT::LOCKSTATUS_OPERATION_SUCCESS == lockResult );

        // After lock: should report locked
        // Note: IsLocked() via TryEnterCriticalSection is re-entrant for the same thread,
        // so it may report unlocked from the owning thread. We do not assert IsLocked() here.

        // Unlock
        MT::TLockStatus unlockResult = mutex.Unlock();
        ASSERT_TRUE( MT::LOCKSTATUS_OPERATION_SUCCESS == unlockResult );

        // After unlock: should not be locked
        ASSERT_FALSE( mutex.IsLocked() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformScopeMutexTests( void )
{
    GUCEF_TESTFW_TESTCASE( "Mutex: CScopeMutex RAII lock/unlock" )
    try
    {
        MT::CMutex mutex;
        ASSERT_FALSE( mutex.IsLocked() );

        {
            MT::CScopeMutex scopeLock( mutex );
            ASSERT_TRUE( scopeLock.IsLocked() );
        }

        // Scope exited: mutex should now be released
        ASSERT_FALSE( mutex.IsLocked() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/**
 *  Specifically tests the bug that was fixed: a bool isLocked guard in Unlock()
 *  would skip LeaveCriticalSection on the second call when the same thread
 *  had locked the mutex recursively, leaving it permanently locked.
 */
static void
PerformRecursiveMutexTests( void )
{
    GUCEF_TESTFW_TESTCASE( "Mutex: Recursive lock/unlock by same thread" )
    try
    {
        MT::CMutex mutex;
        ASSERT_FALSE( mutex.IsLocked() );

        // First lock
        MT::TLockStatus lock1 = mutex.Lock( MUTEX_TEST_LOCK_TIMEOUT_IN_MS );
        ASSERT_TRUE( MT::LOCKSTATUS_OPERATION_SUCCESS == lock1 );

        // Second (recursive) lock — critical sections allow this
        MT::TLockStatus lock2 = mutex.Lock( MUTEX_TEST_LOCK_TIMEOUT_IN_MS );
        ASSERT_TRUE( MT::LOCKSTATUS_OPERATION_SUCCESS == lock2 );

        // First unlock — CS count goes from 2 to 1, still held
        MT::TLockStatus unlock1 = mutex.Unlock();
        ASSERT_TRUE( MT::LOCKSTATUS_OPERATION_SUCCESS == unlock1 );

        // Second unlock — CS count goes from 1 to 0, now fully released
        // Prior to the fix, the isLocked bool guard caused this call to
        // skip LeaveCriticalSection, leaving the CS permanently locked.
        MT::TLockStatus unlock2 = mutex.Unlock();
        ASSERT_TRUE( MT::LOCKSTATUS_OPERATION_SUCCESS == unlock2 );

        // Mutex should now be fully released; another lock must succeed immediately
        MT::TLockStatus lock3 = mutex.Lock( 0 );   // timeout=0: instant fail if locked
        ASSERT_TRUE( MT::LOCKSTATUS_OPERATION_SUCCESS == lock3 );
        mutex.Unlock();

        ASSERT_FALSE( mutex.IsLocked() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformMutexTimeoutTest( void )
{
    GUCEF_TESTFW_TESTCASE( "Mutex: Lock timeout when mutex is already held" )
    try
    {
        MT::CMutex mutex;

        // Acquire the lock
        MT::TLockStatus lockResult = mutex.Lock( MUTEX_TEST_LOCK_TIMEOUT_IN_MS );
        ASSERT_TRUE( MT::LOCKSTATUS_OPERATION_SUCCESS == lockResult );

        // Spin up a thread to try acquiring with a short timeout — must time out
        CMutexTestThread thread( mutex );
        // We'll use a very short timeout directly on the mutex from this thread
        // to avoid needing thread synchronization here.
        // Lock with timeout=1ms — should time out since we already hold the lock
        // (Note: TryEnterCriticalSection is re-entrant for same thread, so we
        //  test this from a second thread below. Here we verify the status enum.)

        // Release the lock
        MT::TLockStatus unlockResult = mutex.Unlock();
        ASSERT_TRUE( MT::LOCKSTATUS_OPERATION_SUCCESS == unlockResult );

        // After unlock, immediate lock with zero timeout must now succeed
        MT::TLockStatus lock2 = mutex.Lock( 0 );
        ASSERT_TRUE( MT::LOCKSTATUS_OPERATION_SUCCESS == lock2 );
        mutex.Unlock();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/**
 *  Tests that a second thread is truly blocked while the first thread holds
 *  the mutex, and that it unblocks once the first thread releases.
 *  This is the primary real-world scenario where the CS bug manifested.
 */
static void
PerformMultiThreadedMutexTests( void )
{
    GUCEF_TESTFW_TESTCASE( "Mutex: Multi-threaded: blocked thread unblocks on release" )
    try
    {
        MT::CMutex mutex;

        // Acquire the lock on the main thread
        MT::TLockStatus lockResult = mutex.Lock( MUTEX_TEST_LOCK_TIMEOUT_IN_MS );
        ASSERT_TRUE( MT::LOCKSTATUS_OPERATION_SUCCESS == lockResult );

        // Start a worker thread that will block waiting for the mutex
        CMutexTestThread thread( mutex );
        ASSERT_TRUE( thread.Activate( GUCEF_NULL, true ) );

        // Wait for the thread to start
        MT::UInt32 waitMs = 0;
        while ( !thread.WasThreadStarted() && waitMs < 5000 )
        {
            MT::PrecisionDelay( 10 );
            waitMs += 10;
        }
        ASSERT_TRUE( thread.WasThreadStarted() );

        // Give the thread a moment to hit the blocking Lock() call
        MT::PrecisionDelay( 100 );

        // Thread should still be blocked (not done yet)
        ASSERT_FALSE( thread.IsDone() );

        // Release the mutex — thread should now be able to proceed
        MT::TLockStatus unlockResult = mutex.Unlock();
        ASSERT_TRUE( MT::LOCKSTATUS_OPERATION_SUCCESS == unlockResult );

        // Wait for thread to finish (up to 5 seconds)
        MT::UInt32 waitResult = thread.WaitForThreadToFinish( 5000 );
        ASSERT_TRUE( GUCEF_THREAD_WAIT_OK == waitResult || GUCEF_THREAD_WAIT_ABANDONEND == waitResult );

        // The thread must have acquired and released the lock
        ASSERT_TRUE( thread.WasLockAcquired() );
        ASSERT_TRUE( thread.WasLockReleased() );

        // Mutex must be fully released
        ASSERT_FALSE( mutex.IsLocked() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/**
 *  Tests that after recursive locking/unlocking by the main thread,
 *  a waiting second thread correctly unblocks.
 *  This is the exact failure scenario introduced by the isLocked bool bug.
 */
static void
PerformRecursiveUnlockUnblocksOtherThreadTest( void )
{
    GUCEF_TESTFW_TESTCASE( "Mutex: Recursive unlock correctly unblocks a waiting thread" )
    try
    {
        MT::CMutex mutex;

        // Acquire the lock twice (recursive)
        ASSERT_TRUE( MT::LOCKSTATUS_OPERATION_SUCCESS == mutex.Lock( MUTEX_TEST_LOCK_TIMEOUT_IN_MS ) );
        ASSERT_TRUE( MT::LOCKSTATUS_OPERATION_SUCCESS == mutex.Lock( MUTEX_TEST_LOCK_TIMEOUT_IN_MS ) );

        // Start a worker that will block on the mutex
        CMutexTestThread thread( mutex );
        ASSERT_TRUE( thread.Activate( GUCEF_NULL, true ) );

        // Wait for thread to start and reach its Lock() call
        MT::UInt32 waitMs = 0;
        while ( !thread.WasThreadStarted() && waitMs < 5000 )
        {
            MT::PrecisionDelay( 10 );
            waitMs += 10;
        }
        ASSERT_TRUE( thread.WasThreadStarted() );
        MT::PrecisionDelay( 100 );
        ASSERT_FALSE( thread.IsDone() );

        // First unlock — still held (recursive count = 1)
        ASSERT_TRUE( MT::LOCKSTATUS_OPERATION_SUCCESS == mutex.Unlock() );

        // Thread must still be blocked
        MT::PrecisionDelay( 50 );
        ASSERT_FALSE( thread.IsDone() );

        // Second unlock — fully released (recursive count = 0)
        // Prior to the fix this would silently skip LeaveCriticalSection,
        // leaving the thread blocked forever.
        ASSERT_TRUE( MT::LOCKSTATUS_OPERATION_SUCCESS == mutex.Unlock() );

        // Thread must now unblock and complete within a reasonable time
        MT::UInt32 waitResult = thread.WaitForThreadToFinish( 5000 );
        ASSERT_TRUE( GUCEF_THREAD_WAIT_OK == waitResult || GUCEF_THREAD_WAIT_ABANDONEND == waitResult );

        ASSERT_TRUE( thread.WasLockAcquired() );
        ASSERT_TRUE( thread.WasLockReleased() );
        ASSERT_FALSE( mutex.IsLocked() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/**
 *  Regression: on Linux IsLocked() used to report the inverse of the actual lock state
 */
static void
PerformIsLockedRegressionTest( void )
{
    GUCEF_TESTFW_TESTCASE( "Mutex: IsLocked reports a lock held by another thread" )
    try
    {
        MT::CMutex mutex;

        CMutexIsLockedProbeThread probeWhileUnlocked( mutex );
        ASSERT_TRUE( probeWhileUnlocked.Activate() );
        ASSERT_TRUE( probeWhileUnlocked.WaitForCompletion( MUTEX_TEST_LOCK_TIMEOUT_IN_MS ) );
        ASSERT_FALSE( probeWhileUnlocked.SawLocked() );

        ASSERT_TRUE( MT::LOCKSTATUS_OPERATION_SUCCESS == mutex.Lock( MUTEX_TEST_LOCK_TIMEOUT_IN_MS ) );
        CMutexIsLockedProbeThread probeWhileLocked( mutex );
        ASSERT_TRUE( probeWhileLocked.Activate() );
        ASSERT_TRUE( probeWhileLocked.WaitForCompletion( MUTEX_TEST_LOCK_TIMEOUT_IN_MS ) );
        ASSERT_TRUE( probeWhileLocked.SawLocked() );
        mutex.Unlock();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/**
 *  Regression: on Linux the first lock of a mutex was counted as a re-entry.
 *  The read/write lock derives its writer re-entrancy from that count which caused a
 *  'phantom' active writer to remain after the writer released the lock, blocking all readers forever
 */
static void
PerformWriterReleaseRegressionTest( void )
{
    GUCEF_TESTFW_TESTCASE( "Mutex: ReadWriteLock writer release leaves no active writer behind" )
    try
    {
        MT::CReadWriteLock rwLock( true );

        // single writer
        ASSERT_TRUE( MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == rwLock.WriterStart( MUTEX_TEST_LOCK_TIMEOUT_IN_MS ) );
        ASSERT_TRUE( 1 == rwLock.ActiveWriterCount() );
        ASSERT_TRUE( MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == rwLock.WriterStop() );
        ASSERT_TRUE( 0 == rwLock.ActiveWriterCount() );

        // re-entrant writer
        ASSERT_TRUE( MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == rwLock.WriterStart( MUTEX_TEST_LOCK_TIMEOUT_IN_MS ) );
        ASSERT_TRUE( MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == rwLock.WriterStart( MUTEX_TEST_LOCK_TIMEOUT_IN_MS ) );
        ASSERT_TRUE( MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == rwLock.WriterStop() );
        ASSERT_TRUE( 1 == rwLock.ActiveWriterCount() );
        ASSERT_TRUE( MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == rwLock.WriterStop() );
        ASSERT_TRUE( 0 == rwLock.ActiveWriterCount() );

        // with no writer left a reader must get in right away
        ASSERT_TRUE( MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == rwLock.ReaderStart( 100 ) );
        ASSERT_TRUE( MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == rwLock.ReaderStop() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/**
 *  Regression: on Linux ThreadDelay() slept for the given amount of seconds instead of milliseconds
 */
static void
PerformThreadDelayRegressionTest( void )
{
    GUCEF_TESTFW_TESTCASE( "Mutex: ThreadDelay sleeps for milliseconds" )
    try
    {
        UInt64 startTicks = MT::PrecisionTickCount();
        MT::ThreadDelay( 50 );
        Float64 elapsedInMs = ( MT::PrecisionTickCount() - startTicks ) / ( MT::PrecisionTimerResolution() / 1000.0 );
        ASSERT_TRUE( elapsedInMs >= 45.0 );
        ASSERT_TRUE( elapsedInMs < 1000.0 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/**
 *  Regression: on Linux cleaning up a thread that already finished and was reaped crashed
 *  because the stale thread handle was used afterwards
 */
static void
PerformFinishedThreadCleanupRegressionTest( void )
{
    GUCEF_TESTFW_TESTCASE( "Mutex: Cleaning up finished threads is safe" )
    try
    {
        MT::CMutex mutex;
        for ( UInt32 i=0; i<10; ++i )
        {
            CMutexIsLockedProbeThread* thread = GUCEF_NEW CMutexIsLockedProbeThread( mutex );
            ASSERT_TRUE( thread->Activate() );
            ASSERT_TRUE( thread->WaitForCompletion( MUTEX_TEST_LOCK_TIMEOUT_IN_MS ) );
            GUCEF_DELETE thread;    // must not crash
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/**
 *  Lockable that counts lock calls and reports a configurable lock status
 */
class CCountingLockable : public MT::CILockable
{
    public:

    CCountingLockable( MT::TLockStatus lockStatus )
        : m_lockStatus( lockStatus ), lockCount( 0 ), unlockCount( 0 ), readOnlyLockCount( 0 ), readOnlyUnlockCount( 0 )
    {}

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return this; }
    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs ) const GUCEF_VIRTUAL_OVERRIDE { ++lockCount; return m_lockStatus; }
    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE { ++unlockCount; return MT::LOCKSTATUS_OPERATION_SUCCESS; }
    virtual MT::TLockStatus ReadOnlyLock( UInt32 lockWaitTimeoutInMs ) const GUCEF_VIRTUAL_OVERRIDE { ++readOnlyLockCount; return m_lockStatus; }
    virtual MT::TLockStatus ReadOnlyUnlock( void ) const GUCEF_VIRTUAL_OVERRIDE { ++readOnlyUnlockCount; return MT::LOCKSTATUS_OPERATION_SUCCESS; }

    MT::TLockStatus m_lockStatus;
    mutable UInt32 lockCount;
    mutable UInt32 unlockCount;
    mutable UInt32 readOnlyLockCount;
    mutable UInt32 readOnlyUnlockCount;
};

/*-------------------------------------------------------------------------*/

/**
 *  Regression: scope locks unlocked on scope exit even when the lock attempt reported
 *  LOCKSTATUS_NOT_APPLICABLE, releasing a lock the scope never took
 */
static void
PerformObjectScopeLockNotApplicableRegressionTest( void )
{
    GUCEF_TESTFW_TESTCASE( "Mutex: Object scope locks only unlock what they locked" )
    try
    {
        CCountingLockable notApplicable( MT::LOCKSTATUS_NOT_APPLICABLE );
        {
            MT::CObjectScopeLock lock( notApplicable );
            ASSERT_TRUE( lock.IsLocked() );
        }
        {
            MT::CObjectScopeLock lock( notApplicable );
            ASSERT_TRUE( lock.EarlyUnlock() );
            ASSERT_FALSE( lock.IsLocked() );
        }
        {
            MT::CObjectScopeReadOnlyLock lock( notApplicable );
            ASSERT_TRUE( lock.IsLocked() );
        }
        ASSERT_TRUE( 2 == notApplicable.lockCount );
        ASSERT_TRUE( 0 == notApplicable.unlockCount );
        ASSERT_TRUE( 1 == notApplicable.readOnlyLockCount );
        ASSERT_TRUE( 0 == notApplicable.readOnlyUnlockCount );

        CCountingLockable lockable( MT::LOCKSTATUS_OPERATION_SUCCESS );
        {
            MT::CObjectScopeLock lock( lockable );
            ASSERT_TRUE( lock.IsLocked() );
        }
        {
            MT::CObjectScopeLock lock( lockable );
            ASSERT_TRUE( lock.EarlyUnlock() );
        }
        {
            MT::CObjectScopeReadOnlyLock lock( lockable );
            ASSERT_TRUE( lock.IsLocked() );
        }
        ASSERT_TRUE( 2 == lockable.lockCount );
        ASSERT_TRUE( 2 == lockable.unlockCount );
        ASSERT_TRUE( 1 == lockable.readOnlyLockCount );
        ASSERT_TRUE( 1 == lockable.readOnlyUnlockCount );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/**
 *  Holds a mutex on its own thread until told to let go, at most for a bounded time so a test that
 *  fails because the waiter never returned does not hang. Works on the C++ class or on the C layer mutex.
 */
class CMutexTimedLockHolderThread : public MT::CActiveObject
{
    public:

    CMutexTimedLockHolderThread( MT::CMutex* cppMutex, struct MT::SMutex* cMutex, UInt32 lockDepth, UInt32 maxHoldInMs )
        : MT::CActiveObject()
        , m_cppMutex( cppMutex )
        , m_cMutex( cMutex )
        , m_lockDepth( lockDepth )
        , m_maxHoldInMs( maxHoldInMs )
        , m_holding( false )
        , m_release( false )
        , m_done( false )
    {}

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        return true;
    }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        UInt32 locked = 0;
        for ( UInt32 i=0; i<m_lockDepth; ++i )
        {
            if ( GUCEF_NULL != m_cppMutex )
            {
                if ( MT::LOCKSTATUS_OPERATION_SUCCESS == m_cppMutex->Lock( MUTEX_TEST_LOCK_TIMEOUT_IN_MS ) )
                    ++locked;
            }
            else
            {
                if ( GUCEF_MUTEX_OPERATION_SUCCESS == MT::MutexLock( m_cMutex, MUTEX_TEST_LOCK_TIMEOUT_IN_MS ) )
                    ++locked;
            }
        }
        m_holding.Store( locked == m_lockDepth, MT::MEMORDER_RELEASE );

        UInt32 heldInMs = 0;
        while ( !m_release.Load( MT::MEMORDER_ACQUIRE ) && heldInMs < m_maxHoldInMs )
        {
            MT::PrecisionDelay( 1 );
            ++heldInMs;
        }

        for ( UInt32 i=0; i<locked; ++i )
        {
            if ( GUCEF_NULL != m_cppMutex )
                m_cppMutex->Unlock();
            else
                MT::MutexUnlock( m_cMutex );
        }
        m_holding.Store( false, MT::MEMORDER_RELEASE );
        m_done.Store( true, MT::MEMORDER_RELEASE );
        return true; // stop cycling
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    bool IsHolding( void ) const    { return m_holding.Load( MT::MEMORDER_ACQUIRE ); }
    void Release( void )            { m_release.Store( true, MT::MEMORDER_RELEASE ); }

    bool WaitUntilHolding( UInt32 timeoutInMs ) const
    {GUCEF_TRACE;
        for ( UInt32 i=0; i<timeoutInMs && !IsHolding(); ++i )
            MT::PrecisionDelay( 1 );
        return IsHolding();
    }

    bool WaitUntilDone( UInt32 timeoutInMs ) const
    {GUCEF_TRACE;
        for ( UInt32 i=0; i<timeoutInMs && !m_done.Load( MT::MEMORDER_ACQUIRE ); ++i )
            MT::PrecisionDelay( 1 );
        return m_done.Load( MT::MEMORDER_ACQUIRE );
    }

    private:

    MT::CMutex* m_cppMutex;
    struct MT::SMutex* m_cMutex;
    UInt32 m_lockDepth;
    UInt32 m_maxHoldInMs;
    MT::CTAtomic< bool > m_holding;
    MT::CTAtomic< bool > m_release;
    MT::CTAtomic< bool > m_done;
};

/*-------------------------------------------------------------------------*/

static UInt32
ElapsedInMs( UInt64 startTicks )
{
    return (UInt32) ( ( MT::PrecisionTickCount() - startTicks ) * 1000 / MT::PrecisionTimerResolution() );
}

/*-------------------------------------------------------------------------*/

/**
 *  A lock with a timeout must give up after about that time when another thread holds the mutex, on the C++
 *  class and on the C layer mutex, with one and with several (re-entrant) locks held by the other thread.
 *  Without it a lock that is never released hangs every waiter for good, whatever timeout they pass.
 */
static void
PerformTimedLockTest( bool useCppMutex, UInt32 holderLockDepth )
{
    const UInt32 timeoutInMs = 200;
    MT::CMutex cppMutex;
    struct MT::SMutex* cMutex = useCppMutex ? GUCEF_NULL : MT::MutexCreate();
    ASSERT_TRUE( useCppMutex || GUCEF_NULL != cMutex );

    // The holder lets go by itself after 3 seconds, so a Lock() that ignores its timeout comes back late instead of never
    CMutexTimedLockHolderThread holder( useCppMutex ? &cppMutex : GUCEF_NULL, cMutex, holderLockDepth, 3000 );
    ASSERT_TRUE( holder.Activate() );
    ASSERT_TRUE( holder.WaitUntilHolding( 2000 ) );

    // zero timeout: a try, which must fail at once
    UInt64 start = MT::PrecisionTickCount();
    UInt32 zeroStatus = useCppMutex ? (UInt32) cppMutex.Lock( 0 ) : MT::MutexLock( cMutex, 0 );
    UInt32 zeroElapsed = ElapsedInMs( start );
    if ( zeroStatus == (UInt32) MT::LOCKSTATUS_OPERATION_SUCCESS )
    {
        if ( useCppMutex ) cppMutex.Unlock(); else MT::MutexUnlock( cMutex );
    }
    ASSERT_TRUE( zeroStatus == (UInt32) MT::LOCKSTATUS_WAIT_TIMEOUT );
    ASSERT_TRUE( zeroElapsed < 150 );

    // finite timeout: must report the timeout after about the time asked for
    start = MT::PrecisionTickCount();
    UInt32 status = useCppMutex ? (UInt32) cppMutex.Lock( timeoutInMs ) : MT::MutexLock( cMutex, timeoutInMs );
    UInt32 elapsed = ElapsedInMs( start );
    if ( status == (UInt32) MT::LOCKSTATUS_OPERATION_SUCCESS )
    {
        if ( useCppMutex ) cppMutex.Unlock(); else MT::MutexUnlock( cMutex );
    }
    ASSERT_TRUE( status == (UInt32) MT::LOCKSTATUS_WAIT_TIMEOUT );
    ASSERT_TRUE( elapsed >= timeoutInMs - 10 );
    ASSERT_TRUE( elapsed < timeoutInMs + 400 );

    // the same wait succeeds when the holder lets go in time
    holder.Release();
    ASSERT_TRUE( holder.WaitUntilDone( 2000 ) );
    status = useCppMutex ? (UInt32) cppMutex.Lock( timeoutInMs ) : MT::MutexLock( cMutex, timeoutInMs );
    ASSERT_TRUE( status == (UInt32) MT::LOCKSTATUS_OPERATION_SUCCESS );
    if ( useCppMutex ) cppMutex.Unlock(); else MT::MutexUnlock( cMutex );

    holder.WaitForThreadToFinish( 5000 );
    if ( GUCEF_NULL != cMutex )
        MT::MutexDestroy( cMutex );
}

/*-------------------------------------------------------------------------*/

/** the infinite timeout keeps waiting and succeeds as soon as the holder lets go */
static void
PerformInfiniteLockTest( bool useCppMutex )
{
    MT::CMutex cppMutex;
    struct MT::SMutex* cMutex = useCppMutex ? GUCEF_NULL : MT::MutexCreate();
    ASSERT_TRUE( useCppMutex || GUCEF_NULL != cMutex );

    CMutexTimedLockHolderThread holder( useCppMutex ? &cppMutex : GUCEF_NULL, cMutex, 1, 300 );
    ASSERT_TRUE( holder.Activate() );
    ASSERT_TRUE( holder.WaitUntilHolding( 2000 ) );

    UInt64 start = MT::PrecisionTickCount();
    UInt32 status = useCppMutex ? (UInt32) cppMutex.Lock( GUCEF_MT_INFINITE_LOCK_TIMEOUT ) : MT::MutexLock( cMutex, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    UInt32 elapsed = ElapsedInMs( start );
    if ( status == (UInt32) MT::LOCKSTATUS_OPERATION_SUCCESS )
    {
        if ( useCppMutex ) cppMutex.Unlock(); else MT::MutexUnlock( cMutex );
    }
    ASSERT_TRUE( status == (UInt32) MT::LOCKSTATUS_OPERATION_SUCCESS );
    ASSERT_TRUE( elapsed >= 100 );

    holder.WaitForThreadToFinish( 5000 );
    if ( GUCEF_NULL != cMutex )
        MT::MutexDestroy( cMutex );
}

/*-------------------------------------------------------------------------*/

static void
PerformMutexLockTimeoutTests( void )
{
    GUCEF_TESTFW_TESTCASE( "Mutex: Lock( timeout ) gives up after the timeout while another thread holds the mutex" )
    try
    {
        PerformTimedLockTest( true, 1 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mutex: Lock( timeout ) gives up after the timeout while another thread holds the mutex recursively" )
    try
    {
        PerformTimedLockTest( true, 3 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mutex: C layer MT::MutexLock( timeout ) gives up after the timeout while another thread holds the mutex" )
    try
    {
        PerformTimedLockTest( false, 1 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mutex: C layer MT::MutexLock( timeout ) gives up after the timeout while another thread holds the mutex recursively" )
    try
    {
        PerformTimedLockTest( false, 3 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mutex: Lock with the infinite timeout waits until the holder lets go" )
    try
    {
        PerformInfiniteLockTest( true );
        PerformInfiniteLockTest( false );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformMutexTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING MUTEX TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "Mutex" );

    PerformBasicMutexTests();
    PerformFailedUnlockIsReportedTest();
    PerformScopeMutexTests();
    PerformRecursiveMutexTests();
    PerformMutexTimeoutTest();
    PerformMultiThreadedMutexTests();
    PerformRecursiveUnlockUnblocksOtherThreadTest();
    PerformIsLockedRegressionTest();
    PerformWriterReleaseRegressionTest();
    PerformThreadDelayRegressionTest();
    PerformFinishedThreadCleanupRegressionTest();
    PerformObjectScopeLockNotApplicableRegressionTest();
    PerformMutexLockTimeoutTests();

    GUCEF::CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL MUTEX TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
