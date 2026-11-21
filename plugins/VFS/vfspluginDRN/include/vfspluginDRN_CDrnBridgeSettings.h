/*
 *  vfspluginDRN: GUCEF VFS plugin presenting a share of another DRN node as a VFS archive
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

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGESETTINGS_H
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGESETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCETRANSFEROPTIONS_H
#include "gucefDRN_CDRNResourceTransferOptions.h"
#define GUCEF_DRN_CDRNRESOURCETRANSFEROPTIONS_H
#endif /* GUCEF_DRN_CDRNRESOURCETRANSFEROPTIONS_H ? */

#ifndef GUCEF_VFS_CARCHIVESETTINGS_H
#include "gucefVFS_CArchiveSettings.h"
#define GUCEF_VFS_CARCHIVESETTINGS_H
#endif /* GUCEF_VFS_CARCHIVESETTINGS_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_MACROS_H
#include "vfspluginDRN_macros.h"
#define GUCEF_VFSPLUGIN_DRN_MACROS_H
#endif /* GUCEF_VFSPLUGIN_DRN_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace DRNBRIDGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The settings of a bridge archive. They are read from the archive specific settings of the archive's configuration
 *  (VFS::CArchiveSettings::GetArchiveSpecificSettings(): the entries "ArchiveSpecificSetting.<key>" of the archive's
 *  configuration node), by the keys below; a key that is not there, or whose value cannot be used, leaves the default.
 *  They are listed with their meaning in the README of the plugin.
 */
class GUCEF_VFSPLUGIN_DRN_EXPORT_CPP CDrnBridgeSettings
{
    public:

    // keys of the archive specific settings
    static const char* const KeyMetadataTtlMs;
    static const char* const KeyNegativeTtlMs;
    static const char* const KeyMaxCachedEntries;
    static const char* const KeyServeStaleWhenOffline;
    static const char* const KeyMaxOpenHandles;
    static const char* const KeyChunkCacheChunks;
    static const char* const KeyReadAheadChunks;
    static const char* const KeyCallTimeoutMs;
    static const char* const KeyOpenTimeoutMs;
    static const char* const KeyListPageEntries;
    static const char* const KeyMaxEntriesPerDirectory;
    static const char* const KeyMaxWalkedDirectories;
    static const char* const KeyVerifyMode;
    static const char* const KeyBusyRetries;
    static const char* const KeyBusyBackoffMs;
    static const char* const KeyBlockCacheFolder;
    static const char* const KeyBlockCacheMaxBytes;
    static const char* const KeyBlockCacheBlockBytes;
    static const char* const KeyCacheIdentity;
    static const char* const KeyReadAheadAfterBlocks;
    static const char* const KeyReadAheadBlocks;
    static const char* const KeyMaxBlocksPerFetch;
    static const char* const KeyMaxBackgroundFetches;
    static const char* const KeyMaxQueuedFetches;
    static const char* const KeyFetchThreadPool;
    static const char* const KeyVerifyCompleteFiles;
    static const char* const KeyRefetchPinned;
    static const char* const KeyOfflineListings;
    static const char* const KeyOpenRequiresCompleteFile;

    UInt32 metadataTtlMs;               /**< how long listings and stats are taken as current, 0 = no metadata cache at all, default 3000 */
    UInt32 negativeTtlMs;               /**< how long "there is no such entry" is taken as current, 0 = never remembered, default 1000 */
    UInt32 maxCachedEntries;            /**< entries (stats and listed entries) the metadata cache holds, default 50000; the oldest are dropped first */
    bool serveStaleWhenOffline;         /**< a mount whose share cannot be reached answers from expired metadata rather than saying nothing, default true */
    UInt32 maxOpenHandles;              /**< DRN handles this archive keeps open at once, one per open file; the serving side allows 32 per member, default 12 */
    UInt32 chunkCacheChunks;            /**< chunks of 64 KiB each open file keeps in memory (the facility's cache), default 8 */
    UInt32 readAheadChunks;             /**< chunks asked for ahead of a sequential read, default 4 */
    UInt32 callTimeoutMs;               /**< the time a namespace call (list, stat) may wait for the serving side, default 15000 */
    UInt32 openTimeoutMs;               /**< the time the opening of a file may wait for the serving side, default 15000 */
    UInt32 listPageEntries;             /**< entries asked per page of a listing, 0 = the serving side's page size, default 0 */
    UInt32 maxEntriesPerDirectory;      /**< entries of one directory that are listed at most, default 500000 */
    UInt32 maxWalkedDirectories;        /**< directories a recursive list visits at most, default 100000 */
    DRN::CDRNResourceTransferOptions::TVerifyMode verifyMode;   /**< "link" (default), "tree" or "treeIfAvailable": how the bytes of a file are checked */
    UInt32 busyRetries;                 /**< times a call that the serving side refused as BUSY (a limit it keeps) is tried again before the retry-later result is given, 0 = never, default 8 */
    UInt32 busyBackoffMs;               /**< wait before the first retry; every retry waits twice as long as the one before, at most 2000 ms per wait, default 25 */

    // the persistent block cache and the behaviour of a remote archive (the generic layer of gucefVFS: VFS::CVfsRemoteArchiveCore)
    CORE::CString blockCacheFolder;     /**< the folder of the persistent block cache; empty: the cache that the application made the default one in the VFS (CVfsGlobal), if any, else no cache. The same folder gives the same cache to every archive that names it */
    UInt64 blockCacheMaxBytes;          /**< the size the cache keeps to when this archive is the first to open the folder, 0 = no bound, default 0 */
    UInt32 blockCacheBlockBytes;        /**< the block size of the cache when this archive is the first to open the folder, 0 = the default of the cache (256 KiB) */
    CORE::CString cacheIdentity;        /**< the name of the share in the cache: the cache never interprets it. Empty: the address, drn://<session name>/<member>/<share> or <connection id>|<share>. Two mounts with one identity share the files they hold */
    UInt32 readAheadAfterBlocks;        /**< sequential blocks read before read ahead starts, default 2 */
    UInt32 readAheadBlocks;             /**< blocks fetched ahead in the same run, default 8; with a block cache this is the read ahead, the facility's chunk read ahead (readAheadChunks) is then not used */
    UInt32 maxBlocksPerFetch;           /**< blocks in one run asked of the share, default 16 */
    UInt32 maxBackgroundFetches;        /**< background fetches that run at once for this archive, default 8 */
    UInt32 maxQueuedFetches;            /**< background fetches that wait behind them, default 256; beyond that a miss answers retry-later and raises a failure event */
    CORE::CString fetchThreadPool;      /**< the thread pool the background fetches run on, default VfsRemoteOps */
    bool verifyCompleteFiles;           /**< a file that became complete in the cache is checked against its digest when one is known, default true */
    bool refetchPinned;                 /**< a kept (pinned) file whose current revision is not held completely is fetched again in the background while the link is up, default true */
    bool offlineListings;               /**< while the share cannot be reached a directory is listed from the files the cache holds, default true */
    bool openRequiresCompleteFile;      /**< a thread that must not wait opens a file only when every block is cached (else not-yet-available and a fetch of the whole file starts), default false */

    CDrnBridgeSettings( void );

    /** Reads the keys that are present in the archive specific settings of the archive settings; false when one is present and not usable */
    bool LoadFromArchiveSettings( const VFS::CArchiveSettings& settings );

    /** Reads the keys that are present in the list */
    bool LoadFromValueList( const CORE::CValueList& values );

    /**
     *  Writes every setting as an archive specific setting of an archive configuration node (the attributes "ArchiveSpecificSetting.<key>"),
     *  the form that VFS::CArchiveSettings::LoadConfig() reads, so that a mount can be made through the VFS with these settings
     */
    void SaveToArchiveConfig( CORE::CDataNode& archiveConfig ) const;

    /** Brings values that cannot work to ones that can (at least one open handle, ...) */
    void Normalize( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNBRIDGE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGESETTINGS_H ? */
