/*
 *  vfspluginDRN_TestApp: tests of the DRN VFS plugin
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

#include <stdio.h>
#include <stdlib.h>
#include <exception>

#if defined( __linux__ )
#include <unistd.h>
#include <malloc.h>
#endif

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

#ifndef GUCEF_DRN_H
#include "gucefDRN.h"
#define GUCEF_DRN_H
#endif /* GUCEF_DRN_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#include "TestArchiveConformanceTree.h"
#include "TestDrnRemoteEnv.h"
#include "TestDrnBridgeBaseline.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace DRNBRIDGETEST;

namespace {

typedef CDrnRemoteEnv E;

/** Microseconds on the precision timer */
UInt64
NowUs( void )
{
    const UInt64 resolution = MT::PrecisionTimerResolution();
    return ( MT::PrecisionTickCount() * 1000000ULL ) / ( 0 == resolution ? 1 : resolution );
}

/** What the heap of this process has in use now, in bytes: what the allocations of the program hold, not what the allocator keeps for reuse (glibc; else the resident set on Linux, 0 elsewhere) */
UInt64
ResidentBytes( void )
{
#if defined( __GLIBC__ ) && ( __GLIBC__ > 2 || ( 2 == __GLIBC__ && __GLIBC_MINOR__ >= 33 ) )
    const struct mallinfo2 info = mallinfo2();
    return (UInt64) info.uordblks + (UInt64) info.hblkhd;
#elif defined( __linux__ )
    FILE* file = fopen( "/proc/self/statm", "r" );
    if ( GUCEF_NULL == file )
        return 0;
    unsigned long long total = 0;
    unsigned long long resident = 0;
    const int count = fscanf( file, "%llu %llu", &total, &resident );
    fclose( file );
    return 2 == count ? (UInt64) resident * (UInt64) sysconf( _SC_PAGESIZE ) : 0;
#else
    return 0;
#endif
}

CORE::CString
Number( double value )
{
    // two decimals
    const UInt64 hundredths = (UInt64) ( value * 100.0 + 0.5 );
    CORE::CString fraction = CORE::ToString( hundredths % 100 );
    if ( fraction.Length() < 2 )
        fraction = "0" + fraction;
    return CORE::ToString( hundredths / 100 ) + "." + fraction;
}

/** Logs a line and keeps it for the file of the results */
void
Report( CORE::CString::StringVector& lines, const CORE::CString& line )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Baseline: " << line );
    lines.push_back( line );
}

bool
WriteFilled( const CORE::CString& path, UInt32 contentId, UInt64 size )
{
    if ( !CORE::CreatePathDirectories( path ) )
        return false;
    CORE::CFileAccess file( path, "wb" );
    if ( !file.Opened() )
        return false;
    CORE::CDynamicBuffer block( 1048576, true );
    UInt64 position = 0;
    while ( position < size )
    {
        const UInt32 count = (UInt32) ( size - position < 1048576 ? size - position : 1048576 );
        CArchiveConformanceTree::FillContent( contentId, position, block.GetBufferPtr(), count );
        if ( count != file.Write( block.GetConstBufferPtr(), 1, count ) )
            return false;
        position += count;
    }
    file.Flush();
    return true;
}

/** Reads a whole file through the VFS in reads of one MiB: the seconds it took, 0 when it did not read */
double
ReadThrough( VFS::CVFS& vfs, const CORE::CString& path, UInt64 size )
{
    VFS::TBasicVfsResourcePtr resource = vfs.GetFile( path );
    if ( resource.IsNULL() )
        return 0;
    CORE::CDynamicBuffer buffer( 1048576, true );
    UInt64 total = 0;
    const UInt64 begin = NowUs();
    while ( total < size )
    {
        const UInt32 got = resource->GetAccess()->Read( buffer.GetBufferPtr(), 1, 1048576 );
        if ( 0 == got )
            break;
        total += got;
    }
    const UInt64 end = NowUs();
    return total == size ? (double) ( end - begin ) / 1000000.0 : 0;
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDrnBridgeBaseline( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "DrnBridgeBaseline" );

    GUCEF_TESTFW_TESTCASE( "Baseline: round trip of a stat and of an open, first byte cold and cached, sequential reads cold and from the cache, a directory of 5000 entries, the memory of the cache index at 10000 entries (logged, nothing asserted)" )
    try
    {
        CORE::CString::StringVector lines;
#ifdef GUCEF_DEBUG_MODE
        Report( lines, "build: debug" );
#else
        Report( lines, "build: release" );
#endif

        // a stat that goes to the share every time: no metadata cache, no block cache
        {
            E env( "baseline-stat", false );
            SRemoteEnvOptions options;
            options.metadataTtlMs = 0;
            options.useCache = false;
            if ( !env.Setup( options ) )
                throw std::exception();
            const CORE::CString path = env.VfsPath( CRemoteEnv::MidFile );
            env.Vfs().FileExists( path );
            const UInt32 calls = 300;
            UInt64 total = 0;
            UInt64 fastest = 0xFFFFFFFFFFFFFFFFULL;
            UInt64 slowest = 0;
            for ( UInt32 i=0; i<calls; ++i )
            {
                const UInt64 begin = NowUs();
                env.Vfs().FileExists( path );
                const UInt64 took = NowUs() - begin;
                total += took;
                if ( took < fastest )
                    fastest = took;
                if ( took > slowest )
                    slowest = took;
            }
            Report( lines, "stat round trip, loopback, " + CORE::ToString( calls ) + " calls: mean " + Number( (double) total / calls / 1000.0 ) + " ms, fastest " + Number( fastest / 1000.0 ) + " ms, slowest " + Number( slowest / 1000.0 ) + " ms" );
        }

        // a file of 32 MiB on the share, read sequentially
        {
            const UInt64 size = 32ULL * 1024 * 1024;
            E env( "baseline-read", false );
            SRemoteEnvOptions options;
            options.blockBytes = 262144;
            options.metadataTtlMs = 60000;
            if ( !env.Setup( options ) )
                throw std::exception();
            if ( !WriteFilled( CORE::CombinePath( env.GetServedRoot(), "bulk/large.bin" ), 99, size ) )
                throw std::exception();
            const CORE::CString path = env.VfsPath( "bulk/large.bin" );
            env.Vfs().InvalidateRemoteMetadata();

            // the first byte: the open and the read of one byte, nothing cached and then cached
            {
                const UInt64 begin = NowUs();
                VFS::TBasicVfsResourcePtr resource = env.Vfs().GetFile( path );
                char one = 0;
                if ( resource.IsNULL() || 1 != resource->GetAccess()->Read( &one, 1, 1 ) )
                    throw std::exception();
                const UInt64 cold = NowUs() - begin;
                resource.Unlink();
                const UInt64 begin2 = NowUs();
                resource = env.Vfs().GetFile( path );
                if ( resource.IsNULL() || 1 != resource->GetAccess()->Read( &one, 1, 1 ) )
                    throw std::exception();
                const UInt64 cached = NowUs() - begin2;
                Report( lines, "open and first byte, uncached: " + Number( cold / 1000.0 ) + " ms, cached: " + Number( cached / 1000.0 ) + " ms" );
            }

            const double coldSeconds = ReadThrough( env.Vfs(), path, size );
            const double warmSeconds = ReadThrough( env.Vfs(), path, size );
            if ( 0 == coldSeconds || 0 == warmSeconds )
                throw std::exception();
            Report( lines, "sequential read of 32 MiB with the block cache (block 256 KiB, read ahead 2 and 8, 16 blocks a run), loopback: cold " + Number( 32.0 / coldSeconds ) + " MiB/s, from the cache " + Number( 32.0 / warmSeconds ) + " MiB/s" );
        }
        {
            const UInt64 size = 32ULL * 1024 * 1024;
            E env( "baseline-legacy", false );
            SRemoteEnvOptions options;
            options.useCache = false;
            options.metadataTtlMs = 60000;
            if ( !env.Setup( options ) )
                throw std::exception();
            if ( !WriteFilled( CORE::CombinePath( env.GetServedRoot(), "bulk/large.bin" ), 99, size ) )
                throw std::exception();
            env.Vfs().InvalidateRemoteMetadata();
            const double seconds = ReadThrough( env.Vfs(), env.VfsPath( "bulk/large.bin" ), size );
            if ( 0 == seconds )
                throw std::exception();
            Report( lines, "sequential read of 32 MiB without a block cache (the facility's chunk cache, 8 chunks and 4 ahead), loopback: " + Number( 32.0 / seconds ) + " MiB/s" );
        }

        // a directory of 5000 entries
        {
            E env( "baseline-list", false );
            SRemoteEnvOptions options;
            options.metadataTtlMs = 60000;
            if ( !env.Setup( options ) )
                throw std::exception();
            for ( UInt32 i=0; i<5000; ++i )
            {
                CORE::CString number = CORE::ToString( i );
                while ( number.Length() < 5 )
                    number = "0" + number;
                CORE::CFileAccess file( CORE::CombinePath( env.GetServedRoot(), "many/f" + number + ".dat" ), "wb" );
                if ( !file.Opened() )
                {
                    CORE::CreatePathDirectories( CORE::CombinePath( env.GetServedRoot(), "many/f" + number + ".dat" ) );
                    file.Open();
                }
                file.Write( "x", 1, 1 );
            }
            env.Vfs().InvalidateRemoteMetadata();
            VFS::CVFS::TStringVector names;
            const UInt32 listsBefore = env.Backend().GetListCount();
            const UInt64 begin = NowUs();
            env.Vfs().GetFileList( names, env.VfsPath( "many" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX );
            const UInt64 cold = NowUs() - begin;
            const UInt32 count = (UInt32) names.size();
            const UInt32 lists = env.Backend().GetListCount() - listsBefore;
            names.clear();
            const UInt64 begin2 = NowUs();
            env.Vfs().GetFileList( names, env.VfsPath( "many" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX );
            const UInt64 cached = NowUs() - begin2;
            Report( lines, "listing of a directory of 5000 entries, loopback: " + CORE::ToString( count ) + " names, cold " + Number( cold / 1000.0 ) + " ms (" + CORE::ToString( lists ) + " list requests of the share), from memory " + Number( cached / 1000.0 ) + " ms" );
        }

        // the index of the block cache for 10000 entries
        {
            CBlockCacheTestFolder folder( "baseline-index" );
            const UInt32 entries = 10000;
            {
                VFS::CVfsBlockCache cache;
                if ( !cache.Open( folder.GetPath(), 0, 65536, 64 ) )
                    throw std::exception();
                CORE::CDynamicBuffer data( 1000, true );
                for ( UInt32 i=0; i<entries; ++i )
                {
                    const VFS::CVfsBlockCache::TEntryHandle handle = cache.AcquireEntry( "baseline", "dir" + CORE::ToString( i % 100 ) + "/file" + CORE::ToString( i ), "1000-1", 1000, VFS::CVfsDigest(), true );
                    if ( 0 == handle )
                        throw std::exception();
                    cache.StoreBlock( handle, 0, data.GetConstBufferPtr(), 1000 );
                    cache.ReleaseEntry( handle );
                }
                cache.Close();
            }
            const UInt64 before = ResidentBytes();
            VFS::CVfsBlockCache reopened;
            const UInt64 begin = NowUs();
            if ( !reopened.Open( folder.GetPath(), 0, 65536, 64 ) )
                throw std::exception();
            const UInt64 opened = NowUs() - begin;
            const UInt64 after = ResidentBytes();
            VFS::CVfsBlockCache::SStatistics statistics;
            reopened.GetStatistics( statistics );
            const double perEntry = after > before ? (double) ( after - before ) / entries : 0;
            Report( lines, "block cache index of " + CORE::ToString( statistics.entries ) + " entries: Open() of the folder " + Number( opened / 1000.0 ) + " ms, memory held by the open cache " + Number( (double) ( after - before ) / 1048576.0 ) + " MiB (" + Number( perEntry ) + " bytes an entry, measured on the heap in use of the process)" );
            reopened.Close();
        }

        // the results, in a file next to the logs
        CORE::CString resultsPath = CORE::GetEnv( "GUCEF_TEST_SCRATCH_DIR" );
        if ( resultsPath.IsNULLOrEmpty() )
            resultsPath = CORE::RelativePath( "$CURWORKDIR$" );
        resultsPath = CORE::CombinePath( resultsPath, "vfspluginDRN_TestApp_Baseline.txt" );
        CORE::CFileAccess results( resultsPath, "w" );
        for ( size_t i=0; i<lines.size(); ++i )
        {
            const CORE::CString line = lines[ i ] + "\n";
            results.Write( line.C_String(), 1, line.Length() );
        }
    }
    catch ( ... )
    {
        GUCEF_TESTFW_ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
