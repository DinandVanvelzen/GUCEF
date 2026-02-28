/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>
#include <assert.h>

#ifndef GUCEF_CORE_CUTF8STRING_H
#include "gucefCORE_CUtf8String.h"
#define GUCEF_CORE_CUTF8STRING_H
#endif /* GUCEF_CORE_CUTF8STRING_H ? */

#ifndef GUCEF_CORE_CASCIISTRING_H
#include "gucefCORE_CAsciiString.h"
#define GUCEF_CORE_CASCIISTRING_H
#endif /* GUCEF_CORE_CASCIISTRING_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#include "gucefCORE_CUtf32String.h"

#ifndef GUCEF_CORE_GUCEF_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_GUCEF_ESSENTIALS_H
#endif /* GUCEF_CORE_GUCEF_ESSENTIALS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CUtf32String CUtf32String::Empty;
const CUtf32String::StringVector CUtf32String::EmptyStringVector;
const CUtf32String::StringSet CUtf32String::EmptyStringSet;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  Returns the number of UTF-8 bytes that form the code-point whose first
 *  byte is leadByte.  Returns 1 for invalid / continuation bytes (skip).
 */
static inline UInt32
Utf8SeqLen( unsigned char leadByte )
{
    if ( leadByte < 0x80u )              return 1u;
    if ( ( leadByte & 0xE0u ) == 0xC0u ) return 2u;
    if ( ( leadByte & 0xF0u ) == 0xE0u ) return 3u;
    if ( ( leadByte & 0xF8u ) == 0xF0u ) return 4u;
    return 1u; /* invalid / continuation byte */
}

/*-------------------------------------------------------------------------*/

/*
 *  Decodes one UTF-8 code point starting at *ptr, advances *ptr past the
 *  sequence, and returns the decoded UInt32 code point.
 *  ptr must not be NULL and *ptr must not be '\0'.
 */
static inline UInt32
Utf8DecodeOne( const char*& ptr )
{
    unsigned char lead = static_cast<unsigned char>( *ptr );
    UInt32 cp = 0u;

    if ( lead < 0x80u )
    {
        cp = lead;
        ++ptr;
    }
    else if ( ( lead & 0xE0u ) == 0xC0u )
    {
        cp  = static_cast<UInt32>( lead & 0x1Fu ) << 6;
        cp |= static_cast<UInt32>( static_cast<unsigned char>( *( ptr + 1 ) ) & 0x3Fu );
        ptr += 2;
    }
    else if ( ( lead & 0xF0u ) == 0xE0u )
    {
        cp  = static_cast<UInt32>( lead & 0x0Fu ) << 12;
        cp |= static_cast<UInt32>( static_cast<unsigned char>( *( ptr + 1 ) ) & 0x3Fu ) << 6;
        cp |= static_cast<UInt32>( static_cast<unsigned char>( *( ptr + 2 ) ) & 0x3Fu );
        ptr += 3;
    }
    else if ( ( lead & 0xF8u ) == 0xF0u )
    {
        cp  = static_cast<UInt32>( lead & 0x07u ) << 18;
        cp |= static_cast<UInt32>( static_cast<unsigned char>( *( ptr + 1 ) ) & 0x3Fu ) << 12;
        cp |= static_cast<UInt32>( static_cast<unsigned char>( *( ptr + 2 ) ) & 0x3Fu ) << 6;
        cp |= static_cast<UInt32>( static_cast<unsigned char>( *( ptr + 3 ) ) & 0x3Fu );
        ptr += 4;
    }
    else
    {
        /* Invalid / continuation byte - skip one byte */
        cp = static_cast<UInt32>( lead );
        ++ptr;
    }

    return cp;
}

/*-------------------------------------------------------------------------*/

/*
 *  Counts the number of UTF-8 code points in the null-terminated string src.
 *  Returns the count (not byte length).
 */
static UInt32
Utf8CountCodePoints( const char* src )
{
    if ( GUCEF_NULL == src )
        return 0u;

    UInt32 count = 0u;
    const char* p = src;
    while ( '\0' != *p )
    {
        p += Utf8SeqLen( static_cast<unsigned char>( *p ) );
        ++count;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

/*
 *  Counts the number of UTF-8 code points in byteSize bytes of src.
 */
static UInt32
Utf8CountCodePointsInBytes( const char* src, UInt32 byteSize )
{
    if ( GUCEF_NULL == src || 0u == byteSize )
        return 0u;

    UInt32 count = 0u;
    UInt32 i = 0u;
    while ( i < byteSize && '\0' != src[ i ] )
    {
        UInt32 seqLen = Utf8SeqLen( static_cast<unsigned char>( src[ i ] ) );
        if ( i + seqLen > byteSize )
            break;
        i += seqLen;
        ++count;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

/*
 *  Appends the UTF-8 representation of a single code point to out.
 */
static void
Utf32CpToUtf8( UInt32 cp, std::string& out )
{
    if ( cp < 0x80u )
    {
        out += static_cast<char>( cp );
    }
    else if ( cp < 0x800u )
    {
        out += static_cast<char>( 0xC0u | ( cp >> 6 ) );
        out += static_cast<char>( 0x80u | ( cp & 0x3Fu ) );
    }
    else if ( cp < 0x10000u )
    {
        out += static_cast<char>( 0xE0u | ( cp >> 12 ) );
        out += static_cast<char>( 0x80u | ( ( cp >> 6 ) & 0x3Fu ) );
        out += static_cast<char>( 0x80u | ( cp & 0x3Fu ) );
    }
    else
    {
        out += static_cast<char>( 0xF0u | ( cp >> 18 ) );
        out += static_cast<char>( 0x80u | ( ( cp >> 12 ) & 0x3Fu ) );
        out += static_cast<char>( 0x80u | ( ( cp >> 6 ) & 0x3Fu ) );
        out += static_cast<char>( 0x80u | ( cp & 0x3Fu ) );
    }
}

/*-------------------------------------------------------------------------*/

/*
 *  Converts a UTF-32 buffer of length code points to a UTF-8 std::string.
 */
static std::string
Utf32BufferToUtf8Str( const UInt32* buf, UInt32 length )
{
    std::string result;
    if ( GUCEF_NULL == buf || 0u == length )
        return result;

    /* Pre-reserve: average 1.1 bytes per code point for typical content */
    result.reserve( (size_t)length );

    for ( UInt32 i = 0u; i < length; ++i )
    {
        Utf32CpToUtf8( buf[ i ], result );
    }
    return result;
}

/*-------------------------------------------------------------------------*/

/*
 *  Counts the number of code points in a null-terminated UTF-32 (UInt32) buffer.
 */
static UInt32
Utf32Strlen( const UInt32* str )
{
    if ( GUCEF_NULL == str )
        return 0u;
    UInt32 len = 0u;
    while ( 0u != str[ len ] )
        ++len;
    return len;
}

/*-------------------------------------------------------------------------*/

/*
 *  Compares two null-terminated UTF-32 buffers lexicographically by code
 *  point value.  Returns <0, 0, or >0.
 */
static int
Utf32Strcmp( const UInt32* a, const UInt32* b )
{
    while ( *a && ( *a == *b ) )
    {
        ++a;
        ++b;
    }
    if ( *a < *b ) return -1;
    if ( *a > *b ) return  1;
    return 0;
}

/*-------------------------------------------------------------------------*/

/*
 *  Compares n code points of two UTF-32 buffers.
 */
static int
Utf32Memcmp( const UInt32* a, const UInt32* b, UInt32 n )
{
    for ( UInt32 i = 0u; i < n; ++i )
    {
        if ( a[ i ] < b[ i ] ) return -1;
        if ( a[ i ] > b[ i ] ) return  1;
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

/*
 *  Allocates a new UTF-32 buffer from a null-terminated UTF-8 C-string,
 *  setting outLength to the number of code points (excluding null terminator).
 *  Returns NULL if src is NULL or empty.
 */
static UInt32*
AllocFromUtf8( const char* src, UInt32& outLength )
{
    outLength = 0u;
    if ( GUCEF_NULL == src || '\0' == *src )
        return GUCEF_NULL;

    UInt32 cpCount = Utf8CountCodePoints( src );
    if ( 0u == cpCount )
        return GUCEF_NULL;

    UInt32* buf = GUCEF_NEW UInt32[ cpCount + 1u ];
    assert( GUCEF_NULL != buf );

    const char* p = src;
    for ( UInt32 i = 0u; i < cpCount; ++i )
    {
        buf[ i ] = Utf8DecodeOne( p );
    }
    buf[ cpCount ] = 0u;
    outLength = cpCount;
    return buf;
}

/*-------------------------------------------------------------------------*/

/*
 *  Allocates a new UTF-32 buffer from byteSize bytes of UTF-8.
 */
static UInt32*
AllocFromUtf8Bytes( const char* src, UInt32 byteSize, UInt32& outLength )
{
    outLength = 0u;
    if ( GUCEF_NULL == src || 0u == byteSize )
        return GUCEF_NULL;

    UInt32 cpCount = Utf8CountCodePointsInBytes( src, byteSize );
    if ( 0u == cpCount )
        return GUCEF_NULL;

    UInt32* buf = GUCEF_NEW UInt32[ cpCount + 1u ];
    assert( GUCEF_NULL != buf );

    const char* p = src;
    const char* end = src + byteSize;
    UInt32 i = 0u;
    while ( p < end && '\0' != *p && i < cpCount )
    {
        buf[ i++ ] = Utf8DecodeOne( p );
    }
    buf[ cpCount ] = 0u;
    outLength = cpCount;
    return buf;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CUtf32String::CUtf32String( void )
    : m_string( GUCEF_NULL )
    , m_length( 0u )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUtf32String::CUtf32String( const CUtf32String& src )
    : m_string( GUCEF_NULL )
    , m_length( 0u )
{GUCEF_TRACE;

    if ( src.m_length > 0u )
    {
        m_length = src.m_length;
        m_string = GUCEF_NEW UInt32[ m_length + 1u ];
        assert( GUCEF_NULL != m_string );
        memcpy( m_string, src.m_string, ( m_length + 1u ) * sizeof( UInt32 ) );
    }
}

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_RVALUE_REFERENCES_SUPPORTED

CUtf32String::CUtf32String( CUtf32String&& src ) GUCEF_NOEXCEPT
    : m_string( src.m_string )
    , m_length( src.m_length )
{GUCEF_TRACE;

    src.m_string = GUCEF_NULL;
    src.m_length = 0u;
}

#endif /* GUCEF_RVALUE_REFERENCES_SUPPORTED */

/*-------------------------------------------------------------------------*/

CUtf32String::CUtf32String( const CUtf8String& src )
    : m_string( GUCEF_NULL )
    , m_length( 0u )
{GUCEF_TRACE;

    const char* utf8Buf = src.C_String();
    if ( GUCEF_NULL != utf8Buf && '\0' != *utf8Buf )
    {
        m_string = AllocFromUtf8Bytes( utf8Buf, src.ByteSize(), m_length );
    }
}

/*-------------------------------------------------------------------------*/

CUtf32String::CUtf32String( const CAsciiString& src )
    : m_string( GUCEF_NULL )
    , m_length( 0u )
{GUCEF_TRACE;

    UInt32 srcLen = src.Length();
    if ( srcLen > 0u )
    {
        const char* asciiStr = src.C_String();
        m_length = srcLen;
        m_string = GUCEF_NEW UInt32[ m_length + 1u ];
        assert( GUCEF_NULL != m_string );
        for ( UInt32 i = 0u; i < srcLen; ++i )
        {
            m_string[ i ] = static_cast<UInt32>( static_cast<unsigned char>( asciiStr[ i ] ) );
        }
        m_string[ m_length ] = 0u;
    }
}

/*-------------------------------------------------------------------------*/

CUtf32String::CUtf32String( const std::string& src )
    : m_string( GUCEF_NULL )
    , m_length( 0u )
{GUCEF_TRACE;

    if ( !src.empty() )
    {
        m_string = AllocFromUtf8Bytes( src.c_str(), (UInt32) src.size(), m_length );
    }
}

/*-------------------------------------------------------------------------*/

CUtf32String::CUtf32String( const char* src ) GUCEF_NOEXCEPT
    : m_string( GUCEF_NULL )
    , m_length( 0u )
{GUCEF_TRACE;

    if ( GUCEF_NULL != src && '\0' != *src )
    {
        m_string = AllocFromUtf8( src, m_length );
    }
}

/*-------------------------------------------------------------------------*/

CUtf32String::CUtf32String( const char* src              ,
                             UInt32 byteSize              ,
                             bool reexamineByteSize       ) GUCEF_NOEXCEPT
    : m_string( GUCEF_NULL )
    , m_length( 0u )
{GUCEF_TRACE;

    if ( GUCEF_NULL != src && 0u < byteSize )
    {
        if ( reexamineByteSize )
        {
            /* Clamp to actual null-terminated length */
            UInt32 actualLen = 0u;
            while ( actualLen < byteSize && '\0' != src[ actualLen ] )
                ++actualLen;
            byteSize = actualLen;
        }

        if ( 0u < byteSize )
        {
            m_string = AllocFromUtf8Bytes( src, byteSize, m_length );
        }
    }
}

/*-------------------------------------------------------------------------*/

CUtf32String::CUtf32String( const UInt32* src ) GUCEF_NOEXCEPT
    : m_string( GUCEF_NULL )
    , m_length( 0u )
{GUCEF_TRACE;

    if ( GUCEF_NULL != src )
    {
        m_length = Utf32Strlen( src );
        if ( m_length > 0u )
        {
            m_string = GUCEF_NEW UInt32[ m_length + 1u ];
            assert( GUCEF_NULL != m_string );
            memcpy( m_string, src, ( m_length + 1u ) * sizeof( UInt32 ) );
        }
    }
}

/*-------------------------------------------------------------------------*/

CUtf32String::CUtf32String( const UInt32* src                ,
                             UInt32 lengthInCodePoints        ,
                             bool reexamineLength             ) GUCEF_NOEXCEPT
    : m_string( GUCEF_NULL )
    , m_length( 0u )
{GUCEF_TRACE;

    if ( GUCEF_NULL != src && 0u < lengthInCodePoints )
    {
        if ( reexamineLength )
        {
            UInt32 actualLen = 0u;
            while ( actualLen < lengthInCodePoints && 0u != src[ actualLen ] )
                ++actualLen;
            lengthInCodePoints = actualLen;
        }

        if ( 0u < lengthInCodePoints )
        {
            /* Don't add null-terminator twice if already present */
            if ( 0u == src[ lengthInCodePoints - 1u ] )
            {
                m_length = lengthInCodePoints - 1u;
                m_string = GUCEF_NEW UInt32[ lengthInCodePoints ];
                assert( GUCEF_NULL != m_string );
                memcpy( m_string, src, lengthInCodePoints * sizeof( UInt32 ) );
            }
            else
            {
                m_length = lengthInCodePoints;
                m_string = GUCEF_NEW UInt32[ lengthInCodePoints + 1u ];
                assert( GUCEF_NULL != m_string );
                memcpy( m_string, src, lengthInCodePoints * sizeof( UInt32 ) );
                m_string[ lengthInCodePoints ] = 0u;
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

CUtf32String::CUtf32String( const UInt32 utf32CodePoint )
    : m_string( GUCEF_NULL )
    , m_length( 0u )
{GUCEF_TRACE;

    UInt32 cp = utf32CodePoint;
    Set( &cp, 1u );
}

/*-------------------------------------------------------------------------*/

CUtf32String::CUtf32String( const int NULLvalue )
    : m_string( GUCEF_NULL )
    , m_length( 0u )
{GUCEF_TRACE;

    assert( NULLvalue == static_cast<int>( GUCEF_NULL ) );
}

/*-------------------------------------------------------------------------*/

CUtf32String::~CUtf32String()
{GUCEF_TRACE;

    GUCEF_DELETE []m_string;
    m_string = GUCEF_NULL;
    m_length = 0u;
}

/*-------------------------------------------------------------------------*/

CUtf32String&
CUtf32String::operator=( const CUtf32String& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        GUCEF_DELETE []m_string;
        m_string = GUCEF_NULL;
        m_length = src.m_length;

        if ( m_length > 0u )
        {
            m_string = GUCEF_NEW UInt32[ m_length + 1u ];
            assert( GUCEF_NULL != m_string );
            memcpy( m_string, src.m_string, ( m_length + 1u ) * sizeof( UInt32 ) );
        }
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CUtf32String&
CUtf32String::operator=( const char* src )
{GUCEF_TRACE;

    GUCEF_DELETE []m_string;
    m_string = GUCEF_NULL;
    m_length = 0u;

    if ( GUCEF_NULL != src && '\0' != *src )
    {
        m_string = AllocFromUtf8( src, m_length );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CUtf32String&
CUtf32String::operator+=( const CUtf32String& other )
{GUCEF_TRACE;

    Append( other.m_string, other.m_length );
    return *this;
}

/*-------------------------------------------------------------------------*/

CUtf32String&
CUtf32String::operator+=( const char* other )
{GUCEF_TRACE;

    if ( GUCEF_NULL != other && '\0' != *other )
    {
        CUtf32String tmp( other );
        Append( tmp.m_string, tmp.m_length );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CUtf32String&
CUtf32String::operator+=( UInt32 codePoint )
{GUCEF_TRACE;

    Append( &codePoint, 1u );
    return *this;
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32String::operator+( const CUtf32String& addition ) const
{GUCEF_TRACE;

    CUtf32String tmp( *this );
    tmp += addition;
    return tmp;
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32String::operator+( const char* addition ) const
{GUCEF_TRACE;

    CUtf32String tmp( *this );
    tmp += addition;
    return tmp;
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32String::operator+( UInt32 codePoint ) const
{GUCEF_TRACE;

    CUtf32String tmp( *this );
    tmp += codePoint;
    return tmp;
}

/*-------------------------------------------------------------------------*/

bool
CUtf32String::operator==( const CUtf32String& other ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_string && GUCEF_NULL == other.m_string )
        return true;
    if ( GUCEF_NULL == m_string || GUCEF_NULL == other.m_string )
        return false;
    if ( m_length != other.m_length )
        return false;
    return Utf32Strcmp( m_string, other.m_string ) == 0;
}

/*-------------------------------------------------------------------------*/

bool
CUtf32String::operator==( const char* other ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_string && ( GUCEF_NULL == other || '\0' == *other ) )
        return true;
    if ( GUCEF_NULL == m_string || GUCEF_NULL == other )
        return false;
    return *this == CUtf32String( other );
}

/*-------------------------------------------------------------------------*/

bool
CUtf32String::operator==( const int NULLvalue ) const
{GUCEF_TRACE;

    if ( NULLvalue == static_cast<int>( GUCEF_NULL ) )
    {
        return m_length == 0u;
    }
    GUCEF_ASSERT_ALWAYS;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CUtf32String::operator==( UInt32 codePoint ) const
{GUCEF_TRACE;

    return *this == CUtf32String( codePoint );
}

/*-------------------------------------------------------------------------*/

bool
CUtf32String::operator!=( const CUtf32String& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

bool
CUtf32String::operator!=( const char* other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

bool
CUtf32String::operator!=( const int NULLvalue ) const
{GUCEF_TRACE;

    if ( NULLvalue == static_cast<int>( GUCEF_NULL ) )
    {
        return m_length > 0u;
    }
    GUCEF_ASSERT_ALWAYS;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CUtf32String::operator!=( UInt32 codePoint ) const
{GUCEF_TRACE;

    return !( *this == codePoint );
}

/*-------------------------------------------------------------------------*/

bool
CUtf32String::operator<( const CUtf32String& other ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_string && GUCEF_NULL != other.m_string )
    {
        return Utf32Strcmp( m_string, other.m_string ) < 0;
    }
    if ( GUCEF_NULL == m_string && GUCEF_NULL != other.m_string )
    {
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CUtf32String::operator<=( const CUtf32String& other ) const
{GUCEF_TRACE;

    return ( *this < other ) || ( *this == other );
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf32String::operator[]( const UInt32 index ) const
{GUCEF_TRACE;

    if ( index <= m_length )
        return m_string[ index ];
    return 0u;
}

/*-------------------------------------------------------------------------*/

UInt32&
CUtf32String::operator[]( const UInt32 index )
{GUCEF_TRACE;

    static UInt32 outOfBoundsSlot = 0u;
    if ( index <= m_length )
        return m_string[ index ];

    //GUCEF_ASSERT_ALWAYS;
    return outOfBoundsSlot;
}

/*-------------------------------------------------------------------------*/

bool
CUtf32String::IsNULLOrEmpty( void ) const
{GUCEF_TRACE;

    return m_length == 0u;
}

/*-------------------------------------------------------------------------*/

CUtf32String::operator std::string() const
{GUCEF_TRACE;

    return Utf32BufferToUtf8Str( m_string, m_length );
}

/*-------------------------------------------------------------------------*/

const UInt32*
CUtf32String::C_String( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_string )
        return m_string;

    /* Return a pointer to a static null code-point instead of NULL */
    static const UInt32 nullCodePoint = 0u;
    return &nullCodePoint;
}

/*-------------------------------------------------------------------------*/

UInt32*
CUtf32String::C_String( void )
{GUCEF_TRACE;

    return m_string;
}

/*-------------------------------------------------------------------------*/

UInt32*
CUtf32String::Reserve( const UInt32 bufferSizeInCodePoints, Int32 newLength )
{GUCEF_TRACE;

    GUCEF_DELETE []m_string;
    m_string = GUCEF_NULL;

    if ( bufferSizeInCodePoints > 0u )
    {
        m_string = GUCEF_NEW UInt32[ bufferSizeInCodePoints ];
        assert( GUCEF_NULL != m_string );
    }
    m_length = bufferSizeInCodePoints;
    if ( newLength >= 0 )
        m_length = static_cast<UInt32>( newLength );

    return m_string;
}

/*-------------------------------------------------------------------------*/

std::string
CUtf32String::STL_String( void ) const
{GUCEF_TRACE;

    return Utf32BufferToUtf8Str( m_string, m_length );
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf32String::Length( void ) const
{GUCEF_TRACE;

    return m_length;
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf32String::ByteSize( void ) const
{GUCEF_TRACE;

    return ( m_length + 1u ) * static_cast<UInt32>( sizeof( UInt32 ) );
}

/*-------------------------------------------------------------------------*/

void
CUtf32String::SetLength( UInt32 newLength )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_string )
    {
        if ( m_length < newLength )
        {
            UInt32* newBuf = GUCEF_NEW UInt32[ newLength + 1u ];
            assert( GUCEF_NULL != newBuf );
            memcpy( newBuf, m_string, ( m_length + 1u ) * sizeof( UInt32 ) );
            GUCEF_DELETE []m_string;
            m_string = newBuf;
            /* Zero-fill the new code-point slots */
            for ( UInt32 i = m_length; i <= newLength; ++i )
                m_string[ i ] = 0u;
            m_length = newLength;
            return;
        }

        /* Truncate: zero-fill from newLength to old m_length */
        for ( UInt32 i = newLength; i <= m_length; ++i )
            m_string[ i ] = 0u;
        m_length = newLength;
        return;
    }

    if ( newLength > 0u )
    {
        m_string = GUCEF_NEW UInt32[ newLength + 1u ];
        assert( GUCEF_NULL != m_string );
        for ( UInt32 i = 0u; i <= newLength; ++i )
            m_string[ i ] = 0u;
    }
    m_length = newLength;
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf32String::DetermineLength( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_string )
    {
        m_length = Utf32Strlen( m_string );
        return m_length;
    }
    return 0u;
}

/*-------------------------------------------------------------------------*/

void
CUtf32String::Set( const UInt32* str,
                   UInt32 lengthInCodePoints )
{GUCEF_TRACE;

    if ( str != m_string )
    {
        GUCEF_DELETE []m_string;
        m_string = GUCEF_NULL;
        m_length = 0u;

        if ( GUCEF_NULL != str && lengthInCodePoints > 0u )
        {
            m_length = lengthInCodePoints;
            m_string = GUCEF_NEW UInt32[ m_length + 1u ];
            assert( GUCEF_NULL != m_string );
            memcpy( m_string, str, m_length * sizeof( UInt32 ) );
            m_string[ m_length ] = 0u;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CUtf32String::Set( const UInt32* str )
{GUCEF_TRACE;

    Clear();
    if ( GUCEF_NULL != str )
        Set( str, Utf32Strlen( str ) );
}

/*-------------------------------------------------------------------------*/

void
CUtf32String::Set( const char* src )
{GUCEF_TRACE;

    GUCEF_DELETE []m_string;
    m_string = GUCEF_NULL;
    m_length = 0u;

    if ( GUCEF_NULL != src && '\0' != *src )
    {
        m_string = AllocFromUtf8( src, m_length );
    }
}

/*-------------------------------------------------------------------------*/

void
CUtf32String::Scan( const UInt32* str,
                    UInt32 maxCodePoints )
{GUCEF_TRACE;

    if ( GUCEF_NULL == str )
        return;

    for ( UInt32 i = 0u; i < maxCodePoints; ++i )
    {
        if ( 0u == str[ i ] )
        {
            Set( str, i );
            return;
        }
    }
    /* No null terminator found within the limit */
    Set( str, maxCodePoints );
}

/*-------------------------------------------------------------------------*/

void
CUtf32String::Append( const UInt32* appendStr,
                      UInt32 lengthInCodePoints )
{GUCEF_TRACE;

    if ( appendStr != m_string     &&
         GUCEF_NULL != appendStr   &&
         lengthInCodePoints > 0u   )
    {
        if ( m_length > 0u )
        {
            UInt32* newBuf = GUCEF_NEW UInt32[ m_length + lengthInCodePoints + 1u ];
            assert( GUCEF_NULL != newBuf );
            memcpy( newBuf, m_string, m_length * sizeof( UInt32 ) );
            memcpy( newBuf + m_length, appendStr, lengthInCodePoints * sizeof( UInt32 ) );
            m_length = m_length + lengthInCodePoints;
            newBuf[ m_length ] = 0u;
            GUCEF_DELETE []m_string;
            m_string = newBuf;
        }
        else
        {
            GUCEF_DELETE []m_string;
            m_length = lengthInCodePoints;
            m_string = GUCEF_NEW UInt32[ m_length + 1u ];
            assert( GUCEF_NULL != m_string );
            memcpy( m_string, appendStr, m_length * sizeof( UInt32 ) );
            m_string[ m_length ] = 0u;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CUtf32String::Append( const UInt32* appendStr )
{GUCEF_TRACE;

    if ( GUCEF_NULL != appendStr )
        Append( appendStr, Utf32Strlen( appendStr ) );
}

/*-------------------------------------------------------------------------*/

void
CUtf32String::Append( const char* appendStr )
{GUCEF_TRACE;

    if ( GUCEF_NULL != appendStr && '\0' != *appendStr )
    {
        UInt32 decodedLength = 0u;
        UInt32* decoded = AllocFromUtf8( appendStr, decodedLength );
        if ( GUCEF_NULL != decoded )
        {
            Append( decoded, decodedLength );
            GUCEF_DELETE []decoded;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CUtf32String::Clear( void )
{GUCEF_TRACE;

    m_length = 0u;
    GUCEF_DELETE []m_string;
    m_string = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32String::Lowercase( void ) const
{GUCEF_TRACE;

    if ( m_length > 0u )
    {
        UInt32* buf = GUCEF_NEW UInt32[ m_length + 1u ];
        assert( GUCEF_NULL != buf );
        memcpy( buf, m_string, ( m_length + 1u ) * sizeof( UInt32 ) );
        for ( UInt32 i = 0u; i < m_length; ++i )
        {
            /* Basic Latin uppercase A-Z (U+0041 - U+005A) */
            if ( buf[ i ] >= 0x0041u && buf[ i ] <= 0x005Au )
                buf[ i ] += 0x0020u;
        }
        CUtf32String result( buf, m_length );
        GUCEF_DELETE []buf;
        return result;
    }
    return CUtf32String();
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32String::Uppercase( void ) const
{GUCEF_TRACE;

    if ( m_length > 0u )
    {
        UInt32* buf = GUCEF_NEW UInt32[ m_length + 1u ];
        assert( GUCEF_NULL != buf );
        memcpy( buf, m_string, ( m_length + 1u ) * sizeof( UInt32 ) );
        for ( UInt32 i = 0u; i < m_length; ++i )
        {
            /* Basic Latin lowercase a-z (U+0061 - U+007A) */
            if ( buf[ i ] >= 0x0061u && buf[ i ] <= 0x007Au )
                buf[ i ] -= 0x0020u;
        }
        CUtf32String result( buf, m_length );
        GUCEF_DELETE []buf;
        return result;
    }
    return CUtf32String();
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32String::ReplaceChar( UInt32 oldCodePoint,
                           UInt32 newCodePoint ) const
{GUCEF_TRACE;

    if ( m_length > 0u )
    {
        UInt32* buf = GUCEF_NEW UInt32[ m_length + 1u ];
        assert( GUCEF_NULL != buf );
        memcpy( buf, m_string, ( m_length + 1u ) * sizeof( UInt32 ) );
        for ( UInt32 i = 0u; i < m_length; ++i )
        {
            if ( buf[ i ] == oldCodePoint )
                buf[ i ] = newCodePoint;
        }
        CUtf32String result( buf, m_length );
        GUCEF_DELETE []buf;
        return result;
    }
    return CUtf32String();
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32String::ReplaceChars( const UInt32* oldCodePoints    ,
                             UInt32 nrOfOldCodePoints       ,
                             UInt32 newCodePoint            ) const
{GUCEF_TRACE;

    CUtf32String result( *this );

    if ( GUCEF_NULL == oldCodePoints )
        return result;

    for ( UInt32 i = 0u; i < result.m_length; ++i )
    {
        for ( UInt32 n = 0u; n < nrOfOldCodePoints; ++n )
        {
            if ( result.m_string[ i ] == oldCodePoints[ n ] )
                result.m_string[ i ] = newCodePoint;
        }
    }
    return result;
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32String::ReplaceSubstr( const CUtf32String& substr      ,
                              const CUtf32String& replacement ) const
{GUCEF_TRACE;

    CUtf32String testStr( *this );
    CUtf32String newStr;
    Int32 subStrIndex = 0;
    do
    {
        subStrIndex = testStr.HasSubstr( substr, true );
        if ( subStrIndex >= 0 )
        {
            newStr.Append( testStr.m_string, (UInt32)subStrIndex );
            newStr += replacement;
            testStr = testStr.CutChars( (UInt32)subStrIndex + substr.Length(), true );
        }
    }
    while ( subStrIndex >= 0 );
    newStr += testStr;
    return newStr;
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32String::ReplaceEnvelopingSubstr( const CUtf32String& envelopPrefix     ,
                                        const CUtf32String& envelopPostfix    ,
                                        const CUtf32String& newEnvelopPrefix  ,
                                        const CUtf32String& newEnvelopPostfix ) const
{GUCEF_TRACE;

    CUtf32String resultStr;

    Int32 startIndex = 0;
    Int32 envSegIndex = this->HasSubstr( envelopPrefix, startIndex, true );
    while ( envSegIndex >= 0 )
    {
        if ( startIndex < envSegIndex )
        {
            resultStr += this->SubstrFromRange( (UInt32)startIndex, (UInt32)envSegIndex );
        }

        envSegIndex += (Int32)envelopPrefix.Length();
        CUtf32String envelopedSegment = this->SubstrToSubstr( envelopPostfix, (UInt32)envSegIndex, true );
        resultStr += newEnvelopPrefix + envelopedSegment + newEnvelopPostfix;

        startIndex = envSegIndex + (Int32)envelopedSegment.Length() + (Int32)envelopPostfix.Length();
        envSegIndex = this->HasSubstr( envelopPrefix, startIndex, true );
    }

    resultStr += this->SubstrToIndex( (UInt32)startIndex, false );
    return resultStr;
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32String::CutEnvelopedSubstr( const CUtf32String& envelopPrefix  ,
                                   const CUtf32String& envelopPostfix ,
                                   UInt32 envelopedStrIndex           ) const
{GUCEF_TRACE;

    UInt32 currentEnvelopedStrIndex = 0u;
    Int32 startIndex = 0;
    Int32 envSegIndex = HasSubstr( envelopPrefix, startIndex, true );
    while ( envSegIndex >= 0 )
    {
        envSegIndex += (Int32)envelopPrefix.Length();
        envSegIndex = HasSubstr( envelopPostfix, envSegIndex, true );
        if ( envSegIndex >= 0 )
        {
            if ( currentEnvelopedStrIndex == envelopedStrIndex )
            {
                Int32 charCount = ( envSegIndex + (Int32)envelopPostfix.Length() ) - startIndex;
                return CutChars( (UInt32)charCount, true, (UInt32)startIndex );
            }
            ++currentEnvelopedStrIndex;
            startIndex = envSegIndex + (Int32)envelopPostfix.Length();
            envSegIndex = HasSubstr( envelopPrefix, startIndex, true );
        }
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32String::ReplaceSubStr( UInt32 startIndex             ,
                              UInt32 length                 ,
                              const CUtf32String& newSubstr ) const
{GUCEF_TRACE;

    if ( 0u == m_length ) return CUtf32String();
    if ( startIndex >= m_length ) startIndex = m_length - 1u;
    if ( startIndex + length >= m_length ) length = ( m_length - 1u ) - startIndex;

    CUtf32String prefix  = this->SubstrFromRange( 0u, startIndex );
    CUtf32String postfix = this->SubstrFromRange( startIndex + length + 1u, m_length );

    return prefix + newSubstr + postfix;
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32String::SubstrToChar( UInt32 searchCodePoint                 ,
                             UInt32 startIndex                      ,
                             bool frontToBack                       ,
                             bool returnEmptyIfCharNotFound         ) const
{GUCEF_TRACE;

    if ( startIndex < m_length )
    {
        if ( frontToBack )
        {
            for ( UInt32 i = startIndex; i < m_length; ++i )
            {
                if ( m_string[ i ] == searchCodePoint )
                {
                    CUtf32String substr;
                    substr.Set( m_string + startIndex, i - startIndex );
                    return substr;
                }
            }
            if ( returnEmptyIfCharNotFound )
                return CUtf32String();
            else
                return CUtf32String( *this );
        }

        for ( Int32 i = (Int32)startIndex; i >= 0; --i )
        {
            if ( m_string[ i ] == searchCodePoint )
            {
                CUtf32String substr;
                substr.Set( m_string + i + 1u, startIndex - (UInt32)i );
                return substr;
            }
        }
        if ( returnEmptyIfCharNotFound )
            return CUtf32String();
        else
            return CUtf32String( *this );
    }
    return CUtf32String();
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32String::SubstrToChar( UInt32 searchCodePoint                 ,
                             bool frontToBack                       ,
                             bool returnEmptyIfCharNotFound         ) const
{GUCEF_TRACE;

    if ( frontToBack )
    {
        return SubstrToChar( searchCodePoint, 0u, frontToBack, returnEmptyIfCharNotFound );
    }
    if ( m_length > 0u )
    {
        return SubstrToChar( searchCodePoint, m_length - 1u, frontToBack, returnEmptyIfCharNotFound );
    }
    return CUtf32String();
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32String::SubstrToIndex( UInt32 index,
                              bool frontToBack ) const
{GUCEF_TRACE;

    if ( !frontToBack )
    {
        if ( index >= m_length )
            return CUtf32String();
        return SubstrFromRange( index, m_length );
    }

    if ( index >= m_length )
        return CUtf32String( m_string, m_length );
    return SubstrFromRange( 0u, index );
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32String::SubstrFromRange( UInt32 startIndex,
                                UInt32 endIndex   ) const
{GUCEF_TRACE;

    if ( startIndex == endIndex )
        return CUtf32String();

    if ( startIndex > endIndex )
    {
        UInt32 swap = startIndex;
        startIndex = endIndex;
        endIndex = swap;
    }

    UInt32 maxEnd   = endIndex   > m_length ? m_length : endIndex;
    UInt32 maxStart = startIndex > m_length ? m_length : startIndex;

    if ( maxEnd >= maxStart )
    {
        return CUtf32String( m_string + maxStart, maxEnd - maxStart );
    }
    return CUtf32String();
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32String::SubstrToSubstr( const CUtf32String& searchstr,
                               bool frontToBack             ) const
{GUCEF_TRACE;

    if ( searchstr.Length() > m_length || 0u == searchstr.Length() || 0u == m_length )
    {
        return *this;
    }

    if ( frontToBack )
    {
        return SubstrToSubstr( searchstr, 0u, frontToBack );
    }
    return SubstrToSubstr( searchstr, m_length - 1u, frontToBack );
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32String::SubstrToSubstr( const CUtf32String& searchstr,
                               UInt32 startIndex            ,
                               bool frontToBack             ) const
{GUCEF_TRACE;

    UInt32 slen = searchstr.Length();
    if ( slen > m_length || 0u == slen || 0u == m_length )
    {
        return *this;
    }
    if ( startIndex >= m_length )
    {
        startIndex = m_length - 1u;
    }

    if ( frontToBack )
    {
        UInt32 max = m_length - slen;
        for ( UInt32 i = startIndex; i <= max; ++i )
        {
            if ( Utf32Memcmp( m_string + i, searchstr.m_string, slen ) == 0 )
            {
                return CUtf32String( m_string + startIndex, i - startIndex );
            }
        }
        return *this;
    }

    UInt32 max = startIndex - slen;
    for ( Int32 i = (Int32)max; i >= 0; --i )
    {
        if ( Utf32Memcmp( m_string + i, searchstr.m_string, slen ) == 0 )
        {
            CUtf32String substr;
            substr.Set( m_string + i + slen,
                        m_length - ( i + slen ) - ( ( m_length - 1u ) - startIndex ) );
            return substr;
        }
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32String::Trim( bool frontToBack ) const
{GUCEF_TRACE;

    if ( m_length > 0u )
    {
        if ( frontToBack )
        {
            UInt32 charsToCut = 0u;
            while ( charsToCut < m_length &&
                    ( m_string[ charsToCut ] == 0x0020u ||  /* space */
                      m_string[ charsToCut ] == 0x0009u ) ) /* tab   */
            {
                ++charsToCut;
            }
            return CutChars( charsToCut, frontToBack );
        }

        UInt32 i = m_length - 1u;
        UInt32 charsToCut = 0u;
        while ( i > 0u &&
                ( m_string[ i ] == 0x0020u ||
                  m_string[ i ] == 0x0009u ) )
        {
            --i;
            ++charsToCut;
        }
        /* Handle index 0 explicitly */
        if ( i == 0u && ( m_string[ 0u ] == 0x0020u || m_string[ 0u ] == 0x0009u ) )
            ++charsToCut;

        return CutChars( charsToCut, frontToBack );
    }
    return CUtf32String( *this );
}

/*-------------------------------------------------------------------------*/

Int32
CUtf32String::HasChar( UInt32 searchCodePoint,
                        UInt32 startIndex     ,
                        bool frontToBack      ) const
{GUCEF_TRACE;

    if ( frontToBack )
    {
        for ( UInt32 i = startIndex; i < m_length; ++i )
        {
            if ( m_string[ i ] == searchCodePoint )
                return (Int32)i;
        }
        return -1;
    }

    UInt32 max = startIndex > m_length - 1u ? m_length - 1u : startIndex;
    for ( Int32 i = (Int32)max; i >= 0; --i )
    {
        if ( m_string[ i ] == searchCodePoint )
            return i;
    }
    return -1;
}

/*-------------------------------------------------------------------------*/

Int32
CUtf32String::HasChar( UInt32 searchCodePoint,
                        bool frontToBack      ) const
{GUCEF_TRACE;

    if ( frontToBack )
    {
        return HasChar( searchCodePoint, 0u, frontToBack );
    }
    if ( m_length > 0u )
    {
        return HasChar( searchCodePoint, m_length - 1u, frontToBack );
    }
    return -1;
}

/*-------------------------------------------------------------------------*/

Int32
CUtf32String::HasSubstr( const CUtf32String& substr,
                          Int32 startIndex          ,
                          bool startfront           ) const
{GUCEF_TRACE;

    if ( startIndex < 0 || (UInt32)startIndex > m_length ) return -1;

    UInt32 subStrLength = substr.Length();
    if ( subStrLength > m_length )
        return -1;

    if ( startfront )
    {
        UInt32 max = m_length - subStrLength;
        for ( UInt32 i = (UInt32)startIndex; i <= max; ++i )
        {
            if ( Utf32Memcmp( m_string + i, substr.m_string, subStrLength ) == 0 )
                return (Int32)i;
        }
        return -1;
    }

    Int32 max = (Int32)( m_length - subStrLength );
    if ( startIndex - (Int32)subStrLength < max )
    {
        max = startIndex - (Int32)subStrLength;
    }
    for ( Int32 i = max; i >= 0; --i )
    {
        if ( Utf32Memcmp( m_string + i, substr.m_string, subStrLength ) == 0 )
            return i;
    }
    return -1;
}

/*-------------------------------------------------------------------------*/

Int32
CUtf32String::HasSubstr( const CUtf32String& substr,
                          bool startfront           ) const
{GUCEF_TRACE;

    if ( startfront )
    {
        return HasSubstr( substr, 0, startfront );
    }
    if ( m_length > 0u )
    {
        return HasSubstr( substr, (Int32)m_length, startfront );
    }
    return -1;
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf32String::GetCharacterCount( UInt32 searchCodePoint ) const
{GUCEF_TRACE;

    UInt32 count = 0u;
    for ( UInt32 i = 0u; i < m_length; ++i )
    {
        if ( m_string[ i ] == searchCodePoint )
            ++count;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf32String::GetCharactersCount( const UInt32* searchCodePoints,
                                   UInt32 nrOfSearchCodePoints   ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == searchCodePoints )
        return 0u;

    UInt32 count = 0u;
    for ( UInt32 i = 0u; i < m_length; ++i )
    {
        for ( UInt32 n = 0u; n < nrOfSearchCodePoints; ++n )
        {
            if ( m_string[ i ] == searchCodePoints[ n ] )
                ++count;
        }
    }
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf32String::GetNonMatchCharactersCount( const UInt32* searchCodePoints,
                                           UInt32 nrOfSearchCodePoints   ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == searchCodePoints )
        return 0u;

    UInt32 count = 0u;
    for ( UInt32 i = 0u; i < m_length; ++i )
    {
        for ( UInt32 n = 0u; n < nrOfSearchCodePoints; ++n )
        {
            if ( m_string[ i ] != searchCodePoints[ n ] )
                ++count;
        }
    }
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf32String::GetCharacterRepeatCount( UInt32 searchCodePoint ) const
{GUCEF_TRACE;

    UInt32 repeatCount = 0u;
    for ( UInt32 i = 0u; i < m_length; ++i )
    {
        if ( m_string[ i ] == searchCodePoint )
        {
            ++i;
            for ( ; i < m_length; ++i )
            {
                if ( m_string[ i ] == searchCodePoint )
                    ++repeatCount;
                else
                    break;
            }
        }
    }
    return repeatCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf32String::FindMaxSubstrEquality( const CUtf32String& searchStr,
                                      UInt32 startOffset           ,
                                      bool startFront              ,
                                      bool isCaseSensitive         ) const
{GUCEF_TRACE;

    if ( (Int32)m_length - (Int32)startOffset > 0 )
    {
        CUtf32String lowercaseSearchStrStorage;
        CUtf32String lowercaseThisStrStorage;
        const CUtf32String* theSearchStr = &searchStr;
        const CUtf32String* thisStr = this;

        if ( !isCaseSensitive )
        {
            lowercaseSearchStrStorage = searchStr.Lowercase();
            theSearchStr = &lowercaseSearchStrStorage;
            lowercaseThisStrStorage = Lowercase();
            thisStr = &lowercaseThisStrStorage;
        }

        UInt32 max = theSearchStr->Length();
        if ( thisStr->m_length - startOffset < max )
            max = thisStr->m_length - startOffset;

        if ( startFront )
        {
            UInt32 subLength = 1u;
            while ( subLength <= max )
            {
                if ( Utf32Memcmp( thisStr->m_string + startOffset,
                                  theSearchStr->m_string, subLength ) != 0 )
                {
                    return subLength - 1u;
                }
                ++subLength;
            }
            return subLength - 1u;
        }
        else
        {
            const UInt32* str      = thisStr->m_string + thisStr->m_length - startOffset;
            const UInt32* otherStr = theSearchStr->m_string + theSearchStr->m_length;

            UInt32 subLength = 1u;
            while ( subLength <= max )
            {
                if ( Utf32Memcmp( str - subLength, otherStr - subLength, subLength ) != 0 )
                {
                    return subLength - 1u;
                }
                ++subLength;
            }
            return subLength - 1u;
        }
    }
    return 0u;
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32String::CutChars( UInt32 charcount   ,
                         bool startfront    ,
                         UInt32 startOffset ) const
{GUCEF_TRACE;

    if ( startfront )
    {
        if ( 0u == startOffset )
        {
            if ( charcount < m_length )
            {
                return CUtf32String( m_string + charcount, m_length - charcount );
            }
            return CUtf32String();
        }

        return CUtf32String( m_string, startOffset ) +
               CUtf32String( m_string + startOffset + charcount, m_length - startOffset - charcount );
    }

    if ( 0u == startOffset )
    {
        if ( charcount < m_length )
        {
            return CUtf32String( m_string, m_length - charcount );
        }
        return CUtf32String();
    }

    return CUtf32String( m_string, m_length - startOffset - charcount ) +
           CUtf32String( m_string + m_length - charcount, charcount );
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32String::RemoveChar( UInt32 codePointToRemove ) const
{GUCEF_TRACE;

    CUtf32String newStr;
    for ( UInt32 i = 0u; i < m_length; ++i )
    {
        if ( m_string[ i ] != codePointToRemove )
        {
            newStr += m_string[ i ];
        }
    }
    return newStr;
}

/*-------------------------------------------------------------------------*/

bool
CUtf32String::HasRepeatingChar( UInt32 codePointToCheck ) const
{GUCEF_TRACE;

    for ( UInt32 i = 0u; i < m_length; ++i )
    {
        if ( m_string[ i ] == codePointToCheck )
        {
            if ( ( i + 1u < m_length ) &&
                 ( m_string[ i + 1u ] == codePointToCheck ) )
            {
                return true;
            }
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32String::CompactRepeatingChar( UInt32 codePointToCompact ) const
{GUCEF_TRACE;

    UInt32* buf = GUCEF_NEW UInt32[ m_length + 1u ];
    assert( GUCEF_NULL != buf );
    UInt32 newLen = 0u;

    for ( UInt32 i = 0u; i < m_length; ++i )
    {
        buf[ newLen ] = m_string[ i ];
        ++newLen;

        if ( m_string[ i ] == codePointToCompact )
        {
            while ( ( i + 1u < m_length ) &&
                    ( m_string[ i + 1u ] == codePointToCompact ) )
            {
                ++i;
            }
        }
    }

    CUtf32String result( buf, newLen );
    GUCEF_DELETE []buf;
    return result;
}

/*-------------------------------------------------------------------------*/

CUtf32String::StringVector
CUtf32String::ParseElements( UInt32 separator,
                              bool addEmptyElements ) const
{GUCEF_TRACE;

    return ParseElements( m_string, m_length, separator, addEmptyElements );
}

/*-------------------------------------------------------------------------*/

CUtf32String::StringSet
CUtf32String::ParseUniqueElements( UInt32 separator,
                                    bool addEmptyElements ) const
{GUCEF_TRACE;

    return ParseUniqueElements( m_string, m_length, separator, addEmptyElements );
}

/*-------------------------------------------------------------------------*/

CUtf32String::StringVector
CUtf32String::ParseElements( const UInt32* bufferPtr ,
                              UInt32 bufferSize       ,
                              UInt32 separator        ,
                              bool addEmptyElements   )
{GUCEF_TRACE;

    if ( bufferSize > 0u )
    {
        StringVector list;
        CUtf32String entry;
        UInt32 last = 0u;
        for ( UInt32 i = 0u; i < bufferSize; ++i )
        {
            if ( bufferPtr[ i ] == separator )
            {
                UInt32 strLen = i - last;
                if ( ( 0u == strLen && addEmptyElements ) || strLen > 0u )
                {
                    entry.Set( bufferPtr + last, strLen );
                    list.push_back( entry );
                }
                last = i + 1u;
            }
        }

        UInt32 strLen = bufferSize - last;
        if ( ( 0u == strLen && addEmptyElements ) || strLen > 0u )
        {
            entry.Set( bufferPtr + last, strLen );
            list.push_back( entry );
        }
        return list;
    }
    return StringVector();
}

/*-------------------------------------------------------------------------*/

CUtf32String::StringSet
CUtf32String::ParseUniqueElements( const UInt32* bufferPtr ,
                                    UInt32 bufferSize       ,
                                    UInt32 separator        ,
                                    bool addEmptyElements   )
{GUCEF_TRACE;

    if ( bufferSize > 0u )
    {
        StringSet list;
        CUtf32String entry;
        UInt32 last = 0u;
        for ( UInt32 i = 0u; i < bufferSize; ++i )
        {
            if ( bufferPtr[ i ] == separator )
            {
                UInt32 strLen = i - last;
                if ( ( 0u == strLen && addEmptyElements ) || strLen > 0u )
                {
                    entry.Set( bufferPtr + last, strLen );
                    list.insert( entry );
                }
                last = i + 1u;
            }
        }

        UInt32 strLen = bufferSize - last;
        if ( ( 0u == strLen && addEmptyElements ) || strLen > 0u )
        {
            entry.Set( bufferPtr + last, strLen );
            list.insert( entry );
        }
        return list;
    }
    return StringSet();
}

/*-------------------------------------------------------------------------*/

bool
CUtf32String::WildcardEquals( const CUtf32String& strWithWildcards,
                               UInt32 wildCardToken               ,
                               bool caseSensitive                 ,
                               bool biDirectional                 ) const
{GUCEF_TRACE;

    if ( biDirectional )
    {
        if ( strWithWildcards.WildcardEquals( *this, wildCardToken, caseSensitive, false ) )
            return true;
    }

    if ( strWithWildcards == wildCardToken || *this == wildCardToken )
        return true;

    CUtf32String::StringVector segs = strWithWildcards.ParseElements( wildCardToken, false );
    Int32 lastSeg = 0;
    CUtf32String::StringVector::iterator i = segs.begin();
    while ( i != segs.end() )
    {
        lastSeg = HasSubstr( (*i), lastSeg, true );
        if ( 0 > lastSeg )
            return false;
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUtf32String::WildcardEquals( const StringSet& strsWithWildcards,
                               UInt32 wildCardToken              ,
                               bool caseSensitive                ,
                               bool biDirectional                ) const
{GUCEF_TRACE;

    if ( strsWithWildcards.empty() )
        return WildcardEquals( Empty, wildCardToken, caseSensitive, biDirectional );

    StringSet::const_iterator i = strsWithWildcards.begin();
    while ( i != strsWithWildcards.end() )
    {
        if ( WildcardEquals( (*i), wildCardToken, caseSensitive, biDirectional ) )
            return true;
        ++i;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CUtf32String::Equals( const CUtf32String& otherStr,
                       bool caseSensitive           ) const
{GUCEF_TRACE;

    if ( caseSensitive )
    {
        return *this == otherStr;
    }
    else
    {
        return Lowercase() == otherStr.Lowercase();
    }
}

/*-------------------------------------------------------------------------*/

bool
CUtf32String::NotEquals( const CUtf32String& otherStr,
                          bool caseSensitive           ) const
{GUCEF_TRACE;

    if ( caseSensitive )
    {
        return *this != otherStr;
    }
    else
    {
        return Lowercase() != otherStr.Lowercase();
    }
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32String::Combine( const StringVector& elements, UInt32 separator ) const
{GUCEF_TRACE;

    CUtf32String currentStr( *this );
    if ( !currentStr.IsNULLOrEmpty() && !elements.empty() )
        currentStr += separator;

    StringVector::const_iterator i = elements.begin();
    while ( i != elements.end() )
    {
        if ( !(*i).IsNULLOrEmpty() )
            currentStr += (*i);
        ++i;
        if ( i != elements.end() && !(*i).IsNULLOrEmpty() )
            currentStr += separator;
    }
    return currentStr;
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32String::Combine( const StringSet& elements, UInt32 separator ) const
{GUCEF_TRACE;

    CUtf32String currentStr( *this );
    if ( !currentStr.IsNULLOrEmpty() && !elements.empty() )
        currentStr += separator;

    StringSet::const_iterator i = elements.begin();
    while ( i != elements.end() )
    {
        if ( !(*i).IsNULLOrEmpty() )
            currentStr += (*i);
        ++i;
        if ( i != elements.end() && !(*i).IsNULLOrEmpty() )
            currentStr += separator;
    }
    return currentStr;
}

/*-------------------------------------------------------------------------*/

bool
CUtf32String::IsFormattingValid( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_string )
    {
        for ( UInt32 i = 0u; i < m_length; ++i )
        {
            UInt32 cp = m_string[ i ];
            /* Unicode scalar values: 0x0000-0xD7FF and 0xE000-0x10FFFF */
            if ( cp >= 0xD800u && cp <= 0xDFFFu )
                return false; /* surrogate half */
            if ( cp > 0x10FFFFu )
                return false; /* beyond Unicode range */
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32String::ReadLine( CIOAccess* io )
{GUCEF_TRACE;

    if ( GUCEF_NULL != io )
    {
        CUtf32String resultStr;
        while ( !io->Eof() )
        {
            /* Read 4 bytes as a native-endian UTF-32 code point */
            UInt32 cp = 0u;
            if ( sizeof( UInt32 ) != io->Read( &cp, sizeof( UInt32 ), 1 ) )
                break;

            if ( cp == 0u    || cp == static_cast<UInt32>('\n') ||
                 cp == static_cast<UInt32>('\r') )
            {
                break;
            }

            resultStr += cp;
        }
        return resultStr;
    }
    return CUtf32String();
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32String::ReadString( CIOAccess* io )
{GUCEF_TRACE;

    if ( GUCEF_NULL != io )
    {
        CUtf32String resultStr;
        while ( !io->Eof() )
        {
            UInt32 cp = 0u;
            if ( sizeof( UInt32 ) != io->Read( &cp, sizeof( UInt32 ), 1 ) )
                break;

            if ( cp == 0u                            ||
                 cp == static_cast<UInt32>('\n')     ||
                 cp == static_cast<UInt32>('\r')     ||
                 cp == static_cast<UInt32>('\t')     ||
                 cp == static_cast<UInt32>(' ')       )
            {
                break;
            }

            resultStr += cp;
        }
        return resultStr;
    }
    return CUtf32String();
}

/*-------------------------------------------------------------------------*/

bool
operator==( const char* lhs, const CUtf32String& rhs )
{GUCEF_TRACE;

    return rhs == lhs;
}

/*-------------------------------------------------------------------------*/

bool
operator!=( const char* lhs, const CUtf32String& rhs )
{GUCEF_TRACE;

    return rhs != lhs;
}

/*-------------------------------------------------------------------------*/

CUtf32String
operator+( const char* lhs, const CUtf32String& rhs )
{GUCEF_TRACE;

    return CUtf32String( lhs ) + rhs;
}

/*-------------------------------------------------------------------------*/

CUtf32String
operator+( UInt32 lhs, const CUtf32String& rhs )
{GUCEF_TRACE;

    return CUtf32String( lhs ) + rhs;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
