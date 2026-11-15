/*
 *  assetpluginUNITY: the decoder of serialised Unity Mesh objects
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

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLREADER_H
#include "assetpluginUNITY_CUnityYamlReader.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLREADER_H ? */

#include "assetpluginUNITY_CUnityMeshDecoder.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNITY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

struct SChannel
{
    UInt32 stream;
    UInt32 offset;
    UInt32 format;
    UInt32 dimension;
};

/** The size in bytes of one component of a channel format */
UInt32
FormatSize( UInt32 format, bool newLayout )
{
    if ( newLayout )
    {
        switch ( format )
        {
            case 0: return 4;
            case 1: return 2;
            case 2: case 3: case 6: case 7: return 1;
            case 4: case 5: case 8: case 9: return 2;
            case 10: case 11: return 4;
            default: return 0;
        }
    }
    switch ( format )
    {
        case 0: return 4;
        case 1: return 2;
        case 2: case 3: return 1;
        case 4: return 4;
        default: return 0;
    }
}

Float32
HalfToFloat( UInt16 half )
{
    const UInt32 sign = ( half >> 15 ) & 1;
    const UInt32 exponent = ( half >> 10 ) & 0x1F;
    const UInt32 mantissa = half & 0x3FF;
    Float32 value = 0.0f;
    if ( 0 == exponent )
        value = (Float32) ldexp( (double) mantissa, -24 );
    else
    if ( 31 == exponent )
        value = 0 == mantissa ? (Float32) HUGE_VAL : 0.0f;
    else
        value = (Float32) ldexp( (double) ( mantissa | 0x400 ), (int) exponent - 25 );
    return sign ? -value : value;
}

/** A component of a vertex as a float: floats as they are, normalised integers to 0..1 or -1..1 */
Float32
ReadComponent( const UInt8* data, UInt32 format, bool newLayout )
{
    switch ( format )
    {
        case 0:
        {
            Float32 value = 0.0f;
            memcpy( &value, data, 4 );
            return value;
        }
        case 1:
        {
            UInt16 half = (UInt16) ( data[ 0 ] | ( data[ 1 ] << 8 ) );
            return HalfToFloat( half );
        }
        case 2:
            return (Float32) data[ 0 ] / 255.0f;
        case 3:
            return newLayout ? ( (Int8) data[ 0 ] ) / 127.0f : (Float32) data[ 0 ];
        default:
            return 0.0f;
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CUnityMeshDecoder::DecodeHex( const CORE::CString& text, GUCEF::vector< UInt8 >& bytes )
{GUCEF_TRACE;

    bytes.clear();
    const char* p = text.C_String();
    if ( GUCEF_NULL == p )
        return true;
    const size_t length = strlen( p );
    if ( 0 != length % 2 )
        return false;
    bytes.reserve( length / 2 );
    for ( size_t i=0; i<length; i+=2 )
    {
        UInt32 value = 0;
        for ( UInt32 d=0; d<2; ++d )
        {
            const char c = p[ i+d ];
            UInt32 digit = 0;
            if ( c >= '0' && c <= '9' ) digit = (UInt32) ( c - '0' );
            else if ( c >= 'a' && c <= 'f' ) digit = (UInt32) ( c - 'a' + 10 );
            else if ( c >= 'A' && c <= 'F' ) digit = (UInt32) ( c - 'A' + 10 );
            else return false;
            value = value * 16 + digit;
        }
        bytes.push_back( (UInt8) value );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnityMeshDecoder::Decode( const CORE::CDataNode& meshObject, ASSETCONVERTER::CAssetMesh& mesh, CORE::CString& problem )
{GUCEF_TRACE;

    mesh = ASSETCONVERTER::CAssetMesh();
    mesh.name = CUnityYamlReader::GetString( meshObject, "m_Name" );

    const CORE::CDataNode* compressed = meshObject.FindChild( "m_CompressedMesh" );
    const CORE::CDataNode* compressedVertices = GUCEF_NULL != compressed ? compressed->FindChild( "m_Vertices" ) : GUCEF_NULL;
    const CORE::CDataNode* vertexData = meshObject.FindChild( "m_VertexData" );
    if ( GUCEF_NULL == vertexData )
    {
        problem = "the mesh has no m_VertexData";
        return false;
    }
    const UInt32 vertexCount = (UInt32) CUnityYamlReader::GetInt64( *vertexData, "m_VertexCount", 0 );
    if ( 0 == vertexCount )
    {
        if ( GUCEF_NULL != compressedVertices && CUnityYamlReader::GetInt64( *compressedVertices, "m_NumItems", 0 ) > 0 )
            problem = "the mesh is stored compressed, which is not decoded";
        else
            problem = "the mesh has no vertices";
        return false;
    }

    // the channels and the layout of the streams
    CUnityYamlReader::TNodeList channelNodes;
    CUnityYamlReader::GetItems( vertexData->FindChild( "m_Channels" ), channelNodes );
    const bool newLayout = channelNodes.size() > 8;
    GUCEF::vector< SChannel > channels;
    UInt32 streamCount = 0;
    for ( size_t c=0; c<channelNodes.size(); ++c )
    {
        SChannel channel;
        channel.stream = (UInt32) CUnityYamlReader::GetInt64( *channelNodes[ c ], "stream", 0 );
        channel.offset = (UInt32) CUnityYamlReader::GetInt64( *channelNodes[ c ], "offset", 0 );
        channel.format = (UInt32) CUnityYamlReader::GetInt64( *channelNodes[ c ], "format", 0 );
        channel.dimension = (UInt32) CUnityYamlReader::GetInt64( *channelNodes[ c ], "dimension", 0 ) & 0xF;
        channels.push_back( channel );
        if ( channel.dimension > 0 && channel.stream + 1 > streamCount )
            streamCount = channel.stream + 1;
    }
    if ( channels.empty() || 0 == channels[ 0 ].dimension )
    {
        problem = "the mesh has no position channel";
        return false;
    }
    GUCEF::vector< UInt32 > strides( streamCount, 0 );
    for ( size_t c=0; c<channels.size(); ++c )
    {
        if ( 0 == channels[ c ].dimension )
            continue;
        const UInt32 end = channels[ c ].offset + channels[ c ].dimension * FormatSize( channels[ c ].format, newLayout );
        if ( end > strides[ channels[ c ].stream ] )
            strides[ channels[ c ].stream ] = end;
    }
    GUCEF::vector< UInt32 > streamStarts( streamCount, 0 );
    UInt32 position = 0;
    for ( UInt32 s=0; s<streamCount; ++s )
    {
        position = ( position + 15 ) & ~15u;
        streamStarts[ s ] = position;
        position += strides[ s ] * vertexCount;
    }

    GUCEF::vector< UInt8 > data;
    if ( !DecodeHex( CUnityYamlReader::GetString( *vertexData, "_typelessdata" ), data ) )
    {
        problem = "the vertex data is not hexadecimal";
        return false;
    }
    if ( data.size() < position )
    {
        // a single stream needs no padding at its end
        const UInt32 last = streamCount > 0 ? streamStarts[ streamCount-1 ] + strides[ streamCount-1 ] * vertexCount : 0;
        if ( data.size() < last )
        {
            problem = "the vertex data holds " + CORE::UInt32ToString( (UInt32) data.size() ) + " bytes, the channels need " + CORE::UInt32ToString( last );
            return false;
        }
    }

    // the vertex streams: position, normal, UV 0 and colour, in the slots of the layout
    const UInt32 normalSlot = 1;
    const UInt32 colourSlot = newLayout ? 3 : 2;
    const UInt32 uvSlot = newLayout ? 4 : 3;
    ASSETCONVERTER::CAssetSubMesh all;
    for ( UInt32 v=0; v<vertexCount; ++v )
    {
        for ( UInt32 slot=0; slot<channels.size(); ++slot )
        {
            const SChannel& channel = channels[ slot ];
            if ( 0 == channel.dimension || ( slot != 0 && slot != normalSlot && slot != colourSlot && slot != uvSlot ) )
                continue;
            const UInt32 size = FormatSize( channel.format, newLayout );
            if ( 0 == size )
                continue;
            const UInt8* at = &data[ 0 ] + streamStarts[ channel.stream ] + strides[ channel.stream ] * v + channel.offset;
            Float32 values[ 4 ] = { 0.0f, 0.0f, 0.0f, 1.0f };
            for ( UInt32 d=0; d<channel.dimension && d<4; ++d )
                values[ d ] = ReadComponent( at + d * size, channel.format, newLayout );
            if ( 0 == slot )
            {
                all.positions.push_back( values[ 0 ] ); all.positions.push_back( values[ 1 ] ); all.positions.push_back( values[ 2 ] );
            }
            else
            if ( normalSlot == slot && channel.dimension >= 3 )
            {
                all.normals.push_back( values[ 0 ] ); all.normals.push_back( values[ 1 ] ); all.normals.push_back( values[ 2 ] );
            }
            else
            if ( uvSlot == slot && channel.dimension >= 2 )
            {
                all.uvSets[ 0 ].push_back( values[ 0 ] ); all.uvSets[ 0 ].push_back( 1.0f - values[ 1 ] );
            }
            else
            if ( colourSlot == slot && channel.dimension >= 3 )
            {
                for ( UInt32 d=0; d<4; ++d )
                    all.colours.push_back( values[ d ] );
            }
        }
    }

    // the indices of the submeshes
    GUCEF::vector< UInt8 > indexBytes;
    if ( !DecodeHex( CUnityYamlReader::GetString( meshObject, "m_IndexBuffer" ), indexBytes ) )
    {
        problem = "the index buffer is not hexadecimal";
        return false;
    }
    const bool wide = 1 == CUnityYamlReader::GetInt32( meshObject, "m_IndexFormat", 0 );
    const UInt32 indexSize = wide ? 4 : 2;
    CUnityYamlReader::TNodeList subMeshNodes;
    CUnityYamlReader::GetItems( meshObject.FindChild( "m_SubMeshes" ), subMeshNodes );
    for ( size_t s=0; s<subMeshNodes.size(); ++s )
    {
        const CORE::CDataNode& item = *subMeshNodes[ s ];
        const UInt32 firstByte = (UInt32) CUnityYamlReader::GetInt64( item, "firstByte", 0 );
        const UInt32 indexCount = (UInt32) CUnityYamlReader::GetInt64( item, "indexCount", 0 );
        const Int32 topology = CUnityYamlReader::GetInt32( item, "topology", 0 );
        const UInt32 baseVertex = (UInt32) CUnityYamlReader::GetInt64( item, "baseVertex", 0 );
        if ( 0 != topology )
        {
            problem = "submesh " + CORE::UInt32ToString( (UInt32) s ) + " is not made of triangles: it is left out";
            continue;
        }
        if ( (size_t) firstByte + (size_t) indexCount * indexSize > indexBytes.size() )
        {
            problem = "submesh " + CORE::UInt32ToString( (UInt32) s ) + " reaches past the index buffer";
            return false;
        }
        ASSETCONVERTER::CAssetSubMesh subMesh = all;
        for ( UInt32 i=0; i+2<indexCount; i+=3 )
        {
            UInt32 triangle[ 3 ];
            bool valid = true;
            for ( UInt32 k=0; k<3; ++k )
            {
                const UInt8* at = &indexBytes[ firstByte + ( i + k ) * indexSize ];
                UInt32 index = wide ? ( (UInt32) at[ 0 ] | ( (UInt32) at[ 1 ] << 8 ) | ( (UInt32) at[ 2 ] << 16 ) | ( (UInt32) at[ 3 ] << 24 ) ) : ( (UInt32) at[ 0 ] | ( (UInt32) at[ 1 ] << 8 ) );
                index += baseVertex;
                valid = valid && index < vertexCount;
                triangle[ k ] = index;
            }
            if ( !valid )
                continue;
            // Unity's front faces turn clockwise in its left-handed frame, which is counter-clockwise by the right-handed rule
            subMesh.indices.push_back( triangle[ 0 ] );
            subMesh.indices.push_back( triangle[ 1 ] );
            subMesh.indices.push_back( triangle[ 2 ] );
        }
        if ( !subMesh.indices.empty() )
            mesh.subMeshes.push_back( subMesh );
    }
    if ( mesh.subMeshes.empty() )
    {
        if ( problem.IsNULLOrEmpty() )
            problem = "the mesh has no triangles";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNITY */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
