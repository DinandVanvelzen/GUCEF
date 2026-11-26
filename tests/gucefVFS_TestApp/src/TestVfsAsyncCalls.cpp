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
#include <stdexcept>

#ifndef GUCEF_MT_H
#include "gucefMT.h"
#define GUCEF_MT_H
#endif /* GUCEF_MT_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_VFS_CVFSFILESYSTEMACCESS_H
#include "gucefVFS_CVfsFileSystemAccess.h"
#define GUCEF_VFS_CVFSFILESYSTEMACCESS_H
#endif /* GUCEF_VFS_CVFSFILESYSTEMACCESS_H ? */

#include "TestThreads.h"
#include "TestVfsBlockCacheUtils.h"
#include "TestRemoteArchiveFixture.h"

#include "TestVfsAsyncCalls.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/** A precondition: when it does not hold the case ends here, so that the lines after it do not meet what it should have made */
#define REQUIRE(t)      do { if ( !(t) ) { GUCEF_TESTFW_ASSERT_TRUE(t); throw std::runtime_error( "a precondition of the case does not hold" ); } } while (0)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef CRemoteTestEnv E;

const UInt32 B = 65536;

UInt64
NowMs( void )
{
    return CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
}

bool
WaitReady( const CORE::CFutureResult& future, UInt32 timeoutMs )
{
    for ( UInt32 waited=0; waited<=timeoutMs; ++waited )
    {
        if ( future.IsReady() )
            return true;
        MT::ThreadDelay( 1 );
    }
    return false;
}

CORE::TTaskStatus
StatusOf( const CORE::CFutureResult& future )
{
    CORE::CTaskPtr task = future.GetResult( 5000 );
    return task.IsNULL() ? CORE::TASKSTATUS_UNDEFINED : task->GetTaskStatus();
}

/** One asynchronous call: the future and the result the call fills in */
struct SAsync
{
    CORE::CFutureResult future;
    VFS::CVfsAsyncResultPtr result;
    SAsync( void ) : future( CORE::TASKSTATUS_QUEUEING_FAILED ), result() {}

    /** Waits for the call to end; false when it was not queued or did not end in time */
    bool Wait( UInt32 timeoutMs = 20000 ) const { return future.HasAFuture() && WaitReady( future, timeoutMs ) && !result.IsNULL(); }
};

SAsync
MetaData( VFS::CVFS& vfs, const CORE::CString& path )
{
    SAsync a;
    a.future = vfs.GetFileMetaDataAsync( path, a.result );
    return a;
}

SAsync
FileList( VFS::CVFS& vfs, const CORE::CString& location, bool recursive = false, bool includePath = false )
{
    SAsync a;
    a.future = vfs.GetFileListAsync( location, recursive, includePath, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX, a.result );
    return a;
}

SAsync
DirList( VFS::CVFS& vfs, const CORE::CString& location, bool recursive = false, bool includePath = false )
{
    SAsync a;
    a.future = vfs.GetDirListAsync( location, recursive, includePath, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX, a.result );
    return a;
}

SAsync
OpenFile( VFS::CVFS& vfs, const CORE::CString& path )
{
    SAsync a;
    a.future = vfs.GetFileAsync( path, a.result );
    return a;
}

SAsync
ReadRange( VFS::CVFS& vfs, const CORE::CString& path, UInt64 offset, UInt32 length )
{
    SAsync a;
    a.future = vfs.ReadFileRangeAsync( path, offset, length, a.result );
    return a;
}

SAsync
FetchFile( VFS::CVFS& vfs, const CORE::CString& path, VFS::CVfsFetchProgressPtr& progress, bool pin = false )
{
    SAsync a;
    VFS::CVfsFetchRequest request;
    request.pin = pin;
    a.future = vfs.FetchFileAsync( path, request, progress, a.result );
    return a;
}

SAsync
Prefetch( VFS::CVFS& vfs, const VFS::CVfsFetchRequest& request, VFS::CVfsFetchProgressPtr& progress )
{
    SAsync a;
    a.future = vfs.PrefetchAsync( request, progress, a.result );
    return a;
}

/** Names the pool of the remote operations for a case and puts the default back when the case ends, however it ends */
class CPoolScope
{
    public:

    CPoolScope( VFS::CVFS& vfs, const CORE::CString& poolName, UInt32 workers ) : m_vfs( vfs )
    {
        CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( poolName );
        if ( !pool.IsNULL() )
        {
            pool->SetDesiredMinNrOfWorkerThreads( workers );
            pool->SetDesiredMaxTotalNrOfThreads( (Int32) workers );
        }
        m_vfs.SetRemoteOpsThreadPool( poolName );
    }

    ~CPoolScope()
    {
        m_vfs.SetRemoteOpsThreadPool( VFS::CVfsFetchScheduler::DefaultThreadPoolName );
    }

    private:

    CPoolScope( const CPoolScope& src );
    CPoolScope& operator=( const CPoolScope& src );

    VFS::CVFS& m_vfs;
};

/** A folder of the file system mounted as a local archive: three files of known bytes */
class CLocalFiles
{
    public:

    static const UInt32 OneSize = 100;
    static const UInt32 TwoSize = 70000;
    static const UInt32 ThreeSize = 1000;

    CLocalFiles( VFS::CVFS& vfs, const char* name, const CORE::CString& mountPath )
        : m_vfs( vfs )
        , m_name( name )
        , m_folder( name )
        , m_mounted( false )
    {
        WriteFile( "one.txt", 11, OneSize );
        WriteFile( "sub/two.bin", 12, TwoSize );
        WriteFile( "sub/deeper/three.bin", 13, ThreeSize );
        m_mounted = m_vfs.AddRoot( m_folder.GetPath(), m_name, false, false, mountPath );
    }

    ~CLocalFiles()
    {
        if ( m_mounted )
            m_vfs.UnmountArchiveByName( m_name );
    }

    bool IsMounted( void ) const { return m_mounted; }

    static bool Matches( UInt32 seed, UInt64 offset, const CORE::CDynamicBuffer& content, UInt32 count )
    {
        if ( content.GetDataSize() < count )
            return false;
        CORE::CDynamicBuffer expected( count, true );
        BlockCacheTest::FillPattern( seed, offset, expected.GetBufferPtr(), count );
        return 0 == count || 0 == memcmp( expected.GetConstBufferPtr(), content.GetConstBufferPtr(), count );
    }

    private:

    CLocalFiles( const CLocalFiles& src );
    CLocalFiles& operator=( const CLocalFiles& src );

    void WriteFile( const char* relative, UInt32 seed, UInt32 size )
    {
        CORE::CDynamicBuffer content( size, true );
        BlockCacheTest::FillPattern( seed, 0, content.GetBufferPtr(), size );
        content.SetDataSize( size );
        const CORE::CString file = m_folder.GetChildPath( relative );
        CORE::CreatePathDirectories( file );
        BlockCacheTest::WriteWholeFile( file, content );
    }

    VFS::CVFS& m_vfs;
    CORE::CString m_name;
    CBlockCacheTestFolder m_folder;
    bool m_mounted;
};

/** Reads from an open file from the start: the bytes that came */
UInt32
ReadFromStart( VFS::TBasicVfsResourcePtr resource, UInt32 count, CORE::CDynamicBuffer& buffer )
{
    if ( resource.IsNULL() || resource->GetAccess().IsNULL() )
        return 0;
    buffer.SetDataSize( count );
    resource->GetAccess()->Setpos( 0 );
    const UInt32 got = resource->GetAccess()->Read( buffer.GetBufferPtr(), 1, count );
    buffer.SetDataSize( got );
    return got;
}

/** Watches a progress while a fetch runs: monotone, never past the total, and cancels when it is told to (the flag, or the future) */
class CProgressWatcher : public CTestThreadBody
{
    public:

    VFS::CVfsFetchProgress* progress;
    UInt64 cancelAtBytes;
    const CORE::CFutureResult* cancelFuture;
    MT::CTAtomic< bool > stop;
    MT::CTAtomic< UInt32 > decreased;
    MT::CTAtomic< UInt32 > samples;
    MT::CTAtomic< UInt32 > pastTotal;

    CProgressWatcher( VFS::CVfsFetchProgress* p, UInt64 cancelAt, const CORE::CFutureResult* future )
        : progress( p ), cancelAtBytes( cancelAt ), cancelFuture( future ), stop( false ), decreased( 0 ), samples( 0 ), pastTotal( 0 ) {}

    virtual void Run( UInt32 /* index */ ) GUCEF_VIRTUAL_OVERRIDE
    {
        UInt64 last = 0;
        UInt64 lastFiles = 0;
        while ( !stop.Load() )
        {
            const UInt64 now = progress->bytesDone.Load();
            const UInt64 files = (UInt64) progress->filesDone.Load() + progress->filesFailed.Load();
            if ( now < last || files < lastFiles )
                decreased.FetchAdd( 1 );
            if ( progress->bytesTotal.Load() > 0 && now > progress->bytesTotal.Load() )
                pastTotal.FetchAdd( 1 );
            if ( now != last )
                samples.FetchAdd( 1 );
            last = now;
            lastFiles = files;
            if ( cancelAtBytes > 0 && now >= cancelAtBytes )
            {
                if ( GUCEF_NULL != cancelFuture )
                    cancelFuture->RequestCancellation();
                else
                    progress->cancel.Store( true );
            }
            MT::ThreadDelay( 1 );
        }
    }
};

/** Mixed asynchronous calls from a thread: every answer is checked against the content of the place */
class CMixedAsyncCallers : public CTestThreadBody
{
    public:

    E* env;
    MT::CTAtomic< UInt32 > wrong;
    MT::CTAtomic< UInt32 > calls;
    MT::CTAtomic< UInt32 > notQueued;

    explicit CMixedAsyncCallers( E* e ) : env( e ), wrong( 0 ), calls( 0 ), notQueued( 0 ) {}

    virtual void Run( UInt32 index ) GUCEF_VIRTUAL_OVERRIDE
    {
        const char* const names[ 4 ] = { E::ReadmeFile, E::NotesFile, E::MidFile, E::DeepFile };
        for ( UInt32 i=0; i<30; ++i )
        {
            const char* name = names[ ( i + index ) % 4 ];
            const CORE::CString path = env->VfsPath( name );
            const UInt64 size = env->Backend().GetSize( name );
            switch ( ( i + index ) % 5 )
            {
                case 0:
                {
                    SAsync a = MetaData( env->Vfs(), path );
                    if ( !a.Wait() ) { notQueued.FetchAdd( 1 ); break; }
                    if ( !a.result->result.IsSuccess() || !a.result->exists || a.result->metaData.resourceSizeInBytes != size )
                        wrong.FetchAdd( 1 );
                    break;
                }
                case 1:
                {
                    SAsync a = FileList( env->Vfs(), env->VfsPath( "docs" ) );
                    if ( !a.Wait() ) { notQueued.FetchAdd( 1 ); break; }
                    if ( !a.result->result.IsSuccess() || 1 != a.result->names.size() || "readme.txt" != a.result->names[ 0 ] )
                        wrong.FetchAdd( 1 );
                    break;
                }
                case 2:
                {
                    SAsync a = OpenFile( env->Vfs(), path );
                    if ( !a.Wait() ) { notQueued.FetchAdd( 1 ); break; }
                    CORE::CDynamicBuffer buffer;
                    const UInt32 want = size < 2000 ? (UInt32) size : 2000;
                    if ( !a.result->result.IsSuccess() || want != ReadFromStart( a.result->resource, want, buffer ) || !env->Backend().CheckExpected( name, 0, buffer.GetConstBufferPtr(), want ) )
                        wrong.FetchAdd( 1 );
                    break;
                }
                case 3:
                {
                    SAsync a = ReadRange( env->Vfs(), path, 100, 800 );
                    if ( !a.Wait() ) { notQueued.FetchAdd( 1 ); break; }
                    if ( !a.result->result.IsSuccess() || 800 != a.result->data.GetDataSize() || !env->Backend().CheckExpected( name, 100, a.result->data.GetConstBufferPtr(), 800 ) )
                        wrong.FetchAdd( 1 );
                    break;
                }
                default:
                {
                    VFS::CVfsFetchProgressPtr progress;
                    SAsync a = FetchFile( env->Vfs(), path, progress );
                    if ( !a.Wait() ) { notQueued.FetchAdd( 1 ); break; }
                    if ( !a.result->result.IsSuccess() || progress.IsNULL() || 1 != progress->filesTotal.Load() || 1 != progress->filesDone.Load() )
                        wrong.FetchAdd( 1 );
                    break;
                }
            }
            calls.FetchAdd( 1 );
        }
    }
};


/** A local archive whose metadata call takes its time: for the case that shows that the call of an archive that is not remote keeps the mount in place */
class CSlowLocalArchive : public VFS::CFileSystemArchive
{
    public:

    static MT::CTAtomic< UInt32 > inside;
    static MT::CTAtomic< UInt32 > delayMs;

    CSlowLocalArchive( void ) : VFS::CFileSystemArchive() {}

    virtual ~CSlowLocalArchive() GUCEF_VIRTUAL_OVERRIDE {}

    virtual bool GetFileMetaData( const VFS::CString& filePath, CORE::CResourceMetaData& metaData ) const GUCEF_VIRTUAL_OVERRIDE
    {
        inside.FetchAdd( 1 );
        MT::ThreadDelay( delayMs.Load() );
        return VFS::CFileSystemArchive::GetFileMetaData( filePath, metaData );
    }
};

MT::CTAtomic< UInt32 > CSlowLocalArchive::inside( 0 );
MT::CTAtomic< UInt32 > CSlowLocalArchive::delayMs( 0 );

typedef CORE::CTFactory< VFS::CArchive, CSlowLocalArchive, MT::CMutex > TSlowLocalArchiveFactory;
}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformVfsAsyncCallsTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "VfsAsyncCalls" );

    GUCEF_TESTFW_TESTCASE( "Success on a remote mount: the metadata (with the digest of the place), both listings, the open, a range, the fetch of a file and the prefetch of files and a directory each deliver what the place has, and the progress says so" )
    try
    {
        E env( "gucefvfs-async-s1" );
        REQUIRE( env.Setup() );
        env.Backend().ProvideDigest( E::ReadmeFile );
        VFS::CVFS& vfs = env.Vfs();

        // metadata
        SAsync meta = MetaData( vfs, env.VfsPath( E::ReadmeFile ) );
        REQUIRE( meta.Wait() );
        ASSERT_TRUE( meta.result->result.IsSuccess() && meta.result->exists );
        ASSERT_TRUE( 1000 == meta.result->metaData.resourceSizeInBytes && meta.result->metaData.resourceExists );
        VFS::CVfsDigest digest;
        ASSERT_TRUE( vfs.GetFileDigest( env.VfsPath( E::ReadmeFile ), digest ) );
        ASSERT_TRUE( !meta.result->digest.IsEmpty() && meta.result->digest.Matches( digest ) );
        ASSERT_TRUE( meta.result->digest.Matches( env.Backend().ComputeDigest( E::ReadmeFile, VFS::CVfsDigest::ALGORITHM_SHA256 ) ) );

        SAsync noDigest = MetaData( vfs, env.VfsPath( E::NotesFile ) );
        REQUIRE( noDigest.Wait() );
        ASSERT_TRUE( noDigest.result->result.IsSuccess() && noDigest.result->digest.IsEmpty() && 70000 == noDigest.result->metaData.resourceSizeInBytes );

        // listings
        SAsync files = FileList( vfs, env.VfsPath( "docs" ) );
        REQUIRE( files.Wait() );
        ASSERT_TRUE( files.result->result.IsSuccess() && 1 == files.result->names.size() && "readme.txt" == files.result->names[ 0 ] );
        SAsync filesDeep = FileList( vfs, env.VfsPath( "docs" ), true, true );
        REQUIRE( filesDeep.Wait() );
        ASSERT_TRUE( filesDeep.result->result.IsSuccess() && 2 == filesDeep.result->names.size() );
        SAsync dirs = DirList( vfs, env.VfsPath( "docs" ) );
        REQUIRE( dirs.Wait() );
        ASSERT_TRUE( dirs.result->result.IsSuccess() && 1 == dirs.result->names.size() && "notes/" == dirs.result->names[ 0 ] );

        // the open: the file reads on this thread, which is not the thread that opened it
        SAsync open = OpenFile( vfs, env.VfsPath( E::MidFile ) );
        REQUIRE( open.Wait() );
        ASSERT_TRUE( open.result->result.IsSuccess() && !open.result->resource.IsNULL() );
        CORE::CDynamicBuffer buffer;
        ASSERT_TRUE( 3000 == ReadFromStart( open.result->resource, 3000, buffer ) );
        ASSERT_TRUE( env.Matches( E::MidFile, 0, buffer, 3000 ) );

        // a range, one that ends at the end of the file, and ranges with nothing in them
        SAsync range = ReadRange( vfs, env.VfsPath( E::MidFile ), 70000, 100000 );
        REQUIRE( range.Wait() );
        ASSERT_TRUE( range.result->result.IsSuccess() && 100000 == range.result->data.GetDataSize() && 100000 == range.result->bytesDone );
        ASSERT_TRUE( env.Matches( E::MidFile, 70000, range.result->data, 100000 ) );
        SAsync tail = ReadRange( vfs, env.VfsPath( E::MidFile ), 299900, 1000 );
        REQUIRE( tail.Wait() );
        ASSERT_TRUE( tail.result->result.IsSuccess() && 100 == tail.result->data.GetDataSize() && env.Matches( E::MidFile, 299900, tail.result->data, 100 ) );
        SAsync atEnd = ReadRange( vfs, env.VfsPath( E::MidFile ), 300000, 10 );
        REQUIRE( atEnd.Wait() );
        ASSERT_TRUE( atEnd.result->result.IsSuccess() && 0 == atEnd.result->data.GetDataSize() );
        SAsync pastEnd = ReadRange( vfs, env.VfsPath( E::MidFile ), 900000, 10 );
        REQUIRE( pastEnd.Wait() );
        ASSERT_TRUE( pastEnd.result->result.IsSuccess() && 0 == pastEnd.result->data.GetDataSize() );
        SAsync none = ReadRange( vfs, env.VfsPath( E::MidFile ), 10, 0 );
        REQUIRE( none.Wait() );
        ASSERT_TRUE( none.result->result.IsSuccess() && 0 == none.result->data.GetDataSize() );

        // the fetch of a whole file
        VFS::CVfsFetchProgressPtr progress;
        SAsync fetch = FetchFile( vfs, env.VfsPath( E::BigFile ), progress, true );
        REQUIRE( fetch.Wait() );
        ASSERT_TRUE( fetch.result->result.IsSuccess() && !progress.IsNULL() );
        ASSERT_TRUE( 1000000 == fetch.result->bytesDone && 1000000 == progress->bytesDone.Load() && 1000000 == progress->bytesTotal.Load() );
        ASSERT_TRUE( 1 == progress->filesTotal.Load() && 1 == progress->filesDone.Load() && 0 == progress->filesFailed.Load() );
        VFS::CVfsCacheState state;
        ASSERT_TRUE( vfs.GetFileCacheState( env.VfsPath( E::BigFile ), state ) && state.complete && state.pinned );

        // the prefetch of two files and a directory with its listing
        VFS::CVfsFetchRequest request;
        request.files.push_back( env.VfsPath( E::NotesFile ) );
        request.files.push_back( env.VfsPath( E::DeepFile ) );
        request.directories.push_back( env.VfsPath( "data" ) );
        request.maxParallelFiles = 2;
        VFS::CVfsFetchProgressPtr prefetchProgress;
        SAsync prefetch = Prefetch( vfs, request, prefetchProgress );
        REQUIRE( prefetch.Wait() );
        ASSERT_TRUE( prefetch.result->result.IsSuccess() && !prefetchProgress.IsNULL() );
        ASSERT_TRUE( 3 == prefetchProgress->filesTotal.Load() && 3 == prefetchProgress->filesDone.Load() && 0 == prefetchProgress->filesFailed.Load() );
        ASSERT_TRUE( 75000 == prefetchProgress->bytesDone.Load() && 75000 == prefetch.result->bytesDone );
        ASSERT_TRUE( vfs.GetFileCacheState( env.VfsPath( E::NotesFile ), state ) && state.complete );
        ASSERT_TRUE( vfs.GetFileCacheState( env.VfsPath( E::DeepFile ), state ) && state.complete );

        // the listing that the prefetch fetched is in the metadata: no call to the place for it
        env.Backend().ResetCounters();
        VFS::CVFS::TStringVector list;
        ASSERT_TRUE( vfs.GetFileList( list, env.VfsPath( "data" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX ) && 2 == list.size() );
        ASSERT_TRUE( 0 == env.Backend().GetListCount() );

        // an empty request is done at once
        VFS::CVfsFetchProgressPtr emptyProgress;
        SAsync empty = Prefetch( vfs, VFS::CVfsFetchRequest(), emptyProgress );
        REQUIRE( empty.Wait() );
        ASSERT_TRUE( empty.result->result.IsSuccess() && 0 == emptyProgress->filesTotal.Load() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Error mapping on a remote mount: what is not there is NOT_FOUND, a range that is too long is TOO_LONG, a link that is down is CONNECTION_PENDING, a run that is cut says why, and a prefetch of files of which some fail is PARTIAL_SUCCESS" )
    try
    {
        E env( "gucefvfs-async-e1" );
        REQUIRE( env.Setup() );
        VFS::CVFS& vfs = env.Vfs();

        SAsync absent = MetaData( vfs, env.VfsPath( "docs/missing.txt" ) );
        REQUIRE( absent.Wait() );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_FOUND == absent.result->result.result && !absent.result->exists );
        SAsync noFile = OpenFile( vfs, env.VfsPath( "docs/missing.txt" ) );
        REQUIRE( noFile.Wait() );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_FOUND == noFile.result->result.result && noFile.result->resource.IsNULL() );
        SAsync noRange = ReadRange( vfs, env.VfsPath( "docs/missing.txt" ), 0, 10 );
        REQUIRE( noRange.Wait() );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_FOUND == noRange.result->result.result && 0 == noRange.result->data.GetDataSize() );
        SAsync emptyPath = OpenFile( vfs, CORE::CString::Empty );
        REQUIRE( emptyPath.Wait() );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_INVALID_PATH == emptyPath.result->result.result );
        SAsync noDir = FileList( vfs, env.VfsPath( "nodir" ) );
        REQUIRE( noDir.Wait() );
        ASSERT_TRUE( noDir.result->result.IsSuccess() && noDir.result->names.empty() );         // as the synchronous listing: a directory that is not there lists nothing

        VFS::CVfsFetchProgressPtr progress;
        SAsync noFetch = FetchFile( vfs, env.VfsPath( "docs/missing.txt" ), progress );
        REQUIRE( noFetch.Wait() );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_FOUND == noFetch.result->result.result );
        ASSERT_TRUE( 1 == progress->filesTotal.Load() && 1 == progress->filesFailed.Load() && 0 == progress->filesDone.Load() );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_FOUND == progress->GetLastFailure().result );

        // a range longer than one call reads
        vfs.SetMaxAsyncReadBytes( 1000 );
        SAsync tooLong = ReadRange( vfs, env.VfsPath( E::MidFile ), 0, 1001 );
        SAsync justRight = ReadRange( vfs, env.VfsPath( E::MidFile ), 0, 1000 );
        REQUIRE( tooLong.Wait() && justRight.Wait() );
        vfs.SetMaxAsyncReadBytes( 67108864 );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_TOO_LONG == tooLong.result->result.result && 0 == tooLong.result->data.GetDataSize() );
        ASSERT_TRUE( justRight.result->result.IsSuccess() && 1000 == justRight.result->data.GetDataSize() );

        // a run that the link cuts: the read fails with the reason that the access gives
        env.Backend().CutNextFetchAfter( 100 );
        SAsync cut = ReadRange( vfs, env.VfsPath( E::BigFile ), 0, 1000 );
        REQUIRE( cut.Wait() );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == cut.result->result.result && 0 == cut.result->data.GetDataSize() );

        // a prefetch of which some fail: the files that can be fetched are
        VFS::CVfsFetchRequest request;
        request.files.push_back( env.VfsPath( E::MidFile ) );
        request.files.push_back( env.VfsPath( "data/nope.bin" ) );
        request.files.push_back( env.VfsPath( E::ReadmeFile ) );
        VFS::CVfsFetchProgressPtr partialProgress;
        SAsync partial = Prefetch( vfs, request, partialProgress );
        REQUIRE( partial.Wait() );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_PARTIAL_SUCCESS == partial.result->result.result );
        ASSERT_TRUE( 3 == partialProgress->filesTotal.Load() && 2 == partialProgress->filesDone.Load() && 1 == partialProgress->filesFailed.Load() );
        VFS::CVfsCacheState state;
        ASSERT_TRUE( vfs.GetFileCacheState( env.VfsPath( E::MidFile ), state ) && state.complete );

        VFS::CVfsFetchRequest allMissing;
        allMissing.files.push_back( env.VfsPath( "data/nope.bin" ) );
        allMissing.files.push_back( env.VfsPath( "data/nope2.bin" ) );
        VFS::CVfsFetchProgressPtr missingProgress;
        SAsync missing = Prefetch( vfs, allMissing, missingProgress );
        REQUIRE( missing.Wait() );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_FOUND == missing.result->result.result );
        ASSERT_TRUE( 2 == missingProgress->filesTotal.Load() && 2 == missingProgress->filesFailed.Load() );

        // the link is down: what the cache holds is served, what it does not hold is CONNECTION_PENDING and no call reaches the place
        env.Backend().SetLinkUp( false );
        env.Backend().ResetCounters();
        SAsync downMeta = MetaData( vfs, env.VfsPath( E::TopFile ) );
        SAsync downOpen = OpenFile( vfs, env.VfsPath( E::TopFile ) );
        SAsync downRange = ReadRange( vfs, env.VfsPath( E::TopFile ), 0, 10 );
        VFS::CVfsFetchProgressPtr downProgress;
        SAsync downFetch = FetchFile( vfs, env.VfsPath( E::TopFile ), downProgress );
        VFS::CVfsFetchRequest downRequest;
        downRequest.files.push_back( env.VfsPath( E::TopFile ) );
        VFS::CVfsFetchProgressPtr downPrefetchProgress;
        SAsync downPrefetch = Prefetch( vfs, downRequest, downPrefetchProgress );
        REQUIRE( downMeta.Wait() && downOpen.Wait() && downRange.Wait() && downFetch.Wait() && downPrefetch.Wait() );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == downMeta.result->result.result );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == downOpen.result->result.result && downOpen.result->resource.IsNULL() );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == downRange.result->result.result );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == downFetch.result->result.result && 1 == downProgress->filesFailed.Load() );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == downPrefetch.result->result.result );
        ASSERT_TRUE( !downMeta.result->result.message.IsNULLOrEmpty() );
        ASSERT_TRUE( 0 == env.Backend().GetStatCount() && 0 == env.Backend().GetOpenCount() && 0 == env.Backend().GetFetchCount() );
        SAsync cachedRange = ReadRange( vfs, env.VfsPath( E::MidFile ), 0, 1000 );
        REQUIRE( cachedRange.Wait() );
        ASSERT_TRUE( cachedRange.result->result.IsSuccess() && 1000 == cachedRange.result->data.GetDataSize() && env.Matches( E::MidFile, 0, cachedRange.result->data, 1000 ) );
        env.Backend().SetLinkUp( true );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A remote mount above a local mount of the same path: an async call that the remote mount cannot answer yet stops there, as the synchronous call does, and does not give the file of the mount below" )
    try
    {
        E env( "gucefvfs-async-o1" );
        REQUIRE( env.Setup() );
        VFS::CVFS& vfs = env.Vfs();

        CBlockCacheTestFolder folder( "gucefvfs-async-twin" );
        CORE::CDynamicBuffer content;
        content.Append( "LOCAL", 5 );
        const CORE::CString twinFile = folder.GetChildPath( "docs/readme.txt" );
        CORE::CreatePathDirectories( twinFile );
        BlockCacheTest::WriteWholeFile( twinFile, content );
        REQUIRE( vfs.AddRoot( folder.GetPath(), "asynctwin", false, false, E::MountPath ) );

        // the link is up: the remote file, which is the mount above
        SAsync up = OpenFile( vfs, env.VfsPath( E::ReadmeFile ) );
        REQUIRE( up.Wait() );
        CORE::CDynamicBuffer buffer;
        ASSERT_TRUE( up.result->result.IsSuccess() && 1000 == ReadFromStart( up.result->resource, 1000, buffer ) );

        // the link is down and the file of another path was never cached: the remote mount has no answer, the twin is not asked
        env.Backend().SetLinkUp( false );
        SAsync down = OpenFile( vfs, env.VfsPath( E::TopFile ) );
        SAsync downMeta = MetaData( vfs, env.VfsPath( E::TopFile ) );
        REQUIRE( down.Wait() && downMeta.Wait() );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == down.result->result.result && down.result->resource.IsNULL() );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == downMeta.result->result.result );
        const bool syncFile = vfs.FileExists( env.VfsPath( E::TopFile ) );
        VFS::CVfsCallResult syncResult;
        vfs.GetLastResult( syncResult );
        ASSERT_FALSE( syncFile );
        ASSERT_TRUE( syncResult.result == down.result->result.result );
        env.Backend().SetLinkUp( true );

        vfs.UnmountArchiveByName( "asynctwin" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A local mount: the calls give what the synchronous calls give, a fetch is NOT_SUPPORTED (the file is local, there is nothing to fetch) and counted as one failed file, and a prefetch of local files fails the same way" )
    try
    {
        E env( "gucefvfs-async-l1" );
        REQUIRE( env.Setup() );
        VFS::CVFS& vfs = env.Vfs();
        CLocalFiles local( vfs, "asynclocal", "/asynclocal/" );
        REQUIRE( local.IsMounted() );

        SAsync meta = MetaData( vfs, "/asynclocal/one.txt" );
        REQUIRE( meta.Wait() );
        CORE::CResourceMetaData syncMeta;
        ASSERT_TRUE( vfs.GetFileMetaData( "/asynclocal/one.txt", syncMeta ) );
        ASSERT_TRUE( meta.result->result.IsSuccess() && meta.result->exists && meta.result->metaData.resourceSizeInBytes == syncMeta.resourceSizeInBytes && 100 == meta.result->metaData.resourceSizeInBytes );
        ASSERT_TRUE( meta.result->digest.IsEmpty() );               // a digest that the archive would have to make from the content is not made

        SAsync absent = MetaData( vfs, "/asynclocal/none.txt" );
        REQUIRE( absent.Wait() );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_FOUND == absent.result->result.result && !absent.result->exists );

        SAsync files = FileList( vfs, "/asynclocal/sub", true, false );
        SAsync dirs = DirList( vfs, "/asynclocal/", false, false );
        REQUIRE( files.Wait() && dirs.Wait() );
        VFS::CVFS::TStringVector syncFiles, syncDirs;
        ASSERT_TRUE( vfs.GetFileList( syncFiles, "/asynclocal/sub", true, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX ) );
        ASSERT_TRUE( vfs.GetDirList( syncDirs, "/asynclocal/", false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX ) );
        ASSERT_TRUE( files.result->result.IsSuccess() && files.result->names == syncFiles && 2 == syncFiles.size() );
        ASSERT_TRUE( dirs.result->result.IsSuccess() && dirs.result->names == syncDirs && 1 == syncDirs.size() );

        SAsync open = OpenFile( vfs, "/asynclocal/sub/two.bin" );
        REQUIRE( open.Wait() );
        CORE::CDynamicBuffer buffer;
        ASSERT_TRUE( open.result->result.IsSuccess() && 5000 == ReadFromStart( open.result->resource, 5000, buffer ) && CLocalFiles::Matches( 12, 0, buffer, 5000 ) );

        SAsync range = ReadRange( vfs, "/asynclocal/sub/two.bin", 65000, 10000 );
        REQUIRE( range.Wait() );
        ASSERT_TRUE( range.result->result.IsSuccess() && 5000 == range.result->data.GetDataSize() && CLocalFiles::Matches( 12, 65000, range.result->data, 5000 ) );

        // the fetch: not a remote archive
        VFS::CVfsFetchProgressPtr progress;
        SAsync fetch = FetchFile( vfs, "/asynclocal/one.txt", progress );
        REQUIRE( fetch.Wait() );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_SUPPORTED == fetch.result->result.result );
        ASSERT_TRUE( 1 == progress->filesTotal.Load() && 1 == progress->filesFailed.Load() && 0 == progress->filesDone.Load() && 0 == fetch.result->bytesDone );
        VFS::CVfsFetchProgress syncProgress;
        const VFS::CVfsCallResult syncFetch = vfs.FetchFile( "/asynclocal/one.txt", VFS::CVfsFetchRequest(), syncProgress );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_SUPPORTED == syncFetch.result && 1 == syncProgress.filesTotal.Load() && 1 == syncProgress.filesFailed.Load() );

        VFS::CVfsFetchRequest request;
        request.files.push_back( "/asynclocal/one.txt" );
        request.files.push_back( "/asynclocal/sub/two.bin" );
        request.directories.push_back( "/asynclocal/sub" );
        VFS::CVfsFetchProgressPtr prefetchProgress;
        SAsync prefetch = Prefetch( vfs, request, prefetchProgress );
        REQUIRE( prefetch.Wait() );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_SUPPORTED == prefetch.result->result.result );
        ASSERT_TRUE( 3 == prefetchProgress->filesTotal.Load() && 3 == prefetchProgress->filesFailed.Load() && 0 == prefetchProgress->filesDone.Load() );

        // the sync directory fetch: the same answer for a local mount, and the remote one fetches its listings
        VFS::CVfsFetchProgress dirProgress;
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_SUPPORTED == vfs.FetchDirectory( "/asynclocal/sub", true, 10, dirProgress ).result );
        ASSERT_TRUE( 1 == dirProgress.filesTotal.Load() && 1 == dirProgress.filesFailed.Load() );
        VFS::CVfsFetchProgress remoteDirProgress;
        ASSERT_TRUE( vfs.FetchDirectory( env.VfsPath( "docs" ), true, 10, remoteDirProgress ).IsSuccess() );
        ASSERT_TRUE( 2 == remoteDirProgress.filesTotal.Load() && 2 == remoteDirProgress.filesDone.Load() );
        VFS::CVfsFetchProgress absentDirProgress;
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_SUPPORTED == vfs.FetchDirectory( "/nowhere/dir", false, 10, absentDirProgress ).result );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ResolveMount gives the first mount that covers a path with its archive and the path inside it, and nothing for a path that no mount covers; the remote operations pool has a name that can be set, an empty name is the default, and the background fetches follow it" )
    try
    {
        E env( "gucefvfs-async-r1" );
        REQUIRE( env.Setup() );
        VFS::CVFS& vfs = env.Vfs();

        VFS::CVFS::TArchivePtr archive;
        CORE::CString remainder;
        ASSERT_TRUE( vfs.ResolveMount( env.VfsPath( E::ReadmeFile ), archive, remainder ) );
        ASSERT_TRUE( !archive.IsNULL() && "docs/readme.txt" == remainder && E::ArchiveName == archive->GetArchiveName() );
        ASSERT_TRUE( GUCEF_NULL != archive->GetRemoteArchiveInterface() );
        ASSERT_FALSE( vfs.ResolveMount( "/no/such/mount/file.txt", archive, remainder ) );

        ASSERT_TRUE( VFS::CVfsFetchScheduler::DefaultThreadPoolName == vfs.GetRemoteOpsThreadPool() );
        vfs.SetRemoteOpsThreadPool( "gucefvfs-async-pool-name" );
        ASSERT_TRUE( "gucefvfs-async-pool-name" == vfs.GetRemoteOpsThreadPool() );
        ASSERT_TRUE( "gucefvfs-async-pool-name" == VFS::CVfsGlobal::Instance()->GetFetchScheduler().GetThreadPoolName() );
        vfs.SetRemoteOpsThreadPool( CORE::CString::Empty );
        ASSERT_TRUE( VFS::CVfsFetchScheduler::DefaultThreadPoolName == vfs.GetRemoteOpsThreadPool() );
        ASSERT_TRUE( VFS::CVfsFetchScheduler::DefaultThreadPoolName == VFS::CVfsGlobal::Instance()->GetFetchScheduler().GetThreadPoolName() );

        ASSERT_TRUE( 67108864 == vfs.GetMaxAsyncReadBytes() );
        vfs.SetMaxAsyncReadBytes( 12345 );
        ASSERT_TRUE( 12345 == vfs.GetMaxAsyncReadBytes() );
        vfs.SetMaxAsyncReadBytes( 67108864 );

        // the calls ran on the pool that is named: another pool that is named is used from then on
        CPoolScope pool( vfs, "gucefvfs-async-named", 2 );
        SAsync meta = MetaData( vfs, env.VfsPath( E::ReadmeFile ) );
        REQUIRE( meta.Wait() );
        ASSERT_TRUE( meta.result->result.IsSuccess() );
        CORE::ThreadPoolPtr named = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( "gucefvfs-async-named" );
        ASSERT_TRUE( !named.IsNULL() && named->GetActiveNrOfWorkerThreads() >= 1 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Cancel before the call starts: a call that waits for a thread of a busy pool and is cancelled never runs, its future ends as cancelled and its result says the call has not run; the call before it is not affected" )
    try
    {
        E env( "gucefvfs-async-c1" );
        REQUIRE( env.Setup() );
        VFS::CVFS& vfs = env.Vfs();
        CPoolScope pool( vfs, "gucefvfs-async-one", 1 );
        env.Backend().SetDelayMs( 400 );

        SAsync busy = FileList( vfs, env.VfsPath( "docs" ) );
        REQUIRE( busy.future.HasAFuture() );
        for ( UInt32 w=0; w<2000 && 0 == env.Backend().GetListCount(); ++w )
            MT::ThreadDelay( 1 );
        REQUIRE( 1 == env.Backend().GetListCount() );

        SAsync metaQueued = MetaData( vfs, env.VfsPath( E::MidFile ) );
        VFS::CVfsFetchProgressPtr progress;
        SAsync fetchQueued = FetchFile( vfs, env.VfsPath( E::MidFile ), progress );
        SAsync rangeQueued = ReadRange( vfs, env.VfsPath( E::MidFile ), 0, 100 );
        REQUIRE( metaQueued.future.HasAFuture() && fetchQueued.future.HasAFuture() && rangeQueued.future.HasAFuture() );
        ASSERT_FALSE( metaQueued.future.IsReady() );
        metaQueued.future.RequestCancellation();
        fetchQueued.future.RequestCancellation();
        rangeQueued.future.RequestCancellation();
        ASSERT_TRUE( WaitReady( metaQueued.future, 5000 ) && WaitReady( fetchQueued.future, 5000 ) && WaitReady( rangeQueued.future, 5000 ) );
        ASSERT_TRUE( CORE::TASKSTATUS_CANCELLED == StatusOf( metaQueued.future ) && CORE::TASKSTATUS_CANCELLED == StatusOf( fetchQueued.future ) );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_CANCELLED == metaQueued.result->result.result && !metaQueued.result->result.message.IsNULLOrEmpty() );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_CANCELLED == fetchQueued.result->result.result && 0 == progress->filesTotal.Load() );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_CANCELLED == rangeQueued.result->result.result );

        ASSERT_TRUE( busy.Wait() );
        ASSERT_TRUE( busy.result->result.IsSuccess() && 1 == busy.result->names.size() );
        ASSERT_TRUE( 0 == env.Backend().GetStatCount() && 0 == env.Backend().GetOpenCount() && 0 == env.Backend().GetFetchCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Cancel while a fetch runs: the cancel flag of the progress and the cancel request of the future each stop it between block runs, the result is CANCELLED and the blocks that came stay in the cache; a prefetch stops the same way" )
    try
    {
        E env( "gucefvfs-async-c2" );
        SRemoteEnvOptions options;
        options.maxBlocksPerFetch = 2;
        REQUIRE( env.Setup( options ) );
        VFS::CVFS& vfs = env.Vfs();
        env.Backend().SetDelayMs( 15 );

        for ( UInt32 round=0; round<2; ++round )
        {
            const bool byFuture = 1 == round;
            VFS::CVfsFetchProgressPtr progress( GUCEF_NEW VFS::CVfsFetchProgress() );
            SAsync fetch;
            CORE::CFutureResult* futurePtr = GUCEF_NULL;
            CORE::CFutureResult futureCopy( CORE::TASKSTATUS_QUEUEING_FAILED );
            VFS::CVfsFetchRequest request;
            fetch.future = vfs.FetchFileAsync( env.VfsPath( byFuture ? E::BigFile : E::MidFile ), request, progress, fetch.result );
            REQUIRE( fetch.future.HasAFuture() );
            futureCopy = fetch.future;
            futurePtr = &futureCopy;
            CProgressWatcher watcher( progress.GetPointerAlways(), 2 * B, byFuture ? futurePtr : GUCEF_NULL );
            CTestThreads watching( watcher, 1 );
            ASSERT_TRUE( watching.Start() );
            const bool ended = fetch.Wait();
            watcher.stop.Store( true );
            ASSERT_TRUE( watching.Join( 10000 ) );
            REQUIRE( ended );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_CANCELLED == fetch.result->result.result );
            ASSERT_TRUE( progress->bytesDone.Load() >= 2 * B && progress->bytesDone.Load() < 300000 + ( byFuture ? 700000 : 0 ) );
            ASSERT_TRUE( 1 == progress->filesTotal.Load() && 1 == progress->filesFailed.Load() && 0 == progress->filesDone.Load() );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_CANCELLED == progress->GetLastFailure().result );
            VFS::CVfsCacheState state;
            ASSERT_TRUE( vfs.GetFileCacheState( env.VfsPath( byFuture ? E::BigFile : E::MidFile ), state ) && state.cached && !state.complete && state.bytesHeld >= 2 * B );
            ASSERT_TRUE( 0 == watcher.decreased.Load() && 0 == watcher.pastTotal.Load() );
        }

        // the cancelled fetch goes on from the blocks that it left
        VFS::CVfsFetchProgressPtr again;
        SAsync second = FetchFile( vfs, env.VfsPath( E::MidFile ), again );
        REQUIRE( second.Wait() );
        ASSERT_TRUE( second.result->result.IsSuccess() && again->bytesDone.Load() < 300000 && again->bytesDone.Load() > 0 );

        // a prefetch: told to stop through its future once it has fetched something
        env.Backend().ProvideDigest( E::NotesFile );
        VFS::CVfsFetchRequest request;
        request.files.push_back( env.VfsPath( E::BigFile ) );
        request.files.push_back( env.VfsPath( E::NotesFile ) );
        request.files.push_back( env.VfsPath( E::DeepFile ) );
        request.maxParallelFiles = 1;
        VFS::CVfsFetchProgressPtr prefetchProgress( GUCEF_NEW VFS::CVfsFetchProgress() );
        SAsync prefetch;
        prefetch.future = vfs.PrefetchAsync( request, prefetchProgress, prefetch.result );
        REQUIRE( prefetch.future.HasAFuture() );
        CORE::CFutureResult prefetchFuture = prefetch.future;
        CProgressWatcher watcher( prefetchProgress.GetPointerAlways(), B, &prefetchFuture );
        CTestThreads watching( watcher, 1 );
        ASSERT_TRUE( watching.Start() );
        const bool prefetchEnded = prefetch.Wait();
        watcher.stop.Store( true );
        ASSERT_TRUE( watching.Join( 10000 ) );
        REQUIRE( prefetchEnded );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_CANCELLED == prefetch.result->result.result );
        ASSERT_TRUE( prefetchProgress->bytesDone.Load() < 1000000 );
        VFS::CVfsCacheState state;
        ASSERT_FALSE( vfs.GetFileCacheState( env.VfsPath( E::DeepFile ), state ) );             // never started
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Cancel while a call that is not a fetch runs: a call that is waiting for the remote place is not interrupted and ends with its answer, and a range read that is asked to stop before it reads ends as CANCELLED without reading" )
    try
    {
        E env( "gucefvfs-async-c4" );
        REQUIRE( env.Setup() );
        VFS::CVFS& vfs = env.Vfs();
        env.Backend().SetDelayMs( 250 );

        // the call is at the remote place when the cancel is asked: its answer is the answer
        SAsync meta = MetaData( vfs, env.VfsPath( E::ReadmeFile ) );
        SAsync list = FileList( vfs, env.VfsPath( "docs" ) );
        SAsync open = OpenFile( vfs, env.VfsPath( E::NotesFile ) );
        for ( UInt32 w=0; w<2000 && env.Backend().GetStatCount() + env.Backend().GetListCount() < 3; ++w )
            MT::ThreadDelay( 1 );
        REQUIRE( env.Backend().GetStatCount() + env.Backend().GetListCount() >= 3 );
        meta.future.RequestCancellation();
        list.future.RequestCancellation();
        open.future.RequestCancellation();
        REQUIRE( meta.Wait() && list.Wait() && open.Wait() );
        ASSERT_TRUE( meta.result->result.IsSuccess() && meta.result->exists );
        ASSERT_TRUE( list.result->result.IsSuccess() && 1 == list.result->names.size() );
        ASSERT_TRUE( open.result->result.IsSuccess() && !open.result->resource.IsNULL() );

        // a range read: the cancel comes while the place answers the stat; the read that would follow does not start
        env.Backend().ResetCounters();
        SAsync range = ReadRange( vfs, env.VfsPath( E::MidFile ), 0, 1000 );
        for ( UInt32 w=0; w<2000 && 0 == env.Backend().GetStatCount(); ++w )
            MT::ThreadDelay( 1 );
        REQUIRE( 1 == env.Backend().GetStatCount() );
        range.future.RequestCancellation();
        REQUIRE( range.Wait() );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_CANCELLED == range.result->result.result && 0 == range.result->data.GetDataSize() );
        ASSERT_TRUE( 0 == env.Backend().GetFetchCount() );

        // not cancelled: the same read goes through
        env.Backend().SetDelayMs( 0 );
        SAsync again = ReadRange( vfs, env.VfsPath( E::MidFile ), 0, 1000 );
        REQUIRE( again.Wait() );
        ASSERT_TRUE( again.result->result.IsSuccess() && 1000 == again.result->data.GetDataSize() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Cancel after the call is done: the result and the future stay as they were, for every kind of call" )
    try
    {
        E env( "gucefvfs-async-c3" );
        REQUIRE( env.Setup() );
        VFS::CVFS& vfs = env.Vfs();

        SAsync meta = MetaData( vfs, env.VfsPath( E::ReadmeFile ) );
        VFS::CVfsFetchProgressPtr progress;
        SAsync fetch = FetchFile( vfs, env.VfsPath( E::MidFile ), progress );
        VFS::CVfsFetchRequest request;
        request.files.push_back( env.VfsPath( E::NotesFile ) );
        VFS::CVfsFetchProgressPtr prefetchProgress;
        SAsync prefetch = Prefetch( vfs, request, prefetchProgress );
        SAsync range = ReadRange( vfs, env.VfsPath( E::ReadmeFile ), 0, 100 );
        REQUIRE( meta.Wait() && fetch.Wait() && prefetch.Wait() && range.Wait() );

        meta.future.RequestCancellation();
        fetch.future.RequestCancellation();
        prefetch.future.RequestCancellation();
        range.future.RequestCancellation();
        progress->cancel.Store( true );
        MT::ThreadDelay( 20 );
        ASSERT_TRUE( meta.result->result.IsSuccess() && fetch.result->result.IsSuccess() && prefetch.result->result.IsSuccess() && range.result->result.IsSuccess() );
        ASSERT_TRUE( CORE::TASKSTATUS_FINISHED == StatusOf( meta.future ) && CORE::TASKSTATUS_FINISHED == StatusOf( fetch.future ) );
        ASSERT_TRUE( 300000 == progress->bytesDone.Load() && 1 == progress->filesDone.Load() && 0 == progress->filesFailed.Load() );
        ASSERT_TRUE( 100 == range.result->data.GetDataSize() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Progress of the fetch and the prefetch is monotone, never passes its total, and ends with every byte and every file counted" )
    try
    {
        E env( "gucefvfs-async-p1" );
        SRemoteEnvOptions options;
        options.maxBlocksPerFetch = 2;
        REQUIRE( env.Setup( options ) );
        VFS::CVFS& vfs = env.Vfs();
        env.Backend().SetDelayMs( 10 );

        {
            VFS::CVfsFetchProgressPtr progress( GUCEF_NEW VFS::CVfsFetchProgress() );
            SAsync fetch;
            VFS::CVfsFetchRequest request;
            fetch.future = vfs.FetchFileAsync( env.VfsPath( E::BigFile ), request, progress, fetch.result );
            REQUIRE( fetch.future.HasAFuture() );
            CProgressWatcher watcher( progress.GetPointerAlways(), 0, GUCEF_NULL );
            CTestThreads watching( watcher, 1 );
            ASSERT_TRUE( watching.Start() );
            const bool ended = fetch.Wait();
            watcher.stop.Store( true );
            ASSERT_TRUE( watching.Join( 10000 ) );
            REQUIRE( ended );
            ASSERT_TRUE( fetch.result->result.IsSuccess() );
            ASSERT_TRUE( 1000000 == progress->bytesDone.Load() && 1000000 == progress->bytesTotal.Load() );
            ASSERT_TRUE( 0 == watcher.decreased.Load() && 0 == watcher.pastTotal.Load() && watcher.samples.Load() >= 2 );
        }

        {
            VFS::CVfsFetchRequest request;
            request.files.push_back( env.VfsPath( E::MidFile ) );
            request.files.push_back( env.VfsPath( E::NotesFile ) );
            request.files.push_back( env.VfsPath( E::DeepFile ) );
            request.files.push_back( env.VfsPath( E::ReadmeFile ) );
            request.maxParallelFiles = 3;
            VFS::CVfsFetchProgressPtr progress( GUCEF_NEW VFS::CVfsFetchProgress() );
            SAsync prefetch;
            prefetch.future = vfs.PrefetchAsync( request, progress, prefetch.result );
            REQUIRE( prefetch.future.HasAFuture() );
            CProgressWatcher watcher( progress.GetPointerAlways(), 0, GUCEF_NULL );
            CTestThreads watching( watcher, 1 );
            ASSERT_TRUE( watching.Start() );
            const bool ended = prefetch.Wait();
            watcher.stop.Store( true );
            ASSERT_TRUE( watching.Join( 10000 ) );
            REQUIRE( ended );
            ASSERT_TRUE( prefetch.result->result.IsSuccess() );
            ASSERT_TRUE( 376000 == progress->bytesDone.Load() && 376000 == progress->bytesTotal.Load() );
            ASSERT_TRUE( 4 == progress->filesTotal.Load() && 4 == progress->filesDone.Load() && 0 == progress->filesFailed.Load() );
            ASSERT_TRUE( 0 == watcher.decreased.Load() && 0 == watcher.pastTotal.Load() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A prefetch gives files to other threads of the pool: four files that each take about a quarter of a second go in well under their sum, and one thread of one thread pool does them all by itself" )
    try
    {
        E env( "gucefvfs-async-p2" );
        REQUIRE( env.Setup() );
        VFS::CVFS& vfs = env.Vfs();
        env.Backend().SetDelayMs( 60 );

        VFS::CVfsFetchRequest request;
        request.files.push_back( env.VfsPath( E::MidFile ) );
        request.files.push_back( env.VfsPath( E::NotesFile ) );
        request.files.push_back( env.VfsPath( E::DeepFile ) );
        request.files.push_back( env.VfsPath( E::ReadmeFile ) );
        request.maxParallelFiles = 4;
        VFS::CVfsFetchProgressPtr progress;
        const UInt64 started = NowMs();
        SAsync parallel = Prefetch( vfs, request, progress );
        REQUIRE( parallel.Wait() );
        const UInt64 parallelMs = NowMs() - started;
        ASSERT_TRUE( parallel.result->result.IsSuccess() && 4 == progress->filesDone.Load() );
        // each file takes a stat, an open and a fetch: three calls of 60 ms, twelve in all in a row, three with four threads at it
        ASSERT_TRUE( parallelMs < 12 * 60 * 2 / 3 + 200 );

        // a pool of one thread: no helper can ever run, the call does the four files itself and still ends
        env.Vfs().InvalidateRemoteMetadata();
        env.Backend().ResetCounters();
        env.Backend().SetDelayMs( 0 );
        CPoolScope pool( vfs, "gucefvfs-async-one", 1 );
        VFS::CVfsFetchRequest again = request;
        again.files.push_back( env.VfsPath( E::EmptyFile ) );
        VFS::CVfsFetchProgressPtr againProgress;
        SAsync alone = Prefetch( vfs, again, againProgress );
        REQUIRE( alone.Wait( 30000 ) );
        ASSERT_TRUE( alone.result->result.IsSuccess() && 5 == againProgress->filesDone.Load() && 5 == againProgress->filesTotal.Load() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "The thread model: the call runs on a thread of the pool and the caller never waits, also when the caller is a thread that must not wait; the call itself waits for the remote place on its own thread and gets the file" )
    try
    {
        E env( "gucefvfs-async-t1" );
        REQUIRE( env.Setup() );
        VFS::CVFS& vfs = env.Vfs();
        env.Backend().SetDelayMs( 300 );

        VFS::CVfsThreadPolicy marked( VFS::MISSACTION_FAIL_AND_FETCH );
        VFS::CVfsThreadPolicyScope scope( marked );
        vfs.FileExists( "/nowhere/nothing" );                         // clears what an earlier call of this thread recorded

        const UInt64 started = NowMs();
        SAsync meta = MetaData( vfs, env.VfsPath( E::MidFile ) );
        SAsync open = OpenFile( vfs, env.VfsPath( E::MidFile ) );
        SAsync range = ReadRange( vfs, env.VfsPath( E::MidFile ), 0, 4000 );
        SAsync list = FileList( vfs, env.VfsPath( "data" ) );
        VFS::CVfsFetchProgressPtr progress;
        SAsync fetch = FetchFile( vfs, env.VfsPath( E::BigFile ), progress );
        const UInt64 queuedMs = NowMs() - started;
        ASSERT_TRUE( queuedMs < 100 );                                          // queueing never waits for the place
        ASSERT_FALSE( meta.future.IsReady() );

        // the pulse of this thread is all that it does meanwhile: single thread application
        for ( UInt32 w=0; w<20000 && !( meta.future.IsReady() && open.future.IsReady() && range.future.IsReady() && list.future.IsReady() && fetch.future.IsReady() ); ++w )
        {
            E::Pump();
            MT::ThreadDelay( 1 );
        }
        REQUIRE( meta.Wait( 0 ) && open.Wait( 0 ) && range.Wait( 0 ) && list.Wait( 0 ) && fetch.Wait( 0 ) );
        ASSERT_TRUE( meta.result->result.IsSuccess() && 300000 == meta.result->metaData.resourceSizeInBytes );
        ASSERT_TRUE( open.result->result.IsSuccess() && range.result->result.IsSuccess() && 4000 == range.result->data.GetDataSize() );
        ASSERT_TRUE( env.Matches( E::MidFile, 0, range.result->data, 4000 ) );
        ASSERT_TRUE( list.result->result.IsSuccess() && 2 == list.result->names.size() );
        ASSERT_TRUE( fetch.result->result.IsSuccess() && 1000000 == progress->bytesDone.Load() );
        ASSERT_TRUE( env.Backend().GetLastCallThreadId() != MT::GetCurrentThreadID() );

        // the result of the thread that asked is not touched by the calls on the pool
        VFS::CVfsCallResult last;
        vfs.GetLastResult( last );
        ASSERT_TRUE( last.IsSuccess() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A mount or an unmount on this thread does not wait for a slow call of a remote place that runs asynchronously" )
    try
    {
        E env( "gucefvfs-async-m1" );
        REQUIRE( env.Setup() );
        VFS::CVFS& vfs = env.Vfs();
        env.Backend().SetDelayMs( 700 );

        CBlockCacheTestFolder folder( "gucefvfs-async-lockcheck" );
        CORE::CDynamicBuffer content;
        content.Append( "X", 1 );
        BlockCacheTest::WriteWholeFile( folder.GetChildPath( "x.txt" ), content );

        for ( UInt32 kind=0; kind<4; ++kind )
        {
            env.Backend().ResetCounters();
            VFS::CVfsFetchProgressPtr progress;
            SAsync slow;
            switch ( kind )
            {
                case 0: slow = FileList( vfs, env.VfsPath( "docs" ) ); break;
                case 1: slow = OpenFile( vfs, env.VfsPath( E::MidFile ) ); break;
                case 2: slow = FetchFile( vfs, env.VfsPath( E::NotesFile ), progress ); break;
                default: slow = MetaData( vfs, env.VfsPath( E::BigFile ) ); break;
            }
            REQUIRE( slow.future.HasAFuture() );
            for ( UInt32 w=0; w<2000 && 0 == env.Backend().GetStatCount() + env.Backend().GetListCount(); ++w )
                MT::ThreadDelay( 1 );
            REQUIRE( 0 != env.Backend().GetStatCount() + env.Backend().GetListCount() );
            ASSERT_FALSE( slow.future.IsReady() );

            const UInt64 started = NowMs();
            const bool mounted = vfs.AddRoot( folder.GetPath(), "asynclockcheck", false, false, "/asynclockcheck/" );
            const bool unmounted = vfs.UnmountArchiveByName( "asynclockcheck" );
            const UInt64 elapsed = NowMs() - started;
            ASSERT_TRUE( mounted && unmounted );
            ASSERT_TRUE( elapsed < 400 );
            ASSERT_FALSE( slow.future.IsReady() );                              // the call was still waiting for the place when the mount was done

            REQUIRE( slow.Wait() );
            ASSERT_TRUE( slow.result->result.IsSuccess() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Unmounting the remote mount while calls are under way: the unmount does not wait for them, none of them crashes or hangs, and a call made after it finds no mount" )
    try
    {
        E env( "gucefvfs-async-u1" );
        REQUIRE( env.Setup() );
        VFS::CVFS& vfs = env.Vfs();
        env.Backend().SetDelayMs( 400 );

        SAsync meta = MetaData( vfs, env.VfsPath( E::MidFile ) );
        SAsync list = FileList( vfs, env.VfsPath( "docs" ) );
        SAsync open = OpenFile( vfs, env.VfsPath( E::NotesFile ) );
        SAsync range = ReadRange( vfs, env.VfsPath( E::ReadmeFile ), 0, 100 );
        VFS::CVfsFetchProgressPtr progress;
        SAsync fetch = FetchFile( vfs, env.VfsPath( E::BigFile ), progress );
        VFS::CVfsFetchRequest request;
        request.files.push_back( env.VfsPath( E::MidFile ) );
        request.files.push_back( env.VfsPath( E::DeepFile ) );
        request.directories.push_back( env.VfsPath( "docs" ) );
        request.maxParallelFiles = 3;
        VFS::CVfsFetchProgressPtr prefetchProgress;
        SAsync prefetch = Prefetch( vfs, request, prefetchProgress );
        for ( UInt32 w=0; w<2000 && env.Backend().GetStatCount() + env.Backend().GetListCount() < 3; ++w )
            MT::ThreadDelay( 1 );

        const UInt64 started = NowMs();
        ASSERT_TRUE( env.Unmount() );
        ASSERT_TRUE( NowMs() - started < 300 );

        // the calls hold the archive, and the archive the core that they are inside of: it is not freed under them
        ASSERT_TRUE( env.Backend().IsCoreAlive() );

        // every call ends, with whatever it had got to
        env.Backend().SetDelayMs( 0 );
        ASSERT_TRUE( meta.Wait() && list.Wait() && open.Wait() && range.Wait() && fetch.Wait() && prefetch.Wait() );
        ASSERT_FALSE( fetch.result->result.IsSuccess() && 1000000 != progress->bytesDone.Load() );

        // and it goes when the last of them let go of the archive
        meta.result.Unlink();
        list.result.Unlink();
        open.result.Unlink();
        range.result.Unlink();
        fetch.result.Unlink();
        prefetch.result.Unlink();
        bool gone = false;
        for ( UInt32 w=0; w<3000 && !gone; ++w )
        {
            gone = !env.Backend().IsCoreAlive();
            if ( !gone )
                MT::ThreadDelay( 1 );
        }
        ASSERT_TRUE( gone );

        SAsync after = MetaData( vfs, env.VfsPath( E::MidFile ) );
        REQUIRE( after.Wait() );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_FOUND == after.result->result.result );
        VFS::CVfsFetchProgressPtr afterProgress;
        SAsync afterFetch = FetchFile( vfs, env.VfsPath( E::MidFile ), afterProgress );
        REQUIRE( afterFetch.Wait() );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_SUPPORTED == afterFetch.result->result.result || CORE::FILESYSTEMACCESS_NOT_FOUND == afterFetch.result->result.result );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Unmounting while only a fetch is under way: the fetch holds no pointer to the core but its own use of it, and the archive that the call holds keeps the core until the call is over" )
    try
    {
        E env( "gucefvfs-async-u2" );
        REQUIRE( env.Setup() );
        VFS::CVFS& vfs = env.Vfs();
        env.Backend().SetDelayMs( 300 );

        VFS::CVfsFetchProgressPtr progress;
        SAsync fetch = FetchFile( vfs, env.VfsPath( E::BigFile ), progress );
        REQUIRE( fetch.future.HasAFuture() );
        for ( UInt32 w=0; w<2000 && 0 == env.Backend().GetStatCount(); ++w )
            MT::ThreadDelay( 1 );
        REQUIRE( 1 == env.Backend().GetStatCount() );
        ASSERT_FALSE( fetch.future.IsReady() );

        ASSERT_TRUE( env.Unmount() );
        ASSERT_TRUE( env.Backend().IsCoreAlive() );

        env.Backend().SetDelayMs( 0 );
        REQUIRE( fetch.Wait() );
        ASSERT_FALSE( fetch.result->result.IsSuccess() && 1000000 != progress->bytesDone.Load() );
        ASSERT_TRUE( 1 == progress->filesTotal.Load() && 1 == progress->filesDone.Load() + progress->filesFailed.Load() );
        fetch.result.Unlink();
        bool gone = false;
        for ( UInt32 w=0; w<3000 && !gone; ++w )
        {
            gone = !env.Backend().IsCoreAlive();
            if ( !gone )
                MT::ThreadDelay( 1 );
        }
        ASSERT_TRUE( gone );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A pool that is busy does not lose calls: with one worker and a slow place fifty calls queue behind each other and every one of them ends with the right answer" )
    try
    {
        E env( "gucefvfs-async-x1" );
        REQUIRE( env.Setup() );
        VFS::CVFS& vfs = env.Vfs();
        CPoolScope pool( vfs, "gucefvfs-async-one", 1 );
        env.Backend().SetDelayMs( 20 );

        GUCEF::vector< SAsync > calls;
        for ( UInt32 i=0; i<50; ++i )
        {
            const char* name = 0 == i % 3 ? E::ReadmeFile : ( 1 == i % 3 ? E::TopFile : E::EmptyFile );
            calls.push_back( 0 == i % 2 ? MetaData( vfs, env.VfsPath( name ) ) : ReadRange( vfs, env.VfsPath( name ), 0, 50 ) );
        }
        UInt32 wrong = 0;
        for ( size_t i=0; i<calls.size(); ++i )
        {
            if ( !calls[ i ].Wait( 60000 ) || !calls[ i ].result->result.IsSuccess() )
                ++wrong;
        }
        ASSERT_TRUE( 0 == wrong );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A pool that takes no more work: every call gives a future without a task and a result that says QUEUEING_FAILED, a progress is still handed out, and the calls work again when a pool that works is named" )
    try
    {
        E env( "gucefvfs-async-q1" );
        REQUIRE( env.Setup() );
        VFS::CVFS& vfs = env.Vfs();

        {
            CPoolScope pool( vfs, "gucefvfs-async-refuse", 1 );
            CORE::ThreadPoolPtr refusing = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( "gucefvfs-async-refuse" );
            REQUIRE( !refusing.IsNULL() );
            SAsync warm = MetaData( vfs, env.VfsPath( E::ReadmeFile ) );
            REQUIRE( warm.Wait() );
            refusing->RequestAllThreadsToStop( true, false, 10000 );

            SAsync meta = MetaData( vfs, env.VfsPath( E::ReadmeFile ) );
            SAsync files = FileList( vfs, env.VfsPath( "docs" ) );
            SAsync dirs = DirList( vfs, env.VfsPath( "docs" ) );
            SAsync open = OpenFile( vfs, env.VfsPath( E::ReadmeFile ) );
            SAsync range = ReadRange( vfs, env.VfsPath( E::ReadmeFile ), 0, 10 );
            VFS::CVfsFetchProgressPtr progress;
            SAsync fetch = FetchFile( vfs, env.VfsPath( E::ReadmeFile ), progress );
            VFS::CVfsFetchRequest request;
            request.files.push_back( env.VfsPath( E::ReadmeFile ) );
            VFS::CVfsFetchProgressPtr prefetchProgress;
            SAsync prefetch = Prefetch( vfs, request, prefetchProgress );

            const SAsync* all[ 7 ] = { &meta, &files, &dirs, &open, &range, &fetch, &prefetch };
            for ( UInt32 i=0; i<7; ++i )
            {
                ASSERT_TRUE( all[ i ]->future.HasNoFuture() );
                ASSERT_TRUE( !all[ i ]->result.IsNULL() && CORE::FILESYSTEMACCESS_QUEUEING_FAILED == all[ i ]->result->result.result );
                ASSERT_TRUE( !all[ i ]->result->result.message.IsNULLOrEmpty() );
            }
            ASSERT_TRUE( !progress.IsNULL() && !prefetchProgress.IsNULL() );
        }

        // the default pool is named again by the scope
        SAsync meta = MetaData( vfs, env.VfsPath( E::ReadmeFile ) );
        REQUIRE( meta.Wait() );
        ASSERT_TRUE( meta.result->result.IsSuccess() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "The file system access over the VFS says what a remote mount said: NOT_YET_AVAILABLE for a thread that must not wait, CONNECTION_PENDING while the link is down, and NOT_FOUND only for what the place says is not there" )
    try
    {
        E env( "gucefvfs-async-f1" );
        REQUIRE( env.Setup() );
        VFS::CVfsFileSystemAccess access( env.Vfs(), "test" );

        {
            VFS::CVfsThreadPolicyScope marked( ( VFS::CVfsThreadPolicy( VFS::MISSACTION_FAIL ) ) );
            CORE::CResourceMetaData metaData;
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE == access.TryGetMetaData( env.VfsPath( E::MidFile ), metaData ) );
            CORE::TFileSystemEntryType entryType = CORE::FILESYSTEMENTRYTYPE_FILE;
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE == access.TryGetEntryType( env.VfsPath( E::MidFile ), entryType ) );
            ASSERT_TRUE( CORE::FILESYSTEMENTRYTYPE_NONE == entryType );
            CORE::TFileSystemIteratorPtr iterator;
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE == access.TryGetFileSystemIterator( env.VfsPath( "data" ), iterator ) );
        }

        // a thread that waits: the answers of the place
        CORE::CResourceMetaData metaData;
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access.TryGetMetaData( env.VfsPath( E::MidFile ), metaData ) && 300000 == metaData.resourceSizeInBytes );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access.TryGetMetaData( env.VfsPath( "docs" ), metaData ) );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_FOUND == access.TryGetMetaData( env.VfsPath( "docs/missing.txt" ), metaData ) );
        CORE::TFileSystemEntryType entryType = CORE::FILESYSTEMENTRYTYPE_NONE;
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access.TryGetEntryType( env.VfsPath( E::MidFile ), entryType ) && CORE::FILESYSTEMENTRYTYPE_FILE == entryType );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access.TryGetEntryType( env.VfsPath( "docs/missing.txt" ), entryType ) && CORE::FILESYSTEMENTRYTYPE_NONE == entryType );
        CORE::TFileSystemIteratorPtr iterator;
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access.TryGetFileSystemIterator( env.VfsPath( "docs" ), iterator ) && !iterator.IsNULL() );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_FOUND == access.TryGetFileSystemIterator( env.VfsPath( "nodir" ), iterator ) );

        // the link is down and the place is not known: the cause is the link
        env.Backend().SetLinkUp( false );
        env.Vfs().InvalidateRemoteMetadata();
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == access.TryGetMetaData( env.VfsPath( E::DeepFile ), metaData ) );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == access.TryGetEntryType( env.VfsPath( E::DeepFile ), entryType ) );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == access.TryGetFileSystemIterator( env.VfsPath( "data/sub" ), iterator ) );
        env.Backend().SetLinkUp( true );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "The call of an archive that is not remote is made under the lock of the VFS, as the synchronous call does: an unmount waits for it and the call ends with its answer, where an unmount does not wait for a remote call" )
    try
    {
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        TSlowLocalArchiveFactory factory;
        vfs.RegisterArchiveFactory( "slowlocalarchive", factory );

        CBlockCacheTestFolder folder( "gucefvfs-async-slowlocal" );
        CORE::CDynamicBuffer content;
        content.Append( "SLOW", 4 );
        BlockCacheTest::WriteWholeFile( folder.GetChildPath( "f.txt" ), content );

        VFS::CArchiveSettings settings;
        settings.SetArchiveName( "slowlocal" );
        settings.SetArchiveType( "slowlocalarchive" );
        settings.SetArchivePath( folder.GetPath() );
        settings.SetActualArchivePath( folder.GetPath() );
        settings.SetMountPath( "/slowlocal/" );
        settings.SetWriteableRequested( false );
        const bool mounted = vfs.MountArchive( settings );
        if ( mounted )
        {
            CSlowLocalArchive::inside.Store( 0 );
            CSlowLocalArchive::delayMs.Store( 500 );
            SAsync meta = MetaData( vfs, "/slowlocal/f.txt" );
            for ( UInt32 w=0; w<2000 && 0 == CSlowLocalArchive::inside.Load(); ++w )
                MT::ThreadDelay( 1 );
            const bool entered = 1 == CSlowLocalArchive::inside.Load();
            const UInt64 started = NowMs();
            const bool unmounted = vfs.UnmountArchiveByName( "slowlocal" );
            const UInt64 elapsed = NowMs() - started;
            const bool ended = meta.Wait();
            CSlowLocalArchive::delayMs.Store( 0 );
            vfs.UnregisterArchiveFactory( "slowlocalarchive" );
            REQUIRE( entered && unmounted && ended );
            ASSERT_TRUE( elapsed >= 250 );                          // the unmount came while the call was in the archive and waited for it
            ASSERT_TRUE( meta.result->result.IsSuccess() && meta.result->exists && 4 == meta.result->metaData.resourceSizeInBytes );
        }
        else
        {
            vfs.UnregisterArchiveFactory( "slowlocalarchive" );
        }
        REQUIRE( mounted );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Many callers: eight threads make asynchronous calls of every kind at once and every answer is right; the remote operations do not hang and nothing is left running" )
    try
    {
        E env( "gucefvfs-async-k1" );
        REQUIRE( env.Setup() );
        env.Backend().SetDelayMs( 1 );

        CMixedAsyncCallers callers( &env );
        CTestThreads threads( callers, 8 );
        ASSERT_TRUE( threads.Start() );
        ASSERT_TRUE( threads.Join( 120000 ) );
        ASSERT_TRUE( 0 == callers.wrong.Load() && 0 == callers.notQueued.Load() );
        ASSERT_TRUE( 8 * 30 == callers.calls.Load() );

        // the files that were fetched are whole and right, whatever the order of the fetches was
        VFS::CVfsCacheState state;
        ASSERT_TRUE( env.Vfs().GetFileCacheState( env.VfsPath( E::MidFile ), state ) && state.complete );
        CORE::CDynamicBuffer content;
        ASSERT_TRUE( env.ReadAll( E::MidFile, content ) && env.Matches( E::MidFile, 0, content, content.GetDataSize() ) && 300000 == content.GetDataSize() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Several fetches of one file at once from several threads end with the whole file right, and the sync fetch after them has nothing left to fetch" )
    try
    {
        E env( "gucefvfs-async-k2" );
        REQUIRE( env.Setup() );
        VFS::CVFS& vfs = env.Vfs();
        env.Backend().SetDelayMs( 2 );

        GUCEF::vector< VFS::CVfsFetchProgressPtr > progresses;
        GUCEF::vector< SAsync > calls;
        for ( UInt32 i=0; i<6; ++i )
        {
            progresses.push_back( VFS::CVfsFetchProgressPtr() );
            calls.push_back( FetchFile( vfs, env.VfsPath( E::BigFile ), progresses.back() ) );
        }
        UInt32 failed = 0;
        for ( size_t i=0; i<calls.size(); ++i )
            if ( !calls[ i ].Wait( 60000 ) || !calls[ i ].result->result.IsSuccess() )
                ++failed;
        ASSERT_TRUE( 0 == failed );
        CORE::CDynamicBuffer content;
        ASSERT_TRUE( env.ReadAll( E::BigFile, content ) && 1000000 == content.GetDataSize() && env.Matches( E::BigFile, 0, content, 1000000 ) );
        VFS::CVfsFetchProgress syncProgress;
        ASSERT_TRUE( vfs.FetchFile( env.VfsPath( E::BigFile ), VFS::CVfsFetchRequest(), syncProgress ).IsSuccess() && 0 == syncProgress.bytesDone.Load() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}
