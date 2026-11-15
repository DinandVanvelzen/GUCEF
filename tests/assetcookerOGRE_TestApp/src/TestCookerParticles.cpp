/*
 *  assetcookerOGRE_TestApp: Test application for the Ogre 1.x cooker plugin
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

#include "Ogre.h"
#include "OgreParticleSystemManager.h"
#include "OgreParticleEmitter.h"
#include "OgreParticleAffector.h"
#include "OgreCodec.h"

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#include "TestCookerSupport.h"
#include "TestCookerParticles.h"

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

const Ogre::String ParticleGroup = "CookerParticles";
const CORE::CString CampfireFolder = "pack/Fx/Campfire/";

/** A campfire: flames on a disc with a sprite sheet and a fading colour, sparks in bursts, ash that a sub-emitter starts */
void
BuildCampfireScene( CAssetScene& scene, const CORE::CString& textureFolder )
{
    const UInt32 root = scene.AddNode( "Campfire" );
    const UInt32 flames = scene.AddNode( "Flames", (Int32) root );
    const UInt32 sparks = scene.AddNode( "Sparks", (Int32) root );
    const UInt32 ash = scene.AddNode( "Ash", (Int32) root );
    scene.nodes[ sparks ].transform.position = MATH::TFloat32Transform3D::TPositionVector( 0.0f, 0.5f, 0.0f );

    CAssetTexture sheet;
    sheet.name = "fire.png";
    sheet.sources.push_back( textureFolder + "/fire.png" );
    scene.textures.push_back( sheet );
    CAssetMaterial flameMaterial;
    flameMaterial.name = "FlameSheet";
    flameMaterial.baseColorTexture = CAssetTextureReference( 0 );
    scene.materials.push_back( flameMaterial );

    CAssetParticleSystem fire;
    fire.name = "Flames";
    fire.duration = 2.0f;
    fire.prewarm = 2.0f;
    fire.simulationSpeed = 2.0f;
    fire.maxParticles = 30;
    fire.rateOverTime = CAssetParticleCurve( 10.0f );
    fire.startLifetime = CAssetParticleCurve::RandomBetween( 1.0f, 2.0f );
    fire.startSpeed = CAssetParticleCurve( 0.5f );
    fire.startSize = CAssetParticleCurve::RandomBetween( 0.5f, 1.5f );
    fire.shape.type = PARTICLESHAPE_CIRCLE;
    fire.shape.radius = 0.5f;
    fire.shape.directionMode = PARTICLEDIRECTION_FIXED;
    fire.shape.spreadAngle = 10.0f;
    fire.gravity[ 1 ] = 1.0f;
    CAssetParticleCurve::TKeyVector shrink;
    shrink.push_back( CAssetParticleCurve::SKey( 0.0f, 1.0f, -1.0f, -1.0f ) );
    shrink.push_back( CAssetParticleCurve::SKey( 1.0f, 0.0f, -1.0f, -1.0f ) );
    fire.sizeOverLifetimeEnabled = true;
    fire.sizeOverLifetime = CAssetParticleCurve::Curve( shrink );
    CAssetParticleGradient::SGradient fade;
    const Float32 yellow[ 4 ] = { 1.0f, 1.0f, 0.0f, 1.0f };
    const Float32 red[ 4 ] = { 1.0f, 0.0f, 0.0f, 0.5f };
    const Float32 gone[ 4 ] = { 0.2f, 0.0f, 0.0f, 0.0f };
    fade.AddKey( 0.0f, yellow );
    fade.AddKey( 0.5f, red );
    fade.AddKey( 1.0f, gone );
    fire.colourOverLifetimeEnabled = true;
    fire.colourOverLifetime = CAssetParticleGradient::Gradient( fade );
    fire.textureSheet.enabled = true;
    fire.textureSheet.tilesX = 4;
    fire.textureSheet.tilesY = 4;
    fire.renderer.material = 0;
    fire.renderer.blendMode = PARTICLEBLEND_ADDITIVE;
    fire.renderer.pivot[ 1 ] = 0.5f;
    fire.subEmitters.push_back( CAssetParticleSystem::SSubEmitter() );
    fire.subEmitters.back().system = 2;
    fire.subEmitters.back().trigger = PARTICLETRIGGER_DEATH;
    scene.particleSystems.push_back( fire );
    scene.nodes[ flames ].particleSystem = 0;

    CAssetParticleSystem burst;
    burst.name = "Sparks";
    burst.rateOverTime = CAssetParticleCurve( 0.0f );
    CAssetParticleSystem::SBurst puff;
    puff.count = CAssetParticleCurve( 8.0f );
    puff.cycles = 0;
    puff.interval = 1.5f;
    burst.bursts.push_back( puff );
    burst.shape.type = PARTICLESHAPE_SPHERE;
    burst.shape.radius = 0.1f;
    burst.shape.directionMode = PARTICLEDIRECTION_OUTWARD;
    burst.startRotation = CAssetParticleCurve::RandomBetween( 0.0f, 90.0f );
    burst.angularVelocity = CAssetParticleCurve( 45.0f );
    burst.damping = CAssetParticleCurve( 0.5f );
    burst.renderer.mode = PARTICLERENDER_STRETCHED;
    burst.renderer.lengthScale = 2.0f;
    burst.renderer.material = 0;
    scene.particleSystems.push_back( burst );
    scene.nodes[ sparks ].particleSystem = 1;

    CAssetParticleSystem embers;
    embers.name = "Ash";
    embers.renderer.material = 0;
    scene.particleSystems.push_back( embers );
    scene.nodes[ ash ].particleSystem = 2;

    CAssetPrefab prefab;
    prefab.name = "Campfire";
    prefab.rootNode = root;
    scene.prefabs.push_back( prefab );
}

/** The name of the n-th template of a particle script */
Ogre::String
TemplateName( const CORE::CString& script, UInt32 index )
{
    const CORE::CString::StringVector lines = script.ParseElements( '\n', false );
    UInt32 found = 0;
    for ( size_t l=0; l<lines.size(); ++l )
    {
        if ( !lines[ l ].StartsWith( "particle_system \"" ) )
            continue;
        if ( found++ == index )
            return lines[ l ].SubstrFromRange( 17, lines[ l ].Length() - 1 ).STL_String();
    }
    return Ogre::BLANKSTRING;
}

bool
HasAffector( Ogre::ParticleSystem* system, const Ogre::String& type )
{
    for ( unsigned short a=0; a<system->getNumAffectors(); ++a )
        if ( type == system->getAffector( a )->getType() )
            return true;
    return false;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformCookerParticlesTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetcookerOGRE.Particles" );

    CConversionReport report;
    CORE::CString script;
    bool cooked = false;

    GUCEF_TESTFW_TESTCASE( "Particles: a particle script and particle materials are cooked; the colour is reduced to the six stages of Ogre" )
    try
    {
        MakeWorkFolder( "particles" );
        ASSERT_TRUE( WriteTestPng( "work/particles/src/tex/fire.png", 16, 16, 255, 200, 100, 255 ) );
        CAssetScene scene;
        BuildCampfireScene( scene, "work/particles/src/tex" );
        CCookSettings settings;
        CCookSettings::SAsset asset;
        ASSERT_TRUE( MakeIntermediate( scene, "particles", "Fx/Campfire.tscn", asset ) );
        settings.assets.push_back( asset );
        cooked = CookIntermediate( "particles", settings, report );
        ASSERT_TRUE( cooked );
        ASSERT_FALSE( report.HasFailures() );

        const CORE::CString scriptPath = GetProfileDir( "particles" ) + "/" + CampfireFolder + "Campfire.particle";
        ASSERT_TRUE( VfsFileExists( scriptPath ) && ReportHasFile( report, scriptPath ) );
        script = ReadVfsText( scriptPath );
        ASSERT_TRUE( script.HasSubstr( "emitter Ellipsoid" ) >= 0 && script.HasSubstr( "texture_sheet_size 4 4" ) >= 0 && script.HasSubstr( "billboard_origin bottom_center" ) >= 0 );
        ASSERT_TRUE( script.HasSubstr( "affector ColourInterpolator" ) >= 0 && script.HasSubstr( "affector TextureAnimator" ) >= 0 && script.HasSubstr( "affector Scaler" ) >= 0 );
        // the simulation speed 2 halves the lifetimes and doubles the rate; the burst is a single shot every 1.5 s
        ASSERT_TRUE( script.HasSubstr( "time_to_live_max 1" ) >= 0 && script.HasSubstr( "emission_rate 20" ) >= 0 );
        ASSERT_TRUE( script.HasSubstr( "duration -1" ) >= 0 && script.HasSubstr( "repeat_delay 1.5" ) >= 0 && script.HasSubstr( "billboard_type oriented_self" ) >= 0 );

        const CORE::CString materials = ReadVfsText( GetProfileDir( "particles" ) + "/" + CampfireFolder + "Campfire.material" );
        ASSERT_TRUE( materials.HasSubstr( "scene_blend src_alpha one" ) >= 0 && materials.HasSubstr( "diffuse vertexcolour" ) >= 0 && materials.HasSubstr( "depth_write off" ) >= 0 );
        ASSERT_TRUE( materials.HasSubstr( "tex_address_mode clamp" ) >= 0 );

        // what Ogre cannot hold is reported
        ASSERT_TRUE( ReportHas( report, REPORTITEM_SKIPPED, "sub-emitters have no counterpart" ) );
        ASSERT_TRUE( ReportHas( report, REPORTITEM_SKIPPED, "damping" ) );
        ASSERT_TRUE( ReportHas( report, REPORTITEM_SKIPPED, "not attached to its node" ) );
        ASSERT_TRUE( ReportHas( report, REPORTITEM_APPROXIMATION, "prewarm" ) );

        // the colour of the flames has its stages; the sparks keep the start colour of their emitter
        ASSERT_TRUE( script.HasSubstr( "time1 0.5" ) >= 0 && script.HasSubstr( "colour_range_start" ) >= 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Particles: the cooked script loads with the ParticleFX plugin and the systems are made from its templates; the DotScene attaches them" )
    try
    {
        ASSERT_TRUE( cooked );
        ASSERT_TRUE( UseOgreGroup( ParticleGroup, GetProfileDir( "particles" ) ) );
        const Ogre::String flames = TemplateName( script, 0 );
        const Ogre::String sparks = TemplateName( script, 1 );
        ASSERT_TRUE( !flames.empty() && GUCEF_NULL != Ogre::ParticleSystemManager::getSingleton().getTemplate( flames ) );
        ASSERT_TRUE( !sparks.empty() && GUCEF_NULL != Ogre::ParticleSystemManager::getSingleton().getTemplate( sparks ) );

        Ogre::SceneManager* sceneManager = Ogre::Root::getSingleton().createSceneManager();
        Ogre::ParticleSystem* system = sceneManager->createParticleSystem( "FlamesTest", flames );
        ASSERT_TRUE( GUCEF_NULL != system && 1 == system->getNumEmitters() && 30 == system->getParticleQuota() );
        if ( GUCEF_NULL != system && 1 == system->getNumEmitters() )
        {
            const Ogre::ParticleEmitter* emitter = system->getEmitter( 0 );
            ASSERT_TRUE( "Ellipsoid" == emitter->getType() && Near( (float) emitter->getEmissionRate(), 20.0f ) );
            ASSERT_TRUE( Near( (float) emitter->getMinTimeToLive(), 0.5f ) && Near( (float) emitter->getMaxTimeToLive(), 1.0f ) && Near( (float) emitter->getParticleVelocity(), 1.0f ) );
            ASSERT_TRUE( HasAffector( system, "ColourInterpolator" ) && HasAffector( system, "Scaler" ) && HasAffector( system, "TextureAnimator" ) && HasAffector( system, "LinearForce" ) );
            ASSERT_TRUE( "billboard" == system->getRendererName() && system->getMaterialName().find( "Flames" ) != Ogre::String::npos );
            // the stages of the colour: the ends and the middle key of the gradient (red at half alpha), the rest at the end
            for ( unsigned short a=0; a<system->getNumAffectors(); ++a )
            {
                Ogre::ParticleAffector* affector = system->getAffector( a );
                if ( "ColourInterpolator" != affector->getType() )
                    continue;
                ASSERT_TRUE( Near( Ogre::StringConverter::parseReal( affector->getParameter( "time1" ) ), 0.5f ) );
                const Ogre::ColourValue middle = Ogre::StringConverter::parseColourValue( affector->getParameter( "colour1" ) );
                ASSERT_TRUE( Near( middle.r, 1.0f ) && Near( middle.g, 0.0f ) && Near( middle.a, 0.5f ) );
                ASSERT_TRUE( Near( Ogre::StringConverter::parseReal( affector->getParameter( "time5" ) ), 1.0f ) );
            }
        }
        Ogre::ParticleSystem* burst = sceneManager->createParticleSystem( "SparksTest", sparks );
        ASSERT_TRUE( GUCEF_NULL != burst && 1 == burst->getNumEmitters() && HasAffector( burst, "Rotator" ) );
        if ( GUCEF_NULL != burst && 1 == burst->getNumEmitters() )
            ASSERT_TRUE( Near( (float) burst->getEmitter( 0 )->getEmissionRate(), 8.0f ) && Near( (float) burst->getEmitter( 0 )->getMaxRepeatDelay(), 1.5f ) );

        // the scene of the asset: the two attached systems on their nodes, the prewarm in the user data, the sub-emitter left out
        const CORE::CString sceneText = ReadVfsText( GetProfileDir( "particles" ) + "/" + CampfireFolder + "Campfire.scene" );
        ASSERT_TRUE( sceneText.HasSubstr( "<particleSystem name=\"Flames\"" ) >= 0 && sceneText.HasSubstr( "particlePrewarm" ) >= 0 );
        ASSERT_TRUE( sceneText.HasSubstr( "<particleSystem name=\"Ash\"" ) < 0 );
        Ogre::ResourceGroupManager::getSingleton().setWorldResourceGroupName( ParticleGroup );
        Ogre::SceneManager* loaded = Ogre::Root::getSingleton().createSceneManager();
        Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton().openResource( ( CampfireFolder + "Campfire.scene" ).STL_String(), ParticleGroup );
        Ogre::Codec::getCodec( "scene" )->decode( stream, Ogre::Any( loaded->getRootSceneNode() ) );
        ASSERT_TRUE( loaded->hasParticleSystem( "Flames" ) && loaded->hasParticleSystem( "Sparks" ) && !loaded->hasParticleSystem( "Ash" ) );
        if ( loaded->hasParticleSystem( "Flames" ) && loaded->hasSceneNode( "Flames" ) )
        {
            ASSERT_TRUE( loaded->getParticleSystem( "Flames" )->getParentSceneNode() == loaded->getSceneNode( "Flames" ) );
            const Ogre::Any prewarm = loaded->getSceneNode( "Flames" )->getUserObjectBindings().getUserAny( "particlePrewarm" );
            ASSERT_TRUE( prewarm.has_value() && Near( (float) Ogre::any_cast< Ogre::Real >( prewarm ), 1.0f ) );
        }
        Ogre::Root::getSingleton().destroySceneManager( loaded );
        Ogre::Root::getSingleton().destroySceneManager( sceneManager );
        DropOgreGroup( ParticleGroup );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}
