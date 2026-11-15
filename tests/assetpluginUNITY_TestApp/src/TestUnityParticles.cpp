/*
 *  assetpluginUNITY_TestApp: Test application for the Unity importer plugin
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

#include "assetpluginUNITY_CUnityImporter.h"
#include "assetpluginUNITY_CUnityParticleReader.h"

#include "TestUnitySupport.h"
#include "TestUnityContent.h"
#include "TestUnityParticles.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETCONVERTER;
using namespace GUCEF::ASSETPLUGIN::UNITY;

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

typedef CORE::CString TText;

const char* const GuidSparkMaterial = "a1000000000000000000000000000001";
const char* const GuidSparkPrefab = "a1000000000000000000000000000002";

/** A curve key at the indentation of a member of a module */
TText
Key( const TText& indent, const char* time, const char* value, const char* inSlope = "0", const char* outSlope = "0" )
{
    return indent + "- serializedVersion: 3\n" + indent + "  time: " + time + "\n" + indent + "  value: " + value + "\n" + indent + "  inSlope: " + inSlope + "\n" +
           indent + "  outSlope: " + outSlope + "\n" + indent + "  tangentMode: 0\n" + indent + "  weightedMode: 0\n";
}

/** A MinMaxCurve member: its state, scalars and the keys of the upper curve (the lower curve is flat at 0) */
TText
MinMaxCurve( const TText& indent, const char* name, int state, const char* scalar, const char* minScalar, const TText& maxKeys )
{
    const TText inner = indent + "  ";
    TText text = indent + name + ":\n" + inner + "serializedVersion: 2\n" + inner + "minMaxState: " + CORE::Int32ToString( state ) + "\n" +
                 inner + "scalar: " + scalar + "\n" + inner + "minScalar: " + minScalar + "\n";
    text += inner + "maxCurve:\n" + inner + "  serializedVersion: 2\n" + inner + "  m_Curve:\n" +
            ( maxKeys.IsNULLOrEmpty() ? Key( inner + "  ", "0", "1" ) + Key( inner + "  ", "1", "1" ) : maxKeys ) +
            inner + "  m_PreInfinity: 2\n" + inner + "  m_PostInfinity: 2\n" + inner + "  m_RotationOrder: 4\n";
    text += inner + "minCurve:\n" + inner + "  serializedVersion: 2\n" + inner + "  m_Curve:\n" + Key( inner + "  ", "0", "0" ) + Key( inner + "  ", "1", "0" ) +
            inner + "  m_PreInfinity: 2\n" + inner + "  m_PostInfinity: 2\n" + inner + "  m_RotationOrder: 4\n";
    return text;
}

/** A Gradient member: two colour keys and two alpha keys */
TText
GradientKeys( const TText& indent, const char* name, const char* first, const char* last, const char* lastTime )
{
    TText text = indent + name + ":\n" + indent + "  serializedVersion: 2\n" + indent + "  key0: " + first + "\n" + indent + "  key1: " + last + "\n";
    for ( int k=2; k<8; ++k )
        text += indent + "  key" + CORE::Int32ToString( k ) + ": {r: 0, g: 0, b: 0, a: 0}\n";
    text += indent + "  ctime0: 0\n" + indent + "  ctime1: " + lastTime + "\n" + indent + "  atime0: 0\n" + indent + "  atime1: " + lastTime + "\n";
    text += indent + "  m_Mode: 0\n" + indent + "  m_NumColorKeys: 2\n" + indent + "  m_NumAlphaKeys: 2\n";
    return text;
}

TText
GameObject( const char* id, const char* name, const char* transform, const char* system, const char* renderer )
{
    return "--- !u!1 &" + TText( id ) + "\nGameObject:\n  m_ObjectHideFlags: 0\n  m_CorrespondingSourceObject: {fileID: 0}\n  m_PrefabInstance: {fileID: 0}\n"
           "  m_PrefabAsset: {fileID: 0}\n  serializedVersion: 6\n  m_Component:\n  - component: {fileID: " + TText( transform ) + "}\n"
           "  - component: {fileID: " + TText( system ) + "}\n  - component: {fileID: " + TText( renderer ) + "}\n  m_Layer: 0\n  m_Name: " + TText( name ) + "\n  m_IsActive: 1\n";
}

TText
Transform( const char* id, const char* gameObject, const char* father, const TText& children )
{
    return "--- !u!4 &" + TText( id ) + "\nTransform:\n  m_ObjectHideFlags: 0\n  m_CorrespondingSourceObject: {fileID: 0}\n  m_PrefabInstance: {fileID: 0}\n"
           "  m_PrefabAsset: {fileID: 0}\n  m_GameObject: {fileID: " + TText( gameObject ) + "}\n  m_LocalRotation: {x: 0, y: 0, z: 0, w: 1}\n"
           "  m_LocalPosition: {x: 0, y: 0, z: 0}\n  m_LocalScale: {x: 1, y: 1, z: 1}\n  m_Children:" + ( children.IsNULLOrEmpty() ? TText( " []\n" ) : "\n" + children ) +
           "  m_Father: {fileID: " + TText( father ) + "}\n";
}

TText
Renderer( const char* id, const char* gameObject, int renderMode, int sortMode )
{
    return "--- !u!199 &" + TText( id ) + "\nParticleSystemRenderer:\n  serializedVersion: 6\n  m_ObjectHideFlags: 0\n  m_GameObject: {fileID: " + TText( gameObject ) + "}\n"
           "  m_Enabled: 1\n  m_Materials:\n  - {fileID: 2100000, guid: " + TText( GuidSparkMaterial ) + ", type: 2}\n  m_RenderMode: " + CORE::Int32ToString( renderMode ) + "\n"
           "  m_SortMode: " + CORE::Int32ToString( sortMode ) + "\n  m_LengthScale: 3\n  m_VelocityScale: 0.1\n  m_CameraVelocityScale: 0\n  m_RenderAlignment: 0\n"
           "  m_Pivot: {x: 0, y: 0.5, z: 0}\n  m_Flip: {x: 0, y: 0, z: 0}\n  m_Mesh: {fileID: 0}\n";
}

/** The rain (2022 layout: a cone, curves, a gradient, a sprite sheet, a sub-emitter on collision) and the splash (2017 layout of the bursts and the box) */
TText
SparkPrefab( void )
{
    const TText m = "    ";
    TText rain = "--- !u!198 &198000001\nParticleSystem:\n  m_ObjectHideFlags: 0\n  m_GameObject: {fileID: 100001}\n  serializedVersion: 8\n"
                 "  lengthInSec: 4\n  simulationSpeed: 2\n  looping: 1\n  prewarm: 1\n  playOnAwake: 1\n  moveWithTransform: 1\n  scalingMode: 1\n";
    rain += MinMaxCurve( "  ", "startDelay", 0, "0", "0", "" );
    rain += "  InitialModule:\n    serializedVersion: 3\n    enabled: 1\n";
    rain += MinMaxCurve( m, "startLifetime", 3, "2", "1", "" );
    rain += MinMaxCurve( m, "startSpeed", 0, "5", "99", "" );
    rain += "    startColor:\n      serializedVersion: 2\n      minMaxState: 2\n      minColor: {r: 1, g: 0, b: 0, a: 1}\n      maxColor: {r: 0, g: 0, b: 1, a: 1}\n";
    rain += MinMaxCurve( m, "startSize", 3, "0.2", "0.1", "" );
    rain += MinMaxCurve( m, "startRotation", 3, "3.14159265", "0", "" );
    rain += "    maxNumParticles: 250\n    size3D: 0\n    rotation3D: 0\n";
    rain += MinMaxCurve( m, "gravityModifier", 0, "2", "0", "" );
    rain += "  ShapeModule:\n    serializedVersion: 6\n    enabled: 1\n    type: 4\n    angle: 20\n    length: 5\n    radiusThickness: 1\n"
            "    m_Position: {x: 0, y: 1, z: 0}\n    m_Rotation: {x: -90, y: 0, z: 0}\n    m_Scale: {x: 1, y: 1, z: 1}\n"
            "    radius:\n      value: 0.5\n      mode: 0\n    arc:\n      value: 360\n      mode: 0\n    randomDirectionAmount: 0\n    sphericalDirectionAmount: 0\n";
    rain += "  EmissionModule:\n    enabled: 1\n    serializedVersion: 4\n";
    rain += MinMaxCurve( m, "rateOverTime", 0, "40", "7", "" );
    rain += MinMaxCurve( m, "rateOverDistance", 0, "0", "0", "" );
    rain += "    m_BurstCount: 1\n    m_Bursts:\n    - serializedVersion: 2\n      time: 0\n" + MinMaxCurve( "      ", "countCurve", 0, "12", "12", "" ) +
            "      cycleCount: 0\n      repeatInterval: 0.5\n      probability: 1\n";
    rain += "  SizeModule:\n    enabled: 1\n" + MinMaxCurve( m, "curve", 1, "1", "1", Key( "        ", "0", "1", "-1", "-1" ) + Key( "        ", "1", "0", "-1", "-1" ) ) + "    separateAxes: 0\n";
    rain += "  RotationModule:\n    enabled: 1\n" + MinMaxCurve( m, "curve", 3, "1.5707963", "-1.5707963", "" ) + "    separateAxes: 0\n";
    rain += "  ColorModule:\n    enabled: 1\n    gradient:\n      serializedVersion: 2\n      minMaxState: 1\n" +
            GradientKeys( "      ", "maxGradient", "{r: 1, g: 1, b: 1, a: 1}", "{r: 1, g: 1, b: 1, a: 0}", "65535" );
    rain += "  UVModule:\n    enabled: 1\n    mode: 0\n    timeMode: 0\n    fps: 30\n    tilesX: 4\n    tilesY: 2\n    animationType: 0\n    rowIndex: 0\n"
            "    cycles: 2\n    rowMode: 0\n" + MinMaxCurve( m, "frameOverTime", 1, "0.9999", "0.9999", Key( "        ", "0", "0", "1", "1" ) + Key( "        ", "1", "1", "1", "1" ) ) +
            MinMaxCurve( m, "startFrame", 3, "0.9999", "0", "" );
    rain += "  VelocityModule:\n    enabled: 1\n" + MinMaxCurve( m, "x", 0, "0", "0", "" ) + MinMaxCurve( m, "y", 0, "1", "0", "" ) + MinMaxCurve( m, "z", 0, "0", "0", "" ) +
            MinMaxCurve( m, "radial", 0, "0", "0", "" ) + "    inWorldSpace: 1\n";
    rain += "  NoiseModule:\n    enabled: 1\n" + MinMaxCurve( m, "strength", 0, "0.5", "0.5", "" ) + "    frequency: 2\n    octaves: 1\n" + MinMaxCurve( m, "scrollSpeed", 0, "0", "0", "" );
    rain += "  CollisionModule:\n    enabled: 1\n    type: 1\n" + MinMaxCurve( m, "m_Bounce", 0, "0.5", "0", "" );
    rain += "  SubModule:\n    enabled: 1\n    subEmitters:\n    - serializedVersion: 3\n      emitter: {fileID: 198000002}\n      type: 1\n      properties: 0\n      emitProbability: 1\n";
    rain += "  LightsModule:\n    enabled: 1\n  TrailModule:\n    enabled: 0\n";

    TText splash = "--- !u!198 &198000002\nParticleSystem:\n  m_ObjectHideFlags: 0\n  m_GameObject: {fileID: 100002}\n  serializedVersion: 5\n"
                   "  lengthInSec: 1\n  simulationSpeed: 1\n  looping: 0\n  prewarm: 0\n  playOnAwake: 1\n  moveWithTransform: 0\n";
    splash += "  InitialModule:\n    enabled: 1\n" + MinMaxCurve( m, "startLifetime", 0, "0.5", "0", "" ) + MinMaxCurve( m, "startSpeed", 0, "1", "0", "" ) +
              MinMaxCurve( m, "startSize", 0, "0.05", "0", "" ) + "    maxNumParticles: 20\n";
    splash += "  ShapeModule:\n    serializedVersion: 4\n    enabled: 1\n    type: 5\n    angle: 25\n    boxX: 2\n    boxY: 3\n    boxZ: 4\n    radius:\n      value: 1\n";
    splash += "  EmissionModule:\n    enabled: 1\n" + MinMaxCurve( m, "rateOverTime", 0, "0", "0", "" ) +
              "    m_BurstCount: 1\n    m_Bursts:\n    - time: 0\n      minCount: 4\n      maxCount: 8\n      cycleCount: 1\n      repeatInterval: 0.01\n";

    TText text = "%YAML 1.1\n%TAG !u! tag:unity3d.com,2011:\n";
    text += GameObject( "100001", "Rain", "400001", "198000001", "199000001" );
    text += Transform( "400001", "100001", "0", "  - {fileID: 400002}\n" );
    text += rain;
    text += Renderer( "199000001", "100001", 1, 1 );
    text += GameObject( "100002", "Splash", "400002", "198000002", "199000002" );
    text += Transform( "400002", "100002", "400001", "" );
    text += splash;
    text += Renderer( "199000002", "100002", 0, 0 );
    return text;
}

/** Imports the prefab of a project folder the way the pipeline does */
bool
ImportPrefab( const CORE::CString& folder, const CORE::CString& assetPath, CAssetScene& scene, CConversionReport& report )
{
    CAssetPackManifest manifest;
    manifest.id = "particles";
    manifest.source = folder;
    manifest.importer = "unity";
    manifest.intermediate = "/work/unityparticles/intermediate";
    CAssetPackContext context;
    if ( !context.Open( manifest ) )
        return false;
    CUnityImporter importer;
    if ( !importer.IndexPack( context ) )
        return false;
    scene = CAssetScene();
    CORE::CDataNode settings( "settings", GUCEF_DATATYPE_OBJECT );
    importer.GetDefaultSettings( settings );
    scene.settings = settings;
    report.SelectAsset( assetPath );
    return importer.CanImport( context.ToVfsPath( assetPath ), context ) && importer.Import( context.ToVfsPath( assetPath ), context, scene, report );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformUnityParticleTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetpluginUNITY.Particles" );

    GUCEF_TESTFW_TESTCASE( "Particles: MinMaxCurve and MinMaxGradient modes, Unity's Euler order of the shape rotation" )
    try
    {
        CORE::CDataNode curve( "curve", GUCEF_DATATYPE_OBJECT );
        curve.SetAttribute( "minMaxState", "3" );
        curve.SetAttribute( "scalar", "4" );
        curve.SetAttribute( "minScalar", "2" );
        bool weighted = false;
        const CAssetParticleCurve between = CUnityParticleReader::ReadCurve( &curve, 0.5f, weighted );
        ASSERT_TRUE( PARTICLEVALUE_RANDOMBETWEENCONSTANTS == between.mode && Near( 1.0f, between.GetMinimum() ) && Near( 2.0f, between.GetMaximum() ) );
        curve.SetAttribute( "minMaxState", "0" );
        ASSERT_TRUE( Near( 2.0f, CUnityParticleReader::ReadCurve( &curve, 0.5f, weighted ).GetAverage() ) );

        // Z, then X, then Y: (90, 90, 0) turns +Z to -X... checked against the product of the three rotations
        const Float32 euler[ 3 ] = { 0.0f, 90.0f, 0.0f };
        Float32 q[ 4 ];
        CUnityParticleReader::EulerToQuaternion( euler, q );
        ASSERT_TRUE( Near( 0.0f, q[ 0 ] ) && Near( 0.70710678f, q[ 1 ] ) && Near( 0.70710678f, q[ 3 ] ) );
        const Float32 both[ 3 ] = { 90.0f, 90.0f, 0.0f };
        CUnityParticleReader::EulerToQuaternion( both, q );
        ASSERT_TRUE( Near( 0.5f, q[ 0 ] ) && Near( 0.5f, q[ 1 ] ) && Near( -0.5f, q[ 2 ] ) && Near( 0.5f, q[ 3 ] ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Particles: a ParticleSystem with its renderer becomes a particle system of its GameObject: modules, the 2017 burst and box layout, a sub-emitter, the material's blending" )
    try
    {
        MakeWorkFolder( "unityparticles" );
        CTestUnityProject project;
        project.AddText( "Assets/Materials/Spark.mat", GuidSparkMaterial,
            "%YAML 1.1\n%TAG !u! tag:unity3d.com,2011:\n--- !u!21 &2100000\nMaterial:\n  serializedVersion: 6\n  m_Name: Spark\n"
            "  m_Shader: {fileID: 200, guid: 0000000000000000f000000000000000, type: 0}\n  m_ShaderKeywords: \n  m_SavedProperties:\n    serializedVersion: 3\n"
            "    m_TexEnvs: []\n    m_Floats:\n    - _SrcBlend: 5\n    - _DstBlend: 1\n    m_Colors:\n    - _TintColor: {r: 0.5, g: 0.25, b: 0.5, a: 0.5}\n",
            MakeMeta( GuidSparkMaterial, "NativeFormatImporter:\n  mainObjectFileID: 2100000\n  userData: \n" ) );
        project.AddText( "Assets/Prefabs/Spark.prefab", GuidSparkPrefab, SparkPrefab(), MakeMeta( GuidSparkPrefab, "PrefabImporter:\n  userData: \n" ) );
        REQUIRE( project.WriteFolder( WorkDiskPath( "unityparticles/project" ) ) );

        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportPrefab( "/work/unityparticles/project", "Assets/Prefabs/Spark.prefab", scene, report ) );
        REQUIRE( 2 == scene.particleSystems.size() );
        const Int32 rainNode = FindNode( scene, "Rain" );
        REQUIRE( rainNode >= 0 && scene.nodes[ rainNode ].particleSystem >= 0 );
        const CAssetParticleSystem& rain = scene.particleSystems[ scene.nodes[ rainNode ].particleSystem ];
        ASSERT_TRUE( "Rain" == rain.name && Near( 4.0f, rain.duration ) && Near( 4.0f, rain.prewarm ) && Near( 2.0f, rain.simulationSpeed ) && 250 == rain.maxParticles );
        ASSERT_TRUE( PARTICLESPACE_WORLD == rain.simulationSpace );
        ASSERT_TRUE( Near( 1.0f, rain.startLifetime.GetMinimum() ) && Near( 2.0f, rain.startLifetime.GetMaximum() ) && Near( 5.0f, rain.startSpeed.GetAverage() ) );
        ASSERT_TRUE( Near( 180.0f, rain.startRotation.GetMaximum(), 1e-3f ) && Near( -19.62f, rain.gravity[ 1 ], 1e-3f ) );
        Float32 rgba[ 4 ];
        rain.startColour.Evaluate( 0.0f, 1.0f, rgba );
        ASSERT_TRUE( PARTICLECOLOUR_RANDOMBETWEENCOLOURS == rain.startColour.mode && Near( 1.0f, rgba[ 2 ] ) );
        ASSERT_TRUE( PARTICLESHAPE_CONE == rain.shape.type && Near( 20.0f, rain.shape.angle ) && Near( 0.5f, rain.shape.radius ) && Near( 1.0f, rain.shape.position[ 1 ] ) );
        Float32 axis[ 3 ];
        rain.shape.GetAxis( axis );
        ASSERT_TRUE( Near( 1.0f, axis[ 1 ], 1e-4f ) && PARTICLEDIRECTION_OUTWARD == rain.shape.directionMode );
        ASSERT_TRUE( Near( 40.0f, rain.rateOverTime.GetAverage() ) && 1 == rain.bursts.size() );
        if ( 1 == rain.bursts.size() )
            ASSERT_TRUE( 0 == rain.bursts[ 0 ].cycles && Near( 12.0f, rain.bursts[ 0 ].count.GetAverage() ) && Near( 0.5f, rain.bursts[ 0 ].interval ) );
        ASSERT_TRUE( rain.sizeOverLifetimeEnabled && Near( 0.5f, rain.sizeOverLifetime.Evaluate( 0.5f, 0.0f ) ) );
        ASSERT_TRUE( Near( -90.0f, rain.angularVelocity.GetMinimum(), 1e-3f ) && Near( 90.0f, rain.angularVelocity.GetMaximum(), 1e-3f ) );
        rain.colourOverLifetime.Evaluate( 0.5f, 0.0f, rgba );
        ASSERT_TRUE( rain.colourOverLifetimeEnabled && Near( 0.5f, rgba[ 3 ], 1e-3f ) );
        ASSERT_TRUE( rain.textureSheet.enabled && 4 == rain.textureSheet.tilesX && 2 == rain.textureSheet.tilesY && Near( 2.0f, rain.textureSheet.cycles ) && !rain.textureSheet.startFrame.IsConstant() );
        ASSERT_TRUE( rain.velocity.enabled && Near( 1.0f, rain.velocity.y.GetAverage() ) && PARTICLESPACE_WORLD == rain.velocity.space );
        ASSERT_TRUE( rain.noise.enabled && Near( 0.5f, rain.noise.strength.GetAverage() ) && Near( 2.0f, rain.noise.frequency ) );
        ASSERT_TRUE( rain.collision.enabled && rain.collision.world && Near( 0.5f, rain.collision.bounce ) );
        ASSERT_TRUE( PARTICLERENDER_STRETCHED == rain.renderer.mode && Near( 3.0f, rain.renderer.lengthScale ) && Near( 0.5f, rain.renderer.pivot[ 1 ] ) && PARTICLESORT_DISTANCE == rain.renderer.sortMode );
        ASSERT_TRUE( PARTICLEBLEND_ADDITIVE == rain.renderer.blendMode && Near( 1.0f, rain.renderer.tint[ 0 ] ) && Near( 0.5f, rain.renderer.tint[ 1 ] ) );
        ASSERT_TRUE( 1 == rain.subEmitters.size() );
        if ( 1 == rain.subEmitters.size() )
            ASSERT_TRUE( PARTICLETRIGGER_COLLISION == rain.subEmitters[ 0 ].trigger && rain.subEmitters[ 0 ].system >= 0 && "Splash" == scene.particleSystems[ rain.subEmitters[ 0 ].system ].name );
        ASSERT_TRUE( CountItems( report, REPORTITEM_SKIPPED, "LightsModule" ) > 0 );

        const Int32 splashNode = FindNode( scene, "Splash" );
        REQUIRE( splashNode >= 0 && scene.nodes[ splashNode ].particleSystem >= 0 );
        const CAssetParticleSystem& splash = scene.particleSystems[ scene.nodes[ splashNode ].particleSystem ];
        ASSERT_TRUE( !splash.looping && PARTICLESPACE_LOCAL == splash.simulationSpace && PARTICLESHAPE_BOX == splash.shape.type );
        // the box of the older layout in the canonical frame: X, Y (Unity's Z), Z (Unity's Y)
        ASSERT_TRUE( Near( 2.0f, splash.shape.boxSize[ 0 ] ) && Near( 4.0f, splash.shape.boxSize[ 1 ] ) && Near( 3.0f, splash.shape.boxSize[ 2 ] ) );
        ASSERT_TRUE( 1 == splash.bursts.size() && Near( 6.0f, splash.bursts[ 0 ].count.GetAverage() ) && 1 == splash.bursts[ 0 ].cycles );
        ASSERT_TRUE( CAssetParticleSystem::IsSubEmitter( scene.particleSystems, (UInt32) scene.nodes[ splashNode ].particleSystem ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}
