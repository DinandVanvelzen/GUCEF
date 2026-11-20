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

#include <stdlib.h>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#include "gucefAUTH_CMemoryCredentialStore.h"
#define GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#endif /* GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H ? */

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

#ifndef GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H
#include "gucefASSETSTORE_CPrimaryAssetStore.h"
#define GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H
#endif /* GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H ? */

#ifndef GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#include "gucefASSETSTORE_CLocalAssetStoreService.h"
#define GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#include "gucefASSETSTORE_CAssetPipelineOperations.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETREPROCESSREQUEST_H
#include "gucefASSETSTORE_CAssetReprocessRequest.h"
#define GUCEF_ASSETSTORE_CASSETREPROCESSREQUEST_H
#endif /* GUCEF_ASSETSTORE_CASSETREPROCESSREQUEST_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSPEC_H
#include "gucefASSETSTORE_CAssetLibrarySyncSpec.h"
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSPEC_H
#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSPEC_H ? */

#ifndef GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H
#include "gucefASSETSTORE_CPackStorageLayout.h"
#define GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H
#endif /* GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETPACKGENERATIONS_H
#include "gucefASSETSTORE_CAssetPackGenerations.h"
#define GUCEF_ASSETSTORE_CASSETPACKGENERATIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKGENERATIONS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSYNCMARKSTORE_H
#include "gucefASSETSTORE_CAssetSyncMarkStore.h"
#define GUCEF_ASSETSTORE_CASSETSYNCMARKSTORE_H
#endif /* GUCEF_ASSETSTORE_CASSETSYNCMARKSTORE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETPACKSOURCERECORD_H
#include "gucefASSETSTORE_CAssetPackSourceRecord.h"
#define GUCEF_ASSETSTORE_CASSETPACKSOURCERECORD_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKSOURCERECORD_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLCLIENT_H
#include "dbpluginPOSTGRESQL_CPostgreSqlClient.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLCLIENT_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLCLIENT_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLCONNECTION_H
#include "dbpluginPOSTGRESQL_CPostgreSqlConnection.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLCONNECTION_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLCONNECTION_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLSQL_H
#include "dbpluginPOSTGRESQL_CPostgreSqlSql.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLSQL_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLSQL_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLSETTINGS_H
#include "dbpluginPOSTGRESQL_CPostgreSqlSettings.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLSETTINGS_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLSETTINGS_H ? */

#ifndef TESTASSETSTORESUPPORT_H
#include "TestAssetStoreSupport.h"
#define TESTASSETSTORESUPPORT_H
#endif /* TESTASSETSTORESUPPORT_H ? */

#ifndef TESTFAKECONVERTERHOOK_H
#include "TestFakeConverterHook.h"
#define TESTFAKECONVERTERHOOK_H
#endif /* TESTFAKECONVERTERHOOK_H ? */

#ifndef TESTFAKESTOREPROVIDER_H
#include "TestFakeStoreProvider.h"
#define TESTFAKESTOREPROVIDER_H
#endif /* TESTFAKESTOREPROVIDER_H ? */

#ifndef TESTFAKESTORESERVER_H
#include "TestFakeStoreServer.h"
#define TESTFAKESTORESERVER_H
#endif /* TESTFAKESTORESERVER_H ? */

#ifndef TESTASSETSTOREREMOTEWORLD_H
#include "TestAssetStoreRemoteWorld.h"
#define TESTASSETSTOREREMOTEWORLD_H
#endif /* TESTASSETSTOREREMOTEWORLD_H ? */

#include "TestReprocess.h"

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

const char* const ReproStoreId = "reprostore";
const char* const ReproItemId = "repro-01";

struct SReprocessFixture
{
    CString label;
    CString base;
    COMCORE::CDatabaseClientSettings database;
    AUTH::ICredentialStore* credentials;
    SReprocessFixture( void ) : label(), base(), database(), credentials( GUCEF_NULL ) {}
};

/** The item of the fake store: two models and a texture in one format; 'seed' changes the content of the first model */
CAssetStoreItem
AddReproItem( CFakeStoreServer& server, UInt32 seed, bool add )
{
    CAssetStoreItem item;
    item.providerId = ReproStoreId;
    item.itemId = ReproItemId;
    item.title = "Reprocess Test Pack";
    item.seller = "Test Seller";
    item.listingType = "3d-model";
    item.licenceName = "Standard License";
    item.changedAtMs = 1700000000000LL;
    CAssetStoreFormat format;
    format.formatId = "fbx";
    format.name = "FBX";
    static const char* const names[] = { "SM_Rock_01.fbx", "SM_Tree_01.fbx", "T_Rock_01.tga" };
    for ( UInt32 f=0; f<3; ++f )
    {
        CDynamicBuffer content;
        const CString text = CString( "content of " ) + names[ f ] + ( 0 == f ? " seed " + ToString( seed ) : CString::Empty );
        content.Append( text.C_String(), text.Length() );
        const CString fileId = CString( ReproItemId ) + "-" + ToString( f );
        server.SetFileContent( fileId, content );
        CAssetStoreFile file;
        file.fileId = fileId;
        file.name = names[ f ];
        file.size = content.GetDataSize();
        file.digestAlgorithm = "sha256";
        file.digest = Sha256Of( content );
        format.files.push_back( file );
    }
    item.formats.push_back( format );
    if ( add )
        server.AddItem( item );
    else
        server.ReplaceItem( item );
    return item;
}

/** The fake store with the item, registered as the provider "reprostore" */
class CReprocessStoreWorld
{
    public:

    CFakeStoreServer server;
    CFakeStoreProviderFactory factory;
    bool started;

    CReprocessStoreWorld( void )
        : server(), factory( ReproStoreId ), started( false )
    {
        AddReproItem( server, 1, true );
        started = server.Start();
        CFakeStoreProvider::SetServer( &server );
        CAssetStoreGlobal::Instance()->GetProviderFactory().RegisterConcreteFactory( ReproStoreId, &factory );
        CAssetStoreGlobal::Instance()->GetRegistry().RefreshProviders();
    }

    ~CReprocessStoreWorld()
    {
        CAssetStoreGlobal::Instance()->GetRegistry().RemoveProvider( ReproStoreId );
        CAssetStoreGlobal::Instance()->GetProviderFactory().UnregisterConcreteFactory( ReproStoreId );
        CFakeStoreProvider::SetServer( GUCEF_NULL );
        server.Stop();
    }
};

CAssetStoreServiceConfig
MakeReprocessConfig( const SReprocessFixture& fixture, CTestConverterHook& hook )
{
    CAssetStoreServiceConfig config;
    config.primaryStore.locations.clear();
    CAssetStorageLocation main;
    main.locationId = "main";
    main.name = "Main";
    main.vfsRoot = fixture.base + "/main";
    config.primaryStore.locations.push_back( main );
    config.primaryStore.defaultLocationId = "main";
    config.primaryStore.analysisConcurrency = 2;
    config.primaryStore.database = fixture.database;
    config.credentialStore = fixture.credentials;
    if ( GUCEF_NULL != fixture.credentials )
    {
        config.primaryStore.credentialProvider = "gucefASSETSTORE.test";
        config.primaryStore.credentialAccount = "catalogue";
    }
    CAssetSourcePolicy policy;
    policy.sourceId = ReproStoreId;
    policy.mode = ASSETINGEST_MODE_MANUAL;
    policy.keep = ASSETKEEP_BOTH;
    config.primaryStore.SetPolicy( policy );
    CAssetSourcePolicy disk;
    disk.sourceId = "disk";
    disk.mode = ASSETINGEST_MODE_MANUAL;
    disk.keep = ASSETKEEP_BOTH;
    config.primaryStore.SetPolicy( disk );
    config.converterHook = &hook;
    config.downloads.retryBaseDelayInMs = 20;
    config.downloads.retryMaxDelayInMs = 2000;
    config.downloads.maxAttempts = 2;
    config.downloads.progressIntervalInMs = 10;
    config.downloads.connectTimeoutInMs = 5000;
    config.reprocess.maxConcurrentJobs = 1;
    config.reprocess.maxJobsPerPackPerHour = 2;
    config.reprocess.maxMachineRedownloadsPerDay = 5;
    return config;
}

bool
WaitForJob( CIAssetStoreService& service, const CString& jobId, CAssetJobEntry& job, UInt32 timeoutInMs = 60000 )
{
    for ( UInt32 waited=0; waited<timeoutInMs; waited+=10 )
    {
        if ( ASSETSTORE_STATUS_OK == service.GetJob( jobId, job ) && job.IsEnded() )
            return true;
        PumpFor( 10 );
    }
    return false;
}

/** A reprocessing request of the operation for the pack; machine: as the MCP tools ask */
CAssetPipelineRequest
MakeReprocess( const CString& operation, const CString& packId, bool machine = false )
{
    CAssetReprocessRequest spec;
    spec.operation = operation;
    spec.packId = packId;
    spec.caller = machine ? CString( "mcp:test-session" ) : CString( "test-owner" );
    spec.machine = machine;
    CAssetPipelineRequest request;
    spec.ToRequest( request );
    return request;
}

CAssetPipelineRequest
MakeReprocess( const CAssetReprocessRequest& spec )
{
    CAssetPipelineRequest request;
    spec.ToRequest( request );
    return request;
}

/** Starts a job and waits for its end; the status of the start, the job and its result */
TAssetStoreStatus
RunJob( CIAssetStoreService& service, const CAssetPipelineRequest& request, CAssetJobEntry& job, CAssetServiceDocument& result, CString* jobId = GUCEF_NULL )
{
    CAssetServiceResult started;
    const TAssetStoreStatus status = service.StartPipelineJob( CString::Empty, request, started );
    if ( GUCEF_NULL != jobId )
        *jobId = started.id;
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    if ( !WaitForJob( service, started.id, job ) )
        return ASSETSTORE_STATUS_TIMED_OUT;
    service.GetJobResult( started.id, CString::Empty, CString::Empty, 0, result );
    return ASSETSTORE_STATUS_OK;
}

CString
Text( const CDataNode& node, const char* path )
{
    const CDataNode* at = &node;
    CString::StringVector parts = CString( path ).ParseElements( '/', false );
    for ( size_t i=0; i+1<parts.size() && GUCEF_NULL != at; ++i )
        at = at->FindChild( parts[ i ] );
    return GUCEF_NULL == at ? CString::Empty : at->GetAttributeValueOrChildValueByName( parts.back() ).AsString( CString::Empty, true );
}

UInt32
Number( const CDataNode& node, const char* path )
{
    return StringToUInt32( Text( node, path ), 0 );
}

CString
ReadText( const CString& vfsPath )
{
    CDynamicBuffer content;
    if ( !ReadVfsFile( vfsPath, content ) )
        return CString::Empty;
    return CString( static_cast< const char* >( content.GetConstBufferPtr() ), content.GetDataSize() );
}

const CAssetRecord*
FindAsset( const TAssetRecordVector& assets, const CString& name )
{
    for ( size_t a=0; a<assets.size(); ++a )
        if ( assets[ a ].name == name )
            return &assets[ a ];
    return GUCEF_NULL;
}

/** The current pack of the item of the fake store */
CString
CurrentPackOfItem( CPrimaryAssetStore& store )
{
    TAssetPackSourceRecordVector facts;
    store.GetSyncStore().GetPackSourcesOfItem( ReproStoreId, ReproItemId, facts );
    for ( size_t f=0; f<facts.size(); ++f )
        if ( facts[ f ].IsCurrent() )
            return facts[ f ].packId;
    return CString::Empty;
}

UInt32
CountHistory( CIAssetStoreService& service, const CString& packId, const CString& origin )
{
    CAssetIngestHistoryPage page;
    service.GetIngestHistory( packId, CString::Empty, 500, page );
    UInt32 count = 0;
    for ( size_t e=0; e<page.entries.size(); ++e )
        if ( page.entries[ e ].origin == origin )
            ++count;
    return count;
}

bool
HistoryNames( CIAssetStoreService& service, const CString& packId, const CString& origin, const CString& callerPart )
{
    CAssetIngestHistoryPage page;
    service.GetIngestHistory( packId, CString::Empty, 500, page );
    for ( size_t e=0; e<page.entries.size(); ++e )
        if ( page.entries[ e ].origin == origin && page.entries[ e ].policy.HasSubstr( callerPart ) >= 0 )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

void
RunReprocessSuite( SReprocessFixture& fixture )
{
    MountFreshDirectory( fixture.label + "_main", fixture.base + "/main" );
    MountFreshDirectory( fixture.label + "_incoming", fixture.base + "/incoming" );
    CReprocessStoreWorld world;
    CTestConverterHook hook;
    CString error;
    CAssetStoreServiceConfig config = MakeReprocessConfig( fixture, hook );
    TAssetStoreServicePtr servicePtr = CAssetStoreGlobal::Instance()->CreateService( config, error );
    const bool opened = !servicePtr.IsNULL() && ASSETSTORE_STATUS_OK == servicePtr->Open();
    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": the service with the fake store and the test converter opens" ).C_String() )
        ASSERT_TRUE( world.started && opened );
    GUCEF_TESTFW_TESTCASE_END;
    if ( !opened || !world.started )
        return;
    CIAssetStoreService& service = *servicePtr;
    CLocalAssetStoreService& local = static_cast< CLocalAssetStoreService& >( service );
    CPrimaryAssetStore& store = *local.GetPrimaryStore();
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    CString packId;
    CString intermediate;
    CString cooked;
    CString generations;
    CString originalRock;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a library sync brings the item in as a pack with its outputs and store facts" ).C_String() )
        try
        {
            CAssetLibrarySyncProfile profile;
            profile.reservePercent = 0;
            profile.reserveBytes = 0;
            profile.indexAfter = false;
            CAssetServiceResult set;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.SetSyncProfile( CString::Empty, ReproStoreId, profile, set ) );
            CAssetLibrarySyncSpec spec;
            spec.storeId = ReproStoreId;
            spec.mode = CAssetLibrarySyncSpec::ModeRun;
            spec.useStoreProfile = true;
            spec.itemIds.push_back( ReproItemId );
            CAssetPipelineRequest request;
            spec.ToRequest( request );
            CAssetJobEntry job;
            CAssetServiceDocument result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == RunJob( service, request, job, result ) );
            packId = CurrentPackOfItem( store );
            ASSERT_TRUE( !packId.IsNULLOrEmpty() );
            CAssetPackRecord pack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( packId, pack ) && CAssetConversionRecord::OutcomeConverted == pack.conversion.outcome );
            const CPackStorageLayout layout( pack.rootPath );
            const CString root = store.GetLocationRoot( pack.locationId );
            intermediate = layout.GetIntermediateFolder( root, CString::Empty );
            cooked = layout.GetCookedFolder( root, CString::Empty );
            generations = layout.GetGenerationsFolder( root );
            originalRock = layout.Resolve( root, "original/SM_Rock_01.fbx" );
            ASSERT_TRUE( "converted SM_Rock_01 v1" == ReadText( intermediate + "/SM_Rock_01.gltf" ) );
            ASSERT_TRUE( "cooked SM_Rock_01 v1" == ReadText( cooked + "/SM_Rock_01.mesh" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
    if ( packId.IsNULLOrEmpty() )
    {
        service.Close();
        return;
    }

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": reprocess_plan tells what a regeneration would do, read only: it would run, two models in no scope, no rollback yet" ).C_String() )
        try
        {
            CAssetPipelineRequest request = MakeReprocess( CAssetPipelineOperations::ReprocessPlan, packId, true );
            request.arguments.SetAttribute( "operation", CAssetPipelineOperations::PackRegenerateIntermediate );
            CAssetServiceDocument plan;
            const UInt32 calls = hook.GetCallCount();
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.RunPipelineQuery( request, plan ) );
            ASSERT_TRUE( "true" == Text( plan.document, "wouldRun" ) && "false" == Text( plan.document, "rollbackAvailable" ) );
            ASSERT_TRUE( CAssetPipelineOperations::PackRegenerateIntermediate == Text( plan.document, "operation" ) );
            ASSERT_TRUE( Number( plan.document, "outputs/intermediateBytes" ) > 0 );
            ASSERT_TRUE( calls == hook.GetCallCount() );
            // a rollback is planned as refused
            CAssetPipelineRequest rollback = MakeReprocess( CAssetPipelineOperations::ReprocessPlan, packId );
            rollback.arguments.SetAttribute( "operation", CAssetPipelineOperations::ReprocessRollback );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.RunPipelineQuery( rollback, plan ) && "false" == Text( plan.document, "wouldRun" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": pack_regenerate_intermediate replaces the outputs in one swap, keeps the previous generation, reports before and after and writes the audit" ).C_String() )
        try
        {
            hook.SetVersion( 2 );
            CAssetJobEntry job;
            CAssetServiceDocument result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == RunJob( service, MakeReprocess( CAssetPipelineOperations::PackRegenerateIntermediate, packId ), job, result ) );
            ASSERT_TRUE( ASSETJOB_STATE_DONE == job.state && "reprocess" == job.kind );
            ASSERT_TRUE( hook.GetLastRequest().reconvert && !hook.GetLastRequest().cookOnly && hook.GetLastRequest().assetGlobs.empty() );
            ASSERT_TRUE( "converted SM_Rock_01 v2" == ReadText( intermediate + "/SM_Rock_01.gltf" ) && "cooked SM_Rock_01 v2" == ReadText( cooked + "/SM_Rock_01.mesh" ) );
            ASSERT_TRUE( "converted SM_Rock_01 v1" == ReadText( generations + "/previous/intermediate/SM_Rock_01.gltf" ) );
            ASSERT_TRUE( "cooked SM_Rock_01 v1" == ReadText( generations + "/previous/gucef-ogre3d/SM_Rock_01.mesh" ) );
            ASSERT_TRUE( vfs.FileExists( generations + "/previous/generation.json" ) && !vfs.FileExists( generations + "/swap.json" ) );
            ASSERT_TRUE( 2 == Number( result.document, "before/converted" ) && 2 == Number( result.document, "after/converted" ) );
            ASSERT_TRUE( "true" == Text( result.document, "rollbackAvailable" ) && !Text( result.document, "summary" ).IsNULLOrEmpty() );
            ASSERT_TRUE( "true" == Text( result.document, "intermediate/replaced" ) );
            ASSERT_TRUE( HistoryNames( service, packId, "reprocess:intermediate", "caller=test-owner" ) );
            // nothing is left in the staging area
            VFS::CVFS::TStringVector staged;
            vfs.GetDirList( staged, fixture.base + "/main/staging", false, false, CString::Empty );
            for ( size_t s=0; s<staged.size(); ++s )
                ASSERT_TRUE( staged[ s ].HasSubstr( "regen-" ) < 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": reprocess_rollback makes the previous generation current with its catalogue state, and a second rollback restores the regenerated one" ).C_String() )
        try
        {
            CAssetJobEntry job;
            CAssetServiceDocument result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == RunJob( service, MakeReprocess( CAssetPipelineOperations::ReprocessRollback, packId ), job, result ) );
            ASSERT_TRUE( ASSETJOB_STATE_DONE == job.state );
            ASSERT_TRUE( "converted SM_Rock_01 v1" == ReadText( intermediate + "/SM_Rock_01.gltf" ) && "cooked SM_Rock_01 v1" == ReadText( cooked + "/SM_Rock_01.mesh" ) );
            ASSERT_TRUE( "converted SM_Rock_01 v2" == ReadText( generations + "/previous/intermediate/SM_Rock_01.gltf" ) );
            TAssetRecordVector assets;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetAssets( packId, assets ) && GUCEF_NULL != FindAsset( assets, "SM_Rock_01" ) && !FindAsset( assets, "SM_Rock_01" )->convertedPath.IsNULLOrEmpty() );
            ASSERT_TRUE( 1 == CountHistory( service, packId, "reprocess:rollback" ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == RunJob( service, MakeReprocess( CAssetPipelineOperations::ReprocessRollback, packId ), job, result ) );
            ASSERT_TRUE( "converted SM_Rock_01 v2" == ReadText( intermediate + "/SM_Rock_01.gltf" ) && "converted SM_Rock_01 v1" == ReadText( generations + "/previous/intermediate/SM_Rock_01.gltf" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a regeneration that fails half way leaves the outputs, the previous generation and the catalogue as they were" ).C_String() )
        try
        {
            CAssetPackRecord before;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( packId, before ) );
            hook.SetVersion( 9 );
            hook.SetMode( CTestConverterHook::MODE_FAIL_MIDWAY );
            CAssetJobEntry job;
            CAssetServiceDocument result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == RunJob( service, MakeReprocess( CAssetPipelineOperations::PackRegenerateIntermediate, packId ), job, result ) );
            hook.SetMode( CTestConverterHook::MODE_CONVERT );
            ASSERT_TRUE( ASSETJOB_STATE_FAILED == job.state );
            ASSERT_TRUE( "converted SM_Rock_01 v2" == ReadText( intermediate + "/SM_Rock_01.gltf" ) && "converted SM_Rock_01 v1" == ReadText( generations + "/previous/intermediate/SM_Rock_01.gltf" ) );
            CAssetPackRecord after;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( packId, after ) && after.conversion.outcome == before.conversion.outcome && after.state == before.state );
            ASSERT_TRUE( "false" == Text( result.document, "intermediate/replaced" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": asset_regenerate_target cooks one asset again from the intermediate: the other asset keeps its cooked file" ).C_String() )
        try
        {
            hook.SetVersion( 3 );
            CAssetReprocessRequest spec;
            spec.operation = CAssetPipelineOperations::AssetRegenerateTarget;
            spec.packId = packId;
            spec.assets.push_back( "original/SM_Rock_01.fbx" );
            spec.targets.push_back( "ogre" );
            CAssetJobEntry job;
            CAssetServiceDocument result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == RunJob( service, MakeReprocess( spec ), job, result ) );
            ASSERT_TRUE( ASSETJOB_STATE_DONE == job.state );
            ASSERT_TRUE( hook.GetLastRequest().cookOnly && 1 == hook.GetLastRequest().assetGlobs.size() && "SM_Rock_01.fbx" == hook.GetLastRequest().assetGlobs[ 0 ] );
            ASSERT_TRUE( "cooked SM_Rock_01 v3" == ReadText( cooked + "/SM_Rock_01.mesh" ) && "cooked SM_Tree_01 v2" == ReadText( cooked + "/SM_Tree_01.mesh" ) );
            ASSERT_TRUE( "converted SM_Rock_01 v2" == ReadText( intermediate + "/SM_Rock_01.gltf" ) );
            // only the cooked area was replaced: the previous generation holds the cooked files before
            ASSERT_TRUE( "cooked SM_Rock_01 v2" == ReadText( generations + "/previous/gucef-ogre3d/SM_Rock_01.mesh" ) && !vfs.DirExists( generations + "/previous/intermediate" ) );
            // an asset form without assets is refused
            CAssetServiceResult refused;
            ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == service.StartPipelineJob( CString::Empty, MakeReprocess( CAssetPipelineOperations::AssetRegenerateTarget, packId ), refused ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a regeneration with a regressed asset is not put in place unless acceptErrors; the result names the regressed asset; onlyStatuses failed regenerates it alone later" ).C_String() )
        try
        {
            CString::StringSet failing;
            failing.insert( "SM_Tree_01" );
            hook.SetFailingAssets( failing );
            hook.SetVersion( 4 );
            CAssetJobEntry job;
            CAssetServiceDocument result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == RunJob( service, MakeReprocess( CAssetPipelineOperations::PackRegenerateIntermediate, packId ), job, result ) );
            ASSERT_TRUE( ASSETJOB_STATE_FAILED == job.state && 1 == Number( result.document, "changes/regressedCount" ) );
            ASSERT_TRUE( "converted SM_Rock_01 v2" == ReadText( intermediate + "/SM_Rock_01.gltf" ) );

            CAssetReprocessRequest accept;
            accept.operation = CAssetPipelineOperations::PackRegenerateIntermediate;
            accept.packId = packId;
            accept.acceptErrors = true;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == RunJob( service, MakeReprocess( accept ), job, result ) );
            ASSERT_TRUE( ASSETJOB_STATE_DONE == job.state && "converted SM_Rock_01 v4" == ReadText( intermediate + "/SM_Rock_01.gltf" ) );
            ASSERT_TRUE( 1 == Number( result.document, "after/failed" ) && 1 == Number( result.document, "changes/regressedCount" ) );
            TAssetRecordVector assets;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetAssets( packId, assets ) && FindAsset( assets, "SM_Tree_01" )->convertedPath.IsNULLOrEmpty() );

            // the importer "improved": only the failed asset runs, and it improves
            hook.SetFailingAssets( CString::StringSet() );
            hook.SetVersion( 5 );
            CAssetReprocessRequest failed;
            failed.operation = CAssetPipelineOperations::PackRegenerateIntermediate;
            failed.packId = packId;
            failed.onlyStatuses.insert( "failed" );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == RunJob( service, MakeReprocess( failed ), job, result ) );
            ASSERT_TRUE( ASSETJOB_STATE_DONE == job.state && 1 == hook.GetLastRequest().assetGlobs.size() && "SM_Tree_01.fbx" == hook.GetLastRequest().assetGlobs[ 0 ] );
            ASSERT_TRUE( 1 == Number( result.document, "changes/improvedCount" ) && 0 == Number( result.document, "after/failed" ) && 2 == Number( result.document, "after/converted" ) );
            ASSERT_TRUE( "converted SM_Tree_01 v5" == ReadText( intermediate + "/SM_Tree_01.gltf" ) && "converted SM_Rock_01 v4" == ReadText( intermediate + "/SM_Rock_01.gltf" ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetAssets( packId, assets ) && !FindAsset( assets, "SM_Tree_01" )->convertedPath.IsNULLOrEmpty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": one reprocessing of a pack at a time: a second one is refused as busy while the first runs, a cancel ends it" ).C_String() )
        try
        {
            hook.SetMode( CTestConverterHook::MODE_WAIT_FOR_CANCEL );
            const UInt32 waits = hook.GetStartedWaitCount();
            CAssetServiceResult first;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.StartPipelineJob( CString::Empty, MakeReprocess( CAssetPipelineOperations::PackRegenerateTarget, packId ), first ) );
            for ( UInt32 w=0; w<10000 && hook.GetStartedWaitCount() == waits; w+=10 )
                PumpFor( 10 );
            CAssetServiceResult second;
            ASSERT_TRUE( ASSETSTORE_STATUS_ALREADY_EXISTS == service.StartPipelineJob( CString::Empty, MakeReprocess( CAssetPipelineOperations::PackRegenerateIntermediate, packId ), second ) );
            ASSERT_TRUE( second.id == first.id && second.message.HasSubstr( "busy" ) >= 0 );
            CAssetServiceResult cancelled;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.ControlJob( CString::Empty, first.id, ASSETJOB_ACTION_CANCEL, cancelled ) );
            CAssetJobEntry job;
            ASSERT_TRUE( WaitForJob( service, first.id, job ) && ASSETJOB_STATE_CANCELLED == job.state );
            hook.SetMode( CTestConverterHook::MODE_CONVERT );
            ASSERT_TRUE( "converted SM_Rock_01 v4" == ReadText( intermediate + "/SM_Rock_01.gltf" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a machine caller is held to maxJobsPerPackPerHour; the user is not; the plan says so" ).C_String() )
        try
        {
            CAssetJobEntry job;
            CAssetServiceDocument result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == RunJob( service, MakeReprocess( CAssetPipelineOperations::PackRegenerateTarget, packId, true ), job, result ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == RunJob( service, MakeReprocess( CAssetPipelineOperations::PackRegenerateTarget, packId, true ), job, result ) );
            CAssetServiceResult refused;
            ASSERT_TRUE( ASSETSTORE_STATUS_RATE_LIMITED == service.StartPipelineJob( CString::Empty, MakeReprocess( CAssetPipelineOperations::PackRegenerateTarget, packId, true ), refused ) );
            ASSERT_TRUE( refused.message.HasSubstr( "maxJobsPerPackPerHour" ) >= 0 );
            CAssetPipelineRequest plan = MakeReprocess( CAssetPipelineOperations::ReprocessPlan, packId, true );
            plan.arguments.SetAttribute( "operation", CAssetPipelineOperations::PackRegenerateTarget );
            CAssetServiceDocument planned;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.RunPipelineQuery( plan, planned ) && "false" == Text( planned.document, "wouldRun" ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == RunJob( service, MakeReprocess( CAssetPipelineOperations::PackRegenerateTarget, packId, false ), job, result ) );
            ASSERT_TRUE( ASSETJOB_STATE_DONE == job.state );
            ASSERT_TRUE( HistoryNames( service, packId, "reprocess:target", "mcp:test-session (machine)" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": pack_redownload_original of the same files is verified identical: no new pack; a damaged original file is put back" ).C_String() )
        try
        {
            ASSERT_TRUE( WriteVfsFile( originalRock, "damaged" ) );
            CAssetJobEntry job;
            CAssetServiceDocument result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == RunJob( service, MakeReprocess( CAssetPipelineOperations::PackRedownloadOriginal, packId ), job, result ) );
            ASSERT_TRUE( ASSETJOB_STATE_DONE == job.state && "verified-identical" == Text( result.document, "redownload/outcome" ) );
            ASSERT_TRUE( packId == CurrentPackOfItem( store ) );
            ASSERT_TRUE( "content of SM_Rock_01.fbx seed 1" == ReadText( originalRock ) );
            CString::StringVector repaired;
            const CDataNode* node = result.document.FindChild( "redownload" );
            ASSERT_TRUE( GUCEF_NULL != node && node->GetValuesOfChildByName( "repairedFiles", repaired ) && 1 == repaired.size() );
            ASSERT_TRUE( 1 == CountHistory( service, packId, "reprocess:redownload" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": pack_redownload_original of changed files makes a new pack, keeps the old one as superseded, and converts the new one" ).C_String() )
        try
        {
            AddReproItem( world.server, 2, false );
            CAssetJobEntry job;
            CAssetServiceDocument result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == RunJob( service, MakeReprocess( CAssetPipelineOperations::PackRedownloadOriginal, packId ), job, result ) );
            ASSERT_TRUE( ASSETJOB_STATE_DONE == job.state && "new-pack" == Text( result.document, "redownload/outcome" ) );
            const CString newPackId = Text( result.document, "newPackId" );
            ASSERT_TRUE( !newPackId.IsNULLOrEmpty() && newPackId != packId && newPackId == CurrentPackOfItem( store ) );
            CAssetPackSourceRecord facts;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetSyncStore().GetPackSource( packId, facts ) && facts.supersededBy == newPackId );
            CAssetPackRecord oldPack;
            CAssetPackRecord newPack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( packId, oldPack ) && ASSETSTORE_STATUS_OK == store.GetPack( newPackId, newPack ) );
            ASSERT_TRUE( CAssetConversionRecord::OutcomeConverted == newPack.conversion.outcome );
            ASSERT_TRUE( vfs.FileExists( originalRock ) );
            // the superseded pack is not downloaded again
            CAssetServiceDocument again;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == RunJob( service, MakeReprocess( CAssetPipelineOperations::PackRedownloadOriginal, packId ), job, again ) );
            ASSERT_TRUE( ASSETJOB_STATE_FAILED == job.state && "superseded" == Text( again.document, "redownload/outcome" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a pack from a folder has no store source: it cannot be downloaded again, and the plan says so" ).C_String() )
        try
        {
            const CString folder = fixture.base + "/incoming/Disk Pack";
            ASSERT_TRUE( WriteVfsFile( folder + "/SM_Disk_01.fbx", "a disk model" ) );
            CAssetIngestRequest ingest;
            ingest.sourceId = "disk";
            ingest.contentPath = folder;
            CAssetIngestResult ingested;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.Ingest( CString::Empty, ingest, ingested ) );
            CAssetJobEntry job;
            CAssetServiceDocument result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == RunJob( service, MakeReprocess( CAssetPipelineOperations::PackRedownloadOriginal, ingested.packId ), job, result ) );
            ASSERT_TRUE( ASSETJOB_STATE_FAILED == job.state && "no-store-source" == Text( result.document, "redownload/outcome" ) );
            ASSERT_TRUE( Text( result.document, "summary" ).HasSubstr( "no store source" ) >= 0 );
            CAssetPipelineRequest plan = MakeReprocess( CAssetPipelineOperations::ReprocessPlan, ingested.packId );
            plan.arguments.SetAttribute( "operation", CAssetPipelineOperations::PackRedownloadOriginal );
            CAssetServiceDocument planned;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.RunPipelineQuery( plan, planned ) && "false" == Text( planned.document, "wouldRun" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a store that is not signed in: the download again ends with the sign in needed state, not as a failure, and nothing is downloaded" ).C_String() )
        try
        {
            CFakeSignInStoreProviderFactory signInFactory( "reprosignin", false );
            CAssetStoreGlobal::Instance()->GetProviderFactory().RegisterConcreteFactory( "reprosignin", &signInFactory );
            CAssetStoreGlobal::Instance()->GetRegistry().RefreshProviders();
            PumpFor( 50 );
            const CString folder = fixture.base + "/incoming/Signed Pack";
            ASSERT_TRUE( WriteVfsFile( folder + "/SM_Signed_01.fbx", "a model of a store" ) );
            CAssetIngestRequest ingest;
            ingest.sourceId = "disk";
            ingest.contentPath = folder;
            CAssetIngestResult ingested;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.Ingest( CString::Empty, ingest, ingested ) );
            CAssetPackSourceRecord facts;
            facts.packId = ingested.packId;
            facts.sourceId = "reprosignin";
            facts.sourceItemId = "signed-item";
            facts.sourceFormatId = "zip";
            facts.revision = "unk:";
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetSyncStore().WritePackSource( facts ) );
            CAssetPipelineRequest plan = MakeReprocess( CAssetPipelineOperations::ReprocessPlan, ingested.packId );
            plan.arguments.SetAttribute( "operation", CAssetPipelineOperations::PackRedownloadOriginal );
            CAssetServiceDocument planned;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.RunPipelineQuery( plan, planned ) && "true" == Text( planned.document, "redownload/signInNeeded" ) && "false" == Text( planned.document, "wouldRun" ) );
            CAssetJobEntry job;
            CAssetServiceDocument result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == RunJob( service, MakeReprocess( CAssetPipelineOperations::PackRedownloadOriginal, ingested.packId ), job, result ) );
            ASSERT_TRUE( ASSETJOB_STATE_DONE == job.state && ASSETSTORE_STATUS_NOT_SIGNED_IN == job.status && "sign-in-needed" == job.stateText );
            ASSERT_TRUE( "sign-in-needed" == Text( result.document, "redownload/outcome" ) );
            CAssetStoreGlobal::Instance()->GetRegistry().RemoveProvider( "reprosignin" );
            CAssetStoreGlobal::Instance()->GetProviderFactory().UnregisterConcreteFactory( "reprosignin" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
    service.Close();
}

/*-------------------------------------------------------------------------*/

void
RunReprocessRemoteSuite( SReprocessFixture& fixture )
{
    MountFreshDirectory( fixture.label + "_main", fixture.base + "/main" );
    CReprocessStoreWorld storeWorld;
    CTestConverterHook hook;
    CRemoteAssetStoreWorld world( "reprocess_" + fixture.label );
    world.bindingSettings.roles.insert( "mcp" );
    world.bindingSettings.roles.insert( "viewer" );
    CString error;
    const bool started = world.StartServer( MakeReprocessConfig( fixture, hook ), error );
    TAssetStoreServicePtr ownerPtr;
    TAssetStoreServicePtr mcpPtr;
    TAssetStoreServicePtr viewerPtr;
    if ( started )
    {
        ownerPtr = world.MakeClient( "rp-owner", error );
        CString::StringSet mcpRoles;
        mcpRoles.insert( "mcp" );
        mcpPtr = world.MakeClient( "rp-mcp", error, &mcpRoles, false );
        CString::StringSet viewerRoles;
        viewerRoles.insert( "viewer" );
        viewerPtr = world.MakeClient( "rp-viewer", error, &viewerRoles, false );
    }
    const bool connected = started && !ownerPtr.IsNULL() && !mcpPtr.IsNULL() && !viewerPtr.IsNULL() &&
                           ASSETSTORE_STATUS_OK == ownerPtr->Open() && ASSETSTORE_STATUS_OK == mcpPtr->Open() && ASSETSTORE_STATUS_OK == viewerPtr->Open();
    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": remote: the server and an owner, a machine (mcp) and a viewer device connect" ).C_String() )
        ASSERT_TRUE( connected );
    GUCEF_TESTFW_TESTCASE_END;
    if ( !connected )
    {
        world.StopServer();
        return;
    }
    CIAssetStoreService& server = *world.serverService;
    CPrimaryAssetStore& store = *static_cast< CLocalAssetStoreService& >( server ).GetPrimaryStore();
    CString packId;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": remote: the owner device syncs the item and regenerates its intermediate over the wire, the audit names the device" ).C_String() )
        try
        {
            CAssetLibrarySyncProfile profile;
            profile.reservePercent = 0;
            profile.reserveBytes = 0;
            profile.indexAfter = false;
            CAssetServiceResult set;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == ownerPtr->SetSyncProfile( CString::Empty, ReproStoreId, profile, set ) );
            CAssetLibrarySyncSpec spec;
            spec.storeId = ReproStoreId;
            spec.useStoreProfile = true;
            spec.itemIds.push_back( ReproItemId );
            CAssetPipelineRequest request;
            spec.ToRequest( request );
            CAssetJobEntry job;
            CAssetServiceDocument result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == RunJob( *ownerPtr, request, job, result ) );
            packId = CurrentPackOfItem( store );
            ASSERT_TRUE( !packId.IsNULLOrEmpty() );
            hook.SetVersion( 2 );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == RunJob( *ownerPtr, MakeReprocess( CAssetPipelineOperations::PackRegenerateIntermediate, packId ), job, result ) );
            ASSERT_TRUE( ASSETJOB_STATE_DONE == job.state && "true" == Text( result.document, "rollbackAvailable" ) && 2 == Number( result.document, "after/converted" ) );
            ASSERT_TRUE( HistoryNames( server, packId, "reprocess:intermediate", "caller=device:" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": remote: a machine device may plan, regenerate and roll back, is held to the limits whatever it says of itself; a viewer device is refused" ).C_String() )
        try
        {
            ASSERT_TRUE( !packId.IsNULLOrEmpty() );
            CAssetPipelineRequest plan = MakeReprocess( CAssetPipelineOperations::ReprocessPlan, packId, false );
            plan.arguments.SetAttribute( "operation", CAssetPipelineOperations::ReprocessRollback );
            CAssetServiceDocument planned;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == mcpPtr->RunPipelineQuery( plan, planned ) && "true" == Text( planned.document, "wouldRun" ) );
            CAssetJobEntry job;
            CAssetServiceDocument result;
            // it says it is no machine: the host makes it one
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == RunJob( *mcpPtr, MakeReprocess( CAssetPipelineOperations::ReprocessRollback, packId, false ), job, result ) );
            ASSERT_TRUE( ASSETJOB_STATE_DONE == job.state );
            ASSERT_TRUE( HistoryNames( server, packId, "reprocess:rollback", "(machine)" ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == RunJob( *mcpPtr, MakeReprocess( CAssetPipelineOperations::PackRegenerateTarget, packId, false ), job, result ) );
            CAssetServiceResult refused;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK != mcpPtr->StartPipelineJob( CString::Empty, MakeReprocess( CAssetPipelineOperations::PackRegenerateTarget, packId, false ), refused ) );
            ASSERT_TRUE( refused.message.HasSubstr( "maxJobsPerPackPerHour" ) >= 0 );
            CAssetServiceResult viewerRefused;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK != viewerPtr->StartPipelineJob( CString::Empty, MakeReprocess( CAssetPipelineOperations::PackRegenerateTarget, packId ), viewerRefused ) );
            CAssetServiceDocument viewerPlan;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK != viewerPtr->RunPipelineQuery( plan, viewerPlan ) );
            // a library sync run stays the user's
            CAssetLibrarySyncSpec spec;
            spec.storeId = ReproStoreId;
            spec.useStoreProfile = true;
            CAssetPipelineRequest sync;
            spec.ToRequest( sync );
            CAssetServiceResult syncRefused;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK != mcpPtr->StartPipelineJob( CString::Empty, sync, syncRefused ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
    ownerPtr->Close();
    mcpPtr->Close();
    viewerPtr->Close();
    world.StopServer();
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformReprocessVfsTablesTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "ReprocessVfsTables" );

    SReprocessFixture fixture;
    fixture.label = "rp_vfstables";
    fixture.base = "astest/rp_vfstables";
    MountFreshDirectory( "rp_vfstables_catalogue", "astest/rp_vfstables/catalogue" );
    fixture.database.backendName = "vfstables";
    fixture.database.database = "astest/rp_vfstables/catalogue/db";
    fixture.database.backendOptions.SetAllowMultipleValues( false );
    fixture.database.backendOptions.Set( "syncWrites", CVariant( "false" ) );
    RunReprocessSuite( fixture );
    UnmountDirectory( "rp_vfstables_catalogue" );
    UnmountDirectory( "rp_vfstables_main" );
    UnmountDirectory( "rp_vfstables_incoming" );

    SReprocessFixture remote( fixture );
    remote.label = "rp_vfstables_remote";
    remote.base = "astest/rp_vfstables_remote";
    MountFreshDirectory( "rp_vfstables_remote_catalogue", "astest/rp_vfstables_remote/catalogue" );
    remote.database.database = "astest/rp_vfstables_remote/catalogue/db";
    RunReprocessRemoteSuite( remote );
    UnmountDirectory( "rp_vfstables_remote_catalogue" );
    UnmountDirectory( "rp_vfstables_remote_main" );
}

/*-------------------------------------------------------------------------*/

void
PerformReprocessPostgreSqlTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "ReprocessPostgreSql" );

    CString connectionString;
    if ( !GetPostgreSqlConnectionString( connectionString ) )
    {
        GUCEF_LOG( LOGLEVEL_IMPORTANT, "ReprocessPostgreSql: GUCEF_TEST_POSTGRESQL is not set, the PostgreSQL tests are skipped" );
        GUCEF_TESTFW_TESTCASE( "PostgreSQL: the reprocessing against a PostgreSQL test database" )
            GUCEF_TESTFW_SKIP( "GUCEF_TEST_POSTGRESQL is not set" );
        GUCEF_TESTFW_TESTCASE_END;
        return;
    }
    CString reachReason;
    if ( !IsPostgreSqlReachable( connectionString, reachReason ) )
    {
        GUCEF_TESTFW_TESTCASE( "PostgreSQL: the test database can be reached" )
            ASSERT_TRUE( reachReason.IsNULLOrEmpty() );
        GUCEF_TESTFW_TESTCASE_END;
        return;
    }
    AUTH::CMemoryCredentialStore credentials;
    const char* password = getenv( "PGPASSWORD" );
    const char* const labels[] = { "rp_postgresql", "rp_postgresql_remote" };
    for ( UInt32 run=0; run<2; ++run )
    {
        SReprocessFixture fixture;
        fixture.label = labels[ run ];
        fixture.base = CString( "astest/" ) + labels[ run ];
        const CString schema = MakeTestSchemaName();
        fixture.database.backendName = "postgresql";
        CValueList parsed;
        CString error;
        if ( DBPLUGIN::POSTGRESQL::CPostgreSqlSettings::ParseConnectionString( connectionString, parsed, error ) )
            fixture.database.database = parsed.GetValueAlways( CString( "dbname" ) ).AsString();
        fixture.database.backendOptions.SetAllowMultipleValues( false );
        fixture.database.backendOptions.Set( "connectionString", CVariant( connectionString ) );
        fixture.database.backendOptions.Set( "schema", CVariant( schema ) );
        fixture.database.backendOptions.Set( "createSchema", CVariant( "true" ) );
        fixture.database.backendOptions.Set( "applicationName", CVariant( "gucefASSETSTORE_TestApp" ) );
        fixture.database.backendOptions.Set( "connectTimeout", CVariant( "10" ) );
        if ( GUCEF_NULL != password && '\0' != password[ 0 ] )
        {
            AUTH::CSecretString secret( ( CString( password ) ) );
            credentials.Store( "gucefASSETSTORE.test", "catalogue", secret );
            fixture.credentials = &credentials;
        }
        if ( 0 == run )
            RunReprocessSuite( fixture );
        else
            RunReprocessRemoteSuite( fixture );

        // the schema of the run goes
        CPrimaryAssetStoreSettings settings;
        settings.database = fixture.database;
        if ( GUCEF_NULL != fixture.credentials )
        {
            settings.credentialProvider = "gucefASSETSTORE.test";
            settings.credentialAccount = "catalogue";
        }
        COMCORE::CDatabaseClientSettings databaseSettings;
        if ( settings.GetDatabaseSettingsWithCredentials( fixture.credentials, databaseSettings, error ) )
        {
            COMCORE::CIDatabaseClientPtr client = COMCORE::CComCoreGlobal::Instance()->GetDatabaseClientFactory().Create( "postgresql" );
            if ( !client.IsNULL() && client->Open( databaseSettings ) )
            {
                DBPLUGIN::POSTGRESQL::CPostgreSqlConnectionPtr connection = static_cast< DBPLUGIN::POSTGRESQL::CPostgreSqlClient* >( client.GetPointerAlways() )->GetConnection();
                if ( !connection.IsNULL() )
                {
                    MT::CScopeMutex lock( connection->GetLock(), GUCEF_MT_INFINITE_LOCK_TIMEOUT );
                    CString dropError;
                    connection->Execute( "DROP SCHEMA " + DBPLUGIN::POSTGRESQL::CPostgreSqlSql::QuoteIdentifier( schema ) + " CASCADE", dropError );
                }
                client->Close();
            }
        }
        UnmountDirectory( CString( labels[ run ] ) + "_main" );
        UnmountDirectory( CString( labels[ run ] ) + "_incoming" );
    }
}

/*-------------------------------------------------------------------------*/
