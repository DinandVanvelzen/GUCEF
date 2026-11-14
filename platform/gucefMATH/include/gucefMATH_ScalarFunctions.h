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

#ifndef GUCEF_MATH_SCALARFUNCTIONS_H
#define GUCEF_MATH_SCALARFUNCTIONS_H

/*
 *  Scalar functions over every numeric kind (Float32, Float64, fixed point, integers, fractions), through
 *  CTNumericTraits< T >. A function that needs a property the kind lacks (division, square root, transcendental
 *  functions) does not compile: its static assertion names the reason. Include the traits header of fixed point or
 *  fractions to use those kinds. All functions are realtime-safe, allocation free and without tracing.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CINT128_H
#include "gucefCORE_CInt128.h"
#define GUCEF_CORE_CINT128_H
#endif /* GUCEF_CORE_CINT128_H ? */

#ifndef GUCEF_MATH_CTNUMERICTRAITS_H
#include "gucefMATH_CTNumericTraits.h"
#define GUCEF_MATH_CTNUMERICTRAITS_H
#endif /* GUCEF_MATH_CTNUMERICTRAITS_H ? */

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

/** value limited to [ minValue, maxValue ] */
template < typename T >
GUCEF_MATH_FORCE_INLINE T
Clamp( const T& value, const T& minValue, const T& maxValue ) GUCEF_MATH_REALTIME_CONTEXT
{
    return value < minValue ? minValue : ( maxValue < value ? maxValue : value );
}

/*-------------------------------------------------------------------------*/

/** value limited to [ 0, 1 ] (saturate) */
template < typename T >
GUCEF_MATH_FORCE_INLINE T
Clamp01( const T& value ) GUCEF_MATH_REALTIME_CONTEXT
{
    return Clamp( value, CTNumericTraits< T >::Zero(), CTNumericTraits< T >::One() );
}

/*-------------------------------------------------------------------------*/

/** a + ( b - a ) * t, not clamped: t outside [0, 1] extrapolates */
template < typename T >
GUCEF_MATH_PRECISE_INLINE T
Lerp( const T& a, const T& b, const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    return a + ( b - a ) * t;
}

/*-------------------------------------------------------------------------*/

/** Lerp with t clamped to [0, 1] */
template < typename T >
GUCEF_MATH_PRECISE_INLINE T
LerpClamped( const T& a, const T& b, const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    return Lerp( a, b, Clamp01( t ) );
}

/*-------------------------------------------------------------------------*/

/** ( value - a ) / ( b - a ), not clamped; 0 when a == b */
template < typename T >
GUCEF_MATH_PRECISE_INLINE T
InverseLerp( const T& a, const T& b, const T& value ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( CTNumericTraits< T >::HasDivision, InverseLerp_requires_a_kind_with_division );
    if ( a == b )
        return CTNumericTraits< T >::Zero();
    return ( value - a ) / ( b - a );
}

/*-------------------------------------------------------------------------*/

/** Maps value from [ inMin, inMax ] to [ outMin, outMax ], not clamped; outMin when inMin == inMax */
template < typename T >
GUCEF_MATH_PRECISE_INLINE T
Remap( const T& value, const T& inMin, const T& inMax, const T& outMin, const T& outMax ) GUCEF_MATH_REALTIME_CONTEXT
{
    return Lerp( outMin, outMax, InverseLerp( inMin, inMax, value ) );
}

/*-------------------------------------------------------------------------*/

/** Hermite step t^2 ( 3 - 2 t ) of t = clamp01( ( x - edge0 ) / ( edge1 - edge0 ) ) */
template < typename T >
GUCEF_MATH_PRECISE_INLINE T
SmoothStep( const T& edge0, const T& edge1, const T& x ) GUCEF_MATH_REALTIME_CONTEXT
{
    typedef CTNumericTraits< T > TTraits;
    T t = Clamp01( InverseLerp( edge0, edge1, x ) );
    return t * t * ( TTraits::FromInt32( 3 ) - TTraits::Two() * t );
}

/*-------------------------------------------------------------------------*/

/** Perlin's step t^3 ( t ( 6 t - 15 ) + 10 ), zero first and second derivative at both edges */
template < typename T >
GUCEF_MATH_PRECISE_INLINE T
SmootherStep( const T& edge0, const T& edge1, const T& x ) GUCEF_MATH_REALTIME_CONTEXT
{
    typedef CTNumericTraits< T > TTraits;
    T t = Clamp01( InverseLerp( edge0, edge1, x ) );
    return t * t * t * ( t * ( t * TTraits::FromInt32( 6 ) - TTraits::FromInt32( 15 ) ) + TTraits::FromInt32( 10 ) );
}

/*-------------------------------------------------------------------------*/

/**
 *  Implementation detail of Wrap, PingPong and MoveTowards. Kinds whose values are integer multiples of one unit
 *  (integers, fixed point) are computed exactly on a 128 bit image of the value, so differences such as
 *  100 - ( -100 ) in Int8 or 3 - 5 in UInt32 cannot wrap; the other kinds use their own arithmetic.
 */
template < typename T, int IsIntegerImage >
class CTScalarPeriodicOps
{
    public:

    /** Repeats value in [ minValue, maxValue ) */
    static T Wrap( const T& value, const T& minValue, const T& maxValue ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( !( minValue < maxValue ) )
            return minValue;
        T result = minValue + CTNumericTraits< T >::ModFloor( value - minValue, maxValue - minValue );
        // the addition can round up to maxValue: the range is half-open
        if ( !( result < maxValue ) )
            result = minValue;
        return result;
    }

    static T PingPong( const T& value, const T& length ) GUCEF_MATH_REALTIME_CONTEXT
    {
        typedef CTNumericTraits< T > TTraits;
        if ( !( TTraits::Zero() < length ) )
            return TTraits::Zero();
        T t = TTraits::ModFloor( value, length * TTraits::Two() );
        return length < t ? length * TTraits::Two() - t : t;
    }

    static T MoveTowards( const T& current, const T& target, const T& maxDelta ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( current < target )
            return !( maxDelta < target - current ) ? target : current + maxDelta;
        if ( target < current )
            return !( maxDelta < current - target ) ? target : current - maxDelta;
        return target;
    }
};

/*-------------------------------------------------------------------------*/

template < typename T >
class CTScalarPeriodicOps< T, 1 >
{
    public:

    typedef CORE::CInt128 TWideImage;

    /** The value as an exact integer: the integer itself, or the raw value of fixed point */
    static TWideImage ToImage( const T& value ) GUCEF_MATH_REALTIME_CONTEXT
    {
        TNumericValueParts parts = CTNumericTraits< T >::ToParts( value );
        TWideImage image( 0, parts.magnitude );
        return parts.negative ? TWideImage::Negate( image ) : image;
    }

    /** The value of an image within the range of T */
    static T FromImage( const TWideImage& image ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const Int32 unitExponent = CTNumericTraits< T >::ToParts( CTNumericTraits< T >::Zero() ).exponent2;
        T result = CTNumericTraits< T >::Zero();
        CTNumericTraits< T >::FromParts( MathMakeValueParts( image.IsNegative(), image.GetMagnitude().low, unitExponent, 1 ), CORE::ROUNDINGMODE_TOWARD_ZERO, result );
        return result;
    }

    static TWideImage FloorModulo( const TWideImage& value, const TWideImage& period ) GUCEF_MATH_REALTIME_CONTEXT
    {
        TWideImage quotient;
        TWideImage remainder;
        TWideImage::DivModFloor( value, period, quotient, remainder );
        return remainder;
    }

    static T Wrap( const T& value, const T& minValue, const T& maxValue ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( !( minValue < maxValue ) )
            return minValue;
        TWideImage low = ToImage( minValue );
        TWideImage range = TWideImage::Subtract( ToImage( maxValue ), low );
        return FromImage( TWideImage::Add( low, FloorModulo( TWideImage::Subtract( ToImage( value ), low ), range ) ) );
    }

    static T PingPong( const T& value, const T& length ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( !( CTNumericTraits< T >::Zero() < length ) )
            return CTNumericTraits< T >::Zero();
        TWideImage half = ToImage( length );
        TWideImage period = TWideImage::Add( half, half );
        TWideImage t = FloorModulo( ToImage( value ), period );
        return FromImage( TWideImage::Compare( half, t ) < 0 ? TWideImage::Subtract( period, t ) : t );
    }

    static T MoveTowards( const T& current, const T& target, const T& maxDelta ) GUCEF_MATH_REALTIME_CONTEXT
    {
        TWideImage from = ToImage( current );
        TWideImage to = ToImage( target );
        TWideImage step = ToImage( maxDelta );
        Int32 direction = TWideImage::Compare( from, to );
        if ( 0 == direction )
            return target;
        TWideImage gap = direction < 0 ? TWideImage::Subtract( to, from ) : TWideImage::Subtract( from, to );
        if ( TWideImage::Compare( step, gap ) >= 0 )
            return target;
        return FromImage( direction < 0 ? TWideImage::Add( from, step ) : TWideImage::Subtract( from, step ) );
    }
};

/*-------------------------------------------------------------------------*/

/** Repeats value in the half-open range [ minValue, maxValue ); minValue when the range is empty */
template < typename T >
GUCEF_MATH_PRECISE_INLINE T
Wrap( const T& value, const T& minValue, const T& maxValue ) GUCEF_MATH_REALTIME_CONTEXT
{
    typedef CTNumericTraits< T > TTraits;
    return CTScalarPeriodicOps< T, TTraits::IsInteger || TTraits::IsFixedPoint >::Wrap( value, minValue, maxValue );
}

/*-------------------------------------------------------------------------*/

/** Goes back and forth between 0 and length: PingPong( 4, 3 ) == 2; 0 for a length <= 0 */
template < typename T >
GUCEF_MATH_PRECISE_INLINE T
PingPong( const T& value, const T& length ) GUCEF_MATH_REALTIME_CONTEXT
{
    typedef CTNumericTraits< T > TTraits;
    return CTScalarPeriodicOps< T, TTraits::IsInteger || TTraits::IsFixedPoint >::PingPong( value, length );
}

/*-------------------------------------------------------------------------*/

/** Implementation detail of Snap: integers divide with rounding, the other kinds round the quotient */
template < typename T, int IsInteger >
class CTSnapToStep
{
    public:
    static T Snap( const T& value, const T& step, TRoundingMode mode ) GUCEF_MATH_REALTIME_CONTEXT
    {
        typedef CTNumericTraits< T > TTraits;
        return TTraits::Round( value / step, mode ) * step;
    }
};

template < typename T >
class CTSnapToStep< T, 1 >
{
    public:
    static T Snap( const T& value, const T& step, TRoundingMode mode ) GUCEF_MATH_REALTIME_CONTEXT
    {
        T quotient = value;
        CTNumericTraits< T >::DivideRounded( value, step, mode, quotient );
        return quotient * step;
    }
};

/*-------------------------------------------------------------------------*/

/** Nearest multiple of step with 'mode'; value unchanged for a zero step */
template < typename T >
inline T
Snap( const T& value, const T& step, TRoundingMode mode ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( CTNumericTraits< T >::HasDivision || CTNumericTraits< T >::IsInteger, Snap_requires_a_kind_with_division_or_an_integer_kind );
    if ( CTNumericTraits< T >::Zero() == step )
        return value;
    return CTSnapToStep< T, CTNumericTraits< T >::IsInteger >::Snap( value, step, mode );
}

/*-------------------------------------------------------------------------*/

/** -1, 0 or +1 in the kind */
template < typename T >
GUCEF_MATH_FORCE_INLINE T
Sign( const T& value ) GUCEF_MATH_REALTIME_CONTEXT
{
    return CTNumericTraits< T >::Sign( value );
}

/*-------------------------------------------------------------------------*/

/** Moves current toward target by at most maxDelta (>= 0) without overshooting; exact for integers and fixed point */
template < typename T >
GUCEF_MATH_PRECISE_INLINE T
MoveTowards( const T& current, const T& target, const T& maxDelta ) GUCEF_MATH_REALTIME_CONTEXT
{
    typedef CTNumericTraits< T > TTraits;
    return CTScalarPeriodicOps< T, TTraits::IsInteger || TTraits::IsFixedPoint >::MoveTowards( current, target, maxDelta );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_FORCE_INLINE T
Square( const T& value ) GUCEF_MATH_REALTIME_CONTEXT
{
    return value * value;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_FORCE_INLINE T
Cube( const T& value ) GUCEF_MATH_REALTIME_CONTEXT
{
    return value * value * value;
}

/*-------------------------------------------------------------------------*/

/** Square root of the kind (IEEE correctly rounded for floats, correctly rounded for fixed point) */
template < typename T >
GUCEF_MATH_FORCE_INLINE T
Sqrt( const T& value ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( CTNumericTraits< T >::HasSqrt, Sqrt_requires_a_numeric_kind_with_sqrt );
    return CTNumericTraits< T >::Sqrt( value );
}

/*-------------------------------------------------------------------------*/

/** 1 / sqrt( value ), the core of every normalisation */
template < typename T >
GUCEF_MATH_FORCE_INLINE T
InverseSqrt( const T& value ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( CTNumericTraits< T >::HasSqrt, Normalization_requires_a_numeric_kind_with_sqrt );
    return CTNumericTraits< T >::InverseSqrt( value );
}

/*-------------------------------------------------------------------------*/

/** exp( lerp( ln a, ln b, t ) ): interpolation on a logarithmic scale, a and b > 0 (frequencies, gains) */
template < typename T >
GUCEF_MATH_PRECISE_INLINE T
LogLerp( const T& a, const T& b, const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( CTNumericTraits< T >::HasTranscendentals, LogLerp_requires_transcendentals );
    typedef CTNumericTraits< T > TTraits;
    return TTraits::Exp( Lerp( TTraits::Log( a ), TTraits::Log( b ), t ) );
}

/*-------------------------------------------------------------------------*/

/** |a - b| <= max( absolute, relative * max( |a|, |b| ) ); NaN never equals anything */
template < typename T >
GUCEF_MATH_FORCE_INLINE bool
EqualsWithin( const T& a, const T& b, const T& absolute, const T& relative ) GUCEF_MATH_REALTIME_CONTEXT
{
    return CTNumericTraits< T >::EqualsWithin( a, b, absolute, relative );
}

/*-------------------------------------------------------------------------*/

/** EqualsWithin with the default tolerances of the kind */
template < typename T >
GUCEF_MATH_FORCE_INLINE bool
Approximately( const T& a, const T& b ) GUCEF_MATH_REALTIME_CONTEXT
{
    typedef CTNumericTraits< T > TTraits;
    return TTraits::EqualsWithin( a, b, TTraits::DefaultAbsoluteTolerance(), TTraits::DefaultRelativeTolerance() );
}

/*-------------------------------------------------------------------------*/

/** Distance in units in the last place (floats: ordered bit patterns; fixed point: raw units) */
template < typename T >
GUCEF_MATH_FORCE_INLINE bool
EqualsUlp( const T& a, const T& b, UInt32 ulps ) GUCEF_MATH_REALTIME_CONTEXT
{
    return CTNumericTraits< T >::EqualsUlp( a, b, ulps );
}

/*-------------------------------------------------------------------------*/

/** value rounded to the integer kind TInt with 'mode': OVERFLOW saturates, NaN gives 0, see the status form */
template < typename TInt, typename T >
inline TMathStatus
RoundToInteger( const T& value, TRoundingMode mode, TInt& out ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( CTNumericTraits< TInt >::IsInteger, RoundToInteger_requires_an_integer_target );
    return CTNumericTraits< TInt >::FromParts( CTNumericTraits< T >::ToParts( value ), mode, out );
}

/*-------------------------------------------------------------------------*/

/** value rounded to the integer kind TInt with 'mode', saturating; 0 for NaN and for ROUNDINGMODE_UNNECESSARY when inexact */
template < typename TInt, typename T >
inline TInt
RoundToInteger( const T& value, TRoundingMode mode ) GUCEF_MATH_REALTIME_CONTEXT
{
    TInt out = TInt( 0 );
    RoundToInteger< TInt, T >( value, mode, out );
    return out;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The precision-sensitive functions above are defined with GUCEF_MATH_PRECISE_INLINE: for Float32 and Float64 the
 *  module's instantiations (gucefMATH_ScalarFunctions.cpp, compiled without floating point contraction) are used, so
 *  the results do not depend on the flags of the calling code. GUCEF_MATH_INLINE_ALL inlines them instead.
 */
#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
GUCEF_MATH_EXTERN_TEMPLATE Float32 Lerp< Float32 >( const Float32&, const Float32&, const Float32& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE Float32 LerpClamped< Float32 >( const Float32&, const Float32&, const Float32& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE Float32 InverseLerp< Float32 >( const Float32&, const Float32&, const Float32& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE Float32 Remap< Float32 >( const Float32&, const Float32&, const Float32&, const Float32&, const Float32& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE Float32 SmoothStep< Float32 >( const Float32&, const Float32&, const Float32& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE Float32 SmootherStep< Float32 >( const Float32&, const Float32&, const Float32& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE Float32 Wrap< Float32 >( const Float32&, const Float32&, const Float32& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE Float32 PingPong< Float32 >( const Float32&, const Float32& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE Float32 MoveTowards< Float32 >( const Float32&, const Float32&, const Float32& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE Float32 LogLerp< Float32 >( const Float32&, const Float32&, const Float32& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE Float64 Lerp< Float64 >( const Float64&, const Float64&, const Float64& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE Float64 LerpClamped< Float64 >( const Float64&, const Float64&, const Float64& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE Float64 InverseLerp< Float64 >( const Float64&, const Float64&, const Float64& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE Float64 Remap< Float64 >( const Float64&, const Float64&, const Float64&, const Float64&, const Float64& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE Float64 SmoothStep< Float64 >( const Float64&, const Float64&, const Float64& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE Float64 SmootherStep< Float64 >( const Float64&, const Float64&, const Float64& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE Float64 Wrap< Float64 >( const Float64&, const Float64&, const Float64& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE Float64 PingPong< Float64 >( const Float64&, const Float64& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE Float64 MoveTowards< Float64 >( const Float64&, const Float64&, const Float64& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE Float64 LogLerp< Float64 >( const Float64&, const Float64&, const Float64& ) GUCEF_MATH_REALTIME_CONTEXT;
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_SCALARFUNCTIONS_H ? */
