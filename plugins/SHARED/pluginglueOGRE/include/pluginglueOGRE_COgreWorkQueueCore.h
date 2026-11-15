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

#ifndef GUCEF_PLUGINGLUE_OGRE_COGREWORKQUEUECORE_H
#define GUCEF_PLUGINGLUE_OGRE_COGREWORKQUEUECORE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <deque>
#include <functional>

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSEMAPHORE_H
#include "gucefMT_CSemaphore.h"
#define GUCEF_MT_CSEMAPHORE_H
#endif /* GUCEF_MT_CSEMAPHORE_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_MACROS_H
#include "pluginglueOGRE_macros.h"
#define GUCEF_PLUGINGLUE_OGRE_MACROS_H
#endif /* GUCEF_PLUGINGLUE_OGRE_MACROS_H ? */

namespace Ogre { class RenderSystem; }

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The part of COgreTaskWorkQueue that its worker threads share with the queue: the task lists of Ogre's work queue,
 *  their state and their counters.
 *
 *  The worker threads are GUCEF tasks: at the start one task per worker thread (COgreWorkQueueTaskConsumer) is
 *  started on a thread pool of the task manager, each on a thread of its own that carries the name of the pool. Such
 *  a worker takes the background tasks of Ogre (WorkQueue::addTask()) from the list in the order they were added and
 *  runs them; with more than one worker they may end in another order. A worker that finds the list empty sleeps on
 *  its wake up semaphore until a task is added (one idle worker is woken per task added) or, at most, the idle wait
 *  of the task manager has passed, so a stop request of the thread pool is seen. The workers end with the shut down.
 *  They hold this object through a shared pointer.
 *
 *  The task manager shows the workers as tasks of the pool; the Ogre tasks themselves are counted here: queued,
 *  running, run, failed, dropped, and the time they waited in the list and ran.
 *
 *  Tasks for the main thread (WorkQueue::addMainThreadTask()) are kept in a second list that ProcessMainThreadTasks()
 *  runs on the thread that calls it, within its time budget.
 *
 *  Without worker threads (a worker thread count of 0, or an Ogre built without thread support, as on the Web) a
 *  background task runs at once on the thread that adds it, inside AddTask(), as Ogre's own work queue does without
 *  threads; main thread tasks still wait for ProcessMainThreadTasks().
 *
 *  Exceptions that escape a task are caught, logged and counted, on both sides. Shut down, the lists are emptied:
 *  what did not start is dropped while Ogre still exists, so nothing that a task holds is released after the Root.
 */
class GUCEF_PLUGINGLUE_OGRE_EXPORT_CPP COgreWorkQueueCore : public CORE::CTSharedObjCreator< COgreWorkQueueCore, MT::CMutex >
{
    public:

    typedef std::function< void() >                                                     TOgreTask;
    typedef CORE::CTSharedObjCreator< COgreWorkQueueCore, MT::CMutex >::TSharedPtrType   TOgreWorkQueueCorePtr;

    static const CORE::CString DefaultThreadPoolName;           /**< "OgreWork" */
    static const UInt32 DefaultWorkerThreadCount;               /**< 2, as many as Ogre's own work queue uses at most */
    static const UInt32 DefaultMainThreadBudgetInMs;            /**< 10, Ogre's default */
    static const UInt32 WorkerIdleWaitInMs;                     /**< the longest a worker sleeps without looking at its stop request */
    static const UInt32 StartupTimeoutInMs;                     /**< how long the workers are waited for at the start */
    static const UInt32 ShutdownTaskTimeoutInMs;                /**< how long a worker is waited for at the shut down */

    COgreWorkQueueCore( void );

    virtual ~COgreWorkQueueCore();

    /** The thread pool of the task manager that the workers run on, made when it is not there. Used from the next start */
    void SetThreadPoolName( const CORE::CString& poolName );

    CORE::CString GetThreadPoolName( void ) const;

    /** The number of worker threads, used from the next start. 0 runs background tasks on the thread that adds them */
    void SetWorkerThreadCount( UInt32 threadCount );

    UInt32 GetWorkerThreadCount( void ) const;

    /** True when background tasks run on the thread that adds them: no worker threads, or an Ogre without thread support */
    bool IsRunningTasksOnCallingThread( void ) const;

    /**
     *  Whether background tasks may use the render system. When set, the thread that starts the queue calls
     *  RenderSystem::preExtraThreadsStarted(), each worker calls RenderSystem::registerThread() before it takes tasks,
     *  and when all did the starting thread calls RenderSystem::postExtraThreadsStarted(); each worker calls
     *  RenderSystem::unregisterThread() when it ends. Used from the next start. Off by default, as Ogre does unless it
     *  is built with OGRE_THREAD_SUPPORT 1. Note that the EGL support of Ogre 14.6.3 (GL3Plus on Linux, GLES2) binds
     *  the context of a registered thread to the window surface of the main context: only one thread can register,
     *  and it keeps that surface current, so the render thread cannot take it back while it does
     */
    void SetWorkersCanAccessRenderSystem( bool canAccess );

    bool GetWorkersCanAccessRenderSystem( void ) const;

    /** The time ProcessMainThreadTasks() may take, checked after each task. 0 is no limit */
    void SetMainThreadBudgetInMs( UInt32 budgetInMs );

    UInt32 GetMainThreadBudgetInMs( void ) const;

    /**
     *  Starts the queue: the workers start on the thread pool (and register with the render system when that is asked
     *  for and one is given), then they take the background tasks added before the start. False when the workers could
     *  not be started; background tasks then run on the calling thread
     */
    bool Startup( Ogre::RenderSystem* renderSystem );

    /**
     *  Stops the queue: no tasks are taken any more, the workers finish the task they run and end (they unregister
     *  from the render system), and the tasks that did not run are dropped. Called on the thread that started the queue
     */
    void Shutdown( void );

    bool IsRunning( void ) const;

    bool IsShuttingDown( void ) const;

    /** Adds a background task. Dropped when tasks are not accepted or the queue shuts down */
    void AddTask( const TOgreTask& task );

    /** Adds a task for ProcessMainThreadTasks(). Dropped when tasks are not accepted or the queue shuts down */
    void AddMainThreadTask( const TOgreTask& task );

    /** Runs the main thread tasks on the calling thread, also those added meanwhile, within the budget. Returns how many ran */
    UInt32 ProcessMainThreadTasks( void );

    /** Takes the background tasks that did not start out of the list; they are not run. Returns how many */
    UInt32 AbortQueuedTasks( void );

    /** While paused, the workers take no background tasks; those that run go on */
    void SetPaused( bool paused );

    bool IsPaused( void ) const;

    /** Whether new tasks are taken, of both kinds */
    void SetTasksAccepted( bool accepted );

    bool AreTasksAccepted( void ) const;

    /** Background tasks that wait to start */
    UInt32 GetNrOfQueuedTasks( void ) const;

    UInt32 GetNrOfQueuedMainThreadTasks( void ) const;

    /** Background tasks that run at this moment */
    UInt32 GetNrOfRunningTasks( void ) const;

    /** Workers that run, as tasks of the thread pool */
    UInt32 GetNrOfWorkers( void ) const;

    UInt64 GetNrOfTasksQueued( void ) const;            /**< background tasks taken into the list, in total */
    UInt64 GetNrOfTasksRun( void ) const;               /**< background tasks that ran, including those that failed */
    UInt64 GetNrOfTasksFailed( void ) const;            /**< background tasks that ended with an exception */
    UInt64 GetNrOfMainThreadTasksRun( void ) const;     /**< main thread tasks that ran, including those that failed */
    UInt64 GetNrOfMainThreadTasksFailed( void ) const;  /**< main thread tasks that ended with an exception */
    UInt64 GetNrOfTasksDropped( void ) const;           /**< tasks of both kinds that were not taken or were dropped unrun */
    UInt64 GetTotalTaskQueueTimeInUs( void ) const;     /**< the time background tasks waited in the list before a worker took them, together */
    UInt64 GetTotalTaskRunTimeInUs( void ) const;       /**< the time background tasks ran on the workers, together */

    /** The thread pool while the queue runs workers on one, otherwise NULL */
    CORE::ThreadPoolPtr GetThreadPool( void ) const;

    bool IsCurrentThreadRegisteredWithRenderSystem( void ) const;

    UInt32 GetNrOfThreadsRegisteredWithRenderSystem( void ) const;

    /**
     *  The loop of a worker, called by COgreWorkQueueTaskConsumer on its thread of the pool: it runs until the queue
     *  shuts down or the consumer is asked to stop
     */
    void RunWorker( UInt32 workerIndex, CORE::CTaskConsumer& consumer );

    private:

    typedef std::deque< TOgreTask, GUCEF::gucef_allocator< TOgreTask > >    TOgreTaskList;
    typedef std::deque< UInt64, GUCEF::gucef_allocator< UInt64 > >          TTickList;
    typedef GUCEF::set< TThreadId >                                         TThreadIdSet;
    typedef GUCEF::vector< MT::CSemaphore* >                                TSemaphoreVector;
    typedef GUCEF::vector< CORE::CFutureResult >                            TFutureVector;
    typedef GUCEF::vector< UInt32 >                                         TWorkerIndexVector;
    typedef GUCEF::vector< bool >                                           TBoolVector;

    COgreWorkQueueCore( const COgreWorkQueueCore& src );              /**< not implemented */
    COgreWorkQueueCore& operator=( const COgreWorkQueueCore& src );   /**< not implemented */

    /** Runs a task on the calling thread, catching what escapes. False when it failed */
    static bool RunTask( const TOgreTask& task, const char* kind );

    /** Takes one idle worker off the idle list, with the lock held. Its wake up semaphore, NULL when none is idle */
    MT::CSemaphore* TakeIdleWorkerLocked( void );

    /** Takes the worker off the idle list when it is on it, with the lock held */
    void RemoveIdleWorkerLocked( UInt32 workerIndex );

    /** Wakes every worker, with the lock held for the list; the semaphores are posted after */
    void WakeAllWorkers( void );

    private:

    mutable MT::CMutex m_lock;
    CORE::CString m_poolName;
    UInt32 m_workerThreadCount;
    bool m_workersCanAccessRenderSystem;
    UInt32 m_mainThreadBudgetInMs;

    bool m_running;
    bool m_shuttingDown;
    bool m_stopWorkers;
    bool m_paused;
    bool m_acceptTasks;
    bool m_runOnCallingThread;

    TOgreTaskList m_tasks;
    TTickList m_taskQueuedTicks;
    TOgreTaskList m_mainThreadTasks;
    MT::CTAtomic< UInt32 > m_mainThreadTaskCount;   /**< read without the lock: an empty list costs no lock per frame */
    UInt32 m_runningTasks;

    CORE::ThreadPoolPtr m_threadPool;
    TFutureVector m_workers;                        /**< the futures of the worker tasks */
    TSemaphoreVector m_workerWakeUps;               /**< the wake up of each worker, made at the start */
    TBoolVector m_workerIdle;                       /**< which worker is on the idle list */
    TWorkerIndexVector m_idleWorkers;
    UInt32 m_runningWorkers;
    MT::CSemaphore m_workerStarted;                 /**< posted by each worker when it is ready (and registered) */

    Ogre::RenderSystem* m_renderSystem;
    TThreadIdSet m_registeredThreads;

    UInt64 m_tasksQueued;
    UInt64 m_tasksRun;
    UInt64 m_tasksFailed;
    UInt64 m_mainThreadTasksRun;
    UInt64 m_mainThreadTasksFailed;
    UInt64 m_tasksDropped;
    UInt64 m_queueTicks;
    UInt64 m_runTicks;
};

/*-------------------------------------------------------------------------*/

typedef COgreWorkQueueCore::TOgreWorkQueueCorePtr   COgreWorkQueueCorePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLUGINGLUE_OGRE_COGREWORKQUEUECORE_H ? */
