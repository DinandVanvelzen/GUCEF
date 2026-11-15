/*
 *  AssetViewerLib_TestApp: Test application for the asset viewer library (AssetViewerLib)
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_IMAGE_H
#include "gucefIMAGE.h"
#define GUCEF_IMAGE_H
#endif /* GUCEF_IMAGE_H ? */

#ifndef GUCEF_IMAGE_CPNGIMAGECODEC_H
#include "gucefIMAGE_CPngImageCodec.h"
#define GUCEF_IMAGE_CPNGIMAGECODEC_H
#endif /* GUCEF_IMAGE_CPNGIMAGECODEC_H ? */

#include "TestAssetViewerAssets.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The binary buffer of the glTF: little endian floats, 16 bit indices and bytes, with the views on it */
class CBufferBuilder
{
    public:

    CBufferBuilder( void ) : m_size( 0 ), m_viewStart( 0 ) { memset( m_data, 0, sizeof( m_data ) ); }

    void Float( float value )  { Append( &value, sizeof( value ) ); }
    void U16( UInt16 value )   { Append( &value, sizeof( value ) ); }
    void U8( UInt8 value )     { Append( &value, sizeof( value ) ); }
    void Vec3( float x, float y, float z ) { Float( x ); Float( y ); Float( z ); }

    /** Ends a view that started where the previous one ended: returns the JSON of the buffer view */
    CORE::CString EndView( bool isVertexData )
    {
        // views are aligned to 4 bytes
        while ( 0 != ( m_size % 4 ) )
            U8( 0 );
        CORE::CString json = "{\"buffer\":0,\"byteOffset\":" + CORE::ToString( m_viewStart ) +
                             ",\"byteLength\":" + CORE::ToString( m_size - m_viewStart ) +
                             ",\"target\":" + ( isVertexData ? "34962" : "34963" ) + "}";
        m_viewStart = m_size;
        return json;
    }

    /** A view without a target: animation data and inverse bind matrices */
    CORE::CString EndPlainView( void )
    {
        CORE::CString json = EndView( true );
        json = json.ReplaceSubstr( ",\"target\":34962", "" );
        return json;
    }

    UInt32 Size( void ) const { return m_size; }
    CORE::CString AsBase64( void ) const { return CORE::Base64Encode( m_data, m_size ); }

    private:

    void Append( const void* data, UInt32 size )
    {
        memcpy( m_data + m_size, data, size );
        m_size += size;
    }

    UInt8 m_data[ 2048 ];
    UInt32 m_size;
    UInt32 m_viewStart;
};

CORE::CString
Num( UInt32 value )
{
    return CORE::ToString( value );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString
BuildTestGltf( void )
{GUCEF_TRACE;

    CBufferBuilder buffer;
    CORE::CString views;

    // The cube. Every face has its own 4 vertices; n is the normal of the face, u and v the directions of its
    // texture coordinates, u x v = n so that the triangles wind counter clockwise seen from outside
    static const float faces[ 6 ][ 9 ] =
    {
        {  1, 0, 0,   0, 1, 0,   0, 0, 1 },
        { -1, 0, 0,   0, 0, 1,   0, 1, 0 },
        {  0, 1, 0,   0, 0, 1,   1, 0, 0 },
        {  0,-1, 0,   1, 0, 0,   0, 0, 1 },
        {  0, 0, 1,   1, 0, 0,   0, 1, 0 },
        {  0, 0,-1,   0, 1, 0,   1, 0, 0 }
    };
    static const float corners[ 4 ][ 2 ] = { { -1, -1 }, { 1, -1 }, { 1, 1 }, { -1, 1 } };

    for ( int f=0; f<6; ++f )
        for ( int c=0; c<4; ++c )
            buffer.Vec3( 0.5f * ( faces[ f ][ 0 ] + corners[ c ][ 0 ] * faces[ f ][ 3 ] + corners[ c ][ 1 ] * faces[ f ][ 6 ] ) ,
                         0.5f * ( faces[ f ][ 1 ] + corners[ c ][ 0 ] * faces[ f ][ 4 ] + corners[ c ][ 1 ] * faces[ f ][ 7 ] ) ,
                         0.5f * ( faces[ f ][ 2 ] + corners[ c ][ 0 ] * faces[ f ][ 5 ] + corners[ c ][ 1 ] * faces[ f ][ 8 ] ) );
    views += buffer.EndView( true );                                    // view 0: cube positions

    for ( int f=0; f<6; ++f )
        for ( int c=0; c<4; ++c )
            buffer.Vec3( faces[ f ][ 0 ], faces[ f ][ 1 ], faces[ f ][ 2 ] );
    views += "," + buffer.EndView( true );                              // view 1: cube normals

    for ( int f=0; f<6; ++f )
        for ( int c=0; c<4; ++c )
        {
            buffer.Float( 0.5f + 0.5f * corners[ c ][ 0 ] );
            buffer.Float( 0.5f + 0.5f * corners[ c ][ 1 ] );
        }
    views += "," + buffer.EndView( true );                              // view 2: cube texture coordinates

    for ( int f=0; f<6; ++f )
    {
        static const int order[ 6 ] = { 0, 1, 2, 0, 2, 3 };
        for ( int i=0; i<6; ++i )
            buffer.U16( (UInt16) ( f * 4 + order[ i ] ) );
    }
    views += "," + buffer.EndView( false );                             // view 3: cube indices

    // The accessors and nodes of the cube
    CORE::CString accessors =
        "{\"bufferView\":0,\"componentType\":5126,\"count\":24,\"type\":\"VEC3\",\"min\":[-0.5,-0.5,-0.5],\"max\":[0.5,0.5,0.5]},"
        "{\"bufferView\":1,\"componentType\":5126,\"count\":24,\"type\":\"VEC3\"},"
        "{\"bufferView\":2,\"componentType\":5126,\"count\":24,\"type\":\"VEC2\"},"
        "{\"bufferView\":3,\"componentType\":5123,\"count\":36,\"type\":\"SCALAR\"}";
    CORE::CString meshes =
        "{\"name\":\"CubeMesh\",\"primitives\":[{\"attributes\":{\"POSITION\":0,\"NORMAL\":1,\"TEXCOORD_0\":2},\"indices\":3,\"material\":0}]}";
    CORE::CString nodes = "{\"name\":\"CubeNode\",\"mesh\":0}";
    CORE::CString sceneNodes = "0";

    // The strip: 3 rows of 2 vertices at y 0, 1 and 2, in the plane z = 0
    for ( int row=0; row<3; ++row )
    {
        buffer.Vec3( -0.5f, (float) row, 0.0f );
        buffer.Vec3(  0.5f, (float) row, 0.0f );
    }
    views += "," + buffer.EndView( true );                              // view 4: strip positions
    for ( int i=0; i<6; ++i )
        buffer.Vec3( 0.0f, 0.0f, 1.0f );
    views += "," + buffer.EndView( true );                              // view 5: strip normals
    for ( int row=0; row<3; ++row )
    {
        buffer.Float( 0.0f ); buffer.Float( 0.5f * row );
        buffer.Float( 1.0f ); buffer.Float( 0.5f * row );
    }
    views += "," + buffer.EndView( true );                              // view 6: strip texture coordinates

    // Row 0 is bound to Bone0 only, row 1 to both bones in equal parts, row 2 to Bone1 only
    static const UInt8 joints[ 3 ][ 4 ] = { { 0, 0, 0, 0 }, { 0, 1, 0, 0 }, { 1, 0, 0, 0 } };
    static const float weights[ 3 ][ 4 ] = { { 1, 0, 0, 0 }, { 0.5f, 0.5f, 0, 0 }, { 1, 0, 0, 0 } };
    for ( int row=0; row<3; ++row )
        for ( int v=0; v<2; ++v )
            for ( int j=0; j<4; ++j )
                buffer.U8( joints[ row ][ j ] );
    views += "," + buffer.EndView( true );                              // view 7: strip joints
    for ( int row=0; row<3; ++row )
        for ( int v=0; v<2; ++v )
            for ( int j=0; j<4; ++j )
                buffer.Float( weights[ row ][ j ] );
    views += "," + buffer.EndView( true );                              // view 8: strip weights

    static const UInt16 stripIndices[ 12 ] = { 0, 1, 2,  2, 1, 3,  2, 3, 4,  4, 3, 5 };
    for ( int i=0; i<12; ++i )
        buffer.U16( stripIndices[ i ] );
    views += "," + buffer.EndView( false );                             // view 9: strip indices

    // Inverse bind matrices, column major: Bone0 sits at the origin, Bone1 one up
    static const float inverseBind[ 2 ][ 16 ] =
    {
        { 1,0,0,0,  0,1,0,0,  0,0,1,0,  0, 0,0,1 },
        { 1,0,0,0,  0,1,0,0,  0,0,1,0,  0,-1,0,1 }
    };
    for ( int m=0; m<2; ++m )
        for ( int i=0; i<16; ++i )
            buffer.Float( inverseBind[ m ][ i ] );
    views += "," + buffer.EndPlainView();                               // view 10: inverse bind matrices

    buffer.Float( 0.0f ); buffer.Float( 2.0f );
    views += "," + buffer.EndPlainView();                               // view 11: animation times
    // Bone1 turns from rest to 45 degrees around z
    buffer.Float( 0.0f ); buffer.Float( 0.0f ); buffer.Float( 0.0f );        buffer.Float( 1.0f );
    buffer.Float( 0.0f ); buffer.Float( 0.0f ); buffer.Float( 0.3826834f );  buffer.Float( 0.9238795f );
    views += "," + buffer.EndPlainView();                               // view 12: animation rotations

    // The cube is bound to Bone0 only: a loader that makes a skeleton leaves out the meshes without bone weights
    for ( int i=0; i<24; ++i )
    {
        buffer.U8( 0 ); buffer.U8( 0 ); buffer.U8( 0 ); buffer.U8( 0 );
    }
    views += "," + buffer.EndView( true );                              // view 13: cube joints
    for ( int i=0; i<24; ++i )
    {
        buffer.Float( 1.0f ); buffer.Float( 0.0f ); buffer.Float( 0.0f ); buffer.Float( 0.0f );
    }
    views += "," + buffer.EndView( true );                              // view 14: cube weights

    accessors +=
        ",{\"bufferView\":4,\"componentType\":5126,\"count\":6,\"type\":\"VEC3\",\"min\":[-0.5,0,0],\"max\":[0.5,2,0]}"
        ",{\"bufferView\":5,\"componentType\":5126,\"count\":6,\"type\":\"VEC3\"}"
        ",{\"bufferView\":6,\"componentType\":5126,\"count\":6,\"type\":\"VEC2\"}"
        ",{\"bufferView\":7,\"componentType\":5121,\"count\":6,\"type\":\"VEC4\"}"
        ",{\"bufferView\":8,\"componentType\":5126,\"count\":6,\"type\":\"VEC4\"}"
        ",{\"bufferView\":9,\"componentType\":5123,\"count\":12,\"type\":\"SCALAR\"}"
        ",{\"bufferView\":10,\"componentType\":5126,\"count\":2,\"type\":\"MAT4\"}"
        ",{\"bufferView\":11,\"componentType\":5126,\"count\":2,\"type\":\"SCALAR\",\"min\":[0],\"max\":[2]}"
        ",{\"bufferView\":12,\"componentType\":5126,\"count\":2,\"type\":\"VEC4\"}"
        ",{\"bufferView\":13,\"componentType\":5121,\"count\":24,\"type\":\"VEC4\"}"
        ",{\"bufferView\":14,\"componentType\":5126,\"count\":24,\"type\":\"VEC4\"}";
    meshes = meshes.ReplaceSubstr( "\"TEXCOORD_0\":2}", "\"TEXCOORD_0\":2,\"JOINTS_0\":13,\"WEIGHTS_0\":14}" );
    nodes = "{\"name\":\"CubeNode\",\"mesh\":0,\"skin\":0}";
    meshes +=
        ",{\"name\":\"StripMesh\",\"primitives\":[{\"attributes\":{\"POSITION\":4,\"NORMAL\":5,\"TEXCOORD_0\":6,"
        "\"JOINTS_0\":7,\"WEIGHTS_0\":8},\"indices\":9,\"material\":1}]}";
    nodes +=
        ",{\"name\":\"StripNode\",\"mesh\":1,\"skin\":0}"
        ",{\"name\":\"Bone0\",\"children\":[3]}"
        ",{\"name\":\"Bone1\",\"translation\":[0,1,0]}";
    sceneNodes += ",1,2";
    const CORE::CString extra =
        ",\"skins\":[{\"name\":\"StripSkin\",\"inverseBindMatrices\":10,\"skeleton\":2,\"joints\":[2,3]}]"
        ",\"animations\":[{\"name\":\"Wave\",\"samplers\":[{\"input\":11,\"output\":12,\"interpolation\":\"LINEAR\"}],"
        "\"channels\":[{\"sampler\":0,\"target\":{\"node\":3,\"path\":\"rotation\"}}]}]";

    const CORE::CString materials =
        "{\"name\":\"CubeMaterial\",\"pbrMetallicRoughness\":{\"baseColorTexture\":{\"index\":0},\"metallicFactor\":0.0,\"roughnessFactor\":0.8}}"
        ",{\"name\":\"StripMaterial\",\"pbrMetallicRoughness\":{\"baseColorFactor\":[0.2,0.6,0.9,1.0],\"metallicFactor\":0.0}}";

    return "{\"asset\":{\"version\":\"2.0\",\"generator\":\"AssetViewerLib_TestApp\"}"
           ",\"scene\":0,\"scenes\":[{\"name\":\"TestScene\",\"nodes\":[" + sceneNodes + "]}]"
           ",\"nodes\":[" + nodes + "]"
           ",\"meshes\":[" + meshes + "]"
           ",\"materials\":[" + materials + "]"
           ",\"textures\":[{\"source\":0}]"
           ",\"images\":[{\"uri\":\"checker.png\"}]"
           + extra +
           ",\"buffers\":[{\"byteLength\":" + Num( buffer.Size() ) + ",\"uri\":\"data:application/octet-stream;base64," + buffer.AsBase64() + "\"}]"
           ",\"bufferViews\":[" + views + "]"
           ",\"accessors\":[" + accessors + "]}";
}

/*-------------------------------------------------------------------------*/

bool
WriteTestPng( const CORE::CString& path, UInt32 width, UInt32 height )
{GUCEF_TRACE;

    IMAGE::TPixelMapPtr pixels( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, width, height, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
    UInt8* data = static_cast< UInt8* >( pixels->GetDataPtr() );
    for ( UInt32 y=0; y<height; ++y )
    {
        for ( UInt32 x=0; x<width; ++x )
        {
            UInt8* pixel = data + ( (size_t) y * width + x ) * 4;
            const bool left = x < width / 2;
            pixel[ 0 ] = left ? 255 : 0;
            pixel[ 1 ] = 0;
            pixel[ 2 ] = left ? 0 : 255;
            pixel[ 3 ] = 255;
        }
    }
    CORE::CreatePathDirectories( path.SubstrToChar( '/', false ) );
    CORE::CString problem;
    return IMAGE::CPngImageCodec::WritePixelMapToFile( *pixels, path, problem );
}

/*-------------------------------------------------------------------------*/

bool
WriteTestDds( const CORE::CString& path, UInt32 width, UInt32 height, UInt16 colour565 )
{GUCEF_TRACE;

    // the header of a DDS file is 128 bytes: the magic, the 124 byte header with the pixel format at 76 and the four character code
    UInt8 header[ 128 ];
    memset( header, 0, sizeof( header ) );
    memcpy( header, "DDS ", 4 );
    UInt32* fields = reinterpret_cast< UInt32* >( header + 4 );
    const UInt32 blocks = ( ( width + 3 ) / 4 ) * ( ( height + 3 ) / 4 );
    fields[ 0 ] = 124;                              // size of the header
    fields[ 1 ] = 0x1 | 0x2 | 0x4 | 0x1000 | 0x80000;   // caps, height, width, pixel format, linear size
    fields[ 2 ] = height;
    fields[ 3 ] = width;
    fields[ 4 ] = blocks * 8;                       // linear size
    fields[ 18 ] = 32;                              // size of the pixel format
    fields[ 19 ] = 0x4;                             // the format has a four character code
    memcpy( header + 4 + 80, "DXT1", 4 );
    fields[ 26 ] = 0x1000;                          // caps: texture

    CORE::CreatePathDirectories( path.SubstrToChar( '/', false ) );
    CORE::CFileAccess file( path, "wb" );
    if ( !file.IsValid() || 1 != file.Write( header, sizeof( header ), 1 ) )
        return false;
    // the two end points of a block are the same colour: all sixteen pixels are that colour
    for ( UInt32 i=0; i<blocks; ++i )
    {
        UInt8 block[ 8 ] = { (UInt8) ( colour565 & 0xFF ), (UInt8) ( colour565 >> 8 ), (UInt8) ( colour565 & 0xFF ), (UInt8) ( colour565 >> 8 ), 0, 0, 0, 0 };
        if ( 1 != file.Write( block, sizeof( block ), 1 ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
BuildTestScene( const CORE::CString& meshFile )
{GUCEF_TRACE;

    return "<scene formatVersion=\"1.1\">\n"
           "    <nodes>\n"
           "        <node name=\"Parent\">\n"
           "            <position x=\"1\" y=\"2\" z=\"3\" />\n"
           "            <scale x=\"2\" y=\"2\" z=\"2\" />\n"
           "            <node name=\"Child\">\n"
           "                <position x=\"0\" y=\"5\" z=\"0\" />\n"
           "                <entity name=\"TheCube\" meshFile=\"" + meshFile + "\" />\n"
           "            </node>\n"
           "            <node name=\"Sibling\">\n"
           "                <position x=\"-1\" y=\"0\" z=\"0\" />\n"
           "            </node>\n"
           "        </node>\n"
           "        <node name=\"Lamp\">\n"
           "            <position x=\"-4\" y=\"8\" z=\"1\" />\n"
           "            <light name=\"LampLight\" type=\"point\">\n"
           "                <colourDiffuse r=\"0.4\" g=\"0.4\" b=\"0.5\" />\n"
           "            </light>\n"
           "        </node>\n"
           "    </nodes>\n"
           "</scene>\n";
}

/*-------------------------------------------------------------------------*/

CORE::CString
BuildTestMaterialScript( void )
{GUCEF_TRACE;

    return "material TestRed\n"
           "{\n"
           "    technique\n"
           "    {\n"
           "        pass\n"
           "        {\n"
           "            ambient 0.2 0.0 0.0 1.0\n"
           "            diffuse 1.0 0.0 0.0 1.0\n"
           "            specular 0.0 0.0 0.0 1.0 12.5\n"
           "        }\n"
           "    }\n"
           "}\n"
           "\n"
           "material \"TestTextured\" : TestRed\n"
           "{\n"
           "    technique\n"
           "    {\n"
           "        pass\n"
           "        {\n"
           "            texture_unit\n"
           "            {\n"
           "                texture checker.png\n"
           "            }\n"
           "        }\n"
           "    }\n"
           "}\n";
}

/*-------------------------------------------------------------------------*/
