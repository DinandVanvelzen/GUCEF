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

#ifndef GUCEF_WEB_CURLPATHTEMPLATE_H
#include "gucefWEB_CUrlPathTemplate.h"
#define GUCEF_WEB_CURLPATHTEMPLATE_H
#endif /* GUCEF_WEB_CURLPATHTEMPLATE_H ? */

#include "gucefWEB_CWriteRequestRule.h"

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

static bool
IsAlnum( char c )
{
    return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' );
}

/*-------------------------------------------------------------------------*/

/** unreserved and the sub-delimiters of RFC 3986 without ";", plus ":" and "@" */
static bool
IsStrictPathChar( char c )
{
    if ( IsAlnum( c ) )
        return true;
    switch ( c )
    {
        case '-': case '.': case '_': case '~':
        case '!': case '$': case '&': case '\'': case '(': case ')': case '*': case '+': case ',': case '=':
        case ':': case '@':
            return true;
        default:
            return false;
    }
}

/*-------------------------------------------------------------------------*/

static bool
IsHostChar( char c )
{
    return IsAlnum( c ) || c == '-' || c == '.' || c == ':' || c == '[' || c == ']';
}

/*-------------------------------------------------------------------------*/

static bool
IsQueryChar( char c )
{
    const unsigned char u = (unsigned char) c;
    return u > 0x20 && u < 0x7F && c != '#';
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWriteRequestRule::CWriteRequestRule( void )
    : method()
    , scheme( "https" )
    , host()
    , path()
    , segments()
    , queryMode( QUERY_NONE )
    , query()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CWriteRequestRule::IsPlaceholder( const CORE::CString& segment )
{GUCEF_TRACE;

    return segment == "{n}" || segment == "{guid}" || segment == "{hex}" || segment == "{id}" || segment == "{any}";
}

/*-------------------------------------------------------------------------*/

bool
CWriteRequestRule::SplitStrictPath( const CORE::CString& path, CORE::CString::StringVector& segments, bool allowPlaceholders )
{GUCEF_TRACE;

    segments.clear();
    const char* p = path.C_String();
    const UInt32 len = ByteLength( path );
    if ( 0 == len || p[ 0 ] != '/' )
        return false;

    UInt32 start = 1;
    while ( true )
    {
        UInt32 end = start;
        while ( end < len && p[ end ] != '/' )
            ++end;
        const CORE::CString segment( p + start, end - start );
        const bool last = end >= len;
        if ( end == start )
        {
            // an empty segment: only as the last one, the trailing slash (or the root path)
            if ( !last )
                return false;
        }
        else
        if ( allowPlaceholders && p[ start ] == '{' )
        {
            if ( !IsPlaceholder( segment ) )
                return false;
        }
        else
        {
            for ( UInt32 i=start; i<end; ++i )
                if ( !IsStrictPathChar( p[ i ] ) )
                    return false;
            if ( segment == "." || segment == ".." )
                return false;
        }
        segments.push_back( segment );
        if ( last )
            break;
        start = end + 1;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWriteRequestRule::SegmentMatches( const CORE::CString& ruleSegment, const CORE::CString& requestSegment )
{GUCEF_TRACE;

    const UInt32 len = ByteLength( requestSegment );
    const char* s = requestSegment.C_String();
    if ( ruleSegment == "{n}" )
    {
        if ( 0 == len || len > 20 )
            return false;
        for ( UInt32 i=0; i<len; ++i )
            if ( s[ i ] < '0' || s[ i ] > '9' )
                return false;
        return true;
    }
    if ( ruleSegment == "{any}" )
    {
        if ( 0 == len )
            return false;
        for ( UInt32 i=0; i<len; ++i )
            if ( !IsAlnum( s[ i ] ) && s[ i ] != '-' && s[ i ] != '.' && s[ i ] != '_' && s[ i ] != '~' )
                return false;
        return requestSegment != "." && requestSegment != "..";
    }
    if ( ruleSegment == "{guid}" || ruleSegment == "{hex}" || ruleSegment == "{id}" )
        return 0 != len && CUrlPathTemplate::TemplateSegment( requestSegment ) == ruleSegment;
    return ruleSegment == requestSegment;
}

/*-------------------------------------------------------------------------*/

bool
CWriteRequestRule::Parse( const CORE::CString& text, CWriteRequestRule& rule, CORE::CString* problem )
{GUCEF_TRACE;

    CORE::CString why;
    CWriteRequestRule parsed;
    const CORE::CString trimmed = text.Trim( true ).Trim( false );
    const char* t = trimmed.C_String();
    const UInt32 len = ByteLength( trimmed );

    // METHOD
    UInt32 i = 0;
    while ( i < len && ( ( t[ i ] >= 'a' && t[ i ] <= 'z' ) || ( t[ i ] >= 'A' && t[ i ] <= 'Z' ) ) )
        ++i;
    if ( 0 == i || i >= len || ( t[ i ] != ' ' && t[ i ] != '\t' ) )
        why = "a rule starts with a method of letters followed by a space";
    else
    {
        parsed.method = CORE::CString( t, i );
        while ( i < len && ( t[ i ] == ' ' || t[ i ] == '\t' ) )
            ++i;

        // [scheme://]
        CORE::CString rest( t + i, len - i );
        const CORE::CString lowerRest = rest.Lowercase();
        if ( lowerRest.StartsWith( "https://" ) )
            rest = rest.CutChars( 8, true );
        else
        if ( lowerRest.StartsWith( "http://" ) )
        {
            parsed.scheme = "http";
            rest = rest.CutChars( 7, true );
        }
        else
        if ( lowerRest.HasSubstr( "://", true ) >= 0 )
            why = "only http and https are allowed";

        if ( why.IsNULLOrEmpty() )
        {
            const char* r = rest.C_String();
            const UInt32 rlen = ByteLength( rest );
            UInt32 hostEnd = 0;
            while ( hostEnd < rlen && r[ hostEnd ] != '/' )
                ++hostEnd;
            parsed.host = CORE::CString( r, hostEnd ).Lowercase();
            for ( UInt32 h=0; h<hostEnd && why.IsNULLOrEmpty(); ++h )
                if ( !IsHostChar( r[ h ] ) )
                    why = "the host holds a character that is not allowed in a host (no patterns, no user info)";
            if ( parsed.host.IsNULLOrEmpty() )
                why = "a rule names a host";
            else
            if ( hostEnd >= rlen )
                why = "a rule names a path, at least \"/\"";
            if ( why.IsNULLOrEmpty() )
            {
                UInt32 pathEnd = hostEnd;
                while ( pathEnd < rlen && r[ pathEnd ] != '?' )
                    ++pathEnd;
                parsed.path = CORE::CString( r + hostEnd, pathEnd - hostEnd );
                if ( !SplitStrictPath( parsed.path, parsed.segments, true ) )
                    why = "the path is not a plain path: no percent escapes, backslashes, \";\", dot or empty segments, placeholders only as {n}, {guid}, {hex}, {id} or {any}";
                else
                if ( pathEnd < rlen )
                {
                    const CORE::CString q( r + pathEnd + 1, rlen - pathEnd - 1 );
                    if ( q == "*" )
                        parsed.queryMode = QUERY_ANY;
                    else
                    if ( q.IsNULLOrEmpty() )
                        why = "an empty query: leave the \"?\" out for none, or write \"?*\" for any";
                    else
                    {
                        const char* qt = q.C_String();
                        const UInt32 qlen = ByteLength( q );
                        for ( UInt32 c=0; c<qlen && why.IsNULLOrEmpty(); ++c )
                            if ( !IsQueryChar( qt[ c ] ) )
                                why = "the query holds a space, a control character, a non-ASCII character or a fragment";
                        parsed.queryMode = QUERY_EXACT;
                        parsed.query = q;
                    }
                }
            }
        }
    }
    if ( !why.IsNULLOrEmpty() )
    {
        if ( GUCEF_NULL != problem )
            *problem = why;
        return false;
    }
    rule = parsed;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWriteRequestRule::ToString( void ) const
{GUCEF_TRACE;

    CORE::CString text = method + " ";
    if ( scheme == "http" )
        text += "http://";
    text += host + path;
    if ( queryMode == QUERY_ANY )
        text += "?*";
    else
    if ( queryMode == QUERY_EXACT )
        text += "?" + query;
    return text;
}

/*-------------------------------------------------------------------------*/

bool
CWriteRequestRule::MatchesMethod( const CORE::CString& requestMethod ) const
{GUCEF_TRACE;

    return method == requestMethod;
}

/*-------------------------------------------------------------------------*/

bool
CWriteRequestRule::MatchesHost( const CORE::CString& requestHost ) const
{GUCEF_TRACE;

    return host == requestHost;
}

/*-------------------------------------------------------------------------*/

bool
CWriteRequestRule::MatchesScheme( const CORE::CString& requestScheme ) const
{GUCEF_TRACE;

    return scheme == requestScheme;
}

/*-------------------------------------------------------------------------*/

bool
CWriteRequestRule::MatchesPath( const CORE::CString::StringVector& requestSegments ) const
{GUCEF_TRACE;

    if ( requestSegments.size() != segments.size() )
        return false;
    for ( size_t i=0; i<segments.size(); ++i )
        if ( !SegmentMatches( segments[ i ], requestSegments[ i ] ) )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWriteRequestRule::MatchesQuery( bool hasQuery, const CORE::CString& requestQuery ) const
{GUCEF_TRACE;

    switch ( queryMode )
    {
        case QUERY_ANY:     return true;
        case QUERY_EXACT:   return hasQuery && requestQuery == query;
        default:            return !hasQuery;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
