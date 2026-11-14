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

#ifndef GUCEF_MATH_BITFUNCTIONS_H
#include "gucefMATH_BitFunctions.h"
#define GUCEF_MATH_BITFUNCTIONS_H
#endif /* GUCEF_MATH_BITFUNCTIONS_H ? */

#ifndef GUCEF_MATH_CXOSHIRO256STARSTAR_H
#include "gucefMATH_CXoshiro256StarStar.h"
#define GUCEF_MATH_CXOSHIRO256STARSTAR_H
#endif /* GUCEF_MATH_CXOSHIRO256STARSTAR_H ? */

#include "TestBitFunctions.h"

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

static MATH::UInt32
SlowCountBits( MATH::UInt64 v )
{
    MATH::UInt32 n = 0;
    while ( v ) { n += (MATH::UInt32) ( v & 1 ); v >>= 1; }
    return n;
}

/*-------------------------------------------------------------------------*/

void
PerformBitFunctionsTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING BitFunctions TESTS" );

    GUCEF_TESTFW_TESTCASE( "BitFunctions: powers of two" )
    {
        ASSERT_FALSE( MATH::IsPowerOfTwo( (MATH::UInt32) 0 ) );
        ASSERT_FALSE( MATH::IsPowerOfTwo( (MATH::UInt64) 0 ) );
        ASSERT_TRUE( MATH::IsPowerOfTwo( (MATH::UInt32) 1 ) );
        ASSERT_TRUE( MATH::IsPowerOfTwo( (MATH::UInt32) 0x80000000U ) );
        ASSERT_FALSE( MATH::IsPowerOfTwo( (MATH::UInt32) 6 ) );
        ASSERT_TRUE( MATH::IsPowerOfTwo( (MATH::UInt64) 0x8000000000000000ULL ) );
        ASSERT_FALSE( MATH::IsPowerOfTwo( (MATH::UInt64) 0x8000000000000001ULL ) );

        ASSERT_TRUE( MATH::NextPowerOfTwo( (MATH::UInt32) 0 ) == 1 );
        ASSERT_TRUE( MATH::NextPowerOfTwo( (MATH::UInt32) 1 ) == 1 );
        ASSERT_TRUE( MATH::NextPowerOfTwo( (MATH::UInt32) 5 ) == 8 );
        ASSERT_TRUE( MATH::NextPowerOfTwo( (MATH::UInt32) 8 ) == 8 );
        ASSERT_TRUE( MATH::NextPowerOfTwo( (MATH::UInt32) 0x80000000U ) == 0x80000000U );
        ASSERT_TRUE( MATH::NextPowerOfTwo( (MATH::UInt32) 0x80000001U ) == 0 );
        ASSERT_TRUE( MATH::NextPowerOfTwo( (MATH::UInt64) 0 ) == 1 );
        ASSERT_TRUE( MATH::NextPowerOfTwo( (MATH::UInt64) 0x80000001ULL ) == 0x100000000ULL );
        ASSERT_TRUE( MATH::NextPowerOfTwo( (MATH::UInt64) 0x8000000000000000ULL ) == 0x8000000000000000ULL );
        ASSERT_TRUE( MATH::NextPowerOfTwo( (MATH::UInt64) 0x8000000000000001ULL ) == 0 );

        ASSERT_TRUE( MATH::PreviousPowerOfTwo( (MATH::UInt32) 0 ) == 0 );
        ASSERT_TRUE( MATH::PreviousPowerOfTwo( (MATH::UInt32) 5 ) == 4 );
        ASSERT_TRUE( MATH::PreviousPowerOfTwo( (MATH::UInt32) 8 ) == 8 );
        ASSERT_TRUE( MATH::PreviousPowerOfTwo( (MATH::UInt32) 0xFFFFFFFFU ) == 0x80000000U );
        ASSERT_TRUE( MATH::PreviousPowerOfTwo( (MATH::UInt64) 0xFFFFFFFFFFFFFFFFULL ) == 0x8000000000000000ULL );

        // For every value of a range: the next power is the smallest power >= value
        for ( MATH::UInt32 v = 1; v < 5000; ++v )
        {
            MATH::UInt32 expected = 1;
            while ( expected < v ) expected <<= 1;
            ASSERT_TRUE( MATH::NextPowerOfTwo( v ) == expected );
            ASSERT_TRUE( MATH::NextPowerOfTwo( (MATH::UInt64) v ) == expected );
            ASSERT_TRUE( MATH::PreviousPowerOfTwo( v ) == ( MATH::IsPowerOfTwo( v ) ? v : expected / 2 ) );
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BitFunctions: logarithms" )
    {
        ASSERT_TRUE( MATH::Log2Floor( (MATH::UInt32) 0 ) == 0 );
        ASSERT_TRUE( MATH::Log2Ceil( (MATH::UInt32) 0 ) == 0 );
        ASSERT_TRUE( MATH::Log2Floor( (MATH::UInt32) 1 ) == 0 );
        ASSERT_TRUE( MATH::Log2Ceil( (MATH::UInt32) 1 ) == 0 );
        ASSERT_TRUE( MATH::Log2Floor( (MATH::UInt32) 2 ) == 1 );
        ASSERT_TRUE( MATH::Log2Ceil( (MATH::UInt32) 3 ) == 2 );
        ASSERT_TRUE( MATH::Log2Floor( (MATH::UInt32) 0xFFFFFFFFU ) == 31 );
        ASSERT_TRUE( MATH::Log2Ceil( (MATH::UInt32) 0xFFFFFFFFU ) == 32 );
        ASSERT_TRUE( MATH::Log2Floor( (MATH::UInt64) 1 ) == 0 );
        ASSERT_TRUE( MATH::Log2Floor( (MATH::UInt64) ( 1ULL << 40 ) ) == 40 );
        ASSERT_TRUE( MATH::Log2Ceil( (MATH::UInt64) ( 1ULL << 40 ) ) == 40 );
        ASSERT_TRUE( MATH::Log2Ceil( (MATH::UInt64) ( ( 1ULL << 40 ) + 1 ) ) == 41 );
        ASSERT_TRUE( MATH::Log2Floor( (MATH::UInt64) 0xFFFFFFFFFFFFFFFFULL ) == 63 );
        ASSERT_TRUE( MATH::Log2Ceil( (MATH::UInt64) 0xFFFFFFFFFFFFFFFFULL ) == 64 );
        for ( MATH::UInt32 shift = 0; shift < 32; ++shift )
        {
            ASSERT_TRUE( MATH::Log2Floor( (MATH::UInt32) ( 1U << shift ) ) == shift );
            ASSERT_TRUE( MATH::Log2Ceil( (MATH::UInt32) ( 1U << shift ) ) == shift );
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BitFunctions: bit counting against a loop, random values" )
    {
        MATH::CXoshiro256StarStar generator( 4242 );
        ASSERT_TRUE( MATH::CountBits( (MATH::UInt32) 0 ) == 0 );
        ASSERT_TRUE( MATH::CountBits( (MATH::UInt32) 0xFFFFFFFFU ) == 32 );
        ASSERT_TRUE( MATH::CountBits( (MATH::UInt64) 0xFFFFFFFFFFFFFFFFULL ) == 64 );
        ASSERT_TRUE( MATH::CountLeadingZeros( (MATH::UInt32) 0 ) == 32 );
        ASSERT_TRUE( MATH::CountLeadingZeros( (MATH::UInt64) 0 ) == 64 );
        ASSERT_TRUE( MATH::CountTrailingZeros( (MATH::UInt32) 0 ) == 32 );
        ASSERT_TRUE( MATH::CountTrailingZeros( (MATH::UInt64) 0 ) == 64 );
        ASSERT_TRUE( MATH::CountLeadingZeros( (MATH::UInt32) 1 ) == 31 );
        ASSERT_TRUE( MATH::CountTrailingZeros( (MATH::UInt32) 0x80000000U ) == 31 );
        ASSERT_TRUE( MATH::CountLeadingZeros( (MATH::UInt64) 1 ) == 63 );
        ASSERT_TRUE( MATH::CountTrailingZeros( (MATH::UInt64) 0x8000000000000000ULL ) == 63 );

        for ( int i = 0; i < 2000; ++i )
        {
            MATH::UInt64 v = generator.NextUInt64() >> ( generator.NextUInt32() % 64 );
            const MATH::UInt32 v32 = (MATH::UInt32) v;
            MATH::UInt32 lz = 0;
            while ( lz < 64 && !( ( v >> ( 63 - lz ) ) & 1ULL ) ) { ++lz; if ( lz == 64 ) break; }
            MATH::UInt32 tz = 0;
            while ( tz < 64 && !( ( v >> tz ) & 1ULL ) ) ++tz;
            if ( 0 == v ) { lz = 64; tz = 64; }
            MATH::UInt32 lz32 = 0;
            while ( lz32 < 32 && !( ( v32 >> ( 31 - lz32 ) ) & 1U ) ) ++lz32;
            MATH::UInt32 tz32 = 0;
            while ( tz32 < 32 && !( ( v32 >> tz32 ) & 1U ) ) ++tz32;
            const bool ok = MATH::CountBits( v ) == SlowCountBits( v ) && MATH::CountBits( v32 ) == SlowCountBits( v32 ) &&
                            MATH::CountLeadingZeros( v ) == lz && MATH::CountTrailingZeros( v ) == tz &&
                            MATH::CountLeadingZeros( v32 ) == lz32 && MATH::CountTrailingZeros( v32 ) == tz32;
            if ( !ok )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "BitFunctions: counting differs for " << v );
            ASSERT_TRUE( ok );
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BitFunctions: rotation, reversal and alignment" )
    {
        ASSERT_TRUE( MATH::RotateLeft( (MATH::UInt32) 0x80000001U, 1 ) == 0x00000003U );
        ASSERT_TRUE( MATH::RotateRight( (MATH::UInt32) 0x80000001U, 1 ) == 0xC0000000U );
        ASSERT_TRUE( MATH::RotateLeft( (MATH::UInt32) 0x12345678U, 0 ) == 0x12345678U );
        ASSERT_TRUE( MATH::RotateLeft( (MATH::UInt32) 0x12345678U, 32 ) == 0x12345678U );
        ASSERT_TRUE( MATH::RotateLeft( (MATH::UInt32) 0x12345678U, 36 ) == MATH::RotateLeft( (MATH::UInt32) 0x12345678U, 4 ) );
        ASSERT_TRUE( MATH::RotateLeft( (MATH::UInt64) 0x8000000000000001ULL, 1 ) == 3ULL );
        ASSERT_TRUE( MATH::RotateRight( (MATH::UInt64) 0x8000000000000001ULL, 1 ) == 0xC000000000000000ULL );
        ASSERT_TRUE( MATH::RotateRight( (MATH::UInt64) 0x0123456789ABCDEFULL, 64 ) == 0x0123456789ABCDEFULL );
        ASSERT_TRUE( MATH::RotateLeft( (MATH::UInt32) 0xDEADBEEFU, 13 ) == MATH::RotateRight( (MATH::UInt32) 0xDEADBEEFU, 19 ) );
        ASSERT_TRUE( MATH::ReverseBits( (MATH::UInt32) 1 ) == 0x80000000U );
        ASSERT_TRUE( MATH::ReverseBits( (MATH::UInt32) 0x12345678U ) == 0x1E6A2C48U );
        ASSERT_TRUE( MATH::ReverseBits( (MATH::UInt64) 1 ) == 0x8000000000000000ULL );
        ASSERT_TRUE( MATH::ReverseBits( MATH::ReverseBits( (MATH::UInt64) 0x0123456789ABCDEFULL ) ) == 0x0123456789ABCDEFULL );
        ASSERT_TRUE( MATH::ReverseBits( (MATH::UInt64) 0x0123456789ABCDEFULL ) == 0xF7B3D591E6A2C480ULL );

        ASSERT_TRUE( MATH::AlignUp( (MATH::UInt32) 0, 16 ) == 0 );
        ASSERT_TRUE( MATH::AlignUp( (MATH::UInt32) 1, 16 ) == 16 );
        ASSERT_TRUE( MATH::AlignUp( (MATH::UInt32) 16, 16 ) == 16 );
        ASSERT_TRUE( MATH::AlignUp( (MATH::UInt32) 17, 16 ) == 32 );
        ASSERT_TRUE( MATH::AlignDown( (MATH::UInt32) 31, 16 ) == 16 );
        ASSERT_TRUE( MATH::AlignDown( (MATH::UInt32) 16, 16 ) == 16 );
        ASSERT_TRUE( MATH::AlignUp( (MATH::UInt64) 0x100000001ULL, 0x100000000ULL ) == 0x200000000ULL );
        ASSERT_TRUE( MATH::AlignDown( (MATH::UInt64) 0x1FFFFFFFFULL, 0x100000000ULL ) == 0x100000000ULL );
        ASSERT_TRUE( MATH::AlignUp( (MATH::UInt32) 5, 1 ) == 5 );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BitFunctions: Morton codes" )
    {
        ASSERT_TRUE( MATH::MortonEncode2D( 3, 0 ) == 5 );
        ASSERT_TRUE( MATH::MortonEncode2D( 0, 3 ) == 10 );
        ASSERT_TRUE( MATH::MortonEncode2D( 1, 1 ) == 3 );
        ASSERT_TRUE( MATH::MortonEncode2D( 0xFFFFFFFFU, 0xFFFFFFFFU ) == 0xFFFFFFFFFFFFFFFFULL );
        ASSERT_TRUE( MATH::MortonEncode3D( 1, 1, 1 ) == 7 );
        ASSERT_TRUE( MATH::MortonEncode3D( 1, 0, 0 ) == 1 );
        ASSERT_TRUE( MATH::MortonEncode3D( 0, 1, 0 ) == 2 );
        ASSERT_TRUE( MATH::MortonEncode3D( 0, 0, 1 ) == 4 );
        ASSERT_TRUE( MATH::MortonEncode3D( 0x1FFFFFU, 0x1FFFFFU, 0x1FFFFFU ) == 0x7FFFFFFFFFFFFFFFULL );

        // Round trip over a full 256 x 256 domain
        bool ok2D = true;
        for ( MATH::UInt32 y = 0; y < 256; ++y )
        {
            for ( MATH::UInt32 x = 0; x < 256; ++x )
            {
                MATH::UInt32 dx = 0, dy = 0;
                MATH::MortonDecode2D( MATH::MortonEncode2D( x, y ), dx, dy );
                if ( dx != x || dy != y ) ok2D = false;
            }
        }
        ASSERT_TRUE( ok2D );

        // Round trip over a full 64 x 64 x 64 domain
        bool ok3D = true;
        for ( MATH::UInt32 z = 0; z < 64; ++z )
        {
            for ( MATH::UInt32 y = 0; y < 64; ++y )
            {
                for ( MATH::UInt32 x = 0; x < 64; ++x )
                {
                    MATH::UInt32 dx = 0, dy = 0, dz = 0;
                    MATH::MortonDecode3D( MATH::MortonEncode3D( x, y, z ), dx, dy, dz );
                    if ( dx != x || dy != y || dz != z ) ok3D = false;
                }
            }
        }
        ASSERT_TRUE( ok3D );

        // Random full range values
        MATH::CXoshiro256StarStar generator( 31337 );
        for ( int i = 0; i < 1000; ++i )
        {
            const MATH::UInt32 x = generator.NextUInt32();
            const MATH::UInt32 y = generator.NextUInt32();
            MATH::UInt32 dx = 0, dy = 0;
            MATH::MortonDecode2D( MATH::MortonEncode2D( x, y ), dx, dy );
            ASSERT_TRUE( dx == x && dy == y );
            const MATH::UInt32 a = x & 0x1FFFFFU, b = y & 0x1FFFFFU, c = generator.NextUInt32() & 0x1FFFFFU;
            MATH::UInt32 da = 0, db = 0, dc = 0;
            MATH::MortonDecode3D( MATH::MortonEncode3D( a, b, c ), da, db, dc );
            ASSERT_TRUE( da == a && db == b && dc == c );
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BitFunctions: Hilbert curve, order 8: round trip, bijection, neighbouring cells" )
    {
        const MATH::UInt32 order = 8;
        const MATH::UInt32 size = 1U << order;
        bool roundTrip = true;
        bool bijection = true;
        bool neighbours = true;
        MATH::UInt32 previousX = 0, previousY = 0;
        MATH::HilbertDecode2D( order, 0, previousX, previousY );
        ASSERT_TRUE( 0 == previousX && 0 == previousY );    // the curve starts in the corner
        for ( MATH::UInt64 d = 0; d < (MATH::UInt64) size * size; ++d )
        {
            MATH::UInt32 x = 0, y = 0;
            MATH::HilbertDecode2D( order, d, x, y );
            if ( x >= size || y >= size ) { bijection = false; continue; }
            if ( MATH::HilbertEncode2D( order, x, y ) != d ) roundTrip = false;
            if ( d > 0 )
            {
                const MATH::UInt32 dx = x > previousX ? x - previousX : previousX - x;
                const MATH::UInt32 dy = y > previousY ? y - previousY : previousY - y;
                if ( dx + dy != 1 ) neighbours = false;
            }
            previousX = x;
            previousY = y;
        }
        ASSERT_TRUE( roundTrip );
        ASSERT_TRUE( bijection );
        ASSERT_TRUE( neighbours );

        // Every cell is visited exactly once: the encoding of all cells is a permutation of 0 .. size^2 - 1
        bool seen[ 256 * 256 ];
        for ( MATH::UInt32 i = 0; i < 256 * 256; ++i ) seen[ i ] = false;
        bool permutation = true;
        for ( MATH::UInt32 y = 0; y < size; ++y )
        {
            for ( MATH::UInt32 x = 0; x < size; ++x )
            {
                const MATH::UInt64 d = MATH::HilbertEncode2D( order, x, y );
                if ( d >= (MATH::UInt64) size * size || seen[ d ] ) permutation = false; else seen[ d ] = true;
            }
        }
        ASSERT_TRUE( permutation );

        // Low orders against the published curve of order 1 and 2
        ASSERT_TRUE( MATH::HilbertEncode2D( 1, 0, 0 ) == 0 );
        ASSERT_TRUE( MATH::HilbertEncode2D( 1, 0, 1 ) == 1 );
        ASSERT_TRUE( MATH::HilbertEncode2D( 1, 1, 1 ) == 2 );
        ASSERT_TRUE( MATH::HilbertEncode2D( 1, 1, 0 ) == 3 );
        ASSERT_TRUE( MATH::HilbertEncode2D( 0, 0, 0 ) == 0 );

        // Order 32 uses the full 64 bits
        MATH::UInt32 x = 0, y = 0;
        MATH::HilbertDecode2D( 32, MATH::HilbertEncode2D( 32, 0xDEADBEEFU, 0x12345678U ), x, y );
        ASSERT_TRUE( 0xDEADBEEFU == x && 0x12345678U == y );
        MATH::HilbertDecode2D( 32, MATH::HilbertEncode2D( 32, 0xFFFFFFFFU, 0 ), x, y );
        ASSERT_TRUE( 0xFFFFFFFFU == x && 0 == y );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "BitFunctions: hashes" )
    {
        // Known outputs of the published finalisers
        ASSERT_TRUE( MATH::HashUInt32( 0 ) == 0 );
        ASSERT_TRUE( MATH::HashUInt64( 0 ) == 0 );
        // The SplitMix64 output for state s is HashUInt64( s + increment ), so the first output for seed 0 pins the finaliser
        ASSERT_TRUE( MATH::HashUInt64( 0x9E3779B97F4A7C15ULL ) == 0xe220a8397b1dcdafULL );
        // Distinct inputs map to distinct outputs on a sample, and bits avalanche
        bool distinct = true;
        MATH::UInt32 previous = MATH::HashUInt32( 0 );
        for ( MATH::UInt32 i = 1; i < 10000; ++i )
        {
            const MATH::UInt32 h = MATH::HashUInt32( i );
            if ( h == previous ) distinct = false;
            previous = h;
        }
        ASSERT_TRUE( distinct );
        MATH::UInt32 flipped = 0;
        for ( MATH::UInt32 i = 1; i <= 1000; ++i )
            flipped += MATH::CountBits( (MATH::UInt32) ( MATH::HashUInt32( i ) ^ MATH::HashUInt32( i ^ 1U ) ) );
        const MATH::Float64 averageFlips = (MATH::Float64) flipped / 1000.0;
        ASSERT_TRUE( averageFlips > 14.0 && averageFlips < 18.0 );

        ASSERT_TRUE( MATH::HashCombine( 1, 2 ) != MATH::HashCombine( 2, 1 ) );
        ASSERT_TRUE( MATH::HashCombine( 1, 2 ) == MATH::HashCombine( 1, 2 ) );
        ASSERT_TRUE( MATH::HashCombine( MATH::HashCombine( 0, 1 ), 2 ) != MATH::HashCombine( MATH::HashCombine( 0, 2 ), 1 ) );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "BitFunctions TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
