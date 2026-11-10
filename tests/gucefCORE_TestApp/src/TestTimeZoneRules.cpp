/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CTIMEZONERULES_H
#include "gucefCORE_CTimeZoneRules.h"
#define GUCEF_CORE_CTIMEZONERULES_H
#endif /* GUCEF_CORE_CTIMEZONERULES_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestJsonDataNodeFidelity.h"       /* the json codec of the test application */

#include "TestTimeZoneRules.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::CORE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const Int64 SECONDS_PER_HOUR = 3600;
static const Int64 SECONDS_PER_DAY = 86400;

/*-------------------------------------------------------------------------*/

/** A UTC instant in seconds */
static Int64
Utc( Int32 year, Int32 month, Int32 day, Int32 hour, Int32 minute )
{
    return CTimeZoneRules::DaysFromCivil( year, month, day ) * SECONDS_PER_DAY + (Int64) hour * 3600 + (Int64) minute * 60;
}

/*-------------------------------------------------------------------------*/

/** The day number of a date */
static Int32
DayOf( Int32 year, Int32 month, Int32 day )
{
    return (Int32) CTimeZoneRules::DaysFromCivil( year, month, day );
}

/*-------------------------------------------------------------------------*/

static CString
FixturePath( void )
{
    return RelativePath( ResolveVars( "$MODULEDIR$/../data/gucefCORE_TestApp/TimeZoneRulesFixture.json" ) );
}

/*-------------------------------------------------------------------------*/

/** Loads the fixture of the test application. False, with the reason in the log, when that fails: the tests that follow then fail on their own */
static bool
LoadFixture( CTimeZoneRules& rules )
{
    CString error;
    if ( rules.LoadFromFile( FixturePath(), error ) )
        return true;
    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "TimeZoneRules test: the fixture " << FixturePath() << " cannot be loaded: " << error );
    return false;
}

/*-------------------------------------------------------------------------*/

/** Loads a JSON document held in memory */
static bool
LoadJson( CTimeZoneRules& rules, const CString& text, CString& error )
{
    CDynamicBuffer buffer;
    buffer.CopyFrom( text.ByteSize() - 1, text.C_String() );
    CDynamicBufferAccess access( buffer );
    return rules.LoadFromIOAccess( access, "json", error );
}

/*-------------------------------------------------------------------------*/

/** A deterministic generator for the property tests */
class CRandom
{
    public:

    explicit CRandom( UInt64 seed ) : m_state( seed ) {}

    UInt64 Next( void )
    {
        m_state = m_state * 6364136223846793005ULL + 1442695040888963407ULL;
        return m_state >> 11;
    }

    Int64 Between( Int64 low, Int64 high ) { return low + (Int64) ( Next() % (UInt64) ( high - low + 1 ) ); }

    private:

    UInt64 m_state;
};

/*-------------------------------------------------------------------------*/

/** A document with one rule and one zone that uses it, for the tests of the forms a transition can be written in */
static CString
OneZoneDocument( const CString& zoneName, Int32 standardOffsetMinutes, Int32 saveMinutes, const CString& start, const CString& end )
{
    return "{ \"rules\": [ { \"name\": \"R\", \"periods\": [ { \"fromYear\": 2024, \"toYear\": 2025, \"saveMinutes\": " + ToString( saveMinutes ) +
           ", \"start\": \"" + start + "\", \"end\": \"" + end + "\" } ] } ], "
           "\"zones\": [ { \"name\": \"" + zoneName + "\", \"standardOffsetMinutes\": " + ToString( standardOffsetMinutes ) +
           ", \"periods\": [ { \"fromYear\": 2024, \"toYear\": 2025, \"rule\": \"R\" } ] } ] }";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformLoadTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: The fixture loads from a file, with its zones, standard offsets and the years it is valid for" )
        try
        {
            CTimeZoneRules rules;
            CString error;
            ASSERT_TRUE( rules.LoadFromFile( FixturePath(), error ) );
            ASSERT_TRUE( 19 == rules.GetZoneCount() );
            ASSERT_TRUE( rules.HasZone( "America/New_York" ) && rules.HasZone( "Australia/Sydney" ) );
            ASSERT_FALSE( rules.HasZone( "Nowhere/Nothing" ) );

            CString::StringVector names;
            rules.GetZoneNames( names );
            ASSERT_TRUE( 19 == names.size() );

            Int32 offset = 0;
            ASSERT_TRUE( rules.GetStandardOffsetMinutes( "America/New_York", offset ) && -300 == offset );
            ASSERT_TRUE( rules.GetStandardOffsetMinutes( "Asia/Kolkata", offset ) && 330 == offset );
            ASSERT_FALSE( rules.GetStandardOffsetMinutes( "Nowhere/Nothing", offset ) );

            Int32 first = 0, last = 0;
            ASSERT_TRUE( rules.GetKnownYearRange( "America/New_York", first, last ) && 1987 == first && 9999 == last );
            ASSERT_TRUE( rules.GetKnownYearRange( "America/Mexico_City", first, last ) && 2002 == first && 9999 == last );
            ASSERT_FALSE( rules.GetKnownYearRange( "Nowhere/Nothing", first, last ) );
            ASSERT_TRUE( rules.IsYearKnown( "America/New_York", 1987 ) && rules.IsYearKnown( "America/New_York", 2100 ) );
            ASSERT_FALSE( rules.IsYearKnown( "America/New_York", 1986 ) );
            ASSERT_FALSE( rules.IsYearKnown( "Nowhere/Nothing", 2024 ) );

            // a second load adds to what is there, a clear removes it
            ASSERT_TRUE( LoadJson( rules, "{ \"zones\": [ { \"name\": \"Etc/Plus3\", \"standardOffsetMinutes\": 180, \"periods\": [ { \"fromYear\": 1, \"toYear\": 9999, \"rule\": \"NONE\" } ] } ] }", error ) );
            ASSERT_TRUE( 20 == rules.GetZoneCount() && rules.HasZone( "Etc/Plus3" ) && rules.HasZone( "Europe/Paris" ) );
            rules.Clear();
            ASSERT_TRUE( 0 == rules.GetZoneCount() );
            Int32 dummy = 0;
            ASSERT_FALSE( rules.GetOffsetAtInstant( "America/New_York", Utc( 2024, 1, 1, 0, 0 ), dummy ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Loading refuses what is not a file, a codec or a valid document, and leaves the rules as they were" )
        try
        {
            CTimeZoneRules rules;
            ASSERT_TRUE( LoadFixture( rules ) );
            CString error;

            ASSERT_FALSE( rules.LoadFromFile( RelativePath( ResolveVars( "$MODULEDIR$/../data/gucefCORE_TestApp/DoesNotExist.json" ) ), error ) );
            ASSERT_FALSE( error.IsNULLOrEmpty() );

            CDynamicBuffer buffer;
            const CString text = "{}";
            buffer.CopyFrom( text.ByteSize() - 1, text.C_String() );
            CDynamicBufferAccess access( buffer );
            error.Clear();
            ASSERT_FALSE( rules.LoadFromIOAccess( access, "nosuchcodec", error ) );
            ASSERT_FALSE( error.IsNULLOrEmpty() );
            error.Clear();
            ASSERT_FALSE( rules.LoadFromIOAccess( access, "", error ) );
            ASSERT_FALSE( error.IsNULLOrEmpty() );

            static const char* const invalid[] =
            {
                // a rule without periods, without a name
                "{ \"rules\": [ { \"name\": \"X\" } ] }",
                "{ \"rules\": [ { \"periods\": [ { \"fromYear\": 2000, \"toYear\": 2001, \"start\": \"Mar Sun>=8 2:00\", \"end\": \"Nov Sun>=1 2:00\" } ] } ] }",
                // a period that ends before it starts, that has no first year, with a save of nothing
                "{ \"rules\": [ { \"name\": \"X\", \"periods\": [ { \"fromYear\": 2002, \"toYear\": 2001, \"start\": \"Mar Sun>=8 2:00\", \"end\": \"Nov Sun>=1 2:00\" } ] } ] }",
                "{ \"rules\": [ { \"name\": \"X\", \"periods\": [ { \"toYear\": 2001, \"start\": \"Mar Sun>=8 2:00\", \"end\": \"Nov Sun>=1 2:00\" } ] } ] }",
                "{ \"rules\": [ { \"name\": \"X\", \"periods\": [ { \"fromYear\": 2000, \"toYear\": 2001, \"saveMinutes\": 0, \"start\": \"Mar Sun>=8 2:00\", \"end\": \"Nov Sun>=1 2:00\" } ] } ] }",
                // transitions that are not valid: month, weekday, day, missing time, time out of range, missing part, extra part
                "{ \"rules\": [ { \"name\": \"X\", \"periods\": [ { \"fromYear\": 2000, \"toYear\": 2001, \"start\": \"Foo Sun>=8 2:00\", \"end\": \"Nov Sun>=1 2:00\" } ] } ] }",
                "{ \"rules\": [ { \"name\": \"X\", \"periods\": [ { \"fromYear\": 2000, \"toYear\": 2001, \"start\": \"Mar Foo>=8 2:00\", \"end\": \"Nov Sun>=1 2:00\" } ] } ] }",
                "{ \"rules\": [ { \"name\": \"X\", \"periods\": [ { \"fromYear\": 2000, \"toYear\": 2001, \"start\": \"Mar Sun>=0 2:00\", \"end\": \"Nov Sun>=1 2:00\" } ] } ] }",
                "{ \"rules\": [ { \"name\": \"X\", \"periods\": [ { \"fromYear\": 2000, \"toYear\": 2001, \"start\": \"Mar Sun>=8 2\", \"end\": \"Nov Sun>=1 2:00\" } ] } ] }",
                "{ \"rules\": [ { \"name\": \"X\", \"periods\": [ { \"fromYear\": 2000, \"toYear\": 2001, \"start\": \"Mar Sun>=8 25:00\", \"end\": \"Nov Sun>=1 2:00\" } ] } ] }",
                "{ \"rules\": [ { \"name\": \"X\", \"periods\": [ { \"fromYear\": 2000, \"toYear\": 2001, \"start\": \"Mar Sun>=8 2:75\", \"end\": \"Nov Sun>=1 2:00\" } ] } ] }",
                "{ \"rules\": [ { \"name\": \"X\", \"periods\": [ { \"fromYear\": 2000, \"toYear\": 2001, \"start\": \"Mar Sun>=8\", \"end\": \"Nov Sun>=1 2:00\" } ] } ] }",
                "{ \"rules\": [ { \"name\": \"X\", \"periods\": [ { \"fromYear\": 2000, \"toYear\": 2001, \"start\": \"Mar Sun>=8 2:00 extra\", \"end\": \"Nov Sun>=1 2:00\" } ] } ] }",
                "{ \"rules\": [ { \"name\": \"X\", \"periods\": [ { \"fromYear\": 2000, \"toYear\": 2001, \"start\": \"Mar Sun>=8 2:00\" } ] } ] }",
                // a zone without name, without periods, with an offset out of range, with a period that is not valid, that names a rule that is not there
                "{ \"zones\": [ { \"standardOffsetMinutes\": 0, \"periods\": [ { \"fromYear\": 2000, \"toYear\": 2001, \"rule\": \"NONE\" } ] } ] }",
                "{ \"zones\": [ { \"name\": \"Z\", \"standardOffsetMinutes\": 0 } ] }",
                "{ \"zones\": [ { \"name\": \"Z\", \"standardOffsetMinutes\": 900, \"periods\": [ { \"fromYear\": 2000, \"toYear\": 2001, \"rule\": \"NONE\" } ] } ] }",
                "{ \"zones\": [ { \"name\": \"Z\", \"standardOffsetMinutes\": -900, \"periods\": [ { \"fromYear\": 2000, \"toYear\": 2001, \"rule\": \"NONE\" } ] } ] }",
                "{ \"zones\": [ { \"name\": \"Z\", \"standardOffsetMinutes\": 0, \"periods\": [ { \"fromYear\": 2001, \"toYear\": 2000, \"rule\": \"NONE\" } ] } ] }",
                "{ \"zones\": [ { \"name\": \"Z\", \"standardOffsetMinutes\": 0, \"periods\": [ { \"fromYear\": 2000, \"toYear\": 2001, \"rule\": \"NoSuchRule\" } ] } ] }",
                // all or nothing: a valid zone followed by one that is not
                "{ \"zones\": [ { \"name\": \"Z/Good\", \"standardOffsetMinutes\": 60, \"periods\": [ { \"fromYear\": 2000, \"toYear\": 2001, \"rule\": \"NONE\" } ] }, "
                             "{ \"name\": \"Z/Bad\", \"standardOffsetMinutes\": 60, \"periods\": [ { \"fromYear\": 2000, \"toYear\": 2001, \"rule\": \"NoSuchRule\" } ] } ] }"
            };
            for ( UInt32 i=0; i<sizeof( invalid ) / sizeof( invalid[ 0 ] ); ++i )
            {
                error.Clear();
                if ( LoadJson( rules, invalid[ i ], error ) )
                    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "TimeZoneRules test: an invalid document is accepted: " << invalid[ i ] );
                ASSERT_FALSE( LoadJson( rules, invalid[ i ], error ) );
                ASSERT_FALSE( error.IsNULLOrEmpty() );
            }

            // nothing of the refused documents is in
            ASSERT_TRUE( 19 == rules.GetZoneCount() );
            ASSERT_FALSE( rules.HasZone( "Z/Good" ) || rules.HasZone( "Z/Bad" ) || rules.HasZone( "Z" ) );
            Int32 offset = 0;
            ASSERT_TRUE( rules.GetOffsetAtInstant( "America/New_York", Utc( 2024, 7, 1, 12, 0 ), offset ) && -240 == offset );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A zone or rule of the same name replaces the one that was there, other entries of the document are ignored, a copy is independent" )
        try
        {
            CTimeZoneRules rules;
            CString error;
            ASSERT_TRUE( LoadJson( rules, "{ \"comment\": \"text\", \"status\": \"x\", \"rules\": [ { \"name\": \"R\", \"status\": \"to-confirm\", \"periods\": [ "
                                          "{ \"fromYear\": 2024, \"toYear\": 2025, \"start\": \"Mar Sun>=8 2:00\", \"end\": \"Nov Sun>=1 2:00\", \"note\": 1 } ] } ], "
                                          "\"zones\": [ { \"name\": \"Test/Zone\", \"status\": \"iana-checked\", \"standardOffsetMinutes\": -300, "
                                          "\"periods\": [ { \"fromYear\": 2024, \"toYear\": 2025, \"rule\": \"R\" } ] } ] }", error ) );
            Int32 offset = 0;
            bool daylight = false;
            ASSERT_TRUE( rules.GetOffsetAtInstant( "Test/Zone", Utc( 2024, 7, 1, 12, 0 ), offset, &daylight ) && -240 == offset && daylight );

            const CTimeZoneRules copy( rules );
            CTimeZoneRules assigned;
            assigned = rules;

            // the same names with other values
            ASSERT_TRUE( LoadJson( rules, "{ \"rules\": [ { \"name\": \"R\", \"periods\": [ { \"fromYear\": 2024, \"toYear\": 2025, \"saveMinutes\": 30, "
                                          "\"start\": \"Mar Sun>=8 2:00\", \"end\": \"Nov Sun>=1 2:00\" } ] } ], "
                                          "\"zones\": [ { \"name\": \"Test/Zone\", \"standardOffsetMinutes\": -360, \"periods\": [ { \"fromYear\": 2024, \"toYear\": 2025, \"rule\": \"R\" } ] } ] }", error ) );
            ASSERT_TRUE( 1 == rules.GetZoneCount() );
            ASSERT_TRUE( rules.GetOffsetAtInstant( "Test/Zone", Utc( 2024, 7, 1, 12, 0 ), offset, &daylight ) && -330 == offset && daylight );
            ASSERT_TRUE( rules.GetOffsetAtInstant( "Test/Zone", Utc( 2024, 1, 1, 12, 0 ), offset, &daylight ) && -360 == offset && !daylight );

            // the copies still have the first values
            ASSERT_TRUE( copy.GetOffsetAtInstant( "Test/Zone", Utc( 2024, 7, 1, 12, 0 ), offset, &daylight ) && -240 == offset && daylight );
            ASSERT_TRUE( assigned.GetOffsetAtInstant( "Test/Zone", Utc( 2024, 7, 1, 12, 0 ), offset, &daylight ) && -240 == offset && daylight );
            rules.Clear();
            ASSERT_TRUE( copy.GetOffsetAtInstant( "Test/Zone", Utc( 2024, 1, 1, 12, 0 ), offset, &daylight ) && -300 == offset && !daylight );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformTransitionTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: The time zone rules give the transitions of the US, the EU and Australia for known years" )
        try
        {
            CTimeZoneRules zones;
            ASSERT_TRUE( LoadFixture( zones ) );

            // US: the second Sunday of March 02:00 standard time (07:00 UTC in New York), the first Sunday of November 02:00 daylight time (06:00 UTC); before 2007 April and October
            struct SYear { Int32 year; Int32 startMonth; Int32 startDay; Int32 endMonth; Int32 endDay; };
            static const SYear us[] =
            {
                { 1999, 4, 4, 10, 31 }, { 2006, 4, 2, 10, 29 }, { 2007, 3, 11, 11, 4 }, { 2012, 3, 11, 11, 4 }, { 2019, 3, 10, 11, 3 }, { 2020, 3, 8, 11, 1 },
                { 2021, 3, 14, 11, 7 }, { 2024, 3, 10, 11, 3 }, { 2025, 3, 9, 11, 2 }, { 2026, 3, 8, 11, 1 }, { 2030, 3, 10, 11, 3 }
            };
            for ( UInt32 i=0; i<sizeof( us ) / sizeof( us[ 0 ] ); ++i )
            {
                bool has = false;
                Int64 start = 0, end = 0;
                ASSERT_TRUE( zones.GetTransitionsOfYear( "America/New_York", us[ i ].year, has, start, end ) && has );
                ASSERT_TRUE( start == Utc( us[ i ].year, us[ i ].startMonth, us[ i ].startDay, 7, 0 ) );
                ASSERT_TRUE( end == Utc( us[ i ].year, us[ i ].endMonth, us[ i ].endDay, 6, 0 ) );
                ASSERT_TRUE( zones.GetTransitionsOfYear( "America/Chicago", us[ i ].year, has, start, end ) && start == Utc( us[ i ].year, us[ i ].startMonth, us[ i ].startDay, 8, 0 ) );
            }

            // EU: the last Sunday of March and of October at 01:00 UTC, for every zone of the rule
            static const SYear eu[] =
            {
                { 1996, 3, 31, 10, 27 }, { 2000, 3, 26, 10, 29 }, { 2012, 3, 25, 10, 28 }, { 2021, 3, 28, 10, 31 }, { 2024, 3, 31, 10, 27 }, { 2025, 3, 30, 10, 26 }, { 2026, 3, 29, 10, 25 }
            };
            static const char* const euZones[] = { "Europe/Paris", "Europe/London", "Europe/Berlin", "Europe/Amsterdam" };
            for ( UInt32 i=0; i<sizeof( eu ) / sizeof( eu[ 0 ] ); ++i )
            {
                for ( UInt32 z=0; z<4; ++z )
                {
                    bool has = false;
                    Int64 start = 0, end = 0;
                    ASSERT_TRUE( zones.GetTransitionsOfYear( euZones[ z ], eu[ i ].year, has, start, end ) && has );
                    ASSERT_TRUE( start == Utc( eu[ i ].year, eu[ i ].startMonth, eu[ i ].startDay, 1, 0 ) );
                    ASSERT_TRUE( end == Utc( eu[ i ].year, eu[ i ].endMonth, eu[ i ].endDay, 1, 0 ) );
                }
            }

            // Sydney: the first Sunday of October and of April at 02:00 standard time (16:00 UTC the day before): southern summer
            static const SYear au[] = { { 2008, 10, 4, 4, 5 }, { 2020, 10, 3, 4, 4 }, { 2024, 10, 5, 4, 6 }, { 2025, 10, 4, 4, 5 } };      // the UTC day of the instant
            for ( UInt32 i=0; i<sizeof( au ) / sizeof( au[ 0 ] ); ++i )
            {
                bool has = false;
                Int64 start = 0, end = 0;
                ASSERT_TRUE( zones.GetTransitionsOfYear( "Australia/Sydney", au[ i ].year, has, start, end ) && has );
                ASSERT_TRUE( start == Utc( au[ i ].year, au[ i ].startMonth, au[ i ].startDay, 16, 0 ) );
                ASSERT_TRUE( end == Utc( au[ i ].year, au[ i ].endMonth, au[ i ].endDay, 16, 0 ) );
            }

            // offsets in force
            Int32 offset = 0;
            bool daylight = false;
            ASSERT_TRUE( zones.GetOffsetAtInstant( "America/New_York", Utc( 2024, 3, 10, 6, 59 ), offset, &daylight ) && -300 == offset && !daylight );
            ASSERT_TRUE( zones.GetOffsetAtInstant( "America/New_York", Utc( 2024, 3, 10, 7, 0 ), offset, &daylight ) && -240 == offset && daylight );
            ASSERT_TRUE( zones.GetOffsetAtInstant( "America/New_York", Utc( 2024, 11, 3, 5, 59 ), offset, &daylight ) && -240 == offset && daylight );
            ASSERT_TRUE( zones.GetOffsetAtInstant( "America/New_York", Utc( 2024, 11, 3, 6, 0 ), offset, &daylight ) && -300 == offset && !daylight );
            ASSERT_TRUE( zones.GetOffsetAtInstant( "Europe/Paris", Utc( 2024, 3, 31, 0, 59 ), offset, &daylight ) && 60 == offset && !daylight );
            ASSERT_TRUE( zones.GetOffsetAtInstant( "Europe/Paris", Utc( 2024, 3, 31, 1, 0 ), offset, &daylight ) && 120 == offset && daylight );
            ASSERT_TRUE( zones.GetOffsetAtInstant( "Europe/London", Utc( 2024, 7, 1, 12, 0 ), offset, &daylight ) && 60 == offset && daylight );
            ASSERT_TRUE( zones.GetOffsetAtInstant( "Australia/Sydney", Utc( 2024, 1, 15, 0, 0 ), offset, &daylight ) && 660 == offset && daylight );       // southern summer
            ASSERT_TRUE( zones.GetOffsetAtInstant( "Australia/Sydney", Utc( 2024, 7, 15, 0, 0 ), offset, &daylight ) && 600 == offset && !daylight );
            ASSERT_TRUE( zones.GetOffsetAtInstant( "Australia/Sydney", Utc( 2024, 4, 6, 15, 59 ), offset, &daylight ) && 660 == offset );
            ASSERT_TRUE( zones.GetOffsetAtInstant( "Australia/Sydney", Utc( 2024, 4, 6, 16, 0 ), offset, &daylight ) && 600 == offset );
            ASSERT_FALSE( zones.GetOffsetAtInstant( "Nowhere/Nothing", Utc( 2024, 1, 1, 0, 0 ), offset ) );

            // a second inside a minute is in that minute
            ASSERT_TRUE( zones.GetOffsetAtInstant( "America/New_York", Utc( 2024, 3, 10, 6, 59 ) + 59, offset ) && -300 == offset );
            ASSERT_TRUE( zones.GetOffsetAtInstant( "America/New_York", Utc( 2024, 3, 10, 7, 0 ) + 1, offset ) && -240 == offset );
            ASSERT_TRUE( zones.GetOffsetAtInstant( "America/New_York", Utc( 2024, 3, 10, 7, 0 ) - 1, offset ) && -300 == offset );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The offset changes exactly at the transitions of every year, in the northern and the southern hemisphere" )
        try
        {
            CTimeZoneRules zones;
            ASSERT_TRUE( LoadFixture( zones ) );

            static const char* const names[] = { "America/New_York", "America/Chicago", "Europe/London", "Europe/Paris", "Australia/Sydney" };
            for ( UInt32 z=0; z<sizeof( names ) / sizeof( names[ 0 ] ); ++z )
            {
                Int32 standard = 0;
                ASSERT_TRUE( zones.GetStandardOffsetMinutes( names[ z ], standard ) );
                for ( Int32 year = 2008; year <= 2040; ++year )
                {
                    bool has = false;
                    Int64 start = 0, end = 0;
                    ASSERT_TRUE( zones.GetTransitionsOfYear( names[ z ], year, has, start, end ) && has );

                    Int32 offset = 0;
                    bool daylight = false;
                    ASSERT_TRUE( zones.GetOffsetAtInstant( names[ z ], start - 1, offset, &daylight ) && standard == offset && !daylight );
                    ASSERT_TRUE( zones.GetOffsetAtInstant( names[ z ], start, offset, &daylight ) && standard + 60 == offset && daylight );
                    ASSERT_TRUE( zones.GetOffsetAtInstant( names[ z ], end - 1, offset, &daylight ) && standard + 60 == offset && daylight );
                    ASSERT_TRUE( zones.GetOffsetAtInstant( names[ z ], end, offset, &daylight ) && standard == offset && !daylight );
                }
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Zones without daylight saving have none, with the years their rule is valid for as data" )
        try
        {
            CTimeZoneRules zones;
            ASSERT_TRUE( LoadFixture( zones ) );

            static const char* const none[] = { "America/Sao_Paulo", "America/Argentina/Buenos_Aires", "America/Bogota", "America/Lima", "Asia/Tokyo", "Asia/Hong_Kong", "Asia/Shanghai", "Asia/Singapore", "Asia/Kolkata" };
            for ( UInt32 z=0; z<sizeof( none ) / sizeof( none[ 0 ] ); ++z )
            {
                Int32 standard = 0;
                ASSERT_TRUE( zones.GetStandardOffsetMinutes( none[ z ], standard ) );
                for ( Int32 year = 2020; year <= 2035; year += 3 )
                {
                    bool has = true;
                    Int64 start = 0, end = 0;
                    ASSERT_TRUE( zones.GetTransitionsOfYear( none[ z ], year, has, start, end ) && !has );
                    Int32 offset = 0;
                    bool daylight = true;
                    ASSERT_TRUE( zones.GetOffsetAtInstant( none[ z ], Utc( year, 1, 15, 12, 0 ), offset, &daylight ) && standard == offset && !daylight );
                    ASSERT_TRUE( zones.GetOffsetAtInstant( none[ z ], Utc( year, 7, 15, 12, 0 ), offset, &daylight ) && standard == offset && !daylight );
                }
            }
            Int32 standard = 0;
            ASSERT_TRUE( zones.GetStandardOffsetMinutes( "America/Sao_Paulo", standard ) && -180 == standard );
            ASSERT_TRUE( zones.GetStandardOffsetMinutes( "Asia/Kolkata", standard ) && 330 == standard );

            // the rule of Sao Paulo is valid from 2020: it had daylight saving until early 2019, which the data does not hold, so those years are unknown and not guessed
            Int32 offset = 0;
            ASSERT_FALSE( zones.GetOffsetAtInstant( "America/Sao_Paulo", Utc( 2018, 12, 1, 12, 0 ), offset ) );
            ASSERT_FALSE( zones.GetOffsetAtInstant( "America/Sao_Paulo", Utc( 2019, 7, 1, 12, 0 ), offset ) );
            ASSERT_TRUE( zones.GetOffsetAtInstant( "America/Sao_Paulo", Utc( 2020, 1, 1, 12, 0 ), offset ) && -180 == offset );
            bool has = false;
            Int64 start = 0, end = 0;
            ASSERT_FALSE( zones.GetTransitionsOfYear( "America/Sao_Paulo", 2019, has, start, end ) );
            ASSERT_FALSE( has );
            Int32 day = 0;
            ASSERT_FALSE( zones.GetLocalDay( "America/Sao_Paulo", Utc( 2019, 7, 1, 12, 0 ), day ) );
            ASSERT_FALSE( zones.GetLocalDayStart( "America/Sao_Paulo", DayOf( 2019, 7, 1 ), start ) );

            // New York has rules from 1987 on
            ASSERT_TRUE( zones.GetOffsetAtInstant( "America/New_York", Utc( 1987, 1, 1, 12, 0 ), offset ) && -300 == offset );
            ASSERT_FALSE( zones.GetOffsetAtInstant( "America/New_York", Utc( 1986, 7, 1, 12, 0 ), offset ) );

            // Mexico City: daylight saving until 2022, none after
            ASSERT_TRUE( zones.GetTransitionsOfYear( "America/Mexico_City", 2022, has, start, end ) && has );
            ASSERT_TRUE( start == Utc( 2022, 4, 3, 8, 0 ) && end == Utc( 2022, 10, 30, 7, 0 ) );
            ASSERT_TRUE( zones.GetTransitionsOfYear( "America/Mexico_City", 2023, has, start, end ) && !has );
            ASSERT_TRUE( zones.GetOffsetAtInstant( "America/Mexico_City", Utc( 2023, 7, 1, 12, 0 ), offset ) && -360 == offset );
            ASSERT_TRUE( zones.GetOffsetAtInstant( "America/Mexico_City", Utc( 2022, 7, 1, 12, 0 ), offset ) && -300 == offset );
            ASSERT_FALSE( zones.GetOffsetAtInstant( "America/Mexico_City", Utc( 2001, 7, 1, 12, 0 ), offset ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A transition can be written as a fixed day, a weekday on or after or on or before a day, the last weekday, in wall, standard or UTC time, up to 24:00" )
        try
        {
            CTimeZoneRules zones;
            CString error;
            bool has = false;
            Int64 start = 0, end = 0;
            Int32 offset = 0;
            bool daylight = false;

            // a fixed day: the start in standard time (the zone is on UTC), the end in wall time of the daylight offset
            ASSERT_TRUE( LoadJson( zones, OneZoneDocument( "Test/Fixed", 0, 60, "Apr 15 0:00s", "Oct 20 0:00" ), error ) );
            ASSERT_TRUE( zones.GetTransitionsOfYear( "Test/Fixed", 2024, has, start, end ) && has );
            ASSERT_TRUE( start == Utc( 2024, 4, 15, 0, 0 ) && end == Utc( 2024, 10, 19, 23, 0 ) );

            // the weekday on or before: 25 March 2024 was a Monday, so the Sunday on or before it is the 24th; the start in wall time of the standard offset of 90 minutes
            // east of UTC, the end in UTC; a daylight saving of 30 minutes
            zones.Clear();
            ASSERT_TRUE( LoadJson( zones, OneZoneDocument( "Test/Before", 90, 30, "Mar Sun<=25 2:00", "Oct lastSun 2:00u" ), error ) );
            ASSERT_TRUE( zones.GetTransitionsOfYear( "Test/Before", 2024, has, start, end ) && has );
            ASSERT_TRUE( start == Utc( 2024, 3, 24, 0, 30 ) && end == Utc( 2024, 10, 27, 2, 0 ) );
            ASSERT_TRUE( zones.GetTransitionsOfYear( "Test/Before", 2025, has, start, end ) && has );
            ASSERT_TRUE( start == Utc( 2025, 3, 23, 0, 30 ) && end == Utc( 2025, 10, 26, 2, 0 ) );
            ASSERT_TRUE( zones.GetOffsetAtInstant( "Test/Before", Utc( 2024, 6, 1, 0, 0 ), offset, &daylight ) && 120 == offset && daylight );
            ASSERT_TRUE( zones.GetOffsetAtInstant( "Test/Before", Utc( 2024, 1, 1, 0, 0 ), offset, &daylight ) && 90 == offset && !daylight );

            // 24:00 is the midnight that ends the day: the start in wall time of the standard offset of an hour west of UTC, the end in wall time of the daylight offset
            zones.Clear();
            ASSERT_TRUE( LoadJson( zones, OneZoneDocument( "Test/Midnight", -60, 60, "Mar lastSun 24:00", "Oct lastSun 24:00" ), error ) );
            ASSERT_TRUE( zones.GetTransitionsOfYear( "Test/Midnight", 2024, has, start, end ) && has );
            ASSERT_TRUE( start == Utc( 2024, 4, 1, 1, 0 ) && end == Utc( 2024, 10, 28, 0, 0 ) );

            // the weekday on or after, the last weekday of a month that ends on that weekday, in the month of February of a leap year and of a common year
            zones.Clear();
            ASSERT_TRUE( LoadJson( zones, OneZoneDocument( "Test/After", 0, 60, "Feb Sun>=22 1:00u", "Nov lastThu 1:00u" ), error ) );
            ASSERT_TRUE( zones.GetTransitionsOfYear( "Test/After", 2024, has, start, end ) && has );
            ASSERT_TRUE( start == Utc( 2024, 2, 25, 1, 0 ) && end == Utc( 2024, 11, 28, 1, 0 ) );
            ASSERT_TRUE( zones.GetTransitionsOfYear( "Test/After", 2025, has, start, end ) && has );
            ASSERT_TRUE( start == Utc( 2025, 2, 23, 1, 0 ) && end == Utc( 2025, 11, 27, 1, 0 ) );
            zones.Clear();
            ASSERT_TRUE( LoadJson( zones, OneZoneDocument( "Test/Leap", 0, 60, "Feb lastSun 1:00u", "Feb 29 12:00u" ), error ) );
            ASSERT_TRUE( zones.GetTransitionsOfYear( "Test/Leap", 2024, has, start, end ) && has );
            ASSERT_TRUE( start == Utc( 2024, 2, 25, 1, 0 ) && end == Utc( 2024, 2, 29, 12, 0 ) );

            // lower case, case does not matter in the names
            zones.Clear();
            ASSERT_TRUE( LoadJson( zones, OneZoneDocument( "Test/Case", 0, 60, "mar LASTsun 1:00u", "OCT lastSUN 1:00u" ), error ) );
            ASSERT_TRUE( zones.GetTransitionsOfYear( "Test/Case", 2024, has, start, end ) && has );
            ASSERT_TRUE( start == Utc( 2024, 3, 31, 1, 0 ) && end == Utc( 2024, 10, 27, 1, 0 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformLocalDayTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: A local day starts at local midnight, and the days of a transition are 23 and 25 hours long" )
        try
        {
            CTimeZoneRules zones;
            ASSERT_TRUE( LoadFixture( zones ) );
            Int64 start = 0, end = 0;

            struct SDay { const char* zone; Int32 year; Int32 month; Int32 day; Int64 hours; Int64 startUtc; };
            static const SDay days[] =
            {
                { "America/New_York", 2024, 3, 9, 24, Utc( 2024, 3, 9, 5, 0 ) },
                { "America/New_York", 2024, 3, 10, 23, Utc( 2024, 3, 10, 5, 0 ) },          // the day the clocks go forward: midnight EST
                { "America/New_York", 2024, 3, 11, 24, Utc( 2024, 3, 11, 4, 0 ) },          // midnight EDT
                { "America/New_York", 2024, 11, 3, 25, Utc( 2024, 11, 3, 4, 0 ) },          // the day the clocks go back: midnight EDT
                { "America/New_York", 2024, 11, 4, 24, Utc( 2024, 11, 4, 5, 0 ) },
                { "Europe/Paris", 2024, 3, 31, 23, Utc( 2024, 3, 30, 23, 0 ) },
                { "Europe/Paris", 2024, 10, 27, 25, Utc( 2024, 10, 26, 22, 0 ) },
                { "Europe/London", 2025, 3, 30, 23, Utc( 2025, 3, 30, 0, 0 ) },
                { "Europe/London", 2025, 10, 26, 25, Utc( 2025, 10, 25, 23, 0 ) },
                { "Australia/Sydney", 2024, 10, 6, 23, Utc( 2024, 10, 5, 14, 0 ) },        // midnight AEST, the day goes forward
                { "Australia/Sydney", 2024, 4, 7, 25, Utc( 2024, 4, 6, 13, 0 ) },          // midnight AEDT
                { "America/Sao_Paulo", 2024, 3, 10, 24, Utc( 2024, 3, 10, 3, 0 ) },
                { "America/Sao_Paulo", 2024, 10, 20, 24, Utc( 2024, 10, 20, 3, 0 ) },
                { "Asia/Kolkata", 2024, 6, 1, 24, Utc( 2024, 5, 31, 18, 30 ) }
            };
            for ( UInt32 i=0; i<sizeof( days ) / sizeof( days[ 0 ] ); ++i )
            {
                const Int32 day = DayOf( days[ i ].year, days[ i ].month, days[ i ].day );
                ASSERT_TRUE( zones.GetLocalDayBounds( days[ i ].zone, day, start, end ) );
                ASSERT_TRUE( start == days[ i ].startUtc );
                ASSERT_TRUE( end - start == days[ i ].hours * SECONDS_PER_HOUR );
                Int32 local = 0;
                ASSERT_TRUE( zones.GetLocalDay( days[ i ].zone, start, local ) && local == day );
                ASSERT_TRUE( zones.GetLocalDay( days[ i ].zone, end - 1, local ) && local == day );
                ASSERT_TRUE( zones.GetLocalDay( days[ i ].zone, end, local ) && local == day + 1 );
                ASSERT_TRUE( zones.GetLocalDay( days[ i ].zone, start - 1, local ) && local == day - 1 );
                Int64 dayStart = 0;
                ASSERT_TRUE( zones.GetLocalDayStart( days[ i ].zone, day, dayStart ) && dayStart == start );
            }

            // 20:00 in New York in winter is 01:00 UTC of the next day: the local date is still the day before
            Int32 local = 0;
            ASSERT_TRUE( zones.GetLocalDay( "America/New_York", Utc( 2024, 1, 13, 1, 0 ), local ) && local == DayOf( 2024, 1, 12 ) );
            ASSERT_TRUE( zones.GetLocalDay( "America/New_York", Utc( 2024, 1, 13, 4, 59 ), local ) && local == DayOf( 2024, 1, 12 ) );
            ASSERT_TRUE( zones.GetLocalDay( "America/New_York", Utc( 2024, 1, 13, 5, 0 ), local ) && local == DayOf( 2024, 1, 13 ) );
            // and in summer the same wall clock time is an hour earlier in UTC
            ASSERT_TRUE( zones.GetLocalDay( "America/New_York", Utc( 2024, 7, 13, 3, 59 ), local ) && local == DayOf( 2024, 7, 12 ) );
            ASSERT_TRUE( zones.GetLocalDay( "America/New_York", Utc( 2024, 7, 13, 4, 0 ), local ) && local == DayOf( 2024, 7, 13 ) );

            // the date of the instant, before the epoch as well
            CDate date;
            ASSERT_TRUE( zones.GetLocalDate( "America/New_York", Utc( 2024, 1, 13, 4, 59 ), date ) && 2024 == date.GetYear() && 1 == date.GetMonth() && 12 == date.GetDay() );
            ASSERT_TRUE( zones.GetLocalDate( "America/New_York", Utc( 1999, 7, 1, 3, 0 ), date ) && 1999 == date.GetYear() && 6 == date.GetMonth() && 30 == date.GetDay() );
            ASSERT_FALSE( zones.GetLocalDate( "Nowhere/Nothing", Utc( 2024, 1, 13, 4, 59 ), date ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A day that has no midnight starts at the instant of the transition that skips it" )
        try
        {
            // a zone whose clocks go forward at midnight: 00:00 does not exist on that day, the day starts at 01:00 (the transition), and is 23 hours long
            CTimeZoneRules zones;
            CString error;
            ASSERT_TRUE( LoadJson( zones, OneZoneDocument( "Test/Midnight", 0, 60, "Mar Sun>=8 0:00", "Nov Sun>=1 0:00" ), error ) );
            bool has = false;
            Int64 start = 0, end = 0, dayStart = 0, dayEnd = 0;
            ASSERT_TRUE( zones.GetTransitionsOfYear( "Test/Midnight", 2024, has, start, end ) && has );
            ASSERT_TRUE( start == Utc( 2024, 3, 10, 0, 0 ) );
            ASSERT_TRUE( zones.GetLocalDayBounds( "Test/Midnight", DayOf( 2024, 3, 10 ), dayStart, dayEnd ) );
            ASSERT_TRUE( dayStart == Utc( 2024, 3, 10, 0, 0 ) && dayEnd - dayStart == 23 * SECONDS_PER_HOUR );
            Int32 day = 0;
            ASSERT_TRUE( zones.GetLocalDay( "Test/Midnight", dayStart, day ) && day == DayOf( 2024, 3, 10 ) );
            ASSERT_TRUE( zones.GetLocalDay( "Test/Midnight", dayStart - 1, day ) && day == DayOf( 2024, 3, 9 ) );
            ASSERT_TRUE( zones.GetLocalDay( "Test/Midnight", dayEnd - 1, day ) && day == DayOf( 2024, 3, 10 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Random instants of the zones are on exactly one local day, whose bounds contain them" )
        try
        {
            CTimeZoneRules zones;
            ASSERT_TRUE( LoadFixture( zones ) );
            static const char* const names[] = { "America/New_York", "America/Chicago", "America/Toronto", "America/Mexico_City", "America/Sao_Paulo", "Europe/London", "Europe/Paris", "Europe/Zurich", "Asia/Tokyo", "Asia/Kolkata", "Australia/Sydney" };
            CRandom random( 20240308 );
            for ( UInt32 z=0; z<sizeof( names ) / sizeof( names[ 0 ] ); ++z )
            {
                for ( UInt32 n=0; n<500; ++n )
                {
                    const Int64 instant = random.Between( DayOf( 2023, 1, 1 ), DayOf( 2040, 12, 31 ) ) * 86400LL + random.Between( 0, 86399 );
                    Int32 day = 0;
                    Int64 start = 0, end = 0;
                    ASSERT_TRUE( zones.GetLocalDay( names[ z ], instant, day ) );
                    ASSERT_TRUE( zones.GetLocalDayBounds( names[ z ], day, start, end ) );
                    ASSERT_TRUE( start <= instant && instant < end );
                    const Int64 hours = ( end - start ) / SECONDS_PER_HOUR;
                    ASSERT_TRUE( 23 == hours || 24 == hours || 25 == hours );
                    Int32 neighbour = 0;
                    ASSERT_TRUE( zones.GetLocalDay( names[ z ], start - 1, neighbour ) && neighbour == day - 1 );
                    ASSERT_TRUE( zones.GetLocalDay( names[ z ], end, neighbour ) && neighbour == day + 1 );
                }
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformLocalTimeTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: A local time that does not occur is read with the offset before the transition, after it or refused, as the policy says" )
        try
        {
            CTimeZoneRules zones;
            ASSERT_TRUE( LoadFixture( zones ) );
            Int64 utc = 0;
            CTimeZoneRules::TLocalTimeKind kind = CTimeZoneRules::LOCALTIME_UNIQUE;

            struct SGap { const char* zone; Int64 local; Int64 forward; Int64 backward; };
            static const SGap gaps[] =
            {
                // 02:30 on the day the clocks go forward: 02:00 EST becomes 03:00 EDT at 07:00 UTC
                { "America/New_York", Utc( 2024, 3, 10, 2, 30 ), Utc( 2024, 3, 10, 7, 30 ), Utc( 2024, 3, 10, 6, 30 ) },
                { "America/Chicago", Utc( 2024, 3, 10, 2, 30 ), Utc( 2024, 3, 10, 8, 30 ), Utc( 2024, 3, 10, 7, 30 ) },
                // 02:00 CET becomes 03:00 CEST at 01:00 UTC
                { "Europe/Paris", Utc( 2024, 3, 31, 2, 30 ), Utc( 2024, 3, 31, 1, 30 ), Utc( 2024, 3, 31, 0, 30 ) },
                // 01:00 GMT becomes 02:00 BST at 01:00 UTC
                { "Europe/London", Utc( 2025, 3, 30, 1, 30 ), Utc( 2025, 3, 30, 1, 30 ), Utc( 2025, 3, 30, 0, 30 ) },
                // 02:00 AEST becomes 03:00 AEDT at 16:00 UTC the day before
                { "Australia/Sydney", Utc( 2024, 10, 6, 2, 30 ), Utc( 2024, 10, 5, 16, 30 ), Utc( 2024, 10, 5, 15, 30 ) },
                { "America/Mexico_City", Utc( 2022, 4, 3, 2, 30 ), Utc( 2022, 4, 3, 8, 30 ), Utc( 2022, 4, 3, 7, 30 ) }
            };
            for ( UInt32 i=0; i<sizeof( gaps ) / sizeof( gaps[ 0 ] ); ++i )
            {
                ASSERT_TRUE( zones.LocalToUtc( gaps[ i ].zone, gaps[ i ].local, CTimeZoneRules::AMBIGUOUSTIME_REJECT, CTimeZoneRules::NONEXISTENTTIME_SHIFT_FORWARD, utc, &kind ) );
                ASSERT_TRUE( utc == gaps[ i ].forward && CTimeZoneRules::LOCALTIME_NONEXISTENT == kind );
                ASSERT_TRUE( zones.LocalToUtc( gaps[ i ].zone, gaps[ i ].local, CTimeZoneRules::AMBIGUOUSTIME_REJECT, CTimeZoneRules::NONEXISTENTTIME_SHIFT_BACKWARD, utc, &kind ) );
                ASSERT_TRUE( utc == gaps[ i ].backward && CTimeZoneRules::LOCALTIME_NONEXISTENT == kind );
                kind = CTimeZoneRules::LOCALTIME_UNIQUE;
                ASSERT_FALSE( zones.LocalToUtc( gaps[ i ].zone, gaps[ i ].local, CTimeZoneRules::AMBIGUOUSTIME_EARLIER, CTimeZoneRules::NONEXISTENTTIME_REJECT, utc, &kind ) );
                ASSERT_TRUE( CTimeZoneRules::LOCALTIME_NONEXISTENT == kind );

                CTimeZoneRules::TLocalTimeKind asked = CTimeZoneRules::LOCALTIME_UNIQUE;
                ASSERT_TRUE( zones.GetLocalTimeKind( gaps[ i ].zone, gaps[ i ].local, asked ) && CTimeZoneRules::LOCALTIME_NONEXISTENT == asked );
            }

            // the seconds of the local time are kept
            ASSERT_TRUE( zones.LocalToUtc( "America/New_York", Utc( 2024, 3, 10, 2, 30 ) + 17, CTimeZoneRules::AMBIGUOUSTIME_REJECT, CTimeZoneRules::NONEXISTENTTIME_SHIFT_FORWARD, utc ) && utc == Utc( 2024, 3, 10, 7, 30 ) + 17 );

            // the edges of the gap: 01:59:59 exists, 02:00:00 does not, 02:59:59 does not, 03:00:00 does
            ASSERT_TRUE( zones.LocalToUtc( "America/New_York", Utc( 2024, 3, 10, 1, 59 ) + 59, CTimeZoneRules::AMBIGUOUSTIME_REJECT, CTimeZoneRules::NONEXISTENTTIME_REJECT, utc, &kind ) && CTimeZoneRules::LOCALTIME_UNIQUE == kind && utc == Utc( 2024, 3, 10, 6, 59 ) + 59 );
            ASSERT_FALSE( zones.LocalToUtc( "America/New_York", Utc( 2024, 3, 10, 2, 0 ), CTimeZoneRules::AMBIGUOUSTIME_REJECT, CTimeZoneRules::NONEXISTENTTIME_REJECT, utc ) );
            ASSERT_FALSE( zones.LocalToUtc( "America/New_York", Utc( 2024, 3, 10, 2, 59 ) + 59, CTimeZoneRules::AMBIGUOUSTIME_REJECT, CTimeZoneRules::NONEXISTENTTIME_REJECT, utc ) );
            ASSERT_TRUE( zones.LocalToUtc( "America/New_York", Utc( 2024, 3, 10, 3, 0 ), CTimeZoneRules::AMBIGUOUSTIME_REJECT, CTimeZoneRules::NONEXISTENTTIME_REJECT, utc, &kind ) && CTimeZoneRules::LOCALTIME_UNIQUE == kind && utc == Utc( 2024, 3, 10, 7, 0 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A local time that occurs twice is read as the first or the second occurrence, or refused, as the policy says" )
        try
        {
            CTimeZoneRules zones;
            ASSERT_TRUE( LoadFixture( zones ) );
            Int64 utc = 0;
            CTimeZoneRules::TLocalTimeKind kind = CTimeZoneRules::LOCALTIME_UNIQUE;

            struct SOverlap { const char* zone; Int64 local; Int64 earlier; Int64 later; };
            static const SOverlap overlaps[] =
            {
                // 02:00 EDT becomes 01:00 EST at 06:00 UTC: 01:30 is 05:30 UTC (EDT) and 06:30 UTC (EST)
                { "America/New_York", Utc( 2024, 11, 3, 1, 30 ), Utc( 2024, 11, 3, 5, 30 ), Utc( 2024, 11, 3, 6, 30 ) },
                { "America/New_York", Utc( 2024, 11, 3, 1, 0 ), Utc( 2024, 11, 3, 5, 0 ), Utc( 2024, 11, 3, 6, 0 ) },
                { "America/New_York", Utc( 2024, 11, 3, 1, 59 ) + 59, Utc( 2024, 11, 3, 5, 59 ) + 59, Utc( 2024, 11, 3, 6, 59 ) + 59 },
                // 03:00 CEST becomes 02:00 CET at 01:00 UTC
                { "Europe/Paris", Utc( 2024, 10, 27, 2, 30 ), Utc( 2024, 10, 27, 0, 30 ), Utc( 2024, 10, 27, 1, 30 ) },
                // 02:00 BST becomes 01:00 GMT at 01:00 UTC
                { "Europe/London", Utc( 2025, 10, 26, 1, 30 ), Utc( 2025, 10, 26, 0, 30 ), Utc( 2025, 10, 26, 1, 30 ) },
                // 03:00 AEDT becomes 02:00 AEST at 16:00 UTC the day before
                { "Australia/Sydney", Utc( 2024, 4, 7, 2, 30 ), Utc( 2024, 4, 6, 15, 30 ), Utc( 2024, 4, 6, 16, 30 ) }
            };
            for ( UInt32 i=0; i<sizeof( overlaps ) / sizeof( overlaps[ 0 ] ); ++i )
            {
                ASSERT_TRUE( zones.LocalToUtc( overlaps[ i ].zone, overlaps[ i ].local, CTimeZoneRules::AMBIGUOUSTIME_EARLIER, CTimeZoneRules::NONEXISTENTTIME_REJECT, utc, &kind ) );
                ASSERT_TRUE( utc == overlaps[ i ].earlier && CTimeZoneRules::LOCALTIME_AMBIGUOUS == kind );
                ASSERT_TRUE( zones.LocalToUtc( overlaps[ i ].zone, overlaps[ i ].local, CTimeZoneRules::AMBIGUOUSTIME_LATER, CTimeZoneRules::NONEXISTENTTIME_REJECT, utc, &kind ) );
                ASSERT_TRUE( utc == overlaps[ i ].later && CTimeZoneRules::LOCALTIME_AMBIGUOUS == kind );
                kind = CTimeZoneRules::LOCALTIME_UNIQUE;
                ASSERT_FALSE( zones.LocalToUtc( overlaps[ i ].zone, overlaps[ i ].local, CTimeZoneRules::AMBIGUOUSTIME_REJECT, CTimeZoneRules::NONEXISTENTTIME_SHIFT_FORWARD, utc, &kind ) );
                ASSERT_TRUE( CTimeZoneRules::LOCALTIME_AMBIGUOUS == kind );

                CTimeZoneRules::TLocalTimeKind asked = CTimeZoneRules::LOCALTIME_UNIQUE;
                ASSERT_TRUE( zones.GetLocalTimeKind( overlaps[ i ].zone, overlaps[ i ].local, asked ) && CTimeZoneRules::LOCALTIME_AMBIGUOUS == asked );

                // both occurrences are the same local time
                CDateTime local;
                ASSERT_TRUE( zones.UtcToLocal( overlaps[ i ].zone, overlaps[ i ].earlier, local ) );
                const Int16 earlierOffset = local.GetTimeZoneUTCOffsetInMins();
                ASSERT_TRUE( zones.UtcToLocal( overlaps[ i ].zone, overlaps[ i ].later, local ) );
                ASSERT_TRUE( earlierOffset == local.GetTimeZoneUTCOffsetInMins() + 60 );
            }

            // the edges of the overlap: 00:59:59 and 02:00:00 occur once, 01:00:00 and 01:59:59 twice
            ASSERT_TRUE( zones.LocalToUtc( "America/New_York", Utc( 2024, 11, 3, 0, 59 ) + 59, CTimeZoneRules::AMBIGUOUSTIME_REJECT, CTimeZoneRules::NONEXISTENTTIME_REJECT, utc, &kind ) && CTimeZoneRules::LOCALTIME_UNIQUE == kind && utc == Utc( 2024, 11, 3, 4, 59 ) + 59 );
            ASSERT_TRUE( zones.LocalToUtc( "America/New_York", Utc( 2024, 11, 3, 2, 0 ), CTimeZoneRules::AMBIGUOUSTIME_REJECT, CTimeZoneRules::NONEXISTENTTIME_REJECT, utc, &kind ) && CTimeZoneRules::LOCALTIME_UNIQUE == kind && utc == Utc( 2024, 11, 3, 7, 0 ) );
            ASSERT_FALSE( zones.LocalToUtc( "America/New_York", Utc( 2024, 11, 3, 1, 0 ), CTimeZoneRules::AMBIGUOUSTIME_REJECT, CTimeZoneRules::NONEXISTENTTIME_REJECT, utc ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A local time of an ordinary day is one instant, in zones with and without daylight saving, with half hour offsets, and the policies do not matter" )
        try
        {
            CTimeZoneRules zones;
            ASSERT_TRUE( LoadFixture( zones ) );
            Int64 utc = 0;
            CTimeZoneRules::TLocalTimeKind kind = CTimeZoneRules::LOCALTIME_AMBIGUOUS;

            struct SOrdinary { const char* zone; Int64 local; Int64 expected; };
            static const SOrdinary ordinary[] =
            {
                { "America/New_York", Utc( 2024, 1, 15, 9, 30 ), Utc( 2024, 1, 15, 14, 30 ) },
                { "America/New_York", Utc( 2024, 7, 15, 9, 30 ), Utc( 2024, 7, 15, 13, 30 ) },
                { "Europe/Paris", Utc( 2024, 7, 15, 9, 0 ), Utc( 2024, 7, 15, 7, 0 ) },
                { "Asia/Kolkata", Utc( 2024, 6, 1, 12, 0 ), Utc( 2024, 6, 1, 6, 30 ) },
                { "Asia/Kolkata", Utc( 2024, 6, 1, 0, 0 ), Utc( 2024, 5, 31, 18, 30 ) },
                { "Australia/Sydney", Utc( 2024, 1, 15, 10, 0 ), Utc( 2024, 1, 14, 23, 0 ) },
                { "Australia/Sydney", Utc( 2024, 7, 15, 10, 0 ), Utc( 2024, 7, 15, 0, 0 ) },
                { "America/Sao_Paulo", Utc( 2024, 7, 15, 10, 0 ), Utc( 2024, 7, 15, 13, 0 ) },
                // before the epoch is fine for a zone that has rules for those years
                { "America/New_York", Utc( 1990, 7, 15, 9, 30 ), Utc( 1990, 7, 15, 13, 30 ) }
            };
            for ( UInt32 i=0; i<sizeof( ordinary ) / sizeof( ordinary[ 0 ] ); ++i )
            {
                ASSERT_TRUE( zones.LocalToUtc( ordinary[ i ].zone, ordinary[ i ].local, CTimeZoneRules::AMBIGUOUSTIME_REJECT, CTimeZoneRules::NONEXISTENTTIME_REJECT, utc, &kind ) );
                ASSERT_TRUE( utc == ordinary[ i ].expected && CTimeZoneRules::LOCALTIME_UNIQUE == kind );
            }

            // refused: an unknown zone, years the zone has no rule for
            ASSERT_FALSE( zones.LocalToUtc( "Nowhere/Nothing", Utc( 2024, 1, 15, 9, 30 ), CTimeZoneRules::AMBIGUOUSTIME_EARLIER, CTimeZoneRules::NONEXISTENTTIME_SHIFT_FORWARD, utc ) );
            ASSERT_FALSE( zones.LocalToUtc( "America/Sao_Paulo", Utc( 2019, 7, 1, 10, 0 ), CTimeZoneRules::AMBIGUOUSTIME_EARLIER, CTimeZoneRules::NONEXISTENTTIME_SHIFT_FORWARD, utc ) );
            ASSERT_FALSE( zones.LocalToUtc( "America/New_York", Utc( 1986, 7, 1, 10, 0 ), CTimeZoneRules::AMBIGUOUSTIME_EARLIER, CTimeZoneRules::NONEXISTENTTIME_SHIFT_FORWARD, utc ) );
            CTimeZoneRules::TLocalTimeKind asked = CTimeZoneRules::LOCALTIME_UNIQUE;
            ASSERT_FALSE( zones.GetLocalTimeKind( "America/New_York", Utc( 1986, 7, 1, 10, 0 ), asked ) );
            ASSERT_FALSE( zones.GetLocalTimeKind( "Nowhere/Nothing", Utc( 2024, 7, 1, 10, 0 ), asked ) );
            ASSERT_TRUE( zones.GetLocalTimeKind( "America/New_York", Utc( 2024, 7, 1, 10, 0 ), asked ) && CTimeZoneRules::LOCALTIME_UNIQUE == asked );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Random instants go to local time and back to the same instant, whatever the zone, with the policy that picks the occurrence" )
        try
        {
            CTimeZoneRules zones;
            ASSERT_TRUE( LoadFixture( zones ) );
            static const char* const names[] = { "America/New_York", "America/Chicago", "America/Mexico_City", "America/Sao_Paulo", "Europe/London", "Europe/Paris", "Asia/Tokyo", "Asia/Kolkata", "Australia/Sydney" };
            CRandom random( 20241103 );
            UInt32 ambiguousSeen = 0;
            for ( UInt32 z=0; z<sizeof( names ) / sizeof( names[ 0 ] ); ++z )
            {
                for ( UInt32 n=0; n<800; ++n )
                {
                    // half of the instants are within a day of a transition of the year, where the interesting cases are
                    Int64 instant = random.Between( DayOf( 2021, 1, 1 ), DayOf( 2035, 12, 31 ) ) * 86400LL + random.Between( 0, 86399 );
                    if ( 0 == ( n & 1 ) )
                    {
                        bool has = false;
                        Int64 start = 0, end = 0;
                        const Int32 year = (Int32) random.Between( 2021, 2035 );
                        if ( zones.GetTransitionsOfYear( names[ z ], year, has, start, end ) && has )
                            instant = ( 0 == ( n & 2 ) ? start : end ) + random.Between( -2 * 3600, 2 * 3600 );
                    }

                    Int32 offset = 0;
                    ASSERT_TRUE( zones.GetOffsetAtInstant( names[ z ], instant, offset ) );
                    const Int64 localSeconds = instant + (Int64) offset * 60;

                    CTimeZoneRules::TLocalTimeKind kind = CTimeZoneRules::LOCALTIME_UNIQUE;
                    ASSERT_TRUE( zones.GetLocalTimeKind( names[ z ], localSeconds, kind ) );
                    ASSERT_TRUE( CTimeZoneRules::LOCALTIME_NONEXISTENT != kind );

                    Int64 earlier = 0, later = 0;
                    ASSERT_TRUE( zones.LocalToUtc( names[ z ], localSeconds, CTimeZoneRules::AMBIGUOUSTIME_EARLIER, CTimeZoneRules::NONEXISTENTTIME_REJECT, earlier ) );
                    ASSERT_TRUE( zones.LocalToUtc( names[ z ], localSeconds, CTimeZoneRules::AMBIGUOUSTIME_LATER, CTimeZoneRules::NONEXISTENTTIME_REJECT, later ) );
                    if ( CTimeZoneRules::LOCALTIME_AMBIGUOUS == kind )
                    {
                        ++ambiguousSeen;
                        ASSERT_TRUE( earlier < later && 3600 == later - earlier );
                        ASSERT_TRUE( instant == earlier || instant == later );
                    }
                    else
                    {
                        ASSERT_TRUE( earlier == later && instant == earlier );
                    }

                    // the same through the date and time
                    CDateTime local;
                    ASSERT_TRUE( zones.UtcToLocal( names[ z ], instant, local ) );
                    ASSERT_TRUE( offset == local.GetTimeZoneUTCOffsetInMins() );
                }
            }
            ASSERT_TRUE( ambiguousSeen > 100 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformDateTimeTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: A UTC date and time becomes the local date and time with the offset of the zone, the milliseconds kept" )
        try
        {
            CTimeZoneRules zones;
            ASSERT_TRUE( LoadFixture( zones ) );
            CDateTime local;

            // 05:30 UTC is 01:30 EDT, one hour later 01:30 EST
            ASSERT_TRUE( zones.UtcToLocal( "America/New_York", CDateTime( 2024, 11, 3, 5, 30, 0, 250, 0 ), local ) );
            ASSERT_TRUE( 2024 == local.GetYear() && 11 == local.GetMonth() && 3 == local.GetDay() && 1 == local.GetHours() && 30 == local.GetMinutes() && 0 == local.GetSeconds() );
            ASSERT_TRUE( 250 == local.GetMilliseconds() && -240 == local.GetTimeZoneUTCOffsetInMins() );
            ASSERT_TRUE( zones.UtcToLocal( "America/New_York", CDateTime( 2024, 11, 3, 6, 30, 0, 250, 0 ), local ) );
            ASSERT_TRUE( 1 == local.GetHours() && 30 == local.GetMinutes() && 250 == local.GetMilliseconds() && -300 == local.GetTimeZoneUTCOffsetInMins() );

            // the day changes, and the year
            ASSERT_TRUE( zones.UtcToLocal( "Asia/Tokyo", CDateTime( 2024, 12, 31, 20, 15, 59, 999, 0 ), local ) );
            ASSERT_TRUE( 2025 == local.GetYear() && 1 == local.GetMonth() && 1 == local.GetDay() && 5 == local.GetHours() && 15 == local.GetMinutes() && 59 == local.GetSeconds() && 999 == local.GetMilliseconds() && 540 == local.GetTimeZoneUTCOffsetInMins() );
            ASSERT_TRUE( zones.UtcToLocal( "America/New_York", CDateTime( 2025, 1, 1, 3, 0, 0, 0, 0 ), local ) );
            ASSERT_TRUE( 2024 == local.GetYear() && 12 == local.GetMonth() && 31 == local.GetDay() && 22 == local.GetHours() );
            ASSERT_TRUE( zones.UtcToLocal( "Asia/Kolkata", CDateTime( 2024, 6, 1, 6, 30, 0, 0, 0 ), local ) );
            ASSERT_TRUE( 12 == local.GetHours() && 0 == local.GetMinutes() && 330 == local.GetTimeZoneUTCOffsetInMins() );

            // the instant is read with its own offset
            ASSERT_TRUE( zones.UtcToLocal( "America/New_York", CDateTime( 2024, 3, 10, 1, 59, 0, 0, -300 ), local ) );        // 06:59 UTC
            ASSERT_TRUE( 1 == local.GetHours() && 59 == local.GetMinutes() && -300 == local.GetTimeZoneUTCOffsetInMins() );
            ASSERT_TRUE( zones.UtcToLocal( "America/New_York", CDateTime( 2024, 3, 10, 2, 0, 0, 0, -300 ), local ) );         // 07:00 UTC
            ASSERT_TRUE( 3 == local.GetHours() && 0 == local.GetMinutes() && -240 == local.GetTimeZoneUTCOffsetInMins() );
            ASSERT_TRUE( zones.UtcToLocal( "America/New_York", CDateTime( 2024, 3, 10, 8, 0, 0, 0, 60 ), local ) );          // 07:00 UTC
            ASSERT_TRUE( 3 == local.GetHours() && -240 == local.GetTimeZoneUTCOffsetInMins() );

            // the epoch and before it
            ASSERT_TRUE( zones.UtcToLocal( "Asia/Tokyo", 0, local ) );
            ASSERT_TRUE( 1970 == local.GetYear() && 1 == local.GetMonth() && 1 == local.GetDay() && 9 == local.GetHours() );
            ASSERT_TRUE( zones.UtcToLocal( "America/New_York", Utc( 1999, 12, 31, 23, 59 ) + 59, local ) );
            ASSERT_TRUE( 1999 == local.GetYear() && 12 == local.GetMonth() && 31 == local.GetDay() && 18 == local.GetHours() && 59 == local.GetMinutes() && 59 == local.GetSeconds() );

            // the offset in force at an instant given as a date and time
            Int32 offset = 0;
            bool daylight = false;
            ASSERT_TRUE( zones.GetOffsetAtInstant( "America/New_York", CDateTime( 2024, 3, 10, 6, 59, 59, 999, 0 ), offset, &daylight ) && -300 == offset && !daylight );
            ASSERT_TRUE( zones.GetOffsetAtInstant( "America/New_York", CDateTime( 2024, 3, 10, 7, 0, 0, 0, 0 ), offset, &daylight ) && -240 == offset && daylight );

            // refused: not a date and time, an unknown zone, a year without rule
            ASSERT_FALSE( zones.UtcToLocal( "America/New_York", CDateTime( 2024, 13, 1, 0, 0, 0, 0, 0 ), local ) );
            ASSERT_FALSE( zones.UtcToLocal( "America/New_York", CDateTime( 2024, 2, 30, 0, 0, 0, 0, 0 ), local ) );
            ASSERT_FALSE( zones.UtcToLocal( "America/New_York", CDateTime( 2024, 0, 0, 0, 0, 0, 0, 0 ), local ) );
            ASSERT_FALSE( zones.UtcToLocal( "Nowhere/Nothing", CDateTime( 2024, 7, 1, 0, 0, 0, 0, 0 ), local ) );
            ASSERT_FALSE( zones.UtcToLocal( "America/New_York", CDateTime( 1980, 7, 1, 0, 0, 0, 0, 0 ), local ) );
            ASSERT_FALSE( zones.GetOffsetAtInstant( "America/New_York", CDateTime( 2024, 0, 0, 0, 0, 0, 0, 0 ), offset ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A local date and time becomes a UTC date and time with the policies, the offset it carries ignored" )
        try
        {
            CTimeZoneRules zones;
            ASSERT_TRUE( LoadFixture( zones ) );
            CDateTime utc;
            CTimeZoneRules::TLocalTimeKind kind = CTimeZoneRules::LOCALTIME_UNIQUE;

            // 01:30 on the day the clocks go back: ambiguous. The offset the date and time carries (an arbitrary +02:00 here) does not take part
            const CDateTime ambiguous( 2024, 11, 3, 1, 30, 0, 500, 120 );
            ASSERT_TRUE( zones.LocalToUtc( "America/New_York", ambiguous, CTimeZoneRules::AMBIGUOUSTIME_EARLIER, CTimeZoneRules::NONEXISTENTTIME_REJECT, utc, &kind ) );
            ASSERT_TRUE( CTimeZoneRules::LOCALTIME_AMBIGUOUS == kind );
            ASSERT_TRUE( 2024 == utc.GetYear() && 11 == utc.GetMonth() && 3 == utc.GetDay() && 5 == utc.GetHours() && 30 == utc.GetMinutes() && 500 == utc.GetMilliseconds() && 0 == utc.GetTimeZoneUTCOffsetInMins() );
            ASSERT_TRUE( zones.LocalToUtc( "America/New_York", ambiguous, CTimeZoneRules::AMBIGUOUSTIME_LATER, CTimeZoneRules::NONEXISTENTTIME_REJECT, utc, &kind ) );
            ASSERT_TRUE( 6 == utc.GetHours() && 30 == utc.GetMinutes() && 500 == utc.GetMilliseconds() );
            ASSERT_FALSE( zones.LocalToUtc( "America/New_York", ambiguous, CTimeZoneRules::AMBIGUOUSTIME_REJECT, CTimeZoneRules::NONEXISTENTTIME_REJECT, utc, &kind ) );
            ASSERT_TRUE( CTimeZoneRules::LOCALTIME_AMBIGUOUS == kind );

            // 02:30 on the day the clocks go forward: nonexistent
            const CDateTime missing( 2024, 3, 10, 2, 30, 0, 0, 0 );
            ASSERT_TRUE( zones.LocalToUtc( "America/New_York", missing, CTimeZoneRules::AMBIGUOUSTIME_REJECT, CTimeZoneRules::NONEXISTENTTIME_SHIFT_FORWARD, utc, &kind ) );
            ASSERT_TRUE( CTimeZoneRules::LOCALTIME_NONEXISTENT == kind && 7 == utc.GetHours() && 30 == utc.GetMinutes() );
            ASSERT_TRUE( zones.LocalToUtc( "America/New_York", missing, CTimeZoneRules::AMBIGUOUSTIME_REJECT, CTimeZoneRules::NONEXISTENTTIME_SHIFT_BACKWARD, utc, &kind ) );
            ASSERT_TRUE( 6 == utc.GetHours() && 30 == utc.GetMinutes() );
            ASSERT_FALSE( zones.LocalToUtc( "America/New_York", missing, CTimeZoneRules::AMBIGUOUSTIME_REJECT, CTimeZoneRules::NONEXISTENTTIME_REJECT, utc, &kind ) );

            // an ordinary time, across midnight and a year
            ASSERT_TRUE( zones.LocalToUtc( "Asia/Tokyo", CDateTime( 2025, 1, 1, 5, 15, 59, 999, 0 ), CTimeZoneRules::AMBIGUOUSTIME_REJECT, CTimeZoneRules::NONEXISTENTTIME_REJECT, utc, &kind ) );
            ASSERT_TRUE( CTimeZoneRules::LOCALTIME_UNIQUE == kind );
            ASSERT_TRUE( 2024 == utc.GetYear() && 12 == utc.GetMonth() && 31 == utc.GetDay() && 20 == utc.GetHours() && 15 == utc.GetMinutes() && 59 == utc.GetSeconds() && 999 == utc.GetMilliseconds() );

            // the round trip of a UTC date and time
            CDateTime local, back;
            const CDateTime instant( 2024, 7, 4, 19, 45, 12, 345, 0 );
            ASSERT_TRUE( zones.UtcToLocal( "Europe/Paris", instant, local ) );
            ASSERT_TRUE( 21 == local.GetHours() && 120 == local.GetTimeZoneUTCOffsetInMins() );
            ASSERT_TRUE( zones.LocalToUtc( "Europe/Paris", local, CTimeZoneRules::AMBIGUOUSTIME_REJECT, CTimeZoneRules::NONEXISTENTTIME_REJECT, back ) );
            ASSERT_TRUE( instant.GetYear() == back.GetYear() && instant.GetMonth() == back.GetMonth() && instant.GetDay() == back.GetDay() &&
                         instant.GetHours() == back.GetHours() && instant.GetMinutes() == back.GetMinutes() && instant.GetSeconds() == back.GetSeconds() &&
                         instant.GetMilliseconds() == back.GetMilliseconds() && 0 == back.GetTimeZoneUTCOffsetInMins() );

            // refused: not a date and time, an unknown zone
            ASSERT_FALSE( zones.LocalToUtc( "America/New_York", CDateTime( 2024, 0, 0, 0, 0, 0, 0, 0 ), CTimeZoneRules::AMBIGUOUSTIME_EARLIER, CTimeZoneRules::NONEXISTENTTIME_SHIFT_FORWARD, utc ) );
            ASSERT_FALSE( zones.LocalToUtc( "Nowhere/Nothing", back, CTimeZoneRules::AMBIGUOUSTIME_EARLIER, CTimeZoneRules::NONEXISTENTTIME_SHIFT_FORWARD, utc ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The calendar helpers count days of the proleptic Gregorian calendar, before 1970 and in leap years" )
        try
        {
            ASSERT_TRUE( 0 == CTimeZoneRules::DaysFromCivil( 1970, 1, 1 ) );
            ASSERT_TRUE( -1 == CTimeZoneRules::DaysFromCivil( 1969, 12, 31 ) );
            ASSERT_TRUE( 19723 == CTimeZoneRules::DaysFromCivil( 2024, 1, 1 ) );
            ASSERT_TRUE( 11016 == CTimeZoneRules::DaysFromCivil( 2000, 2, 29 ) );
            ASSERT_TRUE( CTimeZoneRules::DaysFromCivil( 2000, 3, 1 ) - CTimeZoneRules::DaysFromCivil( 2000, 2, 28 ) == 2 );
            ASSERT_TRUE( CTimeZoneRules::DaysFromCivil( 1900, 3, 1 ) - CTimeZoneRules::DaysFromCivil( 1900, 2, 28 ) == 1 );

            // the same count as the date class
            CDate date( 2024, 3, 10 );
            ASSERT_TRUE( date.ToDaysSinceUnixEpoch() == CTimeZoneRules::DaysFromCivil( 2024, 3, 10 ) );

            // and back, over a wide range
            for ( Int64 days = -800000; days <= 800000; days += 997 )
            {
                Int64 year = 0;
                Int32 month = 0, day = 0;
                CTimeZoneRules::CivilFromDays( days, year, month, day );
                ASSERT_TRUE( month >= 1 && month <= 12 && day >= 1 && day <= 31 );
                ASSERT_TRUE( days == CTimeZoneRules::DaysFromCivil( year, month, day ) );
            }

            // 1970-01-01 was a Thursday, 2024-03-10 a Sunday
            ASSERT_TRUE( 4 == CTimeZoneRules::WeekdayOfDay( 0 ) );
            ASSERT_TRUE( 3 == CTimeZoneRules::WeekdayOfDay( -1 ) );
            ASSERT_TRUE( 0 == CTimeZoneRules::WeekdayOfDay( CTimeZoneRules::DaysFromCivil( 2024, 3, 10 ) ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/** Asks a rule set that is only read, as every other thread does at the same time */
class CTimeZoneRulesTestWorker : public MT::CActiveObject
{
    public:

    bool allCorrect;

    explicit CTimeZoneRulesTestWorker( const CTimeZoneRules* rules )
        : MT::CActiveObject()
        , allCorrect( false )
        , m_rules( rules )
    {}

    virtual ~CTimeZoneRulesTestWorker()
    {}

    bool WaitForCompletion( UInt32 timeoutInMs )
    {
        UInt32 waitedInMs = 0;
        while ( IsActive() && waitedInMs < timeoutInMs )
        {
            MT::ThreadDelay( 5 );
            waitedInMs += 5;
        }
        return !IsActive();
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return this;
    }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        return true;
    }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        allCorrect = true;
        for ( UInt32 i=0; i<3000; ++i )
        {
            Int32 offset = 0;
            if ( !m_rules->GetOffsetAtInstant( "America/New_York", Utc( 2024, 3, 10, 6, 59 ) + 59, offset ) || -300 != offset )
                allCorrect = false;
            if ( !m_rules->GetOffsetAtInstant( "America/New_York", Utc( 2024, 3, 10, 7, 0 ), offset ) || -240 != offset )
                allCorrect = false;

            Int64 start = 0, end = 0;
            if ( !m_rules->GetLocalDayBounds( "America/New_York", DayOf( 2024, 11, 3 ), start, end ) || start != Utc( 2024, 11, 3, 4, 0 ) || end - start != 25 * SECONDS_PER_HOUR )
                allCorrect = false;

            Int64 utc = 0;
            if ( !m_rules->LocalToUtc( "Europe/Paris", Utc( 2024, 3, 31, 2, 30 ), CTimeZoneRules::AMBIGUOUSTIME_REJECT, CTimeZoneRules::NONEXISTENTTIME_SHIFT_FORWARD, utc ) || utc != Utc( 2024, 3, 31, 1, 30 ) )
                allCorrect = false;

            CDateTime local;
            if ( !m_rules->UtcToLocal( "Australia/Sydney", Utc( 2024, 1, 15, 0, 0 ), local ) || 11 != local.GetHours() || 660 != local.GetTimeZoneUTCOffsetInMins() )
                allCorrect = false;
        }
        return true; // done, the thread ends
    }

    private:

    const CTimeZoneRules* m_rules;
};

/*-------------------------------------------------------------------------*/

static void
PerformThreadTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: Rules that are loaded are used by several threads at once" )
        try
        {
            CTimeZoneRules rules;
            ASSERT_TRUE( LoadFixture( rules ) );

            static const UInt32 nrOfWorkers = 4;
            CTimeZoneRulesTestWorker* workers[ nrOfWorkers ];
            for ( UInt32 w=0; w<nrOfWorkers; ++w )
            {
                workers[ w ] = new CTimeZoneRulesTestWorker( &rules );
                ASSERT_TRUE( workers[ w ]->Activate() );
            }
            for ( UInt32 w=0; w<nrOfWorkers; ++w )
            {
                ASSERT_TRUE( workers[ w ]->WaitForCompletion( 60000 ) );
                ASSERT_TRUE( workers[ w ]->allCorrect );
                delete workers[ w ];
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformTimeZoneRulesTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( LOGLEVEL_NORMAL, "COMMENCING TimeZoneRules TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "TimeZoneRules" );

    GUCEF_TESTFW_TESTCASE( "Test: the json codec for the time zone data is available" )
        ASSERT_TRUE( JsonTestCodecAvailable() );
    GUCEF_TESTFW_TESTCASE_END

    if ( !JsonTestCodecAvailable() )
        return;

    PerformLoadTests();
    PerformTransitionTests();
    PerformLocalDayTests();
    PerformLocalTimeTests();
    PerformDateTimeTests();
    PerformThreadTests();
}

/*-------------------------------------------------------------------------*/
