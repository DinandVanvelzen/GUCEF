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

#ifndef GUCEF_CORE_CSTOPWATCH_H
#include "gucefCORE_CStopwatch.h"
#define GUCEF_CORE_CSTOPWATCH_H
#endif /* GUCEF_CORE_CSTOPWATCH_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestStopwatch.h"

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

// Helper function to create a small delay
static void SmallDelay( void )
{
    volatile int dummy = 0;
    for ( int i = 0; i < 500000; ++i ) { dummy += i; }
}

/*-------------------------------------------------------------------------*/

void
PerformStopwatchTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CStopwatch TESTS" );
    
    GUCEF_TESTFW_SUITE_SCOPE( "CStopwatch" );

    GUCEF_TESTFW_TESTCASE( "Test: Stopwatch operations" )
    try
    {                              
        // Test 1: Default construction
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 1: Default construction" );
        {
            CORE::CStopwatch sw;
            ASSERT_FALSE( sw.IsRunning() );
            ASSERT_FALSE( sw.HasStarted() );
            ASSERT_FALSE( sw.HasStopped() );
            ASSERT_TRUE( sw.GetElapsedMilliseconds() == 0 );
        }

        // Test 2: Construction with immediate start
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 2: Construction with immediate start" );
        {
            CORE::CStopwatch sw( true );
            ASSERT_TRUE( sw.IsRunning() );
            ASSERT_TRUE( sw.HasStarted() );
            ASSERT_FALSE( sw.HasStopped() );
        }

        // Test 3: Start and Stop
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 3: Start and Stop" );
        {
            CORE::CStopwatch sw;
            
            sw.Start();
            ASSERT_TRUE( sw.IsRunning() );
            ASSERT_TRUE( sw.HasStarted() );
            ASSERT_FALSE( sw.HasStopped() );
            
            SmallDelay();
            
            sw.Stop();
            ASSERT_FALSE( sw.IsRunning() );
            ASSERT_TRUE( sw.HasStarted() );
            ASSERT_TRUE( sw.HasStopped() );
            
            // Elapsed time should be > 0
            ASSERT_TRUE( sw.GetElapsedMicroseconds() > 0 );
        }

        // Test 4: Reset
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 4: Reset" );
        {
            CORE::CStopwatch sw;
            sw.Start();
            SmallDelay();
            sw.Stop();
            
            ASSERT_TRUE( sw.GetElapsedMicroseconds() > 0 );
            
            sw.Reset();
            ASSERT_FALSE( sw.IsRunning() );
            ASSERT_FALSE( sw.HasStarted() );
            ASSERT_FALSE( sw.HasStopped() );
            ASSERT_TRUE( sw.GetElapsedMilliseconds() == 0 );
        }

        // Test 5: Restart
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 5: Restart" );
        {
            CORE::CStopwatch sw;
            sw.Start();
            SmallDelay();
            sw.Stop();
            
            UInt64 firstElapsed = sw.GetElapsedMicroseconds();
            ASSERT_TRUE( firstElapsed > 0 );
            
            sw.Restart();
            ASSERT_TRUE( sw.IsRunning() );
            
            // Elapsed should be very small after restart
            ASSERT_TRUE( sw.GetElapsedMicroseconds() < firstElapsed );
        }

        // Test 6: Elapsed time while running
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 6: Elapsed time while running" );
        {
            CORE::CStopwatch sw;
            sw.Start();
            
            SmallDelay();
            UInt64 elapsed1 = sw.GetElapsedMicroseconds();
            
            SmallDelay();
            UInt64 elapsed2 = sw.GetElapsedMicroseconds();
            
            // Elapsed time should increase while running
            ASSERT_TRUE( elapsed2 > elapsed1 );
            
            sw.Stop();
            UInt64 elapsed3 = sw.GetElapsedMicroseconds();
            
            // After stopping, elapsed should be fixed
            ASSERT_TRUE( elapsed3 >= elapsed2 );
            
            SmallDelay();
            UInt64 elapsed4 = sw.GetElapsedMicroseconds();
            
            // Should be the same after stopping
            ASSERT_TRUE( elapsed4 == elapsed3 );
        }

        // Test 7: Different time units
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 7: Different time units" );
        {
            CORE::CStopwatch sw;
            sw.Start();
            SmallDelay();
            sw.Stop();
            
            UInt64 elapsedNs = sw.GetElapsedNanoseconds();
            UInt64 elapsedUs = sw.GetElapsedMicroseconds();
            UInt64 elapsedMs = sw.GetElapsedMilliseconds();
            Float64 elapsedSec = sw.GetElapsedSeconds();
            
            // Verify relationships between units
            ASSERT_TRUE( elapsedNs > 0 );
            ASSERT_TRUE( elapsedUs > 0 );
            
            // Nanoseconds should be >= microseconds * 1000
            ASSERT_TRUE( elapsedNs >= elapsedUs * 1000 - 1000 );  // Allow some tolerance
            
            // Microseconds should be >= milliseconds * 1000
            if ( elapsedMs > 0 )
            {
                ASSERT_TRUE( elapsedUs >= elapsedMs * 1000 - 1000 );
            }
            
            // Seconds should be reasonable
            ASSERT_TRUE( elapsedSec >= 0.0 );
            ASSERT_TRUE( elapsedSec < 10.0 );  // Should be much less than 10 seconds
            
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, CORE::CString( "  Elapsed: " ) + 
                       CORE::UInt64ToString( elapsedNs ) + " ns, " +
                       CORE::UInt64ToString( elapsedUs ) + " us, " +
                       CORE::UInt64ToString( elapsedMs ) + " ms, " +
                       CORE::DoubleToString( elapsedSec ) + " sec" );
        }

        // Test 8: Copy constructor
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 8: Copy constructor" );
        {
            CORE::CStopwatch sw1;
            sw1.Start();
            SmallDelay();
            sw1.Stop();
            
            CORE::CStopwatch sw2( sw1 );
            
            ASSERT_TRUE( sw1.IsRunning() == sw2.IsRunning() );
            ASSERT_TRUE( sw1.HasStarted() == sw2.HasStarted() );
            ASSERT_TRUE( sw1.HasStopped() == sw2.HasStopped() );
            ASSERT_TRUE( sw1.GetElapsedMicroseconds() == sw2.GetElapsedMicroseconds() );
        }

        // Test 9: Assignment operator
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 9: Assignment operator" );
        {
            CORE::CStopwatch sw1;
            sw1.Start();
            SmallDelay();
            sw1.Stop();
            
            CORE::CStopwatch sw2;
            sw2 = sw1;
            
            ASSERT_TRUE( sw1.GetElapsedMicroseconds() == sw2.GetElapsedMicroseconds() );
        }

        // Test 10: StartNew static method
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 10: StartNew static method" );
        {
            CORE::CStopwatch sw = CORE::CStopwatch::StartNew();
            
            ASSERT_TRUE( sw.IsRunning() );
            ASSERT_TRUE( sw.HasStarted() );
            
            SmallDelay();
            sw.Stop();
            
            ASSERT_TRUE( sw.GetElapsedMicroseconds() > 0 );
        }

        // Test 11: GetStartTimestamp and GetStopTimestamp
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 11: GetStartTimestamp and GetStopTimestamp" );
        {
            CORE::CStopwatch sw;
            
            // Before starting
            ASSERT_FALSE( sw.GetStartTimestamp().IsValid() );
            ASSERT_FALSE( sw.GetStopTimestamp().IsValid() );
            
            sw.Start();
            ASSERT_TRUE( sw.GetStartTimestamp().IsValid() );
            ASSERT_FALSE( sw.GetStopTimestamp().IsValid() );
            
            SmallDelay();
            sw.Stop();
            
            ASSERT_TRUE( sw.GetStartTimestamp().IsValid() );
            ASSERT_TRUE( sw.GetStopTimestamp().IsValid() );
            
            // Stop timestamp should be after start
            ASSERT_TRUE( sw.GetStopTimestamp() > sw.GetStartTimestamp() );
        }

        // Test 12: GetEffectiveEndTimestamp
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 12: GetEffectiveEndTimestamp" );
        {
            CORE::CStopwatch sw;
            
            // Before starting - should be empty
            ASSERT_FALSE( sw.GetEffectiveEndTimestamp().IsValid() );
            
            sw.Start();
            
            // While running - should return current time (valid)
            CORE::CTimestamp effectiveEnd1 = sw.GetEffectiveEndTimestamp();
            ASSERT_TRUE( effectiveEnd1.IsValid() );
            
            SmallDelay();
            
            // Should change while running
            CORE::CTimestamp effectiveEnd2 = sw.GetEffectiveEndTimestamp();
            ASSERT_TRUE( effectiveEnd2 > effectiveEnd1 );
            
            sw.Stop();
            
            // After stopping - should return stop timestamp
            CORE::CTimestamp effectiveEnd3 = sw.GetEffectiveEndTimestamp();
            ASSERT_TRUE( effectiveEnd3 == sw.GetStopTimestamp() );
        }

        // Test 13: Multiple Stop calls
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 13: Multiple Stop calls" );
        {
            CORE::CStopwatch sw;
            sw.Start();
            SmallDelay();
            sw.Stop();
            
            UInt64 elapsed1 = sw.GetElapsedMicroseconds();
            
            SmallDelay();
            sw.Stop();  // Second stop should have no effect
            
            UInt64 elapsed2 = sw.GetElapsedMicroseconds();
            
            ASSERT_TRUE( elapsed1 == elapsed2 );
        }

        // Test 14: GetElapsedTicks
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 14: GetElapsedTicks" );
        {
            CORE::CStopwatch sw;
            sw.Start();
            SmallDelay();
            sw.Stop();
            
            Int64 ticks = sw.GetElapsedTicks();
            ASSERT_TRUE( ticks > 0 );
            
            // Verify ticks relationship to nanoseconds
            UInt64 ns = sw.GetElapsedNanoseconds();
            // They should be related by the platform's tick resolution
            ASSERT_TRUE( ticks > 0 && ns > 0 );
        }

        // Test 15: Typical usage pattern (timing a loop)
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 15: Typical usage pattern" );
        {
            CORE::CStopwatch sw;
            UInt64 totalUs = 0;
            const int iterations = 5;
            
            for ( int i = 0; i < iterations; ++i )
            {
                sw.Restart();
                SmallDelay();
                sw.Stop();
                totalUs += sw.GetElapsedMicroseconds();
            }
            
            ASSERT_TRUE( totalUs > 0 );
            
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, CORE::CString( "  Total time for " ) + 
                       CORE::Int32ToString( iterations ) + " iterations: " +
                       CORE::UInt64ToString( totalUs ) + " us, avg: " +
                       CORE::UInt64ToString( totalUs / iterations ) + " us" );
        }

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL CStopwatch TESTS PASSED" );
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, CORE::CString( "Exception caught: " ) + e.what() );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL CStopwatch TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
