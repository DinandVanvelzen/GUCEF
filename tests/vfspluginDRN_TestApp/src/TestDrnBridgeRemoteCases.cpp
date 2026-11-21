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

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_DRN_H
#include "gucefDRN.h"
#define GUCEF_DRN_H
#endif /* GUCEF_DRN_H ? */

#ifndef GUCEF_DRN_CDRNCLOCK_H
#include "gucefDRN_CDRNClock.h"
#define GUCEF_DRN_CDRNCLOCK_H
#endif /* GUCEF_DRN_CDRNCLOCK_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "vfspluginDRN_CDrnBridgeGlobal.h"
#include "vfspluginDRN_CDrnBridgeMounter.h"

#include "TestDrnBridgeFixture.h"
#include "TestDrnRemoteEnv.h"
#include "TestDrnBridgeRemoteCases.h"

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

/** A precondition: when it does not hold the case ends here, so that the lines after it do not meet what it should have made */
#define REQUIRE(t)      do { if ( !(t) ) { GUCEF_TESTFW_ASSERT_TRUE(t); throw std::exception(); } } while (0)

namespace {

typedef CDrnRemoteEnv E;

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
    return DRN::CDRNClock::NowInMilliseconds();
}

VFS::CVfsCachedIOAccess*
OpenAccess( E& env, const CORE::CString& relative, VFS::TBasicVfsResourcePtr& resource )
{
    resource = env.Vfs().GetFile( env.VfsPath( relative ) );
    if ( resource.IsNULL() )
        return GUCEF_NULL;
    return static_cast< VFS::CVfsCachedIOAccess* >( resource->GetAccess().GetPointerAlways() );
}

UInt32
ReadAt( VFS::CVfsCachedIOAccess* access, UInt64 offset, UInt32 count, CORE::CDynamicBuffer& buffer )
{
    buffer.SetDataSize( count );
    access->Setpos( offset );
    const UInt32 got = access->Read( buffer.GetBufferPtr(), 1, count );
    buffer.SetDataSize( got );
    return got;
}

bool
HasText( const CORE::CString& text, const CORE::CString& part )
{
    return text.HasSubstr( part ) >= 0;
}

bool
IsLowerHex( const CORE::CString& text )
{
    for ( UInt32 i=0; i<text.Length(); ++i )
    {
        const char c = text[ i ];
        if ( !( ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'f' ) ) )
            return false;
    }
    return true;
}

CORE::CString
UpperCase( const CORE::CString& text )
{
    CORE::CString upper;
    for ( UInt32 i=0; i<text.Length(); ++i )
    {
        char c = text[ i ];
        if ( c >= 'a' && c <= 'f' )
            c = (char) ( c - 'a' + 'A' );
        upper = upper + c;
    }
    return upper;
}

/** Mounts a second archive of the same share, as an application's configuration would */
bool
MountSecond( E& env, const CORE::CString& archiveName, const CORE::CString& mountPath, const CORE::CString& cacheFolder, const CORE::CString& identity, VFS::CVFS::TArchivePtr& archive )
{
    const CORE::CString source = "drn://" + env.GetSessionName() + "/sequencer/" + CLoopbackShare::ShareName;
    CDrnBridgeSettings bridge;
    bridge.metadataTtlMs = 60000;
    bridge.negativeTtlMs = 60000;
    bridge.blockCacheFolder = cacheFolder;
    bridge.blockCacheBlockBytes = B;
    bridge.cacheIdentity = identity;
    bridge.fetchThreadPool = "VfsRemoteOpsTest";
    CORE::CDataNode config;
    config.SetAttribute( "archiveName", archiveName );
    config.SetAttribute( "archiveType", CORE::CString( "drn" ) );
    config.SetAttribute( "actualArchivePath", source );
    config.SetAttribute( "path", source );
    config.SetAttribute( "mountPath", mountPath );
    config.SetAttribute( "writeable", false );
    bridge.SaveToArchiveConfig( config );
    VFS::CArchiveSettings settings;
    settings.LoadConfig( config );
    settings.SetActualArchivePath( source );
    if ( !env.Vfs().MountArchive( settings ) )
        return false;
    archive = env.Vfs().GetMountedArchive( archiveName );
    return !archive.IsNULL();
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDrnBridgeRemoteCases( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "DrnBridgeRemoteCases" );

    GUCEF_TESTFW_TESTCASE( "Settings: the keys of the block cache, the reading and the background fetches are written to an archive configuration and read back unchanged, have the defaults of the README, are brought to what can work, and reach the archive and its scheduler" )
    try
    {
        CDrnBridgeSettings defaults;
        ASSERT_TRUE( defaults.blockCacheFolder.IsNULLOrEmpty() && 0 == defaults.blockCacheMaxBytes && 0 == defaults.blockCacheBlockBytes && defaults.cacheIdentity.IsNULLOrEmpty() );
        ASSERT_TRUE( 2 == defaults.readAheadAfterBlocks && 8 == defaults.readAheadBlocks && 16 == defaults.maxBlocksPerFetch );
        ASSERT_TRUE( 8 == defaults.maxBackgroundFetches && 256 == defaults.maxQueuedFetches && "VfsRemoteOps" == defaults.fetchThreadPool );
        ASSERT_TRUE( defaults.verifyCompleteFiles && defaults.offlineListings && !defaults.openRequiresCompleteFile );
        ASSERT_TRUE( defaults.refetchPinned );

        CDrnBridgeSettings changed;
        changed.blockCacheFolder = "/some/folder";
        changed.blockCacheMaxBytes = 5368709120ULL;
        changed.blockCacheBlockBytes = 131072;
        changed.cacheIdentity = "the-share";
        changed.readAheadAfterBlocks = 3;
        changed.readAheadBlocks = 5;
        changed.maxBlocksPerFetch = 7;
        changed.maxBackgroundFetches = 3;
        changed.maxQueuedFetches = 11;
        changed.fetchThreadPool = "OtherPool";
        changed.verifyCompleteFiles = false;
        changed.offlineListings = false;
        changed.openRequiresCompleteFile = true;
        changed.refetchPinned = false;
        CORE::CDataNode config;
        changed.SaveToArchiveConfig( config );
        VFS::CArchiveSettings archiveSettings;
        archiveSettings.LoadConfig( config );
        CDrnBridgeSettings back;
        ASSERT_TRUE( back.LoadFromArchiveSettings( archiveSettings ) );
        ASSERT_TRUE( back.blockCacheFolder == changed.blockCacheFolder && back.blockCacheMaxBytes == changed.blockCacheMaxBytes && back.blockCacheBlockBytes == changed.blockCacheBlockBytes );
        ASSERT_TRUE( back.cacheIdentity == changed.cacheIdentity && 3 == back.readAheadAfterBlocks && 5 == back.readAheadBlocks && 7 == back.maxBlocksPerFetch );
        ASSERT_TRUE( 3 == back.maxBackgroundFetches && 11 == back.maxQueuedFetches && "OtherPool" == back.fetchThreadPool );
        ASSERT_FALSE( back.verifyCompleteFiles );
        ASSERT_FALSE( back.offlineListings );
        ASSERT_TRUE( back.openRequiresCompleteFile );
        ASSERT_FALSE( back.refetchPinned );
        // the old keys are written as before
        ASSERT_TRUE( back.metadataTtlMs == changed.metadataTtlMs && back.maxOpenHandles == changed.maxOpenHandles );

        // what cannot work is brought to what can
        CDrnBridgeSettings none;
        none.maxBlocksPerFetch = 0;
        none.maxBackgroundFetches = 0;
        none.fetchThreadPool.Clear();
        none.Normalize();
        ASSERT_TRUE( 1 == none.maxBlocksPerFetch && 1 == none.maxBackgroundFetches && "VfsRemoteOps" == none.fetchThreadPool );

        // what an archive made from them is: the scheduler, the cache and the identity
        E env( "settings", false );
        env.AddBridgeSetting( "maxQueuedFetches", "9" );
        env.AddBridgeSetting( "cacheIdentity", "settings-identity" );
        SRemoteEnvOptions options;
        options.maxRunning = 3;
        options.cacheMaxBytes = 7 * 1024 * 1024;
        REQUIRE( env.Setup( options ) );
        CDrnBridgeArchive* bridge = env.GetBridge();
        REQUIRE( GUCEF_NULL != bridge );
        ASSERT_TRUE( bridge->GetCacheIdentity() == "settings-identity" );
        ASSERT_TRUE( 3 == bridge->GetFetchScheduler()->GetMaxRunning() && 9 == bridge->GetFetchScheduler()->GetMaxQueued() );
        ASSERT_TRUE( "VfsRemoteOpsTest" == bridge->GetFetchScheduler()->GetThreadPoolName() );
        VFS::CVfsBlockCachePtr cache = bridge->GetBlockCache();
        REQUIRE( !cache.IsNULL() );
        ASSERT_TRUE( 7 * 1024 * 1024 == cache->GetMaxBytes() && B == cache->GetBlockBytes() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Identity: the share is known in the block cache by its address unless the settings name it, two mounts with one identity share what the cache holds and two identities do not" )
    try
    {
        E env( "identity", false );
        env.AddBridgeSetting( "cacheIdentity", "" );
        SRemoteEnvOptions options;
        options.metadataTtlMs = 60000;
        REQUIRE( env.Setup( options ) );
        CDrnBridgeArchive* bridge = env.GetBridge();
        REQUIRE( GUCEF_NULL != bridge );
        ASSERT_TRUE( bridge->GetCacheIdentity() == "drn://" + env.GetSessionName() + "/sequencer/" + CLoopbackShare::ShareName );

        // a connection that is registered by id: the identity is that form of the address
        CDrnBridgeConnectionPtr connection = CDrnBridgeGlobal::Instance()->RegisterConnection( "idform", env.Client(), env.GetServerPeerId() );
        REQUIRE( !connection.IsNULL() );
        {
            VFS::CVFS::TArchivePtr byId = env.Vfs().CreateArchive( "drn" );
            REQUIRE( !byId.IsNULL() );
            CDrnBridgeSettings idSettings;
            idSettings.blockCacheFolder = env.GetCacheFolder();
            idSettings.blockCacheBlockBytes = B;
            CORE::CDataNode config;
            config.SetAttribute( "archiveName", CORE::CString( "byid" ) );
            config.SetAttribute( "archiveType", CORE::CString( "drn" ) );
            config.SetAttribute( "actualArchivePath", CORE::CString( "idform|files" ) );
            config.SetAttribute( "path", CORE::CString( "idform|files" ) );
            idSettings.SaveToArchiveConfig( config );
            VFS::CArchiveSettings settings;
            settings.LoadConfig( config );
            settings.SetActualArchivePath( "idform|files" );
            REQUIRE( byId->LoadArchive( settings ) );
            ASSERT_TRUE( static_cast< CDrnBridgeArchive* >( byId.GetPointerAlways() )->GetCacheIdentity() == "idform|files" );
            byId->UnloadArchive();
        }
        CDrnBridgeGlobal::Instance()->UnregisterConnection( "idform" );

        // one identity, one cache folder: a second mount holds what the first one fetched
        CORE::CDynamicBuffer content;
        ASSERT_TRUE( env.ReadAll( CRemoteEnv::MidFile, content ) && env.Matches( CRemoteEnv::MidFile, 0, content, 300000 ) );
        const CORE::CString sameIdentity = bridge->GetCacheIdentity();
        VFS::CVFS::TArchivePtr second;
        REQUIRE( MountSecond( env, "second", "/second/", env.GetCacheFolder(), sameIdentity, second ) );
        VFS::CVfsCacheState state;
        ASSERT_TRUE( env.Vfs().FileExists( "/second/data/mid.bin" ) );
        ASSERT_TRUE( env.Vfs().GetFileCacheState( "/second/data/mid.bin", state ) && state.cached && state.complete );
        CDrnBridgeArchive* secondBridge = static_cast< CDrnBridgeArchive* >( second.GetPointerAlways() );
        ASSERT_TRUE( secondBridge->GetBlockCache().GetPointerAlways() == bridge->GetBlockCache().GetPointerAlways() );
        {
            VFS::TBasicVfsResourcePtr resource = env.Vfs().GetFile( "/second/data/mid.bin" );
            REQUIRE( !resource.IsNULL() );
            CORE::CDynamicBuffer buffer( 300000, true );
            const UInt32 got = resource->GetAccess()->Read( buffer.GetBufferPtr(), 1, 300000 );
            buffer.SetDataSize( got );
            ASSERT_TRUE( 300000 == got );
            ASSERT_TRUE( env.Matches( CRemoteEnv::MidFile, 0, buffer, 300000 ) );
            ASSERT_TRUE( 0 == secondBridge->GetFetchCallCount() );
        }
        env.Vfs().UnmountArchiveByName( "second" );

        // another identity: the same files are not what this mount holds
        VFS::CVFS::TArchivePtr third;
        REQUIRE( MountSecond( env, "third", "/third/", env.GetCacheFolder(), "another-identity", third ) );
        ASSERT_TRUE( env.Vfs().FileExists( "/third/data/mid.bin" ) );
        ASSERT_FALSE( env.Vfs().GetFileCacheState( "/third/data/mid.bin", state ) && state.cached );
        env.Vfs().UnmountArchiveByName( "third" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Open without a handle: opening takes no handle, a handle is opened when a block is missing, a cached file is read with none, a handle closed for room is not opened again for what the cache holds, and an open is pinned at the revision of the metadata it was made from" )
    try
    {
        E env( "nohandle", false );
        env.AddBridgeSetting( "maxOpenHandles", "1" );
        SRemoteEnvOptions options;
        options.metadataTtlMs = 60000;
        REQUIRE( env.Setup( options ) );
        CDrnBridgeArchive* bridge = env.GetBridge();
        REQUIRE( GUCEF_NULL != bridge );

        // a file that is read whole is in the cache, and nothing holds a handle when the access that read it is gone
        CORE::CDynamicBuffer content;
        ASSERT_TRUE( env.ReadAll( CRemoteEnv::MidFile, content ) && env.Matches( CRemoteEnv::MidFile, 0, content, 300000 ) );
        ASSERT_TRUE( 0 == bridge->GetOpenHandleCount() );

        // a cached file: open and read, no handle opened at all
        const UInt32 opensBefore = env.GetOpenCount();
        {
            VFS::TBasicVfsResourcePtr resource = env.Vfs().GetFile( env.VfsPath( CRemoteEnv::MidFile ) );
            REQUIRE( !resource.IsNULL() );
            ASSERT_TRUE( 0 == bridge->GetOpenHandleCount() && opensBefore == env.GetOpenCount() );
            CORE::CDynamicBuffer buffer( 300000, true );
            const UInt32 got = resource->GetAccess()->Read( buffer.GetBufferPtr(), 1, 300000 );
            buffer.SetDataSize( got );
            ASSERT_TRUE( 300000 == got );
            ASSERT_TRUE( env.Matches( CRemoteEnv::MidFile, 0, buffer, 300000 ) );
            ASSERT_TRUE( 0 == bridge->GetOpenHandleCount() && opensBefore == env.GetOpenCount() );
        }

        // a file that is not cached: the open takes none, the first block that is missing opens one
        VFS::TBasicVfsResourcePtr big;
        VFS::CVfsCachedIOAccess* bigAccess = OpenAccess( env, CRemoteEnv::BigFile, big );
        REQUIRE( GUCEF_NULL != bigAccess );
        ASSERT_TRUE( 0 == bridge->GetOpenHandleCount() && opensBefore == env.GetOpenCount() );
        CORE::CDynamicBuffer buffer;
        ASSERT_TRUE( 1000000 == ReadAt( bigAccess, 0, 1000000, buffer ) && env.Matches( CRemoteEnv::BigFile, 0, buffer, 1000000 ) );
        ASSERT_TRUE( 1 == bridge->GetOpenHandleCount() && opensBefore + 1 == env.GetOpenCount() );

        // the one handle there is goes to another file; what the first file has in the cache is read without opening it again
        VFS::TBasicVfsResourcePtr notes;
        VFS::CVfsCachedIOAccess* notesAccess = OpenAccess( env, CRemoteEnv::NotesFile, notes );
        REQUIRE( GUCEF_NULL != notesAccess );
        ASSERT_TRUE( 1000 == ReadAt( notesAccess, 0, 1000, buffer ) );
        ASSERT_TRUE( bridge->GetHandleEvictionCount() >= 1 );
        const UInt64 reopensBefore = bridge->GetHandleReopenCount();
        const UInt32 fetchesBefore = env.GetFetchCount();
        ASSERT_TRUE( 5000 == ReadAt( bigAccess, 900000, 5000, buffer ) && env.Matches( CRemoteEnv::BigFile, 900000, buffer, 5000 ) );
        ASSERT_TRUE( reopensBefore == bridge->GetHandleReopenCount() && fetchesBefore == env.GetFetchCount() );
        big.Unlink();
        notes.Unlink();
        ASSERT_TRUE( 0 == bridge->GetOpenHandleCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Open without a handle: a file that changed after the metadata was taken is opened at the revision of that metadata, reads what the cache holds of it and fails as changed at the first block it does not hold; with no time to live the metadata is asked at every open" )
    try
    {
        {
            E env( "pinned", false );
            SRemoteEnvOptions options;
            options.metadataTtlMs = 60000;
            REQUIRE( env.Setup( options ) );
            ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( CRemoteEnv::MidFile ) ) );
            ASSERT_TRUE( 300000 == env.Vfs().GetFileSize( env.VfsPath( CRemoteEnv::MidFile ) ) );

            // the first block is in the cache at the old revision; then the file changes behind the archive's back
            {
                VFS::TBasicVfsResourcePtr resource;
                VFS::CVfsCachedIOAccess* access = OpenAccess( env, CRemoteEnv::MidFile, resource );
                REQUIRE( GUCEF_NULL != access );
                CORE::CDynamicBuffer first;
                ASSERT_TRUE( 1000 == ReadAt( access, 0, 1000, first ) );
                ASSERT_TRUE( env.Matches( CRemoteEnv::MidFile, 0, first, 1000 ) );
            }
            const CORE::CString oldRevision = env.Backend().GetRevision( CRemoteEnv::MidFile );
            CORE::CDynamicBuffer oldBytes( 1000, true );
            REQUIRE( env.Backend().FillExpected( CRemoteEnv::MidFile, 0, oldBytes.GetBufferPtr(), 1000 ) );
            env.Backend().ChangeFile( CRemoteEnv::MidFile );
            ASSERT_TRUE( env.Backend().GetRevision( CRemoteEnv::MidFile ) != oldRevision );
            env.Events().Clear();

            // opened from the metadata that is not older than the time: the revision it had, and the bytes the cache holds of it
            VFS::TBasicVfsResourcePtr resource;
            VFS::CVfsCachedIOAccess* access = OpenAccess( env, CRemoteEnv::MidFile, resource );
            REQUIRE( GUCEF_NULL != access );
            ASSERT_TRUE( access->GetRevision() == oldRevision );
            CORE::CDynamicBuffer buffer;
            ASSERT_TRUE( 1000 == ReadAt( access, 0, 1000, buffer ) && 0 == memcmp( buffer.GetConstBufferPtr(), oldBytes.GetConstBufferPtr(), 1000 ) );

            // a block it does not hold is asked of the share at the revision the open was pinned at: the share says it is another one now (it refuses the open, it does not give a handle that is then given back)
            const UInt64 refusedBefore = env.Server().GetResources().GetStats().opensRefused;
            ASSERT_TRUE( 0 == ReadAt( access, 2 * B, 1000, buffer ) );
            ASSERT_TRUE( refusedBefore + 1 == env.Server().GetResources().GetStats().opensRefused );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_RESOURCE_CHANGED == access->GetLastResult().result );
            ASSERT_FALSE( access->IsValid() );
            REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::FileChangedEvent, CRemoteEnv::MidFile, 1, 4000 ) );

            // the next open learns the new revision and reads the new bytes
            VFS::TBasicVfsResourcePtr again;
            VFS::CVfsCachedIOAccess* fresh = OpenAccess( env, CRemoteEnv::MidFile, again );
            REQUIRE( GUCEF_NULL != fresh );
            ASSERT_TRUE( fresh->GetRevision() == env.Backend().GetRevision( CRemoteEnv::MidFile ) );
            ASSERT_TRUE( 1000 == ReadAt( fresh, 2 * B, 1000, buffer ) && env.Matches( CRemoteEnv::MidFile, 2 * B, buffer, 1000 ) );
        }

        {
            E env( "pinnedzero", false );
            SRemoteEnvOptions options;
            options.metadataTtlMs = 0;
            REQUIRE( env.Setup( options ) );
            ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( CRemoteEnv::MidFile ) ) );
            env.Backend().ChangeFile( CRemoteEnv::MidFile );
            VFS::TBasicVfsResourcePtr resource;
            VFS::CVfsCachedIOAccess* access = OpenAccess( env, CRemoteEnv::MidFile, resource );
            REQUIRE( GUCEF_NULL != access );
            ASSERT_TRUE( access->GetRevision() == env.Backend().GetRevision( CRemoteEnv::MidFile ) );
            CORE::CDynamicBuffer buffer;
            ASSERT_TRUE( 1000 == ReadAt( access, 2 * B, 1000, buffer ) && env.Matches( CRemoteEnv::MidFile, 2 * B, buffer, 1000 ) );
            ASSERT_TRUE( access->IsValid() && access->GetLastResult().IsSuccess() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Digests: the share's digest is a SHA-256 in lower case with its algorithm named, GetFileHash() stays upper case hexadecimal and empty when the share has none, a hint of the application is given back and does not become the hash, and InvalidateMetadata through the VFS sends the next call to the share, for a path and for every mount" )
    try
    {
        E env( "digests", false );
        SRemoteEnvOptions options;
        options.metadataTtlMs = 60000;
        REQUIRE( env.Setup( options ) );
        CDrnBridgeArchive* bridge = env.GetBridge();
        REQUIRE( GUCEF_NULL != bridge );
        env.Backend().ProvideDigest( CRemoteEnv::MidFile );

        VFS::CVfsDigest digest;
        ASSERT_TRUE( env.Vfs().GetFileDigest( env.VfsPath( CRemoteEnv::MidFile ), digest ) );
        ASSERT_TRUE( VFS::CVfsDigest::ALGORITHM_SHA256 == digest.algorithm && 64 == digest.value.Length() && IsLowerHex( digest.value ) );
        ASSERT_TRUE( digest.Matches( env.Backend().ComputeDigest( CRemoteEnv::MidFile, VFS::CVfsDigest::ALGORITHM_SHA256 ) ) );
        const CORE::CString hash = bridge->GetFileHash( CRemoteEnv::MidFile );
        ASSERT_TRUE( 64 == hash.Length() && UpperCase( digest.value ) == hash );

        // a file whose share has no digest: none, in either form
        VFS::CVfsDigest none;
        ASSERT_FALSE( env.Vfs().GetFileDigest( env.VfsPath( CRemoteEnv::TopFile ), none ) );
        ASSERT_TRUE( none.IsEmpty() && bridge->GetFileHash( CRemoteEnv::TopFile ).IsNULLOrEmpty() );

        // a hint is what the digest is for that file, the hash of the share is still none
        const VFS::CVfsDigest hint = env.Backend().ComputeDigest( CRemoteEnv::TopFile, VFS::CVfsDigest::ALGORITHM_SHA256 );
        env.Vfs().NoteFileDigest( env.VfsPath( CRemoteEnv::TopFile ), hint, env.Backend().GetRevision( CRemoteEnv::TopFile ) );
        VFS::CVfsDigest hinted;
        ASSERT_TRUE( env.Vfs().GetFileDigest( env.VfsPath( CRemoteEnv::TopFile ), hinted ) && hinted.Matches( hint ) );
        ASSERT_TRUE( bridge->GetFileHash( CRemoteEnv::TopFile ).IsNULLOrEmpty() );
        ASSERT_TRUE( bridge->GetFileDigest( CRemoteEnv::TopFile ).Matches( hint ) );

        // InvalidateMetadata through the VFS: the cache of the plugin goes too, so the next call is a stat of the share
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( CRemoteEnv::ReadmeFile ) ) );
        const UInt32 stats = env.Backend().GetStatCount();
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( CRemoteEnv::ReadmeFile ) ) );
        ASSERT_TRUE( stats == env.Backend().GetStatCount() );
        env.Vfs().InvalidateRemoteMetadata( env.VfsPath( CRemoteEnv::ReadmeFile ) );
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( CRemoteEnv::ReadmeFile ) ) );
        ASSERT_TRUE( stats + 1 == env.Backend().GetStatCount() );
        const UInt32 afterOne = env.Backend().GetStatCount();
        env.Vfs().InvalidateRemoteMetadata();
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( CRemoteEnv::ReadmeFile ) ) );
        ASSERT_TRUE( afterOne + 1 == env.Backend().GetStatCount() );
        // and through the archive itself
        bridge->InvalidateMetadata( CRemoteEnv::ReadmeFile );
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( CRemoteEnv::ReadmeFile ) ) );
        ASSERT_TRUE( afterOne + 2 == env.Backend().GetStatCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Statistics: the statistics of the VFS are filled from the counters of the bridge: calls to the share, handles and what the pool did, and from the cache" )
    try
    {
        E env( "statistics", false );
        env.AddBridgeSetting( "maxOpenHandles", "1" );
        SRemoteEnvOptions options;
        options.cacheMaxBytes = 6 * 1024 * 1024;
        REQUIRE( env.Setup( options ) );
        CDrnBridgeArchive* bridge = env.GetBridge();
        REQUIRE( GUCEF_NULL != bridge );

        VFS::TBasicVfsResourcePtr first;
        VFS::TBasicVfsResourcePtr second;
        VFS::CVfsCachedIOAccess* a = OpenAccess( env, CRemoteEnv::BigFile, first );
        VFS::CVfsCachedIOAccess* b = OpenAccess( env, CRemoteEnv::MidFile, second );
        REQUIRE( GUCEF_NULL != a && GUCEF_NULL != b );
        CORE::CDynamicBuffer buffer;
        ASSERT_TRUE( 1000 == ReadAt( a, 0, 1000, buffer ) );
        ASSERT_TRUE( 1000 == ReadAt( b, 0, 1000, buffer ) );
        ASSERT_TRUE( 1000 == ReadAt( a, 700000, 1000, buffer ) );

        VFS::CVfsRemoteStatistics statistics;
        REQUIRE( env.Vfs().GetRemoteStatistics( env.VfsPath( CRemoteEnv::ReadmeFile ), statistics ) );
        CDrnBridgeConnectionPtr connection = bridge->GetConnection();
        ASSERT_TRUE( statistics.statCalls == connection->GetStatCallCount() && statistics.statCalls >= 2 );
        ASSERT_TRUE( statistics.listCalls == connection->GetListCallCount() );
        ASSERT_TRUE( statistics.openCalls == connection->GetOpenCallCount() && statistics.openCalls >= 2 );
        ASSERT_TRUE( statistics.busyRetries == connection->GetBusyRetryCount() );
        ASSERT_TRUE( statistics.handlesOpen == bridge->GetOpenHandleCount() && 1 == statistics.handlesOpen );
        ASSERT_TRUE( statistics.handlesPeak == bridge->GetPeakOpenHandleCount() && 1 == statistics.handlesPeak );
        ASSERT_TRUE( statistics.handleEvictions == bridge->GetHandleEvictionCount() && statistics.handleEvictions >= 1 );
        ASSERT_TRUE( statistics.handleReopens == bridge->GetHandleReopenCount() );
        ASSERT_TRUE( statistics.bytesFetched > 0 && statistics.fetchCalls > 0 && statistics.bytesFetched == env.GetBytesServed() );
        ASSERT_TRUE( 6 * 1024 * 1024 == statistics.maxBytes && statistics.blocksStored > 0 && statistics.entries >= 2 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Link state: the state of the link is raised as an event once per change, on the thread that pumps the pulse generator, not again when it is asked for, for the member sequencer" )
    try
    {
        E env( "linkstate", false );
        REQUIRE( env.Setup() );
        CDrnBridgeArchive* bridge = env.GetBridge();
        REQUIRE( GUCEF_NULL != bridge );
        env.Events().Clear();
        ASSERT_TRUE( bridge->IsConnected() && bridge->IsConnected() );
        CRemoteEnv::Pump();
        ASSERT_TRUE( 0 == env.Events().Count( VFS::CRemoteArchiveEvents::ConnectionStateChangedEvent ) );

        env.Backend().SetLinkUp( false );
        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::ConnectionStateChangedEvent, CORE::CString::Empty, 1, 5000 ) );
        ASSERT_FALSE( bridge->IsConnected() );
        ASSERT_FALSE( bridge->IsConnected() );
        MT::ThreadDelay( 100 );
        CRemoteEnv::Pump();
        CTestEventRecorder::TRecordVector records = env.Events().Find( VFS::CRemoteArchiveEvents::ConnectionStateChangedEvent );
        ASSERT_TRUE( 1 == records.size() );
        ASSERT_TRUE( !records[ 0 ].connected && records[ 0 ].mountPath == "/remote/" && records[ 0 ].thread == MT::GetCurrentThreadID() );

        env.Backend().SetLinkUp( true );
        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::ConnectionStateChangedEvent, CORE::CString::Empty, 2, 5000 ) );
        ASSERT_TRUE( bridge->IsConnected() && bridge->IsConnected() );
        MT::ThreadDelay( 100 );
        CRemoteEnv::Pump();
        records = env.Events().Find( VFS::CRemoteArchiveEvents::ConnectionStateChangedEvent );
        ASSERT_TRUE( 2 == records.size() );
        ASSERT_TRUE( records[ 1 ].connected && records[ 1 ].thread == MT::GetCurrentThreadID() );
        // the next call works with no mount made again
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( CRemoteEnv::MidFile ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Thread that never waits, end to end: with a link that takes 400 ms per call every call of a marked thread returns within 100 ms, the events of what it asked for arrive on the pumping thread, and then the same calls are answered from what arrived, the bytes from the cache" )
    try
    {
        E env( "nowait", false );
        SRemoteEnvOptions options;
        options.metadataTtlMs = 60000;
        REQUIRE( env.Setup( options ) );
        env.Backend().SetDelayMs( 400 );

        UInt64 slowest = 0;
        const UInt64 begin = NowMs();
        {
            VFS::CVfsThreadPolicyScope marked( Marked() );
            UInt64 t = NowMs();
            ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( CRemoteEnv::MidFile ) ) );
            slowest = NowMs() - t;
            VFS::CVfsCallResult result;
            env.Vfs().GetLastResult( result );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE == result.result && result.fetchStarted );
            ASSERT_TRUE( slowest < 100 );

            // the result of the call in the form of the bridge: not yet available is not an outage, and the hash that is not known is not asked for
            CDrnBridgeArchive* bridge = env.GetBridge();
            REQUIRE( GUCEF_NULL != bridge );
            ASSERT_TRUE( bridge->GetLastResult().IsNotYetAvailable() && !bridge->GetLastResult().IsUnreachable() && CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE == bridge->GetLastResult().fileSystemResult );
            t = NowMs();
            ASSERT_TRUE( bridge->GetFileHash( CRemoteEnv::BigFile ).IsNULLOrEmpty() );
            ASSERT_TRUE( NowMs() - t < 100 );

            VFS::CVFS::TStringVector list;
            t = NowMs();
            ASSERT_FALSE( env.Vfs().GetFileList( list, env.VfsPath( "data" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX ) );
            ASSERT_TRUE( NowMs() - t < 100 );

            REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::MetadataArrivedEvent, CRemoteEnv::MidFile, 1, 5000 ) );
            REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::ListingArrivedEvent, "data/", 1, 5000 ) );

            t = NowMs();
            ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( CRemoteEnv::MidFile ) ) );
            VFS::TBasicVfsResourcePtr resource;
            VFS::CVfsCachedIOAccess* access = OpenAccess( env, CRemoteEnv::MidFile, resource );
            REQUIRE( GUCEF_NULL != access );
            CORE::CDynamicBuffer buffer;
            ASSERT_TRUE( 0 == ReadAt( access, 0, 1000, buffer ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE == access->GetLastResult().result );
            ASSERT_TRUE( NowMs() - t < 100 );

            REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::FileArrivedEvent, CRemoteEnv::MidFile, 1, 8000 ) );
            t = NowMs();
            ASSERT_TRUE( 1000 == ReadAt( access, 0, 1000, buffer ) && env.Matches( CRemoteEnv::MidFile, 0, buffer, 1000 ) );
            ASSERT_TRUE( access->GetLastResult().IsSuccess() );
            ASSERT_TRUE( NowMs() - t < 100 );
        }
        ASSERT_TRUE( NowMs() - begin >= 400 );
        ASSERT_TRUE( 0 == env.Events().Count( VFS::CRemoteArchiveEvents::FetchFailedEvent ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Single thread application: the client node has no thread of its own and the thread that makes the calls pumps it: a call that must not wait never blocks, the jobs on the pool are answered because that thread works the node, the events come by the same pulse, and a call that waits is served by the thread that pumps" )
    try
    {
        E env( "pumped", false );
        env.SetClientOwnThread( false );
        SRemoteEnvOptions options;
        options.metadataTtlMs = 60000;
        REQUIRE( env.Setup( options ) );
        REQUIRE( env.Client().GetNode().IsPumpThread() && !env.Client().GetNode().IsRunningOwnThread() );

        {
            VFS::CVfsThreadPolicyScope marked( Marked() );
            UInt64 t = NowMs();
            ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( CRemoteEnv::MidFile ) ) );
            ASSERT_TRUE( NowMs() - t < 100 );
            VFS::CVfsCallResult result;
            env.Vfs().GetLastResult( result );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE == result.result );

            // the answer of the share is delivered by the node, which this thread works while it waits for the event
            REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::MetadataArrivedEvent, CRemoteEnv::MidFile, 1, 10000 ) );
            ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( CRemoteEnv::MidFile ) ) );

            VFS::TBasicVfsResourcePtr resource;
            VFS::CVfsCachedIOAccess* access = OpenAccess( env, CRemoteEnv::MidFile, resource );
            REQUIRE( GUCEF_NULL != access );
            CORE::CDynamicBuffer buffer;
            t = NowMs();
            ASSERT_TRUE( 0 == ReadAt( access, 0, 1000, buffer ) );
            ASSERT_TRUE( NowMs() - t < 100 );
            REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::FileArrivedEvent, CRemoteEnv::MidFile, 1, 10000 ) );
            ASSERT_TRUE( 1000 == ReadAt( access, 0, 1000, buffer ) && env.Matches( CRemoteEnv::MidFile, 0, buffer, 1000 ) );
        }

        // this thread, waiting: it works the node itself while the call waits
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( CRemoteEnv::BigFile ) ) );
        CORE::CDynamicBuffer content;
        ASSERT_TRUE( env.ReadAll( CRemoteEnv::BigFile, content ) && env.Matches( CRemoteEnv::BigFile, 0, content, 1000000 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Failure injection (TCP through a fault proxy): a link cut in the middle of a run keeps the blocks that arrived whole, ends the read with the failure and the retry after the link is back fetches only what is missing; cut under a background job it ends with one failure event and gives its place back" )
    try
    {
        E env( "cutrun", true );
        SRemoteEnvOptions options;
        options.maxBlocksPerFetch = 4;
        options.readAheadBlocks = 0;
        REQUIRE( env.Setup( options ) );

        VFS::TBasicVfsResourcePtr resource;
        VFS::CVfsCachedIOAccess* access = OpenAccess( env, CRemoteEnv::BigFile, resource );
        REQUIRE( GUCEF_NULL != access );
        env.GetProvider()->CutNextReadsAfter( 2 * (UInt64) B + 500, true );
        CORE::CDynamicBuffer buffer;
        const UInt32 got = ReadAt( access, 0, 4 * B, buffer );
        ASSERT_TRUE( got < 4 * B && 0 == got % B && env.Matches( CRemoteEnv::BigFile, 0, buffer, got ) );
        ASSERT_FALSE( access->GetLastResult().IsSuccess() );
        VFS::CVfsCacheState state;
        ASSERT_TRUE( env.Vfs().GetFileCacheState( env.VfsPath( CRemoteEnv::BigFile ), state ) && got == state.bytesHeld );

        env.Backend().SetLinkUp( true );
        env.Backend().ResetCounters();
        ASSERT_TRUE( 4 * B - got == ReadAt( access, got, 4 * B - got, buffer ) && env.Matches( CRemoteEnv::BigFile, got, buffer, 4 * B - got ) );
        ASSERT_TRUE( 4 * B - got == env.Backend().GetBytesServed() );

        // under a background job: one failure event, the place of the job is free again, and the next job arrives
        VFS::TBasicVfsResourcePtr other;
        VFS::CVfsCachedIOAccess* marked = GUCEF_NULL;
        env.Events().Clear();
        {
            VFS::CVfsThreadPolicyScope scope( Marked() );
            ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( CRemoteEnv::MidFile ) ) || env.WaitForEvents( VFS::CRemoteArchiveEvents::MetadataArrivedEvent, CRemoteEnv::MidFile, 1, 5000 ) );
            REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::MetadataArrivedEvent, CRemoteEnv::MidFile, 0, 1000 ) );
            marked = OpenAccess( env, CRemoteEnv::MidFile, other );
            for ( UInt32 attempt=0; GUCEF_NULL == marked && attempt<50; ++attempt )
            {
                env.WaitForEvents( VFS::CRemoteArchiveEvents::MetadataArrivedEvent, CRemoteEnv::MidFile, 1, 100 );
                marked = OpenAccess( env, CRemoteEnv::MidFile, other );
            }
            REQUIRE( GUCEF_NULL != marked );
            env.GetProvider()->CutNextReadsAfter( 500, true );
            ASSERT_TRUE( 0 == ReadAt( marked, 0, 1000, buffer ) );
            REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::FetchFailedEvent, CRemoteEnv::MidFile, 1, 10000 ) );
            REQUIRE( env.WaitUntilIdle( 5000 ) );
            ASSERT_TRUE( 1 == env.Events().Count( VFS::CRemoteArchiveEvents::FetchFailedEvent ) );
            ASSERT_TRUE( 0 == env.Scheduler().GetRunningCount() );
        }
        env.Backend().SetLinkUp( true );
        {
            VFS::CVfsThreadPolicyScope scope( Marked() );
            env.Events().Clear();
            ReadAt( marked, 0, 1000, buffer );
            REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::FileArrivedEvent, CRemoteEnv::MidFile, 1, 10000 ) );
            ASSERT_TRUE( 1000 == ReadAt( marked, 0, 1000, buffer ) && env.Matches( CRemoteEnv::MidFile, 0, buffer, 1000 ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Failure injection: a session that is replaced by another of the same name keeps the mount, what the cache holds is read with no call to the share, the rest is fetched through the new session" )
    try
    {
        E env( "replaced", false );
        SRemoteEnvOptions options;
        options.metadataTtlMs = 60000;
        REQUIRE( env.Setup( options ) );
        CDrnBridgeArchive* bridge = env.GetBridge();
        CORE::CDynamicBuffer content;
        ASSERT_TRUE( env.ReadAll( CRemoteEnv::MidFile, content ) && env.Matches( CRemoteEnv::MidFile, 0, content, 300000 ) );

        REQUIRE( env.ReplaceClientSession() );
        bool connected = false;
        for ( UInt32 waited=0; waited<5000 && !connected; waited+=5 )
        {
            connected = bridge->IsConnected();
            if ( !connected )
                MT::ThreadDelay( 5 );
        }
        ASSERT_TRUE( connected );

        const UInt32 fetches = env.GetFetchCount();
        ASSERT_TRUE( env.ReadAll( CRemoteEnv::MidFile, content ) && env.Matches( CRemoteEnv::MidFile, 0, content, 300000 ) );
        ASSERT_TRUE( fetches == env.GetFetchCount() );
        ASSERT_TRUE( env.ReadAll( CRemoteEnv::BigFile, content ) && env.Matches( CRemoteEnv::BigFile, 0, content, 1000000 ) );
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( CRemoteEnv::NotesFile ) ) );
        ASSERT_TRUE( env.Vfs().IsMountedArchive( CRemoteEnv::MountPath ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Failure injection: a serving node that is restarted: a file that is open reads on at the revision it was opened at when the share is the same, and fails as changed when it is not" )
    try
    {
        E env( "restarted", false );
        SRemoteEnvOptions options;
        options.maxBlocksPerFetch = 4;
        options.readAheadBlocks = 0;
        REQUIRE( env.Setup( options ) );
        CDrnBridgeArchive* bridge = env.GetBridge();

        VFS::TBasicVfsResourcePtr resource;
        VFS::CVfsCachedIOAccess* access = OpenAccess( env, CRemoteEnv::BigFile, resource );
        REQUIRE( GUCEF_NULL != access );
        CORE::CDynamicBuffer buffer;
        ASSERT_TRUE( 2 * B == ReadAt( access, 0, 2 * B, buffer ) );

        REQUIRE( env.RestartServer() );
        bool connected = false;
        for ( UInt32 waited=0; waited<10000 && !connected; waited+=5 )
        {
            connected = bridge->IsConnected();
            if ( !connected )
                MT::ThreadDelay( 5 );
        }
        REQUIRE( connected );

        // the same files: the handle is opened again at the revision the file was pinned at
        ASSERT_TRUE( 2 * B == ReadAt( access, 800000, 2 * B, buffer ) && env.Matches( CRemoteEnv::BigFile, 800000, buffer, 2 * B ) );
        ASSERT_TRUE( access->IsValid() );
        ASSERT_TRUE( bridge->GetHandleReopenCount() + bridge->GetHandleEvictionCount() >= 0 );

        // a share that was changed while the node was away: what is open fails as changed at its next uncached read
        REQUIRE( env.RestartServer() );
        env.Backend().ChangeFile( CRemoteEnv::BigFile );
        connected = false;
        for ( UInt32 waited=0; waited<10000 && !connected; waited+=5 )
        {
            connected = bridge->IsConnected();
            if ( !connected )
                MT::ThreadDelay( 5 );
        }
        REQUIRE( connected );
        ASSERT_TRUE( 0 == ReadAt( access, 300000, B, buffer ) );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_RESOURCE_CHANGED == access->GetLastResult().result );
        ASSERT_FALSE( access->IsValid() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Failure injection: a limit of the serving side (BUSY) that the retries do not get past ends the read as retry-later, a background fetch ends with one failure event that says so and gives its place back, and the read works when there is room" )
    try
    {
        E env( "busylimit", false );
        env.AddBridgeSetting( "busyRetries", "2" );
        env.AddBridgeSetting( "busyBackoffMs", "10" );
        SRemoteEnvOptions options;
        options.metadataTtlMs = 60000;
        REQUIRE( env.Setup( options ) );
        CDrnBridgeArchive* bridge = env.GetBridge();

        // the first handle is opened while the limits are the defaults: the mount learns "32"
        VFS::TBasicVfsResourcePtr firstResource;
        VFS::CVfsCachedIOAccess* first = OpenAccess( env, CRemoteEnv::NotesFile, firstResource );
        REQUIRE( GUCEF_NULL != first );
        CORE::CDynamicBuffer buffer;
        ASSERT_TRUE( 1000 == ReadAt( first, 0, 1000, buffer ) );

        // then the limit drops to what is in use: this test holds one handle of the member's budget, the mount holds one
        DRN::CDRNResourceTransferSettings limits = env.Server().GetResources().GetSettings();
        limits.maxOpenHandlesPerPeer = 2;
        env.Server().GetResources().SetSettings( limits );
        DRN::CDRNResourceHandlePtr other = env.Client().GetResources().OpenHandle( env.GetServerPeerId(), CLoopbackShare::ShareName, CRemoteEnv::TopFile );
        REQUIRE( other->WaitOpen( 10000 ) );

        VFS::TBasicVfsResourcePtr secondResource;
        VFS::CVfsCachedIOAccess* second = OpenAccess( env, CRemoteEnv::BigFile, secondResource );
        REQUIRE( GUCEF_NULL != second );
        const UInt64 retriesBefore = bridge->GetConnection()->GetBusyRetryCount();
        ASSERT_TRUE( 0 == ReadAt( second, 0, 1000, buffer ) );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CAPLIMIT_RETRY_LATER == second->GetLastResult().result );
        ASSERT_TRUE( 2 == bridge->GetConnection()->GetBusyRetryCount() - retriesBefore );

        // in the background: one failure event, the retry-later result in it, the place free again
        env.Events().Clear();
        {
            VFS::CVfsThreadPolicyScope scope( Marked() );
            ASSERT_TRUE( 0 == ReadAt( second, 0, 1000, buffer ) );
            REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::FetchFailedEvent, CRemoteEnv::BigFile, 1, 10000 ) );
            REQUIRE( env.WaitUntilIdle( 5000 ) );
            CTestEventRecorder::TRecordVector failures = env.Events().Find( VFS::CRemoteArchiveEvents::FetchFailedEvent );
            ASSERT_TRUE( 1 == failures.size() && CORE::FILESYSTEMACCESS_REMOTE_CAPLIMIT_RETRY_LATER == failures[ 0 ].result );
            ASSERT_TRUE( 0 == env.Scheduler().GetRunningCount() );
        }

        // room is made: the same read works
        other->Close();
        MT::ThreadDelay( 100 );
        ASSERT_TRUE( 1000 == ReadAt( second, 0, 1000, buffer ) && env.Matches( CRemoteEnv::BigFile, 0, buffer, 1000 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Keep offline by path: a path that is fetched with a pin stays in a cache that evicts what is not pinned, is there when the link is down, and unpinning gives it up to eviction" )
    try
    {
        E env( "keepoffline", false );
        SRemoteEnvOptions options;
        options.cacheMaxBytes = 400 * 1024;
        options.metadataTtlMs = 60000;
        REQUIRE( env.Setup( options ) );

        VFS::CVfsFetchRequest request;
        request.pin = true;
        VFS::CVfsFetchProgress progress;
        ASSERT_TRUE( env.Vfs().FetchFile( env.VfsPath( CRemoteEnv::MidFile ), request, progress ).IsSuccess() );
        VFS::CVfsCacheState state;
        ASSERT_TRUE( env.Vfs().GetFileCacheState( env.VfsPath( CRemoteEnv::MidFile ), state ) && state.pinned && state.complete );

        // other files that do not fit next to it: what is not pinned is evicted, the pinned file is not
        CORE::CDynamicBuffer content;
        ASSERT_TRUE( env.ReadAll( CRemoteEnv::NotesFile, content ) && env.Matches( CRemoteEnv::NotesFile, 0, content, 70000 ) );
        ASSERT_TRUE( env.ReadAll( CRemoteEnv::TopFile, content ) );
        ASSERT_TRUE( env.ReadAll( CRemoteEnv::BigFile, content ) && env.Matches( CRemoteEnv::BigFile, 0, content, 1000000 ) );
        ASSERT_TRUE( env.ReadAll( CRemoteEnv::DeepFile, content ) );
        VFS::CVfsRemoteStatistics statistics;
        REQUIRE( env.Vfs().GetRemoteStatistics( env.VfsPath( CRemoteEnv::MidFile ), statistics ) );
        ASSERT_TRUE( statistics.evictedEntries >= 1 );
        ASSERT_FALSE( env.Vfs().GetFileCacheState( env.VfsPath( CRemoteEnv::NotesFile ), state ) && state.cached );
        ASSERT_TRUE( env.Vfs().GetFileCacheState( env.VfsPath( CRemoteEnv::MidFile ), state ) && state.pinned && state.complete );

        // the link is down: the pinned path opens and reads, with no call to the share
        env.Backend().SetLinkUp( false );
        env.Backend().ResetCounters();
        ASSERT_TRUE( env.ReadAll( CRemoteEnv::MidFile, content ) && env.Matches( CRemoteEnv::MidFile, 0, content, 300000 ) );
        ASSERT_TRUE( 0 == env.Backend().GetFetchCount() && 0 == env.Backend().GetOpenCount() );
        env.Backend().SetLinkUp( true );

        // unpinned: what the cache needs room for takes it
        ASSERT_TRUE( env.Vfs().SetFilePinned( env.VfsPath( CRemoteEnv::MidFile ), false ) );
        ASSERT_TRUE( env.Vfs().GetFileCacheState( env.VfsPath( CRemoteEnv::MidFile ), state ) && !state.pinned );
        ASSERT_TRUE( env.ReadAll( CRemoteEnv::BigFile, content ) );
        ASSERT_TRUE( env.ReadAll( CRemoteEnv::NotesFile, content ) );
        ASSERT_TRUE( env.ReadAll( CRemoteEnv::ReadmeFile, content ) );
        ASSERT_FALSE( env.Vfs().GetFileCacheState( env.VfsPath( CRemoteEnv::MidFile ), state ) && state.complete );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Keep offline by path, a pinned file that changed: the link goes before the new revision is held and the old complete copy is opened and read, flagged as an older revision, with no call to the share; with the link back the new revision is fetched without being asked, the pin moves to it and the old copy is no longer served" )
    try
    {
        E env( "pinsuperseded", false );
        SRemoteEnvOptions options;
        options.metadataTtlMs = 60000;
        REQUIRE( env.Setup( options ) );

        VFS::CVfsFetchRequest request;
        request.pin = true;
        VFS::CVfsFetchProgress progress;
        ASSERT_TRUE( env.Vfs().FetchFile( env.VfsPath( CRemoteEnv::MidFile ), request, progress ).IsSuccess() );
        VFS::CVfsCacheState state;
        ASSERT_TRUE( env.Vfs().GetFileCacheState( env.VfsPath( CRemoteEnv::MidFile ), state ) && state.pinned && state.complete );
        const CORE::CString oldRevision = state.revision;
        CORE::CDynamicBuffer oldContent;
        REQUIRE( env.ReadAll( CRemoteEnv::MidFile, oldContent ) && 300000 == oldContent.GetDataSize() );

        // the share changes and the archive learns of it; the fetch of the new revision that the archive starts by itself is cut
        env.Backend().CutNextFetchAfter( 1000 );
        env.Backend().ChangeFile( CRemoteEnv::MidFile );
        env.Vfs().InvalidateRemoteMetadata();
        ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( CRemoteEnv::MidFile ) ) );
        const CORE::CString newRevision = env.Backend().GetRevision( CRemoteEnv::MidFile );
        ASSERT_TRUE( newRevision != oldRevision );
        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::FetchFailedEvent, CRemoteEnv::MidFile, 1, 20000 ) );
        REQUIRE( env.WaitUntilIdle( 20000 ) );

        // the link goes: the old copy is the one that is there, and it says so
        env.Backend().SetLinkUp( false );
        env.Backend().ResetCounters();
        VFS::TBasicVfsResourcePtr offline;
        VFS::CVfsCachedIOAccess* access = OpenAccess( env, CRemoteEnv::MidFile, offline );
        REQUIRE( GUCEF_NULL != access );
        ASSERT_TRUE( access->IsStaleRevision() && access->GetRevision() == oldRevision );
        CORE::CDynamicBuffer buffer;
        ASSERT_TRUE( 300000 == ReadAt( access, 0, 300000, buffer ) && buffer.GetDataSize() == oldContent.GetDataSize() && 0 == memcmp( buffer.GetConstBufferPtr(), oldContent.GetConstBufferPtr(), 300000 ) );
        ASSERT_TRUE( env.Vfs().GetFileCacheState( env.VfsPath( CRemoteEnv::MidFile ), state ) && state.staleRevision && state.pinned && state.complete );
        VFS::CVfsRemoteStatistics statistics;
        REQUIRE( env.Vfs().GetRemoteStatistics( env.VfsPath( CRemoteEnv::MidFile ), statistics ) );
        ASSERT_TRUE( statistics.staleServes >= 1 && statistics.pinnedEntries >= 1 );
        ASSERT_TRUE( 0 == env.Backend().GetFetchCount() && 0 == env.Backend().GetOpenCount() );
        offline.Unlink();

        // the link is back: nobody asks, the new revision is fetched and takes the pin
        env.Backend().SetLinkUp( true );
        bool healed = false;
        for ( UInt32 waited=0; waited<30000 && !healed; waited+=5 )
        {
            E::Pump();
            env.Vfs().FileExists( env.VfsPath( CRemoteEnv::MidFile ) );
            healed = env.Vfs().GetFileCacheState( env.VfsPath( CRemoteEnv::MidFile ), state ) && state.pinned && state.complete && !state.staleRevision && state.revision == newRevision;
            if ( !healed )
                MT::PrecisionDelay( 5 );
        }
        REQUIRE( healed );
        REQUIRE( env.WaitUntilIdle( 20000 ) );
        CORE::CDynamicBuffer content;
        ASSERT_TRUE( env.ReadAll( CRemoteEnv::MidFile, content ) && env.Matches( CRemoteEnv::MidFile, 0, content, 300000 ) );
        env.Backend().SetLinkUp( false );
        VFS::TBasicVfsResourcePtr again;
        access = OpenAccess( env, CRemoteEnv::MidFile, again );
        REQUIRE( GUCEF_NULL != access );
        ASSERT_FALSE( access->IsStaleRevision() );
        ASSERT_TRUE( access->GetRevision() == newRevision && 300000 == ReadAt( access, 0, 300000, buffer ) && env.Matches( CRemoteEnv::MidFile, 0, buffer, 300000 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mounting helpers: the settings of the block cache and the identity pass through the mounter and through the attributes of the settings of a mount, the share is read through the cache, and the cache folder is the one of the settings" )
    try
    {
        E env( "mounterpass", false );
        SRemoteEnvOptions options;
        options.useCache = false;
        REQUIRE( env.Setup( options ) );
        ASSERT_TRUE( env.GetBridge()->GetBlockCache().IsNULL() );

        CBlockCacheTestFolder folder( "mounterpass-cache" );
        CDrnBridgeSettings settings;
        settings.blockCacheFolder = folder.GetPath();
        settings.blockCacheMaxBytes = 2 * 1024 * 1024;
        settings.blockCacheBlockBytes = B;
        settings.cacheIdentity = "mounter-identity";
        settings.fetchThreadPool = "VfsRemoteOpsTest";
        CDrnBridgeMounter mounter( env.Vfs() );
        const CDrnBridgeMountResult mounted = mounter.Mount( env.GetSessionName(), "sequencer", CLoopbackShare::ShareName, "/viamounter/", &settings, "viamounter" );
        REQUIRE( mounted.mounted );
        VFS::CVFS::TArchivePtr archive = env.Vfs().GetMountedArchive( "viamounter" );
        REQUIRE( !archive.IsNULL() );
        CDrnBridgeArchive* bridge = static_cast< CDrnBridgeArchive* >( archive.GetPointerAlways() );
        ASSERT_TRUE( bridge->GetCacheIdentity() == "mounter-identity" );
        VFS::CVfsBlockCachePtr cache = bridge->GetBlockCache();
        REQUIRE( !cache.IsNULL() );
        ASSERT_TRUE( 2 * 1024 * 1024 == cache->GetMaxBytes() && B == cache->GetBlockBytes() );

        VFS::TBasicVfsResourcePtr resource = env.Vfs().GetFile( "/viamounter/data/mid.bin" );
        REQUIRE( !resource.IsNULL() );
        CORE::CDynamicBuffer buffer( 300000, true );
        const UInt32 got = resource->GetAccess()->Read( buffer.GetBufferPtr(), 1, 300000 );
        buffer.SetDataSize( got );
        ASSERT_TRUE( 300000 == got && env.Matches( CRemoteEnv::MidFile, 0, buffer, 300000 ) );
        VFS::CVfsRemoteStatistics statistics;
        REQUIRE( env.Vfs().GetRemoteStatistics( "/viamounter/data/mid.bin", statistics ) );
        ASSERT_TRUE( statistics.blocksStored >= 5 && 2 * 1024 * 1024 == statistics.maxBytes );
        resource.Unlink();
        ASSERT_TRUE( mounter.Unmount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Setting openRequiresCompleteFile: a thread that must not wait opens a file only when every block is cached, else it is told it is not yet available and the whole file is fetched in the background" )
    try
    {
        E env( "completeonly", false );
        env.AddBridgeSetting( "openRequiresCompleteFile", "true" );
        SRemoteEnvOptions options;
        options.metadataTtlMs = 60000;
        REQUIRE( env.Setup( options ) );
        ASSERT_TRUE( env.GetBridge()->GetSettings().openRequiresCompleteFile );

        VFS::CVfsThreadPolicyScope marked( Marked() );
        ASSERT_FALSE( env.Vfs().FileExists( env.VfsPath( CRemoteEnv::MidFile ) ) );
        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::MetadataArrivedEvent, CRemoteEnv::MidFile, 1, 5000 ) );

        // known, not cached: no open, one fetch of the whole file
        VFS::TBasicVfsResourcePtr resource = env.Vfs().GetFile( env.VfsPath( CRemoteEnv::MidFile ) );
        ASSERT_TRUE( resource.IsNULL() );
        VFS::CVfsCallResult result;
        env.Vfs().GetLastResult( result );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE == result.result && result.fetchStarted );
        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::FileArrivedEvent, CRemoteEnv::MidFile, 1, 10000 ) );
        VFS::CVfsCacheState state;
        ASSERT_TRUE( env.Vfs().GetFileCacheState( env.VfsPath( CRemoteEnv::MidFile ), state ) && state.complete );

        // complete: it opens, and reads the bytes at once
        resource = env.Vfs().GetFile( env.VfsPath( CRemoteEnv::MidFile ) );
        REQUIRE( !resource.IsNULL() );
        CORE::CDynamicBuffer buffer( 300000, true );
        const UInt32 got = resource->GetAccess()->Read( buffer.GetBufferPtr(), 1, 300000 );
        buffer.SetDataSize( got );
        ASSERT_TRUE( 300000 == got && env.Matches( CRemoteEnv::MidFile, 0, buffer, 300000 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Without a block cache: a file that changed while it is read through the handles of the facility ends as changed, tells the generic layer (FileChanged is raised) and what the layer knew of the path is dropped, so the new size is seen at once" )
    try
    {
        E env( "legacychange", false );
        SRemoteEnvOptions options;
        options.useCache = false;
        options.metadataTtlMs = 60000;
        REQUIRE( env.Setup( options ) );
        CDrnBridgeArchive* bridge = env.GetBridge();
        ASSERT_TRUE( 1000000 == env.Vfs().GetFileSize( env.VfsPath( CRemoteEnv::BigFile ) ) );

        VFS::TBasicVfsResourcePtr resource = env.Vfs().GetFile( env.VfsPath( CRemoteEnv::BigFile ) );
        REQUIRE( !resource.IsNULL() );
        ASSERT_TRUE( 1 == bridge->GetOpenHandleCount() );
        CORE::CDynamicBuffer buffer( 1000, true );
        ASSERT_TRUE( 1000 == resource->GetAccess()->Read( buffer.GetBufferPtr(), 1, 1000 ) );

        env.Events().Clear();
        env.Backend().ChangeFileSize( CRemoteEnv::BigFile, 1010000 );
        resource->GetAccess()->Setpos( 800000 );
        ASSERT_TRUE( 0 == resource->GetAccess()->Read( buffer.GetBufferPtr(), 1, 1000 ) );
        ASSERT_FALSE( resource->GetAccess()->IsValid() );
        ASSERT_TRUE( bridge->GetLastResult().IsChanged() );
        REQUIRE( env.WaitForEvents( VFS::CRemoteArchiveEvents::FileChangedEvent, CRemoteEnv::BigFile, 1, 4000 ) );
        ASSERT_TRUE( 1010000 == env.Vfs().GetFileSize( env.VfsPath( CRemoteEnv::BigFile ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Setting serveStaleWhenOffline: with true what is known of a file is answered while the share cannot be reached, with false it is not (the files that the block cache holds are still there)" )
    try
    {
        for ( UInt32 round=0; round<2; ++round )
        {
            const bool serveStale = 0 == round;
            E env( serveStale ? "stale-on" : "stale-off", false );
            env.AddBridgeSetting( "serveStaleWhenOffline", serveStale ? "true" : "false" );
            SRemoteEnvOptions options;
            options.metadataTtlMs = 60000;
            REQUIRE( env.Setup( options ) );
            ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( CRemoteEnv::TopFile ) ) );
            CORE::CDynamicBuffer content;
            ASSERT_TRUE( env.ReadAll( CRemoteEnv::MidFile, content ) );

            env.Backend().SetLinkUp( false );
            VFS::CVfsCallResult result;
            ASSERT_TRUE( serveStale == env.Vfs().FileExists( env.VfsPath( CRemoteEnv::TopFile ) ) );
            env.Vfs().GetLastResult( result );
            ASSERT_TRUE( serveStale ? result.IsSuccess() : CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == result.result );
            ASSERT_TRUE( env.Vfs().FileExists( env.VfsPath( CRemoteEnv::MidFile ) ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

}

/*-------------------------------------------------------------------------*/
