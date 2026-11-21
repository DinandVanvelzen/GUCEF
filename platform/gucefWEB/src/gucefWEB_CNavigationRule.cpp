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

#include <string.h>

#ifndef GUCEF_CORE_CGLOBPATTERN_H
#include "gucefCORE_CGlobPattern.h"
#define GUCEF_CORE_CGLOBPATTERN_H
#endif /* GUCEF_CORE_CGLOBPATTERN_H ? */

#include "gucefWEB_CNavigationRule.h"

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

/** The size in bytes, without the terminator */
static UInt32
ByteLength( const CORE::CString& text )
{
    return text.IsNULLOrEmpty() ? 0 : text.ByteSize() - 1;
}

/*-------------------------------------------------------------------------*/

/** Does a path pattern hold a glob character: '*', '?' or '['. Braces are literal in paths ({n} of a template) */
static bool
IsPathGlob( const CORE::CString& pattern )
{
    return pattern.HasChar( '*' ) >= 0 || pattern.HasChar( '?' ) >= 0 || pattern.HasChar( '[' ) >= 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CNavigationRule::CNavigationRule( void )
    : hostPattern()
    , pathPattern()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CNavigationRule::CNavigationRule( const CORE::CString& host, const CORE::CString& path )
    : hostPattern( host )
    , pathPattern( path )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CNavigationRule::Parse( const CORE::CString& text, CNavigationRule& rule )
{GUCEF_TRACE;

    CORE::CString rest = text.Trim( true ).Trim( false );
    const CORE::CString lower = rest.Lowercase();
    if ( lower.StartsWith( "https://" ) )
        rest = rest.CutChars( 8, true );
    else
    if ( lower.StartsWith( "http://" ) )
        rest = rest.CutChars( 7, true );

    const Int32 slash = rest.HasChar( '/', true );
    CORE::CString host = slash >= 0 ? rest.SubstrToIndex( (UInt32) slash, true ) : rest;
    CORE::CString path = slash >= 0 ? rest.CutChars( (UInt32) slash, true ) : CORE::CString::Empty;
    if ( host.IsNULLOrEmpty() )
        return false;
    for ( UInt32 i=0; i<ByteLength( host ); ++i )
    {
        const char c = host.C_String()[ i ];
        const bool ok = ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) ||
                        c == '.' || c == '-' || c == '_' || c == ':' || c == '*' || c == '?' || c == '[' || c == ']' ||
                        c == '{' || c == '}' || c == ',';
        if ( !ok )
            return false;
    }
    for ( UInt32 i=0; i<ByteLength( path ); ++i )
    {
        const char c = path.C_String()[ i ];
        if ( (unsigned char) c <= 32 || c == '?' || c == '#' || c == '\\' )
            return false;
    }
    if ( path == "/" )
        path.Clear();
    rule.hostPattern = host.Lowercase();
    rule.pathPattern = path;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CNavigationRule::ToString( void ) const
{GUCEF_TRACE;

    return hostPattern + pathPattern;
}

/*-------------------------------------------------------------------------*/

bool
CNavigationRule::MatchesHost( const CORE::CString& hostName, const CORE::CString& hostAndPort ) const
{GUCEF_TRACE;

    if ( hostPattern.IsNULLOrEmpty() )
        return false;
    const bool withPort = hostPattern.HasChar( ':' ) >= 0;
    return CORE::CGlobPattern::IsMatch( hostPattern, withPort ? hostAndPort : hostName, false );
}

/*-------------------------------------------------------------------------*/

bool
CNavigationRule::MatchesPath( const CORE::CString& normalisedPath, bool caseSensitive ) const
{GUCEF_TRACE;

    if ( pathPattern.IsNULLOrEmpty() )
        return true;
    if ( IsPathGlob( pathPattern ) )
        return CORE::CGlobPattern::IsMatch( EscapeBraces( pathPattern ), normalisedPath, caseSensitive );

    const CORE::CString pattern = caseSensitive ? pathPattern : pathPattern.Lowercase();
    const CORE::CString path = caseSensitive ? normalisedPath : normalisedPath.Lowercase();
    const UInt32 patternLength = ByteLength( pattern );
    if ( ByteLength( path ) < patternLength || 0 != strncmp( path.C_String(), pattern.C_String(), patternLength ) )
        return false;
    // a prefix ends at a segment boundary
    if ( ByteLength( path ) == patternLength || pattern.C_String()[ patternLength - 1 ] == '/' )
        return true;
    return path.C_String()[ patternLength ] == '/';
}

/*-------------------------------------------------------------------------*/

bool
CNavigationRule::Matches( const CORE::CString& hostName, const CORE::CString& hostAndPort, const CORE::CString& normalisedPath, bool caseSensitivePath ) const
{GUCEF_TRACE;

    return MatchesHost( hostName, hostAndPort ) && MatchesPath( normalisedPath, caseSensitivePath );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CNavigationRule::EscapeBraces( const CORE::CString& pattern )
{GUCEF_TRACE;

    return pattern.ReplaceSubstr( "{", "\\{" ).ReplaceSubstr( "}", "\\}" );
}

/*-------------------------------------------------------------------------*/

static Int32
HexValue( char c )
{
    if ( c >= '0' && c <= '9' )
        return c - '0';
    if ( c >= 'a' && c <= 'f' )
        return c - 'a' + 10;
    if ( c >= 'A' && c <= 'F' )
        return c - 'A' + 10;
    return -1;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CNavigationRule::NormalisePath( const CORE::CString& path, bool& ambiguous )
{GUCEF_TRACE;

    static const char* const hexDigits = "0123456789ABCDEF";
    ambiguous = false;

    // escapes: unreserved characters decoded, the rest kept in one spelling
    CORE::CString decoded;
    const char* p = path.C_String();
    const UInt32 len = ByteLength( path );
    for ( UInt32 i=0; i<len; ++i )
    {
        const char c = p[ i ];
        if ( c == '\\' )
            ambiguous = true;
        if ( c == '%' && i + 2 < len && HexValue( p[ i+1 ] ) >= 0 && HexValue( p[ i+2 ] ) >= 0 )
        {
            const char value = (char) ( HexValue( p[ i+1 ] ) * 16 + HexValue( p[ i+2 ] ) );
            if ( value == '/' || value == '\\' )
                ambiguous = true;
            const bool unreserved = ( value >= 'a' && value <= 'z' ) || ( value >= 'A' && value <= 'Z' ) || ( value >= '0' && value <= '9' ) ||
                                    value == '-' || value == '.' || value == '_' || value == '~';
            if ( unreserved )
                decoded += value;
            else
            {
                decoded += '%';
                decoded += hexDigits[ ( (unsigned char) value ) >> 4 ];
                decoded += hexDigits[ ( (unsigned char) value ) & 15 ];
            }
            i += 2;
            continue;
        }
        decoded += c;
    }

    // dot segments, RFC 3986 section 5.2.4
    CORE::CString::StringVector segments = decoded.ParseElements( '/', true );
    CORE::CString::StringVector output;
    const UInt32 decodedLength = ByteLength( decoded );
    const bool trailingSlash = decodedLength > 0 && decoded.C_String()[ decodedLength - 1 ] == '/';
    bool endsInDirectory = trailingSlash;
    for ( size_t i=0; i<segments.size(); ++i )
    {
        const CORE::CString& segment = segments[ i ];
        if ( i == 0 && segment.IsNULLOrEmpty() )
            continue;       // before the leading slash
        if ( segment == "." )
        {
            endsInDirectory = true;
            continue;
        }
        if ( segment == ".." )
        {
            if ( !output.empty() )
                output.pop_back();
            endsInDirectory = true;
            continue;
        }
        if ( i + 1 == segments.size() && segment.IsNULLOrEmpty() )
            continue;       // after the trailing slash
        output.push_back( segment );
        endsInDirectory = trailingSlash;
    }
    CORE::CString result;
    for ( size_t i=0; i<output.size(); ++i )
        result += "/" + output[ i ];
    if ( result.IsNULLOrEmpty() || endsInDirectory )
        result += "/";
    return result;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
