/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CDATE_H
#include "gucefCORE_CDate.h"
#define GUCEF_CORE_CDATE_H
#endif /* GUCEF_CORE_CDATE_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_CDECIMAL_H
#include "gucefCORE_CDecimal.h"
#define GUCEF_CORE_CDECIMAL_H
#endif /* GUCEF_CORE_CDECIMAL_H ? */

#include "gucefCORE_CDataTableValues.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const Int64 g_secondsPerDay = 86400;

/** Floor division for a positive divisor */
inline Int64
FloorDiv64( Int64 a, Int64 b )
{
    Int64 q = a / b;
    if ( ( a % b ) < 0 )
        --q;
    return q;
}

/** Floor division with a remainder in [0, b) for a positive divisor, without overflow */
inline void
FloorDivMod64( Int64 a, Int64 b, Int64& quotient, Int64& remainder )
{
    quotient = a / b;
    remainder = a % b;
    if ( remainder < 0 )
    {
        remainder += b;
        --quotient;
    }
}

/** Year, month and day of a day count since 1970-01-01, days from civil by H. Hinnant, in 64 bits */
void
CivilFromDays64( Int64 days, Int64& year, UInt32& month, UInt32& day )
{
    Int64 z = days + 719468;
    Int64 era = FloorDiv64( z, 146097 );
    Int64 dayOfEra = z - era * 146097;                                                          // [0, 146096]
    Int64 yearOfEra = ( dayOfEra - dayOfEra / 1460 + dayOfEra / 36524 - dayOfEra / 146096 ) / 365;    // [0, 399]
    Int64 y = yearOfEra + era * 400;
    Int64 dayOfYear = dayOfEra - ( 365 * yearOfEra + yearOfEra / 4 - yearOfEra / 100 );        // [0, 365]
    Int64 mp = ( 5 * dayOfYear + 2 ) / 153;                                                     // [0, 11]
    day = (UInt32) ( dayOfYear - ( 153 * mp + 2 ) / 5 + 1 );                                    // [1, 31]
    month = (UInt32) ( mp < 10 ? mp + 3 : mp - 9 );                                             // [1, 12]
    year = month <= 2 ? y + 1 : y;
}

/** The day count since 1970-01-01 of a civil date, in 64 bits */
Int64
DaysFromCivil64( Int64 year, UInt32 month, UInt32 day )
{
    Int64 y = month <= 2 ? year - 1 : year;
    Int64 era = FloorDiv64( y, 400 );
    Int64 yearOfEra = y - era * 400;                                                            // [0, 399]
    Int64 dayOfYear = ( 153 * ( month > 2 ? (Int64) month - 3 : (Int64) month + 9 ) + 2 ) / 5 + (Int64) day - 1;    // [0, 365]
    Int64 dayOfEra = yearOfEra * 365 + yearOfEra / 4 - yearOfEra / 100 + dayOfYear;            // [0, 146096]
    return era * 146097 + dayOfEra - 719468;
}

/** Days in the month of the year */
UInt32
DaysInMonth( Int64 year, UInt32 month )
{
    static const UInt32 days[ 12 ] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if ( 2 == month )
    {
        bool leap = ( 0 == year % 4 && 0 != year % 100 ) || 0 == year % 400;
        return leap ? 29 : 28;
    }
    return days[ month - 1 ];
}

/** Writes the year: 4 digits for 0..9999, otherwise a sign and at least 4 digits */
UInt32
WriteYear( Int64 year, char* out )
{
    UInt32 pos = 0;
    UInt64 magnitude = year < 0 ? (UInt64) 0 - (UInt64) year : (UInt64) year;
    if ( year < 0 )
        out[ pos++ ] = '-';
    else
    if ( year > 9999 )
        out[ pos++ ] = '+';

    char digits[ 24 ];
    UInt32 count = 0;
    do
    {
        digits[ count++ ] = (char) ( '0' + ( magnitude % 10 ) );
        magnitude /= 10;
    }
    while ( magnitude > 0 );
    while ( count < 4 )
        digits[ count++ ] = '0';
    while ( count > 0 )
        out[ pos++ ] = digits[ --count ];
    return pos;
}

/** Writes value as exactly digitCount decimal digits */
inline void
WriteDigits( UInt64 value, UInt32 digitCount, char* out )
{
    for ( UInt32 i=digitCount; i>0; --i )
    {
        out[ i-1 ] = (char) ( '0' + ( value % 10 ) );
        value /= 10;
    }
}

/** Reads exactly digitCount decimal digits */
inline bool
ReadDigits( const char* text, UInt32 digitCount, UInt32& value )
{
    UInt32 result = 0;
    for ( UInt32 i=0; i<digitCount; ++i )
    {
        char c = text[ i ];
        if ( c < '0' || c > '9' )
            return false;
        result = result * 10 + (UInt32) ( c - '0' );
    }
    value = result;
    return true;
}

/** Parses the yyyy-mm-dd part at the start of the text, returns the characters used or 0 */
UInt32
ParseDatePart( const char* text, UInt32 length, Int64& days )
{
    UInt32 pos = 0;
    bool negative = false;
    bool signedYear = false;
    if ( pos < length && ( '-' == text[ pos ] || '+' == text[ pos ] ) )
    {
        negative = '-' == text[ pos ];
        signedYear = true;
        ++pos;
    }

    UInt32 yearDigits = 0;
    Int64 year = 0;
    while ( pos < length && text[ pos ] >= '0' && text[ pos ] <= '9' )
    {
        if ( yearDigits >= 9 )
            return 0;
        year = year * 10 + ( text[ pos ] - '0' );
        ++yearDigits;
        ++pos;
    }
    if ( yearDigits < 4 || ( yearDigits > 4 && !signedYear ) )
        return 0;
    if ( negative )
        year = -year;

    UInt32 month = 0, day = 0;
    if ( pos + 6 > length || '-' != text[ pos ] || !ReadDigits( text + pos + 1, 2, month ) ||
         '-' != text[ pos + 3 ] || !ReadDigits( text + pos + 4, 2, day ) )
        return 0;
    pos += 6;

    if ( month < 1 || month > 12 || day < 1 || day > DaysInMonth( year, month ) )
        return 0;

    days = DaysFromCivil64( year, month, day );
    return pos;
}

/** Multiplies with an overflow check */
inline bool
CheckedMultiply( Int64 a, Int64 b, Int64& result )
{
    if ( 0 == a || 0 == b )
    {
        result = 0;
        return true;
    }
    if ( a > 0 ? ( b > 0 ? a > GUCEF_INT64MAX / b : b < GUCEF_INT64MIN / a )
               : ( b > 0 ? a < GUCEF_INT64MIN / b : a < GUCEF_INT64MAX / b ) )
        return false;
    result = a * b;
    return true;
}

/** Adds with an overflow check */
inline bool
CheckedAdd( Int64 a, Int64 b, Int64& result )
{
    if ( ( b > 0 && a > GUCEF_INT64MAX - b ) || ( b < 0 && a < GUCEF_INT64MIN - b ) )
        return false;
    result = a + b;
    return true;
}

/** The power of 10 between two units, positive when to is finer than from */
inline Int32
UnitExponentDelta( TDataTableTimeUnit fromUnit, TDataTableTimeUnit toUnit )
{
    return 3 * ( (Int32) toUnit - (Int32) fromUnit );
}

const Int64 g_powersOf10[] = { 1LL, 10LL, 100LL, 1000LL, 10000LL, 100000LL, 1000000LL, 10000000LL, 100000000LL, 1000000000LL };

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVariant
CDataTableValues::Date( const CDate& date )
{GUCEF_TRACE;

    if ( !date.IsValid() )
        return CVariant();
    return Date( ToDays( date ) );
}

/*-------------------------------------------------------------------------*/

CVariant
CDataTableValues::Date( Int32 daysSinceEpoch )
{GUCEF_TRACE;

    CVariant result;
    result.Set( &daysSinceEpoch, sizeof( daysSinceEpoch ), GUCEF_DATATYPE_DATE_IN_DAYS_SINCE_UNIX_EPOCH, false );
    return result;
}

/*-------------------------------------------------------------------------*/

CVariant
CDataTableValues::Timestamp( Int64 ticks, TDataTableTimeUnit unit )
{GUCEF_TRACE;

    CVariant result;
    result.Set( &ticks, sizeof( ticks ), TimestampTypeId( unit ), false );
    return result;
}

/*-------------------------------------------------------------------------*/

CVariant
CDataTableValues::Timestamp( const CDateTime& dateTime, TDataTableTimeUnit unit )
{GUCEF_TRACE;

    Int64 ticks = 0;
    if ( !TicksFromDateTime( dateTime, unit, ticks ) )
        return CVariant();
    return Timestamp( ticks, unit );
}

/*-------------------------------------------------------------------------*/

CVariant
CDataTableValues::Decimal( const CDecimal& value )
{GUCEF_TRACE;

    return CVariant( value );
}

/*-------------------------------------------------------------------------*/

Int32
CDataTableValues::ToDays( const CDate& date )
{GUCEF_TRACE;

    return date.ToDaysSinceUnixEpoch();
}

/*-------------------------------------------------------------------------*/

CDate
CDataTableValues::FromDays( Int32 days )
{GUCEF_TRACE;

    CDate date;
    if ( !date.FromDaysSinceUnixEpoch( days ) )
        return CDate();
    return date;
}

/*-------------------------------------------------------------------------*/

UInt8
CDataTableValues::TimestampTypeId( TDataTableTimeUnit unit )
{GUCEF_TRACE;

    switch ( unit )
    {
        case DATATABLE_TIMEUNIT_MILLISECONDS:   return GUCEF_DATATYPE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH;
        case DATATABLE_TIMEUNIT_NANOSECONDS:    return GUCEF_DATATYPE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH;
        case DATATABLE_TIMEUNIT_MICROSECONDS:
        default:                                return GUCEF_DATATYPE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH;
    }
}

/*-------------------------------------------------------------------------*/

bool
CDataTableValues::TimeUnitFromTypeId( UInt8 typeId, TDataTableTimeUnit& unit )
{GUCEF_TRACE;

    switch ( typeId )
    {
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH:        { unit = DATATABLE_TIMEUNIT_MILLISECONDS; return true; }
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH:    { unit = DATATABLE_TIMEUNIT_MICROSECONDS; return true; }
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH:     { unit = DATATABLE_TIMEUNIT_NANOSECONDS; return true; }
        default:                                                        { return false; }
    }
}

/*-------------------------------------------------------------------------*/

Int64
CDataTableValues::TicksPerSecond( TDataTableTimeUnit unit )
{GUCEF_TRACE;

    switch ( unit )
    {
        case DATATABLE_TIMEUNIT_MILLISECONDS:   return 1000LL;
        case DATATABLE_TIMEUNIT_NANOSECONDS:    return 1000000000LL;
        case DATATABLE_TIMEUNIT_MICROSECONDS:
        default:                                return 1000000LL;
    }
}

/*-------------------------------------------------------------------------*/

Int64
CDataTableValues::TicksPerDay( TDataTableTimeUnit unit )
{GUCEF_TRACE;

    return g_secondsPerDay * TicksPerSecond( unit );
}

/*-------------------------------------------------------------------------*/

bool
CDataTableValues::ConvertTicks( Int64 ticks, TDataTableTimeUnit fromUnit, TDataTableTimeUnit toUnit, Int64& result )
{GUCEF_TRACE;

    Int32 delta = UnitExponentDelta( fromUnit, toUnit );
    if ( 0 == delta )
    {
        result = ticks;
        return true;
    }
    if ( delta > 0 )
        return CheckedMultiply( ticks, g_powersOf10[ delta ], result );

    Int64 divisor = g_powersOf10[ -delta ];
    if ( 0 != ticks % divisor )
        return false;
    result = ticks / divisor;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableValues::ConvertTicksRounded( Int64 ticks, TDataTableTimeUnit fromUnit, TDataTableTimeUnit toUnit, bool roundUp, Int64& result )
{GUCEF_TRACE;

    Int32 delta = UnitExponentDelta( fromUnit, toUnit );
    if ( delta >= 0 )
        return ConvertTicks( ticks, fromUnit, toUnit, result );

    Int64 divisor = g_powersOf10[ -delta ];
    Int64 quotient = 0, remainder = 0;
    FloorDivMod64( ticks, divisor, quotient, remainder );
    if ( roundUp && 0 != remainder )
        ++quotient;
    result = quotient;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableValues::TicksFromDateTime( const CDateTime& dateTime, TDataTableTimeUnit unit, Int64& ticks )
{GUCEF_TRACE;

    if ( !dateTime.IsValid() )
        return false;

    Int64 days = (Int64) dateTime.ToDaysSinceUnixEpoch();
    Int64 milliseconds = days * g_secondsPerDay * 1000 +
                         (Int64) dateTime.GetHours() * 3600000 +
                         (Int64) dateTime.GetMinutes() * 60000 +
                         (Int64) dateTime.GetSeconds() * 1000 +
                         (Int64) dateTime.GetMilliseconds() -
                         (Int64) dateTime.GetTimeZoneUTCOffsetInMins() * 60000;
    return ConvertTicks( milliseconds, DATATABLE_TIMEUNIT_MILLISECONDS, unit, ticks );
}

/*-------------------------------------------------------------------------*/

bool
CDataTableValues::DateTimeFromTicks( Int64 ticks, TDataTableTimeUnit unit, CDateTime& dateTime )
{GUCEF_TRACE;

    Int64 milliseconds = 0;
    ConvertTicksRounded( ticks, unit, DATATABLE_TIMEUNIT_MILLISECONDS, false, milliseconds );

    const Int64 msPerDay = g_secondsPerDay * 1000;
    Int64 days = 0, msOfDay = 0;
    FloorDivMod64( milliseconds, msPerDay, days, msOfDay );
    if ( days < GUCEF_INT32MIN || days > GUCEF_INT32MAX )
        return false;

    CDate date;
    if ( !date.FromDaysSinceUnixEpoch( (Int32) days ) )
        return false;

    CTime time( (Int8) ( msOfDay / 3600000 ), (Int8) ( ( msOfDay / 60000 ) % 60 ), (Int8) ( ( msOfDay / 1000 ) % 60 ), (Int16) ( msOfDay % 1000 ) );
    dateTime = CDateTime( date, time, 0 );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableValues::TicksFromVariant( const CVariant& value, TDataTableTimeUnit unit, Int64& ticks )
{GUCEF_TRACE;

    const UInt8 typeId = value.GetTypeId();
    switch ( typeId )
    {
        case GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH:
        {
            return CheckedMultiply( (Int64) value.AsInt32(), TicksPerDay( unit ), ticks );
        }
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH:
        {
            CVariant native( value );
            native.ToNativeEndian();
            return CheckedMultiply( native.AsInt64(), TicksPerSecond( unit ), ticks );
        }
        default:
        {
            TDataTableTimeUnit valueUnit = DATATABLE_TIMEUNIT_MICROSECONDS;
            if ( !TimeUnitFromTypeId( typeId, valueUnit ) )
                return false;
            CVariant native( value );
            native.ToNativeEndian();
            return ConvertTicks( native.AsInt64(), valueUnit, unit, ticks );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CDataTableValues::CivilFromDays( Int32 days, Int32& year, UInt32& month, UInt32& day )
{GUCEF_TRACE;

    Int64 y = 0;
    CivilFromDays64( days, y, month, day );
    year = (Int32) y;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableValues::DaysFromCivil( Int32 year, UInt32 month, UInt32 day, Int32& days )
{GUCEF_TRACE;

    if ( month < 1 || month > 12 || day < 1 || day > DaysInMonth( year, month ) )
        return false;
    Int64 result = DaysFromCivil64( year, month, day );
    if ( result < GUCEF_INT32MIN || result > GUCEF_INT32MAX )
        return false;
    days = (Int32) result;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CDataTableValues::FormatDate( Int32 days, char* buffer, UInt32 bufferSize )
{GUCEF_TRACE;

    if ( GUCEF_NULL == buffer || bufferSize < MaxDateTextLength + 1 )
        return 0;

    Int64 year = 0;
    UInt32 month = 0, day = 0;
    CivilFromDays64( days, year, month, day );

    UInt32 pos = WriteYear( year, buffer );
    buffer[ pos++ ] = '-';
    WriteDigits( month, 2, buffer + pos ); pos += 2;
    buffer[ pos++ ] = '-';
    WriteDigits( day, 2, buffer + pos ); pos += 2;
    buffer[ pos ] = '\0';
    return pos;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableValues::ParseDate( const char* text, UInt32 length, Int32& days )
{GUCEF_TRACE;

    if ( GUCEF_NULL == text )
        return false;

    Int64 result = 0;
    UInt32 used = ParseDatePart( text, length, result );
    if ( 0 == used || used != length || result < GUCEF_INT32MIN || result > GUCEF_INT32MAX )
        return false;
    days = (Int32) result;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CDataTableValues::FormatTimestamp( Int64 ticks, TDataTableTimeUnit unit, bool utcSuffix, char* buffer, UInt32 bufferSize )
{GUCEF_TRACE;

    if ( GUCEF_NULL == buffer || bufferSize < MaxTimestampTextLength + 1 )
        return 0;

    const Int64 perDay = TicksPerDay( unit );
    const Int64 perSecond = TicksPerSecond( unit );
    Int64 days = 0, tickOfDay = 0;
    FloorDivMod64( ticks, perDay, days, tickOfDay );
    Int64 secondOfDay = tickOfDay / perSecond;
    Int64 fraction = tickOfDay % perSecond;

    Int64 year = 0;
    UInt32 month = 0, day = 0;
    CivilFromDays64( days, year, month, day );

    UInt32 pos = WriteYear( year, buffer );
    buffer[ pos++ ] = '-';
    WriteDigits( month, 2, buffer + pos ); pos += 2;
    buffer[ pos++ ] = '-';
    WriteDigits( day, 2, buffer + pos ); pos += 2;
    buffer[ pos++ ] = 'T';
    WriteDigits( (UInt64) ( secondOfDay / 3600 ), 2, buffer + pos ); pos += 2;
    buffer[ pos++ ] = ':';
    WriteDigits( (UInt64) ( ( secondOfDay / 60 ) % 60 ), 2, buffer + pos ); pos += 2;
    buffer[ pos++ ] = ':';
    WriteDigits( (UInt64) ( secondOfDay % 60 ), 2, buffer + pos ); pos += 2;
    buffer[ pos++ ] = '.';
    UInt32 fractionDigits = 3 + 3 * (UInt32) unit;
    WriteDigits( (UInt64) fraction, fractionDigits, buffer + pos ); pos += fractionDigits;
    if ( utcSuffix )
        buffer[ pos++ ] = 'Z';
    buffer[ pos ] = '\0';
    return pos;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableValues::ParseTimestamp( const char* text, UInt32 length, TDataTableTimeUnit unit, Int64& ticks )
{GUCEF_TRACE;

    if ( GUCEF_NULL == text )
        return false;

    Int64 days = 0;
    UInt32 pos = ParseDatePart( text, length, days );
    if ( 0 == pos )
        return false;

    Int64 secondOfDay = 0;
    Int64 fractionTicks = 0;
    Int64 offsetSeconds = 0;
    if ( pos < length )
    {
        if ( 'T' != text[ pos ] && 't' != text[ pos ] && ' ' != text[ pos ] )
            return false;
        ++pos;

        UInt32 hours = 0, minutes = 0, seconds = 0;
        if ( pos + 5 > length || !ReadDigits( text + pos, 2, hours ) || ':' != text[ pos + 2 ] || !ReadDigits( text + pos + 3, 2, minutes ) )
            return false;
        pos += 5;
        if ( pos < length && ':' == text[ pos ] )
        {
            if ( pos + 3 > length || !ReadDigits( text + pos + 1, 2, seconds ) )
                return false;
            pos += 3;

            if ( pos < length && ( '.' == text[ pos ] || ',' == text[ pos ] ) )
            {
                ++pos;
                const UInt32 unitDigits = 3 + 3 * (UInt32) unit;
                UInt32 digits = 0;
                while ( pos < length && text[ pos ] >= '0' && text[ pos ] <= '9' )
                {
                    if ( digits >= 9 )
                        return false;
                    Int64 digit = text[ pos ] - '0';
                    if ( digits < unitDigits )
                        fractionTicks = fractionTicks * 10 + digit;
                    else
                    if ( 0 != digit )
                        return false;   // finer than the unit can hold
                    ++digits;
                    ++pos;
                }
                if ( 0 == digits )
                    return false;
                for ( UInt32 d=digits; d<unitDigits; ++d )
                    fractionTicks *= 10;
            }
        }
        if ( hours > 23 || minutes > 59 || seconds > 59 )
            return false;
        secondOfDay = (Int64) hours * 3600 + (Int64) minutes * 60 + (Int64) seconds;

        if ( pos < length )
        {
            char zone = text[ pos ];
            if ( 'Z' == zone || 'z' == zone )
            {
                ++pos;
            }
            else
            if ( '+' == zone || '-' == zone )
            {
                UInt32 offsetHours = 0, offsetMinutes = 0;
                if ( pos + 3 > length || !ReadDigits( text + pos + 1, 2, offsetHours ) )
                    return false;
                pos += 3;
                if ( pos < length )
                {
                    if ( ':' == text[ pos ] )
                        ++pos;
                    if ( pos + 2 > length || !ReadDigits( text + pos, 2, offsetMinutes ) )
                        return false;
                    pos += 2;
                }
                if ( offsetHours > 23 || offsetMinutes > 59 )
                    return false;
                offsetSeconds = (Int64) offsetHours * 3600 + (Int64) offsetMinutes * 60;
                if ( '-' == zone )
                    offsetSeconds = -offsetSeconds;
            }
        }
        if ( pos != length )
            return false;
    }

    // local time minus the offset is UTC; for days before the epoch a day is moved into the time of day,
    // so the product of days and ticks per day does not leave the Int64 range before the sum would
    const Int64 perDay = TicksPerDay( unit );
    Int64 intraDay = ( secondOfDay - offsetSeconds ) * TicksPerSecond( unit ) + fractionTicks;
    if ( days < 0 )
    {
        days += 1;
        intraDay -= perDay;
    }
    Int64 dayTicks = 0, total = 0;
    if ( !CheckedMultiply( days, perDay, dayTicks ) || !CheckedAdd( dayTicks, intraDay, total ) )
        return false;
    ticks = total;
    return true;
}

/*-------------------------------------------------------------------------*/

CString
CDataTableValues::DateToString( Int32 days )
{GUCEF_TRACE;

    char buffer[ MaxDateTextLength + 1 ];
    if ( 0 == FormatDate( days, buffer, sizeof( buffer ) ) )
        return CString::Empty;
    return CString( buffer );
}

/*-------------------------------------------------------------------------*/

CString
CDataTableValues::TimestampToString( Int64 ticks, TDataTableTimeUnit unit, bool utcSuffix )
{GUCEF_TRACE;

    char buffer[ MaxTimestampTextLength + 1 ];
    if ( 0 == FormatTimestamp( ticks, unit, utcSuffix, buffer, sizeof( buffer ) ) )
        return CString::Empty;
    return CString( buffer );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
