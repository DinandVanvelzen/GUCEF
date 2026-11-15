/*
 *  AssetViewerLib_TestApp: Test application for the asset viewer library
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

#include <Ogre.h>

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#include "AssetViewerLib.h"

#include "TestAssetViewerSupport.h"
#include "TestAssetViewerAssets.h"
#include "TestAssetViewerParticles.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETVIEWER;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const UInt32 TileSize = 96;

bool
Near( Float32 a, Float32 b, Float32 tolerance = 1e-4f )
{
    return a - b <= tolerance && b - a <= tolerance;
}

/** Unlit, additive, the particle colour as the colour: what the cooker writes for particles, without a texture */
const char* const GlowMaterial =
    "material \"fx/Glow\"\n{\n    technique\n    {\n        pass\n        {\n            lighting off\n            diffuse vertexcolour\n"
    "            scene_blend src_alpha one\n            depth_write off\n            cull_hardware none\n        }\n    }\n}\n";

/** Two templates: a fountain of points in a box, a slow ring of larger ones */
const char* const GlowParticles =
    "// fountain\nparticle_system \"fx/Fountain\"\n{\n    quota 200\n    material \"fx/Glow\"\n    particle_width 0.3\n    particle_height 0.3\n"
    "    renderer billboard\n    billboard_type point\n\n    emitter Box\n    {\n        direction 0 1 0\n        up 1 0 0\n        width 0.5\n        height 0.5\n        depth 0.2\n"
    "        angle 20\n        emission_rate 60\n        velocity_min 1\n        velocity_max 2\n        time_to_live_min 1.5\n        time_to_live_max 2\n"
    "        colour_range_start 1 1 0.5 1\n        colour_range_end 1 0.5 0 1\n    }\n    affector LinearForce\n    {\n        force_vector 0 -1 0\n"
    "        force_application add\n    }\n}\n\n"
    "// ring\nparticle_system \"fx/Ring\"\n{\n    quota 50\n    material \"fx/Glow\"\n    particle_width 0.5\n    particle_height 0.5\n    renderer billboard\n\n"
    "    emitter HollowEllipsoid\n    {\n        direction 0 0 1\n        up 0 1 0\n        width 2\n        height 2\n        depth 0.1\n        inner_width 0.8\n"
    "        inner_height 0.8\n        inner_depth 0.8\n        angle 0\n        emission_rate 20\n        velocity_min 0\n        velocity_max 0\n"
    "        time_to_live_min 3\n        time_to_live_max 3\n    }\n    affector ColourInterpolator\n    {\n        time0 0\n        colour0 0 1 1 1\n"
    "        time1 1\n        colour1 0 0 1 0\n    }\n}\n";

/** A scene with a node of the fountain, which starts warm */
CORE::CString
BuildParticleScene( void )
{
    return "<scene formatVersion=\"1.1\">\n  <nodes>\n    <node name=\"Well\">\n      <position x=\"0\" y=\"0\" z=\"0\" />\n"
           "      <particleSystem name=\"WellFountain\" template=\"fx/Fountain\" />\n"
           "      <userData>\n        <property type=\"float\" name=\"particlePrewarm\" data=\"1.5\" />\n      </userData>\n    </node>\n  </nodes>\n</scene>\n";
}

bool
WriteParticleFolder( const CORE::CString& dir )
{
    return WriteTestFile( dir + "/fx/glow.material", GlowMaterial ) && WriteTestFile( dir + "/fx/glow.particle", GlowParticles ) &&
           WriteTestFile( dir + "/fx/well.scene", BuildParticleScene() );
}

/** The brightest pixel of a picture, as the sum of its colour channels */
UInt32
Brightest( const IMAGE::TPixelMapPtr& pixels )
{
    UInt32 best = 0;
    if ( pixels.IsNULL() )
        return 0;
    for ( UInt32 y=0; y<pixels->GetHeightInPixels(); y+=2 )
        for ( UInt32 x=0; x<pixels->GetWidthInPixels(); x+=2 )
        {
            UInt8 rgba[ 4 ];
            GetPixel( pixels, (Int32) x, (Int32) y, rgba );
            const UInt32 sum = (UInt32) rgba[ 0 ] + rgba[ 1 ] + rgba[ 2 ];
            best = sum > best ? sum : best;
        }
    return best;
}

}

/*-------------------------------------------------------------------------*/

static void
TestParticleScript( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Particle preview: a particle script parses with ParticleFX, its templates play side by side, a scrub goes back to the start, the particles are drawn and captured" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        const CORE::CString dir = MakeCleanTestDir( "particles_script" );
        ASSERT_TRUE( WriteParticleFolder( dir ) );
        CAssetMounter mounter;
        ASSERT_TRUE( mounter.Mount( "avfx", dir ) );
        ASSERT_TRUE( ASSETKIND_PARTICLE == CAssetTypeResolver::GetKind( "avfx/fx/glow.particle" ) && "Particles" == CAssetTypeResolver::GetKindName( ASSETKIND_PARTICLE ) );

        CAssetCapture capture( AsOgreWindow( GetMainTestWindow() ) );
        ASSERT_TRUE( capture.Initialize( TileSize ) );
        CORE::CString relative;
        const CORE::CString group = mounter.GetResourceGroup( "avfx/fx/glow.particle", relative );
        CORE::CString::StringVector templates;
        ASSERT_TRUE( CParticlePreview::ParseParticleScript( group, relative, templates ) );
        ASSERT_TRUE( 2 == templates.size() && "fx/Fountain" == templates[ 0 ] && "fx/Ring" == templates[ 1 ] );

        CViewerStage& stage = capture.GetStage();
        {
            CParticlePreview preview;
            ASSERT_TRUE( 2 == preview.CreateFromTemplates( stage.GetSceneManager(), stage.GetContentNode(), templates ) );
            ASSERT_TRUE( 0 == preview.GetParticleCount() && preview.GetLengthInMs() >= 2000 );
            preview.SetTime( 2.0f );
            const UInt32 running = preview.GetParticleCount();
            ASSERT_TRUE( running > 60 && running <= 250 && Near( 2.0f, preview.GetTime() ) );
            COrbitController::SVector3 center;
            Float32 radius = 0.0f;
            ASSERT_TRUE( preview.GetBoundingSphere( center, radius ) && radius > 0.5f );
            preview.Advance( 0.5f );
            ASSERT_TRUE( preview.GetParticleCount() > 0 );
            preview.SetTime( 0.0f );
            ASSERT_TRUE( 0 == preview.GetParticleCount() );
            preview.SetTime( 2.0f );

            CORE::CDataNode data;
            preview.WriteInspectorData( data );
            const CORE::CDataNode* particles = data.FindChild( "Particles" );
            ASSERT_TRUE( GUCEF_NULL != particles && "2" == particles->GetAttributeValue( "Systems" ).AsString() && particles->GetAttributeValue( "Particles" ).AsUInt32() > 0 );

            // drawn: additive particles light up the dark stage
            COrbitController controller;
            controller.SetBounds( center, radius );
            stage.ApplyController( controller );
            IMAGE::TPixelMapPtr picture;
            ASSERT_TRUE( capture.RenderStage( picture ) );
            DumpPicture( picture, "particles_" + CORE::CString( GetTestRenderer() ) );
            ASSERT_TRUE( Brightest( picture ) > 400 );
        }

        // the capture of the script: the templates after a few seconds
        IMAGE::TPixelMapPtr sheet;
        ASSERT_TRUE( capture.CaptureContactSheet( mounter, "avfx/fx/glow.particle", sheet ) );
        DumpPicture( sheet, "particles_sheet_" + CORE::CString( GetTestRenderer() ) );
        ASSERT_TRUE( Brightest( sheet ) > 400 );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestParticleScene( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Particle preview: a DotScene with a particle system and the scripts next to it loads, the system is collected warm (prewarm user data) and described in the inspector" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        const CORE::CString dir = MakeCleanTestDir( "particles_scene" );
        ASSERT_TRUE( WriteParticleFolder( dir ) );
        CAssetMounter mounter;
        ASSERT_TRUE( mounter.Mount( "avfxscene", dir ) );
        CAssetCapture capture( AsOgreWindow( GetMainTestWindow() ) );
        ASSERT_TRUE( capture.Initialize( TileSize ) );
        CSceneView scene( capture.GetStage() );
        CORE::CString relative;
        const CORE::CString group = mounter.GetResourceGroup( "avfxscene/fx/well.scene", relative );
        ASSERT_TRUE( scene.Load( group, relative ) && GUCEF_NULL != scene.GetRoot() );
        {
            CParticlePreview preview;
            ASSERT_TRUE( 1 == preview.Collect( scene.GetRoot() ) );
            // 1.5 s of prewarm at 60 per second
            ASSERT_TRUE( preview.GetParticleCount() > 50 );
            ASSERT_TRUE( scene.Select( "/Well/WellFountain.particles" ) );
            CORE::CDataNode data;
            scene.WriteInspectorData( data );
            const CORE::CDataNode* object = data.FindChild( "Object" );
            ASSERT_TRUE( GUCEF_NULL != object && "particles" == object->GetAttributeValue( "Kind" ).AsString() && "200" == object->GetAttributeValue( "Quota" ).AsString() );
            ASSERT_TRUE( GUCEF_NULL != object && object->GetAttributeValue( "Particles" ).AsUInt32() > 50 );
            preview.Clear();
        }
        scene.Unload();

        IMAGE::TPixelMapPtr sheet;
        ASSERT_TRUE( capture.CaptureContactSheet( mounter, "avfxscene/fx/well.scene", sheet ) );
        DumpPicture( sheet, "particles_scene_" + CORE::CString( GetTestRenderer() ) );
        ASSERT_TRUE( Brightest( sheet ) > 400 );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestParticleShell( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Particle preview: the shell opens a particle script with a 'particles' clip that plays, pauses and scrubs, and counts the particles in the inspector" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        const CORE::CString dir = MakeCleanTestDir( "particles_shell" );
        ASSERT_TRUE( WriteParticleFolder( dir ) );
        CAssetMounter mounter;
        ASSERT_TRUE( mounter.Mount( "avfxshell", dir ) );
        GUI::TGuiContextPtr contextPtr = CreateViewerContext();
        GUI::CGUIContext* context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        CAssetCapture* capture = GUCEF_NEW CAssetCapture( AsOgreWindow( GetMainTestWindow() ) );
        CViewerShell* shell = GUCEF_NEW CViewerShell( mounter );
        if ( capture->Initialize( 64 ) && shell->Initialize( *context, capture, MakeCleanTestDir( "particles_shell_cache" ) ) )
        {
            ASSERT_TRUE( shell->OpenAsset( "/avfxshell/fx/glow.particle" ) );
            ASSERT_TRUE( ASSETKIND_PARTICLE == shell->GetShownKind() && 2 == shell->GetParticlePreview().GetSystemCount() );
            GUI::CTimelineForm* timeline = shell->GetTimeline();
            ASSERT_TRUE( 1 == timeline->GetClips().size() && "particles" == timeline->GetClips()[ 0 ].name && timeline->IsPlaying() );

            // playing: the time line runs the systems
            timeline->Update( 500 );
            timeline->Update( 500 );
            ASSERT_TRUE( shell->GetParticlePreview().GetParticleCount() > 0 && Near( 1.0f, shell->GetParticlePreview().GetTime(), 0.05f ) );

            // paused, then scrubbed back to the start
            timeline->Pause();
            ASSERT_FALSE( shell->GetParticlePreview().IsPlaying() );
            ASSERT_TRUE( timeline->SetTime( 0 ) );
            ASSERT_TRUE( 0 == shell->GetParticlePreview().GetParticleCount() );
            ASSERT_TRUE( timeline->SetTime( 1500 ) );
            ASSERT_TRUE( shell->GetParticlePreview().GetParticleCount() > 0 );

            // the inspector counts them (refreshed by the changes of the time)
            const CORE::CDataNode& data = shell->GetInspector()->GetData();
            ASSERT_TRUE( GUCEF_NULL != data.FindChild( "Particles" ) );

            shell->CloseAsset();
            ASSERT_TRUE( 0 == shell->GetParticlePreview().GetSystemCount() && timeline->GetClips().empty() );
        }
        else
        {
            ASSERT_TRUE( false );
        }
        shell->Shutdown();
        GUCEF_DELETE shell;
        contextPtr.Unlink();
        capture->Shutdown();
        GUCEF_DELETE capture;
        mounter.UnmountAll();
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformAssetViewerParticleTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AssetViewerLib.Particles" );

    TestParticleScript();
    TestParticleScene();
    TestParticleShell();
}
