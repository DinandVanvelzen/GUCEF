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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_MT_H
#include "gucefMT.h"
#define GUCEF_MT_H
#endif /* GUCEF_MT_H ? */

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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

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

TCache::SStatistics
GetStats( const TCache& cache )
{
    TCache::SStatistics statistics;
    cache.GetStatistics( statistics );
    return statistics;
}

/*-------------------------------------------------------------------------*/

/** What a thread of the load case does: it asks for blocks of shared files, stores what is missing and checks what it reads */
struct SWorker
{
    TCache* cache;
    UInt32 index;
    UInt32 iterations;
    UInt32 fileCount;
    UInt32 blocksPerFile;
    MT::CTAtomic< UInt32 > wrongBytes;
    MT::CTAtomic< UInt32 > hits;
    MT::CTAtomic< UInt32 > stores;
    MT::CTAtomic< UInt32 > storeFailures;
    MT::CTAtomic< UInt32 > noEntry;
    MT::CTAtomic< UInt32 > done;
    MT::SThreadData* thread;

    SWorker( void ) : cache( GUCEF_NULL ), index( 0 ), iterations( 0 ), fileCount( 0 ), blocksPerFile( 0 ), wrongBytes( 0 ), hits( 0 ), stores( 0 ), storeFailures( 0 ), noEntry( 0 ), done( 0 ), thread( GUCEF_NULL ) {}
};

/*-------------------------------------------------------------------------*/

UInt32
NextRandom( UInt32& state )
{
    state = state * 1664525U + 1013904223U;
    return state >> 8;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_CALLSPEC_PREFIX
WorkerMain( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    SWorker& worker = *static_cast< SWorker* >( data );
    UInt32 random = 12345 + worker.index * 7919;
    GUCEF::vector< UInt8 > block;
    GUCEF::vector< UInt8 > expected;
    block.resize( B );
    expected.resize( B );
    for ( UInt32 i=0; i<worker.iterations; ++i )
    {
        // every thread asks for the same files, so the same blocks are stored by several of them
        const UInt32 file = NextRandom( random ) % worker.fileCount;
        const UInt32 blockIndex = NextRandom( random ) % worker.blocksPerFile;
        const UInt64 fileSize = (UInt64) worker.blocksPerFile * B;
        const THandle h = worker.cache->AcquireEntry( "share", "file" + CORE::ToString( file ), "r1", fileSize, VFS::CVfsDigest(), true );
        if ( 0 == h )
        {
            worker.noEntry.FetchAdd( 1 );
            continue;
        }
        BlockCacheTest::FillPattern( 1000 + file, (UInt64) blockIndex * B, &expected[ 0 ], B );
        const TCache::EReadResult result = worker.cache->ReadBlock( h, blockIndex, 0, &block[ 0 ], B );
        if ( TCache::READ_HIT == result )
        {
            worker.hits.FetchAdd( 1 );
            if ( 0 != memcmp( &block[ 0 ], &expected[ 0 ], B ) )
                worker.wrongBytes.FetchAdd( 1 );
        }
        else
        {
            if ( worker.cache->StoreBlock( h, blockIndex, &expected[ 0 ], B ) )
                worker.stores.FetchAdd( 1 );
            else
                worker.storeFailures.FetchAdd( 1 );
        }
        worker.cache->ReleaseEntry( h );
    }
    worker.done.Store( 1 );
    return 0;
}

/*-------------------------------------------------------------------------*/

/** Runs the workers to the end; false when they did not finish in time (a deadlock) */
bool
RunWorkers( TCache& cache, UInt32 threadCount, UInt32 iterations, UInt32 fileCount, UInt32 blocksPerFile, UInt32& wrongBytes, UInt32& hits, UInt32& stores, UInt32& noEntry, UInt32& storeFailures )
{
    GUCEF::vector< SWorker* > workers;
    for ( UInt32 t=0; t<threadCount; ++t )
    {
        SWorker* worker = GUCEF_NEW SWorker();
        worker->cache = &cache;
        worker->index = t;
        worker->iterations = iterations;
        worker->fileCount = fileCount;
        worker->blocksPerFile = blocksPerFile;
        worker->thread = MT::ThreadDataReserve();
        workers.push_back( worker );
    }
    bool started = true;
    for ( UInt32 t=0; t<threadCount; ++t )
        started = ( 0 != MT::ThreadCreate( workers[ t ]->thread, &WorkerMain, workers[ t ] ) ) && started;

    bool finished = started;
    for ( UInt32 t=0; t<threadCount; ++t )
    {
        if ( started && 1 != MT::ThreadWait( workers[ t ]->thread, 120000 ) )
            finished = false;
    }

    wrongBytes = hits = stores = noEntry = storeFailures = 0;
    for ( UInt32 t=0; t<threadCount; ++t )
    {
        wrongBytes += workers[ t ]->wrongBytes.Load();
        hits += workers[ t ]->hits.Load();
        stores += workers[ t ]->stores.Load();
        noEntry += workers[ t ]->noEntry.Load();
        storeFailures += workers[ t ]->storeFailures.Load();
        if ( finished )
            MT::ThreadDataCleanup( workers[ t ]->thread );
        if ( finished )
            GUCEF_DELETE workers[ t ];
    }
    return finished;
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformVfsBlockCacheStressTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "VfsBlockCacheStress" );

    GUCEF_TESTFW_TESTCASE( "BlockCache: more entries held than files may be open (2000 entries, 64 open files): the limit is never passed, every read is right, and the process keeps its descriptors" )
    try
    {
        CBlockCacheTestFolder folder( "openfiles" );
        TCache cache;
        ASSERT_TRUE( cache.Open( folder.GetPath(), 0, B, 64 ) );
        const UInt32 descriptorsBefore = BlockCacheTest::CountOpenDescriptors();
        const UInt32 entryCount = 2000;
        GUCEF::vector< THandle > handles;
        UInt32 limitPassed = 0;
        UInt32 maxDescriptors = 0;
        for ( UInt32 e=0; e<entryCount; ++e )
        {
            const THandle h = cache.AcquireEntry( "share", "f" + CORE::ToString( e ), "r1", 100 + e % 50, VFS::CVfsDigest(), true );
            ASSERT_TRUE( 0 != h );
            handles.push_back( h );
            ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, h, e ) );
            if ( 0 == e % 20 )
            {
                if ( GetStats( cache ).openFiles > 64 )
                    ++limitPassed;
                const UInt32 descriptors = BlockCacheTest::CountOpenDescriptors();
                if ( descriptors > maxDescriptors )
                    maxDescriptors = descriptors;
            }
        }
        ASSERT_TRUE( 0 == limitPassed );
        ASSERT_TRUE( GetStats( cache ).openFiles <= 64 );

        // every one of them reads right, in another order, with the files being closed and opened again behind
        UInt32 wrong = 0;
        for ( UInt32 e=entryCount; e>0; --e )
        {
            if ( !BlockCacheTest::BlockMatches( cache, handles[ e - 1 ], e - 1, 0 ) )
                ++wrong;
            if ( 0 == e % 20 && GetStats( cache ).openFiles > 64 )
                ++limitPassed;
        }
        ASSERT_TRUE( 0 == wrong );
        ASSERT_TRUE( 0 == limitPassed );
        ASSERT_TRUE( GetStats( cache ).fileReopens > 0 );
        ASSERT_TRUE( GetStats( cache ).fileReopens > entryCount / 2 );

        // the descriptors of the process: the data files that may be open, plus a few of the moment
        if ( descriptorsBefore > 0 )
            ASSERT_TRUE( maxDescriptors <= descriptorsBefore + 64 + 8 );

        for ( UInt32 e=0; e<entryCount; ++e )
            cache.ReleaseEntry( handles[ e ] );
        ASSERT_TRUE( 0 == GetStats( cache ).openFiles );
        cache.Close();

        // after a restart they are all there
        ASSERT_TRUE( cache.Open( folder.GetPath(), 0, B, 64 ) );
        ASSERT_TRUE( entryCount == GetStats( cache ).entries );
        UInt32 missing = 0;
        for ( UInt32 e=0; e<entryCount; e += 37 )
        {
            const THandle h = cache.AcquireEntry( "share", "f" + CORE::ToString( e ), "r1", 100 + e % 50, VFS::CVfsDigest(), false );
            if ( 0 == h || !BlockCacheTest::BlockMatches( cache, h, e, 0 ) )
                ++missing;
            cache.ReleaseEntry( h );
        }
        ASSERT_TRUE( 0 == missing );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockCache: eight threads store and read the same and different blocks at once, and never see a wrong byte" )
    try
    {
        CBlockCacheTestFolder folder( "threads" );
        TCache cache;
        ASSERT_TRUE( cache.Open( folder.GetPath(), 0, B, 128 ) );
        UInt32 wrong = 0, hits = 0, stores = 0, noEntry = 0, storeFailures = 0;
        ASSERT_TRUE( RunWorkers( cache, 8, 600, 6, 8, wrong, hits, stores, noEntry, storeFailures ) );
        ASSERT_TRUE( 0 == wrong );
        ASSERT_TRUE( 0 == noEntry && 0 == storeFailures );
        ASSERT_TRUE( stores >= 6 * 8 && hits > 0 );
        // what the threads left is whole and right
        UInt32 present = 0;
        UInt32 bad = 0;
        for ( UInt32 f=0; f<6; ++f )
        {
            const THandle h = cache.AcquireEntry( "share", "file" + CORE::ToString( f ), "r1", 8 * (UInt64) B, VFS::CVfsDigest(), false );
            ASSERT_TRUE( 0 != h );
            for ( UInt32 b=0; b<8; ++b )
            {
                if ( cache.HasBlock( h, b ) )
                {
                    ++present;
                    if ( !BlockCacheTest::BlockMatches( cache, h, 1000 + f, b ) )
                        ++bad;
                }
            }
            cache.ReleaseEntry( h );
        }
        ASSERT_TRUE( 0 == bad && present > 0 );
        ASSERT_TRUE( cache.GetUsedBytes() == present * (UInt64) B );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockCache: the cap holds under eight threads that fill more than it, and the open file limit of 3 with eight threads stalls none of them" )
    try
    {
        CBlockCacheTestFolder folder( "threadscap" );
        TCache cache;
        const UInt64 cap = 10 * (UInt64) B;
        ASSERT_TRUE( cache.Open( folder.GetPath(), cap, B, 3 ) );
        UInt32 wrong = 0, hits = 0, stores = 0, noEntry = 0, storeFailures = 0;
        ASSERT_TRUE( RunWorkers( cache, 8, 300, 12, 4, wrong, hits, stores, noEntry, storeFailures ) );
        ASSERT_TRUE( 0 == wrong );
        ASSERT_TRUE( 0 == storeFailures );
        ASSERT_TRUE( stores > 0 );
        // nothing is held any more: the cap is the cap
        ASSERT_TRUE( cache.GetUsedBytes() <= cap );
        ASSERT_TRUE( GetStats( cache ).evictedEntries > 0 );
        ASSERT_TRUE( GetStats( cache ).openFiles == 0 );
        // the bytes counted are the bytes of the blocks that are held
        UInt64 counted = 0;
        for ( UInt32 f=0; f<12; ++f )
        {
            const THandle h = cache.AcquireEntry( "share", "file" + CORE::ToString( f ), "r1", 4 * (UInt64) B, VFS::CVfsDigest(), false );
            if ( 0 != h )
            {
                counted += (UInt64) cache.GetBlocksHeld( h ) * B;
                cache.ReleaseEntry( h );
            }
        }
        ASSERT_TRUE( counted == cache.GetUsedBytes() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockCache: a file above 4 GiB is cached at its own offsets (64 bit), sparse, and read back after a restart" )
    CBlockCacheTestFolder folder( "large" );
    GUCEF_TESTFW_SKIP_IF( !BlockCacheTest::IsFolderSparse( folder.GetPath() ), "the scratch folder does not keep files sparse (or this platform was not looked at): a file of 5 GiB is not written for a test" );
    try
    {
        const UInt64 fileSize = 5 * 1024ULL * 1024ULL * 1024ULL + 123;
        const UInt32 blockCount = (UInt32) ( ( fileSize + B - 1 ) / B );
        const UInt32 atFourGiB = (UInt32) ( 4294967296ULL / B );
        ASSERT_TRUE( 81921 == blockCount );
        {
            TCache cache;
            ASSERT_TRUE( cache.Open( folder.GetPath(), 0, B, 128 ) );
            const THandle h = cache.AcquireEntry( "share", "huge.bin", "r1", fileSize, VFS::CVfsDigest(), true );
            ASSERT_TRUE( 0 != h );
            ASSERT_TRUE( blockCount == cache.GetBlockCount( h ) && fileSize == cache.GetFileSize( h ) );
            ASSERT_TRUE( 123 == cache.GetBlockSize( h, blockCount - 1 ) );
            const UInt32 indexes[] = { 0, atFourGiB - 1, atFourGiB, atFourGiB + 1, blockCount - 1 };
            for ( UInt32 i=0; i<5; ++i )
                ASSERT_TRUE( BlockCacheTest::StoreBlocks( cache, h, 220, indexes[ i ], 1 ) );
            for ( UInt32 i=0; i<5; ++i )
                ASSERT_TRUE( BlockCacheTest::BlockMatches( cache, h, 220, indexes[ i ] ) );
            ASSERT_TRUE( 5 == cache.GetBlocksHeld( h ) );
            ASSERT_FALSE( cache.HasBlock( h, atFourGiB + 2 ) && cache.HasBlock( h, 1 ) );
            ASSERT_TRUE( 4 * (UInt64) B + 123 == cache.GetUsedBytes() );
            // the bytes just below and above 4 GiB
            UInt8 below = 0;
            UInt8 above = 0;
            UInt8 expected = 0;
            ASSERT_TRUE( TCache::READ_HIT == cache.ReadBlock( h, atFourGiB - 1, B - 1, &below, 1 ) );
            BlockCacheTest::FillPattern( 220, 4294967295ULL, &expected, 1 );
            ASSERT_TRUE( below == expected );
            ASSERT_TRUE( TCache::READ_HIT == cache.ReadBlock( h, atFourGiB, 0, &above, 1 ) );
            BlockCacheTest::FillPattern( 220, 4294967296ULL, &expected, 1 );
            ASSERT_TRUE( above == expected );
            cache.ReleaseEntry( h );
        }
        const CORE::CString::StringVector datas = BlockCacheTest::ListEntryFiles( folder.GetPath(), ".vbd" );
        ASSERT_TRUE( 1 == datas.size() );
        ASSERT_TRUE( CORE::FileSize( datas[ 0 ] ) == fileSize );
        {
            TCache cache;
            ASSERT_TRUE( cache.Open( folder.GetPath(), 0, B, 128 ) );
            ASSERT_TRUE( 4 * (UInt64) B + 123 == cache.GetUsedBytes() );
            const THandle h = cache.AcquireEntry( "share", "huge.bin", "r1", fileSize, VFS::CVfsDigest(), false );
            ASSERT_TRUE( 0 != h );
            ASSERT_TRUE( cache.HasBlock( h, atFourGiB ) && cache.HasBlock( h, blockCount - 1 ) && !cache.HasBlock( h, atFourGiB + 2 ) );
            ASSERT_TRUE( BlockCacheTest::BlockMatches( cache, h, 220, atFourGiB + 1 ) && BlockCacheTest::BlockMatches( cache, h, 220, blockCount - 1 ) );
            cache.ReleaseEntry( h );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockCache: a cap smaller than one block, or than the entry that is filled, loses nothing the reader holds and leaves no entry behind once it is released" )
    try
    {
        CBlockCacheTestFolder folder( "tinycap" );
        TCache cache;
        ASSERT_TRUE( cache.Open( folder.GetPath(), 1, B, 8 ) );
        for ( UInt32 round = 0; round < 5; ++round )
        {
            const THandle h = cache.AcquireEntry( "share", "f" + CORE::ToString( round % 2 ), "r1", 4 * (UInt64) B, VFS::CVfsDigest(), true );
            ASSERT_TRUE( 0 != h );
            ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, h, 230 + round % 2 ) );
            ASSERT_TRUE( cache.IsComplete( h ) );
            for ( UInt32 b=0; b<4; ++b )
                ASSERT_TRUE( BlockCacheTest::BlockMatches( cache, h, 230 + round % 2, b ) );
            ASSERT_TRUE( 4 * (UInt64) B == cache.GetUsedBytes() );      // held: the cap does not take it
            cache.ReleaseEntry( h );
            ASSERT_TRUE( 0 == cache.GetUsedBytes() );
            ASSERT_TRUE( 0 == GetStats( cache ).entries );
            ASSERT_TRUE( 0 == BlockCacheTest::ListEntryFiles( folder.GetPath(), ".vbi" ).size() );
        }
        ASSERT_TRUE( 5 == GetStats( cache ).evictedEntries );

        // a cap of one block and a file of three blocks: the pin is refused with its own result, the file stays as any other entry, then a limit that fits takes the pin
        cache.SetMaxBytes( B );
        const THandle p = cache.AcquireEntry( "share", "pinned", "r1", 3 * (UInt64) B, VFS::CVfsDigest(), true );
        ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, p, 240 ) );
        ASSERT_TRUE( TCache::PIN_OVER_LIMIT == cache.TryPin( "share", "pinned" ) );
        ASSERT_FALSE( cache.SetPinned( "share", "pinned", true ) );
        ASSERT_TRUE( 2 == GetStats( cache ).pinsRefused );
        cache.SetMaxBytes( 3 * (UInt64) B );
        ASSERT_TRUE( cache.SetPinned( "share", "pinned", true ) );
        cache.SetMaxBytes( B );                                 // a limit lowered below what the pins hold leaves the pins as they are
        ASSERT_TRUE( cache.IsPinned( "share", "pinned" ) );
        cache.ReleaseEntry( p );
        ASSERT_TRUE( 3 * (UInt64) B == cache.GetUsedBytes() );
        const THandle q = cache.AcquireEntry( "share", "q", "r1", (UInt64) B, VFS::CVfsDigest(), true );
        ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, q, 241 ) );
        cache.ReleaseEntry( q );
        ASSERT_TRUE( 3 * (UInt64) B == cache.GetUsedBytes() );
        ASSERT_TRUE( 0 == cache.AcquireEntry( "share", "q", "r1", (UInt64) B, VFS::CVfsDigest(), false ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformVfsBlockCacheGlobalTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "VfsBlockCacheGlobal" );

    GUCEF_TESTFW_TESTCASE( "CVfsGlobal: the same folder gives the same cache, another folder another one, the folder is closed when the last user lets go, and there is no cache until one is asked for" )
    try
    {
        CBlockCacheTestFolder folderA( "globala" );
        CBlockCacheTestFolder folderB( "globalb" );
        VFS::CVfsGlobal* global = VFS::CVfsGlobal::Instance();
        ASSERT_TRUE( global->GetDefaultBlockCache().IsNULL() );
        ASSERT_TRUE( global->AcquireBlockCache( CORE::CString::Empty, 0 ).IsNULL() );

        {
            VFS::CVfsBlockCachePtr first = global->AcquireBlockCache( folderA.GetPath(), 100 * (UInt64) B, B );
            ASSERT_FALSE( first.IsNULL() );
            ASSERT_TRUE( first->IsOpen() && 100 * (UInt64) B == first->GetMaxBytes() );
            VFS::CVfsBlockCachePtr second = global->AcquireBlockCache( folderA.GetPath() + "/", 5, 2 * B );
            ASSERT_FALSE( second.IsNULL() );
            ASSERT_TRUE( first.GetPointerAlways() == second.GetPointerAlways() );
            ASSERT_TRUE( 100 * (UInt64) B == second->GetMaxBytes() && B == second->GetBlockBytes() );          // the first caller's settings
            VFS::CVfsBlockCachePtr other = global->AcquireBlockCache( folderB.GetPath(), 0, B );
            ASSERT_FALSE( other.IsNULL() );
            ASSERT_TRUE( other.GetPointerAlways() != first.GetPointerAlways() );

            // it is a cache like any: what is stored through one is there through the other
            const THandle h = first->AcquireEntry( "share", "f", "r1", 2 * (UInt64) B, VFS::CVfsDigest(), true );
            ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( *first, h, 250 ) );
            first->ReleaseEntry( h );
            const THandle again = second->AcquireEntry( "share", "f", "r1", 2 * (UInt64) B, VFS::CVfsDigest(), false );
            ASSERT_TRUE( 0 != again && BlockCacheTest::BlockMatches( *second, again, 250, 1 ) );
            second->ReleaseEntry( again );
            ASSERT_TRUE( CORE::FileExists( folderA.GetChildPath( "cache.lock" ) ) );

            // nobody else may have the folder of the registry
            TCache outsider;
            ASSERT_FALSE( outsider.Open( folderA.GetPath(), 0, B ) );
        }
        // the last pointers are gone: the folders are closed
        ASSERT_FALSE( CORE::FileExists( folderA.GetChildPath( "cache.lock" ) ) );
        ASSERT_FALSE( CORE::FileExists( folderB.GetChildPath( "cache.lock" ) ) );

        // and can be opened again, with what was kept
        VFS::CVfsBlockCachePtr reopened = global->AcquireBlockCache( folderA.GetPath(), 0, B );
        ASSERT_FALSE( reopened.IsNULL() );
        const THandle h = reopened->AcquireEntry( "share", "f", "r1", 2 * (UInt64) B, VFS::CVfsDigest(), false );
        ASSERT_TRUE( 0 != h && BlockCacheTest::BlockMatches( *reopened, h, 250, 0 ) );
        reopened->ReleaseEntry( h );

        // a folder that another cache has is not given out
        TCache holder;
        ASSERT_TRUE( holder.Open( folderB.GetPath(), 0, B ) );
        ASSERT_TRUE( global->AcquireBlockCache( folderB.GetPath(), 0, B ).IsNULL() );

        // the default cache of the application
        global->SetDefaultBlockCache( reopened );
        ASSERT_TRUE( global->GetDefaultBlockCache().GetPointerAlways() == reopened.GetPointerAlways() );
        global->SetDefaultBlockCache( VFS::CVfsBlockCachePtr() );
        ASSERT_TRUE( global->GetDefaultBlockCache().IsNULL() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "VfsConfig: the BlockCache section of the VFS config opens the default cache, is saved back as it was, and no section means no cache" )
    try
    {
        CBlockCacheTestFolder folder( "configcache" );
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        ASSERT_TRUE( VFS::CVfsGlobal::Instance()->GetDefaultBlockCache().IsNULL() );

        CORE::CDataNode config( "Config" );
        CORE::CDataNode* vfsNode = config.Structure( "VFS", '/' );
        ASSERT_TRUE( GUCEF_NULL != vfsNode );
        CORE::CDataNode* section = vfsNode->AddChild( "BlockCache" );
        ASSERT_TRUE( GUCEF_NULL != section );
        section->SetAttribute( "folder", folder.GetPath() );
        section->SetAttribute( "maxBytes", (UInt64) ( 20 * (UInt64) B ) );
        section->SetAttribute( "blockBytes", (UInt32) ( 2 * B ) );
        section->SetAttribute( "maxOpenFiles", (UInt32) 33 );
        ASSERT_TRUE( vfs.LoadConfig( config ) );

        VFS::CVfsBlockCachePtr cache = VFS::CVfsGlobal::Instance()->GetDefaultBlockCache();
        ASSERT_FALSE( cache.IsNULL() );
        ASSERT_TRUE( cache->IsOpen() && 20 * (UInt64) B == cache->GetMaxBytes() && 2 * B == cache->GetBlockBytes() && 33 == cache->GetMaxOpenFiles() );
        ASSERT_TRUE( CORE::FileExists( folder.GetChildPath( "cache.lock" ) ) );

        CORE::CDataNode saved( "Config" );
        ASSERT_TRUE( vfs.SaveConfig( saved ) );
        const CORE::CDataNode* savedVfs = saved.FindChild( "VFS" );
        const CORE::CDataNode* savedSection = GUCEF_NULL != savedVfs ? savedVfs->FindChild( "BlockCache" ) : GUCEF_NULL;
        ASSERT_TRUE( GUCEF_NULL != savedSection );
        if ( GUCEF_NULL != savedSection )
        {
            ASSERT_TRUE( savedSection->GetAttributeValueOrChildValueByName( "folder" ).AsString() == folder.GetPath() );
            ASSERT_TRUE( 20 * (UInt64) B == savedSection->GetAttributeValueOrChildValueByName( "maxBytes" ).AsUInt64() );
            ASSERT_TRUE( 2 * B == savedSection->GetAttributeValueOrChildValueByName( "blockBytes" ).AsUInt32() );
            ASSERT_TRUE( 33 == savedSection->GetAttributeValueOrChildValueByName( "maxOpenFiles" ).AsUInt32() );
        }

        // a section without a folder is no cache; the old one goes when it is let go
        CORE::CDataNode noCache( "Config" );
        CORE::CDataNode* emptySection = noCache.Structure( "VFS", '/' )->AddChild( "BlockCache" );
        emptySection->SetAttribute( "folder", "" );
        ASSERT_TRUE( vfs.LoadConfig( noCache ) );
        CORE::CDataNode savedAgain( "Config" );
        ASSERT_TRUE( vfs.SaveConfig( savedAgain ) );
        const CORE::CDataNode* againVfs = savedAgain.FindChild( "VFS" );
        ASSERT_TRUE( GUCEF_NULL != againVfs && GUCEF_NULL == againVfs->FindChild( "BlockCache" ) );

        cache.Unlink();
        VFS::CVfsGlobal::Instance()->SetDefaultBlockCache( VFS::CVfsBlockCachePtr() );
        ASSERT_FALSE( CORE::FileExists( folder.GetChildPath( "cache.lock" ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
