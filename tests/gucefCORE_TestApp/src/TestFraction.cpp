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

#ifndef GUCEF_CORE_CTFRACTION_H
#include "gucefCORE_CTFraction.h"
#define GUCEF_CORE_CTFRACTION_H
#endif /* GUCEF_CORE_CTFRACTION_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTDECIMALREFERENCE_H
#include "TestDecimalReference.h"
#define GUCEF_CORE_TESTAPP_TESTDECIMALREFERENCE_H
#endif /* GUCEF_CORE_TESTAPP_TESTDECIMALREFERENCE_H ? */

#ifndef GUCEF_CORE_CBIGINTEGER_H
#include "gucefCORE_CBigInteger.h"
#define GUCEF_CORE_CBIGINTEGER_H
#endif /* GUCEF_CORE_CBIGINTEGER_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTALLOCATIONCOUNTER_H
#include "TestAllocationCounter.h"
#define GUCEF_CORE_TESTAPP_TESTALLOCATIONCOUNTER_H
#endif /* GUCEF_CORE_TESTAPP_TESTALLOCATIONCOUNTER_H ? */

#include "TestFraction.h"

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

/*-------------------------------------------------------------------------*/

static const UInt64 g_testSeedFraction = 0x5EED0F4AC71ULL;
static const Int64 g_fractionInt64Min = -9223372036854775807LL - 1;
static const Int64 g_fractionInt64Max = 9223372036854775807LL;

/*-------------------------------------------------------------------------*/

/** Exact rational reference: numerator and positive denominator in lowest terms, built on the base 10^9 reference integer */
struct SRefFraction
{
    CRefInt n;
    CRefInt d;
};

/*-------------------------------------------------------------------------*/

static CRefInt
RefGcd( CRefInt a, CRefInt b )
{
    a = CRefInt::Abs( a );
    b = CRefInt::Abs( b );
    while ( !b.IsZero() )
    {
        CRefInt q, r;
        CRefInt::DivMod( a, b, q, r );
        a = b;
        b = r;
    }
    return a;
}

/*-------------------------------------------------------------------------*/

static SRefFraction
RefReduce( const CRefInt& n, const CRefInt& d )
{
    SRefFraction result;
    bool negative = ( n.negative != d.negative ) && !n.IsZero();
    CRefInt g = RefGcd( n, d );
    CRefInt q, r;
    CRefInt::DivMod( CRefInt::Abs( n ), g, q, r );
    result.n = q;
    result.n.negative = negative && !q.IsZero();
    CRefInt::DivMod( CRefInt::Abs( d ), g, q, r );
    result.d = q;
    return result;
}

/*-------------------------------------------------------------------------*/

/** 0 add, 1 subtract, 2 multiply, 3 divide of two Int64 fractions, exact */
static SRefFraction
RefOperation( UInt32 op, const CORE::TInt64Fraction& a, const CORE::TInt64Fraction& b )
{
    CRefInt an = CRefInt::FromInt64( a.GetNumerator() );
    CRefInt ad = CRefInt::FromInt64( a.GetDenominator() );
    CRefInt bn = CRefInt::FromInt64( b.GetNumerator() );
    CRefInt bd = CRefInt::FromInt64( b.GetDenominator() );
    switch ( op )
    {
        case 0: return RefReduce( CRefInt::Add( CRefInt::Multiply( an, bd ), CRefInt::Multiply( bn, ad ) ), CRefInt::Multiply( ad, bd ) );
        case 1: return RefReduce( CRefInt::Subtract( CRefInt::Multiply( an, bd ), CRefInt::Multiply( bn, ad ) ), CRefInt::Multiply( ad, bd ) );
        case 2: return RefReduce( CRefInt::Multiply( an, bn ), CRefInt::Multiply( ad, bd ) );
        default: return RefReduce( CRefInt::Multiply( an, bd ), CRefInt::Multiply( ad, bn ) );
    }
}

/*-------------------------------------------------------------------------*/

/** Fits the Int64 range -2^63 .. 2^63 - 1 */
static bool
RefFitsInt64( const CRefInt& value )
{
    CRefInt limit = CRefInt::PowerOfTwo( 63 );
    Int32 c = CRefInt::CompareMagnitude( value, limit );
    if ( c < 0 )
        return true;
    return 0 == c && value.negative;
}

/*-------------------------------------------------------------------------*/

static UInt64
FractionRandomWord( CSplitMix64& rng )
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

/** Random Int64 fraction, a share of them close to 2^62 so that cross products and sums overflow 64 bit often */
static CORE::TInt64Fraction
RandomInt64Fraction( CSplitMix64& rng )
{
    UInt64 nMagnitude = 0;
    UInt64 dMagnitude = 0;
    if ( 0 == rng.Below( 4 ) )
    {
        nMagnitude = ( 1ULL << 62 ) + rng.Below( 1ULL << 20 );
        dMagnitude = ( 1ULL << 62 ) + rng.Below( 1ULL << 20 );
    }
    else
    {
        nMagnitude = FractionRandomWord( rng );
        dMagnitude = FractionRandomWord( rng ) & 0x7FFFFFFFFFFFFFFFULL;
    }
    if ( 0 == dMagnitude )
        dMagnitude = 1;
    Int64 n = 0 == rng.Below( 2 ) ? (Int64) ( nMagnitude & 0x7FFFFFFFFFFFFFFFULL ) : -(Int64) ( nMagnitude & 0x7FFFFFFFFFFFFFFFULL );
    return CORE::TInt64Fraction( n, (Int64) dMagnitude );
}

/*-------------------------------------------------------------------------*/

/** Test local plain Euclid oracle for the normalisation of CTFraction< T, T >: 10000 seeded pairs */
template < typename T >
static bool
CheckNormalizeAgainstOracle( UInt64 seed )
{
    CSplitMix64 rng( seed );
    const UInt32 bits = (UInt32) sizeof( T ) * 8;
    const bool isSigned = ( (T) -1 ) < ( (T) 0 );
    for ( UInt32 i=0; i<10000; ++i )
    {
        T n = (T) FractionRandomWord( rng );
        T d = (T) FractionRandomWord( rng );
        if ( 0 == d )
            continue;
        UInt64 nMagnitude = n;
        UInt64 dMagnitude = d;
        bool nNegative = false;
        bool dNegative = false;
        if ( isSigned )
        {
            nNegative = n < (T) 0;
            dNegative = d < (T) 0;
            nMagnitude = nNegative ? 0 - (UInt64) (Int64) n : (UInt64) (Int64) n;
            dMagnitude = dNegative ? 0 - (UInt64) (Int64) d : (UInt64) (Int64) d;
            // the type minimum has no representable negation: not a defined input of the old code
            if ( nNegative && nMagnitude == 1ULL << ( bits - 1 ) )
                continue;
            if ( dNegative && dMagnitude == 1ULL << ( bits - 1 ) )
                continue;
        }
        UInt64 a = nMagnitude;
        UInt64 b = dMagnitude;
        while ( 0 != b )
        {
            UInt64 t = a % b;
            a = b;
            b = t;
        }
        UInt64 expectedN = nMagnitude / a;
        UInt64 expectedD = dMagnitude / a;
        bool expectedNegative = ( nNegative != dNegative ) && 0 != expectedN;
        CORE::CTFraction< T, T > f( n, d );
        UInt64 gotN = f.GetNumerator();
        UInt64 gotD = f.GetDenominator();
        bool gotNegative = false;
        if ( isSigned )
        {
            gotNegative = f.GetNumerator() < (T) 0;
            gotN = gotNegative ? 0 - (UInt64) (Int64) f.GetNumerator() : (UInt64) (Int64) f.GetNumerator();
            gotD = (UInt64) (Int64) f.GetDenominator();
        }
        if ( 0 == expectedN )
            expectedD = 1;
        if ( gotN != expectedN || gotD != expectedD || gotNegative != expectedNegative )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Normalize oracle mismatch at iteration " << i << " seed " << seed << " bits " << bits );
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

typedef CORE::TBigIntegerFraction TBigFraction;

/*-------------------------------------------------------------------------*/

static CORE::CBigInteger
BigOf( Int64 value )
{
    return CORE::CBigInteger( value );
}

/*-------------------------------------------------------------------------*/

/** Same value: the big fraction holds exactly the Int64 fraction's numerator and denominator (both are in lowest terms) */
static bool
SameFraction( const TBigFraction& big, const CORE::TInt64Fraction& small )
{
    return big.GetNumerator() == BigOf( small.GetNumerator() ) && big.GetDenominator() == BigOf( small.GetDenominator() );
}

/*-------------------------------------------------------------------------*/

/** T-F6 to T-F8 and the CBigInteger half of T-F10: CTFraction over CBigInteger */
static void
PerformBigIntegerFractionTests( void )
{
    // T-F6: the arithmetic of the Int64 fractions repeated with big integer fractions, and exact sums
    GUCEF_TESTFW_TESTCASE( "T-F6: CTFraction< CBigInteger > arithmetic, comparison and mixed fractions equal the exact Int64 results" )
        try
        {
            TBigFraction zero;
            ASSERT_TRUE( zero.GetNumerator().IsZero() && zero.GetDenominator().IsOne() );
            TBigFraction a( 6, -8 );
            ASSERT_TRUE( a.GetNumerator() == -3 && a.GetDenominator() == 4 );
            TBigFraction b( 1, 6 );
            ASSERT_TRUE( ( a + b ).ToString() == "-7/12" && ( a - b ).ToString() == "-11/12" );
            ASSERT_TRUE( ( a * b ).ToString() == "-1/8" && ( a / b ).ToString() == "-9/2" );
            ASSERT_TRUE( ( a + 2 ).ToString() == "5/4" && ( a - 2 ).ToString() == "-11/4" && ( a * 2 ).ToString() == "-3/2" && ( a / 3 ).ToString() == "-1/4" );
            ASSERT_TRUE( a < b && b > a && a <= a && a >= a && a != b && a == TBigFraction( -9, 12 ) );
            ASSERT_TRUE( ( a - a ).GetNumerator().IsZero() && ( a - a ).GetDenominator().IsOne() );
            ASSERT_TRUE( ( a * TBigFraction() ).GetDenominator().IsOne() );
            CORE::CBigInteger whole;
            TBigFraction part;
            ASSERT_TRUE( TBigFraction( 7, 3 ).ToMixedFraction( whole, part ) && whole == 2 && part.ToString() == "1/3" );
            TBigFraction mixed;
            mixed.FromMixedFraction( 2, TBigFraction( 1, 3 ) );
            ASSERT_TRUE( mixed.ToString() == "7/3" );
            bool threw = false;
            try
            {
                TBigFraction bad( 1, 0 );
            }
            catch ( const GUCEF::invalid_argument& )
            {
                threw = true;
            }
            ASSERT_TRUE( threw );
            threw = false;
            try
            {
                a /= TBigFraction();
            }
            catch ( const GUCEF::invalid_argument& )
            {
                threw = true;
            }
            ASSERT_TRUE( threw );
            TBigFraction tryResult( 5, 7 );
            ASSERT_FALSE( a.TryDivide( TBigFraction(), tryResult ) );
            ASSERT_TRUE( tryResult.ToString() == "5/7" );
            ASSERT_TRUE( a.TryAdd( b, tryResult ) && tryResult.ToString() == "-7/12" );

            // H(100) summed term by term and a telescoping product
            TBigFraction harmonic;
            for ( Int32 k=1; k<=100; ++k )
                harmonic += TBigFraction( 1, k );
            ASSERT_TRUE( harmonic.ToString() == "14466636279520351160221518043104131447711/2788815009188499086581352357412492142272" );
            TBigFraction product( 1 );
            for ( Int32 k=1; k<=500; ++k )
                product *= TBigFraction( k, k + 1 );
            ASSERT_TRUE( product.ToString() == "1/501" );

            // seeded full range Int64 operands: every operation and every comparison equals the Int64 fraction
            // (exact 128 bit intermediates) where that one fits, the comparisons always (T-F5 against big fractions)
            CSplitMix64 rng( g_testSeedFraction + 600 );
            UInt32 failures = 0;
            for ( UInt32 i=0; i<20000 && failures < 5; ++i )
            {
                Int64 n1 = (Int64) rng.Next() >> (UInt32) rng.Below( 63 );
                Int64 d1 = (Int64) ( rng.Next() >> ( 1 + (UInt32) rng.Below( 63 ) ) ) + 1;
                Int64 n2 = (Int64) rng.Next() >> (UInt32) rng.Below( 63 );
                Int64 d2 = (Int64) ( rng.Next() >> ( 1 + (UInt32) rng.Below( 63 ) ) ) + 1;
                if ( n1 == g_fractionInt64Min || n2 == g_fractionInt64Min )
                    continue;
                CORE::TInt64Fraction x( n1, d1 );
                CORE::TInt64Fraction y( n2, d2 );
                TBigFraction bx( n1, d1 );
                TBigFraction by( n2, d2 );
                bool ok = SameFraction( bx, x ) && SameFraction( by, y );
                ok = ok && ( bx < by ) == ( x < y ) && ( bx <= by ) == ( x <= y ) && ( bx > by ) == ( x > y );
                ok = ok && ( bx >= by ) == ( x >= y ) && ( bx == by ) == ( x == y ) && ( bx != by ) == ( x != y );
                CORE::TInt64Fraction r;
                if ( x.TryAdd( y, r ) ) ok = ok && SameFraction( bx + by, r );
                if ( x.TrySubtract( y, r ) ) ok = ok && SameFraction( bx - by, r );
                if ( x.TryMultiply( y, r ) ) ok = ok && SameFraction( bx * by, r );
                if ( 0 != n2 && x.TryDivide( y, r ) ) ok = ok && SameFraction( bx / by, r );
                // the big result is exact in every case: ( x + y ) - y == x
                ok = ok && ( bx + by ) - by == bx && ( 0 == n2 || ( bx * by ) / by == bx );
                if ( !ok )
                {
                    ++failures;
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "T-F6 big fraction mismatch at iteration " << i << " seed " << ( g_testSeedFraction + 600 ) );
                    ERRORHERE;
                }
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // T-F7: text of big integer fractions is parsed strictly; the built-in types keep their results
    GUCEF_TESTFW_TESTCASE( "T-F7: CTFraction< CBigInteger >::FromString is strict, built-in types keep their results" )
        try
        {
            TBigFraction f;
            ASSERT_TRUE( f.FromString( "123456789012345678901234567890/9876543210" ) );
            TBigFraction g;
            ASSERT_TRUE( g.FromString( f.ToString() ) && g == f );
            ASSERT_TRUE( f == TBigFraction( CORE::CBigInteger::Parse( "123456789012345678901234567890" ), CORE::CBigInteger( 9876543210LL ) ) );
            ASSERT_TRUE( f.FromString( "-6/-4" ) && f.ToString() == "3/2" );
            static const char* bad[] = { "abc/3", "1/0", "/3", "1/", "1/x", "12x/5", " 5/10" };
            for ( UInt32 i=0; i<sizeof( bad ) / sizeof( bad[ 0 ] ); ++i )
            {
                TBigFraction h( 9, 7 );
                ASSERT_FALSE( h.FromString( bad[ i ] ) );
                ASSERT_TRUE( h.ToString() == "9/7" );     // unchanged, not a silently zero numerator
            }

            // built-in types: recorded with the header before the text conversion was routed through the integer
            // operations policy (both versions compared by a probe): malformed numerator text reads as 0
            CORE::TInt32Fraction i32( 9, 7 );
            ASSERT_TRUE( i32.FromString( "abc/3" ) && i32.ToString() == "0/1" );
            i32 = CORE::TInt32Fraction( 9, 7 );
            ASSERT_TRUE( i32.FromString( "/3" ) && i32.ToString() == "0/1" );
            i32 = CORE::TInt32Fraction( 9, 7 );
            ASSERT_FALSE( i32.FromString( "1/" ) );
            ASSERT_TRUE( i32.ToString() == "9/7" );
            ASSERT_FALSE( i32.FromString( "1/0" ) );
            ASSERT_TRUE( i32.FromString( "12x/5" ) && i32.ToString() == "12/5" );
            CORE::TUInt32Fraction u32( 9, 7 );
            ASSERT_TRUE( u32.FromString( "2/-4" ) && u32.ToString() == "1/2147483646" );
            CORE::TInt64Fraction i64( 9, 7 );
            ASSERT_TRUE( i64.FromString( "abc/3" ) && i64.ToString() == "0/1" );
            ASSERT_TRUE( i64.FromString( "2/-4" ) && i64.ToString() == "-1/2" );

            // decimal conversions of big fractions
            CORE::CDecimal decimal;
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == TBigFraction( 2, 3 ).ToDecimal( CORE::CDecimalContext( 18, CORE::ROUNDINGMODE_HALF_EVEN ), decimal ) );
            ASSERT_TRUE( decimal.ToString() == "0.666666666666666667" );
            TBigFraction fromDecimal;
            ASSERT_TRUE( fromDecimal.FromDecimal( CORE::CDecimal::Parse( "-12.50" ) ) && fromDecimal.ToString() == "-25/2" );
            ASSERT_TRUE( fromDecimal.FromDecimal( CORE::CDecimal::Parse( "0.000000000000000001" ) ) && fromDecimal.ToString() == "1/1000000000000000000" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // T-F8: no heap use for values within 256 bits
    GUCEF_TESTFW_TESTCASE( "T-F8: CTFraction< CBigInteger > operations within 256 bits make no heap allocation" )
        GUCEF_TESTFW_SKIP_IF( !CORETEST::IsAllocationCountingAvailable(), CORETEST::GetAllocationCountingUnavailableReason() );
        try
        {
            TBigFraction x( CORE::CBigInteger::Parse( "-1267650600228229401496703205653" ), CORE::CBigInteger::Parse( "633825300114114700748351602943" ) );
            TBigFraction y( CORE::CBigInteger::Parse( "1180591620717411303423" ), CORE::CBigInteger::Parse( "1237940039285380274899124191" ) );
            TBigFraction r;
            CORETEST::StartAllocationCounting();
            r = x + y;
            r = x - y;
            r = x * y;
            r = x / y;
            r += 7;
            bool less = x < y;
            bool equal = x == y;
            Float64 asDouble = x.ToFloat64();
            UInt32 allocations = CORETEST::StopAllocationCounting();
            ASSERT_TRUE( 0 == allocations );
            ASSERT_TRUE( less && !equal && asDouble < -1.99 && asDouble > -2.01 );
            if ( 0 != allocations )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "T-F8 allocations: " << allocations << "\n" << CORETEST::GetCountedAllocationStacks() );

            // above 256 bits, comparing for equality copies nothing
            TBigFraction big( CORE::CBigInteger( 1 ) << 400, ( CORE::CBigInteger( 1 ) << 300 ) + 1 );
            TBigFraction other( big );
            CORETEST::StartAllocationCounting();
            bool same = big == other;
            allocations = CORETEST::StopAllocationCounting();
            ASSERT_TRUE( same && 0 == allocations );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // T-F10, second half: the correctly rounded conversion
    GUCEF_TESTFW_TESTCASE( "T-F10: CTFraction< CBigInteger >::ToFloat64 is correctly rounded" )
        try
        {
            TBigFraction f( 3706778661852469502LL, 239877 );
            ASSERT_TRUE( 15452830666768.676 == f.ToFloat64() );
            Float64 helper = 0.0;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INEXACT == CORE::CBigInteger::RatioToFloat64( 3706778661852469502LL, 239877, CORE::ROUNDINGMODE_HALF_EVEN, helper ) && 15452830666768.676 == helper );
            ASSERT_TRUE( 1.0f / 3.0f == TBigFraction( 1, 3 ).ToFloat32() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformFractionTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CTFraction TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "CTFraction" );

    // Test 1: Default construction (0/1)
    GUCEF_TESTFW_TESTCASE( "Test 1: Default construction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 1: Default construction" );
            CORE::TInt32Fraction f;
            ASSERT_TRUE( f.GetNumerator() == 0 );
            ASSERT_TRUE( f.GetDenominator() == 1 );
            ASSERT_TRUE( f.ToFloat32() == 0.0f );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 2: Basic parameterized construction - coprime values
    GUCEF_TESTFW_TESTCASE( "Test 2: Basic construction - coprime values" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 2: Basic construction - coprime values" );
            CORE::TInt32Fraction f( 1, 2 );
            ASSERT_TRUE( f.GetNumerator() == 1 );
            ASSERT_TRUE( f.GetDenominator() == 2 );

            CORE::TInt32Fraction g( 3, 7 );
            ASSERT_TRUE( g.GetNumerator() == 3 );
            ASSERT_TRUE( g.GetDenominator() == 7 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 3: Normalization by GCD reduction (positive values)
    GUCEF_TESTFW_TESTCASE( "Test 3: Normalization by GCD" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 3: Normalization by GCD" );
            // 6/4 -> GCD(6,4)=2 -> 3/2
            CORE::TInt32Fraction f( 6, 4 );
            ASSERT_TRUE( f.GetNumerator() == 3 );
            ASSERT_TRUE( f.GetDenominator() == 2 );

            // 9/3 -> GCD(9,3)=3 -> 3/1
            CORE::TInt32Fraction g( 9, 3 );
            ASSERT_TRUE( g.GetNumerator() == 3 );
            ASSERT_TRUE( g.GetDenominator() == 1 );

            // 2/6 -> GCD(2,6)=2 -> 1/3
            CORE::TInt32Fraction h( 2, 6 );
            ASSERT_TRUE( h.GetNumerator() == 1 );
            ASSERT_TRUE( h.GetDenominator() == 3 );

            // Already normalized: 1/1
            CORE::TInt32Fraction i( 4, 4 );
            ASSERT_TRUE( i.GetNumerator() == 1 );
            ASSERT_TRUE( i.GetDenominator() == 1 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 4: Zero denominator throws an exception
    GUCEF_TESTFW_TESTCASE( "Test 4: Zero denominator throws" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 4: Zero denominator throws" );
            bool exceptionThrown = false;
            try
            {
                CORE::TInt32Fraction f( 1, 0 );
            }
            catch( const GUCEF::invalid_argument& )
            {
                exceptionThrown = true;
            }
            catch( const std::invalid_argument& )
            {
                exceptionThrown = true;
            }
            catch( ... )
            {
                exceptionThrown = true;
            }
            ASSERT_TRUE( exceptionThrown );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 5: Unsigned type construction
    GUCEF_TESTFW_TESTCASE( "Test 5: Unsigned type construction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 5: Unsigned type construction" );
            CORE::TUInt32Fraction f( 3, 6 );  // GCD(3,6)=3 -> 1/2
            ASSERT_TRUE( f.GetNumerator() == 1 );
            ASSERT_TRUE( f.GetDenominator() == 2 );

            CORE::TUInt64UInt32Fraction g( 10, 4 );  // GCD(10,4)=2 -> 5/2
            ASSERT_TRUE( g.GetNumerator() == 5 );
            ASSERT_TRUE( g.GetDenominator() == 2 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 6: Construction from TInt32T2Fraction C-style struct
    GUCEF_TESTFW_TESTCASE( "Test 6: Construction from TInt32T2Fraction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 6: Construction from TInt32T2Fraction" );
            CORE::TInt32T2Fraction cs;
            cs.numerator = 3;
            cs.denominator = 4;
            CORE::TInt32Fraction f( cs );
            ASSERT_TRUE( f.GetNumerator() == 3 );
            ASSERT_TRUE( f.GetDenominator() == 4 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 7: Construction from TUInt32T2Fraction C-style struct
    GUCEF_TESTFW_TESTCASE( "Test 7: Construction from TUInt32T2Fraction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 7: Construction from TUInt32T2Fraction" );
            CORE::TUInt32T2Fraction cs;
            cs.numerator = 5;
            cs.denominator = 6;
            CORE::TUInt32Fraction f( cs );
            ASSERT_TRUE( f.GetNumerator() == 5 );
            ASSERT_TRUE( f.GetDenominator() == 6 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 8: Construction from TInt64WInt32Fraction C-style struct
    GUCEF_TESTFW_TESTCASE( "Test 8: Construction from TInt64WInt32Fraction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 8: Construction from TInt64WInt32Fraction" );
            CORE::TInt64WInt32Fraction cs;
            cs.numerator = 7;
            cs.denominator = 2;
            CORE::TInt64Int32Fraction f( cs );
            ASSERT_TRUE( f.GetNumerator() == 7 );
            ASSERT_TRUE( f.GetDenominator() == 2 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 9: Construction from TUInt64WUInt32Fraction C-style struct
    GUCEF_TESTFW_TESTCASE( "Test 9: Construction from TUInt64WUInt32Fraction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 9: Construction from TUInt64WUInt32Fraction" );
            CORE::TUInt64WUInt32Fraction cs;
            cs.numerator = 8;
            cs.denominator = 3;
            CORE::TUInt64UInt32Fraction f( cs );
            ASSERT_TRUE( f.GetNumerator() == 8 );
            ASSERT_TRUE( f.GetDenominator() == 3 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 10: Construction from TInt64T2Fraction C-style struct
    GUCEF_TESTFW_TESTCASE( "Test 10: Construction from TInt64T2Fraction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 10: Construction from TInt64T2Fraction" );
            CORE::TInt64T2Fraction cs;
            cs.numerator = 5;
            cs.denominator = 9;
            CORE::TInt64Fraction f( cs );
            ASSERT_TRUE( f.GetNumerator() == 5 );
            ASSERT_TRUE( f.GetDenominator() == 9 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 11: operator== and operator!=
    GUCEF_TESTFW_TESTCASE( "Test 11: operator== and operator!=" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 11: operator== and operator!=" );
            CORE::TInt32Fraction a( 1, 2 );
            CORE::TInt32Fraction b( 1, 2 );
            CORE::TInt32Fraction c( 2, 4 );  // normalizes to 1/2
            CORE::TInt32Fraction d( 1, 3 );

            ASSERT_TRUE( a == b );
            ASSERT_TRUE( a == c );  // both normalize to 1/2
            ASSERT_FALSE( a == d );
            ASSERT_TRUE( a != d );
            ASSERT_FALSE( a != b );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 12: Comparison operators <, <=, >, >=
    GUCEF_TESTFW_TESTCASE( "Test 12: Comparison operators" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 12: Comparison operators" );
            CORE::TInt32Fraction oneThird( 1, 3 );
            CORE::TInt32Fraction oneHalf( 1, 2 );
            CORE::TInt32Fraction twoThirds( 2, 3 );
            CORE::TInt32Fraction oneHalfDup( 1, 2 );

            ASSERT_TRUE( oneThird < oneHalf );
            ASSERT_TRUE( oneHalf < twoThirds );
            ASSERT_TRUE( oneThird < twoThirds );

            ASSERT_FALSE( oneHalf < oneThird );
            ASSERT_FALSE( oneHalf < oneHalf );

            ASSERT_TRUE( oneHalf <= oneHalf );
            ASSERT_TRUE( oneThird <= oneHalf );
            ASSERT_FALSE( oneHalf <= oneThird );

            ASSERT_TRUE( twoThirds > oneHalf );
            ASSERT_FALSE( oneThird > oneHalf );
            ASSERT_FALSE( oneHalf > oneHalf );

            ASSERT_TRUE( oneHalf >= oneHalf );
            ASSERT_TRUE( twoThirds >= oneHalf );
            ASSERT_FALSE( oneThird >= oneHalf );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 13: operator+= with fraction
    GUCEF_TESTFW_TESTCASE( "Test 13: operator+= with fraction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 13: operator+= with fraction" );
            // 1/4 + 1/4 = 2/4 = 1/2
            CORE::TInt32Fraction f( 1, 4 );
            f += CORE::TInt32Fraction( 1, 4 );
            ASSERT_TRUE( f == CORE::TInt32Fraction( 1, 2 ) );

            // 1/3 + 1/6 = 2/6 + 1/6 = 3/6 = 1/2
            CORE::TInt32Fraction g( 1, 3 );
            g += CORE::TInt32Fraction( 1, 6 );
            ASSERT_TRUE( g == CORE::TInt32Fraction( 1, 2 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 14: operator-= with fraction
    GUCEF_TESTFW_TESTCASE( "Test 14: operator-= with fraction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 14: operator-= with fraction" );
            // 3/4 - 1/4 = 2/4 = 1/2
            CORE::TInt32Fraction f( 3, 4 );
            f -= CORE::TInt32Fraction( 1, 4 );
            ASSERT_TRUE( f == CORE::TInt32Fraction( 1, 2 ) );

            // 1/1 - 1/2 = 2/2 - 1/2 = 1/2
            CORE::TInt32Fraction g( 1, 1 );
            g -= CORE::TInt32Fraction( 1, 2 );
            ASSERT_TRUE( g == CORE::TInt32Fraction( 1, 2 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 15: operator*= with fraction
    GUCEF_TESTFW_TESTCASE( "Test 15: operator*= with fraction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 15: operator*= with fraction" );
            // 2/3 * 3/4 = 6/12 = 1/2
            CORE::TInt32Fraction f( 2, 3 );
            f *= CORE::TInt32Fraction( 3, 4 );
            ASSERT_TRUE( f == CORE::TInt32Fraction( 1, 2 ) );

            // 3/5 * 5/3 = 15/15 = 1/1
            CORE::TInt32Fraction g( 3, 5 );
            g *= CORE::TInt32Fraction( 5, 3 );
            ASSERT_TRUE( g == CORE::TInt32Fraction( 1, 1 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 16: operator/= with fraction
    GUCEF_TESTFW_TESTCASE( "Test 16: operator/= with fraction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 16: operator/= with fraction" );
            // 1/2 / 1/4 = 1/2 * 4/1 = 4/2 = 2/1
            CORE::TInt32Fraction f( 1, 2 );
            f /= CORE::TInt32Fraction( 1, 4 );
            ASSERT_TRUE( f == CORE::TInt32Fraction( 2, 1 ) );

            // 2/3 / 4/3 = 2/3 * 3/4 = 6/12 = 1/2
            CORE::TInt32Fraction g( 2, 3 );
            g /= CORE::TInt32Fraction( 4, 3 );
            ASSERT_TRUE( g == CORE::TInt32Fraction( 1, 2 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 17: operator/= with fraction - division by zero fraction
    GUCEF_TESTFW_TESTCASE( "Test 17: Division by zero fraction throws" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 17: Division by zero fraction throws" );
            bool exceptionThrown = false;
            try
            {
                CORE::TInt32Fraction f( 1, 2 );
                f /= CORE::TInt32Fraction( 0, 1 );
            }
            catch( const GUCEF::invalid_argument& )
            {
                exceptionThrown = true;
            }
            catch( const std::invalid_argument& )
            {
                exceptionThrown = true;
            }
            catch( ... )
            {
                exceptionThrown = true;
            }
            ASSERT_TRUE( exceptionThrown );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 18: operator+= with scalar
    GUCEF_TESTFW_TESTCASE( "Test 18: operator+= with scalar" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 18: operator+= with scalar" );
            // 1/4 + 1 = 1/4 + 4/4 = 5/4
            CORE::TInt32Fraction f( 1, 4 );
            f += (Int32) 1;
            ASSERT_TRUE( f == CORE::TInt32Fraction( 5, 4 ) );

            // 1/3 + 2 = 1/3 + 6/3 = 7/3
            CORE::TInt32Fraction g( 1, 3 );
            g += (Int32) 2;
            ASSERT_TRUE( g == CORE::TInt32Fraction( 7, 3 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 19: operator-= with scalar
    GUCEF_TESTFW_TESTCASE( "Test 19: operator-= with scalar" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 19: operator-= with scalar" );
            // 5/4 - 1 = 5/4 - 4/4 = 1/4
            CORE::TInt32Fraction f( 5, 4 );
            f -= (Int32) 1;
            ASSERT_TRUE( f == CORE::TInt32Fraction( 1, 4 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 20: operator*= with scalar
    GUCEF_TESTFW_TESTCASE( "Test 20: operator*= with scalar" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 20: operator*= with scalar" );
            // 1/4 * 3 = 3/4
            CORE::TInt32Fraction f( 1, 4 );
            f *= (Int32) 3;
            ASSERT_TRUE( f == CORE::TInt32Fraction( 3, 4 ) );

            // 1/6 * 2 = 2/6 = 1/3
            CORE::TInt32Fraction g( 1, 6 );
            g *= (Int32) 2;
            ASSERT_TRUE( g == CORE::TInt32Fraction( 1, 3 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 21: operator/= with scalar
    GUCEF_TESTFW_TESTCASE( "Test 21: operator/= with scalar" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 21: operator/= with scalar" );
            // 3/4 / 3 = 3/(4*3) = 3/12 = 1/4
            CORE::TInt32Fraction f( 3, 4 );
            f /= (Int32) 3;
            ASSERT_TRUE( f == CORE::TInt32Fraction( 1, 4 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 22: operator/= with scalar zero throws
    GUCEF_TESTFW_TESTCASE( "Test 22: Scalar division by zero throws" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 22: Scalar division by zero throws" );
            bool exceptionThrown = false;
            try
            {
                CORE::TInt32Fraction f( 1, 2 );
                f /= (Int32) 0;
            }
            catch( const GUCEF::invalid_argument& )
            {
                exceptionThrown = true;
            }
            catch( const std::invalid_argument& )
            {
                exceptionThrown = true;
            }
            catch( ... )
            {
                exceptionThrown = true;
            }
            ASSERT_TRUE( exceptionThrown );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 23: Binary operator+
    GUCEF_TESTFW_TESTCASE( "Test 23: Binary operator+" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 23: Binary operator+" );
            // 1/4 + 1/4 = 1/2
            CORE::TInt32Fraction result = CORE::TInt32Fraction( 1, 4 ) + CORE::TInt32Fraction( 1, 4 );
            ASSERT_TRUE( result == CORE::TInt32Fraction( 1, 2 ) );

            // 1/3 + 2 = 7/3
            CORE::TInt32Fraction result2 = CORE::TInt32Fraction( 1, 3 ) + (Int32) 2;
            ASSERT_TRUE( result2 == CORE::TInt32Fraction( 7, 3 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 24: Binary operator-
    GUCEF_TESTFW_TESTCASE( "Test 24: Binary operator-" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 24: Binary operator-" );
            // 3/4 - 1/4 = 1/2
            CORE::TInt32Fraction result = CORE::TInt32Fraction( 3, 4 ) - CORE::TInt32Fraction( 1, 4 );
            ASSERT_TRUE( result == CORE::TInt32Fraction( 1, 2 ) );

            // 5/4 - 1 = 1/4
            CORE::TInt32Fraction result2 = CORE::TInt32Fraction( 5, 4 ) - (Int32) 1;
            ASSERT_TRUE( result2 == CORE::TInt32Fraction( 1, 4 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 25: Binary operator*
    GUCEF_TESTFW_TESTCASE( "Test 25: Binary operator*" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 25: Binary operator*" );
            // 1/2 * 1/3 = 1/6
            CORE::TInt32Fraction result = CORE::TInt32Fraction( 1, 2 ) * CORE::TInt32Fraction( 1, 3 );
            ASSERT_TRUE( result == CORE::TInt32Fraction( 1, 6 ) );

            // 1/4 * 2 = 2/4 = 1/2
            CORE::TInt32Fraction result2 = CORE::TInt32Fraction( 1, 4 ) * (Int32) 2;
            ASSERT_TRUE( result2 == CORE::TInt32Fraction( 1, 2 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 26: Binary operator/
    GUCEF_TESTFW_TESTCASE( "Test 26: Binary operator/" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 26: Binary operator/" );
            // 1/2 / 1/4 = 2
            CORE::TInt32Fraction result = CORE::TInt32Fraction( 1, 2 ) / CORE::TInt32Fraction( 1, 4 );
            ASSERT_TRUE( result == CORE::TInt32Fraction( 2, 1 ) );

            // 3/4 / 3 = 1/4
            CORE::TInt32Fraction result2 = CORE::TInt32Fraction( 3, 4 ) / (Int32) 3;
            ASSERT_TRUE( result2 == CORE::TInt32Fraction( 1, 4 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 27: ToFloat32 - exact fractions
    GUCEF_TESTFW_TESTCASE( "Test 27: ToFloat32" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 27: ToFloat32" );
            ASSERT_TRUE( CORE::TInt32Fraction( 1, 2 ).ToFloat32() == 0.5f );
            ASSERT_TRUE( CORE::TInt32Fraction( 1, 4 ).ToFloat32() == 0.25f );
            ASSERT_TRUE( CORE::TInt32Fraction( 3, 4 ).ToFloat32() == 0.75f );
            ASSERT_TRUE( CORE::TInt32Fraction( 1, 1 ).ToFloat32() == 1.0f );
            ASSERT_TRUE( CORE::TInt32Fraction( 0, 1 ).ToFloat32() == 0.0f );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 28: ToFloat64 - exact fractions
    GUCEF_TESTFW_TESTCASE( "Test 28: ToFloat64" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 28: ToFloat64" );
            ASSERT_TRUE( CORE::TInt32Fraction( 1, 2 ).ToFloat64() == 0.5 );
            ASSERT_TRUE( CORE::TInt32Fraction( 1, 4 ).ToFloat64() == 0.25 );
            ASSERT_TRUE( CORE::TInt32Fraction( 3, 4 ).ToFloat64() == 0.75 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 29: FromFloat32 and back
    GUCEF_TESTFW_TESTCASE( "Test 29: FromFloat32 round-trip" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 29: FromFloat32 round-trip" );
            CORE::TInt32Fraction f;
            f.FromFloat32( 0.5f );
            ASSERT_TRUE( f.ToFloat32() == 0.5f );

            CORE::TInt32Fraction g;
            g.FromFloat32( 0.25f );
            ASSERT_TRUE( g.ToFloat32() == 0.25f );

            CORE::TInt32Fraction h;
            h.FromFloat32( 0.75f );
            ASSERT_TRUE( h.ToFloat32() == 0.75f );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 30: ConvertFloat32ToFraction normalizes to exact fraction
    GUCEF_TESTFW_TESTCASE( "Test 30: ConvertFloat32ToFraction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 30: ConvertFloat32ToFraction" );
            // 0.5 * 1000000 = 500000 / 1000000 -> GCD -> 1/2
            CORE::TInt32Fraction f = CORE::TInt32Fraction::ConvertFloat32ToFraction( 0.5f );
            ASSERT_TRUE( f == CORE::TInt32Fraction( 1, 2 ) );

            // 0.25 * 1000000 = 250000 / 1000000 -> GCD -> 1/4
            CORE::TInt32Fraction g = CORE::TInt32Fraction::ConvertFloat32ToFraction( 0.25f );
            ASSERT_TRUE( g == CORE::TInt32Fraction( 1, 4 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 31: FromFloat64 and back
    GUCEF_TESTFW_TESTCASE( "Test 31: FromFloat64 round-trip" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 31: FromFloat64 round-trip" );
            CORE::TInt32Fraction f;
            f.FromFloat64( 0.5 );
            ASSERT_TRUE( f.ToFloat64() == 0.5 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 32: ToMixedFraction - proper fraction (< 1)
    GUCEF_TESTFW_TESTCASE( "Test 32: ToMixedFraction - proper fraction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 32: ToMixedFraction - proper fraction" );
            CORE::TInt32Fraction f( 3, 4 );
            Int32 wholePart = 99;
            CORE::TInt32Fraction fractionalPart;
            bool ok = f.ToMixedFraction( wholePart, fractionalPart );
            ASSERT_TRUE( ok );
            ASSERT_TRUE( wholePart == 0 );
            ASSERT_TRUE( fractionalPart == CORE::TInt32Fraction( 3, 4 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 33: ToMixedFraction - improper fraction (>= 1)
    GUCEF_TESTFW_TESTCASE( "Test 33: ToMixedFraction - improper fraction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 33: ToMixedFraction - improper fraction" );
            // 7/4 = 1 + 3/4
            CORE::TInt32Fraction f( 7, 4 );
            Int32 wholePart = 0;
            CORE::TInt32Fraction fractionalPart;
            bool ok = f.ToMixedFraction( wholePart, fractionalPart );
            ASSERT_TRUE( ok );
            ASSERT_TRUE( wholePart == 1 );
            ASSERT_TRUE( fractionalPart == CORE::TInt32Fraction( 3, 4 ) );

            // 9/3 = 3 (normalizes to 3/1) -> wholePart=3, fractional=0/1
            CORE::TInt32Fraction g( 9, 3 );
            Int32 wholePart2 = 0;
            CORE::TInt32Fraction frac2;
            g.ToMixedFraction( wholePart2, frac2 );
            ASSERT_TRUE( wholePart2 == 3 );
            ASSERT_TRUE( frac2 == CORE::TInt32Fraction( 0, 1 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 34: FromMixedFraction round-trip
    GUCEF_TESTFW_TESTCASE( "Test 34: FromMixedFraction round-trip" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 34: FromMixedFraction round-trip" );
            // 1 + 3/4 = 7/4
            CORE::TInt32Fraction f;
            f.FromMixedFraction( (Int32) 1, CORE::TInt32Fraction( 3, 4 ) );
            ASSERT_TRUE( f == CORE::TInt32Fraction( 7, 4 ) );

            // 0 + 1/2 = 1/2
            CORE::TInt32Fraction g;
            g.FromMixedFraction( (Int32) 0, CORE::TInt32Fraction( 1, 2 ) );
            ASSERT_TRUE( g == CORE::TInt32Fraction( 1, 2 ) );

            // 3 + 0/1 = 3/1
            CORE::TInt32Fraction h;
            h.FromMixedFraction( (Int32) 3, CORE::TInt32Fraction( 0, 1 ) );
            ASSERT_TRUE( h == CORE::TInt32Fraction( 3, 1 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 35: ToString
    GUCEF_TESTFW_TESTCASE( "Test 35: ToString" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 35: ToString" );
            ASSERT_TRUE( CORE::TInt32Fraction( 1, 2 ).ToString() == "1/2" );
            ASSERT_TRUE( CORE::TInt32Fraction( 3, 4 ).ToString() == "3/4" );
            ASSERT_TRUE( CORE::TInt32Fraction( 0, 1 ).ToString() == "0/1" );
            ASSERT_TRUE( CORE::TInt32Fraction( 1, 1 ).ToString() == "1/1" );

            // Normalized form: 6/4 -> 3/2
            ASSERT_TRUE( CORE::TInt32Fraction( 6, 4 ).ToString() == "3/2" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 36: FromString - valid input
    // NOTE: As of this writing, FromString has a bug where m_denominator is never assigned
    // from the parsed string (only checked for zero). The test below checks the intended
    // behavior; if it fails, the implementation needs to be fixed.
    GUCEF_TESTFW_TESTCASE( "Test 36: FromString - valid input" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 36: FromString - valid input" );
            CORE::TInt32Fraction f;
            bool ok = f.FromString( "3/4" );
            ASSERT_TRUE( ok );
            ASSERT_TRUE( f.GetNumerator() == 3 );
            ASSERT_TRUE( f.GetDenominator() == 4 );

            CORE::TInt32Fraction g;
            ok = g.FromString( "1/2" );
            ASSERT_TRUE( ok );
            ASSERT_TRUE( g.GetNumerator() == 1 );
            ASSERT_TRUE( g.GetDenominator() == 2 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 37: FromString - invalid input (no slash)
    GUCEF_TESTFW_TESTCASE( "Test 37: FromString - no slash returns false" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 37: FromString - no slash returns false" );
            CORE::TInt32Fraction f;
            ASSERT_FALSE( f.FromString( "34" ) );
            ASSERT_FALSE( f.FromString( "" ) );
            ASSERT_FALSE( f.FromString( "abc" ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 38: FromString - zero denominator returns false
    GUCEF_TESTFW_TESTCASE( "Test 38: FromString - zero denominator returns false" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 38: FromString - zero denominator returns false" );
            CORE::TInt32Fraction f;
            ASSERT_FALSE( f.FromString( "1/0" ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 39: ToString/FromString round-trip
    // NOTE: Depends on FromString being correctly implemented (see Test 36 note).
    GUCEF_TESTFW_TESTCASE( "Test 39: ToString/FromString round-trip" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 39: ToString/FromString round-trip" );
            CORE::TInt32Fraction original( 5, 7 );
            CORE::CString str = original.ToString();

            CORE::TInt32Fraction parsed;
            ASSERT_TRUE( parsed.FromString( str ) );
            ASSERT_TRUE( parsed == original );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 40: Multiple typedef instantiations
    GUCEF_TESTFW_TESTCASE( "Test 40: Multiple typedef instantiations" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 40: Multiple typedef instantiations" );
            CORE::TInt8Fraction   i8f( 1, 4 );
            ASSERT_TRUE( i8f.GetNumerator() == 1 && i8f.GetDenominator() == 4 );

            CORE::TUInt8Fraction  u8f( 3, 6 );
            ASSERT_TRUE( u8f.GetNumerator() == 1 && u8f.GetDenominator() == 2 );

            CORE::TInt16Fraction  i16f( 5, 10 );
            ASSERT_TRUE( i16f.GetNumerator() == 1 && i16f.GetDenominator() == 2 );

            CORE::TUInt16Fraction u16f( 7, 3 );
            ASSERT_TRUE( u16f.GetNumerator() == 7 && u16f.GetDenominator() == 3 );

            CORE::TInt64Fraction  i64f( 100, 25 );
            ASSERT_TRUE( i64f.GetNumerator() == 4 && i64f.GetDenominator() == 1 );

            CORE::TUInt64Fraction u64f( 8, 4 );
            ASSERT_TRUE( u64f.GetNumerator() == 2 && u64f.GetDenominator() == 1 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 41: Identity operations
    GUCEF_TESTFW_TESTCASE( "Test 41: Identity operations" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 41: Identity operations" );
            CORE::TInt32Fraction f( 3, 7 );

            // Adding zero fraction
            CORE::TInt32Fraction result = f + CORE::TInt32Fraction( 0, 1 );
            ASSERT_TRUE( result == f );

            // Multiplying by 1/1
            result = f * CORE::TInt32Fraction( 1, 1 );
            ASSERT_TRUE( result == f );

            // Dividing by 1/1
            result = f / CORE::TInt32Fraction( 1, 1 );
            ASSERT_TRUE( result == f );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // T-F2: the normalisation equals a plain Euclid oracle on every compiler, for several types
    GUCEF_TESTFW_TESTCASE( "T-F2: Normalize equals the Euclid oracle for Int8, Int32, UInt32, Int64 and UInt64" )
        try
        {
            ASSERT_TRUE( CheckNormalizeAgainstOracle< Int8 >( g_testSeedFraction ) );
            ASSERT_TRUE( CheckNormalizeAgainstOracle< Int32 >( g_testSeedFraction + 1 ) );
            ASSERT_TRUE( CheckNormalizeAgainstOracle< UInt32 >( g_testSeedFraction + 2 ) );
            ASSERT_TRUE( CheckNormalizeAgainstOracle< Int64 >( g_testSeedFraction + 3 ) );
            ASSERT_TRUE( CheckNormalizeAgainstOracle< UInt64 >( g_testSeedFraction + 4 ) );

            CORE::TInt32Fraction zero( 0, -5 );
            ASSERT_TRUE( 0 == zero.GetNumerator() && 1 == zero.GetDenominator() );
            CORE::TInt32Fraction flipped( 6, -4 );
            ASSERT_TRUE( -3 == flipped.GetNumerator() && 2 == flipped.GetDenominator() );
            CORE::TInt32Fraction both( -6, -4 );
            ASSERT_TRUE( 3 == both.GetNumerator() && 2 == both.GetDenominator() );
            CORE::TInt64Fraction zero64( 0, 77 );
            ASSERT_TRUE( 0 == zero64.GetNumerator() && 1 == zero64.GetDenominator() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // T-F3: INT64_MIN handling, defined and without UB
    GUCEF_TESTFW_TESTCASE( "T-F3: INT64_MIN handling" )
        try
        {
            CORE::TInt64Fraction a( g_fractionInt64Min, 2 );
            ASSERT_TRUE( -4611686018427387904LL == a.GetNumerator() && 1 == a.GetDenominator() );
            CORE::TInt64Fraction b( g_fractionInt64Min, 1 );
            ASSERT_TRUE( g_fractionInt64Min == b.GetNumerator() && 1 == b.GetDenominator() );
            CORE::TInt64Fraction c( 2, g_fractionInt64Min );
            ASSERT_TRUE( -1 == c.GetNumerator() && 4611686018427387904LL == c.GetDenominator() );
            CORE::TInt64Fraction d( g_fractionInt64Min, g_fractionInt64Min );
            ASSERT_TRUE( 1 == d.GetNumerator() && 1 == d.GetDenominator() );
            CORE::TInt64Fraction e( 0, g_fractionInt64Min );
            ASSERT_TRUE( 0 == e.GetNumerator() && 1 == e.GetDenominator() );

            // ( 1, INT64_MIN ) is -1 / 2^63: the denominator 2^63 is not representable
            bool thrown = false;
            try
            {
                CORE::TInt64Fraction f( 1, g_fractionInt64Min );
            }
            catch ( const std::overflow_error& )
            {
                thrown = true;
            }
            ASSERT_TRUE( thrown );

            // ( INT64_MIN, -1 ) is 2^63: the numerator is not representable
            thrown = false;
            try
            {
                CORE::TInt64Fraction g( g_fractionInt64Min, -1 );
            }
            catch ( const std::overflow_error& )
            {
                thrown = true;
            }
            ASSERT_TRUE( thrown );

            // arithmetic that lands on the minimum is fine
            CORE::TInt64Fraction minusHalf( -1, 2 );
            CORE::TInt64Fraction product = CORE::TInt64Fraction( g_fractionInt64Min / 2, 1 ) * CORE::TInt64Fraction( 2, 1 );
            ASSERT_TRUE( g_fractionInt64Min == product.GetNumerator() && 1 == product.GetDenominator() );
            ASSERT_TRUE( minusHalf < CORE::TInt64Fraction( 0, 1 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // T-F4: Int64 cross product overflow in arithmetic
    GUCEF_TESTFW_TESTCASE( "T-F4: Int64 arithmetic with 128 bit intermediates" )
        try
        {
            CORE::TInt64Fraction tiny( 1, 3037000500LL );
            CORE::TInt64Fraction sum = tiny + tiny;
            ASSERT_TRUE( 1 == sum.GetNumerator() && 1518500250LL == sum.GetDenominator() );

            CORE::TInt64Fraction p = CORE::TInt64Fraction( 3037000499LL, 3037000500LL ) * CORE::TInt64Fraction( 3037000500LL, 3037000499LL );
            ASSERT_TRUE( 1 == p.GetNumerator() && 1 == p.GetDenominator() );

            // 1/3037000500 + 1/3037000501: consecutive denominators are coprime, the denominator 3037000500 * 3037000501
            // is 9223372040037250500 > 2^63 - 1 and nothing cancels
            CORE::TInt64Fraction x( 1, 3037000500LL );
            CORE::TInt64Fraction y( 1, 3037000501LL );
            CORE::TInt64Fraction result( 7, 1 );
            ASSERT_FALSE( x.TryAdd( y, result ) );
            ASSERT_TRUE( 7 == result.GetNumerator() && 1 == result.GetDenominator() );
            bool thrown = false;
            try
            {
                CORE::TInt64Fraction z = x + y;
            }
            catch ( const std::overflow_error& )
            {
                thrown = true;
            }
            ASSERT_TRUE( thrown );
            thrown = false;
            try
            {
                CORE::TInt64Fraction z = x;
                z += y;
            }
            catch ( const std::overflow_error& )
            {
                thrown = true;
            }
            ASSERT_TRUE( thrown );

            ASSERT_TRUE( x.TryAdd( x, result ) && 1 == result.GetNumerator() && 1518500250LL == result.GetDenominator() );
            ASSERT_TRUE( x.TrySubtract( x, result ) && 0 == result.GetNumerator() && 1 == result.GetDenominator() );
            ASSERT_TRUE( x.TryMultiply( y, result ) == false );
            ASSERT_TRUE( x.TryDivide( y, result ) && 3037000501LL == result.GetNumerator() && 3037000500LL == result.GetDenominator() );
            ASSERT_FALSE( x.TryDivide( CORE::TInt64Fraction( 0, 1 ), result ) );

            // the scalar operators and the mixed fraction go through the same exact path
            CORE::TInt64Fraction s( 1, 3037000500LL );
            s *= 3037000500LL;
            ASSERT_TRUE( 1 == s.GetNumerator() && 1 == s.GetDenominator() );
            // 1 / 3037000501^2: the denominator 9223372043074251001 exceeds 2^63 - 1 and nothing cancels
            CORE::TInt64Fraction t( 1, 3037000501LL );
            thrown = false;
            try
            {
                t /= 3037000501LL;
            }
            catch ( const std::overflow_error& )
            {
                thrown = true;
            }
            ASSERT_TRUE( thrown );
            ASSERT_TRUE( 1 == t.GetNumerator() && 3037000501LL == t.GetDenominator() );
            t = CORE::TInt64Fraction( 3037000500LL, 7 );
            t /= 3037000500LL;
            ASSERT_TRUE( 1 == t.GetNumerator() && 7 == t.GetDenominator() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "T-F4: Int64 arithmetic against the exact reference, 20000 seeded cases per operation" )
        try
        {
            CSplitMix64 rng( g_testSeedFraction + 10 );
            UInt32 overflows = 0;
            UInt32 fitting = 0;
            for ( UInt32 i=0; i<20000; ++i )
            {
                CORE::TInt64Fraction a = RandomInt64Fraction( rng );
                CORE::TInt64Fraction b = RandomInt64Fraction( rng );
                for ( UInt32 op=0; op<4; ++op )
                {
                    if ( 3 == op && 0 == b.GetNumerator() )
                        continue;
                    SRefFraction expected = RefOperation( op, a, b );
                    bool fits = RefFitsInt64( expected.n ) && RefFitsInt64( expected.d );
                    CORE::TInt64Fraction result( 9, 1 );
                    bool ok = false;
                    switch ( op )
                    {
                        case 0: ok = a.TryAdd( b, result ); break;
                        case 1: ok = a.TrySubtract( b, result ); break;
                        case 2: ok = a.TryMultiply( b, result ); break;
                        default: ok = a.TryDivide( b, result ); break;
                    }
                    bool good = ok == fits;
                    if ( fits )
                    {
                        ++fitting;
                        good = good && 0 == CRefInt::Compare( CRefInt::FromInt64( result.GetNumerator() ), expected.n )
                                    && 0 == CRefInt::Compare( CRefInt::FromInt64( result.GetDenominator() ), expected.d );
                    }
                    else
                    {
                        ++overflows;
                        good = good && 9 == result.GetNumerator() && 1 == result.GetDenominator();
                    }
                    if ( good && 0 == i % 10 )
                    {
                        // the operator form throws exactly when the exact result does not fit
                        bool thrown = false;
                        CORE::TInt64Fraction viaOperator;
                        try
                        {
                            switch ( op )
                            {
                                case 0: viaOperator = a + b; break;
                                case 1: viaOperator = a - b; break;
                                case 2: viaOperator = a * b; break;
                                default: viaOperator = a / b; break;
                            }
                        }
                        catch ( const std::overflow_error& )
                        {
                            thrown = true;
                        }
                        good = thrown == !fits && ( thrown || viaOperator == result );
                    }
                    if ( !good )
                    {
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Int64 fraction arithmetic mismatch at iteration " << i << " operation " << op << " seed " << ( g_testSeedFraction + 10 ) );
                        ERRORHERE;
                        i = 20000;
                        break;
                    }
                }
            }
            // the generator must exercise both outcomes or the test proves nothing
            ASSERT_TRUE( overflows > 100 && fitting > 1000 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // T-F5: Int64 comparison overflow
    GUCEF_TESTFW_TESTCASE( "T-F5: Int64 comparison is exact" )
        try
        {
            CORE::TInt64Fraction lo( 4611686018427387903LL, 4611686018427387904LL );
            CORE::TInt64Fraction hi( 4611686018427387904LL, 4611686018427387905LL );
            ASSERT_TRUE( lo < hi && lo <= hi && hi > lo && hi >= lo && !( hi < lo ) && !( lo >= hi ) );
            ASSERT_TRUE( lo <= lo && lo >= lo && !( lo < lo ) && !( lo > lo ) && lo == lo );

            CSplitMix64 rng( g_testSeedFraction + 20 );
            for ( UInt32 i=0; i<100000; ++i )
            {
                CORE::TInt64Fraction a = RandomInt64Fraction( rng );
                CORE::TInt64Fraction b = RandomInt64Fraction( rng );
                Int32 c = CRefInt::Compare( CRefInt::Multiply( CRefInt::FromInt64( a.GetNumerator() ), CRefInt::FromInt64( b.GetDenominator() ) ),
                                            CRefInt::Multiply( CRefInt::FromInt64( b.GetNumerator() ), CRefInt::FromInt64( a.GetDenominator() ) ) );
                bool good = ( a < b ) == ( c < 0 ) && ( a <= b ) == ( c <= 0 ) && ( a > b ) == ( c > 0 ) && ( a >= b ) == ( c >= 0 )
                         && ( a == b ) == ( 0 == c ) && ( a != b ) == ( 0 != c );
                if ( !good )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Int64 fraction comparison mismatch at iteration " << i << " seed " << ( g_testSeedFraction + 20 ) );
                    ERRORHERE;
                    break;
                }
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // T-F9: the mixed type narrowing suspicion is pinned, not changed
    GUCEF_TESTFW_TESTCASE( "T-F9: TInt64Int32Fraction division by 2^33 pins the existing narrowing" )
        try
        {
            CORE::TInt64Int32Fraction f( 1, 2 );
            CORE::TInt64Int32Fraction divisor( 8589934592LL, 1 );
            ASSERT_TRUE( 8589934592LL == divisor.GetNumerator() && 1 == divisor.GetDenominator() );
            bool outcomeThrew = false;
            Int64 gotNumerator = 0;
            Int32 gotDenominator = 0;
            try
            {
                f /= divisor;
                gotNumerator = f.GetNumerator();
                gotDenominator = f.GetDenominator();
            }
            catch ( ... )
            {
                outcomeThrew = true;
            }
            // recorded before the change (the pre-change header gives the same): the Int32 denominator 2 * 2^33
            // wraps to 0, no exception is thrown and the fraction is left as 1/0. No behaviour change is made here.
            ASSERT_FALSE( outcomeThrew );
            ASSERT_TRUE( 1 == gotNumerator && 0 == gotDenominator );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // T-F10: the float conversion is not correctly rounded: existing behaviour pinned
    GUCEF_TESTFW_TESTCASE( "T-F10: ToFloat64 divides the separately rounded numerator, existing behaviour pinned" )
        try
        {
            CORE::TInt64Fraction f( 3706778661852469502LL, 239877 );
            // the exact quotient rounds to 15452830666768.676 (checked offline with exact rational arithmetic),
            // the existing conversion rounds numerator and denominator separately and yields ...674
            ASSERT_TRUE( 3706778661852469502LL == f.GetNumerator() && 239877 == f.GetDenominator() );
            ASSERT_TRUE( 15452830666768.674 == f.ToFloat64() );
            ASSERT_FALSE( 15452830666768.676 == f.ToFloat64() );
            ASSERT_TRUE( 15452830666768.674 != 15452830666768.676 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // T-F11 is the unchanged TestVariant test 54, TestDecimalVariant fraction cases and the cases above: no edit of those files.

    // T-F12: the Int64 specialization is the same instantiation in every translation unit
    GUCEF_TESTFW_TESTCASE( "T-F12: Int64 fraction arithmetic from a unit that includes only the fraction header" )
        try
        {
            // this unit and the CVariant implementation in the library both instantiate TInt64Fraction: a duplicate
            // definition would fail at link time, and both must compute the same exact results
            CORE::TInt64Fraction a( 1, 3037000500LL );
            CORE::TInt64Fraction b = a + a;
            ASSERT_TRUE( 1 == b.GetNumerator() && 1518500250LL == b.GetDenominator() );
            ASSERT_TRUE( b.ToString() == "1/1518500250" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // H5: decimal conversions
    GUCEF_TESTFW_TESTCASE( "H5: fraction to and from decimal" )
        try
        {
            CORE::CDecimal out;
            CORE::TInt64Fraction third( 1, 3 );
            CORE::TDecimalStatus status = third.ToDecimal( CORE::CDecimalContext( 4, CORE::ROUNDINGMODE_HALF_EVEN ), out );
            ASSERT_TRUE( ( status & ~(CORE::TDecimalStatus) CORE::DECIMALSTATUS_INEXACT ) == 0 );
            ASSERT_TRUE( 3333 == out.GetUnscaledValue() && 4 == out.GetScale() );
            CORE::TInt64Fraction minusTwoThirds( -2, 3 );
            status = minusTwoThirds.ToDecimal( CORE::CDecimalContext( 2, CORE::ROUNDINGMODE_HALF_EVEN ), out );
            ASSERT_TRUE( -67 == out.GetUnscaledValue() && 2 == out.GetScale() );

            CORE::TInt64Fraction fromDecimal;
            ASSERT_TRUE( fromDecimal.FromDecimal( CORE::CDecimal::Parse( "0.0825" ) ) );
            ASSERT_TRUE( 33 == fromDecimal.GetNumerator() && 400 == fromDecimal.GetDenominator() );
            ASSERT_TRUE( fromDecimal.FromDecimal( CORE::CDecimal::Parse( "-12.50" ) ) );
            ASSERT_TRUE( -25 == fromDecimal.GetNumerator() && 2 == fromDecimal.GetDenominator() );
            ASSERT_TRUE( fromDecimal.FromDecimal( CORE::CDecimal::Parse( "0.000000000000000001" ) ) );
            ASSERT_TRUE( 1 == fromDecimal.GetNumerator() && 1000000000000000000LL == fromDecimal.GetDenominator() );
            CORE::TInt32Fraction narrow;
            ASSERT_FALSE( narrow.FromDecimal( CORE::CDecimal::Parse( "0.000000000000000001" ) ) );
            ASSERT_TRUE( narrow.FromDecimal( CORE::CDecimal::Parse( "1.5" ) ) && 3 == narrow.GetNumerator() && 2 == narrow.GetDenominator() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    PerformBigIntegerFractionTests();

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL CTFraction TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
