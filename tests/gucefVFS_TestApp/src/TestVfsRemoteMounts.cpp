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
#include "TestRemoteArchiveFixture.h"

#include "TestVfsRemoteMounts.h"

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

/** Mounts a directory with one file below the mount path of the remote archive, behind it: the mount that a stop at a remote miss must not reach */
class CLocalTwin
{
    public:

    CLocalTwin( VFS::CVFS& vfs )
        : m_vfs( vfs )
        , m_folder( "localtwin" )
        , m_mounted( false )
    {
        CORE::CDynamicBuffer content;
        content.Append( "LOCAL", 5 );
        const CORE::CString file = m_folder.GetChildPath( "docs/readme.txt" );
        CORE::CreatePathDirectories( file );
        BlockCacheTest::WriteWholeFile( file, content );
        const CORE::CString only = m_folder.GetChildPath( "docs/only_local.txt" );
        BlockCacheTest::WriteWholeFile( only, content );
        m_mounted = m_vfs.AddRoot( m_folder.GetPath(), "localtwin", false, false, CRemoteTestEnv::MountPath );
    }

    ~CLocalTwin()
    {
        m_vfs.UnmountArchiveByName( "localtwin" );
    }

    bool IsMounted( void ) const { return m_mounted; }

    private:

    CLocalTwin( const CLocalTwin& src );
    CLocalTwin& operator=( const CLocalTwin& src );

    VFS::CVFS& m_vfs;
    CBlockCacheTestFolder m_folder;
    bool m_mounted;
};

/** What the calls of the VFS that loop over the mounts say of a path: which of them gave the answer of the local twin */
struct SLoopAnswers
{
    bool fileExists;
    bool dirExists;
    UInt64 fileSize;
    bool gotFile;
    bool metaData;
    bool hashNotEmpty;
    bool digest;
    bool modificationTime;
    bool inFileList;
    bool inDirList;
    SLoopAnswers( void ) : fileExists( false ), dirExists( false ), fileSize( 0 ), gotFile( false ), metaData( false ), hashNotEmpty( false ), digest( false ), modificationTime( false ), inFileList( false ), inDirList( false ) {}
};

SLoopAnswers
AskAll( VFS::CVFS& vfs, const CORE::CString& vfsFile, const CORE::CString& vfsDir, const CORE::CString& listedName )
{
    SLoopAnswers answers;
    answers.fileExists = vfs.FileExists( vfsFile );
    answers.dirExists = vfs.DirExists( vfsDir );
    answers.fileSize = vfs.GetFileSize( vfsFile );
    answers.gotFile = !vfs.GetFile( vfsFile ).IsNULL();
    CORE::CResourceMetaData metaData;
    answers.metaData = vfs.GetFileMetaData( vfsFile, metaData );
    answers.hashNotEmpty = !vfs.GetFileHash( vfsFile ).IsNULLOrEmpty();
    VFS::CVfsDigest digest;
    answers.digest = vfs.GetFileDigest( vfsFile, digest );
    answers.modificationTime = vfs.GetFileModificationTime( vfsFile ).ToUnixEpochBasedTicksInMillisecs() != CORE::CDateTime().ToUnixEpochBasedTicksInMillisecs();
    VFS::CVFS::TStringVector files, dirs;
    vfs.GetFileList( files, vfsDir, false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX );
    for ( size_t i=0; i<files.size(); ++i )
        if ( files[ i ] == listedName )
            answers.inFileList = true;
    vfs.GetDirList( dirs, vfsDir, false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX );
    answers.inDirList = !dirs.empty();
    return answers;
}

/** An observer behind a notifier of its own that is pumped by a thread of its own */
class CPumpThreadBody : public CTestThreadBody
{
    public:

    CORE::PulseGeneratorPtr pulse;
    MT::CTAtomic< bool > stop;
    MT::CTAtomic< UInt64 > pumpThread;

    explicit CPumpThreadBody( CORE::PulseGeneratorPtr p ) : pulse( p ), stop( false ), pumpThread( 0 ) {}

    virtual void Run( UInt32 /* index */ ) GUCEF_VIRTUAL_OVERRIDE
    {
        pumpThread.Store( MT::GetCurrentThreadID() );
        while ( !stop.Load() )
        {
            pulse->ForceDirectPulse();
            MT::ThreadDelay( 1 );
        }
        pulse->ForceDirectPulse();
    }
};

/** Tells the pump thread to stop when the scope ends, also when a case ends early */
class CStopOnExit
{
    public:

    explicit CStopOnExit( CPumpThreadBody& body ) : m_body( body ) {}
    ~CStopOnExit() { m_body.stop.Store( true ); }

    private:

    CStopOnExit( const CStopOnExit& src );
    CStopOnExit& operator=( const CStopOnExit& src );

    CPumpThreadBody& m_body;
};

/** Reads the last result of the VFS on its own thread, after another thread recorded one */
class CLastResultOtherThread : public CTestThreadBody
{
    public:

    MT::CTAtomic< UInt32 > seen;
    CLastResultOtherThread( void ) : seen( 999 ) {}

    virtual void Run( UInt32 /* index */ ) GUCEF_VIRTUAL_OVERRIDE
    {
        VFS::CVfsCallResult result;
        VFS::CVfsGlobal::Instance()->GetVfs().GetLastResult( result );
        seen.Store( (UInt32) result.result );
    }
};

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformVfsRemoteMountsTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "VfsRemoteMounts" );

    GUCEF_TESTFW_TESTCASE( "Events: an event raised on a pool thread reaches the observers of the VFS on the thread that pumps the pulse generator, with the mount path in front of the path" )
    try
    {
        CRemoteTestEnv env( "gucefvfs-evt1" );
        REQUIRE( env.Setup() );
        VFS::CVfsThreadPolicyScope marked( ( VFS::CVfsThreadPolicy( VFS::MISSACTION_FAIL_AND_FETCH ) ) );
        ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( CRemoteTestEnv::ReadmeFile ) ) );
        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::MetadataArrivedEvent, CRemoteTestEnv::ReadmeFile, 1, 4000 ) );

        CTestEventRecorder::TRecordVector records = env.Events().Find( VFS::CRemoteArchiveEvents::MetadataArrivedEvent, env.VfsPath( CRemoteTestEnv::ReadmeFile ) );
        ASSERT_TRUE( 1 == records.size() );
        ASSERT_TRUE( records[ 0 ].thread == MT::GetCurrentThreadID() );
        ASSERT_TRUE( records[ 0 ].thread != env.Backend().GetLastCallThreadId() );
        ASSERT_TRUE( env.Backend().GetLastCallThreadId() != 0 );
        ASSERT_TRUE( records[ 0 ].path == env.VfsPath( CRemoteTestEnv::ReadmeFile ) );
        ASSERT_TRUE( records[ 0 ].revision == env.Backend().GetRevision( CRemoteTestEnv::ReadmeFile ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Events: two observers each get every event once, and one that subscribes late gets nothing of what came before" )
    try
    {
        CRemoteTestEnv env( "gucefvfs-evt2" );
        REQUIRE( env.Setup() );
        CTestEventRecorder second;
        second.Listen( &env.Vfs() );

        VFS::CVfsThreadPolicyScope marked( ( VFS::CVfsThreadPolicy( VFS::MISSACTION_FAIL_AND_FETCH ) ) );
        ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( CRemoteTestEnv::TopFile ) ) );
        ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( CRemoteTestEnv::ReadmeFile ) ) );
        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::MetadataArrivedEvent, CRemoteTestEnv::ReadmeFile, 1, 4000 ) );
        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::MetadataArrivedEvent, CRemoteTestEnv::TopFile, 1, 4000 ) );
        CRemoteTestEnv::Pump();

        ASSERT_TRUE( 2 == env.Events().Count( VFS::CRemoteArchiveEvents::MetadataArrivedEvent ) );
        ASSERT_TRUE( 2 == second.Count( VFS::CRemoteArchiveEvents::MetadataArrivedEvent ) );
        ASSERT_TRUE( 1 == second.Count( VFS::CRemoteArchiveEvents::MetadataArrivedEvent, env.VfsPath( CRemoteTestEnv::TopFile ) ) );

        CTestEventRecorder late;
        late.Listen( &env.Vfs() );
        CRemoteTestEnv::Pump();
        ASSERT_TRUE( 0 == late.GetTotal() );

        // and it gets what comes after
        ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( CRemoteTestEnv::DeepFile ) ) );
        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::MetadataArrivedEvent, CRemoteTestEnv::DeepFile, 1, 4000 ) );
        CRemoteTestEnv::Pump();
        ASSERT_TRUE( 1 == late.GetTotal() );
        ASSERT_TRUE( 1 == second.Count( VFS::CRemoteArchiveEvents::MetadataArrivedEvent, env.VfsPath( CRemoteTestEnv::DeepFile ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Events: the events arrive in the order the jobs ended" )
    try
    {
        CRemoteTestEnv env( "gucefvfs-evt3" );
        SRemoteEnvOptions options;
        options.maxRunning = 1;
        REQUIRE( env.Setup( options ) );
        const char* const names[ 5 ] = { CRemoteTestEnv::ReadmeFile, CRemoteTestEnv::NotesFile, CRemoteTestEnv::BigFile, CRemoteTestEnv::MidFile, CRemoteTestEnv::TopFile };
        VFS::CVfsThreadPolicyScope marked( ( VFS::CVfsThreadPolicy( VFS::MISSACTION_FAIL_AND_FETCH ) ) );
        for ( UInt32 i=0; i<5; ++i )
        {
            ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( names[ i ] ) ) );
        }
        REQUIRE( env.WaitUntilIdle( 4000 ) );
        CTestEventRecorder::TRecordVector all = env.Events().GetAll();
        ASSERT_TRUE( 5 == all.size() );
        for ( UInt32 i=0; i<5; ++i )
        {
            ASSERT_TRUE( all[ i ].path == env.VfsPath( names[ i ] ) );
            ASSERT_TRUE( i == 0 || all[ i ].sequence > all[ i - 1 ].sequence );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Events: an observer behind a notifier of its own on a pulse generator of its own receives on the thread that pumps that one" )
    try
    {
        CRemoteTestEnv env( "gucefvfs-evt4" );
        REQUIRE( env.Setup() );

        CORE::PulseGeneratorPtr ownPulse = CORE::CPulseGenerator::CreateSharedObj();
        CORE::CTSGNotifier gateway( ownPulse, false, true );
        gateway.SubscribeTo( &env.Vfs() );
        CTestEventRecorder behind;
        behind.Listen( &gateway );

        CPumpThreadBody pumpBody( ownPulse );
        {
            CTestThreads pump( pumpBody, 1 );
            CStopOnExit stopper( pumpBody );
            ASSERT_TRUE( pump.Start() );

            VFS::CVfsThreadPolicyScope marked( ( VFS::CVfsThreadPolicy( VFS::MISSACTION_FAIL_AND_FETCH ) ) );
            ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( CRemoteTestEnv::ReadmeFile ) ) );
            REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::MetadataArrivedEvent, CRemoteTestEnv::ReadmeFile, 1, 4000 ) );
            for ( UInt32 waited=0; waited<5000 && 0 == behind.Count( VFS::CRemoteArchiveEvents::MetadataArrivedEvent ); ++waited )
                MT::ThreadDelay( 1 );

            pumpBody.stop.Store( true );
            ASSERT_TRUE( pump.Join( 10000 ) );
        }
        CTestEventRecorder::TRecordVector records = behind.Find( VFS::CRemoteArchiveEvents::MetadataArrivedEvent );
        ASSERT_TRUE( 1 == records.size() );
        ASSERT_TRUE( records[ 0 ].thread == pumpBody.pumpThread.Load() );
        ASSERT_TRUE( records[ 0 ].thread != MT::GetCurrentThreadID() );
        gateway.UnsubscribeFrom( &env.Vfs() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Events: an application with one thread, which calls under FAIL_AND_FETCH and pumps its own pulse generator, gets its events with nothing but that pump" )
    try
    {
        CRemoteTestEnv env( "gucefvfs-evt5" );
        env.Setup();
        env.Backend().SetDelayMs( 30 );
        VFS::CVfsThreadPolicyScope marked( ( VFS::CVfsThreadPolicy( VFS::MISSACTION_FAIL_AND_FETCH ) ) );
        const UInt64 start = CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
        ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( CRemoteTestEnv::ReadmeFile ) ) );
        ASSERT_TRUE( CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs() - start < 25 );
        ASSERT_TRUE( 0 == env.Events().GetTotal() );
        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::MetadataArrivedEvent, CRemoteTestEnv::ReadmeFile, 1, 4000 ) );
        CTestEventRecorder::TRecordVector all = env.Events().GetAll();
        for ( size_t i=0; i<all.size(); ++i )
        {
            ASSERT_TRUE( all[ i ].thread == MT::GetCurrentThreadID() );
        }
        // the same call now answers
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( CRemoteTestEnv::ReadmeFile ) ) );
        ASSERT_TRUE( 1000 == env.Vfs().GetFileSize( env.VfsPath( CRemoteTestEnv::ReadmeFile ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Events: the link going down and coming back is passed on once per change with the mount path, and a repeated state says nothing" )
    try
    {
        CRemoteTestEnv env( "gucefvfs-evt6" );
        REQUIRE( env.Setup() );
        env.Backend().SetLinkUp( true );
        env.Backend().SetLinkUp( false );
        env.Backend().SetLinkUp( false );
        env.Backend().SetLinkUp( true );
        CRemoteTestEnv::Pump();
        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::ConnectionStateChangedEvent, CORE::CString::Empty, 2, 4000 ) );
        CTestEventRecorder::TRecordVector records = env.Events().Find( VFS::CRemoteArchiveEvents::ConnectionStateChangedEvent );
        ASSERT_TRUE( 2 == records.size() );
        ASSERT_TRUE( !records[ 0 ].connected && records[ 1 ].connected );
        ASSERT_TRUE( records[ 0 ].mountPath == CORE::CString( CRemoteTestEnv::MountPath ) );
        ASSERT_TRUE( records[ 0 ].sequence < records[ 1 ].sequence );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Events: nothing is raised once the archive is unloaded: jobs under way are dropped, what was mailed before is dropped by the VFS, and no thread is left blocked" )
    try
    {
        CRemoteTestEnv env( "gucefvfs-evt7" );
        REQUIRE( env.Setup() );
        {
            VFS::CVfsThreadPolicyScope marked( ( VFS::CVfsThreadPolicy( VFS::MISSACTION_FAIL_AND_FETCH ) ) );
            ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( CRemoteTestEnv::ReadmeFile ) ) );
            ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( CRemoteTestEnv::ReadmeFile ) ) );
            ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( CRemoteTestEnv::TopFile ) ) );
            ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( CRemoteTestEnv::DeepFile ) ) );
        }
        // the jobs are under way (the backend takes 150 ms per call)
        MT::ThreadDelay( 30 );
        REQUIRE( env.Unmount() );
        ASSERT_TRUE( 0 == env.Scheduler().GetRunningCount() && 0 == env.Scheduler().GetQueuedCount() );
        for ( UInt32 i=0; i<400; ++i )
        {
            CRemoteTestEnv::Pump();
            MT::ThreadDelay( 1 );
        }
        ASSERT_TRUE( 0 == env.Events().Count( VFS::CRemoteArchiveEvents::MetadataArrivedEvent ) );
        ASSERT_TRUE( 0 == env.Events().Count( VFS::CRemoteArchiveEvents::FetchFailedEvent ) );

        // an event that was mailed before the unmount is dropped
        env.Backend().SetDelayMs( 0 );
        REQUIRE( env.Restart( false ) );
        {
            VFS::CVfsThreadPolicyScope marked( ( VFS::CVfsThreadPolicy( VFS::MISSACTION_FAIL_AND_FETCH ) ) );
            ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( CRemoteTestEnv::ReadmeFile ) ) );
        }
        ASSERT_TRUE( env.Scheduler().WaitUntilIdle( 10000 ) );
        REQUIRE( env.Unmount() );
        for ( UInt32 i=0; i<50; ++i )
        {
            CRemoteTestEnv::Pump();
            MT::ThreadDelay( 1 );
        }
        ASSERT_TRUE( 0 == env.Events().GetTotal() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "StopRule: a remote mount that has no answer yet stops the calls that loop over the mounts: the file of the mount below it with the same path is not given, in any of the ten calls" )
    try
    {
        CRemoteTestEnv env( "gucefvfs-stop1" );
        REQUIRE( env.Setup() );
        CLocalTwin twin( env.Vfs() );
        REQUIRE( twin.IsMounted() );

        {
            VFS::CVfsThreadPolicyScope marked( ( VFS::CVfsThreadPolicy( VFS::MISSACTION_FAIL ) ) );
            const SLoopAnswers answers = AskAll( env.Vfs(), env.VfsPath( CRemoteTestEnv::ReadmeFile ), env.VfsPath( "docs" ), "readme.txt" );
            ASSERT_FALSE( answers.fileExists );
            ASSERT_FALSE( answers.dirExists );
            ASSERT_TRUE( 0 == answers.fileSize );
            ASSERT_FALSE( answers.gotFile );
            ASSERT_FALSE( answers.metaData );
            ASSERT_FALSE( answers.hashNotEmpty );
            ASSERT_FALSE( answers.digest );
            ASSERT_FALSE( answers.modificationTime );
            ASSERT_FALSE( answers.inFileList );
            ASSERT_FALSE( answers.inDirList );
            VFS::CVfsCallResult result;
            env.Vfs().GetLastResult( result );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE == result.result );
        }

        // a thread that waits gets the remote answer, which is the mount above the twin
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( CRemoteTestEnv::ReadmeFile ) ) );
        ASSERT_TRUE( 1000 == env.Vfs().GetFileSize( env.VfsPath( CRemoteTestEnv::ReadmeFile ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "StopRule: while the link is down a remote mount that has nothing stops the calls with CONNECTION_PENDING; a real absence (the remote place says no) falls through, and a path only the local mount has is found" )
    try
    {
        CRemoteTestEnv env( "gucefvfs-stop2" );
        REQUIRE( env.Setup() );
        CLocalTwin twin( env.Vfs() );
        REQUIRE( twin.IsMounted() );

        // the remote place says it has no such file: that is an answer, the twin is asked
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( "docs/only_local.txt" ) ) );
        ASSERT_TRUE( 5 == env.Vfs().GetFileSize( env.VfsPath( "docs/only_local.txt" ) ) );
        VFS::CVfsCallResult result;
        env.Vfs().GetLastResult( result );
        ASSERT_TRUE( result.IsSuccess() );

        // the link goes down: what the remote mount does not know is not known, whatever the twin has
        env.Backend().SetLinkUp( false );
        env.Vfs().InvalidateRemoteMetadata();
        ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( "docs/only_local.txt" ) ) );
        env.Vfs().GetLastResult( result );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == result.result );
        ASSERT_TRUE( 0 == env.Vfs().GetFileSize( env.VfsPath( CRemoteTestEnv::ReadmeFile ) ) );
        ASSERT_TRUE( env.Vfs().GetFile( env.VfsPath( CRemoteTestEnv::ReadmeFile ) ).IsNULL() );
        env.Vfs().GetLastResult( result );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == result.result );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "StopRule: a local mount never stops the calls: a missing file falls through to the next mount, and the result of the VFS is a success after a call that went well" )
    try
    {
        CBlockCacheTestFolder first( "stoplocal1" );
        CBlockCacheTestFolder second( "stoplocal2" );
        CORE::CDynamicBuffer content;
        content.Append( "SECOND", 6 );
        const CORE::CString file = second.GetChildPath( "d/f.txt" );
        CORE::CreatePathDirectories( file );
        BlockCacheTest::WriteWholeFile( file, content );

        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        ASSERT_TRUE( vfs.AddRoot( first.GetPath(), "stoplocal1", false, false, "/stoplocal/" ) );
        ASSERT_TRUE( vfs.AddRoot( second.GetPath(), "stoplocal2", false, false, "/stoplocal/" ) );
        const bool found = vfs.FileExists( "/stoplocal/d/f.txt" );
        const UInt64 size = vfs.GetFileSize( "/stoplocal/d/f.txt" );
        const bool missing = vfs.FileExists( "/stoplocal/d/none.txt" );
        VFS::CVfsCallResult result;
        vfs.GetLastResult( result );
        ASSERT_TRUE( vfs.UnmountArchiveByName( "stoplocal1" ) );
        ASSERT_TRUE( vfs.UnmountArchiveByName( "stoplocal2" ) );
        ASSERT_TRUE( found && 6 == size );
        ASSERT_FALSE( missing );
        ASSERT_TRUE( result.IsSuccess() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "StopRule: the listing of a remote mount that is not complete yet is not completed with the mount below it; a complete listing is the union of the mounts as before" )
    try
    {
        CRemoteTestEnv env( "gucefvfs-stop3" );
        REQUIRE( env.Setup() );
        CLocalTwin twin( env.Vfs() );
        REQUIRE( twin.IsMounted() );

        VFS::CVFS::TStringVector files;
        {
            VFS::CVfsThreadPolicyScope marked( ( VFS::CVfsThreadPolicy( VFS::MISSACTION_FAIL_AND_FETCH ) ) );
            ASSERT_FALSE( env.Vfs().GetFileList( files, env.VfsPath( "docs" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX ) );
            ASSERT_TRUE( files.empty() );
            REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::ListingArrivedEvent, "docs/", 1, 4000 ) );
            // the event of a listing is about the directory
            ASSERT_TRUE( 1 == env.Events().Count( VFS::CRemoteArchiveEvents::ListingArrivedEvent, env.VfsPath( "docs/" ) ) );
            VFS::CVfsCallResult result;
            ASSERT_TRUE( env.Vfs().GetFileList( files, env.VfsPath( "docs" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX ) );
            env.Vfs().GetLastResult( result );
            ASSERT_TRUE( result.IsSuccess() );
        }
        bool remoteReadme = false, localOnly = false;
        for ( size_t i=0; i<files.size(); ++i )
        {
            if ( files[ i ] == "readme.txt" ) remoteReadme = true;
            if ( files[ i ] == "only_local.txt" ) localOnly = true;
        }
        ASSERT_TRUE( remoteReadme && localOnly );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "StopRule: the result of the VFS is kept per thread" )
    try
    {
        CRemoteTestEnv env( "gucefvfs-stop4" );
        REQUIRE( env.Setup() );
        {
            VFS::CVfsThreadPolicyScope marked( ( VFS::CVfsThreadPolicy( VFS::MISSACTION_FAIL ) ) );
            ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( CRemoteTestEnv::ReadmeFile ) ) );
        }
        VFS::CVfsCallResult mine;
        env.Vfs().GetLastResult( mine );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE == mine.result );

        CLastResultOtherThread body;
        {
            CTestThreads threads( body, 1 );
            ASSERT_TRUE( threads.Start() );
            ASSERT_TRUE( threads.Join( 10000 ) );
        }
        ASSERT_TRUE( (UInt32) CORE::FILESYSTEMACCESS_SUCCESS == body.seen.Load() );

        // a call that goes well on this thread removes the result
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( CRemoteTestEnv::ReadmeFile ) ) );
        env.Vfs().GetLastResult( mine );
        ASSERT_TRUE( mine.IsSuccess() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "VFS wrappers: fetching a file of a mount that is not remote says NOT_SUPPORTED, and the cache state and pins of a path that no remote mount covers say no" )
    try
    {
        CBlockCacheTestFolder folder( "wrapperlocal" );
        CORE::CDynamicBuffer content;
        content.Append( "LOCAL", 5 );
        const CORE::CString file = folder.GetChildPath( "f.txt" );
        BlockCacheTest::WriteWholeFile( file, content );
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        ASSERT_TRUE( vfs.AddRoot( folder.GetPath(), "wrapperlocal", false, false, "/wrapperlocal/" ) );

        VFS::CVfsFetchProgress progress;
        const VFS::CVfsCallResult fetched = vfs.FetchFile( "/wrapperlocal/f.txt", VFS::CVfsFetchRequest(), progress );
        VFS::CVfsCacheState state;
        const bool hasState = vfs.GetFileCacheState( "/wrapperlocal/f.txt", state );
        const bool pinned = vfs.SetFilePinned( "/wrapperlocal/f.txt", true );
        VFS::CVfsRemoteStatistics statistics;
        const bool hasStatistics = vfs.GetRemoteStatistics( "/wrapperlocal/f.txt", statistics );
        const bool noted = vfs.NoteFileDigest( "/wrapperlocal/f.txt", VFS::CVfsDigest() );
        vfs.UnmountArchiveByName( "wrapperlocal" );

        ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_SUPPORTED == fetched.result );
        ASSERT_FALSE( hasState );
        ASSERT_FALSE( pinned );
        ASSERT_FALSE( hasStatistics );
        ASSERT_FALSE( noted );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
