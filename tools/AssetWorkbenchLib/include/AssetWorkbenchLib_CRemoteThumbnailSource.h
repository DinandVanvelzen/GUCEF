/*
 *  AssetWorkbenchLib: the thumbnails of a remote asset store service
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

#ifndef ASSETWORKBENCHLIB_CREMOTETHUMBNAILSOURCE_H
#define ASSETWORKBENCHLIB_CREMOTETHUMBNAILSOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_ASSETSTORE_CREMOTEASSETSHARES_H
#include "gucefASSETSTORE_CRemoteAssetShares.h"
#define GUCEF_ASSETSTORE_CREMOTEASSETSHARES_H
#endif /* GUCEF_ASSETSTORE_CREMOTEASSETSHARES_H ? */

#ifndef GUCEF_VFS_CVFSASYNCRESULT_H
#include "gucefVFS_CVfsAsyncResult.h"
#define GUCEF_VFS_CVFSASYNCRESULT_H
#endif /* GUCEF_VFS_CVFSASYNCRESULT_H ? */

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
 *  The thumbnails of the results and of the files of a remote asset store service (service design 5.2): fetched lazily, for the rows
 *  that are visible only (the thumbnail provider asks when a row needs its picture and says when it goes), in batches of at most 64
 *  assets per call and per size, coalesced over 50 ms of requests, and kept as PNG files in a folder of the disk, so a picture that
 *  was fetched once is never asked for again. The service gives the stored picture as a file of its location: the file is read like
 *  any file of the service, in one call, and the block cache is not used for it.
 *
 *  A picture is for an asset id; the search results tell which VFS path is which asset (NoteAsset()). A path that is no result is
 *  not asked for. An asset without a stored picture is remembered for a minute and answered with none, so the provider renders it
 *  from its files when they are in the cache (and puts the render on the service with Upload(), so that every device has it).
 *
 *  Lookup(), Update() and Upload() are called on the thread of the interface. ThumbnailReadyEvent comes from Update().
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CRemoteThumbnailSource : public CORE::CObservingNotifier
{
    public:

    enum ELookup
    {
        LOOKUP_READY = 0,           /**< the picture is a file of the cache folder: 'cacheFile' */
        LOOKUP_PENDING,             /**< it is asked for: ThumbnailReadyEvent follows */
        LOOKUP_NONE                 /**< the service has no picture, or the path is no result of the service */
    };

    enum EDefaults
    {
        MaxBatch = 64,
        CoalesceMs = 50,
        MaxInFlight = 2,
        NegativeMs = 60000,
        MaxPictureBytes = 2097152
    };

    struct ASSETWORKBENCHLIB_PUBLIC_CPP SReady
    {
        CORE::CString path;
        UInt32 size;
        bool found;
        SReady( void );
    };
    typedef CORE::CTCloneableObj< SReady > TReadyEventData;

    static const CORE::CEvent ThumbnailReadyEvent;      /**< TReadyEventData */

    static void RegisterEvents( void );

    CRemoteThumbnailSource( void );

    virtual ~CRemoteThumbnailSource() GUCEF_VIRTUAL_OVERRIDE;

    /** The service and its file system (neither owned); NULL for both: nothing is remote */
    void SetService( ASSETSTORE::CIAssetStoreService* service, ASSETSTORE::CRemoteAssetShares* system );

    /** The folder of the pictures (a path of the disk, made when it is not there) */
    void SetCacheFolder( const CORE::CString& folder );
    const CORE::CString& GetCacheFolder( void ) const;

    /** The VFS path of the client for a path of the service (a path of a search result); the same path when nothing is remote */
    CORE::CString ToClientPath( const CORE::CString& servicePath ) const;

    /** Whether a VFS path is a file of the service */
    bool IsRemotePath( const CORE::CString& vfsPath ) const;

    /** Whether the whole file of a VFS path is in the cache of the files: it can be rendered here without a wait */
    bool IsFileAvailable( const CORE::CString& vfsPath ) const;

    /** The folder of the pack of a file ("<location>/<area>/<pack folder>", on this computer), false when the path is no file of a pack in the layout of the store */
    bool GetPackFolder( const CORE::CString& vfsPath, CORE::CString& packFolder ) const;

    /** Tells which asset a VFS path is (a search result: the path of its original); an id of "pack:<id>" is the automatic picture of a pack */
    void NoteAsset( const CORE::CString& vfsPath, const CORE::CString& assetId );

    ELookup Lookup( const CORE::CString& vfsPath, UInt32 size, CORE::CString& cacheFile );

    /** Takes back what the requests no longer need (a row that left the screen): a request that was not sent yet is dropped */
    void Cancel( const CORE::CString& vfsPath, UInt32 size );

    /** Sends the requests that are due, takes the answers: ThumbnailReadyEvent for each path that has its answer */
    void Update( void );

    /** Whether this computer may put pictures on the service (the role "owner"); the pane sets it from what the service says of the roles. Default true; false: Upload() and UploadRender() do nothing */
    void SetMayUpload( bool mayUpload );
    bool GetMayUpload( void ) const;

    /** Puts the picture (PNG bytes) of the path on the service (a render of the workbench): a command of the service, the answer is not waited for */
    bool Upload( const CORE::CString& vfsPath, UInt32 size, const CORE::CDynamicBuffer& png );

    /** As Upload(), for a render that is in memory (the picture of what the views show): its width is the size; once per asset in a run */
    bool UploadRender( const CORE::CString& vfsPath, const IMAGE::TPixelMapPtr& pixels );

    UInt32 GetRequestedCount( void ) const;
    UInt32 GetFetchedCount( void ) const;
    UInt32 GetMissingCount( void ) const;
    UInt32 GetBatchCount( void ) const;
    UInt32 GetUploadCount( void ) const;
    UInt32 GetPendingCount( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CRemoteThumbnailSource > TEventCallback;

    struct SWanted
    {
        CORE::CString assetId;
        UInt32 size;
        GUCEF::vector< CORE::CString > paths;
        UInt64 queuedMs;
        bool sent;
        SWanted( void );
    };
    typedef GUCEF::vector< SWanted > TWantedVector;

    struct SBatch
    {
        CORE::CFutureResult future;
        UInt32 size;
        CORE::CString::StringVector ids;
        SBatch( void );
    };
    typedef GUCEF::vector< SBatch > TBatchVector;

    struct SPicture
    {
        CORE::CFutureResult future;
        VFS::CVfsAsyncResultPtr result;                  /**< the bytes of the picture file, filled in before the future ends */
        CORE::CString assetId;
        UInt32 size;
        SPicture( void );
    };
    typedef GUCEF::vector< SPicture > TPictureVector;

    CRemoteThumbnailSource( const CRemoteThumbnailSource& src );
    CRemoteThumbnailSource& operator=( const CRemoteThumbnailSource& src );

    CORE::CString MakeCacheFile( const CORE::CString& assetId, UInt32 size ) const;
    void Finish( const CORE::CString& assetId, UInt32 size, bool found );
    void OnThumbnailUpdated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    static UInt64 NowMs( void );

    private:

    ASSETSTORE::CIAssetStoreService* m_service;
    ASSETSTORE::CRemoteAssetShares* m_system;
    CORE::CString m_folder;
    GUCEF::map< CORE::CString, CORE::CString > m_assetOfPath;
    GUCEF::map< CORE::CString, UInt64 > m_missing;                  /**< "id\nsize" to the time it was found missing */
    TWantedVector m_wanted;
    TBatchVector m_batches;
    TPictureVector m_pictures;
    GUCEF::vector< CORE::CFutureResult > m_uploads;
    UInt32 m_requested;
    UInt32 m_fetched;
    UInt32 m_missingCount;
    UInt32 m_batchCount;
    UInt32 m_uploadCount;
    UInt32 m_uploadNumber;
    bool m_mayUpload;
    GUCEF::set< CORE::CString > m_renderedAssets;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CREMOTETHUMBNAILSOURCE_H ? */
