/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_COM_CMIMECODEC_H
#include "gucefCOM_CMimeCodec.h"
#define GUCEF_COM_CMIMECODEC_H
#endif /* GUCEF_COM_CMIMECODEC_H ? */

#include "gucefCOM_CImapValue.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CImapValue::CImapValue( void )
    : m_type( TYPE_NIL )
    , m_text()
    , m_bytes()
    , m_number( 0 )
    , m_elements()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImapValue::CImapValue( const CImapValue& src )
    : m_type( TYPE_NIL )
    , m_text()
    , m_bytes()
    , m_number( 0 )
    , m_elements()
{GUCEF_TRACE;

    CopyFrom( src );
}

/*-------------------------------------------------------------------------*/

CImapValue::~CImapValue()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

CImapValue&
CImapValue::operator=( const CImapValue& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        Clear();
        CopyFrom( src );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CImapValue::CopyFrom( const CImapValue& src )
{GUCEF_TRACE;

    m_type = src.m_type;
    m_text = src.m_text;
    m_bytes = src.m_bytes;
    m_number = src.m_number;
    for ( size_t i=0; i<src.m_elements.size(); ++i )
        m_elements.push_back( GUCEF_NEW CImapValue( *src.m_elements[ i ] ) );
}

/*-------------------------------------------------------------------------*/

void
CImapValue::Clear( void )
{GUCEF_TRACE;

    // an iterative release would be needed for lists nested without a limit: the parser limits the nesting
    for ( size_t i=0; i<m_elements.size(); ++i )
        GUCEF_DELETE m_elements[ i ];
    m_elements.clear();
    m_type = TYPE_NIL;
    m_text.Clear();
    m_bytes.Clear();
    m_number = 0;
}

/*-------------------------------------------------------------------------*/

CImapValue::EType CImapValue::GetType( void ) const { return m_type; }
bool CImapValue::IsNil( void ) const { return TYPE_NIL == m_type; }
bool CImapValue::IsAtom( void ) const { return TYPE_ATOM == m_type; }
bool CImapValue::IsNumber( void ) const { return TYPE_NUMBER == m_type; }
bool CImapValue::IsString( void ) const { return TYPE_STRING == m_type; }
bool CImapValue::IsList( void ) const { return TYPE_LIST == m_type; }
bool CImapValue::IsText( void ) const { return TYPE_ATOM == m_type || TYPE_STRING == m_type; }

/*-------------------------------------------------------------------------*/

CORE::CString
CImapValue::GetString( void ) const
{GUCEF_TRACE;

    switch ( m_type )
    {
        case TYPE_ATOM:
        case TYPE_NUMBER:
            return m_text;
        case TYPE_STRING:
            return m_bytes.GetDataSize() > 0 ? CMimeCodec::SanitizeUtf8( static_cast< const char* >( m_bytes.GetConstBufferPtr() ), m_bytes.GetDataSize() ) : CORE::CString();
        default:
            return CORE::CString();
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CDynamicBuffer&
CImapValue::GetBytes( void ) const
{GUCEF_TRACE;

    return m_bytes;
}

/*-------------------------------------------------------------------------*/

UInt64
CImapValue::GetNumber( void ) const
{GUCEF_TRACE;

    return m_number;
}

/*-------------------------------------------------------------------------*/

bool
CImapValue::AtomEquals( const char* text ) const
{GUCEF_TRACE;

    return TYPE_ATOM == m_type && m_text.Equals( text, false );
}

/*-------------------------------------------------------------------------*/

UInt32
CImapValue::GetListSize( void ) const
{GUCEF_TRACE;

    return (UInt32) m_elements.size();
}

/*-------------------------------------------------------------------------*/

const CImapValue&
CImapValue::GetElement( UInt32 index ) const
{GUCEF_TRACE;

    static const CImapValue nil;
    return index < m_elements.size() ? *m_elements[ index ] : nil;
}

/*-------------------------------------------------------------------------*/

void CImapValue::SetNil( void ) { Clear(); }

void
CImapValue::SetAtom( const CORE::CString& text )
{GUCEF_TRACE;

    Clear();
    m_type = TYPE_ATOM;
    m_text = text;
}

void
CImapValue::SetNumber( UInt64 number )
{GUCEF_TRACE;

    Clear();
    m_type = TYPE_NUMBER;
    m_number = number;
    m_text = CORE::UInt64ToString( number );
}

void
CImapValue::SetString( const void* data, UInt32 size )
{GUCEF_TRACE;

    Clear();
    m_type = TYPE_STRING;
    if ( size > 0 )
        m_bytes.CopyFrom( size, data );
}

void
CImapValue::SetList( void )
{GUCEF_TRACE;

    Clear();
    m_type = TYPE_LIST;
}

/*-------------------------------------------------------------------------*/

CImapValue*
CImapValue::AddElement( void )
{GUCEF_TRACE;

    CImapValue* element = GUCEF_NEW CImapValue();
    m_elements.push_back( element );
    return element;
}

/*-------------------------------------------------------------------------*/

CImapValueParser::CImapValueParser( const void* data, UInt32 size, UInt32 maxDepth, UInt32 maxValues )
    : m_data( static_cast< const char* >( data ) )
    , m_size( size )
    , m_position( 0 )
    , m_maxDepth( maxDepth )
    , m_maxValues( maxValues )
    , m_valueCount( 0 )
    , m_error()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CImapValueParser::Fail( const char* reason )
{GUCEF_TRACE;

    if ( m_error.IsNULLOrEmpty() )
        m_error = reason;
    return false;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CImapValueParser::GetError( void ) const
{GUCEF_TRACE;

    return m_error;
}

/*-------------------------------------------------------------------------*/

void
CImapValueParser::SkipSpaces( void )
{GUCEF_TRACE;

    while ( m_position < m_size && ' ' == m_data[ m_position ] )
        ++m_position;
}

/*-------------------------------------------------------------------------*/

bool
CImapValueParser::AtEnd( void ) const
{GUCEF_TRACE;

    return m_position >= m_size;
}

/*-------------------------------------------------------------------------*/

UInt32
CImapValueParser::GetPosition( void ) const
{GUCEF_TRACE;

    return m_position;
}

/*-------------------------------------------------------------------------*/

char
CImapValueParser::Peek( void ) const
{GUCEF_TRACE;

    return m_position < m_size ? m_data[ m_position ] : '\0';
}

/*-------------------------------------------------------------------------*/

void
CImapValueParser::Advance( void )
{GUCEF_TRACE;

    if ( m_position < m_size )
        ++m_position;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CImapValueParser::ReadWord( void )
{GUCEF_TRACE;

    SkipSpaces();
    const UInt32 start = m_position;
    while ( m_position < m_size && ' ' != m_data[ m_position ] && '(' != m_data[ m_position ] && ')' != m_data[ m_position ] && '\r' != m_data[ m_position ] && '\n' != m_data[ m_position ] )
        ++m_position;
    return m_position > start ? CORE::CString( m_data + start, m_position - start ) : CORE::CString();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CImapValueParser::ReadRest( void )
{GUCEF_TRACE;

    SkipSpaces();
    const UInt32 start = m_position;
    m_position = m_size;
    return m_size > start ? CMimeCodec::SanitizeUtf8( m_data + start, m_size - start ) : CORE::CString();
}

/*-------------------------------------------------------------------------*/

bool
CImapValueParser::ParseValue( CImapValue& value )
{GUCEF_TRACE;

    value.Clear();
    SkipSpaces();
    const bool ok = ParseValueAtDepth( value, 0 );
    if ( !ok )
        value.Clear();
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
CImapValueParser::ParseValuesUntilEnd( CImapValue& list )
{GUCEF_TRACE;

    list.SetList();
    for ( ;; )
    {
        SkipSpaces();
        if ( AtEnd() )
            return true;
        if ( '\r' == Peek() || '\n' == Peek() )
        {
            Advance();
            continue;
        }
        CImapValue* element = list.AddElement();
        if ( !ParseValueAtDepth( *element, 0 ) )
        {
            list.Clear();
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CImapValueParser::ParseValueAtDepth( CImapValue& value, UInt32 depth )
{GUCEF_TRACE;

    if ( ++m_valueCount > m_maxValues )
        return Fail( "The response has more values than the limit" );
    if ( m_position >= m_size )
        return Fail( "A value was expected" );

    const char c = m_data[ m_position ];

    // a list
    if ( '(' == c )
    {
        if ( depth >= m_maxDepth )
            return Fail( "The lists of the response are nested too deep" );
        ++m_position;
        value.SetList();
        for ( ;; )
        {
            SkipSpaces();
            if ( m_position >= m_size )
                return Fail( "A list was not closed" );
            if ( ')' == m_data[ m_position ] )
            {
                ++m_position;
                return true;
            }
            CImapValue* element = value.AddElement();
            if ( !ParseValueAtDepth( *element, depth + 1 ) )
                return false;
        }
    }

    // a quoted string
    if ( '"' == c )
    {
        ++m_position;
        GUCEF::vector< char > bytes;
        for ( ;; )
        {
            if ( m_position >= m_size )
                return Fail( "A quoted string was not closed" );
            const char q = m_data[ m_position++ ];
            if ( '"' == q )
                break;
            if ( '\r' == q || '\n' == q || '\0' == q )
                return Fail( "A quoted string has a line break or NUL" );
            if ( '\\' == q )
            {
                if ( m_position >= m_size )
                    return Fail( "A quoted string was not closed" );
                bytes.push_back( m_data[ m_position++ ] );
            }
            else
                bytes.push_back( q );
        }
        value.SetString( bytes.empty() ? "" : &bytes[ 0 ], (UInt32) bytes.size() );
        return true;
    }

    // a literal: {n} or {n+}, CR LF, n bytes
    if ( '{' == c )
    {
        UInt32 i = m_position + 1;
        UInt64 count = 0;
        UInt32 digits = 0;
        while ( i < m_size && m_data[ i ] >= '0' && m_data[ i ] <= '9' )
        {
            count = count * 10 + (UInt64)( m_data[ i ] - '0' );
            if ( count > 0xFFFFFFFFull )
                return Fail( "A literal is larger than the limit" );
            ++i;
            ++digits;
        }
        if ( 0 == digits )
            return Fail( "A literal has no size" );
        if ( i < m_size && '+' == m_data[ i ] )
            ++i;
        if ( i >= m_size || '}' != m_data[ i ] )
            return Fail( "A literal is not closed" );
        ++i;
        if ( i < m_size && '\r' == m_data[ i ] )
            ++i;
        if ( i >= m_size || '\n' != m_data[ i ] )
            return Fail( "A literal has no line break" );
        ++i;
        // checked against what is there before anything is made of it
        if ( count > m_size - i )
            return Fail( "A literal is larger than the data that came with it" );
        value.SetString( m_data + i, (UInt32) count );
        m_position = i + (UInt32) count;
        return true;
    }

    if ( ')' == c || '\r' == c || '\n' == c )
        return Fail( "A value was expected" );

    // an atom, a number or NIL. A section in brackets belongs to the atom, spaces and lists inside it too
    const UInt32 start = m_position;
    UInt32 bracketDepth = 0;
    while ( m_position < m_size )
    {
        const char a = m_data[ m_position ];
        if ( '[' == a )
            ++bracketDepth;
        else
        if ( ']' == a && bracketDepth > 0 )
            --bracketDepth;
        else
        if ( 0 == bracketDepth && ( ' ' == a || '(' == a || ')' == a || '\r' == a || '\n' == a || '"' == a ) )
            break;
        if ( '\r' == a || '\n' == a || '\0' == a )
            return Fail( "An atom has a line break" );
        ++m_position;
    }
    if ( bracketDepth > 0 )
        return Fail( "A section was not closed" );
    const UInt32 length = m_position - start;
    if ( 0 == length )
        return Fail( "A value was expected" );
    if ( length > 65536 )
        return Fail( "An atom is too long" );

    bool allDigits = true;
    for ( UInt32 i=0; i<length; ++i )
        if ( m_data[ start + i ] < '0' || m_data[ start + i ] > '9' )
            allDigits = false;
    if ( allDigits && length <= 19 )
    {
        UInt64 number = 0;
        for ( UInt32 i=0; i<length; ++i )
            number = number * 10 + (UInt64)( m_data[ start + i ] - '0' );
        value.SetNumber( number );
        return true;
    }
    if ( 3 == length && CORE::CString( m_data + start, 3 ).Equals( "NIL", false ) )
    {
        value.SetNil();
        return true;
    }
    value.SetAtom( CMimeCodec::SanitizeUtf8( m_data + start, length ) );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
