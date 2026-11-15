/*
 *  assetpluginGLTF_TestApp: Test application for the glTF plugin of the asset converter (assetpluginGLTF)
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

#include "TestGltfScenes.h"

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
SetTransform( CAssetNode& node, Float32 x, Float32 y, Float32 z, Float32 qx = 0.0f, Float32 qy = 0.0f, Float32 qz = 0.0f, Float32 qw = 1.0f )
{
    node.transform.position = MATH::TFloat32Transform3D::TPositionVector( x, y, z );
    node.transform.rotation.x = qx;
    node.transform.rotation.y = qy;
    node.transform.rotation.z = qz;
    node.transform.rotation.w = qw;
}

MATH::TFloat32Matrix4D
Translation( Float32 x, Float32 y, Float32 z )
{
    MATH::TFloat32Matrix4D matrix;
    matrix.SetTranslation( MATH::TFloat32Matrix4D::TVector3( x, y, z ) );
    return matrix;
}

/** A quad of four vertices with every stream */
void
FillQuad( CAssetSubMesh& subMesh, Float32 size )
{
    const Float32 corners[ 4 ][ 2 ] = { { -1.0f, -1.0f }, { 1.0f, -1.0f }, { 1.0f, 1.0f }, { -1.0f, 1.0f } };
    for ( UInt32 v=0; v<4; ++v )
    {
        Append( subMesh.positions, corners[ v ][ 0 ] * size, corners[ v ][ 1 ] * size, 0.1f * (Float32) v );
        Append( subMesh.normals, 0.0f, 0.0f, 1.0f );
        Append( subMesh.tangents, 1.0f, 0.0f, 0.0f, v % 2 ? -1.0f : 1.0f );
        subMesh.uvSets[ 0 ].push_back( corners[ v ][ 0 ] * 0.5f + 0.5f );
        subMesh.uvSets[ 0 ].push_back( corners[ v ][ 1 ] * 0.5f + 0.5f );
        subMesh.uvSets[ 1 ].push_back( 1.0f / 3.0f * (Float32) v );
        subMesh.uvSets[ 1 ].push_back( 0.1f );
        Append( subMesh.colours, 0.1f * (Float32) v, 0.5f, 1.0f - 0.1f * (Float32) v, 0.75f );
    }
    const UInt32 indices[ 6 ] = { 0, 1, 2, 0, 2, 3 };
    subMesh.indices.assign( indices, indices + 6 );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      BUILDING                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

void
BuildTriangleScene( CAssetScene& scene, const CORE::CString& name )
{GUCEF_TRACE;

    scene.name = name;
    CAssetTexture texture;
    texture.name = "tri.png";
    texture.sources.push_back( "work/" + name + "/tex/tri.png" );
    scene.textures.push_back( texture );

    CAssetMaterial material;
    material.name = "TriMaterial";
    material.baseColorTexture = CAssetTextureReference( 0 );
    scene.materials.push_back( material );

    const UInt32 node = scene.AddNode( "Triangle" );
    CAssetMesh mesh;
    mesh.name = "TriangleMesh";
    CAssetSubMesh subMesh;
    Append( subMesh.positions, 0.0f, 0.0f, 0.0f );
    Append( subMesh.positions, 1.0f, 0.0f, 0.0f );
    Append( subMesh.positions, 0.0f, 1.0f, 0.0f );
    for ( UInt32 v=0; v<3; ++v )
        Append( subMesh.normals, 0.0f, 0.0f, 1.0f );
    subMesh.uvSets[ 0 ].push_back( 0.0f ); subMesh.uvSets[ 0 ].push_back( 0.0f );
    subMesh.uvSets[ 0 ].push_back( 1.0f ); subMesh.uvSets[ 0 ].push_back( 0.0f );
    subMesh.uvSets[ 0 ].push_back( 0.0f ); subMesh.uvSets[ 0 ].push_back( 1.0f );
    subMesh.indices.push_back( 0 ); subMesh.indices.push_back( 1 ); subMesh.indices.push_back( 2 );
    subMesh.materialIndex = 0;
    mesh.subMeshes.push_back( subMesh );
    scene.meshes.push_back( mesh );
    scene.nodes[ node ].mesh = 0;
}

/*-------------------------------------------------------------------------*/

void
BuildFullScene( CAssetScene& scene, const CORE::CString& name )
{GUCEF_TRACE;

    scene.name = name;
    scene.extras.SetAttribute( "note", "scene extras" );

    // textures: one per sampler configuration
    {
        const char* const files[ 3 ] = { "albedo.png", "normal.png", "orm.png" };
        const TTextureUsage usages[ 3 ] = { TEXTUREUSAGE_COLOUR, TEXTUREUSAGE_NORMAL, TEXTUREUSAGE_MASK };
        const IMAGE::TWrapMode wraps[ 3 ] = { IMAGE::WRAPMODE_REPEAT, IMAGE::WRAPMODE_CLAMP, IMAGE::WRAPMODE_MIRROR };
        for ( UInt32 t=0; t<3; ++t )
        {
            CAssetTexture texture;
            texture.name = files[ t ];
            texture.sources.push_back( "work/" + name + "/tex/" + files[ t ] );
            texture.usage = usages[ t ];
            texture.wrapS = wraps[ t ];
            texture.wrapT = wraps[ ( t + 1 ) % 3 ];
            scene.textures.push_back( texture );
        }
    }

    // materials
    {
        CAssetMaterial body;
        body.name = "Body";
        body.baseColorFactor[ 0 ] = 0.8f; body.baseColorFactor[ 1 ] = 0.7f; body.baseColorFactor[ 2 ] = 0.6f; body.baseColorFactor[ 3 ] = 1.0f;
        body.baseColorTexture = CAssetTextureReference( 0, 1 );
        body.baseColorTexture.hasTransform = true;
        body.baseColorTexture.offset[ 0 ] = 0.25f; body.baseColorTexture.offset[ 1 ] = 0.5f;
        body.baseColorTexture.tiling[ 0 ] = 2.0f; body.baseColorTexture.tiling[ 1 ] = 3.0f;
        body.baseColorTexture.rotation = 0.5f;
        body.metallicFactor = 0.25f;
        body.roughnessFactor = 0.75f;
        body.metallicRoughnessTexture = CAssetTextureReference( 2 );
        body.normalTexture = CAssetTextureReference( 1 );
        body.normalTexture.scale = 0.8f;
        body.occlusionTexture = CAssetTextureReference( 2 );
        body.occlusionTexture.scale = 0.9f;
        body.emissiveFactor[ 0 ] = 0.1f; body.emissiveFactor[ 1 ] = 0.2f; body.emissiveFactor[ 2 ] = 0.3f;
        body.emissiveStrength = 4.0f;
        body.doubleSided = true;
        CORE::CDataNode* clearcoat = body.extensions.AddChild( "KHR_materials_clearcoat", GUCEF_DATATYPE_OBJECT );
        clearcoat->SetAttribute( "clearcoatFactor", 0.5f );
        clearcoat->SetAttribute( "clearcoatRoughnessFactor", 0.125f );
        scene.materials.push_back( body );

        CAssetMaterial glass;
        glass.name = "Glass";
        glass.baseColorFactor[ 3 ] = 0.5f;
        glass.alphaMode = ALPHAMODE_BLEND;
        glass.unlit = true;
        scene.materials.push_back( glass );

        CAssetMaterial leaf;
        leaf.name = "Leaf";
        leaf.alphaMode = ALPHAMODE_MASK;
        leaf.alphaCutoff = 0.35f;
        scene.materials.push_back( leaf );
    }

    // the hierarchy: 0 Root, 1 Hip, 2 Spine, 3 Head, 4 Body, 5 Prop, 6 Crate, 7 8 9 its levels, 10 Lamp, 11 Sun, 12 Spot, 13 Instance
    const UInt32 root = scene.AddNode( "Root" );
    SetTransform( scene.nodes[ root ], 1.0f, 2.0f, 3.0f, 0.0f, 0.70710678f, 0.0f, 0.70710678f );
    const UInt32 hip = scene.AddNode( "Hip", (Int32) root );
    SetTransform( scene.nodes[ hip ], 0.0f, 1.0f, 0.0f );
    const UInt32 spine = scene.AddNode( "Spine", (Int32) hip );
    SetTransform( scene.nodes[ spine ], 0.0f, 0.5f, 0.0f );
    const UInt32 head = scene.AddNode( "Head", (Int32) spine );
    SetTransform( scene.nodes[ head ], 0.0f, 0.5f, 0.0f );
    scene.nodes[ head ].transform.scale = MATH::TFloat32Transform3D::TVector( 1.5f, 1.5f, 1.5f );
    const UInt32 body = scene.AddNode( "Body", (Int32) root );
    const UInt32 prop = scene.AddNode( "Prop", (Int32) root );
    scene.nodes[ prop ].transform.position = MATH::TFloat32Transform3D::TPositionVector( -2.0f, 0.0f, 0.25f );
    const UInt32 crate = scene.AddNode( "Crate", (Int32) root );
    const UInt32 level0 = scene.AddNode( "Crate_LOD0", (Int32) crate );
    const UInt32 level1 = scene.AddNode( "Crate_LOD1", (Int32) crate );
    const UInt32 level2 = scene.AddNode( "Crate_LOD2", (Int32) crate );
    SetTransform( scene.nodes[ level1 ], 0.0f, 0.0f, 0.5f );
    const UInt32 lamp = scene.AddNode( "Lamp", (Int32) root );
    const UInt32 sun = scene.AddNode( "Sun", (Int32) root );
    SetTransform( scene.nodes[ sun ], 0.0f, 10.0f, 0.0f, -0.38268343f, 0.0f, 0.0f, 0.92387953f );
    const UInt32 spot = scene.AddNode( "Spot", (Int32) root );
    const UInt32 instance = scene.AddNode( "Instance", (Int32) root );
    scene.nodes[ instance ].prefabInstance = "Prefabs/Tree.prefab";

    scene.nodes[ body ].extras.SetAttribute( "tag", "body" );
    scene.nodes[ body ].extras.SetAttribute( "weight", 1.5f );
    CORE::CDataNode* nested = scene.nodes[ body ].extras.AddChild( "nested", GUCEF_DATATYPE_OBJECT );
    nested->SetAttribute( "count", (Int32) 3 );

    // the meshes
    {
        CAssetMesh bodyMesh;
        bodyMesh.name = "BodyMesh";
        CAssetSubMesh skinned;
        FillQuad( skinned, 1.0f );
        skinned.materialIndex = 0;
        skinned.influencesPerVertex = 6;
        const Float32 weightRows[ 4 ][ 6 ] = { { 0.3f, 0.25f, 0.2f, 0.1f, 0.1f, 0.05f }, { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
                                               { 0.125f, 0.125f, 0.25f, 0.25f, 0.125f, 0.125f }, { 0.5f, 0.2f, 0.2f, 0.05f, 0.05f, 0.0f } };
        for ( UInt32 v=0; v<4; ++v )
        {
            for ( UInt32 i=0; i<6; ++i )
            {
                skinned.joints.push_back( ( v + i ) % 3 );
                skinned.weights.push_back( weightRows[ v ][ i ] );
            }
        }
        bodyMesh.subMeshes.push_back( skinned );

        CAssetSubMesh plain;
        Append( plain.positions, 0.0f, 0.0f, 0.0f );
        Append( plain.positions, 2.0f, 0.0f, 0.0f );
        Append( plain.positions, 0.0f, 2.0f, 0.0f );
        plain.indices.push_back( 0 ); plain.indices.push_back( 1 ); plain.indices.push_back( 2 );
        plain.materialIndex = 1;
        bodyMesh.subMeshes.push_back( plain );
        scene.meshes.push_back( bodyMesh );

        CAssetMesh propMesh;
        propMesh.name = "PropMesh";
        CAssetSubMesh morphing;
        FillQuad( morphing, 0.5f );
        morphing.materialIndex = 2;
        CAssetMorphTarget smile;
        smile.name = "smile";
        smile.defaultWeight = 0.5f;
        CAssetMorphTarget blink;
        blink.name = "blink";
        for ( UInt32 v=0; v<4; ++v )
        {
            Append( smile.positionDeltas, 0.01f * (Float32) v, 0.02f, -0.03f );
            Append( smile.normalDeltas, 0.0f, 0.1f * (Float32) v, 0.0f );
            Append( blink.positionDeltas, 0.0f, -0.05f, 0.0f );
        }
        morphing.morphTargets.push_back( smile );
        morphing.morphTargets.push_back( blink );
        propMesh.subMeshes.push_back( morphing );
        scene.meshes.push_back( propMesh );

        CAssetMesh crateMesh;
        crateMesh.name = "CrateMesh";
        CAssetSubMesh crateSubMesh;
        FillQuad( crateSubMesh, 2.0f );
        crateSubMesh.materialIndex = 0;
        crateMesh.subMeshes.push_back( crateSubMesh );
        scene.meshes.push_back( crateMesh );
    }
    scene.nodes[ body ].mesh = 0;
    scene.nodes[ body ].skin = 0;
    scene.nodes[ prop ].mesh = 1;
    scene.nodes[ level0 ].mesh = 2;
    scene.nodes[ level1 ].mesh = 2;
    scene.nodes[ level2 ].mesh = 2;

    // the skin
    {
        CAssetSkin skin;
        skin.name = "Skeleton";
        skin.joints.push_back( hip );
        skin.joints.push_back( spine );
        skin.joints.push_back( head );
        skin.inverseBindMatrices.push_back( Translation( 0.0f, -1.0f, 0.0f ) );
        skin.inverseBindMatrices.push_back( Translation( 0.0f, -1.5f, 0.0f ) );
        skin.inverseBindMatrices.push_back( Translation( 0.0f, -2.0f, 0.1f ) );
        skin.skeletonRoot = (Int32) hip;
        scene.skins.push_back( skin );
    }

    // the animations
    {
        CAssetAnimation walk;
        walk.name = "Walk";
        walk.loop = true;
        walk.sampled = true;
        walk.frameRate = 24.0f;
        walk.firstFrame = 0.0f;
        walk.lastFrame = 48.0f;
        walk.sourceTake = "Take 001";

        CAssetAnimationChannel translation;
        translation.targetNode = hip;
        translation.path = ANIMATIONPATH_TRANSLATION;
        translation.times.push_back( 0.0f ); translation.times.push_back( 0.5f ); translation.times.push_back( 2.0f );
        Append( translation.values, 0.0f, 1.0f, 0.0f ); Append( translation.values, 0.0f, 1.2f, 0.3f ); Append( translation.values, 0.1f, 1.0f, 0.6f );
        walk.channels.push_back( translation );

        CAssetAnimationChannel rotation;
        rotation.targetNode = spine;
        rotation.path = ANIMATIONPATH_ROTATION;
        rotation.times.push_back( 0.0f ); rotation.times.push_back( 1.0f ); rotation.times.push_back( 2.0f );
        Append( rotation.values, 0.0f, 0.0f, 0.0f, 1.0f ); Append( rotation.values, 0.0f, 0.38268343f, 0.0f, 0.92387953f ); Append( rotation.values, 0.0f, 0.70710678f, 0.0f, 0.70710678f );
        walk.channels.push_back( rotation );

        CAssetAnimationChannel scale;
        scale.targetNode = head;
        scale.path = ANIMATIONPATH_SCALE;
        scale.interpolation = ANIMATIONINTERPOLATION_STEP;
        scale.times.push_back( 0.0f ); scale.times.push_back( 1.0f );
        Append( scale.values, 1.0f, 1.0f, 1.0f ); Append( scale.values, 2.0f, 2.0f, 2.0f );
        walk.channels.push_back( scale );

        CAssetAnimationChannel cubic;
        cubic.targetNode = head;
        cubic.path = ANIMATIONPATH_TRANSLATION;
        cubic.interpolation = ANIMATIONINTERPOLATION_CUBICSPLINE;
        cubic.times.push_back( 0.0f ); cubic.times.push_back( 1.0f );
        for ( UInt32 k=0; k<2; ++k )
        {
            Append( cubic.values, 0.0f, 0.0f, 0.0f );                                       // in tangent
            Append( cubic.values, 0.0f, 0.5f + 0.5f * (Float32) k, 0.0f );                  // value
            Append( cubic.values, 0.0f, 0.25f, 0.0f );                                      // out tangent
        }
        walk.channels.push_back( cubic );

        CAssetAnimationChannel weights;
        weights.targetNode = prop;
        weights.path = ANIMATIONPATH_WEIGHTS;
        weights.times.push_back( 0.0f ); weights.times.push_back( 1.0f );
        weights.values.push_back( 0.0f ); weights.values.push_back( 1.0f );
        weights.values.push_back( 1.0f ); weights.values.push_back( 0.25f );
        walk.channels.push_back( weights );
        scene.animations.push_back( walk );

        CAssetAnimation idle;
        idle.name = "Idle";
        CAssetAnimationChannel breathe;
        breathe.targetNode = spine;
        breathe.path = ANIMATIONPATH_SCALE;
        breathe.times.push_back( 0.0f ); breathe.times.push_back( 3.0f );
        Append( breathe.values, 1.0f, 1.0f, 1.0f ); Append( breathe.values, 1.0f, 1.05f, 1.0f );
        idle.channels.push_back( breathe );
        scene.animations.push_back( idle );
    }

    // lights
    {
        CAssetLight point;
        point.name = "LampLight";
        point.type = LIGHTTYPE_POINT;
        point.colour[ 0 ] = 1.0f; point.colour[ 1 ] = 0.9f; point.colour[ 2 ] = 0.8f;
        point.intensity = 3.0f;
        point.range = 12.5f;
        scene.lights.push_back( point );
        CAssetLight directional;
        directional.name = "SunLight";
        directional.type = LIGHTTYPE_DIRECTIONAL;
        directional.intensity = 2.5f;
        scene.lights.push_back( directional );
        CAssetLight spotLight;
        spotLight.name = "SpotLight";
        spotLight.type = LIGHTTYPE_SPOT;
        spotLight.innerConeAngle = 0.2f;
        spotLight.outerConeAngle = 0.6f;
        scene.lights.push_back( spotLight );
    }
    scene.nodes[ lamp ].light = 0;
    scene.nodes[ sun ].light = 1;
    scene.nodes[ spot ].light = 2;

    // the levels of detail of the crate
    {
        CAssetLodGroup group;
        group.name = "Crate";
        group.AddLevel( level0, 0.6f );
        group.AddLevel( level1, 0.3f );
        group.AddLevel( level2, 0.1f );
        scene.lodGroups.push_back( group );
        scene.nodes[ crate ].lodGroup = 0;
    }

    // colliders of every shape
    {
        CAssetCollider box;
        box.name = "PropBox";
        box.shape = COLLIDERSHAPE_BOX;
        box.halfExtents[ 0 ] = 0.5f; box.halfExtents[ 1 ] = 1.0f; box.halfExtents[ 2 ] = 1.5f;
        box.transform.position = MATH::TFloat32Transform3D::TPositionVector( 0.0f, 1.0f, 0.0f );
        scene.nodes[ prop ].colliders.push_back( box );
        CAssetCollider sphere;
        sphere.name = "PropZone";
        sphere.shape = COLLIDERSHAPE_SPHERE;
        sphere.radius = 0.75f;
        sphere.trigger = true;
        scene.nodes[ prop ].colliders.push_back( sphere );
        CAssetCollider convex;
        convex.name = "PropHull";
        convex.shape = COLLIDERSHAPE_CONVEX;
        convex.meshIndex = 2;
        scene.nodes[ prop ].colliders.push_back( convex );
        CAssetCollider capsule;
        capsule.name = "PropCapsule";
        capsule.shape = COLLIDERSHAPE_CAPSULE;
        capsule.height = 2.0f;
        capsule.radius = 0.4f;
        capsule.transform.rotation.x = 0.38268343f;
        capsule.transform.rotation.w = 0.92387953f;
        scene.nodes[ prop ].colliders.push_back( capsule );
        CAssetCollider cylinder;
        cylinder.name = "CrateCylinder";
        cylinder.shape = COLLIDERSHAPE_CYLINDER;
        cylinder.height = 1.5f;
        cylinder.radius = 0.6f;
        scene.nodes[ crate ].colliders.push_back( cylinder );
        CAssetCollider trimesh;
        trimesh.name = "CrateMesh";
        trimesh.shape = COLLIDERSHAPE_TRIMESH;
        trimesh.meshIndex = 2;
        scene.nodes[ crate ].colliders.push_back( trimesh );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      COMPARING                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

#define CHECK( condition, text ) do { if ( !( condition ) ) { problem = CORE::CString( text ); return false; } } while ( 0 )

bool
SameFloats( const GUCEF::vector< Float32 >& a, const GUCEF::vector< Float32 >& b, Float32 tolerance )
{
    if ( a.size() != b.size() )
        return false;
    for ( size_t i=0; i<a.size(); ++i )
        if ( !Near( a[ i ], b[ i ], tolerance ) )
            return false;
    return true;
}

bool
SameReference( const CAssetTextureReference& a, const CAssetTextureReference& b )
{
    if ( a.textureIndex != b.textureIndex )
        return false;
    if ( a.textureIndex < 0 )
        return true;
    if ( a.texCoord != b.texCoord || !Near( a.scale, b.scale ) || a.hasTransform != b.hasTransform )
        return false;
    return !a.hasTransform || ( Near( a.offset[ 0 ], b.offset[ 0 ] ) && Near( a.offset[ 1 ], b.offset[ 1 ] ) && Near( a.tiling[ 0 ], b.tiling[ 0 ] ) && Near( a.tiling[ 1 ], b.tiling[ 1 ] ) && Near( a.rotation, b.rotation ) );
}

bool
SameTransform( const MATH::TFloat32Transform3D& a, const MATH::TFloat32Transform3D& b )
{
    return Near( a.position.x, b.position.x ) && Near( a.position.y, b.position.y ) && Near( a.position.z, b.position.z ) &&
           Near( a.rotation.x, b.rotation.x ) && Near( a.rotation.y, b.rotation.y ) && Near( a.rotation.z, b.rotation.z ) && Near( a.rotation.w, b.rotation.w ) &&
           Near( a.scale.x, b.scale.x ) && Near( a.scale.y, b.scale.y ) && Near( a.scale.z, b.scale.z );
}

}

bool
CompareScenes( const CAssetScene& a, const CAssetScene& b, CORE::CString& problem )
{GUCEF_TRACE;

    // nodes
    CHECK( a.nodes.size() == b.nodes.size(), "node count" );
    CHECK( a.rootNodes == b.rootNodes, "root nodes" );
    for ( size_t n=0; n<a.nodes.size(); ++n )
    {
        const CORE::CString at = " (node " + a.nodes[ n ].name + ")";
        const CAssetNode& x = a.nodes[ n ];
        const CAssetNode& y = b.nodes[ n ];
        CHECK( x.name == y.name, "node name" + at );
        CHECK( x.parent == y.parent, "node parent" + at );
        CHECK( x.children == y.children, "node children" + at );
        CHECK( SameTransform( x.transform, y.transform ), "node transform" + at );
        CHECK( x.mesh == y.mesh && x.skin == y.skin && x.light == y.light && x.lodGroup == y.lodGroup, "node references" + at );
        CHECK( x.prefabInstance == y.prefabInstance, "node prefab instance" + at );
        CHECK( CCanonicalJson::ToJson( x.extras ) == CCanonicalJson::ToJson( y.extras ), "node extras" + at );
        CHECK( x.colliders.size() == y.colliders.size(), "collider count" + at );
        for ( size_t c=0; c<x.colliders.size(); ++c )
        {
            const CAssetCollider& p = x.colliders[ c ];
            const CAssetCollider& q = y.colliders[ c ];
            CHECK( p.name == q.name && p.shape == q.shape && p.trigger == q.trigger && p.meshIndex == q.meshIndex, "collider kind" + at );
            CHECK( SameTransform( p.transform, q.transform ), "collider transform" + at );
            switch ( p.shape )
            {
                case COLLIDERSHAPE_BOX : CHECK( Near( p.halfExtents[ 0 ], q.halfExtents[ 0 ] ) && Near( p.halfExtents[ 1 ], q.halfExtents[ 1 ] ) && Near( p.halfExtents[ 2 ], q.halfExtents[ 2 ] ), "box extents" + at ); break;
                case COLLIDERSHAPE_SPHERE : CHECK( Near( p.radius, q.radius ), "sphere radius" + at ); break;
                case COLLIDERSHAPE_CAPSULE :
                case COLLIDERSHAPE_CYLINDER : CHECK( Near( p.radius, q.radius ) && Near( p.height, q.height ), "capsule or cylinder size" + at ); break;
                default : break;
            }
        }
    }

    // meshes
    CHECK( a.meshes.size() == b.meshes.size(), "mesh count" );
    for ( size_t m=0; m<a.meshes.size(); ++m )
    {
        const CORE::CString at = " (mesh " + a.meshes[ m ].name + ")";
        CHECK( a.meshes[ m ].name == b.meshes[ m ].name, "mesh name" + at );
        CHECK( a.meshes[ m ].subMeshes.size() == b.meshes[ m ].subMeshes.size(), "submesh count" + at );
        for ( size_t s=0; s<a.meshes[ m ].subMeshes.size(); ++s )
        {
            const CAssetSubMesh& x = a.meshes[ m ].subMeshes[ s ];
            const CAssetSubMesh& y = b.meshes[ m ].subMeshes[ s ];
            CHECK( SameFloats( x.positions, y.positions, 0.0f ), "positions" + at );
            CHECK( SameFloats( x.normals, y.normals, 0.0f ), "normals" + at );
            CHECK( SameFloats( x.tangents, y.tangents, 0.0f ), "tangents" + at );
            for ( UInt32 uv=0; uv<CAssetSubMesh::MaxUvSets; ++uv )
                CHECK( SameFloats( x.uvSets[ uv ], y.uvSets[ uv ], 0.0f ), "uv set" + at );
            CHECK( SameFloats( x.colours, y.colours, 0.0f ), "colours" + at );
            CHECK( x.indices == y.indices, "indices" + at );
            CHECK( x.materialIndex == y.materialIndex, "material index" + at );
            CHECK( x.influencesPerVertex == y.influencesPerVertex && x.joints == y.joints, "joints" + at );
            CHECK( SameFloats( x.weights, y.weights, 0.0f ), "weights" + at );
            CHECK( x.morphTargets.size() == y.morphTargets.size(), "morph target count" + at );
            for ( size_t t=0; t<x.morphTargets.size(); ++t )
            {
                CHECK( x.morphTargets[ t ].name == y.morphTargets[ t ].name && Near( x.morphTargets[ t ].defaultWeight, y.morphTargets[ t ].defaultWeight, 0.0f ), "morph target name or weight" + at );
                CHECK( SameFloats( x.morphTargets[ t ].positionDeltas, y.morphTargets[ t ].positionDeltas, 0.0f ), "morph position deltas" + at );
                CHECK( SameFloats( x.morphTargets[ t ].normalDeltas, y.morphTargets[ t ].normalDeltas, 0.0f ), "morph normal deltas" + at );
                CHECK( SameFloats( x.morphTargets[ t ].tangentDeltas, y.morphTargets[ t ].tangentDeltas, 0.0f ), "morph tangent deltas" + at );
            }
        }
    }

    // skins
    CHECK( a.skins.size() == b.skins.size(), "skin count" );
    for ( size_t s=0; s<a.skins.size(); ++s )
    {
        CHECK( a.skins[ s ].name == b.skins[ s ].name && a.skins[ s ].joints == b.skins[ s ].joints && a.skins[ s ].skeletonRoot == b.skins[ s ].skeletonRoot, "skin joints" );
        CHECK( a.skins[ s ].inverseBindMatrices.size() == b.skins[ s ].inverseBindMatrices.size(), "bind matrix count" );
        for ( size_t m=0; m<a.skins[ s ].inverseBindMatrices.size(); ++m )
            for ( UInt32 e=0; e<16; ++e )
                CHECK( Near( a.skins[ s ].inverseBindMatrices[ m ].GetData()[ e ], b.skins[ s ].inverseBindMatrices[ m ].GetData()[ e ] ), "bind matrix" );
    }

    // animations
    CHECK( a.animations.size() == b.animations.size(), "animation count" );
    for ( size_t n=0; n<a.animations.size(); ++n )
    {
        const CAssetAnimation& x = a.animations[ n ];
        const CAssetAnimation& y = b.animations[ n ];
        const CORE::CString at = " (animation " + x.name + ")";
        CHECK( x.name == y.name && x.loop == y.loop && x.sampled == y.sampled && x.sourceTake == y.sourceTake, "animation facts" + at );
        CHECK( Near( x.frameRate, y.frameRate ) && Near( x.firstFrame, y.firstFrame ) && Near( x.lastFrame, y.lastFrame ), "animation frames" + at );
        CHECK( x.channels.size() == y.channels.size(), "channel count" + at );
        for ( size_t c=0; c<x.channels.size(); ++c )
        {
            CHECK( x.channels[ c ].targetNode == y.channels[ c ].targetNode && x.channels[ c ].path == y.channels[ c ].path && x.channels[ c ].interpolation == y.channels[ c ].interpolation, "channel kind" + at );
            CHECK( SameFloats( x.channels[ c ].times, y.channels[ c ].times, 1e-6f ), "channel times" + at );
            CHECK( SameFloats( x.channels[ c ].values, y.channels[ c ].values, 1e-6f ), "channel values" + at );
        }
    }

    // materials and textures
    CHECK( a.materials.size() == b.materials.size(), "material count" );
    for ( size_t m=0; m<a.materials.size(); ++m )
    {
        const CAssetMaterial& x = a.materials[ m ];
        const CAssetMaterial& y = b.materials[ m ];
        const CORE::CString at = " (material " + x.name + ")";
        CHECK( x.name == y.name && x.alphaMode == y.alphaMode && x.doubleSided == y.doubleSided && x.unlit == y.unlit, "material facts" + at );
        for ( UInt32 c=0; c<4; ++c )
            CHECK( Near( x.baseColorFactor[ c ], y.baseColorFactor[ c ] ), "base colour" + at );
        for ( UInt32 c=0; c<3; ++c )
            CHECK( Near( x.emissiveFactor[ c ], y.emissiveFactor[ c ] ), "emissive" + at );
        CHECK( Near( x.metallicFactor, y.metallicFactor ) && Near( x.roughnessFactor, y.roughnessFactor ) && Near( x.emissiveStrength, y.emissiveStrength ) && Near( x.alphaCutoff, y.alphaCutoff ), "material factors" + at );
        CHECK( SameReference( x.baseColorTexture, y.baseColorTexture ) && SameReference( x.metallicRoughnessTexture, y.metallicRoughnessTexture ) && SameReference( x.normalTexture, y.normalTexture ) &&
               SameReference( x.occlusionTexture, y.occlusionTexture ) && SameReference( x.emissiveTexture, y.emissiveTexture ), "texture references" + at );
        CHECK( CCanonicalJson::ToJson( x.extensions ) == CCanonicalJson::ToJson( y.extensions ), "material extensions" + at );
    }
    CHECK( a.textures.size() == b.textures.size(), "texture count" );
    for ( size_t t=0; t<a.textures.size(); ++t )
    {
        CHECK( a.textures[ t ].name == b.textures[ t ].name && a.textures[ t ].usage == b.textures[ t ].usage && a.textures[ t ].wrapS == b.textures[ t ].wrapS && a.textures[ t ].wrapT == b.textures[ t ].wrapT, "texture facts" );
        CHECK( a.textures[ t ].sources.size() == b.textures[ t ].sources.size() && ( a.textures[ t ].sources.empty() || a.textures[ t ].sources[ 0 ] == b.textures[ t ].sources[ 0 ] ), "texture source" );
    }

    // lights and LOD groups
    CHECK( a.lights.size() == b.lights.size(), "light count" );
    for ( size_t l=0; l<a.lights.size(); ++l )
    {
        const CAssetLight& x = a.lights[ l ];
        const CAssetLight& y = b.lights[ l ];
        CHECK( x.name == y.name && x.type == y.type && Near( x.intensity, y.intensity ) && Near( x.range, y.range ), "light facts" );
        CHECK( Near( x.colour[ 0 ], y.colour[ 0 ] ) && Near( x.colour[ 1 ], y.colour[ 1 ] ) && Near( x.colour[ 2 ], y.colour[ 2 ] ), "light colour" );
        if ( LIGHTTYPE_SPOT == x.type )
            CHECK( Near( x.innerConeAngle, y.innerConeAngle ) && Near( x.outerConeAngle, y.outerConeAngle ), "spot angles" );
    }
    CHECK( a.lodGroups.size() == b.lodGroups.size(), "LOD group count" );
    for ( size_t g=0; g<a.lodGroups.size(); ++g )
    {
        CHECK( a.lodGroups[ g ].name == b.lodGroups[ g ].name && a.lodGroups[ g ].levels.size() == b.lodGroups[ g ].levels.size(), "LOD group facts" );
        for ( size_t l=0; l<a.lodGroups[ g ].levels.size(); ++l )
            CHECK( a.lodGroups[ g ].levels[ l ].node == b.lodGroups[ g ].levels[ l ].node && Near( a.lodGroups[ g ].levels[ l ].screenRelativeHeight, b.lodGroups[ g ].levels[ l ].screenRelativeHeight ), "LOD level" );
    }

    // the summary the sidecar carries
    CAssetSceneSummary summaryA;
    summaryA.Compute( a );
    CAssetSceneSummary summaryB;
    summaryB.Compute( b );
    CORE::CDataNode nodeA( "summary", GUCEF_DATATYPE_OBJECT );
    CORE::CDataNode nodeB( "summary", GUCEF_DATATYPE_OBJECT );
    summaryA.Serialize( nodeA );
    summaryB.Serialize( nodeB );
    CHECK( CCanonicalJson::ToJson( nodeA ) == CCanonicalJson::ToJson( nodeB ), "scene summary" );
    return true;
}

/*-------------------------------------------------------------------------*/
