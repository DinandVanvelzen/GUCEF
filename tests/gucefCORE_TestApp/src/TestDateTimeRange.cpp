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

#include "TestDateTimeRange.h"

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
PerformDateTimeRangeTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CDateTimeRange TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "CDateTimeRange" );

    // Reference datetimes used throughout the tests (all UTC, timezone offset = 0)
    //   dt1 = 2025-01-15 01:00:00
    //   dt2 = 2025-01-15 02:00:00
    //   dt3 = 2025-01-15 02:30:00
    //   dt4 = 2025-01-15 03:00:00
    //   dt5 = 2025-01-15 04:00:00
    static const CORE::CDateTime dt1( 2025, 1, 15, 1, 0, 0, 0, 0 );
    static const CORE::CDateTime dt2( 2025, 1, 15, 2, 0, 0, 0, 0 );
    static const CORE::CDateTime dt3( 2025, 1, 15, 2, 30, 0, 0, 0 );
    static const CORE::CDateTime dt4( 2025, 1, 15, 3, 0, 0, 0, 0 );
    static const CORE::CDateTime dt5( 2025, 1, 15, 4, 0, 0, 0, 0 );

    // Test 1: CDateTimeRange::Empty - default-constructed, endpoints equal default CDateTime
    GUCEF_TESTFW_TESTCASE( "Test 1: CDateTimeRange::Empty" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 1: CDateTimeRange::Empty" );
            ASSERT_TRUE( CORE::CDateTimeRange::Empty == CORE::CDateTimeRange() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 2: Default construction - start and end are the same default value
    GUCEF_TESTFW_TESTCASE( "Test 2: Default construction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 2: Default construction" );
            CORE::CDateTimeRange r;
            ASSERT_TRUE( r.GetStart() == r.GetEnd() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 3: Two-argument constructor stores start and end correctly
    GUCEF_TESTFW_TESTCASE( "Test 3: Two-argument constructor" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 3: Two-argument constructor" );
            CORE::CDateTimeRange r( dt2, dt4 );
            ASSERT_TRUE( r.GetStart() == dt2 );
            ASSERT_TRUE( r.GetEnd()   == dt4 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 4: Copy constructor produces equal range
    GUCEF_TESTFW_TESTCASE( "Test 4: Copy constructor" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 4: Copy constructor" );
            CORE::CDateTimeRange r1( dt2, dt4 );
            CORE::CDateTimeRange r2( r1 );
            ASSERT_TRUE( r1 == r2 );
            ASSERT_TRUE( r2.GetStart() == dt2 );
            ASSERT_TRUE( r2.GetEnd()   == dt4 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 5: Assignment operator
    GUCEF_TESTFW_TESTCASE( "Test 5: Assignment operator" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 5: Assignment operator" );
            CORE::CDateTimeRange r1( dt2, dt4 );
            CORE::CDateTimeRange r2;
            r2 = r1;
            ASSERT_TRUE( r1 == r2 );
            ASSERT_TRUE( r2.GetStart() == dt2 );
            ASSERT_TRUE( r2.GetEnd()   == dt4 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 6: operator== - identical ranges are equal
    GUCEF_TESTFW_TESTCASE( "Test 6: operator==" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 6: operator==" );
            CORE::CDateTimeRange r1( dt2, dt4 );
            CORE::CDateTimeRange r2( dt2, dt4 );
            ASSERT_TRUE( r1 == r2 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 7: operator!= - ranges with different start/end are not equal
    GUCEF_TESTFW_TESTCASE( "Test 7: operator!=" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 7: operator!=" );
            CORE::CDateTimeRange r1( dt2, dt4 );
            CORE::CDateTimeRange r2( dt1, dt4 );
            CORE::CDateTimeRange r3( dt2, dt5 );
            ASSERT_TRUE( r1 != r2 );
            ASSERT_TRUE( r1 != r3 );
            ASSERT_FALSE( r1 != r1 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 8: operator< (range vs range) - earlier start means less
    GUCEF_TESTFW_TESTCASE( "Test 8: operator< range vs range" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 8: operator< range vs range" );
            CORE::CDateTimeRange r1( dt1, dt4 );   // starts at 01:00
            CORE::CDateTimeRange r2( dt2, dt4 );   // starts at 02:00
            CORE::CDateTimeRange r3( dt2, dt5 );   // same start, later end
            ASSERT_TRUE(  r1 < r2 );     // earlier start
            ASSERT_FALSE( r2 < r1 );
            ASSERT_TRUE(  r2 < r3 );     // same start, shorter range (earlier end)
            ASSERT_FALSE( r3 < r2 );
            ASSERT_FALSE( r2 < r2 );     // same range
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 9: operator<= (range vs range)
    GUCEF_TESTFW_TESTCASE( "Test 9: operator<= range vs range" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 9: operator<= range vs range" );
            CORE::CDateTimeRange r1( dt1, dt4 );
            CORE::CDateTimeRange r2( dt2, dt4 );
            ASSERT_TRUE(  r1 <= r2 );
            ASSERT_TRUE(  r2 <= r2 );   // equal is <=
            ASSERT_FALSE( r2 <= r1 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 10: operator> (range vs range)
    GUCEF_TESTFW_TESTCASE( "Test 10: operator> range vs range" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 10: operator> range vs range" );
            CORE::CDateTimeRange r1( dt1, dt4 );
            CORE::CDateTimeRange r2( dt2, dt4 );
            ASSERT_TRUE(  r2 > r1 );
            ASSERT_FALSE( r1 > r2 );
            ASSERT_FALSE( r2 > r2 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 11: operator>= (range vs range)
    GUCEF_TESTFW_TESTCASE( "Test 11: operator>= range vs range" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 11: operator>= range vs range" );
            CORE::CDateTimeRange r1( dt1, dt4 );
            CORE::CDateTimeRange r2( dt2, dt4 );
            ASSERT_TRUE(  r2 >= r1 );
            ASSERT_TRUE(  r2 >= r2 );   // equal is >=
            ASSERT_FALSE( r1 >= r2 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 12: operator< (range vs CDateTime)
    //   "range is entirely before dt" means range.end < dt
    GUCEF_TESTFW_TESTCASE( "Test 12: operator< range vs CDateTime" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 12: operator< range vs CDateTime" );
            // range (02:00, 03:00) entirely before dt5 (04:00)
            CORE::CDateTimeRange r( dt2, dt4 );
            ASSERT_TRUE(  r < dt5 );    // end(03:00) < dt5(04:00) = true
            ASSERT_FALSE( r < dt4 );    // end(03:00) < dt4(03:00) = false (not strictly less)
            ASSERT_FALSE( r < dt3 );    // end(03:00) < dt3(02:30) = false
            ASSERT_FALSE( r < dt2 );    // end(03:00) < dt2(02:00) = false
            ASSERT_FALSE( r < dt1 );    // end(03:00) < dt1(01:00) = false
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 13: operator<= (range vs CDateTime)
    //   "range ends at or before dt" means range.end <= dt
    GUCEF_TESTFW_TESTCASE( "Test 13: operator<= range vs CDateTime" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 13: operator<= range vs CDateTime" );
            CORE::CDateTimeRange r( dt2, dt4 );
            ASSERT_TRUE(  r <= dt5 );   // end(03:00) <= dt5(04:00) = true
            ASSERT_TRUE(  r <= dt4 );   // end(03:00) <= dt4(03:00) = true (equal)
            ASSERT_FALSE( r <= dt3 );   // end(03:00) <= dt3(02:30) = false
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 14: operator> (range vs CDateTime)
    //   "range is entirely after dt" means range.start > dt
    GUCEF_TESTFW_TESTCASE( "Test 14: operator> range vs CDateTime" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 14: operator> range vs CDateTime" );
            CORE::CDateTimeRange r( dt2, dt4 );
            ASSERT_TRUE(  r > dt1 );    // start(02:00) > dt1(01:00) = true
            ASSERT_FALSE( r > dt2 );    // start(02:00) > dt2(02:00) = false (equal)
            ASSERT_FALSE( r > dt3 );    // start(02:00) > dt3(02:30) = false
            ASSERT_FALSE( r > dt5 );    // start(02:00) > dt5(04:00) = false
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 15: operator>= (range vs CDateTime)
    //   "range starts at or after dt" means range.start >= dt
    GUCEF_TESTFW_TESTCASE( "Test 15: operator>= range vs CDateTime" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 15: operator>= range vs CDateTime" );
            CORE::CDateTimeRange r( dt2, dt4 );
            ASSERT_TRUE(  r >= dt1 );   // start(02:00) >= dt1(01:00) = true
            ASSERT_TRUE(  r >= dt2 );   // start(02:00) >= dt2(02:00) = true (equal)
            ASSERT_FALSE( r >= dt3 );   // start(02:00) >= dt3(02:30) = false
            ASSERT_FALSE( r >= dt5 );   // start(02:00) >= dt5(04:00) = false
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 16: "Contains" pattern using range operators
    //   A CDateTime is within [start, end] when:
    //   !( range < dt ) && !( range > dt )
    //   i.e. range does not end before dt AND range does not start after dt
    GUCEF_TESTFW_TESTCASE( "Test 16: Contains CDateTime pattern" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 16: Contains CDateTime pattern" );
            CORE::CDateTimeRange r( dt2, dt4 );   // [02:00, 03:00]

            // dt3(02:30) is within [02:00, 03:00]
            ASSERT_FALSE( r < dt3 );   // range does not end before 02:30
            ASSERT_FALSE( r > dt3 );   // range does not start after 02:30

            // dt1(01:00) is before the range
            ASSERT_TRUE( r > dt1 );    // range starts after 01:00

            // dt5(04:00) is after the range
            ASSERT_TRUE( r < dt5 );    // range ends before 04:00

            // dt2(02:00) - at the start boundary
            ASSERT_FALSE( r < dt2 );   // range does not end before 02:00 (end=03:00)
            ASSERT_FALSE( r > dt2 );   // range does not start after 02:00 (start==dt2)

            // dt4(03:00) - at the end boundary
            ASSERT_FALSE( r < dt4 );   // end(03:00) not < dt4(03:00)
            ASSERT_FALSE( r > dt4 );   // start(02:00) not > dt4(03:00)
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 17: SetStart / GetStart
    GUCEF_TESTFW_TESTCASE( "Test 17: SetStart and GetStart" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 17: SetStart and GetStart" );
            CORE::CDateTimeRange r( dt2, dt4 );
            r.SetStart( dt1 );
            ASSERT_TRUE( r.GetStart() == dt1 );
            ASSERT_TRUE( r.GetEnd()   == dt4 );  // end unchanged
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 18: SetEnd / GetEnd
    GUCEF_TESTFW_TESTCASE( "Test 18: SetEnd and GetEnd" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 18: SetEnd and GetEnd" );
            CORE::CDateTimeRange r( dt2, dt4 );
            r.SetEnd( dt5 );
            ASSERT_TRUE( r.GetStart() == dt2 );  // start unchanged
            ASSERT_TRUE( r.GetEnd()   == dt5 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 19: non-const GetStart returns mutable reference
    GUCEF_TESTFW_TESTCASE( "Test 19: Mutable GetStart reference" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 19: Mutable GetStart reference" );
            CORE::CDateTimeRange r( dt2, dt4 );
            CORE::CDateTime& startRef = r.GetStart();
            startRef = dt1;
            ASSERT_TRUE( r.GetStart() == dt1 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 20: non-const GetEnd returns mutable reference
    GUCEF_TESTFW_TESTCASE( "Test 20: Mutable GetEnd reference" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 20: Mutable GetEnd reference" );
            CORE::CDateTimeRange r( dt2, dt4 );
            CORE::CDateTime& endRef = r.GetEnd();
            endRef = dt5;
            ASSERT_TRUE( r.GetEnd() == dt5 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 21: GetTimeRangeTimeInMilliSecs - 1 hour = 3,600,000 ms
    GUCEF_TESTFW_TESTCASE( "Test 21: GetTimeRangeTimeInMilliSecs one hour" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 21: GetTimeRangeTimeInMilliSecs one hour" );
            // dt2 = 02:00:00, dt4 = 03:00:00 -> exactly 1 hour apart
            CORE::CDateTimeRange r( dt2, dt4 );
            CORE::UInt64 durationMs = r.GetTimeRangeTimeInMilliSecs();
            ASSERT_TRUE( durationMs == 3600000 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 22: GetTimeRangeTimeInMilliSecs - 30 minutes = 1,800,000 ms
    GUCEF_TESTFW_TESTCASE( "Test 22: GetTimeRangeTimeInMilliSecs thirty minutes" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 22: GetTimeRangeTimeInMilliSecs thirty minutes" );
            // dt2 = 02:00:00, dt3 = 02:30:00 -> 30 minutes = 1,800,000 ms
            CORE::CDateTimeRange r( dt2, dt3 );
            CORE::UInt64 durationMs = r.GetTimeRangeTimeInMilliSecs();
            ASSERT_TRUE( durationMs == 1800000 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 23: GetTimeRangeTimeInMilliSecs - reversed endpoints returns abs value
    GUCEF_TESTFW_TESTCASE( "Test 23: GetTimeRangeTimeInMilliSecs reversed endpoints" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 23: GetTimeRangeTimeInMilliSecs reversed endpoints" );
            // end < start - implementation takes absolute value
            CORE::CDateTimeRange r( dt4, dt2 );   // reversed: start=03:00, end=02:00
            CORE::UInt64 durationMs = r.GetTimeRangeTimeInMilliSecs();
            ASSERT_TRUE( durationMs == 3600000 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 24: HasRangeBegun - range entirely in the past has begun
    GUCEF_TESTFW_TESTCASE( "Test 24: HasRangeBegun past range" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 24: HasRangeBegun past range" );
            // 2024: entirely in the past relative to today (2026-03-01)
            CORE::CDateTime past1( 2024, 1,  1, 0, 0, 0, 0, 0 );
            CORE::CDateTime past2( 2024, 12, 31, 23, 59, 0, 0, 0 );
            CORE::CDateTimeRange pastRange( past1, past2 );
            ASSERT_TRUE( pastRange.HasRangeBegun() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 25: HasRangeEnded - range entirely in the past has ended
    GUCEF_TESTFW_TESTCASE( "Test 25: HasRangeEnded past range" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 25: HasRangeEnded past range" );
            CORE::CDateTime past1( 2024, 1,  1, 0, 0, 0, 0, 0 );
            CORE::CDateTime past2( 2024, 12, 31, 23, 59, 0, 0, 0 );
            CORE::CDateTimeRange pastRange( past1, past2 );
            ASSERT_TRUE( pastRange.HasRangeEnded() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 26: HasRangeBegun - range entirely in the future has NOT begun
    GUCEF_TESTFW_TESTCASE( "Test 26: HasRangeBegun future range" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 26: HasRangeBegun future range" );
            // 2030: entirely in the future relative to today (2026-03-01)
            CORE::CDateTime future1( 2030, 1, 1, 0, 0, 0, 0, 0 );
            CORE::CDateTime future2( 2031, 1, 1, 0, 0, 0, 0, 0 );
            CORE::CDateTimeRange futureRange( future1, future2 );
            ASSERT_FALSE( futureRange.HasRangeBegun() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 27: HasRangeEnded - range entirely in the future has NOT ended
    GUCEF_TESTFW_TESTCASE( "Test 27: HasRangeEnded future range" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 27: HasRangeEnded future range" );
            CORE::CDateTime future1( 2030, 1, 1, 0, 0, 0, 0, 0 );
            CORE::CDateTime future2( 2031, 1, 1, 0, 0, 0, 0, 0 );
            CORE::CDateTimeRange futureRange( future1, future2 );
            ASSERT_FALSE( futureRange.HasRangeEnded() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 28: TimeTillStartInMilliSecs returns 0 for a range that has already begun
    GUCEF_TESTFW_TESTCASE( "Test 28: TimeTillStartInMilliSecs past range" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 28: TimeTillStartInMilliSecs past range" );
            CORE::CDateTime past1( 2024, 1, 1, 0, 0, 0, 0, 0 );
            CORE::CDateTime past2( 2024, 12, 31, 23, 59, 0, 0, 0 );
            CORE::CDateTimeRange pastRange( past1, past2 );
            ASSERT_TRUE( pastRange.TimeTillStartInMilliSecs() == 0 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 29: TimeTillEndInMilliSecs returns 0 for a range that has already ended
    GUCEF_TESTFW_TESTCASE( "Test 29: TimeTillEndInMilliSecs past range" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 29: TimeTillEndInMilliSecs past range" );
            CORE::CDateTime past1( 2024, 1, 1, 0, 0, 0, 0, 0 );
            CORE::CDateTime past2( 2024, 12, 31, 23, 59, 0, 0, 0 );
            CORE::CDateTimeRange pastRange( past1, past2 );
            ASSERT_TRUE( pastRange.TimeTillEndInMilliSecs() == 0 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 30: TimeTillStartInMilliSecs returns > 0 for a future range
    GUCEF_TESTFW_TESTCASE( "Test 30: TimeTillStartInMilliSecs future range" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 30: TimeTillStartInMilliSecs future range" );
            CORE::CDateTime future1( 2030, 1, 1, 0, 0, 0, 0, 0 );
            CORE::CDateTime future2( 2031, 1, 1, 0, 0, 0, 0, 0 );
            CORE::CDateTimeRange futureRange( future1, future2 );
            ASSERT_TRUE( futureRange.TimeTillStartInMilliSecs() > 0 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 31: TimeTillEndInMilliSecs returns > 0 for a future range
    GUCEF_TESTFW_TESTCASE( "Test 31: TimeTillEndInMilliSecs future range" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 31: TimeTillEndInMilliSecs future range" );
            CORE::CDateTime future1( 2030, 1, 1, 0, 0, 0, 0, 0 );
            CORE::CDateTime future2( 2031, 1, 1, 0, 0, 0, 0, 0 );
            CORE::CDateTimeRange futureRange( future1, future2 );
            ASSERT_TRUE( futureRange.TimeTillEndInMilliSecs() > 0 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 32: TimeTillEndInMilliSecs > TimeTillStartInMilliSecs for future range
    GUCEF_TESTFW_TESTCASE( "Test 32: TimeTillEnd > TimeTillStart for future range" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 32: TimeTillEnd > TimeTillStart for future range" );
            CORE::CDateTime future1( 2030, 1, 1, 0, 0, 0, 0, 0 );
            CORE::CDateTime future2( 2031, 1, 1, 0, 0, 0, 0, 0 );
            CORE::CDateTimeRange futureRange( future1, future2 );
            ASSERT_TRUE( futureRange.TimeTillEndInMilliSecs() > futureRange.TimeTillStartInMilliSecs() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 33: Clear() resets to default state
    GUCEF_TESTFW_TESTCASE( "Test 33: Clear" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 33: Clear" );
            CORE::CDateTimeRange r( dt2, dt4 );
            ASSERT_FALSE( r == CORE::CDateTimeRange() );
            r.Clear();
            ASSERT_TRUE( r == CORE::CDateTimeRange() );
            ASSERT_TRUE( r.GetStart() == r.GetEnd() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
}

/*-------------------------------------------------------------------------*/
