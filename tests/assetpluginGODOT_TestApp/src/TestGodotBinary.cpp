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

#include "assetpluginGODOT_CGodotBinaryReader.h"
#include "assetpluginGODOT_CGodotVariant.h"
#include "assetpluginGODOT_CGodotDocument.h"
#include "assetpluginGODOT_CGodotImportSettings.h"
#include "assetpluginGODOT_CGodotTextReader.h"
#include "assetpluginGODOT_CGodotPaths.h"

#include "TestGodotProject.h"
#include "TestGodotBinary.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETCONVERTER;
using namespace GUCEF::ASSETPLUGIN::GODOT;

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

typedef vector< UInt8 > TBytes;

void Put32( TBytes& bytes, UInt32 value ) { for ( UInt32 i=0; i<4; ++i ) bytes.push_back( (UInt8) ( ( value >> ( 8 * i ) ) & 0xFF ) ); }
void Put64( TBytes& bytes, UInt64 value ) { Put32( bytes, (UInt32) ( value & 0xFFFFFFFF ) ); Put32( bytes, (UInt32) ( value >> 32 ) ); }
void PutFloat( TBytes& bytes, float value ) { UInt32 raw; memcpy( &raw, &value, 4 ); Put32( bytes, raw ); }
void PutString( TBytes& bytes, const CORE::CString& text )
{
    Put32( bytes, (UInt32) text.ByteSize() );
    for ( UInt32 i=0; i<text.ByteSize(); ++i )
        bytes.push_back( (UInt8) text.C_String()[ i ] );
}
void Append( TBytes& bytes, const TBytes& more ) { bytes.insert( bytes.end(), more.begin(), more.end() ); }

/** The values of a resource file as the file has them */
TBytes VNil( void ) { TBytes b; Put32( b, 1 ); return b; }
TBytes VBool( bool v ) { TBytes b; Put32( b, 2 ); Put32( b, v ? 1 : 0 ); return b; }
TBytes VInt( Int32 v ) { TBytes b; Put32( b, 3 ); Put32( b, (UInt32) v ); return b; }
TBytes VInt64( Int64 v ) { TBytes b; Put32( b, 40 ); Put64( b, (UInt64) v ); return b; }
TBytes VFloat( float v ) { TBytes b; Put32( b, 4 ); PutFloat( b, v ); return b; }
TBytes VDouble( double v ) { TBytes b; Put32( b, 41 ); UInt64 raw; memcpy( &raw, &v, 8 ); Put64( b, raw ); return b; }
TBytes VString( const CORE::CString& v ) { TBytes b; Put32( b, 5 ); PutString( b, v ); return b; }
TBytes VName( const CORE::CString& v ) { TBytes b; Put32( b, 44 ); PutString( b, v ); return b; }
TBytes VFloats( UInt32 type, const float* values, UInt32 count ) { TBytes b; Put32( b, type ); for ( UInt32 i=0; i<count; ++i ) PutFloat( b, values[ i ] ); return b; }
TBytes VVector3( float x, float y, float z ) { const float v[ 3 ] = { x, y, z }; return VFloats( 12, v, 3 ); }
TBytes VVector4( float x, float y, float z, float w ) { const float v[ 4 ] = { x, y, z, w }; return VFloats( 50, v, 4 ); }
TBytes VColour( float r, float g, float bl, float a ) { const float v[ 4 ] = { r, g, bl, a }; return VFloats( 20, v, 4 ); }
TBytes VAabb( float x, float y, float z, float sx, float sy, float sz ) { const float v[ 6 ] = { x, y, z, sx, sy, sz }; return VFloats( 15, v, 6 ); }
TBytes VObjectSub( UInt32 index ) { TBytes b; Put32( b, 24 ); Put32( b, 2 ); Put32( b, index ); return b; }
TBytes VObjectExt( UInt32 index ) { TBytes b; Put32( b, 24 ); Put32( b, 3 ); Put32( b, index ); return b; }
TBytes VObjectNull( void ) { TBytes b; Put32( b, 24 ); Put32( b, 0 ); return b; }
TBytes VBytes( const TBytes& v ) { TBytes b; Put32( b, 31 ); Put32( b, (UInt32) v.size() ); Append( b, v ); while ( 0 != b.size() % 4 ) b.push_back( 0 ); return b; }
TBytes VArray( const vector< TBytes >& elements ) { TBytes b; Put32( b, 30 ); Put32( b, (UInt32) elements.size() ); for ( size_t i=0; i<elements.size(); ++i ) Append( b, elements[ i ] ); return b; }
TBytes VDictionary( const vector< TBytes >& keysAndValues ) { TBytes b; Put32( b, 26 ); Put32( b, (UInt32) ( keysAndValues.size() / 2 ) ); for ( size_t i=0; i<keysAndValues.size(); ++i ) Append( b, keysAndValues[ i ] ); return b; }
TBytes VPacked( UInt32 type, const TBytes& raw, UInt32 count ) { TBytes b; Put32( b, type ); Put32( b, count ); Append( b, raw ); return b; }

struct SProperty
{
    CORE::CString name;
    TBytes value;
    SProperty( const char* n, const TBytes& v ) : name( n ), value( v ) {}
};
typedef vector< SProperty > TProperties;

/** A binary resource file: the tables and the resources, the last of which is the main one */
class CRsrcBuilder
{
    public:

    CRsrcBuilder( void ) : m_strings(), m_externals(), m_internals() {}

    UInt32 String( const CORE::CString& text )
    {
        for ( size_t i=0; i<m_strings.size(); ++i )
            if ( m_strings[ i ] == text )
                return (UInt32) i;
        m_strings.push_back( text );
        return (UInt32) m_strings.size() - 1;
    }

    UInt32 AddExternal( const CORE::CString& type, const CORE::CString& path, UInt64 uid )
    {
        SExternal external;
        external.type = type;
        external.path = path;
        external.uid = uid;
        m_externals.push_back( external );
        return (UInt32) m_externals.size() - 1;
    }

    UInt32 AddInternal( const CORE::CString& id, const CORE::CString& type, const TProperties& properties )
    {
        SInternal internal;
        internal.id = id;
        PutString( internal.data, type );
        Put32( internal.data, (UInt32) properties.size() );
        for ( size_t p=0; p<properties.size(); ++p )
        {
            Put32( internal.data, String( properties[ p ].name ) );
            Append( internal.data, properties[ p ].value );
        }
        m_internals.push_back( internal );
        return (UInt32) m_internals.size() - 1;
    }

    /** A node path as the file has it: the names and subnames are places in the string table */
    TBytes NodePath( const CORE::CString& name, const CORE::CString& subname )
    {
        TBytes b;
        Put32( b, 22 );
        b.push_back( 1 ); b.push_back( 0 );
        b.push_back( subname.IsNULLOrEmpty() ? 0 : 1 ); b.push_back( 0 );
        Put32( b, 0 );
        Put32( b, String( name ) );
        if ( !subname.IsNULLOrEmpty() )
            Put32( b, String( subname ) );
        return b;
    }

    TBytes Build( const CORE::CString& mainType, UInt64 uid, bool real64 = false, UInt32 format = 6 )
    {
        TBytes file;
        file.push_back( 'R' ); file.push_back( 'S' ); file.push_back( 'R' ); file.push_back( 'C' );
        Put32( file, 0 );
        Put32( file, real64 ? 1 : 0 );
        Put32( file, 4 );
        Put32( file, 5 );
        Put32( file, format );
        PutString( file, mainType );
        Put64( file, 0 );
        Put32( file, 3 );
        Put64( file, uid );
        for ( UInt32 r=0; r<11; ++r )
            Put32( file, 0 );
        Put32( file, (UInt32) m_strings.size() );
        for ( size_t s=0; s<m_strings.size(); ++s )
            PutString( file, m_strings[ s ] );
        Put32( file, (UInt32) m_externals.size() );
        for ( size_t e=0; e<m_externals.size(); ++e )
        {
            PutString( file, m_externals[ e ].type );
            PutString( file, m_externals[ e ].path );
            Put64( file, m_externals[ e ].uid );
        }
        Put32( file, (UInt32) m_internals.size() );
        UInt32 tableBytes = 0;
        for ( size_t i=0; i<m_internals.size(); ++i )
            tableBytes += 4 + (UInt32) CORE::CString( "local://" + m_internals[ i ].id ).ByteSize() + 8;
        UInt64 offset = file.size() + tableBytes;
        for ( size_t i=0; i<m_internals.size(); ++i )
        {
            PutString( file, "local://" + m_internals[ i ].id );
            Put64( file, offset );
            offset += m_internals[ i ].data.size();
        }
        for ( size_t i=0; i<m_internals.size(); ++i )
            Append( file, m_internals[ i ].data );
        file.push_back( 'R' ); file.push_back( 'S' ); file.push_back( 'R' ); file.push_back( 'C' );
        return file;
    }

    private:

    struct SExternal { CORE::CString type; CORE::CString path; UInt64 uid; };
    struct SInternal { CORE::CString id; TBytes data; };

    vector< CORE::CString > m_strings;
    vector< SExternal > m_externals;
    vector< SInternal > m_internals;
};

/** The dictionary of a surface as an ArrayMesh file has it */
TBytes
SurfaceDictionary( const SGodotSurface& surface, Int32 materialIndex )
{
    TBytes vertexData;
    TBytes attributeData;
    TBytes indexData;
    UInt64 format = 0;
    GodotSurfaceData( surface, vertexData, attributeData, indexData, format );
    vector< TBytes > entries;
    entries.push_back( VString( "format" ) ); entries.push_back( VInt64( (Int64) format ) );
    entries.push_back( VString( "primitive" ) ); entries.push_back( VInt( surface.primitive ) );
    entries.push_back( VString( "vertex_data" ) ); entries.push_back( VBytes( vertexData ) );
    entries.push_back( VString( "vertex_count" ) ); entries.push_back( VInt( (Int32) ( surface.positions.size() / 3 ) ) );
    if ( !attributeData.empty() )
    {
        entries.push_back( VString( "attribute_data" ) ); entries.push_back( VBytes( attributeData ) );
    }
    entries.push_back( VString( "aabb" ) ); entries.push_back( VAabb( 0, 0, 0, 1, 1, 0 ) );
    entries.push_back( VString( "uv_scale" ) ); entries.push_back( VVector4( 0, 0, 0, 0 ) );
    if ( !indexData.empty() )
    {
        entries.push_back( VString( "index_data" ) ); entries.push_back( VBytes( indexData ) );
        entries.push_back( VString( "index_count" ) ); entries.push_back( VInt( (Int32) surface.indices.size() ) );
    }
    if ( materialIndex >= 0 )
    {
        entries.push_back( VString( "material" ) ); entries.push_back( VObjectSub( (UInt32) materialIndex ) );
    }
    return VDictionary( entries );
}

/**
 *  An ArrayMesh resource file as the editor of Godot 4 saves an extracted mesh: the material the model had as an
 *  internal resource first, the mesh last, the surfaces pointing at the material.
 */
bool
WriteBinaryMesh( const CORE::CString& relativePath, UInt64 uid, const vector< SGodotSurface >& surfaces, const char* materialName, bool blendShapeNames )
{
    CRsrcBuilder builder;
    TProperties materialProperties;
    materialProperties.push_back( SProperty( "resource_name", VString( materialName ) ) );
    materialProperties.push_back( SProperty( "vertex_color_use_as_albedo", VBool( true ) ) );
    materialProperties.push_back( SProperty( "albedo_color", VColour( 0.5f, 0.5f, 0.5f, 1.0f ) ) );
    materialProperties.push_back( SProperty( "script", VNil() ) );
    builder.AddInternal( CORE::CString( "StandardMaterial3D_" ) + materialName, "StandardMaterial3D", materialProperties );

    vector< TBytes > surfaceValues;
    for ( size_t s=0; s<surfaces.size(); ++s )
        surfaceValues.push_back( SurfaceDictionary( surfaces[ s ], 0 ) );
    TProperties meshProperties;
    meshProperties.push_back( SProperty( "resource_name", VString( "" ) ) );
    if ( blendShapeNames )
    {
        TBytes names;
        PutString( names, "smile" );
        meshProperties.push_back( SProperty( "_blend_shape_names", VPacked( 34, names, 1 ) ) );
    }
    meshProperties.push_back( SProperty( "_surfaces", VArray( surfaceValues ) ) );
    meshProperties.push_back( SProperty( "blend_shape_mode", VInt( 1 ) ) );
    meshProperties.push_back( SProperty( "custom_aabb", VAabb( 0, 0, 0, 0, 0, 0 ) ) );
    meshProperties.push_back( SProperty( "shadow_mesh", VObjectNull() ) );
    meshProperties.push_back( SProperty( "script", VNil() ) );
    builder.AddInternal( "ArrayMesh_main", "ArrayMesh", meshProperties );
    const TBytes file = builder.Build( "ArrayMesh", uid );
    return WriteWorkFile( relativePath, &file[ 0 ], (UInt32) file.size() );
}

/** A convex shape resource file */
bool
WriteBinaryConvexShape( const CORE::CString& relativePath, UInt64 uid )
{
    CRsrcBuilder builder;
    TBytes points;
    const float values[ 12 ] = { 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 2 };
    for ( UInt32 i=0; i<12; ++i )
        PutFloat( points, values[ i ] );
    TProperties properties;
    properties.push_back( SProperty( "resource_name", VString( "" ) ) );
    properties.push_back( SProperty( "margin", VFloat( 0.04f ) ) );
    properties.push_back( SProperty( "points", VPacked( 35, points, 4 ) ) );
    builder.AddInternal( "ConvexPolygonShape3D_main", "ConvexPolygonShape3D", properties );
    const TBytes file = builder.Build( "ConvexPolygonShape3D", uid );
    return WriteWorkFile( relativePath, &file[ 0 ], (UInt32) file.size() );
}

UInt32
Crc32( const TBytes& data )
{
    UInt32 crc = 0xFFFFFFFF;
    for ( size_t i=0; i<data.size(); ++i )
    {
        crc ^= data[ i ];
        for ( UInt32 k=0; k<8; ++k )
            crc = ( crc >> 1 ) ^ ( 0xEDB88320 & ( 0 - ( crc & 1 ) ) );
    }
    return ~crc;
}

void Put16( TBytes& bytes, UInt32 value ) { bytes.push_back( (UInt8) ( value & 0xFF ) ); bytes.push_back( (UInt8) ( ( value >> 8 ) & 0xFF ) ); }

/** A zip file with the entries stored as they are */
class CZipBuilder
{
    public:

    void Add( const CORE::CString& name, const TBytes& content )
    {
        SEntry entry;
        entry.name = name;
        entry.crc = Crc32( content );
        entry.size = (UInt32) content.size();
        entry.offset = (UInt32) m_data.size();
        Put32( m_data, 0x04034B50 ); Put16( m_data, 20 ); Put16( m_data, 0 ); Put16( m_data, 0 ); Put16( m_data, 0 ); Put16( m_data, 0x21 );
        Put32( m_data, entry.crc ); Put32( m_data, entry.size ); Put32( m_data, entry.size );
        Put16( m_data, (UInt32) name.Length() ); Put16( m_data, 0 );
        for ( UInt32 i=0; i<name.Length(); ++i ) m_data.push_back( (UInt8) name.C_String()[ i ] );
        Append( m_data, content );
        m_entries.push_back( entry );
    }

    void AddText( const CORE::CString& name, const CORE::CString& text )
    {
        TBytes content;
        for ( UInt32 i=0; i+1<text.ByteSize(); ++i )
            content.push_back( (UInt8) text.C_String()[ i ] );
        Add( name, content );
    }

    TBytes Finish( void )
    {
        TBytes file = m_data;
        const UInt32 directoryStart = (UInt32) file.size();
        for ( size_t e=0; e<m_entries.size(); ++e )
        {
            const SEntry& entry = m_entries[ e ];
            Put32( file, 0x02014B50 ); Put16( file, 20 ); Put16( file, 20 ); Put16( file, 0 ); Put16( file, 0 ); Put16( file, 0 ); Put16( file, 0x21 );
            Put32( file, entry.crc ); Put32( file, entry.size ); Put32( file, entry.size );
            Put16( file, (UInt32) entry.name.Length() ); Put16( file, 0 ); Put16( file, 0 ); Put16( file, 0 ); Put16( file, 0 ); Put32( file, 0 ); Put32( file, entry.offset );
            for ( UInt32 i=0; i<entry.name.Length(); ++i ) file.push_back( (UInt8) entry.name.C_String()[ i ] );
        }
        const UInt32 directorySize = (UInt32) file.size() - directoryStart;
        Put32( file, 0x06054B50 ); Put16( file, 0 ); Put16( file, 0 ); Put16( file, (UInt32) m_entries.size() ); Put16( file, (UInt32) m_entries.size() );
        Put32( file, directorySize ); Put32( file, directoryStart ); Put16( file, 0 );
        return file;
    }

    private:

    struct SEntry { CORE::CString name; UInt32 crc; UInt32 size; UInt32 offset; };
    TBytes m_data;
    vector< SEntry > m_entries;
};

TBytes
ToBytes( const CORE::CString& text )
{
    TBytes content;
    for ( UInt32 i=0; i+1<text.ByteSize(); ++i )
        content.push_back( (UInt8) text.C_String()[ i ] );
    return content;
}

const UInt64 MeshUid = 1234567890123456789ULL;
const UInt64 OtherUid = 1000000007ULL;
const UInt64 ShapeUid = 99999999999ULL;

/** The three files of a pack that the real projects have: a text material, a texture and the settings of a model that is not there */
bool
WriteSupportFiles( const CORE::CString& folder )
{
    return WriteGodotProject( folder ) &&
           WriteTestPng( "work/" + folder + "/Assets/Pack/Textures/T.png", 4, 4, 100, 150, 200, 255 ) &&
           WriteTestPng( "work/" + folder + "/Assets/Pack/Textures/E.png", 4, 4, 255, 255, 255, 255 ) &&
           WriteWorkText( folder + "/Assets/Pack/Textures/T.png.import", "[remap]\n\nimporter=\"texture\"\ntype=\"CompressedTexture2D\"\nuid=\"uid://tex_t\"\n" ) &&
           WriteWorkText( folder + "/Assets/Pack/Models/Thing.fbx.import", "[remap]\n\nimporter=\"scene\"\nimporter_version=1\ntype=\"PackedScene\"\nuid=\"uid://model_thing\"\n\n[deps]\n\nsource_file=\"res://Assets/Pack/Models/Thing.fbx\"\n\n[params]\n\nnodes/root_script=null\nnodes/root_scale=1.0\nmeshes/ensure_tangents=1\nanimation/import=false\n"
                           "_subresources={\n\"animations\": {},\n\"materials\": {},\n\"meshes\": {\n\"Root Scene_Thing\": {\n\"save_to_file/enabled\": true,\n\"save_to_file/fallback_path\": \"res://Assets/Pack/Models/extracted/Thing.mesh\",\n\"save_to_file/path\": \"uid://thing_mesh\"\n}\n},\n\"nodes\": {\n\"PATH:AnimationPlayer\": {\n\"optimizer/enabled\": true,\n\"optimizer/max_angular_error\": 0.00277777777777777\n}\n}\n}\nfbx/importer=0\n" ) &&
           WriteWorkText( folder + "/Assets/Pack/Materials/Mat_01.tres",
                           "[gd_resource type=\"StandardMaterial3D\" load_steps=3 format=3 uid=\"uid://mat_one\"]\n\n"
                           "[ext_resource type=\"Texture2D\" uid=\"uid://tex_t\" path=\"res://assets/Pack/Textures/T.png\" id=\"1_a\"]\n"
                           "[ext_resource type=\"Texture2D\" path=\"res://Assets/Pack/Textures/E.png\" id=\"2_b\"]\n\n"
                           "[resource]\nresource_name = \"Mat_01\"\nalbedo_texture = ExtResource(\"1_a\")\nemission_enabled = true\nemission = Color(1, 1, 1, 1)\nemission_operator = 1\nemission_texture = ExtResource(\"2_b\")\n" );
}

/** The mesh files of the pack: the mesh of the thing, another one, a shape */
bool
WriteMeshFiles( const CORE::CString& folder )
{
    vector< SGodotSurface > thing;
    SGodotSurface surface = GodotTriangleSurface( "" );
    for ( UInt32 v=0; v<3; ++v )
        for ( UInt32 c=0; c<4; ++c )
            surface.tangents.push_back( 0 == c ? 1.0f : ( 3 == c ? -1.0f : 0.0f ) );
    surface.uvs2 = surface.uvs;
    thing.push_back( surface );
    vector< SGodotSurface > other;
    other.push_back( GodotTriangleSurface( "" ) );
    return WriteBinaryMesh( folder + "/Assets/Pack/Models/extracted/Thing.res", MeshUid, thing, "ThingMaterial", false ) &&
           WriteBinaryMesh( folder + "/Assets/Pack/Models/extracted/Other.res", OtherUid, other, "OtherMaterial", false ) &&
           WriteBinaryConvexShape( folder + "/Assets/Pack/Models/Collision/Shape.res", ShapeUid );
}

CORE::CString
Uid( UInt64 value )
{
    return CGodotBinaryReader::UidToText( value );
}

/** The prefab of the thing the way a project of Godot 4 saves it: the paths with another letter case, uids, the override of the material, a body with a shape */
CORE::CString
ThingScene( void )
{
    return "[gd_scene load_steps=5 format=3 uid=\"uid://scene_thing\"]\n\n"
           "[ext_resource type=\"ArrayMesh\" uid=\"" + Uid( MeshUid ) + "\" path=\"res://assets/Pack/Models/extracted/Thing.res\" id=\"1_a\"]\n"
           "[ext_resource type=\"Material\" uid=\"uid://mat_one\" path=\"res://Assets/Pack/Materials/Mat_01.tres\" id=\"2_m\"]\n"
           "[ext_resource type=\"ArrayMesh\" uid=\"" + Uid( OtherUid ) + "\" path=\"res://moved/away/Other.res\" id=\"3_o\"]\n"
           "[ext_resource type=\"Shape3D\" path=\"res://Assets/Pack/Models/Collision/Shape.res\" id=\"4_s\"]\n"
           "[ext_resource type=\"Skin\" uid=\"uid://skin_one\" path=\"res://Assets/Pack/Models/extracted/Thing_skin.tres\" id=\"5_k\"]\n\n"
           "[node name=\"Thing\" type=\"MeshInstance3D\"]\ngi_mode = 2\nmesh = ExtResource(\"1_a\")\nsurface_material_override/0 = ExtResource(\"2_m\")\n\n"
           "[node name=\"MeshCollider\" type=\"StaticBody3D\" parent=\".\"]\n\n"
           "[node name=\"CollisionShape3D\" type=\"CollisionShape3D\" parent=\"MeshCollider\"]\nshape = ExtResource(\"4_s\")\n\n"
           "[node name=\"Other\" type=\"MeshInstance3D\" parent=\".\"]\nmesh = ExtResource(\"3_o\")\nskeleton = NodePath(\"\")\n";
}

bool
SetUpRealPack( const CORE::CString& folder )
{
    MakeWorkFolder( folder );
    return WriteSupportFiles( folder ) && WriteMeshFiles( folder ) && WriteWorkText( folder + "/Assets/Pack/Prefabs/Thing.tscn", ThingScene() );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGodotBinaryTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetpluginGODOT.Binary" );

    GUCEF_TESTFW_TESTCASE( "Binary resources: the tables, the sections and every kind of value of a resource file as the text form gives them" )
    try
    {
        CRsrcBuilder builder;
        const UInt32 external = builder.AddExternal( "Texture2D", "res://Assets/a.png", 34 );
        builder.AddExternal( "Material", "res://Assets/m.tres", 0 );
        TProperties inner;
        inner.push_back( SProperty( "resource_name", VString( "inner" ) ) );
        const UInt32 innerIndex = builder.AddInternal( "Inner_1", "StandardMaterial3D", inner );
        TBytes int32s; Put32( int32s, (UInt32) -5 ); Put32( int32s, 70000 );
        TBytes int64s; Put64( int64s, 5000000000ULL );
        TBytes floats; PutFloat( floats, 0.5f ); PutFloat( floats, 1.5f ); PutFloat( floats, 2.5f );
        TBytes doubles; { double d = 0.1; UInt64 raw; memcpy( &raw, &d, 8 ); Put64( doubles, raw ); }
        TBytes colours; for ( UInt32 i=0; i<8; ++i ) PutFloat( colours, 0.125f * i );
        TBytes strings; PutString( strings, "alpha" ); PutString( strings, "beta" );
        TBytes five; for ( UInt32 i=0; i<5; ++i ) five.push_back( (UInt8) ( 10 + i ) );
        vector< TBytes > list;
        list.push_back( VInt( 1 ) ); list.push_back( VString( "two" ) ); list.push_back( VBool( true ) );
        vector< TBytes > dictionary;
        dictionary.push_back( VString( "key" ) ); dictionary.push_back( VFloat( 0.25f ) );
        dictionary.push_back( VInt( 7 ) ); dictionary.push_back( VString( "seven" ) );
        const float transform[ 12 ] = { 1, 0, 0, 0, 1, 0, 0, 0, 1, 3, 4, 5 };
        const float plane[ 4 ] = { 0, 1, 0, 2 };
        const float quaternion[ 4 ] = { 0, 0, 0, 1 };
        const float rect[ 4 ] = { 1, 2, 3, 4 };
        const float vector2[ 2 ] = { 5, 6 };
        const float basis[ 9 ] = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };
        TProperties main;
        main.push_back( SProperty( "nothing", VNil() ) );
        main.push_back( SProperty( "flag", VBool( true ) ) );
        main.push_back( SProperty( "small", VInt( -7 ) ) );
        main.push_back( SProperty( "big", VInt64( -5000000000LL ) ) );
        main.push_back( SProperty( "single", VFloat( 1.25f ) ) );
        main.push_back( SProperty( "double", VDouble( 2.5 ) ) );
        main.push_back( SProperty( "text", VString( "hello" ) ) );
        main.push_back( SProperty( "name", VName( "named" ) ) );
        main.push_back( SProperty( "vector2", VFloats( 10, vector2, 2 ) ) );
        main.push_back( SProperty( "rect", VFloats( 11, rect, 4 ) ) );
        main.push_back( SProperty( "vector3", VVector3( 1, 2, 3 ) ) );
        main.push_back( SProperty( "plane", VFloats( 13, plane, 4 ) ) );
        main.push_back( SProperty( "quaternion", VFloats( 14, quaternion, 4 ) ) );
        main.push_back( SProperty( "box", VAabb( 1, 2, 3, 4, 5, 6 ) ) );
        main.push_back( SProperty( "basis", VFloats( 16, basis, 9 ) ) );
        main.push_back( SProperty( "transform", VFloats( 17, transform, 12 ) ) );
        main.push_back( SProperty( "vector4", VVector4( 1, 2, 3, 4 ) ) );
        main.push_back( SProperty( "colour", VColour( 0.5f, 0.25f, 1.0f, 0.75f ) ) );
        main.push_back( SProperty( "path", builder.NodePath( "Armature/Skeleton3D", "bone" ) ) );
        main.push_back( SProperty( "subresource", VObjectSub( innerIndex ) ) );
        main.push_back( SProperty( "external", VObjectExt( external ) ) );
        main.push_back( SProperty( "empty", VObjectNull() ) );
        main.push_back( SProperty( "list", VArray( list ) ) );
        main.push_back( SProperty( "dictionary", VDictionary( dictionary ) ) );
        main.push_back( SProperty( "bytes", VBytes( five ) ) );
        main.push_back( SProperty( "int32s", VPacked( 32, int32s, 2 ) ) );
        main.push_back( SProperty( "int64s", VPacked( 48, int64s, 1 ) ) );
        main.push_back( SProperty( "floats", VPacked( 33, floats, 3 ) ) );
        main.push_back( SProperty( "doubles", VPacked( 49, doubles, 1 ) ) );
        main.push_back( SProperty( "vectors", VPacked( 35, floats, 1 ) ) );
        main.push_back( SProperty( "colours", VPacked( 36, colours, 2 ) ) );
        main.push_back( SProperty( "strings", VPacked( 34, strings, 2 ) ) );
        builder.AddInternal( "Main_1", "Resource", main );
        const TBytes file = builder.Build( "ResourceType", 1000 );

        CORE::CDataNode root;
        CGodotBinaryReader reader;
        REQUIRE( reader.Parse( &file[ 0 ], (UInt32) file.size(), root ) );
        ASSERT_TRUE( "gd_resource" == root.GetAttributeValue( "kind" ).AsString() );

        // the sections
        const CORE::CDataNode* header = root.GetFirstChild();
        REQUIRE( GUCEF_NULL != header );
        ASSERT_TRUE( "gd_resource" == header->GetAttributeValue( "tag" ).AsString() && "ResourceType" == header->GetAttributeValue( "type" ).AsString() && 3 == header->GetAttributeValue( "format" ).AsInt32() && Uid( 1000 ) == header->GetAttributeValue( "uid" ).AsString() );
        CGodotDocument document;
        REQUIRE( document.LoadText( "[gd_resource type=\"X\" format=3]\n", "x.tres" ) );
        const CORE::CDataNode* ext0 = root.GetFirstChild()->GetNext();
        ASSERT_TRUE( "ext_resource" == ext0->GetAttributeValue( "tag" ).AsString() && "res://Assets/a.png" == ext0->GetAttributeValue( "path" ).AsString() && "0" == ext0->GetAttributeValue( "id" ).AsString() && Uid( 34 ) == ext0->GetAttributeValue( "uid" ).AsString() );
        ASSERT_FALSE( ext0->GetNext()->IsAttribute( "uid" ) );
        const CORE::CDataNode* sub = ext0->GetNext()->GetNext();
        ASSERT_TRUE( "sub_resource" == sub->GetAttributeValue( "tag" ).AsString() && "Inner_1" == sub->GetAttributeValue( "id" ).AsString() && "StandardMaterial3D" == sub->GetAttributeValue( "type" ).AsString() );
        ASSERT_TRUE( "inner" == CGodotVariant::GetString( CGodotVariant::FindProperty( *sub, "resource_name" ) ) );
        const CORE::CDataNode* mainSection = sub->GetNext();
        ASSERT_TRUE( "resource" == mainSection->GetAttributeValue( "tag" ).AsString() && GUCEF_NULL == mainSection->GetNext() );

        // the values
        const CORE::CDataNode& m = *mainSection;
        ASSERT_TRUE( CGodotVariant::IsType( CGodotVariant::FindProperty( m, "nothing" ), "null" ) && CGodotVariant::GetBool( CGodotVariant::FindProperty( m, "flag" ), false ) );
        ASSERT_TRUE( -7 == CGodotVariant::GetInteger( CGodotVariant::FindProperty( m, "small" ), 0 ) && -5000000000LL == CGodotVariant::GetInteger( CGodotVariant::FindProperty( m, "big" ), 0 ) );
        ASSERT_TRUE( Near( (Float32) CGodotVariant::GetNumber( CGodotVariant::FindProperty( m, "single" ), 0.0 ), 1.25f ) && Near( (Float32) CGodotVariant::GetNumber( CGodotVariant::FindProperty( m, "double" ), 0.0 ), 2.5f ) );
        ASSERT_TRUE( "hello" == CGodotVariant::GetString( CGodotVariant::FindProperty( m, "text" ) ) && CGodotVariant::IsType( CGodotVariant::FindProperty( m, "name" ), "StringName" ) );
        const char* const geometric[][ 2 ] = { { "vector2", "Vector2" }, { "rect", "Rect2" }, { "vector3", "Vector3" }, { "plane", "Plane" }, { "quaternion", "Quaternion" }, { "box", "AABB" }, { "basis", "Basis" }, { "transform", "Transform3D" }, { "vector4", "Vector4" }, { "colour", "Color" } };
        const UInt32 counts[ 10 ] = { 2, 4, 3, 4, 4, 6, 9, 12, 4, 4 };
        for ( UInt32 g=0; g<10; ++g )
        {
            CGodotVariant::TNumberVector numbers;
            ASSERT_TRUE( CGodotVariant::IsType( CGodotVariant::FindProperty( m, geometric[ g ][ 0 ] ), geometric[ g ][ 1 ] ) && CGodotVariant::GetNumbers( CGodotVariant::FindProperty( m, geometric[ g ][ 0 ] ), numbers ) && counts[ g ] == numbers.size() );
        }
        Float32 colour[ 4 ];
        ASSERT_TRUE( CGodotVariant::GetColour( CGodotVariant::FindProperty( m, "colour" ), colour ) && Near( colour[ 1 ], 0.25f ) && Near( colour[ 3 ], 0.75f ) );
        MATH::TFloat32Transform3D placed;
        ASSERT_TRUE( CGodotVariant::GetTransform( CGodotVariant::FindProperty( m, "transform" ), placed ) && Near( placed.position.z, 5.0f ) );
        ASSERT_TRUE( "Armature/Skeleton3D:bone" == CGodotVariant::GetString( CGodotVariant::FindProperty( m, "path" ) ) );
        CORE::CString kind;
        CORE::CString id;
        ASSERT_TRUE( CGodotVariant::GetReference( CGodotVariant::FindProperty( m, "subresource" ), kind, id ) && "SubResource" == kind && "Inner_1" == id );
        ASSERT_TRUE( CGodotVariant::GetReference( CGodotVariant::FindProperty( m, "external" ), kind, id ) && "ExtResource" == kind && "0" == id );
        ASSERT_TRUE( CGodotVariant::IsType( CGodotVariant::FindProperty( m, "empty" ), "null" ) );
        const CORE::CDataNode* listValue = CGodotVariant::FindProperty( m, "list" );
        ASSERT_TRUE( CGodotVariant::IsType( listValue, "Array" ) && 3 == listValue->GetNrOfDirectChildNodes() && "two" == CGodotVariant::GetString( listValue->GetFirstChild()->GetNext() ) );
        const CORE::CDataNode* dictionaryValue = CGodotVariant::FindProperty( m, "dictionary" );
        ASSERT_TRUE( Near( (Float32) CGodotVariant::GetNumber( CGodotVariant::FindMember( dictionaryValue, "key" ), 0.0 ), 0.25f ) && "seven" == CGodotVariant::GetString( CGodotVariant::FindMember( dictionaryValue, "7" ) ) );
        CGodotVariant::TByteVector bytes;
        ASSERT_TRUE( CGodotVariant::GetBytes( CGodotVariant::FindProperty( m, "bytes" ), bytes ) && 5 == bytes.size() && 14 == bytes[ 4 ] );
        CGodotVariant::TNumberVector numbers;
        ASSERT_TRUE( CGodotVariant::GetNumbers( CGodotVariant::FindProperty( m, "int32s" ), numbers ) && 2 == numbers.size() && -5.0 == numbers[ 0 ] && 70000.0 == numbers[ 1 ] );
        ASSERT_TRUE( CGodotVariant::GetNumbers( CGodotVariant::FindProperty( m, "int64s" ), numbers ) && 5000000000.0 == numbers[ 0 ] );
        ASSERT_TRUE( CGodotVariant::GetNumbers( CGodotVariant::FindProperty( m, "floats" ), numbers ) && 3 == numbers.size() && 2.5 == numbers[ 2 ] );
        ASSERT_TRUE( CGodotVariant::GetNumbers( CGodotVariant::FindProperty( m, "doubles" ), numbers ) && fabs( numbers[ 0 ] - 0.1 ) < 1e-15 );
        ASSERT_TRUE( CGodotVariant::GetNumbers( CGodotVariant::FindProperty( m, "vectors" ), numbers ) && 3 == numbers.size() && 1 == CGodotVariant::FindProperty( m, "vectors" )->GetAttributeValue( "count" ).AsUInt32() );
        ASSERT_TRUE( 2 == CGodotVariant::FindProperty( m, "colours" )->GetAttributeValue( "count" ).AsUInt32() && 4 == CGodotVariant::FindProperty( m, "colours" )->GetAttributeValue( "components" ).AsUInt32() );
        CORE::CString::StringVector texts;
        ASSERT_TRUE( CGodotVariant::GetStrings( CGodotVariant::FindProperty( m, "strings" ), texts ) && 2 == texts.size() && "beta" == texts[ 1 ] );

        // the header alone gives the type and the uid
        CORE::CString type;
        CORE::CString uid;
        ASSERT_TRUE( CGodotBinaryReader::ReadHeader( &file[ 0 ], 120, type, uid ) && "ResourceType" == type && Uid( 1000 ) == uid );
        ASSERT_TRUE( "uid://ba" == CGodotBinaryReader::UidToText( 34 ) && "uid://b" == CGodotBinaryReader::UidToText( 1 ) && "uid://i" == CGodotBinaryReader::UidToText( 8 ) );
        ASSERT_TRUE( "uid://0" == CGodotBinaryReader::UidToText( 25 ) && "uid://8" == CGodotBinaryReader::UidToText( 33 ) && "uid://y" == CGodotBinaryReader::UidToText( 24 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Binary resources: files that are cut off, of Godot 3, in big endian, compressed, with a value of an unknown type" )
    try
    {
        CRsrcBuilder builder;
        TProperties properties;
        properties.push_back( SProperty( "size", VVector3( 1, 2, 3 ) ) );
        builder.AddInternal( "Main", "Resource", properties );
        const TBytes good = builder.Build( "Resource", 1 );
        CORE::CDataNode root;
        CGodotBinaryReader reader;
        ASSERT_TRUE( reader.Parse( &good[ 0 ], (UInt32) good.size(), root ) );
        for ( UInt32 cut=1; cut<(UInt32) good.size() - 4; cut += 7 )
        {
            CGodotBinaryReader cutReader;
            CORE::CDataNode cutRoot;
            ASSERT_FALSE( cutReader.Parse( &good[ 0 ], cut, cutRoot ) );
        }
        TBytes bigEndian = good;
        bigEndian[ 4 ] = 1;
        ASSERT_FALSE( reader.Parse( &bigEndian[ 0 ], (UInt32) bigEndian.size(), root ) && true );
        ASSERT_TRUE( reader.GetProblem().HasSubstr( "big endian" ) >= 0 );
        TBytes old = builder.Build( "Resource", 1, false, 2 );
        old[ 12 ] = 3;
        ASSERT_FALSE( reader.Parse( &old[ 0 ], (UInt32) old.size(), root ) );
        ASSERT_TRUE( reader.GetProblem().HasSubstr( "Godot 3" ) >= 0 );
        TBytes compressed = good;
        compressed[ 2 ] = 'C';
        ASSERT_FALSE( reader.Parse( &compressed[ 0 ], (UInt32) compressed.size(), root ) );
        ASSERT_TRUE( reader.GetProblem().HasSubstr( "compressed" ) >= 0 && CGodotBinaryReader::IsBinaryResource( &compressed[ 0 ], (UInt32) compressed.size() ) );
        ASSERT_FALSE( CGodotBinaryReader::IsBinaryResource( "RSXX", 4 ) );

        CRsrcBuilder unknown;
        TProperties unknownProperties;
        TBytes strange;
        Put32( strange, 99 );
        unknownProperties.push_back( SProperty( "x", strange ) );
        unknown.AddInternal( "Main", "Resource", unknownProperties );
        const TBytes unknownFile = unknown.Build( "Resource", 1 );
        ASSERT_FALSE( reader.Parse( &unknownFile[ 0 ], (UInt32) unknownFile.size(), root ) );
        ASSERT_TRUE( reader.GetProblem().HasSubstr( "99" ) >= 0 );
        ASSERT_FALSE( reader.Parse( "not a resource", 14, root ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Real project shapes: a prefab with an external binary mesh at a path of another letter case, a material file, a uid that leads to a moved file, a shape file and a skin" )
    try
    {
        REQUIRE( SetUpRealPack( "real1" ) );
        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportGodot( "work/real1", "Assets/Pack/Prefabs/Thing.tscn", scene, report ) );
        ASSERT_TRUE( 0 == report.GetItemCount( REPORTITEM_ERROR ) && 0 == report.GetItemCount( REPORTITEM_WARNING ) );
        // the node of the shape is not kept: its collider is on the body
        REQUIRE( 3 == scene.nodes.size() && FindNodeByName( scene, "CollisionShape3D" ) < 0 );
        const Int32 thing = FindNodeByName( scene, "Thing" );
        const Int32 other = FindNodeByName( scene, "Other" );
        const Int32 collider = FindNodeByName( scene, "MeshCollider" );
        REQUIRE( thing >= 0 && other >= 0 && collider >= 0 );

        // the mesh of the thing: the surfaces of the file, the material of the prefab instead of the one of the file
        REQUIRE( scene.nodes[ thing ].mesh >= 0 );
        const CAssetSubMesh& subMesh = scene.meshes[ scene.nodes[ thing ].mesh ].subMeshes[ 0 ];
        ASSERT_TRUE( 3 == subMesh.GetVertexCount() && 3 == subMesh.indices.size() && 0 == subMesh.indices[ 0 ] && 1 == subMesh.indices[ 1 ] && 2 == subMesh.indices[ 2 ] );
        ASSERT_TRUE( Near( subMesh.normals[ 2 ], 1.0f, 2e-3f ) && Near( subMesh.tangents[ 3 ], -1.0f ) && 6 == subMesh.uvSets[ 1 ].size() );
        REQUIRE( subMesh.materialIndex >= 0 );
        ASSERT_TRUE( "Mat_01" == scene.materials[ subMesh.materialIndex ].name );
        // the multiply operator of the emission is the one of glTF
        ASSERT_TRUE( scene.materials[ subMesh.materialIndex ].emissiveTexture.IsUsed() && Near( scene.materials[ subMesh.materialIndex ].emissiveFactor[ 2 ], 1.0f ) );
        ASSERT_FALSE( ReportHas( report, REPORTITEM_APPROXIMATION, "emission colour" ) );
        // the uid of the texture leads to it by its path in another letter case, and its own file is read once
        ASSERT_TRUE( scene.materials[ subMesh.materialIndex ].baseColorTexture.IsUsed() && 2 == scene.textures.size() );

        // the mesh found by its uid at a path that is not there keeps the material of its file
        REQUIRE( scene.nodes[ other ].mesh >= 0 );
        const CAssetSubMesh& otherMesh = scene.meshes[ scene.nodes[ other ].mesh ].subMeshes[ 0 ];
        REQUIRE( otherMesh.materialIndex >= 0 );
        ASSERT_TRUE( "OtherMaterial" == scene.materials[ otherMesh.materialIndex ].name && Near( scene.materials[ otherMesh.materialIndex ].baseColorFactor[ 0 ], CGodotVariant::SrgbToLinear( 0.5f ), 1e-4f ) );
        // the material of the first file is not in the scene: the prefab replaced it
        for ( size_t m=0; m<scene.materials.size(); ++m )
            ASSERT_TRUE( "ThingMaterial" != scene.materials[ m ].name );
        ASSERT_TRUE( 2 == scene.materials.size() && 3 == scene.meshes.size() );

        // the shape file
        REQUIRE( 1 == scene.nodes[ collider ].colliders.size() );
        ASSERT_TRUE( COLLIDERSHAPE_CONVEX == scene.nodes[ collider ].colliders[ 0 ].shape && scene.nodes[ collider ].colliders[ 0 ].meshIndex >= 0 );
        const CAssetSubMesh& hull = scene.meshes[ scene.nodes[ collider ].colliders[ 0 ].meshIndex ].subMeshes[ 0 ];
        ASSERT_TRUE( 4 == hull.GetVertexCount() && 4 == hull.GetTriangleCount() );
        // the dependencies hold the files of the prefab
        bool meshDependency = false;
        for ( size_t d=0; d<scene.dependencies.size(); ++d )
            if ( scene.dependencies[ d ].HasSubstr( "extracted/Thing.res" ) >= 0 )
                meshDependency = true;
        ASSERT_TRUE( meshDependency );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Real project shapes: a skinned mesh file with blend shape names, a skeleton, an animation player and a skin: the geometry is read, the rest is reported" )
    try
    {
        MakeWorkFolder( "real2" );
        ASSERT_TRUE( WriteGodotProject( "real2" ) );
        SGodotSurface surface = GodotTriangleSurface( "" );
        surface.extraFormat = 1024 | 2048;
        vector< SGodotSurface > surfaces;
        surfaces.push_back( surface );
        ASSERT_TRUE( WriteBinaryMesh( "real2/Models/extracted/Person.mesh", 77, surfaces, "PersonMaterial", true ) );
        ASSERT_TRUE( WriteWorkText( "real2/Models/extracted/Person_skin.tres", "[gd_resource type=\"Skin\" format=3 uid=\"uid://skin_p\"]\n\n[resource]\nbind_count = 2\nbind/0/name = &\"Hips\"\nbind/0/bone = -1\nbind/0/pose = Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0)\nbind/1/name = &\"Spine\"\nbind/1/bone = -1\nbind/1/pose = Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0)\n" ) );
        ASSERT_TRUE( WriteWorkText( "real2/Prefabs/Person.tscn",
            "[gd_scene format=3 uid=\"uid://person\"]\n\n"
            "[ext_resource type=\"ArrayMesh\" path=\"res://Models/extracted/Person.mesh\" id=\"1_m\"]\n"
            "[ext_resource type=\"Skin\" path=\"res://Models/extracted/Person_skin.tres\" id=\"2_s\"]\n\n"
            "[node name=\"Person\" type=\"CharacterBody3D\"]\n\n"
            "[node name=\"Skeleton3D\" type=\"Skeleton3D\" parent=\".\"]\nbones/0/name = \"Hips\"\nbones/0/parent = -1\nbones/0/rest = Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0)\nbones/0/enabled = true\nbones/0/position = Vector3(0, 0, 0)\nbones/0/rotation = Quaternion(0, 0, 0, 1)\nbones/0/scale = Vector3(1, 1, 1)\n\n"
            "[node name=\"Body\" type=\"MeshInstance3D\" parent=\"Skeleton3D\"]\nmesh = ExtResource(\"1_m\")\nskin = ExtResource(\"2_s\")\nskeleton = NodePath(\"..\")\n\n"
            "[node name=\"AnimationPlayer\" type=\"AnimationPlayer\" parent=\".\"]\n" ) );
        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportGodot( "work/real2", "Prefabs/Person.tscn", scene, report ) );
        ASSERT_TRUE( 0 == report.GetItemCount( REPORTITEM_ERROR ) );
        const Int32 body = FindNodeByName( scene, "Body" );
        REQUIRE( body >= 0 && scene.nodes[ body ].mesh >= 0 );
        ASSERT_TRUE( 3 == scene.meshes[ scene.nodes[ body ].mesh ].subMeshes[ 0 ].GetVertexCount() );
        ASSERT_TRUE( ReportHas( report, REPORTITEM_SKIPPED, "Skeleton3D" ) && ReportHas( report, REPORTITEM_SKIPPED, "AnimationPlayer" ) && ReportHas( report, REPORTITEM_INFO, "bone indices" ) );
        ASSERT_TRUE( "CharacterBody3D" == scene.nodes[ 0 ].extras.GetAttributeValue( "godotType" ).AsString() && "kinematic" == scene.nodes[ 0 ].extras.FindChild( "godotBody" )->GetAttributeValue( "motion" ).AsString() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Real project shapes: the settings of a model that is not in the pack, the project file, and what the import settings say about extracted meshes" )
    try
    {
        MakeWorkFolder( "real3" );
        ASSERT_TRUE( SetUpRealPack( "real3" ) );
        ASSERT_TRUE( WriteWorkText( "real3/project.godot",
            "; Engine configuration file.\n; It's best edited using the editor UI and not directly,\n\nconfig_version=5\n\n[animation]\n\ncompatibility/default_parent_skeleton_in_mesh_instance_3d=true\n\n"
            "[application]\n\nconfig/name=\"Test_Project\"\nrun/main_scene=\"uid://mainscene1\"\nconfig/features=PackedStringArray(\"4.6\", \"Forward Plus\")\nboot_splash/bg_color=Color(0, 0, 0, 1)\nconfig/icon=\"uid://iconfile1\"\n" ) );
        CAssetPackContext context;
        REQUIRE( OpenPackContext( context, "work/real3", "work/real3_inter" ) );
        CGodotImportSettings settings;
        REQUIRE( settings.Load( context, "Assets/Pack/Models/Thing.fbx" ) );
        ASSERT_TRUE( "scene" == settings.GetImporter() && "uid://model_thing" == settings.GetUid() && Near( (Float32) settings.GetRootScale(), 1.0f ) );
        ASSERT_FALSE( settings.GetImportAnimations() );
        ASSERT_FALSE( settings.IsNodeSkipped( "AnimationPlayer" ) );
        CORE::CDataNode plain( "settings", GUCEF_DATATYPE_OBJECT );
        settings.AddPlain( plain );
        ASSERT_TRUE( 1 == plain.GetAttributeValue( "meshes/ensure_tangents" ).AsInt32() && GUCEF_NULL != plain.FindChild( "_subresources" ) );

        // the uid of a model that is not in the pack leads nowhere; the uid of the files that are there does
        CGodotPaths::IndexUids( context );
        CORE::CString found;
        ASSERT_FALSE( CGodotPaths::Resolve( context, "Assets/Pack/Prefabs/Thing.tscn", "res://Assets/Pack/Models/Thing.fbx", "uid://model_thing", found ) );
        ASSERT_TRUE( CGodotPaths::Resolve( context, "Assets/Pack/Prefabs/Thing.tscn", "res://wrong.res", Uid( MeshUid ), found ) && "Assets/Pack/Models/extracted/Thing.res" == found );
        ASSERT_TRUE( CGodotPaths::Resolve( context, "Assets/Pack/Prefabs/Thing.tscn", "res://ASSETS/pack/textures/t.PNG", "", found ) && "Assets/Pack/Textures/T.png" == found );
        ASSERT_TRUE( CGodotPaths::Resolve( context, "Assets/Pack/Prefabs/Thing.tscn", "res://x.png", "uid://tex_t", found ) && "Assets/Pack/Textures/T.png" == found );

        // the project file has sections and values like every other file
        CGodotDocument project;
        REQUIRE( project.Load( context, "project.godot" ) );
        const CORE::CDataNode* application = project.FindSection( "application" );
        REQUIRE( GUCEF_NULL != application );
        CORE::CString::StringVector features;
        ASSERT_TRUE( "Test_Project" == CGodotVariant::GetString( CGodotVariant::FindProperty( *application, "config/name" ) ) && CGodotVariant::GetStrings( CGodotVariant::FindProperty( *application, "config/features" ), features ) && 2 == features.size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Real project shapes: particle scenes (a particle node, curves, gradients, a quad mesh with a billboard material) are read into a particle system, without errors" )
    try
    {
        MakeWorkFolder( "real4" );
        ASSERT_TRUE( WriteGodotProject( "real4" ) );
        ASSERT_TRUE( WriteTestPng( "work/real4/Textures/Flame.png", 4, 4, 255, 200, 0, 255 ) );
        ASSERT_TRUE( WriteWorkText( "real4/Materials/Flame.tres",
            "[gd_resource type=\"StandardMaterial3D\" load_steps=2 format=3 uid=\"uid://flame\"]\n\n[ext_resource type=\"Texture2D\" path=\"res://Textures/Flame.png\" id=\"1_t\"]\n\n[resource]\nresource_local_to_scene = false\ntransparency = 1\nblend_mode = 1\ncull_mode = 2\nshading_mode = 0\nvertex_color_use_as_albedo = true\nalbedo_texture = ExtResource(\"1_t\")\nbillboard_mode = 3\nbillboard_keep_scale = true\nparticles_anim_h_frames = 8\nparticles_anim_v_frames = 8\nparticles_anim_loop = true\n" ) );
        ASSERT_TRUE( WriteWorkText( "real4/Prefabs/Flame.tscn",
            "[gd_scene load_steps=8 format=3 uid=\"uid://fx_flame\"]\n\n[ext_resource type=\"Material\" uid=\"uid://flame\" path=\"res://Materials/Flame.tres\" id=\"1_m\"]\n\n"
            "[sub_resource type=\"Gradient\" id=\"Gradient_a\"]\noffsets = PackedFloat32Array(0, 0.32867134, 0.986014)\ncolors = PackedColorArray(1, 1, 1, 0.36078432, 0.9, 0.65, 0.01, 0.71, 0.9, 0.65, 0.01, 0.71)\n\n"
            "[sub_resource type=\"GradientTexture1D\" id=\"GradientTexture1D_a\"]\ngradient = SubResource(\"Gradient_a\")\n\n"
            "[sub_resource type=\"Curve\" id=\"Curve_a\"]\n_data = [Vector2(0, 1), 0.0, 0.0, 0, 0, Vector2(0.5, 0.93), 0.0, 0.0, 0, 0]\npoint_count = 2\n\n"
            "[sub_resource type=\"CurveTexture\" id=\"CurveTexture_a\"]\ncurve = SubResource(\"Curve_a\")\n\n"
            "[sub_resource type=\"ParticleProcessMaterial\" id=\"ParticleProcessMaterial_a\"]\nparticle_flag_rotate_y = true\nangle_min = -179.99998\ngravity = Vector3(0, 0, 0)\nscale_curve = SubResource(\"CurveTexture_a\")\ncolor_ramp = SubResource(\"GradientTexture1D_a\")\n\n"
            "[sub_resource type=\"QuadMesh\" id=\"QuadMesh_a\"]\nmaterial = ExtResource(\"1_m\")\nsize = Vector2(0.5, 1)\ncenter_offset = Vector3(0, 0.455, 0)\n\n"
            "[sub_resource type=\"RibbonTrailMesh\" id=\"RibbonTrailMesh_a\"]\nsize = 0.1\n\n"
            "[node name=\"FX_Flame\" type=\"GPUParticles3D\"]\namount = 6\nspeed_scale = 0.5\nfixed_fps = 0\nvisibility_aabb = AABB(-0.1, 0, -0.1, 0.2, 0.3, 0.2)\nprocess_material = SubResource(\"ParticleProcessMaterial_a\")\ndraw_pass_1 = SubResource(\"QuadMesh_a\")\n\n"
            "[node name=\"Trail\" type=\"MeshInstance3D\" parent=\".\"]\nmesh = SubResource(\"RibbonTrailMesh_a\")\n" ) );
        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportGodot( "work/real4", "Prefabs/Flame.tscn", scene, report ) );
        ASSERT_TRUE( 0 == report.GetItemCount( REPORTITEM_ERROR ) && 2 == scene.nodes.size() );
        ASSERT_TRUE( 1 == scene.particleSystems.size() && 0 == scene.nodes[ 0 ].particleSystem && ReportHas( report, REPORTITEM_SKIPPED, "RibbonTrailMesh" ) );
        ASSERT_TRUE( 1 == scene.particleSystems.size() && scene.particleSystems[ 0 ].textureSheet.enabled && 8 == scene.particleSystems[ 0 ].textureSheet.tilesX && scene.particleSystems[ 0 ].colourOverLifetimeEnabled );
        ASSERT_TRUE( "GPUParticles3D" == scene.nodes[ 0 ].extras.GetAttributeValue( "godotType" ).AsString() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Real project shapes: a mesh file that is compressed or broken, and a nested scene that is broken, leave out what they hold with a warning; the scene stays an asset" )
    try
    {
        MakeWorkFolder( "real5" );
        ASSERT_TRUE( WriteGodotProject( "real5" ) );
        const UInt8 compressed[ 20 ] = { 'R', 'S', 'C', 'C', 2, 0, 0, 0, 0, 16, 0, 0, 165, 36, 0, 0, 85, 6, 0, 0 };
        ASSERT_TRUE( WriteWorkFile( "real5/Models/extracted/Packed.res", compressed, 20 ) );
        ASSERT_TRUE( WriteWorkText( "real5/Models/extracted/Broken.res", "RSRC but not a resource" ) );
        vector< SGodotSurface > surfaces;
        surfaces.push_back( GodotTriangleSurface( "" ) );
        ASSERT_TRUE( WriteBinaryMesh( "real5/Models/extracted/Good.res", 5, surfaces, "GoodMaterial", false ) );
        ASSERT_TRUE( WriteWorkText( "real5/Prefabs/Nested.tscn", "[gd_scene format=3]\n\n[node name=\"Nested\" type=\"Node3D\"\nbroken" ) );
        ASSERT_TRUE( WriteWorkText( "real5/Prefabs/Scene.tscn",
            "[gd_scene format=3]\n\n"
            "[ext_resource type=\"ArrayMesh\" path=\"res://Models/extracted/Packed.res\" id=\"1_p\"]\n"
            "[ext_resource type=\"ArrayMesh\" path=\"res://Models/extracted/Broken.res\" id=\"2_b\"]\n"
            "[ext_resource type=\"ArrayMesh\" path=\"res://Models/extracted/Good.res\" id=\"3_g\"]\n"
            "[ext_resource type=\"PackedScene\" path=\"res://Prefabs/Nested.tscn\" id=\"4_n\"]\n\n"
            "[node name=\"Root\" type=\"Node3D\"]\n\n"
            "[node name=\"Packed\" type=\"MeshInstance3D\" parent=\".\"]\nmesh = ExtResource(\"1_p\")\n\n"
            "[node name=\"Broken\" type=\"MeshInstance3D\" parent=\".\"]\nmesh = ExtResource(\"2_b\")\n\n"
            "[node name=\"Good\" type=\"MeshInstance3D\" parent=\".\"]\nmesh = ExtResource(\"3_g\")\n\n"
            "[node name=\"Inner\" parent=\".\" instance=ExtResource(\"4_n\")]\n" ) );
        CAssetScene scene;
        CConversionReport report;
        REQUIRE( ImportGodot( "work/real5", "Prefabs/Scene.tscn", scene, report ) );
        ASSERT_TRUE( 0 == report.GetItemCount( REPORTITEM_ERROR ) && report.GetItemCount( REPORTITEM_WARNING ) >= 3 );
        ASSERT_TRUE( ReportHas( report, REPORTITEM_WARNING, "compressed" ) && ReportHas( report, REPORTITEM_WARNING, "Broken.res" ) && ReportHas( report, REPORTITEM_WARNING, "Nested.tscn" ) );
        REQUIRE( 5 == scene.nodes.size() );
        ASSERT_TRUE( scene.nodes[ FindNodeByName( scene, "Good" ) ].mesh >= 0 && scene.nodes[ FindNodeByName( scene, "Packed" ) ].mesh < 0 && scene.nodes[ FindNodeByName( scene, "Broken" ) ].mesh < 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Archives: a project in a zip file read through the VFS (paths of another letter case, uids found when a path leads nowhere, a binary mesh), as a pack source and with --mount" )
    try
    {
        MakeWorkFolder( "zip1" );
        // the pack as a zip: a folder of the project in it, as the downloads of packs have it
        MakeWorkFolder( "zipsrc" );
        REQUIRE( SetUpRealPack( "zipsrc" ) );
        CZipBuilder zip;
        zip.AddText( "proj-01/project.godot", "config_version=5\n" );
        zip.AddText( "proj-01/Assets/Pack/Prefabs/Thing.tscn", ThingScene() );
        zip.AddText( "proj-01/Assets/Pack/Prefabs/Other.tscn", "[gd_scene format=3]\n\n[node name=\"Other\" type=\"Node3D\"]\n" );
        zip.AddText( "proj-01/Assets/Pack/Materials/Mat_01.tres", ReadVfsText( "work/zipsrc/Assets/Pack/Materials/Mat_01.tres" ) );
        zip.AddText( "proj-01/Assets/Pack/Textures/T.png.import", ReadVfsText( "work/zipsrc/Assets/Pack/Textures/T.png.import" ) );
        const char* const binaries[] = { "Assets/Pack/Textures/T.png", "Assets/Pack/Textures/E.png", "Assets/Pack/Models/extracted/Thing.res", "Assets/Pack/Models/extracted/Other.res", "Assets/Pack/Models/Collision/Shape.res" };
        for ( UInt32 b=0; b<sizeof( binaries ) / sizeof( binaries[ 0 ] ); ++b )
        {
            CORE::CDynamicBuffer content;
            REQUIRE( ReadVfsBytes( CORE::CString( "work/zipsrc/" ) + binaries[ b ], content ) );
            TBytes bytes( static_cast< const UInt8* >( content.GetConstBufferPtr() ), static_cast< const UInt8* >( content.GetConstBufferPtr() ) + content.GetDataSize() );
            zip.Add( CORE::CString( "proj-01/" ) + binaries[ b ], bytes );
        }
        const TBytes zipFile = zip.Finish();
        ASSERT_TRUE( WriteWorkFile( "zip1/pack.zip", &zipFile[ 0 ], (UInt32) zipFile.size() ) );

        // the zip as the source of a pack
        TAssetImporterPtr importer;
        REQUIRE( CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetImporter( "godot", importer ) );
        CAssetPackManifest manifest;
        manifest.id = "zipped";
        manifest.source = "/work/zip1/pack.zip";
        manifest.intermediate = "/work/zip1_inter";
        CAssetPackContext context;
        REQUIRE( context.Open( manifest ) );
        ASSERT_TRUE( context.IsArchiveMounted() && context.HasFile( "proj-01/project.godot" ) );
        ASSERT_TRUE( importer->IndexPack( context ) );
        // the uids of an archive are not read before they are needed
        CORE::CString indexed;
        ASSERT_FALSE( context.TryResolve( "uid", Uid( MeshUid ), indexed ) );
        CAssetScene scene;
        CConversionReport report;
        scene.name = "Thing";
        REQUIRE( importer->Import( context.ToVfsPath( "proj-01/Assets/Pack/Prefabs/Thing.tscn" ), context, scene, report ) );
        ASSERT_TRUE( 0 == report.GetItemCount( REPORTITEM_ERROR ) && 0 == report.GetItemCount( REPORTITEM_WARNING ) );
        REQUIRE( 3 == scene.nodes.size() );
        const Int32 other = FindNodeByName( scene, "Other" );
        REQUIRE( other >= 0 && scene.nodes[ other ].mesh >= 0 );
        ASSERT_TRUE( "OtherMaterial" == scene.materials[ scene.meshes[ scene.nodes[ other ].mesh ].subMeshes[ 0 ].materialIndex ].name );
        ASSERT_TRUE( context.TryResolve( "uid", Uid( OtherUid ), indexed ) && "proj-01/Assets/Pack/Models/extracted/Other.res" == indexed );
        ASSERT_TRUE( importer->CanImport( context.ToVfsPath( "proj-01/Assets/Pack/Prefabs/Other.tscn" ), context ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CLI: --mount takes a zip file, the pack source can be a zip file too, and \"assets\" selects what is converted while the rest of the pack stays visible" )
    try
    {
        MakeWorkFolder( "zip2" );
        MakeWorkFolder( "zipsrc2" );
        REQUIRE( SetUpRealPack( "zipsrc2" ) );
        CZipBuilder zip;
        zip.AddText( "proj-01/project.godot", "config_version=5\n" );
        zip.AddText( "proj-01/Assets/Pack/Prefabs/Thing.tscn", ThingScene() );
        zip.AddText( "proj-01/Assets/Pack/Prefabs/Second.tscn", "[gd_scene format=3]\n\n[node name=\"Second\" type=\"Node3D\"]\n" );
        zip.AddText( "proj-01/Assets/Pack/Materials/Mat_01.tres", ReadVfsText( "work/zipsrc2/Assets/Pack/Materials/Mat_01.tres" ) );
        zip.AddText( "proj-01/Assets/Pack/Textures/T.png.import", ReadVfsText( "work/zipsrc2/Assets/Pack/Textures/T.png.import" ) );
        const char* const binaries[] = { "Assets/Pack/Textures/T.png", "Assets/Pack/Textures/E.png", "Assets/Pack/Models/extracted/Thing.res", "Assets/Pack/Models/extracted/Other.res", "Assets/Pack/Models/Collision/Shape.res" };
        for ( UInt32 b=0; b<sizeof( binaries ) / sizeof( binaries[ 0 ] ); ++b )
        {
            CORE::CDynamicBuffer content;
            REQUIRE( ReadVfsBytes( CORE::CString( "work/zipsrc2/" ) + binaries[ b ], content ) );
            zip.Add( CORE::CString( "proj-01/" ) + binaries[ b ], TBytes( static_cast< const UInt8* >( content.GetConstBufferPtr() ), static_cast< const UInt8* >( content.GetConstBufferPtr() ) + content.GetDataSize() ) );
        }
        const TBytes zipFile = zip.Finish();
        ASSERT_TRUE( WriteWorkFile( "zip2/folder/pack.zip", &zipFile[ 0 ], (UInt32) zipFile.size() ) );
        ASSERT_TRUE( WriteLicence( "zip2/manifests", "rec" ) );

        // the zip mounted as a folder, one asset selected
        ASSERT_TRUE( WriteWorkText( "zip2/manifests/mounted.assetpack.json",
            "{\n  \"id\": \"mounted\",\n  \"licenceRecord\": \"rec\",\n  \"licenceFolder\": \"/pack\",\n  \"source\": \"/zipped\",\n  \"importer\": \"godot\",\n"
            "  \"assets\": [ \"**/Thing.tscn\" ],\n  \"intermediate\": \"/out/intermediate\",\n  \"output\": \"/out\"\n}\n" ) );
        CORE::CString::StringVector arguments;
        arguments.push_back( "--mount" ); arguments.push_back( "zipped=" + WorkDiskPath( "zip2/folder/pack.zip" ) );
        arguments.push_back( "--manifest" ); arguments.push_back( WorkDiskPath( "zip2/manifests/mounted.assetpack.json" ) );
        arguments.push_back( "--out" ); arguments.push_back( WorkDiskPath( "zip2/out1" ) );
        CConversionReport report;
        ASSERT_TRUE( CAssetConverterApp::ExitSuccess == RunConverter( arguments, &report ) );
        ASSERT_TRUE( 0 == report.GetAssetCount( ASSETSTATUS_FAILED ) && 1 == report.GetAssetCount( ASSETSTATUS_CONVERTED ) + report.GetAssetCount( ASSETSTATUS_APPROXIMATED ) );
        const CORE::CString intermediate = WorkDiskPath( "zip2/out1/intermediate/proj-01/Assets/Pack/Prefabs" );
        ASSERT_TRUE( CORE::FileExists( CORE::CombinePath( intermediate, "Thing.gltf" ) ) && !CORE::FileExists( CORE::CombinePath( intermediate, "Second.gltf" ) ) );
        CORE::CString problem;
        ASSERT_TRUE( ValidateWithCgltf( CORE::CombinePath( intermediate, "Thing.gltf" ), problem ) );

        // the zip as the source of the pack, both scenes selected by another glob
        ASSERT_TRUE( WriteWorkText( "zip2/manifests/source.assetpack.json",
            "{\n  \"id\": \"source\",\n  \"licenceRecord\": \"rec\",\n  \"licenceFolder\": \"/pack\",\n  \"source\": \"/folder/pack.zip\",\n  \"importer\": \"godot\",\n"
            "  \"assets\": [ \"**/Prefabs/*.tscn\" ],\n  \"intermediate\": \"/out/intermediate\",\n  \"output\": \"/out\"\n}\n" ) );
        CORE::CString::StringVector second;
        second.push_back( "--mount" ); second.push_back( "folder=" + WorkDiskPath( "zip2/folder" ) );
        second.push_back( "--manifest" ); second.push_back( WorkDiskPath( "zip2/manifests/source.assetpack.json" ) );
        second.push_back( "--out" ); second.push_back( WorkDiskPath( "zip2/out2" ) );
        ASSERT_TRUE( CAssetConverterApp::ExitSuccess == RunConverter( second, &report ) );
        ASSERT_TRUE( 0 == report.GetAssetCount( ASSETSTATUS_FAILED ) && 2 == report.GetAssetCount( ASSETSTATUS_CONVERTED ) + report.GetAssetCount( ASSETSTATUS_APPROXIMATED ) );

        // a file that is no archive
        CORE::CString::StringVector bad;
        bad.push_back( "--mount" ); bad.push_back( "x=" + WorkDiskPath( "zip2/manifests/rec.licence.json" ) );
        bad.push_back( "--manifest" ); bad.push_back( WorkDiskPath( "zip2/manifests/source.assetpack.json" ) );
        bad.push_back( "--out" ); bad.push_back( WorkDiskPath( "zip2/out3" ) );
        ASSERT_TRUE( CAssetConverterApp::ExitConfig == RunConverter( bad, &report ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
