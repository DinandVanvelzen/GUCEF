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

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#define GUCEF_CORE_CDATATABLEVALUES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DATATABLETYPES_H
#include "gucefCORE_DataTableTypes.h"
#define GUCEF_CORE_DATATABLETYPES_H
#endif /* GUCEF_CORE_DATATABLETYPES_H ? */

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CDate;
class CDateTime;
class CDecimal;

/**
 *  Literal builders and the date and tick conversions of the data table layer.
 *
 *  Dates are signed Int32 day counts since 1970-01-01 in the proleptic Gregorian calendar, timestamps are signed Int64
 *  tick counts since the unix epoch (UTC) in a time unit. Conversions between them are done in integer arithmetic,
 *  so dates and times before 1970 are as exact as later ones. The text forms are ISO 8601: dates as yyyy-mm-dd and
 *  timestamps as yyyy-mm-ddThh:mm:ss with exactly the fraction digits of the unit (3, 6 or 9). Years outside
 *  0000..9999 are written with a sign and at least 4 digits, as ISO 8601 expanded years.
 *
 *  Stateless: every member is static and every member is reentrant.
 */
class GUCEF_CORE_PUBLIC_CPP CDataTableValues
{
    public:

    /** Longest text FormatDate writes, without the terminating zero: "+5881580-07-11" */
    static const UInt32 MaxDateTextLength = 14;

    /** Longest text FormatTimestamp writes, without the terminating zero: a signed 9 digit year with 9 fraction digits and 'Z' */
    static const UInt32 MaxTimestampTextLength = 40;

    /** A variant holding the date under GUCEF_DATATYPE_DATE_IN_DAYS_SINCE_UNIX_EPOCH, an empty variant for an invalid date */
    static CVariant Date( const CDate& date );

    /** A variant holding the day count under GUCEF_DATATYPE_DATE_IN_DAYS_SINCE_UNIX_EPOCH */
    static CVariant Date( Int32 daysSinceEpoch );

    /** A variant holding the ticks under the timestamp type id of the unit (see TimestampTypeId) */
    static CVariant Timestamp( Int64 ticks, TDataTableTimeUnit unit );

    /** A variant holding the date time, converted to UTC, as ticks of the unit; an empty variant when it does not fit */
    static CVariant Timestamp( const CDateTime& dateTime, TDataTableTimeUnit unit );

    /** A variant holding the decimal under GUCEF_DATATYPE_INT64_UINT8_DECIMAL */
    static CVariant Decimal( const CDecimal& value );

    /** Days since 1970-01-01 of the date, which must be valid */
    static Int32 ToDays( const CDate& date );

    /** The date of the day count, an invalid (default) date when the year does not fit CDate */
    static CDate FromDays( Int32 days );

    /** The native byte order variant type id for timestamps of the unit: the ms, micros or nanos id */
    static UInt8 TimestampTypeId( TDataTableTimeUnit unit );

    /**
     *  The unit of a timestamp variant type id, either byte order. False for other ids and for the seconds id,
     *  which no column unit matches: use TicksFromVariant for those.
     */
    static bool TimeUnitFromTypeId( UInt8 typeId, TDataTableTimeUnit& unit );

    /** 1000, 1000000 or 1000000000 */
    static Int64 TicksPerSecond( TDataTableTimeUnit unit );

    /** Ticks per day of the unit */
    static Int64 TicksPerDay( TDataTableTimeUnit unit );

    /**
     *  Converts ticks between units without loss: to a finer unit when the result fits Int64, to a coarser one only
     *  when no fraction of the coarser tick is dropped. False otherwise, the result is then untouched.
     */
    static bool ConvertTicks( Int64 ticks, TDataTableTimeUnit fromUnit, TDataTableTimeUnit toUnit, Int64& result );

    /**
     *  Converts ticks to a coarser or finer unit; a fraction of the coarser tick is rounded towards negative infinity
     *  (floor) when roundUp is false and towards positive infinity (ceiling) when it is true. False when it does not
     *  fit Int64.
     */
    static bool ConvertTicksRounded( Int64 ticks, TDataTableTimeUnit fromUnit, TDataTableTimeUnit toUnit, bool roundUp, Int64& result );

    /** Ticks of the unit for the date time converted to UTC. False when the date is invalid or the result does not fit */
    static bool TicksFromDateTime( const CDateTime& dateTime, TDataTableTimeUnit unit, Int64& ticks );

    /** The UTC date time of the ticks, truncated to milliseconds (floor). False when the year does not fit CDate */
    static bool DateTimeFromTicks( Int64 ticks, TDataTableTimeUnit unit, CDateTime& dateTime );

    /**
     *  Ticks of the unit for a variant holding a timestamp of any unit (seconds included) or a date (midnight UTC).
     *  Exact only: false for other types, for a fraction of a tick of the unit and for results that do not fit.
     */
    static bool TicksFromVariant( const CVariant& value, TDataTableTimeUnit unit, Int64& ticks );

    /** Splits a day count into year, month (1..12) and day (1..31), for any Int32 day count */
    static void CivilFromDays( Int32 days, Int32& year, UInt32& month, UInt32& day );

    /** The day count of the given year, month (1..12) and day (1..31); false for a month or day out of range */
    static bool DaysFromCivil( Int32 year, UInt32 month, UInt32 day, Int32& days );

    /**
     *  Writes the date as yyyy-mm-dd and a terminating zero. Returns the length without the terminator,
     *  0 when the buffer is smaller than MaxDateTextLength + 1.
     */
    static UInt32 FormatDate( Int32 days, char* buffer, UInt32 bufferSize );

    /** Parses yyyy-mm-dd (the year may carry a sign and more than 4 digits). Strict: the whole text must match */
    static bool ParseDate( const char* text, UInt32 length, Int32& days );

    /**
     *  Writes the timestamp as yyyy-mm-ddThh:mm:ss.f with the fraction digits of the unit, then 'Z' when utcSuffix
     *  is set, and a terminating zero. Returns the length without the terminator, 0 when the buffer is smaller than
     *  MaxTimestampTextLength + 1.
     */
    static UInt32 FormatTimestamp( Int64 ticks, TDataTableTimeUnit unit, bool utcSuffix, char* buffer, UInt32 bufferSize );

    /**
     *  Parses an ISO 8601 date time: yyyy-mm-dd, then 'T' or a space, hh:mm, optionally :ss and a fraction of up to
     *  9 digits, then optionally 'Z' or an offset +hh:mm / -hh:mm which is applied to give UTC ticks. A text
     *  without a zone is taken as UTC. A bare yyyy-mm-dd is midnight. Digits of the fraction finer than the unit must be
     *  zero. Strict: the whole text must match and the result must fit Int64.
     */
    static bool ParseTimestamp( const char* text, UInt32 length, TDataTableTimeUnit unit, Int64& ticks );

    /** FormatDate into a string */
    static CString DateToString( Int32 days );

    /** FormatTimestamp into a string */
    static CString TimestampToString( Int64 ticks, TDataTableTimeUnit unit, bool utcSuffix = true );

    private:

    CDataTableValues( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */
