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

#include <malloc.h>

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

#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

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
    UInt32 lockedByThread;
    Int32 reentrancyCount;
    
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    #if ( GUCEF_MT_MUTEX_WINDOWS_LOCK_IMPLEMENTATION == GUCEF_MT_MUTEX_LOCK_IMPLEMENTATION_WIN32_MUTEX )
    
    HANDLE id;

    #elif ( GUCEF_MT_MUTEX_WINDOWS_LOCK_IMPLEMENTATION == GUCEF_MT_MUTEX_LOCK_IMPLEMENTATION_WIN32_CRITICAL_SECTION )

    CRITICAL_SECTION critsection;
    
    #endif

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

    pthread_mutex_t id;

    #else
    #error Unsuported target platform
    #endif
};

typedef struct SMutex TMutex;

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
        mutex->reentrancyCount = 0;
        InitializeCriticalSection( &mutex->critsection );
        GUCEF_TRACE_EXCLUSIVE_LOCK_CREATED( &mutex->critsection );
    }
    return mutex;
    
    #endif
    
    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )
    
    pthread_mutexattr_t attr;
    pthread_mutexattr_init( &attr );
    pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE );

    TMutex* mutex = malloc( sizeof( TMutex ) );
    mutex->reentrancyCount = 0;
    mutex->lockedByThread = 0;

    if ( pthread_mutex_init( &mutex->id, &attr ) != 0 )
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
    MutexLock( mutex, GUCEF_UINT32MAX );
    
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
    
    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )
    
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
            UInt32 callerThreadId = (UInt32) GetCurrentThreadId();
            if ( callerThreadId == mutex->lockedByThread )             
                ++mutex->reentrancyCount;
            else
                mutex->lockedByThread = callerThreadId;

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
            UInt32 callerThreadId = (UInt32) GetCurrentThreadId();
            mutex->lockedByThread = callerThreadId;
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
            UInt32 callerThreadId = (UInt32) GetCurrentThreadId();
            if ( callerThreadId == mutex->lockedByThread )             
                ++mutex->reentrancyCount;
            else
                mutex->lockedByThread = callerThreadId;

            GUCEF_TRACE_EXCLUSIVE_LOCK_OBTAINED( &mutex->critsection );
            return LOCKSTATUS_OPERATION_SUCCESS;
        }
    }
    while ( timeWaitedInMs < timeoutInMs );
    
    return LOCKSTATUS_WAIT_TIMEOUT;

    #endif
    
    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )
    
    UInt32 callerThreadId = (UInt32) pthread_self();
    int lockResult = pthread_mutex_lock( &mutex->id );

    if ( 0 != lockResult ) 
        return GUCEF_MUTEX_OPERATION_FAILED;
    
    mutex->lockedByThread = callerThreadId;
    if ( callerThreadId == mutex->lockedByThread )             
        ++mutex->reentrancyCount;
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

    UInt32 callerThreadId = (UInt32) GetCurrentThreadId();
    if ( mutex->lockedByThread == callerThreadId )
    {
        if ( mutex->reentrancyCount > 0 )
            --mutex->reentrancyCount;
        else
            mutex->lockedByThread = 0;
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

    UInt32 callerThreadId = (UInt32) GetCurrentThreadId();
    if ( mutex->lockedByThread == callerThreadId )
    {
        if ( mutex->reentrancyCount > 0 )
            --mutex->reentrancyCount;
        else
            mutex->lockedByThread = 0;
    
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
    
    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )
    
    UInt32 callerThreadId = (UInt32) pthread_self();
    if ( mutex->lockedByThread == callerThreadId )
    {
        if ( mutex->reentrancyCount > 0 )
            --mutex->reentrancyCount;
        else
            mutex->lockedByThread = 0;
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
