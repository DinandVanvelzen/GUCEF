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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */
#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */
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
#ifndef GUCEF_ASSETSTORE_CASSETCONTENTANALYZERREGISTRY_H
#include "gucefASSETSTORE_CAssetContentAnalyzerRegistry.h"
#define GUCEF_ASSETSTORE_CASSETCONTENTANALYZERREGISTRY_H
#endif /* GUCEF_ASSETSTORE_CASSETCONTENTANALYZERREGISTRY_H ? */
#ifndef GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H
#include "gucefASSETSTORE_CPrimaryAssetStore.h"
#define GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H
#endif /* GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETCONVERSIONEVENTDATA_H
#include "gucefASSETSTORE_CAssetConversionEventData.h"
#define GUCEF_ASSETSTORE_CASSETCONVERSIONEVENTDATA_H
#endif /* GUCEF_ASSETSTORE_CASSETCONVERSIONEVENTDATA_H ? */
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
#ifndef TESTMODELANALYZER_H
#include "TestModelAnalyzer.h"
#define TESTMODELANALYZER_H
#endif /* TESTMODELANALYZER_H ? */
#ifndef TESTFAKECONVERTERHOOK_H
#include "TestFakeConverterHook.h"
#define TESTFAKECONVERTERHOOK_H
#endif /* TESTFAKECONVERTERHOOK_H ? */
#include "TestConversion.h"

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

/** What one run of the suite works with */
struct SConversionFixture
{
    CString label;
    CString base;                               /**< VFS root of the folders of the run */
    COMCORE::CDatabaseClientSettings database;
    AUTH::ICredentialStore* credentials;

    SConversionFixture( void ) : label(), base(), database(), credentials( GUCEF_NULL ) {}
};

CPrimaryAssetStoreSettings
MakeSettings( const SConversionFixture& fixture )
{
    CPrimaryAssetStoreSettings settings;
    settings.locations.clear();
    CAssetStorageLocation main;
    main.locationId = "main";
    main.name = "Main";
    main.vfsRoot = fixture.base + "/main";
    settings.locations.push_back( main );
    CAssetStorageLocation mirror;
    mirror.locationId = "mirror";
    mirror.name = "Mirror";
    mirror.vfsRoot = fixture.base + "/mirror";
    settings.locations.push_back( mirror );
    settings.defaultLocationId = "main";
    settings.database = fixture.database;
    settings.analysisConcurrency = 2;
    if ( GUCEF_NULL != fixture.credentials )
    {
        settings.credentialProvider = "gucefASSETSTORE.test";
        settings.credentialAccount = "catalogue";
    }
    return settings;
}

/** Counts the conversion events of the store */
class CConversionEventRecorder : public CObserver
{
    public:

    UInt32 progress;
    UInt32 progressOfIngests;
    UInt32 finished;
    UInt32 failed;
    CString lastFinishedStatus;
    CString lastFailedStatus;

    CConversionEventRecorder( void ) : CObserver(), progress( 0 ), progressOfIngests( 0 ), finished( 0 ), failed( 0 ), lastFinishedStatus(), lastFailedStatus() {}
    virtual ~CConversionEventRecorder() { UnsubscribeAllFromObserver(); }

    void Observe( CPrimaryAssetStore& store )
    {
        TEventCallback onProgress( this, &CConversionEventRecorder::OnProgress );
        TEventCallback onFinished( this, &CConversionEventRecorder::OnFinished );
        TEventCallback onFailed( this, &CConversionEventRecorder::OnFailed );
        SubscribeTo( &store, CPrimaryAssetStore::ConversionProgressEvent, onProgress );
        SubscribeTo( &store, CPrimaryAssetStore::ConversionFinishedEvent, onFinished );
        SubscribeTo( &store, CPrimaryAssetStore::ConversionFailedEvent, onFailed );
    }

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CString classTypeName = "ASSETSTORETEST::CConversionEventRecorder";
        return classTypeName;
    }

    private:

    typedef CTEventHandlerFunctor< CConversionEventRecorder > TEventCallback;

    void OnProgress( CNotifier* notifier, const CEvent& eventId, CICloneable* eventData )
    {
        ++progress;
        const CAssetConversionEventData* data = static_cast< const CAssetConversionEventData* >( eventData );
        if ( GUCEF_NULL != data && data->forIngest )
            ++progressOfIngests;
    }

    void OnFinished( CNotifier* notifier, const CEvent& eventId, CICloneable* eventData )
    {
        ++finished;
        const CAssetConversionEventData* data = static_cast< const CAssetConversionEventData* >( eventData );
        if ( GUCEF_NULL != data )
            lastFinishedStatus = data->status;
    }

    void OnFailed( CNotifier* notifier, const CEvent& eventId, CICloneable* eventData )
    {
        ++failed;
        const CAssetConversionEventData* data = static_cast< const CAssetConversionEventData* >( eventData );
        if ( GUCEF_NULL != data )
            lastFailedStatus = data->status;
    }
};

bool
WriteText( const CString& path, const CString& text )
{
    return WriteVfsFile( path, text );
}

/** A pack of two models and a texture, with the licence of its pack.json */
bool
WriteModelPack( const CString& folder, const CString& title, const CString& licenceJson )
{
    bool written = WriteText( folder + "/pack.json", "{ \"title\": \"" + title + "\", \"author\": \"Forge Works\", \"licence\": " + licenceJson + " }" );
    // every pack has files of its own: a pack whose files the catalogue has all of is a duplicate
    written = WriteText( folder + "/Meshes/SM_Wep_Sword_01.fbx", "triangles=1200;vertices=900;rigged=0;animated=0;pack=" + title ) && written;
    written = WriteText( folder + "/Meshes/SM_Wep_Axe_01.fbx", "triangles=6000;vertices=4000;rigged=0;pack=" + title ) && written;
    CDynamicBuffer png;
    written = MakePng( 64, 32, false, png ) && WriteVfsFile( folder + "/Textures/T_Sword_D.png", png ) && written;
    return written;
}

const CAssetRecord*
FindAsset( const TAssetRecordVector& assets, const CString& name )
{
    for ( size_t a=0; a<assets.size(); ++a )
        if ( assets[ a ].name == name )
            return &assets[ a ];
    return GUCEF_NULL;
}

UInt64
CountRows( CPrimaryAssetStore& store, const CString& table )
{
    CAssetCatalogue::TRowVector rows;
    store.GetCatalogue().SelectAll( table, rows );
    return rows.size();
}

bool
PackHasHit( CPrimaryAssetStore& store, const CString& packId )
{
    CAssetSearchQuery query;
    query.pageSize = 100;
    query.packIds.insert( packId );
    CAssetSearchResultPage page;
    return ASSETSTORE_STATUS_OK == store.Search( query, page ) && !page.hits.empty();
}

/** Ingests a pack of the source folder with the keep policy; the request is the one of a pack on disk */
TAssetStoreStatus
IngestPack( CPrimaryAssetStore& store, const CString& folder, const CString& keep, CAssetIngestResult& result )
{
    CAssetIngestRequest request;
    request.sourceId = "disk";
    request.contentPath = folder;
    request.keep = keep;
    return store.Ingest( request, result );
}

/** Pumps until the hook has started waiting for a cancel request */
bool
PumpUntilWaiting( CTestConverterHook& hook, UInt32 timeoutInMs )
{
    for ( UInt32 waited=0; waited<timeoutInMs; waited+=5 )
    {
        if ( hook.GetStartedWaitCount() > 0 )
            return true;
        PumpFor( 5 );
    }
    return hook.GetStartedWaitCount() > 0;
}

const char* const CCBY = "{ \"name\": \"CC BY 4.0\" }";

/** The suite, against the database of the fixture */
void
RunConversionSuite( SConversionFixture& fixture )
{
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CString& base = fixture.base;
    MountFreshDirectory( fixture.label + "_cv_main", base + "/main" );
    MountFreshDirectory( fixture.label + "_cv_mirror", base + "/mirror" );
    MountFreshDirectory( fixture.label + "_cv_source", base + "/source" );

    // the analyser of models of the tests: a model without it has no properties, and the conversion does not need them
    CTestModelAnalyzer* analyzer = GUCEF_NEW CTestModelAnalyzer();
    TAssetContentAnalyzerPtr analyzerPtr( analyzer );
    CAssetStoreGlobal::Instance()->GetContentAnalyzerRegistry().Register( analyzerPtr );

    bool sourcesWritten = true;
    const char* const packNames[] = { "Conv_Both", "Conv_Only", "Conv_Original", "Conv_Refused", "Conv_Failed", "Conv_Cancel", "Conv_IoError", "Conv_Later", "Conv_LaterRefused", "Conv_Batch" };
    for ( UInt32 p=0; p<sizeof( packNames ) / sizeof( packNames[ 0 ] ); ++p )
        sourcesWritten = WriteModelPack( base + "/source/" + packNames[ p ], CString( packNames[ p ] ).ReplaceChar( '_', ' ' ), CCBY ) && sourcesWritten;
    sourcesWritten = WriteText( base + "/mirror/Rocks/SM_Rock_01.fbx", "triangles=300;vertices=200" ) && sourcesWritten;

    CPrimaryAssetStore store;
    store.SetSettings( MakeSettings( fixture ) );
    store.SetCredentialStore( fixture.credentials );
    CTestConverterHook hook;
    store.SetConverterHook( &hook );
    CConversionEventRecorder events;
    events.Observe( store );

    CString bothId;
    CString onlyId;
    CString laterId;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": the catalogue is at the schema version with the conversions table" ).C_String() )
        try
        {
            ASSERT_TRUE( sourcesWritten );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Open() );
            ASSERT_TRUE( CAssetCatalogue::CurrentSchemaVersion == store.GetCatalogue().GetSchemaVersion() );
            ASSERT_TRUE( 0 == CountRows( store, "pack_conversions" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": keep both: the models are converted by the hook and the originals stay, with the conversion in the catalogue" ).C_String() )
        try
        {
            CAssetIngestResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == IngestPack( store, base + "/source/Conv_Both", "both", result ) );
            ASSERT_TRUE( ASSETPACK_STATE_INGESTED == result.state && 3 == result.assetCount );
            bothId = result.packId;
            PumpFor( 50 );
            ASSERT_TRUE( 1 == hook.GetCallCount() );
            ASSERT_TRUE( events.progressOfIngests >= 3 );

            CAssetPackRecord pack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( bothId, pack ) );
            ASSERT_TRUE( ASSETPACK_STATE_INGESTED == pack.state && pack.stateReason.IsNULLOrEmpty() );
            ASSERT_TRUE( pack.formatsConverted.end() != pack.formatsConverted.find( "gltf" ) && 1 == pack.formatsConverted.size() );
            ASSERT_TRUE( CAssetConversionRecord::OutcomeConverted == pack.conversion.outcome && "test" == pack.conversion.profile );
            ASSERT_TRUE( pack.conversion.converters.end() != pack.conversion.converters.find( "importer:test/1" ) );
            ASSERT_TRUE( 2 == pack.conversion.convertedCount && pack.conversion.reportPath == "converted/test/report.json" && pack.conversion.convertedAtMs > 0 );
            ASSERT_TRUE( 1 == CountRows( store, "pack_conversions" ) );

            TAssetRecordVector assets;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetAssets( bothId, assets ) && 3 == assets.size() );
            const CAssetRecord* sword = FindAsset( assets, "SM_Wep_Sword_01" );
            const CAssetRecord* texture = FindAsset( assets, "T_Sword_D" );
            ASSERT_TRUE( GUCEF_NULL != sword && GUCEF_NULL != texture );
            if ( GUCEF_NULL != sword && GUCEF_NULL != texture )
            {
                ASSERT_TRUE( "converted/test/SM_Wep_Sword_01.gltf" == sword->convertedPath && "gltf" == sword->convertedFormat && "fbx" == sword->sourceFormat );
                ASSERT_TRUE( texture->convertedPath.IsNULLOrEmpty() && texture->convertedFormat.IsNULLOrEmpty() );
                ASSERT_TRUE( 1 == sword->files.size() );
            }

            // the placed layout: originals and converted files side by side
            const CString root = base + "/main/" + pack.rootPath;
            ASSERT_TRUE( vfs.FileExists( root + "/original/Meshes/SM_Wep_Sword_01.fbx" ) );
            ASSERT_TRUE( vfs.FileExists( root + "/converted/test/SM_Wep_Sword_01.gltf" ) && vfs.FileExists( root + "/converted/test/report.json" ) );
            CString::StringVector pending;
            store.GetPendingIngests( pending );
            ASSERT_TRUE( pending.empty() );

            // the converted file resolves, the original too
            if ( GUCEF_NULL != sword )
            {
                CString vfsPath, message;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetAssetConvertedVfsPath( sword->assetId, vfsPath, message ) );
                ASSERT_TRUE( vfsPath == root + "/converted/test/SM_Wep_Sword_01.gltf" && vfs.FileExists( vfsPath ) );
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetAssetVfsPath( sword->assetId, vfsPath, message ) && vfs.FileExists( vfsPath ) );
            }
            if ( GUCEF_NULL != texture )
            {
                CString vfsPath, message;
                ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == store.GetAssetConvertedVfsPath( texture->assetId, vfsPath, message ) );
            }

            // the format is searchable
            CAssetSearchQuery query;
            query.pageSize = 100;
            query.formats.insert( "gltf" );
            query.packIds.insert( bothId );
            CAssetSearchResultPage page;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && !page.hits.empty() );

            GUCEF::vector< CAssetIngestHistoryEntry > history;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetIngestHistory( history, bothId ) );
            ASSERT_TRUE( 1 == history.size() && "ingested" == history[ 0 ].outcome );

            CAssetVerificationReport report;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.VerifyLocation( "main", report ) && report.IsIntact() && report.filesChecked >= 3 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": keep converted: the originals of what converted go, after a conversion without errors, and the catalogue says so" ).C_String() )
        try
        {
            CAssetIngestResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == IngestPack( store, base + "/source/Conv_Only", "converted", result ) );
            ASSERT_TRUE( ASSETPACK_STATE_INGESTED == result.state );
            onlyId = result.packId;
            CAssetPackRecord pack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( onlyId, pack ) && ASSETKEEP_CONVERTED == pack.keep );
            const CString root = base + "/main/" + pack.rootPath;
            TAssetRecordVector assets;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetAssets( onlyId, assets ) );
            const CAssetRecord* sword = FindAsset( assets, "SM_Wep_Sword_01" );
            const CAssetRecord* texture = FindAsset( assets, "T_Sword_D" );
            ASSERT_TRUE( GUCEF_NULL != sword && GUCEF_NULL != texture );
            if ( GUCEF_NULL != sword && GUCEF_NULL != texture )
            {
                ASSERT_TRUE( !sword->convertedPath.IsNULLOrEmpty() && sword->files.empty() );
                ASSERT_TRUE( texture->convertedPath.IsNULLOrEmpty() && 1 == texture->files.size() );
                ASSERT_FALSE( vfs.FileExists( root + "/original/Meshes/SM_Wep_Sword_01.fbx" ) );
                ASSERT_FALSE( vfs.FileExists( root + "/original/Meshes/SM_Wep_Axe_01.fbx" ) );
                ASSERT_TRUE( vfs.FileExists( root + "/original/Textures/T_Sword_D.png" ) );
                ASSERT_TRUE( vfs.FileExists( root + "/converted/test/SM_Wep_Sword_01.gltf" ) );
                CString vfsPath, message;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetAssetConvertedVfsPath( sword->assetId, vfsPath, message ) && vfs.FileExists( vfsPath ) );
            }
            CAssetVerificationReport report;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.VerifyLocation( "main", report ) && report.IsIntact() );

            // the original is not there to convert again
            CAssetConversionOutcome outcome;
            ASSERT_TRUE( ASSETSTORE_STATUS_UNSUPPORTED == store.ConvertPack( onlyId, outcome ) );
            ASSERT_TRUE( outcome.message.HasSubstr( "originals" ) >= 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": keep original: the converter is not asked" ).C_String() )
        try
        {
            const UInt32 calls = hook.GetCallCount();
            CAssetIngestResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == IngestPack( store, base + "/source/Conv_Original", "original", result ) );
            ASSERT_TRUE( ASSETPACK_STATE_INGESTED == result.state && calls == hook.GetCallCount() );
            CAssetPackRecord pack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( result.packId, pack ) );
            ASSERT_TRUE( pack.formatsConverted.empty() && !pack.conversion.IsSet() && 0 == vfs.DirExists( base + "/main/" + pack.rootPath + "/converted" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a refusal is an ingest outcome, not a failure: the pack is original with the state conversion refused and the reason" ).C_String() )
        try
        {
            hook.SetMode( CTestConverterHook::MODE_REFUSE );
            CAssetIngestResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == IngestPack( store, base + "/source/Conv_Refused", "both", result ) );
            ASSERT_TRUE( ASSETPACK_STATE_CONVERSION_REFUSED == result.state && ASSETSTORE_STATUS_OK == result.status );
            bool noted = false;
            for ( size_t p=0; p<result.problems.size(); ++p )
                noted = noted || 0 == result.problems[ p ].HasSubstr( "conversion refused: the licence record does not allow it", true );
            ASSERT_TRUE( noted );
            CAssetPackRecord pack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( result.packId, pack ) );
            ASSERT_TRUE( ASSETPACK_STATE_CONVERSION_REFUSED == pack.state && pack.stateReason == "conversion refused: the licence record does not allow it" );
            ASSERT_TRUE( CAssetConversionRecord::OutcomeRefused == pack.conversion.outcome && pack.formatsConverted.empty() );
            const CString root = base + "/main/" + pack.rootPath;
            ASSERT_TRUE( vfs.FileExists( root + "/original/Meshes/SM_Wep_Sword_01.fbx" ) );
            ASSERT_FALSE( vfs.FileExists( root + "/converted/test/SM_Wep_Sword_01.gltf" ) );
            TAssetRecordVector assets;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetAssets( result.packId, assets ) );
            for ( size_t a=0; a<assets.size(); ++a )
                ASSERT_TRUE( assets[ a ].convertedPath.IsNULLOrEmpty() );
            ASSERT_TRUE( PackHasHit( store, result.packId ) );
            GUCEF::vector< CAssetIngestHistoryEntry > history;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetIngestHistory( history, result.packId ) && 1 == history.size() );
            ASSERT_TRUE( !history.empty() && "ingested" == history[ 0 ].outcome && !history[ 0 ].problems.empty() );

            // the same pack, the licence now in order: it converts later
            hook.SetMode( CTestConverterHook::MODE_CONVERT );
            CAssetConversionOutcome outcome;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.ConvertPack( result.packId, outcome ) );
            ASSERT_TRUE( ASSETPACK_STATE_INGESTED == outcome.state && 2 == outcome.record.convertedCount );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( result.packId, pack ) );
            ASSERT_TRUE( ASSETPACK_STATE_INGESTED == pack.state && pack.stateReason.IsNULLOrEmpty() && CAssetConversionRecord::OutcomeConverted == pack.conversion.outcome );
            ASSERT_TRUE( vfs.FileExists( root + "/converted/test/SM_Wep_Sword_01.gltf" ) && !pack.formatsConverted.empty() );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetIngestHistory( history, result.packId ) && 2 == history.size() && "converted" == history[ 1 ].outcome );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a conversion with errors leaves the original, marks the pack pending with the reason, and can be repeated" ).C_String() )
        try
        {
            hook.SetMode( CTestConverterHook::MODE_FAIL );
            CAssetIngestResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == IngestPack( store, base + "/source/Conv_Failed", "both", result ) );
            ASSERT_TRUE( ASSETPACK_STATE_CONVERSION_PENDING == result.state );
            CAssetPackRecord pack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( result.packId, pack ) );
            ASSERT_TRUE( ASSETPACK_STATE_CONVERSION_PENDING == pack.state && pack.stateReason.HasSubstr( "conversion failed: 1 error(s)" ) == 0 );
            ASSERT_TRUE( CAssetConversionRecord::OutcomeFailed == pack.conversion.outcome && pack.formatsConverted.empty() );
            bool kept = false;
            for ( size_t p=0; p<result.problems.size(); ++p )
                kept = kept || result.problems[ p ].HasSubstr( "kept as original" ) >= 0;
            ASSERT_TRUE( kept );
            const CString root = base + "/main/" + pack.rootPath;
            ASSERT_TRUE( vfs.FileExists( root + "/original/Meshes/SM_Wep_Sword_01.fbx" ) && vfs.FileExists( root + "/original/Textures/T_Sword_D.png" ) );
            TAssetRecordVector assets;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetAssets( result.packId, assets ) );
            for ( size_t a=0; a<assets.size(); ++a )
                ASSERT_TRUE( assets[ a ].convertedPath.IsNULLOrEmpty() );

            // the same failure again, later: the pack stays as it is, the history has it
            CAssetConversionOutcome outcome;
            ASSERT_TRUE( ASSETSTORE_STATUS_FAILED == store.ConvertPack( result.packId, outcome ) );
            ASSERT_TRUE( ASSETPACK_STATE_CONVERSION_PENDING == outcome.state && !outcome.problems.empty() );
            GUCEF::vector< CAssetIngestHistoryEntry > history;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetIngestHistory( history, result.packId ) && 2 == history.size() && "conversionFailed" == history[ 1 ].outcome );

            // then in order: the pending packs are converted in one run
            hook.SetMode( CTestConverterHook::MODE_CONVERT );
            CString::StringVector pendingPacks;
            store.GetPacksPendingConversion( pendingPacks );
            ASSERT_TRUE( 1 == pendingPacks.size() && pendingPacks[ 0 ] == result.packId );
            CAssetConversionOutcome batch;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.ConvertPendingPacks( batch ) );
            ASSERT_TRUE( 1 == batch.packsConverted && 0 == batch.packsFailed && 0 == batch.packsRefused );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( result.packId, pack ) && ASSETPACK_STATE_INGESTED == pack.state && !pack.formatsConverted.empty() );
            ASSERT_TRUE( vfs.FileExists( root + "/converted/test/SM_Wep_Axe_01.gltf" ) );
            store.GetPacksPendingConversion( pendingPacks );
            ASSERT_TRUE( pendingPacks.empty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": cancel of an ingest in its conversion: the pack stays staged and a resume converts it" ).C_String() )
        try
        {
            hook.SetMode( CTestConverterHook::MODE_WAIT_FOR_CANCEL );
            CAssetIngestRequest request;
            request.sourceId = "disk";
            request.contentPath = base + "/source/Conv_Cancel";
            request.keep = "both";
            CFutureResult future = store.IngestAsync( request );
            ASSERT_TRUE( future.HasAFuture() );
            ASSERT_TRUE( PumpUntilWaiting( hook, 20000 ) );
            future.RequestCancellation();
            ASSERT_TRUE( WaitForFuture( future, 20000 ) );
            CTaskPtr task = future.GetResult( 0 );
            ASSERT_TRUE( !task.IsNULL() && TASKSTATUS_CANCELLED == task->GetTaskStatus() );

            CString::StringVector pending;
            store.GetPendingIngests( pending );
            ASSERT_TRUE( 1 == pending.size() );
            CAssetCatalogue::TRowVector rows;
            CDataTableQuery query;
            query.WhereEquals( "state", CVariant( CString( "failed" ) ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetCatalogue().Select( "packs", query, rows ) && 1 == rows.size() );
            if ( 1 == rows.size() )
                ASSERT_TRUE( CAssetCatalogue::GetText( rows[ 0 ], "state_reason" ).HasSubstr( "step convert" ) == 0 );

            hook.SetMode( CTestConverterHook::MODE_CONVERT );
            CAssetIngestResult resumed;
            ASSERT_TRUE( 1 == pending.size() && ASSETSTORE_STATUS_OK == store.ResumeIngest( pending[ 0 ], resumed ) );
            ASSERT_TRUE( ASSETPACK_STATE_INGESTED == resumed.state );
            CAssetPackRecord pack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( resumed.packId, pack ) && !pack.formatsConverted.empty() );
            ASSERT_TRUE( vfs.FileExists( base + "/main/" + pack.rootPath + "/converted/test/SM_Wep_Sword_01.gltf" ) );
            store.GetPendingIngests( pending );
            ASSERT_TRUE( pending.empty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a conversion that cannot run fails the convert step and a resume converts" ).C_String() )
        try
        {
            hook.SetMode( CTestConverterHook::MODE_IO_ERROR );
            CAssetIngestResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_IO_ERROR == IngestPack( store, base + "/source/Conv_IoError", "both", result ) );
            ASSERT_TRUE( ASSETINGEST_STEP_CONVERT == result.failedStep && ASSETPACK_STATE_FAILED == result.state );
            ASSERT_TRUE( result.message.HasSubstr( "cannot be made" ) >= 0 );
            CString::StringVector pending;
            store.GetPendingIngests( pending );
            ASSERT_TRUE( 1 == pending.size() );
            GUCEF::vector< CAssetIngestHistoryEntry > history;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetIngestHistory( history, result.packId ) && 1 == history.size() && "failed" == history[ 0 ].outcome && "convert" == history[ 0 ].step );

            hook.SetMode( CTestConverterHook::MODE_CONVERT );
            CAssetIngestResult resumed;
            ASSERT_TRUE( 1 == pending.size() && ASSETSTORE_STATUS_OK == store.ResumeIngest( pending[ 0 ], resumed ) );
            ASSERT_TRUE( ASSETPACK_STATE_INGESTED == resumed.state && result.packId == resumed.packId );
            CAssetPackRecord pack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( result.packId, pack ) && !pack.formatsConverted.empty() );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetIngestHistory( history, result.packId ) && 2 == history.size() && "ingested" == history[ 1 ].outcome );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": without a converter a pack is pending, and converts later as a pool task" ).C_String() )
        try
        {
            store.SetConverterHook( GUCEF_NULL );
            CAssetIngestResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == IngestPack( store, base + "/source/Conv_Later", "both", result ) );
            ASSERT_TRUE( ASSETPACK_STATE_CONVERSION_PENDING == result.state );
            laterId = result.packId;
            CAssetPackRecord pack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( laterId, pack ) && pack.stateReason.IsNULLOrEmpty() && !pack.conversion.IsSet() );
            CAssetConversionOutcome outcome;
            ASSERT_TRUE( ASSETSTORE_STATUS_UNSUPPORTED == store.ConvertPack( laterId, outcome ) );

            store.SetConverterHook( &hook );
            const UInt32 finishedBefore = events.finished;
            CFutureResult future = store.ConvertPackAsync( laterId );
            ASSERT_TRUE( GetFutureResult< CAssetConversionOutcome >( future, outcome ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == outcome.status && ASSETPACK_STATE_INGESTED == outcome.state && 2 == outcome.record.convertedCount );
            PumpFor( 50 );
            ASSERT_TRUE( events.finished > finishedBefore && "ok" == events.lastFinishedStatus );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( laterId, pack ) );
            ASSERT_TRUE( ASSETPACK_STATE_INGESTED == pack.state && !pack.formatsConverted.empty() && CAssetConversionRecord::OutcomeConverted == pack.conversion.outcome );
            TAssetRecordVector assets;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetAssets( laterId, assets ) );
            const CAssetRecord* sword = FindAsset( assets, "SM_Wep_Sword_01" );
            ASSERT_TRUE( GUCEF_NULL != sword && sword->convertedPath == "converted/test/SM_Wep_Sword_01.gltf" );
            ASSERT_TRUE( vfs.FileExists( base + "/main/" + pack.rootPath + "/converted/test/SM_Wep_Sword_01.gltf" ) );
            GUCEF::vector< CAssetIngestHistoryEntry > history;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetIngestHistory( history, laterId ) && 2 == history.size() );
            ASSERT_TRUE( 2 == history.size() && "ingested" == history[ 0 ].outcome && "converted" == history[ 1 ].outcome && "conversion" == history[ 1 ].origin );
            CAssetVerificationReport report;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.VerifyLocation( "main", report ) && report.IsIntact() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a refusal and a failure of a later conversion leave an earlier conversion as it is; cancel of the task of a conversion" ).C_String() )
        try
        {
            CAssetConversionOutcome outcome;
            hook.SetMode( CTestConverterHook::MODE_REFUSE );
            ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == store.ConvertPack( laterId, outcome ) );
            hook.SetMode( CTestConverterHook::MODE_FAIL );
            ASSERT_TRUE( ASSETSTORE_STATUS_FAILED == store.ConvertPack( laterId, outcome ) );
            CAssetPackRecord pack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( laterId, pack ) );
            ASSERT_TRUE( ASSETPACK_STATE_INGESTED == pack.state && CAssetConversionRecord::OutcomeConverted == pack.conversion.outcome && !pack.formatsConverted.empty() );
            ASSERT_TRUE( vfs.FileExists( base + "/main/" + pack.rootPath + "/converted/test/SM_Wep_Sword_01.gltf" ) );
            GUCEF::vector< CAssetIngestHistoryEntry > history;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetIngestHistory( history, laterId ) && 4 == history.size() );
            ASSERT_TRUE( 4 == history.size() && "conversionRefused" == history[ 2 ].outcome && "conversionFailed" == history[ 3 ].outcome );

            // a task of a conversion that is asked to stop ends cancelled and changes nothing
            hook.SetMode( CTestConverterHook::MODE_WAIT_FOR_CANCEL );
            const UInt32 waits = hook.GetStartedWaitCount();
            CFutureResult future = store.ConvertPackAsync( laterId );
            ASSERT_TRUE( future.HasAFuture() );
            for ( UInt32 w=0; w<4000 && hook.GetStartedWaitCount() == waits; ++w )
                PumpFor( 5 );
            ASSERT_TRUE( hook.GetStartedWaitCount() > waits );
            future.RequestCancellation();
            ASSERT_TRUE( WaitForFuture( future, 20000 ) );
            CTaskPtr task = future.GetResult( 0 );
            ASSERT_TRUE( !task.IsNULL() && TASKSTATUS_CANCELLED == task->GetTaskStatus() );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetIngestHistory( history, laterId ) && 4 == history.size() );
            PumpFor( 50 );
            ASSERT_TRUE( events.failed > 0 && "cancelled" == events.lastFailedStatus );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a conversion that a part of the converter made with another version is stale, and converting again makes it current" ).C_String() )
        try
        {
            hook.SetMode( CTestConverterHook::MODE_CONVERT );
            bool stale = true;
            CString reason;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.IsConversionStale( laterId, stale, reason ) && !stale );
            CString::StringVector stalePacks;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetStaleConversions( stalePacks ) && stalePacks.empty() );

            hook.SetVersion( 2 );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.IsConversionStale( laterId, stale, reason ) && stale );
            ASSERT_TRUE( reason.HasSubstr( "importer:test" ) >= 0 && reason.HasSubstr( "version 2" ) >= 0 );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetStaleConversions( stalePacks ) && stalePacks.size() >= 3 );
            bool listed = false;
            for ( size_t p=0; p<stalePacks.size(); ++p )
                listed = listed || stalePacks[ p ] == laterId;
            ASSERT_TRUE( listed );

            CAssetConversionOutcome outcome;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.ConvertPack( laterId, outcome ) );
            CAssetPackRecord pack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( laterId, pack ) );
            ASSERT_TRUE( pack.conversion.converters.end() != pack.conversion.converters.find( "importer:test/2" ) && 1 == pack.conversion.converters.size() );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.IsConversionStale( laterId, stale, reason ) && !stale );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetStaleConversions( stalePacks ) && stalePacks.size() >= 2 );
            hook.SetVersion( 1 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a pack referenced in place is pending with its reason and is not converted" ).C_String() )
        try
        {
            CAssetIngestRequest request;
            request.sourceId = "disk";
            request.contentPath = base + "/mirror/Rocks";
            request.keep = "both";
            request.copyContent = 0;
            CAssetIngestResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Ingest( request, result ) );
            ASSERT_TRUE( ASSETPACK_STATE_CONVERSION_PENDING == result.state );
            CAssetPackRecord pack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( result.packId, pack ) );
            ASSERT_TRUE( pack.stateReason.HasSubstr( "referenced in place" ) >= 0 );
            CAssetConversionOutcome outcome;
            ASSERT_TRUE( ASSETSTORE_STATUS_UNSUPPORTED == store.ConvertPack( result.packId, outcome ) );
            ASSERT_TRUE( outcome.message.HasSubstr( "referenced in place" ) >= 0 );

            // a batch over the pending packs leaves it alone, and its result is the one of the asynchronous run too
            CFutureResult future = store.ConvertPendingPacksAsync();
            CAssetConversionOutcome batch;
            ASSERT_TRUE( GetFutureResult< CAssetConversionOutcome >( future, batch ) );
            ASSERT_TRUE( 0 == batch.packsConverted && 0 == batch.packsFailed && 0 == batch.packsRefused );
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == store.ConvertPack( "pk-unknown", outcome ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a pack that is removed takes its conversion row with it" ).C_String() )
        try
        {
            const UInt64 rows = CountRows( store, "pack_conversions" );
            ASSERT_TRUE( rows > 0 );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.RemovePack( bothId, true ) );
            ASSERT_TRUE( rows - 1 == CountRows( store, "pack_conversions" ) );
            store.Close();
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Open() );
            CAssetPackRecord pack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( laterId, pack ) && CAssetConversionRecord::OutcomeConverted == pack.conversion.outcome );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    store.SetConverterHook( GUCEF_NULL );
    events.UnsubscribeAllFromObserver();
    store.Close();
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformConversionVfsTablesTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "ConversionVfsTables" );

    SConversionFixture fixture;
    fixture.label = "vfstables";
    fixture.base = "astest/cvvfstables";
    MountFreshDirectory( "cvvfstables_catalogue", "astest/cvvfstables/catalogue" );
    fixture.database.backendName = "vfstables";
    fixture.database.database = "astest/cvvfstables/catalogue/db";
    fixture.database.backendOptions.SetAllowMultipleValues( false );
    fixture.database.backendOptions.Set( "syncWrites", CVariant( "false" ) );
    RunConversionSuite( fixture );
    UnmountDirectory( "cvvfstables_catalogue" );
}

/*-------------------------------------------------------------------------*/

void
PerformConversionPostgreSqlTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "ConversionPostgreSql" );

    CString connectionString;
    if ( !GetPostgreSqlConnectionString( connectionString ) )
    {
        GUCEF_LOG( LOGLEVEL_IMPORTANT, "ConversionPostgreSql: GUCEF_TEST_POSTGRESQL is not set, the PostgreSQL tests are skipped" );
        GUCEF_TESTFW_TESTCASE( "PostgreSQL: the conversion of packs against a PostgreSQL test database" )
            GUCEF_TESTFW_SKIP( "GUCEF_TEST_POSTGRESQL is not set" );
        GUCEF_TESTFW_TESTCASE_END;
        return;
    }

    SConversionFixture fixture;
    fixture.label = "postgresql";
    fixture.base = "astest/cvpostgresql";
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
    fixture.database.backendOptions.Set( "maxRowsPerStatement", CVariant( "500" ) );

    // the password of the environment goes through a credential store, as the settings of an application would have it
    AUTH::CMemoryCredentialStore credentials;
    const char* password = getenv( "PGPASSWORD" );
    if ( GUCEF_NULL != password && '\0' != password[ 0 ] )
    {
        AUTH::CSecretString secret( ( CString( password ) ) );
        credentials.Store( "gucefASSETSTORE.test", "catalogue", secret );
        fixture.credentials = &credentials;
    }
    GUCEF_LOG( LOGLEVEL_NORMAL, "ConversionPostgreSql: testing in schema \"" << schema << "\"" );
    RunConversionSuite( fixture );

    // the schema of this run goes
    GUCEF_TESTFW_TESTCASE( "PostgreSQL: the schema of the conversion run is dropped" )
        try
        {
            CPrimaryAssetStoreSettings settings;
            settings.database = fixture.database;
            if ( GUCEF_NULL != fixture.credentials )
            {
                settings.credentialProvider = "gucefASSETSTORE.test";
                settings.credentialAccount = "catalogue";
            }
            COMCORE::CDatabaseClientSettings databaseSettings;
            ASSERT_TRUE( settings.GetDatabaseSettingsWithCredentials( fixture.credentials, databaseSettings, error ) );
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
