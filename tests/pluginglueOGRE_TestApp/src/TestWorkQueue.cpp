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
#include <exception>
#include <future>
#include <memory>
#include <stdexcept>

#include "OgreRoot.h"
#include "OgreDefaultWorkQueue.h"
#include "OgreException.h"

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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_CSEMAPHORE_H
#include "gucefMT_CSemaphore.h"
#define GUCEF_MT_CSEMAPHORE_H
#endif /* GUCEF_MT_CSEMAPHORE_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_H
#include "pluginglueOGRE.h"
#define GUCEF_PLUGINGLUE_OGRE_H
#endif /* GUCEF_PLUGINGLUE_OGRE_H ? */

#include "TestWorkQueue.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using PLUGINGLUE::OGRE::COgreTaskWorkQueue;
using PLUGINGLUE::OGRE::COgreWorkQueueCorePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** What the tasks of a test did, in the order they did it, and on which threads */
class CTaskLog
{
    public:

    CTaskLog( void ) : m_lock(), m_entries(), m_threads(), m_systemThreads() {}

    void Record( Int32 entry )
    {
        MT::CScopeMutex lock( m_lock );
        m_entries.push_back( entry );
        m_threads.push_back( MT::GetCurrentThreadID() );
        m_systemThreads.push_back( MT::GetCurrentThreadSystemID() );
    }

    UInt32 Count( void ) const
    {
        MT::CScopeMutex lock( m_lock );
        return (UInt32) m_entries.size();
    }

    GUCEF::vector< Int32 > Entries( void ) const
    {
        MT::CScopeMutex lock( m_lock );
        return m_entries;
    }

    GUCEF::vector< TThreadId > Threads( void ) const
    {
        MT::CScopeMutex lock( m_lock );
        return m_threads;
    }

    GUCEF::vector< TSystemThreadId > SystemThreads( void ) const
    {
        MT::CScopeMutex lock( m_lock );
        return m_systemThreads;
    }

    /** Waits until the given number of entries is there, at most the given time */
    bool WaitForCount( UInt32 count, UInt32 timeoutInMs ) const
    {
        const UInt64 start = MT::PrecisionTickCount();
        while ( Count() < count )
        {
            if ( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) > timeoutInMs )
                return false;
            MT::PrecisionDelay( 2 );
        }
        return true;
    }

    private:

    mutable MT::CMutex m_lock;
    GUCEF::vector< Int32 > m_entries;
    GUCEF::vector< TThreadId > m_threads;
    GUCEF::vector< TSystemThreadId > m_systemThreads;
};

/*-------------------------------------------------------------------------*/

/** Records its number in a log, after an optional delay */
class CRecordTask
{
    public:

    CRecordTask( CTaskLog& log, Int32 entry, UInt32 delayInMs = 0 ) : m_log( &log ), m_entry( entry ), m_delayInMs( delayInMs ) {}

    void operator()( void )
    {
        if ( 0 != m_delayInMs )
            MT::PrecisionDelay( m_delayInMs );
        m_log->Record( m_entry );
    }

    private:

    CTaskLog* m_log;
    Int32 m_entry;
    UInt32 m_delayInMs;
};

/*-------------------------------------------------------------------------*/

/** Throws what it is told to: 0 an Ogre exception, 1 a standard one, 2 something else */
class CThrowTask
{
    public:

    explicit CThrowTask( Int32 kind ) : m_kind( kind ) {}

    void operator()( void )
    {
        if ( 0 == m_kind )
            OGRE_EXCEPT( Ogre::Exception::ERR_INTERNAL_ERROR, "a test task fails", "CThrowTask" );
        if ( 1 == m_kind )
            throw std::runtime_error( "a test task fails" );
        throw m_kind;
    }

    private:

    Int32 m_kind;
};

/*-------------------------------------------------------------------------*/

/** Says it started, then waits until it is let go or the wait is over */
class CGateTask
{
    public:

    CGateTask( MT::CSemaphore& started, MT::CSemaphore& release, CTaskLog& log, Int32 entry, UInt32 waitInMs = 10000 )
        : m_started( &started ), m_release( &release ), m_log( &log ), m_entry( entry ), m_waitInMs( waitInMs ) {}

    void operator()( void )
    {
        m_started->Post();
        m_release->WaitOrTimeout( m_waitInMs );
        m_log->Record( m_entry );
    }

    private:

    MT::CSemaphore* m_started;
    MT::CSemaphore* m_release;
    CTaskLog* m_log;
    Int32 m_entry;
    UInt32 m_waitInMs;
};

/*-------------------------------------------------------------------------*/

/** A background task that adds a main thread task to the queue */
class CAddMainThreadTask
{
    public:

    CAddMainThreadTask( Ogre::WorkQueue& queue, CTaskLog& log, Int32 entry ) : m_queue( &queue ), m_log( &log ), m_entry( entry ) {}

    void operator()( void )
    {
        m_log->Record( m_entry );
        m_queue->addMainThreadTask( CRecordTask( *m_log, m_entry + 1 ) );
    }

    private:

    Ogre::WorkQueue* m_queue;
    CTaskLog* m_log;
    Int32 m_entry;
};

/*-------------------------------------------------------------------------*/

/** Counts how many of its kind were destroyed: what a task holds goes with the task */
class CDropProbe
{
    public:

    explicit CDropProbe( MT::CMutex& lock, UInt32& destroyed ) : m_lock( &lock ), m_destroyed( &destroyed ) {}

    ~CDropProbe()
    {
        MT::CScopeMutex lock( *m_lock );
        ++( *m_destroyed );
    }

    private:

    MT::CMutex* m_lock;
    UInt32* m_destroyed;
};

/*-------------------------------------------------------------------------*/

/** A task that holds a probe */
class CProbeTask
{
    public:

    CProbeTask( const std::shared_ptr< CDropProbe >& probe, CTaskLog& log, Int32 entry ) : m_probe( probe ), m_log( &log ), m_entry( entry ) {}

    void operator()( void ) { m_log->Record( m_entry ); }

    private:

    std::shared_ptr< CDropProbe > m_probe;
    CTaskLog* m_log;
    Int32 m_entry;
};

/*-------------------------------------------------------------------------*/

/** A packaged task as Ogre's ResourceBackgroundQueue gives them to the queue */
class CPackagedTask
{
    public:

    explicit CPackagedTask( const std::shared_ptr< std::packaged_task< void() > >& task ) : m_task( task ) {}

    void operator()( void ) { (*m_task)(); }

    private:

    std::shared_ptr< std::packaged_task< void() > > m_task;
};

/*-------------------------------------------------------------------------*/

void
DoNothing( void )
{
}

/*-------------------------------------------------------------------------*/

bool
ContainsThread( const CORE::CThreadPool::TThreadIdVector& threads, TSystemThreadId threadId )
{GUCEF_TRACE;

    for ( size_t i=0; i<threads.size(); ++i )
        if ( threads[ i ] == threadId )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

/** The processor time the thread used so far, in nanoseconds; 0 when the platform does not say */
UInt64
ThreadCpuTimeInNs( TSystemThreadId threadId )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    char path[ 64 ];
    snprintf( path, sizeof( path ), "/proc/self/task/%u/schedstat", (unsigned int) threadId );
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

}

/*-------------------------------------------------------------------------*/

static void
TestOrderOnOneWorker( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Work queue: with one worker thread the background tasks run one after the other in the order they were added, on a thread of the named GUCEF thread pool" )
        try
        {
            COgreTaskWorkQueue queue;
            queue.SetThreadPoolName( "OgreWorkTestOrder" );
            queue.setWorkerThreadCount( 1 );
            ASSERT_FALSE( queue.IsRunning() );
            queue.startup();
            ASSERT_TRUE( queue.IsRunning() );
            ASSERT_FALSE( queue.IsRunningTasksOnCallingThread() );

            CORE::ThreadPoolPtr pool = queue.GetThreadPool();
            ASSERT_TRUE( !pool.IsNULL() && "OgreWorkTestOrder" == pool->GetThreadPoolName() );
            ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool( "OgreWorkTestOrder" ) == pool );

            CTaskLog log;
            for ( Int32 i=0; i<20; ++i )
                queue.addTask( CRecordTask( log, i, 0 == i % 5 ? 3 : 0 ) );
            ASSERT_TRUE( log.WaitForCount( 20, 10000 ) );

            const GUCEF::vector< Int32 > entries = log.Entries();
            for ( Int32 i=0; i<20; ++i )
                ASSERT_TRUE( i == entries[ i ] );

            // threads of the task manager, not this one
            CORE::CThreadPool::TThreadIdVector poolThreads;
            pool->GetAllCurrentThreadIds( poolThreads );
            const GUCEF::vector< TSystemThreadId > threads = log.SystemThreads();
            for ( size_t i=0; i<threads.size(); ++i )
            {
                ASSERT_TRUE( ContainsThread( poolThreads, threads[ i ] ) );
                ASSERT_TRUE( threads[ i ] != MT::GetCurrentThreadSystemID() );
            }
            ASSERT_TRUE( 20 == queue.GetCore()->GetNrOfTasksQueued() );
            ASSERT_TRUE( 20 == queue.GetCore()->GetNrOfTasksRun() );
            ASSERT_TRUE( 0 == queue.GetCore()->GetNrOfTasksFailed() );

            // the worker is a task of the pool, on a thread of its own, and the queue measured the Ogre tasks
            ASSERT_TRUE( 1 == queue.GetCore()->GetNrOfWorkers() );
            ASSERT_TRUE( 1 == pool->GetActiveNrOfDedicatedThreads() );
            CORE::CThreadPoolCounters counters;
            pool->GetCounters( counters );
            ASSERT_TRUE( counters.GetStartedTotal() >= 1 );
            ASSERT_TRUE( queue.GetCore()->GetTotalTaskRunTimeInUs() >= 4 * 3000 );
            queue.shutdown();
            ASSERT_TRUE( 0 == queue.GetCore()->GetNrOfWorkers() );
            ASSERT_FALSE( queue.IsRunning() );
            ASSERT_TRUE( queue.GetThreadPool().IsNULL() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestSeveralWorkers( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Work queue: with several worker threads the background tasks start in the order they were added and run side by side on the threads of the pool" )
        try
        {
            COgreTaskWorkQueue queue;
            queue.SetThreadPoolName( "OgreWorkTestSeveral" );
            queue.setWorkerThreadCount( 3 );
            ASSERT_TRUE( 3 == queue.getWorkerThreadCount() );
            queue.startup();

            // the workers run with the start, as Ogre's threads do
            CORE::ThreadPoolPtr pool = queue.GetThreadPool();
            ASSERT_TRUE( !pool.IsNULL() && 3 == queue.GetCore()->GetNrOfWorkers() );

            CTaskLog starts;
            CTaskLog ends;
            MT::CSemaphore started( 0 );
            MT::CSemaphore release( 0 );
            // three that wait for each other can only end when they run side by side
            for ( Int32 i=0; i<3; ++i )
                queue.addTask( CGateTask( started, release, ends, i ) );
            UInt32 startedCount = 0;
            for ( Int32 i=0; i<3; ++i )
                if ( 0 == started.WaitOrTimeout( 10000 ) )
                    ++startedCount;
            ASSERT_TRUE( 3 == startedCount );
            ASSERT_TRUE( 3 == queue.GetCore()->GetNrOfRunningTasks() );
            for ( Int32 i=0; i<3; ++i )
                release.Post();
            ASSERT_TRUE( ends.WaitForCount( 3, 10000 ) );

            GUCEF::set< TSystemThreadId > distinct;
            const GUCEF::vector< TSystemThreadId > threads = ends.SystemThreads();
            for ( size_t i=0; i<threads.size(); ++i )
                distinct.insert( threads[ i ] );
            ASSERT_TRUE( 3 == distinct.size() );

            for ( Int32 i=0; i<50; ++i )
                queue.addTask( CRecordTask( starts, i ) );
            ASSERT_TRUE( starts.WaitForCount( 50, 10000 ) );
            ASSERT_TRUE( 53 == queue.GetCore()->GetNrOfTasksRun() );
            queue.shutdown();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestBeforeStartupAndPause( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Work queue: background tasks added before the start, or while paused, wait and then run" )
        try
        {
            COgreTaskWorkQueue queue;
            queue.SetThreadPoolName( "OgreWorkTestPause" );
            queue.setWorkerThreadCount( 1 );

            CTaskLog log;
            queue.addTask( CRecordTask( log, 1 ) );
            queue.addTask( CRecordTask( log, 2 ) );
            MT::PrecisionDelay( 50 );
            ASSERT_TRUE( 0 == log.Count() );
            ASSERT_TRUE( 2 == queue.GetCore()->GetNrOfQueuedTasks() );
            queue.startup();
            ASSERT_TRUE( log.WaitForCount( 2, 10000 ) );

            queue.setPaused( true );
            ASSERT_TRUE( queue.isPaused() );
            queue.addTask( CRecordTask( log, 3 ) );
            queue.addTask( CRecordTask( log, 4 ) );
            MT::PrecisionDelay( 50 );
            ASSERT_TRUE( 2 == log.Count() );
            queue.setPaused( false );
            ASSERT_TRUE( log.WaitForCount( 4, 10000 ) );
            const GUCEF::vector< Int32 > entries = log.Entries();
            ASSERT_TRUE( 1 == entries[ 0 ] && 2 == entries[ 1 ] && 3 == entries[ 2 ] && 4 == entries[ 3 ] );

            // a restart without force keeps the queue running; with force it starts again
            queue.startup( false );
            ASSERT_TRUE( queue.IsRunning() );
            queue.startup( true );
            ASSERT_TRUE( queue.IsRunning() );
            queue.addTask( CRecordTask( log, 5 ) );
            ASSERT_TRUE( log.WaitForCount( 5, 10000 ) );
            queue.shutdown();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestMainThreadTasks( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Work queue: main thread tasks run only in processMainThreadTasks(), on the calling thread, in order, within the time limit" )
        try
        {
            COgreTaskWorkQueue queue;
            queue.SetThreadPoolName( "OgreWorkTestMain" );
            queue.setWorkerThreadCount( 1 );
            queue.startup();
            ASSERT_TRUE( 10 == queue.getMainThreadProcessingTimeLimit() );

            CTaskLog log;
            // a background task gives the render thread its part, as resource loading does
            queue.addTask( CAddMainThreadTask( queue, log, 10 ) );
            ASSERT_TRUE( log.WaitForCount( 1, 10000 ) );
            queue.addMainThreadTask( CRecordTask( log, 20 ) );
            MT::PrecisionDelay( 30 );
            ASSERT_TRUE( 1 == log.Count() );
            ASSERT_TRUE( 2 == queue.GetCore()->GetNrOfQueuedMainThreadTasks() );

            queue.processMainThreadTasks();
            ASSERT_TRUE( 3 == log.Count() );
            const GUCEF::vector< Int32 > entries = log.Entries();
            ASSERT_TRUE( 11 == entries[ 1 ] && 20 == entries[ 2 ] );
            const GUCEF::vector< TThreadId > threads = log.Threads();
            ASSERT_TRUE( threads[ 1 ] == MT::GetCurrentThreadID() && threads[ 2 ] == MT::GetCurrentThreadID() );
            ASSERT_TRUE( queue.IsRunningTasksOnCallingThread() || threads[ 0 ] != MT::GetCurrentThreadID() );

            // the limit is checked after each task: tasks of 15 ms with 20 ms give two per call
            queue.setMainThreadProcessingTimeLimit( 20 );
            for ( Int32 i=0; i<6; ++i )
                queue.addMainThreadTask( CRecordTask( log, 100 + i, 15 ) );
            queue.processMainThreadTasks();
            ASSERT_TRUE( 5 == log.Count() );
            queue.processMainThreadTasks();
            ASSERT_TRUE( 7 == log.Count() );

            // without a limit everything runs
            queue.setMainThreadProcessingTimeLimit( 0 );
            queue.processMainThreadTasks();
            ASSERT_TRUE( 9 == log.Count() );
            ASSERT_TRUE( 0 == queue.GetCore()->GetNrOfQueuedMainThreadTasks() );
            ASSERT_TRUE( 8 == queue.GetCore()->GetNrOfMainThreadTasksRun() );
            queue.shutdown();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestAcceptAndAbort( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Work queue: tasks that are not accepted are dropped, and aborting drops the background tasks that did not start while the running one ends" )
        try
        {
            COgreTaskWorkQueue queue;
            queue.SetThreadPoolName( "OgreWorkTestAbort" );
            queue.setWorkerThreadCount( 1 );
            queue.startup();

            CTaskLog log;
            queue.setRequestsAccepted( false );
            ASSERT_FALSE( queue.getRequestsAccepted() );
            queue.addTask( CRecordTask( log, 1 ) );
            queue.addMainThreadTask( CRecordTask( log, 2 ) );
            queue.processMainThreadTasks();
            MT::PrecisionDelay( 30 );
            ASSERT_TRUE( 0 == log.Count() );
            ASSERT_TRUE( 2 == queue.GetCore()->GetNrOfTasksDropped() );
            queue.setRequestsAccepted( true );

            MT::CSemaphore started( 0 );
            MT::CSemaphore release( 0 );
            queue.addTask( CGateTask( started, release, log, 0 ) );
            ASSERT_TRUE( 0 == started.WaitOrTimeout( 10000 ) );

            // what waits behind it: a packaged task as ResourceBackgroundQueue makes them, and plain ones
            std::shared_ptr< std::packaged_task< void() > > packaged( new std::packaged_task< void() >( DoNothing ) );
            std::future< void > future = packaged->get_future();
            queue.addTask( CPackagedTask( packaged ) );
            packaged.reset();
            for ( Int32 i=1; i<=4; ++i )
                queue.addTask( CRecordTask( log, i ) );
            ASSERT_TRUE( 5 == queue.GetCore()->GetNrOfQueuedTasks() );
            ASSERT_TRUE( 5 == queue.AbortQueuedTasks() );
            ASSERT_TRUE( 0 == queue.GetCore()->GetNrOfQueuedTasks() );

            // the waiter of a dropped packaged task learns it will not come
            bool brokenPromise = false;
            try
            {
                future.get();
            }
            catch ( const std::future_error& e )
            {
                brokenPromise = std::future_errc::broken_promise == e.code();
            }
            ASSERT_TRUE( brokenPromise );

            release.Post();
            ASSERT_TRUE( log.WaitForCount( 1, 10000 ) );
            MT::PrecisionDelay( 50 );
            ASSERT_TRUE( 1 == log.Count() && 0 == log.Entries()[ 0 ] );

            // the worker goes on with what comes next
            queue.addTask( CRecordTask( log, 7 ) );
            ASSERT_TRUE( log.WaitForCount( 2, 10000 ) );
            ASSERT_TRUE( 1 == queue.GetCore()->GetNrOfWorkers() );
            queue.shutdown();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestExceptions( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Work queue: an exception that escapes a task, of either kind, is caught and counted and the tasks after it run" )
        try
        {
            COgreTaskWorkQueue queue;
            queue.SetThreadPoolName( "OgreWorkTestExceptions" );
            queue.setWorkerThreadCount( 1 );
            queue.startup();

            CTaskLog log;
            for ( Int32 kind=0; kind<3; ++kind )
            {
                queue.addTask( CThrowTask( kind ) );
                queue.addMainThreadTask( CThrowTask( kind ) );
            }
            queue.addTask( CRecordTask( log, 1 ) );
            queue.addMainThreadTask( CRecordTask( log, 2 ) );
            ASSERT_TRUE( log.WaitForCount( 1, 10000 ) );
            queue.processMainThreadTasks();
            ASSERT_TRUE( 2 == log.Count() );
            ASSERT_TRUE( 4 == queue.GetCore()->GetNrOfTasksRun() );
            ASSERT_TRUE( 3 == queue.GetCore()->GetNrOfTasksFailed() );
            ASSERT_TRUE( 4 == queue.GetCore()->GetNrOfMainThreadTasksRun() );
            ASSERT_TRUE( 3 == queue.GetCore()->GetNrOfMainThreadTasksFailed() );
            queue.shutdown();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestShutdownInFlight( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Work queue: the shut down waits for the running task, drops the others with what they hold, and no task runs after it" )
        try
        {
            MT::CMutex probeLock;
            UInt32 probesDestroyed = 0;
            CTaskLog log;
            MT::CSemaphore started( 0 );
            MT::CSemaphore release( 0 );
            {
                COgreTaskWorkQueue* queue = new COgreTaskWorkQueue();
                queue->SetThreadPoolName( "OgreWorkTestShutdown" );
                queue->setWorkerThreadCount( 1 );
                queue->startup();

                // the running one ends 300 ms after it started, nobody lets it go earlier
                queue->addTask( CGateTask( started, release, log, 0, 300 ) );
                ASSERT_TRUE( 0 == started.WaitOrTimeout( 10000 ) );
                for ( Int32 i=1; i<=10; ++i )
                {
                    std::shared_ptr< CDropProbe > probe( new CDropProbe( probeLock, probesDestroyed ) );
                    queue->addTask( CProbeTask( probe, log, i ) );
                    queue->addMainThreadTask( CProbeTask( probe, log, 100 + i ) );
                }
                {
                    MT::CScopeMutex lock( probeLock );
                    ASSERT_TRUE( 0 == probesDestroyed );
                }
                queue->addTask( CRecordTask( log, 50 ) );
                const UInt64 shutdownStart = MT::PrecisionTickCount();
                queue->shutdown();
                const UInt64 shutdownMs = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - shutdownStart );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "pluginglueOGRE_TestApp: the shut down waited " << shutdownMs << " ms for the running task" );
                ASSERT_TRUE( shutdownMs >= 100 );

                // the running task ended before the shut down did; nothing else ran
                ASSERT_TRUE( 1 == log.Count() && 0 == log.Entries()[ 0 ] );
                {
                    MT::CScopeMutex lock( probeLock );
                    ASSERT_TRUE( 10 == probesDestroyed );
                }
                ASSERT_TRUE( 0 == queue->GetCore()->GetNrOfRunningTasks() );
                ASSERT_TRUE( 0 == queue->GetCore()->GetNrOfQueuedTasks() );
                ASSERT_TRUE( 0 == queue->GetCore()->GetNrOfQueuedMainThreadTasks() );

                // nothing is taken while shut down
                queue->addTask( CRecordTask( log, 60 ) );
                queue->processMainThreadTasks();
                delete queue;
            }
            MT::PrecisionDelay( 100 );
            ASSERT_TRUE( 1 == log.Count() );

            // destroyed with tasks in flight, without a shut down first: the destructor does it
            MT::CSemaphore started2( 0 );
            MT::CSemaphore release2( 0 );
            COgreTaskWorkQueue* queue = new COgreTaskWorkQueue();
            queue->SetThreadPoolName( "OgreWorkTestShutdown" );
            queue->setWorkerThreadCount( 2 );
            queue->startup();
            queue->addTask( CGateTask( started2, release2, log, 2 ) );
            ASSERT_TRUE( 0 == started2.WaitOrTimeout( 10000 ) );
            for ( Int32 i=0; i<100; ++i )
                queue->addTask( CRecordTask( log, 1000 + i, 1 ) );
            release2.Post();
            delete queue;
            const UInt32 countAtDelete = log.Count();
            MT::PrecisionDelay( 100 );
            ASSERT_TRUE( countAtDelete == log.Count() );
            ASSERT_TRUE( countAtDelete >= 2 && countAtDelete <= 102 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestNoWorkerThreads( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Work queue: without worker threads (as on the Web) a background task runs at once on the thread that adds it, main thread tasks still wait" )
        try
        {
            COgreTaskWorkQueue queue;
            queue.setWorkerThreadCount( 0 );
            ASSERT_TRUE( queue.IsRunningTasksOnCallingThread() );

            // also before the start, as Ogre's queue without threads
            CTaskLog log;
            queue.addTask( CRecordTask( log, 1 ) );
            ASSERT_TRUE( 1 == log.Count() );
            queue.startup();
            ASSERT_TRUE( queue.IsRunning() && queue.IsRunningTasksOnCallingThread() );
            ASSERT_TRUE( queue.GetThreadPool().IsNULL() );

            // Ogre code that waits for a task right after adding it does not wait forever
            std::shared_ptr< std::packaged_task< void() > > packaged( new std::packaged_task< void() >( DoNothing ) );
            std::future< void > future = packaged->get_future();
            queue.addTask( CPackagedTask( packaged ) );
            ASSERT_TRUE( std::future_status::ready == future.wait_for( std::chrono::milliseconds( 0 ) ) );

            queue.addTask( CAddMainThreadTask( queue, log, 10 ) );
            ASSERT_TRUE( 2 == log.Count() );
            queue.addTask( CThrowTask( 1 ) );
            ASSERT_TRUE( 1 == queue.GetCore()->GetNrOfTasksFailed() );
            queue.processMainThreadTasks();
            ASSERT_TRUE( 3 == log.Count() );

            const GUCEF::vector< TThreadId > threads = log.Threads();
            for ( size_t i=0; i<threads.size(); ++i )
                ASSERT_TRUE( threads[ i ] == MT::GetCurrentThreadID() );
            const GUCEF::vector< Int32 > entries = log.Entries();
            ASSERT_TRUE( 1 == entries[ 0 ] && 10 == entries[ 1 ] && 11 == entries[ 2 ] );
            queue.shutdown();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestIdleWorkersSleep( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Work queue: idle worker threads sleep on the wake up of the task manager, they do not poll, and a new task wakes one at once" )
        try
        {
            #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_LINUX )
            GUCEF_TESTFW_SKIP( "the processor time of a thread is read from /proc" );
            #endif

            COgreTaskWorkQueue queue;
            queue.SetThreadPoolName( "OgreWorkTestIdle" );
            queue.setWorkerThreadCount( 2 );
            queue.startup();
            CTaskLog log;
            queue.addTask( CRecordTask( log, 1 ) );
            ASSERT_TRUE( log.WaitForCount( 1, 10000 ) );

            CORE::CThreadPool::TThreadIdVector poolThreads;
            queue.GetThreadPool()->GetAllCurrentThreadIds( poolThreads );
            ASSERT_TRUE( poolThreads.size() >= 2 );

            // idle for 1.5 s
            MT::PrecisionDelay( 100 );
            GUCEF::vector< UInt64 > before;
            for ( size_t i=0; i<poolThreads.size(); ++i )
                before.push_back( ThreadCpuTimeInNs( poolThreads[ i ] ) );
            MT::PrecisionDelay( 1500 );
            UInt64 maxUsedNs = 0;
            for ( size_t i=0; i<poolThreads.size(); ++i )
            {
                const UInt64 after = ThreadCpuTimeInNs( poolThreads[ i ] );
                ASSERT_TRUE( 0 != after );
                if ( after - before[ i ] > maxUsedNs )
                    maxUsedNs = after - before[ i ];
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "pluginglueOGRE_TestApp: most processor time of an idle worker thread in 1.5 s: " << ( maxUsedNs / 1000 ) << " us" );
            // a thread that polls uses far more; one that waits wakes at most a few times
            ASSERT_TRUE( maxUsedNs < 20000000 );

            // the wake up: a task added to the idle queue starts well within the idle wait of the task manager
            const UInt64 start = MT::PrecisionTickCount();
            queue.addTask( CRecordTask( log, 2 ) );
            ASSERT_TRUE( log.WaitForCount( 2, 10000 ) );
            const UInt64 latencyMs = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "pluginglueOGRE_TestApp: an idle worker started a new task after " << latencyMs << " ms" );
            ASSERT_TRUE( latencyMs < 500 );
            queue.shutdown();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestQueueOfTheRoot( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Work queue: the Root of the support library has the queue on the GUCEF thread pool of the configuration, and Ogre's own queue when that is configured" )
        try
        {
            Ogre::Root* root = Ogre::Root::getSingletonPtr();
            ASSERT_TRUE( GUCEF_NULL != root );
            if ( GUCEF_NULL == root )
                return;
            PLUGINGLUE::OGRE::COgreRootSetup& setup = PLUGINGLUE::OGRE::COgreGlobal::Instance()->GetRootSetup();
            ASSERT_TRUE( PLUGINGLUE::OGRE::COgreRootSetup::WorkQueueTypeGucef == setup.GetWorkQueueType() );
            COgreTaskWorkQueue* queue = setup.GetTaskWorkQueue();
            ASSERT_TRUE( GUCEF_NULL != queue && root->getWorkQueue() == queue );
            ASSERT_TRUE( "OgreWork" == queue->GetThreadPoolName() && 2 == queue->getWorkerThreadCount() );
            // the Root starts it with the first render window; this test application has none
            ASSERT_FALSE( queue->IsRunning() );

            // the configuration
            CORE::CDataNode config;
            CORE::CDataNode* section = config.AddChild( "Ogre" );
            section->SetAttribute( "workQueue", "Gucef" );
            section->SetAttribute( "workerPool", "OgreWorkRoot" );
            section->SetAttribute( "workerThreads", (UInt32) 3 );
            section->SetAttribute( "mainThreadBudgetMs", (UInt32) 4 );
            section->SetAttribute( "workersCanAccessRenderSystem", false );
            ASSERT_TRUE( setup.LoadConfig( config ) );
            ASSERT_TRUE( setup.GetTaskWorkQueue() == queue );
            ASSERT_TRUE( "OgreWorkRoot" == queue->GetThreadPoolName() && 3 == queue->getWorkerThreadCount() );
            ASSERT_TRUE( 4 == queue->getMainThreadProcessingTimeLimit() );
            ASSERT_FALSE( queue->GetWorkersCanAccessRenderSystem() );

            CORE::CDataNode saved;
            ASSERT_TRUE( setup.SaveConfig( saved ) );
            const CORE::CDataNode* savedSection = saved.Find( "Ogre" );
            ASSERT_TRUE( GUCEF_NULL != savedSection );
            if ( GUCEF_NULL != savedSection )
            {
                ASSERT_TRUE( "OgreWorkRoot" == savedSection->GetAttributeValueOrChildValueByName( "workerPool" ).AsString() );
                ASSERT_TRUE( 3 == savedSection->GetAttributeValueOrChildValueByName( "workerThreads" ).AsUInt32() );
            }

            // Ogre's own queue back, and again the one on the pool
            section->SetAttribute( "workQueue", "Ogre" );
            ASSERT_TRUE( setup.LoadConfig( config ) );
            ASSERT_TRUE( GUCEF_NULL == setup.GetTaskWorkQueue() );
            ASSERT_TRUE( GUCEF_NULL != root->getWorkQueue() );
            ASSERT_TRUE( 3 == root->getWorkQueue()->getWorkerThreadCount() );
            section->SetAttribute( "workQueue", "Gucef" );
            section->SetAttribute( "workerPool", "OgreWork" );
            section->SetAttribute( "workerThreads", (UInt32) 2 );
            section->SetAttribute( "mainThreadBudgetMs", (UInt32) 10 );
            ASSERT_TRUE( setup.LoadConfig( config ) );
            queue = setup.GetTaskWorkQueue();
            ASSERT_TRUE( GUCEF_NULL != queue && root->getWorkQueue() == queue );

            // started by hand here, as the Root does with the first window: the work of Ogre goes through it
            queue->startup();
            CTaskLog log;
            root->getWorkQueue()->addTask( CAddMainThreadTask( *root->getWorkQueue(), log, 1 ) );
            ASSERT_TRUE( log.WaitForCount( 1, 10000 ) );
            root->getWorkQueue()->processMainThreadTasks();
            ASSERT_TRUE( 2 == log.Count() );
            CORE::CThreadPool::TThreadIdVector poolThreads;
            CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool( "OgreWork" )->GetAllCurrentThreadIds( poolThreads );
            ASSERT_TRUE( ContainsThread( poolThreads, log.SystemThreads()[ 0 ] ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformWorkQueueTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "pluginglueOGRE.WorkQueue" );

    // Ogre built without thread support (the Web) runs every background task on the calling thread,
    // the cases that look at worker threads and pools are made for the other builds
    #if ( OGRE_THREAD_SUPPORT != 0 )
    TestOrderOnOneWorker();
    TestSeveralWorkers();
    TestBeforeStartupAndPause();
    #endif /* OGRE_THREAD_SUPPORT != 0 ? */
    TestMainThreadTasks();
    #if ( OGRE_THREAD_SUPPORT != 0 )
    TestAcceptAndAbort();
    #endif /* OGRE_THREAD_SUPPORT != 0 ? */
    TestExceptions();
    #if ( OGRE_THREAD_SUPPORT != 0 )
    TestShutdownInFlight();
    #endif /* OGRE_THREAD_SUPPORT != 0 ? */
    TestNoWorkerThreads();
    #if ( OGRE_THREAD_SUPPORT != 0 )
    TestIdleWorkersSleep();
    TestQueueOfTheRoot();
    #endif /* OGRE_THREAD_SUPPORT != 0 ? */
}

/*-------------------------------------------------------------------------*/
