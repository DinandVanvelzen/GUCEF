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

#include "gucefCORE_CUtf16String.h"

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

const CUtf16String CUtf16String::Empty;
const CUtf16String::StringVector CUtf16String::EmptyStringVector;
const CUtf16String::StringSet    CUtf16String::EmptyStringSet;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/* --- UTF-16 low-level helpers ------------------------------------------ */

static inline bool
IsHighSurrogate( UInt16 cu )
{
    return ( cu >= 0xD800u ) && ( cu <= 0xDBFFu );
}

/*-------------------------------------------------------------------------*/

static inline bool
IsLowSurrogate( UInt16 cu )
{
    return ( cu >= 0xDC00u ) && ( cu <= 0xDFFFu );
}

/*-------------------------------------------------------------------------*/

/*
 *  Decodes one UTF-16 code point from *ptr, advances ptr past the code unit(s).
 *  Handles surrogate pairs.  Does NOT check for null terminator.
 */
static inline UInt32
Utf16DecodeOne( const UInt16*& ptr )
{
    UInt16 cu0 = *ptr;
    if ( IsHighSurrogate( cu0 ) )
    {
        UInt16 cu1 = *( ptr + 1 );
        if ( IsLowSurrogate( cu1 ) )
        {
            ptr += 2;
            return 0x10000u
                + ( ( static_cast<UInt32>( cu0 ) - 0xD800u ) << 10 )
                + ( static_cast<UInt32>( cu1 )  - 0xDC00u );
        }
    }
    ++ptr;
    return static_cast<UInt32>( cu0 );
}

/*-------------------------------------------------------------------------*/

/*
 *  Encodes one UTF-32 code point into buf (must have >= 2 slots).
 *  Returns number of code units written (1 or 2).
 */
static inline UInt32
Utf16EncodeOne( UInt32 cp, UInt16* buf )
{
    if ( cp < 0x10000u )
    {
        buf[ 0 ] = static_cast<UInt16>( cp );
        return 1u;
    }
    cp -= 0x10000u;
    buf[ 0 ] = static_cast<UInt16>( 0xD800u | ( cp >> 10 ) );
    buf[ 1 ] = static_cast<UInt16>( 0xDC00u | ( cp & 0x3FFu ) );
    return 2u;
}

/*-------------------------------------------------------------------------*/

/*
 *  Returns the number of UInt16 code units in the null-terminated buffer
 *  (not counting the null terminator).
 */
static UInt32
Utf16Strlen( const UInt16* str )
{
    if ( GUCEF_NULL == str )
        return 0u;
    UInt32 n = 0u;
    while ( 0u != str[ n ] )
        ++n;
    return n;
}

/*-------------------------------------------------------------------------*/

/*
 *  Counts code points in exactly codeUnits code units (stops at null or limit).
 */
static UInt32
Utf16CountCodePoints( const UInt16* buf, UInt32 codeUnits )
{
    if ( GUCEF_NULL == buf || 0u == codeUnits )
        return 0u;
    UInt32 count = 0u;
    UInt32 i = 0u;
    while ( i < codeUnits && 0u != buf[ i ] )
    {
        if ( IsHighSurrogate( buf[ i ] ) && ( i + 1u ) < codeUnits && IsLowSurrogate( buf[ i + 1u ] ) )
            i += 2u;
        else
            ++i;
        ++count;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

/*
 *  Counts code points in a null-terminated UTF-16 buffer.
 */
static UInt32
Utf16CountCodePointsNullTerm( const UInt16* buf )
{
    if ( GUCEF_NULL == buf )
        return 0u;
    UInt32 count = 0u;
    const UInt16* p = buf;
    while ( 0u != *p )
    {
        if ( IsHighSurrogate( *p ) && IsLowSurrogate( *( p + 1 ) ) )
            p += 2;
        else
            ++p;
        ++count;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

/*
 *  Lexicographic comparison of two null-terminated UTF-16 strings by code
 *  point value.  Returns < 0, 0, or > 0.
 */
static int
Utf16Strcmp( const UInt16* a, const UInt16* b )
{
    if ( a == b )
        return 0;
    if ( GUCEF_NULL == a )
        return -1;
    if ( GUCEF_NULL == b )
        return 1;
    for (;;)
    {
        const UInt16* pa = a;
        const UInt16* pb = b;
        UInt32 cpa = Utf16DecodeOne( pa );
        UInt32 cpb = Utf16DecodeOne( pb );
        if ( cpa < cpb ) return -1;
        if ( cpa > cpb ) return  1;
        if ( 0u == cpa ) return  0;
        a = pa;
        b = pb;
    }
}

/*-------------------------------------------------------------------------*/

/*
 *  Compares exactly codeUnits code units (raw UInt16 comparison).
 */
static int
Utf16Memcmp( const UInt16* a, const UInt16* b, UInt32 codeUnits )
{
    for ( UInt32 i = 0u; i < codeUnits; ++i )
    {
        if ( a[ i ] < b[ i ] ) return -1;
        if ( a[ i ] > b[ i ] ) return  1;
    }
    return 0;
}

/* --- UTF-8 decode helpers (same as CUtf32String) ----------------------- */

static inline UInt32
Utf8SeqLen( unsigned char leadByte )
{
    if ( leadByte < 0x80u )              return 1u;
    if ( ( leadByte & 0xE0u ) == 0xC0u ) return 2u;
    if ( ( leadByte & 0xF0u ) == 0xE0u ) return 3u;
    if ( ( leadByte & 0xF8u ) == 0xF0u ) return 4u;
    return 1u;
}

/*-------------------------------------------------------------------------*/

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
        cp = static_cast<UInt32>( lead );
        ++ptr;
    }
    return cp;
}

/*-------------------------------------------------------------------------*/

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
 *  Converts codeUnits UTF-16 code units to a UTF-8 std::string.
 */
static std::string
Utf16BufferToUtf8Str( const UInt16* buf, UInt32 codeUnits )
{
    std::string result;
    if ( GUCEF_NULL == buf || 0u == codeUnits )
        return result;
    result.reserve( codeUnits );
    const UInt16* p   = buf;
    const UInt16* end = buf + codeUnits;
    while ( p < end && 0u != *p )
    {
        UInt32 cp = Utf16DecodeOne( p );
        Utf32CpToUtf8( cp, result );
    }
    return result;
}

/* --- UTF-8 count helpers ----------------------------------------------- */

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

/* --- UTF-8 → UTF-16 allocation helpers --------------------------------- */

/*
 *  Allocates a null-terminated UTF-16 buffer from a null-terminated UTF-8 string.
 *  outCodeUnits = number of UInt16 code units (excluding null).
 *  outLength    = number of Unicode code points.
 *  Returns GUCEF_NULL if src is NULL or empty.
 */
static UInt16*
Utf8ToUtf16Alloc( const char* src, UInt32& outCodeUnits, UInt32& outLength )
{
    outCodeUnits = 0u;
    outLength    = 0u;
    if ( GUCEF_NULL == src || '\0' == *src )
        return GUCEF_NULL;

    /* First pass: count code points and required code units */
    UInt32 cpCount = 0u;
    UInt32 cuCount = 0u;
    const char* p = src;
    while ( '\0' != *p )
    {
        UInt32 cp = Utf8DecodeOne( p );
        ++cpCount;
        cuCount += ( cp >= 0x10000u ) ? 2u : 1u;
    }
    if ( 0u == cpCount )
        return GUCEF_NULL;

    UInt16* buf = GUCEF_NEW UInt16[ cuCount + 1u ];
    assert( GUCEF_NULL != buf );

    /* Second pass: encode */
    p = src;
    UInt32 out = 0u;
    for ( UInt32 i = 0u; i < cpCount; ++i )
    {
        UInt32 cp = Utf8DecodeOne( p );
        out += Utf16EncodeOne( cp, buf + out );
    }
    buf[ cuCount ] = 0u;
    outCodeUnits = cuCount;
    outLength    = cpCount;
    return buf;
}

/*-------------------------------------------------------------------------*/

/*
 *  Same as above but reads exactly byteSize bytes of UTF-8.
 */
static UInt16*
Utf8BytesToUtf16Alloc( const char* src, UInt32 byteSize,
                        UInt32& outCodeUnits, UInt32& outLength )
{
    outCodeUnits = 0u;
    outLength    = 0u;
    if ( GUCEF_NULL == src || 0u == byteSize )
        return GUCEF_NULL;

    /* First pass */
    UInt32 cpCount = 0u;
    UInt32 cuCount = 0u;
    const char* p   = src;
    const char* end = src + byteSize;
    while ( p < end && '\0' != *p )
    {
        UInt32 cp = Utf8DecodeOne( p );
        ++cpCount;
        cuCount += ( cp >= 0x10000u ) ? 2u : 1u;
    }
    if ( 0u == cpCount )
        return GUCEF_NULL;

    UInt16* buf = GUCEF_NEW UInt16[ cuCount + 1u ];
    assert( GUCEF_NULL != buf );

    /* Second pass */
    p = src;
    UInt32 out = 0u;
    for ( UInt32 i = 0u; i < cpCount; ++i )
    {
        UInt32 cp = Utf8DecodeOne( p );
        out += Utf16EncodeOne( cp, buf + out );
    }
    buf[ cuCount ] = 0u;
    outCodeUnits = cuCount;
    outLength    = cpCount;
    return buf;
}

/*-------------------------------------------------------------------------*/

/*
 *  Allocates a UTF-16 buffer from a wchar_t string.
 *  On Windows sizeof(wchar_t)==2 (already UTF-16); on Linux it is 4 (UTF-32).
 *  maxLen is ignored when reexamine==false and src is null-terminated.
 */
static UInt16*
WcharToUtf16Alloc( const wchar_t* src, UInt32 maxLen, bool reexamine,
                   UInt32& outCodeUnits, UInt32& outLength )
{
    outCodeUnits = 0u;
    outLength    = 0u;
    if ( GUCEF_NULL == src )
        return GUCEF_NULL;

    if ( 2u == sizeof( wchar_t ) )
    {
        /* Windows: wchar_t is already a UTF-16 code unit */
        const UInt16* ws = reinterpret_cast<const UInt16*>( src );
        UInt32 wlen = 0u;
        if ( reexamine )
        {
            while ( wlen < maxLen && 0u != ws[ wlen ] ) ++wlen;
        }
        else
        {
            wlen = Utf16Strlen( ws );
        }
        if ( 0u == wlen )
            return GUCEF_NULL;

        UInt16* buf = GUCEF_NEW UInt16[ wlen + 1u ];
        assert( GUCEF_NULL != buf );
        memcpy( buf, ws, wlen * sizeof( UInt16 ) );
        buf[ wlen ] = 0u;
        outCodeUnits = wlen;
        outLength    = Utf16CountCodePoints( buf, wlen );
        return buf;
    }
    else
    {
        /* Linux/Mac: wchar_t is UTF-32 */
        UInt32 wlen = 0u;
        if ( reexamine )
        {
            while ( wlen < maxLen && 0 != src[ wlen ] ) ++wlen;
        }
        else
        {
            while ( 0 != src[ wlen ] ) ++wlen;
        }
        if ( 0u == wlen )
            return GUCEF_NULL;

        /* Count required code units */
        UInt32 cuCount = 0u;
        for ( UInt32 i = 0u; i < wlen; ++i )
            cuCount += ( static_cast<UInt32>( src[ i ] ) >= 0x10000u ) ? 2u : 1u;

        UInt16* buf = GUCEF_NEW UInt16[ cuCount + 1u ];
        assert( GUCEF_NULL != buf );
        UInt32 out = 0u;
        for ( UInt32 i = 0u; i < wlen; ++i )
            out += Utf16EncodeOne( static_cast<UInt32>( src[ i ] ), buf + out );
        buf[ cuCount ] = 0u;
        outCodeUnits = cuCount;
        outLength    = wlen;
        return buf;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CUtf16String::CUtf16String( void )
    : m_string( GUCEF_NULL )
    , m_length( 0u )
    , m_byteSize( 0u )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CUtf16String::CUtf16String( const CUtf16String& src )
    : m_string( GUCEF_NULL )
    , m_length( 0u )
    , m_byteSize( 0u )
{GUCEF_TRACE;
    if ( src.m_byteSize > 0u && GUCEF_NULL != src.m_string )
    {
        m_string = GUCEF_NEW UInt16[ src.m_byteSize / sizeof( UInt16 ) ];
        assert( GUCEF_NULL != m_string );
        memcpy( m_string, src.m_string, src.m_byteSize );
        m_length   = src.m_length;
        m_byteSize = src.m_byteSize;
    }
}

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_RVALUE_REFERENCES_SUPPORTED

CUtf16String::CUtf16String( CUtf16String&& src ) GUCEF_NOEXCEPT
    : m_string( src.m_string )
    , m_length( src.m_length )
    , m_byteSize( src.m_byteSize )
{GUCEF_TRACE;
    src.m_string   = GUCEF_NULL;
    src.m_length   = 0u;
    src.m_byteSize = 0u;
}

#endif /* GUCEF_RVALUE_REFERENCES_SUPPORTED */

/*-------------------------------------------------------------------------*/

CUtf16String::CUtf16String( const CUtf8String& src )
    : m_string( GUCEF_NULL )
    , m_length( 0u )
    , m_byteSize( 0u )
{GUCEF_TRACE;
    const char* utf8Buf = src.C_String();
    if ( GUCEF_NULL != utf8Buf )
    {
        UInt32 cuCount = 0u;
        m_string = Utf8BytesToUtf16Alloc( utf8Buf, src.ByteSize(), cuCount, m_length );
        if ( GUCEF_NULL != m_string )
            m_byteSize = ( cuCount + 1u ) * sizeof( UInt16 );
    }
}

/*-------------------------------------------------------------------------*/

CUtf16String::CUtf16String( const CAsciiString& src )
    : m_string( GUCEF_NULL )
    , m_length( 0u )
    , m_byteSize( 0u )
{GUCEF_TRACE;
    UInt32 srcLen = src.Length();
    if ( srcLen > 0u )
    {
        const char* ascii = src.C_String();
        m_string = GUCEF_NEW UInt16[ srcLen + 1u ];
        assert( GUCEF_NULL != m_string );
        for ( UInt32 i = 0u; i < srcLen; ++i )
            m_string[ i ] = static_cast<UInt16>( static_cast<unsigned char>( ascii[ i ] ) );
        m_string[ srcLen ] = 0u;
        m_length   = srcLen;
        m_byteSize = ( srcLen + 1u ) * sizeof( UInt16 );
    }
}

/*-------------------------------------------------------------------------*/

CUtf16String::CUtf16String( const std::string& src )
    : m_string( GUCEF_NULL )
    , m_length( 0u )
    , m_byteSize( 0u )
{GUCEF_TRACE;
    if ( !src.empty() )
    {
        UInt32 cuCount = 0u;
        m_string = Utf8BytesToUtf16Alloc( src.c_str(), (UInt32) src.size(), cuCount, m_length );
        if ( GUCEF_NULL != m_string )
            m_byteSize = ( cuCount + 1u ) * sizeof( UInt16 );
    }
}

/*-------------------------------------------------------------------------*/

CUtf16String::CUtf16String( const char* src )
    : m_string( GUCEF_NULL )
    , m_length( 0u )
    , m_byteSize( 0u )
{GUCEF_TRACE;
    if ( GUCEF_NULL != src && '\0' != *src )
    {
        UInt32 cuCount = 0u;
        m_string = Utf8ToUtf16Alloc( src, cuCount, m_length );
        if ( GUCEF_NULL != m_string )
            m_byteSize = ( cuCount + 1u ) * sizeof( UInt16 );
    }
}

/*-------------------------------------------------------------------------*/

CUtf16String::CUtf16String( const char* src              ,
                             UInt32 byteSize              ,
                             bool reexamineByteSize       )
    : m_string( GUCEF_NULL )
    , m_length( 0u )
    , m_byteSize( 0u )
{GUCEF_TRACE;
    if ( GUCEF_NULL != src && 0u < byteSize )
    {
        if ( reexamineByteSize )
        {
            UInt32 actualLen = 0u;
            while ( actualLen < byteSize && '\0' != src[ actualLen ] )
                ++actualLen;
            byteSize = actualLen;
        }
        if ( 0u < byteSize )
        {
            UInt32 cuCount = 0u;
            m_string = Utf8BytesToUtf16Alloc( src, byteSize, cuCount, m_length );
            if ( GUCEF_NULL != m_string )
                m_byteSize = ( cuCount + 1u ) * sizeof( UInt16 );
        }
    }
}

/*-------------------------------------------------------------------------*/

CUtf16String::CUtf16String( const char* src              ,
                             UInt32 byteSize              ,
                             UInt32 lengthInCodePoints    ,
                             bool reexamineByteSize       )
    : m_string( GUCEF_NULL )
    , m_length( 0u )
    , m_byteSize( 0u )
{GUCEF_TRACE;
    /* lengthInCodePoints is a hint; just use the byte-size constructor */
    if ( GUCEF_NULL != src && 0u < byteSize )
    {
        if ( reexamineByteSize )
        {
            UInt32 actualLen = 0u;
            while ( actualLen < byteSize && '\0' != src[ actualLen ] )
                ++actualLen;
            byteSize = actualLen;
        }
        if ( 0u < byteSize )
        {
            UInt32 cuCount = 0u;
            m_string = Utf8BytesToUtf16Alloc( src, byteSize, cuCount, m_length );
            if ( GUCEF_NULL != m_string )
                m_byteSize = ( cuCount + 1u ) * sizeof( UInt16 );
        }
    }
    (void) lengthInCodePoints;
}

/*-------------------------------------------------------------------------*/

CUtf16String::CUtf16String( const UInt16* src )
    : m_string( GUCEF_NULL )
    , m_length( 0u )
    , m_byteSize( 0u )
{GUCEF_TRACE;
    if ( GUCEF_NULL != src && 0u != *src )
    {
        UInt32 cuCount = Utf16Strlen( src );
        m_string = GUCEF_NEW UInt16[ cuCount + 1u ];
        assert( GUCEF_NULL != m_string );
        memcpy( m_string, src, ( cuCount + 1u ) * sizeof( UInt16 ) );
        m_length   = Utf16CountCodePoints( m_string, cuCount );
        m_byteSize = ( cuCount + 1u ) * sizeof( UInt16 );
    }
}

/*-------------------------------------------------------------------------*/

CUtf16String::CUtf16String( const UInt16* src   ,
                             UInt32 codeUnits    ,
                             bool reexamine      )
    : m_string( GUCEF_NULL )
    , m_length( 0u )
    , m_byteSize( 0u )
{GUCEF_TRACE;
    if ( GUCEF_NULL != src && 0u < codeUnits )
    {
        if ( reexamine )
        {
            UInt32 actual = 0u;
            while ( actual < codeUnits && 0u != src[ actual ] ) ++actual;
            codeUnits = actual;
        }
        if ( 0u < codeUnits )
        {
            m_string = GUCEF_NEW UInt16[ codeUnits + 1u ];
            assert( GUCEF_NULL != m_string );
            memcpy( m_string, src, codeUnits * sizeof( UInt16 ) );
            m_string[ codeUnits ] = 0u;
            m_length   = Utf16CountCodePoints( m_string, codeUnits );
            m_byteSize = ( codeUnits + 1u ) * sizeof( UInt16 );
        }
    }
}

/*-------------------------------------------------------------------------*/

CUtf16String::CUtf16String( const UInt16* src              ,
                             UInt32 codeUnits               ,
                             UInt32 lengthInCodePoints      ,
                             bool reexamine                 )
    : m_string( GUCEF_NULL )
    , m_length( 0u )
    , m_byteSize( 0u )
{GUCEF_TRACE;
    if ( GUCEF_NULL != src && 0u < codeUnits )
    {
        if ( reexamine )
        {
            UInt32 actual = 0u;
            while ( actual < codeUnits && 0u != src[ actual ] ) ++actual;
            codeUnits = actual;
        }
        if ( 0u < codeUnits )
        {
            m_string = GUCEF_NEW UInt16[ codeUnits + 1u ];
            assert( GUCEF_NULL != m_string );
            memcpy( m_string, src, codeUnits * sizeof( UInt16 ) );
            m_string[ codeUnits ] = 0u;
            m_length   = lengthInCodePoints;
            m_byteSize = ( codeUnits + 1u ) * sizeof( UInt16 );
        }
    }
}

/*-------------------------------------------------------------------------*/

CUtf16String::CUtf16String( const wchar_t* src )
    : m_string( GUCEF_NULL )
    , m_length( 0u )
    , m_byteSize( 0u )
{GUCEF_TRACE;
    if ( GUCEF_NULL != src && 0 != *src )
    {
        UInt32 cuCount = 0u;
        m_string = WcharToUtf16Alloc( src, 0u, false, cuCount, m_length );
        if ( GUCEF_NULL != m_string )
            m_byteSize = ( cuCount + 1u ) * sizeof( UInt16 );
    }
}

/*-------------------------------------------------------------------------*/

CUtf16String::CUtf16String( const wchar_t* src             ,
                             UInt32 lengthInWCodePoints     ,
                             bool reexamine                 )
    : m_string( GUCEF_NULL )
    , m_length( 0u )
    , m_byteSize( 0u )
{GUCEF_TRACE;
    if ( GUCEF_NULL != src && 0u < lengthInWCodePoints )
    {
        UInt32 cuCount = 0u;
        m_string = WcharToUtf16Alloc( src, lengthInWCodePoints, reexamine, cuCount, m_length );
        if ( GUCEF_NULL != m_string )
            m_byteSize = ( cuCount + 1u ) * sizeof( UInt16 );
    }
}

/*-------------------------------------------------------------------------*/

CUtf16String::CUtf16String( const UInt32 utf32CodePoint )
    : m_string( GUCEF_NULL )
    , m_length( 0u )
    , m_byteSize( 0u )
{GUCEF_TRACE;
    if ( 0u != utf32CodePoint )
    {
        UInt32 cuCount = ( utf32CodePoint >= 0x10000u ) ? 2u : 1u;
        m_string = GUCEF_NEW UInt16[ cuCount + 1u ];
        assert( GUCEF_NULL != m_string );
        Utf16EncodeOne( utf32CodePoint, m_string );
        m_string[ cuCount ] = 0u;
        m_length   = 1u;
        m_byteSize = ( cuCount + 1u ) * sizeof( UInt16 );
    }
}

/*-------------------------------------------------------------------------*/

CUtf16String::CUtf16String( const Int32 NULLvalue )
    : m_string( GUCEF_NULL )
    , m_length( 0u )
    , m_byteSize( 0u )
{GUCEF_TRACE;
    GUCEF_ASSERT_ALWAYS( 0 == NULLvalue );
}

/*-------------------------------------------------------------------------*/

CUtf16String::~CUtf16String()
{GUCEF_TRACE;
    Clear();
}

/*-------------------------------------------------------------------------*/

Int32
CUtf16String::EncodeUtf32CodePointToUtf16( UInt32 utf32CodePoint  ,
                                            UInt16* outBuf         ,
                                            UInt32 outBufCodeUnits )
{GUCEF_TRACE;
    if ( GUCEF_NULL == outBuf || 0u == outBufCodeUnits )
        return -1;
    if ( utf32CodePoint < 0x10000u )
    {
        if ( outBufCodeUnits < 1u ) return -1;
        *outBuf = static_cast<UInt16>( utf32CodePoint );
        return 1;
    }
    if ( outBufCodeUnits < 2u ) return -1;
    UInt32 cp = utf32CodePoint - 0x10000u;
    outBuf[ 0 ] = static_cast<UInt16>( 0xD800u | ( cp >> 10 ) );
    outBuf[ 1 ] = static_cast<UInt16>( 0xDC00u | ( cp & 0x3FFu ) );
    return 2;
}

/*-------------------------------------------------------------------------*/

Int32
CUtf16String::EncodeUtf16CodePointToUtf32( const UInt16* buf    ,
                                            UInt32 bufCodeUnits  ,
                                            UInt32& outUtf32     )
{GUCEF_TRACE;
    outUtf32 = 0u;
    if ( GUCEF_NULL == buf || 0u == bufCodeUnits )
        return -1;
    UInt16 cu0 = buf[ 0 ];
    if ( IsHighSurrogate( cu0 ) )
    {
        if ( bufCodeUnits < 2u ) return -1;
        UInt16 cu1 = buf[ 1 ];
        if ( !IsLowSurrogate( cu1 ) ) return -1;
        outUtf32 = 0x10000u
            + ( ( static_cast<UInt32>( cu0 ) - 0xD800u ) << 10 )
            + ( static_cast<UInt32>( cu1 ) - 0xDC00u );
        return 2;
    }
    if ( IsLowSurrogate( cu0 ) ) return -1;
    outUtf32 = static_cast<UInt32>( cu0 );
    return 1;
}

/*-------------------------------------------------------------------------*/

void
CUtf16String::Clear( void )
{GUCEF_TRACE;
    GUCEF_DELETE[] m_string;
    m_string   = GUCEF_NULL;
    m_length   = 0u;
    m_byteSize = 0u;
}

/*-------------------------------------------------------------------------*/

UInt16*
CUtf16String::Reserve( const UInt32 codeUnits, Int32 newLengthInCodePoints )
{GUCEF_TRACE;
    GUCEF_DELETE[] m_string;
    m_string   = GUCEF_NULL;
    m_length   = 0u;
    m_byteSize = 0u;

    if ( 0u == codeUnits )
        return GUCEF_NULL;

    m_string = GUCEF_NEW UInt16[ codeUnits + 1u ];
    assert( GUCEF_NULL != m_string );
    memset( m_string, 0, ( codeUnits + 1u ) * sizeof( UInt16 ) );
    m_byteSize = ( codeUnits + 1u ) * sizeof( UInt16 );

    if ( newLengthInCodePoints >= 0 )
        m_length = static_cast<UInt32>( newLengthInCodePoints );

    return m_string;
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf16String::DetermineLength( void )
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string )
    {
        m_length   = 0u;
        m_byteSize = 0u;
        return 0u;
    }
    UInt32 cuCount = Utf16Strlen( m_string );
    m_length   = Utf16CountCodePoints( m_string, cuCount );
    m_byteSize = ( cuCount + 1u ) * sizeof( UInt16 );
    return m_length;
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf16String::Length( void ) const
{GUCEF_TRACE;
    return m_length;
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf16String::ByteSize( void ) const
{GUCEF_TRACE;
    return m_byteSize;
}

/*-------------------------------------------------------------------------*/

const UInt16*
CUtf16String::C_String( void ) const
{GUCEF_TRACE;
    return m_string;
}

/*-------------------------------------------------------------------------*/

UInt16*
CUtf16String::C_String( void )
{GUCEF_TRACE;
    return m_string;
}

/*-------------------------------------------------------------------------*/

bool
CUtf16String::IsNULLOrEmpty( void ) const
{GUCEF_TRACE;
    return ( GUCEF_NULL == m_string ) || ( 0u == m_length );
}

/*-------------------------------------------------------------------------*/

CUtf16String::operator std::string() const
{GUCEF_TRACE;
    return STL_String();
}

/*-------------------------------------------------------------------------*/

std::string
CUtf16String::STL_String( void ) const
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string || 0u == m_byteSize )
        return std::string();
    UInt32 cuCount = ( m_byteSize / sizeof( UInt16 ) ) - 1u;
    return Utf16BufferToUtf8Str( m_string, cuCount );
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf16String::CodepointAtIndex( const UInt32 index ) const
{GUCEF_TRACE;
    if ( index >= m_length || GUCEF_NULL == m_string )
        return 0u;
    const UInt16* p = m_string;
    UInt32 i = 0u;
    while ( i < index && 0u != *p )
    {
        Utf16DecodeOne( p );
        ++i;
    }
    if ( 0u == *p )
        return 0u;
    return Utf16DecodeOne( p );
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf16String::operator[]( const UInt32 index ) const
{GUCEF_TRACE;
    return CodepointAtIndex( index );
}

/*-------------------------------------------------------------------------*/

/* static */
const UInt16*
CUtf16String::CodepointPtrAtIndex( const UInt16* str          ,
                                    const UInt32 totalCodeUnits ,
                                    const UInt32 length         ,
                                    const UInt32 index          ,
                                    UInt32& codeUnitsFromStart  )
{GUCEF_TRACE;
    codeUnitsFromStart = 0u;
    if ( GUCEF_NULL == str )
        return GUCEF_NULL;
    /* Allow index == length (to support exclusive-end range access) */
    if ( index > length )
        return GUCEF_NULL;

    const UInt16* p = str;
    const UInt16* end = str + totalCodeUnits;
    UInt32 i = 0u;
    while ( i < index && p < end && 0u != *p )
    {
        const UInt16* prev = p;
        Utf16DecodeOne( p );
        codeUnitsFromStart += (UInt32)( p - prev );
        ++i;
    }
    if ( i == index )
        return p;
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const UInt16*
CUtf16String::CodepointPtrAtIndex( const UInt32 index,
                                    UInt32& codeUnitsFromStart ) const
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string )
        return GUCEF_NULL;
    UInt32 totalCU = ( m_byteSize > 0u ) ? ( m_byteSize / sizeof( UInt16 ) ) - 1u : 0u;
    return CodepointPtrAtIndex( m_string, totalCU, m_length, index, codeUnitsFromStart );
}

/*-------------------------------------------------------------------------*/

const UInt16*
CUtf16String::NextCodepointPtr( const UInt16* currentCpPos ) const
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string || GUCEF_NULL == currentCpPos )
        return GUCEF_NULL;
    UInt32 totalCU = ( m_byteSize > 0u ) ? ( m_byteSize / sizeof( UInt16 ) ) - 1u : 0u;
    const UInt16* end = m_string + totalCU;
    if ( currentCpPos < m_string || currentCpPos >= end )
        return GUCEF_NULL;
    if ( 0u == *currentCpPos )
        return GUCEF_NULL;
    const UInt16* p = currentCpPos;
    Utf16DecodeOne( p );
    if ( p > end )
        return GUCEF_NULL;
    return p;
}

/*-------------------------------------------------------------------------*/

const UInt16*
CUtf16String::NextCodepointPtr( const UInt16* currentCpPos ,
                                 UInt32& currentUtf32Cp    ) const
{GUCEF_TRACE;
    currentUtf32Cp = 0u;
    if ( GUCEF_NULL == m_string || GUCEF_NULL == currentCpPos )
        return GUCEF_NULL;
    UInt32 totalCU = ( m_byteSize > 0u ) ? ( m_byteSize / sizeof( UInt16 ) ) - 1u : 0u;
    const UInt16* end = m_string + totalCU;
    if ( currentCpPos < m_string || currentCpPos >= end )
        return GUCEF_NULL;
    if ( 0u == *currentCpPos )
        return GUCEF_NULL;
    const UInt16* p = currentCpPos;
    currentUtf32Cp = Utf16DecodeOne( p );
    if ( p > end )
        return GUCEF_NULL;
    return p;
}

/*-------------------------------------------------------------------------*/

Int32
CUtf16String::CodepointIndexAtPtr( const UInt16* subStrPtr, UInt32& codePoint ) const
{GUCEF_TRACE;
    codePoint = 0u;
    if ( GUCEF_NULL == m_string || GUCEF_NULL == subStrPtr )
        return -1;
    UInt32 totalCU = ( m_byteSize > 0u ) ? ( m_byteSize / sizeof( UInt16 ) ) - 1u : 0u;
    const UInt16* end = m_string + totalCU;
    if ( subStrPtr < m_string || subStrPtr > end )
        return -1;

    Int32 cpIndex = 0;
    const UInt16* p = m_string;
    while ( p < subStrPtr && 0u != *p )
    {
        Utf16DecodeOne( p );
        ++cpIndex;
    }
    if ( p == subStrPtr )
    {
        if ( 0u != *p )
        {
            const UInt16* peek = p;
            codePoint = Utf16DecodeOne( peek );
        }
        return cpIndex;
    }
    return -1;
}

/* --- Set ---------------------------------------------------------------- */

void
CUtf16String::Set( const CUtf16String& new_str )
{GUCEF_TRACE;
    if ( &new_str != this )
        Set( new_str.m_string,
             ( new_str.m_byteSize > 0u ) ? ( new_str.m_byteSize / sizeof( UInt16 ) ) - 1u : 0u,
             new_str.m_length,
             false );
}

/*-------------------------------------------------------------------------*/

void
CUtf16String::Set( const UInt16* new_str              ,
                   UInt32 codeUnits                    ,
                   UInt32 lengthInCodePoints           ,
                   bool reexamine                      )
{GUCEF_TRACE;
    Clear();
    if ( GUCEF_NULL == new_str || 0u == codeUnits )
        return;
    if ( reexamine )
    {
        UInt32 actual = 0u;
        while ( actual < codeUnits && 0u != new_str[ actual ] ) ++actual;
        codeUnits = actual;
    }
    if ( 0u == codeUnits )
        return;

    m_string = GUCEF_NEW UInt16[ codeUnits + 1u ];
    assert( GUCEF_NULL != m_string );
    memcpy( m_string, new_str, codeUnits * sizeof( UInt16 ) );
    m_string[ codeUnits ] = 0u;
    m_length   = lengthInCodePoints;
    m_byteSize = ( codeUnits + 1u ) * sizeof( UInt16 );
}

/*-------------------------------------------------------------------------*/

void
CUtf16String::Set( const UInt16* new_str ,
                   UInt32 codeUnits      ,
                   bool reexamine        )
{GUCEF_TRACE;
    Clear();
    if ( GUCEF_NULL == new_str || 0u == codeUnits )
        return;
    if ( reexamine )
    {
        UInt32 actual = 0u;
        while ( actual < codeUnits && 0u != new_str[ actual ] ) ++actual;
        codeUnits = actual;
    }
    if ( 0u == codeUnits )
        return;

    m_string = GUCEF_NEW UInt16[ codeUnits + 1u ];
    assert( GUCEF_NULL != m_string );
    memcpy( m_string, new_str, codeUnits * sizeof( UInt16 ) );
    m_string[ codeUnits ] = 0u;
    m_length   = Utf16CountCodePoints( m_string, codeUnits );
    m_byteSize = ( codeUnits + 1u ) * sizeof( UInt16 );
}

/*-------------------------------------------------------------------------*/

void
CUtf16String::Set( const UInt16* new_str )
{GUCEF_TRACE;
    Clear();
    if ( GUCEF_NULL == new_str || 0u == *new_str )
        return;
    UInt32 cuCount = Utf16Strlen( new_str );
    m_string = GUCEF_NEW UInt16[ cuCount + 1u ];
    assert( GUCEF_NULL != m_string );
    memcpy( m_string, new_str, ( cuCount + 1u ) * sizeof( UInt16 ) );
    m_length   = Utf16CountCodePoints( m_string, cuCount );
    m_byteSize = ( cuCount + 1u ) * sizeof( UInt16 );
}

/*-------------------------------------------------------------------------*/

void
CUtf16String::Set( const char* new_str )
{GUCEF_TRACE;
    Clear();
    if ( GUCEF_NULL == new_str || '\0' == *new_str )
        return;
    UInt32 cuCount = 0u;
    m_string = Utf8ToUtf16Alloc( new_str, cuCount, m_length );
    if ( GUCEF_NULL != m_string )
        m_byteSize = ( cuCount + 1u ) * sizeof( UInt16 );
}

/*-------------------------------------------------------------------------*/

void
CUtf16String::Set( const char* new_str  ,
                   UInt32 byteSize      ,
                   bool reexamineByteSize )
{GUCEF_TRACE;
    Clear();
    if ( GUCEF_NULL == new_str || 0u == byteSize )
        return;
    if ( reexamineByteSize )
    {
        UInt32 actual = 0u;
        while ( actual < byteSize && '\0' != new_str[ actual ] ) ++actual;
        byteSize = actual;
    }
    if ( 0u == byteSize )
        return;
    UInt32 cuCount = 0u;
    m_string = Utf8BytesToUtf16Alloc( new_str, byteSize, cuCount, m_length );
    if ( GUCEF_NULL != m_string )
        m_byteSize = ( cuCount + 1u ) * sizeof( UInt16 );
}

/*-------------------------------------------------------------------------*/

void
CUtf16String::Set( const char* new_str              ,
                   UInt32 byteSize                  ,
                   UInt32 lengthInCodePoints        ,
                   bool reexamineByteSize           )
{GUCEF_TRACE;
    Set( new_str, byteSize, reexamineByteSize );
    (void) lengthInCodePoints;
}

/*-------------------------------------------------------------------------*/

void
CUtf16String::Set( const wchar_t* new_str )
{GUCEF_TRACE;
    Clear();
    if ( GUCEF_NULL == new_str || 0 == *new_str )
        return;
    UInt32 cuCount = 0u;
    m_string = WcharToUtf16Alloc( new_str, 0u, false, cuCount, m_length );
    if ( GUCEF_NULL != m_string )
        m_byteSize = ( cuCount + 1u ) * sizeof( UInt16 );
}

/*-------------------------------------------------------------------------*/

void
CUtf16String::Set( const wchar_t* new_str           ,
                   UInt32 lengthInWCodePoints        ,
                   bool reexamine                    )
{GUCEF_TRACE;
    Clear();
    if ( GUCEF_NULL == new_str || 0u == lengthInWCodePoints )
        return;
    UInt32 cuCount = 0u;
    m_string = WcharToUtf16Alloc( new_str, lengthInWCodePoints, reexamine, cuCount, m_length );
    if ( GUCEF_NULL != m_string )
        m_byteSize = ( cuCount + 1u ) * sizeof( UInt16 );
}

/*-------------------------------------------------------------------------*/

void
CUtf16String::SetLength( UInt32 newLength, UInt32 /*maxCodeUnitSize*/ )
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string )
        return;
    if ( newLength == m_length )
        return;

    UInt32 totalCU = ( m_byteSize / sizeof( UInt16 ) ) - 1u;

    if ( newLength < m_length )
    {
        /* Truncate: find code-unit position at newLength */
        const UInt16* p = m_string;
        for ( UInt32 i = 0u; i < newLength && 0u != *p; ++i )
            Utf16DecodeOne( p );
        UInt32 truncCU = (UInt32)( p - m_string );
        m_string[ truncCU ] = 0u;
        m_length   = newLength;
        m_byteSize = ( truncCU + 1u ) * sizeof( UInt16 );
    }
    else
    {
        /* Extend with space (U+0020) code points */
        UInt32 addCps = newLength - m_length;
        UInt32 newCU  = totalCU + addCps; /* spaces are always 1 code unit */
        UInt16* newBuf = GUCEF_NEW UInt16[ newCU + 1u ];
        assert( GUCEF_NULL != newBuf );
        memcpy( newBuf, m_string, totalCU * sizeof( UInt16 ) );
        for ( UInt32 i = 0u; i < addCps; ++i )
            newBuf[ totalCU + i ] = static_cast<UInt16>( ' ' );
        newBuf[ newCU ] = 0u;
        GUCEF_DELETE[] m_string;
        m_string   = newBuf;
        m_length   = newLength;
        m_byteSize = ( newCU + 1u ) * sizeof( UInt16 );
    }
}

/*-------------------------------------------------------------------------*/

void
CUtf16String::Scan( const UInt16* newStr, UInt32 maxCodeUnits )
{GUCEF_TRACE;
    Set( newStr, maxCodeUnits, true );
}

/* --- Append ------------------------------------------------------------ */

void
CUtf16String::Append( const UInt16* appendStr              ,
                      UInt32 codeUnits                      ,
                      Int32 lengthInCodePoints              )
{GUCEF_TRACE;
    if ( GUCEF_NULL == appendStr || 0u == codeUnits )
        return;

    UInt32 oldCU = ( m_byteSize > 0u ) ? ( m_byteSize / sizeof( UInt16 ) ) - 1u : 0u;
    UInt32 newCU = oldCU + codeUnits;
    UInt32 addLen = ( lengthInCodePoints >= 0 )
                    ? static_cast<UInt32>( lengthInCodePoints )
                    : Utf16CountCodePoints( appendStr, codeUnits );

    UInt16* newBuf = GUCEF_NEW UInt16[ newCU + 1u ];
    assert( GUCEF_NULL != newBuf );

    if ( GUCEF_NULL != m_string && oldCU > 0u )
        memcpy( newBuf, m_string, oldCU * sizeof( UInt16 ) );
    memcpy( newBuf + oldCU, appendStr, codeUnits * sizeof( UInt16 ) );
    newBuf[ newCU ] = 0u;

    GUCEF_DELETE[] m_string;
    m_string   = newBuf;
    m_length   += addLen;
    m_byteSize = ( newCU + 1u ) * sizeof( UInt16 );
}

/*-------------------------------------------------------------------------*/

void
CUtf16String::Append( const UInt16* appendStr )
{GUCEF_TRACE;
    if ( GUCEF_NULL == appendStr || 0u == *appendStr )
        return;
    UInt32 cuCount = Utf16Strlen( appendStr );
    Append( appendStr, cuCount, -1 );
}

/*-------------------------------------------------------------------------*/

void
CUtf16String::Append( const char* appendStr )
{GUCEF_TRACE;
    if ( GUCEF_NULL == appendStr || '\0' == *appendStr )
        return;
    UInt32 cuCount = 0u, cpCount = 0u;
    UInt16* tmp = Utf8ToUtf16Alloc( appendStr, cuCount, cpCount );
    if ( GUCEF_NULL != tmp )
    {
        Append( tmp, cuCount, (Int32)cpCount );
        GUCEF_DELETE[] tmp;
    }
}

/*-------------------------------------------------------------------------*/

void
CUtf16String::Append( UInt32 utf32CodePoint )
{GUCEF_TRACE;
    if ( 0u == utf32CodePoint )
        return;
    UInt16 cuBuf[ 2 ] = { 0u, 0u };
    UInt32 cuCount = Utf16EncodeOne( utf32CodePoint, cuBuf );
    Append( cuBuf, cuCount, 1 );
}

/*-------------------------------------------------------------------------*/

void
CUtf16String::Append( Int32 utf32CodePoint )
{GUCEF_TRACE;
    Append( static_cast<UInt32>( utf32CodePoint ) );
}

/*-------------------------------------------------------------------------*/

void
CUtf16String::Append( const CUtf16String& appendStr )
{GUCEF_TRACE;
    if ( 0u == appendStr.m_byteSize || GUCEF_NULL == appendStr.m_string )
        return;
    UInt32 cuCount = ( appendStr.m_byteSize / sizeof( UInt16 ) ) - 1u;
    Append( appendStr.m_string, cuCount, (Int32)appendStr.m_length );
}

/* --- Assignment operators ---------------------------------------------- */

CUtf16String&
CUtf16String::operator=( const CUtf16String& src )
{GUCEF_TRACE;
    if ( &src != this )
        Set( src );
    return *this;
}

/*-------------------------------------------------------------------------*/

CUtf16String&
CUtf16String::operator=( const std::string& src )
{GUCEF_TRACE;
    Set( src.c_str(), (UInt32) src.size(), false );
    return *this;
}

/*-------------------------------------------------------------------------*/

CUtf16String&
CUtf16String::operator=( const char* src )
{GUCEF_TRACE;
    Set( src );
    return *this;
}

/* --- Compound-assignment operators ------------------------------------- */

CUtf16String&
CUtf16String::operator+=( const CUtf16String& other )
{GUCEF_TRACE;
    Append( other );
    return *this;
}

/*-------------------------------------------------------------------------*/

CUtf16String&
CUtf16String::operator+=( const char* other )
{GUCEF_TRACE;
    Append( other );
    return *this;
}

/*-------------------------------------------------------------------------*/

CUtf16String&
CUtf16String::operator+=( UInt32 utf32CodePoint )
{GUCEF_TRACE;
    Append( utf32CodePoint );
    return *this;
}

/*-------------------------------------------------------------------------*/

CUtf16String&
CUtf16String::operator+=( char singleByte )
{GUCEF_TRACE;
    Append( static_cast<UInt32>( static_cast<unsigned char>( singleByte ) ) );
    return *this;
}

/* --- Binary + operators ------------------------------------------------ */

CUtf16String
CUtf16String::operator+( const CUtf16String& addition ) const
{GUCEF_TRACE;
    CUtf16String result( *this );
    result.Append( addition );
    return result;
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16String::operator+( const char* addition ) const
{GUCEF_TRACE;
    CUtf16String result( *this );
    result.Append( addition );
    return result;
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16String::operator+( UInt32 utf32CodePoint ) const
{GUCEF_TRACE;
    CUtf16String result( *this );
    result.Append( utf32CodePoint );
    return result;
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16String::operator+( char singleByte ) const
{GUCEF_TRACE;
    CUtf16String result( *this );
    result += singleByte;
    return result;
}

/* --- Comparison operators ---------------------------------------------- */

bool
CUtf16String::operator==( const CUtf16String& other ) const
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string && GUCEF_NULL == other.m_string )
        return true;
    if ( m_length != other.m_length )
        return false;
    if ( GUCEF_NULL == m_string )
        return other.m_length == 0u;
    if ( GUCEF_NULL == other.m_string )
        return m_length == 0u;
    return 0 == Utf16Strcmp( m_string, other.m_string );
}

/*-------------------------------------------------------------------------*/

bool
CUtf16String::operator==( const char* other ) const
{GUCEF_TRACE;
    return *this == CUtf16String( other );
}

/*-------------------------------------------------------------------------*/

bool
CUtf16String::operator==( const Int32 NULLvalue ) const
{GUCEF_TRACE;
    if ( 0 == NULLvalue )
        return m_length == 0u;
    return *this == CUtf16String( static_cast<UInt32>( NULLvalue ) );
}

/*-------------------------------------------------------------------------*/

bool
CUtf16String::operator==( const UInt32 utf32CodePoint ) const
{GUCEF_TRACE;
    if ( 0u == utf32CodePoint )
        return m_length == 0u;
    if ( m_length != 1u )
        return false;
    return CodepointAtIndex( 0u ) == utf32CodePoint;
}

/*-------------------------------------------------------------------------*/

bool
CUtf16String::operator==( const char character ) const
{GUCEF_TRACE;
    return *this == static_cast<UInt32>( static_cast<unsigned char>( character ) );
}

/*-------------------------------------------------------------------------*/

bool
CUtf16String::operator!=( const CUtf16String& other ) const
{GUCEF_TRACE;
    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

bool
CUtf16String::operator!=( const char* other ) const
{GUCEF_TRACE;
    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

bool
CUtf16String::operator!=( const Int32 NULLvalue ) const
{GUCEF_TRACE;
    return !( *this == NULLvalue );
}

/*-------------------------------------------------------------------------*/

bool
CUtf16String::operator!=( const UInt32 utf32CodePoint ) const
{GUCEF_TRACE;
    return !( *this == utf32CodePoint );
}

/*-------------------------------------------------------------------------*/

bool
CUtf16String::operator!=( const char character ) const
{GUCEF_TRACE;
    return !( *this == character );
}

/*-------------------------------------------------------------------------*/

bool
CUtf16String::operator<( const CUtf16String& other ) const
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string && GUCEF_NULL == other.m_string )
        return false;
    if ( GUCEF_NULL == m_string )
        return other.m_length > 0u;
    if ( GUCEF_NULL == other.m_string )
        return false;
    return Utf16Strcmp( m_string, other.m_string ) < 0;
}

/*-------------------------------------------------------------------------*/

bool
CUtf16String::operator<=( const CUtf16String& other ) const
{GUCEF_TRACE;
    return !( other < *this );
}

/* --- Case --------------------------------------------------------------- */

CUtf16String
CUtf16String::Lowercase( void ) const
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string || 0u == m_byteSize )
        return CUtf16String();
    UInt32 totalCU = ( m_byteSize / sizeof( UInt16 ) ) - 1u;
    CUtf16String result;
    result.m_string = GUCEF_NEW UInt16[ totalCU + 1u ];
    assert( GUCEF_NULL != result.m_string );
    memcpy( result.m_string, m_string, ( totalCU + 1u ) * sizeof( UInt16 ) );
    result.m_length   = m_length;
    result.m_byteSize = m_byteSize;
    /* Basic Latin lowercase: U+0041–U+005A → U+0061–U+007A */
    for ( UInt32 i = 0u; i < totalCU; ++i )
    {
        UInt16 cu = result.m_string[ i ];
        if ( cu >= 0x0041u && cu <= 0x005Au )
            result.m_string[ i ] = static_cast<UInt16>( cu + 0x0020u );
    }
    return result;
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16String::Uppercase( void ) const
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string || 0u == m_byteSize )
        return CUtf16String();
    UInt32 totalCU = ( m_byteSize / sizeof( UInt16 ) ) - 1u;
    CUtf16String result;
    result.m_string = GUCEF_NEW UInt16[ totalCU + 1u ];
    assert( GUCEF_NULL != result.m_string );
    memcpy( result.m_string, m_string, ( totalCU + 1u ) * sizeof( UInt16 ) );
    result.m_length   = m_length;
    result.m_byteSize = m_byteSize;
    /* Basic Latin uppercase: U+0061–U+007A → U+0041–U+005A */
    for ( UInt32 i = 0u; i < totalCU; ++i )
    {
        UInt16 cu = result.m_string[ i ];
        if ( cu >= 0x0061u && cu <= 0x007Au )
            result.m_string[ i ] = static_cast<UInt16>( cu - 0x0020u );
    }
    return result;
}

/* --- Search ------------------------------------------------------------ */

Int32
CUtf16String::HasChar( UInt32 searchCodePoint, bool frontToBack ) const
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string || 0u == m_length )
        return -1;

    if ( frontToBack )
    {
        const UInt16* p = m_string;
        Int32 idx = 0;
        while ( 0u != *p )
        {
            UInt32 cp = Utf16DecodeOne( p );
            if ( cp == searchCodePoint )
                return idx;
            ++idx;
        }
    }
    else
    {
        const UInt16* p = m_string;
        Int32 lastFound = -1;
        Int32 idx = 0;
        while ( 0u != *p )
        {
            UInt32 cp = Utf16DecodeOne( p );
            if ( cp == searchCodePoint )
                lastFound = idx;
            ++idx;
        }
        return lastFound;
    }
    return -1;
}

/*-------------------------------------------------------------------------*/

Int32
CUtf16String::HasChar( char searchChar, bool frontToBack ) const
{GUCEF_TRACE;
    return HasChar( static_cast<UInt32>( static_cast<unsigned char>( searchChar ) ), frontToBack );
}

/*-------------------------------------------------------------------------*/

Int32
CUtf16String::HasChar( UInt32 searchCodePoint ,
                       UInt32 startIndex      ,
                       bool frontToBack       ) const
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string || 0u == m_length || startIndex >= m_length )
        return -1;

    if ( frontToBack )
    {
        const UInt16* p = m_string;
        UInt32 idx = 0u;
        /* skip to startIndex */
        while ( idx < startIndex && 0u != *p )
        {
            Utf16DecodeOne( p );
            ++idx;
        }
        while ( 0u != *p )
        {
            UInt32 cp = Utf16DecodeOne( p );
            if ( cp == searchCodePoint )
                return (Int32)idx;
            ++idx;
        }
    }
    else
    {
        /* Scan backward from end to startIndex */
        const UInt16* p = m_string;
        Int32 lastFound = -1;
        UInt32 idx = 0u;
        while ( idx <= startIndex && 0u != *p )
        {
            UInt32 cp = Utf16DecodeOne( p );
            if ( cp == searchCodePoint )
                lastFound = (Int32)idx;
            ++idx;
        }
        return lastFound;
    }
    return -1;
}

/*-------------------------------------------------------------------------*/

Int32
CUtf16String::HasChar( char searchChar  ,
                       UInt32 startIndex ,
                       bool frontToBack  ) const
{GUCEF_TRACE;
    return HasChar( static_cast<UInt32>( static_cast<unsigned char>( searchChar ) ), startIndex, frontToBack );
}

/*-------------------------------------------------------------------------*/

Int32
CUtf16String::HasSubstr( const CUtf16String& substr ,
                          Int32 startIndex           ,
                          bool frontToBack           ,
                          bool shiftSearch           ) const
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string || 0u == m_length )
        return -1;
    if ( GUCEF_NULL == substr.m_string || 0u == substr.m_length )
        return -1;
    if ( substr.m_length > m_length )
        return -1;

    UInt32 totalCU    = ( m_byteSize / sizeof( UInt16 ) ) - 1u;
    UInt32 subTotalCU = ( substr.m_byteSize / sizeof( UInt16 ) ) - 1u;

    if ( frontToBack )
    {
        UInt32 cpIdx = ( startIndex >= 0 ) ? (UInt32)startIndex : 0u;
        const UInt16* p = m_string;
        /* Seek to startIndex */
        for ( UInt32 i = 0u; i < cpIdx && 0u != *p; ++i )
            Utf16DecodeOne( p );

        while ( 0u != *p )
        {
            UInt32 remaining = (UInt32)( m_string + totalCU - p );
            if ( remaining >= subTotalCU &&
                 0 == Utf16Memcmp( p, substr.m_string, subTotalCU ) )
                return (Int32)cpIdx;
            if ( !shiftSearch )
                return -1;
            if ( 0u == *p ) break;
            Utf16DecodeOne( p );
            ++cpIdx;
        }
    }
    else
    {
        /* Back-to-front: scan forward keeping last match */
        UInt32 endCpIdx = ( startIndex >= 0 ) ? (UInt32)startIndex : m_length - 1u;
        const UInt16* p = m_string;
        Int32 lastFound = -1;
        UInt32 cpIdx    = 0u;

        while ( 0u != *p && cpIdx <= endCpIdx )
        {
            UInt32 remaining = (UInt32)( m_string + totalCU - p );
            if ( remaining >= subTotalCU &&
                 0 == Utf16Memcmp( p, substr.m_string, subTotalCU ) )
            {
                lastFound = (Int32)cpIdx;
                if ( !shiftSearch )
                    return lastFound;
            }
            Utf16DecodeOne( p );
            ++cpIdx;
        }
        return lastFound;
    }
    return -1;
}

/*-------------------------------------------------------------------------*/

Int32
CUtf16String::HasSubstr( const CUtf16String& substr ,
                          bool frontToBack           ,
                          bool shiftSearch           ) const
{GUCEF_TRACE;
    return HasSubstr( substr, frontToBack ? 0 : -1, frontToBack, shiftSearch );
}

/*-------------------------------------------------------------------------*/

bool
CUtf16String::StartsWith( const CUtf16String& substr ) const
{GUCEF_TRACE;
    if ( substr.m_length == 0u )
        return true;
    if ( substr.m_length > m_length )
        return false;
    if ( GUCEF_NULL == m_string || GUCEF_NULL == substr.m_string )
        return false;
    UInt32 subCU = ( substr.m_byteSize / sizeof( UInt16 ) ) - 1u;
    return 0 == Utf16Memcmp( m_string, substr.m_string, subCU );
}

/*-------------------------------------------------------------------------*/

bool
CUtf16String::EndsWith( const CUtf16String& substr ) const
{GUCEF_TRACE;
    if ( substr.m_length == 0u )
        return true;
    if ( substr.m_length > m_length || GUCEF_NULL == m_string || GUCEF_NULL == substr.m_string )
        return false;

    UInt32 totalCU = ( m_byteSize / sizeof( UInt16 ) ) - 1u;
    UInt32 subCU   = ( substr.m_byteSize / sizeof( UInt16 ) ) - 1u;
    if ( totalCU < subCU )
        return false;

    return 0 == Utf16Memcmp( m_string + totalCU - subCU, substr.m_string, subCU );
}

/* --- Substring extraction ---------------------------------------------- */

CUtf16String
CUtf16String::SubstrToChar( UInt32 searchCodePoint               ,
                             bool frontToBack                     ,
                             bool returnEmptyIfCharNotFound       ) const
{GUCEF_TRACE;
    return SubstrToChar( searchCodePoint, frontToBack ? 0u : m_length - 1u, frontToBack, returnEmptyIfCharNotFound );
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16String::SubstrToChar( char searchChar                       ,
                             bool frontToBack                      ,
                             bool returnEmptyIfCharNotFound        ) const
{GUCEF_TRACE;
    return SubstrToChar( static_cast<UInt32>( static_cast<unsigned char>( searchChar ) ), frontToBack, returnEmptyIfCharNotFound );
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16String::SubstrToChar( UInt32 searchCodePoint               ,
                             UInt32 startIndex                    ,
                             bool frontToBack                     ,
                             bool returnEmptyIfCharNotFound       ) const
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string || 0u == m_length )
    {
        if ( returnEmptyIfCharNotFound ) return CUtf16String();
        return *this;
    }
    UInt32 totalCU = ( m_byteSize / sizeof( UInt16 ) ) - 1u;

    if ( frontToBack )
    {
        /* Walk from startIndex, return everything before the found char */
        const UInt16* p = m_string;
        for ( UInt32 i = 0u; i < startIndex && 0u != *p; ++i )
            Utf16DecodeOne( p );

        while ( 0u != *p )
        {
            const UInt16* cpStart = p;
            UInt32 cp = Utf16DecodeOne( p );
            if ( cp == searchCodePoint )
            {
                UInt32 cuCount = (UInt32)( cpStart - m_string );
                return CUtf16String( m_string, cuCount );
            }
        }
    }
    else
    {
        /* Walk backwards from startIndex, return everything after found char */
        const UInt16* lastFound = GUCEF_NULL;
        const UInt16* afterFound = GUCEF_NULL;
        const UInt16* p = m_string;
        UInt32 idx = 0u;
        while ( 0u != *p && idx <= startIndex )
        {
            const UInt16* cpStart = p;
            UInt32 cp = Utf16DecodeOne( p );
            if ( cp == searchCodePoint )
            {
                lastFound  = cpStart;
                afterFound = p;
            }
            ++idx;
        }
        if ( GUCEF_NULL != lastFound )
        {
            UInt32 cuOffset = (UInt32)( afterFound - m_string );
            UInt32 cuCount  = totalCU - cuOffset;
            return CUtf16String( afterFound, cuCount );
        }
    }

    if ( returnEmptyIfCharNotFound ) return CUtf16String();
    return *this;
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16String::SubstrToChar( char searchChar                       ,
                             UInt32 startIndex                     ,
                             bool frontToBack                      ,
                             bool returnEmptyIfCharNotFound        ) const
{GUCEF_TRACE;
    return SubstrToChar( static_cast<UInt32>( static_cast<unsigned char>( searchChar ) ), startIndex, frontToBack, returnEmptyIfCharNotFound );
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16String::SubstrToSubstr( const CUtf16String& searchstr ,
                               bool frontToBack              ) const
{GUCEF_TRACE;
    return SubstrToSubstr( searchstr, frontToBack ? 0u : ( m_length > 0u ? m_length - 1u : 0u ), frontToBack );
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16String::SubstrToSubstr( const CUtf16String& searchstr ,
                               UInt32 startIndex             ,
                               bool frontToBack              ) const
{GUCEF_TRACE;
    Int32 foundIdx = HasSubstr( searchstr, (Int32)startIndex, frontToBack, true );
    if ( foundIdx < 0 )
        return *this;

    if ( frontToBack )
    {
        /* Return text from startIndex up to (but not including) where searchstr was found */
        UInt32 startCU = 0u;
        CodepointPtrAtIndex( startIndex, startCU );
        UInt32 foundCU = 0u;
        CodepointPtrAtIndex( (UInt32)foundIdx, foundCU );
        if ( foundCU <= startCU )
            return CUtf16String();
        return CUtf16String( m_string + startCU, foundCU - startCU );
    }
    else
    {
        /* Return text from just after the found searchstr up to startIndex (inclusive) */
        UInt32 afterFoundIdx = (UInt32)foundIdx + searchstr.m_length;
        UInt32 afterFoundCU  = 0u;
        CodepointPtrAtIndex( afterFoundIdx, afterFoundCU );
        UInt32 endCU = 0u;
        CodepointPtrAtIndex( startIndex + 1u, endCU );
        if ( endCU <= afterFoundCU )
            return CUtf16String();
        return CUtf16String( m_string + afterFoundCU, endCU - afterFoundCU );
    }
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16String::SubstrToIndex( UInt32 index, bool frontToBack ) const
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string || 0u == m_length )
        return CUtf16String();
    if ( index >= m_length )
        return frontToBack ? *this : CUtf16String();

    UInt32 totalCU = ( m_byteSize / sizeof( UInt16 ) ) - 1u;

    if ( frontToBack )
    {
        UInt32 cuOffset = 0u;
        CodepointPtrAtIndex( index, cuOffset );
        return CUtf16String( m_string, cuOffset );
    }
    else
    {
        UInt32 cuOffset = 0u;
        const UInt16* p = CodepointPtrAtIndex( index, cuOffset );
        if ( GUCEF_NULL == p )
            return CUtf16String();
        UInt32 remaining = totalCU - cuOffset;
        return CUtf16String( p, remaining );
    }
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16String::SubstrFromRange( UInt32 startIndex ,
                                UInt32 endIndex   ) const
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string || startIndex >= endIndex || startIndex >= m_length )
        return CUtf16String();

    UInt32 clampedEnd = ( endIndex > m_length ) ? m_length : endIndex;

    UInt32 startCU = 0u;
    CodepointPtrAtIndex( startIndex, startCU );
    UInt32 endCU = 0u;
    CodepointPtrAtIndex( clampedEnd, endCU );

    if ( endCU <= startCU )
        return CUtf16String();
    return CUtf16String( m_string + startCU, endCU - startCU );
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16String::Trim( bool frontToBack ) const
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string || 0u == m_length )
        return CUtf16String();

    UInt32 totalCU = ( m_byteSize / sizeof( UInt16 ) ) - 1u;

    const UInt16* start = m_string;
    const UInt16* end   = m_string + totalCU;

    if ( frontToBack )
    {
        while ( start < end )
        {
            const UInt16* p = start;
            UInt32 cp = Utf16DecodeOne( p );
            if ( cp > 0x20u ) break;
            start = p;
        }
    }
    else
    {
        while ( end > start )
        {
            /* Walk forward to find the last code point */
            const UInt16* p = start;
            const UInt16* prev = p;
            UInt32 cp = 0u;
            while ( p < end )
            {
                prev = p;
                cp = Utf16DecodeOne( p );
            }
            if ( cp > 0x20u ) break;
            end = prev;
        }
    }

    UInt32 cuCount = (UInt32)( end - start );
    if ( 0u == cuCount ) return CUtf16String();
    return CUtf16String( start, cuCount );
}

/* --- Replace ------------------------------------------------------------ */

CUtf16String
CUtf16String::ReplaceChar( UInt32 oldCodePoint, UInt32 newCodePoint ) const
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string || 0u == m_length )
        return CUtf16String();

    UInt32 totalCU = ( m_byteSize / sizeof( UInt16 ) ) - 1u;
    /* Worst case: every old cp is supplementary (2 CU) → newCp also 2 CU → same total */
    UInt32 newCpCU = ( newCodePoint >= 0x10000u ) ? 2u : 1u;

    /* Use temporary std::string to build result */
    CUtf16String result;
    const UInt16* p = m_string;
    while ( 0u != *p )
    {
        UInt32 cp = Utf16DecodeOne( p );
        if ( cp == oldCodePoint )
            result.Append( newCodePoint );
        else
        {
            UInt16 tmp[ 2 ] = { 0u, 0u };
            UInt32 cu = Utf16EncodeOne( cp, tmp );
            result.Append( tmp, cu, 1 );
        }
    }
    (void) newCpCU;
    return result;
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16String::ReplaceChar( Int32 oldCodePoint, Int32 newCodePoint ) const
{GUCEF_TRACE;
    return ReplaceChar( static_cast<UInt32>( oldCodePoint ), static_cast<UInt32>( newCodePoint ) );
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16String::ReplaceChars( const Int32* oldCodePoints ,
                             UInt32 nrOfOldCodePoints   ,
                             Int32 newCodePoint         ) const
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string || 0u == m_length || GUCEF_NULL == oldCodePoints || 0u == nrOfOldCodePoints )
        return *this;

    CUtf16String result;
    const UInt16* p = m_string;
    while ( 0u != *p )
    {
        const UInt16* cpStart = p;
        UInt32 cp = Utf16DecodeOne( p );
        bool found = false;
        for ( UInt32 i = 0u; i < nrOfOldCodePoints && !found; ++i )
            found = ( cp == static_cast<UInt32>( oldCodePoints[ i ] ) );
        if ( found )
            result.Append( static_cast<UInt32>( newCodePoint ) );
        else
        {
            UInt32 cuCount = (UInt32)( p - cpStart );
            result.Append( cpStart, cuCount, 1 );
        }
    }
    return result;
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16String::ReplaceSubstr( const CUtf16String& substr      ,
                              const CUtf16String& replacement ) const
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string || 0u == m_length || 0u == substr.m_length )
        return *this;

    UInt32 totalCU = ( m_byteSize / sizeof( UInt16 ) ) - 1u;
    UInt32 subCU   = ( substr.m_byteSize / sizeof( UInt16 ) ) - 1u;

    CUtf16String result;
    const UInt16* p   = m_string;
    const UInt16* end = m_string + totalCU;

    while ( p < end && 0u != *p )
    {
        UInt32 remaining = (UInt32)( end - p );
        if ( remaining >= subCU && 0 == Utf16Memcmp( p, substr.m_string, subCU ) )
        {
            result.Append( replacement );
            p += subCU;
        }
        else
        {
            const UInt16* cpStart = p;
            UInt32 cp = Utf16DecodeOne( p );
            UInt32 cuCount = (UInt32)( p - cpStart );
            result.Append( cpStart, cuCount, 1 );
            (void) cp;
        }
    }
    return result;
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16String::ReplaceEnvelopingSubstr( const CUtf16String& envelopPrefix     ,
                                        const CUtf16String& envelopPostfix    ,
                                        const CUtf16String& newEnvelopPrefix  ,
                                        const CUtf16String& newEnvelopPostfix ) const
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string || 0u == m_length )
        return *this;

    CUtf16String result;
    UInt32 searchStart = 0u;
    while ( searchStart < m_length )
    {
        Int32 prefixIdx = HasSubstr( envelopPrefix, (Int32)searchStart, true, true );
        if ( prefixIdx < 0 )
        {
            result.Append( SubstrFromRange( searchStart, m_length ) );
            break;
        }
        /* Append everything before the prefix */
        result.Append( SubstrFromRange( searchStart, (UInt32)prefixIdx ) );

        Int32 postfixIdx = HasSubstr( envelopPostfix, prefixIdx + (Int32)envelopPrefix.m_length, true, true );
        if ( postfixIdx < 0 )
        {
            result.Append( SubstrFromRange( (UInt32)prefixIdx, m_length ) );
            break;
        }
        result.Append( newEnvelopPrefix );
        result.Append( SubstrFromRange( (UInt32)prefixIdx + envelopPrefix.m_length, (UInt32)postfixIdx ) );
        result.Append( newEnvelopPostfix );
        searchStart = (UInt32)postfixIdx + envelopPostfix.m_length;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16String::CutEnvelopedSubstr( const CUtf16String& envelopPrefix  ,
                                   const CUtf16String& envelopPostfix ,
                                   UInt32 envelopedStrIndex            ) const
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string || 0u == m_length )
        return CUtf16String();

    UInt32 searchStart = 0u;
    UInt32 count = 0u;
    while ( searchStart < m_length )
    {
        Int32 prefixIdx = HasSubstr( envelopPrefix, (Int32)searchStart, true, true );
        if ( prefixIdx < 0 ) break;

        UInt32 contentStart = (UInt32)prefixIdx + envelopPrefix.m_length;
        Int32 postfixIdx = HasSubstr( envelopPostfix, (Int32)contentStart, true, true );
        if ( postfixIdx < 0 ) break;

        if ( count == envelopedStrIndex )
            return SubstrFromRange( contentStart, (UInt32)postfixIdx );

        ++count;
        searchStart = (UInt32)postfixIdx + envelopPostfix.m_length;
    }
    return CUtf16String();
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16String::ReplaceSubStr( UInt32 startIndex                ,
                              UInt32 length                    ,
                              const CUtf16String& newSubstr    ) const
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string || 0u == m_length )
        return *this;
    CUtf16String result( SubstrFromRange( 0u, startIndex ) );
    result.Append( newSubstr );
    result.Append( SubstrFromRange( startIndex + length, m_length ) );
    return result;
}

/* --- CutChars / RemoveChar / Compact ----------------------------------- */

CUtf16String
CUtf16String::CutChars( UInt32 charcount    ,
                         bool frontToBack    ,
                         UInt32 startOffset  ) const
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string || 0u == m_length || 0u == charcount )
        return *this;

    if ( frontToBack )
    {
        UInt32 removeEnd = startOffset + charcount;
        if ( removeEnd > m_length ) removeEnd = m_length;
        CUtf16String left( SubstrFromRange( 0u, startOffset ) );
        left.Append( SubstrFromRange( removeEnd, m_length ) );
        return left;
    }
    else
    {
        if ( charcount >= m_length )
            return CUtf16String();
        if ( 0u == startOffset )
        {
            /* Remove the last charcount chars */
            return SubstrFromRange( 0u, m_length - charcount );
        }
        else
        {
            /* Remove charcount chars ending at (m_length - startOffset - 1) */
            UInt32 cutEnd   = ( m_length > startOffset ) ? m_length - startOffset : 0u;
            UInt32 cutStart = ( cutEnd >= charcount ) ? cutEnd - charcount : 0u;
            CUtf16String left( SubstrFromRange( 0u, cutStart ) );
            left.Append( SubstrFromRange( cutEnd, m_length ) );
            return left;
        }
    }
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16String::RemoveChar( UInt32 codePointToRemove ) const
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string || 0u == m_length )
        return *this;
    CUtf16String result;
    const UInt16* p = m_string;
    while ( 0u != *p )
    {
        const UInt16* cpStart = p;
        UInt32 cp = Utf16DecodeOne( p );
        if ( cp != codePointToRemove )
            result.Append( cpStart, (UInt32)( p - cpStart ), 1 );
    }
    return result;
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16String::RemoveChar( Int32 codePointToRemove ) const
{GUCEF_TRACE;
    return RemoveChar( static_cast<UInt32>( codePointToRemove ) );
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16String::CompactRepeatingChar( UInt32 codePointToCompact ) const
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string || 0u == m_length )
        return *this;
    CUtf16String result;
    const UInt16* p = m_string;
    UInt32 prevCp = 0u;
    while ( 0u != *p )
    {
        const UInt16* cpStart = p;
        UInt32 cp = Utf16DecodeOne( p );
        if ( cp != codePointToCompact || cp != prevCp )
            result.Append( cpStart, (UInt32)( p - cpStart ), 1 );
        prevCp = cp;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16String::CompactRepeatingChar( Int32 codePointToCompact ) const
{GUCEF_TRACE;
    return CompactRepeatingChar( static_cast<UInt32>( codePointToCompact ) );
}

/* --- Character counts -------------------------------------------------- */

UInt32
CUtf16String::GetCharacterCount( UInt32 searchCodePoint ) const
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string || 0u == m_length )
        return 0u;
    UInt32 count = 0u;
    const UInt16* p = m_string;
    while ( 0u != *p )
    {
        if ( Utf16DecodeOne( p ) == searchCodePoint )
            ++count;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf16String::GetCharacterCount( Int32 searchCodePoint ) const
{GUCEF_TRACE;
    return GetCharacterCount( static_cast<UInt32>( searchCodePoint ) );
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf16String::GetCharactersCount( const Int32* searchCodePoints ,
                                   UInt32 nrOfSearchCodePoints   ) const
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string || 0u == m_length || GUCEF_NULL == searchCodePoints || 0u == nrOfSearchCodePoints )
        return 0u;
    UInt32 count = 0u;
    const UInt16* p = m_string;
    while ( 0u != *p )
    {
        UInt32 cp = Utf16DecodeOne( p );
        for ( UInt32 i = 0u; i < nrOfSearchCodePoints; ++i )
        {
            if ( cp == static_cast<UInt32>( searchCodePoints[ i ] ) )
            {
                ++count;
                break;
            }
        }
    }
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf16String::GetCharactersCount( const char* searchChars ,
                                   UInt32 nrOfSearchChars  ) const
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string || 0u == m_length || GUCEF_NULL == searchChars || 0u == nrOfSearchChars )
        return 0u;
    UInt32 count = 0u;
    const UInt16* p = m_string;
    while ( 0u != *p )
    {
        UInt32 cp = Utf16DecodeOne( p );
        for ( UInt32 i = 0u; i < nrOfSearchChars; ++i )
        {
            if ( cp == static_cast<UInt32>( static_cast<unsigned char>( searchChars[ i ] ) ) )
            {
                ++count;
                break;
            }
        }
    }
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf16String::GetNonMatchCharactersCount( const CUtf16String& searchChars ) const
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string || 0u == m_length )
        return 0u;
    UInt32 count = 0u;
    const UInt16* p = m_string;
    while ( 0u != *p )
    {
        UInt32 cp = Utf16DecodeOne( p );
        if ( searchChars.HasChar( cp ) < 0 )
            ++count;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf16String::GetNonMatchCharactersCount( const Int32* searchCodePoints ,
                                           UInt32 nrOfSearchCodePoints   ) const
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string || 0u == m_length || GUCEF_NULL == searchCodePoints || 0u == nrOfSearchCodePoints )
        return m_length;
    UInt32 count = 0u;
    const UInt16* p = m_string;
    while ( 0u != *p )
    {
        UInt32 cp = Utf16DecodeOne( p );
        bool found = false;
        for ( UInt32 i = 0u; i < nrOfSearchCodePoints && !found; ++i )
            found = ( cp == static_cast<UInt32>( searchCodePoints[ i ] ) );
        if ( !found ) ++count;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf16String::GetNonMatchCharactersCount( const char* searchChars ,
                                           UInt32 nrOfSearchChars  ) const
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string || 0u == m_length || GUCEF_NULL == searchChars || 0u == nrOfSearchChars )
        return m_length;
    UInt32 count = 0u;
    const UInt16* p = m_string;
    while ( 0u != *p )
    {
        UInt32 cp = Utf16DecodeOne( p );
        bool found = false;
        for ( UInt32 i = 0u; i < nrOfSearchChars && !found; ++i )
            found = ( cp == static_cast<UInt32>( static_cast<unsigned char>( searchChars[ i ] ) ) );
        if ( !found ) ++count;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf16String::GetCharacterRepeatCount( UInt32 searchCodePoint ) const
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string || 0u == m_length )
        return 0u;
    UInt32 maxRun = 0u, curRun = 0u;
    bool prevWasMatch = false;
    const UInt16* p = m_string;
    while ( 0u != *p )
    {
        UInt32 cp = Utf16DecodeOne( p );
        if ( cp == searchCodePoint )
        {
            /* Only count additional consecutive occurrences (not the first one in a run) */
            if ( prevWasMatch )
            {
                ++curRun;
                if ( curRun > maxRun ) maxRun = curRun;
            }
            prevWasMatch = true;
        }
        else
        {
            curRun = 0u;
            prevWasMatch = false;
        }
    }
    return maxRun;
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf16String::GetCharacterRepeatCount( Int32 searchCodePoint ) const
{GUCEF_TRACE;
    return GetCharacterRepeatCount( static_cast<UInt32>( searchCodePoint ) );
}

/* --- Equality helpers -------------------------------------------------- */

UInt32
CUtf16String::FindMaxSubstrEquality( const CUtf16String& searchStr ,
                                      UInt32 startOffset            ,
                                      bool frontToBack              ,
                                      bool isCaseSensitive          ) const
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string || GUCEF_NULL == searchStr.m_string )
        return 0u;

    const CUtf16String& a = isCaseSensitive ? *this      : Lowercase();
    const CUtf16String& b = isCaseSensitive ? searchStr  : searchStr.Lowercase();

    UInt32 matchLen = 0u;
    UInt32 aLen = a.m_length;
    UInt32 bLen = b.m_length;

    if ( frontToBack )
    {
        UInt32 aIdx = startOffset;
        UInt32 bIdx = 0u;
        while ( aIdx < aLen && bIdx < bLen )
        {
            if ( a.CodepointAtIndex( aIdx ) == b.CodepointAtIndex( bIdx ) )
            {
                ++matchLen;
                ++aIdx;
                ++bIdx;
            }
            else break;
        }
    }
    else
    {
        Int32 aIdx = (Int32)( aLen - 1u ) - (Int32)startOffset;
        Int32 bIdx = (Int32)( bLen - 1u );
        while ( aIdx >= 0 && bIdx >= 0 )
        {
            if ( a.CodepointAtIndex( (UInt32)aIdx ) == b.CodepointAtIndex( (UInt32)bIdx ) )
            {
                ++matchLen;
                --aIdx;
                --bIdx;
            }
            else break;
        }
    }
    return matchLen;
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf16String::FindMaxSegmentEquality( const CUtf16String& otherStr    ,
                                       UInt32 segmentDividerCodePoint  ,
                                       bool startFront                 ,
                                       bool isCaseSensitive            ) const
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string || GUCEF_NULL == otherStr.m_string )
        return 0u;

    /* Parse both strings into segments using segmentDividerCodePoint */
    StringVector mySegs    = ParseElements( segmentDividerCodePoint, false );
    StringVector otherSegs = otherStr.ParseElements( segmentDividerCodePoint, false );

    UInt32 myCount    = (UInt32) mySegs.size();
    UInt32 otherCount = (UInt32) otherSegs.size();
    UInt32 minCount   = ( myCount < otherCount ) ? myCount : otherCount;
    UInt32 matchCount = 0u;

    for ( UInt32 i = 0u; i < minCount; ++i )
    {
        UInt32 mi = startFront ? i : ( myCount - 1u - i );
        UInt32 oi = startFront ? i : ( otherCount - 1u - i );

        bool eq = isCaseSensitive
                  ? ( mySegs[ mi ] == otherSegs[ oi ] )
                  : mySegs[ mi ].Equals( otherSegs[ oi ], false );
        if ( eq )
            ++matchCount;
        else
            break;
    }
    return matchCount;
}

/* --- Parsing ------------------------------------------------------------ */

CUtf16String::StringVector
CUtf16String::ParseElements( UInt32 separator,
                              bool addEmptyElements ) const
{GUCEF_TRACE;
    StringVector result;
    if ( GUCEF_NULL == m_string || 0u == m_byteSize )
        return result;
    UInt32 totalCU = ( m_byteSize / sizeof( UInt16 ) ) - 1u;
    return ParseElements( m_string, totalCU, separator, addEmptyElements );
}

/*-------------------------------------------------------------------------*/

/* static */
CUtf16String::StringVector
CUtf16String::ParseElements( const UInt16* bufferPtr ,
                              UInt32 bufferCodeUnits  ,
                              UInt32 separator        ,
                              bool addEmptyElements   )
{GUCEF_TRACE;
    StringVector result;
    if ( GUCEF_NULL == bufferPtr || 0u == bufferCodeUnits )
        return result;

    const UInt16* segStart = bufferPtr;
    const UInt16* p        = bufferPtr;
    const UInt16* end      = bufferPtr + bufferCodeUnits;

    while ( p <= end )
    {
        bool isSep = false;
        const UInt16* cpStart = p;
        if ( p < end && 0u != *p )
        {
            UInt32 cp = Utf16DecodeOne( p );
            isSep = ( cp == separator );
        }
        else
        {
            p = end + 1u; /* force flush */
        }

        if ( isSep || p > end )
        {
            UInt32 segCU = (UInt32)( cpStart - segStart );
            if ( segCU > 0u || addEmptyElements )
                result.push_back( CUtf16String( segStart, segCU ) );
            segStart = p;
        }
    }
    return result;
}

/*-------------------------------------------------------------------------*/

CUtf16String::StringSet
CUtf16String::ParseUniqueElements( UInt32 separator,
                                    bool addEmptyElements ) const
{GUCEF_TRACE;
    StringSet result;
    if ( GUCEF_NULL == m_string || 0u == m_byteSize )
        return result;
    UInt32 totalCU = ( m_byteSize / sizeof( UInt16 ) ) - 1u;
    return ParseUniqueElements( m_string, totalCU, separator, addEmptyElements );
}

/*-------------------------------------------------------------------------*/

/* static */
CUtf16String::StringSet
CUtf16String::ParseUniqueElements( const UInt16* bufferPtr ,
                                    UInt32 bufferCodeUnits  ,
                                    UInt32 separator        ,
                                    bool addEmptyElements   )
{GUCEF_TRACE;
    StringSet result;
    StringVector v = ParseElements( bufferPtr, bufferCodeUnits, separator, addEmptyElements );
    for ( UInt32 i = 0u; i < (UInt32)v.size(); ++i )
        result.insert( v[ i ] );
    return result;
}

/* --- Wildcard / Equals -------------------------------------------------- */

bool
CUtf16String::Equals( const CUtf16String& otherStr ,
                      bool caseSensitive            ) const
{GUCEF_TRACE;
    if ( caseSensitive )
        return *this == otherStr;
    return Lowercase() == otherStr.Lowercase();
}

/*-------------------------------------------------------------------------*/

bool
CUtf16String::NotEquals( const CUtf16String& otherStr ,
                          bool caseSensitive           ) const
{GUCEF_TRACE;
    return !Equals( otherStr, caseSensitive );
}

/*-------------------------------------------------------------------------*/

bool
CUtf16String::WildcardEquals( const CUtf16String& strWithWildcards  ,
                               UInt32 wildCardToken                  ,
                               bool caseSensitive                    ,
                               bool biDirectional                    ) const
{GUCEF_TRACE;
    const CUtf16String& a = caseSensitive ? *this             : Lowercase();
    const CUtf16String& b = caseSensitive ? strWithWildcards  : strWithWildcards.Lowercase();

    StringVector parts = b.ParseElements( wildCardToken, true );
    if ( parts.empty() )
        return true;

    UInt32 aPos = 0u;
    for ( UInt32 i = 0u; i < (UInt32)parts.size(); ++i )
    {
        if ( parts[ i ].IsNULLOrEmpty() )
            continue;
        Int32 found = a.HasSubstr( parts[ i ], (Int32)aPos, true, true );
        if ( found < 0 )
        {
            if ( biDirectional )
                return strWithWildcards.WildcardEquals( *this, wildCardToken, caseSensitive, false );
            return false;
        }
        aPos = (UInt32)found + parts[ i ].m_length;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUtf16String::WildcardEquals( const StringSet& strsWithWildcards         ,
                               UInt32 wildCardToken                       ,
                               bool caseSensitive                         ,
                               bool biDirectional                         ) const
{GUCEF_TRACE;
    for ( StringSet::const_iterator it = strsWithWildcards.begin(); it != strsWithWildcards.end(); ++it )
    {
        if ( WildcardEquals( *it, wildCardToken, caseSensitive, biDirectional ) )
            return true;
    }
    return false;
}

/* --- Combine ------------------------------------------------------------ */

CUtf16String
CUtf16String::Combine( const StringVector& elements, UInt32 separator ) const
{GUCEF_TRACE;
    CUtf16String result;
    for ( UInt32 i = 0u; i < (UInt32)elements.size(); ++i )
    {
        if ( i > 0u )
            result.Append( separator );
        result.Append( elements[ i ] );
    }
    return result;
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16String::Combine( const StringSet& elements, UInt32 separator ) const
{GUCEF_TRACE;
    CUtf16String result;
    bool first = true;
    for ( StringSet::const_iterator it = elements.begin(); it != elements.end(); ++it )
    {
        if ( !first )
            result.Append( separator );
        result.Append( *it );
        first = false;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16String::Combine( const StringMap& elements, UInt32 valueSeperator, UInt32 kvSeperator ) const
{GUCEF_TRACE;
    CUtf16String result;
    bool first = true;
    for ( StringMap::const_iterator it = elements.begin(); it != elements.end(); ++it )
    {
        if ( !first )
            result.Append( kvSeperator );
        result.Append( it->first );
        result.Append( valueSeperator );
        result.Append( it->second );
        first = false;
    }
    return result;
}

/* --- IsFormattingValid -------------------------------------------------- */

bool
CUtf16String::IsFormattingValid( void ) const
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string || 0u == m_byteSize )
        return true;
    UInt32 totalCU = ( m_byteSize / sizeof( UInt16 ) ) - 1u;
    for ( UInt32 i = 0u; i < totalCU; ++i )
    {
        UInt16 cu = m_string[ i ];
        if ( IsHighSurrogate( cu ) )
        {
            if ( i + 1u >= totalCU || !IsLowSurrogate( m_string[ i + 1u ] ) )
                return false;
            ++i; /* skip low surrogate */
        }
        else if ( IsLowSurrogate( cu ) )
        {
            /* Lone low surrogate */
            return false;
        }
    }
    return true;
}

/* --- ForceToAscii ------------------------------------------------------- */

CAsciiString
CUtf16String::ForceToAscii( char asciiReplacement ) const
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_string || 0u == m_length )
        return CAsciiString();

    std::string out;
    out.reserve( m_length );
    const UInt16* p = m_string;
    while ( 0u != *p )
    {
        UInt32 cp = Utf16DecodeOne( p );
        out += ( cp < 0x80u ) ? static_cast<char>( cp ) : asciiReplacement;
    }
    return CAsciiString( out.c_str() );
}

/* --- Static I/O -------------------------------------------------------- */

/* static */
CUtf16String
CUtf16String::ReadLine( CIOAccess* io )
{GUCEF_TRACE;
    if ( GUCEF_NULL == io )
        return CUtf16String();

    CUtf16String result;
    UInt16 cu = 0u;
    while ( 1 == io->Read( &cu, sizeof( UInt16 ), 1 ) )
    {
        if ( cu == static_cast<UInt16>( '\n' ) )
            break;
        if ( cu == static_cast<UInt16>( '\r' ) )
            continue;
        UInt16 tmp[ 1 ] = { cu };
        result.Append( tmp, 1u );
    }
    return result;
}

/*-------------------------------------------------------------------------*/

/* static */
CUtf16String
CUtf16String::ReadString( CIOAccess* io )
{GUCEF_TRACE;
    if ( GUCEF_NULL == io )
        return CUtf16String();

    CUtf16String result;
    UInt16 cu = 0u;
    while ( 1 == io->Read( &cu, sizeof( UInt16 ), 1 ) )
    {
        if ( 0u == cu )
            break;
        UInt16 tmp[ 1 ] = { cu };
        result.Append( tmp, 1u );
    }
    return result;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      FREE FUNCTIONS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
operator!=( const char* lhs, const CUtf16String& rhs )
{GUCEF_TRACE;
    return !( rhs == lhs );
}

/*-------------------------------------------------------------------------*/

bool
operator==( const char* lhs, const CUtf16String& rhs )
{GUCEF_TRACE;
    return rhs == lhs;
}

/*-------------------------------------------------------------------------*/

CUtf16String
operator+( const char* lhs, const CUtf16String& rhs )
{GUCEF_TRACE;
    CUtf16String result( lhs );
    result.Append( rhs );
    return result;
}

/*-------------------------------------------------------------------------*/

CUtf16String
operator+( const char lhs, const CUtf16String& rhs )
{GUCEF_TRACE;
    CUtf16String result( static_cast<UInt32>( static_cast<unsigned char>( lhs ) ) );
    result.Append( rhs );
    return result;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
