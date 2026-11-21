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
#include <string.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#include "gucefAUTH_CMemoryCredentialStore.h"
#define GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#endif /* GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H ? */

#ifndef GUCEF_COM_CJSONRPCENDPOINT_H
#include "gucefCOM_CJsonRpcEndpoint.h"
#define GUCEF_COM_CJSONRPCENDPOINT_H
#endif /* GUCEF_COM_CJSONRPCENDPOINT_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSERVICEOPERATIONS_H
#include "gucefASSETSTORE_CAssetServiceOperations.h"
#define GUCEF_ASSETSTORE_CASSETSERVICEOPERATIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETSERVICEOPERATIONS_H ? */

#ifndef GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#include "gucefASSETSTORE_CLocalAssetStoreService.h"
#define GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H ? */

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

#ifndef TESTASSETSTORESERVICESUITE_H
#include "TestAssetStoreServiceSuite.h"
#define TESTASSETSTORESERVICESUITE_H
#endif /* TESTASSETSTORESERVICESUITE_H ? */

#ifndef TESTFAKEASSETSTORESERVICE_H
#include "TestFakeAssetStoreService.h"
#define TESTFAKEASSETSTORESERVICE_H
#endif /* TESTFAKEASSETSTORESERVICE_H ? */

#include "TestAssetStoreService.h"

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

/** The in-process service as the module's factory makes it, on the database the run gives it */
class CLocalServiceTarget : public CAssetStoreServiceSuiteTarget
{
    public:

    CString label;
    COMCORE::CDatabaseClientSettings database;
    AUTH::ICredentialStore* credentials;

    CLocalServiceTarget( void ) : label(), database(), credentials( GUCEF_NULL ) {}

    virtual CString GetLabel( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return label;
    }

    virtual void ConfigureDatabase( CAssetStoreServiceConfig& config ) GUCEF_VIRTUAL_OVERRIDE
    {
        config.primaryStore.database = database;
        config.credentialStore = credentials;
        if ( GUCEF_NULL != credentials )
        {
            config.primaryStore.credentialProvider = "gucefASSETSTORE.test";
            config.primaryStore.credentialAccount = "catalogue";
        }
    }

    virtual TAssetStoreServicePtr MakeService( const CAssetStoreServiceConfig& config ) GUCEF_VIRTUAL_OVERRIDE
    {
        CString error;
        return CAssetStoreGlobal::Instance()->CreateService( config, error );
    }

    virtual void Pump( UInt32 durationInMs ) GUCEF_VIRTUAL_OVERRIDE
    {
        PumpFor( durationInMs );
    }
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAssetStoreServiceUnitTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "AssetStoreServiceSeam" );

    GUCEF_TESTFW_TESTCASE( "Service seam: a call carries its arguments through a DataNode and back, a page through its serialised form" )
        try
        {
            CAssetServiceCall call( CAssetServiceOperations::GetAssets, "cmd-7" );
            call.SetText( "packId", "pk1" );
            call.SetUInt32( "maxEntries", 25 );
            call.SetUInt64( "sinceRevision", 1234567890123ULL );
            call.SetBool( "deleteFiles", true );
            CString::StringVector ids;
            ids.push_back( "a" );
            ids.push_back( "b" );
            call.SetTexts( "assetIds", ids );
            CAssetSearchQuery query;
            query.text = "barrel";
            query.kinds.insert( "model" );
            query.minTriangles = 10;
            call.SetRecord( "query", query );
            CDataNode node;
            CDataNodeSerializableSettings settings;
            ASSERT_TRUE( call.Serialize( node, settings ) );
            CAssetServiceCall back;
            ASSERT_TRUE( back.Deserialize( node, settings ) );
            ASSERT_TRUE( back.operation == CAssetServiceOperations::GetAssets && "cmd-7" == back.commandId && "pk1" == back.GetText( "packId" ) );
            ASSERT_TRUE( 25 == back.GetUInt32( "maxEntries" ) && 1234567890123ULL == back.GetUInt64( "sinceRevision" ) && back.GetBool( "deleteFiles" ) );
            ASSERT_TRUE( ids == back.GetTexts( "assetIds" ) );
            CAssetSearchQuery queryBack;
            ASSERT_TRUE( back.GetRecord( "query", queryBack ) && "barrel" == queryBack.text && 1 == queryBack.kinds.count( "model" ) && 10 == queryBack.minTriangles );
            ASSERT_FALSE( back.GetRecord( "missing", queryBack ) );

            CAssetJobPage page;
            CAssetJobEntry job;
            job.jobId = "ingest:x";
            job.kind = "ingest";
            job.state = ASSETJOB_STATE_FAILED;
            job.SetActionAvailable( ASSETJOB_ACTION_RETRY, true );
            page.entries.push_back( job );
            page.total = 1;
            page.revision = 9;
            page.nextPageToken = "1";
            CDataNode pageNode;
            ASSERT_TRUE( page.Serialize( pageNode, settings ) );
            CAssetJobPage pageBack;
            ASSERT_TRUE( pageBack.Deserialize( pageNode, settings ) && 1 == pageBack.entries.size() && 9 == pageBack.revision && "1" == pageBack.nextPageToken );
            ASSERT_TRUE( pageBack.entries[ 0 ].IsEnded() && pageBack.entries[ 0 ].IsActionAvailable( ASSETJOB_ACTION_RETRY ) && !pageBack.entries[ 0 ].IsActionAvailable( ASSETJOB_ACTION_REMOVE ) );
            CICloneable* clone = page.Clone();
            ASSERT_TRUE( GUCEF_NULL != CICloneable::CloneableAs< CAssetJobPage >( clone ) && GUCEF_NULL == CICloneable::CloneableAs< CAssetRecordPage >( clone ) );
            GUCEF_DELETE clone;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Service seam: pages are bounded, futures of answers end at once with the result or the status, the names of the operations are known" )
        try
        {
            ASSERT_TRUE( CIAssetStoreService::DefaultPageEntries == CIAssetStoreService::ClampPageEntries( 0 ) );
            ASSERT_TRUE( CIAssetStoreService::MaxPageEntries == CIAssetStoreService::ClampPageEntries( 1000000 ) );
            GUCEF::vector< CAssetJobEntry > all;
            for ( UInt32 i=0; i<5; ++i )
            {
                CAssetJobEntry entry;
                entry.jobId = "j" + ToString( i );
                all.push_back( entry );
            }
            CAssetJobPage first;
            CIAssetStoreService::FillPage( all, CString::Empty, 2, first );
            ASSERT_TRUE( 2 == first.entries.size() && 5 == first.total && "2" == first.nextPageToken );
            CAssetJobPage last;
            CIAssetStoreService::FillPage( all, "4", 2, last );
            ASSERT_TRUE( 1 == last.entries.size() && "j4" == last.entries[ 0 ].jobId && last.nextPageToken.IsNULLOrEmpty() );

            CAssetServiceResult* ok = GUCEF_NEW CAssetServiceResult();
            ok->id = "x";
            CFutureResult done = CIAssetStoreService::MakeEndedFuture( ASSETSTORE_STATUS_OK, ok, CString::Empty );
            CAssetServiceResult outcome;
            ASSERT_TRUE( done.IsReady() && CIAssetStoreService::TryGetFutureResult( done, outcome ) && "x" == outcome.id );
            CFutureResult failed = CIAssetStoreService::MakeEndedFuture( ASSETSTORE_STATUS_NOT_FOUND, GUCEF_NULL, "the pack is gone" );
            ASSERT_TRUE( failed.IsReady() && ASSETSTORE_STATUS_NOT_FOUND == CIAssetStoreService::GetStatusOfFuture( failed ) && "the pack is gone" == CIAssetStoreService::GetMessageOfFuture( failed ) );
            ASSERT_TRUE( CIAssetStoreService::IsOutcomeStatus( CAssetServiceOperations::Ingest, ASSETSTORE_STATUS_DUPLICATE ) );
            ASSERT_FALSE( CIAssetStoreService::IsOutcomeStatus( CAssetServiceOperations::Search, ASSETSTORE_STATUS_DUPLICATE ) );
            ASSERT_TRUE( CAssetServiceOperations::IsKnown( "Catalogue/Search" ) && !CAssetServiceOperations::IsKnown( "Catalogue/Nothing" ) );
            ASSERT_TRUE( CAssetServiceOperations::IsMutation( CAssetServiceOperations::ControlJob ) && !CAssetServiceOperations::IsMutation( CAssetServiceOperations::ListJobs ) );
            ASSERT_TRUE( CAssetPipelineOperations::IsJob( "convert_pack" ) && CAssetPipelineOperations::IsWrite( "manifest_edit" ) && CAssetPipelineOperations::IsKnown( "pack_list" ) );
            TAssetJobAction action = ASSETJOB_ACTION_PAUSE;
            ASSERT_TRUE( AssetJobActionFromString( "Retry", action ) && ASSETJOB_ACTION_RETRY == action && !AssetJobActionFromString( "jump", action ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Service seam: the fake service records calls with their arguments, answers what was scripted, defers futures when asked" )
        try
        {
            CFakeAssetStoreService fake;
            CAssetPackRecord pack;
            pack.packId = "pk1";
            pack.title = "Fake Pack";
            fake.SetAnswer( CAssetServiceOperations::GetPack, ASSETSTORE_STATUS_OK, &pack );
            CAssetPackRecord answered;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == fake.GetPack( "pk1", answered ) && "Fake Pack" == answered.title );
            CAssetServiceCall call;
            ASSERT_TRUE( 1 == fake.CountCalls( CAssetServiceOperations::GetPack ) && fake.GetLastCall( CAssetServiceOperations::GetPack, call ) && "pk1" == call.GetText( "packId" ) );
            CAssetRecord asset;
            ASSERT_TRUE( ASSETSTORE_STATUS_UNSUPPORTED == fake.GetAsset( "a1", asset ) );

            fake.SetDeferAsync( true );
            CFutureResult future = fake.GetPackAsync( "pk1" );
            ASSERT_TRUE( !future.IsReady() && 1 == fake.GetPendingCallCount() );
            ASSERT_TRUE( 1 == fake.CompletePendingCalls() && future.IsReady() );
            CAssetPackRecord deferred;
            ASSERT_TRUE( CIAssetStoreService::TryGetFutureResult( future, deferred ) && "pk1" == deferred.packId && 2 == fake.CountCalls( CAssetServiceOperations::GetPack ) );

            CAssetServiceResult refused;
            refused.message = "no";
            fake.SetAnswer( CAssetServiceOperations::ControlJob, ASSETSTORE_STATUS_INVALID_ARGUMENT, &refused );
            CFutureResult control = fake.ControlJobAsync( "c1", "job1", ASSETJOB_ACTION_CANCEL );
            fake.CompletePendingCalls();
            ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == CIAssetStoreService::GetStatusOfFuture( control ) && "no" == CIAssetStoreService::GetMessageOfFuture( control ) );
            ASSERT_TRUE( fake.GetLastCall( CAssetServiceOperations::ControlJob, call ) && "c1" == call.commandId && "cancel" == call.GetText( "action" ) );
            CAssetServiceResult secret;
            fake.UnlockKeyring( "k1", AUTH::CSecretString( CString( "hidden-code" ) ), secret );
            ASSERT_TRUE( fake.GetLastCall( CAssetServiceOperations::UnlockKeyring, call ) && 0 == call.args.GetAttCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Service settings: the folder of the thumbnails is empty by default (the storage location), set by the key thumbnailsDirectory and kept by the serialised form" )
        try
        {
            CAssetStoreServiceConfig defaults;
            ASSERT_TRUE( defaults.thumbnailsDirectory.IsNULLOrEmpty() );
            CDataNode tree;
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ParseJsonText( "{ \"AssetStoreService\": { \"thumbnailsDirectory\": \"/data/thumbnails\" } }", tree ) );
            const CDataNode* section = tree.FindChild( "AssetStoreService" );
            ASSERT_TRUE( GUCEF_NULL != section );
            CDataNodeSerializableSettings serializerSettings;
            CAssetStoreServiceConfig config;
            ASSERT_TRUE( config.Deserialize( *section, serializerSettings ) && "/data/thumbnails" == config.thumbnailsDirectory );
            // a section without the key leaves the default as it is
            CDataNode withoutKey;
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ParseJsonText( "{ \"AssetStoreService\": { \"resumeIngestsOnStart\": true } }", withoutKey ) );
            CAssetStoreServiceConfig plain;
            ASSERT_TRUE( GUCEF_NULL != withoutKey.FindChild( "AssetStoreService" ) && plain.Deserialize( *withoutKey.FindChild( "AssetStoreService" ), serializerSettings ) && plain.thumbnailsDirectory.IsNULLOrEmpty() );
            // the settings survive their own serialised form
            CDataNode saved( "AssetStoreService", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( config.Serialize( saved, serializerSettings ) );
            CAssetStoreServiceConfig again;
            ASSERT_TRUE( again.Deserialize( saved, serializerSettings ) && "/data/thumbnails" == again.thumbnailsDirectory );
            // the mount point ends in the folder name that the licence gate recognises as a picture
            ASSERT_TRUE( CAssetStoreServiceConfig::ThumbnailsVfsRoot.HasSubstr( "/thumbnails" ) >= 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Service settings: the section of the site file sets the automatic policy of a source, the state folder, the queue file, the resume of the ingests and the index after an ingest" )
        try
        {
            CDataNode tree;
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ParseJsonText(
                "{ \"AssetStoreService\": { \"stateDirectory\": \"/data/state/assetstore\", \"resumeIngestsOnStart\": false, \"ingestResumeAttempts\": 5,\n"
                "    \"downloads\": { \"queueFilePath\": \"assetstore-state/downloads.json\", \"autoSaveQueue\": true },\n"
                "    \"primaryStore\": { \"indexAfterIngest\": true, \"sources\": [ { \"sourceId\": \"ExampleStore\", \"mode\": \"automatic\", \"keep\": \"original\" }, { \"sourceId\": \"other\", \"mode\": \"manual\" } ] } } }", tree ) );
            const CDataNode* section = tree.FindChild( "AssetStoreService" );
            ASSERT_TRUE( GUCEF_NULL != section );
            CAssetStoreServiceConfig defaults;
            ASSERT_TRUE( defaults.stateDirectory.IsNULLOrEmpty() && defaults.resumeIngestsOnStart && 3 == defaults.ingestResumeAttempts && defaults.downloads.queueFilePath.IsNULLOrEmpty() && !defaults.primaryStore.indexAfterIngest );
            CAssetStoreServiceConfig config;
            CDataNodeSerializableSettings serializerSettings;
            ASSERT_TRUE( GUCEF_NULL != section && config.Deserialize( *section, serializerSettings ) );
            ASSERT_TRUE( "/data/state/assetstore" == config.stateDirectory && !config.resumeIngestsOnStart && 5 == config.ingestResumeAttempts );
            ASSERT_TRUE( "assetstore-state/downloads.json" == config.downloads.queueFilePath && config.primaryStore.indexAfterIngest );
            // the policy of a source, by its id in any letter case; a source without one is manual
            ASSERT_TRUE( ASSETINGEST_MODE_AUTOMATIC == config.primaryStore.GetPolicy( "examplestore" ).mode && ASSETKEEP_ORIGINAL == config.primaryStore.GetPolicy( "examplestore" ).keep );
            ASSERT_TRUE( ASSETINGEST_MODE_MANUAL == config.primaryStore.GetPolicy( "other" ).mode && ASSETINGEST_MODE_MANUAL == config.primaryStore.GetPolicy( "unlisted" ).mode );
            // the settings survive their own serialised form
            CDataNode saved( "AssetStoreService", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( config.Serialize( saved, serializerSettings ) );
            CAssetStoreServiceConfig again;
            ASSERT_TRUE( again.Deserialize( saved, serializerSettings ) && again.stateDirectory == config.stateDirectory && 5 == again.ingestResumeAttempts && again.primaryStore.indexAfterIngest );
            ASSERT_TRUE( ASSETINGEST_MODE_AUTOMATIC == again.primaryStore.GetPolicy( "examplestore" ).mode );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Service seam: the factory of the module makes the local implementation by id; a service without a catalogue says why and still lists its stores" )
        try
        {
            CAssetStoreServiceFactory& factory = CAssetStoreGlobal::Instance()->GetServiceFactory();
            ASSERT_TRUE( factory.IsConstructible( CAssetStoreServiceConfig::LocalImplementationId ) && factory.IsConstructible( CAssetStoreServiceConfig::RemoteImplementationId ) );
            CAssetStoreServiceConfig config;
            config.implementationId.Clear();
            config.primaryStore.database.backendName = "nosuchbackend";
            config.loadDownloadQueue = false;
            config.startDownloads = false;
            CString error;
            TAssetStoreServicePtr service = CAssetStoreGlobal::Instance()->CreateService( config, error );
            ASSERT_TRUE( !service.IsNULL() && CAssetStoreServiceConfig::LocalImplementationId == service->GetType() && service->IsInProcess() );
            if ( !service.IsNULL() )
            {
                ASSERT_TRUE( ASSETSTORE_STATUS_OK != service->Open() && service->IsOpen() && !service->IsCatalogueAvailable() );
                ASSERT_FALSE( service->GetStatusMessage().IsNULLOrEmpty() );
                CAssetSearchResultPage page;
                ASSERT_TRUE( ASSETSTORE_STATUS_UNAVAILABLE == service->Search( CAssetSearchQuery(), page ) );
                CAssetStoreInfoPage stores;
                CAssetJobPage jobs;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == service->ListStores( stores ) && ASSETSTORE_STATUS_OK == service->ListJobs( CString::Empty, 0, jobs ) );
                CAssetServiceResult result;
                CAssetConversionJobRequest request;
                request.packId = "pk";
                ASSERT_TRUE( ASSETSTORE_STATUS_UNAVAILABLE == service->RequestConversion( CString::Empty, request, result ) && !result.message.IsNULLOrEmpty() );
                service->Close();
                ASSERT_FALSE( service->IsOpen() );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

void
PerformAssetStoreServiceVfsTablesTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "AssetStoreServiceVfsTables" );

    CLocalServiceTarget target;
    target.label = "localvfstables";
    MountFreshDirectory( "svc_vfstables_catalogue", "astest/svc_localvfstables/catalogue" );
    target.database.backendName = "vfstables";
    target.database.database = "astest/svc_localvfstables/catalogue/db";
    target.database.backendOptions.SetAllowMultipleValues( false );
    target.database.backendOptions.Set( "syncWrites", CVariant( "false" ) );
    RunAssetStoreServiceSuite( target );
    RunAssetStoreServiceRestartSuite( target );
    UnmountDirectory( "svc_vfstables_catalogue" );
}

/*-------------------------------------------------------------------------*/

void
PerformAssetStoreServicePostgreSqlTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "AssetStoreServicePostgreSql" );

    CString connectionString;
    if ( !GetPostgreSqlConnectionString( connectionString ) )
    {
        GUCEF_LOG( LOGLEVEL_IMPORTANT, "AssetStoreServicePostgreSql: GUCEF_TEST_POSTGRESQL is not set, the PostgreSQL tests are skipped" );
        GUCEF_TESTFW_TESTCASE( "PostgreSQL: the asset store service against a PostgreSQL test database" )
            GUCEF_TESTFW_SKIP( "GUCEF_TEST_POSTGRESQL is not set" );
        GUCEF_TESTFW_TESTCASE_END;
        return;
    }

    {
        CString reachReason;
        if ( !IsPostgreSqlReachable( connectionString, reachReason ) )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "AssetStoreServicePostgreSql: the test database cannot be reached: " << reachReason );
            GUCEF_TESTFW_TESTCASE( "PostgreSQL: the test database can be reached" )
                try
                {
                    ASSERT_TRUE( reachReason.IsNULLOrEmpty() );
                }
                catch ( ... )
                {
                    ERRORHERE;
                }
            GUCEF_TESTFW_TESTCASE_END;
            return;
        }
    }

    CLocalServiceTarget target;
    target.label = "localpostgresql";
    const CString schema = MakeTestSchemaName();
    target.database.backendName = "postgresql";
    CValueList parsed;
    CString error;
    if ( DBPLUGIN::POSTGRESQL::CPostgreSqlSettings::ParseConnectionString( connectionString, parsed, error ) )
        target.database.database = parsed.GetValueAlways( CString( "dbname" ) ).AsString();
    target.database.backendOptions.SetAllowMultipleValues( false );
    target.database.backendOptions.Set( "connectionString", CVariant( connectionString ) );
    target.database.backendOptions.Set( "schema", CVariant( schema ) );
    target.database.backendOptions.Set( "createSchema", CVariant( "true" ) );
    target.database.backendOptions.Set( "applicationName", CVariant( "gucefASSETSTORE_TestApp" ) );
    target.database.backendOptions.Set( "connectTimeout", CVariant( "10" ) );
    target.database.backendOptions.Set( "maxRowsPerStatement", CVariant( "500" ) );

    // the password of the environment goes through a credential store, as the settings of an application would have it
    AUTH::CMemoryCredentialStore credentials;
    const char* password = getenv( "PGPASSWORD" );
    if ( GUCEF_NULL != password && '\0' != password[ 0 ] )
    {
        AUTH::CSecretString secret( ( CString( password ) ) );
        credentials.Store( "gucefASSETSTORE.test", "catalogue", secret );
        target.credentials = &credentials;
    }
    GUCEF_LOG( LOGLEVEL_NORMAL, "AssetStoreServicePostgreSql: testing in schema \"" << schema << "\"" );
    RunAssetStoreServiceSuite( target );
    RunAssetStoreServiceRestartSuite( target );

    // the schema of this run goes
    GUCEF_TESTFW_TESTCASE( "PostgreSQL: the schema of the run of the service suite is dropped" )
        try
        {
            CPrimaryAssetStoreSettings settings;
            settings.database = target.database;
            if ( GUCEF_NULL != target.credentials )
            {
                settings.credentialProvider = "gucefASSETSTORE.test";
                settings.credentialAccount = "catalogue";
            }
            COMCORE::CDatabaseClientSettings databaseSettings;
            ASSERT_TRUE( settings.GetDatabaseSettingsWithCredentials( target.credentials, databaseSettings, error ) );
            COMCORE::CIDatabaseClientPtr client = COMCORE::CComCoreGlobal::Instance()->GetDatabaseClientFactory().Create( "postgresql" );
            ASSERT_TRUE( !client.IsNULL() && client->Open( databaseSettings ) );
            if ( !client.IsNULL() && client->IsOpen() )
            {
                DBPLUGIN::POSTGRESQL::CPostgreSqlConnectionPtr connection = static_cast< DBPLUGIN::POSTGRESQL::CPostgreSqlClient* >( client.GetPointerAlways() )->GetConnection();
                ASSERT_TRUE( !connection.IsNULL() );
                if ( !connection.IsNULL() )
                {
                    MT::CScopeMutex lock( connection->GetLock(), GUCEF_MT_INFINITE_LOCK_TIMEOUT );
                    CString dropError;
                    ASSERT_TRUE( DATATABLE_STATUS_OK == connection->Execute( "DROP SCHEMA " + DBPLUGIN::POSTGRESQL::CPostgreSqlSql::QuoteIdentifier( schema ) + " CASCADE", dropError ) );
                }
                client->Close();
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/
