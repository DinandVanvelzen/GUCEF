/*
 *  assetpluginGLTF: the binary buffer of a glTF file with its buffer views and accessors
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

#include "assetpluginGLTF_CGltfBufferBuilder.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace GLTF {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGltfBufferBuilder::CGltfBufferBuilder( void )
    : m_binary( 4096, true )
    , m_accessors()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGltfBufferBuilder::~CGltfBufferBuilder()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CGltfBufferBuilder::GetComponentCount( EElementType type )
{GUCEF_TRACE;

    switch ( type )
    {
        case ELEMENT_SCALAR : return 1;
        case ELEMENT_VEC2 : return 2;
        case ELEMENT_VEC3 : return 3;
        case ELEMENT_VEC4 : return 4;
        default : return 16;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CGltfBufferBuilder::GetComponentSize( EComponentType type )
{GUCEF_TRACE;

    switch ( type )
    {
        case COMPONENT_UBYTE : return 1;
        case COMPONENT_USHORT : return 2;
        default : return 4;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CGltfBufferBuilder::AddAccessor( const void* data, UInt32 elementCount, EComponentType componentType, EElementType elementType, ETarget target, const Float32* min, const Float32* max )
{GUCEF_TRACE;

    SAccessor accessor;
    memset( &accessor, 0, sizeof( accessor ) );
    accessor.byteLength = elementCount * GetComponentCount( elementType ) * GetComponentSize( componentType );
    accessor.target = target;
    accessor.componentType = componentType;
    accessor.elementType = elementType;
    accessor.count = elementCount;

    // 4 byte alignment of the start of the view
    while ( 0 != m_binary.GetDataSize() % 4 )
    {
        const char zero = 0;
        m_binary.Append( &zero, 1 );
    }
    accessor.byteOffset = m_binary.GetDataSize();
    if ( accessor.byteLength > 0 )
        m_binary.Append( data, accessor.byteLength );

    if ( GUCEF_NULL != min && GUCEF_NULL != max )
    {
        accessor.hasBounds = true;
        const UInt32 components = GetComponentCount( elementType );
        for ( UInt32 c=0; c<components && c<16; ++c )
        {
            accessor.min[ c ] = min[ c ];
            accessor.max[ c ] = max[ c ];
        }
    }
    m_accessors.push_back( accessor );
    return (UInt32) m_accessors.size() - 1;
}

/*-------------------------------------------------------------------------*/

UInt32
CGltfBufferBuilder::AddFloats( const Float32* values, UInt32 elementCount, EElementType elementType, ETarget target, bool withBounds )
{GUCEF_TRACE;

    const UInt32 components = GetComponentCount( elementType );
    Float32 min[ 16 ];
    Float32 max[ 16 ];
    for ( UInt32 c=0; c<16; ++c )
    {
        min[ c ] = 0.0f;
        max[ c ] = 0.0f;
    }
    if ( withBounds && elementCount > 0 )
    {
        for ( UInt32 c=0; c<components; ++c )
        {
            min[ c ] = values[ c ];
            max[ c ] = values[ c ];
        }
        for ( UInt32 e=1; e<elementCount; ++e )
        {
            for ( UInt32 c=0; c<components; ++c )
            {
                const Float32 value = values[ e * components + c ];
                if ( value < min[ c ] ) min[ c ] = value;
                if ( value > max[ c ] ) max[ c ] = value;
            }
        }
    }
    return AddAccessor( values, elementCount, COMPONENT_FLOAT, elementType, target, withBounds && elementCount > 0 ? min : GUCEF_NULL, withBounds && elementCount > 0 ? max : GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

UInt32
CGltfBufferBuilder::AddIndices( const UInt32* indices, UInt32 count )
{GUCEF_TRACE;

    UInt32 highest = 0;
    for ( UInt32 i=0; i<count; ++i )
        if ( indices[ i ] > highest )
            highest = indices[ i ];

    if ( highest < 65535 )
    {
        GUCEF::vector< UInt16 > narrow;
        narrow.resize( count );
        for ( UInt32 i=0; i<count; ++i )
            narrow[ i ] = (UInt16) indices[ i ];
        return AddAccessor( count > 0 ? &narrow[ 0 ] : GUCEF_NULL, count, COMPONENT_USHORT, ELEMENT_SCALAR, TARGET_ELEMENT_ARRAY, GUCEF_NULL, GUCEF_NULL );
    }
    return AddAccessor( indices, count, COMPONENT_UINT, ELEMENT_SCALAR, TARGET_ELEMENT_ARRAY, GUCEF_NULL, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

UInt32
CGltfBufferBuilder::AddUInt16Vec4( const UInt32* values, UInt32 elementCount, ETarget target )
{GUCEF_TRACE;

    GUCEF::vector< UInt16 > narrow;
    narrow.resize( (size_t) elementCount * 4 );
    for ( size_t i=0; i<narrow.size(); ++i )
        narrow[ i ] = (UInt16) values[ i ];
    return AddAccessor( narrow.empty() ? GUCEF_NULL : &narrow[ 0 ], elementCount, COMPONENT_USHORT, ELEMENT_VEC4, target, GUCEF_NULL, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

UInt32
CGltfBufferBuilder::GetAccessorCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_accessors.size();
}

/*-------------------------------------------------------------------------*/

const CORE::CDynamicBuffer&
CGltfBufferBuilder::GetBinary( void ) const
{GUCEF_TRACE;

    return m_binary;
}

/*-------------------------------------------------------------------------*/

UInt32
CGltfBufferBuilder::GetByteLength( void ) const
{GUCEF_TRACE;

    return ( m_binary.GetDataSize() + 3 ) & ~(UInt32) 3;
}

/*-------------------------------------------------------------------------*/

void
CGltfBufferBuilder::WriteTo( CGltfJsonWriter& writer ) const
{GUCEF_TRACE;

    static const char* typeNames[] = { "SCALAR", "VEC2", "VEC3", "VEC4", "MAT4" };

    writer.Key( "bufferViews" );
    writer.BeginArray();
    for ( size_t i=0; i<m_accessors.size(); ++i )
    {
        const SAccessor& accessor = m_accessors[ i ];
        writer.BeginObject();
        writer.Member( "buffer", (Int32) 0 );
        writer.Member( "byteOffset", accessor.byteOffset );
        writer.Member( "byteLength", accessor.byteLength );
        if ( 0 != accessor.target )
            writer.Member( "target", accessor.target );
        writer.EndObject();
    }
    writer.EndArray();

    writer.Key( "accessors" );
    writer.BeginArray();
    for ( size_t i=0; i<m_accessors.size(); ++i )
    {
        const SAccessor& accessor = m_accessors[ i ];
        writer.BeginObject();
        writer.Member( "bufferView", (UInt32) i );
        writer.Member( "componentType", accessor.componentType );
        writer.Member( "count", accessor.count );
        writer.Member( "type", typeNames[ accessor.elementType ] );
        if ( accessor.hasBounds )
        {
            const UInt32 components = GetComponentCount( (EElementType) accessor.elementType );
            writer.MemberFloatArray( "min", accessor.min, components );
            writer.MemberFloatArray( "max", accessor.max, components );
        }
        writer.EndObject();
    }
    writer.EndArray();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GLTF */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
