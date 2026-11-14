/*
 *  gucefMATH: GUCEF module providing math: scalar, vector, matrix and numeric classes
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

#ifndef GUCEF_MATH_CTNUMERICTRAITSFRACTION_H
#define GUCEF_MATH_CTNUMERICTRAITSFRACTION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTFRACTION_H
#include "gucefCORE_CTFraction.h"
#define GUCEF_CORE_CTFRACTION_H
#endif /* GUCEF_CORE_CTFRACTION_H ? */

#ifndef GUCEF_CORE_CBIGINTEGER_H
#include "gucefCORE_CBigInteger.h"
#define GUCEF_CORE_CBIGINTEGER_H
#endif /* GUCEF_CORE_CBIGINTEGER_H ? */

#ifndef GUCEF_MATH_CTNUMERICTRAITS_H
#include "gucefMATH_CTNumericTraits.h"
#define GUCEF_MATH_CTNUMERICTRAITS_H
#endif /* GUCEF_MATH_CTNUMERICTRAITS_H ? */

#ifndef GUCEF_MATH_CTNUMERICCONVERTER_H
#include "gucefMATH_CTNumericConverter.h"
#define GUCEF_MATH_CTNUMERICCONVERTER_H
#endif /* GUCEF_MATH_CTNUMERICCONVERTER_H ? */

#ifndef GUCEF_MATH_CTFIXEDPOINT_H
#include "gucefMATH_CTFixedPoint.h"
#define GUCEF_MATH_CTFIXEDPOINT_H
#endif /* GUCEF_MATH_CTFIXEDPOINT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Greatest common divisor of two 128 bit magnitudes (Euclid), gcd( 0, 0 ) = 0 */
inline CORE::CUInt128
MathGcd128( CORE::CUInt128 a, CORE::CUInt128 b )
{
    while ( !b.IsZero() )
    {
        CORE::CUInt128 q;
        CORE::CUInt128 r;
        CORE::CUInt128::DivMod( a, b, q, r );
        a = b;
        b = r;
    }
    return a;
}

/*-------------------------------------------------------------------------*/

/**
 *  The best rational approximation of N / D (N, D > 0, N / D <= maxNumerator) with numerator <= maxNumerator and
 *  denominator <= maxDenominator, by continued fraction convergents and the last semiconvergent. When the
 *  semiconvergent and the previous convergent are equally qualified by the half rule, the convergent is chosen.
 */
inline void
MathBestRationalApproximation( CORE::CUInt128 n, CORE::CUInt128 d, UInt64 maxNumerator, UInt64 maxDenominator, UInt64& outN, UInt64& outD )
{
    UInt64 p0 = 0, q0 = 1, p1 = 1, q1 = 0;
    for ( ;; )
    {
        CORE::CUInt128 a128;
        CORE::CUInt128 r;
        CORE::CUInt128::DivMod( n, d, a128, r );
        // the next convergent p2 / q2 = ( a p1 + p0 ) / ( a q1 + q0 ), if it fits
        UInt64 tMax = ~(UInt64) 0;
        if ( 0 != q1 )
            tMax = ( maxDenominator - q0 ) / q1;
        if ( 0 != p1 )
        {
            UInt64 limit = ( maxNumerator - p0 ) / p1;
            if ( limit < tMax )
                tMax = limit;
        }
        if ( 0 != a128.high || a128.low > tMax )
        {
            // semiconvergent with the largest t that fits, used when t > a / 2 (t == a / 2: the convergent is kept)
            UInt64 t = tMax;
            CORE::CUInt128 twiceT = CORE::CUInt128::ShiftLeft( CORE::CUInt128( t ), 1 );
            if ( 0 != q1 && CORE::CUInt128::Compare( twiceT, a128 ) > 0 )
            {
                outN = t * p1 + p0;
                outD = t * q1 + q0;
            }
            else
            {
                outN = p1;
                outD = q1;
            }
            if ( 0 == outD )
            {
                outN = maxNumerator;
                outD = 1;
            }
            return;
        }
        UInt64 a = a128.low;
        UInt64 p2 = a * p1 + p0;
        UInt64 q2 = a * q1 + q0;
        p0 = p1; q0 = q1; p1 = p2; q1 = q2;
        if ( r.IsZero() )
        {
            outN = p1;
            outD = q1;
            return;
        }
        n = d;
        d = r;
    }
}

/*-------------------------------------------------------------------------*/

/**
 *  An exact value as a fraction of two Int64 in lowest terms with a positive denominator. Exact whenever the reduced
 *  value fits; a magnitude above the Int64 range saturates with OVERFLOW; otherwise the best rational approximation
 *  with |numerator| and denominator within Int64 is returned with INEXACT (the rounding mode does not direct it;
 *  ROUNDINGMODE_UNNECESSARY gives ROUNDING_REQUIRED and leaves the output unchanged). NaN and infinity give 0 / 1 with
 *  INVALID_OPERAND.
 */
inline TMathStatus
MathPartsToInt64Ratio( const TNumericValueParts& parts, TRoundingMode mode, Int64& outN, Int64& outD )
{
    if ( !MathIsValidRoundingMode( mode ) )
        return MATHSTATUS_INVALID_OPERAND;
    if ( parts.isNaN || parts.isInfinite )
    {
        outN = 0;
        outD = 1;
        return MATHSTATUS_INVALID_OPERAND;
    }
    if ( 0 == parts.magnitude )
    {
        outN = 0;
        outD = 1;
        return MATHSTATUS_OK;
    }

    const bool negative = parts.negative;
    const UInt64 maxNumerator = negative ? 0x8000000000000000ULL : 0x7FFFFFFFFFFFFFFFULL;
    const UInt64 maxDenominator = 0x7FFFFFFFFFFFFFFFULL;

    UInt64 m = parts.magnitude;
    Int32 e = parts.exponent2;
    UInt64 den = parts.denominator;
    if ( 1 == den )
    {
        UInt32 zeros = MathCountTrailingZeros( m );
        m >>= zeros;
        e += (Int32) zeros;
    }

    CORE::CUInt128 n;
    CORE::CUInt128 d;
    if ( e >= 0 )
    {
        if ( MathBitLength( m ) + (UInt32) e > 127 )
        {
            outN = negative ? MathApplySign( true, maxNumerator ) : (Int64) maxNumerator;
            outD = 1;
            return MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT;
        }
        n = CORE::CUInt128::ShiftLeft( CORE::CUInt128( m ), (UInt32) e );
        d = CORE::CUInt128( den );
    }
    else
    {
        UInt32 k = (UInt32) ( -( e + 1 ) ) + 1;
        if ( MathBitLength( den ) + k > 127 )
        {
            // |value| < 2^-63: the candidates are 0 and 1 / ( 2^63 - 1 ); the threshold is taken as 2^-64
            if ( CORE::ROUNDINGMODE_UNNECESSARY == mode )
                return MATHSTATUS_ROUNDING_REQUIRED;
            Int32 lead = (Int32) MathBitLength( m ) - 1 + e - ( (Int32) MathBitLength( den ) - 1 );
            if ( lead >= -64 )
            {
                outN = negative ? -1 : 1;
                outD = (Int64) maxDenominator;
            }
            else
            {
                outN = 0;
                outD = 1;
            }
            return MATHSTATUS_INEXACT;
        }
        n = CORE::CUInt128( m );
        d = CORE::CUInt128::ShiftLeft( CORE::CUInt128( den ), k );
    }

    CORE::CUInt128 g = MathGcd128( n, d );
    CORE::CUInt128 remainder;
    CORE::CUInt128::DivMod( n, g, n, remainder );
    CORE::CUInt128::DivMod( d, g, d, remainder );

    if ( 0 == n.high && n.low <= maxNumerator && 0 == d.high && d.low <= maxDenominator )
    {
        outN = MathApplySign( negative, n.low );
        outD = (Int64) d.low;
        return MATHSTATUS_OK;
    }

    // value above the numerator range: saturate
    CORE::CUInt128 whole;
    CORE::CUInt128::DivMod( n, d, whole, remainder );
    if ( 0 != whole.high || whole.low > maxNumerator || ( whole.low == maxNumerator && !remainder.IsZero() ) )
    {
        outN = MathApplySign( negative, maxNumerator );
        outD = 1;
        return MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT;
    }
    if ( CORE::ROUNDINGMODE_UNNECESSARY == mode )
        return MATHSTATUS_ROUNDING_REQUIRED;

    UInt64 bestN = 0;
    UInt64 bestD = 1;
    MathBestRationalApproximation( n, d, maxNumerator, maxDenominator, bestN, bestD );
    outN = MathApplySign( negative, bestN );
    outD = (Int64) bestD;
    return MATHSTATUS_INEXACT;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Numeric traits of CTFraction< Int64, Int64 >, the bounded exact rational kind (see CTNumericTraits for the
 *  contract). Arithmetic is exact with 128 bit intermediates; a reduced result that does not fit Int64 makes the
 *  operators throw std::overflow_error (use the Try* members of CTFraction where that matters). Conversions to
 *  floats are correctly rounded (CTFraction::ToFloat64 is not). Abs and Sign of the lowest value saturate.
 */
template <>
class CTNumericTraits< CORE::CTFraction< Int64, Int64 > >
{
    public:

    typedef CORE::CTFraction< Int64, Int64 > TValue;
    typedef TValue TWide;
    typedef Float64 TFloat;

    enum
    {
        IsFloatingPoint = 0,
        IsFixedPoint = 0,
        IsInteger = 0,
        IsSigned = 1,
        IsExact = 1,
        HasSqrt = 0,
        HasTranscendentals = 0,
        HasDivision = 1,
        IsDeterministic = 1,
        IsRealtimeSafe = 1,
        IsSimdCandidate = 0
    };

    static TValue Zero( void ) { return TValue( 0, 1 ); }
    static TValue One( void ) { return TValue( 1, 1 ); }
    static TValue Two( void ) { return TValue( 2, 1 ); }
    static TValue Half( void ) { return TValue( 1, 2 ); }
    static TValue Epsilon( void ) { return Zero(); }
    static TValue DefaultAbsoluteTolerance( void ) { return Zero(); }
    static TValue DefaultRelativeTolerance( void ) { return Zero(); }
    static TValue Lowest( void ) { return TValue( MathApplySign( true, 0x8000000000000000ULL ), 1 ); }
    static TValue Max( void ) { return TValue( (Int64) 0x7FFFFFFFFFFFFFFFULL, 1 ); }

    static bool IsNegative( const TValue& x ) { return x.GetNumerator() < 0; }

    /** |x|; the magnitude of the lowest value (2^63) saturates to Max() */
    static TValue Abs( const TValue& x )
    {
        Int64 n = x.GetNumerator();
        if ( n >= 0 )
            return x;
        if ( MathMagnitude( n ) > 0x7FFFFFFFFFFFFFFFULL )
            return Max();
        return TValue( -n, x.GetDenominator() );
    }

    static TValue Min( const TValue& a, const TValue& b ) { return b < a ? b : a; }
    static TValue Max( const TValue& a, const TValue& b ) { return a < b ? b : a; }
    static TValue Sign( const TValue& x ) { return x.GetNumerator() < 0 ? TValue( -1, 1 ) : ( 0 == x.GetNumerator() ? Zero() : One() ); }

    /** Integral value rounded with 'mode' (ROUNDINGMODE_UNNECESSARY truncates) */
    static TValue Round( const TValue& x, TRoundingMode mode )
    {
        if ( !MathIsValidRoundingMode( mode ) || CORE::ROUNDINGMODE_UNNECESSARY == mode )
            mode = CORE::ROUNDINGMODE_TOWARD_ZERO;
        Int64 whole = 0;
        MathRoundPartsToSigned( ToParts( x ), 0, 64, mode, whole );
        return TValue( whole, 1 );
    }

    static TValue Floor( const TValue& x ) { return Round( x, CORE::ROUNDINGMODE_FLOOR ); }
    static TValue Ceil( const TValue& x ) { return Round( x, CORE::ROUNDINGMODE_CEILING ); }
    static TValue Truncate( const TValue& x ) { return Round( x, CORE::ROUNDINGMODE_TOWARD_ZERO ); }

    /** x - period * floor( x / period ), exact; period must not be 0 */
    static TValue ModFloor( const TValue& x, const TValue& period )
    {
        return x - period * Floor( x / period );
    }

    static TValue FromInt32( Int32 value ) { return TValue( (Int64) value, 1 ); }

    /** Exact; the mode is accepted for the common signature */
    static TMathStatus FromInt64( Int64 value, TRoundingMode mode, TValue& out )
    {
        if ( !MathIsValidRoundingMode( mode ) )
            return MATHSTATUS_INVALID_OPERAND;
        out = TValue( value, 1 );
        return MATHSTATUS_OK;
    }

    /** Exact when value = m 2^e fits, else the best rational approximation (INEXACT), see MathPartsToInt64Ratio */
    static TMathStatus FromFloat64( Float64 value, TRoundingMode mode, TValue& out )
    {
        return FromParts( MathFloat64ToParts( value ), mode, out );
    }

    /** Correctly rounded to nearest (ties to even) */
    static Float64 ToFloat64( const TValue& x )
    {
        Float64 result = 0.0;
        MathRoundPartsToFloat( ToParts( x ), CORE::ROUNDINGMODE_HALF_EVEN, result );
        return result;
    }

    /** Correctly rounded to nearest (ties to even): one rounding, no Float64 intermediate */
    static Float32 ToFloat32( const TValue& x )
    {
        Float32 result = 0.0f;
        MathRoundPartsToFloat( ToParts( x ), CORE::ROUNDINGMODE_HALF_EVEN, result );
        return result;
    }

    static TNumericValueParts ToParts( const TValue& x )
    {
        Int64 n = x.GetNumerator();
        return MathMakeValueParts( n < 0, MathMagnitude( n ), 0, (UInt64) x.GetDenominator() );
    }

    static TMathStatus FromParts( const TNumericValueParts& parts, TRoundingMode mode, TValue& out )
    {
        if ( !MathIsValidRoundingMode( mode ) )
            return MATHSTATUS_INVALID_OPERAND;
        Int64 n = 0;
        Int64 d = 1;
        TMathStatus status = MathPartsToInt64Ratio( parts, mode, n, d );
        if ( 0 != ( status & MATHSTATUS_ROUNDING_REQUIRED ) )
            return status;
        out = TValue( n, d );
        return status;
    }

    static bool IsFinite( const TValue& /*x*/ ) { return true; }

    /** a * b + c, exact; throws std::overflow_error when a reduced intermediate does not fit */
    static TValue MultiplyAdd( const TValue& a, const TValue& b, const TValue& c ) { return a * b + c; }

    /** "numerator/denominator", or "numerator" when the denominator is 1 */
    static CORE::CString ToString( const TValue& x )
    {
        if ( 1 == x.GetDenominator() )
            return CORE::Int64ToString( x.GetNumerator() );
        return CORE::Int64ToString( x.GetNumerator() ) + "/" + CORE::Int64ToString( x.GetDenominator() );
    }

    /** Strict: "[+|-]digits" or "[+|-]digits/digits" with a non-zero denominator, reduced. false leaves 'out' unchanged */
    static bool FromString( const CORE::CString& text, TValue& out )
    {
        Int32 slash = text.HasChar( '/' );
        Int64 n = 0;
        Int64 d = 1;
        if ( slash < 0 )
        {
            if ( !CTNumericTraits< Int64 >::FromString( text, n ) )
                return false;
        }
        else
        {
            if ( !CTNumericTraits< Int64 >::FromString( text.SubstrFromRange( 0, (UInt32) slash ), n ) ||
                 !CTNumericTraits< Int64 >::FromString( text.SubstrFromRange( (UInt32) slash + 1, text.Length() ), d ) || d <= 0 )
                return false;
        }
        TValue value;
        if ( MATHSTATUS_OK != FromParts( MathMakeValueParts( n < 0, MathMagnitude( n ), 0, (UInt64) d ), CORE::ROUNDINGMODE_UNNECESSARY, value ) )
            return false;
        out = value;
        return true;
    }

    /** |a - b| <= max( absolute, relative * max( |a|, |b| ) ) exactly; false when an intermediate does not fit Int64 */
    static bool EqualsWithin( const TValue& a, const TValue& b, const TValue& absoluteTolerance, const TValue& relativeTolerance )
    {
        if ( a == b )
            return true;
        TValue difference;
        if ( !a.TrySubtract( b, difference ) )
            return false;
        difference = Abs( difference );
        if ( !( absoluteTolerance < difference ) )
            return true;
        TValue limit;
        if ( !relativeTolerance.TryMultiply( Max( Abs( a ), Abs( b ) ), limit ) )
            return false;
        return !( limit < difference );
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  Numeric traits of CTFraction< CBigInteger, CBigInteger >, the unbounded exact rational kind (see CTNumericTraits
 *  for the contract). Every value of every other kind converts into it exactly. Not realtime-safe: values beyond 256
 *  bits use the heap. There is no Lowest() / Max(). Conversions to floats are correctly rounded
 *  (CBigInteger::RatioToFloat64 / RatioToFloat32).
 */
template <>
class CTNumericTraits< CORE::CTFraction< CORE::CBigInteger, CORE::CBigInteger > >
{
    public:

    typedef CORE::CTFraction< CORE::CBigInteger, CORE::CBigInteger > TValue;
    typedef TValue TWide;
    typedef Float64 TFloat;

    enum
    {
        IsFloatingPoint = 0,
        IsFixedPoint = 0,
        IsInteger = 0,
        IsSigned = 1,
        IsExact = 1,
        HasSqrt = 0,
        HasTranscendentals = 0,
        HasDivision = 1,
        IsDeterministic = 1,
        IsRealtimeSafe = 0,
        IsSimdCandidate = 0
    };

    static TValue Zero( void ) { return TValue( 0, 1 ); }
    static TValue One( void ) { return TValue( 1, 1 ); }
    static TValue Two( void ) { return TValue( 2, 1 ); }
    static TValue Half( void ) { return TValue( 1, 2 ); }
    static TValue Epsilon( void ) { return Zero(); }
    static TValue DefaultAbsoluteTolerance( void ) { return Zero(); }
    static TValue DefaultRelativeTolerance( void ) { return Zero(); }

    static bool IsNegative( const TValue& x ) { return x.GetNumerator().IsNegative(); }
    static TValue Abs( const TValue& x ) { return IsNegative( x ) ? TValue( -x.GetNumerator(), x.GetDenominator() ) : x; }
    static TValue Min( const TValue& a, const TValue& b ) { return b < a ? b : a; }
    static TValue Max( const TValue& a, const TValue& b ) { return a < b ? b : a; }
    static TValue Sign( const TValue& x ) { return TValue( x.GetNumerator().GetSign(), 1 ); }

    /** Integral value rounded with 'mode' (ROUNDINGMODE_UNNECESSARY truncates) */
    static TValue Round( const TValue& x, TRoundingMode mode )
    {
        if ( !MathIsValidRoundingMode( mode ) || CORE::ROUNDINGMODE_UNNECESSARY == mode )
            mode = CORE::ROUNDINGMODE_TOWARD_ZERO;
        CORE::CBigInteger quotient;
        CORE::CBigInteger remainder;
        x.GetNumerator().DivideRounded( x.GetDenominator(), mode, quotient, remainder );
        return TValue( quotient, 1 );
    }

    static TValue Floor( const TValue& x ) { return Round( x, CORE::ROUNDINGMODE_FLOOR ); }
    static TValue Ceil( const TValue& x ) { return Round( x, CORE::ROUNDINGMODE_CEILING ); }
    static TValue Truncate( const TValue& x ) { return Round( x, CORE::ROUNDINGMODE_TOWARD_ZERO ); }

    /** x - period * floor( x / period ), exact; period must not be 0 */
    static TValue ModFloor( const TValue& x, const TValue& period ) { return x - period * Floor( x / period ); }

    static TValue FromInt32( Int32 value ) { return TValue( value, 1 ); }

    static TMathStatus FromInt64( Int64 value, TRoundingMode mode, TValue& out )
    {
        if ( !MathIsValidRoundingMode( mode ) )
            return MATHSTATUS_INVALID_OPERAND;
        out = TValue( value, 1 );
        return MATHSTATUS_OK;
    }

    /** Exact for every finite value; NaN and infinity give 0 with INVALID_OPERAND */
    static TMathStatus FromFloat64( Float64 value, TRoundingMode mode, TValue& out )
    {
        return FromParts( MathFloat64ToParts( value ), mode, out );
    }

    /** Exact: magnitude * 2^exponent2 / denominator in lowest terms; the mode is only validated */
    static TMathStatus FromParts( const TNumericValueParts& parts, TRoundingMode mode, TValue& out )
    {
        if ( !MathIsValidRoundingMode( mode ) )
            return MATHSTATUS_INVALID_OPERAND;
        if ( parts.isNaN || parts.isInfinite )
        {
            out = Zero();
            return MATHSTATUS_INVALID_OPERAND;
        }
        CORE::CBigInteger numerator( parts.magnitude );
        CORE::CBigInteger denominator( parts.denominator );
        if ( parts.exponent2 >= 0 )
            numerator.ShiftLeft( (UInt32) parts.exponent2, numerator );
        else
            denominator.ShiftLeft( (UInt32) ( -( parts.exponent2 + 1 ) ) + 1, denominator );
        if ( parts.negative )
            numerator.Negate( numerator );
        out = TValue( numerator, denominator );
        return MATHSTATUS_OK;
    }

    /** Correctly rounded to nearest (ties to even) */
    static Float64 ToFloat64( const TValue& x )
    {
        Float64 result = 0.0;
        CORE::CBigInteger::RatioToFloat64( x.GetNumerator(), x.GetDenominator(), CORE::ROUNDINGMODE_HALF_EVEN, result );
        return result;
    }

    static Float32 ToFloat32( const TValue& x )
    {
        Float32 result = 0.0f;
        CORE::CBigInteger::RatioToFloat32( x.GetNumerator(), x.GetDenominator(), CORE::ROUNDINGMODE_HALF_EVEN, result );
        return result;
    }

    static bool IsFinite( const TValue& /*x*/ ) { return true; }
    static TValue MultiplyAdd( const TValue& a, const TValue& b, const TValue& c ) { return a * b + c; }

    /** "numerator/denominator", or "numerator" when the denominator is 1 */
    static CORE::CString ToString( const TValue& x )
    {
        if ( x.GetDenominator().IsOne() )
            return x.GetNumerator().ToString( 10 );
        return x.GetNumerator().ToString( 10 ) + "/" + x.GetDenominator().ToString( 10 );
    }

    /** Strict: "[+|-]digits" or "[+|-]digits/digits" with a positive denominator. false leaves 'out' unchanged */
    static bool FromString( const CORE::CString& text, TValue& out )
    {
        Int32 slash = text.HasChar( '/' );
        CORE::CBigInteger numerator;
        CORE::CBigInteger denominator( 1 );
        if ( slash < 0 )
        {
            if ( CORE::BIGINTEGERSTATUS_OK != CORE::CBigInteger::FromString( text, 10, numerator ) )
                return false;
        }
        else
        {
            if ( CORE::BIGINTEGERSTATUS_OK != CORE::CBigInteger::FromString( text.SubstrFromRange( 0, (UInt32) slash ), 10, numerator ) ||
                 CORE::BIGINTEGERSTATUS_OK != CORE::CBigInteger::FromString( text.SubstrFromRange( (UInt32) slash + 1, text.Length() ), 10, denominator ) ||
                 !denominator.IsPositive() )
                return false;
        }
        out = TValue( numerator, denominator );
        return true;
    }

    /** |a - b| <= max( absolute, relative * max( |a|, |b| ) ), exact */
    static bool EqualsWithin( const TValue& a, const TValue& b, const TValue& absoluteTolerance, const TValue& relativeTolerance )
    {
        TValue difference = Abs( a - b );
        if ( !( absoluteTolerance < difference ) )
            return true;
        return !( relativeTolerance * Max( Abs( a ), Abs( b ) ) < difference );
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONVERSIONS OF THE UNBOUNDED FRACTIONS                             //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CORE::CTFraction< CORE::CBigInteger, CORE::CBigInteger > TMathBigFraction;

/** Number of fraction bits of a kind with a binary point: FRACTION_BITS of fixed point, 0 for integers */
template < typename T > class CTBinaryFractionBits { public: enum { Value = 0 }; };
template < typename TStorage, UInt32 F > class CTBinaryFractionBits< CTFixedPoint< TStorage, F > > { public: enum { Value = F }; };

/*-------------------------------------------------------------------------*/

/** n / d rounded once into a fixed point or integer kind T: DivideRounded of n * 2^F by d, then range checked */
template < typename T >
inline TMathStatus
MathConvertBigFraction( const TMathBigFraction& in, T& out, TRoundingMode mode )
{
    if ( !MathIsValidRoundingMode( mode ) )
        return MATHSTATUS_INVALID_OPERAND;
    const UInt32 fractionBits = CTBinaryFractionBits< T >::Value;
    CORE::CBigInteger scaled;
    in.GetNumerator().ShiftLeft( fractionBits, scaled );
    CORE::CBigInteger quotient;
    CORE::CBigInteger remainder;
    TMathStatus status = (TMathStatus) scaled.DivideRounded( in.GetDenominator(), mode, quotient, remainder );
    if ( 0 != ( status & MATHSTATUS_ROUNDING_REQUIRED ) )
        return status;
    bool negative = quotient.IsNegative();
    CORE::CBigInteger magnitude;
    quotient.Abs( magnitude );
    UInt64 m = 0;
    TNumericValueParts parts = CORE::BIGINTEGERSTATUS_OK == magnitude.ToUInt64( m )
                                    ? MathMakeValueParts( negative, m, -(Int32) fractionBits, 1 )
                                    : MathMakeValueParts( negative, 1, 128, 1 );        // beyond every 64 bit target: saturates
    return CTNumericTraits< T >::FromParts( parts, CORE::ROUNDINGMODE_TOWARD_ZERO, out ) | status;
}

/*-------------------------------------------------------------------------*/

inline TMathStatus
MathConvertBigFraction( const TMathBigFraction& in, Float64& out, TRoundingMode mode )
{
    return (TMathStatus) CORE::CBigInteger::RatioToFloat64( in.GetNumerator(), in.GetDenominator(), mode, out );
}

inline TMathStatus
MathConvertBigFraction( const TMathBigFraction& in, Float32& out, TRoundingMode mode )
{
    return (TMathStatus) CORE::CBigInteger::RatioToFloat32( in.GetNumerator(), in.GetDenominator(), mode, out );
}

inline TMathStatus
MathConvertBigFraction( const TMathBigFraction& in, TMathBigFraction& out, TRoundingMode mode )
{
    if ( !MathIsValidRoundingMode( mode ) )
        return MATHSTATUS_INVALID_OPERAND;
    out = in;
    return MATHSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

/**
 *  Into CTFraction< Int64 >: exact when the reduced terms fit; a magnitude above the Int64 range saturates with
 *  OVERFLOW; otherwise the best rational approximation with Int64 terms (INEXACT). Terms beyond 127 bits are first
 *  truncated to 127 significant bits, a relative change below 2^-120.
 */
inline TMathStatus
MathConvertBigFraction( const TMathBigFraction& in, CORE::CTFraction< Int64, Int64 >& out, TRoundingMode mode )
{
    if ( !MathIsValidRoundingMode( mode ) )
        return MATHSTATUS_INVALID_OPERAND;
    const CORE::CBigInteger& n = in.GetNumerator();
    const CORE::CBigInteger& d = in.GetDenominator();
    Int64 n64 = 0;
    Int64 d64 = 0;
    if ( CORE::BIGINTEGERSTATUS_OK == n.ToInt64( n64 ) && CORE::BIGINTEGERSTATUS_OK == d.ToInt64( d64 ) )
    {
        out = CORE::CTFraction< Int64, Int64 >( n64, d64 );
        return MATHSTATUS_OK;
    }
    const bool negative = n.IsNegative();
    const UInt64 maxNumerator = negative ? 0x8000000000000000ULL : 0x7FFFFFFFFFFFFFFFULL;
    CORE::CBigInteger magnitude;
    n.Abs( magnitude );
    CORE::CBigInteger whole;
    CORE::CBigInteger remainder;
    magnitude.DivMod( d, whole, remainder );
    if ( whole.Compare( CORE::CBigInteger( maxNumerator ) ) > 0 || ( 0 == whole.Compare( CORE::CBigInteger( maxNumerator ) ) && !remainder.IsZero() ) )
    {
        out = CORE::CTFraction< Int64, Int64 >( MathApplySign( negative, maxNumerator ), 1 );
        return MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT;
    }
    if ( CORE::ROUNDINGMODE_UNNECESSARY == mode )
        return MATHSTATUS_ROUNDING_REQUIRED;
    UInt32 bits = magnitude.GetBitLength() > d.GetBitLength() ? magnitude.GetBitLength() : d.GetBitLength();
    CORE::CBigInteger nShifted;
    CORE::CBigInteger dShifted;
    magnitude.ShiftRight( bits > 127 ? bits - 127 : 0, nShifted );
    d.ShiftRight( bits > 127 ? bits - 127 : 0, dShifted );
    CORE::CUInt128 n128;
    CORE::CUInt128 d128;
    nShifted.ToCUInt128( n128 );
    dShifted.ToCUInt128( d128 );
    if ( d128.IsZero() )
        d128 = CORE::CUInt128( (UInt64) 1 );
    UInt64 bestN = 0;
    UInt64 bestD = 1;
    if ( n128.IsZero() )
        bestN = 0;
    else
        MathBestRationalApproximation( n128, d128, maxNumerator, 0x7FFFFFFFFFFFFFFFULL, bestN, bestD );
    out = CORE::CTFraction< Int64, Int64 >( MathApplySign( negative, bestN ), (Int64) bestD );
    return MATHSTATUS_INEXACT;
}

/*-------------------------------------------------------------------------*/

/** The value rounded once to 'scale' decimal digits (CBigInteger::RatioToDecimal); result unchanged on error */
inline TMathStatus
ToDecimal( const TMathBigFraction& value, UInt8 scale, TRoundingMode mode, CORE::CDecimal& result )
{
    if ( !MathIsValidRoundingMode( mode ) )
        return MATHSTATUS_INVALID_OPERAND;
    return (TMathStatus) CORE::CBigInteger::RatioToDecimal( value.GetNumerator(), value.GetDenominator(), CORE::CDecimalContext( scale, mode ), result );
}

/*-------------------------------------------------------------------------*/

/** As ToDecimalAutoScale of gucefMATH_DecimalConversions.h for the unbounded fractions */
inline TMathStatus
ToDecimalAutoScale( const TMathBigFraction& value, TRoundingMode mode, CORE::CDecimal& result )
{
    CORE::CDecimal candidate;
    for ( UInt8 scale=0; scale<=CORE::CDecimal::MaxScale; ++scale )
    {
        TMathStatus status = ToDecimal( value, scale, CORE::ROUNDINGMODE_UNNECESSARY, candidate );
        if ( MATHSTATUS_OK == status )
        {
            result = candidate;
            return status;
        }
        if ( 0 != ( status & ( MATHSTATUS_OVERFLOW | MATHSTATUS_INVALID_OPERAND ) ) )
            break;
    }
    TMathStatus status = MATHSTATUS_OVERFLOW;
    for ( Int32 scale=CORE::CDecimal::MaxScale; scale>=0; --scale )
    {
        status = ToDecimal( value, (UInt8) scale, mode, candidate );
        if ( !MathStatusIsError( status ) )
        {
            result = candidate;
            return status;
        }
        if ( 0 == ( status & MATHSTATUS_OVERFLOW ) )
            return status;
    }
    return status;
}

/*-------------------------------------------------------------------------*/

/** From the unbounded fractions to any kind: one rounding with the rules of CTNumericConverter */
template < typename TTo >
class CTNumericConverter< TMathBigFraction, TTo >
{
    public:

    static TMathStatus Convert( const TMathBigFraction& in, TTo& out, TRoundingMode mode )
    {
        return MathConvertBigFraction( in, out, mode );
    }
};

/*-------------------------------------------------------------------------*/

template <>
class CTNumericConverter< TMathBigFraction, CORE::CDecimal >
{
    public:

    static TMathStatus Convert( const TMathBigFraction& in, CORE::CDecimal& out, TRoundingMode mode )
    {
        return ToDecimalAutoScale( in, mode, out );
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTNUMERICTRAITSFRACTION_H ? */
