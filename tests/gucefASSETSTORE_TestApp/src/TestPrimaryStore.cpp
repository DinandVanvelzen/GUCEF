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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */
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
#include "TestPrimaryStore.h"

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
struct SStoreFixture
{
    CString label;
    CString base;                               /**< VFS root of the folders of the run */
    COMCORE::CDatabaseClientSettings database;
    AUTH::ICredentialStore* credentials;

    SStoreFixture( void ) : label(), base(), database(), credentials( GUCEF_NULL ) {}
};

CPrimaryAssetStoreSettings
MakeSettings( const SStoreFixture& fixture )
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

bool
WriteText( const CString& path, const CString& text )
{
    return WriteVfsFile( path, text );
}

bool
WritePng( const CString& path, UInt32 width, UInt32 height, bool normal )
{
    CDynamicBuffer content;
    return MakePng( width, height, normal, content ) && WriteVfsFile( path, content );
}

bool
WriteTga( const CString& path, UInt32 width, UInt32 height )
{
    CDynamicBuffer content;
    MakeTga( width, height, content );
    return WriteVfsFile( path, content );
}

const CAssetRecord*
FindAsset( const TAssetRecordVector& assets, const CString& name )
{
    for ( size_t a=0; a<assets.size(); ++a )
        if ( assets[ a ].name == name )
            return &assets[ a ];
    return GUCEF_NULL;
}

bool
HasHitNamed( const CAssetSearchResultPage& page, const CString& name )
{
    for ( size_t h=0; h<page.hits.size(); ++h )
        if ( page.hits[ h ].name == name )
            return true;
    return false;
}

CAssetSearchQuery
AllQuery( void )
{
    CAssetSearchQuery query;
    query.pageSize = 1000;
    return query;
}

UInt64
CountRows( CPrimaryAssetStore& store, const CString& table )
{
    CAssetCatalogue::TRowVector rows;
    store.GetCatalogue().SelectAll( table, rows );
    return rows.size();
}

/** The suite, against the database of the fixture */
void
RunPrimaryStoreSuite( SStoreFixture& fixture )
{
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CString& base = fixture.base;
    MountFreshDirectory( fixture.label + "_main", base + "/main" );
    MountFreshDirectory( fixture.label + "_mirror", base + "/mirror" );
    MountFreshDirectory( fixture.label + "_source", base + "/source" );

    // the analyser of models of the tests, in the place of the plugins that come later
    CTestModelAnalyzer* analyzer = GUCEF_NEW CTestModelAnalyzer();
    TAssetContentAnalyzerPtr analyzerPtr( analyzer );
    CAssetContentAnalyzerRegistry& analyzers = CAssetStoreGlobal::Instance()->GetContentAnalyzerRegistry();
    analyzers.Register( analyzerPtr );

    // the packs on disk
    const CString packA = base + "/source/Medieval_Weapons";
    bool sourcesWritten = WriteText( packA + "/pack.json", "{ \"title\": \"Medieval Weapons\", \"author\": \"Forge Works\", \"tags\": [ \"sword\", \"medieval\" ], "
                                                             "\"licence\": { \"name\": \"CC BY 4.0\", \"restrictions\": [ \"noMachineLearning\" ] } }" );
    sourcesWritten = WriteText( packA + "/Meshes/SM_Wep_Sword_01.fbx", "triangles=1200;vertices=900;rigged=0;animated=0" ) && sourcesWritten;
    sourcesWritten = WriteText( packA + "/Meshes/SM_Wep_Axe_01.fbx", "triangles=60000;vertices=40000;rigged=0" ) && sourcesWritten;
    sourcesWritten = WriteText( packA + "/Meshes/SM_Wep_Axe_01_LOD1.fbx", "triangles=20000;vertices=15000" ) && sourcesWritten;
    sourcesWritten = WriteText( packA + "/Meshes/UCX_SM_Wep_Axe_01.fbx", "triangles=12;vertices=8" ) && sourcesWritten;
    sourcesWritten = WritePng( packA + "/Textures/T_Sword_D.png", 64, 32, false ) && sourcesWritten;
    sourcesWritten = WriteTga( packA + "/Textures/T_Sword_N.tga", 128, 128 ) && sourcesWritten;
    CDynamicBuffer sound;
    MakePattern( 4096, 77, sound );
    sourcesWritten = WriteVfsFile( packA + "/Audio/sword_hit.wav", sound ) && sourcesWritten;
    const CString packD = base + "/source/Barrels";
    sourcesWritten = WriteText( packD + "/SM_Barrel_01.fbx", "triangles=400;vertices=300" ) && sourcesWritten;
    sourcesWritten = WriteText( packD + "/SM_Barrel_02.fbx", "triangles=450;vertices=320" ) && sourcesWritten;
    sourcesWritten = WriteText( base + "/mirror/Rocks/SM_Rock_01.fbx", "triangles=300;vertices=200" ) && sourcesWritten;
    sourcesWritten = WritePng( base + "/mirror/Rocks/T_Rock_D.png", 32, 32, false ) && sourcesWritten;

    CPrimaryAssetStore store;
    store.SetSettings( MakeSettings( fixture ) );
    store.SetCredentialStore( fixture.credentials );
    CString packAId;
    CString packBId;
    CString rockAssetId;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": migrations make the tables, then seed the vocabulary; a newer catalogue is refused" ).C_String() )
        try
        {
            ASSERT_TRUE( sourcesWritten );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Open( 1 ) );
            ASSERT_TRUE( 1 == store.GetCatalogue().GetSchemaVersion() );
            ASSERT_TRUE( 0 == CountRows( store, "categories" ) && 0 == CountRows( store, "saved_searches" ) );
            store.Close();
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Open() );
            ASSERT_TRUE( CAssetCatalogue::CurrentSchemaVersion == store.GetCatalogue().GetSchemaVersion() );
            ASSERT_TRUE( CountRows( store, "categories" ) > 100 && 25 == CountRows( store, "saved_searches" ) );
            ASSERT_TRUE( CountRows( store, "category_map" ) >= 100 && CountRows( store, "category_rules" ) >= 90 && CountRows( store, "licence_map" ) >= 20 );
            store.Close();
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Open() );
            ASSERT_TRUE( CAssetCatalogue::CurrentSchemaVersion == store.GetCatalogue().GetSchemaVersion() && 6 == CAssetCatalogue::CurrentSchemaVersion && CAssetCatalogue::CurrentSchemaVersion == CountRows( store, "schema_version" ) );
            ASSERT_TRUE( 2 == CountRows( store, "store_locations" ) );

            // a catalogue of a later build
            CAssetCatalogue::TRowVector rows( 1 );
            rows[ 0 ][ "version" ] = CVariant( (Int64) 99 );
            rows[ 0 ][ "description" ] = CVariant( "from the future" );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetCatalogue().Write( "schema_version", rows ) );
            CAssetCatalogue other;
            ASSERT_TRUE( ASSETSTORE_STATUS_UNSUPPORTED == other.Open( store.GetCatalogue().GetClient(), GUCEF_NULL ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetCatalogue().DeleteWhereEquals( "schema_version", "version", CVariant( (Int64) 99 ) ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == other.Open( store.GetCatalogue().GetClient(), GUCEF_NULL ) && CAssetCatalogue::CurrentSchemaVersion == other.GetSchemaVersion() );
            other.Close();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a folder from disk is copied, identified, normalised, analysed, placed, indexed and recorded" ).C_String() )
        try
        {
            CAssetIngestRequest request;
            request.sourceId = "disk";
            request.contentPath = packA;
            CAssetIngestResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Ingest( request, result ) );
            ASSERT_TRUE( ASSETPACK_STATE_CONVERSION_PENDING == result.state && 7 == result.assetCount );
            packAId = result.packId;
            ASSERT_TRUE( vfs.FileExists( packA + "/Meshes/SM_Wep_Sword_01.fbx" ) );

            CAssetPackRecord pack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( packAId, pack ) );
            ASSERT_TRUE( "Medieval Weapons" == pack.title && "Forge Works" == pack.seller && "disk" == pack.sourceId && "main" == pack.locationId );
            ASSERT_TRUE( pack.rootPath == "layout2:disk/Medieval_Weapons" );
            ASSERT_TRUE( CPackStorageLayout::LayoutAreas == CPackStorageLayout( pack.rootPath ).GetVersion() && "disk/Medieval_Weapons" == CPackStorageLayout( pack.rootPath ).GetPackFolder() );
            ASSERT_TRUE( vfs.DirExists( base + "/main/original/disk/Medieval_Weapons/Meshes" ) && !vfs.DirExists( base + "/main/packs" ) );
            ASSERT_TRUE( ASSETPACK_STATE_CONVERSION_PENDING == pack.state && ASSETKEEP_BOTH == pack.keep );
            ASSERT_TRUE( pack.categories.end() != pack.categories.find( "weapons/melee" ) );
            ASSERT_TRUE( pack.themes.end() != pack.themes.find( "medieval" ) );
            ASSERT_TRUE( "attribution" == pack.licence.licenceClass && "CC BY 4.0" == pack.licence.name );
            ASSERT_TRUE( pack.licence.HasRestriction( CAssetLicence::NoMachineLearning ) && pack.licence.HasRestriction( CAssetLicence::AttributionRequired ) );
            ASSERT_TRUE( pack.formatsOriginal.end() != pack.formatsOriginal.find( "fbx" ) && pack.pbrRoles.end() != pack.pbrRoles.find( "normal" ) );
            ASSERT_TRUE( 7 == pack.fileCount && 7 == pack.assetCount && pack.ingestedAtMs > 0 );
            ASSERT_TRUE( pack.analyzers.end() != pack.analyzers.find( "texture/1" ) && pack.analyzers.end() != pack.analyzers.find( "testmodel/1" ) );

            // the placed layout
            const CString root = base + "/main/original/disk/Medieval_Weapons";
            ASSERT_TRUE( vfs.FileExists( root + "/Meshes/SM_Wep_Sword_01.fbx" ) && vfs.FileExists( root + "/Textures/T_Sword_N.tga" ) );
            ASSERT_TRUE( vfs.FileExists( root + "/pack.json" ) );
            CString::StringVector pending;
            store.GetPendingIngests( pending );
            ASSERT_TRUE( pending.empty() );

            // the catalogue rows
            TAssetRecordVector assets;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetAssets( packAId, assets ) && 7 == assets.size() );
            const CAssetRecord* sword = FindAsset( assets, "SM_Wep_Sword_01" );
            const CAssetRecord* axe = FindAsset( assets, "SM_Wep_Axe_01" );
            const CAssetRecord* albedo = FindAsset( assets, "T_Sword_D" );
            const CAssetRecord* normal = FindAsset( assets, "T_Sword_N" );
            const CAssetRecord* hit = FindAsset( assets, "sword_hit" );
            ASSERT_TRUE( GUCEF_NULL != sword && GUCEF_NULL != axe && GUCEF_NULL != albedo && GUCEF_NULL != normal && GUCEF_NULL != hit );
            if ( GUCEF_NULL != sword && GUCEF_NULL != axe && GUCEF_NULL != albedo && GUCEF_NULL != normal && GUCEF_NULL != hit )
            {
                ASSERT_TRUE( "static-model" == sword->kind && "fbx" == sword->sourceFormat && "weapons/melee" == sword->categoryId );
                ASSERT_TRUE( sword->model.isSet && 1200 == sword->model.triangles && 900 == sword->model.vertices && !sword->model.rigged );
                ASSERT_TRUE( "original/Meshes/SM_Wep_Sword_01.fbx" == sword->originalPath && 64 == sword->digest.Length() );
                ASSERT_TRUE( 1 == sword->files.size() && sword->files[ 0 ].digest == sword->digest );
                ASSERT_TRUE( axe->model.hasCollision && 2 == axe->model.lods );
                ASSERT_TRUE( albedo->texture.isSet && 64 == albedo->texture.width && 32 == albedo->texture.height && 3 == albedo->texture.channels );
                ASSERT_TRUE( "albedo" == albedo->texture.role && "srgb" == albedo->texture.colourSpace && "png" == albedo->texture.compression );
                ASSERT_TRUE( normal->texture.isSet && 128 == normal->texture.width && "normal" == normal->texture.role && "linear" == normal->texture.colourSpace );
                ASSERT_TRUE( "sound-effect" == hit->kind && 4096 == hit->sizeBytes );
            }
            ASSERT_TRUE( analyzer->GetAnalysedCount() >= 4 );

            GUCEF::vector< CAssetIngestHistoryEntry > history;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetIngestHistory( history, packAId ) );
            ASSERT_TRUE( 1 == history.size() && "ingested" == history[ 0 ].outcome && "record" == history[ 0 ].step && history[ 0 ].durationMs >= 0 );
            ASSERT_TRUE( !history.empty() && history[ 0 ].origin == packA && history[ 0 ].policy.HasSubstr( "keep=both" ) >= 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a pack whose files the catalogue has is a duplicate: skipped, or kept when asked" ).C_String() )
        try
        {
            CAssetIngestRequest request;
            request.sourceId = "disk";
            request.contentPath = packA;
            CAssetIngestResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_DUPLICATE == store.Ingest( request, result ) );
            ASSERT_TRUE( packAId == result.duplicateOfPackId );
            CAssetPackRecord gone;
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == store.GetPack( result.packId, gone ) );
            GUCEF::vector< CAssetIngestHistoryEntry > history;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetIngestHistory( history, result.packId ) && 1 == history.size() && "duplicate" == history[ 0 ].outcome );

            request.duplicatePolicy = "keep";
            CAssetIngestResult kept;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Ingest( request, kept ) && packAId == kept.duplicateOfPackId );
            CAssetPackRecord copy;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( kept.packId, copy ) && copy.packId != packAId );
            // the name is taken: the folder of the copy has the short id of its pack
            ASSERT_TRUE( copy.rootPath == "layout2:disk/Medieval_Weapons [" + CPackStorageLayout::MakeShortId( copy.packId ) + "]" );
            const CString copyRoot = CPackStorageLayout( copy.rootPath ).GetOriginalFolder( base + "/main" );
            ASSERT_TRUE( vfs.DirExists( copyRoot ) && vfs.DirExists( base + "/main/original/disk/Medieval_Weapons" ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.RemovePack( kept.packId, true ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == store.GetPack( kept.packId, copy ) );
            ASSERT_FALSE( vfs.DirExists( copyRoot ) );
            TAssetRecordVector assets;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetAssets( kept.packId, assets ) && assets.empty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a download of a store in the staging area is ingested with the item of the store; the same item again is a duplicate" ).C_String() )
        try
        {
            const CString staging = store.MakeDownloadStagingPath();
            ASSERT_TRUE( 0 == staging.HasSubstr( base + "/main/staging/dl" ) );
            ASSERT_TRUE( WriteText( staging + "/SK_Knight.fbx", "triangles=8000;vertices=6000;rigged=1;animated=1;bones=50;animations=3" ) );
            ASSERT_TRUE( WritePng( staging + "/Textures/T_Knight_BaseColor.png", 256, 256, false ) );
            ASSERT_TRUE( WritePng( staging + "/Textures/T_Knight_Normal.png", 512, 512, true ) );
            CAssetIngestRequest request;
            request.sourceId = "fab";
            request.contentPath = staging;
            request.downloadJobId = "dl-test";
            request.item.itemId = "fab-knight";
            request.item.version = "1.0";
            request.item.title = "Knight Character";
            request.item.seller = "Synty Studios";
            request.item.licenceName = "Standard License";
            request.item.categories.push_back( "3D/Characters & Creatures" );
            request.item.categories.push_back( "3D/Unheard Of" );
            request.item.tags.push_back( "knight" );
            CAssetIngestResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Ingest( request, result ) && 3 == result.assetCount );
            packBId = result.packId;
            ASSERT_FALSE( vfs.DirExists( staging ) );
            CAssetPackRecord pack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( packBId, pack ) );
            ASSERT_TRUE( "fab" == pack.sourceId && "fab-knight" == pack.sourceItemId && "1.0" == pack.sourceVersion && "storeItem/1" == pack.normalizer );
            ASSERT_TRUE( pack.categories.end() != pack.categories.find( "characters" ) );
            ASSERT_TRUE( 1 == pack.unmappedCategories.size() && pack.unmappedCategories.end() != pack.unmappedCategories.find( "3D/Unheard Of" ) );
            ASSERT_TRUE( pack.artStyles.end() != pack.artStyles.find( "low-poly" ) && pack.artStyles.end() != pack.artStyles.find( "stylised" ) );
            ASSERT_TRUE( "royalty-free" == pack.licence.licenceClass && pack.licence.AllowsMachineLearning() );
            ASSERT_TRUE( "layout2:fab/Knight Character" == pack.rootPath );
            TAssetRecordVector assets;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetAssets( packBId, assets ) );
            const CAssetRecord* knight = FindAsset( assets, "SK_Knight" );
            ASSERT_TRUE( GUCEF_NULL != knight && "skeletal-model" == knight->kind && knight->model.rigged && knight->model.animated && 8000 == knight->model.triangles );
            GUCEF::vector< CAssetIngestHistoryEntry > history;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetIngestHistory( history, packBId ) && 1 == history.size() && "download dl-test" == history[ 0 ].origin );

            // the same item of the same store, other files
            const CString again = store.MakeDownloadStagingPath();
            ASSERT_TRUE( WriteText( again + "/SK_Knight_v2.fbx", "triangles=9000" ) );
            CAssetIngestRequest same( request );
            same.contentPath = again;
            CAssetIngestResult duplicate;
            ASSERT_TRUE( ASSETSTORE_STATUS_DUPLICATE == store.Ingest( same, duplicate ) && packBId == duplicate.duplicateOfPackId );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": keyword search with ranking, prefixes and the intersection of words" ).C_String() )
        try
        {
            CAssetSearchQuery query = AllQuery();
            query.text = "sword";
            CAssetSearchResultPage page;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) );
            ASSERT_TRUE( 7 == page.total && !page.hits.empty() && "SM_Wep_Sword_01" == page.hits[ 0 ].name );
            ASSERT_TRUE( page.hits[ 0 ].score > page.hits[ page.hits.size()-1 ].score && page.hits[ 0 ].available );
            ASSERT_TRUE( page.hits[ 0 ].vfsPath == base + "/main/original/disk/Medieval_Weapons/Meshes/SM_Wep_Sword_01.fbx" );

            query.text = "swo";
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 0 == page.total );
            query.prefixLastWord = true;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 7 == page.total );
            query.text = "kni";
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 3 == page.total );

            query.prefixLastWord = false;
            query.text = "Medieval SWORDS";
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 7 == page.total && "SM_Wep_Sword_01" == page.hits[ 0 ].name );
            query.text = "knight sword";
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 0 == page.total );
            query.text = "knight normal";
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 1 == page.total && "T_Knight_Normal" == page.hits[ 0 ].name );

            CString::StringVector suggestions;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Suggest( "Kn", 5, suggestions ) && !suggestions.empty() && "knight" == suggestions[ 0 ] );

            CAssetSearchQuery asyncQuery = AllQuery();
            asyncQuery.text = "axe";
            CAssetSearchResultPage asyncPage;
            ASSERT_TRUE( GetFutureResult( store.SearchAsync( asyncQuery ), asyncPage ) && asyncPage.total >= 1 && HasHitNamed( asyncPage, "SM_Wep_Axe_01" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": typed filters on kind, category, format, bytes, polygons, resolution, flags, store, seller, theme; facets and pages" ).C_String() )
        try
        {
            CAssetSearchResultPage page;
            CAssetSearchQuery all = AllQuery();
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( all, page ) && 10 == page.total );
            ASSERT_TRUE( 4 == page.GetFacetCount( "kind", "texture" ) && 1 == page.GetFacetCount( "kind", "skeletal-model" ) );
            ASSERT_TRUE( 7 == page.GetFacetCount( "source", "disk" ) && 3 == page.GetFacetCount( "source", "fab" ) );
            ASSERT_TRUE( 7 == page.GetFacetCount( "licenceClass", "attribution" ) && 3 == page.GetFacetCount( "category", "characters" ) );

            CAssetSearchQuery paged = AllQuery();
            paged.pageSize = 3;
            paged.pageIndex = 3;
            paged.sort = "name";
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( paged, page ) && 10 == page.total && 1 == page.hits.size() );

            CAssetSearchQuery query = AllQuery();
            query.kinds.insert( "texture" );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 4 == page.total );
            query = AllQuery();
            query.categories.insert( "weapons" );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && page.total >= 2 && HasHitNamed( page, "SM_Wep_Sword_01" ) );
            query = AllQuery();
            query.maxTriangles = 1500;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 2 == page.total && HasHitNamed( page, "SM_Wep_Sword_01" ) && HasHitNamed( page, "UCX_SM_Wep_Axe_01" ) );
            query = AllQuery();
            query.minTriangles = 50000;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 1 == page.total && "SM_Wep_Axe_01" == page.hits[ 0 ].name );
            query = AllQuery();
            query.minTextureResolution = 300;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 1 == page.total && "T_Knight_Normal" == page.hits[ 0 ].name );
            query = AllQuery();
            query.maxTriangles = 2000;
            query.maxTextureResolution = 100;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 3 == page.total && HasHitNamed( page, "T_Sword_D" ) );
            query = AllQuery();
            query.rigged = 1;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 1 == page.total && "SK_Knight" == page.hits[ 0 ].name );
            query = AllQuery();
            query.hasCollision = 1;
            query.hasLods = 1;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 1 == page.total && "SM_Wep_Axe_01" == page.hits[ 0 ].name );
            query = AllQuery();
            query.minAssetBytes = 4000;
            query.maxAssetBytes = 5000;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 1 == page.total && "sword_hit" == page.hits[ 0 ].name );
            query = AllQuery();
            query.sources.insert( "FAB" );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 3 == page.total );
            query = AllQuery();
            query.sellers.insert( "Forge Works" );
            query.kinds.insert( "static-model" );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 4 == page.total );
            query = AllQuery();
            query.themes.insert( "medieval" );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 7 == page.total );
            query = AllQuery();
            query.artStyles.insert( "stylised" );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 3 == page.total );

            CAssetSearchQuery packs = AllQuery();
            packs.resultKind = "packs";
            packs.formats.insert( "fbx" );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( packs, page ) && 2 == page.total && page.hits[ 0 ].assetId.IsNULLOrEmpty() );
            packs.formats.clear();
            packs.formats.insert( "gltf" );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( packs, page ) && 0 == page.total );
            packs.formats.clear();
            packs.pbrRoles.insert( "normal" );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( packs, page ) && 2 == page.total );
            packs.pbrRoles.clear();
            packs.packFormatsOnly.insert( "fbx" );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( packs, page ) && 2 == page.total );
            packs.packFormatsOnly.clear();
            packs.packStates.insert( "failed" );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( packs, page ) && 0 == page.total );
            packs.packStates.clear();
            packs.sort = "newest";
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( packs, page ) && 2 == page.total && packBId == page.hits[ 0 ].packId );
            ASSERT_TRUE( 1 == page.GetFacetCount( "source", "fab" ) && 1 == page.GetFacetCount( "kind", "skeletal-model" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": licence restriction flags are stored, filtered on, and leave packs out of learning based features" ).C_String() )
        try
        {
            CAssetCatalogue::TRowVector licences;
            CDataTableQuery query;
            query.WhereEquals( "licence_class", CVariant( "attribution" ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetCatalogue().Select( "licences", query, licences ) && 1 == licences.size() );
            ASSERT_TRUE( !licences.empty() && CAssetCatalogue::SplitSet( CAssetCatalogue::GetText( licences[ 0 ], "restrictions" ) ).count( "noMachineLearning" ) == 1 );

            CAssetSearchQuery required = AllQuery();
            required.resultKind = "packs";
            required.requireLicenceRestrictions.insert( CAssetLicence::NoMachineLearning );
            CAssetSearchResultPage page;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( required, page ) && 1 == page.total && packAId == page.hits[ 0 ].packId );

            CAssetSearchQuery excluded = AllQuery();
            CPrimaryAssetStore::ExcludeRestricted( excluded, CAssetLicence::NoMachineLearning );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( excluded, page ) && 3 == page.total );
            for ( size_t h=0; h<page.hits.size(); ++h )
                ASSERT_TRUE( packBId == page.hits[ h ].packId );

            CString::StringSet allowed;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPacksAllowingUse( CAssetLicence::NoMachineLearning, allowed ) );
            ASSERT_TRUE( 1 == allowed.size() && 1 == allowed.count( packBId ) );
            bool isAllowed = true;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.IsUseAllowed( packAId, CAssetLicence::NoMachineLearning, isAllowed ) && !isAllowed );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.IsUseAllowed( packBId, CAssetLicence::NoMachineLearning, isAllowed ) && isAllowed );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.IsUseAllowed( packAId, CAssetLicence::NoCommercialUse, isAllowed ) && isAllowed );
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == store.IsUseAllowed( "pk-none", CAssetLicence::NoMachineLearning, isAllowed ) );

            CAssetStoreItem item;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetItem( packAId, item ) && 1 == item.licenceRestrictions.count( CAssetLicence::NoMachineLearning ) );
            ASSERT_TRUE( 1 == item.formats.size() && 7 == item.formats[ 0 ].files.size() && "CC BY 4.0" == item.licenceName );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": the precanned saved searches run, users add and remove their own" ).C_String() )
        try
        {
            TAssetSavedSearchVector searches;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetSavedSearches( searches ) && 25 == searches.size() );
            ASSERT_TRUE( !searches.empty() && "characters-rigged" == searches[ 0 ].searchId && searches[ 0 ].precanned );
            CAssetSearchResultPage page;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.RunSavedSearch( "hero-assets", 0, page ) && 1 == page.total && "SM_Wep_Axe_01" == page.hits[ 0 ].name );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.RunSavedSearch( "characters-rigged", 0, page ) && 1 == page.total && "SK_Knight" == page.hits[ 0 ].name );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.RunSavedSearch( "licence-no-machine-learning", 0, page ) && 1 == page.total && packAId == page.hits[ 0 ].packId );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.RunSavedSearch( "conversion-pending", 0, page ) && 2 == page.total );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.RunSavedSearch( "gltf-or-fbx", 0, page ) && 2 == page.total );
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == store.RunSavedSearch( "nope", 0, page ) );

            CAssetSavedSearch mine;
            mine.searchId = "my-swords";
            mine.name = "My swords";
            mine.query.text = "sword";
            mine.query.kinds.insert( "static-model" );
            mine.sortOrder = 100;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.SaveSearch( mine ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetSavedSearches( searches ) && 26 == searches.size() );
            ASSERT_TRUE( "my-swords" == searches.back().searchId && "sword" == searches.back().query.text && 1 == searches.back().query.kinds.size() && !searches.back().precanned );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.RunSavedSearch( "my-swords", 0, page ) && 4 == page.total );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.DeleteSavedSearch( "my-swords" ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetSavedSearches( searches ) && 25 == searches.size() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a pack in place in a location; the location goes offline, its packs are unavailable, it is relocated" ).C_String() )
        try
        {
            CAssetIngestRequest request;
            request.sourceId = "disk";
            request.contentPath = base + "/mirror/Rocks";
            request.copyContent = 0;
            CAssetIngestResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Ingest( request, result ) );
            CAssetPackRecord pack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( result.packId, pack ) && "mirror" == pack.locationId && "Rocks" == pack.rootPath );
            ASSERT_TRUE( vfs.FileExists( base + "/mirror/Rocks/SM_Rock_01.fbx" ) );
            TAssetRecordVector assets;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetAssets( result.packId, assets ) );
            const CAssetRecord* rock = FindAsset( assets, "SM_Rock_01" );
            ASSERT_TRUE( GUCEF_NULL != rock && "SM_Rock_01.fbx" == rock->originalPath );
            if ( GUCEF_NULL != rock )
                rockAssetId = rock->assetId;
            CString path;
            CString message;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetAssetVfsPath( rockAssetId, path, message ) && path == base + "/mirror/Rocks/SM_Rock_01.fbx" );

            // the folder of the location is no longer mounted
            UnmountDirectory( fixture.label + "_mirror" );
            store.RefreshLocationStates();
            ASSERT_FALSE( store.IsLocationOnline( "mirror" ) );
            ASSERT_TRUE( store.IsLocationOnline( "main" ) );
            CAssetSearchQuery query = AllQuery();
            query.text = "rock";
            CAssetSearchResultPage page;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 2 == page.total );
            for ( size_t h=0; h<page.hits.size(); ++h )
                ASSERT_TRUE( !page.hits[ h ].available && "mirror" == page.hits[ h ].locationId );
            ASSERT_TRUE( ASSETSTORE_STATUS_UNAVAILABLE == store.GetAssetVfsPath( rockAssetId, path, message ) && message.HasSubstr( "mirror" ) >= 0 );
            CAssetIngestRequest offline;
            offline.sourceId = "disk";
            offline.contentPath = packD;
            offline.locationId = "mirror";
            CAssetIngestResult offlineResult;
            ASSERT_TRUE( ASSETSTORE_STATUS_UNAVAILABLE == store.Ingest( offline, offlineResult ) );

            // the same folder at another root
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.RelocateLocation( "mirror", base + "/mirror2", TestDirectory( fixture.label + "_mirror" ) ) );
            ASSERT_TRUE( store.IsLocationOnline( "mirror" ) && store.GetLocationRoot( "mirror" ) == base + "/mirror2" );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetAssetVfsPath( rockAssetId, path, message ) && path == base + "/mirror2/Rocks/SM_Rock_01.fbx" );
            ASSERT_TRUE( vfs.FileExists( path ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 2 == page.total && page.hits[ 0 ].available );
            CAssetCatalogue::TRowVector rows;
            CDataTableQuery locationQuery;
            locationQuery.WhereEquals( "location_id", CVariant( "mirror" ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetCatalogue().Select( "store_locations", locationQuery, rows ) && 1 == rows.size() );
            ASSERT_TRUE( !rows.empty() && base + "/mirror2" == CAssetCatalogue::GetText( rows[ 0 ], "vfs_root" ) && CAssetCatalogue::GetBool( rows[ 0 ], "online" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a location is verified in the background; missing and changed files are found" ).C_String() )
        try
        {
            CAssetVerificationReport report;
            ASSERT_TRUE( GetFutureResult( store.VerifyLocationAsync( "main" ), report ) );
            ASSERT_TRUE( report.IsIntact() && 2 == report.packsChecked && 10 == report.filesChecked );

            CAssetPackRecord pack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( packAId, pack ) );
            const CString root = base + "/main/original/disk/Medieval_Weapons";
            ASSERT_TRUE( WriteText( root + "/Meshes/SM_Wep_Sword_01.fbx", "triangles=1;changed" ) );
            ASSERT_TRUE( vfs.DeleteFile( root + "/Audio/sword_hit.wav", false ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.VerifyLocation( "main", report ) );
            ASSERT_FALSE( report.IsIntact() );
            ASSERT_TRUE( 1 == report.missingFiles.size() && 1 == report.mismatchedFiles.size() );
            ASSERT_TRUE( !report.mismatchedFiles.empty() && report.mismatchedFiles[ 0 ].HasSubstr( "SM_Wep_Sword_01.fbx" ) >= 0 );
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == store.VerifyLocation( "nowhere", report ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a failed ingest stays staged with its reason and resumes from the step that failed" ).C_String() )
        try
        {
            analyzer->FailNext( 1 );
            CAssetIngestRequest request;
            request.sourceId = "disk";
            request.contentPath = packD;
            CAssetIngestResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_IO_ERROR == store.Ingest( request, result ) );
            ASSERT_TRUE( ASSETINGEST_STEP_ANALYSE == result.failedStep && ASSETPACK_STATE_FAILED == result.state );
            CAssetPackRecord pack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( result.packId, pack ) && ASSETPACK_STATE_FAILED == pack.state );
            ASSERT_TRUE( pack.stateReason.HasSubstr( "analyse" ) >= 0 && pack.stateReason.HasSubstr( "testmodel" ) >= 0 );
            CString::StringVector pending;
            store.GetPendingIngests( pending );
            ASSERT_TRUE( 1 == pending.size() && result.ingestId == pending[ 0 ] );
            CAssetSearchQuery query = AllQuery();
            query.text = "barrel";
            CAssetSearchResultPage page;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 0 == page.total );

            CAssetIngestResult resumed;
            ASSERT_TRUE( GetFutureResult( store.ResumeIngestAsync( result.ingestId ), resumed ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == resumed.status && result.packId == resumed.packId && 2 == resumed.assetCount );
            store.GetPendingIngests( pending );
            ASSERT_TRUE( pending.empty() );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( result.packId, pack ) && ASSETPACK_STATE_CONVERSION_PENDING == pack.state && pack.stateReason.IsNULLOrEmpty() );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 2 == page.total );
            GUCEF::vector< CAssetIngestHistoryEntry > history;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetIngestHistory( history, result.packId ) && 2 == history.size() );
            ASSERT_TRUE( 2 == history.size() && "failed" == history[ 0 ].outcome && "analyse" == history[ 0 ].step && "ingested" == history[ 1 ].outcome );
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == store.ResumeIngest( result.ingestId, resumed ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a pack whose indexing was interrupted is repaired when the store opens" ).C_String() )
        try
        {
            CAssetCatalogue::TRowVector packRows( 1 );
            packRows[ 0 ][ "pack_id" ] = CVariant( "pk-interrupted" );
            packRows[ 0 ][ "title" ] = CVariant( "Half Written" );
            packRows[ 0 ][ "location_id" ] = CVariant( "main" );
            packRows[ 0 ][ "state" ] = CVariant( "indexing" );
            CAssetCatalogue::TRowVector assetRows( 1 );
            assetRows[ 0 ][ "asset_id" ] = CVariant( "as-interrupted" );
            assetRows[ 0 ][ "pack_id" ] = CVariant( "pk-interrupted" );
            assetRows[ 0 ][ "name" ] = CVariant( "orphan" );
            CAssetCatalogue::TRowVector keywordRows( 1 );
            keywordRows[ 0 ][ "keyword" ] = CVariant( "orphan" );
            keywordRows[ 0 ][ "object_id" ] = CVariant( "as-interrupted" );
            keywordRows[ 0 ][ "object_type" ] = CVariant( "asset" );
            keywordRows[ 0 ][ "pack_id" ] = CVariant( "pk-interrupted" );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetCatalogue().Write( "packs", packRows ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetCatalogue().Write( "assets", assetRows ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetCatalogue().Write( "keywords", keywordRows ) );
            store.Close();
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Open() );
            CAssetPackRecord pack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( "pk-interrupted", pack ) && ASSETPACK_STATE_FAILED == pack.state );
            ASSERT_TRUE( pack.stateReason.HasSubstr( "interrupted" ) >= 0 );
            TAssetRecordVector assets;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetAssets( "pk-interrupted", assets ) && assets.empty() );
            CAssetSearchQuery query = AllQuery();
            query.text = "orphan";
            CAssetSearchResultPage page;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 0 == page.total );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.RemovePack( "pk-interrupted", false ) );

            // the rest of the catalogue came through the reopening
            CAssetSearchQuery all = AllQuery();
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( all, page ) && 14 == page.total );
            ASSERT_TRUE( store.IsLocationOnline( "mirror" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": the ingest history lists every ingest in order, and the store browses as a provider" ).C_String() )
        try
        {
            GUCEF::vector< CAssetIngestHistoryEntry > history;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetIngestHistory( history ) );
            UInt32 ingested = 0;
            UInt32 duplicates = 0;
            UInt32 failed = 0;
            for ( size_t h=0; h<history.size(); ++h )
            {
                ingested += "ingested" == history[ h ].outcome ? 1 : 0;
                duplicates += "duplicate" == history[ h ].outcome ? 1 : 0;
                failed += "failed" == history[ h ].outcome ? 1 : 0;
                if ( h > 0 )
                    ASSERT_TRUE( history[ h-1 ].startedAtMs <= history[ h ].startedAtMs );
            }
            ASSERT_TRUE( 5 == ingested && 2 == duplicates && 1 == failed );

            CAssetStoreQuery library;
            library.pageSize = 2;
            CAssetStoreLibraryPage page;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.QueryLibrary( library, page ) && 2 == page.items.size() && 4 == page.total && "1" == page.nextCursor );
            library.text = "knight";
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.QueryLibrary( library, page ) && 1 == page.items.size() && packBId == page.items[ 0 ].itemId );
            CAssetStoreDownloadPlan plan;
            ASSERT_TRUE( ASSETSTORE_STATUS_UNSUPPORTED == store.PlanDownload( packBId, "original", CString::StringVector(), plan ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": layout 2: a folder on the disk that the catalogue does not know (another catalogue on the same share, a left over) is never taken over: the pack gets the short id, the files there stay" ).C_String() )
        try
        {
            const CString mainRoot = base + "/main";
            ASSERT_TRUE( WriteText( mainRoot + "/original/disk/Foreign_Pack/keep.txt", "of another catalogue" ) );
            ASSERT_TRUE( WriteText( mainRoot + "/gucef-ogre3d/disk/Other_Foreign/keep.mesh", "cooked by another catalogue" ) );
            ASSERT_TRUE( WriteText( base + "/source/Foreign_Pack/Meshes/SM_Foreign.fbx", "triangles=23;vertices=11" ) );
            ASSERT_TRUE( WriteText( base + "/source/Other_Foreign/Meshes/SM_Other.fbx", "triangles=24;vertices=12" ) );
            CAssetIngestRequest request;
            request.sourceId = "disk";
            request.contentPath = base + "/source/Foreign_Pack";
            CAssetIngestResult first;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Ingest( request, first ) );
            request.contentPath = base + "/source/Other_Foreign";
            CAssetIngestResult second;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Ingest( request, second ) );
            CAssetPackRecord packFirst;
            CAssetPackRecord packSecond;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( first.packId, packFirst ) && ASSETSTORE_STATUS_OK == store.GetPack( second.packId, packSecond ) );
            ASSERT_TRUE( "layout2:disk/Foreign_Pack [" + CPackStorageLayout::MakeShortId( first.packId ) + "]" == packFirst.rootPath );
            ASSERT_TRUE( "layout2:disk/Other_Foreign [" + CPackStorageLayout::MakeShortId( second.packId ) + "]" == packSecond.rootPath );
            const CPackStorageLayout layoutFirst( packFirst.rootPath );
            ASSERT_TRUE( vfs.FileExists( layoutFirst.Resolve( mainRoot, "original/Meshes/SM_Foreign.fbx" ) ) );
            ASSERT_FALSE( vfs.FileExists( mainRoot + "/original/disk/Foreign_Pack/Meshes/SM_Foreign.fbx" ) );

            // removing the packs with their files leaves the folders the catalogue never owned
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.RemovePack( first.packId, true ) && ASSETSTORE_STATUS_OK == store.RemovePack( second.packId, true ) );
            ASSERT_TRUE( vfs.FileExists( mainRoot + "/original/disk/Foreign_Pack/keep.txt" ) && vfs.FileExists( mainRoot + "/gucef-ogre3d/disk/Other_Foreign/keep.mesh" ) );
            ASSERT_FALSE( vfs.DirExists( layoutFirst.GetOriginalFolder( mainRoot ) ) );
            vfs.DeleteDir( mainRoot + "/original/disk/Foreign_Pack", true );
            vfs.DeleteDir( mainRoot + "/gucef-ogre3d/disk/Other_Foreign", true );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": layout 2: a name that is taken gets the short id of its pack, removing a pack removes the folders of its three areas" ).C_String() )
        try
        {
            ASSERT_TRUE( WriteText( base + "/source/Case_Pack/Meshes/SM_Case_A.fbx", "triangles=21;vertices=9" ) );
            ASSERT_TRUE( WriteText( base + "/source/case_pack/Meshes/SM_Case_B.fbx", "triangles=22;vertices=10" ) );
            CAssetIngestRequest request;
            request.sourceId = "disk";
            request.contentPath = base + "/source/Case_Pack";
            CAssetIngestResult first;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Ingest( request, first ) );
            request.contentPath = base + "/source/case_pack";
            CAssetIngestResult second;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Ingest( request, second ) );

            CAssetPackRecord packFirst;
            CAssetPackRecord packSecond;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( first.packId, packFirst ) && ASSETSTORE_STATUS_OK == store.GetPack( second.packId, packSecond ) );
            ASSERT_TRUE( "layout2:disk/Case_Pack" == packFirst.rootPath );
            ASSERT_TRUE( "layout2:disk/case_pack [" + CPackStorageLayout::MakeShortId( second.packId ) + "]" == packSecond.rootPath );
            const CString mainRoot = base + "/main";
            const CPackStorageLayout layoutFirst( packFirst.rootPath );
            const CPackStorageLayout layoutSecond( packSecond.rootPath );
            ASSERT_TRUE( vfs.FileExists( layoutFirst.Resolve( mainRoot, "original/Meshes/SM_Case_A.fbx" ) ) );
            ASSERT_TRUE( vfs.FileExists( layoutSecond.Resolve( mainRoot, "original/Meshes/SM_Case_B.fbx" ) ) );
            ASSERT_FALSE( vfs.DirExists( mainRoot + "/staging/" + first.ingestId ) );

            // the paths of the assets follow the folder of their pack
            TAssetRecordVector assets;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetAssets( first.packId, assets ) && 1 == assets.size() );
            CString vfsPath, message;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetAssetVfsPath( ( assets.empty() ? CString::Empty : assets[ 0 ].assetId ), vfsPath, message ) && vfsPath == layoutFirst.Resolve( mainRoot, "original/Meshes/SM_Case_A.fbx" ) );

            // what the conversion and the cooking leave goes with the pack, the other pack stays
            ASSERT_TRUE( WriteText( layoutSecond.GetIntermediateFolder( mainRoot, "test" ) + "/report.json", "{}" ) );
            ASSERT_TRUE( WriteText( layoutSecond.GetCookedFolder( mainRoot, "test" ) + "/desktop/a.mesh", "mesh" ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.RemovePack( second.packId, true ) );
            ASSERT_FALSE( vfs.DirExists( layoutSecond.GetOriginalFolder( mainRoot ) ) || vfs.DirExists( layoutSecond.GetIntermediateFolder( mainRoot, "test" ) ) ||
                          vfs.DirExists( layoutSecond.GetCookedFolder( mainRoot, "test" ) ) );
            ASSERT_TRUE( vfs.FileExists( layoutFirst.Resolve( mainRoot, "original/Meshes/SM_Case_A.fbx" ) ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.RemovePack( first.packId, true ) );
            ASSERT_FALSE( vfs.DirExists( layoutFirst.GetOriginalFolder( mainRoot ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a pack of the earlier layout stays readable and moves to the three areas by the migration: dry run, refusal, apply, again" ).C_String() )
        try
        {
            ASSERT_TRUE( WriteText( base + "/source/Layout_Old/Meshes/SM_Old_01.fbx", "triangles=31;vertices=12" ) );
            CAssetIngestRequest request;
            request.sourceId = "disk";
            request.contentPath = base + "/source/Layout_Old";
            CAssetIngestResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Ingest( request, result ) );
            const CString packId = result.packId;
            const CString mainRoot = base + "/main";

            // make it a pack of the earlier layout: folders and row
            const CString oldRoot = "packs/disk/Forge Works/Layout Old [" + packId + "]";
            ASSERT_TRUE( vfs.CreateDir( mainRoot + "/" + oldRoot ) );
            ASSERT_TRUE( vfs.MoveDir( mainRoot + "/original/disk/Layout_Old", mainRoot + "/" + oldRoot + "/original" ) );
            ASSERT_TRUE( WriteText( mainRoot + "/" + oldRoot + "/converted/test/SM_Old_01.gltf", "converted" ) );
            CAssetCatalogue::TRowVector rows;
            CDataTableQuery query;
            query.WhereEquals( "pack_id", CVariant( packId ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetCatalogue().Select( "packs", query, rows ) && 1 == rows.size() );
            rows[ 0 ][ "root_path" ] = CVariant( oldRoot );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetCatalogue().Write( "packs", rows ) );

            // still readable: the paths follow the layout of the pack
            CAssetPackRecord pack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( packId, pack ) && oldRoot == pack.rootPath && !CPackStorageLayout( pack.rootPath ).IsAreaLayout() );
            TAssetRecordVector assets;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetAssets( packId, assets ) && 1 == assets.size() );
            CString vfsPath, message;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetAssetVfsPath( ( assets.empty() ? CString::Empty : assets[ 0 ].assetId ), vfsPath, message ) );
            ASSERT_TRUE( vfsPath == mainRoot + "/" + oldRoot + "/original/Meshes/SM_Old_01.fbx" && vfs.FileExists( vfsPath ) );
            CAssetVerificationReport verified;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.VerifyLocation( "main", verified ) );
            for ( size_t m=0; m<verified.missingFiles.size(); ++m )
                ASSERT_TRUE( verified.missingFiles[ m ].HasSubstr( "Layout Old" ) < 0 );

            // a dry run reports and moves nothing, also as a job
            CAssetLayoutMigrationReport report;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.MigrateStorageLayout( "main", true, report ) );
            ASSERT_TRUE( report.dryRun && 1 == report.packsInOldLayout && 1 == report.packsMigrated && 0 == report.packsRefused && 1 == report.actions.size() );
            ASSERT_TRUE( report.actions[ 0 ] == packId + ": " + oldRoot + " -> layout2:disk/Layout Old [" + packId + "]" );
            CAssetLayoutMigrationReport jobReport;
            ASSERT_TRUE( GetFutureResult( store.MigrateStorageLayoutAsync( "main", true ), jobReport ) && 1 == jobReport.packsMigrated && jobReport.dryRun );
            ASSERT_TRUE( vfs.FileExists( vfsPath ) && ASSETSTORE_STATUS_OK == store.GetPack( packId, pack ) && oldRoot == pack.rootPath );

            // a target folder that exists refuses the pack, nothing moves
            const CString folder = "disk/Layout Old [" + packId + "]";
            ASSERT_TRUE( vfs.CreateDir( mainRoot + "/original/" + folder ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.MigrateStorageLayout( "main", false, report ) );
            ASSERT_TRUE( 1 == report.packsInOldLayout && 0 == report.packsMigrated && 1 == report.packsRefused && 1 == report.problems.size() );
            ASSERT_TRUE( vfs.FileExists( vfsPath ) && ASSETSTORE_STATUS_OK == store.GetPack( packId, pack ) && oldRoot == pack.rootPath );
            ASSERT_TRUE( vfs.DeleteDir( mainRoot + "/original/" + folder, true ) );

            // another location is not touched
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.MigrateStorageLayout( "mirror", false, report ) );
            ASSERT_TRUE( 0 == report.packsInOldLayout && 0 == report.packsMigrated && vfs.FileExists( vfsPath ) );

            // applied: the folders move, the row changes, the paths resolve in the new places
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.MigrateStorageLayout( "main", false, report ) );
            ASSERT_TRUE( !report.dryRun && 1 == report.packsInOldLayout && 1 == report.packsMigrated && 0 == report.packsRefused && report.problems.empty() );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( packId, pack ) && "layout2:" + folder == pack.rootPath );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetAssetVfsPath( ( assets.empty() ? CString::Empty : assets[ 0 ].assetId ), vfsPath, message ) );
            ASSERT_TRUE( vfsPath == mainRoot + "/original/" + folder + "/Meshes/SM_Old_01.fbx" && vfs.FileExists( vfsPath ) );
            ASSERT_TRUE( vfs.FileExists( mainRoot + "/intermediate/" + folder + "/converted/test/SM_Old_01.gltf" ) );
            ASSERT_FALSE( vfs.DirExists( mainRoot + "/" + oldRoot ) );

            // again: nothing left to move; the cooked and intermediate files of the converted paths are found
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.MigrateStorageLayout( "", false, report ) );
            ASSERT_TRUE( 0 == report.packsInOldLayout && 0 == report.packsMigrated && 0 == report.packsRefused && report.actions.empty() );
            ASSERT_TRUE( vfs.FileExists( CPackStorageLayout( pack.rootPath ).Resolve( mainRoot, "converted/test/SM_Old_01.gltf" ) ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.RemovePack( packId, true ) );
            ASSERT_FALSE( vfs.DirExists( mainRoot + "/original/" + folder ) || vfs.DirExists( mainRoot + "/intermediate/" + folder ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": the HTML description of a store item is kept and searched as its text; a row that still holds HTML is read as text; the packs of an item are found by its id" ).C_String() )
        try
        {
            const CString staging = store.MakeDownloadStagingPath();
            ASSERT_TRUE( WriteText( staging + "/SM_Bundle_Pine_Description.fbx", "triangles=431;vertices=377;description=html" ) );
            CAssetIngestRequest request;
            request.sourceId = "example-store";
            request.contentPath = staging;
            request.item.itemId = "example-bundle";
            request.item.title = "Example Bundle";
            request.item.seller = "Example Seller";
            request.item.description = "<span>This bundle includes the following packs:</span><p></p><p><a target=\"_blank\" href=\"https://www.example.com/listings/1\" "
                                       "rel=\"noopener noreferrer nofollow\"><span><u>Realistic Pine Pack</u></span></a></p><script>track()</script>";
            CAssetIngestResult result;
            const TAssetStoreStatus ingested = store.Ingest( request, result );
            if ( ASSETSTORE_STATUS_OK != ingested )
                GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "TestPrimaryStore: the ingest of the item with an HTML description ended with " << AssetStoreStatusToString( ingested ) << ": " << result.message );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == ingested );
            const CString bundleId = result.packId;
            CAssetPackRecord pack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( bundleId, pack ) );
            ASSERT_TRUE( "This bundle includes the following packs:\n\nRealistic Pine Pack (https://www.example.com/listings/1)" == pack.description );

            // the words of the text are searched, the markup is not
            CAssetSearchQuery words;
            words.text = "pine";
            words.resultKind = "packs";
            CAssetSearchResultPage page;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( words, page ) && 1 == page.total && bundleId == page.hits[ 0 ].packId );
            words.text = "noopener";
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( words, page ) && 0 == page.total );

            // the packs that an item of a store became, by the id of the item
            CAssetSearchQuery ofItem;
            ofItem.resultKind = "packs";
            ofItem.sources.insert( "example-store" );
            ofItem.sourceItemIds.insert( "example-bundle" );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( ofItem, page ) && 1 == page.total && bundleId == page.hits[ 0 ].packId );
            ofItem.sourceItemIds.clear();
            ofItem.sourceItemIds.insert( "not-an-item" );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( ofItem, page ) && 0 == page.total );
            CDataNode queryNode;
            CDataNodeSerializableSettings serializerSettings;
            ASSERT_TRUE( ofItem.Serialize( queryNode, serializerSettings ) );
            CAssetSearchQuery queryBack;
            ASSERT_TRUE( queryBack.Deserialize( queryNode, serializerSettings ) && 1 == queryBack.sourceItemIds.size() && "not-an-item" == *queryBack.sourceItemIds.begin() );

            // a catalogue that kept the HTML of a store (written before the conversion existed): read as its text
            CDataTableQuery query;
            query.WhereEquals( "pack_id", CVariant( bundleId ) );
            CAssetCatalogue::TRowVector rows;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetCatalogue().Select( "packs", query, rows ) && 1 == rows.size() );
            if ( 1 == rows.size() )
            {
                rows[ 0 ][ "description" ] = CVariant( CString( "<p>Old &amp; <b>stored</b></p><p>row</p>" ) );
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetCatalogue().Write( "packs", rows ) );
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( bundleId, pack ) && "Old & stored\n\nrow" == pack.description );
            }

            // the pack goes again: the store is left with the packs the other cases count
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.RemovePack( bundleId, true ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    store.Close();
    analyzers.Unregister( "testmodel" );
    UnmountDirectory( fixture.label + "_main" );
    UnmountDirectory( fixture.label + "_source" );
    UnmountDirectory( fixture.label + "_mirror" );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      FUNCTIONS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformPrimaryStoreVfsTablesTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "PrimaryStoreVfsTables" );

    SStoreFixture fixture;
    fixture.label = "vfstables";
    fixture.base = "astest/vfstables";
    MountFreshDirectory( "vfstables_catalogue", "astest/vfstables/catalogue" );
    fixture.database.backendName = "vfstables";
    fixture.database.database = "astest/vfstables/catalogue/db";
    fixture.database.backendOptions.SetAllowMultipleValues( false );
    fixture.database.backendOptions.Set( "syncWrites", CVariant( "false" ) );
    RunPrimaryStoreSuite( fixture );
    UnmountDirectory( "vfstables_catalogue" );
}

/*-------------------------------------------------------------------------*/

void
PerformPrimaryStorePostgreSqlTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "PrimaryStorePostgreSql" );

    CString connectionString;
    if ( !GetPostgreSqlConnectionString( connectionString ) )
    {
        GUCEF_LOG( LOGLEVEL_IMPORTANT, "PrimaryStorePostgreSql: GUCEF_TEST_POSTGRESQL is not set, the PostgreSQL tests are skipped" );
        GUCEF_TESTFW_TESTCASE( "PostgreSQL: the primary store against a PostgreSQL test database" )
            GUCEF_TESTFW_SKIP( "GUCEF_TEST_POSTGRESQL is not set" );
        GUCEF_TESTFW_TESTCASE_END;
        return;
    }

    {
        CString reachReason;
        if ( !IsPostgreSqlReachable( connectionString, reachReason ) )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "PrimaryStorePostgreSql: the test database cannot be reached: " << reachReason );
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

    SStoreFixture fixture;
    fixture.label = "postgresql";
    fixture.base = "astest/postgresql";
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
    GUCEF_LOG( LOGLEVEL_NORMAL, "PrimaryStorePostgreSql: testing in schema \"" << schema << "\"" );
    RunPrimaryStoreSuite( fixture );

    // the schema of this run goes
    GUCEF_TESTFW_TESTCASE( "PostgreSQL: the schema of the run is dropped" )
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
