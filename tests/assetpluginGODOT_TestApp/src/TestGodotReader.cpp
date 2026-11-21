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

#include "AssetConverterLib_CAssetPluginRegistry.h"
#include "AssetConverterLib_CAssetConverterGlobal.h"
#include "assetpluginGODOT_CGodotTextReader.h"
#include "assetpluginGODOT_CGodotVariant.h"
#include "assetpluginGODOT_CGodotDocument.h"
#include "assetpluginGODOT_CGodotPaths.h"
#include "assetpluginGODOT_CGodotImportSettings.h"

#include "TestGodotProject.h"
#include "TestGodotReader.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETCONVERTER;
using namespace GUCEF::ASSETPLUGIN::GODOT;

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

/** Parses a literal; the node has the type and the value */
bool
ParseLiteral( const CORE::CString& text, CORE::CDataNode& value )
{
    CGodotTextReader reader;
    return reader.ParseValue( text, value );
}

bool
NumbersAre( const CORE::CDataNode& value, const double* expected, UInt32 count )
{
    CGodotVariant::TNumberVector numbers;
    if ( !CGodotVariant::GetNumbers( &value, numbers ) || numbers.size() != count )
        return false;
    for ( UInt32 i=0; i<count; ++i )
        if ( fabs( numbers[ i ] - expected[ i ] ) > 1e-5 )
            return false;
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/

static void
TestGodotPackRecogniser( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Recogniser: a Godot project is recognised by project.godot, with its version, main scene, other scenes and noise" )
    try
    {
        TAssetImporterPtr importer;
        ASSERT_TRUE( CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetImporter( "godot", importer ) );
        CAssetPackListing project;
        project.AddFile( "project.godot", "config_version=5\n\n[application]\n\nconfig/name=\"Demo\"\nrun/main_scene=\"res://scenes/main.tscn\"\nconfig/features=PackedStringArray(\"4.2\", \"Forward Plus\")\n" );
        project.AddFile( "scenes/main.tscn" );
        project.AddFile( "scenes/Sample.tscn" );
        project.AddFile( "levels/other.tscn" );
        project.AddFile( "icon.svg.import" );
        project.AddFile( ".godot/imported/a.ctex" );
        CAssetPackProfile profile;
        ASSERT_TRUE( importer->RecognisePack( project, profile ) );
        ASSERT_TRUE( "godot-project" == profile.packType && "Godot project" == profile.packTypeName && "Godot" == profile.engine && "4.2" == profile.engineVersion && 90 == profile.confidence );
        ASSERT_TRUE( 3 == profile.showcaseCandidates.size() && "scenes/main.tscn" == profile.showcaseCandidates[ 0 ] && "scenes/Sample.tscn" == profile.showcaseCandidates[ 1 ] );
        ASSERT_TRUE( profile.IsNoise( "icon.svg.import" ) && profile.IsNoise( ".godot/imported/a.ctex" ) && !profile.IsNoise( "scenes/main.tscn" ) );
        ASSERT_TRUE( profile.groupingHints.size() >= 3 );

        // a project one folder down, as a zip of a repository holds it
        CAssetPackListing nested;
        nested.AddFile( "game-main/project.godot", "config_version=5\nconfig/features=PackedStringArray(\"4.4\")\n" );
        nested.AddFile( "game-main/main.tscn" );
        ASSERT_TRUE( importer->RecognisePack( nested, profile ) );
        ASSERT_TRUE( "game-main" == profile.root && "4.4" == profile.engineVersion && 1 == profile.showcaseCandidates.size() && "main.tscn" == profile.showcaseCandidates[ 0 ] );

        CAssetPackListing other;
        other.AddFile( "scenes/main.tscn" );
        ASSERT_FALSE( importer->RecognisePack( other, profile ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Recogniser: the Godot importer declares its file kinds (scene, resource, shader, package, import metadata)" )
    try
    {
        TAssetImporterPtr importer;
        ASSERT_TRUE( CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetImporter( "godot", importer ) );
        TAssetFileKindVector declared;
        importer->GetFileKinds( declared );
        CAssetKindRegistry kinds;
        kinds.RegisterFileKinds( declared, "importer:godot" );
        ASSERT_TRUE( ASSETKIND_PREFAB == kinds.GetKindOfPath( "a.tscn" ) && ASSETKIND_DATA == kinds.GetKindOfPath( "a.tres" ) && ASSETKIND_SHADER == kinds.GetKindOfPath( "a.gdshader" ) );
        ASSERT_TRUE( ASSETKIND_PACKAGE == kinds.GetKindOfPath( "game.pck" ) && ASSETKIND_METADATA == kinds.GetKindOfPath( "a.png.import" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}


void
PerformGodotReaderTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetpluginGODOT.Reader" );

    TestGodotPackRecogniser();

    GUCEF_TESTFW_TESTCASE( "Plugin: the importer \"godot\" is registered with its version and takes scenes and mesh resources only" )
    try
    {
        TAssetImporterPtr importer;
        ASSERT_TRUE( CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetImporter( "godot", importer ) );
        ASSERT_TRUE( 3 == importer->GetVersion() );
        MakeWorkFolder( "plugin" );
        ASSERT_TRUE( WriteWorkText( "plugin/mesh.tres", "[gd_resource type=\"ArrayMesh\" format=3]\n\n[resource]\n" ) );
        ASSERT_TRUE( WriteWorkText( "plugin/box.tres", "[gd_resource type=\"BoxMesh\" format=3]\n\n[resource]\n" ) );
        ASSERT_TRUE( WriteWorkText( "plugin/material.tres", "[gd_resource type=\"StandardMaterial3D\" format=3]\n\n[resource]\n" ) );
        ASSERT_TRUE( WriteWorkText( "plugin/scene.tscn", "[gd_scene format=3]\n\n[node name=\"Root\" type=\"Node3D\"]\n" ) );
        ASSERT_TRUE( WriteWorkText( "plugin/model.glb", "x" ) );
        CAssetPackContext context;
        ASSERT_TRUE( importer->CanImport( "work/plugin/scene.tscn", context ) && importer->CanImport( "work/plugin/SCENE.TSCN", context ) );
        ASSERT_TRUE( importer->CanImport( "work/plugin/mesh.tres", context ) && importer->CanImport( "work/plugin/box.tres", context ) );
        ASSERT_FALSE( importer->CanImport( "work/plugin/material.tres", context ) || importer->CanImport( "work/plugin/model.glb", context ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Reader: every section type of a scene with its header values, references and groups" )
    try
    {
        const CORE::CString text =
            "[gd_scene load_steps=4 format=3 uid=\"uid://cecaux1sm7mo0\"]\n\n"
            "[ext_resource type=\"Script\" path=\"res://Player.gd\" id=\"1_9ur2p\"]\n"
            "[ext_resource type=\"PackedScene\" uid=\"uid://b1x\" path=\"res://model.glb\" id=\"2_abc\"]\n\n"
            "[sub_resource type=\"BoxShape3D\" id=\"BoxShape3D_1\"]\nsize = Vector3(2, 2, 2)\n\n"
            "[node name=\"Root\" type=\"Node3D\"]\ntransform = Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0)\n\n"
            "[node name=\"Child\" parent=\".\" instance=ExtResource(\"2_abc\") groups=[\"a\", \"b\"] index=\"0\"]\nvisible = false\n\n"
            "[connection signal=\"ready\" from=\".\" to=\"Child\" method=\"_on_ready\"]\n"
            "[editable path=\"Child\"]\n";
        CGodotDocument document;
        REQUIRE( document.LoadText( text, "a.tscn" ) );
        ASSERT_TRUE( "gd_scene" == document.GetKind() && 3 == document.GetFormat() && "uid://cecaux1sm7mo0" == document.GetUid() );
        CORE::CString type;
        CORE::CString path;
        CORE::CString uid;
        ASSERT_TRUE( document.GetExtResourceInfo( "2_abc", type, path, uid ) && "PackedScene" == type && "res://model.glb" == path && "uid://b1x" == uid );
        ASSERT_TRUE( document.GetExtResourceInfo( "1_9ur2p", type, path, uid ) && "Script" == type && uid.IsNULLOrEmpty() );
        ASSERT_FALSE( document.GetExtResourceInfo( "nope", type, path, uid ) );
        const CORE::CDataNode* box = document.FindSubResource( "BoxShape3D_1" );
        REQUIRE( GUCEF_NULL != box && "BoxShape3D" == box->GetAttributeValue( "type" ).AsString() );
        const double expectedSize[ 3 ] = { 2, 2, 2 };
        ASSERT_TRUE( CGodotVariant::IsType( CGodotVariant::FindProperty( *box, "size" ), "Vector3" ) && NumbersAre( *CGodotVariant::FindProperty( *box, "size" ), expectedSize, 3 ) );

        REQUIRE( 2 == document.GetNodeSections().size() );
        const CORE::CDataNode& root = *document.GetNodeSections()[ 0 ];
        ASSERT_TRUE( "Root" == root.GetAttributeValue( "name" ).AsString() && "Node3D" == root.GetAttributeValue( "type" ).AsString() && !root.IsAttribute( "parent" ) );
        const CORE::CDataNode& child = *document.GetNodeSections()[ 1 ];
        ASSERT_TRUE( "." == child.GetAttributeValue( "parent" ).AsString() && "2_abc" == child.GetAttributeValue( "instance" ).AsString() && "0" == child.GetAttributeValue( "index" ).AsString() );
        const CORE::CDataNode* header = child.FindChild( "header" );
        REQUIRE( GUCEF_NULL != header );
        ASSERT_TRUE( CGodotVariant::IsType( header->FindChild( "instance" ), "ExtResource" ) );
        CORE::CString::StringVector groups;
        ASSERT_TRUE( CGodotVariant::GetStrings( header->FindChild( "groups" ), groups ) && 2 == groups.size() && "a" == groups[ 0 ] && "b" == groups[ 1 ] );
        ASSERT_FALSE( CGodotVariant::GetBool( CGodotVariant::FindProperty( child, "visible" ), true ) );

        const CORE::CDataNode* connection = document.FindSection( "connection" );
        ASSERT_TRUE( GUCEF_NULL != connection && "ready" == connection->GetAttributeValue( "signal" ).AsString() && "_on_ready" == connection->GetAttributeValue( "method" ).AsString() );
        const CORE::CDataNode* editable = document.FindSection( "editable" );
        ASSERT_TRUE( GUCEF_NULL != editable && "Child" == editable->GetAttributeValue( "path" ).AsString() );

        // a resource file
        CGodotDocument resource;
        REQUIRE( resource.LoadText( "[gd_resource type=\"StandardMaterial3D\" format=3]\n\n[resource]\nalbedo_color = Color(1, 0, 0, 1)\n", "m.tres" ) );
        ASSERT_TRUE( "gd_resource" == resource.GetKind() && "StandardMaterial3D" == resource.GetResourceType() && GUCEF_NULL != resource.GetMainResource() );
        ASSERT_TRUE( CGodotVariant::IsType( CGodotVariant::FindProperty( *resource.GetMainResource(), "albedo_color" ), "Color" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Reader: the scalar literals (numbers in every form, booleans, null, strings with escapes, StringName, NodePath)" )
    try
    {
        CORE::CDataNode value;
        REQUIRE( ParseLiteral( "42", value ) );
        ASSERT_TRUE( CGodotVariant::IsType( &value, "int" ) && 42 == CGodotVariant::GetInteger( &value, 0 ) );
        REQUIRE( ParseLiteral( "-7", value ) );
        ASSERT_TRUE( CGodotVariant::IsType( &value, "int" ) && -7 == CGodotVariant::GetInteger( &value, 0 ) );
        REQUIRE( ParseLiteral( "0x1F", value ) );
        ASSERT_TRUE( 31 == CGodotVariant::GetInteger( &value, 0 ) );
        REQUIRE( ParseLiteral( "34359742487", value ) );
        ASSERT_TRUE( CGodotVariant::IsType( &value, "int" ) && 34359742487LL == CGodotVariant::GetInteger( &value, 0 ) );
        REQUIRE( ParseLiteral( "3.5", value ) );
        ASSERT_TRUE( CGodotVariant::IsType( &value, "float" ) && Near( (Float32) CGodotVariant::GetNumber( &value, 0.0 ), 3.5f ) );
        REQUIRE( ParseLiteral( "-1.5e-3", value ) );
        ASSERT_TRUE( Near( (Float32) CGodotVariant::GetNumber( &value, 0.0 ), -0.0015f, 1e-8f ) );
        REQUIRE( ParseLiteral( "1e3", value ) );
        ASSERT_TRUE( CGodotVariant::IsType( &value, "float" ) && Near( (Float32) CGodotVariant::GetNumber( &value, 0.0 ), 1000.0f ) );
        REQUIRE( ParseLiteral( "inf", value ) );
        ASSERT_TRUE( CGodotVariant::GetNumber( &value, 0.0 ) > 1e300 );
        REQUIRE( ParseLiteral( "-inf", value ) );
        ASSERT_TRUE( CGodotVariant::GetNumber( &value, 0.0 ) < -1e300 );
        REQUIRE( ParseLiteral( "nan", value ) );
        ASSERT_TRUE( CGodotVariant::IsType( &value, "float" ) && CGodotVariant::GetNumber( &value, 0.0 ) != CGodotVariant::GetNumber( &value, 0.0 ) );
        REQUIRE( ParseLiteral( "true", value ) );
        ASSERT_TRUE( CGodotVariant::IsType( &value, "bool" ) && CGodotVariant::GetBool( &value, false ) );
        REQUIRE( ParseLiteral( "false", value ) );
        ASSERT_FALSE( CGodotVariant::GetBool( &value, true ) );
        REQUIRE( ParseLiteral( "null", value ) );
        ASSERT_TRUE( CGodotVariant::IsType( &value, "null" ) );
        REQUIRE( ParseLiteral( "\"a\\nb\\\"c\\\\d\\u00e9\"", value ) );
        ASSERT_TRUE( CGodotVariant::IsType( &value, "String" ) && CORE::CString( "a\nb\"c\\d\xC3\xA9" ) == CGodotVariant::GetString( &value ) );
        REQUIRE( ParseLiteral( "&\"hello\"", value ) );
        ASSERT_TRUE( CGodotVariant::IsType( &value, "StringName" ) && "hello" == CGodotVariant::GetString( &value ) );
        REQUIRE( ParseLiteral( "NodePath(\"Armature/Skeleton3D:bone\")", value ) );
        ASSERT_TRUE( CGodotVariant::IsType( &value, "NodePath" ) && "Armature/Skeleton3D:bone" == CGodotVariant::GetString( &value ) );
        REQUIRE( ParseLiteral( "ExtResource(\"3_x\")", value ) );
        CORE::CString kind;
        CORE::CString id;
        ASSERT_TRUE( CGodotVariant::GetReference( &value, kind, id ) && "ExtResource" == kind && "3_x" == id );
        REQUIRE( ParseLiteral( "SubResource(\"Mat_1\")", value ) );
        ASSERT_TRUE( CGodotVariant::GetReference( &value, kind, id ) && "SubResource" == kind && "Mat_1" == id );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Reader: the geometric literals (Vector2/3/4 and the i variants, Quaternion, Color, Plane, AABB, Rect2, Basis, Transform2D, Transform3D, Projection)" )
    try
    {
        struct SCase { const char* text; const char* type; UInt32 count; };
        const SCase cases[] = {
            { "Vector2(1, 2)", "Vector2", 2 }, { "Vector3(1, 2, 3)", "Vector3", 3 }, { "Vector4(1, 2, 3, 4)", "Vector4", 4 },
            { "Vector2i(3, 4)", "Vector2i", 2 }, { "Vector3i(1, 2, 3)", "Vector3i", 3 }, { "Vector4i(1, 2, 3, 4)", "Vector4i", 4 },
            { "Color(0.5, 0.25, 1, 0.5)", "Color", 4 }, { "Quaternion(0, 0, 0, 1)", "Quaternion", 4 }, { "Plane(0, 1, 0, 2)", "Plane", 4 },
            { "AABB(-1, -1, -1, 2, 2, 2)", "AABB", 6 }, { "Rect2(0, 0, 4, 3)", "Rect2", 4 }, { "Rect2i(0, 0, 4, 3)", "Rect2i", 4 },
            { "Transform2D(1, 0, 0, 1, 5, 6)", "Transform2D", 6 }, { "Basis(1, 0, 0, 0, 1, 0, 0, 0, 1)", "Basis", 9 },
            { "Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 2, 3)", "Transform3D", 12 },
            { "Projection(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1)", "Projection", 16 } };
        for ( UInt32 c=0; c<sizeof( cases ) / sizeof( cases[ 0 ] ); ++c )
        {
            CORE::CDataNode value;
            ASSERT_TRUE( ParseLiteral( cases[ c ].text, value ) );
            CGodotVariant::TNumberVector numbers;
            ASSERT_TRUE( CGodotVariant::IsType( &value, cases[ c ].type ) && CGodotVariant::GetNumbers( &value, numbers ) && cases[ c ].count == numbers.size() );
        }
        CORE::CDataNode value;
        REQUIRE( ParseLiteral( "Color(0.5, 0.25, 1, 0.5)", value ) );
        Float32 colour[ 4 ];
        ASSERT_TRUE( CGodotVariant::GetColour( &value, colour ) && Near( colour[ 1 ], 0.25f ) && Near( colour[ 3 ], 0.5f ) );
        REQUIRE( ParseLiteral( "Color(0.5, 0.25, 1)", value ) );
        ASSERT_TRUE( CGodotVariant::GetColour( &value, colour ) && Near( colour[ 3 ], 1.0f ) );
        REQUIRE( ParseLiteral( "Vector3(inf, -1, 2.5)", value ) );
        Float32 vector3[ 3 ];
        ASSERT_TRUE( CGodotVariant::GetVector3( &value, vector3 ) && Near( vector3[ 2 ], 2.5f ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Reader: arrays, dictionaries, typed containers, objects and nesting" )
    try
    {
        CORE::CDataNode value;
        REQUIRE( ParseLiteral( "[1, \"two\", [3, 4], {\"k\": 5}, ]", value ) );
        ASSERT_TRUE( CGodotVariant::IsType( &value, "Array" ) && 4 == value.GetNrOfDirectChildNodes() );
        const CORE::CDataNode* first = value.GetFirstChild();
        ASSERT_TRUE( 1 == CGodotVariant::GetInteger( first, 0 ) && "two" == CGodotVariant::GetString( first->GetNext() ) );
        const CORE::CDataNode* nested = first->GetNext()->GetNext();
        ASSERT_TRUE( CGodotVariant::IsType( nested, "Array" ) && 4 == CGodotVariant::GetInteger( nested->GetLastChild(), 0 ) );
        const CORE::CDataNode* dictionary = nested->GetNext();
        ASSERT_TRUE( CGodotVariant::IsType( dictionary, "Dictionary" ) && 5 == CGodotVariant::GetInteger( CGodotVariant::FindMember( dictionary, "k" ), 0 ) );

        REQUIRE( ParseLiteral( "{\n\"a\": 1, ; a comment\n\"b\": [\n1,\n2\n],\n2: \"two\", \"c\": { \"d\": null }\n}", value ) );
        ASSERT_TRUE( 1 == CGodotVariant::GetInteger( CGodotVariant::FindMember( &value, "a" ), 0 ) && 2 == CGodotVariant::FindMember( &value, "b" )->GetNrOfDirectChildNodes() );
        ASSERT_TRUE( "two" == CGodotVariant::GetString( CGodotVariant::FindMember( &value, "2" ) ) && "int" == CGodotVariant::FindMember( &value, "2" )->GetAttributeValue( "keyType" ).AsString() );
        ASSERT_TRUE( CGodotVariant::IsType( CGodotVariant::FindMember( CGodotVariant::FindMember( &value, "c" ), "d" ), "null" ) );

        REQUIRE( ParseLiteral( "Array[int]([1, 2, 3])", value ) );
        ASSERT_TRUE( CGodotVariant::IsType( &value, "Array" ) && "int" == value.GetAttributeValue( "elementType" ).AsString() && 3 == value.GetNrOfDirectChildNodes() );
        REQUIRE( ParseLiteral( "Array[ExtResource(\"1_x\")]([null])", value ) );
        ASSERT_TRUE( "ExtResource(\"1_x\")" == value.GetAttributeValue( "elementType" ).AsString() && 1 == value.GetNrOfDirectChildNodes() );
        REQUIRE( ParseLiteral( "Dictionary[String, int]({\"a\": 1})", value ) );
        ASSERT_TRUE( CGodotVariant::IsType( &value, "Dictionary" ) && 1 == CGodotVariant::GetInteger( CGodotVariant::FindMember( &value, "a" ), 0 ) );

        REQUIRE( ParseLiteral( "Object(InputEventKey,\"resource_local_to_scene\":false,\"keycode\":4194309,\"name\":\"x\")", value ) );
        ASSERT_TRUE( CGodotVariant::IsType( &value, "Object" ) && "InputEventKey" == value.GetAttributeValue( "class" ).AsString() );
        ASSERT_TRUE( 4194309 == CGodotVariant::GetInteger( CGodotVariant::FindMember( &value, "keycode" ), 0 ) && !CGodotVariant::GetBool( CGodotVariant::FindMember( &value, "resource_local_to_scene" ), true ) );

        // the plain tree for the sidecar
        REQUIRE( ParseLiteral( "{\"size\": Vector3(1, 2, 3), \"list\": [1, 2], \"ref\": SubResource(\"m\"), \"name\": \"x\"}", value ) );
        CORE::CDataNode plain( "plain", GUCEF_DATATYPE_OBJECT );
        CGodotVariant::AddPlain( plain, "entry", value );
        const CORE::CDataNode* entry = plain.FindChild( "entry" );
        REQUIRE( GUCEF_NULL != entry );
        ASSERT_TRUE( "x" == entry->GetAttributeValue( "name" ).AsString() && "SubResource(m)" == entry->GetAttributeValue( "ref" ).AsString() );
        CORE::CDataNode::TVariantVector sizeValues;
        ASSERT_TRUE( entry->GetValuesOfChildByName( "size", sizeValues ) && 3 == sizeValues.size() && Near( sizeValues[ 1 ].AsFloat32(), 2.0f ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Reader: the packed arrays (bytes as numbers and as base64, int32, int64, float32, float64, vectors, colours, strings, empty)" )
    try
    {
        CORE::CDataNode value;
        CGodotVariant::TByteVector bytes;
        REQUIRE( ParseLiteral( "PackedByteArray(1, 2, 255)", value ) );
        ASSERT_TRUE( CGodotVariant::GetBytes( &value, bytes ) && 3 == bytes.size() && 255 == bytes[ 2 ] && 3 == value.GetAttributeValue( "count" ).AsUInt32() );
        REQUIRE( ParseLiteral( "PackedByteArray(\"AQL/\")", value ) );
        ASSERT_TRUE( CGodotVariant::GetBytes( &value, bytes ) && 3 == bytes.size() && 1 == bytes[ 0 ] && 2 == bytes[ 1 ] && 255 == bytes[ 2 ] );
        REQUIRE( ParseLiteral( "PackedByteArray()", value ) );
        ASSERT_TRUE( CGodotVariant::GetBytes( &value, bytes ) && bytes.empty() );

        const double ints[ 3 ] = { -1, 0, 70000 };
        REQUIRE( ParseLiteral( "PackedInt32Array(-1, 0, 70000)", value ) );
        ASSERT_TRUE( NumbersAre( value, ints, 3 ) && "i32" == value.GetAttributeValue( "elem" ).AsString() );
        const double wide[ 2 ] = { 5000000000.0, -2.0 };
        REQUIRE( ParseLiteral( "PackedInt64Array(5000000000, -2)", value ) );
        ASSERT_TRUE( NumbersAre( value, wide, 2 ) );
        const double floats[ 2 ] = { 0.5, 1.5 };
        REQUIRE( ParseLiteral( "PackedFloat32Array(0.5, 1.5)", value ) );
        ASSERT_TRUE( NumbersAre( value, floats, 2 ) && "f32" == value.GetAttributeValue( "elem" ).AsString() );
        REQUIRE( ParseLiteral( "PackedFloat64Array(0.1, 2)", value ) );
        CGodotVariant::TNumberVector numbers;
        ASSERT_TRUE( CGodotVariant::GetNumbers( &value, numbers ) && 2 == numbers.size() && fabs( numbers[ 0 ] - 0.1 ) < 1e-15 );
        const double vectors[ 6 ] = { 1, 2, 3, 4, 5, 6 };
        REQUIRE( ParseLiteral( "PackedVector3Array(1, 2, 3, 4, 5, 6)", value ) );
        ASSERT_TRUE( NumbersAre( value, vectors, 6 ) && 2 == value.GetAttributeValue( "count" ).AsUInt32() && 3 == value.GetAttributeValue( "components" ).AsUInt32() );
        REQUIRE( ParseLiteral( "PackedVector2Array(1, 2, 3, 4)", value ) );
        ASSERT_TRUE( 2 == value.GetAttributeValue( "count" ).AsUInt32() );
        REQUIRE( ParseLiteral( "PackedColorArray(1, 0, 0, 1, 0, 1, 0, 0.5)", value ) );
        ASSERT_TRUE( 2 == value.GetAttributeValue( "count" ).AsUInt32() && 4 == value.GetAttributeValue( "components" ).AsUInt32() );
        CORE::CString::StringVector strings;
        REQUIRE( ParseLiteral( "PackedStringArray(\"a\", \"b c\")", value ) );
        ASSERT_TRUE( CGodotVariant::GetStrings( &value, strings ) && 2 == strings.size() && "b c" == strings[ 1 ] );

        // a large array
        CORE::CString big = "PackedByteArray(";
        for ( UInt32 i=0; i<100000; ++i )
            big += ( 0 == i ? "" : "," ) + CORE::UInt32ToString( i % 256 );
        big += ")";
        REQUIRE( ParseLiteral( big, value ) );
        ASSERT_TRUE( CGodotVariant::GetBytes( &value, bytes ) && 100000 == bytes.size() && 159 == bytes[ 99999 ] && 0 == bytes[ 256 ] );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Reader: comments, line breaks inside values, Windows line ends, a byte order mark and lines before the first section" )
    try
    {
        const CORE::CString text =
            "\xEF\xBB\xBF; file comment\r\nconfig_version=5\r\n\r\n[application]\r\n\r\nconfig/name=\"Test ; not a comment\"\r\n"
            "list=[\r\n    1,\r\n    ; between the elements\r\n    2\r\n]\r\n"
            "[input]\r\nui_accept={\r\n\"deadzone\": 0.5,\r\n\"events\": [Object(InputEventKey,\"keycode\":32)]\r\n}\r\n";
        CORE::CDataNode root;
        CGodotTextReader reader;
        REQUIRE( reader.Parse( text, root ) );
        ASSERT_TRUE( 3 == root.GetNrOfDirectChildNodes() && root.GetAttributeValue( "kind" ).AsString().IsNULLOrEmpty() );
        const CORE::CDataNode* first = root.GetFirstChild();
        ASSERT_TRUE( first->GetAttributeValue( "tag" ).AsString().IsNULLOrEmpty() && 5 == CGodotVariant::GetInteger( CGodotVariant::FindProperty( *first, "config_version" ), 0 ) );
        const CORE::CDataNode* application = first->GetNext();
        ASSERT_TRUE( "Test ; not a comment" == CGodotVariant::GetString( CGodotVariant::FindProperty( *application, "config/name" ) ) );
        ASSERT_TRUE( 2 == CGodotVariant::FindProperty( *application, "list" )->GetNrOfDirectChildNodes() );
        const CORE::CDataNode* input = application->GetNext();
        ASSERT_TRUE( GUCEF_NULL != CGodotVariant::FindMember( CGodotVariant::FindProperty( *input, "ui_accept" ), "events" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Reader: text that is not valid fails with the line of the problem" )
    try
    {
        CGodotTextReader reader;
        CORE::CDataNode root;
        ASSERT_FALSE( reader.Parse( "[gd_scene format=3]\n\n[node name=\"x]\n", root ) );
        ASSERT_TRUE( reader.GetProblem().HasSubstr( "line" ) >= 0 );
        ASSERT_FALSE( reader.Parse( "[node name=\"a\"]\nkey = [1, 2\n", root ) );
        ASSERT_FALSE( reader.Parse( "[node name=\"a\"]\nkey = @\n", root ) );
        ASSERT_FALSE( reader.Parse( "[node name=\"a\"]\nkey 5\n", root ) );
        ASSERT_FALSE( reader.Parse( "[node name=\"a\"\n", root ) );
        CORE::CDataNode value;
        ASSERT_FALSE( reader.ParseValue( "Vector3(1, 2", value ) );
        ASSERT_FALSE( reader.ParseValue( "{\"a\" 1}", value ) );
        ASSERT_FALSE( reader.ParseValue( "1 2", value ) );
        ASSERT_FALSE( reader.ParseFile( "work/does/not/exist.tscn", root ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Variants: transforms (rotation, uniform scale, mirror, shear), the sRGB conversion" )
    try
    {
        CORE::CDataNode value;
        MATH::TFloat32Transform3D transform;
        REQUIRE( ParseLiteral( GodotTransformText( 90.0, 1.0, 1.0, 2.0, 3.0 ), value ) );
        ASSERT_TRUE( CGodotVariant::GetTransform( &value, transform ) );
        ASSERT_TRUE( Near( transform.position.x, 1.0f ) && Near( transform.position.y, 2.0f ) && Near( transform.position.z, 3.0f ) );
        ASSERT_TRUE( Near( transform.rotation.y, 0.70710678f, 1e-4f ) && Near( transform.rotation.w, 0.70710678f, 1e-4f ) && Near( transform.rotation.x, 0.0f, 1e-4f ) );
        ASSERT_TRUE( Near( transform.scale.x, 1.0f, 1e-4f ) && Near( transform.scale.y, 1.0f, 1e-4f ) );
        REQUIRE( ParseLiteral( GodotTransformText( 0.0, 2.5, 0.0, 0.0, 0.0 ), value ) );
        ASSERT_TRUE( CGodotVariant::GetTransform( &value, transform ) && Near( transform.scale.x, 2.5f, 1e-4f ) && Near( transform.scale.z, 2.5f, 1e-4f ) );
        REQUIRE( ParseLiteral( "Transform3D(-1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0)", value ) );
        ASSERT_TRUE( CGodotVariant::GetTransform( &value, transform ) && Near( transform.scale.x, -1.0f, 1e-4f ) && Near( transform.scale.y, 1.0f, 1e-4f ) );
        REQUIRE( ParseLiteral( "Basis(2, 0, 0, 0, 3, 0, 0, 0, 4)", value ) );
        ASSERT_TRUE( CGodotVariant::GetTransform( &value, transform ) && Near( transform.scale.y, 3.0f, 1e-4f ) && Near( transform.position.x, 0.0f ) );
        REQUIRE( ParseLiteral( "Transform3D(1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0)", value ) );
        ASSERT_FALSE( CGodotVariant::GetTransform( &value, transform ) );
        ASSERT_FALSE( CGodotVariant::GetTransform( GUCEF_NULL, transform ) );
        ASSERT_TRUE( Near( CGodotVariant::SrgbToLinear( 0.5f ), 0.21404114f, 1e-5f ) && Near( CGodotVariant::SrgbToLinear( 1.0f ), 1.0f ) && Near( CGodotVariant::SrgbToLinear( 0.0f ), 0.0f ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Import settings: the scale, the animations, the loops, the external material, the skipped nodes and the uid of a .import file" )
    try
    {
        MakeWorkFolder( "imp" );
        ASSERT_TRUE( WriteWorkText( "imp/m.glb", "x" ) );
        ASSERT_TRUE( WriteWorkText( "imp/plain.glb", "x" ) );
        ASSERT_TRUE( WriteWorkText( "imp/m.glb.import",
            "[remap]\n\nimporter=\"scene\"\nimporter_version=1\ntype=\"PackedScene\"\nuid=\"uid://c0abc\"\npath=\"res://.godot/imported/m.glb-xyz.scn\"\n\n"
            "[deps]\n\nsource_file=\"res://m.glb\"\ndest_files=[\"res://.godot/imported/m.glb-xyz.scn\"]\n\n"
            "[params]\n\nnodes/root_type=\"\"\nnodes/root_name=\"Model\"\nnodes/apply_root_scale=true\nnodes/root_scale=2.5\nmeshes/generate_lods=false\nanimation/import=false\n"
            "_subresources={\n\"animations\": {\n\"Walk\": {\n\"settings/loop_mode\": 1\n},\n\"Idle\": {\n\"settings/loop_mode\": 0\n}\n},\n"
            "\"materials\": {\n\"Painted\": {\n\"use_external/enabled\": true,\n\"use_external/path\": \"uid://c9\",\n\"use_external/fallback_path\": \"res://mats/paint.tres\"\n}\n},\n"
            "\"nodes\": {\n\"PATH:Crate/Lid\": {\n\"import/skip_import\": true\n}\n}\n}\n" ) );
        CAssetPackContext context;
        REQUIRE( OpenPackContext( context, "work/imp", "work/imp_inter" ) );
        CGodotImportSettings settings;
        REQUIRE( settings.Load( context, "m.glb" ) );
        ASSERT_TRUE( settings.IsPresent() && "scene" == settings.GetImporter() && "uid://c0abc" == settings.GetUid() );
        ASSERT_TRUE( Near( (Float32) settings.GetRootScale(), 2.5f ) && "Model" == settings.GetRootName() );
        ASSERT_FALSE( settings.GetImportAnimations() || settings.GetGenerateLods() );
        bool loop = false;
        ASSERT_TRUE( settings.GetAnimationLoop( "Walk", loop ) && loop );
        ASSERT_TRUE( settings.GetAnimationLoop( "Idle", loop ) && !loop );
        ASSERT_FALSE( settings.GetAnimationLoop( "Run", loop ) );
        CORE::CString path;
        CORE::CString uid;
        ASSERT_TRUE( settings.GetExternalMaterial( "Painted", path, uid ) && "res://mats/paint.tres" == path && "uid://c9" == uid );
        ASSERT_FALSE( settings.GetExternalMaterial( "Other", path, uid ) );
        ASSERT_TRUE( settings.IsNodeSkipped( "Crate/Lid" ) && !settings.IsNodeSkipped( "Crate" ) );
        CORE::CDataNode plain( "settings", GUCEF_DATATYPE_OBJECT );
        settings.AddPlain( plain );
        ASSERT_TRUE( Near( plain.GetAttributeValue( "nodes/root_scale" ).AsFloat32(), 2.5f ) );

        // the defaults without a file
        CGodotImportSettings none;
        ASSERT_FALSE( none.Load( context, "plain.glb" ) );
        ASSERT_TRUE( !none.IsPresent() && Near( (Float32) none.GetRootScale(), 1.0f ) && none.GetImportAnimations() && none.GetGenerateLods() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Paths: res:// against the project of the file, several projects in a pack, uid:// from .tscn and .import files, the tail and the letter case" )
    try
    {
        MakeWorkFolder( "paths" );
        ASSERT_TRUE( WriteGodotProject( "paths/p1" ) && WriteGodotProject( "paths/p2" ) );
        ASSERT_TRUE( WriteWorkText( "paths/p1/art/t.png", "1" ) && WriteWorkText( "paths/p2/art/t.png", "2" ) && WriteWorkText( "paths/p2/art/Only.png", "3" ) );
        ASSERT_TRUE( WriteWorkText( "paths/p1/scenes/a.tscn", "[gd_scene load_steps=2 format=3 uid=\"uid://scene_a\"]\n\n[node name=\"A\" type=\"Node3D\"]\n" ) );
        ASSERT_TRUE( WriteWorkText( "paths/p2/scenes/b.tscn", "[gd_scene format=3]\n" ) );
        ASSERT_TRUE( WriteWorkText( "paths/p1/models/m.glb", "x" ) );
        ASSERT_TRUE( WriteWorkText( "paths/p1/models/m.glb.import", "[remap]\n\nimporter=\"scene\"\nuid=\"uid://model_m\"\n" ) );
        ASSERT_TRUE( WriteWorkText( "paths/loose/stuff/Deep/tex.png", "4" ) );
        CAssetPackContext context;
        REQUIRE( OpenPackContext( context, "work/paths", "work/paths_inter" ) );
        ASSERT_TRUE( CGodotPaths::IndexUids( context ) >= 2 );

        CORE::CString found;
        ASSERT_TRUE( CGodotPaths::Resolve( context, "p1/scenes/a.tscn", "res://art/t.png", "", found ) && "p1/art/t.png" == found );
        ASSERT_TRUE( CGodotPaths::Resolve( context, "p2/scenes/b.tscn", "res://art/t.png", "", found ) && "p2/art/t.png" == found );
        ASSERT_TRUE( CGodotPaths::Resolve( context, "p2/scenes/b.tscn", "res://ART/only.PNG", "", found ) && "p2/art/Only.png" == found );
        ASSERT_TRUE( CGodotPaths::Resolve( context, "p2/scenes/b.tscn", "res://missing.png", "uid://scene_a", found ) && "p1/scenes/a.tscn" == found );
        ASSERT_TRUE( CGodotPaths::Resolve( context, "p2/scenes/b.tscn", "res://x", "uid://model_m", found ) && "p1/models/m.glb" == found );
        ASSERT_TRUE( CGodotPaths::Resolve( context, "loose/other.tscn", "res://Deep/tex.png", "", found ) && "loose/stuff/Deep/tex.png" == found );
        ASSERT_FALSE( CGodotPaths::Resolve( context, "p1/scenes/a.tscn", "res://nothing/here.png", "", found ) );
        CORE::CString root;
        ASSERT_TRUE( CGodotPaths::FindProjectRoot( context, "p2/scenes/b.tscn", root ) && "p2" == root );
        ASSERT_FALSE( CGodotPaths::FindProjectRoot( context, "loose/other.tscn", root ) );
        ASSERT_TRUE( "art/t.png" == CGodotPaths::StripScheme( "res://art/t.png" ) && "uid://scene_a" == CGodotPaths::ReadHeaderUid( context.ToVfsPath( "p1/scenes/a.tscn" ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
