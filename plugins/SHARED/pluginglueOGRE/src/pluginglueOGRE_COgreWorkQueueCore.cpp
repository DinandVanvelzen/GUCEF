/*
 *  pluginglueOGRE: Support library of the Ogre 1.x plugin set: Ogre Root setup, log, plugin loading
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

#include <exception>

#include "OgrePrerequisites.h"
#include "OgreRenderSystem.h"
#include "OgreException.h"

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_COGREWORKQUEUETASKDATA_H
#include "pluginglueOGRE_COgreWorkQueueTaskData.h"
#define GUCEF_PLUGINGLUE_OGRE_COGREWORKQUEUETASKDATA_H
#endif /* GUCEF_PLUGINGLUE_OGRE_COGREWORKQUEUETASKDATA_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_COGREWORKQUEUETASKCONSUMER_H
#include "pluginglueOGRE_COgreWorkQueueTaskConsumer.h"
#define GUCEF_PLUGINGLUE_OGRE_COGREWORKQUEUETASKCONSUMER_H
#endif /* GUCEF_PLUGINGLUE_OGRE_COGREWORKQUEUETASKCONSUMER_H ? */

#include "pluginglueOGRE_COgreWorkQueueCore.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PLUGINGLUE {
namespace OGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString COgreWorkQueueCore::DefaultThreadPoolName = "OgreWork";
const UInt32 COgreWorkQueueCore::DefaultWorkerThreadCount = 2;
const UInt32 COgreWorkQueueCore::DefaultMainThreadBudgetInMs = 10;
const UInt32 COgreWorkQueueCore::WorkerIdleWaitInMs = 1000;
const UInt32 COgreWorkQueueCore::StartupTimeoutInMs = 10000;
const UInt32 COgreWorkQueueCore::ShutdownTaskTimeoutInMs = 30000;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

COgreWorkQueueCore::COgreWorkQueueCore( void )
    : CORE::CTSharedObjCreator< COgreWorkQueueCore, MT::CMutex >( this )
    , m_lock()
    , m_poolName( DefaultThreadPoolName )
    , m_workerThreadCount( DefaultWorkerThreadCount )
    , m_workersCanAccessRenderSystem( 1 == OGRE_THREAD_SUPPORT )
    , m_mainThreadBudgetInMs( DefaultMainThreadBudgetInMs )
    , m_running( false )
    , m_shuttingDown( false )
    , m_stopWorkers( false )
    , m_paused( false )
    , m_acceptTasks( true )
    , m_runOnCallingThread( false )
    , m_tasks()
    , m_taskQueuedTicks()
    , m_mainThreadTasks()
    , m_mainThreadTaskCount( 0 )
    , m_runningTasks( 0 )
    , m_threadPool()
    , m_workers()
    , m_workerWakeUps()
    , m_workerIdle()
    , m_idleWorkers()
    , m_runningWorkers( 0 )
    , m_workerStarted( 0 )
    , m_renderSystem( GUCEF_NULL )
    , m_registeredThreads()
    , m_tasksQueued( 0 )
    , m_tasksRun( 0 )
    , m_tasksFailed( 0 )
    , m_mainThreadTasksRun( 0 )
    , m_mainThreadTasksFailed( 0 )
    , m_tasksDropped( 0 )
    , m_queueTicks( 0 )
    , m_runTicks( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

COgreWorkQueueCore::~COgreWorkQueueCore()
{GUCEF_TRACE;

    // the last reference: no worker runs any more
    for ( size_t i=0; i<m_workerWakeUps.size(); ++i )
        GUCEF_DELETE m_workerWakeUps[ i ];
    m_workerWakeUps.clear();
}

/*-------------------------------------------------------------------------*/

void
COgreWorkQueueCore::SetThreadPoolName( const CORE::CString& poolName )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_poolName = poolName.IsNULLOrEmpty() ? DefaultThreadPoolName : poolName;
}

/*-------------------------------------------------------------------------*/

CORE::CString
COgreWorkQueueCore::GetThreadPoolName( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_poolName;
}

/*-------------------------------------------------------------------------*/

void
COgreWorkQueueCore::SetWorkerThreadCount( UInt32 threadCount )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_workerThreadCount = threadCount;
}

/*-------------------------------------------------------------------------*/

UInt32
COgreWorkQueueCore::GetWorkerThreadCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_workerThreadCount;
}

/*-------------------------------------------------------------------------*/

bool
COgreWorkQueueCore::IsRunningTasksOnCallingThread( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_running )
        return m_runOnCallingThread;
    return 0 == OGRE_THREAD_SUPPORT || 0 == m_workerThreadCount;
}

/*-------------------------------------------------------------------------*/

void
COgreWorkQueueCore::SetWorkersCanAccessRenderSystem( bool canAccess )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_workersCanAccessRenderSystem = canAccess;
}

/*-------------------------------------------------------------------------*/

bool
COgreWorkQueueCore::GetWorkersCanAccessRenderSystem( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_workersCanAccessRenderSystem;
}

/*-------------------------------------------------------------------------*/

void
COgreWorkQueueCore::SetMainThreadBudgetInMs( UInt32 budgetInMs )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_mainThreadBudgetInMs = budgetInMs;
}

/*-------------------------------------------------------------------------*/

UInt32
COgreWorkQueueCore::GetMainThreadBudgetInMs( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_mainThreadBudgetInMs;
}

/*-------------------------------------------------------------------------*/

bool
COgreWorkQueueCore::Startup( Ogre::RenderSystem* renderSystem )
{GUCEF_TRACE;

    UInt32 workerCount = 0;
    bool registerThreads = false;
    CORE::CString poolName;
    {
        MT::CScopeMutex lock( m_lock );
        if ( m_running )
            return true;

        m_shuttingDown = false;
        m_stopWorkers = false;
        m_runOnCallingThread = 0 == OGRE_THREAD_SUPPORT || 0 == m_workerThreadCount;
        m_running = true;
        if ( m_runOnCallingThread )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "OgreWorkQueue: Started without worker threads, background tasks run on the threads that add them" );
            return true;
        }

        m_threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( m_poolName );
        if ( m_threadPool.IsNULL() )
        {
            // the work is still done, on the threads that add it
            m_runOnCallingThread = true;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreWorkQueue: The thread pool " << m_poolName
                << " is not available, background tasks run on the threads that add them" );
            return false;
        }

        workerCount = m_workerThreadCount;
        poolName = m_poolName;
        registerThreads = m_workersCanAccessRenderSystem && GUCEF_NULL != renderSystem;
        m_renderSystem = registerThreads ? renderSystem : GUCEF_NULL;
        while ( m_workerWakeUps.size() < workerCount )
            m_workerWakeUps.push_back( GUCEF_NEW MT::CSemaphore( 0 ) );
        for ( size_t i=0; i<m_workerWakeUps.size(); ++i )
            while ( 0 == m_workerWakeUps[ i ]->TryWait() ) {}
        while ( 0 == m_workerStarted.TryWait() ) {}
        m_workerIdle.assign( workerCount, false );
        m_idleWorkers.clear();
        m_workers.clear();
    }

    if ( registerThreads )
    {
        if ( 1 != OGRE_THREAD_SUPPORT )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "OgreWorkQueue: Worker threads register with the render system, but Ogre is built with OGRE_THREAD_SUPPORT "
                << OGRE_THREAD_SUPPORT << ": its resource managers take no locks for tasks that use the render system" );
        }
        // Ogre's order: the starting thread lets go of its context, the threads register, the starting thread takes it back
        try
        {
            renderSystem->preExtraThreadsStarted();
        }
        catch ( const Ogre::Exception& e )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreWorkQueue: The render system failed to prepare for worker threads: " << e.getFullDescription() );
        }
    }

    // one task of the pool per worker, each on a thread of its own, for as long as the queue runs
    UInt32 started = 0;
    for ( UInt32 i=0; i<workerCount; ++i )
    {
        COgreWorkQueueTaskData taskData( CreateSharedPtr(), i );
        CORE::CFutureResult future = m_threadPool->StartTask( COgreWorkQueueTaskConsumer::TaskType, &taskData, false );
        if ( future.HasNoFuture() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreWorkQueue: Failed to start worker " << i << " on thread pool " << poolName );
            continue;
        }
        MT::CScopeMutex lock( m_lock );
        m_workers.push_back( future );
        ++started;
    }

    // the workers are ready (and registered) before the start returns, as with Ogre's own queue
    UInt32 ready = 0;
    while ( ready < started && 0 == m_workerStarted.WaitOrTimeout( StartupTimeoutInMs ) )
        ++ready;
    if ( ready < started )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreWorkQueue: Only " << ready << " of " << started << " workers on thread pool " << poolName
            << " got ready within " << StartupTimeoutInMs << " ms" );
    }

    if ( registerThreads )
    {
        try
        {
            renderSystem->postExtraThreadsStarted();
        }
        catch ( const Ogre::Exception& e )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreWorkQueue: The render system failed to end the start of worker threads: " << e.getFullDescription() );
        }
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "OgreWorkQueue: " << GetNrOfThreadsRegisteredWithRenderSystem() << " worker threads registered with the render system" );
    }

    if ( 0 == started )
    {
        MT::CScopeMutex lock( m_lock );
        m_runOnCallingThread = true;
        m_threadPool.Unlink();
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreWorkQueue: No worker started, background tasks run on the threads that add them" );
        return false;
    }

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "OgreWorkQueue: Started with " << started << " workers on thread pool " << poolName );
    return started == workerCount;
}

/*-------------------------------------------------------------------------*/

void
COgreWorkQueueCore::Shutdown( void )
{GUCEF_TRACE;

    TFutureVector workers;
    bool wasRunning = false;
    {
        MT::CScopeMutex lock( m_lock );
        wasRunning = m_running;
        m_shuttingDown = true;
        m_stopWorkers = true;
        workers = m_workers;
    }
    WakeAllWorkers();

    // each worker ends after the task it runs
    for ( size_t i=0; i<workers.size(); ++i )
    {
        const UInt64 startTicks = MT::PrecisionTickCount();
        bool loggedWait = false;
        while ( !workers[ i ].IsReady() )
        {
            const Float64 waitedMs = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - startTicks );
            if ( waitedMs >= ShutdownTaskTimeoutInMs )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "OgreWorkQueue: A worker did not end within " << ShutdownTaskTimeoutInMs
                    << " ms of the shut down, it is no longer waited for" );
                break;
            }
            if ( !loggedWait && waitedMs >= 1000 )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "OgreWorkQueue: Waiting for a background task to end before the shut down" );
                loggedWait = true;
            }
            MT::PrecisionDelay( 1 );
        }
    }

    // what did not run is dropped here, while the Root and its managers still exist
    TOgreTaskList droppedTasks;
    TOgreTaskList droppedMainThreadTasks;
    {
        MT::CScopeMutex lock( m_lock );
        droppedTasks.swap( m_tasks );
        m_taskQueuedTicks.clear();
        droppedMainThreadTasks.swap( m_mainThreadTasks );
        m_mainThreadTaskCount.Store( 0, MT::MEMORDER_RELEASE );
        m_tasksDropped += droppedTasks.size() + droppedMainThreadTasks.size();
        m_workers.clear();
        m_idleWorkers.clear();
        m_threadPool.Unlink();
        m_renderSystem = GUCEF_NULL;
        m_running = false;
    }

    if ( wasRunning || !droppedTasks.empty() || !droppedMainThreadTasks.empty() )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "OgreWorkQueue: Shut down, " << droppedTasks.size() << " background tasks and "
            << droppedMainThreadTasks.size() << " main thread tasks that did not run are dropped" );
    }

    // the dropped tasks release what they hold when the lists go, outside of the lock
}

/*-------------------------------------------------------------------------*/

bool
COgreWorkQueueCore::IsRunning( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_running;
}

/*-------------------------------------------------------------------------*/

bool
COgreWorkQueueCore::IsShuttingDown( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_shuttingDown;
}

/*-------------------------------------------------------------------------*/

bool
COgreWorkQueueCore::RunTask( const TOgreTask& task, const char* kind )
{GUCEF_TRACE;

    try
    {
        task();
        return true;
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreWorkQueue: A " << kind << " task failed: " << e.getFullDescription() );
    }
    catch ( const std::exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreWorkQueue: A " << kind << " task failed: " << e.what() );
    }
    catch ( ... )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreWorkQueue: A " << kind << " task failed with an unknown exception" );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

MT::CSemaphore*
COgreWorkQueueCore::TakeIdleWorkerLocked( void )
{GUCEF_TRACE;

    if ( m_idleWorkers.empty() )
        return GUCEF_NULL;
    const UInt32 worker = m_idleWorkers.back();
    m_idleWorkers.pop_back();
    m_workerIdle[ worker ] = false;
    return m_workerWakeUps[ worker ];
}

/*-------------------------------------------------------------------------*/

void
COgreWorkQueueCore::RemoveIdleWorkerLocked( UInt32 workerIndex )
{GUCEF_TRACE;

    if ( !m_workerIdle[ workerIndex ] )
        return;
    m_workerIdle[ workerIndex ] = false;
    for ( size_t i=0; i<m_idleWorkers.size(); ++i )
    {
        if ( m_idleWorkers[ i ] == workerIndex )
        {
            m_idleWorkers.erase( m_idleWorkers.begin() + i );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
COgreWorkQueueCore::WakeAllWorkers( void )
{GUCEF_TRACE;

    TSemaphoreVector wakeUps;
    {
        MT::CScopeMutex lock( m_lock );
        MT::CSemaphore* wakeUp = GUCEF_NULL;
        while ( GUCEF_NULL != ( wakeUp = TakeIdleWorkerLocked() ) )
            wakeUps.push_back( wakeUp );
    }
    for ( size_t i=0; i<wakeUps.size(); ++i )
        wakeUps[ i ]->Post();
}

/*-------------------------------------------------------------------------*/

void
COgreWorkQueueCore::AddTask( const TOgreTask& task )
{GUCEF_TRACE;

    MT::CSemaphore* wakeUp = GUCEF_NULL;
    bool runNow = false;
    {
        MT::CScopeMutex lock( m_lock );
        if ( !m_acceptTasks || m_shuttingDown || !task )
        {
            ++m_tasksDropped;
            return;
        }
        runNow = m_running ? m_runOnCallingThread : ( 0 == OGRE_THREAD_SUPPORT || 0 == m_workerThreadCount );
        if ( runNow )
        {
            ++m_runningTasks;
        }
        else
        {
            m_tasks.push_back( task );
            m_taskQueuedTicks.push_back( MT::PrecisionTickCount() );
            ++m_tasksQueued;
            // one idle worker per task: busy workers take the next task without sleeping
            if ( !m_paused )
                wakeUp = TakeIdleWorkerLocked();
        }
    }

    if ( !runNow )
    {
        // queued: a worker that sleeps is woken for it
        if ( GUCEF_NULL != wakeUp )
            wakeUp->Post();
        return;
    }

    // without worker threads: at once, on this thread, as Ogre does without thread support
    const UInt64 startTicks = MT::PrecisionTickCount();
    const bool success = RunTask( task, "background" );
    const UInt64 endTicks = MT::PrecisionTickCount();

    MT::CScopeMutex lock( m_lock );
    --m_runningTasks;
    ++m_tasksRun;
    m_runTicks += endTicks - startTicks;
    if ( !success )
        ++m_tasksFailed;
}

/*-------------------------------------------------------------------------*/

void
COgreWorkQueueCore::AddMainThreadTask( const TOgreTask& task )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_acceptTasks || m_shuttingDown || !task )
    {
        ++m_tasksDropped;
        return;
    }
    m_mainThreadTasks.push_back( task );
    m_mainThreadTaskCount.Store( (UInt32) m_mainThreadTasks.size(), MT::MEMORDER_RELEASE );
}

/*-------------------------------------------------------------------------*/

UInt32
COgreWorkQueueCore::ProcessMainThreadTasks( void )
{GUCEF_TRACE;

    // called every frame: an empty list costs one atomic read
    if ( 0 == m_mainThreadTaskCount.Load( MT::MEMORDER_ACQUIRE ) )
        return 0;

    const UInt64 startTicks = MT::PrecisionTickCount();
    UInt32 budgetInMs = 0;
    UInt32 tasksRun = 0;
    UInt32 tasksFailed = 0;
    while ( true )
    {
        TOgreTask task;
        {
            MT::CScopeMutex lock( m_lock );
            if ( m_mainThreadTasks.empty() )
                break;
            budgetInMs = m_mainThreadBudgetInMs;
            task.swap( m_mainThreadTasks.front() );
            m_mainThreadTasks.pop_front();
            m_mainThreadTaskCount.Store( (UInt32) m_mainThreadTasks.size(), MT::MEMORDER_RELEASE );
        }

        if ( !RunTask( task, "main thread" ) )
            ++tasksFailed;
        task = TOgreTask();
        ++tasksRun;

        // the budget is checked after each task, as Ogre does: one long task can overshoot it
        if ( 0 != budgetInMs && MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - startTicks ) > budgetInMs )
            break;
    }

    MT::CScopeMutex lock( m_lock );
    m_mainThreadTasksRun += tasksRun;
    m_mainThreadTasksFailed += tasksFailed;
    return tasksRun;
}

/*-------------------------------------------------------------------------*/

UInt32
COgreWorkQueueCore::AbortQueuedTasks( void )
{GUCEF_TRACE;

    TOgreTaskList aborted;
    {
        MT::CScopeMutex lock( m_lock );
        aborted.swap( m_tasks );
        m_taskQueuedTicks.clear();
        m_tasksDropped += aborted.size();
    }
    return (UInt32) aborted.size();
}

/*-------------------------------------------------------------------------*/

void
COgreWorkQueueCore::SetPaused( bool paused )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        m_paused = paused;
    }
    if ( !paused )
        WakeAllWorkers();
}

/*-------------------------------------------------------------------------*/

bool
COgreWorkQueueCore::IsPaused( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_paused;
}

/*-------------------------------------------------------------------------*/

void
COgreWorkQueueCore::SetTasksAccepted( bool accepted )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_acceptTasks = accepted;
}

/*-------------------------------------------------------------------------*/

bool
COgreWorkQueueCore::AreTasksAccepted( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_acceptTasks;
}

/*-------------------------------------------------------------------------*/

void
COgreWorkQueueCore::RunWorker( UInt32 workerIndex, CORE::CTaskConsumer& consumer )
{GUCEF_TRACE;

    Ogre::RenderSystem* renderSystem = GUCEF_NULL;
    MT::CSemaphore* wakeUp = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        if ( workerIndex >= m_workerWakeUps.size() )
            return;
        renderSystem = m_renderSystem;
        wakeUp = m_workerWakeUps[ workerIndex ];
        ++m_runningWorkers;
    }

    bool registered = false;
    if ( GUCEF_NULL != renderSystem )
    {
        try
        {
            renderSystem->registerThread();
            registered = true;
            MT::CScopeMutex lock( m_lock );
            m_registeredThreads.insert( MT::GetCurrentThreadID() );
        }
        catch ( const Ogre::Exception& e )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreWorkQueue: A worker thread failed to register with the render system: " << e.getFullDescription() );
        }
    }
    m_workerStarted.Post();

    while ( true )
    {
        TOgreTask task;
        UInt64 queuedTicks = 0;
        {
            MT::CScopeMutex lock( m_lock );
            if ( m_stopWorkers )
                break;
            if ( !m_paused && !m_tasks.empty() )
            {
                task.swap( m_tasks.front() );
                m_tasks.pop_front();
                queuedTicks = m_taskQueuedTicks.front();
                m_taskQueuedTicks.pop_front();
                ++m_runningTasks;
                // woken by its time out, not by an adder: it is not idle any more
                RemoveIdleWorkerLocked( workerIndex );
            }
            else
            if ( !m_workerIdle[ workerIndex ] )
            {
                m_workerIdle[ workerIndex ] = true;
                m_idleWorkers.push_back( workerIndex );
            }
        }

        if ( task )
        {
            const UInt64 startTicks = MT::PrecisionTickCount();
            const bool success = RunTask( task, "background" );
            task = TOgreTask();
            const UInt64 endTicks = MT::PrecisionTickCount();

            MT::CScopeMutex lock( m_lock );
            --m_runningTasks;
            ++m_tasksRun;
            if ( !success )
                ++m_tasksFailed;
            m_queueTicks += startTicks > queuedTicks ? startTicks - queuedTicks : 0;
            m_runTicks += endTicks - startTicks;
            continue;
        }

        // asleep until a task comes for this worker; at most the idle wait of the task manager, to see a stop request
        if ( 0 != wakeUp->WaitOrTimeout( WorkerIdleWaitInMs ) && consumer.IsDeactivationRequested() )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "OgreWorkQueue: Worker " << workerIndex << " is asked to stop by its thread pool" );
            break;
        }
    }

    {
        MT::CScopeMutex lock( m_lock );
        RemoveIdleWorkerLocked( workerIndex );
        --m_runningWorkers;
    }

    if ( registered )
    {
        try
        {
            renderSystem->unregisterThread();
        }
        catch ( const Ogre::Exception& e )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreWorkQueue: A worker thread failed to unregister from the render system: " << e.getFullDescription() );
        }
        MT::CScopeMutex lock( m_lock );
        m_registeredThreads.erase( MT::GetCurrentThreadID() );
    }
}

/*-------------------------------------------------------------------------*/

UInt32
COgreWorkQueueCore::GetNrOfQueuedTasks( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_tasks.size();
}

/*-------------------------------------------------------------------------*/

UInt32
COgreWorkQueueCore::GetNrOfQueuedMainThreadTasks( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_mainThreadTasks.size();
}

/*-------------------------------------------------------------------------*/

UInt32
COgreWorkQueueCore::GetNrOfRunningTasks( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_runningTasks;
}

/*-------------------------------------------------------------------------*/

UInt32
COgreWorkQueueCore::GetNrOfWorkers( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_runningWorkers;
}

/*-------------------------------------------------------------------------*/

UInt64
COgreWorkQueueCore::GetNrOfTasksQueued( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_tasksQueued;
}

/*-------------------------------------------------------------------------*/

UInt64
COgreWorkQueueCore::GetNrOfTasksRun( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_tasksRun;
}

/*-------------------------------------------------------------------------*/

UInt64
COgreWorkQueueCore::GetNrOfTasksFailed( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_tasksFailed;
}

/*-------------------------------------------------------------------------*/

UInt64
COgreWorkQueueCore::GetNrOfMainThreadTasksRun( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_mainThreadTasksRun;
}

/*-------------------------------------------------------------------------*/

UInt64
COgreWorkQueueCore::GetNrOfMainThreadTasksFailed( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_mainThreadTasksFailed;
}

/*-------------------------------------------------------------------------*/

UInt64
COgreWorkQueueCore::GetNrOfTasksDropped( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_tasksDropped;
}

/*-------------------------------------------------------------------------*/

UInt64
COgreWorkQueueCore::GetTotalTaskQueueTimeInUs( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt64) ( MT::PrecisionTimerTicksToMs( m_queueTicks ) * 1000.0 );
}

/*-------------------------------------------------------------------------*/

UInt64
COgreWorkQueueCore::GetTotalTaskRunTimeInUs( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt64) ( MT::PrecisionTimerTicksToMs( m_runTicks ) * 1000.0 );
}

/*-------------------------------------------------------------------------*/

CORE::ThreadPoolPtr
COgreWorkQueueCore::GetThreadPool( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_threadPool;
}

/*-------------------------------------------------------------------------*/

bool
COgreWorkQueueCore::IsCurrentThreadRegisteredWithRenderSystem( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_registeredThreads.end() != m_registeredThreads.find( MT::GetCurrentThreadID() );
}

/*-------------------------------------------------------------------------*/

UInt32
COgreWorkQueueCore::GetNrOfThreadsRegisteredWithRenderSystem( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_registeredThreads.size();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
