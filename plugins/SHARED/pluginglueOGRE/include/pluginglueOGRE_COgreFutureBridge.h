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

#ifndef GUCEF_PLUGINGLUE_OGRE_COGREFUTUREBRIDGE_H
#define GUCEF_PLUGINGLUE_OGRE_COGREFUTUREBRIDGE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "OgreResource.h"

#ifndef GUCEF_CORE_CNOTIFIER_H
#include "CNotifier.h"
#define GUCEF_CORE_CNOTIFIER_H
#endif /* GUCEF_CORE_CNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_COGRERESOURCEREQUEST_H
#include "pluginglueOGRE_COgreResourceRequest.h"
#define GUCEF_PLUGINGLUE_OGRE_COGRERESOURCEREQUEST_H
#endif /* GUCEF_PLUGINGLUE_OGRE_COGRERESOURCEREQUEST_H ? */

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
 *  Ogre's background preparation and loading of resources, with the GUCEF conventions for asynchronous work: the
 *  asynchronous form returns a CORE::CFutureResult, the synchronous form does the same work on the calling thread.
 *
 *  The work goes through the work queue of the Root (COgreTaskWorkQueue on a GUCEF thread pool, or whatever queue the
 *  Root has): the files are read and decoded on a worker thread, the GPU objects are made in
 *  WorkQueue::processMainThreadTasks() on the render thread, which the Root calls at the end of each frame
 *  (Root::_fireFrameEnded()). The future is completed there, after the resource is loaded, and RequestFinishedEvent is
 *  notified on that thread with the task id of the future (CFutureResult::GetTaskId()).
 *
 *  So the render thread must not wait for such a future: it completes it. The render thread uses the synchronous
 *  form, other threads may wait (CFutureResult::Await()) or subscribe to the event. A failure ends the future with
 *  TASKSTATUS_EXCEPTION_DURING_RUN and the reason as the extra info of the task; a cancellation asked for with
 *  CFutureResult::RequestCancellation() is honoured between the two parts; a request that the work queue drops
 *  unrun (shut down, aborted) ends cancelled, without the event.
 *
 *  Applications that want a std::future keep using Ogre's own ResourceBackgroundQueue; its futures are ready when the
 *  background part is done, before the main thread part has run.
 */
class GUCEF_PLUGINGLUE_OGRE_EXPORT_CPP COgreFutureBridge : public CORE::CNotifier ,
                                                          public CORE::CTSharedObjCreator< COgreFutureBridge, MT::CMutex >
{
    public:

    static const CORE::CEvent RequestFinishedEvent;
    typedef CORE::TCloneableUInt32 TRequestFinishedEventData;   /**< the task id of the future of the request */

    static void RegisterEvents( void );

    typedef CORE::CTSharedObjCreator< COgreFutureBridge, MT::CMutex >::TSharedPtrType     TOgreFutureBridgePtr;

    COgreFutureBridge( void );

    virtual ~COgreFutureBridge() GUCEF_VIRTUAL_OVERRIDE;

    /** Prepares the resources of a group in the background (ResourceGroupManager::prepareResourceGroup()) */
    CORE::CFutureResult PrepareResourceGroupAsync( const CORE::CString& groupName );

    bool PrepareResourceGroup( const CORE::CString& groupName );

    /** Prepares the resources of a group in the background, then loads them on the render thread */
    CORE::CFutureResult LoadResourceGroupAsync( const CORE::CString& groupName );

    bool LoadResourceGroup( const CORE::CString& groupName );

    /** Prepares a resource in the background (Resource::prepare()), then tells its listeners on the render thread */
    CORE::CFutureResult PrepareResourceAsync( const Ogre::ResourcePtr& resource );

    bool PrepareResource( const Ogre::ResourcePtr& resource );

    /** Prepares a resource in the background, then loads it on the render thread (Resource::load()) */
    CORE::CFutureResult LoadResourceAsync( const Ogre::ResourcePtr& resource );

    bool LoadResource( const Ogre::ResourcePtr& resource );

    /** Requests of the bridge whose future ended, in total */
    UInt64 GetNrOfFinishedRequests( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    friend class COgreResourceRequest;

    /** Called by a request on the render thread when its future ended */
    void OnRequestFinished( UInt32 futureTaskId );

    CORE::CFutureResult StartRequest( COgreResourceRequest::TRequestType type, const CORE::CString& groupName, const Ogre::ResourcePtr& resource );

    bool RunRequest( COgreResourceRequest::TRequestType type, const CORE::CString& groupName, const Ogre::ResourcePtr& resource );

    COgreFutureBridge( const COgreFutureBridge& src );              /**< not implemented */
    COgreFutureBridge& operator=( const COgreFutureBridge& src );   /**< not implemented */

    private:

    mutable MT::CMutex m_lock;
    UInt64 m_finishedRequests;
};

/*-------------------------------------------------------------------------*/

typedef COgreFutureBridge::TOgreFutureBridgePtr     COgreFutureBridgePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLUGINGLUE_OGRE_COGREFUTUREBRIDGE_H ? */
