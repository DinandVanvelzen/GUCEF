/*
 *  assetpluginGODOT_TestApp: Test application for the Godot importer plugin
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
#include "TestGodotParticles.h"

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

/** The texture and the material of the sparks: additive, a sprite sheet of 4 x 2 frames, the particle colour used */
const char* const SparkMaterial =
    "[gd_resource type=\"StandardMaterial3D\" load_steps=2 format=3]\n\n"
    "[ext_resource type=\"Texture2D\" path=\"res://spark.png\" id=\"1_tex\"]\n\n"
    "[resource]\n"
    "transparency = 1\n"
    "blend_mode = 1\n"
    "cull_mode = 2\n"
    "shading_mode = 0\n"
    "vertex_color_use_as_albedo = true\n"
    "albedo_color = Color(1, 1, 1, 0.5)\n"
    "albedo_texture = ExtResource(\"1_tex\")\n"
    "billboard_mode = 3\n"
    "particles_anim_h_frames = 4\n"
    "particles_anim_v_frames = 2\n"
    "particles_anim_loop = true\n";

/** A scene with the resources of a GPU particle system: curves, gradients, a process material and a quad */
const char* const SparkScene =
    "[gd_scene load_steps=9 format=3]\n\n"
    "[ext_resource type=\"Material\" path=\"res://spark.tres\" id=\"1_mat\"]\n\n"
    "[sub_resource type=\"Curve\" id=\"Curve_size\"]\n"
    "_data = [Vector2(0, 1), 0.0, -1.0, 0, 0, Vector2(1, 0), -1.0, 0.0, 0, 0]\n"
    "point_count = 2\n\n"
    "[sub_resource type=\"CurveTexture\" id=\"CurveTexture_size\"]\n"
    "curve = SubResource(\"Curve_size\")\n\n"
    "[sub_resource type=\"Gradient\" id=\"Gradient_fade\"]\n"
    "offsets = PackedFloat32Array(0, 0.5, 1)\n"
    "colors = PackedColorArray(1, 0.5, 0, 1, 1, 0.25, 0, 1, 0.5, 0, 0, 0)\n\n"
    "[sub_resource type=\"GradientTexture1D\" id=\"GradientTexture1D_fade\"]\n"
    "gradient = SubResource(\"Gradient_fade\")\n\n"
    "[sub_resource type=\"ParticleProcessMaterial\" id=\"Process_spark\"]\n"
    "lifetime_randomness = 0.5\n"
    "emission_shape = 3\n"
    "emission_box_extents = Vector3(2, 0.5, 1)\n"
    "emission_shape_offset = Vector3(0, 1, 0)\n"
    "direction = Vector3(0, 2, 0)\n"
    "spread = 20.0\n"
    "initial_velocity_min = 1.0\n"
    "initial_velocity_max = 3.0\n"
    "angle_min = -30.0\n"
    "angle_max = 30.0\n"
    "gravity = Vector3(0, -2, 0)\n"
    "scale_min = 0.5\n"
    "scale_max = 1.5\n"
    "scale_curve = SubResource(\"CurveTexture_size\")\n"
    "color_ramp = SubResource(\"GradientTexture1D_fade\")\n"
    "anim_speed_min = 1.0\n"
    "anim_speed_max = 1.0\n"
    "damping_max = 0.5\n"
    "turbulence_enabled = true\n"
    "turbulence_noise_strength = 2.0\n"
    "turbulence_noise_scale = 4.0\n\n"
    "[sub_resource type=\"QuadMesh\" id=\"Quad_spark\"]\n"
    "material = ExtResource(\"1_mat\")\n"
    "size = Vector2(0.5, 1)\n"
    "center_offset = Vector3(0, 0.5, 0)\n\n"
    "[sub_resource type=\"ParticleProcessMaterial\" id=\"Process_ring\"]\n"
    "emission_shape = 6\n"
    "emission_ring_axis = Vector3(0, 1, 0)\n"
    "emission_ring_radius = 2.0\n"
    "emission_ring_inner_radius = 1.0\n"
    "emission_ring_height = 0.2\n"
    "direction = Vector3(0, 0, 0)\n"
    "spread = 180.0\n\n"
    "[node name=\"Sparks\" type=\"GPUParticles3D\"]\n"
    "amount = 20\n"
    "lifetime = 2.0\n"
    "speed_scale = 0.5\n"
    "explosiveness = 0.25\n"
    "local_coords = true\n"
    "visibility_aabb = AABB(-2, 0, -1, 4, 3, 2)\n"
    "process_material = SubResource(\"Process_spark\")\n"
    "draw_pass_1 = SubResource(\"Quad_spark\")\n\n"
    "[node name=\"Ring\" type=\"GPUParticles3D\" parent=\".\"]\n"
    "transform = Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0)\n"
    "one_shot = true\n"
    "process_material = SubResource(\"Process_ring\")\n"
    "draw_pass_1 = SubResource(\"Quad_spark\")\n";

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGodotParticleTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetpluginGODOT.Particles" );

    GUCEF_TESTFW_TESTCASE( "Particles: a GPUParticles3D with its process material, curve and gradient textures, quad and material becomes a particle system of its node" )
    try
    {
        MakeWorkFolder( "particles" );
        ASSERT_TRUE( WriteWorkText( "particles/project.godot", "config_version=5\n" ) );
        ASSERT_TRUE( WriteTestPng( "work/particles/spark.png", 8, 4, 255, 255, 255, 255 ) );
        ASSERT_TRUE( WriteWorkText( "particles/spark.tres", SparkMaterial ) );
        ASSERT_TRUE( WriteWorkText( "particles/sparks.tscn", SparkScene ) );
        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportGodot( "work/particles", "sparks.tscn", scene, report ) );
        REQUIRE( 2 == scene.particleSystems.size() );
        const Int32 node = scene.FindNode( "Sparks" );
        REQUIRE( node >= 0 && 0 == scene.nodes[ node ].particleSystem );

        const CAssetParticleSystem& s = scene.particleSystems[ 0 ];
        ASSERT_TRUE( "Sparks" == s.name && 20 == s.maxParticles && Near( 2.0f, s.duration ) && s.looping && Near( 0.5f, s.simulationSpeed ) );
        ASSERT_TRUE( Near( 10.0f, s.rateOverTime.GetAverage() ) && Near( 0.25f, s.explosiveness ) && PARTICLESPACE_LOCAL == s.simulationSpace );
        ASSERT_TRUE( Near( 1.0f, s.startLifetime.GetMinimum() ) && Near( 2.0f, s.startLifetime.GetMaximum() ) );
        ASSERT_TRUE( s.hasBounds && Near( -2.0f, s.boundsMin[ 0 ] ) && Near( 3.0f, s.boundsMax[ 1 ] ) );
        ASSERT_TRUE( PARTICLESHAPE_BOX == s.shape.type && Near( 4.0f, s.shape.boxSize[ 0 ] ) && Near( 1.0f, s.shape.boxSize[ 1 ] ) && Near( 1.0f, s.shape.position[ 1 ] ) );
        ASSERT_TRUE( PARTICLEDIRECTION_FIXED == s.shape.directionMode && Near( 1.0f, s.shape.direction[ 1 ] ) && Near( 20.0f, s.shape.spreadAngle ) );
        ASSERT_TRUE( Near( 1.0f, s.startSpeed.GetMinimum() ) && Near( 3.0f, s.startSpeed.GetMaximum() ) && Near( -2.0f, s.gravity[ 1 ] ) );
        ASSERT_TRUE( Near( -30.0f, s.startRotation.GetMinimum() ) && Near( 30.0f, s.startRotation.GetMaximum() ) );
        ASSERT_TRUE( Near( 0.5f, s.startSize.GetMinimum() ) && Near( 1.5f, s.startSize.GetMaximum() ) );
        ASSERT_TRUE( s.sizeOverLifetimeEnabled && Near( 0.5f, s.sizeOverLifetime.Evaluate( 0.5f, 0.0f ) ) );
        ASSERT_TRUE( s.colourOverLifetimeEnabled );
        Float32 rgba[ 4 ];
        s.colourOverLifetime.Evaluate( 0.75f, 0.0f, rgba );
        ASSERT_TRUE( Near( 0.75f, rgba[ 0 ] ) && Near( 0.125f, rgba[ 1 ] ) && Near( 0.5f, rgba[ 3 ] ) );
        ASSERT_TRUE( Near( 0.25f, s.damping.GetAverage() ) && s.noise.enabled && Near( 0.25f, s.noise.frequency ) && Near( 0.2f, s.noise.strength.GetAverage() ) );

        // the draw pass and its material
        ASSERT_TRUE( PARTICLERENDER_BILLBOARD == s.renderer.mode && Near( 0.5f, s.renderer.quadSize[ 0 ] ) && Near( 0.5f, s.renderer.pivot[ 1 ] ) );
        ASSERT_TRUE( PARTICLEBLEND_ADDITIVE == s.renderer.blendMode && s.renderer.useParticleColour && !s.renderer.lit && Near( 0.5f, s.renderer.tint[ 3 ] ) );
        ASSERT_TRUE( s.textureSheet.enabled && 4 == s.textureSheet.tilesX && 2 == s.textureSheet.tilesY && Near( 1.0f, s.textureSheet.cycles ) );
        ASSERT_TRUE( s.renderer.material >= 0 && (size_t) s.renderer.material < scene.materials.size() );
        if ( s.renderer.material >= 0 && (size_t) s.renderer.material < scene.materials.size() )
            ASSERT_TRUE( scene.materials[ s.renderer.material ].baseColorTexture.IsUsed() );
        ASSERT_TRUE( GUCEF_NULL != s.source.FindChild( "processMaterial" ) && GUCEF_NULL != s.source.FindChild( "node" ) );

        // the ring: its axis, its radii; a zero direction is +Z; a one shot system does not loop
        const CAssetParticleSystem& r = scene.particleSystems[ 1 ];
        ASSERT_TRUE( PARTICLESHAPE_RING == r.shape.type && Near( 2.0f, r.shape.radius ) && Near( 1.0f, r.shape.ringInnerRadius ) && Near( 0.2f, r.shape.ringHeight ) );
        Float32 axis[ 3 ];
        r.shape.GetAxis( axis );
        ASSERT_TRUE( Near( 1.0f, axis[ 1 ], 1e-4f ) && Near( 1.0f, r.shape.direction[ 2 ] ) && Near( 180.0f, r.shape.spreadAngle ) );
        ASSERT_TRUE( !r.looping && Near( 8.0f, r.rateOverTime.GetAverage() ) && PARTICLESPACE_WORLD == r.simulationSpace );

        // the node type is no longer skipped
        ASSERT_TRUE( GUCEF_NULL != scene.nodes[ node ].extras.GetAttribute( "godotType" ) );
        bool skippedNode = false;
        const CConversionReport::TEntryVector& entries = report.GetEntries();
        for ( size_t e=0; e<entries.size(); ++e )
            for ( size_t i=0; i<entries[ e ].items.size(); ++i )
                skippedNode = skippedNode || entries[ e ].items[ i ].message.HasSubstr( "node type GPUParticles3D" ) >= 0;
        ASSERT_FALSE( skippedNode );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Particles: CPUParticles3D reads the same properties from the node; class defaults where the file writes none; a sub-emitter by node path" )
    try
    {
        MakeWorkFolder( "cpuparticles" );
        ASSERT_TRUE( WriteWorkText( "cpuparticles/project.godot", "config_version=5\n" ) );
        ASSERT_TRUE( WriteWorkText( "cpuparticles/cpu.tscn",
            "[gd_scene load_steps=4 format=3]\n\n"
            "[sub_resource type=\"Gradient\" id=\"Gradient_ramp\"]\n"
            "colors = PackedColorArray(0, 0, 1, 1, 1, 1, 1, 1)\n\n"
            "[sub_resource type=\"QuadMesh\" id=\"Quad\"]\n\n"
            "[sub_resource type=\"ParticleProcessMaterial\" id=\"Process\"]\n"
            "sub_emitter_mode = 2\n\n"
            "[node name=\"Root\" type=\"Node3D\"]\n\n"
            "[node name=\"Puffs\" type=\"CPUParticles3D\" parent=\".\"]\n"
            "amount = 4\n"
            "mesh = SubResource(\"Quad\")\n"
            "emission_shape = 1\n"
            "emission_sphere_radius = 0.25\n"
            "scale_amount_min = 2.0\n"
            "scale_amount_max = 2.0\n"
            "color_ramp = SubResource(\"Gradient_ramp\")\n\n"
            "[node name=\"Shower\" type=\"GPUParticles3D\" parent=\".\"]\n"
            "process_material = SubResource(\"Process\")\n"
            "draw_pass_1 = SubResource(\"Quad\")\n"
            "sub_emitter = NodePath(\"../Puffs\")\n" ) );
        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportGodot( "work/cpuparticles", "cpu.tscn", scene, report ) );
        REQUIRE( 2 == scene.particleSystems.size() );
        const CAssetParticleSystem& puffs = scene.particleSystems[ 0 ];
        ASSERT_TRUE( 4 == puffs.maxParticles && Near( 4.0f, puffs.rateOverTime.GetAverage() ) && PARTICLESHAPE_SPHERE == puffs.shape.type && Near( 0.25f, puffs.shape.radius ) );
        ASSERT_TRUE( Near( 2.0f, puffs.startSize.GetAverage() ) && puffs.colourOverLifetimeEnabled );
        Float32 rgba[ 4 ];
        puffs.colourOverLifetime.Evaluate( 0.0f, 0.0f, rgba );
        ASSERT_TRUE( Near( 0.0f, rgba[ 0 ] ) && Near( 1.0f, rgba[ 2 ] ) );

        // the defaults of ParticleProcessMaterial and GPUParticles3D
        const CAssetParticleSystem& shower = scene.particleSystems[ 1 ];
        ASSERT_TRUE( 8 == shower.maxParticles && Near( 1.0f, shower.duration ) && Near( 1.0f, shower.shape.direction[ 0 ] ) && Near( 45.0f, shower.shape.spreadAngle ) );
        ASSERT_TRUE( Near( -9.8f, shower.gravity[ 1 ] ) && PARTICLESHAPE_POINT == shower.shape.type && PARTICLESPACE_WORLD == shower.simulationSpace );
        // without a material the quad faces +Z of its space
        ASSERT_TRUE( PARTICLERENDER_BILLBOARD == shower.renderer.mode && -1 == shower.renderer.material );
        ASSERT_TRUE( 1 == shower.subEmitters.size() && 0 == shower.subEmitters[ 0 ].system && PARTICLETRIGGER_DEATH == shower.subEmitters[ 0 ].trigger );
        ASSERT_TRUE( CAssetParticleSystem::IsSubEmitter( scene.particleSystems, 0 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}
