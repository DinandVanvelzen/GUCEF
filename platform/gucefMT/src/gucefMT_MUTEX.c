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

#ifdef __APPLE__
#include <stdlib.h>
#else
#include <malloc.h>
#endif

#ifndef GUCEF_MT_MUTEX_H
#include "gucefMT_mutex.h"     /* mutex utility prototypes */
#define GUCEF_MT_MUTEX_H
#endif /* GUCEF_MT_MUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

#include <windows.h>
#include <processthreadsapi.h>

#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_MACOS ) )

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

struct SMutex
{
    TThreadId lockedByThread;    /* GetCurrentThreadID() of the holder, 0 when no thread holds the mutex */
    GUCEF_SYSTEM_THREAD_ID_MEMBER( lockedBySystemThread )  /* GetCurrentThreadSystemID() of the holder, display only */
    Int32 reentrancyCount;
    
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    #if ( GUCEF_MT_MUTEX_WINDOWS_LOCK_IMPLEMENTATION == GUCEF_MT_MUTEX_LOCK_IMPLEMENTATION_WIN32_MUTEX )
    
    HANDLE id;

    #elif ( GUCEF_MT_MUTEX_WINDOWS_LOCK_IMPLEMENTATION == GUCEF_MT_MUTEX_LOCK_IMPLEMENTATION_WIN32_CRITICAL_SECTION )

    CRITICAL_SECTION critsection;
    
    #endif

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_MACOS ) )

    pthread_mutex_t id;

    #else
    #error Unsuported target platform
    #endif
};

typedef struct SMutex TMutex;

/*--------------------------------------------------------------------------*/

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_MACOS ) )

#if ( defined( __ANDROID_API__ ) && ( __ANDROID_API__ < 21 ) ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_MACOS )
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

struct SMutex*
MutexCreate( void )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    
    #if ( GUCEF_MT_MUTEX_WINDOWS_LOCK_IMPLEMENTATION == GUCEF_MT_MUTEX_LOCK_IMPLEMENTATION_WIN32_MUTEX )

    TMutex* mutex = malloc( sizeof( TMutex ) );
    if ( GUCEF_NULL != mutex )
    {
        mutex->lockedByThread = 0;
        GUCEF_SYSTEM_THREAD_ID_CODE( mutex->lockedBySystemThread = 0; )
        mutex->reentrancyCount = 0;
        mutex->id = CreateMutex( NULL, FALSE, NULL );
        if ( 0 == mutex->id )
        {
            free( mutex );
            return GUCEF_NULL;
        }
        GUCEF_TRACE_EXCLUSIVE_LOCK_CREATED( mutex->id );
    }
    return mutex;

    #elif ( GUCEF_MT_MUTEX_WINDOWS_LOCK_IMPLEMENTATION == GUCEF_MT_MUTEX_LOCK_IMPLEMENTATION_WIN32_CRITICAL_SECTION )

    TMutex* mutex = malloc( sizeof( TMutex ) );
    if ( GUCEF_NULL != mutex )
    {
        mutex->lockedByThread = 0;
        GUCEF_SYSTEM_THREAD_ID_CODE( mutex->lockedBySystemThread = 0; )
        mutex->reentrancyCount = 0;
        InitializeCriticalSection( &mutex->critsection );
        GUCEF_TRACE_EXCLUSIVE_LOCK_CREATED( &mutex->critsection );
    }
    return mutex;
    
    #endif
    
    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_MACOS ) )
    
    pthread_mutexattr_t attr;
    pthread_mutexattr_init( &attr );
    pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE );

    TMutex* mutex = malloc( sizeof( TMutex ) );
    mutex->reentrancyCount = 0;
    mutex->lockedByThread = 0;
    GUCEF_SYSTEM_THREAD_ID_CODE( mutex->lockedBySystemThread = 0; )

    int initResult = pthread_mutex_init( &mutex->id, &attr );
    pthread_mutexattr_destroy( &attr );
    if ( initResult != 0 )
    {
        free( mutex );
        return GUCEF_NULL;
    }
    
    GUCEF_TRACE_EXCLUSIVE_LOCK_CREATED( &mutex->id );
    return mutex;
    
    #endif
}

/*--------------------------------------------------------------------------*/

void
MutexDestroy( struct SMutex* mutex )
{
    /*
     *  Waits for a thread that may still hold the lock, then lets go again: a mutex must not be destroyed while
     *  it is locked. pthreads leaves that undefined and musl (Emscripten) keeps a locked recursive mutex linked in
     *  the owning thread's list of held mutexes, where the next lock or unlock by that thread writes into freed memory
     */
    MutexLock( mutex, GUCEF_UINT32MAX );
    MutexUnlock( mutex );
    
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    #if ( GUCEF_MT_MUTEX_WINDOWS_LOCK_IMPLEMENTATION == GUCEF_MT_MUTEX_LOCK_IMPLEMENTATION_WIN32_MUTEX )
    
    GUCEF_TRACE_EXCLUSIVE_LOCK_DESTROY( mutex->id );
    CloseHandle( mutex->id );
    free( mutex );

    #elif ( GUCEF_MT_MUTEX_WINDOWS_LOCK_IMPLEMENTATION == GUCEF_MT_MUTEX_LOCK_IMPLEMENTATION_WIN32_CRITICAL_SECTION )

    GUCEF_TRACE_EXCLUSIVE_LOCK_DESTROY( mutex->id );
    DeleteCriticalSection( &mutex->critsection );
    free( mutex );

    #endif
    
    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_MACOS ) )
    
    GUCEF_TRACE_EXCLUSIVE_LOCK_DESTROY( &mutex->id );
    pthread_mutex_destroy( &mutex->id );
    free( mutex );
    
    #endif
}

/*--------------------------------------------------------------------------*/

UInt32
MutexLock( struct SMutex* mutex, UInt32 timeoutInMs )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    
    #if ( GUCEF_MT_MUTEX_WINDOWS_LOCK_IMPLEMENTATION == GUCEF_MT_MUTEX_LOCK_IMPLEMENTATION_WIN32_MUTEX )
    
    GUCEF_BEGIN;
    DWORD waitResult = WaitForSingleObject( mutex->id, (DWORD) timeoutInMs );
    switch ( waitResult )
    {
        case WAIT_OBJECT_0:
        {
            struct SThreadIds caller;
            GetCurrentThreadIds( &caller );
            if ( caller.threadId == mutex->lockedByThread )             
                ++mutex->reentrancyCount;
            else
            {
                mutex->lockedByThread = caller.threadId;
                GUCEF_SYSTEM_THREAD_ID_CODE( mutex->lockedBySystemThread = caller.systemThreadId; )
            }

            GUCEF_TRACE_EXCLUSIVE_LOCK_OBTAINED( mutex->id );
            GUCEF_END;
            return GUCEF_MUTEX_OPERATION_SUCCESS;
        }
        case WAIT_TIMEOUT:
        {
            GUCEF_END;
            return GUCEF_MUTEX_WAIT_TIMEOUT;
        }
        case WAIT_ABANDONED:
        {
            struct SThreadIds caller;
            GetCurrentThreadIds( &caller );
            mutex->lockedByThread = caller.threadId;
            GUCEF_SYSTEM_THREAD_ID_CODE( mutex->lockedBySystemThread = caller.systemThreadId; )
            mutex->reentrancyCount = 0;

            GUCEF_TRACE_EXCLUSIVE_LOCK_ABANDONED( mutex->id );
            GUCEF_END;
            return GUCEF_MUTEX_ABANDONED;
        }
        default:
        {
            GUCEF_END;
            return GUCEF_MUTEX_OPERATION_FAILED;
        }
    }
    
    #elif ( GUCEF_MT_MUTEX_WINDOWS_LOCK_IMPLEMENTATION == GUCEF_MT_MUTEX_LOCK_IMPLEMENTATION_WIN32_CRITICAL_SECTION )

    ULONGLONG tickCountAtWaitStart = GetTickCount64();
    UInt32 timeWaitedInMs = 0;
    do
    {
        BOOL lockSuccess = TryEnterCriticalSection( &mutex->critsection );
        if ( FALSE == lockSuccess )
        {
            timeWaitedInMs = (UInt32) ( GetTickCount64() - tickCountAtWaitStart );
            Int32 timeRemaining = timeoutInMs - timeWaitedInMs;
            if ( timeRemaining > 0 )
                PrecisionDelay( 1 );    
        }
        else
        {
            struct SThreadIds caller;
            GetCurrentThreadIds( &caller );
            if ( caller.threadId == mutex->lockedByThread )             
                ++mutex->reentrancyCount;
            else
            {
                mutex->lockedByThread = caller.threadId;
                GUCEF_SYSTEM_THREAD_ID_CODE( mutex->lockedBySystemThread = caller.systemThreadId; )
            }

            GUCEF_TRACE_EXCLUSIVE_LOCK_OBTAINED( &mutex->critsection );
            return LOCKSTATUS_OPERATION_SUCCESS;
        }
    }
    while ( timeWaitedInMs < timeoutInMs );
    
    return LOCKSTATUS_WAIT_TIMEOUT;

    #endif
    
    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_MACOS ) )
    
    struct SThreadIds caller;
    GetCurrentThreadIds( &caller );
    /* honours the timeout: a mutex that stays locked for longer is reported as a timeout, as on Windows */
    int lockResult = PosixMutexLockWithTimeout( &mutex->id, timeoutInMs );

    if ( ETIMEDOUT == lockResult )
        return GUCEF_MUTEX_WAIT_TIMEOUT;
    if ( 0 != lockResult ) 
        return GUCEF_MUTEX_OPERATION_FAILED;
    
    /* same semantics as the Windows implementations: the first lock is not a re-entry */
    if ( caller.threadId == mutex->lockedByThread )             
        ++mutex->reentrancyCount;
    else
    {
        mutex->lockedByThread = caller.threadId;
        GUCEF_SYSTEM_THREAD_ID_CODE( mutex->lockedBySystemThread = caller.systemThreadId; )
    }
    GUCEF_TRACE_EXCLUSIVE_LOCK_OBTAINED( &mutex->id );
    return GUCEF_MUTEX_OPERATION_SUCCESS;
    
    #endif
}

/*--------------------------------------------------------------------------*/

UInt32
MutexUnlock( struct SMutex* mutex )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    #if ( GUCEF_MT_MUTEX_WINDOWS_LOCK_IMPLEMENTATION == GUCEF_MT_MUTEX_LOCK_IMPLEMENTATION_WIN32_MUTEX )
    
    GUCEF_BEGIN;

    TThreadId callerThreadId = GetCurrentThreadID();
    if ( mutex->lockedByThread == callerThreadId )
    {
        if ( mutex->reentrancyCount > 0 )
            --mutex->reentrancyCount;
        else
        {
            mutex->lockedByThread = 0;
            GUCEF_SYSTEM_THREAD_ID_CODE( mutex->lockedBySystemThread = 0; )
        }
    }
    
    GUCEF_TRACE_EXCLUSIVE_LOCK_RELEASED( mutex->id );
    if ( 0 == ReleaseMutex( mutex->id ) )
    {
        /*
         *  Since we failed to release the lock we have to undo our 
         *  administration updates
         */
        if ( mutex->lockedByThread == callerThreadId )
        {
            if ( 0 == mutex->lockedByThread )
                mutex->lockedByThread = callerThreadId;
            else
                ++mutex->reentrancyCount;    

            GUCEF_TRACE_EXCLUSIVE_LOCK_OBTAINED( mutex->id );
        }
        
        GUCEF_END;
        return GUCEF_MUTEX_OPERATION_FAILED;
    }

    GUCEF_END;
    return GUCEF_MUTEX_OPERATION_SUCCESS;

    #elif ( GUCEF_MT_MUTEX_WINDOWS_LOCK_IMPLEMENTATION == GUCEF_MT_MUTEX_LOCK_IMPLEMENTATION_WIN32_CRITICAL_SECTION )

    GUCEF_BEGIN;

    TThreadId callerThreadId = GetCurrentThreadID();
    if ( mutex->lockedByThread == callerThreadId )
    {
        if ( mutex->reentrancyCount > 0 )
            --mutex->reentrancyCount;
        else
        {
            mutex->lockedByThread = 0;
            GUCEF_SYSTEM_THREAD_ID_CODE( mutex->lockedBySystemThread = 0; )
        }
    
        GUCEF_TRACE_EXCLUSIVE_LOCK_RELEASED( &mutex->critsection );
        LeaveCriticalSection( &mutex->critsection );

        GUCEF_END;
        return LOCKSTATUS_OPERATION_SUCCESS;
    }
    else
    {
        /*
         *  From MS Docs:
         *      If a thread calls LeaveCriticalSection when it does not have ownership of the 
         *      specified critical section object, an error occurs that may cause another thread 
         *      using EnterCriticalSection to wait indefinitely.
         * 
         *  If your code is structured correctly we should never come here
         */
        GUCEF_END;
        return LOCKSTATUS_OPERATION_FAILED;
    }

    #endif
    
    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_MACOS ) )
    
    TThreadId callerThreadId = GetCurrentThreadID();
    if ( mutex->lockedByThread == callerThreadId )
    {
        if ( mutex->reentrancyCount > 0 )
            --mutex->reentrancyCount;
        else
        {
            mutex->lockedByThread = 0;
            GUCEF_SYSTEM_THREAD_ID_CODE( mutex->lockedBySystemThread = 0; )
        }
    }

    GUCEF_TRACE_EXCLUSIVE_LOCK_RELEASED( &mutex->id );
    if ( pthread_mutex_unlock( &mutex->id ) != 0 ) 
    {
        /*
         *  Since we failed to release the lock we have to undo our 
         *  administration updates
         */
        if ( mutex->lockedByThread == callerThreadId )
        {
            if ( 0 == mutex->lockedByThread )
                mutex->lockedByThread = callerThreadId;
            else
                ++mutex->reentrancyCount;    

            GUCEF_TRACE_EXCLUSIVE_LOCK_OBTAINED( &mutex->id );
        }
        return GUCEF_MUTEX_OPERATION_FAILED;
    }
    return GUCEF_MUTEX_OPERATION_SUCCESS;
    
    #endif
}

/*--------------------------------------------------------------------------*/

UInt32
MutexLocked( struct SMutex* mutex )
{
    if ( GUCEF_NULL != mutex )
        return 0 != mutex->lockedByThread ? 1 : 0;
    return 0;
}

/*--------------------------------------------------------------------------*/

Int32
MutexReentrancy( struct SMutex* mutex )
{
    if ( GUCEF_NULL != mutex )
        return mutex->reentrancyCount;
    return 0;
}

/*--------------------------------------------------------------------------*/

TThreadId
MutexLockedByThread( struct SMutex* mutex )
{
    if ( GUCEF_NULL != mutex )
        return mutex->lockedByThread;
    return 0;
}

/*--------------------------------------------------------------------------*/

TSystemThreadId
MutexLockedBySystemThread( struct SMutex* mutex )
{
    if ( GUCEF_NULL != mutex )
        return GUCEF_SYSTEM_THREAD_ID_OR_THREAD_ID( mutex->lockedBySystemThread, mutex->lockedByThread );
    return 0;
}

/*--------------------------------------------------------------------------*/

UInt32
MutexStructSize( void )
{
    return (UInt32) sizeof( struct SMutex );
}

/*--------------------------------------------------------------------------*/
