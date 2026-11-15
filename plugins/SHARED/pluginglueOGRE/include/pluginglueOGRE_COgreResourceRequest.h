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

#ifndef GUCEF_PLUGINGLUE_OGRE_COGRERESOURCEREQUEST_H
#define GUCEF_PLUGINGLUE_OGRE_COGRERESOURCEREQUEST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "OgreResource.h"

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_MACROS_H
#include "pluginglueOGRE_macros.h"
#define GUCEF_PLUGINGLUE_OGRE_MACROS_H
#endif /* GUCEF_PLUGINGLUE_OGRE_MACROS_H ? */

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

class COgreFutureBridge;

/**
 *  One preparation or load of an Ogre resource or resource group that COgreFutureBridge carries out through Ogre's work
 *  queue, with the GUCEF future that tells when it is done.
 *
 *  The work has two parts, as in Ogre's ResourceBackgroundQueue: a background part (the files are read and decoded:
 *  ResourceGroupManager::prepareResourceGroup(), Resource::prepare()) on a worker thread, and a main thread part (the
 *  GPU objects are made: ResourceGroupManager::loadResourceGroup(), Resource::load(), and the listeners of the resource
 *  are told) in WorkQueue::processMainThreadTasks() on the render thread. The future is completed on the render thread,
 *  after the main thread part. When Ogre is built with OGRE_THREAD_SUPPORT 1 the background part loads as well.
 *
 *  The synchronous forms of the bridge run the same parts with RunPart() on the calling thread.
 *
 *  The work queue holds the request through a shared pointer in its tasks. When the queue drops them unrun (it was shut
 *  down, or the tasks were aborted) the request goes without having been completed, and its future ends cancelled.
 */
class GUCEF_PLUGINGLUE_OGRE_EXPORT_CPP COgreResourceRequest : public CORE::CTSharedObjCreator< COgreResourceRequest, MT::CMutex >
{
    public:

    enum ERequestType
    {
        REQUESTTYPE_PREPARE_RESOURCE_GROUP = 0,
        REQUESTTYPE_LOAD_RESOURCE_GROUP,
        REQUESTTYPE_PREPARE_RESOURCE,
        REQUESTTYPE_LOAD_RESOURCE
    };
    typedef enum ERequestType TRequestType;

    typedef CORE::CTSharedObjCreator< COgreResourceRequest, MT::CMutex >::TSharedPtrType   TOgreResourceRequestPtr;
    typedef CORE::CTSharedPtr< COgreFutureBridge, MT::CMutex >                              TOgreFutureBridgePtr;

    COgreResourceRequest( void );

    virtual ~COgreResourceRequest();

    /** What to do, with the group name or the resource it is for, and the bridge that is told when it ends */
    void Setup( TRequestType type, const CORE::CString& groupName, const Ogre::ResourcePtr& resource, const TOgreFutureBridgePtr& bridge );

    TRequestType GetType( void ) const;

    CORE::CFutureResult GetFuture( void ) const;

    /** The name of the resource or the resource group, for messages */
    CORE::CString GetName( void ) const;

    /** Runs the background part (background true) or the main thread part of a request on the calling thread. False with the reason when it failed */
    static bool RunPart( TRequestType type, bool background, const CORE::CString& groupName, const Ogre::ResourcePtr& resource, CORE::CString& error );

    /** Gives the background part of the request to Ogre's work queue. False when there is no queue that takes tasks */
    static bool Queue( const TOgreResourceRequestPtr& request );

    /** The background part, run by the work queue; it queues the main thread part */
    void RunBackgroundPart( void );

    /** The main thread part, run by the work queue on the render thread; it completes the future */
    void RunMainThreadPart( bool backgroundSucceeded, const CORE::CString& backgroundError );

    /** Completes the future and tells the bridge, on the thread that runs main thread tasks. False when it was completed already */
    bool Finish( CORE::TTaskStatus status, const CORE::CString& info );

    private:

    COgreResourceRequest( const COgreResourceRequest& src );              /**< not implemented */
    COgreResourceRequest& operator=( const COgreResourceRequest& src );   /**< not implemented */

    private:

    mutable MT::CMutex m_lock;
    TRequestType m_type;
    CORE::CString m_groupName;
    Ogre::ResourcePtr m_resource;
    TOgreFutureBridgePtr m_bridge;
    CORE::CFutureResultSource m_source;
};

/*-------------------------------------------------------------------------*/

typedef COgreResourceRequest::TOgreResourceRequestPtr   COgreResourceRequestPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLUGINGLUE_OGRE_COGRERESOURCEREQUEST_H ? */
