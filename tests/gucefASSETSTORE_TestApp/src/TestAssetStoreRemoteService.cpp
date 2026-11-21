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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACT_H
#include "gucefCOMCORE_CRpcContract.h"
#define GUCEF_COMCORE_CRPCCONTRACT_H
#endif /* GUCEF_COMCORE_CRPCCONTRACT_H ? */

#ifndef GUCEF_COM_CJSONRPCENDPOINT_H
#include "gucefCOM_CJsonRpcEndpoint.h"
#define GUCEF_COM_CJSONRPCENDPOINT_H
#endif /* GUCEF_COM_CJSONRPCENDPOINT_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREREMOTECONTRACTS_H
#include "gucefASSETSTORE_CAssetStoreRemoteContracts.h"
#define GUCEF_ASSETSTORE_CASSETSTOREREMOTECONTRACTS_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREREMOTECONTRACTS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREREMOTECODEC_H
#include "gucefASSETSTORE_CAssetStoreRemoteCodec.h"
#define GUCEF_ASSETSTORE_CASSETSTOREREMOTECODEC_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREREMOTECODEC_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSERVICENOTICE_H
#include "gucefASSETSTORE_CAssetServiceNotice.h"
#define GUCEF_ASSETSTORE_CASSETSERVICENOTICE_H
#endif /* GUCEF_ASSETSTORE_CASSETSERVICENOTICE_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

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

#ifndef GUCEF_TEST_LOG_CAPTURE_H
#include "gucef_test_log_capture.h"
#define GUCEF_TEST_LOG_CAPTURE_H
#endif /* GUCEF_TEST_LOG_CAPTURE_H ? */

#ifndef GUCEF_COMCORE_CSERVICEFINDER_H
#include "gucefCOMCORE_CServiceFinder.h"
#define GUCEF_COMCORE_CSERVICEFINDER_H
#endif /* GUCEF_COMCORE_CSERVICEFINDER_H ? */

#ifndef TESTASSETSTOREREMOTEWORLD_H
#include "TestAssetStoreRemoteWorld.h"
#define TESTASSETSTOREREMOTEWORLD_H
#endif /* TESTASSETSTOREREMOTEWORLD_H ? */

#include "TestAssetStoreRemoteService.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

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

/** Keeps the notices and the states of the connection a service raised */
class CRemoteNoticeRecorder : public CObservingNotifier
{
    public:

    typedef CTEventHandlerFunctor< CRemoteNoticeRecorder > TEventCallback;

    GUCEF::vector< CAssetServiceNotice > notices;
    GUCEF::vector< CAssetServiceNotice > states;

    explicit CRemoteNoticeRecorder( CIAssetStoreService& service )
        : CObservingNotifier(), notices(), states()
    {
        TEventCallback notice( this, &CRemoteNoticeRecorder::OnNotice );
        SubscribeTo( &service, CIAssetStoreService::NoticeEvent, notice );
        TEventCallback state( this, &CRemoteNoticeRecorder::OnState );
        SubscribeTo( &service, CIAssetStoreService::ConnectionStateChangedEvent, state );
    }

    virtual ~CRemoteNoticeRecorder()
    {
        UnsubscribeAllFromObserver();
    }

    bool HasNotice( const CString& code ) const
    {
        for ( size_t i=0; i<notices.size(); ++i )
            if ( notices[ i ].code == code )
                return true;
        return false;
    }

    bool HasState( const CString& code ) const
    {
        for ( size_t i=0; i<states.size(); ++i )
            if ( states[ i ].code == code )
                return true;
        return false;
    }

    private:

    void OnNotice( CNotifier* notifier, const CEvent& eventId, CICloneable* eventData )
    {
        const CAssetServiceNotice* notice = CICloneable::CloneableAs< CAssetServiceNotice >( eventData );
        if ( GUCEF_NULL != notice )
            notices.push_back( *notice );
    }

    void OnState( CNotifier* notifier, const CEvent& eventId, CICloneable* eventData )
    {
        const CAssetServiceNotice* notice = CICloneable::CloneableAs< CAssetServiceNotice >( eventData );
        if ( GUCEF_NULL != notice )
            states.push_back( *notice );
    }
};

/*-------------------------------------------------------------------------*/

/** Pumps until the connection of a remote service is connected; true when it got there in time */
bool
PumpUntilConnected( CIAssetStoreService& service, UInt32 timeoutMs )
{
    CRemoteAssetStoreService* remote = CAssetStoreServiceConfig::RemoteImplementationId == service.GetType() ? static_cast< CRemoteAssetStoreService* >( &service ) : GUCEF_NULL;
    if ( GUCEF_NULL == remote || GUCEF_NULL == remote->GetConnection() )
        return false;
    for ( UInt32 waited=0; waited<timeoutMs; waited+=5 )
    {
        if ( DRNUTILS::REMOTECONNECTION_CONNECTED == remote->GetConnection()->GetState() )
            return true;
        PumpFor( 5 );
    }
    return DRNUTILS::REMOTECONNECTION_CONNECTED == remote->GetConnection()->GetState();
}

/*-------------------------------------------------------------------------*/

/** The connection of a remote service */
DRNUTILS::CRemoteServiceConnection*
ConnectionOf( CIAssetStoreService& service )
{
    CRemoteAssetStoreService* remote = CAssetStoreServiceConfig::RemoteImplementationId == service.GetType() ? static_cast< CRemoteAssetStoreService* >( &service ) : GUCEF_NULL;
    return GUCEF_NULL == remote ? GUCEF_NULL : remote->GetConnection();
}

/*-------------------------------------------------------------------------*/

/** The file of the pack with the digest as the file set of its asset gives it (with its VFS path), false when none has it */
bool
FindPackFile( CIAssetStoreService& service, const CString& packId, const CString& digest, CAssetFileRecord& file )
{
    CAssetRecordPage assets;
    if ( ASSETSTORE_STATUS_OK != service.GetAssets( packId, CString::Empty, 0, assets ) )
        return false;
    for ( size_t a=0; a<assets.entries.size(); ++a )
    {
        CAssetFileSet fileSet;
        if ( ASSETSTORE_STATUS_OK != service.GetAssetFileSet( assets.entries[ a ].assetId, fileSet ) )
            continue;
        for ( size_t f=0; f<fileSet.files.size(); ++f )
            if ( fileSet.files[ f ].digest == digest )
            {
                file = fileSet.files[ f ];
                return true;
            }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/** Reads a whole file of the service (at most 4 MiB) */
bool
ReadWholeFile( CIAssetStoreService& service, const CString& vfsPath, UInt32 size, CDynamicBuffer& content )
{
    CAssetFileBlock block;
    const TAssetStoreStatus status = service.ReadFileBlock( vfsPath, 0, size, block );
    content = block.data;
    if ( ASSETSTORE_STATUS_OK != status || content.GetDataSize() != size || !block.endOfFile )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "ReadWholeFile: " << vfsPath << ": " << AssetStoreStatusToString( status ) << ", " << content.GetDataSize() << " of " << size
                         << " bytes, end of file " << ( block.endOfFile ? "yes" : "no" ) << ", file size " << block.fileSize );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Uploads the files (relative path, content) as a pack and commits it; the result of the commit */
TAssetStoreStatus
UploadPack( CIAssetStoreService& service, const CString& commandId, const CString& name, const CString::StringVector& paths,
            const GUCEF::vector< CDynamicBuffer >& contents, CAssetServiceResult& committed )
{
    CAssetFileStatPage files;
    for ( size_t i=0; i<paths.size(); ++i )
    {
        CAssetFileRecord file;
        file.path = paths[ i ];
        file.sizeBytes = (Int64) contents[ i ].GetDataSize();
        file.digest = Sha256Of( contents[ i ] );
        files.entries.push_back( file );
    }
    CAssetServiceResult begun;
    TAssetStoreStatus status = service.BeginUploadFiles( commandId + "-begin", name, files, begun );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    for ( size_t i=0; i<paths.size(); ++i )
    {
        CAssetServiceResult written;
        status = service.WriteUploadBlock( begun.id, paths[ i ], 0, contents[ i ], written );
        if ( ASSETSTORE_STATUS_OK != status )
            return status;
    }
    CAssetIngestRequest ingest;
    ingest.keep = "original";
    return service.CommitUpload( commandId + "-commit", begun.id, ingest, committed );
}

/*-------------------------------------------------------------------------*/

/** A configuration of the server side for the fault suites: vfstables, one location, a pack to ingest */
CAssetStoreServiceConfig
MakeFaultServerConfig( const CString& name, CString& packPath )
{
    const CString base = "astest/remote_" + name;
    MountFreshDirectory( "remote_" + name + "_main", base + "/main" );
    MountFreshDirectory( "remote_" + name + "_source", base + "/source" );
    MountFreshDirectory( "remote_" + name + "_catalogue", base + "/catalogue" );
    packPath = base + "/source/Crates";
    CDynamicBuffer png;
    MakePng( 32, 32, false, png );
    WriteVfsFile( packPath + "/pack.json", CString( "{ \"title\": \"Crates\", \"author\": \"Test Works\", \"tags\": [ \"crate\" ] }" ) );
    WriteVfsFile( packPath + "/Textures/T_Crate_D.png", png );
    CDynamicBuffer tga;
    MakeTga( 16, 16, tga );
    WriteVfsFile( base + "/source/Barrels/Textures/T_Barrel_D.tga", tga );

    CAssetStoreServiceConfig config;
    config.primaryStore.locations.clear();
    CAssetStorageLocation main;
    main.locationId = "main";
    main.name = "Main";
    main.vfsRoot = base + "/main";
    config.primaryStore.locations.push_back( main );
    config.primaryStore.defaultLocationId = "main";
    config.primaryStore.database.backendName = "vfstables";
    config.primaryStore.database.database = base + "/catalogue/db";
    config.primaryStore.database.backendOptions.SetAllowMultipleValues( false );
    config.primaryStore.database.backendOptions.Set( "syncWrites", CVariant( "false" ) );
    config.loadDownloadQueue = false;
    config.startDownloads = false;
    return config;
}

/*-------------------------------------------------------------------------*/

void
UnmountFaultServer( const CString& name )
{
    UnmountDirectory( "remote_" + name + "_main" );
    UnmountDirectory( "remote_" + name + "_source" );
    UnmountDirectory( "remote_" + name + "_catalogue" );
}

/*-------------------------------------------------------------------------*/

/** Drops the schema of a PostgreSQL run */
bool
DropTestSchema( const CPrimaryAssetStoreSettings& settings, AUTH::ICredentialStore* credentials, const CString& schema )
{
    COMCORE::CDatabaseClientSettings databaseSettings;
    CString error;
    if ( !settings.GetDatabaseSettingsWithCredentials( credentials, databaseSettings, error ) )
        return false;
    COMCORE::CIDatabaseClientPtr client = COMCORE::CComCoreGlobal::Instance()->GetDatabaseClientFactory().Create( "postgresql" );
    if ( client.IsNULL() || !client->Open( databaseSettings ) )
        return false;
    bool dropped = false;
    DBPLUGIN::POSTGRESQL::CPostgreSqlConnectionPtr connection = static_cast< DBPLUGIN::POSTGRESQL::CPostgreSqlClient* >( client.GetPointerAlways() )->GetConnection();
    if ( !connection.IsNULL() )
    {
        MT::CScopeMutex lock( connection->GetLock(), GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        CString dropError;
        dropped = DATATABLE_STATUS_OK == connection->Execute( "DROP SCHEMA " + DBPLUGIN::POSTGRESQL::CPostgreSqlSql::QuoteIdentifier( schema ) + " CASCADE", dropError );
    }
    client->Close();
    return dropped;
}


}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAssetStoreRemoteUnitTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "AssetStoreRemoteUnits" );

    GUCEF_TESTFW_TESTCASE( "Remote service: the contracts build, the secret travels in a sensitive field of a sensitive method, every operation has its record" )
        try
        {
            COMCORE::CRpcContractPtr control = CAssetStoreRemoteContracts::GetControlContract();
            COMCORE::CRpcContractPtr files = CAssetStoreRemoteContracts::GetFilesContract();
            ASSERT_TRUE( !control.IsNULL() && !files.IsNULL() );
            if ( !control.IsNULL() && !files.IsNULL() )
            {
                const COMCORE::CRpcTypeDescriptor* secret = control->FindType( "SecretRequest" );
                ASSERT_TRUE( GUCEF_NULL != secret );
                COMCORE::CRpcMessage request( control, secret );
                request.SetString( CAssetStoreRemoteContracts::SecretValueField, "do-not-show" );
                CDataNode redacted;
                ASSERT_TRUE( request.ToDataNode( redacted, true ) );
                ASSERT_TRUE( redacted.GetAttributeValueOrChildValueByName( "secret" ).AsString().HasSubstr( "do-not-show" ) < 0 );
                ASSERT_TRUE( GUCEF_NULL != files->FindType( "Block" ) && GUCEF_NULL != files->FindType( "WriteBlockRequest" ) );
            }
            // every operation of the interface answers a record the client can make
            const char* operations[] = { "Info/GetServiceInfo", "Catalogue/Search", "Catalogue/GetPack", "Files/Stat", "Files/Read", "Files/BeginUploadFiles",
                                         "Files/QueryUpload", "Locations/Verify", "Ingest/Ingest", "Jobs/List", "Jobs/Control", "Stores/QueryLibrary",
                                         "SignIn/Answer", "Admin/UnlockKeyring", "Pipeline/Query", "LocalCopy/Request" };
            for ( UInt32 i=0; i<sizeof( operations ) / sizeof( operations[ 0 ] ); ++i )
            {
                CIDataNodeSerializable* record = CAssetStoreRemoteCodec::NewResult( operations[ i ] );
                ASSERT_TRUE( GUCEF_NULL != record );
                GUCEF_DELETE record;
            }
            ASSERT_TRUE( GUCEF_NULL == CAssetStoreRemoteCodec::NewResult( "Catalogue/NoSuchOperation" ) );
            ASSERT_TRUE( CAssetStoreRemoteContracts::IsFilesOperation( CAssetServiceOperations::ReadFileBlock ) && !CAssetStoreRemoteContracts::IsFilesOperation( CAssetServiceOperations::Search ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == CAssetStoreRemoteCodec::StatusOfRpcFailure( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_PERMISSION_DENIED ) ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_UNAVAILABLE == CAssetStoreRemoteCodec::StatusOfRpcFailure( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_UNAVAILABLE ) ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == CAssetStoreRemoteCodec::StatusOfRpcFailure( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_INVALID_ARGUMENT ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Remote service: an answer carries the record of the operation and the status both ways; ingest results and verification reports have a DataNode form" )
        try
        {
            COMCORE::CRpcContractPtr control = CAssetStoreRemoteContracts::GetControlContract();
            if ( !control.IsNULL() )
            {
                CAssetPackRecord pack;
                pack.packId = "pk-1";
                pack.title = "Remote Pack";
                COMCORE::CRpcMessage answer( control, control->FindType( "CallAnswer" ) );
                CAssetStoreRemoteCodec::FillAnswer( answer, ASSETSTORE_STATUS_OK, CString::Empty, &pack, 17, false );
                CIDataNodeSerializable* record = GUCEF_NULL;
                CString message;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == CAssetStoreRemoteCodec::ReadAnswer( CAssetServiceOperations::GetPack, answer, record, message ) );
                const CAssetPackRecord* back = CICloneable::CloneableAs< CAssetPackRecord >( record );
                ASSERT_TRUE( GUCEF_NULL != back && "Remote Pack" == back->title && 17 == answer.GetUInt64( CAssetStoreRemoteContracts::AnswerEventSeqField ) );
                GUCEF_DELETE record;

                COMCORE::CRpcMessage refused( control, control->FindType( "CallAnswer" ) );
                CAssetStoreRemoteCodec::FillAnswer( refused, ASSETSTORE_STATUS_NOT_FOUND, "no pack pk-2", GUCEF_NULL, 0, false );
                ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == CAssetStoreRemoteCodec::ReadAnswer( CAssetServiceOperations::SaveSearch, refused, record, message ) );
                const CAssetServiceResult* outcome = CICloneable::CloneableAs< CAssetServiceResult >( record );
                ASSERT_TRUE( GUCEF_NULL != outcome && "no pack pk-2" == outcome->message && "no pack pk-2" == message );
                GUCEF_DELETE record;
            }
            CAssetIngestResult ingest;
            ingest.ingestId = "in-1";
            ingest.status = ASSETSTORE_STATUS_DUPLICATE;
            ingest.duplicateOfPackId = "pk-9";
            ingest.problems.push_back( "a problem" );
            CDataNode node;
            CDataNodeSerializableSettings settings;
            ASSERT_TRUE( ingest.Serialize( node, settings ) );
            CAssetIngestResult ingestBack;
            ASSERT_TRUE( ingestBack.Deserialize( node, settings ) && ASSETSTORE_STATUS_DUPLICATE == ingestBack.status && "pk-9" == ingestBack.duplicateOfPackId && 1 == ingestBack.problems.size() );
            CAssetVerificationReport report;
            report.filesChecked = 12;
            report.missingFiles.push_back( "a/b.png" );
            CDataNode reportNode;
            ASSERT_TRUE( report.Serialize( reportNode, settings ) );
            CAssetVerificationReport reportBack;
            ASSERT_TRUE( reportBack.Deserialize( reportNode, settings ) && 12 == reportBack.filesChecked && !reportBack.IsIntact() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Remote service: settings refuse what they do not know; without credentials the service says the device is not paired" )
        try
        {
            CRemoteAssetStoreServiceSettings settings;
            CDataNode config( "service", GUCEF_DATATYPE_OBJECT );
            config.SetAttribute( "endpoint", "tcp://worker.lan:47400" );
            config.SetAttribute( "bulkChannels", (UInt32) 1 );
            ASSERT_TRUE( settings.LoadConfig( config ) && "worker.lan:47400" == settings.GetHostAndPort() );
            config.SetAttribute( "endpiont", "typo" );
            CRemoteAssetStoreServiceSettings refused;
            ASSERT_FALSE( refused.LoadConfig( config ) );
            CAssetStoreServiceHostSettings hostSettings;
            CDataNode hostConfig( "host", GUCEF_DATATYPE_OBJECT );
            hostConfig.SetAttribute( "roles", "owner,curator" );
            ASSERT_TRUE( hostSettings.LoadConfig( hostConfig ) && 2 == hostSettings.roles.size() );
            hostConfig.SetAttribute( "stagingDir", "x" );
            ASSERT_FALSE( hostSettings.LoadConfig( hostConfig ) );

            AUTH::CMemoryCredentialStore empty;
            CAssetStoreServiceConfig clientConfig;
            clientConfig.implementationId = CAssetStoreServiceConfig::RemoteImplementationId;
            clientConfig.credentialStore = &empty;
            CString error;
            TAssetStoreServicePtr client = CAssetStoreGlobal::Instance()->CreateService( clientConfig, error );
            ASSERT_TRUE( !client.IsNULL() && CAssetStoreServiceConfig::RemoteImplementationId == client->GetType() && !client->IsInProcess() );
            if ( !client.IsNULL() )
            {
                CAssetPackRecord pack;
                ASSERT_TRUE( ASSETSTORE_STATUS_UNAVAILABLE == client->GetPack( "pk", pack ) );
                ASSERT_TRUE( ASSETSTORE_STATUS_UNAVAILABLE == client->Open() && !client->IsCatalogueAvailable() );
                ASSERT_TRUE( client->GetStatusMessage().HasSubstr( "not paired" ) >= 0 );
                client->Close();
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Remote service: the authoring sources rpc/gucef.assetstore.json and rpc/gucef.assetstore.files.json are the contracts the code builds (GUCEF_ASSETSTORE_RPC_DIR)" )
        try
        {
            const char* rpcDir = getenv( "GUCEF_ASSETSTORE_RPC_DIR" );
            if ( GUCEF_NULL == rpcDir || '\0' == rpcDir[ 0 ] )
                GUCEF_TESTFW_SKIP( "GUCEF_ASSETSTORE_RPC_DIR is not set" );
            const char* files[ 2 ] = { "gucef.assetstore.json", "gucef.assetstore.files.json" };
            COMCORE::CRpcContractPtr built[ 2 ] = { CAssetStoreRemoteContracts::GetControlContract(), CAssetStoreRemoteContracts::GetFilesContract() };
            for ( UInt32 i=0; i<2; ++i )
            {
                CDynamicBuffer content;
                ASSERT_TRUE( content.LoadContentFromFile( CombinePath( rpcDir, files[ i ] ), 0, 1048576 ) );
                CString text;
                text.Set( (const char*) content.GetConstBufferPtr(), content.GetDataSize() );
                CDataNode tree;
                ASSERT_TRUE( COM::CJsonRpcEndpoint::ParseJsonText( text, tree ) );
                CString error;
                COMCORE::CRpcContractPtr authored = COMCORE::CRpcContract::LoadFromDataNode( tree, error );
                if ( authored.IsNULL() )
                    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "AssetStoreRemoteUnits: " << files[ i ] << ": " << error );
                ASSERT_TRUE( !authored.IsNULL() && !built[ i ].IsNULL() );
                if ( !authored.IsNULL() && !built[ i ].IsNULL() )
                {
                    if ( !( authored->GetHash() == built[ i ]->GetHash() ) )
                        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "AssetStoreRemoteUnits: " << files[ i ] << " hash " << authored->GetHash().ToHexString() << " built " << built[ i ]->GetHash().ToHexString() );
                    ASSERT_TRUE( authored->GetHash() == built[ i ]->GetHash() );
                }
            }
        }
        catch ( ::GUCEF::TESTFW::STestSkipRequest& )
        {
            throw;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

void
PerformAssetStoreRemoteServiceVfsTablesTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "AssetStoreRemoteServiceVfsTables" );

    CRemoteServiceSuiteTarget target( "remotevfstables" );
    MountFreshDirectory( "svc_remotevfstables_catalogue", "astest/svc_remotevfstables/catalogue" );
    target.database.backendName = "vfstables";
    target.database.database = "astest/svc_remotevfstables/catalogue/db";
    target.database.backendOptions.SetAllowMultipleValues( false );
    target.database.backendOptions.Set( "syncWrites", CVariant( "false" ) );
    RunAssetStoreServiceSuite( target );
    target.world.StopServer();
    UnmountDirectory( "svc_remotevfstables_catalogue" );
}

/*-------------------------------------------------------------------------*/

void
PerformAssetStoreRemoteServicePostgreSqlTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "AssetStoreRemoteServicePostgreSql" );

    CString connectionString;
    if ( !GetPostgreSqlConnectionString( connectionString ) )
    {
        GUCEF_LOG( LOGLEVEL_IMPORTANT, "AssetStoreRemoteServicePostgreSql: GUCEF_TEST_POSTGRESQL is not set, the PostgreSQL tests are skipped" );
        GUCEF_TESTFW_TESTCASE( "PostgreSQL: the remote asset store service against a PostgreSQL test database" )
            GUCEF_TESTFW_SKIP( "GUCEF_TEST_POSTGRESQL is not set" );
        GUCEF_TESTFW_TESTCASE_END;
        return;
    }

    {
        CString reachReason;
        if ( !IsPostgreSqlReachable( connectionString, reachReason ) )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "AssetStoreRemoteServicePostgreSql: the test database cannot be reached: " << reachReason );
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

    CRemoteServiceSuiteTarget target( "remotepostgresql" );
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
    AUTH::CMemoryCredentialStore credentials;
    const char* password = getenv( "PGPASSWORD" );
    if ( GUCEF_NULL != password && '\0' != password[ 0 ] )
    {
        AUTH::CSecretString secret( ( CString( password ) ) );
        credentials.Store( "gucefASSETSTORE.test", "catalogue", secret );
        target.credentials = &credentials;
    }
    GUCEF_LOG( LOGLEVEL_NORMAL, "AssetStoreRemoteServicePostgreSql: testing in schema \"" << schema << "\"" );
    RunAssetStoreServiceSuite( target );
    target.world.StopServer();

    GUCEF_TESTFW_TESTCASE( "PostgreSQL: the schema of the run of the remote service suite is dropped" )
        try
        {
            CPrimaryAssetStoreSettings settings;
            settings.database = target.database;
            if ( GUCEF_NULL != target.credentials )
            {
                settings.credentialProvider = "gucefASSETSTORE.test";
                settings.credentialAccount = "catalogue";
            }
            ASSERT_TRUE( DropTestSchema( settings, target.credentials, schema ) );
            // a schema of a local run of the service program, named by the caller (only assetsvc_ schemas)
            const char* extra = getenv( "GUCEF_TEST_ASSETSTORE_DROP_SCHEMA" );
            if ( GUCEF_NULL != extra && 0 == CString( extra ).HasSubstr( "assetsvc_" ) )
                ASSERT_TRUE( DropTestSchema( settings, target.credentials, CString( extra ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

/** Whether the page lists the store */
bool
ServiceSuiteFindStoreIn( const CAssetStoreInfoPage& page, const CString& storeId )
{
    for ( size_t i=0; i<page.entries.size(); ++i )
        if ( page.entries[ i ].storeId == storeId )
            return true;
    return false;
}

/** Whether the page lists the store in the sign in state */
bool
ServiceSuiteFindStoreInState( const CAssetStoreInfoPage& page, const CString& storeId, TAssetStoreSignInState state )
{
    for ( size_t i=0; i<page.entries.size(); ++i )
        if ( page.entries[ i ].storeId == storeId )
            return page.entries[ i ].signInState == state;
    return false;
}

void
PerformAssetStoreRemoteFaultTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "AssetStoreRemoteFaults" );

    CString packPath;
    CAssetStoreServiceConfig serverConfig = MakeFaultServerConfig( "faults", packPath );
    CRemoteAssetStoreWorld world( "faults" );
    CString error;
    const bool started = world.StartServer( serverConfig, error );
    if ( !started )
        GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "AssetStoreRemoteFaults: the server side did not start: " << error );
    TAssetStoreServicePtr clientPtr;
    TESTFW::CLogTextCapture* capture = GUCEF_NEW TESTFW::CLogTextCapture();
    if ( started )
        clientPtr = world.MakeClient( "faults-a", error );
    const CString pairingCode = world.lastCode;
    const bool codeLogged = !pairingCode.IsNULLOrEmpty() && capture->Contains( pairingCode );
    const bool logsCaptured = !capture->GetText().IsNULLOrEmpty();
    GUCEF_DELETE capture;
    CString packId;

    GUCEF_TESTFW_TESTCASE( "Remote faults: a device pairs with the code of a window (the code is in no log), connects and ingests a pack" )
        try
        {
            ASSERT_TRUE( started && !clientPtr.IsNULL() );
            ASSERT_TRUE( logsCaptured && !pairingCode.IsNULLOrEmpty() && !codeLogged );
            if ( !clientPtr.IsNULL() )
            {
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == clientPtr->Open() );
                CAssetServiceInfo info;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == clientPtr->GetServiceInfo( info ) && 1 == info.roles.count( "owner" ) && info.HasCapability( "uploads" ) );
                CAssetIngestRequest request;
                request.sourceId = CPrimaryAssetStoreSettings::DiskSourceId;
                request.contentPath = packPath;
                request.keep = "original";
                CAssetIngestResult result;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == clientPtr->Ingest( "fault-ingest-1", request, result ) && !result.packId.IsNULLOrEmpty() );
                packId = result.packId;
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    if ( clientPtr.IsNULL() || !clientPtr->IsCatalogueAvailable() )
    {
        world.StopServer();
        UnmountFaultServer( "faults" );
        return;
    }
    CIAssetStoreService& client = *clientPtr;
    CServiceEventRecorder events( client );
    CRemoteNoticeRecorder notices( client );

    GUCEF_TESTFW_TESTCASE( "Remote faults: the link is cut while job listings are on their way: every listing completes with the whole list, the session resumes" )
        try
        {
            CAssetJobPage baseline;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == client.ListJobs( CString::Empty, 0, baseline ) && baseline.total >= 1 );
            DRNUTILS::CRemoteServiceConnection* connection = ConnectionOf( client );
            ASSERT_TRUE( GUCEF_NULL != connection );
            if ( GUCEF_NULL != connection )
            {
                const UInt32 resumesBefore = connection->GetRpcResumeCount() + connection->GetSessionResumeCount() + connection->GetSessionReplaceCount();
                GUCEF::vector< CFutureResult > listings;
                for ( UInt32 i=0; i<30; ++i )
                {
                    listings.push_back( client.ListJobsAsync( CString::Empty, 0 ) );
                    if ( 10 == i )
                        connection->DropLink();
                }
                UInt32 complete = 0;
                for ( size_t i=0; i<listings.size(); ++i )
                {
                    CAssetJobPage page;
                    if ( WaitForFuture( listings[ i ], 30000 ) && CIAssetStoreService::TryGetFutureResult( listings[ i ], page ) && page.total == baseline.total &&
                         page.entries.size() == baseline.entries.size() )
                        ++complete;
                }
                ASSERT_TRUE( 30 == complete );
                ASSERT_TRUE( PumpUntilConnected( client, 10000 ) );
                ASSERT_TRUE( connection->GetRpcResumeCount() + connection->GetSessionResumeCount() + connection->GetSessionReplaceCount() > resumesBefore );
                ASSERT_TRUE( notices.HasState( "RECONNECTING" ) || notices.HasState( "reconnecting" ) || notices.states.size() >= 2 );

                // the events still come after the cut
                const UInt32 before = events.catalogueChanges;
                CAssetServiceResult saved;
                CAssetSavedSearch search;
                search.searchId = "user-fault-crates";
                search.name = "Crates";
                search.query.text = "crate";
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == client.SaveSearch( "fault-save-1", search, saved ) );
                PumpFor( 50 );
                ASSERT_TRUE( events.catalogueChanges > before && events.HasChange( ASSETCHANGE_SAVED_SEARCHES, "user-fault-crates" ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Remote faults: the service restarts; a mutation repeated with its command id is answered once, from the journal; the id stays bound to its request" )
        try
        {
            CAssetIngestRequest request;
            request.sourceId = CPrimaryAssetStoreSettings::DiskSourceId;
            request.contentPath = "astest/remote_faults/source/Barrels";
            request.keep = "original";
            CAssetIngestResult first;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == client.Ingest( "fault-restart-1", request, first ) && !first.packId.IsNULLOrEmpty() );
            DRNUTILS::CRemoteServiceConnection* connection = ConnectionOf( client );
            const UInt32 replacedBefore = GUCEF_NULL == connection ? 0 : connection->GetSessionReplaceCount();

            // the whole server goes: the service of the server side, its host and its binding; the catalogue stays on disk
            world.StopServer();
            CString restartError;
            ASSERT_TRUE( world.StartServer( world.serverConfig, restartError ) );
            ASSERT_TRUE( PumpUntilConnected( client, 20000 ) );
            for ( UInt32 waited=0; GUCEF_NULL != connection && connection->GetSessionReplaceCount() == replacedBefore && waited<10000; waited+=10 )
                PumpFor( 10 );
            ASSERT_TRUE( GUCEF_NULL != connection && connection->GetSessionReplaceCount() > replacedBefore );

            // a new execution would answer DUPLICATE: the journal answers the first outcome
            CAssetIngestResult repeat;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == client.Ingest( "fault-restart-1", request, repeat ) );
            ASSERT_TRUE( repeat.packId == first.packId && repeat.ingestId == first.ingestId && ASSETSTORE_STATUS_OK == repeat.status );
            CValueList info;
            world.binding->GetInfo( info );
            ASSERT_TRUE( info.GetValueAlways( CString( "assetstore.replays" ) ).AsUInt64() >= 1 );
            CAssetIngestRequest other = request;
            other.keep = "both";
            CAssetIngestResult reused;
            ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == client.Ingest( "fault-restart-1", other, reused ) );
            // without the command id it is a new ingest of the same pack
            CAssetIngestResult again;
            ASSERT_TRUE( ASSETSTORE_STATUS_DUPLICATE == client.Ingest( CString::Empty, request, again ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Remote faults: an upload is cut, continues from what arrived, is committed with its digests checked, ingested and read back; a wrong digest is refused" )
        try
        {
            CDynamicBuffer big;
            MakeTga( 420, 420, big );
            CDynamicBuffer small;
            MakePng( 24, 24, true, small );
            CAssetFileStatPage files;
            CAssetFileRecord bigFile;
            bigFile.path = "Textures/T_Upload_Big.tga";
            bigFile.sizeBytes = (Int64) big.GetDataSize();
            bigFile.digest = Sha256Of( big );
            files.entries.push_back( bigFile );
            CAssetFileRecord smallFile;
            smallFile.path = "Textures/T_Upload_N.png";
            smallFile.sizeBytes = (Int64) small.GetDataSize();
            smallFile.digest = Sha256Of( small );
            files.entries.push_back( smallFile );
            CAssetServiceResult begun;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == client.BeginUploadFiles( "fault-upload-1", "Uploaded Textures", files, begun ) && !begun.id.IsNULLOrEmpty() );
            const UInt32 block = CAssetStoreRemoteContracts::BlockSize;
            ASSERT_TRUE( big.GetDataSize() > 2 * block );

            CDynamicBuffer first;
            first.Append( big.GetConstBufferPtr(), block );
            CAssetServiceResult written;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == client.WriteUploadBlock( begun.id, bigFile.path, 0, first, written ) );
            // the link goes: the next block waits for the reconnect and arrives
            DRNUTILS::CRemoteServiceConnection* connection = ConnectionOf( client );
            if ( GUCEF_NULL != connection )
                connection->DropLink();
            CDynamicBuffer second;
            second.Append( static_cast< const UInt8* >( big.GetConstBufferPtr() ) + block, block );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == client.WriteUploadBlock( begun.id, bigFile.path, block, second, written ) );
            CAssetFileStatPage received;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == client.QueryUpload( begun.id, received ) && 2 == received.entries.size() );
            ASSERT_TRUE( (Int64) ( 2 * block ) == received.entries[ 0 ].sizeBytes && 0 == received.entries[ 0 ].role.HasSubstr( "received:" ) );
            ASSERT_TRUE( 0 == received.entries[ 1 ].sizeBytes );

            // the rest from where it is, then the commit checks the digests and ingests
            const UInt32 from = (UInt32) received.entries[ 0 ].sizeBytes;
            CDynamicBuffer rest;
            rest.Append( static_cast< const UInt8* >( big.GetConstBufferPtr() ) + from, big.GetDataSize() - from );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == client.WriteUploadBlock( begun.id, bigFile.path, from, rest, written ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == client.WriteUploadBlock( begun.id, smallFile.path, 0, small, written ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == client.QueryUpload( begun.id, received ) && "complete" == received.entries[ 0 ].role && "complete" == received.entries[ 1 ].role );
            CAssetIngestRequest ingest;
            ingest.keep = "original";
            CAssetServiceResult committed;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == client.CommitUpload( "fault-commit-1", begun.id, ingest, committed ) && !committed.relatedId.IsNULLOrEmpty() );
            // the pack is named as the upload, not by the folder that holds the staged files
            CAssetPackRecord uploadedPack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == client.GetPack( committed.relatedId, uploadedPack ) );
            ASSERT_TRUE( "Uploaded Textures" == uploadedPack.title && uploadedPack.rootPath.HasSubstr( "Uploaded" ) >= 0 && uploadedPack.rootPath.HasSubstr( "files" ) < 0 );
            CAssetFileRecord placed;
            ASSERT_TRUE( FindPackFile( client, committed.relatedId, bigFile.digest, placed ) );
            CDynamicBuffer readBack;
            ASSERT_TRUE( ReadWholeFile( client, placed.path, big.GetDataSize(), readBack ) && Sha256Of( readBack ) == bigFile.digest );
            CAssetServiceResult gone;
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == client.QueryUpload( begun.id, received ) );

            // a file whose bytes are not those of its digest
            CAssetFileStatPage wrong;
            CAssetFileRecord wrongFile = smallFile;
            wrongFile.path = "Textures/T_Wrong.png";
            wrongFile.digest = Sha256Of( big );
            wrong.entries.push_back( wrongFile );
            CAssetServiceResult wrongBegun;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == client.BeginUploadFiles( "fault-upload-2", "Wrong", wrong, wrongBegun ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == client.WriteUploadBlock( wrongBegun.id, wrongFile.path, 0, small, written ) );
            CAssetServiceResult refused;
            ASSERT_TRUE( ASSETSTORE_STATUS_DIGEST_MISMATCH == client.CommitUpload( "fault-commit-2", wrongBegun.id, ingest, refused ) && !refused.message.IsNULLOrEmpty() );
            CAssetServiceResult aborted;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == client.AbortUpload( "fault-abort-2", wrongBegun.id, aborted ) );
            CAssetFileStatPage unsafeFiles;
            CAssetFileRecord unsafe = smallFile;
            unsafe.path = "../outside.png";
            unsafeFiles.entries.push_back( unsafe );
            CAssetServiceResult unsafeBegun;
            ASSERT_TRUE( ASSETSTORE_STATUS_UNSAFE_NAME == client.BeginUploadFiles( "fault-upload-3", "Unsafe", unsafeFiles, unsafeBegun ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Remote faults: three clients of three devices: a change of one is an event at all, their searches run side by side" )
        try
        {
            CString clientError;
            TAssetStoreServicePtr second = world.MakeClient( "faults-b", clientError );
            TAssetStoreServicePtr third = world.MakeClient( "faults-c", clientError );
            ASSERT_TRUE( !second.IsNULL() && !third.IsNULL() );
            if ( !second.IsNULL() && !third.IsNULL() )
            {
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == second->Open() && ASSETSTORE_STATUS_OK == third->Open() );
                CServiceEventRecorder secondEvents( *second );
                CServiceEventRecorder thirdEvents( *third );
                CAssetSavedSearch search;
                search.searchId = "user-three";
                search.name = "Three";
                search.query.text = "barrel";
                CAssetServiceResult saved;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == second->SaveSearch( "three-1", search, saved ) );
                for ( UInt32 waited=0; waited<5000 && !( events.HasChange( ASSETCHANGE_SAVED_SEARCHES, "user-three" ) && thirdEvents.HasChange( ASSETCHANGE_SAVED_SEARCHES, "user-three" ) ); waited+=10 )
                    PumpFor( 10 );
                ASSERT_TRUE( events.HasChange( ASSETCHANGE_SAVED_SEARCHES, "user-three" ) && secondEvents.HasChange( ASSETCHANGE_SAVED_SEARCHES, "user-three" ) &&
                             thirdEvents.HasChange( ASSETCHANGE_SAVED_SEARCHES, "user-three" ) );
                CAssetSearchQuery all;
                all.pageSize = 100;
                CAssetSearchResultPage baseline;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == client.Search( all, baseline ) && baseline.total > 0 );
                GUCEF::vector< CFutureResult > searches;
                for ( UInt32 i=0; i<10; ++i )
                {
                    searches.push_back( client.SearchAsync( all ) );
                    searches.push_back( second->SearchAsync( all ) );
                    searches.push_back( third->SearchAsync( all ) );
                }
                UInt32 same = 0;
                for ( size_t i=0; i<searches.size(); ++i )
                {
                    CAssetSearchResultPage page;
                    if ( WaitForFuture( searches[ i ], 30000 ) && CIAssetStoreService::TryGetFutureResult( searches[ i ], page ) && page.total == baseline.total )
                        ++same;
                }
                ASSERT_TRUE( 30 == same );
                second->Close();
                third->Close();
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Remote faults: the service drains: the client is told, mutations are refused, reads still answer; after the restart it connects again" )
        try
        {
            world.host->BeginDrain( "test drain", 1 );
            for ( UInt32 waited=0; waited<3000 && !notices.HasNotice( "SERVICE_STOPPING" ); waited+=10 )
                PumpFor( 10 );
            ASSERT_TRUE( notices.HasNotice( "SERVICE_STOPPING" ) );
            CAssetSavedSearch search;
            search.searchId = "user-drain";
            search.name = "Drain";
            CAssetServiceResult refused;
            ASSERT_TRUE( ASSETSTORE_STATUS_UNAVAILABLE == client.SaveSearch( "drain-1", search, refused ) );
            world.StopHost();
            CString restartError;
            ASSERT_TRUE( world.StartHost( restartError ) );
            ASSERT_TRUE( PumpUntilConnected( client, 20000 ) );
            CAssetServiceResult saved;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == client.SaveSearch( "drain-1", search, saved ) && !saved.replayed );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    client.Close();
    clientPtr.Unlink();
    world.StopServer();
    UnmountFaultServer( "faults" );
}

/*-------------------------------------------------------------------------*/

void
PerformAssetStoreRemoteSignInRoleTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "AssetStoreRemoteSignInRoles" );

    // a world of its own: a machine caller (the role mcp) may use the service, but never the sign in and the keyring
    CString packPath;
    CAssetStoreServiceConfig serverConfig = MakeFaultServerConfig( "roles", packPath );
    serverConfig.signIn.storeSettings.AddChild( "faultsign", GUCEF_DATATYPE_OBJECT )->SetAttribute( "clientIdFile", "/run/secrets/fake/clientId" );
    CRemoteAssetStoreWorld world( "roles" );
    world.bindingSettings.roles.insert( "mcp" );
    CString error;
    const bool started = world.StartServer( serverConfig, error );
    if ( !started )
        GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "AssetStoreRemoteSignInRoles: the server side did not start: " << error );
    TAssetStoreServicePtr clientPtr;
    if ( started )
        clientPtr = world.MakeClient( "roles-owner", error );
    if ( clientPtr.IsNULL() || ASSETSTORE_STATUS_OK != clientPtr->Open() )
    {
        GUCEF_TESTFW_TESTCASE( "Remote sign in roles: the owner device connects" )
            ASSERT_TRUE( false );
        GUCEF_TESTFW_TESTCASE_END;
        world.StopServer();
        UnmountFaultServer( "roles" );
        return;
    }
    CIAssetStoreService& client = *clientPtr;

    GUCEF_TESTFW_TESTCASE( "Remote sign in roles: only the owner devices drive a sign in and the keyring: a device with the role mcp is refused every operation of them, gets no prompt, and cannot answer one" )
        try
        {
            CFakeSignInStoreProviderFactory signInFactory( "faultsign", false );
            CAssetStoreGlobal::Instance()->GetProviderFactory().RegisterConcreteFactory( "faultsign", &signInFactory );
            CAssetStoreGlobal::Instance()->GetRegistry().RefreshProviders();
            PumpFor( 100 );

            CString roleError;
            CString::StringSet mcpRoles;
            mcpRoles.insert( "mcp" );
            TAssetStoreServicePtr mcpPtr = world.MakeClient( "roles-mcp", roleError, &mcpRoles, false );
            ASSERT_TRUE( !mcpPtr.IsNULL() );
            if ( !mcpPtr.IsNULL() )
            {
                CIAssetStoreService& mcp = *mcpPtr;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == mcp.Open() );
                CServiceEventRecorder ownerRecorder( client );
                CSignInEventRecorder ownerPrompts( client );
                CSignInEventRecorder mcpPrompts( mcp );

                // what a machine caller may do: read, including the list of the stores with the state of their sign in
                CAssetServiceInfo mcpInfo;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == mcp.GetServiceInfo( mcpInfo ) && 1 == mcpInfo.roles.count( "mcp" ) && 0 == mcpInfo.roles.count( "owner" ) );
                CAssetStoreInfoPage stores;
                bool listed = false;
                for ( UInt32 waited=0; waited<5000 && !listed; waited+=10 )
                {
                    listed = ASSETSTORE_STATUS_OK == mcp.ListStores( stores ) && ServiceSuiteFindStoreIn( stores, "faultsign" );
                    if ( !listed )
                        PumpFor( 10 );
                }
                ASSERT_TRUE( listed );

                // what it may not: every operation of the sign in and of the keyring is refused, and nothing happens at the store
                CAssetServiceResult refused;
                ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == mcp.BeginSignIn( CString::Empty, "faultsign", refused ) && !refused.message.IsNULLOrEmpty() );
                ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == mcp.CancelSignIn( CString::Empty, "faultsign", refused ) );
                ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == mcp.SignOut( CString::Empty, "faultsign", refused ) );
                CAssetSignInPromptPage prompts;
                ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == mcp.GetSignInPrompts( prompts ) );
                ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == mcp.AnswerPrompt( CString::Empty, "faultsign-1", AUTH::CSecretString( CString( "RIGHTCODE1234" ) ), refused ) );
                ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == mcp.UnlockKeyring( CString::Empty, AUTH::CSecretString( CString( "KEYRING-CODE-123" ) ), refused ) );
                ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == mcp.LockKeyring( CString::Empty, refused ) );
                CFutureResult refusedAsync = mcp.BeginSignInAsync( CString::Empty, "faultsign" );
                ASSERT_TRUE( WaitForFuture( refusedAsync, 10000 ) && ASSETSTORE_STATUS_REFUSED == CIAssetStoreService::GetStatusOfFuture( refusedAsync ) );
                CAssetSignInPromptPage ownerPrompts0;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == client.GetSignInPrompts( ownerPrompts0 ) && ownerPrompts0.entries.empty() );

                // the owner device begins: its session gets the prompts, the one of the machine caller does not
                CAssetServiceResult begun;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == client.BeginSignIn( CString::Empty, "faultsign", begun ) );
                for ( UInt32 waited=0; waited<5000 && ownerPrompts.prompts.size() < 2; waited+=10 )
                    PumpFor( 10 );
                PumpFor( 200 );
                ASSERT_TRUE( 2 == ownerPrompts.prompts.size() && mcpPrompts.prompts.empty() && mcpPrompts.closed.empty() );
                ASSERT_TRUE( mcpPrompts.dump.HasSubstr( "login.fake.invalid" ) < 0 );

                // and the machine caller cannot answer the step of the owner: the sign in still waits
                CString codePromptId;
                for ( size_t i=0; i<ownerPrompts.prompts.size(); ++i )
                    if ( "enterCode" == ownerPrompts.prompts[ i ].kind )
                        codePromptId = ownerPrompts.prompts[ i ].promptId;
                ASSERT_TRUE( !codePromptId.IsNULLOrEmpty() );
                ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == mcp.AnswerPrompt( CString::Empty, codePromptId, AUTH::CSecretString( CString( "RIGHTCODE1234" ) ), refused ) );
                PumpFor( 200 );
                CAssetStoreInfoPage ownerStores;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == client.ListStores( ownerStores ) && ServiceSuiteFindStoreInState( ownerStores, "faultsign", ASSETSTORE_SIGNINSTATE_IN_PROGRESS ) );
                ASSERT_TRUE( mcpPrompts.dump.HasSubstr( "RIGHTCODE1234" ) < 0 );

                // the owner finishes it
                CAssetServiceResult cancelled;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == client.CancelSignIn( CString::Empty, "faultsign", cancelled ) );
                mcp.Close();
                PumpFor( 300 );

                // and the settings of the host do not let the role mcp in as a sign in role
                CAssetStoreServiceHostSettings hostSettings;
                ASSERT_TRUE( 1 == hostSettings.signInRoles.size() && 1 == hostSettings.signInRoles.count( "owner" ) );
                CDataNode withMcp( "AssetStoreHost", GUCEF_DATATYPE_OBJECT );
                withMcp.SetAttribute( "signInRoles", "owner,mcp" );
                ASSERT_FALSE( hostSettings.LoadConfig( withMcp ) );
                CAssetStoreServiceHostSettings accepted;
                CDataNode withCurator( "AssetStoreHost", GUCEF_DATATYPE_OBJECT );
                withCurator.SetAttribute( "signInRoles", "owner,curator" );
                ASSERT_TRUE( accepted.LoadConfig( withCurator ) && 2 == accepted.signInRoles.size() );
            }
            CAssetStoreGlobal::Instance()->GetRegistry().RemoveProvider( "faultsign" );
            CAssetStoreGlobal::Instance()->GetProviderFactory().UnregisterConcreteFactory( "faultsign" );
            // the clients read the list of the stores again when it changes: that call ends before the next test
            PumpFor( 500 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    client.Close();
    clientPtr.Unlink();
    world.StopServer();
    UnmountFaultServer( "roles" );
}

/*-------------------------------------------------------------------------*/

void
PerformAssetStoreRemoteLiveSmokeTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "AssetStoreRemoteLiveSmoke" );

    const char* endpoint = getenv( "GUCEF_TEST_ASSETSTORE_REMOTE_ENDPOINT" );
    const char* code = getenv( "GUCEF_TEST_ASSETSTORE_PAIRING_CODE" );
    if ( GUCEF_NULL == endpoint || '\0' == endpoint[ 0 ] || GUCEF_NULL == code || '\0' == code[ 0 ] )
    {
        GUCEF_TESTFW_TESTCASE( "Live: a running asset store service (GUCEF_TEST_ASSETSTORE_REMOTE_ENDPOINT, GUCEF_TEST_ASSETSTORE_PAIRING_CODE)" )
            GUCEF_TESTFW_SKIP( "GUCEF_TEST_ASSETSTORE_REMOTE_ENDPOINT or GUCEF_TEST_ASSETSTORE_PAIRING_CODE is not set" );
        GUCEF_TESTFW_TESTCASE_END;
        return;
    }

    // GUCEF_TEST_ASSETSTORE_FIND_SERVICE=1: a finder with the default settings looks for the service on the local network first
    const char* find = getenv( "GUCEF_TEST_ASSETSTORE_FIND_SERVICE" );
    if ( GUCEF_NULL != find && '1' == find[ 0 ] )
    {
        GUCEF_TESTFW_TESTCASE( "Live: a finder with the default settings finds the running service on the local network" )
            try
            {
                COMCORE::CServiceFinderSettings finderSettings;
                finderSettings.serviceType = CRemoteAssetStoreServiceSettings::DefaultServiceType;
                COMCORE::CServiceFinder::TServiceCandidateVector candidates;
                ASSERT_TRUE( COMCORE::CServiceFinder::FindOnce( finderSettings, candidates ) && !candidates.empty() );
                for ( size_t i=0; i<candidates.size(); ++i )
                    GUCEF_LOG( LOGLEVEL_NORMAL, "AssetStoreRemoteLiveSmoke: found " << candidates[ i ].announcement.displayName << " at " << candidates[ i ].GetFirstEndpoint() );
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END;
    }

    // the pairing listener: the setting, else the session host with the pairing port of the design
    CRemoteAssetStoreServiceSettings parsed;
    parsed.endpoint = endpoint;
    CString hostAndPort = parsed.GetHostAndPort();
    const Int32 colon = hostAndPort.HasChar( ':', false );
    const CString hostName = colon > 0 ? hostAndPort.SubstrToIndex( (UInt32) colon, true ) : hostAndPort;
    const char* pairingEndpoint = getenv( "GUCEF_TEST_ASSETSTORE_PAIRING_ENDPOINT" );
    const CString pairing = GUCEF_NULL != pairingEndpoint && '\0' != pairingEndpoint[ 0 ] ? CString( pairingEndpoint ) : "tcp://" + hostName + ":47401";
    const char* serviceName = getenv( "GUCEF_TEST_ASSETSTORE_SERVICE_NAME" );
    const char* sessionName = getenv( "GUCEF_TEST_ASSETSTORE_DRN_SESSION" );
    AUTH::CMemoryCredentialStore store;
    TAssetStoreServicePtr service;
    CString packId;

    GUCEF_TESTFW_TESTCASE( "Live: this device pairs with the code, connects and the service describes itself" )
        try
        {
            CString error;
            AUTH::CSecretString secret( ( CString( code ) ) );
            const bool paired = CRemoteAssetStoreService::PairDevice( pairing, secret, GUCEF_NULL != serviceName ? CString( serviceName ) : CString( "gucef.assetstore" ),
                                                                      GUCEF_NULL != sessionName ? CString( sessionName ) : CString( "assetstore" ),
                                                                      "live smoke test", "probe", store, "live", error );
            secret.Clear();
            if ( !paired )
                GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "AssetStoreRemoteLiveSmoke: pairing failed: " << error );
            ASSERT_TRUE( paired );
            CAssetStoreServiceConfig config;
            config.implementationId = CAssetStoreServiceConfig::RemoteImplementationId;
            config.credentialStore = &store;
            config.implementationSettings.SetName( "service" );
            config.implementationSettings.SetNodeType( GUCEF_DATATYPE_OBJECT );
            config.implementationSettings.SetAttribute( "endpoint", CString( endpoint ) );
            config.implementationSettings.SetAttribute( "credentialsSlot", "live" );
            config.implementationSettings.SetAttribute( "clientKind", "probe" );
            config.implementationSettings.SetAttribute( "lanDiscovery", false );
            service = CAssetStoreGlobal::Instance()->CreateService( config, error );
            ASSERT_TRUE( !service.IsNULL() );
            if ( !service.IsNULL() )
            {
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == service->Open() );
                CAssetServiceInfo info;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == service->GetServiceInfo( info ) && "remote" == info.implementationId && 1 == info.roles.count( "owner" ) );
                ASSERT_TRUE( info.HasCapability( "catalogue" ) && info.HasCapability( "uploads" ) );
                GUCEF_LOG( LOGLEVEL_NORMAL, "AssetStoreRemoteLiveSmoke: the service has " << info.locations.size() << " locations, revision " << info.catalogueRevision );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    if ( service.IsNULL() || !service->IsCatalogueAvailable() )
        return;

    GUCEF_TESTFW_TESTCASE( "Live: a search of the catalogue, an upload of a generated pack ingested and read back with its digest, the job list" )
        try
        {
            CAssetSearchQuery all;
            all.pageSize = 50;
            CAssetSearchResultPage page;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service->Search( all, page ) );
            GUCEF_LOG( LOGLEVEL_NORMAL, "AssetStoreRemoteLiveSmoke: the catalogue has " << page.total << " results" );

            // content of this run only, so the pack is never a duplicate of an earlier run
            const UInt32 seed = (UInt32) ( DRNUTILS::CRemoteServiceHost::NowUtcMs() % 1000000 );
            CDynamicBuffer tga;
            MakeTga( 64, 64, tga );
            CDynamicBuffer pattern;
            MakePattern( 4096, seed, pattern );
            memcpy( static_cast< UInt8* >( tga.GetBufferPtr() ) + 18, pattern.GetConstBufferPtr(), 4096 );
            CString::StringVector paths;
            GUCEF::vector< CDynamicBuffer > contents;
            paths.push_back( "Textures/T_Smoke_" + ToString( seed ) + ".tga" );
            contents.push_back( tga );
            CAssetServiceResult committed;
            const TAssetStoreStatus status = UploadPack( *service, "smoke-" + ToString( seed ), "Smoke " + ToString( seed ), paths, contents, committed );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == status && !committed.relatedId.IsNULLOrEmpty() );
            CAssetFileRecord placed;
            ASSERT_TRUE( FindPackFile( *service, committed.relatedId, Sha256Of( tga ), placed ) );
            CDynamicBuffer readBack;
            ASSERT_TRUE( ReadWholeFile( *service, placed.path, tga.GetDataSize(), readBack ) && Sha256Of( readBack ) == Sha256Of( tga ) );
            CAssetJobPage jobs;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service->ListJobs( CString::Empty, 0, jobs ) );
            GUCEF_LOG( LOGLEVEL_NORMAL, "AssetStoreRemoteLiveSmoke: uploaded pack " << committed.relatedId << ", " << jobs.total << " jobs" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    service->Close();
}

/*-------------------------------------------------------------------------*/
