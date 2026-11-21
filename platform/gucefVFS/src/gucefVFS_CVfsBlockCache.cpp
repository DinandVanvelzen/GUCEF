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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_VFS_CVFSDIGESTHASHER_H
#include "gucefVFS_CVfsDigestHasher.h"
#define GUCEF_VFS_CVFSDIGESTHASHER_H
#endif /* GUCEF_VFS_CVFSDIGESTHASHER_H ? */

#include "gucefVFS_CVfsBlockCache.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

// "VBC1" as the first four bytes of the index
static const UInt32 IndexMagic = 0x31434256;
static const UInt32 IndexVersion = 1;
static const UInt32 HeaderBytes = 2048;
static const UInt32 TableEntryBytes = 8;
static const UInt32 OffsetBlockBytes = 8;
static const UInt32 OffsetFlags = 12;
static const UInt32 OffsetFileSize = 16;
static const UInt32 OffsetBlockCount = 24;
static const UInt32 OffsetBlocksHeld = 28;
static const UInt32 OffsetDigestAlgorithm = 32;
static const UInt32 OffsetDigestLength = 36;
static const UInt32 OffsetDigest = 40;
static const UInt32 DigestSlotBytes = 64;
static const UInt32 OffsetIdentityLength = 104;
static const UInt32 OffsetRevisionLength = 108;
static const UInt32 OffsetPathLength = 112;
static const UInt32 OffsetHeaderCrc = 116;         // the room that is left in the first 128 bytes: a CRC-32 of the header (this field as zero)
static const UInt32 OffsetCreatedMs = 120;         // and the time the entry was made
static const UInt32 OffsetIdentity = 128;
static const UInt32 OffsetRevision = 384;
static const UInt32 OffsetPath = 640;
static const UInt32 OffsetBytesHeld = 1664;       // after the path slot, in the room that is left: the bytes of the held blocks, so that a start does not have to read the tables for the used bytes
static const UInt32 FlagPinned = 1;
static const UInt32 FlagSuperseded = 2;
static const UInt32 FlagDigestVerified = 4;
static const UInt32 FlagsKnown = 7;
static const UInt32 RowsPerFlush = 16;
static const UInt32 OpenWaitStepsMax = 5000;       // one millisecond each: how long an open waits for a data file slot that is in use

/*-------------------------------------------------------------------------*/

struct SCrcTable
{
    UInt32 table[ 256 ];
    SCrcTable( void )
    {
        for ( UInt32 n=0; n<256; ++n )
        {
            UInt32 c = n;
            for ( UInt32 k=0; k<8; ++k )
                c = ( c & 1 ) ? ( 0xEDB88320U ^ ( c >> 1 ) ) : ( c >> 1 );
            table[ n ] = c;
        }
    }
};
static const SCrcTable s_crc;

/*-------------------------------------------------------------------------*/

UInt64
NowMs( void )
{
    const UInt64 resolution = MT::PrecisionTimerResolution();
    return 1 + ( resolution > 0 ? ( MT::PrecisionTickCount() * 1000ULL ) / resolution : 0 );
}

/*-------------------------------------------------------------------------*/

UInt64
NowUtcMs( void )
{
    return CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
}

/*-------------------------------------------------------------------------*/

UInt32
CurrentProcessId( void )
{
    return (UInt32) MT::GetProcessID();
}

/*-------------------------------------------------------------------------*/

void
PutU32( UInt8* buffer, UInt32 offset, UInt32 value )
{
    memcpy( buffer + offset, &value, sizeof( value ) );
}

void
PutU64( UInt8* buffer, UInt32 offset, UInt64 value )
{
    memcpy( buffer + offset, &value, sizeof( value ) );
}

UInt32
GetU32( const UInt8* buffer, UInt32 offset )
{
    UInt32 value = 0;
    memcpy( &value, buffer + offset, sizeof( value ) );
    return value;
}

UInt64
GetU64( const UInt8* buffer, UInt32 offset )
{
    UInt64 value = 0;
    memcpy( &value, buffer + offset, sizeof( value ) );
    return value;
}

/*-------------------------------------------------------------------------*/

/** The size of block 'index' of a file: a full block, less for the last one */
UInt32
BlockSizeOf( UInt64 fileSize, UInt32 blockBytes, UInt32 index )
{
    const UInt64 start = (UInt64) index * blockBytes;
    if ( start >= fileSize )
        return 0;
    const UInt64 left = fileSize - start;
    return left < blockBytes ? (UInt32) left : blockBytes;
}

/*-------------------------------------------------------------------------*/

UInt32
BlockCountOf( UInt64 fileSize, UInt32 blockBytes )
{
    return (UInt32) ( ( fileSize + blockBytes - 1 ) / blockBytes );
}

/*-------------------------------------------------------------------------*/

void
SubClamped( MT::CTAtomic< UInt64 >& value, UInt64 amount )
{
    UInt64 current = value.Load();
    for ( ;; )
    {
        const UInt64 wanted = current >= amount ? current - amount : 0;
        if ( value.CompareExchangeWeak( current, wanted ) )
            return;
    }
}

/*-------------------------------------------------------------------------*/

bool
IsLowerHex( const CORE::CString& text, UInt32 length )
{
    if ( text.Length() != length )
        return false;
    const char* chars = text.C_String();
    for ( UInt32 i=0; i<length; ++i )
    {
        const char c = chars[ i ];
        if ( !( ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'f' ) ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** The number of bytes of a text: Length() counts code points and ByteSize() counts the terminator */
UInt32
ByteCount( const CORE::CString& text )
{
    const UInt32 byteSize = text.ByteSize();
    return byteSize > 0 ? byteSize - 1 : 0;
}

/*-------------------------------------------------------------------------*/

bool
HasLineFeed( const CORE::CString& text )
{
    return ByteCount( text ) > 0 && NULL != memchr( text.C_String(), '\n', ByteCount( text ) );
}

} // anonymous namespace

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

struct CVfsBlockCache::SEntry
{
    CORE::CString key;
    CORE::CString identity;
    CORE::CString revision;
    CORE::CString path;
    CORE::CString pathKey;              /**< identity, a line feed and the path */
    CVfsDigest digest;
    UInt64 fileSize;
    UInt32 blockCount;
    UInt32 blocksHeld;
    UInt64 bytesHeld;
    UInt64 createdMs;
    bool pinned;
    bool superseded;
    bool digestVerified;
    bool tableLoaded;
    bool headerDirty;
    bool storeWarned;
    bool everOpened;
    bool dead;
    UInt32 unflushed;
    GUCEF::vector< UInt8 > present;
    GUCEF::vector< UInt32 > crc;
    GUCEF::vector< UInt8 > verified;    /**< the block was checked against its CRC in this run (or stored in this run) */
    GUCEF::vector< UInt32 > pending;    /**< the rows that are not in the index file yet */
    UInt32 refs;
    UInt64 useSeq;
    UInt64 lastUseMs;
    UInt64 lruSeq;
    int lruGroup;                       /**< 0: not in an eviction list, 1: the normal list, 2: the superseded list */
    TEntryHandle handle;
    CORE::CFileAccess* data;
    bool inRing;
    UInt32 ringIndex;
    MT::CTAtomic< UInt32 > referenced;
    MT::CMutex lock;

    SEntry( void )
        : key(), identity(), revision(), path(), pathKey(), digest(), fileSize( 0 ), blockCount( 0 ), blocksHeld( 0 ), bytesHeld( 0 ), createdMs( 0 )
        , pinned( false ), superseded( false ), digestVerified( false ), tableLoaded( false ), headerDirty( false ), storeWarned( false ), everOpened( false )
        , dead( false ), unflushed( 0 ), present(), crc(), verified(), pending(), refs( 0 ), useSeq( 0 ), lastUseMs( 0 ), lruSeq( 0 ), lruGroup( 0 )
        , handle( 0 ), data( GUCEF_NULL ), inRing( false ), ringIndex( 0 ), referenced( 0 ), lock()
    {
    }
};

/*-------------------------------------------------------------------------*/

/** What the first 2 KiB of an index say */
struct SIndexHeaderFields
{
    UInt32 flags;
    UInt64 fileSize;
    UInt32 blockCount;
    UInt32 blocksHeld;
    CVfsDigest digest;
    CORE::CString identity;
    CORE::CString revision;
    CORE::CString path;
    UInt64 createdMs;
    UInt64 bytesHeld;

    SIndexHeaderFields( void ) : flags( 0 ), fileSize( 0 ), blockCount( 0 ), blocksHeld( 0 ), digest(), identity(), revision(), path(), createdMs( 0 ), bytesHeld( 0 ) {}
};

/*-------------------------------------------------------------------------*/

static bool
ParseIndexHeader( const UInt8* header, UInt32 blockBytes, SIndexHeaderFields& fields )
{
    if ( IndexMagic != GetU32( header, 0 ) || IndexVersion != GetU32( header, 4 ) || blockBytes != GetU32( header, OffsetBlockBytes ) )
        return false;

    // the checksum covers the whole header with its own field as zero
    UInt8 copy[ HeaderBytes ];
    memcpy( copy, header, HeaderBytes );
    const UInt32 storedCrc = GetU32( copy, OffsetHeaderCrc );
    PutU32( copy, OffsetHeaderCrc, 0 );
    if ( CVfsBlockCache::Checksum( copy, HeaderBytes ) != storedCrc )
        return false;

    fields.flags = GetU32( header, OffsetFlags );
    fields.fileSize = GetU64( header, OffsetFileSize );
    fields.blockCount = GetU32( header, OffsetBlockCount );
    fields.blocksHeld = GetU32( header, OffsetBlocksHeld );
    fields.createdMs = GetU64( header, OffsetCreatedMs );
    fields.bytesHeld = GetU64( header, OffsetBytesHeld );
    const UInt32 identityLength = GetU32( header, OffsetIdentityLength );
    const UInt32 revisionLength = GetU32( header, OffsetRevisionLength );
    const UInt32 pathLength = GetU32( header, OffsetPathLength );
    const UInt32 digestAlgorithm = GetU32( header, OffsetDigestAlgorithm );
    const UInt32 digestLength = GetU32( header, OffsetDigestLength );
    if ( 0 != ( fields.flags & ~FlagsKnown ) || 0 == fields.fileSize || 0 == pathLength || identityLength > CVfsBlockCache::MaxIdentityBytes ||
         revisionLength > CVfsBlockCache::MaxRevisionBytes || pathLength > CVfsBlockCache::MaxPathBytes || digestLength > DigestSlotBytes || digestAlgorithm > CVfsDigest::ALGORITHM_SHA256 )
        return false;
    if ( fields.blockCount != BlockCountOf( fields.fileSize, blockBytes ) || fields.blocksHeld > fields.blockCount )
        return false;
    if ( fields.bytesHeld > fields.fileSize || ( fields.bytesHeld > 0 ) != ( fields.blocksHeld > 0 ) || fields.bytesHeld < fields.blocksHeld )
        return false;

    fields.identity = CORE::CString( reinterpret_cast< const char* >( header ) + OffsetIdentity, identityLength );
    fields.revision = CORE::CString( reinterpret_cast< const char* >( header ) + OffsetRevision, revisionLength );
    fields.path = CORE::CString( reinterpret_cast< const char* >( header ) + OffsetPath, pathLength );
    fields.digest = CVfsDigest( (CVfsDigest::EAlgorithm) digestAlgorithm, CORE::CString( reinterpret_cast< const char* >( header ) + OffsetDigest, digestLength ) );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVfsBlockCache::SEntryInfo::SEntryInfo( void )
    : identity(), path(), revision(), fileSize( 0 ), digest(), blockCount( 0 ), blocksHeld( 0 ), bytesHeld( 0 ), pinned( false ), complete( false )
    , superseded( false ), staleRevision( false ), digestVerified( false ), lastUseMs( 0 ), createdMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsBlockCache::SStatistics::SStatistics( void )
    : maxBytes( 0 ), usedBytes( 0 ), pinnedBytes( 0 ), entries( 0 ), pinnedEntries( 0 ), blockHits( 0 ), blockMisses( 0 ), blocksStored( 0 )
    , corruptBlocks( 0 ), evictedEntries( 0 ), evictedBytes( 0 ), supersededEvictions( 0 ), pinsRefused( 0 ), pinnedReservedBytes( 0 ), storeFailures( 0 ), openFiles( 0 ), fileReopens( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsBlockCache::CVfsBlockCache( void )
    : m_lock()
    , m_handleLock()
    , m_openLock()
    , m_folder()
    , m_open( false )
    , m_lockHeld( false )
    , m_maxBytes( 0 )
    , m_blockBytes( DefaultBlockBytes )
    , m_maxOpenFiles( DefaultMaxOpenFiles )
    , m_entries()
    , m_handles()
    , m_byPath()
    , m_lruNormal()
    , m_lruSuperseded()
    , m_nextHandle( 1 )
    , m_useCounter( 0 )
    , m_lastCreatedMs( 0 )
    , m_openRing()
    , m_clockHand( 0 )
    , m_usedBytes( 0 )
    , m_blockHits( 0 )
    , m_blockMisses( 0 )
    , m_blocksStored( 0 )
    , m_corruptBlocks( 0 )
    , m_evictedEntries( 0 )
    , m_evictedBytes( 0 )
    , m_supersededEvictions( 0 )
    , m_storeFailures( 0 )
    , m_fileReopens( 0 )
    , m_pinsRefused( 0 )
{GUCEF_TRACE;

    memset( m_fanOutKnown, 0, sizeof( m_fanOutKnown ) );
}

/*-------------------------------------------------------------------------*/

CVfsBlockCache::~CVfsBlockCache()
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

UInt32
CVfsBlockCache::Checksum( const void* data, UInt32 length, UInt32 seed )
{
    UInt32 c = seed ^ 0xFFFFFFFFU;
    const UInt8* bytes = static_cast< const UInt8* >( data );
    for ( UInt32 i=0; i<length; ++i )
        c = s_crc.table[ ( c ^ bytes[ i ] ) & 0xFF ] ^ ( c >> 8 );
    return c ^ 0xFFFFFFFFU;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CVfsBlockCache::MakeKey( const CORE::CString& identity, const CORE::CString& path, const CORE::CString& revision )
{GUCEF_TRACE;

    CORE::CSha256 sha;
    sha.Update( identity.C_String(), ByteCount( identity ) );
    sha.Update( "\n", 1 );
    sha.Update( path.C_String(), ByteCount( path ) );
    sha.Update( "\n", 1 );
    sha.Update( revision.C_String(), ByteCount( revision ) );
    UInt8 digest[ CORE::CSha256::DigestSize ];
    sha.Final( digest );
    return CORE::Base16Encode( digest, 16 ).Lowercase();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CVfsBlockCache::IndexPath( const CORE::CString& key ) const
{GUCEF_TRACE;

    return CORE::CombinePath( CORE::CombinePath( m_folder, key.SubstrFromRange( 0, 2 ) ), key + ".vbi" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CVfsBlockCache::DataPath( const CORE::CString& key ) const
{GUCEF_TRACE;

    return CORE::CombinePath( CORE::CombinePath( m_folder, key.SubstrFromRange( 0, 2 ) ), key + ".vbd" );
}

/*-------------------------------------------------------------------------*/

UInt64
CVfsBlockCache::NextUseSeq( void )
{GUCEF_TRACE;

    return m_useCounter.FetchAdd( 1 ) + 1;
}

/*-------------------------------------------------------------------------*/

UInt64
CVfsBlockCache::NextCreatedMs( void )
{GUCEF_TRACE;

    // the order of the revisions of a path: strictly increasing within a cache folder
    UInt64 now = NowUtcMs();
    if ( now <= m_lastCreatedMs )
        now = m_lastCreatedMs + 1;
    m_lastCreatedMs = now;
    return now;
}

/*-------------------------------------------------------------------------*/

void
CVfsBlockCache::RemoveEntryFiles( const CORE::CString& key ) const
{GUCEF_TRACE;

    // the index first: an entry without an index is gone, a data file without an index is cleaned up by the next Open()
    CORE::DeleteFile( IndexPath( key ) );
    CORE::DeleteFile( DataPath( key ) );
}

/*-------------------------------------------------------------------------*/

bool
CVfsBlockCache::TakeFolderLock( bool breakStaleLock )
{GUCEF_TRACE;

    const CORE::CString lockPath = CORE::CombinePath( m_folder, "cache.lock" );
    CORE::CString text = "pid=" + CORE::UInt32ToString( CurrentProcessId() ) + "\nstartedUtcMs=" + CORE::UInt64ToString( NowUtcMs() ) + "\n";

    for ( UInt32 attempt = 0; attempt < 2; ++attempt )
    {
        CORE::CString problem;
        if ( CORE::CreateOwnerOnlyFile( lockPath, text.C_String(), text.Length(), problem ) )
        {
            m_lockHeld = true;
            return true;
        }

        if ( !CORE::FileExists( lockPath ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CVfsBlockCache: The lock file " << lockPath << " cannot be made: " << problem );
            return false;
        }

        if ( breakStaleLock && 0 == attempt )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "CVfsBlockCache: Breaking the lock " << lockPath << " as asked: the process that held it is taken to be gone" );
            if ( !CORE::DeleteFile( lockPath ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CVfsBlockCache: The lock file " << lockPath << " cannot be removed" );
                return false;
            }
            continue;
        }

        // refuse, and say who has it
        CORE::CString owner = "an unreadable lock file";
        {
            CORE::CFileAccess lockFile( lockPath, "rb" );
            if ( lockFile.Opened() )
            {
                char buffer[ 256 ];
                const UInt32 got = lockFile.Read( buffer, 1, sizeof( buffer ) - 1 );
                buffer[ got ] = '\0';
                owner = CORE::CString( buffer ).ReplaceChar( '\n', ' ' );
                const char* pidText = strstr( buffer, "pid=" );
                if ( GUCEF_NULL != pidText )
                {
                    bool alive = false;
                    const CORE::TProcessId pid = (CORE::TProcessId) strtoul( pidText + 4, GUCEF_NULL, 10 );
                    if ( CORE::CheckOnProcessAliveStatus( pid, alive ) )
                        owner += alive ? " (that process is running)" : " (that process is not running: a crash left the lock behind, open with breakStaleLock to remove it)";
                }
            }
        }
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CVfsBlockCache: The cache folder " << m_folder << " is in use by another cache: " << owner );
        return false;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CVfsBlockCache::ReleaseFolderLock( void )
{GUCEF_TRACE;

    if ( m_lockHeld )
    {
        CORE::DeleteFile( CORE::CombinePath( m_folder, "cache.lock" ) );
        m_lockHeld = false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CVfsBlockCache::Open( const CORE::CString& folder, UInt64 maxBytes, UInt32 blockBytes, UInt32 maxOpenFiles, bool breakStaleLock )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_open )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CVfsBlockCache: Open() on a cache that is open already (folder " << m_folder << ")" );
        return false;
    }
    if ( folder.IsNULLOrEmpty() )
        return false;
    if ( blockBytes < MinBlockBytes || blockBytes > MaxBlockBytes || 0 != ( blockBytes & ( blockBytes - 1 ) ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CVfsBlockCache: The block size " << blockBytes << " is not a power of two between " << (UInt32) MinBlockBytes << " and " << (UInt32) MaxBlockBytes );
        return false;
    }
    if ( 0 == maxOpenFiles )
        maxOpenFiles = DefaultMaxOpenFiles;

    const CORE::CString actualFolder = CORE::RelativePath( folder );
    if ( !CORE::DirExists( actualFolder ) && !CORE::CreateDirs( actualFolder ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CVfsBlockCache: The cache folder " << actualFolder << " cannot be made" );
        return false;
    }
    m_folder = actualFolder;
    if ( !TakeFolderLock( breakStaleLock ) )
    {
        m_folder.Clear();
        return false;
    }

    m_maxBytes = maxBytes;
    m_blockBytes = blockBytes;
    m_maxOpenFiles = maxOpenFiles;
    m_usedBytes.Store( 0 );
    m_open = true;
    ScanFolder();
    EvictLocked();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CVfsBlockCache: Cache folder " << m_folder << " holds " << (UInt32) m_entries.size() << " entries, " << m_usedBytes.Load() << " bytes" );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CVfsBlockCache::Close( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_open )
        return;

    UInt32 stillAcquired = 0;
    TEntryMap::iterator i = m_entries.begin();
    while ( i != m_entries.end() )
    {
        SEntry* entry = i->second;
        {
            MT::CScopeMutex entryLock( entry->lock );
            if ( entry->refs > 0 )
                ++stillAcquired;
            if ( ( entry->tableLoaded || entry->headerDirty ) && !entry->dead )
                PersistLocked( *entry );
            entry->dead = true;
            if ( GUCEF_NULL != entry->data )
            {
                delete entry->data;
                entry->data = GUCEF_NULL;
            }
        }
        GUCEF_DELETE entry;
        ++i;
    }
    if ( stillAcquired > 0 )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CVfsBlockCache: Closed with " << stillAcquired << " entries still acquired" );

    m_entries.clear();
    {
        MT::CScopeMutex handleLock( m_handleLock );
        m_handles.clear();
    }
    m_byPath.clear();
    m_lruNormal.clear();
    m_lruSuperseded.clear();
    {
        MT::CScopeMutex openLock( m_openLock );
        m_openRing.clear();
        m_clockHand = 0;
    }
    m_usedBytes.Store( 0 );
    memset( m_fanOutKnown, 0, sizeof( m_fanOutKnown ) );
    ReleaseFolderLock();
    m_open = false;
}

/*-------------------------------------------------------------------------*/

bool
CVfsBlockCache::IsOpen( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_open;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CVfsBlockCache::GetFolder( void ) const
{GUCEF_TRACE;

    return m_folder;
}

/*-------------------------------------------------------------------------*/

void
CVfsBlockCache::SetMaxBytes( UInt64 maxBytes )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_maxBytes = maxBytes;
    EvictLocked();
}

/*-------------------------------------------------------------------------*/

UInt64
CVfsBlockCache::GetMaxBytes( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_maxBytes;
}

/*-------------------------------------------------------------------------*/

UInt64
CVfsBlockCache::GetUsedBytes( void ) const
{GUCEF_TRACE;

    return m_usedBytes.Load();
}

/*-------------------------------------------------------------------------*/

UInt32
CVfsBlockCache::GetBlockBytes( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_blockBytes;
}

/*-------------------------------------------------------------------------*/

UInt32
CVfsBlockCache::GetMaxOpenFiles( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_maxOpenFiles;
}

/*-------------------------------------------------------------------------*/

void
CVfsBlockCache::ScanFolder( void )
{GUCEF_TRACE;

    // the folders of the layout: two hexadecimal characters
    CORE::CString::StringVector fanOutFolders;
    {
        CORE::CFileSystemIterator iterator;
        if ( iterator.FindFirst( m_folder ) )
        {
            do
            {
                if ( !iterator.IsADirectory() || iterator.IsSymlink() )
                    continue;
                const CORE::CString name = iterator.GetResourceName();
                if ( IsLowerHex( name, 2 ) )
                    fanOutFolders.push_back( name );
            }
            while ( iterator.FindNext() );
            iterator.FindClose();
        }
    }

    // the entries of the folders, oldest index first: the order of use after a start. Data files that have no index are removed
    typedef GUCEF::map< Int64, CORE::CString::StringVector > TByTime;
    TByTime byTime;
    UInt32 orphans = 0;
    for ( size_t f=0; f<fanOutFolders.size(); ++f )
    {
        const CORE::CString& fan = fanOutFolders[ f ];
        UInt32 fanByte = 0;
        for ( UInt32 c=0; c<2; ++c )
            fanByte = ( fanByte << 4 ) | (UInt32) ( fan[ c ] <= '9' ? fan[ c ] - '0' : fan[ c ] - 'a' + 10 );
        m_fanOutKnown[ fanByte ] = true;

        CORE::CString::StringSet indexKeys;
        CORE::CString::StringSet dataKeys;
        CORE::CFileSystemIterator iterator;
        if ( !iterator.FindFirst( CORE::CombinePath( m_folder, fan ) ) )
            continue;
        do
        {
            if ( !iterator.IsAFile() )
                continue;
            const CORE::CString name = iterator.GetResourceName();
            if ( name.Length() != 36 )
                continue;
            const CORE::CString key = name.SubstrFromRange( 0, 32 );
            const CORE::CString extension = name.SubstrFromRange( 32, 36 );
            if ( !IsLowerHex( key, 32 ) || key.SubstrFromRange( 0, 2 ) != fan )
                continue;
            if ( extension == ".vbi" )
                indexKeys.insert( key );
            else
            if ( extension == ".vbd" )
                dataKeys.insert( key );
        }
        while ( iterator.FindNext() );
        iterator.FindClose();

        for ( CORE::CString::StringSet::const_iterator d = dataKeys.begin(); d != dataKeys.end(); ++d )
        {
            if ( indexKeys.find( *d ) == indexKeys.end() )
            {
                CORE::DeleteFile( DataPath( *d ) );
                ++orphans;
            }
        }
        for ( CORE::CString::StringSet::const_iterator k = indexKeys.begin(); k != indexKeys.end(); ++k )
        {
            const CORE::CDateTime modified = CORE::GetFileModificationTime( IndexPath( *k ) );
            byTime[ (Int64) modified.ToUnixEpochBasedTicksInMillisecs() ].push_back( *k );
        }
    }

    UInt64 rank = 0;
    UInt32 dropped = 0;
    for ( TByTime::const_iterator t = byTime.begin(); t != byTime.end(); ++t )
    {
        for ( size_t k=0; k<t->second.size(); ++k )
        {
            if ( !LoadStubLocked( t->second[ k ], rank + 1 ) )
                ++dropped;
            else
                ++rank;
        }
    }
    if ( rank > m_useCounter.Load() )
        m_useCounter.Store( rank );
    if ( dropped > 0 || orphans > 0 )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CVfsBlockCache: Removed " << dropped << " damaged entries and " << orphans << " data files without an index from " << m_folder );
}

/*-------------------------------------------------------------------------*/

bool
CVfsBlockCache::LoadStubLocked( const CORE::CString& key, UInt64 useRank )
{GUCEF_TRACE;

    SIndexHeaderFields fields;
    bool good = false;
    {
        CORE::CFileAccess file( IndexPath( key ), "rb" );
        if ( file.Opened() )
        {
            UInt8 header[ HeaderBytes ];
            if ( HeaderBytes == file.Read( header, 1, HeaderBytes ) && ParseIndexHeader( header, m_blockBytes, fields ) )
            {
                // a cut short or a longer index is not what its header says
                good = file.GetSize() == (UInt64) HeaderBytes + (UInt64) fields.blockCount * TableEntryBytes;
            }
        }
    }
    if ( good )
        good = MakeKey( fields.identity, fields.path, fields.revision ) == key && CORE::FileExists( DataPath( key ) );
    if ( good && m_entries.find( key ) != m_entries.end() )
        return true;
    if ( !good )
    {
        RemoveEntryFiles( key );
        return false;
    }

    SEntry* entry = GUCEF_NEW SEntry();
    entry->key = key;
    entry->identity = fields.identity;
    entry->revision = fields.revision;
    entry->path = fields.path;
    entry->pathKey = fields.identity + "\n" + fields.path;
    entry->digest = fields.digest;
    entry->fileSize = fields.fileSize;
    entry->blockCount = fields.blockCount;
    entry->blocksHeld = fields.blocksHeld;
    entry->bytesHeld = fields.bytesHeld;
    entry->createdMs = fields.createdMs;
    entry->pinned = 0 != ( fields.flags & FlagPinned );
    entry->superseded = 0 != ( fields.flags & FlagSuperseded );
    entry->digestVerified = 0 != ( fields.flags & FlagDigestVerified );
    entry->useSeq = useRank;
    if ( fields.createdMs > m_lastCreatedMs )
        m_lastCreatedMs = fields.createdMs;

    RegisterEntryLocked( entry );
    m_usedBytes.FetchAdd( entry->bytesHeld );
    LruInsertLocked( entry );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CVfsBlockCache::RegisterEntryLocked( SEntry* entry )
{GUCEF_TRACE;

    m_entries[ entry->key ] = entry;
    entry->handle = m_nextHandle++;
    {
        MT::CScopeMutex handleLock( m_handleLock );
        m_handles[ entry->handle ] = entry;
    }
    m_byPath[ entry->pathKey ].push_back( entry );
}

/*-------------------------------------------------------------------------*/

void
CVfsBlockCache::UnregisterEntryLocked( SEntry* entry )
{GUCEF_TRACE;

    m_entries.erase( entry->key );
    {
        MT::CScopeMutex handleLock( m_handleLock );
        m_handles.erase( entry->handle );
    }
    TPathMap::iterator g = m_byPath.find( entry->pathKey );
    if ( g != m_byPath.end() )
    {
        TEntryVector& group = g->second;
        for ( size_t i=0; i<group.size(); ++i )
        {
            if ( group[ i ] == entry )
            {
                group.erase( group.begin() + i );
                break;
            }
        }
        if ( group.empty() )
            m_byPath.erase( g );
    }
}

/*-------------------------------------------------------------------------*/

CVfsBlockCache::SEntry*
CVfsBlockCache::Resolve( TEntryHandle handle ) const
{GUCEF_TRACE;

    MT::CScopeMutex handleLock( m_handleLock );
    THandleMap::const_iterator i = m_handles.find( handle );
    return i == m_handles.end() ? GUCEF_NULL : i->second;
}

/*-------------------------------------------------------------------------*/

CVfsBlockCache::SEntry*
CVfsBlockCache::FindEntryLocked( const CORE::CString& key, const CORE::CString& identity, const CORE::CString& path, const CORE::CString& revision ) const
{GUCEF_TRACE;

    TEntryMap::const_iterator i = m_entries.find( key );
    if ( i == m_entries.end() )
        return GUCEF_NULL;
    SEntry* entry = i->second;
    // the key is a hash: the entry has to be the one asked for
    if ( entry->identity != identity || entry->path != path || entry->revision != revision )
        return GUCEF_NULL;
    return entry;
}

/*-------------------------------------------------------------------------*/

CVfsBlockCache::SEntry*
CVfsBlockCache::NewestOfPathLocked( const CORE::CString& pathKey, bool includeSuperseded ) const
{GUCEF_TRACE;

    TPathMap::const_iterator g = m_byPath.find( pathKey );
    if ( g == m_byPath.end() )
        return GUCEF_NULL;
    SEntry* newest = GUCEF_NULL;
    const TEntryVector& group = g->second;
    for ( size_t i=0; i<group.size(); ++i )
    {
        SEntry* candidate = group[ i ];
        if ( ( candidate->superseded && !includeSuperseded ) || candidate->dead )
            continue;
        if ( GUCEF_NULL == newest || candidate->createdMs > newest->createdMs )
            newest = candidate;
    }
    return newest;
}

/*-------------------------------------------------------------------------*/

CVfsBlockCache::SEntry*
CVfsBlockCache::SelectOfflineLocked( const CORE::CString& pathKey, bool& stale ) const
{GUCEF_TRACE;

    stale = false;
    SEntry* newest = NewestOfPathLocked( pathKey, false );
    if ( GUCEF_NULL != newest )
    {
        MT::CScopeMutex newestLock( newest->lock );
        if ( newest->blocksHeld == newest->blockCount )
            return newest;
    }

    // the newest revision is not held completely: a pinned older revision that is, stands in for it
    TPathMap::const_iterator g = m_byPath.find( pathKey );
    if ( g != m_byPath.end() )
    {
        SEntry* older = GUCEF_NULL;
        const TEntryVector& group = g->second;
        for ( size_t i=0; i<group.size(); ++i )
        {
            SEntry* candidate = group[ i ];
            if ( candidate == newest || candidate->dead || !candidate->pinned )
                continue;
            MT::CScopeMutex candidateLock( candidate->lock );
            if ( candidate->blockCount > 0 && candidate->blocksHeld == candidate->blockCount && ( GUCEF_NULL == older || candidate->createdMs > older->createdMs ) )
                older = candidate;
        }
        if ( GUCEF_NULL != older )
        {
            stale = true;
            return older;
        }
    }
    return newest;
}

/*-------------------------------------------------------------------------*/

UInt64
CVfsBlockCache::PinnedReservedBytesLocked( void ) const
{GUCEF_TRACE;

    // per path the largest pinned revision: a pin that follows its path to a newer revision does not count twice
    UInt64 reserved = 0;
    for ( TPathMap::const_iterator g = m_byPath.begin(); g != m_byPath.end(); ++g )
    {
        UInt64 pathBytes = 0;
        const TEntryVector& group = g->second;
        for ( size_t i=0; i<group.size(); ++i )
        {
            if ( group[ i ]->pinned && !group[ i ]->dead && group[ i ]->fileSize > pathBytes )
                pathBytes = group[ i ]->fileSize;
        }
        reserved += pathBytes;
    }
    return reserved;
}

/*-------------------------------------------------------------------------*/

void
CVfsBlockCache::LruRemoveLocked( SEntry* entry )
{GUCEF_TRACE;

    if ( 1 == entry->lruGroup )
        m_lruNormal.erase( entry->lruSeq );
    else
    if ( 2 == entry->lruGroup )
        m_lruSuperseded.erase( entry->lruSeq );
    entry->lruGroup = 0;
}

/*-------------------------------------------------------------------------*/

void
CVfsBlockCache::LruInsertLocked( SEntry* entry )
{GUCEF_TRACE;

    LruRemoveLocked( entry );

    // what can be removed to make room: not pinned and not acquired
    if ( entry->refs > 0 || entry->pinned || entry->dead )
        return;

    entry->lruSeq = entry->useSeq;
    if ( entry->superseded )
    {
        m_lruSuperseded[ entry->lruSeq ] = entry;
        entry->lruGroup = 2;
    }
    else
    {
        m_lruNormal[ entry->lruSeq ] = entry;
        entry->lruGroup = 1;
    }
}

/*-------------------------------------------------------------------------*/

void
CVfsBlockCache::EvictLocked( void )
{GUCEF_TRACE;

    if ( 0 == m_maxBytes )
        return;

    while ( m_usedBytes.Load() > m_maxBytes )
    {
        // superseded entries first, each list oldest use first
        SEntry* victim = GUCEF_NULL;
        bool wasSuperseded = false;
        if ( !m_lruSuperseded.empty() )
        {
            victim = m_lruSuperseded.begin()->second;
            wasSuperseded = true;
        }
        else
        if ( !m_lruNormal.empty() )
        {
            victim = m_lruNormal.begin()->second;
        }
        if ( GUCEF_NULL == victim )
            return;

        const UInt64 bytes = victim->bytesHeld;
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CVfsBlockCache: Removing " << victim->path << " (" << bytes << " bytes) to stay under the limit" );
        DeleteEntryLocked( victim, true );
        m_evictedEntries.FetchAdd( 1 );
        m_evictedBytes.FetchAdd( bytes );
        if ( wasSuperseded )
            m_supersededEvictions.FetchAdd( 1 );
    }
}

/*-------------------------------------------------------------------------*/

void
CVfsBlockCache::DeleteEntryLocked( SEntry* entry, bool entryCounted )
{GUCEF_TRACE;

    {
        MT::CScopeMutex entryLock( entry->lock );
        entry->dead = true;
        CloseDataLocked( *entry );
    }
    RemoveEntryFiles( entry->key );
    if ( entryCounted )
        SubClamped( m_usedBytes, entry->bytesHeld );
    LruRemoveLocked( entry );
    UnregisterEntryLocked( entry );
    GUCEF_DELETE entry;
}

/*-------------------------------------------------------------------------*/

void
CVfsBlockCache::BuildHeader( const SEntry& entry, UInt8* header ) const
{GUCEF_TRACE;

    memset( header, 0, HeaderBytes );
    PutU32( header, 0, IndexMagic );
    PutU32( header, 4, IndexVersion );
    PutU32( header, OffsetBlockBytes, m_blockBytes );
    PutU32( header, OffsetFlags, ( entry.pinned ? FlagPinned : 0 ) | ( entry.superseded ? FlagSuperseded : 0 ) | ( entry.digestVerified ? FlagDigestVerified : 0 ) );
    PutU64( header, OffsetFileSize, entry.fileSize );
    PutU32( header, OffsetBlockCount, entry.blockCount );
    PutU32( header, OffsetBlocksHeld, entry.blocksHeld );
    PutU32( header, OffsetDigestAlgorithm, (UInt32) entry.digest.algorithm );
    const UInt32 digestLength = entry.digest.value.Length() > DigestSlotBytes ? DigestSlotBytes : entry.digest.value.Length();
    PutU32( header, OffsetDigestLength, digestLength );
    memcpy( header + OffsetDigest, entry.digest.value.C_String(), digestLength );
    PutU32( header, OffsetIdentityLength, ByteCount( entry.identity ) );
    PutU32( header, OffsetRevisionLength, ByteCount( entry.revision ) );
    PutU32( header, OffsetPathLength, ByteCount( entry.path ) );
    PutU64( header, OffsetCreatedMs, entry.createdMs );
    PutU64( header, OffsetBytesHeld, entry.bytesHeld );
    memcpy( header + OffsetIdentity, entry.identity.C_String(), ByteCount( entry.identity ) );
    memcpy( header + OffsetRevision, entry.revision.C_String(), ByteCount( entry.revision ) );
    memcpy( header + OffsetPath, entry.path.C_String(), ByteCount( entry.path ) );
    PutU32( header, OffsetHeaderCrc, Checksum( header, HeaderBytes ) );
}

/*-------------------------------------------------------------------------*/

bool
CVfsBlockCache::WriteNewIndex( SEntry& entry ) const
{GUCEF_TRACE;

    const CORE::CString indexPath = IndexPath( entry.key );
    {
        CORE::CFileAccess index( indexPath, "wb" );
        if ( !index.Opened() )
            return false;
        UInt8 header[ HeaderBytes ];
        BuildHeader( entry, header );
        if ( HeaderBytes != index.Write( header, 1, HeaderBytes ) )
            return false;
        index.Flush();
    }

    // the table starts as zero rows: no block held. The size is set in one step, the rows are not written one by one
    return CORE::TruncateFile( indexPath, (UInt64) HeaderBytes + (UInt64) entry.blockCount * TableEntryBytes );
}

/*-------------------------------------------------------------------------*/

CVfsBlockCache::SEntry*
CVfsBlockCache::CreateEntryLocked( const CORE::CString& identity, const CORE::CString& path, const CORE::CString& revision, UInt64 fileSize, const CVfsDigest& digest )
{GUCEF_TRACE;

    const CORE::CString key = MakeKey( identity, path, revision );
    const CORE::CString fan = key.SubstrFromRange( 0, 2 );
    UInt32 fanByte = 0;
    for ( UInt32 c=0; c<2; ++c )
        fanByte = ( fanByte << 4 ) | (UInt32) ( fan[ c ] <= '9' ? fan[ c ] - '0' : fan[ c ] - 'a' + 10 );
    if ( !m_fanOutKnown[ fanByte ] )
    {
        const CORE::CString fanFolder = CORE::CombinePath( m_folder, fan );
        if ( !CORE::DirExists( fanFolder ) && !CORE::CreateDirs( fanFolder ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CVfsBlockCache: The folder " << fanFolder << " cannot be made" );
            return GUCEF_NULL;
        }
        m_fanOutKnown[ fanByte ] = true;
    }

    SEntry* entry = GUCEF_NEW SEntry();
    entry->key = key;
    entry->identity = identity;
    entry->revision = revision;
    entry->path = path;
    entry->pathKey = identity + "\n" + path;
    entry->digest = digest;
    entry->fileSize = fileSize;
    entry->blockCount = BlockCountOf( fileSize, m_blockBytes );
    entry->createdMs = NextCreatedMs();
    entry->present.assign( entry->blockCount, 0 );
    entry->crc.assign( entry->blockCount, 0 );
    entry->verified.assign( entry->blockCount, 0 );
    entry->tableLoaded = true;
    entry->useSeq = NextUseSeq();

    {
        CORE::CFileAccess data( DataPath( key ), "wb" );
        if ( !data.Opened() || !WriteNewIndex( *entry ) )
        {
            GUCEF_DELETE entry;
            RemoveEntryFiles( key );
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CVfsBlockCache: The cache files of " << path << " cannot be made in " << m_folder );
            return GUCEF_NULL;
        }
    }

    RegisterEntryLocked( entry );
    return entry;
}

/*-------------------------------------------------------------------------*/

CVfsBlockCache::TEntryHandle
CVfsBlockCache::AcquireEntry( const CORE::CString& identity, const CORE::CString& path, const CORE::CString& revision,
                              UInt64 fileSize, const CVfsDigest& digest, bool create )
{GUCEF_TRACE;

    if ( 0 == fileSize || path.IsNULLOrEmpty() || ByteCount( identity ) > MaxIdentityBytes || ByteCount( revision ) > MaxRevisionBytes || ByteCount( path ) > MaxPathBytes )
        return 0;
    if ( HasLineFeed( identity ) || HasLineFeed( path ) || HasLineFeed( revision ) )
        return 0;

    MT::CScopeMutex lock( m_lock );
    if ( !m_open )
        return 0;
    if ( BlockCountOf( fileSize, m_blockBytes ) == 0 || fileSize / m_blockBytes >= 0xFFFFFFFFULL )
        return 0;

    const CORE::CString key = MakeKey( identity, path, revision );
    SEntry* entry = FindEntryLocked( key, identity, path, revision );
    if ( GUCEF_NULL != entry )
    {
        const bool digestConflict = !digest.IsEmpty() && !entry->digest.IsEmpty() && digest.algorithm == entry->digest.algorithm && digest.value != entry->digest.value;
        if ( entry->fileSize != fileSize || digestConflict )
        {
            // the revision did not identify the content: no old reader can be moved to other bytes, so with one around there is no entry
            if ( !create || entry->refs > 0 )
                return 0;
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CVfsBlockCache: The content of " << path << " at revision \"" << revision << "\" is not what the cache holds, the cached blocks are dropped" );
            DeleteEntryLocked( entry, true );
            entry = GUCEF_NULL;
        }
    }
    if ( GUCEF_NULL == entry )
    {
        if ( !create )
            return 0;
        entry = CreateEntryLocked( identity, path, revision, fileSize, digest );
        if ( GUCEF_NULL == entry )
            return 0;
    }
    else
    if ( entry->digest.IsEmpty() && !digest.IsEmpty() )
    {
        MT::CScopeMutex entryLock( entry->lock );
        entry->digest = digest;
        entry->digestVerified = false;
        entry->headerDirty = true;
    }

    ++entry->refs;
    LruRemoveLocked( entry );
    entry->useSeq = NextUseSeq();
    entry->lastUseMs = NowMs();
    return entry->handle;
}

/*-------------------------------------------------------------------------*/

void
CVfsBlockCache::ReleaseEntry( TEntryHandle handle )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    THandleMap::const_iterator i = m_handles.find( handle );
    if ( i == m_handles.end() || i->second->refs == 0 )
        return;
    SEntry* entry = i->second;
    if ( --entry->refs > 0 )
        return;

    {
        MT::CScopeMutex entryLock( entry->lock );
        PersistLocked( *entry );
        CloseDataLocked( *entry );
        entry->lastUseMs = NowMs();
    }
    LruInsertLocked( entry );
    EvictLocked();
}

/*-------------------------------------------------------------------------*/

void
CVfsBlockCache::CloseDataLocked( SEntry& entry )
{GUCEF_TRACE;

    if ( entry.inRing )
    {
        MT::CScopeMutex openLock( m_openLock );
        if ( entry.inRing && entry.ringIndex < m_openRing.size() && m_openRing[ entry.ringIndex ] == &entry )
        {
            SEntry* last = m_openRing.back();
            m_openRing[ entry.ringIndex ] = last;
            last->ringIndex = entry.ringIndex;
            m_openRing.pop_back();
            if ( m_clockHand > m_openRing.size() )
                m_clockHand = 0;
        }
        entry.inRing = false;
    }
    if ( GUCEF_NULL != entry.data )
    {
        delete entry.data;
        entry.data = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

bool
CVfsBlockCache::EnsureTableLocked( SEntry& entry )
{GUCEF_TRACE;

    if ( entry.tableLoaded )
        return true;
    if ( entry.dead )
        return false;

    GUCEF::vector< UInt8 > table;
    bool read = false;
    {
        CORE::CFileAccess index( IndexPath( entry.key ), "rb" );
        if ( index.Opened() && 0 == index.Setpos( HeaderBytes ) )
        {
            table.resize( (size_t) entry.blockCount * TableEntryBytes );
            read = table.empty() || (UInt32) table.size() == index.Read( &table[ 0 ], 1, (UInt32) table.size() );
        }
    }

    if ( !read )
    {
        // the index is gone or cut short since it was looked at: the entry starts again, empty
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CVfsBlockCache: The index of " << entry.path << " cannot be read, the entry is emptied" );
        return EmptyEntryLocked( entry );
    }

    entry.present.assign( entry.blockCount, 0 );
    entry.crc.assign( entry.blockCount, 0 );
    entry.verified.assign( entry.blockCount, 0 );
    UInt32 held = 0;
    UInt64 bytes = 0;
    for ( UInt32 b=0; b<entry.blockCount; ++b )
    {
        if ( 1 == GetU32( &table[ 0 ], b * TableEntryBytes + 4 ) )
        {
            entry.present[ b ] = 1;
            entry.crc[ b ] = GetU32( &table[ 0 ], b * TableEntryBytes );
            ++held;
            bytes += BlockSizeOf( entry.fileSize, m_blockBytes, b );
        }
    }

    // what the header said was advisory: the table is what counts
    if ( held != entry.blocksHeld )
        entry.headerDirty = true;
    if ( bytes >= entry.bytesHeld )
        m_usedBytes.FetchAdd( bytes - entry.bytesHeld );
    else
        SubClamped( m_usedBytes, entry.bytesHeld - bytes );
    entry.blocksHeld = held;
    entry.bytesHeld = bytes;
    entry.tableLoaded = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVfsBlockCache::EmptyEntryLocked( SEntry& entry )
{GUCEF_TRACE;

    CloseDataLocked( entry );
    SubClamped( m_usedBytes, entry.bytesHeld );
    entry.blocksHeld = 0;
    entry.bytesHeld = 0;
    entry.present.assign( entry.blockCount, 0 );
    entry.crc.assign( entry.blockCount, 0 );
    entry.verified.assign( entry.blockCount, 0 );
    entry.pending.clear();
    entry.digest = CVfsDigest();
    entry.digestVerified = false;
    entry.unflushed = 0;
    entry.headerDirty = false;
    entry.tableLoaded = true;

    CORE::CFileAccess data( DataPath( entry.key ), "wb" );
    if ( !data.Opened() || !WriteNewIndex( entry ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CVfsBlockCache: The cache files of " << entry.path << " cannot be made again in " << m_folder );
        entry.headerDirty = true;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVfsBlockCache::PersistLocked( SEntry& entry )
{GUCEF_TRACE;

    if ( entry.dead )
        return false;

    // the data goes to the disk before the rows that say it is there
    if ( GUCEF_NULL != entry.data )
        entry.data->Flush();
    if ( !entry.headerDirty && entry.pending.empty() )
    {
        entry.unflushed = 0;
        return true;
    }

    CORE::CFileAccess index( IndexPath( entry.key ), "r+b" );
    if ( !index.Opened() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CVfsBlockCache: The index of " << entry.path << " cannot be opened for writing in " << m_folder );
        return false;
    }

    UInt8 header[ HeaderBytes ];
    BuildHeader( entry, header );
    bool good = 0 == index.Setpos( 0 ) && HeaderBytes == index.Write( header, 1, HeaderBytes );
    if ( entry.tableLoaded )
    {
        for ( size_t i=0; i<entry.pending.size() && good; ++i )
        {
            const UInt32 block = entry.pending[ i ];
            if ( block >= entry.blockCount )
                continue;
            UInt8 row[ TableEntryBytes ];
            PutU32( row, 0, entry.present[ block ] ? entry.crc[ block ] : 0 );
            PutU32( row, 4, entry.present[ block ] ? 1 : 0 );
            good = 0 == index.Setpos( (UInt64) HeaderBytes + (UInt64) block * TableEntryBytes ) && TableEntryBytes == index.Write( row, 1, TableEntryBytes );
        }
    }
    index.Flush();
    if ( good )
    {
        entry.pending.clear();
        entry.headerDirty = false;
        entry.unflushed = 0;
    }
    return good;
}

/*-------------------------------------------------------------------------*/

bool
CVfsBlockCache::EnsureOpenLocked( SEntry& entry )
{GUCEF_TRACE;

    if ( GUCEF_NULL != entry.data )
    {
        entry.referenced.Store( 1 );
        return true;
    }
    if ( entry.dead )
        return false;

    // a slot among the open data files: a free one, or one of an entry that is not busy at the moment (the clock decides which)
    bool haveSlot = false;
    for ( UInt32 step = 0; step < OpenWaitStepsMax && !haveSlot; ++step )
    {
        SEntry* victim = GUCEF_NULL;
        {
            MT::CScopeMutex openLock( m_openLock );
            if ( m_openRing.size() < m_maxOpenFiles )
            {
                entry.ringIndex = (UInt32) m_openRing.size();
                m_openRing.push_back( &entry );
                entry.inRing = true;
                haveSlot = true;
            }
            else
            {
                const size_t ringSize = m_openRing.size();
                for ( size_t probe = 0; probe < 2 * ringSize && GUCEF_NULL == victim; ++probe )
                {
                    const size_t at = ( m_clockHand + probe ) % ringSize;
                    SEntry* candidate = m_openRing[ at ];
                    if ( candidate == &entry )
                        continue;
                    if ( 0 != candidate->referenced.Exchange( 0 ) )
                        continue;
                    // never wait for an entry in use: it is the one of another thread, which may wait for an open slot as well
                    const MT::TLockStatus status = candidate->lock.Lock( 0 );
                    if ( MT::LOCKSTATUS_OPERATION_SUCCESS != status && MT::LOCKSTATUS_ABANDONED != status )
                        continue;
                    m_openRing[ at ] = &entry;
                    entry.ringIndex = (UInt32) at;
                    entry.inRing = true;
                    candidate->inRing = false;
                    m_clockHand = (UInt32) ( ( at + 1 ) % ringSize );
                    victim = candidate;
                    haveSlot = true;
                }
            }
        }
        if ( GUCEF_NULL != victim )
        {
            // its file is closed to make room; the rows that are not written yet go first. It is opened again when it is touched
            PersistLocked( *victim );
            CloseDataLocked( *victim );
            victim->lock.Unlock();
        }
        else
        if ( !haveSlot )
        {
            MT::PrecisionDelay( 1 );
        }
    }
    if ( !haveSlot )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CVfsBlockCache: No data file could be opened for " << entry.path << ": all " << m_maxOpenFiles << " open files stay in use" );
        return false;
    }

    const CORE::CString dataPath = DataPath( entry.key );
    entry.data = GUCEF_NEW CORE::CFileAccess( dataPath, "r+b" );
    if ( !entry.data->Opened() && !CORE::FileExists( dataPath ) )
    {
        // the data file was removed under us: start an empty one, the blocks the index claims fail their checksum and are fetched again
        delete entry.data;
        entry.data = GUCEF_NEW CORE::CFileAccess( dataPath, "w+b" );
    }
    else
    if ( !entry.data->Opened() )
    {
        // a file that cannot be written (a read only cache or file): its blocks can still be read, a store fails and is counted
        delete entry.data;
        entry.data = GUCEF_NEW CORE::CFileAccess( dataPath, "rb" );
    }
    if ( !entry.data->Opened() )
    {
        CloseDataLocked( entry );
        return false;
    }
    if ( entry.everOpened )
        m_fileReopens.FetchAdd( 1 );
    entry.everOpened = true;
    entry.referenced.Store( 1 );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CVfsBlockCache::DropBlockLocked( SEntry& entry, UInt32 index )
{GUCEF_TRACE;

    if ( 0 == entry.present[ index ] )
        return;
    const UInt32 size = BlockSizeOf( entry.fileSize, m_blockBytes, index );
    entry.present[ index ] = 0;
    entry.verified[ index ] = 0;
    entry.crc[ index ] = 0;
    --entry.blocksHeld;
    entry.bytesHeld -= size;
    entry.digestVerified = false;
    entry.pending.push_back( index );
    entry.headerDirty = true;
    SubClamped( m_usedBytes, size );
}

/*-------------------------------------------------------------------------*/

int
CVfsBlockCache::ReadFromBlockLocked( SEntry& entry, UInt32 index, UInt32 offsetInBlock, void* dest, UInt32 length )
{GUCEF_TRACE;

    const UInt32 blockSize = BlockSizeOf( entry.fileSize, m_blockBytes, index );
    const UInt64 start = (UInt64) index * m_blockBytes;
    if ( !EnsureOpenLocked( entry ) )
        return 2;

    if ( 0 == entry.verified[ index ] )
    {
        // the first read of a block in this run: the whole block against its checksum
        bool good = false;
        if ( 0 == offsetInBlock && length == blockSize )
        {
            good = 0 == entry.data->Setpos( start ) && blockSize == entry.data->Read( dest, 1, blockSize ) && Checksum( dest, blockSize ) == entry.crc[ index ];
        }
        else
        {
            GUCEF::vector< UInt8 > block;
            block.resize( blockSize );
            good = 0 == entry.data->Setpos( start ) && blockSize == entry.data->Read( &block[ 0 ], 1, blockSize ) && Checksum( &block[ 0 ], blockSize ) == entry.crc[ index ];
            if ( good )
                memcpy( dest, &block[ 0 ] + offsetInBlock, length );
        }
        if ( good )
        {
            entry.verified[ index ] = 1;
            return 0;
        }
        return 1;
    }

    return ( 0 == entry.data->Setpos( start + offsetInBlock ) && length == entry.data->Read( dest, 1, length ) ) ? 0 : 1;
}

/*-------------------------------------------------------------------------*/

CVfsBlockCache::EReadResult
CVfsBlockCache::ReadBlock( TEntryHandle handle, UInt32 index, UInt32 offsetInBlock, void* dest, UInt32 length )
{GUCEF_TRACE;

    SEntry* entry = Resolve( handle );
    if ( GUCEF_NULL == entry )
        return READ_MISS;

    CORE::CString damagedPath;
    UInt32 damagedBytes = 0;
    {
        MT::CScopeMutex lock( entry->lock );
        if ( entry->dead || !EnsureTableLocked( *entry ) || index >= entry->blockCount || 0 == entry->present[ index ] )
        {
            m_blockMisses.FetchAdd( 1 );
            return READ_MISS;
        }
        const UInt32 blockSize = BlockSizeOf( entry->fileSize, m_blockBytes, index );
        if ( offsetInBlock > blockSize || length > blockSize - offsetInBlock )
            return READ_MISS;

        const int outcome = ReadFromBlockLocked( *entry, index, offsetInBlock, dest, length );
        if ( 2 == outcome )
        {
            m_blockMisses.FetchAdd( 1 );
            return READ_MISS;
        }
        if ( 1 == outcome )
        {
            damagedPath = entry->path;
            damagedBytes = blockSize;
            DropBlockLocked( *entry, index );
        }
        else
        {
            entry->useSeq = NextUseSeq();
        }
    }

    if ( damagedBytes > 0 )
    {
        m_corruptBlocks.FetchAdd( 1 );
        m_blockMisses.FetchAdd( 1 );
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CVfsBlockCache: Block " << index << " of " << damagedPath << " is damaged in the cache and is fetched again" );
        return READ_CORRUPT;
    }
    m_blockHits.FetchAdd( 1 );
    return READ_HIT;
}

/*-------------------------------------------------------------------------*/

bool
CVfsBlockCache::StoreBlock( TEntryHandle handle, UInt32 index, const void* data, UInt32 length )
{GUCEF_TRACE;

    SEntry* entry = Resolve( handle );
    if ( GUCEF_NULL == entry || GUCEF_NULL == data )
        return false;

    bool added = false;
    bool becameComplete = false;
    UInt32 blockSize = 0;
    {
        MT::CScopeMutex lock( entry->lock );
        if ( entry->dead || !EnsureTableLocked( *entry ) || index >= entry->blockCount )
            return false;
        blockSize = BlockSizeOf( entry->fileSize, m_blockBytes, index );
        if ( length != blockSize )
            return false;

        const UInt64 start = (UInt64) index * m_blockBytes;
        if ( !EnsureOpenLocked( *entry ) || 0 != entry->data->Setpos( start ) || length != entry->data->Write( data, 1, length ) )
        {
            m_storeFailures.FetchAdd( 1 );
            if ( !entry->storeWarned )
            {
                entry->storeWarned = true;
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CVfsBlockCache: A block of " << entry->path << " cannot be kept in " << m_folder << ": the data goes on without the cache" );
            }
            return false;
        }

        const UInt32 crc = Checksum( data, length );
        added = 0 == entry->present[ index ];
        entry->present[ index ] = 1;
        entry->crc[ index ] = crc;
        entry->verified[ index ] = 1;
        entry->pending.push_back( index );
        entry->digestVerified = false;
        entry->headerDirty = true;
        if ( added )
        {
            ++entry->blocksHeld;
            entry->bytesHeld += blockSize;
            becameComplete = entry->blocksHeld == entry->blockCount;
        }
        entry->useSeq = NextUseSeq();
        if ( ++entry->unflushed >= RowsPerFlush )
            PersistLocked( *entry );
    }

    m_blocksStored.FetchAdd( 1 );
    if ( added )
    {
        m_usedBytes.FetchAdd( blockSize );
        MT::CScopeMutex cacheLock( m_lock );
        if ( becameComplete )
            OnEntryCompleteLocked( entry );
        EvictLocked();
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CVfsBlockCache::OnEntryCompleteLocked( SEntry* entry )
{GUCEF_TRACE;

    // a pin follows its path to the newer revision once that one is complete: the older one goes to the superseded
    TPathMap::iterator g = m_byPath.find( entry->pathKey );
    if ( g == m_byPath.end() || entry->superseded || entry->dead )
        return;

    bool moved = false;
    TEntryVector group = g->second;
    for ( size_t i=0; i<group.size(); ++i )
    {
        SEntry* older = group[ i ];
        if ( older == entry || !older->pinned || older->createdMs >= entry->createdMs )
            continue;
        {
            MT::CScopeMutex olderLock( older->lock );
            older->pinned = false;
            older->superseded = true;
            older->headerDirty = true;
            PersistLocked( *older );
        }
        LruInsertLocked( older );
        moved = true;
    }
    if ( moved && !entry->pinned )
    {
        {
            MT::CScopeMutex entryLock( entry->lock );
            entry->pinned = true;
            entry->headerDirty = true;
            PersistLocked( *entry );
        }
        LruRemoveLocked( entry );
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CVfsBlockCache::GetBlockCount( TEntryHandle handle ) const
{GUCEF_TRACE;

    const SEntry* entry = Resolve( handle );
    return GUCEF_NULL == entry ? 0 : entry->blockCount;
}

/*-------------------------------------------------------------------------*/

UInt64
CVfsBlockCache::GetFileSize( TEntryHandle handle ) const
{GUCEF_TRACE;

    const SEntry* entry = Resolve( handle );
    return GUCEF_NULL == entry ? 0 : entry->fileSize;
}

/*-------------------------------------------------------------------------*/

UInt32
CVfsBlockCache::GetBlockSize( TEntryHandle handle, UInt32 index ) const
{GUCEF_TRACE;

    const SEntry* entry = Resolve( handle );
    if ( GUCEF_NULL == entry || index >= entry->blockCount )
        return 0;
    return BlockSizeOf( entry->fileSize, m_blockBytes, index );
}

/*-------------------------------------------------------------------------*/

bool
CVfsBlockCache::HasBlock( TEntryHandle handle, UInt32 index ) const
{GUCEF_TRACE;

    SEntry* entry = Resolve( handle );
    if ( GUCEF_NULL == entry )
        return false;
    MT::CScopeMutex lock( entry->lock );
    return !entry->dead && const_cast< CVfsBlockCache* >( this )->EnsureTableLocked( *entry ) && index < entry->blockCount && 0 != entry->present[ index ];
}

/*-------------------------------------------------------------------------*/

bool
CVfsBlockCache::IsComplete( TEntryHandle handle ) const
{GUCEF_TRACE;

    SEntry* entry = Resolve( handle );
    if ( GUCEF_NULL == entry )
        return false;
    MT::CScopeMutex lock( entry->lock );
    return !entry->dead && const_cast< CVfsBlockCache* >( this )->EnsureTableLocked( *entry ) && entry->blocksHeld == entry->blockCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CVfsBlockCache::GetBlocksHeld( TEntryHandle handle ) const
{GUCEF_TRACE;

    SEntry* entry = Resolve( handle );
    if ( GUCEF_NULL == entry )
        return 0;
    MT::CScopeMutex lock( entry->lock );
    if ( entry->dead || !const_cast< CVfsBlockCache* >( this )->EnsureTableLocked( *entry ) )
        return 0;
    return entry->blocksHeld;
}

/*-------------------------------------------------------------------------*/

void
CVfsBlockCache::FillInfo( const SEntry& entry, SEntryInfo& info ) const
{GUCEF_TRACE;

    info.identity = entry.identity;
    info.path = entry.path;
    info.revision = entry.revision;
    info.fileSize = entry.fileSize;
    info.digest = entry.digest;
    info.blockCount = entry.blockCount;
    info.blocksHeld = entry.blocksHeld;
    info.bytesHeld = entry.bytesHeld;
    info.pinned = entry.pinned;
    info.complete = entry.blocksHeld == entry.blockCount;
    info.superseded = entry.superseded;
    info.staleRevision = false;
    info.digestVerified = entry.digestVerified;
    info.lastUseMs = entry.lastUseMs;
    info.createdMs = entry.createdMs;
}

/*-------------------------------------------------------------------------*/

bool
CVfsBlockCache::GetEntryInfo( TEntryHandle handle, SEntryInfo& info ) const
{GUCEF_TRACE;

    SEntry* entry = Resolve( handle );
    if ( GUCEF_NULL == entry )
        return false;
    MT::CScopeMutex lock( entry->lock );
    if ( entry->dead || !const_cast< CVfsBlockCache* >( this )->EnsureTableLocked( *entry ) )
        return false;
    FillInfo( *entry, info );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVfsBlockCache::FindNewestRevision( const CORE::CString& identity, const CORE::CString& path, SEntryInfo& info ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    SEntry* entry = NewestOfPathLocked( identity + "\n" + path, false );
    if ( GUCEF_NULL == entry )
        return false;
    MT::CScopeMutex entryLock( entry->lock );
    FillInfo( *entry, info );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVfsBlockCache::FindOfflineRevision( const CORE::CString& identity, const CORE::CString& path, SEntryInfo& info ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    bool stale = false;
    SEntry* entry = SelectOfflineLocked( identity + "\n" + path, stale );
    if ( GUCEF_NULL == entry )
        return false;
    MT::CScopeMutex entryLock( entry->lock );
    FillInfo( *entry, info );
    info.staleRevision = stale;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CVfsBlockCache::GetEntryInfosBelow(
 const CORE::CString& identity, const CORE::CString& dirPath, bool recursive, TEntryInfoVector& infos ) const
{GUCEF_TRACE;

    infos.clear();

    // the directory without a leading or trailing separator
    CORE::CString dir = dirPath.ReplaceChar( '\\', '/' );
    while ( dir.Length() > 0 && '/' == dir[ 0 ] )
        dir = dir.SubstrFromRange( 1, dir.Length() );
    while ( dir.Length() > 0 && '/' == dir[ dir.Length() - 1 ] )
        dir = dir.SubstrFromRange( 0, dir.Length() - 1 );
    const CORE::CString prefix = identity + "\n" + ( dir.IsNULLOrEmpty() ? CORE::CString::Empty : dir + "/" );

    MT::CScopeMutex lock( m_lock );
    TPathMap::const_iterator g = m_byPath.lower_bound( prefix );
    while ( g != m_byPath.end() && ByteCount( g->first ) >= ByteCount( prefix ) && 0 == memcmp( g->first.C_String(), prefix.C_String(), ByteCount( prefix ) ) )
    {
        const CORE::CString rest = g->first.SubstrFromRange( prefix.Length(), g->first.Length() );
        if ( rest.Length() > 0 && ( recursive || -1 == rest.HasChar( '/', true ) ) )
        {
            bool stale = false;
            SEntry* entry = SelectOfflineLocked( g->first, stale );
            if ( GUCEF_NULL != entry )
            {
                SEntryInfo info;
                {
                    MT::CScopeMutex entryLock( entry->lock );
                    FillInfo( *entry, info );
                }
                info.staleRevision = stale;
                infos.push_back( info );
            }
        }
        ++g;
    }
}

/*-------------------------------------------------------------------------*/

CVfsBlockCache::EVerifyResult
CVfsBlockCache::VerifyDigest( TEntryHandle handle, const CVfsDigest& expected )
{GUCEF_TRACE;

    SEntry* entry = Resolve( handle );
    if ( GUCEF_NULL == entry )
        return VERIFY_INCOMPLETE;
    if ( !expected.IsWellFormed() )
        return VERIFY_UNSUPPORTED;

    CVfsDigestHasher hasher( expected.algorithm );
    if ( !hasher.IsSupported() )
        return VERIFY_UNSUPPORTED;

    UInt32 damagedBlocks = 0;
    CVfsDigest actual;
    {
        MT::CScopeMutex lock( entry->lock );
        if ( entry->dead || !EnsureTableLocked( *entry ) || entry->blocksHeld != entry->blockCount )
            return VERIFY_INCOMPLETE;

        // verified before with this digest: the index says so, nothing is hashed again
        if ( entry->digestVerified && entry->digest.Matches( expected ) )
            return VERIFY_MATCH;

        GUCEF::vector< UInt8 > block;
        block.resize( m_blockBytes );
        for ( UInt32 b=0; b<entry->blockCount; ++b )
        {
            const UInt32 size = BlockSizeOf( entry->fileSize, m_blockBytes, b );
            const int outcome = ReadFromBlockLocked( *entry, b, 0, &block[ 0 ], size );
            if ( 0 != outcome )
            {
                if ( 1 == outcome )
                {
                    DropBlockLocked( *entry, b );
                    ++damagedBlocks;
                }
                break;
            }
            hasher.Update( &block[ 0 ], size );
        }
        if ( entry->blocksHeld != entry->blockCount )
        {
            if ( damagedBlocks > 0 )
            {
                m_corruptBlocks.FetchAdd( damagedBlocks );
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CVfsBlockCache: A block of " << entry->path << " is damaged in the cache: it is dropped and the file is not complete any more" );
            }
            return VERIFY_INCOMPLETE;
        }

        actual = hasher.Final();
        if ( !actual.Matches( expected ) )
            return VERIFY_MISMATCH;

        // what was hashed is what is in the index from now on
        entry->digest = actual;
        entry->digestVerified = true;
        entry->headerDirty = true;
        PersistLocked( *entry );
    }
    return VERIFY_MATCH;
}

/*-------------------------------------------------------------------------*/

void
CVfsBlockCache::SetEntryDigest( TEntryHandle handle, const CVfsDigest& digest )
{GUCEF_TRACE;

    SEntry* entry = Resolve( handle );
    if ( GUCEF_NULL == entry || digest.IsEmpty() )
        return;
    MT::CScopeMutex lock( entry->lock );
    if ( entry->digest.IsEmpty() )
    {
        entry->digest = digest;
        entry->digestVerified = false;
        entry->headerDirty = true;
    }
}

/*-------------------------------------------------------------------------*/

void
CVfsBlockCache::ResetEntry( TEntryHandle handle )
{GUCEF_TRACE;

    MT::CScopeMutex cacheLock( m_lock );
    THandleMap::const_iterator i = m_handles.find( handle );
    if ( i == m_handles.end() )
        return;
    SEntry* entry = i->second;
    MT::CScopeMutex lock( entry->lock );
    EmptyEntryLocked( *entry );
}

/*-------------------------------------------------------------------------*/

CVfsBlockCache::EPinResult
CVfsBlockCache::TryPin( const CORE::CString& identity, const CORE::CString& path )
{GUCEF_TRACE;

    const CORE::CString pathKey = identity + "\n" + path;
    MT::CScopeMutex lock( m_lock );
    if ( !m_open )
        return PIN_NO_ENTRY;

    // the newest revision that is current
    SEntry* entry = NewestOfPathLocked( pathKey, false );
    if ( GUCEF_NULL == entry )
        return PIN_NO_ENTRY;
    if ( !entry->pinned )
    {
        // a new pin has to fit what the limit leaves after the pins that are there; a path that is pinned already is moving to a newer revision
        TPathMap::const_iterator g = m_byPath.find( pathKey );
        bool pathPinned = false;
        if ( g != m_byPath.end() )
        {
            for ( size_t i=0; i<g->second.size(); ++i )
                pathPinned = pathPinned || g->second[ i ]->pinned;
        }
        if ( !pathPinned && 0 != m_maxBytes && PinnedReservedBytesLocked() + entry->fileSize > m_maxBytes )
        {
            m_pinsRefused.FetchAdd( 1 );
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CVfsBlockCache: Not pinning " << path << " (" << entry->fileSize << " bytes): the pinned files would pass the limit of " << m_maxBytes << " bytes" );
            return PIN_OVER_LIMIT;
        }
        MT::CScopeMutex entryLock( entry->lock );
        entry->pinned = true;
        entry->headerDirty = true;
        PersistLocked( *entry );
    }
    LruRemoveLocked( entry );
    return PIN_OK;
}

/*-------------------------------------------------------------------------*/

bool
CVfsBlockCache::SetPinned( const CORE::CString& identity, const CORE::CString& path, bool pinned )
{GUCEF_TRACE;

    if ( pinned )
        return PIN_OK == TryPin( identity, path );

    const CORE::CString pathKey = identity + "\n" + path;
    MT::CScopeMutex lock( m_lock );
    if ( !m_open )
        return false;

    TPathMap::iterator g = m_byPath.find( pathKey );
    if ( g == m_byPath.end() )
        return false;
    TEntryVector group = g->second;
    for ( size_t i=0; i<group.size(); ++i )
    {
        SEntry* entry = group[ i ];
        if ( entry->pinned )
        {
            {
                MT::CScopeMutex entryLock( entry->lock );
                entry->pinned = false;
                entry->headerDirty = true;
                PersistLocked( *entry );
            }
            LruInsertLocked( entry );
        }
    }
    EvictLocked();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVfsBlockCache::IsPinned( const CORE::CString& identity, const CORE::CString& path ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TPathMap::const_iterator g = m_byPath.find( identity + "\n" + path );
    if ( g == m_byPath.end() )
        return false;
    for ( size_t i=0; i<g->second.size(); ++i )
    {
        if ( g->second[ i ]->pinned )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CVfsBlockCache::HasStalePinnedCopy( const CORE::CString& identity, const CORE::CString& path ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TPathMap::const_iterator g = m_byPath.find( identity + "\n" + path );
    if ( g == m_byPath.end() )
        return false;
    for ( size_t i=0; i<g->second.size(); ++i )
    {
        if ( g->second[ i ]->pinned && g->second[ i ]->superseded && !g->second[ i ]->dead )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CVfsBlockCache::GetStalePinnedPaths( const CORE::CString& identity, CORE::CString::StringVector& paths ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    CORE::CString::StringSet found;
    for ( TEntryMap::const_iterator i = m_entries.begin(); i != m_entries.end(); ++i )
    {
        const SEntry* entry = i->second;
        if ( entry->pinned && entry->superseded && !entry->dead && entry->identity == identity )
            found.insert( entry->path );
    }
    for ( CORE::CString::StringSet::const_iterator p = found.begin(); p != found.end(); ++p )
        paths.push_back( (*p) );
}

/*-------------------------------------------------------------------------*/

UInt32
CVfsBlockCache::SetSupersededLocked(
 SEntry& entry, bool superseded )
{GUCEF_TRACE;

    if ( entry.superseded == superseded )
        return 0;
    {
        MT::CScopeMutex entryLock( entry.lock );
        entry.superseded = superseded;
        entry.headerDirty = true;
        PersistLocked( entry );
    }
    LruInsertLocked( &entry );
    return 1;
}

/*-------------------------------------------------------------------------*/

UInt32
CVfsBlockCache::SupersedeOtherRevisions( const CORE::CString& identity, const CORE::CString& path, const CORE::CString& keepRevision )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TPathMap::iterator g = m_byPath.find( identity + "\n" + path );
    if ( g == m_byPath.end() )
        return 0;

    UInt32 newlySuperseded = 0;
    TEntryVector group = g->second;
    for ( size_t i=0; i<group.size(); ++i )
    {
        SEntry* entry = group[ i ];
        const bool shouldBeSuperseded = entry->revision != keepRevision;
        if ( SetSupersededLocked( *entry, shouldBeSuperseded ) > 0 && shouldBeSuperseded )
            ++newlySuperseded;
    }
    EvictLocked();
    return newlySuperseded;
}

/*-------------------------------------------------------------------------*/

UInt64
CVfsBlockCache::RemoveEntries( const CORE::CString& identity, const CORE::CString& path )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TPathMap::iterator g = m_byPath.find( identity + "\n" + path );
    if ( g == m_byPath.end() )
        return 0;

    UInt64 freed = 0;
    TEntryVector group = g->second;
    for ( size_t i=0; i<group.size(); ++i )
    {
        SEntry* entry = group[ i ];
        if ( entry->refs > 0 )
            continue;
        freed += entry->bytesHeld;
        DeleteEntryLocked( entry, true );
    }
    return freed;
}

/*-------------------------------------------------------------------------*/

UInt64
CVfsBlockCache::Clear( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TEntryVector victims;
    for ( TEntryMap::const_iterator i = m_entries.begin(); i != m_entries.end(); ++i )
    {
        if ( i->second->refs == 0 && !i->second->pinned )
            victims.push_back( i->second );
    }

    UInt64 freed = 0;
    for ( size_t v=0; v<victims.size(); ++v )
    {
        freed += victims[ v ]->bytesHeld;
        DeleteEntryLocked( victims[ v ], true );
    }
    return freed;
}

/*-------------------------------------------------------------------------*/

void
CVfsBlockCache::Flush( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( TEntryMap::iterator i = m_entries.begin(); i != m_entries.end(); ++i )
    {
        SEntry* entry = i->second;
        MT::CScopeMutex entryLock( entry->lock );
        PersistLocked( *entry );
    }
}

/*-------------------------------------------------------------------------*/

void
CVfsBlockCache::GetStatistics( SStatistics& statistics ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    statistics.maxBytes = m_maxBytes;
    statistics.usedBytes = m_usedBytes.Load();
    statistics.entries = (UInt32) m_entries.size();
    statistics.pinnedBytes = 0;
    statistics.pinnedEntries = 0;
    for ( TEntryMap::const_iterator i = m_entries.begin(); i != m_entries.end(); ++i )
    {
        if ( i->second->pinned )
        {
            ++statistics.pinnedEntries;
            statistics.pinnedBytes += i->second->bytesHeld;
        }
    }
    statistics.blockHits = m_blockHits.Load();
    statistics.blockMisses = m_blockMisses.Load();
    statistics.blocksStored = m_blocksStored.Load();
    statistics.corruptBlocks = m_corruptBlocks.Load();
    statistics.evictedEntries = m_evictedEntries.Load();
    statistics.evictedBytes = m_evictedBytes.Load();
    statistics.supersededEvictions = m_supersededEvictions.Load();
    statistics.pinsRefused = m_pinsRefused.Load();
    statistics.pinnedReservedBytes = PinnedReservedBytesLocked();
    statistics.storeFailures = m_storeFailures.Load();
    statistics.fileReopens = m_fileReopens.Load();
    {
        MT::CScopeMutex openLock( m_openLock );
        statistics.openFiles = (UInt32) m_openRing.size();
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
