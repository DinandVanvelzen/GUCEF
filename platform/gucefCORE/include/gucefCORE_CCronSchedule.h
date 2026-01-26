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

#ifndef GUCEF_CORE_CCRONSCHEDULE_H
#define GUCEF_CORE_CCRONSCHEDULE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <set>
#include <vector>

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      FORWARD DECLARATIONS                                               //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {
    class CDateTimeRange;
}
}

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
 *  Class representing a Unix/Linux cron schedule
 *  Supports standard cron format: minute hour day-of-month month day-of-week
 *  Special characters: * (all), , (list), - (range), / (step)
 *  Ranges: minute (0-59), hour (0-23), day-of-month (1-31), month (1-12), day-of-week (0-6, Sunday=0)
 */
class GUCEF_CORE_PUBLIC_CPP CCronSchedule
{
    public:

    typedef GUCEF::set< UInt8 > TUInt8Set;
    typedef GUCEF::vector< CDateTime > TDateTimeVector;

    static const CCronSchedule Empty;

    /**
     *  Create common preset schedules
     */
    static CCronSchedule EveryMinute( void );          // "* * * * *"
    static CCronSchedule Hourly( void );               // "0 * * * *"
    static CCronSchedule Daily( void );                // "0 0 * * *"
    static CCronSchedule Weekly( void );               // "0 0 * * 0"
    static CCronSchedule Monthly( void );              // "0 0 1 * *"
    static CCronSchedule Yearly( void );               // "0 0 1 1 *"
    static CCronSchedule BusinessHours40hrWw( void );  // "0 9-17 * * 1-5"

    CCronSchedule( void );

    CCronSchedule( const CString& cronExpression );

    CCronSchedule( const CCronSchedule& src );

    ~CCronSchedule();

    CCronSchedule& operator=( const CCronSchedule& src );

    bool operator==( const CCronSchedule& other ) const;

    bool operator!=( const CCronSchedule& other ) const;

    /**
     *  Parse a cron expression string
     *  Format: "minute hour day-of-month month day-of-week"
     *  Examples: "0 2 * * *" = 2am daily, "* /15 * * * *" = every 15 minutes
     *                                     Extra space above between * and / is to prevent markdown issues
     *  Returns true if parsing was successful
     */
    bool FromCronString( const CString& cronExpression );

    /**
     *  Convert the schedule back to a cron expression string
     */
    CString ToCronString( void ) const;

    /**
     *  Check if the given datetime matches this cron schedule
     */
    bool Matches( const CDateTime& dt ) const;

    /**
     *  Calculate the next occurrence after the given datetime
     *  Returns true if a next occurrence was found
     */
    bool GetNextOccurrence( const CDateTime& from, CDateTime& next ) const;

    /**
     *  Calculate the previous occurrence before the given datetime
     *  Returns true if a previous occurrence was found
     */
    bool GetPreviousOccurrence( const CDateTime& from, CDateTime& previous ) const;

    /**
     *  Check if the given datetime matches this cron schedule AND falls within the given range
     *  Useful for: "Run every hour, but only between 9am-5pm on weekdays"
     */
    bool Matches( const CDateTime& dt, const CDateTimeRange& validWindow ) const;

    /**
     *  Get next occurrence that also falls within the valid time range
     *  Returns true if a next occurrence was found within the range
     */
    bool GetNextOccurrence( const CDateTime& from             ,
                            const CDateTimeRange& validWindow ,
                            CDateTime& next                   ) const;

    /**
     *  Calculate the next N occurrences after the given datetime
     *  Useful for pre-scheduling multiple task executions
     *  Returns the number of occurrences found (may be less than maxOccurrences)
     */
    UInt32 GetNextOccurrences( const CDateTime& from        ,
                               UInt32 maxOccurrences        ,
                               TDateTimeVector& occurrences ) const;

    /**
     *  Calculate occurrences within a specific time range
     *  Useful for: "Show me all times this task will run next week"
     *  Returns the number of occurrences found (may be less than maxOccurrences)
     */
    UInt32 GetOccurrencesInRange( const CDateTimeRange& range  ,
                                  UInt32 maxOccurrences        ,
                                  TDateTimeVector& occurrences ) const;

    /**
     *  Calculate milliseconds until the next scheduled occurrence from now
     *  Returns 0 if no next occurrence can be determined
     */
    UInt64 GetMillisecondsUntilNext( void ) const;

    /**
     *  Calculate milliseconds until the next scheduled occurrence from the given time
     *  Returns 0 if no next occurrence can be determined
     */
    UInt64 GetMillisecondsUntilNext( const CDateTime& from ) const;

    /**
     *  Returns true if this schedule triggers every minute (e.g., "* * * * *")
     */
    bool IsEveryMinute( void ) const;

    /**
     *  Returns true if this schedule triggers every hour (e.g., "0 * * * *")
     */
    bool IsEveryHour( void ) const;

    /**
     *  Returns true if this schedule triggers daily (e.g., "0 0 * * *")
     */
    bool IsDaily( void ) const;

    /**
     *  Get approximate interval in milliseconds between occurrences
     *  Returns 0 if the interval is variable or cannot be determined
     */
    UInt64 GetApproximateIntervalInMilliSecs( void ) const;

    /**
     *  Get a human-readable description of the schedule
     *  e.g., "Every 15 minutes", "Daily at 2:00 AM", "Weekdays at 9:00 AM"
     */
    CString GetDescription( void ) const;

    /**
     *  Check if this schedule will trigger at any point
     *  Returns false if the schedule is impossible (e.g., Feb 31)
     */
    bool IsValid( void ) const;

    /**
     *  Get the set of minutes when this schedule triggers (0-59)
     */
    const TUInt8Set& GetMinutes( void ) const;

    /**
     *  Get the set of hours when this schedule triggers (0-23)
     */
    const TUInt8Set& GetHours( void ) const;

    /**
     *  Get the set of days of month when this schedule triggers (1-31)
     */
    const TUInt8Set& GetDaysOfMonth( void ) const;

    /**
     *  Get the set of months when this schedule triggers (1-12)
     */
    const TUInt8Set& GetMonths( void ) const;

    /**
     *  Get the set of days of week when this schedule triggers (0-6, Sunday=0)
     */
    const TUInt8Set& GetDaysOfWeek( void ) const;

    /**
     *  Set the minutes when this schedule triggers (0-59)
     */
    void SetMinutes( const TUInt8Set& minutes );

    /**
     *  Set the hours when this schedule triggers (0-23)
     */
    void SetHours( const TUInt8Set& hours );

    /**
     *  Set the days of month when this schedule triggers (1-31)
     */
    void SetDaysOfMonth( const TUInt8Set& daysOfMonth );

    /**
     *  Set the months when this schedule triggers (1-12)
     */
    void SetMonths( const TUInt8Set& months );

    /**
     *  Set the days of week when this schedule triggers (0-6, Sunday=0)
     */
    void SetDaysOfWeek( const TUInt8Set& daysOfWeek );

    /**
     *  Clear all schedule data
     */
    void Clear( void );

    private:

    bool ParseCronField( const CString& field, UInt8 minValue, UInt8 maxValue, TUInt8Set& values );

    bool MatchesDayConstraints( const CDateTime& dt ) const;

    void IncrementToNextMinute( CDateTime& dt ) const;

    void IncrementToNextHour( CDateTime& dt ) const;

    void IncrementToNextDay( CDateTime& dt ) const;

    void IncrementToNextMonth( CDateTime& dt ) const;

    void DecrementToPreviousMinute( CDateTime& dt ) const;

    void DecrementToPreviousHour( CDateTime& dt ) const;

    void DecrementToPreviousDay( CDateTime& dt ) const;

    void DecrementToPreviousMonth( CDateTime& dt ) const;

    UInt8 GetNextValue( const TUInt8Set& values, UInt8 current, UInt8 maxValue, bool& wrapped ) const;

    UInt8 GetPreviousValue( const TUInt8Set& values, UInt8 current, UInt8 maxValue, bool& wrapped ) const;

    CString SetToCronField( const TUInt8Set& values, UInt8 minValue, UInt8 maxValue ) const;

    private:

    TUInt8Set m_minutes;       // 0-59
    TUInt8Set m_hours;         // 0-23
    TUInt8Set m_daysOfMonth;   // 1-31
    TUInt8Set m_months;        // 1-12
    TUInt8Set m_daysOfWeek;    // 0-6 (Sunday=0)
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CCRONSCHEDULE_H ? */
