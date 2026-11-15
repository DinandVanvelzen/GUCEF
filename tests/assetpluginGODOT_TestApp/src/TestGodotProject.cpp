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

#include "TestGodotProject.h"

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

namespace {

void
AppendFloat( vector< UInt8 >& bytes, float value )
{
    UInt8 raw[ 4 ];
    memcpy( raw, &value, 4 );
    bytes.insert( bytes.end(), raw, raw + 4 );
}

void
AppendU16( vector< UInt8 >& bytes, UInt32 value )
{
    bytes.push_back( (UInt8) ( value & 0xFF ) );
    bytes.push_back( (UInt8) ( ( value >> 8 ) & 0xFF ) );
}

UInt32
ToU16( float value )
{
    const float clamped = value < 0.0f ? 0.0f : ( value > 1.0f ? 1.0f : value );
    return (UInt32) ( clamped * 65535.0f + 0.5f );
}

CORE::CString
Base64Text( const vector< UInt8 >& bytes )
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

}

/*-------------------------------------------------------------------------*/

CORE::CString
GodotNumbers( const double* values, UInt32 count )
{
    CORE::CString text;
    for ( UInt32 i=0; i<count; ++i )
    {
        if ( i > 0 )
            text += ", ";
        text += CORE::DoubleToString( values[ i ] );
    }
    return text;
}

/*-------------------------------------------------------------------------*/

CORE::CString
GodotTransformText( double degreesAboutY, double scale, double x, double y, double z )
{
    const double angle = degreesAboutY * 3.14159265358979323846 / 180.0;
    const double c = cos( angle ) * scale;
    const double s = sin( angle ) * scale;
    // the rows of the basis: rotation about Y, then the origin
    const double values[ 12 ] = { c, 0, s, 0, scale, 0, -s, 0, c, x, y, z };
    return "Transform3D(" + GodotNumbers( values, 12 ) + ")";
}

/*-------------------------------------------------------------------------*/

CORE::CString
GodotBytesText( const vector< UInt8 >& bytes, bool base64 )
{
    if ( base64 )
        return "PackedByteArray(\"" + Base64Text( bytes ) + "\")";
    CORE::CString text = "PackedByteArray(";
    for ( size_t i=0; i<bytes.size(); ++i )
    {
        if ( i > 0 )
            text += ", ";
        text += CORE::UInt32ToString( bytes[ i ] );
    }
    return text + ")";
}

/*-------------------------------------------------------------------------*/

void
GodotOctahedronEncode( float x, float y, float z, float& u, float& v )
{
    const float length = (float) sqrt( x * x + y * y + z * z );
    x /= length; y /= length; z /= length;
    const float sum = (float) ( fabs( x ) + fabs( y ) + fabs( z ) );
    x /= sum; y /= sum; z /= sum;
    float ox = x;
    float oy = y;
    if ( z < 0.0f )
    {
        ox = ( 1.0f - (float) fabs( y ) ) * ( x >= 0.0f ? 1.0f : -1.0f );
        oy = ( 1.0f - (float) fabs( x ) ) * ( y >= 0.0f ? 1.0f : -1.0f );
    }
    u = ox * 0.5f + 0.5f;
    v = oy * 0.5f + 0.5f;
}

/*-------------------------------------------------------------------------*/

SGodotSurface
GodotTriangleSurface( const CORE::CString& material )
{
    SGodotSurface surface;
    const float positions[ 9 ] = { 0, 0, 0,  1, 0, 0,  0, 1, 0 };
    const float uvs[ 6 ] = { 0, 1,  1, 1,  0, 0 };
    for ( UInt32 i=0; i<9; ++i )
    {
        surface.positions.push_back( positions[ i ] );
        surface.normals.push_back( 2 == i % 3 ? 1.0f : 0.0f );
    }
    for ( UInt32 i=0; i<6; ++i )
        surface.uvs.push_back( uvs[ i ] );
    // the winding of Godot is clockwise: seen from +Z this order is clockwise
    surface.indices.push_back( 0 );
    surface.indices.push_back( 2 );
    surface.indices.push_back( 1 );
    surface.material = material;
    return surface;
}

/*-------------------------------------------------------------------------*/

void
GodotSurfaceData( const SGodotSurface& surface, vector< UInt8 >& vertexData, vector< UInt8 >& attributeData, vector< UInt8 >& indexData, UInt64& format )
{
    const UInt32 vertexCount = (UInt32) ( surface.positions.size() / 3 );
    const bool hasNormal = surface.normals.size() == (size_t) vertexCount * 3;
    const bool hasTangent = surface.tangents.size() == (size_t) vertexCount * 4;
    const bool hasUv = surface.uvs.size() == (size_t) vertexCount * 2;
    const bool hasUv2 = surface.uvs2.size() == (size_t) vertexCount * 2;
    const bool hasColour = surface.colours.size() == (size_t) vertexCount * 4;
    const bool hasIndex = !surface.indices.empty();

    format = 1;
    if ( hasNormal ) format |= 2;
    if ( hasTangent ) format |= 4;
    if ( hasColour ) format |= 8;
    if ( hasUv ) format |= 16;
    if ( hasUv2 ) format |= 32;
    if ( hasIndex ) format |= 4096;
    format |= surface.extraFormat;
    format |= (UInt64) 1 << 35;

    // the positions of every vertex, then the normals and tangents
    for ( size_t i=0; i<surface.positions.size(); ++i )
        AppendFloat( vertexData, surface.positions[ i ] );
    for ( UInt32 v=0; v<vertexCount; ++v )
    {
        float a = 0.0f, b = 0.0f;
        if ( hasNormal )
        {
            GodotOctahedronEncode( surface.normals[ v * 3 ], surface.normals[ v * 3 + 1 ], surface.normals[ v * 3 + 2 ], a, b );
            AppendU16( vertexData, ToU16( a ) );
            AppendU16( vertexData, ToU16( b ) );
        }
        if ( hasTangent )
        {
            GodotOctahedronEncode( surface.tangents[ v * 4 ], surface.tangents[ v * 4 + 1 ], surface.tangents[ v * 4 + 2 ], a, b );
            b = b < 0.0001f ? 0.0001f : b;
            b = b * 0.5f + 0.5f;
            if ( surface.tangents[ v * 4 + 3 ] < 0.0f )
                b = 1.0f - b;
            AppendU16( vertexData, ToU16( a ) );
            AppendU16( vertexData, ToU16( b ) );
        }
    }
    for ( UInt32 v=0; v<vertexCount; ++v )
    {
        if ( hasColour )
            for ( UInt32 c=0; c<4; ++c )
                attributeData.push_back( surface.colours[ v * 4 + c ] );
        if ( hasUv )
        {
            AppendFloat( attributeData, surface.uvs[ v * 2 ] );
            AppendFloat( attributeData, surface.uvs[ v * 2 + 1 ] );
        }
        if ( hasUv2 )
        {
            AppendFloat( attributeData, surface.uvs2[ v * 2 ] );
            AppendFloat( attributeData, surface.uvs2[ v * 2 + 1 ] );
        }
    }
    for ( size_t i=0; i<surface.indices.size(); ++i )
        AppendU16( indexData, surface.indices[ i ] );

}

/*-------------------------------------------------------------------------*/

CORE::CString
GodotSurfaceText( const SGodotSurface& surface )
{
    vector< UInt8 > vertexData;
    vector< UInt8 > attributeData;
    vector< UInt8 > indexData;
    UInt64 format = 0;
    GodotSurfaceData( surface, vertexData, attributeData, indexData, format );
    const UInt32 vertexCount = (UInt32) ( surface.positions.size() / 3 );
    CORE::CString text = "{\n";
    text += "\"aabb\": AABB(0, 0, 0, 1, 1, 0),\n";
    if ( !attributeData.empty() )
        text += "\"attribute_data\": " + GodotBytesText( attributeData, surface.useBase64 ) + ",\n";
    text += "\"format\": " + CORE::UInt64ToString( format ) + ",\n";
    text += "\"index_count\": " + CORE::UInt32ToString( (UInt32) surface.indices.size() ) + ",\n";
    if ( !indexData.empty() )
        text += "\"index_data\": " + GodotBytesText( indexData, surface.useBase64 ) + ",\n";
    if ( !surface.material.IsNULLOrEmpty() )
        text += "\"material\": " + surface.material + ",\n";
    text += "\"name\": \"surface\",\n";
    text += "\"primitive\": " + CORE::Int32ToString( surface.primitive ) + ",\n";
    text += "\"uv_scale\": Vector4(0, 0, 0, 0),\n";
    text += "\"vertex_count\": " + CORE::UInt32ToString( vertexCount ) + ",\n";
    text += "\"vertex_data\": " + GodotBytesText( vertexData, surface.useBase64 ) + "\n}";
    return text;
}

/*-------------------------------------------------------------------------*/

CORE::CString
GodotArrayMeshText( const CORE::CString& id, const vector< SGodotSurface >& surfaces )
{
    CORE::CString text = "[sub_resource type=\"ArrayMesh\" id=\"" + id + "\"]\n_surfaces = [";
    for ( size_t s=0; s<surfaces.size(); ++s )
    {
        if ( s > 0 )
            text += ", ";
        text += GodotSurfaceText( surfaces[ s ] );
    }
    text += "]\n";
    return text;
}

/*-------------------------------------------------------------------------*/

bool
WriteGodotProject( const CORE::CString& relativeFolder )
{
    return WriteWorkText( relativeFolder + "/project.godot", "; Engine configuration file.\nconfig_version=5\n\n[application]\n\nconfig/name=\"Test\"\n" );
}

/*-------------------------------------------------------------------------*/

bool
WriteGltfModel( const CORE::CString& vfsFolder, const CORE::CString& name )
{
    CAssetScene scene;
    scene.name = name;
    CAssetMaterial material;
    material.name = "Painted";
    material.baseColorFactor[ 0 ] = 0.8f; material.baseColorFactor[ 1 ] = 0.1f; material.baseColorFactor[ 2 ] = 0.1f;
    material.metallicFactor = 0.2f;
    material.roughnessFactor = 0.7f;
    scene.materials.push_back( material );

    CAssetMesh mesh;
    mesh.name = "TriMesh";
    CAssetSubMesh subMesh;
    const float positions[ 9 ] = { 0, 0, 0,  1, 0, 0,  0, 1, 0 };
    for ( UInt32 i=0; i<9; ++i )
    {
        subMesh.positions.push_back( positions[ i ] );
        subMesh.normals.push_back( 2 == i % 3 ? 1.0f : 0.0f );
    }
    subMesh.indices.push_back( 0 );
    subMesh.indices.push_back( 1 );
    subMesh.indices.push_back( 2 );
    subMesh.materialIndex = 0;
    mesh.subMeshes.push_back( subMesh );
    scene.meshes.push_back( mesh );

    const UInt32 root = scene.AddNode( "Crate" );
    const UInt32 lid = scene.AddNode( "Lid", (Int32) root );
    scene.nodes[ lid ].mesh = 0;
    scene.nodes[ lid ].transform.position = MATH::TFloat32Transform3D::TPositionVector( 0.0f, 1.0f, 0.0f );
    const UInt32 wheel = scene.AddNode( "Wheel", (Int32) root );
    scene.nodes[ wheel ].mesh = 0;
    scene.nodes[ wheel ].transform.position = MATH::TFloat32Transform3D::TPositionVector( 2.0f, 0.0f, 0.0f );

    CAssetAnimation animation;
    animation.name = "Spin";
    CAssetAnimationChannel channel;
    channel.targetNode = wheel;
    channel.path = ANIMATIONPATH_ROTATION;
    channel.times.push_back( 0.0f );
    channel.times.push_back( 1.0f );
    const float rotations[ 8 ] = { 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.70710678f, 0.0f, 0.70710678f };
    for ( UInt32 i=0; i<8; ++i )
        channel.values.push_back( rotations[ i ] );
    animation.channels.push_back( channel );
    scene.animations.push_back( animation );

    TAssetExporterPtr exporter;
    if ( !CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetExporter( "gltf", exporter ) )
        return false;
    VFS::CVfsGlobal::Instance()->GetVfs().CreateDir( vfsFolder );
    CConversionReport report;
    return exporter->Export( scene, vfsFolder, report );
}

/*-------------------------------------------------------------------------*/

CORE::CString
MakeTriangleFbx( bool centimetresZUp )
{
    const double side = centimetresZUp ? 100.0 : 1.0;
    CORE::CString text = "; FBX 7.4.0 project file\n"
                         "FBXHeaderExtension:  {\n    FBXHeaderVersion: 1003\n    FBXVersion: 7400\n    Creator: \"assetpluginGODOT_TestApp\"\n}\n"
                         "GlobalSettings:  {\n    Version: 1000\n    Properties70:  {\n";
    if ( centimetresZUp )
        text += "        P: \"UpAxis\", \"int\", \"Integer\", \"\",2\n        P: \"UpAxisSign\", \"int\", \"Integer\", \"\",1\n"
                "        P: \"FrontAxis\", \"int\", \"Integer\", \"\",1\n        P: \"FrontAxisSign\", \"int\", \"Integer\", \"\",-1\n"
                "        P: \"CoordAxis\", \"int\", \"Integer\", \"\",0\n        P: \"CoordAxisSign\", \"int\", \"Integer\", \"\",1\n"
                "        P: \"UnitScaleFactor\", \"double\", \"Number\", \"\",1\n        P: \"OriginalUnitScaleFactor\", \"double\", \"Number\", \"\",1\n";
    else
        text += "        P: \"UpAxis\", \"int\", \"Integer\", \"\",1\n        P: \"UpAxisSign\", \"int\", \"Integer\", \"\",1\n"
                "        P: \"FrontAxis\", \"int\", \"Integer\", \"\",2\n        P: \"FrontAxisSign\", \"int\", \"Integer\", \"\",1\n"
                "        P: \"CoordAxis\", \"int\", \"Integer\", \"\",0\n        P: \"CoordAxisSign\", \"int\", \"Integer\", \"\",1\n"
                "        P: \"UnitScaleFactor\", \"double\", \"Number\", \"\",100\n        P: \"OriginalUnitScaleFactor\", \"double\", \"Number\", \"\",100\n";
    text += "        P: \"TimeMode\", \"enum\", \"\", \"\",6\n    }\n}\n";
    const double vertices[ 9 ] = { 0, 0, 0, side, 0, 0, 0, side, 0 };
    text += "Objects:  {\n"
            "    Geometry: 1001, \"Geometry::Tri\", \"Mesh\" {\n        Vertices: *9 {\n            a: " + GodotNumbers( vertices, 9 ) + "\n        }\n"
            "        PolygonVertexIndex: *3 {\n            a: 0,1,-3\n        }\n        GeometryVersion: 124\n    }\n"
            "    Model: 2001, \"Model::Tri\", \"Mesh\" {\n        Version: 232\n        Properties70:  {\n"
            "            P: \"Lcl Translation\", \"Lcl Translation\", \"\", \"A\",0,0,0\n        }\n    }\n}\n"
            "Connections:  {\n    C: \"OO\",1001,2001\n    C: \"OO\",2001,0\n}\n";
    return text;
}

/*-------------------------------------------------------------------------*/

bool
ImportGodot( const CORE::CString& vfsFolder, const CORE::CString& assetPath, CAssetScene& scene, CConversionReport& report )
{
    TAssetImporterPtr importer;
    if ( !CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetImporter( "godot", importer ) )
        return false;
    CAssetPackContext context;
    if ( !OpenPackContext( context, vfsFolder, vfsFolder + "_inter" ) )
        return false;
    importer->IndexPack( context );
    scene.name = CVfsPaths::StemOf( assetPath );
    scene.provenance.assetPath = assetPath;
    const bool imported = importer->Import( context.ToVfsPath( assetPath ), context, scene, report );
    if ( !imported )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Import of " << assetPath << " failed: " << report.ToSummaryText() );
    return imported;
}

/*-------------------------------------------------------------------------*/

Int32
RunConverter( const CORE::CString::StringVector& arguments, CConversionReport* report )
{
    CORE::CString::StringVector texts;
    texts.push_back( "AssetConverter" );
    texts.insert( texts.end(), arguments.begin(), arguments.end() );
    vector< char* > pointers;
    for ( size_t i=0; i<texts.size(); ++i )
        pointers.push_back( const_cast< char* >( texts[ i ].C_String() ) );
    pointers.push_back( GUCEF_NULL );
    CAssetConverterApp app( (int) texts.size(), &pointers[ 0 ] );
    if ( !app.ParseCommandLine() )
        return -1;
    const Int32 exitCode = app.Run();
    if ( GUCEF_NULL != report )
        *report = app.GetReport();
    app.Unmount();
    return exitCode;
}

/*-------------------------------------------------------------------------*/

Int32
FindNodeByName( const CAssetScene& scene, const char* name )
{
    return scene.FindNode( name );
}

/*-------------------------------------------------------------------------*/

bool
ReportHas( const CConversionReport& report, TReportItemKind kind, const char* part )
{
    const CConversionReport::TEntryVector& entries = report.GetEntries();
    for ( size_t e=0; e<entries.size(); ++e )
        for ( size_t i=0; i<entries[ e ].items.size(); ++i )
            if ( entries[ e ].items[ i ].kind == kind && ( entries[ e ].items[ i ].message.HasSubstr( part ) >= 0 || entries[ e ].items[ i ].subject.HasSubstr( part ) >= 0 ) )
                return true;
    return false;
}

/*-------------------------------------------------------------------------*/
