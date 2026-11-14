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

#ifndef GUCEF_MATH_CTNUMERICTRAITS_H
#define GUCEF_MATH_CTNUMERICTRAITS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <cmath>

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_MATH_NUMERICROUNDINGFUNCTIONS_H
#include "gucefMATH_NumericRoundingFunctions.h"
#define GUCEF_MATH_NUMERICROUNDINGFUNCTIONS_H
#endif /* GUCEF_MATH_NUMERICROUNDINGFUNCTIONS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Properties and scalar operations of a numeric kind. Every class template of gucefMATH works on its kind T only
 *  through this class, so floats, fixed point, integers and exact fractions are interchangeable where the kind
 *  supports the operation.
 *
 *  The primary template has no definition: using a type that is not a numeric kind is a compile error.
 *  Specialisations: Float32, Float64 and the built-in integers here; fixed point in
 *  gucefMATH_CTNumericTraitsFixedPoint.h; fractions in gucefMATH_CTNumericTraitsFraction.h.
 *
 *  Contract (all members static, functions inline, no state):
 *  - types TValue, TWide (accumulation type of integer products), TFloat (approximate fallback and display kind)
 *  - enum constants IsFloatingPoint, IsFixedPoint, IsInteger, IsSigned, IsExact, HasSqrt, HasTranscendentals,
 *    HasDivision, IsDeterministic, IsRealtimeSafe, IsSimdCandidate
 *  - Zero, One, Two, Half (HasDivision), Epsilon, DefaultAbsoluteTolerance, DefaultRelativeTolerance, Lowest, Max
 *  - Abs, Min, Max, Sign, IsNegative, Floor, Ceil, Truncate, Round( x, mode ), ModFloor( x, period )
 *  - Sqrt, InverseSqrt (HasSqrt); Sin, Cos, Tan, Asin, Acos, Atan, Atan2, Exp, Log, Exp2, Log2, Pow and the constants
 *    Pi, TwoPi, HalfPi, E, Ln2, Sqrt2 (HasTranscendentals)
 *  - FromInt32, FromInt64, FromFloat64, ToFloat64, ToFloat32, ToParts, FromParts, IsFinite, MultiplyAdd
 *  - ToString, FromString (locale independent, exact round trip), EqualsWithin, EqualsUlp (floats and fixed point)
 */
template < typename T >
class CTNumericTraits;

/*-------------------------------------------------------------------------*/

/**
 *  The exact value of decimal text "[+|-]digits[.digits][(e|E)[+|-]digits]" rounded once to the nearest Float32
 *  (ties to even), locale independent. false for malformed text and for values beyond the Float32 range ('value' is then
 *  unchanged); values below half the smallest subnormal give a signed zero. Not realtime-safe (exact big arithmetic).
 */
GUCEF_MATH_PUBLIC_CPP bool
MathParseDecimalFloat32( const CORE::CString& text, Float32& value );

/*-------------------------------------------------------------------------*/

/**
 *  Shared implementation of the Float32 and Float64 traits. Basic operations and the square root are IEEE correctly
 *  rounded; the transcendental functions are those of <math.h> and are therefore not deterministic across platforms.
 */
template < typename TFloatType, int SimdCandidate >
class CTFloatingPointNumericTraits
{
    public:

    typedef TFloatType TValue;
    typedef TFloatType TWide;
    typedef TFloatType TFloat;

    enum
    {
        IsFloatingPoint = 1,
        IsFixedPoint = 0,
        IsInteger = 0,
        IsSigned = 1,
        IsExact = 0,
        HasSqrt = 1,
        HasTranscendentals = 1,
        HasDivision = 1,
        IsDeterministic = 0,
        IsRealtimeSafe = 1,
        IsSimdCandidate = SimdCandidate
    };

    static GUCEF_MATH_FORCE_INLINE TValue Zero( void ) GUCEF_MATH_REALTIME_CONTEXT { return TValue( 0 ); }
    static GUCEF_MATH_FORCE_INLINE TValue One( void ) GUCEF_MATH_REALTIME_CONTEXT { return TValue( 1 ); }
    static GUCEF_MATH_FORCE_INLINE TValue Two( void ) GUCEF_MATH_REALTIME_CONTEXT { return TValue( 2 ); }
    static GUCEF_MATH_FORCE_INLINE TValue Half( void ) GUCEF_MATH_REALTIME_CONTEXT { return TValue( 0.5 ); }

    static GUCEF_MATH_FORCE_INLINE TValue Abs( TValue x ) GUCEF_MATH_REALTIME_CONTEXT { return std::fabs( x ); }
    static GUCEF_MATH_FORCE_INLINE TValue Min( TValue a, TValue b ) GUCEF_MATH_REALTIME_CONTEXT { return b < a ? b : a; }
    static GUCEF_MATH_FORCE_INLINE TValue Max( TValue a, TValue b ) GUCEF_MATH_REALTIME_CONTEXT { return a < b ? b : a; }
    static GUCEF_MATH_FORCE_INLINE bool IsNegative( TValue x ) GUCEF_MATH_REALTIME_CONTEXT { return x < TValue( 0 ); }

    /** -1, 0 or +1; NaN gives 0 */
    static GUCEF_MATH_FORCE_INLINE TValue Sign( TValue x ) GUCEF_MATH_REALTIME_CONTEXT { return x > TValue( 0 ) ? TValue( 1 ) : ( x < TValue( 0 ) ? TValue( -1 ) : TValue( 0 ) ); }

    static GUCEF_MATH_FORCE_INLINE TValue Floor( TValue x ) GUCEF_MATH_REALTIME_CONTEXT { return std::floor( x ); }
    static GUCEF_MATH_FORCE_INLINE TValue Ceil( TValue x ) GUCEF_MATH_REALTIME_CONTEXT { return std::ceil( x ); }
    static GUCEF_MATH_FORCE_INLINE TValue Truncate( TValue x ) GUCEF_MATH_REALTIME_CONTEXT { return x < TValue( 0 ) ? std::ceil( x ) : std::floor( x ); }

    /**
     *  Rounds to an integral value with 'mode'. The fraction is taken from the magnitude, where x - floor( x ) is exact,
     *  so ties are detected exactly. ROUNDINGMODE_UNNECESSARY and invalid modes truncate.
     */
    static TValue Round( TValue x, TRoundingMode mode ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( !( x == x ) )
            return x;
        bool negative = x < TValue( 0 );
        TValue magnitude = negative ? -x : x;
        TValue whole = std::floor( magnitude );
        TValue fraction = magnitude - whole;
        TMathRemainderClass remainderClass = MATHREMAINDER_ZERO;
        if ( fraction > TValue( 0 ) )
        {
            if ( fraction < TValue( 0.5 ) )
                remainderClass = MATHREMAINDER_BELOW_HALF;
            else if ( fraction == TValue( 0.5 ) )
                remainderClass = MATHREMAINDER_HALF;
            else
                remainderClass = MATHREMAINDER_ABOVE_HALF;
        }
        bool odd = std::fmod( whole, TValue( 2 ) ) != TValue( 0 );
        if ( MathRoundingIncrementsMagnitude( mode, negative, odd, remainderClass ) )
            whole += TValue( 1 );
        return negative ? -whole : whole;
    }

    /** Floored modulo: x - period * floor( x / period ), in [0, period) for a positive period */
    static TValue ModFloor( TValue x, TValue period ) GUCEF_MATH_REALTIME_CONTEXT
    {
        TValue r = std::fmod( x, period );
        if ( r != TValue( 0 ) && ( ( r < TValue( 0 ) ) != ( period < TValue( 0 ) ) ) )
        {
            r += period;
            if ( period > TValue( 0 ) ? r >= period : r <= period )
                r = TValue( 0 );
        }
        return r;
    }

    static GUCEF_MATH_FORCE_INLINE TValue Sqrt( TValue x ) GUCEF_MATH_REALTIME_CONTEXT { return std::sqrt( x ); }
    static GUCEF_MATH_FORCE_INLINE TValue InverseSqrt( TValue x ) GUCEF_MATH_REALTIME_CONTEXT { return TValue( 1 ) / std::sqrt( x ); }

    static GUCEF_MATH_FORCE_INLINE TValue Sin( TValue x ) GUCEF_MATH_REALTIME_CONTEXT { return std::sin( x ); }
    static GUCEF_MATH_FORCE_INLINE TValue Cos( TValue x ) GUCEF_MATH_REALTIME_CONTEXT { return std::cos( x ); }
    static GUCEF_MATH_FORCE_INLINE TValue Tan( TValue x ) GUCEF_MATH_REALTIME_CONTEXT { return std::tan( x ); }
    static GUCEF_MATH_FORCE_INLINE TValue Asin( TValue x ) GUCEF_MATH_REALTIME_CONTEXT { return std::asin( x ); }
    static GUCEF_MATH_FORCE_INLINE TValue Acos( TValue x ) GUCEF_MATH_REALTIME_CONTEXT { return std::acos( x ); }
    static GUCEF_MATH_FORCE_INLINE TValue Atan( TValue x ) GUCEF_MATH_REALTIME_CONTEXT { return std::atan( x ); }
    static GUCEF_MATH_FORCE_INLINE TValue Atan2( TValue y, TValue x ) GUCEF_MATH_REALTIME_CONTEXT { return std::atan2( y, x ); }
    static GUCEF_MATH_FORCE_INLINE TValue Exp( TValue x ) GUCEF_MATH_REALTIME_CONTEXT { return std::exp( x ); }
    static GUCEF_MATH_FORCE_INLINE TValue Log( TValue x ) GUCEF_MATH_REALTIME_CONTEXT { return std::log( x ); }
    static GUCEF_MATH_FORCE_INLINE TValue Pow( TValue a, TValue b ) GUCEF_MATH_REALTIME_CONTEXT { return std::pow( a, b ); }

    /** 2^x through pow, which is exact for integral x on IEEE platforms. Same code in every language standard */
    static GUCEF_MATH_FORCE_INLINE TValue Exp2( TValue x ) GUCEF_MATH_REALTIME_CONTEXT { return std::pow( TValue( 2 ), x ); }

    /** log2 as the exact binary exponent plus log2 of the mantissa in [0.5, 1): exact for powers of two */
    static TValue Log2( TValue x ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( !( x > TValue( 0 ) ) || !IsFinite( x ) )
            return std::log( x );
        int exponent = 0;
        TValue mantissa = std::frexp( x, &exponent );
        return TValue( exponent ) + std::log( mantissa ) / Ln2();
    }

    static GUCEF_MATH_FORCE_INLINE TValue Pi( void ) GUCEF_MATH_REALTIME_CONTEXT { return TValue( 3.14159265358979323846264338327950288 ); }
    static GUCEF_MATH_FORCE_INLINE TValue TwoPi( void ) GUCEF_MATH_REALTIME_CONTEXT { return TValue( 6.28318530717958647692528676655900577 ); }
    static GUCEF_MATH_FORCE_INLINE TValue HalfPi( void ) GUCEF_MATH_REALTIME_CONTEXT { return TValue( 1.57079632679489661923132169163975144 ); }
    static GUCEF_MATH_FORCE_INLINE TValue E( void ) GUCEF_MATH_REALTIME_CONTEXT { return TValue( 2.71828182845904523536028747135266250 ); }
    static GUCEF_MATH_FORCE_INLINE TValue Ln2( void ) GUCEF_MATH_REALTIME_CONTEXT { return TValue( 0.69314718055994530941723212145817657 ); }
    static GUCEF_MATH_FORCE_INLINE TValue Sqrt2( void ) GUCEF_MATH_REALTIME_CONTEXT { return TValue( 1.41421356237309504880168872420969808 ); }

    static GUCEF_MATH_FORCE_INLINE TValue FromInt32( Int32 value ) GUCEF_MATH_REALTIME_CONTEXT { return TValue( value ); }

    static TMathStatus FromInt64( Int64 value, TRoundingMode mode, TValue& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return MathRoundPartsToFloat( MathMakeValueParts( value < 0, MathMagnitude( value ), 0, 1 ), mode, out );
    }

    static GUCEF_MATH_FORCE_INLINE Float64 ToFloat64( TValue x ) GUCEF_MATH_REALTIME_CONTEXT { return (Float64) x; }
    static GUCEF_MATH_FORCE_INLINE Float32 ToFloat32( TValue x ) GUCEF_MATH_REALTIME_CONTEXT { return (Float32) x; }

    static GUCEF_MATH_FORCE_INLINE bool IsFinite( TValue x ) GUCEF_MATH_REALTIME_CONTEXT { return x == x && x - x == TValue( 0 ); }

    /** Tolerant equality: |a - b| <= max( absolute, relative * max( |a|, |b| ) ); NaN never equals anything */
    static bool EqualsWithin( TValue a, TValue b, TValue absoluteTolerance, TValue relativeTolerance ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( a == b )
            return true;
        if ( !IsFinite( a ) || !IsFinite( b ) )
            return false;
        TValue difference = std::fabs( a - b );
        TValue largest = Max( std::fabs( a ), std::fabs( b ) );
        return difference <= Max( absoluteTolerance, relativeTolerance * largest );
    }
};

/*-------------------------------------------------------------------------*/

template <>
class CTNumericTraits< Float32 > : public CTFloatingPointNumericTraits< Float32, 1 >
{
    public:

    static GUCEF_MATH_FORCE_INLINE TValue Epsilon( void ) GUCEF_MATH_REALTIME_CONTEXT { return FLT_EPSILON; }
    static GUCEF_MATH_FORCE_INLINE TValue DefaultAbsoluteTolerance( void ) GUCEF_MATH_REALTIME_CONTEXT { return 1e-6f; }
    static GUCEF_MATH_FORCE_INLINE TValue DefaultRelativeTolerance( void ) GUCEF_MATH_REALTIME_CONTEXT { return 1e-5f; }
    static GUCEF_MATH_FORCE_INLINE TValue Lowest( void ) GUCEF_MATH_REALTIME_CONTEXT { return -FLT_MAX; }
    static GUCEF_MATH_FORCE_INLINE TValue Max( void ) GUCEF_MATH_REALTIME_CONTEXT { return FLT_MAX; }
    using CTFloatingPointNumericTraits< Float32, 1 >::Max;

    /**
     *  a * b + c with two roundings in that order, never fused. Precision-sensitive (section 5.10): unless the build
     *  defines GUCEF_MATH_INLINE_ALL, it is compiled in the module (without floating point contraction) and exported,
     *  so it gives the same result whatever flags the calling code is compiled with.
     */
    #ifdef GUCEF_MATH_INLINE_ALL
    static inline TValue MultiplyAdd( TValue a, TValue b, TValue c ) GUCEF_MATH_REALTIME_CONTEXT { TValue p = a * b; return p + c; }
    #else
    static GUCEF_MATH_PUBLIC_CPP TValue MultiplyAdd( TValue a, TValue b, TValue c ) GUCEF_MATH_REALTIME_CONTEXT;
    #endif

    /** One rounding of the exact Float64 value with 'mode' (overflow as IEEE per mode, see MathRoundPartsToFloat) */
    static TMathStatus FromFloat64( Float64 value, TRoundingMode mode, TValue& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return MathRoundPartsToFloat( MathFloat64ToParts( value ), mode, out );
    }

    static GUCEF_MATH_FORCE_INLINE TNumericValueParts ToParts( TValue x ) GUCEF_MATH_REALTIME_CONTEXT { return MathFloat32ToParts( x ); }

    static TMathStatus FromParts( const TNumericValueParts& parts, TRoundingMode mode, TValue& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return MathRoundPartsToFloat( parts, mode, out );
    }

    /** Distance in units in the last place on the ordered integer line of the bit patterns (-0 and +0 are 0 apart) */
    static bool EqualsUlp( TValue a, TValue b, UInt32 ulps ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( !( a == a ) || !( b == b ) )
            return false;
        Int64 ia = OrderedBits( a );
        Int64 ib = OrderedBits( b );
        UInt64 distance = ia < ib ? (UInt64) ( ib - ia ) : (UInt64) ( ia - ib );
        return distance <= ulps;
    }

    /** Shortest text with 6 to 9 significant digits that reads back to the same value; "nan", "inf", "-inf" */
    static CORE::CString ToString( TValue x )
    {
        if ( !( x == x ) )
            return "nan";
        if ( !IsFinite( x ) )
            return x < 0 ? "-inf" : "inf";
        CORE::CString text;
        for ( int precision = 6; precision <= 9; ++precision )
        {
            // "%.9g" of a float needs at most 16 characters: sprintf (C89) is safe here and needs no C99 snprintf
            char buffer[ 48 ];
            int length = sprintf( buffer, "%.*g", precision, (Float64) x );
            if ( length <= 0 || length >= (int) sizeof( buffer ) )
                return CORE::CString::Empty;
            for ( int i=0; i<length; ++i )
                if ( ',' == buffer[ i ] )
                    buffer[ i ] = '.';
            text = buffer;
            Float32 readBack = 0.0f;
            if ( MathParseDecimalFloat32( text, readBack ) && readBack == x )
                break;
        }
        return text;
    }

    /**
     *  Parses "[+|-]digits[.digits][(e|E)[+|-]digits]" (and "nan", "inf", "-inf"), locale independent, and rounds the
     *  exact decimal value once to the nearest Float32 (ties to even). Returns false for malformed text and for values
     *  beyond the Float32 range, leaving 'out' unchanged.
     */
    static bool FromString( const CORE::CString& text, TValue& out )
    {
        if ( text == "nan" || text == "inf" || text == "-inf" )
        {
            Float64 special = 0.0;
            ParseFloatText( text, special );
            out = (Float32) special;
            return true;
        }
        return MathParseDecimalFloat32( text, out );
    }

    /** Parses decimal text and the words nan, inf, -inf */
    static bool ParseFloatText( const CORE::CString& text, Float64& value )
    {
        if ( text == "nan" ) { value = MathQuietNaN64(); return true; }
        if ( text == "inf" ) { value = MathInfinity64(); return true; }
        if ( text == "-inf" ) { value = -MathInfinity64(); return true; }
        return CORE::TryParseFloat64Invariant( text, value );
    }

    private:

    static Int64 OrderedBits( TValue x ) GUCEF_MATH_REALTIME_CONTEXT
    {
        UInt32 bits = 0;
        memcpy( &bits, &x, sizeof( bits ) );
        Int64 magnitude = (Int64) ( bits & 0x7FFFFFFFu );
        return 0 != ( bits & 0x80000000u ) ? -magnitude : magnitude;
    }
};

/*-------------------------------------------------------------------------*/

template <>
class CTNumericTraits< Float64 > : public CTFloatingPointNumericTraits< Float64, 0 >
{
    public:

    static GUCEF_MATH_FORCE_INLINE TValue Epsilon( void ) GUCEF_MATH_REALTIME_CONTEXT { return DBL_EPSILON; }
    static GUCEF_MATH_FORCE_INLINE TValue DefaultAbsoluteTolerance( void ) GUCEF_MATH_REALTIME_CONTEXT { return 1e-12; }
    static GUCEF_MATH_FORCE_INLINE TValue DefaultRelativeTolerance( void ) GUCEF_MATH_REALTIME_CONTEXT { return 1e-10; }
    static GUCEF_MATH_FORCE_INLINE TValue Lowest( void ) GUCEF_MATH_REALTIME_CONTEXT { return -DBL_MAX; }
    static GUCEF_MATH_FORCE_INLINE TValue Max( void ) GUCEF_MATH_REALTIME_CONTEXT { return DBL_MAX; }
    using CTFloatingPointNumericTraits< Float64, 0 >::Max;

    /**
     *  a * b + c with two roundings in that order, never fused. Precision-sensitive (section 5.10): unless the build
     *  defines GUCEF_MATH_INLINE_ALL, it is compiled in the module (without floating point contraction) and exported,
     *  so it gives the same result whatever flags the calling code is compiled with.
     */
    #ifdef GUCEF_MATH_INLINE_ALL
    static inline TValue MultiplyAdd( TValue a, TValue b, TValue c ) GUCEF_MATH_REALTIME_CONTEXT { TValue p = a * b; return p + c; }
    #else
    static GUCEF_MATH_PUBLIC_CPP TValue MultiplyAdd( TValue a, TValue b, TValue c ) GUCEF_MATH_REALTIME_CONTEXT;
    #endif

    static TMathStatus FromFloat64( Float64 value, TRoundingMode mode, TValue& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( !MathIsValidRoundingMode( mode ) )
            return MATHSTATUS_INVALID_OPERAND;
        out = value;
        return MATHSTATUS_OK;
    }

    static GUCEF_MATH_FORCE_INLINE TNumericValueParts ToParts( TValue x ) GUCEF_MATH_REALTIME_CONTEXT { return MathFloat64ToParts( x ); }

    static TMathStatus FromParts( const TNumericValueParts& parts, TRoundingMode mode, TValue& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return MathRoundPartsToFloat( parts, mode, out );
    }

    static bool EqualsUlp( TValue a, TValue b, UInt32 ulps ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( !( a == a ) || !( b == b ) )
            return false;
        UInt64 ma = 0;
        UInt64 mb = 0;
        bool na = Split( a, ma );
        bool nb = Split( b, mb );
        UInt64 distance = 0;
        if ( na == nb )
            distance = ma < mb ? mb - ma : ma - mb;
        else
        {
            distance = ma + mb;
            if ( distance < ma )
                return false;
        }
        return distance <= ulps;
    }

    /** Shortest of 15 to 17 significant digits that reads back exactly; "nan", "inf", "-inf" */
    static CORE::CString ToString( TValue x )
    {
        if ( !( x == x ) )
            return "nan";
        if ( !IsFinite( x ) )
            return x < 0 ? "-inf" : "inf";
        return CORE::Float64ToRoundTripString( x );
    }

    /** Parses plain decimal text (and "nan", "inf", "-inf"). Returns false for malformed text, 'out' unchanged */
    static bool FromString( const CORE::CString& text, TValue& out )
    {
        return CTNumericTraits< Float32 >::ParseFloatText( text, out );
    }

    private:

    static bool Split( TValue x, UInt64& magnitude ) GUCEF_MATH_REALTIME_CONTEXT
    {
        UInt64 bits = 0;
        memcpy( &bits, &x, sizeof( bits ) );
        magnitude = bits & 0x7FFFFFFFFFFFFFFFULL;
        return 0 != ( bits >> 63 );
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  Shared implementation of the integer traits. +, - and * wrap as the built-in type does (MultiplyAdd computes in the
 *  unsigned image so the wrap is defined). Integers have no division in the field sense, no square root and no
 *  transcendental functions: those members do not exist, so their use is a compile error.
 */
template < typename TInt, typename TUnsignedInt, typename TWideInt, typename TFloatKind, int Signed, int Bits >
class CTIntegerNumericTraits
{
    public:

    typedef TInt TValue;
    typedef TWideInt TWide;
    typedef TFloatKind TFloat;

    enum
    {
        IsFloatingPoint = 0,
        IsFixedPoint = 0,
        IsInteger = 1,
        IsSigned = Signed,
        IsExact = 1,
        HasSqrt = 0,
        HasTranscendentals = 0,
        HasDivision = 0,
        IsDeterministic = 1,
        IsRealtimeSafe = 1,
        IsSimdCandidate = 0
    };

    static GUCEF_MATH_FORCE_INLINE TValue Zero( void ) GUCEF_MATH_REALTIME_CONTEXT { return TValue( 0 ); }
    static GUCEF_MATH_FORCE_INLINE TValue One( void ) GUCEF_MATH_REALTIME_CONTEXT { return TValue( 1 ); }
    static GUCEF_MATH_FORCE_INLINE TValue Two( void ) GUCEF_MATH_REALTIME_CONTEXT { return TValue( 2 ); }
    static GUCEF_MATH_FORCE_INLINE TValue Epsilon( void ) GUCEF_MATH_REALTIME_CONTEXT { return TValue( 0 ); }
    static GUCEF_MATH_FORCE_INLINE TValue DefaultAbsoluteTolerance( void ) GUCEF_MATH_REALTIME_CONTEXT { return TValue( 0 ); }
    static GUCEF_MATH_FORCE_INLINE TValue DefaultRelativeTolerance( void ) GUCEF_MATH_REALTIME_CONTEXT { return TValue( 0 ); }

    static GUCEF_MATH_FORCE_INLINE TValue Lowest( void ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return Signed ? (TValue) MathApplySign( true, (UInt64) 1 << ( Bits - 1 ) ) : TValue( 0 );
    }

    static GUCEF_MATH_FORCE_INLINE TValue Max( void ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return (TValue) ( Signed ? ( ( (UInt64) 1 << ( Bits - 1 ) ) - 1 ) : ( ~(UInt64) 0 >> ( 64 - Bits ) ) );
    }

    /** |x|; the magnitude of the minimum of a signed type is not representable and wraps to the minimum */
    static GUCEF_MATH_FORCE_INLINE TValue Abs( TValue x ) GUCEF_MATH_REALTIME_CONTEXT { return IsNegative( x ) ? (TValue) ( (TUnsignedInt) 0 - (TUnsignedInt) x ) : x; }
    static GUCEF_MATH_FORCE_INLINE TValue Min( TValue a, TValue b ) GUCEF_MATH_REALTIME_CONTEXT { return b < a ? b : a; }
    static GUCEF_MATH_FORCE_INLINE TValue Max( TValue a, TValue b ) GUCEF_MATH_REALTIME_CONTEXT { return a < b ? b : a; }
    static GUCEF_MATH_FORCE_INLINE bool IsNegative( TValue x ) GUCEF_MATH_REALTIME_CONTEXT { return Signed && x < TValue( 0 ); }
    static GUCEF_MATH_FORCE_INLINE TValue Sign( TValue x ) GUCEF_MATH_REALTIME_CONTEXT { return IsNegative( x ) ? (TValue) -1 : ( x == TValue( 0 ) ? TValue( 0 ) : TValue( 1 ) ); }

    /** Integral already: identity */
    static GUCEF_MATH_FORCE_INLINE TValue Floor( TValue x ) GUCEF_MATH_REALTIME_CONTEXT { return x; }
    static GUCEF_MATH_FORCE_INLINE TValue Ceil( TValue x ) GUCEF_MATH_REALTIME_CONTEXT { return x; }
    static GUCEF_MATH_FORCE_INLINE TValue Truncate( TValue x ) GUCEF_MATH_REALTIME_CONTEXT { return x; }
    static GUCEF_MATH_FORCE_INLINE TValue Round( TValue x, TRoundingMode /*mode*/ ) GUCEF_MATH_REALTIME_CONTEXT { return x; }

    /** Floored modulo, in [0, period) for a positive period. period must not be 0 */
    static TValue ModFloor( TValue x, TValue period ) GUCEF_MATH_REALTIME_CONTEXT
    {
        // every integer is a multiple of -1; the lowest value % -1 would trap
        if ( Signed && (TValue) -1 == period )
            return TValue( 0 );
        TValue r = (TValue) ( x % period );
        if ( r != TValue( 0 ) && ( IsNegative( r ) != IsNegative( period ) ) )
            r = (TValue) ( r + period );
        return r;
    }

    /**
     *  a / b rounded with 'mode' (exact, through the magnitudes). Division by zero gives DIVISION_BY_ZERO and leaves
     *  'out' unchanged, as do ROUNDING_REQUIRED and an invalid mode; a result outside the type saturates with OVERFLOW.
     */
    static TMathStatus DivideRounded( TValue a, TValue b, TRoundingMode mode, TValue& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( TValue( 0 ) == b )
            return MATHSTATUS_DIVISION_BY_ZERO;
        bool negative = IsNegative( a ) != IsNegative( b );
        UInt64 magnitudeA = IsNegative( a ) ? MathMagnitude( (Int64) a ) : (UInt64) a;
        UInt64 magnitudeB = IsNegative( b ) ? MathMagnitude( (Int64) b ) : (UInt64) b;
        return FromParts( MathMakeValueParts( negative, magnitudeA, 0, magnitudeB ), mode, out );
    }

    static GUCEF_MATH_FORCE_INLINE TValue FromInt32( Int32 value ) GUCEF_MATH_REALTIME_CONTEXT { return (TValue) value; }

    /** Range checked; OVERFLOW saturates */
    static TMathStatus FromInt64( Int64 value, TRoundingMode mode, TValue& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return FromParts( MathMakeValueParts( value < 0, MathMagnitude( value ), 0, 1 ), mode, out );
    }

    /** One rounding with 'mode', saturating; NaN and infinity give 0 with INVALID_OPERAND */
    static TMathStatus FromFloat64( Float64 value, TRoundingMode mode, TValue& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return FromParts( MathFloat64ToParts( value ), mode, out );
    }

    static GUCEF_MATH_FORCE_INLINE Float64 ToFloat64( TValue x ) GUCEF_MATH_REALTIME_CONTEXT { return (Float64) x; }
    static GUCEF_MATH_FORCE_INLINE Float32 ToFloat32( TValue x ) GUCEF_MATH_REALTIME_CONTEXT { return (Float32) x; }
    static GUCEF_MATH_FORCE_INLINE bool IsFinite( TValue /*x*/ ) GUCEF_MATH_REALTIME_CONTEXT { return true; }

    static GUCEF_MATH_FORCE_INLINE TValue MultiplyAdd( TValue a, TValue b, TValue c ) GUCEF_MATH_REALTIME_CONTEXT
    {
        // computed in 64 bit unsigned arithmetic: narrow types would otherwise promote to int, where the product can
        // overflow; the wrap modulo 2^Bits is the same
        return (TValue) (TUnsignedInt) ( (UInt64) (Int64) a * (UInt64) (Int64) b + (UInt64) (Int64) c );
    }

    static GUCEF_MATH_FORCE_INLINE TNumericValueParts ToParts( TValue x ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return MathMakeValueParts( IsNegative( x ), IsNegative( x ) ? MathMagnitude( (Int64) x ) : (UInt64) x, 0, 1 );
    }

    static TMathStatus FromParts( const TNumericValueParts& parts, TRoundingMode mode, TValue& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( !MathIsValidRoundingMode( mode ) )
            return MATHSTATUS_INVALID_OPERAND;
        TMathStatus status = MATHSTATUS_OK;
        if ( Signed )
        {
            Int64 value = 0;
            status = MathRoundPartsToSigned( parts, 0, Bits, mode, value );
            if ( 0 == ( status & MATHSTATUS_ROUNDING_REQUIRED ) )
                out = (TValue) value;
        }
        else
        {
            UInt64 value = 0;
            status = MathRoundPartsToUnsigned( parts, Bits, mode, value );
            if ( 0 == ( status & MATHSTATUS_ROUNDING_REQUIRED ) )
                out = (TValue) value;
        }
        return status;
    }

    /** |a - b| <= max( absolute, relative * max( |a|, |b| ) ), computed without overflow */
    static bool EqualsWithin( TValue a, TValue b, TValue absoluteTolerance, TValue relativeTolerance ) GUCEF_MATH_REALTIME_CONTEXT
    {
        UInt64 ma = IsNegative( a ) ? MathMagnitude( (Int64) a ) : (UInt64) a;
        UInt64 mb = IsNegative( b ) ? MathMagnitude( (Int64) b ) : (UInt64) b;
        UInt64 distance = 0;
        if ( IsNegative( a ) == IsNegative( b ) )
            distance = ma < mb ? mb - ma : ma - mb;
        else
            distance = ma + mb;     // at most 2^64 - 1 for 64 bit types
        if ( !IsNegative( absoluteTolerance ) && distance <= (UInt64) absoluteTolerance )
            return true;
        if ( IsNegative( relativeTolerance ) || TValue( 0 ) == relativeTolerance )
            return false;
        CORE::CUInt128 limit = CORE::CUInt128::Multiply( (UInt64) relativeTolerance, ma < mb ? mb : ma );
        return CORE::CUInt128::Compare( CORE::CUInt128( distance ), limit ) <= 0;
    }

    static CORE::CString ToString( TValue x )
    {
        return Signed ? CORE::Int64ToString( (Int64) x ) : CORE::UInt64ToString( (UInt64) x );
    }

    /** Strict: optional sign, decimal digits, nothing else; range checked. false leaves 'out' unchanged */
    static bool FromString( const CORE::CString& text, TValue& out )
    {
        const char* data = text.C_String();
        UInt32 length = text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
        if ( GUCEF_NULL == data || 0 == length )
            return false;
        UInt32 i = 0;
        bool negative = false;
        if ( '+' == data[ 0 ] || '-' == data[ 0 ] )
        {
            negative = '-' == data[ 0 ];
            ++i;
        }
        if ( i >= length )
            return false;
        UInt64 magnitude = 0;
        for ( ; i<length; ++i )
        {
            char c = data[ i ];
            if ( c < '0' || c > '9' )
                return false;
            UInt64 digit = (UInt64) ( c - '0' );
            if ( magnitude > ( ~(UInt64) 0 - digit ) / 10 )
                return false;
            magnitude = magnitude * 10 + digit;
        }
        TValue value = 0;
        if ( MATHSTATUS_OK != FromParts( MathMakeValueParts( negative, magnitude, 0, 1 ), CORE::ROUNDINGMODE_UNNECESSARY, value ) )
            return false;
        out = value;
        return true;
    }
};

/*-------------------------------------------------------------------------*/

template <> class CTNumericTraits< Int8 > : public CTIntegerNumericTraits< Int8, UInt8, Int32, Float32, 1, 8 > {};
template <> class CTNumericTraits< Int16 > : public CTIntegerNumericTraits< Int16, UInt16, Int32, Float32, 1, 16 > {};
template <> class CTNumericTraits< Int32 > : public CTIntegerNumericTraits< Int32, UInt32, Int64, Float64, 1, 32 > {};
template <> class CTNumericTraits< Int64 > : public CTIntegerNumericTraits< Int64, UInt64, Int64, Float64, 1, 64 > {};
template <> class CTNumericTraits< UInt8 > : public CTIntegerNumericTraits< UInt8, UInt8, UInt32, Float32, 0, 8 > {};
template <> class CTNumericTraits< UInt16 > : public CTIntegerNumericTraits< UInt16, UInt16, UInt32, Float32, 0, 16 > {};
template <> class CTNumericTraits< UInt32 > : public CTIntegerNumericTraits< UInt32, UInt32, UInt64, Float64, 0, 32 > {};
template <> class CTNumericTraits< UInt64 > : public CTIntegerNumericTraits< UInt64, UInt64, UInt64, Float64, 0, 64 > {};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTNUMERICTRAITS_H ? */
