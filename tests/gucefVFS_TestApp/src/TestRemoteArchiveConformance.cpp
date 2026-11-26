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
#include "TestRemoteEnvironment.h"

#include "TestRemoteArchiveConformance.h"

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

typedef CRemoteEnv E;

/** The arrangement the cases run against: made by the environment of each case */
TRemoteEnvFactory g_envFactory = GUCEF_NULL;

/** Makes the environment of a case and ends it with the case */
class CEnvHolder
{
    public:

    explicit CEnvHolder( const char* name ) : m_env( g_envFactory( name ) ) {}
    ~CEnvHolder() { GUCEF_DELETE m_env; }
    E& Get( void ) { return *m_env; }

    private:

    CEnvHolder( const CEnvHolder& src );
    CEnvHolder& operator=( const CEnvHolder& src );

    E* m_env;
};

const UInt32 B = 65536;

VFS::CVfsThreadPolicy
Marked( bool completeOnly = false )
{
    VFS::CVfsThreadPolicy policy( VFS::MISSACTION_FAIL_AND_FETCH );
    policy.openRequiresCompleteFile = completeOnly;
    return policy;
}

UInt64
NowMs( void )
{
    return CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
}

bool
Has( const CORE::CString& text, const CORE::CString& part )
{
    return text.HasSubstr( part ) >= 0;
}

VFS::CVfsCachedIOAccess*
Open( E& env, const CORE::CString& relative, VFS::TBasicVfsResourcePtr& resource )
{
    resource = env.Vfs().GetFile( env.VfsPath( relative ) );
    if ( resource.IsNULL() )
        return GUCEF_NULL;
    return static_cast< VFS::CVfsCachedIOAccess* >( resource->GetAccess().GetPointerAlways() );
}

/** Reads from an offset; the bytes that came */
UInt32
ReadAt( VFS::CVfsCachedIOAccess* access, UInt64 offset, UInt32 count, CORE::CDynamicBuffer& buffer )
{
    buffer.SetDataSize( count );
    access->Setpos( offset );
    const UInt32 got = access->Read( buffer.GetBufferPtr(), 1, count );
    buffer.SetDataSize( got );
    return got;
}

VFS::CVfsRemoteStatistics
Statistics( E& env )
{
    VFS::CVfsRemoteStatistics statistics;
    env.Vfs().GetRemoteStatistics( env.VfsPath( CRemoteEnv::ReadmeFile ), statistics );
    return statistics;
}

/** Reads a whole file on a thread that waits, which fills the cache with it */
bool
CacheWhole( E& env, const char* relative )
{
    CORE::CDynamicBuffer content;
    return env.ReadAll( relative, content ) && env.Matches( relative, 0, content, content.GetDataSize() ) && content.GetDataSize() == env.Backend().GetSize( relative );
}

/** Waits, with the link up, until the kept file is held completely at the revision and pinned: what the archive does on its own for a kept path that changed */
bool
WaitForKept( E& env, const char* relative, const CORE::CString& revision, UInt32 timeoutMs )
{
    for ( UInt32 waited=0; waited<=timeoutMs; waited+=5 )
    {
        E::Pump();
        env.Vfs().FileExists( env.VfsPath( relative ) );        // a call tells the layer of the link
        VFS::CVfsCacheState state;
        if ( env.Vfs().GetFileCacheState( env.VfsPath( relative ), state ) && state.complete && state.pinned && !state.staleRevision && state.revision == revision )
            return true;
        MT::ThreadDelay( 5 );
    }
    return false;
}

/** Reads the whole file in order from several threads at once */
class CReaderThreads : public CTestThreadBody
{
    public:

    E* env;
    MT::CTAtomic< UInt32 > wrong;
    MT::CTAtomic< UInt32 > done;
    explicit CReaderThreads( E* e ) : env( e ), wrong( 0 ), done( 0 ) {}

    virtual void Run( UInt32 index ) GUCEF_VIRTUAL_OVERRIDE
    {
        const char* const names[ 3 ] = { E::BigFile, E::MidFile, E::NotesFile };
        for ( UInt32 round=0; round<3; ++round )
        {
            const char* name = names[ ( index + round ) % 3 ];
            CORE::CDynamicBuffer content;
            if ( !env->ReadAll( name, content ) || !env->Matches( name, 0, content, content.GetDataSize() ) )
                wrong.FetchAdd( 1 );
        }
        done.FetchAdd( 1 );
    }
};

/** Calls the VFS from a thread that must not wait and from one that waits, listings and stats and opens */
class CMixedCallers : public CTestThreadBody
{
    public:

    E* env;
    MT::CTAtomic< UInt32 > wrong;
    explicit CMixedCallers( E* e ) : env( e ), wrong( 0 ) {}

    virtual void Run( UInt32 index ) GUCEF_VIRTUAL_OVERRIDE
    {
        const bool marked = 0 == index % 2;
        VFS::CVfsThreadPolicyScope scope( marked ? VFS::CVfsThreadPolicy( VFS::MISSACTION_FAIL_AND_FETCH ) : VFS::CVfsThreadPolicy() );
        const char* const names[ 4 ] = { E::ReadmeFile, E::NotesFile, E::MidFile, E::DeepFile };
        for ( UInt32 i=0; i<60; ++i )
        {
            const char* name = names[ ( i + index ) % 4 ];
            VFS::CVFS::TStringVector list;
            env->Vfs().GetFileList( list, env->VfsPath( "docs" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX );
            const bool exists = env->Vfs().FileExists( env->VfsPath( name ) );
            if ( !marked && !exists )
                wrong.FetchAdd( 1 );
            if ( exists )
            {
                VFS::TBasicVfsResourcePtr resource = env->Vfs().GetFile( env->VfsPath( name ) );
                if ( !resource.IsNULL() )
                {
                    CORE::CDynamicBuffer buffer( 2000, true );
                    const UInt32 got = resource->GetAccess()->Read( buffer.GetBufferPtr(), 1, 2000 );
                    if ( got > 0 && !env->Backend().CheckExpected( name, 0, buffer.GetConstBufferPtr(), got ) )
                        wrong.FetchAdd( 1 );
                }
                else
                if ( !marked )
                    wrong.FetchAdd( 1 );
            }
        }
    }
};

/** Watches a progress while a fetch runs: monotone, never past the total, and cancels when it is told to */
class CProgressWatcher : public CTestThreadBody
{
    public:

    VFS::CVfsFetchProgress* progress;
    UInt64 cancelAtBytes;
    MT::CTAtomic< bool > stop;
    MT::CTAtomic< UInt32 > decreased;
    MT::CTAtomic< UInt32 > samples;
    MT::CTAtomic< UInt32 > pastTotal;

    CProgressWatcher( VFS::CVfsFetchProgress* p, UInt64 cancelAt ) : progress( p ), cancelAtBytes( cancelAt ), stop( false ), decreased( 0 ), samples( 0 ), pastTotal( 0 ) {}

    virtual void Run( UInt32 /* index */ ) GUCEF_VIRTUAL_OVERRIDE
    {
        UInt64 last = 0;
        while ( !stop.Load() )
        {
            const UInt64 now = progress->bytesDone.Load();
            if ( now < last )
                decreased.FetchAdd( 1 );
            if ( progress->bytesTotal.Load() > 0 && now > progress->bytesTotal.Load() )
                pastTotal.FetchAdd( 1 );
            if ( now != last )
                samples.FetchAdd( 1 );
            last = now;
            if ( cancelAtBytes > 0 && now >= cancelAtBytes )
                progress->cancel.Store( true );
            MT::ThreadDelay( 1 );
        }
    }
};


/** What a synchronous call that failed recorded for the thread: the failure that the asynchronous call of the same name has to give */
VFS::CVfsCallResult
SyncFailure( VFS::CVFS& vfs, CORE::TFileSystemAccessResult fallback )
{
    VFS::CVfsCallResult last;
    vfs.GetLastResult( last );
    return last.IsSuccess() ? VFS::CVfsCallResult( fallback ) : last;
}

/** An asynchronous call: its future and its result */
struct SAsyncCall
{
    CORE::CFutureResult future;
    VFS::CVfsAsyncResultPtr result;
    SAsyncCall( void ) : future( CORE::TASKSTATUS_QUEUEING_FAILED ), result() {}
    bool Wait( UInt32 timeoutMs = 20000 )
    {
        if ( future.HasNoFuture() || result.IsNULL() )
            return false;
        for ( UInt32 waited=0; waited<=timeoutMs; ++waited )
        {
            if ( future.IsReady() )
                return true;
            MT::ThreadDelay( 1 );
        }
        return false;
    }
};

/** What the synchronous way of reading a range gives: the open, the position and the read */
bool
SyncReadRange( VFS::CVFS& vfs, const CORE::CString& path, UInt64 offset, UInt32 length, CORE::CDynamicBuffer& data )
{
    data.SetDataSize( 0 );
    VFS::TBasicVfsResourcePtr resource = vfs.GetFile( path );
    if ( resource.IsNULL() )
        return false;
    CORE::CIOAccess* access = resource->GetAccess().GetPointerAlways();
    const UInt64 size = access->GetSize();
    UInt64 want = length;
    if ( offset >= size )
        want = 0;
    else
    if ( want > size - offset )
        want = size - offset;
    if ( 0 == want )
        return true;
    data.SetDataSize( (UInt32) want );
    access->Setpos( offset );
    const UInt32 got = access->Read( data.GetBufferPtr(), 1, (UInt32) want );
    data.SetDataSize( got );
    return got == want;
}

bool
SameBytes( const CORE::CDynamicBuffer& a, const CORE::CDynamicBuffer& b )
{
    return a.GetDataSize() == b.GetDataSize() && ( 0 == a.GetDataSize() || 0 == memcmp( a.GetConstBufferPtr(), b.GetConstBufferPtr(), a.GetDataSize() ) );
}
}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformRemoteArchiveCases( const char* suiteName )
{
    GUCEF_TESTFW_SUITE_SCOPE( suiteName );

    GUCEF_TESTFW_TESTCASE( "R1: a thread that must not wait asks for a file that is not known: false with NOT_YET_AVAILABLE at once, the arrival of its metadata follows, and the same call then answers; the same for a listing" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-r1" );
        E& env = envHolder.Get();
        REQUIRE( env.Setup() );
        VFS::CVfsThreadPolicyScope marked( Marked() );
        const UInt32 statsBefore = env.Backend().GetStatCount();

        ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( E::MidFile ) ) );
        VFS::CVfsCallResult result;
        env.Vfs().GetLastResult( result );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE == result.result && result.fetchStarted );
        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::MetadataArrivedEvent, E::MidFile, 1, 4000 ) );
        ASSERT_TRUE( statsBefore + 1 == env.Backend().GetStatCount() );
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( E::MidFile ) ) );
        ASSERT_TRUE( 300000 == env.Vfs().GetFileSize( env.VfsPath( E::MidFile ) ) );
        env.Vfs().GetLastResult( result );
        ASSERT_TRUE( result.IsSuccess() );

        // the listing of a directory that is not known
        VFS::CVFS::TStringVector list;
        ASSERT_FALSE( env.Vfs().GetFileList( list, env.VfsPath( "docs" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX ) );
        ASSERT_TRUE( list.empty() );
        env.Vfs().GetLastResult( result );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE == result.result );
        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::ListingArrivedEvent, "docs/", 1, 4000 ) );
        ASSERT_TRUE( env.Vfs().GetFileList( list, env.VfsPath( "docs" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX ) );
        ASSERT_TRUE( 1 == list.size() && "readme.txt" == list[ 0 ] );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "R2: a thread that must not wait opens a file whose metadata is known and never read: the read ends at the first missing block with NOT_YET_AVAILABLE, the arrival of the range follows, and the same read then succeeds with the bytes of the file; read in order the whole file comes" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-r2" );
        E& env = envHolder.Get();
        REQUIRE( env.Setup() );
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( E::MidFile ) ) );

        VFS::CVfsThreadPolicyScope marked( Marked() );
        VFS::TBasicVfsResourcePtr resource;
        VFS::CVfsCachedIOAccess* access = Open( env, E::MidFile, resource );
        REQUIRE( GUCEF_NULL != access );
        ASSERT_TRUE( 0 == env.Backend().GetOpenCount() );              // no handle at the open

        CORE::CDynamicBuffer buffer;
        ASSERT_TRUE( 0 == ReadAt( access, 0, 1000, buffer ) );
        ASSERT_FALSE( access->Eof() );
        VFS::CVfsCallResult result = access->GetLastResult();
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE == result.result && result.fetchStarted );
        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::FileArrivedEvent, E::MidFile, 1, 4000 ) );

        CTestEventRecorder::TRecordVector arrived = env.Events().Find( VFS::CRemoteArchiveEvents::FileArrivedEvent, env.VfsPath( E::MidFile ) );
        ASSERT_TRUE( 1 == arrived.size() );
        ASSERT_TRUE( 0 == arrived[ 0 ].firstByte && B == arrived[ 0 ].byteCount && !arrived[ 0 ].fileComplete );

        ASSERT_TRUE( 1000 == ReadAt( access, 0, 1000, buffer ) );
        ASSERT_TRUE( env.Matches( E::MidFile, 0, buffer, 1000 ) );
        ASSERT_TRUE( access->GetLastResult().IsSuccess() );

        // a read that goes past the end of the first block is a short read, and ends at the block that is missing
        ASSERT_TRUE( B - 10 == ReadAt( access, 10, 2 * B, buffer ) );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE == access->GetLastResult().result );
        ASSERT_FALSE( access->Eof() );

        // the whole file in order, one arrival per missing block
        UInt64 position = 0;
        const UInt64 size = 300000;
        CORE::CDynamicBuffer whole;
        for ( UInt32 round=0; round<20 && position < size; ++round )
        {
            const UInt32 got = ReadAt( access, position, 50000, buffer );
            ASSERT_TRUE( env.Matches( E::MidFile, position, buffer, got ) );
            position += got;
            if ( got < 50000 && position < size )
            {
                ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE == access->GetLastResult().result );
                const UInt32 wanted = env.Events().Count( VFS::CRemoteArchiveEvents::FileArrivedEvent ) + 1;
                REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::FileArrivedEvent, E::MidFile, wanted, 4000 ) );
            }
        }
        ASSERT_TRUE( size == position );
        access->Setpos( size );
        ASSERT_TRUE( access->Eof() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "R3: a thread that must not wait is answered from stale metadata at once and the metadata is checked in the background: no event when nothing changed, the arrival of the new revision when it did" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-r3" );
        E& env = envHolder.Get();
        SRemoteEnvOptions options;
        options.metadataTtlMs = 120;
        REQUIRE( env.Setup( options ) );
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( E::MidFile ) ) );
        ASSERT_TRUE( 1 == env.Backend().GetStatCount() );

        VFS::CVfsThreadPolicyScope marked( Marked() );
        MT::ThreadDelay( 250 );
        ASSERT_TRUE( VFS::CIRemoteArchive::METADATA_STALE == env.Core()->GetMetadataState( E::MidFile ) );
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( E::MidFile ) ) );
        REQUIRE( env.WaitUntilIdle( 4000 ) );
        ASSERT_TRUE( 2 == env.Backend().GetStatCount() );
        ASSERT_TRUE( 300000 == env.Vfs().GetFileSize( env.VfsPath( E::MidFile ) ) );
        ASSERT_TRUE( 0 == env.Events().Count( VFS::CRemoteArchiveEvents::MetadataArrivedEvent ) );
        ASSERT_TRUE( 0 == env.Events().Count( VFS::CRemoteArchiveEvents::FetchFailedEvent ) );

        // the file changed: the stale answer still comes at once, the arrival tells the new revision
        const CORE::CString oldRevision = env.Backend().GetRevision( E::MidFile );
        env.Backend().ChangeFileSize( E::MidFile, 310000 );
        MT::ThreadDelay( 250 );
        ASSERT_TRUE( VFS::CIRemoteArchive::METADATA_STALE == env.Core()->GetMetadataState( E::MidFile ) );
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( E::MidFile ) ) );
        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::MetadataArrivedEvent, E::MidFile, 1, 4000 ) );
        CTestEventRecorder::TRecordVector records = env.Events().Find( VFS::CRemoteArchiveEvents::MetadataArrivedEvent );
        ASSERT_TRUE( 1 == records.size() );
        ASSERT_TRUE( records[ 0 ].revision == env.Backend().GetRevision( E::MidFile ) && records[ 0 ].oldRevision == oldRevision );
        ASSERT_TRUE( 310000 == env.Vfs().GetFileSize( env.VfsPath( E::MidFile ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "R4: with openRequiresCompleteFile a file opens only when every block is held: the open fails with NOT_YET_AVAILABLE and one fetch of the whole file runs, then it opens and reads equal bytes" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-r4" );
        E& env = envHolder.Get();
        REQUIRE( env.Setup() );
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( E::MidFile ) ) );

        env.Backend().SetDelayMs( 80 );
        VFS::CVfsThreadPolicyScope marked( Marked( true ) );
        ASSERT_TRUE( env.Vfs().GetFile( env.VfsPath( E::MidFile ) ).IsNULL() );
        VFS::CVfsCallResult result;
        env.Vfs().GetLastResult( result );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE == result.result );
        ASSERT_TRUE( env.Vfs().GetFile( env.VfsPath( E::MidFile ) ).IsNULL() );
        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::FileArrivedEvent, E::MidFile, 1, 4000 ) );
        env.Backend().SetDelayMs( 0 );
        REQUIRE( env.WaitUntilIdle( 4000 ) );

        CTestEventRecorder::TRecordVector arrived = env.Events().Find( VFS::CRemoteArchiveEvents::FileArrivedEvent );
        ASSERT_TRUE( 1 == arrived.size() && arrived[ 0 ].fileComplete && 300000 == arrived[ 0 ].byteCount );

        VFS::TBasicVfsResourcePtr resource;
        VFS::CVfsCachedIOAccess* access = Open( env, E::MidFile, resource );
        REQUIRE( GUCEF_NULL != access );
        CORE::CDynamicBuffer buffer;
        ASSERT_TRUE( 300000 == ReadAt( access, 0, 300000, buffer ) );
        ASSERT_TRUE( env.Matches( E::MidFile, 0, buffer, 300000 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "R5: with the policy FAIL a miss starts no job: nothing reaches the remote place and no event follows, for an unknown file, an unknown listing and a read" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-r5" );
        E& env = envHolder.Get();
        REQUIRE( env.Setup() );
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( E::MidFile ) ) );
        env.Backend().ResetCounters();

        VFS::CVfsThreadPolicyScope scope( ( VFS::CVfsThreadPolicy( VFS::MISSACTION_FAIL ) ) );
        ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( E::ReadmeFile ) ) );
        VFS::CVfsCallResult result;
        env.Vfs().GetLastResult( result );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE == result.result && !result.fetchStarted );
        VFS::CVFS::TStringVector list;
        ASSERT_FALSE( env.Vfs().GetFileList( list, env.VfsPath( "docs" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX ) );

        VFS::TBasicVfsResourcePtr resource;
        VFS::CVfsCachedIOAccess* access = Open( env, E::MidFile, resource );
        REQUIRE( GUCEF_NULL != access );
        CORE::CDynamicBuffer buffer;
        ASSERT_TRUE( 0 == ReadAt( access, 0, 100, buffer ) );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE == access->GetLastResult().result && !access->GetLastResult().fetchStarted );

        REQUIRE( env.WaitUntilIdle( 4000 ) );
        MT::ThreadDelay( 30 );
        CRemoteEnv::Pump();
        ASSERT_TRUE( 0 == env.Backend().GetStatCount() && 0 == env.Backend().GetListCount() && 0 == env.Backend().GetOpenCount() && 0 == env.Backend().GetFetchCount() );
        ASSERT_TRUE( 0 == env.Events().GetTotal() );
        VFS::CVfsFetchScheduler::SCounters counters;
        env.Scheduler().GetCounters( counters );
        ASSERT_TRUE( 0 == counters.accepted );
        ASSERT_TRUE( Statistics( env ).policyMisses >= 3 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "R6: with a remote that takes 500 ms per call every call of a thread that must not wait returns within 50 ms while the pool waits for the remote" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-r6" );
        E& env = envHolder.Get();
        REQUIRE( env.Setup() );
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( E::MidFile ) ) );
        env.Backend().SetDelayMs( 500 );

        VFS::CVfsThreadPolicyScope marked( Marked() );
        UInt64 slowest = 0;
        UInt64 start = NowMs();
        ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( E::ReadmeFile ) ) );
        slowest = NowMs() - start;
        start = NowMs();
        ASSERT_TRUE( env.Vfs().GetFile( env.VfsPath( E::NotesFile ) ).IsNULL() );
        slowest = NowMs() - start > slowest ? NowMs() - start : slowest;
        start = NowMs();
        VFS::CVFS::TStringVector list;
        env.Vfs().GetFileList( list, env.VfsPath( "docs" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX );
        slowest = NowMs() - start > slowest ? NowMs() - start : slowest;
        start = NowMs();
        VFS::TBasicVfsResourcePtr resource;
        VFS::CVfsCachedIOAccess* access = Open( env, E::MidFile, resource );
        REQUIRE( GUCEF_NULL != access );
        CORE::CDynamicBuffer buffer;
        ASSERT_TRUE( 0 == ReadAt( access, 0, 100, buffer ) );
        slowest = NowMs() - start > slowest ? NowMs() - start : slowest;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "R6: the slowest call of the thread that must not wait took " << (UInt32) slowest << " ms" );
        ASSERT_TRUE( slowest < 50 );

        // the pool is waiting for the remote meanwhile
        ASSERT_TRUE( env.Scheduler().GetRunningCount() > 0 );
        env.Backend().SetDelayMs( 0 );
        REQUIRE( env.WaitUntilIdle( 4000 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "R7: two observers each get each event once; one that subscribes after an event gets nothing of it" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-r7" );
        E& env = envHolder.Get();
        REQUIRE( env.Setup() );
        CTestEventRecorder second;
        second.Listen( &env.Vfs() );
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( E::MidFile ) ) );

        VFS::CVfsThreadPolicyScope marked( Marked( true ) );
        ASSERT_TRUE( env.Vfs().GetFile( env.VfsPath( E::MidFile ) ).IsNULL() );
        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::FileArrivedEvent, E::MidFile, 1, 4000 ) );
        CRemoteEnv::Pump();
        ASSERT_TRUE( 1 == env.Events().Count( VFS::CRemoteArchiveEvents::FileArrivedEvent ) );
        ASSERT_TRUE( 1 == second.Count( VFS::CRemoteArchiveEvents::FileArrivedEvent ) );

        CTestEventRecorder late;
        late.Listen( &env.Vfs() );
        CRemoteEnv::Pump();
        ASSERT_TRUE( 0 == late.GetTotal() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "R8: with the link down a file that was read opens and reads equal bytes, and the stat family and the digest answer from what is known, without a call to the remote place" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-r8" );
        E& env = envHolder.Get();
        REQUIRE( env.Setup() );
        env.Backend().ProvideDigest( E::MidFile );
        ASSERT_TRUE( CacheWhole( env, E::MidFile ) );
        const VFS::CVfsDigest expectedDigest = env.Backend().ComputeDigest( E::MidFile, VFS::CVfsDigest::ALGORITHM_SHA256 );
        env.Vfs().InvalidateRemoteMetadata();
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( E::MidFile ) ) );
        env.Backend().SetLinkUp( false );
        env.Backend().ResetCounters();

        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( E::MidFile ) ) );
        ASSERT_TRUE( 300000 == env.Vfs().GetFileSize( env.VfsPath( E::MidFile ) ) );
        CORE::CResourceMetaData metaData;
        ASSERT_TRUE( env.Vfs().GetFileMetaData( env.VfsPath( E::MidFile ), metaData ) && metaData.resourceExists && 300000 == metaData.resourceSizeInBytes );
        VFS::CVfsDigest digest;
        ASSERT_TRUE( env.Vfs().GetFileDigest( env.VfsPath( E::MidFile ), digest ) && digest.Matches( expectedDigest ) );

        CORE::CDynamicBuffer content;
        VFS::CVfsCallResult readResult;
        ASSERT_TRUE( env.ReadAll( E::MidFile, content, &readResult ) );
        ASSERT_TRUE( env.Matches( E::MidFile, 0, content, 300000 ) );
        ASSERT_TRUE( readResult.IsSuccess() );
        ASSERT_TRUE( 0 == env.Backend().GetStatCount() && 0 == env.Backend().GetOpenCount() && 0 == env.Backend().GetFetchCount() );
        ASSERT_TRUE( Statistics( env ).offlineReads > 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "R9: with the link down a partly cached file reads in the range that is cached, and a read that reaches a missing block ends there with CONNECTION_PENDING and a message with the file, the bytes and the place" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-r9" );
        E& env = envHolder.Get();
        REQUIRE( env.Setup() );
        {
            VFS::TBasicVfsResourcePtr resource;
            VFS::CVfsCachedIOAccess* access = Open( env, E::BigFile, resource );
            REQUIRE( GUCEF_NULL != access );
            CORE::CDynamicBuffer buffer;
            ASSERT_TRUE( 2 * B == ReadAt( access, 0, 2 * B, buffer ) );
        }
        env.Backend().SetLinkUp( false );

        VFS::TBasicVfsResourcePtr resource;
        VFS::CVfsCachedIOAccess* access = Open( env, E::BigFile, resource );
        REQUIRE( GUCEF_NULL != access );
        CORE::CDynamicBuffer buffer;
        ASSERT_TRUE( 2 * B == ReadAt( access, 0, 2 * B, buffer ) );
        ASSERT_TRUE( env.Matches( E::BigFile, 0, buffer, 2 * B ) );

        const UInt32 got = ReadAt( access, 2 * B - 72, 200, buffer );
        ASSERT_TRUE( 72 == got );
        const VFS::CVfsCallResult result = access->GetLastResult();
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == result.result );
        ASSERT_TRUE( Has( result.message, E::BigFile ) && Has( result.message, "131072-131199" ) && Has( result.message, env.GetPlaceText() ) );
        ASSERT_FALSE( access->Eof() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "R10: with the link down a file that was never cached cannot be opened: false and NULL with CONNECTION_PENDING and no job, also for a thread that must not wait; what is not known is not asked" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-r10" );
        E& env = envHolder.Get();
        REQUIRE( env.Setup() );
        // listed, never read
        VFS::CVFS::TStringVector list;
        ASSERT_TRUE( env.Vfs().GetFileList( list, env.VfsPath( "data" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX ) );
        env.Backend().SetLinkUp( false );
        env.Backend().ResetCounters();

        ASSERT_TRUE( env.Vfs().GetFile( env.VfsPath( E::BigFile ) ).IsNULL() );
        VFS::CVfsCallResult result;
        env.Vfs().GetLastResult( result );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == result.result );

        VFS::CVfsThreadPolicyScope marked( Marked() );
        ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( E::ReadmeFile ) ) );
        env.Vfs().GetLastResult( result );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == result.result && !result.fetchStarted );
        ASSERT_TRUE( env.Vfs().GetFile( env.VfsPath( E::ReadmeFile ) ).IsNULL() );
        ASSERT_TRUE( env.Vfs().GetFile( env.VfsPath( E::BigFile ) ).IsNULL() );

        env.WaitUntilIdle( 5000 );
        ASSERT_TRUE( 0 == env.Backend().GetStatCount() && 0 == env.Backend().GetListCount() && 0 == env.Backend().GetOpenCount() && 0 == env.Backend().GetFetchCount() );
        VFS::CVfsFetchScheduler::SCounters counters;
        env.Scheduler().GetCounters( counters );
        ASSERT_TRUE( 0 == counters.accepted );
        ASSERT_TRUE( Statistics( env ).offlineMisses >= 3 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "R11: with the link down a listing is made from the files the cache holds, with the result and message that say so, and nothing else is invented; with the link up it is the whole listing" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-r11" );
        E& env = envHolder.Get();
        REQUIRE( env.Setup() );
        ASSERT_TRUE( CacheWhole( env, E::ReadmeFile ) );
        env.Vfs().InvalidateRemoteMetadata();

        // online: the listing is the whole one: a file and a directory
        VFS::CVFS::TStringVector files, dirs;
        ASSERT_TRUE( env.Vfs().GetFileList( files, env.VfsPath( "docs" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX ) );
        ASSERT_TRUE( env.Vfs().GetDirList( dirs, env.VfsPath( "docs" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX ) );
        ASSERT_TRUE( 1 == files.size() && 1 == dirs.size() );

        env.Backend().SetLinkUp( false );
        env.Vfs().InvalidateRemoteMetadata();
        files.clear();
        dirs.clear();
        ASSERT_FALSE( env.Vfs().GetFileList( files, env.VfsPath( "docs" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX ) );
        VFS::CVfsCallResult result;
        env.Vfs().GetLastResult( result );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == result.result );
        ASSERT_TRUE( result.message == "offline: only files kept in the cache of this computer are listed" );
        ASSERT_TRUE( 1 == files.size() && "readme.txt" == files[ 0 ] );
        // the directory below has no file in the cache: it is not listed
        ASSERT_FALSE( env.Vfs().GetDirList( dirs, env.VfsPath( "docs" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX ) );
        ASSERT_TRUE( dirs.empty() );

        // one block of a file in the directory below is cached: the directory is implied
        env.Backend().SetLinkUp( true );
        {
            VFS::TBasicVfsResourcePtr resource;
            VFS::CVfsCachedIOAccess* access = Open( env, E::NotesFile, resource );
            REQUIRE( GUCEF_NULL != access );
            CORE::CDynamicBuffer buffer;
            ASSERT_TRUE( 100 == ReadAt( access, 0, 100, buffer ) );
        }
        env.Backend().SetLinkUp( false );
        env.Vfs().InvalidateRemoteMetadata();
        dirs.clear();
        env.Vfs().GetDirList( dirs, env.VfsPath( "docs" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX );
        ASSERT_TRUE( 1 == dirs.size() && "notes/" == dirs[ 0 ] );
        // a directory with nothing cached in it, and one that is not there: nothing
        files.clear();
        env.Vfs().GetFileList( files, env.VfsPath( "data" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX );
        ASSERT_TRUE( files.empty() );
        env.Vfs().GetFileList( files, env.VfsPath( "nowhere" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX );
        ASSERT_TRUE( files.empty() );
        ASSERT_TRUE( env.Vfs().DirExists( env.VfsPath( "docs" ) ) );
        ASSERT_FALSE( env.Vfs().DirExists( env.VfsPath( "data" ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "R12: after a restart (a new archive and a new cache on the same folder) while the link is down, R8 to R11 hold with no metadata in memory" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-r12" );
        E& env = envHolder.Get();
        REQUIRE( env.Setup() );
        env.Backend().ProvideDigest( E::MidFile );
        ASSERT_TRUE( CacheWhole( env, E::MidFile ) );
        ASSERT_TRUE( CacheWhole( env, E::ReadmeFile ) );
        {
            VFS::TBasicVfsResourcePtr resource;
            VFS::CVfsCachedIOAccess* access = Open( env, E::BigFile, resource );
            REQUIRE( GUCEF_NULL != access );
            CORE::CDynamicBuffer buffer;
            ASSERT_TRUE( 2 * B == ReadAt( access, 0, 2 * B, buffer ) );
        }
        const VFS::CVfsDigest expectedDigest = env.Backend().ComputeDigest( E::MidFile, VFS::CVfsDigest::ALGORITHM_SHA256 );
        env.Backend().SetLinkUp( false );
        REQUIRE( env.Restart( true ) );
        env.Backend().ResetCounters();

        // R8
        ASSERT_TRUE( VFS::CIRemoteArchive::METADATA_UNKNOWN == env.Core()->GetMetadataState( E::MidFile ) );
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( E::MidFile ) ) );
        ASSERT_TRUE( 300000 == env.Vfs().GetFileSize( env.VfsPath( E::MidFile ) ) );
        VFS::CVfsDigest digest;
        ASSERT_TRUE( env.Vfs().GetFileDigest( env.VfsPath( E::MidFile ), digest ) && digest.Matches( expectedDigest ) );
        CORE::CDynamicBuffer content;
        ASSERT_TRUE( env.ReadAll( E::MidFile, content ) && env.Matches( E::MidFile, 0, content, 300000 ) );

        // R9
        {
            VFS::TBasicVfsResourcePtr resource;
            VFS::CVfsCachedIOAccess* access = Open( env, E::BigFile, resource );
            REQUIRE( GUCEF_NULL != access );
            CORE::CDynamicBuffer buffer;
            ASSERT_TRUE( 2 * B == ReadAt( access, 0, 2 * B, buffer ) && env.Matches( E::BigFile, 0, buffer, 2 * B ) );
            ASSERT_TRUE( 0 == ReadAt( access, 2 * B, 100, buffer ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == access->GetLastResult().result );
        }

        // R10
        ASSERT_TRUE( env.Vfs().GetFile( env.VfsPath( E::NotesFile ) ).IsNULL() );
        ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( E::NotesFile ) ) );

        // R11
        VFS::CVFS::TStringVector files;
        ASSERT_FALSE( env.Vfs().GetFileList( files, env.VfsPath( "data" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX ) );
        ASSERT_TRUE( 2 == files.size() );       // mid.bin and big.bin
        files.clear();
        ASSERT_FALSE( env.Vfs().GetFileList( files, env.VfsPath( "docs" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX ) );
        ASSERT_TRUE( 1 == files.size() && "readme.txt" == files[ 0 ] );

        ASSERT_TRUE( 0 == env.Backend().GetStatCount() && 0 == env.Backend().GetListCount() && 0 == env.Backend().GetFetchCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "R13: when the link comes back the state change is raised and the next call works with no remount" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-r13" );
        E& env = envHolder.Get();
        REQUIRE( env.Setup() );
        env.Backend().SetLinkUp( false );
        ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( E::MidFile ) ) );
        env.Backend().SetLinkUp( true );
        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::ConnectionStateChangedEvent, CORE::CString::Empty, 2, 4000 ) );
        CTestEventRecorder::TRecordVector records = env.Events().Find( VFS::CRemoteArchiveEvents::ConnectionStateChangedEvent );
        ASSERT_TRUE( 2 == records.size() && !records[ 0 ].connected && records[ 1 ].connected );
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( E::MidFile ) ) );
        CORE::CDynamicBuffer content;
        ASSERT_TRUE( env.ReadAll( E::MidFile, content ) && env.Matches( E::MidFile, 0, content, 300000 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "R14: a file that changed at the remote place: an open file reads what it holds at its revision and fails as CHANGED at its next uncached read, FileChanged is raised, the old copy is superseded and not offered offline, the next open reads the new content and no byte of the old revision is in it" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-r14" );
        E& env = envHolder.Get();
        REQUIRE( env.Setup() );
        VFS::TBasicVfsResourcePtr resource;
        VFS::CVfsCachedIOAccess* access = Open( env, E::BigFile, resource );
        REQUIRE( GUCEF_NULL != access );
        CORE::CDynamicBuffer first, firstAgain, buffer;
        ASSERT_TRUE( B == ReadAt( access, 0, B, first ) );
        ASSERT_TRUE( env.Matches( E::BigFile, 0, first, B ) );
        const CORE::CString oldRevision = access->GetRevision();
        ASSERT_TRUE( oldRevision == env.Backend().GetRevision( E::BigFile ) );

        env.Backend().ChangeFile( E::BigFile );
        const CORE::CString newRevision = env.Backend().GetRevision( E::BigFile );
        ASSERT_TRUE( newRevision != oldRevision );

        // what is held at the old revision still reads, until a block is missing
        ASSERT_TRUE( B == ReadAt( access, 0, B, firstAgain ) );
        ASSERT_TRUE( 0 == memcmp( first.GetConstBufferPtr(), firstAgain.GetConstBufferPtr(), B ) );
        ASSERT_FALSE( env.Matches( E::BigFile, 0, firstAgain, B ) );            // the remote bytes are other bytes now

        ASSERT_TRUE( 0 == ReadAt( access, 5 * B, 1000, buffer ) );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_RESOURCE_CHANGED == access->GetLastResult().result );
        ASSERT_FALSE( access->IsValid() );
        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::FileChangedEvent, E::BigFile, 1, 4000 ) );
        ASSERT_TRUE( oldRevision == env.Events().Find( VFS::CRemoteArchiveEvents::FileChangedEvent, env.VfsPath( E::BigFile ) )[ 0 ].oldRevision );

        // the next open reads the new content, all of it
        VFS::TBasicVfsResourcePtr second;
        VFS::CVfsCachedIOAccess* fresh = Open( env, E::BigFile, second );
        REQUIRE( GUCEF_NULL != fresh );
        ASSERT_TRUE( newRevision == fresh->GetRevision() );
        CORE::CDynamicBuffer all;
        ASSERT_TRUE( 1000000 == ReadAt( fresh, 0, 1000000, all ) );
        ASSERT_TRUE( env.Matches( E::BigFile, 0, all, 1000000 ) );

        // the old copy is superseded
        const VFS::CVfsBlockCache::TEntryHandle oldEntry = env.Cache()->AcquireEntry( "remote-test-place", E::BigFile, oldRevision, 1000000, VFS::CVfsDigest(), false );
        ASSERT_TRUE( 0 != oldEntry );
        VFS::CVfsBlockCache::SEntryInfo oldInfo;
        ASSERT_TRUE( env.Cache()->GetEntryInfo( oldEntry, oldInfo ) && oldInfo.superseded );
        env.Cache()->ReleaseEntry( oldEntry );
        VFS::CVfsBlockCache::SEntryInfo newest;
        ASSERT_TRUE( env.Cache()->FindNewestRevision( "remote-test-place", E::BigFile, newest ) && newest.revision == newRevision );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "R14b: a copy that is superseded is not served while the link is down: the file is not available offline although its old blocks are in the cache" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-r14b" );
        E& env = envHolder.Get();
        REQUIRE( env.Setup() );
        ASSERT_TRUE( CacheWhole( env, E::MidFile ) );
        env.Backend().ChangeFile( E::MidFile );
        env.Vfs().InvalidateRemoteMetadata();
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( E::MidFile ) ) );          // learns the new revision: the old copy is superseded
        env.Backend().SetLinkUp( false );
        env.Vfs().InvalidateRemoteMetadata();
        ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( E::MidFile ) ) );
        ASSERT_TRUE( env.Vfs().GetFile( env.VfsPath( E::MidFile ) ).IsNULL() );
        VFS::CVfsCallResult result;
        env.Vfs().GetLastResult( result );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == result.result );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "R15: a pinned file survives a cache that evicts everything else, unpinning releases it, and the pin follows a newer complete revision" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-r15" );
        E& env = envHolder.Get();
        SRemoteEnvOptions options;
        options.cacheMaxBytes = 100000;
        REQUIRE( env.Setup( options ) );

        VFS::CVfsFetchProgress progress;
        VFS::CVfsFetchRequest request;
        request.pin = true;
        ASSERT_TRUE( env.Vfs().FetchFile( env.VfsPath( E::NotesFile ), request, progress ).IsSuccess() );
        VFS::CVfsCacheState state;
        ASSERT_TRUE( env.Vfs().GetFileCacheState( env.VfsPath( E::NotesFile ), state ) && state.complete && state.pinned );

        // other files pass through a cache that is too small for them
        ASSERT_TRUE( CacheWhole( env, E::MidFile ) );
        ASSERT_TRUE( CacheWhole( env, E::ReadmeFile ) );
        ASSERT_TRUE( CacheWhole( env, E::TopFile ) );
        ASSERT_FALSE( env.Vfs().GetFileCacheState( env.VfsPath( E::MidFile ), state ) && state.cached );
        {
            const VFS::CVfsRemoteStatistics s = Statistics( env );
            ASSERT_TRUE( 1 == s.pinnedEntries && 70000 == s.pinnedBytes && 100000 == s.maxBytes );
            ASSERT_TRUE( s.evictedEntries >= 1 && s.evictedBytes > 0 );
        }
        ASSERT_TRUE( env.Vfs().GetFileCacheState( env.VfsPath( E::NotesFile ), state ) && state.complete && state.pinned );

        // unpinned, it goes like the others
        ASSERT_TRUE( env.Vfs().SetFilePinned( env.VfsPath( E::NotesFile ), false ) );
        ASSERT_TRUE( CacheWhole( env, E::MidFile ) );
        ASSERT_TRUE( CacheWhole( env, E::ReadmeFile ) );
        ASSERT_FALSE( env.Vfs().GetFileCacheState( env.VfsPath( E::NotesFile ), state ) && state.cached );

        // a pin needs an entry: an unknown file cannot be pinned
        ASSERT_FALSE( env.Vfs().SetFilePinned( env.VfsPath( "nofile" ), true ) );

        // a file that was read, not fetched, is pinned by the call and then survives the passing of other files
        ASSERT_TRUE( CacheWhole( env, E::TopFile ) );
        ASSERT_TRUE( env.Vfs().GetFileCacheState( env.VfsPath( E::TopFile ), state ) && !state.pinned );
        ASSERT_TRUE( env.Vfs().SetFilePinned( env.VfsPath( E::TopFile ), true ) );
        ASSERT_TRUE( env.Vfs().GetFileCacheState( env.VfsPath( E::TopFile ), state ) && state.pinned );
        ASSERT_TRUE( CacheWhole( env, E::MidFile ) );
        ASSERT_TRUE( CacheWhole( env, E::NotesFile ) );
        ASSERT_TRUE( CacheWhole( env, E::ReadmeFile ) );
        ASSERT_TRUE( env.Vfs().GetFileCacheState( env.VfsPath( E::TopFile ), state ) && state.cached && state.complete && state.pinned );
        ASSERT_TRUE( env.Vfs().SetFilePinned( env.VfsPath( E::TopFile ), false ) );
        ASSERT_TRUE( env.Vfs().GetFileCacheState( env.VfsPath( E::TopFile ), state ) && !state.pinned );

        // the pin follows a newer complete revision
        VFS::CVfsFetchProgress second;
        ASSERT_TRUE( env.Vfs().FetchFile( env.VfsPath( E::NotesFile ), request, second ).IsSuccess() );
        const CORE::CString oldRevision = env.Backend().GetRevision( E::NotesFile );
        env.Backend().ChangeFile( E::NotesFile );
        env.Vfs().InvalidateRemoteMetadata();
        VFS::CVfsFetchProgress third;
        VFS::CVfsFetchRequest noPin;
        ASSERT_TRUE( env.Vfs().FetchFile( env.VfsPath( E::NotesFile ), noPin, third ).IsSuccess() );
        ASSERT_TRUE( env.Vfs().GetFileCacheState( env.VfsPath( E::NotesFile ), state ) );
        ASSERT_TRUE( state.complete && state.pinned && state.revision == env.Backend().GetRevision( E::NotesFile ) );
        const VFS::CVfsBlockCache::TEntryHandle oldEntry = env.Cache()->AcquireEntry( "remote-test-place", E::NotesFile, oldRevision, 70000, VFS::CVfsDigest(), false );
        if ( 0 != oldEntry )
        {
            VFS::CVfsBlockCache::SEntryInfo info;
            ASSERT_TRUE( env.Cache()->GetEntryInfo( oldEntry, info ) );
            ASSERT_TRUE( !info.pinned && info.superseded );
            env.Cache()->ReleaseEntry( oldEntry );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "R23: a kept (pinned) path whose share changed and whose new revision is not held yet: with the link down the old complete copy is opened and read, flagged as an older revision, on a thread that waits and on one that does not; a path that is not kept is not served; with the link back the new revision is fetched without being asked, the pin moves to it and the old copy is not served any more" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-r23" );
        E& env = envHolder.Get();
        SRemoteEnvOptions options;
        options.metadataTtlMs = 60000;
        REQUIRE( env.Setup( options ) );

        // one file is kept, one is only cached
        VFS::CVfsFetchProgress progress;
        VFS::CVfsFetchRequest request;
        request.pin = true;
        ASSERT_TRUE( env.Vfs().FetchFile( env.VfsPath( E::MidFile ), request, progress ).IsSuccess() );
        ASSERT_TRUE( CacheWhole( env, E::NotesFile ) );
        CORE::CDynamicBuffer oldContent;
        REQUIRE( env.ReadAll( E::MidFile, oldContent ) && 300000 == oldContent.GetDataSize() && env.Matches( E::MidFile, 0, oldContent, 300000 ) );
        VFS::CVfsCacheState state;
        REQUIRE( env.Vfs().GetFileCacheState( env.VfsPath( E::MidFile ), state ) && state.pinned && state.complete && !state.staleRevision );
        const CORE::CString oldRevision = state.revision;

        // the share changes and the archive learns of it; the first fetch of the new revision (the refetch of the kept file) is cut, so the old copy is the only complete one
        env.Backend().CutNextFetchAfter( 1000 );
        env.Backend().ChangeFile( E::MidFile );
        env.Backend().ChangeFile( E::NotesFile );
        env.Vfs().InvalidateRemoteMetadata();
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( E::MidFile ) ) );
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( E::NotesFile ) ) );
        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::FetchFailedEvent, E::MidFile, 1, 20000 ) );
        REQUIRE( env.WaitUntilIdle( 20000 ) );
        const CORE::CString newRevision = env.Backend().GetRevision( E::MidFile );
        ASSERT_TRUE( newRevision != oldRevision );

        env.Backend().SetLinkUp( false );
        env.Vfs().InvalidateRemoteMetadata();

        // the state of the cache says it: an older, kept copy that is complete
        ASSERT_TRUE( env.Vfs().GetFileCacheState( env.VfsPath( E::MidFile ), state ) );
        ASSERT_TRUE( state.staleRevision && state.pinned && state.complete && state.superseded && state.revision == oldRevision && 300000 == state.fileSize );

        // the open and the read serve it, and the access says it is not the current revision
        for ( UInt32 pass = 0; pass < 2; ++pass )
        {
            VFS::CVfsThreadPolicyScope scope( 0 == pass ? VFS::CVfsThreadPolicy() : Marked() );
            VFS::TBasicVfsResourcePtr resource;
            VFS::CVfsCachedIOAccess* access = Open( env, E::MidFile, resource );
            REQUIRE( GUCEF_NULL != access );
            ASSERT_TRUE( access->IsStaleRevision() && access->GetRevision() == oldRevision && 300000 == access->GetSize() );
            CORE::CDynamicBuffer buffer;
            ASSERT_TRUE( 300000 == ReadAt( access, 0, 300000, buffer ) && SameBytes( buffer, oldContent ) );
            ASSERT_TRUE( access->GetLastResult().IsSuccess() );
            ASSERT_FALSE( env.Matches( E::MidFile, 0, buffer, 300000 ) );       // they are the bytes of the revision before
        }
        ASSERT_TRUE( Statistics( env ).staleServes >= 2 );

        // the listing offers the kept copy too
        VFS::CVFS::TStringVector files;
        env.Vfs().GetFileList( files, env.VfsPath( "data" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX );
        bool listed = false;
        for ( size_t i=0; i<files.size(); ++i )
            listed = listed || "mid.bin" == files[ i ];
        ASSERT_TRUE( listed );

        // the file that is only cached is not served in place of its new revision
        ASSERT_TRUE( env.Vfs().GetFile( env.VfsPath( E::NotesFile ) ).IsNULL() );
        VFS::CVfsCallResult result;
        env.Vfs().GetLastResult( result );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == result.result );
        ASSERT_FALSE( env.Vfs().GetFileCacheState( env.VfsPath( E::NotesFile ), state ) && state.staleRevision );

        // the link is back: the new revision is fetched without anyone asking, the pin moves to it
        env.Backend().SetLinkUp( true );
        REQUIRE( WaitForKept( env, E::MidFile, newRevision, 30000 ) );
        REQUIRE( env.WaitUntilIdle( 20000 ) );
        ASSERT_TRUE( env.Vfs().GetFileCacheState( env.VfsPath( E::MidFile ), state ) );
        ASSERT_TRUE( state.pinned && state.complete && !state.staleRevision && !state.superseded && state.revision == newRevision );
        const VFS::CVfsBlockCache::TEntryHandle oldEntry = env.Cache()->AcquireEntry( "remote-test-place", E::MidFile, oldRevision, 300000, VFS::CVfsDigest(), false );
        if ( 0 != oldEntry )
        {
            VFS::CVfsBlockCache::SEntryInfo info;
            ASSERT_TRUE( env.Cache()->GetEntryInfo( oldEntry, info ) && !info.pinned && info.superseded );
            env.Cache()->ReleaseEntry( oldEntry );
        }
        CORE::CDynamicBuffer content;
        ASSERT_TRUE( env.ReadAll( E::MidFile, content ) && env.Matches( E::MidFile, 0, content, 300000 ) );

        // and the copy of the new revision is the one served offline from now on, not flagged
        env.Backend().SetLinkUp( false );
        env.Vfs().InvalidateRemoteMetadata();
        {
            VFS::TBasicVfsResourcePtr resource;
            VFS::CVfsCachedIOAccess* access = Open( env, E::MidFile, resource );
            REQUIRE( GUCEF_NULL != access );
            ASSERT_FALSE( access->IsStaleRevision() );
            ASSERT_TRUE( access->GetRevision() == newRevision );
            CORE::CDynamicBuffer buffer;
            ASSERT_TRUE( 300000 == ReadAt( access, 0, 300000, buffer ) && env.Matches( E::MidFile, 0, buffer, 300000 ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "R24: a path that is not kept (never pinned, or released) is never served as an older revision, also when an older complete copy of it is in the cache; the same path kept again is" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-r24" );
        E& env = envHolder.Get();
        SRemoteEnvOptions options;
        options.metadataTtlMs = 60000;
        REQUIRE( env.Setup( options ) );

        VFS::CVfsFetchProgress progress;
        VFS::CVfsFetchRequest request;
        request.pin = true;
        ASSERT_TRUE( env.Vfs().FetchFile( env.VfsPath( E::MidFile ), request, progress ).IsSuccess() );
        env.Backend().CutNextFetchAfter( 1000 );
        env.Backend().ChangeFile( E::MidFile );
        env.Vfs().InvalidateRemoteMetadata();
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( E::MidFile ) ) );
        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::FetchFailedEvent, E::MidFile, 1, 20000 ) );
        REQUIRE( env.WaitUntilIdle( 20000 ) );
        env.Backend().SetLinkUp( false );

        // kept: served, flagged
        {
            VFS::TBasicVfsResourcePtr resource;
            VFS::CVfsCachedIOAccess* access = Open( env, E::MidFile, resource );
            REQUIRE( GUCEF_NULL != access );
            ASSERT_TRUE( access->IsStaleRevision() );
        }

        // released: the old copy is a superseded copy that is not pinned, and it is not served
        ASSERT_TRUE( env.Vfs().SetFilePinned( env.VfsPath( E::MidFile ), false ) );
        ASSERT_TRUE( env.Vfs().GetFile( env.VfsPath( E::MidFile ) ).IsNULL() );
        VFS::CVfsCallResult result;
        env.Vfs().GetLastResult( result );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == result.result );
        VFS::CVfsCacheState state;
        ASSERT_FALSE( env.Vfs().GetFileCacheState( env.VfsPath( E::MidFile ), state ) && state.staleRevision );
        {
            VFS::CVfsThreadPolicyScope scope( Marked() );
            ASSERT_TRUE( env.Vfs().GetFile( env.VfsPath( E::MidFile ) ).IsNULL() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "R25: kept paths and the size limit of the cache: a pin that does not fit is refused with NO_SPACE and a message that says why, by a fetch with a pin and by a call, the file stays a cache entry that is not kept, the kept files are never dropped for it, and what is kept is still there after a restart" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-r25" );
        E& env = envHolder.Get();
        SRemoteEnvOptions options;
        options.cacheMaxBytes = 320000;
        options.metadataTtlMs = 60000;
        REQUIRE( env.Setup( options ) );

        VFS::CVfsFetchRequest request;
        request.pin = true;
        {
            VFS::CVfsFetchProgress progress;
            ASSERT_TRUE( env.Vfs().FetchFile( env.VfsPath( E::MidFile ), request, progress ).IsSuccess() );
        }
        VFS::CVfsCacheState state;
        ASSERT_TRUE( env.Vfs().GetFileCacheState( env.VfsPath( E::MidFile ), state ) && state.pinned && state.complete );

        // a fetch with a pin of a file that does not fit what the limit leaves
        {
            VFS::CVfsFetchProgress progress;
            const VFS::CVfsCallResult refused = env.Vfs().FetchFile( env.VfsPath( E::BigFile ), request, progress );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_NO_SPACE == refused.result );
            ASSERT_TRUE( Has( refused.message, "kept offline" ) && Has( refused.message, "size limit" ) );
            ASSERT_FALSE( env.Vfs().GetFileCacheState( env.VfsPath( E::BigFile ), state ) && state.pinned );
        }

        // a call: the file is open and read, so that the cache cannot give its room back, and it does not fit next to the kept one
        {
            VFS::TBasicVfsResourcePtr resource;
            VFS::CVfsCachedIOAccess* access = Open( env, E::NotesFile, resource );
            REQUIRE( GUCEF_NULL != access );
            CORE::CDynamicBuffer buffer;
            ASSERT_TRUE( 70000 == ReadAt( access, 0, 70000, buffer ) );
            ASSERT_FALSE( env.Vfs().SetFilePinned( env.VfsPath( E::NotesFile ), true ) );
            VFS::CVfsCallResult last;
            env.Vfs().GetLastResult( last );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_NO_SPACE == last.result && Has( last.message, "size limit" ) );
        }
        ASSERT_TRUE( Statistics( env ).pinsRefused >= 2 );

        // nothing that was kept is dropped: the pinned file is whole and pinned, whatever passed through the cache
        ASSERT_TRUE( CacheWhole( env, E::TopFile ) );
        ASSERT_TRUE( CacheWhole( env, E::NotesFile ) );
        ASSERT_TRUE( env.Vfs().GetFileCacheState( env.VfsPath( E::MidFile ), state ) && state.pinned && state.complete );
        ASSERT_TRUE( 1 == Statistics( env ).pinnedEntries );

        // letting the kept file go makes room for the other one
        ASSERT_TRUE( env.Vfs().SetFilePinned( env.VfsPath( E::MidFile ), false ) );
        ASSERT_TRUE( CacheWhole( env, E::NotesFile ) );
        ASSERT_TRUE( env.Vfs().SetFilePinned( env.VfsPath( E::NotesFile ), true ) );
        ASSERT_TRUE( env.Vfs().GetFileCacheState( env.VfsPath( E::NotesFile ), state ) && state.pinned && state.complete );

        // after a restart with the link down the kept file is there, and so is the pin
        env.Backend().SetLinkUp( false );
        REQUIRE( env.Restart( true ) );
        ASSERT_TRUE( env.Vfs().GetFileCacheState( env.VfsPath( E::NotesFile ), state ) && state.pinned && state.complete );
        CORE::CDynamicBuffer content;
        ASSERT_TRUE( env.ReadAll( E::NotesFile, content ) && env.Matches( E::NotesFile, 0, content, 70000 ) );
        ASSERT_TRUE( 1 == Statistics( env ).pinnedEntries );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "R26: with the link up, a kept path whose share changed is fetched again by the archive on its own once the change is learned, the pin moves to the new revision and the old copy is the first to be evicted; a file that is not kept is not fetched again" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-r26" );
        E& env = envHolder.Get();
        SRemoteEnvOptions options;
        options.cacheMaxBytes = 400000;
        options.metadataTtlMs = 60000;
        REQUIRE( env.Setup( options ) );

        VFS::CVfsFetchRequest request;
        request.pin = true;
        {
            VFS::CVfsFetchProgress progress;
            ASSERT_TRUE( env.Vfs().FetchFile( env.VfsPath( E::MidFile ), request, progress ).IsSuccess() );
        }
        ASSERT_TRUE( CacheWhole( env, E::NotesFile ) );
        VFS::CVfsCacheState state;
        REQUIRE( env.Vfs().GetFileCacheState( env.VfsPath( E::MidFile ), state ) && state.pinned && state.complete );
        const CORE::CString oldRevision = state.revision;
        ASSERT_TRUE( oldRevision == env.Backend().GetRevision( E::MidFile ) );

        env.Backend().ChangeFile( E::MidFile );
        env.Backend().ChangeFile( E::NotesFile );
        env.Vfs().InvalidateRemoteMetadata();
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( E::MidFile ) ) );
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( E::NotesFile ) ) );
        const CORE::CString newRevision = env.Backend().GetRevision( E::MidFile );
        REQUIRE( WaitForKept( env, E::MidFile, newRevision, 30000 ) );
        REQUIRE( env.WaitUntilIdle( 20000 ) );

        // the pin moved, the old copy went first: the limit holds the new one
        ASSERT_TRUE( env.Vfs().GetFileCacheState( env.VfsPath( E::MidFile ), state ) );
        ASSERT_TRUE( state.pinned && state.complete && !state.staleRevision && state.revision == newRevision );
        ASSERT_TRUE( 0 == env.Cache()->AcquireEntry( "remote-test-place", E::MidFile, oldRevision, 300000, VFS::CVfsDigest(), false ) );
        ASSERT_TRUE( 1 == Statistics( env ).pinnedEntries && Statistics( env ).supersededEvictions >= 1 );
        CORE::CDynamicBuffer content;
        ASSERT_TRUE( env.ReadAll( E::MidFile, content ) && env.Matches( E::MidFile, 0, content, 300000 ) );

        // the file that is only cached was not fetched again by the archive: its fetches are the ones of the kept file
        ASSERT_FALSE( env.Vfs().GetFileCacheState( env.VfsPath( E::NotesFile ), state ) && state.complete && state.revision == env.Backend().GetRevision( E::NotesFile ) );
        ASSERT_TRUE( 0 == env.Events().Count( VFS::CRemoteArchiveEvents::FileArrivedEvent, env.VfsPath( E::NotesFile ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "R27: a refetch of a kept path that failed (the link dropped in the middle) is started again by the next open of the path while the link is up, and the pin moves to the new revision when it is complete" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-r27" );
        E& env = envHolder.Get();
        SRemoteEnvOptions options;
        options.metadataTtlMs = 60000;
        REQUIRE( env.Setup( options ) );

        VFS::CVfsFetchRequest request;
        request.pin = true;
        {
            VFS::CVfsFetchProgress progress;
            ASSERT_TRUE( env.Vfs().FetchFile( env.VfsPath( E::MidFile ), request, progress ).IsSuccess() );
        }
        env.Backend().CutNextFetchAfter( 1000 );
        env.Backend().ChangeFile( E::MidFile );
        env.Vfs().InvalidateRemoteMetadata();
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( E::MidFile ) ) );
        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::FetchFailedEvent, E::MidFile, 1, 20000 ) );
        REQUIRE( env.WaitUntilIdle( 20000 ) );
        VFS::CVfsCacheState state;
        ASSERT_TRUE( env.Vfs().GetFileCacheState( env.VfsPath( E::MidFile ), state ) && state.staleRevision && state.pinned );
        const CORE::CString newRevision = env.Backend().GetRevision( E::MidFile );

        // nothing refetches it by itself now (the link did not change and the metadata is known); the next open of the path does
        env.Backend().ResetCounters();
        REQUIRE( env.WaitUntilIdle( 5000 ) );
        ASSERT_TRUE( 0 == env.Backend().GetFetchCount() );
        {
            VFS::TBasicVfsResourcePtr resource = env.Vfs().GetFile( env.VfsPath( E::MidFile ) );
            ASSERT_FALSE( resource.IsNULL() );
        }
        REQUIRE( WaitForKept( env, E::MidFile, newRevision, 30000 ) );
        REQUIRE( env.WaitUntilIdle( 20000 ) );
        ASSERT_TRUE( env.Backend().GetFetchCount() > 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "R28: a kept path whose refetch failed is fetched again when the link comes back, with no call of the application, also when the metadata of the path is still known" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-r28" );
        E& env = envHolder.Get();
        SRemoteEnvOptions options;
        options.metadataTtlMs = 60000;
        REQUIRE( env.Setup( options ) );

        VFS::CVfsFetchRequest request;
        request.pin = true;
        {
            VFS::CVfsFetchProgress progress;
            ASSERT_TRUE( env.Vfs().FetchFile( env.VfsPath( E::MidFile ), request, progress ).IsSuccess() );
        }
        env.Backend().CutNextFetchAfter( 1000 );
        env.Backend().ChangeFile( E::MidFile );
        env.Vfs().InvalidateRemoteMetadata();
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( E::MidFile ) ) );
        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::FetchFailedEvent, E::MidFile, 1, 20000 ) );
        REQUIRE( env.WaitUntilIdle( 20000 ) );
        const CORE::CString newRevision = env.Backend().GetRevision( E::MidFile );

        // the link goes and comes back; the metadata of the path (the new revision) is still held and fresh, so nothing else asks for the file
        env.Backend().SetLinkUp( false );
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( E::MidFile ) ) );
        env.Backend().SetLinkUp( true );
        REQUIRE( WaitForKept( env, E::MidFile, newRevision, 30000 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "R29: with the refetch of kept paths switched off nothing is fetched on its own and the old kept copy is what is served offline, also when only the index of the cache is left to say what is there; switched on, the same share change is healed" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-r29" );
        E& env = envHolder.Get();
        SRemoteEnvOptions options;
        options.metadataTtlMs = 60000;
        options.refetchPinned = false;
        REQUIRE( env.Setup( options ) );

        VFS::CVfsFetchRequest request;
        request.pin = true;
        {
            VFS::CVfsFetchProgress progress;
            ASSERT_TRUE( env.Vfs().FetchFile( env.VfsPath( E::MidFile ), request, progress ).IsSuccess() );
        }
        const CORE::CString oldRevision = env.Backend().GetRevision( E::MidFile );
        env.Backend().ChangeFile( E::MidFile );
        env.Vfs().InvalidateRemoteMetadata();
        env.Backend().ResetCounters();

        // the change is learned by a listing: the old copy is superseded, and with the switch off no fetch follows
        VFS::CVFS::TStringVector files;
        ASSERT_TRUE( env.Vfs().GetFileList( files, env.VfsPath( "data" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX ) );
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( E::MidFile ) ) );
        REQUIRE( env.WaitUntilIdle( 5000 ) );
        ASSERT_TRUE( 0 == env.Backend().GetFetchCount() );

        // a stat that learns of the change again is the other place where a refetch would start
        env.Vfs().InvalidateRemoteMetadata();
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( E::MidFile ) ) );
        REQUIRE( env.WaitUntilIdle( 5000 ) );
        ASSERT_TRUE( 0 == env.Backend().GetFetchCount() );
        VFS::CVfsCacheState state;
        ASSERT_TRUE( env.Vfs().GetFileCacheState( env.VfsPath( E::MidFile ), state ) && state.staleRevision && state.revision == oldRevision );

        // the link goes and nothing is in memory: the index of the cache alone says what is there, and the kept copy is served
        env.Backend().SetLinkUp( false );
        env.Vfs().InvalidateRemoteMetadata();
        {
            VFS::TBasicVfsResourcePtr resource;
            VFS::CVfsCachedIOAccess* access = Open( env, E::MidFile, resource );
            REQUIRE( GUCEF_NULL != access );
            ASSERT_TRUE( access->IsStaleRevision() && access->GetRevision() == oldRevision );
            CORE::CDynamicBuffer buffer;
            ASSERT_TRUE( 300000 == ReadAt( access, 0, 300000, buffer ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "R30: a change of a kept path that a listing of its directory finds is healed like one that a stat finds: the new revision is fetched without being asked and takes the pin" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-r30" );
        E& env = envHolder.Get();
        SRemoteEnvOptions options;
        options.metadataTtlMs = 60000;
        REQUIRE( env.Setup( options ) );

        VFS::CVfsFetchRequest request;
        request.pin = true;
        {
            VFS::CVfsFetchProgress progress;
            ASSERT_TRUE( env.Vfs().FetchFile( env.VfsPath( E::MidFile ), request, progress ).IsSuccess() );
        }
        env.Backend().ChangeFile( E::MidFile );
        env.Vfs().InvalidateRemoteMetadata();
        VFS::CVFS::TStringVector files;
        ASSERT_TRUE( env.Vfs().GetFileList( files, env.VfsPath( "data" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX ) );
        REQUIRE( WaitForKept( env, E::MidFile, env.Backend().GetRevision( E::MidFile ), 30000 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "R16: a whole file fetch: progress is monotone and ends at the size, the digest is verified, one arrival of the complete file follows; a cancel ends with CANCELLED and a failure event, and a second fetch continues from the blocks held" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-r16" );
        E& env = envHolder.Get();
        SRemoteEnvOptions options;
        options.maxBlocksPerFetch = 2;
        REQUIRE( env.Setup( options ) );
        env.Backend().ProvideDigest( E::BigFile );
        env.Backend().SetDelayMs( 15 );

        {
            VFS::CVfsFetchProgress progress;
            CProgressWatcher watcher( &progress, 0 );
            CTestThreads watching( watcher, 1 );
            ASSERT_TRUE( watching.Start() );
            const VFS::CVfsCallResult result = env.Vfs().FetchFile( env.VfsPath( E::BigFile ), VFS::CVfsFetchRequest(), progress );
            watcher.stop.Store( true );
            ASSERT_TRUE( watching.Join( 10000 ) );
            ASSERT_TRUE( result.IsSuccess() );
            ASSERT_TRUE( 1000000 == progress.bytesDone.Load() && 1000000 == progress.bytesTotal.Load() );
            ASSERT_TRUE( 1 == progress.filesTotal.Load() && 1 == progress.filesDone.Load() && 0 == progress.filesFailed.Load() );
            ASSERT_TRUE( 0 == watcher.decreased.Load() && 0 == watcher.pastTotal.Load() && watcher.samples.Load() >= 2 );
        }
        VFS::CVfsCacheState state;
        ASSERT_TRUE( env.Vfs().GetFileCacheState( env.VfsPath( E::BigFile ), state ) && state.complete && state.digestVerified && !state.pinned );
        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::FileArrivedEvent, E::BigFile, 1, 4000 ) );
        CTestEventRecorder::TRecordVector arrived = env.Events().Find( VFS::CRemoteArchiveEvents::FileArrivedEvent );
        ASSERT_TRUE( 1 == arrived.size() && arrived[ 0 ].fileComplete && 1000000 == arrived[ 0 ].byteCount );
        ASSERT_TRUE( 1 == Statistics( env ).filesVerified );

        // a cancel: the blocks that were stored stay, the fetch ends as cancelled, and one failure event says so
        ASSERT_TRUE( env.Vfs().GetFileCacheState( env.VfsPath( E::MidFile ), state ) == false );
        env.Backend().ProvideDigest( E::MidFile );
        env.Backend().ResetCounters();
        {
            VFS::CVfsFetchProgress progress;
            CProgressWatcher watcher( &progress, 2 * B );
            CTestThreads watching( watcher, 1 );
            ASSERT_TRUE( watching.Start() );
            VFS::CVfsFetchRequest request;
            const VFS::CVfsCallResult result = env.Vfs().FetchFile( env.VfsPath( E::MidFile ), request, progress );
            watcher.stop.Store( true );
            ASSERT_TRUE( watching.Join( 10000 ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_CANCELLED == result.result );
            ASSERT_TRUE( progress.bytesDone.Load() >= 2 * B && progress.bytesDone.Load() < 300000 );
            ASSERT_TRUE( 1 == progress.filesFailed.Load() && CORE::FILESYSTEMACCESS_CANCELLED == progress.GetLastFailure().result );
        }
        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::FetchFailedEvent, E::MidFile, 1, 4000 ) );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_CANCELLED == env.Events().Find( VFS::CRemoteArchiveEvents::FetchFailedEvent, env.VfsPath( E::MidFile ) )[ 0 ].result );
        ASSERT_TRUE( env.Vfs().GetFileCacheState( env.VfsPath( E::MidFile ), state ) && state.cached && !state.complete );
        const UInt64 heldAfterCancel = state.bytesHeld;
        ASSERT_TRUE( heldAfterCancel >= 2 * B );

        // the second fetch fetches what is missing only
        env.Backend().ResetCounters();
        VFS::CVfsFetchProgress again;
        ASSERT_TRUE( env.Vfs().FetchFile( env.VfsPath( E::MidFile ), VFS::CVfsFetchRequest(), again ).IsSuccess() );
        ASSERT_TRUE( env.Backend().GetBytesServed() == 300000 - heldAfterCancel );
        ASSERT_TRUE( again.bytesDone.Load() == 300000 - heldAfterCancel );
        ASSERT_TRUE( env.Vfs().GetFileCacheState( env.VfsPath( E::MidFile ), state ) && state.complete && state.digestVerified );
        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::FileArrivedEvent, E::MidFile, 1, 4000 ) );
        ASSERT_TRUE( 1 == env.Events().Count( VFS::CRemoteArchiveEvents::FileArrivedEvent, env.VfsPath( E::MidFile ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "R17: a digest that does not match empties the copy and ends with RESOURCE_CORRUPT after one new fetch; a wrong hint does the same to a complete copy; a right hint verifies it once and the flag survives a restart" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-r17" );
        E& env = envHolder.Get();
        REQUIRE( env.Setup() );

        // the remote place gives a digest that is not the one of its content: fetched twice, then corrupt
        env.Backend().ProvideWrongDigest( E::MidFile );
        env.Backend().ResetCounters();
        VFS::CVfsFetchProgress progress;
        const VFS::CVfsCallResult result = env.Vfs().FetchFile( env.VfsPath( E::MidFile ), VFS::CVfsFetchRequest(), progress );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_RESOURCE_CORRUPT == result.result );
        ASSERT_TRUE( 600000 == env.Backend().GetBytesServed() );
        VFS::CVfsCacheState state;
        ASSERT_FALSE( env.Vfs().GetFileCacheState( env.VfsPath( E::MidFile ), state ) && state.cached );
        ASSERT_TRUE( 2 == Statistics( env ).digestMismatches );
        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::FetchFailedEvent, E::MidFile, 1, 4000 ) );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_RESOURCE_CORRUPT == env.Events().Find( VFS::CRemoteArchiveEvents::FetchFailedEvent )[ 0 ].result );

        // a wrong hint for a complete copy: emptied
        ASSERT_TRUE( CacheWhole( env, E::NotesFile ) );
        VFS::CVfsDigest wrong( VFS::CVfsDigest::ALGORITHM_SHA256, "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" );
        ASSERT_TRUE( env.Vfs().NoteFileDigest( env.VfsPath( E::NotesFile ), wrong ) );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_RESOURCE_CORRUPT == env.Core()->GetLastResult().result );
        ASSERT_FALSE( env.Vfs().GetFileCacheState( env.VfsPath( E::NotesFile ), state ) && state.cached );
        ASSERT_TRUE( 3 == Statistics( env ).digestMismatches );

        // a right hint verifies a complete copy once
        env.Core()->ForgetFileDigests();
        ASSERT_TRUE( CacheWhole( env, E::NotesFile ) );
        const VFS::CVfsDigest right = env.Backend().ComputeDigest( E::NotesFile, VFS::CVfsDigest::ALGORITHM_SHA256 );
        ASSERT_TRUE( env.Vfs().NoteFileDigest( env.VfsPath( E::NotesFile ), right ) );
        ASSERT_TRUE( env.Vfs().GetFileCacheState( env.VfsPath( E::NotesFile ), state ) && state.complete && state.digestVerified );
        ASSERT_TRUE( state.digest.Matches( right ) );
        ASSERT_TRUE( 1 == Statistics( env ).filesVerified );
        VFS::CVfsDigest answered;
        ASSERT_TRUE( env.Vfs().GetFileDigest( env.VfsPath( E::NotesFile ), answered ) && answered.Matches( right ) );

        // the flag is in the index: a restart does not hash again
        env.Backend().SetLinkUp( false );
        REQUIRE( env.Restart( true ) );
        ASSERT_TRUE( env.Vfs().GetFileCacheState( env.VfsPath( E::NotesFile ), state ) && state.complete && state.digestVerified );
        ASSERT_TRUE( env.Vfs().GetFileDigest( env.VfsPath( E::NotesFile ), answered ) && answered.Matches( right ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "R17b: bytes that the remote place delivers damaged are found when the file becomes complete, by the read that completed it, and the copy is emptied" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-r17b" );
        E& env = envHolder.Get();
        REQUIRE( env.Setup() );
        env.Backend().ProvideDigest( E::NotesFile );
        env.Backend().SetCorruptContent( true );

        VFS::TBasicVfsResourcePtr resource;
        VFS::CVfsCachedIOAccess* access = Open( env, E::NotesFile, resource );
        REQUIRE( GUCEF_NULL != access );
        CORE::CDynamicBuffer buffer;
        const UInt32 got = ReadAt( access, 0, 70000, buffer );
        ASSERT_TRUE( got < 70000 );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_RESOURCE_CORRUPT == access->GetLastResult().result );
        VFS::CVfsCacheState state;
        ASSERT_FALSE( env.Vfs().GetFileCacheState( env.VfsPath( E::NotesFile ), state ) && state.cached );

        // the next read fetches it again, and with the remote place right it is right
        env.Backend().SetCorruptContent( false );
        ASSERT_TRUE( 70000 == ReadAt( access, 0, 70000, buffer ) );
        ASSERT_TRUE( env.Matches( E::NotesFile, 0, buffer, 70000 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "R18: a block of the cache that was damaged on the disk is found by its checksum at the first read after a restart, fetched again, and the bytes are right" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-r18" );
        E& env = envHolder.Get();
        REQUIRE( env.Setup() );
        ASSERT_TRUE( CacheWhole( env, E::MidFile ) );
        REQUIRE( env.Unmount() );
        env.CloseCache();
        const CORE::CString::StringVector dataFiles = BlockCacheTest::ListEntryFiles( env.GetCacheFolder(), ".vbd" );
        REQUIRE( 1 == dataFiles.size() );
        ASSERT_TRUE( BlockCacheTest::DamageByte( dataFiles[ 0 ], 2 * B + 7 ) );

        REQUIRE( env.Restart( true ) );
        env.Backend().ResetCounters();
        CORE::CDynamicBuffer content;
        ASSERT_TRUE( env.ReadAll( E::MidFile, content ) );
        ASSERT_TRUE( env.Matches( E::MidFile, 0, content, 300000 ) );
        ASSERT_TRUE( 1 == env.Backend().GetFetchCount() && B == env.Backend().GetBytesServed() );
        ASSERT_TRUE( 1 == Statistics( env ).corruptBlocks );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "R19: with one running and one waiting job allowed and a slow remote place, the third distinct miss answers CAPLIMIT_RETRY_LATER and raises a failure event, and the other two arrive" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-r19" );
        E& env = envHolder.Get();
        SRemoteEnvOptions options;
        options.maxRunning = 1;
        options.maxQueued = 1;
        REQUIRE( env.Setup( options ) );
        env.Backend().SetDelayMs( 300 );

        VFS::CVfsThreadPolicyScope marked( Marked() );
        VFS::CVfsCallResult result;
        ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( E::ReadmeFile ) ) );
        env.Vfs().GetLastResult( result );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE == result.result );
        ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( E::TopFile ) ) );
        env.Vfs().GetLastResult( result );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE == result.result );
        ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( E::DeepFile ) ) );
        env.Vfs().GetLastResult( result );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CAPLIMIT_RETRY_LATER == result.result && !result.fetchStarted );

        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::FetchFailedEvent, E::DeepFile, 1, 4000 ) );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CAPLIMIT_RETRY_LATER == env.Events().Find( VFS::CRemoteArchiveEvents::FetchFailedEvent, env.VfsPath( E::DeepFile ) )[ 0 ].result );
        env.Backend().SetDelayMs( 0 );
        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::MetadataArrivedEvent, E::ReadmeFile, 1, 4000 ) );
        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::MetadataArrivedEvent, E::TopFile, 1, 4000 ) );
        ASSERT_TRUE( 0 == env.Events().Count( VFS::CRemoteArchiveEvents::MetadataArrivedEvent, env.VfsPath( E::DeepFile ) ) );
        ASSERT_TRUE( 1 == Statistics( env ).fetchJobsRefused );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "R20: the asynchronous calls give what the synchronous calls give: for files, directories and paths that are not there, the metadata and the digest, both listings, the open and the bytes, ranges (inside, at and past the end), the fetch and the prefetch, and the failure of a link that is down" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-r20" );
        E& env = envHolder.Get();
        REQUIRE( env.Setup() );
        VFS::CVFS& vfs = env.Vfs();
        env.Backend().ProvideDigest( E::ReadmeFile );

        const char* const files[] = { E::ReadmeFile, E::NotesFile, E::MidFile, E::EmptyFile, E::TopFile, "docs/missing.txt", "nodir/none.bin", "docs" };
        for ( UInt32 f=0; f<sizeof( files ) / sizeof( files[ 0 ] ); ++f )
        {
            const CORE::CString path = env.VfsPath( files[ f ] );

            // metadata and digest
            CORE::CResourceMetaData syncMeta;
            const bool syncFound = vfs.GetFileMetaData( path, syncMeta );
            const VFS::CVfsCallResult syncMetaFailure = SyncFailure( vfs, CORE::FILESYSTEMACCESS_NOT_FOUND );
            VFS::CVfsDigest syncDigest;
            const bool syncHasDigest = vfs.GetFileDigest( path, syncDigest );
            SAsyncCall meta;
            meta.future = vfs.GetFileMetaDataAsync( path, meta.result );
            REQUIRE( meta.Wait() );
            ASSERT_TRUE( syncFound == meta.result->exists && syncFound == meta.result->result.IsSuccess() );
            if ( syncFound )
            {
                ASSERT_TRUE( syncMeta.resourceSizeInBytes == meta.result->metaData.resourceSizeInBytes && syncMeta.name == meta.result->metaData.name );
                ASSERT_TRUE( syncHasDigest ? ( !meta.result->digest.IsEmpty() && syncDigest.Matches( meta.result->digest ) ) : meta.result->digest.IsEmpty() );
            }
            else
            {
                ASSERT_TRUE( syncMetaFailure.result == meta.result->result.result );
            }

            // the open and what it reads
            VFS::TBasicVfsResourcePtr syncResource = vfs.GetFile( path );
            const VFS::CVfsCallResult syncOpenFailure = SyncFailure( vfs, CORE::FILESYSTEMACCESS_NOT_FOUND );
            SAsyncCall open;
            open.future = vfs.GetFileAsync( path, open.result );
            REQUIRE( open.Wait() );
            ASSERT_TRUE( syncResource.IsNULL() == open.result->resource.IsNULL() && syncResource.IsNULL() != open.result->result.IsSuccess() );
            if ( !syncResource.IsNULL() )
            {
                CORE::CDynamicBuffer syncBytes( 2000, true ), asyncBytes( 2000, true );
                const UInt32 syncGot = syncResource->GetAccess()->Read( syncBytes.GetBufferPtr(), 1, 2000 );
                const UInt32 asyncGot = open.result->resource->GetAccess()->Read( asyncBytes.GetBufferPtr(), 1, 2000 );
                syncBytes.SetDataSize( syncGot );
                asyncBytes.SetDataSize( asyncGot );
                ASSERT_TRUE( syncGot == asyncGot && SameBytes( syncBytes, asyncBytes ) );
            }
            else
            {
                ASSERT_TRUE( syncOpenFailure.result == open.result->result.result );
            }

            // ranges
            const UInt64 size = syncFound ? syncMeta.resourceSizeInBytes : 0;
            const UInt64 offsets[ 5 ] = { 0, 500, size > 10 ? size - 10 : 0, size, size + 5 };
            const UInt32 lengths[ 5 ] = { 100, 1000, 100, 10, 10 };
            for ( UInt32 r=0; r<5; ++r )
            {
                CORE::CDynamicBuffer syncData;
                const bool syncOk = SyncReadRange( vfs, path, offsets[ r ], lengths[ r ], syncData );
                SAsyncCall range;
                range.future = vfs.ReadFileRangeAsync( path, offsets[ r ], lengths[ r ], range.result );
                REQUIRE( range.Wait() );
                ASSERT_TRUE( syncOk == range.result->result.IsSuccess() );
                ASSERT_TRUE( SameBytes( syncData, range.result->data ) && range.result->bytesDone == range.result->data.GetDataSize() );
            }
        }

        // listings, both kinds, of directories that are there and that are not
        const char* const dirs[] = { "", "docs", "docs/notes", "data", "nodir" };
        for ( UInt32 d=0; d<sizeof( dirs ) / sizeof( dirs[ 0 ] ); ++d )
        {
            for ( UInt32 variant=0; variant<4; ++variant )
            {
                const bool recursive = 0 != ( variant & 1 );
                const bool includePath = 0 != ( variant & 2 );
                const CORE::CString location = env.VfsPath( dirs[ d ] );
                VFS::CVFS::TStringVector syncFiles, syncDirs;
                const bool syncFilesOk = vfs.GetFileList( syncFiles, location, recursive, includePath, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX );
                const bool syncDirsOk = vfs.GetDirList( syncDirs, location, recursive, includePath, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX );
                SAsyncCall fileList, dirList;
                fileList.future = vfs.GetFileListAsync( location, recursive, includePath, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX, fileList.result );
                dirList.future = vfs.GetDirListAsync( location, recursive, includePath, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX, dirList.result );
                REQUIRE( fileList.Wait() && dirList.Wait() );
                ASSERT_TRUE( syncFilesOk == fileList.result->result.IsSuccess() && syncFiles == fileList.result->names );
                ASSERT_TRUE( syncDirsOk == dirList.result->result.IsSuccess() && syncDirs == dirList.result->names );
            }
        }

        // the fetch: the asynchronous one first, then the synchronous one finds nothing left to fetch; the answers and the counters are those of the call
        {
            VFS::CVfsFetchProgressPtr asyncProgress;
            VFS::CVfsFetchRequest request;
            SAsyncCall fetch;
            fetch.future = vfs.FetchFileAsync( env.VfsPath( E::BigFile ), request, asyncProgress, fetch.result );
            REQUIRE( fetch.Wait() );
            VFS::CVfsFetchProgress syncProgress;
            const VFS::CVfsCallResult syncResult = vfs.FetchFile( env.VfsPath( E::BigFile ), request, syncProgress );
            ASSERT_TRUE( fetch.result->result.IsSuccess() && syncResult.IsSuccess() );
            ASSERT_TRUE( 1000000 == asyncProgress->bytesDone.Load() && 0 == syncProgress.bytesDone.Load() && 1000000 == syncProgress.bytesTotal.Load() );
            ASSERT_TRUE( 1 == asyncProgress->filesTotal.Load() && 1 == syncProgress.filesTotal.Load() && 1 == asyncProgress->filesDone.Load() && 1 == syncProgress.filesDone.Load() );

            VFS::CVfsFetchProgressPtr missingAsyncProgress;
            SAsyncCall missing;
            missing.future = vfs.FetchFileAsync( env.VfsPath( "data/nope.bin" ), request, missingAsyncProgress, missing.result );
            REQUIRE( missing.Wait() );
            VFS::CVfsFetchProgress missingSyncProgress;
            const VFS::CVfsCallResult missingSync = vfs.FetchFile( env.VfsPath( "data/nope.bin" ), request, missingSyncProgress );
            ASSERT_TRUE( missing.result->result.result == missingSync.result && CORE::FILESYSTEMACCESS_NOT_FOUND == missingSync.result );
            ASSERT_TRUE( missingAsyncProgress->filesTotal.Load() == missingSyncProgress.filesTotal.Load() && missingAsyncProgress->filesFailed.Load() == missingSyncProgress.filesFailed.Load() && 1 == missingSyncProgress.filesFailed.Load() );
        }

        // the prefetch: what it fetches is what a fetch of each file would have left in the cache
        {
            VFS::CVfsFetchRequest request;
            request.files.push_back( env.VfsPath( E::NotesFile ) );
            request.files.push_back( env.VfsPath( E::ReadmeFile ) );
            request.directories.push_back( env.VfsPath( "data" ) );
            VFS::CVfsFetchProgressPtr progress;
            SAsyncCall prefetch;
            prefetch.future = vfs.PrefetchAsync( request, progress, prefetch.result );
            REQUIRE( prefetch.Wait() );
            ASSERT_TRUE( prefetch.result->result.IsSuccess() && 3 == progress->filesDone.Load() );
            VFS::CVfsCacheState state;
            ASSERT_TRUE( vfs.GetFileCacheState( env.VfsPath( E::NotesFile ), state ) && state.complete && 70000 == state.fileSize );
            ASSERT_TRUE( vfs.GetFileCacheState( env.VfsPath( E::ReadmeFile ), state ) && state.complete && 1000 == state.fileSize );
            for ( UInt32 i=0; i<request.files.size(); ++i )
            {
                VFS::CVfsFetchProgress syncProgress;
                ASSERT_TRUE( vfs.FetchFile( request.files[ i ], VFS::CVfsFetchRequest(), syncProgress ).IsSuccess() && 0 == syncProgress.bytesDone.Load() );
            }
        }

        // the link is down: the same failure from both, for a file that was never cached
        env.Backend().SetLinkUp( false );
        env.Vfs().InvalidateRemoteMetadata();
        {
            const CORE::CString path = env.VfsPath( E::DeepFile );
            CORE::CResourceMetaData syncMeta;
            ASSERT_FALSE( vfs.GetFileMetaData( path, syncMeta ) );
            const VFS::CVfsCallResult syncMetaFailure = SyncFailure( vfs, CORE::FILESYSTEMACCESS_NOT_FOUND );
            SAsyncCall meta;
            meta.future = vfs.GetFileMetaDataAsync( path, meta.result );
            REQUIRE( meta.Wait() );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == syncMetaFailure.result && syncMetaFailure.result == meta.result->result.result );

            ASSERT_TRUE( vfs.GetFile( path ).IsNULL() );
            const VFS::CVfsCallResult syncOpenFailure = SyncFailure( vfs, CORE::FILESYSTEMACCESS_NOT_FOUND );
            SAsyncCall open;
            open.future = vfs.GetFileAsync( path, open.result );
            REQUIRE( open.Wait() );
            ASSERT_TRUE( syncOpenFailure.result == open.result->result.result && CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == open.result->result.result );

            VFS::CVfsFetchProgress syncProgress;
            const VFS::CVfsCallResult syncFetch = vfs.FetchFile( path, VFS::CVfsFetchRequest(), syncProgress );
            VFS::CVfsFetchProgressPtr asyncProgress;
            SAsyncCall fetch;
            fetch.future = vfs.FetchFileAsync( path, VFS::CVfsFetchRequest(), asyncProgress, fetch.result );
            REQUIRE( fetch.Wait() );
            ASSERT_TRUE( syncFetch.result == fetch.result->result.result && CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == syncFetch.result );
            ASSERT_TRUE( 1 == syncProgress.filesFailed.Load() && 1 == asyncProgress->filesFailed.Load() );

            // the listings of a directory of which the cache holds a file: the one that is made from the cache, flagged, from both
            ASSERT_TRUE( vfs.GetFileMetaData( env.VfsPath( E::ReadmeFile ), syncMeta ) );
            VFS::CVFS::TStringVector syncOfflineFiles, syncOfflineDirs;
            const bool syncOfflineFilesOk = vfs.GetFileList( syncOfflineFiles, env.VfsPath( "docs" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX );
            const VFS::CVfsCallResult syncListFailure = SyncFailure( vfs, CORE::FILESYSTEMACCESS_FAILED );
            const bool syncOfflineDirsOk = vfs.GetDirList( syncOfflineDirs, env.VfsPath( "docs" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX );
            SAsyncCall offlineFiles, offlineDirs;
            offlineFiles.future = vfs.GetFileListAsync( env.VfsPath( "docs" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX, offlineFiles.result );
            offlineDirs.future = vfs.GetDirListAsync( env.VfsPath( "docs" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX, offlineDirs.result );
            REQUIRE( offlineFiles.Wait() && offlineDirs.Wait() );
            ASSERT_TRUE( syncOfflineFilesOk == offlineFiles.result->result.IsSuccess() && syncOfflineFiles == offlineFiles.result->names );
            ASSERT_TRUE( syncOfflineDirsOk == offlineDirs.result->result.IsSuccess() && syncOfflineDirs == offlineDirs.result->names );
            ASSERT_TRUE( !syncOfflineFilesOk && CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == syncListFailure.result && syncListFailure.result == offlineFiles.result->result.result );
            ASSERT_TRUE( 1 == syncOfflineFiles.size() && "readme.txt" == syncOfflineFiles[ 0 ] );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "R21: InvalidateMetadata through the VFS makes the next stat go to the remote place: for one path, for the directory of a path, and for every mount" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-r21" );
        E& env = envHolder.Get();
        REQUIRE( env.Setup() );
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( E::MidFile ) ) );
        ASSERT_TRUE( 1 == env.Backend().GetStatCount() );
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( E::MidFile ) ) );
        ASSERT_TRUE( 1 == env.Backend().GetStatCount() );               // within the time to live: no call
        ASSERT_TRUE( VFS::CIRemoteArchive::METADATA_FRESH == env.Core()->GetMetadataState( E::MidFile ) );

        env.Vfs().InvalidateRemoteMetadata( env.VfsPath( E::MidFile ) );
        ASSERT_TRUE( VFS::CIRemoteArchive::METADATA_UNKNOWN == env.Core()->GetMetadataState( E::MidFile ) );
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( E::MidFile ) ) );
        ASSERT_TRUE( 2 == env.Backend().GetStatCount() );

        // the path form of a directory: its listing and the listing of its parent go, a stat of a file in it is asked again
        VFS::CVFS::TStringVector list;
        ASSERT_TRUE( env.Vfs().GetFileList( list, env.VfsPath( "data" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX ) );
        ASSERT_TRUE( 1 == env.Backend().GetListCount() );
        ASSERT_TRUE( env.Vfs().GetFileList( list, env.VfsPath( "data" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX ) );
        ASSERT_TRUE( 1 == env.Backend().GetListCount() );
        env.Vfs().InvalidateRemoteMetadata( env.VfsPath( "data/" ) );
        ASSERT_TRUE( env.Vfs().GetFileList( list, env.VfsPath( "data" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX ) );
        ASSERT_TRUE( 2 == env.Backend().GetListCount() );

        // the path of a file in it: the listing of its directory goes too, which is what answers the stat of the files beside it
        env.Vfs().InvalidateRemoteMetadata( env.VfsPath( E::MidFile ) );
        ASSERT_TRUE( env.Vfs().GetFileList( list, env.VfsPath( "data" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX ) );
        ASSERT_TRUE( 3 == env.Backend().GetListCount() );

        // every mount
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( E::MidFile ) ) );
        const UInt32 stats = env.Backend().GetStatCount();
        env.Vfs().InvalidateRemoteMetadata();
        ASSERT_TRUE( VFS::CIRemoteArchive::METADATA_UNKNOWN == env.Core()->GetMetadataState( E::MidFile ) );
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( E::MidFile ) ) );
        ASSERT_TRUE( stats + 1 == env.Backend().GetStatCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "R22: the statistics move as the cases above say: fetches, calls to the remote place, misses of threads that must not wait, jobs by outcome, events, offline answers and the cache" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-r22" );
        E& env = envHolder.Get();
        SRemoteEnvOptions options;
        options.maxRunning = 1;
        options.cacheMaxBytes = 4 * 1024 * 1024;
        REQUIRE( env.Setup( options ) );
        VFS::CVfsRemoteStatistics s = Statistics( env );
        ASSERT_TRUE( 0 == s.bytesFetched && 0 == s.fetchCalls && 0 == s.statCalls && 0 == s.listCalls && 0 == s.openCalls && 0 == s.policyMisses && 0 == s.eventsRaised );
        ASSERT_TRUE( 4 * 1024 * 1024 == s.maxBytes && 0 == s.entries );

        // a stat, a listing, a read of one block on a thread that waits
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( E::MidFile ) ) );
        VFS::CVFS::TStringVector list;
        ASSERT_TRUE( env.Vfs().GetFileList( list, env.VfsPath( "data" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX ) );
        {
            VFS::TBasicVfsResourcePtr resource;
            VFS::CVfsCachedIOAccess* access = Open( env, E::MidFile, resource );
            REQUIRE( GUCEF_NULL != access );
            CORE::CDynamicBuffer buffer;
            ASSERT_TRUE( 1000 == ReadAt( access, 0, 1000, buffer ) );
            ASSERT_TRUE( 1000 == ReadAt( access, 0, 1000, buffer ) );
        }
        s = Statistics( env );
        ASSERT_TRUE( 1 == s.statCalls && 1 == s.listCalls && 1 == s.openCalls && 1 == s.fetchCalls && B == s.bytesFetched );
        ASSERT_TRUE( 1 == s.entries && 1 == s.blocksStored && B == s.usedBytes );
        ASSERT_TRUE( 0 == s.policyMisses && 0 == s.fetchJobsAccepted );

        // misses of a thread that must not wait: two jobs, one of them merged; one that fails; one that is cancelled
        {
            VFS::CVfsThreadPolicyScope marked( Marked() );
            env.Backend().SetDelayMs( 100 );
            ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( E::ReadmeFile ) ) );          // runs, waits for the remote place
            ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( E::ReadmeFile ) ) );          // merged into it
            ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( E::TopFile ) ) );             // waits behind it
            ASSERT_TRUE( env.Scheduler().CancelJob( VFS::CVfsRemoteFetchJob::MakeKey( VFS::FETCHJOB_STAT, env.Core(), E::TopFile, 0 ) ) );
            env.Backend().SetDelayMs( 0 );
            REQUIRE( env.WaitUntilIdle( 4000 ) );
            env.Backend().FailNext( CTestRemotePlace::OP_STAT, 1, CORE::FILESYSTEMACCESS_FAILED );
            ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( E::DeepFile ) ) );
            REQUIRE( env.WaitUntilIdle( 4000 ) );
        }
        s = Statistics( env );
        ASSERT_TRUE( 4 == s.policyMisses );
        ASSERT_TRUE( 3 == s.fetchJobsAccepted && 1 == s.fetchJobsMerged && 0 == s.fetchJobsRefused );
        ASSERT_TRUE( 1 == s.fetchJobsCancelled && 1 == s.fetchJobsFailed );
        ASSERT_TRUE( 1 == env.Events().Count( VFS::CRemoteArchiveEvents::MetadataArrivedEvent ) );
        ASSERT_TRUE( 2 == env.Events().Count( VFS::CRemoteArchiveEvents::FetchFailedEvent ) );
        ASSERT_TRUE( 3 == s.eventsRaised );

        // offline answers and misses
        env.Backend().SetLinkUp( false );
        env.Vfs().InvalidateRemoteMetadata();
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( E::MidFile ) ) );
        ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( "nothere/file" ) ) );
        s = Statistics( env );
        ASSERT_TRUE( s.offlineReads >= 1 && s.offlineMisses >= 1 );
        ASSERT_TRUE( 4 == s.eventsRaised );                 // the state change
        ASSERT_TRUE( ( env.HasHandleStatistics() || 0 == s.handlesOpen ) && 0 == s.busyRetries && 0 == s.digestMismatches );

        // the fields of the cache are those of the cache, field by field
        VFS::CVfsBlockCache::SStatistics cache;
        env.Cache()->GetStatistics( cache );
        ASSERT_TRUE( cache.maxBytes == s.maxBytes && cache.usedBytes == s.usedBytes && cache.pinnedBytes == s.pinnedBytes && cache.entries == s.entries && cache.pinnedEntries == s.pinnedEntries );
        ASSERT_TRUE( cache.blockHits == s.blockHits && cache.blockMisses == s.blockMisses && cache.blocksStored == s.blocksStored && cache.corruptBlocks == s.corruptBlocks );
        ASSERT_TRUE( cache.evictedEntries == s.evictedEntries && cache.evictedBytes == s.evictedBytes && cache.supersededEvictions == s.supersededEvictions && cache.storeFailures == s.storeFailures );
        ASSERT_TRUE( cache.openFiles == s.openFiles && cache.fileReopens == s.fileReopens );
        ASSERT_TRUE( env.HasHandleStatistics() || ( 0 == s.handlesPeak && 0 == s.handleEvictions && 0 == s.handleReopens ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Cached access: position, size, seek, end of file, a copy with a position of its own, writes that are refused, an empty file, and a closed access" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-access" );
        E& env = envHolder.Get();
        REQUIRE( env.Setup() );
        VFS::TBasicVfsResourcePtr resource;
        VFS::CVfsCachedIOAccess* access = Open( env, E::ReadmeFile, resource );
        REQUIRE( GUCEF_NULL != access );
        ASSERT_TRUE( access->Opened() && access->IsValid() && access->IsReadable() && !access->IsWriteable() );
        ASSERT_TRUE( 1000 == access->GetSize() && 0 == access->Tell() && !access->Eof() );

        CORE::CDynamicBuffer buffer;
        ASSERT_TRUE( 10 == ReadAt( access, 0, 10, buffer ) );
        ASSERT_TRUE( 10 == access->Tell() );
        ASSERT_TRUE( 0 == access->Seek( 100, SEEK_SET ) && 100 == access->Tell() );
        ASSERT_TRUE( 0 == access->Seek( 50, SEEK_CUR ) && 150 == access->Tell() );
        ASSERT_TRUE( 0 == access->Seek( -1, SEEK_END ) && 999 == access->Tell() );
        ASSERT_TRUE( 0 != access->Seek( -1, SEEK_SET ) && 0 != access->Seek( 0, 99 ) );
        const char last = access->GetChar();
        UInt8 expected = 0;
        ASSERT_TRUE( env.Backend().FillExpected( E::ReadmeFile, 999, &expected, 1 ) );
        ASSERT_TRUE( (UInt8) last == expected );
        ASSERT_TRUE( access->Eof() && 1000 == access->Tell() );
        ASSERT_TRUE( 0 == access->Read( buffer.GetBufferPtr(), 1, 10 ) );
        ASSERT_TRUE( access->GetLastResult().IsSuccess() );

        // writes do nothing
        const char data[ 4 ] = { 1, 2, 3, 4 };
        ASSERT_TRUE( 0 == access->Write( data, 1, 4 ) );

        // a copy has its own position and reads the same bytes
        access->Setpos( 500 );
        VFS::CVfsCachedIOAccess* copy = static_cast< VFS::CVfsCachedIOAccess* >( access->Clone() );
        REQUIRE( GUCEF_NULL != copy );
        ASSERT_TRUE( 500 == copy->Tell() && copy->GetRevision() == access->GetRevision() );
        copy->Setpos( 0 );
        CORE::CDynamicBuffer fromCopy;
        ASSERT_TRUE( 100 == ReadAt( copy, 0, 100, fromCopy ) && env.Matches( E::ReadmeFile, 0, fromCopy, 100 ) );
        ASSERT_TRUE( 500 == access->Tell() );
        GUCEF_DELETE copy;

        // a read of two elements of two bytes
        access->Setpos( 0 );
        buffer.SetDataSize( 100 );
        ASSERT_TRUE( 10 == access->Read( buffer.GetBufferPtr(), 2, 10 ) && 20 == access->Tell() );

        // closed: nothing is read, and it is not valid
        access->Close();
        ASSERT_FALSE( access->Opened() || access->IsValid() );
        ASSERT_TRUE( 0 == access->Read( buffer.GetBufferPtr(), 1, 10 ) );

        // an empty file has no blocks and reads nothing
        VFS::TBasicVfsResourcePtr emptyResource;
        VFS::CVfsCachedIOAccess* empty = Open( env, E::EmptyFile, emptyResource );
        REQUIRE( GUCEF_NULL != empty );
        ASSERT_TRUE( 0 == empty->GetSize() && empty->Eof() && 0 == empty->Read( buffer.GetBufferPtr(), 1, 10 ) );

        // a directory is no file, and a file that is not there gives nothing
        ASSERT_TRUE( env.Vfs().GetFile( env.VfsPath( "docs" ) ).IsNULL() );
        ASSERT_TRUE( env.Vfs().GetFile( env.VfsPath( "docs/none.txt" ) ).IsNULL() );
        ASSERT_TRUE( env.Vfs().GetFile( env.VfsPath( E::ReadmeFile ), "wb" ).IsNULL() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Read ahead: a reader that goes through a file in order gets the blocks ahead of it in the runs it waits for, and every byte is right" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-readahead" );
        E& env = envHolder.Get();
        REQUIRE( env.Setup() );
        VFS::TBasicVfsResourcePtr resource;
        VFS::CVfsCachedIOAccess* access = Open( env, E::BigFile, resource );
        REQUIRE( GUCEF_NULL != access );
        UInt64 position = 0;
        CORE::CDynamicBuffer buffer;
        while ( position < 1000000 )
        {
            const UInt32 got = ReadAt( access, position, 40000, buffer );
            ASSERT_TRUE( got > 0 );
            ASSERT_TRUE( env.Matches( E::BigFile, position, buffer, got ) );
            position += got;
        }
        ASSERT_TRUE( 1000000 == position );
        const VFS::CVfsRemoteStatistics s = Statistics( env );
        ASSERT_TRUE( s.readAheadBlocks > 0 );
        ASSERT_TRUE( env.Backend().GetFetchCount() < 16 );
        ASSERT_TRUE( 1000000 == env.Backend().GetBytesServed() );          // every byte once
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Concurrency: several threads reading the same files at once get the right bytes" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-readers" );
        E& env = envHolder.Get();
        REQUIRE( env.Setup() );
        CReaderThreads body( &env );
        CTestThreads threads( body, 4 );
        ASSERT_TRUE( threads.Start() );
        ASSERT_TRUE( threads.Join( 120000 ) );
        ASSERT_TRUE( 4 == body.done.Load() );
        ASSERT_TRUE( 0 == body.wrong.Load() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Concurrency: callers of both kinds from several threads at once (waiting and not waiting, listings, stats and opens) never get a wrong byte or a deadlock" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-mixed" );
        E& env = envHolder.Get();
        REQUIRE( env.Setup() );
        env.Backend().SetDelayMs( 1 );
        CMixedCallers body( &env );
        CTestThreads threads( body, 6 );
        ASSERT_TRUE( threads.Start() );
        for ( UInt32 pumped=0; pumped<200; ++pumped )
        {
            CRemoteEnv::Pump();
            MT::ThreadDelay( 2 );
        }
        ASSERT_TRUE( threads.Join( 120000 ) );
        ASSERT_TRUE( 0 == body.wrong.Load() );
        REQUIRE( env.WaitUntilIdle( 4000 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Shutdown: unloading the archive while fetches are in flight and readers are reading: nothing hangs, no event is raised afterwards, and a file that is still open fails or ends but never crashes" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-shutdown" );
        E& env = envHolder.Get();
        REQUIRE( env.Setup() );
        env.Backend().SetDelayMs( 20 );

        VFS::TBasicVfsResourcePtr resource;
        VFS::CVfsCachedIOAccess* access = Open( env, E::BigFile, resource );
        REQUIRE( GUCEF_NULL != access );
        {
            VFS::CVfsThreadPolicyScope marked( Marked() );
            ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( E::ReadmeFile ) ) );
            ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( E::TopFile ) ) );
        }

        CReaderThreads body( &env );
        CTestThreads threads( body, 3 );
        ASSERT_TRUE( threads.Start() );
        MT::ThreadDelay( 30 );
        REQUIRE( env.Unmount() );
        ASSERT_TRUE( threads.Join( 120000 ) );

        const UInt32 total = env.Events().GetTotal();
        for ( UInt32 i=0; i<100; ++i )
        {
            CRemoteEnv::Pump();
            MT::ThreadDelay( 1 );
        }
        ASSERT_TRUE( total == env.Events().GetTotal() );

        // the file that is still open: a read is answered or refused, never a crash
        CORE::CDynamicBuffer buffer;
        ReadAt( access, 0, 1000, buffer );
        ReadAt( access, 900000, 1000, buffer );
        ASSERT_FALSE( access->IsValid() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Digest hints: GetFileDigest answers the digest of the remote place, else the hint of the application, else the one the cache verified, and nothing is computed for it" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-hints" );
        E& env = envHolder.Get();
        REQUIRE( env.Setup() );
        VFS::CVfsDigest digest;
        ASSERT_FALSE( env.Vfs().GetFileDigest( env.VfsPath( E::TopFile ), digest ) );

        // a hint
        const VFS::CVfsDigest right = env.Backend().ComputeDigest( E::TopFile, VFS::CVfsDigest::ALGORITHM_SHA256 );
        ASSERT_TRUE( env.Vfs().NoteFileDigest( env.VfsPath( E::TopFile ), right ) );
        ASSERT_TRUE( env.Vfs().GetFileDigest( env.VfsPath( E::TopFile ), digest ) && digest.Matches( right ) );
        env.Core()->ForgetFileDigests();
        ASSERT_FALSE( env.Vfs().GetFileDigest( env.VfsPath( E::TopFile ), digest ) );

        // a hint for another revision is not the digest of this one
        ASSERT_TRUE( env.Vfs().NoteFileDigest( env.VfsPath( E::TopFile ), right, "100-77" ) );
        ASSERT_FALSE( env.Vfs().GetFileDigest( env.VfsPath( E::TopFile ), digest ) );
        env.Core()->ForgetFileDigests();

        // the digest of the remote place wins over a hint that differs
        env.Backend().ProvideDigest( E::TopFile );
        VFS::CVfsDigest wrong( VFS::CVfsDigest::ALGORITHM_SHA256, "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" );
        env.Vfs().InvalidateRemoteMetadata();
        ASSERT_TRUE( env.Vfs().NoteFileDigest( env.VfsPath( E::TopFile ), wrong ) );
        ASSERT_TRUE( env.Vfs().GetFileDigest( env.VfsPath( E::TopFile ), digest ) && digest.Matches( right ) );

        // the digest that was verified is answered when the place gives none
        env.Core()->ForgetFileDigests();
        ASSERT_TRUE( CacheWhole( env, E::ReadmeFile ) );
        const VFS::CVfsDigest readmeDigest = env.Backend().ComputeDigest( E::ReadmeFile, VFS::CVfsDigest::ALGORITHM_SHA256 );
        ASSERT_TRUE( env.Vfs().NoteFileDigest( env.VfsPath( E::ReadmeFile ), readmeDigest ) );
        env.Core()->ForgetFileDigests();
        ASSERT_TRUE( env.Vfs().GetFileDigest( env.VfsPath( E::ReadmeFile ), digest ) && digest.Matches( readmeDigest ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mount root: the root of a remote mount exists and is no file without a call to the remote place, whatever the rule of the thread, the link and what is known; a folder below it is asked as before" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-mountroot" );
        E& env = envHolder.Get();
        REQUIRE( env.Setup() );
        env.Backend().ResetCounters();
        const CORE::CString root = env.VfsPath( CORE::CString::Empty );
        VFS::CVfsCallResult result;

        // a thread that never waits, and nothing is known yet
        {
            VFS::CVfsThreadPolicyScope marked( ( VFS::CVfsThreadPolicy( VFS::MISSACTION_FAIL ) ) );
            ASSERT_TRUE( env.Vfs().DirExists( root ) );
            env.Vfs().GetLastResult( result );
            ASSERT_TRUE( result.IsSuccess() );
            ASSERT_FALSE( env.Vfs().FileExists( root ) );
            env.Vfs().GetLastResult( result );
            ASSERT_TRUE( result.IsSuccess() );
        }
        ASSERT_TRUE( 0 == env.Backend().GetStatCount() && 0 == env.Backend().GetListCount() );

        // a thread that waits
        ASSERT_TRUE( env.Vfs().DirExists( root ) );
        ASSERT_FALSE( env.Vfs().FileExists( root ) );
        ASSERT_TRUE( 0 == env.Backend().GetStatCount() && 0 == env.Backend().GetListCount() );

        // the link is down and nothing is known: the root exists, a folder below it is not known
        env.Backend().SetLinkUp( false );
        env.Vfs().InvalidateRemoteMetadata();
        ASSERT_TRUE( env.Vfs().DirExists( root ) );
        env.Vfs().GetLastResult( result );
        ASSERT_TRUE( result.IsSuccess() );
        ASSERT_FALSE( env.Vfs().FileExists( root ) );
        ASSERT_FALSE( env.Vfs().DirExists( env.VfsPath( "docs" ) ) );
        env.Vfs().GetLastResult( result );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == result.result );
        ASSERT_TRUE( 0 == env.Backend().GetStatCount() && 0 == env.Backend().GetListCount() );

        // the archive itself answers the same for its root, whatever the name of it
        VFS::CVFS::TArchivePtr archive = env.Vfs().GetMountedArchive( E::ArchiveName );
        ASSERT_TRUE( !archive.IsNULL() );
        if ( !archive.IsNULL() )
        {
            ASSERT_TRUE( archive->DirExists( CORE::CString::Empty ) && archive->DirExists( "/" ) );
            ASSERT_FALSE( archive->FileExists( CORE::CString::Empty ) );
            ASSERT_FALSE( archive->FileExists( "/" ) );
            ASSERT_TRUE( 0 == env.Backend().GetStatCount() && 0 == env.Backend().GetListCount() );
        }

        // a folder below the root is asked of the remote place
        env.Backend().SetLinkUp( true );
        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::ConnectionStateChangedEvent, CORE::CString::Empty, 2, 4000 ) );
        env.Vfs().InvalidateRemoteMetadata();
        env.Backend().ResetCounters();
        ASSERT_TRUE( env.Vfs().DirExists( env.VfsPath( "docs" ) ) );
        ASSERT_TRUE( env.Backend().GetStatCount() + env.Backend().GetListCount() > 0 );
        ASSERT_FALSE( env.Vfs().DirExists( env.VfsPath( "absent_folder" ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Digest hints: CVFS::ForgetFileDigests() of a path takes the hints of the remote mounts that cover it away (the root of the mount too), and says no for a path that no remote mount covers" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-forgethints" );
        E& env = envHolder.Get();
        REQUIRE( env.Setup() );
        VFS::CVfsDigest digest;
        const VFS::CVfsDigest right = env.Backend().ComputeDigest( E::TopFile, VFS::CVfsDigest::ALGORITHM_SHA256 );
        const VFS::CVfsDigest readme = env.Backend().ComputeDigest( E::ReadmeFile, VFS::CVfsDigest::ALGORITHM_SHA256 );

        // the file below the mount
        ASSERT_TRUE( env.Vfs().NoteFileDigest( env.VfsPath( E::TopFile ), right ) );
        ASSERT_TRUE( env.Vfs().GetFileDigest( env.VfsPath( E::TopFile ), digest ) && digest.Matches( right ) );
        ASSERT_TRUE( env.Vfs().ForgetFileDigests( env.VfsPath( E::TopFile ) ) );
        ASSERT_FALSE( env.Vfs().GetFileDigest( env.VfsPath( E::TopFile ), digest ) );

        // the root of the mount covers all of it
        ASSERT_TRUE( env.Vfs().NoteFileDigest( env.VfsPath( E::TopFile ), right ) );
        ASSERT_TRUE( env.Vfs().NoteFileDigest( env.VfsPath( E::ReadmeFile ), readme ) );
        ASSERT_TRUE( env.Vfs().ForgetFileDigests( env.VfsPath( CORE::CString::Empty ) ) );
        ASSERT_FALSE( env.Vfs().GetFileDigest( env.VfsPath( E::TopFile ), digest ) );
        ASSERT_FALSE( env.Vfs().GetFileDigest( env.VfsPath( E::ReadmeFile ), digest ) );

        // hints that are noted after it are kept as usual
        ASSERT_TRUE( env.Vfs().NoteFileDigest( env.VfsPath( E::TopFile ), right ) );
        ASSERT_TRUE( env.Vfs().GetFileDigest( env.VfsPath( E::TopFile ), digest ) && digest.Matches( right ) );

        // a path that no remote mount covers
        ASSERT_FALSE( env.Vfs().ForgetFileDigests( "/not_a_mount_of_this_case/file.bin" ) );
        ASSERT_TRUE( env.Vfs().GetFileDigest( env.VfsPath( E::TopFile ), digest ) && digest.Matches( right ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Without a block cache: a thread that waits reads through a window, a thread that must not wait cannot read bytes (NOT_SUPPORTED), and metadata and events work as with a cache" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-nocache" );
        E& env = envHolder.Get();
        SRemoteEnvOptions options;
        options.useCache = false;
        REQUIRE( env.Setup( options ) );
        CORE::CDynamicBuffer content;
        ASSERT_TRUE( env.ReadAll( E::MidFile, content ) && env.Matches( E::MidFile, 0, content, 300000 ) );
        VFS::CVfsCacheState state;
        ASSERT_FALSE( env.Vfs().GetFileCacheState( env.VfsPath( E::MidFile ), state ) );

        VFS::CVfsThreadPolicyScope marked( Marked() );
        VFS::TBasicVfsResourcePtr resource;
        VFS::CVfsCachedIOAccess* access = Open( env, E::MidFile, resource );
        REQUIRE( GUCEF_NULL != access );
        CORE::CDynamicBuffer buffer;
        ASSERT_TRUE( 0 == ReadAt( access, 0, 100, buffer ) );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_SUPPORTED == access->GetLastResult().result );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Failure injection: a link that drops in the middle of a run keeps the blocks that arrived whole, the read ends with the transport result, and a retry fetches only what is missing" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-cut" );
        E& env = envHolder.Get();
        SRemoteEnvOptions options;
        options.maxBlocksPerFetch = 4;
        REQUIRE( env.Setup( options ) );
        VFS::TBasicVfsResourcePtr resource;
        VFS::CVfsCachedIOAccess* access = Open( env, E::BigFile, resource );
        REQUIRE( GUCEF_NULL != access );
        env.Backend().CutNextFetchAfter( 2 * (UInt64) B + 500 );

        CORE::CDynamicBuffer buffer;
        const UInt32 got = ReadAt( access, 0, 4 * B, buffer );
        ASSERT_TRUE( 2 * B == got );
        ASSERT_TRUE( env.Matches( E::BigFile, 0, buffer, got ) );
        ASSERT_TRUE( env.GetCutFetchResult() == access->GetLastResult().result );
        VFS::CVfsCacheState state;
        ASSERT_TRUE( env.Vfs().GetFileCacheState( env.VfsPath( E::BigFile ), state ) && 2 * B == state.bytesHeld );

        env.Backend().ResetCounters();
        ASSERT_TRUE( 4 * B == ReadAt( access, 0, 4 * B, buffer ) );
        ASSERT_TRUE( env.Matches( E::BigFile, 0, buffer, 4 * B ) );
        ASSERT_TRUE( 2 * B == env.Backend().GetBytesServed() );

        // a failure of the remote place that is not a link: a job that fails ends with the failure event, the slot is free again
        VFS::CVfsThreadPolicyScope marked( Marked() );
        env.Backend().FailNext( CTestRemotePlace::OP_STAT, 1, CORE::FILESYSTEMACCESS_TIMEOUT );
        ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( E::ReadmeFile ) ) );
        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::FetchFailedEvent, E::ReadmeFile, 1, 4000 ) );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_TIMEOUT == env.Events().Find( VFS::CRemoteArchiveEvents::FetchFailedEvent )[ 0 ].result );
        REQUIRE( env.WaitUntilIdle( 4000 ) );
        ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( E::ReadmeFile ) ) );          // asks again
        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::MetadataArrivedEvent, E::ReadmeFile, 1, 4000 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Failure injection: a file that changes during a whole file fetch ends the fetch as CHANGED and the blocks of the old revision stay in the superseded copy, none in the new one" )
    try
    {
        CEnvHolder envHolder( "gucefvfs-midchange" );
        E& env = envHolder.Get();
        SRemoteEnvOptions options;
        options.maxBlocksPerFetch = 2;
        REQUIRE( env.Setup( options ) );
        env.Backend().SetOnFetchHook( 2, E::MidFile );              // the file moves after the second fetch

        VFS::CVfsFetchProgress progress;
        const VFS::CVfsCallResult result = env.Vfs().FetchFile( env.VfsPath( E::MidFile ), VFS::CVfsFetchRequest(), progress );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_RESOURCE_CHANGED == result.result );
        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::FileChangedEvent, E::MidFile, 1, 4000 ) );

        // the next fetch is of the new revision and ends complete with the bytes of the new revision only
        env.Backend().SetOnFetchHook( 0, CORE::CString::Empty );
        VFS::CVfsFetchProgress second;
        ASSERT_TRUE( env.Vfs().FetchFile( env.VfsPath( E::MidFile ), VFS::CVfsFetchRequest(), second ).IsSuccess() );
        CORE::CDynamicBuffer content;
        ASSERT_TRUE( env.ReadAll( E::MidFile, content ) && env.Matches( E::MidFile, 0, content, 300000 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

}

/*-------------------------------------------------------------------------*/

void
PerformRemoteArchiveCasesOn( TRemoteEnvFactory factory, const char* suiteName )
{
    g_envFactory = factory;
    PerformRemoteArchiveCases( suiteName );
}

/*-------------------------------------------------------------------------*/
