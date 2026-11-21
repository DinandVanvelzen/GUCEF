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

#ifndef GUCEF_ASSETSTORE_CREMOTEFILETRANSFER_H
#define GUCEF_ASSETSTORE_CREMOTEFILETRANSFER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_ASSETSTORE_CREMOTEASSETFILESYSTEM_H
#include "gucefASSETSTORE_CRemoteAssetFileSystem.h"
#define GUCEF_ASSETSTORE_CREMOTEASSETFILESYSTEM_H
#endif /* GUCEF_ASSETSTORE_CREMOTEASSETFILESYSTEM_H ? */

#ifndef GUCEF_ASSETSTORE_CIREMOTEFILEDISCOVERY_H
#include "gucefASSETSTORE_CIRemoteFileDiscovery.h"
#define GUCEF_ASSETSTORE_CIREMOTEFILEDISCOVERY_H
#endif /* GUCEF_ASSETSTORE_CIREMOTEFILEDISCOVERY_H ? */

#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */

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

/**
 *  A copy of files of a remote asset store service to this computer (remote service design 5.4 and 5.10). What it does with the files
 *  depends on the mode:
 *
 *  - PREFETCH: every block of the files is in the block cache (an asset opens in the views from there, without a wait);
 *  - KEEP_OFFLINE: as PREFETCH, and the entries are pinned, so the cache never removes them to make room;
 *  - RELEASE_OFFLINE: the pins of the files are released;
 *  - DOWNLOAD: the files are written to a folder of the disk under their relative names. A file is written as "name.part" and
 *    renamed when it is complete and its SHA-256 is right (when the catalogue gave one). A "name.part" of an earlier run (a cancel, a
 *    link that did not come back, a restart) is continued from its last whole block; a file that is there already with the right size
 *    and digest is left alone. Cancel() leaves the part for the next run.
 *
 *  Run() runs on the calling thread (a task of the thread pool when it is started with CRemoteAssetFileSystem::StartTransfer()); the
 *  progress, the state and Cancel() are for any thread. A call that fails because the service cannot be reached is tried again with a
 *  growing wait, so a link that comes back goes on without a new transfer.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CRemoteFileTransfer
{
    public:

    enum EMode
    {
        MODE_PREFETCH = 0,
        MODE_KEEP_OFFLINE,
        MODE_RELEASE_OFFLINE,
        MODE_DOWNLOAD
    };

    enum EState
    {
        STATE_QUEUED = 0,
        STATE_LISTING,
        STATE_RUNNING,
        STATE_DONE,
        STATE_FAILED,
        STATE_CANCELLED
    };

    enum EDefaults
    {
        MaxAttempts = 5,
        ChunkBytes = 1048576
    };

    CRemoteFileTransfer( EMode mode, const CORE::CString& name );

    ~CRemoteFileTransfer();

    EMode GetMode( void ) const;
    const CORE::CString& GetName( void ) const;

    /** A file of the service with its name below the destination folder; the size and digest as the catalogue gave them (0 and empty when unknown) */
    void AddFile( const CORE::CString& servicePath, const CORE::CString& targetPath, UInt64 size, const CORE::CString& digest );

    /** A folder of the service with everything below it; the names below the destination folder start with 'targetPrefix' */
    void AddFolder( const CORE::CString& serviceFolder, const CORE::CString& targetPrefix );

    /** The files of a pack: its folder, with the digests that the catalogue has for the files of its assets. False with 'message' for an unknown pack */
    bool AddPack( CIAssetStoreService& service, const CORE::CString& packId, CORE::CString& message );

    /** The files of an asset (its file set), the names below the destination are relative to the folder that all of them are in */
    bool AddAsset( CIAssetStoreService& service, const CORE::CString& assetId, CORE::CString& message );

    /**
     *  As AddPack() and AddAsset(), made when the transfer runs (on its thread, so the calls to the service are no wait of the thread that
     *  started it); the transfer ends as failed with the reason when the pack or the asset cannot be had. The service is not owned
     */
    void RequestPack( CIAssetStoreService& service, const CORE::CString& packId );
    void RequestAsset( CIAssetStoreService& service, const CORE::CString& assetId );

    void SetDestination( const CORE::CString& folder );
    const CORE::CString& GetDestination( void ) const;

    /**
     *  PREFETCH, KEEP_OFFLINE: a folder whose list is read into the file system before the files are fetched, so the sizes and kinds that the views ask for are known without a wait.
     *  With 'recursive' the folders below it are read as well (to a depth and a number of folders that are bounded)
     */
    void AddListing( const CORE::CString& serviceFolder, bool recursive = false );

    /** PREFETCH: asks 'discovery' for more files once the given ones are in the cache; it is deleted with the transfer when 'takeOwnership'. Replaces the discoveries that were set */
    void SetDiscovery( CIRemoteFileDiscovery* discovery, bool takeOwnership = false );

    /** PREFETCH: as SetDiscovery(), but one more. The discoveries are asked in the order they were added, and each one sees the files that the ones before it got */
    void AddDiscovery( CIRemoteFileDiscovery* discovery, bool takeOwnership = false );

    /**
     *  A discovery that is asked only when the assets that the transfer added (AddAsset(), RequestAsset()) have no relations in the catalogue
     *  (CAssetFileSet::relationCount 0 for each): the companion files of an asset come from its relations first, and a discovery that finds
     *  them in another way (by their names) is the fallback. It is asked in every mode but RELEASE_OFFLINE, after the given files are done; it
     *  gets the file set of each asset first (CIRemoteFileDiscovery::OnAssetFileSet()). Downloaded files it finds are named below the
     *  destination relative to the folder of the files of the asset, as the files of the set are
     */
    void AddFallbackDiscovery( CIRemoteFileDiscovery* discovery, bool takeOwnership = false );

    /** How many relations the file sets of the assets of the transfer followed, once it ran: 0 when they have none (the fallback discoveries were asked) */
    UInt32 GetAssetRelationCount( void ) const;

    /**
     *  PREFETCH: ends the transfer as done after this many bytes were fetched, the rest of the files is left for the reads that ask for
     *  it (a set that is too large to wait for: it is opened with what is there); 0 for no limit
     */
    void SetFetchLimit( UInt64 bytes );

    /** Does the work on the calling thread; the status of the end (the same as GetStatus()) */
    TAssetStoreStatus Run( CRemoteAssetFileSystem& system );

    void Cancel( void );
    bool IsCancelRequested( void ) const;

    EState GetState( void ) const;
    bool IsEnded( void ) const;
    TAssetStoreStatus GetStatus( void ) const;

    /** What went wrong, or how it ended, in plain words */
    CORE::CString GetMessage( void ) const;
    CORE::CString GetCurrentFile( void ) const;

    UInt64 GetBytesDone( void ) const;
    UInt64 GetBytesTotal( void ) const;
    UInt32 GetFilesDone( void ) const;
    UInt32 GetFileCount( void ) const;

    /** The bytes that a restart did not have to fetch: parts continued, files that were there, blocks that were cached */
    UInt64 GetBytesReused( void ) const;

    private:

    CRemoteFileTransfer( const CRemoteFileTransfer& src );
    CRemoteFileTransfer& operator=( const CRemoteFileTransfer& src );

    struct SItem
    {
        CORE::CString servicePath;
        CORE::CString target;
        UInt64 size;
        CORE::CString digest;
        SItem( void );
    };
    typedef GUCEF::vector< SItem > TItemVector;

    struct SFolder
    {
        CORE::CString servicePath;
        CORE::CString target;
    };
    typedef GUCEF::vector< SFolder > TFolderVector;
    typedef GUCEF::vector< std::pair< CORE::CString, CORE::CString > > TDigestVector;
    struct SRequest
    {
        CIAssetStoreService* service;
        CORE::CString id;
        bool pack;
        SRequest( void ) : service( GUCEF_NULL ), id(), pack( true ) {}
    };
    typedef GUCEF::vector< SRequest > TRequestVector;
    struct SDiscovery
    {
        CIRemoteFileDiscovery* discovery;
        bool owned;
        bool fallback;
        SDiscovery( void ) : discovery( GUCEF_NULL ), owned( false ), fallback( false ) {}
    };
    typedef GUCEF::vector< SDiscovery > TDiscoveryVector;

    bool Expand( CRemoteAssetFileSystem& system );
    TAssetStoreStatus RunItem( CRemoteAssetFileSystem& system, const SItem& item, CORE::CString& message );
    TAssetStoreStatus DownloadItem( CRemoteAssetFileSystem& system, const SItem& item, CORE::CString& message );
    TAssetStoreStatus DownloadAttempt( CRemoteAssetFileSystem& system, const SItem& item, const CORE::CString& part, bool fromStart, bool& partWasBad, CORE::CString& message );
    static bool HashFile( const CORE::CString& path, CORE::CString& hex );
    static bool IsRetryable( TAssetStoreStatus status );
    void Finish( EState state, TAssetStoreStatus status, const CORE::CString& message );
    void SetCurrent( const CORE::CString& file );

    private:

    EMode m_mode;
    CORE::CString m_name;
    CORE::CString m_destination;
    TItemVector m_items;
    TFolderVector m_folders;
    TDigestVector m_pendingDigests;
    TRequestVector m_requests;
    CORE::CString::StringVector m_listings;
    CORE::CString::StringSet m_recursiveListings;
    TDiscoveryVector m_discoveries;
    UInt32 m_assetCount;
    MT::CTAtomic< UInt32 > m_assetRelations;
    CORE::CString m_assetFolder;
    UInt64 m_fetchLimit;
    mutable MT::CMutex m_lock;
    CORE::CString m_message;
    CORE::CString m_current;
    MT::CTAtomic< UInt32 > m_state;
    MT::CTAtomic< UInt32 > m_status;
    MT::CTAtomic< bool > m_cancel;
    MT::CTAtomic< UInt64 > m_bytesDone;
    MT::CTAtomic< UInt64 > m_bytesTotal;
    MT::CTAtomic< UInt64 > m_bytesReused;
    MT::CTAtomic< UInt32 > m_filesDone;
    MT::CTAtomic< UInt32 > m_fileCount;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CREMOTEFILETRANSFER_H ? */
