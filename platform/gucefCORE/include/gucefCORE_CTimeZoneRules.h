/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_CTIMEZONERULES_H
#define GUCEF_CORE_CTIMEZONERULES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDATE_H
#include "gucefCORE_CDate.h"
#define GUCEF_CORE_CDATE_H
#endif /* GUCEF_CORE_CDATE_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_STL_H
#include "gucef_stl.h"
#define GUCEF_STL_H
#endif /* GUCEF_STL_H ? */

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
 *  Time zone rules as data: a zone name (an IANA name such as "America/New_York") maps to a standard offset from UTC and a
 *  daylight saving rule, each valid for a range of years. CDateTime holds a fixed offset from UTC only and the operating system
 *  zone database is not the same on every platform, so an application that has to convert between UTC and the local time of a
 *  place supplies the rules it needs as data. GUCEF ships no zone data: the application owns the data and with it the question
 *  of how far the entries have been confirmed against the IANA tz database.
 *
 *  Data format (a data node, loaded from a file of any codec the data store codec registry knows, JSON in the examples):
 *
 *      {
 *          "rules": [ { "name": "US", "periods": [
 *              { "fromYear": 2007, "toYear": 9999, "saveMinutes": 60, "start": "Mar Sun>=8 2:00", "end": "Nov Sun>=1 2:00" } ] } ],
 *          "zones": [ { "name": "America/New_York", "standardOffsetMinutes": -300, "periods": [ { "fromYear": 1987, "toYear": 9999, "rule": "US" } ] } ]
 *      }
 *
 *  A transition is written as in the IANA rule lines: month (the first three letters of the name), day ("15", "lastSun",
 *  "Sun>=8", "Sun<=25") and the time of day with an optional suffix: none or "w" wall clock time (of the offset in force before
 *  the transition), "s" standard time, "u" UTC. A zone period without "rule", or with the rule "NONE", has no daylight saving.
 *  A period whose daylight saving starts in a later month than it ends in (October to April) is a southern hemisphere rule.
 *  Other attributes of the entries (a comment, a status) are ignored by the loader, which makes them available to the application.
 *
 *  Years: a zone answers for the years its periods cover and refuses all others, instead of guessing. The year of an instant is
 *  the year of the standard time of the zone. A zone with a rule that has no period for the year is refused for that year too.
 *
 *  Instants are UTC seconds since 1970-01-01T00:00:00Z (Int64, negative before it). Offsets are minutes east of UTC. A day number
 *  is a count of days since 1970-01-01 of the local calendar of the zone, the same count as CDate::ToDaysSinceUnixEpoch().
 *  Everything above the second is whole minutes in the data, so a conversion from a finer unit floors to the second.
 *
 *  Local times that are not one instant: when the clocks go back a local time occurs twice (ambiguous) and when they go forward
 *  one hour of local times does not occur (nonexistent). The conversion from local time to UTC takes both policies as parameters,
 *  nothing is decided silently, and the kind of the local time can be asked for.
 *
 *  Thread safety: the rules are immutable once loaded. The load functions modify the object and must not run at the same time
 *  as another call; every other function is const and can be called from any number of threads at once.
 */
class GUCEF_CORE_PUBLIC_CPP CTimeZoneRules
{
    public:

    /** What the conversion from local time to UTC does with a local time that occurs twice (the clocks go back) */
    enum EAmbiguousTimePolicy
    {
        AMBIGUOUSTIME_EARLIER = 0,      /**< the first occurrence: the offset from before the transition (daylight time) */
        AMBIGUOUSTIME_LATER,            /**< the second occurrence: the offset from after the transition (standard time) */
        AMBIGUOUSTIME_REJECT            /**< the conversion fails */
    };
    typedef enum EAmbiguousTimePolicy TAmbiguousTimePolicy;

    /** What the conversion from local time to UTC does with a local time that does not occur (the clocks go forward) */
    enum ENonexistentTimePolicy
    {
        NONEXISTENTTIME_SHIFT_FORWARD = 0,  /**< the local time is read with the offset from before the transition: 02:30 becomes the instant of 03:30 daylight time */
        NONEXISTENTTIME_SHIFT_BACKWARD,     /**< the local time is read with the offset from after the transition: 02:30 becomes the instant of 01:30 standard time */
        NONEXISTENTTIME_REJECT              /**< the conversion fails */
    };
    typedef enum ENonexistentTimePolicy TNonexistentTimePolicy;

    /** How many instants a local time stands for */
    enum ELocalTimeKind
    {
        LOCALTIME_UNIQUE = 0,           /**< one instant */
        LOCALTIME_AMBIGUOUS,            /**< two instants: the clocks went back */
        LOCALTIME_NONEXISTENT           /**< no instant: the clocks went forward over it */
    };
    typedef enum ELocalTimeKind TLocalTimeKind;

    CTimeZoneRules( void );
    CTimeZoneRules( const CTimeZoneRules& src );
    CTimeZoneRules& operator=( const CTimeZoneRules& src );
    ~CTimeZoneRules();

    /** Forgets all rules and zones */
    void Clear( void );

    /**
     *  Adds the rules and zones of the data node. Entries of the same name replace the ones that are there. All or nothing: when an entry
     *  is not valid, false is returned with the reason in 'error' and the object is as it was.
     */
    bool LoadFromDataNode( const CDataNode& root, CString& error );

    /** Reads a document with the data store codec of the given type name ("json", "xml", ...) and adds it as LoadFromDataNode() does */
    bool LoadFromIOAccess( CIOAccess& access, const CString& codecTypeName, CString& error );

    /** Reads a file of the file system, the codec follows from the extension, and adds it as LoadFromDataNode() does */
    bool LoadFromFile( const CString& filePath, CString& error );

    /* ---- the zones ---- */

    bool HasZone( const CString& zoneName ) const;
    UInt32 GetZoneCount( void ) const { return (UInt32) m_zones.size(); }
    void GetZoneNames( CString::StringVector& names ) const;

    /** The standard offset of the zone in minutes east of UTC; false for an unknown zone */
    bool GetStandardOffsetMinutes( const CString& zoneName, Int32& offsetMinutes ) const;

    /** Whether the zone has rules for the year: answered by its periods, whether or not the zone has daylight saving in that year. False for an unknown zone */
    bool IsYearKnown( const CString& zoneName, Int32 year ) const;

    /** The first and the last year of the periods of the zone. Years between the two can still be unknown when the periods have a gap. False for an unknown zone */
    bool GetKnownYearRange( const CString& zoneName, Int32& firstYear, Int32& lastYear ) const;

    /* ---- UTC instant to local time ---- */

    /** The offset from UTC at the instant in minutes east of UTC; false for an unknown zone or a year the zone has no rule for */
    bool GetOffsetAtInstant( const CString& zoneName, Int64 utcSeconds, Int32& offsetMinutes, bool* isDaylightSaving = GUCEF_NULL ) const;

    /** As above for an instant given as a date and time, which is read with its own offset (UTC when it has none) */
    bool GetOffsetAtInstant( const CString& zoneName, const CDateTime& instant, Int32& offsetMinutes, bool* isDaylightSaving = GUCEF_NULL ) const;

    /** The local calendar day (day number) of the instant */
    bool GetLocalDay( const CString& zoneName, Int64 utcSeconds, Int32& day ) const;

    /** The local calendar date of the instant. False as well when the year does not fit a CDate */
    bool GetLocalDate( const CString& zoneName, Int64 utcSeconds, CDate& date ) const;

    /** The local date and time of the instant, as a date and time that carries the offset of the zone at the instant */
    bool UtcToLocal( const CString& zoneName, Int64 utcSeconds, CDateTime& local ) const;

    /** As above for an instant given as a date and time, which is read with its own offset (UTC when it has none). The milliseconds are kept */
    bool UtcToLocal( const CString& zoneName, const CDateTime& instant, CDateTime& local ) const;

    /* ---- local time to UTC instant ---- */

    /**
     *  The instant of a local time. The local time is given as the seconds since 1970-01-01T00:00:00 of the local calendar, that is the
     *  wall clock read as if it were UTC. Ambiguous and nonexistent local times are resolved with the policies. 'kind' tells which
     *  of the three it was, also when the conversion is rejected by the policy: then false is returned and 'kind' says why. False as well
     *  for an unknown zone or a year the zone has no rule for around the local time (up to a day either side).
     */
    bool LocalToUtc( const CString& zoneName               ,
                     Int64 localSeconds                    ,
                     TAmbiguousTimePolicy ambiguousPolicy  ,
                     TNonexistentTimePolicy nonexistentPolicy ,
                     Int64& utcSeconds                     ,
                     TLocalTimeKind* kind = GUCEF_NULL     ) const;

    /** As above for a local date and time. The offset the date and time carries is ignored: it is a wall clock reading. The result is UTC, with the milliseconds of the input */
    bool LocalToUtc( const CString& zoneName               ,
                     const CDateTime& local                ,
                     TAmbiguousTimePolicy ambiguousPolicy  ,
                     TNonexistentTimePolicy nonexistentPolicy ,
                     CDateTime& utc                        ,
                     TLocalTimeKind* kind = GUCEF_NULL     ) const;

    /** Which kind the local time (seconds as in LocalToUtc()) is; false for an unknown zone or a year without rule */
    bool GetLocalTimeKind( const CString& zoneName, Int64 localSeconds, TLocalTimeKind& kind ) const;

    /* ---- local days ---- */

    /** The first instant of the local day: local midnight, or the instant of the transition when a transition skips midnight */
    bool GetLocalDayStart( const CString& zoneName, Int32 day, Int64& utcSeconds ) const;

    /** [start, end) of the local day in UTC. The length is 24 hours, except on the days of a transition */
    bool GetLocalDayBounds( const CString& zoneName, Int32 day, Int64& startUtcSeconds, Int64& endUtcSeconds ) const;

    /** The instants of the two transitions of the year; 'hasDaylightSaving' is false when the zone has none in that year. False when the zone is unknown or has no rule for the year */
    bool GetTransitionsOfYear( const CString& zoneName, Int32 year, bool& hasDaylightSaving, Int64& daylightStartUtcSeconds, Int64& daylightEndUtcSeconds ) const;

    /* ---- calendar helpers ---- */

    /** The day number of a date of the proleptic Gregorian calendar */
    static Int64 DaysFromCivil( Int64 year, Int32 month, Int32 day );

    /** The date of a day number */
    static void CivilFromDays( Int64 days, Int64& year, Int32& month, Int32& day );

    /** 0 = Sunday .. 6 = Saturday */
    static Int32 WeekdayOfDay( Int64 days );

    private:

    enum EDayKind { DAYKIND_FIXED = 0, DAYKIND_LAST, DAYKIND_ON_OR_AFTER, DAYKIND_ON_OR_BEFORE };

    struct STransition
    {
        Int32 month;
        EDayKind kind;
        Int32 weekday;          /**< 0 = Sunday */
        Int32 dayOfMonth;
        Int32 timeMinutes;
        char basis;             /**< 'w', 's' or 'u' */

        STransition( void );
    };

    struct SRulePeriod
    {
        Int32 fromYear;
        Int32 toYear;
        Int32 saveMinutes;
        STransition start;
        STransition end;

        SRulePeriod( void );
    };

    struct SZonePeriod
    {
        Int32 fromYear;
        Int32 toYear;
        CString ruleName;       /**< empty: no daylight saving */

        SZonePeriod( void );
    };

    struct SZone
    {
        Int32 standardOffsetMinutes;
        GUCEF::vector< SZonePeriod > periods;

        SZone( void );
    };

    typedef GUCEF::vector< SRulePeriod > TRulePeriodVector;
    typedef GUCEF::map< CString, TRulePeriodVector > TRuleMap;
    typedef GUCEF::map< CString, SZone > TZoneMap;

    static bool ParseTransition( const CString& text, STransition& transition );
    static bool ParseRules( const CDataNode& root, TRuleMap& rules, CString& error );
    static bool ParseZones( const CDataNode& root, const TRuleMap& rules, TZoneMap& zones, CString& error );
    static Int64 TransitionMinutes( const STransition& transition, Int32 year, Int32 standardOffsetMinutes, Int32 offsetBeforeMinutes );
    static bool ToUtcSeconds( const CDateTime& dateTime, Int64& seconds, Int32& milliseconds );
    static bool FromUtcSeconds( Int64 seconds, Int32 milliseconds, Int32 offsetMinutes, CDateTime& dateTime );
    const SRulePeriod* FindRulePeriod( const SZone& zone, Int32 year, bool& hasRule ) const;
    bool OffsetAtMinute( const SZone& zone, Int64 utcMinute, Int32& offsetMinutes, bool* isDaylightSaving ) const;
    bool ResolveLocalMinute( const SZone& zone, Int64 localMinute, TAmbiguousTimePolicy ambiguousPolicy, TNonexistentTimePolicy nonexistentPolicy, Int64& utcMinute, TLocalTimeKind& kind ) const;
    bool FindZone( const CString& zoneName, const SZone*& zone ) const;

    TRuleMap m_rules;
    TZoneMap m_zones;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTIMEZONERULES_H ? */
