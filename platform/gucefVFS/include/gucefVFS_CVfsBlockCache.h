/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
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

#ifndef GUCEF_VFS_CVFSBLOCKCACHE_H
#define GUCEF_VFS_CVFSBLOCKCACHE_H

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

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_VFS_CVFSDIGEST_H
#include "gucefVFS_CVfsDigest.h"
#define GUCEF_VFS_CVFSDIGEST_H
#endif /* GUCEF_VFS_CVFSDIGEST_H ? */

#ifndef GUCEF_VFS_MACROS_H
#include "gucefVFS_macros.h"
#define GUCEF_VFS_MACROS_H
#endif /* GUCEF_VFS_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The persistent block cache of the files of remote or slow archives.
 *
 *  A file is kept as a sparse data file and an index in the cache folder. The index records which blocks of
 *  GetBlockBytes() the data file holds and a CRC-32 of each, so an interrupted fetch continues with the missing blocks after a
 *  reconnect or a restart, and a block that the disk gave back damaged is found when it is first read in a run (it is
 *  dropped and the caller fetches it again). The CRC is not a security measure: whatever carries the bytes to the cache
 *  verifies them (and a complete file is checked against its digest with VerifyDigest()).
 *
 *  An entry is one revision of one file of one remote place: the key is (identity, path, revision). Two revisions of
 *  a path are two entries, so a reader of the old revision keeps its entry while a reader of the new one gets another, and no
 *  byte of one revision is ever read as a byte of another. The identity is a name that the archive gives to the remote place
 *  (the cache never interprets it); the revision is an opaque string of the archive.
 *
 *  Layout of the folder:
 *      cache.lock                      held while a process has the folder open
 *      xx/key.vbi                      the index of an entry, xx = the first byte of the key in hexadecimal
 *      xx/key.vbd                      the data of an entry at the offsets of the file
 *  Only files of this naming below the two character folders are looked at: anything else in the folder (for example
 *  the files of another cache format) is left alone.
 *
 *  The cache has a size limit on the bytes of the blocks it holds. When the bytes pass it, entries are removed whole:
 *  first the superseded ones (a newer revision of their path is known), least recently used first, then the
 *  other ones, least recently used first. Pinned entries (kept offline) and acquired entries are never removed, so the limit is a
 *  soft one. A limit of 0 means no limit.
 *
 *  Everything is thread safe. A read or a store of a block takes the lock of its entry only; no call waits for anything but
 *  the local disk. At most 'maxOpenFiles' data files are open at a time: an entry is reopened when it is touched after its
 *  file was closed to make room, so a process that holds thousands of acquired entries stays within the descriptor limit.
 *  The index of an entry is opened when its rows are written (every 16 stored blocks, at release and at Close()), not kept open.
 *
 *  One process per folder: Open() takes cache.lock with an exclusive create and Close() removes it. The lock of a
 *  process that did not get to close (a crash) is left behind and has to be broken by the application on purpose,
 *  see Open().
 *
 *  Platform notes: the data file is sparse where the file system supports it (Linux file systems, NTFS only when the file is
 *  marked sparse, which this class does not do, FAT and exFAT never): without it a partly cached large file takes the disk
 *  space of its highest block. The index is written in host byte order: a cache folder is not moved between machines.
 *  Built and tested on Linux only.
 */
class GUCEF_VFS_PUBLIC_CPP CVfsBlockCache
{
    public:

    enum EDefaults
    {
        DefaultBlockBytes = 262144,             /**< the size of a block: the unit of the checksums and, for a remote source, of a fetch */
        MinBlockBytes = 65536,
        MaxBlockBytes = 4194304,
        DefaultMaxOpenFiles = 128,
        MaxIdentityBytes = 256,                 /**< longer identities, revisions and paths are not cached (the entry cannot be made) */
        MaxRevisionBytes = 256,
        MaxPathBytes = 1024
    };

    typedef UInt32 TEntryHandle;                /**< 0 is no entry */

    enum EPinResult
    {
        PIN_OK = 0,                             /**< the path is pinned (it was already) */
        PIN_NO_ENTRY,                           /**< the cache holds no current revision of the path to pin, or is closed */
        PIN_OVER_LIMIT                          /**< the pinned files would not fit the limit: nothing was pinned, nothing was dropped */
    };

    enum EReadResult
    {
        READ_HIT = 0,                           /**< the bytes are there */
        READ_MISS,                              /**< the block is not in the cache */
        READ_CORRUPT                            /**< the block was damaged on the disk: it is dropped, fetch it again */
    };

    enum EVerifyResult
    {
        VERIFY_MATCH = 0,                       /**< the content has the digest */
        VERIFY_MISMATCH,                        /**< the content does not have the digest: the entry is not changed, the caller decides (ResetEntry()) */
        VERIFY_INCOMPLETE,                      /**< not every block is held (a damaged one found while hashing is dropped, which makes the entry incomplete) */
        VERIFY_UNSUPPORTED                      /**< the digest has no known algorithm or is not well formed */
    };

    struct GUCEF_VFS_PUBLIC_CPP SEntryInfo
    {
        CORE::CString identity;
        CORE::CString path;
        CORE::CString revision;
        UInt64 fileSize;
        CVfsDigest digest;                      /**< the digest of the whole file when it is known */
        UInt32 blockCount;
        UInt32 blocksHeld;                      /**< of an entry whose table is not loaded yet: what its index said when it was last written */
        UInt64 bytesHeld;
        bool pinned;
        bool complete;
        bool superseded;                        /**< a newer revision of the path is known */
        bool staleRevision;                     /**< set by the offline lookups only: the entry is an older, complete, pinned revision that stands in for a newer one that is not held completely */
        bool digestVerified;                    /**< the content was hashed and has 'digest' */
        UInt64 lastUseMs;                       /**< milliseconds on a clock of this run: only for comparisons, 0 for an entry not used in this run */
        UInt64 createdMs;                       /**< when the entry was made, milliseconds since the Unix epoch (UTC); the order of the revisions of a path */
        SEntryInfo( void );
    };
    typedef GUCEF::vector< SEntryInfo > TEntryInfoVector;

    struct GUCEF_VFS_PUBLIC_CPP SStatistics
    {
        UInt64 maxBytes;
        UInt64 usedBytes;
        UInt64 pinnedBytes;
        UInt32 entries;
        UInt32 pinnedEntries;
        UInt64 blockHits;
        UInt64 blockMisses;
        UInt64 blocksStored;
        UInt64 corruptBlocks;
        UInt64 evictedEntries;
        UInt64 evictedBytes;
        UInt64 supersededEvictions;             /**< of the evicted entries, the ones that were superseded */
        UInt64 pinsRefused;                     /**< pins that were refused because the pinned files would not fit the limit */
        UInt64 pinnedReservedBytes;             /**< the file sizes of the pinned paths (the newest pinned revision of each): what the pins have taken of the limit */
        UInt64 storeFailures;                   /**< blocks that could not be kept (the disk, the folder) */
        UInt32 openFiles;                       /**< data files open now */
        UInt64 fileReopens;                     /**< data files opened again after they were closed to make room */
        SStatistics( void );
    };

    CVfsBlockCache( void );

    ~CVfsBlockCache();

    /**
     *  Takes the folder (made when it is not there), takes cache.lock and reads what the folder holds.
     *  False, with the reason logged, when the folder is no good, the block size is not a power of two between MinBlockBytes and
     *  MaxBlockBytes, or another process (or another cache object of this process) has the folder.
     *
     *  The lock of a process that crashed is left behind and the cache cannot tell that process is gone: Open() refuses, and
     *  names the process id and the start time that the lock file holds. The application decides, after asking the user,
     *  to open again with 'breakStaleLock' true, which removes the lock first.
     *
     *  Opening reads the first part of the index of every entry (not the block tables, and no data file): an entry
     *  whose index is cut short, has the wrong magic, version, checksum or block size is removed with its data file,
     *  and so is a data file without an index. Nothing else in the folder is touched.
     */
    bool Open( const CORE::CString& folder, UInt64 maxBytes, UInt32 blockBytes = DefaultBlockBytes, UInt32 maxOpenFiles = DefaultMaxOpenFiles, bool breakStaleLock = false );

    /** Writes everything that is not written yet, lets go of everything and removes cache.lock; the files stay. Entries that are acquired then are invalid */
    void Close( void );

    bool IsOpen( void ) const;

    const CORE::CString& GetFolder( void ) const;

    /** The limit on the bytes held; a smaller one removes entries at once; 0 is no limit */
    void SetMaxBytes( UInt64 maxBytes );
    UInt64 GetMaxBytes( void ) const;
    UInt64 GetUsedBytes( void ) const;
    UInt32 GetBlockBytes( void ) const;
    UInt32 GetMaxOpenFiles( void ) const;

    /**
     *  The entry of a revision of a file of this size (and digest, when it is known). 'create' false only finds an entry (an offline
     *  read must not make entries). An entry exists once per (identity, path, revision): every caller gets the same handle and the entry
     *  stays while at least one of them has not released it. 0 when there is no entry: an empty file (it has no blocks),
     *  a part of the key that is too long or holds a line feed, the disk, a closed cache, or an entry of that key that holds another
     *  size or another digest of the same algorithm while someone has it acquired (a revision that does not identify its content;
     *  with nobody holding it the entry is emptied and used for the new content).
     */
    TEntryHandle AcquireEntry( const CORE::CString& identity, const CORE::CString& path, const CORE::CString& revision,
                               UInt64 fileSize, const CVfsDigest& digest, bool create );

    void ReleaseEntry( TEntryHandle entry );

    /**
     *  The newest revision of a path that the cache holds, that is not superseded, without knowing its size or revision: what
     *  an offline open starts from. The block counts of an entry that was not used in this run are those its index had when it was written.
     */
    bool FindNewestRevision( const CORE::CString& identity, const CORE::CString& path, SEntryInfo& info ) const;

    /**
     *  What an offline open of a path is served from. The newest revision that is not superseded when the cache holds it completely;
     *  when it does not (its blocks are not all held, or the cache holds no such revision) and an older revision of the path is pinned
     *  and complete, that older revision, with 'staleRevision' set in 'info'; else what FindNewestRevision() gives. A copy that is
     *  not pinned is never served in place of a newer revision.
     */
    bool FindOfflineRevision( const CORE::CString& identity, const CORE::CString& path, SEntryInfo& info ) const;

    /**
     *  The entries (newest not superseded revision of each path) of the files below a directory: the offline listing. 'dirPath' is
     *  relative to the share like the paths ("" is the root), not recursive lists the files directly in the directory.
     */
    void GetEntryInfosBelow( const CORE::CString& identity, const CORE::CString& dirPath, bool recursive, TEntryInfoVector& infos ) const;

    bool GetEntryInfo( TEntryHandle entry, SEntryInfo& info ) const;

    UInt32 GetBlockCount( TEntryHandle entry ) const;
    UInt64 GetFileSize( TEntryHandle entry ) const;

    /** The size of block 'index' of the file: GetBlockBytes(), less for the last one */
    UInt32 GetBlockSize( TEntryHandle entry, UInt32 index ) const;

    bool HasBlock( TEntryHandle entry, UInt32 index ) const;

    bool IsComplete( TEntryHandle entry ) const;

    UInt32 GetBlocksHeld( TEntryHandle entry ) const;

    /** Reads bytes of a block that is held: 'offsetInBlock' and 'length' stay within the block */
    EReadResult ReadBlock( TEntryHandle entry, UInt32 index, UInt32 offsetInBlock, void* dest, UInt32 length );

    /**
     *  Keeps a block: 'length' has to be the size of the block. False when the cache cannot keep it (the disk is full or gone,
     *  the folder cannot be written, no file could be opened): the caller carries on without the cache for these bytes.
     */
    bool StoreBlock( TEntryHandle entry, UInt32 index, const void* data, UInt32 length );

    /**
     *  Checks a complete entry against a digest of the file (MD5, SHA-1 or SHA-256) by hashing its content, block checksums included.
     *  A match is kept in the index (the digest and that it was verified), so the next run and the next call with the same digest
     *  do not hash again.
     */
    EVerifyResult VerifyDigest( TEntryHandle entry, const CVfsDigest& expected );

    /** The digest of the whole file, as far as it is known: it is not verified until VerifyDigest() says so. Ignored when the entry has a digest already */
    void SetEntryDigest( TEntryHandle entry, const CVfsDigest& digest );

    /** Empties an entry: the blocks and the digest go (a file that fails its digest, a content that changed); pins and the superseded flag stay */
    void ResetEntry( TEntryHandle entry );

    /** Pins or unpins the path: a pin goes on the newest revision that is not superseded and needs an entry; unpinning takes the pin off every revision */
    bool SetPinned( const CORE::CString& identity, const CORE::CString& path, bool pinned );

    /**
     *  Pins the newest revision of the path that is not superseded, and says why when it cannot. A pin is an opt-in promise to keep the
     *  file: the pinned entries are never removed to make room, and a pin is never dropped to make room for another. A limit above 0 is
     *  therefore kept by refusing a new pin (PIN_OVER_LIMIT) when the file sizes of the pinned paths, with this one, would pass the limit; a path that
     *  has a pin already (a pin that follows the path to a newer revision) is not refused. A limit that is lowered below what the pins
     *  hold leaves the pins as they are (the limit is soft for them) and entries that are not pinned go first.
     */
    EPinResult TryPin( const CORE::CString& identity, const CORE::CString& path );

    /**
     *  Whether a revision of the path is pinned. A pin follows the path: when a newer revision of a pinned path becomes complete the pin goes
     *  to it and the older revision is unpinned and superseded.
     */
    bool IsPinned( const CORE::CString& identity, const CORE::CString& path ) const;

    /**
     *  Whether the path has a pinned revision that a newer revision has superseded: the pin has not moved yet because the newer revision is not
     *  held completely. This is what an application that keeps files offline has to fetch again (the pin moves by itself when it is complete).
     */
    bool HasStalePinnedCopy( const CORE::CString& identity, const CORE::CString& path ) const;

    /** The paths of the place 'identity' for which HasStalePinnedCopy() is true */
    void GetStalePinnedPaths( const CORE::CString& identity, CORE::CString::StringVector& paths ) const;

    /** Marks the other revisions of a path as superseded (the archive learned the current revision) and the current one as not; the number marked now */
    UInt32 SupersedeOtherRevisions( const CORE::CString& identity, const CORE::CString& path, const CORE::CString& keepRevision );

    /** Removes the entries of a path, all revisions, pinned ones too, except the acquired ones; the bytes freed */
    UInt64 RemoveEntries( const CORE::CString& identity, const CORE::CString& path );

    /** Removes every entry that is not pinned or acquired; the bytes freed */
    UInt64 Clear( void );

    /** Writes what is not written yet to the disk */
    void Flush( void );

    void GetStatistics( SStatistics& statistics ) const;

    /** The CRC-32 of a block (the checksum of the index) */
    static UInt32 Checksum( const void* data, UInt32 length, UInt32 seed = 0 );

    /** The name of the files of an entry (without folder and extension): 32 lower case hexadecimal characters */
    static CORE::CString MakeKey( const CORE::CString& identity, const CORE::CString& path, const CORE::CString& revision );

    private:

    CVfsBlockCache( const CVfsBlockCache& src );
    CVfsBlockCache& operator=( const CVfsBlockCache& src );

    struct SEntry;
    typedef GUCEF::map< CORE::CString, SEntry* > TEntryMap;                     /**< by key */
    typedef GUCEF::map< TEntryHandle, SEntry* > THandleMap;
    typedef GUCEF::vector< SEntry* > TEntryVector;
    typedef GUCEF::map< CORE::CString, TEntryVector > TPathMap;                 /**< by identity, line feed and path: the revisions of a path */
    typedef GUCEF::map< UInt64, SEntry* > TLruMap;                              /**< by the sequence number of the last use */

    SEntry* Resolve( TEntryHandle entry ) const;
    SEntry* FindEntryLocked( const CORE::CString& key, const CORE::CString& identity, const CORE::CString& path, const CORE::CString& revision ) const;
    SEntry* NewestOfPathLocked( const CORE::CString& pathKey, bool includeSuperseded ) const;
    SEntry* SelectOfflineLocked( const CORE::CString& pathKey, bool& stale ) const;
    UInt64 PinnedReservedBytesLocked( void ) const;
    void ScanFolder( void );
    bool LoadStubLocked( const CORE::CString& key, UInt64 useRank );
    void RegisterEntryLocked( SEntry* entry );
    void UnregisterEntryLocked( SEntry* entry );
    void LruInsertLocked( SEntry* entry );
    void LruRemoveLocked( SEntry* entry );
    void EvictLocked( void );
    void DeleteEntryLocked( SEntry* entry, bool entryCounted );
    SEntry* CreateEntryLocked( const CORE::CString& identity, const CORE::CString& path, const CORE::CString& revision, UInt64 fileSize, const CVfsDigest& digest );
    void RemoveEntryFiles( const CORE::CString& key ) const;
    CORE::CString IndexPath( const CORE::CString& key ) const;
    CORE::CString DataPath( const CORE::CString& key ) const;
    bool WriteNewIndex( SEntry& entry ) const;
    void BuildHeader( const SEntry& entry, UInt8* header ) const;
    bool EnsureTableLocked( SEntry& entry );
    bool EnsureOpenLocked( SEntry& entry );
    void CloseDataLocked( SEntry& entry );
    bool PersistLocked( SEntry& entry );
    bool EmptyEntryLocked( SEntry& entry );
    void DropBlockLocked( SEntry& entry, UInt32 index );
    int ReadFromBlockLocked( SEntry& entry, UInt32 index, UInt32 offsetInBlock, void* dest, UInt32 length );
    void OnEntryCompleteLocked( SEntry* entry );
    void FillInfo( const SEntry& entry, SEntryInfo& info ) const;
    bool TakeFolderLock( bool breakStaleLock );
    void ReleaseFolderLock( void );
    UInt32 SetSupersededLocked( SEntry& entry, bool superseded );
    UInt64 NextUseSeq( void );
    UInt64 NextCreatedMs( void );

    private:

    mutable MT::CMutex m_lock;                  /**< the maps, the lru order and the settings; taken before an entry lock, never while holding one */
    mutable MT::CMutex m_handleLock;            /**< the handle map only, held for a lookup: a read or a store of a block finds its entry with it */
    mutable MT::CMutex m_openLock;              /**< the ring of entries with an open data file; the last lock in the order */
    CORE::CString m_folder;
    bool m_open;
    bool m_lockHeld;
    UInt64 m_maxBytes;
    UInt32 m_blockBytes;
    UInt32 m_maxOpenFiles;
    TEntryMap m_entries;
    THandleMap m_handles;
    TPathMap m_byPath;
    TLruMap m_lruNormal;
    TLruMap m_lruSuperseded;
    TEntryHandle m_nextHandle;
    MT::CTAtomic< UInt64 > m_useCounter;
    UInt64 m_lastCreatedMs;
    TEntryVector m_openRing;
    UInt32 m_clockHand;
    bool m_fanOutKnown[ 256 ];
    MT::CTAtomic< UInt64 > m_usedBytes;
    MT::CTAtomic< UInt64 > m_blockHits;
    MT::CTAtomic< UInt64 > m_blockMisses;
    MT::CTAtomic< UInt64 > m_blocksStored;
    MT::CTAtomic< UInt64 > m_corruptBlocks;
    MT::CTAtomic< UInt64 > m_evictedEntries;
    MT::CTAtomic< UInt64 > m_evictedBytes;
    MT::CTAtomic< UInt64 > m_supersededEvictions;
    MT::CTAtomic< UInt64 > m_storeFailures;
    MT::CTAtomic< UInt64 > m_fileReopens;
    MT::CTAtomic< UInt64 > m_pinsRefused;
};

typedef CORE::CTSharedPtr< CVfsBlockCache, MT::CMutex > CVfsBlockCachePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSBLOCKCACHE_H ? */
