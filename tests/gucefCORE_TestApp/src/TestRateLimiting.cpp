/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CRATELIMITER_H
#include "gucefCORE_CRateLimiter.h"
#define GUCEF_CORE_CRATELIMITER_H
#endif /* GUCEF_CORE_CRATELIMITER_H ? */

#ifndef GUCEF_CORE_CBACKOFFPOLICY_H
#include "gucefCORE_CBackoffPolicy.h"
#define GUCEF_CORE_CBACKOFFPOLICY_H
#endif /* GUCEF_CORE_CBACKOFFPOLICY_H ? */

#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestRateLimiting.h"

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

class CFakeClock : public CORE::CIMonotonicClock
{
    public:

    CFakeClock( void ) : m_nowInMs( 1000000 ) {}

    virtual UInt64 GetMonotonicTimeInMs( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_nowInMs; }

    void Advance( UInt64 ms ) { m_nowInMs += ms; }

    private:

    UInt64 m_nowInMs;
};

/*-------------------------------------------------------------------------*/

void
PerformRateLimitingTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Rate Limiting TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "RateLimiting" );

    GUCEF_TESTFW_TESTCASE( "Test: CRateLimiter burst, refill and delay" )
        try
        {
            CFakeClock clock;
            CORE::CRateLimiter limiter( 10.0, 5, 0.0, &clock );    // 10 per second, burst of 5

            // the bucket starts full
            for ( UInt32 i=0; i<5; ++i )
                ASSERT_TRUE( limiter.TryAcquire() );
            ASSERT_FALSE( limiter.TryAcquire() );
            ASSERT_TRUE( 100 == limiter.GetDelayUntilAvailableInMs() );

            // checking the delay takes nothing
            ASSERT_TRUE( 100 == limiter.GetDelayUntilAvailableInMs() );

            clock.Advance( 99 );
            ASSERT_FALSE( limiter.TryAcquire() );
            ASSERT_TRUE( 1 == limiter.GetDelayUntilAvailableInMs() );
            clock.Advance( 1 );
            ASSERT_TRUE( 0 == limiter.GetDelayUntilAvailableInMs() );
            ASSERT_TRUE( limiter.TryAcquire() );
            ASSERT_FALSE( limiter.TryAcquire() );

            // multiple tokens at once
            ASSERT_TRUE( 300 == limiter.GetDelayUntilAvailableInMs( 3 ) );
            clock.Advance( 300 );
            ASSERT_TRUE( limiter.TryAcquire( 3 ) );
            ASSERT_FALSE( limiter.TryAcquire( 1 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CRateLimiter never accumulates more than the burst size" )
        try
        {
            CFakeClock clock;
            CORE::CRateLimiter limiter( 10.0, 5, 0.0, &clock );
            ASSERT_TRUE( limiter.TryAcquire( 5 ) );

            clock.Advance( 3600 * 1000 );    // an hour of idling
            ASSERT_TRUE( limiter.GetAvailableTokens() > 4.99 && limiter.GetAvailableTokens() < 5.01 );
            ASSERT_TRUE( limiter.TryAcquire( 5 ) );
            ASSERT_FALSE( limiter.TryAcquire( 1 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CRateLimiter requests that can never be granted" )
        try
        {
            CFakeClock clock;
            CORE::CRateLimiter limiter( 10.0, 5, 0.0, &clock );
            ASSERT_FALSE( limiter.TryAcquire( 6 ) );
            ASSERT_TRUE( GUCEF_UINT32MAX == limiter.GetDelayUntilAvailableInMs( 6 ) );

            CORE::CRateLimiter noRefill( 0.0, 2, 0.0, &clock );
            ASSERT_TRUE( noRefill.TryAcquire( 2 ) );
            clock.Advance( 100000 );
            ASSERT_FALSE( noRefill.TryAcquire() );
            ASSERT_TRUE( GUCEF_UINT32MAX == noRefill.GetDelayUntilAvailableInMs() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CRateLimiter jitter only adds delay within the configured percentage" )
        try
        {
            CFakeClock clock;
            CORE::CRateLimiter limiter( 10.0, 1, 25.0, &clock );
            ASSERT_TRUE( limiter.TryAcquire() );

            bool sawJitter = false;
            for ( UInt32 i=0; i<200; ++i )
            {
                UInt32 delay = limiter.GetDelayUntilAvailableInMs();
                ASSERT_TRUE( delay >= 100 && delay <= 125 );
                if ( delay > 100 )
                    sawJitter = true;
            }
            ASSERT_TRUE( sawJitter );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CRateLimiter works with the system clock" )
        try
        {
            CORE::CRateLimiter limiter( 1000.0, 2 );
            ASSERT_TRUE( limiter.TryAcquire( 2 ) );
            ASSERT_FALSE( limiter.TryAcquire( 2 ) );
            ASSERT_TRUE( limiter.GetDelayUntilAvailableInMs( 2 ) <= 2000 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CBackoffPolicy exponential growth, cap and reset" )
        try
        {
            CORE::CBackoffPolicy policy( 100, 2.0, 1000 );
            ASSERT_TRUE( 100 == policy.NextDelayInMs() );
            ASSERT_TRUE( 200 == policy.NextDelayInMs() );
            ASSERT_TRUE( 400 == policy.NextDelayInMs() );
            ASSERT_TRUE( 800 == policy.NextDelayInMs() );
            ASSERT_TRUE( 1000 == policy.NextDelayInMs() );
            ASSERT_TRUE( 1000 == policy.NextDelayInMs() );
            ASSERT_TRUE( 6 == policy.GetAttemptCount() );

            policy.Reset();
            ASSERT_TRUE( 0 == policy.GetAttemptCount() );
            ASSERT_TRUE( 100 == policy.NextDelayInMs() );

            // many attempts must not overflow
            for ( UInt32 i=0; i<5000; ++i )
                ASSERT_TRUE( policy.NextDelayInMs() <= 1000 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CBackoffPolicy factor below 1 gives a constant delay" )
        try
        {
            CORE::CBackoffPolicy policy( 250, 0.5, 5000 );
            ASSERT_TRUE( 250 == policy.NextDelayInMs() );
            ASSERT_TRUE( 250 == policy.NextDelayInMs() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CBackoffPolicy jitter stays within bounds and the cap" )
        try
        {
            bool sawLower = false;
            bool sawHigher = false;
            for ( UInt32 i=0; i<300; ++i )
            {
                CORE::CBackoffPolicy policy( 1000, 2.0, 1100, 50.0 );
                UInt32 first = policy.NextDelayInMs();
                ASSERT_TRUE( first >= 500 && first <= 1100 );
                if ( first < 1000 )
                    sawLower = true;
                if ( first > 1000 )
                    sawHigher = true;
                // second attempt would be 2000 which is capped to 1100, jitter must not exceed the cap
                UInt32 second = policy.NextDelayInMs();
                ASSERT_TRUE( second >= 550 && second <= 1100 );
            }
            ASSERT_TRUE( sawLower );
            ASSERT_TRUE( sawHigher );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL Rate Limiting TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
