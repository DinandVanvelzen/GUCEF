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

#include "OgreRoot.h"
#include "OgreWorkQueue.h"
#include "OgreResourceGroupManager.h"
#include "OgreException.h"

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_COGREFUTUREBRIDGE_H
#include "pluginglueOGRE_COgreFutureBridge.h"
#define GUCEF_PLUGINGLUE_OGRE_COGREFUTUREBRIDGE_H
#endif /* GUCEF_PLUGINGLUE_OGRE_COGREFUTUREBRIDGE_H ? */

#include "pluginglueOGRE_COgreResourceRequest.h"

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The background part of a request as a task of Ogre's work queue (a function object: the queue copies it) */
class CBackgroundPart
{
    public:

    explicit CBackgroundPart( const COgreResourceRequestPtr& request ) : m_request( request ) {}

    void operator()( void ) { m_request->RunBackgroundPart(); }

    private:

    COgreResourceRequestPtr m_request;
};

/** The main thread part of a request as a task of Ogre's work queue */
class CMainThreadPart
{
    public:

    CMainThreadPart( const COgreResourceRequestPtr& request, bool backgroundSucceeded, const CORE::CString& backgroundError )
        : m_request( request ), m_backgroundSucceeded( backgroundSucceeded ), m_backgroundError( backgroundError ) {}

    void operator()( void ) { m_request->RunMainThreadPart( m_backgroundSucceeded, m_backgroundError ); }

    private:

    COgreResourceRequestPtr m_request;
    bool m_backgroundSucceeded;
    CORE::CString m_backgroundError;
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

COgreResourceRequest::COgreResourceRequest( void )
    : CORE::CTSharedObjCreator< COgreResourceRequest, MT::CMutex >( this )
    , m_lock()
    , m_type( REQUESTTYPE_LOAD_RESOURCE )
    , m_groupName()
    , m_resource()
    , m_bridge()
    , m_source( CORE::TASKSTATUS_QUEUED )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

COgreResourceRequest::~COgreResourceRequest()
{GUCEF_TRACE;

    // the work queue dropped the request before it was done: whoever waits for it is not left waiting
    if ( !m_source.IsCompleted() )
    {
        m_source.Complete( CORE::TASKSTATUS_CANCELLED, "The Ogre work queue dropped the request before it was done" );
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "OgreResourceRequest: The request for " << GetName() << " was dropped unfinished, its future is cancelled" );
    }
}

/*-------------------------------------------------------------------------*/

void
COgreResourceRequest::Setup( TRequestType type, const CORE::CString& groupName, const Ogre::ResourcePtr& resource, const TOgreFutureBridgePtr& bridge )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_type = type;
    m_groupName = groupName;
    m_resource = resource;
    m_bridge = bridge;
}

/*-------------------------------------------------------------------------*/

COgreResourceRequest::TRequestType
COgreResourceRequest::GetType( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_type;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
COgreResourceRequest::GetFuture( void ) const
{GUCEF_TRACE;

    return m_source.GetFuture();
}

/*-------------------------------------------------------------------------*/

CORE::CString
COgreResourceRequest::GetName( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_resource )
        return CORE::CString( m_resource->getName().c_str() );
    return m_groupName;
}

/*-------------------------------------------------------------------------*/

bool
COgreResourceRequest::RunPart( TRequestType type, bool background, const CORE::CString& groupName, const Ogre::ResourcePtr& resource, CORE::CString& error )
{GUCEF_TRACE;

    try
    {
        const bool resourceRequest = REQUESTTYPE_PREPARE_RESOURCE == type || REQUESTTYPE_LOAD_RESOURCE == type;
        if ( resourceRequest && !resource )
        {
            error = "There is no resource";
            return false;
        }
        Ogre::ResourceGroupManager* groupManager = Ogre::ResourceGroupManager::getSingletonPtr();
        if ( !resourceRequest && GUCEF_NULL == groupManager )
        {
            error = "There is no Ogre resource group manager";
            return false;
        }

        switch ( type )
        {
            case REQUESTTYPE_PREPARE_RESOURCE_GROUP:
            {
                if ( background )
                    groupManager->prepareResourceGroup( groupName.C_String() );
                return true;
            }
            case REQUESTTYPE_LOAD_RESOURCE_GROUP:
            {
                // as ResourceBackgroundQueue: the files in the background, the GPU objects on the render thread,
                // unless Ogre is built to load in the background too
                #if OGRE_THREAD_SUPPORT == 1
                if ( background )
                    groupManager->loadResourceGroup( groupName.C_String() );
                #else
                if ( background )
                    groupManager->prepareResourceGroup( groupName.C_String() );
                else
                    groupManager->loadResourceGroup( groupName.C_String() );
                #endif
                return true;
            }
            case REQUESTTYPE_PREPARE_RESOURCE:
            {
                if ( background )
                {
                    resource->prepare( true );
                }
                else
                {
                    resource->_firePreparingComplete();
                }
                if ( !resource->isPrepared() && !resource->isLoaded() )
                {
                    error = "The resource is not prepared";
                    return false;
                }
                return true;
            }
            case REQUESTTYPE_LOAD_RESOURCE:
            {
                #if OGRE_THREAD_SUPPORT == 1
                if ( background )
                    resource->load( true );
                else
                    resource->_fireLoadingComplete();
                #else
                if ( background )
                {
                    resource->prepare( true );
                    if ( !resource->isPrepared() && !resource->isLoaded() )
                    {
                        error = "The resource is not prepared";
                        return false;
                    }
                    return true;
                }
                resource->load( true );
                resource->_fireLoadingComplete();
                #endif
                if ( !background && !resource->isLoaded() )
                {
                    error = "The resource is not loaded";
                    return false;
                }
                return true;
            }
        }
        error = "Unknown request";
        return false;
    }
    catch ( const Ogre::Exception& e )
    {
        error = e.getFullDescription().c_str();
    }
    catch ( const std::exception& e )
    {
        error = e.what();
    }
    catch ( ... )
    {
        error = "Unknown exception";
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
COgreResourceRequest::Queue( const COgreResourceRequestPtr& request )
{GUCEF_TRACE;

    Ogre::Root* root = Ogre::Root::getSingletonPtr();
    Ogre::WorkQueue* workQueue = GUCEF_NULL != root ? root->getWorkQueue() : GUCEF_NULL;
    if ( GUCEF_NULL == workQueue || !workQueue->getRequestsAccepted() )
        return false;

    // a queue that drops the task (shut down) releases the request with it, which cancels the future
    COgreResourceRequestPtr queued( request );
    queued->m_source.SetStatus( CORE::TASKSTATUS_QUEUED );
    workQueue->addTask( CBackgroundPart( queued ) );
    return true;
}

/*-------------------------------------------------------------------------*/

void
COgreResourceRequest::RunBackgroundPart( void )
{GUCEF_TRACE;

    TRequestType type;
    CORE::CString groupName;
    Ogre::ResourcePtr resource;
    {
        MT::CScopeMutex lock( m_lock );
        type = m_type;
        groupName = m_groupName;
        resource = m_resource;
    }

    bool success = false;
    CORE::CString error;
    if ( m_source.IsCancellationRequested() )
    {
        error = "Cancelled";
    }
    else
    {
        m_source.SetStatus( CORE::TASKSTATUS_RUNNING );
        success = RunPart( type, true, groupName, resource, error );
    }

    // the future is completed on the render thread, also when this part failed
    Ogre::Root* root = Ogre::Root::getSingletonPtr();
    Ogre::WorkQueue* workQueue = GUCEF_NULL != root ? root->getWorkQueue() : GUCEF_NULL;
    if ( GUCEF_NULL != workQueue )
        workQueue->addMainThreadTask( CMainThreadPart( CreateSharedPtr(), success, error ) );
}

/*-------------------------------------------------------------------------*/

void
COgreResourceRequest::RunMainThreadPart( bool backgroundSucceeded, const CORE::CString& backgroundError )
{GUCEF_TRACE;

    if ( !backgroundSucceeded )
    {
        if ( m_source.IsCancellationRequested() )
            Finish( CORE::TASKSTATUS_CANCELLED, "Cancelled" );
        else
            Finish( CORE::TASKSTATUS_EXCEPTION_DURING_RUN, backgroundError );
        return;
    }
    if ( m_source.IsCancellationRequested() )
    {
        Finish( CORE::TASKSTATUS_CANCELLED, "Cancelled" );
        return;
    }

    TRequestType type;
    CORE::CString groupName;
    Ogre::ResourcePtr resource;
    {
        MT::CScopeMutex lock( m_lock );
        type = m_type;
        groupName = m_groupName;
        resource = m_resource;
    }
    CORE::CString error;
    if ( RunPart( type, false, groupName, resource, error ) )
        Finish( CORE::TASKSTATUS_FINISHED, CORE::CString() );
    else
        Finish( CORE::TASKSTATUS_EXCEPTION_DURING_RUN, error );
}

/*-------------------------------------------------------------------------*/

bool
COgreResourceRequest::Finish( CORE::TTaskStatus status, const CORE::CString& info )
{GUCEF_TRACE;

    bool ended = false;
    if ( CORE::TASKSTATUS_FINISHED == status )
        ended = m_source.Complete();
    else
    if ( CORE::TASKSTATUS_CANCELLED == status )
        ended = m_source.Complete( CORE::TASKSTATUS_CANCELLED, info );
    else
        ended = m_source.Fail( status, info );

    if ( !ended )
        return false;

    if ( CORE::TASKSTATUS_FINISHED != status )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreResourceRequest: The request for " << GetName() << " did not succeed: " << info );
    }

    TOgreFutureBridgePtr bridge;
    {
        MT::CScopeMutex lock( m_lock );
        bridge = m_bridge;
    }
    if ( !bridge.IsNULL() )
        bridge->OnRequestFinished( m_source.GetFuture().GetTaskId() );
    return true;
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
