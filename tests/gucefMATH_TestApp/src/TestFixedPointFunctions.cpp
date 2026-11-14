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
#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_MATH_FIXEDPOINT_H
#include "gucefMATH_fixedpoint.h"
#define GUCEF_MATH_FIXEDPOINT_H
#endif /* GUCEF_MATH_FIXEDPOINT_H ? */

#ifndef GUCEF_MATH_CTMATHCONSTANTS_H
#include "gucefMATH_CTMathConstants.h"
#define GUCEF_MATH_CTMATHCONSTANTS_H
#endif /* GUCEF_MATH_CTMATHCONSTANTS_H ? */

#ifndef GUCEF_CORE_CBIGINTEGER_H
#include "gucefCORE_CBigInteger.h"
#define GUCEF_CORE_CBIGINTEGER_H
#endif /* GUCEF_CORE_CBIGINTEGER_H ? */

#include "TestMathReference.h"
#include "TestFixedPointFunctions.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::MATH;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/


/**
 *  Independent reference for the Q16.16 square root: the floor root of v = raw * 2^16 (< 2^47) from the Float64 root
 *  corrected with exact 64 bit integer checks, then rounded to nearest (v - s^2 > s). No code shared with the kernel.
 */
static Int64
ReferenceSqrtQ16( Int64 raw )
{
    UInt64 v = (UInt64) raw << 16;
    UInt64 s = (UInt64) sqrt( (Float64) v );
    while ( s * s > v )
        --s;
    while ( ( s + 1 ) * ( s + 1 ) <= v )
        ++s;
    if ( v - s * s > s )
        ++s;
    return (Int64) s;
}

/*-------------------------------------------------------------------------*/

/** Accuracy limits in raw units per format, section 3.6.5 (Int32 storage: Q16.16 column; Int64 storage: Q32.32 column) */
struct SAccuracyTargets
{
    Float64 sinCos;
    Float64 atan2;
    Float64 exp2Log2;
};

/*-------------------------------------------------------------------------*/

/**
 *  Maximum error in raw units of a fixed point function against the Float64 reference over the sweep, after
 *  subtracting the uncertainty of the reference itself (one unit in the last place of the Float64 result, in raw units).
 *  Returns the excess over 'limit' (<= 0 means within the target) and reports the worst input.
 */
class CErrorTracker
{
    public:

    CErrorTracker( const char* name, Float64 limit )
        : m_name( name ), m_limit( limit ), m_worstExcess( -1e300 ), m_worstError( 0 ), m_worstInput( 0 )
    {
    }

    void Add( Int64 resultRaw, Float64 reference, UInt32 fractionBits, Float64 input, Float64 extraTolerance )
    {
        Float64 scale = ldexp( 1.0, (int) fractionBits );
        Float64 error = fabs( (Float64) resultRaw - reference * scale );
        Float64 referenceUncertainty = fabs( reference ) * scale * ldexp( 1.0, -52 );
        Float64 excess = error - referenceUncertainty - extraTolerance - m_limit;
        if ( excess > m_worstExcess )
        {
            m_worstExcess = excess;
            m_worstError = error;
            m_worstInput = input;
        }
    }

    bool IsWithin( UInt64 seed ) const
    {
        if ( m_worstExcess <= 0 )
            return true;
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FixedPointFunctions: seed " << seed << " " << m_name << " error " << m_worstError << " raw units at input " << m_worstInput << " exceeds " << m_limit );
        return false;
    }

    private:

    const char* m_name;
    Float64 m_limit;
    Float64 m_worstExcess;
    Float64 m_worstError;
    Float64 m_worstInput;
};

/*-------------------------------------------------------------------------*/

/** 2^16 seeded inputs per function in [-8 pi, 8 pi] and in the valid domains of the other functions */
template < typename TStorage, UInt32 F >
static bool
AccuracyWithinTargets( const SAccuracyTargets& targets, Float64 exp2Max, UInt64 seed )
{
    typedef CTFixedPoint< TStorage, F > T;
    typedef CTNumericTraits< T > TT;
    const Float64 pi = 3.14159265358979323846;
    MathReference::CSeededSequence generator( seed );
    CErrorTracker sinError( "sin", targets.sinCos ), cosError( "cos", targets.sinCos ), atan2Error( "atan2", targets.atan2 );
    CErrorTracker asinError( "asin", targets.atan2 ), acosError( "acos", targets.atan2 ), atanError( "atan", targets.atan2 );
    CErrorTracker exp2Error( "exp2", targets.exp2Log2 ), log2Error( "log2", targets.exp2Log2 );
    CErrorTracker expError( "exp", targets.exp2Log2 ), logError( "log", targets.exp2Log2 );
    CErrorTracker tanError( "tan", targets.exp2Log2 ), powError( "pow", targets.exp2Log2 );
    CErrorTracker pythagoras( "sin^2 + cos^2 - 1", 4 );
    for ( UInt32 i=0; i<65536; ++i )
    {
        T x( generator.NextFloat64( -8.0 * pi, 8.0 * pi ) );
        Float64 xd = x.ToFloat64();
        T s = TT::Sin( x );
        T c = TT::Cos( x );
        sinError.Add( s.GetRaw(), sin( xd ), F, xd, 0 );
        cosError.Add( c.GetRaw(), cos( xd ), F, xd, 0 );
        pythagoras.Add( ( s * s + c * c ).GetRaw(), 1.0, F, xd, 0 );

        T y( generator.NextFloat64( -8.0 * pi, 8.0 * pi ) );
        atan2Error.Add( TT::Atan2( y, x ).GetRaw(), atan2( y.ToFloat64(), xd ), F, xd, 0 );
        atanError.Add( TT::Atan( y ).GetRaw(), atan( y.ToFloat64() ), F, y.ToFloat64(), 0 );

        T u( generator.NextFloat64( -1.0, 1.0 ) );
        asinError.Add( TT::Asin( u ).GetRaw(), asin( u.ToFloat64() ), F, u.ToFloat64(), 0 );
        acosError.Add( TT::Acos( u ).GetRaw(), acos( u.ToFloat64() ), F, u.ToFloat64(), 0 );

        // exp2 and exp over their non-saturating ranges
        T e( generator.NextFloat64( -(Float64) F, exp2Max ) );
        exp2Error.Add( TT::Exp2( e ).GetRaw(), pow( 2.0, e.ToFloat64() ), F, e.ToFloat64(), 0 );
        T ex( generator.NextFloat64( -(Float64) F * 0.69, exp2Max * 0.69 ) );
        expError.Add( TT::Exp( ex ).GetRaw(), exp( ex.ToFloat64() ), F, ex.ToFloat64(), 0 );

        // log2 over positive values of every magnitude: a uniformly chosen bit length of the raw value
        Int64 raw = generator.NextSignedWithRandomLength( sizeof( TStorage ) * 8 );
        T positive = T::FromRaw( (TStorage) ( raw < 0 ? -raw : raw ) );
        log2Error.Add( TT::Log2( positive ).GetRaw(), log( positive.ToFloat64() ) / log( 2.0 ), F, positive.ToFloat64(), 0 );
        logError.Add( TT::Log( positive ).GetRaw(), log( positive.ToFloat64() ), F, positive.ToFloat64(), 0 );

        // tan away from the poles; pow of moderate values
        T t( generator.NextFloat64( -1.5, 1.5 ) );
        Float64 tanReference = tan( t.ToFloat64() );
        tanError.Add( TT::Tan( t ).GetRaw(), tanReference, F, t.ToFloat64(), 0 );
        T base( generator.NextFloat64( 0.125, 16.0 ) );
        T exponent( generator.NextFloat64( -3.0, 3.0 ) );
        Float64 powReference = pow( base.ToFloat64(), exponent.ToFloat64() );
        powError.Add( TT::Pow( base, exponent ).GetRaw(), powReference, F, base.ToFloat64(), 0 );
    }
    // every tracker reports its own failure, so all of them are evaluated
    UInt32 failures = 0;
    CErrorTracker* trackers[ 13 ] = { &sinError, &cosError, &atan2Error, &atanError, &asinError, &acosError, &exp2Error,
                                      &log2Error, &expError, &logError, &tanError, &powError, &pythagoras };
    for ( UInt32 t=0; t<13; ++t )
        if ( !trackers[ t ]->IsWithin( seed ) )
            ++failures;
    return 0 == failures;
}

/*-------------------------------------------------------------------------*/

/** Exact special values that the design pins */
template < typename TStorage, UInt32 F >
static bool
SpecialValuesHold( void )
{
    typedef CTFixedPoint< TStorage, F > T;
    typedef CTNumericTraits< T > TT;
    bool ok = TT::Sin( T() ) == T() && TT::Cos( T() ) == T( 1 );
    ok = ok && TT::Atan2( T(), T( -1 ) ) == TT::Pi() && TT::Atan2( T(), T() ) == T();
    ok = ok && TT::Atan2( T( 1 ), T() ) == TT::HalfPi() && TT::Atan2( T( -1 ), T() ) == -TT::HalfPi();
    ok = ok && TT::Atan2( T(), T( 5 ) ) == T() && TT::Atan( T( 1 ) ) == CTMathConstants< T >::QuarterPi();
    ok = ok && TT::Asin( T( 1 ) ) == TT::HalfPi() && TT::Acos( T( -1 ) ) == TT::Pi() && TT::Acos( T( 1 ) ) == T();
    ok = ok && TT::Exp2( T( 3 ) ) == T( 8 ) && TT::Exp2( T( -2 ) ) == T( 0.25 ) && TT::Exp2( T() ) == T( 1 );
    ok = ok && TT::Log2( T( 8 ) ) == T( 3 ) && TT::Log2( T( 0.25 ) ) == T( -2 ) && TT::Log2( T( 1 ) ) == T();
    ok = ok && TT::Log( T( 1 ) ) == T() && TT::Exp( T() ) == T( 1 ) && TT::Exp( T( 1 ) ) == TT::E();
    ok = ok && TT::Pow( T( 2 ), T( 10 ) ) == T( 1024 ) && TT::Pow( T( 9 ), T( 0.5 ) ) == T( 3 ) && TT::Pow( T( 5 ), T() ) == T( 1 );
    ok = ok && TT::Pow( T(), T( 2 ) ) == T() && TT::Pow( T( -2 ), T( 2 ) ) == T();
    ok = ok && TT::Log2( T() ) == TT::Lowest() && TT::Log( T( -1 ) ) == TT::Lowest();
    ok = ok && TT::Exp2( T( (Int32) ( sizeof( TStorage ) * 8 - F ) ) ) == TT::Max() && TT::Exp2( T( -100 ) ) == T();
    ok = ok && TT::Tan( T() ) == T() && TT::Tan( CTMathConstants< T >::QuarterPi() ) == T( 1 );
    ok = ok && TT::Sqrt( T( 4 ) ) == T( 2 ) && TT::Sqrt( T() ) == T() && TT::Sqrt( TT::Max() ).GetRaw() > 0;
    T out( 5 );
    ok = ok && MATHSTATUS_INVALID_OPERAND == FixedTrySqrt( T( -1 ), out ) && T() == out;
    ok = ok && TT::InverseSqrt( T( 4 ) ) == T( 0.5 );
    return ok;
}

/*-------------------------------------------------------------------------*/

/** FNV-1a 64 of the raw outputs of sqrt, sin, cos, atan2, exp2 and log2 over a fixed seeded sweep */
template < typename TStorage, UInt32 F >
static UInt64
DeterminismHash( Float64 exp2Max )
{
    typedef CTFixedPoint< TStorage, F > T;
    typedef CTNumericTraits< T > TT;
    MathReference::CSeededSequence generator( 0x5EED0F1C5EDULL );
    UInt64 hash = MathReference::Fnv1aOffsetBasis();
    for ( UInt32 i=0; i<4096; ++i )
    {
        T x = T::FromRaw( (TStorage) generator.NextSignedWithRandomLength( sizeof( TStorage ) * 8 ) );
        T y = T::FromRaw( (TStorage) generator.NextSignedWithRandomLength( sizeof( TStorage ) * 8 ) );
        T e( generator.NextFloat64( -(Float64) F, exp2Max ) );
        T magnitude = x.GetRaw() < 0 ? -x : x;
        hash = MathReference::Fnv1aInt64( (Int64) TT::Sqrt( magnitude ).GetRaw(), hash );
        hash = MathReference::Fnv1aInt64( (Int64) TT::Sin( x ).GetRaw(), hash );
        hash = MathReference::Fnv1aInt64( (Int64) TT::Cos( x ).GetRaw(), hash );
        hash = MathReference::Fnv1aInt64( (Int64) TT::Atan2( y, x ).GetRaw(), hash );
        hash = MathReference::Fnv1aInt64( (Int64) TT::Exp2( e ).GetRaw(), hash );
        hash = MathReference::Fnv1aInt64( (Int64) TT::Log2( magnitude ).GetRaw(), hash );
    }
    return hash;
}

/*-------------------------------------------------------------------------*/

/** Raw values of the constants of a format, in the order Pi, TwoPi, HalfPi, QuarterPi, InversePi, E, Ln2, Ln10, Sqrt2, InverseSqrt2, Sqrt3, GoldenRatio, DegreesPerRadian, RadiansPerDegree */
template < typename T >
static void
GetConstantRaws( Int64* raws )
{
    typedef CTMathConstants< T > TC;
    raws[ 0 ] = TC::Pi().GetRaw(); raws[ 1 ] = TC::TwoPi().GetRaw(); raws[ 2 ] = TC::HalfPi().GetRaw(); raws[ 3 ] = TC::QuarterPi().GetRaw();
    raws[ 4 ] = TC::InversePi().GetRaw(); raws[ 5 ] = TC::E().GetRaw(); raws[ 6 ] = TC::Ln2().GetRaw(); raws[ 7 ] = TC::Ln10().GetRaw();
    raws[ 8 ] = TC::Sqrt2().GetRaw(); raws[ 9 ] = TC::InverseSqrt2().GetRaw(); raws[ 10 ] = TC::Sqrt3().GetRaw(); raws[ 11 ] = TC::GoldenRatio().GetRaw();
    raws[ 12 ] = TC::DegreesPerRadian().GetRaw(); raws[ 13 ] = TC::RadiansPerDegree().GetRaw();
}

/*-------------------------------------------------------------------------*/

/** CBigInteger reference of the square root of 3.6.4: s = ISqrt( raw * 2^F ), plus one when v - s^2 > s */
template < typename TStorage, UInt32 F >
static bool
SqrtMatchesBigInteger( TStorage raw )
{
    CORE::CBigInteger v, s, square, rest;
    CORE::CBigInteger( (Int64) raw ).ShiftLeft( F, v );
    v.ISqrt( s );
    s.Square( square );
    v.Subtract( square, rest );
    if ( rest.Compare( s ) > 0 )
        s.Add( 1, s );
    return 0 == s.Compare( CORE::CBigInteger( (Int64) FixedSqrt( CTFixedPoint< TStorage, F >::FromRaw( raw ) ).GetRaw() ) );
}

/*-------------------------------------------------------------------------*/

/**
 *  A literal with 'bits' fractional bits recomputed from a decimal expansion with CBigInteger: round half up of
 *  numeratorScale * digits * 2^bits / ( denominatorScale * 10^decimals ), or of the reciprocal when 'reciprocal'.
 */
static CORE::CBigInteger
LiteralFromExpansion( const char* expansion, UInt32 bits, Int64 numeratorScale, Int64 denominatorScale, bool reciprocal )
{
    CORE::CString text( expansion );
    Int32 point = text.HasChar( '.' );
    UInt32 decimals = point < 0 ? 0 : text.Length() - (UInt32) point - 1;
    CORE::CString digits = point < 0 ? text : text.SubstrFromRange( 0, (UInt32) point ) + text.SubstrFromRange( (UInt32) point + 1, text.Length() );
    CORE::CBigInteger value = CORE::CBigInteger::Parse( digits.C_String() );
    CORE::CBigInteger power;
    CORE::CBigInteger( 10 ).Pow( decimals, power );
    CORE::CBigInteger numerator, denominator, q, r;
    if ( reciprocal )
    {
        power.Multiply( numeratorScale, numerator );
        value.Multiply( denominatorScale, denominator );
    }
    else
    {
        value.Multiply( numeratorScale, numerator );
        power.Multiply( denominatorScale, denominator );
    }
    numerator.ShiftLeft( bits, numerator );
    numerator.DivideRounded( denominator, CORE::ROUNDINGMODE_HALF_UP, q, r );
    return q;
}

/*-------------------------------------------------------------------------*/

/**
 *  sin and cos of large arguments, where the range reduction matters: raw values with up to 'rawBits' significant bits
 *  (so the Float64 reference input is exact), against the Float64 functions, which reduce exactly
 */
template < typename TStorage, UInt32 F >
static bool
LargeArgumentsWithinTargets( UInt32 rawBits, Float64 limit, UInt64 seed )
{
    typedef CTFixedPoint< TStorage, F > T;
    typedef CTNumericTraits< T > TT;
    MathReference::CSeededSequence generator( seed );
    CErrorTracker sinError( "sin of large arguments", limit ), cosError( "cos of large arguments", limit );
    for ( UInt32 i=0; i<16384; ++i )
    {
        T x = T::FromRaw( (TStorage) generator.NextSignedWithRandomLength( rawBits + 1 ) );
        Float64 xd = x.ToFloat64();
        sinError.Add( TT::Sin( x ).GetRaw(), sin( xd ), F, xd, 0 );
        cosError.Add( TT::Cos( x ).GetRaw(), cos( xd ), F, xd, 0 );
    }
    bool sinOk = sinError.IsWithin( seed );
    bool cosOk = cosError.IsWithin( seed );
    return sinOk && cosOk;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformFixedPointFunctionsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "FixedPointFunctions" );

    GUCEF_TESTFW_TESTCASE( "FixedPointFunctions: square root of 2 and correct rounding of the Q16.16 subset" )
    try
    {
        ASSERT_TRUE( 92682 == FixedSqrt( TFixed16_16( 2 ) ).GetRaw() );
        ASSERT_TRUE( 6074001000LL == FixedSqrt( TFixed32_32( 2 ) ).GetRaw() );
        ASSERT_TRUE( 92682 == FixedSqrt( TFixed48_16( 2 ) ).GetRaw() );

        // every 1021st raw value plus the 1000 smallest and the 1000 largest
        UInt32 failures = 0;
        UInt32 checked = 0;
        for ( Int64 raw=0; raw<=0x7FFFFFFF; raw += 1021 )
        {
            if ( FixedSqrt( TFixed16_16::FromRaw( (Int32) raw ) ).GetRaw() != ReferenceSqrtQ16( raw ) )
                ++failures;
            ++checked;
        }
        for ( Int64 raw=0; raw<1000; ++raw )
        {
            if ( FixedSqrt( TFixed16_16::FromRaw( (Int32) raw ) ).GetRaw() != ReferenceSqrtQ16( raw ) )
                ++failures;
            Int64 top = 0x7FFFFFFF - raw;
            if ( FixedSqrt( TFixed16_16::FromRaw( (Int32) top ) ).GetRaw() != ReferenceSqrtQ16( top ) )
                ++failures;
            checked += 2;
        }
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FixedPointFunctions: Q16.16 sqrt failures " << failures << " of " << checked );
        ASSERT_TRUE( 0 == failures );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FixedPointFunctions: Q16.16 square root correctly rounded for all 2^31 inputs (GUCEF_TEST_MATH_EXHAUSTIVE=1)" )
    {
        GUCEF_TESTFW_SKIP_IF( !MathReference::ExhaustiveTestsEnabled(), "set GUCEF_TEST_MATH_EXHAUSTIVE=1 to run the exhaustive sweep" );
        try
        {
            UInt32 failures = 0;
            for ( Int64 raw=0; raw<=0x7FFFFFFF; ++raw )
                if ( FixedSqrt( TFixed16_16::FromRaw( (Int32) raw ) ).GetRaw() != ReferenceSqrtQ16( raw ) )
                    ++failures;
            ASSERT_TRUE( 0 == failures );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FixedPointFunctions: square roots equal the CBigInteger::ISqrt reference (Q16.16 subset, 10^5 seeded Q32.32 and Q48.16)" )
    try
    {
        UInt32 failures = 0;
        for ( Int64 raw=0; raw<=0x7FFFFFFF; raw += 1021 * 7 )
            if ( !( SqrtMatchesBigInteger< Int32, 16 >( (Int32) raw ) ) )
                ++failures;
        MathReference::CSeededSequence generator( 6464 );
        for ( UInt32 i=0; i<100000; ++i )
        {
            Int64 raw = generator.NextSignedWithRandomLength( 64 );
            raw = raw < 0 ? -raw : raw;
            if ( !( SqrtMatchesBigInteger< Int64, 32 >( raw ) ) || !( SqrtMatchesBigInteger< Int64, 16 >( raw ) ) )
                ++failures;
        }
        ASSERT_TRUE( ( SqrtMatchesBigInteger< Int64, 32 >( (Int64) 0x7FFFFFFFFFFFFFFFULL ) ) && ( SqrtMatchesBigInteger< Int64, 16 >( (Int64) 0x7FFFFFFFFFFFFFFFULL ) ) );
        ASSERT_TRUE( 0 == failures );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FixedPointFunctions: every constant literal equals its decimal expansion or exact rational, computed with CBigInteger" )
    try
    {
        const char* pi = "3.14159265358979323846264338327950288419716939937510582097494459230781640628620899862803482534211706798";
        const char* e = "2.71828182845904523536028747135266249775724709369995957496696762772407663035354759457138217852516642742";
        const char* ln2 = "0.69314718055994530941723212145817656807550013436025525412068000949339362196969471560586332699641868754";
        const char* ln10 = "2.30258509299404568401799145468436420760110148862877297603332790096757260967735248023599720508959829834";
        const char* sqrt2 = "1.41421356237309504880168872420969807856967187537694807317667973799073247846210703885038753432764157273";
        const char* sqrt3 = "1.73205080756887729352744634150587236694280525381038062805580697945193301690880003708114618675724857567";
        const char* golden = "1.61803398874989484820458683436563811772030917980576286213544862270526046281890244970720720418939113748";
        ASSERT_TRUE( 0 == LiteralFromExpansion( pi, 61, 1, 1, false ).Compare( CORE::CBigInteger( FixedQ61Pi() ) ) );
        ASSERT_TRUE( 0 == LiteralFromExpansion( pi, 61, 1, 2, false ).Compare( CORE::CBigInteger( FixedQ61HalfPi() ) ) );
        ASSERT_TRUE( 0 == LiteralFromExpansion( pi, 61, 1, 1, true ).Compare( CORE::CBigInteger( FixedQ61InversePi() ) ) );
        CORE::CBigInteger inverseTwoPi = LiteralFromExpansion( pi, 128, 1, 2, true );
        CORE::CBigInteger high, low;
        inverseTwoPi.ShiftRight( 64, high );
        inverseTwoPi.And( CORE::CBigInteger( ~(UInt64) 0 ), low );
        ASSERT_TRUE( 0 == high.Compare( CORE::CBigInteger( FixedQ128InverseTwoPiHigh() ) ) && 0 == low.Compare( CORE::CBigInteger( FixedQ128InverseTwoPiLow() ) ) );
        ASSERT_TRUE( 0 == LiteralFromExpansion( ln2, 61, 1, 1, false ).Compare( CORE::CBigInteger( FixedQ61Ln2() ) ) );
        ASSERT_TRUE( 0 == LiteralFromExpansion( ln2, 61, 1, 1, true ).Compare( CORE::CBigInteger( FixedQ61InverseLn2() ) ) );
        ASSERT_TRUE( 0 == LiteralFromExpansion( ln10, 61, 1, 1, false ).Compare( CORE::CBigInteger( FixedQ61Ln10() ) ) );
        ASSERT_TRUE( 0 == LiteralFromExpansion( e, 61, 1, 1, false ).Compare( CORE::CBigInteger( FixedQ61E() ) ) );
        ASSERT_TRUE( 0 == LiteralFromExpansion( sqrt2, 61, 1, 1, false ).Compare( CORE::CBigInteger( FixedQ61Sqrt2() ) ) );
        ASSERT_TRUE( 0 == LiteralFromExpansion( sqrt2, 61, 1, 2, false ).Compare( CORE::CBigInteger( FixedQ61InverseSqrt2() ) ) );       // 1 / sqrt 2 = sqrt 2 / 2
        ASSERT_TRUE( 0 == LiteralFromExpansion( sqrt3, 61, 1, 1, false ).Compare( CORE::CBigInteger( FixedQ61Sqrt3() ) ) );
        ASSERT_TRUE( 0 == LiteralFromExpansion( golden, 61, 1, 1, false ).Compare( CORE::CBigInteger( FixedQ61GoldenRatio() ) ) );
        ASSERT_TRUE( 0 == LiteralFromExpansion( pi, 57, 180, 1, true ).Compare( CORE::CBigInteger( FixedQ57DegreesPerRadian() ) ) );
        ASSERT_TRUE( 0 == LiteralFromExpansion( pi, 64, 1, 180, false ).Compare( CORE::CBigInteger( FixedQ64RadiansPerDegree() ) ) );

        // exact rationals: 1 / n! and 1 / ( 2k + 1 ) in Q2.61
        CORE::CBigInteger factorial( 1 );
        for ( UInt32 n=0; n<16; ++n )
        {
            if ( n > 0 )
                factorial.Multiply( (Int64) n, factorial );
            CORE::CBigInteger q, r, one;
            CORE::CBigInteger( 1 ).ShiftLeft( 61, one );
            one.DivideRounded( factorial, CORE::ROUNDINGMODE_HALF_UP, q, r );
            ASSERT_TRUE( 0 == q.Compare( CORE::CBigInteger( FixedQ61ExpCoefficient( n ) ) ) );
        }
        for ( UInt32 k=0; k<15; ++k )
        {
            CORE::CBigInteger q, r, one;
            CORE::CBigInteger( 1 ).ShiftLeft( 61, one );
            one.DivideRounded( CORE::CBigInteger( (Int64) ( 2 * k + 1 ) ), CORE::ROUNDINGMODE_HALF_UP, q, r );
            ASSERT_TRUE( 0 == q.Compare( CORE::CBigInteger( FixedQ61OddReciprocal( k ) ) ) );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FixedPointFunctions: special values (sin 0, cos 0, atan2 of the axes, exact powers, saturation)" )
    try
    {
        ASSERT_TRUE( ( SpecialValuesHold< Int32, 16 >() ) );
        ASSERT_TRUE( ( SpecialValuesHold< Int64, 32 >() ) );
        ASSERT_TRUE( ( SpecialValuesHold< Int64, 16 >() ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FixedPointFunctions: accuracy against Float64 over 2^16 seeded inputs per function within the targets of 3.6.5" )
    try
    {
        SAccuracyTargets narrow = { 2, 3, 4 };      // Q16.16
        SAccuracyTargets wide = { 4, 6, 8 };        // Q32.32 (and Q48.16, which has the resolution of Q16.16)
        ASSERT_TRUE( ( AccuracyWithinTargets< Int32, 16 >( narrow, 14.99, 1001 ) ) );
        ASSERT_TRUE( ( AccuracyWithinTargets< Int64, 32 >( wide, 30.99, 1002 ) ) );
        ASSERT_TRUE( ( AccuracyWithinTargets< Int64, 16 >( wide, 46.99, 1003 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FixedPointFunctions: range reduction of large arguments (Q48.16 up to 2^46 raw, Q32.32 up to 2^53 raw)" )
    try
    {
        ASSERT_TRUE( ( LargeArgumentsWithinTargets< Int64, 16 >( 46, 4, 2046 ) ) );
        ASSERT_TRUE( ( LargeArgumentsWithinTargets< Int64, 32 >( 53, 4, 3253 ) ) );
        ASSERT_TRUE( ( LargeArgumentsWithinTargets< Int32, 16 >( 31, 2, 1631 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FixedPointFunctions: format constants are the correctly rounded values of the exact constants" )
    try
    {
        // the values of the exact constants rounded to nearest at 16 and 32 fraction bits, computed independently from the
        // decimal expansions of the header comment with exact rational arithmetic (Python Decimal at 120 digits)
        static const Int64 q16[ 14 ] = { 205887, 411775, 102944, 51472, 20861, 178145, 45426, 150902, 92682, 46341, 113512, 106039, 3754936, 1144 };
        static const Int64 q32[ 14 ] = { 13493037705LL, 26986075409LL, 6746518852LL, 3373259426LL, 1367130551LL, 11674931555LL, 2977044472LL,
                                         9889527671LL, 6074001000LL, 3037000500LL, 7439101574LL, 6949403065LL, 246083499208LL, 74961321LL };
        Int64 raws[ 14 ];
        GetConstantRaws< TFixed16_16 >( raws );
        ASSERT_TRUE( 0 == memcmp( raws, q16, sizeof( raws ) ) );
        GetConstantRaws< TFixed48_16 >( raws );
        ASSERT_TRUE( 0 == memcmp( raws, q16, sizeof( raws ) ) );
        GetConstantRaws< TFixed32_32 >( raws );
        ASSERT_TRUE( 0 == memcmp( raws, q32, sizeof( raws ) ) );
        ASSERT_TRUE( CTNumericTraits< TFixed32 >::Pi() == CTMathConstants< TFixed32 >::Pi() && CTNumericTraits< TFixed64 >::Sqrt2() == CTMathConstants< TFixed64 >::Sqrt2() );

        // the series coefficients are exact rationals rounded to nearest in Q2.61: checked with exact 128 bit division
        UInt64 factorial = 1;
        for ( UInt32 n=0; n<16; ++n )
        {
            if ( n > 0 )
                factorial *= n;
            CORE::CUInt128 q;
            UInt64 r = 0;
            CORE::CUInt128::DivMod( CORE::CUInt128( (UInt64) 1 << 61 ), factorial, q, r );
            UInt64 expected = q.low + ( 2 * r >= factorial ? 1 : 0 );
            ASSERT_TRUE( (Int64) expected == FixedQ61ExpCoefficient( n ) );
            if ( 1 == ( n & 1 ) )
            {
                Int64 signedExpected = 0 == ( ( n / 2 ) & 1 ) ? (Int64) expected : -(Int64) expected;
                ASSERT_TRUE( signedExpected == FixedQ61SinCoefficient( n / 2 ) );
            }
        }
        for ( UInt32 k=0; k<15; ++k )
        {
            UInt64 d = 2 * k + 1;
            CORE::CUInt128 q;
            UInt64 r = 0;
            CORE::CUInt128::DivMod( CORE::CUInt128( (UInt64) 1 << 61 ), d, q, r );
            ASSERT_TRUE( (Int64) ( q.low + ( 2 * r >= d ? 1 : 0 ) ) == FixedQ61OddReciprocal( k ) );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FixedPointFunctions: determinism, pinned FNV-1a 64 hashes of the raw outputs per format" )
    try
    {
        // values from the first green run (x86-64, GCC); the same on every platform, compiler and build type
        const UInt64 expectedQ16_16 = 16484752906276867190ULL;
        const UInt64 expectedQ32_32 = 10867056326290988735ULL;
        const UInt64 expectedQ48_16 = 1775864561272885377ULL;
        UInt64 hashQ16_16 = DeterminismHash< Int32, 16 >( 14.99 );
        UInt64 hashQ32_32 = DeterminismHash< Int64, 32 >( 30.99 );
        UInt64 hashQ48_16 = DeterminismHash< Int64, 16 >( 46.99 );
        if ( hashQ16_16 != expectedQ16_16 || hashQ32_32 != expectedQ32_32 || hashQ48_16 != expectedQ48_16 )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FixedPointFunctions: determinism hashes " << CORE::UInt64ToString( hashQ16_16 ) << " " << CORE::UInt64ToString( hashQ32_32 ) << " " << CORE::UInt64ToString( hashQ48_16 ) );
        ASSERT_TRUE( expectedQ16_16 == hashQ16_16 );
        ASSERT_TRUE( expectedQ32_32 == hashQ32_32 );
        ASSERT_TRUE( expectedQ48_16 == hashQ48_16 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
