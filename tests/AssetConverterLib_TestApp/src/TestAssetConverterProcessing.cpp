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

#include <math.h>
#include <algorithm>

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#include "TestAssetConverterSupport.h"
#include "TestAssetConverterProcessing.h"

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

/** A box of the sizes around the origin of the node: 8 corners, 12 triangles */
Int32
AddBoxMesh( CAssetScene& scene, UInt32 node, Float32 sx, Float32 sy, Float32 sz )
{
    CAssetMesh mesh;
    mesh.name = scene.nodes[ node ].name;
    CAssetSubMesh subMesh;
    for ( UInt32 i=0; i<8; ++i )
    {
        subMesh.positions.push_back( ( i & 1 ) ? sx * 0.5f : -sx * 0.5f );
        subMesh.positions.push_back( ( i & 2 ) ? sy * 0.5f : -sy * 0.5f );
        subMesh.positions.push_back( ( i & 4 ) ? sz * 0.5f : -sz * 0.5f );
    }
    const UInt32 faces[] = { 0,2,1, 1,2,3,  4,5,6, 5,7,6,  0,1,4, 1,5,4,  2,6,3, 3,6,7,  0,4,2, 2,4,6,  1,3,5, 3,7,5 };
    for ( UInt32 i=0; i<sizeof( faces ) / sizeof( faces[ 0 ] ); ++i )
        subMesh.indices.push_back( faces[ i ] );
    mesh.subMeshes.push_back( subMesh );
    scene.meshes.push_back( mesh );
    scene.nodes[ node ].mesh = (Int32) scene.meshes.size() - 1;
    return scene.nodes[ node ].mesh;
}

UInt32
CountItems( const CConversionReport& report, TReportItemKind kind, const CORE::CString& source )
{
    UInt32 count = 0;
    const CConversionReport::TEntryVector& entries = report.GetEntries();
    for ( size_t e=0; e<entries.size(); ++e )
        for ( size_t i=0; i<entries[ e ].items.size(); ++i )
            if ( entries[ e ].items[ i ].kind == kind && entries[ e ].items[ i ].source == source )
                ++count;
    return count;
}

const CAssetCollider*
FindCollider( const CAssetNode& node, TColliderShape shape )
{
    for ( size_t c=0; c<node.colliders.size(); ++c )
        if ( node.colliders[ c ].shape == shape )
            return &node.colliders[ c ];
    return GUCEF_NULL;
}

bool
SameRotation( const MATH::TFloat32Quaternion& a, const MATH::TFloat32Quaternion& b )
{
    const Float32 dot = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    return fabs( fabs( dot ) - 1.0f ) < 1e-4f;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAssetConverterProcessingTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AssetConverterLib.Processing" );

    GUCEF_TESTFW_TESTCASE( "Normalisation: a left-handed Y up source is mirrored into glTF conventions, DirectX normal maps get a green flip" )
    try
    {
        CAssetScene scene;
        scene.coordinateSystem = MATH::CCoordinateSystem::Unity();
        scene.normalMapConvention = NORMALMAPCONVENTION_DIRECTX;
        const UInt32 node = scene.AddNode( "Node" );
        scene.nodes[ node ].transform.position = MATH::TFloat32Transform3D::TPositionVector( 1.0f, 2.0f, 3.0f );
        scene.nodes[ node ].transform.scale = MATH::TFloat32Vector3D( 1.0f, 2.0f, 3.0f );
        const Float32 s = (Float32) sqrt( 0.5 );
        scene.nodes[ node ].transform.rotation = MATH::TFloat32Quaternion::FromXYZW( 0.0f, s, 0.0f, s );
        CAssetCollider box;
        box.shape = COLLIDERSHAPE_BOX;
        box.halfExtents[ 0 ] = 1.0f; box.halfExtents[ 1 ] = 2.0f; box.halfExtents[ 2 ] = 3.0f;
        box.transform.position = MATH::TFloat32Transform3D::TPositionVector( 0.0f, 0.0f, 1.0f );
        scene.nodes[ node ].colliders.push_back( box );

        CAssetMesh mesh;
        CAssetSubMesh subMesh;
        const Float32 positions[] = { 1,2,3,  4,5,6,  7,8,9 };
        for ( UInt32 i=0; i<9; ++i )
            subMesh.positions.push_back( positions[ i ] );
        for ( UInt32 v=0; v<3; ++v )
        {
            subMesh.normals.push_back( 0.0f ); subMesh.normals.push_back( 0.0f ); subMesh.normals.push_back( 1.0f );
            subMesh.tangents.push_back( 1.0f ); subMesh.tangents.push_back( 0.0f ); subMesh.tangents.push_back( 0.0f ); subMesh.tangents.push_back( 1.0f );
        }
        subMesh.indices.push_back( 0 ); subMesh.indices.push_back( 1 ); subMesh.indices.push_back( 2 );
        mesh.subMeshes.push_back( subMesh );
        scene.meshes.push_back( mesh );
        scene.nodes[ node ].mesh = 0;

        // one normal map used by two materials is flipped once
        CAssetTexture normalMap;
        normalMap.name = "normal";
        normalMap.usage = TEXTUREUSAGE_NORMAL;
        scene.textures.push_back( normalMap );
        CAssetMaterial material;
        material.normalTexture = CAssetTextureReference( 0 );
        scene.materials.push_back( material );
        scene.materials.push_back( material );

        CConversionReport report;
        report.SelectAsset( "unity.stub" );
        ASSERT_TRUE( CAssetNormaliser::Normalise( scene, report ) );
        const CAssetSubMesh& converted = scene.meshes[ 0 ].subMeshes[ 0 ];
        ASSERT_TRUE( Near( 1.0f, converted.positions[ 0 ] ) && Near( 2.0f, converted.positions[ 1 ] ) && Near( -3.0f, converted.positions[ 2 ] ) );
        ASSERT_TRUE( Near( -9.0f, converted.positions[ 8 ] ) );
        ASSERT_TRUE( Near( -1.0f, converted.normals[ 2 ] ) && Near( 1.0f, converted.tangents[ 0 ] ) && Near( -1.0f, converted.tangents[ 3 ] ) );
        ASSERT_TRUE( 0 == converted.indices[ 0 ] && 2 == converted.indices[ 1 ] && 1 == converted.indices[ 2 ] );
        const MATH::TFloat32Transform3D& transform = scene.nodes[ node ].transform;
        ASSERT_TRUE( Near( 1.0f, transform.position.x ) && Near( 2.0f, transform.position.y ) && Near( -3.0f, transform.position.z ) );
        ASSERT_TRUE( Near( 1.0f, transform.scale.x ) && Near( 2.0f, transform.scale.y ) && Near( 3.0f, transform.scale.z ) );
        ASSERT_TRUE( SameRotation( MATH::TFloat32Quaternion::FromXYZW( 0.0f, -s, 0.0f, s ), transform.rotation ) );
        const CAssetCollider& collider = scene.nodes[ node ].colliders[ 0 ];
        ASSERT_TRUE( Near( -1.0f, collider.transform.position.z ) && Near( 2.0f, collider.halfExtents[ 1 ] ) && Near( 3.0f, collider.halfExtents[ 2 ] ) );

        ASSERT_TRUE( 1 == scene.textures[ 0 ].operations.size() && TEXTUREOPERATION_GREENFLIP == scene.textures[ 0 ].operations[ 0 ].type );
        ASSERT_TRUE( NORMALMAPCONVENTION_OPENGL == scene.normalMapConvention && scene.coordinateSystem == MATH::CCoordinateSystem::GucefDefault() );
        ASSERT_TRUE( scene.normalisation.GetAttributeValueOrChildValueByName( "mirrored" ).AsBool( false ) );
        ASSERT_TRUE( 1 == scene.normalisation.GetAttributeValueOrChildValueByName( "normalMapsFlipped" ).AsUInt32() );
        ASSERT_TRUE( MATH::CCoordinateSystem::Unity().ToString() == scene.normalisation.GetAttributeValueOrChildValueByName( "from" ).AsString() );
        ASSERT_TRUE( 2 == report.GetItemCount( REPORTITEM_INFO ) );

        // normalised again: nothing changes
        ASSERT_TRUE( CAssetNormaliser::Normalise( scene, report ) );
        ASSERT_TRUE( Near( -3.0f, scene.meshes[ 0 ].subMeshes[ 0 ].positions[ 2 ] ) && 1 == scene.textures[ 0 ].operations.size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Normalisation: centimetres and Z up become metres and Y up, skins stay bound; settings override the system of the importer" )
    try
    {
        const MATH::CCoordinateSystem unreal = MATH::CCoordinateSystem::Unreal();
        const MATH::CCoordinateSystem target = MATH::CCoordinateSystem::GucefDefault();
        const MATH::TFloat32Matrix4D conversion = unreal.GetConversionMatrix4To< Float32 >( target );

        CAssetScene scene;
        scene.coordinateSystem = unreal;
        const UInt32 root = scene.AddNode( "Root" );
        const UInt32 joint = scene.AddNode( "Joint", (Int32) root );
        scene.nodes[ root ].transform.position = MATH::TFloat32Transform3D::TPositionVector( 100.0f, 0.0f, 50.0f );
        scene.nodes[ joint ].transform.position = MATH::TFloat32Transform3D::TPositionVector( 0.0f, 30.0f, 0.0f );
        scene.nodes[ joint ].transform.rotation = MATH::TFloat32Quaternion::FromXYZW( 0.0f, 0.0f, (Float32) sin( 0.3 ), (Float32) cos( 0.3 ) );
        CAssetSkin skin;
        skin.joints.push_back( joint );
        skin.inverseBindMatrices.push_back( scene.GetWorldMatrix( joint ).Inverse() );
        scene.skins.push_back( skin );
        CAssetMesh mesh;
        CAssetSubMesh subMesh;
        subMesh.positions.push_back( 100.0f ); subMesh.positions.push_back( 200.0f ); subMesh.positions.push_back( 300.0f );
        mesh.subMeshes.push_back( subMesh );
        scene.meshes.push_back( mesh );
        CAssetAnimation animation;
        CAssetAnimationChannel channel;
        channel.targetNode = joint;
        channel.times.push_back( 0.0f );
        channel.values.push_back( 100.0f ); channel.values.push_back( 200.0f ); channel.values.push_back( 300.0f );
        animation.channels.push_back( channel );
        scene.animations.push_back( animation );

        CConversionReport report;
        ASSERT_TRUE( CAssetNormaliser::Normalise( scene, report ) );
        const MATH::TFloat32Point3D expected = conversion.TransformPoint( MATH::TFloat32Point3D( 100.0f, 200.0f, 300.0f ) );
        const CAssetSubMesh& converted = scene.meshes[ 0 ].subMeshes[ 0 ];
        ASSERT_TRUE( Near( expected.x, converted.positions[ 0 ] ) && Near( expected.y, converted.positions[ 1 ] ) && Near( expected.z, converted.positions[ 2 ] ) );
        ASSERT_TRUE( Near( 3.0f, converted.positions[ 1 ] ) );    // 300 cm up is 3 m along +Y
        const CAssetAnimationChannel& keys = scene.animations[ 0 ].channels[ 0 ];
        ASSERT_TRUE( Near( expected.x, keys.values[ 0 ] ) && Near( expected.y, keys.values[ 1 ] ) && Near( expected.z, keys.values[ 2 ] ) );
        ASSERT_TRUE( Near( 0.01f, scene.normalisation.GetAttributeValueOrChildValueByName( "scale" ).AsFloat32() ) );

        // the joint still sits where its inverse bind matrix says: the product is the identity
        const MATH::TFloat32Matrix4D bound = scene.skins[ 0 ].inverseBindMatrices[ 0 ] * scene.GetWorldMatrix( joint );
        bool identity = true;
        for ( UInt32 r=0; r<4; ++r )
            for ( UInt32 c=0; c<4; ++c )
                identity = identity && Near( r == c ? 1.0f : 0.0f, bound( r, c ), 1e-3f );
        ASSERT_TRUE( identity );
        const MATH::TFloat32Point3D jointWorld = conversion.TransformPoint( MATH::TFloat32Point3D( 100.0f, 30.0f, 50.0f ) );
        const MATH::TFloat32Matrix4D world = scene.GetWorldMatrix( joint );
        ASSERT_TRUE( Near( jointWorld.x, world( 0, 3 ), 1e-3f ) && Near( jointWorld.y, world( 1, 3 ), 1e-3f ) && Near( jointWorld.z, world( 2, 3 ), 1e-3f ) );

        // the units of the settings over those of the importer
        CAssetScene scaled;
        scaled.settings.SetAttribute( "unitsPerMetre", (UInt32) 100 );
        scaled.meshes.push_back( mesh );
        ASSERT_TRUE( CAssetNormaliser::Normalise( scaled, report ) );
        ASSERT_TRUE( Near( 1.0f, scaled.meshes[ 0 ].subMeshes[ 0 ].positions[ 0 ] ) && Near( 3.0f, scaled.meshes[ 0 ].subMeshes[ 0 ].positions[ 2 ] ) );

        // a whole system from the settings: Z up becomes Y up
        CAssetScene blender;
        blender.settings.SetAttribute( "coordinateSystem", MATH::CCoordinateSystem::Blender().ToString() );
        blender.meshes.push_back( mesh );
        ASSERT_TRUE( CAssetNormaliser::Normalise( blender, report ) );
        ASSERT_TRUE( Near( 300.0f, blender.meshes[ 0 ].subMeshes[ 0 ].positions[ 1 ] ) );

        // a setting that is no coordinate system is reported, and the system of the importer stays
        CAssetScene garbage;
        garbage.settings.SetAttribute( "coordinateSystem", "sideways" );
        garbage.meshes.push_back( mesh );
        const UInt32 warnings = report.GetItemCount( REPORTITEM_WARNING );
        ASSERT_TRUE( CAssetNormaliser::Normalise( garbage, report ) && warnings + 1 == report.GetItemCount( REPORTITEM_WARNING ) );
        ASSERT_TRUE( Near( 100.0f, garbage.meshes[ 0 ].subMeshes[ 0 ].positions[ 0 ] ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Normalisation: an invalid coordinate system of the importer is refused with an error, the scene stays as it is" )
    try
    {
        // the validity assert of the constructor is off in this test application (GUCEF_MATH_NO_COORDINATESYSTEM_VALIDITY_ASSERT)
        const MATH::CCoordinateSystem invalid( MATH::SIGNEDAXIS_POSITIVE_X, MATH::SIGNEDAXIS_POSITIVE_X, MATH::SIGNEDAXIS_NEGATIVE_Z, 1.0 );
        ASSERT_FALSE( invalid.IsValid() );

        CAssetMesh mesh;
        CAssetSubMesh subMesh;
        subMesh.positions.push_back( 1.0f ); subMesh.positions.push_back( 2.0f ); subMesh.positions.push_back( 3.0f );
        mesh.subMeshes.push_back( subMesh );

        CAssetScene scene;
        scene.coordinateSystem = invalid;
        scene.meshes.push_back( mesh );
        CConversionReport report;
        ASSERT_FALSE( CAssetNormaliser::Normalise( scene, report ) );
        ASSERT_TRUE( 1 == report.GetItemCount( REPORTITEM_ERROR ) );
        ASSERT_TRUE( scene.coordinateSystem == invalid );
        ASSERT_TRUE( Near( 1.0f, scene.meshes[ 0 ].subMeshes[ 0 ].positions[ 0 ] ) && Near( 3.0f, scene.meshes[ 0 ].subMeshes[ 0 ].positions[ 2 ] ) );

        // units of the settings do not make the axes valid
        CAssetScene scaled;
        scaled.coordinateSystem = invalid;
        scaled.settings.SetAttribute( "unitsPerMetre", (UInt32) 100 );
        scaled.meshes.push_back( mesh );
        ASSERT_FALSE( CAssetNormaliser::Normalise( scaled, report ) );
        ASSERT_TRUE( 2 == report.GetItemCount( REPORTITEM_ERROR ) );

        // a whole system from the settings takes the place of the invalid one
        CAssetScene overridden;
        overridden.coordinateSystem = invalid;
        overridden.settings.SetAttribute( "coordinateSystem", MATH::CCoordinateSystem::Blender().ToString() );
        overridden.meshes.push_back( mesh );
        ASSERT_TRUE( CAssetNormaliser::Normalise( overridden, report ) );
        ASSERT_TRUE( 2 == report.GetItemCount( REPORTITEM_ERROR ) );
        ASSERT_TRUE( overridden.coordinateSystem == MATH::CCoordinateSystem::GucefDefault() );
        ASSERT_TRUE( Near( 3.0f, overridden.meshes[ 0 ].subMeshes[ 0 ].positions[ 1 ] ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "NameHints: Unreal collision prefixes become colliders of their mesh, fitted to the vertices" )
    try
    {
        CAssetScene scene;
        const UInt32 root = scene.AddNode( "Root" );
        const UInt32 rock = scene.AddNode( "Rock", (Int32) root );
        scene.nodes[ rock ].transform.position = MATH::TFloat32Transform3D::TPositionVector( 5.0f, 0.0f, 0.0f );
        AddBoxMesh( scene, rock, 2.0f, 4.0f, 6.0f );
        const char* const names[] = { "UBX_Rock", "USP_Rock_01", "UCP_Rock", "UCX_Rock" };
        for ( UInt32 i=0; i<4; ++i )
        {
            const UInt32 node = scene.AddNode( names[ i ], (Int32) root );
            scene.nodes[ node ].transform.position = MATH::TFloat32Transform3D::TPositionVector( 5.0f, 0.0f, 0.0f );
            if ( 2 == i )
                AddBoxMesh( scene, node, 2.0f, 2.0f, 10.0f );
            else
                AddBoxMesh( scene, node, 2.0f, 4.0f, 6.0f );
        }
        const UInt32 orphan = scene.AddNode( "UBX_Nothing", (Int32) root );
        AddBoxMesh( scene, orphan, 1.0f, 1.0f, 1.0f );

        CNameHintsPostProcessor processor;
        CORE::CDataNode settings( "settings", GUCEF_DATATYPE_OBJECT );
        settings.SetAttribute( "set", "unreal" );
        CConversionReport report;
        report.SelectAsset( "rock.stub" );
        ASSERT_TRUE( processor.Process( scene, settings, report ) );

        // the collision nodes are gone, the one without a mesh to belong to stays
        ASSERT_TRUE( 3 == scene.nodes.size() && scene.FindNode( "UBX_Nothing" ) >= 0 && scene.FindNode( "UCX_Rock" ) < 0 );
        ASSERT_TRUE( 1 == CountItems( report, REPORTITEM_WARNING, "NameHints" ) );
        const CAssetNode& rockNode = scene.nodes[ scene.FindNode( "Rock" ) ];
        ASSERT_TRUE( 4 == rockNode.colliders.size() );

        const CAssetCollider* fittedBox = FindCollider( rockNode, COLLIDERSHAPE_BOX );
        ASSERT_TRUE( GUCEF_NULL != fittedBox );
        if ( GUCEF_NULL != fittedBox )
        {
            Float32 extents[ 3 ] = { fittedBox->halfExtents[ 0 ], fittedBox->halfExtents[ 1 ], fittedBox->halfExtents[ 2 ] };
            std::sort( extents, extents + 3 );
            ASSERT_TRUE( Near( 1.0f, extents[ 0 ], 1e-3f ) && Near( 2.0f, extents[ 1 ], 1e-3f ) && Near( 3.0f, extents[ 2 ], 1e-3f ) );
            ASSERT_TRUE( Near( 0.0f, fittedBox->transform.position.x, 1e-3f ) );   // in the frame of Rock
        }
        const CAssetCollider* sphere = FindCollider( rockNode, COLLIDERSHAPE_SPHERE );
        ASSERT_TRUE( GUCEF_NULL != sphere && Near( (Float32) sqrt( 14.0 ), sphere->radius, 1e-3f ) );
        const CAssetCollider* capsule = FindCollider( rockNode, COLLIDERSHAPE_CAPSULE );
        ASSERT_TRUE( GUCEF_NULL != capsule );
        if ( GUCEF_NULL != capsule )
        {
            ASSERT_TRUE( Near( (Float32) sqrt( 2.0 ), capsule->radius, 1e-3f ) && Near( 2.0f * ( 5.0f + (Float32) sqrt( 2.0 ) ), capsule->height, 1e-2f ) );
            const MATH::TFloat32Vector3D axis = capsule->transform.rotation.Rotate( MATH::TFloat32Vector3D( 0.0f, 1.0f, 0.0f ) );
            ASSERT_TRUE( Near( 1.0f, (Float32) fabs( axis.z ), 1e-3f ) );
        }
        const CAssetCollider* convex = FindCollider( rockNode, COLLIDERSHAPE_CONVEX );
        ASSERT_TRUE( GUCEF_NULL != convex && convex->meshIndex >= 0 && (size_t) convex->meshIndex < scene.meshes.size() );
        if ( GUCEF_NULL != convex && convex->meshIndex >= 0 )
        {
            ASSERT_TRUE( 8 == scene.meshes[ convex->meshIndex ].GetVertexCount() && 12 == scene.meshes[ convex->meshIndex ].GetTriangleCount() );
        }
        // the meshes of the collision nodes went, except the hull of the convex collider
        ASSERT_TRUE( 3 == scene.meshes.size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "NameHints: LOD suffixes make LOD groups; Godot suffixes make colliders, leave nodes out and loop clips; auto picks the set" )
    try
    {
        CAssetScene scene;
        const UInt32 root = scene.AddNode( "Root" );
        const UInt32 tree0 = scene.AddNode( "Tree_LOD0", (Int32) root );
        const UInt32 tree1 = scene.AddNode( "Tree_LOD1", (Int32) root );
        const UInt32 tree = scene.AddNode( "Tree", (Int32) root );
        const UInt32 bush1 = scene.AddNode( "Bush_LOD1" );
        const UInt32 bush0 = scene.AddNode( "Bush_LOD0" );
        const UInt32 grass = scene.AddNode( "Grass", (Int32) root );
        const UInt32 grass0 = scene.AddNode( "Grass_lod0", (Int32) grass );
        const UInt32 grass1 = scene.AddNode( "Grass_lod1", (Int32) grass );
        const UInt32 wall = scene.AddNode( "Wall-col", (Int32) root );
        AddBoxMesh( scene, wall, 1.0f, 1.0f, 1.0f );
        const UInt32 fence = scene.AddNode( "Fence-convcolonly", (Int32) root );
        AddBoxMesh( scene, fence, 1.0f, 1.0f, 1.0f );
        const UInt32 junk = scene.AddNode( "Junk-noimp", (Int32) root );
        scene.AddNode( "JunkChild", (Int32) junk );
        const UInt32 box = scene.AddNode( "UBX_Wall", (Int32) root );
        AddBoxMesh( scene, box, 1.0f, 1.0f, 1.0f );
        CAssetAnimation animation;
        animation.name = "Run-loop";
        scene.animations.push_back( animation );
        animation.name = "Walk-cycle";
        scene.animations.push_back( animation );
        animation.name = "Idle";
        scene.animations.push_back( animation );
        ( void ) tree0; ( void ) tree1; ( void ) bush1; ( void ) bush0; ( void ) grass0; ( void ) grass1;

        CNameHintsPostProcessor processor;
        CConversionReport report;
        report.SelectAsset( "godot.stub" );
        scene.settings.SetAttribute( "nameHints", "godot" );
        CORE::CDataNode settings( "settings", GUCEF_DATATYPE_OBJECT );
        ASSERT_TRUE( processor.Process( scene, settings, report ) );

        // LOD groups, held by the sibling of the name, by the parent that holds only them, or by a new node
        ASSERT_TRUE( 3 == scene.lodGroups.size() );
        const CAssetNode& treeNode = scene.nodes[ scene.FindNode( "Tree" ) ];
        ASSERT_TRUE( treeNode.lodGroup >= 0 );
        if ( treeNode.lodGroup >= 0 )
        {
            ASSERT_TRUE( 2 == scene.lodGroups[ treeNode.lodGroup ].levels.size() );
            ASSERT_TRUE( scene.FindNode( "Tree_LOD0" ) == (Int32) scene.lodGroups[ treeNode.lodGroup ].levels[ 0 ].node );
            ASSERT_TRUE( scene.lodGroups[ treeNode.lodGroup ].levels[ 0 ].screenRelativeHeight < 0.0f );
        }
        const CAssetNode& grassNode = scene.nodes[ scene.FindNode( "Grass" ) ];
        ASSERT_TRUE( grassNode.lodGroup >= 0 && 2 == scene.lodGroups[ grassNode.lodGroup < 0 ? 0 : grassNode.lodGroup ].levels.size() );
        const Int32 bushHolder = scene.FindNode( "Bush" );
        ASSERT_TRUE( bushHolder >= 0 && scene.nodes[ bushHolder ].lodGroup >= 0 );
        if ( bushHolder >= 0 && scene.nodes[ bushHolder ].lodGroup >= 0 )
        {
            const CAssetLodGroup& bushGroup = scene.lodGroups[ scene.nodes[ bushHolder ].lodGroup ];
            ASSERT_TRUE( scene.FindNode( "Bush_LOD0" ) == (Int32) bushGroup.levels[ 0 ].node && scene.FindNode( "Bush_LOD1" ) == (Int32) bushGroup.levels[ 1 ].node );
            ASSERT_TRUE( bushHolder == scene.nodes[ bushGroup.levels[ 0 ].node ].parent );
        }
        ( void ) tree;

        // Godot: colliders, left out nodes, loops; the Unreal prefixes are not of this set
        const Int32 wallNode = scene.FindNode( "Wall" );
        ASSERT_TRUE( wallNode >= 0 && 1 == scene.nodes[ wallNode ].colliders.size() && COLLIDERSHAPE_TRIMESH == scene.nodes[ wallNode ].colliders[ 0 ].shape && scene.nodes[ wallNode ].mesh >= 0 );
        const Int32 fenceNode = scene.FindNode( "Fence" );
        ASSERT_TRUE( fenceNode >= 0 && 1 == scene.nodes[ fenceNode ].colliders.size() && COLLIDERSHAPE_CONVEX == scene.nodes[ fenceNode ].colliders[ 0 ].shape );
        ASSERT_TRUE( fenceNode >= 0 && scene.nodes[ fenceNode ].mesh < 0 && scene.nodes[ fenceNode ].colliders[ 0 ].meshIndex >= 0 );
        ASSERT_TRUE( scene.FindNode( "Junk-noimp" ) < 0 && scene.FindNode( "JunkChild" ) < 0 && 1 == CountItems( report, REPORTITEM_SKIPPED, "NameHints" ) );
        ASSERT_TRUE( scene.FindNode( "UBX_Wall" ) >= 0 );
        ASSERT_TRUE( "Run" == scene.animations[ 0 ].name && scene.animations[ 0 ].loop && "Walk" == scene.animations[ 1 ].name && scene.animations[ 1 ].loop && !scene.animations[ 2 ].loop );

        // what "auto" stands for
        CAssetScene probe;
        ASSERT_TRUE( "both" == CNameHintsPostProcessor::ResolveSet( "auto", probe ) );
        probe.provenance.sourceEngine = "Unreal";
        ASSERT_TRUE( "unreal" == CNameHintsPostProcessor::ResolveSet( "", probe ) );
        probe.provenance.sourceEngine = "unity";
        ASSERT_TRUE( "none" == CNameHintsPostProcessor::ResolveSet( "auto", probe ) );
        probe.provenance.sourceEngine.Clear();
        probe.provenance.importerName = "godot";
        ASSERT_TRUE( "godot" == CNameHintsPostProcessor::ResolveSet( "AUTO", probe ) );
        ASSERT_TRUE( "none" == CNameHintsPostProcessor::ResolveSet( "none", probe ) );

        // the set "none" changes nothing
        CAssetScene untouched;
        untouched.AddNode( "UBX_A" );
        untouched.AddNode( "A" );
        AddBoxMesh( untouched, 0, 1.0f, 1.0f, 1.0f );
        settings.SetAttribute( "set", "none" );
        ASSERT_TRUE( processor.Process( untouched, settings, report ) && 2 == untouched.nodes.size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GenerateTangents: normal mapped submeshes without tangents get them from the UVs, with the handedness in w" )
    try
    {
        CAssetScene scene;
        CAssetMaterial normalMapped;
        normalMapped.normalTexture = CAssetTextureReference( 0 );
        scene.materials.push_back( normalMapped );
        scene.materials.push_back( CAssetMaterial() );
        CAssetMesh mesh;
        CAssetSubMesh quad;
        const Float32 positions[] = { 0,0,0,  1,0,0,  1,1,0,  0,1,0 };
        const Float32 uvs[] = { 0,0,  1,0,  1,1,  0,1 };
        for ( UInt32 i=0; i<12; ++i )
            quad.positions.push_back( positions[ i ] );
        for ( UInt32 i=0; i<8; ++i )
            quad.uvSets[ 0 ].push_back( uvs[ i ] );
        for ( UInt32 v=0; v<4; ++v )
        {
            quad.normals.push_back( 0.0f ); quad.normals.push_back( 0.0f ); quad.normals.push_back( 1.0f );
        }
        const UInt32 indices[] = { 0,1,2, 0,2,3 };
        for ( UInt32 i=0; i<6; ++i )
            quad.indices.push_back( indices[ i ] );
        quad.materialIndex = 0;
        mesh.subMeshes.push_back( quad );
        CAssetSubMesh mirrored = quad;
        for ( UInt32 v=0; v<4; ++v )
            mirrored.uvSets[ 0 ][ v*2 ] = 1.0f - mirrored.uvSets[ 0 ][ v*2 ];
        mesh.subMeshes.push_back( mirrored );
        CAssetSubMesh plain = quad;
        plain.materialIndex = 1;
        mesh.subMeshes.push_back( plain );
        CAssetSubMesh noNormals = quad;
        noNormals.normals.clear();
        mesh.subMeshes.push_back( noNormals );
        scene.meshes.push_back( mesh );

        CGenerateTangentsPostProcessor processor;
        CConversionReport report;
        report.SelectAsset( "quad.stub" );
        ASSERT_TRUE( processor.Process( scene, CORE::CDataNode(), report ) );
        const CAssetSubMesh& first = scene.meshes[ 0 ].subMeshes[ 0 ];
        ASSERT_TRUE( first.HasTangents() );
        for ( UInt32 v=0; v<4; ++v )
            ASSERT_TRUE( Near( 1.0f, first.tangents[ v*4 ] ) && Near( 0.0f, first.tangents[ v*4+1 ] ) && Near( 0.0f, first.tangents[ v*4+2 ] ) && Near( 1.0f, first.tangents[ v*4+3 ] ) );
        const CAssetSubMesh& second = scene.meshes[ 0 ].subMeshes[ 1 ];
        ASSERT_TRUE( second.HasTangents() && Near( -1.0f, second.tangents[ 0 ] ) && Near( -1.0f, second.tangents[ 3 ] ) );
        ASSERT_FALSE( scene.meshes[ 0 ].subMeshes[ 2 ].HasTangents() );
        ASSERT_FALSE( scene.meshes[ 0 ].subMeshes[ 3 ].HasTangents() );
        ASSERT_TRUE( 1 == CountItems( report, REPORTITEM_APPROXIMATION, "GenerateTangents" ) && 1 == CountItems( report, REPORTITEM_WARNING, "GenerateTangents" ) );

        // tangents that are there stay
        scene.meshes[ 0 ].subMeshes[ 0 ].tangents[ 0 ] = 0.5f;
        CConversionReport again;
        ASSERT_TRUE( processor.Process( scene, CORE::CDataNode(), again ) );
        ASSERT_TRUE( 0.5f == scene.meshes[ 0 ].subMeshes[ 0 ].tangents[ 0 ] );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PruneEmptyNodes and MergeIdenticalMaterials: empty leaves go, materials that look the same are merged" )
    try
    {
        CAssetScene scene;
        const UInt32 root = scene.AddNode( "Root" );
        const UInt32 empty = scene.AddNode( "Empty", (Int32) root );
        scene.AddNode( "EmptyChild", (Int32) empty );
        const UInt32 joint = scene.AddNode( "Joint", (Int32) root );
        const UInt32 animated = scene.AddNode( "Animated", (Int32) root );
        const UInt32 holder = scene.AddNode( "Holder", (Int32) root );
        const UInt32 tagged = scene.AddNode( "Tagged", (Int32) root );
        scene.nodes[ tagged ].extras.SetAttribute( "tag", "keep" );
        AddBoxMesh( scene, holder, 1.0f, 1.0f, 1.0f );
        CAssetSkin skin;
        skin.joints.push_back( joint );
        skin.inverseBindMatrices.push_back( MATH::TFloat32Matrix4D::Identity() );
        scene.skins.push_back( skin );
        CAssetAnimation animation;
        CAssetAnimationChannel channel;
        channel.targetNode = animated;
        animation.channels.push_back( channel );
        scene.animations.push_back( animation );

        CPruneEmptyNodesPostProcessor prune;
        CConversionReport report;
        report.SelectAsset( "prune.stub" );
        ASSERT_TRUE( prune.Process( scene, CORE::CDataNode(), report ) );
        ASSERT_TRUE( 5 == scene.nodes.size() && scene.FindNode( "Empty" ) < 0 && scene.FindNode( "EmptyChild" ) < 0 );
        ASSERT_TRUE( scene.FindNode( "Joint" ) >= 0 && scene.FindNode( "Animated" ) >= 0 && scene.FindNode( "Holder" ) >= 0 && scene.FindNode( "Tagged" ) >= 0 );
        ASSERT_TRUE( scene.IsJoint( (UInt32) scene.FindNode( "Joint" ) ) && scene.IsAnimationTarget( (UInt32) scene.FindNode( "Animated" ) ) );
        ASSERT_TRUE( 1 == CountItems( report, REPORTITEM_INFO, "PruneEmptyNodes" ) );

        CAssetScene materials;
        const char* const names[] = { "Red", "Blue", "RedToo", "Unused" };
        const Float32 colours[][ 3 ] = { { 1, 0, 0 }, { 0, 0, 1 }, { 1, 0, 0 }, { 0, 1, 0 } };
        for ( UInt32 i=0; i<4; ++i )
        {
            CAssetMaterial material;
            material.name = names[ i ];
            for ( UInt32 c=0; c<3; ++c )
                material.baseColorFactor[ c ] = colours[ i ][ c ];
            material.source.shaderName = names[ i ];
            materials.materials.push_back( material );
        }
        CAssetMesh mesh;
        for ( Int32 i=0; i<3; ++i )
        {
            CAssetSubMesh subMesh;
            subMesh.materialIndex = i;
            mesh.subMeshes.push_back( subMesh );
        }
        materials.meshes.push_back( mesh );
        ASSERT_TRUE( materials.materials[ 0 ].HasSameAppearance( materials.materials[ 2 ] ) && !materials.materials[ 0 ].HasSameAppearance( materials.materials[ 1 ] ) );

        CMergeIdenticalMaterialsPostProcessor merge;
        ASSERT_TRUE( merge.Process( materials, CORE::CDataNode(), report ) );
        ASSERT_TRUE( 3 == materials.materials.size() && "Red" == materials.materials[ 0 ].name && "Blue" == materials.materials[ 1 ].name && "Unused" == materials.materials[ 2 ].name );
        ASSERT_TRUE( 0 == materials.meshes[ 0 ].subMeshes[ 0 ].materialIndex && 1 == materials.meshes[ 0 ].subMeshes[ 1 ].materialIndex && 0 == materials.meshes[ 0 ].subMeshes[ 2 ].materialIndex );
        ASSERT_TRUE( 1 == CountItems( report, REPORTITEM_INFO, "MergeIdenticalMaterials" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Validators: texture size, missing textures, bone limit, triangle budget and naming, with their severities" )
    try
    {
        CAssetPluginRegistry& plugins = CAssetConverterGlobal::Instance()->GetPluginRegistry();
        CAssetScene scene;
        scene.provenance.assetPath = "props/rock.stub";
        CAssetTexture big;
        big.name = "big";
        big.width = 4096; big.height = 4096;
        scene.textures.push_back( big );
        CAssetTexture odd;
        odd.name = "odd";
        odd.width = 300; odd.height = 200;
        scene.textures.push_back( odd );
        CAssetTexture lost;
        lost.name = "lost";
        lost.missing = true;
        lost.sources.push_back( "work/none.png" );
        scene.textures.push_back( lost );
        CAssetMaterial material;
        material.name = "M_Rock";
        material.emissiveTexture = CAssetTextureReference( 9 );
        scene.materials.push_back( material );

        TAssetValidatorPtr validator;
        CORE::CDataNode settings( "settings", GUCEF_DATATYPE_OBJECT );
        CConversionReport report;
        report.SelectAsset( scene.provenance.assetPath );

        ASSERT_TRUE( plugins.TryGetValidator( "TextureSize", validator ) );
        settings.SetAttribute( "maxSize", (UInt32) 2048 );
        settings.SetAttribute( "powerOfTwo", true );
        ASSERT_TRUE( validator->Validate( scene, settings, report ) );
        ASSERT_TRUE( 2 == CountItems( report, REPORTITEM_ERROR, "TextureSize" ) );
        settings.SetAttribute( "severity", "warning" );
        ASSERT_TRUE( validator->Validate( scene, settings, report ) && 2 == CountItems( report, REPORTITEM_WARNING, "TextureSize" ) );

        ASSERT_TRUE( plugins.TryGetValidator( "MissingTextures", validator ) );
        ASSERT_TRUE( validator->Validate( scene, CORE::CDataNode(), report ) && 2 == CountItems( report, REPORTITEM_ERROR, "MissingTextures" ) );

        CAssetMesh mesh;
        mesh.name = "Rock";
        CAssetSubMesh skinned;
        skinned.influencesPerVertex = 4;
        for ( UInt32 j=0; j<4; ++j )
        {
            skinned.joints.push_back( j );
            skinned.weights.push_back( 0.25f );
        }
        for ( UInt32 i=0; i<12; ++i )
            skinned.indices.push_back( i % 3 );
        mesh.subMeshes.push_back( skinned );
        scene.meshes.push_back( mesh );
        ASSERT_TRUE( 4 == skinned.GetUsedJointCount() );
        ASSERT_TRUE( plugins.TryGetValidator( "BoneLimit", validator ) );
        CORE::CDataNode bones( "settings", GUCEF_DATATYPE_OBJECT );
        ASSERT_TRUE( validator->Validate( scene, bones, report ) && 0 == CountItems( report, REPORTITEM_ERROR, "BoneLimit" ) );
        bones.SetAttribute( "maxBones", (UInt32) 3 );
        ASSERT_TRUE( validator->Validate( scene, bones, report ) && 1 == CountItems( report, REPORTITEM_ERROR, "BoneLimit" ) );

        // triangles per LOD level: the level nodes with what is below them, the rest counts for level 0
        const UInt32 level0 = scene.AddNode( "Level0" );
        const UInt32 level1 = scene.AddNode( "Level1" );
        const UInt32 other = scene.AddNode( "Other" );
        scene.nodes[ level0 ].mesh = 0;
        scene.nodes[ level1 ].mesh = 0;
        scene.nodes[ other ].mesh = 0;
        CAssetLodGroup group;
        group.AddLevel( level0, 0.5f );
        group.AddLevel( level1, 0.2f );
        scene.lodGroups.push_back( group );
        ASSERT_TRUE( plugins.TryGetValidator( "TriangleBudget", validator ) );
        CORE::CDataNode budget( "settings", GUCEF_DATATYPE_OBJECT );
        CORE::CDataNode* budgets = budget.AddChild( "budgets", GUCEF_DATATYPE_ARRAY );
        CORE::CDataNode* elsewhere = budgets->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        elsewhere->SetAttribute( "match", "characters/**" );
        CORE::CDataNode::TVariantVector limits;
        limits.push_back( CORE::CVariant( (UInt32) 1 ) );
        elsewhere->SetValuesOfChildByName( "levels", limits );
        CORE::CDataNode* props = budgets->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        props->SetAttribute( "match", "props/**" );
        limits.clear();
        limits.push_back( CORE::CVariant( (UInt32) 7 ) );
        limits.push_back( CORE::CVariant( (UInt32) 7 ) );
        props->SetValuesOfChildByName( "levels", limits );
        ASSERT_TRUE( validator->Validate( scene, budget, report ) );
        ASSERT_TRUE( 1 == CountItems( report, REPORTITEM_WARNING, "TriangleBudget" ) );

        ASSERT_TRUE( plugins.TryGetValidator( "Naming", validator ) );
        CORE::CDataNode naming( "settings", GUCEF_DATATYPE_OBJECT );
        CORE::CDataNode* patterns = naming.AddChild( "patterns", GUCEF_DATATYPE_OBJECT );
        patterns->SetAttribute( "mesh", "SM_*" );
        patterns->SetAttribute( "material", "M_*" );
        patterns->SetAttribute( "asset", "*.stub" );
        ASSERT_TRUE( validator->Validate( scene, naming, report ) && 1 == CountItems( report, REPORTITEM_WARNING, "Naming" ) );
        CORE::CString::StringVector globs;
        globs.push_back( "SM_*" );
        globs.push_back( "Rock" );
        patterns->DelAttribute( "mesh" );
        patterns->SetValuesOfChildByName( "mesh", globs );
        ASSERT_TRUE( validator->Validate( scene, naming, report ) && 1 == CountItems( report, REPORTITEM_WARNING, "Naming" ) );
        ASSERT_TRUE( REPORTITEM_ERROR == CIAssetValidator::GetSeverity( CORE::CDataNode(), REPORTITEM_ERROR ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Texture operations: every step of a chain, on the operations of gucefIMAGE" )
    try
    {
        CTextureBaker::TPixelMapVector sources;
        sources.push_back( MakePicture( 2, 2, 200, 100, 50, 255 ) );
        sources.push_back( MakePicture( 4, 4, 10, 20, 30, 40 ) );
        CAssetTexture texture;
        IMAGE::TPixelMapPtr result;
        CORE::CString problem;

        texture.operations.push_back( CTextureOperation::ExtractChannel( 1 ) );
        ASSERT_TRUE( CTextureBaker::ApplyOperations( texture, sources, result, problem ) );
        ASSERT_TRUE( IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE == result->GetPixelStorageFormat() && 100 == Texels( *result )[ 0 ] );

        texture.operations.clear();
        texture.operations.push_back( CTextureOperation::Swizzle( "bgra" ) );
        ASSERT_TRUE( CTextureBaker::ApplyOperations( texture, sources, result, problem ) && 50 == Texels( *result )[ 0 ] && 200 == Texels( *result )[ 2 ] );

        texture.operations.clear();
        texture.operations.push_back( CTextureOperation::Invert( 0x8 ) );
        texture.operations.push_back( CTextureOperation::ScaleBias( 0x1, 0.5f, 0.0f ) );
        ASSERT_TRUE( CTextureBaker::ApplyOperations( texture, sources, result, problem ) );
        ASSERT_TRUE( 0 == Texels( *result )[ 3 ] && 100 == Texels( *result )[ 0 ] && 100 == Texels( *result )[ 1 ] );

        // the green flip of a DirectX normal map
        texture.operations.clear();
        texture.AddGreenFlip();
        ASSERT_TRUE( CTextureBaker::ApplyOperations( texture, sources, result, problem ) && 155 == Texels( *result )[ 1 ] && 200 == Texels( *result )[ 0 ] );

        // a repack of channels of two sources of other sizes: the second is resampled to the size of the first
        const Int32 packSources[ 4 ] = { 0, 1, -1, 1 };
        const UInt32 packChannels[ 4 ] = { 0, 1, 0, 3 };
        texture.operations.clear();
        texture.operations.push_back( CTextureOperation::Pack( packSources, packChannels ) );
        ASSERT_TRUE( CTextureBaker::ApplyOperations( texture, sources, result, problem ) );
        ASSERT_TRUE( 2 == result->GetWidthInPixels() && 200 == Texels( *result )[ 0 ] && 20 == Texels( *result )[ 1 ] && 0 == Texels( *result )[ 2 ] && 40 == Texels( *result )[ 3 ] );

        // metallic in R and smoothness in A of a source become glTF metal-roughness: B = R, G = 1 - A
        CTextureBaker::TPixelMapVector metallic;
        metallic.push_back( MakePicture( 2, 2, 255, 0, 0, 204 ) );
        const Int32 mrSources[ 4 ] = { -1, 0, 0, -1 };
        const UInt32 mrChannels[ 4 ] = { 0, 3, 0, 0 };
        texture.operations.clear();
        texture.operations.push_back( CTextureOperation::Pack( mrSources, mrChannels ) );
        texture.operations.push_back( CTextureOperation::Invert( 0x2 ) );
        ASSERT_TRUE( CTextureBaker::ApplyOperations( texture, metallic, result, problem ) );
        ASSERT_TRUE( 0 == Texels( *result )[ 0 ] && 51 == Texels( *result )[ 1 ] && 255 == Texels( *result )[ 2 ] );

        // alpha premultiplication on and off
        CTextureBaker::TPixelMapVector translucent;
        translucent.push_back( MakePicture( 1, 1, 200, 100, 50, 128 ) );
        texture.operations.clear();
        texture.operations.push_back( CTextureOperation::PremultiplyAlpha( IMAGE::COLOURSPACE_LINEAR ) );
        ASSERT_TRUE( CTextureBaker::ApplyOperations( texture, translucent, result, problem ) );
        ASSERT_TRUE( abs( 100 - (Int32) Texels( *result )[ 0 ] ) <= 1 && 128 == Texels( *result )[ 3 ] );
        texture.operations.push_back( CTextureOperation::UnpremultiplyAlpha( IMAGE::COLOURSPACE_LINEAR ) );
        ASSERT_TRUE( CTextureBaker::ApplyOperations( texture, translucent, result, problem ) && abs( 200 - (Int32) Texels( *result )[ 0 ] ) <= 2 );

        // a colour space tag names the space and changes no texel
        texture.operations.clear();
        texture.usage = TEXTUREUSAGE_COLOUR;
        ASSERT_TRUE( IMAGE::COLOURSPACE_SRGB == texture.GetColourSpace() );
        texture.operations.push_back( CTextureOperation::ColourSpaceTag( IMAGE::COLOURSPACE_LINEAR ) );
        ASSERT_TRUE( IMAGE::COLOURSPACE_LINEAR == texture.GetColourSpace() && !texture.HasTexelOperations() && texture.GetChainKeyText().IsNULLOrEmpty() );
        ASSERT_TRUE( CTextureBaker::ApplyOperations( texture, sources, result, problem ) && 200 == Texels( *result )[ 0 ] && 100 == Texels( *result )[ 1 ] );
        texture.usage = TEXTUREUSAGE_NORMAL;
        texture.operations.clear();
        ASSERT_TRUE( IMAGE::COLOURSPACE_LINEAR == texture.GetColourSpace() );

        // the operations as data and as key text
        CTextureOperation op = CTextureOperation::ScaleBias( 0x3, 0.25f, 0.5f );
        CORE::CDataNode node( "op", GUCEF_DATATYPE_OBJECT );
        op.Serialize( node );
        CTextureOperation read;
        ASSERT_TRUE( read.Deserialize( node ) && read == op && "scaleBias(3,0.25,0.5)" == op.ToKeyText() );
        CTextureOperation pack = CTextureOperation::Pack( packSources, packChannels );
        node = CORE::CDataNode( "op", GUCEF_DATATYPE_OBJECT );
        pack.Serialize( node );
        ASSERT_TRUE( read.Deserialize( node ) && read == pack && -1 == read.packSources[ 2 ] && 3 == read.packChannels[ 3 ] );

        // a step that cannot be done fails the chain, with the step named
        texture.operations.clear();
        texture.operations.push_back( CTextureOperation::ExtractChannel( 7 ) );
        ASSERT_FALSE( CTextureBaker::ApplyOperations( texture, sources, result, problem ) );
        ASSERT_TRUE( problem.HasSubstr( "extractChannel(7)", 0 ) >= 0 );
        texture.operations.clear();
        const Int32 badSources[ 4 ] = { 5, -1, -1, -1 };
        texture.operations.push_back( CTextureOperation::Pack( badSources, packChannels ) );
        ASSERT_FALSE( CTextureBaker::ApplyOperations( texture, sources, result, problem ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Texture baker: the result of a chain is written once, PNG sources without operations are copied, missing sources reported" )
    try
    {
        MakeWorkFolder( "baker" );
        ASSERT_TRUE( WriteTestPng( "work/baker/src/a.png", 4, 4, 200, 100, 50, 255 ) );
        IMAGE::TPixelMapPtr written;
        ASSERT_TRUE( CTextureBaker::LoadPicture( "work/baker/src/a.png", written ) );
        ASSERT_TRUE( !written.IsNULL() && IMAGE::PSF_RGBA == written->GetPixelStorageFormat() && 200 == Texels( *written )[ 0 ] && 50 == Texels( *written )[ 2 ] );

        CConversionReport report;
        report.SelectAsset( "baker.stub" );
        CTextureBaker baker( "work/baker/textures" );
        CAssetTexture flipped;
        flipped.name = "flipped";
        flipped.sources.push_back( "work/baker/src/a.png" );
        flipped.operations.push_back( CTextureOperation::Invert( 0x2 ) );
        CAssetTexture same = flipped;
        CAssetTexture tagged = flipped;
        tagged.operations.push_back( CTextureOperation::ColourSpaceTag( IMAGE::COLOURSPACE_LINEAR ) );
        CAssetTexture copied;
        copied.name = "copied";
        copied.sources.push_back( "work/baker/src/a.png" );
        CAssetTexture lost;
        lost.name = "lost";
        lost.sources.push_back( "work/baker/src/none.png" );

        ASSERT_TRUE( baker.Bake( flipped, report ) && baker.Bake( same, report ) && baker.Bake( tagged, report ) && baker.Bake( copied, report ) );
        ASSERT_TRUE( flipped.bakedPath == same.bakedPath && flipped.bakedPath == tagged.bakedPath && flipped.bakedPath != copied.bakedPath );
        ASSERT_TRUE( "work/baker/textures" == CVfsPaths::FolderOf( flipped.bakedPath ) && CVfsPaths::NameOf( flipped.bakedPath ).HasSubstr( "a.", 0 ) == 0 );
        ASSERT_TRUE( 4 == flipped.width && 4 == flipped.height && !flipped.missing );
        ASSERT_TRUE( 2 == baker.GetWrittenCount() && 2 == baker.GetReusedCount() );
        IMAGE::TPixelMapPtr baked;
        ASSERT_TRUE( CTextureBaker::LoadPicture( flipped.bakedPath, baked ) && 155 == Texels( *baked )[ 1 ] && 200 == Texels( *baked )[ 0 ] );
        ASSERT_TRUE( CContentHash::HashFile( copied.bakedPath ) == CContentHash::HashFile( "work/baker/src/a.png" ) );

        ASSERT_FALSE( baker.Bake( lost, report ) );
        ASSERT_TRUE( lost.missing && 1 == CountItems( report, REPORTITEM_ERROR, "TextureBaker" ) );
        CAssetTexture sourceless;
        ASSERT_FALSE( baker.Bake( sourceless, report ) );

        // a later run finds the file of the chain and writes nothing
        CTextureBaker later( "work/baker/textures" );
        CAssetTexture again;
        again.sources.push_back( "work/baker/src/a.png" );
        again.operations.push_back( CTextureOperation::Invert( 0x2 ) );
        ASSERT_TRUE( later.Bake( again, report ) && again.bakedPath == flipped.bakedPath && 0 == later.GetWrittenCount() && 1 == later.GetReusedCount() );

        // the files are listed for the asset
        const CConversionReportEntry* entry = report.FindEntry( "baker.stub" );
        ASSERT_TRUE( GUCEF_NULL != entry && 2 == entry->files.size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
