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

#include <string.h>

#include <exception>

#ifndef GUCEF_MT_H
#include "gucefMT.h"
#define GUCEF_MT_H
#endif /* GUCEF_MT_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

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

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEMOUNTER_H
#include "vfspluginDRN_CDrnBridgeMounter.h"
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEMOUNTER_H
#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEMOUNTER_H ? */

#include "TestDrnBridgeFixture.h"
#include "TestDrnBridgeMounting.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace DRNBRIDGETEST;
using namespace VFSPLUGIN::DRNBRIDGE;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)
#define REQUIRE_TRUE(t)  do { if ( !(t) ) { ::GUCEF::TESTFW::CTestContext::Instance().RecordFailure( #t, __FILE__, __LINE__ ); throw std::exception(); } } while ( 0 )

namespace {

/*-------------------------------------------------------------------------*/

UInt8
ByteAt( UInt8 seed, UInt64 position )
{
    return (UInt8) ( seed + position * 7 + ( position >> 9 ) );
}

CORE::CString
BedDirectory( const CORE::CString& name )
{
    CORE::CString base = CORE::GetEnv( "GUCEF_TEST_SCRATCH_DIR" );
    if ( base.IsNULLOrEmpty() )
        base = CORE::TempDir();
    CORE::CString dir = CORE::CombinePath( base, "vfsdrn-mount-" + CORE::UInt32ToString( MT::GetProcessID() ) + "-" + name );
    if ( CORE::DirExists( dir ) )
        CORE::DeleteDir( dir, true );
    CORE::CreateDirs( dir );
    return dir;
}

bool
WriteSeeded( const CORE::CString& path, UInt64 size, UInt8 seed )
{
    if ( !CORE::CreatePathDirectories( path ) )
        return false;
    CORE::CFileAccess file( path, "wb" );
    if ( !file.Opened() )
        return false;
    CORE::CDynamicBuffer block( (UInt32) size, true );
    UInt8* out = static_cast< UInt8* >( block.GetBufferPtr() );
    for ( UInt64 i=0; i<size; ++i )
        out[ i ] = ByteAt( seed, i );
    if ( size != file.Write( block.GetConstBufferPtr(), 1, (UInt32) size ) )
        return false;
    file.Flush();
    return true;
}

bool
ReadSeeded( CORE::CIOAccess& access, UInt64 offset, UInt32 count, UInt8 seed )
{
    CORE::CDynamicBuffer buffer( count, true );
    if ( 0 != access.Setpos( offset ) )
        return false;
    if ( count != access.Read( buffer.GetBufferPtr(), 1, count ) )
        return false;
    const UInt8* in = static_cast< const UInt8* >( buffer.GetConstBufferPtr() );
    for ( UInt32 i=0; i<count; ++i )
        if ( in[ i ] != ByteAt( seed, offset + i ) )
            return false;
    return true;
}

/** A directory and the node pair that serves it under a session name, started and stopped as the test wants */
class CServedDirectory
{
    public:

    CORE::CString name;
    CORE::CString sessionName;
    CORE::CString root;
    CLoopbackShare* share;

    CServedDirectory( const CORE::CString& bedName, const CORE::CString& sessionNameToServeAs )
        : name( bedName ), sessionName( sessionNameToServeAs ), root( BedDirectory( bedName ) ), share( GUCEF_NULL ) {}

    ~CServedDirectory()
    {
        Stop();
        if ( !root.IsNULLOrEmpty() )
            CORE::DeleteDir( root, true );
    }

    /** Serves the directory: a session of the name is there afterwards, registered in the DRN session registry when 'registered' */
    bool Start( bool registered )
    {
        Stop();
        share = GUCEF_NEW CLoopbackShare( sessionName );
        if ( !share->Start( root ) )
            return false;
        if ( registered )
            share->RegisterClientSession();
        return true;
    }

    void Stop( void )
    {
        if ( GUCEF_NULL != share )
        {
            share->Stop();
            GUCEF_DELETE share;
            share = GUCEF_NULL;
        }
    }

    CORE::CString Path( const CORE::CString& relative ) const
    {
        return CORE::CombinePath( root, relative );
    }
};

CORE::CString
UniqueSessionName( const CORE::CString& tag )
{
    return "vfsdrn_" + tag + "_" + CORE::UInt32ToString( MT::GetProcessID() );
}

/** Whether the archive is connected within the time, polled because the answer follows the nodes' own threads */
bool
BecomesConnected( VFS::CArchive& archive, UInt32 timeoutMs )
{
    const UInt64 start = MT::PrecisionTickCount();
    while ( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) < timeoutMs )
    {
        if ( archive.IsConnected() )
            return true;
        MT::ThreadDelay( 5 );
    }
    return archive.IsConnected();
}

bool
StateBecomes( CDrnBridgeMounter& mounter, CDrnBridgeMounter::EState state, UInt32 timeoutMs )
{
    const UInt64 start = MT::PrecisionTickCount();
    while ( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) < timeoutMs )
    {
        if ( state == mounter.GetState() )
            return true;
        MT::ThreadDelay( 5 );
    }
    return state == mounter.GetState();
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE CASES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

void
TestLateBinding( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Late binding: a mount made before the session exists says the share cannot be reached, and works by itself when the session is registered" )
        const CORE::CString sessionName = UniqueSessionName( "late" );
        CServedDirectory served( "late", sessionName );
        try
        {
            REQUIRE_TRUE( WriteSeeded( served.Path( "a.bin" ), 5000, 11 ) );
            REQUIRE_TRUE( WriteSeeded( served.Path( "dir/b.bin" ), 700, 12 ) );
            REQUIRE_TRUE( WriteSeeded( served.Path( "c.bin" ), 70, 13 ) );
            CBridgeMountOptions options;

            // no session of the name exists: the mount is made all the same
            VFS::CVFS::TArchivePtr archive = CreateBridgeArchive( "Late", "drn://" + sessionName + "/sequencer/files", options );
            REQUIRE_TRUE( !archive.IsNULL() );
            CDrnBridgeArchive& bridge = *static_cast< CDrnBridgeArchive* >( archive.GetPointerAlways() );
            ASSERT_TRUE( bridge.GetConnection()->IsBoundByName() );
            ASSERT_FALSE( archive->IsConnected() );
            ASSERT_TRUE( archive->IsHealthy() );

            // every call answers "pending", with the value of the file system result for it, and nothing waits
            const UInt64 start = MT::PrecisionTickCount();
            ASSERT_FALSE( archive->FileExists( "a.bin" ) );
            ASSERT_TRUE( bridge.GetLastResult().IsUnreachable() );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == bridge.GetLastResult().fileSystemResult );
            ASSERT_TRUE( bridge.GetLastResult().message.HasSubstr( "cannot be reached" ) >= 0 );
            ASSERT_TRUE( bridge.GetLastResult().message.HasSubstr( sessionName ) >= 0 );
            ASSERT_TRUE( 0 == archive->GetFileSize( "a.bin" ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == bridge.GetLastResult().fileSystemResult );
            ASSERT_TRUE( archive->GetFile( "a.bin", "rb", 0, false ).IsNULL() );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == bridge.GetLastResult().fileSystemResult );
            CORE::CString::StringVector names;
            archive->GetFileList( names, CORE::CString::Empty, "dir", false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX );
            ASSERT_TRUE( names.empty() );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == bridge.GetLastResult().fileSystemResult );
            ASSERT_TRUE( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) < 2000.0 );

            // the session appears: made, joined and registered after the mount
            REQUIRE_TRUE( served.Start( true ) );
            ASSERT_TRUE( BecomesConnected( *archive, 10000 ) );
            ASSERT_TRUE( archive->IsHealthy() );
            ASSERT_TRUE( 5000 == archive->GetFileSize( "a.bin" ) );
            ASSERT_TRUE( bridge.GetLastResult().IsOk() );
            names.clear();
            archive->GetFileList( names, CORE::CString::Empty, "dir", false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX );
            ASSERT_TRUE( 1 == names.size() );
            VFS::TBasicVfsResourcePtr open = archive->GetFile( "a.bin", "rb", 0, false );
            REQUIRE_TRUE( !open.IsNULL() );
            ASSERT_TRUE( ReadSeeded( *open->GetAccess(), 0, 5000, 11 ) );
            CORE::CResourceMetaData metaData;
            ASSERT_TRUE( archive->GetFileMetaData( "a.bin", metaData ) );
            ASSERT_TRUE( !metaData.hasIsOffline || !metaData.isOffline );

            // a session that is not registered (any more) is a session that is not there: the mount follows the registry, and again when it is back
            DRN::CDRNGlobal::Instance()->GetSessionRegistry().Unregister( served.share->Client() );
            ASSERT_FALSE( archive->IsConnected() );
            ASSERT_TRUE( archive->IsHealthy() );
            ASSERT_TRUE( 5000 == archive->GetFileSize( "a.bin" ) );     // what is cached answers
            ASSERT_FALSE( archive->FileExists( "c.bin" ) );              // what is not cached cannot be asked
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == bridge.GetLastResult().fileSystemResult );
            REQUIRE_TRUE( DRN::CDRNGlobal::Instance()->GetSessionRegistry().Register( served.share->Client() ) );
            ASSERT_TRUE( BecomesConnected( *archive, 10000 ) );
            ASSERT_TRUE( archive->FileExists( "c.bin" ) );
            ASSERT_TRUE( ReadSeeded( *open->GetAccess(), 100, 500, 11 ) );
            DRN::CDRNGlobal::Instance()->GetSessionRegistry().Unregister( served.share->Client() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        served.Stop();
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
TestReplacedSession( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Late binding: when the session is destroyed and another of the same name is registered, the mount recovers without being made again, and what was open reads again or fails as changed" )
        const CORE::CString sessionName = UniqueSessionName( "replaced" );
        CServedDirectory served( "replaced", sessionName );
        try
        {
            REQUIRE_TRUE( WriteSeeded( served.Path( "same.bin" ), 1000000, 21 ) );
            REQUIRE_TRUE( WriteSeeded( served.Path( "changes.bin" ), 1000000, 22 ) );
            REQUIRE_TRUE( WriteSeeded( served.Path( "info.bin" ), 1234, 23 ) );
            CBridgeMountOptions options;
            options.metadataTtlMs = 300;
            options.negativeTtlMs = 300;
            REQUIRE_TRUE( served.Start( true ) );
            VFS::CVFS::TArchivePtr archive = CreateBridgeArchive( "Replaced", "drn://" + sessionName + "/sequencer/files", options );
            REQUIRE_TRUE( !archive.IsNULL() );
            CDrnBridgeArchive& bridge = *static_cast< CDrnBridgeArchive* >( archive.GetPointerAlways() );
            ASSERT_TRUE( BecomesConnected( *archive, 10000 ) );

            // what is known and what is open before the session goes
            ASSERT_TRUE( 1234 == archive->GetFileSize( "info.bin" ) );
            VFS::TBasicVfsResourcePtr same = archive->GetFile( "same.bin", "rb", 0, false );
            VFS::TBasicVfsResourcePtr changes = archive->GetFile( "changes.bin", "rb", 0, false );
            REQUIRE_TRUE( !same.IsNULL() && !changes.IsNULL() );
            ASSERT_TRUE( ReadSeeded( *same->GetAccess(), 0, 1000, 21 ) );
            ASSERT_TRUE( ReadSeeded( *changes->GetAccess(), 0, 1000, 22 ) );
            MT::ThreadDelay( 400 );

            // the session is destroyed: the mount stays and says that it cannot be reached; what is cached answers, offline
            served.Stop();
            ASSERT_FALSE( archive->IsConnected() );
            ASSERT_TRUE( archive->IsHealthy() );
            ASSERT_TRUE( 1234 == archive->GetFileSize( "info.bin" ) );
            CORE::CResourceMetaData metaData;
            ASSERT_TRUE( archive->GetFileMetaData( "info.bin", metaData ) );
            ASSERT_TRUE( metaData.hasIsOffline && metaData.isOffline );
            ASSERT_FALSE( archive->FileExists( "never-asked.bin" ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == bridge.GetLastResult().fileSystemResult );

            // another session of the name, serving a directory in which one of the files changed meanwhile and a new one is
            REQUIRE_TRUE( WriteSeeded( served.Path( "changes.bin" ), 1000000, 99 ) );
            REQUIRE_TRUE( WriteSeeded( served.Path( "new.bin" ), 4321, 24 ) );
            REQUIRE_TRUE( served.Start( true ) );
            ASSERT_TRUE( BecomesConnected( *archive, 10000 ) );

            // the mount works again: no remount, a new file is there, the old ones are what they were
            ASSERT_TRUE( 4321 == archive->GetFileSize( "new.bin" ) );
            ASSERT_TRUE( archive->FileExists( "info.bin" ) );
            ASSERT_TRUE( bridge.GetLastResult().IsOk() );
            VFS::TBasicVfsResourcePtr fresh = archive->GetFile( "new.bin", "rb", 0, false );
            REQUIRE_TRUE( !fresh.IsNULL() );
            ASSERT_TRUE( ReadSeeded( *fresh->GetAccess(), 0, 4321, 24 ) );

            // the handles of the old session are dead: the file that did not change opens again at the revision it was pinned at and reads on,
            // the file that changed fails as changed and stays failed
            ASSERT_TRUE( ReadSeeded( *same->GetAccess(), 900000, 1000, 21 ) );
            ASSERT_TRUE( same->GetAccess()->IsValid() );
            CORE::CDynamicBuffer buffer( 100, true );
            ASSERT_TRUE( 0 == changes->GetAccess()->Setpos( 900000 ) );
            ASSERT_TRUE( 0 == changes->GetAccess()->Read( buffer.GetBufferPtr(), 1, 100 ) );
            ASSERT_FALSE( changes->GetAccess()->IsValid() );

            // opening it again gives the new revision
            VFS::TBasicVfsResourcePtr changed = archive->GetFile( "changes.bin", "rb", 0, false );
            REQUIRE_TRUE( !changed.IsNULL() );
            ASSERT_TRUE( ReadSeeded( *changed->GetAccess(), 0, 1000, 99 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        served.Stop();
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
TestFollowsTheRegistry( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Late binding: a second session that is alive is registered in place of the first one of the name: the mount follows the registry and reads the share of the second" )
        const CORE::CString sessionName = UniqueSessionName( "follows" );
        CServedDirectory first( "followsfirst", sessionName );
        CServedDirectory second( "followssecond", sessionName );
        try
        {
            REQUIRE_TRUE( WriteSeeded( first.Path( "v.bin" ), 1000, 71 ) );
            REQUIRE_TRUE( WriteSeeded( second.Path( "v.bin" ), 2000, 72 ) );
            CBridgeMountOptions options;
            options.metadataTtlMs = 100;
            REQUIRE_TRUE( first.Start( true ) );
            REQUIRE_TRUE( second.Start( false ) );
            VFS::CVFS::TArchivePtr archive = CreateBridgeArchive( "Follows", "drn://" + sessionName + "/sequencer/files", options );
            REQUIRE_TRUE( !archive.IsNULL() );
            ASSERT_TRUE( BecomesConnected( *archive, 10000 ) );
            ASSERT_TRUE( 1000 == archive->GetFileSize( "v.bin" ) );
            VFS::TBasicVfsResourcePtr open = archive->GetFile( "v.bin", "rb", 0, false );
            REQUIRE_TRUE( !open.IsNULL() );
            ASSERT_TRUE( ReadSeeded( *open->GetAccess(), 0, 1000, 71 ) );

            // the first session is not the registered one any more, the second is: both are alive
            DRN::CDRNGlobal::Instance()->GetSessionRegistry().Unregister( first.share->Client() );
            REQUIRE_TRUE( DRN::CDRNGlobal::Instance()->GetSessionRegistry().Register( second.share->Client() ) );
            MT::ThreadDelay( 200 );
            ASSERT_TRUE( BecomesConnected( *archive, 10000 ) );
            ASSERT_TRUE( 2000 == archive->GetFileSize( "v.bin" ) );
            VFS::TBasicVfsResourcePtr again = archive->GetFile( "v.bin", "rb", 0, false );
            REQUIRE_TRUE( !again.IsNULL() );
            ASSERT_TRUE( ReadSeeded( *again->GetAccess(), 0, 2000, 72 ) );

            DRN::CDRNGlobal::Instance()->GetSessionRegistry().Unregister( second.share->Client() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        second.Stop();
        first.Stop();
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
TestAddressesAndTwoMounts( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Late binding: a malformed address is still refused, the id form still needs a registered connection, and two mounts of one name follow the same session" )
        const CORE::CString sessionName = UniqueSessionName( "addresses" );
        CServedDirectory served( "addresses", sessionName );
        try
        {
            REQUIRE_TRUE( WriteSeeded( served.Path( "a.bin" ), 100, 31 ) );
            CBridgeMountOptions options;

            // refused whether the session exists or not
            const char* const badSources[] = { "drn://%S%/sequencer", "drn://%S%/sequencer/files/more", "drn://%S%//files", "drn:///sequencer/files", "drn://", "drn:",
                                               "no connection|files", "nothing at all", "unregistered id|files", "|files", "" };
            for ( UInt32 round=0; round<2; ++round )
            {
                if ( 1 == round )
                    REQUIRE_TRUE( served.Start( true ) );
                for ( UInt32 i=0; i<sizeof( badSources ) / sizeof( badSources[ 0 ] ); ++i )
                {
                    CORE::CString source = CORE::CString( badSources[ i ] ).ReplaceSubstr( "%S%", sessionName );
                    VFS::CVFS::TArchivePtr archive = CreateBridgeArchive( "Bad", source, options );
                    if ( !archive.IsNULL() )
                    {
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DrnBridgeTest: the source \"" << source << "\" was mounted but should not have been" );
                    }
                    ASSERT_TRUE( archive.IsNULL() );
                }

                // a share name that is none is refused as well, as before
                ASSERT_TRUE( CreateBridgeArchive( "Bad", "drn://" + sessionName + "/sequencer/not a share", options ).IsNULL() );
            }
            served.Stop();

            // the address of the mount is the name of the session: two mounts follow the one session that appears, and neither has to be made again
            VFS::CVFS::TArchivePtr first = CreateBridgeArchive( "First", "drn://" + sessionName + "/sequencer/files", options );
            VFS::CVFS::TArchivePtr second = CreateBridgeArchive( "Second", "drn://" + sessionName + "/sequencer/files", options );
            REQUIRE_TRUE( !first.IsNULL() && !second.IsNULL() );
            ASSERT_FALSE( first->IsConnected() || second->IsConnected() );
            REQUIRE_TRUE( served.Start( true ) );
            ASSERT_TRUE( BecomesConnected( *first, 10000 ) );
            ASSERT_TRUE( BecomesConnected( *second, 10000 ) );
            ASSERT_TRUE( 100 == first->GetFileSize( "a.bin" ) && 100 == second->GetFileSize( "a.bin" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        served.Stop();
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
TestMounter( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Mounter: mounts a share of a session by name at a mount point, tells its state through link loss and reconnect, and unmounts in every way" )
        const CORE::CString sessionName = UniqueSessionName( "mounter" );
        CServedDirectory served( "mounter", sessionName );
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        try
        {
            REQUIRE_TRUE( WriteSeeded( served.Path( "a.bin" ), 2000, 41 ) );
            REQUIRE_TRUE( WriteSeeded( served.Path( "sub/b.bin" ), 3000, 42 ) );

            // mounted before the session exists: mounted, not reachable
            {
                CDrnBridgeMounter mounter( vfs );
                ASSERT_FALSE( mounter.IsMounted() );
                ASSERT_TRUE( CDrnBridgeMounter::STATE_NOT_MOUNTED == mounter.GetState() );
                ASSERT_FALSE( mounter.Unmount() );

                CDrnBridgeMountResult result = mounter.Mount( sessionName, "sequencer", "files", "mounter/point" );
                ASSERT_TRUE( result.mounted );
                ASSERT_TRUE( result.reason.IsNULLOrEmpty() );
                ASSERT_TRUE( result.mountPoint == "mounter/point" );
                ASSERT_FALSE( result.archiveName.IsNULLOrEmpty() );
                REQUIRE_TRUE( result.mounted );
                ASSERT_TRUE( mounter.IsMounted() );
                ASSERT_TRUE( vfs.IsMountedArchive( "mounter/point" ) );
                ASSERT_TRUE( mounter.GetAddress() == "drn://" + sessionName + "/sequencer/files" );
                ASSERT_TRUE( CDrnBridgeMounter::STATE_OFFLINE == mounter.GetState() );
                ASSERT_FALSE( vfs.FileExists( "mounter/point/a.bin" ) );

                // the mounter holds one mount: another is refused, and the first is not touched
                CDrnBridgeMountResult again = mounter.Mount( sessionName, "sequencer", "files", "mounter/other" );
                ASSERT_FALSE( again.mounted );
                ASSERT_FALSE( again.reason.IsNULLOrEmpty() );
                ASSERT_FALSE( vfs.IsMountedArchive( "mounter/other" ) );
                ASSERT_TRUE( mounter.IsMounted() );

                // the session appears: online, and the files are readable through the VFS
                REQUIRE_TRUE( served.Start( true ) );
                ASSERT_TRUE( StateBecomes( mounter, CDrnBridgeMounter::STATE_ONLINE, 10000 ) );
                ASSERT_TRUE( vfs.FileExists( "mounter/point/a.bin" ) );
                ASSERT_TRUE( 3000 == vfs.GetFileSize( "mounter/point/sub/b.bin" ) );
                VFS::TBasicVfsResourcePtr file = vfs.GetFile( "mounter/point/a.bin", "rb", false );
                REQUIRE_TRUE( !file.IsNULL() );
                ASSERT_TRUE( ReadSeeded( *file->GetAccess(), 0, 2000, 41 ) );

                // the link is lost: offline; it is back: online
                served.share->DropClient();
                ASSERT_TRUE( StateBecomes( mounter, CDrnBridgeMounter::STATE_OFFLINE, 10000 ) );
                ASSERT_TRUE( mounter.IsMounted() );
                REQUIRE_TRUE( served.share->RejoinClient() );
                ASSERT_TRUE( StateBecomes( mounter, CDrnBridgeMounter::STATE_ONLINE, 10000 ) );
                ASSERT_TRUE( ReadSeeded( *file->GetAccess(), 1000, 1000, 41 ) );

                // unmounting: the mount point is free, the answers are gone, and a second Unmount() does no harm
                ASSERT_TRUE( mounter.Unmount() );
                ASSERT_FALSE( mounter.IsMounted() );
                ASSERT_TRUE( CDrnBridgeMounter::STATE_NOT_MOUNTED == mounter.GetState() );
                ASSERT_FALSE( vfs.IsMountedArchive( "mounter/point" ) );
                ASSERT_FALSE( mounter.Unmount() );
                ASSERT_FALSE( vfs.FileExists( "mounter/point/a.bin" ) );

                // it mounts again
                ASSERT_TRUE( mounter.Mount( sessionName, "sequencer", "files", "mounter/point" ).mounted );
                ASSERT_TRUE( StateBecomes( mounter, CDrnBridgeMounter::STATE_ONLINE, 10000 ) );
            }

            // the destructor unmounted what the mounter mounted
            ASSERT_FALSE( vfs.IsMountedArchive( "mounter/point" ) );
            ASSERT_FALSE( vfs.FileExists( "mounter/point/a.bin" ) );
            served.Stop();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        served.Stop();
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
TestMounterRefusals( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Mounter: refuses with a reason what cannot be mounted (an address that is none, a mount point or a name in use, an unknown type), and unmounts only what it mounted" )
        const CORE::CString sessionName = UniqueSessionName( "refusals" );
        CServedDirectory served( "refusals", sessionName );
        CServedDirectory other( "refusalsother", UniqueSessionName( "refusalsother" ) );
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        try
        {
            REQUIRE_TRUE( WriteSeeded( served.Path( "a.bin" ), 100, 51 ) );
            REQUIRE_TRUE( WriteSeeded( other.Path( "o.bin" ), 100, 52 ) );
            CDrnBridgeMounter mounter( vfs );

            // addresses that are none
            const CDrnBridgeMountResult noSession = mounter.Mount( "", "sequencer", "files", "refusals/a" );
            ASSERT_FALSE( noSession.mounted );
            ASSERT_FALSE( noSession.reason.IsNULLOrEmpty() );
            ASSERT_FALSE( mounter.Mount( sessionName, "", "files", "refusals/a" ).mounted );
            ASSERT_FALSE( mounter.Mount( sessionName, "a/b", "files", "refusals/a" ).mounted );
            ASSERT_FALSE( mounter.Mount( "x/y", "sequencer", "files", "refusals/a" ).mounted );
            ASSERT_FALSE( mounter.Mount( sessionName, "sequencer", "", "refusals/a" ).mounted );
            ASSERT_FALSE( mounter.Mount( sessionName, "sequencer", "not a share", "refusals/a" ).mounted );
            ASSERT_FALSE( mounter.Mount( sessionName, "sequencer", "a/b", "refusals/a" ).mounted );
            ASSERT_FALSE( vfs.IsMountedArchive( "refusals/a" ) );
            ASSERT_FALSE( mounter.IsMounted() );

            // mount points that are none
            ASSERT_FALSE( mounter.Mount( sessionName, "sequencer", "files", "" ).mounted );
            ASSERT_FALSE( mounter.Mount( sessionName, "sequencer", "files", "/" ).mounted );

            // a mount point in use by an archive that was there: refused and the archive is untouched
            const CORE::CString localDir = BedDirectory( "refusalslocal" );
            REQUIRE_TRUE( WriteSeeded( CORE::CombinePath( localDir, "local.bin" ), 10, 53 ) );
            REQUIRE_TRUE( vfs.AddRoot( localDir, "refusalsLocalArchive", false, false, "refusals/busy" ) );
            const CDrnBridgeMountResult busy = mounter.Mount( sessionName, "sequencer", "files", "refusals/busy" );
            ASSERT_FALSE( busy.mounted );
            ASSERT_TRUE( busy.reason.HasSubstr( "in use" ) >= 0 );
            ASSERT_FALSE( mounter.IsMounted() );
            ASSERT_TRUE( vfs.FileExists( "refusals/busy/local.bin" ) );
            ASSERT_TRUE( vfs.IsMountedArchive( "refusals/busy" ) );

            // a name in use: refused
            const CDrnBridgeMountResult named = mounter.Mount( sessionName, "sequencer", "files", "refusals/named", GUCEF_NULL, "refusalsLocalArchive" );
            ASSERT_FALSE( named.mounted );
            ASSERT_TRUE( named.reason.HasSubstr( "refusalsLocalArchive" ) >= 0 );
            ASSERT_FALSE( vfs.IsMountedArchive( "refusals/named" ) );

            // the settings are the archive's: handed on
            CDrnBridgeSettings settings;
            settings.maxOpenHandles = 3;
            settings.metadataTtlMs = 777;
            settings.verifyMode = DRN::CDRNResourceTransferOptions::VERIFY_TREE;
            const CDrnBridgeMountResult first = mounter.Mount( sessionName, "sequencer", "files", "refusals/first", &settings, "refusalsFirst" );
            REQUIRE_TRUE( first.mounted );
            ASSERT_TRUE( first.archiveName == "refusalsFirst" );
            {
                VFS::CVFS::TArchivePtr archive = vfs.GetMountedArchive( "refusalsFirst" );
                REQUIRE_TRUE( !archive.IsNULL() );
                CDrnBridgeArchive* bridge = static_cast< CDrnBridgeArchive* >( archive.GetPointerAlways() );
                ASSERT_TRUE( 3 == bridge->GetSettings().maxOpenHandles );
                ASSERT_TRUE( 777 == bridge->GetSettings().metadataTtlMs );
                ASSERT_TRUE( DRN::CDRNResourceTransferOptions::VERIFY_TREE == bridge->GetSettings().verifyMode );
                ASSERT_TRUE( bridge->GetShare() == "files" );
            }

            // the mount point of another mounter is in use too
            {
                CDrnBridgeMounter second( vfs );
                const CDrnBridgeMountResult overlay = second.Mount( other.sessionName, "sequencer", "files", "refusals/first" );
                ASSERT_FALSE( overlay.mounted );
                ASSERT_TRUE( overlay.reason.HasSubstr( "in use" ) >= 0 );
                ASSERT_FALSE( second.IsMounted() );
                ASSERT_FALSE( second.Unmount() );

                // ...and what it did not mount is not unmounted by it, nor by its destructor
                ASSERT_TRUE( vfs.IsMountedArchive( "refusals/first" ) );
            }
            ASSERT_TRUE( vfs.IsMountedArchive( "refusals/first" ) );
            ASSERT_TRUE( mounter.IsMounted() );

            // the archive is unmounted by someone else: the mounter does not have it any more and does not take another one of the name away
            ASSERT_TRUE( vfs.UnmountArchiveByName( "refusalsFirst" ) );
            ASSERT_FALSE( mounter.IsMounted() );
            ASSERT_TRUE( CDrnBridgeMounter::STATE_NOT_MOUNTED == mounter.GetState() );
            ASSERT_TRUE( vfs.AddRoot( localDir, "refusalsFirst", false, false, "refusals/taken" ) );
            ASSERT_FALSE( mounter.Unmount() );
            ASSERT_TRUE( vfs.IsMountedArchive( "refusals/taken" ) );
            ASSERT_TRUE( vfs.FileExists( "refusals/taken/local.bin" ) );
            ASSERT_TRUE( vfs.UnmountArchiveByName( "refusalsFirst" ) );

            // the default archive name is its own: two mounters, two mount points
            {
                CDrnBridgeMounter a( vfs );
                CDrnBridgeMounter b( vfs );
                ASSERT_TRUE( a.Mount( sessionName, "sequencer", "files", "refusals/da" ).mounted );
                ASSERT_TRUE( b.Mount( other.sessionName, "sequencer", "files", "refusals/db" ).mounted );
                ASSERT_TRUE( a.GetArchiveName() != b.GetArchiveName() );
                ASSERT_TRUE( a.Unmount() );
                ASSERT_TRUE( b.IsMounted() );
                ASSERT_TRUE( vfs.IsMountedArchive( "refusals/db" ) );
            }
            ASSERT_FALSE( vfs.IsMountedArchive( "refusals/db" ) );

            // an archive type that the VFS does not know: the plugin is not loaded
            CDrnBridgeGlobal::Deinstance();
            const CDrnBridgeMountResult unknown = mounter.Mount( sessionName, "sequencer", "files", "refusals/unknown" );
            CDrnBridgeGlobal::Instance();
            ASSERT_FALSE( unknown.mounted );
            ASSERT_TRUE( unknown.reason.HasSubstr( "plugin is not loaded" ) >= 0 );
            ASSERT_FALSE( vfs.IsMountedArchive( "refusals/unknown" ) );
            ASSERT_TRUE( mounter.Mount( sessionName, "sequencer", "files", "refusals/known" ).mounted );
            ASSERT_TRUE( mounter.Unmount() );

            vfs.UnmountArchiveByName( "refusalsLocalArchive" );
            CORE::DeleteDir( localDir, true );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        served.Stop();
    GUCEF_TESTFW_TESTCASE_END
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------*/

void
PerformDrnBridgeMountingTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "vfspluginDRN.Mounting" );

    // the archive type is made known to the VFS by the global of the plugin
    CDrnBridgeGlobal::Instance();

    TestLateBinding();
    TestReplacedSession();
    TestFollowsTheRegistry();
    TestAddressesAndTwoMounts();
    TestMounter();
    TestMounterRefusals();
}

/*-------------------------------------------------------------------------*/
