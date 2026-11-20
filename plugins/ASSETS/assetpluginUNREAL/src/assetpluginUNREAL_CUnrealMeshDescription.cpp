/*
 *  assetpluginUNREAL: The mesh description of a static mesh source: elements and attributes
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALMESHDESCRIPTION_H
#include "assetpluginUNREAL_CUnrealMeshDescription.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALMESHDESCRIPTION_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALMESHDESCRIPTION_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The bytes a value of an attribute type takes; 0 for the type name, which is a string */
static UInt32
BytesOfType( UInt32 type )
{GUCEF_TRACE;

    switch ( type )
    {
        case CUnrealMeshDescription::ATTRIBUTE_VECTOR4: return 16;
        case CUnrealMeshDescription::ATTRIBUTE_VECTOR3: return 12;
        case CUnrealMeshDescription::ATTRIBUTE_VECTOR2: return 8;
        case CUnrealMeshDescription::ATTRIBUTE_FLOAT: return 4;
        case CUnrealMeshDescription::ATTRIBUTE_INT32: return 4;
        case CUnrealMeshDescription::ATTRIBUTE_BOOL: return 1;
        default: return 0;
    }
}

/*-------------------------------------------------------------------------*/

/** A cursor over the payload that never reads outside it; the first failure stays */
class CCursor
{
    public:

    CCursor( const UInt8* bytes, UInt64 size ) : m_bytes( bytes ), m_size( size ), m_position( 0 ), m_error() {}

    bool Fail( const CORE::CString& reason )
    {
        if ( m_error.IsNULLOrEmpty() )
            m_error = reason + " (at offset " + CORE::UInt64ToString( m_position ) + ")";
        return false;
    }

    bool HasError( void ) const { return !m_error.IsNULLOrEmpty(); }
    const CORE::CString& GetError( void ) const { return m_error; }
    UInt64 GetRemaining( void ) const { return m_size - m_position; }

    UInt32 ReadUInt32( void )
    {
        if ( HasError() || m_size - m_position < 4 )
        {
            Fail( "The mesh description ends inside a number" );
            return 0;
        }
        const UInt8* p = m_bytes + m_position;
        m_position += 4;
        return (UInt32) p[ 0 ] | ( (UInt32) p[ 1 ] << 8 ) | ( (UInt32) p[ 2 ] << 16 ) | ( (UInt32) p[ 3 ] << 24 );
    }

    /** The number of the bytes ahead (from the current position, plus ahead) as it is stored; false when it lies outside the payload */
    bool PeekUInt32( UInt64 ahead, UInt32& value ) const
    {
        if ( HasError() || ahead + 4 > m_size - m_position )
            return false;
        const UInt8* p = m_bytes + m_position + ahead;
        value = (UInt32) p[ 0 ] | ( (UInt32) p[ 1 ] << 8 ) | ( (UInt32) p[ 2 ] << 16 ) | ( (UInt32) p[ 3 ] << 24 );
        return true;
    }

    /** Whether a string with the length prefix at the offset ahead is a plausible name: its length and a zero behind it */
    bool PlausibleString( UInt64 ahead ) const
    {
        UInt32 length = 0;
        if ( !PeekUInt32( ahead, length ) || length < 1 || length > 4096 || ahead + 4 + length > m_size - m_position )
            return false;
        return 0 == m_bytes[ m_position + ahead + 4 + length - 1 ];
    }

    bool ReadBytes( void* dest, UInt64 count )
    {
        if ( HasError() || count > m_size - m_position )
            return Fail( "The mesh description ends inside a block of values" );
        if ( count > 0 )
            memcpy( dest, m_bytes + m_position, (size_t) count );
        m_position += count;
        return true;
    }

    bool Skip( UInt64 count )
    {
        if ( HasError() || count > m_size - m_position )
            return Fail( "The mesh description ends inside a block of values" );
        m_position += count;
        return true;
    }

    bool ReadString( CORE::CString& text )
    {
        const UInt32 length = ReadUInt32();
        if ( HasError() )
            return false;
        if ( 0 == length || length > 4096 || length > m_size - m_position )
            return Fail( "A name of the mesh description has a length that is not possible" );
        // the name of the attribute Position is stored with a blank behind it: trailing blanks are no part of a name
        UInt32 characters = length - 1;
        while ( characters > 0 && ' ' == (char) m_bytes[ m_position + characters - 1 ] )
            --characters;
        text = CORE::CString( reinterpret_cast< const char* >( m_bytes + m_position ), characters );
        m_position += length;
        return true;
    }

    private:
    const UInt8* m_bytes;
    UInt64 m_size;
    UInt64 m_position;
    CORE::CString m_error;
};

/*-------------------------------------------------------------------------*/

CUnrealMeshDescription::SAttribute::SAttribute( void )
    : name()
    , type( 0 )
    , valuesPerId( 1 )
    , slotCount( 0 )
    , bytesPerValue( 0 )
    , blocks()
    , names()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

Float32
CUnrealMeshDescription::SAttribute::GetFloat( UInt32 block, UInt32 id, UInt32 index, UInt32 component ) const
{GUCEF_TRACE;

    if ( block >= blocks.size() || id >= slotCount || index >= valuesPerId )
        return 0.0f;
    const size_t offset = ( (size_t) id * valuesPerId + index ) * bytesPerValue + (size_t) component * 4;
    if ( offset + 4 > blocks[ block ].size() )
        return 0.0f;
    Float32 value;
    memcpy( &value, &blocks[ block ][ offset ], 4 );
    return value;
}

/*-------------------------------------------------------------------------*/

Int32
CUnrealMeshDescription::SAttribute::GetInt32( UInt32 block, UInt32 id, UInt32 index ) const
{GUCEF_TRACE;

    if ( block >= blocks.size() || id >= slotCount || index >= valuesPerId )
        return -1;
    const size_t offset = ( (size_t) id * valuesPerId + index ) * bytesPerValue;
    if ( offset + 4 > blocks[ block ].size() )
        return -1;
    Int32 value;
    memcpy( &value, &blocks[ block ][ offset ], 4 );
    return value;
}

/*-------------------------------------------------------------------------*/

CUnrealMeshDescription::SChannel::SChannel( void )
    : slotCount( 0 )
    , allocated()
    , attributes()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CUnrealMeshDescription::SAttribute*
CUnrealMeshDescription::SChannel::Find( const CORE::CString& name ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<attributes.size(); ++i )
        if ( attributes[ i ].name == name )
            return &attributes[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealMeshDescription::SChannel::IsAllocated( UInt32 id ) const
{GUCEF_TRACE;

    return id < allocated.size() && 0 != allocated[ id ];
}

/*-------------------------------------------------------------------------*/

UInt32
CUnrealMeshDescription::SChannel::GetAllocatedCount( void ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( size_t i=0; i<allocated.size(); ++i )
        if ( 0 != allocated[ i ] )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

CUnrealMeshDescription::CUnrealMeshDescription( void )
    : m_elements()
    , m_error()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CUnrealMeshDescription::GetError( void ) const
{GUCEF_TRACE;

    return m_error;
}

/*-------------------------------------------------------------------------*/

const GUCEF::vector< CUnrealMeshDescription::SElement >&
CUnrealMeshDescription::GetElements( void ) const
{GUCEF_TRACE;

    return m_elements;
}

/*-------------------------------------------------------------------------*/

const CUnrealMeshDescription::SElement*
CUnrealMeshDescription::FindElement( const CORE::CString& name ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_elements.size(); ++i )
        if ( m_elements[ i ].name == name )
            return &m_elements[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

static bool
ReadAttribute( CCursor& cursor, UInt32 slotCount, CUnrealMeshDescription::SAttribute& attribute )
{GUCEF_TRACE;

    if ( !cursor.ReadString( attribute.name ) )
        return false;
    attribute.type = cursor.ReadUInt32();
    attribute.valuesPerId = cursor.ReadUInt32();
    attribute.slotCount = cursor.ReadUInt32();
    const UInt32 blockCount = cursor.ReadUInt32();
    if ( cursor.HasError() )
        return false;
    const UInt32 typeBytes = BytesOfType( attribute.type );
    if ( attribute.type > CUnrealMeshDescription::ATTRIBUTE_NAME || attribute.valuesPerId < 1 || attribute.valuesPerId > 64 || attribute.slotCount != slotCount || blockCount > 64 )
        return cursor.Fail( "The attribute " + attribute.name + " has a type, count or shape that is not known" );
    attribute.bytesPerValue = typeBytes;
    for ( UInt32 b=0; b<blockCount; ++b )
    {
        const UInt32 valuesPerId = cursor.ReadUInt32();
        const UInt32 valueBytes = CUnrealMeshDescription::ATTRIBUTE_NAME == attribute.type ? 0 : cursor.ReadUInt32();     // names have no size of a value
        const UInt32 valueCount = cursor.ReadUInt32();
        if ( cursor.HasError() )
            return false;
        if ( valuesPerId != attribute.valuesPerId || valueCount != (UInt64) slotCount * valuesPerId )
            return cursor.Fail( "The attribute " + attribute.name + " has a block whose value count does not fit its slots" );
        if ( ( CUnrealMeshDescription::ATTRIBUTE_NAME == attribute.type ) )
        {
            // two forms: one name per value (UE 5.3), or a table of the different names and what follows that: with a single
            // name all values have it (UE 5.6 and later); the form is told by what the next words are
            UInt32 first = 0;
            const bool tableForm = cursor.PeekUInt32( 0, first ) && first >= 1 && first <= valueCount && cursor.PlausibleString( 4 ) && !( first > 1 && cursor.PlausibleString( 0 ) );
            GUCEF::vector< CORE::CString > values;
            if ( tableForm )
            {
                const UInt32 unique = cursor.ReadUInt32();
                if ( unique > cursor.GetRemaining() / 5 )
                    return cursor.Fail( "The attribute " + attribute.name + " has more names than the payload can hold" );
                GUCEF::vector< CORE::CString > table;
                for ( UInt32 n=0; n<unique; ++n )
                {
                    CORE::CString text;
                    if ( !cursor.ReadString( text ) )
                        return false;
                    table.push_back( text );
                }
                if ( 1 == unique )
                    values.assign( valueCount, table[ 0 ] );
                else
                {
                    // the names of several different values are followed by the table index of every value (not seen in the samples)
                    if ( (UInt64) valueCount * 4 > cursor.GetRemaining() )
                        return cursor.Fail( "The attribute " + attribute.name + " has more name indices than the payload can hold" );
                    for ( UInt32 v=0; v<valueCount; ++v )
                    {
                        const UInt32 index = cursor.ReadUInt32();
                        if ( index >= unique )
                            return cursor.Fail( "The attribute " + attribute.name + " has a name index outside its table" );
                        values.push_back( table[ index ] );
                    }
                }
            }
            else
            {
                if ( valueCount > cursor.GetRemaining() / 5 )
                    return cursor.Fail( "The attribute " + attribute.name + " has more names than the payload can hold" );
                values.reserve( valueCount );
                for ( UInt32 v=0; v<valueCount; ++v )
                {
                    CORE::CString text;
                    if ( !cursor.ReadString( text ) )
                        return false;
                    values.push_back( text );
                }
            }
            attribute.names.push_back( values );
            attribute.blocks.push_back( GUCEF::vector< UInt8 >() );
        }
        else
        {
            if ( valueBytes != typeBytes )
                return cursor.Fail( "The attribute " + attribute.name + " has values of " + CORE::UInt32ToString( valueBytes ) + " bytes where its type has " + CORE::UInt32ToString( typeBytes ) );
            const UInt64 total = (UInt64) valueCount * valueBytes;
            if ( total > cursor.GetRemaining() )
                return cursor.Fail( "The attribute " + attribute.name + " holds more values than the payload" );
            GUCEF::vector< UInt8 > data( (size_t) total );
            if ( total > 0 && !cursor.ReadBytes( &data[ 0 ], total ) )
                return false;
            attribute.blocks.push_back( data );
        }
    }
    // the default value, then the flags
    if ( ( CUnrealMeshDescription::ATTRIBUTE_NAME == attribute.type ) )
    {
        CORE::CString text;
        if ( !cursor.ReadString( text ) )
            return false;
    }
    else if ( !cursor.Skip( CUnrealMeshDescription::ATTRIBUTE_BOOL == attribute.type ? 4 : typeBytes ) )   // a boolean default is written as 32 bits like the engine writes booleans
        return false;
    cursor.ReadUInt32();
    return !cursor.HasError();
}

/*-------------------------------------------------------------------------*/

bool
CUnrealMeshDescription::Read( const UInt8* bytes, UInt64 size )
{GUCEF_TRACE;

    m_elements.clear();
    m_error.Clear();
    CCursor cursor( bytes, size );
    const UInt32 elementCount = cursor.ReadUInt32();
    if ( cursor.HasError() || elementCount < 1 || elementCount > 64 )
    {
        cursor.Fail( "The mesh description does not start with a plausible number of elements" );
        m_error = cursor.GetError();
        return false;
    }
    for ( UInt32 e=0; e<elementCount; ++e )
    {
        SElement element;
        if ( !cursor.ReadString( element.name ) )
            break;
        const UInt32 channelCount = cursor.ReadUInt32();
        if ( cursor.HasError() )
            break;
        if ( channelCount < 1 || channelCount > 256 )
        {
            cursor.Fail( "The element " + element.name + " has a number of channels that is not possible" );
            break;
        }
        for ( UInt32 c=0; c<channelCount; ++c )
        {
            SChannel channel;
            channel.slotCount = cursor.ReadUInt32();
            if ( cursor.HasError() )
                break;
            const UInt64 words = ( (UInt64) channel.slotCount + 31 ) / 32;
            if ( channel.slotCount > 0x7FFFFFF || words * 4 > cursor.GetRemaining() )
            {
                cursor.Fail( "The element " + element.name + " has more slots than the payload can hold" );
                break;
            }
            channel.allocated.assign( channel.slotCount, 0 );
            for ( UInt64 w=0; w<words; ++w )
            {
                const UInt32 word = cursor.ReadUInt32();
                for ( UInt32 bit=0; bit<32; ++bit )
                {
                    const UInt64 slot = w * 32 + bit;
                    if ( slot < channel.slotCount && ( word & ( 1U << bit ) ) )
                        channel.allocated[ (size_t) slot ] = 1;
                }
            }
            const UInt32 freeCount = cursor.ReadUInt32();
            const UInt32 slotsAgain = cursor.ReadUInt32();
            const UInt32 attributeCount = cursor.ReadUInt32();
            if ( cursor.HasError() )
                break;
            if ( 0 != freeCount || slotsAgain != channel.slotCount || attributeCount > 256 )
            {
                cursor.Fail( "The element " + element.name + " has a list of free slots or a slot count that this reader does not know" );
                break;
            }
            for ( UInt32 a=0; a<attributeCount; ++a )
            {
                SAttribute attribute;
                if ( !ReadAttribute( cursor, channel.slotCount, attribute ) )
                    break;
                channel.attributes.push_back( attribute );
            }
            if ( cursor.HasError() )
                break;
            element.channels.push_back( channel );
        }
        if ( cursor.HasError() )
            break;
        m_elements.push_back( element );
    }
    if ( cursor.HasError() )
    {
        m_error = cursor.GetError();
        m_elements.clear();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** A bit array of the UE 4 layout: the number of bits, then 32-bit words; the allocated ids and their number */
static bool
ReadUe4BitArray( CCursor& cursor, const char* what, GUCEF::vector< UInt8 >& allocated, UInt32& setCount )
{GUCEF_TRACE;

    setCount = 0;
    const UInt32 bits = cursor.ReadUInt32();
    if ( cursor.HasError() )
        return false;
    const UInt64 words = ( (UInt64) bits + 31 ) / 32;
    if ( bits > 0x7FFFFFF || words * 4 > cursor.GetRemaining() )
        return cursor.Fail( CORE::CString( "The " ) + what + " of the mesh description have more ids than the payload can hold" );
    allocated.assign( bits, 0 );
    for ( UInt64 w=0; w<words; ++w )
    {
        const UInt32 word = cursor.ReadUInt32();
        for ( UInt32 bit=0; bit<32; ++bit )
        {
            const UInt64 slot = w * 32 + bit;
            if ( slot < bits && ( word & ( 1U << bit ) ) )
            {
                allocated[ (size_t) slot ] = 1;
                ++setCount;
            }
        }
    }
    return !cursor.HasError();
}

/*-------------------------------------------------------------------------*/

/** An attribute of the UE 4 layout: name, type, id count, channels (size of a value, count, values; names as strings), default, flags */
static bool
ReadUe4Attribute( CCursor& cursor, UInt32 slotCount, CUnrealMeshDescription::SAttribute& attribute )
{GUCEF_TRACE;

    if ( !cursor.ReadString( attribute.name ) )
        return false;
    attribute.type = cursor.ReadUInt32();
    const UInt32 count = cursor.ReadUInt32();
    const UInt32 channelCount = cursor.ReadUInt32();
    if ( cursor.HasError() )
        return false;
    if ( attribute.type > CUnrealMeshDescription::ATTRIBUTE_NAME || count != slotCount || channelCount > 64 )
        return cursor.Fail( "The attribute " + attribute.name + " has a type, count or number of channels that is not known" );
    const UInt32 typeBytes = BytesOfType( attribute.type );
    attribute.valuesPerId = 1;
    attribute.slotCount = slotCount;
    attribute.bytesPerValue = typeBytes;
    for ( UInt32 c=0; c<channelCount; ++c )
    {
        if ( CUnrealMeshDescription::ATTRIBUTE_NAME == attribute.type )
        {
            const UInt32 valueCount = cursor.ReadUInt32();
            if ( cursor.HasError() )
                return false;
            if ( valueCount != slotCount || valueCount > cursor.GetRemaining() / 5 )
                return cursor.Fail( "The attribute " + attribute.name + " has a number of names that does not fit its ids" );
            GUCEF::vector< CORE::CString > values;
            values.reserve( valueCount );
            for ( UInt32 v=0; v<valueCount; ++v )
            {
                CORE::CString text;
                if ( !cursor.ReadString( text ) )
                    return false;
                values.push_back( text );
            }
            attribute.names.push_back( values );
            attribute.blocks.push_back( GUCEF::vector< UInt8 >() );
            continue;
        }
        const UInt32 valueBytes = cursor.ReadUInt32();
        const UInt32 valueCount = cursor.ReadUInt32();
        if ( cursor.HasError() )
            return false;
        if ( valueBytes != typeBytes || valueCount != slotCount )
            return cursor.Fail( "The attribute " + attribute.name + " has values of " + CORE::UInt32ToString( valueBytes ) + " bytes or a count that does not fit its type and ids" );
        const UInt64 total = (UInt64) valueCount * valueBytes;
        if ( total > cursor.GetRemaining() )
            return cursor.Fail( "The attribute " + attribute.name + " holds more values than the payload" );
        GUCEF::vector< UInt8 > data( (size_t) total );
        if ( total > 0 && !cursor.ReadBytes( &data[ 0 ], total ) )
            return false;
        attribute.blocks.push_back( data );
    }
    // the default value (a boolean as 32 bits), then the flags
    if ( CUnrealMeshDescription::ATTRIBUTE_NAME == attribute.type )
    {
        CORE::CString text;
        if ( !cursor.ReadString( text ) )
            return false;
    }
    else if ( !cursor.Skip( CUnrealMeshDescription::ATTRIBUTE_BOOL == attribute.type ? 4 : typeBytes ) )
        return false;
    cursor.ReadUInt32();
    return !cursor.HasError();
}

/*-------------------------------------------------------------------------*/

/** An attribute set of the UE 4 layout: the id count, the number of attributes, the attributes */
static bool
ReadUe4AttributeSet( CCursor& cursor, const char* what, CUnrealMeshDescription::SChannel& channel )
{GUCEF_TRACE;

    const UInt32 elementCount = cursor.ReadUInt32();
    const UInt32 attributeCount = cursor.ReadUInt32();
    if ( cursor.HasError() )
        return false;
    if ( elementCount != channel.slotCount || attributeCount > 256 )
        return cursor.Fail( CORE::CString( "The attributes of the " ) + what + " are for " + CORE::UInt32ToString( elementCount ) + " ids where the element has " + CORE::UInt32ToString( channel.slotCount ) );
    for ( UInt32 a=0; a<attributeCount; ++a )
    {
        CUnrealMeshDescription::SAttribute attribute;
        if ( !ReadUe4Attribute( cursor, channel.slotCount, attribute ) )
            return false;
        channel.attributes.push_back( attribute );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** An int32 attribute with one value per id, all -1 until set */
static void
MakeIndexAttribute( const char* name, UInt32 slotCount, UInt32 valuesPerId, CUnrealMeshDescription::SAttribute& attribute )
{GUCEF_TRACE;

    attribute.name = name;
    attribute.type = CUnrealMeshDescription::ATTRIBUTE_INT32;
    attribute.valuesPerId = valuesPerId;
    attribute.slotCount = slotCount;
    attribute.bytesPerValue = 4;
    attribute.blocks.assign( 1, GUCEF::vector< UInt8 >( (size_t) slotCount * valuesPerId * 4, 0xFF ) );
}

/*-------------------------------------------------------------------------*/

static void
SetIndexValue( CUnrealMeshDescription::SAttribute& attribute, UInt32 id, UInt32 index, UInt32 value )
{
    memcpy( &attribute.blocks[ 0 ][ ( (size_t) id * attribute.valuesPerId + index ) * 4 ], &value, 4 );
}

/*-------------------------------------------------------------------------*/

bool
CUnrealMeshDescription::ReadUe4Layout( const UInt8* bytes, UInt64 size )
{GUCEF_TRACE;

    m_elements.clear();
    m_error.Clear();
    CCursor cursor( bytes, size );
    SElement vertices, instances, edges, polygons, triangles, groups;
    vertices.name = "Vertices";
    instances.name = "VertexInstances";
    edges.name = "Edges";
    polygons.name = "Polygons";
    triangles.name = "Triangles";
    groups.name = "PolygonGroups";
    SChannel vertexChannel, instanceChannel, edgeChannel, polygonChannel, triangleChannel, groupChannel;
    UInt32 setCount = 0;
    bool ok = true;

    // vertices: the allocated ids only
    ok = ok && ReadUe4BitArray( cursor, "vertices", vertexChannel.allocated, setCount );
    vertexChannel.slotCount = (UInt32) vertexChannel.allocated.size();

    // vertex instances: the vertex of each
    SAttribute vertexIndex;
    if ( ok )
        ok = ReadUe4BitArray( cursor, "vertex instances", instanceChannel.allocated, setCount );
    if ( ok )
    {
        instanceChannel.slotCount = (UInt32) instanceChannel.allocated.size();
        if ( (UInt64) setCount * 4 > cursor.GetRemaining() )
            ok = cursor.Fail( "The vertex instances of the mesh description hold more vertices than the payload" );
        MakeIndexAttribute( "VertexIndex", instanceChannel.slotCount, 1, vertexIndex );
        for ( UInt32 id=0; ok && id<instanceChannel.slotCount; ++id )
            if ( instanceChannel.IsAllocated( id ) )
                SetIndexValue( vertexIndex, id, 0, cursor.ReadUInt32() );
    }

    // edges: the two vertices of each (not used by the readers of the meshes)
    if ( ok )
        ok = ReadUe4BitArray( cursor, "edges", edgeChannel.allocated, setCount ) && cursor.Skip( (UInt64) setCount * 8 );
    edgeChannel.slotCount = (UInt32) edgeChannel.allocated.size();

    // polygons: the perimeter (empty in the packages; its vertex instances are passed over) and the polygon group of each
    GUCEF::vector< Int32 > groupOfPolygon;
    if ( ok )
        ok = ReadUe4BitArray( cursor, "polygons", polygonChannel.allocated, setCount );
    polygonChannel.slotCount = (UInt32) polygonChannel.allocated.size();
    if ( ok )
    {
        groupOfPolygon.assign( polygonChannel.slotCount, -1 );
        for ( UInt32 id=0; ok && id<polygonChannel.slotCount; ++id )
        {
            if ( !polygonChannel.IsAllocated( id ) )
                continue;
            const UInt32 perimeter = cursor.ReadUInt32();
            ok = !cursor.HasError() && ( (UInt64) perimeter * 4 <= cursor.GetRemaining() || cursor.Fail( "A polygon of the mesh description has a perimeter longer than the payload" ) );
            ok = ok && cursor.Skip( (UInt64) perimeter * 4 );
            groupOfPolygon[ id ] = (Int32) cursor.ReadUInt32();
            ok = ok && !cursor.HasError();
        }
    }

    // polygon groups: the allocated ids only
    if ( ok )
        ok = ReadUe4BitArray( cursor, "polygon groups", groupChannel.allocated, setCount );
    groupChannel.slotCount = (UInt32) groupChannel.allocated.size();

    // the attribute sets
    ok = ok && ReadUe4AttributeSet( cursor, "vertices", vertexChannel );
    ok = ok && ReadUe4AttributeSet( cursor, "vertex instances", instanceChannel );
    ok = ok && ReadUe4AttributeSet( cursor, "edges", edgeChannel );
    ok = ok && ReadUe4AttributeSet( cursor, "polygons", polygonChannel );
    ok = ok && ReadUe4AttributeSet( cursor, "polygon groups", groupChannel );

    // triangles: three vertex instances and the polygon of each; the polygon gives the group
    if ( ok )
        ok = ReadUe4BitArray( cursor, "triangles", triangleChannel.allocated, setCount );
    if ( ok )
    {
        triangleChannel.slotCount = (UInt32) triangleChannel.allocated.size();
        if ( (UInt64) setCount * 16 > cursor.GetRemaining() )
            ok = cursor.Fail( "The triangles of the mesh description hold more corners than the payload" );
        SAttribute corners, group;
        MakeIndexAttribute( "VertexInstanceIndex", triangleChannel.slotCount, 3, corners );
        MakeIndexAttribute( "PolygonGroupIndex", triangleChannel.slotCount, 1, group );
        for ( UInt32 id=0; ok && id<triangleChannel.slotCount; ++id )
        {
            if ( !triangleChannel.IsAllocated( id ) )
                continue;
            for ( UInt32 c=0; c<3; ++c )
                SetIndexValue( corners, id, c, cursor.ReadUInt32() );
            const UInt32 polygon = cursor.ReadUInt32();
            if ( polygon >= polygonChannel.slotCount || !polygonChannel.IsAllocated( polygon ) )
                ok = cursor.Fail( "A triangle of the mesh description refers to a polygon that does not exist" );
            else
                SetIndexValue( group, id, 0, (UInt32) groupOfPolygon[ polygon ] );
        }
        triangleChannel.attributes.push_back( corners );
        triangleChannel.attributes.push_back( group );
    }
    ok = ok && ReadUe4AttributeSet( cursor, "triangles", triangleChannel );
    if ( ok && 0 != cursor.GetRemaining() )
        ok = cursor.Fail( "The mesh description has " + CORE::UInt64ToString( cursor.GetRemaining() ) + " bytes behind the end of its layout" );
    if ( !ok || cursor.HasError() )
    {
        m_error = cursor.GetError();
        return false;
    }

    instanceChannel.attributes.insert( instanceChannel.attributes.begin(), vertexIndex );
    vertices.channels.push_back( vertexChannel );
    instances.channels.push_back( instanceChannel );
    edges.channels.push_back( edgeChannel );
    polygons.channels.push_back( polygonChannel );
    triangles.channels.push_back( triangleChannel );
    groups.channels.push_back( groupChannel );
    m_elements.push_back( vertices );
    m_elements.push_back( instances );
    m_elements.push_back( edges );
    m_elements.push_back( polygons );
    m_elements.push_back( triangles );
    m_elements.push_back( groups );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
