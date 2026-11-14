/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#ifndef GUCEF_CORE_CUINT128_H
#include "gucefCORE_CUInt128.h"
#define GUCEF_CORE_CUINT128_H
#endif /* GUCEF_CORE_CUINT128_H ? */

#ifndef GUCEF_CORE_CINT128_H
#include "gucefCORE_CInt128.h"
#define GUCEF_CORE_CINT128_H
#endif /* GUCEF_CORE_CINT128_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTDECIMALREFERENCE_H
#include "TestDecimalReference.h"
#define GUCEF_CORE_TESTAPP_TESTDECIMALREFERENCE_H
#endif /* GUCEF_CORE_TESTAPP_TESTDECIMALREFERENCE_H ? */

#include "TestInt128.h"

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
typedef CORE::CInt128 CInt128;

/*-------------------------------------------------------------------------*/

static const UInt64 g_testSeedInt128 = 0x5EED0128A11CEULL;
static const Int64 g_int64Min = -9223372036854775807LL - 1;
static const Int64 g_int64Max = 9223372036854775807LL;

/*-------------------------------------------------------------------------*/

#ifdef __SIZEOF_INT128__
typedef unsigned __int128 TCompilerUInt128;
typedef __int128 TCompilerInt128;

static TCompilerUInt128
Native( const CUInt128& value )
{
    return ( ( (TCompilerUInt128) value.high ) << 64 ) | value.low;
}

static TCompilerInt128
Native( const CInt128& value )
{
    return (TCompilerInt128) ( ( ( (TCompilerUInt128) value.high ) << 64 ) | value.low );
}
#endif

/*-------------------------------------------------------------------------*/

static CRefInt
RefOf( const CUInt128& value )
{
    return CRefInt::FromWords( value.high, value.low );
}

/*-------------------------------------------------------------------------*/

static CRefInt
RefOf( const CInt128& value )
{
    CUInt128 magnitude = value.GetMagnitude();
    CRefInt r = CRefInt::FromWords( magnitude.high, magnitude.low );
    r.negative = value.IsNegative() && !r.IsZero();
    return r;
}

/*-------------------------------------------------------------------------*/

/** Value of a reference integer when it lies in the CInt128 range */
static bool
FromRef( const CRefInt& reference, CInt128& out )
{
    UInt64 high = 0;
    UInt64 low = 0;
    if ( !reference.ToWords( high, low ) )
        return false;
    return CInt128::FromSignMagnitude( reference.negative, CUInt128( high, low ), out );
}

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
        value |= 1ULL << ( bits - 1 );
    return value;
}

/*-------------------------------------------------------------------------*/

static CUInt128
RandomU128( CSplitMix64& rng )
{
    return CUInt128( RandomWord( rng ), RandomWord( rng ) );
}

/*-------------------------------------------------------------------------*/

static CInt128
RandomI128( CSplitMix64& rng )
{
    CUInt128 raw = RandomU128( rng );
    if ( 0 == rng.Below( 2 ) )
        raw = CUInt128::Negate( raw );
    return CInt128( raw.high, raw.low );
}

/*-------------------------------------------------------------------------*/

static Int64
RandomI64( CSplitMix64& rng )
{
    UInt64 w = RandomWord( rng );
    if ( 0 == rng.Below( 2 ) )
        w = 0 - w;
    return (Int64) w;
}

/*-------------------------------------------------------------------------*/

static bool
SameU( const CUInt128& value, const CRefInt& reference )
{
    UInt64 high = 0;
    UInt64 low = 0;
    if ( reference.negative || !reference.ToWords( high, low ) )
        return false;
    return value.high == high && value.low == low;
}

/*-------------------------------------------------------------------------*/

/** n / d with the identities q * d + r == n and r < d, the native type where present and the reference when asked */
static bool
CheckDivMod128( const CUInt128& n, const CUInt128& d, bool useReference )
{
    CUInt128 q;
    CUInt128 r;
    CUInt128::DivMod( n, d, q, r );
    if ( CUInt128::Compare( r, d ) >= 0 )
        return false;
    CUInt128 product;
    if ( !CUInt128::MultiplyChecked( q, d, product ) )
        return false;
    CUInt128 sum;
    if ( !CUInt128::AddChecked( product, r, sum ) || !( sum == n ) )
        return false;

    #ifdef __SIZEOF_INT128__
    if ( Native( q ) != Native( n ) / Native( d ) || Native( r ) != Native( n ) % Native( d ) )
        return false;
    #endif

    if ( useReference )
    {
        CRefInt refQ, refR;
        CRefInt::DivMod( RefOf( n ), RefOf( d ), refQ, refR );
        if ( !SameU( q, refQ ) || !SameU( r, refR ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

static CUInt128
Pow2( UInt32 bits )
{
    return CUInt128::ShiftLeft( CUInt128( 1 ), bits );
}

/*-------------------------------------------------------------------------*/

static CInt128
FromInt( Int64 value )
{
    return CInt128( value );
}

/*-------------------------------------------------------------------------*/

/** -2^127 */
static CInt128
MinI128( void )
{
    return CInt128( 0x8000000000000000ULL, 0 );
}

/*-------------------------------------------------------------------------*/

/** 2^127 - 1 */
static CInt128
MaxI128( void )
{
    return CInt128( 0x7FFFFFFFFFFFFFFFULL, ~0ULL );
}

/*-------------------------------------------------------------------------*/

static bool
IsInt( const CInt128& value, Int64 expected )
{
    return value == CInt128( expected );
}

/*-------------------------------------------------------------------------*/

void
PerformInt128Tests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CInt128 and additive CUInt128 TESTS" );

    GUCEF_TESTFW_TESTCASE( "Test: CUInt128 DivMod 128 by 128 boundaries" )
        try
        {
            const UInt64 m = ~0ULL;
            GUCEF::vector< CUInt128 > values;
            values.push_back( CUInt128() );
            values.push_back( CUInt128( 1 ) );
            values.push_back( CUInt128( 2 ) );
            values.push_back( CUInt128( m ) );
            values.push_back( CUInt128( 1, 0 ) );
            values.push_back( CUInt128( 1, 1 ) );
            values.push_back( CUInt128( 1, m ) );
            values.push_back( CUInt128( 2, 0 ) );
            values.push_back( CUInt128( 3, 5 ) );
            values.push_back( CUInt128( 0x7FFFFFFFFFFFFFFFULL, m ) );
            values.push_back( CUInt128( 0x8000000000000000ULL, 0 ) );
            values.push_back( CUInt128( 0x8000000000000000ULL, 1 ) );
            values.push_back( CUInt128( m, 0 ) );
            values.push_back( CUInt128( m, m - 1 ) );
            values.push_back( CUInt128( m, m ) );
            values.push_back( CUInt128( 0x0123456789ABCDEFULL, 0xFEDCBA9876543210ULL ) );
            values.push_back( CUInt128( 0x00000000FFFFFFFFULL, 0xFFFFFFFF00000000ULL ) );
            values.push_back( CUInt128( 1, 0xFFFFFFFF00000000ULL ) );
            for ( UInt32 k=0; k<=19; ++k )
                values.push_back( CUInt128( CUInt128::Pow10( k ) ) );
            for ( UInt32 b=0; b<128; b+=7 )
            {
                values.push_back( Pow2( b ) );
                values.push_back( CUInt128::Subtract( Pow2( b ), CUInt128( 1 ) ) );
                values.push_back( CUInt128::Add( Pow2( b ), CUInt128( 1 ) ) );
            }
            for ( size_t i=0; i<values.size(); ++i )
            {
                for ( size_t j=0; j<values.size(); ++j )
                {
                    if ( values[ j ].IsZero() )
                        continue;
                    if ( !CheckDivMod128( values[ i ], values[ j ], true ) )
                    {
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DivMod 128 mismatch for " << values[ i ].high << ":" << values[ i ].low << " / " << values[ j ].high << ":" << values[ j ].low );
                        ERRORHERE;
                    }
                }
            }

            // named cases of the design: quotient 0, 1, 2^64 - 1, n == d, n == d - 1, 2^128 - 1 over 2^64 + 1
            CUInt128 q, r;
            CUInt128::DivMod( CUInt128( m, m ), CUInt128( 1, 1 ), q, r );
            ASSERT_TRUE( CUInt128( m ) == q && r.IsZero() );
            CUInt128::DivMod( CUInt128( 5, 5 ), CUInt128( 5, 5 ), q, r );
            ASSERT_TRUE( CUInt128( 1 ) == q && r.IsZero() );
            CUInt128::DivMod( CUInt128( 5, 4 ), CUInt128( 5, 5 ), q, r );
            ASSERT_TRUE( q.IsZero() && r == CUInt128( 5, 4 ) );
            CUInt128::DivMod( CUInt128( 0x8000000000000000ULL, 0 ), CUInt128( 0x8000000000000000ULL, 0 ), q, r );
            ASSERT_TRUE( CUInt128( 1 ) == q && r.IsZero() );
            // d.high == 0 forwards to the 128 by 64 path
            CUInt128::DivMod( CUInt128( 7, 9 ), CUInt128( 3 ), q, r );
            CUInt128 q64;
            UInt64 r64 = 0;
            CUInt128::DivMod( CUInt128( 7, 9 ), 3, q64, r64 );
            ASSERT_TRUE( q == q64 && r == CUInt128( r64 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CUInt128 DivMod 128 by 128 random properties q * d + r == n and r < d" )
        try
        {
            CSplitMix64 rng( g_testSeedInt128 );
            for ( UInt32 i=0; i<1000000; ++i )
            {
                CUInt128 n = RandomU128( rng );
                CUInt128 d = RandomU128( rng );
                if ( d.IsZero() )
                    continue;
                if ( !CheckDivMod128( n, d, 0 == i % 100 ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DivMod 128 property failed at iteration " << i << " seed " << g_testSeedInt128 );
                    ERRORHERE;
                    break;
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CUInt128 MultiplyChecked 128 by 128, Multiply128Low, bitwise helpers and comparison" )
        try
        {
            CUInt128 out;
            // overflow exactly at 2^128
            ASSERT_FALSE( CUInt128::MultiplyChecked( Pow2( 64 ), Pow2( 64 ), out ) );
            ASSERT_TRUE( CUInt128::MultiplyChecked( CUInt128( ~0ULL ), CUInt128( 1, 1 ), out ) );
            ASSERT_TRUE( out == CUInt128( ~0ULL, ~0ULL ) );
            ASSERT_TRUE( CUInt128::MultiplyChecked( CUInt128( 1, 1 ), CUInt128( ~0ULL ), out ) );
            ASSERT_TRUE( out == CUInt128( ~0ULL, ~0ULL ) );
            ASSERT_TRUE( CUInt128::MultiplyChecked( CUInt128(), CUInt128( ~0ULL, ~0ULL ), out ) && out.IsZero() );
            ASSERT_TRUE( CUInt128::MultiplyChecked( CUInt128( ~0ULL, ~0ULL ), CUInt128( 1 ), out ) && out == CUInt128( ~0ULL, ~0ULL ) );
            ASSERT_FALSE( CUInt128::MultiplyChecked( CUInt128( ~0ULL, ~0ULL ), CUInt128( 2 ), out ) );
            ASSERT_FALSE( CUInt128::MultiplyChecked( Pow2( 100 ), Pow2( 28 ), out ) );
            ASSERT_TRUE( CUInt128::MultiplyChecked( Pow2( 100 ), Pow2( 27 ), out ) && out == Pow2( 127 ) );

            CSplitMix64 rng( g_testSeedInt128 + 1 );
            for ( UInt32 i=0; i<200000; ++i )
            {
                CUInt128 a = RandomU128( rng );
                CUInt128 b = RandomU128( rng );
                bool fits = CUInt128::MultiplyChecked( a, b, out );
                CRefInt product = CRefInt::Multiply( RefOf( a ), RefOf( b ) );
                UInt64 high = 0;
                UInt64 low = 0;
                bool refFits = product.ToWords( high, low );
                if ( fits != refFits || ( fits && !( out == CUInt128( high, low ) ) ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MultiplyChecked 128 failed at iteration " << i << " seed " << ( g_testSeedInt128 + 1 ) );
                    ERRORHERE;
                    break;
                }
                #ifdef __SIZEOF_INT128__
                CUInt128 wrapped = CUInt128::Multiply128Low( a, b );
                TCompilerUInt128 expected = Native( a ) * Native( b );
                if ( Native( wrapped ) != expected )
                {
                    ERRORHERE;
                    break;
                }
                #endif
            }

            CUInt128 a( 0xF0F0F0F0F0F0F0F0ULL, 0x00FF00FF00FF00FFULL );
            CUInt128 b( 0x0FF00FF00FF00FF0ULL, 0xFFFF0000FFFF0000ULL );
            ASSERT_TRUE( CUInt128::And( a, b ) == CUInt128( 0x00F000F000F000F0ULL, 0x00FF000000FF0000ULL ) );
            ASSERT_TRUE( CUInt128::Or( a, b ) == CUInt128( 0xFFF0FFF0FFF0FFF0ULL, 0xFFFF00FFFFFF00FFULL ) );
            ASSERT_TRUE( CUInt128::Xor( a, b ) == CUInt128( 0xFF00FF00FF00FF00ULL, 0xFF0000FFFF0000FFULL ) );
            ASSERT_TRUE( CUInt128::Not( CUInt128() ) == CUInt128( ~0ULL, ~0ULL ) );
            ASSERT_TRUE( CUInt128::Not( a ) == CUInt128( ~a.high, ~a.low ) );

            ASSERT_TRUE( 128 == CUInt128().CountTrailingZeros() );
            ASSERT_TRUE( 64 == CORE::CUInt128::CountTrailingZeros( (UInt64) 0 ) );
            for ( UInt32 s=0; s<128; ++s )
            {
                ASSERT_TRUE( s == Pow2( s ).CountTrailingZeros() );
                ASSERT_TRUE( s == CUInt128::Or( Pow2( s ), Pow2( 127 ) ).CountTrailingZeros() );
            }
            ASSERT_TRUE( 0 == CORE::CUInt128::CountTrailingZeros( (UInt64) 1 ) );
            ASSERT_TRUE( 63 == CORE::CUInt128::CountTrailingZeros( 0x8000000000000000ULL ) );

            CUInt128 lo( 0, 5 );
            CUInt128 hi( 1, 0 );
            ASSERT_TRUE( lo < hi && lo <= hi && hi > lo && hi >= lo && lo != hi && !( lo == hi ) );
            ASSERT_TRUE( lo <= lo && lo >= lo && lo == CUInt128( 5 ) && !( lo < lo ) && !( lo > lo ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CInt128 construction, queries and boundaries" )
        try
        {
            CInt128 zero;
            ASSERT_TRUE( zero.IsZero() && !zero.IsNegative() && 0 == zero.GetSign() && !zero.IsOdd() );
            ASSERT_TRUE( 1 == FromInt( 5 ).GetSign() && -1 == FromInt( -5 ).GetSign() );
            ASSERT_TRUE( FromInt( -1 ) == CInt128( ~0ULL, ~0ULL ) );
            ASSERT_TRUE( FromInt( g_int64Min ) == CInt128( ~0ULL, 0x8000000000000000ULL ) );
            ASSERT_TRUE( FromInt( -3 ).IsOdd() && !FromInt( -4 ).IsOdd() );
            ASSERT_TRUE( CInt128::FromUInt64( ~0ULL ) == CInt128( 0, ~0ULL ) );

            ASSERT_TRUE( FromInt( g_int64Min ).FitsInt64() && FromInt( g_int64Max ).FitsInt64() );
            ASSERT_FALSE( CInt128( 0, 0x8000000000000000ULL ).FitsInt64() );
            ASSERT_FALSE( CInt128( ~0ULL, 0x7FFFFFFFFFFFFFFFULL ).FitsInt64() );
            ASSERT_FALSE( MinI128().FitsInt64() || MaxI128().FitsInt64() );
            ASSERT_TRUE( CInt128( 0, ~0ULL ).FitsUInt64() && !FromInt( -1 ).FitsUInt64() && !CInt128( 1, 0 ).FitsUInt64() );

            ASSERT_TRUE( MinI128().IsNegative() && !MaxI128().IsNegative() );
            ASSERT_TRUE( 128 == MinI128().BitLength() );
            ASSERT_TRUE( 127 == MaxI128().BitLength() );
            ASSERT_TRUE( 0 == zero.BitLength() && 1 == FromInt( -1 ).BitLength() && 64 == FromInt( g_int64Min ).BitLength() );
            ASSERT_TRUE( MinI128().GetMagnitude() == Pow2( 127 ) );
            ASSERT_TRUE( FromInt( -7 ).GetMagnitude() == CUInt128( 7 ) );

            // ordering
            ASSERT_TRUE( MinI128() < FromInt( g_int64Min ) && FromInt( g_int64Min ) < FromInt( -1 ) );
            ASSERT_TRUE( FromInt( -1 ) < zero && zero < FromInt( 1 ) && FromInt( g_int64Max ) < MaxI128() );
            ASSERT_TRUE( MinI128() < MaxI128() && MaxI128() > MinI128() && MinI128() <= MinI128() && MaxI128() >= MaxI128() );
            ASSERT_TRUE( 0 == CInt128::Compare( MinI128(), MinI128() ) && -1 == CInt128::Compare( MinI128(), MaxI128() ) && 1 == CInt128::Compare( MaxI128(), MinI128() ) );

            // checked conversions
            Int64 i64 = 0;
            UInt64 u64 = 0;
            CUInt128 u128;
            ASSERT_TRUE( FromInt( g_int64Min ).ToInt64Checked( i64 ) && g_int64Min == i64 );
            ASSERT_FALSE( CInt128( 0, 0x8000000000000000ULL ).ToInt64Checked( i64 ) );
            ASSERT_TRUE( CInt128( 0, ~0ULL ).ToUInt64Checked( u64 ) && ~0ULL == u64 );
            ASSERT_FALSE( FromInt( -1 ).ToUInt64Checked( u64 ) );
            ASSERT_TRUE( MaxI128().ToCUInt128Checked( u128 ) && u128 == CUInt128( 0x7FFFFFFFFFFFFFFFULL, ~0ULL ) );
            ASSERT_FALSE( FromInt( -1 ).ToCUInt128Checked( u128 ) );

            CInt128 fromU;
            ASSERT_TRUE( CInt128::FromCUInt128Checked( CUInt128( 0x7FFFFFFFFFFFFFFFULL, ~0ULL ), fromU ) && fromU == MaxI128() );
            ASSERT_FALSE( CInt128::FromCUInt128Checked( CUInt128( 0x8000000000000000ULL, 0 ), fromU ) );

            // sign magnitude edges: 2^127 is accepted as negative and rejected as positive
            CInt128 sm;
            ASSERT_TRUE( CInt128::FromSignMagnitude( true, Pow2( 127 ), sm ) && sm == MinI128() );
            ASSERT_FALSE( CInt128::FromSignMagnitude( false, Pow2( 127 ), sm ) );
            ASSERT_TRUE( CInt128::FromSignMagnitude( false, CUInt128::Subtract( Pow2( 127 ), CUInt128( 1 ) ), sm ) && sm == MaxI128() );
            ASSERT_FALSE( CInt128::FromSignMagnitude( true, CUInt128::Add( Pow2( 127 ), CUInt128( 1 ) ), sm ) );
            ASSERT_TRUE( CInt128::FromSignMagnitude( true, CUInt128(), sm ) && sm.IsZero() );
            ASSERT_TRUE( CInt128::FromSignMagnitude( true, CUInt128( 5 ), sm ) && IsInt( sm, -5 ) );

            ASSERT_TRUE( 1 == CInt128::Pow10( 0 ) && 10000000000000000000ULL == CInt128::Pow10( 19 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CInt128 add, subtract, negate, checked variants" )
        try
        {
            CInt128 out;
            ASSERT_FALSE( CInt128::NegateChecked( MinI128(), out ) );
            ASSERT_TRUE( out == MinI128() );
            ASSERT_TRUE( CInt128::NegateChecked( MaxI128(), out ) && CInt128::Add( out, FromInt( -1 ) ) == MinI128() );
            ASSERT_TRUE( CInt128::Negate( MinI128() ) == MinI128() );
            ASSERT_TRUE( IsInt( CInt128::Negate( FromInt( 9 ) ), -9 ) );

            ASSERT_FALSE( CInt128::AddChecked( MaxI128(), FromInt( 1 ), out ) );
            ASSERT_TRUE( out == MinI128() );
            ASSERT_TRUE( CInt128::AddChecked( MaxI128(), FromInt( -1 ), out ) && out == CInt128::Subtract( MaxI128(), FromInt( 1 ) ) );
            ASSERT_FALSE( CInt128::AddChecked( MinI128(), FromInt( -1 ), out ) );
            ASSERT_TRUE( CInt128::AddChecked( MinI128(), MaxI128(), out ) && IsInt( out, -1 ) );
            ASSERT_FALSE( CInt128::SubtractChecked( MinI128(), FromInt( 1 ), out ) );
            ASSERT_FALSE( CInt128::SubtractChecked( MaxI128(), FromInt( -1 ), out ) );
            ASSERT_TRUE( CInt128::SubtractChecked( MinI128(), MinI128(), out ) && out.IsZero() );
            ASSERT_TRUE( CInt128::SubtractChecked( FromInt( 5 ), FromInt( 9 ), out ) && IsInt( out, -4 ) );
            ASSERT_TRUE( CInt128::SubtractChecked( FromInt( -1 ), MinI128(), out ) && out == MaxI128() );   // -1 - min = 2^127 - 1 fits

            // random: wrapping and checked add/subtract against the reference
            CSplitMix64 rng( g_testSeedInt128 + 2 );
            for ( UInt32 i=0; i<100000; ++i )
            {
                CInt128 a = RandomI128( rng );
                CInt128 b = RandomI128( rng );
                CRefInt sum = CRefInt::Add( RefOf( a ), RefOf( b ) );
                CRefInt diff = CRefInt::Subtract( RefOf( a ), RefOf( b ) );
                CInt128 expected;
                bool sumFits = FromRef( sum, expected );
                bool ok = sumFits == CInt128::AddChecked( a, b, out ) && ( !sumFits || out == expected );
                bool diffFits = FromRef( diff, expected );
                ok = ok && diffFits == CInt128::SubtractChecked( a, b, out ) && ( !diffFits || out == expected );
                if ( !ok )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CInt128 add/subtract failed at iteration " << i << " seed " << ( g_testSeedInt128 + 2 ) );
                    ERRORHERE;
                    break;
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CInt128 multiply" )
        try
        {
            // INT64_MIN * INT64_MIN == 2^126
            ASSERT_TRUE( CInt128::Multiply( g_int64Min, g_int64Min ) == CInt128( 0x4000000000000000ULL, 0 ) );
            ASSERT_TRUE( CInt128::Multiply( g_int64Min, g_int64Max ) == CInt128::Negate( CInt128::Subtract( CInt128( 0x4000000000000000ULL, 0 ), CInt128( 0, 0x8000000000000000ULL ) ) ) );
            ASSERT_TRUE( CInt128::Multiply( 0, g_int64Min ).IsZero() );
            ASSERT_TRUE( IsInt( CInt128::Multiply( -3, 7 ), -21 ) && IsInt( CInt128::Multiply( -3, -7 ), 21 ) );

            CInt128 out;
            ASSERT_FALSE( CInt128::MultiplyChecked( CInt128( 0x4000000000000000ULL, 0 ), FromInt( 2 ), out ) );
            ASSERT_TRUE( CInt128::MultiplyChecked( CInt128( 0x4000000000000000ULL, 0 ), FromInt( -2 ), out ) && out == MinI128() );
            ASSERT_TRUE( CInt128::MultiplyChecked( MinI128(), FromInt( 1 ), out ) && out == MinI128() );
            ASSERT_FALSE( CInt128::MultiplyChecked( MinI128(), FromInt( -1 ), out ) );
            ASSERT_FALSE( CInt128::MultiplyChecked( MinI128(), FromInt( 2 ), out ) );
            ASSERT_TRUE( CInt128::MultiplyChecked( MaxI128(), FromInt( -1 ), out ) && CInt128::Add( out, MaxI128() ).IsZero() );
            ASSERT_TRUE( CInt128::MultiplyChecked( MinI128(), 0, out ) && out.IsZero() );
            ASSERT_FALSE( CInt128::MultiplyChecked( MaxI128(), g_int64Min, out ) );

            CSplitMix64 rng( g_testSeedInt128 + 3 );
            for ( UInt32 i=0; i<100000; ++i )
            {
                Int64 a = RandomI64( rng );
                Int64 b = RandomI64( rng );
                CRefInt expected = CRefInt::Multiply( CRefInt::FromInt64( a ), CRefInt::FromInt64( b ) );
                CInt128 got = CInt128::Multiply( a, b );
                bool ok = 0 == CRefInt::Compare( RefOf( got ), expected );

                CInt128 x = RandomI128( rng );
                CInt128 y = RandomI128( rng );
                CRefInt product = CRefInt::Multiply( RefOf( x ), RefOf( y ) );
                CInt128 expectedWide;
                bool fits = FromRef( product, expectedWide );
                ok = ok && fits == CInt128::MultiplyChecked( x, y, out ) && ( !fits || out == expectedWide );
                ok = ok && CInt128::MultiplyChecked( x, a, out ) == FromRef( CRefInt::Multiply( RefOf( x ), CRefInt::FromInt64( a ) ), expectedWide );
                if ( !ok )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CInt128 multiply failed at iteration " << i << " seed " << ( g_testSeedInt128 + 3 ) );
                    ERRORHERE;
                    break;
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CInt128 division semantics table truncating and floor" )
        try
        {
            // n, d, truncating q, r, floor q, r
            static const Int64 table[ 4 ][ 6 ] =
            {
                { -7,  2, -3, -1, -4,  1 },
                {  7, -2, -3,  1, -4, -1 },
                { -7, -2,  3, -1,  3, -1 },
                {  7,  2,  3,  1,  3,  1 }
            };
            for ( UInt32 i=0; i<4; ++i )
            {
                CInt128 q, r;
                ASSERT_TRUE( CInt128::DivMod( FromInt( table[ i ][ 0 ] ), FromInt( table[ i ][ 1 ] ), q, r ) );
                ASSERT_TRUE( IsInt( q, table[ i ][ 2 ] ) && IsInt( r, table[ i ][ 3 ] ) );
                ASSERT_TRUE( CInt128::DivModFloor( FromInt( table[ i ][ 0 ] ), FromInt( table[ i ][ 1 ] ), q, r ) );
                ASSERT_TRUE( IsInt( q, table[ i ][ 4 ] ) && IsInt( r, table[ i ][ 5 ] ) );

                CInt128 q2;
                Int64 r64 = 0;
                ASSERT_TRUE( CInt128::DivMod( FromInt( table[ i ][ 0 ] ), table[ i ][ 1 ], q2, r64 ) );
                ASSERT_TRUE( IsInt( q2, table[ i ][ 2 ] ) && table[ i ][ 3 ] == r64 );
            }

            // overflow and zero divisor
            CInt128 q( 0, 77 );
            CInt128 r( 0, 88 );
            ASSERT_FALSE( CInt128::DivMod( MinI128(), FromInt( -1 ), q, r ) );
            ASSERT_TRUE( 77 == q.low && 88 == r.low );
            ASSERT_FALSE( CInt128::DivMod( FromInt( 5 ), FromInt( 0 ), q, r ) );
            ASSERT_FALSE( CInt128::DivModFloor( FromInt( 5 ), FromInt( 0 ), q, r ) );
            ASSERT_TRUE( CInt128::DivMod( MinI128(), FromInt( 1 ), q, r ) && q == MinI128() && r.IsZero() );
            ASSERT_TRUE( CInt128::DivMod( MinI128(), MinI128(), q, r ) && IsInt( q, 1 ) && r.IsZero() );
            ASSERT_TRUE( CInt128::DivMod( MaxI128(), MinI128(), q, r ) && q.IsZero() && r == MaxI128() );
            ASSERT_TRUE( CInt128::DivMod( MinI128(), MaxI128(), q, r ) && IsInt( q, -1 ) && IsInt( r, -1 ) );
            Int64 r64 = 0;
            ASSERT_TRUE( CInt128::DivMod( MinI128(), g_int64Min, q, r64 ) && q == CInt128( 1, 0 ) && 0 == r64 );
            ASSERT_FALSE( CInt128::DivMod( FromInt( 1 ), 0, q, r64 ) );

            // random: against the reference (truncating) and the identity for floor
            CSplitMix64 rng( g_testSeedInt128 + 4 );
            for ( UInt32 i=0; i<50000; ++i )
            {
                CInt128 n = RandomI128( rng );
                CInt128 d = RandomI128( rng );
                if ( d.IsZero() )
                    continue;
                CRefInt refQ, refR;
                CRefInt::DivMod( RefOf( n ), RefOf( d ), refQ, refR );
                refQ.negative = ( n.IsNegative() != d.IsNegative() ) && !refQ.IsZero();
                refR.negative = n.IsNegative() && !refR.IsZero();
                CInt128 expectedQ, expectedR;
                bool fits = FromRef( refQ, expectedQ );
                FromRef( refR, expectedR );
                bool ok = fits == CInt128::DivMod( n, d, q, r );
                if ( fits )
                    ok = ok && q == expectedQ && r == expectedR;
                if ( ok && fits )
                {
                    // floor: q * d + r == n, r is zero or has the sign of d, |r| < |d|
                    CInt128 fq, fr;
                    bool floorOk = CInt128::DivModFloor( n, d, fq, fr );
                    if ( floorOk )
                    {
                        // the identity is checked in the reference: q * d alone may leave the 128 bit range
                        floorOk = 0 == CRefInt::Compare( CRefInt::Add( CRefInt::Multiply( RefOf( fq ), RefOf( d ) ), RefOf( fr ) ), RefOf( n ) );
                        floorOk = floorOk && ( fr.IsZero() || fr.IsNegative() == d.IsNegative() );
                        floorOk = floorOk && CUInt128::Compare( fr.GetMagnitude(), d.GetMagnitude() ) < 0;
                    }
                    ok = floorOk;
                }
                if ( !ok )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CInt128 division failed at iteration " << i << " seed " << ( g_testSeedInt128 + 4 ) );
                    ERRORHERE;
                    break;
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CInt128 shifts and bitwise operations" )
        try
        {
            ASSERT_TRUE( CInt128::ShiftRightArithmetic( FromInt( -1 ), 200 ) == FromInt( -1 ) );
            ASSERT_TRUE( CInt128::ShiftRightArithmetic( MinI128(), 127 ) == FromInt( -1 ) );
            ASSERT_TRUE( CInt128::ShiftRightArithmetic( MinI128(), 128 ) == FromInt( -1 ) );
            ASSERT_TRUE( CInt128::ShiftRightArithmetic( MaxI128(), 127 ).IsZero() );
            ASSERT_TRUE( CInt128::ShiftRightArithmetic( MaxI128(), 500 ).IsZero() );
            ASSERT_TRUE( IsInt( CInt128::ShiftRightArithmetic( FromInt( -7 ), 1 ), -4 ) );     // floor
            ASSERT_TRUE( IsInt( CInt128::ShiftRightArithmetic( FromInt( -8 ), 1 ), -4 ) );
            ASSERT_TRUE( IsInt( CInt128::ShiftRightArithmetic( FromInt( 7 ), 1 ), 3 ) );
            ASSERT_TRUE( CInt128::ShiftRightArithmetic( MinI128(), 64 ) == CInt128( ~0ULL, 0x8000000000000000ULL ) );
            ASSERT_TRUE( CInt128::ShiftRightArithmetic( FromInt( -5 ), 0 ) == FromInt( -5 ) );

            ASSERT_TRUE( CInt128::ShiftRightLogical( FromInt( -1 ), 1 ) == MaxI128() );
            ASSERT_TRUE( CInt128::ShiftRightLogical( FromInt( -1 ), 128 ).IsZero() );
            ASSERT_TRUE( CInt128::ShiftLeft( FromInt( 1 ), 127 ) == MinI128() );
            ASSERT_TRUE( CInt128::ShiftLeft( FromInt( 1 ), 128 ).IsZero() );
            ASSERT_TRUE( CInt128::ShiftLeft( FromInt( -1 ), 64 ) == CInt128( ~0ULL, 0 ) );

            CSplitMix64 rng( g_testSeedInt128 + 5 );
            for ( UInt32 i=0; i<20000; ++i )
            {
                CInt128 a = RandomI128( rng );
                UInt32 s = (UInt32) rng.Below( 140 );
                CInt128 got = CInt128::ShiftRightArithmetic( a, s );
                // floor( a / 2^s ) by DivModFloor, valid for s < 127
                if ( s < 127 )
                {
                    CInt128 q, r;
                    CInt128::DivModFloor( a, CInt128( CUInt128::ShiftLeft( CUInt128( 1 ), s ).high, CUInt128::ShiftLeft( CUInt128( 1 ), s ).low ), q, r );
                    if ( !( q == got ) )
                    {
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CInt128 arithmetic shift failed at iteration " << i << " seed " << ( g_testSeedInt128 + 5 ) );
                        ERRORHERE;
                        break;
                    }
                }
                #ifdef __SIZEOF_INT128__
                if ( s < 128 && Native( got ) != ( Native( a ) >> s ) )
                {
                    ERRORHERE;
                    break;
                }
                #endif
            }

            CInt128 a( 0xF0F0F0F0F0F0F0F0ULL, 0x00FF00FF00FF00FFULL );
            CInt128 b( 0x0FF00FF00FF00FF0ULL, 0xFFFF0000FFFF0000ULL );
            ASSERT_TRUE( CInt128::And( a, b ) == CInt128( 0x00F000F000F000F0ULL, 0x00FF000000FF0000ULL ) );
            ASSERT_TRUE( CInt128::Or( a, b ) == CInt128( 0xFFF0FFF0FFF0FFF0ULL, 0xFFFF00FFFFFF00FFULL ) );
            ASSERT_TRUE( CInt128::Xor( a, b ) == CInt128( 0xFF00FF00FF00FF00ULL, 0xFF0000FFFF0000FFULL ) );
            ASSERT_TRUE( CInt128::Not( CInt128() ) == FromInt( -1 ) );
            ASSERT_TRUE( CInt128::Not( MinI128() ) == MaxI128() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CInt128 ToFloat64 ties to even and boundaries" )
        try
        {
            ASSERT_TRUE( 0.0 == CInt128().ToFloat64() );
            ASSERT_TRUE( 1.0 == FromInt( 1 ).ToFloat64() && -1.0 == FromInt( -1 ).ToFloat64() );
            ASSERT_TRUE( 9007199254740992.0 == FromInt( 9007199254740992LL ).ToFloat64() );
            // 2^53 + 1 is a tie: rounds to the even neighbour 2^53
            ASSERT_TRUE( 9007199254740992.0 == FromInt( 9007199254740993LL ).ToFloat64() );
            // 2^53 + 3 is a tie: rounds to the even neighbour 2^53 + 4
            ASSERT_TRUE( 9007199254740996.0 == FromInt( 9007199254740995LL ).ToFloat64() );
            ASSERT_TRUE( -9007199254740992.0 == FromInt( -9007199254740993LL ).ToFloat64() );
            // 2^127 - 1 rounds to 2^127, -2^127 is exact
            ASSERT_TRUE( ldexp( 1.0, 127 ) == MaxI128().ToFloat64() );
            ASSERT_TRUE( -ldexp( 1.0, 127 ) == MinI128().ToFloat64() );
            ASSERT_TRUE( ldexp( 1.0, 64 ) == CInt128( 1, 0 ).ToFloat64() );
            // 2^64 + 2^11 is a tie at the 53 bit boundary and rounds to even (2^64), 2^64 + 3 * 2^11 rounds up
            ASSERT_TRUE( ldexp( 1.0, 64 ) == CInt128( 1, 2048 ).ToFloat64() );
            ASSERT_TRUE( ldexp( 1.0, 64 ) + ldexp( 1.0, 13 ) == CInt128( 1, 6144 ).ToFloat64() );
            // just above the tie: rounds up
            ASSERT_TRUE( ldexp( 1.0, 64 ) + ldexp( 1.0, 12 ) == CInt128( 1, 2049 ).ToFloat64() );

            #ifdef __SIZEOF_INT128__
            CSplitMix64 rng( g_testSeedInt128 + 6 );
            for ( UInt32 i=0; i<200000; ++i )
            {
                CInt128 v = RandomI128( rng );
                if ( v.ToFloat64() != (double) Native( v ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CInt128 ToFloat64 failed at iteration " << i << " seed " << ( g_testSeedInt128 + 6 ) );
                    ERRORHERE;
                    break;
                }
            }
            #endif
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL CInt128 and additive CUInt128 TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
