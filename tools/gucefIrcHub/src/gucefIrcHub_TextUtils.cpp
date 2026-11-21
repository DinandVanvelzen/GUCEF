/*
 *  gucefIrcHub: IRC hub with LLM bots for a family IRC server
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

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#include "gucefIrcHub_TextUtils.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFIrcHub {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static std::string
ToLowerAscii( const std::string& text )
{
    std::string result( text );
    for ( size_t i=0; i<result.size(); ++i )
    {
        if ( result[ i ] >= 'A' && result[ i ] <= 'Z' )
            result[ i ] = (char) ( result[ i ] - 'A' + 'a' );
    }
    return result;
}

/*-------------------------------------------------------------------------*/

static bool
IsNickChar( char c )
{
    return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) ||
           '_' == c || '-' == c || '[' == c || ']' == c || '\\' == c || '`' == c || '^' == c || '{' == c || '}' == c || '|' == c;
}

/*-------------------------------------------------------------------------*/

std::string
TrimWhitespace( const std::string& text )
{GUCEF_TRACE;

    const char* whitespace = " \t\r\n";
    const size_t first = text.find_first_not_of( whitespace );
    if ( std::string::npos == first )
        return std::string();
    const size_t last = text.find_last_not_of( whitespace );
    return text.substr( first, last - first + 1 );
}

/*-------------------------------------------------------------------------*/

bool
IsAddressedTo( const CORE::CString& text ,
               const CORE::CString& nick ,
               CORE::CString& rest       )
{GUCEF_TRACE;

    const std::string line = TrimWhitespace( text.STL_String() );
    const std::string lowerLine = ToLowerAscii( line );
    const std::string lowerNick = ToLowerAscii( nick.STL_String() );
    if ( lowerNick.empty() )
        return false;

    // "nick: ...", "nick, ..." and "@nick ..."; a sentence that only starts with the name ("claude is great") is not
    size_t start = 0;
    if ( !lowerLine.empty() && '@' == lowerLine[ 0 ] )
        start = 1;
    if ( 0 == lowerLine.compare( start, lowerNick.size(), lowerNick ) )
    {
        const size_t end = start + lowerNick.size();
        if ( end == lowerLine.size() )
        {
            rest = CORE::CString::Empty;
            return true;
        }
        const char next = lowerLine[ end ];
        if ( ':' == next || ',' == next || ( 1 == start && ( ' ' == next || '\t' == next ) ) )
        {
            rest = TrimWhitespace( line.substr( end + 1 ) );
            return true;
        }
    }

    // "@nick" further on, as a whole word
    const std::string mention = "@" + lowerNick;
    size_t pos = lowerLine.find( mention );
    while ( std::string::npos != pos )
    {
        const size_t end = pos + mention.size();
        const bool wordStart = 0 == pos || !IsNickChar( lowerLine[ pos - 1 ] );
        const bool wordEnd = end >= lowerLine.size() || !IsNickChar( lowerLine[ end ] );
        if ( wordStart && wordEnd )
        {
            rest = line;
            return true;
        }
        pos = lowerLine.find( mention, pos + 1 );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
StartsWithAddressOf( const CORE::CString& text                ,
                     const CORE::CString::StringVector& nicks ,
                     CORE::CString& addressee                 )
{GUCEF_TRACE;

    const std::string lowerLine = ToLowerAscii( TrimWhitespace( text.STL_String() ) );
    for ( size_t i=0; i<nicks.size(); ++i )
    {
        const std::string lowerNick = ToLowerAscii( nicks[ i ].STL_String() );
        if ( lowerNick.empty() || lowerLine.size() <= lowerNick.size() )
            continue;
        if ( 0 == lowerLine.compare( 0, lowerNick.size(), lowerNick ) )
        {
            const char next = lowerLine[ lowerNick.size() ];
            if ( ':' == next || ',' == next )
            {
                addressee = nicks[ i ];
                return true;
            }
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/** The length of the UTF-8 character that starts with byte c; 1 for a byte that does not start one */
static size_t
Utf8CharLength( unsigned char c )
{
    if ( c >= 0xF0 && c <= 0xF7 )
        return 4;
    if ( c >= 0xE0 )
        return 3;
    if ( c >= 0xC0 )
        return 2;
    return 1;
}

/*-------------------------------------------------------------------------*/

static void
WrapLine( const std::string& line, size_t maxBytes, std::vector< std::string >& lines )
{
    std::string remaining = TrimWhitespace( line );
    while ( !remaining.empty() )
    {
        if ( remaining.size() <= maxBytes )
        {
            lines.push_back( remaining );
            return;
        }

        // the longest prefix of whole UTF-8 characters that fits
        size_t fit = 0;
        while ( fit < remaining.size() )
        {
            const size_t charLength = Utf8CharLength( (unsigned char) remaining[ fit ] );
            if ( fit + charLength > maxBytes )
                break;
            fit += charLength;
        }
        if ( 0 == fit )
            fit = Utf8CharLength( (unsigned char) remaining[ 0 ] );

        // at the last space within it, when there is one in its second half
        size_t cut = fit;
        const size_t space = remaining.rfind( ' ', fit );
        if ( std::string::npos != space && space > fit / 2 )
            cut = space;

        lines.push_back( TrimWhitespace( remaining.substr( 0, cut ) ) );
        remaining = TrimWhitespace( remaining.substr( cut ) );
    }
}

/*-------------------------------------------------------------------------*/

void
SplitForIrc( const std::string& text           ,
             size_t maxBytes                   ,
             std::vector< std::string >& lines )
{GUCEF_TRACE;

    if ( maxBytes < 8 )
        maxBytes = 8;
    std::string current;
    for ( size_t i=0; i<=text.size(); ++i )
    {
        const char c = i < text.size() ? text[ i ] : '\n';
        if ( '\n' == c )
        {
            WrapLine( current, maxBytes, lines );
            current.clear();
        }
        else
        if ( '\t' == c )
        {
            current += ' ';
        }
        else
        if ( (unsigned char) c >= 0x20 && 0x7F != c )
        {
            current += c;
        }
        // other control characters, \r included, are left out
    }
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFIrcHub */

/*-------------------------------------------------------------------------*/
