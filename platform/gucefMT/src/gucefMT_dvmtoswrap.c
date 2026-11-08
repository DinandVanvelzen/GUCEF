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

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
  #define _GNU_SOURCE
#endif

#include <assert.h>
#include <malloc.h>
#include <string.h>

#include "gucefMT_dvmtoswrap.h"         /* the function prototypes */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

  #include <Mmsystem.h>  /* needed for timeBeginPeriod() etc */
  #include <processthreadsapi.h>

  #ifndef GUCEF_MT_MSWINUTILS_H
  #include "gucefMT_mswinutils.h"
  #define GUCEF_MT_MSWINUTILS_H
  #endif /* GUCEF_MT_MSWINUTILS_H ? */

#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

  #include <stdio.h>
  #include <stdlib.h>
  #include <unistd.h>
  #include <pthread.h>
  #include <signal.h>
  #include <sys/time.h>
  #include <time.h>
  #include <errno.h>

#elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

  #include <stdio.h>
  #include <stdlib.h>
  #include <unistd.h>
  #include <pthread.h>
  #include <errno.h>
  #include <emscripten/threading.h>
  #include <emscripten/emscripten.h>
  #include <sys/time.h>

#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_MACOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_IPHONEOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_IPADOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_TVOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_VISIONOS ) )

  #include <stdint.h>
  #include <pthread.h>

#endif

/*
 *  Compile time check that TThreadId holds every bit of the platform's thread identity (see GetCurrentThreadID()).
 *  A failure here means the bitness detection in gucef_platform.h picked the wrong width for this target.
 */
#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
typedef char TThreadIdHoldsTheNativeThreadId[ ( sizeof( DWORD ) <= sizeof( TThreadId ) ) ? 1 : -1 ];
#else
typedef char TThreadIdHoldsTheNativeThreadId[ ( sizeof( pthread_t ) <= sizeof( TThreadId ) ) ? 1 : -1 ];
#endif

/*
 *  Compile time check that TSystemThreadId holds the operating system's thread id (see GetCurrentThreadSystemID())
 */
#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
typedef char TSystemThreadIdHoldsTheNativeThreadId[ ( sizeof( DWORD ) <= sizeof( TSystemThreadId ) ) ? 1 : -1 ];
#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
typedef char TSystemThreadIdHoldsTheNativeThreadId[ ( sizeof( pid_t ) <= sizeof( TSystemThreadId ) ) ? 1 : -1 ];
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

struct SThreadData
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    DWORD threadid;
    HANDLE threadhandle;
    LPTHREAD_START_ROUTINE func;
    void* data;

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    pthread_t thread;
    pthread_attr_t attr;
    TThreadFunc func;
    void* data;
    pid_t threadId;
    pthread_cond_t exitSignal;
    pthread_mutex_t startMutex;     /* guards 'started', the hand shake between ThreadCreate() and the new thread */
    pthread_cond_t startSignal;
    int started;                    /* set by the new thread once threadId and threadStatus are valid */

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    pthread_t thread;
    pthread_attr_t attr;
    TThreadFunc func;
    void* data;
    pthread_cond_t exitSignal;

    #endif

    UInt32 returnValue;
    TThreadStatus threadStatus;
};
typedef struct SThreadData TThreadData;

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      needed for PrecisionDelay() and PrecisionTickCount()
 *      Initialized in PrecisionTimerInit()
 */
#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
static LARGE_INTEGER highPerfTimerFrequency = { 0 };
static double highPerfTimerFrequencyTicksPerMs = 0;
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
ThreadDelay( UInt32 delay )
{
    GUCEF_BEGIN;
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    Sleep( delay );
    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    /* note that sleep() takes seconds while delay is in milliseconds */
    struct timespec remaining;
    remaining.tv_sec = (time_t) ( delay / 1000 );
    remaining.tv_nsec = (long) ( ( delay % 1000 ) * 1000000 );
    while ( 0 != nanosleep( &remaining, &remaining ) && EINTR == errno ) {}
    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
    usleep( delay * 1000 ); // usleep expects microseconds
    #endif
    GUCEF_END;
}

/*--------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

static UInt32 GUCEF_CALLSPEC_STD_PREFIX
ThreadMain( void* tdvptr ) GUCEF_CALLSPEC_STD_SUFFIX
{
    if ( GUCEF_NULL == tdvptr )
        return 0;

    GUCEF_BEGIN;
    ((TThreadData*)tdvptr)->threadStatus = THREADSTATUS_RUNNING;
    ((TThreadData*)tdvptr)->returnValue = (UInt32) ( (TThreadData*)tdvptr)->func( ((TThreadData*)tdvptr)->data );
    GUCEF_END;
    return ((TThreadData*)tdvptr)->returnValue;
}

#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

void*
ThreadMain( void* tdvptr )
{
    if ( GUCEF_NULL == tdvptr )
        return GUCEF_NULL;

    GUCEF_BEGIN;
    TThreadData* td = (TThreadData*) tdvptr;

    /*
     *  Publish the thread id and status to the thread that is in ThreadCreate(): it waits for this before it returns,
     *  so ThreadID() is valid for every caller from the moment ThreadCreate() has returned, without a data race.
     */
    pthread_mutex_lock( &td->startMutex );
    td->threadStatus = THREADSTATUS_RUNNING;
    td->threadId = gettid();
    td->started = 1;
    pthread_cond_signal( &td->startSignal );
    pthread_mutex_unlock( &td->startMutex );

    td->returnValue = td->func( td->data );
    pthread_cond_signal( &td->exitSignal );
    GUCEF_END;
    return GUCEF_NULL;
}

#elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

void*
ThreadMain( void* tdvptr )
{
    if ( GUCEF_NULL == tdvptr )
        return GUCEF_NULL;

    GUCEF_BEGIN;
    TThreadData* td = (TThreadData*) tdvptr;
    td->threadStatus = THREADSTATUS_RUNNING;
    td->returnValue = td->func( td->data );
    pthread_cond_signal( &td->exitSignal );
    GUCEF_END;
    return GUCEF_NULL;
}

#endif

/*--------------------------------------------------------------------------*/

struct SThreadData*
ThreadDataReserve( void )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    TThreadData* td = malloc( sizeof( TThreadData ) );
    if ( GUCEF_NULL != td )
    {
        memset( td, 0, sizeof(TThreadData) );
        td->threadStatus = THREADSTATUS_UNDEFINED;
    }
    return td;

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    TThreadData* td = malloc( sizeof( TThreadData ) );
    if ( GUCEF_NULL != td )
    {
        memset( td, 0, sizeof(TThreadData) );
        pthread_cond_init( &td->exitSignal, GUCEF_NULL );
        pthread_mutex_init( &td->startMutex, GUCEF_NULL );
        pthread_cond_init( &td->startSignal, GUCEF_NULL );
        td->threadStatus = THREADSTATUS_UNDEFINED;
    }
    return td;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    TThreadData* td = malloc( sizeof( TThreadData ) );
    if ( GUCEF_NULL != td )
    {
        memset( td, 0, sizeof(TThreadData) );
        pthread_cond_init( &td->exitSignal, GUCEF_NULL );
        td->threadStatus = THREADSTATUS_UNDEFINED;
    }
    return td;

    #else

    #error unsupported target platform
    return GUCEF_NULL;

    #endif
}

/*--------------------------------------------------------------------------*/

void
ThreadDataCleanup( struct SThreadData* td )
{
    if ( GUCEF_NULL != td )
    {
        ThreadKill( td );

        #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )
        pthread_cond_destroy( &td->exitSignal );
        #endif
        #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
        pthread_cond_destroy( &td->startSignal );
        pthread_mutex_destroy( &td->startMutex );
        #endif

        free( td );
    }
}

/*--------------------------------------------------------------------------*/

UInt32
ThreadCreate( struct SThreadData* td ,
              TThreadFunc func       ,
              void* data             )
{
    if ( GUCEF_NULL == td )
        return 0;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    td->func = (LPTHREAD_START_ROUTINE)func;
    td->data = data;
    td->threadhandle = CreateThread( NULL                                ,  /* Security Descriptor */
                                     0                                   ,  /* initial stack size */
                                     (LPTHREAD_START_ROUTINE) ThreadMain ,  /* thread function */
                                     (LPVOID) td                         ,  /* thread argument */
                                     CREATE_SUSPENDED                    ,  /* creation option: the thread id is stored before the thread runs */
                                     &td->threadid                       ); /* thread identifier */
    if ( td->threadhandle == NULL )
    {
        td->threadStatus = THREADSTATUS_CREATION_FAILED;
        return 0;
    }
    if ( (DWORD) -1 == ResumeThread( td->threadhandle ) )
    {
        TerminateThread( td->threadhandle, 1 );
        CloseHandle( td->threadhandle );
        td->threadhandle = NULL;
        td->threadStatus = THREADSTATUS_CREATION_FAILED;
        return 0;
    }
    return 1;

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    int retVal = 0;
    /* the primitives were initialized by ThreadDataReserve(), the memset resets them so they are made again */
    pthread_cond_destroy( &td->exitSignal );
    pthread_cond_destroy( &td->startSignal );
    pthread_mutex_destroy( &td->startMutex );
    memset( td, 0, sizeof( TThreadData ) );
    pthread_cond_init( &td->exitSignal, GUCEF_NULL );
    pthread_mutex_init( &td->startMutex, GUCEF_NULL );
    pthread_cond_init( &td->startSignal, GUCEF_NULL );

    retVal = pthread_attr_init( &td->attr );
    if ( 0 != retVal )
    {
        /* failed to init thread attributes structure */
        td->threadStatus = THREADSTATUS_CREATION_FAILED;
        return 0;
    }

    td->data = data;
    td->func = func;

    if ( 0 != pthread_create( &td->thread         ,
                              &td->attr           ,
                              (void*) ThreadMain  ,
                              (void*) td          ) )
    {
        pthread_cond_signal( &td->exitSignal );
        td->threadStatus = THREADSTATUS_CREATION_FAILED;
        return 0;
    }

    /*
     *  Wait until the new thread has published its thread id and status, so that they can be used
     *  right away by whoever has the thread data, from any thread, without a data race.
     *  The new thread does this before it runs the thread function so this cannot depend on anything the caller holds.
     */
    pthread_mutex_lock( &td->startMutex );
    while ( 0 == td->started )
        pthread_cond_wait( &td->startSignal, &td->startMutex );
    pthread_mutex_unlock( &td->startMutex );
    return 1;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    int retVal = 0;

    retVal = pthread_attr_init( &td->attr );
    if ( 0 != retVal )
    {
        /* failed to init thread attributes structure */
        td->threadStatus = THREADSTATUS_CREATION_FAILED;
        return 0;
    }

    td->data = data;
    td->func = func;

    if ( 0 != pthread_create( &td->thread         ,
                              &td->attr           ,
                              (void* (*)(void*)) ThreadMain  ,
                              (void*) td          ) )
    {
        pthread_cond_signal( &td->exitSignal );
        td->threadStatus = THREADSTATUS_CREATION_FAILED;
        return 0;
    }
    td->threadStatus = THREADSTATUS_RUNNING;
    return 1;

    #else

    #error unsupported target platform
    return 0;

    #endif
}

/*--------------------------------------------------------------------------*/

TSystemThreadId
ThreadID( struct SThreadData* td )
{
    if ( GUCEF_NULL == td )
        return 0;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    return (TSystemThreadId) td->threadid;
    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
      #if GUCEF_SYSTEM_THREAD_ID_IS_THREAD_ID
    /* forced to be the identity, as GetCurrentThreadSystemID() gives it then */
    return (TSystemThreadId) td->thread;
      #else
    return (TSystemThreadId) td->threadId;
      #endif
    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
    /* the id is the handle that the creating thread got from pthread_create(), the new thread never writes it */
    return (TSystemThreadId)(uintptr_t)td->thread;
    #else
    #error unsupported target platform
    #endif
}

/*--------------------------------------------------------------------------*/

TThreadId
ThreadIdentity( struct SThreadData* td )
{
    if ( GUCEF_NULL == td )
        return 0;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    return (TThreadId) td->threadid;
    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    /* the pthread_t that pthread_create() gave the creating thread: what pthread_self() gives on the new thread */
    return (TThreadId) td->thread;
    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
    return (TThreadId)(uintptr_t)td->thread;
    #else
    #error unsupported target platform
    #endif
}

/*--------------------------------------------------------------------------*/

TThreadStatus
ThreadStatus( struct SThreadData* td )
{
    if ( GUCEF_NULL == td )
        return 0;
    return td->threadStatus;
}

/*--------------------------------------------------------------------------*/

UInt32
ThreadSuspend( struct SThreadData* td )
{
    if ( GUCEF_NULL == td )
        return 0;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    return ( -1 != SuspendThread( td->threadhandle ) );

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    /* TODO: Implement with signals */
    return 0;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    /* Thread suspension is not directly supported in Emscripten/pthreads */
    return 0;

    #else

    #error unsupported target platform
    return 0;

    #endif
}

/*--------------------------------------------------------------------------*/

UInt32
ThreadResume( struct SThreadData* td )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    return ( -1 != ResumeThread( td->threadhandle ) );

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    /* TODO: Implement with signals */
    return 0;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    /* Thread resumption is not directly supported in Emscripten/pthreads */
    return 0;

    #else

    #error unsupported target platform
    return 0;

    #endif
}

/*--------------------------------------------------------------------------*/

UInt32
ThreadKill( struct SThreadData* td )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    if ( td != NULL )
    {
        UInt32 retval = TerminateThread( td->threadhandle ,
                                         1                );
        return retval;
    }
    return 0;

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    if ( NULL != td && 0 != td->thread )
    {
        /*
         *  Note that a thread handle is no longer valid once the thread has been joined,
         *  using it after that (as pthread_kill() used to do here) is undefined behaviour and crashes with glibc.
         *  As such td->thread is cleared whenever the thread is reaped.
         */
        Int32 retval = 0;
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
        if ( 0 != pthread_tryjoin_np( td->thread, GUCEF_NULL ) )
        {
            /* The thread is still running: cancellation is the closest equivalent to TerminateThread() */
            retval = (Int32) pthread_cancel( td->thread );
            struct timespec ts;
            clock_gettime( CLOCK_REALTIME, &ts );
            ts.tv_sec += 5;
            if ( 0 != retval || 0 != pthread_timedjoin_np( td->thread, GUCEF_NULL, &ts ) )
            {
                /* We could not reap the thread, let the system clean it up once it ends */
                pthread_detach( td->thread );
            }
        }
        #else
        /* Android has no thread cancellation: detach so the thread is cleaned up once it ends */
        pthread_detach( td->thread );
        #endif
        td->thread = 0;

        pthread_cond_signal( &td->exitSignal );
        pthread_attr_destroy( &td->attr );
        return retval;
    }
    return 0;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    if ( NULL != td && 0 != td->thread )
    {
        pthread_cond_signal( &td->exitSignal );
        return 1;
    }
    return 0;

    #else

    #error unsupported target platform
    return 0;

    #endif
}

/*--------------------------------------------------------------------------*/

UInt32
ThreadWait( struct SThreadData* td ,
            Int32 timeoutInMs      )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    if ( NULL != td && NULL != td->threadhandle )
    {
        DWORD result = WaitForSingleObject( td->threadhandle, timeoutInMs >= 0 ? (DWORD) timeoutInMs : INFINITE );
        switch ( result )
        {
            case WAIT_OBJECT_0:
                return GUCEF_THREAD_WAIT_OK;
            case WAIT_TIMEOUT:
                return GUCEF_THREAD_WAIT_TIMEOUT;
            case WAIT_ABANDONED:
                return GUCEF_THREAD_WAIT_ABANDONEND;

            case WAIT_FAILED:
            default:
                return GUCEF_THREAD_WAIT_FAILED;
        }
    }
    return 0;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

    if ( GUCEF_NULL != td && 0 != td->thread && timeoutInMs < 0 )
    {
        /* A negative timeout means wait indefinitely */
        if ( 0 == pthread_join( td->thread, GUCEF_NULL ) )
        {
            /* the handle is no longer valid once joined */
            td->thread = 0;
            return GUCEF_THREAD_WAIT_OK;
        }
        return GUCEF_THREAD_WAIT_FAILED;
    }
    if ( GUCEF_NULL != td && 0 != td->thread )
    {
        static const long max_nsec = 1000000 * 1000;
        struct timespec ts;
        clock_gettime( CLOCK_REALTIME, &ts );
        ts.tv_sec += timeoutInMs / 1000;
        ts.tv_nsec += 1000000 * (timeoutInMs%1000);
        if ( ts.tv_nsec >= max_nsec )
        {
            ts.tv_sec += 1;
            ts.tv_nsec = ts.tv_nsec%max_nsec;
        }
        int errorCode = pthread_timedjoin_np( td->thread, GUCEF_NULL, &ts );
        if ( 0 == errorCode )
        {
            /* the handle is no longer valid once joined */
            td->thread = 0;
            return GUCEF_THREAD_WAIT_OK;
        }
        if ( ETIMEDOUT == errorCode )
            return GUCEF_THREAD_WAIT_TIMEOUT;
    }
    return GUCEF_THREAD_WAIT_FAILED;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )

    if ( GUCEF_NULL != td && 0 != td->thread )
    {
        static const long max_nsec = 1000000 * 1000;
        struct timespec ts;
        clock_gettime( CLOCK_REALTIME, &ts );
        ts.tv_sec += timeoutInMs / 1000;
        ts.tv_nsec += 1000000 * (timeoutInMs%1000);
        if ( ts.tv_nsec >= max_nsec )
        {
            ts.tv_sec += 1;
            ts.tv_nsec = ts.tv_nsec%max_nsec;
        }
        int errorCode = pthread_cond_timedwait( &td->exitSignal, GUCEF_NULL, &ts );
        if ( 0 == errorCode )
            return GUCEF_THREAD_WAIT_OK;
    }
    return GUCEF_THREAD_WAIT_FAILED;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    if ( GUCEF_NULL != td && 0 != td->thread )
    {
        if ( timeoutInMs < 0 )
        {
            int errorCode = pthread_join( td->thread, GUCEF_NULL );
            if ( 0 == errorCode )
                return GUCEF_THREAD_WAIT_OK;
        }
        else
        {
            struct timespec ts;
            struct timeval tv;
            gettimeofday( &tv, GUCEF_NULL );
            
            ts.tv_sec = tv.tv_sec + timeoutInMs / 1000;
            ts.tv_nsec = ( tv.tv_usec * 1000 ) + ( ( timeoutInMs % 1000 ) * 1000000 );
            if ( ts.tv_nsec >= 1000000000L )
            {
                ts.tv_sec += ts.tv_nsec / 1000000000L;
                ts.tv_nsec %= 1000000000L;
            }

            pthread_mutex_t dummy_mutex;
            pthread_mutex_init( &dummy_mutex, NULL );
            pthread_mutex_lock( &dummy_mutex );
            
            int errorCode = pthread_cond_timedwait( &td->exitSignal, &dummy_mutex, &ts );
            
            pthread_mutex_unlock( &dummy_mutex );
            pthread_mutex_destroy( &dummy_mutex );

            if ( 0 == errorCode )
            {
                pthread_join( td->thread, GUCEF_NULL );
                return GUCEF_THREAD_WAIT_OK;
            }
            else if ( errorCode == ETIMEDOUT )
            {
                return GUCEF_THREAD_WAIT_TIMEOUT;
            }
        }
    }
    return GUCEF_THREAD_WAIT_FAILED;
    
    #else

    #error unsupported target platform
    return 0;

    #endif
}

/*--------------------------------------------------------------------------*/

GUCEF_MT_PUBLIC_C UInt32
ThreadSetCpuAffinity( struct SThreadData* td  ,
                      UInt32 affinityMaskSize ,
                      void* affinityMask      )
{
    if ( GUCEF_NULL == affinityMask || 0 == affinityMaskSize )
        return 0;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    /* we can only set 64 processors in the current processor group for now
       the passed affinity mask is assumed to be the current processor group.
       @TODO: Improve this
    */
    if ( affinityMaskSize <= 8 )
    {
        DWORD_PTR mask = 0;
        if ( 8 == affinityMaskSize )
            mask = (DWORD_PTR) *(UInt64*) affinityMask;
        else
        if ( 4 == affinityMaskSize )
            mask = (DWORD_PTR) *(UInt32*) affinityMask;
        else
        if ( 2 == affinityMaskSize )
            mask = (DWORD_PTR) *(UInt16*) affinityMask;
        else
        if ( 1 == affinityMaskSize )
            mask = (DWORD_PTR) *(UInt8*) affinityMask;

        mask = SetThreadAffinityMask( td->threadhandle, mask );
        if ( 0 != mask )
            return 1;
    }
    return 0;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

    if ( GUCEF_NULL != td && 0 != td->thread )
    {
        int statusCode = pthread_setaffinity_np( td->thread, (size_t) affinityMaskSize, (cpu_set_t*) affinityMask );
        if ( 0 == statusCode )
            return 1;
    }
    return 0;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )

    if ( GUCEF_NULL != td && 0 != td->threadId )
    {
        int statusCode = sched_setaffinity( td->threadId, (size_t) affinityMaskSize, (cpu_set_t*) affinityMask );
        if ( 0 == statusCode )
            return 1;
    }
    return 0;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    return 0;

    #else

    #error unsupported target platform
    return 0;

    #endif
}

/*--------------------------------------------------------------------------*/

UInt32
ThreadGetCpuAffinity( struct SThreadData* td        ,
                      UInt32 affinityMaskBufferSize ,
                      void* affinityMask            ,
                      UInt32* affinityMaskSize      )
{
    if ( GUCEF_NULL == td )
        return 0;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    /*
     *  Windows has SetThreadAffinityMask but no official API function is provided for Getting said mask.
     *  We use the 'undocumented' NtQueryInformationThread function if its available
     */
    struct WIN32_THREAD_BASIC_INFORMATION basicInfo;
    if ( GUCEF_NULL != affinityMaskSize )
    {
        *affinityMaskSize = sizeof( basicInfo.AffinityMask );
    }
    memset( &basicInfo, 0, sizeof(basicInfo) );
    if ( WIN32_NT_SUCCESS( TryNtQueryInformationThread( td->threadhandle, ThreadBasicInformation, &basicInfo, sizeof(basicInfo), NULL ) ) )
    {
        if ( affinityMaskBufferSize >= sizeof( basicInfo.AffinityMask ) )
        {
            memcpy( affinityMask, &basicInfo.AffinityMask, sizeof( basicInfo.AffinityMask ) );

            return 1;
        }
    }
    return 0;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

    if ( GUCEF_NULL != affinityMaskSize )
    {
        *affinityMaskSize = sizeof(cpu_set_t);
    }
    if ( GUCEF_NULL != affinityMask )
    {
        int statusCode = pthread_getaffinity_np( td->thread, (size_t) affinityMaskBufferSize, (cpu_set_t*) affinityMask );
        if ( 0 == statusCode )
            return 1;
    }
    return 0;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )

    if ( GUCEF_NULL != affinityMaskSize )
    {
        *affinityMaskSize = sizeof(cpu_set_t);
    }
    if ( GUCEF_NULL != affinityMask && GUCEF_NULL != td && 0 != td->threadId )
    {
        /* the kernel thread id, as ThreadSetCpuAffinity() uses it: a pthread_t is a pointer on Android, not a thread id */
        int statusCode = sched_getaffinity( td->threadId, (size_t) affinityMaskBufferSize, (cpu_set_t*) affinityMask );
        if ( 0 == statusCode )
            return 1;
    }
    return 0;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    /* CPU affinity is not supported in WebAssembly */
    if ( GUCEF_NULL != affinityMaskSize )
    {
        *affinityMaskSize = 0;
    }
    return 0;

    #else

    #error unsupported target platform
    return 0;

    #endif
}

/*--------------------------------------------------------------------------*/

UInt32
ThreadSetCpuAffinityByCpuId( struct SThreadData* td ,
                             UInt32 cpuId           )
{

    if ( cpuId < 64 )
    {
        UInt64 cpuMask = 0;
        cpuMask |= (UInt64)1 << cpuId;

        return ThreadSetCpuAffinity( td, sizeof( cpuMask ), &cpuMask );
    }
    else
    {
        return 0;
    }
}

/*--------------------------------------------------------------------------*/

UInt32
ThreadGetCpuAffinityByCpuId( struct SThreadData* td ,
                             UInt32* cpuId          )
{
    if ( GUCEF_NULL == td || GUCEF_NULL == cpuId )
        return 0;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    *cpuId = (UInt32) GetCurrentProcessorNumber();
    return 1;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
    
    /* CPU affinity is not supported in WebAssembly */
    *cpuId = 0;
    return 0;
    
    #else

    UInt64 affinityMask = 0;
    UInt32 affinityMaskSize = sizeof( affinityMask );
    if ( 0 != ThreadGetCpuAffinity( td                ,
                                    affinityMaskSize  ,
                                    &affinityMask     ,
                                    &affinityMaskSize ) )
    {
        UInt32 i=0;
        for ( i=0; i<64; ++i )
        {
            if ( 0 != GUCEF_GETBITX( affinityMask, i ) )
            {
                *cpuId = i;
                return 1;
            }
        }
    }
    return 0;

    #endif
}

/*--------------------------------------------------------------------------*/

/*
 *  The identity of the calling thread, see GetCurrentThreadID(). Static, so the compiler inlines it into GetCurrentThreadIds()
 *  and recording both ids of an owner costs no extra call.
 */
static TThreadId
CurrentThreadIdImpl( void )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    /* the Win32 function, spelled with a lowercase d */
    return (TThreadId) GetCurrentThreadId();

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    /*
     *  pthread_t is an integer here (unsigned long in glibc and musl, long in bionic) of the same width as TThreadId:
     *  the conversion keeps every bit. pthread_self() reads a register, gettid() would be a system call per lock.
     */
    return (TThreadId) pthread_self();

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_MACOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_IPHONEOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_IPADOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_TVOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_VISIONOS )

    /*
     *  pthread_t is a pointer here. The main runtime thread of Emscripten has a valid pthread_self() as well, with
     *  or without thread support, so it gets a real id like every other thread (never 0, which means "no thread")
     */
    return (TThreadId) (uintptr_t) pthread_self();

    #else

    #error unsupported target platform
    return 0;

    #endif
}

/*--------------------------------------------------------------------------*/

TThreadId
GetCurrentThreadID( void )
{
    return CurrentThreadIdImpl();
}

/*--------------------------------------------------------------------------*/

#if !GUCEF_SYSTEM_THREAD_ID_IS_THREAD_ID && ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

/*
 *  gettid() is a system call: GetCurrentThreadSystemID() keeps the answer per thread. A per-thread copy is needed, a
 *  copy kept with anything keyed by GetCurrentThreadID() would go stale because pthread_t values are reused for new
 *  threads. fork() gives the forking thread another id in the child while its copy stays: the child clears it.
 */
#if defined( __GLIBC__ ) && ( defined( __GNUC__ ) || defined( __clang__ ) )
  /*
   *  read on every lock that records its owner: the initial-exec model makes that one load relative to the thread
   *  pointer instead of a call to __tls_get_addr(). glibc keeps static TLS space in reserve for libraries loaded with
   *  dlopen(), which this single variable fits in.
   */
  #define GUCEF_MT_SYSTEM_THREAD_ID_TLS_MODEL   __attribute__(( tls_model( "initial-exec" ) ))
#else
  #define GUCEF_MT_SYSTEM_THREAD_ID_TLS_MODEL
#endif
static __thread pid_t g_currentThreadSystemId GUCEF_MT_SYSTEM_THREAD_ID_TLS_MODEL = 0;
static pthread_once_t g_currentThreadSystemIdForkHandlerOnce = PTHREAD_ONCE_INIT;

static void
ClearCurrentThreadSystemIdInForkChild( void )
{
    g_currentThreadSystemId = 0;
}

static void
RegisterCurrentThreadSystemIdForkHandler( void )
{
    pthread_atfork( GUCEF_NULL, GUCEF_NULL, &ClearCurrentThreadSystemIdInForkChild );
}

#endif

/*--------------------------------------------------------------------------*/

/*
 *  The operating system's id of the calling thread, see GetCurrentThreadSystemID()
 */
static TSystemThreadId
CurrentThreadSystemIdImpl( void )
{
    #if GUCEF_SYSTEM_THREAD_ID_IS_THREAD_ID

    /*
     *  The same value as the identity: on Windows both are the DWORD of GetCurrentThreadId(), on Emscripten both are
     *  the pthread handle. A build that forces the define to 1 on another platform gets the identity as well, so the
     *  system id of an owner recorded by an object always equals what this function gives on the owning thread.
     */
    return (TSystemThreadId) CurrentThreadIdImpl();

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    /* only with the define forced to 0: the same DWORD */
    return (TSystemThreadId) GetCurrentThreadId();

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    /* only with the define forced to 0: the pthread handle, as ThreadID() gives it */
    return (TSystemThreadId) (uintptr_t) pthread_self();

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    /* pid_t is 32 bit, the same number ThreadID() stores for a thread created with ThreadCreate() */
    if ( 0 == g_currentThreadSystemId )
    {
        pthread_once( &g_currentThreadSystemIdForkHandlerOnce, &RegisterCurrentThreadSystemIdForkHandler );
        g_currentThreadSystemId = gettid();
    }
    return (TSystemThreadId) g_currentThreadSystemId;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_MACOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_IPHONEOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_IPADOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_TVOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_VISIONOS )

    /* a 32 bit Mach port name, unique among the live threads of the process */
    return (TSystemThreadId) pthread_mach_thread_np( pthread_self() );

    #else

    #error unsupported target platform: decide GUCEF_SYSTEM_THREAD_ID_IS_THREAD_ID for it in gucef_platform.h
    return 0;

    #endif
}

/*--------------------------------------------------------------------------*/

TSystemThreadId
GetCurrentThreadSystemID( void )
{
    return CurrentThreadSystemIdImpl();
}

/*--------------------------------------------------------------------------*/

void
GetCurrentThreadIds( struct SThreadIds* ids )
{
    ids->threadId = CurrentThreadIdImpl();

    #if !GUCEF_SYSTEM_THREAD_ID_IS_THREAD_ID
      #if GUCEF_MT_RECORD_SYSTEM_THREAD_ID
    ids->systemThreadId = CurrentThreadSystemIdImpl();
      #else
    ids->systemThreadId = 0;
      #endif
    #endif
}

/*--------------------------------------------------------------------------*/

UInt64
PrecisionTickCount( void )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    LARGE_INTEGER t;

    assert( sizeof( LARGE_INTEGER ) <= sizeof( UInt64 ) );

    if ( QueryPerformanceCounter( &t ) == TRUE )
    {
        return t.QuadPart;
    }
    else
    {
        return GetTickCount64();
    }
    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    struct timeval time;
    gettimeofday( &time, NULL );

    return time.tv_usec +    /* Microseconds. */
           time.tv_sec * 1000000; /* Seconds. */

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    return (UInt64)(emscripten_get_now() * 1000.0);

    #else

    #error unsupported target platform
    return 0;

    #endif
}

/*--------------------------------------------------------------------------*/

UInt64
PrecisionTimerResolution( void )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    ULARGE_INTEGER t;
    if ( QueryPerformanceFrequency( (LARGE_INTEGER*) &t ) == TRUE )
    {
        return t.QuadPart;
    }
    else
    {
        return 100; /* this is the WIN32 resolution of GetTickCount(); which has a time-slice size of about 10 ms */
    }

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    return 1000000;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    return 1000000; /* microseconds precision */

    #else

    #error unsupported target platform
    return 0;

    #endif
}

/*--------------------------------------------------------------------------*/

void
PrecisionDelay( UInt32 delayInMs )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    /*
     *      note: BE SURE YOU CALL timeBeginPeriod(1) at program startup!!!
     *      note: BE SURE YOU CALL timeEndPeriod(1) at program exit!!!
     *      note: that this code will require linking to winmm.lib
     */

    LARGE_INTEGER startTime;
    LARGE_INTEGER currentTime;
    LONGLONG ticksPassed;
    LONGLONG ticksToWait;
    LONGLONG ticksLeft;
    BOOL qpcSuccess;
    int done = 0;

    qpcSuccess = QueryPerformanceCounter( &startTime );
    if ( FALSE == qpcSuccess )
    {
        /*
         *  Fall back to regular sleep
         */
         Sleep( delayInMs );
         return;
    }

    ticksToWait = (LONGLONG) ( highPerfTimerFrequencyTicksPerMs * delayInMs );

    while ( 1 )
    {
        QueryPerformanceCounter( &currentTime );
        ticksPassed = (LONGLONG) ( currentTime.QuadPart - startTime.QuadPart );
        if ( ticksPassed < 0 )
            return;

        ticksLeft = (LONGLONG) ( ticksToWait - ticksPassed );
        if ( ticksLeft <= 0 )
            return;

        if ( ticksLeft > (Int32) ( highPerfTimerFrequencyTicksPerMs * 50 ) )
        {
            // Lets not hog the CPU if the time we have to wait is lengthy
            UInt32 longSleepTimeInMs = (UInt32) ( ( ticksLeft - (highPerfTimerFrequencyTicksPerMs*15) ) / highPerfTimerFrequencyTicksPerMs );
            Sleep( longSleepTimeInMs );
        }
        else
        if ( ticksLeft > (Int32) ( highPerfTimerFrequencyTicksPerMs * 20 ) )
        {
            Sleep( 10 );
        }
        else

        /*
         *      if > 2ms left, do Sleep(1), which will actually sleep some
         *      steady amount, probably 1-2 ms,
         *      and do so in a nice way (cpu meter drops; laptop battery spared).
         *      otherwise, do a few Sleep(0)'s, which just give up the time slice,
         *      but don't really save cpu or battery, but do pass a tiny
         *     amount of time.
         */
        if ( ticksLeft > (Int32) ( highPerfTimerFrequencyTicksPerMs * 2 ) )
        {
            Sleep( 1 );
        }
        else
        {
            int i;
            for ( i=0; i<10; ++i )
            {
                Sleep( 0 );  /* causes thread to give up its time slice */
            }
            return;
        }
    }

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    usleep( delayInMs * 1000 );

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    usleep( delayInMs * 1000 );

    #else
    #error unsupported target platform
    #endif
}

/*--------------------------------------------------------------------------*/

Float64
PrecisionTimerTicksToMs( UInt64 ticks )
{
    /* divide the ticks by the time resolution per tick */
    Float64 milliSecs = ticks / ( PrecisionTimerResolution() / 1000.0 );
    return milliSecs;
}

/*--------------------------------------------------------------------------*/

void
PrecisionTimerInit( void )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    if ( highPerfTimerFrequency.QuadPart == 0 )
    {
        /*
         *      Change Sleep() resolution to 1-2 milliseconds
         *
         *      Note that calling timeBeginPeriod() also affects the granularity of some
         *      other timing calls, such as CreateWaitableTimer() and WaitForSingleObject();
         *      however, some functions are still unaffected, such as _ftime().
         */
        timeBeginPeriod( 1 );

        /*
         *      Initialize the timer frequency structure for use
         */
        QueryPerformanceFrequency( &highPerfTimerFrequency );
        highPerfTimerFrequencyTicksPerMs = (double) ( highPerfTimerFrequency.QuadPart / 1000.0 );
    }
    #endif
}

/*--------------------------------------------------------------------------*/

void
PrecisionTimerShutdown( void )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    if ( highPerfTimerFrequency.QuadPart != 0 )
    {
        /*
         *      Undo timer resolution change
         */
        timeEndPeriod( 1 );
    }
    #endif
}

/*--------------------------------------------------------------------------*/

UInt32
GetProcessID( void )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    return (UInt32) GetCurrentProcessId();

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    return (UInt32) getpid();

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    return 1; /* Only a single process in WebAssembly */

    #else

    #error unsupported target platform
    return 0;

    #endif
}

/*--------------------------------------------------------------------------*/
