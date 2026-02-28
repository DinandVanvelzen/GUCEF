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

#include "gucefCORE_CSchedule.h"

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

const CSchedule CSchedule::Empty;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSchedule::CSchedule( void )
    : m_cronSchedule()
    , m_timeRange()
    , m_hasCronSchedule( false )
    , m_hasTimeRange( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSchedule::CSchedule( const CCronSchedule& cronSchedule )
    : m_cronSchedule( cronSchedule )
    , m_timeRange()
    , m_hasCronSchedule( true )
    , m_hasTimeRange( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSchedule::CSchedule( const CDateTimeRange& timeRange )
    : m_cronSchedule()
    , m_timeRange( timeRange )
    , m_hasCronSchedule( false )
    , m_hasTimeRange( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSchedule::CSchedule( const CCronSchedule& cronSchedule, const CDateTimeRange& timeRange )
    : m_cronSchedule( cronSchedule )
    , m_timeRange( timeRange )
    , m_hasCronSchedule( true )
    , m_hasTimeRange( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSchedule::CSchedule( const CSchedule& src )
    : m_cronSchedule( src.m_cronSchedule )
    , m_timeRange( src.m_timeRange )
    , m_hasCronSchedule( src.m_hasCronSchedule )
    , m_hasTimeRange( src.m_hasTimeRange )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSchedule::~CSchedule()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSchedule&
CSchedule::operator=( const CSchedule& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_cronSchedule = src.m_cronSchedule;
        m_timeRange = src.m_timeRange;
        m_hasCronSchedule = src.m_hasCronSchedule;
        m_hasTimeRange = src.m_hasTimeRange;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CSchedule::operator==( const CSchedule& other ) const
{GUCEF_TRACE;

    if ( m_hasCronSchedule != other.m_hasCronSchedule || 
         m_hasTimeRange != other.m_hasTimeRange )
        return false;

    if ( m_hasCronSchedule && m_cronSchedule != other.m_cronSchedule )
        return false;

    if ( m_hasTimeRange && m_timeRange != other.m_timeRange )
        return false;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSchedule::operator!=( const CSchedule& other ) const
{GUCEF_TRACE;

    return !(*this == other);
}

/*-------------------------------------------------------------------------*/

bool
CSchedule::operator<( const CSchedule& other ) const
{GUCEF_TRACE;

    // Compare based on time range start time primarily
    if ( m_hasTimeRange && other.m_hasTimeRange )
    {
        if ( m_timeRange < other.m_timeRange )
            return true;
        if ( m_timeRange > other.m_timeRange )
            return false;
    }
    else if ( !m_hasTimeRange && other.m_hasTimeRange )
    {
        // Schedules without time range sort before those with time range
        return true;
    }
    else if ( m_hasTimeRange && !other.m_hasTimeRange )
    {
        return false;
    }

    // If time ranges are equal or both don't have ranges, compare by cron schedule
    // This is less meaningful but provides consistent ordering
    if ( m_hasCronSchedule && other.m_hasCronSchedule )
    {
        // Compare cron strings for consistent ordering
        return m_cronSchedule.ToCronString() < other.m_cronSchedule.ToCronString();
    }

    return !m_hasCronSchedule && other.m_hasCronSchedule;
}

/*-------------------------------------------------------------------------*/

bool
CSchedule::operator<=( const CSchedule& other ) const
{GUCEF_TRACE;

    return (*this) < other || (*this) == other;
}

/*-------------------------------------------------------------------------*/

bool
CSchedule::operator>( const CSchedule& other ) const
{GUCEF_TRACE;

    return other < (*this);
}

/*-------------------------------------------------------------------------*/

bool
CSchedule::operator>=( const CSchedule& other ) const
{GUCEF_TRACE;

    return (*this) > other || (*this) == other;
}

/*-------------------------------------------------------------------------*/

void
CSchedule::SetCronSchedule( const CCronSchedule& cronSchedule )
{GUCEF_TRACE;

    m_cronSchedule = cronSchedule;
    m_hasCronSchedule = true;
}

/*-------------------------------------------------------------------------*/

const CCronSchedule&
CSchedule::GetCronSchedule( void ) const
{GUCEF_TRACE;

    return m_cronSchedule;
}

/*-------------------------------------------------------------------------*/

CCronSchedule&
CSchedule::GetCronSchedule( void )
{GUCEF_TRACE;

    return m_cronSchedule;
}

/*-------------------------------------------------------------------------*/

void
CSchedule::SetTimeRange( const CDateTimeRange& timeRange )
{GUCEF_TRACE;

    m_timeRange = timeRange;
    m_hasTimeRange = true;
}

/*-------------------------------------------------------------------------*/

const CDateTimeRange&
CSchedule::GetTimeRange( void ) const
{GUCEF_TRACE;

    return m_timeRange;
}

/*-------------------------------------------------------------------------*/

CDateTimeRange&
CSchedule::GetTimeRange( void )
{GUCEF_TRACE;

    return m_timeRange;
}

/*-------------------------------------------------------------------------*/

bool
CSchedule::HasCronSchedule( void ) const
{GUCEF_TRACE;

    return m_hasCronSchedule;
}

/*-------------------------------------------------------------------------*/

bool
CSchedule::HasTimeRange( void ) const
{GUCEF_TRACE;

    return m_hasTimeRange;
}

/*-------------------------------------------------------------------------*/

bool
CSchedule::IsActive( void ) const
{GUCEF_TRACE;

    return IsActive( CDateTime::NowUTCDateTime() );
}

/*-------------------------------------------------------------------------*/

bool
CSchedule::IsActive( const CDateTime& dt ) const
{GUCEF_TRACE;

    if ( !m_hasTimeRange )
        return true; // No time range restriction, always active

    // Check if current time is within the time range [start, end] inclusive
    return !( m_timeRange > dt ) && !( m_timeRange < dt );
}

/*-------------------------------------------------------------------------*/

bool
CSchedule::HasStarted( void ) const
{GUCEF_TRACE;

    if ( !m_hasTimeRange )
        return true; // No time range, consider it started

    return m_timeRange.HasRangeBegun();
}

/*-------------------------------------------------------------------------*/

bool
CSchedule::HasEnded( void ) const
{GUCEF_TRACE;

    if ( !m_hasTimeRange )
        return false; // No time range, never ends

    return m_timeRange.HasRangeEnded();
}

/*-------------------------------------------------------------------------*/

bool
CSchedule::Matches( const CDateTime& dt ) const
{GUCEF_TRACE;

    // Check if within time range (if specified)
    if ( m_hasTimeRange && !IsActive( dt ) )
        return false;

    // Check if matches cron pattern (if specified)
    if ( m_hasCronSchedule )
        return m_cronSchedule.Matches( dt );

    // If only time range is specified, check if we're at the start
    if ( m_hasTimeRange )
        return dt == m_timeRange.GetStart();

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CSchedule::GetNextOccurrence( CDateTime& next ) const
{GUCEF_TRACE;

    return GetNextOccurrence( CDateTime::NowUTCDateTime(), next );
}

/*-------------------------------------------------------------------------*/

bool
CSchedule::GetNextOccurrence( const CDateTime& from, CDateTime& next ) const
{GUCEF_TRACE;

    if ( !IsValid() )
        return false;

    // If schedule has ended relative to 'from', no next occurrence
    if ( m_hasTimeRange && from >= m_timeRange.GetEnd() )
        return false;

    // Case 1: Only time range (no cron pattern)
    if ( !m_hasCronSchedule && m_hasTimeRange )
    {
        // Next occurrence is the start of the range if we haven't reached it yet
        if ( from < m_timeRange.GetStart() )
        {
            next = m_timeRange.GetStart();
            return true;
        }
        return false; // Already passed the single occurrence
    }

    // Case 2: Only cron pattern (no time range)
    if ( m_hasCronSchedule && !m_hasTimeRange )
    {
        return m_cronSchedule.GetNextOccurrence( from, next );
    }

    // Case 3: Both cron pattern and time range
    if ( m_hasCronSchedule && m_hasTimeRange )
    {
        return m_cronSchedule.GetNextOccurrence( from, m_timeRange, next );
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CSchedule::GetPreviousOccurrence( CDateTime& previous ) const
{GUCEF_TRACE;

    return GetPreviousOccurrence( CDateTime::NowUTCDateTime(), previous );
}

/*-------------------------------------------------------------------------*/

bool
CSchedule::GetPreviousOccurrence( const CDateTime& from, CDateTime& previous ) const
{GUCEF_TRACE;

    if ( !IsValid() )
        return false;

    // Case 1: Only time range (no cron pattern)
    if ( !m_hasCronSchedule && m_hasTimeRange )
    {
        // Previous occurrence is the start of the range if we've already passed it
        if ( from > m_timeRange.GetStart() )
        {
            previous = m_timeRange.GetStart();
            return true;
        }
        return false; // Haven't reached the single occurrence yet
    }

    // Case 2: Only cron pattern (no time range)
    if ( m_hasCronSchedule && !m_hasTimeRange )
    {
        return m_cronSchedule.GetPreviousOccurrence( from, previous );
    }

    // Case 3: Both cron pattern and time range
    if ( m_hasCronSchedule && m_hasTimeRange )
    {
        CDateTime candidate;
        if ( !m_cronSchedule.GetPreviousOccurrence( from, candidate ) )
            return false;

        // Check if the previous occurrence is within the time range [start, end] inclusive
        if ( !( m_timeRange > candidate ) && !( m_timeRange < candidate ) )
        {
            previous = candidate;
            return true;
        }

        return false;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CSchedule::GetNextOccurrences( UInt32 maxOccurrences, TDateTimeVector& occurrences ) const
{GUCEF_TRACE;

    return GetNextOccurrences( CDateTime::NowUTCDateTime(), maxOccurrences, occurrences );
}

/*-------------------------------------------------------------------------*/

UInt32
CSchedule::GetNextOccurrences( const CDateTime& from, UInt32 maxOccurrences, TDateTimeVector& occurrences ) const
{GUCEF_TRACE;

    occurrences.clear();

    if ( !IsValid() || maxOccurrences == 0 )
        return 0;

    // Case 1: Only time range (no cron pattern) - single occurrence
    if ( !m_hasCronSchedule && m_hasTimeRange )
    {
        if ( from < m_timeRange.GetStart() )
        {
            occurrences.push_back( m_timeRange.GetStart() );
            return 1;
        }
        return 0;
    }

    // Case 2: Only cron pattern (no time range)
    if ( m_hasCronSchedule && !m_hasTimeRange )
    {
        return m_cronSchedule.GetNextOccurrences( from, maxOccurrences, occurrences );
    }

    // Case 3: Both cron pattern and time range
    if ( m_hasCronSchedule && m_hasTimeRange )
    {
        return m_cronSchedule.GetOccurrencesInRange( m_timeRange, maxOccurrences, occurrences );
    }

    return 0;
}

/*-------------------------------------------------------------------------*/

UInt64
CSchedule::GetMillisecondsUntilNext( void ) const
{GUCEF_TRACE;

    return GetMillisecondsUntilNext( CDateTime::NowUTCDateTime() );
}

/*-------------------------------------------------------------------------*/

UInt64
CSchedule::GetMillisecondsUntilNext( const CDateTime& from ) const
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

UInt64
CSchedule::GetMillisecondsUntilStart( void ) const
{GUCEF_TRACE;

    if ( !m_hasTimeRange )
        return 0;

    return m_timeRange.TimeTillStartInMilliSecs();
}

/*-------------------------------------------------------------------------*/

UInt64
CSchedule::GetMillisecondsUntilEnd( void ) const
{GUCEF_TRACE;

    if ( !m_hasTimeRange )
        return 0;

    return m_timeRange.TimeTillEndInMilliSecs();
}

/*-------------------------------------------------------------------------*/

UInt64
CSchedule::GetScheduleDurationInMilliSecs( void ) const
{GUCEF_TRACE;

    if ( !m_hasTimeRange )
        return 0;

    return m_timeRange.GetTimeRangeTimeInMilliSecs();
}

/*-------------------------------------------------------------------------*/

bool
CSchedule::IsValid( void ) const
{GUCEF_TRACE;

    // A schedule is valid if it has at least one component defined
    if ( !m_hasCronSchedule && !m_hasTimeRange )
        return false;

    // If cron schedule is defined, it must be valid
    if ( m_hasCronSchedule && !m_cronSchedule.IsValid() )
        return false;

    return true;
}

/*-------------------------------------------------------------------------*/

CString
CSchedule::GetDescription( void ) const
{GUCEF_TRACE;

    if ( !IsValid() )
        return "Invalid schedule";

    CString description;

    // Case 1: Both cron and time range
    if ( m_hasCronSchedule && m_hasTimeRange )
    {
        description = m_cronSchedule.GetDescription();
        description += " from ";
        description += m_timeRange.GetStart().ToIso8601DateTimeString( true, false );
        description += " to ";
        description += m_timeRange.GetEnd().ToIso8601DateTimeString( true, false );
    }
    // Case 2: Only cron schedule
    else if ( m_hasCronSchedule )
    {
        description = m_cronSchedule.GetDescription();
        description += " (no time limit)";
    }
    // Case 3: Only time range
    else if ( m_hasTimeRange )
    {
        description = "One-time at ";
        description += m_timeRange.GetStart().ToIso8601DateTimeString( true, false );
    }

    return description;
}

/*-------------------------------------------------------------------------*/

void
CSchedule::Clear( void )
{GUCEF_TRACE;

    m_cronSchedule.Clear();
    m_timeRange.Clear();
    m_hasCronSchedule = false;
    m_hasTimeRange = false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
