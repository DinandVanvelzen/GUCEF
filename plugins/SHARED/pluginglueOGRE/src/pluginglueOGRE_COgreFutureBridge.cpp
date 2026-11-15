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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "pluginglueOGRE_COgreFutureBridge.h"

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
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent COgreFutureBridge::RequestFinishedEvent = "GUCEF::PLUGINGLUE::OGRE::COgreFutureBridge::RequestFinishedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
COgreFutureBridge::RegisterEvents( void )
{GUCEF_TRACE;

    RequestFinishedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

COgreFutureBridge::COgreFutureBridge( void )
    : CORE::CNotifier()
    , CORE::CTSharedObjCreator< COgreFutureBridge, MT::CMutex >( this )
    , m_lock()
    , m_finishedRequests( 0 )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

COgreFutureBridge::~COgreFutureBridge()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
COgreFutureBridge::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::PLUGINGLUE::OGRE::COgreFutureBridge";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
COgreFutureBridge::StartRequest( COgreResourceRequest::TRequestType type, const CORE::CString& groupName, const Ogre::ResourcePtr& resource )
{GUCEF_TRACE;

    COgreResourceRequestPtr request = COgreResourceRequest::CreateSharedObj();
    if ( request.IsNULL() )
        return CORE::CFutureResult( CORE::TASKSTATUS_SETUP_FAILED );
    request->Setup( type, groupName, resource, CreateSharedPtr() );

    CORE::CFutureResult future = request->GetFuture();
    if ( !COgreResourceRequest::Queue( request ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreFutureBridge: There is no Ogre work queue that takes tasks, the request for "
            << request->GetName() << " is not made" );
        request->Finish( CORE::TASKSTATUS_QUEUEING_FAILED, "There is no Ogre work queue that takes tasks" );
    }
    return future;
}

/*-------------------------------------------------------------------------*/

bool
COgreFutureBridge::RunRequest( COgreResourceRequest::TRequestType type, const CORE::CString& groupName, const Ogre::ResourcePtr& resource )
{GUCEF_TRACE;

    // the same parts as the asynchronous form, on the calling thread
    CORE::CString error;
    if ( COgreResourceRequest::RunPart( type, true, groupName, resource, error ) &&
         COgreResourceRequest::RunPart( type, false, groupName, resource, error )  )
        return true;

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreFutureBridge: The request for " << ( resource ? CORE::CString( resource->getName().c_str() ) : groupName )
        << " did not succeed: " << error );
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
COgreFutureBridge::PrepareResourceGroupAsync( const CORE::CString& groupName )
{GUCEF_TRACE;

    return StartRequest( COgreResourceRequest::REQUESTTYPE_PREPARE_RESOURCE_GROUP, groupName, Ogre::ResourcePtr() );
}

/*-------------------------------------------------------------------------*/

bool
COgreFutureBridge::PrepareResourceGroup( const CORE::CString& groupName )
{GUCEF_TRACE;

    return RunRequest( COgreResourceRequest::REQUESTTYPE_PREPARE_RESOURCE_GROUP, groupName, Ogre::ResourcePtr() );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
COgreFutureBridge::LoadResourceGroupAsync( const CORE::CString& groupName )
{GUCEF_TRACE;

    return StartRequest( COgreResourceRequest::REQUESTTYPE_LOAD_RESOURCE_GROUP, groupName, Ogre::ResourcePtr() );
}

/*-------------------------------------------------------------------------*/

bool
COgreFutureBridge::LoadResourceGroup( const CORE::CString& groupName )
{GUCEF_TRACE;

    return RunRequest( COgreResourceRequest::REQUESTTYPE_LOAD_RESOURCE_GROUP, groupName, Ogre::ResourcePtr() );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
COgreFutureBridge::PrepareResourceAsync( const Ogre::ResourcePtr& resource )
{GUCEF_TRACE;

    return StartRequest( COgreResourceRequest::REQUESTTYPE_PREPARE_RESOURCE, CORE::CString(), resource );
}

/*-------------------------------------------------------------------------*/

bool
COgreFutureBridge::PrepareResource( const Ogre::ResourcePtr& resource )
{GUCEF_TRACE;

    return RunRequest( COgreResourceRequest::REQUESTTYPE_PREPARE_RESOURCE, CORE::CString(), resource );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
COgreFutureBridge::LoadResourceAsync( const Ogre::ResourcePtr& resource )
{GUCEF_TRACE;

    return StartRequest( COgreResourceRequest::REQUESTTYPE_LOAD_RESOURCE, CORE::CString(), resource );
}

/*-------------------------------------------------------------------------*/

bool
COgreFutureBridge::LoadResource( const Ogre::ResourcePtr& resource )
{GUCEF_TRACE;

    return RunRequest( COgreResourceRequest::REQUESTTYPE_LOAD_RESOURCE, CORE::CString(), resource );
}

/*-------------------------------------------------------------------------*/

UInt64
COgreFutureBridge::GetNrOfFinishedRequests( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_finishedRequests;
}

/*-------------------------------------------------------------------------*/

void
COgreFutureBridge::OnRequestFinished( UInt32 futureTaskId )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        ++m_finishedRequests;
    }
    TRequestFinishedEventData eventData( futureTaskId );
    NotifyObservers( RequestFinishedEvent, &eventData );
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
