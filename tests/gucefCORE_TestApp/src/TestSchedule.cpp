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

#ifndef GUCEF_CORE_CSCHEDULE_H
#include "gucefCORE_CSchedule.h"
#define GUCEF_CORE_CSCHEDULE_H
#endif /* GUCEF_CORE_CSCHEDULE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestSchedule.h"

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
PerformScheduleTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CSchedule TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "CSchedule" );

    // Reference datetimes (all UTC)
    //   dt1 = 2025-01-15 01:00:00
    //   dt2 = 2025-01-15 02:00:00
    //   dt3 = 2025-01-15 02:30:00
    //   dt4 = 2025-01-15 03:00:00
    static const CORE::CDateTime dt1( 2025, 1, 15, 1, 0, 0, 0, 0 );
    static const CORE::CDateTime dt2( 2025, 1, 15, 2, 0, 0, 0, 0 );
    static const CORE::CDateTime dt3( 2025, 1, 15, 2, 30, 0, 0, 0 );
    static const CORE::CDateTime dt4( 2025, 1, 15, 3, 0, 0, 0, 0 );

    // Test 1: CSchedule::Empty is not valid
    GUCEF_TESTFW_TESTCASE( "Test 1: CSchedule::Empty" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 1: CSchedule::Empty" );
            ASSERT_FALSE( CORE::CSchedule::Empty.IsValid() );
            ASSERT_FALSE( CORE::CSchedule::Empty.HasCronSchedule() );
            ASSERT_FALSE( CORE::CSchedule::Empty.HasTimeRange() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 2: Default construction is not valid
    GUCEF_TESTFW_TESTCASE( "Test 2: Default construction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 2: Default construction" );
            CORE::CSchedule s;
            ASSERT_FALSE( s.IsValid() );
            ASSERT_FALSE( s.HasCronSchedule() );
            ASSERT_FALSE( s.HasTimeRange() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 3: Cron-only constructor
    GUCEF_TESTFW_TESTCASE( "Test 3: Cron-only constructor" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 3: Cron-only constructor" );
            CORE::CCronSchedule cron( CORE::CString( "* * * * *" ) );
            CORE::CSchedule s( cron );
            ASSERT_TRUE(  s.IsValid() );
            ASSERT_TRUE(  s.HasCronSchedule() );
            ASSERT_FALSE( s.HasTimeRange() );
            ASSERT_TRUE(  s.GetCronSchedule() == cron );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 4: TimeRange-only constructor
    GUCEF_TESTFW_TESTCASE( "Test 4: TimeRange-only constructor" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 4: TimeRange-only constructor" );
            CORE::CDateTimeRange range( dt2, dt4 );
            CORE::CSchedule s( range );
            ASSERT_TRUE(  s.IsValid() );
            ASSERT_FALSE( s.HasCronSchedule() );
            ASSERT_TRUE(  s.HasTimeRange() );
            ASSERT_TRUE(  s.GetTimeRange() == range );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 5: Combined constructor (cron + time range)
    GUCEF_TESTFW_TESTCASE( "Test 5: Combined constructor" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 5: Combined constructor" );
            CORE::CCronSchedule cron( CORE::CString( "0 * * * *" ) );
            CORE::CDateTimeRange range( dt1, dt4 );
            CORE::CSchedule s( cron, range );
            ASSERT_TRUE( s.IsValid() );
            ASSERT_TRUE( s.HasCronSchedule() );
            ASSERT_TRUE( s.HasTimeRange() );
            ASSERT_TRUE( s.GetCronSchedule() == cron );
            ASSERT_TRUE( s.GetTimeRange()    == range );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 6: Copy constructor
    GUCEF_TESTFW_TESTCASE( "Test 6: Copy constructor" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 6: Copy constructor" );
            CORE::CCronSchedule cron( CORE::CString( "* * * * *" ) );
            CORE::CSchedule s1( cron );
            CORE::CSchedule s2( s1 );
            ASSERT_TRUE( s1 == s2 );
            ASSERT_TRUE( s2.HasCronSchedule() );
            ASSERT_FALSE( s2.HasTimeRange() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 7: Assignment operator
    GUCEF_TESTFW_TESTCASE( "Test 7: Assignment operator" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 7: Assignment operator" );
            CORE::CCronSchedule cron( CORE::CString( "* * * * *" ) );
            CORE::CSchedule s1( cron );
            CORE::CSchedule s2;
            s2 = s1;
            ASSERT_TRUE( s1 == s2 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 8: operator== - identical schedules are equal
    GUCEF_TESTFW_TESTCASE( "Test 8: operator==" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 8: operator==" );
            CORE::CCronSchedule cron( CORE::CString( "0 2 * * *" ) );
            CORE::CSchedule s1( cron );
            CORE::CSchedule s2( cron );
            ASSERT_TRUE( s1 == s2 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 9: operator!= - different schedules are not equal
    GUCEF_TESTFW_TESTCASE( "Test 9: operator!=" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 9: operator!=" );
            CORE::CSchedule s1( CORE::CCronSchedule( CORE::CString( "0 2 * * *" ) ) );
            CORE::CSchedule s2( CORE::CCronSchedule( CORE::CString( "0 3 * * *" ) ) );
            CORE::CSchedule s3( CORE::CDateTimeRange( dt2, dt4 ) );
            ASSERT_TRUE(  s1 != s2 );   // different cron
            ASSERT_TRUE(  s1 != s3 );   // cron vs range
            ASSERT_FALSE( s1 != s1 );   // same object
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 10: operator< - schedule without time range sorts before one with time range
    GUCEF_TESTFW_TESTCASE( "Test 10: operator< ordering" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 10: operator< ordering" );
            CORE::CSchedule cronOnly( CORE::CCronSchedule( CORE::CString( "* * * * *" ) ) );
            CORE::CSchedule rangeOnly( CORE::CDateTimeRange( dt2, dt4 ) );
            // No-range schedule sorts before with-range schedule
            ASSERT_TRUE(  cronOnly < rangeOnly );
            ASSERT_FALSE( rangeOnly < cronOnly );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 11: operator<= and operator>=
    GUCEF_TESTFW_TESTCASE( "Test 11: operator<= and operator>=" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 11: operator<= and operator>=" );
            CORE::CSchedule s1( CORE::CCronSchedule( CORE::CString( "* * * * *" ) ) );
            CORE::CSchedule s2( CORE::CCronSchedule( CORE::CString( "* * * * *" ) ) );
            ASSERT_TRUE( s1 <= s2 );    // equal is <=
            ASSERT_TRUE( s1 >= s2 );    // equal is >=
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 12: SetCronSchedule / GetCronSchedule
    GUCEF_TESTFW_TESTCASE( "Test 12: SetCronSchedule and GetCronSchedule" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 12: SetCronSchedule and GetCronSchedule" );
            CORE::CSchedule s;
            ASSERT_FALSE( s.HasCronSchedule() );
            CORE::CCronSchedule cron( CORE::CString( "0 2 * * *" ) );
            s.SetCronSchedule( cron );
            ASSERT_TRUE( s.HasCronSchedule() );
            ASSERT_TRUE( s.GetCronSchedule() == cron );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 13: SetTimeRange / GetTimeRange
    GUCEF_TESTFW_TESTCASE( "Test 13: SetTimeRange and GetTimeRange" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 13: SetTimeRange and GetTimeRange" );
            CORE::CSchedule s;
            ASSERT_FALSE( s.HasTimeRange() );
            CORE::CDateTimeRange range( dt2, dt4 );
            s.SetTimeRange( range );
            ASSERT_TRUE( s.HasTimeRange() );
            ASSERT_TRUE( s.GetTimeRange() == range );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 14: IsActive - no time range means always active
    GUCEF_TESTFW_TESTCASE( "Test 14: IsActive no time range" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 14: IsActive no time range" );
            CORE::CSchedule s( CORE::CCronSchedule( CORE::CString( "* * * * *" ) ) );
            ASSERT_TRUE( s.IsActive( dt1 ) );
            ASSERT_TRUE( s.IsActive( dt2 ) );
            ASSERT_TRUE( s.IsActive( dt3 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 15: IsActive - with time range [02:00, 03:00]
    GUCEF_TESTFW_TESTCASE( "Test 15: IsActive with time range" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 15: IsActive with time range" );
            // range [02:00, 03:00]
            CORE::CDateTimeRange range( dt2, dt4 );
            CORE::CSchedule s( range );
            ASSERT_FALSE( s.IsActive( dt1 ) );   // 01:00 is before range
            ASSERT_TRUE(  s.IsActive( dt2 ) );   // 02:00 is at range start (inclusive)
            ASSERT_TRUE(  s.IsActive( dt3 ) );   // 02:30 is within range
            ASSERT_TRUE(  s.IsActive( dt4 ) );   // 03:00 is at range end (inclusive)
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 16: HasStarted / HasEnded - no time range
    GUCEF_TESTFW_TESTCASE( "Test 16: HasStarted and HasEnded no time range" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 16: HasStarted and HasEnded no time range" );
            CORE::CSchedule s( CORE::CCronSchedule( CORE::CString( "* * * * *" ) ) );
            ASSERT_TRUE(  s.HasStarted() );  // no time range -> always started
            ASSERT_FALSE( s.HasEnded() );    // no time range -> never ends
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 17: HasStarted / HasEnded - past range (2024)
    GUCEF_TESTFW_TESTCASE( "Test 17: HasStarted and HasEnded past range" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 17: HasStarted and HasEnded past range" );
            CORE::CDateTime past1( 2024, 1,  1, 0, 0, 0, 0, 0 );
            CORE::CDateTime past2( 2024, 12, 31, 23, 59, 0, 0, 0 );
            CORE::CSchedule s( CORE::CDateTimeRange( past1, past2 ) );
            ASSERT_TRUE( s.HasStarted() );
            ASSERT_TRUE( s.HasEnded() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 18: HasStarted / HasEnded - future range (2030)
    GUCEF_TESTFW_TESTCASE( "Test 18: HasStarted and HasEnded future range" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 18: HasStarted and HasEnded future range" );
            CORE::CDateTime future1( 2030, 1, 1, 0, 0, 0, 0, 0 );
            CORE::CDateTime future2( 2031, 1, 1, 0, 0, 0, 0, 0 );
            CORE::CSchedule s( CORE::CDateTimeRange( future1, future2 ) );
            ASSERT_FALSE( s.HasStarted() );
            ASSERT_FALSE( s.HasEnded() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 19: Matches - cron only, "* * * * *" matches any minute
    GUCEF_TESTFW_TESTCASE( "Test 19: Matches cron-only wildcard" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 19: Matches cron-only wildcard" );
            CORE::CSchedule s( CORE::CCronSchedule( CORE::CString( "* * * * *" ) ) );
            ASSERT_TRUE( s.Matches( dt1 ) );
            ASSERT_TRUE( s.Matches( dt2 ) );
            ASSERT_TRUE( s.Matches( dt3 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 20: Matches - cron only "0 2 * * *" matches dt2 (02:00) but not dt3 (02:30)
    GUCEF_TESTFW_TESTCASE( "Test 20: Matches cron-only specific time" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 20: Matches cron-only specific time" );
            CORE::CSchedule s( CORE::CCronSchedule( CORE::CString( "0 2 * * *" ) ) );
            ASSERT_TRUE(  s.Matches( dt2 ) );   // 02:00 matches
            ASSERT_FALSE( s.Matches( dt3 ) );   // 02:30 does not match (minute=30, not 0)
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 21: Matches - time range only fires only at the start time
    GUCEF_TESTFW_TESTCASE( "Test 21: Matches time-range-only" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 21: Matches time-range-only" );
            // Range-only schedule fires only at the exact start time
            CORE::CSchedule s( CORE::CDateTimeRange( dt2, dt4 ) );
            ASSERT_TRUE(  s.Matches( dt2 ) );   // exactly at start -> match
            ASSERT_FALSE( s.Matches( dt3 ) );   // mid-range -> no match (only start fires)
            ASSERT_FALSE( s.Matches( dt1 ) );   // before range -> no match
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 22: Matches - combined: cron "0 * * * *" within range [01:00, 03:00]
    GUCEF_TESTFW_TESTCASE( "Test 22: Matches combined cron and time range" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 22: Matches combined cron and time range" );
            // "0 * * * *" fires at the top of every hour
            // Range [01:00, 03:00] - dt2(02:00) is in range and on the hour
            CORE::CCronSchedule cron;
            ASSERT_TRUE( cron.FromCronString( CORE::CString( "0 * * * *" ) ) );
            CORE::CSchedule s( cron, CORE::CDateTimeRange( dt1, dt4 ) );
            ASSERT_TRUE(  s.Matches( dt2 ) );   // 02:00: in range AND matches cron
            ASSERT_FALSE( s.Matches( dt3 ) );   // 02:30: in range but does not match cron
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 23: Matches - combined: cron fires but outside time range
    GUCEF_TESTFW_TESTCASE( "Test 23: Matches combined cron fires outside time range" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 23: Matches combined cron fires outside time range" );
            // "0 * * * *" fires at 01:00 and 02:00, range is [02:00, 03:00]
            // dt1(01:00) fires cron but is before range start -> no match
            CORE::CCronSchedule cron;
            ASSERT_TRUE( cron.FromCronString( CORE::CString( "0 * * * *" ) ) );
            CORE::CSchedule s( cron, CORE::CDateTimeRange( dt2, dt4 ) );
            ASSERT_FALSE( s.Matches( dt1 ) );   // 01:00: matches cron but outside range
            ASSERT_TRUE(  s.Matches( dt2 ) );   // 02:00: matches cron AND in range
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 24: GetNextOccurrence - cron only "* * * * *" from dt2 -> dt2+1min
    GUCEF_TESTFW_TESTCASE( "Test 24: GetNextOccurrence cron-only" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 24: GetNextOccurrence cron-only" );
            CORE::CSchedule s( CORE::CCronSchedule( CORE::CString( "* * * * *" ) ) );
            CORE::CDateTime next;
            ASSERT_TRUE( s.GetNextOccurrence( dt2, next ) );
            ASSERT_TRUE( next.GetHours()   == 2 );
            ASSERT_TRUE( next.GetMinutes() == 1 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 25: GetNextOccurrence - time range only, from before start -> returns start
    GUCEF_TESTFW_TESTCASE( "Test 25: GetNextOccurrence time-range-only from before start" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 25: GetNextOccurrence time-range-only from before start" );
            // Range starts at dt2 (02:00), querying from dt1 (01:00)
            CORE::CSchedule s( CORE::CDateTimeRange( dt2, dt4 ) );
            CORE::CDateTime next;
            ASSERT_TRUE( s.GetNextOccurrence( dt1, next ) );
            ASSERT_TRUE( next == dt2 );   // next is the range start
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 26: GetNextOccurrence - time range only, from after start -> no next occurrence
    GUCEF_TESTFW_TESTCASE( "Test 26: GetNextOccurrence time-range-only from after start" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 26: GetNextOccurrence time-range-only from after start" );
            // Range starts at dt2, querying from dt3 (02:30) - already past the start
            CORE::CSchedule s( CORE::CDateTimeRange( dt2, dt4 ) );
            CORE::CDateTime next;
            ASSERT_FALSE( s.GetNextOccurrence( dt3, next ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 27: GetPreviousOccurrence - cron only "* * * * *" from dt3 (02:30:00) -> 02:29:00
    GUCEF_TESTFW_TESTCASE( "Test 27: GetPreviousOccurrence cron-only" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 27: GetPreviousOccurrence cron-only" );
            // dt3 = 02:30:00, prev minute is 02:29:00
            CORE::CSchedule s( CORE::CCronSchedule( CORE::CString( "* * * * *" ) ) );
            CORE::CDateTime prev;
            ASSERT_TRUE( s.GetPreviousOccurrence( dt3, prev ) );
            ASSERT_TRUE( prev.GetHours()   == 2 );
            ASSERT_TRUE( prev.GetMinutes() == 29 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 28: GetPreviousOccurrence - time range only, from after start -> returns start
    GUCEF_TESTFW_TESTCASE( "Test 28: GetPreviousOccurrence time-range-only from after start" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 28: GetPreviousOccurrence time-range-only from after start" );
            // Range starts at dt2 (02:00), querying from dt3 (02:30) - past start -> prev = start
            CORE::CSchedule s( CORE::CDateTimeRange( dt2, dt4 ) );
            CORE::CDateTime prev;
            ASSERT_TRUE( s.GetPreviousOccurrence( dt3, prev ) );
            ASSERT_TRUE( prev == dt2 );   // previous is the range start
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 29: GetPreviousOccurrence - time range only, from before start -> no previous
    GUCEF_TESTFW_TESTCASE( "Test 29: GetPreviousOccurrence time-range-only from before start" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 29: GetPreviousOccurrence time-range-only from before start" );
            // Querying from dt1 (01:00) which is before range start dt2 (02:00)
            CORE::CSchedule s( CORE::CDateTimeRange( dt2, dt4 ) );
            CORE::CDateTime prev;
            ASSERT_FALSE( s.GetPreviousOccurrence( dt1, prev ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 30: GetNextOccurrences - cron only, 3 occurrences
    GUCEF_TESTFW_TESTCASE( "Test 30: GetNextOccurrences cron-only" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 30: GetNextOccurrences cron-only" );
            CORE::CSchedule s( CORE::CCronSchedule( CORE::CString( "* * * * *" ) ) );
            CORE::CSchedule::TDateTimeVector occurrences;
            CORE::UInt32 found = s.GetNextOccurrences( dt2, 3, occurrences );
            ASSERT_TRUE( found == 3 );
            ASSERT_TRUE( occurrences[ 0 ].GetMinutes() == 1 );
            ASSERT_TRUE( occurrences[ 1 ].GetMinutes() == 2 );
            ASSERT_TRUE( occurrences[ 2 ].GetMinutes() == 3 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 31: GetNextOccurrences - time range only returns single occurrence
    GUCEF_TESTFW_TESTCASE( "Test 31: GetNextOccurrences time-range-only" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 31: GetNextOccurrences time-range-only" );
            // Range starts at dt2; from dt1 (before start), should get 1 occurrence = dt2
            CORE::CSchedule s( CORE::CDateTimeRange( dt2, dt4 ) );
            CORE::CSchedule::TDateTimeVector occurrences;
            CORE::UInt32 found = s.GetNextOccurrences( dt1, 5, occurrences );
            ASSERT_TRUE( found == 1 );
            ASSERT_TRUE( occurrences[ 0 ] == dt2 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 32: GetMillisecondsUntilNext - cron only, "* * * * *" from dt2 -> 60000ms
    GUCEF_TESTFW_TESTCASE( "Test 32: GetMillisecondsUntilNext cron-only" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 32: GetMillisecondsUntilNext cron-only" );
            CORE::CSchedule s( CORE::CCronSchedule( CORE::CString( "* * * * *" ) ) );
            CORE::UInt64 ms = s.GetMillisecondsUntilNext( dt2 );
            ASSERT_TRUE( ms == 60000 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 33: GetMillisecondsUntilStart - no time range returns 0
    GUCEF_TESTFW_TESTCASE( "Test 33: GetMillisecondsUntilStart no time range" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 33: GetMillisecondsUntilStart no time range" );
            CORE::CSchedule s( CORE::CCronSchedule( CORE::CString( "* * * * *" ) ) );
            ASSERT_TRUE( s.GetMillisecondsUntilStart() == 0 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 34: GetMillisecondsUntilStart - future range returns > 0
    GUCEF_TESTFW_TESTCASE( "Test 34: GetMillisecondsUntilStart future range" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 34: GetMillisecondsUntilStart future range" );
            CORE::CDateTime future1( 2030, 1, 1, 0, 0, 0, 0, 0 );
            CORE::CDateTime future2( 2031, 1, 1, 0, 0, 0, 0, 0 );
            CORE::CSchedule s( CORE::CDateTimeRange( future1, future2 ) );
            ASSERT_TRUE( s.GetMillisecondsUntilStart() > 0 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 35: GetMillisecondsUntilEnd - no time range returns 0
    GUCEF_TESTFW_TESTCASE( "Test 35: GetMillisecondsUntilEnd no time range" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 35: GetMillisecondsUntilEnd no time range" );
            CORE::CSchedule s( CORE::CCronSchedule( CORE::CString( "* * * * *" ) ) );
            ASSERT_TRUE( s.GetMillisecondsUntilEnd() == 0 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 36: GetScheduleDurationInMilliSecs - no time range returns 0
    GUCEF_TESTFW_TESTCASE( "Test 36: GetScheduleDurationInMilliSecs no time range" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 36: GetScheduleDurationInMilliSecs no time range" );
            CORE::CSchedule s( CORE::CCronSchedule( CORE::CString( "* * * * *" ) ) );
            ASSERT_TRUE( s.GetScheduleDurationInMilliSecs() == 0 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 37: GetScheduleDurationInMilliSecs - 1-hour range = 3,600,000 ms
    GUCEF_TESTFW_TESTCASE( "Test 37: GetScheduleDurationInMilliSecs with time range" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 37: GetScheduleDurationInMilliSecs with time range" );
            // dt2(02:00) to dt4(03:00) = 1 hour = 3,600,000 ms
            CORE::CSchedule s( CORE::CDateTimeRange( dt2, dt4 ) );
            ASSERT_TRUE( s.GetScheduleDurationInMilliSecs() == 3600000 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 38: IsValid - various states
    GUCEF_TESTFW_TESTCASE( "Test 38: IsValid various states" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 38: IsValid various states" );
            CORE::CSchedule empty;
            ASSERT_FALSE( empty.IsValid() );  // no components

            CORE::CSchedule cronOnly( CORE::CCronSchedule( CORE::CString( "* * * * *" ) ) );
            ASSERT_TRUE( cronOnly.IsValid() );

            CORE::CSchedule rangeOnly( CORE::CDateTimeRange( dt2, dt4 ) );
            ASSERT_TRUE( rangeOnly.IsValid() );

            CORE::CSchedule combined(
                CORE::CCronSchedule( CORE::CString( "* * * * *" ) ),
                CORE::CDateTimeRange( dt2, dt4 ) );
            ASSERT_TRUE( combined.IsValid() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 39: GetDescription returns non-empty for valid schedules
    GUCEF_TESTFW_TESTCASE( "Test 39: GetDescription" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 39: GetDescription" );
            CORE::CSchedule cronOnly( CORE::CCronSchedule( CORE::CString( "* * * * *" ) ) );
            ASSERT_FALSE( cronOnly.GetDescription().IsNULLOrEmpty() );

            CORE::CSchedule rangeOnly( CORE::CDateTimeRange( dt2, dt4 ) );
            ASSERT_FALSE( rangeOnly.GetDescription().IsNULLOrEmpty() );

            CORE::CSchedule combined(
                CORE::CCronSchedule( CORE::CString( "0 2 * * *" ) ),
                CORE::CDateTimeRange( dt2, dt4 ) );
            ASSERT_FALSE( combined.GetDescription().IsNULLOrEmpty() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 40: Clear() resets everything
    GUCEF_TESTFW_TESTCASE( "Test 40: Clear" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 40: Clear" );
            CORE::CSchedule s(
                CORE::CCronSchedule( CORE::CString( "* * * * *" ) ),
                CORE::CDateTimeRange( dt2, dt4 ) );
            ASSERT_TRUE(  s.IsValid() );
            ASSERT_TRUE(  s.HasCronSchedule() );
            ASSERT_TRUE(  s.HasTimeRange() );
            s.Clear();
            ASSERT_FALSE( s.IsValid() );
            ASSERT_FALSE( s.HasCronSchedule() );
            ASSERT_FALSE( s.HasTimeRange() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
}

/*-------------------------------------------------------------------------*/
