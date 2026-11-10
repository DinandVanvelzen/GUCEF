/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#include "gucefCORE_CJsonSyntaxValidator.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The state of one check: the bytes, the position and the stack of open containers (a set bit: an object) */
class CJsonSyntaxCheck
{
    public:

    CJsonSyntaxCheck( const UInt8* bytes, UInt32 count, UInt32 maxDepth, CString* error )
        : m_bytes( bytes )
        , m_count( count )
        , m_pos( 0 )
        , m_depth( 0 )
        , m_maxDepth( maxDepth )
        , m_error( error )
    {
        memset( m_objectBits, 0, sizeof( m_objectBits ) );
    }

    bool Run( void );

    private:

    bool Fail( const char* reason )
    {
        if ( GUCEF_NULL != m_error )
            *m_error = "JSON syntax: at byte " + ToString( m_pos ) + ": " + reason;
        return false;
    }

    void SkipWhiteSpace( void )
    {
        while ( m_pos < m_count && ( ' ' == m_bytes[ m_pos ] || '\t' == m_bytes[ m_pos ] || '\n' == m_bytes[ m_pos ] || '\r' == m_bytes[ m_pos ] ) )
            ++m_pos;
    }

    bool Push( bool object )
    {
        if ( m_depth >= m_maxDepth )
            return Fail( "objects and arrays nest deeper than allowed" );
        if ( object )
            m_objectBits[ m_depth >> 3 ] |= (UInt8) ( 1u << ( m_depth & 7 ) );
        else
            m_objectBits[ m_depth >> 3 ] &= (UInt8) ~( 1u << ( m_depth & 7 ) );
        ++m_depth;
        ++m_pos;
        return true;
    }

    bool TopIsObject( void ) const
    {
        const UInt32 top = m_depth - 1;
        return 0 != ( m_objectBits[ top >> 3 ] & ( 1u << ( top & 7 ) ) );
    }

    bool ParseLiteral( const char* literal, UInt32 length );
    bool ParseNumber( void );
    bool ParseString( void );
    bool ParseUtf8Sequence( void );

    const UInt8* m_bytes;
    UInt32 m_count;
    UInt32 m_pos;
    UInt32 m_depth;
    UInt32 m_maxDepth;
    CString* m_error;
    UInt8 m_objectBits[ CJsonSyntaxValidator::MaxTrackedNestingDepth / 8 ];
};

/*-------------------------------------------------------------------------*/

bool
CJsonSyntaxCheck::ParseLiteral( const char* literal, UInt32 length )
{
    if ( m_count - m_pos < length || 0 != memcmp( m_bytes + m_pos, literal, length ) )
        return Fail( "a value that is not a JSON value" );
    m_pos += length;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CJsonSyntaxCheck::ParseNumber( void )
{
    if ( m_pos < m_count && '-' == m_bytes[ m_pos ] )
        ++m_pos;
    if ( m_pos >= m_count || m_bytes[ m_pos ] < '0' || m_bytes[ m_pos ] > '9' )
        return Fail( "a number without digits" );
    if ( '0' == m_bytes[ m_pos ] )
    {
        ++m_pos;
        if ( m_pos < m_count && m_bytes[ m_pos ] >= '0' && m_bytes[ m_pos ] <= '9' )
            return Fail( "a number with a leading zero" );
    }
    else
    {
        while ( m_pos < m_count && m_bytes[ m_pos ] >= '0' && m_bytes[ m_pos ] <= '9' )
            ++m_pos;
    }
    if ( m_pos < m_count && '.' == m_bytes[ m_pos ] )
    {
        ++m_pos;
        if ( m_pos >= m_count || m_bytes[ m_pos ] < '0' || m_bytes[ m_pos ] > '9' )
            return Fail( "a fraction without digits" );
        while ( m_pos < m_count && m_bytes[ m_pos ] >= '0' && m_bytes[ m_pos ] <= '9' )
            ++m_pos;
    }
    if ( m_pos < m_count && ( 'e' == m_bytes[ m_pos ] || 'E' == m_bytes[ m_pos ] ) )
    {
        ++m_pos;
        if ( m_pos < m_count && ( '+' == m_bytes[ m_pos ] || '-' == m_bytes[ m_pos ] ) )
            ++m_pos;
        if ( m_pos >= m_count || m_bytes[ m_pos ] < '0' || m_bytes[ m_pos ] > '9' )
            return Fail( "an exponent without digits" );
        while ( m_pos < m_count && m_bytes[ m_pos ] >= '0' && m_bytes[ m_pos ] <= '9' )
            ++m_pos;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CJsonSyntaxCheck::ParseUtf8Sequence( void )
{
    // RFC 3629: no overlong forms, no surrogates, nothing above U+10FFFF
    const UInt8 lead = m_bytes[ m_pos ];
    UInt32 length = 0;
    UInt8 low = 0x80, high = 0xBF;
    if ( lead >= 0xC2 && lead <= 0xDF ) length = 2;
    else if ( 0xE0 == lead ) { length = 3; low = 0xA0; }
    else if ( lead >= 0xE1 && lead <= 0xEC ) length = 3;
    else if ( 0xED == lead ) { length = 3; high = 0x9F; }
    else if ( lead >= 0xEE && lead <= 0xEF ) length = 3;
    else if ( 0xF0 == lead ) { length = 4; low = 0x90; }
    else if ( lead >= 0xF1 && lead <= 0xF3 ) length = 4;
    else if ( 0xF4 == lead ) { length = 4; high = 0x8F; }
    else
        return Fail( "a byte that does not start a UTF-8 sequence" );
    if ( m_count - m_pos < length )
        return Fail( "a UTF-8 sequence cut short" );
    if ( m_bytes[ m_pos + 1 ] < low || m_bytes[ m_pos + 1 ] > high )
        return Fail( "an invalid UTF-8 sequence" );
    for ( UInt32 i=2; i<length; ++i )
    {
        if ( m_bytes[ m_pos + i ] < 0x80 || m_bytes[ m_pos + i ] > 0xBF )
            return Fail( "an invalid UTF-8 sequence" );
    }
    m_pos += length;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CJsonSyntaxCheck::ParseString( void )
{
    ++m_pos;    // the opening quote
    while ( m_pos < m_count )
    {
        const UInt8 c = m_bytes[ m_pos ];
        if ( '"' == c )
        {
            ++m_pos;
            return true;
        }
        if ( c < 0x20 )
            return Fail( "a control character in a string" );
        if ( '\\' == c )
        {
            ++m_pos;
            if ( m_pos >= m_count )
                break;
            const UInt8 e = m_bytes[ m_pos ];
            if ( 'u' == e )
            {
                if ( m_count - m_pos < 5 )
                    return Fail( "a \\u escape cut short" );
                for ( UInt32 i=1; i<=4; ++i )
                {
                    const UInt8 h = m_bytes[ m_pos + i ];
                    if ( !( ( h >= '0' && h <= '9' ) || ( h >= 'a' && h <= 'f' ) || ( h >= 'A' && h <= 'F' ) ) )
                        return Fail( "a \\u escape without four hex digits" );
                }
                m_pos += 5;
                continue;
            }
            if ( '"' != e && '\\' != e && '/' != e && 'b' != e && 'f' != e && 'n' != e && 'r' != e && 't' != e )
                return Fail( "an escape JSON does not have" );
            ++m_pos;
            continue;
        }
        if ( c < 0x80 )
        {
            ++m_pos;
            continue;
        }
        if ( !ParseUtf8Sequence() )
            return false;
    }
    return Fail( "a string without its closing quote" );
}

/*-------------------------------------------------------------------------*/

bool
CJsonSyntaxCheck::Run( void )
{
    // a value is expected at the top of the loop; after a value the container decides what follows
    SkipWhiteSpace();
    while ( true )
    {
        if ( m_pos >= m_count )
            return Fail( "the text ends where a value is expected" );

        bool valueDone = false;
        const UInt8 c = m_bytes[ m_pos ];
        switch ( c )
        {
            case '{':
            {
                if ( !Push( true ) )
                    return false;
                SkipWhiteSpace();
                if ( m_pos < m_count && '}' == m_bytes[ m_pos ] )
                {
                    ++m_pos;
                    --m_depth;
                    valueDone = true;
                    break;
                }
                if ( m_pos >= m_count || '"' != m_bytes[ m_pos ] )
                    return Fail( "an object member without a string name" );
                if ( !ParseString() )
                    return false;
                SkipWhiteSpace();
                if ( m_pos >= m_count || ':' != m_bytes[ m_pos ] )
                    return Fail( "an object member name without ':'" );
                ++m_pos;
                SkipWhiteSpace();
                continue;
            }
            case '[':
            {
                if ( !Push( false ) )
                    return false;
                SkipWhiteSpace();
                if ( m_pos < m_count && ']' == m_bytes[ m_pos ] )
                {
                    ++m_pos;
                    --m_depth;
                    valueDone = true;
                    break;
                }
                continue;
            }
            case '"': { if ( !ParseString() ) return false; valueDone = true; break; }
            case 't': { if ( !ParseLiteral( "true", 4 ) ) return false; valueDone = true; break; }
            case 'f': { if ( !ParseLiteral( "false", 5 ) ) return false; valueDone = true; break; }
            case 'n': { if ( !ParseLiteral( "null", 4 ) ) return false; valueDone = true; break; }
            default:
            {
                if ( '-' != c && ( c < '0' || c > '9' ) )
                    return Fail( "a byte that starts no JSON value" );
                if ( !ParseNumber() )
                    return false;
                valueDone = true;
                break;
            }
        }

        // after a value: close containers, or a ',' and the next element or member
        while ( valueDone )
        {
            SkipWhiteSpace();
            if ( 0 == m_depth )
            {
                if ( m_pos != m_count )
                    return Fail( "more after the JSON value" );
                return true;
            }
            if ( m_pos >= m_count )
                return Fail( "the text ends inside an object or array" );
            const UInt8 next = m_bytes[ m_pos ];
            const bool object = TopIsObject();
            if ( ( object && '}' == next ) || ( !object && ']' == next ) )
            {
                ++m_pos;
                --m_depth;
                continue;
            }
            if ( ',' != next )
                return Fail( object ? "a member not followed by ',' or '}'" : "an element not followed by ',' or ']'" );
            ++m_pos;
            SkipWhiteSpace();
            if ( object )
            {
                if ( m_pos >= m_count || '"' != m_bytes[ m_pos ] )
                    return Fail( "an object member without a string name" );
                if ( !ParseString() )
                    return false;
                SkipWhiteSpace();
                if ( m_pos >= m_count || ':' != m_bytes[ m_pos ] )
                    return Fail( "an object member name without ':'" );
                ++m_pos;
                SkipWhiteSpace();
            }
            valueDone = false;
        }
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CJsonSyntaxValidator::IsValid( const void* text        ,
                               UInt32 byteCount        ,
                               UInt32 maxNestingDepth  ,
                               CString* error          )
{GUCEF_TRACE;

    if ( GUCEF_NULL == text && 0 != byteCount )
    {
        if ( GUCEF_NULL != error )
            *error = "JSON syntax: no text";
        return false;
    }
    if ( maxNestingDepth > MaxTrackedNestingDepth )
        maxNestingDepth = MaxTrackedNestingDepth;
    CJsonSyntaxCheck check( static_cast< const UInt8* >( text ), byteCount, maxNestingDepth, error );
    return check.Run();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
