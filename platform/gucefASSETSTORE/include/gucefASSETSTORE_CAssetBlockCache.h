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

#ifndef GUCEF_ASSETSTORE_CASSETBLOCKCACHE_H
#define GUCEF_ASSETSTORE_CASSETBLOCKCACHE_H

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
 *  The persistent block cache of the files of a remote asset store service (remote service design 5.4, 6.6).
 *
 *  A file is kept as a sparse data file and an index in the cache folder, both named by the key of the VFS path of the file. The
 *  index records which blocks of BlockBytes the data file holds and a checksum of each, so an interrupted fetch continues with the
 *  missing blocks after a reconnect or a restart of the application, and a block that the disk gave back damaged is found when it
 *  is read (it is dropped and the caller fetches it again). The link is authenticated and encrypted, so blocks are not hashed on
 *  the wire: a file that was fetched completely is checked against its SHA-256 by the caller (VerifyDigest()).
 *
 *  The cache has a size limit. When the blocks held pass it, the entries that were used longest ago are removed, whole, except the
 *  ones that are pinned (kept offline) and the ones that are acquired at the moment. Everything is thread safe; a read of a block
 *  takes the lock of its entry only.
 *
 *  An entry is used with AcquireEntry() ... ReleaseEntry(). A file that is empty needs no entry (it has no blocks).
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetBlockCache
{
    public:

    enum EDefaults
    {
        BlockBytes = 262144,                    /**< the size of a block: the unit of the wire, the cache and the checksums */
        MaxPathBytes = 800                      /**< a longer VFS path is not cached (the reads go through without a cache entry) */
    };

    typedef UInt32 TEntryHandle;                /**< 0 is no entry */

    enum EReadResult
    {
        READ_HIT = 0,                           /**< the bytes are there */
        READ_MISS,                              /**< the block is not in the cache */
        READ_CORRUPT                            /**< the block was damaged on the disk: it is dropped, fetch it again */
    };

    struct GUCEF_ASSETSTORE_PUBLIC_CPP SEntryInfo
    {
        CORE::CString path;
        UInt64 fileSize;
        CORE::CString digest;                   /**< the SHA-256 of the whole file when it is known, lower case hexadecimal */
        UInt32 blockCount;
        UInt32 blocksHeld;
        UInt64 bytesHeld;
        bool pinned;
        bool complete;
        UInt64 lastUseMs;
        SEntryInfo( void );
    };
    typedef GUCEF::vector< SEntryInfo > TEntryInfoVector;

    struct GUCEF_ASSETSTORE_PUBLIC_CPP SStatistics
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
        SStatistics( void );
    };

    CAssetBlockCache( void );

    ~CAssetBlockCache();

    /** Takes the folder (made when it is not there), reads the entries that are in it; false with a logged reason when the folder is no good */
    bool Open( const CORE::CString& folder, UInt64 maxBytes );

    /** Flushes the indexes and lets go of everything; the files stay */
    void Close( void );

    bool IsOpen( void ) const;

    const CORE::CString& GetFolder( void ) const;

    /** The limit on the bytes held; a smaller one removes entries at once */
    void SetMaxBytes( UInt64 maxBytes );
    UInt64 GetMaxBytes( void ) const;
    UInt64 GetUsedBytes( void ) const;

    /**
     *  The entry of a file of this size (and digest, when it is known), made when it is not there. An entry of another size or digest
     *  is for another content of the path: it is emptied. 0 when the entry cannot be made (an empty file, a path that is too long, the
     *  disk) or when the entry is in use at the moment for another content
     */
    TEntryHandle AcquireEntry( const CORE::CString& vfsPath, UInt64 fileSize, const CORE::CString& digest );

    void ReleaseEntry( TEntryHandle entry );

    /** The digest of the whole file for an entry that has none yet */
    void SetEntryDigest( TEntryHandle entry, const CORE::CString& digest );

    UInt32 GetBlockCount( TEntryHandle entry ) const;
    UInt64 GetFileSize( TEntryHandle entry ) const;

    /** The size of block 'index' of the file: BlockBytes, less for the last one */
    UInt32 GetBlockSize( TEntryHandle entry, UInt32 index ) const;

    bool HasBlock( TEntryHandle entry, UInt32 index ) const;

    bool IsComplete( TEntryHandle entry ) const;

    UInt32 GetBlocksHeld( TEntryHandle entry ) const;

    /** Reads bytes of a block that is held: 'offsetInBlock' and 'length' stay within the block */
    EReadResult ReadBlock( TEntryHandle entry, UInt32 index, UInt32 offsetInBlock, void* dest, UInt32 length );

    /** Keeps a block: 'length' has to be the size of the block */
    bool StoreBlock( TEntryHandle entry, UInt32 index, const void* data, UInt32 length );

    /** Checks the whole file of a complete entry against its SHA-256; false for an entry that is not complete or has another digest */
    bool VerifyDigest( TEntryHandle entry, const CORE::CString& expectedDigest );

    /** Empties an entry: the blocks go (a file that fails its digest, a content that changed) */
    void ResetEntry( TEntryHandle entry );

    /** Whether the entry of a path is pinned */
    bool IsPinned( const CORE::CString& vfsPath ) const;

    /** Pins or unpins the entry of a path (made by AcquireEntry()): a pinned entry is never removed to make room */
    bool SetPinned( const CORE::CString& vfsPath, bool pinned );
    void SetEntryPinned( TEntryHandle entry, bool pinned );

    /** Removes the entry of a path (not when it is acquired); the bytes it held */
    UInt64 RemoveEntry( const CORE::CString& vfsPath );

    /** Removes every entry that is not pinned or acquired; the bytes freed */
    UInt64 Clear( void );

    bool GetEntryInfo( const CORE::CString& vfsPath, SEntryInfo& info ) const;
    void GetEntryInfos( TEntryInfoVector& infos ) const;

    /** Writes the indexes that changed to the disk */
    void Flush( void );

    void GetStatistics( SStatistics& statistics ) const;

    /** The CRC-32 of a block (the checksum of the index) */
    static UInt32 Checksum( const void* data, UInt32 length, UInt32 seed = 0 );

    /** The name of the files of a path (without the extension) */
    static CORE::CString MakeKey( const CORE::CString& vfsPath );

    private:

    CAssetBlockCache( const CAssetBlockCache& src );
    CAssetBlockCache& operator=( const CAssetBlockCache& src );

    struct SEntry;
    typedef GUCEF::map< CORE::CString, SEntry* > TEntryMap;
    typedef GUCEF::map< TEntryHandle, SEntry* > THandleMap;

    SEntry* Resolve( TEntryHandle entry ) const;
    SEntry* LoadEntryLocked( const CORE::CString& key );
    void ScanFolder( void );
    void EvictLocked( void );
    void DeleteEntryFilesLocked( SEntry* entry );
    bool InitEntry( SEntry& entry, UInt64 fileSize, const CORE::CString& digest );
    bool ReadIndex( SEntry& entry, bool tableToo ) const;
    void WriteHeader( SEntry& entry ) const;
    bool OpenData( SEntry& entry ) const;
    void CloseData( SEntry& entry ) const;
    UInt64 BytesOfBlocks( const SEntry& entry, UInt32 blocks ) const;

    private:

    mutable MT::CMutex m_lock;
    CORE::CString m_folder;
    bool m_open;
    UInt64 m_maxBytes;
    UInt64 m_usedBytes;
    TEntryMap m_entries;
    THandleMap m_handles;
    TEntryHandle m_nextHandle;
    MT::CTAtomic< UInt64 > m_blockHits;
    MT::CTAtomic< UInt64 > m_blockMisses;
    MT::CTAtomic< UInt64 > m_blocksStored;
    MT::CTAtomic< UInt64 > m_corruptBlocks;
    UInt64 m_evictedEntries;
    UInt64 m_evictedBytes;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETBLOCKCACHE_H ? */
