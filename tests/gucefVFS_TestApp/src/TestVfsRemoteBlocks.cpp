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

#ifndef GUCEF_MT_H
#include "gucefMT.h"
#define GUCEF_MT_H
#endif /* GUCEF_MT_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestThreads.h"
#include "TestVfsBlockCacheUtils.h"

#include "TestVfsRemoteBlocks.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** A source of the pattern of a seed: the bytes of a file that is never on a disk */
class CPatternSource : public VFS::CIVfsBlockSource
{
    public:

    UInt32 seed;
    UInt64 cutAfterBytes;           /**< the next call delivers this many bytes and then fails; (UInt64)-1 for no cut */
    MT::CTAtomic< UInt32 > calls;

    explicit CPatternSource( UInt32 patternSeed ) : VFS::CIVfsBlockSource(), seed( patternSeed ), cutAfterBytes( (UInt64) -1 ), calls( 0 ) {}

    virtual VFS::CVfsCallResult FetchRange( UInt64 offset, UInt32 length, void* buffer, UInt32& bytesRead ) GUCEF_VIRTUAL_OVERRIDE
    {
        calls.FetchAdd( 1 );
        UInt32 deliver = length;
        bool cut = false;
        if ( cutAfterBytes < deliver )
        {
            deliver = (UInt32) cutAfterBytes;
            cut = true;
            cutAfterBytes = (UInt64) -1;
        }
        BlockCacheTest::FillPattern( seed, offset, buffer, deliver );
        bytesRead = deliver;
        return cut ? VFS::CVfsCallResult( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING, "cut" ) : VFS::CVfsCallResult();
    }
};

VFS::CVfsRemoteEntry
MakeEntry( const char* name, bool isDirectory, UInt64 size, const char* revision )
{
    VFS::CVfsRemoteEntry entry;
    entry.name = name;
    entry.exists = true;
    entry.isDirectory = isDirectory;
    entry.size = size;
    entry.revision = revision;
    return entry;
}

/** One thread records a failure, reads it back, and clears it */
class CLastResultsThreads : public CTestThreadBody
{
    public:

    VFS::CVfsLastResults* results;
    MT::CTAtomic< UInt32 > wrong;
    explicit CLastResultsThreads( VFS::CVfsLastResults* r ) : results( r ), wrong( 0 ) {}

    virtual void Run( UInt32 index ) GUCEF_VIRTUAL_OVERRIDE
    {
        const CORE::TFileSystemAccessResult mine = ( index % 2 ) ? CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE : CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING;
        for ( UInt32 i=0; i<2000; ++i )
        {
            results->Set( VFS::CVfsCallResult( mine, "x" ) );
            if ( results->Get().result != mine )
                wrong.FetchAdd( 1 );
            results->Clear();
            if ( !results->Get().IsSuccess() )
                wrong.FetchAdd( 1 );
        }
    }
};

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformVfsRemoteBlocksTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "VfsRemoteBlocks" );

    GUCEF_TESTFW_TESTCASE( "RemotePath: paths are normalised, cut at the last separator (also with non-ASCII names) and joined" )
    try
    {
        ASSERT_TRUE( CORE::CString( "a/b/c" ) == VFS::CVfsRemotePath::Normalize( "/a\\b//c/" ) );
        ASSERT_TRUE( CORE::CString::Empty == VFS::CVfsRemotePath::Normalize( "///" ) );
        ASSERT_TRUE( CORE::CString::Empty == VFS::CVfsRemotePath::Normalize( CORE::CString::Empty ) );
        ASSERT_TRUE( CORE::CString( "a/b" ) == VFS::CVfsRemotePath::ParentOf( "a/b/c" ) );
        ASSERT_TRUE( CORE::CString::Empty == VFS::CVfsRemotePath::ParentOf( "a" ) );
        ASSERT_TRUE( CORE::CString::Empty == VFS::CVfsRemotePath::ParentOf( CORE::CString::Empty ) );
        ASSERT_TRUE( CORE::CString( "c" ) == VFS::CVfsRemotePath::LeafOf( "a/b/c" ) );
        ASSERT_TRUE( CORE::CString( "a" ) == VFS::CVfsRemotePath::LeafOf( "a" ) );
        ASSERT_TRUE( CORE::CString( "a/b" ) == VFS::CVfsRemotePath::Join( "a", "b" ) );
        ASSERT_TRUE( CORE::CString( "b" ) == VFS::CVfsRemotePath::Join( CORE::CString::Empty, "b" ) );
        ASSERT_TRUE( CORE::CString( "a" ) == VFS::CVfsRemotePath::Join( "a", CORE::CString::Empty ) );

        const CORE::CString utf8 = "d\xC3\xA9j\xC3\xA0/vu \xE2\x82\xAC/file";
        ASSERT_TRUE( CORE::CString( "file" ) == VFS::CVfsRemotePath::LeafOf( utf8 ) );
        ASSERT_TRUE( CORE::CString( "d\xC3\xA9j\xC3\xA0/vu \xE2\x82\xAC" ) == VFS::CVfsRemotePath::ParentOf( utf8 ) );
        ASSERT_TRUE( utf8 == VFS::CVfsRemotePath::Normalize( "/" + utf8 + "/" ) );

        ASSERT_TRUE( VFS::CVfsRemotePath::IsSameOrBelow( CORE::CString::Empty, "a/b" ) );
        ASSERT_TRUE( VFS::CVfsRemotePath::IsSameOrBelow( "a", "a" ) );
        ASSERT_TRUE( VFS::CVfsRemotePath::IsSameOrBelow( "a", "a/b" ) );
        ASSERT_FALSE( VFS::CVfsRemotePath::IsSameOrBelow( "a", "ab/b" ) );
        ASSERT_FALSE( VFS::CVfsRemotePath::IsSameOrBelow( "a/b", "a" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LastResults: the result is per thread, a success removes it, and clearing costs nothing while no thread has one" )
    try
    {
        VFS::CVfsLastResults results;
        ASSERT_TRUE( results.Get().IsSuccess() );
        ASSERT_TRUE( 0 == results.GetEntryCount() );
        results.Clear();
        results.Set( VFS::CVfsCallResult() );
        ASSERT_TRUE( 0 == results.GetEntryCount() );

        results.Set( VFS::CVfsCallResult( CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE, "wait", true ) );
        ASSERT_TRUE( 1 == results.GetEntryCount() );
        VFS::CVfsCallResult got = results.Get();
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE == got.result && got.fetchStarted && got.message == "wait" );

        results.Set( VFS::CVfsCallResult() );
        ASSERT_TRUE( results.Get().IsSuccess() );
        ASSERT_TRUE( 0 == results.GetEntryCount() );

        CLastResultsThreads body( &results );
        CTestThreads threads( body, 4 );
        ASSERT_TRUE( threads.Start() );
        ASSERT_TRUE( threads.Join( 60000 ) );
        ASSERT_TRUE( 0 == body.wrong.Load() );
        ASSERT_TRUE( 0 == results.GetEntryCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RemoteMetadataCache: an entry is fresh within the time to live, stale after it, unknown before it was stored, and never fresh with a time to live of 0" )
    try
    {
        VFS::CVfsRemoteMetadataCache cache( 150, 100 );
        VFS::CVfsRemoteEntry entry;
        ASSERT_TRUE( VFS::CVfsRemoteMetadataCache::STATE_UNKNOWN == cache.FindEntry( "a/b.txt", entry ) );
        ASSERT_TRUE( cache.StoreEntry( "a/b.txt", MakeEntry( "b.txt", false, 10, "10-0" ) ) );
        ASSERT_TRUE( VFS::CVfsRemoteMetadataCache::STATE_FRESH == cache.FindEntry( "a/b.txt", entry ) );
        ASSERT_TRUE( entry.exists && 10 == entry.size && CORE::CString( "10-0" ) == entry.revision );
        MT::ThreadDelay( 300 );
        ASSERT_TRUE( VFS::CVfsRemoteMetadataCache::STATE_STALE == cache.FindEntry( "a/b.txt", entry ) );
        ASSERT_TRUE( 10 == entry.size );

        VFS::CVfsRemoteMetadataCache never( 0, 100 );
        never.StoreEntry( "x", MakeEntry( "x", false, 1, "1-0" ) );
        ASSERT_TRUE( VFS::CVfsRemoteMetadataCache::STATE_STALE == never.FindEntry( "x", entry ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RemoteMetadataCache: storing an entry says whether it is new or changed and gives the one that was held; an unchanged one says no" )
    try
    {
        VFS::CVfsRemoteMetadataCache cache( 10000, 100 );
        VFS::CVfsRemoteEntry previous;
        bool had = true;
        ASSERT_TRUE( cache.StoreEntry( "f", MakeEntry( "f", false, 5, "5-0" ), &previous, &had ) );
        ASSERT_FALSE( had );
        ASSERT_FALSE( cache.StoreEntry( "f", MakeEntry( "f", false, 5, "5-0" ), &previous, &had ) );
        ASSERT_TRUE( had && CORE::CString( "5-0" ) == previous.revision );
        ASSERT_TRUE( cache.StoreEntry( "f", MakeEntry( "f", false, 5, "5-1" ), &previous, &had ) );
        ASSERT_TRUE( had && CORE::CString( "5-0" ) == previous.revision );

        // the absence of a path is stored too
        VFS::CVfsRemoteEntry absent;
        ASSERT_TRUE( cache.StoreEntry( "gone", absent ) );
        VFS::CVfsRemoteEntry found;
        ASSERT_TRUE( VFS::CVfsRemoteMetadataCache::STATE_FRESH == cache.FindEntry( "gone", found ) );
        ASSERT_FALSE( found.exists );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RemoteMetadataCache: a listing answers for its entries and for a name that is not in it, the newer word wins between a record and a listing, and a listing says whether it changed" )
    try
    {
        VFS::CVfsRemoteMetadataCache cache( 10000, 100 );
        VFS::CVfsRemoteEntry::TVector entries;
        entries.push_back( MakeEntry( "one.bin", false, 100, "100-0" ) );
        entries.push_back( MakeEntry( "sub", true, 0, "" ) );
        ASSERT_TRUE( cache.StoreListing( "dir", entries, true ) );
        ASSERT_FALSE( cache.StoreListing( "dir", entries, true ) );

        VFS::CVfsRemoteEntry found;
        ASSERT_TRUE( VFS::CVfsRemoteMetadataCache::STATE_FRESH == cache.FindEntry( "dir/one.bin", found ) );
        ASSERT_TRUE( found.exists && !found.isDirectory && 100 == found.size );
        ASSERT_TRUE( VFS::CVfsRemoteMetadataCache::STATE_FRESH == cache.FindEntry( "dir/sub", found ) );
        ASSERT_TRUE( found.exists && found.isDirectory );
        ASSERT_TRUE( VFS::CVfsRemoteMetadataCache::STATE_FRESH == cache.FindEntry( "dir/none", found ) );
        ASSERT_FALSE( found.exists );
        // the directory has a listing, so it is there
        ASSERT_TRUE( VFS::CVfsRemoteMetadataCache::STATE_FRESH == cache.FindEntry( "dir", found ) );
        ASSERT_TRUE( found.exists && found.isDirectory );

        // a record fetched after the listing is the newer word
        MT::ThreadDelay( 5 );
        cache.StoreEntry( "dir/one.bin", MakeEntry( "one.bin", false, 200, "200-1" ) );
        ASSERT_TRUE( VFS::CVfsRemoteMetadataCache::STATE_FRESH == cache.FindEntry( "dir/one.bin", found ) );
        ASSERT_TRUE( 200 == found.size );

        // a listing fetched after that is newer again, and takes the record away
        MT::ThreadDelay( 5 );
        entries[ 0 ] = MakeEntry( "one.bin", false, 300, "300-2" );
        ASSERT_TRUE( cache.StoreListing( "dir", entries, true ) );
        ASSERT_TRUE( VFS::CVfsRemoteMetadataCache::STATE_FRESH == cache.FindEntry( "dir/one.bin", found ) );
        ASSERT_TRUE( 300 == found.size );

        // a directory that is not there
        VFS::CVfsRemoteEntry::TVector none;
        cache.StoreListing( "nodir", none, false );
        bool dirExists = true;
        VFS::CVfsRemoteEntry::TVector read;
        ASSERT_TRUE( VFS::CVfsRemoteMetadataCache::STATE_FRESH == cache.FindListing( "nodir", read, dirExists ) );
        ASSERT_FALSE( dirExists );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RemoteMetadataCache: invalidating a path forgets its record, its listing and the listing of its parent; Clear forgets everything" )
    try
    {
        VFS::CVfsRemoteMetadataCache cache( 10000, 100 );
        VFS::CVfsRemoteEntry::TVector entries;
        entries.push_back( MakeEntry( "f", false, 1, "1-0" ) );
        cache.StoreListing( "d", entries, true );
        cache.StoreListing( "d/e", entries, true );
        cache.StoreListing( "other", entries, true );
        cache.StoreEntry( "d/e/f", MakeEntry( "f", false, 1, "1-0" ) );

        cache.Invalidate( "d/e/f" );
        VFS::CVfsRemoteEntry found;
        // the record and the listing of the parent are gone, the listing of the grandparent is not
        ASSERT_TRUE( VFS::CVfsRemoteMetadataCache::STATE_UNKNOWN == cache.GetState( "d/e/f" ) );
        VFS::CVfsRemoteEntry::TVector read;
        bool dirExists = false;
        ASSERT_TRUE( VFS::CVfsRemoteMetadataCache::STATE_UNKNOWN == cache.FindListing( "d/e", read, dirExists ) );
        ASSERT_TRUE( VFS::CVfsRemoteMetadataCache::STATE_FRESH == cache.FindListing( "d", read, dirExists ) );
        ASSERT_TRUE( VFS::CVfsRemoteMetadataCache::STATE_FRESH == cache.FindListing( "other", read, dirExists ) );

        cache.Invalidate( "d" );
        ASSERT_TRUE( VFS::CVfsRemoteMetadataCache::STATE_UNKNOWN == cache.FindListing( "d", read, dirExists ) );

        cache.Clear();
        ASSERT_TRUE( 0 == cache.GetEntryCount() );
        ASSERT_TRUE( VFS::CVfsRemoteMetadataCache::STATE_UNKNOWN == cache.FindListing( "other", read, dirExists ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RemoteMetadataCache: the number of records and listings is bounded and the oldest go first" )
    try
    {
        VFS::CVfsRemoteMetadataCache cache( 10000, 10 );
        for ( UInt32 i=0; i<30; ++i )
        {
            cache.StoreEntry( "f" + CORE::ToString( i ), MakeEntry( "f", false, i, "r" ) );
            MT::ThreadDelay( 1 );
            ASSERT_TRUE( cache.GetEntryCount() <= 10 );
        }
        ASSERT_TRUE( 10 == cache.GetEntryCount() );
        VFS::CVfsRemoteEntry found;
        ASSERT_TRUE( VFS::CVfsRemoteMetadataCache::STATE_FRESH == cache.FindEntry( "f29", found ) );
        ASSERT_TRUE( VFS::CVfsRemoteMetadataCache::STATE_UNKNOWN == cache.FindEntry( "f0", found ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockRunFetcher: a run is asked of the source in one call and every block that arrived whole is stored; the last block of a file is shorter" )
    try
    {
        CBlockCacheTestFolder folder( "runfetch1" );
        VFS::CVfsBlockCache cache;
        const UInt32 B = BlockCacheTest::BlockBytes;
        ASSERT_TRUE( cache.Open( folder.GetPath(), 0, B ) );
        const UInt64 size = 3 * (UInt64) B + 100;
        const VFS::CVfsBlockCache::TEntryHandle entry = cache.AcquireEntry( "share", "f", "r", size, VFS::CVfsDigest(), true );
        ASSERT_TRUE( 0 != entry );
        CPatternSource source( 77 );
        VFS::CVfsRemoteCounters counters;
        VFS::CVfsBlockRunFetcher::SRun run;
        // asks for five blocks, the file has four: the run ends at the end of the file
        ASSERT_TRUE( VFS::CVfsBlockRunFetcher::FetchRun( &cache, entry, B, size, source, 0, 5, &counters, run ).IsSuccess() );
        ASSERT_TRUE( 1 == source.calls.Load() );
        ASSERT_TRUE( 4 == run.blocksStored && size == run.bytesStored && 0 == run.blocksNotKept );
        ASSERT_TRUE( size == run.data.GetDataSize() && 0 == run.firstByte );
        ASSERT_TRUE( cache.IsComplete( entry ) );
        for ( UInt32 b=0; b<4; ++b )
        {
            ASSERT_TRUE( BlockCacheTest::BlockMatches( cache, entry, 77, b ) || 3 == b );
        }
        ASSERT_TRUE( 1 == counters.fetchCalls.Load() && size == counters.bytesFetched.Load() );
        cache.ReleaseEntry( entry );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockRunFetcher: a run that is cut keeps the blocks that arrived whole and drops the partial one; blocks that are held are not stored again" )
    try
    {
        CBlockCacheTestFolder folder( "runfetch2" );
        VFS::CVfsBlockCache cache;
        const UInt32 B = BlockCacheTest::BlockBytes;
        ASSERT_TRUE( cache.Open( folder.GetPath(), 0, B ) );
        const UInt64 size = 6 * (UInt64) B;
        const VFS::CVfsBlockCache::TEntryHandle entry = cache.AcquireEntry( "share", "f", "r", size, VFS::CVfsDigest(), true );
        CPatternSource source( 5 );
        VFS::CVfsBlockRunFetcher::SRun run;

        source.cutAfterBytes = 2 * (UInt64) B + 1000;
        VFS::CVfsCallResult result = VFS::CVfsBlockRunFetcher::FetchRun( &cache, entry, B, size, source, 1, 4, GUCEF_NULL, run );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == result.result );
        ASSERT_TRUE( 2 == run.blocksStored );
        ASSERT_TRUE( !cache.HasBlock( entry, 0 ) && cache.HasBlock( entry, 1 ) && cache.HasBlock( entry, 2 ) && !cache.HasBlock( entry, 3 ) );
        ASSERT_TRUE( 2 * (UInt64) B + 1000 == run.data.GetDataSize() );

        // the run that goes on: the blocks held are left alone
        result = VFS::CVfsBlockRunFetcher::FetchRun( &cache, entry, B, size, source, 0, 6, GUCEF_NULL, run );
        ASSERT_TRUE( result.IsSuccess() );
        ASSERT_TRUE( 4 == run.blocksStored );
        ASSERT_TRUE( cache.IsComplete( entry ) );
        for ( UInt32 b=0; b<6; ++b )
        {
            ASSERT_TRUE( BlockCacheTest::BlockMatches( cache, entry, 5, b ) );
        }
        cache.ReleaseEntry( entry );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockRunFetcher: without an entry nothing is stored and the bytes are given back; a run past the end of the file asks nothing" )
    try
    {
        CBlockCacheTestFolder folder( "runfetch3" );
        VFS::CVfsBlockCache cache;
        const UInt32 B = BlockCacheTest::BlockBytes;
        ASSERT_TRUE( cache.Open( folder.GetPath(), 0, B ) );
        CPatternSource source( 9 );
        VFS::CVfsBlockRunFetcher::SRun run;
        ASSERT_TRUE( VFS::CVfsBlockRunFetcher::FetchRun( &cache, 0, B, 2 * (UInt64) B, source, 1, 1, GUCEF_NULL, run ).IsSuccess() );
        ASSERT_TRUE( 0 == run.blocksStored && B == run.data.GetDataSize() && B == run.firstByte );
        ASSERT_TRUE( 1 == source.calls.Load() );

        ASSERT_TRUE( VFS::CVfsBlockRunFetcher::FetchRun( &cache, 0, B, 2 * (UInt64) B, source, 5, 1, GUCEF_NULL, run ).IsSuccess() );
        ASSERT_TRUE( 1 == source.calls.Load() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BlockCache with non-ASCII names: two paths that differ in their last character after a multi-byte one are two entries, and an entry with such a name is found again after the cache was closed and opened" )
    try
    {
        const CORE::CString pathA = "d\xC3\xA9/na\xC3\xAFve_a";
        const CORE::CString pathB = "d\xC3\xA9/na\xC3\xAFve_b";
        ASSERT_TRUE( VFS::CVfsBlockCache::MakeKey( "share", pathA, "r1" ) != VFS::CVfsBlockCache::MakeKey( "share", pathB, "r1" ) );
        ASSERT_TRUE( VFS::CVfsBlockCache::MakeKey( "sh\xC3\xA9re", "p", "r1" ) != VFS::CVfsBlockCache::MakeKey( "sh\xC3\xA9rf", "p", "r1" ) );

        CBlockCacheTestFolder folder( "nonascii" );
        const UInt32 B = BlockCacheTest::BlockBytes;
        {
            VFS::CVfsBlockCache cache;
            ASSERT_TRUE( cache.Open( folder.GetPath(), 0, B ) );
            const VFS::CVfsBlockCache::TEntryHandle a = cache.AcquireEntry( "share", pathA, "r1", B, VFS::CVfsDigest(), true );
            const VFS::CVfsBlockCache::TEntryHandle b = cache.AcquireEntry( "share", pathB, "r1", B, VFS::CVfsDigest(), true );
            ASSERT_TRUE( 0 != a && 0 != b && a != b );
            ASSERT_TRUE( BlockCacheTest::StoreAllBlocks( cache, a, 11 ) && BlockCacheTest::StoreAllBlocks( cache, b, 22 ) );
            ASSERT_TRUE( BlockCacheTest::BlockMatches( cache, a, 11, 0 ) && BlockCacheTest::BlockMatches( cache, b, 22, 0 ) );
            cache.ReleaseEntry( a );
            cache.ReleaseEntry( b );
        }
        VFS::CVfsBlockCache again;
        ASSERT_TRUE( again.Open( folder.GetPath(), 0, B ) );
        VFS::CVfsBlockCache::SEntryInfo info;
        ASSERT_TRUE( again.FindNewestRevision( "share", pathA, info ) && info.path == pathA && info.complete );
        ASSERT_TRUE( again.FindNewestRevision( "share", pathB, info ) && info.path == pathB && info.complete );
        VFS::CVfsBlockCache::TEntryInfoVector below;
        again.GetEntryInfosBelow( "share", "d\xC3\xA9", false, below );
        ASSERT_TRUE( 2 == below.size() );
        const VFS::CVfsBlockCache::TEntryHandle a = again.AcquireEntry( "share", pathA, "r1", B, VFS::CVfsDigest(), false );
        ASSERT_TRUE( 0 != a && BlockCacheTest::BlockMatches( again, a, 11, 0 ) );
        again.ReleaseEntry( a );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ArrivalInfo: the data of an event is a copy that carries every field" )
    try
    {
        VFS::CVfsArrivalInfo info( VFS::CVfsArrivalInfo::KIND_FAILURE, "a/b" );
        info.revision = "r2";
        info.oldRevision = "r1";
        info.firstByte = 11;
        info.byteCount = 22;
        info.fileComplete = true;
        info.result = CORE::FILESYSTEMACCESS_REMOTE_CAPLIMIT_RETRY_LATER;
        info.message = "m";
        info.jobKind = VFS::FETCHJOB_BLOCKS;

        VFS::CVfsArrivalInfo* copy = static_cast< VFS::CVfsArrivalInfo* >( info.Clone() );
        ASSERT_TRUE( GUCEF_NULL != copy );
        ASSERT_TRUE( VFS::CVfsArrivalInfo::KIND_FAILURE == copy->kind && CORE::CString( "a/b" ) == copy->path );
        ASSERT_TRUE( CORE::CString( "r2" ) == copy->revision && CORE::CString( "r1" ) == copy->oldRevision );
        ASSERT_TRUE( 11 == copy->firstByte && 22 == copy->byteCount && copy->fileComplete );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CAPLIMIT_RETRY_LATER == copy->result && CORE::CString( "m" ) == copy->message );
        ASSERT_TRUE( VFS::FETCHJOB_BLOCKS == copy->jobKind );
        GUCEF_DELETE copy;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RemoteArchiveEvents: six distinct events, each recognised as one of the remote archive events and no other" )
    try
    {
        const CORE::CEvent events[ 6 ] = { VFS::CRemoteArchiveEvents::FileArrivedEvent, VFS::CRemoteArchiveEvents::ListingArrivedEvent, VFS::CRemoteArchiveEvents::MetadataArrivedEvent,
                                           VFS::CRemoteArchiveEvents::FetchFailedEvent, VFS::CRemoteArchiveEvents::FileChangedEvent, VFS::CRemoteArchiveEvents::ConnectionStateChangedEvent };
        for ( UInt32 i=0; i<6; ++i )
        {
            ASSERT_TRUE( VFS::CRemoteArchiveEvents::IsRemoteArchiveEvent( events[ i ] ) );
            ASSERT_TRUE( ( 5 == i ) != VFS::CRemoteArchiveEvents::IsArrivalEvent( events[ i ] ) );
            for ( UInt32 j=i+1; j<6; ++j )
            {
                ASSERT_FALSE( events[ i ] == events[ j ] );
            }
        }
        ASSERT_FALSE( VFS::CRemoteArchiveEvents::IsRemoteArchiveEvent( CORE::CDirectoryWatcherEvents::FileCreatedEvent ) );
        ASSERT_FALSE( VFS::CRemoteArchiveEvents::IsRemoteArchiveEvent( VFS::CVFS::ArchiveMountedEvent ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Value classes: remote entries compare by every field, requests and settings have their defaults, statistics start at zero and reset" )
    try
    {
        VFS::CVfsRemoteEntry a = MakeEntry( "x", false, 5, "5-0" );
        VFS::CVfsRemoteEntry b = a;
        ASSERT_TRUE( a.IsSameAs( b ) );
        b.revision = "5-1";
        ASSERT_FALSE( a.IsSameAs( b ) );
        b = a;
        b.digest = VFS::CVfsDigest( VFS::CVfsDigest::ALGORITHM_MD5, "d41d8cd98f00b204e9800998ecf8427e" );
        ASSERT_FALSE( a.IsSameAs( b ) );
        b = a;
        b.modifiedUtcMs = 5;
        ASSERT_FALSE( a.IsSameAs( b ) );

        VFS::CVfsFetchRequest request;
        ASSERT_TRUE( request.files.empty() && request.directories.empty() && !request.recursive && 1000 == request.maxDirectories );
        ASSERT_TRUE( !request.pin && request.verifyDigest && 4 == request.maxParallelFiles );

        VFS::CVfsRemoteSettings settings;
        ASSERT_TRUE( 3000 == settings.metadataTtlMs && 2 == settings.readAheadAfterBlocks && 8 == settings.readAheadBlocks && 16 == settings.maxBlocksPerFetch );
        ASSERT_TRUE( settings.verifyCompleteFiles && settings.offlineListings );

        VFS::CVfsFetchProgress progress;
        ASSERT_TRUE( 0 == progress.bytesDone.Load() && !progress.cancel.Load() && progress.GetLastFailure().IsSuccess() );
        progress.SetLastFailure( VFS::CVfsCallResult( CORE::FILESYSTEMACCESS_CANCELLED, "c" ) );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_CANCELLED == progress.GetLastFailure().result );

        VFS::CVfsCacheState state;
        ASSERT_TRUE( !state.cached && !state.complete && !state.pinned && !state.superseded && !state.digestVerified && 0 == state.fileSize );

        VFS::CVfsRemoteStatistics statistics;
        ASSERT_TRUE( 0 == statistics.bytesFetched && 0 == statistics.entries && 0 == statistics.handleReopens );
        statistics.bytesFetched = 5;
        statistics.offlineMisses = 6;
        statistics.entries = 7;
        statistics.Reset();
        ASSERT_TRUE( 0 == statistics.bytesFetched && 0 == statistics.offlineMisses && 0 == statistics.entries );

        VFS::CVfsRemoteCounters counters;
        counters.bytesFetched.FetchAdd( 3 );
        counters.fetchJobsRefused.FetchAdd( 4 );
        counters.offlineReads.FetchAdd( 5 );
        VFS::CVfsRemoteStatistics filled;
        filled.entries = 99;
        counters.Fill( filled );
        ASSERT_TRUE( 3 == filled.bytesFetched && 4 == filled.fetchJobsRefused && 5 == filled.offlineReads && 99 == filled.entries );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
