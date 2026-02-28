/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CCRONSCHEDULE_H
#include "gucefCORE_CCronSchedule.h"
#define GUCEF_CORE_CCRONSCHEDULE_H
#endif /* GUCEF_CORE_CCRONSCHEDULE_H ? */

#ifndef GUCEF_CORE_CDATETIMERANGE_H
#include "gucefCORE_CDateTimeRange.h"
#define GUCEF_CORE_CDATETIMERANGE_H
#endif /* GUCEF_CORE_CDATETIMERANGE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestCronSchedule.h"

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

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

void
PerformCronScheduleTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CCronSchedule TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "CCronSchedule" );

    // Reference datetimes used throughout tests
    // Wednesday, January 15, 2025, 02:00:00 UTC
    static const CORE::CDateTime ref1( 2025, 1, 15, 2, 0, 0, 0, 0 );
    // Wednesday, January 15, 2025, 02:01:30 UTC
    static const CORE::CDateTime ref2( 2025, 1, 15, 2, 1, 30, 0, 0 );
    // Wednesday, January 15, 2025, 02:30:00 UTC
    static const CORE::CDateTime ref3( 2025, 1, 15, 2, 30, 0, 0, 0 );

    // Test 1: CCronSchedule::Empty is invalid
    GUCEF_TESTFW_TESTCASE( "Test 1: CCronSchedule::Empty is invalid" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 1: CCronSchedule::Empty is invalid" );
            ASSERT_FALSE( CORE::CCronSchedule::Empty.IsValid() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 2: Default construction gives invalid (empty) schedule
    GUCEF_TESTFW_TESTCASE( "Test 2: Default construction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 2: Default construction" );
            CORE::CCronSchedule s;
            ASSERT_FALSE( s.IsValid() );
            ASSERT_TRUE( s.GetMinutes().empty() );
            ASSERT_TRUE( s.GetHours().empty() );
            ASSERT_TRUE( s.GetDaysOfMonth().empty() );
            ASSERT_TRUE( s.GetMonths().empty() );
            ASSERT_TRUE( s.GetDaysOfWeek().empty() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 3: String constructor with "* * * * *" produces valid schedule
    GUCEF_TESTFW_TESTCASE( "Test 3: String constructor wildcard expression" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 3: String constructor wildcard expression" );
            CORE::CCronSchedule s( CORE::CString( "* * * * *" ) );
            ASSERT_TRUE( s.IsValid() );
            // All 60 minutes (0-59) should be present
            ASSERT_TRUE( s.GetMinutes().size() == 60 );
            // All 24 hours (0-23) should be present
            ASSERT_TRUE( s.GetHours().size() == 24 );
            // All 31 days (1-31) should be present
            ASSERT_TRUE( s.GetDaysOfMonth().size() == 31 );
            // All 12 months (1-12) should be present
            ASSERT_TRUE( s.GetMonths().size() == 12 );
            // All 7 days of week (0-6) should be present
            ASSERT_TRUE( s.GetDaysOfWeek().size() == 7 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 4: FromCronString returns false for invalid expression
    GUCEF_TESTFW_TESTCASE( "Test 4: Invalid cron expression" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 4: Invalid cron expression" );
            CORE::CCronSchedule s;
            ASSERT_FALSE( s.FromCronString( CORE::CString( "not a cron" ) ) );
            ASSERT_FALSE( s.IsValid() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 5: Copy constructor preserves schedule equality
    GUCEF_TESTFW_TESTCASE( "Test 5: Copy constructor" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 5: Copy constructor" );
            CORE::CCronSchedule original( CORE::CString( "0 2 * * *" ) );
            CORE::CCronSchedule copy( original );
            ASSERT_TRUE( copy.IsValid() );
            ASSERT_TRUE( original == copy );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 6: Assignment operator
    GUCEF_TESTFW_TESTCASE( "Test 6: Assignment operator" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 6: Assignment operator" );
            CORE::CCronSchedule s1( CORE::CString( "0 2 * * *" ) );
            CORE::CCronSchedule s2;
            s2 = s1;
            ASSERT_TRUE( s2.IsValid() );
            ASSERT_TRUE( s1 == s2 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 7: operator== and operator!=
    GUCEF_TESTFW_TESTCASE( "Test 7: operator== and operator!=" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 7: operator== and operator!=" );
            CORE::CCronSchedule s1( CORE::CString( "0 2 * * *" ) );
            CORE::CCronSchedule s2( CORE::CString( "0 2 * * *" ) );
            CORE::CCronSchedule s3( CORE::CString( "0 3 * * *" ) );
            ASSERT_TRUE( s1 == s2 );
            ASSERT_FALSE( s1 == s3 );
            ASSERT_FALSE( s1 != s2 );
            ASSERT_TRUE( s1 != s3 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 8: FromCronString "0 2 * * *" - minute={0}, hour={2}
    GUCEF_TESTFW_TESTCASE( "Test 8: FromCronString daily at 2am" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 8: FromCronString daily at 2am" );
            CORE::CCronSchedule s;
            ASSERT_TRUE( s.FromCronString( CORE::CString( "0 2 * * *" ) ) );
            ASSERT_TRUE( s.IsValid() );
            ASSERT_TRUE( s.GetMinutes().size() == 1 );
            ASSERT_TRUE( s.GetMinutes().find( 0 ) != s.GetMinutes().end() );
            ASSERT_TRUE( s.GetHours().size() == 1 );
            ASSERT_TRUE( s.GetHours().find( 2 ) != s.GetHours().end() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 9: FromCronString "*/15 * * * *" - every 15 minutes -> {0,15,30,45}
    GUCEF_TESTFW_TESTCASE( "Test 9: FromCronString every 15 minutes" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 9: FromCronString every 15 minutes" );
            CORE::CCronSchedule s;
            ASSERT_TRUE( s.FromCronString( CORE::CString( "*/15 * * * *" ) ) );
            ASSERT_TRUE( s.IsValid() );
            ASSERT_TRUE( s.GetMinutes().size() == 4 );
            ASSERT_TRUE( s.GetMinutes().find(  0 ) != s.GetMinutes().end() );
            ASSERT_TRUE( s.GetMinutes().find( 15 ) != s.GetMinutes().end() );
            ASSERT_TRUE( s.GetMinutes().find( 30 ) != s.GetMinutes().end() );
            ASSERT_TRUE( s.GetMinutes().find( 45 ) != s.GetMinutes().end() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 10: FromCronString "0 9-17 * * 1-5" - business hours on weekdays
    GUCEF_TESTFW_TESTCASE( "Test 10: FromCronString range expression" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 10: FromCronString range expression" );
            CORE::CCronSchedule s;
            ASSERT_TRUE( s.FromCronString( CORE::CString( "0 9-17 * * 1-5" ) ) );
            ASSERT_TRUE( s.IsValid() );
            // hours should include 9 through 17 inclusive = 9 values
            ASSERT_TRUE( s.GetHours().size() == 9 );
            ASSERT_TRUE( s.GetHours().find(  9 ) != s.GetHours().end() );
            ASSERT_TRUE( s.GetHours().find( 17 ) != s.GetHours().end() );
            ASSERT_FALSE( s.GetHours().find(  8 ) != s.GetHours().end() );
            ASSERT_FALSE( s.GetHours().find( 18 ) != s.GetHours().end() );
            // days of week: Monday(1) through Friday(5) = 5 values
            ASSERT_TRUE( s.GetDaysOfWeek().size() == 5 );
            ASSERT_TRUE( s.GetDaysOfWeek().find( 1 ) != s.GetDaysOfWeek().end() );
            ASSERT_TRUE( s.GetDaysOfWeek().find( 5 ) != s.GetDaysOfWeek().end() );
            ASSERT_FALSE( s.GetDaysOfWeek().find( 0 ) != s.GetDaysOfWeek().end() );
            ASSERT_FALSE( s.GetDaysOfWeek().find( 6 ) != s.GetDaysOfWeek().end() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 11: FromCronString "0 8,12,18 * * *" - list expression
    GUCEF_TESTFW_TESTCASE( "Test 11: FromCronString list expression" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 11: FromCronString list expression" );
            CORE::CCronSchedule s;
            ASSERT_TRUE( s.FromCronString( CORE::CString( "0 8,12,18 * * *" ) ) );
            ASSERT_TRUE( s.IsValid() );
            ASSERT_TRUE( s.GetHours().size() == 3 );
            ASSERT_TRUE( s.GetHours().find(  8 ) != s.GetHours().end() );
            ASSERT_TRUE( s.GetHours().find( 12 ) != s.GetHours().end() );
            ASSERT_TRUE( s.GetHours().find( 18 ) != s.GetHours().end() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 12: ToCronString round-trip
    GUCEF_TESTFW_TESTCASE( "Test 12: ToCronString round-trip" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 12: ToCronString round-trip" );
            CORE::CCronSchedule s1;
            ASSERT_TRUE( s1.FromCronString( CORE::CString( "0 8,12,18 * * *" ) ) );
            CORE::CString cronStr = s1.ToCronString();
            ASSERT_FALSE( cronStr.IsNULLOrEmpty() );
            CORE::CCronSchedule s2;
            ASSERT_TRUE( s2.FromCronString( cronStr ) );
            ASSERT_TRUE( s1 == s2 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 13: Matches() - "* * * * *" matches any valid time
    GUCEF_TESTFW_TESTCASE( "Test 13: Matches wildcard matches any time" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 13: Matches wildcard matches any time" );
            CORE::CCronSchedule s( CORE::CString( "* * * * *" ) );
            ASSERT_TRUE( s.Matches( ref1 ) );
            ASSERT_TRUE( s.Matches( ref2 ) );
            ASSERT_TRUE( s.Matches( ref3 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 14: Matches() - "0 2 * * *" matches 02:00 but not 02:01
    GUCEF_TESTFW_TESTCASE( "Test 14: Matches specific time" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 14: Matches specific time" );
            // ref1 = 2025-01-15 02:00:00
            // ref2 = 2025-01-15 02:01:30
            CORE::CCronSchedule s;
            ASSERT_TRUE( s.FromCronString( CORE::CString( "0 2 * * *" ) ) );
            ASSERT_TRUE(  s.Matches( ref1 ) );  // 02:00 matches "0 2 * * *"
            ASSERT_FALSE( s.Matches( ref2 ) );  // 02:01 does NOT match
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 15: Matches() - "0 2 * * *" does not match 02:30
    GUCEF_TESTFW_TESTCASE( "Test 15: Matches non-matching time" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 15: Matches non-matching time" );
            // ref3 = 2025-01-15 02:30:00
            CORE::CCronSchedule s;
            ASSERT_TRUE( s.FromCronString( CORE::CString( "0 2 * * *" ) ) );
            ASSERT_FALSE( s.Matches( ref3 ) );  // minute=30, not in {0}
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 16: GetNextOccurrence - "* * * * *" from 02:00 -> 02:01
    GUCEF_TESTFW_TESTCASE( "Test 16: GetNextOccurrence every-minute from 02:00" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 16: GetNextOccurrence every-minute from 02:00" );
            // ref1 = 2025-01-15 02:00:00
            CORE::CCronSchedule s( CORE::CString( "* * * * *" ) );
            CORE::CDateTime next;
            ASSERT_TRUE( s.GetNextOccurrence( ref1, next ) );
            ASSERT_TRUE( next.GetHours()   == 2 );
            ASSERT_TRUE( next.GetMinutes() == 1 );
            ASSERT_TRUE( next.GetSeconds() == 0 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 17: GetNextOccurrence - "0 * * * *" from 02:30 -> 03:00
    GUCEF_TESTFW_TESTCASE( "Test 17: GetNextOccurrence hourly from 02:30" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 17: GetNextOccurrence hourly from 02:30" );
            // ref3 = 2025-01-15 02:30:00
            CORE::CCronSchedule s;
            ASSERT_TRUE( s.FromCronString( CORE::CString( "0 * * * *" ) ) );
            CORE::CDateTime next;
            ASSERT_TRUE( s.GetNextOccurrence( ref3, next ) );
            ASSERT_TRUE( next.GetHours()   == 3 );
            ASSERT_TRUE( next.GetMinutes() == 0 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 18: GetNextOccurrence returns false for invalid schedule
    GUCEF_TESTFW_TESTCASE( "Test 18: GetNextOccurrence on invalid schedule" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 18: GetNextOccurrence on invalid schedule" );
            CORE::CCronSchedule s;  // default: invalid
            CORE::CDateTime next;
            ASSERT_FALSE( s.GetNextOccurrence( ref1, next ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 19: GetPreviousOccurrence - "* * * * *" from 02:01:30 -> 02:00:00
    GUCEF_TESTFW_TESTCASE( "Test 19: GetPreviousOccurrence every-minute from 02:01:30" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 19: GetPreviousOccurrence every-minute from 02:01:30" );
            // ref2 = 2025-01-15 02:01:30
            // Implementation: seconds set to 0 then decrement one minute -> 02:00:00
            // "* * * * *" matches 02:00:00
            CORE::CCronSchedule s( CORE::CString( "* * * * *" ) );
            CORE::CDateTime prev;
            ASSERT_TRUE( s.GetPreviousOccurrence( ref2, prev ) );
            ASSERT_TRUE( prev.GetHours()   == 2 );
            ASSERT_TRUE( prev.GetMinutes() == 0 );
            ASSERT_TRUE( prev.GetSeconds() == 0 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 20: GetPreviousOccurrence - "0 * * * *" from 02:30 -> 02:00
    GUCEF_TESTFW_TESTCASE( "Test 20: GetPreviousOccurrence hourly from 02:30" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 20: GetPreviousOccurrence hourly from 02:30" );
            // ref3 = 2025-01-15 02:30:00
            // Implementation: seconds=0 -> 02:30, decrement minute -> 02:29
            // minute 29 not in {0}, find prev <= 28 in {0} -> 0, no wrap -> set minute=0 -> 02:00
            // 02:00 matches -> return true
            CORE::CCronSchedule s;
            ASSERT_TRUE( s.FromCronString( CORE::CString( "0 * * * *" ) ) );
            CORE::CDateTime prev;
            ASSERT_TRUE( s.GetPreviousOccurrence( ref3, prev ) );
            ASSERT_TRUE( prev.GetHours()   == 2 );
            ASSERT_TRUE( prev.GetMinutes() == 0 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 21: GetPreviousOccurrence returns false for invalid schedule
    GUCEF_TESTFW_TESTCASE( "Test 21: GetPreviousOccurrence on invalid schedule" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 21: GetPreviousOccurrence on invalid schedule" );
            CORE::CCronSchedule s;  // default: invalid
            CORE::CDateTime prev;
            ASSERT_FALSE( s.GetPreviousOccurrence( ref1, prev ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 22: GetNextOccurrences - "* * * * *" from 02:00 gives 3 consecutive minutes
    GUCEF_TESTFW_TESTCASE( "Test 22: GetNextOccurrences" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 22: GetNextOccurrences" );
            // ref1 = 2025-01-15 02:00:00
            CORE::CCronSchedule s( CORE::CString( "* * * * *" ) );
            CORE::CCronSchedule::TDateTimeVector occurrences;
            CORE::UInt32 found = s.GetNextOccurrences( ref1, 3, occurrences );
            ASSERT_TRUE( found == 3 );
            ASSERT_TRUE( occurrences.size() == 3 );
            // First occurrence: 02:01
            ASSERT_TRUE( occurrences[ 0 ].GetHours()   == 2 );
            ASSERT_TRUE( occurrences[ 0 ].GetMinutes() == 1 );
            // Second occurrence: 02:02
            ASSERT_TRUE( occurrences[ 1 ].GetHours()   == 2 );
            ASSERT_TRUE( occurrences[ 1 ].GetMinutes() == 2 );
            // Third occurrence: 02:03
            ASSERT_TRUE( occurrences[ 2 ].GetHours()   == 2 );
            ASSERT_TRUE( occurrences[ 2 ].GetMinutes() == 3 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 23: GetOccurrencesInRange - "0 * * * *" in a 3-hour window
    GUCEF_TESTFW_TESTCASE( "Test 23: GetOccurrencesInRange" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 23: GetOccurrencesInRange" );
            // Range: 2025-01-15 00:00:00 to 2025-01-15 03:00:00
            // "0 * * * *" should fire at 01:00 and 02:00 within this range
            CORE::CDateTime rangeStart( 2025, 1, 15, 0, 0, 0, 0, 0 );
            CORE::CDateTime rangeEnd(   2025, 1, 15, 3, 0, 0, 0, 0 );
            CORE::CDateTimeRange range( rangeStart, rangeEnd );

            CORE::CCronSchedule s;
            ASSERT_TRUE( s.FromCronString( CORE::CString( "0 * * * *" ) ) );
            CORE::CCronSchedule::TDateTimeVector occurrences;
            CORE::UInt32 found = s.GetOccurrencesInRange( range, 10, occurrences );
            ASSERT_TRUE( found >= 2 );
            // Each occurrence should have minute == 0
            for ( CORE::UInt32 i = 0; i < found; ++i )
            {
                ASSERT_TRUE( occurrences[ i ].GetMinutes() == 0 );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 24: Preset EveryMinute
    GUCEF_TESTFW_TESTCASE( "Test 24: Preset EveryMinute" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 24: Preset EveryMinute" );
            CORE::CCronSchedule s = CORE::CCronSchedule::EveryMinute();
            ASSERT_TRUE( s.IsValid() );
            ASSERT_TRUE( s.IsEveryMinute() );
            ASSERT_FALSE( s.IsEveryHour() );
            ASSERT_FALSE( s.IsDaily() );
            ASSERT_TRUE( s.GetMinutes().size() == 60 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 25: Preset Hourly
    GUCEF_TESTFW_TESTCASE( "Test 25: Preset Hourly" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 25: Preset Hourly" );
            CORE::CCronSchedule s = CORE::CCronSchedule::Hourly();
            ASSERT_TRUE( s.IsValid() );
            ASSERT_TRUE( s.IsEveryHour() );
            ASSERT_FALSE( s.IsEveryMinute() );
            ASSERT_FALSE( s.IsDaily() );
            // Hourly: minute=0, all hours
            ASSERT_TRUE( s.GetMinutes().size() == 1 );
            ASSERT_TRUE( s.GetMinutes().find( 0 ) != s.GetMinutes().end() );
            ASSERT_TRUE( s.GetHours().size() == 24 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 26: Preset Daily
    GUCEF_TESTFW_TESTCASE( "Test 26: Preset Daily" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 26: Preset Daily" );
            CORE::CCronSchedule s = CORE::CCronSchedule::Daily();
            ASSERT_TRUE( s.IsValid() );
            ASSERT_TRUE( s.IsDaily() );
            ASSERT_FALSE( s.IsEveryMinute() );
            ASSERT_FALSE( s.IsEveryHour() );
            // Daily: minute=0, hour=0, all others wildcard
            ASSERT_TRUE( s.GetMinutes().size() == 1 );
            ASSERT_TRUE( s.GetMinutes().find( 0 ) != s.GetMinutes().end() );
            ASSERT_TRUE( s.GetHours().size() == 1 );
            ASSERT_TRUE( s.GetHours().find( 0 ) != s.GetHours().end() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 27: Preset Weekly
    GUCEF_TESTFW_TESTCASE( "Test 27: Preset Weekly" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 27: Preset Weekly" );
            // "0 0 * * 0" - Sunday at midnight
            CORE::CCronSchedule s = CORE::CCronSchedule::Weekly();
            ASSERT_TRUE( s.IsValid() );
            ASSERT_TRUE( s.GetMinutes().size() == 1 );
            ASSERT_TRUE( s.GetMinutes().find( 0 ) != s.GetMinutes().end() );
            ASSERT_TRUE( s.GetHours().size() == 1 );
            ASSERT_TRUE( s.GetHours().find( 0 ) != s.GetHours().end() );
            ASSERT_TRUE( s.GetDaysOfWeek().size() == 1 );
            ASSERT_TRUE( s.GetDaysOfWeek().find( 0 ) != s.GetDaysOfWeek().end() );  // Sunday=0
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 28: Preset Monthly
    GUCEF_TESTFW_TESTCASE( "Test 28: Preset Monthly" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 28: Preset Monthly" );
            // "0 0 1 * *" - midnight on the 1st of every month
            CORE::CCronSchedule s = CORE::CCronSchedule::Monthly();
            ASSERT_TRUE( s.IsValid() );
            ASSERT_TRUE( s.GetDaysOfMonth().size() == 1 );
            ASSERT_TRUE( s.GetDaysOfMonth().find( 1 ) != s.GetDaysOfMonth().end() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 29: Preset Yearly
    GUCEF_TESTFW_TESTCASE( "Test 29: Preset Yearly" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 29: Preset Yearly" );
            // "0 0 1 1 *" - midnight on January 1st
            CORE::CCronSchedule s = CORE::CCronSchedule::Yearly();
            ASSERT_TRUE( s.IsValid() );
            ASSERT_TRUE( s.GetMonths().size() == 1 );
            ASSERT_TRUE( s.GetMonths().find( 1 ) != s.GetMonths().end() );
            ASSERT_TRUE( s.GetDaysOfMonth().size() == 1 );
            ASSERT_TRUE( s.GetDaysOfMonth().find( 1 ) != s.GetDaysOfMonth().end() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 30: Preset BusinessHours40hrWw
    GUCEF_TESTFW_TESTCASE( "Test 30: Preset BusinessHours40hrWw" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 30: Preset BusinessHours40hrWw" );
            // "0 9-17 * * 1-5" - 9am-5pm weekdays
            CORE::CCronSchedule s = CORE::CCronSchedule::BusinessHours40hrWw();
            ASSERT_TRUE( s.IsValid() );
            // 9 hours: 9,10,11,12,13,14,15,16,17
            ASSERT_TRUE( s.GetHours().size() == 9 );
            ASSERT_TRUE( s.GetHours().find(  9 ) != s.GetHours().end() );
            ASSERT_TRUE( s.GetHours().find( 17 ) != s.GetHours().end() );
            // 5 weekdays: Monday(1) through Friday(5)
            ASSERT_TRUE( s.GetDaysOfWeek().size() == 5 );
            ASSERT_TRUE( s.GetDaysOfWeek().find( 1 ) != s.GetDaysOfWeek().end() );
            ASSERT_TRUE( s.GetDaysOfWeek().find( 5 ) != s.GetDaysOfWeek().end() );
            ASSERT_FALSE( s.GetDaysOfWeek().find( 0 ) != s.GetDaysOfWeek().end() );
            ASSERT_FALSE( s.GetDaysOfWeek().find( 6 ) != s.GetDaysOfWeek().end() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 31: GetApproximateIntervalInMilliSecs for EveryMinute
    GUCEF_TESTFW_TESTCASE( "Test 31: GetApproximateIntervalInMilliSecs EveryMinute" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 31: GetApproximateIntervalInMilliSecs EveryMinute" );
            CORE::CCronSchedule s = CORE::CCronSchedule::EveryMinute();
            CORE::UInt64 intervalMs = s.GetApproximateIntervalInMilliSecs();
            ASSERT_TRUE( intervalMs == 60000 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 32: GetApproximateIntervalInMilliSecs for Hourly
    GUCEF_TESTFW_TESTCASE( "Test 32: GetApproximateIntervalInMilliSecs Hourly" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 32: GetApproximateIntervalInMilliSecs Hourly" );
            CORE::CCronSchedule s = CORE::CCronSchedule::Hourly();
            CORE::UInt64 intervalMs = s.GetApproximateIntervalInMilliSecs();
            ASSERT_TRUE( intervalMs == 3600000 );  // 60 * 60 * 1000
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 33: GetDescription - returns non-empty string for valid schedule
    GUCEF_TESTFW_TESTCASE( "Test 33: GetDescription" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 33: GetDescription" );
            CORE::CCronSchedule s = CORE::CCronSchedule::EveryMinute();
            CORE::CString desc = s.GetDescription();
            ASSERT_FALSE( desc.IsNULLOrEmpty() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 34: Clear() makes schedule invalid
    GUCEF_TESTFW_TESTCASE( "Test 34: Clear" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 34: Clear" );
            CORE::CCronSchedule s( CORE::CString( "* * * * *" ) );
            ASSERT_TRUE( s.IsValid() );
            s.Clear();
            ASSERT_FALSE( s.IsValid() );
            ASSERT_TRUE( s.GetMinutes().empty() );
            ASSERT_TRUE( s.GetHours().empty() );
            ASSERT_TRUE( s.GetDaysOfMonth().empty() );
            ASSERT_TRUE( s.GetMonths().empty() );
            ASSERT_TRUE( s.GetDaysOfWeek().empty() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 35: Field setters (SetMinutes, SetHours, etc.)
    GUCEF_TESTFW_TESTCASE( "Test 35: Field setters" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 35: Field setters" );
            CORE::CCronSchedule s;

            CORE::CCronSchedule::TUInt8Set minutes;
            minutes.insert( 0 );
            minutes.insert( 30 );
            s.SetMinutes( minutes );
            ASSERT_TRUE( s.GetMinutes().size() == 2 );
            ASSERT_TRUE( s.GetMinutes().find(  0 ) != s.GetMinutes().end() );
            ASSERT_TRUE( s.GetMinutes().find( 30 ) != s.GetMinutes().end() );

            CORE::CCronSchedule::TUInt8Set hours;
            for ( CORE::UInt8 h = 0; h < 24; ++h ) hours.insert( h );
            s.SetHours( hours );
            ASSERT_TRUE( s.GetHours().size() == 24 );

            CORE::CCronSchedule::TUInt8Set days;
            for ( CORE::UInt8 d = 1; d <= 31; ++d ) days.insert( d );
            s.SetDaysOfMonth( days );
            ASSERT_TRUE( s.GetDaysOfMonth().size() == 31 );

            CORE::CCronSchedule::TUInt8Set months;
            for ( CORE::UInt8 m = 1; m <= 12; ++m ) months.insert( m );
            s.SetMonths( months );
            ASSERT_TRUE( s.GetMonths().size() == 12 );

            CORE::CCronSchedule::TUInt8Set dow;
            for ( CORE::UInt8 w = 0; w < 7; ++w ) dow.insert( w );
            s.SetDaysOfWeek( dow );
            ASSERT_TRUE( s.GetDaysOfWeek().size() == 7 );

            ASSERT_TRUE( s.IsValid() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 36: GetMillisecondsUntilNext(from) > 0 for valid schedule
    GUCEF_TESTFW_TESTCASE( "Test 36: GetMillisecondsUntilNext from fixed time" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 36: GetMillisecondsUntilNext from fixed time" );
            // "* * * * *" from 02:00:00 -> next is 02:01:00 = 60000ms
            CORE::CCronSchedule s( CORE::CString( "* * * * *" ) );
            CORE::UInt64 msUntilNext = s.GetMillisecondsUntilNext( ref1 );
            ASSERT_TRUE( msUntilNext == 60000 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 37: GetMillisecondsUntilNext(from) returns 0 for invalid schedule
    GUCEF_TESTFW_TESTCASE( "Test 37: GetMillisecondsUntilNext on invalid schedule" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 37: GetMillisecondsUntilNext on invalid schedule" );
            CORE::CCronSchedule s;  // default: invalid
            CORE::UInt64 msUntilNext = s.GetMillisecondsUntilNext( ref1 );
            ASSERT_TRUE( msUntilNext == 0 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
}

/*-------------------------------------------------------------------------*/
