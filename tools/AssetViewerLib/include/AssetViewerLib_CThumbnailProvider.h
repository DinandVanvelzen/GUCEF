/*
 *  AssetViewerLib: thumbnails of the content browser: rendered through the capture path and cached by the hash of the content
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

#ifndef ASSETVIEWERLIB_CTHUMBNAILPROVIDER_H
#define ASSETVIEWERLIB_CTHUMBNAILPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CITHUMBNAILPROVIDER_H
#include "gucefGUI_CIThumbnailProvider.h"
#define GUCEF_GUI_CITHUMBNAILPROVIDER_H
#endif /* GUCEF_GUI_CITHUMBNAILPROVIDER_H ? */

#ifndef ASSETVIEWERLIB_CASSETCAPTURE_H
#include "AssetViewerLib_CAssetCapture.h"
#define ASSETVIEWERLIB_CASSETCAPTURE_H
#endif /* ASSETVIEWERLIB_CASSETCAPTURE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETVIEWER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The thumbnails of the content browser of the viewer. A request is queued and answered later (ThumbnailReadyEvent), when the
 *  application calls Update() between two frames of the window: the GUI thread never waits for a render inside RequestThumbnail().
 *  Update() makes the thumbnail of at most one request per call that has to be rendered, and answers any number of requests that the
 *  cache has within its time budget.
 *
 *  The cache is a folder of PNG files named by the SHA-256 of the content of the asset, the version of the viewer and the size:
 *  the same content is never rendered twice, content that changed has another hash and is rendered again, and a new version of
 *  the viewer (GetViewerVersion(), raised when the pictures it makes change) does not use the old pictures. With no cache folder
 *  the pictures are only rendered.
 *
 *  A path that is no model, scene, material or texture is answered with no picture, at once.
 */
class ASSETVIEWERLIB_PUBLIC_CPP CThumbnailProvider : public GUI::CIThumbnailProvider
{
    public:

    /** Raised when what the thumbnails show changes: it is part of the name of the cache files */
    static UInt32 GetViewerVersion( void );

    CThumbnailProvider( CAssetMounter& mounter, CAssetCapture& capture );

    virtual ~CThumbnailProvider() GUCEF_VIRTUAL_OVERRIDE;

    /** The folder of the cache (a path of the disk, made when it is not there); empty for no cache */
    void SetCacheFolder( const CORE::CString& folder );
    const CORE::CString& GetCacheFolder( void ) const;

    virtual void RequestThumbnail( const CORE::CString& path, UInt32 size ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void CancelThumbnail( const CORE::CString& path, UInt32 size ) GUCEF_VIRTUAL_OVERRIDE;

    /** Answers queued requests, within the budget in milliseconds for the ones that need no render; the number answered */
    UInt32 Update( UInt32 budgetInMs );

    UInt32 GetQueuedCount( void ) const;
    UInt32 GetCacheHitCount( void ) const;
    UInt32 GetRenderCount( void ) const;
    UInt32 GetFailureCount( void ) const;

    /** The SHA-256 of the content of a VFS file as 64 hex digits; empty when the file cannot be read */
    static CORE::CString HashContent( const CORE::CString& vfsPath );

    /** The name of the cache file (without the folder) for a content hash and a size */
    static CORE::CString MakeCacheFileName( const CORE::CString& contentHash, UInt32 size );

    private:

    CThumbnailProvider( const CThumbnailProvider& src );
    CThumbnailProvider& operator=( const CThumbnailProvider& src );

    struct SRequest
    {
        CORE::CString path;
        UInt32 size;
    };
    typedef GUCEF::vector< SRequest > TRequestVector;

    void Answer( const SRequest& request, const IMAGE::TPixelMapPtr& pixels );
    bool ReadCache( const CORE::CString& file, IMAGE::TPixelMapPtr& pixels );
    bool IsRenderable( const CORE::CString& path ) const;

    private:

    CAssetMounter& m_mounter;
    CAssetCapture& m_capture;
    CORE::CString m_cacheFolder;
    TRequestVector m_queue;
    UInt32 m_cacheHits;
    UInt32 m_renders;
    UInt32 m_failures;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETVIEWERLIB_CTHUMBNAILPROVIDER_H ? */
