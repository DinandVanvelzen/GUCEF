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

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestRandom.h"

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

void
PerformRandomTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Random TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "Random" );

    GUCEF_TESTFW_TESTCASE( "Test: GetRandomBytes fills buffers and differs between calls" )
        try
        {
            UInt8 a[ 64 ];
            UInt8 b[ 64 ];
            memset( a, 0, sizeof( a ) );
            memset( b, 0, sizeof( b ) );
            ASSERT_TRUE( CORE::CRandom::GetRandomBytes( a, sizeof( a ) ) );
            ASSERT_TRUE( CORE::CRandom::GetRandomBytes( b, sizeof( b ) ) );

            UInt8 zeros[ 64 ];
            memset( zeros, 0, sizeof( zeros ) );
            ASSERT_TRUE( 0 != memcmp( a, zeros, sizeof( a ) ) );
            ASSERT_TRUE( 0 != memcmp( a, b, sizeof( a ) ) );

            // edge cases
            ASSERT_TRUE( CORE::CRandom::GetRandomBytes( a, 0 ) );
            ASSERT_FALSE( CORE::CRandom::GetRandomBytes( GUCEF_NULL, 10 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: GetRandomBytes handles large requests" )
        try
        {
            // larger than what a single getrandom() call is guaranteed to return
            const UInt32 size = 1024 * 1024;
            UInt8* buffer = GUCEF_NEW UInt8[ size ];
            memset( buffer, 0, size );
            bool success = CORE::CRandom::GetRandomBytes( buffer, size );

            // the last block must have been filled too
            UInt32 nonZeroInTail = 0;
            for ( UInt32 i=size-4096; i<size; ++i )
                if ( 0 != buffer[ i ] )
                    ++nonZeroInTail;
            GUCEF_DELETE [] buffer;

            ASSERT_TRUE( success );
            ASSERT_TRUE( nonZeroInTail > 3000 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: GetRandomUInt32 stays in range and covers it" )
        try
        {
            bool seen[ 6 ] = { false, false, false, false, false, false };
            for ( UInt32 i=0; i<2000; ++i )
            {
                UInt32 value = 0;
                ASSERT_TRUE( CORE::CRandom::GetRandomUInt32( 5, 10, value ) );
                ASSERT_TRUE( value >= 5 && value <= 10 );
                if ( value >= 5 && value <= 10 )
                    seen[ value - 5 ] = true;
            }
            for ( UInt32 n=0; n<6; ++n )
                ASSERT_TRUE( seen[ n ] );

            UInt32 value = 0;
            ASSERT_TRUE( CORE::CRandom::GetRandomUInt32( 7, 7, value ) );
            ASSERT_TRUE( 7 == value );

            // swapped bounds are accepted
            for ( UInt32 i=0; i<100; ++i )
            {
                ASSERT_TRUE( CORE::CRandom::GetRandomUInt32( 10, 5, value ) );
                ASSERT_TRUE( value >= 5 && value <= 10 );
            }

            // entire range must not hang or fail
            ASSERT_TRUE( CORE::CRandom::GetRandomUInt32( 0, GUCEF_UINT32MAX, value ) );

            // convenience variant
            UInt32 convenient = CORE::CRandom::GetRandomUInt32( 100, 200 );
            ASSERT_TRUE( convenient >= 100 && convenient <= 200 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: GetRandomUInt32 distribution is not biased for a range that does not divide 2^32" )
        try
        {
            // With plain modulo a range of 3 is biased by ~1 in 2^31 which is not measurable
            // but a range just over 2^31 would be biased 2:1 for the low values
            const UInt32 rangeMax = 3000000000U;
            const UInt32 samples = 20000;
            UInt32 lowHalfCount = 0;
            for ( UInt32 i=0; i<samples; ++i )
            {
                UInt32 value = 0;
                ASSERT_TRUE( CORE::CRandom::GetRandomUInt32( 0, rangeMax, value ) );
                ASSERT_TRUE( value <= rangeMax );
                if ( value < rangeMax / 2 )
                    ++lowHalfCount;
            }
            // expected 50%, 6 sigma is ~2.1%, allow 5%
            ASSERT_TRUE( lowHalfCount > samples * 45 / 100 );
            ASSERT_TRUE( lowHalfCount < samples * 55 / 100 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: GetRandomFloat64 is within [0,1)" )
        try
        {
            Float64 sum = 0.0;
            for ( UInt32 i=0; i<1000; ++i )
            {
                Float64 value = CORE::CRandom::GetRandomFloat64();
                ASSERT_TRUE( value >= 0.0 && value < 1.0 );
                sum += value;
            }
            ASSERT_TRUE( sum / 1000.0 > 0.4 && sum / 1000.0 < 0.6 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL Random TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
