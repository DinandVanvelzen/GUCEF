/*
 *  gucefMT: GUCEF module providing multithreading solutions
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

#ifndef GUCEF_MT_CSEMAPHORE_H
#include "gucefMT_CSemaphore.h"
#endif /* GUCEF_MT_CSEMAPHORE_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

#include <windows.h>

#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) || \
        ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_MACOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_IPHONEOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_IPADOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_TVOS ) )

#include <errno.h>
#include <pthread.h>
#include <time.h>

#else
#error Unsuported target platform
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

/**
 *  Platform specific semaphore storage.
 *
 *  Windows:       a native semaphore object. It has no way to query its count, so a counter that is kept
 *                 next to it (incremented before each release, decremented after each successful wait)
 *                 provides Value(). That value is a snapshot, as it is for every semaphore.
 *  Everything else (Linux, Android, Emscripten, Apple): a pthread mutex and condition variable guarding the count.
 *                 This is preferred over sem_t because
 *                 - the timeout can be bound to the monotonic clock, so a wall clock change cannot shorten or
 *                   lengthen a wait (sem_timedwait is bound to CLOCK_REALTIME only),
 *                 - sem_init is not supported on macOS and dispatch semaphores cannot report their count,
 *                 - one implementation then serves all POSIX like targets.
 *                 Post() only signals when somebody is waiting, so posting to a semaphore nobody waits on
 *                 costs an uncontended mutex lock and unlock and no system call.
 */
struct SSemaphoreData
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    ::HANDLE handle;
    volatile ::LONG count;

    SSemaphoreData( void ) : handle( GUCEF_NULL ), count( 0 ) {}

    #else

    pthread_mutex_t mutex;
    pthread_cond_t cond;
    UInt32 count;       /**< guarded by mutex */
    UInt32 waiters;     /**< threads blocked in the condition variable, guarded by mutex */

    SSemaphoreData( void ) : mutex(), cond(), count( 0 ), waiters( 0 ) {}

    #endif
};

/*--------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )

/** Scope for holding the mutex of a semaphore */
class CSemaphoreScopeLock
{
    public:

    explicit CSemaphoreScopeLock( SSemaphoreData& data ) : m_mutex( &data.mutex ) { pthread_mutex_lock( m_mutex ); }
    ~CSemaphoreScopeLock() { pthread_mutex_unlock( m_mutex ); }

    private:

    pthread_mutex_t* m_mutex;
    CSemaphoreScopeLock( const CSemaphoreScopeLock& src );
    CSemaphoreScopeLock& operator=( const CSemaphoreScopeLock& src );
};

#endif

/*--------------------------------------------------------------------------*/

/**
 *      constructor, allocates storage for a semaphore.
 */
CSemaphore::CSemaphore( UInt32 initial_value )
    : m_data( GUCEF_NEW SSemaphoreData )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    if ( initial_value > 0x7FFFFFFFU )      // a native semaphore counts in a signed 32 bit value
    {
        GUCEF_DELETE m_data;
        m_data = GUCEF_NULL;
        return;
    }

    m_data->handle = ::CreateSemaphore( NULL, (::LONG) initial_value, 0x7FFFFFFF, NULL );
    if ( GUCEF_NULL == m_data->handle )
    {
        GUCEF_DELETE m_data;
        m_data = GUCEF_NULL;
        return;
    }
    m_data->count = (::LONG) initial_value;

    #else

    if ( 0 != pthread_mutex_init( &m_data->mutex, GUCEF_NULL ) )
    {
        GUCEF_DELETE m_data;
        m_data = GUCEF_NULL;
        return;
    }

    pthread_condattr_t condAttributes;
    bool condOk = 0 == pthread_condattr_init( &condAttributes );
    if ( condOk )
    {
        #if ( ( GUCEF_PLATFORM != GUCEF_PLATFORM_APPLE_MACOS ) && ( GUCEF_PLATFORM != GUCEF_PLATFORM_APPLE_IPHONEOS ) && ( GUCEF_PLATFORM != GUCEF_PLATFORM_APPLE_IPADOS ) && ( GUCEF_PLATFORM != GUCEF_PLATFORM_APPLE_TVOS ) )
        // Apple has no way to select the clock, but its relative timed wait does not use a wall clock at all
        condOk = 0 == pthread_condattr_setclock( &condAttributes, CLOCK_MONOTONIC );
        #endif
        condOk = condOk && 0 == pthread_cond_init( &m_data->cond, &condAttributes );
        pthread_condattr_destroy( &condAttributes );
    }
    if ( !condOk )
    {
        pthread_mutex_destroy( &m_data->mutex );
        GUCEF_DELETE m_data;
        m_data = GUCEF_NULL;
        return;
    }
    m_data->count = initial_value;

    #endif
}

/*--------------------------------------------------------------------------*/

/**
 *      Destructor, de-allocates storage for a semaphore.
 *      No thread may be waiting on the semaphore anymore at this point.
 */
CSemaphore::~CSemaphore()
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_data )
        return;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    ::CloseHandle( m_data->handle );

    #else

    pthread_cond_destroy( &m_data->cond );
    pthread_mutex_destroy( &m_data->mutex );

    #endif

    GUCEF_DELETE m_data;
    m_data = GUCEF_NULL;
}

/*--------------------------------------------------------------------------*/

/**
 *      Suspends the calling thread until the semaphore has a positive
 *      count. It then atomically decreases the semaphore count.
 *      returns false in case of error.
 */
bool
CSemaphore::Wait( void )
{GUCEF_TRACE;

    return 0 == WaitOrTimeout( GUCEF_MT_INFINITE_LOCK_TIMEOUT );
}

/*--------------------------------------------------------------------------*/

/**
 *      Non-blocking variant of Wait(), returns 0 if the wait succeeds,
 *      1 if the wait would block, and -1 on error.
 */
Int32
CSemaphore::TryWait( void )
{GUCEF_TRACE;

    return WaitOrTimeout( 0 );
}

/*--------------------------------------------------------------------------*/

/**
 *      Variant of Wait() with a timeout in milliseconds, returns 0 if
 *      the wait succeeds, 1 if the wait does not succeed in
 *      the allotted time, and -1 on error. A timeout of 0 does not block
 *      and GUCEF_MT_INFINITE_LOCK_TIMEOUT waits without limit.
 */
Int32
CSemaphore::WaitOrTimeout( UInt32 ms )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_data )
        return -1;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    // GUCEF_MT_INFINITE_LOCK_TIMEOUT has the same value as INFINITE
    const ::DWORD waitResult = ::WaitForSingleObject( m_data->handle, (::DWORD) ms );
    switch ( waitResult )
    {
        case WAIT_OBJECT_0:
        {
            ::InterlockedDecrement( &m_data->count );
            return 0;
        }
        case WAIT_TIMEOUT:
        {
            return 1;
        }
        default:
        {
            return -1;
        }
    }

    #else

    CSemaphoreScopeLock lock( *m_data );

    if ( m_data->count > 0 )
    {
        --m_data->count;
        return 0;
    }
    if ( 0 == ms )
        return 1;

    #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_MACOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_IPHONEOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_IPADOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_TVOS ) )

    // relative wait, so it can restart after a spurious wake up with what is left of the timeout
    const UInt64 startTicks = PrecisionTickCount();
    for (;;)
    {
        int waitResult = 0;
        ++m_data->waiters;
        if ( GUCEF_MT_INFINITE_LOCK_TIMEOUT == ms )
        {
            waitResult = pthread_cond_wait( &m_data->cond, &m_data->mutex );
        }
        else
        {
            const Float64 elapsedMs = PrecisionTimerTicksToMs( PrecisionTickCount() - startTicks );
            if ( elapsedMs >= (Float64) ms )
            {
                --m_data->waiters;
                return 1;
            }
            const UInt64 leftNs = (UInt64) ( ( (Float64) ms - elapsedMs ) * 1000000.0 );
            struct timespec relative;
            relative.tv_sec = (time_t) ( leftNs / 1000000000ULL );
            relative.tv_nsec = (long) ( leftNs % 1000000000ULL );
            waitResult = pthread_cond_timedwait_relative_np( &m_data->cond, &m_data->mutex, &relative );
        }
        --m_data->waiters;
        if ( m_data->count > 0 )
        {
            --m_data->count;
            return 0;
        }
        if ( 0 != waitResult && ETIMEDOUT != waitResult )
            return -1;
    }

    #else

    struct timespec deadline;
    if ( GUCEF_MT_INFINITE_LOCK_TIMEOUT != ms )
    {
        // absolute deadline on the monotonic clock: spurious wake ups and signals that are stolen by another
        // thread restart the wait against the same deadline, so the total wait stays correct
        if ( 0 != clock_gettime( CLOCK_MONOTONIC, &deadline ) )
            return -1;
        deadline.tv_sec += (time_t) ( ms / 1000U );
        deadline.tv_nsec += (long) ( ms % 1000U ) * 1000000L;
        if ( deadline.tv_nsec >= 1000000000L )
        {
            deadline.tv_nsec -= 1000000000L;
            ++deadline.tv_sec;
        }
    }

    for (;;)
    {
        int waitResult = 0;
        ++m_data->waiters;
        if ( GUCEF_MT_INFINITE_LOCK_TIMEOUT == ms )
            waitResult = pthread_cond_wait( &m_data->cond, &m_data->mutex );
        else
            waitResult = pthread_cond_timedwait( &m_data->cond, &m_data->mutex, &deadline );
        --m_data->waiters;

        // a post that raced with the timeout counts as success
        if ( m_data->count > 0 )
        {
            --m_data->count;
            return 0;
        }
        if ( ETIMEDOUT == waitResult )
            return 1;
        if ( 0 != waitResult )
            return -1;
    }

    #endif

    #endif
}

/*--------------------------------------------------------------------------*/

/**
 *      Atomically increases the semaphore's count (not blocking)
 *      returns false in case of error.
 */
bool
CSemaphore::Post( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_data )
        return false;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    // counted before the release so that Value() never goes below what is really available
    ::InterlockedIncrement( &m_data->count );
    if ( FALSE == ::ReleaseSemaphore( m_data->handle, 1, NULL ) )
    {
        ::InterlockedDecrement( &m_data->count );
        return false;
    }
    return true;

    #else

    // signalled while holding the mutex: a waiter that wakes and destroys the semaphore can then not
    // overtake this thread, which has not finished with the mutex yet
    CSemaphoreScopeLock lock( *m_data );
    if ( m_data->count == 0xFFFFFFFFU )
        return false;
    ++m_data->count;
    if ( m_data->waiters > 0 )
        return 0 == pthread_cond_signal( &m_data->cond );
    return true;

    #endif
}

/*--------------------------------------------------------------------------*/

/**
 *      Returns the current count of the semaphore
 */
UInt32
CSemaphore::Value( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_data )
        return 0;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    const ::LONG count = m_data->count;
    return count < 0 ? 0 : (UInt32) count;

    #else

    CSemaphoreScopeLock lock( *m_data );
    return m_data->count;

    #endif
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MT */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
