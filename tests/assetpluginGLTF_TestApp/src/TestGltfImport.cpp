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

#include <string.h>

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#include "TestGltfSupport.h"
#include "TestGltfImport.h"

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

typedef GUCEF::vector< UInt8 > TBytes;

void AddFloat( TBytes& bytes, Float32 value ) { const UInt8* p = reinterpret_cast< const UInt8* >( &value ); bytes.insert( bytes.end(), p, p + 4 ); }
void AddUInt16( TBytes& bytes, UInt16 value ) { bytes.push_back( (UInt8) ( value & 255 ) ); bytes.push_back( (UInt8) ( value >> 8 ) ); }
void AddUInt32( TBytes& bytes, UInt32 value ) { for ( UInt32 i=0; i<4; ++i ) bytes.push_back( (UInt8) ( ( value >> ( 8 * i ) ) & 255 ) ); }
void Pad( TBytes& bytes, UInt8 fill ) { while ( 0 != bytes.size() % 4 ) bytes.push_back( fill ); }
void AddText( TBytes& bytes, const CORE::CString& text ) { bytes.insert( bytes.end(), text.C_String(), text.C_String() + text.ByteSize() - 1 ); }

CORE::CString
Base64( const TBytes& bytes )
{
    static const char* const alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    CORE::CString text;
    for ( size_t i=0; i<bytes.size(); i+=3 )
    {
        const UInt32 a = bytes[ i ];
        const UInt32 b = i + 1 < bytes.size() ? bytes[ i + 1 ] : 0;
        const UInt32 c = i + 2 < bytes.size() ? bytes[ i + 2 ] : 0;
        const UInt32 triple = ( a << 16 ) | ( b << 8 ) | c;
        const char group[ 4 ] = { alphabet[ ( triple >> 18 ) & 63 ], alphabet[ ( triple >> 12 ) & 63 ], i + 1 < bytes.size() ? alphabet[ ( triple >> 6 ) & 63 ] : '=', i + 2 < bytes.size() ? alphabet[ triple & 63 ] : '=' };
        text.Append( group, 4 );
    }
    return text;
}

/** The binary glTF container: the JSON text and the buffer */
TBytes
MakeGlb( const CORE::CString& json, const TBytes& binary )
{
    TBytes jsonChunk;
    AddText( jsonChunk, json );
    Pad( jsonChunk, ' ' );
    TBytes binChunk( binary );
    Pad( binChunk, 0 );

    TBytes glb;
    AddUInt32( glb, 0x46546C67 );                                           // "glTF"
    AddUInt32( glb, 2 );
    AddUInt32( glb, (UInt32) ( 12 + 8 + jsonChunk.size() + 8 + binChunk.size() ) );
    AddUInt32( glb, (UInt32) jsonChunk.size() );
    AddUInt32( glb, 0x4E4F534A );                                           // "JSON"
    glb.insert( glb.end(), jsonChunk.begin(), jsonChunk.end() );
    AddUInt32( glb, (UInt32) binChunk.size() );
    AddUInt32( glb, 0x004E4942 );                                           // "BIN"
    glb.insert( glb.end(), binChunk.begin(), binChunk.end() );
    return glb;
}

bool
ImportFile( const CORE::CString& folder, const CORE::CString& file, CAssetScene& scene, CConversionReport& report )
{
    TAssetImporterPtr importer;
    if ( !CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetImporter( "gltf", importer ) )
        return false;
    CAssetPackContext context;
    if ( !OpenPackContext( context, folder, folder + "_inter" ) )
        return false;
    return importer->Import( folder + "/" + file, context, scene, report );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGltfImportTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetpluginGLTF.Import" );

    GUCEF_TESTFW_TESTCASE( "Import: a GLB with an image inside it: the geometry, the material, and the image written below the intermediate folder" )
    try
    {
        MakeWorkFolder( "glb" );
        const CORE::CString pngPath = "work/glb/source.png";
        ASSERT_TRUE( WriteTestPng( pngPath, 4, 4, 200, 100, 50, 255 ) );
        CORE::CDynamicBuffer png;
        ASSERT_TRUE( ReadVfsBytes( pngPath, png ) );

        TBytes binary;
        const Float32 positions[ 9 ] = { 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f };
        for ( UInt32 i=0; i<9; ++i )
            AddFloat( binary, positions[ i ] );
        AddUInt16( binary, 0 ); AddUInt16( binary, 1 ); AddUInt16( binary, 2 );
        Pad( binary, 0 );
        const UInt32 imageOffset = (UInt32) binary.size();
        binary.insert( binary.end(), static_cast< const UInt8* >( png.GetConstBufferPtr() ), static_cast< const UInt8* >( png.GetConstBufferPtr() ) + png.GetDataSize() );

        const CORE::CString json =
            "{ \"asset\": { \"version\": \"2.0\", \"generator\": \"hand written\" }, \"scene\": 0, \"scenes\": [ { \"nodes\": [ 0 ] } ],"
            "  \"nodes\": [ { \"name\": \"Tri\", \"mesh\": 0 } ],"
            "  \"meshes\": [ { \"name\": \"TriMesh\", \"primitives\": [ { \"attributes\": { \"POSITION\": 0 }, \"indices\": 1, \"material\": 0 } ] } ],"
            "  \"materials\": [ { \"name\": \"M\", \"pbrMetallicRoughness\": { \"baseColorTexture\": { \"index\": 0 } } } ],"
            "  \"textures\": [ { \"source\": 0 } ], \"images\": [ { \"bufferView\": 2, \"mimeType\": \"image/png\", \"name\": \"inside\" } ],"
            "  \"buffers\": [ { \"byteLength\": " + CORE::UInt32ToString( (UInt32) binary.size() ) + " } ],"
            "  \"bufferViews\": [ { \"buffer\": 0, \"byteOffset\": 0, \"byteLength\": 36, \"target\": 34962 }, { \"buffer\": 0, \"byteOffset\": 36, \"byteLength\": 6, \"target\": 34963 },"
            "                     { \"buffer\": 0, \"byteOffset\": " + CORE::UInt32ToString( imageOffset ) + ", \"byteLength\": " + CORE::UInt32ToString( png.GetDataSize() ) + " } ],"
            "  \"accessors\": [ { \"bufferView\": 0, \"componentType\": 5126, \"count\": 3, \"type\": \"VEC3\", \"min\": [ 0, 0, 0 ], \"max\": [ 1, 1, 0 ] },"
            "                   { \"bufferView\": 1, \"componentType\": 5123, \"count\": 3, \"type\": \"SCALAR\" } ] }";
        const TBytes glb = MakeGlb( json, binary );
        ASSERT_TRUE( WriteWorkFile( "glb/model.glb", &glb[ 0 ], (UInt32) glb.size() ) );

        CAssetScene scene;
        CConversionReport report;
        scene.name = "model";
        ASSERT_TRUE( ImportFile( "work/glb", "model.glb", scene, report ) );
        ASSERT_TRUE( 0 == report.GetItemCount( REPORTITEM_ERROR ) );
        ASSERT_TRUE( 1 == scene.nodes.size() && "Tri" == scene.nodes[ 0 ].name && 0 == scene.nodes[ 0 ].mesh );
        ASSERT_TRUE( 1 == scene.meshes.size() && 1 == scene.meshes[ 0 ].subMeshes.size() );
        const CAssetSubMesh& subMesh = scene.meshes[ 0 ].subMeshes[ 0 ];
        ASSERT_TRUE( 9 == subMesh.positions.size() && 1.0f == subMesh.positions[ 3 ] && 1.0f == subMesh.positions[ 7 ] );
        ASSERT_TRUE( 3 == subMesh.indices.size() && 2 == subMesh.indices[ 2 ] && 0 == subMesh.materialIndex );
        ASSERT_TRUE( "glb" == scene.provenance.sourceFormat );
        ASSERT_TRUE( "hand written" == scene.provenance.sourceEngine && "2.0" == scene.provenance.sourceFormatVersion );

        // the image of the GLB is a file now, with the bytes it had
        ASSERT_TRUE( 1 == scene.textures.size() && 1 == scene.textures[ 0 ].sources.size() );
        CORE::CDynamicBuffer extracted;
        ASSERT_TRUE( ReadVfsBytes( scene.textures[ 0 ].sources[ 0 ], extracted ) );
        ASSERT_TRUE( scene.textures[ 0 ].sources[ 0 ].StartsWith( "work/glb_inter/embedded/" ) );
        ASSERT_TRUE( extracted.GetDataSize() == png.GetDataSize() && 0 == memcmp( extracted.GetConstBufferPtr(), png.GetConstBufferPtr(), png.GetDataSize() ) );
        ASSERT_TRUE( TEXTUREUSAGE_COLOUR == scene.textures[ 0 ].usage );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Import: a data URI buffer, a triangle strip, normalised 16 bit UV coordinates, a node matrix, and a primitive of points that is skipped" )
    try
    {
        MakeWorkFolder( "strip" );
        TBytes binary;
        const Float32 positions[ 12 ] = { 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f };
        for ( UInt32 i=0; i<12; ++i )
            AddFloat( binary, positions[ i ] );
        const UInt16 uvs[ 8 ] = { 0, 0, 65535, 0, 0, 65535, 32768, 65535 };
        for ( UInt32 i=0; i<8; ++i )
            AddUInt16( binary, uvs[ i ] );
        for ( UInt16 i=0; i<4; ++i )
            AddUInt16( binary, i );

        const CORE::CString json =
            "{ \"asset\": { \"version\": \"2.0\" }, \"scene\": 0, \"scenes\": [ { \"nodes\": [ 0 ] } ],"
            "  \"nodes\": [ { \"name\": \"Matrix\", \"mesh\": 0, \"matrix\": [ 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 2, 0, 1, 2, 3, 1 ] } ],"
            "  \"meshes\": [ { \"primitives\": [ { \"attributes\": { \"POSITION\": 0, \"TEXCOORD_0\": 1 }, \"indices\": 2, \"mode\": 5 }, { \"attributes\": { \"POSITION\": 0 }, \"mode\": 0 } ] } ],"
            "  \"buffers\": [ { \"byteLength\": " + CORE::UInt32ToString( (UInt32) binary.size() ) + ", \"uri\": \"data:application/octet-stream;base64," + Base64( binary ) + "\" } ],"
            "  \"bufferViews\": [ { \"buffer\": 0, \"byteOffset\": 0, \"byteLength\": 48 }, { \"buffer\": 0, \"byteOffset\": 48, \"byteLength\": 16 }, { \"buffer\": 0, \"byteOffset\": 64, \"byteLength\": 8 } ],"
            "  \"accessors\": [ { \"bufferView\": 0, \"componentType\": 5126, \"count\": 4, \"type\": \"VEC3\", \"min\": [ 0, 0, 0 ], \"max\": [ 1, 1, 0 ] },"
            "                   { \"bufferView\": 1, \"componentType\": 5123, \"normalized\": true, \"count\": 4, \"type\": \"VEC2\" },"
            "                   { \"bufferView\": 2, \"componentType\": 5123, \"count\": 4, \"type\": \"SCALAR\" } ] }";
        ASSERT_TRUE( WriteWorkText( "strip/strip.gltf", json ) );

        CAssetScene scene;
        CConversionReport report;
        ASSERT_TRUE( ImportFile( "work/strip", "strip.gltf", scene, report ) );
        ASSERT_TRUE( 1 == scene.meshes.size() && 1 == scene.meshes[ 0 ].subMeshes.size() );
        const CAssetSubMesh& subMesh = scene.meshes[ 0 ].subMeshes[ 0 ];
        const UInt32 expected[ 6 ] = { 0, 1, 2, 2, 1, 3 };
        ASSERT_TRUE( 6 == subMesh.indices.size() && 0 == memcmp( &subMesh.indices[ 0 ], expected, sizeof( expected ) ) );
        ASSERT_TRUE( 8 == subMesh.uvSets[ 0 ].size() && Near( subMesh.uvSets[ 0 ][ 2 ], 1.0f ) && Near( subMesh.uvSets[ 0 ][ 6 ], 32768.0f / 65535.0f ) );
        ASSERT_TRUE( report.GetItemCount( REPORTITEM_SKIPPED ) >= 1 );
        const MATH::TFloat32Transform3D& transform = scene.nodes[ 0 ].transform;
        ASSERT_TRUE( Near( transform.position.x, 1.0f ) && Near( transform.position.y, 2.0f ) && Near( transform.position.z, 3.0f ) && Near( transform.scale.x, 2.0f ) && Near( transform.rotation.w, 1.0f ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Import: specular-glossiness is approximated and reported, extension data of a material goes through, required extensions that are not supported are reported" )
    try
    {
        MakeWorkFolder( "material" );
        const CORE::CString json =
            "{ \"asset\": { \"version\": \"2.0\" }, \"extensionsUsed\": [ \"KHR_materials_pbrSpecularGlossiness\", \"KHR_materials_clearcoat\", \"KHR_draco_mesh_compression\" ],"
            "  \"extensionsRequired\": [ \"KHR_draco_mesh_compression\" ],"
            "  \"materials\": [ { \"name\": \"Old\", \"extensions\": { \"KHR_materials_pbrSpecularGlossiness\": { \"diffuseFactor\": [ 0.5, 0.25, 0.125, 1 ], \"glossinessFactor\": 0.25 } } },"
            "                   { \"name\": \"Coated\", \"pbrMetallicRoughness\": { \"metallicFactor\": 0 }, \"extensions\": { \"KHR_materials_clearcoat\": { \"clearcoatFactor\": 0.5 } } } ] }";
        ASSERT_TRUE( WriteWorkText( "material/materials.gltf", json ) );
        CAssetScene scene;
        CConversionReport report;
        ASSERT_TRUE( ImportFile( "work/material", "materials.gltf", scene, report ) );
        ASSERT_TRUE( 2 == scene.materials.size() );
        ASSERT_TRUE( Near( scene.materials[ 0 ].baseColorFactor[ 0 ], 0.5f ) && Near( scene.materials[ 0 ].baseColorFactor[ 1 ], 0.25f ) && Near( scene.materials[ 0 ].roughnessFactor, 0.75f ) && Near( scene.materials[ 0 ].metallicFactor, 0.0f ) );
        ASSERT_TRUE( "glTF/specularGlossiness" == scene.materials[ 0 ].source.shaderName && "glTF/metallicRoughness" == scene.materials[ 1 ].source.shaderName );
        ASSERT_TRUE( report.GetItemCount( REPORTITEM_APPROXIMATION ) >= 1 && report.GetItemCount( REPORTITEM_WARNING ) >= 1 );
        const CORE::CDataNode* clearcoat = scene.materials[ 1 ].extensions.FindChild( "KHR_materials_clearcoat" );
        ASSERT_TRUE( GUCEF_NULL != clearcoat && Near( clearcoat->GetAttributeValueOrChildValueByName( "clearcoatFactor" ).AsFloat32(), 0.5f ) );
        // the facts of the source are in the extras, for the sidecar
        const CORE::CDataNode* facts = scene.extras.FindChild( "gltf" );
        ASSERT_TRUE( GUCEF_NULL != facts && GUCEF_NULL != facts->FindChild( "extensionsRequired" ) && GUCEF_NULL != facts->FindChild( "extensionsUsed" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Import: files that are not glTF, truncated containers and missing buffers fail with an error in the report" )
    try
    {
        MakeWorkFolder( "bad" );
        ASSERT_TRUE( WriteWorkText( "bad/text.gltf", "this is not JSON at all" ) );
        const TBytes truncated = MakeGlb( "{ \"asset\": { \"version\": \"2.0\" } }", TBytes() );
        ASSERT_TRUE( WriteWorkFile( "bad/cut.glb", &truncated[ 0 ], 20 ) );
        ASSERT_TRUE( WriteWorkText( "bad/missing.gltf", "{ \"asset\": { \"version\": \"2.0\" }, \"buffers\": [ { \"byteLength\": 12, \"uri\": \"nowhere.bin\" } ] }" ) );
        ASSERT_TRUE( WriteWorkText( "bad/old.gltf", "{ \"asset\": { \"version\": \"1.0\" } }" ) );
        const char* const files[] = { "text.gltf", "cut.glb", "missing.gltf", "old.gltf" };
        for ( size_t f=0; f<sizeof( files ) / sizeof( files[ 0 ] ); ++f )
        {
            CAssetScene scene;
            CConversionReport report;
            ASSERT_FALSE( ImportFile( "work/bad", files[ f ], scene, report ) );
            ASSERT_TRUE( report.GetItemCount( REPORTITEM_ERROR ) >= 1 );
        }
        CAssetScene scene;
        CConversionReport report;
        ASSERT_FALSE( ImportFile( "work/bad", "absent.gltf", scene, report ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
