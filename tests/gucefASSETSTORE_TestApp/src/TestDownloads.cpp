/*
 *  gucefASSETSTORE_TestApp: tests of the gucefASSETSTORE module
 *
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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */
#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */
#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#include "gucefASSETSTORE_CAssetStoreGlobal.h"
#define GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H
#include "gucefASSETSTORE_CAssetStoreRegistry.h"
#define GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADMANAGER_H
#include "gucefASSETSTORE_CAssetStoreDownloadManager.h"
#define GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADMANAGER_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADMANAGER_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTORESAFENAMES_H
#include "gucefASSETSTORE_CAssetStoreSafeNames.h"
#define GUCEF_ASSETSTORE_CASSETSTORESAFENAMES_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORESAFENAMES_H ? */
#ifndef GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H
#include "gucefASSETSTORE_CPrimaryAssetStore.h"
#define GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H
#endif /* GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H ? */

#ifndef TESTASSETSTORESUPPORT_H
#include "TestAssetStoreSupport.h"
#define TESTASSETSTORESUPPORT_H
#endif /* TESTASSETSTORESUPPORT_H ? */
#ifndef TESTFAKESTORESERVER_H
#include "TestFakeStoreServer.h"
#define TESTFAKESTORESERVER_H
#endif /* TESTFAKESTORESERVER_H ? */
#ifndef TESTFAKESTOREPROVIDER_H
#include "TestFakeStoreProvider.h"
#define TESTFAKESTOREPROVIDER_H
#endif /* TESTFAKESTOREPROVIDER_H ? */
#ifndef TESTDOWNLOADEVENTRECORDER_H
#include "TestDownloadEventRecorder.h"
#define TESTDOWNLOADEVENTRECORDER_H
#endif /* TESTDOWNLOADEVENTRECORDER_H ? */
#include "TestDownloads.h"

using namespace GUCEF;
using namespace GUCEF::CORE;
using namespace GUCEF::ASSETSTORE;
using namespace ASSETSTORETEST;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The fake store with its items, shared by the tests of this file */
struct SDownloadWorld
{
    CFakeStoreServer server;
    CFakeStoreProviderFactory factoryA;
    CFakeStoreProviderFactory factoryB;
    GUCEF::map< CString, CDynamicBuffer > contents;

    SDownloadWorld( void ) : server(), factoryA( "fakestore" ), factoryB( "fakestore2" ), contents() {}
};

CAssetStoreFile
AddFile( SDownloadWorld& world, CAssetStoreFormat& format, const CString& fileId, const CString& name, UInt32 size, const CString& digestAlgorithm )
{
    CDynamicBuffer content;
    MakePattern( size, (UInt32) fileId.Length() * 7919 + size, content );
    world.contents[ fileId ] = content;
    world.server.SetFileContent( fileId, content );
    CAssetStoreFile file;
    file.fileId = fileId;
    file.name = name;
    file.size = size;
    file.digestAlgorithm = digestAlgorithm;
    if ( digestAlgorithm == "sha1" )
        file.digest = Sha1Of( content );
    else if ( digestAlgorithm == "sha256" )
        file.digest = Sha256Of( content );
    format.files.push_back( file );
    return file;
}

CAssetStoreItem
MakeItem( const CString& itemId, const CString& title, const CString& listingType )
{
    CAssetStoreItem item;
    item.itemId = itemId;
    item.title = title;
    item.seller = "Test Seller";
    item.listingType = listingType;
    item.licenceName = "Standard License";
    item.categories.push_back( "3D/Weapons & Combat/Melee" );
    item.tags.push_back( "sword" );
    return item;
}

void
SetUpWorld( SDownloadWorld& world )
{
    {
        CAssetStoreItem item = MakeItem( "item-1", "Medieval Sword Pack", "3d-model" );
        CAssetStoreFormat format;
        format.formatId = "fbx";
        format.name = "FBX";
        format.group = "3d-exchange";
        AddFile( world, format, "f1", "Models/SM_Sword.fbx", 300 * 1024, "sha1" );
        AddFile( world, format, "f2", "Textures/T_Sword_D.png", 100 * 1024, "sha256" );
        item.formats.push_back( format );
        world.server.AddItem( item );
    }
    {
        CAssetStoreItem item = MakeItem( "item-2", "Sci-fi Crate", "3d-model" );
        CAssetStoreFormat format;
        format.formatId = "fbx";
        AddFile( world, format, "f3", "SM_Crate.fbx", 64 * 1024, CString::Empty );
        item.formats.push_back( format );
        world.server.AddItem( item );
    }
    world.server.AddItem( MakeItem( "item-3", "Fantasy Barrel", "3d-model" ) );
    world.server.AddItem( MakeItem( "item-4", "Rock Material", "material" ) );
    world.server.AddItem( MakeItem( "item-5", "Space Ship", "3d-model" ) );
    {
        CAssetStoreItem item = MakeItem( "item-evil", "Evil Names", "3d-model" );
        CAssetStoreFormat format;
        format.formatId = "fbx";
        AddFile( world, format, "fevil", "../evil.txt", 1024, CString::Empty );
        item.formats.push_back( format );
        world.server.AddItem( item );
    }
    {
        CAssetStoreItem item = MakeItem( "item-corrupt", "Corrupt File", "3d-model" );
        CAssetStoreFormat format;
        format.formatId = "fbx";
        AddFile( world, format, "fcorrupt", "corrupt.bin", 32 * 1024, "sha256" );
        item.formats.push_back( format );
        world.server.AddItem( item );
    }
    for ( UInt32 b=0; b<6; ++b )
    {
        CAssetStoreItem item = MakeItem( "item-big-" + ToString( b ), "Big Pack " + ToString( b ), "3d-model" );
        CAssetStoreFormat format;
        format.formatId = "fbx";
        AddFile( world, format, "fbig" + ToString( b ), "big" + ToString( b ) + ".bin", 160 * 1024, "sha256" );
        item.formats.push_back( format );
        world.server.AddItem( item );
    }
}

CAssetStoreDownloadSettings
FastSettings( const CString& queueFile )
{
    CAssetStoreDownloadSettings settings;
    settings.retryBaseDelayInMs = 20;
    settings.retryMaxDelayInMs = 3000;
    settings.progressIntervalInMs = 10;
    settings.connectTimeoutInMs = 5000;
    settings.queueFilePath = queueFile;
    return settings;
}

bool
FileHasContent( const CString& vfsPath, const CDynamicBuffer& expected )
{
    CDynamicBuffer content;
    return ReadVfsFile( vfsPath, content ) && content.GetDataSize() == expected.GetDataSize() &&
           0 == memcmp( content.GetConstBufferPtr(), expected.GetConstBufferPtr(), expected.GetDataSize() );
}

CFakeStoreProvider*
FakeProvider( const CString& providerId )
{
    TAssetStoreProviderPtr provider;
    if ( !CAssetStoreGlobal::Instance()->GetRegistry().TryGetProvider( providerId, provider ) )
        return GUCEF_NULL;
    return static_cast< CFakeStoreProvider* >( provider.GetPointerAlways() );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      FUNCTIONS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDownloadTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "Downloads" );

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    SDownloadWorld world;
    SetUpWorld( world );
    const bool serverStarted = world.server.Start();
    CFakeStoreProvider::SetServer( &world.server );
    CAssetStoreRegistry& registry = CAssetStoreGlobal::Instance()->GetRegistry();
    CAssetStoreProviderFactory& factory = CAssetStoreGlobal::Instance()->GetProviderFactory();
    MountFreshDirectory( "downloads", "astest/downloads" );

    GUCEF_TESTFW_TESTCASE( "Registry: store plugins register a factory, the registry makes, finds and removes their providers" )
        try
        {
            ASSERT_TRUE( serverStarted );
            factory.RegisterConcreteFactory( "fakestore", &world.factoryA );
            factory.RegisterConcreteFactory( "fakestore2", &world.factoryB );
            ASSERT_TRUE( 2 == registry.RefreshProviders() );
            ASSERT_TRUE( 0 == registry.RefreshProviders() );
            ASSERT_TRUE( registry.HasProvider( "fakestore" ) && registry.HasProvider( "FAKESTORE2" ) );
            CString::StringVector ids;
            registry.GetProviderIds( ids );
            ASSERT_TRUE( 2 == ids.size() && "fakestore" == ids[ 0 ] && "fakestore2" == ids[ 1 ] );
            TAssetStoreProviderPtr provider;
            ASSERT_TRUE( registry.TryGetProvider( "fakestore", provider ) && "fakestore" == provider->GetProviderId() );
            ASSERT_FALSE( registry.AddProvider( provider ) );
            ASSERT_TRUE( registry.RemoveProvider( "fakestore2" ) );
            ASSERT_FALSE( registry.HasProvider( "fakestore2" ) );
            ASSERT_FALSE( registry.RemoveProvider( "fakestore2" ) );
            ASSERT_TRUE( 1 == registry.RefreshProviders() );
            ASSERT_TRUE( registry.HasProvider( "fakestore2" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Library: pages with cursors cover the library once, text and listing type filter it, the async form gives the page" )
        try
        {
            CFakeStoreProvider* provider = FakeProvider( "fakestore" );
            ASSERT_TRUE( GUCEF_NULL != provider );
            CAssetStoreQuery query;
            query.pageSize = 4;
            CString::StringSet seen;
            UInt32 pages = 0;
            UInt64 total = 0;
            for ( ;; )
            {
                CAssetStoreLibraryPage page;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == provider->QueryLibrary( query, page ) );
                ++pages;
                total = page.total;
                ASSERT_TRUE( page.hasTotal && page.items.size() <= 4 );
                for ( size_t i=0; i<page.items.size(); ++i )
                {
                    ASSERT_TRUE( seen.end() == seen.find( page.items[ i ].itemId ) );
                    seen.insert( page.items[ i ].itemId );
                }
                if ( page.nextCursor.IsNULLOrEmpty() || pages > 10 )
                    break;
                query.cursor = page.nextCursor;
            }
            ASSERT_TRUE( 13 == total && 13 == seen.size() && 4 == pages );

            CAssetStoreQuery text;
            text.text = "sword";
            CAssetStoreLibraryPage swords;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == provider->QueryLibrary( text, swords ) );
            ASSERT_TRUE( 1 == swords.items.size() && "item-1" == swords.items[ 0 ].itemId && 2 == swords.items[ 0 ].formats[ 0 ].files.size() );

            CAssetStoreQuery type;
            type.listingType = "material";
            CFutureResult future = provider->QueryLibraryAsync( type );
            CAssetStoreLibraryPage materials;
            ASSERT_TRUE( GetFutureResult( future, materials ) );
            ASSERT_TRUE( 1 == materials.items.size() && "item-4" == materials.items[ 0 ].itemId );

            CAssetStoreItem missing;
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == provider->GetItem( "nope", missing ) );
            CFutureResult missingFuture = provider->GetItemAsync( "nope" );
            ASSERT_TRUE( WaitForFuture( missingFuture, 10000 ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == CIAssetStoreProvider::GetStatusOfFuture( missingFuture ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Download: a job plans, downloads every file with progress events, checks SHA-1 and SHA-256 and renames the parts" )
        try
        {
            CAssetStoreDownloadSettings settings = FastSettings( CString::Empty );
            settings.maxBytesPerSecond = 2 * 1024 * 1024;
            CAssetStoreDownloadManager manager;
            manager.SetSettings( settings );
            CDownloadEventRecorder recorder;
            recorder.Observe( manager );
            manager.Start();
            CString jobId;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == manager.Enqueue( "fakestore", "item-1", "fbx", CString::StringVector(), "astest/downloads/basic", false, jobId, "Medieval Sword Pack" ) );
            ASSERT_TRUE( PumpUntilJobSettled( manager, jobId, 30000 ) );
            CAssetStoreDownloadJob job;
            ASSERT_TRUE( manager.GetJob( jobId, job ) && ASSETDOWNLOAD_STATE_DONE == job.state );
            ASSERT_TRUE( 2 == job.files.size() && job.files[ 0 ].done && job.files[ 1 ].done );
            ASSERT_TRUE( job.GetBytesDone() == 400 * 1024 && job.GetTotalBytes() == 400 * 1024 );
            ASSERT_TRUE( FileHasContent( "astest/downloads/basic/Models/SM_Sword.fbx", world.contents[ "f1" ] ) );
            ASSERT_TRUE( FileHasContent( "astest/downloads/basic/Textures/T_Sword_D.png", world.contents[ "f2" ] ) );
            ASSERT_FALSE( vfs.FileExists( "astest/downloads/basic/Models/SM_Sword.fbx.part" ) );
            ASSERT_TRUE( 1 == recorder.added && 1 == recorder.finished && 0 == recorder.failed );
            ASSERT_TRUE( recorder.progress >= 1 && recorder.lastRate > 0 );
            ASSERT_TRUE( recorder.states.size() >= 3 && "done" == recorder.states.back() );

            // a job of a store that is not there, and one into a folder no writeable mount covers
            CString other;
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == manager.Enqueue( "nostore", "item-1", "fbx", CString::StringVector(), "astest/downloads/basic", false, other ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_IO_ERROR == manager.Enqueue( "fakestore", "item-1", "fbx", CString::StringVector(), "notmounted/x", false, other ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Download: a part on disk resumes with a range, and starts again when the server does not resume" )
        try
        {
            CAssetStoreDownloadManager manager;
            manager.SetSettings( FastSettings( CString::Empty ) );
            manager.Start();
            const CDynamicBuffer& full = world.contents[ "f3" ];
            CDynamicBuffer half;
            half.Append( full.GetConstBufferPtr(), full.GetDataSize() / 2 );

            // the server resumes
            world.server.ResetStats();
            ASSERT_TRUE( WriteVfsFile( "astest/downloads/resume/SM_Crate.fbx.part", half ) );
            CString jobId;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == manager.Enqueue( "fakestore", "item-2", "fbx", CString::StringVector(), "astest/downloads/resume", false, jobId ) );
            ASSERT_TRUE( PumpUntilJobSettled( manager, jobId, 30000 ) );
            CAssetStoreDownloadJob job;
            ASSERT_TRUE( manager.GetJob( jobId, job ) && ASSETDOWNLOAD_STATE_DONE == job.state );
            ASSERT_TRUE( FileHasContent( "astest/downloads/resume/SM_Crate.fbx", full ) );
            ASSERT_TRUE( 1 == world.server.GetStats().rangeRequests );

            // the server ignores the range: the whole file replaces the part
            CFakeFileBehaviour noRange;
            noRange.supportsRange = false;
            world.server.SetFileBehaviour( "f3", noRange );
            world.server.ResetStats();
            CDynamicBuffer garbage;
            MakePattern( (UInt32) full.GetDataSize() / 3, 4242, garbage );
            ASSERT_TRUE( WriteVfsFile( "astest/downloads/restart/SM_Crate.fbx.part", garbage ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == manager.Enqueue( "fakestore", "item-2", "fbx", CString::StringVector(), "astest/downloads/restart", false, jobId ) );
            ASSERT_TRUE( PumpUntilJobSettled( manager, jobId, 30000 ) );
            ASSERT_TRUE( manager.GetJob( jobId, job ) && ASSETDOWNLOAD_STATE_DONE == job.state );
            ASSERT_TRUE( FileHasContent( "astest/downloads/restart/SM_Crate.fbx", full ) );
            ASSERT_TRUE( 1 == world.server.GetStats().rangeRequests );
            world.server.SetFileBehaviour( "f3", CFakeFileBehaviour() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Download: a file with another digest fails the job, the part is removed and nothing is renamed" )
        try
        {
            CFakeFileBehaviour corrupt;
            corrupt.corrupt = true;
            world.server.SetFileBehaviour( "fcorrupt", corrupt );
            CAssetStoreDownloadManager manager;
            manager.SetSettings( FastSettings( CString::Empty ) );
            CDownloadEventRecorder recorder;
            recorder.Observe( manager );
            manager.Start();
            CString jobId;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == manager.Enqueue( "fakestore", "item-corrupt", "fbx", CString::StringVector(), "astest/downloads/corrupt", false, jobId ) );
            ASSERT_TRUE( PumpUntilJobSettled( manager, jobId, 30000 ) );
            CAssetStoreDownloadJob job;
            ASSERT_TRUE( manager.GetJob( jobId, job ) && ASSETDOWNLOAD_STATE_FAILED == job.state && ASSETSTORE_STATUS_DIGEST_MISMATCH == job.status );
            ASSERT_FALSE( vfs.FileExists( "astest/downloads/corrupt/corrupt.bin" ) );
            ASSERT_FALSE( vfs.FileExists( "astest/downloads/corrupt/corrupt.bin.part" ) );
            ASSERT_TRUE( 1 == recorder.failed && ASSETSTORE_STATUS_DIGEST_MISMATCH == recorder.lastFailed.status );

            // after the store fixed it, a retry succeeds
            world.server.SetFileBehaviour( "fcorrupt", CFakeFileBehaviour() );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == manager.Retry( jobId ) );
            ASSERT_TRUE( PumpUntilJobSettled( manager, jobId, 30000 ) );
            ASSERT_TRUE( manager.GetJob( jobId, job ) && ASSETDOWNLOAD_STATE_DONE == job.state );
            ASSERT_TRUE( FileHasContent( "astest/downloads/corrupt/corrupt.bin", world.contents[ "fcorrupt" ] ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Download: 503 and 429 are retried with back-off and Retry-After, an expired address renews the plan once" )
        try
        {
            CAssetStoreDownloadManager manager;
            manager.SetSettings( FastSettings( CString::Empty ) );
            manager.Start();
            CFakeStoreProvider* provider = FakeProvider( "fakestore" );

            CFakeFileBehaviour busy;
            busy.fail503 = 2;
            world.server.SetFileBehaviour( "f3", busy );
            world.server.ResetStats();
            CString jobId;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == manager.Enqueue( "fakestore", "item-2", "fbx", CString::StringVector(), "astest/downloads/busy", false, jobId ) );
            ASSERT_TRUE( PumpUntilJobSettled( manager, jobId, 30000 ) );
            CAssetStoreDownloadJob job;
            ASSERT_TRUE( manager.GetJob( jobId, job ) && ASSETDOWNLOAD_STATE_DONE == job.state && 2 == job.attempts );
            ASSERT_TRUE( 2 == world.server.GetStats().unavailableAnswers && 3 == world.server.GetStats().fileRequests );
            ASSERT_TRUE( FileHasContent( "astest/downloads/busy/SM_Crate.fbx", world.contents[ "f3" ] ) );

            CFakeFileBehaviour limited;
            limited.fail429 = 1;
            limited.retryAfterSeconds = 1;
            world.server.SetFileBehaviour( "f3", limited );
            world.server.ResetStats();
            const UInt64 start = MT::PrecisionTickCount();
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == manager.Enqueue( "fakestore", "item-2", "fbx", CString::StringVector(), "astest/downloads/limited", false, jobId ) );
            ASSERT_TRUE( PumpUntilJobSettled( manager, jobId, 30000 ) );
            const UInt64 elapsedMs = ( MT::PrecisionTickCount() - start ) * 1000 / MT::PrecisionTimerResolution();
            ASSERT_TRUE( manager.GetJob( jobId, job ) && ASSETDOWNLOAD_STATE_DONE == job.state );
            ASSERT_TRUE( 1 == world.server.GetStats().rateLimitedAnswers && elapsedMs >= 950 );

            CFakeFileBehaviour expired;
            expired.expireFirst = 1;
            world.server.SetFileBehaviour( "f3", expired );
            const UInt32 plansBefore = provider->GetPlanCount();
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == manager.Enqueue( "fakestore", "item-2", "fbx", CString::StringVector(), "astest/downloads/expired", false, jobId ) );
            ASSERT_TRUE( PumpUntilJobSettled( manager, jobId, 30000 ) );
            ASSERT_TRUE( manager.GetJob( jobId, job ) && ASSETDOWNLOAD_STATE_DONE == job.state );
            ASSERT_TRUE( 2 == provider->GetPlanCount() - plansBefore );

            // an address that expires again after the renewal fails the job
            expired.expireFirst = 5;
            world.server.SetFileBehaviour( "f3", expired );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == manager.Enqueue( "fakestore", "item-2", "fbx", CString::StringVector(), "astest/downloads/expired2", false, jobId ) );
            ASSERT_TRUE( PumpUntilJobSettled( manager, jobId, 30000 ) );
            ASSERT_TRUE( manager.GetJob( jobId, job ) && ASSETDOWNLOAD_STATE_FAILED == job.state && ASSETSTORE_STATUS_URL_EXPIRED == job.status );
            world.server.SetFileBehaviour( "f3", CFakeFileBehaviour() );

            // a store that stays busy: the job gives up after the attempts of the settings
            CAssetStoreDownloadSettings fewAttempts = FastSettings( CString::Empty );
            fewAttempts.maxAttempts = 2;
            manager.SetSettings( fewAttempts );
            busy.fail503 = 10;
            world.server.SetFileBehaviour( "f3", busy );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == manager.Enqueue( "fakestore", "item-2", "fbx", CString::StringVector(), "astest/downloads/givesup", false, jobId ) );
            ASSERT_TRUE( PumpUntilJobSettled( manager, jobId, 30000 ) );
            ASSERT_TRUE( manager.GetJob( jobId, job ) && ASSETDOWNLOAD_STATE_FAILED == job.state && ASSETSTORE_STATUS_SERVER_ERROR == job.status );
            world.server.SetFileBehaviour( "f3", CFakeFileBehaviour() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Download: 401 renews the session once; when that does not help the job fails with \"sign in again\"" )
        try
        {
            CAssetStoreDownloadManager manager;
            manager.SetSettings( FastSettings( CString::Empty ) );
            manager.Start();
            CFakeStoreProvider* provider = FakeProvider( "fakestore" );
            provider->SetToken( "stale-token" );
            world.server.SetRequireAuth( true );
            world.server.SetTokenEndpointWorks( true );
            world.server.ResetStats();
            const UInt32 renewsBefore = provider->GetRenewCount();
            CString jobId;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == manager.Enqueue( "fakestore", "item-2", "fbx", CString::StringVector(), "astest/downloads/auth", false, jobId ) );
            ASSERT_TRUE( PumpUntilJobSettled( manager, jobId, 30000 ) );
            CAssetStoreDownloadJob job;
            ASSERT_TRUE( manager.GetJob( jobId, job ) && ASSETDOWNLOAD_STATE_DONE == job.state );
            ASSERT_TRUE( 1 == provider->GetRenewCount() - renewsBefore && 1 == world.server.GetStats().refusedSessions );

            provider->SetToken( "stale-token" );
            world.server.SetTokenEndpointWorks( false );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == manager.Enqueue( "fakestore", "item-2", "fbx", CString::StringVector(), "astest/downloads/auth2", false, jobId ) );
            ASSERT_TRUE( PumpUntilJobSettled( manager, jobId, 30000 ) );
            ASSERT_TRUE( manager.GetJob( jobId, job ) && ASSETDOWNLOAD_STATE_FAILED == job.state && ASSETSTORE_STATUS_SESSION_EXPIRED == job.status );
            ASSERT_TRUE( job.message == "sign in again" );
            world.server.SetRequireAuth( false );
            world.server.SetTokenEndpointWorks( true );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Download: pause keeps the part and resume continues with a range; cancel removes the part" )
        try
        {
            CAssetStoreDownloadSettings settings = FastSettings( CString::Empty );
            settings.maxBytesPerSecond = 192 * 1024;
            CAssetStoreDownloadManager manager;
            manager.SetSettings( settings );
            manager.Start();
            world.server.ResetStats();
            CString jobId;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == manager.Enqueue( "fakestore", "item-big-0", "fbx", CString::StringVector(), "astest/downloads/pause", false, jobId ) );
            ASSERT_TRUE( PumpUntilJobBytes( manager, jobId, 40 * 1024, 30000 ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == manager.Pause( jobId ) );
            ASSERT_TRUE( PumpUntilJobSettled( manager, jobId, 30000 ) );
            CAssetStoreDownloadJob job;
            ASSERT_TRUE( manager.GetJob( jobId, job ) && ASSETDOWNLOAD_STATE_PAUSED == job.state );
            ASSERT_TRUE( vfs.FileExists( "astest/downloads/pause/big0.bin.part" ) );
            const UInt64 partSize = vfs.GetFileSize( "astest/downloads/pause/big0.bin.part" );
            ASSERT_TRUE( partSize > 0 && partSize < 160 * 1024 );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == manager.Resume( jobId ) );
            ASSERT_TRUE( PumpUntilJobSettled( manager, jobId, 30000 ) );
            ASSERT_TRUE( manager.GetJob( jobId, job ) && ASSETDOWNLOAD_STATE_DONE == job.state );
            ASSERT_TRUE( FileHasContent( "astest/downloads/pause/big0.bin", world.contents[ "fbig0" ] ) );
            ASSERT_TRUE( 1 == world.server.GetStats().rangeRequests );

            ASSERT_TRUE( ASSETSTORE_STATUS_OK == manager.Enqueue( "fakestore", "item-big-1", "fbx", CString::StringVector(), "astest/downloads/cancel", false, jobId ) );
            ASSERT_TRUE( PumpUntilJobBytes( manager, jobId, 40 * 1024, 30000 ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == manager.Cancel( jobId ) );
            ASSERT_TRUE( PumpUntilJobSettled( manager, jobId, 30000 ) );
            ASSERT_TRUE( manager.GetJob( jobId, job ) && ASSETDOWNLOAD_STATE_CANCELLED == job.state );
            ASSERT_FALSE( vfs.FileExists( "astest/downloads/cancel/big1.bin.part" ) );
            ASSERT_FALSE( vfs.FileExists( "astest/downloads/cancel/big1.bin" ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == manager.Pause( jobId ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == manager.Remove( jobId ) );
            ASSERT_FALSE( manager.GetJob( jobId, job ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Download: unsafe names of a store are refused before anything is written" )
        try
        {
            ASSERT_FALSE( CAssetStoreSafeNames::IsSafeRelativeName( "../evil.txt" ) );
            CAssetStoreDownloadManager manager;
            manager.SetSettings( FastSettings( CString::Empty ) );
            manager.Start();
            CString jobId;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == manager.Enqueue( "fakestore", "item-evil", "fbx", CString::StringVector(), "astest/downloads/evil/inner", false, jobId ) );
            ASSERT_TRUE( PumpUntilJobSettled( manager, jobId, 30000 ) );
            CAssetStoreDownloadJob job;
            ASSERT_TRUE( manager.GetJob( jobId, job ) && ASSETDOWNLOAD_STATE_FAILED == job.state && ASSETSTORE_STATUS_UNSAFE_NAME == job.status );
            ASSERT_FALSE( vfs.FileExists( "astest/downloads/evil/evil.txt" ) );
            ASSERT_FALSE( vfs.FileExists( "astest/downloads/evil/evil.txt.part" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Download: a plan the store refuses fails the job with the reason the store gives for it (GetPlanProblem), not only the status" )
        try
        {
            CFakeSignInStoreProviderFactory refusingFactory( "planrefuse", false );
            CAssetStoreGlobal::Instance()->GetProviderFactory().RegisterConcreteFactory( "planrefuse", &refusingFactory );
            CAssetStoreGlobal::Instance()->GetRegistry().RefreshProviders();
            CAssetStoreDownloadManager manager;
            manager.SetSettings( FastSettings( CString::Empty ) );
            manager.Start();
            CString jobId;
            const bool queued = ASSETSTORE_STATUS_OK == manager.Enqueue( "planrefuse", "item-r", "fbx", CString::StringVector(), "astest/downloads/refused", false, jobId );
            const bool settled = queued && PumpUntilJobSettled( manager, jobId, 30000 );
            CAssetStoreDownloadJob job;
            const bool found = settled && manager.GetJob( jobId, job );
            manager.Stop();
            CAssetStoreGlobal::Instance()->GetRegistry().RemoveProvider( "planrefuse" );
            CAssetStoreGlobal::Instance()->GetProviderFactory().UnregisterConcreteFactory( "planrefuse" );
            ASSERT_TRUE( queued && settled && found );
            ASSERT_TRUE( ASSETDOWNLOAD_STATE_FAILED == job.state && ASSETSTORE_STATUS_UNSUPPORTED == job.status );
            ASSERT_TRUE( job.message.HasSubstr( "the store could not plan the download: the fake sign in store has no downloads (item-r fbx)", true ) >= 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Download: at most the global and the per store number of jobs run at once" )
        try
        {
            CAssetStoreDownloadSettings settings = FastSettings( CString::Empty );
            settings.maxConcurrentJobs = 3;
            settings.maxConcurrentJobsPerProvider = 2;
            settings.maxBytesPerSecond = 256 * 1024;
            CAssetStoreDownloadManager manager;
            manager.SetSettings( settings );
            CString::StringVector jobs;
            for ( UInt32 j=0; j<6; ++j )
            {
                CString jobId;
                const CString store = j < 3 ? "fakestore" : "fakestore2";
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == manager.Enqueue( store, "item-big-" + ToString( j ), "fbx", CString::StringVector(), "astest/downloads/concurrent" + ToString( j ), false, jobId ) );
                jobs.push_back( jobId );
            }
            manager.ResetPeakRunningJobCounts();
            manager.Start();
            UInt32 maxSeen = 0;
            for ( UInt32 waited=0; waited<60000; waited+=5 )
            {
                PumpFor( 5 );
                const UInt32 running = manager.GetRunningJobCount();
                if ( running > maxSeen )
                    maxSeen = running;
                bool allDone = true;
                for ( size_t j=0; j<jobs.size(); ++j )
                    allDone = allDone && manager.WaitForJob( jobs[ j ], 0 );
                if ( allDone )
                    break;
            }
            for ( size_t j=0; j<jobs.size(); ++j )
            {
                CAssetStoreDownloadJob job;
                ASSERT_TRUE( manager.GetJob( jobs[ j ], job ) && ASSETDOWNLOAD_STATE_DONE == job.state );
            }
            ASSERT_TRUE( manager.GetPeakRunningJobCount() <= 3 && manager.GetPeakRunningJobCountOfProvider() <= 2 );
            ASSERT_TRUE( manager.GetPeakRunningJobCount() >= 2 && maxSeen >= 2 && maxSeen <= 3 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Download: the queue is saved as JSON and taken back after a restart, running jobs continue from their parts" )
        try
        {
            const CString queueFile = "astest/downloads/queue/downloads.json";
            CString waitingJob;
            CString runningJob;
            {
                CAssetStoreDownloadSettings settings = FastSettings( queueFile );
                settings.maxBytesPerSecond = 128 * 1024;
                settings.maxConcurrentJobs = 1;
                CAssetStoreDownloadManager manager;
                manager.SetSettings( settings );
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == manager.Enqueue( "fakestore", "item-big-2", "fbx", CString::StringVector(), "astest/downloads/queue/a", false, runningJob ) );
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == manager.Enqueue( "fakestore", "item-big-3", "fbx", CString::StringVector(), "astest/downloads/queue/b", false, waitingJob ) );
                ASSERT_TRUE( vfs.FileExists( queueFile ) );
                manager.Start();
                ASSERT_TRUE( PumpUntilJobBytes( manager, runningJob, 30 * 1024, 30000 ) );
                manager.Stop();
                CAssetStoreDownloadJob job;
                ASSERT_TRUE( manager.GetJob( runningJob, job ) && ASSETDOWNLOAD_STATE_QUEUED == job.state );
            }
            ASSERT_TRUE( vfs.FileExists( "astest/downloads/queue/a/big2.bin.part" ) );

            CAssetStoreDownloadManager restarted;
            restarted.SetSettings( FastSettings( queueFile ) );
            ASSERT_TRUE( restarted.LoadQueue() );
            TAssetStoreDownloadJobVector jobs;
            restarted.GetJobs( jobs );
            ASSERT_TRUE( 2 == jobs.size() && runningJob == jobs[ 0 ].jobId && waitingJob == jobs[ 1 ].jobId );
            ASSERT_TRUE( ASSETDOWNLOAD_STATE_QUEUED == jobs[ 0 ].state && ASSETDOWNLOAD_STATE_QUEUED == jobs[ 1 ].state );
            world.server.ResetStats();
            restarted.Start();
            ASSERT_TRUE( PumpUntilJobSettled( restarted, runningJob, 30000 ) && PumpUntilJobSettled( restarted, waitingJob, 30000 ) );
            CAssetStoreDownloadJob job;
            ASSERT_TRUE( restarted.GetJob( runningJob, job ) && ASSETDOWNLOAD_STATE_DONE == job.state );
            ASSERT_TRUE( restarted.GetJob( waitingJob, job ) && ASSETDOWNLOAD_STATE_DONE == job.state );
            ASSERT_TRUE( FileHasContent( "astest/downloads/queue/a/big2.bin", world.contents[ "fbig2" ] ) );
            ASSERT_TRUE( FileHasContent( "astest/downloads/queue/b/big3.bin", world.contents[ "fbig3" ] ) );
            ASSERT_TRUE( world.server.GetStats().rangeRequests >= 1 );

            // the saved queue holds the end states
            CAssetStoreDownloadManager third;
            third.SetSettings( FastSettings( queueFile ) );
            ASSERT_TRUE( third.LoadQueue() );
            ASSERT_TRUE( third.GetJob( waitingJob, job ) && ASSETDOWNLOAD_STATE_DONE == job.state );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Download for the primary store: a completed download of a store with the automatic policy is ingested, one with the manual policy is not" )
        try
        {
            MountFreshDirectory( "autoingest", "astest/autoingest" );
            CPrimaryAssetStoreSettings settings;
            settings.locations.clear();
            CAssetStorageLocation location;
            location.locationId = "main";
            location.vfsRoot = "astest/autoingest/main";
            settings.locations.push_back( location );
            settings.defaultLocationId = "main";
            settings.database.backendName = "vfstables";
            settings.database.database = "astest/autoingest/catalogue";
            CAssetSourcePolicy automatic;
            automatic.sourceId = "fakestore";
            automatic.mode = ASSETINGEST_MODE_AUTOMATIC;
            automatic.keep = ASSETKEEP_ORIGINAL;
            settings.SetPolicy( automatic );
            vfs.CreateDir( "astest/autoingest/main" );
            CPrimaryAssetStore store;
            store.SetSettings( settings );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Open() );

            CAssetStoreDownloadManager manager;
            manager.SetSettings( FastSettings( CString::Empty ) );
            store.AttachDownloadManager( &manager );
            manager.Start();
            CString automaticJob;
            CString manualJob;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.EnqueueDownload( manager, "fakestore", "item-2", "fbx", CString::StringVector(), automaticJob ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.EnqueueDownload( manager, "fakestore2", "item-1", "fbx", CString::StringVector(), manualJob ) );
            CAssetStoreDownloadJob job;
            ASSERT_TRUE( manager.GetJob( automaticJob, job ) && job.forIngest && 0 == job.destination.HasSubstr( "astest/autoingest/main/staging/dl" ) );
            ASSERT_TRUE( PumpUntilJobSettled( manager, automaticJob, 30000 ) && PumpUntilJobSettled( manager, manualJob, 30000 ) );

            CAssetSearchQuery packs;
            packs.resultKind = "packs";
            CAssetSearchResultPage page;
            for ( UInt32 waited=0; waited<30000 && ( ASSETSTORE_STATUS_OK != store.Search( packs, page ) || page.total < 1 ); waited+=20 )
                PumpFor( 20 );
            PumpFor( 200 );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( packs, page ) && 1 == page.total );
            CAssetPackRecord pack;
            ASSERT_TRUE( !page.hits.empty() && ASSETSTORE_STATUS_OK == store.GetPack( page.hits[ 0 ].packId, pack ) );
            ASSERT_TRUE( "fakestore" == pack.sourceId && "item-2" == pack.sourceItemId && "Sci-fi Crate" == pack.title && ASSETKEEP_ORIGINAL == pack.keep );
            ASSERT_TRUE( ASSETPACK_STATE_INGESTED == pack.state && !pack.categories.empty() );
            ASSERT_TRUE( 1 == pack.unmappedCategories.count( "3D/Weapons & Combat/Melee" ) );
            ASSERT_TRUE( "layout2:Sci-fi Crate" == pack.rootPath );

            // the manual download waits in the staging area to be added
            ASSERT_TRUE( manager.GetJob( manualJob, job ) && vfs.DirExists( job.destination ) );
            CAssetIngestResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Ingest( CPrimaryAssetStore::MakeIngestRequest( job ), result ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( result.packId, pack ) && "fakestore2" == pack.sourceId && "Medieval Sword Pack" == pack.title );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( packs, page ) && 2 == page.total );
            store.AttachDownloadManager( GUCEF_NULL );
            manager.Stop();
            store.Close();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    // the providers go before their factories
    registry.RemoveProvider( "fakestore" );
    registry.RemoveProvider( "fakestore2" );
    factory.UnregisterConcreteFactory( "fakestore" );
    factory.UnregisterConcreteFactory( "fakestore2" );
    CFakeStoreProvider::SetServer( GUCEF_NULL );
    world.server.Stop();
    UnmountDirectory( "downloads" );
    UnmountDirectory( "autoingest" );
}

/*-------------------------------------------------------------------------*/
