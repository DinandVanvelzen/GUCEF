/*
 *  assetpluginGLTF_TestApp: Test application for the glTF plugin of the asset converter (assetpluginGLTF)
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

#include "TestGltfSupport.h"
#include "TestGltfScenes.h"
#include "TestGltfRoundTrip.h"

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

bool
GetPlugins( TAssetImporterPtr& importer, TAssetExporterPtr& exporter )
{
    CAssetPluginRegistry& registry = CAssetConverterGlobal::Instance()->GetPluginRegistry();
    return registry.TryGetImporter( "gltf", importer ) && registry.TryGetExporter( "gltf", exporter );
}

/** Exports the scene into the folder and reads it back; false when a step fails or the report holds errors */
bool
RoundTrip( const CAssetScene& source, const CORE::CString& folder, CAssetScene& result, CConversionReport& report )
{
    TAssetImporterPtr importer;
    TAssetExporterPtr exporter;
    if ( !GetPlugins( importer, exporter ) )
        return false;
    if ( !exporter->Export( source, folder, report ) )
        return false;
    CAssetPackContext context;
    if ( !OpenPackContext( context, folder, folder + "/inter" ) )
        return false;
    result.name = source.name;
    return importer->Import( folder + "/" + source.name + ".gltf", context, result, report );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGltfRoundTripTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetpluginGLTF.RoundTrip" );

    GUCEF_TESTFW_TESTCASE( "Plugin: the importer and the exporter \"gltf\" are registered with their versions, and the importer takes .gltf and .glb files only" )
    try
    {
        TAssetImporterPtr importer;
        TAssetExporterPtr exporter;
        ASSERT_TRUE( GetPlugins( importer, exporter ) );
        ASSERT_TRUE( 1 == importer->GetVersion() && 1 == exporter->GetVersion() );
        CAssetPackContext context;
        ASSERT_TRUE( importer->CanImport( "pack/model.gltf", context ) && importer->CanImport( "pack/model.GLB", context ) );
        ASSERT_FALSE( importer->CanImport( "pack/model.fbx", context ) || importer->CanImport( "pack/model", context ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Round trip: a scene with every part of the model gives the same scene, and cgltf validates the file" )
    try
    {
        MakeWorkFolder( "rt" );
        CAssetScene source;
        BuildFullScene( source, "full" );
        CConversionReport report;
        CAssetScene result;
        ASSERT_TRUE( RoundTrip( source, "work/rt/out", result, report ) );
        ASSERT_TRUE( 0 == report.GetItemCount( REPORTITEM_ERROR ) );
        ASSERT_TRUE( CORE::FileExists( WorkDiskPath( "rt/out/full.gltf" ) ) && CORE::FileExists( WorkDiskPath( "rt/out/full.bin" ) ) );

        CORE::CString problem;
        ASSERT_TRUE( ValidateWithCgltf( WorkDiskPath( "rt/out/full.gltf" ), problem ) );

        CORE::CString difference;
        const bool same = CompareScenes( source, result, difference );
        if ( !same )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Round trip of the full scene differs: " << difference );
        ASSERT_TRUE( same );
        ASSERT_TRUE( "scene extras" == result.extras.GetAttributeValueOrChildValueByName( "note" ).AsString() && GUCEF_NULL != result.extras.FindChild( "gltf" ) );

        // what the file says about itself
        const CORE::CString text = ReadVfsText( "work/rt/out/full.gltf" );
        const char* const names[] = { "KHR_lights_punctual", "KHR_texture_transform", "KHR_materials_emissive_strength", "KHR_materials_unlit", "KHR_materials_clearcoat", "MSFT_lod",
                                      "OMI_physics_shape", "OMI_physics_body", "JOINTS_1", "WEIGHTS_1", "\"targets\"", "\"CUBICSPLINE\"", "\"STEP\"", "MSFT_screencoverage" };
        for ( size_t i=0; i<sizeof( names ) / sizeof( names[ 0 ] ); ++i )
            ASSERT_TRUE( text.HasSubstr( names[ i ] ) >= 0 );
        ASSERT_TRUE( "gltf" == result.provenance.sourceFormat );
        ASSERT_TRUE( "2.0" == result.provenance.sourceFormatVersion );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Determinism: the same scene gives byte-identical files, also after a trip through the importer" )
    try
    {
        MakeWorkFolder( "det" );
        TAssetImporterPtr importer;
        TAssetExporterPtr exporter;
        ASSERT_TRUE( GetPlugins( importer, exporter ) );
        CAssetScene source;
        BuildFullScene( source, "full" );
        CConversionReport report;
        ASSERT_TRUE( exporter->Export( source, "work/det/one", report ) );
        ASSERT_TRUE( exporter->Export( source, "work/det/two", report ) );
        ASSERT_TRUE( SameBytes( "work/det/one/full.gltf", "work/det/two/full.gltf" ) && SameBytes( "work/det/one/full.bin", "work/det/two/full.bin" ) );

        CAssetScene imported;
        ASSERT_TRUE( RoundTrip( source, "work/det/three", imported, report ) );
        ASSERT_TRUE( exporter->Export( imported, "work/det/four", report ) );
        ASSERT_TRUE( SameBytes( "work/det/one/full.gltf", "work/det/four/full.gltf" ) && SameBytes( "work/det/one/full.bin", "work/det/four/full.bin" ) );

        // golden hashes of the files of the full scene
        const CORE::CString gltfHash = CContentHash::HashFile( "work/det/one/full.gltf" );
        const CORE::CString binHash = CContentHash::HashFile( "work/det/one/full.bin" );
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Golden: full.gltf " << gltfHash << " full.bin " << binHash );
        ASSERT_TRUE( "6531c874e6e898c8200337219b80119ca21d59a15e1961bb2e4db528cc311816" == gltfHash );
        ASSERT_TRUE( "fa0558d083c0a08525e51bf013e61448d24a421d68eeff7b3c5064aab7aef1a3" == binHash );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Export: broken parts of a scene are left out with a warning and the file still validates" )
    try
    {
        MakeWorkFolder( "broken" );
        TAssetImporterPtr importer;
        TAssetExporterPtr exporter;
        ASSERT_TRUE( GetPlugins( importer, exporter ) );
        CAssetScene scene;
        BuildTriangleScene( scene, "broken" );
        CAssetSubMesh& subMesh = scene.meshes[ 0 ].subMeshes[ 0 ];
        subMesh.normals.resize( 5 );                                // not three per vertex
        subMesh.indices.push_back( 77 );                            // not a triangle, and outside the vertices
        subMesh.influencesPerVertex = 4;                            // without joints and weights
        CAssetAnimation animation;
        animation.name = "Broken";
        CAssetAnimationChannel channel;
        channel.targetNode = 0;
        channel.times.push_back( 0.0f );
        channel.times.push_back( 1.0f );
        channel.values.push_back( 1.0f );                           // two keys need six values
        animation.channels.push_back( channel );
        scene.animations.push_back( animation );
        scene.nodes[ 0 ].skin = 4;                                  // no such skin
        CAssetCollider collider;
        collider.shape = COLLIDERSHAPE_CONVEX;
        collider.meshIndex = 9;                                     // no such mesh
        scene.nodes[ 0 ].colliders.push_back( collider );

        CConversionReport report;
        ASSERT_TRUE( exporter->Export( scene, "work/broken/out", report ) );
        ASSERT_TRUE( report.GetItemCount( REPORTITEM_WARNING ) >= 4 );
        CORE::CString problem;
        ASSERT_TRUE( ValidateWithCgltf( WorkDiskPath( "broken/out/broken.gltf" ), problem ) );

        // a scene without anything is a valid file as well
        CAssetScene empty;
        empty.name = "empty";
        ASSERT_TRUE( exporter->Export( empty, "work/broken/out", report ) );
        ASSERT_TRUE( ValidateWithCgltf( WorkDiskPath( "broken/out/empty.gltf" ), problem ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Textures: image URIs are relative to the file, also from another folder of the intermediate, and come back as the same source" )
    try
    {
        MakeWorkFolder( "uri" );
        TAssetImporterPtr importer;
        TAssetExporterPtr exporter;
        ASSERT_TRUE( GetPlugins( importer, exporter ) );
        CAssetScene scene;
        BuildTriangleScene( scene, "model" );
        scene.textures[ 0 ].name = "tri tex.png";
        scene.textures[ 0 ].bakedPath = "work/uri/inter/textures/tri tex.png";
        scene.textures[ 0 ].sources[ 0 ] = "work/uri/inter/textures/tri tex.png";
        CConversionReport report;
        ASSERT_TRUE( exporter->Export( scene, "work/uri/inter/props/model", report ) );
        const CORE::CString text = ReadVfsText( "work/uri/inter/props/model/model.gltf" );
        ASSERT_TRUE( text.HasSubstr( "\"uri\": \"../../textures/tri%20tex.png\"" ) >= 0 );

        CAssetPackContext context;
        ASSERT_TRUE( OpenPackContext( context, "work/uri/inter/props/model", "work/uri/inter2" ) );
        CAssetScene result;
        ASSERT_TRUE( importer->Import( "work/uri/inter/props/model/model.gltf", context, result, report ) );
        ASSERT_TRUE( 1 == result.textures.size() && 1 == result.textures[ 0 ].sources.size() && "work/uri/inter/textures/tri tex.png" == result.textures[ 0 ].sources[ 0 ] );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
