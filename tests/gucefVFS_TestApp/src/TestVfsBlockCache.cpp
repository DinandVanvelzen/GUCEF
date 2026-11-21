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
#include <stdlib.h>

#include "gucef_essentials.h"

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
  #include <sys/stat.h>
#endif

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

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

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestVfsBlockCacheUtils.h"
#include "TestVfsBlockCache.h"

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef VFS::CVfsBlockCache TCache;
typedef VFS::CVfsBlockCache::TEntryHandle THandle;
static const UInt32 B = BlockCacheTest::BlockBytes;

/*-------------------------------------------------------------------------*/

bool
OpenCache( TCache& cache, const CBlockCacheTestFolder& folder, UInt64 maxBytes = 0, UInt32 maxOpenFiles = 128 )
{
    return cache.Open( folder.GetPath(), maxBytes, B, maxOpenFiles, false );
}

/*-------------------------------------------------------------------------*/

THandle
Acquire( TCache& cache, const char* identity, const char* path, const char* revision, UInt64 size, bool create = true )
{
    return cache.AcquireEntry( identity, path, revision, size, VFS::CVfsDigest(), create );
}

/*-------------------------------------------------------------------------*/

bool
LockFileExists( const CBlockCacheTestFolder& folder )
{
    return CORE::FileExists( folder.GetChildPath( "cache.lock" ) );
}

/*-------------------------------------------------------------------------*/

TCache::SStatistics
GetStats( const TCache& cache )
{
    TCache::SStatistics statistics;
    cache.GetStatistics( statistics );
    return statistics;
}

/*-------------------------------------------------------------------------*/

/** Whether a file of the same content is still there: the files of other formats in the cache folder are never touched */
bool
FileHasContent( const CORE::CString& path, const char* content )
{
    const CORE::CDynamicBuffer actual = BlockCacheTest::ReadWholeFile( path );
    return actual.GetDataSize() == strlen( content ) && 0 == memcmp( actual.GetConstBufferPtr(), content, actual.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

bool
CopyCacheFolder( const CORE::CString& from, const CORE::CString& to )
{
    CORE::CreateDirs( to );
    bool good = CORE::CopyFile( CORE::CombinePath( from, "cache.lock" ), CORE::CombinePath( to, "cache.lock" ), true );
    const char* extensions[ 2 ] = { ".vbi", ".vbd" };
    for ( UInt32 e=0; e<2; ++e )
    {
        CORE::CString::StringVector files = BlockCacheTest::ListEntryFiles( from, extensions[ e ] );
        for ( size_t f=0; f<files.size(); ++f )
        {
            const CORE::CString name = CORE::ExtractFilename( files[ f ] );
            const CORE::CString fan = name.SubstrFromRange( 0, 2 );
            CORE::CreateDirs( CORE::CombinePath( to, fan ) );
            good = CORE::CopyFile( files[ f ], CORE::CombinePath( CORE::CombinePath( to, fan ), name ), true ) && good;
        }
    }
    return good;
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformVfsBlockCacheTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "VfsBlockCache" );

    GUCEF_TESTFW_TESTCASE( "BlockCache: Open makes the folder, takes cache.lock, refuses a bad block size, Close removes the lock; the files stay" )
    try
    {
        CBlockCacheTestFolder folder( "open" );
        const CORE::CString path = folder.GetChildPath( "cache" );
        CBlockCacheTestFolder lockHolder( "openlock" );
        TCache cache;
        ASSERT_FALSE( cache.IsOpen() );
        ASSERT_FALSE( cache.Open( CORE::CString::Empty, 0 ) );
        ASSERT_FALSE( cache.Open( path, 0, 1000 ) );
        ASSERT_FALSE( cache.Open( path, 0, 3 * B ) );
        ASSERT_FALSE( cache.Open( path, 0, 8388608 ) );
        ASSERT_FALSE( cache.Open( path, 0, 32768 ) );
        ASSERT_FALSE( CORE::DirExists( path ) );

        ASSERT_TRUE( cache.Open( path, 123456789, B ) );
        ASSERT_TRUE( cache.IsOpen() );
        ASSERT_TRUE( CORE::DirExists( path ) );
        ASSERT_TRUE( CORE::FileExists( CORE::CombinePath( path, "cache.lock" ) ) );
        ASSERT_TRUE( B == cache.GetBlockBytes() );
        ASSERT_TRUE( 123456789 == cache.GetMaxBytes() );
        ASSERT_TRUE( 0 == cache.GetUsedBytes() );
        ASSERT_FALSE( cache.Open( path, 0, B ) );                    // not twice

        const THandle h = Acquire( cache, "share", "a/b.bin", "r1", 3 * B );
        ASSERT_TRUE( 0 != h );
        ASSERT_TRUE( BlockCacheTest::StoreBlocks( cache, h, 1, 0, 1 ) );
        cache.ReleaseEntry( h );
        cache.Close();
        ASSERT_FALSE( cache.IsOpen() );
        ASSERT_FALSE( CORE::FileExists( CORE::CombinePath( path, "cache.lock" ) ) );
        ASSERT_TRUE( 1 == BlockCacheTest::ListEntryFiles( path, ".vbi" ).size() && 1 == BlockCacheTest::ListEntryFiles( path, ".vbd" ).size() );

        // the sizes the cache takes
        TCache second;
        ASSERT_TRUE( second.Open( path, 0, 131072 ) );
        ASSERT_TRUE( 131072 == second.GetBlockBytes() );
        second.Close();
        ASSERT_TRUE( second.Open( path, 0, 4194304 ) );
        second.Close();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockCache: the key is 32 lower case hexadecimal characters of the identity, the path and the revision, and the files are <xx>/<key>.vbi and .vbd" )
    try
    {
        const CORE::CString key = TCache::MakeKey( "share", "a/b.bin", "r1" );
        ASSERT_TRUE( 32 == key.Length() );
        ASSERT_TRUE( key == key.Lowercase() );
        for ( UInt32 i=0; i<32; ++i )
            ASSERT_TRUE( ( key[ i ] >= '0' && key[ i ] <= '9' ) || ( key[ i ] >= 'a' && key[ i ] <= 'f' ) );
        ASSERT_TRUE( key == TCache::MakeKey( "share", "a/b.bin", "r1" ) );
        ASSERT_TRUE( key != TCache::MakeKey( "share2", "a/b.bin", "r1" ) );
        ASSERT_TRUE( key != TCache::MakeKey( "share", "a/b2.bin", "r1" ) );
        ASSERT_TRUE( key != TCache::MakeKey( "share", "a/b.bin", "r2" ) );
        // the parts are separated: moving a character from one to the next is another key
        ASSERT_TRUE( TCache::MakeKey( "ab", "c", "d" ) != TCache::MakeKey( "a", "bc", "d" ) );
        ASSERT_TRUE( TCache::MakeKey( "a", "bc", "d" ) != TCache::MakeKey( "a", "b", "cd" ) );

        CBlockCacheTestFolder folder( "layout" );
        TCache cache;
        ASSERT_TRUE( OpenCache( cache, folder ) );
        const THandle h = Acquire( cache, "share", "a/b.bin", "r1", B );
        ASSERT_TRUE( 0 != h );
        ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, h, 2 ) );
        cache.ReleaseEntry( h );
        cache.Close();

        const CORE::CString fan = key.SubstrFromRange( 0, 2 );
        ASSERT_TRUE( CORE::FileExists( CORE::CombinePath( CORE::CombinePath( folder.GetPath(), fan ), key + ".vbi" ) ) );
        ASSERT_TRUE( CORE::FileExists( CORE::CombinePath( CORE::CombinePath( folder.GetPath(), fan ), key + ".vbd" ) ) );
        // the index has the header of 2 KiB and a row of 8 bytes per block, the data the bytes of the block
        ASSERT_TRUE( 2048 + 8 == CORE::FileSize( CORE::CombinePath( CORE::CombinePath( folder.GetPath(), fan ), key + ".vbi" ) ) );
        ASSERT_TRUE( B == CORE::FileSize( CORE::CombinePath( CORE::CombinePath( folder.GetPath(), fan ), key + ".vbd" ) ) );
        const CORE::CDynamicBuffer index = BlockCacheTest::ReadWholeFile( CORE::CombinePath( CORE::CombinePath( folder.GetPath(), fan ), key + ".vbi" ) );
        ASSERT_TRUE( index.GetDataSize() >= 4 && 0 == memcmp( index.GetConstBufferPtr(), "VBC1", 4 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockCache: blocks stored out of order are read back at the edges of the blocks, the last block is short, reads past a block are misses" )
    try
    {
        CBlockCacheTestFolder folder( "blocks" );
        TCache cache;
        ASSERT_TRUE( OpenCache( cache, folder ) );
        const UInt64 fileSize = 3 * (UInt64) B + 1000;
        const THandle h = Acquire( cache, "share", "f.bin", "r1", fileSize );
        ASSERT_TRUE( 0 != h );
        ASSERT_TRUE( 4 == cache.GetBlockCount( h ) );
        ASSERT_TRUE( fileSize == cache.GetFileSize( h ) );
        ASSERT_TRUE( B == cache.GetBlockSize( h, 0 ) && B == cache.GetBlockSize( h, 2 ) && 1000 == cache.GetBlockSize( h, 3 ) );
        ASSERT_TRUE( 0 == cache.GetBlockSize( h, 4 ) );

        ASSERT_TRUE( BlockCacheTest::StoreBlocks( cache, h, 7, 2, 1 ) );
        ASSERT_TRUE( BlockCacheTest::StoreBlocks( cache, h, 7, 0, 1 ) );
        ASSERT_TRUE( BlockCacheTest::StoreBlocks( cache, h, 7, 3, 1 ) );
        ASSERT_TRUE( cache.HasBlock( h, 0 ) && cache.HasBlock( h, 2 ) && cache.HasBlock( h, 3 ) && !cache.HasBlock( h, 1 ) );
        ASSERT_TRUE( 3 == cache.GetBlocksHeld( h ) );
        ASSERT_FALSE( cache.IsComplete( h ) );
        ASSERT_TRUE( 2 * (UInt64) B + 1000 == cache.GetUsedBytes() );

        UInt8 one = 0;
        UInt8 expected = 0;
        ASSERT_TRUE( TCache::READ_MISS == cache.ReadBlock( h, 1, 0, &one, 1 ) );
        ASSERT_TRUE( BlockCacheTest::StoreBlocks( cache, h, 7, 1, 1 ) );
        ASSERT_TRUE( cache.IsComplete( h ) && 4 == cache.GetBlocksHeld( h ) );
        ASSERT_TRUE( fileSize == cache.GetUsedBytes() );

        // the last byte of a block and the first byte of the next
        ASSERT_TRUE( TCache::READ_HIT == cache.ReadBlock( h, 0, B - 1, &one, 1 ) );
        BlockCacheTest::FillPattern( 7, B - 1, &expected, 1 );
        ASSERT_TRUE( one == expected );
        ASSERT_TRUE( TCache::READ_HIT == cache.ReadBlock( h, 1, 0, &one, 1 ) );
        BlockCacheTest::FillPattern( 7, B, &expected, 1 );
        ASSERT_TRUE( one == expected );
        ASSERT_TRUE( TCache::READ_HIT == cache.ReadBlock( h, 3, 999, &one, 1 ) );
        BlockCacheTest::FillPattern( 7, 3 * (UInt64) B + 999, &expected, 1 );
        ASSERT_TRUE( one == expected );

        // a range inside a block
        GUCEF::vector< UInt8 > got;
        GUCEF::vector< UInt8 > want;
        got.resize( 5000 );
        want.resize( 5000 );
        ASSERT_TRUE( TCache::READ_HIT == cache.ReadBlock( h, 2, 100, &got[ 0 ], 5000 ) );
        BlockCacheTest::FillPattern( 7, 2 * (UInt64) B + 100, &want[ 0 ], 5000 );
        ASSERT_TRUE( 0 == memcmp( &got[ 0 ], &want[ 0 ], 5000 ) );

        // not a read of the cache: past the end of the block, past the end of the file, a block that does not exist
        ASSERT_TRUE( TCache::READ_MISS == cache.ReadBlock( h, 3, 1000, &one, 1 ) );
        ASSERT_TRUE( TCache::READ_MISS == cache.ReadBlock( h, 3, 0, &got[ 0 ], 1001 ) );
        ASSERT_TRUE( TCache::READ_MISS == cache.ReadBlock( h, 0, B - 1, &got[ 0 ], 2 ) );
        ASSERT_TRUE( TCache::READ_MISS == cache.ReadBlock( h, 4, 0, &one, 1 ) );
        ASSERT_TRUE( TCache::READ_MISS == cache.ReadBlock( 12345, 0, 0, &one, 1 ) );

        // not a store of the cache: another length than the block has, a block that does not exist
        GUCEF::vector< UInt8 > block;
        block.resize( B );
        ASSERT_FALSE( cache.StoreBlock( h, 0, &block[ 0 ], B - 1 ) );
        ASSERT_FALSE( cache.StoreBlock( h, 3, &block[ 0 ], B ) );
        ASSERT_FALSE( cache.StoreBlock( h, 4, &block[ 0 ], 1000 ) );
        ASSERT_FALSE( cache.StoreBlock( h, 0, GUCEF_NULL, B ) );
        ASSERT_FALSE( cache.StoreBlock( 12345, 0, &block[ 0 ], B ) );

        // the same block again is no new bytes
        ASSERT_TRUE( BlockCacheTest::StoreBlocks( cache, h, 7, 0, 1 ) );
        ASSERT_TRUE( fileSize == cache.GetUsedBytes() && 4 == cache.GetBlocksHeld( h ) );
        cache.ReleaseEntry( h );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockCache: the number of blocks and the size of the last one are right at the edges of a file size (1 byte, a block, a block and one byte, two blocks)" )
    try
    {
        CBlockCacheTestFolder folder( "sizes" );
        TCache cache;
        ASSERT_TRUE( OpenCache( cache, folder ) );
        const UInt64 sizes[] = { 1, B - 1, B, B + 1, 2 * (UInt64) B - 1, 2 * (UInt64) B, 2 * (UInt64) B + 1 };
        const UInt32 counts[] = { 1, 1, 1, 2, 2, 2, 3 };
        const UInt32 lasts[] = { 1, B - 1, B, 1, B - 1, B, 1 };
        for ( UInt32 s=0; s<sizeof( sizes ) / sizeof( sizes[ 0 ] ); ++s )
        {
            const THandle h = cache.AcquireEntry( "share", "size", CORE::ToString( s ), sizes[ s ], VFS::CVfsDigest(), true );
            ASSERT_TRUE( 0 != h );
            ASSERT_TRUE( counts[ s ] == cache.GetBlockCount( h ) );
            ASSERT_TRUE( lasts[ s ] == cache.GetBlockSize( h, counts[ s ] - 1 ) );
            ASSERT_TRUE( 0 == cache.GetBlockSize( h, counts[ s ] ) );
            ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, h, 3 + s ) );
            ASSERT_TRUE( cache.IsComplete( h ) );
            for ( UInt32 b=0; b<counts[ s ]; ++b )
                ASSERT_TRUE( BlockCacheTest::BlockMatches( cache, h, 3 + s, b ) );
            cache.ReleaseEntry( h );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockCache: what was stored is there after a restart, the held blocks, the used bytes and the bytes; an interrupted fetch continues" )
    try
    {
        CBlockCacheTestFolder folder( "persist" );
        const UInt64 fileSize = 3 * (UInt64) B + 1000;
        {
            TCache cache;
            ASSERT_TRUE( OpenCache( cache, folder ) );
            const THandle h = Acquire( cache, "share", "dir/f.bin", "r1", fileSize );
            ASSERT_TRUE( BlockCacheTest::StoreBlocks( cache, h, 5, 0, 1 ) );
            ASSERT_TRUE( BlockCacheTest::StoreBlocks( cache, h, 5, 2, 1 ) );
            cache.ReleaseEntry( h );
        }
        {
            TCache cache;
            ASSERT_TRUE( OpenCache( cache, folder ) );
            ASSERT_TRUE( 2 * (UInt64) B == cache.GetUsedBytes() );
            ASSERT_TRUE( 1 == GetStats( cache ).entries );
            const THandle h = Acquire( cache, "share", "dir/f.bin", "r1", fileSize, false );
            ASSERT_TRUE( 0 != h );
            ASSERT_TRUE( cache.HasBlock( h, 0 ) && cache.HasBlock( h, 2 ) && !cache.HasBlock( h, 1 ) && !cache.HasBlock( h, 3 ) );
            ASSERT_TRUE( 2 == cache.GetBlocksHeld( h ) );
            ASSERT_TRUE( BlockCacheTest::BlockMatches( cache, h, 5, 0 ) && BlockCacheTest::BlockMatches( cache, h, 5, 2 ) );
            ASSERT_TRUE( BlockCacheTest::StoreBlocks( cache, h, 5, 1, 1 ) );
            ASSERT_TRUE( BlockCacheTest::StoreBlocks( cache, h, 5, 3, 1 ) );
            ASSERT_TRUE( cache.IsComplete( h ) );
            cache.ReleaseEntry( h );
        }
        {
            TCache cache;
            ASSERT_TRUE( OpenCache( cache, folder ) );
            ASSERT_TRUE( fileSize == cache.GetUsedBytes() );
            const THandle h = Acquire( cache, "share", "dir/f.bin", "r1", fileSize, false );
            ASSERT_TRUE( 0 != h && cache.IsComplete( h ) );
            for ( UInt32 b=0; b<4; ++b )
                ASSERT_TRUE( BlockCacheTest::BlockMatches( cache, h, 5, b ) );
            TCache::SEntryInfo info;
            ASSERT_TRUE( cache.GetEntryInfo( h, info ) );
            ASSERT_TRUE( info.identity == "share" && info.path == "dir/f.bin" && info.revision == "r1" && info.fileSize == fileSize && info.complete && info.bytesHeld == fileSize );
            cache.ReleaseEntry( h );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockCache: a block that the disk gave back damaged is found by its checksum at its first read after a restart, dropped, and stored again" )
    try
    {
        CBlockCacheTestFolder folder( "crc" );
        const UInt64 fileSize = 3 * (UInt64) B;
        {
            TCache cache;
            ASSERT_TRUE( OpenCache( cache, folder ) );
            const THandle h = Acquire( cache, "share", "f.bin", "r1", fileSize );
            ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, h, 6 ) );
            cache.ReleaseEntry( h );
        }
        const CORE::CString::StringVector data = BlockCacheTest::ListEntryFiles( folder.GetPath(), ".vbd" );
        ASSERT_TRUE( 1 == data.size() );
        ASSERT_TRUE( BlockCacheTest::DamageByte( data[ 0 ], B + 5 ) );                // in block 1
        ASSERT_TRUE( BlockCacheTest::DamageByte( data[ 0 ], 3 * (UInt64) B - 1 ) );   // the last byte of block 2
        {
            TCache cache;
            ASSERT_TRUE( OpenCache( cache, folder ) );
            const THandle h = Acquire( cache, "share", "f.bin", "r1", fileSize, false );
            ASSERT_TRUE( 0 != h );
            ASSERT_TRUE( BlockCacheTest::BlockMatches( cache, h, 6, 0 ) );
            // a read of a few bytes that are not the damaged ones is a read of a damaged block
            UInt8 one = 0;
            ASSERT_TRUE( TCache::READ_CORRUPT == cache.ReadBlock( h, 1, 100, &one, 1 ) );
            ASSERT_TRUE( TCache::READ_MISS == cache.ReadBlock( h, 1, 100, &one, 1 ) );
            ASSERT_FALSE( cache.HasBlock( h, 1 ) );
            ASSERT_TRUE( TCache::READ_CORRUPT == BlockCacheTest::ReadWholeBlock( cache, h, 2 ) );
            ASSERT_TRUE( 1 == cache.GetBlocksHeld( h ) );
            ASSERT_TRUE( B == cache.GetUsedBytes() );
            ASSERT_TRUE( 2 == GetStats( cache ).corruptBlocks );

            ASSERT_TRUE( BlockCacheTest::StoreBlocks( cache, h, 6, 1, 2 ) );
            ASSERT_TRUE( cache.IsComplete( h ) && 3 * (UInt64) B == cache.GetUsedBytes() );
            for ( UInt32 b=0; b<3; ++b )
                ASSERT_TRUE( BlockCacheTest::BlockMatches( cache, h, 6, b ) );
            cache.ReleaseEntry( h );
        }
        // and the repair is in the files
        {
            TCache cache;
            ASSERT_TRUE( OpenCache( cache, folder ) );
            const THandle h = Acquire( cache, "share", "f.bin", "r1", fileSize, false );
            ASSERT_TRUE( 0 != h && cache.IsComplete( h ) );
            for ( UInt32 b=0; b<3; ++b )
                ASSERT_TRUE( BlockCacheTest::BlockMatches( cache, h, 6, b ) );
            ASSERT_TRUE( 0 == GetStats( cache ).corruptBlocks );
            cache.ReleaseEntry( h );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockCache: revisions of a path are separate entries that coexist, an old reader keeps reading while the new one is filled, no byte of one is read from the other" )
    try
    {
        CBlockCacheTestFolder folder( "revisions" );
        TCache cache;
        ASSERT_TRUE( OpenCache( cache, folder ) );
        const THandle old = Acquire( cache, "share", "f.bin", "r1", 2 * (UInt64) B );
        ASSERT_TRUE( 0 != old );
        ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, old, 10 ) );

        // the content changed: another entry, with the old one still held
        const THandle current = Acquire( cache, "share", "f.bin", "r2", 2 * (UInt64) B );
        ASSERT_TRUE( 0 != current && current != old );
        ASSERT_FALSE( cache.HasBlock( current, 0 ) );
        ASSERT_TRUE( BlockCacheTest::BlockMatches( cache, old, 10, 0 ) );
        ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, current, 11 ) );
        ASSERT_TRUE( BlockCacheTest::BlockMatches( cache, old, 10, 0 ) && BlockCacheTest::BlockMatches( cache, old, 10, 1 ) );
        ASSERT_TRUE( BlockCacheTest::BlockMatches( cache, current, 11, 0 ) && BlockCacheTest::BlockMatches( cache, current, 11, 1 ) );
        ASSERT_FALSE( BlockCacheTest::BlockMatches( cache, current, 10, 0 ) );
        ASSERT_TRUE( 4 * (UInt64) B == cache.GetUsedBytes() );

        TCache::SEntryInfo info;
        ASSERT_TRUE( cache.FindNewestRevision( "share", "f.bin", info ) );
        ASSERT_TRUE( info.revision == "r2" );
        ASSERT_FALSE( info.superseded );

        // the archive learns that r2 is the current one
        ASSERT_TRUE( 1 == cache.SupersedeOtherRevisions( "share", "f.bin", "r2" ) );
        ASSERT_TRUE( 0 == cache.SupersedeOtherRevisions( "share", "f.bin", "r2" ) );
        ASSERT_TRUE( cache.GetEntryInfo( old, info ) && info.superseded );
        ASSERT_TRUE( cache.GetEntryInfo( current, info ) && !info.superseded );
        ASSERT_TRUE( BlockCacheTest::BlockMatches( cache, old, 10, 1 ) );        // the open reader of the old revision is not cut off
        ASSERT_TRUE( cache.FindNewestRevision( "share", "f.bin", info ) && info.revision == "r2" );
        cache.ReleaseEntry( old );
        cache.ReleaseEntry( current );
        cache.Close();

        // the flags are in the index
        ASSERT_TRUE( OpenCache( cache, folder ) );
        const THandle oldAgain = Acquire( cache, "share", "f.bin", "r1", 2 * (UInt64) B, false );
        const THandle currentAgain = Acquire( cache, "share", "f.bin", "r2", 2 * (UInt64) B, false );
        ASSERT_TRUE( 0 != oldAgain && 0 != currentAgain );
        ASSERT_TRUE( cache.GetEntryInfo( oldAgain, info ) && info.superseded );
        ASSERT_TRUE( cache.GetEntryInfo( currentAgain, info ) && !info.superseded );
        ASSERT_TRUE( cache.FindNewestRevision( "share", "f.bin", info ) && info.revision == "r2" );

        // r1 is the current one after all: it is not superseded any more and r2 is
        ASSERT_TRUE( 1 == cache.SupersedeOtherRevisions( "share", "f.bin", "r1" ) );
        ASSERT_TRUE( cache.GetEntryInfo( oldAgain, info ) && !info.superseded );
        ASSERT_TRUE( cache.GetEntryInfo( currentAgain, info ) && info.superseded );
        ASSERT_TRUE( cache.FindNewestRevision( "share", "f.bin", info ) && info.revision == "r1" );
        ASSERT_TRUE( 0 == cache.SupersedeOtherRevisions( "share", "nosuchpath", "r1" ) );
        cache.ReleaseEntry( oldAgain );
        cache.ReleaseEntry( currentAgain );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockCache: two identities with one path and one revision do not share an entry" )
    try
    {
        CBlockCacheTestFolder folder( "identities" );
        TCache cache;
        ASSERT_TRUE( OpenCache( cache, folder ) );
        const THandle a = Acquire( cache, "shareA", "f.bin", "r1", 2 * (UInt64) B );
        const THandle b = Acquire( cache, "shareB", "f.bin", "r1", 2 * (UInt64) B );
        ASSERT_TRUE( 0 != a && 0 != b && a != b );
        ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, a, 20 ) );
        ASSERT_FALSE( cache.HasBlock( b, 0 ) );
        ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, b, 21 ) );
        ASSERT_TRUE( BlockCacheTest::BlockMatches( cache, a, 20, 1 ) && BlockCacheTest::BlockMatches( cache, b, 21, 1 ) );
        ASSERT_FALSE( BlockCacheTest::BlockMatches( cache, a, 21, 1 ) );
        TCache::SEntryInfo info;
        ASSERT_TRUE( cache.FindNewestRevision( "shareA", "f.bin", info ) && info.identity == "shareA" );
        ASSERT_TRUE( cache.FindNewestRevision( "shareB", "f.bin", info ) && info.identity == "shareB" );
        ASSERT_FALSE( cache.FindNewestRevision( "shareC", "f.bin", info ) );
        ASSERT_TRUE( 2 == BlockCacheTest::ListEntryFiles( folder.GetPath(), ".vbi" ).size() );
        cache.ReleaseEntry( a );
        cache.ReleaseEntry( b );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockCache: the size cap removes the entries that were used longest ago, whole, and what is used stays" )
    try
    {
        CBlockCacheTestFolder folder( "lru" );
        TCache cache;
        ASSERT_TRUE( OpenCache( cache, folder, 6 * (UInt64) B ) );
        const char* names[] = { "a", "b", "c" };
        for ( UInt32 n=0; n<3; ++n )
        {
            const THandle h = Acquire( cache, "share", names[ n ], "r1", 2 * (UInt64) B );
            ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, h, 30 + n ) );
            cache.ReleaseEntry( h );
        }
        ASSERT_TRUE( 6 * (UInt64) B == cache.GetUsedBytes() );
        ASSERT_TRUE( 0 == GetStats( cache ).evictedEntries );

        // a is used again: b is the one used longest ago
        {
            const THandle h = Acquire( cache, "share", "a", "r1", 2 * (UInt64) B, false );
            ASSERT_TRUE( 0 != h && BlockCacheTest::BlockMatches( cache, h, 30, 0 ) );
            cache.ReleaseEntry( h );
        }
        const THandle d = Acquire( cache, "share", "d", "r1", 2 * (UInt64) B );
        ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, d, 33 ) );
        cache.ReleaseEntry( d );
        ASSERT_TRUE( 6 * (UInt64) B == cache.GetUsedBytes() );
        ASSERT_TRUE( 1 == GetStats( cache ).evictedEntries && 2 * (UInt64) B == GetStats( cache ).evictedBytes );
        ASSERT_TRUE( 0 == Acquire( cache, "share", "b", "r1", 2 * (UInt64) B, false ) );
        ASSERT_TRUE( 3 == BlockCacheTest::ListEntryFiles( folder.GetPath(), ".vbi" ).size() && 3 == BlockCacheTest::ListEntryFiles( folder.GetPath(), ".vbd" ).size() );

        // then c is the one
        const THandle e = Acquire( cache, "share", "e", "r1", 2 * (UInt64) B );
        ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, e, 34 ) );
        cache.ReleaseEntry( e );
        ASSERT_TRUE( 0 == Acquire( cache, "share", "c", "r1", 2 * (UInt64) B, false ) );
        const THandle a = Acquire( cache, "share", "a", "r1", 2 * (UInt64) B, false );
        ASSERT_TRUE( 0 != a && BlockCacheTest::BlockMatches( cache, a, 30, 1 ) );
        cache.ReleaseEntry( a );
        ASSERT_TRUE( cache.GetUsedBytes() <= 6 * (UInt64) B );

        // a smaller limit removes at once; no limit removes nothing more
        cache.SetMaxBytes( 2 * (UInt64) B );
        ASSERT_TRUE( cache.GetUsedBytes() <= 2 * (UInt64) B );
        const UInt64 left = cache.GetUsedBytes();
        cache.SetMaxBytes( 0 );
        ASSERT_TRUE( left == cache.GetUsedBytes() && 0 == cache.GetMaxBytes() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockCache: a pinned entry stays when the cap removes everything else, an acquired one too; the cap is then passed" )
    try
    {
        CBlockCacheTestFolder folder( "pins" );
        TCache cache;
        ASSERT_TRUE( OpenCache( cache, folder, 4 * (UInt64) B ) );
        ASSERT_FALSE( cache.SetPinned( "share", "p", true ) );                 // a pin needs an entry
        const THandle p = Acquire( cache, "share", "p", "r1", 2 * (UInt64) B );
        ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, p, 40 ) );
        cache.ReleaseEntry( p );
        ASSERT_FALSE( cache.IsPinned( "share", "p" ) );
        ASSERT_TRUE( cache.SetPinned( "share", "p", true ) && cache.IsPinned( "share", "p" ) );

        const char* names[] = { "x1", "x2", "x3", "x4" };
        for ( UInt32 n=0; n<4; ++n )
        {
            const THandle h = Acquire( cache, "share", names[ n ], "r1", 2 * (UInt64) B );
            ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, h, 41 + n ) );
            cache.ReleaseEntry( h );
        }
        ASSERT_TRUE( cache.GetUsedBytes() <= 4 * (UInt64) B );
        const THandle again = Acquire( cache, "share", "p", "r1", 2 * (UInt64) B, false );
        ASSERT_TRUE( 0 != again && BlockCacheTest::BlockMatches( cache, again, 40, 0 ) && BlockCacheTest::BlockMatches( cache, again, 40, 1 ) );
        cache.ReleaseEntry( again );
        ASSERT_TRUE( 1 == GetStats( cache ).pinnedEntries && 2 * (UInt64) B == GetStats( cache ).pinnedBytes );

        // the pin is in the index
        cache.Close();
        ASSERT_TRUE( OpenCache( cache, folder, 4 * (UInt64) B ) );
        ASSERT_TRUE( cache.IsPinned( "share", "p" ) );

        // everything pinned or acquired: the cap is passed, nothing is removed
        const THandle q = Acquire( cache, "share", "q", "r1", 4 * (UInt64) B );
        ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, q, 50 ) );
        ASSERT_TRUE( cache.GetUsedBytes() > 4 * (UInt64) B );
        ASSERT_TRUE( BlockCacheTest::BlockMatches( cache, q, 50, 3 ) );
        cache.ReleaseEntry( q );
        ASSERT_TRUE( cache.GetUsedBytes() <= 4 * (UInt64) B + 2 * (UInt64) B );
        const THandle pAgain = Acquire( cache, "share", "p", "r1", 2 * (UInt64) B, false );
        ASSERT_TRUE( 0 != pAgain );
        cache.ReleaseEntry( pAgain );

        // unpinned it can go
        ASSERT_TRUE( cache.SetPinned( "share", "p", false ) );
        ASSERT_FALSE( cache.IsPinned( "share", "p" ) );
        cache.SetMaxBytes( 1 );
        ASSERT_TRUE( 0 == cache.GetUsedBytes() );
        ASSERT_TRUE( 0 == Acquire( cache, "share", "p", "r1", 2 * (UInt64) B, false ) );
        ASSERT_FALSE( cache.SetPinned( "share", "p", false ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockCache: an offline lookup serves the newest revision when it is complete, an older complete revision only when it is pinned, never an unpinned one, and the choice survives a reopen" )
    try
    {
        CBlockCacheTestFolder folder( "offlinerevision" );
        TCache cache;
        ASSERT_TRUE( OpenCache( cache, folder ) );

        // a kept path: r1 is complete and pinned, r2 is the current revision and holds one block of two
        const THandle keptOld = Acquire( cache, "share", "kept", "r1", 2 * (UInt64) B );
        ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, keptOld, 70 ) );
        ASSERT_TRUE( cache.SetPinned( "share", "kept", true ) );
        const THandle keptNew = Acquire( cache, "share", "kept", "r2", 2 * (UInt64) B );
        ASSERT_TRUE( BlockCacheTest::StoreBlocks( cache, keptNew, 71, 0, 1 ) );
        ASSERT_TRUE( 1 == cache.SupersedeOtherRevisions( "share", "kept", "r2" ) );

        // a path that is not kept: the same shape, nothing pinned
        const THandle looseOld = Acquire( cache, "share", "loose", "r1", 2 * (UInt64) B );
        ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, looseOld, 72 ) );
        const THandle looseNew = Acquire( cache, "share", "loose", "r2", 2 * (UInt64) B );
        ASSERT_TRUE( BlockCacheTest::StoreBlocks( cache, looseNew, 73, 0, 1 ) );
        ASSERT_TRUE( 1 == cache.SupersedeOtherRevisions( "share", "loose", "r2" ) );

        // a kept path whose older copy is only partly held does not stand in for anything
        const THandle partOld = Acquire( cache, "share", "partial", "r1", 2 * (UInt64) B );
        ASSERT_TRUE( BlockCacheTest::StoreBlocks( cache, partOld, 75, 0, 1 ) );
        ASSERT_TRUE( cache.SetPinned( "share", "partial", true ) );
        const THandle partNew = Acquire( cache, "share", "partial", "r2", 2 * (UInt64) B );
        ASSERT_TRUE( BlockCacheTest::StoreBlocks( cache, partNew, 76, 0, 1 ) );
        ASSERT_TRUE( 1 == cache.SupersedeOtherRevisions( "share", "partial", "r2" ) );

        // a path that is not kept, whose only copy is superseded
        const THandle goneOld = Acquire( cache, "share", "gone", "r1", 2 * (UInt64) B );
        ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, goneOld, 74 ) );
        ASSERT_TRUE( 1 == cache.SupersedeOtherRevisions( "share", "gone", "r2" ) );

        cache.ReleaseEntry( keptOld );
        cache.ReleaseEntry( keptNew );
        cache.ReleaseEntry( looseOld );
        cache.ReleaseEntry( looseNew );
        cache.ReleaseEntry( goneOld );
        cache.ReleaseEntry( partOld );
        cache.ReleaseEntry( partNew );

        for ( UInt32 round = 0; round < 2; ++round )
        {
            TCache::SEntryInfo info;

            // the lookup of the current revision is as it was: the newest revision that is not superseded, complete or not
            ASSERT_TRUE( cache.FindNewestRevision( "share", "kept", info ) && "r2" == info.revision && !info.complete );

            // offline: the kept path answers with its older complete copy, flagged
            ASSERT_TRUE( cache.FindOfflineRevision( "share", "kept", info ) );
            ASSERT_TRUE( "r1" == info.revision && info.complete && info.pinned && info.superseded && info.staleRevision );

            // the path that is not kept answers with what the current revision holds, and never with the superseded copy
            ASSERT_TRUE( cache.FindOfflineRevision( "share", "loose", info ) );
            ASSERT_TRUE( "r2" == info.revision && !info.complete && !info.staleRevision );
            ASSERT_FALSE( cache.FindOfflineRevision( "share", "gone", info ) );
            ASSERT_TRUE( cache.FindOfflineRevision( "share", "partial", info ) && "r2" == info.revision && !info.staleRevision );
            ASSERT_FALSE( cache.FindOfflineRevision( "share", "nothing", info ) );

            // the offline listing is made the same way
            TCache::TEntryInfoVector infos;
            cache.GetEntryInfosBelow( "share", "", true, infos );
            ASSERT_TRUE( 3 == infos.size() );
            for ( size_t i=0; i<infos.size(); ++i )
            {
                if ( "kept" == infos[ i ].path )
                    ASSERT_TRUE( "r1" == infos[ i ].revision && infos[ i ].staleRevision );
                else
                    ASSERT_TRUE( ( "loose" == infos[ i ].path || "partial" == infos[ i ].path ) && "r2" == infos[ i ].revision && !infos[ i ].staleRevision );
            }

            // the pinned copy that a newer revision superseded is what a refetch is for
            ASSERT_TRUE( cache.HasStalePinnedCopy( "share", "kept" ) );
            ASSERT_FALSE( cache.HasStalePinnedCopy( "share", "loose" ) );
            ASSERT_FALSE( cache.HasStalePinnedCopy( "other", "kept" ) );
            CORE::CString::StringVector stalePaths;
            cache.GetStalePinnedPaths( "share", stalePaths );
            ASSERT_TRUE( 2 == stalePaths.size() && "kept" == stalePaths[ 0 ] && "partial" == stalePaths[ 1 ] );
            stalePaths.clear();
            cache.GetStalePinnedPaths( "other", stalePaths );
            ASSERT_TRUE( stalePaths.empty() );

            if ( 0 == round )
            {
                cache.Close();
                ASSERT_TRUE( OpenCache( cache, folder ) );
            }
        }

        // the current revision becomes complete: it is what is served, not flagged, and the pin moved to it
        const THandle current = Acquire( cache, "share", "kept", "r2", 2 * (UInt64) B, false );
        ASSERT_TRUE( 0 != current && BlockCacheTest::StoreBlocks( cache, current, 71, 1, 1 ) && cache.IsComplete( current ) );
        cache.ReleaseEntry( current );
        TCache::SEntryInfo info;
        ASSERT_TRUE( cache.FindOfflineRevision( "share", "kept", info ) && "r2" == info.revision && info.complete && info.pinned && !info.staleRevision );
        ASSERT_FALSE( cache.HasStalePinnedCopy( "share", "kept" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockCache: pins against the size limit: a pin that does not fit is refused with its own result and nothing is dropped, and the pins that are there stay when the limit is lowered, also after a reopen" )
    try
    {
        CBlockCacheTestFolder folder( "pinlimit" );
        TCache cache;
        ASSERT_TRUE( OpenCache( cache, folder, 3 * (UInt64) B ) );

        const THandle a = Acquire( cache, "share", "a", "r1", 2 * (UInt64) B );
        ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, a, 80 ) );
        ASSERT_TRUE( TCache::PIN_OK == cache.TryPin( "share", "a" ) );
        ASSERT_TRUE( TCache::PIN_OK == cache.TryPin( "share", "a" ) );              // pinned already: not counted twice
        ASSERT_TRUE( 2 * (UInt64) B == GetStats( cache ).pinnedReservedBytes && 0 == GetStats( cache ).pinsRefused );

        // a second kept file would pass the limit: refused, the file is a cache entry like any other, the first pin is untouched
        const THandle b = Acquire( cache, "share", "b", "r1", 2 * (UInt64) B );
        ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, b, 81 ) );
        ASSERT_TRUE( TCache::PIN_OVER_LIMIT == cache.TryPin( "share", "b" ) );
        ASSERT_FALSE( cache.SetPinned( "share", "b", true ) );
        ASSERT_FALSE( cache.IsPinned( "share", "b" ) );
        ASSERT_TRUE( cache.IsPinned( "share", "a" ) );
        ASSERT_TRUE( cache.IsComplete( a ) && cache.IsComplete( b ) );
        ASSERT_TRUE( 2 == GetStats( cache ).pinsRefused && 2 * (UInt64) B == GetStats( cache ).pinnedReservedBytes );

        // a file that is larger than the whole limit cannot be kept at all; a path that has no entry has nothing to keep
        const THandle huge = Acquire( cache, "share", "huge", "r1", 4 * (UInt64) B );
        ASSERT_TRUE( BlockCacheTest::StoreBlocks( cache, huge, 82, 0, 1 ) );
        ASSERT_TRUE( TCache::PIN_OVER_LIMIT == cache.TryPin( "share", "huge" ) );
        ASSERT_TRUE( TCache::PIN_NO_ENTRY == cache.TryPin( "share", "nothing" ) );
        ASSERT_TRUE( 3 == GetStats( cache ).pinsRefused );
        cache.ReleaseEntry( huge );

        // room is made by letting a pin go: the refused file can be kept now, and the other one does not fit next to it
        ASSERT_TRUE( cache.SetPinned( "share", "a", false ) );
        ASSERT_TRUE( 0 == GetStats( cache ).pinnedReservedBytes );
        ASSERT_TRUE( cache.SetPinned( "share", "b", true ) );
        ASSERT_FALSE( cache.SetPinned( "share", "a", true ) );
        ASSERT_TRUE( 2 * (UInt64) B == GetStats( cache ).pinnedReservedBytes );

        // no limit: every pin is kept
        cache.SetMaxBytes( 0 );
        ASSERT_TRUE( cache.SetPinned( "share", "a", true ) );
        ASSERT_TRUE( 4 * (UInt64) B == GetStats( cache ).pinnedReservedBytes );

        // a limit that is lowered below what the pins hold: nothing pinned is dropped
        cache.SetMaxBytes( B );
        cache.ReleaseEntry( a );
        cache.ReleaseEntry( b );
        ASSERT_TRUE( cache.IsPinned( "share", "a" ) && cache.IsPinned( "share", "b" ) );
        ASSERT_TRUE( 4 * (UInt64) B == GetStats( cache ).usedBytes && 2 == GetStats( cache ).pinnedEntries );

        // the same after a reopen, and what is not kept goes first
        cache.Close();
        ASSERT_TRUE( cache.Open( folder.GetPath(), B, B, 128, false ) );
        ASSERT_TRUE( cache.IsPinned( "share", "a" ) && cache.IsPinned( "share", "b" ) );
        ASSERT_TRUE( 4 * (UInt64) B == GetStats( cache ).usedBytes );
        const THandle c = Acquire( cache, "share", "c", "r1", (UInt64) B );
        ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, c, 83 ) );
        cache.ReleaseEntry( c );
        ASSERT_TRUE( 0 == Acquire( cache, "share", "c", "r1", (UInt64) B, false ) );
        ASSERT_TRUE( 4 * (UInt64) B == GetStats( cache ).usedBytes );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockCache: under size pressure superseded copies go first, then the ones that are not pinned, and the older pinned copy of a kept path stays until its newer revision is complete" )
    try
    {
        CBlockCacheTestFolder folder( "pinoldevict" );
        TCache cache;
        ASSERT_TRUE( OpenCache( cache, folder ) );

        // the kept path: r1 is pinned and complete
        const THandle keptOld = Acquire( cache, "share", "kept", "r1", 2 * (UInt64) B );
        ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, keptOld, 90 ) );
        ASSERT_TRUE( cache.SetPinned( "share", "kept", true ) );
        cache.ReleaseEntry( keptOld );

        // a copy that is not pinned, and a superseded one, of other paths
        const THandle plain = Acquire( cache, "share", "plain", "r1", 2 * (UInt64) B );
        ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, plain, 91 ) );
        cache.ReleaseEntry( plain );
        const THandle other = Acquire( cache, "share", "other", "r1", 2 * (UInt64) B );
        ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, other, 92 ) );
        ASSERT_TRUE( 1 == cache.SupersedeOtherRevisions( "share", "other", "r2" ) );
        cache.ReleaseEntry( other );
        ASSERT_TRUE( 6 * (UInt64) B == cache.GetUsedBytes() );

        // room for one file and a half: the superseded copy goes first, then the one that is not pinned; the pinned copy stays
        cache.SetMaxBytes( 3 * (UInt64) B );
        const TCache::SStatistics stats = GetStats( cache );
        ASSERT_TRUE( 2 * (UInt64) B == stats.usedBytes && 2 == stats.evictedEntries && 1 == stats.supersededEvictions );
        ASSERT_TRUE( 0 == Acquire( cache, "share", "other", "r1", 2 * (UInt64) B, false ) );
        ASSERT_TRUE( 0 == Acquire( cache, "share", "plain", "r1", 2 * (UInt64) B, false ) );

        // the current revision of the kept path is learned and is only partly here: the pinned older copy is superseded, and no pressure takes it
        const THandle current = Acquire( cache, "share", "kept", "r2", 2 * (UInt64) B );
        ASSERT_TRUE( 0 != current && BlockCacheTest::StoreBlocks( cache, current, 93, 0, 1 ) );
        ASSERT_TRUE( 1 == cache.SupersedeOtherRevisions( "share", "kept", "r2" ) );
        cache.SetMaxBytes( 1 );
        TCache::SEntryInfo info;
        ASSERT_TRUE( cache.FindOfflineRevision( "share", "kept", info ) && "r1" == info.revision && info.complete && info.pinned && info.staleRevision );
        const THandle held = Acquire( cache, "share", "kept", "r1", 2 * (UInt64) B, false );
        ASSERT_TRUE( 0 != held && BlockCacheTest::BlockMatches( cache, held, 90, 1 ) );
        cache.ReleaseEntry( held );

        // the current revision is complete: the pin moved to it, and the old copy is the first to go
        ASSERT_TRUE( BlockCacheTest::StoreBlocks( cache, current, 93, 1, 1 ) );
        cache.ReleaseEntry( current );
        ASSERT_TRUE( cache.FindOfflineRevision( "share", "kept", info ) && "r2" == info.revision && info.pinned && !info.staleRevision );
        ASSERT_TRUE( 0 == Acquire( cache, "share", "kept", "r1", 2 * (UInt64) B, false ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockCache: a kept path that moves to a newer revision is not refused by the limit: the pin of the older revision is the pin of the path, the path is counted once, and when the newer revision is complete the pin is there alone" )
    try
    {
        CBlockCacheTestFolder folder( "pinmove" );
        TCache cache;
        ASSERT_TRUE( OpenCache( cache, folder, 3 * (UInt64) B ) );
        const THandle oldRev = Acquire( cache, "share", "m", "r1", 2 * (UInt64) B );
        ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, oldRev, 95 ) );
        ASSERT_TRUE( TCache::PIN_OK == cache.TryPin( "share", "m" ) );
        const THandle newRev = Acquire( cache, "share", "m", "r2", 2 * (UInt64) B );
        ASSERT_TRUE( BlockCacheTest::StoreBlocks( cache, newRev, 96, 0, 1 ) );
        ASSERT_TRUE( 1 == cache.SupersedeOtherRevisions( "share", "m", "r2" ) );

        // the newest revision is pinned as well while the older one is: they are one path in the books, and the limit lets it through
        ASSERT_TRUE( TCache::PIN_OK == cache.TryPin( "share", "m" ) );
        TCache::SEntryInfo info;
        ASSERT_TRUE( cache.GetEntryInfo( newRev, info ) && info.pinned );
        ASSERT_TRUE( 2 * (UInt64) B == GetStats( cache ).pinnedReservedBytes && 0 == GetStats( cache ).pinsRefused );

        // complete: the pin is on the newer revision alone, and the older one is the first to go
        ASSERT_TRUE( BlockCacheTest::StoreBlocks( cache, newRev, 96, 1, 1 ) );
        ASSERT_TRUE( cache.GetEntryInfo( oldRev, info ) && !info.pinned && info.superseded );
        ASSERT_TRUE( cache.GetEntryInfo( newRev, info ) && info.pinned && info.complete );
        ASSERT_TRUE( 2 * (UInt64) B == GetStats( cache ).pinnedReservedBytes );
        cache.ReleaseEntry( oldRev );
        cache.ReleaseEntry( newRev );
        ASSERT_TRUE( 0 == Acquire( cache, "share", "m", "r1", 2 * (UInt64) B, false ) );
        ASSERT_TRUE( 2 * (UInt64) B == cache.GetUsedBytes() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockCache: the pin of a path follows a newer revision once that one is complete, and leaves the older one unpinned and superseded" )
    try
    {
        CBlockCacheTestFolder folder( "pinfollow" );
        TCache cache;
        ASSERT_TRUE( OpenCache( cache, folder ) );
        const THandle r1 = Acquire( cache, "share", "f", "r1", 2 * (UInt64) B );
        ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, r1, 60 ) );
        ASSERT_TRUE( cache.SetPinned( "share", "f", true ) );
        TCache::SEntryInfo info;
        ASSERT_TRUE( cache.GetEntryInfo( r1, info ) && info.pinned );

        // the new revision is not complete: the pin stays where it is
        const THandle r2 = Acquire( cache, "share", "f", "r2", 2 * (UInt64) B );
        ASSERT_TRUE( 0 != r2 && r2 != r1 );
        ASSERT_TRUE( BlockCacheTest::StoreBlocks( cache, r2, 61, 0, 1 ) );
        ASSERT_TRUE( cache.GetEntryInfo( r1, info ) && info.pinned && !info.superseded );
        ASSERT_TRUE( cache.GetEntryInfo( r2, info ) && !info.pinned );

        // complete: it moves
        ASSERT_TRUE( BlockCacheTest::StoreBlocks( cache, r2, 61, 1, 1 ) );
        ASSERT_TRUE( cache.GetEntryInfo( r1, info ) && !info.pinned && info.superseded );
        ASSERT_TRUE( cache.GetEntryInfo( r2, info ) && info.pinned && !info.superseded );
        ASSERT_TRUE( cache.IsPinned( "share", "f" ) );
        cache.ReleaseEntry( r1 );
        cache.ReleaseEntry( r2 );

        // the older one is the first to go under a cap, the pinned one is not
        cache.Close();
        ASSERT_TRUE( OpenCache( cache, folder, 2 * (UInt64) B ) );
        ASSERT_TRUE( 2 * (UInt64) B == cache.GetUsedBytes() );
        ASSERT_TRUE( 0 == Acquire( cache, "share", "f", "r1", 2 * (UInt64) B, false ) );
        const THandle r2Again = Acquire( cache, "share", "f", "r2", 2 * (UInt64) B, false );
        ASSERT_TRUE( 0 != r2Again && cache.GetEntryInfo( r2Again, info ) && info.pinned && info.complete );
        ASSERT_TRUE( BlockCacheTest::BlockMatches( cache, r2Again, 61, 1 ) );
        cache.ReleaseEntry( r2Again );

        // an older revision that completes later does not take the pin from a newer one
        const THandle oldRev = Acquire( cache, "share", "g", "r1", 2 * (UInt64) B );
        const THandle newRev = Acquire( cache, "share", "g", "r2", 2 * (UInt64) B );
        ASSERT_TRUE( 0 != oldRev && 0 != newRev );
        ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, newRev, 62 ) );
        ASSERT_TRUE( BlockCacheTest::StoreBlocks( cache, oldRev, 63, 0, 1 ) );

        // the pinned f already takes the limit: a second pin is refused with its own result and nothing is dropped, then it fits a larger limit
        ASSERT_TRUE( TCache::PIN_OVER_LIMIT == cache.TryPin( "share", "g" ) );
        ASSERT_FALSE( cache.IsPinned( "share", "g" ) );
        ASSERT_TRUE( cache.IsPinned( "share", "f" ) );
        cache.SetMaxBytes( 4 * (UInt64) B );
        ASSERT_TRUE( cache.SetPinned( "share", "g", true ) );
        ASSERT_TRUE( cache.GetEntryInfo( newRev, info ) && info.pinned );
        ASSERT_TRUE( BlockCacheTest::StoreBlocks( cache, oldRev, 63, 1, 1 ) && cache.IsComplete( oldRev ) );
        ASSERT_TRUE( cache.GetEntryInfo( newRev, info ) && info.pinned && !info.superseded );
        ASSERT_TRUE( cache.GetEntryInfo( oldRev, info ) && !info.pinned && !info.superseded );
        cache.ReleaseEntry( oldRev );
        cache.ReleaseEntry( newRev );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockCache: an entry that is acquired is not removed by the cap; once released the cap has its way" )
    try
    {
        CBlockCacheTestFolder folder( "acquired" );
        TCache cache;
        ASSERT_TRUE( OpenCache( cache, folder, (UInt64) B ) );
        // the entry was used and let go before: it is one that the cap could take, until someone holds it again
        THandle held = Acquire( cache, "share", "held", "r1", 3 * (UInt64) B );
        ASSERT_TRUE( BlockCacheTest::StoreBlocks( cache, held, 70, 0, 1 ) );
        cache.ReleaseEntry( held );
        held = Acquire( cache, "share", "held", "r1", 3 * (UInt64) B, false );
        ASSERT_TRUE( 0 != held );
        ASSERT_TRUE( BlockCacheTest::StoreBlocks( cache, held, 70, 1, 2 ) );
        const THandle other = Acquire( cache, "share", "other", "r1", 2 * (UInt64) B );
        ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, other, 71 ) );
        cache.ReleaseEntry( other );
        ASSERT_TRUE( 0 == Acquire( cache, "share", "other", "r1", 2 * (UInt64) B, false ) );          // removed, the held one is not
        ASSERT_TRUE( BlockCacheTest::BlockMatches( cache, held, 70, 0 ) && BlockCacheTest::BlockMatches( cache, held, 70, 2 ) );
        ASSERT_TRUE( 3 * (UInt64) B == cache.GetUsedBytes() );
        cache.ReleaseEntry( held );
        ASSERT_TRUE( 0 == cache.GetUsedBytes() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockCache: a superseded entry goes before the others, whatever their use" )
    try
    {
        CBlockCacheTestFolder folder( "supersededfirst" );
        TCache cache;
        ASSERT_TRUE( OpenCache( cache, folder, 4 * (UInt64) B ) );
        // n is the entry that was used longest ago, then comes the old revision of s
        const THandle n = Acquire( cache, "share", "n", "r1", 2 * (UInt64) B );
        ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, n, 80 ) );
        cache.ReleaseEntry( n );
        const THandle old = Acquire( cache, "share", "s", "old", 2 * (UInt64) B );
        ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, old, 81 ) );
        cache.ReleaseEntry( old );
        ASSERT_TRUE( 4 * (UInt64) B == cache.GetUsedBytes() );

        // s has a new revision, and the old one is known to be old
        const THandle current = Acquire( cache, "share", "s", "new", (UInt64) B );
        ASSERT_TRUE( 1 == cache.SupersedeOtherRevisions( "share", "s", "new" ) );
        ASSERT_TRUE( 4 * (UInt64) B == cache.GetUsedBytes() );
        ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, current, 82 ) );
        cache.ReleaseEntry( current );

        ASSERT_TRUE( 0 == Acquire( cache, "share", "s", "old", 2 * (UInt64) B, false ) );
        const THandle nAgain = Acquire( cache, "share", "n", "r1", 2 * (UInt64) B, false );
        ASSERT_TRUE( 0 != nAgain && BlockCacheTest::BlockMatches( cache, nAgain, 80, 1 ) );
        cache.ReleaseEntry( nAgain );
        const TCache::SStatistics stats = GetStats( cache );
        ASSERT_TRUE( 1 == stats.evictedEntries && 1 == stats.supersededEvictions );
        ASSERT_TRUE( 3 * (UInt64) B == cache.GetUsedBytes() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockCache: a complete file is checked against an MD5, SHA-1 or SHA-256, a mismatch is told and changes nothing, an incomplete one cannot be checked" )
    try
    {
        CBlockCacheTestFolder folder( "digest" );
        TCache cache;
        ASSERT_TRUE( OpenCache( cache, folder ) );

        // the published digests of "abc"
        const THandle abc = Acquire( cache, "share", "abc", "r1", 3 );
        ASSERT_TRUE( 0 != abc );
        ASSERT_TRUE( TCache::VERIFY_INCOMPLETE == cache.VerifyDigest( abc, VFS::CVfsDigest( VFS::CVfsDigest::ALGORITHM_MD5, "900150983cd24fb0d6963f7d28e17f72" ) ) );
        ASSERT_TRUE( cache.StoreBlock( abc, 0, "abc", 3 ) );
        const VFS::CVfsDigest md5( VFS::CVfsDigest::ALGORITHM_MD5, "900150983cd24fb0d6963f7d28e17f72" );
        const VFS::CVfsDigest sha1( VFS::CVfsDigest::ALGORITHM_SHA1, "a9993e364706816aba3e25717850c26c9cd0d89d" );
        const VFS::CVfsDigest sha256( VFS::CVfsDigest::ALGORITHM_SHA256, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad" );
        const VFS::CVfsDigest wrongMd5( VFS::CVfsDigest::ALGORITHM_MD5, "900150983cd24fb0d6963f7d28e17f73" );
        const VFS::CVfsDigest wrongSha1( VFS::CVfsDigest::ALGORITHM_SHA1, "b9993e364706816aba3e25717850c26c9cd0d89d" );
        const VFS::CVfsDigest wrongSha256( VFS::CVfsDigest::ALGORITHM_SHA256, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ac" );
        ASSERT_TRUE( TCache::VERIFY_MISMATCH == cache.VerifyDigest( abc, wrongMd5 ) );
        ASSERT_TRUE( TCache::VERIFY_MISMATCH == cache.VerifyDigest( abc, wrongSha1 ) );
        ASSERT_TRUE( TCache::VERIFY_MISMATCH == cache.VerifyDigest( abc, wrongSha256 ) );
        TCache::SEntryInfo info;
        ASSERT_TRUE( cache.GetEntryInfo( abc, info ) && !info.digestVerified && info.digest.IsEmpty() );
        ASSERT_TRUE( cache.IsComplete( abc ) && BlockCacheTest::BlockMatches( cache, abc, 0, 0 ) == false );    // other bytes than the pattern: it is the content "abc"
        ASSERT_TRUE( TCache::VERIFY_MATCH == cache.VerifyDigest( abc, md5 ) );
        ASSERT_TRUE( TCache::VERIFY_MATCH == cache.VerifyDigest( abc, sha1 ) );
        ASSERT_TRUE( TCache::VERIFY_MATCH == cache.VerifyDigest( abc, sha256 ) );
        ASSERT_TRUE( cache.GetEntryInfo( abc, info ) && info.digestVerified && info.digest.Matches( sha256 ) );
        // not a digest at all
        ASSERT_TRUE( TCache::VERIFY_UNSUPPORTED == cache.VerifyDigest( abc, VFS::CVfsDigest() ) );
        ASSERT_TRUE( TCache::VERIFY_UNSUPPORTED == cache.VerifyDigest( abc, VFS::CVfsDigest( VFS::CVfsDigest::ALGORITHM_UNKNOWN, "900150983cd24fb0d6963f7d28e17f72" ) ) );
        ASSERT_TRUE( TCache::VERIFY_UNSUPPORTED == cache.VerifyDigest( abc, VFS::CVfsDigest( VFS::CVfsDigest::ALGORITHM_MD5, "9001" ) ) );
        ASSERT_TRUE( TCache::VERIFY_INCOMPLETE == cache.VerifyDigest( 4242, md5 ) );
        cache.ReleaseEntry( abc );

        // a file of several blocks, the digests of its content
        const UInt64 fileSize = 3 * (UInt64) B + 1234;
        const THandle big = Acquire( cache, "share", "big", "r1", fileSize );
        ASSERT_TRUE( BlockCacheTest::StoreBlocks( cache, big, 90, 0, 3 ) );
        const VFS::CVfsDigest bigSha = BlockCacheTest::PatternDigest( VFS::CVfsDigest::ALGORITHM_SHA256, 90, fileSize );
        ASSERT_TRUE( TCache::VERIFY_INCOMPLETE == cache.VerifyDigest( big, bigSha ) );
        ASSERT_TRUE( BlockCacheTest::StoreBlocks( cache, big, 90, 3, 1 ) );
        ASSERT_TRUE( TCache::VERIFY_MATCH == cache.VerifyDigest( big, BlockCacheTest::PatternDigest( VFS::CVfsDigest::ALGORITHM_MD5, 90, fileSize ) ) );
        ASSERT_TRUE( TCache::VERIFY_MATCH == cache.VerifyDigest( big, BlockCacheTest::PatternDigest( VFS::CVfsDigest::ALGORITHM_SHA1, 90, fileSize ) ) );
        ASSERT_TRUE( TCache::VERIFY_MISMATCH == cache.VerifyDigest( big, BlockCacheTest::PatternDigest( VFS::CVfsDigest::ALGORITHM_SHA256, 91, fileSize ) ) );
        ASSERT_TRUE( TCache::VERIFY_MATCH == cache.VerifyDigest( big, bigSha ) );
        cache.ReleaseEntry( big );
        cache.Close();

        // what was verified is in the index: a restart does not hash again. The proof is a damaged block that a hash would find
        ASSERT_TRUE( OpenCache( cache, folder ) );
        const THandle bigAgain = Acquire( cache, "share", "big", "r1", fileSize, false );
        ASSERT_TRUE( 0 != bigAgain );
        ASSERT_TRUE( cache.GetEntryInfo( bigAgain, info ) && info.digestVerified && info.digest.Matches( bigSha ) );
        cache.ReleaseEntry( bigAgain );
        cache.Close();
        const CORE::CString::StringVector data = BlockCacheTest::ListEntryFiles( folder.GetPath(), ".vbd" );
        CORE::CString bigData;
        for ( size_t d=0; d<data.size(); ++d )
        {
            if ( CORE::FileSize( data[ d ] ) == fileSize )
                bigData = data[ d ];
        }
        ASSERT_FALSE( bigData.IsNULLOrEmpty() );
        ASSERT_TRUE( BlockCacheTest::DamageByte( bigData, B + 10 ) );
        ASSERT_TRUE( OpenCache( cache, folder ) );
        const THandle damaged = Acquire( cache, "share", "big", "r1", fileSize, false );
        ASSERT_TRUE( TCache::VERIFY_MATCH == cache.VerifyDigest( damaged, bigSha ) );
        ASSERT_TRUE( TCache::VERIFY_INCOMPLETE == cache.VerifyDigest( damaged, BlockCacheTest::PatternDigest( VFS::CVfsDigest::ALGORITHM_MD5, 90, fileSize ) ) );
        ASSERT_FALSE( cache.IsComplete( damaged ) );
        ASSERT_TRUE( cache.GetEntryInfo( damaged, info ) && !info.digestVerified );
        ASSERT_TRUE( 1 == GetStats( cache ).corruptBlocks );
        cache.ReleaseEntry( damaged );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockCache: a digest that is known but not verified is kept; ResetEntry empties the entry and keeps its pin; a content with another size or digest is another content" )
    try
    {
        CBlockCacheTestFolder folder( "reset" );
        TCache cache;
        ASSERT_TRUE( OpenCache( cache, folder ) );
        const VFS::CVfsDigest first( VFS::CVfsDigest::ALGORITHM_SHA256, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad" );
        const VFS::CVfsDigest second( VFS::CVfsDigest::ALGORITHM_SHA256, "ca7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad" );
        const THandle h = cache.AcquireEntry( "share", "f", "r1", 2 * (UInt64) B, first, true );
        ASSERT_TRUE( 0 != h );
        TCache::SEntryInfo info;
        ASSERT_TRUE( cache.GetEntryInfo( h, info ) && info.digest.Matches( first ) && !info.digestVerified );
        cache.SetEntryDigest( h, second );                                  // it has one: the first stays
        ASSERT_TRUE( cache.GetEntryInfo( h, info ) && info.digest.Matches( first ) );
        ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, h, 100 ) );
        ASSERT_TRUE( cache.SetPinned( "share", "f", true ) );
        ASSERT_TRUE( 2 * (UInt64) B == cache.GetUsedBytes() );

        // another digest of the same algorithm for this revision: not this content, and the entry is in use
        ASSERT_TRUE( 0 == cache.AcquireEntry( "share", "f", "r1", 2 * (UInt64) B, second, true ) );
        // another size likewise
        ASSERT_TRUE( 0 == Acquire( cache, "share", "f", "r1", 3 * (UInt64) B ) );
        // the same again is the same entry
        ASSERT_TRUE( h == cache.AcquireEntry( "share", "f", "r1", 2 * (UInt64) B, first, true ) );
        cache.ReleaseEntry( h );

        cache.ResetEntry( h );
        ASSERT_TRUE( 0 == cache.GetBlocksHeld( h ) && 0 == cache.GetUsedBytes() );
        ASSERT_FALSE( cache.HasBlock( h, 0 ) );
        ASSERT_TRUE( cache.GetEntryInfo( h, info ) && info.digest.IsEmpty() && !info.digestVerified && info.pinned );
        ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, h, 101 ) );
        ASSERT_TRUE( BlockCacheTest::BlockMatches( cache, h, 101, 0 ) && BlockCacheTest::BlockMatches( cache, h, 101, 1 ) );
        cache.ReleaseEntry( h );

        // released, the entry takes another content under the same key
        const THandle another = cache.AcquireEntry( "share", "f", "r1", 3 * (UInt64) B, VFS::CVfsDigest(), true );
        ASSERT_TRUE( 0 != another );
        ASSERT_TRUE( 3 == cache.GetBlockCount( another ) && 0 == cache.GetBlocksHeld( another ) );
        ASSERT_TRUE( 1 == GetStats( cache ).entries );
        cache.ReleaseEntry( another );
        // the digest of a find, where there is none yet, is kept
        const THandle found = cache.AcquireEntry( "share", "f", "r1", 3 * (UInt64) B, second, false );
        ASSERT_TRUE( 0 != found && cache.GetEntryInfo( found, info ) && info.digest.Matches( second ) );
        cache.ReleaseEntry( found );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockCache: a second cache on the folder is refused while the first one has it, a lock left behind is refused until the application breaks it on purpose" )
    try
    {
        CBlockCacheTestFolder folder( "lock" );
        {
            TCache first;
            TCache second;
            ASSERT_TRUE( OpenCache( first, folder ) );
            ASSERT_TRUE( LockFileExists( folder ) );
            ASSERT_FALSE( OpenCache( second, folder ) );
            ASSERT_FALSE( second.IsOpen() );
            ASSERT_TRUE( LockFileExists( folder ) );                    // the refused opener did not take the lock of the first away
            // not even with the permission to break a lock: that is for a lock of a process that is gone, and this one is
            // not refused by the lock alone, but the application asked for it, so it is done
            first.Close();
            ASSERT_FALSE( LockFileExists( folder ) );
            ASSERT_TRUE( OpenCache( second, folder ) );
            ASSERT_FALSE( OpenCache( first, folder ) );
        }
        ASSERT_FALSE( LockFileExists( folder ) );

        // a crash leaves the lock: a file that says who had it
        ASSERT_TRUE( CORE::WriteStringAsTextFile( folder.GetChildPath( "cache.lock" ), "pid=999999\nstartedUtcMs=1\n" ) );
        TCache cache;
        ASSERT_FALSE( OpenCache( cache, folder ) );
        ASSERT_FALSE( cache.IsOpen() );
        ASSERT_TRUE( LockFileExists( folder ) );
        ASSERT_TRUE( FileHasContent( folder.GetChildPath( "cache.lock" ), "pid=999999\nstartedUtcMs=1\n" ) );
        ASSERT_TRUE( cache.Open( folder.GetPath(), 0, B, 128, true ) );
        ASSERT_TRUE( cache.IsOpen() && LockFileExists( folder ) );
        // the lock is the one of this process now
        const CORE::CDynamicBuffer lockContent = BlockCacheTest::ReadWholeFile( folder.GetChildPath( "cache.lock" ) );
        const CORE::CString lockText( static_cast< const char* >( lockContent.GetConstBufferPtr() ), lockContent.GetDataSize() );
        ASSERT_TRUE( lockText.HasSubstr( "pid=" + CORE::ToString( MT::GetProcessID() ) + "\n" ) >= 0 );
        cache.Close();
        ASSERT_FALSE( LockFileExists( folder ) );

        // breaking a lock that is not there is nothing special
        ASSERT_TRUE( cache.Open( folder.GetPath(), 0, B, 128, true ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockCache: an index that is cut short at any length, or has another magic, block size, checksum or length than it says, is removed with its data file and never served" )
    try
    {
        CBlockCacheTestFolder folder( "tornindex" );
        const UInt64 fileSize = 100;
        {
            TCache cache;
            ASSERT_TRUE( OpenCache( cache, folder ) );
            const THandle h = Acquire( cache, "share", "f.bin", "r1", fileSize );
            ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, h, 110 ) );
            cache.ReleaseEntry( h );
        }
        const CORE::CString::StringVector indexes = BlockCacheTest::ListEntryFiles( folder.GetPath(), ".vbi" );
        const CORE::CString::StringVector datas = BlockCacheTest::ListEntryFiles( folder.GetPath(), ".vbd" );
        ASSERT_TRUE( 1 == indexes.size() && 1 == datas.size() );
        const CORE::CDynamicBuffer indexBytes = BlockCacheTest::ReadWholeFile( indexes[ 0 ] );
        const CORE::CDynamicBuffer dataBytes = BlockCacheTest::ReadWholeFile( datas[ 0 ] );
        ASSERT_TRUE( 2048 + 8 == indexBytes.GetDataSize() && 100 == dataBytes.GetDataSize() );

        UInt32 survived = 0;
        for ( UInt32 length = 0; length < indexBytes.GetDataSize(); ++length )
        {
            CORE::CDynamicBuffer cut;
            cut.SetBufferSize( length > 0 ? length : 1 );
            if ( length > 0 )
                memcpy( cut.GetBufferPtr(), indexBytes.GetConstBufferPtr(), length );
            cut.SetDataSize( length );
            BlockCacheTest::WriteWholeFile( indexes[ 0 ], cut );
            BlockCacheTest::WriteWholeFile( datas[ 0 ], dataBytes );
            TCache cache;
            if ( !OpenCache( cache, folder ) || 0 != GetStats( cache ).entries || 0 != cache.GetUsedBytes() || 0 != Acquire( cache, "share", "f.bin", "r1", fileSize, false ) )
                ++survived;
            cache.Close();
            if ( CORE::FileExists( indexes[ 0 ] ) || CORE::FileExists( datas[ 0 ] ) )
                ++survived;
        }
        ASSERT_TRUE( 0 == survived );

        // longer than it should be
        {
            CORE::CDynamicBuffer longer;
            longer.SetBufferSize( indexBytes.GetDataSize() + 16 );
            memset( longer.GetBufferPtr(), 0, indexBytes.GetDataSize() + 16 );
            memcpy( longer.GetBufferPtr(), indexBytes.GetConstBufferPtr(), indexBytes.GetDataSize() );
            longer.SetDataSize( indexBytes.GetDataSize() + 16 );
            BlockCacheTest::WriteWholeFile( indexes[ 0 ], longer );
            BlockCacheTest::WriteWholeFile( datas[ 0 ], dataBytes );
            TCache cache;
            ASSERT_TRUE( OpenCache( cache, folder ) );
            ASSERT_TRUE( 0 == GetStats( cache ).entries );
        }

        // a byte changed anywhere in the header: the checksum of the header says no (a byte of the table is for the block checksums to find)
        UInt32 headerSurvived = 0;
        const UInt32 offsets[] = { 0, 3, 4, 8, 12, 16, 24, 28, 32, 36, 40, 104, 108, 112, 120, 128, 384, 640, 1000, 1664, 1670, 2047 };
        for ( UInt32 o=0; o<sizeof( offsets ) / sizeof( offsets[ 0 ] ); ++o )
        {
            CORE::CDynamicBuffer changed;
            changed.SetBufferSize( indexBytes.GetDataSize() );
            memcpy( changed.GetBufferPtr(), indexBytes.GetConstBufferPtr(), indexBytes.GetDataSize() );
            changed.SetDataSize( indexBytes.GetDataSize() );
            static_cast< UInt8* >( changed.GetBufferPtr() )[ offsets[ o ] ] ^= 0x01;
            BlockCacheTest::WriteWholeFile( indexes[ 0 ], changed );
            BlockCacheTest::WriteWholeFile( datas[ 0 ], dataBytes );
            TCache cache;
            if ( !OpenCache( cache, folder ) || 0 != GetStats( cache ).entries )
                ++headerSurvived;
        }
        ASSERT_TRUE( 0 == headerSurvived );

        // a cache with another block size does not take the entries of this one
        BlockCacheTest::WriteWholeFile( indexes[ 0 ], indexBytes );
        BlockCacheTest::WriteWholeFile( datas[ 0 ], dataBytes );
        {
            TCache cache;
            ASSERT_TRUE( cache.Open( folder.GetPath(), 0, 2 * B ) );
            ASSERT_TRUE( 0 == GetStats( cache ).entries );
        }
        ASSERT_TRUE( 0 == BlockCacheTest::ListEntryFiles( folder.GetPath(), ".vbi" ).size() && 0 == BlockCacheTest::ListEntryFiles( folder.GetPath(), ".vbd" ).size() );

        // whole it is an entry
        BlockCacheTest::WriteWholeFile( indexes[ 0 ], indexBytes );
        BlockCacheTest::WriteWholeFile( datas[ 0 ], dataBytes );
        {
            TCache cache;
            ASSERT_TRUE( OpenCache( cache, folder ) );
            ASSERT_TRUE( 1 == GetStats( cache ).entries );
            const THandle h = Acquire( cache, "share", "f.bin", "r1", fileSize, false );
            ASSERT_TRUE( 0 != h && BlockCacheTest::BlockMatches( cache, h, 110, 0 ) );
            cache.ReleaseEntry( h );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockCache: a data file shorter than the index says is found by the checksums, the blocks are dropped and stored again, no wrong byte is served" )
    try
    {
        CBlockCacheTestFolder folder( "shortdata" );
        const UInt64 fileSize = 4 * (UInt64) B;
        {
            TCache cache;
            ASSERT_TRUE( OpenCache( cache, folder ) );
            const THandle h = Acquire( cache, "share", "f.bin", "r1", fileSize );
            ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, h, 120 ) );
            cache.ReleaseEntry( h );
        }
        const CORE::CString::StringVector datas = BlockCacheTest::ListEntryFiles( folder.GetPath(), ".vbd" );
        ASSERT_TRUE( 1 == datas.size() );
        ASSERT_TRUE( CORE::TruncateFile( datas[ 0 ], B + 100 ) );
        {
            TCache cache;
            ASSERT_TRUE( OpenCache( cache, folder ) );
            const THandle h = Acquire( cache, "share", "f.bin", "r1", fileSize, false );
            ASSERT_TRUE( 0 != h );
            ASSERT_TRUE( BlockCacheTest::BlockMatches( cache, h, 120, 0 ) );
            ASSERT_TRUE( TCache::READ_CORRUPT == BlockCacheTest::ReadWholeBlock( cache, h, 1 ) );
            ASSERT_TRUE( TCache::READ_CORRUPT == BlockCacheTest::ReadWholeBlock( cache, h, 2 ) );
            ASSERT_TRUE( TCache::READ_CORRUPT == BlockCacheTest::ReadWholeBlock( cache, h, 3 ) );
            ASSERT_TRUE( 1 == cache.GetBlocksHeld( h ) && B == cache.GetUsedBytes() );
            ASSERT_TRUE( BlockCacheTest::StoreBlocks( cache, h, 120, 1, 3 ) );
            for ( UInt32 b=0; b<4; ++b )
                ASSERT_TRUE( BlockCacheTest::BlockMatches( cache, h, 120, b ) );
            cache.ReleaseEntry( h );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockCache: only the files of its own layout are looked at: the files of another format and anything foreign stay, a data file without an index is removed" )
    try
    {
        CBlockCacheTestFolder folder( "foreign" );
        CORE::CreateDirs( folder.GetChildPath( "ab" ) );
        CORE::CreateDirs( folder.GetChildPath( "notafanout" ) );
        const char* const foreign[] = { "readme.txt", "0123456789abcdef0123456789abcdef.idx", "0123456789abcdef0123456789abcdef.dat", "ab/notes.txt",
                                        "ab/abcdef0123456789abcdef0123456789.dat", "ab/abcdef0123456789abcdef0123456789.vbi.bak", "notafanout/0123456789abcdef0123456789abcdef.vbi" };
        for ( UInt32 f=0; f<sizeof( foreign ) / sizeof( foreign[ 0 ] ); ++f )
            ASSERT_TRUE( CORE::WriteStringAsTextFile( folder.GetChildPath( foreign[ f ] ), "foreign file content" ) );
        // a data file of the layout without an index: cleaned up
        ASSERT_TRUE( CORE::WriteStringAsTextFile( folder.GetChildPath( "ab/abcdef0123456789abcdef0123456789.vbd" ), "orphan" ) );

        TCache cache;
        ASSERT_TRUE( OpenCache( cache, folder, 3 * (UInt64) B ) );
        ASSERT_FALSE( CORE::FileExists( folder.GetChildPath( "ab/abcdef0123456789abcdef0123456789.vbd" ) ) );
        for ( UInt32 n=0; n<6; ++n )
        {
            const THandle h = cache.AcquireEntry( "share", "f", CORE::ToString( n ), 2 * (UInt64) B, VFS::CVfsDigest(), true );
            ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, h, 130 + n ) );
            cache.ReleaseEntry( h );
        }
        ASSERT_TRUE( GetStats( cache ).evictedEntries > 0 );
        ASSERT_TRUE( cache.Clear() > 0 );
        ASSERT_TRUE( 0 == GetStats( cache ).entries );
        cache.Close();
        for ( UInt32 f=0; f<sizeof( foreign ) / sizeof( foreign[ 0 ] ); ++f )
            ASSERT_TRUE( FileHasContent( folder.GetChildPath( foreign[ f ] ), "foreign file content" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockCache: the limits of the key (256 bytes of identity and revision, 1024 of path) are kept and an entry over them, an empty file or a line feed in the key is no entry" )
    try
    {
        CBlockCacheTestFolder folder( "limits" );
        TCache cache;
        ASSERT_TRUE( OpenCache( cache, folder ) );
        const CORE::CString identity256( CORE::CString( "i" ).Lowercase() + CORE::CString( "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" ).SubstrFromRange( 0, 255 ) );
        ASSERT_TRUE( 256 == identity256.Length() );
        CORE::CString revision256 = identity256;
        CORE::CString path1024;
        while ( path1024.Length() < 1024 )
            path1024 += "dir0123456789/";
        path1024 = path1024.SubstrFromRange( 0, 1024 );
        ASSERT_TRUE( 1024 == path1024.Length() );

        ASSERT_TRUE( 0 == cache.AcquireEntry( identity256 + "y", "f", "r", B, VFS::CVfsDigest(), true ) );
        ASSERT_TRUE( 0 == cache.AcquireEntry( "id", "f", revision256 + "y", B, VFS::CVfsDigest(), true ) );
        ASSERT_TRUE( 0 == cache.AcquireEntry( "id", path1024 + "y", "r", B, VFS::CVfsDigest(), true ) );
        ASSERT_TRUE( 0 == cache.AcquireEntry( "id", "f", "r", 0, VFS::CVfsDigest(), true ) );
        ASSERT_TRUE( 0 == cache.AcquireEntry( "id", CORE::CString::Empty, "r", B, VFS::CVfsDigest(), true ) );
        ASSERT_TRUE( 0 == cache.AcquireEntry( "id\nx", "f", "r", B, VFS::CVfsDigest(), true ) );
        ASSERT_TRUE( 0 == cache.AcquireEntry( "id", "f\nx", "r", B, VFS::CVfsDigest(), true ) );
        ASSERT_TRUE( 0 == cache.AcquireEntry( "id", "f", "r\nx", B, VFS::CVfsDigest(), true ) );
        ASSERT_TRUE( 0 == GetStats( cache ).entries && 0 == BlockCacheTest::ListEntryFiles( folder.GetPath(), ".vbi" ).size() );

        const THandle h = cache.AcquireEntry( identity256, path1024, revision256, B + 5, VFS::CVfsDigest(), true );
        ASSERT_TRUE( 0 != h );
        ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, h, 140 ) );
        cache.ReleaseEntry( h );
        cache.Close();
        ASSERT_TRUE( OpenCache( cache, folder ) );
        TCache::SEntryInfo info;
        ASSERT_TRUE( cache.FindNewestRevision( identity256, path1024, info ) );
        ASSERT_TRUE( info.identity == identity256 && info.path == path1024 && info.revision == revision256 && info.fileSize == B + 5 && info.complete );
        const THandle again = cache.AcquireEntry( identity256, path1024, revision256, B + 5, VFS::CVfsDigest(), false );
        ASSERT_TRUE( 0 != again && BlockCacheTest::BlockMatches( cache, again, 140, 1 ) );
        cache.ReleaseEntry( again );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockCache: finding an entry makes none, a held entry is one entry for every holder and is not lost when one of them lets go" )
    try
    {
        CBlockCacheTestFolder folder( "find" );
        TCache cache;
        ASSERT_TRUE( OpenCache( cache, folder ) );
        ASSERT_TRUE( 0 == Acquire( cache, "share", "f", "r1", 2 * (UInt64) B, false ) );
        ASSERT_TRUE( 0 == GetStats( cache ).entries && 0 == BlockCacheTest::ListEntryFiles( folder.GetPath(), ".vbi" ).size() );

        const THandle first = Acquire( cache, "share", "f", "r1", 2 * (UInt64) B );
        const THandle second = Acquire( cache, "share", "f", "r1", 2 * (UInt64) B, false );
        ASSERT_TRUE( 0 != first && first == second );
        ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, first, 150 ) );
        cache.ReleaseEntry( first );
        ASSERT_TRUE( BlockCacheTest::BlockMatches( cache, second, 150, 1 ) );          // the other holder reads on
        cache.ReleaseEntry( second );
        cache.ReleaseEntry( second );                                                   // too often is nothing
        cache.ReleaseEntry( 0 );
        cache.ReleaseEntry( 987654 );
        ASSERT_TRUE( 1 == GetStats( cache ).entries && 2 * (UInt64) B == cache.GetUsedBytes() );
        const THandle third = Acquire( cache, "share", "f", "r1", 2 * (UInt64) B, false );
        ASSERT_TRUE( 0 != third && BlockCacheTest::BlockMatches( cache, third, 150, 0 ) );
        cache.ReleaseEntry( third );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockCache: RemoveEntries takes every revision of a path (a pinned one too) but not an acquired one, Clear takes what is not pinned and not acquired" )
    try
    {
        CBlockCacheTestFolder folder( "remove" );
        TCache cache;
        ASSERT_TRUE( OpenCache( cache, folder ) );
        const THandle r1 = Acquire( cache, "share", "f", "r1", (UInt64) B );
        const THandle r2 = Acquire( cache, "share", "f", "r2", (UInt64) B );
        const THandle keep = Acquire( cache, "share", "keep", "r1", (UInt64) B );
        const THandle pinned = Acquire( cache, "share", "pinned", "r1", (UInt64) B );
        const THandle free = Acquire( cache, "share", "free", "r1", (UInt64) B );
        const THandle handles[] = { r1, r2, keep, pinned, free };
        for ( UInt32 h=0; h<5; ++h )
            ASSERT_TRUE( 0 != handles[ h ] && BlockCacheTest::StoreAllBlocks( cache, handles[ h ], 160 + h ) );
        ASSERT_TRUE( cache.SetPinned( "share", "pinned", true ) );
        cache.ReleaseEntry( r1 );
        cache.ReleaseEntry( pinned );
        cache.ReleaseEntry( free );
        ASSERT_TRUE( 5 * (UInt64) B == cache.GetUsedBytes() );

        // r2 is still held
        ASSERT_TRUE( (UInt64) B == cache.RemoveEntries( "share", "f" ) );
        ASSERT_TRUE( 0 == Acquire( cache, "share", "f", "r1", (UInt64) B, false ) );
        ASSERT_TRUE( BlockCacheTest::BlockMatches( cache, r2, 161, 0 ) );
        ASSERT_TRUE( 0 == cache.RemoveEntries( "share", "nosuchpath" ) );
        cache.ReleaseEntry( r2 );
        ASSERT_TRUE( (UInt64) B == cache.RemoveEntries( "share", "f" ) );
        ASSERT_TRUE( 3 * (UInt64) B == cache.GetUsedBytes() );

        // keep is held and pinned is pinned
        ASSERT_TRUE( 1 * (UInt64) B == cache.Clear() );
        ASSERT_TRUE( 2 * (UInt64) B == cache.GetUsedBytes() );
        ASSERT_TRUE( BlockCacheTest::BlockMatches( cache, keep, 162, 0 ) );
        cache.ReleaseEntry( keep );
        ASSERT_TRUE( (UInt64) B == cache.Clear() );
        const THandle stays = Acquire( cache, "share", "pinned", "r1", (UInt64) B, false );
        ASSERT_TRUE( 0 != stays && BlockCacheTest::BlockMatches( cache, stays, 163, 0 ) );
        cache.ReleaseEntry( stays );
        // removing the path takes the pinned one
        ASSERT_TRUE( (UInt64) B == cache.RemoveEntries( "share", "pinned" ) && 0 == cache.GetUsedBytes() );
        ASSERT_TRUE( 0 == BlockCacheTest::ListEntryFiles( folder.GetPath(), ".vbi" ).size() && 0 == BlockCacheTest::ListEntryFiles( folder.GetPath(), ".vbd" ).size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockCache: the newest revision of a path and the entries below a directory, for what an offline listing is made of" )
    try
    {
        CBlockCacheTestFolder folder( "below" );
        TCache cache;
        ASSERT_TRUE( OpenCache( cache, folder ) );
        const char* const paths[] = { "z.bin", "a/d.bin", "a/b/c.bin", "a/b/e.bin", "x/y.bin", "ab/other.bin" };
        for ( UInt32 p=0; p<sizeof( paths ) / sizeof( paths[ 0 ] ); ++p )
        {
            const THandle h = Acquire( cache, "share", paths[ p ], "r1", 100 + p );
            ASSERT_TRUE( 0 != h && BlockCacheTest::StoreAllBlocks( cache, h, 170 + p ) );
            cache.ReleaseEntry( h );
        }
        // an entry of another place, an old revision of one of the paths
        const THandle foreign = Acquire( cache, "other", "a/d.bin", "r1", 50 );
        ASSERT_TRUE( 0 != foreign && BlockCacheTest::StoreAllBlocks( cache, foreign, 180 ) );
        cache.ReleaseEntry( foreign );
        const THandle newer = Acquire( cache, "share", "a/d.bin", "r2", 77 );
        ASSERT_TRUE( 0 != newer && BlockCacheTest::StoreAllBlocks( cache, newer, 181 ) );
        cache.ReleaseEntry( newer );
        cache.SupersedeOtherRevisions( "share", "a/d.bin", "r2" );

        TCache::TEntryInfoVector infos;
        cache.GetEntryInfosBelow( "share", "", false, infos );
        ASSERT_TRUE( 1 == infos.size() && infos[ 0 ].path == "z.bin" );
        cache.GetEntryInfosBelow( "share", "a", false, infos );
        ASSERT_TRUE( 1 == infos.size() && infos[ 0 ].path == "a/d.bin" && infos[ 0 ].revision == "r2" && 77 == infos[ 0 ].fileSize );
        cache.GetEntryInfosBelow( "share", "/a/", true, infos );
        ASSERT_TRUE( 3 == infos.size() );
        cache.GetEntryInfosBelow( "share", "a/b", false, infos );
        ASSERT_TRUE( 2 == infos.size() );
        cache.GetEntryInfosBelow( "share", "", true, infos );
        ASSERT_TRUE( 6 == infos.size() );
        cache.GetEntryInfosBelow( "share", "ab", true, infos );              // not "a": the directory is a whole name
        ASSERT_TRUE( 1 == infos.size() && infos[ 0 ].path == "ab/other.bin" );
        cache.GetEntryInfosBelow( "other", "a", true, infos );
        ASSERT_TRUE( 1 == infos.size() && infos[ 0 ].identity == "other" );
        cache.GetEntryInfosBelow( "share", "nothere", true, infos );
        ASSERT_TRUE( infos.empty() );

        TCache::SEntryInfo info;
        ASSERT_TRUE( cache.FindNewestRevision( "share", "a/d.bin", info ) && info.revision == "r2" && info.complete && 77 == info.bytesHeld );
        ASSERT_FALSE( cache.FindNewestRevision( "share", "a", info ) );
        ASSERT_FALSE( cache.FindNewestRevision( "share", "nothere.bin", info ) );

        // after a restart they are still there, without any of them having been acquired
        cache.Close();
        ASSERT_TRUE( OpenCache( cache, folder ) );
        cache.GetEntryInfosBelow( "share", "a", true, infos );
        ASSERT_TRUE( 3 == infos.size() );
        ASSERT_TRUE( cache.FindNewestRevision( "share", "a/d.bin", info ) && info.revision == "r2" && info.complete );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockCache: the statistics count what happened" )
    try
    {
        CBlockCacheTestFolder folder( "stats" );
        TCache cache;
        ASSERT_TRUE( OpenCache( cache, folder, 10 * (UInt64) B ) );
        const THandle h = Acquire( cache, "share", "f", "r1", 3 * (UInt64) B );
        ASSERT_TRUE( BlockCacheTest::StoreBlocks( cache, h, 190, 0, 2 ) );
        UInt8 one = 0;
        ASSERT_TRUE( TCache::READ_HIT == cache.ReadBlock( h, 0, 0, &one, 1 ) );
        ASSERT_TRUE( TCache::READ_HIT == cache.ReadBlock( h, 1, 0, &one, 1 ) );
        ASSERT_TRUE( TCache::READ_MISS == cache.ReadBlock( h, 2, 0, &one, 1 ) );
        TCache::SStatistics stats = GetStats( cache );
        ASSERT_TRUE( 10 * (UInt64) B == stats.maxBytes && 2 * (UInt64) B == stats.usedBytes && 1 == stats.entries );
        ASSERT_TRUE( 2 == stats.blocksStored && 2 == stats.blockHits && 1 == stats.blockMisses );
        ASSERT_TRUE( 0 == stats.corruptBlocks && 0 == stats.evictedEntries && 0 == stats.pinnedEntries && 0 == stats.storeFailures );
        ASSERT_TRUE( stats.openFiles <= 1 );
        cache.ReleaseEntry( h );
        stats = GetStats( cache );
        ASSERT_TRUE( 0 == stats.openFiles );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockCache: a block that cannot be kept (a data file that cannot be written, a folder that is gone) is a false from StoreBlock and a counter, the rest goes on" )
    #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    GUCEF_TESTFW_SKIP_IF( BlockCacheTest::IsSuperUser(), "a super user can write a file whatever its permissions" );
    #else
    GUCEF_TESTFW_SKIP( "the permission cases are made for the POSIX permissions" );
    #endif
    try
    {
        #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
        CBlockCacheTestFolder folder( "unwritable" );
        TCache cache;
        ASSERT_TRUE( OpenCache( cache, folder ) );
        const THandle h = Acquire( cache, "share", "f", "r1", 3 * (UInt64) B );
        ASSERT_TRUE( BlockCacheTest::StoreBlocks( cache, h, 200, 0, 1 ) );
        cache.ReleaseEntry( h );
        const CORE::CString::StringVector datas = BlockCacheTest::ListEntryFiles( folder.GetPath(), ".vbd" );
        ASSERT_TRUE( 1 == datas.size() );
        ASSERT_TRUE( 0 == chmod( datas[ 0 ].C_String(), 0444 ) );
        const THandle again = Acquire( cache, "share", "f", "r1", 3 * (UInt64) B, false );
        ASSERT_TRUE( 0 != again );
        ASSERT_TRUE( BlockCacheTest::BlockMatches( cache, again, 200, 0 ) );           // reading still works
        GUCEF::vector< UInt8 > block;
        block.resize( B );
        ASSERT_FALSE( cache.StoreBlock( again, 1, &block[ 0 ], B ) );
        ASSERT_FALSE( cache.StoreBlock( again, 2, &block[ 0 ], B ) );
        ASSERT_TRUE( 2 == GetStats( cache ).storeFailures );
        ASSERT_FALSE( cache.HasBlock( again, 1 ) );
        ASSERT_TRUE( B == cache.GetUsedBytes() );
        cache.ReleaseEntry( again );
        chmod( datas[ 0 ].C_String(), 0644 );

        // fixed, it stores again
        const THandle fixed = Acquire( cache, "share", "f", "r1", 3 * (UInt64) B, false );
        ASSERT_TRUE( BlockCacheTest::StoreBlocks( cache, fixed, 200, 1, 2 ) && cache.IsComplete( fixed ) );
        cache.ReleaseEntry( fixed );

        // the folder is gone: no entry can be made, what is held in memory cannot be written, and nothing breaks
        const THandle gone = Acquire( cache, "share", "f", "r1", 3 * (UInt64) B, false );
        ASSERT_TRUE( 0 != gone );
        UInt8 one = 0;
        ASSERT_TRUE( CORE::DeleteDir( CORE::StripFilename( datas[ 0 ] ), true ) );
        // the file cannot be read back, whatever happens to the folder (it may be made again by the next open: then the block is found damaged)
        ASSERT_TRUE( TCache::READ_HIT != cache.ReadBlock( gone, 0, 0, &one, 1 ) );
        ASSERT_FALSE( cache.HasBlock( gone, 0 ) && BlockCacheTest::BlockMatches( cache, gone, 999, 0 ) );
        cache.ReleaseEntry( gone );
        cache.Close();
        #endif
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockCache: a copy of the folder of a process that did not close is usable, at most the rows that were not written are missing and no byte is wrong" )
    try
    {
        CBlockCacheTestFolder folder( "crash" );
        CBlockCacheTestFolder copy( "crashcopy" );
        const UInt64 fileSize = 64 * (UInt64) B;
        TCache cache;
        ASSERT_TRUE( OpenCache( cache, folder ) );
        const THandle h = Acquire( cache, "share", "f.bin", "r1", fileSize );
        ASSERT_TRUE( BlockCacheTest::StoreBlocks( cache, h, 210, 0, 40 ) );
        // the process "dies" here: the folder is copied as it is on the disk, with the lock of the process in it
        ASSERT_TRUE( CopyCacheFolder( folder.GetPath(), copy.GetPath() ) );

        TCache restarted;
        ASSERT_FALSE( OpenCache( restarted, copy ) );                           // the lock of the dead process
        ASSERT_TRUE( restarted.Open( copy.GetPath(), 0, B, 128, true ) );
        ASSERT_TRUE( 1 == GetStats( restarted ).entries );
        const THandle again = Acquire( restarted, "share", "f.bin", "r1", fileSize, false );
        ASSERT_TRUE( 0 != again );
        UInt32 held = 0;
        UInt32 wrong = 0;
        for ( UInt32 b=0; b<64; ++b )
        {
            if ( restarted.HasBlock( again, b ) )
            {
                ++held;
                if ( !BlockCacheTest::BlockMatches( restarted, again, 210, b ) )
                    ++wrong;
            }
        }
        ASSERT_TRUE( 0 == wrong );
        ASSERT_TRUE( held >= 32 && held <= 40 );
        // what is missing is fetched again, and the file is whole
        for ( UInt32 b=0; b<64; ++b )
        {
            if ( !restarted.HasBlock( again, b ) )
                ASSERT_TRUE( BlockCacheTest::StoreBlocks( restarted, again, 210, b, 1 ) );
        }
        ASSERT_TRUE( restarted.IsComplete( again ) );
        for ( UInt32 b=0; b<64; ++b )
            ASSERT_TRUE( BlockCacheTest::BlockMatches( restarted, again, 210, b ) );
        restarted.ReleaseEntry( again );
        cache.ReleaseEntry( h );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
