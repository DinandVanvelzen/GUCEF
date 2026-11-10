/*
 *  AncestryDotComBackupLib: logic of the Ancestry.com tree backup tool
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include <string>

#include "AncestryBackup_CUrlTemplate.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ANCESTRYBACKUP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
IsDigit( char c ) { return c >= '0' && c <= '9'; }

static bool
IsHexDigit( char c ) { return IsDigit( c ) || ( c >= 'a' && c <= 'f' ) || ( c >= 'A' && c <= 'F' ); }

static bool
IsLetter( char c ) { return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ); }

/*-------------------------------------------------------------------------*/

static std::string
ClassifyBase( const std::string& base )
{GUCEF_TRACE;

    const size_t len = base.size();
    if ( len == 0 )
        return base;

    size_t digits = 0, letters = 0, hexDigits = 0, separators = 0, other = 0;
    for ( size_t i=0; i<len; ++i )
    {
        const char c = base[ i ];
        if ( IsDigit( c ) ) ++digits;
        else if ( IsLetter( c ) ) ++letters;
        if ( IsHexDigit( c ) ) ++hexDigits;
        if ( c == '-' || c == '_' || c == ':' ) ++separators;
        else if ( !IsDigit( c ) && !IsLetter( c ) ) ++other;
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

    return base;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUrlTemplate::TemplateSegment( const CORE::CString& segment )
{GUCEF_TRACE;

    std::string seg( segment.STL_String().c_str() );
    std::string ext;
    std::string base = seg;

    // a file extension is short and alphanumeric
    const size_t dot = seg.rfind( '.' );
    if ( dot != std::string::npos && dot > 0 && seg.size() - dot - 1 >= 1 && seg.size() - dot - 1 <= 5 )
    {
        bool extOk = true;
        for ( size_t i=dot+1; i<seg.size(); ++i )
            if ( !IsDigit( seg[ i ] ) && !IsLetter( seg[ i ] ) ) extOk = false;
        if ( extOk )
        {
            base = seg.substr( 0, dot );
            ext = seg.substr( dot );
        }
    }

    return CORE::CString( ( ClassifyBase( base ) + ext ).c_str() );
}

/*-------------------------------------------------------------------------*/

bool
CUrlTemplate::LooksLikeId( const CORE::CString& segment )
{GUCEF_TRACE;

    const CORE::CString templated = TemplateSegment( segment );
    return templated.STL_String().c_str()[ 0 ] == '{';
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUrlTemplate::TemplatePath( const CORE::CString& path )
{GUCEF_TRACE;

    const std::string p( path.STL_String().c_str() );
    std::string result;
    size_t start = 0;
    while ( start <= p.size() )
    {
        size_t end = p.find( '/', start );
        if ( end == std::string::npos )
            end = p.size();
        result += TemplateSegment( CORE::CString( p.substr( start, end - start ).c_str() ) ).STL_String().c_str();
        if ( end < p.size() )
            result += '/';
        start = end + 1;
    }
    return CORE::CString( result.c_str() );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUrlTemplate::FindIdAfterSegment( const CORE::CString& path, const CORE::CString& segmentName )
{GUCEF_TRACE;

    const std::string p( path.STL_String().c_str() );
    const std::string wanted( segmentName.STL_String().c_str() );
    size_t start = 0;
    bool previousMatched = false;
    while ( start <= p.size() )
    {
        size_t end = p.find( '/', start );
        if ( end == std::string::npos )
            end = p.size();
        const std::string seg = p.substr( start, end - start );
        if ( previousMatched )
        {
            if ( LooksLikeId( CORE::CString( seg.c_str() ) ) )
                return CORE::CString( seg.c_str() );
            previousMatched = false;
        }
        if ( seg == wanted )
            previousMatched = true;
        start = end + 1;
    }
    return CORE::CString();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUrlTemplate::PercentDecode( const CORE::CString& text )
{GUCEF_TRACE;

    const std::string t( text.STL_String().c_str() );
    std::string result;
    for ( size_t i=0; i<t.size(); ++i )
    {
        if ( t[ i ] == '%' && i + 2 < t.size() && IsHexDigit( t[ i + 1 ] ) && IsHexDigit( t[ i + 2 ] ) )
        {
            const std::string hex = t.substr( i + 1, 2 );
            result += (char) strtol( hex.c_str(), GUCEF_NULL, 16 );
            i += 2;
        }
        else
        if ( t[ i ] == '+' )
            result += ' ';
        else
            result += t[ i ];
    }
    return CORE::CString( result.c_str() );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUrlTemplate::EscapeJson( const CORE::CString& text )
{GUCEF_TRACE;

    const std::string source = text.STL_String();
    const unsigned char* t = (const unsigned char*) source.c_str();
    std::string result;
    for ( size_t i=0; t[ i ] != 0; ++i )
    {
        switch ( t[ i ] )
        {
            case '"':  result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default:
            {
                if ( t[ i ] < 0x20 )
                {
                    char buf[ 8 ];
                    snprintf( buf, sizeof( buf ), "\\u%04x", (unsigned int) t[ i ] );
                    result += buf;
                }
                else
                    result += (char) t[ i ];     // UTF-8 passes through unchanged
            }
        }
    }
    return CORE::CString( result.c_str() );
}

/*-------------------------------------------------------------------------*/

bool
CUrlTemplate::Parse( const CORE::CString& url, SUrlTemplate& result )
{GUCEF_TRACE;

    result = SUrlTemplate();
    const std::string u( url.STL_String().c_str() );

    const size_t schemeEnd = u.find( "://" );
    if ( schemeEnd == std::string::npos || schemeEnd == 0 )
        return false;

    std::string scheme = u.substr( 0, schemeEnd );
    for ( size_t i=0; i<scheme.size(); ++i )
        scheme[ i ] = (char) tolower( scheme[ i ] );

    size_t hostStart = schemeEnd + 3;
    size_t hostEnd = u.find_first_of( "/?#", hostStart );
    if ( hostEnd == std::string::npos )
        hostEnd = u.size();
    std::string host = u.substr( hostStart, hostEnd - hostStart );
    const size_t at = host.rfind( '@' );
    if ( at != std::string::npos )
        host = host.substr( at + 1 );
    for ( size_t i=0; i<host.size(); ++i )
        host[ i ] = (char) tolower( host[ i ] );

    size_t pathEnd = u.find_first_of( "?#", hostEnd );
    if ( pathEnd == std::string::npos )
        pathEnd = u.size();
    const std::string path = u.substr( hostEnd, pathEnd - hostEnd );

    result.scheme = scheme.c_str();
    result.host = host.c_str();
    result.path = path.c_str();
    result.pathTemplate = TemplatePath( result.path );

    if ( pathEnd < u.size() && u[ pathEnd ] == '?' )
    {
        size_t queryEnd = u.find( '#', pathEnd );
        if ( queryEnd == std::string::npos )
            queryEnd = u.size();
        const std::string query = u.substr( pathEnd + 1, queryEnd - pathEnd - 1 );
        size_t start = 0;
        while ( start <= query.size() && !query.empty() )
        {
            size_t end = query.find( '&', start );
            if ( end == std::string::npos )
                end = query.size();
            const std::string pair = query.substr( start, end - start );
            if ( !pair.empty() )
            {
                const size_t eq = pair.find( '=' );
                if ( eq == std::string::npos )
                    result.queryParams.push_back( std::make_pair( CORE::CString( pair.c_str() ), CORE::CString() ) );
                else
                    result.queryParams.push_back( std::make_pair( CORE::CString( pair.substr( 0, eq ).c_str() ), CORE::CString( pair.substr( eq + 1 ).c_str() ) ) );
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

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
