/*
 *  AssetWorkbenchLib: the asset sources of the workbench: online stores, the primary store and the transfers
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
#ifndef ASSETWORKBENCHLIB_CASSETSTORETHUMBNAILPROVIDER_H
#define ASSETWORKBENCHLIB_CASSETSTORETHUMBNAILPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_GUI_CITHUMBNAILPROVIDER_H
#include "gucefGUI_CIThumbnailProvider.h"
#define GUCEF_GUI_CITHUMBNAILPROVIDER_H
#endif /* GUCEF_GUI_CITHUMBNAILPROVIDER_H ? */

#ifndef GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#include "gucefASSETSTORE_CIAssetStoreService.h"
#define GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTORELIBRARYFILESYSTEMACCESS_H
#include "gucefASSETSTORE_CAssetStoreLibraryFileSystemAccess.h"
#define GUCEF_ASSETSTORE_CASSETSTORELIBRARYFILESYSTEMACCESS_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORELIBRARYFILESYSTEMACCESS_H ? */

#ifndef ASSETWORKBENCHLIB_MACROS_H
#include "AssetWorkbenchLib_macros.h"
#define ASSETWORKBENCHLIB_MACROS_H
#endif /* ASSETWORKBENCHLIB_MACROS_H ? */

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
 *  The thumbnails of the items of an online store for a CContentBrowserForm that browses the library of the store through
 *  ASSETSTORE::CAssetStoreLibraryFileSystemAccess: the picture of an item is the listing picture that the store gives, through the
 *  asset store service (ASSETSTORE::CIAssetStoreService::RequestListingImage(); the in-process service keeps the pictures in its
 *  image cache by their address, a remote one fetches them on the server). The form asks for the thumbnail of the path of an item;
 *  the answer comes with ThumbnailReadyEvent on the thread of the events of the service, which is the GUI thread of the workbench. A
 *  folder, a path that is not an item and an item without a picture are answered at once without a picture. The service and the
 *  file system access are not owned and have to outlive the provider.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CAssetStoreThumbnailProvider : public GUI::CIThumbnailProvider
{
    public:

    typedef CORE::CTEventHandlerFunctor< CAssetStoreThumbnailProvider > TEventCallback;

    CAssetStoreThumbnailProvider( ASSETSTORE::CIAssetStoreService& service, ASSETSTORE::CAssetStoreLibraryFileSystemAccess& library );

    virtual ~CAssetStoreThumbnailProvider() GUCEF_VIRTUAL_OVERRIDE;

    virtual void RequestThumbnail( const CORE::CString& path, UInt32 size ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void CancelThumbnail( const CORE::CString& path, UInt32 size ) GUCEF_VIRTUAL_OVERRIDE;

    /** The number of requests that wait for their picture */
    UInt32 GetWaitingCount( void ) const;

    private:

    struct SWaiting
    {
        CORE::CString url;
        UInt32 size;
        CORE::CString::StringVector paths;
    };
    typedef GUCEF::vector< SWaiting > TWaitingVector;

    void OnImageReady( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void Answer( const CORE::CString& path, UInt32 size, const IMAGE::TPixelMapPtr& pixels );

    CAssetStoreThumbnailProvider( const CAssetStoreThumbnailProvider& src );
    CAssetStoreThumbnailProvider& operator=( const CAssetStoreThumbnailProvider& src );

    private:

    ASSETSTORE::CIAssetStoreService& m_service;
    ASSETSTORE::CAssetStoreLibraryFileSystemAccess& m_library;
    TWaitingVector m_waiting;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CASSETSTORETHUMBNAILPROVIDER_H ? */
