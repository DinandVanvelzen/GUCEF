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

#ifndef GUCEF_CORE_CSCHEDULE_H
#define GUCEF_CORE_CSCHEDULE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_CDATETIMERANGE_H
#include "gucefCORE_CDateTimeRange.h"
#define GUCEF_CORE_CDATETIMERANGE_H
#endif /* GUCEF_CORE_CDATETIMERANGE_H ? */

#ifndef GUCEF_CORE_CCRONSCHEDULE_H
#include "gucefCORE_CCronSchedule.h"
#define GUCEF_CORE_CCRONSCHEDULE_H
#endif /* GUCEF_CORE_CCRONSCHEDULE_H ? */

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

/**
 *  Class representing a complete schedule combining a cron-style recurring pattern
 *  with an optional time window (start/end dates) during which the schedule is active.
 *  
 *  This allows schedules like:
 *  - "Every hour, forever" (cron only, no time range)
 *  - "Every day at 2am, from Jan 1 to Dec 31, 2024" (cron + time range)
 *  - "Only during business hours on Jan 15, 2024" (time range only, no recurrence)
 */
class GUCEF_CORE_PUBLIC_CPP CSchedule
{
    public:

    typedef CCronSchedule::TDateTimeVector TDateTimeVector;

    static const CSchedule Empty;

    CSchedule( void );

    /**
     *  Create a schedule with only a cron pattern (no time range restriction)
     */
    explicit CSchedule( const CCronSchedule& cronSchedule );

    /**
     *  Create a schedule with only a time range (no recurring pattern)
     */
    explicit CSchedule( const CDateTimeRange& timeRange );

    /**
     *  Create a schedule with both a cron pattern and time range
     */
    CSchedule( const CCronSchedule& cronSchedule, const CDateTimeRange& timeRange );

    CSchedule( const CSchedule& src );

    ~CSchedule();

    CSchedule& operator=( const CSchedule& src );

    bool operator==( const CSchedule& other ) const;

    bool operator!=( const CSchedule& other ) const;

    bool operator<( const CSchedule& other ) const;

    bool operator<=( const CSchedule& other ) const;

    bool operator>( const CSchedule& other ) const;

    bool operator>=( const CSchedule& other ) const;

    /**
     *  Set the cron schedule pattern
     */
    void SetCronSchedule( const CCronSchedule& cronSchedule );

    /**
     *  Get the cron schedule pattern
     */
    const CCronSchedule& GetCronSchedule( void ) const;

    CCronSchedule& GetCronSchedule( void );

    /**
     *  Set the time range during which the schedule is active
     */
    void SetTimeRange( const CDateTimeRange& timeRange );

    /**
     *  Get the time range during which the schedule is active
     */
    const CDateTimeRange& GetTimeRange( void ) const;

    CDateTimeRange& GetTimeRange( void );

    /**
     *  Check if the schedule has a cron pattern defined
     */
    bool HasCronSchedule( void ) const;

    /**
     *  Check if the schedule has a time range defined
     */
    bool HasTimeRange( void ) const;

    /**
     *  Check if the schedule is currently active (within time range if specified)
     */
    bool IsActive( void ) const;

    /**
     *  Check if the schedule is currently active at the given time
     */
    bool IsActive( const CDateTime& dt ) const;

    /**
     *  Check if the schedule has started (passed start time)
     */
    bool HasStarted( void ) const;

    /**
     *  Check if the schedule has ended (passed end time)
     */
    bool HasEnded( void ) const;

    /**
     *  Check if the given datetime matches this schedule
     *  Takes into account both cron pattern and time range
     */
    bool Matches( const CDateTime& dt ) const;

    /**
     *  Get the next scheduled occurrence from now
     *  Returns true if a next occurrence was found
     */
    bool GetNextOccurrence( CDateTime& next ) const;

    /**
     *  Get the next scheduled occurrence after the given datetime
     *  Returns true if a next occurrence was found
     */
    bool GetNextOccurrence( const CDateTime& from, CDateTime& next ) const;

    /**
     *  Get the previous scheduled occurrence before now
     *  Returns true if a previous occurrence was found
     */
    bool GetPreviousOccurrence( CDateTime& previous ) const;

    /**
     *  Get the previous scheduled occurrence before the given datetime
     *  Returns true if a previous occurrence was found
     */
    bool GetPreviousOccurrence( const CDateTime& from, CDateTime& previous ) const;

    /**
     *  Calculate the next N occurrences from now
     */
    UInt32 GetNextOccurrences( UInt32 maxOccurrences, TDateTimeVector& occurrences ) const;

    /**
     *  Calculate the next N occurrences after the given datetime
     */
    UInt32 GetNextOccurrences( const CDateTime& from, UInt32 maxOccurrences, TDateTimeVector& occurrences ) const;

    /**
     *  Calculate milliseconds until the next scheduled occurrence from now
     *  Returns 0 if no next occurrence or schedule has ended
     */
    UInt64 GetMillisecondsUntilNext( void ) const;

    /**
     *  Calculate milliseconds until the next scheduled occurrence from the given time
     *  Returns 0 if no next occurrence or schedule has ended
     */
    UInt64 GetMillisecondsUntilNext( const CDateTime& from ) const;

    /**
     *  Calculate milliseconds until schedule starts
     *  Returns 0 if already started or no time range specified
     */
    UInt64 GetMillisecondsUntilStart( void ) const;

    /**
     *  Calculate milliseconds until schedule ends
     *  Returns 0 if already ended or no time range specified
     */
    UInt64 GetMillisecondsUntilEnd( void ) const;

    /**
     *  Get total duration of the schedule time range in milliseconds
     *  Returns 0 if no time range specified
     */
    UInt64 GetScheduleDurationInMilliSecs( void ) const;

    /**
     *  Check if this schedule is valid and can produce occurrences
     */
    bool IsValid( void ) const;

    /**
     *  Get a human-readable description of the schedule
     */
    CString GetDescription( void ) const;

    /**
     *  Clear all schedule data
     */
    void Clear( void );

    private:

    CCronSchedule m_cronSchedule;
    CDateTimeRange m_timeRange;
    bool m_hasCronSchedule;
    bool m_hasTimeRange;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CSCHEDULE_H ? */
