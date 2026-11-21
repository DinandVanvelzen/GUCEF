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
#include "TestGodotMaterials.h"

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

Float32
Linear( Float32 value )
{
    return value <= 0.04045f ? value / 12.92f : (Float32) pow( ( value + 0.055 ) / 1.055, 2.4 );
}

/**
 *  A scene with one box mesh that has one material: the folder gets the pictures a.png, b.png and c.png, and the
 *  scene s.tscn whose material has the properties (lines of "key = value"); a, b and c are ExtResource("a"), ("b")
 *  and ("c") in them.
 */
bool
ImportMaterial( const char* folder, const char* type, const CORE::CString& properties, CAssetScene& scene, CConversionReport& report )
{
    const CORE::CString name = folder;
    MakeWorkFolder( name );
    if ( !WriteTestPng( "work/" + name + "/a.png", 4, 4, 200, 100, 50, 255 ) || !WriteTestPng( "work/" + name + "/b.png", 4, 4, 10, 20, 30, 255 ) ||
         !WriteTestPng( "work/" + name + "/c.png", 4, 4, 128, 128, 255, 255 ) )
        return false;
    const CORE::CString text =
        "[gd_scene load_steps=5 format=3]\n\n"
        "[ext_resource type=\"Texture2D\" path=\"res://a.png\" id=\"a\"]\n"
        "[ext_resource type=\"Texture2D\" path=\"res://b.png\" id=\"b\"]\n"
        "[ext_resource type=\"Texture2D\" path=\"res://c.png\" id=\"c\"]\n\n"
        "[sub_resource type=\"" + CORE::CString( type ) + "\" id=\"m\"]\n" + properties + "\n"
        "[sub_resource type=\"BoxMesh\" id=\"box\"]\nmaterial = SubResource(\"m\")\n\n"
        "[node name=\"Box\" type=\"MeshInstance3D\"]\nmesh = SubResource(\"box\")\n";
    return WriteWorkText( name + "/s.tscn", text ) && ImportGodot( "work/" + name, "s.tscn", scene, report );
}

bool
EndsWith( const CORE::CString& text, const char* tail )
{
    return text.EndsWith( tail );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGodotMaterialTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetpluginGODOT.Materials" );

    GUCEF_TESTFW_TESTCASE( "Material: albedo_color (sRGB to linear) and albedo_texture give the base colour; the source material keeps shader, properties and textures" )
    try
    {
        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportMaterial( "mat1", "StandardMaterial3D", "albedo_color = Color(0.5, 0.5, 0.5, 0.8)\nalbedo_texture = ExtResource(\"a\")\nmetallic_specular = 0.3\n", scene, report ) );
        REQUIRE( 1 == scene.materials.size() && 1 == scene.textures.size() );
        const CAssetMaterial& material = scene.materials[ 0 ];
        ASSERT_TRUE( "m" == material.name && Near( material.baseColorFactor[ 0 ], Linear( 0.5f ), 1e-4f ) && Near( material.baseColorFactor[ 3 ], 0.8f ) );
        ASSERT_TRUE( 0 == material.baseColorTexture.textureIndex && ASSETCONVERTER::TEXTUREUSAGE_COLOUR == scene.textures[ 0 ].usage && EndsWith( scene.textures[ 0 ].sources[ 0 ], "mat1/a.png" ) );
        ASSERT_TRUE( scene.textures[ 0 ].operations.empty() && Near( material.metallicFactor, 0.0f ) && Near( material.roughnessFactor, 1.0f ) );
        ASSERT_TRUE( "StandardMaterial3D" == material.source.shaderName && Near( material.source.properties.GetAttributeValue( "metallic_specular" ).AsFloat32(), 0.3f ) );
        ASSERT_TRUE( EndsWith( material.source.textures.GetAttributeValue( "albedo_texture" ).AsString(), "mat1/a.png" ) );
        ASSERT_TRUE( ALPHAMODE_OPAQUE == material.alphaMode && !material.doubleSided && !material.unlit );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Material: an ORM texture is the metallic-roughness and the occlusion texture without repacking" )
    try
    {
        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportMaterial( "mat2", "ORMMaterial3D", "orm_texture = ExtResource(\"b\")\nmetallic = 1.0\nroughness = 0.8\n", scene, report ) );
        REQUIRE( 1 == scene.materials.size() && 1 == scene.textures.size() );
        const CAssetMaterial& material = scene.materials[ 0 ];
        ASSERT_TRUE( 0 == material.metallicRoughnessTexture.textureIndex && 0 == material.occlusionTexture.textureIndex );
        ASSERT_TRUE( scene.textures[ 0 ].operations.empty() && 1 == scene.textures[ 0 ].sources.size() && ASSETCONVERTER::TEXTUREUSAGE_MASK == scene.textures[ 0 ].usage );
        ASSERT_TRUE( Near( material.metallicFactor, 1.0f ) && Near( material.roughnessFactor, 0.8f ) && "ORMMaterial3D" == material.source.shaderName );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Material: separate metallic and roughness textures are repacked into G and B, with the channels they are read from" )
    try
    {
        {
            CAssetScene scene;
            CConversionReport report;
            REQUIRE( ImportMaterial( "mat3a", "StandardMaterial3D", "metallic_texture = ExtResource(\"a\")\nmetallic_texture_channel = 2\nroughness_texture = ExtResource(\"b\")\nroughness_texture_channel = 0\nmetallic = 1.0\n", scene, report ) );
            REQUIRE( 1 == scene.textures.size() );
            const CAssetTexture& texture = scene.textures[ 0 ];
            REQUIRE( 2 == texture.sources.size() && 1 == texture.operations.size() && TEXTUREOPERATION_PACK == texture.operations[ 0 ].type );
            ASSERT_TRUE( EndsWith( texture.sources[ 0 ], "mat3a/b.png" ) && EndsWith( texture.sources[ 1 ], "mat3a/a.png" ) );
            const CTextureOperation& pack = texture.operations[ 0 ];
            ASSERT_TRUE( -1 == pack.packSources[ 0 ] && 0 == pack.packSources[ 1 ] && 1 == pack.packSources[ 2 ] && -1 == pack.packSources[ 3 ] );
            ASSERT_TRUE( 0 == pack.packChannels[ 1 ] && 2 == pack.packChannels[ 2 ] );
            ASSERT_TRUE( 0 == scene.materials[ 0 ].metallicRoughnessTexture.textureIndex && !scene.materials[ 0 ].occlusionTexture.IsUsed() && ASSETCONVERTER::TEXTUREUSAGE_MASK == texture.usage );
        }
        {
            // both from one file: one source, two channels
            CAssetScene scene;
            CConversionReport report;
            REQUIRE( ImportMaterial( "mat3b", "StandardMaterial3D", "metallic_texture = ExtResource(\"a\")\nmetallic_texture_channel = 3\nroughness_texture = ExtResource(\"a\")\nroughness_texture_channel = 0\n", scene, report ) );
            REQUIRE( 1 == scene.textures.size() && 1 == scene.textures[ 0 ].sources.size() && 1 == scene.textures[ 0 ].operations.size() );
            const CTextureOperation& pack = scene.textures[ 0 ].operations[ 0 ];
            ASSERT_TRUE( 0 == pack.packSources[ 1 ] && 0 == pack.packSources[ 2 ] && 0 == pack.packChannels[ 1 ] && 3 == pack.packChannels[ 2 ] );
        }
        {
            // the layout of glTF already: used as it is
            CAssetScene scene;
            CConversionReport report;
            REQUIRE( ImportMaterial( "mat3c", "StandardMaterial3D", "metallic_texture = ExtResource(\"a\")\nmetallic_texture_channel = 2\nroughness_texture = ExtResource(\"a\")\nroughness_texture_channel = 1\n", scene, report ) );
            REQUIRE( 1 == scene.textures.size() );
            ASSERT_TRUE( scene.textures[ 0 ].operations.empty() && 0 == scene.materials[ 0 ].metallicRoughnessTexture.textureIndex );
        }
        {
            // one of the two: the other channel is a constant
            CAssetScene scene;
            CConversionReport report;
            REQUIRE( ImportMaterial( "mat3d", "StandardMaterial3D", "roughness_texture = ExtResource(\"a\")\nroughness_texture_channel = 3\n", scene, report ) );
            REQUIRE( 1 == scene.textures.size() && 1 == scene.textures[ 0 ].operations.size() );
            ASSERT_TRUE( TEXTUREOPERATION_SWIZZLE == scene.textures[ 0 ].operations[ 0 ].type && "1A11" == scene.textures[ 0 ].operations[ 0 ].mapping );
            CAssetScene metallic;
            CConversionReport metallicReport;
            REQUIRE( ImportMaterial( "mat3e", "StandardMaterial3D", "metallic_texture = ExtResource(\"a\")\nmetallic_texture_channel = 4\n", metallic, metallicReport ) );
            REQUIRE( 1 == metallic.textures.size() && 1 == metallic.textures[ 0 ].operations.size() );
            ASSERT_TRUE( "11R1" == metallic.textures[ 0 ].operations[ 0 ].mapping && ReportHas( metallicReport, REPORTITEM_APPROXIMATION, "grayscale" ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Material: the normal map with its scale, and the normal map that is not enabled" )
    try
    {
        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportMaterial( "mat4a", "StandardMaterial3D", "normal_enabled = true\nnormal_scale = 2.0\nnormal_texture = ExtResource(\"c\")\n", scene, report ) );
        REQUIRE( 1 == scene.textures.size() );
        ASSERT_TRUE( scene.materials[ 0 ].normalTexture.IsUsed() && Near( scene.materials[ 0 ].normalTexture.scale, 2.0f ) && ASSETCONVERTER::TEXTUREUSAGE_NORMAL == scene.textures[ 0 ].usage );
        ASSERT_TRUE( NORMALMAPCONVENTION_OPENGL == scene.normalMapConvention );
        CAssetScene disabled;
        CConversionReport disabledReport;
        REQUIRE( ImportMaterial( "mat4b", "StandardMaterial3D", "normal_texture = ExtResource(\"c\")\n", disabled, disabledReport ) );
        ASSERT_TRUE( !disabled.materials[ 0 ].normalTexture.IsUsed() && disabled.textures.empty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Material: emission with its energy as the strength, HDR colours, and the additive texture that glTF multiplies" )
    try
    {
        {
            CAssetScene scene;
            CConversionReport report;
            REQUIRE( ImportMaterial( "mat5a", "StandardMaterial3D", "emission_enabled = true\nemission = Color(1, 0, 0, 1)\nemission_energy_multiplier = 3.0\n", scene, report ) );
            const CAssetMaterial& material = scene.materials[ 0 ];
            ASSERT_TRUE( Near( material.emissiveFactor[ 0 ], 1.0f ) && Near( material.emissiveFactor[ 1 ], 0.0f ) && Near( material.emissiveStrength, 3.0f ) && !material.emissiveTexture.IsUsed() );
        }
        {
            CAssetScene scene;
            CConversionReport report;
            REQUIRE( ImportMaterial( "mat5b", "StandardMaterial3D", "emission_enabled = true\nemission = Color(2, 0, 0, 1)\nemission_energy_multiplier = 2.0\n", scene, report ) );
            const Float32 excess = Linear( 2.0f );
            ASSERT_TRUE( Near( scene.materials[ 0 ].emissiveFactor[ 0 ], 1.0f ) && Near( scene.materials[ 0 ].emissiveStrength, 2.0f * excess, 1e-3f ) );
        }
        {
            // an additive texture with a black colour is the texture
            CAssetScene scene;
            CConversionReport report;
            REQUIRE( ImportMaterial( "mat5c", "StandardMaterial3D", "emission_enabled = true\nemission_texture = ExtResource(\"a\")\n", scene, report ) );
            ASSERT_TRUE( scene.materials[ 0 ].emissiveTexture.IsUsed() && Near( scene.materials[ 0 ].emissiveFactor[ 0 ], 1.0f ) && Near( scene.materials[ 0 ].emissiveFactor[ 2 ], 1.0f ) );
            ASSERT_FALSE( ReportHas( report, REPORTITEM_APPROXIMATION, "emission colour" ) );
        }
        {
            CAssetScene scene;
            CConversionReport report;
            REQUIRE( ImportMaterial( "mat5d", "StandardMaterial3D", "emission_enabled = true\nemission = Color(0, 1, 0, 1)\nemission_texture = ExtResource(\"a\")\n", scene, report ) );
            ASSERT_TRUE( Near( scene.materials[ 0 ].emissiveFactor[ 1 ], 1.0f ) && Near( scene.materials[ 0 ].emissiveFactor[ 0 ], 0.0f ) && ReportHas( report, REPORTITEM_APPROXIMATION, "emission colour" ) );
        }
        {
            CAssetScene scene;
            CConversionReport report;
            REQUIRE( ImportMaterial( "mat5e", "StandardMaterial3D", "emission = Color(1, 1, 1, 1)\n", scene, report ) );
            ASSERT_TRUE( Near( scene.materials[ 0 ].emissiveFactor[ 0 ], 0.0f ) && Near( scene.materials[ 0 ].emissiveStrength, 1.0f ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Material: the ambient occlusion texture in the channel it is read from, on the first or the second UV set" )
    try
    {
        {
            CAssetScene scene;
            CConversionReport report;
            REQUIRE( ImportMaterial( "mat6a", "StandardMaterial3D", "ao_enabled = true\nao_texture = ExtResource(\"b\")\n", scene, report ) );
            REQUIRE( 1 == scene.textures.size() );
            ASSERT_TRUE( scene.materials[ 0 ].occlusionTexture.IsUsed() && scene.textures[ 0 ].operations.empty() && 0 == scene.materials[ 0 ].occlusionTexture.texCoord );
        }
        {
            CAssetScene scene;
            CConversionReport report;
            REQUIRE( ImportMaterial( "mat6b", "StandardMaterial3D", "ao_enabled = true\nao_texture = ExtResource(\"b\")\nao_texture_channel = 1\nao_on_uv2 = true\nuv1_scale = Vector3(2, 2, 1)\n", scene, report ) );
            REQUIRE( 1 == scene.textures.size() && 1 == scene.textures[ 0 ].operations.size() );
            ASSERT_TRUE( "G111" == scene.textures[ 0 ].operations[ 0 ].mapping && 1 == scene.materials[ 0 ].occlusionTexture.texCoord && !scene.materials[ 0 ].occlusionTexture.hasTransform );
        }
        {
            CAssetScene scene;
            CConversionReport report;
            REQUIRE( ImportMaterial( "mat6c", "StandardMaterial3D", "ao_texture = ExtResource(\"b\")\n", scene, report ) );
            ASSERT_TRUE( !scene.materials[ 0 ].occlusionTexture.IsUsed() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Material: every transparency mode (disabled, alpha, scissor with its threshold, hash recorded as an approximation, depth pre-pass)" )
    try
    {
        struct SCase { const char* properties; TAlphaMode mode; Float32 cutoff; bool approximated; };
        const SCase cases[] = {
            { "transparency = 0\n", ALPHAMODE_OPAQUE, 0.5f, false },
            { "transparency = 1\n", ALPHAMODE_BLEND, 0.5f, false },
            { "transparency = 2\nalpha_scissor_threshold = 0.3\n", ALPHAMODE_MASK, 0.3f, false },
            { "transparency = 2\n", ALPHAMODE_MASK, 0.5f, false },
            { "transparency = 3\nalpha_hash_scale = 1.0\n", ALPHAMODE_MASK, 0.5f, true },
            { "transparency = 4\n", ALPHAMODE_BLEND, 0.5f, false } };
        for ( UInt32 c=0; c<sizeof( cases ) / sizeof( cases[ 0 ] ); ++c )
        {
            CAssetScene scene;
            CConversionReport report;
            REQUIRE( ImportMaterial( "mat7", "StandardMaterial3D", cases[ c ].properties, scene, report ) );
            ASSERT_TRUE( cases[ c ].mode == scene.materials[ 0 ].alphaMode );
            if ( ALPHAMODE_MASK == cases[ c ].mode )
                ASSERT_TRUE( Near( scene.materials[ 0 ].alphaCutoff, cases[ c ].cutoff ) );
            ASSERT_TRUE( cases[ c ].approximated == ReportHas( report, REPORTITEM_APPROXIMATION, "alpha hash" ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Material: cull_mode (disabled is double sided, front is reported) and unshaded shading" )
    try
    {
        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportMaterial( "mat8a", "StandardMaterial3D", "cull_mode = 2\nshading_mode = 0\n", scene, report ) );
        ASSERT_TRUE( scene.materials[ 0 ].doubleSided && scene.materials[ 0 ].unlit );
        CAssetScene back;
        CConversionReport backReport;
        REQUIRE( ImportMaterial( "mat8b", "StandardMaterial3D", "cull_mode = 0\nshading_mode = 1\n", back, backReport ) );
        ASSERT_TRUE( !back.materials[ 0 ].doubleSided && !back.materials[ 0 ].unlit );
        CAssetScene front;
        CConversionReport frontReport;
        REQUIRE( ImportMaterial( "mat8c", "StandardMaterial3D", "cull_mode = 1\n", front, frontReport ) );
        ASSERT_TRUE( !front.materials[ 0 ].doubleSided && ReportHas( frontReport, REPORTITEM_APPROXIMATION, "front faces" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Material: uv1_scale and uv1_offset become the texture transform of the textures on the first UV set; clamped textures; clear coat" )
    try
    {
        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportMaterial( "mat9", "StandardMaterial3D", "albedo_texture = ExtResource(\"a\")\nnormal_enabled = true\nnormal_texture = ExtResource(\"c\")\nuv1_scale = Vector3(2, 3, 1)\nuv1_offset = Vector3(0.5, 0.25, 0)\n"
                                    "texture_repeat = false\nclearcoat_enabled = true\nclearcoat = 0.75\nclearcoat_roughness = 0.1\n", scene, report ) );
        const CAssetMaterial& material = scene.materials[ 0 ];
        ASSERT_TRUE( material.baseColorTexture.hasTransform && material.normalTexture.hasTransform );
        ASSERT_TRUE( Near( material.baseColorTexture.tiling[ 0 ], 2.0f ) && Near( material.baseColorTexture.tiling[ 1 ], 3.0f ) && Near( material.baseColorTexture.offset[ 0 ], 0.5f ) && Near( material.baseColorTexture.offset[ 1 ], 0.25f ) );
        ASSERT_TRUE( IMAGE::WRAPMODE_CLAMP == scene.textures[ 0 ].wrapS && IMAGE::WRAPMODE_CLAMP == scene.textures[ 0 ].wrapT );
        const CORE::CDataNode* clearcoat = material.extensions.FindChild( "KHR_materials_clearcoat" );
        ASSERT_TRUE( GUCEF_NULL != clearcoat && Near( clearcoat->GetAttributeValue( "clearcoatFactor" ).AsFloat32(), 0.75f ) && Near( clearcoat->GetAttributeValue( "clearcoatRoughnessFactor" ).AsFloat32(), 0.1f ) );
        CAssetScene plain;
        CConversionReport plainReport;
        REQUIRE( ImportMaterial( "mat9b", "StandardMaterial3D", "albedo_texture = ExtResource(\"a\")\n", plain, plainReport ) );
        ASSERT_TRUE( !plain.materials[ 0 ].baseColorTexture.hasTransform && IMAGE::WRAPMODE_REPEAT == plain.textures[ 0 ].wrapS );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Material: detail and heightmap properties, and the others that glTF has no place for, go to the sidecar with their textures" )
    try
    {
        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportMaterial( "mat10", "StandardMaterial3D", "detail_enabled = true\ndetail_blend_mode = 1\ndetail_albedo = ExtResource(\"a\")\ndetail_normal = ExtResource(\"c\")\n"
                                     "heightmap_enabled = true\nheightmap_scale = 4.0\nheightmap_texture = ExtResource(\"b\")\nrim_enabled = true\nrim = 0.5\n", scene, report ) );
        const CAssetMaterial& material = scene.materials[ 0 ];
        ASSERT_TRUE( material.source.properties.GetAttributeValue( "detail_enabled" ).AsBool() && 1 == material.source.properties.GetAttributeValue( "detail_blend_mode" ).AsInt32() );
        ASSERT_TRUE( Near( material.source.properties.GetAttributeValue( "heightmap_scale" ).AsFloat32(), 4.0f ) && Near( material.source.properties.GetAttributeValue( "rim" ).AsFloat32(), 0.5f ) );
        ASSERT_TRUE( EndsWith( material.source.textures.GetAttributeValue( "detail_albedo" ).AsString(), "mat10/a.png" ) && EndsWith( material.source.textures.GetAttributeValue( "detail_normal" ).AsString(), "mat10/c.png" ) && EndsWith( material.source.textures.GetAttributeValue( "heightmap_texture" ).AsString(), "mat10/b.png" ) );
        ASSERT_TRUE( !material.baseColorTexture.IsUsed() && !material.normalTexture.IsUsed() && scene.textures.empty() );
        ASSERT_TRUE( ReportHas( report, REPORTITEM_INFO, "detail" ) && ReportHas( report, REPORTITEM_INFO, "heightmap" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Material: a custom shader gives what the names of its parameters tell, and says that it is an approximation" )
    try
    {
        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportMaterial( "mat11", "ShaderMaterial", "shader_parameter/albedo = Color(0, 1, 0, 1)\nshader_parameter/albedo_texture = ExtResource(\"a\")\nshader_parameter/roughness = 0.4\nshader_parameter/speed = 3.0\n", scene, report ) );
        const CAssetMaterial& material = scene.materials[ 0 ];
        ASSERT_TRUE( "ShaderMaterial" == material.source.shaderName && Near( material.baseColorFactor[ 1 ], 1.0f ) && Near( material.baseColorFactor[ 0 ], 0.0f ) && material.baseColorTexture.IsUsed() );
        ASSERT_TRUE( Near( material.roughnessFactor, 0.4f ) && Near( material.source.properties.GetAttributeValue( "shader_parameter/speed" ).AsFloat32(), 3.0f ) );
        ASSERT_TRUE( ReportHas( report, REPORTITEM_APPROXIMATION, "custom shader" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Material: resources are read once (one material and one texture for several uses), a material file of the project, a texture that is not in the pack" )
    try
    {
        MakeWorkFolder( "mat12" );
        ASSERT_TRUE( WriteTestPng( "work/mat12/shared.png", 4, 4, 1, 2, 3, 255 ) );
        ASSERT_TRUE( WriteWorkText( "mat12/wood.tres", "[gd_resource type=\"StandardMaterial3D\" format=3]\n\n[ext_resource type=\"Texture2D\" path=\"res://shared.png\" id=\"1\"]\n\n[resource]\nresource_name = \"Wood\"\nalbedo_texture = ExtResource(\"1\")\nroughness_texture = ExtResource(\"2\")\n" ) );
        ASSERT_TRUE( WriteWorkText( "mat12/s.tscn",
            "[gd_scene format=3]\n\n[ext_resource type=\"Material\" path=\"res://wood.tres\" id=\"1_w\"]\n[ext_resource type=\"Texture2D\" path=\"res://shared.png\" id=\"2_t\"]\n[ext_resource type=\"Texture2D\" path=\"res://nowhere.png\" id=\"3_n\"]\n\n"
            "[sub_resource type=\"StandardMaterial3D\" id=\"own\"]\nalbedo_texture = ExtResource(\"2_t\")\nnormal_enabled = true\nnormal_texture = ExtResource(\"3_n\")\n\n"
            "[sub_resource type=\"BoxMesh\" id=\"b1\"]\nmaterial = ExtResource(\"1_w\")\n\n[sub_resource type=\"BoxMesh\" id=\"b2\"]\nmaterial = ExtResource(\"1_w\")\n\n[sub_resource type=\"BoxMesh\" id=\"b3\"]\nmaterial = SubResource(\"own\")\n\n"
            "[node name=\"Root\" type=\"Node3D\"]\n\n[node name=\"A\" type=\"MeshInstance3D\" parent=\".\"]\nmesh = SubResource(\"b1\")\n\n[node name=\"B\" type=\"MeshInstance3D\" parent=\".\"]\nmesh = SubResource(\"b2\")\n\n"
            "[node name=\"C\" type=\"MeshInstance3D\" parent=\".\"]\nmesh = SubResource(\"b3\")\n" ) );
        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportGodot( "work/mat12", "s.tscn", scene, report ) );
        REQUIRE( 2 == scene.materials.size() );
        ASSERT_TRUE( "Wood" == scene.materials[ 0 ].name && scene.meshes[ scene.nodes[ 1 ].mesh ].subMeshes[ 0 ].materialIndex == scene.meshes[ scene.nodes[ 2 ].mesh ].subMeshes[ 0 ].materialIndex );
        // wood.tres and the sub resource use the same file in the same way: one texture
        ASSERT_TRUE( 1 == scene.textures.size() && scene.materials[ 0 ].baseColorTexture.textureIndex == scene.materials[ 1 ].baseColorTexture.textureIndex );
        ASSERT_TRUE( ReportHas( report, REPORTITEM_WARNING, "normal_texture" ) && !scene.materials[ 1 ].normalTexture.IsUsed() );

        // the relations: the texture that is there, the one that is not by its path, the material file
        bool texture = false;
        bool nowhere = false;
        bool material = false;
        for ( size_t r=0; r<scene.references.size(); ++r )
        {
            const CAssetReference& reference = scene.references[ r ];
            if ( CAssetReference::KindUsesTexture == reference.kind && reference.target.EndsWith( "mat12/shared.png" ) && "res://shared.png" == reference.via )
                texture = true;
            if ( CAssetReference::KindUsesTexture == reference.kind && !reference.IsResolved() && "res://nowhere.png" == reference.via )
                nowhere = true;
            if ( CAssetReference::KindUsesMaterial == reference.kind && reference.target.EndsWith( "mat12/wood.tres" ) )
                material = true;
        }
        ASSERT_TRUE( texture && nowhere && material );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
