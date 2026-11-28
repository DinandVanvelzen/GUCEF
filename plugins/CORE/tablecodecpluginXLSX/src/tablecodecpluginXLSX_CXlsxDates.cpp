/*
 *  tablecodecpluginXLSX: GUCEF table codec plugin for Microsoft Excel (.xlsx) workbooks
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

#include "tablecodecpluginXLSX_CXlsxDates.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace TABLECODECPLUGIN {
namespace XLSX {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const Int32 CXlsxDates::MaxDays = 2932896;
const Int64 CXlsxDates::MicrosPerDay = 86400000000LL;

/*-------------------------------------------------------------------------*/

bool
CXlsxDates::SerialToDays( Int64 serial, bool date1904, Int32& days )
{GUCEF_TRACE;

    if ( date1904 )
    {
        // 0 is 1904-01-01, 1970-01-01 is 24107
        if ( serial < 0 || serial > 2957003 )
            return false;
        days = (Int32) ( serial - 24107 );
        return true;
    }

    // 1 is 1900-01-01, 1970-01-01 is 25569; serial 60 is the 29th of February 1900 that did not exist
    if ( serial < 1 || 60 == serial || serial > 2958465 )
        return false;
    days = (Int32) ( serial < 60 ? serial - 25568 : serial - 25569 );
    return true;
}

/*-------------------------------------------------------------------------*/

Int64
CXlsxDates::FractionToMicros( const char* digits, UInt32 digitCount )
{GUCEF_TRACE;

    // 15 digits: below 1e15, times 864 stays within 64 bits, and 86400000000 / 1e15 = 864 / 1e7
    UInt64 numerator = 0;
    for ( UInt32 i = 0; i < 15; ++i )
        numerator = numerator * 10 + ( i < digitCount ? (UInt64) ( digits[ i ] - '0' ) : 0 );
    return (Int64) ( ( numerator * 864ULL + 5000000ULL ) / 10000000ULL );
}

/*-------------------------------------------------------------------------*/

static bool
IsLeapYear( Int32 year )
{
    return ( 0 == year % 4 && 0 != year % 100 ) || 0 == year % 400;
}

/*-------------------------------------------------------------------------*/

bool
CXlsxDates::CivilToDays( Int32 year, UInt32 month, UInt32 day, Int32& days )
{GUCEF_TRACE;

    static const UInt32 daysInMonth[ 12 ] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if ( month < 1 || month > 12 || day < 1 )
        return false;
    UInt32 monthDays = daysInMonth[ month - 1 ];
    if ( 2 == month && IsLeapYear( year ) )
        monthDays = 29;
    if ( day > monthDays )
        return false;

    const Int32 y = year - ( month <= 2 ? 1 : 0 );
    const Int32 era = ( y >= 0 ? y : y - 399 ) / 400;
    const Int32 yoe = y - era * 400;
    const Int32 doy = (Int32) ( ( 153 * ( month > 2 ? month - 3 : month + 9 ) + 2 ) / 5 + day - 1 );
    const Int32 doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    days = era * 146097 + doe - 719468;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CXlsxDates::DaysToCivil( Int32 days, Int32& year, UInt32& month, UInt32& day )
{GUCEF_TRACE;

    const Int32 z = days + 719468;
    const Int32 era = ( z >= 0 ? z : z - 146096 ) / 146097;
    const Int32 doe = z - era * 146097;
    const Int32 yoe = ( doe - doe / 1460 + doe / 36524 - doe / 146096 ) / 365;
    const Int32 doy = doe - ( 365 * yoe + yoe / 4 - yoe / 100 );
    const Int32 mp = ( 5 * doy + 2 ) / 153;
    day = (UInt32) ( doy - ( 153 * mp + 2 ) / 5 + 1 );
    month = (UInt32) ( mp < 10 ? mp + 3 : mp - 9 );
    year = yoe + era * 400 + ( month <= 2 ? 1 : 0 );
}

/*-------------------------------------------------------------------------*/

static bool
ReadDigits( const char* text, UInt32 length, UInt32& pos, UInt32 count, UInt32& value )
{
    if ( pos + count > length )
        return false;
    value = 0;
    for ( UInt32 i = 0; i < count; ++i )
    {
        const char c = text[ pos + i ];
        if ( c < '0' || c > '9' )
            return false;
        value = value * 10 + (UInt32) ( c - '0' );
    }
    pos += count;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CXlsxDates::ParseIso( const char* text, UInt32 length, Int32& days, Int64& microsOfDay, bool& hasTime )
{GUCEF_TRACE;

    microsOfDay = 0;
    hasTime = false;

    UInt32 pos = 0;
    UInt32 year = 0, month = 0, day = 0;
    if ( !ReadDigits( text, length, pos, 4, year ) || pos >= length || '-' != text[ pos++ ] ||
         !ReadDigits( text, length, pos, 2, month ) || pos >= length || '-' != text[ pos++ ] ||
         !ReadDigits( text, length, pos, 2, day ) )
        return false;
    if ( !CivilToDays( (Int32) year, month, day, days ) || days > MaxDays )
        return false;
    if ( pos == length )
        return true;

    if ( 'T' != text[ pos ] && ' ' != text[ pos ] )
        return false;
    ++pos;
    UInt32 hour = 0, minute = 0, second = 0;
    if ( !ReadDigits( text, length, pos, 2, hour ) || pos >= length || ':' != text[ pos++ ] || !ReadDigits( text, length, pos, 2, minute ) )
        return false;
    Int64 fraction = 0;
    if ( pos < length && ':' == text[ pos ] )
    {
        ++pos;
        if ( !ReadDigits( text, length, pos, 2, second ) )
            return false;
        if ( pos < length && '.' == text[ pos ] )
        {
            ++pos;
            const UInt32 start = pos;
            while ( pos < length && text[ pos ] >= '0' && text[ pos ] <= '9' )
                ++pos;
            if ( pos == start )
                return false;
            // microseconds: six digits, more are cut off, fewer are padded
            Int64 scale = 100000;
            for ( UInt32 i = start; i < pos && scale > 0; ++i, scale /= 10 )
                fraction += (Int64) ( text[ i ] - '0' ) * scale;
        }
    }
    if ( pos < length && 'Z' == text[ pos ] )
        ++pos;
    if ( pos != length || hour > 24 || minute > 59 || second > 59 || ( 24 == hour && ( 0 != minute || 0 != second || 0 != fraction ) ) )
        return false;

    hasTime = true;
    microsOfDay = ( (Int64) hour * 3600 + (Int64) minute * 60 + (Int64) second ) * 1000000LL + fraction;
    return true;
}

/*-------------------------------------------------------------------------*/

static void
AppendTwo( CORE::CString& text, UInt32 value )
{
    char buffer[ 3 ];
    buffer[ 0 ] = (char) ( '0' + ( value / 10 ) % 10 );
    buffer[ 1 ] = (char) ( '0' + value % 10 );
    buffer[ 2 ] = '\0';
    text += buffer;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CXlsxDates::ToIsoText( Int32 days, Int64 microsOfDay )
{GUCEF_TRACE;

    Int32 year = 0;
    UInt32 month = 0, day = 0;
    DaysToCivil( days, year, month, day );

    CORE::CString text;
    char yearText[ 8 ];
    const UInt32 y = (UInt32) ( year < 0 ? 0 : year );
    yearText[ 0 ] = (char) ( '0' + ( y / 1000 ) % 10 );
    yearText[ 1 ] = (char) ( '0' + ( y / 100 ) % 10 );
    yearText[ 2 ] = (char) ( '0' + ( y / 10 ) % 10 );
    yearText[ 3 ] = (char) ( '0' + y % 10 );
    yearText[ 4 ] = '-';
    yearText[ 5 ] = '\0';
    text += yearText;
    AppendTwo( text, month );
    text += "-";
    AppendTwo( text, day );
    if ( 0 != microsOfDay )
    {
        const Int64 seconds = microsOfDay / 1000000;
        const UInt32 micros = (UInt32) ( microsOfDay % 1000000 );
        text += "T";
        AppendTwo( text, (UInt32) ( seconds / 3600 ) );
        text += ":";
        AppendTwo( text, (UInt32) ( ( seconds / 60 ) % 60 ) );
        text += ":";
        AppendTwo( text, (UInt32) ( seconds % 60 ) );
        if ( 0 != micros )
        {
            char fraction[ 8 ];
            fraction[ 0 ] = '.';
            UInt32 scale = 100000;
            for ( UInt32 i = 0; i < 6; ++i, scale /= 10 )
                fraction[ 1 + i ] = (char) ( '0' + ( micros / scale ) % 10 );
            fraction[ 7 ] = '\0';
            text += fraction;
        }
    }
    return text;
}

/*-------------------------------------------------------------------------*/

TXlsxNumberKind
CXlsxDates::KindOfBuiltInFormat( UInt32 numFmtId )
{GUCEF_TRACE;

    switch ( numFmtId )
    {
        case 14: case 15: case 16: case 17:
        case 27: case 28: case 29: case 30: case 31: case 36:
        case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 58:
            return XLSX_NUMBERKIND_DATE;
        case 22:
            return XLSX_NUMBERKIND_DATETIME;
        case 18: case 19: case 20: case 21:
        case 32: case 33: case 34: case 35:
        case 45: case 46: case 47:
            return XLSX_NUMBERKIND_TIME;
        default:
            return XLSX_NUMBERKIND_PLAIN;
    }
}

/*-------------------------------------------------------------------------*/

TXlsxNumberKind
CXlsxDates::KindOfFormatCode( const CORE::CString& formatCode )
{GUCEF_TRACE;

    const char* code = formatCode.C_String();
    const UInt32 length = formatCode.ByteSize() > 0 ? formatCode.ByteSize() - 1 : 0;
    if ( 0 == length )
        return XLSX_NUMBERKIND_PLAIN;

    bool hasDateToken = false;     // y, d
    bool hasTimeToken = false;     // h, s, [h], [m], [s]
    bool hasMonthOrMinute = false; // m

    // only the first section (positive numbers) tells what the number is
    UInt32 i = 0;
    while ( i < length )
    {
        const char c = code[ i ];
        if ( ';' == c )
            break;
        if ( '"' == c )
        {
            ++i;
            while ( i < length && '"' != code[ i ] )
                ++i;
            ++i;
            continue;
        }
        if ( '\\' == c || '_' == c || '*' == c )
        {
            i += 2;
            continue;
        }
        if ( '[' == c )
        {
            // [h] [mm] [ss] count elapsed time, the other brackets are colors, conditions and locales
            UInt32 end = i + 1;
            while ( end < length && ']' != code[ end ] )
                ++end;
            bool onlyOne = end > i + 1;
            char kind = 0;
            for ( UInt32 n = i + 1; n < end; ++n )
            {
                char t = code[ n ];
                if ( t >= 'A' && t <= 'Z' )
                    t = (char) ( t - 'A' + 'a' );
                if ( 0 == kind )
                    kind = t;
                if ( t != kind )
                    onlyOne = false;
            }
            if ( onlyOne && ( 'h' == kind || 'm' == kind || 's' == kind ) )
                hasTimeToken = true;
            i = end + 1;
            continue;
        }
        // AM/PM and A/P are markers of a 12 hour clock, their M is no month
        if ( ( 'A' == c || 'a' == c ) && i + 4 < length + 0 && 0 == strncmp( code + i + 1, "M/P", 3 ) )
        {
            i += 5;
            hasTimeToken = true;
            continue;
        }
        if ( ( 'A' == c || 'a' == c ) && i + 2 < length && '/' == code[ i + 1 ] && ( 'P' == code[ i + 2 ] || 'p' == code[ i + 2 ] ) )
        {
            i += 3;
            hasTimeToken = true;
            continue;
        }
        switch ( c )
        {
            case 'y': case 'Y': case 'd': case 'D': hasDateToken = true; break;
            case 'h': case 'H': case 's': case 'S': hasTimeToken = true; break;
            case 'm': case 'M': hasMonthOrMinute = true; break;
            default: break;
        }
        ++i;
    }

    if ( hasDateToken && hasTimeToken )
        return XLSX_NUMBERKIND_DATETIME;
    if ( hasDateToken )
        return XLSX_NUMBERKIND_DATE;
    if ( hasTimeToken )
        return XLSX_NUMBERKIND_TIME;
    if ( hasMonthOrMinute )
        return XLSX_NUMBERKIND_DATE;       // "mmm" and "mmmm": a month name
    return XLSX_NUMBERKIND_PLAIN;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace XLSX */
}; /* namespace TABLECODECPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
