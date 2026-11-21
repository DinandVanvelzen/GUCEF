/*
 *  gucefMT: GUCEF module providing multithreading solutions
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#include <assert.h>
#include "gucefMT_CMutex.h"     /* Our mutex class definition */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

#include <windows.h>
#include <processthreadsapi.h>

#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>

#ifndef PTHREAD_MUTEX_RECURSIVE
  #define PTHREAD_MUTEX_RECURSIVE PTHREAD_MUTEX_RECURSIVE_NP 
#endif

#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

struct SMutexData
{
    bool isLocked;
    GUCEF_SYSTEM_THREAD_ID_MEMBER( systemThreadLastOwningLock )    /* display only, recorded with threadLastOwningLock */
    TThreadId threadLastOwningLock;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    
    #if ( GUCEF_MT_MUTEX_WINDOWS_LOCK_IMPLEMENTATION == GUCEF_MT_MUTEX_LOCK_IMPLEMENTATION_WIN32_MUTEX )
    
    ::HANDLE id;

    SMutexData( void )
        : isLocked( false )
        GUCEF_SYSTEM_THREAD_ID_MEMBER_INIT( systemThreadLastOwningLock, 0 )
        , threadLastOwningLock( 0 )
        , id( 0 )
    {GUCEF_TRACE;
    }
    
    #elif ( GUCEF_MT_MUTEX_WINDOWS_LOCK_IMPLEMENTATION == GUCEF_MT_MUTEX_LOCK_IMPLEMENTATION_WIN32_CRITICAL_SECTION )
    
    ::CRITICAL_SECTION critsection;
    
    SMutexData( void )
        : isLocked( false )
        GUCEF_SYSTEM_THREAD_ID_MEMBER_INIT( systemThreadLastOwningLock, 0 )
        , threadLastOwningLock( 0 )
        , critsection()
    {GUCEF_TRACE;

    }

    #endif
    
    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

    pthread_mutex_t id;

    SMutexData( void )
        : isLocked( false )
        GUCEF_SYSTEM_THREAD_ID_MEMBER_INIT( systemThreadLastOwningLock, 0 )
        , threadLastOwningLock( 0 )
        , id()
    {GUCEF_TRACE;
    }

    #else
    #error Unsuported target platform
    #endif
};

typedef struct SMutexData TMutexData;

/*--------------------------------------------------------------------------*/

/** Records the calling thread as the one holding the lock: its identity and, where it differs, its system id */
inline void
RecordLockOwner( TMutexData* mutexData )
{
    SThreadIds owner;
    GetCurrentThreadIds( &owner );
    mutexData->threadLastOwningLock = owner.threadId;
    GUCEF_SYSTEM_THREAD_ID_CODE( mutexData->systemThreadLastOwningLock = owner.systemThreadId; )
}

/*--------------------------------------------------------------------------*/

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

#if ( defined( __ANDROID_API__ ) && ( __ANDROID_API__ < 21 ) )
  #define GUCEF_MT_NO_PTHREAD_MUTEX_TIMEDLOCK
#endif

/**
 *  Locks a pthread mutex, waiting at most timeoutInMs milliseconds. GUCEF_MT_INFINITE_LOCK_TIMEOUT waits for as
 *  long as it takes, a timeout of 0 is a try.
 *
 *  @return 0 on success, ETIMEDOUT when the mutex stayed locked for the whole time, another errno value on failure
 */
static int
PosixMutexLockWithTimeout( pthread_mutex_t* mutex, UInt32 timeoutInMs )
{
    struct timespec monotonicStart;
    UInt32 remainingInMs = timeoutInMs;
    int result = 0;

    if ( GUCEF_MT_INFINITE_LOCK_TIMEOUT == timeoutInMs )
        return pthread_mutex_lock( mutex );

    result = pthread_mutex_trylock( mutex );
    if ( EBUSY != result )
        return result;
    if ( 0 == timeoutInMs )
        return ETIMEDOUT;

    clock_gettime( CLOCK_MONOTONIC, &monotonicStart );
    for ( ;; )
    {
        struct timespec monotonicNow;
        Int64 elapsedInMs = 0;
        #ifndef GUCEF_MT_NO_PTHREAD_MUTEX_TIMEDLOCK
        struct timespec deadline;
        #endif

        #ifdef GUCEF_MT_NO_PTHREAD_MUTEX_TIMEDLOCK

        PrecisionDelay( 1 );
        result = pthread_mutex_trylock( mutex );
        if ( EBUSY != result )
            return result;

        #else

        /* the deadline is on the real time clock, which is what pthread_mutex_timedlock() wants */
        clock_gettime( CLOCK_REALTIME, &deadline );
        deadline.tv_sec += remainingInMs / 1000;
        deadline.tv_nsec += 1000000L * (long) ( remainingInMs % 1000 );
        if ( deadline.tv_nsec >= 1000000000L )
        {
            deadline.tv_sec += 1;
            deadline.tv_nsec -= 1000000000L;
        }
        result = pthread_mutex_timedlock( mutex, &deadline );
        if ( ETIMEDOUT != result )
            return result;

        #endif

        /* A step of the real time clock must not shorten or lengthen the wait: the monotonic clock decides */
        clock_gettime( CLOCK_MONOTONIC, &monotonicNow );
        elapsedInMs = ( (Int64) ( monotonicNow.tv_sec - monotonicStart.tv_sec ) * 1000 ) +
                      ( ( (Int64) monotonicNow.tv_nsec - (Int64) monotonicStart.tv_nsec ) / 1000000 );
        if ( elapsedInMs >= (Int64) timeoutInMs )
            return ETIMEDOUT;
        remainingInMs = (UInt32) ( (Int64) timeoutInMs - elapsedInMs );
    }
}

#endif

/*--------------------------------------------------------------------------*/

CMutex::CMutex( void )
    : _mutexdata( GUCEF_NEW TMutexData )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    #if ( GUCEF_MT_MUTEX_WINDOWS_LOCK_IMPLEMENTATION == GUCEF_MT_MUTEX_LOCK_IMPLEMENTATION_WIN32_MUTEX )

    _mutexdata->id = CreateMutex( GUCEF_NULL, FALSE, GUCEF_NULL );
    if ( 0 == _mutexdata->id )
    {
        GUCEF_DELETE _mutexdata;
        _mutexdata = GUCEF_NULL;
        GUCEF_ASSERT_ALWAYS;
        return;
    }
    GUCEF_TRACE_EXCLUSIVE_LOCK_CREATED( _mutexdata->id );

    #elif ( GUCEF_MT_MUTEX_WINDOWS_LOCK_IMPLEMENTATION == GUCEF_MT_MUTEX_LOCK_IMPLEMENTATION_WIN32_CRITICAL_SECTION )

    ::InitializeCriticalSection( (::CRITICAL_SECTION*) &_mutexdata->critsection );
    GUCEF_TRACE_EXCLUSIVE_LOCK_CREATED( (::CRITICAL_SECTION*) &_mutexdata->critsection );

    #endif

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

    pthread_mutexattr_t attr;

    pthread_mutexattr_init( &attr );
    pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE );

    int initResult = pthread_mutex_init( &_mutexdata->id, &attr );
    pthread_mutexattr_destroy( &attr );
    if ( initResult != 0 )
    {
        GUCEF_DELETE _mutexdata;
        _mutexdata = GUCEF_NULL;
        GUCEF_ASSERT_ALWAYS;
        return;
    }
    GUCEF_TRACE_EXCLUSIVE_LOCK_CREATED( &_mutexdata->id );

    #endif
}

/*--------------------------------------------------------------------------*/

#ifdef GUCEF_RVALUE_REFERENCES_SUPPORTED

CMutex::CMutex( CMutex&& src ) GUCEF_NOEXCEPT
    : _mutexdata( src._mutexdata )
{GUCEF_TRACE;

    src._mutexdata = GUCEF_NULL;
}

#endif

/*--------------------------------------------------------------------------*/

CMutex::~CMutex()
{GUCEF_TRACE;

    if ( GUCEF_NULL == _mutexdata )
        return;

    // Waits for a thread that may still hold the lock, then lets go again: a mutex must not be destroyed while it is
    // locked. pthreads leaves that undefined and musl (Emscripten) keeps a locked recursive mutex linked in the owning
    // thread's list of held mutexes, where the next lock or unlock by that thread writes into the freed memory
    Lock( GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    Unlock();

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    
    #if ( GUCEF_MT_MUTEX_WINDOWS_LOCK_IMPLEMENTATION == GUCEF_MT_MUTEX_LOCK_IMPLEMENTATION_WIN32_MUTEX )

    GUCEF_TRACE_EXCLUSIVE_LOCK_DESTROY( ((TMutexData*)_mutexdata)->id );
    CloseHandle( ((TMutexData*)_mutexdata)->id );
    GUCEF_DELETE (TMutexData*)_mutexdata;
    _mutexdata = GUCEF_NULL;

    #elif ( GUCEF_MT_MUTEX_WINDOWS_LOCK_IMPLEMENTATION == GUCEF_MT_MUTEX_LOCK_IMPLEMENTATION_WIN32_CRITICAL_SECTION )

    GUCEF_TRACE_EXCLUSIVE_LOCK_DESTROY( &_mutexdata->critsection );
    ::DeleteCriticalSection( &_mutexdata->critsection );
    GUCEF_DELETE _mutexdata;
    _mutexdata = GUCEF_NULL;

    #endif

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

    GUCEF_TRACE_EXCLUSIVE_LOCK_DESTROY( &((TMutexData*)_mutexdata)->id );
    pthread_mutex_destroy( &((TMutexData*)_mutexdata)->id );
    GUCEF_DELETE (TMutexData*)_mutexdata;
    _mutexdata = GUCEF_NULL;

    #endif
}

/*--------------------------------------------------------------------------*/

TLockStatus
CMutex::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == _mutexdata )
        return LOCKSTATUS_OPERATION_FAILED;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    #if ( GUCEF_MT_MUTEX_WINDOWS_LOCK_IMPLEMENTATION == GUCEF_MT_MUTEX_LOCK_IMPLEMENTATION_WIN32_MUTEX )

    ::DWORD timeoutInMs = lockWaitTimeoutInMs == GUCEF_MT_INFINITE_LOCK_TIMEOUT ? INFINITE : (::DWORD) lockWaitTimeoutInMs;
    ::DWORD waitResult = ::WaitForSingleObject( ((TMutexData*)_mutexdata)->id, timeoutInMs );
    switch ( waitResult )
    {
        case WAIT_OBJECT_0:
        {
            RecordLockOwner( (TMutexData*) _mutexdata );
            ((TMutexData*)_mutexdata)->isLocked = true;
            GUCEF_TRACE_EXCLUSIVE_LOCK_OBTAINED( ((TMutexData*)_mutexdata)->id );
            return LOCKSTATUS_OPERATION_SUCCESS;
        }
        case WAIT_TIMEOUT:
        {
            return LOCKSTATUS_WAIT_TIMEOUT;
        }
        case WAIT_ABANDONED:
        {
            if ( GUCEF_NULL == _mutexdata )
                return LOCKSTATUS_OPERATION_FAILED;

            RecordLockOwner( (TMutexData*) _mutexdata );
            ((TMutexData*)_mutexdata)->isLocked = true;
            GUCEF_TRACE_EXCLUSIVE_LOCK_ABANDONED( ((TMutexData*)_mutexdata)->id );
            return LOCKSTATUS_ABANDONED;
        }
        default:
        {
            // we should never get here since we handled all the cases
            GUCEF_ASSERT_ALWAYS;
            return LOCKSTATUS_OPERATION_FAILED;
        }
    }

    #elif ( GUCEF_MT_MUTEX_WINDOWS_LOCK_IMPLEMENTATION == GUCEF_MT_MUTEX_LOCK_IMPLEMENTATION_WIN32_CRITICAL_SECTION )

    ::ULONGLONG tickCountAtWaitStart = ::GetTickCount64();
    UInt32 timeWaitedInMs = 0;
    do
    {
        ::BOOL lockSuccess = ::TryEnterCriticalSection( (::CRITICAL_SECTION*) &_mutexdata->critsection );
        if ( FALSE == lockSuccess )
        {
            timeWaitedInMs = (UInt32) ( ::GetTickCount64() - tickCountAtWaitStart );
            if ( lockWaitTimeoutInMs == GUCEF_MT_INFINITE_LOCK_TIMEOUT || timeWaitedInMs < lockWaitTimeoutInMs )
                PrecisionDelay( 1 );
        }
        else
        {
            RecordLockOwner( (TMutexData*) _mutexdata );
            _mutexdata->isLocked = true;
            GUCEF_TRACE_EXCLUSIVE_LOCK_OBTAINED( (::CRITICAL_SECTION*) &_mutexdata->critsection );
            return LOCKSTATUS_OPERATION_SUCCESS;
        }
    }
    while ( lockWaitTimeoutInMs == GUCEF_MT_INFINITE_LOCK_TIMEOUT || timeWaitedInMs < lockWaitTimeoutInMs );
    
    return LOCKSTATUS_WAIT_TIMEOUT;
    
    #endif

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

    /*
     *  pthreads reports failure as a positive errno value, never as a negative one. The timeout is honoured as on
     *  Windows: a mutex that stays locked for longer is reported as a timeout instead of waited for without end
     */
    const int lockResult = PosixMutexLockWithTimeout( &((TMutexData*)_mutexdata)->id, lockWaitTimeoutInMs );
    if ( ETIMEDOUT == lockResult )
    {
        return LOCKSTATUS_WAIT_TIMEOUT;
    }
    if ( 0 != lockResult )
    {
        return LOCKSTATUS_OPERATION_FAILED;
    }

    RecordLockOwner( (TMutexData*) _mutexdata );
    ((TMutexData*)_mutexdata)->isLocked = true;
    GUCEF_TRACE_EXCLUSIVE_LOCK_OBTAINED( &((TMutexData*)_mutexdata)->id );
    return LOCKSTATUS_OPERATION_SUCCESS;

    #endif
}

/*--------------------------------------------------------------------------*/

TLockStatus
CMutex::Unlock( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == _mutexdata )
        return LOCKSTATUS_OPERATION_FAILED;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    #if ( GUCEF_MT_MUTEX_WINDOWS_LOCK_IMPLEMENTATION == GUCEF_MT_MUTEX_LOCK_IMPLEMENTATION_WIN32_MUTEX )

    _mutexdata->isLocked = false;
    GUCEF_TRACE_EXCLUSIVE_LOCK_RELEASED( _mutexdata->id );

    if ( ReleaseMutex( _mutexdata->id ) == FALSE )
    {
        ((TMutexData*)_mutexdata)->isLocked = true;
        GUCEF_TRACE_EXCLUSIVE_LOCK_OBTAINED( _mutexdata->id );
        return LOCKSTATUS_OPERATION_FAILED;
    }
    return LOCKSTATUS_OPERATION_SUCCESS;

    #elif ( GUCEF_MT_MUTEX_WINDOWS_LOCK_IMPLEMENTATION == GUCEF_MT_MUTEX_LOCK_IMPLEMENTATION_WIN32_CRITICAL_SECTION )

    /*
     *  Do NOT gate this on _mutexdata->isLocked.
     *  CRITICAL_SECTION is recursive: the same thread can Enter multiple times, and each Enter
     *  must be matched by a Leave. A plain bool isLocked cannot track the recursion count, so
     *  gating on it causes the second Unlock() call (in a recursive lock scenario) to skip
     *  LeaveCriticalSection, leaving the CS permanently locked and blocking other threads.
     *  Instead, only check thread ownership — the OS enforces the Enter/Leave pairing.
     *
     *  From MS Docs:
     *      If a thread calls LeaveCriticalSection when it does not have ownership of the
     *      specified critical section object, an error occurs that may cause another thread
     *      using EnterCriticalSection to wait indefinitely.
     */
    if ( _mutexdata->threadLastOwningLock == GetCurrentThreadID() )
    {
        _mutexdata->isLocked = false;
        GUCEF_TRACE_EXCLUSIVE_LOCK_RELEASED( &_mutexdata->critsection );
        ::LeaveCriticalSection( &_mutexdata->critsection );
        return LOCKSTATUS_OPERATION_SUCCESS;
    }
    return LOCKSTATUS_OPERATION_FAILED;

    #endif
    
    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

    bool wasLocked = ((TMutexData*)_mutexdata)->isLocked;
    ((TMutexData*)_mutexdata)->isLocked = false;
    GUCEF_TRACE_EXCLUSIVE_LOCK_RELEASED( &((TMutexData*)_mutexdata)->id );

    /* pthreads reports failure as a positive errno value (EPERM when we do not own the lock), never as a negative one */
    if ( 0 != pthread_mutex_unlock( &((TMutexData*)_mutexdata)->id ) )
    {
        /* the lock is still held by its owner, undo our administration update */
        ((TMutexData*)_mutexdata)->isLocked = wasLocked;
        if ( wasLocked )
        {
            GUCEF_TRACE_EXCLUSIVE_LOCK_OBTAINED( &((TMutexData*)_mutexdata)->id );
        }
        return LOCKSTATUS_OPERATION_FAILED;
    }
    return LOCKSTATUS_OPERATION_SUCCESS;

    #endif
}

/*--------------------------------------------------------------------------*/

const CILockable*
CMutex::AsLockable( void ) const
{GUCEF_TRACE;

    return this;
}

/*--------------------------------------------------------------------------*/

TThreadId
CMutex::GetThreadIdOfThreadLastHoldingTheLock( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == _mutexdata )
        return 0;

    return ((TMutexData*)_mutexdata)->threadLastOwningLock;
}

/*--------------------------------------------------------------------------*/

TSystemThreadId
CMutex::GetSystemThreadIdOfThreadLastHoldingTheLock( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == _mutexdata )
        return 0;

    const TMutexData* mutexData = (const TMutexData*) _mutexdata;
    return GUCEF_SYSTEM_THREAD_ID_OR_THREAD_ID( mutexData->systemThreadLastOwningLock, mutexData->threadLastOwningLock );
}

/*--------------------------------------------------------------------------*/

bool
CMutex::IsLocked( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == _mutexdata )
        return false;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    #if ( GUCEF_MT_MUTEX_WINDOWS_LOCK_IMPLEMENTATION == GUCEF_MT_MUTEX_LOCK_IMPLEMENTATION_WIN32_MUTEX )

    /* lock with a timeout of 0. Note this is not a cheap operation */
    const DWORD waitResult = WaitForSingleObject( ((TMutexData*)_mutexdata)->id , 0 );
    if ( WAIT_OBJECT_0 == waitResult || WAIT_ABANDONED == waitResult )
    {
        // we got the lock which means nobody else had it: give it back
        ReleaseMutex( ((TMutexData*)_mutexdata)->id );
        return false;
    }
    return true;

    #elif ( GUCEF_MT_MUTEX_WINDOWS_LOCK_IMPLEMENTATION == GUCEF_MT_MUTEX_LOCK_IMPLEMENTATION_WIN32_CRITICAL_SECTION )
    
    /* attempt a lock to see if its locked by anyone */
    ::BOOL lockSuccess = ::TryEnterCriticalSection( &_mutexdata->critsection );
    if ( FALSE != lockSuccess )
    {
        ::LeaveCriticalSection( &_mutexdata->critsection );
        return false;
    }
    return true;

    #endif
    
    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

    /* attempt a lock to see if its locked by anyone, same semantics as the Windows implementation */
    if ( pthread_mutex_trylock( &((TMutexData*)_mutexdata)->id ) == 0 )
    {
        pthread_mutex_unlock( &((TMutexData*)_mutexdata)->id );
        return false;
    }
    return true;

    #else

    #error unsupported platform

    #endif
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
