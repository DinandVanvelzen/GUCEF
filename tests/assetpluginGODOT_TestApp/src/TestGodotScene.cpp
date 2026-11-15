/*
 *  assetpluginGODOT_TestApp: Test application for the Godot plugin of the asset converter (assetpluginGODOT)
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

#include "TestGodotProject.h"
#include "TestGodotScene.h"

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
#define REQUIRE(t)      do { if ( !( t ) ) { GUCEF_TESTFW_ASSERT_TRUE( t ); throw 1; } } while ( 0 )

namespace {

MATH::TFloat32Point3D
WorldPoint( const CAssetScene& scene, UInt32 node, Float32 x, Float32 y, Float32 z )
{
    const MATH::TFloat32Matrix4D matrix = scene.GetWorldMatrix( node );
    return matrix * MATH::TFloat32Point3D( x, y, z );
}

/** Whether every triangle of the submesh faces the way the normals of its vertices do */
bool
FacesOutward( const CAssetSubMesh& subMesh )
{
    if ( subMesh.indices.empty() || !subMesh.HasNormals() )
        return false;
    for ( size_t i=0; i+2<subMesh.indices.size(); i+=3 )
    {
        const Float32* a = &subMesh.positions[ subMesh.indices[ i ] * 3 ];
        const Float32* b = &subMesh.positions[ subMesh.indices[ i + 1 ] * 3 ];
        const Float32* c = &subMesh.positions[ subMesh.indices[ i + 2 ] * 3 ];
        const Float32 e1[ 3 ] = { b[ 0 ] - a[ 0 ], b[ 1 ] - a[ 1 ], b[ 2 ] - a[ 2 ] };
        const Float32 e2[ 3 ] = { c[ 0 ] - a[ 0 ], c[ 1 ] - a[ 1 ], c[ 2 ] - a[ 2 ] };
        const Float32 n[ 3 ] = { e1[ 1 ] * e2[ 2 ] - e1[ 2 ] * e2[ 1 ], e1[ 2 ] * e2[ 0 ] - e1[ 0 ] * e2[ 2 ], e1[ 0 ] * e2[ 1 ] - e1[ 1 ] * e2[ 0 ] };
        const Float32* vn = &subMesh.normals[ subMesh.indices[ i ] * 3 ];
        const Float32 dot = n[ 0 ] * vn[ 0 ] + n[ 1 ] * vn[ 1 ] + n[ 2 ] * vn[ 2 ];
        const Float32 area = n[ 0 ] * n[ 0 ] + n[ 1 ] * n[ 1 ] + n[ 2 ] * n[ 2 ];
        if ( area > 1e-12f && dot <= 0.0f )
            return false;
    }
    return true;
}

void
Bounds( const CAssetSubMesh& subMesh, Float32 minimum[ 3 ], Float32 maximum[ 3 ] )
{
    for ( UInt32 a=0; a<3; ++a )
    {
        minimum[ a ] = 1e30f;
        maximum[ a ] = -1e30f;
    }
    for ( UInt32 v=0; v<subMesh.GetVertexCount(); ++v )
        for ( UInt32 a=0; a<3; ++a )
        {
            if ( subMesh.positions[ v * 3 + a ] < minimum[ a ] ) minimum[ a ] = subMesh.positions[ v * 3 + a ];
            if ( subMesh.positions[ v * 3 + a ] > maximum[ a ] ) maximum[ a ] = subMesh.positions[ v * 3 + a ];
        }
}

/** Writes a scene file below the work folder: the header, then the body */
bool
WriteScene( const CORE::CString& path, const CORE::CString& body )
{
    return WriteWorkText( path, "[gd_scene load_steps=2 format=3]\n\n" + body );
}

const char* const Mat = "[sub_resource type=\"StandardMaterial3D\" id=\"red\"]\nalbedo_color = Color(1, 0, 0, 1)\n\n";

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGodotSceneTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetpluginGODOT.Scene" );

    GUCEF_TESTFW_TESTCASE( "Nodes: a hierarchy with Transform3D, position/rotation/scale and the Euler order of Godot, the root and the provenance" )
    try
    {
        MakeWorkFolder( "nodes" );
        ASSERT_TRUE( WriteScene( "nodes/h.tscn",
            "[node name=\"Root\" type=\"Node3D\"]\ntransform = " + GodotTransformText( 0.0, 1.0, 1.0, 2.0, 3.0 ) + "\n\n"
            "[node name=\"Arm\" type=\"Node3D\" parent=\".\"]\ntransform = " + GodotTransformText( 90.0, 2.0, 0.0, 0.0, 0.0 ) + "\n\n"
            "[node name=\"Hand\" type=\"Node3D\" parent=\"Arm\"]\nposition = Vector3(0, 0, 1)\n\n"
            "[node name=\"Tilted\" type=\"Node3D\" parent=\".\"]\nrotation = Vector3(0, 1.5707963, 0)\nscale = Vector3(1, 2, 3)\n\n"
            "[node name=\"Rolled\" type=\"Node3D\" parent=\".\"]\nrotation = Vector3(1.5707963, 1.5707963, 0)\n\n"
            "[node name=\"Quat\" type=\"Node3D\" parent=\".\"]\nquaternion = Quaternion(0, 0.70710678, 0, 0.70710678)\n" ) );
        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportGodot( "work/nodes", "h.tscn", scene, report ) );
        ASSERT_TRUE( 0 == report.GetItemCount( REPORTITEM_ERROR ) );
        REQUIRE( 6 == scene.nodes.size() && 1 == scene.rootNodes.size() );
        ASSERT_TRUE( scene.coordinateSystem == MATH::CCoordinateSystem::GucefDefault() && "godot" == scene.provenance.sourceEngine && "format 3" == scene.provenance.sourceFormatVersion );
        const Int32 root = FindNodeByName( scene, "Root" );
        const Int32 arm = FindNodeByName( scene, "Arm" );
        const Int32 hand = FindNodeByName( scene, "Hand" );
        const Int32 tilted = FindNodeByName( scene, "Tilted" );
        REQUIRE( 0 == root && arm >= 0 && hand >= 0 && tilted >= 0 );
        ASSERT_TRUE( root == scene.nodes[ arm ].parent && arm == scene.nodes[ hand ].parent );
        ASSERT_TRUE( Near( scene.nodes[ arm ].transform.scale.x, 2.0f, 1e-4f ) && Near( scene.nodes[ arm ].transform.rotation.y, 0.70710678f, 1e-4f ) );
        // the hand is a unit in front of the arm, which is turned 90 degrees about Y and doubled: (2, 0, 0) from the root at (1, 2, 3)
        const MATH::TFloat32Point3D handPosition = WorldPoint( scene, (UInt32) hand, 0.0f, 0.0f, 0.0f );
        ASSERT_TRUE( Near( handPosition.x, 3.0f, 1e-4f ) && Near( handPosition.y, 2.0f, 1e-4f ) && Near( handPosition.z, 3.0f, 1e-4f ) );
        ASSERT_TRUE( Near( scene.nodes[ tilted ].transform.rotation.y, 0.70710678f, 1e-4f ) && Near( scene.nodes[ tilted ].transform.scale.z, 3.0f, 1e-4f ) );
        // the Euler angles of Godot apply Y last: a quarter turn about X takes +Z to -Y, which a turn about Y leaves where it is (the root is at (1, 2, 3))
        const MATH::TFloat32Point3D rolled = WorldPoint( scene, (UInt32) FindNodeByName( scene, "Rolled" ), 0.0f, 0.0f, 1.0f );
        ASSERT_TRUE( Near( rolled.x, 1.0f, 1e-3f ) && Near( rolled.y, 1.0f, 1e-3f ) && Near( rolled.z, 3.0f, 1e-3f ) );
        const MATH::TFloat32Point3D turned = WorldPoint( scene, (UInt32) FindNodeByName( scene, "Quat" ), 0.0f, 0.0f, 1.0f );
        ASSERT_TRUE( Near( turned.x, 2.0f, 1e-3f ) && Near( turned.y, 2.0f, 1e-3f ) && Near( turned.z, 3.0f, 1e-3f ) );
        // the scene is a prefab of its own
        REQUIRE( 1 == scene.prefabs.size() );
        ASSERT_TRUE( 0 == scene.prefabs[ 0 ].rootNode && scene.prefabs[ 0 ].instances.empty() && "Node3D" == scene.nodes[ arm ].extras.GetAttributeValue( "godotType" ).AsString() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MeshInstance3D: an ArrayMesh surface in the packed layout becomes positions, normals, UV coordinates and counter clockwise triangles with the material" )
    try
    {
        MakeWorkFolder( "mesh" );
        vector< SGodotSurface > surfaces;
        surfaces.push_back( GodotTriangleSurface( "SubResource(\"red\")" ) );
        ASSERT_TRUE( WriteScene( "mesh/m.tscn", CORE::CString( Mat ) + GodotArrayMeshText( "ArrayMesh_t", surfaces ) + "\n"
                                 "[node name=\"Tri\" type=\"MeshInstance3D\"]\nmesh = SubResource(\"ArrayMesh_t\")\n" ) );
        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportGodot( "work/mesh", "m.tscn", scene, report ) );
        REQUIRE( 1 == scene.nodes.size() && 0 == scene.nodes[ 0 ].mesh && 1 == scene.meshes.size() && 1 == scene.meshes[ 0 ].subMeshes.size() );
        const CAssetSubMesh& subMesh = scene.meshes[ 0 ].subMeshes[ 0 ];
        REQUIRE( 3 == subMesh.GetVertexCount() && 3 == subMesh.indices.size() && 6 == subMesh.uvSets[ 0 ].size() && 9 == subMesh.normals.size() );
        ASSERT_TRUE( Near( subMesh.positions[ 3 ], 1.0f ) && Near( subMesh.positions[ 7 ], 1.0f ) );
        for ( UInt32 v=0; v<3; ++v )
            ASSERT_TRUE( Near( subMesh.normals[ v * 3 ], 0.0f, 2e-3f ) && Near( subMesh.normals[ v * 3 + 1 ], 0.0f, 2e-3f ) && Near( subMesh.normals[ v * 3 + 2 ], 1.0f, 2e-3f ) );
        ASSERT_TRUE( Near( subMesh.uvSets[ 0 ][ 1 ], 1.0f ) && Near( subMesh.uvSets[ 0 ][ 2 ], 1.0f ) );
        // the clockwise order of Godot is turned over: seen from +Z the triangle is counter clockwise now
        ASSERT_TRUE( 0 == subMesh.indices[ 0 ] && 1 == subMesh.indices[ 1 ] && 2 == subMesh.indices[ 2 ] && FacesOutward( subMesh ) );
        REQUIRE( 0 == subMesh.materialIndex && 1 == scene.materials.size() );
        ASSERT_TRUE( "red" == scene.materials[ 0 ].name && Near( scene.materials[ 0 ].baseColorFactor[ 0 ], 1.0f ) && Near( scene.materials[ 0 ].baseColorFactor[ 1 ], 0.0f ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MeshInstance3D: tangents, vertex colours, a second UV set, base64 data, a triangle strip, no indices, and the colours only where the material asks for them" )
    try
    {
        MakeWorkFolder( "meshfull" );
        SGodotSurface first = GodotTriangleSurface( "SubResource(\"useColour\")" );
        for ( UInt32 v=0; v<3; ++v )
        {
            first.tangents.push_back( 1.0f ); first.tangents.push_back( 0.0f ); first.tangents.push_back( 0.0f ); first.tangents.push_back( 0 == v ? -1.0f : 1.0f );
            first.colours.push_back( 255 ); first.colours.push_back( 128 ); first.colours.push_back( 0 ); first.colours.push_back( 255 );
            first.uvs2.push_back( 0.25f * v ); first.uvs2.push_back( 0.5f );
        }
        first.useBase64 = true;
        SGodotSurface second = first;
        second.material = "SubResource(\"noColour\")";
        second.useBase64 = false;
        SGodotSurface strip;
        const float stripPositions[ 12 ] = { 0, 0, 0,  1, 0, 0,  0, 1, 0,  1, 1, 0 };
        for ( UInt32 i=0; i<12; ++i )
            strip.positions.push_back( stripPositions[ i ] );
        strip.primitive = 4;
        for ( UInt32 i=0; i<4; ++i )
            strip.indices.push_back( i );
        vector< SGodotSurface > surfaces;
        surfaces.push_back( first );
        surfaces.push_back( second );
        surfaces.push_back( strip );
        SGodotSurface unindexed;
        for ( UInt32 i=0; i<9; ++i )
            unindexed.positions.push_back( stripPositions[ i ] );
        vector< SGodotSurface > more;
        more.push_back( unindexed );
        ASSERT_TRUE( WriteScene( "meshfull/m.tscn",
            "[sub_resource type=\"StandardMaterial3D\" id=\"useColour\"]\nvertex_color_use_as_albedo = true\n\n"
            "[sub_resource type=\"StandardMaterial3D\" id=\"noColour\"]\nroughness = 0.5\n\n" + GodotArrayMeshText( "Many", surfaces ) + GodotArrayMeshText( "Plain", more ) + "\n"
            "[node name=\"Many\" type=\"MeshInstance3D\"]\nmesh = SubResource(\"Many\")\n\n"
            "[node name=\"Plain\" type=\"MeshInstance3D\" parent=\".\"]\nmesh = SubResource(\"Plain\")\n" ) );
        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportGodot( "work/meshfull", "m.tscn", scene, report ) );
        REQUIRE( 2 == scene.meshes.size() && 3 == scene.meshes[ 0 ].subMeshes.size() );
        const CAssetSubMesh& withColour = scene.meshes[ 0 ].subMeshes[ 0 ];
        ASSERT_TRUE( 12 == withColour.colours.size() && Near( withColour.colours[ 1 ], 128.0f / 255.0f ) && 4 == withColour.tangents.size() / 3 );
        ASSERT_TRUE( Near( withColour.tangents[ 0 ], 1.0f, 2e-3f ) && Near( withColour.tangents[ 3 ], -1.0f ) && Near( withColour.tangents[ 7 ], 1.0f ) );
        ASSERT_TRUE( 6 == withColour.uvSets[ 1 ].size() && Near( withColour.uvSets[ 1 ][ 2 ], 0.25f ) );
        ASSERT_TRUE( scene.meshes[ 0 ].subMeshes[ 1 ].colours.empty() );
        const CAssetSubMesh& stripped = scene.meshes[ 0 ].subMeshes[ 2 ];
        ASSERT_TRUE( 6 == stripped.indices.size() && 4 == stripped.GetVertexCount() );
        const CAssetSubMesh& plain = scene.meshes[ 1 ].subMeshes[ 0 ];
        ASSERT_TRUE( 3 == plain.indices.size() && 0 == plain.indices[ 0 ] && 2 == plain.indices[ 1 ] && 1 == plain.indices[ 2 ] );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MeshInstance3D: surfaces that cannot be read (lines, compressed attributes, a size that does not fit) are reported and keep their place" )
    try
    {
        MakeWorkFolder( "meshbad" );
        SGodotSurface good = GodotTriangleSurface( "" );
        SGodotSurface lines = good;
        lines.primitive = 1;
        SGodotSurface compressed = good;
        compressed.extraFormat = 536870912;
        SGodotSurface wrongSize = good;
        wrongSize.extraFormat = 4;
        vector< SGodotSurface > surfaces;
        surfaces.push_back( lines );
        surfaces.push_back( compressed );
        surfaces.push_back( wrongSize );
        surfaces.push_back( good );
        ASSERT_TRUE( WriteScene( "meshbad/m.tscn", GodotArrayMeshText( "Bad", surfaces ) + "\n[node name=\"Bad\" type=\"MeshInstance3D\"]\nmesh = SubResource(\"Bad\")\n" ) );
        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportGodot( "work/meshbad", "m.tscn", scene, report ) );
        REQUIRE( 1 == scene.meshes.size() && 4 == scene.meshes[ 0 ].subMeshes.size() );
        ASSERT_TRUE( 0 == scene.meshes[ 0 ].subMeshes[ 0 ].GetVertexCount() && 0 == scene.meshes[ 0 ].subMeshes[ 1 ].GetVertexCount() && 0 == scene.meshes[ 0 ].subMeshes[ 2 ].GetVertexCount() && 3 == scene.meshes[ 0 ].subMeshes[ 3 ].GetVertexCount() );
        ASSERT_TRUE( ReportHas( report, REPORTITEM_SKIPPED, "points or lines" ) && ReportHas( report, REPORTITEM_SKIPPED, "compressed attributes" ) && ReportHas( report, REPORTITEM_SKIPPED, "does not have the size" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Primitive meshes: box, plane, quad, sphere, cylinder and capsule with their sizes, outward triangles and the default material" )
    try
    {
        MakeWorkFolder( "prim" );
        ASSERT_TRUE( WriteScene( "prim/p.tscn",
            "[sub_resource type=\"BoxMesh\" id=\"b\"]\nmaterial = SubResource(\"red\")\nsize = Vector3(2, 4, 6)\n\n" + CORE::CString( Mat ) +
            "[sub_resource type=\"PlaneMesh\" id=\"p\"]\nsize = Vector2(4, 2)\nsubdivide_width = 1\n\n"
            "[sub_resource type=\"QuadMesh\" id=\"q\"]\n\n"
            "[sub_resource type=\"SphereMesh\" id=\"s\"]\nradius = 1.0\nheight = 2.0\nradial_segments = 8\nrings = 4\n\n"
            "[sub_resource type=\"CylinderMesh\" id=\"c\"]\ntop_radius = 0.5\nbottom_radius = 1.0\nheight = 2.0\nradial_segments = 8\nrings = 2\n\n"
            "[sub_resource type=\"CapsuleMesh\" id=\"k\"]\nradius = 0.5\nheight = 3.0\nradial_segments = 8\nrings = 2\n\n"
            "[node name=\"Box\" type=\"MeshInstance3D\"]\nmesh = SubResource(\"b\")\n\n"
            "[node name=\"Plane\" type=\"MeshInstance3D\" parent=\".\"]\nmesh = SubResource(\"p\")\n\n"
            "[node name=\"Quad\" type=\"MeshInstance3D\" parent=\".\"]\nmesh = SubResource(\"q\")\n\n"
            "[node name=\"Sphere\" type=\"MeshInstance3D\" parent=\".\"]\nmesh = SubResource(\"s\")\n\n"
            "[node name=\"Cylinder\" type=\"MeshInstance3D\" parent=\".\"]\nmesh = SubResource(\"c\")\n\n"
            "[node name=\"Capsule\" type=\"MeshInstance3D\" parent=\".\"]\nmesh = SubResource(\"k\")\n" ) );
        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportGodot( "work/prim", "p.tscn", scene, report ) );
        REQUIRE( 6 == scene.nodes.size() && 6 == scene.meshes.size() );
        Float32 low[ 3 ];
        Float32 high[ 3 ];

        const CAssetSubMesh& box = scene.meshes[ scene.nodes[ 0 ].mesh ].subMeshes[ 0 ];
        Bounds( box, low, high );
        ASSERT_TRUE( 24 == box.GetVertexCount() && 12 == box.GetTriangleCount() && FacesOutward( box ) && 0 == box.materialIndex );
        ASSERT_TRUE( Near( low[ 0 ], -1.0f ) && Near( high[ 1 ], 2.0f ) && Near( high[ 2 ], 3.0f ) );

        const CAssetSubMesh& plane = scene.meshes[ scene.nodes[ 1 ].mesh ].subMeshes[ 0 ];
        Bounds( plane, low, high );
        ASSERT_TRUE( 6 == plane.GetVertexCount() && 4 == plane.GetTriangleCount() && FacesOutward( plane ) && Near( plane.normals[ 1 ], 1.0f ) && Near( high[ 0 ], 2.0f ) && Near( high[ 2 ], 1.0f ) && Near( high[ 1 ], 0.0f ) );
        ASSERT_TRUE( -1 == plane.materialIndex );

        const CAssetSubMesh& quad = scene.meshes[ scene.nodes[ 2 ].mesh ].subMeshes[ 0 ];
        Bounds( quad, low, high );
        ASSERT_TRUE( 4 == quad.GetVertexCount() && 2 == quad.GetTriangleCount() && FacesOutward( quad ) && Near( quad.normals[ 2 ], 1.0f ) && Near( high[ 0 ], 0.5f ) && Near( high[ 1 ], 0.5f ) );

        const CAssetSubMesh& sphere = scene.meshes[ scene.nodes[ 3 ].mesh ].subMeshes[ 0 ];
        ASSERT_TRUE( sphere.GetTriangleCount() > 8 && FacesOutward( sphere ) );
        for ( UInt32 v=0; v<sphere.GetVertexCount(); ++v )
        {
            const Float32 length = (Float32) sqrt( sphere.positions[ v * 3 ] * sphere.positions[ v * 3 ] + sphere.positions[ v * 3 + 1 ] * sphere.positions[ v * 3 + 1 ] + sphere.positions[ v * 3 + 2 ] * sphere.positions[ v * 3 + 2 ] );
            ASSERT_TRUE( Near( length, 1.0f, 1e-4f ) );
            ASSERT_TRUE( Near( sphere.normals[ v * 3 + 1 ], sphere.positions[ v * 3 + 1 ], 1e-4f ) );
        }

        const CAssetSubMesh& cylinder = scene.meshes[ scene.nodes[ 4 ].mesh ].subMeshes[ 0 ];
        Bounds( cylinder, low, high );
        ASSERT_TRUE( FacesOutward( cylinder ) && Near( low[ 1 ], -1.0f ) && Near( high[ 1 ], 1.0f ) && Near( high[ 0 ], 1.0f, 1e-4f ) );

        const CAssetSubMesh& capsule = scene.meshes[ scene.nodes[ 5 ].mesh ].subMeshes[ 0 ];
        Bounds( capsule, low, high );
        ASSERT_TRUE( FacesOutward( capsule ) && Near( low[ 1 ], -1.5f, 1e-4f ) && Near( high[ 1 ], 1.5f, 1e-4f ) && Near( high[ 0 ], 0.5f, 1e-4f ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MeshInstance3D with an instanced glTF scene: the nodes below the instance, the material, the placement, and the model once for two instances" )
    try
    {
        MakeWorkFolder( "inst" );
        ASSERT_TRUE( WriteGltfModel( "work/inst/models", "crate" ) );
        ASSERT_TRUE( WriteScene( "inst/level.tscn",
            "[ext_resource type=\"PackedScene\" path=\"res://models/crate.gltf\" id=\"1_c\"]\n\n"
            "[node name=\"Level\" type=\"Node3D\"]\n\n"
            "[node name=\"Crate1\" parent=\".\" instance=ExtResource(\"1_c\")]\ntransform = " + GodotTransformText( 0.0, 1.0, 5.0, 0.0, 0.0 ) + "\n\n"
            "[node name=\"Crate2\" parent=\".\" instance=ExtResource(\"1_c\")]\ntransform = " + GodotTransformText( 0.0, 1.0, -5.0, 0.0, 0.0 ) + "\n" ) );
        ASSERT_TRUE( WriteWorkText( "inst/project.godot", "config_version=5\n" ) );
        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportGodot( "work/inst", "level.tscn", scene, report ) );
        ASSERT_TRUE( 0 == report.GetItemCount( REPORTITEM_ERROR ) );
        REQUIRE( 9 == scene.nodes.size() );
        ASSERT_TRUE( 1 == scene.meshes.size() && 1 == scene.materials.size() && "Painted" == scene.materials[ 0 ].name );
        ASSERT_TRUE( Near( scene.materials[ 0 ].metallicFactor, 0.2f ) && Near( scene.materials[ 0 ].roughnessFactor, 0.7f ) && Near( scene.materials[ 0 ].baseColorFactor[ 0 ], 0.8f ) );
        UInt32 withMesh = 0;
        for ( size_t n=0; n<scene.nodes.size(); ++n )
            if ( scene.nodes[ n ].mesh >= 0 )
                ++withMesh;
        ASSERT_TRUE( 4 == withMesh );
        const Int32 crate1 = FindNodeByName( scene, "Crate1" );
        REQUIRE( crate1 >= 0 && 1 == scene.nodes[ crate1 ].children.size() );
        ASSERT_TRUE( "Crate" == scene.nodes[ scene.nodes[ crate1 ].children[ 0 ] ].name && "models/crate.gltf" == scene.nodes[ crate1 ].extras.GetAttributeValue( "instanceOf" ).AsString() );
        for ( size_t n=0; n<scene.nodes.size(); ++n )
            if ( "Lid" == scene.nodes[ n ].name )
            {
                const MATH::TFloat32Point3D p = WorldPoint( scene, (UInt32) n, 0.0f, 0.0f, 0.0f );
                ASSERT_TRUE( Near( p.y, 1.0f ) && ( Near( p.x, 5.0f ) || Near( p.x, -5.0f ) ) );
            }
        ASSERT_TRUE( 2 == scene.animations.size() && "Spin" == scene.animations[ 0 ].name );
        ASSERT_TRUE( !scene.dependencies.empty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MeshInstance3D with an instanced FBX file: the units and axes of the file are normalised to metres with Y up" )
    try
    {
        MakeWorkFolder( "fbxinst" );
        ASSERT_TRUE( WriteWorkText( "fbxinst/models/tri.fbx", MakeTriangleFbx( true ) ) );
        ASSERT_TRUE( WriteScene( "fbxinst/s.tscn", "[ext_resource type=\"PackedScene\" path=\"res://models/tri.fbx\" id=\"1_f\"]\n\n[node name=\"Root\" type=\"Node3D\"]\n\n[node name=\"Tri\" parent=\".\" instance=ExtResource(\"1_f\")]\n" ) );
        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportGodot( "work/fbxinst", "s.tscn", scene, report ) );
        REQUIRE( 1 == scene.meshes.size() );
        bool foundX = false;
        bool foundBack = false;
        for ( size_t n=0; n<scene.nodes.size(); ++n )
        {
            if ( scene.nodes[ n ].mesh < 0 )
                continue;
            const CAssetSubMesh& subMesh = scene.meshes[ scene.nodes[ n ].mesh ].subMeshes[ 0 ];
            for ( UInt32 v=0; v<subMesh.GetVertexCount(); ++v )
            {
                const MATH::TFloat32Point3D p = WorldPoint( scene, (UInt32) n, subMesh.positions[ v * 3 ], subMesh.positions[ v * 3 + 1 ], subMesh.positions[ v * 3 + 2 ] );
                if ( Near( p.x, 1.0f, 1e-3f ) && Near( p.y, 0.0f, 1e-3f ) && Near( p.z, 0.0f, 1e-3f ) )
                    foundX = true;
                if ( Near( p.x, 0.0f, 1e-3f ) && Near( p.y, 0.0f, 1e-3f ) && Near( p.z, -1.0f, 1e-3f ) )
                    foundBack = true;
            }
        }
        ASSERT_TRUE( foundX && foundBack );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Surface material overrides: a surface, the whole mesh, an override of an instanced mesh, and one copy of the mesh per combination" )
    try
    {
        MakeWorkFolder( "ovr" );
        SGodotSurface a = GodotTriangleSurface( "SubResource(\"matA\")" );
        SGodotSurface b = GodotTriangleSurface( "SubResource(\"matB\")" );
        vector< SGodotSurface > surfaces;
        surfaces.push_back( a );
        surfaces.push_back( b );
        ASSERT_TRUE( WriteGltfModel( "work/ovr/models", "crate" ) );
        ASSERT_TRUE( WriteScene( "ovr/s.tscn",
            "[ext_resource type=\"PackedScene\" path=\"res://models/crate.gltf\" id=\"1_c\"]\n\n"
            "[sub_resource type=\"StandardMaterial3D\" id=\"matA\"]\nalbedo_color = Color(1, 0, 0, 1)\n\n"
            "[sub_resource type=\"StandardMaterial3D\" id=\"matB\"]\nalbedo_color = Color(0, 1, 0, 1)\n\n"
            "[sub_resource type=\"StandardMaterial3D\" id=\"matC\"]\nalbedo_color = Color(0, 0, 1, 1)\n\n" + GodotArrayMeshText( "Two", surfaces ) + "\n"
            "[node name=\"Root\" type=\"Node3D\"]\n\n"
            "[node name=\"Plain\" type=\"MeshInstance3D\" parent=\".\"]\nmesh = SubResource(\"Two\")\n\n"
            "[node name=\"SecondOnly\" type=\"MeshInstance3D\" parent=\".\"]\nmesh = SubResource(\"Two\")\nsurface_material_override/1 = SubResource(\"matC\")\n\n"
            "[node name=\"SecondOnlyAgain\" type=\"MeshInstance3D\" parent=\".\"]\nmesh = SubResource(\"Two\")\nsurface_material_override/1 = SubResource(\"matC\")\n\n"
            "[node name=\"Whole\" type=\"MeshInstance3D\" parent=\".\"]\nmesh = SubResource(\"Two\")\nmaterial_override = SubResource(\"matC\")\n\n"
            "[node name=\"Crate\" parent=\".\" instance=ExtResource(\"1_c\")]\n\n"
            "[node name=\"Lid\" parent=\"Crate/Crate\"]\nsurface_material_override/0 = SubResource(\"matC\")\n" ) );
        ASSERT_TRUE( WriteWorkText( "ovr/project.godot", "config_version=5\n" ) );
        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportGodot( "work/ovr", "s.tscn", scene, report ) );
        const Int32 plain = FindNodeByName( scene, "Plain" );
        const Int32 second = FindNodeByName( scene, "SecondOnly" );
        const Int32 again = FindNodeByName( scene, "SecondOnlyAgain" );
        const Int32 whole = FindNodeByName( scene, "Whole" );
        const Int32 lid = FindNodeByName( scene, "Lid" );
        const Int32 wheel = FindNodeByName( scene, "Wheel" );
        REQUIRE( plain >= 0 && second >= 0 && again >= 0 && whole >= 0 && lid >= 0 && wheel >= 0 );
        const CAssetMesh& plainMesh = scene.meshes[ scene.nodes[ plain ].mesh ];
        const CAssetMesh& secondMesh = scene.meshes[ scene.nodes[ second ].mesh ];
        const CAssetMesh& wholeMesh = scene.meshes[ scene.nodes[ whole ].mesh ];
        ASSERT_TRUE( "matA" == scene.materials[ plainMesh.subMeshes[ 0 ].materialIndex ].name && "matB" == scene.materials[ plainMesh.subMeshes[ 1 ].materialIndex ].name );
        ASSERT_TRUE( "matA" == scene.materials[ secondMesh.subMeshes[ 0 ].materialIndex ].name && "matC" == scene.materials[ secondMesh.subMeshes[ 1 ].materialIndex ].name );
        ASSERT_TRUE( "matC" == scene.materials[ wholeMesh.subMeshes[ 0 ].materialIndex ].name && "matC" == scene.materials[ wholeMesh.subMeshes[ 1 ].materialIndex ].name );
        ASSERT_TRUE( scene.nodes[ second ].mesh == scene.nodes[ again ].mesh && scene.nodes[ plain ].mesh != scene.nodes[ second ].mesh && scene.nodes[ whole ].mesh != scene.nodes[ second ].mesh );
        // the instanced model: the lid has the material of the override, the wheel keeps its own
        ASSERT_TRUE( "matC" == scene.materials[ scene.meshes[ scene.nodes[ lid ].mesh ].subMeshes[ 0 ].materialIndex ].name );
        ASSERT_TRUE( "Painted" == scene.materials[ scene.meshes[ scene.nodes[ wheel ].mesh ].subMeshes[ 0 ].materialIndex ].name && scene.nodes[ lid ].mesh != scene.nodes[ wheel ].mesh );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PackedScene instances: the nested scene is read in place, a prefab instance with the overrides of the instancing scene is recorded, per instance" )
    try
    {
        MakeWorkFolder( "prefab" );
        ASSERT_TRUE( WriteScene( "prefab/scenes/wall.tscn",
            "[sub_resource type=\"BoxMesh\" id=\"b\"]\n\n"
            "[node name=\"Wall\" type=\"Node3D\"]\n\n"
            "[node name=\"Slab\" type=\"MeshInstance3D\" parent=\".\"]\nmesh = SubResource(\"b\")\n\n"
            "[node name=\"Lamp\" type=\"OmniLight3D\" parent=\".\"]\nlight_energy = 2.0\n" ) );
        ASSERT_TRUE( WriteScene( "prefab/scenes/level.tscn",
            "[ext_resource type=\"PackedScene\" path=\"res://scenes/wall.tscn\" id=\"1_w\"]\n\n" + CORE::CString( Mat ) +
            "[node name=\"Level\" type=\"Node3D\"]\n\n"
            "[node name=\"WallA\" parent=\".\" instance=ExtResource(\"1_w\")]\ntransform = " + GodotTransformText( 0.0, 1.0, 10.0, 0.0, 0.0 ) + "\nmetadata/note = \"left\"\n\n"
            "[node name=\"Slab\" parent=\"WallA\"]\nsurface_material_override/0 = SubResource(\"red\")\nposition = Vector3(0, 0.5, 0)\n\n"
            "[node name=\"WallB\" parent=\".\" instance=ExtResource(\"1_w\")]\n" ) );
        ASSERT_TRUE( WriteWorkText( "prefab/project.godot", "config_version=5\n" ) );
        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportGodot( "work/prefab", "scenes/level.tscn", scene, report ) );
        ASSERT_TRUE( 0 == report.GetItemCount( REPORTITEM_ERROR ) );
        REQUIRE( 7 == scene.nodes.size() && 2 == scene.lights.size() );
        const Int32 wallA = FindNodeByName( scene, "WallA" );
        const Int32 wallB = FindNodeByName( scene, "WallB" );
        REQUIRE( wallA >= 0 && wallB >= 0 );
        ASSERT_TRUE( "scenes/wall.tscn" == scene.nodes[ wallA ].prefabInstance && "scenes/wall.tscn" == scene.nodes[ wallB ].prefabInstance );
        ASSERT_TRUE( Near( scene.nodes[ wallA ].transform.position.x, 10.0f ) && Near( scene.nodes[ wallB ].transform.position.x, 0.0f ) );

        // the slab of A is moved and red, the slab of B is as the nested scene has it
        Int32 slabA = -1;
        Int32 slabB = -1;
        for ( size_t n=0; n<scene.nodes.size(); ++n )
            if ( "Slab" == scene.nodes[ n ].name )
                ( scene.nodes[ n ].parent == wallA ? slabA : slabB ) = (Int32) n;
        REQUIRE( slabA >= 0 && slabB >= 0 && scene.nodes[ slabB ].parent == wallB );
        ASSERT_TRUE( Near( scene.nodes[ slabA ].transform.position.y, 0.5f ) && Near( scene.nodes[ slabB ].transform.position.y, 0.0f ) );
        ASSERT_TRUE( 0 <= scene.meshes[ scene.nodes[ slabA ].mesh ].subMeshes[ 0 ].materialIndex && -1 == scene.meshes[ scene.nodes[ slabB ].mesh ].subMeshes[ 0 ].materialIndex );

        // the prefab and its instances
        REQUIRE( 1 == scene.prefabs.size() && 2 == scene.prefabs[ 0 ].instances.size() );
        const CAssetPrefabInstance& instanceA = scene.prefabs[ 0 ].instances[ 0 ];
        ASSERT_TRUE( (Int32) instanceA.node == wallA && "scenes/wall.tscn" == instanceA.sourcePrefab && "left" == instanceA.overrides.GetAttributeValue( "metadata/note" ).AsString() );
        CORE::CDataNode::TVariantVector transformValues;
        ASSERT_TRUE( instanceA.overrides.GetValuesOfChildByName( "transform", transformValues ) && 12 == transformValues.size() && Near( transformValues[ 9 ].AsFloat32(), 10.0f ) );
        const CORE::CDataNode* nodes = instanceA.overrides.FindChild( "nodes" );
        REQUIRE( GUCEF_NULL != nodes );
        const CORE::CDataNode* slab = nodes->FindChild( "Slab" );
        ASSERT_TRUE( GUCEF_NULL != slab && "SubResource(red)" == slab->GetAttributeValue( "surface_material_override/0" ).AsString() );
        ASSERT_TRUE( (Int32) scene.prefabs[ 0 ].instances[ 1 ].node == wallB );

        // the files read are dependencies
        bool wallDependency = false;
        for ( size_t d=0; d<scene.dependencies.size(); ++d )
            if ( scene.dependencies[ d ].HasSubstr( "scenes/wall.tscn" ) >= 0 )
                wallDependency = true;
        ASSERT_TRUE( wallDependency );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Inherited scenes: the root is an instance of a base scene, nodes of the base are changed and nodes are added" )
    try
    {
        MakeWorkFolder( "inherit" );
        ASSERT_TRUE( WriteScene( "inherit/base.tscn",
            "[sub_resource type=\"BoxMesh\" id=\"b\"]\n\n[node name=\"Base\" type=\"Node3D\"]\n\n[node name=\"Mesh\" type=\"MeshInstance3D\" parent=\".\"]\nmesh = SubResource(\"b\")\n" ) );
        ASSERT_TRUE( WriteScene( "inherit/derived.tscn",
            "[ext_resource type=\"PackedScene\" path=\"res://base.tscn\" id=\"1_b\"]\n\n"
            "[node name=\"Derived\" instance=ExtResource(\"1_b\")]\n\n"
            "[node name=\"Mesh\" parent=\".\" index=\"0\"]\nposition = Vector3(7, 0, 0)\n\n"
            "[node name=\"Extra\" type=\"Node3D\" parent=\".\"]\n" ) );
        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportGodot( "work/inherit", "derived.tscn", scene, report ) );
        REQUIRE( 3 == scene.nodes.size() && 1 == scene.rootNodes.size() );
        ASSERT_TRUE( "Derived" == scene.nodes[ 0 ].name && "base.tscn" == scene.nodes[ 0 ].prefabInstance );
        const Int32 mesh = FindNodeByName( scene, "Mesh" );
        REQUIRE( mesh >= 0 );
        ASSERT_TRUE( Near( scene.nodes[ mesh ].transform.position.x, 7.0f ) && scene.nodes[ mesh ].mesh >= 0 && FindNodeByName( scene, "Extra" ) >= 0 );
        REQUIRE( 1 == scene.prefabs.size() && 1 == scene.prefabs[ 0 ].instances.size() );
        ASSERT_TRUE( 0 == scene.prefabs[ 0 ].instances[ 0 ].node && GUCEF_NULL != scene.prefabs[ 0 ].instances[ 0 ].overrides.FindChild( "nodes" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Instances: a scene that instances itself, a Blender source, a file that is not in the pack and a path that is found by its uid" )
    try
    {
        MakeWorkFolder( "inst2" );
        ASSERT_TRUE( WriteScene( "inst2/a.tscn", "[ext_resource type=\"PackedScene\" path=\"res://b.tscn\" id=\"1\"]\n\n[node name=\"A\" type=\"Node3D\"]\n\n[node name=\"B\" parent=\".\" instance=ExtResource(\"1\")]\n" ) );
        ASSERT_TRUE( WriteScene( "inst2/b.tscn", "[ext_resource type=\"PackedScene\" path=\"res://a.tscn\" id=\"1\"]\n\n[node name=\"B\" type=\"Node3D\"]\n\n[node name=\"A\" parent=\".\" instance=ExtResource(\"1\")]\n" ) );
        CAssetScene cycle;
        CConversionReport cycleReport;
        REQUIRE( ImportGodot( "work/inst2", "a.tscn", cycle, cycleReport ) );
        ASSERT_TRUE( ReportHas( cycleReport, REPORTITEM_ERROR, "instances itself" ) );

        MakeWorkFolder( "inst3" );
        ASSERT_TRUE( WriteWorkText( "inst3/robot.blend", "BLENDER" ) );
        ASSERT_TRUE( WriteWorkText( "inst3/moved/real.tscn", "[gd_scene format=3 uid=\"uid://real_one\"]\n\n[node name=\"Real\" type=\"Node3D\"]\n\n[node name=\"Inner\" type=\"Node3D\" parent=\".\"]\n" ) );
        ASSERT_TRUE( WriteScene( "inst3/s.tscn",
            "[ext_resource type=\"PackedScene\" path=\"res://robot.blend\" id=\"1\"]\n"
            "[ext_resource type=\"PackedScene\" path=\"res://gone.glb\" id=\"2\"]\n"
            "[ext_resource type=\"PackedScene\" uid=\"uid://real_one\" path=\"res://old/place.tscn\" id=\"3\"]\n\n"
            "[node name=\"Root\" type=\"Node3D\"]\n\n"
            "[node name=\"Robot\" parent=\".\" instance=ExtResource(\"1\")]\n\n"
            "[node name=\"Gone\" parent=\".\" instance=ExtResource(\"2\")]\n\n"
            "[node name=\"Real\" parent=\".\" instance=ExtResource(\"3\")]\n" ) );
        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportGodot( "work/inst3", "s.tscn", scene, report ) );
        ASSERT_TRUE( ReportHas( report, REPORTITEM_SKIPPED, "Blender" ) && ReportHas( report, REPORTITEM_WARNING, "gone.glb" ) );
        const Int32 robot = FindNodeByName( scene, "Robot" );
        REQUIRE( robot >= 0 && scene.nodes[ robot ].children.empty() );
        const Int32 real = FindNodeByName( scene, "Real" );
        REQUIRE( real >= 0 );
        ASSERT_TRUE( 2 == scene.nodes[ real ].children.size() + 1 || FindNodeByName( scene, "Inner" ) >= 0 );
        ASSERT_TRUE( "moved/real.tscn" == scene.nodes[ real ].prefabInstance );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Collision shapes and bodies: every shape kind, the body kinds, triggers, the scale of the shape, disabled and unsupported shapes" )
    try
    {
        MakeWorkFolder( "phys" );
        ASSERT_TRUE( WriteScene( "phys/p.tscn",
            "[sub_resource type=\"BoxShape3D\" id=\"b\"]\nsize = Vector3(2, 4, 6)\n\n"
            "[sub_resource type=\"SphereShape3D\" id=\"s\"]\nradius = 1.5\n\n"
            "[sub_resource type=\"CapsuleShape3D\" id=\"c\"]\nradius = 0.5\nheight = 2.0\n\n"
            "[sub_resource type=\"CylinderShape3D\" id=\"y\"]\nradius = 1.0\nheight = 3.0\n\n"
            "[sub_resource type=\"ConvexPolygonShape3D\" id=\"v\"]\npoints = PackedVector3Array(0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1)\n\n"
            "[sub_resource type=\"ConcavePolygonShape3D\" id=\"t\"]\ndata = PackedVector3Array(0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1)\n\n"
            "[sub_resource type=\"WorldBoundaryShape3D\" id=\"w\"]\n\n"
            "[sub_resource type=\"PhysicsMaterial\" id=\"pm\"]\nfriction = 0.4\nbounce = 0.2\n\n"
            "[node name=\"World\" type=\"Node3D\"]\n\n"
            "[node name=\"Floor\" type=\"StaticBody3D\" parent=\".\"]\ncollision_layer = 3\n\n"
            "[node name=\"BoxShape\" type=\"CollisionShape3D\" parent=\"Floor\"]\nposition = Vector3(0, 1, 0)\nshape = SubResource(\"b\")\n\n"
            "[node name=\"Off\" type=\"CollisionShape3D\" parent=\"Floor\"]\nshape = SubResource(\"b\")\ndisabled = true\n\n"
            "[node name=\"Plane\" type=\"CollisionShape3D\" parent=\"Floor\"]\nshape = SubResource(\"w\")\n\n"
            "[node name=\"Ball\" type=\"RigidBody3D\" parent=\".\"]\nposition = Vector3(4, 0, 0)\nmass = 5.0\ngravity_scale = 0.5\nphysics_material_override = SubResource(\"pm\")\n\n"
            "[node name=\"SphereShape\" type=\"CollisionShape3D\" parent=\"Ball\"]\nshape = SubResource(\"s\")\n\n"
            "[node name=\"Pill\" type=\"CharacterBody3D\" parent=\".\"]\n\n"
            "[node name=\"CapsuleShape\" type=\"CollisionShape3D\" parent=\"Pill\"]\nposition = Vector3(0, 1, 0)\nshape = SubResource(\"c\")\n\n"
            "[node name=\"Zone\" type=\"Area3D\" parent=\".\"]\n\n"
            "[node name=\"CylinderShape\" type=\"CollisionShape3D\" parent=\"Zone\"]\nscale = Vector3(2, 1, 2)\nshape = SubResource(\"y\")\n\n"
            "[node name=\"Hull\" type=\"StaticBody3D\" parent=\".\"]\n\n"
            "[node name=\"ConvexShape\" type=\"CollisionShape3D\" parent=\"Hull\"]\nscale = Vector3(2, 2, 2)\nshape = SubResource(\"v\")\n\n"
            "[node name=\"Soup\" type=\"StaticBody3D\" parent=\".\"]\n\n"
            "[node name=\"SoupShape\" type=\"CollisionShape3D\" parent=\"Soup\"]\nshape = SubResource(\"t\")\n\n"
            "[node name=\"Lonely\" type=\"CollisionShape3D\" parent=\".\"]\nshape = SubResource(\"b\")\n" ) );
        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportGodot( "work/phys", "p.tscn", scene, report ) );

        const Int32 floor = FindNodeByName( scene, "Floor" );
        REQUIRE( floor >= 0 && 1 == scene.nodes[ floor ].colliders.size() );
        const CAssetCollider& box = scene.nodes[ floor ].colliders[ 0 ];
        ASSERT_TRUE( COLLIDERSHAPE_BOX == box.shape && Near( box.halfExtents[ 0 ], 1.0f ) && Near( box.halfExtents[ 1 ], 2.0f ) && Near( box.halfExtents[ 2 ], 3.0f ) && !box.trigger );
        ASSERT_TRUE( Near( box.transform.position.y, 1.0f ) && "BoxShape" == box.name );
        ASSERT_TRUE( ReportHas( report, REPORTITEM_SKIPPED, "WorldBoundaryShape3D" ) );
        ASSERT_TRUE( "static" == scene.nodes[ floor ].extras.FindChild( "godotBody" )->GetAttributeValue( "motion" ).AsString() && 3 == scene.nodes[ floor ].extras.FindChild( "godotBody" )->GetAttributeValue( "layer" ).AsInt32() );

        const Int32 ball = FindNodeByName( scene, "Ball" );
        REQUIRE( ball >= 0 && 1 == scene.nodes[ ball ].colliders.size() );
        ASSERT_TRUE( COLLIDERSHAPE_SPHERE == scene.nodes[ ball ].colliders[ 0 ].shape && Near( scene.nodes[ ball ].colliders[ 0 ].radius, 1.5f ) );
        const CORE::CDataNode* body = scene.nodes[ ball ].extras.FindChild( "godotBody" );
        REQUIRE( GUCEF_NULL != body );
        ASSERT_TRUE( "dynamic" == body->GetAttributeValue( "motion" ).AsString() && Near( body->GetAttributeValue( "mass" ).AsFloat32(), 5.0f ) && Near( body->GetAttributeValue( "gravityScale" ).AsFloat32(), 0.5f ) );
        ASSERT_TRUE( Near( body->GetAttributeValue( "friction" ).AsFloat32(), 0.4f ) && Near( body->GetAttributeValue( "bounce" ).AsFloat32(), 0.2f ) );

        const Int32 pill = FindNodeByName( scene, "Pill" );
        REQUIRE( pill >= 0 && 1 == scene.nodes[ pill ].colliders.size() );
        ASSERT_TRUE( COLLIDERSHAPE_CAPSULE == scene.nodes[ pill ].colliders[ 0 ].shape && Near( scene.nodes[ pill ].colliders[ 0 ].radius, 0.5f ) && Near( scene.nodes[ pill ].colliders[ 0 ].height, 2.0f ) );
        ASSERT_TRUE( "kinematic" == scene.nodes[ pill ].extras.FindChild( "godotBody" )->GetAttributeValue( "motion" ).AsString() );

        const Int32 zone = FindNodeByName( scene, "Zone" );
        REQUIRE( zone >= 0 && 1 == scene.nodes[ zone ].colliders.size() );
        const CAssetCollider& cylinder = scene.nodes[ zone ].colliders[ 0 ];
        ASSERT_TRUE( COLLIDERSHAPE_CYLINDER == cylinder.shape && cylinder.trigger && Near( cylinder.radius, 2.0f ) && Near( cylinder.height, 3.0f ) );
        ASSERT_TRUE( "trigger" == scene.nodes[ zone ].extras.FindChild( "godotBody" )->GetAttributeValue( "motion" ).AsString() );

        const Int32 hull = FindNodeByName( scene, "Hull" );
        REQUIRE( hull >= 0 && 1 == scene.nodes[ hull ].colliders.size() && COLLIDERSHAPE_CONVEX == scene.nodes[ hull ].colliders[ 0 ].shape );
        const CAssetMesh& hullMesh = scene.meshes[ scene.nodes[ hull ].colliders[ 0 ].meshIndex ];
        Float32 low[ 3 ];
        Float32 high[ 3 ];
        Bounds( hullMesh.subMeshes[ 0 ], low, high );
        ASSERT_TRUE( 4 == hullMesh.subMeshes[ 0 ].GetVertexCount() && 4 == hullMesh.subMeshes[ 0 ].GetTriangleCount() && Near( high[ 0 ], 2.0f ) && Near( high[ 2 ], 2.0f ) );

        const Int32 soup = FindNodeByName( scene, "Soup" );
        REQUIRE( soup >= 0 && 1 == scene.nodes[ soup ].colliders.size() && COLLIDERSHAPE_TRIMESH == scene.nodes[ soup ].colliders[ 0 ].shape );
        const CAssetSubMesh& soupMesh = scene.meshes[ scene.nodes[ soup ].colliders[ 0 ].meshIndex ].subMeshes[ 0 ];
        ASSERT_TRUE( 6 == soupMesh.GetVertexCount() && 2 == soupMesh.GetTriangleCount() );

        const Int32 lonely = FindNodeByName( scene, "Lonely" );
        REQUIRE( lonely >= 0 && 1 == scene.nodes[ lonely ].colliders.size() );
        ASSERT_TRUE( COLLIDERSHAPE_BOX == scene.nodes[ lonely ].colliders[ 0 ].shape );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Lights: OmniLight3D, SpotLight3D and DirectionalLight3D with colour (linear), energy, range and cone" )
    try
    {
        MakeWorkFolder( "lights" );
        ASSERT_TRUE( WriteScene( "lights/l.tscn",
            "[node name=\"Root\" type=\"Node3D\"]\n\n"
            "[node name=\"Omni\" type=\"OmniLight3D\" parent=\".\"]\nlight_color = Color(1, 0.5, 0.25, 1)\nlight_energy = 2.0\nomni_range = 10.0\nshadow_enabled = true\n\n"
            "[node name=\"Spot\" type=\"SpotLight3D\" parent=\".\"]\nlight_energy = 3.0\nspot_range = 8.0\nspot_angle = 30.0\n\n"
            "[node name=\"Sun\" type=\"DirectionalLight3D\" parent=\".\"]\n" ) );
        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportGodot( "work/lights", "l.tscn", scene, report ) );
        REQUIRE( 3 == scene.lights.size() );
        const CAssetLight& omni = scene.lights[ scene.nodes[ FindNodeByName( scene, "Omni" ) ].light ];
        ASSERT_TRUE( LIGHTTYPE_POINT == omni.type && Near( omni.intensity, 2.0f ) && Near( omni.range, 10.0f ) && Near( omni.colour[ 0 ], 1.0f ) );
        ASSERT_TRUE( Near( omni.colour[ 1 ], 0.21404114f, 1e-4f ) && Near( omni.colour[ 2 ], 0.05087609f, 1e-4f ) );
        ASSERT_TRUE( scene.nodes[ FindNodeByName( scene, "Omni" ) ].extras.FindChild( "godotLight" )->GetAttributeValue( "shadowEnabled" ).AsBool() );
        const CAssetLight& spot = scene.lights[ scene.nodes[ FindNodeByName( scene, "Spot" ) ].light ];
        ASSERT_TRUE( LIGHTTYPE_SPOT == spot.type && Near( spot.intensity, 3.0f ) && Near( spot.range, 8.0f ) && Near( spot.outerConeAngle, 0.5235988f, 1e-4f ) && Near( spot.innerConeAngle, 0.0f ) );
        const CAssetLight& sun = scene.lights[ scene.nodes[ FindNodeByName( scene, "Sun" ) ].light ];
        ASSERT_TRUE( LIGHTTYPE_DIRECTIONAL == sun.type && Near( sun.intensity, 1.0f ) && Near( sun.colour[ 2 ], 1.0f ) );
        ASSERT_TRUE( ReportHas( report, REPORTITEM_APPROXIMATION, "light_energy" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Import settings of a model file: the root scale, skipped nodes, the animation loop, an external material and the animations turned off" )
    try
    {
        MakeWorkFolder( "settings" );
        ASSERT_TRUE( WriteGltfModel( "work/settings/models", "crate" ) );
        ASSERT_TRUE( WriteWorkText( "settings/models/crate.gltf.import",
            "[remap]\n\nimporter=\"scene\"\nuid=\"uid://crate_model\"\n\n[params]\n\nnodes/root_scale=2.0\nanimation/import=true\n"
            "_subresources={\n\"animations\": {\n\"Spin\": {\n\"settings/loop_mode\": 1\n}\n},\n\"materials\": {\n\"Painted\": {\n\"use_external/enabled\": true,\n\"use_external/path\": \"uid://nowhere\",\n\"use_external/fallback_path\": \"res://mats/steel.tres\"\n}\n},\n"
            "\"nodes\": {\n\"Crate/Wheel\": {\n\"import/skip_import\": true\n},\n\"PATH:Crate/Wheel\": {\n\"import/skip_import\": true\n}\n}\n}\n" ) );
        ASSERT_TRUE( WriteWorkText( "settings/mats/steel.tres", "[gd_resource type=\"StandardMaterial3D\" format=3]\n\n[resource]\nresource_name = \"Steel\"\nalbedo_color = Color(0, 0, 1, 1)\nmetallic = 1.0\n" ) );
        ASSERT_TRUE( WriteWorkText( "settings/project.godot", "config_version=5\n" ) );
        ASSERT_TRUE( WriteScene( "settings/s.tscn", "[ext_resource type=\"PackedScene\" path=\"res://models/crate.gltf\" id=\"1_c\"]\n\n[node name=\"Root\" type=\"Node3D\"]\n\n[node name=\"Crate\" parent=\".\" instance=ExtResource(\"1_c\")]\n" ) );
        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportGodot( "work/settings", "s.tscn", scene, report ) );
        // Crate (the instance) > Crate (the model root) > Lid; the wheel is skipped
        ASSERT_TRUE( FindNodeByName( scene, "Wheel" ) < 0 && FindNodeByName( scene, "Lid" ) >= 0 );
        const Int32 modelRoot = scene.nodes[ FindNodeByName( scene, "Crate" ) ].children.empty() ? -1 : (Int32) scene.nodes[ FindNodeByName( scene, "Crate" ) ].children[ 0 ];
        REQUIRE( modelRoot >= 0 );
        ASSERT_TRUE( Near( scene.nodes[ modelRoot ].transform.scale.x, 2.0f ) && Near( scene.nodes[ modelRoot ].transform.scale.y, 2.0f ) );
        const MATH::TFloat32Point3D lid = WorldPoint( scene, (UInt32) FindNodeByName( scene, "Lid" ), 0.0f, 0.0f, 0.0f );
        ASSERT_TRUE( Near( lid.y, 2.0f, 1e-4f ) );
        REQUIRE( 1 == scene.animations.size() );
        ASSERT_TRUE( "Spin" == scene.animations[ 0 ].name && scene.animations[ 0 ].loop );
        // the material of the file is replaced by the external one
        const Int32 lidNode = FindNodeByName( scene, "Lid" );
        const Int32 material = scene.meshes[ scene.nodes[ lidNode ].mesh ].subMeshes[ 0 ].materialIndex;
        REQUIRE( material >= 0 );
        ASSERT_TRUE( "Steel" == scene.materials[ material ].name && Near( scene.materials[ material ].metallicFactor, 1.0f ) );
        // the settings are in the extras for the sidecar
        const CORE::CDataNode* record = scene.extras.FindChild( "godotImport" );
        ASSERT_TRUE( GUCEF_NULL != record && GUCEF_NULL != record->FindChild( "models/crate.gltf" ) );

        // the animations off
        ASSERT_TRUE( WriteWorkText( "settings/models/crate.gltf.import", "[remap]\n\nimporter=\"scene\"\n\n[params]\n\nanimation/import=false\n" ) );
        CAssetScene noAnimations;
        CConversionReport noAnimationsReport;
        REQUIRE( ImportGodot( "work/settings", "s.tscn", noAnimations, noAnimationsReport ) );
        ASSERT_TRUE( noAnimations.animations.empty() && FindNodeByName( noAnimations, "Wheel" ) >= 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mesh resources as assets: a .tres ArrayMesh and a .tres BoxMesh become a node with the mesh; Godot 3 files and other resources are refused" )
    try
    {
        MakeWorkFolder( "tres" );
        vector< SGodotSurface > surfaces;
        surfaces.push_back( GodotTriangleSurface( "" ) );
        const CORE::CString arrayMesh = GodotArrayMeshText( "m", surfaces );
        // a resource file holds its mesh as [resource]: the surfaces are the properties of the main resource
        const Int32 start = arrayMesh.HasSubstr( "_surfaces" );
        ASSERT_TRUE( start >= 0 );
        ASSERT_TRUE( WriteWorkText( "tres/tri.tres", "[gd_resource type=\"ArrayMesh\" format=3]\n\n[resource]\n" + arrayMesh.SubstrFromRange( (UInt32) start, arrayMesh.Length() ) ) );
        ASSERT_TRUE( WriteWorkText( "tres/box.tres", "[gd_resource type=\"BoxMesh\" format=3]\n\n[resource]\nsize = Vector3(1, 2, 3)\n" ) );
        ASSERT_TRUE( WriteWorkText( "tres/old.tscn", "[gd_scene load_steps=2 format=2]\n\n[node name=\"Old\" type=\"Spatial\"]\n" ) );
        ASSERT_TRUE( WriteWorkText( "tres/mat.tres", "[gd_resource type=\"StandardMaterial3D\" format=3]\n\n[resource]\n" ) );
        CAssetScene tri;
        CConversionReport triReport;
        REQUIRE( ImportGodot( "work/tres", "tri.tres", tri, triReport ) );
        REQUIRE( 1 == tri.nodes.size() && 0 == tri.nodes[ 0 ].mesh && 3 == tri.meshes[ 0 ].subMeshes[ 0 ].GetVertexCount() && "tri" == tri.nodes[ 0 ].name );
        CAssetScene box;
        CConversionReport boxReport;
        REQUIRE( ImportGodot( "work/tres", "box.tres", box, boxReport ) );
        ASSERT_TRUE( 1 == box.nodes.size() && 24 == box.meshes[ 0 ].subMeshes[ 0 ].GetVertexCount() );
        CAssetScene old;
        CConversionReport oldReport;
        ASSERT_FALSE( ImportGodot( "work/tres", "old.tscn", old, oldReport ) );
        ASSERT_TRUE( ReportHas( oldReport, REPORTITEM_ERROR, "format" ) );
        CAssetScene material;
        CConversionReport materialReport;
        ASSERT_FALSE( ImportGodot( "work/tres", "mat.tres", material, materialReport ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Node types: nodes without a counterpart (camera, audio, navigation) stay as nodes and are reported, a particle node becomes a particle system; scripts are noted once" )
    try
    {
        MakeWorkFolder( "types" );
        ASSERT_TRUE( WriteScene( "types/t.tscn",
            "[ext_resource type=\"Script\" path=\"res://a.gd\" id=\"1\"]\n\n"
            "[node name=\"Root\" type=\"Node3D\"]\nscript = ExtResource(\"1\")\n\n"
            "[node name=\"Camera\" type=\"Camera3D\" parent=\".\"]\ncurrent = true\n\n"
            "[node name=\"Dust\" type=\"GPUParticles3D\" parent=\".\"]\n\n"
            "[node name=\"Plain\" type=\"Marker3D\" parent=\".\"]\nvisible = false\n" ) );
        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportGodot( "work/types", "t.tscn", scene, report ) );
        ASSERT_TRUE( 4 == scene.nodes.size() && ReportHas( report, REPORTITEM_SKIPPED, "Camera3D" ) && !ReportHas( report, REPORTITEM_SKIPPED, "node type GPUParticles3D" ) && ReportHas( report, REPORTITEM_INFO, "scripts" ) );
        ASSERT_TRUE( "Camera3D" == scene.nodes[ FindNodeByName( scene, "Camera" ) ].extras.GetAttributeValue( "godotType" ).AsString() );
        // the particle node without a draw pass: a particle system that draws nothing, reported
        ASSERT_TRUE( 1 == scene.particleSystems.size() && 0 == scene.nodes[ FindNodeByName( scene, "Dust" ) ].particleSystem && ReportHas( report, REPORTITEM_SKIPPED, "no draw pass" ) );
        ASSERT_FALSE( scene.nodes[ FindNodeByName( scene, "Plain" ) ].extras.GetAttributeValue( "visible" ).AsBool( true ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
