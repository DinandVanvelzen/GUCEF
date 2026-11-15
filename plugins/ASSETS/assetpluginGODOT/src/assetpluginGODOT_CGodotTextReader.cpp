/*
 *  assetpluginGODOT: the reader of the Godot text formats
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

#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#include "assetpluginGODOT_CGodotTextReader.h"

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGodotTextReader::CGodotTextReader( void )
    : m_data( GUCEF_NULL )
    , m_size( 0 )
    , m_pos( 0 )
    , m_line( 1 )
    , m_problem()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGodotTextReader::~CGodotTextReader()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CGodotTextReader::GetProblem( void ) const
{GUCEF_TRACE;

    return m_problem;
}

/*-------------------------------------------------------------------------*/

void
CGodotTextReader::Start( const char* data, UInt32 size )
{GUCEF_TRACE;

    m_data = data;
    m_size = size;
    m_pos = 0;
    m_line = 1;
    m_problem.Clear();

    // a byte order mark
    if ( size >= 3 && (UInt8) data[ 0 ] == 0xEF && (UInt8) data[ 1 ] == 0xBB && (UInt8) data[ 2 ] == 0xBF )
        m_pos = 3;
}

/*-------------------------------------------------------------------------*/

bool
CGodotTextReader::Fail( const CORE::CString& message )
{GUCEF_TRACE;

    if ( m_problem.IsNULLOrEmpty() )
        m_problem = "line " + CORE::UInt32ToString( m_line ) + ": " + message;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CGodotTextReader::AtEnd( void ) const
{GUCEF_TRACE;

    return m_pos >= m_size;
}

/*-------------------------------------------------------------------------*/

char
CGodotTextReader::Peek( UInt32 ahead ) const
{GUCEF_TRACE;

    return m_pos + ahead < m_size ? m_data[ m_pos + ahead ] : '\0';
}

/*-------------------------------------------------------------------------*/

void
CGodotTextReader::SkipSpace( void )
{GUCEF_TRACE;

    while ( !AtEnd() )
    {
        const char c = m_data[ m_pos ];
        if ( '\n' == c )
        {
            ++m_line;
            ++m_pos;
        }
        else
        if ( ' ' == c || '\t' == c || '\r' == c )
        {
            ++m_pos;
        }
        else
        if ( ';' == c )
        {
            while ( !AtEnd() && '\n' != m_data[ m_pos ] )
                ++m_pos;
        }
        else
            break;
    }
}

/*-------------------------------------------------------------------------*/

bool
CGodotTextReader::IsIdentifierChar( char c ) const
{GUCEF_TRACE;

    return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || '_' == c;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CGodotTextReader::ReadIdentifier( void )
{GUCEF_TRACE;

    const UInt32 start = m_pos;
    while ( !AtEnd() && IsIdentifierChar( m_data[ m_pos ] ) )
        ++m_pos;
    return CORE::CString( m_data + start, m_pos - start );
}

/*-------------------------------------------------------------------------*/

bool
CGodotTextReader::Expect( char c )
{GUCEF_TRACE;

    SkipSpace();
    if ( Peek() != c )
        return Fail( CORE::CString( "expected '" ) + CORE::CString( &c, 1 ) + "'" );
    ++m_pos;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGodotTextReader::ParseString( CORE::CString& text )
{GUCEF_TRACE;

    // at the opening quote
    ++m_pos;
    text.Clear();
    CORE::CString result;
    UInt32 runStart = m_pos;
    while ( !AtEnd() )
    {
        const char c = m_data[ m_pos ];
        if ( '"' == c )
        {
            result.Append( m_data + runStart, m_pos - runStart );
            ++m_pos;
            text = result;
            return true;
        }
        if ( '\\' == c )
        {
            result.Append( m_data + runStart, m_pos - runStart );
            ++m_pos;
            if ( AtEnd() )
                break;
            const char escaped = m_data[ m_pos ];
            ++m_pos;
            switch ( escaped )
            {
                case 'n': result.Append( "\n", 1 ); break;
                case 't': result.Append( "\t", 1 ); break;
                case 'r': result.Append( "\r", 1 ); break;
                case 'b': result.Append( "\b", 1 ); break;
                case 'f': result.Append( "\f", 1 ); break;
                case 'a': result.Append( "\a", 1 ); break;
                case 'v': result.Append( "\v", 1 ); break;
                case 'u':
                case 'U':
                {
                    const UInt32 digits = 'u' == escaped ? 4 : 6;
                    UInt32 codePoint = 0;
                    for ( UInt32 d=0; d<digits && !AtEnd(); ++d )
                    {
                        const char h = m_data[ m_pos ];
                        UInt32 value = 0;
                        if ( h >= '0' && h <= '9' ) value = h - '0';
                        else if ( h >= 'a' && h <= 'f' ) value = 10 + h - 'a';
                        else if ( h >= 'A' && h <= 'F' ) value = 10 + h - 'A';
                        else break;
                        codePoint = ( codePoint << 4 ) | value;
                        ++m_pos;
                    }
                    char utf8[ 4 ];
                    UInt32 length = 0;
                    if ( codePoint < 0x80 ) { utf8[ 0 ] = (char) codePoint; length = 1; }
                    else if ( codePoint < 0x800 ) { utf8[ 0 ] = (char) ( 0xC0 | ( codePoint >> 6 ) ); utf8[ 1 ] = (char) ( 0x80 | ( codePoint & 0x3F ) ); length = 2; }
                    else if ( codePoint < 0x10000 ) { utf8[ 0 ] = (char) ( 0xE0 | ( codePoint >> 12 ) ); utf8[ 1 ] = (char) ( 0x80 | ( ( codePoint >> 6 ) & 0x3F ) ); utf8[ 2 ] = (char) ( 0x80 | ( codePoint & 0x3F ) ); length = 3; }
                    else { utf8[ 0 ] = (char) ( 0xF0 | ( codePoint >> 18 ) ); utf8[ 1 ] = (char) ( 0x80 | ( ( codePoint >> 12 ) & 0x3F ) ); utf8[ 2 ] = (char) ( 0x80 | ( ( codePoint >> 6 ) & 0x3F ) ); utf8[ 3 ] = (char) ( 0x80 | ( codePoint & 0x3F ) ); length = 4; }
                    result.Append( utf8, length );
                    break;
                }
                default:
                    result.Append( &escaped, 1 );
                    break;
            }
            runStart = m_pos;
            continue;
        }
        if ( '\n' == c )
            ++m_line;
        ++m_pos;
    }
    return Fail( "a string is not closed" );
}

/*-------------------------------------------------------------------------*/

bool
CGodotTextReader::ParseNumberValue( Float64& number, bool& isInteger, Int64& integer )
{GUCEF_TRACE;

    const UInt32 start = m_pos;
    bool negative = false;
    if ( '-' == Peek() || '+' == Peek() )
    {
        negative = '-' == Peek();
        ++m_pos;
    }
    isInteger = true;
    if ( '0' == Peek() && ( 'x' == Peek( 1 ) || 'X' == Peek( 1 ) ) )
    {
        m_pos += 2;
        UInt64 value = 0;
        while ( !AtEnd() )
        {
            const char h = m_data[ m_pos ];
            UInt32 digit = 0;
            if ( h >= '0' && h <= '9' ) digit = h - '0';
            else if ( h >= 'a' && h <= 'f' ) digit = 10 + h - 'a';
            else if ( h >= 'A' && h <= 'F' ) digit = 10 + h - 'A';
            else break;
            value = ( value << 4 ) | digit;
            ++m_pos;
        }
        integer = negative ? -(Int64) value : (Int64) value;
        number = (Float64) integer;
        return true;
    }

    UInt64 accumulated = 0;
    bool overflow = false;
    UInt32 digits = 0;
    while ( !AtEnd() && Peek() >= '0' && Peek() <= '9' )
    {
        const UInt64 next = accumulated * 10 + (UInt64) ( Peek() - '0' );
        if ( accumulated > 922337203685477580ULL || next < accumulated )
            overflow = true;
        accumulated = next;
        ++m_pos;
        ++digits;
    }
    if ( '.' == Peek() )
    {
        isInteger = false;
        ++m_pos;
        while ( !AtEnd() && Peek() >= '0' && Peek() <= '9' )
        {
            ++m_pos;
            ++digits;
        }
    }
    if ( 0 == digits )
        return Fail( "a number was expected" );
    if ( 'e' == Peek() || 'E' == Peek() )
    {
        isInteger = false;
        ++m_pos;
        if ( '-' == Peek() || '+' == Peek() )
            ++m_pos;
        while ( !AtEnd() && Peek() >= '0' && Peek() <= '9' )
            ++m_pos;
    }
    if ( isInteger && !overflow )
    {
        integer = negative ? -(Int64) accumulated : (Int64) accumulated;
        number = (Float64) integer;
        return true;
    }
    isInteger = false;
    const CORE::CString token( m_data + start, m_pos - start );
    number = strtod( token.C_String(), GUCEF_NULL );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGodotTextReader::ParseNumber( CORE::CDataNode& node )
{GUCEF_TRACE;

    // -inf is a number with a letter name
    if ( ( '-' == Peek() || '+' == Peek() ) && ( ( Peek( 1 ) >= 'a' && Peek( 1 ) <= 'z' ) ) )
    {
        const bool negative = '-' == Peek();
        ++m_pos;
        const CORE::CString name = ReadIdentifier();
        Float64 value = 0.0;
        if ( "inf" == name )
            value = negative ? -HUGE_VAL : HUGE_VAL;
        else
        if ( "nan" == name )
            value = HUGE_VAL - HUGE_VAL;
        else
            return Fail( "the name " + name + " is not a number" );
        node.SetAttribute( "t", "float" );
        node.SetValue( CORE::CVariant( value ) );
        return true;
    }

    Float64 number = 0.0;
    Int64 integer = 0;
    bool isInteger = false;
    if ( !ParseNumberValue( number, isInteger, integer ) )
        return false;
    if ( isInteger )
    {
        node.SetAttribute( "t", "int" );
        node.SetValue( CORE::CVariant( integer ) );
    }
    else
    {
        node.SetAttribute( "t", "float" );
        node.SetValue( CORE::CVariant( number ) );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGodotTextReader::ParseArray( CORE::CDataNode& node )
{GUCEF_TRACE;

    ++m_pos;
    node.SetAttribute( "t", "Array" );
    for ( ; ; )
    {
        SkipSpace();
        if ( AtEnd() )
            return Fail( "an array is not closed" );
        if ( ']' == Peek() )
        {
            ++m_pos;
            return true;
        }
        CORE::CDataNode* element = node.AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        if ( !ParseVariant( *element ) )
            return false;
        SkipSpace();
        if ( ',' == Peek() )
            ++m_pos;
        else
        if ( ']' != Peek() )
            return Fail( "',' or ']' was expected in an array" );
    }
}

/*-------------------------------------------------------------------------*/

bool
CGodotTextReader::ParseDictionary( CORE::CDataNode& node )
{GUCEF_TRACE;

    ++m_pos;
    node.SetAttribute( "t", "Dictionary" );
    for ( ; ; )
    {
        SkipSpace();
        if ( AtEnd() )
            return Fail( "a dictionary is not closed" );
        if ( '}' == Peek() )
        {
            ++m_pos;
            return true;
        }
        CORE::CDataNode key( "key", GUCEF_DATATYPE_OBJECT );
        if ( !ParseVariant( key ) )
            return false;
        if ( !Expect( ':' ) )
            return false;

        const CORE::CString keyType = key.GetAttributeValue( "t" ).AsString();
        CORE::CString keyText;
        if ( "String" == keyType || "StringName" == keyType || "int" == keyType || "float" == keyType || "bool" == keyType )
            keyText = key.GetValue().AsString();
        else
            keyText = keyType;
        CORE::CDataNode* entry = node.AddChild( keyText, GUCEF_DATATYPE_OBJECT );
        entry->SetAttribute( "key", keyText );
        entry->SetAttribute( "keyType", keyType );
        if ( !ParseVariant( *entry ) )
            return false;
        SkipSpace();
        if ( ',' == Peek() )
            ++m_pos;
        else
        if ( '}' != Peek() )
            return Fail( "',' or '}' was expected in a dictionary" );
    }
}

/*-------------------------------------------------------------------------*/

bool
CGodotTextReader::ParseObject( CORE::CDataNode& node )
{GUCEF_TRACE;

    // at the '(' of Object(Class, "property": value, ...)
    ++m_pos;
    SkipSpace();
    node.SetAttribute( "t", "Object" );
    node.SetAttribute( "class", ReadIdentifier() );
    for ( ; ; )
    {
        SkipSpace();
        if ( AtEnd() )
            return Fail( "an object is not closed" );
        if ( ')' == Peek() )
        {
            ++m_pos;
            return true;
        }
        if ( ',' == Peek() )
        {
            ++m_pos;
            continue;
        }
        if ( '"' != Peek() )
            return Fail( "a property name was expected in an object" );
        CORE::CString name;
        if ( !ParseString( name ) )
            return false;
        if ( !Expect( ':' ) )
            return false;
        CORE::CDataNode* property = node.AddChild( name, GUCEF_DATATYPE_OBJECT );
        if ( !ParseVariant( *property ) )
            return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CGodotTextReader::ParseTypeSpec( CORE::CString& spec )
{GUCEF_TRACE;

    // at the '[' of Array[int]( ... ): up to the matching ']'
    ++m_pos;
    const UInt32 start = m_pos;
    UInt32 depth = 1;
    while ( !AtEnd() )
    {
        const char c = m_data[ m_pos ];
        if ( '"' == c )
        {
            CORE::CString skipped;
            if ( !ParseString( skipped ) )
                return false;
            continue;
        }
        if ( '[' == c )
            ++depth;
        else
        if ( ']' == c )
        {
            --depth;
            if ( 0 == depth )
            {
                spec = CORE::CString( m_data + start, m_pos - start );
                ++m_pos;
                return true;
            }
        }
        ++m_pos;
    }
    return Fail( "a type is not closed" );
}

/*-------------------------------------------------------------------------*/

bool
CGodotTextReader::ParseTypedContainer( const CORE::CString& name, CORE::CDataNode& node )
{GUCEF_TRACE;

    CORE::CString spec;
    if ( !ParseTypeSpec( spec ) )
        return false;
    if ( !Expect( '(' ) )
        return false;
    SkipSpace();
    if ( !ParseVariant( node ) )
        return false;
    node.SetAttribute( "t", name );
    node.SetAttribute( "elementType", spec );
    return Expect( ')' );
}

/*-------------------------------------------------------------------------*/

void
CGodotTextReader::SetPacked( CORE::CDataNode& node, const CORE::CString& name, const GUCEF::vector< Float64 >& numbers )
{GUCEF_TRACE;

    UInt32 components = 1;
    const char* element = "f32";
    if ( "PackedVector2Array" == name ) components = 2;
    else if ( "PackedVector3Array" == name ) components = 3;
    else if ( "PackedVector4Array" == name || "PackedColorArray" == name ) components = 4;
    if ( "PackedInt32Array" == name ) element = "i32";
    else if ( "PackedInt64Array" == name ) element = "i64";
    else if ( "PackedFloat64Array" == name ) element = "f64";

    node.SetAttribute( "t", name );
    node.SetAttribute( "elem", element );
    node.SetAttribute( "components", components );
    node.SetAttribute( "count", (UInt32) ( numbers.size() / components ) );
    if ( numbers.empty() )
        return;

    if ( 0 == strcmp( element, "i32" ) )
    {
        GUCEF::vector< Int32 > values( numbers.size() );
        for ( size_t i=0; i<numbers.size(); ++i )
            values[ i ] = (Int32) numbers[ i ];
        node.SetValue( CORE::CVariant( &values[ 0 ], (UInt32) ( values.size() * sizeof( Int32 ) ) ) );
    }
    else
    if ( 0 == strcmp( element, "i64" ) )
    {
        GUCEF::vector< Int64 > values( numbers.size() );
        for ( size_t i=0; i<numbers.size(); ++i )
            values[ i ] = (Int64) numbers[ i ];
        node.SetValue( CORE::CVariant( &values[ 0 ], (UInt32) ( values.size() * sizeof( Int64 ) ) ) );
    }
    else
    if ( 0 == strcmp( element, "f64" ) )
    {
        node.SetValue( CORE::CVariant( &numbers[ 0 ], (UInt32) ( numbers.size() * sizeof( Float64 ) ) ) );
    }
    else
    {
        GUCEF::vector< Float32 > values( numbers.size() );
        for ( size_t i=0; i<numbers.size(); ++i )
            values[ i ] = (Float32) numbers[ i ];
        node.SetValue( CORE::CVariant( &values[ 0 ], (UInt32) ( values.size() * sizeof( Float32 ) ) ) );
    }
}

/*-------------------------------------------------------------------------*/

bool
CGodotTextReader::DecodeBase64( const CORE::CString& text, GUCEF::vector< UInt8 >& bytes )
{GUCEF_TRACE;

    UInt32 accumulator = 0;
    UInt32 bits = 0;
    const char* data = text.C_String();
    const UInt32 size = text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
    for ( UInt32 i=0; i<size; ++i )
    {
        const char c = data[ i ];
        Int32 value = -1;
        if ( c >= 'A' && c <= 'Z' ) value = c - 'A';
        else if ( c >= 'a' && c <= 'z' ) value = 26 + c - 'a';
        else if ( c >= '0' && c <= '9' ) value = 52 + c - '0';
        else if ( '+' == c || '-' == c ) value = 62;
        else if ( '/' == c || '_' == c ) value = 63;
        else if ( '=' == c ) break;
        if ( value < 0 )
            continue;
        accumulator = ( accumulator << 6 ) | (UInt32) value;
        bits += 6;
        if ( bits >= 8 )
        {
            bits -= 8;
            bytes.push_back( (UInt8) ( ( accumulator >> bits ) & 0xFF ) );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGodotTextReader::ParsePackedCall( const CORE::CString& name, CORE::CDataNode& node )
{GUCEF_TRACE;

    // at the '(' of PackedXArray( ... )
    ++m_pos;
    SkipSpace();
    node.SetAttribute( "t", name );

    if ( "PackedStringArray" == name )
    {
        for ( ; ; )
        {
            SkipSpace();
            if ( AtEnd() )
                return Fail( "an array is not closed" );
            if ( ')' == Peek() )
            {
                ++m_pos;
                return true;
            }
            if ( ',' == Peek() )
            {
                ++m_pos;
                continue;
            }
            if ( '"' != Peek() )
                return Fail( "a string was expected in a PackedStringArray" );
            CORE::CString text;
            if ( !ParseString( text ) )
                return false;
            CORE::CDataNode* element = node.AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            element->SetAttribute( "t", "String" );
            element->SetValue( CORE::CVariant( text ) );
        }
    }

    const bool isBytes = "PackedByteArray" == name;
    if ( isBytes && '"' == Peek() )
    {
        // the bytes as base64 text
        CORE::CString text;
        if ( !ParseString( text ) )
            return false;
        GUCEF::vector< UInt8 > bytes;
        DecodeBase64( text, bytes );
        node.SetAttribute( "elem", "u8" );
        node.SetAttribute( "components", (UInt32) 1 );
        node.SetAttribute( "count", (UInt32) bytes.size() );
        if ( !bytes.empty() )
            node.SetValue( CORE::CVariant( &bytes[ 0 ], (UInt32) bytes.size() ) );
        return Expect( ')' );
    }

    GUCEF::vector< UInt8 > bytes;
    GUCEF::vector< Float64 > numbers;
    for ( ; ; )
    {
        SkipSpace();
        if ( AtEnd() )
            return Fail( "an array is not closed" );
        if ( ')' == Peek() )
        {
            ++m_pos;
            break;
        }
        if ( ',' == Peek() )
        {
            ++m_pos;
            continue;
        }
        CORE::CDataNode scalar( "n", GUCEF_DATATYPE_OBJECT );
        if ( !ParseVariant( scalar ) )
            return false;
        const CORE::CString type = scalar.GetAttributeValue( "t" ).AsString();
        if ( "int" != type && "float" != type && "bool" != type )
            return Fail( "a number was expected in " + name );
        if ( isBytes )
            bytes.push_back( (UInt8) scalar.GetValue().AsInt64() );
        else
            numbers.push_back( scalar.GetValue().AsFloat64() );
    }

    if ( isBytes )
    {
        node.SetAttribute( "elem", "u8" );
        node.SetAttribute( "components", (UInt32) 1 );
        node.SetAttribute( "count", (UInt32) bytes.size() );
        if ( !bytes.empty() )
            node.SetValue( CORE::CVariant( &bytes[ 0 ], (UInt32) bytes.size() ) );
    }
    else
        SetPacked( node, name, numbers );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGodotTextReader::ParseCall( const CORE::CString& name, CORE::CDataNode& node )
{GUCEF_TRACE;

    // at the '(' of Name( arguments )
    ++m_pos;
    const bool isReference = "ExtResource" == name || "SubResource" == name;
    CORE::CDataNode arguments( "arguments", GUCEF_DATATYPE_OBJECT );
    CORE::CDataNode& target = isReference || "NodePath" == name ? arguments : node;
    for ( ; ; )
    {
        SkipSpace();
        if ( AtEnd() )
            return Fail( "the arguments of " + name + " are not closed" );
        if ( ')' == Peek() )
        {
            ++m_pos;
            break;
        }
        CORE::CDataNode* argument = target.AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        if ( !ParseVariant( *argument ) )
            return false;
        SkipSpace();
        if ( ',' == Peek() )
            ++m_pos;
        else
        if ( ')' != Peek() )
            return Fail( "',' or ')' was expected in the arguments of " + name );
    }

    node.SetAttribute( "t", name );
    if ( isReference || "NodePath" == name )
    {
        CORE::CDataNode* first = arguments.GetFirstChild();
        if ( GUCEF_NULL == first )
            return Fail( name + " needs an argument" );
        node.SetValue( CORE::CVariant( first->GetValue().AsString() ) );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGodotTextReader::ParseIdentifierValue( CORE::CDataNode& node )
{GUCEF_TRACE;

    const CORE::CString name = ReadIdentifier();
    if ( "true" == name || "false" == name )
    {
        node.SetAttribute( "t", "bool" );
        node.SetValue( CORE::CVariant( "true" == name ) );
        return true;
    }
    if ( "null" == name )
    {
        node.SetAttribute( "t", "null" );
        return true;
    }
    if ( "inf" == name || "nan" == name || "inf_neg" == name )
    {
        node.SetAttribute( "t", "float" );
        const Float64 value = "inf" == name ? HUGE_VAL : ( "inf_neg" == name ? -HUGE_VAL : HUGE_VAL - HUGE_VAL );
        node.SetValue( CORE::CVariant( value ) );
        return true;
    }

    // calls: the '(' follows, or the type of a typed container
    if ( '(' == Peek() )
    {
        if ( "Object" == name )
            return ParseObject( node );
        if ( name.StartsWith( "Packed" ) && name.EndsWith( "Array" ) )
            return ParsePackedCall( name, node );
        return ParseCall( name, node );
    }
    if ( '[' == Peek() && ( "Array" == name || "Dictionary" == name ) )
        return ParseTypedContainer( name, node );

    node.SetAttribute( "t", "Identifier" );
    node.SetValue( CORE::CVariant( name ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGodotTextReader::ParseVariant( CORE::CDataNode& node )
{GUCEF_TRACE;

    SkipSpace();
    if ( AtEnd() )
        return Fail( "a value was expected" );

    const char c = Peek();
    if ( '"' == c )
    {
        CORE::CString text;
        if ( !ParseString( text ) )
            return false;
        node.SetAttribute( "t", "String" );
        node.SetValue( CORE::CVariant( text ) );
        return true;
    }
    if ( ( '&' == c || '^' == c ) && '"' == Peek( 1 ) )
    {
        ++m_pos;
        CORE::CString text;
        if ( !ParseString( text ) )
            return false;
        node.SetAttribute( "t", '&' == c ? "StringName" : "NodePath" );
        node.SetValue( CORE::CVariant( text ) );
        return true;
    }
    if ( '[' == c )
        return ParseArray( node );
    if ( '{' == c )
        return ParseDictionary( node );
    if ( ( c >= '0' && c <= '9' ) || '-' == c || '+' == c || '.' == c )
        return ParseNumber( node );
    if ( IsIdentifierChar( c ) )
        return ParseIdentifierValue( node );
    return Fail( CORE::CString( "the character '" ) + CORE::CString( &c, 1 ) + "' cannot start a value" );
}

/*-------------------------------------------------------------------------*/

bool
CGodotTextReader::ParseKey( CORE::CString& key )
{GUCEF_TRACE;

    SkipSpace();
    if ( '"' == Peek() )
        return ParseString( key );
    const UInt32 start = m_pos;
    while ( !AtEnd() )
    {
        const char c = m_data[ m_pos ];
        if ( '=' == c || ' ' == c || '\t' == c || '\r' == c || '\n' == c || ']' == c )
            break;
        ++m_pos;
    }
    key = CORE::CString( m_data + start, m_pos - start );
    if ( key.IsNULLOrEmpty() )
        return Fail( "a name was expected" );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGodotTextReader::SetScalarAttribute( CORE::CDataNode& section, const CORE::CString& key, const CORE::CDataNode& value )
{GUCEF_TRACE;

    const CORE::CString type = value.GetAttributeValue( "t" ).AsString();
    if ( "String" == type || "StringName" == type || "NodePath" == type || "ExtResource" == type || "SubResource" == type ||
         "int" == type || "float" == type || "bool" == type )
        section.SetAttribute( key, value.GetValue() );
}

/*-------------------------------------------------------------------------*/

bool
CGodotTextReader::ParseAssignment( CORE::CDataNode& container, bool isHeader, CORE::CDataNode* section )
{GUCEF_TRACE;

    CORE::CString key;
    if ( !ParseKey( key ) )
        return false;
    if ( !Expect( '=' ) )
        return false;
    CORE::CDataNode* value = container.AddChild( key, GUCEF_DATATYPE_OBJECT );
    if ( !ParseVariant( *value ) )
        return false;
    if ( isHeader && GUCEF_NULL != section )
        SetScalarAttribute( *section, key, *value );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGodotTextReader::ParseSection( CORE::CDataNode& root, CORE::CDataNode*& current, CORE::CDataNode*& properties )
{GUCEF_TRACE;

    ++m_pos;
    SkipSpace();
    const CORE::CString tag = ReadIdentifier();
    if ( tag.IsNULLOrEmpty() )
        return Fail( "a section needs a name" );
    current = root.AddChild( "section", GUCEF_DATATYPE_OBJECT );
    current->SetAttribute( "tag", tag );
    CORE::CDataNode* header = current->AddChild( "header", GUCEF_DATATYPE_OBJECT );
    properties = current->AddChild( "properties", GUCEF_DATATYPE_OBJECT );
    for ( ; ; )
    {
        SkipSpace();
        if ( AtEnd() )
            return Fail( "the section " + tag + " is not closed" );
        if ( ']' == Peek() )
        {
            ++m_pos;
            return true;
        }
        if ( !ParseAssignment( *header, true, current ) )
            return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CGodotTextReader::ParseData( const char* data, UInt32 size, CORE::CDataNode& result )
{GUCEF_TRACE;

    Start( data, size );
    result.Clear();
    result.SetName( "godot" );
    result.SetNodeType( GUCEF_DATATYPE_OBJECT );
    result.SetAttribute( "kind", CORE::CString::Empty );

    CORE::CDataNode* current = GUCEF_NULL;
    CORE::CDataNode* properties = GUCEF_NULL;
    bool kindKnown = false;
    for ( ; ; )
    {
        SkipSpace();
        if ( AtEnd() )
            break;
        if ( '[' == Peek() )
        {
            if ( !ParseSection( result, current, properties ) )
                return false;
            if ( !kindKnown )
            {
                kindKnown = true;
                const CORE::CString tag = current->GetAttributeValue( "tag" ).AsString();
                if ( tag.StartsWith( "gd_" ) )
                    result.SetAttribute( "kind", tag );
            }
            continue;
        }
        if ( GUCEF_NULL == current )
        {
            current = result.AddChild( "section", GUCEF_DATATYPE_OBJECT );
            current->SetAttribute( "tag", CORE::CString::Empty );
            current->AddChild( "header", GUCEF_DATATYPE_OBJECT );
            properties = current->AddChild( "properties", GUCEF_DATATYPE_OBJECT );
            kindKnown = true;
        }
        if ( !ParseAssignment( *properties, false, GUCEF_NULL ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGodotTextReader::Parse( const CORE::CString& text, CORE::CDataNode& result )
{GUCEF_TRACE;

    return ParseData( text.C_String(), text.ByteSize() > 0 ? text.ByteSize() - 1 : 0, result );
}

/*-------------------------------------------------------------------------*/

bool
CGodotTextReader::ParseFile( const CORE::CString& vfsPath, CORE::CDataNode& result )
{GUCEF_TRACE;

    CORE::CDynamicBuffer file;
    if ( !VFS::CVfsGlobal::Instance()->GetVfs().LoadFile( file, vfsPath, "rb" ) )
    {
        m_problem = "the file " + vfsPath + " cannot be read";
        return false;
    }
    return ParseData( static_cast< const char* >( file.GetConstBufferPtr() ), file.GetDataSize(), result );
}

/*-------------------------------------------------------------------------*/

bool
CGodotTextReader::ParseValue( const CORE::CString& text, CORE::CDataNode& value )
{GUCEF_TRACE;

    Start( text.C_String(), text.ByteSize() > 0 ? text.ByteSize() - 1 : 0 );
    value.Clear();
    if ( !ParseVariant( value ) )
        return false;
    SkipSpace();
    if ( !AtEnd() )
        return Fail( "there is text after the value" );
    return true;
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
