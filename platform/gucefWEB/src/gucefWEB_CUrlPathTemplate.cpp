/*
 *  gucefWEB: GUCEF module providing Web application functionality
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

#include <stdio.h>
#include <string.h>

#include "gucefWEB_CUrlPathTemplate.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static UInt32
ByteLength( const CORE::CString& text )
{
    return text.IsNULLOrEmpty() ? 0 : text.ByteSize() - 1;
}

/*-------------------------------------------------------------------------*/

static bool IsDigit( char c ) { return c >= '0' && c <= '9'; }
static bool IsHexDigit( char c ) { return IsDigit( c ) || ( c >= 'a' && c <= 'f' ) || ( c >= 'A' && c <= 'F' ); }
static bool IsLetter( char c ) { return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ); }
static char ToLowerAscii( char c ) { return ( c >= 'A' && c <= 'Z' ) ? (char) ( c - 'A' + 'a' ) : c; }

/*-------------------------------------------------------------------------*/

static int
HexValue( char c )
{
    if ( IsDigit( c ) )
        return c - '0';
    if ( c >= 'a' && c <= 'f' )
        return c - 'a' + 10;
    return c - 'A' + 10;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
LowerBytes( const char* text, UInt32 length )
{
    if ( 0 == length )
        return CORE::CString();
    GUCEF::vector< char > buffer( length );
    for ( UInt32 i=0; i<length; ++i )
        buffer[ i ] = ToLowerAscii( text[ i ] );
    return CORE::CString( &buffer[ 0 ], length );
}

/*-------------------------------------------------------------------------*/

/** The placeholder of a segment without its extension, or GUCEF_NULL when it is not an identifier */
static const char*
ClassifyBase( const char* base, UInt32 len )
{
    if ( 0 == len )
        return GUCEF_NULL;

    UInt32 digits = 0, letters = 0, hexDigits = 0, separators = 0, other = 0;
    for ( UInt32 i=0; i<len; ++i )
    {
        const char c = base[ i ];
        if ( IsDigit( c ) )
            ++digits;
        else
        if ( IsLetter( c ) )
            ++letters;
        if ( IsHexDigit( c ) )
            ++hexDigits;
        if ( c == '-' || c == '_' || c == ':' )
            ++separators;
        else
        if ( !IsDigit( c ) && !IsLetter( c ) )
            ++other;
    }

    // 8-4-4-4-12 GUID, with or without the dashes
    if ( len == 36 && base[ 8 ] == '-' && base[ 13 ] == '-' && base[ 18 ] == '-' && base[ 23 ] == '-' && hexDigits + 4 == len )
        return "{guid}";
    if ( len == 32 && hexDigits == len )
        return "{guid}";
    // numbers, also when composed of several numbers: 123-456 or 123_456
    if ( digits >= 3 && digits + separators == len && other == 0 )
        return "{n}";
    // a hash, or another long run of hex digits that is not a plain word
    if ( len >= 16 && hexDigits == len && digits > 0 )
        return "{hex}";
    // generated ids mixing letters and digits
    if ( len >= 12 && digits > 0 && letters > 0 && other == 0 )
        return "{id}";
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

static void
AppendTemplatedSegment( CORE::CString& out, const char* seg, UInt32 len )
{
    // a file extension is short and alphanumeric
    UInt32 baseLen = len;
    for ( UInt32 i=len; i>0; --i )
    {
        if ( seg[ i-1 ] == '.' )
        {
            const UInt32 dot = i-1;
            const UInt32 extLen = len - dot - 1;
            bool extOk = dot > 0 && extLen >= 1 && extLen <= 5;
            for ( UInt32 e=dot+1; extOk && e<len; ++e )
                if ( !IsDigit( seg[ e ] ) && !IsLetter( seg[ e ] ) )
                    extOk = false;
            if ( extOk )
                baseLen = dot;
            break;
        }
    }

    const char* placeholder = ClassifyBase( seg, baseLen );
    if ( GUCEF_NULL != placeholder )
    {
        out += placeholder;
        if ( baseLen < len )
            out.Append( seg + baseLen, len - baseLen );
    }
    else
    if ( len > 0 )
    {
        out.Append( seg, len );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUrlPathTemplate::CUrlPathTemplate( void )
    : scheme()
    , host()
    , path()
    , pathTemplate()
    , queryParams()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CUrlPathTemplate::GetKey( void ) const
{GUCEF_TRACE;

    return host + pathTemplate;
}

/*-------------------------------------------------------------------------*/

void
CUrlPathTemplate::Clear( void )
{GUCEF_TRACE;

    scheme.Clear();
    host.Clear();
    path.Clear();
    pathTemplate.Clear();
    queryParams.clear();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUrlPathTemplate::TemplateSegment( const CORE::CString& segment )
{GUCEF_TRACE;

    CORE::CString result;
    AppendTemplatedSegment( result, segment.C_String(), ByteLength( segment ) );
    return result;
}

/*-------------------------------------------------------------------------*/

bool
CUrlPathTemplate::LooksLikeId( const CORE::CString& segment )
{GUCEF_TRACE;

    const CORE::CString templated = TemplateSegment( segment );
    return !templated.IsNULLOrEmpty() && templated.C_String()[ 0 ] == '{' && !( !segment.IsNULLOrEmpty() && segment.C_String()[ 0 ] == '{' );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUrlPathTemplate::TemplatePath( const CORE::CString& path )
{GUCEF_TRACE;

    CORE::CString result;
    const char* p = path.C_String();
    const UInt32 len = ByteLength( path );
    UInt32 start = 0;
    while ( start < len || ( start == len && len > 0 && p[ len-1 ] == '/' ) )
    {
        UInt32 end = start;
        while ( end < len && p[ end ] != '/' )
            ++end;
        AppendTemplatedSegment( result, p + start, end - start );
        if ( end < len )
            result += '/';
        start = end + 1;
        if ( end >= len )
            break;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUrlPathTemplate::FindIdAfterSegment( const CORE::CString& path, const CORE::CString& segmentName )
{GUCEF_TRACE;

    const char* p = path.C_String();
    const UInt32 len = ByteLength( path );
    const UInt32 wantedLen = ByteLength( segmentName );
    bool previousMatched = false;
    UInt32 start = 0;
    while ( start <= len && len > 0 )
    {
        UInt32 end = start;
        while ( end < len && p[ end ] != '/' )
            ++end;
        const CORE::CString seg( p + start, end - start );
        if ( previousMatched )
        {
            if ( LooksLikeId( seg ) )
                return seg;
            previousMatched = false;
        }
        if ( wantedLen > 0 && end - start == wantedLen && 0 == memcmp( p + start, segmentName.C_String(), wantedLen ) )
            previousMatched = true;
        start = end + 1;
    }
    return CORE::CString();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUrlPathTemplate::PercentDecode( const CORE::CString& text )
{GUCEF_TRACE;

    const char* t = text.C_String();
    const UInt32 len = ByteLength( text );
    if ( 0 == len )
        return CORE::CString();
    GUCEF::vector< char > buffer( len );
    UInt32 out = 0;
    for ( UInt32 i=0; i<len; ++i )
    {
        if ( t[ i ] == '%' && i + 2 < len && IsHexDigit( t[ i + 1 ] ) && IsHexDigit( t[ i + 2 ] ) )
        {
            buffer[ out++ ] = (char) ( HexValue( t[ i + 1 ] ) * 16 + HexValue( t[ i + 2 ] ) );
            i += 2;
        }
        else
        if ( t[ i ] == '+' )
            buffer[ out++ ] = ' ';
        else
            buffer[ out++ ] = t[ i ];
    }
    return CORE::CString( &buffer[ 0 ], out );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUrlPathTemplate::EscapeJson( const CORE::CString& text )
{GUCEF_TRACE;

    const unsigned char* t = (const unsigned char*) text.C_String();
    const UInt32 len = ByteLength( text );
    CORE::CString result;
    UInt32 runStart = 0;
    for ( UInt32 i=0; i<len; ++i )
    {
        const unsigned char c = t[ i ];
        const char* escape = GUCEF_NULL;
        char buffer[ 8 ];
        switch ( c )
        {
            case '"':  escape = "\\\""; break;
            case '\\': escape = "\\\\"; break;
            case '\n': escape = "\\n"; break;
            case '\r': escape = "\\r"; break;
            case '\t': escape = "\\t"; break;
            default:
            {
                if ( c < 0x20 )
                {
                    snprintf( buffer, sizeof( buffer ), "\\u%04x", (unsigned int) c );
                    escape = buffer;
                }
            }
        }
        if ( GUCEF_NULL != escape )
        {
            if ( i > runStart )
                result.Append( (const char*) t + runStart, i - runStart );
            result += escape;
            runStart = i + 1;
        }
    }
    if ( len > runStart )
        result.Append( (const char*) t + runStart, len - runStart );     // UTF-8 passes through unchanged
    return result;
}

/*-------------------------------------------------------------------------*/

bool
CUrlPathTemplate::Parse( const CORE::CString& url, CUrlPathTemplate& result )
{GUCEF_TRACE;

    result.Clear();
    const char* u = url.C_String();
    const UInt32 len = ByteLength( url );

    UInt32 schemeEnd = 0;
    while ( schemeEnd + 2 < len && !( u[ schemeEnd ] == ':' && u[ schemeEnd+1 ] == '/' && u[ schemeEnd+2 ] == '/' ) )
    {
        if ( u[ schemeEnd ] == '/' || u[ schemeEnd ] == '?' || u[ schemeEnd ] == '#' )
            return false;
        ++schemeEnd;
    }
    if ( schemeEnd == 0 || schemeEnd + 2 >= len )
        return false;

    const UInt32 hostStart = schemeEnd + 3;
    UInt32 hostEnd = hostStart;
    while ( hostEnd < len && u[ hostEnd ] != '/' && u[ hostEnd ] != '?' && u[ hostEnd ] != '#' )
        ++hostEnd;
    UInt32 realHostStart = hostStart;
    for ( UInt32 i=hostStart; i<hostEnd; ++i )
        if ( u[ i ] == '@' )
            realHostStart = i + 1;

    UInt32 pathEnd = hostEnd;
    while ( pathEnd < len && u[ pathEnd ] != '?' && u[ pathEnd ] != '#' )
        ++pathEnd;

    result.scheme = LowerBytes( u, schemeEnd );
    result.host = LowerBytes( u + realHostStart, hostEnd - realHostStart );
    result.path = CORE::CString( u + hostEnd, pathEnd - hostEnd );
    result.pathTemplate = TemplatePath( result.path );

    if ( pathEnd < len && u[ pathEnd ] == '?' )
    {
        UInt32 queryEnd = pathEnd + 1;
        while ( queryEnd < len && u[ queryEnd ] != '#' )
            ++queryEnd;
        UInt32 start = pathEnd + 1;
        while ( start < queryEnd )
        {
            UInt32 end = start;
            while ( end < queryEnd && u[ end ] != '&' )
                ++end;
            if ( end > start )
            {
                UInt32 eq = start;
                while ( eq < end && u[ eq ] != '=' )
                    ++eq;
                if ( eq == end )
                    result.queryParams.push_back( TQueryParam( CORE::CString( u + start, end - start ), CORE::CString() ) );
                else
                    result.queryParams.push_back( TQueryParam( CORE::CString( u + start, eq - start ), CORE::CString( u + eq + 1, end - eq - 1 ) ) );
            }
            start = end + 1;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
