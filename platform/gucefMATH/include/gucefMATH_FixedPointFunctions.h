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

#ifndef GUCEF_MATH_FIXEDPOINTFUNCTIONS_H
#define GUCEF_MATH_FIXEDPOINTFUNCTIONS_H

/*
 *  Deterministic square root, trigonometric, exponential and logarithmic functions of CTFixedPoint.
 *
 *  Everything is integer arithmetic: an internal Q2.61 format (Int64 with 61 fractional bits, range [-4, 4)) with
 *  exact 128 bit products (CInt128), and constants that are integer literals. The results are therefore the same
 *  raw bits on every compiler, CPU and operating system, in debug and release builds.
 *
 *  Constants: each literal is the value rounded to nearest (ties away from zero) in the stated format. The source is
 *  either an exact rational (1/n!, 1/(2k+1)) or a published decimal expansion:
 *      pi   = 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899862803482534211706798
 *      e    = 2.71828182845904523536028747135266249775724709369995957496696762772407663035354759457138217852516642742
 *      ln 2 = 0.69314718055994530941723212145817656807550013436025525412068000949339362196969471560586332699641868754
 *      ln10 = 2.30258509299404568401799145468436420760110148862877297603332790096757260967735248023599720508959829834
 *      sqrt 2 = 1.41421356237309504880168872420969807856967187537694807317667973799073247846210703885038753432764157273
 *      sqrt 3 = 1.73205080756887729352744634150587236694280525381038062805580697945193301690880003708114618675724857567
 *      golden ratio = 1.61803398874989484820458683436563811772030917980576286213544862270526046281890244970720720418939113748
 *  The format constant of a fixed point type is derived from the literal by one rounding shift (ties away from zero);
 *  for Q16.16 and Q32.32 every derived constant equals the correctly rounded value of the exact constant.
 *
 *  Algorithms (section 3.6 of the gucefMATH design):
 *  - Sqrt: restoring digit-by-digit integer square root of raw * 2^F, then rounded to nearest: correctly rounded.
 *  - Sin, Cos: exact range reduction to 64 fractional bits of a turn with a 128 bit constant 1/(2 pi), quadrant from
 *    the top two bits, Taylor series of sin in Horner form up to degree 11 (Int32 storage) or 15 (Int64 storage).
 *  - Atan2, Atan, Asin, Acos: ratio of the smaller to the larger magnitude, two argument halvings
 *    atan( z ) = 2 atan( z / ( 1 + sqrt( 1 + z^2 ) ) ), alternating series with 15 terms, octant fold.
 *  - Log2, Log: normalisation to m in [1/sqrt 2, sqrt 2), ln m = 2 atanh( ( m - 1 ) / ( m + 1 ) ) with 12 terms.
 *  - Exp2, Exp: integer part as a shift, the rest r in [-1/2, 1/2] through the Taylor series of e^( r ln 2 ) (degree 15).
 *  - Tan = Sin / Cos and Pow( a, b ) = Exp2( b Log2( a ) ) are formed in the internal format, rounded once.
 *  Out of range results saturate; negative square roots give 0 (TrySqrt reports INVALID_OPERAND); logarithms of
 *  values <= 0 give the lowest value; Asin and Acos clamp their input to [-1, 1].
 *
 *  All functions are realtime-safe and allocation free.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
//      CONSTANT LITERALS                                                  //
//                                                                         //
//-------------------------------------------------------------------------*/

/** pi in Q2.61 */
inline Int64 FixedQ61Pi( void ) GUCEF_MATH_REALTIME_CONTEXT { return (Int64) 0x6487ED5110B4611AULL; }
/** pi / 2 in Q2.61 */
inline Int64 FixedQ61HalfPi( void ) GUCEF_MATH_REALTIME_CONTEXT { return (Int64) 0x3243F6A8885A308DULL; }
/** 1 / pi in Q2.61 */
inline Int64 FixedQ61InversePi( void ) GUCEF_MATH_REALTIME_CONTEXT { return (Int64) 0x0A2F9836E4E44153ULL; }
/** 1 / ( 2 pi ) in Q0.128: high 64 bits */
inline UInt64 FixedQ128InverseTwoPiHigh( void ) GUCEF_MATH_REALTIME_CONTEXT { return 0x28BE60DB9391054AULL; }
/** 1 / ( 2 pi ) in Q0.128: low 64 bits */
inline UInt64 FixedQ128InverseTwoPiLow( void ) GUCEF_MATH_REALTIME_CONTEXT { return 0x7F09D5F47D4D3770ULL; }
/** ln 2 in Q2.61 */
inline Int64 FixedQ61Ln2( void ) GUCEF_MATH_REALTIME_CONTEXT { return (Int64) 0x162E42FEFA39EF35ULL; }
/** 1 / ln 2 = log2( e ) in Q2.61 */
inline Int64 FixedQ61InverseLn2( void ) GUCEF_MATH_REALTIME_CONTEXT { return (Int64) 0x2E2A8ECA5705FC2FULL; }
/** ln 10 in Q2.61 */
inline Int64 FixedQ61Ln10( void ) GUCEF_MATH_REALTIME_CONTEXT { return (Int64) 0x49AEC6EED554560BULL; }
/** e in Q2.61 */
inline Int64 FixedQ61E( void ) GUCEF_MATH_REALTIME_CONTEXT { return (Int64) 0x56FC2A2C515DA54DULL; }
/** sqrt 2 in Q2.61 */
inline Int64 FixedQ61Sqrt2( void ) GUCEF_MATH_REALTIME_CONTEXT { return (Int64) 0x2D413CCCFE779921ULL; }
/** 1 / sqrt 2 in Q2.61 */
inline Int64 FixedQ61InverseSqrt2( void ) GUCEF_MATH_REALTIME_CONTEXT { return (Int64) 0x16A09E667F3BCC91ULL; }
/** sqrt 3 in Q2.61 */
inline Int64 FixedQ61Sqrt3( void ) GUCEF_MATH_REALTIME_CONTEXT { return (Int64) 0x376CF5D0B09954E7ULL; }
/** ( 1 + sqrt 5 ) / 2 in Q2.61 */
inline Int64 FixedQ61GoldenRatio( void ) GUCEF_MATH_REALTIME_CONTEXT { return (Int64) 0x33C6EF372FE94F83ULL; }
/** 180 / pi in Q6.57 */
inline Int64 FixedQ57DegreesPerRadian( void ) GUCEF_MATH_REALTIME_CONTEXT { return (Int64) 0x729770698F07DEE2ULL; }
/** pi / 180 in Q0.64 */
inline UInt64 FixedQ64RadiansPerDegree( void ) GUCEF_MATH_REALTIME_CONTEXT { return 0x0477D1A894A74E45ULL; }

/*-------------------------------------------------------------------------*/

/** ( -1 )^k / ( 2k + 1 )! in Q2.61, k = 0 .. 7: the series of sin( y ) / y in y^2 */
inline Int64
FixedQ61SinCoefficient( UInt32 k ) GUCEF_MATH_REALTIME_CONTEXT
{
    static const Int64 coefficients[ 8 ] =
    {
        (Int64) 2305843009213693952LL ,     /*  1 / 1!  */
        -(Int64) 384307168202282325LL ,     /* -1 / 3!  */
        (Int64) 19215358410114116LL ,       /*  1 / 5!  */
        -(Int64) 457508533574146LL ,        /* -1 / 7!  */
        (Int64) 6354285188530LL ,           /*  1 / 9!  */
        -(Int64) 57766228987LL ,            /* -1 / 11! */
        (Int64) 370296340LL ,               /*  1 / 13! */
        -(Int64) 1763316LL                  /* -1 / 15! */
    };
    return coefficients[ k ];
}

/*-------------------------------------------------------------------------*/

/** 1 / k! in Q2.61, k = 0 .. 15: the Taylor series of e^y */
inline Int64
FixedQ61ExpCoefficient( UInt32 k ) GUCEF_MATH_REALTIME_CONTEXT
{
    static const Int64 coefficients[ 16 ] =
    {
        (Int64) 2305843009213693952LL , (Int64) 2305843009213693952LL , (Int64) 1152921504606846976LL ,
        (Int64) 384307168202282325LL ,  (Int64) 96076792050570581LL ,   (Int64) 19215358410114116LL ,
        (Int64) 3202559735019019LL ,    (Int64) 457508533574146LL ,     (Int64) 57188566696768LL ,
        (Int64) 6354285188530LL ,       (Int64) 635428518853LL ,        (Int64) 57766228987LL ,
        (Int64) 4813852416LL ,          (Int64) 370296340LL ,           (Int64) 26449739LL ,
        (Int64) 1763316LL
    };
    return coefficients[ k ];
}

/*-------------------------------------------------------------------------*/

/** 1 / ( 2k + 1 ) in Q2.61, k = 0 .. 14: the series of atanh (all positive) and, with alternating signs, atan */
inline Int64
FixedQ61OddReciprocal( UInt32 k ) GUCEF_MATH_REALTIME_CONTEXT
{
    static const Int64 coefficients[ 15 ] =
    {
        (Int64) 2305843009213693952LL , (Int64) 768614336404564651LL ,  (Int64) 461168601842738790LL ,
        (Int64) 329406144173384850LL ,  (Int64) 256204778801521550LL ,  (Int64) 209622091746699450LL ,
        (Int64) 177372539170284150LL ,  (Int64) 153722867280912930LL ,  (Int64) 135637824071393762LL ,
        (Int64) 121360158379668103LL ,  (Int64) 109802048057794950LL ,  (Int64) 100254043878856259LL ,
        (Int64) 92233720368547758LL ,   (Int64) 85401592933840517LL ,   (Int64) 79511827903920481LL
    };
    return coefficients[ k ];
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      INTERNAL Q2.61 KERNELS                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

/** 1.0 in Q2.61 */
GUCEF_MATH_FORCE_INLINE Int64 FixedQ61One( void ) GUCEF_MATH_REALTIME_CONTEXT { return (Int64) 1 << 61; }

/*-------------------------------------------------------------------------*/

/** Rounds v / 2^shift to nearest, ties toward +infinity (floor of v + half), shift 1..127 */
inline CORE::CInt128
FixedRoundShiftRight( const CORE::CInt128& v, UInt32 shift ) GUCEF_MATH_REALTIME_CONTEXT
{
    CORE::CInt128 half = CORE::CInt128::ShiftLeft( CORE::CInt128( (Int64) 1 ), shift - 1 );
    return CORE::CInt128::ShiftRightArithmetic( CORE::CInt128::Add( v, half ), shift );
}

/*-------------------------------------------------------------------------*/

/** a * b in Q2.61; the caller guarantees the result fits */
GUCEF_MATH_FORCE_INLINE Int64
FixedQ61Multiply( Int64 a, Int64 b ) GUCEF_MATH_REALTIME_CONTEXT
{
    return (Int64) FixedRoundShiftRight( CORE::CInt128::Multiply( a, b ), 61 ).low;
}

/*-------------------------------------------------------------------------*/

/** ( numerator << 61 ) / denominator rounded to nearest, numerator < 2^64, denominator > 0 */
inline UInt64
FixedQ61DivideMagnitude( UInt64 numerator, UInt64 denominator ) GUCEF_MATH_REALTIME_CONTEXT
{
    CORE::CUInt128 quotient;
    UInt64 remainder = 0;
    CORE::CUInt128::DivMod( CORE::CUInt128::ShiftLeft( CORE::CUInt128( numerator ), 61 ), denominator, quotient, remainder );
    UInt64 q = quotient.low;
    if ( MATHREMAINDER_BELOW_HALF < MathClassifyRemainder( remainder, denominator ) )
        ++q;
    return q;
}

/*-------------------------------------------------------------------------*/

/** sqrt( v ) in Q2.61 for v in [0, 2^62], correctly rounded */
GUCEF_MATH_FORCE_INLINE Int64
FixedQ61Sqrt( UInt64 v ) GUCEF_MATH_REALTIME_CONTEXT
{
    return CTFixedPointKernels< Int64 >::Sqrt( (Int64) v, 61 );
}

/*-------------------------------------------------------------------------*/

/**
 *  x = raw / 2^fractionBits radians as a fraction of a turn: the low 64 bits of x / ( 2 pi ) * 2^64.
 *  The 128 bit constant keeps the reduction exact to the 64 kept bits for every Int64 raw value.
 */
inline UInt64
FixedTurnsFromRaw( Int64 raw, UInt32 fractionBits ) GUCEF_MATH_REALTIME_CONTEXT
{
    CORE::CInt128 highPart = CORE::CInt128::Multiply( raw, (Int64) FixedQ128InverseTwoPiHigh() );
    CORE::CUInt128 lowMagnitude = CORE::CUInt128::Multiply( MathMagnitude( raw ), FixedQ128InverseTwoPiLow() );
    CORE::CInt128 lowPart( lowMagnitude.high, lowMagnitude.low );
    if ( raw < 0 )
        lowPart = CORE::CInt128::Negate( lowPart );
    CORE::CInt128 turns = CORE::CInt128::Add( highPart, CORE::CInt128::ShiftRightArithmetic( lowPart, 64 ) );
    turns = CORE::CInt128::ShiftRightArithmetic( turns, fractionBits );
    return turns.low;
}

/*-------------------------------------------------------------------------*/

/** sin( 2 pi turn / 2^64 ) in Q2.61 with 'terms' series terms (6: degree 11, 8: degree 15) */
inline Int64
FixedQ61SinOfTurn( UInt64 turn, UInt32 terms ) GUCEF_MATH_REALTIME_CONTEXT
{
    const UInt64 quarter = (UInt64) 1 << 62;
    UInt32 quadrant = (UInt32) ( turn >> 62 );
    UInt64 u = turn & ( quarter - 1 );
    if ( 0 != ( quadrant & 1 ) )
        u = quarter - u;
    Int64 y = (Int64) FixedRoundShiftRight( CORE::CInt128::Multiply( FixedQ61HalfPi(), (Int64) u ), 62 ).low;
    Int64 w = FixedQ61Multiply( y, y );
    Int64 sum = FixedQ61SinCoefficient( terms - 1 );
    for ( UInt32 k=terms-1; k>0; --k )
        sum = FixedQ61SinCoefficient( k - 1 ) + FixedQ61Multiply( w, sum );
    Int64 s = FixedQ61Multiply( y, sum );
    return quadrant >= 2 ? -s : s;
}

/*-------------------------------------------------------------------------*/

/** atan( numerator / denominator ) in Q2.61 for 0 <= numerator <= denominator, denominator > 0 */
inline Int64
FixedQ61AtanOfRatio( UInt64 numerator, UInt64 denominator ) GUCEF_MATH_REALTIME_CONTEXT
{
    Int64 z = (Int64) FixedQ61DivideMagnitude( numerator, denominator );
    for ( UInt32 i=0; i<2; ++i )
    {
        Int64 root = FixedQ61Sqrt( (UInt64) ( FixedQ61One() + FixedQ61Multiply( z, z ) ) );
        z = (Int64) FixedQ61DivideMagnitude( (UInt64) z, (UInt64) ( FixedQ61One() + root ) );
    }
    Int64 w = FixedQ61Multiply( z, z );
    Int64 sum = FixedQ61OddReciprocal( 14 );
    for ( UInt32 k=14; k>0; --k )
    {
        Int64 coefficient = FixedQ61OddReciprocal( k - 1 );
        if ( 0 != ( ( k - 1 ) & 1 ) )
            coefficient = -coefficient;
        sum = coefficient + FixedQ61Multiply( w, sum );
    }
    return FixedQ61Multiply( z, sum ) * 4;
}

/*-------------------------------------------------------------------------*/

/** atan2( y, x ) in Q2.61, range ( -pi, pi ], from signs and magnitudes in any common scale; ( 0, 0 ) gives 0 */
inline Int64
FixedQ61Atan2( bool yNegative, UInt64 yMagnitude, bool xNegative, UInt64 xMagnitude ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( 0 == yMagnitude && 0 == xMagnitude )
        return 0;
    bool swap = yMagnitude > xMagnitude;
    Int64 angle = swap ? FixedQ61AtanOfRatio( xMagnitude, yMagnitude ) : FixedQ61AtanOfRatio( yMagnitude, xMagnitude );
    if ( swap )
        angle = FixedQ61HalfPi() - angle;
    if ( xNegative )
        angle = FixedQ61Pi() - angle;
    return yNegative && 0 != yMagnitude ? -angle : angle;
}

/*-------------------------------------------------------------------------*/

/**
 *  Splits raw / 2^fractionBits (raw > 0) into exponent + ln( m ) / ln 2 with m in [1/sqrt 2, sqrt 2):
 *  returns ln( m ) in Q2.61 and the binary exponent.
 */
inline Int64
FixedQ61LnOfMantissa( UInt64 raw, UInt32 fractionBits, Int32& exponent ) GUCEF_MATH_REALTIME_CONTEXT
{
    UInt32 bitLength = MathBitLength( raw );
    exponent = (Int32) bitLength - 1 - (Int32) fractionBits;
    UInt64 m = bitLength - 1 > 61 ? raw >> ( bitLength - 1 - 61 ) : raw << ( 61 - ( bitLength - 1 ) );
    bool negative = false;
    UInt64 numerator = 0;
    UInt64 denominator = 0;
    if ( m > (UInt64) FixedQ61Sqrt2() )
    {
        // use m / 2: ( m/2 - 1 ) / ( m/2 + 1 ) = ( m - 2 ) / ( m + 2 )
        negative = true;
        numerator = ( (UInt64) 1 << 62 ) - m;
        denominator = m + ( (UInt64) 1 << 62 );
        ++exponent;
    }
    else
    {
        numerator = m - ( (UInt64) 1 << 61 );
        denominator = m + ( (UInt64) 1 << 61 );
    }
    Int64 w = (Int64) FixedQ61DivideMagnitude( numerator, denominator );
    if ( negative )
        w = -w;
    Int64 v = FixedQ61Multiply( w, w );
    Int64 sum = FixedQ61OddReciprocal( 11 );
    for ( UInt32 k=11; k>0; --k )
        sum = FixedQ61OddReciprocal( k - 1 ) + FixedQ61Multiply( v, sum );
    return 2 * FixedQ61Multiply( w, sum );
}

/*-------------------------------------------------------------------------*/

/** log2( raw / 2^fractionBits ) in units of 2^-61, raw > 0 */
inline CORE::CInt128
FixedQ61Log2Total( UInt64 raw, UInt32 fractionBits ) GUCEF_MATH_REALTIME_CONTEXT
{
    Int32 exponent = 0;
    Int64 lnMantissa = FixedQ61LnOfMantissa( raw, fractionBits, exponent );
    CORE::CInt128 whole = CORE::CInt128::ShiftLeft( CORE::CInt128( (Int64) exponent ), 61 );
    return CORE::CInt128::Add( whole, CORE::CInt128( FixedQ61Multiply( lnMantissa, FixedQ61InverseLn2() ) ) );
}

/*-------------------------------------------------------------------------*/

/** ln( raw / 2^fractionBits ) in units of 2^-61, raw > 0 */
inline CORE::CInt128
FixedQ61LnTotal( UInt64 raw, UInt32 fractionBits ) GUCEF_MATH_REALTIME_CONTEXT
{
    Int32 exponent = 0;
    Int64 lnMantissa = FixedQ61LnOfMantissa( raw, fractionBits, exponent );
    return CORE::CInt128::Add( CORE::CInt128::Multiply( (Int64) exponent, FixedQ61Ln2() ), CORE::CInt128( lnMantissa ) );
}

/*-------------------------------------------------------------------------*/

/**
 *  2^( t / 2^61 ) as a raw value with fractionBits, rounded to nearest (ties up), saturated to 'storageBits'.
 */
inline Int64
FixedExp2FromQ61Total( const CORE::CInt128& t, UInt32 fractionBits, UInt32 storageBits ) GUCEF_MATH_REALTIME_CONTEXT
{
    const Int64 maxRaw = (Int64) ( ( (UInt64) 1 << ( storageBits - 1 ) ) - 1 );
    CORE::CInt128 whole = FixedRoundShiftRight( t, 61 );
    Int64 n = 0;
    if ( !whole.ToInt64Checked( n ) || n > 128 )
        return whole.IsNegative() ? 0 : maxRaw;
    if ( n < -200 )
        return 0;
    Int64 r = (Int64) CORE::CInt128::Subtract( t, CORE::CInt128::ShiftLeft( whole, 61 ) ).low;
    Int64 y = FixedQ61Multiply( r, FixedQ61Ln2() );
    Int64 sum = FixedQ61ExpCoefficient( 15 );
    for ( UInt32 k=15; k>0; --k )
        sum = FixedQ61ExpCoefficient( k - 1 ) + FixedQ61Multiply( y, sum );
    UInt64 mantissa = (UInt64) sum;
    Int64 shift = n + (Int64) fractionBits - 61;
    if ( shift >= 0 )
    {
        if ( (Int64) MathBitLength( mantissa ) + shift > (Int64) storageBits - 1 )
            return maxRaw;
        return (Int64) ( mantissa << shift );
    }
    UInt64 down = (UInt64) -shift;
    if ( down >= 63 )
        return 0;
    UInt64 result = ( mantissa + ( (UInt64) 1 << ( down - 1 ) ) ) >> down;
    return result > (UInt64) maxRaw ? maxRaw : (Int64) result;
}

/*-------------------------------------------------------------------------*/

/** Rounds a value in units of 2^-sourceBits to a raw value with fractionBits (ties away from zero), saturating */
inline Int64
FixedRoundToRaw( const CORE::CInt128& value, UInt32 sourceBits, UInt32 fractionBits, UInt32 storageBits ) GUCEF_MATH_REALTIME_CONTEXT
{
    const UInt64 maxPositive = ( (UInt64) 1 << ( storageBits - 1 ) ) - 1;
    bool negative = value.IsNegative();
    CORE::CUInt128 magnitude = value.GetMagnitude();
    if ( sourceBits > fractionBits )
    {
        UInt32 shift = sourceBits - fractionBits;
        magnitude = CORE::CUInt128::Add( magnitude, CORE::CUInt128::ShiftLeft( CORE::CUInt128( (UInt64) 1 ), shift - 1 ) );
        magnitude = CORE::CUInt128::ShiftRight( magnitude, shift );
    }
    else if ( sourceBits < fractionBits )
    {
        UInt32 shift = fractionBits - sourceBits;
        if ( magnitude.BitLength() + shift > 64 )
            return negative ? MathApplySign( true, maxPositive + 1 ) : (Int64) maxPositive;
        magnitude = CORE::CUInt128::ShiftLeft( magnitude, shift );
    }
    if ( 0 != magnitude.high || magnitude.low > ( negative ? maxPositive + 1 : maxPositive ) )
        return negative ? MathApplySign( true, maxPositive + 1 ) : (Int64) maxPositive;
    return MathApplySign( negative, magnitude.low );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      FIXED POINT FUNCTIONS                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A constant literal with 'literalBits' fractional bits as the nearest value of the format, saturated */
template < typename TStorage, UInt32 F >
inline CTFixedPoint< TStorage, F >
FixedFromLiteral( Int64 literal, UInt32 literalBits ) GUCEF_MATH_REALTIME_CONTEXT
{
    return CTFixedPoint< TStorage, F >::FromRaw( (TStorage) FixedRoundToRaw( CORE::CInt128( literal ), literalBits, F, sizeof( TStorage ) * 8 ) );
}

/*-------------------------------------------------------------------------*/

/** An unsigned constant literal with 'literalBits' fractional bits (Q0.64 constants) */
template < typename TStorage, UInt32 F >
inline CTFixedPoint< TStorage, F >
FixedFromUnsignedLiteral( UInt64 literal, UInt32 literalBits ) GUCEF_MATH_REALTIME_CONTEXT
{
    return CTFixedPoint< TStorage, F >::FromRaw( (TStorage) FixedRoundToRaw( CORE::CInt128( 0, literal ), literalBits, F, sizeof( TStorage ) * 8 ) );
}

/*-------------------------------------------------------------------------*/

/** Correctly rounded square root; MATHSTATUS_INVALID_OPERAND and 0 for a negative input */
template < typename TStorage, UInt32 F >
inline TMathStatus
FixedTrySqrt( const CTFixedPoint< TStorage, F >& x, CTFixedPoint< TStorage, F >& out ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( x.GetRaw() < 0 )
    {
        out = CTFixedPoint< TStorage, F >();
        return MATHSTATUS_INVALID_OPERAND;
    }
    out = CTFixedPoint< TStorage, F >::FromRaw( CTFixedPointKernels< TStorage >::Sqrt( x.GetRaw(), F ) );
    return MATHSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

/** Correctly rounded square root; a negative input gives 0 and asserts in debug builds */
template < typename TStorage, UInt32 F >
inline CTFixedPoint< TStorage, F >
FixedSqrt( const CTFixedPoint< TStorage, F >& x ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_ASSERT( x.GetRaw() >= 0 );
    CTFixedPoint< TStorage, F > result;
    FixedTrySqrt( x, result );
    return result;
}

/*-------------------------------------------------------------------------*/

/** 1 / sqrt( x ), formed as the rounded quotient of 1 and the correctly rounded root */
template < typename TStorage, UInt32 F >
inline CTFixedPoint< TStorage, F >
FixedInverseSqrt( const CTFixedPoint< TStorage, F >& x ) GUCEF_MATH_REALTIME_CONTEXT
{
    return CTFixedPoint< TStorage, F >::DivideSaturated( CTFixedPoint< TStorage, F >( 1 ), FixedSqrt( x ) );
}

/*-------------------------------------------------------------------------*/

/** Number of sin series terms for the storage: degree 11 for Int32, degree 15 for Int64 */
template < typename TStorage >
GUCEF_MATH_FORCE_INLINE UInt32
FixedSinTerms( void ) GUCEF_MATH_REALTIME_CONTEXT
{
    return sizeof( TStorage ) == 4 ? 6 : 8;
}

/*-------------------------------------------------------------------------*/

template < typename TStorage, UInt32 F >
inline CTFixedPoint< TStorage, F >
FixedSin( const CTFixedPoint< TStorage, F >& x ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( F <= 61, Fixed_point_transcendentals_require_at_most_61_fraction_bits );
    Int64 s = FixedQ61SinOfTurn( FixedTurnsFromRaw( (Int64) x.GetRaw(), F ), FixedSinTerms< TStorage >() );
    return CTFixedPoint< TStorage, F >::FromRaw( (TStorage) FixedRoundToRaw( CORE::CInt128( s ), 61, F, sizeof( TStorage ) * 8 ) );
}

/*-------------------------------------------------------------------------*/

template < typename TStorage, UInt32 F >
inline CTFixedPoint< TStorage, F >
FixedCos( const CTFixedPoint< TStorage, F >& x ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( F <= 61, Fixed_point_transcendentals_require_at_most_61_fraction_bits );
    UInt64 turn = FixedTurnsFromRaw( (Int64) x.GetRaw(), F ) + ( (UInt64) 1 << 62 );
    Int64 c = FixedQ61SinOfTurn( turn, FixedSinTerms< TStorage >() );
    return CTFixedPoint< TStorage, F >::FromRaw( (TStorage) FixedRoundToRaw( CORE::CInt128( c ), 61, F, sizeof( TStorage ) * 8 ) );
}

/*-------------------------------------------------------------------------*/

/** sin / cos in the internal format, rounded once; the poles give the saturated value */
template < typename TStorage, UInt32 F >
inline CTFixedPoint< TStorage, F >
FixedTan( const CTFixedPoint< TStorage, F >& x ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( F <= 61, Fixed_point_transcendentals_require_at_most_61_fraction_bits );
    typedef CTFixedPoint< TStorage, F > TFixed;
    UInt64 turn = FixedTurnsFromRaw( (Int64) x.GetRaw(), F );
    Int64 s = FixedQ61SinOfTurn( turn, FixedSinTerms< TStorage >() );
    Int64 c = FixedQ61SinOfTurn( turn + ( (UInt64) 1 << 62 ), FixedSinTerms< TStorage >() );
    bool negative = ( s < 0 ) != ( c < 0 ) && 0 != s;
    if ( 0 == c )
        return s < 0 ? TFixed::Lowest() : TFixed::Highest();
    // ( |s| * 2^F ) / |c| rounded to nearest, ties away from zero
    CORE::CUInt128 quotient;
    UInt64 remainder = 0;
    UInt64 divisor = MathMagnitude( c );
    CORE::CUInt128::DivMod( CORE::CUInt128::ShiftLeft( CORE::CUInt128( MathMagnitude( s ) ), F ), divisor, quotient, remainder );
    if ( MATHREMAINDER_HALF <= MathClassifyRemainder( remainder, divisor ) )
        quotient = CORE::CUInt128::Add( quotient, CORE::CUInt128( (UInt64) 1 ) );
    const UInt64 maxPositive = ( (UInt64) 1 << ( sizeof( TStorage ) * 8 - 1 ) ) - 1;
    if ( 0 != quotient.high || quotient.low > maxPositive + ( negative ? 1 : 0 ) )
        return negative ? TFixed::Lowest() : TFixed::Highest();
    return TFixed::FromRaw( (TStorage) MathApplySign( negative, quotient.low ) );
}

/*-------------------------------------------------------------------------*/

/** atan2( y, x ) in ( -pi, pi ]; ( 0, 0 ) gives 0 */
template < typename TStorage, UInt32 F >
inline CTFixedPoint< TStorage, F >
FixedAtan2( const CTFixedPoint< TStorage, F >& y, const CTFixedPoint< TStorage, F >& x ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( F <= 61, Fixed_point_transcendentals_require_at_most_61_fraction_bits );
    Int64 angle = FixedQ61Atan2( y.GetRaw() < 0, MathMagnitude( (Int64) y.GetRaw() ), x.GetRaw() < 0, MathMagnitude( (Int64) x.GetRaw() ) );
    return CTFixedPoint< TStorage, F >::FromRaw( (TStorage) FixedRoundToRaw( CORE::CInt128( angle ), 61, F, sizeof( TStorage ) * 8 ) );
}

/*-------------------------------------------------------------------------*/

template < typename TStorage, UInt32 F >
inline CTFixedPoint< TStorage, F >
FixedAtan( const CTFixedPoint< TStorage, F >& x ) GUCEF_MATH_REALTIME_CONTEXT
{
    return FixedAtan2( x, CTFixedPoint< TStorage, F >( 1 ) );
}

/*-------------------------------------------------------------------------*/

/** x clamped to [-1, 1] in Q2.61 and sqrt( 1 - x^2 ) */
template < typename TStorage, UInt32 F >
inline void
FixedUnitCircleLegs( const CTFixedPoint< TStorage, F >& x, bool& negative, UInt64& xMagnitude, UInt64& complement ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_ASSERT( MathMagnitude( (Int64) x.GetRaw() ) <= ( (UInt64) 1 << F ) );
    negative = x.GetRaw() < 0;
    UInt64 magnitude = MathMagnitude( (Int64) x.GetRaw() );
    if ( magnitude > ( (UInt64) 1 << F ) )
        magnitude = (UInt64) 1 << F;
    xMagnitude = magnitude << ( 61 - F );
    complement = (UInt64) FixedQ61Sqrt( (UInt64) ( FixedQ61One() - FixedQ61Multiply( (Int64) xMagnitude, (Int64) xMagnitude ) ) );
}

/*-------------------------------------------------------------------------*/

/** asin( x ) = atan2( x, sqrt( 1 - x^2 ) ), input clamped to [-1, 1] (asserts in debug) */
template < typename TStorage, UInt32 F >
inline CTFixedPoint< TStorage, F >
FixedAsin( const CTFixedPoint< TStorage, F >& x ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( F <= 61, Fixed_point_transcendentals_require_at_most_61_fraction_bits );
    bool negative = false;
    UInt64 xMagnitude = 0;
    UInt64 complement = 0;
    FixedUnitCircleLegs( x, negative, xMagnitude, complement );
    Int64 angle = FixedQ61Atan2( negative, xMagnitude, false, complement );
    return CTFixedPoint< TStorage, F >::FromRaw( (TStorage) FixedRoundToRaw( CORE::CInt128( angle ), 61, F, sizeof( TStorage ) * 8 ) );
}

/*-------------------------------------------------------------------------*/

/** acos( x ) = atan2( sqrt( 1 - x^2 ), x ), input clamped to [-1, 1] (asserts in debug) */
template < typename TStorage, UInt32 F >
inline CTFixedPoint< TStorage, F >
FixedAcos( const CTFixedPoint< TStorage, F >& x ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( F <= 61, Fixed_point_transcendentals_require_at_most_61_fraction_bits );
    bool negative = false;
    UInt64 xMagnitude = 0;
    UInt64 complement = 0;
    FixedUnitCircleLegs( x, negative, xMagnitude, complement );
    Int64 angle = FixedQ61Atan2( false, complement, negative, xMagnitude );
    return CTFixedPoint< TStorage, F >::FromRaw( (TStorage) FixedRoundToRaw( CORE::CInt128( angle ), 61, F, sizeof( TStorage ) * 8 ) );
}

/*-------------------------------------------------------------------------*/

/** log2( x ); x <= 0 gives the lowest value */
template < typename TStorage, UInt32 F >
inline CTFixedPoint< TStorage, F >
FixedLog2( const CTFixedPoint< TStorage, F >& x ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( F <= 61, Fixed_point_transcendentals_require_at_most_61_fraction_bits );
    if ( x.GetRaw() <= 0 )
        return CTFixedPoint< TStorage, F >::Lowest();
    return CTFixedPoint< TStorage, F >::FromRaw( (TStorage) FixedRoundToRaw( FixedQ61Log2Total( (UInt64) x.GetRaw(), F ), 61, F, sizeof( TStorage ) * 8 ) );
}

/*-------------------------------------------------------------------------*/

/** ln( x ); x <= 0 gives the lowest value */
template < typename TStorage, UInt32 F >
inline CTFixedPoint< TStorage, F >
FixedLog( const CTFixedPoint< TStorage, F >& x ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( F <= 61, Fixed_point_transcendentals_require_at_most_61_fraction_bits );
    if ( x.GetRaw() <= 0 )
        return CTFixedPoint< TStorage, F >::Lowest();
    return CTFixedPoint< TStorage, F >::FromRaw( (TStorage) FixedRoundToRaw( FixedQ61LnTotal( (UInt64) x.GetRaw(), F ), 61, F, sizeof( TStorage ) * 8 ) );
}

/*-------------------------------------------------------------------------*/

/** 2^x, saturating above the range */
template < typename TStorage, UInt32 F >
inline CTFixedPoint< TStorage, F >
FixedExp2( const CTFixedPoint< TStorage, F >& x ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( F <= 61, Fixed_point_transcendentals_require_at_most_61_fraction_bits );
    CORE::CInt128 t = CORE::CInt128::ShiftLeft( CORE::CInt128( (Int64) x.GetRaw() ), 61 - F );
    return CTFixedPoint< TStorage, F >::FromRaw( (TStorage) FixedExp2FromQ61Total( t, F, sizeof( TStorage ) * 8 ) );
}

/*-------------------------------------------------------------------------*/

/** e^x = 2^( x log2 e ), saturating above the range */
template < typename TStorage, UInt32 F >
inline CTFixedPoint< TStorage, F >
FixedExp( const CTFixedPoint< TStorage, F >& x ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( F <= 61 && F >= 1, Fixed_point_exp_requires_1_to_61_fraction_bits );
    CORE::CInt128 t = FixedRoundShiftRight( CORE::CInt128::Multiply( (Int64) x.GetRaw(), FixedQ61InverseLn2() ), F );
    return CTFixedPoint< TStorage, F >::FromRaw( (TStorage) FixedExp2FromQ61Total( t, F, sizeof( TStorage ) * 8 ) );
}

/*-------------------------------------------------------------------------*/

/** a^b = 2^( b log2 a ) for a > 0; a^0 = 1; a <= 0 gives 0 */
template < typename TStorage, UInt32 F >
inline CTFixedPoint< TStorage, F >
FixedPow( const CTFixedPoint< TStorage, F >& a, const CTFixedPoint< TStorage, F >& b ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( F <= 61 && F >= 1, Fixed_point_pow_requires_1_to_61_fraction_bits );
    typedef CTFixedPoint< TStorage, F > TFixed;
    if ( 0 == b.GetRaw() )
        return TFixed( 1 );
    if ( a.GetRaw() <= 0 )
        return TFixed();
    CORE::CInt128 log2a = FixedQ61Log2Total( (UInt64) a.GetRaw(), F );
    CORE::CInt128 product;
    if ( !CORE::CInt128::MultiplyChecked( log2a, CORE::CInt128( (Int64) b.GetRaw() ), product ) )
        return log2a.IsNegative() != ( b.GetRaw() < 0 ) ? TFixed() : TFixed::Highest();
    CORE::CInt128 t = FixedRoundShiftRight( product, F );
    return TFixed::FromRaw( (TStorage) FixedExp2FromQ61Total( t, F, sizeof( TStorage ) * 8 ) );
}

/*-------------------------------------------------------------------------*/

/** degrees * pi / 180 with one rounding of the exact product with the pi literal, saturating */
template < typename TStorage, UInt32 F >
inline CTFixedPoint< TStorage, F >
FixedDegreesToRadians( const CTFixedPoint< TStorage, F >& degrees ) GUCEF_MATH_REALTIME_CONTEXT
{
    CORE::CInt128 quotient;
    Int64 remainder = 0;
    CORE::CInt128::DivMod( CORE::CInt128::Multiply( (Int64) degrees.GetRaw(), FixedQ61Pi() ), (Int64) 180, quotient, remainder );
    return CTFixedPoint< TStorage, F >::FromRaw( (TStorage) FixedRoundToRaw( quotient, 61 + F, F, sizeof( TStorage ) * 8 ) );
}

/*-------------------------------------------------------------------------*/

/** radians * 180 / pi with one rounding of the exact product with the 180 / pi literal, saturating */
template < typename TStorage, UInt32 F >
inline CTFixedPoint< TStorage, F >
FixedRadiansToDegrees( const CTFixedPoint< TStorage, F >& radians ) GUCEF_MATH_REALTIME_CONTEXT
{
    CORE::CInt128 product = CORE::CInt128::Multiply( (Int64) radians.GetRaw(), FixedQ57DegreesPerRadian() );
    return CTFixedPoint< TStorage, F >::FromRaw( (TStorage) FixedRoundToRaw( product, 57 + F, F, sizeof( TStorage ) * 8 ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_FIXEDPOINTFUNCTIONS_H ? */
