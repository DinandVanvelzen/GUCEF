/*
 *  assetcookerOGRE_TestApp: Test application for the Ogre 1.x cooker of the asset converter (assetcookerOGRE)
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

#include "TestCookerScenes.h"

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

namespace {

void
Append( GUCEF::vector< Float32 >& values, Float32 a, Float32 b, Float32 c )
{
    values.push_back( a ); values.push_back( b ); values.push_back( c );
}

void
Append( GUCEF::vector< Float32 >& values, Float32 a, Float32 b, Float32 c, Float32 d )
{
    values.push_back( a ); values.push_back( b ); values.push_back( c ); values.push_back( d );
}

void
SetPosition( CAssetNode& node, Float32 x, Float32 y, Float32 z )
{
    node.transform.position = MATH::TFloat32Transform3D::TPositionVector( x, y, z );
}

MATH::TFloat32Matrix4D
Translation( Float32 x, Float32 y, Float32 z )
{
    MATH::TFloat32Matrix4D matrix;
    matrix.SetTranslation( MATH::TFloat32Matrix4D::TVector3( x, y, z ) );
    return matrix;
}

/** A quad in the XY plane facing +Z: four vertices, normals, 'uvSets' UV sets, two triangles */
void
FillQuad( CAssetSubMesh& subMesh, Float32 size, UInt32 uvSets = 1 )
{
    const Float32 corners[ 4 ][ 2 ] = { { -1.0f, -1.0f }, { 1.0f, -1.0f }, { 1.0f, 1.0f }, { -1.0f, 1.0f } };
    for ( UInt32 v=0; v<4; ++v )
    {
        Append( subMesh.positions, corners[ v ][ 0 ] * size, corners[ v ][ 1 ] * size, 0.0f );
        Append( subMesh.normals, 0.0f, 0.0f, 1.0f );
        for ( UInt32 u=0; u<uvSets; ++u )
        {
            subMesh.uvSets[ u ].push_back( corners[ v ][ 0 ] * 0.5f + 0.5f );
            subMesh.uvSets[ u ].push_back( 0.5f - corners[ v ][ 1 ] * 0.5f );
        }
    }
    const UInt32 indices[ 6 ] = { 0, 1, 2, 0, 2, 3 };
    subMesh.indices.assign( indices, indices + 6 );
}

/** A grid of n x n quads over [-1, 1] in XZ, facing +Y, with a gentle bump so a reduction has something to keep */
void
FillGrid( CAssetSubMesh& subMesh, UInt32 n )
{
    for ( UInt32 z=0; z<=n; ++z )
    {
        for ( UInt32 x=0; x<=n; ++x )
        {
            const Float32 fx = -1.0f + 2.0f * (Float32) x / (Float32) n;
            const Float32 fz = -1.0f + 2.0f * (Float32) z / (Float32) n;
            Append( subMesh.positions, fx, 0.05f * ( fx * fx - fz * fz ), fz );
            Append( subMesh.normals, 0.0f, 1.0f, 0.0f );
            subMesh.uvSets[ 0 ].push_back( (Float32) x / (Float32) n );
            subMesh.uvSets[ 0 ].push_back( (Float32) z / (Float32) n );
        }
    }
    for ( UInt32 z=0; z<n; ++z )
    {
        for ( UInt32 x=0; x<n; ++x )
        {
            const UInt32 a = z * ( n + 1 ) + x;
            const UInt32 b = a + 1;
            const UInt32 c = a + n + 1;
            const UInt32 d = c + 1;
            subMesh.indices.push_back( a ); subMesh.indices.push_back( c ); subMesh.indices.push_back( b );
            subMesh.indices.push_back( b ); subMesh.indices.push_back( c ); subMesh.indices.push_back( d );
        }
    }
}

UInt32
AddTexture( CAssetScene& scene, const CORE::CString& path, TTextureUsage usage )
{
    CAssetTexture texture;
    texture.name = CVfsPaths::NameOf( path );
    texture.sources.push_back( path );
    texture.usage = usage;
    scene.textures.push_back( texture );
    return (UInt32) scene.textures.size() - 1;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      BUILDING                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

void
BuildCharacterScene( CAssetScene& scene, const CORE::CString& textureFolder )
{GUCEF_TRACE;

    const UInt32 skinTexture = AddTexture( scene, textureFolder + "/skin.png", TEXTUREUSAGE_COLOUR );
    const UInt32 normalTexture = AddTexture( scene, textureFolder + "/normal.png", TEXTUREUSAGE_NORMAL );
    CAssetMaterial skin;
    skin.name = "Skin";
    skin.baseColorTexture = CAssetTextureReference( (Int32) skinTexture );
    scene.materials.push_back( skin );
    CAssetMaterial armour;
    armour.name = "Armour";
    armour.normalTexture = CAssetTextureReference( (Int32) normalTexture );
    scene.materials.push_back( armour );

    // 0 Root, 1 Hip, 2 Spine, 3 Head, 4 ArmL, 5 ArmR, 6 Tail, 7 Body, 8 Prop
    const UInt32 root = scene.AddNode( "Root" );
    const UInt32 hip = scene.AddNode( "Hip", (Int32) root );
    SetPosition( scene.nodes[ hip ], 0.0f, 1.0f, 0.0f );
    const UInt32 spine = scene.AddNode( "Spine", (Int32) hip );
    SetPosition( scene.nodes[ spine ], 0.0f, 0.5f, 0.0f );
    const UInt32 head = scene.AddNode( "Head", (Int32) spine );
    SetPosition( scene.nodes[ head ], 0.0f, 0.5f, 0.0f );
    const UInt32 armL = scene.AddNode( "ArmL", (Int32) spine );
    SetPosition( scene.nodes[ armL ], 0.5f, 0.0f, 0.0f );
    const UInt32 armR = scene.AddNode( "ArmR", (Int32) spine );
    SetPosition( scene.nodes[ armR ], -0.5f, 0.0f, 0.0f );
    const UInt32 tail = scene.AddNode( "Tail", (Int32) hip );
    SetPosition( scene.nodes[ tail ], 0.0f, 0.0f, -0.3f );
    const UInt32 body = scene.AddNode( "Body", (Int32) root );
    const UInt32 prop = scene.AddNode( "Prop", (Int32) root );
    SetPosition( scene.nodes[ prop ], 2.0f, 0.0f, 0.0f );

    CAssetSkin skeleton;
    skeleton.name = "Rig";
    const UInt32 joints[ 6 ] = { hip, spine, head, armL, armR, tail };
    const Float32 rest[ 6 ][ 3 ] = { { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.5f, 0.0f }, { 0.0f, 2.0f, 0.0f }, { 0.5f, 1.5f, 0.0f }, { -0.5f, 1.5f, 0.0f }, { 0.0f, 1.0f, -0.3f } };
    for ( UInt32 j=0; j<6; ++j )
    {
        skeleton.joints.push_back( joints[ j ] );
        skeleton.inverseBindMatrices.push_back( Translation( -rest[ j ][ 0 ], -rest[ j ][ 1 ], -rest[ j ][ 2 ] ) );
    }
    skeleton.skeletonRoot = (Int32) hip;
    scene.skins.push_back( skeleton );

    // the body: two quads of Skin (six influences per vertex) and a triangle of Armour
    CAssetMesh bodyMesh;
    bodyMesh.name = "BodyMesh";
    {
        CAssetSubMesh first;
        FillQuad( first, 1.0f );
        first.materialIndex = 0;
        first.influencesPerVertex = 6;
        const Float32 weights[ 4 ][ 6 ] = { { 0.3f, 0.25f, 0.2f, 0.1f, 0.1f, 0.05f }, { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
                                            { 0.0f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f }, { 0.25f, 0.25f, 0.25f, 0.25f, 0.0f, 0.0f } };
        for ( UInt32 v=0; v<4; ++v )
        {
            for ( UInt32 i=0; i<6; ++i )
            {
                first.joints.push_back( i );
                first.weights.push_back( weights[ v ][ i ] );
            }
        }
        bodyMesh.subMeshes.push_back( first );

        CAssetSubMesh second;
        FillQuad( second, 0.5f );
        second.materialIndex = 0;
        second.influencesPerVertex = 6;
        for ( UInt32 v=0; v<4; ++v )
        {
            for ( UInt32 i=0; i<6; ++i )
            {
                second.joints.push_back( i );
                second.weights.push_back( 2 == i ? 1.0f : 0.0f );
            }
        }
        bodyMesh.subMeshes.push_back( second );

        CAssetSubMesh third;
        Append( third.positions, 0.0f, 0.0f, 0.0f );
        Append( third.positions, 1.0f, 0.0f, 0.0f );
        Append( third.positions, 0.0f, 1.0f, 0.0f );
        for ( UInt32 v=0; v<3; ++v )
        {
            Append( third.normals, 0.0f, 0.0f, 1.0f );
            for ( UInt32 i=0; i<6; ++i )
            {
                third.joints.push_back( i );
                third.weights.push_back( 0 == i ? 1.0f : 0.0f );
            }
        }
        third.influencesPerVertex = 6;
        third.uvSets[ 0 ].push_back( 0.0f ); third.uvSets[ 0 ].push_back( 1.0f );
        third.uvSets[ 0 ].push_back( 1.0f ); third.uvSets[ 0 ].push_back( 1.0f );
        third.uvSets[ 0 ].push_back( 0.0f ); third.uvSets[ 0 ].push_back( 0.0f );
        third.indices.push_back( 0 ); third.indices.push_back( 1 ); third.indices.push_back( 2 );
        third.materialIndex = 1;
        bodyMesh.subMeshes.push_back( third );
    }
    scene.meshes.push_back( bodyMesh );
    scene.nodes[ body ].mesh = 0;
    scene.nodes[ body ].skin = 0;

    // the prop with two morph targets
    CAssetMesh propMesh;
    propMesh.name = "PropMesh";
    {
        CAssetSubMesh subMesh;
        FillQuad( subMesh, 0.5f );
        subMesh.materialIndex = 0;
        CAssetMorphTarget smile;
        smile.name = "Smile";
        CAssetMorphTarget blink;
        blink.name = "Blink";
        for ( UInt32 v=0; v<4; ++v )
        {
            Append( smile.positionDeltas, 0.0f, 0.1f * (Float32) v, 0.0f );
            Append( smile.normalDeltas, 0.0f, 0.0f, 0.0f );
            Append( blink.positionDeltas, 0.0f, 0.0f, v < 2 ? 0.2f : 0.0f );
            Append( blink.normalDeltas, 0.0f, 0.0f, 0.0f );
        }
        subMesh.morphTargets.push_back( smile );
        subMesh.morphTargets.push_back( blink );
        propMesh.subMeshes.push_back( subMesh );
    }
    scene.meshes.push_back( propMesh );
    scene.nodes[ prop ].mesh = 1;

    // the clips
    {
        CAssetAnimation walk;
        walk.name = "Walk";
        walk.loop = true;
        CAssetAnimationChannel translation;
        translation.targetNode = hip;
        translation.path = ANIMATIONPATH_TRANSLATION;
        translation.times.push_back( 0.0f ); translation.times.push_back( 0.5f ); translation.times.push_back( 2.0f );
        Append( translation.values, 0.0f, 1.0f, 0.0f ); Append( translation.values, 0.0f, 1.2f, 0.3f ); Append( translation.values, 0.0f, 1.0f, 0.6f );
        walk.channels.push_back( translation );
        CAssetAnimationChannel rotation;
        rotation.targetNode = spine;
        rotation.path = ANIMATIONPATH_ROTATION;
        rotation.times.push_back( 0.0f ); rotation.times.push_back( 1.0f ); rotation.times.push_back( 2.0f );
        Append( rotation.values, 0.0f, 0.0f, 0.0f, 1.0f ); Append( rotation.values, 0.0f, 0.38268343f, 0.0f, 0.92387953f ); Append( rotation.values, 0.0f, 0.0f, 0.0f, 1.0f );
        walk.channels.push_back( rotation );
        CAssetAnimationChannel scale;
        scale.targetNode = head;
        scale.path = ANIMATIONPATH_SCALE;
        scale.times.push_back( 0.0f ); scale.times.push_back( 2.0f );
        Append( scale.values, 1.0f, 1.0f, 1.0f ); Append( scale.values, 2.0f, 2.0f, 2.0f );
        walk.channels.push_back( scale );
        scene.animations.push_back( walk );

        CAssetAnimation wave;
        wave.name = "Wave";
        CAssetAnimationChannel arm;
        arm.targetNode = armL;
        arm.path = ANIMATIONPATH_ROTATION;
        arm.times.push_back( 0.0f ); arm.times.push_back( 1.5f );
        Append( arm.values, 0.0f, 0.0f, 0.0f, 1.0f ); Append( arm.values, 0.0f, 0.0f, 0.70710678f, 0.70710678f );
        wave.channels.push_back( arm );
        scene.animations.push_back( wave );

        CAssetAnimation blink;
        blink.name = "Blink";
        CAssetAnimationChannel weights;
        weights.targetNode = prop;
        weights.path = ANIMATIONPATH_WEIGHTS;
        weights.times.push_back( 0.0f ); weights.times.push_back( 0.5f ); weights.times.push_back( 1.0f );
        weights.values.push_back( 0.0f ); weights.values.push_back( 0.0f );
        weights.values.push_back( 1.0f ); weights.values.push_back( 0.25f );
        weights.values.push_back( 0.0f ); weights.values.push_back( 1.0f );
        blink.channels.push_back( weights );
        scene.animations.push_back( blink );

        CAssetAnimation spin;
        spin.name = "Spin";
        spin.loop = true;
        CAssetAnimationChannel turn;
        turn.targetNode = prop;
        turn.path = ANIMATIONPATH_ROTATION;
        turn.times.push_back( 0.0f ); turn.times.push_back( 4.0f );
        Append( turn.values, 0.0f, 0.0f, 0.0f, 1.0f ); Append( turn.values, 0.0f, 1.0f, 0.0f, 0.0f );
        spin.channels.push_back( turn );
        scene.animations.push_back( spin );
    }
}

/*-------------------------------------------------------------------------*/

void
BuildMaterialScene( CAssetScene& scene, const CORE::CString& textureFolder )
{GUCEF_TRACE;

    const Int32 albedo = (Int32) AddTexture( scene, textureFolder + "/albedo.png", TEXTUREUSAGE_COLOUR );
    const Int32 orm = (Int32) AddTexture( scene, textureFolder + "/orm.png", TEXTUREUSAGE_MASK );
    const Int32 normal = (Int32) AddTexture( scene, textureFolder + "/normal.png", TEXTUREUSAGE_NORMAL );
    const Int32 emissive = (Int32) AddTexture( scene, textureFolder + "/emissive.png", TEXTUREUSAGE_COLOUR );
    const Int32 occlusion = (Int32) AddTexture( scene, textureFolder + "/occlusion.png", TEXTUREUSAGE_MASK );

    CAssetMaterial plain;                   // 0: base colour factor and texture with a transform, factors without texture
    plain.name = "Plain";
    plain.baseColorFactor[ 0 ] = 0.8f; plain.baseColorFactor[ 1 ] = 0.7f; plain.baseColorFactor[ 2 ] = 0.6f; plain.baseColorFactor[ 3 ] = 1.0f;
    plain.baseColorTexture = CAssetTextureReference( albedo, 1 );
    plain.baseColorTexture.hasTransform = true;
    plain.baseColorTexture.tiling[ 0 ] = 2.0f; plain.baseColorTexture.tiling[ 1 ] = 4.0f;
    plain.baseColorTexture.offset[ 0 ] = 0.25f; plain.baseColorTexture.offset[ 1 ] = 0.5f;
    plain.metallicFactor = 0.25f;
    plain.roughnessFactor = 0.75f;
    scene.materials.push_back( plain );

    CAssetMaterial metal;                   // 1: metal-roughness texture with the occlusion in it
    metal.name = "Metal";
    metal.metallicRoughnessTexture = CAssetTextureReference( orm );
    metal.occlusionTexture = CAssetTextureReference( orm );
    scene.materials.push_back( metal );

    CAssetMaterial factored;                // 2: factors and a separate occlusion texture: a derived texture
    factored.name = "Factored";
    factored.metallicRoughnessTexture = CAssetTextureReference( orm );
    factored.metallicFactor = 0.5f;
    factored.roughnessFactor = 0.25f;
    factored.occlusionTexture = CAssetTextureReference( occlusion );
    scene.materials.push_back( factored );

    CAssetMaterial bumpy;                   // 3: normal map
    bumpy.name = "Bumpy";
    bumpy.normalTexture = CAssetTextureReference( normal );
    scene.materials.push_back( bumpy );

    CAssetMaterial glow;                    // 4: emissive texture
    glow.name = "Glow";
    glow.emissiveTexture = CAssetTextureReference( emissive );
    glow.emissiveFactor[ 0 ] = glow.emissiveFactor[ 1 ] = glow.emissiveFactor[ 2 ] = 1.0f;
    scene.materials.push_back( glow );

    CAssetMaterial hot;                     // 5: emissive factor with strength, no texture
    hot.name = "Hot";
    hot.emissiveFactor[ 0 ] = 0.1f; hot.emissiveFactor[ 1 ] = 0.2f; hot.emissiveFactor[ 2 ] = 0.3f;
    hot.emissiveStrength = 4.0f;
    scene.materials.push_back( hot );

    CAssetMaterial leaf;                    // 6: alpha mask, double sided
    leaf.name = "Leaf";
    leaf.alphaMode = ALPHAMODE_MASK;
    leaf.alphaCutoff = 0.35f;
    leaf.doubleSided = true;
    scene.materials.push_back( leaf );

    CAssetMaterial glass;                   // 7: alpha blend
    glass.name = "Glass";
    glass.baseColorFactor[ 3 ] = 0.5f;
    glass.alphaMode = ALPHAMODE_BLEND;
    scene.materials.push_back( glass );

    CAssetMaterial coat;                    // 8: an extension the RTSS has nothing for
    coat.name = "Coat";
    CORE::CDataNode* clearcoat = coat.extensions.AddChild( "KHR_materials_clearcoat", GUCEF_DATATYPE_OBJECT );
    clearcoat->SetAttribute( "clearcoatFactor", 0.5f );
    scene.materials.push_back( coat );

    CAssetMaterial dim;                     // 9: emissive texture with a factor: a derived texture
    dim.name = "Dim";
    dim.emissiveTexture = CAssetTextureReference( emissive );
    dim.emissiveFactor[ 0 ] = dim.emissiveFactor[ 1 ] = dim.emissiveFactor[ 2 ] = 0.5f;
    scene.materials.push_back( dim );

    const UInt32 board = scene.AddNode( "Board" );
    CAssetMesh mesh;
    mesh.name = "BoardMesh";
    for ( UInt32 m=0; m<scene.materials.size(); ++m )
    {
        CAssetSubMesh subMesh;
        FillQuad( subMesh, 0.5f, 2 );
        for ( size_t p=0; p<subMesh.positions.size(); p+=3 )
            subMesh.positions[ p ] += (Float32) m;
        subMesh.materialIndex = (Int32) m;
        mesh.subMeshes.push_back( subMesh );
    }
    scene.meshes.push_back( mesh );
    scene.nodes[ board ].mesh = 0;
}

/*-------------------------------------------------------------------------*/

void
BuildLodScene( CAssetScene& scene )
{GUCEF_TRACE;

    CAssetMaterial material;
    material.name = "Crate";
    scene.materials.push_back( material );

    const UInt32 crate = scene.AddNode( "Crate" );
    const UInt32 level0 = scene.AddNode( "Crate_LOD0", (Int32) crate );
    const UInt32 level1 = scene.AddNode( "Crate_LOD1", (Int32) crate );
    const UInt32 level2 = scene.AddNode( "Crate_LOD2", (Int32) crate );
    const UInt32 grid = scene.AddNode( "Grid" );
    SetPosition( scene.nodes[ grid ], 3.0f, 0.0f, 0.0f );

    const char* const names[ 4 ] = { "CrateHigh", "CrateMid", "CrateLow", "GridMesh" };
    const UInt32 sizes[ 4 ] = { 10, 3, 1, 12 };
    for ( UInt32 m=0; m<4; ++m )
    {
        CAssetMesh mesh;
        mesh.name = names[ m ];
        CAssetSubMesh subMesh;
        FillGrid( subMesh, sizes[ m ] );
        subMesh.materialIndex = 0;
        mesh.subMeshes.push_back( subMesh );
        scene.meshes.push_back( mesh );
    }
    scene.nodes[ level0 ].mesh = 0;
    scene.nodes[ level1 ].mesh = 1;
    scene.nodes[ level2 ].mesh = 2;
    scene.nodes[ grid ].mesh = 3;

    CAssetLodGroup group;
    group.name = "Crate";
    group.AddLevel( level0, 0.6f );
    group.AddLevel( level1, 0.3f );
    group.AddLevel( level2, 0.1f );
    scene.lodGroups.push_back( group );
    scene.nodes[ crate ].lodGroup = 0;
}

/*-------------------------------------------------------------------------*/

void
BuildHouseScene( CAssetScene& scene, const CORE::CString& sourceTexture, const CORE::CString& intermediateTexture )
{GUCEF_TRACE;

    const Int32 wood = (Int32) AddTexture( scene, sourceTexture, TEXTUREUSAGE_COLOUR );
    const Int32 paint = (Int32) AddTexture( scene, intermediateTexture, TEXTUREUSAGE_COLOUR );
    CAssetMaterial woodMaterial;
    woodMaterial.name = "Wood";
    woodMaterial.baseColorTexture = CAssetTextureReference( wood );
    scene.materials.push_back( woodMaterial );
    CAssetMaterial paintMaterial;
    paintMaterial.name = "Paint";
    paintMaterial.baseColorTexture = CAssetTextureReference( paint );
    scene.materials.push_back( paintMaterial );

    CAssetMesh houseMesh;
    houseMesh.name = "HouseMesh";
    CAssetSubMesh walls;
    FillQuad( walls, 2.0f );
    walls.materialIndex = 0;
    houseMesh.subMeshes.push_back( walls );
    scene.meshes.push_back( houseMesh );
    CAssetMesh doorMesh;
    doorMesh.name = "DoorMesh";
    CAssetSubMesh door;
    FillQuad( door, 0.5f );
    door.materialIndex = 1;
    doorMesh.subMeshes.push_back( door );
    scene.meshes.push_back( doorMesh );

    // 0 House, 1 Door, 2 Lamp, 3 Sign, 4 Fan
    const UInt32 house = scene.AddNode( "House" );
    SetPosition( scene.nodes[ house ], 1.0f, 0.0f, 2.0f );
    scene.nodes[ house ].mesh = 0;
    CAssetCollider box;
    box.name = "HouseBox";
    box.shape = COLLIDERSHAPE_BOX;
    box.halfExtents[ 0 ] = 1.0f; box.halfExtents[ 1 ] = 2.0f; box.halfExtents[ 2 ] = 3.0f;
    box.transform.position = MATH::TFloat32Transform3D::TPositionVector( 0.0f, 2.0f, 0.0f );
    scene.nodes[ house ].colliders.push_back( box );

    const UInt32 doorNode = scene.AddNode( "Door", (Int32) house );
    SetPosition( scene.nodes[ doorNode ], 0.0f, 0.0f, 1.0f );
    scene.nodes[ doorNode ].mesh = 1;
    CAssetCollider hull;
    hull.name = "DoorHull";
    hull.shape = COLLIDERSHAPE_CONVEX;
    hull.meshIndex = 1;
    hull.trigger = true;
    scene.nodes[ doorNode ].colliders.push_back( hull );

    const UInt32 lamp = scene.AddNode( "Lamp", (Int32) house );
    SetPosition( scene.nodes[ lamp ], 0.0f, 3.0f, 0.0f );
    CAssetLight light;
    light.name = "LampLight";
    light.type = LIGHTTYPE_POINT;
    light.colour[ 0 ] = 1.0f; light.colour[ 1 ] = 0.9f; light.colour[ 2 ] = 0.8f;
    light.intensity = 5.0f;
    light.range = 10.0f;
    scene.lights.push_back( light );
    scene.nodes[ lamp ].light = 0;

    const UInt32 sign = scene.AddNode( "Sign", (Int32) house );
    SetPosition( scene.nodes[ sign ], -1.0f, 1.0f, 0.0f );
    scene.nodes[ sign ].mesh = 0;
    scene.nodes[ sign ].extras.SetAttribute( "tag", "sign" );
    scene.nodes[ sign ].extras.SetAttribute( "weight", 1.5f );

    const UInt32 fan = scene.AddNode( "Fan", (Int32) house );
    SetPosition( scene.nodes[ fan ], 0.0f, 4.0f, 0.0f );
    scene.nodes[ fan ].mesh = 1;
    CAssetAnimation spin;
    spin.name = "Spin";
    spin.loop = true;
    CAssetAnimationChannel turn;
    turn.targetNode = fan;
    turn.path = ANIMATIONPATH_ROTATION;
    turn.times.push_back( 0.0f ); turn.times.push_back( 1.0f ); turn.times.push_back( 2.0f );
    Append( turn.values, 0.0f, 0.0f, 0.0f, 1.0f ); Append( turn.values, 0.0f, 0.70710678f, 0.0f, 0.70710678f ); Append( turn.values, 0.0f, 1.0f, 0.0f, 0.0f );
    spin.channels.push_back( turn );
    scene.animations.push_back( spin );

    CAssetPrefab doorway;
    doorway.name = "Doorway";
    doorway.rootNode = doorNode;
    CAssetPrefabInstance instance;
    instance.node = doorNode;
    instance.sourcePrefab = "Prefabs/Door.prefab";
    instance.overrides = CORE::CDataNode( "overrides", GUCEF_DATATYPE_OBJECT );
    CORE::CString::StringVector materials;
    materials.push_back( "Wood" );
    instance.overrides.SetValuesOfChildByName( "materials", materials );
    doorway.instances.push_back( instance );
    scene.prefabs.push_back( doorway );
}

/*-------------------------------------------------------------------------*/
