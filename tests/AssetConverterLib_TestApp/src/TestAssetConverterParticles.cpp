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

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#include "TestAssetConverterSupport.h"
#include "TestAssetConverterParticles.h"

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

/** A system with something in every group of the model */
CAssetParticleSystem
MakeFullSystem( void )
{
    CAssetParticleSystem system;
    system.name = "Sparks";
    system.duration = 2.0f;
    system.looping = false;
    system.playOnAwake = false;
    system.prewarm = 1.5f;
    system.simulationSpeed = 0.5f;
    system.simulationSpace = PARTICLESPACE_WORLD;
    system.maxParticles = 77;
    system.hasBounds = true;
    system.boundsMin[ 0 ] = -1.0f; system.boundsMax[ 2 ] = 3.0f;
    system.rateOverTime = CAssetParticleCurve( 12.0f );
    CAssetParticleSystem::SBurst burst;
    burst.time = 0.25f;
    burst.count = CAssetParticleCurve::RandomBetween( 5.0f, 9.0f );
    burst.cycles = 0;
    burst.interval = 0.5f;
    system.bursts.push_back( burst );
    system.explosiveness = 0.25f;
    system.startLifetime = CAssetParticleCurve::RandomBetween( 1.0f, 2.0f );
    system.startSpeed = CAssetParticleCurve( 3.0f );
    system.startSize = CAssetParticleCurve::RandomBetween( 0.1f, 0.3f );
    system.startRotation = CAssetParticleCurve::RandomBetween( -45.0f, 45.0f );
    const Float32 orange[ 4 ] = { 1.0f, 0.5f, 0.0f, 1.0f };
    system.startColour = CAssetParticleGradient::Colour( orange );
    system.shape.type = PARTICLESHAPE_CONE;
    system.shape.radius = 0.5f;
    system.shape.angle = 30.0f;
    system.shape.position[ 2 ] = 2.0f;
    system.shape.boxSize[ 0 ] = 4.0f;
    system.shape.directionMode = PARTICLEDIRECTION_OUTWARD;
    system.gravity[ 1 ] = -9.81f;
    system.velocity.enabled = true;
    system.velocity.z = CAssetParticleCurve( 1.0f );
    CAssetParticleCurve::TKeyVector keys;
    keys.push_back( CAssetParticleCurve::SKey( 0.0f, 1.0f, 0.0f, -1.0f ) );
    keys.push_back( CAssetParticleCurve::SKey( 1.0f, 0.0f, -1.0f, 0.0f ) );
    system.sizeOverLifetimeEnabled = true;
    system.sizeOverLifetime = CAssetParticleCurve::Curve( keys );
    CAssetParticleGradient::SGradient gradient;
    const Float32 opaque[ 4 ] = { 1.0f, 1.0f, 1.0f, 1.0f };
    const Float32 clear[ 4 ] = { 1.0f, 1.0f, 1.0f, 0.0f };
    gradient.AddKey( 0.0f, opaque );
    gradient.AddKey( 1.0f, clear );
    system.colourOverLifetimeEnabled = true;
    system.colourOverLifetime = CAssetParticleGradient::Gradient( gradient );
    system.textureSheet.enabled = true;
    system.textureSheet.tilesX = 4;
    system.textureSheet.tilesY = 2;
    system.renderer.mode = PARTICLERENDER_STRETCHED;
    system.renderer.material = 1;
    system.renderer.blendMode = PARTICLEBLEND_ADDITIVE;
    system.renderer.quadSize[ 1 ] = 2.0f;
    system.renderer.pivot[ 1 ] = 0.5f;
    CAssetParticleSystem::SSubEmitter sub;
    sub.system = 1;
    sub.trigger = PARTICLETRIGGER_DEATH;
    system.subEmitters.push_back( sub );
    system.source.SetAttribute( "type", "GPUParticles3D" );
    return system;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAssetConverterParticleTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AssetConverterLib.Particles" );

    GUCEF_TESTFW_TESTCASE( "Particle values: constants, random constants and Hermite curves evaluate, give their range, mean and line; gradients keep colour and alpha keys apart" )
    try
    {
        const CAssetParticleCurve constant( 2.0f );
        ASSERT_TRUE( constant.IsConstant() && Near( 2.0f, constant.Evaluate( 0.7f, 0.3f ) ) && Near( 2.0f, constant.GetAverage() ) );
        const CAssetParticleCurve random = CAssetParticleCurve::RandomBetween( 1.0f, 3.0f );
        ASSERT_TRUE( !random.IsConstant() && random.IsConstantOverTime() && Near( 1.0f, random.GetMinimum() ) && Near( 3.0f, random.GetMaximum() ) && Near( 2.0f, random.GetAverage() ) );
        ASSERT_TRUE( CAssetParticleCurve::RandomBetween( 4.0f, 4.0f ).IsConstant() );

        // a straight line from 1 to 0 (tangents -1): its fit is exact
        CAssetParticleCurve::TKeyVector line;
        line.push_back( CAssetParticleCurve::SKey( 0.0f, 1.0f, -1.0f, -1.0f ) );
        line.push_back( CAssetParticleCurve::SKey( 1.0f, 0.0f, -1.0f, -1.0f ) );
        const CAssetParticleCurve fading = CAssetParticleCurve::Curve( line, 2.0f );
        ASSERT_TRUE( !fading.IsConstantOverTime() && Near( 1.0f, fading.Evaluate( 0.5f, 0.0f ) ) && Near( 2.0f, fading.GetMaximum() ) && Near( 0.0f, fading.GetMinimum() ) );
        Float32 intercept = 0.0f, slope = 0.0f, error = 1.0f;
        fading.GetLinearFit( intercept, slope, error );
        ASSERT_TRUE( Near( 2.0f, intercept, 1e-3f ) && Near( -2.0f, slope, 1e-3f ) && error < 1e-3f );

        // a bell: flat ends, a peak in the middle; its line is flat, its error large
        CAssetParticleCurve::TKeyVector bell;
        bell.push_back( CAssetParticleCurve::SKey( 0.0f, 0.0f ) );
        bell.push_back( CAssetParticleCurve::SKey( 0.5f, 1.0f ) );
        bell.push_back( CAssetParticleCurve::SKey( 1.0f, 0.0f ) );
        const CAssetParticleCurve bellCurve = CAssetParticleCurve::Curve( bell );
        ASSERT_TRUE( Near( 0.5f, bellCurve.Evaluate( 0.25f, 0.0f ) ) && Near( 1.0f, bellCurve.Evaluate( 0.5f, 0.0f ) ) );
        bellCurve.GetLinearFit( intercept, slope, error );
        ASSERT_TRUE( Near( 0.0f, slope, 1e-3f ) && error > 0.3f );

        // a step: a slope that is not finite holds the value of the key
        CAssetParticleCurve::TKeyVector step;
        step.push_back( CAssetParticleCurve::SKey( 0.0f, 1.0f, 0.0f, 1.0e30f ) );
        step.push_back( CAssetParticleCurve::SKey( 1.0f, 5.0f, 1.0e30f, 0.0f ) );
        ASSERT_TRUE( Near( 1.0f, CAssetParticleCurve::EvaluateKeys( step, 0.9f ) ) );

        CAssetParticleGradient::SGradient keys;
        keys.colourKeys.push_back( CAssetParticleGradient::SColourKey( 0.0f, 1.0f, 0.0f, 0.0f ) );
        keys.colourKeys.push_back( CAssetParticleGradient::SColourKey( 1.0f, 0.0f, 0.0f, 1.0f ) );
        keys.alphaKeys.push_back( CAssetParticleGradient::SAlphaKey( 0.0f, 1.0f ) );
        keys.alphaKeys.push_back( CAssetParticleGradient::SAlphaKey( 0.5f, 0.0f ) );
        const CAssetParticleGradient gradient = CAssetParticleGradient::Gradient( keys );
        Float32 rgba[ 4 ];
        gradient.Evaluate( 0.25f, 0.0f, rgba );
        ASSERT_TRUE( Near( 0.75f, rgba[ 0 ] ) && Near( 0.25f, rgba[ 2 ] ) && Near( 0.5f, rgba[ 3 ] ) );
        gradient.Evaluate( 0.75f, 0.0f, rgba );
        ASSERT_TRUE( Near( 0.0f, rgba[ 3 ] ) );
        GUCEF::vector< Float32 > times;
        gradient.GetKeyTimes( times );
        ASSERT_TRUE( 3 == times.size() && Near( 0.5f, times[ 1 ] ) );
        ASSERT_TRUE( CAssetParticleGradient().IsWhite() && !gradient.IsConstantOverTime() );

        // a colour picked from a gradient does not vary over time; its mean is the mean of the gradient
        CAssetParticleGradient picked = gradient;
        picked.mode = PARTICLECOLOUR_RANDOMFROMGRADIENT;
        ASSERT_TRUE( picked.IsConstantOverTime() );
        picked.GetAverageAt( 0.3f, rgba );
        ASSERT_TRUE( Near( 0.5f, rgba[ 0 ], 0.02f ) && Near( 0.5f, rgba[ 2 ], 0.02f ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Particle system: every group survives the sidecar, through canonical JSON, with the node it is attached to" )
    try
    {
        CAssetScene scene;
        const UInt32 root = scene.AddNode( "Root" );
        const UInt32 fire = scene.AddNode( "Fire", (Int32) root );
        CAssetMaterial plain;
        plain.name = "Plain";
        scene.materials.push_back( plain );
        CAssetMaterial flame;
        flame.name = "Flame";
        scene.materials.push_back( flame );
        scene.particleSystems.push_back( MakeFullSystem() );
        CAssetParticleSystem splash;
        splash.name = "Splash";
        scene.particleSystems.push_back( splash );
        scene.nodes[ fire ].particleSystem = 0;

        CConversionReportEntry entry;
        CAssetPackContext context;
        CORE::CDataNode sidecar;
        CConversionSidecar::Build( scene, entry, CORE::CString::StringVector(), "licence", context, "intermediate", sidecar );
        CORE::CDataNode parsed;
        ASSERT_TRUE( CCanonicalJson::Parse( CCanonicalJson::ToJson( sidecar ), parsed ) );
        const CORE::CDataNode* list = parsed.FindChild( "particles" );
        UInt32 entries = 0;
        for ( const CORE::CDataNode* e = GUCEF_NULL != list ? list->GetFirstChild() : GUCEF_NULL; GUCEF_NULL != e; e = e->GetNext() )
            ++entries;
        ASSERT_TRUE( GUCEF_NULL != list && 2 == entries );
        if ( GUCEF_NULL != list )
        {
            const CORE::CDataNode* first = list->GetFirstChild();
            ASSERT_TRUE( "Fire" == first->GetAttributeValueOrChildValueByName( "node" ).AsString() && "Flame" == first->GetAttributeValueOrChildValueByName( "materialName" ).AsString() );
        }
        const CORE::CDataNode* summary = parsed.FindChild( "summary" );
        ASSERT_TRUE( GUCEF_NULL != summary && 2 == summary->GetAttributeValueOrChildValueByName( "particleSystems" ).AsUInt32() );

        GUCEF::vector< CAssetParticleSystem > systems;
        GUCEF::vector< Int32 > nodes;
        ASSERT_TRUE( CConversionSidecar::ReadParticleSystems( parsed, systems, nodes ) );
        ASSERT_TRUE( 2 == systems.size() && 2 == nodes.size() && (Int32) fire == nodes[ 0 ] && -1 == nodes[ 1 ] );
        if ( 2 == systems.size() )
        {
            const CAssetParticleSystem& s = systems[ 0 ];
            ASSERT_TRUE( "Sparks" == s.name && Near( 2.0f, s.duration ) && !s.looping && !s.playOnAwake && Near( 1.5f, s.prewarm ) && Near( 0.5f, s.simulationSpeed ) );
            ASSERT_TRUE( PARTICLESPACE_WORLD == s.simulationSpace && 77 == s.maxParticles && s.hasBounds && Near( 3.0f, s.boundsMax[ 2 ] ) );
            ASSERT_TRUE( Near( 12.0f, s.rateOverTime.GetAverage() ) && 1 == s.bursts.size() && Near( 0.25f, s.explosiveness ) );
            ASSERT_TRUE( 1 == s.bursts.size() && 0 == s.bursts[ 0 ].cycles && Near( 7.0f, s.bursts[ 0 ].count.GetAverage() ) && Near( 0.5f, s.bursts[ 0 ].interval ) );
            ASSERT_TRUE( PARTICLEVALUE_RANDOMBETWEENCONSTANTS == s.startLifetime.mode && Near( 2.0f, s.startLifetime.GetMaximum() ) && Near( -45.0f, s.startRotation.GetMinimum() ) );
            Float32 rgba[ 4 ];
            s.startColour.Evaluate( 0.0f, 0.0f, rgba );
            ASSERT_TRUE( Near( 0.5f, rgba[ 1 ] ) );
            ASSERT_TRUE( PARTICLESHAPE_CONE == s.shape.type && Near( 30.0f, s.shape.angle ) && Near( 2.0f, s.shape.position[ 2 ] ) && Near( 4.0f, s.shape.boxSize[ 0 ] ) && PARTICLEDIRECTION_OUTWARD == s.shape.directionMode );
            ASSERT_TRUE( Near( -9.81f, s.gravity[ 1 ] ) && s.velocity.enabled && Near( 1.0f, s.velocity.z.GetAverage() ) );
            ASSERT_TRUE( s.sizeOverLifetimeEnabled && Near( 0.5f, s.sizeOverLifetime.Evaluate( 0.5f, 0.0f ) ) );
            ASSERT_TRUE( s.colourOverLifetimeEnabled && !s.colourOverLifetime.IsConstantOverTime() );
            ASSERT_TRUE( s.textureSheet.enabled && 4 == s.textureSheet.tilesX && 2 == s.textureSheet.tilesY );
            ASSERT_TRUE( PARTICLERENDER_STRETCHED == s.renderer.mode && 1 == s.renderer.material && PARTICLEBLEND_ADDITIVE == s.renderer.blendMode && Near( 2.0f, s.renderer.quadSize[ 1 ] ) && Near( 0.5f, s.renderer.pivot[ 1 ] ) );
            ASSERT_TRUE( 1 == s.subEmitters.size() && 1 == s.subEmitters[ 0 ].system && PARTICLETRIGGER_DEATH == s.subEmitters[ 0 ].trigger );
            ASSERT_TRUE( "GPUParticles3D" == s.source.GetAttributeValueOrChildValueByName( "type" ).AsString() );
            ASSERT_TRUE( CAssetParticleSystem::IsSubEmitter( systems, 1 ) && !CAssetParticleSystem::IsSubEmitter( systems, 0 ) );
        }

        // a scene without particle systems keeps the sidecar it had
        CAssetScene plainScene;
        plainScene.AddNode( "Root" );
        CORE::CDataNode plainSidecar;
        CConversionSidecar::Build( plainScene, entry, CORE::CString::StringVector(), "licence", context, "intermediate", plainSidecar );
        ASSERT_TRUE( GUCEF_NULL == plainSidecar.FindChild( "particles" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Particle system references: materials and meshes used only by particles stay, removals and merges remap them" )
    try
    {
        CAssetScene scene;
        scene.AddNode( "Root" );
        CAssetMaterial unused;
        unused.name = "Unused";
        CAssetMaterial flame;
        flame.name = "Flame";
        scene.materials.push_back( unused );
        scene.materials.push_back( flame );
        scene.materials.push_back( flame );
        CAssetMesh rock;
        rock.name = "Rock";
        scene.meshes.push_back( rock );
        CAssetParticleSystem system;
        system.renderer.material = 1;
        system.renderer.trailMaterial = 2;
        system.renderer.mesh = 0;
        scene.particleSystems.push_back( system );

        ASSERT_TRUE( scene.IsMeshUsed( 0 ) && 0 == scene.RemoveUnusedMeshes() );
        ASSERT_TRUE( 1 == scene.RemoveUnusedMaterials() );
        ASSERT_TRUE( 2 == scene.materials.size() && 0 == scene.particleSystems[ 0 ].renderer.material && 1 == scene.particleSystems[ 0 ].renderer.trailMaterial );
        scene.ReplaceMaterial( 1, 0 );
        ASSERT_TRUE( 0 == scene.particleSystems[ 0 ].renderer.trailMaterial );
        ASSERT_TRUE( 1 == scene.RemoveUnusedMaterials() && 1 == scene.materials.size() );
        CAssetScene::TUInt32Set meshes;
        meshes.insert( 0 );
        ASSERT_TRUE( 1 == scene.RemoveMeshes( meshes ) && -1 == scene.particleSystems[ 0 ].renderer.mesh );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Particle system normalisation: Unity's left handed vectors, velocity curves and shape rotation become right handed; centimetres become metres" )
    try
    {
        CAssetScene scene;
        scene.coordinateSystem = MATH::CCoordinateSystem::Unity();
        const UInt32 node = scene.AddNode( "Emitter" );
        CAssetParticleSystem system;
        system.shape.position[ 2 ] = 2.0f;
        system.shape.direction[ 0 ] = 0.0f; system.shape.direction[ 1 ] = 0.0f; system.shape.direction[ 2 ] = 1.0f;
        // the shape turned +90 degrees about X: its axis (+Y) points along +Z in Unity
        const Float32 h = 0.70710678f;
        system.shape.rotation[ 0 ] = h; system.shape.rotation[ 3 ] = h;
        system.gravity[ 1 ] = -9.81f;
        system.velocity.enabled = true;
        system.velocity.z = CAssetParticleCurve( 4.0f );
        system.startSpeed = CAssetParticleCurve( 3.0f );
        scene.particleSystems.push_back( system );
        scene.nodes[ node ].particleSystem = 0;

        CConversionReport report;
        report.SelectAsset( "particles.stub" );
        ASSERT_TRUE( CAssetNormaliser::Normalise( scene, report ) );
        const CAssetParticleSystem& converted = scene.particleSystems[ 0 ];
        ASSERT_TRUE( Near( -2.0f, converted.shape.position[ 2 ] ) && Near( -1.0f, converted.shape.direction[ 2 ] ) );
        ASSERT_TRUE( Near( -9.81f, converted.gravity[ 1 ] ) && Near( -4.0f, converted.velocity.z.GetAverage() ) && Near( 3.0f, converted.startSpeed.GetAverage() ) );
        Float32 axis[ 3 ];
        converted.shape.GetAxis( axis );
        ASSERT_TRUE( Near( 0.0f, axis[ 0 ], 1e-3f ) && Near( 0.0f, axis[ 1 ], 1e-3f ) && Near( -1.0f, axis[ 2 ], 1e-3f ) );

        CAssetScene centimetres;
        const MATH::CCoordinateSystem metric = MATH::CCoordinateSystem::GucefDefault();
        centimetres.coordinateSystem = MATH::CCoordinateSystem( metric.GetRightAxis(), metric.GetUpAxis(), metric.GetForwardAxis(), 100.0 );
        centimetres.AddNode( "Emitter" );
        CAssetParticleSystem small;
        small.startSize = CAssetParticleCurve( 50.0f );
        small.shape.radius = 200.0f;
        small.shape.boxSize[ 0 ] = 100.0f;
        small.gravity[ 1 ] = -981.0f;
        centimetres.particleSystems.push_back( small );
        ASSERT_TRUE( CAssetNormaliser::Normalise( centimetres, report ) );
        const CAssetParticleSystem& metres = centimetres.particleSystems[ 0 ];
        ASSERT_TRUE( Near( 0.5f, metres.startSize.GetAverage() ) && Near( 2.0f, metres.shape.radius ) && Near( 1.0f, metres.shape.boxSize[ 0 ] ) && Near( -9.81f, metres.gravity[ 1 ], 1e-3f ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}
