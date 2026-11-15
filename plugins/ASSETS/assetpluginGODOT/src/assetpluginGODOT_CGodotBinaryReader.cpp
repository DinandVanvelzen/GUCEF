/*
 *  assetpluginGODOT: the reader of the binary resource format of Godot
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

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#include "assetpluginGODOT_CGodotBinaryReader.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace GODOT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static const UInt32 FlagUids = 2;
static const UInt32 FlagScriptClass = 8;

// the types of the values in the file
static const UInt32 VariantNil = 1;
static const UInt32 VariantBool = 2;
static const UInt32 VariantInt = 3;
static const UInt32 VariantFloat = 4;
static const UInt32 VariantString = 5;
static const UInt32 VariantVector2 = 10;
static const UInt32 VariantRect2 = 11;
static const UInt32 VariantVector3 = 12;
static const UInt32 VariantPlane = 13;
static const UInt32 VariantQuaternion = 14;
static const UInt32 VariantAabb = 15;
static const UInt32 VariantBasis = 16;
static const UInt32 VariantTransform3D = 17;
static const UInt32 VariantTransform2D = 18;
static const UInt32 VariantColour = 20;
static const UInt32 VariantNodePath = 22;
static const UInt32 VariantObject = 24;
static const UInt32 VariantDictionary = 26;
static const UInt32 VariantArray = 30;
static const UInt32 VariantPackedBytes = 31;
static const UInt32 VariantPackedInt32 = 32;
static const UInt32 VariantPackedFloat32 = 33;
static const UInt32 VariantPackedStrings = 34;
static const UInt32 VariantPackedVector3 = 35;
static const UInt32 VariantPackedColours = 36;
static const UInt32 VariantPackedVector2 = 37;
static const UInt32 VariantInt64 = 40;
static const UInt32 VariantDouble = 41;
static const UInt32 VariantStringName = 44;
static const UInt32 VariantVector2i = 45;
static const UInt32 VariantRect2i = 46;
static const UInt32 VariantVector3i = 47;
static const UInt32 VariantPackedInt64 = 48;
static const UInt32 VariantPackedFloat64 = 49;
static const UInt32 VariantVector4 = 50;
static const UInt32 VariantVector4i = 51;
static const UInt32 VariantProjection = 52;
static const UInt32 VariantPackedVector4 = 53;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGodotBinaryReader::CGodotBinaryReader( void )
    : m_data( GUCEF_NULL )
    , m_size( 0 )
    , m_pos( 0 )
    , m_real64( false )
    , m_strings()
    , m_internalIds()
    , m_problem()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGodotBinaryReader::~CGodotBinaryReader()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CGodotBinaryReader::GetProblem( void ) const
{GUCEF_TRACE;

    return m_problem;
}

/*-------------------------------------------------------------------------*/

bool
CGodotBinaryReader::IsBinaryResource( const void* data, UInt32 size )
{GUCEF_TRACE;

    return size >= 4 && ( 0 == memcmp( data, "RSRC", 4 ) || 0 == memcmp( data, "RSCC", 4 ) );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CGodotBinaryReader::UidToText( UInt64 uid )
{GUCEF_TRACE;

    // base 34: the letters a to y, then the digits 0 to 8
    if ( 0 == uid )
        return CORE::CString::Empty;
    char digits[ 32 ];
    UInt32 count = 0;
    while ( uid > 0 && count < sizeof( digits ) )
    {
        const UInt32 value = (UInt32) ( uid % 34 );
        digits[ count++ ] = (char) ( value < 25 ? 'a' + value : '0' + ( value - 25 ) );
        uid /= 34;
    }
    CORE::CString text = "uid://";
    while ( count > 0 )
        text.Append( &digits[ --count ], 1 );
    return text;
}

/*-------------------------------------------------------------------------*/

bool
CGodotBinaryReader::Fail( const CORE::CString& message )
{GUCEF_TRACE;

    if ( m_problem.IsNULLOrEmpty() )
        m_problem = "binary resource at byte " + CORE::UInt32ToString( m_pos ) + ": " + message;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CGodotBinaryReader::Need( UInt32 bytes )
{GUCEF_TRACE;

    if ( bytes > m_size || m_pos > m_size - bytes )
        return Fail( "the file ends inside a value" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGodotBinaryReader::ReadU32( UInt32& value )
{GUCEF_TRACE;

    if ( !Need( 4 ) )
        return false;
    memcpy( &value, m_data + m_pos, 4 );
    m_pos += 4;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGodotBinaryReader::ReadU64( UInt64& value )
{GUCEF_TRACE;

    if ( !Need( 8 ) )
        return false;
    memcpy( &value, m_data + m_pos, 8 );
    m_pos += 8;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGodotBinaryReader::ReadReal( Float64& value )
{GUCEF_TRACE;

    if ( m_real64 )
    {
        if ( !Need( 8 ) )
            return false;
        memcpy( &value, m_data + m_pos, 8 );
        m_pos += 8;
        return true;
    }
    if ( !Need( 4 ) )
        return false;
    Float32 narrow;
    memcpy( &narrow, m_data + m_pos, 4 );
    m_pos += 4;
    value = narrow;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGodotBinaryReader::ReadString( CORE::CString& text )
{GUCEF_TRACE;

    UInt32 length = 0;
    if ( !ReadU32( length ) || !Need( length ) )
        return false;
    // the length counts the terminator
    UInt32 characters = length;
    while ( characters > 0 && 0 == m_data[ m_pos + characters - 1 ] )
        --characters;
    text = CORE::CString( reinterpret_cast< const char* >( m_data + m_pos ), characters );
    m_pos += length;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGodotBinaryReader::ReadHeader( const void* data, UInt32 size, CORE::CString& type, CORE::CString& uid )
{GUCEF_TRACE;

    type.Clear();
    uid.Clear();
    if ( size < 4 || 0 != memcmp( data, "RSRC", 4 ) )
        return false;
    CGodotBinaryReader reader;
    reader.m_data = static_cast< const UInt8* >( data );
    reader.m_size = size;
    reader.m_pos = 4;
    UInt32 bigEndian = 0, real64 = 0, major = 0, minor = 0, format = 0, flags = 0;
    UInt64 importOffset = 0;
    if ( !reader.ReadU32( bigEndian ) || !reader.ReadU32( real64 ) || !reader.ReadU32( major ) || !reader.ReadU32( minor ) || !reader.ReadU32( format ) ||
         !reader.ReadString( type ) || !reader.ReadU64( importOffset ) || !reader.ReadU32( flags ) )
        return false;
    UInt64 uidValue = 0;
    if ( 0 != ( flags & FlagUids ) && reader.ReadU64( uidValue ) )
        uid = UidToText( uidValue );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGodotBinaryReader::ReadNumbers( CORE::CDataNode& node, const char* type, UInt32 count, bool integers )
{GUCEF_TRACE;

    node.SetAttribute( "t", type );
    for ( UInt32 i=0; i<count; ++i )
    {
        CORE::CDataNode* number = node.AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        if ( integers )
        {
            UInt32 raw = 0;
            if ( !ReadU32( raw ) )
                return false;
            number->SetAttribute( "t", "int" );
            number->SetValue( CORE::CVariant( (Int64) (Int32) raw ) );
        }
        else
        {
            Float64 value = 0.0;
            if ( !ReadReal( value ) )
                return false;
            number->SetAttribute( "t", "float" );
            number->SetValue( CORE::CVariant( value ) );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGodotBinaryReader::ReadPacked( CORE::CDataNode& node, const char* type, const char* element, UInt32 components, UInt32 elementSize, bool pad )
{GUCEF_TRACE;

    UInt32 count = 0;
    if ( !ReadU32( count ) )
        return false;
    const UInt64 total = (UInt64) count * components * elementSize;
    if ( total > m_size )
        return Fail( "an array is longer than the file" );
    const UInt32 bytes = (UInt32) total;
    if ( !Need( bytes ) )
        return false;
    node.SetAttribute( "t", type );
    node.SetAttribute( "elem", element );
    node.SetAttribute( "components", components );
    node.SetAttribute( "count", count );
    if ( bytes > 0 )
        node.SetValue( CORE::CVariant( m_data + m_pos, bytes ) );
    m_pos += bytes;
    if ( pad )
        m_pos += ( 4 - bytes % 4 ) % 4;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGodotBinaryReader::ReadVariant( CORE::CDataNode& node )
{GUCEF_TRACE;

    UInt32 type = 0;
    if ( !ReadU32( type ) )
        return false;
    switch ( type )
    {
        case VariantNil:
            node.SetAttribute( "t", "null" );
            return true;
        case VariantBool:
        {
            UInt32 value = 0;
            if ( !ReadU32( value ) )
                return false;
            node.SetAttribute( "t", "bool" );
            node.SetValue( CORE::CVariant( 0 != value ) );
            return true;
        }
        case VariantInt:
        {
            UInt32 value = 0;
            if ( !ReadU32( value ) )
                return false;
            node.SetAttribute( "t", "int" );
            node.SetValue( CORE::CVariant( (Int64) (Int32) value ) );
            return true;
        }
        case VariantInt64:
        {
            UInt64 value = 0;
            if ( !ReadU64( value ) )
                return false;
            node.SetAttribute( "t", "int" );
            node.SetValue( CORE::CVariant( (Int64) value ) );
            return true;
        }
        case VariantFloat:
        {
            Float64 value = 0.0;
            if ( !ReadReal( value ) )
                return false;
            node.SetAttribute( "t", "float" );
            node.SetValue( CORE::CVariant( value ) );
            return true;
        }
        case VariantDouble:
        {
            UInt64 raw = 0;
            if ( !ReadU64( raw ) )
                return false;
            Float64 value;
            memcpy( &value, &raw, 8 );
            node.SetAttribute( "t", "float" );
            node.SetValue( CORE::CVariant( value ) );
            return true;
        }
        case VariantString:
        case VariantStringName:
        {
            CORE::CString text;
            if ( !ReadString( text ) )
                return false;
            node.SetAttribute( "t", VariantString == type ? "String" : "StringName" );
            node.SetValue( CORE::CVariant( text ) );
            return true;
        }
        case VariantVector2:    return ReadNumbers( node, "Vector2", 2, false );
        case VariantRect2:      return ReadNumbers( node, "Rect2", 4, false );
        case VariantVector3:    return ReadNumbers( node, "Vector3", 3, false );
        case VariantPlane:      return ReadNumbers( node, "Plane", 4, false );
        case VariantQuaternion: return ReadNumbers( node, "Quaternion", 4, false );
        case VariantAabb:       return ReadNumbers( node, "AABB", 6, false );
        case VariantBasis:      return ReadNumbers( node, "Basis", 9, false );
        case VariantTransform3D:return ReadNumbers( node, "Transform3D", 12, false );
        case VariantTransform2D:return ReadNumbers( node, "Transform2D", 6, false );
        case VariantVector4:    return ReadNumbers( node, "Vector4", 4, false );
        case VariantProjection: return ReadNumbers( node, "Projection", 16, false );
        case VariantVector2i:   return ReadNumbers( node, "Vector2i", 2, true );
        case VariantRect2i:     return ReadNumbers( node, "Rect2i", 4, true );
        case VariantVector3i:   return ReadNumbers( node, "Vector3i", 3, true );
        case VariantVector4i:   return ReadNumbers( node, "Vector4i", 4, true );
        case VariantColour:
        {
            // a colour is four 32 bit floats whatever the size of the reals
            node.SetAttribute( "t", "Color" );
            for ( UInt32 c=0; c<4; ++c )
            {
                if ( !Need( 4 ) )
                    return false;
                Float32 value;
                memcpy( &value, m_data + m_pos, 4 );
                m_pos += 4;
                CORE::CDataNode* component = node.AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
                component->SetAttribute( "t", "float" );
                component->SetValue( CORE::CVariant( (Float64) value ) );
            }
            return true;
        }
        case VariantNodePath:
        {
            UInt32 names = 0, subnames = 0, flags = 0;
            if ( !Need( 8 ) )
                return false;
            names = (UInt32) m_data[ m_pos ] | ( (UInt32) m_data[ m_pos + 1 ] << 8 );
            subnames = (UInt32) m_data[ m_pos + 2 ] | ( (UInt32) m_data[ m_pos + 3 ] << 8 );
            m_pos += 4;
            if ( !ReadU32( flags ) )
                return false;
            if ( 0 != ( flags & 2 ) )
                ++subnames;
            CORE::CString path = 0 != ( flags & 1 ) ? "/" : "";
            for ( UInt32 n=0; n<names+subnames; ++n )
            {
                UInt32 index = 0;
                if ( !ReadU32( index ) )
                    return false;
                if ( index >= m_strings.size() )
                    return Fail( "a node path names a string that is not in the table" );
                if ( n == names )
                    path += ":";
                else
                if ( n > 0 )
                    path += n < names ? "/" : ":";
                path += m_strings[ index ];
            }
            node.SetAttribute( "t", "NodePath" );
            node.SetValue( CORE::CVariant( path ) );
            return true;
        }
        case VariantObject:
        {
            UInt32 kind = 0;
            if ( !ReadU32( kind ) )
                return false;
            if ( 0 == kind )
            {
                node.SetAttribute( "t", "null" );
                return true;
            }
            UInt32 index = 0;
            if ( ( 2 != kind && 3 != kind ) || !ReadU32( index ) )
                return Fail( "an object value that is not a reference to a resource" );
            if ( 2 == kind )
            {
                if ( index >= m_internalIds.size() )
                    return Fail( "a reference to an internal resource that the file does not have" );
                node.SetAttribute( "t", "SubResource" );
                node.SetValue( CORE::CVariant( m_internalIds[ index ] ) );
            }
            else
            {
                node.SetAttribute( "t", "ExtResource" );
                node.SetValue( CORE::CVariant( CORE::UInt32ToString( index ) ) );
            }
            return true;
        }
        case VariantDictionary:
        {
            UInt32 count = 0;
            if ( !ReadU32( count ) )
                return false;
            count &= 0x7FFFFFFF;
            node.SetAttribute( "t", "Dictionary" );
            for ( UInt32 i=0; i<count; ++i )
            {
                CORE::CDataNode key( "key", GUCEF_DATATYPE_OBJECT );
                if ( !ReadVariant( key ) )
                    return false;
                const CORE::CString keyType = key.GetAttributeValue( "t" ).AsString();
                CORE::CString keyText = keyType;
                if ( "String" == keyType || "StringName" == keyType || "int" == keyType || "float" == keyType || "bool" == keyType )
                    keyText = key.GetValue().AsString();
                CORE::CDataNode* entry = node.AddChild( keyText, GUCEF_DATATYPE_OBJECT );
                entry->SetAttribute( "key", keyText );
                entry->SetAttribute( "keyType", keyType );
                if ( !ReadVariant( *entry ) )
                    return false;
            }
            return true;
        }
        case VariantArray:
        {
            UInt32 count = 0;
            if ( !ReadU32( count ) )
                return false;
            count &= 0x7FFFFFFF;
            if ( count > m_size )
                return Fail( "an array is longer than the file" );
            node.SetAttribute( "t", "Array" );
            for ( UInt32 i=0; i<count; ++i )
            {
                CORE::CDataNode* element = node.AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
                if ( !ReadVariant( *element ) )
                    return false;
            }
            return true;
        }
        case VariantPackedBytes:    return ReadPacked( node, "PackedByteArray", "u8", 1, 1, true );
        case VariantPackedInt32:    return ReadPacked( node, "PackedInt32Array", "i32", 1, 4, false );
        case VariantPackedInt64:    return ReadPacked( node, "PackedInt64Array", "i64", 1, 8, false );
        case VariantPackedFloat64:  return ReadPacked( node, "PackedFloat64Array", "f64", 1, 8, false );
        case VariantPackedColours:  return ReadPacked( node, "PackedColorArray", "f32", 4, 4, false );
        case VariantPackedFloat32:  return ReadPacked( node, "PackedFloat32Array", "f32", 1, 4, false );
        case VariantPackedVector2:  return ReadPacked( node, "PackedVector2Array", "f32", 2, m_real64 ? 8 : 4, false ) && ( !m_real64 || Fail( "packed vectors of 64 bit reals" ) );
        case VariantPackedVector3:  return ReadPacked( node, "PackedVector3Array", "f32", 3, m_real64 ? 8 : 4, false ) && ( !m_real64 || Fail( "packed vectors of 64 bit reals" ) );
        case VariantPackedVector4:  return ReadPacked( node, "PackedVector4Array", "f32", 4, m_real64 ? 8 : 4, false ) && ( !m_real64 || Fail( "packed vectors of 64 bit reals" ) );
        case VariantPackedStrings:
        {
            UInt32 count = 0;
            if ( !ReadU32( count ) )
                return false;
            if ( count > m_size )
                return Fail( "an array is longer than the file" );
            node.SetAttribute( "t", "PackedStringArray" );
            for ( UInt32 i=0; i<count; ++i )
            {
                CORE::CString text;
                if ( !ReadString( text ) )
                    return false;
                CORE::CDataNode* element = node.AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
                element->SetAttribute( "t", "String" );
                element->SetValue( CORE::CVariant( text ) );
            }
            return true;
        }
        default:
            return Fail( "the value type " + CORE::UInt32ToString( type ) + " is not supported" );
    }
}

/*-------------------------------------------------------------------------*/

bool
CGodotBinaryReader::ReadProperties( CORE::CDataNode& properties, CORE::CString& type )
{GUCEF_TRACE;

    UInt32 count = 0;
    if ( !ReadString( type ) || !ReadU32( count ) )
        return false;
    for ( UInt32 p=0; p<count; ++p )
    {
        UInt32 nameIndex = 0;
        if ( !ReadU32( nameIndex ) )
            return false;
        if ( nameIndex >= m_strings.size() )
            return Fail( "a property has a name that is not in the string table" );
        CORE::CDataNode* property = properties.AddChild( m_strings[ nameIndex ], GUCEF_DATATYPE_OBJECT );
        if ( !ReadVariant( *property ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGodotBinaryReader::Parse( const void* data, UInt32 size, CORE::CDataNode& result )
{GUCEF_TRACE;

    m_data = static_cast< const UInt8* >( data );
    m_size = size;
    m_pos = 0;
    m_problem.Clear();
    m_strings.clear();
    m_internalIds.clear();
    result.Clear();
    result.SetName( "godot" );
    result.SetNodeType( GUCEF_DATATYPE_OBJECT );
    result.SetAttribute( "kind", "gd_resource" );

    if ( !IsBinaryResource( data, size ) )
        return Fail( "the file is not a binary resource" );
    if ( 0 == memcmp( data, "RSCC", 4 ) )
        return Fail( "the resource is compressed (Godot saves a resource with zstd blocks when it is asked to compress it): this reader has no decompressor" );
    m_pos = 4;
    UInt32 bigEndian = 0, real64 = 0, major = 0, minor = 0, format = 0, flags = 0;
    CORE::CString mainType;
    UInt64 importOffset = 0;
    if ( !ReadU32( bigEndian ) || !ReadU32( real64 ) || !ReadU32( major ) || !ReadU32( minor ) || !ReadU32( format ) ||
         !ReadString( mainType ) || !ReadU64( importOffset ) || !ReadU32( flags ) )
        return false;
    if ( 0 != bigEndian )
        return Fail( "the file is in big endian byte order" );
    if ( major < 4 || format < 3 )
        return Fail( "the file is a resource of Godot " + CORE::UInt32ToString( major ) + ", only Godot 4 is read" );
    m_real64 = 0 != real64;
    UInt64 uid = 0;
    if ( 0 != ( flags & FlagUids ) && !ReadU64( uid ) )
        return false;
    CORE::CString scriptClass;
    if ( 0 != ( flags & FlagScriptClass ) && !ReadString( scriptClass ) )
        return false;
    for ( UInt32 r=0; r<11; ++r )
    {
        UInt32 reserved = 0;
        if ( !ReadU32( reserved ) )
            return false;
    }

    UInt32 count = 0;
    if ( !ReadU32( count ) || count > m_size )
        return Fail( "the string table is not valid" );
    for ( UInt32 s=0; s<count; ++s )
    {
        CORE::CString text;
        if ( !ReadString( text ) )
            return false;
        m_strings.push_back( text );
    }

    // the header section and the external resources
    CORE::CDataNode* header = result.AddChild( "section", GUCEF_DATATYPE_OBJECT );
    header->SetAttribute( "tag", "gd_resource" );
    header->SetAttribute( "type", mainType );
    header->SetAttribute( "format", (Int32) 3 );
    if ( 0 != uid )
        header->SetAttribute( "uid", UidToText( uid ) );
    header->AddChild( "header", GUCEF_DATATYPE_OBJECT );
    header->AddChild( "properties", GUCEF_DATATYPE_OBJECT );

    if ( !ReadU32( count ) || count > m_size )
        return Fail( "the table of external resources is not valid" );
    for ( UInt32 e=0; e<count; ++e )
    {
        CORE::CString type;
        CORE::CString path;
        UInt64 externalUid = 0;
        if ( !ReadString( type ) || !ReadString( path ) || ( 0 != ( flags & FlagUids ) && !ReadU64( externalUid ) ) )
            return false;
        CORE::CDataNode* section = result.AddChild( "section", GUCEF_DATATYPE_OBJECT );
        section->SetAttribute( "tag", "ext_resource" );
        section->SetAttribute( "type", type );
        section->SetAttribute( "path", path );
        if ( 0 != externalUid && externalUid != 0xFFFFFFFFFFFFFFFFULL )
            section->SetAttribute( "uid", UidToText( externalUid ) );
        section->SetAttribute( "id", CORE::UInt32ToString( e ) );
        section->AddChild( "header", GUCEF_DATATYPE_OBJECT );
        section->AddChild( "properties", GUCEF_DATATYPE_OBJECT );
    }

    // the internal resources: the ids first, as the values refer to them by place
    if ( !ReadU32( count ) || count == 0 || count > m_size )
        return Fail( "the table of internal resources is not valid" );
    GUCEF::vector< UInt64 > offsets;
    for ( UInt32 i=0; i<count; ++i )
    {
        CORE::CString path;
        UInt64 offset = 0;
        if ( !ReadString( path ) || !ReadU64( offset ) )
            return false;
        if ( path.StartsWith( "local://" ) )
            path = path.SubstrFromRange( 8, path.Length() );
        m_internalIds.push_back( path );
        offsets.push_back( offset );
    }
    for ( UInt32 i=0; i<count; ++i )
    {
        if ( offsets[ i ] >= m_size )
            return Fail( "an internal resource starts outside the file" );
        m_pos = (UInt32) offsets[ i ];
        const bool isMain = i + 1 == count;
        CORE::CDataNode* section = result.AddChild( "section", GUCEF_DATATYPE_OBJECT );
        section->SetAttribute( "tag", isMain ? "resource" : "sub_resource" );
        if ( !isMain )
            section->SetAttribute( "id", m_internalIds[ i ] );
        section->AddChild( "header", GUCEF_DATATYPE_OBJECT );
        CORE::CDataNode* properties = section->AddChild( "properties", GUCEF_DATATYPE_OBJECT );
        CORE::CString type;
        if ( !ReadProperties( *properties, type ) )
            return false;
        if ( !isMain )
            section->SetAttribute( "type", type );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGodotBinaryReader::ParseFile( const CORE::CString& vfsPath, CORE::CDataNode& result )
{GUCEF_TRACE;

    CORE::CDynamicBuffer file;
    if ( !VFS::CVfsGlobal::Instance()->GetVfs().LoadFile( file, vfsPath, "rb" ) )
    {
        m_problem = "the file " + vfsPath + " cannot be read";
        return false;
    }
    return Parse( file.GetConstBufferPtr(), file.GetDataSize(), result );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GODOT */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
