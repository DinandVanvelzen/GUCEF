/*
 *  pluginglueOGRE_TestApp: Test application for the Ogre glue modules (pluginglueOGRE, OgrePlugin_GucefVFS, OgreCodec_GucefIMAGE)
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <chrono>
#include <new>
#include <thread>

#if ( defined( __linux__ ) )
#include <dirent.h>
#endif

#include "OgreRoot.h"
#include "OgreDefaultWorkQueue.h"
#include "OgreLogManager.h"

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_MT_CSEMAPHORE_H
#include "gucefMT_CSemaphore.h"
#define GUCEF_MT_CSEMAPHORE_H
#endif /* GUCEF_MT_CSEMAPHORE_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_H
#include "pluginglueOGRE.h"
#define GUCEF_PLUGINGLUE_OGRE_H
#endif /* GUCEF_PLUGINGLUE_OGRE_H ? */

#include "TestWorkQueueBenchmark.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using PLUGINGLUE::OGRE::COgreTaskWorkQueue;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

#define SKIP_UNLESS_BENCH() \
    GUCEF_TESTFW_SKIP_IF( GUCEF_TESTFW_GETENV( "GUCEF_TEST_OGRE_BENCH" ) != "1", "GUCEF_TEST_OGRE_BENCH is not 1" )

/*-------------------------------------------------------------------------//
//                                                                         //
//      ALLOCATION COUNTING                                                //
//                                                                         //
//-------------------------------------------------------------------------*/

#if !defined( __SANITIZE_ADDRESS__ ) && !defined( __SANITIZE_THREAD__ ) && ( defined( __linux__ ) )
  #define GUCEF_OGRE_BENCH_COUNTS_ALLOCATIONS 1
#else
  #define GUCEF_OGRE_BENCH_COUNTS_ALLOCATIONS 0
#endif

static MT::CTAtomic< UInt64 > g_allocations( 0 );

#if ( 1 == GUCEF_OGRE_BENCH_COUNTS_ALLOCATIONS )

// The allocations of the whole process go through these, so the benchmark can count them per task
void* operator new( std::size_t size )
{
    g_allocations.FetchAdd( 1, MT::MEMORDER_RELAXED );
    void* p = ::malloc( 0 == size ? 1 : size );
    if ( GUCEF_NULL == p )
        throw std::bad_alloc();
    return p;
}

void* operator new[]( std::size_t size )
{
    g_allocations.FetchAdd( 1, MT::MEMORDER_RELAXED );
    void* p = ::malloc( 0 == size ? 1 : size );
    if ( GUCEF_NULL == p )
        throw std::bad_alloc();
    return p;
}

void* operator new( std::size_t size, const std::nothrow_t& ) GUCEF_NOEXCEPT
{
    g_allocations.FetchAdd( 1, MT::MEMORDER_RELAXED );
    return ::malloc( 0 == size ? 1 : size );
}

void* operator new[]( std::size_t size, const std::nothrow_t& ) GUCEF_NOEXCEPT
{
    g_allocations.FetchAdd( 1, MT::MEMORDER_RELAXED );
    return ::malloc( 0 == size ? 1 : size );
}

void operator delete( void* p ) GUCEF_NOEXCEPT { ::free( p ); }
void operator delete[]( void* p ) GUCEF_NOEXCEPT { ::free( p ); }
void operator delete( void* p, std::size_t ) GUCEF_NOEXCEPT { ::free( p ); }
void operator delete[]( void* p, std::size_t ) GUCEF_NOEXCEPT { ::free( p ); }
void operator delete( void* p, const std::nothrow_t& ) GUCEF_NOEXCEPT { ::free( p ); }
void operator delete[]( void* p, const std::nothrow_t& ) GUCEF_NOEXCEPT { ::free( p ); }

#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const UInt32 WorkerCount = 2;

typedef GUCEF::vector< UInt64 > TNsVector;

/*-------------------------------------------------------------------------*/

UInt64
NowNs( void )
{
    return (UInt64) std::chrono::duration_cast< std::chrono::nanoseconds >( std::chrono::steady_clock::now().time_since_epoch() ).count();
}

/*-------------------------------------------------------------------------*/

/** What the tasks of a run share: how many are done, when the last is, and when the last one started */
class CBenchState
{
    public:

    CBenchState( void ) : done( 0 ), target( 0 ), lastStartNs( 0 ), finished( 0 ) {}

    void Reset( UInt64 count )
    {
        done.Store( 0 );
        target.Store( count );
        lastStartNs.Store( 0 );
        while ( 0 == finished.TryWait() ) {}
    }

    bool WaitFinished( void ) { return 0 == finished.WaitOrTimeout( 120000 ); }

    MT::CTAtomic< UInt64 > done;
    MT::CTAtomic< UInt64 > target;
    MT::CTAtomic< UInt64 > lastStartNs;
    MT::CSemaphore finished;
};

/*-------------------------------------------------------------------------*/

/** A task that works for the given time (busy, as work does) and counts itself done */
class CBenchTask
{
    public:

    CBenchTask( CBenchState& state, UInt64 workNs ) : m_state( &state ), m_workNs( workNs ) {}

    void operator()( void )
    {
        const UInt64 start = NowNs();
        m_state->lastStartNs.Store( start );
        if ( 0 != m_workNs )
            while ( NowNs() - start < m_workNs ) {}
        if ( m_state->done.FetchAdd( 1 ) + 1 == m_state->target.Load() )
            m_state->finished.Post();
    }

    private:

    CBenchState* m_state;
    UInt64 m_workNs;
};

/*-------------------------------------------------------------------------*/

/** A task that does nothing at all */
class CEmptyTask
{
    public:

    void operator()( void ) {}
};

/*-------------------------------------------------------------------------*/

Ogre::WorkQueue*
MakeQueue( bool gucef )
{GUCEF_TRACE;

    Ogre::WorkQueue* queue = GUCEF_NULL;
    if ( gucef )
    {
        COgreTaskWorkQueue* taskQueue = OGRE_NEW COgreTaskWorkQueue();
        taskQueue->SetThreadPoolName( "OgreWorkBench" );
        queue = taskQueue;
    }
    else
    {
        queue = OGRE_NEW Ogre::DefaultWorkQueue( "Bench" );
    }
    queue->setWorkerThreadCount( WorkerCount );
    queue->setResponseProcessingTimeLimit( 0 );
    queue->startup();
    return queue;
}

/*-------------------------------------------------------------------------*/

const char*
QueueName( bool gucef )
{
    return gucef ? "COgreTaskWorkQueue" : "Ogre DefaultWorkQueue";
}

/*-------------------------------------------------------------------------*/

UInt64
Percentile( TNsVector values, UInt32 percent )
{
    if ( values.empty() )
        return 0;
    std::sort( values.begin(), values.end() );
    size_t index = ( values.size() - 1 ) * percent / 100;
    return values[ index ];
}

/*-------------------------------------------------------------------------*/

/** The processor time the thread used so far, in nanoseconds; 0 when the platform does not say */
UInt64
ThreadCpuTimeInNs( UInt64 threadId )
{
    #if ( defined( __linux__ ) )
    char path[ 64 ];
    snprintf( path, sizeof( path ), "/proc/self/task/%llu/schedstat", (unsigned long long) threadId );
    FILE* file = fopen( path, "r" );
    if ( GUCEF_NULL == file )
        return 0;
    unsigned long long ns = 0;
    if ( 1 != fscanf( file, "%llu", &ns ) )
        ns = 0;
    fclose( file );
    return (UInt64) ns;
    #else
    return 0;
    #endif
}

/*-------------------------------------------------------------------------*/

/** The ids of the threads of the process whose name starts with the given text */
GUCEF::vector< UInt64 >
ThreadsNamed( const char* namePrefix )
{
    GUCEF::vector< UInt64 > threads;
    #if ( defined( __linux__ ) )
    DIR* tasks = ::opendir( "/proc/self/task" );
    if ( GUCEF_NULL == tasks )
        return threads;
    const size_t prefixLength = strlen( namePrefix );
    struct dirent* entry = GUCEF_NULL;
    while ( GUCEF_NULL != ( entry = ::readdir( tasks ) ) )
    {
        if ( '.' == entry->d_name[ 0 ] )
            continue;
        char path[ 300 ];
        snprintf( path, sizeof( path ), "/proc/self/task/%s/comm", entry->d_name );
        FILE* file = fopen( path, "r" );
        if ( GUCEF_NULL == file )
            continue;
        char name[ 64 ];
        memset( name, 0, sizeof( name ) );
        if ( GUCEF_NULL != fgets( name, sizeof( name ), file ) && 0 == strncmp( name, namePrefix, prefixLength ) )
            threads.push_back( (UInt64) strtoull( entry->d_name, GUCEF_NULL, 10 ) );
        fclose( file );
    }
    ::closedir( tasks );
    #endif
    return threads;
}

/*-------------------------------------------------------------------------*/

/** Adds its share of tasks to the queue once it is let go */
void
ProduceTasks( Ogre::WorkQueue* queue, CBenchState* state, UInt32 count, MT::CTAtomic< UInt32 >* go )
{
    while ( 0 == go->Load() ) {}
    for ( UInt32 i=0; i<count; ++i )
        queue->addTask( CBenchTask( *state, 0 ) );
}

/*-------------------------------------------------------------------------*/

Float64
NsPer( UInt64 ns, UInt64 count )
{
    return 0 == count ? 0.0 : (Float64) ns / (Float64) count;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
BenchThroughput( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "WorkQueueBenchmark: throughput and overhead per task, empty tasks and tasks of 10 us, 100 us and 1 ms, 2 worker threads (report only, opt in)" )
    SKIP_UNLESS_BENCH();
    try
    {
        const UInt64 workNs[ 4 ] = { 0, 10000, 100000, 1000000 };
        const UInt64 counts[ 4 ] = { 100000, 100000, 20000, 2000 };
        for ( UInt32 q=0; q<2; ++q )
        {
            const bool gucef = 1 == q;
            Ogre::WorkQueue* queue = MakeQueue( gucef );
            CBenchState state;
            // warm up: the threads run, the lists have their blocks
            state.Reset( 1000 );
            for ( UInt32 i=0; i<1000; ++i )
                queue->addTask( CBenchTask( state, 0 ) );
            ASSERT_TRUE( state.WaitFinished() );

            for ( UInt32 w=0; w<4; ++w )
            {
                state.Reset( counts[ w ] );
                const UInt64 start = NowNs();
                for ( UInt64 i=0; i<counts[ w ]; ++i )
                    queue->addTask( CBenchTask( state, workNs[ w ] ) );
                const UInt64 added = NowNs();
                ASSERT_TRUE( state.WaitFinished() );
                const UInt64 elapsed = NowNs() - start;
                // the work of the tasks spread over the workers; what is left is the cost of the queue per task
                const Float64 wallPerTask = NsPer( elapsed, counts[ w ] );
                const Float64 overheadPerTask = wallPerTask * WorkerCount - (Float64) workNs[ w ];
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "OgreWorkQueueBench|throughput|" << QueueName( gucef ) << "|work_ns=" << workNs[ w ]
                    << "|tasks=" << counts[ w ] << "|wall_ns_per_task=" << wallPerTask << "|add_ns_per_task=" << NsPer( added - start, counts[ w ] )
                    << "|overhead_ns_per_task=" << overheadPerTask << "|tasks_per_s=" << ( 1.0e9 / wallPerTask ) );
            }
            OGRE_DELETE queue;
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
BenchLatency( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "WorkQueueBenchmark: time from addTask() to the start of the task, with a busy and with an idle queue (report only, opt in)" )
    SKIP_UNLESS_BENCH();
    try
    {
        for ( UInt32 q=0; q<2; ++q )
        {
            const bool gucef = 1 == q;
            Ogre::WorkQueue* queue = MakeQueue( gucef );
            CBenchState state;
            for ( UInt32 mode=0; mode<2; ++mode )
            {
                // mode 0: one after the other; mode 1: after 20 ms of nothing, the workers are asleep
                const UInt32 samples = 0 == mode ? 2000 : 200;
                TNsVector latencies;
                for ( UInt32 i=0; i<samples; ++i )
                {
                    if ( 1 == mode )
                        MT::PrecisionDelay( 20 );
                    state.Reset( 1 );
                    const UInt64 added = NowNs();
                    queue->addTask( CBenchTask( state, 0 ) );
                    ASSERT_TRUE( state.WaitFinished() );
                    latencies.push_back( state.lastStartNs.Load() - added );
                }
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "OgreWorkQueueBench|latency|" << QueueName( gucef ) << "|" << ( 0 == mode ? "back_to_back" : "idle_wakeup" )
                    << "|samples=" << samples << "|median_ns=" << Percentile( latencies, 50 ) << "|p99_ns=" << Percentile( latencies, 99 ) );
            }
            OGRE_DELETE queue;
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
BenchMainThreadTasks( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "WorkQueueBenchmark: processMainThreadTasks() per call, empty and with 1, 100 and 10000 queued main thread tasks (report only, opt in)" )
    SKIP_UNLESS_BENCH();
    try
    {
        for ( UInt32 q=0; q<2; ++q )
        {
            const bool gucef = 1 == q;
            Ogre::WorkQueue* queue = MakeQueue( gucef );

            const UInt32 emptyCalls = 1000000;
            UInt64 start = NowNs();
            for ( UInt32 i=0; i<emptyCalls; ++i )
                queue->processMainThreadTasks();
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "OgreWorkQueueBench|main_thread|" << QueueName( gucef ) << "|queued=0|ns_per_call="
                << NsPer( NowNs() - start, emptyCalls ) );

            const UInt32 queued[ 3 ] = { 1, 100, 10000 };
            const UInt32 rounds[ 3 ] = { 10000, 1000, 20 };
            for ( UInt32 k=0; k<3; ++k )
            {
                UInt64 total = 0;
                for ( UInt32 r=0; r<rounds[ k ]; ++r )
                {
                    for ( UInt32 i=0; i<queued[ k ]; ++i )
                        queue->addMainThreadTask( CEmptyTask() );
                    start = NowNs();
                    queue->processMainThreadTasks();
                    total += NowNs() - start;
                }
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "OgreWorkQueueBench|main_thread|" << QueueName( gucef ) << "|queued=" << queued[ k ]
                    << "|ns_per_call=" << NsPer( total, rounds[ k ] ) << "|ns_per_task=" << NsPer( total, (UInt64) rounds[ k ] * queued[ k ] ) );
            }
            OGRE_DELETE queue;
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
BenchConcurrentAdd( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "WorkQueueBenchmark: addTask() from four threads at once, 25000 empty tasks each (report only, opt in)" )
    SKIP_UNLESS_BENCH();
    try
    {
        for ( UInt32 q=0; q<2; ++q )
        {
            const bool gucef = 1 == q;
            Ogre::WorkQueue* queue = MakeQueue( gucef );
            CBenchState state;
            const UInt32 perThread = 25000;
            state.Reset( 4 * perThread );
            MT::CTAtomic< UInt32 > go( 0 );
            std::thread producers[ 4 ];
            for ( UInt32 t=0; t<4; ++t )
                producers[ t ] = std::thread( ProduceTasks, queue, &state, perThread, &go );
            MT::PrecisionDelay( 20 );
            const UInt64 start = NowNs();
            go.Store( 1 );
            for ( UInt32 t=0; t<4; ++t )
                producers[ t ].join();
            const UInt64 added = NowNs();
            ASSERT_TRUE( state.WaitFinished() );
            const UInt64 finished = NowNs();
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "OgreWorkQueueBench|concurrent_add|" << QueueName( gucef ) << "|tasks=" << ( 4 * perThread )
                << "|add_wall_ms=" << ( ( added - start ) / 1000000.0 ) << "|add_ns_per_task=" << NsPer( ( added - start ) * 4, 4 * perThread )
                << "|done_wall_ms=" << ( ( finished - start ) / 1000000.0 ) );
            OGRE_DELETE queue;
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
BenchIdle( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "WorkQueueBenchmark: processor time of the idle worker threads over 10 s (report only, opt in)" )
    SKIP_UNLESS_BENCH();
    #if ( !defined( __linux__ ) )
    GUCEF_TESTFW_SKIP( "the processor time of a thread is read from /proc" );
    #endif
    try
    {
        for ( UInt32 q=0; q<2; ++q )
        {
            const bool gucef = 1 == q;
            Ogre::WorkQueue* queue = MakeQueue( gucef );
            CBenchState state;
            state.Reset( 10 );
            for ( UInt32 i=0; i<10; ++i )
                queue->addTask( CBenchTask( state, 0 ) );
            ASSERT_TRUE( state.WaitFinished() );

            GUCEF::vector< UInt64 > threads;
            if ( gucef )
            {
                CORE::CThreadPool::TThreadIdVector poolThreads;
                static_cast< COgreTaskWorkQueue* >( queue )->GetThreadPool()->GetAllCurrentThreadIds( poolThreads );
                for ( size_t i=0; i<poolThreads.size(); ++i )
                    threads.push_back( (UInt64) poolThreads[ i ] );
            }
            else
            {
                threads = ThreadsNamed( "[Ogre] Worker" );
            }
            ASSERT_TRUE( threads.size() >= WorkerCount );

            MT::PrecisionDelay( 200 );
            GUCEF::vector< UInt64 > before;
            for ( size_t i=0; i<threads.size(); ++i )
                before.push_back( ThreadCpuTimeInNs( threads[ i ] ) );
            MT::PrecisionDelay( 10000 );
            UInt64 total = 0;
            for ( size_t i=0; i<threads.size(); ++i )
                total += ThreadCpuTimeInNs( threads[ i ] ) - before[ i ];
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "OgreWorkQueueBench|idle|" << QueueName( gucef ) << "|threads=" << threads.size()
                << "|cpu_us_in_10s_all_workers=" << ( total / 1000 ) );
            OGRE_DELETE queue;
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
BenchAllocations( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "WorkQueueBenchmark: memory allocations per task, background and main thread tasks (report only, opt in)" )
    SKIP_UNLESS_BENCH();
    if ( 1 != GUCEF_OGRE_BENCH_COUNTS_ALLOCATIONS )
        GUCEF_TESTFW_SKIP( "allocations are counted only in builds without sanitizers on Linux" );
    try
    {
        for ( UInt32 q=0; q<2; ++q )
        {
            const bool gucef = 1 == q;
            Ogre::WorkQueue* queue = MakeQueue( gucef );
            CBenchState state;
            state.Reset( 1000 );
            for ( UInt32 i=0; i<1000; ++i )
                queue->addTask( CBenchTask( state, 0 ) );
            ASSERT_TRUE( state.WaitFinished() );

            const UInt32 count = 10000;
            state.Reset( count );
            UInt64 before = g_allocations.Load();
            for ( UInt32 i=0; i<count; ++i )
                queue->addTask( CBenchTask( state, 0 ) );
            ASSERT_TRUE( state.WaitFinished() );
            MT::PrecisionDelay( 50 );
            const UInt64 background = g_allocations.Load() - before;

            before = g_allocations.Load();
            for ( UInt32 i=0; i<count; ++i )
                queue->addMainThreadTask( CEmptyTask() );
            queue->processMainThreadTasks();
            const UInt64 mainThread = g_allocations.Load() - before;

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "OgreWorkQueueBench|allocations|" << QueueName( gucef ) << "|per_background_task="
                << ( (Float64) background / count ) << "|per_main_thread_task=" << ( (Float64) mainThread / count ) );
            OGRE_DELETE queue;
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformWorkQueueBenchmarkTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "pluginglueOGRE.WorkQueueBenchmark" );

    // Ogre's DefaultWorkQueue writes a trivial log message per task; COgreLogAdapter passes those to the GUCEF log, so
    // with it Ogre's queue pays for the GUCEF log. The comparison is made at Ogre's default level, where they are not
    // logged (but still formatted); one run at the level of the adapter shows that cost
    Ogre::Log* log = GUCEF_NULL != Ogre::LogManager::getSingletonPtr() ? Ogre::LogManager::getSingleton().getDefaultLog() : GUCEF_NULL;
    if ( GUCEF_NULL != log && "1" == GUCEF_TESTFW_GETENV( "GUCEF_TEST_OGRE_BENCH" ) )
    {
        const UInt64 count = 20000;
        Ogre::WorkQueue* queue = MakeQueue( false );
        CBenchState state;
        state.Reset( count );
        const UInt64 start = NowNs();
        for ( UInt64 i=0; i<count; ++i )
            queue->addTask( CBenchTask( state, 0 ) );
        state.WaitFinished();
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "OgreWorkQueueBench|throughput_with_trivial_log|" << QueueName( false ) << "|work_ns=0|tasks=" << count
            << "|wall_ns_per_task=" << NsPer( NowNs() - start, count ) );
        OGRE_DELETE queue;
        log->setMinLogLevel( Ogre::LML_NORMAL );
    }

    BenchThroughput();
    BenchLatency();
    BenchMainThreadTasks();
    BenchConcurrentAdd();
    BenchIdle();
    BenchAllocations();

    if ( GUCEF_NULL != log )
        log->setMinLogLevel( Ogre::LML_TRIVIAL );
}

/*-------------------------------------------------------------------------*/
