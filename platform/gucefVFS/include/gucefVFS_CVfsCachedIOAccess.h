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

#ifndef GUCEF_VFS_CVFSCACHEDIOACCESS_H
#define GUCEF_VFS_CVFSCACHEDIOACCESS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_VFS_CVFSBLOCKCACHE_H
#include "gucefVFS_CVfsBlockCache.h"
#define GUCEF_VFS_CVFSBLOCKCACHE_H
#endif /* GUCEF_VFS_CVFSBLOCKCACHE_H ? */

#ifndef GUCEF_VFS_CIVFSBLOCKSOURCE_H
#include "gucefVFS_CIVfsBlockSource.h"
#define GUCEF_VFS_CIVFSBLOCKSOURCE_H
#endif /* GUCEF_VFS_CIVFSBLOCKSOURCE_H ? */

#ifndef GUCEF_VFS_CVFSREMOTEENTRY_H
#include "gucefVFS_CVfsRemoteEntry.h"
#define GUCEF_VFS_CVFSREMOTEENTRY_H
#endif /* GUCEF_VFS_CVFSREMOTEENTRY_H ? */

#ifndef GUCEF_VFS_CVFSCALLRESULT_H
#include "gucefVFS_CVfsCallResult.h"
#define GUCEF_VFS_CVFSCALLRESULT_H
#endif /* GUCEF_VFS_CVFSCALLRESULT_H ? */

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

class CVfsRemoteArchiveCore;

/**
 *  A read only access to a file of a remote place that reads through the block cache: blocks that are held are read
 *  from the cache, a block that is missing is fetched from the remote place, and what a miss does depends on the thread that
 *  reads (the rules of CVfsThreadPolicy) and on the link:
 *
 *  - a thread that waits: the missing blocks are fetched in one run of up to maxBlocksPerFetch blocks, with read ahead after
 *    a few sequential blocks, stored in the cache and read;
 *  - a thread that must not wait (FAIL_AND_FETCH): the read stops at the first missing block and returns the bytes it has, the
 *    call records FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE and a background job fetches the run; an arrival event follows.
 *    FAIL does the same and starts no job;
 *  - the link is down: the read stops at the first missing block and records FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING with
 *    a message that names the file, the bytes and the place.
 *
 *  A read that ends before the end of the file because of a miss is a failure and GetLastResult() says why: Eof() stays
 *  false until the position is at the size. A consumer that takes a short read for the end of the file must not read
 *  under FAIL_AND_FETCH (or it asks the policy to open complete files only).
 *
 *  The access is pinned to the revision of the file when it was opened: its bytes never come from another revision. When
 *  the remote place says that the file changed, the access fails for good (IsValid() false) and the archive raises FileChangedEvent;
 *  opening the path again gives the new revision. The bytes of a block that the cache could not keep are served from a
 *  window in memory. One thread reads at a time (the access has a lock); several accesses to a file read at the same time.
 */
class GUCEF_VFS_PUBLIC_CPP CVfsCachedIOAccess : public CORE::CIOAccess
{
    public:

    /**
     *  @param core         the archive layer this is a file of; the access keeps it alive
     *  @param path         the path of the file in the archive
     *  @param entry        what the remote place said of the file: size, revision, digest
     *  @param cacheEntry   the entry of the block cache for this revision, acquired for this access, which releases it; 0 for none
     *  @param source       where missing bytes come from; none when the link was down at the open: it is made when it is needed
     */
    CVfsCachedIOAccess( const CORE::CTSharedPtr< CVfsRemoteArchiveCore, MT::CMutex >& core ,
                        const CORE::CString& path                                         ,
                        const CVfsRemoteEntry& entry                                      ,
                        CVfsBlockCache::TEntryHandle cacheEntry                           ,
                        const CIVfsBlockSourcePtr& source                                 );

    virtual ~CVfsCachedIOAccess() GUCEF_VIRTUAL_OVERRIDE;

    const CORE::CString& GetPath( void ) const;

    /** The revision of the file this access is pinned to */
    const CORE::CString& GetRevision( void ) const;

    /**
     *  Whether the access reads an older revision of the file than the current one: set when the link was down and the path is kept offline
     *  (pinned) and its newest revision is not held completely, so that the older complete copy answers. The bytes are those of GetRevision(), complete and
     *  checked as when they were fetched; a reader that needs the current revision looks at this and asks again when the link is back.
     */
    bool IsStaleRevision( void ) const;
    void SetStaleRevision( bool stale );

    /** Why the latest read ended short, or the failure that ended the access; a success when the latest read went well */
    CVfsCallResult GetLastResult( void ) const;

    virtual void Open( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Opened( void ) const GUCEF_VIRTUAL_OVERRIDE;

    using CORE::CIOAccess::Read;
    using CORE::CIOAccess::Write;

    virtual UInt32 Read( void* dest, UInt32 esize, UInt32 elements ) GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt64 Tell( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual Int32 Seek( Int64 offset, Int32 origin ) GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt32 Setpos( UInt64 position ) GUCEF_VIRTUAL_OVERRIDE;
    virtual char GetChar( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Eof( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsReadable( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsWriteable( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsValid( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt64 GetSize( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void Flush( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt32 Write( const void* srcdata, UInt32 esize, UInt32 elements ) GUCEF_VIRTUAL_OVERRIDE;

    /** Another access to the same file at the same revision and position; it makes its own source once it needs one */
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CVfsCachedIOAccess( const CVfsCachedIOAccess& src );                    /**< used by Clone() only */
    CVfsCachedIOAccess& operator=( const CVfsCachedIOAccess& src );         /**< not implemented */

    UInt32 BlockLength( UInt32 block ) const;
    bool BlockAvailable( UInt64 position, UInt32 block ) const;
    bool CopyFromWindow( UInt64 position, void* dest, UInt32 length ) const;
    void NoteBlocksRead( UInt32 firstBlock, UInt32 lastBlock );
    void ComputeRun( UInt32 block, UInt32 lastBlock, UInt32& firstOfRun, UInt32& countOfRun, UInt32& aheadBlocks ) const;
    bool HandleMiss( UInt64 position, UInt64 remaining );
    void ReleaseCacheEntry( void );

    private:

    mutable MT::CMutex m_lock;
    CORE::CTSharedPtr< CVfsRemoteArchiveCore, MT::CMutex > m_core;
    CVfsBlockCachePtr m_cache;
    CORE::CString m_identity;
    CORE::CString m_path;
    CORE::CString m_revision;
    UInt64 m_size;
    CVfsDigest m_digest;
    UInt32 m_blockBytes;
    UInt32 m_blockCount;
    CVfsBlockCache::TEntryHandle m_cacheEntry;
    CIVfsBlockSourcePtr m_source;
    UInt64 m_position;
    bool m_closed;
    bool m_failed;
    UInt32 m_sequentialBlocks;
    UInt32 m_sequentialForRead;             /**< of those, the blocks that this read goes on from: 0 for a read that starts somewhere else */
    UInt32 m_lastBlockRead;
    bool m_hasLastBlock;
    CORE::CDynamicBuffer m_window;
    UInt64 m_windowStart;
    CVfsCallResult m_lastResult;
    bool m_fetchFailedInRead;               /**< the fetch of the read that is under way failed: the read ends at the next block that is missing, and says why */
    CVfsCallResult m_fetchFailure;
    bool m_staleRevision;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSCACHEDIOACCESS_H ? */
