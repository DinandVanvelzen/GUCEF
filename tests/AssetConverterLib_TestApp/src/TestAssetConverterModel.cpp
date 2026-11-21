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

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#include "TestAssetConverterSupport.h"
#include "TestAssetConverterModel.h"

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

CORE::CString
Attr( const CORE::CDataNode& node, const char* name )
{
    return node.GetAttributeValueOrChildValueByName( name ).AsString();
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAssetConverterModelTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AssetConverterLib.Model" );

    GUCEF_TESTFW_TESTCASE( "Registry: the built-in post-processors and validators are there, plugins register by name" )
    try
    {
        CAssetPluginRegistry& plugins = CAssetConverterGlobal::Instance()->GetPluginRegistry();
        const char* const processors[] = { "NameHints", "GenerateTangents", "PruneEmptyNodes", "MergeIdenticalMaterials" };
        for ( UInt32 i=0; i<4; ++i )
        {
            TAssetPostProcessorPtr processor;
            ASSERT_TRUE( plugins.TryGetPostProcessor( processors[ i ], processor ) && processor->GetName() == processors[ i ] && processor->GetVersion() >= 1 );
            ASSERT_TRUE( plugins.GetPostProcessors().IsBuiltIn( processors[ i ] ) );
        }
        const char* const validators[] = { "TextureSize", "MissingTextures", "BoneLimit", "TriangleBudget", "Naming", "LayerPenetration" };
        for ( UInt32 i=0; i<6; ++i )
        {
            TAssetValidatorPtr validator;
            ASSERT_TRUE( plugins.TryGetValidator( validators[ i ], validator ) && validator->GetName() == validators[ i ] );
        }

        // the plugins of the tests, found by name in any letter case
        TAssetImporterPtr importer;
        ASSERT_TRUE( plugins.TryGetImporter( "STUB", importer ) && "stub" == importer->GetName() );
        TAssetExporterPtr exporter;
        ASSERT_TRUE( plugins.TryGetExporter( "gltf", exporter ) );
        TAssetCookerPtr cooker;
        ASSERT_TRUE( plugins.TryGetCooker( "stubcook", cooker ) && "stubcook" == cooker->GetTargetName() );
        ASSERT_FALSE( plugins.TryGetImporter( "nosuchimporter", importer ) );

        // a cooker is also found by an alias (the name of the cooked area of the store): the target name is what the cooker has
        ASSERT_FALSE( plugins.TryGetCooker( "stubcook-area", cooker ) );
        ASSERT_TRUE( plugins.RegisterCookerAlias( "Stubcook-Area", "stubcook" ) );
        ASSERT_TRUE( plugins.TryGetCooker( "stubcook-area", cooker ) && "stubcook" == cooker->GetTargetName() );
        ASSERT_TRUE( "stubcook" == plugins.ResolveCookerName( "STUBCOOK-AREA" ) && "stubcook" == plugins.ResolveCookerName( "stubcook" ) && "other" == plugins.ResolveCookerName( "other" ) );
        ASSERT_FALSE( plugins.RegisterCookerAlias( "stubcook", "stubcook" ) );
        ASSERT_FALSE( plugins.RegisterCookerAlias( CORE::CString::Empty, "stubcook" ) );
        ASSERT_TRUE( plugins.UnregisterCookerAlias( "stubcook-area" ) && !plugins.UnregisterCookerAlias( "stubcook-area" ) );
        ASSERT_FALSE( plugins.TryGetCooker( "stubcook-area", cooker ) );
        ASSERT_TRUE( plugins.TryGetCooker( "stubcook", cooker ) );

        // a name is taken once; it can be taken again after it was given up
        ASSERT_FALSE( plugins.RegisterPostProcessor( TAssetPostProcessorPtr( GUCEF_NEW COrderPostProcessor( "OrderA" ) ) ) );
        ASSERT_TRUE( plugins.RegisterPostProcessor( TAssetPostProcessorPtr( GUCEF_NEW COrderPostProcessor( "OrderC" ) ) ) );
        TAssetPostProcessorPtr orderC;
        ASSERT_TRUE( plugins.TryGetPostProcessor( "OrderC", orderC ) );
        orderC.Unlink();
        ASSERT_TRUE( plugins.UnregisterPostProcessor( "OrderC" ) );
        ASSERT_FALSE( plugins.TryGetPostProcessor( "OrderC", orderC ) );
        ASSERT_FALSE( plugins.UnregisterPostProcessor( "OrderC" ) );

        GUCEF::vector< TAssetImporterPtr > importers;
        plugins.GetImporters( importers );
        bool hasStub = false;
        for ( size_t i=0; i<importers.size(); ++i )
            hasStub = hasStub || "stub" == importers[ i ]->GetName();
        ASSERT_TRUE( hasStub );

        // the default stack of an importer is the built-in processors, in order
        CORE::CString::StringVector stack;
        importer->GetDefaultPostProcessors( stack );
        ASSERT_TRUE( 4 == stack.size() && "NameHints" == stack[ 0 ] && "MergeIdenticalMaterials" == stack[ 3 ] );
        ASSERT_TRUE( CAssetConverterGlobal::GetConverterVersion() >= 1 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CanonicalJson: members sorted, round-trip numbers, escapes, the same text after a read through the JSON codec" )
    try
    {
        CORE::CDataNode node( "root", GUCEF_DATATYPE_OBJECT );
        node.SetAttribute( "zeta", "a\"b\nc" );
        node.SetAttribute( "alpha", (Int32) -3 );
        node.SetAttribute( "mid", true );
        CORE::CDataNode* list = node.AddChild( "list", GUCEF_DATATYPE_ARRAY );
        list->AddValueAsChild( CORE::CVariant( 0.1f ) );
        list->AddValueAsChild( CORE::CVariant( 1.0f ) );
        list->AddValueAsChild( CORE::CVariant( (Float64) 3.141592653589793 ) );
        list->AddValueAsChild( CORE::CVariant( (UInt32) 4000000000u ) );
        CORE::CDataNode* inner = node.AddChild( "beta", GUCEF_DATATYPE_OBJECT );
        inner->SetAttribute( "x", (Int32) 1 );
        node.AddChild( "empty", GUCEF_DATATYPE_ARRAY );

        const CORE::CString expected = "{\n"
                                       "    \"alpha\": -3,\n"
                                       "    \"beta\": {\n"
                                       "        \"x\": 1\n"
                                       "    },\n"
                                       "    \"empty\": [],\n"
                                       "    \"list\": [\n"
                                       "        0.1,\n"
                                       "        1,\n"
                                       "        3.141592653589793,\n"
                                       "        4000000000\n"
                                       "    ],\n"
                                       "    \"mid\": true,\n"
                                       "    \"zeta\": \"a\\\"b\\nc\"\n"
                                       "}\n";
        const CORE::CString text = CCanonicalJson::ToJson( node );
        ASSERT_TRUE( expected == text );

        // the shortest text that reads back as the same number
        ASSERT_TRUE( "0.1" == CCanonicalJson::FormatFloat32( 0.1f ) );
        ASSERT_TRUE( "0.1" == CCanonicalJson::FormatFloat64( 0.1 ) );
        ASSERT_TRUE( "1" == CCanonicalJson::FormatFloat32( 1.0f ) );
        ASSERT_TRUE( "-2.5" == CCanonicalJson::FormatFloat64( -2.5 ) );
        const Float32 third = 1.0f / 3.0f;
        ASSERT_TRUE( third == (Float32) atof( CCanonicalJson::FormatFloat32( third ).C_String() ) );
        ASSERT_TRUE( "1e-07" == CCanonicalJson::FormatFloat32( 1e-7f ) || "1e-7" == CCanonicalJson::FormatFloat32( 1e-7f ) );
        volatile Float32 zero = 0.0f;
        ASSERT_TRUE( "null" == CCanonicalJson::FormatFloat32( zero / zero ) );
        ASSERT_TRUE( "\"\\u0001x\"" == CCanonicalJson::QuoteString( CORE::CString( "\x01x" ) ) );

        // read through the codec and written again: the same text
        CORE::CDataNode parsed;
        ASSERT_TRUE( CCanonicalJson::Parse( text, parsed ) );
        ASSERT_TRUE( text == CCanonicalJson::ToJson( parsed ) );

        // the same tree, the same hash; another tree, another hash
        ASSERT_TRUE( CCanonicalJson::Hash( node ) == CCanonicalJson::Hash( parsed ) );
        inner->SetAttribute( "x", (Int32) 2 );
        ASSERT_TRUE( CCanonicalJson::Hash( node ) != CCanonicalJson::Hash( parsed ) );
        ASSERT_TRUE( 64 == CCanonicalJson::Hash( node ).Length() );

        // through a file of the VFS
        MakeWorkFolder( "json" );
        ASSERT_TRUE( CCanonicalJson::Store( "work/json/sub/folder/node.json", node ) );
        CORE::CDataNode loaded;
        ASSERT_TRUE( CCanonicalJson::Load( "work/json/sub/folder/node.json", loaded ) );
        ASSERT_TRUE( CCanonicalJson::ToJson( node ) == CCanonicalJson::ToJson( loaded ) );
        ASSERT_TRUE( CCanonicalJson::ToJson( node ) == ReadVfsText( "work/json/sub/folder/node.json" ) );
        ASSERT_TRUE( CContentHash::HashText( CCanonicalJson::ToJson( node ) ) == CContentHash::HashFile( "work/json/sub/folder/node.json" ) );
        ASSERT_TRUE( CContentHash::HashFile( "work/json/nosuchfile.json" ).IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "VfsPaths: normalising, joining and splitting VFS paths" )
    try
    {
        ASSERT_TRUE( "a/b/c" == CVfsPaths::Normalize( "/a//b\\c/" ) );
        ASSERT_TRUE( "a/b/c.x" == CVfsPaths::Join( "a/b/", "/c.x" ) );
        ASSERT_TRUE( "c.x" == CVfsPaths::Join( "", "c.x" ) );
        ASSERT_TRUE( "a/b" == CVfsPaths::FolderOf( "a/b/c.tar.gz" ) );
        ASSERT_TRUE( CVfsPaths::FolderOf( "c.x" ).IsNULLOrEmpty() );
        ASSERT_TRUE( "c.tar.gz" == CVfsPaths::NameOf( "a/b/c.tar.gz" ) );
        ASSERT_TRUE( "c.tar" == CVfsPaths::StemOf( "a/b/c.tar.gz" ) );
        ASSERT_TRUE( "gz" == CVfsPaths::ExtensionOf( "a/b/C.TAR.GZ" ) );
        ASSERT_TRUE( "a/b/c" == CVfsPaths::WithoutExtension( "a/b/c.fbx" ) );
        ASSERT_TRUE( CVfsPaths::ExtensionOf( "a/.hidden" ).IsNULLOrEmpty() );
        CORE::CString relative;
        ASSERT_TRUE( CVfsPaths::MakeRelative( "a/b", "a/b/c/d.x", relative ) && "c/d.x" == relative );
        ASSERT_FALSE( CVfsPaths::MakeRelative( "a/b", "a/bc/d.x", relative ) );
        ASSERT_FALSE( CVfsPaths::MakeRelative( "a/b", "a/b", relative ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Scene: removing nodes keeps the hierarchy, skins, animations, LOD groups and prefabs right" )
    try
    {
        CAssetScene scene;
        const UInt32 root = scene.AddNode( "Root" );
        const UInt32 a = scene.AddNode( "A", (Int32) root );
        const UInt32 b = scene.AddNode( "B", (Int32) a );
        const UInt32 c = scene.AddNode( "C", (Int32) root );
        ASSERT_TRUE( 1 == scene.rootNodes.size() && 2 == scene.nodes[ root ].children.size() && (Int32) a == scene.nodes[ b ].parent );
        ASSERT_TRUE( (Int32) c == scene.FindNode( "C" ) && -1 == scene.FindNode( "D" ) );
        ASSERT_TRUE( scene.IsInSubtree( b, root ) && !scene.IsInSubtree( c, a ) );

        CAssetSkin skin;
        skin.joints.push_back( a );
        skin.joints.push_back( b );
        skin.joints.push_back( c );
        for ( UInt32 j=0; j<3; ++j )
        {
            MATH::TFloat32Matrix4D matrix = MATH::TFloat32Matrix4D::Identity();
            matrix( 0, 3 ) = (Float32) j;
            skin.inverseBindMatrices.push_back( matrix );
        }
        scene.skins.push_back( skin );
        CAssetAnimation animation;
        CAssetAnimationChannel channel;
        channel.targetNode = b;
        animation.channels.push_back( channel );
        channel.targetNode = c;
        animation.channels.push_back( channel );
        scene.animations.push_back( animation );
        CAssetLodGroup group;
        group.AddLevel( a, 0.5f );
        group.AddLevel( c, 0.1f );
        scene.lodGroups.push_back( group );
        CAssetPrefab prefab;
        prefab.rootNode = root;
        CAssetPrefabInstance instance;
        instance.node = b;
        prefab.instances.push_back( instance );
        instance.node = c;
        prefab.instances.push_back( instance );
        scene.prefabs.push_back( prefab );
        ASSERT_TRUE( scene.IsJoint( b ) && scene.IsAnimationTarget( c ) && scene.IsLodLevel( a ) && scene.IsPrefabNode( root ) );

        CAssetScene::TUInt32Set removed;
        removed.insert( a );
        ASSERT_TRUE( 2 == scene.RemoveNodes( removed ) );
        ASSERT_TRUE( 2 == scene.nodes.size() && "Root" == scene.nodes[ 0 ].name && "C" == scene.nodes[ 1 ].name );
        ASSERT_TRUE( 1 == scene.nodes[ 0 ].children.size() && 1 == scene.nodes[ 0 ].children[ 0 ] && 0 == scene.nodes[ 1 ].parent );
        ASSERT_TRUE( 1 == scene.skins[ 0 ].joints.size() && 1 == scene.skins[ 0 ].joints[ 0 ] );
        ASSERT_TRUE( 1 == scene.skins[ 0 ].inverseBindMatrices.size() && 2.0f == scene.skins[ 0 ].inverseBindMatrices[ 0 ]( 0, 3 ) );
        ASSERT_TRUE( 1 == scene.animations[ 0 ].channels.size() && 1 == scene.animations[ 0 ].channels[ 0 ].targetNode );
        ASSERT_TRUE( 1 == scene.lodGroups[ 0 ].levels.size() && 1 == scene.lodGroups[ 0 ].levels[ 0 ].node && Near( 0.1f, scene.lodGroups[ 0 ].levels[ 0 ].screenRelativeHeight ) );
        ASSERT_TRUE( 1 == scene.prefabs.size() && 1 == scene.prefabs[ 0 ].instances.size() && 1 == scene.prefabs[ 0 ].instances[ 0 ].node );

        // meshes and materials no one uses go when asked; those in use stay with their references updated
        scene.meshes.resize( 3 );
        scene.meshes[ 2 ].subMeshes.resize( 1 );
        scene.meshes[ 2 ].subMeshes[ 0 ].materialIndex = 1;
        scene.nodes[ 1 ].mesh = 2;
        scene.materials.resize( 2 );
        ASSERT_TRUE( 2 == scene.RemoveUnusedMeshes() && 0 == scene.nodes[ 1 ].mesh );
        ASSERT_TRUE( 1 == scene.RemoveUnusedMaterials() && 0 == scene.meshes[ 0 ].subMeshes[ 0 ].materialIndex );

        // the world matrix combines the transforms of the parents
        scene.nodes[ 0 ].transform.position = MATH::TFloat32Transform3D::TPositionVector( 1.0f, 0.0f, 0.0f );
        scene.nodes[ 1 ].transform.position = MATH::TFloat32Transform3D::TPositionVector( 0.0f, 2.0f, 0.0f );
        const MATH::TFloat32Matrix4D world = scene.GetWorldMatrix( 1 );
        ASSERT_TRUE( Near( 1.0f, world( 0, 3 ) ) && Near( 2.0f, world( 1, 3 ) ) );

        // the summary counts what is there
        CAssetSceneSummary summary;
        summary.Compute( scene );
        ASSERT_TRUE( 2 == summary.nodeCount && 1 == summary.meshCount && 1 == summary.materialCount && 1 == summary.boneCount && 1 == summary.lodLevelCount );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Manifest: every member is read and paths resolve against the folder of the manifest" )
    try
    {
        MakeWorkFolder( "manifest" );
        ASSERT_TRUE( WriteWorkFile( "manifest/pack.assetpack.json",
            "{ \"id\": \"pack1\", \"title\": \"Pack One\", \"store\": \"Test store\", \"licenceRecord\": \"rec1\", \"licenceFolder\": \"licences\",\n"
            "  \"source\": \"src\", \"importer\": \"stub\", \"include\": [ \"**/*.stub\", \"textures/**\" ], \"exclude\": [ \"**/skip*\" ],\n"
            "  \"rules\": { \"lod\": { \"policy\": \"generate\" }, \"colliders\": { \"policy\": \"none\" }, \"unitsPerMetre\": 100 },\n"
            "  \"nameHints\": \"unreal\",\n"
            "  \"presets\": [ { \"match\": \"props/**\", \"settings\": { \"a\": 1 } }, { \"match\": \"**\", \"importer\": \"other\", \"settings\": { \"b\": 2 } } ],\n"
            "  \"overrides\": \"overrides\",\n"
            "  \"postProcessors\": { \"stub\": [ \"OrderB\", { \"name\": \"OrderA\", \"settings\": { \"tag\": \"x\" } } ], \"*\": [ \"PruneEmptyNodes\" ] },\n"
            "  \"exporter\": \"gltf\", \"intermediate\": \"/abs/inter\", \"output\": \"out\",\n"
            "  \"cookTargets\": [ \"stubcook\" ], \"cookProfiles\": [ { \"name\": \"desktop\", \"maxTextureSize\": 2048 }, { \"name\": \"web\", \"generateMips\": false } ] }" ) );

        CAssetPackManifest manifest;
        ASSERT_TRUE( manifest.Load( "work/manifest/pack.assetpack.json" ) );
        ASSERT_TRUE( "pack1" == manifest.id && "Pack One" == manifest.title && "Test store" == manifest.store && "stub" == manifest.importer );
        ASSERT_TRUE( "work/manifest" == manifest.manifestFolder );
        ASSERT_TRUE( "work/manifest/src" == manifest.GetSourcePath() );
        ASSERT_TRUE( "abs/inter" == manifest.GetIntermediatePath() );
        ASSERT_TRUE( "work/manifest/out" == manifest.GetOutputPath() );
        ASSERT_TRUE( "work/manifest/overrides" == manifest.GetOverridesPath() );
        ASSERT_TRUE( "work/manifest/licences/rec1.licence.json" == manifest.GetLicenceRecordPath() );

        ASSERT_TRUE( manifest.IsIncluded( "props/rock.stub" ) && manifest.IsIncluded( "rock.stub" ) && manifest.IsIncluded( "textures/a.png" ) );
        ASSERT_FALSE( manifest.IsIncluded( "props/skip_me.stub" ) );
        ASSERT_FALSE( manifest.IsIncluded( "readme.txt" ) );

        ASSERT_TRUE( 2 == manifest.presets.size() && "props/**" == manifest.presets[ 0 ].match && "other" == manifest.presets[ 1 ].importer );
        ASSERT_TRUE( manifest.presets[ 0 ].Matches( "props/a/b.stub", "stub" ) && !manifest.presets[ 0 ].Matches( "rock.stub", "stub" ) );
        ASSERT_FALSE( manifest.presets[ 1 ].Matches( "rock.stub", "stub" ) );
        ASSERT_TRUE( manifest.presets[ 1 ].Matches( "rock.stub", "OTHER" ) );

        CPluginStackEntry::TEntryVector stack;
        ASSERT_TRUE( manifest.GetPostProcessorStack( "stub", stack ) );
        ASSERT_TRUE( 2 == stack.size() && "OrderB" == stack[ 0 ].name && "OrderA" == stack[ 1 ].name && "x" == Attr( stack[ 1 ].settings, "tag" ) );
        ASSERT_TRUE( manifest.GetPostProcessorStack( "fbx", stack ) && 1 == stack.size() && "PruneEmptyNodes" == stack[ 0 ].name );

        CPluginStackEntry::TEntryVector validators;
        manifest.GetValidators( validators );
        ASSERT_TRUE( !manifest.hasValidators && 4 == validators.size() && "MissingTextures" == validators[ 0 ].name && "LayerPenetration" == validators[ 3 ].name );

        ASSERT_TRUE( "generate" == manifest.GetLodPolicy() && "none" == manifest.GetColliderPolicy() );
        CORE::CDataNode pack;
        manifest.GetPackSettings( pack );
        ASSERT_TRUE( "unreal" == Attr( pack, "nameHints" ) && 100 == pack.GetAttributeValueOrChildValueByName( "unitsPerMetre" ).AsInt32() );
        ASSERT_TRUE( 1 == manifest.cookTargets.size() && 2 == manifest.cookProfiles.size() );
        ASSERT_TRUE( 2048 == manifest.cookProfiles[ 0 ].maxTextureSize && manifest.cookProfiles[ 0 ].generateMips && !manifest.cookProfiles[ 1 ].generateMips );
        ASSERT_TRUE( "png" == manifest.cookProfiles[ 1 ].GetTextureFormat( TEXTUREUSAGE_NORMAL ) );

        // written and read again: the same manifest
        CORE::CDataNode written( "manifest", GUCEF_DATATYPE_OBJECT );
        manifest.Serialize( written );
        CAssetPackManifest again;
        ASSERT_TRUE( again.Deserialize( written ) );
        CORE::CDataNode rewritten( "manifest", GUCEF_DATATYPE_OBJECT );
        again.Serialize( rewritten );
        ASSERT_TRUE( CCanonicalJson::ToJson( written ) == CCanonicalJson::ToJson( rewritten ) );

        // a manifest needs an id
        CORE::CDataNode noId( "manifest", GUCEF_DATATYPE_OBJECT );
        ASSERT_FALSE( again.Deserialize( noId ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Licence records: read from JSON; ueOnly, referenceOnly and unreviewed restricted licences refuse" )
    try
    {
        MakeWorkFolder( "licence" );
        ASSERT_TRUE( WriteLicence( "licence", "ok", false, false, false, false ) );
        CLicenceRecord record;
        ASSERT_TRUE( record.Load( "work/licence/ok.licence.json" ) );
        ASSERT_TRUE( "ok" == record.id && "Test store" == record.store && 1 == record.seats );
        CORE::CString reason;
        ASSERT_TRUE( record.AllowsConversion( reason ) && reason.IsNULLOrEmpty() );

        record.ueOnly = true;
        ASSERT_FALSE( record.AllowsConversion( reason ) );
        ASSERT_TRUE( reason.HasSubstr( "ueOnly", 0 ) >= 0 );
        record.ueOnly = false;
        record.referenceOnly = true;
        ASSERT_FALSE( record.AllowsConversion( reason ) );
        ASSERT_TRUE( reason.HasSubstr( "referenceOnly", 0 ) >= 0 );
        record.referenceOnly = false;
        record.restrictedOrNonStandard = true;
        ASSERT_FALSE( record.AllowsConversion( reason ) );
        record.reviewed = true;
        ASSERT_TRUE( record.AllowsConversion( reason ) );
        record.humbleOneSeat = true;
        ASSERT_TRUE( record.AllowsConversion( reason ) );

        CORE::CDataNode noId( "licence", GUCEF_DATATYPE_OBJECT );
        ASSERT_FALSE( record.Deserialize( noId ) );
        ASSERT_FALSE( record.Load( "work/licence/missing.licence.json" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Settings: importer defaults, pack rules, presets top to bottom by glob and importer, the per-asset override last" )
    try
    {
        MakeWorkFolder( "settings" );
        ASSERT_TRUE( WriteWorkFile( "settings/overrides/props/rock.stub.import.json", "{ \"obj\": { \"x\": 7 }, \"final\": true }" ) );

        CORE::CDataNode defaults( "settings", GUCEF_DATATYPE_OBJECT );
        defaults.SetAttribute( "stubDefault", "default" );
        defaults.SetAttribute( "keep", (Int32) 1 );
        CORE::CDataNode* obj = defaults.AddChild( "obj", GUCEF_DATATYPE_OBJECT );
        obj->SetAttribute( "x", (Int32) 1 );
        obj->SetAttribute( "y", (Int32) 2 );

        CAssetPackManifest manifest;
        manifest.id = "settings";
        manifest.manifestFolder = "work/settings";
        manifest.overrides = "overrides";
        manifest.rules.AddChild( "obj", GUCEF_DATATYPE_OBJECT )->SetAttribute( "y", (Int32) 3 );
        CORE::CDataNode::TVariantVector list;
        list.push_back( CORE::CVariant( (Int32) 1 ) );
        list.push_back( CORE::CVariant( (Int32) 2 ) );
        manifest.rules.SetValuesOfChildByName( "list", list );
        CAssetPreset preset;
        preset.match = "props/**";
        preset.settings.SetAttribute( "keep", (Int32) 2 );
        list.clear();
        list.push_back( CORE::CVariant( (Int32) 9 ) );
        preset.settings.SetValuesOfChildByName( "list", list );
        manifest.presets.push_back( preset );
        preset = CAssetPreset();
        preset.match = "props/rock*";
        preset.settings.SetAttribute( "keep", (Int32) 3 );
        manifest.presets.push_back( preset );
        preset = CAssetPreset();
        preset.match = "**";
        preset.importer = "other";
        preset.settings.SetAttribute( "keep", (Int32) 99 );
        manifest.presets.push_back( preset );

        CORE::CDataNode settings;
        CORE::CString::StringVector layers;
        CImportSettingsBuilder::Build( defaults, manifest, "props/rock.stub", "stub", settings, layers );
        ASSERT_TRUE( 3 == settings.GetAttributeValueOrChildValueByName( "keep" ).AsInt32() );
        ASSERT_TRUE( "default" == Attr( settings, "stubDefault" ) );
        ASSERT_TRUE( settings.GetAttributeValueOrChildValueByName( "final" ).AsBool( false ) );
        const CORE::CDataNode* merged = settings.FindChild( "obj" );
        ASSERT_TRUE( GUCEF_NULL != merged && 7 == merged->GetAttributeValueOrChildValueByName( "x" ).AsInt32() && 3 == merged->GetAttributeValueOrChildValueByName( "y" ).AsInt32() );
        CORE::CDataNode::TVariantVector values;
        ASSERT_TRUE( settings.GetValuesOfChildByName( "list", values ) && 1 == values.size() && 9 == values[ 0 ].AsInt32() );
        ASSERT_TRUE( 5 == layers.size() && "defaults" == layers[ 0 ] && "pack" == layers[ 1 ] && "preset 0: props/**" == layers[ 2 ] && "preset 1: props/rock*" == layers[ 3 ] );
        ASSERT_TRUE( "override work/settings/overrides/props/rock.stub.import.json" == layers[ 4 ] );

        CImportSettingsBuilder::Build( defaults, manifest, "other/tree.stub", "stub", settings, layers );
        ASSERT_TRUE( 1 == settings.GetAttributeValueOrChildValueByName( "keep" ).AsInt32() && 2 == layers.size() );
        values.clear();
        ASSERT_TRUE( settings.GetValuesOfChildByName( "list", values ) && 2 == values.size() );
        CImportSettingsBuilder::Build( defaults, manifest, "other/tree.stub", "other", settings, layers );
        ASSERT_TRUE( 99 == settings.GetAttributeValueOrChildValueByName( "keep" ).AsInt32() );

        // a value replaces an object of the same name and the other way around
        CORE::CDataNode target( "settings", GUCEF_DATATYPE_OBJECT );
        target.AddChild( "a", GUCEF_DATATYPE_OBJECT )->SetAttribute( "x", (Int32) 1 );
        target.SetAttribute( "b", (Int32) 1 );
        CORE::CDataNode layer( "settings", GUCEF_DATATYPE_OBJECT );
        layer.SetAttribute( "a", (Int32) 5 );
        layer.AddChild( "b", GUCEF_DATATYPE_OBJECT )->SetAttribute( "y", (Int32) 2 );
        CImportSettingsBuilder::Merge( target, layer );
        ASSERT_TRUE( GUCEF_NULL == target.FindChild( "a" ) && 5 == target.GetAttributeValueOrChildValueByName( "a" ).AsInt32() );
        ASSERT_TRUE( GUCEF_NULL != target.FindChild( "b" ) && !target.IsAttribute( "b" ) );

        // the settings of a plugin: those of its entry, with its own child of the effective settings over them
        CORE::CDataNode effective( "settings", GUCEF_DATATYPE_OBJECT );
        effective.AddChild( "OrderA", GUCEF_DATATYPE_OBJECT )->SetAttribute( "tag", "b" );
        CORE::CDataNode entry( "settings", GUCEF_DATATYPE_OBJECT );
        entry.SetAttribute( "tag", "a" );
        entry.SetAttribute( "n", (Int32) 1 );
        CORE::CDataNode pluginSettings;
        CImportSettingsBuilder::GetPluginSettings( effective, entry, "OrderA", pluginSettings );
        ASSERT_TRUE( "b" == Attr( pluginSettings, "tag" ) && 1 == pluginSettings.GetAttributeValueOrChildValueByName( "n" ).AsInt32() );
        CImportSettingsBuilder::GetPluginSettings( effective, entry, "OrderB", pluginSettings );
        ASSERT_TRUE( "a" == Attr( pluginSettings, "tag" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Cache entries: the key covers source, settings, plugin versions and dependencies; the comparison names what changed" )
    try
    {
        CAssetCacheEntry stored;
        stored.assetPath = "props/rock.stub";
        stored.sourceHash = "s1";
        stored.settingsHash = "h1";
        stored.plugins[ "importer:stub" ] = 1;
        stored.plugins[ "converter:AssetConverterLib" ] = 1;
        stored.dependencies[ "work/pack/notes.txt" ] = "d1";
        CConversionReportFile file;
        file.path = "work/inter/props/rock.gltf";
        stored.files.push_back( file );
        stored.ComputeKey();
        ASSERT_TRUE( 64 == stored.key.Length() );

        CORE::CString detail;
        CAssetCacheEntry current = stored;
        ASSERT_TRUE( ACTIVITYREASON_UNCHANGED == stored.CompareWith( current, detail ) );
        current.sourceHash = "s2";
        ASSERT_TRUE( ACTIVITYREASON_SOURCE_CHANGED == stored.CompareWith( current, detail ) && current.GetKeyOfParts() != stored.key );
        current = stored;
        current.settingsHash = "h2";
        ASSERT_TRUE( ACTIVITYREASON_SETTINGS_CHANGED == stored.CompareWith( current, detail ) && current.GetKeyOfParts() != stored.key );
        current = stored;
        current.plugins[ "importer:stub" ] = 2;
        ASSERT_TRUE( ACTIVITYREASON_VERSION_CHANGED == stored.CompareWith( current, detail ) && "importer:stub 1 -> 2" == detail );
        current = stored;
        current.plugins[ "postProcessor:NameHints" ] = 1;
        ASSERT_TRUE( ACTIVITYREASON_VERSION_CHANGED == stored.CompareWith( current, detail ) && "postProcessor:NameHints added" == detail );
        current = stored;
        current.plugins.erase( "importer:stub" );
        ASSERT_TRUE( ACTIVITYREASON_VERSION_CHANGED == stored.CompareWith( current, detail ) && "importer:stub removed" == detail );
        current = stored;
        current.dependencies[ "work/pack/notes.txt" ] = "d2";
        ASSERT_TRUE( ACTIVITYREASON_DEPENDENCY_CHANGED == stored.CompareWith( current, detail ) && "work/pack/notes.txt" == detail && current.GetKeyOfParts() != stored.key );
        current = stored;
        current.files.clear();
        ASSERT_TRUE( ACTIVITYREASON_UNCHANGED == stored.CompareWith( current, detail ) );

        MakeWorkFolder( "cache" );
        const CORE::CString path = CAssetCacheEntry::GetEntryPath( "work/cache/inter", stored.assetPath );
        ASSERT_TRUE( "work/cache/inter/.cache/props/rock.stub.cache.json" == path );
        ASSERT_TRUE( stored.Save( path ) );
        CAssetCacheEntry loaded;
        ASSERT_TRUE( loaded.Load( path ) );
        ASSERT_TRUE( loaded.key == stored.key && loaded.GetKeyOfParts() == stored.key && 1 == loaded.files.size() && file.path == loaded.files[ 0 ].path );
        ASSERT_TRUE( ACTIVITYREASON_UNCHANGED == loaded.CompareWith( stored, detail ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Activity log: each run is appended with its UTC time, entries read back" )
    try
    {
        MakeWorkFolder( "activity" );
        const CORE::CString path = CImportActivityLog::GetLogPath( "work/activity" );
        ASSERT_TRUE( "work/activity/activity.json" == path );
        CImportActivityLog::SRun run;
        run.time = CImportActivityLog::GetUtcTimeText();
        run.packId = "pack";
        CImportActivityEntry entry;
        entry.assetPath = "a.stub";
        entry.action = ACTIVITYACTION_IMPORTED;
        entry.reason = ACTIVITYREASON_VERSION_CHANGED;
        entry.detail = "importer:stub 1 -> 2";
        entry.durationInMs = 12;
        run.entries.push_back( entry );
        ASSERT_TRUE( CImportActivityLog::AppendRun( path, run ) );
        run.entries[ 0 ].action = ACTIVITYACTION_CACHED;
        run.entries[ 0 ].reason = ACTIVITYREASON_UNCHANGED;
        run.entries[ 0 ].detail.Clear();
        ASSERT_TRUE( CImportActivityLog::AppendRun( path, run ) );

        CImportActivityLog log;
        ASSERT_TRUE( log.Load( path ) );
        ASSERT_TRUE( 2 == log.GetRuns().size() );
        const CImportActivityEntry& first = log.GetRuns()[ 0 ].entries[ 0 ];
        ASSERT_TRUE( "a.stub" == first.assetPath && ACTIVITYACTION_IMPORTED == first.action && ACTIVITYREASON_VERSION_CHANGED == first.reason && "importer:stub 1 -> 2" == first.detail && 12 == first.durationInMs );
        ASSERT_TRUE( ACTIVITYACTION_CACHED == log.GetRuns()[ 1 ].entries[ 0 ].action && "pack" == log.GetRuns()[ 1 ].packId );
        ASSERT_TRUE( log.GetRuns()[ 0 ].time.Length() >= 19 && log.GetRuns()[ 0 ].time.HasChar( 'T' ) > 0 );
        ASSERT_TRUE( ReadVfsText( path ).HasSubstr( "version changed", 0 ) > 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "References and sizes: the graph of the cooked files of a profile, exclusive and shared sizes per asset" )
    try
    {
        CConversionReport report;
        report.SelectAsset( "a.stub" );
        CORE::CString::StringVector references;
        references.push_back( "out/desktop/a.mat" );
        report.AddOutputFile( "out/desktop/a.mesh", references, "desktop" );
        report.AddOutputFile( "out/web/a.mesh", references, "web" );
        report.AddOutputFile( "inter/a.gltf", CORE::CString::StringVector() );
        references.clear();
        references.push_back( "out/desktop/a.tex" );
        references.push_back( "out/desktop/shared.tex" );
        report.AddOutputFile( "out/desktop/a.mat", references, "desktop" );
        report.SelectAsset( "b.stub" );
        references.clear();
        references.push_back( "out/desktop/b.mat" );
        report.AddOutputFile( "out/desktop/b.mesh", references, "desktop" );
        references.clear();
        references.push_back( "out/desktop/b.tex" );
        references.push_back( "out/desktop/shared.tex" );
        report.AddOutputFile( "out/desktop/b.mat", references, "desktop" );

        CReferenceGraph graph;
        graph.Build( report, "desktop", "out/desktop" );
        ASSERT_TRUE( 4 == graph.GetFiles().size() );
        CReferenceGraph::TFileMap::const_iterator mesh = graph.GetFiles().find( "a.mesh" );
        ASSERT_TRUE( mesh != graph.GetFiles().end() && "a.stub" == (*mesh).second.asset && 1 == (*mesh).second.references.size() && "a.mat" == (*mesh).second.references[ 0 ] );
        CORE::CString::StringSet closure;
        graph.GetClosure( "a.mesh", closure );
        ASSERT_TRUE( 4 == closure.size() && closure.find( "shared.tex" ) != closure.end() );

        CSizeReport::TFileSizeMap sizes;
        sizes[ "a.mesh" ] = 10; sizes[ "a.mat" ] = 5; sizes[ "a.tex" ] = 20; sizes[ "shared.tex" ] = 100;
        sizes[ "b.mesh" ] = 10; sizes[ "b.mat" ] = 5; sizes[ "b.tex" ] = 30;
        CSizeReport sizeReport;
        sizeReport.Build( graph, sizes );
        ASSERT_TRUE( 2 == sizeReport.GetAssets().size() && 7 == sizeReport.GetFileSizes().size() );
        const CSizeReport::SAsset& a = sizeReport.GetAssets().find( "a.stub" )->second;
        const CSizeReport::SAsset& b = sizeReport.GetAssets().find( "b.stub" )->second;
        ASSERT_TRUE( 35 == a.exclusiveSize && 100 == a.sharedSize && 4 == a.fileCount );
        ASSERT_TRUE( 45 == b.exclusiveSize && 100 == b.sharedSize );

        // written and read again
        MakeWorkFolder( "references" );
        ASSERT_TRUE( graph.Save( "work/references/references.json" ) && sizeReport.Save( "work/references/sizes.json" ) );
        CReferenceGraph graphAgain;
        CSizeReport sizesAgain;
        ASSERT_TRUE( graphAgain.Load( "work/references/references.json" ) && sizesAgain.Load( "work/references/sizes.json" ) );
        ASSERT_TRUE( "desktop" == graphAgain.GetProfile() && 4 == graphAgain.GetFiles().size() );
        ASSERT_TRUE( 35 == sizesAgain.GetAssets().find( "a.stub" )->second.exclusiveSize && 100 == sizesAgain.GetFileSizes().find( "shared.tex" )->second );
        ASSERT_TRUE( ReadVfsText( "work/references/sizes.json" ).HasSubstr( "\"inclusiveSize\": 135", 0 ) > 0 );

        // the other profile has its own graph
        CReferenceGraph web;
        web.Build( report, "web", "out/web" );
        ASSERT_TRUE( 1 == web.GetFiles().size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Relations: the importer's references become the normalized relations of the sidecar, paths in the pack, canonical, unresolved kept, dependencies by the safety net; the graph keeps them with the cooked files of their targets" )
    try
    {
        // the vocabulary and the closure rules
        ASSERT_TRUE( CAssetReference::IsKnownKind( "usesTexture" ) && CAssetReference::IsKnownKind( "usesFile" ) && !CAssetReference::IsKnownKind( "uses-texture" ) );
        ASSERT_TRUE( CAssetReference::IsFollowedKind( "usesMesh" ) && CAssetReference::IsFollowedKind( "dependsOn" ) );
        ASSERT_FALSE( CAssetReference::IsFollowedKind( "inScene" ) || CAssetReference::IsFollowedKind( "fitsSkeleton" ) || CAssetReference::IsFollowedKind( "somethingNew" ) );
        ASSERT_TRUE( CAssetReference::KindUsesFile == CAssetReference::GetKindOfDependency( "props/rock.stub.meta" ) && CAssetReference::KindDependsOn == CAssetReference::GetKindOfDependency( "props/rock.notes.txt" ) );
        ASSERT_TRUE( 8 == CAssetReference::MaxClosureDepth && 4096 == CAssetReference::MaxClosureFiles );

        const CORE::CString manifestPath = WriteStandardPack( "relations_model", CORE::CString::Empty );
        WriteWorkFile( "relations_model/src/props/rock.bin", "buffer" );
        CAssetPackManifest manifest;
        ASSERT_TRUE( manifest.Load( manifestPath ) );
        CAssetPackContext context;
        ASSERT_TRUE( context.Open( manifest ) );

        CAssetScene scene;
        const CORE::CString root = context.GetSourceRoot();
        scene.AddReference( CAssetReference::KindUsesTexture, CVfsPaths::Join( root, "props/rock.png" ), "rock.png" );
        scene.AddReference( CAssetReference::KindUsesTexture, CVfsPaths::Join( root, "props/rock.png" ), "rock.png" );    // once
        scene.AddReference( CAssetReference::KindUsesFile, CVfsPaths::Join( root, "props/rock.bin" ), "rock.bin" );
        scene.AddReference( CAssetReference::KindDependsOn, CVfsPaths::Join( root, "props/rock.stub" ), "self" );          // the asset itself
        scene.AddReference( CAssetReference::KindUsesMesh, "work/elsewhere/other.fbx", "other.fbx" );                     // not in the pack
        scene.AddReference( CAssetReference::KindDependsOn, CVfsPaths::Join( root, "readme.txt" ), "readme", true );
        scene.AddUnresolvedReference( CAssetReference::KindUsesTexture, "guid:0123456789abcdef" );
        scene.AddUnresolvedReference( CAssetReference::KindUsesTexture, CORE::CString::Empty );                         // nothing to keep
        scene.AddDependency( CVfsPaths::Join( root, "props/rock.png" ) );                                                 // recorded already
        scene.AddDependency( CVfsPaths::Join( root, "props/rock.notes.txt" ) );                                           // the safety net
        ASSERT_TRUE( 6 == scene.references.size() );

        CAssetReference::TReferenceVector relations;
        CConversionSidecar::BuildRelations( scene, context, "props/rock.stub", relations );
        ASSERT_TRUE( 5 == relations.size() );
        // canonical: by kind, then path, then via; the unresolved one (no path) first of its kind
        ASSERT_TRUE( "dependsOn" == relations[ 0 ].kind && "props/rock.notes.txt" == relations[ 0 ].target && relations[ 0 ].via.IsNULLOrEmpty() && !relations[ 0 ].optional );
        ASSERT_TRUE( "dependsOn" == relations[ 1 ].kind && "readme.txt" == relations[ 1 ].target && relations[ 1 ].optional );
        ASSERT_TRUE( "usesFile" == relations[ 2 ].kind && "props/rock.bin" == relations[ 2 ].target && "rock.bin" == relations[ 2 ].via );
        ASSERT_TRUE( "usesTexture" == relations[ 3 ].kind && !relations[ 3 ].IsResolved() && "guid:0123456789abcdef" == relations[ 3 ].via );
        ASSERT_TRUE( "usesTexture" == relations[ 4 ].kind && "props/rock.png" == relations[ 4 ].target );

        // the sidecar holds them, through canonical JSON and back
        CConversionReportEntry entry;
        entry.assetPath = "props/rock.stub";
        CORE::CDataNode sidecar;
        CConversionSidecar::Build( scene, entry, CORE::CString::StringVector(), "rec", context, "work/relations_model/intermediate", sidecar );
        const CORE::CString json = CCanonicalJson::ToJson( sidecar );
        ASSERT_TRUE( json.HasSubstr( "\"relations\"" ) > 0 && json.HasSubstr( "\"optional\": true" ) > 0 );
        CORE::CDataNode parsed;
        ASSERT_TRUE( CCanonicalJson::Parse( json, parsed ) );
        CAssetReference::TReferenceVector read;
        ASSERT_TRUE( CConversionSidecar::ReadRelations( parsed, read ) && read.size() == relations.size() );
        for ( size_t i=0; i<read.size(); ++i )
            ASSERT_TRUE( read[ i ] == relations[ i ] );
        ASSERT_TRUE( json == CCanonicalJson::ToJson( sidecar ) );
        CORE::CDataNode old( "sidecar", GUCEF_DATATYPE_OBJECT );
        ASSERT_TRUE( CConversionSidecar::ReadRelations( old, read ) && read.empty() );

        // a nested scene's dependencies and relations are taken over once
        CAssetScene outer;
        outer.MergeDependenciesOf( scene );
        outer.MergeDependenciesOf( scene );
        ASSERT_TRUE( outer.references.size() == scene.references.size() && outer.dependencies.size() == scene.dependencies.size() );

        // the graph of a profile keeps the relations with the cooked files of the targets that are converted assets
        CConversionReport report;
        report.SelectAsset( "props/rock.stub" );
        report.AddOutputFile( "out/desktop/rock.mesh", CORE::CString::StringVector(), "desktop" );
        report.SelectAsset( "props/crate.stub" );
        report.AddOutputFile( "out/desktop/crate.mesh", CORE::CString::StringVector(), "desktop" );
        report.AddOutputFile( "out/desktop/crate.scene", CORE::CString::StringVector(), "desktop" );
        CReferenceGraph graph;
        graph.Build( report, "desktop", "out/desktop" );
        CAssetReference::TReferenceVector rockRelations = relations;
        rockRelations.push_back( CAssetReference( CAssetReference::KindInstancesPrefab, "props/crate.stub", "crate", false ) );
        graph.SetRelations( "props/rock.stub", rockRelations );
        graph.SetRelations( "props/crate.stub", CAssetReference::TReferenceVector() );
        ASSERT_TRUE( 1 == graph.GetAssetRelations().size() );
        const CReferenceGraph::TRelationVector& listed = graph.GetAssetRelations().find( "props/rock.stub" )->second;
        ASSERT_TRUE( 6 == listed.size() );
        bool instance = false;
        for ( size_t i=0; i<listed.size(); ++i )
            if ( "instancesPrefab" == listed[ i ].relation.kind )
                instance = 2 == listed[ i ].files.size() && "crate.mesh" == listed[ i ].files[ 0 ] && "crate.scene" == listed[ i ].files[ 1 ];
            else
                ASSERT_TRUE( listed[ i ].files.empty() );
        ASSERT_TRUE( instance );
        MakeWorkFolder( "relations_graph" );
        ASSERT_TRUE( graph.Save( "work/relations_graph/references.json" ) );
        const CORE::CString written = ReadVfsText( "work/relations_graph/references.json" );
        ASSERT_TRUE( written.HasSubstr( "\"assets\"" ) > 0 && written.HasSubstr( "guid:0123456789abcdef" ) > 0 );
        CReferenceGraph again;
        ASSERT_TRUE( again.Load( "work/relations_graph/references.json" ) && 1 == again.GetAssetRelations().size() && 6 == again.GetAssetRelations().begin()->second.size() );
        CORE::CString::StringVector crateFiles;
        again.GetFilesOfAsset( "props/crate.stub", crateFiles );
        ASSERT_TRUE( 2 == crateFiles.size() );

        // a graph without relations is written as before: no member "assets"
        CReferenceGraph plain;
        plain.Build( report, "desktop", "out/desktop" );
        ASSERT_TRUE( plain.Save( "work/relations_graph/plain.json" ) && ReadVfsText( "work/relations_graph/plain.json" ).HasSubstr( "\"assets\"" ) < 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Report: statuses from the findings, counts, JSON round trip and the readable summary" )
    try
    {
        CConversionReport report;
        report.SetPackId( "pack" );
        report.SetLicenceRecordId( "rec" );
        report.SelectAsset( "converted.stub" );
        report.AddInfo( "test", "x", "fine" );
        report.SelectAsset( "approx.stub" );
        report.AddApproximation( "test", "roughness", "1 - smoothness" );
        report.SelectAsset( "failed.stub" );
        report.AddError( "test", "x", "broken" );
        report.SelectAsset( "skipped.stub" ).skipped = true;
        report.AddSkipped( "test", "y", "not wanted" );
        report.SelectAsset( "cached.stub" ).cached = true;
        ASSERT_TRUE( 1 == report.GetAssetCount( ASSETSTATUS_CONVERTED ) && 1 == report.GetAssetCount( ASSETSTATUS_APPROXIMATED ) );
        ASSERT_TRUE( 1 == report.GetAssetCount( ASSETSTATUS_FAILED ) && 1 == report.GetAssetCount( ASSETSTATUS_SKIPPED ) && 1 == report.GetAssetCount( ASSETSTATUS_CACHED ) );
        ASSERT_TRUE( report.HasFailures() && 1 == report.GetItemCount( REPORTITEM_ERROR ) );
        ASSERT_TRUE( GUCEF_NULL != report.FindEntry( "approx.stub" ) && GUCEF_NULL == report.FindEntry( "none.stub" ) );

        MakeWorkFolder( "report" );
        ASSERT_TRUE( report.SaveJson( "work/report/report.json" ) && report.SaveSummary( "work/report/report.txt" ) );
        CConversionReport loaded;
        ASSERT_TRUE( loaded.LoadJson( "work/report/report.json" ) );
        ASSERT_TRUE( "pack" == loaded.GetPackId() && "rec" == loaded.GetLicenceRecordId() && 5 == loaded.GetEntries().size() );
        ASSERT_TRUE( 1 == loaded.GetAssetCount( ASSETSTATUS_APPROXIMATED ) && 1 == loaded.GetAssetCount( ASSETSTATUS_CACHED ) && 1 == loaded.GetAssetCount( ASSETSTATUS_FAILED ) );
        const CORE::CString summary = ReadVfsText( "work/report/report.txt" );
        ASSERT_TRUE( summary.HasSubstr( "pack", 0 ) >= 0 && summary.HasSubstr( "1 - smoothness", 0 ) > 0 && summary.HasSubstr( "broken", 0 ) > 0 );
        ASSERT_TRUE( summary.HasSubstr( "fine", 0 ) < 0 );

        // a merge replaces the entries of the same asset
        CConversionReport other;
        other.SelectAsset( "failed.stub" );
        other.AddInfo( "test", "x", "fixed" );
        report.Merge( other );
        ASSERT_TRUE( 0 == report.GetAssetCount( ASSETSTATUS_FAILED ) && !report.HasFailures() );
        report.SetRefused( "no licence" );
        ASSERT_TRUE( report.IsRefused() && report.HasFailures() && "no licence" == report.GetRefusalReason() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
