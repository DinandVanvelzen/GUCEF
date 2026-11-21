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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_VFS_CVFSBLOCKRUNFETCHER_H
#include "gucefVFS_CVfsBlockRunFetcher.h"
#define GUCEF_VFS_CVFSBLOCKRUNFETCHER_H
#endif /* GUCEF_VFS_CVFSBLOCKRUNFETCHER_H ? */

#ifndef GUCEF_VFS_CVFSREMOTEARCHIVECORE_H
#include "gucefVFS_CVfsRemoteArchiveCore.h"
#define GUCEF_VFS_CVFSREMOTEARCHIVECORE_H
#endif /* GUCEF_VFS_CVFSREMOTEARCHIVECORE_H ? */

#include "gucefVFS_CVfsCachedIOAccess.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVfsCachedIOAccess::CVfsCachedIOAccess( const CORE::CTSharedPtr< CVfsRemoteArchiveCore, MT::CMutex >& core ,
                                        const CORE::CString& path                                         ,
                                        const CVfsRemoteEntry& entry                                      ,
                                        CVfsBlockCache::TEntryHandle cacheEntry                           ,
                                        const CIVfsBlockSourcePtr& source                                 )
    : CORE::CIOAccess()
    , m_lock()
    , m_core( core )
    , m_cache( core->GetBlockCache() )
    , m_identity( core->GetIdentity() )
    , m_path( path )
    , m_revision( entry.revision )
    , m_size( entry.size )
    , m_digest( entry.digest )
    , m_blockBytes( m_cache.IsNULL() ? (UInt32) CVfsBlockCache::DefaultBlockBytes : m_cache->GetBlockBytes() )
    , m_blockCount( 0 )
    , m_cacheEntry( cacheEntry )
    , m_source( source )
    , m_position( 0 )
    , m_closed( false )
    , m_failed( false )
    , m_sequentialBlocks( 0 )
    , m_sequentialForRead( 0 )
    , m_lastBlockRead( 0 )
    , m_hasLastBlock( false )
    , m_window()
    , m_windowStart( 0 )
    , m_lastResult()
    , m_fetchFailedInRead( false )
    , m_fetchFailure()
    , m_staleRevision( false )
{GUCEF_TRACE;

    m_blockCount = (UInt32) ( ( m_size + m_blockBytes - 1 ) / m_blockBytes );
}

/*-------------------------------------------------------------------------*/

CVfsCachedIOAccess::CVfsCachedIOAccess( const CVfsCachedIOAccess& src )
    : CORE::CIOAccess( src )
    , m_lock()
    , m_core( src.m_core )
    , m_cache( src.m_cache )
    , m_identity( src.m_identity )
    , m_path( src.m_path )
    , m_revision( src.m_revision )
    , m_size( src.m_size )
    , m_digest( src.m_digest )
    , m_blockBytes( src.m_blockBytes )
    , m_blockCount( src.m_blockCount )
    , m_cacheEntry( 0 )
    , m_source()
    , m_position( src.m_position )
    , m_closed( src.m_closed )
    , m_failed( src.m_failed )
    , m_sequentialBlocks( 0 )
    , m_sequentialForRead( 0 )
    , m_lastBlockRead( 0 )
    , m_hasLastBlock( false )
    , m_window()
    , m_windowStart( 0 )
    , m_lastResult( src.m_lastResult )
    , m_fetchFailedInRead( false )
    , m_fetchFailure()
    , m_staleRevision( src.m_staleRevision )
{GUCEF_TRACE;

    if ( !m_cache.IsNULL() && 0 != src.m_cacheEntry )
        m_cacheEntry = m_cache->AcquireEntry( m_identity, m_path, m_revision, m_size, m_digest, false );
}

/*-------------------------------------------------------------------------*/

CVfsCachedIOAccess::~CVfsCachedIOAccess()
{GUCEF_TRACE;

    ReleaseCacheEntry();
}

/*-------------------------------------------------------------------------*/

void
CVfsCachedIOAccess::ReleaseCacheEntry( void )
{GUCEF_TRACE;

    if ( 0 != m_cacheEntry && !m_cache.IsNULL() )
        m_cache->ReleaseEntry( m_cacheEntry );
    m_cacheEntry = 0;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CVfsCachedIOAccess::GetPath( void ) const
{GUCEF_TRACE;

    return m_path;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CVfsCachedIOAccess::GetRevision( void ) const
{GUCEF_TRACE;

    return m_revision;
}

/*-------------------------------------------------------------------------*/

bool
CVfsCachedIOAccess::IsStaleRevision( void ) const
{GUCEF_TRACE;

    return m_staleRevision;
}

/*-------------------------------------------------------------------------*/

void
CVfsCachedIOAccess::SetStaleRevision( bool stale )
{GUCEF_TRACE;

    m_staleRevision = stale;
}

/*-------------------------------------------------------------------------*/

CVfsCallResult
CVfsCachedIOAccess::GetLastResult( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_lastResult;
}

/*-------------------------------------------------------------------------*/

void
CVfsCachedIOAccess::Open( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CVfsCachedIOAccess::Close( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_closed = true;
    m_source.Unlink();
    m_window.Clear( false );
    ReleaseCacheEntry();
}

/*-------------------------------------------------------------------------*/

bool
CVfsCachedIOAccess::Opened( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return !m_closed;
}

/*-------------------------------------------------------------------------*/

UInt32
CVfsCachedIOAccess::BlockLength( UInt32 block ) const
{GUCEF_TRACE;

    const UInt64 start = (UInt64) block * m_blockBytes;
    if ( start >= m_size )
        return 0;
    const UInt64 left = m_size - start;
    return left < m_blockBytes ? (UInt32) left : m_blockBytes;
}

/*-------------------------------------------------------------------------*/

bool
CVfsCachedIOAccess::CopyFromWindow( UInt64 position, void* dest, UInt32 length ) const
{GUCEF_TRACE;

    const UInt64 windowSize = m_window.GetDataSize();
    if ( 0 == windowSize || position < m_windowStart || position + length > m_windowStart + windowSize )
        return false;
    memcpy( dest, static_cast< const UInt8* >( m_window.GetConstBufferPtr() ) + ( position - m_windowStart ), length );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVfsCachedIOAccess::BlockAvailable( UInt64 position, UInt32 block ) const
{GUCEF_TRACE;

    const UInt32 length = BlockLength( block );
    if ( 0 == length )
        return false;
    const UInt64 windowSize = m_window.GetDataSize();
    if ( windowSize > 0 && position >= m_windowStart && position < m_windowStart + windowSize )
        return true;
    return 0 != m_cacheEntry && m_cache->HasBlock( m_cacheEntry, block );
}

/*-------------------------------------------------------------------------*/

void
CVfsCachedIOAccess::NoteBlocksRead( UInt32 firstBlock, UInt32 lastBlock )
{GUCEF_TRACE;

    // sequential: this read starts in the block the last one ended in, or in the next one
    if ( m_hasLastBlock && ( firstBlock == m_lastBlockRead || firstBlock == m_lastBlockRead + 1 ) )
        m_sequentialBlocks += ( lastBlock - firstBlock ) + ( firstBlock == m_lastBlockRead ? 0 : 1 );
    else
        m_sequentialBlocks = ( lastBlock - firstBlock ) + 1;
    m_lastBlockRead = lastBlock;
    m_hasLastBlock = true;
}

/*-------------------------------------------------------------------------*/

void
CVfsCachedIOAccess::ComputeRun( UInt32 block, UInt32 lastBlock, UInt32& firstOfRun, UInt32& countOfRun, UInt32& aheadBlocks ) const
{GUCEF_TRACE;

    const CVfsRemoteSettings& settings = m_core->GetSettings();
    const UInt32 maxBlocks = settings.maxBlocksPerFetch < 1 ? 1 : settings.maxBlocksPerFetch;

    // the run covers the blocks of the read that are missing, in order: it stops at a block that is held (which is never asked for again)
    firstOfRun = block;
    UInt32 last = block;
    while ( last < lastBlock && last - firstOfRun + 1 < maxBlocks && ( 0 == m_cacheEntry || !m_cache->HasBlock( m_cacheEntry, last + 1 ) ) )
        ++last;

    // read ahead: after a few sequential blocks, more in the same run, never past the end of the file and never over a block that is held
    aheadBlocks = 0;
    if ( last == lastBlock && m_sequentialForRead >= settings.readAheadAfterBlocks && settings.readAheadBlocks > 0 && 0 != m_cacheEntry )
    {
        while ( aheadBlocks < settings.readAheadBlocks && last + 1 < m_blockCount && !m_cache->HasBlock( m_cacheEntry, last + 1 ) )
        {
            ++last;
            ++aheadBlocks;
        }
    }
    countOfRun = last - firstOfRun + 1;
}

/*-------------------------------------------------------------------------*/

bool
CVfsCachedIOAccess::HandleMiss( UInt64 position, UInt64 remaining )
{GUCEF_TRACE;

    // a fetch of this read failed: what it left is read, the rest of the read is not asked for again
    if ( m_fetchFailedInRead )
    {
        m_lastResult = m_fetchFailure;
        return false;
    }

    CVfsRemoteCounters& counters = m_core->GetCounters();
    const UInt32 block = (UInt32) ( position / m_blockBytes );
    const UInt32 lastBlock = (UInt32) ( ( position + remaining - 1 ) / m_blockBytes );

    // the link is down: what is not in the cache is not available, and no job is made for it
    if ( !m_core->IsLinkUp() )
    {
        counters.offlineMisses.FetchAdd( 1 );
        m_lastResult = CVfsCallResult( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING, m_core->DescribeOffline( m_path, position, position + remaining - 1 ) );
        return false;
    }

    const CVfsThreadPolicy policy = m_core->GetCurrentThreadPolicy();
    if ( policy.NeverWaits() )
    {
        counters.policyMisses.FetchAdd( 1 );
        if ( 0 == m_cacheEntry )
        {
            m_lastResult = CVfsCallResult( CORE::FILESYSTEMACCESS_NOT_SUPPORTED, "the bytes of \"" + m_path + "\" are not held and there is no block cache to fetch them into" );
            return false;
        }
        if ( MISSACTION_FAIL_AND_FETCH == policy.missAction )
        {
            UInt32 firstOfRun = 0, countOfRun = 0, ahead = 0;
            ComputeRun( block, lastBlock, firstOfRun, countOfRun, ahead );
            CVfsRemoteEntry entry;
            entry.exists = true;
            entry.size = m_size;
            entry.revision = m_revision;
            entry.digest = m_digest;
            m_lastResult = m_core->ScheduleBlocks( m_path, entry, firstOfRun, countOfRun );
        }
        else
        {
            m_lastResult = CVfsCallResult( CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE, "\"" + m_path + "\" is not held locally and this thread does not wait for it" );
        }
        return false;
    }

    // a thread that waits: the source is made when it is needed, the run is fetched here
    if ( m_source.IsNULL() )
    {
        CVfsCallResult openResult;
        m_source = m_core->OpenSource( m_path, m_revision, openResult );
        if ( m_source.IsNULL() )
        {
            if ( CORE::FILESYSTEMACCESS_REMOTE_RESOURCE_CHANGED == openResult.result )
            {
                m_failed = true;
                m_core->OnSourceChanged( m_path, m_revision );
            }
            m_lastResult = openResult.IsSuccess() ? CVfsCallResult( CORE::FILESYSTEMACCESS_FAILED, "no source for \"" + m_path + "\"" ) : openResult;
            return false;
        }
    }

    UInt32 firstOfRun = 0, countOfRun = 0, ahead = 0;
    ComputeRun( block, lastBlock, firstOfRun, countOfRun, ahead );

    CVfsBlockRunFetcher::SRun run;
    CVfsCallResult fetchResult = CVfsBlockRunFetcher::FetchRun( m_cache.GetPointerAlways(), m_cacheEntry, m_blockBytes, m_size, *m_source.GetPointerAlways(), firstOfRun, countOfRun, &counters, run );
    if ( ahead > 0 && fetchResult.IsSuccess() )
        counters.readAheadBlocks.FetchAdd( ahead );

    // the bytes of a block that the cache could not keep (or of a file without a cache) are served from a window
    if ( 0 == m_cacheEntry || run.blocksNotKept > 0 )
    {
        m_window.Clear( true );
        m_window.Append( run.data.GetConstBufferPtr(), run.data.GetDataSize() );
        m_windowStart = run.firstByte;
    }

    if ( !fetchResult.IsSuccess() )
    {
        if ( CORE::FILESYSTEMACCESS_REMOTE_RESOURCE_CHANGED == fetchResult.result )
        {
            m_failed = true;
            m_core->OnSourceChanged( m_path, m_revision );
        }
        m_lastResult = fetchResult;
        m_fetchFailedInRead = true;
        m_fetchFailure = fetchResult;
        // blocks of the run that arrived before the break may cover what the read asked for
        return !m_failed && BlockAvailable( position, block );
    }

    if ( 0 != m_cacheEntry && m_cache->IsComplete( m_cacheEntry ) )
    {
        const CVfsCallResult verified = m_core->VerifyIfComplete( m_cacheEntry, m_path, m_revision, m_digest );
        if ( !verified.IsSuccess() )
        {
            m_lastResult = verified;
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CVfsCachedIOAccess::Read( void* dest, UInt32 esize, UInt32 elements )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    if ( m_failed )
        return 0;           // the failure that ended the access stays what GetLastResult() says
    m_lastResult = CVfsCallResult();
    m_fetchFailedInRead = false;
    if ( m_closed || GUCEF_NULL == dest || 0 == esize || 0 == elements || m_position >= m_size )
        return 0;

    UInt64 want = (UInt64) esize * elements;
    if ( want > m_size - m_position )
        want = m_size - m_position;

    const UInt32 firstBlockOfRead = (UInt32) ( m_position / m_blockBytes );
    m_sequentialForRead = ( m_hasLastBlock && ( firstBlockOfRead == m_lastBlockRead || firstBlockOfRead == m_lastBlockRead + 1 ) ) ? m_sequentialBlocks : 0;

    UInt8* out = static_cast< UInt8* >( dest );
    UInt64 done = 0;
    UInt32 missesSinceProgress = 0;
    while ( done < want )
    {
        const UInt64 position = m_position + done;
        const UInt32 block = (UInt32) ( position / m_blockBytes );
        const UInt32 offsetInBlock = (UInt32) ( position - (UInt64) block * m_blockBytes );
        UInt64 chunk = want - done;
        if ( chunk > (UInt64) BlockLength( block ) - offsetInBlock )
            chunk = (UInt64) BlockLength( block ) - offsetInBlock;

        if ( CopyFromWindow( position, out + done, (UInt32) chunk ) )
        {
            done += chunk;
            missesSinceProgress = 0;
            continue;
        }
        if ( 0 != m_cacheEntry && m_cache->HasBlock( m_cacheEntry, block ) )
        {
            if ( CVfsBlockCache::READ_HIT == m_cache->ReadBlock( m_cacheEntry, block, offsetInBlock, out + done, (UInt32) chunk ) )
            {
                done += chunk;
                missesSinceProgress = 0;
                continue;
            }
            // a block that the disk gave back damaged: the cache dropped it and it is fetched again like a missing one
        }

        // a miss; one that was handled and still has no bytes is not tried again
        if ( missesSinceProgress >= 2 || !HandleMiss( position, want - done ) )
        {
            if ( m_lastResult.IsSuccess() )
                m_lastResult = CVfsCallResult( CORE::FILESYSTEMACCESS_FAILED, "\"" + m_path + "\": the bytes could not be read" );
            break;
        }
        ++missesSinceProgress;
    }

    // read ahead counts the blocks of the reads before this one: it starts when the reader goes through the file in order
    if ( done > 0 )
        NoteBlocksRead( (UInt32) ( m_position / m_blockBytes ), (UInt32) ( ( m_position + done - 1 ) / m_blockBytes ) );
    m_position += done;
    return (UInt32) ( done / esize );
}

/*-------------------------------------------------------------------------*/

UInt64
CVfsCachedIOAccess::Tell( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_position;
}

/*-------------------------------------------------------------------------*/

Int32
CVfsCachedIOAccess::Seek( Int64 offset, Int32 origin )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    Int64 base = 0;
    switch ( origin )
    {
        case SEEK_SET: { base = 0; break; }
        case SEEK_CUR: { base = (Int64) m_position; break; }
        case SEEK_END: { base = (Int64) m_size; break; }
        default: return -1;
    }
    const Int64 target = base + offset;
    if ( target < 0 )
        return -1;
    m_position = (UInt64) target;
    return 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CVfsCachedIOAccess::Setpos( UInt64 position )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_position = position;
    return 0;
}

/*-------------------------------------------------------------------------*/

char
CVfsCachedIOAccess::GetChar( void )
{GUCEF_TRACE;

    char value = 0;
    if ( 1 != Read( &value, 1, 1 ) )
        return 0;
    return value;
}

/*-------------------------------------------------------------------------*/

bool
CVfsCachedIOAccess::Eof( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_position >= m_size;
}

/*-------------------------------------------------------------------------*/

bool
CVfsCachedIOAccess::IsReadable( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVfsCachedIOAccess::IsWriteable( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CVfsCachedIOAccess::IsValid( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return !m_failed && !m_closed && !m_core->IsShutdown();
}

/*-------------------------------------------------------------------------*/

UInt64
CVfsCachedIOAccess::GetSize( void ) const
{GUCEF_TRACE;

    return m_size;
}

/*-------------------------------------------------------------------------*/

void
CVfsCachedIOAccess::Flush( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CVfsCachedIOAccess::Write( const void* /* srcdata */, UInt32 /* esize */, UInt32 /* elements */ )
{GUCEF_TRACE;

    return 0;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CVfsCachedIOAccess::Clone( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return GUCEF_NEW CVfsCachedIOAccess( *this );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
