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

#include "OgreRoot.h"

#include "pluginglueOGRE_COgreTaskWorkQueue.h"

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

COgreTaskWorkQueue::COgreTaskWorkQueue( void )
    : Ogre::WorkQueue()
    , m_core( COgreWorkQueueCore::CreateSharedObj() )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

COgreTaskWorkQueue::~COgreTaskWorkQueue()
{GUCEF_TRACE;

    // the Root shut the queue down already; a queue that was never given to a Root is shut down here
    m_core->Shutdown();
}

/*-------------------------------------------------------------------------*/

size_t
COgreTaskWorkQueue::getWorkerThreadCount( void ) const
{GUCEF_TRACE;

    return (size_t) m_core->GetWorkerThreadCount();
}

/*-------------------------------------------------------------------------*/

void
COgreTaskWorkQueue::setWorkerThreadCount( size_t threadCount )
{GUCEF_TRACE;

    m_core->SetWorkerThreadCount( (UInt32) threadCount );
}

/*-------------------------------------------------------------------------*/

void
COgreTaskWorkQueue::startup( bool forceRestart )
{GUCEF_TRACE;

    if ( m_core->IsRunning() )
    {
        if ( !forceRestart )
            return;
        m_core->Shutdown();
    }

    // the render system, for worker threads that may use it, is the one of an initialised Root
    Ogre::Root* root = Ogre::Root::getSingletonPtr();
    Ogre::RenderSystem* renderSystem = ( GUCEF_NULL != root && root->isInitialised() ) ? root->getRenderSystem() : GUCEF_NULL;
    m_core->Startup( renderSystem );
}

/*-------------------------------------------------------------------------*/

void
COgreTaskWorkQueue::addTask( std::function< void() > task )
{GUCEF_TRACE;

    m_core->AddTask( task );
}

/*-------------------------------------------------------------------------*/

void
COgreTaskWorkQueue::setPaused( bool pause )
{GUCEF_TRACE;

    m_core->SetPaused( pause );
}

/*-------------------------------------------------------------------------*/

bool
COgreTaskWorkQueue::isPaused( void ) const
{GUCEF_TRACE;

    return m_core->IsPaused();
}

/*-------------------------------------------------------------------------*/

void
COgreTaskWorkQueue::setRequestsAccepted( bool accept )
{GUCEF_TRACE;

    m_core->SetTasksAccepted( accept );
}

/*-------------------------------------------------------------------------*/

bool
COgreTaskWorkQueue::getRequestsAccepted( void ) const
{GUCEF_TRACE;

    return m_core->AreTasksAccepted();
}

/*-------------------------------------------------------------------------*/

void
COgreTaskWorkQueue::processMainThreadTasks( void )
{GUCEF_TRACE;

    m_core->ProcessMainThreadTasks();
}

/*-------------------------------------------------------------------------*/

unsigned long
COgreTaskWorkQueue::getResponseProcessingTimeLimit( void ) const
{GUCEF_TRACE;

    return (unsigned long) m_core->GetMainThreadBudgetInMs();
}

/*-------------------------------------------------------------------------*/

void
COgreTaskWorkQueue::setResponseProcessingTimeLimit( unsigned long ms )
{GUCEF_TRACE;

    m_core->SetMainThreadBudgetInMs( (UInt32) ms );
}

/*-------------------------------------------------------------------------*/

void
COgreTaskWorkQueue::addMainThreadTask( std::function< void() > task )
{GUCEF_TRACE;

    m_core->AddMainThreadTask( task );
}

/*-------------------------------------------------------------------------*/

void
COgreTaskWorkQueue::shutdown( void )
{GUCEF_TRACE;

    m_core->Shutdown();
}

/*-------------------------------------------------------------------------*/

void
COgreTaskWorkQueue::SetThreadPoolName( const CORE::CString& poolName )
{GUCEF_TRACE;

    m_core->SetThreadPoolName( poolName );
}

/*-------------------------------------------------------------------------*/

CORE::CString
COgreTaskWorkQueue::GetThreadPoolName( void ) const
{GUCEF_TRACE;

    return m_core->GetThreadPoolName();
}

/*-------------------------------------------------------------------------*/

void
COgreTaskWorkQueue::SetWorkersCanAccessRenderSystem( bool canAccess )
{GUCEF_TRACE;

    m_core->SetWorkersCanAccessRenderSystem( canAccess );
}

/*-------------------------------------------------------------------------*/

bool
COgreTaskWorkQueue::GetWorkersCanAccessRenderSystem( void ) const
{GUCEF_TRACE;

    return m_core->GetWorkersCanAccessRenderSystem();
}

/*-------------------------------------------------------------------------*/

bool
COgreTaskWorkQueue::IsRunning( void ) const
{GUCEF_TRACE;

    return m_core->IsRunning();
}

/*-------------------------------------------------------------------------*/

bool
COgreTaskWorkQueue::IsRunningTasksOnCallingThread( void ) const
{GUCEF_TRACE;

    return m_core->IsRunningTasksOnCallingThread();
}

/*-------------------------------------------------------------------------*/

UInt32
COgreTaskWorkQueue::AbortQueuedTasks( void )
{GUCEF_TRACE;

    return m_core->AbortQueuedTasks();
}

/*-------------------------------------------------------------------------*/

CORE::ThreadPoolPtr
COgreTaskWorkQueue::GetThreadPool( void ) const
{GUCEF_TRACE;

    return m_core->GetThreadPool();
}

/*-------------------------------------------------------------------------*/

const COgreWorkQueueCorePtr&
COgreTaskWorkQueue::GetCore( void ) const
{GUCEF_TRACE;

    return m_core;
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
