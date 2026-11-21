/*
 *  AssetWorkbenchLib: gives the resources of the resource view the thumbnails that exist for the files of the pack
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

#ifndef ASSETWORKBENCHLIB_CRESOURCEVIEWTHUMBNAILPROVIDER_H
#define ASSETWORKBENCHLIB_CRESOURCEVIEWTHUMBNAILPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CITHUMBNAILPROVIDER_H
#include "gucefGUI_CIThumbnailProvider.h"
#define GUCEF_GUI_CITHUMBNAILPROVIDER_H
#endif /* GUCEF_GUI_CITHUMBNAILPROVIDER_H ? */

#ifndef ASSETWORKBENCHLIB_CTHUMBNAILPROVIDER_H
#include "AssetWorkbenchLib_CThumbnailProvider.h"
#define ASSETWORKBENCHLIB_CTHUMBNAILPROVIDER_H
#endif /* ASSETWORKBENCHLIB_CTHUMBNAILPROVIDER_H ? */

#ifndef ASSETWORKBENCHLIB_CRESOURCEVIEWFILESYSTEMACCESS_H
#include "AssetWorkbenchLib_CResourceViewFileSystemAccess.h"
#define ASSETWORKBENCHLIB_CRESOURCEVIEWFILESYSTEMACCESS_H
#endif /* ASSETWORKBENCHLIB_CRESOURCEVIEWFILESYSTEMACCESS_H ? */

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
 *  The thumbnail provider of the content browser while it shows the resource view: a path of the resource view is the file of the
 *  pack it names, and only a thumbnail that exists already (cached, or automatic) is given: nothing is rendered for the thousands of
 *  resources of a pack. In the other roots it passes everything on as it is. The answers are told with the path that was asked for.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CResourceViewThumbnailProvider : public GUI::CIThumbnailProvider
{
    public:

    CResourceViewThumbnailProvider( CThumbnailProvider& inner, const CResourceViewFileSystemAccess& view );
    virtual ~CResourceViewThumbnailProvider() GUCEF_VIRTUAL_OVERRIDE;

    /** Whether the paths asked for are paths of the resource view (the shell says so when the browser changes root) */
    void SetResourceMode( bool resourceMode );
    bool IsResourceMode( void ) const;

    virtual void RequestThumbnail( const CORE::CString& path, UInt32 size ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void CancelThumbnail( const CORE::CString& path, UInt32 size ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CResourceViewThumbnailProvider( const CResourceViewThumbnailProvider& src );
    CResourceViewThumbnailProvider& operator=( const CResourceViewThumbnailProvider& src );

    typedef CORE::CTEventHandlerFunctor< CResourceViewThumbnailProvider > TEventCallback;
    typedef GUCEF::map< CORE::CString, CORE::CString::StringVector > TPendingMap;

    void OnInnerThumbnailReady( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    CThumbnailProvider& m_inner;
    const CResourceViewFileSystemAccess& m_view;
    bool m_resourceMode;
    TPendingMap m_pending;      /**< "<file in the VFS>\n<size>" to the paths of the view that asked for it */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CRESOURCEVIEWTHUMBNAILPROVIDER_H ? */

