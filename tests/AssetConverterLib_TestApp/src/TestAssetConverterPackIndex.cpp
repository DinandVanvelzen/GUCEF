/*
 *  AssetConverterLib_TestApp: Test application for the asset converter library (AssetConverterLib)
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

#include <algorithm>

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#include "TestAssetConverterSupport.h"
#include "TestAssetConverterPackIndex.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETCONVERTER;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

const CAssetKindRegistry&
Kinds( void )
{
    return CAssetConverterGlobal::Instance()->GetPluginRegistry().GetKinds();
}

/** A listing of paths without a file system behind it: the sizes come from the builder */
void
AddFiles( CAssetPackListing& listing, const char* const* paths )
{
    for ( ; GUCEF_NULL != *paths; ++paths )
        listing.AddFile( *paths );
}

CPackIndexBuildOptions
SyntheticOptions( void )
{
    CPackIndexBuildOptions options;
    options.useFileSystemSizes = false;
    options.packName = "synthetic";
    options.plugins[ "recogniser:test" ] = 1;
    return options;
}

/** The group and sub-group the engine gives a path of a synthetic pack */
bool
Place( const CPackIndexDocument& doc, const CORE::CString& path, CORE::CString& group, CORE::CString& subGroup )
{
    const CPackIndexResource* resource = doc.FindResourceByPath( path );
    if ( GUCEF_NULL == resource )
        return false;
    group = resource->group;
    subGroup = resource->subGroup;
    return true;
}

bool
Build( const char* const* paths, const CAssetPackProfile& profile, CPackIndexDocument& doc, const CPackIndexBuildOptions& options = SyntheticOptions() )
{
    CAssetPackListing listing;
    AddFiles( listing, paths );
    CPackIndexBuilder builder( Kinds() );
    return builder.BuildStructure( listing, profile, options, doc, GUCEF_NULL );
}

CPackIndexResource
MakeResource( const CORE::CString& path, UInt64 size )
{
    CPackIndexResource resource;
    resource.path = path;
    resource.size = size;
    resource.id = CPackIndexResource::MakeId( "key", path );
    CPackGroupRules rules( Kinds() );
    rules.Assign( resource );
    return resource;
}

/** A document with some records of every member kind the model has */
CPackIndexDocument
MakeRichDocument( void )
{
    CPackIndexDocument doc;
    doc.identity.packKey = "aa11";
    doc.identity.treeHash = "aa11";
    doc.identity.fileCount = 3;
    doc.identity.totalBytes = 600;
    doc.identity.name = "rich";
    doc.producer.tool = "AssetConverterLib";
    doc.producer.indexKey = "bb22";
    doc.producer.settingsHash = "cc33";
    doc.producer.plugins[ "recogniser:test" ] = 2;
    doc.licence.recordId = "rec";
    doc.licence.restrictions.push_back( "noMachineLearning" );
    doc.licence.restrictions.push_back( "attributionRequired" );
    doc.profile.packType = "unity-package";
    doc.profile.groupingHints.push_back( "Assets/Models" );
    CPackGroupRule rule;
    rule.glob = "*/Special/*";
    rule.subGroup = "special";
    rule.priority = 5;
    doc.profile.groupRules.push_back( rule );

    CPackIndexResource model = MakeResource( "Assets/Models/SM_Bld_Shop_01.fbx", 300 );
    model.SetStat( "triangles", 1200 );
    model.SetStat( "bones", 0 );
    model.hasBounds = true;
    model.boundsMin[ 0 ] = -1.5f; model.boundsMin[ 1 ] = 0.0f; model.boundsMin[ 2 ] = -2.0f;
    model.boundsMax[ 0 ] = 1.5f; model.boundsMax[ 1 ] = 3.25f; model.boundsMax[ 2 ] = 2.0f;
    CPackIndexReference ref;
    ref.kind = "usesTexture";
    ref.to = "asZZ";
    ref.via = "guid:1234";
    model.refs.push_back( ref );
    ref.kind = "usesMaterial";
    ref.to = "asAA";
    model.refs.push_back( ref );
    model.outputs.intermediate.path = "Assets/Models/SM_Bld_Shop_01.gltf";
    model.outputs.intermediate.size = 99;
    model.outputs.cacheKey = "dd44";
    model.outputs.sidecar.path = "Assets/Models/SM_Bld_Shop_01.conversion.json";
    CPackIndexOutputFile cooked;
    cooked.profile = "desktop";
    cooked.kind = "mesh";
    cooked.path = "out/shop.mesh";
    model.outputs.cooked.push_back( cooked );
    CPackIndexThumbnail thumbnail;
    thumbnail.size = 128;
    thumbnail.digest = "ee55";
    thumbnail.rendererId = "r";
    thumbnail.rendererVersion = 3;
    thumbnail.automatic = true;
    model.thumbnails.push_back( thumbnail );
    model.tags.push_back( "shop" );
    model.tags.push_back( "bld" );
    model.metadataFiles.push_back( "Assets/Models/SM_Bld_Shop_01.fbx.meta" );
    model.status.state = CPackIndexStatus::Converted;
    CPackIndexStepStatus step( CPackIndexStepStatus::Fresh );
    step.key = "ff66";
    model.status.SetStep( CPackIndexStatus::StepImport, step );
    doc.resources.push_back( model );
    CPackIndexResource texture = MakeResource( "Assets/Textures/T_Bld_Shop_01.png", 200 );
    texture.SetStat( "width", 512 );
    texture.SetStat( "height", 256 );
    doc.resources.push_back( texture );
    doc.resources.push_back( MakeResource( "Assets/Audio/door.wav", 100 ) );
    doc.AddLevel( CPackIndexDocument::LevelStructure );
    doc.AddLevel( CPackIndexDocument::LevelAnalysis );
    doc.state = CPackIndexDocument::StateComplete;
    doc.UpdateSummaries();
    doc.Normalise();
    return doc;
}

bool
VfsExists( const CORE::CString& vfsPath )
{
    return VFS::CVfsGlobal::Instance()->GetVfs().FileExists( vfsPath );
}

/** The index folder of a test case: a new folder below the work folder; its VFS path */
CORE::CString
IndexFolder( const CORE::CString& name )
{
    MakeWorkFolder( name );
    return "work/" + name;
}

CORE::CString
TextOfManifest( const CORE::CString& folder )
{
    return ReadVfsText( CPackIndexLocator::GetManifestPath( folder ) );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAssetConverterPackIndexTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AssetConverterLib.PackIndex" );

    GUCEF_TESTFW_TESTCASE( "Document: a record with every member round trips through the canonical writer and reader; the hash is that of the stored bytes' tree and the second write is byte identical" )
    try
    {
        CPackIndexDocument doc = MakeRichDocument();
        const CORE::CString folder = IndexFolder( "pidx_round" );
        CPackIndexWriter writer;
        ASSERT_TRUE( writer.Write( doc, folder ) );
        ASSERT_TRUE( !doc.documentHash.IsNULLOrEmpty() && doc.shards.empty() );
        const CORE::CString first = TextOfManifest( folder );

        CPackIndexReader reader;
        CPackIndexDocument back;
        ASSERT_TRUE( CPackIndexReader::RESULT_OK == reader.Read( folder, back ) );
        ASSERT_TRUE( back.documentHash == doc.documentHash && back.ComputeDocumentHash() == doc.documentHash );
        ASSERT_TRUE( 3 == back.resources.size() && back.identity == doc.identity && back.producer == doc.producer && back.profile == doc.profile );
        ASSERT_TRUE( back.licence == doc.licence && back.counts == doc.counts && back.groups == doc.groups );
        const CPackIndexResource* model = back.FindResourceByPath( "Assets/Models/SM_Bld_Shop_01.fbx" );
        ASSERT_TRUE( GUCEF_NULL != model );
        UInt64 triangles = 0;
        ASSERT_TRUE( model->GetStat( "triangles", triangles ) && 1200 == triangles && model->hasBounds );
        ASSERT_TRUE( Near( model->boundsMax[ 1 ], 3.25f ) && Near( model->boundsMin[ 0 ], -1.5f ) );
        ASSERT_TRUE( 2 == model->refs.size() && "usesMaterial" == model->refs[ 0 ].kind && "usesTexture" == model->refs[ 1 ].kind );
        ASSERT_TRUE( model->outputs == doc.resources[ 0 ].outputs || model->outputs == doc.FindResourceByPath( model->path )->outputs );
        ASSERT_TRUE( 1 == model->thumbnails.size() && model->thumbnails[ 0 ].automatic && 3 == model->thumbnails[ 0 ].rendererVersion );
        ASSERT_TRUE( model->status == doc.FindResourceByPath( model->path )->status );
        ASSERT_TRUE( 1 == model->metadataFiles.size() );

        // written again from what was read: the same bytes
        CPackIndexWriter again;
        ASSERT_TRUE( again.Write( back, folder ) );
        ASSERT_TRUE( first == TextOfManifest( folder ) );
        // the produced block is no part of the hash
        back.producedUtc = "2030-01-01T00:00:00.000Z";
        ASSERT_TRUE( back.ComputeDocumentHash() == doc.documentHash );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Canonical: the same records in another order, and the same document written twice, give the same bytes and the same hash" )
    try
    {
        CPackIndexDocument a = MakeRichDocument();
        CPackIndexDocument b = MakeRichDocument();
        std::reverse( b.resources.begin(), b.resources.end() );
        std::reverse( b.levels.begin(), b.levels.end() );
        std::reverse( b.licence.restrictions.begin(), b.licence.restrictions.end() );
        const CORE::CString folderA = IndexFolder( "pidx_canon_a" );
        const CORE::CString folderB = IndexFolder( "pidx_canon_b" );
        CPackIndexWriter writerA;
        CPackIndexWriter writerB;
        ASSERT_TRUE( writerA.Write( a, folderA ) && writerB.Write( b, folderB ) );
        ASSERT_TRUE( a.documentHash == b.documentHash );
        ASSERT_TRUE( TextOfManifest( folderA ) == TextOfManifest( folderB ) );
        // the group order is the fixed order of the vocabulary, not the alphabet
        ASSERT_TRUE( 3 == a.groups.size() && "static-models" == a.groups[ 0 ].id && "textures" == a.groups[ 1 ].id && "audio" == a.groups[ 2 ].id );
        ASSERT_TRUE( a.resources[ 0 ].group == "static-models" && a.resources[ 2 ].group == "audio" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Sharding: 5,000 resources go to shards cut at group borders and at 2,000; one reader gives them all back; one change rewrites one shard" )
    try
    {
        CAssetPackListing listing;
        CPackIndexBuilder builder( Kinds() );
        for ( UInt32 i=0; i<2500; ++i )
        {
            listing.AddFile( "Assets/Props/SM_Prop_" + CORE::UInt32ToString( 100000 + i ) + ".fbx" );
            listing.AddFile( "Assets/Textures/T_" + CORE::UInt32ToString( 100000 + i ) + ".png" );
            builder.SetFileSize( "Assets/Props/SM_Prop_" + CORE::UInt32ToString( 100000 + i ) + ".fbx", 1000 + i );
            builder.SetFileSize( "Assets/Textures/T_" + CORE::UInt32ToString( 100000 + i ) + ".png", 2000 + i );
        }
        CPackIndexDocument doc;
        ASSERT_TRUE( builder.BuildStructure( listing, CAssetPackProfile(), SyntheticOptions(), doc, GUCEF_NULL ) );
        ASSERT_TRUE( 5000 == doc.resources.size() && 5000 == builder.GetGroupedCount() );
        const CORE::CString folder = IndexFolder( "pidx_shards" );
        CPackIndexWriter writer;
        ASSERT_TRUE( writer.Write( doc, folder ) );
        ASSERT_TRUE( 4 == doc.shards.size() && 4 == writer.GetShardsWritten() && 0 == writer.GetShardsReused() );
        ASSERT_TRUE( 2000 == doc.shards[ 0 ].count && 500 == doc.shards[ 1 ].count && 2000 == doc.shards[ 2 ].count && 500 == doc.shards[ 3 ].count );
        ASSERT_TRUE( "static-models" == doc.shards[ 0 ].group && "static-models" == doc.shards[ 1 ].group && "textures" == doc.shards[ 2 ].group );
        ASSERT_TRUE( doc.shards[ 0 ].lastPath < doc.shards[ 1 ].firstPath );
        const CORE::CString manifest = TextOfManifest( folder );
        ASSERT_TRUE( manifest.HasSubstr( "SM_Prop_101000" ) < 0 );   // the records are not in the manifest

        CPackIndexReader reader;
        CPackIndexDocument manifestOnly;
        ASSERT_TRUE( CPackIndexReader::RESULT_OK == reader.ReadManifest( folder, manifestOnly ) && manifestOnly.resources.empty() && 4 == manifestOnly.shards.size() );
        ASSERT_TRUE( 2 == manifestOnly.groups.size() && 2500 == manifestOnly.groups[ 0 ].count );
        CPackIndexResource::TResourceVector oneShard;
        ASSERT_TRUE( CPackIndexReader::RESULT_OK == reader.ReadShard( folder, manifestOnly.shards[ 1 ], oneShard ) && 500 == oneShard.size() );
        CPackIndexDocument all;
        ASSERT_TRUE( CPackIndexReader::RESULT_OK == reader.Read( folder, all ) && 5000 == all.resources.size() && all.documentHash == doc.documentHash );

        // a changed size in one record: one shard is written, three are kept
        builder.SetFileSize( "Assets/Textures/T_102499.png", 99 );
        CPackIndexDocument changed;
        ASSERT_TRUE( builder.BuildStructure( listing, CAssetPackProfile(), SyntheticOptions(), changed, &doc ) );
        CPackIndexWriter second;
        ASSERT_TRUE( second.Write( changed, folder ) );
        ASSERT_TRUE( 1 == second.GetShardsWritten() && 3 == second.GetShardsReused() && changed.documentHash != doc.documentHash );
        ASSERT_TRUE( 4 == changed.shards.size() && changed.shards[ 0 ].file == doc.shards[ 0 ].file && changed.shards[ 3 ].file != doc.shards[ 3 ].file );
        // the shard of the old manifest is gone, the new index reads
        CPackIndexDocument reread;
        ASSERT_TRUE( CPackIndexReader::RESULT_OK == reader.Read( folder, reread ) && 5000 == reread.resources.size() );
        VFS::CVFS::TStringVector files;
        VFS::CVfsGlobal::Instance()->GetVfs().GetFileList( files, folder + "/pack-index.d", false, false, CORE::CString::Empty, 100 );
        ASSERT_TRUE( 4 == files.size() );

        // a damaged shard is corrupt, not silently short
        ASSERT_TRUE( WriteWorkFile( "pidx_shards/pack-index.d/" + changed.shards[ 0 ].file, "{}" ) );
        CPackIndexDocument damaged;
        ASSERT_TRUE( CPackIndexReader::RESULT_CORRUPT == reader.Read( folder, damaged ) );

        // over the ceiling the pack is refused with a clear message
        CPackIndexDocument huge;
        huge.resources.resize( CPackIndexDocument::MaxResources + 1 );
        CPackIndexWriter refuser;
        ASSERT_FALSE( refuser.Write( huge, IndexFolder( "pidx_huge" ) ) );
        ASSERT_TRUE( refuser.GetError().HasSubstr( "limit" ) >= 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Version: a newer version, another format and a damaged document are refused with a message; unknown members of a readable version are ignored; a damaged manifest is renamed once" )
    try
    {
        CPackIndexDocument doc = MakeRichDocument();
        const CORE::CString folder = IndexFolder( "pidx_version" );
        CPackIndexWriter writer;
        ASSERT_TRUE( writer.Write( doc, folder ) );
        CPackIndexReader reader;
        CPackIndexDocument back;

        // a member from a later minor build: ignored, and the document is still good when its hash was made with it
        CORE::CDataNode node;
        ASSERT_TRUE( CCanonicalJson::Load( CPackIndexLocator::GetManifestPath( folder ), node ) );
        node.SetAttribute( "futureMember", CORE::CString( "from a later build" ) );
        node.DelAttribute( "documentHash" );
        node.DelChild( "produced" );
        node.SetAttribute( "documentHash", CCanonicalJson::Hash( node ) );
        ASSERT_TRUE( CCanonicalJson::Store( CPackIndexLocator::GetManifestPath( folder ), node ) );
        ASSERT_TRUE( CPackIndexReader::RESULT_OK == reader.ReadManifest( folder, back ) && 3 == back.resources.size() );

        // version 2: refused, and the message says what to do
        node.SetAttribute( "version", (UInt32) 2 );
        ASSERT_TRUE( CCanonicalJson::Store( CPackIndexLocator::GetManifestPath( folder ), node ) );
        ASSERT_TRUE( CPackIndexReader::RESULT_VERSION_TOO_NEW == reader.ReadManifest( folder, back ) );
        ASSERT_TRUE( reader.GetMessage().HasSubstr( "version 2" ) >= 0 && reader.GetMessage().HasSubstr( "newer build" ) >= 0 );

        // another format
        node.SetAttribute( "format", CORE::CString( "something.else" ) );
        node.SetAttribute( "version", (UInt32) 1 );
        ASSERT_TRUE( CCanonicalJson::Store( CPackIndexLocator::GetManifestPath( folder ), node ) );
        ASSERT_TRUE( CPackIndexReader::RESULT_NOT_A_PACK_INDEX == reader.ReadManifest( folder, back ) );

        // a changed byte: the hash says so
        ASSERT_TRUE( writer.Write( doc, folder ) );
        CORE::CString text = TextOfManifest( folder );
        text = text.ReplaceSubstr( "\"bytes\": 600", "\"bytes\": 601" );
        ASSERT_TRUE( WriteWorkFile( "pidx_version/pack-index.json", text ) );
        ASSERT_TRUE( CPackIndexReader::RESULT_CORRUPT == reader.ReadManifest( folder, back ) );
        ASSERT_TRUE( reader.MarkBad( folder ) );
        ASSERT_TRUE( VfsExists( folder + "/pack-index.json.bad" ) && CPackIndexReader::RESULT_MISSING == reader.ReadManifest( folder, back ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Groups: every kind lands in its function group; the stages decide in the order P, K, F, T; noise is hidden and counted, metadata is attached to its owner" )
    try
    {
        static const char* const files[] =
        {
            "Assets/Packs/Models/SM_Bld_Shop_01.fbx", "Assets/Packs/Models/SM_Prop_Table_01.fbx", "Assets/Packs/Models/SM_Veh_Truck_01.fbx",
            "Assets/Packs/Models/Misc/SM_Sign_Open.fbx",
            "Assets/Packs/Textures/T_Shop_01.png", "Assets/Packs/Textures/T_Shop_02.dds", "Assets/Packs/Materials/M_Shop.material",
            "Assets/Packs/Scenes/Demo.scene", "Assets/Packs/Prefabs/Shop.prefab", "Assets/Packs/Animations/Open.anim", "Assets/Packs/Audio/door.wav",
            "Assets/Packs/Shaders/water.shader", "Assets/Packs/Fonts/sign.ttf", "Assets/Packs/Particles/Smoke.particle", "Assets/Packs/Models/Rig.skeleton",
            "Assets/Packs/readme.txt", "Assets/Packs/Data/table.json", "Assets/Packs/Models/old.zip", "Assets/Packs/Models/mystery.zzz",
            "Assets/Packs/Models/SM_Bld_Shop_01.fbx.meta", "Assets/Packs/Models/orphan.fbx.meta",
            "Assets/Packs/.DS_Store", "Assets/Packs/Models/Thumbs.db", "Assets/Packs/Models/.hidden/x.fbx", "Assets/Packs/Tmp/leftover.bak",
            GUCEF_NULL
        };
        CAssetPackProfile profile;
        profile.noisePatterns.push_back( "*.bak" );
        CPackIndexDocument doc;
        ASSERT_TRUE( Build( files, profile, doc ) );
        struct SExpect { const char* path; const char* group; };
        static const SExpect expected[] =
        {
            { "Assets/Packs/Models/SM_Bld_Shop_01.fbx", "static-models" }, { "Assets/Packs/Textures/T_Shop_01.png", "textures" }, { "Assets/Packs/Textures/T_Shop_02.dds", "textures" },
            { "Assets/Packs/Materials/M_Shop.material", "materials" }, { "Assets/Packs/Scenes/Demo.scene", "scenes" }, { "Assets/Packs/Prefabs/Shop.prefab", "prefabs" },
            { "Assets/Packs/Animations/Open.anim", "animations" }, { "Assets/Packs/Audio/door.wav", "audio" }, { "Assets/Packs/Shaders/water.shader", "shaders" },
            { "Assets/Packs/Fonts/sign.ttf", "fonts" }, { "Assets/Packs/Particles/Smoke.particle", "visual-effects" }, { "Assets/Packs/Models/Rig.skeleton", "characters" },
            { "Assets/Packs/readme.txt", "other" }, { "Assets/Packs/Data/table.json", "other" }, { "Assets/Packs/Models/old.zip", "packages" }, { "Assets/Packs/Models/mystery.zzz", "other" }
        };
        for ( size_t i=0; i<sizeof( expected ) / sizeof( expected[ 0 ] ); ++i )
        {
            CORE::CString group, sub;
            ASSERT_TRUE( Place( doc, expected[ i ].path, group, sub ) );
            ASSERT_TRUE( group == expected[ i ].group );
        }
        // the noise is not listed and counted; the metadata is not listed, counted and attached
        ASSERT_TRUE( 19 == doc.resources.size() && 4 == doc.counts.noise && 2 == doc.counts.metadata );
        ASSERT_TRUE( GUCEF_NULL == doc.FindResourceByPath( "Assets/Packs/.DS_Store" ) && GUCEF_NULL == doc.FindResourceByPath( "Assets/Packs/Tmp/leftover.bak" ) );
        ASSERT_TRUE( GUCEF_NULL == doc.FindResourceByPath( "Assets/Packs/Models/SM_Bld_Shop_01.fbx.meta" ) );
        ASSERT_TRUE( 1 == doc.FindResourceByPath( "Assets/Packs/Models/SM_Bld_Shop_01.fbx" )->metadataFiles.size() );
        // the pack's own folder (Packs) is no sub-group; the function folders are structural; the name tokens give the sub-groups
        CORE::CString group, sub;
        ASSERT_TRUE( Place( doc, "Assets/Packs/Models/SM_Bld_Shop_01.fbx", group, sub ) && "buildings" == sub );
        ASSERT_TRUE( Place( doc, "Assets/Packs/Models/SM_Prop_Table_01.fbx", group, sub ) && "props" == sub );
        ASSERT_TRUE( Place( doc, "Assets/Packs/Models/SM_Veh_Truck_01.fbx", group, sub ) && "vehicles" == sub );
        // a generic folder ("Misc") is replaced by the token of the name
        ASSERT_TRUE( Place( doc, "Assets/Packs/Models/Misc/SM_Sign_Open.fbx", group, sub ) && "signs" == sub );
        ASSERT_TRUE( Place( doc, "Assets/Packs/Textures/T_Shop_01.png", group, sub ) && sub.IsNULLOrEmpty() );
        ASSERT_TRUE( "static-model" == doc.FindResourceByPath( "Assets/Packs/Models/SM_Bld_Shop_01.fbx" )->storeKind && "Bld shop 01" == doc.FindResourceByPath( "Assets/Packs/Models/SM_Bld_Shop_01.fbx" )->title );
        ASSERT_TRUE( "kind+token" == doc.FindResourceByPath( "Assets/Packs/Models/SM_Bld_Shop_01.fbx" )->groupRule );
        // the counts of the manifest are those of the records
        ASSERT_TRUE( doc.counts.resources == doc.resources.size() && 4 == doc.FindGroup( "static-models" )->count );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Groups: the prefix of a name gives the function of a compiled resource or a package asset (data, package, other kinds); a package asset of no known class is a package asset, not Other; editor leftovers and sidecars are not listed" )
    try
    {
        // a Godot-like project: compiled resources by prefix, the editor's cache files as noise of the profile, uid files as sidecars
        static const char* const godot[] =
        {
            "res/SM_Wall_01.res", "res/SM_Wall_01.res.import", "res/SK_Hero.res", "res/T_Wall_D.res", "res/M_Stone.tres", "res/MI_Stone_Wet.tres", "res/A_Walk.res",
            "res/PS_Smoke.res", "res/Plain.res", "res/script.gd", "res/script.gd.uid", "res/Wall.unwrap_cache", "res/Wall.depren", "res/logo.svg", GUCEF_NULL
        };
        CAssetPackProfile godotProfile;
        godotProfile.packType = "godot-project";
        godotProfile.noisePatterns.push_back( "*.unwrap_cache" );
        godotProfile.noisePatterns.push_back( "*.depren" );
        CPackIndexDocument doc;
        ASSERT_TRUE( Build( godot, godotProfile, doc ) );
        CORE::CString group, sub;
        ASSERT_TRUE( Place( doc, "res/SM_Wall_01.res", group, sub ) && "static-models" == group && "prefix" == doc.FindResourceByPath( "res/SM_Wall_01.res" )->groupRule.SubstrFromRange( 0, 6 ) );
        ASSERT_TRUE( Place( doc, "res/SK_Hero.res", group, sub ) && "characters" == group );
        ASSERT_TRUE( Place( doc, "res/T_Wall_D.res", group, sub ) && "textures" == group );
        ASSERT_TRUE( Place( doc, "res/M_Stone.tres", group, sub ) && "materials" == group );
        ASSERT_TRUE( Place( doc, "res/MI_Stone_Wet.tres", group, sub ) && "materials" == group );
        ASSERT_TRUE( Place( doc, "res/A_Walk.res", group, sub ) && "animations" == group );
        ASSERT_TRUE( Place( doc, "res/PS_Smoke.res", group, sub ) && "visual-effects" == group );
        ASSERT_TRUE( Place( doc, "res/Plain.res", group, sub ) && "other" == group );
        ASSERT_TRUE( Place( doc, "res/logo.svg", group, sub ) && "other" == group );
        ASSERT_TRUE( 2 == doc.counts.noise );
        // an Unreal-like package: prefixes by convention, the rest are package assets
        static const char* const unreal[] =
        {
            "Content/Props/SM_Barrel.uasset", "Content/Chars/SKM_Hero.uasset", "Content/Mats/MI_Rock.uasset", "Content/Tex/T_Rock_D.uasset", "Content/Anim/A_Run.uasset",
            "Content/Other/Rock.uasset", "Content/Other/Rock.uexp", "Content/Maps/Town.umap", "Content/Pack.mhpkg", GUCEF_NULL
        };
        CAssetPackProfile unrealProfile;
        CPackIndexDocument unrealDoc;
        ASSERT_TRUE( Build( unreal, unrealProfile, unrealDoc ) );
        ASSERT_TRUE( Place( unrealDoc, "Content/Props/SM_Barrel.uasset", group, sub ) && "static-models" == group );
        ASSERT_TRUE( Place( unrealDoc, "Content/Chars/SKM_Hero.uasset", group, sub ) && "characters" == group );
        ASSERT_TRUE( Place( unrealDoc, "Content/Mats/MI_Rock.uasset", group, sub ) && "materials" == group );
        ASSERT_TRUE( Place( unrealDoc, "Content/Tex/T_Rock_D.uasset", group, sub ) && "textures" == group );
        ASSERT_TRUE( Place( unrealDoc, "Content/Anim/A_Run.uasset", group, sub ) && "animations" == group );
        ASSERT_TRUE( Place( unrealDoc, "Content/Other/Rock.uasset", group, sub ) && "packages" == group );
        ASSERT_TRUE( Place( unrealDoc, "Content/Pack.mhpkg", group, sub ) && "packages" == group );
        ASSERT_TRUE( Place( unrealDoc, "Content/Maps/Town.umap", group, sub ) && "scenes" == group );
        ASSERT_TRUE( "Package assets" == CPackGroupVocabulary::GetGroupName( "packages" ) && CPackGroupVocabulary::GetGroupOrder( "packages" ) > CPackGroupVocabulary::GetGroupOrder( "fonts" ) && CPackGroupVocabulary::GetGroupOrder( "packages" ) < CPackGroupVocabulary::GetGroupOrder( "other" ) );
        // a profile rule still goes before the prefix
        CPackIndexBuildOptions options = SyntheticOptions();
        CPackGroupRule rule;
        rule.glob = "*SM_Barrel*";
        rule.group = "scenes";
        options.profileRules.push_back( rule );
        CPackIndexDocument ruled;
        ASSERT_TRUE( Build( unreal, unrealProfile, ruled, options ) );
        ASSERT_TRUE( Place( ruled, "Content/Props/SM_Barrel.uasset", group, sub ) && "scenes" == group );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Group rules: profile rules go before the kind and the folders, by priority; folder rules give the sub-group; grouping hints are roots; refinement makes a skinned model a character" )
    try
    {
        static const char* const files[] =
        {
            "Content/Gadgets/Special/Crate.fbx", "Content/Gadgets/Plain/Crate2.fbx", "Content/Gadgets/Plain/Walker.fbx", "Content/Gadgets/Plain/Hero_Walk.anim",
            "Content/Gadgets/Deep/Rooms/Left/Chair.fbx", "Content/Gadgets/Deep/Rooms/Left/Chair.png", "Content/Gadgets/Extra.fbx", GUCEF_NULL
        };
        CAssetPackProfile profile;
        profile.groupingHints.push_back( "Content/Gadgets" );
        CPackIndexBuildOptions options = SyntheticOptions();
        CPackGroupRule low;
        low.folder = "Content/Gadgets/Special";
        low.group = "scenes";
        low.subGroup = "low";
        low.priority = 1;
        CPackGroupRule high;
        high.glob = "*crate.fbx";
        high.subGroup = "high";
        high.priority = 9;
        CPackGroupRule onlyGroup;
        onlyGroup.kind = "animation";
        onlyGroup.group = "characters";
        options.profileRules.push_back( low );
        options.profileRules.push_back( high );
        options.profileRules.push_back( onlyGroup );
        CPackIndexDocument doc;
        ASSERT_TRUE( Build( files, profile, doc, options ) );
        CORE::CString group, sub;
        // both rules match: the higher priority decides, the group is left to the kind registry
        ASSERT_TRUE( Place( doc, "Content/Gadgets/Special/Crate.fbx", group, sub ) && "static-models" == group && "high" == sub );
        ASSERT_TRUE( "profile:1+profile" == doc.FindResourceByPath( "Content/Gadgets/Special/Crate.fbx" )->groupRule );
        // a rule with a group only: the sub-group still comes from the folders
        ASSERT_TRUE( Place( doc, "Content/Gadgets/Plain/Hero_Walk.anim", group, sub ) && "characters" == group && "plain" == sub );
        // the grouping hint makes Content and Gadgets roots: the nearest folders are the sub-group, up to two levels
        ASSERT_TRUE( Place( doc, "Content/Gadgets/Plain/Crate2.fbx", group, sub ) && "plain" == sub );
        ASSERT_TRUE( Place( doc, "Content/Gadgets/Deep/Rooms/Left/Chair.fbx", group, sub ) && "deep/rooms" == sub );
        ASSERT_TRUE( Place( doc, "Content/Gadgets/Extra.fbx", group, sub ) && sub.IsNULLOrEmpty() );

        // refinement: statistics make a model with bones a character, once, and back when the statistic changes
        CPackIndexResource* walker = const_cast< CPackIndexResource* >( doc.FindResourceByPath( "Content/Gadgets/Plain/Walker.fbx" ) );
        ASSERT_TRUE( GUCEF_NULL != walker && "static-models" == walker->group );
        CPackGroupRules rules( Kinds() );
        rules.SetProfile( doc.profile );
        walker->SetStat( "bones", 22 );
        rules.Refine( *walker );
        ASSERT_TRUE( "characters" == walker->group && walker->groupRule.HasSubstr( "refine:skinned" ) == 0 );
        // an explicit profile rule is not overruled by a statistic
        CPackIndexResource* crate = const_cast< CPackIndexResource* >( doc.FindResourceByPath( "Content/Gadgets/Special/Crate.fbx" ) );
        crate->SetStat( "bones", 3 );
        crate->groupRule = "profile:0";
        rules.Refine( *crate );
        ASSERT_TRUE( "static-models" == crate->group );

        // the rules hash changes with the rules, not with the files
        CPackGroupRules a( Kinds() );
        CPackGroupRules b( Kinds() );
        CPackIndexProfile withRules;
        withRules.groupRules.push_back( low );
        a.SetProfile( CPackIndexProfile() );
        b.SetProfile( withRules );
        ASSERT_TRUE( a.GetRulesHash() != b.GetRulesHash() && a.GetRulesHash() == a.GetRulesHash() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Builder: Unity-like, Godot-like and Unreal-like listings are indexed by the same engine; engine metadata is attached or counted, profile noise is hidden" )
    try
    {
        // Unity-like
        static const char* const unity[] =
        {
            "Assets/Vendor/Prefabs/Shop.prefab", "Assets/Vendor/Prefabs/Shop.prefab.meta", "Assets/Vendor/Models/Shop.fbx", "Assets/Vendor/Models/Shop.fbx.meta",
            "Assets/Vendor/Textures/Shop_D.png", "Assets/Vendor/Textures/Shop_D.png.meta", "Assets/Vendor/Materials/Shop.material", "Library/cache.bin", GUCEF_NULL
        };
        CAssetPackProfile unityProfile;
        unityProfile.packType = "unity-package";
        unityProfile.noisePatterns.push_back( "Library/*" );
        CPackIndexDocument doc;
        ASSERT_TRUE( Build( unity, unityProfile, doc ) );
        ASSERT_TRUE( 4 == doc.resources.size() && 3 == doc.counts.metadata && 1 == doc.counts.noise );
        ASSERT_TRUE( 1 == doc.FindResourceByPath( "Assets/Vendor/Textures/Shop_D.png" )->metadataFiles.size() );
        ASSERT_TRUE( "unity-package" == doc.profile.packType && doc.IsComplete() && doc.HasLevel( "structure" ) );

        // Godot-like
        static const char* const godot[] =
        {
            "project.godot", "assets/models/tree.glb", "assets/models/tree.glb.import", "assets/scenes/main.tscn", "assets/shaders/water.gdshader", "assets/audio/wind.ogg", "export_presets.cfg", GUCEF_NULL
        };
        CAssetPackProfile godotProfile;
        godotProfile.packType = "godot-project";
        CPackIndexDocument godotDoc;
        ASSERT_TRUE( Build( godot, godotProfile, godotDoc ) );
        CORE::CString group, sub;
        ASSERT_TRUE( Place( godotDoc, "assets/models/tree.glb", group, sub ) && "static-models" == group );
        ASSERT_TRUE( Place( godotDoc, "assets/scenes/main.tscn", group, sub ) && "prefabs" == group );
        ASSERT_TRUE( Place( godotDoc, "assets/shaders/water.gdshader", group, sub ) && "shaders" == group );
        ASSERT_TRUE( Place( godotDoc, "project.godot", group, sub ) && "other" == group && 1 == godotDoc.counts.metadata );

        // Unreal-like: the companion files belong to the uasset
        static const char* const unreal[] =
        {
            "Content/Props/SM_Barrel.uasset", "Content/Props/SM_Barrel.uexp", "Content/Props/SM_Barrel.ubulk", "Content/Maps/Town.umap", "Content/Maps/Town.uexp", GUCEF_NULL
        };
        CAssetPackProfile unrealProfile;
        unrealProfile.packType = "unreal-project";
        CPackIndexDocument unrealDoc;
        ASSERT_TRUE( Build( unreal, unrealProfile, unrealDoc ) );
        ASSERT_TRUE( 2 == unrealDoc.resources.size() && 3 == unrealDoc.counts.metadata );
        ASSERT_TRUE( 2 == unrealDoc.FindResourceByPath( "Content/Props/SM_Barrel.uasset" )->metadataFiles.size() );
        ASSERT_TRUE( Place( unrealDoc, "Content/Maps/Town.umap", group, sub ) && "scenes" == group );

        // the same listing twice: the same document
        CPackIndexDocument again;
        ASSERT_TRUE( Build( unity, unityProfile, again ) );
        ASSERT_TRUE( again.ComputeDocumentHash() == doc.ComputeDocumentHash() && again.producer.indexKey == doc.producer.indexKey );
        // one file more: another pack key
        static const char* const unityPlus[] = { "Assets/Vendor/Prefabs/Shop.prefab", "Assets/Vendor/Models/Shop.fbx", "Assets/Vendor/Models/Extra.fbx", GUCEF_NULL };
        CPackIndexDocument plus;
        ASSERT_TRUE( Build( unityPlus, unityProfile, plus ) );
        ASSERT_TRUE( plus.identity.packKey != doc.identity.packKey && plus.producer.indexKey != doc.producer.indexKey );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Builder on a folder: sizes come from the files, the structure level reads no content, the image headers are probed once and not again" )
    try
    {
        MakeWorkFolder( "pidx_folder" );
        WriteTestPng( "work/pidx_folder/src/props/rock.png", 8, 4, 1, 2, 3, 255 );
        WriteWorkFile( "pidx_folder/src/props/rock.txt", "twelve bytes" );
        WriteWorkFile( "pidx_folder/src/props/broken.png", "this is not a png" );
        CAssetPackListing listing;
        ASSERT_TRUE( listing.LoadFromVfs( "work/pidx_folder/src" ) );
        CAssetPackProfile profile;
        CPackIndexBuildOptions options;
        options.plugins[ "recogniser:test" ] = 1;
        CPackIndexUpdater updater( Kinds() );
        CPackIndexUpdateOptions update;
        update.level = CPackIndexUpdateOptions::LEVEL_STRUCTURE;
        CPackIndexDocument doc;
        CPackIndexRunResult result;
        const CORE::CString folder = "work/pidx_folder/index";
        ASSERT_TRUE( updater.EnsureIndex( listing, profile, options, update, folder, doc, result ) );
        ASSERT_TRUE( result.structureBuilt && result.written && 3 == result.resourcesGrouped && 0 == result.resourcesAnalysed );
        ASSERT_TRUE( 12 == doc.FindResourceByPath( "props/rock.txt" )->size && doc.FindResourceByPath( "props/rock.png" )->size > 8 );
        ASSERT_FALSE( doc.HasLevel( "analysis" ) );

        // no folder given: the index lives in the user cache folder of the pack key
        {
            MakeWorkFolder( "pidx_usercache" );
            CPackIndexUpdateOptions cached;
            cached.userCacheRoot = "work/pidx_usercache";
            CPackIndexRunResult cacheResult;
            CPackIndexDocument cacheDoc;
            ASSERT_TRUE( updater.EnsureIndex( listing, profile, options, cached, CORE::CString::Empty, cacheDoc, cacheResult ) && cacheResult.written );
            ASSERT_TRUE( cacheResult.indexFolder == CPackIndexLocator::GetUserCacheFolder( "work/pidx_usercache", cacheDoc.identity.packKey ) );
            ASSERT_TRUE( VfsExists( cacheResult.indexFolder + "/pack-index.json" ) );
            ASSERT_TRUE( updater.EnsureIndex( listing, profile, options, cached, CORE::CString::Empty, cacheDoc, cacheResult ) && cacheResult.reused && 0 == cacheResult.resourcesGrouped );
        }

        // a second call: fresh, nothing is built or written
        ASSERT_TRUE( updater.EnsureIndex( listing, profile, options, update, folder, doc, result ) );
        ASSERT_TRUE( result.reused && !result.structureBuilt && !result.written && 0 == result.resourcesGrouped && CPackGateDecision::OUTCOME_FRESH == result.indexDecision.outcome );

        // the analysis level on top: the headers are probed, the broken file fails and stays failed
        update.level = CPackIndexUpdateOptions::LEVEL_ANALYSIS;
        ASSERT_TRUE( updater.EnsureIndex( listing, profile, options, update, folder, doc, result ) );
        ASSERT_TRUE( !result.structureBuilt && result.written && 1 == result.resourcesAnalysed && 1 == result.analysisFailed );
        UInt64 width = 0, height = 0;
        const CPackIndexResource* rock = doc.FindResourceByPath( "props/rock.png" );
        ASSERT_TRUE( rock->GetStat( "width", width ) && rock->GetStat( "height", height ) && 8 == width && 4 == height );
        ASSERT_TRUE( CPackIndexStatus::Failed == doc.FindResourceByPath( "props/broken.png" )->status.state && CPackIndexDocument::StateCompleteWithFailures == doc.state );
        ASSERT_TRUE( doc.HasLevel( "analysis" ) );
        ASSERT_TRUE( updater.EnsureIndex( listing, profile, options, update, folder, doc, result ) );
        ASSERT_TRUE( result.reused && 0 == result.resourcesAnalysed && 0 == result.analysisFailed );

        // retry of the failed one, and only that
        update.redo.retryFailed = true;
        ASSERT_TRUE( updater.EnsureIndex( listing, profile, options, update, folder, doc, result ) );
        ASSERT_TRUE( 1 == result.analysisFailed && 2 == result.analysisReused );
        update.redo = CPackRedoRequest();

        // a changed file is analysed again alone, the structure is built again keeping the other records
        WriteTestPng( "work/pidx_folder/src/props/rock.png", 16, 16, 9, 9, 9, 255 );
        ASSERT_TRUE( listing.LoadFromVfs( "work/pidx_folder/src" ) );
        ASSERT_TRUE( updater.EnsureIndex( listing, profile, options, update, folder, doc, result ) );
        ASSERT_TRUE( result.structureBuilt && CPackGateDecision::OUTCOME_STALE == result.indexDecision.outcome && ACTIVITYREASON_SOURCE_CHANGED == result.indexDecision.reason );
        ASSERT_TRUE( 2 == result.recordsKept && 1 == result.resourcesAnalysed );
        ASSERT_TRUE( doc.FindResourceByPath( "props/rock.png" )->GetStat( "width", width ) && 16 == width );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Gate: fresh, stale for every input, missing, corrupt, forced and refused; the conversion steps equal the comparison of the cache entry" )
    try
    {
        static const char* const files[] = { "a/x.fbx", "a/y.png", GUCEF_NULL };
        CAssetPackListing listing;
        AddFiles( listing, files );
        CPackIndexBuilder builder( Kinds() );
        CPackIndexBuildOptions options = SyntheticOptions();
        CPackIndexDocument stored;
        ASSERT_TRUE( builder.BuildStructure( listing, CAssetPackProfile(), options, stored, GUCEF_NULL ) );
        CPackIndexDocument current;
        ASSERT_TRUE( builder.PrepareInputs( listing, CAssetPackProfile(), options, current ) );
        CPackRedoRequest none;
        const CPackIndexReader::EResult ok = CPackIndexReader::RESULT_OK;

        CPackGateDecision decision = CPackWorkGate::DecideIndex( ok, stored, current, none );
        ASSERT_TRUE( decision.IsFresh() && !decision.NeedsWork() && ACTIVITYREASON_UNCHANGED == decision.reason );
        ASSERT_TRUE( CPackGateDecision::OUTCOME_MISSING == CPackWorkGate::DecideIndex( CPackIndexReader::RESULT_MISSING, stored, current, none ).outcome );
        decision = CPackWorkGate::DecideIndex( CPackIndexReader::RESULT_CORRUPT, stored, current, none );
        ASSERT_TRUE( ACTIVITYREASON_DOCUMENT_CORRUPT == decision.reason && decision.NeedsWork() );
        decision = CPackWorkGate::DecideIndex( CPackIndexReader::RESULT_VERSION_TOO_NEW, stored, current, none );
        ASSERT_TRUE( CPackGateDecision::OUTCOME_REFUSED == decision.outcome && ACTIVITYREASON_SCHEMA_CHANGED == decision.reason && !decision.NeedsWork() );
        CPackRedoRequest reindex;
        reindex.reindex = true;
        decision = CPackWorkGate::DecideIndex( ok, stored, current, reindex );
        ASSERT_TRUE( CPackGateDecision::OUTCOME_FORCED == decision.outcome && ACTIVITYREASON_FORCED == decision.reason );

        // each input in turn
        CPackIndexDocument c = current; c.identity.fileCount = 3;
        decision = CPackWorkGate::DecideIndex( ok, stored, c, none );
        ASSERT_TRUE( ACTIVITYREASON_SOURCE_CHANGED == decision.reason && decision.detail.HasSubstr( "file count" ) >= 0 );
        c = current; c.identity.totalBytes += 1;
        ASSERT_TRUE( ACTIVITYREASON_SOURCE_CHANGED == CPackWorkGate::DecideIndex( ok, stored, c, none ).reason );
        c = current; c.identity.treeHash = "other";
        ASSERT_TRUE( ACTIVITYREASON_SOURCE_CHANGED == CPackWorkGate::DecideIndex( ok, stored, c, none ).reason );
        c = current; c.producer.settingsHash = "other";
        ASSERT_TRUE( ACTIVITYREASON_SETTINGS_CHANGED == CPackWorkGate::DecideIndex( ok, stored, c, none ).reason );
        c = current; c.producer.plugins[ "recogniser:test" ] = 2;
        decision = CPackWorkGate::DecideIndex( ok, stored, c, none );
        ASSERT_TRUE( ACTIVITYREASON_VERSION_CHANGED == decision.reason && decision.detail.HasSubstr( "recogniser:test" ) >= 0 );
        c = current; c.producer.plugins[ "recogniser:new" ] = 1;
        ASSERT_TRUE( ACTIVITYREASON_VERSION_CHANGED == CPackWorkGate::DecideIndex( ok, stored, c, none ).reason );
        c = current; c.producer.kindRegistryHash = "other";
        ASSERT_TRUE( ACTIVITYREASON_VERSION_CHANGED == CPackWorkGate::DecideIndex( ok, stored, c, none ).reason );
        c = current; c.producer.groupRulesHash = "other";
        ASSERT_TRUE( ACTIVITYREASON_VERSION_CHANGED == CPackWorkGate::DecideIndex( ok, stored, c, none ).reason );
        c = current; c.producer.schemaWriter += 1;
        ASSERT_TRUE( ACTIVITYREASON_VERSION_CHANGED == CPackWorkGate::DecideIndex( ok, stored, c, none ).reason );
        CPackIndexDocument oldSchema = stored; oldSchema.version = 0;
        ASSERT_TRUE( ACTIVITYREASON_SCHEMA_CHANGED == CPackWorkGate::DecideIndex( ok, oldSchema, current, none ).reason );
        CPackIndexDocument partial = stored; partial.state = CPackIndexDocument::StatePartial;
        ASSERT_TRUE( CPackWorkGate::DecideIndex( ok, partial, current, none ).NeedsWork() );
        // content is verified only on request
        CPackRedoRequest verify;
        verify.verifyContent = true;
        ASSERT_TRUE( CPackWorkGate::DecideIndex( ok, stored, current, none ).IsFresh() && CPackWorkGate::DecideIndex( ok, stored, current, verify ).NeedsWork() );
        c = current; c.identity.contentHash = "abc"; CPackIndexDocument s = stored; s.identity.contentHash = "abc";
        ASSERT_TRUE( CPackWorkGate::DecideIndex( ok, s, c, verify ).IsFresh() );

        // recognise
        ASSERT_TRUE( CPackWorkGate::DecideRecognise( &stored, current, none ).IsFresh() );
        c = current; c.profile.recognitionKey = "other";
        ASSERT_TRUE( ACTIVITYREASON_SOURCE_CHANGED == CPackWorkGate::DecideRecognise( &stored, c, none ).reason && CPackGateDecision::OUTCOME_MISSING == CPackWorkGate::DecideRecognise( GUCEF_NULL, c, none ).outcome );

        // analyse, per record
        CPackIndexResource record = MakeResource( "a/y.png", 10 );
        const CORE::CString key = CPackWorkGate::MakeAnalyseKey( "p", 1, "s", CORE::CString::Empty, 10 );
        ASSERT_TRUE( CPackGateDecision::OUTCOME_MISSING == CPackWorkGate::DecideAnalyse( &record, key, 10, none ).outcome );
        CPackIndexStepStatus step( CPackIndexStepStatus::Fresh );
        step.key = key;
        record.status.SetStep( CPackIndexStatus::StepAnalyse, step );
        ASSERT_TRUE( CPackWorkGate::DecideAnalyse( &record, key, 10, none ).IsFresh() );
        ASSERT_TRUE( ACTIVITYREASON_SOURCE_CHANGED == CPackWorkGate::DecideAnalyse( &record, CPackWorkGate::MakeAnalyseKey( "p", 1, "s", CORE::CString::Empty, 11 ), 11, none ).reason );
        ASSERT_TRUE( ACTIVITYREASON_VERSION_CHANGED == CPackWorkGate::DecideAnalyse( &record, CPackWorkGate::MakeAnalyseKey( "p", 2, "s", CORE::CString::Empty, 10 ), 10, none ).reason );
        CPackRedoRequest reanalyse;
        reanalyse.reanalyse = true;
        ASSERT_TRUE( CPackGateDecision::OUTCOME_FORCED == CPackWorkGate::DecideAnalyse( &record, key, 10, reanalyse ).outcome );
        record.status.steps[ CPackIndexStatus::StepAnalyse ].state = CPackIndexStepStatus::Failed;
        CPackRedoRequest retry;
        retry.retryFailed = true;
        ASSERT_TRUE( CPackWorkGate::DecideAnalyse( &record, key, 10, none ).IsFresh() && CPackWorkGate::DecideAnalyse( &record, key, 10, retry ).NeedsWork() );

        // convert: the answer of the gate is the answer of CompareWith(), plus the licence and the forcing
        CAssetCacheEntry storedEntry;
        storedEntry.assetPath = "a/x.fbx";
        storedEntry.sourceHash = "s1";
        storedEntry.settingsHash = "t1";
        storedEntry.plugins[ "importer:x" ] = 1;
        storedEntry.ComputeKey();
        CAssetCacheEntry now = storedEntry;
        ASSERT_TRUE( CPackWorkGate::DecideConvert( &storedEntry, now, false, CORE::CString::Empty, none ).IsFresh() );
        ASSERT_TRUE( CPackGateDecision::OUTCOME_MISSING == CPackWorkGate::DecideConvert( GUCEF_NULL, now, false, CORE::CString::Empty, none ).outcome );
        CAssetCacheEntry changes[ 3 ] = { storedEntry, storedEntry, storedEntry };
        changes[ 0 ].sourceHash = "s2"; changes[ 0 ].ComputeKey();
        changes[ 1 ].settingsHash = "t2"; changes[ 1 ].ComputeKey();
        changes[ 2 ].plugins[ "importer:x" ] = 2; changes[ 2 ].ComputeKey();
        for ( int i=0; i<3; ++i )
        {
            CORE::CString detail;
            const TActivityReason expected = storedEntry.CompareWith( changes[ i ], detail );
            const CPackGateDecision answer = CPackWorkGate::DecideConvert( &storedEntry, changes[ i ], false, CORE::CString::Empty, none );
            ASSERT_TRUE( CPackGateDecision::OUTCOME_STALE == answer.outcome && expected == answer.reason && detail == answer.detail );
        }
        CPackRedoRequest reimport;
        reimport.reimport = true;
        ASSERT_TRUE( CPackGateDecision::OUTCOME_FORCED == CPackWorkGate::DecideConvert( &storedEntry, now, false, CORE::CString::Empty, reimport ).outcome );
        const CPackGateDecision refused = CPackWorkGate::DecideConvert( &storedEntry, now, true, "no conversion", reimport );
        ASSERT_TRUE( CPackGateDecision::OUTCOME_REFUSED == refused.outcome && ACTIVITYREASON_LICENCE == refused.reason );

        // thumbnail
        CPackIndexThumbnail wanted;
        wanted.size = 128; wanted.digest = "d"; wanted.rendererId = "r"; wanted.rendererVersion = 1;
        CPackIndexResource shown = MakeResource( "a/x.fbx", 10 );
        ASSERT_TRUE( CPackGateDecision::OUTCOME_MISSING == CPackWorkGate::DecideThumbnail( &shown, wanted, none ).outcome );
        shown.thumbnails.push_back( wanted );
        ASSERT_TRUE( CPackWorkGate::DecideThumbnail( &shown, wanted, none ).IsFresh() );
        CPackIndexThumbnail newer = wanted; newer.rendererVersion = 2;
        ASSERT_TRUE( ACTIVITYREASON_VERSION_CHANGED == CPackWorkGate::DecideThumbnail( &shown, newer, none ).reason );
        newer = wanted; newer.digest = "e";
        ASSERT_TRUE( ACTIVITYREASON_SOURCE_CHANGED == CPackWorkGate::DecideThumbnail( &shown, newer, none ).reason );
        CPackRedoRequest rethumbnail;
        rethumbnail.rethumbnail = true;
        ASSERT_TRUE( CPackGateDecision::OUTCOME_FORCED == CPackWorkGate::DecideThumbnail( &shown, wanted, rethumbnail ).outcome );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Image probe: the header of each supported format gives the size without decoding; a file of another format gives nothing" )
    try
    {
        UInt32 w = 0, h = 0, c = 0, d = 0;
        const UInt8 png[ 26 ] = { 0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A, 0, 0, 0, 13, 'I', 'H', 'D', 'R', 0, 0, 1, 0, 0, 0, 0, 128, 8, 6 };
        ASSERT_TRUE( CPackIndexImageProbe::ProbeBytes( png, 26, "png", w, h, c, d ) && 256 == w && 128 == h && 4 == c && 8 == d );
        const UInt8 jpg[ 24 ] = { 0xFF, 0xD8, 0xFF, 0xC0, 0, 17, 8, 0, 64, 0, 32, 3, 1, 0x22, 0, 2, 0x11, 1, 3, 0x11, 1, 0, 0, 0 };
        ASSERT_TRUE( CPackIndexImageProbe::ProbeBytes( jpg, 24, "jpg", w, h, c, d ) && 32 == w && 64 == h && 3 == c );
        UInt8 bmp[ 30 ] = { 'B', 'M' };
        bmp[ 18 ] = 20; bmp[ 22 ] = 10; bmp[ 28 ] = 24;
        ASSERT_TRUE( CPackIndexImageProbe::ProbeBytes( bmp, 30, "bmp", w, h, c, d ) && 20 == w && 10 == h && 3 == c );
        const UInt8 gif[ 10 ] = { 'G', 'I', 'F', '8', '9', 'a', 5, 0, 7, 0 };
        ASSERT_TRUE( CPackIndexImageProbe::ProbeBytes( gif, 10, "gif", w, h, c, d ) && 5 == w && 7 == h );
        UInt8 tga[ 18 ] = { 0 };
        tga[ 12 ] = 32; tga[ 14 ] = 16; tga[ 16 ] = 32;
        ASSERT_TRUE( CPackIndexImageProbe::ProbeBytes( tga, 18, "tga", w, h, c, d ) && 32 == w && 16 == h && 4 == c );
        UInt8 dds[ 20 ] = { 'D', 'D', 'S', ' ' };
        dds[ 12 ] = 64; dds[ 16 ] = 128;
        ASSERT_TRUE( CPackIndexImageProbe::ProbeBytes( dds, 20, "dds", w, h, c, d ) && 128 == w && 64 == h );
        ASSERT_FALSE( CPackIndexImageProbe::ProbeBytes( png, 26, "jpg", w, h, c, d ) );
        ASSERT_FALSE( CPackIndexImageProbe::ProbeBytes( png, 10, "png", w, h, c, d ) );
        ASSERT_TRUE( CPackIndexImageProbe::IsSupported( "png" ) && !CPackIndexImageProbe::IsSupported( "psd" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Pipeline: a run writes the index with the outputs; the second run reuses it (counters, not time); the flags redo exactly their steps; no-index and index-only" )
    try
    {
        const CORE::CString manifestPath = WriteStandardPack( "pidx_run", CORE::CString::Empty );
        const CORE::CString indexFolder = "work/pidx_run/intermediate";
        CAssetPackManifest manifest;
        ASSERT_TRUE( manifest.Load( manifestPath ) );
        CAssetPipeline pipeline;
        CConversionReport report;

        CAssetPipelineOptions analysing;
        analysing.analyse = true;
        ASSERT_TRUE( pipeline.RunPack( manifest, analysing, report ) );
        const CPackIndexRunResult first = pipeline.GetLastIndexResult();
        ASSERT_TRUE( first.structureBuilt && first.written && first.resourcesGrouped > 0 && first.outputsRefreshed > 0 );
        ASSERT_TRUE( VfsExists( indexFolder + "/pack-index.json" ) );
        CPackIndexReader reader;
        CPackIndexDocument doc;
        ASSERT_TRUE( CPackIndexReader::RESULT_OK == reader.Read( indexFolder, doc ) && doc.IsComplete() );
        const CPackIndexResource* rock = doc.FindResourceByPath( "props/rock.stub" );
        ASSERT_TRUE( GUCEF_NULL != rock && !rock->outputs.cacheKey.IsNULLOrEmpty() && !rock->outputs.intermediate.path.IsNULLOrEmpty() && !rock->outputs.cooked.empty() );
        ASSERT_TRUE( CPackIndexStatus::Converted == rock->status.state && GUCEF_NULL != rock->status.FindStep( CPackIndexStatus::StepImport ) );
        ASSERT_TRUE( doc.HasLevel( "outputs" ) && doc.HasLevel( "analysis" ) );
        UInt64 width = 0;
        ASSERT_TRUE( GUCEF_NULL != doc.FindResourceByPath( "props/rock.png" ) && doc.FindResourceByPath( "props/rock.png" )->GetStat( "width", width ) && 4 == width );
        const CORE::CString before = TextOfManifest( indexFolder );

        // a second run: every asset is cached and the index stands: no grouping, no analysis, no write
        const UInt32 importsBefore = GetStubImporter().importCount;
        ASSERT_TRUE( pipeline.RunPack( manifest, CAssetPipelineOptions(), report ) );
        CPackIndexRunResult second = pipeline.GetLastIndexResult();
        ASSERT_TRUE( importsBefore == GetStubImporter().importCount );
        ASSERT_TRUE( second.reused && !second.written && !second.structureBuilt && 0 == second.resourcesGrouped && 0 == second.resourcesAnalysed );
        ASSERT_TRUE( before == TextOfManifest( indexFolder ) );

        // re-index: the structure is built again, the records are kept, nothing is analysed
        CAssetPipelineOptions options;
        options.reindex = true;
        ASSERT_TRUE( pipeline.RunPack( manifest, options, report ) );
        CPackIndexRunResult third = pipeline.GetLastIndexResult();
        ASSERT_TRUE( third.structureBuilt && third.resourcesGrouped == doc.resources.size() && third.recordsKept == doc.resources.size() && 0 == third.resourcesAnalysed );
        ASSERT_TRUE( CPackGateDecision::OUTCOME_FORCED == third.indexDecision.outcome );

        // re-analyse: the structure stands, the headers are read again
        options = CAssetPipelineOptions();
        options.reanalyse = true;
        ASSERT_TRUE( pipeline.RunPack( manifest, options, report ) );
        CPackIndexRunResult fourth = pipeline.GetLastIndexResult();
        ASSERT_TRUE( !fourth.structureBuilt && 0 == fourth.resourcesGrouped && 2 == fourth.resourcesAnalysed );

        // --force keeps its meaning (cooking despite validation errors): it redoes no index step
        options = CAssetPipelineOptions();
        options.force = true;
        ASSERT_TRUE( pipeline.RunPack( manifest, options, report ) );
        ASSERT_TRUE( pipeline.GetLastIndexResult().reused );

        // a changed source: the cache entry is stale, the file list is the same: the index follows through the outputs
        WriteWorkFile( "pidx_run/src/props/crate.stub", MakeStubSource( "CrateChanged", "crate.png" ) );
        ASSERT_TRUE( pipeline.RunPack( manifest, CAssetPipelineOptions(), report ) );
        ASSERT_TRUE( pipeline.GetLastIndexResult().indexDecision.IsFresh() || pipeline.GetLastIndexResult().indexDecision.NeedsWork() );

        // no-index: the file is not written; index-only: no conversion, the index is
        CORE::CString text = TextOfManifest( indexFolder );
        VFS::CVfsGlobal::Instance()->GetVfs().DeleteFile( indexFolder + "/pack-index.json", true );
        options = CAssetPipelineOptions();
        options.noIndex = true;
        ASSERT_TRUE( pipeline.RunPack( manifest, options, report ) );
        ASSERT_FALSE( VfsExists( indexFolder + "/pack-index.json" ) );
        const UInt32 importsMid = GetStubImporter().importCount;
        options = CAssetPipelineOptions();
        options.indexOnly = true;
        options.reimport = true;
        ASSERT_TRUE( pipeline.RunPack( manifest, options, report ) );
        ASSERT_TRUE( importsMid == GetStubImporter().importCount && VfsExists( indexFolder + "/pack-index.json" ) && pipeline.GetLastIndexResult().written );

        // the dry run says what the index step would do
        CAssetPackPlan plan;
        ASSERT_TRUE( pipeline.PlanPack( manifest, CAssetPipelineOptions(), plan ) );
        ASSERT_TRUE( plan.indexNote.HasSubstr( "fresh" ) >= 0 );
        options = CAssetPipelineOptions();
        options.reindex = true;
        ASSERT_TRUE( pipeline.PlanPack( manifest, options, plan ) && plan.indexNote.HasSubstr( "forced" ) >= 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Relations: the index takes refs and unresolvedRefs from the sidecars (ids and paths), they round trip; an index of an existing intermediate gets them without a conversion; references.json lists them per asset" )
    try
    {
        const CORE::CString manifestPath = WriteStandardPack( "pidx_rel", CORE::CString::Empty );
        WriteWorkFile( "pidx_rel/src/props/lamp.stub", "{ \"nodes\": [ { \"name\": \"Lamp\", \"mesh\": true, \"size\": 1, \"material\": 0 } ], "
                       "\"materials\": [ { \"name\": \"LampMaterial\", \"baseColor\": \"crate.png\" } ], "
                       "\"references\": [ { \"kind\": \"instancesPrefab\", \"path\": \"crate.stub\", \"via\": \"guid:crate\" }, "
                       "{ \"kind\": \"dependsOn\", \"path\": \"rock.notes.txt\", \"via\": \"notes\", \"optional\": true }, "
                       "{ \"kind\": \"usesMesh\", \"via\": \"guid:engine-mesh\" }, { \"kind\": \"usesTexture\", \"via\": \"missing_lamp.png\" } ] }" );
        const CORE::CString indexFolder = "work/pidx_rel/intermediate";
        CAssetPackManifest manifest;
        ASSERT_TRUE( manifest.Load( manifestPath ) );
        CAssetPipeline pipeline;
        CConversionReport report;
        ASSERT_TRUE( pipeline.RunPack( manifest, CAssetPipelineOptions(), report ) );

        CPackIndexReader reader;
        CPackIndexDocument doc;
        ASSERT_TRUE( CPackIndexReader::RESULT_OK == reader.Read( indexFolder, doc ) );
        const CORE::CString idBase = doc.identity.name;
        const CPackIndexResource* rock = doc.FindResourceByPath( "props/rock.stub" );
        ASSERT_TRUE( GUCEF_NULL != rock && 2 == rock->refs.size() );
        // the texture of the material and, by the safety net, the dependency of the import
        ASSERT_TRUE( "dependsOn" == rock->refs[ 0 ].kind && "props/rock.notes.txt" == rock->refs[ 0 ].path && CPackIndexResource::MakeId( idBase, "props/rock.notes.txt" ) == rock->refs[ 0 ].to );
        ASSERT_TRUE( "usesTexture" == rock->refs[ 1 ].kind && "props/rock.png" == rock->refs[ 1 ].path && "rock.png" == rock->refs[ 1 ].via );
        const CPackIndexResource* texture = doc.FindResourceByPath( "props/rock.png" );
        ASSERT_TRUE( GUCEF_NULL != texture && texture->id == rock->refs[ 1 ].to && texture->refs.empty() );
        const CPackIndexResource* lamp = doc.FindResourceByPath( "props/lamp.stub" );
        ASSERT_TRUE( GUCEF_NULL != lamp && 3 == lamp->refs.size() && 2 == lamp->unresolvedRefs.size() );
        ASSERT_TRUE( "dependsOn" == lamp->refs[ 0 ].kind && lamp->refs[ 0 ].optional && "instancesPrefab" == lamp->refs[ 1 ].kind && "props/crate.stub" == lamp->refs[ 1 ].path && "guid:crate" == lamp->refs[ 1 ].via );
        ASSERT_TRUE( "usesTexture" == lamp->refs[ 2 ].kind && "props/crate.png" == lamp->refs[ 2 ].path && !lamp->refs[ 2 ].optional );
        ASSERT_TRUE( "guid:engine-mesh" == lamp->unresolvedRefs[ 0 ] && "missing_lamp.png" == lamp->unresolvedRefs[ 1 ] );
        const CORE::CString manifestText = TextOfManifest( indexFolder );
        ASSERT_TRUE( manifestText.HasSubstr( "\"path\": \"props/crate.stub\"" ) > 0 && manifestText.HasSubstr( "\"optional\": true" ) > 0 );

        // the document round trips with the members of the references
        MakeWorkFolder( "pidx_rel_copy" );
        CPackIndexWriter writer;
        ASSERT_TRUE( writer.Write( doc, "work/pidx_rel_copy" ) );
        CPackIndexDocument copy;
        ASSERT_TRUE( CPackIndexReader::RESULT_OK == reader.Read( "work/pidx_rel_copy", copy ) );
        const CPackIndexResource* lampCopy = copy.FindResourceByPath( "props/lamp.stub" );
        ASSERT_TRUE( GUCEF_NULL != lampCopy && 3 == lampCopy->refs.size() && lampCopy->refs[ 0 ].optional && "props/crate.stub" == lampCopy->refs[ 1 ].path && 2 == lampCopy->unresolvedRefs.size() );
        ASSERT_TRUE( TextOfManifest( "work/pidx_rel_copy" ) == manifestText );

        // the cooked output lists the relations per asset; the cooked target of the instance is named
        const CORE::CString references = ReadVfsText( "work/pidx_rel/out/desktop/references.json" );
        ASSERT_TRUE( references.HasSubstr( "\"assets\"" ) > 0 && references.HasSubstr( "guid:engine-mesh" ) > 0 );
        CReferenceGraph graph;
        ASSERT_TRUE( graph.Load( "work/pidx_rel/out/desktop/references.json" ) );
        CReferenceGraph::TAssetRelationMap::const_iterator lampRelations = graph.GetAssetRelations().find( "props/lamp.stub" );
        ASSERT_TRUE( lampRelations != graph.GetAssetRelations().end() );
        bool instanceFiles = false;
        for ( size_t r=0; r<(*lampRelations).second.size(); ++r )
            if ( "instancesPrefab" == (*lampRelations).second[ r ].relation.kind )
                instanceFiles = !(*lampRelations).second[ r ].files.empty();
        ASSERT_TRUE( instanceFiles );

        // an index made from the intermediate that is there (no conversion): the refs come from the sidecars again
        VFS::CVfsGlobal::Instance()->GetVfs().DeleteFile( indexFolder + "/pack-index.json", true );
        const UInt32 imports = GetStubImporter().importCount;
        CAssetPipelineOptions indexOnly;
        indexOnly.indexOnly = true;
        ASSERT_TRUE( pipeline.RunPack( manifest, indexOnly, report ) );
        ASSERT_TRUE( imports == GetStubImporter().importCount );
        CPackIndexDocument rebuilt;
        ASSERT_TRUE( CPackIndexReader::RESULT_OK == reader.Read( indexFolder, rebuilt ) );
        const CPackIndexResource* lampAgain = rebuilt.FindResourceByPath( "props/lamp.stub" );
        ASSERT_TRUE( GUCEF_NULL != lampAgain && 3 == lampAgain->refs.size() && 2 == lampAgain->unresolvedRefs.size() );

        // ReadSidecarRelations: a resource without a sidecar keeps what it has
        CPackIndexResource loose;
        loose.path = "readme.txt";
        CPackIndexReference kept;
        kept.kind = "dependsOn";
        kept.to = "x";
        loose.refs.push_back( kept );
        ASSERT_FALSE( CPackIndexUpdater::ReadSidecarRelations( indexFolder, idBase, loose ) );
        ASSERT_TRUE( 1 == loose.refs.size() );
        CPackIndexResource converted;
        converted.path = "props/rock.stub";
        ASSERT_TRUE( CPackIndexUpdater::ReadSidecarRelations( indexFolder, "otherBase", converted ) && 2 == converted.refs.size() );
        ASSERT_TRUE( CPackIndexResource::MakeId( "otherBase", "props/rock.png" ) == converted.refs[ 1 ].to );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Concurrency and recovery: a second writer is refused with the holder named and reads the stored index; a corrupt manifest is renamed and built again; a newer index is never touched" )
    try
    {
        MakeWorkFolder( "pidx_conc" );
        WriteWorkFile( "pidx_conc/src/a.fbx", "mesh" );
        WriteWorkFile( "pidx_conc/src/b.png", "pic" );
        CAssetPackListing listing;
        ASSERT_TRUE( listing.LoadFromVfs( "work/pidx_conc/src" ) );
        CPackIndexBuildOptions options;
        CPackIndexUpdater updater( Kinds() );
        CPackIndexUpdateOptions update;
        CPackIndexDocument doc;
        CPackIndexRunResult result;
        const CORE::CString folder = "work/pidx_conc/index";

        {
            CIntermediateLock holder( CPackIndexLocator::GetShardFolder( folder ) );
            ASSERT_TRUE( holder.IsHeld() );
            // no stored index and the lock held by someone else: nothing is written, the holder is named
            ASSERT_FALSE( updater.EnsureIndex( listing, CAssetPackProfile(), options, update, folder, doc, result ) );
            ASSERT_TRUE( result.locked && !result.written && result.holder.HasSubstr( "process" ) >= 0 );
            ASSERT_FALSE( VfsExists( folder + "/pack-index.json" ) );
        }
        ASSERT_TRUE( updater.EnsureIndex( listing, CAssetPackProfile(), options, update, folder, doc, result ) && result.written );
        {
            // a change while another writer holds the lock: the stored index is returned
            WriteWorkFile( "pidx_conc/src/c.fbx", "more" );
            ASSERT_TRUE( listing.LoadFromVfs( "work/pidx_conc/src" ) );
            CIntermediateLock holder( CPackIndexLocator::GetShardFolder( folder ) );
            ASSERT_TRUE( updater.EnsureIndex( listing, CAssetPackProfile(), options, update, folder, doc, result ) );
            ASSERT_TRUE( result.locked && !result.written && 2 == doc.resources.size() );
        }
        ASSERT_TRUE( updater.EnsureIndex( listing, CAssetPackProfile(), options, update, folder, doc, result ) && result.written && 3 == doc.resources.size() );
        ASSERT_FALSE( CORE::FileExists( WorkDiskPath( "pidx_conc/index/pack-index.d/.lock" ) ) );

        // a corrupt manifest: renamed, built again
        WriteWorkFile( "pidx_conc/index/pack-index.json", "{ not json" );
        ASSERT_TRUE( updater.EnsureIndex( listing, CAssetPackProfile(), options, update, folder, doc, result ) );
        ASSERT_TRUE( ACTIVITYREASON_DOCUMENT_CORRUPT == result.indexDecision.reason && result.structureBuilt && result.written && VfsExists( folder + "/pack-index.json.bad" ) );

        // a newer version is refused and left as it is
        CORE::CDataNode node;
        ASSERT_TRUE( CCanonicalJson::Load( folder + "/pack-index.json", node ) );
        node.SetAttribute( "version", (UInt32) 7 );
        ASSERT_TRUE( CCanonicalJson::Store( folder + "/pack-index.json", node ) );
        const CORE::CString newer = TextOfManifest( folder );
        ASSERT_FALSE( updater.EnsureIndex( listing, CAssetPackProfile(), options, update, folder, doc, result ) );
        ASSERT_TRUE( result.refused && !result.written && newer == TextOfManifest( folder ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
