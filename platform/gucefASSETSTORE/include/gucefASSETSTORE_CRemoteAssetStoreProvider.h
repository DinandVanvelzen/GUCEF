/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CREMOTEASSETSTOREPROVIDER_H
#define GUCEF_ASSETSTORE_CREMOTEASSETSTOREPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H
#include "gucefASSETSTORE_CIAssetStoreProvider.h"
#define GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CIAssetStoreService;

/**
 *  An online store of a remote asset store service as a provider (the handle CIAssetStoreService::GetStoreProvider()
 *  gives, for the browsing of its library through CAssetStoreLibraryFileSystemAccess): the library and the items are
 *  calls of the service, the store's session stays on the server. Downloads are made by the server
 *  (CIAssetStoreService::EnqueueDownload()), so PlanDownload() answers UNSUPPORTED. Owned by the remote service, not
 *  registered in the global registry, so a process can have local store plugins and a remote service side by side. When
 *  the service closes it lets go of its providers: their calls answer UNAVAILABLE from then on.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CRemoteAssetStoreProvider : public CIAssetStoreProvider
{
    public:

    CRemoteAssetStoreProvider( CIAssetStoreService* service, const CORE::CString& storeId, const CORE::CString& displayName, bool signedIn );

    virtual ~CRemoteAssetStoreProvider() GUCEF_VIRTUAL_OVERRIDE;

    /** What the service last said of the store */
    void Update( const CORE::CString& displayName, bool signedIn );

    /** The service goes: calls answer UNAVAILABLE from now on */
    void Detach( void );

    virtual const CORE::CString& GetProviderId( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetDisplayName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsSignedIn( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual TAssetStoreStatus QueryLibrary( const CAssetStoreQuery& query, CAssetStoreLibraryPage& page ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TAssetStoreStatus GetItem( const CORE::CString& itemId, CAssetStoreItem& item ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TAssetStoreStatus PlanDownload( const CORE::CString& itemId              ,
                                            const CORE::CString& formatId            ,
                                            const CORE::CString::StringVector& fileIds,
                                            CAssetStoreDownloadPlan& plan            ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CRemoteAssetStoreProvider( const CRemoteAssetStoreProvider& src );              /**< not implemented */
    CRemoteAssetStoreProvider& operator=( const CRemoteAssetStoreProvider& src );   /**< not implemented */

    CIAssetStoreService* m_service;
    CORE::CString m_storeId;
    CORE::CString m_displayName;
    bool m_signedIn;
    mutable MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CREMOTEASSETSTOREPROVIDER_H ? */
