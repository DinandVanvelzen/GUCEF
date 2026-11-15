/*
 *  assetpluginUNITY_TestApp: Test application for the Unity plugin of the asset converter (assetpluginUNITY)
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

#include <math.h>

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#include "assetpluginUNITY_CUnityImporter.h"
#include "assetpluginUNITY_CUnityProject.h"

#include "TestUnitySupport.h"
#include "TestUnityContent.h"
#include "TestUnityImport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETCONVERTER;
using namespace GUCEF::ASSETPLUGIN::UNITY;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

const char* const ProjectName = "import";

/** The source of the test project: the package or the folder */
CORE::CString
SourceOf( bool package )
{
    return package ? CORE::CString( "/work/import/test.unitypackage" ) : CORE::CString( "/work/import/project" );
}

/** Imports one asset of the test project the way the pipeline does: the index of the pack, the defaults of the settings, the import */
bool
ImportAsset( bool package, const CORE::CString& assetPath, CAssetScene& scene, CConversionReport& report, const CORE::CDataNode* rules = GUCEF_NULL )
{
    CAssetPackManifest manifest;
    if ( GUCEF_NULL != rules )
        manifest.rules = *rules;
    manifest.id = package ? "unitypackage" : "unityfolder";
    manifest.source = SourceOf( package );
    manifest.importer = "unity";
    manifest.intermediate = "/work/import/intermediate";
    CAssetPackContext context;
    if ( !context.Open( manifest ) )
        return false;
    CUnityImporter importer;
    if ( !importer.IndexPack( context ) )
        return false;
    scene = CAssetScene();
    scene.name = CVfsPaths::StemOf( assetPath );
    CORE::CDataNode settings( "settings", GUCEF_DATATYPE_OBJECT );
    importer.GetDefaultSettings( settings );
    scene.settings = settings;
    report.Clear();
    report.SelectAsset( assetPath );
    if ( !importer.CanImport( context.ToVfsPath( assetPath ), context ) )
        return false;
    return importer.Import( context.ToVfsPath( assetPath ), context, scene, report );
}

Int32
MeshOf( const CAssetScene& scene, const char* node )
{
    const Int32 index = scene.FindNode( node );
    return index >= 0 ? scene.nodes[ index ].mesh : -1;
}

Float32
MeshSize( const CAssetScene& scene, const char* node, UInt32 axis )
{
    const Int32 mesh = MeshOf( scene, node );
    if ( mesh < 0 )
        return -1.0f;
    Float32 min[ 3 ], max[ 3 ];
    GetMeshBounds( scene.meshes[ mesh ], min, max );
    return max[ axis ] - min[ axis ];
}

const CAssetMaterial*
MaterialOf( const CAssetScene& scene, const char* node )
{
    const Int32 mesh = MeshOf( scene, node );
    if ( mesh < 0 || scene.meshes[ mesh ].subMeshes.empty() )
        return GUCEF_NULL;
    const Int32 material = scene.meshes[ mesh ].subMeshes[ 0 ].materialIndex;
    return material >= 0 ? &scene.materials[ material ] : GUCEF_NULL;
}

const CAssetTexture*
TextureOf( const CAssetScene& scene, const CAssetTextureReference& reference )
{
    return reference.IsUsed() && (size_t) reference.textureIndex < scene.textures.size() ? &scene.textures[ reference.textureIndex ] : GUCEF_NULL;
}

bool
SourceEndsWith( const CAssetTexture* texture, const char* name )
{
    return GUCEF_NULL != texture && !texture->sources.empty() && texture->sources[ 0 ].EndsWith( name );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformUnityImportTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetpluginUNITY.Import" );

    bool written = false;
    GUCEF_TESTFW_TESTCASE( "Project: the synthetic project is written as a package and as a folder, and both are indexed by GUID" )
    try
    {
        written = WriteTestProject( ProjectName );
        ASSERT_TRUE( written );
        for ( UInt32 p=0; p<2; ++p )
        {
            CAssetPackManifest manifest;
            manifest.id = 0 == p ? "indexpackage" : "indexfolder";
            manifest.source = SourceOf( 0 == p );
            CAssetPackContext context;
            ASSERT_TRUE( context.Open( manifest ) );
            CUnityImporter importer;
            ASSERT_TRUE( importer.IndexPack( context ) );
            ASSERT_TRUE( context.GetIndexSize( CUnityProject::GuidIndexName ) >= 20 );
            CORE::CString path;
            ASSERT_TRUE( context.TryResolve( CUnityProject::GuidIndexName, GuidHousePrefab, path ) && path.EndsWith( "Assets/Prefabs/House.prefab" ) );
            ASSERT_TRUE( importer.CanImport( context.ToVfsPath( "Assets/Prefabs/House.prefab" ), context ) );
            ASSERT_TRUE( importer.CanImport( context.ToVfsPath( "Assets/Models/house.fbx" ), context ) );
            ASSERT_TRUE( importer.CanImport( context.ToVfsPath( "Assets/Scenes/Demo.unity" ), context ) );
            ASSERT_FALSE( importer.CanImport( context.ToVfsPath( "Assets/Materials/Standard.mat" ), context ) );
            ASSERT_FALSE( importer.CanImport( context.ToVfsPath( "Assets/Textures/albedo.png" ), context ) );
        }
        ASSERT_TRUE( "work/x" == CUnityImporter::GetProjectRoot( "/work/x/Assets/Prefabs" ) && "work/x" == CUnityImporter::GetProjectRoot( "/work/x" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
    if ( !written )
        return;

    GUCEF_TESTFW_TESTCASE( "Scale: a centimetre model with globalScale 100 is 16 metres wide, with globalScale 1 0.16 metres, before useFileScale 16, also after the normalisation" )
    try
    {
        CAssetScene scene;
        CConversionReport report;
        ASSERT_TRUE( ImportAsset( true, "Assets/Models/house.fbx", scene, report ) );
        Float32 min[ 3 ], max[ 3 ];
        ASSERT_TRUE( GetSceneBounds( scene, min, max ) );
        // Unity mirrors X: the house goes from -16 to 0 there
        ASSERT_TRUE( Near( min[ 0 ], -16.0f, 1e-3f ) && Near( max[ 0 ], 0.0f, 1e-3f ) && Near( max[ 1 ] - min[ 1 ], 16.0f, 1e-3f ) && Near( max[ 2 ] - min[ 2 ], 16.0f, 1e-3f ) );
        ASSERT_TRUE( MATH::CCoordinateSystem::Unity() == scene.coordinateSystem );
        ASSERT_TRUE( CAssetNormaliser::Normalise( scene, report ) );
        ASSERT_TRUE( GetSceneBounds( scene, min, max ) );
        ASSERT_TRUE( Near( max[ 0 ] - min[ 0 ], 16.0f, 1e-3f ) && Near( max[ 1 ] - min[ 1 ], 16.0f, 1e-3f ) && Near( max[ 2 ] - min[ 2 ], 16.0f, 1e-3f ) );

        CAssetScene small;
        ASSERT_TRUE( ImportAsset( false, "Assets/Models/house_small.fbx", small, report ) );
        ASSERT_TRUE( GetSceneBounds( small, min, max ) && Near( max[ 1 ] - min[ 1 ], 0.16f, 1e-5f ) );
        // before useFileScale the units of the file are Unity units
        CAssetScene old;
        ASSERT_TRUE( ImportAsset( true, "Assets/Models/house_old.fbx", old, report ) );
        ASSERT_TRUE( GetSceneBounds( old, min, max ) && Near( max[ 1 ] - min[ 1 ], 16.0f, 1e-3f ) );
        // no materials imported: Unity's default material, the texture path of the file is not followed
        ASSERT_TRUE( 1 == small.materials.size() && "Default-Material" == small.materials[ 0 ].name && small.textures.empty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Prefab (older generation): hierarchy, meshes by the IDs of the .meta with duplicate names, built-ins and unresolved references" )
    try
    {
        CAssetScene scene;
        CConversionReport report;
        ASSERT_TRUE( ImportAsset( true, "Assets/Prefabs/House.prefab", scene, report ) );
        ASSERT_TRUE( 11 == scene.nodes.size() && 1 == scene.rootNodes.size() && "House" == scene.nodes[ scene.rootNodes[ 0 ] ].name );
        ASSERT_TRUE( 1 == scene.prefabs.size() && scene.prefabs[ 0 ].rootNode == scene.rootNodes[ 0 ] && scene.prefabs[ 0 ].instances.empty() );
        const Int32 house = scene.FindNode( "House" );
        ASSERT_TRUE( house >= 0 && 8 == scene.nodes[ house ].children.size() );
        const Int32 windowA = scene.FindNode( "WindowA" );
        ASSERT_TRUE( windowA >= 0 && Near( scene.nodes[ windowA ].transform.position.x, -5.0f ) && scene.nodes[ windowA ].parent == house );

        // the meshes: 4300004 and 4300006 are both "Window", told apart by the number
        ASSERT_TRUE( Near( MeshSize( scene, "House", 1 ), 16.0f, 1e-3f ) && Near( MeshSize( scene, "Door", 1 ), 2.0f, 1e-3f ) );
        ASSERT_TRUE( Near( MeshSize( scene, "WindowA", 1 ), 1.0f, 1e-3f ) && Near( MeshSize( scene, "WindowB", 1 ), 3.0f, 1e-3f ) );
        ASSERT_TRUE( 24 == scene.meshes[ MeshOf( scene, "Primitive" ) ].GetVertexCount() );
        ASSERT_TRUE( MeshOf( scene, "Lost" ) < 0 && CountItems( report, REPORTITEM_WARNING, "99999999999999999999999999999999" ) >= 1 );
        ASSERT_TRUE( CountItems( report, REPORTITEM_WARNING, "built-in default material" ) >= 1 );
        const Int32 primitive = scene.FindNode( "Primitive" );
        ASSERT_TRUE( primitive >= 0 && !scene.nodes[ primitive ].extras.GetAttributeValueOrChildValueByName( "active" ).AsBool( true, true ) );
        ASSERT_TRUE( 0 == report.GetItemCount( REPORTITEM_ERROR ) );

        // the folder gives the same
        CAssetScene folder;
        ASSERT_TRUE( ImportAsset( false, "Assets/Prefabs/House.prefab", folder, report ) );
        ASSERT_TRUE( folder.nodes.size() == scene.nodes.size() && folder.meshes.size() == scene.meshes.size() && folder.materials.size() == scene.materials.size() &&
                     folder.textures.size() == scene.textures.size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Materials: Standard with its maps, keywords and alpha mode; the colour space by slot; tiling as texture transform" )
    try
    {
        CAssetScene scene;
        CConversionReport report;
        ASSERT_TRUE( ImportAsset( true, "Assets/Prefabs/House.prefab", scene, report ) );
        const CAssetMaterial* standard = MaterialOf( scene, "House" );
        ASSERT_TRUE( GUCEF_NULL != standard );
        if ( GUCEF_NULL != standard )
        {
            ASSERT_TRUE( "Standard_Painted" == standard->name && "Standard" == standard->source.shaderName );
            const CAssetTexture* base = TextureOf( scene, standard->baseColorTexture );
            ASSERT_TRUE( SourceEndsWith( base, "albedo.png" ) && TEXTUREUSAGE_COLOUR == base->usage && IMAGE::COLOURSPACE_SRGB == base->GetColourSpace() );
            ASSERT_TRUE( standard->baseColorTexture.hasTransform && Near( standard->baseColorTexture.tiling[ 0 ], 2.0f ) && Near( standard->baseColorTexture.offset[ 0 ], 0.5f ) &&
                         Near( standard->baseColorTexture.offset[ 1 ], -1.0f ) );
            ASSERT_TRUE( IMAGE::WRAPMODE_CLAMP == base->wrapS );
            // the metallic map is flagged sRGB by its importer; the slot makes it linear data, repacked
            const CAssetTexture* metal = TextureOf( scene, standard->metallicRoughnessTexture );
            ASSERT_TRUE( SourceEndsWith( metal, "metal.png" ) && TEXTUREUSAGE_MASK == metal->usage && IMAGE::COLOURSPACE_LINEAR == metal->GetColourSpace() );
            ASSERT_TRUE( GUCEF_NULL != metal && 2 == metal->operations.size() && TEXTUREOPERATION_PACK == metal->operations[ 0 ].type &&
                         TEXTUREOPERATION_SCALEBIAS == metal->operations[ 1 ].type && Near( metal->operations[ 1 ].scale, -0.5f ) );
            ASSERT_TRUE( 1.0f == standard->metallicFactor && 1.0f == standard->roughnessFactor );
            ASSERT_TRUE( CountItems( report, REPORTITEM_INFO, "the slot decides" ) >= 1 );
            const CAssetTexture* normal = TextureOf( scene, standard->normalTexture );
            ASSERT_TRUE( SourceEndsWith( normal, "normal.png" ) && TEXTUREUSAGE_NORMAL == normal->usage && Near( standard->normalTexture.scale, 1.5f ) );
            // the Standard shader samples every map with the tiling and offset of _MainTex
            ASSERT_TRUE( standard->normalTexture.hasTransform && Near( standard->normalTexture.tiling[ 0 ], 2.0f ) && standard->metallicRoughnessTexture.hasTransform &&
                         standard->emissiveTexture.hasTransform && Near( standard->emissiveTexture.offset[ 0 ], 0.5f ) );
            // emission with the keyword: HDR colour (2, 1, 0) as factor and strength
            ASSERT_TRUE( SourceEndsWith( TextureOf( scene, standard->emissiveTexture ), "emission.png" ) && standard->emissiveStrength > 4.0f && Near( standard->emissiveFactor[ 0 ], 1.0f ) );
            ASSERT_TRUE( ALPHAMODE_MASK == standard->alphaMode && Near( standard->alphaCutoff, 0.3f ) );
        }
        // without the _EMISSION keyword the emission colour is not used; mode 3 blends
        const CAssetMaterial* noKeyword = MaterialOf( scene, "LOD1" );
        ASSERT_TRUE( GUCEF_NULL != noKeyword && "NoKeyword" == noKeyword->name && 0.0f == noKeyword->emissiveFactor[ 0 ] && ALPHAMODE_BLEND == noKeyword->alphaMode );
        ASSERT_TRUE( CountItems( report, REPORTITEM_INFO, "without the _EMISSION keyword" ) >= 1 );
        ASSERT_TRUE( GUCEF_NULL != noKeyword && Near( noKeyword->roughnessFactor, 1.0f ) && Near( noKeyword->baseColorFactor[ 0 ], 0.214f, 1e-3f ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Materials: a custom shader by its declared properties, a shader graph, a missing shader, the default material" )
    try
    {
        CAssetScene scene;
        CConversionReport report;
        ASSERT_TRUE( ImportAsset( true, "Assets/Prefabs/House.prefab", scene, report ) );
        // the custom shader: _Texture is the base colour (not the left-over _MainTex), _Emission the emissive map, _Opacity a colour, alpha:fade blends
        const CAssetMaterial* custom = MaterialOf( scene, "Door" );
        ASSERT_TRUE( GUCEF_NULL != custom );
        if ( GUCEF_NULL != custom )
        {
            ASSERT_TRUE( "Test/Triplanar" == custom->source.shaderName );
            ASSERT_TRUE( SourceEndsWith( TextureOf( scene, custom->baseColorTexture ), "grid.png" ) );
            ASSERT_TRUE( SourceEndsWith( TextureOf( scene, custom->emissiveTexture ), "emission.png" ) && Near( custom->emissiveFactor[ 1 ], 1.0f ) );
            ASSERT_TRUE( ALPHAMODE_BLEND == custom->alphaMode && Near( custom->baseColorFactor[ 3 ], 0.214f, 1e-3f ) );
            ASSERT_TRUE( CountItems( report, REPORTITEM_SKIPPED, "_Overlay" ) >= 1 && CountItems( report, REPORTITEM_APPROXIMATION, "triplanar" ) >= 1 );
            ASSERT_TRUE( CountItems( report, REPORTITEM_INFO, "_Tiling" ) >= 1 );
        }
        const CAssetMaterial* graph = MaterialOf( scene, "WindowB" );
        ASSERT_TRUE( GUCEF_NULL != graph && "Shader Graphs/Grid" == graph->source.shaderName && SourceEndsWith( TextureOf( scene, graph->baseColorTexture ), "grid.png" ) );
        const CAssetMaterial* missing = MaterialOf( scene, "WindowA" );
        ASSERT_TRUE( GUCEF_NULL != missing && SourceEndsWith( TextureOf( scene, missing->baseColorTexture ), "albedo.png" ) && Near( missing->roughnessFactor, 0.75f ) );
        ASSERT_TRUE( CountItems( report, REPORTITEM_WARNING, "12345678901234567890123456789012" ) >= 1 );
        const CAssetMaterial* fallback = MaterialOf( scene, "Primitive" );
        ASSERT_TRUE( GUCEF_NULL != fallback && "Default-Material" == fallback->name );
        // every original property is kept
        ASSERT_TRUE( GUCEF_NULL != custom && GUCEF_NULL != custom->source.properties.FindChild( "texEnvs" ) && GUCEF_NULL != custom->source.properties.FindChild( "texEnvs" )->FindChild( "_MainTex" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Colliders, LOD group and light: box, sphere, capsule along X, a convex mesh of a generated .asset" )
    try
    {
        CAssetScene scene;
        CConversionReport report;
        ASSERT_TRUE( ImportAsset( true, "Assets/Prefabs/House.prefab", scene, report ) );
        const Int32 house = scene.FindNode( "House" );
        ASSERT_TRUE( house >= 0 && 1 == scene.nodes[ house ].colliders.size() );
        if ( house >= 0 && 1 == scene.nodes[ house ].colliders.size() )
        {
            const CAssetCollider& box = scene.nodes[ house ].colliders[ 0 ];
            ASSERT_TRUE( COLLIDERSHAPE_BOX == box.shape && Near( box.halfExtents[ 0 ], 8.0f ) && Near( box.transform.position.x, -8.0f ) && !box.trigger );
        }
        const Int32 door = scene.FindNode( "Door" );
        ASSERT_TRUE( door >= 0 && 1 == scene.nodes[ door ].colliders.size() && COLLIDERSHAPE_SPHERE == scene.nodes[ door ].colliders[ 0 ].shape &&
                     Near( scene.nodes[ door ].colliders[ 0 ].radius, 0.5f ) && scene.nodes[ door ].colliders[ 0 ].trigger );
        const Int32 window = scene.FindNode( "WindowB" );
        ASSERT_TRUE( window >= 0 && 1 == scene.nodes[ window ].colliders.size() && COLLIDERSHAPE_CAPSULE == scene.nodes[ window ].colliders[ 0 ].shape &&
                     Near( scene.nodes[ window ].colliders[ 0 ].height, 2.0f ) && Near( fabsf( scene.nodes[ window ].colliders[ 0 ].transform.rotation.z ), 0.7071f ) );
        const Int32 collision = scene.FindNode( "Collision" );
        ASSERT_TRUE( collision >= 0 && 1 == scene.nodes[ collision ].colliders.size() );
        if ( collision >= 0 && 1 == scene.nodes[ collision ].colliders.size() )
        {
            const CAssetCollider& convex = scene.nodes[ collision ].colliders[ 0 ];
            ASSERT_TRUE( COLLIDERSHAPE_CONVEX == convex.shape && convex.meshIndex >= 0 );
            if ( convex.meshIndex >= 0 )
            {
                ASSERT_TRUE( 4 == scene.meshes[ convex.meshIndex ].GetVertexCount() && 4 == scene.meshes[ convex.meshIndex ].GetTriangleCount() );
                ASSERT_TRUE( 1.0f == scene.meshes[ convex.meshIndex ].subMeshes[ 0 ].positions[ 3 ] );
            }
        }
        const Int32 lods = scene.FindNode( "Lods" );
        ASSERT_TRUE( lods >= 0 && scene.nodes[ lods ].lodGroup >= 0 );
        if ( lods >= 0 && scene.nodes[ lods ].lodGroup >= 0 )
        {
            const CAssetLodGroup& group = scene.lodGroups[ scene.nodes[ lods ].lodGroup ];
            ASSERT_TRUE( 2 == group.levels.size() && (Int32) group.levels[ 0 ].node == scene.FindNode( "LOD0" ) && Near( group.levels[ 0 ].screenRelativeHeight, 0.5f ) &&
                         Near( group.levels[ 1 ].screenRelativeHeight, 0.1f ) );
        }
        const Int32 lamp = scene.FindNode( "Lamp" );
        ASSERT_TRUE( lamp >= 0 && scene.nodes[ lamp ].light >= 0 && LIGHTTYPE_POINT == scene.lights[ scene.nodes[ lamp ].light ].type &&
                     Near( scene.lights[ scene.nodes[ lamp ].light ].range, 12.0f ) && Near( scene.lights[ scene.nodes[ lamp ].light ].intensity, 2.0f ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Prefab instance (2018.3 and later): modifications applied, a removed component, an added child, URP Lit, the record of the instance" )
    try
    {
        CAssetScene scene;
        CConversionReport report;
        ASSERT_TRUE( ImportAsset( true, "Assets/Prefabs/Street.prefab", scene, report ) );
        const Int32 street = scene.FindNode( "Street" );
        const Int32 house = scene.FindNode( "HouseA" );
        ASSERT_TRUE( street >= 0 && house >= 0 && scene.nodes[ house ].parent == street && Near( scene.nodes[ house ].transform.position.x, 5.0f ) );
        ASSERT_TRUE( house >= 0 && scene.nodes[ house ].colliders.empty() );
        const Int32 sign = scene.FindNode( "Sign" );
        ASSERT_TRUE( sign >= 0 && scene.nodes[ sign ].parent == house && Near( scene.nodes[ sign ].transform.position.y, 3.0f ) );
        ASSERT_TRUE( scene.FindNode( "Door" ) >= 0 && scene.nodes[ scene.FindNode( "Door" ) ].parent == house );

        const CAssetMaterial* urp = MaterialOf( scene, "HouseA" );
        ASSERT_TRUE( GUCEF_NULL != urp && "Urp_Glass" == urp->name && "Universal Render Pipeline/Lit" == urp->source.shaderName );
        if ( GUCEF_NULL != urp )
        {
            ASSERT_TRUE( ALPHAMODE_BLEND == urp->alphaMode && urp->doubleSided && Near( urp->roughnessFactor, 0.2f ) && Near( urp->baseColorFactor[ 3 ], 0.5f ) );
            ASSERT_TRUE( SourceEndsWith( TextureOf( scene, urp->baseColorTexture ), "albedo.png" ) );
        }

        ASSERT_TRUE( 1 == scene.prefabs.size() && 1 == scene.prefabs[ 0 ].instances.size() );
        if ( 1 == scene.prefabs.size() && 1 == scene.prefabs[ 0 ].instances.size() )
        {
            const CAssetPrefabInstance& instance = scene.prefabs[ 0 ].instances[ 0 ];
            ASSERT_TRUE( (Int32) instance.node == house && "Assets/Prefabs/House.prefab" == instance.sourcePrefab );
            CORE::CString::StringVector materials;
            instance.overrides.GetValuesOfChildByName( "materials", materials );
            ASSERT_TRUE( 1 == materials.size() && "Urp_Glass" == materials[ 0 ] );
            const CORE::CDataNode* modifications = instance.overrides.FindChild( "modifications" );
            ASSERT_TRUE( GUCEF_NULL != modifications && 4 == modifications->GetNrOfDirectChildNodes() );
            ASSERT_TRUE( 1 == instance.overrides.GetAttributeValueOrChildValueByName( "unapplied" ).AsInt32( 0, true ) );
        }
        ASSERT_TRUE( CountItems( report, REPORTITEM_WARNING, "modification(s) of the prefab instance" ) >= 1 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Scene: an instance of the older generation with its modifications, scene settings, a directional light" )
    try
    {
        CAssetScene scene;
        CConversionReport report;
        ASSERT_TRUE( ImportAsset( false, "Assets/Scenes/Demo.unity", scene, report ) );
        ASSERT_TRUE( 1 == scene.rootNodes.size() && "Demo" == scene.nodes[ scene.rootNodes[ 0 ] ].name );
        const Int32 house = scene.FindNode( "House" );
        ASSERT_TRUE( house >= 0 && scene.nodes[ house ].parent == (Int32) scene.rootNodes[ 0 ] && Near( scene.nodes[ house ].transform.position.z, 3.0f ) &&
                     Near( scene.nodes[ house ].transform.rotation.y, 0.7071f ) );
        const Int32 sun = scene.FindNode( "Sun" );
        ASSERT_TRUE( sun >= 0 && scene.nodes[ sun ].light >= 0 && LIGHTTYPE_DIRECTIONAL == scene.lights[ scene.nodes[ sun ].light ].type );
        ASSERT_TRUE( 1 == scene.prefabs.size() && 1 == scene.prefabs[ 0 ].instances.size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Later ID table: a prefab that names its mesh by an internalIDToNameTable ID" )
    try
    {
        CAssetScene scene;
        CConversionReport report;
        ASSERT_TRUE( ImportAsset( true, "Assets/Prefabs/NewHouse.prefab", scene, report ) );
        // globalScale .00999999978 over a centimetre file
        ASSERT_TRUE( MeshOf( scene, "NewHouse" ) >= 0 && Near( MeshSize( scene, "NewHouse", 1 ), 0.0016f, 1e-6f ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Overlay packages: a package inside the pack named by the rules replaces the assets of its GUIDs" )
    try
    {
        CORE::CDataNode rules;
        ASSERT_TRUE( CCanonicalJson::Parse( "{ \"unity\": { \"overlayPackages\": [ \"**/URP_ExtractMe.unitypackage\" ] } }", rules ) );
        CAssetScene scene;
        CConversionReport report;
        ASSERT_TRUE( ImportAsset( true, "Assets/Prefabs/House.prefab", scene, report, &rules ) );
        const CAssetMaterial* material = MaterialOf( scene, "House" );
        ASSERT_TRUE( GUCEF_NULL != material && "Standard_Urp" == material->name && "Universal Render Pipeline/Lit" == material->source.shaderName );
        ASSERT_TRUE( GUCEF_NULL != material && SourceEndsWith( TextureOf( scene, material->baseColorTexture ), "grid.png" ) );
        // without the rule the material of the pack itself
        CAssetScene plain;
        ASSERT_TRUE( ImportAsset( true, "Assets/Prefabs/House.prefab", plain, report ) );
        ASSERT_TRUE( GUCEF_NULL != MaterialOf( plain, "House" ) && "Standard_Painted" == MaterialOf( plain, "House" )->name );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Later ID scheme: an instance of a model through the IDs of its root objects, meshes named by computed IDs" )
    try
    {
        CAssetScene scene;
        CConversionReport report;
        ASSERT_TRUE( ImportAsset( true, "Assets/Prefabs/Crate.prefab", scene, report ) );
        const Int32 crate = scene.FindNode( "CrateA" );
        ASSERT_TRUE( crate >= 0 && Near( scene.nodes[ crate ].transform.position.x, 2.0f ) && Near( MeshSize( scene, "CrateA", 1 ), 1.0f, 1e-4f ) );
        const CAssetMaterial* material = MaterialOf( scene, "CrateA" );
        ASSERT_TRUE( GUCEF_NULL != material && "Standard_Painted" == material->name );
        ASSERT_TRUE( 1 == scene.prefabs.size() && 1 == scene.prefabs[ 0 ].instances.size() && "Assets/Models/crate.fbx" == scene.prefabs[ 0 ].instances[ 0 ].sourcePrefab );
        if ( 1 == scene.prefabs.size() && 1 == scene.prefabs[ 0 ].instances.size() )
        {
            ASSERT_TRUE( 0 == scene.prefabs[ 0 ].instances[ 0 ].overrides.GetAttributeValueOrChildValueByName( "unapplied" ).AsInt32( 1, true ) );
            CORE::CString::StringVector materials;
            scene.prefabs[ 0 ].instances[ 0 ].overrides.GetValuesOfChildByName( "materials", materials );
            ASSERT_TRUE( 1 == materials.size() && "Standard_Painted" == materials[ 0 ] );
        }

        // a computed mesh ID of a model with two meshes: the mesh of the node with the name of the GameObject
        CAssetScene pair;
        ASSERT_TRUE( ImportAsset( true, "Assets/Prefabs/Pair.prefab", pair, report ) );
        ASSERT_TRUE( Near( MeshSize( pair, "Right", 1 ), 2.0f, 1e-4f ) && CountItems( report, REPORTITEM_APPROXIMATION, "computed from names" ) >= 1 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Clips: the take of a model split by clipAnimations into named clips with their loop flags and IDs" )
    try
    {
        CAssetScene scene;
        CConversionReport report;
        ASSERT_TRUE( ImportAsset( true, "Assets/Models/mover.fbx", scene, report ) );
        ASSERT_TRUE( 2 == scene.animations.size() );
        if ( 2 == scene.animations.size() )
        {
            ASSERT_TRUE( "Open" == scene.animations[ 0 ].name && "Close" == scene.animations[ 1 ].name && "Take 001" == scene.animations[ 0 ].sourceTake );
            ASSERT_TRUE( Near( scene.animations[ 0 ].GetDuration(), 1.0f, 1e-3f ) && Near( scene.animations[ 1 ].GetDuration(), 1.0f, 1e-3f ) );
            ASSERT_TRUE( scene.animations[ 0 ].loop && !scene.animations[ 1 ].loop );
            // the second clip starts where the first ends: at 50 file units, 0.5 m with the scale of 0.01, mirrored on X
            const CAssetAnimationChannel& channel = scene.animations[ 1 ].channels[ 0 ];
            ASSERT_TRUE( ANIMATIONPATH_TRANSLATION == channel.path && Near( fabsf( channel.values[ 0 ] ), 0.5f, 1e-3f ) );
        }
        ASSERT_TRUE( CountItems( report, REPORTITEM_SKIPPED, "animation event" ) >= 1 );
        const CORE::CDataNode* facts = scene.extras.FindChild( "unity" );
        ASSERT_TRUE( GUCEF_NULL != facts && GUCEF_NULL != facts->FindChild( "clips" ) && 2 == facts->FindChild( "clips" )->GetNrOfDirectChildNodes() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Animation clips (.anim): exact Hermite rotations as cubic splines, stepped positions, sampled float curves, muscle curves reported" )
    try
    {
        CAssetScene scene;
        CConversionReport report;
        ASSERT_TRUE( ImportAsset( true, "Assets/Animation/Spin.anim", scene, report ) );
        ASSERT_TRUE( 1 == scene.animations.size() && scene.FindNode( "Arm" ) >= 0 && scene.FindNode( "Hand" ) >= 0 );
        if ( 1 == scene.animations.size() )
        {
            const CAssetAnimation& clip = scene.animations[ 0 ];
            ASSERT_TRUE( "Spin" == clip.name && clip.loop && 3 == clip.channels.size() );
            UInt32 checked = 0;
            for ( size_t c=0; c<clip.channels.size(); ++c )
            {
                const CAssetAnimationChannel& channel = clip.channels[ c ];
                if ( ANIMATIONPATH_ROTATION == channel.path )
                {
                    ASSERT_TRUE( ANIMATIONINTERPOLATION_CUBICSPLINE == channel.interpolation && 2 == channel.times.size() && 24 == channel.values.size() );
                    ASSERT_TRUE( Near( channel.values[ 9 ], 0.785398f ) );    // the out tangent of the first key
                    ++checked;
                }
                else
                if ( ANIMATIONPATH_TRANSLATION == channel.path )
                {
                    ASSERT_TRUE( ANIMATIONINTERPOLATION_STEP == channel.interpolation && 6 == channel.values.size() && (Int32) channel.targetNode == scene.FindNode( "Hand" ) );
                    ++checked;
                }
                else
                if ( ANIMATIONPATH_SCALE == channel.path )
                {
                    ASSERT_TRUE( ANIMATIONINTERPOLATION_LINEAR == channel.interpolation && 31 == channel.times.size() && Near( channel.values.back() , 1.0f ) && Near( channel.values[ channel.values.size() - 3 ], 2.0f ) );
                    ++checked;
                }
            }
            ASSERT_TRUE( 3 == checked );
        }
        ASSERT_TRUE( CountItems( report, REPORTITEM_SKIPPED, "humanoid muscle" ) >= 1 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
