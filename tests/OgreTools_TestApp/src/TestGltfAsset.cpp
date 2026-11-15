/*
 *  OgreTools_TestApp: Test application for the Ogre asset tools and plugins (OgreAssimpConverter, OgreXMLConverter,
 *  OgreMeshUpgrader, OgreCodec_Assimp, OgrePlugin_DotScene)
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

#include "TestGltfAsset.h"

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

/*-------------------------------------------------------------------------*/

static CORE::CString
Num( UInt32 value )
{
    return CORE::ToString( value );
}

} // anonymous namespace

/*-------------------------------------------------------------------------*/

CORE::CString
BuildTestGltf( bool withStrip )
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
    CORE::CString extra;

    if ( withStrip )
    {
        // The strip: 3 rows of 2 vertices at y 0, 1 and 2, in the plane z = 0
        for ( int row=0; row<3; ++row )
        {
            buffer.Vec3( -0.5f, (float) row, 0.0f );
            buffer.Vec3(  0.5f, (float) row, 0.0f );
        }
        views += "," + buffer.EndView( true );                          // view 4: strip positions
        for ( int i=0; i<6; ++i )
            buffer.Vec3( 0.0f, 0.0f, 1.0f );
        views += "," + buffer.EndView( true );                          // view 5: strip normals
        for ( int row=0; row<3; ++row )
        {
            buffer.Float( 0.0f ); buffer.Float( 0.5f * row );
            buffer.Float( 1.0f ); buffer.Float( 0.5f * row );
        }
        views += "," + buffer.EndView( true );                          // view 6: strip texture coordinates

        // Row 0 is bound to Bone0 only, row 1 to both bones in equal parts, row 2 to Bone1 only
        static const UInt8 joints[ 3 ][ 4 ] = { { 0, 0, 0, 0 }, { 0, 1, 0, 0 }, { 1, 0, 0, 0 } };
        static const float weights[ 3 ][ 4 ] = { { 1, 0, 0, 0 }, { 0.5f, 0.5f, 0, 0 }, { 1, 0, 0, 0 } };
        for ( int row=0; row<3; ++row )
            for ( int v=0; v<2; ++v )
                for ( int j=0; j<4; ++j )
                    buffer.U8( joints[ row ][ j ] );
        views += "," + buffer.EndView( true );                          // view 7: strip joints
        for ( int row=0; row<3; ++row )
            for ( int v=0; v<2; ++v )
                for ( int j=0; j<4; ++j )
                    buffer.Float( weights[ row ][ j ] );
        views += "," + buffer.EndView( true );                          // view 8: strip weights

        static const UInt16 stripIndices[ 12 ] = { 0, 1, 2,  2, 1, 3,  2, 3, 4,  4, 3, 5 };
        for ( int i=0; i<12; ++i )
            buffer.U16( stripIndices[ i ] );
        views += "," + buffer.EndView( false );                         // view 9: strip indices

        // Inverse bind matrices, column major: Bone0 sits at the origin, Bone1 one up
        static const float inverseBind[ 2 ][ 16 ] =
        {
            { 1,0,0,0,  0,1,0,0,  0,0,1,0,  0, 0,0,1 },
            { 1,0,0,0,  0,1,0,0,  0,0,1,0,  0,-1,0,1 }
        };
        for ( int m=0; m<2; ++m )
            for ( int i=0; i<16; ++i )
                buffer.Float( inverseBind[ m ][ i ] );
        views += "," + buffer.EndPlainView();                           // view 10: inverse bind matrices

        buffer.Float( 0.0f ); buffer.Float( TestGltfAnimationLength );
        views += "," + buffer.EndPlainView();                           // view 11: animation times
        // Bone1 turns from rest to 45 degrees around z
        buffer.Float( 0.0f ); buffer.Float( 0.0f ); buffer.Float( 0.0f );        buffer.Float( 1.0f );
        buffer.Float( 0.0f ); buffer.Float( 0.0f ); buffer.Float( 0.3826834f );  buffer.Float( 0.9238795f );
        views += "," + buffer.EndPlainView();                           // view 12: animation rotations

        // The cube is bound to Bone0 only: a loader that makes a skeleton leaves out the meshes without bone weights
        for ( int i=0; i<24; ++i )
        {
            buffer.U8( 0 ); buffer.U8( 0 ); buffer.U8( 0 ); buffer.U8( 0 );
        }
        views += "," + buffer.EndView( true );                          // view 13: cube joints
        for ( int i=0; i<24; ++i )
        {
            buffer.Float( 1.0f ); buffer.Float( 0.0f ); buffer.Float( 0.0f ); buffer.Float( 0.0f );
        }
        views += "," + buffer.EndView( true );                          // view 14: cube weights

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
        extra =
            ",\"skins\":[{\"name\":\"StripSkin\",\"inverseBindMatrices\":10,\"skeleton\":2,\"joints\":[2,3]}]"
            ",\"animations\":[{\"name\":\"Wave\",\"samplers\":[{\"input\":11,\"output\":12,\"interpolation\":\"LINEAR\"}],"
            "\"channels\":[{\"sampler\":0,\"target\":{\"node\":3,\"path\":\"rotation\"}}]}]";
    }

    CORE::CString materials =
        "{\"name\":\"CubeMaterial\",\"pbrMetallicRoughness\":{\"baseColorTexture\":{\"index\":0},\"metallicFactor\":0.0,\"roughnessFactor\":0.8}}";
    if ( withStrip )
        materials += ",{\"name\":\"StripMaterial\",\"pbrMetallicRoughness\":{\"baseColorFactor\":[0.2,0.6,0.9,1.0],\"metallicFactor\":0.0}}";

    CORE::CString json =
        "{\"asset\":{\"version\":\"2.0\",\"generator\":\"OgreTools_TestApp\"}"
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
    return json;
}

/*-------------------------------------------------------------------------*/
