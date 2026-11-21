/*
 *  AssetWorkbenchLib: thumbnails of the content browser: rendered through the capture path and cached by the hash of the content
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

#ifndef ASSETWORKBENCHLIB_CTHUMBNAILPROVIDER_H
#define ASSETWORKBENCHLIB_CTHUMBNAILPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CITHUMBNAILPROVIDER_H
#include "gucefGUI_CIThumbnailProvider.h"
#define GUCEF_GUI_CITHUMBNAILPROVIDER_H
#endif /* GUCEF_GUI_CITHUMBNAILPROVIDER_H ? */

#ifndef ASSETWORKBENCHLIB_CAUTOTHUMBNAILSTORE_H
#include "AssetWorkbenchLib_CAutoThumbnailStore.h"
#define ASSETWORKBENCHLIB_CAUTOTHUMBNAILSTORE_H
#endif /* ASSETWORKBENCHLIB_CAUTOTHUMBNAILSTORE_H ? */

#ifndef ASSETWORKBENCHLIB_CASSETCAPTURE_H
#include "AssetWorkbenchLib_CAssetCapture.h"
#define ASSETWORKBENCHLIB_CASSETCAPTURE_H
#endif /* ASSETWORKBENCHLIB_CASSETCAPTURE_H ? */

#ifndef ASSETWORKBENCHLIB_CREMOTETHUMBNAILSOURCE_H
#include "AssetWorkbenchLib_CRemoteThumbnailSource.h"
#define ASSETWORKBENCHLIB_CREMOTETHUMBNAILSOURCE_H
#endif /* ASSETWORKBENCHLIB_CREMOTETHUMBNAILSOURCE_H ? */

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
 *  The thumbnails of the content browser of the workbench. A request is queued and answered later (ThumbnailReadyEvent), when the
 *  application calls Update() between two frames of the window: the GUI thread never waits for a render inside RequestThumbnail().
 *  Update() makes the thumbnail of at most one request per call that has to be rendered, and answers any number of requests that the
 *  cache has within its time budget.
 *
 *  The cache is a folder of PNG files named by the SHA-256 of the content of the asset, the version of the workbench and the size:
 *  the same content is never rendered twice, content that changed has another hash and is rendered again, and a new version of
 *  the workbench (GetWorkbenchVersion(), raised when the pictures it makes change) does not use the old pictures. With no cache folder
 *  the pictures are only rendered.
 *
 *  A path that is no model, scene, material or texture is answered with no picture, at once.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CThumbnailProvider : public GUI::CIThumbnailProvider
{
    public:

    /** Raised when what the thumbnails show changes: it is part of the name of the cache files */
    static UInt32 GetWorkbenchVersion( void );

    CThumbnailProvider( CAssetMounter& mounter, CAssetCapture& capture );

    virtual ~CThumbnailProvider() GUCEF_VIRTUAL_OVERRIDE;

    /** The folder of the cache (a path of the disk, made when it is not there); empty for no cache */
    void SetCacheFolder( const CORE::CString& folder );
    const CORE::CString& GetCacheFolder( void ) const;

    virtual void RequestThumbnail( const CORE::CString& path, UInt32 size ) GUCEF_VIRTUAL_OVERRIDE;
    /** As RequestThumbnail(), for a thumbnail that exists already (a cached one, an automatic one): nothing is rendered for it, and the answer has no picture when there is none */
    void RequestExistingThumbnail( const CORE::CString& path, UInt32 size );

    virtual void CancelThumbnail( const CORE::CString& path, UInt32 size ) GUCEF_VIRTUAL_OVERRIDE;

    /** Answers queued requests, within the budget in milliseconds for the ones that need no render; the number answered */
    UInt32 Update( UInt32 budgetInMs );

    UInt32 GetQueuedCount( void ) const;
    UInt32 GetCacheHitCount( void ) const;
    UInt32 GetRenderCount( void ) const;
    UInt32 GetFailureCount( void ) const;

    /**
     *  The automatic thumbnails (the renders of presentations): shown for the root of a pack, and for a model that has no thumbnail of its
     *  own in the cache yet. A thumbnail of the cache of the content always comes first. Not owned, may be NULL
     */
    void SetAutoThumbnails( CAutoThumbnailStore* store );

    /**
     *  The pictures of a remote asset store service (not owned, may be NULL): a path that is a file of the service is answered with the picture that
     *  the service stores for its asset (fetched for the rows that are shown, kept on the disk); without one it is rendered here when its files are in
     *  the cache of the files already (and the render is put on the service for the other devices), else it has no picture. Nothing waits for the service
     */
    void SetRemoteSource( CRemoteThumbnailSource* source );
    UInt32 GetRemoteHitCount( void ) const;
    /** How many answers were automatic thumbnails */
    UInt32 GetAutoHitCount( void ) const;
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
        bool existingOnly;
    };
    typedef GUCEF::vector< SRequest > TRequestVector;

    void Answer( const SRequest& request, const IMAGE::TPixelMapPtr& pixels );
    bool ReadCache( const CORE::CString& file, IMAGE::TPixelMapPtr& pixels );
    bool IsRenderable( const CORE::CString& path ) const;

    private:

    CAssetMounter& m_mounter;
    CAssetCapture& m_capture;
    CAutoThumbnailStore* m_auto;
    UInt32 m_autoHits;
    GUCEF::map< CORE::CString, CORE::CString > m_packHashes;
    CORE::CString m_cacheFolder;
    TRequestVector m_queue;
    TRequestVector m_waitingRemote;
    CRemoteThumbnailSource* m_remote;
    UInt32 m_remoteHits;
    UInt32 m_cacheHits;
    UInt32 m_renders;
    UInt32 m_failures;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CTHUMBNAILPROVIDER_H ? */
