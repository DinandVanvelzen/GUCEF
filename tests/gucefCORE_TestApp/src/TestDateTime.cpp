/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#include <time.h>

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestDateTime.h"

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
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const CORE::CString testDateTimeString1 = "2020-07-10 15:01:02.300";
static const GUCEF::vector< Int32 > testDateTimeString1Result = { 2020, 7, 10, 15, 1, 2, 300 };
static const Int32 testDateTimeString1TzOffsetResult = 0;

static const CORE::CString testDateTimeString2 = "2013-07-16T19:23:51Z";
static const GUCEF::vector< Int32 > testDateTimeString2Result = { 2013, 7, 16, 19, 23, 51, 0 };
static const Int32 testDateTimeString2TzOffsetResult = 0;

static const CORE::CString testDateTimeString3 = "2020-07-10 15:01:02.300Z";
static const GUCEF::vector< Int32 > testDateTimeString3Result = { 2020, 7, 10, 15, 1, 2, 300 };
static const Int32 testDateTimeString3TzOffsetResult = 0;

static const CORE::CString testDateTimeString4 = "2020/07/10 15:01:02.300Z";
static const GUCEF::vector< Int32 > testDateTimeString4Result = { 2020, 7, 10, 15, 1, 2, 300 };
static const Int32 testDateTimeString4TzOffsetResult = 0;

static const CORE::CString testDateTimeString5 = "2020:07:10 15:01:02.300Z";
static const GUCEF::vector< Int32 > testDateTimeString5Result = { 2020, 7, 10, 15, 1, 2, 300 };
static const Int32 testDateTimeString5TzOffsetResult = 0;

static const CORE::CString testDateTimeString6 = "2020.07.10 15.01.02.300Z";
static const GUCEF::vector< Int32 > testDateTimeString6Result = { 2020, 7, 10, 15, 1, 2, 300 };
static const Int32 testDateTimeString6TzOffsetResult = 0;

static const CORE::CString testDateTimeString7 = "+20:07:10";
static const GUCEF::vector< Int32 > testDateTimeString7Result = { 0, 0, 0, 20, 7, 10, 0 };
static const Int32 testDateTimeString7TzOffsetResult = 0;

static const CORE::CString testDateTimeString8 = "-20:07:10";
static const GUCEF::vector< Int32 > testDateTimeString8Result = { 0, 0, 0, -20, -07, -10, 0 };
static const Int32 testDateTimeString8TzOffsetResult = 0;

static const CORE::CString testDateTimeString9 = "+20:07:10.300";
static const GUCEF::vector< Int32 > testDateTimeString9Result = { 0, 0, 0, 20, 7, 10, 300 };
static const Int32 testDateTimeString9TzOffsetResult = 0;

static const CORE::CString testDateTimeString10 = "2013-07-16T19:23:51+04:30";
static const GUCEF::vector< Int32 > testDateTimeString10Result = { 2013, 7, 16, 19, 23, 51, 0 };
static const Int32 testDateTimeString10TzOffsetResult = 30+(4*60);

static const CORE::CString testDateTimeString11 = "2013-07-16T19:23:51-04:30";
static const GUCEF::vector< Int32 > testDateTimeString11Result = { 2013, 7, 16, 19, 23, 51, 0 };
static const Int32 testDateTimeString11TzOffsetResult = -(30+(4*60));

static const CORE::CString testDateTimeString12 = "2020/07/10";
static const GUCEF::vector< Int32 > testDateTimeString12Result = { 2020, 7, 10, 0, 0, 0, 0 };
static const Int32 testDateTimeString12TzOffsetResult = 0;

static const CORE::CString testDateTimeString13 = "2020-07-10";
static const GUCEF::vector< Int32 > testDateTimeString13Result = { 2020, 7, 10, 0, 0, 0, 0 };
static const Int32 testDateTimeString13TzOffsetResult = 0;

static const CORE::CString testDateTimeString14 = "2020 07 10";
static const GUCEF::vector< Int32 > testDateTimeString14Result = { 2020, 7, 10, 0, 0, 0, 0 };
static const Int32 testDateTimeString14TzOffsetResult = 0;

static const CORE::CString testDateTimeString15 = "20 07 10";
static const GUCEF::vector< Int32 > testDateTimeString15Result = { 0, 0, 0, 20, 7, 10, 0 };
static const Int32 testDateTimeString15TzOffsetResult = 0;

static const CORE::CString testDateTimeString16 = "200710";
static const GUCEF::vector< Int32 > testDateTimeString16Result = { 0, 0, 0, 20, 7, 10, 0 };
static const Int32 testDateTimeString16TzOffsetResult = 0;

static const CORE::CString testDateTimeString17 = "20200710";
static const GUCEF::vector< Int32 > testDateTimeString17Result = { 2020, 7, 10, 0, 0, 0, 0 };
static const Int32 testDateTimeString17TzOffsetResult = 0;

static const CORE::CString testDateTimeString18 = "20200710060730";
static const GUCEF::vector< Int32 > testDateTimeString18Result = { 2020, 7, 10, 6, 7, 30, 0 };
static const Int32 testDateTimeString18TzOffsetResult = 0;

static const CORE::CString testDateTimeString19 = "20200710060730.600";
static const GUCEF::vector< Int32 > testDateTimeString19Result = { 2020, 7, 10, 6, 7, 30, 600 };
static const Int32 testDateTimeString19TzOffsetResult = 0;

static const CORE::CString testDateTimeString20 = "2020-7-1";
static const GUCEF::vector< Int32 > testDateTimeString20Result = { 2020, 7, 1, 0, 0, 0, 0 };
static const Int32 testDateTimeString20TzOffsetResult = 0;

static const CORE::CString testDateTimeString21 = "+20:7:1";
static const GUCEF::vector< Int32 > testDateTimeString21Result = { 0, 0, 0, 20, 7, 1, 0 };
static const Int32 testDateTimeString21TzOffsetResult = 0;

static const CORE::CString testDateTimeString22 = "2020-7-1 15:1:2.300";
static const GUCEF::vector< Int32 > testDateTimeString22Result = { 2020, 7, 1, 15, 1, 2, 300 };
static const Int32 testDateTimeString22TzOffsetResult = 0;

static const CORE::CString testDateTimeString23 = "2020-7-1 15:1:2.300+04:30";
static const GUCEF::vector< Int32 > testDateTimeString23Result = { 2020, 7, 1, 15, 1, 2, 300 };
static const Int32 testDateTimeString23TzOffsetResult = 30+(4*60);

static const CORE::CString testDateTimeString24 = "2020-7-1 15:1:2.300-4:30";
static const GUCEF::vector< Int32 > testDateTimeString24Result = { 2020, 7, 1, 15, 1, 2, 300 };
static const Int32 testDateTimeString24TzOffsetResult = -(30+(4*60));

static const CORE::CString testDateTimeString25 = "2024-265";
static const GUCEF::vector< Int32 > testDateTimeString25Result = { 2024, 9, 21, 0, 0, 0, 0 };
static const Int32 testDateTimeString25TzOffsetResult = 0;

static const CORE::CString testDateTimeString26 = "2013-07-16T19:23:51Z\0This is a test, this should not be included";
static const GUCEF::vector< Int32 > testDateTimeString26Result = { 2013, 7, 16, 19, 23, 51, 0 };
static const Int32 testDateTimeString26TzOffsetResult = 0;

static const CORE::CString testDateTimeString27 = "2013-07-16T19:23:51Z This is a test, this should not be included";
static const GUCEF::vector< Int32 > testDateTimeString27Result = { 2013, 7, 16, 19, 23, 51, 0 };
static const Int32 testDateTimeString27TzOffsetResult = 0;

static const CORE::CString testDateTimeString28 = "2013-07-16T19:23:51Z. This is a test, this should not be included";
static const GUCEF::vector< Int32 > testDateTimeString28Result = { 2013, 7, 16, 19, 23, 51, 0 };
static const Int32 testDateTimeString28TzOffsetResult = 0;

static const CORE::CString testDateTimeString29 = "20241017090031.330Z";
static const GUCEF::vector< Int32 > testDateTimeString29Result = { 2024, 10, 17, 9, 0, 31, 330 };
static const Int32 testDateTimeString29TzOffsetResult = 0;

struct StringParseTestCase
{
    const CORE::CString& dateTimeString;
    const GUCEF::vector< Int32 >& dateTimeStringResult;
    const Int32& tzOffsetResult;
};

static const GUCEF::vector< StringParseTestCase > stringParseTestCases = {
    { testDateTimeString1, testDateTimeString1Result, testDateTimeString1TzOffsetResult },
    { testDateTimeString2, testDateTimeString2Result, testDateTimeString2TzOffsetResult },
    { testDateTimeString3, testDateTimeString3Result, testDateTimeString3TzOffsetResult },
    { testDateTimeString4, testDateTimeString4Result, testDateTimeString4TzOffsetResult },
    { testDateTimeString5, testDateTimeString5Result, testDateTimeString5TzOffsetResult },
    { testDateTimeString6, testDateTimeString6Result, testDateTimeString6TzOffsetResult },
    { testDateTimeString7, testDateTimeString7Result, testDateTimeString7TzOffsetResult },
    { testDateTimeString8, testDateTimeString8Result, testDateTimeString8TzOffsetResult },
    { testDateTimeString9, testDateTimeString9Result, testDateTimeString9TzOffsetResult },
    { testDateTimeString10, testDateTimeString10Result, testDateTimeString10TzOffsetResult },
    { testDateTimeString11, testDateTimeString11Result, testDateTimeString11TzOffsetResult },
    { testDateTimeString12, testDateTimeString12Result, testDateTimeString12TzOffsetResult },
    { testDateTimeString13, testDateTimeString13Result, testDateTimeString13TzOffsetResult },
    { testDateTimeString14, testDateTimeString14Result, testDateTimeString14TzOffsetResult },
    { testDateTimeString15, testDateTimeString15Result, testDateTimeString15TzOffsetResult },
    { testDateTimeString16, testDateTimeString16Result, testDateTimeString16TzOffsetResult },
    { testDateTimeString17, testDateTimeString17Result, testDateTimeString17TzOffsetResult },
    { testDateTimeString18, testDateTimeString18Result, testDateTimeString18TzOffsetResult },
    { testDateTimeString19, testDateTimeString19Result, testDateTimeString19TzOffsetResult },
    { testDateTimeString20, testDateTimeString20Result, testDateTimeString20TzOffsetResult },
    { testDateTimeString21, testDateTimeString21Result, testDateTimeString21TzOffsetResult },
    { testDateTimeString22, testDateTimeString22Result, testDateTimeString22TzOffsetResult },
    { testDateTimeString23, testDateTimeString23Result, testDateTimeString23TzOffsetResult },
    { testDateTimeString24, testDateTimeString24Result, testDateTimeString24TzOffsetResult },
    { testDateTimeString25, testDateTimeString25Result, testDateTimeString25TzOffsetResult },
    { testDateTimeString26, testDateTimeString26Result, testDateTimeString26TzOffsetResult },
    { testDateTimeString27, testDateTimeString27Result, testDateTimeString27TzOffsetResult },
    { testDateTimeString28, testDateTimeString28Result, testDateTimeString28TzOffsetResult },
    { testDateTimeString29, testDateTimeString29Result, testDateTimeString29TzOffsetResult }

};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDateTimeStringParsingTests( void )
{
    GUCEF_TESTFW_TESTCASE( "Test: DateTime string parsing" )
        try
        {
            for ( UInt32 i=0; i<stringParseTestCases.size(); ++i )
            {
                const StringParseTestCase& testCase = stringParseTestCases[ i ];
                CORE::CDateTime dateTime;

                ASSERT_TRUE( dateTime.FromIso8601DateTimeString( testCase.dateTimeString ) );
                ASSERT_TRUE( dateTime.GetYear() == testCase.dateTimeStringResult[ 0 ] );
                ASSERT_TRUE( dateTime.GetMonth() == testCase.dateTimeStringResult[ 1 ] );
                ASSERT_TRUE( dateTime.GetDay() == testCase.dateTimeStringResult[ 2 ] );
                ASSERT_TRUE( dateTime.GetHours() == testCase.dateTimeStringResult[ 3 ] );
                ASSERT_TRUE( dateTime.GetMinutes() == testCase.dateTimeStringResult[ 4 ] );
                ASSERT_TRUE( dateTime.GetSeconds() == testCase.dateTimeStringResult[ 5 ] );
                ASSERT_TRUE( dateTime.GetMilliseconds() == testCase.dateTimeStringResult[ 6 ] );
                ASSERT_TRUE( dateTime.GetTimeZoneUTCOffsetInMins() == testCase.tzOffsetResult );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformDateCalendarTests( void )
{
    GUCEF_TESTFW_TESTCASE( "Test: Date validity" )
        try
        {
            ASSERT_TRUE( CORE::CDate( 2024, 2, 29 ).IsValid() );
            ASSERT_FALSE( CORE::CDate( 2023, 2, 29 ).IsValid() );
            ASSERT_FALSE( CORE::CDate( 1900, 2, 29 ).IsValid() );
            ASSERT_TRUE( CORE::CDate( 2000, 2, 29 ).IsValid() );
            ASSERT_FALSE( CORE::CDate( 2024, 0, 10 ).IsValid() );
            ASSERT_FALSE( CORE::CDate( 2024, 13, 10 ).IsValid() );
            ASSERT_FALSE( CORE::CDate( 2024, 4, 31 ).IsValid() );
            ASSERT_FALSE( CORE::CDate( 2024, 4, 0 ).IsValid() );
            ASSERT_FALSE( CORE::CDate().IsValid() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Date ordering" )
        try
        {
            CORE::CDate a( 2024, 3, 15 );
            CORE::CDate b( 2024, 3, 16 );
            CORE::CDate c( 2024, 4, 1 );
            CORE::CDate d( 2025, 1, 1 );
            ASSERT_TRUE( a < b && b < c && c < d );
            ASSERT_TRUE( a <= a && a <= b );
            ASSERT_TRUE( d > c && c > b && b > a );
            ASSERT_TRUE( a >= a && d >= a );
            ASSERT_FALSE( b < a );
            ASSERT_FALSE( a < a );
            ASSERT_FALSE( a > a );
            ASSERT_FALSE( a > b );
            ASSERT_FALSE( a >= b );
            ASSERT_FALSE( b <= a );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Days since unix epoch, known values" )
        try
        {
            ASSERT_TRUE( 0 == CORE::CDate( 1970, 1, 1 ).ToDaysSinceUnixEpoch() );
            ASSERT_TRUE( -1 == CORE::CDate( 1969, 12, 31 ).ToDaysSinceUnixEpoch() );
            ASSERT_TRUE( 11017 == CORE::CDate( 2000, 3, 1 ).ToDaysSinceUnixEpoch() );
            ASSERT_TRUE( 18793 == CORE::CDate( 2021, 6, 15 ).ToDaysSinceUnixEpoch() );
            ASSERT_TRUE( -25567 == CORE::CDate( 1900, 1, 1 ).ToDaysSinceUnixEpoch() );

            CORE::CDate date;
            ASSERT_TRUE( date.FromDaysSinceUnixEpoch( 18793 ) );
            ASSERT_TRUE( date == CORE::CDate( 2021, 6, 15 ) );
            ASSERT_TRUE( date.FromDaysSinceUnixEpoch( -25567 ) );
            ASSERT_TRUE( date == CORE::CDate( 1900, 1, 1 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Every day from year 1600 to 2400 steps by exactly one, round trips, and agrees with the C library where it can
    GUCEF_TESTFW_TESTCASE( "Test: Days since unix epoch, exhaustive consecutive days and round trip" )
        try
        {
            CORE::CDate date( 1600, 1, 1 );
            Int32 previous = date.ToDaysSinceUnixEpoch();
            const CORE::CDate last( 2400, 12, 31 );
            Int32 lastDays = last.ToDaysSinceUnixEpoch();
            bool allOk = true;
            for ( Int32 days = previous + 1; days <= lastDays; ++days )
            {
                CORE::CDate next;
                if ( !next.FromDaysSinceUnixEpoch( days ) || !next.IsValid() || !( date < next ) ||
                     next.ToDaysSinceUnixEpoch() != days || date.GetDaysTowards( next ) != 1 )
                {
                    allOk = false;
                    break;
                }

                // within the range every platform time_t can handle
                if ( days >= 0 && days <= 25000 )
                {
                    time_t t = (time_t) days * 86400;
                    struct tm* utc = ::gmtime( &t );
                    if ( utc == GUCEF_NULL || utc->tm_year + 1900 != next.GetYear() ||
                         utc->tm_mon + 1 != next.GetMonth() || utc->tm_mday != next.GetDay() ||
                         utc->tm_wday != next.GetDayOfWeek() )
                    {
                        allOk = false;
                        break;
                    }
                }
                date = next;
            }
            ASSERT_TRUE( allOk );
            ASSERT_TRUE( date == last );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Days between dates" )
        try
        {
            CORE::CDate a( 2024, 1, 1 );
            CORE::CDate b( 2025, 1, 1 );
            ASSERT_TRUE( 366 == a.GetDaysTowards( b ) );
            ASSERT_TRUE( -366 == b.GetDaysTowards( a ) );
            ASSERT_TRUE( 0 == a.GetDaysTowards( a ) );
            ASSERT_TRUE( 365 == CORE::CDate( 2025, 1, 1 ).GetDaysTowards( CORE::CDate( 2026, 1, 1 ) ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Adding days and weeks" )
        try
        {
            CORE::CDate date( 2024, 2, 28 );
            ASSERT_TRUE( date.AddDays( 1 ) );
            ASSERT_TRUE( date == CORE::CDate( 2024, 2, 29 ) );
            ASSERT_TRUE( date.AddDays( 1 ) );
            ASSERT_TRUE( date == CORE::CDate( 2024, 3, 1 ) );
            ASSERT_TRUE( date.AddDays( -366 ) );
            ASSERT_TRUE( date == CORE::CDate( 2023, 3, 1 ) );
            ASSERT_TRUE( date.AddWeeks( 2 ) );
            ASSERT_TRUE( date == CORE::CDate( 2023, 3, 15 ) );
            ASSERT_TRUE( date.AddWeeks( -3 ) );
            ASSERT_TRUE( date == CORE::CDate( 2023, 2, 22 ) );

            CORE::CDate invalid( 2023, 2, 30 );
            ASSERT_FALSE( invalid.AddDays( 1 ) );
            ASSERT_TRUE( invalid == CORE::CDate( 2023, 2, 30 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Adding months clamps to the end of the month" )
        try
        {
            CORE::CDate date( 2023, 1, 31 );
            ASSERT_TRUE( date.AddMonths( 1 ) );
            ASSERT_TRUE( date == CORE::CDate( 2023, 2, 28 ) );

            date = CORE::CDate( 2024, 1, 31 );
            ASSERT_TRUE( date.AddMonths( 1 ) );
            ASSERT_TRUE( date == CORE::CDate( 2024, 2, 29 ) );

            date = CORE::CDate( 2024, 1, 31 );
            ASSERT_TRUE( date.AddMonths( 13 ) );
            ASSERT_TRUE( date == CORE::CDate( 2025, 2, 28 ) );

            date = CORE::CDate( 2024, 3, 31 );
            ASSERT_TRUE( date.AddMonths( -1 ) );
            ASSERT_TRUE( date == CORE::CDate( 2024, 2, 29 ) );

            date = CORE::CDate( 2024, 12, 15 );
            ASSERT_TRUE( date.AddMonths( 1 ) );
            ASSERT_TRUE( date == CORE::CDate( 2025, 1, 15 ) );

            date = CORE::CDate( 2025, 1, 15 );
            ASSERT_TRUE( date.AddMonths( -1 ) );
            ASSERT_TRUE( date == CORE::CDate( 2024, 12, 15 ) );

            date = CORE::CDate( 2025, 1, 15 );
            ASSERT_TRUE( date.AddMonths( -12 ) );
            ASSERT_TRUE( date == CORE::CDate( 2024, 1, 15 ) );

            date = CORE::CDate( 2025, 1, 15 );
            ASSERT_TRUE( date.AddMonths( 0 ) );
            ASSERT_TRUE( date == CORE::CDate( 2025, 1, 15 ) );

            date = CORE::CDate( 2025, 1, 15 );
            ASSERT_TRUE( date.AddMonths( -13 ) );
            ASSERT_TRUE( date == CORE::CDate( 2023, 12, 15 ) );

            // month end to month end stays the clamped day, it does not remember the original day
            date = CORE::CDate( 2024, 1, 31 );
            ASSERT_TRUE( date.AddMonths( 1 ) );
            ASSERT_TRUE( date.AddMonths( 1 ) );
            ASSERT_TRUE( date == CORE::CDate( 2024, 3, 29 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Adding years" )
        try
        {
            CORE::CDate date( 2024, 2, 29 );
            ASSERT_TRUE( date.AddYears( 1 ) );
            ASSERT_TRUE( date == CORE::CDate( 2025, 2, 28 ) );

            date = CORE::CDate( 2024, 2, 29 );
            ASSERT_TRUE( date.AddYears( 4 ) );
            ASSERT_TRUE( date == CORE::CDate( 2028, 2, 29 ) );

            date = CORE::CDate( 2024, 2, 29 );
            ASSERT_TRUE( date.AddYears( -1 ) );
            ASSERT_TRUE( date == CORE::CDate( 2023, 2, 28 ) );

            date = CORE::CDate( 2024, 7, 4 );
            ASSERT_TRUE( date.AddYears( -100 ) );
            ASSERT_TRUE( date == CORE::CDate( 1924, 7, 4 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Calendar arithmetic refuses out of range and invalid dates and leaves the date unchanged" )
        try
        {
            CORE::CDate date( 32767, 6, 15 );
            ASSERT_FALSE( date.AddYears( 1 ) );
            ASSERT_TRUE( date == CORE::CDate( 32767, 6, 15 ) );
            ASSERT_FALSE( date.AddMonths( 7 ) );
            ASSERT_TRUE( date == CORE::CDate( 32767, 6, 15 ) );
            ASSERT_TRUE( date.AddMonths( 6 ) );
            ASSERT_TRUE( date == CORE::CDate( 32767, 12, 15 ) );

            CORE::CDate invalid( 2024, 13, 1 );
            ASSERT_FALSE( invalid.AddMonths( 1 ) );
            ASSERT_FALSE( invalid.AddYears( 1 ) );
            ASSERT_TRUE( invalid == CORE::CDate( 2024, 13, 1 ) );

            CORE::CDate big( 2024, 1, 1 );
            ASSERT_FALSE( big.AddDays( GUCEF_INT32MAX ) );
            ASSERT_TRUE( big == CORE::CDate( 2024, 1, 1 ) );
            ASSERT_FALSE( big.AddMonths( GUCEF_INT32MAX ) );
            ASSERT_FALSE( big.AddYears( GUCEF_INT32MAX ) );
            ASSERT_FALSE( big.AddWeeks( GUCEF_INT32MIN ) );
            ASSERT_TRUE( big == CORE::CDate( 2024, 1, 1 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Adding months to a date time keeps the time of day" )
        try
        {
            CORE::CDateTime dt( 2024, 1, 31, 13, 45, 12, 250, 0 );
            ASSERT_TRUE( dt.AddMonths( 1 ) );
            ASSERT_TRUE( 2024 == dt.GetYear() && 2 == dt.GetMonth() && 29 == dt.GetDay() );
            ASSERT_TRUE( 13 == dt.GetHours() && 45 == dt.GetMinutes() && 12 == dt.GetSeconds() && 250 == dt.GetMilliseconds() );
            ASSERT_TRUE( dt.AddYears( 1 ) );
            ASSERT_TRUE( 2025 == dt.GetYear() && 2 == dt.GetMonth() && 28 == dt.GetDay() );
            ASSERT_TRUE( 13 == dt.GetHours() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // The day count plus the era offset must not overflow a signed 32 bit integer: run under UBSan to see it
    GUCEF_TESTFW_TESTCASE( "Test: Day counts at the Int32 limits are refused without overflow and leave the date unchanged" )
        try
        {
            CORE::CDate date( 1970, 1, 1 );
            ASSERT_FALSE( date.AddDays( GUCEF_INT32MAX ) );
            ASSERT_TRUE( date == CORE::CDate( 1970, 1, 1 ) );
            ASSERT_FALSE( date.AddDays( GUCEF_INT32MIN ) );
            ASSERT_TRUE( date == CORE::CDate( 1970, 1, 1 ) );
            ASSERT_FALSE( date.FromDaysSinceUnixEpoch( GUCEF_INT32MAX ) );
            ASSERT_TRUE( date == CORE::CDate( 1970, 1, 1 ) );
            ASSERT_FALSE( date.FromDaysSinceUnixEpoch( GUCEF_INT32MAX - 719468 + 1 ) );
            ASSERT_FALSE( date.FromDaysSinceUnixEpoch( GUCEF_INT32MIN ) );
            ASSERT_TRUE( date == CORE::CDate( 1970, 1, 1 ) );
            ASSERT_FALSE( date.AddWeeks( GUCEF_INT32MAX / 7 ) );
            ASSERT_FALSE( date.AddWeeks( GUCEF_INT32MIN / 7 ) );
            ASSERT_TRUE( date == CORE::CDate( 1970, 1, 1 ) );

            // the largest date still in range keeps working
            CORE::CDate last( 32767, 12, 31 );
            Int32 lastDays = last.ToDaysSinceUnixEpoch();
            ASSERT_TRUE( date.FromDaysSinceUnixEpoch( lastDays ) && date == last );
            ASSERT_FALSE( date.FromDaysSinceUnixEpoch( lastDays + 1 ) );
            ASSERT_TRUE( date == last );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformDateTimeTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING DateTime TESTS" );
    
    GUCEF_TESTFW_SUITE_SCOPE( "CDateTime" );

    PerformDateTimeStringParsingTests();
    PerformDateCalendarTests();

    // Regression: on Linux the time difference 'from A towards B' had its sign inverted
    GUCEF_TESTFW_TESTCASE( "Test: Time difference towards a later time is positive" )
        try
        {
            CORE::CDateTime earlier( 2030, 1, 1, 0, 0, 0, 0, 0 );
            CORE::CDateTime later( 2030, 1, 1, 0, 0, 1, 500, 0 );
            ASSERT_TRUE( 1500 == earlier.GetTimeDifferenceInMillisecondsTowards( later ) );
            ASSERT_TRUE( -1500 == later.GetTimeDifferenceInMillisecondsTowards( earlier ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Regression: on Linux the day of the month was shifted by one in the conversions to and from the O/S
    GUCEF_TESTFW_TESTCASE( "Test: Conversion to and from unix time keeps the date" )
        try
        {
            // 2021-06-15 12:34:56 UTC
            const UInt64 knownTicks = 1623760496000ULL;
            CORE::CDateTime utc( 2021, 6, 15, 12, 34, 56, 0, 0 );
            ASSERT_TRUE( knownTicks == utc.ToUnixEpochBasedTicksInMillisecs() );

            CORE::CDateTime fromTicks;
            fromTicks.FromUnixEpochBasedTicksInMillisecs( knownTicks );
            ASSERT_TRUE( 2021 == fromTicks.GetYear() && 6 == fromTicks.GetMonth() && 15 == fromTicks.GetDay() );
            ASSERT_TRUE( 12 == fromTicks.GetHours() && 34 == fromTicks.GetMinutes() && 56 == fromTicks.GetSeconds() );

            CORE::CDateTime fromTimeT( (time_t) ( knownTicks / 1000 ), true );
            ASSERT_TRUE( 2021 == fromTimeT.GetYear() && 6 == fromTimeT.GetMonth() && 15 == fromTimeT.GetDay() );
            ASSERT_TRUE( 12 == fromTimeT.GetHours() && 34 == fromTimeT.GetMinutes() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Regression: on Linux the timezone offset was ignored when converting to UTC
    GUCEF_TESTFW_TESTCASE( "Test: A local time with a timezone offset converts to the right UTC time" )
        try
        {
            // 07:34:56 at UTC-5 is 12:34:56 UTC
            CORE::CDateTime local( 2021, 6, 15, 7, 34, 56, 0, -300 );
            ASSERT_TRUE( 1623760496000ULL == local.ToUnixEpochBasedTicksInMillisecs() );

            CORE::CDateTime utc = local.ToUTC();
            ASSERT_TRUE( 0 == utc.GetTimeZoneUTCOffsetInMins() );
            ASSERT_TRUE( 15 == utc.GetDay() && 12 == utc.GetHours() && 34 == utc.GetMinutes() );

            // the same moment in different timezones compares equal
            ASSERT_TRUE( local == CORE::CDateTime( 2021, 6, 15, 12, 34, 56, 0, 0 ) );

            // 23:30 at UTC-5 is the next day in UTC
            CORE::CDateTime lateEvening( 2021, 6, 15, 23, 30, 0, 0, -300 );
            CORE::CDateTime nextDay = lateEvening.ToUTC();
            ASSERT_TRUE( 16 == nextDay.GetDay() && 4 == nextDay.GetHours() && 30 == nextDay.GetMinutes() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Regression: on Linux the current date used the day of the week instead of the day of the month
    GUCEF_TESTFW_TESTCASE( "Test: The current local and UTC date-times match the C library" )
        try
        {
            time_t now = ::time( GUCEF_NULL );
            struct tm localNow = *::localtime( &now );
            struct tm utcNow = *::gmtime( &now );

            CORE::CDateTime local = CORE::CDateTime::NowLocalDateTime();
            CORE::CDateTime utc = CORE::CDateTime::NowUTCDateTime();
            CORE::CDate today = CORE::CDate::TodaysLocalDate();
            CORE::CDate todayUtc = CORE::CDate::TodaysUTCDate();

            // allow for the clock passing midnight during the test
            ASSERT_TRUE( localNow.tm_mday == local.GetDay() || ::time( GUCEF_NULL ) != now );
            ASSERT_TRUE( localNow.tm_hour == local.GetHours() || ::time( GUCEF_NULL ) != now );
            ASSERT_TRUE( utcNow.tm_mday == utc.GetDay() || ::time( GUCEF_NULL ) != now );
            ASSERT_TRUE( utcNow.tm_hour == utc.GetHours() || ::time( GUCEF_NULL ) != now );
            ASSERT_TRUE( localNow.tm_mday == today.GetDay() || ::time( GUCEF_NULL ) != now );
            ASSERT_TRUE( utcNow.tm_mday == todayUtc.GetDay() || ::time( GUCEF_NULL ) != now );

            // both describe the same moment
            Int64 differenceInMs = local.GetTimeDifferenceInMillisecondsTowards( utc );
            ASSERT_TRUE( differenceInMs > -2000 && differenceInMs < 2000 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL DateTime TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
