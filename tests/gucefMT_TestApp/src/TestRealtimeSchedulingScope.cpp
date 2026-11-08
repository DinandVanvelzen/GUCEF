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

#ifndef GUCEF_MT_CREALTIMESCHEDULINGSCOPE_H
#include "gucefMT_CRealtimeSchedulingScope.h"
#define GUCEF_MT_CREALTIMESCHEDULINGSCOPE_H
#endif /* GUCEF_MT_CREALTIMESCHEDULINGSCOPE_H ? */

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

/* the system headers follow the platform, which the headers above define */
#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #include <windows.h>
  #define GUCEF_MT_TEST_REALTIME_MSWIN
#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
  #include <errno.h>
  #include <pthread.h>
  #include <sched.h>
  #include <sys/resource.h>
  #define GUCEF_MT_TEST_REALTIME_POSIX
#endif

#include "TestRealtimeSchedulingScope.h"

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

/** The scheduling of the calling thread as the system reports it */
struct SScheduling
{
    Int32 policy;       /**< POSIX scheduling policy, 0 on Windows */
    Int32 priority;     /**< POSIX static priority, the thread priority on Windows */

    SScheduling( void ) : policy( 0 ), priority( 0 ) {}

    bool operator==( const SScheduling& other ) const { return policy == other.policy && priority == other.priority; }
};

SScheduling
ReadScheduling( void )
{
    SScheduling scheduling;

    #if defined( GUCEF_MT_TEST_REALTIME_POSIX )
    int policy = 0;
    struct sched_param param;
    param.sched_priority = 0;
    if ( 0 == ::pthread_getschedparam( ::pthread_self(), &policy, &param ) )
    {
        scheduling.policy = (Int32) policy;
        scheduling.priority = (Int32) param.sched_priority;
    }
    #elif defined( GUCEF_MT_TEST_REALTIME_MSWIN )
    scheduling.priority = (Int32) ::GetThreadPriority( ::GetCurrentThread() );
    #endif

    return scheduling;
}

/** True when the scheduling is real-time at the priority a scope asked for, as far as the platform can tell */
bool
IsRealtimeAt( const SScheduling& scheduling, UInt32 askedPriority )
{
    #if defined( GUCEF_MT_TEST_REALTIME_POSIX )
    const int maxPriority = ::sched_get_priority_max( SCHED_FIFO );
    const int minPriority = ::sched_get_priority_min( SCHED_FIFO );
    int expected = ( askedPriority > (UInt32) maxPriority ) ? maxPriority : (int) askedPriority;
    if ( expected < minPriority )
        expected = minPriority;
    return SCHED_FIFO == scheduling.policy && expected == scheduling.priority;
    #else
    // MMCSS manages the priority itself, the thread priority need not show it
    return true;
    #endif
}

/** Holds a scope on its own thread and notes what that thread saw */
class CScopeThread : public MT::CActiveObject
{
    public:

    CScopeThread( void )
        : MT::CActiveObject()
        , active( false )
        , restored( false )
        , realtimeInside( false )
        , inScope( false )
        , release( false )
        , done( false )
    {}

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        const SScheduling before = ReadScheduling();
        {
            MT::CRealtimeSchedulingScope scope;
            active.Store( scope.IsActive() );
            realtimeInside.Store( scope.IsActive() && IsRealtimeAt( ReadScheduling(), MT::CRealtimeSchedulingScope::DefaultPriority ) );
            inScope.Store( true );
            UInt32 waited = 0;
            while ( !release.Load() && waited < 10000 )
            {
                MT::PrecisionDelay( 1 );
                ++waited;
            }
        }
        restored.Store( before == ReadScheduling() );
        done.Store( true );
        return true;
    }

    MT::CTAtomic< bool > active;
    MT::CTAtomic< bool > restored;
    MT::CTAtomic< bool > realtimeInside;
    MT::CTAtomic< bool > inScope;
    MT::CTAtomic< bool > release;
    MT::CTAtomic< bool > done;
};

bool
WaitFor( const MT::CTAtomic< bool >& flag, UInt32 timeoutInMs )
{
    UInt32 waited = 0;
    while ( !flag.Load() && waited < timeoutInMs )
    {
        MT::PrecisionDelay( 1 );
        ++waited;
    }
    return flag.Load();
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformRealtimeSchedulingScopeTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING RealtimeSchedulingScope TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefMT.RealtimeSchedulingScope" );

    GUCEF_TESTFW_TESTCASE( "RealtimeSchedulingScope: The scope makes the thread real-time or changes nothing, and restores the thread when it ends" )
    try
    {
        const SScheduling before = ReadScheduling();
        bool wasActive = false;
        Int32 errorCode = 0;
        {
            MT::CRealtimeSchedulingScope scope;
            wasActive = scope.IsActive();
            errorCode = scope.GetErrorCode();
            const SScheduling inside = ReadScheduling();
            if ( wasActive )
            {
                ASSERT_TRUE( 0 == errorCode );
                ASSERT_TRUE( IsRealtimeAt( inside, MT::CRealtimeSchedulingScope::DefaultPriority ) );
            }
            else
            {
                // refused or not supported: nothing changed, and a refusal says why
                ASSERT_TRUE( before == inside );
                ASSERT_TRUE( MT::CRealtimeSchedulingScope::IsSupported() == ( 0 != errorCode ) );
            }
        }
        ASSERT_TRUE( before == ReadScheduling() );

        if ( wasActive )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RealtimeSchedulingScope: real-time scheduling was granted, the active case was tested" );
        }
        else
        if ( MT::CRealtimeSchedulingScope::IsSupported() )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RealtimeSchedulingScope: real-time scheduling was refused (error " << errorCode << "), the refused case was tested; grant rtprio to test the active case" );
        }
        else
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RealtimeSchedulingScope: this platform has no real-time scheduling, the inactive case was tested" );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RealtimeSchedulingScope: Without the right to real-time scheduling the scope is inactive, reports EPERM and changes nothing" )
    #if defined( GUCEF_MT_TEST_REALTIME_POSIX )
    try
    {
        // RLIMIT_RTPRIO 0 takes the right away from a process without CAP_SYS_NICE; lowering the soft limit needs no right
        // and the old soft limit can be restored since the hard limit stays
        struct rlimit original;
        ASSERT_TRUE( 0 == ::getrlimit( RLIMIT_RTPRIO, &original ) );
        struct rlimit none = original;
        none.rlim_cur = 0;
        ASSERT_TRUE( 0 == ::setrlimit( RLIMIT_RTPRIO, &none ) );

        const SScheduling before = ReadScheduling();
        bool wasActive = false;
        {
            MT::CRealtimeSchedulingScope scope( 50 );
            wasActive = scope.IsActive();
            if ( !wasActive )
            {
                ASSERT_TRUE( EPERM == scope.GetErrorCode() );
                ASSERT_TRUE( before == ReadScheduling() );
            }
        }
        ASSERT_TRUE( before == ReadScheduling() );
        ASSERT_TRUE( 0 == ::setrlimit( RLIMIT_RTPRIO, &original ) );

        if ( wasActive )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RealtimeSchedulingScope: the process has CAP_SYS_NICE, a refusal cannot be provoked; restoring was tested" );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    #else
    GUCEF_TESTFW_SKIP( "the refusal is provoked through RLIMIT_RTPRIO, which only Linux and Android have" );
    #endif
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RealtimeSchedulingScope: Nested scopes restore in turn and the priority is limited to the range of the system" )
    try
    {
        const SScheduling before = ReadScheduling();
        {
            MT::CRealtimeSchedulingScope outer( 10 );
            const SScheduling inOuter = ReadScheduling();
            if ( outer.IsActive() )
                ASSERT_TRUE( IsRealtimeAt( inOuter, 10 ) );
            {
                // far above the maximum: the maximum (99 on Linux)
                MT::CRealtimeSchedulingScope inner( 100000 );
                ASSERT_TRUE( inner.IsActive() == outer.IsActive() );
                if ( inner.IsActive() )
                    ASSERT_TRUE( IsRealtimeAt( ReadScheduling(), 100000 ) );
                else
                    ASSERT_TRUE( before == ReadScheduling() );
            }
            ASSERT_TRUE( inOuter == ReadScheduling() );
            {
                // 0 is below the minimum of SCHED_FIFO: the minimum (1 on Linux)
                MT::CRealtimeSchedulingScope lowest( 0 );
                if ( lowest.IsActive() )
                    ASSERT_TRUE( IsRealtimeAt( ReadScheduling(), 0 ) );
            }
            ASSERT_TRUE( inOuter == ReadScheduling() );
        }
        ASSERT_TRUE( before == ReadScheduling() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RealtimeSchedulingScope: A scope changes only the thread that made it" )
    try
    {
        const SScheduling before = ReadScheduling();
        CScopeThread thread;
        ASSERT_TRUE( thread.Activate( GUCEF_NULL, 0, 0 ) );
        ASSERT_TRUE( WaitFor( thread.inScope, 10000 ) );

        // while the other thread holds its scope this one is as it was
        ASSERT_TRUE( before == ReadScheduling() );
        if ( thread.active.Load() )
            ASSERT_TRUE( thread.realtimeInside.Load() );

        thread.release.Store( true );
        ASSERT_TRUE( WaitFor( thread.done, 10000 ) );
        ASSERT_TRUE( thread.restored.Load() );
        thread.Deactivate( false, true );
        ASSERT_TRUE( before == ReadScheduling() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
