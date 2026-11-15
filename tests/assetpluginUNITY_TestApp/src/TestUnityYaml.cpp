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

#include "assetpluginUNITY_CUnityYamlReader.h"
#include "assetpluginUNITY_CUnityReference.h"
#include "assetpluginUNITY_CUnityMeta.h"
#include "assetpluginUNITY_CUnityShaderInfo.h"
#include "assetpluginUNITY_CUnityMeshDecoder.h"
#include "assetpluginUNITY_CUnityBuiltins.h"
#include "assetpluginUNITY_CUnityPrefabReader.h"

#include "TestUnitySupport.h"
#include "TestUnityYaml.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETPLUGIN::UNITY;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformUnityYamlTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetpluginUNITY.Yaml" );

    GUCEF_TESTFW_TESTCASE( "YAML: documents with class and signed 64 bit file IDs, stripped documents, directives skipped" )
    try
    {
        const CORE::CString text = "%YAML 1.1\n%TAG !u! tag:unity3d.com,2011:\n--- !u!1 &100000\nGameObject:\n  m_Name: Root\n"
                                   "--- !u!4 &199967262512027732 stripped\nTransform:\n  m_PrefabInstance: {fileID: -7000000000000000001}\n"
                                   "--- !u!114 &-4229432104447468213\nMonoBehaviour:\n  m_Name: \n  version: 1\n"
                                   "--- !u!1002 &2100001\nEditorExtensionImpl:\n  serializedVersion: 6\n--- !u!1 &5\nGameObject:\n";
        CUnityYamlFile file;
        CORE::CString problem;
        ASSERT_TRUE( CUnityYamlReader::Parse( text, file, problem ) );
        ASSERT_TRUE( 5 == file.GetDocumentCount() );
        const CUnityYamlDocument* root = file.FindDocument( 100000 );
        ASSERT_TRUE( GUCEF_NULL != root && 1 == root->classId && "GameObject" == root->typeName && "Root" == CUnityYamlReader::GetString( root->body, "m_Name" ) && !root->stripped );
        const CUnityYamlDocument* stripped = file.FindDocument( (Int64) 199967262512027732LL );
        ASSERT_TRUE( GUCEF_NULL != stripped && stripped->stripped && 4 == stripped->classId && "Transform" == stripped->typeName );
        if ( GUCEF_NULL != stripped )
        {
            const CUnityReference instance = CUnityReference::Get( stripped->body, "m_PrefabInstance" );
            ASSERT_TRUE( (Int64) -7000000000000000001LL == instance.fileId && instance.IsLocal() );
        }
        const CUnityYamlDocument* behaviour = file.FindDocument( (Int64) -4229432104447468213LL );
        ASSERT_TRUE( GUCEF_NULL != behaviour && 114 == behaviour->classId && CUnityYamlReader::HasScalar( behaviour->body, "m_Name" ) && CUnityYamlReader::GetString( behaviour->body, "m_Name", "x" ).IsNULLOrEmpty() );
        const CUnityYamlDocument* empty = file.FindDocument( 5 );
        ASSERT_TRUE( GUCEF_NULL != empty && "GameObject" == empty->typeName && 0 == empty->body.GetAttCount() );
        ASSERT_TRUE( GUCEF_NULL != file.FindFirstOfClass( 1002 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "YAML: flow mappings over several lines, nested flow collections, empty flow collections" )
    try
    {
        const CORE::CString text = "--- !u!21 &2100000\nMaterial:\n  m_Shader: {fileID: 4800000, guid: 933532a4fcc9baf4fa0491de14d08ed7,\n    type: 3}\n"
                                   "  stringTagMap: {}\n  disabledShaderPasses: []\n  m_List: [1, 2, {a: b}]\n  m_Nested: {a: {b: c, d: [x, y]}, e: 'f, g'}\n";
        CUnityYamlFile file;
        CORE::CString problem;
        ASSERT_TRUE( CUnityYamlReader::Parse( text, file, problem ) );
        const CUnityYamlDocument* material = file.FindDocument( 2100000 );
        ASSERT_TRUE( GUCEF_NULL != material );
        if ( GUCEF_NULL != material )
        {
            const CUnityReference shader = CUnityReference::Get( material->body, "m_Shader" );
            ASSERT_TRUE( 4800000 == shader.fileId && "933532a4fcc9baf4fa0491de14d08ed7" == shader.guid && 3 == shader.type );
            const CORE::CDataNode* tags = material->body.FindChild( "stringTagMap" );
            ASSERT_TRUE( GUCEF_NULL != tags && GUCEF_DATATYPE_OBJECT == tags->GetNodeType() && !tags->HasChildren() && 0 == tags->GetAttCount() );
            const CORE::CDataNode* passes = material->body.FindChild( "disabledShaderPasses" );
            ASSERT_TRUE( GUCEF_NULL != passes && GUCEF_DATATYPE_ARRAY == passes->GetNodeType() && !passes->HasChildren() );
            CUnityYamlReader::TNodeList items;
            CUnityYamlReader::GetItems( material->body.FindChild( "m_List" ), items );
            ASSERT_TRUE( 3 == items.size() && "2" == CUnityYamlReader::GetItemText( *items[ 1 ] ) && "b" == CUnityYamlReader::GetString( *items[ 2 ], "a" ) );
            const CORE::CDataNode* nested = material->body.FindChild( "m_Nested" );
            ASSERT_TRUE( GUCEF_NULL != nested && GUCEF_NULL != nested->FindChild( "a" ) && "c" == CUnityYamlReader::GetString( *nested->FindChild( "a" ), "b" ) && "f, g" == CUnityYamlReader::GetString( *nested, "e" ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "YAML: numbers as Unity writes them: leading dot, negative zero, exponents, infinities, 64 bit limits" )
    try
    {
        Float64 value = 1.0;
        ASSERT_TRUE( CUnityYamlReader::ParseFloat( ".5", value ) && 0.5 == value );
        ASSERT_TRUE( CUnityYamlReader::ParseFloat( "-.25", value ) && -0.25 == value );
        ASSERT_TRUE( CUnityYamlReader::ParseFloat( "-0", value ) && 0.0 == value );
        ASSERT_TRUE( CUnityYamlReader::ParseFloat( ".00999999978", value ) && fabs( value - 0.01 ) < 1e-8 );
        ASSERT_TRUE( CUnityYamlReader::ParseFloat( "1e-05", value ) && fabs( value - 1e-5 ) < 1e-12 );
        ASSERT_TRUE( CUnityYamlReader::ParseFloat( "Infinity", value ) && value > 1e300 );
        ASSERT_TRUE( CUnityYamlReader::ParseFloat( "-Infinity", value ) && value < -1e300 );
        ASSERT_TRUE( CUnityYamlReader::ParseFloat( "NaN", value ) && value != value );
        ASSERT_FALSE( CUnityYamlReader::ParseFloat( "abc", value ) );
        ASSERT_FALSE( CUnityYamlReader::ParseFloat( "", value ) );
        Int64 id = 0;
        ASSERT_TRUE( CUnityYamlReader::ParseInt64( "9223372036854775807", id ) && GUCEF_INT64MAX == id );
        ASSERT_TRUE( CUnityYamlReader::ParseInt64( "-9223372036854775808", id ) && GUCEF_INT64MIN == id );
        ASSERT_TRUE( CUnityYamlReader::ParseInt64( "-6465566751694194690", id ) && (Int64) -6465566751694194690LL == id );
        ASSERT_FALSE( CUnityYamlReader::ParseInt64( "9223372036854775808", id ) );
        ASSERT_FALSE( CUnityYamlReader::ParseInt64( "12a", id ) );
        ASSERT_TRUE( CUnityYamlReader::ParseInt64( "-0", id ) && 0 == id );

        CORE::CDataNode mapping( GUCEF_DATATYPE_OBJECT );
        mapping.SetAttribute( "a", ".5" );
        mapping.SetAttribute( "b", "-0" );
        mapping.SetAttribute( "c", "1" );
        ASSERT_TRUE( Near( CUnityYamlReader::GetFloat( mapping, "a", 0.0f ), 0.5f ) && 0.0f == CUnityYamlReader::GetFloat( mapping, "b", 7.0f ) );
        ASSERT_TRUE( CUnityYamlReader::GetBool( mapping, "c", false ) && 3.0f == CUnityYamlReader::GetFloat( mapping, "missing", 3.0f ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "YAML: empty values, quoted scalars over several lines, plain scalars that wrap, block scalars" )
    try
    {
        const CORE::CString text = "ModelImporter:\n  motionNodeName: \n  m_EditorClassIdentifier:\n  rigImportWarnings: \"No animation data available\n    in this model, \\\"really\\\"\\n\n    done\"\n"
                                   "  single: 'it''s\n    folded'\n  m_ShaderKeywords: _ALPHATEST_ON _EMISSION _METALLICGLOSSMAP\n    _NORMALMAP\n"
                                   "  literal: |\n    line one\n    line two\n  after: 1\n";
        CUnityYamlFile file;
        CORE::CString problem;
        ASSERT_TRUE( CUnityYamlReader::Parse( text, file, problem ) );
        ASSERT_TRUE( 1 == file.GetDocumentCount() && !file.GetDocument( 0 ).HasHeader() );
        const CORE::CDataNode* importer = file.GetDocument( 0 ).body.FindChild( "ModelImporter" );
        ASSERT_TRUE( GUCEF_NULL != importer );
        if ( GUCEF_NULL != importer )
        {
            ASSERT_TRUE( CUnityYamlReader::HasScalar( *importer, "motionNodeName" ) && CUnityYamlReader::GetString( *importer, "motionNodeName", "x" ).IsNULLOrEmpty() );
            ASSERT_TRUE( CUnityYamlReader::HasScalar( *importer, "m_EditorClassIdentifier" ) );
            ASSERT_TRUE( "No animation data available in this model, \"really\"\n done" == CUnityYamlReader::GetString( *importer, "rigImportWarnings" ) );
            ASSERT_TRUE( "it's folded" == CUnityYamlReader::GetString( *importer, "single" ) );
            ASSERT_TRUE( "_ALPHATEST_ON _EMISSION _METALLICGLOSSMAP _NORMALMAP" == CUnityYamlReader::GetString( *importer, "m_ShaderKeywords" ) );
            ASSERT_TRUE( "line one\nline two\n" == CUnityYamlReader::GetString( *importer, "literal" ) );
            ASSERT_TRUE( 1 == CUnityYamlReader::GetInt32( *importer, "after", 0 ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "YAML: sequences at the indentation of their key, '- key: value' items, lists of single-key mappings" )
    try
    {
        const CORE::CString text = "--- !u!1 &1\nGameObject:\n  m_Component:\n  - component: {fileID: 4}\n  - 23: {fileID: 23}\n"
                                   "  m_SavedProperties:\n    m_TexEnvs:\n    - _MainTex:\n        m_Texture: {fileID: 2800000, guid: a0000000000000000000000000000001, type: 3}\n"
                                   "        m_Scale: {x: 2, y: 2}\n    - _BumpMap:\n        m_Texture: {fileID: 0}\n    m_Floats:\n    - _Cutoff: .5\n    - _Mode: 1\n"
                                   "  m_Nested:\n  - - a\n    - b\n  - c\n  after: yes\n";
        CUnityYamlFile file;
        CORE::CString problem;
        ASSERT_TRUE( CUnityYamlReader::Parse( text, file, problem ) );
        const CUnityYamlDocument* object = file.FindDocument( 1 );
        ASSERT_TRUE( GUCEF_NULL != object );
        if ( GUCEF_NULL != object )
        {
            CUnityYamlReader::TNodeList components;
            CUnityYamlReader::GetItems( object->body.FindChild( "m_Component" ), components );
            ASSERT_TRUE( 2 == components.size() );
            if ( 2 == components.size() )
            {
                ASSERT_TRUE( 4 == CUnityReference::Get( *components[ 0 ], "component" ).fileId && 23 == CUnityReference::Get( *components[ 1 ], "23" ).fileId );
            }
            const CORE::CDataNode* saved = object->body.FindChild( "m_SavedProperties" );
            ASSERT_TRUE( GUCEF_NULL != saved );
            if ( GUCEF_NULL != saved )
            {
                CORE::CString::StringMap scalars;
                GUCEF::vector< std::pair< CORE::CString, const CORE::CDataNode* > > nodes;
                CUnityYamlReader::GetKeyedEntries( saved->FindChild( "m_TexEnvs" ), scalars, nodes );
                ASSERT_TRUE( 2 == nodes.size() && "_MainTex" == nodes[ 0 ].first );
                if ( 2 == nodes.size() )
                {
                    Float32 scale[ 2 ] = { 1.0f, 1.0f };
                    ASSERT_TRUE( CUnityYamlReader::GetComponents( *nodes[ 0 ].second, "m_Scale", "xy", scale ) && 2.0f == scale[ 0 ] );
                    ASSERT_TRUE( CUnityReference::Get( *nodes[ 1 ].second, "m_Texture" ).IsNull() );
                }
                CUnityYamlReader::GetKeyedEntries( saved->FindChild( "m_Floats" ), scalars, nodes );
                ASSERT_TRUE( 2 == scalars.size() && ".5" == scalars[ "_Cutoff" ] );
            }
            CUnityYamlReader::TNodeList nested;
            CUnityYamlReader::GetItems( object->body.FindChild( "m_Nested" ), nested );
            ASSERT_TRUE( 2 == nested.size() && GUCEF_DATATYPE_ARRAY == nested[ 0 ]->GetNodeType() && 2 == nested[ 0 ]->GetNrOfDirectChildNodes() );
            ASSERT_TRUE( "yes" == CUnityYamlReader::GetString( object->body, "after" ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "YAML: a document that cannot be read is reported and the others are kept" )
    try
    {
        const CORE::CString text = "--- !u!1 &1\nGameObject:\n  m_Name: A\n--- !u!1 &2\nGameObject:\n  m_Shader: {fileID: 4\n--- !u!1 &3\nGameObject:\n  m_Name: C\n";
        CUnityYamlFile file;
        CORE::CString problem;
        ASSERT_FALSE( CUnityYamlReader::Parse( text, file, problem ) );
        ASSERT_FALSE( problem.IsNULLOrEmpty() );
        ASSERT_TRUE( 3 == file.GetDocumentCount() && GUCEF_NULL != file.FindDocument( 3 ) && "C" == CUnityYamlReader::GetString( file.FindDocument( 3 )->body, "m_Name" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Meta: fileIDToRecycleName by number with duplicate names, the internalIDToNameTable alias, the clips and the texture flags" )
    try
    {
        CUnityMeta meta;
        CORE::CString problem;
        ASSERT_TRUE( meta.Parse( "fileFormatVersion: 2\nguid: B0000000000000000000000000000001\nModelImporter:\n  serializedVersion: 18\n"
                                 "  fileIDToRecycleName:\n    100000: //RootNode\n    4300000: House\n    4300002: Window\n    4300004: Window\n    7400000: Open\n"
                                 "  materials:\n    importMaterials: 0\n    materialName: 3\n    materialSearch: 2\n"
                                 "  animations:\n    clipAnimations:\n    - serializedVersion: 16\n      name: Open\n      takeName: Take 001\n      firstFrame: 0\n"
                                 "      lastFrame: 30\n      loopTime: 1\n      events: []\n"
                                 "  meshes:\n    globalScale: 100\n    addColliders: 1\n    useFileUnits: 1\n    useFileScale: 1\n  importAnimation: 1\n  animationType: 3\n", problem ) );
        ASSERT_TRUE( "b0000000000000000000000000000001" == meta.guid && "ModelImporter" == meta.importerType && 18 == meta.importerVersion && meta.IsModel() );
        ASSERT_TRUE( 5 == meta.idTable.size() && "fileIDToRecycleName" == meta.idTableKey );
        CUnityMeta::SIdEntry entry;
        ASSERT_TRUE( meta.TryGetIdEntry( 4300004, entry ) && 43 == entry.classId && "Window" == entry.name );
        ASSERT_TRUE( 0 == meta.GetNameRank( 4300002 ) && 1 == meta.GetNameRank( 4300004 ) && 0 == meta.GetNameRank( 4300000 ) );
        ASSERT_TRUE( !meta.importMaterials && 3 == meta.materialName && 2 == meta.materialSearch && meta.addColliders && meta.importAnimation && 3 == meta.animationType );
        ASSERT_TRUE( 1 == meta.clips.size() && "Open" == meta.clips[ 0 ].name && "Take 001" == meta.clips[ 0 ].takeName && 30.0f == meta.clips[ 0 ].lastFrame && meta.clips[ 0 ].loopTime );
        GUCEF::vector< Int64 > clipIds;
        meta.GetIdsOfClass( 74, clipIds );
        ASSERT_TRUE( 1 == clipIds.size() && 7400000 == clipIds[ 0 ] );

        CUnityMeta later;
        ASSERT_TRUE( later.Parse( "fileFormatVersion: 2\nguid: b0000000000000000000000000000004\nModelImporter:\n  serializedVersion: 21\n"
                                  "  internalIDToNameTable:\n  - first:\n      43: -2345678901234567890\n    second: House\n  - first:\n      74: 1827226128182048838\n    second: Take 001\n"
                                  "  externalObjects:\n  - first:\n      type: UnityEngine:Material\n      assembly: UnityEngine.CoreModule\n      name: Mat\n"
                                  "    second: {fileID: 2100000, guid: c0000000000000000000000000000001, type: 2}\n"
                                  "  materials:\n    materialImportMode: 2\n  meshes:\n    globalScale: 1\n", problem ) );
        ASSERT_TRUE( "internalIDToNameTable" == later.idTableKey && 2 == later.idTable.size() );
        ASSERT_TRUE( later.TryGetIdEntry( (Int64) -2345678901234567890LL, entry ) && 43 == entry.classId && "House" == entry.name );
        ASSERT_TRUE( 1 == later.externalObjects.size() && later.externalObjects.find( "Mat" ) != later.externalObjects.end() && 2 == later.materialImportMode );

        CUnityMeta texture;
        ASSERT_TRUE( texture.Parse( "fileFormatVersion: 2\nguid: a0000000000000000000000000000002\nTextureImporter:\n  serializedVersion: 4\n  mipmaps:\n    sRGBTexture: 1\n"
                                    "  textureSettings:\n    wrapMode: -1\n    wrapU: 1\n  textureType: 0\n", problem ) );
        ASSERT_TRUE( texture.IsTexture() && 1 == texture.sRGBTexture && -1 == texture.linearTexture && 1 == texture.wrapU && -1 == texture.wrapMode );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Meta: the scale of a model, globalScale times the file unit where the flag of its version applies it" )
    try
    {
        CORE::CString problem;
        CUnityMeta centimetres;     // version 18: useFileScale
        ASSERT_TRUE( centimetres.Parse( "guid: 1\nModelImporter:\n  serializedVersion: 18\n  meshes:\n    globalScale: 100\n    useFileUnits: 1\n    useFileScale: 1\n", problem ) );
        ASSERT_TRUE( Near( (Float32) centimetres.GetEffectiveScale( 0.01 ), 1.0f, 1e-6f ) );
        CUnityMeta noFileScale;
        ASSERT_TRUE( noFileScale.Parse( "guid: 2\nModelImporter:\n  serializedVersion: 18\n  meshes:\n    globalScale: 1\n    useFileUnits: 1\n    useFileScale: 0\n", problem ) );
        ASSERT_TRUE( Near( (Float32) noFileScale.GetEffectiveScale( 0.01 ), 1.0f, 1e-6f ) );
        CUnityMeta older;           // version 16: no useFileScale, the units of the file are Unity units
        ASSERT_TRUE( older.Parse( "guid: 3\nModelImporter:\n  serializedVersion: 16\n  meshes:\n    globalScale: 1\n    useFileUnits: 1\n", problem ) );
        ASSERT_TRUE( !older.hasUseFileScale && !older.AppliesFileScale() && Near( (Float32) older.GetEffectiveScale( 0.01 ), 1.0f, 1e-6f ) );
        CUnityMeta metres;
        ASSERT_TRUE( metres.Parse( "guid: 4\nModelImporter:\n  serializedVersion: 21\n  meshes:\n    globalScale: .00999999978\n    useFileScale: 1\n", problem ) );
        ASSERT_TRUE( Near( (Float32) metres.GetEffectiveScale( 1.0 ), 0.01f, 1e-7f ) && !metres.bakeAxisConversion );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Shaders: the Properties block of a ShaderLab shader and the properties of shader graphs of both formats" )
    try
    {
        CUnityShaderInfo shader;
        ASSERT_TRUE( shader.ParseShaderLab( "// made by a node editor\nShader \"Vendor/Triplanar\"\n{\n\tProperties\n\t{\n"
                                            "\t\t_Texture(\"Texture\", 2D) = \"white\" {}\n\t\t/* a comment */ [HDR]_EmissionColor(\"Emission Color\", Color) = (0,0,0,0)\n"
                                            "\t\t_Tiling(\"Tiling\", Range( 0 , 10)) = 1\n\t\t[Toggle(_X_ON)] _X(\"X\", Float) = 0\n"
                                            "\t\t[HideInInspector] _texcoord( \"\", 2D ) = \"white\" {}\n\t}\n"
                                            "\tSubShader { Tags{ \"Queue\" = \"AlphaTest+0\" } }\n}\n" ) );
        ASSERT_TRUE( "Vendor/Triplanar" == shader.name && 5 == shader.properties.size() && shader.alphaTested && !shader.transparent );
        const CUnityShaderInfo::SProperty* tiling = shader.FindProperty( "_Tiling" );
        ASSERT_TRUE( GUCEF_NULL != tiling && "Range" == tiling->type && "1" == tiling->defaultValue );
        const CUnityShaderInfo::SProperty* emission = shader.FindProperty( "_EmissionColor" );
        ASSERT_TRUE( GUCEF_NULL != emission && CUnityShaderInfo::IsColourType( emission->type ) && shader.HasAttribute( *emission, "HDR" ) );
        const CUnityShaderInfo::SProperty* toggle = shader.FindProperty( "_X" );
        ASSERT_TRUE( GUCEF_NULL != toggle && shader.HasAttribute( *toggle, "Toggle" ) );
        const CUnityShaderInfo::SProperty* hidden = shader.FindProperty( "_texcoord" );
        ASSERT_TRUE( GUCEF_NULL != hidden && shader.HasAttribute( *hidden, "HideInInspector" ) && CUnityShaderInfo::IsTextureType( hidden->type ) );

        CUnityShaderInfo graph;
        ASSERT_TRUE( graph.ParseShaderGraph( "{\n \"m_SerializedProperties\": [\n  {\n   \"typeInfo\": { \"fullName\": \"UnityEditor.ShaderGraph.Texture2DShaderProperty\" },\n"
                                             "   \"JSONnodeData\": \"{\\n \\\"m_Name\\\": \\\"Grid\\\",\\n \\\"m_DefaultReferenceName\\\": \\\"Texture2D_ABC\\\",\\n \\\"m_OverrideReferenceName\\\": \\\"_Grid\\\"\\n}\"\n  },\n"
                                             "  {\n   \"typeInfo\": { \"fullName\": \"UnityEditor.ShaderGraph.Vector1ShaderProperty\" },\n"
                                             "   \"JSONnodeData\": \"{\\n \\\"m_Name\\\": \\\"Scale\\\",\\n \\\"m_DefaultReferenceName\\\": \\\"Vector1_DEF\\\",\\n \\\"m_OverrideReferenceName\\\": \\\"\\\"\\n}\"\n  }\n ],\n"
                                             " \"m_SurfaceType\": 1\n}\n", "Grid" ) );
        ASSERT_TRUE( "Shader Graphs/Grid" == graph.name && 2 == graph.properties.size() && graph.transparent );
        const CUnityShaderInfo::SProperty* grid = graph.FindProperty( "_Grid" );
        ASSERT_TRUE( GUCEF_NULL != grid && "2D" == grid->type && "Grid" == grid->displayName );
        const CUnityShaderInfo::SProperty* scale = graph.FindProperty( "Vector1_DEF" );
        ASSERT_TRUE( GUCEF_NULL != scale && "Float" == scale->type );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mesh objects: hexadecimal vertex data and indices of 16 and 32 bits, streams, submeshes" )
    try
    {
        // two triangles of a quad, 32 bit indices, positions and UVs in one stream (the 2019 layout of 14 channels)
        const char* positions = "00000000" "00000000" "00000000"   "0000803f" "00000000" "00000000"   "0000803f" "0000803f" "00000000"   "00000000" "0000803f" "00000000";
        CORE::CString channels;
        for ( UInt32 c=0; c<14; ++c )
        {
            const char* dimension = 0 == c ? "3" : ( 4 == c ? "2" : "0" );
            const char* offset = 4 == c ? "12" : "0";
            channels += "    - stream: 0\n      offset: " + CORE::CString( offset ) + "\n      format: 0\n      dimension: " + CORE::CString( dimension ) + "\n";
        }
        CORE::CString vertexData;
        const char* uvs[ 4 ] = { "00000000" "00000000", "0000803f" "00000000", "0000803f" "0000803f", "00000000" "0000803f" };
        for ( UInt32 v=0; v<4; ++v )
            vertexData += CORE::CString( positions ).SubstrFromRange( v * 24, v * 24 + 24 ) + uvs[ v ];
        const CORE::CString text = "--- !u!43 &4300000\nMesh:\n  m_Name: Quad\n  serializedVersion: 10\n  m_SubMeshes:\n  - serializedVersion: 2\n    firstByte: 0\n    indexCount: 6\n"
                                   "    topology: 0\n    baseVertex: 0\n    firstVertex: 0\n    vertexCount: 4\n  m_IndexFormat: 1\n"
                                   "  m_IndexBuffer: 000000000100000002000000000000000200000003000000\n  m_VertexData:\n    serializedVersion: 3\n    m_VertexCount: 4\n"
                                   "    m_Channels:\n" + channels + "    m_DataSize: 80\n    _typelessdata: " + vertexData + "\n";
        CUnityYamlFile file;
        CORE::CString problem;
        ASSERT_TRUE( CUnityYamlReader::Parse( text, file, problem ) );
        {
            const CUnityYamlDocument* document = file.FindDocument( 4300000 );
            ASSERT_TRUE( GUCEF_NULL != document );
            if ( GUCEF_NULL != document )
            {
                ASSETCONVERTER::CAssetMesh mesh;
                ASSERT_TRUE( CUnityMeshDecoder::Decode( document->body, mesh, problem ) );
                ASSERT_TRUE( "Quad" == mesh.name && 1 == mesh.subMeshes.size() );
                if ( 1 == mesh.subMeshes.size() )
                {
                    const ASSETCONVERTER::CAssetSubMesh& subMesh = mesh.subMeshes[ 0 ];
                    ASSERT_TRUE( 4 == subMesh.GetVertexCount() && 2 == subMesh.GetTriangleCount() && 3 == subMesh.indices[ 5 ] );
                    ASSERT_TRUE( 1.0f == subMesh.positions[ 6 ] && 1.0f == subMesh.positions[ 7 ] && 8 == subMesh.uvSets[ 0 ].size() );
                    // Unity's V goes up, the model's down
                    ASSERT_TRUE( 1 == subMesh.GetUvSetCount() && 1.0f == subMesh.uvSets[ 0 ][ 1 ] && 0.0f == subMesh.uvSets[ 0 ][ 5 ] );
                }
            }
        }
        GUCEF::vector< UInt8 > bytes;
        ASSERT_TRUE( CUnityMeshDecoder::DecodeHex( "00ff7F", bytes ) && 3 == bytes.size() && 0xFF == bytes[ 1 ] && 0x7F == bytes[ 2 ] );
        ASSERT_FALSE( CUnityMeshDecoder::DecodeHex( "0g", bytes ) );
        ASSERT_FALSE( CUnityMeshDecoder::DecodeHex( "abc", bytes ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Built-ins and modifications: the shader table, primitive meshes, property paths of prefab instances" )
    try
    {
        CORE::CString name;
        ASSERT_TRUE( CUnityBuiltins::SHADERKIND_STANDARD == CUnityBuiltins::ClassifyBuiltinShader( 46, name ) && "Standard" == name );
        ASSERT_TRUE( CUnityBuiltins::SHADERKIND_STANDARDSPECULAR == CUnityBuiltins::ClassifyBuiltinShader( 45, name ) );
        ASSERT_TRUE( CUnityBuiltins::SHADERKIND_LEGACYDIFFUSE == CUnityBuiltins::ClassifyBuiltinShader( 7, name ) );
        ASSERT_TRUE( CUnityBuiltins::SHADERKIND_BUILTINPARTICLE == CUnityBuiltins::ClassifyBuiltinShader( 203, name ) );
        ASSERT_TRUE( CUnityBuiltins::SHADERKIND_BUILTINSPRITE == CUnityBuiltins::ClassifyBuiltinShader( 10752, name ) );
        ASSERT_TRUE( CUnityBuiltins::SHADERKIND_BUILTINUNKNOWN == CUnityBuiltins::ClassifyBuiltinShader( 12345, name ) );
        ASSERT_TRUE( (Int64) -6465566751694194690LL == CUnityBuiltins::GetShaderGraphMainObjectId() );

        ASSETCONVERTER::CAssetMesh cube;
        ASSERT_TRUE( CUnityBuiltins::MakeBuiltinMesh( 10202, cube ) && "Cube" == cube.name && 24 == cube.GetVertexCount() && 12 == cube.GetTriangleCount() );
        Float32 min[ 3 ], max[ 3 ];
        GetMeshBounds( cube, min, max );
        ASSERT_TRUE( Near( min[ 0 ], -0.5f ) && Near( max[ 1 ], 0.5f ) );
        ASSETCONVERTER::CAssetMesh sphere;
        ASSERT_TRUE( CUnityBuiltins::MakeBuiltinMesh( 10207, sphere ) && sphere.GetTriangleCount() > 100 );
        ASSETCONVERTER::CAssetMesh capsule;
        ASSERT_FALSE( CUnityBuiltins::MakeBuiltinMesh( 10208, capsule ) );

        CORE::CDataNode renderer( GUCEF_DATATYPE_OBJECT );
        CORE::CDataNode* materials = renderer.AddChild( "m_Materials", GUCEF_DATATYPE_ARRAY );
        CORE::CDataNode* first = materials->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        first->SetAttribute( "fileID", "2100000" );
        first->SetAttribute( "guid", "c0000000000000000000000000000001" );
        CORE::CDataNode reference( GUCEF_DATATYPE_OBJECT );
        reference.SetAttribute( "fileID", "2100000" );
        reference.SetAttribute( "guid", "c0000000000000000000000000000003" );
        reference.SetAttribute( "type", "2" );
        ASSERT_TRUE( CUnityPrefabReader::ApplyPropertyPath( renderer, "m_Materials.Array.data[0]", CORE::CString::Empty, &reference ) );
        ASSERT_TRUE( "c0000000000000000000000000000003" == CUnityYamlReader::GetString( *materials->GetChildAtIndex( 0 ), "guid" ) );
        ASSERT_TRUE( CUnityPrefabReader::ApplyPropertyPath( renderer, "m_Materials.Array.size", "3", GUCEF_NULL ) && 3 == materials->GetNrOfDirectChildNodes() );
        ASSERT_TRUE( CUnityPrefabReader::ApplyPropertyPath( renderer, "m_Materials.Array.size", "1", GUCEF_NULL ) && 1 == materials->GetNrOfDirectChildNodes() );
        CORE::CDataNode none( GUCEF_DATATYPE_OBJECT );
        none.SetAttribute( "fileID", "0" );
        ASSERT_TRUE( CUnityPrefabReader::ApplyPropertyPath( renderer, "m_LocalPosition.x", "5", &none ) && 5.0f == CUnityYamlReader::GetFloat( *renderer.FindChild( "m_LocalPosition" ), "x", 0.0f ) );
        ASSERT_TRUE( CUnityPrefabReader::ApplyPropertyPath( renderer, "m_Name", "HouseA", &none ) && "HouseA" == CUnityYamlReader::GetString( renderer, "m_Name" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
