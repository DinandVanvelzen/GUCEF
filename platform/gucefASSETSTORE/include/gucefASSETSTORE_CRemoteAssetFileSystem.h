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

#ifndef GUCEF_ASSETSTORE_CREMOTEASSETFILESYSTEM_H
#define GUCEF_ASSETSTORE_CREMOTEASSETFILESYSTEM_H

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

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETBLOCKCACHE_H
#include "gucefASSETSTORE_CAssetBlockCache.h"
#define GUCEF_ASSETSTORE_CASSETBLOCKCACHE_H
#endif /* GUCEF_ASSETSTORE_CASSETBLOCKCACHE_H ? */

#ifndef GUCEF_ASSETSTORE_CREMOTEFILESSETTINGS_H
#include "gucefASSETSTORE_CRemoteFilesSettings.h"
#define GUCEF_ASSETSTORE_CREMOTEFILESSETTINGS_H
#endif /* GUCEF_ASSETSTORE_CREMOTEFILESSETTINGS_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#include "gucefASSETSTORE_CIAssetStoreService.h"
#define GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H ? */

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

class CRemoteFileTransfer;
typedef CORE::CTSharedPtr< CRemoteFileTransfer, MT::CMutex > TRemoteFileTransferPtr;

/**
 *  The files of a remote asset store service on the client (remote service design 5.4): every VFS path that the catalogue gives
 *  opens on the client unchanged. Each storage location of the service is mounted as a VFS archive of the type "assetstore"
 *  (CRemoteAssetArchive) at the VFS root of the location, and the archive reads through this object.
 *
 *  What is asked of the service is bounded: the sizes and kinds of files come from the lists of their folders (never from a call per
 *  file), a read fetches only the blocks it touches (CAssetBlockCache, 256 KiB each, in calls of at most maxBlocksPerRequest blocks)
 *  and keeps them on the disk, a sequential read fetches the next blocks in the same call (read ahead), and a file that is complete is checked against its
 *  SHA-256 when the catalogue gave it (NoteFiles()). A damaged block of the cache is fetched again.
 *
 *  Threads. Every call is thread safe. The thread that was given to SetNoWaitThread() (the thread of the interface) never waits for
 *  the service: what is not in the cache fails at once, the fetch starts in the background and Update() raises FileArrivedEvent or
 *  ListingArrivedEvent from that thread when it is there, so the application can try again. Every other thread waits for the answer
 *  of the service on its own thread (at most blockingReadTimeoutMs for a call) and never pumps the pulse generator of someone else.
 *
 *  Offline. A file whose blocks are all in the cache opens and reads while the service cannot be reached. Anything else fails with a
 *  message that says that the service cannot be reached and that the file is not in the cache.
 *
 *  The registry. A mounted archive finds its file system through the registry of the objects of this class by an id; Find() gives
 *  the file system of a VFS path so that other code (the digest of a file for a thumbnail cache) can ask it without a read.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CRemoteAssetFileSystem : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent FileArrivedEvent;         /**< CORE::TCloneableString: the VFS path of a file whose blocks that a read of the no-wait thread missed are in the cache now */
    static const CORE::CEvent ListingArrivedEvent;      /**< CORE::TCloneableString: the VFS path of a folder whose list the no-wait thread asked for is known now */
    static const CORE::CEvent StateChangedEvent;        /**< no data: the service became unreachable or reachable again (see IsOffline()) */
    static const CORE::CString ArchiveTypeName;         /**< "assetstore" */

    static void RegisterEvents( void );

    /** The kind and size of a file or folder */
    struct GUCEF_ASSETSTORE_PUBLIC_CPP SFileInfo
    {
        bool exists;
        bool isDirectory;
        UInt64 size;
        CORE::CString digest;               /**< the SHA-256 of the file when it is known (the catalogue gave it), else empty */
        SFileInfo( void );
    };

    /** A file open for reading; made by OpenFile() and given back with CloseFile() */
    struct GUCEF_ASSETSTORE_PUBLIC_CPP SOpenFile
    {
        CORE::CString path;
        UInt64 size;
        CORE::CString digest;
        CAssetBlockCache::TEntryHandle entry;
        UInt64 nextSequential;              /**< the offset a sequential read continues at */
        UInt32 sequentialBlocks;            /**< blocks that were read in sequence */
        UInt64 windowStart;                 /**< a file that has no cache entry keeps the last fetched run in memory */
        GUCEF::vector< UInt8 > window;
        bool cancelled;
        TAssetStoreStatus status;           /**< of the last read: OK, or what went wrong */
        CORE::CString message;              /**< the last failure in plain words */
        SOpenFile( void );
    };

    struct GUCEF_ASSETSTORE_PUBLIC_CPP SStatistics
    {
        UInt64 bytesFetched;
        UInt64 fetchCalls;
        UInt64 readAheadBlocks;             /**< blocks fetched ahead of the read that asked for them */
        UInt64 listCalls;
        UInt64 missesOfNoWaitThread;
        UInt64 digestMismatches;
        UInt64 filesVerified;
        SStatistics( void );
    };

    /** The service is not owned and has to outlive this object */
    CRemoteAssetFileSystem( CIAssetStoreService& service, const CRemoteFilesSettings& settings );

    virtual ~CRemoteAssetFileSystem() GUCEF_VIRTUAL_OVERRIDE;

    /** Opens the block cache (when the settings name a folder), puts the archive type in the VFS and the object in the registry */
    bool Open( void );

    /** Unmounts the locations and lets go of the cache */
    void Close( void );

    bool IsOpen( void ) const;

    const CORE::CString& GetId( void ) const;
    const CRemoteFilesSettings& GetSettings( void ) const;
    CIAssetStoreService& GetService( void ) const;
    CAssetBlockCache& GetCache( void );

    /** The file system that has the mount a VFS path of the client is in, with the path as the service has it; NULL for a path that is not on a remote location */
    static CRemoteAssetFileSystem* FindForPath( const CORE::CString& vfsPath, CORE::CString& servicePath );
    static CRemoteAssetFileSystem* FindById( const CORE::CString& id );

    /**
     *  Mounts one storage location (the VFS root the service reports) as an archive of the type "assetstore" at 'mountPrefix' and that
     *  root; with no prefix the VFS paths of the catalogue open on the client unchanged. A prefix is for a process that has the
     *  service in it too (the paths of the service are then another VFS). False with a logged reason
     */
    bool MountLocation( const CORE::CString& serviceRoot, const CORE::CString& mountPrefix = CORE::CString::Empty );

    /** Mounts every location that the service reports as online; the number that was mounted now */
    UInt32 MountLocations( const CORE::CString& mountPrefix = CORE::CString::Empty );

    void UnmountAll( void );

    bool IsMounted( const CORE::CString& serviceRoot ) const;
    void GetMounts( CORE::CString::StringVector& serviceRoots ) const;

    /** Whether a VFS path of the client is below a mount of this object; 'servicePath' is the path as the service has it */
    bool ToServicePath( const CORE::CString& clientPath, CORE::CString& servicePath ) const;

    /** The path of the client for a path of the service (the first mount that holds it) */
    CORE::CString ToClientPath( const CORE::CString& servicePath ) const;

    /** The thread that never waits for the service (the thread of the interface); 0 for none */
    void SetNoWaitThread( TThreadId threadId );

    /** Answers the fetches of the no-wait thread that arrived and raises their events; call it between two frames */
    void Update( void );

    /** The paths that a read of the no-wait thread missed since the last call */
    void TakeMissedPaths( CORE::CString::StringSet& paths );

    /**
     *  Size and kind of a path: from what is known, else from the list of its folder (asked of the service on a thread that may wait).
     *  False for a path that is not there, and for one that is not known yet on the no-wait thread (the list is asked for)
     */
    bool GetFileInfo( const CORE::CString& vfsPath, SFileInfo& info );

    /** The SHA-256 that the catalogue gave for a file; false when it is not known (nothing is asked of the service) */
    bool GetKnownDigest( const CORE::CString& vfsPath, CORE::CString& digest ) const;

    /** Remembers sizes and digests that the catalogue gave (the file set of an asset) */
    void NoteFiles( const GUCEF::vector< CAssetFileRecord >& files );
    void NoteFile( const CORE::CString& vfsPath, UInt64 size, const CORE::CString& digest );

    /** Remembers the SHA-256 of a file (its size is learned from the list of its folder) */
    void NoteDigest( const CORE::CString& vfsPath, const CORE::CString& digest );

    /** The names in a folder; false when the list is not known (the no-wait thread: it is asked for) or the folder is not there */
    bool GetFolderEntries( const CORE::CString& vfsFolder, CORE::CString::StringVector& dirs, CORE::CString::StringVector& files, bool* truncated = GUCEF_NULL );

    /** Starts the list of a folder in the background; the event comes when it is in */
    void RequestFolder( const CORE::CString& vfsFolder );

    bool IsFolderKnown( const CORE::CString& vfsFolder ) const;

    /** Forgets what is known of files and folders (a change of the catalogue, a new session) */
    void ForgetMetadata( void );

    /** Opens a file for reading; false with 'message' in plain words when the file is not there or cannot be had */
    bool OpenFile( const CORE::CString& vfsPath, SOpenFile& file, CORE::CString& message );

    void CloseFile( SOpenFile& file );

    /**
     *  Reads bytes at an offset: from the cache, else from the service. Gives the bytes read; less than asked at the end of the file or
     *  when something went wrong (file.status and file.message say what)
     */
    UInt32 ReadFile( SOpenFile& file, UInt64 offset, void* dest, UInt32 length );

    /**
     *  Makes sure every block of a file is in the cache, in runs of maxBlocksPerRequest blocks, and checks the digest when it is known
     *  (a mismatch empties the entry and is tried once more). 'cancel' is looked at between two calls; 'done' counts the bytes that
     *  were added to what the cache held. OK, or what went wrong in 'message'
     */
    TAssetStoreStatus FetchWholeFile( const CORE::CString& vfsPath, bool pin, const MT::CTAtomic< bool >* cancel, MT::CTAtomic< UInt64 >* done, CORE::CString& message );

    /** Whether every block of the file is in the cache (and its size is known) */
    bool IsFileCached( const CORE::CString& vfsPath, UInt64 size ) const;

    /** Releases the pin of the entry of a file */
    void UnpinFile( const CORE::CString& vfsPath );

    /**
     *  Starts a transfer (CRemoteFileTransfer) as a task of a thread pool; the future ends with it (its status and message are those of
     *  the transfer). The progress is read from the transfer object. Close() cancels the transfers that run and waits for them
     */
    CORE::CFutureResult StartTransfer( const TRemoteFileTransferPtr& transfer, const CORE::CString& threadPoolName = CORE::CString::Empty );

    /** Called by the task of a transfer when it ends */
    void TransferEnded( const TRemoteFileTransferPtr& transfer );

    UInt32 GetRunningTransferCount( void ) const;

    /** Whether the service could not be reached at the last call */
    bool IsOffline( void ) const;

    void GetStatistics( SStatistics& statistics ) const;

    /** A message in plain words for a status of a failed call about a file */
    static CORE::CString DescribeFailure( const CORE::CString& vfsPath, TAssetStoreStatus status, const CORE::CString& serviceMessage );

    /** The path as the file system has it: '/' separators, no '/' first and none at the end */
    static CORE::CString Normalize( const CORE::CString& vfsPath );

    private:

    CRemoteAssetFileSystem( const CRemoteAssetFileSystem& src );
    CRemoteAssetFileSystem& operator=( const CRemoteAssetFileSystem& src );

    struct SMeta
    {
        UInt64 size;
        CORE::CString digest;
        UInt32 kind;                        /**< 1 file, 2 folder, 3 not there */
        UInt64 validUntilMs;                /**< 0 for no end */
        SMeta( void );
    };
    typedef GUCEF::map< CORE::CString, SMeta > TMetaMap;
    typedef GUCEF::vector< TRemoteFileTransferPtr > TTransferVector;

    struct SListing
    {
        CORE::CString::StringVector dirs;
        CORE::CString::StringVector files;
        bool truncated;
        UInt64 loadedMs;
        SListing( void );
    };
    typedef GUCEF::map< CORE::CString, SListing > TListingMap;

    struct SAsyncList
    {
        CORE::CString folder;
        CORE::CFutureResult future;
        SListing listing;
        UInt32 entries;
        SAsyncList( void );
    };
    typedef GUCEF::vector< SAsyncList > TAsyncListVector;

    struct SAsyncRun
    {
        CORE::CString path;
        UInt64 size;
        CORE::CString digest;
        CAssetBlockCache::TEntryHandle entry;
        CORE::CFutureResult future;
        UInt32 firstBlock;
        UInt32 blockCount;
        SAsyncRun( void );
    };
    typedef GUCEF::vector< SAsyncRun > TAsyncRunVector;

    bool IsNoWaitThread( void ) const;
    bool LookupMeta( const CORE::CString& path, SMeta& meta ) const;
    void PutMeta( const CORE::CString& path, const SMeta& meta );
    bool LoadListing( const CORE::CString& folder, SListing& listing, bool& exists );
    void ApplyPage( const CORE::CString& folder, const CAssetFileStatPage& page, SListing& listing, UInt32& entries );
    void StoreListing( const CORE::CString& folder, const SListing& listing );
    UInt32 ReadFromCacheOrFetch( SOpenFile& file, UInt64 offset, void* dest, UInt32 length );
    bool FetchRun( SOpenFile& file, UInt32 firstBlock, UInt32 blockCount, GUCEF::vector< UInt8 >& data );
    void StoreRun( CAssetBlockCache::TEntryHandle entry, UInt32 firstBlock, const UInt8* data, UInt32 length );
    void ScheduleMissedRun( SOpenFile& file, UInt32 firstBlock, UInt32 blockCount );
    void CheckComplete( SOpenFile& file );
    void SetOffline( bool offline );
    void OnCatalogueChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    typedef CORE::CTEventHandlerFunctor< CRemoteAssetFileSystem > TEventCallback;

    private:

    CIAssetStoreService& m_service;
    CRemoteFilesSettings m_settings;
    CAssetBlockCache m_cache;
    CORE::CString m_id;
    bool m_open;
    bool m_cacheOpen;
    struct SMount
    {
        CORE::CString serviceRoot;
        CORE::CString mountPath;
    };
    typedef GUCEF::vector< SMount > TMountVector;
    TMountVector m_mounts;
    TThreadId m_noWaitThread;
    TMetaMap m_meta;
    TListingMap m_listings;
    TAsyncListVector m_asyncLists;
    TAsyncRunVector m_asyncRuns;
    CORE::CString::StringSet m_missed;
    CORE::CString::StringSet m_arrived;
    GUCEF::map< CORE::CString, CORE::CString > m_verified;
    GUCEF::map< CORE::CString, CORE::CString > m_digests;
    TTransferVector m_transfers;
    bool m_offline;
    bool m_offlineChanged;
    mutable MT::CMutex m_lock;
    MT::CTAtomic< UInt64 > m_bytesFetched;
    MT::CTAtomic< UInt64 > m_fetchCalls;
    MT::CTAtomic< UInt64 > m_readAheadBlocks;
    MT::CTAtomic< UInt64 > m_listCalls;
    MT::CTAtomic< UInt64 > m_noWaitMisses;
    MT::CTAtomic< UInt64 > m_digestMismatches;
    MT::CTAtomic< UInt64 > m_filesVerified;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CREMOTEASSETFILESYSTEM_H ? */
