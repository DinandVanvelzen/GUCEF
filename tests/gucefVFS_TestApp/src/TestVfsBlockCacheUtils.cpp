/*
 *  gucefVFS_TestApp: GUCEF test application for the VFS module
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

#include "gucef_essentials.h"

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
  #include <unistd.h>
  #include <sys/stat.h>
#endif

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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_VFS_CVFSDIGESTHASHER_H
#include "gucefVFS_CVfsDigestHasher.h"
#define GUCEF_VFS_CVFSDIGESTHASHER_H
#endif /* GUCEF_VFS_CVFSDIGESTHASHER_H ? */

#include "TestVfsBlockCacheUtils.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static UInt32 g_folderCounter = 0;

/*-------------------------------------------------------------------------*/

CBlockCacheTestFolder::CBlockCacheTestFolder( const char* name )
    : m_path()
{GUCEF_TRACE;

    CORE::CString base = CORE::GetEnv( "GUCEF_TEST_SCRATCH_DIR" );
    if ( base.IsNULLOrEmpty() )
        base = CORE::TempDir();
    m_path = CORE::CombinePath( base, "gucefvfs-blockcache-" + CORE::ToString( MT::GetProcessID() ) + "-" + CORE::ToString( ++g_folderCounter ) + "-" + CORE::CString( name ) );
    Reset();
}

/*-------------------------------------------------------------------------*/

CBlockCacheTestFolder::~CBlockCacheTestFolder()
{GUCEF_TRACE;

    if ( m_path.HasSubstr( "gucefvfs-blockcache-" ) >= 0 )
        CORE::DeleteDir( m_path, true );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CBlockCacheTestFolder::GetPath( void ) const
{GUCEF_TRACE;

    return m_path;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CBlockCacheTestFolder::GetChildPath( const CORE::CString& relativePath ) const
{GUCEF_TRACE;

    return CORE::CombinePath( m_path, relativePath );
}

/*-------------------------------------------------------------------------*/

void
CBlockCacheTestFolder::Reset( void )
{GUCEF_TRACE;

    if ( m_path.HasSubstr( "gucefvfs-blockcache-" ) >= 0 )
    {
        if ( CORE::DirExists( m_path ) )
            CORE::DeleteDir( m_path, true );
        CORE::CreateDirs( m_path );
    }
}

/*-------------------------------------------------------------------------*/

namespace BlockCacheTest {

void
FillPattern( UInt32 seed, UInt64 offset, void* dest, UInt32 length )
{GUCEF_TRACE;

    UInt8* bytes = static_cast< UInt8* >( dest );
    for ( UInt32 i=0; i<length; ++i )
    {
        const UInt64 p = offset + i;
        bytes[ i ] = (UInt8) ( ( ( p * 2654435761ULL ) >> 7 ) ^ ( p >> 11 ) ^ ( (UInt64) seed * 131ULL ) ^ ( p >> 24 ) );
    }
}

/*-------------------------------------------------------------------------*/

bool
StoreBlocks( VFS::CVfsBlockCache& cache, VFS::CVfsBlockCache::TEntryHandle entry, UInt32 seed, UInt32 firstBlock, UInt32 count )
{GUCEF_TRACE;

    GUCEF::vector< UInt8 > block;
    for ( UInt32 b = firstBlock; b < firstBlock + count; ++b )
    {
        const UInt32 size = cache.GetBlockSize( entry, b );
        if ( 0 == size )
            return false;
        block.resize( size );
        FillPattern( seed, (UInt64) b * cache.GetBlockBytes(), &block[ 0 ], size );
        if ( !cache.StoreBlock( entry, b, &block[ 0 ], size ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
StoreAllBlocks( VFS::CVfsBlockCache& cache, VFS::CVfsBlockCache::TEntryHandle entry, UInt32 seed )
{GUCEF_TRACE;

    return StoreBlocks( cache, entry, seed, 0, cache.GetBlockCount( entry ) );
}

/*-------------------------------------------------------------------------*/

bool
BlockMatches( VFS::CVfsBlockCache& cache, VFS::CVfsBlockCache::TEntryHandle entry, UInt32 seed, UInt32 index )
{GUCEF_TRACE;

    const UInt32 size = cache.GetBlockSize( entry, index );
    if ( 0 == size )
        return false;
    GUCEF::vector< UInt8 > actual;
    GUCEF::vector< UInt8 > expected;
    actual.resize( size );
    expected.resize( size );
    if ( VFS::CVfsBlockCache::READ_HIT != cache.ReadBlock( entry, index, 0, &actual[ 0 ], size ) )
        return false;
    FillPattern( seed, (UInt64) index * cache.GetBlockBytes(), &expected[ 0 ], size );
    return 0 == memcmp( &actual[ 0 ], &expected[ 0 ], size );
}

/*-------------------------------------------------------------------------*/

VFS::CVfsBlockCache::EReadResult
ReadWholeBlock( VFS::CVfsBlockCache& cache, VFS::CVfsBlockCache::TEntryHandle entry, UInt32 index )
{GUCEF_TRACE;

    const UInt32 size = cache.GetBlockSize( entry, index );
    GUCEF::vector< UInt8 > actual;
    actual.resize( size > 0 ? size : 1 );
    return cache.ReadBlock( entry, index, 0, &actual[ 0 ], size );
}

/*-------------------------------------------------------------------------*/

VFS::CVfsDigest
PatternDigest( VFS::CVfsDigest::EAlgorithm algorithm, UInt32 seed, UInt64 fileSize )
{GUCEF_TRACE;

    VFS::CVfsDigestHasher hasher( algorithm );
    GUCEF::vector< UInt8 > piece;
    piece.resize( 40000 );
    UInt64 position = 0;
    while ( position < fileSize )
    {
        const UInt32 count = (UInt32) ( fileSize - position < piece.size() ? fileSize - position : piece.size() );
        FillPattern( seed, position, &piece[ 0 ], count );
        hasher.Update( &piece[ 0 ], count );
        position += count;
    }
    return hasher.Final();
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
ListEntryFiles( const CORE::CString& cacheFolder, const char* extension )
{GUCEF_TRACE;

    CORE::CString::StringVector paths;
    const CORE::CString ext( extension );
    CORE::CFileSystemIterator top;
    CORE::CString::StringVector folders;
    if ( top.FindFirst( cacheFolder ) )
    {
        do
        {
            if ( top.IsADirectory() && top.GetResourceName().Length() == 2 )
                folders.push_back( top.GetResourceName() );
        }
        while ( top.FindNext() );
        top.FindClose();
    }
    for ( size_t f=0; f<folders.size(); ++f )
    {
        CORE::CFileSystemIterator inner;
        const CORE::CString folder = CORE::CombinePath( cacheFolder, folders[ f ] );
        if ( inner.FindFirst( folder ) )
        {
            do
            {
                const CORE::CString name = inner.GetResourceName();
                if ( inner.IsAFile() && name.Length() > ext.Length() && name.SubstrFromRange( name.Length() - ext.Length(), name.Length() ) == ext )
                    paths.push_back( CORE::CombinePath( folder, name ) );
            }
            while ( inner.FindNext() );
            inner.FindClose();
        }
    }
    return paths;
}

/*-------------------------------------------------------------------------*/

bool
DamageByte( const CORE::CString& path, UInt64 offset )
{GUCEF_TRACE;

    CORE::CFileAccess file( path, "r+b" );
    if ( !file.Opened() || 0 != file.Setpos( offset ) )
        return false;
    UInt8 value = 0;
    if ( 1 != file.Read( &value, 1, 1 ) )
        return false;
    ++value;
    return 0 == file.Setpos( offset ) && 1 == file.Write( &value, 1, 1 );
}

/*-------------------------------------------------------------------------*/

CORE::CDynamicBuffer
ReadWholeFile( const CORE::CString& path )
{GUCEF_TRACE;

    CORE::CDynamicBuffer content;
    CORE::CFileAccess file( path, "rb" );
    if ( !file.Opened() )
        return content;
    const UInt64 size = file.GetSize();
    if ( size > 0 )
    {
        content.SetBufferSize( (UInt32) size );
        const UInt32 got = file.Read( content.GetBufferPtr(), 1, (UInt32) size );
        content.SetDataSize( got );
    }
    return content;
}

/*-------------------------------------------------------------------------*/

bool
WriteWholeFile( const CORE::CString& path, const CORE::CDynamicBuffer& content )
{GUCEF_TRACE;

    CORE::CFileAccess file( path, "wb" );
    if ( !file.Opened() )
        return false;
    if ( content.GetDataSize() > 0 && content.GetDataSize() != file.Write( content.GetConstBufferPtr(), 1, content.GetDataSize() ) )
        return false;
    file.Flush();
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CountOpenDescriptors( void )
{GUCEF_TRACE;

    UInt32 count = 0;
    #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    CORE::CFileSystemIterator iterator;
    if ( iterator.FindFirst( "/proc/self/fd" ) )
    {
        do
        {
            const CORE::CString name = iterator.GetResourceName();
            if ( name != "." && name != ".." )
                ++count;
        }
        while ( iterator.FindNext() );
        iterator.FindClose();
    }
    #endif
    return count;
}

/*-------------------------------------------------------------------------*/

bool
IsSuperUser( void )
{GUCEF_TRACE;

    #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    return 0 == geteuid();
    #else
    return false;
    #endif
}

/*-------------------------------------------------------------------------*/

bool
IsFolderSparse( const CORE::CString& folder )
{GUCEF_TRACE;

    #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    const CORE::CString probe = CORE::CombinePath( folder, "sparse_probe.bin" );
    bool sparse = false;
    {
        CORE::CFileAccess file( probe, "wb" );
        if ( file.Opened() && 0 == file.Setpos( 1073741824ULL ) && 1 == file.Write( "x", 1, 1 ) )
        {
            file.Flush();
            file.Close();
            struct stat info;
            if ( 0 == ::stat( probe.C_String(), &info ) )
                sparse = (UInt64) info.st_blocks * 512 < 16777216ULL;
        }
    }
    CORE::DeleteFile( probe );
    return sparse;
    #else
    return false;
    #endif
}

}; /* namespace BlockCacheTest */

/*-------------------------------------------------------------------------*/
