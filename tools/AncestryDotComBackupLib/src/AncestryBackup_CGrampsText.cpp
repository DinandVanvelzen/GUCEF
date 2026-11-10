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

#include "AncestryBackup_CGrampsText.h"

#include <cstdlib>
#include <cctype>
#include <cstdio>

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

static void
AppendUtf8( std::string& out, UInt32 cp )
{
    if ( cp < 0x80 )
        out += (char) cp;
    else
    if ( cp < 0x800 )
    {
        out += (char) ( 0xC0 | ( cp >> 6 ) );
        out += (char) ( 0x80 | ( cp & 0x3F ) );
    }
    else
    if ( cp < 0x10000 )
    {
        out += (char) ( 0xE0 | ( cp >> 12 ) );
        out += (char) ( 0x80 | ( ( cp >> 6 ) & 0x3F ) );
        out += (char) ( 0x80 | ( cp & 0x3F ) );
    }
    else
    {
        out += (char) ( 0xF0 | ( cp >> 18 ) );
        out += (char) ( 0x80 | ( ( cp >> 12 ) & 0x3F ) );
        out += (char) ( 0x80 | ( ( cp >> 6 ) & 0x3F ) );
        out += (char) ( 0x80 | ( cp & 0x3F ) );
    }
}

/*-------------------------------------------------------------------------*/

/** Is XML 1.0 content: tab, line feed, carriage return and everything from the space up except the non-characters */
static bool
IsXmlCharacter( UInt32 cp )
{
    return cp == 0x9 || cp == 0xA || cp == 0xD || ( cp >= 0x20 && cp <= 0xD7FF ) || ( cp >= 0xE000 && cp <= 0xFFFD && cp != 0xFFFE ) || ( cp >= 0x10000 && cp <= 0x10FFFF );
}

/*-------------------------------------------------------------------------*/

std::string
CGrampsText::XmlEscape( const std::string& text, bool inAttribute )
{GUCEF_TRACE;

    std::string out;
    out.reserve( text.size() + text.size() / 8 );
    const size_t size = text.size();
    size_t i = 0;
    while ( i < size )
    {
        const unsigned char c = (unsigned char) text[ i ];
        if ( c < 0x80 )
        {
            switch ( c )
            {
                case '&': out += "&amp;"; break;
                case '<': out += "&lt;"; break;
                case '>': out += "&gt;"; break;
                case '"': out += "&quot;"; break;
                case '\'': out += "&apos;"; break;
                case '\t': out += inAttribute ? "&#9;" : "\t"; break;
                case '\n': out += inAttribute ? "&#10;" : "\n"; break;
                case '\r': out += inAttribute ? "&#13;" : "&#13;"; break;
                default:
                    if ( c >= 0x20 && c != 0x7F )
                        out += (char) c;
                    // other control characters are not allowed in XML and are dropped
                    break;
            }
            ++i;
            continue;
        }

        // a multi byte sequence which has to be valid, shortest form, and a character XML allows
        UInt32 len = 0;
        UInt32 cp = 0;
        if ( c >= 0xC2 && c <= 0xDF ) { len = 2; cp = c & 0x1F; }
        else
        if ( c >= 0xE0 && c <= 0xEF ) { len = 3; cp = c & 0x0F; }
        else
        if ( c >= 0xF0 && c <= 0xF4 ) { len = 4; cp = c & 0x07; }

        bool valid = len > 0 && i + len <= size;
        for ( UInt32 k=1; valid && k<len; ++k )
        {
            const unsigned char cc = (unsigned char) text[ i + k ];
            if ( ( cc & 0xC0 ) != 0x80 )
                valid = false;
            else
                cp = ( cp << 6 ) | ( cc & 0x3F );
        }
        if ( valid )
        {
            if ( ( len == 3 && cp < 0x800 ) || ( len == 4 && cp < 0x10000 ) || cp > 0x10FFFF || ( cp >= 0xD800 && cp <= 0xDFFF ) )
                valid = false;
        }
        if ( valid && IsXmlCharacter( cp ) )
        {
            out.append( text, i, len );
            i += len;
        }
        else
        if ( valid )
        {
            i += len;       // a character XML does not allow, such as U+FFFF
        }
        else
        {
            out += "\xEF\xBF\xBD";
            ++i;
        }
    }
    return out;
}

/*-------------------------------------------------------------------------*/

std::string
CGrampsText::DecodeHtmlEntities( const std::string& text )
{GUCEF_TRACE;

    if ( text.find( '&' ) == std::string::npos )
        return text;

    std::string out;
    out.reserve( text.size() );
    const size_t size = text.size();
    for ( size_t i=0; i<size; ++i )
    {
        if ( text[ i ] != '&' )
        {
            out += text[ i ];
            continue;
        }

        const size_t semi = text.find( ';', i + 1 );
        if ( semi == std::string::npos || semi - i > 10 )
        {
            out += '&';
            continue;
        }
        const std::string name = text.substr( i + 1, semi - i - 1 );
        UInt32 cp = 0;
        bool known = true;
        if ( name == "amp" ) cp = '&';
        else if ( name == "lt" ) cp = '<';
        else if ( name == "gt" ) cp = '>';
        else if ( name == "quot" ) cp = '"';
        else if ( name == "apos" ) cp = '\'';
        else if ( name == "nbsp" ) cp = ' ';
        else if ( name.size() > 1 && name[ 0 ] == '#' )
        {
            char* endp = GUCEF_NULL;
            const bool hex = name[ 1 ] == 'x' || name[ 1 ] == 'X';
            const unsigned long v = ::strtoul( name.c_str() + ( hex ? 2 : 1 ), &endp, hex ? 16 : 10 );
            if ( endp != GUCEF_NULL && *endp == '\0' && endp != name.c_str() + ( hex ? 2 : 1 ) && v > 0 && v <= 0x10FFFF )
                cp = (UInt32) v;
            else
                known = false;
        }
        else
            known = false;

        if ( !known )
        {
            out += '&';
            continue;
        }
        AppendUtf8( out, cp );
        i = semi;
    }
    return out;
}

/*-------------------------------------------------------------------------*/

std::string
CGrampsText::HtmlToPlainText( const std::string& text )
{GUCEF_TRACE;

    // an escaped tag is a tag as well: the text of the export has them as &lt;p&gt;
    const std::string decoded = DecodeHtmlEntities( text );
    if ( decoded.find( '<' ) == std::string::npos )
        return decoded;

    std::string out;
    out.reserve( decoded.size() );
    const size_t size = decoded.size();
    for ( size_t i=0; i<size; ++i )
    {
        if ( decoded[ i ] != '<' )
        {
            out += decoded[ i ];
            continue;
        }
        const size_t close = decoded.find( '>', i );
        // something like "a < b" is not a tag: a tag starts with a letter or a slash
        const bool looksLikeTag = close != std::string::npos && i + 1 < size && ( ::isalpha( (unsigned char) decoded[ i + 1 ] ) || decoded[ i + 1 ] == '/' || decoded[ i + 1 ] == '!' );
        if ( !looksLikeTag )
        {
            out += '<';
            continue;
        }
        std::string name = ToLowerAscii( decoded.substr( i + 1, close - i - 1 ) );
        bool lineBreak = false;
        size_t s = name[ 0 ] == '/' ? 1 : 0;
        size_t e = s;
        while ( e < name.size() && ::isalnum( (unsigned char) name[ e ] ) )
            ++e;
        name = name.substr( s, e - s );
        if ( name == "br" || name == "p" || name == "div" || name == "li" || name == "tr" || name == "h1" || name == "h2" || name == "h3" )
            lineBreak = true;
        if ( lineBreak && !out.empty() && out[ out.size() - 1 ] != '\n' )
            out += '\n';
        i = close;
    }

    // tidy up: no more than one empty line in a row and nothing trailing
    std::string tidy;
    tidy.reserve( out.size() );
    UInt32 newlines = 0;
    for ( size_t i=0; i<out.size(); ++i )
    {
        if ( out[ i ] == '\n' )
        {
            if ( ++newlines > 2 )
                continue;
        }
        else
            newlines = 0;
        tidy += out[ i ];
    }
    return Trim( tidy );
}

/*-------------------------------------------------------------------------*/

std::string
CGrampsText::Trim( const std::string& text )
{GUCEF_TRACE;

    size_t b = 0;
    size_t e = text.size();
    while ( b < e && ( text[ b ] == ' ' || text[ b ] == '\t' || text[ b ] == '\n' || text[ b ] == '\r' ) )
        ++b;
    while ( e > b && ( text[ e - 1 ] == ' ' || text[ e - 1 ] == '\t' || text[ e - 1 ] == '\n' || text[ e - 1 ] == '\r' ) )
        --e;
    return text.substr( b, e - b );
}

/*-------------------------------------------------------------------------*/

std::string
CGrampsText::ToLowerAscii( const std::string& text )
{GUCEF_TRACE;

    std::string out( text );
    for ( size_t i=0; i<out.size(); ++i )
        if ( out[ i ] >= 'A' && out[ i ] <= 'Z' )
            out[ i ] = (char) ( out[ i ] - 'A' + 'a' );
    return out;
}

/*-------------------------------------------------------------------------*/

static UInt64
Mix( UInt64 x )
{
    x ^= x >> 30; x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27; x *= 0x94d049bb133111ebULL;
    x ^= x >> 31;
    return x;
}

/*-------------------------------------------------------------------------*/

std::string
CGrampsText::HashHex( const std::string& text )
{GUCEF_TRACE;

    // two FNV-1a runs with different seeds, each finished with a mixer so that small differences spread
    UInt64 a = 0xcbf29ce484222325ULL;
    UInt64 b = 0x84222325cbf29ce4ULL ^ 0x9e3779b97f4a7c15ULL;
    for ( size_t i=0; i<text.size(); ++i )
    {
        a ^= (unsigned char) text[ i ];
        a *= 0x100000001b3ULL;
        b ^= (unsigned char) text[ i ] + 0x9bULL;
        b *= 0x100000001b3ULL;
        b = ( b << 13 ) | ( b >> 51 );
    }
    a = Mix( a ^ ( (UInt64) text.size() ) );
    b = Mix( b + a );
    char buffer[ 40 ];
    ::snprintf( buffer, sizeof( buffer ), "%016llx%016llx", (unsigned long long) a, (unsigned long long) b );
    return std::string( buffer );
}

/*-------------------------------------------------------------------------*/

std::string
CGrampsText::MakeId( const std::string& text )
{GUCEF_TRACE;

    std::string out( text );
    for ( size_t i=0; i<out.size(); ++i )
    {
        const char c = out[ i ];
        if ( !( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || c == '_' || c == '-' || c == '.' ) )
            out[ i ] = '_';
    }
    return out;
}

/*-------------------------------------------------------------------------*/

Int64
CGrampsText::DaysSinceEpoch( Int32 year, UInt32 month, UInt32 day )
{GUCEF_TRACE;

    // days from civil, Howard Hinnant's algorithm
    year -= month <= 2 ? 1 : 0;
    const Int64 era = ( year >= 0 ? year : year - 399 ) / 400;
    const Int64 yoe = year - era * 400;
    const Int64 doy = ( 153 * ( month + ( month > 2 ? -3 : 9 ) ) + 2 ) / 5 + day - 1;
    const Int64 doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + doe - 719468;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
