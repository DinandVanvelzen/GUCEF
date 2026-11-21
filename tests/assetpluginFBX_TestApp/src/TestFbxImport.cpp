/*
 *  assetpluginFBX_TestApp: Test application for the FBX plugin of the asset converter (assetpluginFBX)
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
#include <string.h>

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#include "TestFbxSupport.h"
#include "TestFbxWriter.h"
#include "TestFbxImport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/* near and far are macros of the Windows headers, undefine them so they can be used as identifiers */
#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif

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
#define REQUIRE(t)      do { if ( !( t ) ) { GUCEF_TESTFW_ASSERT_TRUE( t ); throw 1; } } while ( 0 )

namespace {

CORE::CString
Base64( const CORE::CDynamicBuffer& bytes )
{
    static const char* const alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const UInt8* data = static_cast< const UInt8* >( bytes.GetConstBufferPtr() );
    const UInt32 size = bytes.GetDataSize();
    CORE::CString text;
    for ( UInt32 i=0; i<size; i+=3 )
    {
        const UInt32 a = data[ i ];
        const UInt32 b = i + 1 < size ? data[ i + 1 ] : 0;
        const UInt32 c = i + 2 < size ? data[ i + 2 ] : 0;
        const UInt32 triple = ( a << 16 ) | ( b << 8 ) | c;
        const char group[ 4 ] = { alphabet[ ( triple >> 18 ) & 63 ], alphabet[ ( triple >> 12 ) & 63 ], i + 1 < size ? alphabet[ ( triple >> 6 ) & 63 ] : '=', i + 2 < size ? alphabet[ triple & 63 ] : '=' };
        text.Append( group, 4 );
    }
    return text;
}

bool
ImportFile( const CORE::CString& folder, const CORE::CString& file, CAssetScene& scene, CConversionReport& report )
{
    TAssetImporterPtr importer;
    if ( !CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetImporter( "fbx", importer ) )
        return false;
    CAssetPackContext context;
    if ( !OpenPackContext( context, folder, folder + "_inter" ) )
        return false;
    const bool imported = importer->Import( folder + "/" + file, context, scene, report );
    if ( !imported )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Import of " << file << " failed: " << report.ToSummaryText() );
    return imported;
}

/** The first vertex of the submesh at the position, with the normal given when 'normalZ' is not zero */
Int32
FindVertex( const CAssetSubMesh& subMesh, Float32 x, Float32 y, Float32 z, Float32 normalZ )
{
    for ( UInt32 v=0; v<subMesh.GetVertexCount(); ++v )
    {
        if ( Near( subMesh.positions[ v*3 ], x ) && Near( subMesh.positions[ v*3+1 ], y ) && Near( subMesh.positions[ v*3+2 ], z ) &&
             ( 0.0f == normalZ || Near( subMesh.normals[ v*3+2 ], normalZ ) ) )
            return (Int32) v;
    }
    return -1;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformFbxImportTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetpluginFBX.Import" );

    GUCEF_TESTFW_TESTCASE( "Plugin: the importer \"fbx\" is registered with its version and takes .fbx files only" )
    try
    {
        TAssetImporterPtr importer;
        ASSERT_TRUE( CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetImporter( "fbx", importer ) );
        ASSERT_TRUE( 1 == importer->GetVersion() );
        CAssetPackContext context;
        ASSERT_TRUE( importer->CanImport( "pack/model.fbx", context ) && importer->CanImport( "pack/MODEL.FBX", context ) );
        ASSERT_FALSE( importer->CanImport( "pack/model.gltf", context ) || importer->CanImport( "pack/model", context ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Cube: positions, normals, UV coordinates (V flipped), indices, node, material and textures of an ASCII FBX file" )
    try
    {
        MakeWorkFolder( "cube" );
        ASSERT_TRUE( WriteTestPng( "work/cube/albedo.png", 4, 4, 255, 0, 0, 255 ) && WriteTestPng( "work/cube/normal.png", 4, 4, 128, 128, 255, 255 ) );
        const double translation[ 3 ] = { 1.0, 2.0, 3.0 };
        ASSERT_TRUE( WriteWorkText( "cube/cube.fbx", MakeCubeFbx( FbxYUpMetres(), 1.0, translation, "albedo.png", "normal.png" ) ) );

        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportFile( "work/cube", "cube.fbx", scene, report ) );
        ASSERT_TRUE( 0 == report.GetItemCount( REPORTITEM_ERROR ) );

        // the file facts
        ASSERT_TRUE( "7400 ascii" == scene.provenance.sourceFormatVersion && "fbx" == scene.provenance.sourceFormat );
        ASSERT_TRUE( scene.coordinateSystem == MATH::CCoordinateSystem::GucefDefault() );

        // the node and the mesh
        REQUIRE( 1 == scene.nodes.size() && 1 == scene.meshes.size() && 1 == scene.meshes[ 0 ].subMeshes.size() && 1 == scene.materials.size() );
        ASSERT_TRUE( "Cube" == scene.nodes[ 0 ].name && 0 == scene.nodes[ 0 ].mesh && 1 == scene.rootNodes.size() );
        const MATH::TFloat32Transform3D& transform = scene.nodes[ 0 ].transform;
        ASSERT_TRUE( Near( transform.position.x, 1.0f ) && Near( transform.position.y, 2.0f ) && Near( transform.position.z, 3.0f ) );
        const CAssetSubMesh& subMesh = scene.meshes[ 0 ].subMeshes[ 0 ];
        ASSERT_TRUE( 24 == subMesh.GetVertexCount() && 36 == subMesh.indices.size() && 12 == subMesh.GetTriangleCount() );
        ASSERT_TRUE( 72 == subMesh.normals.size() && 48 == subMesh.uvSets[ 0 ].size() && subMesh.uvSets[ 1 ].empty() && 0 == subMesh.materialIndex );
        const Int32 corner = FindVertex( subMesh, 0.0f, 0.0f, 0.0f, -1.0f );
        ASSERT_TRUE( corner >= 0 && Near( subMesh.uvSets[ 0 ][ corner * 2 ], 0.0f ) && Near( subMesh.uvSets[ 0 ][ corner * 2 + 1 ], 1.0f ) );
        const Int32 far = FindVertex( subMesh, 1.0f, 1.0f, 1.0f, 1.0f );
        ASSERT_TRUE( far >= 0 && Near( subMesh.uvSets[ 0 ][ far * 2 ], 1.0f ) && Near( subMesh.uvSets[ 0 ][ far * 2 + 1 ], 0.0f ) );
        CAssetSceneSummary summary;
        summary.Compute( scene );
        ASSERT_TRUE( summary.hasBounds && Near( summary.boundsMin[ 0 ], 1.0f ) && Near( summary.boundsMax[ 2 ], 4.0f ) );

        // the material and its textures
        const CAssetMaterial& material = scene.materials[ 0 ];
        REQUIRE( material.baseColorTexture.IsUsed() && material.normalTexture.IsUsed() && 2 == scene.textures.size() );
        ASSERT_TRUE( "Mat" == material.name && Near( material.baseColorFactor[ 0 ], 0.8f ) && Near( material.baseColorFactor[ 1 ], 0.4f ) && Near( material.baseColorFactor[ 2 ], 0.2f ) );
        ASSERT_TRUE( Near( material.roughnessFactor, (Float32) sqrt( 2.0 / 22.0 ), 1e-3f ) && Near( material.metallicFactor, 0.0f ) );
        ASSERT_FALSE( material.metallicRoughnessTexture.IsUsed() );
        ASSERT_TRUE( "work/cube/albedo.png" == scene.textures[ material.baseColorTexture.textureIndex ].sources[ 0 ] && TEXTUREUSAGE_COLOUR == scene.textures[ material.baseColorTexture.textureIndex ].usage );
        ASSERT_TRUE( "work/cube/normal.png" == scene.textures[ material.normalTexture.textureIndex ].sources[ 0 ] && TEXTUREUSAGE_NORMAL == scene.textures[ material.normalTexture.textureIndex ].usage );
        ASSERT_TRUE( "FBX/FBX Phong" == material.source.shaderName && !material.source.properties.GetAttributeValueOrChildValueByName( "ShininessExponent" ).IsNULLOrEmpty() );
        ASSERT_TRUE( report.GetItemCount( REPORTITEM_APPROXIMATION ) >= 1 );
        ASSERT_TRUE( scene.dependencies.size() == 2 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Units and axes: a file in centimetres with Z up states them, and the normalisation turns it into metres with Y up" )
    try
    {
        MakeWorkFolder( "zup" );
        const double translation[ 3 ] = { 100.0, 200.0, 50.0 };
        ASSERT_TRUE( WriteWorkText( "zup/cube.fbx", MakeCubeFbx( FbxZUpCentimetres(), 100.0, translation, CORE::CString::Empty, CORE::CString::Empty ) ) );

        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportFile( "work/zup", "cube.fbx", scene, report ) );
        REQUIRE( 1 == scene.nodes.size() && 1 == scene.meshes.size() && 1 == scene.meshes[ 0 ].subMeshes.size() && 1 == scene.materials.size() );
        ASSERT_TRUE( scene.coordinateSystem == MATH::CCoordinateSystem( MATH::SIGNEDAXIS_POSITIVE_X, MATH::SIGNEDAXIS_POSITIVE_Z, MATH::SIGNEDAXIS_POSITIVE_Y, 100.0 ) );
        // as the file has it: centimetres
        ASSERT_TRUE( Near( scene.nodes[ 0 ].transform.position.y, 200.0f ) && FindVertex( scene.meshes[ 0 ].subMeshes[ 0 ], 100.0f, 100.0f, 100.0f, 0.0f ) >= 0 );
        ASSERT_FALSE( scene.materials[ 0 ].baseColorTexture.IsUsed() );

        ASSERT_TRUE( CAssetNormaliser::Normalise( scene, report ) );
        ASSERT_TRUE( scene.coordinateSystem == MATH::CCoordinateSystem::GucefDefault() );
        ASSERT_TRUE( Near( scene.nodes[ 0 ].transform.position.x, 1.0f ) && Near( scene.nodes[ 0 ].transform.position.y, 0.5f ) && Near( scene.nodes[ 0 ].transform.position.z, -2.0f ) );
        const CAssetSubMesh& subMesh = scene.meshes[ 0 ].subMeshes[ 0 ];
        ASSERT_TRUE( FindVertex( subMesh, 0.0f, 0.0f, 0.0f, 0.0f ) >= 0 && FindVertex( subMesh, 1.0f, 1.0f, -1.0f, 0.0f ) >= 0 );
        ASSERT_TRUE( Near( (Float32) scene.normalisation.GetAttributeValueOrChildValueByName( "scale" ).AsFloat64(), 0.01f, 1e-6f ) );
        ASSERT_FALSE( scene.normalisation.GetAttributeValueOrChildValueByName( "mirrored" ).AsBool( true ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Skin: a strip bound to five bones keeps every influence, the bind matrices and the skeleton" )
    try
    {
        MakeWorkFolder( "skin" );
        ASSERT_TRUE( WriteWorkText( "skin/strip.fbx", MakeSkinnedFbx( FbxYUpMetres(), 5 ) ) );
        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportFile( "work/skin", "strip.fbx", scene, report ) );
        ASSERT_TRUE( 0 == report.GetItemCount( REPORTITEM_ERROR ) );

        REQUIRE( 1 == scene.skins.size() && 5 == scene.skins[ 0 ].joints.size() && 5 == scene.skins[ 0 ].inverseBindMatrices.size() );
        // the bones are the nodes Bone0 to Bone4, in a chain below the root bone
        for ( UInt32 b=0; b<5; ++b )
            ASSERT_TRUE( ( "Bone" + CORE::UInt32ToString( b ) ) == scene.nodes[ scene.skins[ 0 ].joints[ b ] ].name );
        ASSERT_TRUE( scene.skins[ 0 ].skeletonRoot >= 0 && "Bone0" == scene.nodes[ scene.skins[ 0 ].skeletonRoot ].name );
        // the inverse bind matrix of bone 3 undoes its place at 3 units up
        ASSERT_TRUE( Near( scene.skins[ 0 ].inverseBindMatrices[ 3 ].GetData()[ 13 ], -3.0f ) && Near( scene.skins[ 0 ].inverseBindMatrices[ 0 ].GetData()[ 13 ], 0.0f ) );

        const Int32 meshNode = scene.FindNode( "Strip" );
        REQUIRE( meshNode >= 0 && scene.nodes[ meshNode ].mesh >= 0 && 0 == scene.nodes[ meshNode ].skin );
        const CAssetSubMesh& subMesh = scene.meshes[ scene.nodes[ meshNode ].mesh ].subMeshes[ 0 ];
        REQUIRE( 5 == subMesh.influencesPerVertex );
        ASSERT_TRUE( subMesh.GetVertexCount() >= 6 && 12 == subMesh.indices.size() );
        REQUIRE( subMesh.joints.size() == subMesh.GetVertexCount() * 5 && subMesh.weights.size() == subMesh.GetVertexCount() * 5 );

        // the vertex with five influences: strongest first, all of them there, summing to one
        const Int32 first = FindVertex( subMesh, 0.0f, 0.0f, 0.0f, 0.0f );
        ASSERT_TRUE( first >= 0 );
        const Float32 expected[ 5 ] = { 0.3f, 0.25f, 0.2f, 0.15f, 0.1f };
        for ( UInt32 i=0; i<5 && first >= 0; ++i )
            ASSERT_TRUE( Near( subMesh.weights[ first * 5 + i ], expected[ i ], 1e-4f ) && i == subMesh.joints[ first * 5 + i ] );
        for ( UInt32 v=0; v<subMesh.GetVertexCount(); ++v )
        {
            Float32 sum = 0.0f;
            for ( UInt32 i=0; i<5; ++i )
                sum += subMesh.weights[ v * 5 + i ];
            ASSERT_TRUE( Near( sum, 1.0f, 1e-4f ) );
        }
        const Int32 half = FindVertex( subMesh, 0.0f, 1.0f, 0.0f, 0.0f );
        ASSERT_TRUE( half >= 0 && Near( subMesh.weights[ half * 5 ], 0.5f, 1e-4f ) && Near( subMesh.weights[ half * 5 + 1 ], 0.5f, 1e-4f ) && Near( subMesh.weights[ half * 5 + 2 ], 0.0f ) );

        // the take: the second bone turns 90 degrees about Z in one second
        REQUIRE( 1 == scene.animations.size() );
        const CAssetAnimation& animation = scene.animations[ 0 ];
        ASSERT_TRUE( "Take 001" == animation.name && "Take 001" == animation.sourceTake && animation.sampled && !animation.loop );
        ASSERT_TRUE( Near( animation.GetDuration(), 1.0f, 1e-3f ) );
        const CAssetAnimationChannel* rotation = GUCEF_NULL;
        for ( size_t c=0; c<animation.channels.size(); ++c )
            if ( ANIMATIONPATH_ROTATION == animation.channels[ c ].path && "Bone1" == scene.nodes[ animation.channels[ c ].targetNode ].name )
                rotation = &animation.channels[ c ];
        ASSERT_TRUE( GUCEF_NULL != rotation && rotation->times.size() >= 2 && 4 * rotation->times.size() == rotation->values.size() );
        if ( GUCEF_NULL != rotation && rotation->times.size() >= 2 )
        {
            const size_t last = rotation->times.size() - 1;
            ASSERT_TRUE( Near( rotation->times[ 0 ], 0.0f ) && Near( rotation->times[ last ], 1.0f, 1e-3f ) );
            ASSERT_TRUE( Near( rotation->values[ 3 ], 1.0f, 1e-3f ) );
            ASSERT_TRUE( Near( rotation->values[ last * 4 + 2 ], 0.70710678f, 1e-3f ) && Near( rotation->values[ last * 4 + 3 ], 0.70710678f, 1e-3f ) );
        }
        ASSERT_TRUE( 1 == animation.channels.size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Blend shape: a channel becomes a morph target with its name, its offsets and its weight" )
    try
    {
        MakeWorkFolder( "shape" );
        ASSERT_TRUE( WriteWorkText( "shape/plane.fbx", MakeBlendShapeFbx( FbxYUpMetres(), false ) ) );
        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportFile( "work/shape", "plane.fbx", scene, report ) );
        REQUIRE( 1 == scene.meshes.size() && 1 == scene.meshes[ 0 ].subMeshes.size() );
        const CAssetSubMesh& subMesh = scene.meshes[ 0 ].subMeshes[ 0 ];
        ASSERT_TRUE( 1 == subMesh.morphTargets.size() );
        if ( 1 == subMesh.morphTargets.size() )
        {
            const CAssetMorphTarget& target = subMesh.morphTargets[ 0 ];
            ASSERT_TRUE( "Lift" == target.name && Near( target.defaultWeight, 0.5f, 1e-4f ) && target.positionDeltas.size() == subMesh.positions.size() );
            for ( UInt32 v=0; v<subMesh.GetVertexCount() && target.positionDeltas.size() == subMesh.positions.size(); ++v )
                ASSERT_TRUE( Near( target.positionDeltas[ v*3 ], 0.0f ) && Near( target.positionDeltas[ v*3+1 ], 0.5f ) && Near( target.positionDeltas[ v*3+2 ], 0.0f ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Blend shape animation: the weight of the channel over a take becomes a weights channel of the node of the mesh" )
    try
    {
        MakeWorkFolder( "morph" );
        ASSERT_TRUE( WriteWorkText( "morph/plane.fbx", MakeBlendShapeFbx( FbxYUpMetres(), true ) ) );
        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportFile( "work/morph", "plane.fbx", scene, report ) );
        REQUIRE( 1 == scene.nodes.size() && 1 == scene.animations.size() && 1 == scene.animations[ 0 ].channels.size() );
        const CAssetAnimationChannel& channel = scene.animations[ 0 ].channels[ 0 ];
        ASSERT_TRUE( "Morph" == scene.animations[ 0 ].name && ANIMATIONPATH_WEIGHTS == channel.path && 0 == channel.targetNode );
        REQUIRE( channel.times.size() >= 2 && channel.values.size() == channel.times.size() );
        ASSERT_TRUE( Near( channel.times[ 0 ], 0.0f ) && Near( channel.times[ channel.times.size() - 1 ], 1.0f, 1e-3f ) );
        ASSERT_TRUE( Near( channel.values[ 0 ], 0.0f, 1e-3f ) && Near( channel.values[ channel.values.size() - 1 ], 1.0f, 1e-3f ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PBR: the property set of a 3ds Max metal/rough material gives base colour, roughness, metalness and its textures" )
    try
    {
        MakeWorkFolder( "pbr" );
        ASSERT_TRUE( WriteTestPng( "work/pbr/base.png", 4, 4, 10, 20, 30, 255 ) && WriteTestPng( "work/pbr/norm.png", 4, 4, 128, 128, 255, 255 ) );
        ASSERT_TRUE( WriteWorkText( "pbr/cube.fbx", MakePbrFbx( FbxYUpMetres(), "base.png", "norm.png" ) ) );
        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportFile( "work/pbr", "cube.fbx", scene, report ) );
        REQUIRE( 1 == scene.materials.size() );
        const CAssetMaterial& material = scene.materials[ 0 ];
        ASSERT_TRUE( "PbrMat" == material.name && "FBX/3ds Max PBR (metal/rough)" == material.source.shaderName );
        ASSERT_TRUE( Near( material.baseColorFactor[ 0 ], 0.2f ) && Near( material.baseColorFactor[ 1 ], 0.6f ) && Near( material.baseColorFactor[ 2 ], 0.9f ) );
        ASSERT_TRUE( Near( material.roughnessFactor, 0.35f, 1e-4f ) && Near( material.metallicFactor, 0.9f, 1e-4f ) );
        ASSERT_TRUE( material.baseColorTexture.IsUsed() && material.normalTexture.IsUsed() );
        ASSERT_TRUE( report.GetItemCount( REPORTITEM_APPROXIMATION ) == 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Embedded texture: the picture inside the FBX is written below the intermediate folder and is the source of the texture" )
    try
    {
        MakeWorkFolder( "inside" );
        ASSERT_TRUE( WriteTestPng( "work/inside_source.png", 4, 4, 90, 80, 70, 255 ) );
        CORE::CDynamicBuffer png;
        ASSERT_TRUE( VFS::CVfsGlobal::Instance()->GetVfs().LoadFile( png, "work/inside_source.png", "rb" ) );
        ASSERT_TRUE( WriteWorkText( "inside/cube.fbx", MakeEmbeddedTextureFbx( FbxYUpMetres(), "inside.png", Base64( png ) ) ) );

        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportFile( "work/inside", "cube.fbx", scene, report ) );
        REQUIRE( 1 == scene.materials.size() && scene.materials[ 0 ].baseColorTexture.IsUsed() && 1 == scene.textures.size() && 1 == scene.textures[ 0 ].sources.size() );
        ASSERT_TRUE( scene.textures[ 0 ].sources[ 0 ].StartsWith( "work/inside_inter/embedded/" ) && scene.textures[ 0 ].sources[ 0 ].EndsWith( "inside.png" ) );
        CORE::CDynamicBuffer extracted;
        ASSERT_TRUE( VFS::CVfsGlobal::Instance()->GetVfs().LoadFile( extracted, scene.textures[ 0 ].sources[ 0 ], "rb" ) );
        ASSERT_TRUE( extracted.GetDataSize() == png.GetDataSize() && 0 == memcmp( extracted.GetConstBufferPtr(), png.GetConstBufferPtr(), png.GetDataSize() ) );
        ASSERT_TRUE( 0 == scene.dependencies.size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Textures that are not in the pack leave the slot empty with a warning, and files that are not FBX fail" )
    try
    {
        MakeWorkFolder( "lost" );
        const double translation[ 3 ] = { 0.0, 0.0, 0.0 };
        ASSERT_TRUE( WriteWorkText( "lost/cube.fbx", MakeCubeFbx( FbxYUpMetres(), 1.0, translation, "C:\\Users\\artist\\textures\\missing.png", CORE::CString::Empty ) ) );
        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportFile( "work/lost", "cube.fbx", scene, report ) );
        REQUIRE( 1 == scene.materials.size() );
        ASSERT_TRUE( !scene.materials[ 0 ].baseColorTexture.IsUsed() && scene.textures.empty() );
        ASSERT_TRUE( report.GetItemCount( REPORTITEM_WARNING ) >= 1 );

        // the file name of the texture is looked for in the whole pack
        ASSERT_TRUE( WriteTestPng( "work/lost/deep/missing.png", 2, 2, 1, 2, 3, 255 ) );
        CAssetScene found;
        CConversionReport foundReport;
        REQUIRE( ImportFile( "work/lost", "cube.fbx", found, foundReport ) );
        REQUIRE( 1 == found.materials.size() );
        ASSERT_TRUE( found.materials[ 0 ].baseColorTexture.IsUsed() && 1 == found.textures.size() && "work/lost/deep/missing.png" == found.textures[ 0 ].sources[ 0 ] );

        ASSERT_TRUE( WriteWorkText( "lost/notfbx.fbx", "this is not an FBX file" ) );
        CAssetScene bad;
        CConversionReport badReport;
        ASSERT_FALSE( ImportFile( "work/lost", "notfbx.fbx", bad, badReport ) );
        ASSERT_TRUE( badReport.GetItemCount( REPORTITEM_ERROR ) >= 1 );
        CAssetScene absent;
        CConversionReport absentReport;
        ASSERT_FALSE( ImportFile( "work/lost", "absent.fbx", absent, absentReport ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
