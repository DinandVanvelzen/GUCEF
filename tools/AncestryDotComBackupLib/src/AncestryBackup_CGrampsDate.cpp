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

#include "AncestryBackup_CGrampsDate.h"

#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>

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

static const char* const MonthNames[ 12 ] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
static const char* const LongMonthNames[ 12 ] = { "JANUARY", "FEBRUARY", "MARCH", "APRIL", "MAY", "JUNE", "JULY", "AUGUST", "SEPTEMBER", "OCTOBER", "NOVEMBER", "DECEMBER" };

/*-------------------------------------------------------------------------*/

static std::string
Upper( const std::string& text )
{
    std::string out( text );
    for ( size_t i=0; i<out.size(); ++i )
        if ( out[ i ] >= 'a' && out[ i ] <= 'z' )
            out[ i ] = (char) ( out[ i ] - 'a' + 'A' );
    return out;
}

/*-------------------------------------------------------------------------*/

static UInt32
MonthOf( const std::string& upperToken )
{
    for ( UInt32 i=0; i<12; ++i )
        if ( upperToken == MonthNames[ i ] || upperToken == LongMonthNames[ i ] )
            return i + 1;
    if ( upperToken == "SEPT" )
        return 9;
    return 0;
}

/*-------------------------------------------------------------------------*/

static bool
IsLeap( Int32 year )
{
    return ( year % 4 == 0 && year % 100 != 0 ) || year % 400 == 0;
}

/*-------------------------------------------------------------------------*/

static UInt32
DaysIn( Int32 year, UInt32 month )
{
    static const UInt32 days[ 12 ] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    return month == 2 && IsLeap( year ) ? 29 : days[ month - 1 ];
}

/*-------------------------------------------------------------------------*/

static bool
IsDigits( const std::string& s, size_t maxLen )
{
    if ( s.empty() || s.size() > maxLen )
        return false;
    for ( size_t i=0; i<s.size(); ++i )
        if ( s[ i ] < '0' || s[ i ] > '9' )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

static void
Split( const std::string& text, const char* separators, std::vector< std::string >& tokens )
{
    std::string current;
    for ( size_t i=0; i<text.size(); ++i )
    {
        bool sep = false;
        for ( const char* s = separators; *s != '\0'; ++s )
            if ( text[ i ] == *s )
                sep = true;
        if ( sep )
        {
            if ( !current.empty() )
                tokens.push_back( current );
            current.clear();
        }
        else
            current += text[ i ];
    }
    if ( !current.empty() )
        tokens.push_back( current );
}

/*-------------------------------------------------------------------------*/

bool
CGrampsDate::ParseSimple( const std::string& text, Int32& year, UInt32& month, UInt32& day )
{GUCEF_TRACE;

    std::string u = Upper( CGrampsText::Trim( text ) );
    if ( u.empty() )
        return false;

    // an era at the end
    bool bce = false;
    static const char* const eras[] = { " B.C.E.", " B.C.", " BCE", " BC" };
    for ( int i=0; i<4; ++i )
    {
        const std::string era( eras[ i ] );
        if ( u.size() > era.size() && u.compare( u.size() - era.size(), era.size(), era ) == 0 )
        {
            u.erase( u.size() - era.size() );
            bce = true;
            break;
        }
    }

    year = 0;
    month = 0;
    day = 0;

    // numeric forms with - . or / between the parts
    bool numeric = true;
    for ( size_t i=0; i<u.size(); ++i )
        if ( !( ( u[ i ] >= '0' && u[ i ] <= '9' ) || u[ i ] == '-' || u[ i ] == '.' || u[ i ] == '/' ) )
            numeric = false;
    std::vector< std::string > parts;
    if ( numeric )
    {
        Split( u, "-./", parts );
        if ( parts.size() == 1 )
        {
            if ( !IsDigits( parts[ 0 ], 4 ) || u.find_first_of( "-./" ) != std::string::npos )
                return false;
            year = ::atoi( parts[ 0 ].c_str() );
        }
        else
        if ( parts.size() == 3 )
        {
            UInt32 a = 0, b = 0;
            if ( parts[ 0 ].size() == 4 && IsDigits( parts[ 0 ], 4 ) && IsDigits( parts[ 1 ], 2 ) && IsDigits( parts[ 2 ], 2 ) )
            {
                year = ::atoi( parts[ 0 ].c_str() );
                month = (UInt32) ::atoi( parts[ 1 ].c_str() );
                day = (UInt32) ::atoi( parts[ 2 ].c_str() );
            }
            else
            if ( parts[ 2 ].size() == 4 && IsDigits( parts[ 2 ], 4 ) && IsDigits( parts[ 0 ], 2 ) && IsDigits( parts[ 1 ], 2 ) )
            {
                a = (UInt32) ::atoi( parts[ 0 ].c_str() );
                b = (UInt32) ::atoi( parts[ 1 ].c_str() );
                year = ::atoi( parts[ 2 ].c_str() );
                // 3-4-1900 can be the 3rd of April or the 4th of March: only a part which is more than 12 settles it
                if ( a > 12 && b >= 1 && b <= 12 )
                { day = a; month = b; }
                else
                if ( b > 12 && a >= 1 && a <= 12 )
                { month = a; day = b; }
                else
                if ( a == b && a >= 1 && a <= 12 )
                { month = a; day = b; }
                else
                    return false;
            }
            else
                return false;
        }
        else
            return false;
    }
    else
    {
        Split( u, " ,.", parts );
        if ( parts.empty() || parts.size() > 3 )
            return false;
        if ( parts.size() == 1 )
        {
            if ( !IsDigits( parts[ 0 ], 4 ) )
                return false;
            year = ::atoi( parts[ 0 ].c_str() );
        }
        else
        if ( parts.size() == 2 )
        {
            month = MonthOf( parts[ 0 ] );
            if ( month == 0 || !IsDigits( parts[ 1 ], 4 ) )
                return false;
            year = ::atoi( parts[ 1 ].c_str() );
        }
        else
        {
            if ( MonthOf( parts[ 1 ] ) != 0 && IsDigits( parts[ 0 ], 2 ) && IsDigits( parts[ 2 ], 4 ) )
            {
                day = (UInt32) ::atoi( parts[ 0 ].c_str() );
                month = MonthOf( parts[ 1 ] );
                year = ::atoi( parts[ 2 ].c_str() );
            }
            else
            if ( MonthOf( parts[ 0 ] ) != 0 && IsDigits( parts[ 1 ], 2 ) && IsDigits( parts[ 2 ], 4 ) )
            {
                month = MonthOf( parts[ 0 ] );
                day = (UInt32) ::atoi( parts[ 1 ].c_str() );
                year = ::atoi( parts[ 2 ].c_str() );
            }
            else
                return false;
        }
    }

    if ( year < 1 || year > 9999 )
        return false;
    if ( month != 0 && ( month < 1 || month > 12 ) )
        return false;
    if ( day != 0 && ( month == 0 || day < 1 || day > DaysIn( year, month ) ) )
        return false;
    if ( bce )
        year = -year;
    return true;
}

/*-------------------------------------------------------------------------*/

static std::string
FormatSimple( Int32 year, UInt32 month, UInt32 day )
{
    char buffer[ 32 ];
    const Int32 y = year < 0 ? -year : year;
    if ( month == 0 )
        ::snprintf( buffer, sizeof( buffer ), "%s%04d", year < 0 ? "-" : "", (int) y );
    else
    if ( day == 0 )
        ::snprintf( buffer, sizeof( buffer ), "%s%04d-%02u", year < 0 ? "-" : "", (int) y, (unsigned) month );
    else
        ::snprintf( buffer, sizeof( buffer ), "%s%04d-%02u-%02u", year < 0 ? "-" : "", (int) y, (unsigned) month, (unsigned) day );
    return std::string( buffer );
}

/*-------------------------------------------------------------------------*/

static bool
SimpleToValue( const std::string& text, std::string& value )
{
    Int32 y; UInt32 m, d;
    if ( !CGrampsDate::ParseSimple( text, y, m, d ) )
        return false;
    value = FormatSimple( y, m, d );
    return true;
}

/*-------------------------------------------------------------------------*/

/** Splits a range like 1921-1940 or 1921 - 1940 or 1921 AND 1940 or with an en dash */
static bool
SplitRange( const std::string& text, std::string& first, std::string& second )
{
    static const char* const separators[] = { " AND ", "\xE2\x80\x93", "\xE2\x80\x94", " - ", " TO " };
    for ( int i=0; i<5; ++i )
    {
        const size_t at = text.find( separators[ i ] );
        if ( at != std::string::npos )
        {
            first = CGrampsText::Trim( text.substr( 0, at ) );
            second = CGrampsText::Trim( text.substr( at + ::strlen( separators[ i ] ) ) );
            return true;
        }
    }
    // 1921-1940, only between two years, a date like 1-5-1921 has other dashes in it
    const size_t dash = text.find( '-' );
    if ( dash != std::string::npos && text.find( '-', dash + 1 ) == std::string::npos )
    {
        first = CGrampsText::Trim( text.substr( 0, dash ) );
        second = CGrampsText::Trim( text.substr( dash + 1 ) );
        return IsDigits( first, 4 ) && IsDigits( second, 4 );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

EGrampsDateKind
CGrampsDate::ToXml( const std::string& gedcomDate, std::string& xml )
{GUCEF_TRACE;

    xml.clear();
    const std::string original = CGrampsText::Trim( gedcomDate );
    if ( original.empty() )
        return GRAMPSDATE_NONE;

    std::string u = Upper( original );

    // the keyword in front
    std::string keyword;
    std::string rest = u;
    {
        size_t e = 0;
        while ( e < u.size() && u[ e ] >= 'A' && u[ e ] <= 'Z' )
            ++e;
        if ( e > 0 && e < u.size() )
        {
            keyword = u.substr( 0, e );
            rest = u.substr( e );
            if ( !rest.empty() && rest[ 0 ] == '.' )
                rest.erase( 0, 1 );
            rest = CGrampsText::Trim( rest );
        }
        else
        if ( e == u.size() )
            keyword.clear();
    }

    const char* type = GUCEF_NULL;
    const char* quality = GUCEF_NULL;
    bool compound = false;
    bool recognizedKeyword = true;
    if ( keyword == "ABT" || keyword == "ABOUT" || keyword == "CIRCA" || keyword == "CA" )
        type = "about";
    else
    if ( keyword == "EST" || keyword == "ESTIMATED" )
    { type = "about"; quality = "estimated"; }
    else
    if ( keyword == "CAL" || keyword == "CALCULATED" )
    { type = "about"; quality = "calculated"; }
    else
    if ( keyword == "BEF" || keyword == "BEFORE" )
        type = "before";
    else
    if ( keyword == "AFT" || keyword == "AFTER" )
        type = "after";
    else
    if ( keyword == "BET" || keyword == "BETWEEN" || keyword == "FROM" )
        compound = true;
    else
        recognizedKeyword = false;

    std::string value, first, second;
    if ( !recognizedKeyword )
    {
        // no keyword: a plain date, or text
        if ( SimpleToValue( u, value ) )
        {
            xml = "<dateval val=\"" + value + "\"/>";
            return GRAMPSDATE_VALUE;
        }
    }
    else
    if ( !compound )
    {
        if ( SimpleToValue( rest, value ) )
        {
            xml = "<dateval val=\"" + value + "\" type=\"" + type + "\"";
            if ( quality != GUCEF_NULL )
                xml += std::string( " quality=\"" ) + quality + "\"";
            xml += "/>";
            return GRAMPSDATE_VALUE;
        }
    }
    else
    {
        std::string v1, v2;
        if ( SplitRange( rest, first, second ) && SimpleToValue( first, v1 ) && SimpleToValue( second, v2 ) )
        {
            const bool isSpan = keyword == "FROM";
            xml = std::string( isSpan ? "<datespan" : "<daterange" ) + " start=\"" + v1 + "\" stop=\"" + v2 + "\"/>";
            return isSpan ? GRAMPSDATE_SPAN : GRAMPSDATE_RANGE;
        }
    }

    xml = "<datestr val=\"" + CGrampsText::XmlEscape( CGrampsText::DecodeHtmlEntities( original ) ) + "\"/>";
    return GRAMPSDATE_TEXT;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
