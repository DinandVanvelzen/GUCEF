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
#include <algorithm>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */
#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */
#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */
#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */
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
#ifndef GUCEF_ASSETSTORE_CASSETPACKINDEXLOADER_H
#include "gucefASSETSTORE_CAssetPackIndexLoader.h"
#define GUCEF_ASSETSTORE_CASSETPACKINDEXLOADER_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKINDEXLOADER_H ? */
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
#ifndef TESTPACKINDEXDOCUMENT_H
#include "TestPackIndexDocument.h"
#define TESTPACKINDEXDOCUMENT_H
#endif /* TESTPACKINDEXDOCUMENT_H ? */

#include "TestPackIndex.h"

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

struct SIndexFixture
{
    CString label;
    CString base;
    COMCORE::CDatabaseClientSettings database;
    AUTH::ICredentialStore* credentials;

    SIndexFixture( void ) : label(), base(), database(), credentials( GUCEF_NULL ) {}
};

CPrimaryAssetStoreSettings
MakeIndexSettings( const SIndexFixture& fixture )
{
    CPrimaryAssetStoreSettings settings;
    settings.locations.clear();
    CAssetStorageLocation main;
    main.locationId = "main";
    main.name = "Main";
    main.vfsRoot = fixture.base + "/main";
    settings.locations.push_back( main );
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

UInt64
CountRowsOfPack( CPrimaryAssetStore& store, const CString& table, const CString& packId )
{
    CDataTableQuery query;
    query.WhereEquals( "pack_id", CVariant( packId ) );
    CAssetCatalogue::TRowVector rows;
    store.GetCatalogue().Select( table, query, rows );
    return rows.size();
}

UInt64
CountAllRows( CPrimaryAssetStore& store, const CString& table )
{
    CAssetCatalogue::TRowVector rows;
    store.GetCatalogue().SelectAll( table, rows );
    return rows.size();
}

const CAssetRecord*
FindAssetByName( const TAssetRecordVector& assets, const CString& name )
{
    for ( size_t a=0; a<assets.size(); ++a )
        if ( assets[ a ].name == name )
            return &assets[ a ];
    return GUCEF_NULL;
}

/** The ids of the resources of the pack index of a pack in the order of the index, with their group and path: what two loads have to agree on */
CString
SnapshotOf( CPrimaryAssetStore& store, const CString& packId )
{
    CString text;
    TAssetPackResourceVector resources;
    if ( ASSETSTORE_STATUS_OK != store.GetPackIndexResources( packId, CString::Empty, CString::Empty, resources ) )
        return "failed";
    for ( size_t r=0; r<resources.size(); ++r )
        text += resources[ r ].resourceId + "|" + resources[ r ].groupId + "|" + resources[ r ].subGroup + "|" + resources[ r ].path + "|" + resources[ r ].assetId + "\n";
    TAssetPackGroupVector groups;
    store.GetPackIndexGroups( packId, groups );
    for ( size_t g=0; g<groups.size(); ++g )
        text += "G:" + groups[ g ].groupId + "/" + groups[ g ].subGroup + "=" + UInt64ToString( groups[ g ].resourceCount ) + "\n";
    return text;
}

/** Counts PackIndexUpdatedEvent of a store with the ids */
class CPackIndexEvents : public CObservingNotifier
{
    public:

    typedef CTEventHandlerFunctor< CPackIndexEvents > TEventCallback;

    CString::StringVector ids;

    explicit CPackIndexEvents( CPrimaryAssetStore& store )
        : CObservingNotifier()
        , ids()
    {
        TEventCallback callback( this, &CPackIndexEvents::OnEvent );
        SubscribeTo( &store, CPrimaryAssetStore::PackIndexUpdatedEvent, callback );
    }

    virtual ~CPackIndexEvents()
    {
        UnsubscribeAllFromObserver();
    }

    UInt32 CountFor( const CString& packId ) const
    {
        UInt32 count = 0;
        for ( size_t i=0; i<ids.size(); ++i )
            if ( ids[ i ] == packId )
                ++count;
        return count;
    }

    private:

    void OnEvent( CNotifier* notifier, const CEvent& eventId, CICloneable* eventData )
    {
        const TCloneableString* packId = static_cast< const TCloneableString* >( eventData );
        if ( GUCEF_NULL != packId )
            ids.push_back( packId->GetData() );
    }
};

/** The suite, against the database of the fixture */
void
RunPackIndexSuite( SIndexFixture& fixture )
{
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CString& base = fixture.base;
    MountFreshDirectory( fixture.label + "_main", base + "/main" );
    MountFreshDirectory( fixture.label + "_source", base + "/source" );

    CTestModelAnalyzer* analyzer = GUCEF_NEW CTestModelAnalyzer();
    TAssetContentAnalyzerPtr analyzerPtr( analyzer );
    CAssetStoreGlobal::Instance()->GetContentAnalyzerRegistry().Register( analyzerPtr );

    // the packs on disk
    const CString packA = base + "/source/Medieval_Weapons";
    const CString packB = base + "/source/Barrels";
    const CString packC = base + "/source/Crates";
    const CString packD = base + "/source/Rocks";
    const CString packE = base + "/source/Lanterns";
    bool written = WriteVfsFile( packA + "/pack.json", CString( "{ \"title\": \"Medieval Weapons\", \"author\": \"Forge Works\", \"tags\": [ \"sword\", \"medieval\" ], "
                                                              "\"licence\": { \"name\": \"CC BY 4.0\", \"restrictions\": [ \"noMachineLearning\" ] } }" ) );
    written = WriteVfsFile( packA + "/Meshes/SM_Wep_Sword_01.fbx", CString( "triangles=1200;vertices=900;rigged=0;animated=0" ) ) && written;
    written = WriteVfsFile( packA + "/Meshes/SM_Wep_Axe_01.fbx", CString( "triangles=60000;vertices=40000;rigged=0" ) ) && written;
    CDynamicBuffer png;
    written = MakePng( 64, 32, false, png ) && WriteVfsFile( packA + "/Textures/T_Sword_D.png", png ) && written;
    written = MakePng( 32, 32, true, png ) && WriteVfsFile( packA + "/Textures/T_Sword_N.png", png ) && written;
    CDynamicBuffer sound;
    MakePattern( 4096, 77, sound );
    written = WriteVfsFile( packA + "/Audio/sword_hit.wav", sound ) && written;
    written = WriteVfsFile( packB + "/SM_Barrel_01.fbx", CString( "triangles=400;vertices=300" ) ) && written;
    written = WriteVfsFile( packB + "/SM_Barrel_02.fbx", CString( "triangles=450;vertices=320" ) ) && written;
    written = WriteVfsFile( packC + "/Meshes/SM_Crate_01.fbx", CString( "triangles=96;vertices=64" ) ) && written;
    written = WriteVfsFile( packC + "/Meshes/SM_Crate_02.fbx", CString( "triangles=120;vertices=80" ) ) && written;
    written = MakePng( 16, 16, false, png ) && WriteVfsFile( packC + "/Textures/T_Crate_D.png", png ) && written;
    written = WriteVfsFile( packD + "/Meshes/SM_Rock_01.fbx", CString( "triangles=300;vertices=200" ) ) && written;
    written = WriteVfsFile( packD + "/Meshes/SM_Rock_02.fbx", CString( "triangles=310;vertices=210" ) ) && written;
    written = WriteVfsFile( packE + "/Meshes/SM_Lantern_01.fbx", CString( "triangles=500;vertices=400" ) ) && written;
    written = WriteVfsFile( packE + "/Meshes/SM_Lantern_02.fbx", CString( "triangles=520;vertices=410" ) ) && written;

    CTestConverterHook hook;
    CPrimaryAssetStore store;
    store.SetSettings( MakeIndexSettings( fixture ) );
    store.SetCredentialStore( fixture.credentials );
    store.SetConverterHook( &hook );
    CString packAId, packBId, packCId, packDId, packEId;
    CString swordId, axeId, textureId;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": migration 3 to 4 on a populated catalogue adds the tables of the pack index and keeps the rows; before it nothing is indexed" ).C_String() )
        try
        {
            ASSERT_TRUE( written );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Open( 3 ) );
            ASSERT_TRUE( 3 == store.GetCatalogue().GetSchemaVersion() );
            CAssetPackIndexLoader loader( store.GetCatalogue(), GUCEF_NULL );
            ASSERT_FALSE( loader.IsAvailable() );
            CAssetCatalogue::TRowVector probe;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK != store.GetCatalogue().SelectAll( "pack_resources", probe ) );

            CAssetIngestRequest request;
            request.sourceId = "disk";
            request.contentPath = packB;
            request.keep = "original";
            CAssetIngestResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Ingest( request, result ) && 2 == result.assetCount );
            packBId = result.packId;
            CAssetPackIndexOutcome outcome;
            ASSERT_TRUE( ASSETSTORE_STATUS_UNSUPPORTED == store.IndexPack( packBId, CAssetPackIndexOptions(), outcome ) );
            CAssetSearchQuery groupQuery;
            groupQuery.groups.insert( "static-models" );
            CAssetSearchResultPage page;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( groupQuery, page ) && 0 == page.hits.size() );

            // the next open runs migration 4 (and the later ones): the tables are there, empty, and the rows of the pack are as they were
            store.Close();
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Open( 4 ) );
            ASSERT_TRUE( 4 == store.GetCatalogue().GetSchemaVersion() && 4 == CountAllRows( store, "schema_version" ) );
            ASSERT_TRUE( 0 == CountAllRows( store, "pack_indexes" ) && 0 == CountAllRows( store, "pack_groups" ) && 0 == CountAllRows( store, "pack_resources" ) );
            CAssetPackRecord pack;
            TAssetRecordVector assets;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( packBId, pack ) && ASSETSTORE_STATUS_OK == store.GetAssets( packBId, assets ) && 2 == assets.size() );
            CAssetPackIndexInfo info;
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == store.GetPackIndexInfo( packBId, false, info ) );
            CAssetPackIndexLoader loader4( store.GetCatalogue(), GUCEF_NULL );
            ASSERT_TRUE( loader4.IsAvailable() );
            CString::StringVector missing;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == loader4.GetPacksWithoutIndex( missing ) && 1 == missing.size() && packBId == missing[ 0 ] );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": the loader writes the groups, the resources with the ids of the store, the references and the names of what is no asset from a document; unchanged documents load once" ).C_String() )
        try
        {
            CAssetIngestRequest request;
            request.sourceId = "disk";
            request.contentPath = packA;
            request.keep = "original";
            CAssetIngestResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Ingest( request, result ) );
            packAId = result.packId;
            CAssetPackRecord pack;
            TAssetRecordVector assets;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( packAId, pack ) && ASSETSTORE_STATUS_OK == store.GetAssets( packAId, assets ) );
            const CAssetRecord* sword = FindAssetByName( assets, "SM_Wep_Sword_01" );
            const CAssetRecord* axe = FindAssetByName( assets, "SM_Wep_Axe_01" );
            const CAssetRecord* texture = FindAssetByName( assets, "T_Sword_D" );
            ASSERT_TRUE( GUCEF_NULL != sword && GUCEF_NULL != axe && GUCEF_NULL != texture );
            swordId = sword->assetId;
            axeId = axe->assetId;
            textureId = texture->assetId;
            // the id formula of the store is the formula of the loader
            ASSERT_TRUE( swordId == CAssetPackIndexLoader::MakeResourceId( packAId, "Meshes/SM_Wep_Sword_01.fbx" ) );
            ASSERT_TRUE( 5 == assets.size() );

            CTestPackIndexDocument document;
            document.packName = "documentbase";
            document.fileCount = (UInt64) pack.fileCount;
            document.totalBytes = (UInt64) pack.sizeBytes;
            document.restrictions.push_back( "noMachineLearning" );
            document.resources.push_back( CTestPackIndexResource( "Meshes/SM_Wep_Sword_01.fbx", "model", "meshes", 100 ).Stat( "triangles", 5 ).Stat( "vertices", 5 ).Ref( "usesTexture", "Textures/T_Sword_D.png" ) );
            document.resources.back().state = "analysed";
            document.resources.back().hasThumbnail = true;
            document.resources.push_back( CTestPackIndexResource( "Meshes/SM_Wep_Axe_01.fbx", "model", "meshes", 200 ) );
            document.resources.back().state = "converted";
            document.resources.push_back( CTestPackIndexResource( "Textures/T_Sword_D.png", "texture", "textures", 300 ).Stat( "width", 64 ).Stat( "height", 32 ) );
            document.resources.push_back( CTestPackIndexResource( "Textures/T_Sword_N.png", "texture", "textures", 400 ) );
            document.resources.push_back( CTestPackIndexResource( "Audio/sword_hit.wav", "audio", "audio", 500 ).Stat( "channels", 1 ).Stat( "sampleRate", 44100 ) );
            document.resources.push_back( CTestPackIndexResource( "Meta/Notes.txt", "document", "meta", 10 ) );
            document.resources.push_back( CTestPackIndexResource( "Meta/Credits.txt", "document", "meta", 20 ) );
            const CString folder = base + "/main/scratch/a";
            const CString manifestPath = document.WriteTo( folder );
            ASSERT_FALSE( manifestPath.IsNULLOrEmpty() );

            CAssetPackIndexLoader loader( store.GetCatalogue(), &store.GetVocabulary().GetKeywordNormalizer() );
            CAssetPackIndexOutcome outcome;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == loader.Load( packAId, manifestPath, false, outcome ) );
            ASSERT_TRUE( outcome.loaded && 7 == outcome.resourceCount && !outcome.indexKey.IsNULLOrEmpty() && !outcome.documentHash.IsNULLOrEmpty() );

            CAssetPackIndexInfo info;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexInfo( packAId, true, info ) );
            ASSERT_TRUE( CAssetPackIndexInfo::StateLoaded == info.state && !info.stale && 7 == info.resourceCount && info.indexKey == outcome.indexKey && info.documentPath == manifestPath );
            ASSERT_TRUE( 2 == info.levels.size() && 0 == info.shardCount && info.hasManifest && info.HasLicenceRestriction( "noMachineLearning" ) && !info.HasLicenceRestriction( "attributionRequired" ) );
            ASSERT_TRUE( "TestPackIndexDocument/1" == info.producer );
            ASSERT_TRUE( 8 == info.groups.size() && 7 == CountRowsOfPack( store, "pack_resources", packAId ) );

            // the groups: the fixed order, the sub-groups below their function group
            ASSERT_TRUE( "static-models" == info.groups[ 0 ].groupId && info.groups[ 0 ].subGroup.IsNULLOrEmpty() && 2 == info.groups[ 0 ].resourceCount && 300 == info.groups[ 0 ].sizeBytes );
            ASSERT_TRUE( "static-models" == info.groups[ 1 ].groupId && "meshes" == info.groups[ 1 ].subGroup && 2 == info.groups[ 1 ].resourceCount );
            ASSERT_TRUE( "textures" == info.groups[ 2 ].groupId && "audio" == info.groups[ 4 ].groupId && "other" == info.groups[ 6 ].groupId && 2 == info.groups[ 6 ].resourceCount );

            // the resources: the ids of the store, an asset id when the resource is an asset
            TAssetPackResourceVector resources;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexResources( packAId, CString::Empty, CString::Empty, resources ) && 7 == resources.size() );
            ASSERT_TRUE( "static-models" == resources[ 0 ].groupId && "Meshes/SM_Wep_Axe_01.fbx" == resources[ 0 ].path && axeId == resources[ 0 ].assetId && axeId == resources[ 0 ].resourceId );
            ASSERT_TRUE( swordId == resources[ 1 ].assetId && resources[ 1 ].analysed && !resources[ 1 ].converted && resources[ 1 ].hasThumbnail && "analysed" == resources[ 1 ].status );
            ASSERT_TRUE( resources[ 0 ].converted && "converted" == resources[ 0 ].status );
            ASSERT_TRUE( "other" == resources[ 5 ].groupId && "Meta/Credits.txt" == resources[ 5 ].path && resources[ 5 ].assetId.IsNULLOrEmpty() && "txt" == resources[ 5 ].format );
            CAssetPackResource one;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexResource( swordId, one ) && "Meshes/SM_Wep_Sword_01.fbx" == one.path && "model" == one.kind );
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == store.GetPackIndexResource( "asnothing", one ) );
            TAssetPackResourceVector meshes;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexResources( packAId, "static-models", "meshes", meshes ) && 2 == meshes.size() );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexResources( packAId, "textures", "nothing", meshes ) && meshes.empty() );

            // the references of the document are relations of the store, between the ids of the store
            CDataTableQuery relationQuery;
            relationQuery.WhereEquals( "pack_id", CVariant( packAId ) ).WhereEquals( "kind", CVariant( CString( "usesTexture" ) ) );
            CAssetCatalogue::TRowVector relations;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetCatalogue().Select( "relations", relationQuery, relations ) && 1 == relations.size() );
            ASSERT_TRUE( swordId == CAssetCatalogue::GetText( relations[ 0 ], "from_asset" ) && textureId == CAssetCatalogue::GetText( relations[ 0 ], "to_asset" ) );

            // what is no asset is searchable by name through the keywords of the store; an asset keeps its own
            CDataTableQuery keywordQuery;
            keywordQuery.WhereEquals( "pack_id", CVariant( packAId ) ).WhereEquals( "object_type", CVariant( CString( "resource" ) ) );
            CAssetCatalogue::TRowVector keywords;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetCatalogue().Select( "keywords", keywordQuery, keywords ) && !keywords.empty() );
            bool hasNote = false;
            for ( size_t k=0; k<keywords.size(); ++k )
                hasNote = hasNote || ( "note" == CAssetCatalogue::GetText( keywords[ k ], "keyword" ) && CAssetCatalogue::GetText( keywords[ k ], "object_id" ) == CAssetPackIndexLoader::MakeResourceId( packAId, "Meta/Notes.txt" ) );
            ASSERT_TRUE( hasNote );

            // the statistics of the assets stay as the analysis made them: the document fills only what is missing
            CAssetRecord swordRecord;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetAssets( packAId, assets ) );
            sword = FindAssetByName( assets, "SM_Wep_Sword_01" );
            ASSERT_TRUE( GUCEF_NULL != sword && sword->model.isSet && 1200 == sword->model.triangles );

            // the same document again: the catalogue holds it, nothing is written; forced, it is written again
            CAssetPackIndexOutcome again;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == loader.Load( packAId, manifestPath, false, again ) && !again.loaded );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == loader.Load( packAId, manifestPath, true, again ) && again.loaded );
            ASSERT_TRUE( 7 == CountRowsOfPack( store, "pack_resources", packAId ) && 1 == CountAllRows( store, "pack_indexes" ) && 8 == CountRowsOfPack( store, "pack_groups", packAId ) );

            // a changed document replaces the rows: the resource that went is gone, the new one is there
            document.resources.pop_back();
            document.resources.push_back( CTestPackIndexResource( "Effects/Spark.png", "texture", "effects", 5 ) );
            document.indexKey = "second-index-key";
            const CString secondPath = document.WriteTo( folder );
            CAssetPackIndexOutcome second;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == loader.Load( packAId, secondPath, false, second ) && second.loaded && "second-index-key" == second.indexKey );
            ASSERT_TRUE( 7 == CountRowsOfPack( store, "pack_resources", packAId ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == store.GetPackIndexResource( CAssetPackIndexLoader::MakeResourceId( packAId, "Meta/Credits.txt" ), one ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexResource( CAssetPackIndexLoader::MakeResourceId( packAId, "Effects/Spark.png" ), one ) && "effects" == one.subGroup );
            ASSERT_TRUE( 9 == CountRowsOfPack( store, "pack_groups", packAId ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": references with the paths of their targets are relations by the ids of the store, also to a file the document does not list; the companion files of an asset follow them transitively, once each, without inScene" ).C_String() )
        try
        {
            ASSERT_FALSE( packAId.IsNULLOrEmpty() || swordId.IsNULLOrEmpty() );
            CAssetPackRecord pack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( packAId, pack ) );
            const CString normalId = CAssetPackIndexLoader::MakeResourceId( packAId, "Textures/T_Sword_N.png" );
            const CString audioId = CAssetPackIndexLoader::MakeResourceId( packAId, "Audio/sword_hit.wav" );

            CTestPackIndexDocument document;
            document.packName = "relationsbase";
            document.indexKey = "relations-index-key";
            document.fileCount = (UInt64) pack.fileCount;
            document.totalBytes = (UInt64) pack.sizeBytes;
            // the sword: its texture, the sound (no resource of the document, a wrong id: the path decides), the axe it is in (not followed)
            document.resources.push_back( CTestPackIndexResource( "Meshes/SM_Wep_Sword_01.fbx", "model", "meshes", 100 )
                                          .RefPath( "usesTexture", "Textures/T_Sword_D.png" ).RefPath( "usesFile", "Audio/sword_hit.wav", true )
                                          .RefPath( "inScene", "Meshes/SM_Wep_Axe_01.fbx" ) );
            // the axe: a reference of an earlier writer, by the document id only
            document.resources.push_back( CTestPackIndexResource( "Meshes/SM_Wep_Axe_01.fbx", "model", "meshes", 200 ).Ref( "usesTexture", "Textures/T_Sword_D.png" ) );
            // the texture leads to the normal map, which leads back to the sword: a cycle
            document.resources.push_back( CTestPackIndexResource( "Textures/T_Sword_D.png", "texture", "textures", 300 ).RefPath( "usesTexture", "Textures/T_Sword_N.png" ) );
            document.resources.push_back( CTestPackIndexResource( "Textures/T_Sword_N.png", "texture", "textures", 400 ).RefPath( "dependsOn", "Meshes/SM_Wep_Sword_01.fbx" ) );
            const CString manifestPath = document.WriteTo( base + "/main/scratch/relations" );
            ASSERT_FALSE( manifestPath.IsNULLOrEmpty() );
            CAssetPackIndexLoader loader( store.GetCatalogue(), &store.GetVocabulary().GetKeywordNormalizer() );
            CAssetPackIndexOutcome outcome;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == loader.Load( packAId, manifestPath, true, outcome ) && outcome.loaded );

            ASSERT_TRUE( 6 == CountRowsOfPack( store, "relations", packAId ) );
            CDataTableQuery fileQuery;
            fileQuery.WhereEquals( "from_asset", CVariant( swordId ) ).WhereEquals( "kind", CVariant( CString( "usesFile" ) ) );
            CAssetCatalogue::TRowVector rows;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetCatalogue().Select( "relations", fileQuery, rows ) && 1 == rows.size() && audioId == CAssetCatalogue::GetText( rows[ 0 ], "to_asset" ) );
            CDataTableQuery axeQuery;
            axeQuery.WhereEquals( "from_asset", CVariant( axeId ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetCatalogue().Select( "relations", axeQuery, rows ) && 1 == rows.size() && textureId == CAssetCatalogue::GetText( rows[ 0 ], "to_asset" ) );

            // the companion files of the sword: breadth first in the order of the kinds, the cycle ends, the axe is not one of them
            TAssetFileRecordVector companions;
            UInt32 followed = 0;
            bool truncated = true;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetCompanionFiles( swordId, companions, followed, truncated ) );
            ASSERT_TRUE( 3 == companions.size() && 4 == followed && !truncated );
            ASSERT_TRUE( companions[ 0 ].path.EndsWith( "Audio/sword_hit.wav" ) && companions[ 1 ].path.EndsWith( "Textures/T_Sword_D.png" ) && companions[ 2 ].path.EndsWith( "Textures/T_Sword_N.png" ) );
            for ( size_t f=0; f<companions.size(); ++f )
                ASSERT_TRUE( "referenced" == companions[ f ].role && !companions[ f ].digest.IsNULLOrEmpty() && vfs.FileExists( companions[ f ].path ) );

            // the axe reaches the sword through the textures; the sound has no relations of its own
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetCompanionFiles( axeId, companions, followed, truncated ) && 4 == companions.size() && companions[ 2 ].path.EndsWith( "SM_Wep_Sword_01.fbx" ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetCompanionFiles( audioId, companions, followed, truncated ) && companions.empty() && 0 == followed && !truncated );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetCompanionFiles( normalId, companions, followed, truncated ) && 3 == companions.size() );
            ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == store.GetCompanionFiles( CString::Empty, companions, followed, truncated ) );
            ASSERT_TRUE( CPrimaryAssetStore::IsCompanionRelationKind( "usesFile" ) && !CPrimaryAssetStore::IsCompanionRelationKind( "inScene" ) && !CPrimaryAssetStore::IsCompanionRelationKind( "fitsSkeleton" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": documents that cannot be loaded are refused with the reason and leave the rows as they were; a pack that changed makes the index stale; large packs load from shards" ).C_String() )
        try
        {
            CAssetPackRecord pack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( packAId, pack ) );
            CAssetPackIndexLoader loader( store.GetCatalogue(), &store.GetVocabulary().GetKeywordNormalizer() );
            const CString before = SnapshotOf( store, packAId );
            CAssetPackIndexOutcome outcome;
            const CString folder = base + "/main/scratch/bad";

            CTestPackIndexDocument bad;
            bad.format = "another.format";
            bad.resources.push_back( CTestPackIndexResource( "a.fbx", "model", "", 1 ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == loader.Load( packAId, bad.WriteTo( folder ), true, outcome ) && !outcome.message.IsNULLOrEmpty() );
            bad.format = "gucef.packIndex";
            bad.version = 2;
            ASSERT_TRUE( ASSETSTORE_STATUS_UNSUPPORTED == loader.Load( packAId, bad.WriteTo( folder ), true, outcome ) && outcome.message.HasSubstr( "version" ) >= 0 );
            bad.version = 1;
            bad.packKey.Clear();
            ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == loader.Load( packAId, bad.WriteTo( folder ), true, outcome ) );
            bad.packKey = "key";
            bad.state = "failed";
            ASSERT_TRUE( ASSETSTORE_STATUS_FAILED == loader.Load( packAId, bad.WriteTo( folder ), true, outcome ) );
            ASSERT_TRUE( WriteVfsFile( folder + "/pack-index.json", CString( "this is not json" ) ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == loader.Load( packAId, folder + "/pack-index.json", true, outcome ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == loader.Load( packAId, folder + "/missing/pack-index.json", true, outcome ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == loader.Load( "nopack", base + "/main/scratch/a/pack-index.json", true, outcome ) );
            ASSERT_TRUE( before == SnapshotOf( store, packAId ) );

            // a shard that does not match its hash
            CTestPackIndexDocument large;
            large.shardSize = 10;
            for ( UInt32 i=0; i<25; ++i )
                large.resources.push_back( CTestPackIndexResource( "Large/Part" + ToString( i ) + ".fbx", "model", "large", 10 + i ) );
            const CString largeFolder = base + "/main/scratch/large";
            const CString largePath = large.WriteTo( largeFolder );
            ASSERT_FALSE( largePath.IsNULLOrEmpty() );
            ASSERT_TRUE( WriteVfsFile( largeFolder + "/pack-index.d/shard-2.json", CString( "{ \"resources\": [] }" ) ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_DIGEST_MISMATCH == loader.Load( packAId, largePath, true, outcome ) );
            ASSERT_TRUE( before == SnapshotOf( store, packAId ) );
            ASSERT_FALSE( large.WriteTo( largeFolder ).IsNULLOrEmpty() );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == loader.Load( packAId, largePath, false, outcome ) && outcome.loaded && 25 == outcome.resourceCount );
            CAssetPackIndexInfo info;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexInfo( packAId, true, info ) && 3 == info.shardCount && 25 == info.resourceCount && 25 == CountRowsOfPack( store, "pack_resources", packAId ) );
            TAssetPackResourceVector resources;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexResources( packAId, "static-models", "large", resources ) && 25 == resources.size() && "Large/Part0.fbx" == resources[ 0 ].path );

            // a document of a pack that is not the pack: loaded, and marked
            CTestPackIndexDocument other;
            other.fileCount = (UInt64) pack.fileCount - 1;
            other.totalBytes = (UInt64) pack.sizeBytes;
            other.resources.push_back( CTestPackIndexResource( "Meshes/SM_Wep_Sword_01.fbx", "model", "meshes", 1 ) );
            other.indexKey = "stale-one";
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == loader.Load( packAId, other.WriteTo( base + "/main/scratch/stale" ), false, outcome ) && outcome.loaded );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexInfo( packAId, false, info ) && info.stale && CAssetPackIndexInfo::StateStale == info.state && !info.stateReason.IsNULLOrEmpty() );
            ASSERT_TRUE( 1 == CountRowsOfPack( store, "pack_resources", packAId ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == loader.MarkStale( packAId, "by the test" ) && ASSETSTORE_STATUS_NOT_FOUND == loader.MarkStale( "nopack", "x" ) );

            // the good document again: loaded, no longer stale
            CTestPackIndexDocument good;
            good.fileCount = (UInt64) pack.fileCount;
            good.totalBytes = (UInt64) pack.sizeBytes;
            good.indexKey = "good-one";
            good.restrictions.push_back( "noMachineLearning" );
            good.resources.push_back( CTestPackIndexResource( "Meshes/SM_Wep_Sword_01.fbx", "model", "meshes", 100 ).Stat( "triangles", 5 ) );
            good.resources.push_back( CTestPackIndexResource( "Meshes/SM_Wep_Axe_01.fbx", "model", "meshes", 200 ) );
            good.resources.push_back( CTestPackIndexResource( "Textures/T_Sword_D.png", "texture", "textures", 300 ) );
            good.resources.push_back( CTestPackIndexResource( "Textures/T_Sword_N.png", "texture", "textures", 400 ) );
            good.resources.push_back( CTestPackIndexResource( "Audio/sword_hit.wav", "audio", "audio", 500 ) );
            good.resources.push_back( CTestPackIndexResource( "Meta/Notes.txt", "document", "meta", 10 ) );
            // the document of the pack in its place in the storage: what a rebuild from the storage finds
            const CString storedPath = good.WriteTo( base + "/main/intermediate/Medieval_Weapons" );
            ASSERT_FALSE( storedPath.IsNULLOrEmpty() );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == loader.Load( packAId, storedPath, false, outcome ) && outcome.loaded );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexInfo( packAId, false, info ) && !info.stale && CAssetPackIndexInfo::StateLoaded == info.state && 6 == info.resourceCount );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": search by group and sub-group, alone and with text, packs and the other filters; a pack that was never indexed has no groups" ).C_String() )
        try
        {
            CAssetSearchResultPage page;
            CAssetSearchQuery query;
            query.pageSize = 100;
            query.groups.insert( "static-models" );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 2 == page.hits.size() );
            query.groups.clear();
            query.groups.insert( "textures" );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 2 == page.hits.size() );
            query.groups.insert( "audio" );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 3 == page.hits.size() );
            query.groups.clear();
            query.subGroups.insert( "meshes" );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 2 == page.hits.size() );
            query.subGroups.clear();

            // with text: the keywords and the group agree
            query.groups.insert( "textures" );
            query.text = "sword";
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 2 == page.hits.size() );
            query.text = "axe";
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 0 == page.hits.size() );
            query.groups.clear();
            query.groups.insert( "static-models" );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 1 == page.hits.size() && "SM_Wep_Axe_01" == page.hits[ 0 ].name );
            query.text.Clear();

            // with the other filters and the scope of a pack
            query.groups.clear();
            query.groups.insert( "static-models" );
            query.minTriangles = 2000;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 1 == page.hits.size() && "SM_Wep_Axe_01" == page.hits[ 0 ].name );
            query.minTriangles = -1;
            query.packIds.insert( packBId );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 0 == page.hits.size() );
            query.packIds.clear();
            query.packIds.insert( packAId );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 2 == page.hits.size() );
            query.groups.clear();
            query.groups.insert( "no-such-group" );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 0 == page.hits.size() );

            // the filters are part of the query that is saved and sent
            CAssetSearchQuery saved;
            saved.groups.insert( "audio" );
            saved.subGroups.insert( "sfx" );
            CDataNode node;
            CDataNodeSerializableSettings settings;
            ASSERT_TRUE( saved.Serialize( node, settings ) );
            CAssetSearchQuery back;
            ASSERT_TRUE( back.Deserialize( node, settings ) && 1 == back.groups.count( "audio" ) && 1 == back.subGroups.count( "sfx" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": an interrupted load of an index is made again from its document when the store opens" ).C_String() )
        try
        {
            CAssetPackIndexInfo info;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexInfo( packAId, false, info ) && CAssetPackIndexInfo::StateLoaded == info.state );
            const CString documentPath = info.documentPath;
            CDataTableQuery query;
            query.WhereEquals( "pack_id", CVariant( packAId ) );
            CAssetCatalogue::TRowVector rows;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetCatalogue().Select( "pack_indexes", query, rows ) && 1 == rows.size() );
            rows[ 0 ][ "state" ] = CVariant( CAssetPackIndexInfo::StateLoading );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetCatalogue().Write( "pack_indexes", rows ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetCatalogue().DeleteWhereEquals( "pack_resources", "pack_id", CVariant( packAId ) ) );
            store.Close();
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Open() );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexInfo( packAId, false, info ) && CAssetPackIndexInfo::StateLoaded == info.state && 6 == info.resourceCount );
            ASSERT_TRUE( 6 == CountRowsOfPack( store, "pack_resources", packAId ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    // the converter hook with the pack index
    hook.SetIndexMode( CTestConverterHook::INDEX_BUILD );
    CPackIndexEvents events( store );

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": work once: the ingest loads the document the conversion left and builds none, IndexPack builds one for a pack without, and a second call, a forced load and a redo do what each says" ).C_String() )
        try
        {
            // keep: original, no conversion runs, so no document: the ingest builds none, the job does (IndexPack builds once)
            CAssetIngestRequest request;
            request.sourceId = "disk";
            request.contentPath = packC;
            request.keep = "original";
            CAssetIngestResult result;
            const UInt32 callsBefore = hook.GetIndexCallCount();
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Ingest( request, result ) );
            packCId = result.packId;
            ASSERT_TRUE( callsBefore == hook.GetIndexCallCount() && 0 == hook.GetIndexBuildCount() );
            CAssetPackIndexInfo info;
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == store.GetPackIndexInfo( packCId, false, info ) );
            CAssetPackIndexOutcome built;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.IndexPack( packCId, CAssetPackIndexOptions(), built ) && built.built && built.loaded && !built.reused );
            ASSERT_TRUE( callsBefore + 1 == hook.GetIndexCallCount() && 1 == hook.GetIndexBuildCount() );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexInfo( packCId, true, info ) );
            ASSERT_TRUE( CAssetPackIndexInfo::StateLoaded == info.state && 3 == info.resourceCount && info.hasManifest && !info.stale );
            ASSERT_TRUE( 4 == info.groups.size() );
            PumpFor( 20 );
            ASSERT_TRUE( 1 == events.CountFor( packCId ) );     // the build of IndexPack() raised it; the ingest leaves the event to the service

            // a second call: the document is there, loaded and fresh: nothing is built, nothing called, nothing raised
            CAssetPackIndexOutcome outcome;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.IndexPack( packCId, CAssetPackIndexOptions(), outcome ) );
            ASSERT_TRUE( outcome.reused && !outcome.loaded && !outcome.built );
            ASSERT_TRUE( callsBefore + 1 == hook.GetIndexCallCount() && 1 == hook.GetIndexBuildCount() );
            PumpFor( 20 );
            ASSERT_TRUE( 1 == events.CountFor( packCId ) );

            // forced: the rows are written again from the document, the hook is not asked
            CAssetPackIndexOptions force;
            force.force = true;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.IndexPack( packCId, force, outcome ) && outcome.reused && outcome.loaded );
            ASSERT_TRUE( callsBefore + 1 == hook.GetIndexCallCount() );
            PumpFor( 20 );
            ASSERT_TRUE( 2 == events.CountFor( packCId ) );

            // the redo of the index asks the hook to build again
            CAssetPackIndexOptions redo;
            redo.reindex = true;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.IndexPack( packCId, redo, outcome ) && outcome.built && !outcome.reused );
            ASSERT_TRUE( callsBefore + 2 == hook.GetIndexCallCount() && 2 == hook.GetIndexBuildCount() );

            // keep: both, the conversion leaves the document in the intermediate folder: ingest loads it, the hook builds nothing
            request.contentPath = packD;
            request.keep = "both";
            const UInt32 builds = hook.GetIndexBuildCount();
            const UInt32 calls = hook.GetIndexCallCount();
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Ingest( request, result ) );
            packDId = result.packId;
            ASSERT_TRUE( builds == hook.GetIndexBuildCount() && calls == hook.GetIndexCallCount() );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexInfo( packDId, false, info ) && CAssetPackIndexInfo::StateLoaded == info.state && 2 == info.resourceCount );
            CAssetPackRecord packD;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( packDId, packD ) );
            ASSERT_TRUE( vfs.FileExists( base + "/main/intermediate/Rocks/pack-index.json" ) && info.documentPath == base + "/main/intermediate/Rocks/pack-index.json" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a store without a converter that builds indexes leaves a pack without one; the index is derived from the catalogue for packs that have none, without reading a file of the pack" ).C_String() )
        try
        {
            hook.SetIndexMode( CTestConverterHook::INDEX_UNSUPPORTED );
            CAssetIngestRequest request;
            request.sourceId = "disk";
            request.contentPath = packE;
            request.keep = "original";
            CAssetIngestResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Ingest( request, result ) );
            packEId = result.packId;
            CAssetPackIndexInfo info;
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == store.GetPackIndexInfo( packEId, false, info ) );
            CAssetPackIndexOutcome outcome;
            ASSERT_TRUE( ASSETSTORE_STATUS_UNSUPPORTED == store.IndexPack( packEId, CAssetPackIndexOptions(), outcome ) && !outcome.message.IsNULLOrEmpty() );
            ASSERT_TRUE( 0 == CountRowsOfPack( store, "pack_resources", packEId ) );

            // the packs of the catalogue that have no index: B (ingested before the tables) and E; derived, no file is read
            hook.SetIndexMode( CTestConverterHook::INDEX_BUILD );
            const UInt32 callsBefore = hook.GetIndexCallCount();
            CAssetPackIndexOptions derive;
            derive.derive = true;
            CAssetPackIndexReport report;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.IndexPacksWithoutIndex( derive, report ) );
            ASSERT_TRUE( 2 == report.packsSeen && 2 == report.packsDerived && 0 == report.packsFailed );
            ASSERT_TRUE( callsBefore + 2 == hook.GetIndexCallCount() );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexInfo( packEId, true, info ) && 2 == info.resourceCount && info.hasManifest );
            const CDataNode* producer = info.manifest.FindChild( "producer" );
            ASSERT_TRUE( GUCEF_NULL != producer && producer->GetAttributeValueOrChildValueByName( "derivedFromCatalogue" ).AsBool( false, true ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexInfo( packBId, false, info ) && 2 == info.resourceCount );
            CAssetPackIndexReport none;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.IndexPacksWithoutIndex( derive, none ) && 0 == none.packsSeen );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": the rows are rebuilt from the documents on the storage; a document that no pack owns is reported and not loaded; a removed pack takes its rows" ).C_String() )
        try
        {
            CString::StringVector packIds;
            packIds.push_back( packAId );
            packIds.push_back( packBId );
            packIds.push_back( packCId );
            packIds.push_back( packDId );
            packIds.push_back( packEId );
            CString::StringVector snapshots;
            for ( size_t p=0; p<packIds.size(); ++p )
                snapshots.push_back( SnapshotOf( store, packIds[ p ] ) );

            // the database is emptied of the derived rows: the documents are what is left
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetCatalogue().DeleteWhereEquals( "pack_resources", "pack_id", CVariant( packAId ) ) );
            CAssetPackIndexLoader loader( store.GetCatalogue(), GUCEF_NULL );
            for ( size_t p=0; p<packIds.size(); ++p )
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == loader.Remove( packIds[ p ] ) );
            ASSERT_TRUE( 0 == CountAllRows( store, "pack_indexes" ) && 0 == CountAllRows( store, "pack_resources" ) && 0 == CountAllRows( store, "pack_groups" ) );

            // a document in the storage that no pack owns
            CTestPackIndexDocument stray;
            stray.resources.push_back( CTestPackIndexResource( "x.fbx", "model", "", 1 ) );
            ASSERT_FALSE( stray.WriteTo( base + "/main/intermediate/Stray_Folder" ).IsNULLOrEmpty() );

            const UInt32 callsBefore = hook.GetIndexCallCount();
            CAssetPackIndexReport report;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.ReindexFromStorage( false, CAssetPackIndexOptions(), report ) );
            GUCEF_LOG( LOGLEVEL_NORMAL, "PackIndexTest: reindex report: " << report.ToSummaryText() << " " << ( report.problems.empty() ? CString::Empty : report.problems[ 0 ] ) );
            ASSERT_TRUE( 5 == report.packsSeen && 5 == report.packsLoaded && 0 == report.packsFailed && 0 == report.packsBuilt && 1 == report.documentsWithoutPack );
            ASSERT_TRUE( callsBefore == hook.GetIndexCallCount() );
            for ( size_t p=0; p<packIds.size(); ++p )
                ASSERT_TRUE( snapshots[ p ] == SnapshotOf( store, packIds[ p ] ) );
            ASSERT_TRUE( 5 == CountAllRows( store, "pack_indexes" ) );
            CAssetPackRecord strayPack;
            ASSERT_TRUE( 5 == CountAllRows( store, "packs" ) );

            // again: nothing changes, nothing is written
            CAssetPackIndexReport second;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.ReindexFromStorage( false, CAssetPackIndexOptions(), second ) && 5 == second.packsUnchanged && 0 == second.packsLoaded );

            // a pack without a document: skipped, or built when asked
            ASSERT_TRUE( vfs.DeleteFile( base + "/main/intermediate/Lanterns/pack-index.json", true ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == loader.Remove( packEId ) );
            CAssetPackIndexReport skipped;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.ReindexFromStorage( false, CAssetPackIndexOptions(), skipped ) && 1 == skipped.packsSkipped && 4 == skipped.packsUnchanged );
            CAssetPackIndexReport built;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.ReindexFromStorage( true, CAssetPackIndexOptions(), built ) && 1 == built.packsBuilt );
            CAssetPackIndexInfo info;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexInfo( packEId, false, info ) && 2 == info.resourceCount );

            // a removed pack takes its rows with it
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.RemovePack( packCId, true ) );
            ASSERT_TRUE( 0 == CountRowsOfPack( store, "pack_indexes", packCId ) && 0 == CountRowsOfPack( store, "pack_resources", packCId ) && 0 == CountRowsOfPack( store, "pack_groups", packCId ) );
            ASSERT_TRUE( 4 == CountAllRows( store, "pack_indexes" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a pack of the earlier layout keeps its index in the folder of the profile of its conversion; the index of a pack is found where the pack is now" ).C_String() )
        try
        {
            // the pack takes the earlier layout: its folder in the location, the document in "converted/<profile>"
            CAssetPackIndexInfo info;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexInfo( packEId, false, info ) );
            CDynamicBuffer document;
            ASSERT_TRUE( ReadVfsFile( info.documentPath, document ) );
            const CString legacyFolder = base + "/main/packs/Legacy/Lanterns/converted/test";
            ASSERT_TRUE( WriteVfsFile( legacyFolder + "/pack-index.json", document ) );
            CDataTableQuery query;
            query.WhereEquals( "pack_id", CVariant( packEId ) );
            CAssetCatalogue::TRowVector rows;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetCatalogue().Select( "packs", query, rows ) && 1 == rows.size() );
            rows[ 0 ][ "root_path" ] = CVariant( CString( "packs/Legacy/Lanterns" ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetCatalogue().Write( "packs", rows ) );
            CAssetPackIndexLoader loader( store.GetCatalogue(), GUCEF_NULL );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == loader.Remove( packEId ) );
            CAssetPackIndexReport report;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.ReindexFromStorage( false, CAssetPackIndexOptions(), report ) );
            ASSERT_TRUE( 4 == report.packsSeen && 4 == report.packsUnchanged + report.packsLoaded + report.packsSkipped );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexInfo( packEId, true, info ) );
            ASSERT_TRUE( info.documentPath == legacyFolder + "/pack-index.json" && 2 == info.resourceCount && info.hasManifest );
            ASSERT_TRUE( 2 == CountRowsOfPack( store, "pack_resources", packEId ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": with indexAfterIngest the ingest has the converter build the pack index when the conversion wrote none; without a converter that can, the pack stays ingested without one" ).C_String() )
        try
        {
            const CString packF = base + "/source/Anvils";
            ASSERT_TRUE( WriteVfsFile( packF + "/Meshes/SM_Anvil_01.fbx", CString( "triangles=250;vertices=180" ) ) );
            ASSERT_TRUE( WriteVfsFile( packF + "/Meshes/SM_Anvil_02.fbx", CString( "triangles=260;vertices=190" ) ) );
            CPrimaryAssetStoreSettings settings = store.GetSettings();
            ASSERT_FALSE( settings.indexAfterIngest );
            settings.indexAfterIngest = true;
            store.SetSettings( settings );

            // keep original: no conversion, no document; the ingest asks the hook for one
            hook.SetIndexMode( CTestConverterHook::INDEX_BUILD );
            const UInt32 builds = hook.GetIndexBuildCount();
            CAssetIngestRequest request;
            request.sourceId = "disk";
            request.contentPath = packF;
            request.keep = "original";
            CAssetIngestResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Ingest( request, result ) );
            CAssetPackIndexInfo info;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexInfo( result.packId, false, info ) && CAssetPackIndexInfo::StateLoaded == info.state && 2 == info.resourceCount );
            ASSERT_TRUE( builds + 1 == hook.GetIndexBuildCount() );

            // a converter that builds none: the pack is ingested, with no index and no problem of the ingest
            hook.SetIndexMode( CTestConverterHook::INDEX_UNSUPPORTED );
            const CString packG = base + "/source/Hammers";
            ASSERT_TRUE( WriteVfsFile( packG + "/Meshes/SM_Hammer_01.fbx", CString( "triangles=180;vertices=120" ) ) );
            request.contentPath = packG;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Ingest( request, result ) && result.problems.empty() );
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == store.GetPackIndexInfo( result.packId, false, info ) );

            settings.indexAfterIngest = false;
            store.SetSettings( settings );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    store.Close();
    UnmountDirectory( fixture.label + "_source" );
    UnmountDirectory( fixture.label + "_main" );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformPackIndexVfsTablesTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "PackIndexVfsTables" );

    SIndexFixture fixture;
    fixture.label = "pi_vfstables";
    fixture.base = "astest/pi_vfstables";
    MountFreshDirectory( "pi_vfstables_catalogue", "astest/pi_vfstables/catalogue" );
    fixture.database.backendName = "vfstables";
    fixture.database.database = "astest/pi_vfstables/catalogue/db";
    fixture.database.backendOptions.SetAllowMultipleValues( false );
    fixture.database.backendOptions.Set( "syncWrites", CVariant( "false" ) );
    RunPackIndexSuite( fixture );
    UnmountDirectory( "pi_vfstables_catalogue" );
}

/*-------------------------------------------------------------------------*/

void
PerformPackIndexPostgreSqlTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "PackIndexPostgreSql" );

    CString connectionString;
    if ( !GetPostgreSqlConnectionString( connectionString ) )
    {
        GUCEF_LOG( LOGLEVEL_IMPORTANT, "PackIndexPostgreSql: GUCEF_TEST_POSTGRESQL is not set, the PostgreSQL tests are skipped" );
        GUCEF_TESTFW_TESTCASE( "PostgreSQL: the pack index against a PostgreSQL test database" )
            GUCEF_TESTFW_SKIP( "GUCEF_TEST_POSTGRESQL is not set" );
        GUCEF_TESTFW_TESTCASE_END;
        return;
    }
    {
        CString reachReason;
        if ( !IsPostgreSqlReachable( connectionString, reachReason ) )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "PackIndexPostgreSql: the test database cannot be reached: " << reachReason );
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

    SIndexFixture fixture;
    fixture.label = "pi_postgresql";
    fixture.base = "astest/pi_postgresql";
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

    AUTH::CMemoryCredentialStore credentials;
    const char* password = getenv( "PGPASSWORD" );
    if ( GUCEF_NULL != password && '\0' != password[ 0 ] )
    {
        AUTH::CSecretString secret( ( CString( password ) ) );
        credentials.Store( "gucefASSETSTORE.test", "catalogue", secret );
        fixture.credentials = &credentials;
    }
    GUCEF_LOG( LOGLEVEL_NORMAL, "PackIndexPostgreSql: testing in schema \"" << schema << "\"" );
    RunPackIndexSuite( fixture );

    GUCEF_TESTFW_TESTCASE( "PostgreSQL: the schema of the run of the pack index suite is dropped" )
        try
        {
            CPrimaryAssetStoreSettings settings;
            settings.database = fixture.database;
            if ( GUCEF_NULL != fixture.credentials )
            {
                settings.credentialProvider = "gucefASSETSTORE.test";
                settings.credentialAccount = "catalogue";
            }
            CString error;
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
