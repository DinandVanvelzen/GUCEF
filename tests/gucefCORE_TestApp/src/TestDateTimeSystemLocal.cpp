/*
 *  gucefCORE_TestApp: GUCEF test application for the gucefCORE module
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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_CTIMEZONERULES_H
#include "gucefCORE_CTimeZoneRules.h"
#define GUCEF_CORE_CTIMEZONERULES_H
#endif /* GUCEF_CORE_CTIMEZONERULES_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestDateTimeSystemLocal.h"

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The instant of a date and time with its own offset, in seconds since 1970-01-01T00:00:00Z */
Int64
InstantOf( const CORE::CDateTime& dateTime )
{
    return CORE::CTimeZoneRules::DaysFromCivil( dateTime.GetYear(), dateTime.GetMonth(), dateTime.GetDay() ) * 86400 +
           (Int64) dateTime.GetHours() * 3600 + (Int64) dateTime.GetMinutes() * 60 + dateTime.GetSeconds() -
           (Int64) dateTime.GetTimeZoneUTCOffsetInMins() * 60;
}

#if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )

/** Sets the TZ variable of the process for a scope and puts back what it was (also not set at all) */
class CTimeZoneScope
{
    public:

    explicit CTimeZoneScope( const char* zone )
        : m_hadValue( false )
        , m_previous()
    {
        const char* previous = getenv( "TZ" );
        if ( GUCEF_NULL != previous )
        {
            m_hadValue = true;
            m_previous = previous;
        }
        setenv( "TZ", zone, 1 );
        tzset();
    }

    ~CTimeZoneScope()
    {
        if ( m_hadValue )
            setenv( "TZ", m_previous.C_String(), 1 );
        else
            unsetenv( "TZ" );
        tzset();
    }

    private:

    bool m_hadValue;
    CORE::CString m_previous;
};

#endif /* platform that follows TZ ? */

/** The local date and time of a UTC instant, as text "YYYY-MM-DD HH:MM:SS.mmm +offset" */
CORE::CString
LocalText( const CORE::CDateTime& utc )
{
    CORE::CDateTime local = utc.ToSystemLocal();
    char buffer[ 64 ];
    sprintf( buffer, "%04d-%02d-%02d %02d:%02d:%02d.%03d %+d", (int) local.GetYear(), (int) local.GetMonth(), (int) local.GetDay(),
             (int) local.GetHours(), (int) local.GetMinutes(), (int) local.GetSeconds(), (int) local.GetMilliseconds(),
             (int) local.GetTimeZoneUTCOffsetInMins() );
    return buffer;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDateTimeSystemLocalTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "DateTimeSystemLocal" );

    GUCEF_TESTFW_TESTCASE( "Whatever the zone of the machine is, the instant is kept and the offset is the one of that zone" )
    try
    {
        // a winter and a summer instant, with milliseconds, and one before 1970
        Int64 instants[] = { 1768478400LL, 1784116800LL, 1700000000LL, 951782400LL, -86400LL * 400 - 1 };
        for ( UInt32 i=0; i<sizeof( instants ) / sizeof( instants[ 0 ] ); ++i )
        {
            Int64 days = instants[ i ] / 86400;
            Int64 secondOfDay = instants[ i ] % 86400;
            if ( secondOfDay < 0 )
            {
                secondOfDay += 86400;
                --days;
            }
            Int64 year = 0;
            Int32 month = 0, day = 0;
            CORE::CTimeZoneRules::CivilFromDays( days, year, month, day );
            CORE::CDateTime utc( (Int16) year, (UInt8) month, (UInt8) day, (Int8) ( secondOfDay / 3600 ), (Int8) ( ( secondOfDay % 3600 ) / 60 ), (Int8) ( secondOfDay % 60 ), 321, 0 );
            CORE::CDateTime local = utc.ToSystemLocal();
            ASSERT_TRUE( InstantOf( local ) == instants[ i ] );
            ASSERT_TRUE( 321 == local.GetMilliseconds() );
            ASSERT_TRUE( InstantOf( local ) == InstantOf( utc ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )

    // These follow the TZ variable with the rules written in the variable itself, so they do not depend on the zone database of the machine.
    // The Windows conversion uses the zone the user has chosen and ignores the variable.

    GUCEF_TESTFW_TESTCASE( "A value that carries an offset is converted by its instant, not by its clock reading" )
    try
    {
        CTimeZoneScope zone( "UTC0" );
        // 12:00 at +01:00 is 11:00 UTC; the machine zone is UTC
        CORE::CDateTime withOffset( 2026, 6, 1, 12, 0, 0, 0, 60 );
        CORE::CDateTime local = withOffset.ToSystemLocal();
        ASSERT_TRUE( 11 == local.GetHours() && 0 == local.GetMinutes() && 1 == local.GetDay() && 0 == local.GetTimeZoneUTCOffsetInMins() );
        ASSERT_TRUE( InstantOf( local ) == InstantOf( withOffset ) );

        // across midnight and across a year end, backward and forward
        CORE::CDateTime earlyAtPlus( 2026, 1, 1, 0, 30, 0, 0, 120 );
        local = earlyAtPlus.ToSystemLocal();
        ASSERT_TRUE( 2025 == local.GetYear() && 12 == local.GetMonth() && 31 == local.GetDay() && 22 == local.GetHours() && 30 == local.GetMinutes() );
        CORE::CDateTime lateAtMinus( 2025, 12, 31, 23, 30, 0, 0, -120 );
        local = lateAtMinus.ToSystemLocal();
        ASSERT_TRUE( 2026 == local.GetYear() && 1 == local.GetMonth() && 1 == local.GetDay() && 1 == local.GetHours() && 30 == local.GetMinutes() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "UTC zone: the value is unchanged and the offset is zero" )
    try
    {
        CTimeZoneScope zone( "UTC0" );
        ASSERT_TRUE( "2026-11-15 13:05:09.250 +0" == LocalText( CORE::CDateTime( 2026, 11, 15, 13, 5, 9, 250, 0 ) ) );
        ASSERT_TRUE( "1969-12-31 23:59:59.999 +0" == LocalText( CORE::CDateTime( 1969, 12, 31, 23, 59, 59, 999, 0 ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A zone with a half hour offset, east and west of UTC" )
    try
    {
        {
            CTimeZoneScope zone( "IST-5:30" );
            ASSERT_TRUE( "2026-06-01 05:30:00.000 +330" == LocalText( CORE::CDateTime( 2026, 6, 1, 0, 0, 0, 0, 0 ) ) );
            ASSERT_TRUE( "2026-01-01 03:29:59.500 +330" == LocalText( CORE::CDateTime( 2025, 12, 31, 21, 59, 59, 500, 0 ) ) );
        }
        {
            CTimeZoneScope zone( "NST3:30" );
            ASSERT_TRUE( "2026-05-31 20:30:00.000 -210" == LocalText( CORE::CDateTime( 2026, 6, 1, 0, 0, 0, 0, 0 ) ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Northern hemisphere daylight saving: the instants around the spring forward and the fall back of 2026 (US rules)" )
    try
    {
        CTimeZoneScope zone( "EST5EDT,M3.2.0/2,M11.1.0/2" );

        // spring forward: 2026-03-08 02:00 EST is 07:00 UTC; the clocks jump to 03:00 EDT
        ASSERT_TRUE( "2026-03-08 01:59:59.999 -300" == LocalText( CORE::CDateTime( 2026, 3, 8, 6, 59, 59, 999, 0 ) ) );
        ASSERT_TRUE( "2026-03-08 03:00:00.000 -240" == LocalText( CORE::CDateTime( 2026, 3, 8, 7, 0, 0, 0, 0 ) ) );
        ASSERT_TRUE( "2026-03-08 03:00:00.001 -240" == LocalText( CORE::CDateTime( 2026, 3, 8, 7, 0, 0, 1, 0 ) ) );

        // fall back: 2026-11-01 02:00 EDT is 06:00 UTC; the clocks return to 01:00 EST, so 01:xx occurs twice
        ASSERT_TRUE( "2026-11-01 01:30:00.000 -240" == LocalText( CORE::CDateTime( 2026, 11, 1, 5, 30, 0, 0, 0 ) ) );
        ASSERT_TRUE( "2026-11-01 01:59:59.999 -240" == LocalText( CORE::CDateTime( 2026, 11, 1, 5, 59, 59, 999, 0 ) ) );
        ASSERT_TRUE( "2026-11-01 01:00:00.000 -300" == LocalText( CORE::CDateTime( 2026, 11, 1, 6, 0, 0, 0, 0 ) ) );
        ASSERT_TRUE( "2026-11-01 01:30:00.000 -300" == LocalText( CORE::CDateTime( 2026, 11, 1, 6, 30, 0, 0, 0 ) ) );

        // far from the changes: standard time in winter, daylight saving time in summer
        ASSERT_TRUE( "2026-01-15 07:00:00.000 -300" == LocalText( CORE::CDateTime( 2026, 1, 15, 12, 0, 0, 0, 0 ) ) );
        ASSERT_TRUE( "2026-07-15 08:00:00.000 -240" == LocalText( CORE::CDateTime( 2026, 7, 15, 12, 0, 0, 0, 0 ) ) );

        // the same instant, given with another offset, gives the same local time
        ASSERT_TRUE( "2026-03-08 03:00:00.000 -240" == LocalText( CORE::CDateTime( 2026, 3, 8, 8, 0, 0, 0, 60 ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Southern hemisphere daylight saving: it starts in October and ends in April (Sydney rules)" )
    try
    {
        CTimeZoneScope zone( "AEST-10AEDT,M10.1.0/2,M4.1.0/3" );

        // starts 2026-10-04 02:00 AEST, which is 2026-10-03 16:00 UTC
        ASSERT_TRUE( "2026-10-04 01:59:59.000 +600" == LocalText( CORE::CDateTime( 2026, 10, 3, 15, 59, 59, 0, 0 ) ) );
        ASSERT_TRUE( "2026-10-04 03:00:00.000 +660" == LocalText( CORE::CDateTime( 2026, 10, 3, 16, 0, 0, 0, 0 ) ) );

        // ends 2026-04-05 03:00 AEDT, which is 2026-04-04 16:00 UTC; the clocks return to 02:00 AEST
        ASSERT_TRUE( "2026-04-05 02:59:59.000 +660" == LocalText( CORE::CDateTime( 2026, 4, 4, 15, 59, 59, 0, 0 ) ) );
        ASSERT_TRUE( "2026-04-05 02:00:00.000 +600" == LocalText( CORE::CDateTime( 2026, 4, 4, 16, 0, 0, 0, 0 ) ) );

        // January is summer there
        ASSERT_TRUE( "2026-01-15 23:00:00.000 +660" == LocalText( CORE::CDateTime( 2026, 1, 15, 12, 0, 0, 0, 0 ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "The variable is followed when it changes while the process runs" )
    try
    {
        CORE::CDateTime utc( 2026, 7, 15, 12, 0, 0, 0, 0 );
        {
            CTimeZoneScope zone( "EST5EDT,M3.2.0/2,M11.1.0/2" );
            ASSERT_TRUE( -240 == utc.ToSystemLocal().GetTimeZoneUTCOffsetInMins() );
            setenv( "TZ", "CET-1CEST,M3.5.0/2,M10.5.0/3", 1 );
            ASSERT_TRUE( 120 == utc.ToSystemLocal().GetTimeZoneUTCOffsetInMins() );
            ASSERT_TRUE( 60 == CORE::CDateTime( 2026, 1, 15, 12, 0, 0, 0, 0 ).ToSystemLocal().GetTimeZoneUTCOffsetInMins() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    #endif /* platform that follows TZ ? */
}

/*-------------------------------------------------------------------------*/
