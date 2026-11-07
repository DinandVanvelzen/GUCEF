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

#ifndef GUCEF_CORE_CDATE_H
#define GUCEF_CORE_CDATE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CIDATE_H
#include "gucefCORE_CIDate.h"
#define GUCEF_CORE_CIDATE_H
#endif /* GUCEF_CORE_CIDATE_H ? */

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

class GUCEF_CORE_PUBLIC_CPP CDate : public CIDate
{
    public:

    enum EDateMonths
    {
        DATEMONTHS_UNKNOWN = 0,

        DATEMONTHS_JANUARY,
        DATEMONTHS_FEBRUARY,
        DATEMONTHS_MARCH,
        DATEMONTHS_APRIL,
        DATEMONTHS_MAY,
        DATEMONTHS_JUNE,
        DATEMONTHS_JULY,
        DATEMONTHS_AUGUST,
        DATEMONTHS_SEPTEMBER,
        DATEMONTHS_OCTOBER,
        DATEMONTHS_NOVEMBER,
        DATEMONTHS_DECEMBER
    };
    typedef enum EDateMonths TDateMonths;

    static CDate TodaysLocalDate( void );

    static CDate TodaysUTCDate( void );

    static CDate UnixEpochDate( void );

    bool SetYear( Int16 year );

    virtual Int16 GetYear( void ) const GUCEF_VIRTUAL_OVERRIDE;

    bool SetMonth( UInt8 month );
    
    virtual UInt8 GetMonth( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetMonthName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    bool SetDay( UInt8 day );
    
    virtual UInt8 GetDay( void ) const GUCEF_VIRTUAL_OVERRIDE;

    bool IsLeapYear( void ) const;

    UInt8 GetDaysInMonth( void ) const;

    /**
     *  Returns the day of week for this date.
     *  0 = Sunday, 1 = Monday, ..., 6 = Saturday
     *  Uses the Tomohiko Sakamoto algorithm.
     */
    UInt8 GetDayOfWeek( void ) const;

    CDate& operator=( const CDate& src );

    bool operator==( const CDate& src ) const;

    bool operator!=( const CDate& src ) const;

    bool operator<( const CDate& src ) const;

    bool operator<=( const CDate& src ) const;

    bool operator>( const CDate& src ) const;

    bool operator>=( const CDate& src ) const;

    /**
     *  Returns true if the year, month and day describe an existing day of the
     *  proleptic Gregorian calendar.
     */
    bool IsValid( void ) const;

    /**
     *  Number of days since 1970-01-01 (negative before that date).
     *  Only meaningful for valid dates, see IsValid().
     */
    Int32 ToDaysSinceUnixEpoch( void ) const;

    /**
     *  Sets this date to the day that is the given number of days since 1970-01-01.
     *  Returns false and leaves the date unchanged if the result does not fit the year range.
     */
    bool FromDaysSinceUnixEpoch( Int32 daysSinceUnixEpoch );

    /**
     *  Number of days from this date to the given date, positive if the given date is later.
     *  Only meaningful for valid dates.
     */
    Int32 GetDaysTowards( const CDate& other ) const;

    /**
     *  Calendar arithmetic. Returns false and leaves the date unchanged if this date is not valid
     *  or the result does not fit the year range.
     */
    bool AddDays( Int32 daysToAdd );
    bool AddWeeks( Int32 weeksToAdd );

    /**
     *  Adds calendar months. The day of month is clamped to the last day of the resulting month,
     *  so January 31st plus one month is the last day of February.
     */
    bool AddMonths( Int32 monthsToAdd );

    /**
     *  Adds calendar years. February 29th becomes February 28th in a non leap result year.
     */
    bool AddYears( Int32 yearsToAdd );

    CDate( void );
    CDate( const time_t& src, bool isUtc );
    CDate( UInt16 year, UInt8 month, UInt8 day );
    CDate( const CDate& src );
    virtual ~CDate();

    virtual CString ToIso8601DateString( bool includeDelimeters ) const;
    
    protected:

    Int16 m_year;
    UInt8 m_month;
    UInt8 m_day;
};

/*-------------------------------------------------------------------------*/

inline CString ToString( const CDate& value ) { return value.ToIso8601DateString( true ); }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATE_H ? */
