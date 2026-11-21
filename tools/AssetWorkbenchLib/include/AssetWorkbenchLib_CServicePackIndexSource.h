/*
 *  AssetWorkbenchLib: the library of the asset workbench
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


#ifndef ASSETWORKBENCHLIB_CSERVICEPACKINDEXSOURCE_H
#define ASSETWORKBENCHLIB_CSERVICEPACKINDEXSOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#include "gucefASSETSTORE_CIAssetStoreService.h"
#define GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H ? */

#ifndef ASSETWORKBENCHLIB_IPACKINDEXSOURCE_H
#include "AssetWorkbenchLib_IPackIndexSource.h"
#define ASSETWORKBENCHLIB_IPACKINDEXSOURCE_H
#endif /* ASSETWORKBENCHLIB_IPACKINDEXSOURCE_H ? */

#ifndef ASSETWORKBENCHLIB_CPACKINDEXCLIENTCACHE_H
#include "AssetWorkbenchLib_CPackIndexClientCache.h"
#define ASSETWORKBENCHLIB_CPACKINDEXCLIENTCACHE_H
#endif /* ASSETWORKBENCHLIB_CPACKINDEXCLIENTCACHE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The pack index through the asset store service behind IPackIndexSource (section 6.3 of the pack index design): the
 *  operations GetPackIndex(), GetResourceGroups() and GetResources() of a CIAssetStoreService, which the service of this
 *  process (CLocalAssetStoreService) and the proxy of a remote one (CRemoteAssetStoreService) answer the same, so this source
 *  serves both. IsCheap() is the IsInProcess() of the service: a call to a remote service waits for the network and is not
 *  made on the thread of a GUI.
 *
 *  Caches (CPackIndexClientCache): the manifest of a pack is asked once and kept, in memory and on the disk when a folder is set;
 *  the change journal of the service tells when it may have changed (PACK_INDEX_UPDATED, PACK_UPDATED, PACK_REMOVED, RESYNC):
 *  the manifest is then asked again and kept when its index key is the same. The last pages of resources are kept per
 *  (pack, group, sub-group, page) and go with the events of their pack. A service that cannot be reached answers the manifest
 *  of the cache (memory, then disk) and says so (IsServingFromCache()): a pack opened offline shows what was known of it.
 *
 *  The flag learning makes the source a machine learning consumer: the service leaves the packs with the licence flag
 *  noMachineLearning out for it. A remote host decides that for the devices of its learning roles whatever is set here.
 *  The events of the service are followed on the thread the service raises them on. Thread safe.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CServicePackIndexSource : public IPackIndexSource, public CORE::CObservingNotifier
{
    public:

    /** The service is not owned and has to outlive the source */
    explicit CServicePackIndexSource( ASSETSTORE::CIAssetStoreService& service );
    virtual ~CServicePackIndexSource() GUCEF_VIRTUAL_OVERRIDE;

    /** The folder of the manifests on the disk (VFS); empty for none */
    void SetDiskCacheFolder( const CORE::CString& vfsFolder );

    /** Whether the source is a machine learning consumer (see above); false by default */
    void SetLearning( bool learning );
    bool IsLearning( void ) const;

    const CPackIndexClientCache& GetCache( void ) const;

    /** The calls made to the service (what the caches saved is not in it) */
    UInt32 GetServiceCallCount( void ) const;

    /** Whether the last manifest came from the cache because the service could not be reached */
    bool IsServingFromCache( void ) const;

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsCheap( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual EResult GetManifest( const CORE::CString& packId, ASSETCONVERTER::CPackIndexDocument& manifest, bool& stale ) GUCEF_VIRTUAL_OVERRIDE;
    virtual EResult GetGroups( const CORE::CString& packId, ASSETCONVERTER::CPackIndexGroup::TGroupVector& groups ) GUCEF_VIRTUAL_OVERRIDE;
    virtual EResult GetResources( const CORE::CString& packId, const CORE::CString& groupId, const CORE::CString& subGroup,
                                  const CORE::CString& pageToken, UInt32 maxEntries,
                                  ASSETCONVERTER::CPackIndexResource::TResourceVector& page, CORE::CString& nextPageToken, UInt64& total ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CString GetLastMessage( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The record of the index of a pack as the service gives it: the members the catalogue holds */
    static void ResourceFromRecord( const ASSETSTORE::CAssetPackResource& record, ASSETCONVERTER::CPackIndexResource& resource );

    /** The group summaries of the service (function groups and their sub-groups, flat) as the groups of a manifest */
    static void GroupsFromRows( const ASSETSTORE::TAssetPackGroupVector& rows, ASSETCONVERTER::CPackIndexGroup::TGroupVector& groups );

    /** The document of the answer of the service: its manifest when it could be read, else what the catalogue holds of it */
    static void DocumentFromInfo( const ASSETSTORE::CAssetPackIndexInfo& info, ASSETCONVERTER::CPackIndexDocument& document );

    private:

    typedef CORE::CTEventHandlerFunctor< CServicePackIndexSource > TEventCallback;

    EResult Fail( EResult result, const CORE::CString& message );
    EResult ResultOf( ASSETSTORE::TAssetStoreStatus status, const CORE::CString& what );
    void OnCatalogueChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    private:

    ASSETSTORE::CIAssetStoreService& m_service;
    CPackIndexClientCache m_cache;
    bool m_learning;
    bool m_servingFromCache;
    UInt32 m_calls;
    CORE::CString m_lastMessage;
    mutable MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CSERVICEPACKINDEXSOURCE_H ? */
