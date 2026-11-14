/*
 *  gucefMATH_TestApp: GUCEF test application for the Math module
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

#include <math.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_MATH_CTRUNNINGSTATISTICS_H
#include "gucefMATH_CTRunningStatistics.h"
#define GUCEF_MATH_CTRUNNINGSTATISTICS_H
#endif /* GUCEF_MATH_CTRUNNINGSTATISTICS_H ? */

#ifndef GUCEF_MATH_STATISTICSFUNCTIONS_H
#include "gucefMATH_StatisticsFunctions.h"
#define GUCEF_MATH_STATISTICSFUNCTIONS_H
#endif /* GUCEF_MATH_STATISTICSFUNCTIONS_H ? */

#ifndef GUCEF_MATH_CDECIMALSTATISTICS_H
#include "gucefMATH_CDecimalStatistics.h"
#define GUCEF_MATH_CDECIMALSTATISTICS_H
#endif /* GUCEF_MATH_CDECIMALSTATISTICS_H ? */

#ifndef GUCEF_MATH_CXOSHIRO256STARSTAR_H
#include "gucefMATH_CXoshiro256StarStar.h"
#define GUCEF_MATH_CXOSHIRO256STARSTAR_H
#endif /* GUCEF_MATH_CXOSHIRO256STARSTAR_H ? */

#include "TestStatistics.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/

static CORE::CDecimal
Dec( const char* text )
{
    return CORE::CDecimal::Parse( text );
}

/*-------------------------------------------------------------------------*/

static void
PerformDecimalStatisticsTests( void )
{GUCEF_TRACE;

    const CORE::CDecimalContext scale0( 0, CORE::ROUNDINGMODE_HALF_EVEN );
    const CORE::CDecimalContext scale1( 1, CORE::ROUNDINGMODE_HALF_EVEN );
    const CORE::CDecimalContext scale3( 3, CORE::ROUNDINGMODE_HALF_EVEN );
    const CORE::CDecimalContext scale6( 6, CORE::ROUNDINGMODE_HALF_EVEN );

    GUCEF_TESTFW_TESTCASE( "DecimalStatistics: the mean of 0.1, 0.2, 0.3 is exactly 0.2" )
    {
        const CORE::CDecimal values[ 3 ] = { Dec( "0.1" ), Dec( "0.2" ), Dec( "0.3" ) };
        CORE::CDecimal result;
        ASSERT_TRUE( CORE::DECIMALSTATUS_OK == MATH::CDecimalStatistics::Mean( values, 3, scale1, result ) );
        ASSERT_TRUE( result.IsIdentical( Dec( "0.2" ) ) );

        // The sum 0.6 is exact where the binary floats of the same values are not
        ASSERT_TRUE( CORE::DECIMALSTATUS_OK == MATH::CDecimalStatistics::Mean( values, 3, scale3, result ) );
        ASSERT_TRUE( result.IsIdentical( Dec( "0.200" ) ) );

        // Rounding follows the context: 1.5 -> 2 (ties to even), 2.5 -> 2
        const CORE::CDecimal ties[ 2 ] = { Dec( "1" ), Dec( "2" ) };
        ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == MATH::CDecimalStatistics::Mean( ties, 2, scale0, result ) );
        ASSERT_TRUE( result.IsIdentical( Dec( "2" ) ) );
        const CORE::CDecimal ties2[ 2 ] = { Dec( "2" ), Dec( "3" ) };
        MATH::CDecimalStatistics::Mean( ties2, 2, scale0, result );
        ASSERT_TRUE( result.IsIdentical( Dec( "2" ) ) );
        const CORE::CDecimalContext halfUp( 0, CORE::ROUNDINGMODE_HALF_UP );
        MATH::CDecimalStatistics::Mean( ties2, 2, halfUp, result );
        ASSERT_TRUE( result.IsIdentical( Dec( "3" ) ) );

        // Empty input is 0 at the result scale
        ASSERT_TRUE( CORE::DECIMALSTATUS_OK == MATH::CDecimalStatistics::Mean( values, 0, scale3, result ) );
        ASSERT_TRUE( result.IsIdentical( Dec( "0.000" ) ) );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DecimalStatistics: variance of { 2, 4, 4, 4, 5, 5, 7, 9 }" )
    {
        const CORE::CDecimal values[ 8 ] = { Dec( "2" ), Dec( "4" ), Dec( "4" ), Dec( "4" ), Dec( "5" ), Dec( "5" ), Dec( "7" ), Dec( "9" ) };
        CORE::CDecimal result;
        ASSERT_TRUE( CORE::DECIMALSTATUS_OK == MATH::CDecimalStatistics::Variance( values, 8, false, scale0, result ) );
        ASSERT_TRUE( result.IsIdentical( Dec( "4" ) ) );
        ASSERT_TRUE( CORE::DECIMALSTATUS_OK == MATH::CDecimalStatistics::Mean( values, 8, scale1, result ) );
        ASSERT_TRUE( result.IsIdentical( Dec( "5.0" ) ) );
        // sample variance 32 / 7 = 4.571428571...
        ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == MATH::CDecimalStatistics::Variance( values, 8, true, scale6, result ) );
        ASSERT_TRUE( result.IsIdentical( Dec( "4.571429" ) ) );

        // Fewer values than the divisor needs
        ASSERT_TRUE( CORE::DECIMALSTATUS_OK == MATH::CDecimalStatistics::Variance( values, 1, true, scale3, result ) );
        ASSERT_TRUE( result.IsIdentical( Dec( "0.000" ) ) );
        ASSERT_TRUE( CORE::DECIMALSTATUS_OK == MATH::CDecimalStatistics::Variance( values, 0, false, scale3, result ) );
        ASSERT_TRUE( result.IsIdentical( Dec( "0.000" ) ) );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DecimalStatistics: the large offset set has sample variance 30, in any order" )
    {
        const CORE::CDecimal values[ 4 ] = { Dec( "1000000004" ), Dec( "1000000007" ), Dec( "1000000013" ), Dec( "1000000016" ) };
        const CORE::CDecimal reversed[ 4 ] = { Dec( "1000000016" ), Dec( "1000000013" ), Dec( "1000000007" ), Dec( "1000000004" ) };
        CORE::CDecimal a, b;
        ASSERT_TRUE( CORE::DECIMALSTATUS_OK == MATH::CDecimalStatistics::Variance( values, 4, true, scale3, a ) );
        ASSERT_TRUE( CORE::DECIMALSTATUS_OK == MATH::CDecimalStatistics::Variance( reversed, 4, true, scale3, b ) );
        ASSERT_TRUE( a.IsIdentical( Dec( "30.000" ) ) && b.IsIdentical( a ) );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DecimalStatistics: seeded random decimals agree with Float64 and with each other under permutation" )
    {
        const MATH::UInt64 seed = 20261113;
        MATH::CXoshiro256StarStar generator( seed );
        for ( int round = 0; round < 50; ++round )
        {
            const MATH::UInt32 count = 2 + generator.NextUInt32() % 40;
            CORE::CDecimal values[ 42 ];
            CORE::CDecimal shuffled[ 42 ];
            MATH::Float64 asFloat[ 42 ];
            for ( MATH::UInt32 i = 0; i < count; ++i )
            {
                const MATH::Int64 unscaled = (MATH::Int64) ( generator.NextUInt32() % 2000000 ) - 1000000;
                CORE::CDecimal::FromUnscaled( unscaled, 2, values[ i ] );
                asFloat[ i ] = (MATH::Float64) unscaled / 100.0;
            }
            for ( MATH::UInt32 i = 0; i < count; ++i )
                shuffled[ i ] = values[ count - 1 - i ];

            CORE::CDecimal variance, varianceShuffled, mean;
            const CORE::TDecimalStatus s1 = MATH::CDecimalStatistics::Variance( values, count, true, scale6, variance );
            const CORE::TDecimalStatus s2 = MATH::CDecimalStatistics::Variance( shuffled, count, true, scale6, varianceShuffled );
            const CORE::TDecimalStatus s3 = MATH::CDecimalStatistics::Mean( values, count, scale6, mean );
            const bool ok = !CORE::DecimalStatusIsError( s1 ) && !CORE::DecimalStatusIsError( s2 ) && !CORE::DecimalStatusIsError( s3 ) &&
                            variance.IsIdentical( varianceShuffled ) &&
                            fabs( variance.ToFloat64() - MATH::Variance( asFloat, count, true ) ) <= 1e-6 * ( 1.0 + fabs( variance.ToFloat64() ) ) &&
                            fabs( mean.ToFloat64() - MATH::Mean( asFloat, count ) ) <= 1e-6;
            if ( !ok )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DecimalStatistics: seed " << seed << " round " << round << " count " << count << " variance " << variance.ToString() << " float " << MATH::Variance( asFloat, count, true ) );
            ASSERT_TRUE( ok );
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DecimalStatistics: standard deviation, exact root, rounding modes and agreement with Float64" )
    {
        const CORE::CDecimal values[ 8 ] = { Dec( "2" ), Dec( "4" ), Dec( "4" ), Dec( "4" ), Dec( "5" ), Dec( "5" ), Dec( "7" ), Dec( "9" ) };
        CORE::CDecimal result;
        ASSERT_TRUE( CORE::DECIMALSTATUS_OK == MATH::CDecimalStatistics::StandardDeviation( values, 8, false, scale0, result ) );
        ASSERT_TRUE( result.IsIdentical( Dec( "2" ) ) );
        ASSERT_TRUE( CORE::DECIMALSTATUS_OK == MATH::CDecimalStatistics::StandardDeviation( values, 8, false, scale3, result ) );
        ASSERT_TRUE( result.IsIdentical( Dec( "2.000" ) ) );
        const CORE::CDecimalContext unnecessary( 3, CORE::ROUNDINGMODE_UNNECESSARY );
        ASSERT_TRUE( CORE::DECIMALSTATUS_OK == MATH::CDecimalStatistics::StandardDeviation( values, 8, false, unnecessary, result ) );

        // sample: sqrt( 32 / 7 ) = 2.138089935299395...
        ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == MATH::CDecimalStatistics::StandardDeviation( values, 8, true, CORE::CDecimalContext( 6, CORE::ROUNDINGMODE_HALF_EVEN ), result ) );
        ASSERT_TRUE( result.IsIdentical( Dec( "2.138090" ) ) );
        MATH::CDecimalStatistics::StandardDeviation( values, 8, true, CORE::CDecimalContext( 6, CORE::ROUNDINGMODE_TOWARD_ZERO ), result );
        ASSERT_TRUE( result.IsIdentical( Dec( "2.138089" ) ) );
        MATH::CDecimalStatistics::StandardDeviation( values, 8, true, CORE::CDecimalContext( 6, CORE::ROUNDINGMODE_CEILING ), result );
        ASSERT_TRUE( result.IsIdentical( Dec( "2.138090" ) ) );
        ASSERT_TRUE( CORE::DECIMALSTATUS_ROUNDING_REQUIRED == MATH::CDecimalStatistics::StandardDeviation( values, 8, true, CORE::CDecimalContext( 6, CORE::ROUNDINGMODE_UNNECESSARY ), result ) );

        // a tie: values { 0, 1 }: population variance 1/4, deviation 0.5 exactly; at scale 0 the tie goes to even (0) or up (1)
        const CORE::CDecimal pair[ 2 ] = { Dec( "0" ), Dec( "1" ) };
        ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == MATH::CDecimalStatistics::StandardDeviation( pair, 2, false, scale0, result ) );
        ASSERT_TRUE( result.IsIdentical( Dec( "0" ) ) );
        MATH::CDecimalStatistics::StandardDeviation( pair, 2, false, CORE::CDecimalContext( 0, CORE::ROUNDINGMODE_HALF_UP ), result );
        ASSERT_TRUE( result.IsIdentical( Dec( "1" ) ) );
        MATH::CDecimalStatistics::StandardDeviation( pair, 2, false, scale1, result );
        ASSERT_TRUE( result.IsIdentical( Dec( "0.5" ) ) );

        ASSERT_TRUE( CORE::DECIMALSTATUS_OK == MATH::CDecimalStatistics::StandardDeviation( values, 1, true, scale3, result ) && result.IsZero() );

        MATH::CXoshiro256StarStar generator( 4711 );
        for ( int round = 0; round < 30; ++round )
        {
            const MATH::UInt32 count = 2 + generator.NextUInt32() % 30;
            CORE::CDecimal decimals[ 32 ];
            MATH::Float64 asFloat[ 32 ];
            for ( MATH::UInt32 i = 0; i < count; ++i )
            {
                const MATH::Int64 unscaled = (MATH::Int64) ( generator.NextUInt32() % 100000 );
                CORE::CDecimal::FromUnscaled( unscaled, 2, decimals[ i ] );
                asFloat[ i ] = (MATH::Float64) unscaled / 100.0;
            }
            CORE::CDecimal deviation;
            const CORE::TDecimalStatus status = MATH::CDecimalStatistics::StandardDeviation( decimals, count, true, scale6, deviation );
            const bool ok = !CORE::DecimalStatusIsError( status ) && fabs( deviation.ToFloat64() - MATH::StandardDeviation( asFloat, count, true ) ) <= 2e-6;
            if ( !ok )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DecimalStatistics: standard deviation differs, round " << round << " count " << count << " got " << deviation.ToString() );
            ASSERT_TRUE( ok );
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DecimalStatistics: covariance and regression of y = 2x + 1" )
    {
        CORE::CDecimal xs[ 10 ], ys[ 10 ];
        for ( int i = 0; i < 10; ++i )
        {
            CORE::CDecimal::FromUnscaled( ( i - 3 ) * 25, 1, xs[ i ] );              // -0.75 ... 1.5 style steps of 2.5 at scale 1
            CORE::CDecimal two = Dec( "2" );
            CORE::CDecimal product, one = Dec( "1" );
            two.Multiply( xs[ i ], product );
            product.Add( one, ys[ i ] );
        }
        CORE::CDecimal slope, intercept, rSquared, covariance, variance;
        ASSERT_TRUE( CORE::DECIMALSTATUS_OK == MATH::CDecimalStatistics::LinearRegression( xs, ys, 10, scale3, slope, intercept, rSquared ) );
        ASSERT_TRUE( slope.IsIdentical( Dec( "2.000" ) ) );
        ASSERT_TRUE( intercept.IsIdentical( Dec( "1.000" ) ) );
        ASSERT_TRUE( rSquared.IsIdentical( Dec( "1.000" ) ) );
        ASSERT_TRUE( !CORE::DecimalStatusIsError( MATH::CDecimalStatistics::Covariance( xs, ys, 10, false, scale3, covariance ) ) );
        ASSERT_TRUE( !CORE::DecimalStatusIsError( MATH::CDecimalStatistics::Variance( xs, 10, false, scale3, variance ) ) );
        CORE::CDecimal twiceVariance;
        variance.Multiply( (MATH::Int64) 2, twiceVariance );
        // each is rounded once to 3 digits: they agree within two units in the last place
        ASSERT_TRUE( fabs( covariance.ToFloat64() - twiceVariance.ToFloat64() ) <= 0.0021 );

        // Degenerate input
        ASSERT_TRUE( CORE::DECIMALSTATUS_DIVISION_BY_ZERO == MATH::CDecimalStatistics::LinearRegression( xs, ys, 1, scale3, slope, intercept, rSquared ) );
        CORE::CDecimal constant[ 4 ] = { Dec( "3" ), Dec( "3" ), Dec( "3" ), Dec( "3" ) };
        ASSERT_TRUE( CORE::DECIMALSTATUS_DIVISION_BY_ZERO == MATH::CDecimalStatistics::LinearRegression( constant, ys, 4, scale3, slope, intercept, rSquared ) );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DecimalStatistics: regression of noisy decimals agrees with Float64" )
    {
        const MATH::UInt64 seed = 77;
        MATH::CXoshiro256StarStar generator( seed );
        CORE::CDecimal xs[ 30 ], ys[ 30 ];
        MATH::Float64 fx[ 30 ], fy[ 30 ];
        for ( int i = 0; i < 30; ++i )
        {
            const MATH::Int64 x = i * 10 + (MATH::Int64) ( generator.NextUInt32() % 7 );
            const MATH::Int64 y = 3 * x + 500 + (MATH::Int64) ( generator.NextUInt32() % 200 ) - 100;
            CORE::CDecimal::FromUnscaled( x, 2, xs[ i ] );
            CORE::CDecimal::FromUnscaled( y, 2, ys[ i ] );
            fx[ i ] = (MATH::Float64) x / 100.0;
            fy[ i ] = (MATH::Float64) y / 100.0;
        }
        CORE::CDecimal slope, intercept, rSquared;
        ASSERT_TRUE( !CORE::DecimalStatusIsError( MATH::CDecimalStatistics::LinearRegression( xs, ys, 30, scale6, slope, intercept, rSquared ) ) );
        MATH::Float64 fs = 0, fi = 0, fr = 0;
        MATH::LinearRegression( fx, fy, 30, fs, fi, fr );
        const bool ok = fabs( slope.ToFloat64() - fs ) < 2e-6 && fabs( intercept.ToFloat64() - fi ) < 3e-6 && fabs( rSquared.ToFloat64() - fr ) < 3e-6;
        if ( !ok )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DecimalStatistics: seed " << seed << " slope " << slope.ToString() << " / " << fs << " intercept " << intercept.ToString() << " / " << fi << " r2 " << rSquared.ToString() << " / " << fr );
        ASSERT_TRUE( ok );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DecimalStatistics: scale and overflow limits are reported" )
    {
        const CORE::CDecimal fine[ 3 ] = { Dec( "0.0000000001" ), Dec( "0.0000000002" ), Dec( "0.0000000004" ) };     // scale 10
        CORE::CDecimal result;
        ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_SCALE == MATH::CDecimalStatistics::Variance( fine, 3, false, scale6, result ) );
        const CORE::CDecimal big[ 2 ] = { Dec( "9000000000000000000" ), Dec( "9000000000000000000" ) };
        ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == MATH::CDecimalStatistics::Mean( big, 2, scale0, result ) );
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformStatisticsTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Statistics TESTS" );

    GUCEF_TESTFW_TESTCASE( "Statistics: values 1 to 10" )
    {
        MATH::Float64 values[ 10 ];
        for ( int i = 0; i < 10; ++i ) values[ i ] = i + 1;

        ASSERT_TRUE( fabs( MATH::Mean( values, 10 ) - 5.5 ) < 1e-12 );
        ASSERT_TRUE( fabs( MATH::Variance( values, 10 ) - 8.25 ) < 1e-12 );
        ASSERT_TRUE( fabs( MATH::Variance( values, 10, true ) - 9.1666666666666667 ) < 1e-12 );
        ASSERT_TRUE( fabs( MATH::StandardDeviation( values, 10 ) - sqrt( 8.25 ) ) < 1e-12 );
        ASSERT_TRUE( fabs( MATH::StandardDeviation( values, 10, true ) - sqrt( 9.1666666666666667 ) ) < 1e-12 );

        MATH::CTRunningStatistics< MATH::Float64 > running;
        running.Add( values, 10 );
        ASSERT_TRUE( 10 == running.GetCount() );
        ASSERT_TRUE( fabs( running.GetMean() - 5.5 ) < 1e-12 );
        ASSERT_TRUE( fabs( running.GetPopulationVariance() - 8.25 ) < 1e-12 );
        ASSERT_TRUE( fabs( running.GetSampleVariance() - 9.1666666666666667 ) < 1e-12 );
        ASSERT_TRUE( fabs( running.GetPopulationStandardDeviation() - sqrt( 8.25 ) ) < 1e-12 );
        ASSERT_TRUE( fabs( running.GetSampleStandardDeviation() - sqrt( 9.1666666666666667 ) ) < 1e-12 );
        ASSERT_TRUE( running.GetMin() == 1.0 && running.GetMax() == 10.0 );

        running.Reset();
        ASSERT_TRUE( 0 == running.GetCount() && 0.0 == running.GetMean() && 0.0 == running.GetSampleVariance() );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Statistics: empty and single value inputs are defined" )
    {
        MATH::Float64 one[ 1 ] = { 42.0 };
        ASSERT_TRUE( MATH::Mean( one, 0 ) == 0.0 );
        ASSERT_TRUE( MATH::Variance( one, 0 ) == 0.0 );
        ASSERT_TRUE( MATH::Variance( one, 1, true ) == 0.0 );
        ASSERT_TRUE( MATH::Variance( one, 1 ) == 0.0 );
        ASSERT_TRUE( MATH::Mean( one, 1 ) == 42.0 );
        MATH::CTRunningStatistics< MATH::Float64 > running;
        ASSERT_TRUE( 0.0 == running.GetPopulationVariance() && 0.0 == running.GetPopulationStandardDeviation() );
        running.Add( 42.0 );
        ASSERT_TRUE( 0.0 == running.GetSampleVariance() && 42.0 == running.GetMean() && 42.0 == running.GetMin() && 42.0 == running.GetMax() );
        MATH::Float64 scratch[ 1 ] = { 5.0 };
        ASSERT_TRUE( MATH::Median( scratch, 0 ) == 0.0 );
        ASSERT_TRUE( MATH::Median( scratch, 1 ) == 5.0 );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Statistics: large offset data keeps its precision (sample variance 30)" )
    {
        const MATH::Float64 values[ 4 ] = { 1e9 + 4.0, 1e9 + 7.0, 1e9 + 13.0, 1e9 + 16.0 };
        ASSERT_TRUE( fabs( MATH::Variance( values, 4, true ) - 30.0 ) < 1e-6 );
        ASSERT_TRUE( fabs( MATH::Mean( values, 4 ) - ( 1e9 + 10.0 ) ) < 1e-6 );
        MATH::CTRunningStatistics< MATH::Float64 > running;
        running.Add( values, 4 );
        ASSERT_TRUE( fabs( running.GetSampleVariance() - 30.0 ) < 1e-6 );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Statistics: regression of y = 2x + 1, covariance and correlation" )
    {
        MATH::Float64 xs[ 20 ], ys[ 20 ], negative[ 20 ];
        for ( int i = 0; i < 20; ++i )
        {
            xs[ i ] = i - 5;
            ys[ i ] = 2.0 * xs[ i ] + 1.0;
            negative[ i ] = -3.0 * xs[ i ];
        }
        MATH::Float64 slope = 0, intercept = 0, rSquared = 0;
        ASSERT_TRUE( MATH::LinearRegression( xs, ys, 20, slope, intercept, rSquared ) );
        ASSERT_TRUE( slope == 2.0 && intercept == 1.0 && rSquared == 1.0 );
        ASSERT_TRUE( fabs( MATH::Correlation( xs, ys, 20 ) - 1.0 ) < 1e-15 );
        ASSERT_TRUE( fabs( MATH::Correlation( xs, negative, 20 ) + 1.0 ) < 1e-15 );
        ASSERT_TRUE( fabs( MATH::Covariance( xs, ys, 20 ) - 2.0 * MATH::Variance( xs, 20 ) ) < 1e-9 );
        ASSERT_TRUE( fabs( MATH::Covariance( xs, ys, 20, true ) - 2.0 * MATH::Variance( xs, 20, true ) ) < 1e-9 );
        ASSERT_TRUE( fabs( MATH::Covariance( xs, xs, 20 ) - MATH::Variance( xs, 20 ) ) < 1e-12 );

        // A noisy line: the fit lies between the extremes and r squared below 1
        MATH::CXoshiro256StarStar generator( 2025 );
        MATH::Float64 noisy[ 20 ];
        for ( int i = 0; i < 20; ++i )
            noisy[ i ] = ys[ i ] + ( (MATH::Float64) ( generator.NextUInt32() % 1000 ) / 1000.0 - 0.5 );
        ASSERT_TRUE( MATH::LinearRegression( xs, noisy, 20, slope, intercept, rSquared ) );
        ASSERT_TRUE( fabs( slope - 2.0 ) < 0.1 && rSquared < 1.0 && rSquared > 0.99 );

        // Degenerate: fewer than two points, or all x equal
        MATH::Float64 constant[ 5 ] = { 3, 3, 3, 3, 3 };
        ASSERT_FALSE( MATH::LinearRegression( xs, ys, 1, slope, intercept, rSquared ) );
        ASSERT_TRUE( slope == 0.0 && intercept == 0.0 && rSquared == 0.0 );
        ASSERT_FALSE( MATH::LinearRegression( constant, ys, 5, slope, intercept, rSquared ) );
        ASSERT_TRUE( MATH::Correlation( constant, ys, 5 ) == 0.0 );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Statistics: Merge of two halves equals the whole" )
    {
        MATH::CXoshiro256StarStar generator( 8675309 );
        MATH::Float64 values[ 1000 ];
        for ( int i = 0; i < 1000; ++i )
            values[ i ] = 100.0 + (MATH::Float64) ( generator.NextUInt32() % 100000 ) / 1000.0;

        MATH::CTRunningStatistics< MATH::Float64 > whole;
        whole.Add( values, 1000 );
        MATH::CTRunningStatistics< MATH::Float64 > left, right;
        left.Add( values, 400 );
        right.Add( values + 400, 600 );
        left.Merge( right );
        ASSERT_TRUE( left.GetCount() == whole.GetCount() );
        ASSERT_TRUE( fabs( left.GetMean() - whole.GetMean() ) < 1e-9 );
        ASSERT_TRUE( fabs( left.GetSampleVariance() - whole.GetSampleVariance() ) < 1e-9 );
        ASSERT_TRUE( left.GetMin() == whole.GetMin() && left.GetMax() == whole.GetMax() );

        // The running statistics agree with the two pass functions
        ASSERT_TRUE( fabs( whole.GetMean() - MATH::Mean( values, 1000 ) ) < 1e-9 );
        ASSERT_TRUE( fabs( whole.GetPopulationVariance() - MATH::Variance( values, 1000 ) ) < 1e-8 );

        // Merging with an empty accumulator changes nothing, in both directions
        MATH::CTRunningStatistics< MATH::Float64 > empty;
        MATH::CTRunningStatistics< MATH::Float64 > copy = whole;
        copy.Merge( empty );
        ASSERT_TRUE( copy.GetCount() == whole.GetCount() && copy.GetMean() == whole.GetMean() );
        empty.Merge( whole );
        ASSERT_TRUE( empty.GetCount() == whole.GetCount() && empty.GetMean() == whole.GetMean() && empty.GetMin() == whole.GetMin() );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Statistics: median and percentiles, all methods" )
    {
        MATH::Float64 odd[ 5 ] = { 9, 1, 5, 3, 7 };
        ASSERT_TRUE( MATH::Median( odd, 5 ) == 5.0 );
        MATH::Float64 even[ 4 ] = { 4, 1, 3, 2 };
        ASSERT_TRUE( MATH::Median( even, 4 ) == 2.5 );

        const MATH::Float64 source[ 5 ] = { 15, 20, 35, 40, 50 };
        MATH::Float64 scratch[ 5 ];
        #define COPYSOURCE for ( int c = 0; c < 5; ++c ) scratch[ c ] = source[ 4 - c ]
        COPYSOURCE;
        ASSERT_TRUE( MATH::Percentile( scratch, 5, 0.0 ) == 15.0 );
        COPYSOURCE;
        ASSERT_TRUE( MATH::Percentile( scratch, 5, 100.0 ) == 50.0 );
        COPYSOURCE;
        ASSERT_TRUE( fabs( MATH::Percentile( scratch, 5, 40.0 ) - 29.0 ) < 1e-12 );                  // type 7: position 1.6
        COPYSOURCE;
        ASSERT_TRUE( fabs( MATH::Percentile( scratch, 5, 40.0, MATH::PERCENTILE_LINEAR_EXCLUSIVE ) - 26.0 ) < 1e-12 );   // type 6: position 1.4, 20 + 0.4 * 15
        COPYSOURCE;
        ASSERT_TRUE( MATH::Percentile( scratch, 5, 40.0, MATH::PERCENTILE_NEAREST_RANK ) == 20.0 );  // rank ceil( 0.4 * 5 ) = 2
        COPYSOURCE;
        ASSERT_TRUE( MATH::Percentile( scratch, 5, 30.0, MATH::PERCENTILE_NEAREST_RANK ) == 20.0 );  // rank ceil( 1.5 ) = 2
        COPYSOURCE;
        ASSERT_TRUE( MATH::Percentile( scratch, 5, 40.0, MATH::PERCENTILE_LOWER ) == 20.0 );
        COPYSOURCE;
        ASSERT_TRUE( MATH::Percentile( scratch, 5, 40.0, MATH::PERCENTILE_HIGHER ) == 35.0 );
        COPYSOURCE;
        ASSERT_TRUE( MATH::Percentile( scratch, 5, 40.0, MATH::PERCENTILE_MIDPOINT ) == 27.5 );
        COPYSOURCE;
        ASSERT_TRUE( MATH::Percentile( scratch, 5, 50.0, MATH::PERCENTILE_HIGHER ) == 35.0 );        // exact position: no interpolation partner
        COPYSOURCE;
        ASSERT_TRUE( MATH::Percentile( scratch, 5, 250.0 ) == 50.0 );                                  // clamped
        COPYSOURCE;
        ASSERT_TRUE( MATH::Percentile( scratch, 5, -3.0 ) == 15.0 );
        #undef COPYSOURCE
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Statistics: SortAscending sorts random arrays, duplicates and presorted input" )
    {
        MATH::CXoshiro256StarStar generator( 1234 );
        bool ok = true;
        for ( int round = 0; round < 200; ++round )
        {
            const MATH::UInt32 count = generator.NextUInt32() % 64;
            MATH::Float64 values[ 64 ];
            for ( MATH::UInt32 i = 0; i < count; ++i )
                values[ i ] = (MATH::Float64) ( generator.NextUInt32() % 20 ) - 10.0;
            MATH::Float64 sumBefore = 0.0;
            for ( MATH::UInt32 i = 0; i < count; ++i ) sumBefore += values[ i ];
            MATH::SortAscending( values, count );
            MATH::Float64 sumAfter = 0.0;
            for ( MATH::UInt32 i = 0; i < count; ++i )
            {
                sumAfter += values[ i ];
                if ( i > 0 && values[ i ] < values[ i - 1 ] ) ok = false;
            }
            if ( sumBefore != sumAfter ) ok = false;
        }
        ASSERT_TRUE( ok );
    }
    GUCEF_TESTFW_TESTCASE_END

    PerformDecimalStatisticsTests();

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Statistics TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
