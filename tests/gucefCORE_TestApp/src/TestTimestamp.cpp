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

#ifndef GUCEF_CORE_CTIMESTAMP_H
#include "gucefCORE_CTimestamp.h"
#define GUCEF_CORE_CTIMESTAMP_H
#endif /* GUCEF_CORE_CTIMESTAMP_H ? */

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

#include "TestTimestamp.h"

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
PerformTimestampTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CTimestamp TESTS" );
    
    GUCEF_TESTFW_SUITE_SCOPE( "CTimestamp" );
                              
    // Test 1: Default construction
    GUCEF_TESTFW_TESTCASE( "Test 1: Default construction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 1: Default construction" );
            CORE::CTimestamp ts;
            ASSERT_FALSE( ts.IsValid() );
            ASSERT_TRUE( ts.GetTickCount() == 0 );
            ASSERT_TRUE( ts == CORE::CTimestamp::Empty );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 2: NowLocalTime and NowUTCTime
    GUCEF_TESTFW_TESTCASE( "Test 2: NowLocalTime and NowUTCTime" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 2: NowLocalTime and NowUTCTime" );
            CORE::CTimestamp localTs = CORE::CTimestamp::NowLocalTime();
            CORE::CTimestamp utcTs = CORE::CTimestamp::NowUTCTime();
            
            ASSERT_TRUE( localTs.IsValid() );
            ASSERT_TRUE( utcTs.IsValid() );
            ASSERT_TRUE( localTs.GetTickCount() > 0 );
            ASSERT_TRUE( utcTs.GetTickCount() > 0 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 3: Copy constructor and assignment
    GUCEF_TESTFW_TESTCASE( "Test 3: Copy constructor and assignment" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 3: Copy constructor and assignment" );
            CORE::CTimestamp ts1 = CORE::CTimestamp::NowUTCTime();
            CORE::CTimestamp ts2( ts1 );
            CORE::CTimestamp ts3;
            ts3 = ts1;
            
            ASSERT_TRUE( ts1 == ts2 );
            ASSERT_TRUE( ts1 == ts3 );
            ASSERT_TRUE( ts2 == ts3 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 4: Comparison operators
    GUCEF_TESTFW_TESTCASE( "Test 4: Comparison operators" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 4: Comparison operators" );
            CORE::CTimestamp ts1 = CORE::CTimestamp::NowUTCTime();
            
            // Small delay to ensure different timestamps
            volatile int dummy = 0;
            for ( int i = 0; i < 100000; ++i ) { dummy += i; }
            
            CORE::CTimestamp ts2 = CORE::CTimestamp::NowUTCTime();
            
            ASSERT_TRUE( ts1 < ts2 );
            ASSERT_TRUE( ts1 <= ts2 );
            ASSERT_TRUE( ts2 > ts1 );
            ASSERT_TRUE( ts2 >= ts1 );
            ASSERT_TRUE( ts1 != ts2 );
            ASSERT_FALSE( ts1 == ts2 );
            
            // Test equality
            CORE::CTimestamp ts3 = ts1;
            ASSERT_TRUE( ts1 == ts3 );
            ASSERT_TRUE( ts1 <= ts3 );
            ASSERT_TRUE( ts1 >= ts3 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 5: Arithmetic operators
    GUCEF_TESTFW_TESTCASE( "Test 5: Arithmetic operators" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 5: Arithmetic operators" );
            CORE::CTimestamp ts1 = CORE::CTimestamp::NowUTCTime();
            Int64 delta = 1000000;  // Some tick delta
            
            CORE::CTimestamp ts2 = ts1 + delta;
            ASSERT_TRUE( ts2 > ts1 );
            ASSERT_TRUE( ts2.GetTickCount() == ts1.GetTickCount() + delta );
            
            CORE::CTimestamp ts3 = ts2 - delta;
            ASSERT_TRUE( ts3 == ts1 );
            
            CORE::CTimestamp ts4 = ts1;
            ts4 += delta;
            ASSERT_TRUE( ts4 == ts2 );
            
            ts4 -= delta;
            ASSERT_TRUE( ts4 == ts1 );
            
            // Test difference operator
            Int64 diff = ts2 - ts1;
            ASSERT_TRUE( diff == delta );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 6: Conversion to/from CDateTime
    GUCEF_TESTFW_TESTCASE( "Test 6: Conversion to/from CDateTime" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 6: Conversion to/from CDateTime" );
            CORE::CDateTime dt = CORE::CDateTime::NowUTCDateTime();
            CORE::CTimestamp ts( dt );
            
            ASSERT_TRUE( ts.IsValid() );
            
            // Convert back and compare (may lose some precision)
            CORE::CDateTime dt2 = ts.ToDateTimeUTC();
            
            // Compare at millisecond precision
            ASSERT_TRUE( dt.GetYear() == dt2.GetYear() );
            ASSERT_TRUE( dt.GetMonth() == dt2.GetMonth() );
            ASSERT_TRUE( dt.GetDay() == dt2.GetDay() );
            ASSERT_TRUE( dt.GetHours() == dt2.GetHours() );
            ASSERT_TRUE( dt.GetMinutes() == dt2.GetMinutes() );
            ASSERT_TRUE( dt.GetSeconds() == dt2.GetSeconds() );
            // Milliseconds might differ slightly due to timing
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 7: Assignment from CDateTime
    GUCEF_TESTFW_TESTCASE( "Test 7: Assignment from CDateTime" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 7: Assignment from CDateTime" );
            CORE::CDateTime dt = CORE::CDateTime::NowUTCDateTime();
            CORE::CTimestamp ts;
            ts = dt;
            
            ASSERT_TRUE( ts.IsValid() );
            
            CORE::CDateTime dt2 = ts.ToDateTimeUTC();
            ASSERT_TRUE( dt.GetYear() == dt2.GetYear() );
            ASSERT_TRUE( dt.GetMonth() == dt2.GetMonth() );
            ASSERT_TRUE( dt.GetDay() == dt2.GetDay() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 8: Millisecond conversions
    GUCEF_TESTFW_TESTCASE( "Test 8: Millisecond conversions" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 8: Millisecond conversions" );
            CORE::CTimestamp ts1 = CORE::CTimestamp::NowUTCTime();
            UInt64 ms = ts1.ToMillisecondsSinceEpoch();
            
            ASSERT_TRUE( ms > 0 );
            
            CORE::CTimestamp ts2;
            ts2.FromMillisecondsSinceEpoch( ms );
            
            // Should be equal at millisecond precision
            Int64 diff = ts1.GetTimeDifferenceInMillisecondsTo( ts2 );
            ASSERT_TRUE( diff >= -1 && diff <= 1 );  // Allow 1ms tolerance
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 9: Microsecond conversions
    GUCEF_TESTFW_TESTCASE( "Test 9: Microsecond conversions" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 9: Microsecond conversions" );
            CORE::CTimestamp ts1 = CORE::CTimestamp::NowUTCTime();
            UInt64 us = ts1.ToMicrosecondsSinceEpoch();
            
            ASSERT_TRUE( us > 0 );
            
            CORE::CTimestamp ts2;
            ts2.FromMicrosecondsSinceEpoch( us );
            
            Int64 diff = ts1.GetTimeDifferenceInMicrosecondsTo( ts2 );
            ASSERT_TRUE( diff >= -1 && diff <= 1 );  // Allow 1us tolerance
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 10: Nanosecond conversions
    GUCEF_TESTFW_TESTCASE( "Test 10: Nanosecond conversions" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 10: Nanosecond conversions" );
            CORE::CTimestamp ts1 = CORE::CTimestamp::NowUTCTime();
            UInt64 ns = ts1.ToNanosecondsSinceEpoch();
            
            ASSERT_TRUE( ns > 0 );
            
            CORE::CTimestamp ts2;
            ts2.FromNanosecondsSinceEpoch( ns );
            
            // Check they are reasonably close (platform resolution dependent)
            UInt32 resolution = CORE::CTimestamp::GetResolutionInNanoseconds();
            Int64 diffNs = (Int64)ts2.ToNanosecondsSinceEpoch() - (Int64)ns;
            ASSERT_TRUE( diffNs >= -(Int64)resolution && diffNs <= (Int64)resolution );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 11: GetTimeDifference methods
    GUCEF_TESTFW_TESTCASE( "Test 11: GetTimeDifference methods" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 11: GetTimeDifference methods" );
            CORE::CTimestamp ts1 = CORE::CTimestamp::NowUTCTime();
            
            // Create ts2 that is 100ms later (using ticks)
            UInt64 msInTicks = 0;
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
            msInTicks = 100 * 10000;  // 100ms in 100ns ticks
            #else
            msInTicks = 100 * 1000000;  // 100ms in nanoseconds
            #endif
            
            CORE::CTimestamp ts2 = ts1 + msInTicks;
            
            Int64 diffMs = ts1.GetTimeDifferenceInMillisecondsTo( ts2 );
            ASSERT_TRUE( diffMs >= 99 && diffMs <= 101 );  // ~100ms
            
            Int64 diffUs = ts1.GetTimeDifferenceInMicrosecondsTo( ts2 );
            ASSERT_TRUE( diffUs >= 99000 && diffUs <= 101000 );  // ~100000us
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 12: Clear method
    GUCEF_TESTFW_TESTCASE( "Test 12: Clear method" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 12: Clear method" );
            CORE::CTimestamp ts = CORE::CTimestamp::NowUTCTime();
            ASSERT_TRUE( ts.IsValid() );
            
            ts.Clear();
            ASSERT_FALSE( ts.IsValid() );
            ASSERT_TRUE( ts.GetTickCount() == 0 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 13: UnixEpoch
    GUCEF_TESTFW_TESTCASE( "Test 13: UnixEpoch" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 13: UnixEpoch" );
            CORE::CTimestamp epoch = CORE::CTimestamp::UnixEpoch();
            CORE::CTimestamp now = CORE::CTimestamp::NowUTCTime();
            
            // Unix epoch should be earlier than now
            ASSERT_TRUE( epoch < now );
            
            // Milliseconds since epoch at Unix epoch should be 0
            UInt64 epochMs = epoch.ToMillisecondsSinceEpoch();
            #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
            ASSERT_TRUE( epochMs == 0 );
            #endif
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 14: GetResolutionInNanoseconds
    GUCEF_TESTFW_TESTCASE( "Test 14: GetResolutionInNanoseconds" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 14: GetResolutionInNanoseconds" );
            UInt32 resolution = CORE::CTimestamp::GetResolutionInNanoseconds();
            
            // Resolution should be reasonable (1ns to 1ms)
            ASSERT_TRUE( resolution >= 1 && resolution <= 1000000 );
            
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, CORE::CString( "Platform timestamp resolution: " ) + 
                       CORE::UInt32ToString( resolution ) + " ns" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 15: Comparison with CDateTime
    GUCEF_TESTFW_TESTCASE( "Test 15: Comparison with CDateTime" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 15: Comparison with CDateTime" );
            CORE::CDateTime dt = CORE::CDateTime::NowUTCDateTime();
            CORE::CTimestamp ts( dt );
            
            // Timestamp should equal the datetime it was created from
            ASSERT_TRUE( ts == dt );
            ASSERT_FALSE( ts != dt );
            
            // Create a datetime 1 second later
            CORE::CDateTime dtLater( dt.GetYear(), dt.GetMonth(), dt.GetDay(),
                                     dt.GetHours(), dt.GetMinutes(), dt.GetSeconds() + 1,
                                     dt.GetMilliseconds(), dt.GetTimeZoneUTCOffsetInMins() );
            
            ASSERT_TRUE( ts < dtLater );
            ASSERT_TRUE( ts <= dtLater );
            ASSERT_FALSE( ts > dtLater );
            ASSERT_FALSE( ts >= dtLater );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 16: Tick count get/set
    GUCEF_TESTFW_TESTCASE( "Test 16: Tick count get/set" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 16: Tick count get/set" );
            CORE::CTimestamp ts1 = CORE::CTimestamp::NowUTCTime();
            UInt64 ticks = ts1.GetTickCount();
            
            CORE::CTimestamp ts2;
            ts2.SetTickCount( ticks );
            
            ASSERT_TRUE( ts1 == ts2 );
            ASSERT_TRUE( ts2.GetTickCount() == ticks );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 17: Construction from ticks
    GUCEF_TESTFW_TESTCASE( "Test 17: Construction from ticks" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 17: Construction from ticks" );
            UInt64 ticks = 1234567890123456ULL;
            CORE::CTimestamp ts( ticks );
            
            ASSERT_TRUE( ts.GetTickCount() == ticks );
            ASSERT_TRUE( ts.IsValid() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 18: Empty constant
    GUCEF_TESTFW_TESTCASE( "Test 18: Empty constant" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 18: Empty constant" );
            ASSERT_FALSE( CORE::CTimestamp::Empty.IsValid() );
            ASSERT_TRUE( CORE::CTimestamp::Empty.GetTickCount() == 0 );
            
            CORE::CTimestamp ts;
            ASSERT_TRUE( ts == CORE::CTimestamp::Empty );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    // Test 19: Windows FILETIME conversion (Windows only)
    GUCEF_TESTFW_TESTCASE( "Test 19: Windows FILETIME conversion" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 19: Windows FILETIME conversion" );
            CORE::CTimestamp ts1 = CORE::CTimestamp::NowUTCTime();
            
            FILETIME ft = ts1.ToWindowsFiletime();
            
            CORE::CTimestamp ts2;
            ts2.FromWindowsFiletime( ft );
            
            ASSERT_TRUE( ts1 == ts2 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
    #endif

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL CTimestamp TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
