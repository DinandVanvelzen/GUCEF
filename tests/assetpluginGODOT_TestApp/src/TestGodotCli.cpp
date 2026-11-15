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

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#include "TestGodotProject.h"
#include "TestGodotCli.h"

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

/** A command line: the texts and the char pointers a program gets */
class CCommandLine
{
    public:

    CCommandLine( void )
        : m_texts()
        , m_pointers()
    {
        Add( "AssetConverter" );
    }

    CCommandLine& Add( const CORE::CString& argument )
    {
        m_texts.push_back( argument );
        return *this;
    }

    int GetCount( void ) const
    {
        return (int) m_texts.size();
    }

    char** GetArguments( void )
    {
        m_pointers.clear();
        for ( size_t i=0; i<m_texts.size(); ++i )
            m_pointers.push_back( const_cast< char* >( m_texts[ i ].C_String() ) );
        m_pointers.push_back( GUCEF_NULL );
        return &m_pointers[ 0 ];
    }

    private:

    CORE::CString::StringVector m_texts;
    GUCEF::vector< char* > m_pointers;
};

Int32
RunCommandLine( CCommandLine& commandLine, CConversionReport* report = GUCEF_NULL )
{
    CAssetConverterApp app( commandLine.GetCount(), commandLine.GetArguments() );
    if ( !app.ParseCommandLine() )
        return -1;
    const Int32 exitCode = app.Run();
    if ( GUCEF_NULL != report )
        *report = app.GetReport();
    app.Unmount();
    return exitCode;
}

bool
LoadJsonFile( const CORE::CString& diskPath, CORE::CDataNode& node )
{
    CORE::CString text;
    return CORE::LoadTextFileAsString( diskPath, text ) && CCanonicalJson::Parse( text, node );
}

/** A Godot project with a level, a nested scene, a model file and textures, below the work folder */
bool
WriteProject( const CORE::CString& folder )
{
    const CORE::CString project = folder + "/project";
    MakeWorkFolder( folder );
    if ( !WriteGodotProject( project ) || !WriteGltfModel( "work/" + project + "/models", "crate" ) )
        return false;
    if ( !WriteTestPng( "work/" + project + "/textures/albedo.png", 8, 8, 200, 30, 30, 255 ) || !WriteTestPng( "work/" + project + "/textures/orm.png", 8, 8, 255, 128, 64, 255 ) ||
         !WriteTestPng( "work/" + project + "/textures/emit.png", 8, 8, 255, 200, 0, 255 ) )
        return false;
    if ( !WriteLicence( folder + "/licences", "rec" ) )
        return false;
    if ( !WriteWorkText( project + "/scenes/wall.tscn",
        "[gd_scene load_steps=2 format=3 uid=\"uid://wall_scene\"]\n\n[sub_resource type=\"BoxMesh\" id=\"b\"]\nsize = Vector3(4, 2, 0.5)\n\n"
        "[node name=\"Wall\" type=\"Node3D\"]\n\n[node name=\"Slab\" type=\"MeshInstance3D\" parent=\".\"]\nmesh = SubResource(\"b\")\n" ) )
        return false;
    return WriteWorkText( project + "/scenes/level.tscn",
        "[gd_scene load_steps=9 format=3 uid=\"uid://level_scene\"]\n\n"
        "[ext_resource type=\"PackedScene\" path=\"res://models/crate.gltf\" id=\"1_crate\"]\n"
        "[ext_resource type=\"PackedScene\" uid=\"uid://wall_scene\" path=\"res://old/wall.tscn\" id=\"2_wall\"]\n"
        "[ext_resource type=\"Texture2D\" path=\"res://textures/albedo.png\" id=\"3_a\"]\n"
        "[ext_resource type=\"Texture2D\" path=\"res://textures/orm.png\" id=\"4_orm\"]\n"
        "[ext_resource type=\"Texture2D\" path=\"res://textures/emit.png\" id=\"5_e\"]\n\n"
        "[sub_resource type=\"StandardMaterial3D\" id=\"Paint\"]\ntransparency = 3\nalbedo_texture = ExtResource(\"3_a\")\nuv1_scale = Vector3(2, 2, 1)\nuv1_offset = Vector3(0.5, 0, 0)\n"
        "emission_enabled = true\nemission = Color(1, 0.5, 0, 1)\nemission_energy_multiplier = 2.0\nemission_texture = ExtResource(\"5_e\")\ndetail_enabled = true\n\n"
        "[sub_resource type=\"ORMMaterial3D\" id=\"Packed\"]\norm_texture = ExtResource(\"4_orm\")\nmetallic = 1.0\ncull_mode = 2\n\n"
        "[sub_resource type=\"BoxMesh\" id=\"Box\"]\nmaterial = SubResource(\"Paint\")\n\n"
        "[sub_resource type=\"SphereMesh\" id=\"Ball\"]\nmaterial = SubResource(\"Packed\")\nradial_segments = 8\nrings = 4\n\n"
        "[sub_resource type=\"BoxShape3D\" id=\"Shape\"]\nsize = Vector3(10, 1, 10)\n\n"
        "[node name=\"Level\" type=\"Node3D\"]\n\n"
        "[node name=\"Floor\" type=\"StaticBody3D\" parent=\".\"]\n\n"
        "[node name=\"FloorShape\" type=\"CollisionShape3D\" parent=\"Floor\"]\nshape = SubResource(\"Shape\")\n\n"
        "[node name=\"Sun\" type=\"DirectionalLight3D\" parent=\".\"]\nlight_energy = 1.5\n\n"
        "[node name=\"Box\" type=\"MeshInstance3D\" parent=\".\"]\nmesh = SubResource(\"Box\")\ntransform = " + GodotTransformText( 45.0, 1.0, 0.0, 1.0, 0.0 ) + "\n\n"
        "[node name=\"Debug-noimp\" type=\"MeshInstance3D\" parent=\".\"]\nmesh = SubResource(\"Box\")\n\n"
        "[node name=\"Ball\" type=\"MeshInstance3D\" parent=\".\"]\nmesh = SubResource(\"Ball\")\nposition = Vector3(3, 1, 0)\n\n"
        "[node name=\"Crate\" parent=\".\" instance=ExtResource(\"1_crate\")]\nposition = Vector3(-4, 0, 0)\n\n"
        "[node name=\"Wall\" parent=\".\" instance=ExtResource(\"2_wall\")]\nposition = Vector3(0, 0, -6)\n" );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGodotCliTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetpluginGODOT.Cli" );

    GUCEF_TESTFW_TESTCASE( "CLI: a Godot project folder converts to the intermediate glTF with textures and the sidecar; again from the cache" )
    try
    {
        REQUIRE( WriteProject( "cli1" ) );
        ASSERT_TRUE( WriteWorkText( "cli1/godot.assetpack.json",
            "{\n  \"id\": \"godotpack\",\n  \"title\": \"Godot test pack\",\n  \"licenceRecord\": \"rec\",\n  \"licenceFolder\": \"/pack/licences\",\n  \"source\": \"/proj\",\n"
            "  \"importer\": \"godot\",\n  \"intermediate\": \"/out/intermediate\",\n  \"output\": \"/out\"\n}\n" ) );
        const CORE::CString outFolder = WorkDiskPath( "cli1/out" );
        CCommandLine commandLine;
        commandLine.Add( "--mount" ).Add( "proj=" + WorkDiskPath( "cli1/project" ) ).Add( "--manifest" ).Add( WorkDiskPath( "cli1/godot.assetpack.json" ) )
                   .Add( "--out" ).Add( outFolder ).Add( "--report" ).Add( WorkDiskPath( "cli1/report.json" ) );
        CConversionReport report;
        ASSERT_TRUE( CAssetConverterApp::ExitSuccess == RunCommandLine( commandLine, &report ) );
        ASSERT_TRUE( 0 == report.GetAssetCount( ASSETSTATUS_FAILED ) && 2 == report.GetAssetCount( ASSETSTATUS_APPROXIMATED ) + report.GetAssetCount( ASSETSTATUS_CONVERTED ) );
        ASSERT_TRUE( CORE::FileExists( WorkDiskPath( "cli1/report.json" ) ) );

        const CORE::CString intermediate = CORE::CombinePath( outFolder, "intermediate" );
        const CORE::CString gltfPath = CORE::CombinePath( CORE::CombinePath( intermediate, "scenes" ), "level.gltf" );
        REQUIRE( CORE::FileExists( gltfPath ) && CORE::FileExists( CORE::CombinePath( CORE::CombinePath( intermediate, "scenes" ), "level.conversion.json" ) ) );
        CORE::CString problem;
        ASSERT_TRUE( ValidateWithCgltf( gltfPath, problem ) );
        CORE::CString gltf;
        ASSERT_TRUE( CORE::LoadTextFileAsString( gltfPath, gltf ) );
        ASSERT_TRUE( gltf.HasSubstr( "KHR_texture_transform" ) >= 0 && gltf.HasSubstr( "OMI_physics_shape" ) >= 0 && gltf.HasSubstr( "KHR_lights_punctual" ) >= 0 && gltf.HasSubstr( "KHR_materials_emissive_strength" ) >= 0 );
        ASSERT_TRUE( gltf.HasSubstr( "\"alphaMode\": \"MASK\"" ) >= 0 && gltf.HasSubstr( "\"doubleSided\": true" ) >= 0 && gltf.HasSubstr( "\"name\": \"Crate\"" ) >= 0 && gltf.HasSubstr( "\"name\": \"Slab\"" ) >= 0 );

        // the name hint of Godot: a node marked as not imported is left out
        ASSERT_TRUE( gltf.HasSubstr( "Debug-noimp" ) < 0 && gltf.HasSubstr( "\"name\": \"Ball\"" ) >= 0 );

        // the pictures are baked into the intermediate
        ASSERT_TRUE( gltf.HasSubstr( "textures/" ) >= 0 );

        CORE::CDataNode sidecar;
        ASSERT_TRUE( LoadJsonFile( CORE::CombinePath( CORE::CombinePath( intermediate, "scenes" ), "level.conversion.json" ), sidecar ) );
        const CORE::CDataNode* provenance = sidecar.FindChild( "provenance" );
        REQUIRE( GUCEF_NULL != provenance );
        ASSERT_TRUE( "godot" == provenance->GetAttributeValueOrChildValueByName( "importer" ).AsString() && "tscn" == provenance->GetAttributeValueOrChildValueByName( "sourceFormat" ).AsString() );
        ASSERT_TRUE( "godot" == provenance->GetAttributeValueOrChildValueByName( "sourceEngine" ).AsString() && "uid://level_scene" == provenance->GetAttributeValueOrChildValueByName( "sourceId" ).AsString() );
        const CORE::CDataNode* materials = sidecar.FindChild( "materials" );
        REQUIRE( GUCEF_NULL != materials && materials->GetNrOfDirectChildNodes() >= 3 );
        bool paintFound = false;
        for ( const CORE::CDataNode* material = materials->GetFirstChild(); GUCEF_NULL != material; material = material->GetNext() )
        {
            const CORE::CDataNode* source = material->FindChild( "source" );
            if ( GUCEF_NULL != source && "Paint" == material->GetAttributeValueOrChildValueByName( "name" ).AsString() )
            {
                paintFound = true;
                ASSERT_TRUE( "StandardMaterial3D" == source->GetAttributeValueOrChildValueByName( "shader" ).AsString() || "StandardMaterial3D" == source->GetAttributeValueOrChildValueByName( "shaderName" ).AsString() );
            }
        }
        ASSERT_TRUE( paintFound );
        const CORE::CDataNode* prefabs = sidecar.FindChild( "prefabs" );
        REQUIRE( GUCEF_NULL != prefabs && 1 == prefabs->GetNrOfDirectChildNodes() );
        const CORE::CDataNode* instances = prefabs->GetFirstChild()->FindChild( "instances" );
        REQUIRE( GUCEF_NULL != instances && 1 == instances->GetNrOfDirectChildNodes() );
        ASSERT_TRUE( "scenes/wall.tscn" == instances->GetFirstChild()->GetAttributeValueOrChildValueByName( "sourcePrefab" ).AsString() );
        const CORE::CDataNode* lossy = sidecar.FindChild( "lossySteps" );
        ASSERT_TRUE( GUCEF_NULL != lossy && lossy->GetNrOfDirectChildNodes() >= 2 );
        const CORE::CDataNode* colliders = sidecar.FindChild( "colliders" );
        ASSERT_TRUE( GUCEF_NULL != colliders && colliders->GetNrOfDirectChildNodes() >= 1 );
        const CORE::CDataNode* summary = sidecar.FindChild( "summary" );
        ASSERT_TRUE( GUCEF_NULL != summary && 1 == summary->GetAttributeValueOrChildValueByName( "prefabs" ).AsInt32() );

        // again: nothing changed
        ASSERT_TRUE( CAssetConverterApp::ExitSuccess == RunCommandLine( commandLine, &report ) );
        ASSERT_TRUE( 2 == report.GetAssetCount( ASSETSTATUS_CACHED ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CLI: a pack with the importer auto converts the scenes through the Godot importer and the model files through the glTF importer" )
    try
    {
        REQUIRE( WriteProject( "cli2" ) );
        ASSERT_TRUE( WriteWorkText( "cli2/godot.assetpack.json",
            "{\n  \"id\": \"godotauto\",\n  \"title\": \"Godot test pack\",\n  \"licenceRecord\": \"rec\",\n  \"licenceFolder\": \"/pack/licences\",\n  \"source\": \"/proj\",\n"
            "  \"importer\": \"auto\",\n  \"intermediate\": \"/out/intermediate\",\n  \"output\": \"/out\"\n}\n" ) );
        const CORE::CString outFolder = WorkDiskPath( "cli2/out" );
        CCommandLine commandLine;
        commandLine.Add( "--mount" ).Add( "proj=" + WorkDiskPath( "cli2/project" ) ).Add( "--manifest" ).Add( WorkDiskPath( "cli2/godot.assetpack.json" ) ).Add( "--out" ).Add( outFolder );
        CConversionReport report;
        ASSERT_TRUE( CAssetConverterApp::ExitSuccess == RunCommandLine( commandLine, &report ) );
        ASSERT_TRUE( 0 == report.GetAssetCount( ASSETSTATUS_FAILED ) && 3 == report.GetAssetCount( ASSETSTATUS_APPROXIMATED ) + report.GetAssetCount( ASSETSTATUS_CONVERTED ) );
        const CORE::CString intermediate = CORE::CombinePath( outFolder, "intermediate" );
        ASSERT_TRUE( CORE::FileExists( CORE::CombinePath( CORE::CombinePath( intermediate, "scenes" ), "level.gltf" ) ) );
        ASSERT_TRUE( CORE::FileExists( CORE::CombinePath( CORE::CombinePath( intermediate, "scenes" ), "wall.gltf" ) ) );
        ASSERT_TRUE( CORE::FileExists( CORE::CombinePath( CORE::CombinePath( intermediate, "models" ), "crate.gltf" ) ) );
        CORE::CString problem;
        ASSERT_TRUE( ValidateWithCgltf( CORE::CombinePath( CORE::CombinePath( intermediate, "scenes" ), "wall.gltf" ), problem ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CLI: a scene of a project with a .import file for its model: the settings reach the intermediate" )
    try
    {
        REQUIRE( WriteProject( "cli3" ) );
        ASSERT_TRUE( WriteWorkText( "cli3/project/models/crate.gltf.import", "[remap]\n\nimporter=\"scene\"\n\n[params]\n\nnodes/root_scale=3.0\n" ) );
        ASSERT_TRUE( WriteWorkText( "cli3/godot.assetpack.json",
            "{\n  \"id\": \"godotscale\",\n  \"title\": \"Godot test pack\",\n  \"licenceRecord\": \"rec\",\n  \"licenceFolder\": \"/pack/licences\",\n  \"source\": \"/proj\",\n"
            "  \"importer\": \"godot\",\n  \"intermediate\": \"/out/intermediate\",\n  \"output\": \"/out\"\n}\n" ) );
        CCommandLine commandLine;
        commandLine.Add( "--mount" ).Add( "proj=" + WorkDiskPath( "cli3/project" ) ).Add( "--manifest" ).Add( WorkDiskPath( "cli3/godot.assetpack.json" ) ).Add( "--out" ).Add( WorkDiskPath( "cli3/out" ) );
        CConversionReport report;
        ASSERT_TRUE( CAssetConverterApp::ExitSuccess == RunCommandLine( commandLine, &report ) );
        CORE::CDataNode sidecar;
        ASSERT_TRUE( LoadJsonFile( CORE::CombinePath( CORE::CombinePath( CORE::CombinePath( WorkDiskPath( "cli3/out" ), "intermediate" ), "scenes" ), "level.conversion.json" ), sidecar ) );
        const CORE::CDataNode* extras = sidecar.FindChild( "extras" );
        REQUIRE( GUCEF_NULL != extras );
        const CORE::CDataNode* record = extras->FindChild( "godotImport" );
        REQUIRE( GUCEF_NULL != record && GUCEF_NULL != record->FindChild( "models/crate.gltf" ) );
        ASSERT_TRUE( Near( record->FindChild( "models/crate.gltf" )->GetAttributeValueOrChildValueByName( "nodes/root_scale" ).AsFloat32(), 3.0f ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
