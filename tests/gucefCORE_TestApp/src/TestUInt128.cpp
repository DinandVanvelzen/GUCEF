/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

/* the helper is internal to gucefCORE: not exported, header only, so the test compiles the same code */
#ifndef GUCEF_CORE_CUINT128_H
#include "../../../platform/gucefCORE/src/gucefCORE_CUInt128.h"
#define GUCEF_CORE_CUINT128_H
#endif /* GUCEF_CORE_CUINT128_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTDECIMALREFERENCE_H
#include "TestDecimalReference.h"
#define GUCEF_CORE_TESTAPP_TESTDECIMALREFERENCE_H
#endif /* GUCEF_CORE_TESTAPP_TESTDECIMALREFERENCE_H ? */

#include "TestUInt128.h"

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
using namespace TestDecimalReference;
typedef CORE::CUInt128 CUInt128;

/*-------------------------------------------------------------------------*/

static const UInt64 g_testSeedUInt128 = 0x5EED0128DEC1A1ULL;

/*-------------------------------------------------------------------------*/

static CRefInt
RefOf( const CUInt128& value )
{
    return CRefInt::FromWords( value.high, value.low );
}

/*-------------------------------------------------------------------------*/

static bool
SameAs( const CUInt128& value, const CRefInt& reference )
{
    UInt64 high = 0;
    UInt64 low = 0;
    if ( !reference.ToWords( high, low ) )
        return false;
    return value.high == high && value.low == low;
}

/*-------------------------------------------------------------------------*/

#ifdef __SIZEOF_INT128__
typedef unsigned __int128 TCompilerUInt128;

static TCompilerUInt128
Native( const CUInt128& value )
{
    return ( ( (TCompilerUInt128) value.high ) << 64 ) | value.low;
}
#endif

/*-------------------------------------------------------------------------*/

/** Random 64 bit value with a random number of significant bits so small, large and boundary magnitudes all occur */
static UInt64
RandomWord( CSplitMix64& rng )
{
    UInt32 bits = (UInt32) rng.Below( 65 );
    if ( 0 == bits )
        return 0;
    UInt64 value = rng.Next();
    if ( bits < 64 )
        value &= ( 1ULL << bits ) - 1;
    if ( 0 == rng.Below( 8 ) )
        value |= 1ULL << ( bits - 1 );     // top bit set: normalized divisors
    return value;
}

/*-------------------------------------------------------------------------*/

/** Checks one division against the identity, the reference and where available the compiler type */
static bool
CheckDivMod( const CUInt128& n, UInt64 d, bool useReference )
{
    CUInt128 q;
    UInt64 r = 0;
    CUInt128::DivMod( n, d, q, r );
    if ( r >= d )
        return false;

    // q * d + r == n, computed with the checked multiply and the add
    CUInt128 product;
    if ( !CUInt128::MultiplyChecked( q, d, product ) )
        return false;
    CUInt128 sum;
    if ( !CUInt128::AddChecked( product, CUInt128( r ), sum ) || 0 != CUInt128::Compare( sum, n ) )
        return false;

    #ifdef __SIZEOF_INT128__
    if ( Native( q ) != Native( n ) / d || (TCompilerUInt128) r != Native( n ) % d )
        return false;
    #endif

    if ( useReference )
    {
        CRefInt refQ, refR;
        CRefInt::DivMod( RefOf( n ), CRefInt::FromUInt64( d ), refQ, refR );
        if ( !SameAs( q, refQ ) || !SameAs( CUInt128( r ), refR ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
PerformUInt128Tests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CUInt128 TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "CUInt128" );

    GUCEF_TESTFW_TESTCASE( "Test: Multiply on boundary operands matches the reference" )
        try
        {
            const UInt64 operands[] = { 0ULL, 1ULL, 2ULL, 0xFFFFFFFFULL, 0x100000000ULL, 0x8000000000000000ULL,
                                        0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFF00000000ULL, 0x00000000FFFFFFFFULL, 10000000000000000000ULL };
            const UInt32 count = (UInt32) ( sizeof( operands ) / sizeof( operands[ 0 ] ) );
            for ( UInt32 i=0; i<count; ++i )
            {
                for ( UInt32 j=0; j<count; ++j )
                {
                    CUInt128 product = CUInt128::Multiply( operands[ i ], operands[ j ] );
                    ASSERT_TRUE( SameAs( product, CRefInt::Multiply( CRefInt::FromUInt64( operands[ i ] ), CRefInt::FromUInt64( operands[ j ] ) ) ) );
                }
            }

            // (2^64 - 1)^2 = 2^128 - 2^65 + 1
            CUInt128 square = CUInt128::Multiply( 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL );
            ASSERT_TRUE( 0xFFFFFFFFFFFFFFFEULL == square.high && 1ULL == square.low );

            CUInt128 p = CUInt128::Multiply( 0x100000000ULL, 0x100000000ULL );
            ASSERT_TRUE( 1ULL == p.high && 0ULL == p.low );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: MultiplyChecked overflows exactly at 2^128" )
        try
        {
            CUInt128 out;
            // 2^127 * 2 = 2^128 does not fit
            ASSERT_FALSE( CUInt128::MultiplyChecked( CUInt128( 0x8000000000000000ULL, 0 ), 2, out ) );
            // (2^127 - 1) * 2 = 2^128 - 2 fits
            ASSERT_TRUE( CUInt128::MultiplyChecked( CUInt128( 0x7FFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL ), 2, out ) );
            ASSERT_TRUE( 0xFFFFFFFFFFFFFFFFULL == out.high && 0xFFFFFFFFFFFFFFFEULL == out.low );
            // (2^128 - 1) * 1 fits, * 0 is 0
            ASSERT_TRUE( CUInt128::MultiplyChecked( CUInt128( ~0ULL, ~0ULL ), 1, out ) && ~0ULL == out.high && ~0ULL == out.low );
            ASSERT_TRUE( CUInt128::MultiplyChecked( CUInt128( ~0ULL, ~0ULL ), 0, out ) && out.IsZero() );
            // 2^64 * (2^64 - 1) = 2^128 - 2^64 fits, 2^64 * 2^64 is out of reach of a 64 bit factor anyway
            ASSERT_TRUE( CUInt128::MultiplyChecked( CUInt128( 1, 0 ), ~0ULL, out ) && ~0ULL == out.high && 0 == out.low );
            // a carry out of the low product into an all ones high word overflows
            ASSERT_FALSE( CUInt128::MultiplyChecked( CUInt128( 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL ), 2, out ) );
            ASSERT_TRUE( CUInt128::MultiplyChecked( CUInt128( 0, ~0ULL ), ~0ULL, out ) );     // (2^64-1)^2 fits

            // 10^38 fits, 10^39 does not
            CUInt128 power( 1 );
            ASSERT_TRUE( CUInt128::MultiplyByPow10Checked( power, 38 ) );
            ASSERT_TRUE( SameAs( power, CRefInt::PowerOfTen( 38 ) ) );
            CUInt128 tooBig( 1 );
            ASSERT_FALSE( CUInt128::MultiplyByPow10Checked( tooBig, 39 ) );
            CUInt128 zero;
            ASSERT_TRUE( CUInt128::MultiplyByPow10Checked( zero, 1000 ) && zero.IsZero() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: DivMod by boundary divisors with the high word below and at or above the divisor" )
        try
        {
            GUCEF::vector< UInt64 > divisors;
            divisors.push_back( 1 );
            divisors.push_back( 2 );
            divisors.push_back( 3 );
            divisors.push_back( 7 );
            for ( UInt32 k=1; k<=19; ++k )
                divisors.push_back( CUInt128::Pow10( k ) );
            divisors.push_back( 0x8000000000000000ULL );
            divisors.push_back( 0xFFFFFFFFFFFFFFFFULL );
            divisors.push_back( 0xFFFFFFFFULL );
            divisors.push_back( 0x100000000ULL );
            divisors.push_back( 0x100000001ULL );
            // top 32 bit digit of the divisor just above half: the worst cases for the digit estimates
            divisors.push_back( 0x8000000000000001ULL );
            divisors.push_back( 0x80000000FFFFFFFFULL );
            divisors.push_back( 0x8000000100000000ULL );
            divisors.push_back( 0x80000001FFFFFFFFULL );
            divisors.push_back( 0x7FFFFFFFFFFFFFFFULL );
            divisors.push_back( 0x0000000180000000ULL );

            bool allOk = true;
            for ( size_t i=0; i<divisors.size(); ++i )
            {
                UInt64 d = divisors[ i ];
                GUCEF::vector< CUInt128 > dividends;
                dividends.push_back( CUInt128() );
                dividends.push_back( CUInt128( 0, d - 1 ) );
                dividends.push_back( CUInt128( 0, d ) );
                dividends.push_back( CUInt128( 0, ~0ULL ) );
                dividends.push_back( CUInt128( d - 1, ~0ULL ) );        // high word below the divisor, the largest quotient
                dividends.push_back( CUInt128( d - 1, 0 ) );
                dividends.push_back( CUInt128( d, 0 ) );                // high word equal to the divisor
                dividends.push_back( CUInt128( ~0ULL, ~0ULL ) );        // high word above the divisor
                dividends.push_back( CUInt128( ~0ULL, 0 ) );
                dividends.push_back( CUInt128( 0x8000000000000000ULL, 1 ) );
                dividends.push_back( CUInt128( d >> 1, 0x8000000000000000ULL ) );
                for ( size_t j=0; j<dividends.size(); ++j )
                {
                    if ( !CheckDivMod( dividends[ j ], d, true ) )
                    {
                        allOk = false;
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DivMod mismatch for divisor " << d << " dividend " << dividends[ j ].high << ":" << dividends[ j ].low );
                    }
                }
            }
            ASSERT_TRUE( allOk );

            // 10^38 / 10^19 = 10^19
            CUInt128 n( 1 );
            CUInt128::MultiplyByPow10Checked( n, 38 );
            CUInt128 q;
            UInt64 r = 1;
            CUInt128::DivMod( n, CUInt128::Pow10( 19 ), q, r );
            ASSERT_TRUE( 0 == r && SameAs( q, CRefInt::PowerOfTen( 19 ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: DivMod random properties q * d + r == n and r < d" )
        try
        {
            CSplitMix64 rng( g_testSeedUInt128 );
            UInt32 failures = 0;
            const UInt32 iterations = 100000;
            for ( UInt32 i=0; i<iterations && 0 == failures; ++i )
            {
                CUInt128 n( RandomWord( rng ), RandomWord( rng ) );
                UInt64 d = RandomWord( rng );
                if ( 0 == d )
                    d = 1;
                // the reference is slow: every 50th case is also checked against it
                if ( !CheckDivMod( n, d, 0 == i % 50 ) )
                {
                    ++failures;
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DivMod property failed at iteration " << i << " seed " << g_testSeedUInt128 );
                }
            }
            ASSERT_TRUE( 0 == failures );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: random products, sums and differences match the reference" )
        try
        {
            CSplitMix64 rng( g_testSeedUInt128 + 1 );
            UInt32 failures = 0;
            for ( UInt32 i=0; i<5000 && 0 == failures; ++i )
            {
                UInt64 a = RandomWord( rng );
                UInt64 b = RandomWord( rng );
                CUInt128 product = CUInt128::Multiply( a, b );
                CRefInt refProduct = CRefInt::Multiply( CRefInt::FromUInt64( a ), CRefInt::FromUInt64( b ) );
                if ( !SameAs( product, refProduct ) )
                    ++failures;

                CUInt128 x( RandomWord( rng ) >> 1, RandomWord( rng ) );
                CUInt128 y( RandomWord( rng ) >> 1, RandomWord( rng ) );
                CUInt128 sum;
                if ( !CUInt128::AddChecked( x, y, sum ) || !SameAs( sum, CRefInt::Add( RefOf( x ), RefOf( y ) ) ) )
                    ++failures;
                const CUInt128& big = CUInt128::Compare( x, y ) >= 0 ? x : y;
                const CUInt128& small = CUInt128::Compare( x, y ) >= 0 ? y : x;
                if ( !SameAs( CUInt128::Subtract( big, small ), CRefInt::Subtract( RefOf( big ), RefOf( small ) ) ) )
                    ++failures;
                Int32 cmp = CUInt128::Compare( x, y );
                if ( cmp != CRefInt::Compare( RefOf( x ), RefOf( y ) ) )
                    ++failures;

                UInt64 m = RandomWord( rng );
                CUInt128 scaled;
                bool fits = CUInt128::MultiplyChecked( x, m, scaled );
                CRefInt refScaled = CRefInt::Multiply( RefOf( x ), CRefInt::FromUInt64( m ) );
                UInt64 h = 0, l = 0;
                if ( fits != refScaled.ToWords( h, l ) || ( fits && !SameAs( scaled, refScaled ) ) )
                    ++failures;

                #ifdef __SIZEOF_INT128__
                if ( Native( product ) != (TCompilerUInt128) a * b )
                    ++failures;
                #endif
            }
            if ( 0 != failures )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CUInt128 arithmetic mismatch, seed " << ( g_testSeedUInt128 + 1 ) );
            ASSERT_TRUE( 0 == failures );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: AddChecked, wrapping arithmetic, negation and comparison" )
        try
        {
            CUInt128 out;
            ASSERT_FALSE( CUInt128::AddChecked( CUInt128( ~0ULL, ~0ULL ), CUInt128( 1 ), out ) );
            ASSERT_TRUE( out.IsZero() );
            ASSERT_TRUE( CUInt128::AddChecked( CUInt128( 0, ~0ULL ), CUInt128( 1 ), out ) && 1 == out.high && 0 == out.low );
            ASSERT_FALSE( CUInt128::AddChecked( CUInt128( ~0ULL, 0 ), CUInt128( 1, 0 ), out ) );
            ASSERT_FALSE( CUInt128::AddChecked( CUInt128( ~0ULL, ~0ULL ), CUInt128( 0, ~0ULL ), out ) );

            CUInt128 w = CUInt128::Subtract( CUInt128(), CUInt128( 1 ) );
            ASSERT_TRUE( ~0ULL == w.high && ~0ULL == w.low );
            CUInt128 borrow = CUInt128::Subtract( CUInt128( 1, 0 ), CUInt128( 1 ) );
            ASSERT_TRUE( 0 == borrow.high && ~0ULL == borrow.low );
            CUInt128 neg = CUInt128::Negate( CUInt128( 5 ) );
            ASSERT_TRUE( CUInt128::Add( neg, CUInt128( 5 ) ).IsZero() );
            ASSERT_TRUE( CUInt128::Negate( CUInt128() ).IsZero() );

            ASSERT_TRUE( 0 == CUInt128::Compare( CUInt128( 3, 4 ), CUInt128( 3, 4 ) ) );
            ASSERT_TRUE( -1 == CUInt128::Compare( CUInt128( 3, 4 ), CUInt128( 3, 5 ) ) );
            ASSERT_TRUE( 1 == CUInt128::Compare( CUInt128( 4, 0 ), CUInt128( 3, ~0ULL ) ) );
            ASSERT_TRUE( CUInt128( 0, 3 ).IsOdd() && !CUInt128( 1, 2 ).IsOdd() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: shifts, leading zero count and bit length" )
        try
        {
            ASSERT_TRUE( 64 == CUInt128::CountLeadingZeros( 0ULL ) );
            ASSERT_TRUE( 63 == CUInt128::CountLeadingZeros( 1ULL ) );
            ASSERT_TRUE( 0 == CUInt128::CountLeadingZeros( 0x8000000000000000ULL ) );
            ASSERT_TRUE( 32 == CUInt128::CountLeadingZeros( 0xFFFFFFFFULL ) );
            ASSERT_TRUE( 31 == CUInt128::CountLeadingZeros( 0x100000000ULL ) );
            for ( UInt32 bit=0; bit<64; ++bit )
                ASSERT_TRUE( 63 - bit == CUInt128::CountLeadingZeros( 1ULL << bit ) );

            ASSERT_TRUE( 128 == CUInt128().CountLeadingZeros() );
            ASSERT_TRUE( 0 == CUInt128().BitLength() );
            ASSERT_TRUE( 65 == CUInt128( 1, 0 ).BitLength() );
            ASSERT_TRUE( 128 == CUInt128( 0x8000000000000000ULL, 0 ).BitLength() );

            CUInt128 one( 1 );
            for ( UInt32 s=0; s<128; ++s )
            {
                CUInt128 shifted = CUInt128::ShiftLeft( one, s );
                ASSERT_TRUE( s + 1 == shifted.BitLength() );
                ASSERT_TRUE( 0 == CUInt128::Compare( CUInt128::ShiftRight( shifted, s ), one ) );
            }
            ASSERT_TRUE( CUInt128::ShiftLeft( one, 128 ).IsZero() );
            ASSERT_TRUE( CUInt128::ShiftRight( CUInt128( ~0ULL, ~0ULL ), 128 ).IsZero() );
            CUInt128 mixed = CUInt128::ShiftLeft( CUInt128( 0x0123456789ABCDEFULL, 0xFEDCBA9876543210ULL ), 4 );
            ASSERT_TRUE( 0x123456789ABCDEFFULL == mixed.high && 0xEDCBA98765432100ULL == mixed.low );
            CUInt128 back = CUInt128::ShiftRight( CUInt128( 0x0123456789ABCDEFULL, 0xFEDCBA9876543210ULL ), 68 );
            ASSERT_TRUE( 0 == back.high && 0x00123456789ABCDEULL == back.low );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL CUInt128 TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
