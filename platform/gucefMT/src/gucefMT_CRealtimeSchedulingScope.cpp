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

#ifndef GUCEF_MT_CREALTIMESCHEDULINGSCOPE_H
#include "gucefMT_CRealtimeSchedulingScope.h"
#endif /* GUCEF_MT_CREALTIMESCHEDULINGSCOPE_H ? */

/* the system headers follow the platform, which the header above defines */
#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

  #include <windows.h>
  #define GUCEF_MT_REALTIME_SCHEDULING_MSWIN

#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

  #include <string.h>
  #include <pthread.h>
  #include <sched.h>
  #define GUCEF_MT_REALTIME_SCHEDULING_POSIX

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
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 CRealtimeSchedulingScope::DefaultPriority = 70;

namespace {

/** How a scope made its thread real-time (CRealtimeSchedulingScope::m_mechanism) */
enum ERealtimeMechanism
{
    REALTIMEMECHANISM_NONE              = 0,
    REALTIMEMECHANISM_POSIX_FIFO        = 1,    /**< SCHED_FIFO through pthread_setschedparam */
    REALTIMEMECHANISM_MSWIN_MMCSS       = 2,    /**< MMCSS task "Pro Audio" */
    REALTIMEMECHANISM_MSWIN_PRIORITY    = 3     /**< THREAD_PRIORITY_TIME_CRITICAL */
};

#ifdef GUCEF_MT_REALTIME_SCHEDULING_MSWIN

typedef HANDLE ( WINAPI* TAvSetMmThreadCharacteristicsW )( LPCWSTR taskName, LPDWORD taskIndex );
typedef BOOL ( WINAPI* TAvRevertMmThreadCharacteristics )( HANDLE avrtHandle );

#endif /* GUCEF_MT_REALTIME_SCHEDULING_MSWIN ? */

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRealtimeSchedulingScope::CRealtimeSchedulingScope( UInt32 priority )
    : m_mechanism( REALTIMEMECHANISM_NONE )
    , m_errorCode( 0 )
    , m_previousPolicy( 0 )
    , m_previousPriority( 0 )
    , m_handle( GUCEF_NULL )
    , m_module( GUCEF_NULL )
{
    #if defined( GUCEF_MT_REALTIME_SCHEDULING_POSIX )

    const pthread_t self = ::pthread_self();
    int policy = 0;
    struct sched_param previous;
    ::memset( &previous, 0, sizeof( previous ) );
    int result = ::pthread_getschedparam( self, &policy, &previous );
    if ( 0 != result )
    {
        m_errorCode = (Int32) result;
        return;
    }

    // the priority is limited to what the system has for the policy (1..99 on Linux)
    const int minPriority = ::sched_get_priority_min( SCHED_FIFO );
    const int maxPriority = ::sched_get_priority_max( SCHED_FIFO );
    int wanted = ( priority > (UInt32) maxPriority ) ? maxPriority : (int) priority;
    if ( wanted < minPriority )
        wanted = minPriority;

    struct sched_param realtime;
    ::memset( &realtime, 0, sizeof( realtime ) );
    realtime.sched_priority = wanted;
    result = ::pthread_setschedparam( self, SCHED_FIFO, &realtime );
    if ( 0 != result )
    {
        // EPERM without RLIMIT_RTPRIO or CAP_SYS_NICE: the thread keeps what it had
        m_errorCode = (Int32) result;
        return;
    }

    m_previousPolicy = (Int32) policy;
    m_previousPriority = (Int32) previous.sched_priority;
    m_mechanism = REALTIMEMECHANISM_POSIX_FIFO;

    #elif defined( GUCEF_MT_REALTIME_SCHEDULING_MSWIN )

    (void) priority;     /* not used on this platform */

    // MMCSS first: it is meant for this and also works for users who may not raise thread priorities themselves
    HMODULE avrt = ::LoadLibraryW( L"avrt.dll" );
    if ( GUCEF_NULL != avrt )
    {
        TAvSetMmThreadCharacteristicsW setCharacteristics = (TAvSetMmThreadCharacteristicsW) ::GetProcAddress( avrt, "AvSetMmThreadCharacteristicsW" );
        TAvRevertMmThreadCharacteristics revertCharacteristics = (TAvRevertMmThreadCharacteristics) ::GetProcAddress( avrt, "AvRevertMmThreadCharacteristics" );
        if ( GUCEF_NULL != setCharacteristics && GUCEF_NULL != revertCharacteristics )
        {
            DWORD taskIndex = 0;
            HANDLE task = setCharacteristics( L"Pro Audio", &taskIndex );
            if ( GUCEF_NULL != task )
            {
                m_handle = task;
                m_module = avrt;
                m_mechanism = REALTIMEMECHANISM_MSWIN_MMCSS;
                return;
            }
        }
        ::FreeLibrary( avrt );
    }

    const int previous = ::GetThreadPriority( ::GetCurrentThread() );
    if ( THREAD_PRIORITY_ERROR_RETURN == previous )
    {
        m_errorCode = (Int32) ::GetLastError();
        return;
    }
    if ( 0 == ::SetThreadPriority( ::GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL ) )
    {
        m_errorCode = (Int32) ::GetLastError();
        return;
    }
    m_previousPriority = (Int32) previous;
    m_mechanism = REALTIMEMECHANISM_MSWIN_PRIORITY;

    #else

    (void) priority;     /* not used on this platform */

    #endif
}

/*-------------------------------------------------------------------------*/

CRealtimeSchedulingScope::~CRealtimeSchedulingScope()
{
    #if defined( GUCEF_MT_REALTIME_SCHEDULING_POSIX )

    if ( REALTIMEMECHANISM_POSIX_FIFO == m_mechanism )
    {
        struct sched_param previous;
        ::memset( &previous, 0, sizeof( previous ) );
        previous.sched_priority = (int) m_previousPriority;
        ::pthread_setschedparam( ::pthread_self(), (int) m_previousPolicy, &previous );
    }

    #elif defined( GUCEF_MT_REALTIME_SCHEDULING_MSWIN )

    if ( REALTIMEMECHANISM_MSWIN_MMCSS == m_mechanism )
    {
        HMODULE avrt = (HMODULE) m_module;
        TAvRevertMmThreadCharacteristics revertCharacteristics = (TAvRevertMmThreadCharacteristics) ::GetProcAddress( avrt, "AvRevertMmThreadCharacteristics" );
        if ( GUCEF_NULL != revertCharacteristics )
            revertCharacteristics( (HANDLE) m_handle );
        ::FreeLibrary( avrt );
    }
    else
    if ( REALTIMEMECHANISM_MSWIN_PRIORITY == m_mechanism )
    {
        ::SetThreadPriority( ::GetCurrentThread(), (int) m_previousPriority );
    }

    #endif

    m_mechanism = REALTIMEMECHANISM_NONE;
    m_handle = GUCEF_NULL;
    m_module = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CRealtimeSchedulingScope::IsActive( void ) const
{
    return REALTIMEMECHANISM_NONE != m_mechanism;
}

/*-------------------------------------------------------------------------*/

Int32
CRealtimeSchedulingScope::GetErrorCode( void ) const
{
    return m_errorCode;
}

/*-------------------------------------------------------------------------*/

bool
CRealtimeSchedulingScope::IsSupported( void )
{
    #if defined( GUCEF_MT_REALTIME_SCHEDULING_POSIX ) || defined( GUCEF_MT_REALTIME_SCHEDULING_MSWIN )
    return true;
    #else
    return false;
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
