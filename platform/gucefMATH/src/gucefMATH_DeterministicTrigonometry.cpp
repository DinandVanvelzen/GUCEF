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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_SIMD_CONFIG_H
#include "gucefMATH_simd_config.h"
#define GUCEF_MATH_SIMD_CONFIG_H
#endif /* GUCEF_MATH_SIMD_CONFIG_H ? */

#include "gucefMATH_DeterministicTrigonometry.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  Every value below is a literal with 17 significant digits, which every conforming compiler converts to the same
 *  binary64 value. The low parts are the remainder of the exact value after the high part was rounded to nearest.
 */

/** pi / 4 as a high and a low part */
static const Float64 QuarterPiHigh = 0.78539816339744828;
static const Float64 QuarterPiLow = 3.061616997868383e-17;

/** sqrt( 1/2 ), the sine and the cosine of pi / 4, rounded to nearest */
static const Float64 SquareRootOfHalf = 0.70710678118654757;

/** Taylor coefficients of the sine: ( -1 )^k / ( 2k + 1 )!, high parts and the low parts of the terms that need them */
static const Float64 SineCoefficientHigh[ 15 ] =
{
    1.0,
    -0.16666666666666666,
    0.0083333333333333332,
    -0.00019841269841269841,
    2.7557319223985893e-06,
    -2.505210838544172e-08,
    1.6059043836821613e-10,
    -7.6471637318198164e-13,
    2.8114572543455206e-15,
    -8.2206352466243295e-18,
    1.9572941063391263e-20,
    -3.8681701706306841e-23,
    6.4469502843844736e-26,
    -9.183689863795546e-29,
    1.1309962886447716e-31
};

static const Float64 SineCoefficientLow[ 11 ] =
{
    0.0,
    -9.2518585385429707e-18,
    1.1564823173178714e-19,
    -1.7209558293420705e-22,
    -1.8583932740464721e-22,
    1.448814070935912e-24,
    1.2585294588752098e-26,
    -7.03872877733453e-30,
    1.6508842730861433e-31,
    -2.2141894119604265e-34,
    -1.3643503830087908e-36
};

/** Taylor coefficients of the cosine: ( -1 )^k / ( 2k )!, same layout */
static const Float64 CosineCoefficientHigh[ 15 ] =
{
    1.0,
    -0.5,
    0.041666666666666664,
    -0.0013888888888888889,
    2.4801587301587302e-05,
    -2.7557319223985888e-07,
    2.08767569878681e-09,
    -1.1470745597729725e-11,
    4.7794773323873853e-14,
    -1.5619206968586225e-16,
    4.1103176233121648e-19,
    -8.8967913924505741e-22,
    1.6117375710961184e-24,
    -2.4795962632247976e-27,
    3.2798892370698378e-30
};

static const Float64 CosineCoefficientLow[ 11 ] =
{
    0.0,
    0.0,
    2.3129646346357427e-18,
    5.3005439543735771e-20,
    2.1511947866775882e-23,
    -2.3767714622250297e-23,
    -1.20734505911326e-25,
    -2.0655512752830745e-28,
    4.3992054858340813e-31,
    -1.1910679660273754e-32,
    1.4412973378659527e-36
};

/** Number of Taylor terms, and the number of those with a low part: the later terms are too small to need one */
static const UInt32 TermCount = 15;
static const UInt32 LowPartTermCount = 11;

/*-------------------------------------------------------------------------//
//                                                                         //
//      DOUBLE-DOUBLE ARITHMETIC                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  A value is the exact sum of a high and a low part ( |low| <= ulp( high ) / 2 ). The operations use the error free
 *  transformations of Knuth and Dekker, which hold for round to nearest binary64 arithmetic without fused operations.
 *  Every function disables contraction where the compiler needs to be told so.
 */

struct SDoubleDouble
{
    Float64 high;
    Float64 low;
};

/** Exact sum a + b as high + low, for any a and b */
static inline void
TwoSum( Float64 a, Float64 b, Float64& high, Float64& low ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_FP_CONTRACT_OFF
    const Float64 sum = a + b;
    const Float64 bVirtual = sum - a;
    const Float64 aVirtual = sum - bVirtual;
    low = ( a - aVirtual ) + ( b - bVirtual );
    high = sum;
}

/*-------------------------------------------------------------------------*/

/** Exact sum a + b as high + low, for |a| >= |b| or a == 0 */
static inline void
QuickTwoSum( Float64 a, Float64 b, Float64& high, Float64& low ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_FP_CONTRACT_OFF
    const Float64 sum = a + b;
    low = b - ( sum - a );
    high = sum;
}

/*-------------------------------------------------------------------------*/

/** Splits a into two halves of 26 bits that add up to a exactly (Veltkamp) */
static inline void
SplitDouble( Float64 a, Float64& high, Float64& low ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_FP_CONTRACT_OFF
    const Float64 scaled = 134217729.0 * a;     // 2^27 + 1
    high = scaled - ( scaled - a );
    low = a - high;
}

/*-------------------------------------------------------------------------*/

/** Exact product a * b as high + low (Dekker): no fused multiply-add needed */
static inline void
TwoProduct( Float64 a, Float64 b, Float64& high, Float64& low ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_FP_CONTRACT_OFF
    const Float64 product = a * b;
    Float64 aHigh, aLow, bHigh, bLow;
    SplitDouble( a, aHigh, aLow );
    SplitDouble( b, bHigh, bLow );
    low = ( ( ( aHigh * bHigh - product ) + aHigh * bLow ) + aLow * bHigh ) + aLow * bLow;
    high = product;
}

/*-------------------------------------------------------------------------*/

static inline SDoubleDouble
DoubleDoubleAdd( const SDoubleDouble& a, const SDoubleDouble& b ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_FP_CONTRACT_OFF
    Float64 sumHigh, sumLow, lowHigh, lowLow;
    TwoSum( a.high, b.high, sumHigh, sumLow );
    TwoSum( a.low, b.low, lowHigh, lowLow );
    sumLow += lowHigh;
    QuickTwoSum( sumHigh, sumLow, sumHigh, sumLow );
    sumLow += lowLow;
    SDoubleDouble result;
    QuickTwoSum( sumHigh, sumLow, result.high, result.low );
    return result;
}

/*-------------------------------------------------------------------------*/

static inline SDoubleDouble
DoubleDoubleMultiply( const SDoubleDouble& a, const SDoubleDouble& b ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_FP_CONTRACT_OFF
    Float64 productHigh, productLow;
    TwoProduct( a.high, b.high, productHigh, productLow );
    productLow += a.high * b.low + a.low * b.high;
    SDoubleDouble result;
    QuickTwoSum( productHigh, productLow, result.high, result.low );
    return result;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The fraction numerator / denominator, with 0 < numerator <= denominator <= 2^59, as a double-double value
 *  of 107 bits of accuracy: exact integer long division, no floating point division.
 */
static SDoubleDouble
FractionAsDoubleDouble( UInt64 numerator, UInt64 denominator ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_FP_CONTRACT_OFF
    SDoubleDouble result;
    if ( numerator == denominator )
    {
        result.high = 1.0;
        result.low = 0.0;
        return result;
    }

    // scale the numerator into [ denominator / 2, denominator ): the quotient then starts with a set bit
    UInt32 shift = 0;
    while ( 2 * numerator < denominator )
    {
        numerator <<= 1;
        ++shift;
    }

    // 53 bits of quotient for the high part, the next 53 and a rounding bit for the low part
    UInt64 remainder = numerator;
    UInt64 highBits = 0;
    for ( UInt32 i = 0; i < 53; ++i )
    {
        remainder <<= 1;
        highBits <<= 1;
        if ( remainder >= denominator )
        {
            remainder -= denominator;
            highBits |= 1;
        }
    }
    UInt64 lowBits = 0;
    for ( UInt32 i = 0; i < 53; ++i )
    {
        remainder <<= 1;
        lowBits <<= 1;
        if ( remainder >= denominator )
        {
            remainder -= denominator;
            lowBits |= 1;
        }
    }
    remainder <<= 1;
    if ( remainder >= denominator )
        ++lowBits;

    // all of these scalings are by powers of two and so exact
    const Float64 twoPow53 = 9007199254740992.0;
    const Float64 scale = 1.0 / (Float64) ( ( (UInt64) 1 ) << shift );
    const Float64 high = ( (Float64) highBits / twoPow53 ) * scale;
    const Float64 low = ( ( (Float64) lowBits / twoPow53 ) / twoPow53 ) * scale;
    QuickTwoSum( high, low, result.high, result.low );
    return result;
}

/*-------------------------------------------------------------------------*/

/**
 *  Sine and cosine of the angle ( pi / 4 ) * numerator / denominator, for 0 <= numerator <= denominator, as double values
 *  that are the correctly rounded results in all but the rarest cases.
 */
static void
SinCosOfEighthTurnFraction( UInt64 numerator, UInt64 denominator, Float64& sineResult, Float64& cosineResult ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_FP_CONTRACT_OFF

    if ( 0 == numerator )
    {
        sineResult = 0.0;
        cosineResult = 1.0;
        return;
    }
    if ( numerator == denominator )
    {
        sineResult = SquareRootOfHalf;
        cosineResult = SquareRootOfHalf;
        return;
    }

    // the angle in radians, and its square
    SDoubleDouble quarterPi;
    quarterPi.high = QuarterPiHigh;
    quarterPi.low = QuarterPiLow;
    const SDoubleDouble angle = DoubleDoubleMultiply( quarterPi, FractionAsDoubleDouble( numerator, denominator ) );
    const SDoubleDouble square = DoubleDoubleMultiply( angle, angle );

    // Horner in the square: the terms beyond LowPartTermCount in plain double, the others in double-double
    Float64 sineTail = SineCoefficientHigh[ TermCount - 1 ];
    Float64 cosineTail = CosineCoefficientHigh[ TermCount - 1 ];
    for ( UInt32 k = TermCount - 1; k > LowPartTermCount; --k )
    {
        sineTail = sineTail * square.high + SineCoefficientHigh[ k - 1 ];
        cosineTail = cosineTail * square.high + CosineCoefficientHigh[ k - 1 ];
    }
    SDoubleDouble sinePolynomial;
    SDoubleDouble cosinePolynomial;
    sinePolynomial.high = sineTail;
    sinePolynomial.low = 0.0;
    cosinePolynomial.high = cosineTail;
    cosinePolynomial.low = 0.0;
    for ( UInt32 k = LowPartTermCount; k > 0; --k )
    {
        SDoubleDouble sineCoefficient;
        sineCoefficient.high = SineCoefficientHigh[ k - 1 ];
        sineCoefficient.low = SineCoefficientLow[ k - 1 ];
        SDoubleDouble cosineCoefficient;
        cosineCoefficient.high = CosineCoefficientHigh[ k - 1 ];
        cosineCoefficient.low = CosineCoefficientLow[ k - 1 ];
        sinePolynomial = DoubleDoubleAdd( DoubleDoubleMultiply( sinePolynomial, square ), sineCoefficient );
        cosinePolynomial = DoubleDoubleAdd( DoubleDoubleMultiply( cosinePolynomial, square ), cosineCoefficient );
    }

    sineResult = DoubleDoubleMultiply( angle, sinePolynomial ).high;
    cosineResult = cosinePolynomial.high;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
SinCosOfTurnFraction( Int64 numerator, Int64 denominator, Float64& sineResult, Float64& cosineResult ) GUCEF_MATH_REALTIME_CONTEXT
{
    sineResult = 0.0;
    cosineResult = 0.0;
    if ( 0 == denominator || denominator > GUCEF_MATH_TURNFRACTION_MAX_DENOMINATOR || denominator < -GUCEF_MATH_TURNFRACTION_MAX_DENOMINATOR )
        return false;

    // the sign of the fraction, and its magnitude as unsigned values (two's complement negation is defined for unsigned)
    bool negative = ( numerator < 0 );
    const UInt64 magnitude = negative ? ( (UInt64) 0 - (UInt64) numerator ) : (UInt64) numerator;
    UInt64 unit = (UInt64) ( denominator < 0 ? -denominator : denominator );
    if ( denominator < 0 )
        negative = !negative;

    // exact reduction to [ 0, 1 ) turn, then to the octant and the angle within it
    UInt64 remainder = magnitude % unit;
    if ( negative && 0 != remainder )
        remainder = unit - remainder;
    const UInt64 eighths = 8 * remainder;       // < 2^62
    const UInt32 octant = (UInt32) ( eighths / unit );
    UInt64 within = eighths - (UInt64) octant * unit;
    if ( 0 != ( octant & 1 ) )
        within = unit - within;                 // the angle is measured back from the end of an odd octant

    Float64 s, c;
    SinCosOfEighthTurnFraction( within, unit, s, c );

    // the octant decides which of the two is which and the signs; 0.0 - x keeps a zero positive
    switch ( octant )
    {
        case 0: sineResult = s;         cosineResult = c;         break;
        case 1: sineResult = c;         cosineResult = s;         break;
        case 2: sineResult = c;         cosineResult = 0.0 - s;   break;
        case 3: sineResult = s;         cosineResult = 0.0 - c;   break;
        case 4: sineResult = 0.0 - s;   cosineResult = 0.0 - c;   break;
        case 5: sineResult = 0.0 - c;   cosineResult = 0.0 - s;   break;
        case 6: sineResult = 0.0 - c;   cosineResult = s;         break;
        default: sineResult = 0.0 - s;  cosineResult = c;         break;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
SinCosOfTurnFraction( Int64 numerator, Int64 denominator, Float32& sineResult, Float32& cosineResult ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float64 s, c;
    const bool ok = SinCosOfTurnFraction( numerator, denominator, s, c );
    sineResult = (Float32) s;
    cosineResult = (Float32) c;
    return ok;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MATH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
