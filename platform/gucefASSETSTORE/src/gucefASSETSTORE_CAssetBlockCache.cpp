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

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefASSETSTORE_CAssetBlockCache.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

static const UInt32 IndexMagic = 0x31434247;            // "GBC1"
static const UInt32 IndexVersion = 1;
static const UInt32 HeaderBytes = 1024;
static const UInt32 PathOffset = 128;
static const UInt32 TableEntryBytes = 8;
static const UInt32 FlagPinned = 1;
static const UInt64 ScannedUseBase = 0;
static const UInt64 RuntimeUseBase = 1000000000ULL;

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
    return RuntimeUseBase + ( resolution > 0 ? ( MT::PrecisionTickCount() * 1000ULL ) / resolution : 0 );
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

} // anonymous namespace

/*-------------------------------------------------------------------------*/

struct CAssetBlockCache::SEntry
{
    CORE::CString path;
    CORE::CString key;
    CORE::CString digest;
    UInt64 fileSize;
    UInt32 blockCount;
    UInt32 blocksHeld;
    bool pinned;
    bool tableLoaded;
    bool headerDirty;
    UInt32 unflushed;
    GUCEF::vector< UInt8 > present;
    GUCEF::vector< UInt32 > crc;
    GUCEF::vector< UInt8 > verified;
    UInt32 refs;
    UInt64 lastUseMs;
    TEntryHandle handle;
    CORE::CFileAccess* data;
    CORE::CFileAccess* index;
    MT::CMutex lock;

    SEntry( void )
        : path(), key(), digest(), fileSize( 0 ), blockCount( 0 ), blocksHeld( 0 ), pinned( false ), tableLoaded( false ), headerDirty( false )
        , unflushed( 0 ), present(), crc(), verified(), refs( 0 ), lastUseMs( 0 ), handle( 0 ), data( GUCEF_NULL ), index( GUCEF_NULL ), lock()
    {
    }
};

/*-------------------------------------------------------------------------*/

CAssetBlockCache::SEntryInfo::SEntryInfo( void )
    : path(), fileSize( 0 ), digest(), blockCount( 0 ), blocksHeld( 0 ), bytesHeld( 0 ), pinned( false ), complete( false ), lastUseMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetBlockCache::SStatistics::SStatistics( void )
    : maxBytes( 0 ), usedBytes( 0 ), pinnedBytes( 0 ), entries( 0 ), pinnedEntries( 0 ), blockHits( 0 ), blockMisses( 0 ), blocksStored( 0 )
    , corruptBlocks( 0 ), evictedEntries( 0 ), evictedBytes( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetBlockCache::CAssetBlockCache( void )
    : m_lock()
    , m_folder()
    , m_open( false )
    , m_maxBytes( 0 )
    , m_usedBytes( 0 )
    , m_entries()
    , m_handles()
    , m_nextHandle( 1 )
    , m_blockHits( 0 )
    , m_blockMisses( 0 )
    , m_blocksStored( 0 )
    , m_corruptBlocks( 0 )
    , m_evictedEntries( 0 )
    , m_evictedBytes( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetBlockCache::~CAssetBlockCache()
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetBlockCache::Checksum( const void* data, UInt32 length, UInt32 seed )
{
    UInt32 c = seed ^ 0xFFFFFFFFU;
    const UInt8* bytes = static_cast< const UInt8* >( data );
    for ( UInt32 i=0; i<length; ++i )
        c = s_crc.table[ ( c ^ bytes[ i ] ) & 0xFF ] ^ ( c >> 8 );
    return c ^ 0xFFFFFFFFU;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetBlockCache::MakeKey( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    CORE::CSha256 sha;
    sha.Update( vfsPath.C_String(), vfsPath.Length() );
    UInt8 digest[ CORE::CSha256::DigestSize ];
    sha.Final( digest );
    CORE::CString hex = CORE::ConvertBytesToHexString( digest, 16, false, false );
    return hex.Lowercase();
}

/*-------------------------------------------------------------------------*/

UInt64
CAssetBlockCache::BytesOfBlocks( const SEntry& entry, UInt32 blocks ) const
{
    // all blocks are full except the last one of the file
    return (UInt64) blocks * BlockBytes;
}

/*-------------------------------------------------------------------------*/

bool
CAssetBlockCache::Open( const CORE::CString& folder, UInt64 maxBytes )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_open )
        return true;
    if ( folder.IsNULLOrEmpty() )
        return false;
    if ( !CORE::DirExists( folder ) && !CORE::CreateDirs( folder ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CAssetBlockCache: The cache folder " << folder << " cannot be made" );
        return false;
    }
    m_folder = folder;
    m_maxBytes = maxBytes;
    m_open = true;
    ScanFolder();
    EvictLocked();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CAssetBlockCache: Cache folder " << m_folder << " holds " << (UInt32) m_entries.size() << " files, " << m_usedBytes << " bytes" );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetBlockCache::Close( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TEntryMap::iterator i = m_entries.begin();
    while ( i != m_entries.end() )
    {
        SEntry* entry = i->second;
        {
            MT::CScopeMutex entryLock( entry->lock );
            if ( entry->headerDirty )
                WriteHeader( *entry );
            CloseData( *entry );
        }
        GUCEF_DELETE entry;
        ++i;
    }
    m_entries.clear();
    m_handles.clear();
    m_usedBytes = 0;
    m_open = false;
}

/*-------------------------------------------------------------------------*/

bool
CAssetBlockCache::IsOpen( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_open;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetBlockCache::GetFolder( void ) const
{
    return m_folder;
}

/*-------------------------------------------------------------------------*/

void
CAssetBlockCache::SetMaxBytes( UInt64 maxBytes )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_maxBytes = maxBytes;
    EvictLocked();
}

/*-------------------------------------------------------------------------*/

UInt64
CAssetBlockCache::GetMaxBytes( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_maxBytes;
}

/*-------------------------------------------------------------------------*/

UInt64
CAssetBlockCache::GetUsedBytes( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_usedBytes;
}

/*-------------------------------------------------------------------------*/

void
CAssetBlockCache::ScanFolder( void )
{GUCEF_TRACE;

    // the entries that are in the folder, oldest first by the time of their index: the order of use after a restart
    typedef GUCEF::map< Int64, CORE::CString::StringVector > TByTime;
    TByTime byTime;
    CORE::SDI_Data* dir = CORE::DI_First_Dir_Entry( m_folder.C_String() );
    if ( GUCEF_NULL == dir )
        return;
    do
    {
        if ( 0 == CORE::DI_Is_It_A_File( dir ) )
            continue;
        const CORE::CString name = CORE::DI_Name( dir );
        if ( name.Length() < 5 || name.SubstrFromRange( name.Length() - 4, name.Length() ) != ".idx" )
            continue;
        byTime[ (Int64) CORE::DI_Timestamp( dir ) ].push_back( name.SubstrFromRange( 0, name.Length() - 4 ) );
    }
    while ( 0 != CORE::DI_Next_Dir_Entry( dir ) );
    CORE::DI_Cleanup( dir );

    UInt64 rank = ScannedUseBase;
    for ( TByTime::const_iterator t = byTime.begin(); t != byTime.end(); ++t )
    {
        for ( size_t k=0; k<t->second.size(); ++k )
        {
            SEntry* entry = LoadEntryLocked( t->second[ k ] );
            if ( GUCEF_NULL != entry )
                entry->lastUseMs = ++rank;
        }
    }
}

/*-------------------------------------------------------------------------*/

CAssetBlockCache::SEntry*
CAssetBlockCache::LoadEntryLocked( const CORE::CString& key )
{GUCEF_TRACE;

    SEntry* entry = GUCEF_NEW SEntry();
    entry->key = key;
    if ( !ReadIndex( *entry, false ) || MakeKey( entry->path ) != key || !CORE::FileExists( CORE::CombinePath( m_folder, key + ".dat" ) ) )
    {
        GUCEF_DELETE entry;
        // an index without data, or one that is no index: gone
        CORE::DeleteFile( CORE::CombinePath( m_folder, key + ".idx" ) );
        CORE::DeleteFile( CORE::CombinePath( m_folder, key + ".dat" ) );
        return GUCEF_NULL;
    }
    if ( m_entries.find( entry->path ) != m_entries.end() )
    {
        GUCEF_DELETE entry;
        return GUCEF_NULL;
    }
    entry->handle = m_nextHandle++;
    m_entries[ entry->path ] = entry;
    m_handles[ entry->handle ] = entry;
    m_usedBytes += BytesOfBlocks( *entry, entry->blocksHeld );
    return entry;
}

/*-------------------------------------------------------------------------*/

bool
CAssetBlockCache::ReadIndex( SEntry& entry, bool tableToo ) const
{GUCEF_TRACE;

    CORE::CFileAccess file( CORE::CombinePath( m_folder, entry.key + ".idx" ), "rb" );
    if ( !file.Opened() )
        return false;
    UInt8 header[ HeaderBytes ];
    if ( HeaderBytes != file.Read( header, 1, HeaderBytes ) )
        return false;
    if ( IndexMagic != GetU32( header, 0 ) || IndexVersion != GetU32( header, 4 ) || BlockBytes != GetU32( header, 8 ) )
        return false;
    const UInt32 flags = GetU32( header, 12 );
    const UInt64 fileSize = GetU64( header, 16 );
    const UInt32 blockCount = GetU32( header, 24 );
    const UInt32 blocksHeld = GetU32( header, 28 );
    const UInt32 pathLength = GetU32( header, 96 );
    if ( pathLength == 0 || pathLength > MaxPathBytes || blocksHeld > blockCount || blockCount != (UInt32) ( ( fileSize + BlockBytes - 1 ) / BlockBytes ) )
        return false;
    entry.pinned = 0 != ( flags & FlagPinned );
    entry.fileSize = fileSize;
    entry.blockCount = blockCount;
    entry.blocksHeld = blocksHeld;
    char digest[ 65 ];
    memcpy( digest, header + 32, 64 );
    digest[ 64 ] = '\0';
    entry.digest = digest;
    entry.path.Set( reinterpret_cast< const char* >( header ) + PathOffset, pathLength );
    if ( !tableToo )
        return true;

    // the table: which blocks are held, and their checksums
    entry.present.assign( blockCount, 0 );
    entry.crc.assign( blockCount, 0 );
    entry.verified.assign( blockCount, 0 );
    GUCEF::vector< UInt8 > table;
    table.resize( (size_t) blockCount * TableEntryBytes );
    if ( blockCount > 0 && table.size() != file.Read( &table[ 0 ], 1, (UInt32) table.size() ) )
        return false;
    UInt32 held = 0;
    for ( UInt32 b=0; b<blockCount; ++b )
    {
        if ( 1 == GetU32( &table[ 0 ], b * TableEntryBytes + 4 ) )
        {
            entry.present[ b ] = 1;
            entry.crc[ b ] = GetU32( &table[ 0 ], b * TableEntryBytes );
            ++held;
        }
    }
    entry.blocksHeld = held;
    entry.tableLoaded = true;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetBlockCache::WriteHeader( SEntry& entry ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == entry.index )
        return;
    UInt8 header[ HeaderBytes ];
    memset( header, 0, sizeof( header ) );
    PutU32( header, 0, IndexMagic );
    PutU32( header, 4, IndexVersion );
    PutU32( header, 8, BlockBytes );
    PutU32( header, 12, entry.pinned ? FlagPinned : 0 );
    PutU64( header, 16, entry.fileSize );
    PutU32( header, 24, entry.blockCount );
    PutU32( header, 28, entry.blocksHeld );
    memcpy( header + 32, entry.digest.C_String(), entry.digest.Length() > 64 ? 64 : entry.digest.Length() );
    PutU32( header, 96, entry.path.Length() );
    memcpy( header + PathOffset, entry.path.C_String(), entry.path.Length() );
    entry.index->Setpos( 0 );
    entry.index->Write( header, 1, HeaderBytes );
    entry.index->Flush();
    entry.headerDirty = false;
    entry.unflushed = 0;
}

/*-------------------------------------------------------------------------*/

bool
CAssetBlockCache::OpenData( SEntry& entry ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != entry.data && GUCEF_NULL != entry.index )
        return true;
    CloseData( entry );
    entry.data = GUCEF_NEW CORE::CFileAccess( CORE::CombinePath( m_folder, entry.key + ".dat" ), "r+b" );
    entry.index = GUCEF_NEW CORE::CFileAccess( CORE::CombinePath( m_folder, entry.key + ".idx" ), "r+b" );
    if ( !entry.data->Opened() || !entry.index->Opened() )
    {
        CloseData( entry );
        return false;
    }
    if ( !entry.tableLoaded && !ReadIndex( entry, true ) )
    {
        CloseData( entry );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetBlockCache::CloseData( SEntry& entry ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != entry.index )
    {
        entry.index->Flush();
        GUCEF_DELETE entry.index;
        entry.index = GUCEF_NULL;
    }
    if ( GUCEF_NULL != entry.data )
    {
        entry.data->Flush();
        GUCEF_DELETE entry.data;
        entry.data = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

bool
CAssetBlockCache::InitEntry( SEntry& entry, UInt64 fileSize, const CORE::CString& digest )
{GUCEF_TRACE;

    // a new content: the data file is made empty and the index has no blocks
    CloseData( entry );
    entry.fileSize = fileSize;
    entry.blockCount = (UInt32) ( ( fileSize + BlockBytes - 1 ) / BlockBytes );
    entry.blocksHeld = 0;
    entry.digest = digest;
    entry.present.assign( entry.blockCount, 0 );
    entry.crc.assign( entry.blockCount, 0 );
    entry.verified.assign( entry.blockCount, 0 );
    entry.tableLoaded = true;
    {
        CORE::CFileAccess data( CORE::CombinePath( m_folder, entry.key + ".dat" ), "wb" );
        if ( !data.Opened() )
            return false;
    }
    UInt8 header[ HeaderBytes ];
    memset( header, 0, sizeof( header ) );
    PutU32( header, 0, IndexMagic );
    PutU32( header, 4, IndexVersion );
    PutU32( header, 8, BlockBytes );
    PutU32( header, 12, entry.pinned ? FlagPinned : 0 );
    PutU64( header, 16, entry.fileSize );
    PutU32( header, 24, entry.blockCount );
    PutU32( header, 28, 0 );
    memcpy( header + 32, digest.C_String(), digest.Length() > 64 ? 64 : digest.Length() );
    PutU32( header, 96, entry.path.Length() );
    memcpy( header + PathOffset, entry.path.C_String(), entry.path.Length() );
    CORE::CFileAccess index( CORE::CombinePath( m_folder, entry.key + ".idx" ), "wb" );
    if ( !index.Opened() )
        return false;
    index.Write( header, 1, HeaderBytes );
    GUCEF::vector< UInt8 > table;
    table.assign( (size_t) entry.blockCount * TableEntryBytes, 0 );
    if ( !table.empty() )
        index.Write( &table[ 0 ], 1, (UInt32) table.size() );
    index.Flush();
    return true;
}

/*-------------------------------------------------------------------------*/

CAssetBlockCache::TEntryHandle
CAssetBlockCache::AcquireEntry( const CORE::CString& vfsPath, UInt64 fileSize, const CORE::CString& digest )
{GUCEF_TRACE;

    if ( 0 == fileSize || vfsPath.IsNULLOrEmpty() || vfsPath.Length() > MaxPathBytes )
        return 0;
    MT::CScopeMutex lock( m_lock );
    if ( !m_open )
        return 0;
    SEntry* entry = GUCEF_NULL;
    TEntryMap::iterator found = m_entries.find( vfsPath );
    if ( found != m_entries.end() )
    {
        entry = found->second;
        const bool otherContent = entry->fileSize != fileSize || ( !digest.IsNULLOrEmpty() && !entry->digest.IsNULLOrEmpty() && digest != entry->digest );
        if ( otherContent )
        {
            if ( entry->refs > 0 )
                return 0;
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CAssetBlockCache: The content of " << vfsPath << " changed on the service, the cached blocks are dropped" );
            m_usedBytes -= BytesOfBlocks( *entry, entry->blocksHeld );
            if ( !InitEntry( *entry, fileSize, digest ) )
                return 0;
        }
    }
    else
    {
        entry = GUCEF_NEW SEntry();
        entry->path = vfsPath;
        entry->key = MakeKey( vfsPath );
        if ( !InitEntry( *entry, fileSize, digest ) )
        {
            GUCEF_DELETE entry;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CAssetBlockCache: The cache files of " << vfsPath << " cannot be made in " << m_folder );
            return 0;
        }
        entry->handle = m_nextHandle++;
        m_entries[ vfsPath ] = entry;
        m_handles[ entry->handle ] = entry;
    }
    {
        MT::CScopeMutex entryLock( entry->lock );
        if ( !OpenData( *entry ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CAssetBlockCache: The cache files of " << vfsPath << " cannot be opened in " << m_folder );
            return 0;
        }
        if ( entry->digest.IsNULLOrEmpty() && !digest.IsNULLOrEmpty() )
        {
            entry->digest = digest;
            entry->headerDirty = true;
        }
    }
    ++entry->refs;
    entry->lastUseMs = NowMs();
    return entry->handle;
}

/*-------------------------------------------------------------------------*/

CAssetBlockCache::SEntry*
CAssetBlockCache::Resolve( TEntryHandle entry ) const
{
    MT::CScopeMutex lock( m_lock );
    THandleMap::const_iterator i = m_handles.find( entry );
    return i == m_handles.end() ? GUCEF_NULL : i->second;
}

/*-------------------------------------------------------------------------*/

void
CAssetBlockCache::ReleaseEntry( TEntryHandle handle )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    THandleMap::const_iterator i = m_handles.find( handle );
    if ( i == m_handles.end() || i->second->refs == 0 )
        return;
    SEntry* entry = i->second;
    if ( --entry->refs > 0 )
        return;
    MT::CScopeMutex entryLock( entry->lock );
    if ( entry->headerDirty || entry->unflushed > 0 )
        WriteHeader( *entry );
    CloseData( *entry );
    EvictLocked();
}

/*-------------------------------------------------------------------------*/

void
CAssetBlockCache::SetEntryDigest( TEntryHandle handle, const CORE::CString& digest )
{GUCEF_TRACE;

    SEntry* entry = Resolve( handle );
    if ( GUCEF_NULL == entry )
        return;
    MT::CScopeMutex lock( entry->lock );
    if ( entry->digest.IsNULLOrEmpty() && !digest.IsNULLOrEmpty() )
    {
        entry->digest = digest;
        entry->headerDirty = true;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetBlockCache::GetBlockCount( TEntryHandle handle ) const
{
    const SEntry* entry = Resolve( handle );
    return GUCEF_NULL == entry ? 0 : entry->blockCount;
}

/*-------------------------------------------------------------------------*/

UInt64
CAssetBlockCache::GetFileSize( TEntryHandle handle ) const
{
    const SEntry* entry = Resolve( handle );
    return GUCEF_NULL == entry ? 0 : entry->fileSize;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetBlockCache::GetBlockSize( TEntryHandle handle, UInt32 index ) const
{
    const SEntry* entry = Resolve( handle );
    if ( GUCEF_NULL == entry || index >= entry->blockCount )
        return 0;
    const UInt64 start = (UInt64) index * BlockBytes;
    const UInt64 left = entry->fileSize - start;
    return left < BlockBytes ? (UInt32) left : (UInt32) BlockBytes;
}

/*-------------------------------------------------------------------------*/

bool
CAssetBlockCache::HasBlock( TEntryHandle handle, UInt32 index ) const
{
    SEntry* entry = Resolve( handle );
    if ( GUCEF_NULL == entry )
        return false;
    MT::CScopeMutex lock( entry->lock );
    return index < entry->blockCount && 0 != entry->present[ index ];
}

/*-------------------------------------------------------------------------*/

bool
CAssetBlockCache::IsComplete( TEntryHandle handle ) const
{
    SEntry* entry = Resolve( handle );
    if ( GUCEF_NULL == entry )
        return false;
    MT::CScopeMutex lock( entry->lock );
    return entry->blocksHeld == entry->blockCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetBlockCache::GetBlocksHeld( TEntryHandle handle ) const
{
    SEntry* entry = Resolve( handle );
    if ( GUCEF_NULL == entry )
        return 0;
    MT::CScopeMutex lock( entry->lock );
    return entry->blocksHeld;
}

/*-------------------------------------------------------------------------*/

CAssetBlockCache::EReadResult
CAssetBlockCache::ReadBlock( TEntryHandle handle, UInt32 index, UInt32 offsetInBlock, void* dest, UInt32 length )
{GUCEF_TRACE;

    SEntry* entry = Resolve( handle );
    if ( GUCEF_NULL == entry )
        return READ_MISS;
    bool corrupt = false;
    {
        MT::CScopeMutex lock( entry->lock );
        if ( index >= entry->blockCount || 0 == entry->present[ index ] || GUCEF_NULL == entry->data )
        {
            m_blockMisses.FetchAdd( 1 );
            return READ_MISS;
        }
        const UInt64 start = (UInt64) index * BlockBytes;
        const UInt64 left = entry->fileSize - start;
        const UInt32 blockSize = left < BlockBytes ? (UInt32) left : (UInt32) BlockBytes;
        if ( offsetInBlock + length > blockSize )
            return READ_MISS;
        if ( 0 == entry->verified[ index ] )
        {
            // the first read of a block in this run: the whole block against its checksum
            GUCEF::vector< UInt8 > block;
            block.resize( blockSize );
            if ( 0 != entry->data->Setpos( start ) || blockSize != entry->data->Read( &block[ 0 ], 1, blockSize ) || Checksum( &block[ 0 ], blockSize ) != entry->crc[ index ] )
            {
                corrupt = true;
            }
            else
            {
                entry->verified[ index ] = 1;
                memcpy( dest, &block[ 0 ] + offsetInBlock, length );
            }
        }
        else
        if ( 0 != entry->data->Setpos( start + offsetInBlock ) || length != entry->data->Read( dest, 1, length ) )
        {
            corrupt = true;
        }
        if ( corrupt )
        {
            entry->present[ index ] = 0;
            entry->verified[ index ] = 0;
            --entry->blocksHeld;
            entry->headerDirty = true;
            UInt8 cleared[ TableEntryBytes ];
            memset( cleared, 0, sizeof( cleared ) );
            if ( GUCEF_NULL != entry->index )
            {
                entry->index->Setpos( HeaderBytes + (UInt64) index * TableEntryBytes );
                entry->index->Write( cleared, 1, TableEntryBytes );
            }
        }
    }
    if ( corrupt )
    {
        m_corruptBlocks.FetchAdd( 1 );
        m_blockMisses.FetchAdd( 1 );
        {
            MT::CScopeMutex cacheLock( m_lock );
            m_usedBytes -= m_usedBytes >= BlockBytes ? BlockBytes : m_usedBytes;
        }
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CAssetBlockCache: Block " << index << " of " << entry->path << " is damaged in the cache and is fetched again" );
        return READ_CORRUPT;
    }
    m_blockHits.FetchAdd( 1 );
    entry->lastUseMs = NowMs();
    return READ_HIT;
}

/*-------------------------------------------------------------------------*/

bool
CAssetBlockCache::StoreBlock( TEntryHandle handle, UInt32 index, const void* data, UInt32 length )
{GUCEF_TRACE;

    SEntry* entry = Resolve( handle );
    if ( GUCEF_NULL == entry )
        return false;
    bool added = false;
    {
        MT::CScopeMutex lock( entry->lock );
        if ( index >= entry->blockCount || GUCEF_NULL == entry->data || GUCEF_NULL == entry->index )
            return false;
        const UInt64 start = (UInt64) index * BlockBytes;
        const UInt64 left = entry->fileSize - start;
        const UInt32 blockSize = left < BlockBytes ? (UInt32) left : (UInt32) BlockBytes;
        if ( length != blockSize )
            return false;
        if ( 0 != entry->data->Setpos( start ) || length != entry->data->Write( data, 1, length ) )
            return false;
        const UInt32 crc = Checksum( data, length );
        UInt8 row[ TableEntryBytes ];
        PutU32( row, 0, crc );
        PutU32( row, 4, 1 );
        entry->index->Setpos( HeaderBytes + (UInt64) index * TableEntryBytes );
        entry->index->Write( row, 1, TableEntryBytes );
        added = 0 == entry->present[ index ];
        entry->present[ index ] = 1;
        entry->crc[ index ] = crc;
        entry->verified[ index ] = 1;
        if ( added )
            ++entry->blocksHeld;
        entry->headerDirty = true;
        if ( ++entry->unflushed >= 16 )
        {
            entry->data->Flush();
            WriteHeader( *entry );
        }
    }
    m_blocksStored.FetchAdd( 1 );
    MT::CScopeMutex cacheLock( m_lock );
    if ( added )
    {
        m_usedBytes += BlockBytes;
        entry->lastUseMs = NowMs();
        EvictLocked();
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetBlockCache::VerifyDigest( TEntryHandle handle, const CORE::CString& expectedDigest )
{GUCEF_TRACE;

    SEntry* entry = Resolve( handle );
    if ( GUCEF_NULL == entry )
        return false;
    MT::CScopeMutex lock( entry->lock );
    if ( entry->blocksHeld != entry->blockCount || GUCEF_NULL == entry->data )
        return false;
    CORE::CSha256 sha;
    GUCEF::vector< UInt8 > piece;
    piece.resize( BlockBytes );
    entry->data->Setpos( 0 );
    UInt64 left = entry->fileSize;
    while ( left > 0 )
    {
        const UInt32 want = left < BlockBytes ? (UInt32) left : (UInt32) BlockBytes;
        if ( want != entry->data->Read( &piece[ 0 ], 1, want ) )
            return false;
        sha.Update( &piece[ 0 ], want );
        left -= want;
    }
    UInt8 digest[ CORE::CSha256::DigestSize ];
    sha.Final( digest );
    CORE::CString hex = CORE::ConvertBytesToHexString( digest, CORE::CSha256::DigestSize, false, false );
    return hex.Lowercase() == expectedDigest.Lowercase();
}

/*-------------------------------------------------------------------------*/

void
CAssetBlockCache::ResetEntry( TEntryHandle handle )
{GUCEF_TRACE;

    MT::CScopeMutex cacheLock( m_lock );
    THandleMap::const_iterator i = m_handles.find( handle );
    if ( i == m_handles.end() )
        return;
    SEntry* entry = i->second;
    MT::CScopeMutex lock( entry->lock );
    const UInt64 freed = BytesOfBlocks( *entry, entry->blocksHeld );
    m_usedBytes -= freed < m_usedBytes ? freed : m_usedBytes;
    const bool pinned = entry->pinned;
    const UInt32 refs = entry->refs;
    InitEntry( *entry, entry->fileSize, CORE::CString::Empty );
    entry->pinned = pinned;
    if ( refs > 0 )
        OpenData( *entry );
}

/*-------------------------------------------------------------------------*/

bool
CAssetBlockCache::IsPinned( const CORE::CString& vfsPath ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TEntryMap::const_iterator i = m_entries.find( vfsPath );
    return i != m_entries.end() && i->second->pinned;
}

/*-------------------------------------------------------------------------*/

bool
CAssetBlockCache::SetPinned( const CORE::CString& vfsPath, bool pinned )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TEntryMap::iterator i = m_entries.find( vfsPath );
    if ( i == m_entries.end() )
        return false;
    SEntry* entry = i->second;
    MT::CScopeMutex entryLock( entry->lock );
    if ( entry->pinned != pinned )
    {
        entry->pinned = pinned;
        // the flag is in the index at once, so a restart keeps it
        if ( GUCEF_NULL != entry->index )
        {
            WriteHeader( *entry );
        }
        else
        {
            CORE::CFileAccess index( CORE::CombinePath( m_folder, entry->key + ".idx" ), "r+b" );
            if ( index.Opened() )
            {
                UInt32 flags = pinned ? FlagPinned : 0;
                index.Setpos( 12 );
                index.Write( &flags, 1, sizeof( flags ) );
                index.Flush();
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetBlockCache::SetEntryPinned( TEntryHandle handle, bool pinned )
{GUCEF_TRACE;

    SEntry* entry = Resolve( handle );
    if ( GUCEF_NULL != entry )
        SetPinned( entry->path, pinned );
}

/*-------------------------------------------------------------------------*/

void
CAssetBlockCache::DeleteEntryFilesLocked( SEntry* entry )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( entry->lock );
        CloseData( *entry );
    }
    CORE::DeleteFile( CORE::CombinePath( m_folder, entry->key + ".dat" ) );
    CORE::DeleteFile( CORE::CombinePath( m_folder, entry->key + ".idx" ) );
}

/*-------------------------------------------------------------------------*/

UInt64
CAssetBlockCache::RemoveEntry( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TEntryMap::iterator i = m_entries.find( vfsPath );
    if ( i == m_entries.end() || i->second->refs > 0 )
        return 0;
    SEntry* entry = i->second;
    const UInt64 freed = BytesOfBlocks( *entry, entry->blocksHeld );
    DeleteEntryFilesLocked( entry );
    m_usedBytes -= freed < m_usedBytes ? freed : m_usedBytes;
    m_handles.erase( entry->handle );
    m_entries.erase( i );
    GUCEF_DELETE entry;
    return freed;
}

/*-------------------------------------------------------------------------*/

UInt64
CAssetBlockCache::Clear( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    UInt64 freed = 0;
    TEntryMap::iterator i = m_entries.begin();
    while ( i != m_entries.end() )
    {
        SEntry* entry = i->second;
        if ( entry->refs > 0 || entry->pinned )
        {
            ++i;
            continue;
        }
        const UInt64 bytes = BytesOfBlocks( *entry, entry->blocksHeld );
        freed += bytes;
        DeleteEntryFilesLocked( entry );
        m_usedBytes -= bytes < m_usedBytes ? bytes : m_usedBytes;
        m_handles.erase( entry->handle );
        GUCEF_DELETE entry;
        m_entries.erase( i++ );
    }
    return freed;
}

/*-------------------------------------------------------------------------*/

void
CAssetBlockCache::EvictLocked( void )
{GUCEF_TRACE;

    if ( 0 == m_maxBytes )
        return;
    while ( m_usedBytes > m_maxBytes )
    {
        // the entry that was used longest ago, that is not pinned and not in use
        SEntry* oldest = GUCEF_NULL;
        for ( TEntryMap::iterator i = m_entries.begin(); i != m_entries.end(); ++i )
        {
            SEntry* entry = i->second;
            if ( entry->refs > 0 || entry->pinned )
                continue;
            if ( GUCEF_NULL == oldest || entry->lastUseMs < oldest->lastUseMs )
                oldest = entry;
        }
        if ( GUCEF_NULL == oldest )
            return;
        const UInt64 bytes = BytesOfBlocks( *oldest, oldest->blocksHeld );
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CAssetBlockCache: Removing " << oldest->path << " (" << bytes << " bytes) to stay under the limit" );
        DeleteEntryFilesLocked( oldest );
        m_usedBytes -= bytes < m_usedBytes ? bytes : m_usedBytes;
        ++m_evictedEntries;
        m_evictedBytes += bytes;
        m_handles.erase( oldest->handle );
        m_entries.erase( oldest->path );
        GUCEF_DELETE oldest;
    }
}

/*-------------------------------------------------------------------------*/

bool
CAssetBlockCache::GetEntryInfo( const CORE::CString& vfsPath, SEntryInfo& info ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TEntryMap::const_iterator i = m_entries.find( vfsPath );
    if ( i == m_entries.end() )
        return false;
    const SEntry& entry = *i->second;
    info.path = entry.path;
    info.fileSize = entry.fileSize;
    info.digest = entry.digest;
    info.blockCount = entry.blockCount;
    info.blocksHeld = entry.blocksHeld;
    info.bytesHeld = BytesOfBlocks( entry, entry.blocksHeld );
    info.pinned = entry.pinned;
    info.complete = entry.blocksHeld == entry.blockCount;
    info.lastUseMs = entry.lastUseMs;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetBlockCache::GetEntryInfos( TEntryInfoVector& infos ) const
{GUCEF_TRACE;

    infos.clear();
    MT::CScopeMutex lock( m_lock );
    for ( TEntryMap::const_iterator i = m_entries.begin(); i != m_entries.end(); ++i )
    {
        const SEntry& entry = *i->second;
        SEntryInfo info;
        info.path = entry.path;
        info.fileSize = entry.fileSize;
        info.digest = entry.digest;
        info.blockCount = entry.blockCount;
        info.blocksHeld = entry.blocksHeld;
        info.bytesHeld = BytesOfBlocks( entry, entry.blocksHeld );
        info.pinned = entry.pinned;
        info.complete = entry.blocksHeld == entry.blockCount;
        info.lastUseMs = entry.lastUseMs;
        infos.push_back( info );
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetBlockCache::Flush( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( TEntryMap::iterator i = m_entries.begin(); i != m_entries.end(); ++i )
    {
        SEntry* entry = i->second;
        MT::CScopeMutex entryLock( entry->lock );
        if ( GUCEF_NULL != entry->data )
            entry->data->Flush();
        if ( entry->headerDirty )
            WriteHeader( *entry );
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetBlockCache::GetStatistics( SStatistics& statistics ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    statistics.maxBytes = m_maxBytes;
    statistics.usedBytes = m_usedBytes;
    statistics.entries = (UInt32) m_entries.size();
    statistics.pinnedBytes = 0;
    statistics.pinnedEntries = 0;
    for ( TEntryMap::const_iterator i = m_entries.begin(); i != m_entries.end(); ++i )
    {
        if ( i->second->pinned )
        {
            ++statistics.pinnedEntries;
            statistics.pinnedBytes += BytesOfBlocks( *i->second, i->second->blocksHeld );
        }
    }
    statistics.blockHits = m_blockHits.Load();
    statistics.blockMisses = m_blockMisses.Load();
    statistics.blocksStored = m_blocksStored.Load();
    statistics.corruptBlocks = m_corruptBlocks.Load();
    statistics.evictedEntries = m_evictedEntries;
    statistics.evictedBytes = m_evictedBytes;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
