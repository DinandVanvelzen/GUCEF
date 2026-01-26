/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <sstream>
#include <algorithm>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDATETIMERANGE_H
#include "gucefCORE_CDateTimeRange.h"
#define GUCEF_CORE_CDATETIMERANGE_H
#endif /* GUCEF_CORE_CDATETIMERANGE_H ? */

#include "gucefCORE_CCronSchedule.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CCronSchedule CCronSchedule::Empty;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CCronSchedule::CCronSchedule( void )
    : m_minutes()
    , m_hours()
    , m_daysOfMonth()
    , m_months()
    , m_daysOfWeek()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCronSchedule::CCronSchedule( const CString& cronExpression )
    : m_minutes()
    , m_hours()
    , m_daysOfMonth()
    , m_months()
    , m_daysOfWeek()
{GUCEF_TRACE;

    FromCronString( cronExpression );
}

/*-------------------------------------------------------------------------*/

CCronSchedule::CCronSchedule( const CCronSchedule& src )
    : m_minutes( src.m_minutes )
    , m_hours( src.m_hours )
    , m_daysOfMonth( src.m_daysOfMonth )
    , m_months( src.m_months )
    , m_daysOfWeek( src.m_daysOfWeek )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCronSchedule::~CCronSchedule()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCronSchedule&
CCronSchedule::operator=( const CCronSchedule& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_minutes = src.m_minutes;
        m_hours = src.m_hours;
        m_daysOfMonth = src.m_daysOfMonth;
        m_months = src.m_months;
        m_daysOfWeek = src.m_daysOfWeek;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CCronSchedule::operator==( const CCronSchedule& other ) const
{GUCEF_TRACE;

    return m_minutes == other.m_minutes &&
           m_hours == other.m_hours &&
           m_daysOfMonth == other.m_daysOfMonth &&
           m_months == other.m_months &&
           m_daysOfWeek == other.m_daysOfWeek;
}

/*-------------------------------------------------------------------------*/

bool
CCronSchedule::operator!=( const CCronSchedule& other ) const
{GUCEF_TRACE;

    return !(*this == other);
}

/*-------------------------------------------------------------------------*/

bool
CCronSchedule::ParseCronField( const CString& field, UInt8 minValue, UInt8 maxValue, TUInt8Set& values )
{GUCEF_TRACE;

    values.clear();

    if ( field.IsNULLOrEmpty() )
        return false;

    CString trimmedField = field.Trim( true );

    // Handle wildcard
    if ( trimmedField == "*" )
    {
        for ( UInt8 i = minValue; i <= maxValue; ++i )
            values.insert( i );
        return true;
    }

    // Split by comma for lists
    CString::StringVector parts = trimmedField.ParseElements( ',', false );
    for ( size_t i = 0; i < parts.size(); ++i )
    {
        CString part = parts[ i ].Trim( true );

        // Check for step values (e.g., */5 or 10-20/2)
        Int32 slashPos = part.HasChar( '/', true );
        UInt8 step = 1;
        CString rangePart = part;

        if ( slashPos >= 0 )
        {
            CString stepStr = part.SubstrFromRange( slashPos + 1, part.Length() - 1 );
            Int32 stepValue = CORE::StringToInt32( stepStr, 1 );
            if ( stepValue < 1 )
                return false;
            step = (UInt8) stepValue;
            rangePart = part.SubstrFromRange( 0, slashPos - 1 );
        }

        // Check for range (e.g., 10-20)
        Int32 dashPos = rangePart.HasChar( '-', true );
        if ( dashPos >= 0 )
        {
            CString startStr = rangePart.SubstrFromRange( 0, dashPos - 1 );
            CString endStr = rangePart.SubstrFromRange( dashPos + 1, rangePart.Length() - 1 );

            Int32 startVal = CORE::StringToInt32( startStr, -1 );
            Int32 endVal = CORE::StringToInt32( endStr, -1 );

            if ( startVal < minValue || startVal > maxValue || 
                 endVal < minValue || endVal > maxValue ||
                 startVal > endVal )
                return false;

            for ( Int32 v = startVal; v <= endVal; v += step )
                values.insert( (UInt8) v );
        }
        else if ( rangePart == "*" )
        {
            // Wildcard with step (e.g., */5)
            for ( UInt8 v = minValue; v <= maxValue; v += step )
                values.insert( v );
        }
        else
        {
            // Single value
            Int32 val = CORE::StringToInt32( rangePart, -1 );
            if ( val < minValue || val > maxValue )
                return false;
            values.insert( (UInt8) val );
        }
    }

    return !values.empty();
}

/*-------------------------------------------------------------------------*/

bool
CCronSchedule::FromCronString( const CString& cronExpression )
{GUCEF_TRACE;

    Clear();

    if ( cronExpression.IsNULLOrEmpty() )
        return false;

    // Parse: minute hour day-of-month month day-of-week
    CString::StringVector fields = cronExpression.ParseElements( ' ', false );
    if ( fields.size() != 5 )
        return false;

    // Minute (0-59)
    if ( !ParseCronField( fields[ 0 ], 0, 59, m_minutes ) )
        return false;

    // Hour (0-23)
    if ( !ParseCronField( fields[ 1 ], 0, 23, m_hours ) )
        return false;

    // Day of month (1-31)
    if ( !ParseCronField( fields[ 2 ], 1, 31, m_daysOfMonth ) )
        return false;

    // Month (1-12)
    if ( !ParseCronField( fields[ 3 ], 1, 12, m_months ) )
        return false;

    // Day of week (0-6, Sunday=0)
    if ( !ParseCronField( fields[ 4 ], 0, 6, m_daysOfWeek ) )
        return false;

    return true;
}

/*-------------------------------------------------------------------------*/

CString
CCronSchedule::SetToCronField( const TUInt8Set& values, UInt8 minValue, UInt8 maxValue ) const
{GUCEF_TRACE;

    if ( values.empty() )
        return "*";

    // Check if all values are present (wildcard)
    if ( values.size() == (size_t)( maxValue - minValue + 1 ) )
    {
        bool isComplete = true;
        for ( UInt8 i = minValue; i <= maxValue; ++i )
        {
            if ( values.find( i ) == values.end() )
            {
                isComplete = false;
                break;
            }
        }
        if ( isComplete )
            return "*";
    }

    // Build comma-separated list or ranges
    CString result;
    TUInt8Set::const_iterator it = values.begin();
    UInt8 rangeStart = *it;
    UInt8 rangeLast = *it;
    ++it;

    for ( ; it != values.end(); ++it )
    {
        UInt8 current = *it;
        if ( current == rangeLast + 1 )
        {
            rangeLast = current;
        }
        else
        {
            // Close previous range
            if ( !result.IsNULLOrEmpty() )
                result += ",";

            if ( rangeStart == rangeLast )
                result += CORE::UInt8ToString( rangeStart );
            else if ( rangeStart + 1 == rangeLast )
                result += CORE::UInt8ToString( rangeStart ) + "," + CORE::UInt8ToString( rangeLast );
            else
                result += CORE::UInt8ToString( rangeStart ) + "-" + CORE::UInt8ToString( rangeLast );

            rangeStart = current;
            rangeLast = current;
        }
    }

    // Close final range
    if ( !result.IsNULLOrEmpty() )
        result += ",";

    if ( rangeStart == rangeLast )
        result += CORE::UInt8ToString( rangeStart );
    else if ( rangeStart + 1 == rangeLast )
        result += CORE::UInt8ToString( rangeStart ) + "," + CORE::UInt8ToString( rangeLast );
    else
        result += CORE::UInt8ToString( rangeStart ) + "-" + CORE::UInt8ToString( rangeLast );

    return result;
}

/*-------------------------------------------------------------------------*/

CString
CCronSchedule::ToCronString( void ) const
{GUCEF_TRACE;

    CString result;
    result += SetToCronField( m_minutes, 0, 59 );
    result += " ";
    result += SetToCronField( m_hours, 0, 23 );
    result += " ";
    result += SetToCronField( m_daysOfMonth, 1, 31 );
    result += " ";
    result += SetToCronField( m_months, 1, 12 );
    result += " ";
    result += SetToCronField( m_daysOfWeek, 0, 6 );

    return result;
}

/*-------------------------------------------------------------------------*/

bool
CCronSchedule::MatchesDayConstraints( const CDateTime& dt ) const
{GUCEF_TRACE;

    UInt8 dayOfMonth = dt.GetDay();
    UInt8 month = dt.GetMonth();
    
    // Get day of week (0=Sunday)
    CDate dateOnly( dt.GetYear(), dt.GetMonth(), dt.GetDay() );
    UInt8 dayOfWeek = (UInt8) dateOnly.GetDayOfWeek();

    bool dayOfMonthMatches = m_daysOfMonth.find( dayOfMonth ) != m_daysOfMonth.end();
    bool dayOfWeekMatches = m_daysOfWeek.find( dayOfWeek ) != m_daysOfWeek.end();

    // In cron, day-of-month and day-of-week are OR'd together
    // If both are specified (not *), either can match
    bool bothSpecified = m_daysOfMonth.size() < 31 && m_daysOfWeek.size() < 7;
    
    if ( bothSpecified )
        return dayOfMonthMatches || dayOfWeekMatches;
    else
        return dayOfMonthMatches && dayOfWeekMatches;
}

/*-------------------------------------------------------------------------*/

bool
CCronSchedule::Matches( const CDateTime& dt ) const
{GUCEF_TRACE;

    if ( m_minutes.empty() || m_hours.empty() || m_daysOfMonth.empty() || 
         m_months.empty() || m_daysOfWeek.empty() )
        return false;

    UInt8 minute = (UInt8) dt.GetMinutes();
    UInt8 hour = (UInt8) dt.GetHours();
    UInt8 month = dt.GetMonth();

    if ( m_minutes.find( minute ) == m_minutes.end() )
        return false;

    if ( m_hours.find( hour ) == m_hours.end() )
        return false;

    if ( m_months.find( month ) == m_months.end() )
        return false;

    return MatchesDayConstraints( dt );
}

/*-------------------------------------------------------------------------*/

UInt8
CCronSchedule::GetNextValue( const TUInt8Set& values, UInt8 current, UInt8 maxValue, bool& wrapped ) const
{GUCEF_TRACE;

    wrapped = false;

    if ( values.empty() )
        return current;

    // Find the first value >= current
    TUInt8Set::const_iterator it = values.lower_bound( current );
    if ( it != values.end() && *it >= current )
        return *it;

    // Wrap around
    wrapped = true;
    return *values.begin();
}

/*-------------------------------------------------------------------------*/

UInt8
CCronSchedule::GetPreviousValue( const TUInt8Set& values, UInt8 current, UInt8 maxValue, bool& wrapped ) const
{GUCEF_TRACE;

    wrapped = false;

    if ( values.empty() )
        return current;

    // Find the first value <= current
    TUInt8Set::const_iterator it = values.upper_bound( current );
    if ( it != values.begin() )
    {
        --it;
        return *it;
    }

    // Wrap around
    wrapped = true;
    return *values.rbegin();
}

/*-------------------------------------------------------------------------*/

void
CCronSchedule::IncrementToNextMinute( CDateTime& dt ) const
{GUCEF_TRACE;

    dt.AddMinutes( 1 );
    dt.SetSeconds( 0 );
    dt.SetMilliseconds( 0 );
}

/*-------------------------------------------------------------------------*/

void
CCronSchedule::IncrementToNextHour( CDateTime& dt ) const
{GUCEF_TRACE;

    dt.AddHours( 1 );
    dt.SetMinutes( 0 );
    dt.SetSeconds( 0 );
    dt.SetMilliseconds( 0 );
}

/*-------------------------------------------------------------------------*/

void
CCronSchedule::IncrementToNextDay( CDateTime& dt ) const
{GUCEF_TRACE;

    dt.AddDays( 1 );
    dt.SetHours( 0 );
    dt.SetMinutes( 0 );
    dt.SetSeconds( 0 );
    dt.SetMilliseconds( 0 );
}

/*-------------------------------------------------------------------------*/

void
CCronSchedule::IncrementToNextMonth( CDateTime& dt ) const
{GUCEF_TRACE;

    UInt8 month = dt.GetMonth();
    Int16 year = dt.GetYear();

    if ( month == 12 )
    {
        month = 1;
        ++year;
    }
    else
    {
        ++month;
    }

    dt.Set( year, month, 1, 0, 0, 0, 0, dt.GetTimeZoneUTCOffsetInMins() );
}

/*-------------------------------------------------------------------------*/

void
CCronSchedule::DecrementToPreviousMinute( CDateTime& dt ) const
{GUCEF_TRACE;

    dt.AddMinutes( -1 );
    dt.SetSeconds( 0 );
    dt.SetMilliseconds( 0 );
}

/*-------------------------------------------------------------------------*/

void
CCronSchedule::DecrementToPreviousHour( CDateTime& dt ) const
{GUCEF_TRACE;

    dt.AddHours( -1 );
    dt.SetMinutes( 59 );
    dt.SetSeconds( 0 );
    dt.SetMilliseconds( 0 );
}

/*-------------------------------------------------------------------------*/

void
CCronSchedule::DecrementToPreviousDay( CDateTime& dt ) const
{GUCEF_TRACE;

    dt.AddDays( -1 );
    dt.SetHours( 23 );
    dt.SetMinutes( 59 );
    dt.SetSeconds( 0 );
    dt.SetMilliseconds( 0 );
}

/*-------------------------------------------------------------------------*/

void
CCronSchedule::DecrementToPreviousMonth( CDateTime& dt ) const
{GUCEF_TRACE;

    UInt8 month = dt.GetMonth();
    Int16 year = dt.GetYear();

    if ( month == 1 )
    {
        month = 12;
        --year;
    }
    else
    {
        --month;
    }

    // Get the last day of the previous month
    CDate tempDate( year, month, 1 );
    UInt8 lastDay = tempDate.GetDaysInMonth();

    dt.Set( year, month, lastDay, 23, 59, 0, 0, dt.GetTimeZoneUTCOffsetInMins() );
}

/*-------------------------------------------------------------------------*/

bool
CCronSchedule::GetNextOccurrence( const CDateTime& from, CDateTime& next ) const
{GUCEF_TRACE;

    if ( !IsValid() )
        return false;

    // Start from the next minute
    next = from;
    next.SetSeconds( 0 );
    next.SetMilliseconds( 0 );
    IncrementToNextMinute( next );

    // Maximum iterations to prevent infinite loops
    const Int32 maxIterations = 366 * 24 * 60; // One year worth of minutes
    Int32 iterations = 0;

    while ( iterations < maxIterations )
    {
        ++iterations;

        UInt8 minute = (UInt8) next.GetMinutes();
        UInt8 hour = (UInt8) next.GetHours();
        UInt8 day = next.GetDay();
        UInt8 month = next.GetMonth();

        // Check month
        if ( m_months.find( month ) == m_months.end() )
        {
            bool wrapped = false;
            UInt8 nextMonth = GetNextValue( m_months, month + 1, 12, wrapped );
            if ( wrapped )
            {
                next.Set( next.GetYear() + 1, nextMonth, 1, 0, 0, 0, 0, next.GetTimeZoneUTCOffsetInMins() );
            }
            else
            {
                next.Set( next.GetYear(), nextMonth, 1, 0, 0, 0, 0, next.GetTimeZoneUTCOffsetInMins() );
            }
            continue;
        }

        // Check day constraints
        if ( !MatchesDayConstraints( next ) )
        {
            IncrementToNextDay( next );
            continue;
        }

        // Check hour
        if ( m_hours.find( hour ) == m_hours.end() )
        {
            bool wrapped = false;
            UInt8 nextHour = GetNextValue( m_hours, hour + 1, 23, wrapped );
            if ( wrapped )
            {
                IncrementToNextDay( next );
            }
            else
            {
                next.SetHours( nextHour );
                next.SetMinutes( 0 );
            }
            continue;
        }

        // Check minute
        if ( m_minutes.find( minute ) == m_minutes.end() )
        {
            bool wrapped = false;
            UInt8 nextMinute = GetNextValue( m_minutes, minute + 1, 59, wrapped );
            if ( wrapped )
            {
                IncrementToNextHour( next );
            }
            else
            {
                next.SetMinutes( nextMinute );
            }
            continue;
        }

        // All constraints matched
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CCronSchedule::GetNextOccurrence( const CDateTime& from, const CDateTimeRange& validWindow, CDateTime& next ) const
{GUCEF_TRACE;

    if ( !IsValid() )
        return false;

    CDateTime candidate = from;
    const Int32 maxIterations = 366 * 24 * 60; // One year worth of minutes
    Int32 iterations = 0;

    while ( iterations < maxIterations )
    {
        ++iterations;

        // Get the next occurrence without range constraint
        if ( !GetNextOccurrence( candidate, next ) )
            return false;

        // Check if it's within the valid window
        if ( validWindow == next )
            return true;

        // If the next occurrence is beyond the window, we're done
        if ( next > validWindow.GetEnd() )
            return false;

        // Continue searching from this occurrence
        candidate = next;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CCronSchedule::GetNextOccurrences( const CDateTime& from, UInt32 maxOccurrences, TDateTimeVector& occurrences ) const
{GUCEF_TRACE;

    occurrences.clear();

    if ( !IsValid() || maxOccurrences == 0 )
        return 0;

    CDateTime current = from;
    CDateTime next;

    for ( UInt32 i = 0; i < maxOccurrences; ++i )
    {
        if ( !GetNextOccurrence( current, next ) )
            break;

        occurrences.push_back( next );
        current = next;
    }

    return (UInt32) occurrences.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CCronSchedule::GetOccurrencesInRange( const CDateTimeRange& range, UInt32 maxOccurrences, TDateTimeVector& occurrences ) const
{GUCEF_TRACE;

    occurrences.clear();

    if ( !IsValid() || maxOccurrences == 0 )
        return 0;

    CDateTime current = range.GetStart();
    CDateTime next;

    for ( UInt32 i = 0; i < maxOccurrences; ++i )
    {
        if ( !GetNextOccurrence( current, next ) )
            break;

        // Check if next occurrence is beyond the range
        if ( next > range.GetEnd() )
            break;

        occurrences.push_back( next );
        current = next;
    }

    return (UInt32) occurrences.size();
}

/*-------------------------------------------------------------------------*/

UInt64
CCronSchedule::GetMillisecondsUntilNext( void ) const
{GUCEF_TRACE;

    return GetMillisecondsUntilNext( CDateTime::NowUTCDateTime() );
}

/*-------------------------------------------------------------------------*/

UInt64
CCronSchedule::GetMillisecondsUntilNext( const CDateTime& from ) const
{GUCEF_TRACE;

    CDateTime next;
    if ( GetNextOccurrence( from, next ) )
    {
        Int64 diff = from.GetTimeDifferenceInMillisecondsTowards( next );
        if ( diff > 0 )
            return (UInt64) diff;
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

bool
CCronSchedule::IsEveryMinute( void ) const
{GUCEF_TRACE;

    if ( !IsValid() )
        return false;

    // Check if all fields allow all values (wildcard)
    return m_minutes.size() == 60 &&
           m_hours.size() == 24 &&
           m_daysOfMonth.size() == 31 &&
           m_months.size() == 12 &&
           m_daysOfWeek.size() == 7;
}

/*-------------------------------------------------------------------------*/

bool
CCronSchedule::IsEveryHour( void ) const
{GUCEF_TRACE;

    if ( !IsValid() )
        return false;

    // Check if triggers at minute 0 and all other fields are wildcards
    return m_minutes.size() == 1 &&
           m_minutes.find( 0 ) != m_minutes.end() &&
           m_hours.size() == 24 &&
           m_daysOfMonth.size() == 31 &&
           m_months.size() == 12 &&
           m_daysOfWeek.size() == 7;
}

/*-------------------------------------------------------------------------*/

bool
CCronSchedule::IsDaily( void ) const
{GUCEF_TRACE;

    if ( !IsValid() )
        return false;

    // Check if triggers at 00:00 and all other fields are wildcards
    return m_minutes.size() == 1 &&
           m_minutes.find( 0 ) != m_minutes.end() &&
           m_hours.size() == 1 &&
           m_hours.find( 0 ) != m_hours.end() &&
           m_daysOfMonth.size() == 31 &&
           m_months.size() == 12 &&
           m_daysOfWeek.size() == 7;
}

/*-------------------------------------------------------------------------*/

CCronSchedule
CCronSchedule::EveryMinute( void )
{GUCEF_TRACE;

    CCronSchedule schedule;
    schedule.FromCronString( "* * * * *" );
    return schedule;
}

/*-------------------------------------------------------------------------*/

CCronSchedule
CCronSchedule::Hourly( void )
{GUCEF_TRACE;

    CCronSchedule schedule;
    schedule.FromCronString( "0 * * * *" );
    return schedule;
}

/*-------------------------------------------------------------------------*/

CCronSchedule
CCronSchedule::Daily( void )
{GUCEF_TRACE;

    CCronSchedule schedule;
    schedule.FromCronString( "0 0 * * *" );
    return schedule;
}

/*-------------------------------------------------------------------------*/

CCronSchedule
CCronSchedule::Weekly( void )
{GUCEF_TRACE;

    CCronSchedule schedule;
    schedule.FromCronString( "0 0 * * 0" );
    return schedule;
}

/*-------------------------------------------------------------------------*/

CCronSchedule
CCronSchedule::Monthly( void )
{GUCEF_TRACE;

    CCronSchedule schedule;
    schedule.FromCronString( "0 0 1 * *" );
    return schedule;
}

/*-------------------------------------------------------------------------*/

CCronSchedule
CCronSchedule::Yearly( void )
{GUCEF_TRACE;

    CCronSchedule schedule;
    schedule.FromCronString( "0 0 1 1 *" );
    return schedule;
}

/*-------------------------------------------------------------------------*/

CCronSchedule
CCronSchedule::BusinessHours40hrWw( void )
{GUCEF_TRACE;

    CCronSchedule schedule;
    schedule.FromCronString( "0 9-17 * * 1-5" );
    return schedule;
}

/*-------------------------------------------------------------------------*/

void
CCronSchedule::Clear( void )
{GUCEF_TRACE;

    m_minutes.clear();
    m_hours.clear();
    m_daysOfMonth.clear();
    m_months.clear();
    m_daysOfWeek.clear();
}

/*-------------------------------------------------------------------------*/

bool
CCronSchedule::Matches( const CDateTime& dt               ,
                        const CDateTimeRange& validWindow ) const
{GUCEF_TRACE;

    // Check if datetime is within the valid window
    if ( validWindow == dt )
    {
        // Now check if it matches the cron schedule
        return Matches( dt );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CCronSchedule::GetNextOccurrence( const CDateTime& from             ,
                                  const CDateTimeRange& validWindow ,
                                  CDateTime& next                   ) const
{GUCEF_TRACE;

    if ( !IsValid() )
        return false;

    CDateTime candidate = from;
    const Int32 maxIterations = 366 * 24 * 60; // One year worth of minutes
    Int32 iterations = 0;

    while ( iterations < maxIterations )
    {
        ++iterations;

        // Get the next occurrence without range constraint
        if ( !GetNextOccurrence( candidate, next ) )
            return false;

        // Check if it's within the valid window
        if ( validWindow == next )
            return true;

        // If the next occurrence is beyond the window, we're done
        if ( next > validWindow.GetEnd() )
            return false;

        // Continue searching from this occurrence
        candidate = next;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CCronSchedule::GetNextOccurrences( const CDateTime& from, UInt32 maxOccurrences, TDateTimeVector& occurrences ) const
{GUCEF_TRACE;

    occurrences.clear();

    if ( !IsValid() || maxOccurrences == 0 )
        return 0;

    CDateTime current = from;
    CDateTime next;

    for ( UInt32 i = 0; i < maxOccurrences; ++i )
    {
        if ( !GetNextOccurrence( current, next ) )
            break;

        occurrences.push_back( next );
        current = next;
    }

    return (UInt32) occurrences.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CCronSchedule::GetOccurrencesInRange( const CDateTimeRange& range, UInt32 maxOccurrences, TDateTimeVector& occurrences ) const
{GUCEF_TRACE;

    occurrences.clear();

    if ( !IsValid() || maxOccurrences == 0 )
        return 0;

    CDateTime current = range.GetStart();
    CDateTime next;

    for ( UInt32 i = 0; i < maxOccurrences; ++i )
    {
        if ( !GetNextOccurrence( current, next ) )
            break;

        // Check if next occurrence is beyond the range
        if ( next > range.GetEnd() )
            break;

        occurrences.push_back( next );
        current = next;
    }

    return (UInt32) occurrences.size();
}

/*-------------------------------------------------------------------------*/

UInt64
CCronSchedule::GetMillisecondsUntilNext( void ) const
{GUCEF_TRACE;

    return GetMillisecondsUntilNext( CDateTime::NowUTCDateTime() );
}

/*-------------------------------------------------------------------------*/

UInt64
CCronSchedule::GetMillisecondsUntilNext( const CDateTime& from ) const
{GUCEF_TRACE;

    CDateTime next;
    if ( GetNextOccurrence( from, next ) )
    {
        Int64 diff = from.GetTimeDifferenceInMillisecondsTowards( next );
        if ( diff > 0 )
            return (UInt64) diff;
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

bool
CCronSchedule::IsEveryMinute( void ) const
{GUCEF_TRACE;

    if ( !IsValid() )
        return false;

    // Check if all fields allow all values (wildcard)
    return m_minutes.size() == 60 &&
           m_hours.size() == 24 &&
           m_daysOfMonth.size() == 31 &&
           m_months.size() == 12 &&
           m_daysOfWeek.size() == 7;
}

/*-------------------------------------------------------------------------*/

bool
CCronSchedule::IsEveryHour( void ) const
{GUCEF_TRACE;

    if ( !IsValid() )
        return false;

    // Check if triggers at minute 0 and all other fields are wildcards
    return m_minutes.size() == 1 &&
           m_minutes.find( 0 ) != m_minutes.end() &&
           m_hours.size() == 24 &&
           m_daysOfMonth.size() == 31 &&
           m_months.size() == 12 &&
           m_daysOfWeek.size() == 7;
}

/*-------------------------------------------------------------------------*/

bool
CCronSchedule::IsDaily( void ) const
{GUCEF_TRACE;

    if ( !IsValid() )
        return false;

    // Check if triggers at 00:00 and all other fields are wildcards
    return m_minutes.size() == 1 &&
           m_minutes.find( 0 ) != m_minutes.end() &&
           m_hours.size() == 1 &&
           m_hours.find( 0 ) != m_hours.end() &&
           m_daysOfMonth.size() == 31 &&
           m_months.size() == 12 &&
           m_daysOfWeek.size() == 7;
}

/*-------------------------------------------------------------------------*/

UInt64
CCronSchedule::GetApproximateIntervalInMilliSecs( void ) const
{GUCEF_TRACE;

    if ( !IsValid() )
        return 0;

    // Calculate based on schedule type
    if ( IsEveryMinute() )
        return 60 * 1000; // 1 minute

    if ( IsEveryHour() )
        return 60 * 60 * 1000; // 1 hour

    if ( IsDaily() )
        return 24 * 60 * 60 * 1000; // 1 day

    // For more complex schedules, try to determine the minimum interval
    if ( m_minutes.size() == 1 && m_hours.size() == 24 && 
         m_daysOfMonth.size() == 31 && m_months.size() == 12 )
    {
        // Hourly at specific minute
        return 60 * 60 * 1000;
    }

    if ( m_minutes.size() > 1 && m_hours.size() == 24 && 
         m_daysOfMonth.size() == 31 && m_months.size() == 12 )
    {
        // Multiple times per hour
        // Calculate interval between minutes
        TUInt8Set::const_iterator it1 = m_minutes.begin();
        TUInt8Set::const_iterator it2 = it1;
        ++it2;
        
        if ( it2 != m_minutes.end() )
        {
            UInt8 interval = *it2 - *it1;
            return interval * 60 * 1000;
        }
    }

    // Variable interval, cannot determine
    return 0;
}

/*-------------------------------------------------------------------------*/

CString
CCronSchedule::GetDescription( void ) const
{GUCEF_TRACE;

    if ( !IsValid() )
        return "Invalid schedule";

    // Check for common patterns
    if ( IsEveryMinute() )
        return "Every minute";

    if ( IsEveryHour() )
        return "Every hour";

    if ( IsDaily() )
        return "Daily at midnight";

    // Check for hourly at specific minute
    if ( m_minutes.size() == 1 && m_hours.size() == 24 && 
         m_daysOfMonth.size() == 31 && m_months.size() == 12 )
    {
        UInt8 minute = *m_minutes.begin();
        return "Every hour at minute " + CORE::UInt8ToString( minute );
    }

    // Check for specific time daily
    if ( m_minutes.size() == 1 && m_hours.size() == 1 && 
         m_daysOfMonth.size() == 31 && m_months.size() == 12 )
    {
        UInt8 minute = *m_minutes.begin();
        UInt8 hour = *m_hours.begin();
        return "Daily at " + CORE::UInt8ToString( hour ) + ":" + 
               CORE::UInt8ToString( minute ).PadLeft( 2, '0' );
    }

    // Check for weekly
    if ( m_minutes.size() == 1 && m_hours.size() == 1 && 
         m_daysOfMonth.size() == 31 && m_months.size() == 12 &&
         m_daysOfWeek.size() == 1 )
    {
        UInt8 minute = *m_minutes.begin();
        UInt8 hour = *m_hours.begin();
        UInt8 dayOfWeek = *m_daysOfWeek.begin();
        
        const char* dayNames[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
        CString dayName = dayOfWeek < 7 ? dayNames[dayOfWeek] : "Unknown";
        
        return "Weekly on " + dayName + " at " + CORE::UInt8ToString( hour ) + ":" + 
               CORE::UInt8ToString( minute ).PadLeft( 2, '0' );
    }

    // Check for monthly
    if ( m_minutes.size() == 1 && m_hours.size() == 1 && 
         m_daysOfMonth.size() == 1 && m_months.size() == 12 )
    {
        UInt8 minute = *m_minutes.begin();
        UInt8 hour = *m_hours.begin();
        UInt8 day = *m_daysOfMonth.begin();
        
        return "Monthly on day " + CORE::UInt8ToString( day ) + " at " + 
               CORE::UInt8ToString( hour ) + ":" + CORE::UInt8ToString( minute ).PadLeft( 2, '0' );
    }

    // Check for interval patterns (e.g., every 15 minutes)
    if ( m_minutes.size() > 1 && m_hours.size() == 24 && 
         m_daysOfMonth.size() == 31 && m_months.size() == 12 )
    {
        // Check if it's a regular interval
        TUInt8Set::const_iterator it1 = m_minutes.begin();
        TUInt8Set::const_iterator it2 = it1;
        ++it2;
        
        if ( it2 != m_minutes.end() )
        {
            UInt8 interval = *it2 - *it1;
            bool isRegularInterval = true;
            
            TUInt8Set::const_iterator prev = it1;
            for ( TUInt8Set::const_iterator curr = it2; curr != m_minutes.end(); ++curr )
            {
                if ( *curr - *prev != interval )
                {
                    isRegularInterval = false;
                    break;
                }
                prev = curr;
            }
            
            if ( isRegularInterval )
                return "Every " + CORE::UInt8ToString( interval ) + " minutes";
        }
    }

    // For complex schedules, return the cron expression
    return "Custom schedule: " + ToCronString();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
