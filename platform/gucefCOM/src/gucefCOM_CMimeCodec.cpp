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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#include "gucefCOM_CMimeCodec.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef GUCEF::vector< char > TCharVector;

/*-------------------------------------------------------------------------*/

inline UInt32
ByteLength( const CORE::CString& text )
{
    return text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
}

/*-------------------------------------------------------------------------*/

inline CORE::CString
MakeString( const TCharVector& chars )
{
    return chars.empty() ? CORE::CString() : CORE::CString( &chars[ 0 ], (UInt32) chars.size() );
}

/*-------------------------------------------------------------------------*/

inline void
PutBytes( TCharVector& chars, const char* data, UInt32 size )
{
    if ( size > 0 )
        chars.insert( chars.end(), data, data + size );
}

/*-------------------------------------------------------------------------*/

inline bool
IsWhite( char c )
{
    return ' ' == c || '\t' == c;
}

/*-------------------------------------------------------------------------*/

inline char
ToLowerAscii( char c )
{
    return ( c >= 'A' && c <= 'Z' ) ? (char)( c - 'A' + 'a' ) : c;
}

/*-------------------------------------------------------------------------*/

inline int
HexValue( char c )
{
    if ( c >= '0' && c <= '9' ) return c - '0';
    if ( c >= 'A' && c <= 'F' ) return c - 'A' + 10;
    if ( c >= 'a' && c <= 'f' ) return c - 'a' + 10;
    return -1;
}

/*-------------------------------------------------------------------------*/

/** Batches bytes so that a large decode does not append byte by byte to the buffer */
class CChunkWriter
{
    public:

    CChunkWriter( CORE::CDynamicBuffer& out, UInt32 maxSize )
        : m_out( out ), m_used( 0 ), m_total( out.GetDataSize() ), m_maxSize( maxSize ), m_overflow( false )
    {
        if ( CORE::CDynamicBuffer::GROWTHSTRATEGY_EXACT == m_out.GetGrowthStrategy() )
            m_out.SetGrowthStrategy( CORE::CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC );
    }

    bool Put( char c )
    {
        if ( m_total >= m_maxSize )
        {
            m_overflow = true;
            return false;
        }
        if ( m_used == sizeof( m_chunk ) )
            Flush();
        m_chunk[ m_used++ ] = c;
        ++m_total;
        return true;
    }

    void PutString( const char* s, UInt32 size )
    {
        for ( UInt32 i=0; i<size; ++i )
            Put( s[ i ] );
    }

    void Flush( void )
    {
        if ( m_used > 0 )
        {
            m_out.Append( m_chunk, m_used, true );
            m_used = 0;
        }
    }

    bool Finish( void )
    {
        Flush();
        return !m_overflow;
    }

    private:

    CORE::CDynamicBuffer& m_out;
    char m_chunk[ 4096 ];
    UInt32 m_used;
    UInt32 m_total;
    UInt32 m_maxSize;
    bool m_overflow;
};

/*-------------------------------------------------------------------------*/

void
AppendUtf8CodePoint( TCharVector& out, UInt32 cp )
{
    if ( cp < 0x80 )
    {
        out.push_back( (char) cp );
    }
    else
    if ( cp < 0x800 )
    {
        out.push_back( (char)( 0xC0 | ( cp >> 6 ) ) );
        out.push_back( (char)( 0x80 | ( cp & 0x3F ) ) );
    }
    else
    if ( cp < 0x10000 )
    {
        out.push_back( (char)( 0xE0 | ( cp >> 12 ) ) );
        out.push_back( (char)( 0x80 | ( ( cp >> 6 ) & 0x3F ) ) );
        out.push_back( (char)( 0x80 | ( cp & 0x3F ) ) );
    }
    else
    {
        out.push_back( (char)( 0xF0 | ( cp >> 18 ) ) );
        out.push_back( (char)( 0x80 | ( ( cp >> 12 ) & 0x3F ) ) );
        out.push_back( (char)( 0x80 | ( ( cp >> 6 ) & 0x3F ) ) );
        out.push_back( (char)( 0x80 | ( cp & 0x3F ) ) );
    }
}

/*-------------------------------------------------------------------------*/

/** @return the code point at p or 0xFFFFFFFF when the bytes there are not a valid UTF-8 sequence. 'consumed' is the number of bytes to skip, at least 1 */
UInt32
DecodeUtf8At( const UInt8* p, UInt32 remaining, UInt32& consumed )
{
    consumed = 1;
    const UInt8 b0 = p[ 0 ];
    if ( b0 < 0x80 )
        return b0;

    UInt32 needed = 0;
    UInt32 cp = 0;
    UInt32 minimum = 0;
    if ( b0 >= 0xC2 && b0 <= 0xDF ) { needed = 1; cp = b0 & 0x1F; minimum = 0x80; }
    else if ( b0 >= 0xE0 && b0 <= 0xEF ) { needed = 2; cp = b0 & 0x0F; minimum = 0x800; }
    else if ( b0 >= 0xF0 && b0 <= 0xF4 ) { needed = 3; cp = b0 & 0x07; minimum = 0x10000; }
    else return 0xFFFFFFFF;

    if ( remaining < needed + 1 )
        return 0xFFFFFFFF;
    for ( UInt32 i=1; i<=needed; ++i )
    {
        if ( 0x80 != ( p[ i ] & 0xC0 ) )
            return 0xFFFFFFFF;
        cp = ( cp << 6 ) | ( p[ i ] & 0x3F );
    }
    if ( cp < minimum || cp > 0x10FFFF || ( cp >= 0xD800 && cp <= 0xDFFF ) )
        return 0xFFFFFFFF;
    consumed = needed + 1;
    return cp;
}

/*-------------------------------------------------------------------------*/

void
SanitizeUtf8Into( const char* data, UInt32 size, TCharVector& out )
{
    const UInt8* p = reinterpret_cast< const UInt8* >( data );
    UInt32 i = 0;
    while ( i < size )
    {
        UInt32 consumed = 1;
        UInt32 cp = DecodeUtf8At( p + i, size - i, consumed );
        if ( 0xFFFFFFFF == cp )
        {
            AppendUtf8CodePoint( out, 0xFFFD );
            ++i;
        }
        else
        {
            if ( 0 == cp )
                AppendUtf8CodePoint( out, 0xFFFD );     // a NUL cannot live in a string
            else
            if ( cp < 0x80 )
                out.push_back( (char) cp );
            else
                PutBytes( out, data + i, consumed );
            i += consumed;
        }
    }
}

/*-------------------------------------------------------------------------*/

const UInt16 Windows1252High[ 32 ] =
{
    0x20AC, 0xFFFD, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0xFFFD, 0x017D, 0xFFFD,
    0xFFFD, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0xFFFD, 0x017E, 0x0178
};

/*-------------------------------------------------------------------------*/

enum ECharset
{
    CHARSET_UTF8,
    CHARSET_ASCII,
    CHARSET_LATIN1,
    CHARSET_LATIN9,
    CHARSET_WIN1252,
    CHARSET_UTF16,
    CHARSET_UTF16LE,
    CHARSET_UTF16BE,
    CHARSET_UNKNOWN
};

/*-------------------------------------------------------------------------*/

ECharset
IdentifyCharset( const CORE::CString& charsetName )
{
    // the RFC 2231 language suffix "*en" is not part of the name
    CORE::CString name = charsetName.Lowercase();
    Int32 star = name.HasChar( '*' );
    if ( star >= 0 )
        name = name.SubstrToIndex( (UInt32) star, true );

    if ( name == "utf-8" || name == "utf8" ) return CHARSET_UTF8;
    if ( name == "us-ascii" || name == "ascii" || name == "ansi_x3.4-1968" ) return CHARSET_ASCII;
    if ( name == "iso-8859-1" || name == "iso8859-1" || name == "latin1" || name == "iso_8859-1" || name == "l1" ) return CHARSET_LATIN1;
    if ( name == "iso-8859-15" || name == "iso8859-15" || name == "latin9" ) return CHARSET_LATIN9;
    if ( name == "windows-1252" || name == "cp1252" ) return CHARSET_WIN1252;
    if ( name == "utf-16" ) return CHARSET_UTF16;
    if ( name == "utf-16le" ) return CHARSET_UTF16LE;
    if ( name == "utf-16be" ) return CHARSET_UTF16BE;
    return CHARSET_UNKNOWN;
}

/*-------------------------------------------------------------------------*/

bool
IsMimeTokenChar( char c )
{
    // RFC 2045 token: any printable ASCII except space, controls and tspecials
    if ( c <= ' ' || c >= 0x7F )
        return false;
    return NULL == strchr( "()<>@,;:\\\"/[]?=", c );
}

/*-------------------------------------------------------------------------*/

bool
IsAttributeChar( char c )
{
    // RFC 2231 attribute-char: token characters except * ' %
    return IsMimeTokenChar( c ) && '*' != c && '\'' != c && '%' != c;
}

/*-------------------------------------------------------------------------*/

const char* const MonthNames[ 12 ] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
const char* const DayNames[ 7 ] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

/*-------------------------------------------------------------------------*/

int
DayOfWeek( int year, int month, int day )
{
    static const int t[ 12 ] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
    if ( month < 3 )
        --year;
    return ( year + year / 4 - year / 100 + year / 400 + t[ month - 1 ] + day ) % 7;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CMimeCodec::AppendBase64Wrapped( CORE::CDynamicBuffer& out, const void* data, UInt32 size, UInt32 lineLength )
{GUCEF_TRACE;

    if ( 0 == size || GUCEF_NULL == data )
        return;
    if ( lineLength < 4 )
        lineLength = 4;
    lineLength -= lineLength % 4;

    CORE::CString encoded = CORE::Base64Encode( data, size );
    const char* text = encoded.C_String();
    const UInt32 length = ByteLength( encoded );

    CChunkWriter writer( out, GUCEF_UINT32MAX );
    for ( UInt32 i=0; i<length; i+=lineLength )
    {
        UInt32 count = length - i < lineLength ? length - i : lineLength;
        writer.PutString( text + i, count );
        writer.Put( '\r' );
        writer.Put( '\n' );
    }
    writer.Finish();
}

/*-------------------------------------------------------------------------*/

bool
CMimeCodec::DecodeBase64Lenient( const char* text, UInt32 size, CORE::CDynamicBuffer& out, UInt32 maxSize, bool* hadErrors )
{GUCEF_TRACE;

    if ( GUCEF_NULL != hadErrors )
        *hadErrors = false;

    CChunkWriter writer( out, maxSize );
    UInt32 accumulator = 0;
    UInt32 bits = 0;
    for ( UInt32 i=0; i<size; ++i )
    {
        const char c = text[ i ];
        int value = -1;
        if ( c >= 'A' && c <= 'Z' ) value = c - 'A';
        else if ( c >= 'a' && c <= 'z' ) value = c - 'a' + 26;
        else if ( c >= '0' && c <= '9' ) value = c - '0' + 52;
        else if ( '+' == c || '-' == c ) value = 62;        // base64url is tolerated
        else if ( '/' == c || '_' == c ) value = 63;
        else if ( '=' == c ) break;
        else if ( ' ' == c || '\t' == c || '\r' == c || '\n' == c ) continue;
        else
        {
            if ( GUCEF_NULL != hadErrors )
                *hadErrors = true;
            continue;
        }

        accumulator = ( accumulator << 6 ) | (UInt32) value;
        bits += 6;
        if ( bits >= 8 )
        {
            bits -= 8;
            if ( !writer.Put( (char)( ( accumulator >> bits ) & 0xFF ) ) )
                break;
        }
    }
    return writer.Finish();
}

/*-------------------------------------------------------------------------*/

void
CMimeCodec::AppendQuotedPrintable( CORE::CDynamicBuffer& out, const char* text, UInt32 size )
{GUCEF_TRACE;

    static const char hex[] = "0123456789ABCDEF";
    CChunkWriter writer( out, GUCEF_UINT32MAX );
    UInt32 column = 0;

    UInt32 i = 0;
    while ( i < size )
    {
        char c = text[ i ];

        // line ends
        if ( '\r' == c || '\n' == c )
        {
            if ( '\r' == c && i + 1 < size && '\n' == text[ i + 1 ] )
                ++i;
            writer.Put( '\r' );
            writer.Put( '\n' );
            column = 0;
            ++i;
            continue;
        }

        // what the byte becomes
        bool literal = ( c >= 33 && c <= 126 && '=' != c ) || ( ( ' ' == c || '\t' == c ) && i + 1 < size && '\r' != text[ i + 1 ] && '\n' != text[ i + 1 ] );
        UInt32 needed = literal ? 1 : 3;

        // a soft break keeps the line within 76 characters, the '=' of the break included
        if ( column + needed > 75 )
        {
            writer.Put( '=' );
            writer.Put( '\r' );
            writer.Put( '\n' );
            column = 0;
        }

        if ( literal )
        {
            writer.Put( c );
        }
        else
        {
            writer.Put( '=' );
            writer.Put( hex[ ( (UInt8) c ) >> 4 ] );
            writer.Put( hex[ ( (UInt8) c ) & 0x0F ] );
        }
        column += needed;
        ++i;
    }
    writer.Finish();
}

/*-------------------------------------------------------------------------*/

bool
CMimeCodec::DecodeQuotedPrintable( const char* text, UInt32 size, CORE::CDynamicBuffer& out, bool qEncoding, UInt32 maxSize )
{GUCEF_TRACE;

    CChunkWriter writer( out, maxSize );
    TCharVector lineWhite;      // white space that may turn out to be padding in front of a line end
    UInt32 i = 0;
    while ( i < size )
    {
        const char c = text[ i ];
        if ( '=' == c )
        {
            // soft line break
            UInt32 j = i + 1;
            while ( !qEncoding && j < size && IsWhite( text[ j ] ) )
                ++j;
            if ( j < size && '\n' == text[ j ] )
            {
                i = j + 1;
                continue;
            }
            if ( j + 1 < size && '\r' == text[ j ] && '\n' == text[ j + 1 ] )
            {
                i = j + 2;
                continue;
            }
            if ( i + 2 < size && HexValue( text[ i + 1 ] ) >= 0 && HexValue( text[ i + 2 ] ) >= 0 )
            {
                for ( size_t w=0; w<lineWhite.size(); ++w )
                    writer.Put( lineWhite[ w ] );
                lineWhite.clear();
                if ( !writer.Put( (char)( HexValue( text[ i + 1 ] ) * 16 + HexValue( text[ i + 2 ] ) ) ) )
                    break;
                i += 3;
                continue;
            }
            // not an escape: kept
            for ( size_t w=0; w<lineWhite.size(); ++w )
                writer.Put( lineWhite[ w ] );
            lineWhite.clear();
            if ( !writer.Put( '=' ) )
                break;
            ++i;
            continue;
        }

        if ( qEncoding )
        {
            if ( !writer.Put( '_' == c ? ' ' : c ) )
                break;
            ++i;
            continue;
        }

        if ( IsWhite( c ) )
        {
            lineWhite.push_back( c );
            ++i;
            continue;
        }

        if ( '\r' == c || '\n' == c )
        {
            // padding at the end of a line is not part of the data
            lineWhite.clear();
            if ( !writer.Put( c ) )
                break;
            ++i;
            continue;
        }

        for ( size_t w=0; w<lineWhite.size(); ++w )
            writer.Put( lineWhite[ w ] );
        lineWhite.clear();
        if ( !writer.Put( c ) )
            break;
        ++i;
    }
    for ( size_t w=0; w<lineWhite.size(); ++w )
        writer.Put( lineWhite[ w ] );
    return writer.Finish();
}

/*-------------------------------------------------------------------------*/

bool
CMimeCodec::IsAscii( const char* data, UInt32 size )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<size; ++i )
        if ( ( (UInt8) data[ i ] ) >= 0x80 )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMimeCodec::HasLineBreakOrNul( const CORE::CString& text )
{GUCEF_TRACE;

    const char* p = text.C_String();
    const UInt32 size = ByteLength( text );
    for ( UInt32 i=0; i<size; ++i )
        if ( '\r' == p[ i ] || '\n' == p[ i ] || '\0' == p[ i ] )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CMimeCodec::HasControlCharacter( const CORE::CString& text )
{GUCEF_TRACE;

    const char* p = text.C_String();
    const UInt32 size = ByteLength( text );
    for ( UInt32 i=0; i<size; ++i )
    {
        const UInt8 c = (UInt8) p[ i ];
        if ( c < 0x20 || 0x7F == c )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMimeCodec::SanitizeUtf8( const char* data, UInt32 size )
{GUCEF_TRACE;

    TCharVector out;
    out.reserve( size );
    SanitizeUtf8Into( data, size, out );
    return MakeString( out );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMimeCodec::ToUtf8( const CORE::CString& charset, const char* data, UInt32 size, bool& lossy )
{GUCEF_TRACE;

    lossy = false;
    TCharVector out;
    out.reserve( size + size / 4 );
    const UInt8* p = reinterpret_cast< const UInt8* >( data );

    switch ( IdentifyCharset( charset ) )
    {
        case CHARSET_UTF8:
        {
            SanitizeUtf8Into( data, size, out );
            break;
        }
        case CHARSET_ASCII:
        {
            for ( UInt32 i=0; i<size; ++i )
            {
                if ( p[ i ] < 0x80 && 0 != p[ i ] )
                    out.push_back( (char) p[ i ] );
                else
                {
                    AppendUtf8CodePoint( out, 0xFFFD );
                    lossy = true;
                }
            }
            break;
        }
        case CHARSET_LATIN1:
        case CHARSET_LATIN9:
        case CHARSET_WIN1252:
        {
            const ECharset set = IdentifyCharset( charset );
            for ( UInt32 i=0; i<size; ++i )
            {
                UInt32 cp = p[ i ];
                if ( 0 == cp )
                    cp = 0xFFFD;
                else
                if ( CHARSET_WIN1252 == set && cp >= 0x80 && cp <= 0x9F )
                    cp = Windows1252High[ cp - 0x80 ];
                else
                if ( CHARSET_LATIN9 == set )
                {
                    switch ( cp )
                    {
                        case 0xA4: cp = 0x20AC; break;
                        case 0xA6: cp = 0x0160; break;
                        case 0xA8: cp = 0x0161; break;
                        case 0xB4: cp = 0x017D; break;
                        case 0xB8: cp = 0x017E; break;
                        case 0xBC: cp = 0x0152; break;
                        case 0xBD: cp = 0x0153; break;
                        case 0xBE: cp = 0x0178; break;
                        default: break;
                    }
                }
                AppendUtf8CodePoint( out, cp );
            }
            break;
        }
        case CHARSET_UTF16:
        case CHARSET_UTF16LE:
        case CHARSET_UTF16BE:
        {
            ECharset set = IdentifyCharset( charset );
            UInt32 i = 0;
            bool littleEndian = CHARSET_UTF16LE == set;
            if ( CHARSET_UTF16 == set && size >= 2 )
            {
                // a byte order mark decides, RFC 2781 says big endian without
                if ( 0xFF == p[ 0 ] && 0xFE == p[ 1 ] ) { littleEndian = true; i = 2; }
                else if ( 0xFE == p[ 0 ] && 0xFF == p[ 1 ] ) { i = 2; }
            }
            while ( i + 1 < size )
            {
                UInt32 unit = littleEndian ? ( p[ i ] | ( p[ i + 1 ] << 8 ) ) : ( ( p[ i ] << 8 ) | p[ i + 1 ] );
                i += 2;
                UInt32 cp = unit;
                if ( unit >= 0xD800 && unit <= 0xDBFF )
                {
                    UInt32 low = 0;
                    if ( i + 1 < size )
                        low = littleEndian ? ( p[ i ] | ( p[ i + 1 ] << 8 ) ) : ( ( p[ i ] << 8 ) | p[ i + 1 ] );
                    if ( low >= 0xDC00 && low <= 0xDFFF )
                    {
                        cp = 0x10000 + ( ( unit - 0xD800 ) << 10 ) + ( low - 0xDC00 );
                        i += 2;
                    }
                    else
                    {
                        cp = 0xFFFD;
                        lossy = true;
                    }
                }
                else
                if ( unit >= 0xDC00 && unit <= 0xDFFF )
                {
                    cp = 0xFFFD;
                    lossy = true;
                }
                if ( 0 == cp )
                    cp = 0xFFFD;
                AppendUtf8CodePoint( out, cp );
            }
            if ( i < size )
            {
                AppendUtf8CodePoint( out, 0xFFFD );
                lossy = true;
            }
            break;
        }
        case CHARSET_UNKNOWN:
        default:
        {
            lossy = true;
            SanitizeUtf8Into( data, size, out );
            break;
        }
    }
    return MakeString( out );
}

/*-------------------------------------------------------------------------*/

bool
CMimeCodec::IsPlainHeaderText( const CORE::CString& text )
{GUCEF_TRACE;

    const char* p = text.C_String();
    const UInt32 size = ByteLength( text );
    for ( UInt32 i=0; i<size; ++i )
    {
        const UInt8 c = (UInt8) p[ i ];
        if ( ( c < 0x20 && '\t' != c ) || c >= 0x7F )
            return false;
        if ( '=' == c && i + 1 < size && '?' == p[ i + 1 ] )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMimeCodec::EncodeHeaderText( const CORE::CString& utf8Text )
{GUCEF_TRACE;

    if ( IsPlainHeaderText( utf8Text ) )
        return utf8Text;

    CORE::CString clean = SanitizeUtf8( utf8Text.C_String(), ByteLength( utf8Text ) );
    const char* p = clean.C_String();
    const UInt32 size = ByteLength( clean );

    // 45 bytes become 60 base64 characters, with the 12 of "=?UTF-8?B?" and "?=" that is 72 characters: below the limit of 75
    static const UInt32 maxChunk = 45;
    TCharVector result;
    UInt32 start = 0;
    while ( start < size )
    {
        UInt32 end = start + maxChunk < size ? start + maxChunk : size;
        // not in the middle of a character
        while ( end < size && end > start && 0x80 == ( ( (UInt8) p[ end ] ) & 0xC0 ) )
            --end;
        if ( end == start )
            end = size < start + maxChunk ? size : start + maxChunk;

        if ( !result.empty() )
            result.push_back( ' ' );
        CORE::CString word = "=?UTF-8?B?" + CORE::Base64Encode( p + start, end - start ) + "?=";
        PutBytes( result, word.C_String(), ByteLength( word ) );
        start = end;
    }
    return MakeString( result );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMimeCodec::DecodeHeaderText( const CORE::CString& wireText )
{GUCEF_TRACE;

    const char* p = wireText.C_String();
    const UInt32 size = ByteLength( wireText );
    if ( wireText.HasSubstr( "=?" ) < 0 )
        return SanitizeUtf8( p, size );

    TCharVector out;
    TCharVector pending;
    CORE::CString pendingCharset;
    TCharVector pendingWhite;
    bool lastWasEncoded = false;

    UInt32 i = 0;
    while ( i < size )
    {
        if ( '=' == p[ i ] && i + 1 < size && '?' == p[ i + 1 ] )
        {
            // =?charset?enc?text?=
            UInt32 charsetStart = i + 2;
            UInt32 charsetEnd = charsetStart;
            while ( charsetEnd < size && '?' != p[ charsetEnd ] && !IsWhite( p[ charsetEnd ] ) )
                ++charsetEnd;
            bool ok = charsetEnd + 3 < size && '?' == p[ charsetEnd ] && '?' == p[ charsetEnd + 2 ] && charsetEnd > charsetStart;
            char encoding = 0;
            UInt32 textStart = 0;
            UInt32 textEnd = 0;
            if ( ok )
            {
                encoding = ToLowerAscii( p[ charsetEnd + 1 ] );
                ok = 'b' == encoding || 'q' == encoding;
                textStart = charsetEnd + 3;
                textEnd = textStart;
                while ( textEnd + 1 < size && !( '?' == p[ textEnd ] && '=' == p[ textEnd + 1 ] ) && !IsWhite( p[ textEnd ] ) && '\r' != p[ textEnd ] && '\n' != p[ textEnd ] )
                    ++textEnd;
                ok = ok && textEnd + 1 < size && '?' == p[ textEnd ] && '=' == p[ textEnd + 1 ];
            }

            if ( ok )
            {
                CORE::CString charset( p + charsetStart, charsetEnd - charsetStart );
                CORE::CDynamicBuffer decoded;
                if ( 'b' == encoding )
                    DecodeBase64Lenient( p + textStart, textEnd - textStart, decoded, 65536 );
                else
                    DecodeQuotedPrintable( p + textStart, textEnd - textStart, decoded, true, 65536 );

                // adjacent words in the same character set are joined before conversion: a character may be split over two words
                if ( !pending.empty() && !pendingCharset.Equals( charset, false ) )
                {
                    bool lossy = false;
                    CORE::CString converted = ToUtf8( pendingCharset, &pending[ 0 ], (UInt32) pending.size(), lossy );
                    PutBytes( out, converted.C_String(), ByteLength( converted ) );
                    pending.clear();
                }
                pendingCharset = charset;
                PutBytes( pending, static_cast< const char* >( decoded.GetConstBufferPtr() ), decoded.GetDataSize() );
                pendingWhite.clear();       // white space between two encoded words is not part of the text
                lastWasEncoded = true;
                i = textEnd + 2;
                continue;
            }
        }

        const char c = p[ i ];
        if ( lastWasEncoded )
        {
            if ( IsWhite( c ) || '\r' == c || '\n' == c )
            {
                pendingWhite.push_back( c );
                ++i;
                continue;
            }
            // a word of the plain text follows: the encoded text is complete
            if ( !pending.empty() )
            {
                bool lossy = false;
                CORE::CString converted = ToUtf8( pendingCharset, &pending[ 0 ], (UInt32) pending.size(), lossy );
                PutBytes( out, converted.C_String(), ByteLength( converted ) );
                pending.clear();
            }
            PutBytes( out, pendingWhite.empty() ? "" : &pendingWhite[ 0 ], (UInt32) pendingWhite.size() );
            pendingWhite.clear();
            lastWasEncoded = false;
        }
        out.push_back( c );
        ++i;
    }

    if ( !pending.empty() )
    {
        bool lossy = false;
        CORE::CString converted = ToUtf8( pendingCharset, &pending[ 0 ], (UInt32) pending.size(), lossy );
        PutBytes( out, converted.C_String(), ByteLength( converted ) );
    }
    PutBytes( out, pendingWhite.empty() ? "" : &pendingWhite[ 0 ], (UInt32) pendingWhite.size() );

    if ( out.empty() )
        return CORE::CString();
    return SanitizeUtf8( &out[ 0 ], (UInt32) out.size() );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMimeCodec::Unfold( const char* data, UInt32 size )
{GUCEF_TRACE;

    TCharVector out;
    out.reserve( size );
    for ( UInt32 i=0; i<size; ++i )
        if ( '\r' != data[ i ] && '\n' != data[ i ] )
            out.push_back( data[ i ] );

    // leading and trailing white space is not part of the value
    size_t first = 0;
    size_t last = out.size();
    while ( first < last && IsWhite( out[ first ] ) )
        ++first;
    while ( last > first && IsWhite( out[ last - 1 ] ) )
        --last;
    return last > first ? CORE::CString( &out[ first ], (UInt32)( last - first ) ) : CORE::CString();
}

/*-------------------------------------------------------------------------*/

bool
CMimeCodec::IsValidHeaderName( const CORE::CString& name )
{GUCEF_TRACE;

    const UInt32 size = ByteLength( name );
    if ( 0 == size )
        return false;
    const char* p = name.C_String();
    for ( UInt32 i=0; i<size; ++i )
    {
        const UInt8 c = (UInt8) p[ i ];
        if ( c <= 0x20 || c >= 0x7F || ':' == c )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMimeCodec::AppendHeaderField( CORE::CDynamicBuffer& out, const CORE::CString& name, const CORE::CString& wireValue )
{GUCEF_TRACE;

    if ( !IsValidHeaderName( name ) || HasLineBreakOrNul( wireValue ) )
        return false;

    TCharVector line;
    const UInt32 nameSize = ByteLength( name );
    PutBytes( line, name.C_String(), nameSize );
    line.push_back( ':' );
    UInt32 column = nameSize + 1;

    const char* v = wireValue.C_String();
    const UInt32 valueSize = ByteLength( wireValue );
    UInt32 i = 0;
    while ( i < valueSize )
    {
        while ( i < valueSize && IsWhite( v[ i ] ) )
            ++i;
        UInt32 tokenStart = i;
        while ( i < valueSize && !IsWhite( v[ i ] ) )
            ++i;
        if ( i == tokenStart )
            break;
        const UInt32 tokenSize = i - tokenStart;

        if ( column + 1 + tokenSize > PreferredHeaderLineLength && column > 1 )
        {
            // folding in front of the first token is allowed too, a name that long is not worth a special case
            line.push_back( '\r' );
            line.push_back( '\n' );
            line.push_back( ' ' );
            column = 1;
        }
        else
        {
            line.push_back( ' ' );
            ++column;
        }
        PutBytes( line, v + tokenStart, tokenSize );
        column += tokenSize;
        if ( column > MaxHeaderLineLength )
            return false;
    }
    line.push_back( '\r' );
    line.push_back( '\n' );
    out.Append( &line[ 0 ], (UInt32) line.size(), true );
    return true;
}

/*-------------------------------------------------------------------------*/

namespace {

struct SRawParam
{
    CORE::CString baseName;
    Int32 section;          // -1: no section
    bool extended;
    TCharVector value;
};

/*-------------------------------------------------------------------------*/

void
DecodePercent( const char* p, UInt32 size, TCharVector& out )
{
    for ( UInt32 i=0; i<size; ++i )
    {
        if ( '%' == p[ i ] && i + 2 < size && HexValue( p[ i + 1 ] ) >= 0 && HexValue( p[ i + 2 ] ) >= 0 )
        {
            out.push_back( (char)( HexValue( p[ i + 1 ] ) * 16 + HexValue( p[ i + 2 ] ) ) );
            i += 2;
        }
        else
            out.push_back( p[ i ] );
    }
}

}

/*-------------------------------------------------------------------------*/

void
CMimeCodec::ParseValueWithParameters( const CORE::CString& wireValue, CORE::CString& value, TParamVector& params )
{GUCEF_TRACE;

    params.clear();
    const char* p = wireValue.C_String();
    const UInt32 size = ByteLength( wireValue );

    // the value in front of the first semicolon
    UInt32 i = 0;
    while ( i < size && ';' != p[ i ] )
    {
        if ( '"' == p[ i ] )
        {
            ++i;
            while ( i < size && '"' != p[ i ] )
                i += ( '\\' == p[ i ] && i + 1 < size ) ? 2 : 1;
        }
        if ( i < size )
            ++i;
    }
    UInt32 valueEnd = i < size ? i : size;
    value = Unfold( p, valueEnd );

    GUCEF::vector< SRawParam > raws;
    while ( i < size )
    {
        while ( i < size && ( ';' == p[ i ] || IsWhite( p[ i ] ) || '\r' == p[ i ] || '\n' == p[ i ] ) )
            ++i;
        UInt32 nameStart = i;
        while ( i < size && '=' != p[ i ] && ';' != p[ i ] )
            ++i;
        if ( i >= size || '=' != p[ i ] )
            continue;       // an attribute without value is ignored
        UInt32 nameEnd = i;
        while ( nameEnd > nameStart && ( IsWhite( p[ nameEnd - 1 ] ) || '\r' == p[ nameEnd - 1 ] || '\n' == p[ nameEnd - 1 ] ) )
            --nameEnd;
        CORE::CString fullName = CORE::CString( p + nameStart, nameEnd - nameStart ).Lowercase();
        ++i;
        while ( i < size && ( IsWhite( p[ i ] ) || '\r' == p[ i ] || '\n' == p[ i ] ) )
            ++i;

        SRawParam raw;
        if ( i < size && '"' == p[ i ] )
        {
            ++i;
            while ( i < size && '"' != p[ i ] )
            {
                if ( '\\' == p[ i ] && i + 1 < size )
                    ++i;
                if ( '\r' != p[ i ] && '\n' != p[ i ] )
                    raw.value.push_back( p[ i ] );
                ++i;
            }
            if ( i < size )
                ++i;
        }
        else
        {
            while ( i < size && ';' != p[ i ] && !IsWhite( p[ i ] ) && '\r' != p[ i ] && '\n' != p[ i ] )
                raw.value.push_back( p[ i++ ] );
        }

        // name, name*, name*0, name*0*
        raw.section = -1;
        raw.extended = false;
        raw.baseName = fullName;
        Int32 star = fullName.HasChar( '*' );
        if ( star >= 0 )
        {
            raw.baseName = fullName.SubstrToIndex( (UInt32) star, true );
            CORE::CString suffix = fullName.SubstrFromRange( (UInt32) star + 1, fullName.Length() );
            if ( suffix.EndsWith( "*" ) )
            {
                raw.extended = true;
                suffix = suffix.SubstrToIndex( suffix.Length() - 1, true );
            }
            if ( suffix.IsNULLOrEmpty() )
                raw.extended = true;    // name*
            else
            {
                raw.section = 0;
                const char* digits = suffix.C_String();
                for ( UInt32 d=0; d<ByteLength( suffix ) && d<6; ++d )
                    raw.section = raw.section * 10 + ( digits[ d ] >= '0' && digits[ d ] <= '9' ? digits[ d ] - '0' : 0 );
            }
        }
        raws.push_back( raw );
        if ( raws.size() > 256 )
            break;
    }

    // parameters in the order of their first appearance, sections joined
    GUCEF::set< CORE::CString > done;
    for ( size_t r=0; r<raws.size(); ++r )
    {
        if ( done.find( raws[ r ].baseName ) != done.end() )
            continue;
        done.insert( raws[ r ].baseName );

        GUCEF::map< Int32, size_t > sections;
        bool anyExtended = false;
        for ( size_t s=r; s<raws.size(); ++s )
        {
            if ( raws[ s ].baseName != raws[ r ].baseName )
                continue;
            if ( sections.find( raws[ s ].section ) == sections.end() )
                sections[ raws[ s ].section ] = s;
            anyExtended = anyExtended || raws[ s ].extended;
        }

        TCharVector bytes;
        CORE::CString charset;
        bool first = true;
        for ( GUCEF::map< Int32, size_t >::const_iterator s = sections.begin(); s != sections.end(); ++s )
        {
            const SRawParam& raw = raws[ (*s).second ];
            const char* v = raw.value.empty() ? "" : &raw.value[ 0 ];
            UInt32 vSize = (UInt32) raw.value.size();
            if ( raw.extended )
            {
                if ( first )
                {
                    // charset'language'text
                    UInt32 q1 = 0;
                    while ( q1 < vSize && '\'' != v[ q1 ] )
                        ++q1;
                    UInt32 q2 = q1 + 1;
                    while ( q2 < vSize && '\'' != v[ q2 ] )
                        ++q2;
                    if ( q1 < vSize && q2 < vSize )
                    {
                        charset = CORE::CString( v, q1 );
                        v += q2 + 1;
                        vSize -= q2 + 1;
                    }
                }
                DecodePercent( v, vSize, bytes );
            }
            else
                PutBytes( bytes, v, vSize );
            first = false;
        }

        TParam param;
        param.name = raws[ r ].baseName;
        if ( anyExtended )
        {
            bool lossy = false;
            param.value = ToUtf8( charset.IsNULLOrEmpty() ? CORE::CString( "utf-8" ) : charset, bytes.empty() ? "" : &bytes[ 0 ], (UInt32) bytes.size(), lossy );
        }
        else
        {
            // a plain value; names of files that carry encoded words are decoded as mail clients do
            param.value = SanitizeUtf8( bytes.empty() ? "" : &bytes[ 0 ], (UInt32) bytes.size() );
            if ( ( param.name == "filename" || param.name == "name" ) && param.value.HasSubstr( "=?" ) >= 0 )
                param.value = DecodeHeaderText( param.value );
        }
        params.push_back( param );
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMimeCodec::FindParameter( const TParamVector& params, const CORE::CString& lowerCaseName )
{GUCEF_TRACE;

    for ( size_t i=0; i<params.size(); ++i )
        if ( params[ i ].name == lowerCaseName )
            return params[ i ].value;
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMimeCodec::FormatParameter( const CORE::CString& name, const CORE::CString& utf8Value )
{GUCEF_TRACE;

    const char* v = utf8Value.C_String();
    const UInt32 size = ByteLength( utf8Value );

    bool plain = true;
    bool isToken = size > 0;
    for ( UInt32 i=0; i<size; ++i )
    {
        const UInt8 c = (UInt8) v[ i ];
        if ( c < 0x20 || c >= 0x7F )
            plain = false;
        if ( !IsMimeTokenChar( (char) c ) )
            isToken = false;
    }

    TCharVector out;
    PutBytes( out, name.C_String(), ByteLength( name ) );

    if ( plain && isToken )
    {
        out.push_back( '=' );
        PutBytes( out, v, size );
    }
    else
    if ( plain )
    {
        out.push_back( '=' );
        out.push_back( '"' );
        for ( UInt32 i=0; i<size; ++i )
        {
            if ( '"' == v[ i ] || '\\' == v[ i ] )
                out.push_back( '\\' );
            out.push_back( v[ i ] );
        }
        out.push_back( '"' );
    }
    else
    {
        // RFC 2231: percent encoded UTF-8, in sections of at most 50 encoded characters when long
        static const char hex[] = "0123456789ABCDEF";
        CORE::CString clean = SanitizeUtf8( v, size );
        v = clean.C_String();
        const UInt32 cleanSize = ByteLength( clean );
        TCharVector encoded;
        GUCEF::vector< UInt32 > sectionEnds;
        UInt32 sectionStart = 0;
        for ( UInt32 i=0; i<cleanSize; ++i )
        {
            if ( IsAttributeChar( v[ i ] ) )
                encoded.push_back( v[ i ] );
            else
            {
                encoded.push_back( '%' );
                encoded.push_back( hex[ ( (UInt8) v[ i ] ) >> 4 ] );
                encoded.push_back( hex[ ( (UInt8) v[ i ] ) & 0x0F ] );
            }
            // end a section after a whole character
            bool endOfCharacter = i + 1 >= cleanSize || 0x80 != ( ( (UInt8) v[ i + 1 ] ) & 0xC0 );
            if ( endOfCharacter && encoded.size() - sectionStart >= 40 && i + 1 < cleanSize )
            {
                sectionEnds.push_back( (UInt32) encoded.size() );
                sectionStart = (UInt32) encoded.size();
            }
        }
        sectionEnds.push_back( (UInt32) encoded.size() );

        if ( 1 == sectionEnds.size() )
        {
            PutBytes( out, "*=utf-8''", 9 );
            PutBytes( out, encoded.empty() ? "" : &encoded[ 0 ], (UInt32) encoded.size() );
        }
        else
        {
            out.clear();
            UInt32 start = 0;
            for ( size_t s=0; s<sectionEnds.size(); ++s )
            {
                if ( s > 0 )
                {
                    out.push_back( ';' );
                    out.push_back( ' ' );
                }
                PutBytes( out, name.C_String(), ByteLength( name ) );
                out.push_back( '*' );
                CORE::CString index = CORE::UInt32ToString( (UInt32) s );
                PutBytes( out, index.C_String(), ByteLength( index ) );
                PutBytes( out, "*=", 2 );
                if ( 0 == s )
                    PutBytes( out, "utf-8''", 7 );
                PutBytes( out, &encoded[ start ], sectionEnds[ s ] - start );
                start = sectionEnds[ s ];
            }
        }
    }
    return MakeString( out );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMimeCodec::FormatDate( const CORE::CDateTime& date )
{GUCEF_TRACE;

    int year = date.GetYear();
    int month = date.GetMonth();
    int day = date.GetDay();
    if ( month < 1 || month > 12 ) month = 1;
    if ( day < 1 || day > 31 ) day = 1;
    if ( year < 1900 ) year = 1970;
    const int offset = date.GetTimeZoneUTCOffsetInMins();
    const int absOffset = offset < 0 ? -offset : offset;

    // two digits, with a leading zero
    #define GUCEF_COM_TWO_DIGITS( value ) ( ( (value) < 10 ? CORE::CString( "0" ) : CORE::CString() ) + CORE::Int32ToString( value ) )
    CORE::CString yearText = CORE::Int32ToString( year );
    while ( yearText.Length() < 4 )
        yearText = "0" + yearText;
    const CORE::CString text = CORE::CString( DayNames[ DayOfWeek( year, month, day ) ] ) + ", " + GUCEF_COM_TWO_DIGITS( day ) + " " + MonthNames[ month - 1 ] + " " + yearText + " " +
                               GUCEF_COM_TWO_DIGITS( (int) date.GetHours() ) + ":" + GUCEF_COM_TWO_DIGITS( (int) date.GetMinutes() ) + ":" + GUCEF_COM_TWO_DIGITS( (int) date.GetSeconds() ) +
                               " " + ( offset < 0 ? "-" : "+" ) + GUCEF_COM_TWO_DIGITS( absOffset / 60 ) + GUCEF_COM_TWO_DIGITS( absOffset % 60 );
    #undef GUCEF_COM_TWO_DIGITS
    return text;
}

/*-------------------------------------------------------------------------*/

bool
CMimeCodec::ParseDate( const CORE::CString& text, CORE::CDateTime& date )
{GUCEF_TRACE;

    // comments and commas out, then split at white space
    const char* p = text.C_String();
    const UInt32 size = ByteLength( text );
    TCharVector clean;
    int depth = 0;
    for ( UInt32 i=0; i<size; ++i )
    {
        if ( '(' == p[ i ] ) { ++depth; continue; }
        if ( ')' == p[ i ] ) { if ( depth > 0 ) --depth; continue; }
        if ( depth > 0 ) continue;
        clean.push_back( ( ',' == p[ i ] || '\t' == p[ i ] || '\r' == p[ i ] || '\n' == p[ i ] ) ? ' ' : p[ i ] );
    }
    if ( clean.empty() )
        return false;

    GUCEF::vector< CORE::CString > tokens;
    size_t i = 0;
    while ( i < clean.size() )
    {
        while ( i < clean.size() && ' ' == clean[ i ] )
            ++i;
        size_t start = i;
        while ( i < clean.size() && ' ' != clean[ i ] )
            ++i;
        if ( i > start )
            tokens.push_back( CORE::CString( &clean[ start ], (UInt32)( i - start ) ) );
        if ( tokens.size() > 12 )
            return false;
    }

    size_t t = 0;
    // optional day of week
    if ( t < tokens.size() && ByteLength( tokens[ t ] ) >= 3 && !( tokens[ t ].C_String()[ 0 ] >= '0' && tokens[ t ].C_String()[ 0 ] <= '9' ) )
    {
        bool isDay = false;
        for ( int d=0; d<7; ++d )
            if ( tokens[ t ].SubstrToIndex( 3, true ).Equals( DayNames[ d ], false ) )
                isDay = true;
        if ( isDay )
            ++t;
    }
    if ( tokens.size() < t + 4 )
        return false;

    const char* dayText = tokens[ t ].C_String();
    int day = 0;
    UInt32 dayDigits = 0;
    while ( dayText[ dayDigits ] >= '0' && dayText[ dayDigits ] <= '9' && dayDigits < 3 )
        day = day * 10 + ( dayText[ dayDigits++ ] - '0' );
    if ( 0 == dayDigits || dayDigits > 2 || '\0' != dayText[ dayDigits ] )
        return false;
    ++t;

    int month = 0;
    if ( ByteLength( tokens[ t ] ) >= 3 )
    {
        CORE::CString m = tokens[ t ].SubstrToIndex( 3, true );
        for ( int k=0; k<12; ++k )
            if ( m.Equals( MonthNames[ k ], false ) )
                month = k + 1;
    }
    if ( 0 == month )
        return false;
    ++t;

    const char* yearText = tokens[ t ].C_String();
    int year = 0;
    UInt32 yearDigits = 0;
    while ( yearText[ yearDigits ] >= '0' && yearText[ yearDigits ] <= '9' && yearDigits < 5 )
        year = year * 10 + ( yearText[ yearDigits++ ] - '0' );
    if ( yearDigits < 2 || yearDigits > 4 || '\0' != yearText[ yearDigits ] )
        return false;
    if ( 2 == yearDigits ) year += year < 50 ? 2000 : 1900;
    else if ( 3 == yearDigits ) year += 1900;
    ++t;

    // hh:mm[:ss]
    if ( t >= tokens.size() )
        return false;
    int clock[ 3 ] = { 0, 0, 0 };
    {
        const char* c = tokens[ t ].C_String();
        int part = 0;
        UInt32 digits = 0;
        for ( ; '\0' != *c; ++c )
        {
            if ( *c >= '0' && *c <= '9' && digits < 3 )
            {
                clock[ part ] = clock[ part ] * 10 + ( *c - '0' );
                ++digits;
            }
            else
            if ( ':' == *c && part < 2 && digits > 0 )
            {
                ++part;
                digits = 0;
            }
            else
                return false;
        }
        if ( part < 1 || 0 == digits )
            return false;
    }
    ++t;

    int zoneMinutes = 0;
    if ( t < tokens.size() )
    {
        const CORE::CString& zone = tokens[ t ];
        const char* z = zone.C_String();
        if ( ( '+' == z[ 0 ] || '-' == z[ 0 ] ) && ByteLength( zone ) == 5 )
        {
            int hh = ( z[ 1 ] - '0' ) * 10 + ( z[ 2 ] - '0' );
            int mm = ( z[ 3 ] - '0' ) * 10 + ( z[ 4 ] - '0' );
            if ( hh < 0 || hh > 99 || mm < 0 || mm > 59 )
                return false;
            zoneMinutes = ( hh * 60 + mm ) * ( '-' == z[ 0 ] ? -1 : 1 );
        }
        else
        {
            CORE::CString up = zone.Uppercase();
            if ( up == "EST" ) zoneMinutes = -5 * 60;
            else if ( up == "EDT" ) zoneMinutes = -4 * 60;
            else if ( up == "CST" ) zoneMinutes = -6 * 60;
            else if ( up == "CDT" ) zoneMinutes = -5 * 60;
            else if ( up == "MST" ) zoneMinutes = -7 * 60;
            else if ( up == "MDT" ) zoneMinutes = -6 * 60;
            else if ( up == "PST" ) zoneMinutes = -8 * 60;
            else if ( up == "PDT" ) zoneMinutes = -7 * 60;
            // GMT, UT, Z and the military zones count as UTC
        }
    }

    if ( day < 1 || day > 31 || clock[ 0 ] > 23 || clock[ 1 ] > 59 || clock[ 2 ] > 60 )
        return false;
    if ( clock[ 2 ] > 59 )
        clock[ 2 ] = 59;
    date = CORE::CDateTime( (Int16) year, (UInt8) month, (UInt8) day, (Int8) clock[ 0 ], (Int8) clock[ 1 ], (Int8) clock[ 2 ], 0, (Int16) zoneMinutes );
    return true;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
RandomHex( UInt32 byteCount )
{GUCEF_TRACE;

    UInt8 bytes[ 32 ];
    if ( byteCount > sizeof( bytes ) )
        byteCount = sizeof( bytes );
    if ( !CORE::CRandom::GetRandomBytes( bytes, byteCount ) )
    {
        // the system has no random source: a time based value is not secret but it keeps the ids unique enough
        UInt64 ticks = MT::PrecisionTickCount();
        for ( UInt32 i=0; i<byteCount; ++i )
            bytes[ i ] = (UInt8)( ( ticks >> ( ( i % 8 ) * 8 ) ) ^ ( i * 31 ) );
    }
    return CORE::Base16Encode( bytes, byteCount ).Lowercase();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMimeCodec::GenerateMessageId( const CORE::CString& domain )
{GUCEF_TRACE;

    bool usable = ByteLength( domain ) > 0 && ByteLength( domain ) < 200;
    const char* d = domain.C_String();
    for ( UInt32 i=0; usable && i<ByteLength( domain ); ++i )
    {
        const char c = d[ i ];
        usable = ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || '.' == c || '-' == c;
    }
    return "<" + RandomHex( 16 ) + "@" + ( usable ? domain : CORE::CString( "localhost.localdomain" ) ) + ">";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMimeCodec::GenerateBoundary( void )
{GUCEF_TRACE;

    return "gucefmime-" + RandomHex( 18 );
}

/*-------------------------------------------------------------------------*/

bool
CMimeCodec::IsValidMessageId( const CORE::CString& messageId )
{GUCEF_TRACE;

    const UInt32 size = ByteLength( messageId );
    const char* p = messageId.C_String();
    if ( size < 5 || size > 255 || '<' != p[ 0 ] || '>' != p[ size - 1 ] )
        return false;
    UInt32 atCount = 0;
    for ( UInt32 i=1; i+1<size; ++i )
    {
        const UInt8 c = (UInt8) p[ i ];
        if ( c <= 0x20 || c >= 0x7F || '<' == c || '>' == c || '"' == c || '\\' == c || '(' == c || ')' == c )
            return false;
        if ( '@' == c )
            ++atCount;
    }
    return 1 == atCount;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
