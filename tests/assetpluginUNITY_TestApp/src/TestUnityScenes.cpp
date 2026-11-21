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

#include "AssetConverterLib_CAssetPluginRegistry.h"
#include "AssetConverterLib_CAssetConverterGlobal.h"
#include "assetpluginUNITY_CUnityImporter.h"
#include "assetpluginUNITY_CUnityProject.h"
#include "assetpluginUNITY_CUnityMaterialReader.h"
#include "assetpluginUNITY_CUnityPrefabReader.h"

#include "TestUnitySupport.h"
#include "TestUnityContent.h"
#include "TestUnityScenes.h"

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

namespace {

typedef CORE::CString TText;

const UInt32 InstanceCount = 3000;
const UInt32 UnresolvedCount = 2;
const char* const GuidCrate = "c0000000000000000000000000000001";
const char* const GuidScript = "c0000000000000000000000000000002";
const char* const GuidScene = "c0000000000000000000000000000003";
const char* const GuidMissing = "c0000000000000000000000000009999";
const char* const PackagePath = "scenes/many.unitypackage";
const char* const YamlHeader = "%YAML 1.1\n%TAG !u! tag:unity3d.com,2011:\n";

/** The prefab: a cube with a box collider */
TText
MakeCratePrefab( void )
{
    TText text = YamlHeader;
    text += "--- !u!1 &100\nGameObject:\n  m_ObjectHideFlags: 0\n  serializedVersion: 6\n  m_Component:\n  - component: {fileID: 400}\n  - component: {fileID: 3300}\n"
            "  - component: {fileID: 2300}\n  - component: {fileID: 6500}\n  m_Layer: 0\n  m_Name: Crate\n  m_IsActive: 1\n";
    text += "--- !u!4 &400\nTransform:\n  m_GameObject: {fileID: 100}\n  m_LocalRotation: {x: 0, y: 0, z: 0, w: 1}\n  m_LocalPosition: {x: 0, y: 0, z: 0}\n"
            "  m_LocalScale: {x: 1, y: 1, z: 1}\n  m_Children: []\n  m_Father: {fileID: 0}\n";
    text += "--- !u!33 &3300\nMeshFilter:\n  m_GameObject: {fileID: 100}\n  m_Mesh: {fileID: 10202, guid: 0000000000000000e000000000000000, type: 0}\n";
    text += "--- !u!23 &2300\nMeshRenderer:\n  m_GameObject: {fileID: 100}\n  m_Enabled: 1\n  m_Materials:\n  - {fileID: 10303, guid: 0000000000000000f000000000000000, type: 0}\n";
    text += "--- !u!65 &6500\nBoxCollider:\n  m_GameObject: {fileID: 100}\n  m_IsTrigger: 0\n  m_Enabled: 1\n  m_Size: {x: 1, y: 1, z: 1}\n  m_Center: {x: 0, y: 0, z: 0}\n";
    return text;
}

/** An instance of the prefab with a name and a position, as the editor writes it */
TText
MakeInstance( UInt32 id, const char* prefabGuid, UInt32 number )
{
    const TText target = ", guid: " + TText( prefabGuid ) + ", type: 3}";
    return "--- !u!1001 &" + CORE::UInt32ToString( id ) + "\nPrefabInstance:\n  m_ObjectHideFlags: 0\n  serializedVersion: 2\n  m_Modification:\n"
           "    m_TransformParent: {fileID: 0}\n    m_Modifications:\n"
           "    - target: {fileID: 400" + target + "\n      propertyPath: m_LocalPosition.x\n      value: " + CORE::UInt32ToString( number % 60 ) + "\n      objectReference: {fileID: 0}\n"
           "    - target: {fileID: 400" + target + "\n      propertyPath: m_LocalPosition.z\n      value: " + CORE::UInt32ToString( number / 60 ) + "\n      objectReference: {fileID: 0}\n"
           "    - target: {fileID: 100" + target + "\n      propertyPath: m_Name\n      value: Crate (" + CORE::UInt32ToString( number ) + ")\n      objectReference: {fileID: 0}\n"
           "    m_RemovedComponents: []\n  m_SourcePrefab: {fileID: 100100000" + target + "\n";
}

/** The scene: the instances, instances of a prefab that is not in the package, a camera with an audio listener and a script, and the scene settings */
TText
MakeScene( void )
{
    TText text = YamlHeader;
    text += "--- !u!104 &2\nRenderSettings:\n  m_ObjectHideFlags: 0\n  serializedVersion: 9\n  m_Fog: 1\n  m_FogDensity: 0.02\n"
            "  m_SkyboxMaterial: {fileID: 11500000, guid: " + TText( GuidScript ) + ", type: 3}\n";
    text += "--- !u!1 &500\nGameObject:\n  m_ObjectHideFlags: 0\n  serializedVersion: 6\n  m_Component:\n  - component: {fileID: 501}\n  - component: {fileID: 502}\n"
            "  - component: {fileID: 503}\n  - component: {fileID: 504}\n  m_Layer: 0\n  m_Name: Main View\n  m_IsActive: 1\n";
    text += "--- !u!4 &501\nTransform:\n  m_GameObject: {fileID: 500}\n  m_LocalRotation: {x: 0, y: 0, z: 0, w: 1}\n  m_LocalPosition: {x: 0, y: 5, z: -10}\n"
            "  m_LocalScale: {x: 1, y: 1, z: 1}\n  m_Children: []\n  m_Father: {fileID: 0}\n";
    text += "--- !u!20 &502\nCamera:\n  m_GameObject: {fileID: 500}\n  m_Enabled: 1\n  m_ClearFlags: 1\n  orthographic: 0\n  orthographic size: 5\n"
            "  field of view: 42\n  near clip plane: 0.5\n  far clip plane: 250\n  m_Depth: -1\n";
    text += "--- !u!81 &503\nAudioListener:\n  m_GameObject: {fileID: 500}\n  m_Enabled: 1\n";
    text += "--- !u!114 &504\nMonoBehaviour:\n  m_GameObject: {fileID: 500}\n  m_Enabled: 1\n  m_Script: {fileID: 11500000, guid: " + TText( GuidScript ) + ", type: 3}\n"
            "  m_Name: \n  speed: 3.5\n  title: hello\n";
    for ( UInt32 i=0; i<InstanceCount; ++i )
        text += MakeInstance( 1000 + i, GuidCrate, i );
    for ( UInt32 i=0; i<UnresolvedCount; ++i )
        text += MakeInstance( 900000 + i, GuidMissing, i );
    return text;
}

bool
WriteScenePackage( void )
{
    CTestUnityProject project;
    project.AddFolder( "Assets", "c0000000000000000000000000000010" );
    project.AddFolder( "Assets/Scenes", "c0000000000000000000000000000011" );
    project.AddText( "Assets/Prefabs/Crate.prefab", GuidCrate, MakeCratePrefab(), MakeMeta( GuidCrate, "PrefabImporter:\n  userData: \n" ) );
    project.AddText( "Assets/Scripts/Mover.cs", GuidScript, "public class Mover {}\n", MakeMeta( GuidScript, "MonoImporter:\n  userData: \n" ) );
    project.AddText( "Assets/Scenes/Many.unity", GuidScene, MakeScene(), MakeMeta( GuidScene, "DefaultImporter:\n  userData: \n" ) );
    return project.WritePackage( WorkDiskPath( PackagePath ) );
}

bool
OpenPack( CAssetPackContext& context, const CORE::CString& source, const CORE::CString& id )
{
    CAssetPackManifest manifest;
    manifest.id = id;
    manifest.source = source;
    manifest.importer = "unity";
    manifest.intermediate = "/work/scenes/intermediate";
    return context.Open( manifest );
}

/** The whole import of an asset as the pipeline does it */
bool
ImportScene( CAssetPackContext& context, CUnityImporter& importer, const CORE::CString& assetPath, CAssetScene& scene, CConversionReport& report )
{
    scene = CAssetScene();
    scene.name = CVfsPaths::StemOf( assetPath );
    CORE::CDataNode settings( "settings", GUCEF_DATATYPE_OBJECT );
    importer.GetDefaultSettings( settings );
    scene.settings = settings;
    report.Clear();
    report.SelectAsset( assetPath );
    return importer.Import( context.ToVfsPath( assetPath ), context, scene, report );
}

/** The message of the first report item of the kind with the text in the message, empty when there is none */
CORE::CString
FirstMessage( const CConversionReport& report, TReportItemKind kind, const CORE::CString& text )
{
    const CConversionReport::TEntryVector& entries = report.GetEntries();
    for ( size_t e=0; e<entries.size(); ++e )
        for ( size_t i=0; i<entries[ e ].items.size(); ++i )
            if ( entries[ e ].items[ i ].kind == kind && entries[ e ].items[ i ].message.HasSubstr( text ) >= 0 )
                return entries[ e ].items[ i ].message;
    return CORE::CString::Empty;
}

/** The value of an environment variable, empty when it is not set */
CORE::CString
EnvironmentValue( const char* name )
{
    return CORE::ResolveVars( "$ENVVAR:" + CORE::CString( name ) + "$" );
}

UInt32
CountNodesWithExtra( const CAssetScene& scene, const char* name )
{
    UInt32 count = 0;
    for ( size_t n=0; n<scene.nodes.size(); ++n )
        if ( GUCEF_NULL != scene.nodes[ n ].extras.FindChild( name ) )
            ++count;
    return count;
}

/** The counts of a scene as one line each, for the golden files of the corpus test */
CORE::CString
Describe( const CAssetScene& scene )
{
    UInt32 colliders = 0;
    UInt32 instances = 0;
    for ( size_t n=0; n<scene.nodes.size(); ++n )
        colliders += (UInt32) scene.nodes[ n ].colliders.size();
    for ( size_t p=0; p<scene.prefabs.size(); ++p )
        instances += (UInt32) scene.prefabs[ p ].instances.size();
    return "nodes=" + CORE::UInt32ToString( (UInt32) scene.nodes.size() ) + "\nmeshes=" + CORE::UInt32ToString( (UInt32) scene.meshes.size() ) +
           "\nmaterials=" + CORE::UInt32ToString( (UInt32) scene.materials.size() ) + "\ntextures=" + CORE::UInt32ToString( (UInt32) scene.textures.size() ) +
           "\nlights=" + CORE::UInt32ToString( (UInt32) scene.lights.size() ) + "\nparticleSystems=" + CORE::UInt32ToString( (UInt32) scene.particleSystems.size() ) +
           "\ncolliders=" + CORE::UInt32ToString( colliders ) + "\ninstances=" + CORE::UInt32ToString( instances ) + "\n";
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The scenes of a package named by the environment against golden counts that are kept outside the repositories */
static void
TestCorpusScenes( void )
{GUCEF_TRACE;

    const CORE::CString enabled = EnvironmentValue( "GUCEF_TEST_ASSET_CORPUS" );
    const CORE::CString package = EnvironmentValue( "GUCEF_TEST_UNITY_SCENES_PACKAGE" );
    const CORE::CString goldenFolder = EnvironmentValue( "GUCEF_TEST_UNITY_SCENES_GOLDEN" );
    if ( enabled != "1" || package.IsNULLOrEmpty() || goldenFolder.IsNULLOrEmpty() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Unity scenes: the corpus test is off (GUCEF_TEST_ASSET_CORPUS, GUCEF_TEST_UNITY_SCENES_PACKAGE and GUCEF_TEST_UNITY_SCENES_GOLDEN)" );
        return;
    }

    GUCEF_TESTFW_TESTCASE( "Corpus: every scene of the package named by the environment converts to the golden counts, nothing unresolved without a report item" )
    try
    {
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        ASSERT_TRUE( vfs.AddRoot( CORE::StripFilename( package ), "UnityScenesCorpusDir", false, false, "scenescorpus" ) );
        CAssetPackContext context;
        ASSERT_TRUE( OpenPack( context, "/scenescorpus/" + CORE::ExtractFilename( package ), "corpus-scenes" ) );
        CUnityImporter importer;
        ASSERT_TRUE( importer.IndexPack( context ) );
        CORE::CreateDirs( goldenFolder );

        UInt32 scenes = 0;
        const CORE::CString::StringVector& files = context.GetFiles();
        for ( size_t f=0; f<files.size(); ++f )
        {
            if ( "unity" != CVfsPaths::ExtensionOf( files[ f ] ) )
                continue;
            ++scenes;
            CAssetScene scene;
            CConversionReport report;
            ASSERT_TRUE( ImportScene( context, importer, files[ f ], scene, report ) );
            const CORE::CString counts = Describe( scene );
            const CORE::CString goldenPath = CORE::CombinePath( goldenFolder, files[ f ].ReplaceChar( '/', '_' ) + ".txt" );
            if ( !CORE::FileExists( goldenPath ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Unity scenes: the golden file " << goldenPath << " is made" );
                ASSERT_TRUE( CORE::WriteStringAsTextFile( goldenPath, counts ) );
                continue;
            }
            CORE::CString golden;
            ASSERT_TRUE( CORE::LoadTextFileAsString( goldenPath, golden ) );
            ASSERT_TRUE( golden == counts );
            // every instance is counted: those the scene could not expand are in the report
            ASSERT_TRUE( FirstMessage( report, REPORTITEM_INFO, "prefab instance(s) met" ).Length() > 0 );
        }
        ASSERT_TRUE( scenes > 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/** A cancellation check for the tests: says stop from its 'trigger'th poll on, or 'afterMs' milliseconds after its first poll */
struct SCancelAfter
{
    SCancelAfter( void ) : polls( 0 ), trigger( 0xFFFFFFFF ), afterMs( 0 ), firstPollTick( 0 ), firstStopTick( 0 ) {}
    UInt32 polls;
    UInt32 trigger;
    UInt32 afterMs;
    UInt64 firstPollTick;
    UInt64 firstStopTick;
};

static bool
CancelAfterPolls( void* context )
{
    SCancelAfter* state = static_cast< SCancelAfter* >( context );
    ++state->polls;
    if ( 0 == state->firstPollTick )
        state->firstPollTick = MT::PrecisionTickCount();
    bool stop = state->polls >= state->trigger;
    if ( !stop && state->afterMs > 0 )
        stop = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - state->firstPollTick ) >= (Float64) state->afterMs;
    if ( stop && 0 == state->firstStopTick )
        state->firstStopTick = MT::PrecisionTickCount();
    return stop;
}

void
PerformUnitySceneTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetpluginUNITY.Scenes" );

    bool written = false;
    GUCEF_TESTFW_TESTCASE( "Scenes: a package with a scene of thousands of prefab instances is written and indexed" )
    try
    {
        written = WriteScenePackage();
        ASSERT_TRUE( written );
        CAssetPackContext context;
        ASSERT_TRUE( OpenPack( context, "/work/scenes/many.unitypackage", "scenespackage" ) );
        CUnityImporter importer;
        ASSERT_TRUE( importer.IndexPack( context ) );
        ASSERT_TRUE( context.GetIndexSize( CUnityProject::GuidIndexName ) >= 5 );
        CORE::CString path;
        ASSERT_TRUE( context.TryResolve( CUnityProject::GuidIndexName, GuidCrate, path ) && path.EndsWith( "Assets/Prefabs/Crate.prefab" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Pack context: the files of a pack are found by name, case ignored, in the order of the pack, and the map is made once" )
    try
    {
        ASSERT_TRUE( written );
        CAssetPackContext context;
        ASSERT_TRUE( OpenPack( context, "/work/scenes/many.unitypackage", "scenesnames" ) );
        CORE::CString::StringVector found;
        ASSERT_TRUE( context.FindFilesByName( "CRATE.PREFAB", found ) && 1 == found.size() && found[ 0 ].EndsWith( "Assets/Prefabs/Crate.prefab" ) );
        ASSERT_TRUE( context.FindFilesByName( "crate.prefab", found ) && 1 == found.size() );
        ASSERT_TRUE( context.FindFilesByName( "Crate.prefab.meta", found ) && 1 == found.size() );
        ASSERT_FALSE( context.FindFilesByName( "nothing.fbx", found ) );
        ASSERT_TRUE( found.size() == 1 );     // a miss leaves the result as it was
        // a closed context has no files to find
        context.Close();
        ASSERT_FALSE( context.FindFilesByName( "crate.prefab", found ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Scene of 3000 prefab instances: every file is read once, the cube is made once, the counts are exact" )
    try
    {
        ASSERT_TRUE( written );
        CAssetPackContext context;
        ASSERT_TRUE( OpenPack( context, "/work/scenes/many.unitypackage", "scenescounts" ) );
        CUnityImporter importer;
        ASSERT_TRUE( importer.IndexPack( context ) );

        // the reader straight, to count the files it reads and the source meshes it fetches
        CAssetScene scene;
        CConversionReport report;
        CORE::CDataNode settings( "settings", GUCEF_DATATYPE_OBJECT );
        importer.GetDefaultSettings( settings );
        scene.settings = settings;
        report.SelectAsset( "Assets/Scenes/Many.unity" );
        const CORE::CString vfsPath = context.ToVfsPath( "Assets/Scenes/Many.unity" );
        CUnityProject project( context, scene, report, vfsPath );
        CUnityMaterialReader materials( project );
        CUnityPrefabReader prefabs( project, materials );
        const UInt64 started = MT::PrecisionTickCount();
        ASSERT_TRUE( prefabs.ReadAsset( vfsPath ) );
        const Float64 elapsed = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - started );

        // the work does not grow with the instances: the scene and the prefab are read once, the cube is fetched once
        ASSERT_TRUE( 2 == project.GetYamlLoadCount() );
        ASSERT_TRUE( 0 == project.GetModelLoadCount() );
        ASSERT_TRUE( 1 == prefabs.GetSourceMeshCount() );
        // a generous bound as a guard against the work growing with the square of the instances; the counts above are the test
        ASSERT_TRUE( elapsed < 120000 );

        // the counts: a root with the camera object, the 3000 crates and the stand-ins of the instances of the missing prefab
        ASSERT_TRUE( 1 == scene.meshes.size() );
        ASSERT_TRUE( InstanceCount == scene.prefabs[ 0 ].instances.size() + 0 || InstanceCount + UnresolvedCount == scene.prefabs[ 0 ].instances.size() );
        UInt32 colliders = 0;
        UInt32 withMesh = 0;
        for ( size_t n=0; n<scene.nodes.size(); ++n )
        {
            colliders += (UInt32) scene.nodes[ n ].colliders.size();
            withMesh += scene.nodes[ n ].mesh >= 0 ? 1 : 0;
        }
        ASSERT_TRUE( InstanceCount == colliders );
        ASSERT_TRUE( InstanceCount == withMesh );
        const Int32 first = scene.FindNode( "Crate (0)" );
        const Int32 last = scene.FindNode( "Crate (2999)" );
        ASSERT_TRUE( first >= 0 && last >= 0 );
        ASSERT_TRUE( last >= 0 && Near( scene.nodes[ last ].transform.position.x, 59.0f ) && Near( scene.nodes[ last ].transform.position.z, 49.0f ) );

        // the instances are counted in the report, those that could not be expanded are named
        const CORE::CString counted = FirstMessage( report, REPORTITEM_INFO, "prefab instance(s) met" );
        ASSERT_TRUE( counted.HasSubstr( "3002 prefab instance(s) met, 3000 expanded, 2 unresolved" ) >= 0 );
        ASSERT_TRUE( CountItems( report, REPORTITEM_WARNING, "which is not in the package: an empty node stands in for it" ) >= 1 );
        // the stand-in takes the name its modifications give it, made unique like the others
        const Int32 standIn = scene.FindNode( "Crate (0) 1" );
        ASSERT_TRUE( standIn >= 0 && scene.nodes[ standIn ].mesh < 0 && scene.nodes[ standIn ].colliders.empty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Scene components: the camera, the audio listener and the script are kept with their properties, the scene settings with the scene" )
    try
    {
        ASSERT_TRUE( written );
        CAssetPackContext context;
        ASSERT_TRUE( OpenPack( context, "/work/scenes/many.unitypackage", "scenescomponents" ) );
        CUnityImporter importer;
        ASSERT_TRUE( importer.IndexPack( context ) );
        CAssetScene scene;
        CConversionReport report;
        ASSERT_TRUE( ImportScene( context, importer, "Assets/Scenes/Many.unity", scene, report ) );

        const Int32 view = scene.FindNode( "Main View" );
        ASSERT_TRUE( view >= 0 );
        if ( view >= 0 )
        {
            const CORE::CDataNode& extras = scene.nodes[ view ].extras;
            const CORE::CDataNode* camera = extras.FindChild( "camera" );
            ASSERT_TRUE( GUCEF_NULL != camera );
            if ( GUCEF_NULL != camera )
            {
                ASSERT_TRUE( Near( camera->GetAttributeValueOrChildValueByName( "fieldOfView" ).AsFloat32( 0.0f, true ), 42.0f ) );
                ASSERT_TRUE( Near( camera->GetAttributeValueOrChildValueByName( "nearClip" ).AsFloat32( 0.0f, true ), 0.5f ) );
                ASSERT_TRUE( Near( camera->GetAttributeValueOrChildValueByName( "farClip" ).AsFloat32( 0.0f, true ), 250.0f ) );
                ASSERT_FALSE( camera->GetAttributeValueOrChildValueByName( "orthographic" ).AsBool( true, true ) );
            }
            const CORE::CDataNode* components = extras.FindChild( "components" );
            ASSERT_TRUE( GUCEF_NULL != components && 3 == components->GetNrOfDirectChildNodes() );
            bool foundScript = false;
            if ( GUCEF_NULL != components )
            {
                CORE::CDataNode::const_iterator i = components->ConstBegin();
                while ( i != components->ConstEnd() )
                {
                    if ( "MonoBehaviour" == (*i)->GetAttributeValueOrChildValueByName( "type" ).AsString() )
                    {
                        foundScript = true;
                        ASSERT_TRUE( (*i)->GetAttributeValueOrChildValueByName( "script" ).AsString().EndsWith( "Assets/Scripts/Mover.cs" ) );
                        const CORE::CDataNode* properties = (*i)->FindChild( "properties" );
                        ASSERT_TRUE( GUCEF_NULL != properties && "hello" == properties->GetAttributeValueOrChildValueByName( "title" ).AsString() );
                    }
                    ++i;
                }
            }
            ASSERT_TRUE( foundScript );
        }
        ASSERT_TRUE( 1 == CountNodesWithExtra( scene, "camera" ) );

        // nothing of the components is left out without a word in the report
        ASSERT_TRUE( FirstMessage( report, REPORTITEM_APPROXIMATION, "MonoBehaviour components have no element in the intermediate" ).Length() > 0 );
        ASSERT_TRUE( FirstMessage( report, REPORTITEM_APPROXIMATION, "AudioListener components have no element in the intermediate" ).Length() > 0 );

        // the scene settings are with the scene, the references in them given as paths of the package
        const CORE::CDataNode* facts = scene.extras.FindChild( CUnityProject::SettingsName );
        const CORE::CDataNode* sceneSettings = GUCEF_NULL != facts ? facts->FindChild( "sceneSettings" ) : GUCEF_NULL;
        const CORE::CDataNode* render = GUCEF_NULL != sceneSettings ? sceneSettings->FindChild( "RenderSettings" ) : GUCEF_NULL;
        ASSERT_TRUE( GUCEF_NULL != render );
        if ( GUCEF_NULL != render )
        {
            ASSERT_TRUE( 1 == render->GetAttributeValueOrChildValueByName( "m_Fog" ).AsInt32( 0, true ) );
            const CORE::CDataNode* skybox = render->FindChild( "m_SkyboxMaterial" );
            ASSERT_TRUE( GUCEF_NULL != skybox && skybox->GetAttributeValueOrChildValueByName( "path" ).AsString().EndsWith( "Assets/Scripts/Mover.cs" ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Scene conversion is repeatable: the same package gives the same counts twice, with the instances of one prefab sharing its mesh" )
    try
    {
        ASSERT_TRUE( written );
        CORE::CString first;
        CORE::CString second;
        for ( UInt32 run=0; run<2; ++run )
        {
            CAssetPackContext context;
            ASSERT_TRUE( OpenPack( context, "/work/scenes/many.unitypackage", "scenesrepeat" ) );
            CUnityImporter importer;
            ASSERT_TRUE( importer.IndexPack( context ) );
            CAssetScene scene;
            CConversionReport report;
            ASSERT_TRUE( ImportScene( context, importer, "Assets/Scenes/Many.unity", scene, report ) );
            ( 0 == run ? first : second ) = Describe( scene );
        }
        ASSERT_TRUE( first == second );
        ASSERT_TRUE( first.HasSubstr( "meshes=1\n" ) >= 0 );
        ASSERT_TRUE( first.HasSubstr( "colliders=3000\n" ) >= 0 );
        ASSERT_TRUE( first.HasSubstr( "instances=3002\n" ) >= 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Cancellation: a scene of 3000 instances stops within a second of the cancel, nothing of it is stored, and an uncancelled run still converts" )
    try
    {
        ASSERT_TRUE( written );
        CAssetPackContext context;
        ASSERT_TRUE( OpenPack( context, "/work/scenes/many.unitypackage", "scenescancel" ) );
        CUnityImporter importer;
        ASSERT_TRUE( importer.IndexPack( context ) );

        // the reader straight: the check says stop from its 40th poll on; the reader ends at its next poll with nothing read
        {
            SCancelAfter cancel;
            cancel.trigger = 40;
            CAssetScene scene;
            CConversionReport report;
            CORE::CDataNode settings( "settings", GUCEF_DATATYPE_OBJECT );
            importer.GetDefaultSettings( settings );
            scene.settings = settings;
            report.SelectAsset( "Assets/Scenes/Many.unity" );
            report.SetCancellationCheck( &CancelAfterPolls, &cancel );
            const CORE::CString vfsPath = context.ToVfsPath( "Assets/Scenes/Many.unity" );
            CUnityProject project( context, scene, report, vfsPath );
            CUnityMaterialReader materials( project );
            CUnityPrefabReader prefabs( project, materials );
            ASSERT_FALSE( prefabs.ReadAsset( vfsPath ) );
            ASSERT_TRUE( cancel.polls >= cancel.trigger && cancel.polls <= cancel.trigger + 8 );
            ASSERT_TRUE( scene.prefabs.empty() );
        }

        // the pipeline: the check says stop 100 ms after its first poll; the conversion ends within a second of that, is reported as cancelled
        // and skipped, and writes no glTF file and no cache entry
        {
            TAssetExporterPtr exporter;
            ASSERT_TRUE( CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetExporter( "gltf", exporter ) );
            TAssetImporterPtr importerPtr;
            ASSERT_TRUE( CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetImporter( "unity", importerPtr ) );
            CAssetPackManifest manifest;
            manifest.id = "scenescancel";
            manifest.title = manifest.id;
            manifest.importer = "unity";
            manifest.exporter = "gltf";
            manifest.source = "/work/scenes/many.unitypackage";
            manifest.intermediate = "/work/scenes/cancel-intermediate";
            manifest.output = "/work/scenes/cancel-out";
            CAssetPackContext packContext;
            ASSERT_TRUE( packContext.Open( manifest ) && importerPtr->IndexPack( packContext ) );
            CTextureBaker baker( CVfsPaths::Join( manifest.GetIntermediatePath(), "textures" ) );
            CAssetConversionJob job;
            job.manifest = &manifest;
            job.context = &packContext;
            job.baker = &baker;
            job.importer = importerPtr;
            job.exporter = exporter;
            job.assetPath = "Assets/Scenes/Many.unity";
            SCancelAfter cancel;
            cancel.trigger = 0xFFFFFFFF;
            cancel.afterMs = 100;
            job.report.SetCancellationCheck( &CancelAfterPolls, &cancel );
            const UInt64 started = MT::PrecisionTickCount();
            const bool converted = CAssetPipeline::ConvertAsset( job );
            const UInt64 ended = MT::PrecisionTickCount();
            if ( 0 != cancel.firstStopTick )
            {
                // the scene took longer than the delay: the cancel was seen, and it was seen quickly
                ASSERT_FALSE( converted );
                ASSERT_TRUE( job.report.IsCancelled() && !job.converted && !job.cached );
                ASSERT_TRUE( MT::PrecisionTimerTicksToMs( ended - cancel.firstStopTick ) < 1000.0 );
                const CConversionReportEntry* entry = job.report.FindEntry( job.assetPath );
                ASSERT_TRUE( GUCEF_NULL != entry && entry->skipped && !entry->HasErrors() );
                ASSERT_TRUE( ACTIVITYACTION_SKIPPED == job.activity.action && ACTIVITYREASON_CANCELLED == job.activity.reason );
                VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
                ASSERT_FALSE( vfs.FileExists( CAssetCacheEntry::GetEntryPath( manifest.GetIntermediatePath(), job.assetPath ) ) );
                ASSERT_FALSE( vfs.FileExists( CVfsPaths::Join( manifest.GetIntermediatePath(), "Assets/Scenes/Many.gltf" ) ) );
            }
            else
            {
                // the whole scene was faster than the delay: the cancel never came, the conversion is complete
                ASSERT_TRUE( converted && job.converted );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Cancellation test: the scene converted in " << MT::PrecisionTimerTicksToMs( ended - started ) << " ms, before the cancel was due" );
            }
        }

        // a conversion that is never cancelled is not touched by the check
        {
            CAssetScene scene;
            CConversionReport report;
            SCancelAfter never;
            never.trigger = 0xFFFFFFFF;
            report.SetCancellationCheck( &CancelAfterPolls, &never );
            CUnityImporter again;
            ASSERT_TRUE( again.IndexPack( context ) );
            scene.name = "Many";
            CORE::CDataNode settings( "settings", GUCEF_DATATYPE_OBJECT );
            again.GetDefaultSettings( settings );
            scene.settings = settings;
            report.SelectAsset( "Assets/Scenes/Many.unity" );
            ASSERT_TRUE( again.Import( context.ToVfsPath( "Assets/Scenes/Many.unity" ), context, scene, report ) );
            ASSERT_TRUE( never.polls > 0 && 0 == never.firstStopTick && 1 == scene.meshes.size() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Cache: an asset that converted and was stored is a cache hit the next time, and one that was changed is converted again" )
    try
    {
        ASSERT_TRUE( written );
        TAssetExporterPtr exporter;
        TAssetImporterPtr importerPtr;
        ASSERT_TRUE( CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetExporter( "gltf", exporter ) );
        ASSERT_TRUE( CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetImporter( "unity", importerPtr ) );
        CAssetPackManifest manifest;
        manifest.id = "scenescache";
        manifest.title = manifest.id;
        manifest.importer = "unity";
        manifest.exporter = "gltf";
        manifest.source = "/work/scenes/many.unitypackage";
        manifest.intermediate = "/work/scenes/cache-intermediate";
        manifest.output = "/work/scenes/cache-out";
        CAssetPackContext packContext;
        ASSERT_TRUE( packContext.Open( manifest ) && importerPtr->IndexPack( packContext ) );
        CTextureBaker baker( CVfsPaths::Join( manifest.GetIntermediatePath(), "textures" ) );
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        const CORE::CString entryPath = CAssetCacheEntry::GetEntryPath( manifest.GetIntermediatePath(), "Assets/Prefabs/Crate.prefab" );

        CAssetConversionJob first;
        first.manifest = &manifest;
        first.context = &packContext;
        first.baker = &baker;
        first.importer = importerPtr;
        first.exporter = exporter;
        first.assetPath = "Assets/Prefabs/Crate.prefab";
        ASSERT_TRUE( CAssetPipeline::ConvertAsset( first ) && first.converted && !first.cached );
        ASSERT_FALSE( vfs.FileExists( entryPath ) );
        ASSERT_TRUE( CAssetPipeline::StoreCacheEntry( first ) && vfs.FileExists( entryPath ) );

        CAssetConversionJob second = first;
        second.report = CConversionReport();
        ASSERT_TRUE( CAssetPipeline::ConvertAsset( second ) && second.cached && !second.converted );

        // a job that is cancelled stores nothing
        CAssetConversionJob cancelled = first;
        cancelled.report = CConversionReport();
        cancelled.converted = true;
        cancelled.report.SelectAsset( cancelled.assetPath );
        cancelled.report.SetCancelled();
        ASSERT_FALSE( CAssetPipeline::StoreCacheEntry( cancelled ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    TestCorpusScenes();
}

/*-------------------------------------------------------------------------*/
