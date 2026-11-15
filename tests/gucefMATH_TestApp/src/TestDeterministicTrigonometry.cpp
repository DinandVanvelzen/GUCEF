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
#include <float.h>
#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_MATH_DETERMINISTICTRIGONOMETRY_H
#include "gucefMATH_DeterministicTrigonometry.h"
#define GUCEF_MATH_DETERMINISTICTRIGONOMETRY_H
#endif /* GUCEF_MATH_DETERMINISTICTRIGONOMETRY_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_MATH_CPCG32_H
#include "gucefMATH_CPcg32.h"
#define GUCEF_MATH_CPCG32_H
#endif /* GUCEF_MATH_CPCG32_H ? */

#include "TestMathAllocationCounter.h"
#include "TestDeterministicTrigonometry.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/** The long double reference needs the 64 bit mantissa of the x87 format or more; elsewhere (long double is double) the pinned values stand alone */
#if defined( LDBL_MANT_DIG ) && ( LDBL_MANT_DIG >= 64 )
  #define TRIGTEST_HAS_LONG_DOUBLE_REFERENCE 1
#else
  #define TRIGTEST_HAS_LONG_DOUBLE_REFERENCE 0
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      PINNED VALUES                                                      //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The correctly rounded sine and cosine of 2 pi * numerator / denominator, as the bit patterns of the Float64 values,
 *  computed with decimal arithmetic of 100 digits from the exact fraction (independent of the code under test).
 *  The first 64 rows are the whole circle in 64 steps, the rest are fractions of a degree, large denominators up to the
 *  limit 2^59, numerators beyond a turn, negative numerators and denominators, and the exact multiples of a quarter turn.
 */
struct SPinnedTrigonometry
{
    Int64 numerator;
    Int64 denominator;
    UInt64 sineBits;
    UInt64 cosineBits;
};

static const SPinnedTrigonometry PinnedValues[] =
{
    { 0LL, 64LL, 0x0000000000000000ULL, 0x3ff0000000000000ULL },
    { 1LL, 64LL, 0x3fb917a6bc29b42cULL, 0x3fefd88da3d12526ULL },
    { 2LL, 64LL, 0x3fc8f8b83c69a60bULL, 0x3fef6297cff75cb0ULL },
    { 3LL, 64LL, 0x3fd294062ed59f06ULL, 0x3fee9f4156c62ddaULL },
    { 4LL, 64LL, 0x3fd87de2a6aea963ULL, 0x3fed906bcf328d46ULL },
    { 5LL, 64LL, 0x3fde2b5d3806f63bULL, 0x3fec38b2f180bdb1ULL },
    { 6LL, 64LL, 0x3fe1c73b39ae68c8ULL, 0x3fea9b66290ea1a3ULL },
    { 7LL, 64LL, 0x3fe44cf325091dd6ULL, 0x3fe8bc806b151741ULL },
    { 8LL, 64LL, 0x3fe6a09e667f3bcdULL, 0x3fe6a09e667f3bcdULL },
    { 9LL, 64LL, 0x3fe8bc806b151741ULL, 0x3fe44cf325091dd6ULL },
    { 10LL, 64LL, 0x3fea9b66290ea1a3ULL, 0x3fe1c73b39ae68c8ULL },
    { 11LL, 64LL, 0x3fec38b2f180bdb1ULL, 0x3fde2b5d3806f63bULL },
    { 12LL, 64LL, 0x3fed906bcf328d46ULL, 0x3fd87de2a6aea963ULL },
    { 13LL, 64LL, 0x3fee9f4156c62ddaULL, 0x3fd294062ed59f06ULL },
    { 14LL, 64LL, 0x3fef6297cff75cb0ULL, 0x3fc8f8b83c69a60bULL },
    { 15LL, 64LL, 0x3fefd88da3d12526ULL, 0x3fb917a6bc29b42cULL },
    { 16LL, 64LL, 0x3ff0000000000000ULL, 0x0000000000000000ULL },
    { 17LL, 64LL, 0x3fefd88da3d12526ULL, 0xbfb917a6bc29b42cULL },
    { 18LL, 64LL, 0x3fef6297cff75cb0ULL, 0xbfc8f8b83c69a60bULL },
    { 19LL, 64LL, 0x3fee9f4156c62ddaULL, 0xbfd294062ed59f06ULL },
    { 20LL, 64LL, 0x3fed906bcf328d46ULL, 0xbfd87de2a6aea963ULL },
    { 21LL, 64LL, 0x3fec38b2f180bdb1ULL, 0xbfde2b5d3806f63bULL },
    { 22LL, 64LL, 0x3fea9b66290ea1a3ULL, 0xbfe1c73b39ae68c8ULL },
    { 23LL, 64LL, 0x3fe8bc806b151741ULL, 0xbfe44cf325091dd6ULL },
    { 24LL, 64LL, 0x3fe6a09e667f3bcdULL, 0xbfe6a09e667f3bcdULL },
    { 25LL, 64LL, 0x3fe44cf325091dd6ULL, 0xbfe8bc806b151741ULL },
    { 26LL, 64LL, 0x3fe1c73b39ae68c8ULL, 0xbfea9b66290ea1a3ULL },
    { 27LL, 64LL, 0x3fde2b5d3806f63bULL, 0xbfec38b2f180bdb1ULL },
    { 28LL, 64LL, 0x3fd87de2a6aea963ULL, 0xbfed906bcf328d46ULL },
    { 29LL, 64LL, 0x3fd294062ed59f06ULL, 0xbfee9f4156c62ddaULL },
    { 30LL, 64LL, 0x3fc8f8b83c69a60bULL, 0xbfef6297cff75cb0ULL },
    { 31LL, 64LL, 0x3fb917a6bc29b42cULL, 0xbfefd88da3d12526ULL },
    { 32LL, 64LL, 0x0000000000000000ULL, 0xbff0000000000000ULL },
    { 33LL, 64LL, 0xbfb917a6bc29b42cULL, 0xbfefd88da3d12526ULL },
    { 34LL, 64LL, 0xbfc8f8b83c69a60bULL, 0xbfef6297cff75cb0ULL },
    { 35LL, 64LL, 0xbfd294062ed59f06ULL, 0xbfee9f4156c62ddaULL },
    { 36LL, 64LL, 0xbfd87de2a6aea963ULL, 0xbfed906bcf328d46ULL },
    { 37LL, 64LL, 0xbfde2b5d3806f63bULL, 0xbfec38b2f180bdb1ULL },
    { 38LL, 64LL, 0xbfe1c73b39ae68c8ULL, 0xbfea9b66290ea1a3ULL },
    { 39LL, 64LL, 0xbfe44cf325091dd6ULL, 0xbfe8bc806b151741ULL },
    { 40LL, 64LL, 0xbfe6a09e667f3bcdULL, 0xbfe6a09e667f3bcdULL },
    { 41LL, 64LL, 0xbfe8bc806b151741ULL, 0xbfe44cf325091dd6ULL },
    { 42LL, 64LL, 0xbfea9b66290ea1a3ULL, 0xbfe1c73b39ae68c8ULL },
    { 43LL, 64LL, 0xbfec38b2f180bdb1ULL, 0xbfde2b5d3806f63bULL },
    { 44LL, 64LL, 0xbfed906bcf328d46ULL, 0xbfd87de2a6aea963ULL },
    { 45LL, 64LL, 0xbfee9f4156c62ddaULL, 0xbfd294062ed59f06ULL },
    { 46LL, 64LL, 0xbfef6297cff75cb0ULL, 0xbfc8f8b83c69a60bULL },
    { 47LL, 64LL, 0xbfefd88da3d12526ULL, 0xbfb917a6bc29b42cULL },
    { 48LL, 64LL, 0xbff0000000000000ULL, 0x0000000000000000ULL },
    { 49LL, 64LL, 0xbfefd88da3d12526ULL, 0x3fb917a6bc29b42cULL },
    { 50LL, 64LL, 0xbfef6297cff75cb0ULL, 0x3fc8f8b83c69a60bULL },
    { 51LL, 64LL, 0xbfee9f4156c62ddaULL, 0x3fd294062ed59f06ULL },
    { 52LL, 64LL, 0xbfed906bcf328d46ULL, 0x3fd87de2a6aea963ULL },
    { 53LL, 64LL, 0xbfec38b2f180bdb1ULL, 0x3fde2b5d3806f63bULL },
    { 54LL, 64LL, 0xbfea9b66290ea1a3ULL, 0x3fe1c73b39ae68c8ULL },
    { 55LL, 64LL, 0xbfe8bc806b151741ULL, 0x3fe44cf325091dd6ULL },
    { 56LL, 64LL, 0xbfe6a09e667f3bcdULL, 0x3fe6a09e667f3bcdULL },
    { 57LL, 64LL, 0xbfe44cf325091dd6ULL, 0x3fe8bc806b151741ULL },
    { 58LL, 64LL, 0xbfe1c73b39ae68c8ULL, 0x3fea9b66290ea1a3ULL },
    { 59LL, 64LL, 0xbfde2b5d3806f63bULL, 0x3fec38b2f180bdb1ULL },
    { 60LL, 64LL, 0xbfd87de2a6aea963ULL, 0x3fed906bcf328d46ULL },
    { 61LL, 64LL, 0xbfd294062ed59f06ULL, 0x3fee9f4156c62ddaULL },
    { 62LL, 64LL, 0xbfc8f8b83c69a60bULL, 0x3fef6297cff75cb0ULL },
    { 63LL, 64LL, 0xbfb917a6bc29b42cULL, 0x3fefd88da3d12526ULL },
    { 1LL, 3LL, 0x3febb67ae8584caaULL, 0xbfe0000000000000ULL },
    { 1LL, 5LL, 0x3fee6f0e134454ffULL, 0x3fd3c6ef372fe950ULL },
    { 1LL, 7LL, 0x3fe904c37505de4bULL, 0x3fe3f3a0e28bedd1ULL },
    { 2LL, 7LL, 0x3fef329c0558e969ULL, 0xbfcc7b90e3024582ULL },
    { 3LL, 7LL, 0x3fdbc4c04d71abc1ULL, 0xbfecd4bca9cb5c71ULL },
    { 1LL, 9LL, 0x3fe491b7523c161dULL, 0x3fe8836fa2cf5039ULL },
    { 1LL, 10LL, 0x3fe2cf2304755a5eULL, 0x3fe9e3779b97f4a8ULL },
    { 1LL, 12LL, 0x3fe0000000000000ULL, 0x3febb67ae8584caaULL },
    { 1LL, 360LL, 0x3f91df0b2b89dd1eULL, 0x3feffec097f5af8aULL },
    { 7LL, 360LL, 0x3fbf32d44c4f62d3ULL, 0x3fefc2f025a23e8bULL },
    { 45LL, 360LL, 0x3fe6a09e667f3bcdULL, 0x3fe6a09e667f3bcdULL },
    { 100LL, 360LL, 0x3fef838b8c811c17ULL, 0xbfc63a1a7e0b738aULL },
    { 359LL, 360LL, 0xbf91df0b2b89dd1eULL, 0x3feffec097f5af8aULL },
    { 1LL, 1000LL, 0x3f79bc5a9d91f679ULL, 0x3fefffd69aa0b99dULL },
    { 999LL, 1000LL, 0xbf79bc5a9d91f679ULL, 0x3fefffd69aa0b99dULL },
    { 355LL, 113LL, 0x3fe8dbfd7f2218a9ULL, 0x3fe426578f10f90eULL },
    { -1LL, 3LL, 0xbfebb67ae8584caaULL, 0xbfe0000000000000ULL },
    { -2LL, 7LL, 0xbfef329c0558e969ULL, 0xbfcc7b90e3024582ULL },
    { 5LL, 3LL, 0xbfebb67ae8584caaULL, 0xbfe0000000000000ULL },
    { 100003LL, 17LL, 0xbfc7851aacd6c6b4ULL, 0xbfef7484007faef3ULL },
    { 1LL, 1099511627776LL, 0x3d9921fb54442d18ULL, 0x3ff0000000000000ULL },
    { 1099511627775LL, 1099511627776LL, 0xbd9921fb54442d18ULL, 0x3ff0000000000000ULL },
    { 12345678901LL, 34359738368LL, 0x3fe8bebc7f3ff49cULL, 0xbfe44a39c246488bULL },
    { 3LL, 8LL, 0x3fe6a09e667f3bcdULL, 0xbfe6a09e667f3bcdULL },
    { 5LL, 8LL, 0xbfe6a09e667f3bcdULL, 0xbfe6a09e667f3bcdULL },
    { 7LL, 8LL, 0xbfe6a09e667f3bcdULL, 0x3fe6a09e667f3bcdULL },
    { 1LL, 16LL, 0x3fd87de2a6aea963ULL, 0x3fed906bcf328d46ULL },
    { 15LL, 16LL, 0xbfd87de2a6aea963ULL, 0x3fed906bcf328d46ULL },
    { 0LL, 5LL, 0x0000000000000000ULL, 0x3ff0000000000000ULL },
    { 5LL, 5LL, 0x0000000000000000ULL, 0x3ff0000000000000ULL },
    { 5LL, 4LL, 0x3ff0000000000000ULL, 0x0000000000000000ULL },
    { 7LL, 4LL, 0xbff0000000000000ULL, 0x0000000000000000ULL },
    { -3LL, 8LL, 0xbfe6a09e667f3bcdULL, 0xbfe6a09e667f3bcdULL },
    { 1LL, -3LL, 0xbfebb67ae8584caaULL, 0xbfe0000000000000ULL },
    { -1LL, -3LL, 0x3febb67ae8584caaULL, 0xbfe0000000000000ULL },
    { 2LL, -8LL, 0xbff0000000000000ULL, 0x0000000000000000ULL },
    { 1LL, 576460752303423488LL, 0x3c6921fb54442d18ULL, 0x3ff0000000000000ULL },
    { 576460752303423487LL, 576460752303423488LL, 0xbc6921fb54442d18ULL, 0x3ff0000000000000ULL },
    { 432345564227579961LL, 576460752303423488LL, 0xbff0000000000000ULL, 0x3d42efdec30a2e8aULL },
    { 123456789123456789LL, 288230376151711743LL, 0x3fdbdb6f02c7969cULL, 0xbfeccf43bb48f165ULL },
    { 1LL, 576460752303423487LL, 0x3c6921fb54442d18ULL, 0x3ff0000000000000ULL },
    { 576460752303423486LL, 576460752303423487LL, 0xbc6921fb54442d18ULL, 0x3ff0000000000000ULL },
    { 9223372036854775807LL, 1000003LL, 0xbfec8b19890423b1ULL, 0xbfdceecbd03dfd56ULL },
    { (-9223372036854775807LL - 1), 7LL, 0xbfe904c37505de4bULL, 0x3fe3f3a0e28bedd1ULL },
    { 9223372036854775807LL, 576460752303423488LL, 0xbc6921fb54442d18ULL, 0x3ff0000000000000ULL },
    { -4611686018427400249LL, 576460752303423485LL, 0xbd42f94b8149c81bULL, 0x3ff0000000000000ULL },
    { 1LL, 216172782113783808LL, 0x3c80c152382d7366ULL, 0x3ff0000000000000ULL },
    { 7777777LL, 3377699720527872LL, 0x3e4f11f941cd432aULL, 0x3fefffffffffffffULL },
    { 1LL, 1LL, 0x0000000000000000ULL, 0x3ff0000000000000ULL },
    { 0LL, 1LL, 0x0000000000000000ULL, 0x3ff0000000000000ULL },
    { 4LL, 1LL, 0x0000000000000000ULL, 0x3ff0000000000000ULL },
    { -1LL, 1LL, 0x0000000000000000ULL, 0x3ff0000000000000ULL }
};

static const UInt32 PinnedValueCount = (UInt32) ( sizeof( PinnedValues ) / sizeof( PinnedValues[ 0 ] ) );

/*
 *  FNV-1a 64 bit hash of the results of SinCosOfTurnFraction over a large sweep: every fraction j / N for the
 *  denominators 2 to 2^16 in powers of two, and 2000 fractions with odd denominators
 */
static const UInt64 PinnedSweepHash = 0x895d3db2494d03b4ULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static UInt64
BitsOf( Float64 value )
{
    UInt64 bits = 0;
    memcpy( &bits, &value, sizeof( bits ) );
    return bits;
}

static UInt64
HashBits( UInt64 hash, Float64 value )
{
    const UInt64 bits = BitsOf( value );
    for ( UInt32 i = 0; i < 8; ++i )
    {
        hash ^= ( bits >> ( 8 * i ) ) & 0xFF;
        hash *= (UInt64) 0x100000001b3ULL;
    }
    return hash;
}

static GUCEF::CORE::CString
HashToText( UInt64 hash )
{
    char text[ 24 ];
    for ( UInt32 i = 0; i < 16; ++i )
        text[ i ] = "0123456789abcdef"[ ( hash >> ( 60 - 4 * i ) ) & 0xF ];
    text[ 16 ] = '\0';
    return GUCEF::CORE::CString( text );
}

static UInt64
ComputeSweepHash( void )
{
    UInt64 hash = (UInt64) 0xcbf29ce484222325ULL;
    for ( UInt32 exponent = 1; exponent <= 16; ++exponent )
    {
        const Int64 size = (Int64) 1 << exponent;
        for ( Int64 j = 0; j < size; ++j )
        {
            Float64 s, c;
            MATH::SinCosOfTurnFraction( j, size, s, c );
            hash = HashBits( HashBits( hash, s ), c );
        }
    }
    for ( Int64 d = 1; d < 2000; d += 2 )
    {
        for ( Int64 n = -d; n < 2 * d; n += 1 + d / 5 )
        {
            Float64 s, c;
            MATH::SinCosOfTurnFraction( n * 7919, d, s, c );
            hash = HashBits( HashBits( hash, s ), c );
        }
    }
    return hash;
}

#if TRIGTEST_HAS_LONG_DOUBLE_REFERENCE

/**
 *  Sine and cosine of 2 pi * numerator / denominator in long double. The fraction is reduced to the nearest quarter turn
 *  with integer arithmetic first, so that the angle left is at most an eighth of a turn and has a small relative error
 *  (the values near zero of the sine or the cosine are then as accurate as the others). Needs 0 < denominator <= 2^59.
 */
static void
ReferenceSinCos( Int64 numerator, Int64 denominator, long double& s, long double& c )
{
    const long double pi = 3.14159265358979323846264338327950288L;
    Int64 r = numerator % denominator;
    if ( r < 0 )
        r += denominator;
    const Int64 quarters = ( 4 * r + denominator / 2 ) / denominator;        // nearest quarter turn, 0 to 4
    const Int64 rest = 4 * r - quarters * denominator;                       // the angle left, in units of 1 / ( 4 denominator ) turn
    const long double angle = 2.0L * pi * (long double) rest / ( 4.0L * (long double) denominator );
    const long double sa = sin( angle );
    const long double ca = cos( angle );
    switch ( quarters % 4 )
    {
        case 0: s = sa;     c = ca;     break;
        case 1: s = ca;     c = -sa;    break;
        case 2: s = -sa;    c = -ca;    break;
        default: s = -ca;   c = sa;     break;
    }
}

/** Distance of 'value' from 'reference' in units of the spacing of Float64 values at the reference; 0 for an exact zero, large when only one is zero */
static long double
UlpError( Float64 value, long double reference )
{
    if ( 0.0L == reference )
        return 0.0 == value ? 0.0L : 1e30L;
    int exponent = 0;
    frexp( reference, &exponent );
    const long double ulp = ldexp( 1.0L, exponent - 53 );
    long double difference = (long double) value - reference;
    if ( difference < 0.0L )
        difference = -difference;
    return difference / ulp;
}

static long double
UlpErrorFloat32( Float32 value, long double reference )
{
    if ( 0.0L == reference )
        return 0.0f == value ? 0.0L : 1e30L;
    int exponent = 0;
    frexp( reference, &exponent );
    const long double ulp = ldexp( 1.0L, exponent - 24 );
    long double difference = (long double) value - reference;
    if ( difference < 0.0L )
        difference = -difference;
    return difference / ulp;
}

#endif /* TRIGTEST_HAS_LONG_DOUBLE_REFERENCE ? */

/** Calls marked as a real time context: in a RealtimeSanitizer build any allocation or lock in them is reported */
static Float64
SinCosRealtime( Int64 numerator, Int64 denominator ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float64 s, c;
    Float32 s32, c32;
    MATH::SinCosOfTurnFraction( numerator, denominator, s, c );
    MATH::SinCosOfTurnFraction( numerator, denominator, s32, c32 );
    return s + c + (Float64) s32 + (Float64) c32;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDeterministicTrigonometryTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING DeterministicTrigonometry TESTS" );

    GUCEF_TESTFW_TESTCASE( "DeterministicTrigonometry: every pinned value is the correctly rounded result, bit for bit" )
    try
    {
        UInt32 mismatches = 0;
        for ( UInt32 i = 0; i < PinnedValueCount; ++i )
        {
            Float64 s = -7.0, c = -7.0;
            const bool ok = MATH::SinCosOfTurnFraction( PinnedValues[ i ].numerator, PinnedValues[ i ].denominator, s, c );
            if ( !ok || BitsOf( s ) != PinnedValues[ i ].sineBits || BitsOf( c ) != PinnedValues[ i ].cosineBits )
            {
                ++mismatches;
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DeterministicTrigonometry: pinned value " << i << " ( " << PinnedValues[ i ].numerator << " / " << PinnedValues[ i ].denominator << " ) differs" );
            }
        }
        ASSERT_TRUE( 0 == mismatches );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DeterministicTrigonometry: quarter turns are exact (positive zero), eighth turns give equal sine and cosine" )
    try
    {
        Float64 s, c;
        const Float64 expectedSine[ 4 ] = { 0.0, 1.0, 0.0, -1.0 };
        const Float64 expectedCosine[ 4 ] = { 1.0, 0.0, -1.0, 0.0 };
        const Int64 sizes[ 6 ] = { 4, 8, 12, 64, 1000, (Int64) 1 << 59 };
        for ( UInt32 k = 0; k < 6; ++k )
        {
            for ( Int64 q = -5; q < 9; ++q )
            {
                ASSERT_TRUE( MATH::SinCosOfTurnFraction( q * ( sizes[ k ] / 4 ), sizes[ k ], s, c ) );
                const UInt32 index = (UInt32) ( ( ( q % 4 ) + 4 ) % 4 );
                ASSERT_TRUE( BitsOf( s ) == BitsOf( expectedSine[ index ] ) );
                ASSERT_TRUE( BitsOf( c ) == BitsOf( expectedCosine[ index ] ) );
            }
        }
        for ( Int64 e = -9; e < 17; e += 2 )
        {
            // an odd multiple of an eighth turn: the angle is 45 degrees up to the signs
            ASSERT_TRUE( MATH::SinCosOfTurnFraction( e, 8, s, c ) );
            ASSERT_TRUE( ( s < 0 ? -s : s ) == ( c < 0 ? -c : c ) );
            ASSERT_TRUE( fabs( fabs( s ) - 0.70710678118654757 ) < 1e-16 );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DeterministicTrigonometry: exact symmetries hold bit for bit (negation, period, scaling, complement, half turn)" )
    try
    {
        MATH::CPcg32 random( 77, 5 );
        for ( UInt32 i = 0; i < 4000; ++i )
        {
            const Int64 d = 1 + (Int64) ( random.NextUInt32() % 100000 );
            const Int64 n = (Int64) ( random.NextUInt32() % 300000 ) - 100000;
            Float64 s, c, s2, c2;
            MATH::SinCosOfTurnFraction( n, d, s, c );

            MATH::SinCosOfTurnFraction( -n, d, s2, c2 );                    // sin( -x ) = -sin( x ), cos( -x ) = cos( x )
            ASSERT_TRUE( BitsOf( s2 ) == BitsOf( 0.0 - s ) && BitsOf( c2 ) == BitsOf( c ) );
            MATH::SinCosOfTurnFraction( n, -d, s2, c2 );                    // the sign of the denominator is the sign of the angle
            ASSERT_TRUE( BitsOf( s2 ) == BitsOf( 0.0 - s ) && BitsOf( c2 ) == BitsOf( c ) );
            MATH::SinCosOfTurnFraction( n + 3 * d, d, s2, c2 );             // whole turns change nothing
            ASSERT_TRUE( BitsOf( s2 ) == BitsOf( s ) && BitsOf( c2 ) == BitsOf( c ) );
            MATH::SinCosOfTurnFraction( 7 * n, 7 * d, s2, c2 );             // the same fraction, not reduced
            ASSERT_TRUE( BitsOf( s2 ) == BitsOf( s ) && BitsOf( c2 ) == BitsOf( c ) );
            MATH::SinCosOfTurnFraction( d - 4 * n, 4 * d, s2, c2 );         // 1/4 turn minus x: sine and cosine swap
            ASSERT_TRUE( BitsOf( s2 ) == BitsOf( c ) && BitsOf( c2 ) == BitsOf( s ) );
            MATH::SinCosOfTurnFraction( d - 2 * n, 2 * d, s2, c2 );         // 1/2 turn minus x: the sine stays, the cosine flips
            ASSERT_TRUE( BitsOf( s2 ) == BitsOf( s ) && BitsOf( c2 ) == BitsOf( 0.0 - c ) );
            MATH::SinCosOfTurnFraction( d + 2 * n, 2 * d, s2, c2 );         // 1/2 turn plus x: both flip
            ASSERT_TRUE( BitsOf( s2 ) == BitsOf( 0.0 - s ) && BitsOf( c2 ) == BitsOf( 0.0 - c ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DeterministicTrigonometry: a zero denominator or one beyond 2^59 is refused with zero outputs, the extremes of the numerator work" )
    try
    {
        Float64 s = 3.0, c = 3.0;
        ASSERT_FALSE( MATH::SinCosOfTurnFraction( 1, 0, s, c ) );
        ASSERT_TRUE( 0.0 == s && 0.0 == c );
        s = 3.0; c = 3.0;
        ASSERT_FALSE( MATH::SinCosOfTurnFraction( 1, GUCEF_MATH_TURNFRACTION_MAX_DENOMINATOR + 1, s, c ) );
        ASSERT_TRUE( 0.0 == s && 0.0 == c );
        s = 3.0; c = 3.0;
        ASSERT_FALSE( MATH::SinCosOfTurnFraction( 1, -GUCEF_MATH_TURNFRACTION_MAX_DENOMINATOR - 1, s, c ) );
        ASSERT_TRUE( 0.0 == s && 0.0 == c );
        Float32 s32 = 3.0f, c32 = 3.0f;
        ASSERT_FALSE( MATH::SinCosOfTurnFraction( 1, 0, s32, c32 ) );
        ASSERT_TRUE( 0.0f == s32 && 0.0f == c32 );

        ASSERT_TRUE( MATH::SinCosOfTurnFraction( 1, GUCEF_MATH_TURNFRACTION_MAX_DENOMINATOR, s, c ) );
        ASSERT_TRUE( s > 0.0 && c == 1.0 );
        ASSERT_TRUE( MATH::SinCosOfTurnFraction( 1, -GUCEF_MATH_TURNFRACTION_MAX_DENOMINATOR, s, c ) );
        ASSERT_TRUE( s < 0.0 && c == 1.0 );
        // the most negative and the most positive numerator: finite results of unit length (pinned values cover their exact bits)
        ASSERT_TRUE( MATH::SinCosOfTurnFraction( (Int64) ( -9223372036854775807LL - 1 ), 3, s, c ) );
        ASSERT_TRUE( fabs( s * s + c * c - 1.0 ) < 1e-15 );
        ASSERT_TRUE( MATH::SinCosOfTurnFraction( (Int64) 9223372036854775807LL, 3, s, c ) );
        ASSERT_TRUE( fabs( s * s + c * c - 1.0 ) < 1e-15 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DeterministicTrigonometry: the Float32 form is the Float64 result rounded, bit for bit" )
    try
    {
        MATH::CPcg32 random( 5, 11 );
        for ( UInt32 i = 0; i < 20000; ++i )
        {
            const Int64 d = 1 + (Int64) ( random.NextUInt32() % 5000000 );
            const Int64 n = (Int64) random.NextUInt32() - (Int64) 2000000000;
            Float64 s, c;
            Float32 s32, c32;
            ASSERT_TRUE( MATH::SinCosOfTurnFraction( n, d, s, c ) );
            ASSERT_TRUE( MATH::SinCosOfTurnFraction( n, d, s32, c32 ) );
            ASSERT_TRUE( s32 == (Float32) s && c32 == (Float32) c );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DeterministicTrigonometry: the error against a long double reference is at most 0.51 ulp (correct rounding) over all j / N for many N and for random fractions" )
    GUCEF_TESTFW_SKIP_IF( !TRIGTEST_HAS_LONG_DOUBLE_REFERENCE, "long double has no 64 bit mantissa on this platform: the pinned values stand alone" );
    try
    {
        #if TRIGTEST_HAS_LONG_DOUBLE_REFERENCE
        long double worst = 0.0L;
        long double worst32 = 0.0L;
        UInt32 checked = 0;
        Int64 sizes[ 40 ];
        UInt32 sizeCount = 0;
        for ( UInt32 exponent = 1; exponent <= 13; ++exponent )
            sizes[ sizeCount++ ] = (Int64) 1 << exponent;
        const Int64 others[ 9 ] = { 3, 360, 1000, 4095, 10007, 3 << 10, 48000, 44100, 65537 };
        for ( UInt32 i = 0; i < 9; ++i )
            sizes[ sizeCount++ ] = others[ i ];

        for ( UInt32 k = 0; k < sizeCount; ++k )
        {
            for ( Int64 j = 0; j < sizes[ k ]; ++j )
            {
                Float64 s, c;
                Float32 s32, c32;
                MATH::SinCosOfTurnFraction( j, sizes[ k ], s, c );
                MATH::SinCosOfTurnFraction( j, sizes[ k ], s32, c32 );
                long double rs, rc;
                ReferenceSinCos( j, sizes[ k ], rs, rc );
                const long double es = UlpError( s, rs );
                const long double ec = UlpError( c, rc );
                if ( es > worst ) worst = es;
                if ( ec > worst ) worst = ec;
                const long double es32 = UlpErrorFloat32( s32, rs );
                const long double ec32 = UlpErrorFloat32( c32, rc );
                if ( es32 > worst32 ) worst32 = es32;
                if ( ec32 > worst32 ) worst32 = ec32;
                checked += 2;
            }
        }

        // random fractions, denominators of every size up to the limit
        MATH::CPcg32 random( 31337, 3 );
        for ( UInt32 i = 0; i < 30000; ++i )
        {
            const UInt32 bits = 1 + random.NextUInt32() % 59;
            UInt64 raw = ( (UInt64) random.NextUInt32() << 32 ) | (UInt64) random.NextUInt32();
            const Int64 d = 1 + (Int64) ( raw % ( (UInt64) 1 << bits ) );
            raw = ( (UInt64) random.NextUInt32() << 32 ) | (UInt64) random.NextUInt32();
            const Int64 n = (Int64) ( raw % ( (UInt64) 3 * (UInt64) d ) ) - d;
            Float64 s, c;
            MATH::SinCosOfTurnFraction( n, d, s, c );
            long double rs, rc;
            ReferenceSinCos( n, d, rs, rc );
            const long double es = UlpError( s, rs );
            const long double ec = UlpError( c, rc );
            if ( es > worst ) worst = es;
            if ( ec > worst ) worst = ec;
            checked += 2;
        }

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DeterministicTrigonometry: " << checked << " values checked, largest error " << (Float64) worst << " ulp of Float64, " << (Float64) worst32 << " ulp of Float32" );
        ASSERT_TRUE( worst <= 0.51L );
        ASSERT_TRUE( worst32 <= 0.5001L );
        #endif
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DeterministicTrigonometry: the hash over a sweep of 2^17 fractions equals the pinned value (the same on every platform)" )
    try
    {
        const UInt64 hash = ComputeSweepHash();
        if ( hash != PinnedSweepHash )
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DeterministicTrigonometry: the sweep hash is 0x" << HashToText( hash ) );
        ASSERT_TRUE( PinnedSweepHash == hash );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DeterministicTrigonometry: runs in a realtime context and does not allocate" )
    GUCEF_TESTFW_SKIP_IF( !MathAllocationCounter::IsAvailable(), "no allocation counter with a sanitizer that replaces the allocator (the RealtimeSanitizer build proves the same in a realtime context)" );
    try
    {
        MathAllocationCounter::StartCounting();
        MathAllocationCounter::AllocateProbe();
        ASSERT_TRUE( 1 == MathAllocationCounter::StopCounting() );
        Float64 sum = 0.0;
        MathAllocationCounter::StartCounting();
        for ( Int64 j = 0; j < 1000; ++j )
            sum += SinCosRealtime( j, 1000 );
        const UInt32 allocations = MathAllocationCounter::StopCounting();
        ASSERT_TRUE( 0 == allocations );
        ASSERT_TRUE( sum == sum );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DeterministicTrigonometry TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
