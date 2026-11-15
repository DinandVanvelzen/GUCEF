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

#ifndef GUCEF_MATH_TESTAPP_TESTRANDOMSTATISTICS_H
#define GUCEF_MATH_TESTAPP_TESTRANDOMSTATISTICS_H

/*
 *  Statistics helpers of the tests of random numbers, sampling and noise: the chi-squared statistic and its critical
 *  value, and the check macro that prints the seed of a failing seeded test. Specific to the gucefMATH test
 *  application, so it lives here.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>
#include <string.h>

#ifndef GUCEF_MATH_MACROS_H
#include "gucefMATH_macros.h"
#define GUCEF_MATH_MACROS_H
#endif /* GUCEF_MATH_MACROS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestMathReference.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace RandomStatistics {

/** The chi-squared statistic of observed counts against expected counts */
inline double
ChiSquared( const double* observed, const double* expected, GUCEF::UInt32 binCount )
{
    double sum = 0.0;
    for ( GUCEF::UInt32 i=0; i<binCount; ++i )
        if ( expected[ i ] > 0.0 )
            sum += ( observed[ i ] - expected[ i ] ) * ( observed[ i ] - expected[ i ] ) / expected[ i ];
    return sum;
}

/** Chi-squared statistic of counts against equal expected counts, over 'binCount' bins */
inline double
ChiSquaredUniform( const double* observed, GUCEF::UInt32 binCount, double total )
{
    double expected[ 256 ];
    for ( GUCEF::UInt32 i=0; i<binCount && i<256; ++i )
        expected[ i ] = total / binCount;
    return ChiSquared( observed, expected, binCount < 256 ? binCount : 256 );
}

/**
 *  The chi-squared value that a true distribution exceeds with probability 0.001 for 'degreesOfFreedom' degrees of
 *  freedom ( Wilson-Hilferty approximation, a little above the exact value: 28.1 for 9 degrees where the exact one is
 *  27.88 ). A statistic below it passes at p > 0.001.
 */
inline double
ChiSquaredCritical( GUCEF::UInt32 degreesOfFreedom )
{
    const double k = (double) degreesOfFreedom;
    const double z = 3.0902;
    const double c = 2.0 / ( 9.0 * k );
    const double base = 1.0 - c + z * sqrt( c );
    return k * base * base * base;
}

/** The bits of a Float64 as a signed integer, for hashing */
inline GUCEF::Int64
BitsOf( GUCEF::Float64 value )
{
    GUCEF::Int64 bits = 0;
    memcpy( &bits, &value, sizeof( bits ) );
    return bits;
}

}; /* namespace RandomStatistics */

/** Asserts 'condition' and, when it fails, logs the seed that reproduces the failure */
#define ASSERT_SEEDED( condition, seed )                                                                                \
    {                                                                                                                   \
        const bool conditionHolds = ( condition );                                                                      \
        if ( !conditionHolds )                                                                                          \
            GUCEF_ERROR_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "Seeded test failed, seed " << (GUCEF::UInt64) ( seed ) );  \
        GUCEF_TESTFW_ASSERT_TRUE( conditionHolds );                                                                     \
    }

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_TESTAPP_TESTRANDOMSTATISTICS_H ? */
