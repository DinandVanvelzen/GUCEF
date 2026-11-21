/*
 *  AssetStoreConverterHook_TestApp: Test application for the converter hook of the primary asset store (AssetStoreConverterHook)
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
#include "gucefASSETSTORE.h"
#include "gucefASSETSTORE_CPrimaryAssetStore.h"
#include "gucefASSETSTORE_CAssetPackIndexLoader.h"
#include "AssetConverterLib.h"
#include "AssetConverterLib_CPackIndexReader.h"
#include "AssetConverterLib_CPackIndexDocument.h"
#include "AssetStoreConverterHook.h"
#include "dbpluginPOSTGRESQL_CPostgreSqlClient.h"
#include "dbpluginPOSTGRESQL_CPostgreSqlConnection.h"
#include "dbpluginPOSTGRESQL_CPostgreSqlSql.h"
#include "dbpluginPOSTGRESQL_CPostgreSqlSettings.h"
#ifndef TESTHOOKSUPPORT_H
#include "TestHookSupport.h"
#define TESTHOOKSUPPORT_H
#endif /* TESTHOOKSUPPORT_H ? */
#include "TestHookPackIndex.h"

using namespace GUCEF;
using namespace GUCEF::CORE;
using namespace GUCEF::ASSETSTORE;
using namespace GUCEF::ASSETSTORECONVERTER;
using namespace HOOKTEST;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

struct SHookIndexFixture
{
    CString label;
    CString base;
    COMCORE::CDatabaseClientSettings database;
    AUTH::ICredentialStore* credentials;

    SHookIndexFixture( void ) : label(), base(), database(), credentials( GUCEF_NULL ) {}
};

CPrimaryAssetStoreSettings
MakeIndexSettings( const SHookIndexFixture& fixture )
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

TAssetStoreStatus
Ingest( CPrimaryAssetStore& store, const CString& folder, const CString& keep, CAssetIngestResult& result )
{
    CAssetIngestRequest request;
    request.sourceId = "disk";
    request.contentPath = folder;
    request.keep = keep;
    return store.Ingest( request, result );
}

UInt64
CountRows( CPrimaryAssetStore& store, const CString& table )
{
    CAssetCatalogue::TRowVector rows;
    store.GetCatalogue().SelectAll( table, rows );
    return rows.size();
}

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

/** The folder of the index of a pack: the folder of its manifest */
CString
FolderOfManifest( const CString& manifestPath )
{
    const Int32 slash = manifestPath.HasChar( '/', false );
    return slash < 0 ? CString::Empty : manifestPath.SubstrToIndex( (UInt32) slash, true );
}

void
RunHookPackIndexSuite( SHookIndexFixture& fixture )
{
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CString& base = fixture.base;
    MountFreshDirectory( fixture.label + "_hpi_main", base + "/main" );
    MountFreshDirectory( fixture.label + "_hpi_source", base + "/source" );

    bool sourcesWritten = WriteGltfPack( base + "/source/Idx_Both", "Idx Both", "{ \"name\": \"CC BY 4.0\" }", 2 );
    sourcesWritten = WriteGltfPack( base + "/source/Idx_Only", "Idx Only", "{ \"name\": \"CC BY 4.0\" }", 3 ) && sourcesWritten;
    sourcesWritten = WriteGltfPack( base + "/source/Idx_NoLicence", "Idx No Licence", CString::Empty, 1 ) && sourcesWritten;
    sourcesWritten = WriteGltfPack( base + "/source/Idx_Learning", "Idx Learning", "{ \"name\": \"CC BY 4.0\", \"restrictions\": [ \"noMachineLearning\" ] }", 1 ) && sourcesWritten;

    CPrimaryAssetStore store;
    store.SetSettings( MakeIndexSettings( fixture ) );
    store.SetCredentialStore( fixture.credentials );
    CPipelineConverterHook hook;
    store.SetConverterHook( &hook );
    CString bothId, onlyId, noLicenceId, learningId;
    CString bothHash, onlyHash;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": keep both: the document the converter wrote is loaded by the store at the ingest, and the converter reads what the store loaded" ).C_String() )
        try
        {
            ASSERT_TRUE( sourcesWritten && ASSETSTORE_STATUS_OK == store.Open() );
            CAssetIngestResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == Ingest( store, base + "/source/Idx_Both", "both", result ) );
            bothId = result.packId;
            CAssetPackIndexInfo info;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexInfo( bothId, true, info ) );
            ASSERT_TRUE( CAssetPackIndexInfo::StateLoaded == info.state && !info.stale );
            ASSERT_TRUE( info.hasManifest );
            ASSERT_TRUE( "AssetConverterLib" == info.manifest.FindChild( "producer" )->GetAttributeValueOrChildValueByName( "tool" ).AsString() );
            ASSERT_TRUE( info.documentPath == base + "/main/intermediate/Idx_Both/pack-index.json" && vfs.FileExists( info.documentPath ) );
            ASSERT_TRUE( 4 == info.resourceCount );     // the two models, the picture and the descriptor of the pack, which is no asset
            bothHash = info.documentHash;

            // the reader of the converter reads the same document: the same resources with the same groups
            ASSETCONVERTER::CPackIndexReader reader;
            ASSETCONVERTER::CPackIndexDocument document;
            ASSERT_TRUE( ASSETCONVERTER::CPackIndexReader::RESULT_OK == reader.Read( FolderOfManifest( info.documentPath ), document ) );
            ASSERT_TRUE( 4 == document.resources.size() && document.documentHash == info.documentHash && document.producer.indexKey == info.indexKey );
            TAssetPackResourceVector rows;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexResources( bothId, CString::Empty, CString::Empty, rows ) && 4 == rows.size() );
            TAssetRecordVector assets;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetAssets( bothId, assets ) && 3 == assets.size() );
            for ( size_t r=0; r<document.resources.size(); ++r )
            {
                const ASSETCONVERTER::CPackIndexResource& resource = document.resources[ r ];
                const CString id = CAssetPackIndexLoader::MakeResourceId( bothId, resource.path );
                CAssetPackResource row;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexResource( id, row ) );
                ASSERT_TRUE( row.path == resource.path && row.kind == resource.kind && row.groupId == resource.group && row.subGroup == resource.subGroup );
                ASSERT_TRUE( row.format == resource.format && row.sizeBytes == resource.size && row.name == resource.name );
                // the files of the pack that the store holds as assets have the ids of the store; the descriptor of the pack is a resource and no asset
                bool isAsset = false;
                for ( size_t a=0; a<assets.size(); ++a )
                    isAsset = isAsset || assets[ a ].assetId == id;
                ASSERT_TRUE( isAsset == ( "pack.json" != resource.path ) );
                ASSERT_TRUE( isAsset ? row.assetId == id : row.assetId.IsNULLOrEmpty() );
            }
            // the group summaries of the document are those of the rows
            TAssetPackGroupVector groups;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexGroups( bothId, groups ) );
            for ( size_t g=0; g<document.groups.size(); ++g )
            {
                bool found = false;
                for ( size_t r=0; r<groups.size(); ++r )
                    if ( groups[ r ].groupId == document.groups[ g ].id && groups[ r ].subGroup.IsNULLOrEmpty() )
                        found = groups[ r ].resourceCount == document.groups[ g ].count && groups[ r ].sizeBytes == document.groups[ g ].bytes;
                ASSERT_TRUE( found );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": relations: what the glTF importer resolved (the texture of each model) is in the document and in the relations of the catalogue; the companion files of a model hold its texture; a forced re-index and a derived index fill the relations again" ).C_String() )
        try
        {
            ASSERT_FALSE( bothId.IsNULLOrEmpty() );
            CAssetPackIndexInfo info;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexInfo( bothId, false, info ) );
            ASSETCONVERTER::CPackIndexReader reader;
            ASSETCONVERTER::CPackIndexDocument document;
            ASSERT_TRUE( ASSETCONVERTER::CPackIndexReader::RESULT_OK == reader.Read( FolderOfManifest( info.documentPath ), document ) );
            const ASSETCONVERTER::CPackIndexResource* model = document.FindResourceByPath( "Models/SM_Tri_01.gltf" );
            ASSERT_TRUE( GUCEF_NULL != model );
            bool texture = false;
            for ( size_t r=0; GUCEF_NULL != model && r<model->refs.size(); ++r )
                if ( "usesTexture" == model->refs[ r ].kind && "Models/tex.png" == model->refs[ r ].path && "tex.png" == model->refs[ r ].via )
                    texture = true;
            ASSERT_TRUE( texture );

            const CString modelId = CAssetPackIndexLoader::MakeResourceId( bothId, "Models/SM_Tri_01.gltf" );
            const CString textureId = CAssetPackIndexLoader::MakeResourceId( bothId, "Models/tex.png" );
            CDataTableQuery query;
            query.WhereEquals( "from_asset", CVariant( modelId ) ).WhereEquals( "kind", CVariant( CString( "usesTexture" ) ) );
            CAssetCatalogue::TRowVector rows;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetCatalogue().Select( "relations", query, rows ) && 1 == rows.size() && textureId == CAssetCatalogue::GetText( rows[ 0 ], "to_asset" ) );
            TAssetFileRecordVector companions;
            UInt32 followed = 0;
            bool truncated = false;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetCompanionFiles( modelId, companions, followed, truncated ) && 1 == companions.size() && 1 == followed );
            ASSERT_TRUE( companions[ 0 ].path.EndsWith( "/Models/tex.png" ) && vfs.FileExists( companions[ 0 ].path ) );

            // a catalogue without the relations (a pack ingested before them): a forced re-index loads them from the document again
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetCatalogue().DeleteWhereEquals( "relations", "pack_id", CVariant( bothId ) ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetCompanionFiles( modelId, companions, followed, truncated ) && companions.empty() && 0 == followed );
            CAssetPackIndexOptions reindex;
            reindex.force = true;
            reindex.reindex = true;
            CAssetPackIndexOutcome outcome;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.IndexPack( bothId, reindex, outcome ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetCompanionFiles( modelId, companions, followed, truncated ) && 1 == companions.size() );

            // derived from the catalogue (no file of the pack read) where the document is gone: the relations of the catalogue and the sidecars
            // of the intermediate are kept
            const CString documentPath = info.documentPath;
            ASSERT_TRUE( vfs.DeleteFile( documentPath, true ) );
            CAssetPackIndexOptions derive;
            derive.derive = true;
            derive.force = true;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.IndexPack( bothId, derive, outcome ) && outcome.derived );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexInfo( bothId, false, info ) );
            ASSETCONVERTER::CPackIndexDocument derived;
            ASSERT_TRUE( ASSETCONVERTER::CPackIndexReader::RESULT_OK == reader.Read( FolderOfManifest( info.documentPath ), derived ) && derived.producer.derivedFromCatalogue );
            model = derived.FindResourceByPath( "Models/SM_Tri_01.gltf" );
            ASSERT_TRUE( GUCEF_NULL != model );
            texture = false;
            for ( size_t r=0; GUCEF_NULL != model && r<model->refs.size(); ++r )
                if ( "usesTexture" == model->refs[ r ].kind && "Models/tex.png" == model->refs[ r ].path )
                    texture = true;
            ASSERT_TRUE( texture );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetCompanionFiles( modelId, companions, followed, truncated ) && 1 == companions.size() );

            // and a derived index of a catalogue without relations takes them from the sidecars
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetCatalogue().DeleteWhereEquals( "relations", "pack_id", CVariant( bothId ) ) );
            ASSERT_TRUE( vfs.DeleteFile( documentPath, true ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.IndexPack( bothId, derive, outcome ) && outcome.derived );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetCompanionFiles( modelId, companions, followed, truncated ) && 1 == companions.size() );

            // back to the document of the converter for the cases below
            ASSERT_TRUE( vfs.DeleteFile( documentPath, true ) );
            CAssetPackIndexOptions rebuild;
            rebuild.force = true;
            rebuild.reindex = true;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.IndexPack( bothId, rebuild, outcome ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a pack without a document is indexed by the converter: once, the structure level, then the analysis level, a redo that gives the same document" ).C_String() )
        try
        {
            CAssetIngestResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == Ingest( store, base + "/source/Idx_Only", "original", result ) );
            onlyId = result.packId;
            CAssetPackIndexInfo info;
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == store.GetPackIndexInfo( onlyId, false, info ) );
            ASSERT_FALSE( vfs.FileExists( base + "/main/intermediate/Idx_Only/pack-index.json" ) );

            CAssetPackIndexOutcome outcome;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.IndexPack( onlyId, CAssetPackIndexOptions(), outcome ) );
            ASSERT_TRUE( outcome.built && outcome.loaded && !outcome.reused && !outcome.derived && 5 == outcome.resourceCount );
            ASSERT_TRUE( vfs.FileExists( base + "/main/intermediate/Idx_Only/pack-index.json" ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexInfo( onlyId, true, info ) );
            ASSERT_TRUE( CAssetPackIndexInfo::StateLoaded == info.state && !info.stale && 5 == info.resourceCount && 1 == info.levels.size() && "structure" == info.levels[ 0 ] );
            onlyHash = info.documentHash;

            // work once: the document stands, the converter is not asked
            CAssetPackIndexOutcome again;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.IndexPack( onlyId, CAssetPackIndexOptions(), again ) );
            ASSERT_TRUE( again.reused && !again.built && !again.loaded );

            // the analysis level: the headers of the files, the statistics in the document
            CAssetPackIndexOptions analyse;
            analyse.analyse = true;
            analyse.reanalyse = true;
            CAssetPackIndexOutcome analysed;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.IndexPack( onlyId, analyse, analysed ) );
            ASSERT_TRUE( analysed.built && analysed.loaded );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexInfo( onlyId, true, info ) && 2 == info.levels.size() && info.documentHash != onlyHash );
            ASSETCONVERTER::CPackIndexReader reader;
            ASSETCONVERTER::CPackIndexDocument document;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexInfo( onlyId, false, info ) );
            ASSERT_TRUE( ASSETCONVERTER::CPackIndexReader::RESULT_OK == reader.Read( FolderOfManifest( info.documentPath ), document ) );
            const ASSETCONVERTER::CPackIndexResource* texture = document.FindResourceByPath( "Models/tex.png" );
            UInt64 width = 0;
            ASSERT_TRUE( GUCEF_NULL != texture && texture->GetStat( "width", width ) && 16 == width );
            TAssetPackResourceVector rows;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexResources( onlyId, "textures", CString::Empty, rows ) && 1 == rows.size() && rows[ 0 ].analysed && "analysed" == rows[ 0 ].status );
            const CString analysedHash = info.documentHash;

            // the redo of the index: the converter regroups and gives the same document (the same inputs, the same bytes)
            CAssetPackIndexOptions redo;
            redo.reindex = true;
            redo.analyse = true;
            CAssetPackIndexOutcome regrouped;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.IndexPack( onlyId, redo, regrouped ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexInfo( onlyId, false, info ) && analysedHash == info.documentHash );

            // the gate of the converter decides freshness: verify asks it, and a file that came into the pack makes the index stale
            CAssetPackIndexOptions verify;
            verify.verify = true;
            verify.analyse = true;
            CAssetPackIndexOutcome fresh;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.IndexPack( onlyId, verify, fresh ) && fresh.reused && !fresh.built && !fresh.loaded );
            ASSERT_TRUE( WriteVfsFile( base + "/main/original/Idx_Only/Models/Extra.txt", CString( "an extra file" ) ) );
            CAssetPackIndexOutcome trusted;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.IndexPack( onlyId, CAssetPackIndexOptions(), trusted ) && trusted.reused && !trusted.built );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexInfo( onlyId, false, info ) && 5 == info.resourceCount );
            CAssetPackIndexOutcome stale;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.IndexPack( onlyId, verify, stale ) && stale.built && stale.loaded && !stale.reused );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexInfo( onlyId, false, info ) && 6 == info.resourceCount && analysedHash != info.documentHash );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": the licence flags of the pack are in the document; a pack the converter refuses is indexed all the same (the index is metadata); derived from the catalogue it reads no file" ).C_String() )
        try
        {
            CAssetIngestResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == Ingest( store, base + "/source/Idx_Learning", "original", result ) );
            learningId = result.packId;
            CAssetPackIndexOutcome outcome;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.IndexPack( learningId, CAssetPackIndexOptions(), outcome ) && outcome.built );
            CAssetPackIndexInfo info;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexInfo( learningId, true, info ) && info.HasLicenceRestriction( CAssetLicence::NoMachineLearning ) );

            // no licence record: the conversion is refused, the index is made
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == Ingest( store, base + "/source/Idx_NoLicence", "both", result ) );
            noLicenceId = result.packId;
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == store.GetPackIndexInfo( noLicenceId, false, info ) );
            CAssetPackRecord pack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( noLicenceId, pack ) && ASSETPACK_STATE_CONVERSION_REFUSED == pack.state );
            CAssetPackIndexOptions derive;
            derive.derive = true;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.IndexPack( noLicenceId, derive, outcome ) && outcome.built && outcome.derived );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPackIndexInfo( noLicenceId, true, info ) );
            ASSERT_TRUE( CAssetPackIndexInfo::StateLoaded == info.state && !info.stale && 2 == info.resourceCount );
            const CDataNode* producer = info.manifest.FindChild( "producer" );
            ASSERT_TRUE( GUCEF_NULL != producer && producer->GetAttributeValueOrChildValueByName( "derivedFromCatalogue" ).AsBool( false, true ) );
            // the converter reads the derived document: its hash is the canonical one
            ASSETCONVERTER::CPackIndexReader reader;
            ASSETCONVERTER::CPackIndexDocument document;
            ASSERT_TRUE( ASSETCONVERTER::CPackIndexReader::RESULT_OK == reader.Read( FolderOfManifest( info.documentPath ), document ) );
            ASSERT_TRUE( document.producer.derivedFromCatalogue && 2 == document.resources.size() && document.IsComplete() );
            TAssetRecordVector assets;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetAssets( noLicenceId, assets ) && 2 == assets.size() );
            for ( size_t r=0; r<document.resources.size(); ++r )
            {
                bool matches = false;
                for ( size_t a=0; a<assets.size(); ++a )
                    matches = matches || ( assets[ a ].assetId == CAssetPackIndexLoader::MakeResourceId( noLicenceId, document.resources[ r ].path ) && assets[ a ].digest == document.resources[ r ].digest );
                ASSERT_TRUE( matches );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": the rows are rebuilt from the documents the converter wrote, and the group filter finds the models" ).C_String() )
        try
        {
            CString::StringVector packIds;
            packIds.push_back( bothId );
            packIds.push_back( onlyId );
            packIds.push_back( learningId );
            packIds.push_back( noLicenceId );
            CString::StringVector snapshots;
            CAssetPackIndexLoader loader( store.GetCatalogue(), GUCEF_NULL );
            for ( size_t p=0; p<packIds.size(); ++p )
            {
                snapshots.push_back( SnapshotOf( store, packIds[ p ] ) );
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == loader.Remove( packIds[ p ] ) );
            }
            ASSERT_TRUE( 0 == CountRows( store, "pack_indexes" ) && 0 == CountRows( store, "pack_resources" ) );
            CAssetPackIndexReport report;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.ReindexFromStorage( false, CAssetPackIndexOptions(), report ) );
            ASSERT_TRUE( 4 == report.packsSeen && 4 == report.packsLoaded && 0 == report.packsFailed && 0 == report.documentsWithoutPack );
            for ( size_t p=0; p<packIds.size(); ++p )
                ASSERT_TRUE( snapshots[ p ] == SnapshotOf( store, packIds[ p ] ) );

            // the models: the glTF files of the packs
            CAssetSearchQuery query;
            query.pageSize = 100;
            query.groups.insert( "static-models" );
            CAssetSearchResultPage page;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 7 == page.hits.size() );
            query.packIds.insert( onlyId );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 3 == page.hits.size() );
            query.groups.clear();
            query.groups.insert( "textures" );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Search( query, page ) && 1 == page.hits.size() && "tex" == page.hits[ 0 ].name );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    store.Close();
    UnmountDirectory( fixture.label + "_hpi_source" );
    UnmountDirectory( fixture.label + "_hpi_main" );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformHookPackIndexVfsTablesTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "ConverterHook.PackIndexVfsTables" );

    SHookIndexFixture fixture;
    fixture.label = "pi_vfstables";
    fixture.base = "hooktest/pi_vfstables";
    MountFreshDirectory( "pi_vfstables_catalogue", "hooktest/pi_vfstables/catalogue" );
    fixture.database.backendName = "vfstables";
    fixture.database.database = "hooktest/pi_vfstables/catalogue/db";
    fixture.database.backendOptions.SetAllowMultipleValues( false );
    fixture.database.backendOptions.Set( "syncWrites", CVariant( "false" ) );
    RunHookPackIndexSuite( fixture );
    UnmountDirectory( "pi_vfstables_catalogue" );
}

/*-------------------------------------------------------------------------*/

void
PerformHookPackIndexPostgreSqlTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "ConverterHook.PackIndexPostgreSql" );

    CString connectionString;
    if ( !GetPostgreSqlConnectionString( connectionString ) )
    {
        GUCEF_LOG( LOGLEVEL_IMPORTANT, "ConverterHook.PackIndexPostgreSql: GUCEF_TEST_POSTGRESQL is not set, the PostgreSQL tests are skipped" );
        GUCEF_TESTFW_TESTCASE( "PostgreSQL: the pack index through the converter hook against a PostgreSQL test database" )
            GUCEF_TESTFW_SKIP( "GUCEF_TEST_POSTGRESQL is not set" );
        GUCEF_TESTFW_TESTCASE_END;
        return;
    }

    SHookIndexFixture fixture;
    fixture.label = "pi_postgresql";
    fixture.base = "hooktest/pi_postgresql";
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
    fixture.database.backendOptions.Set( "applicationName", CVariant( "AssetStoreConverterHook_TestApp" ) );
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
    GUCEF_LOG( LOGLEVEL_NORMAL, "ConverterHook.PackIndexPostgreSql: testing in schema \"" << schema << "\"" );
    RunHookPackIndexSuite( fixture );

    GUCEF_TESTFW_TESTCASE( "PostgreSQL: the schema of the pack index run through the converter hook is dropped" )
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
