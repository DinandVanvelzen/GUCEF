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

#ifndef GUCEF_PLUGINGLUE_OGRE_COGRETASKWORKQUEUE_H
#define GUCEF_PLUGINGLUE_OGRE_COGRETASKWORKQUEUE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "OgreWorkQueue.h"

#ifndef GUCEF_PLUGINGLUE_OGRE_COGREWORKQUEUECORE_H
#include "pluginglueOGRE_COgreWorkQueueCore.h"
#define GUCEF_PLUGINGLUE_OGRE_COGREWORKQUEUECORE_H
#endif /* GUCEF_PLUGINGLUE_OGRE_COGREWORKQUEUECORE_H ? */

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
 *  Ogre's work queue on a GUCEF thread pool: the background tasks of Ogre (resource preparation through
 *  ResourceBackgroundQueue, and every other WorkQueue::addTask()) run on worker tasks of a thread pool of the task
 *  manager, one per worker thread, so the worker threads are threads of the task manager (named after the pool,
 *  listed, stopped and measured by it). Ogre makes no threads of its own for it. The Ogre tasks are counted by the
 *  queue (COgreWorkQueueCore: queued, running, run, failed, dropped, time in the list and running).
 *
 *  The queue is installed in the Root with Root::setWorkQueue(), which gives the Root its ownership; COgreRootSetup
 *  does that when it makes the Root. The Root starts it with the first render window (Root::oneTimePostWindowInit())
 *  and shuts it down first in Root::shutdown(). Tasks added before the start wait for it.
 *
 *  The work queue contract of Ogre 14 holds, with these choices:
 *  - Background tasks start in the order they were added; with more than one worker thread they may end in another.
 *  - Main thread tasks run in processMainThreadTasks() on the calling thread, which the Root calls in
 *    Root::_fireFrameEnded(), within the time limit (setMainThreadProcessingTimeLimit(), 10 ms by default).
 *  - Without worker threads (setWorkerThreadCount( 0 ), and always when Ogre is built with OGRE_THREAD_SUPPORT 0, as
 *    for the Web) addTask() runs the task at once on the calling thread, as Ogre's own queue does without threads.
 *    Ogre code that waits for the future of a task right after adding it (the Terrain component does) relies on that.
 *  - An exception that escapes a task is caught, logged and counted; Ogre's own queue ends the process for one of a
 *    background task and passes one of a main thread task on to the caller of processMainThreadTasks().
 *  - setPaused( true ) holds background tasks that did not start until setPaused( false ).
 *  - Idle workers sleep on their wake up semaphore; adding a task wakes one idle worker, busy workers take the next
 *    task without sleeping. A sleeping worker looks at the stop request of its thread pool once per second.
 *  - shutdown() ends the workers after the task each one runs (so no task runs Ogre code once the Root goes on with
 *    its shut down; they unregister from the render system as they end) and drops the tasks that did not run. Ogre's own queue keeps them for a restart; dropping them releases what they hold (resources,
 *    packaged tasks, which report a broken promise to their futures) while the Root and its managers still exist.
 *  - AbortQueuedTasks() drops the background tasks that did not start.
 *  - There is no priority: neither Ogre's work queue nor the GUCEF thread pool has one.
 */
class GUCEF_PLUGINGLUE_OGRE_EXPORT_CPP COgreTaskWorkQueue : public Ogre::WorkQueue
{
    public:

    COgreTaskWorkQueue( void );

    virtual ~COgreTaskWorkQueue() GUCEF_VIRTUAL_OVERRIDE;

    /** The minimal number of worker threads of the thread pool while the queue runs; 0 runs tasks on the calling thread */
    virtual size_t getWorkerThreadCount( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void setWorkerThreadCount( size_t threadCount ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void startup( bool forceRestart = true ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void addTask( std::function< void() > task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void setPaused( bool pause ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool isPaused( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void setRequestsAccepted( bool accept ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool getRequestsAccepted( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void processMainThreadTasks( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual unsigned long getResponseProcessingTimeLimit( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void setResponseProcessingTimeLimit( unsigned long ms ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void addMainThreadTask( std::function< void() > task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void shutdown( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** The thread pool of the task manager that runs the tasks, made when it is not there. Used from the next start */
    void SetThreadPoolName( const CORE::CString& poolName );

    CORE::CString GetThreadPoolName( void ) const;

    /** See COgreWorkQueueCore::SetWorkersCanAccessRenderSystem(). Used from the next start */
    void SetWorkersCanAccessRenderSystem( bool canAccess );

    bool GetWorkersCanAccessRenderSystem( void ) const;

    bool IsRunning( void ) const;

    /** True when background tasks run on the thread that adds them */
    bool IsRunningTasksOnCallingThread( void ) const;

    /** Drops the background tasks that did not start. Returns how many */
    UInt32 AbortQueuedTasks( void );

    /** The thread pool while the queue runs on one, otherwise NULL */
    CORE::ThreadPoolPtr GetThreadPool( void ) const;

    /** The state and counters shared with the tasks on the thread pool */
    const COgreWorkQueueCorePtr& GetCore( void ) const;

    private:

    COgreTaskWorkQueue( const COgreTaskWorkQueue& src );              /**< not implemented */
    COgreTaskWorkQueue& operator=( const COgreTaskWorkQueue& src );   /**< not implemented */

    private:

    COgreWorkQueueCorePtr m_core;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLUGINGLUE_OGRE_COGRETASKWORKQUEUE_H ? */
