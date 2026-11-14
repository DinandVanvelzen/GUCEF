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

#ifndef GUCEF_MATH_EASINGFUNCTIONS_H
#define GUCEF_MATH_EASINGFUNCTIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <cmath>

#ifndef GUCEF_MATH_MACROS_H
#include "gucefMATH_macros.h"
#define GUCEF_MATH_MACROS_H
#endif /* GUCEF_MATH_MACROS_H ? */

#ifndef GUCEF_MATH_CTMATHCONSTANTS_H
#include "gucefMATH_CTMathConstants.h"
#define GUCEF_MATH_CTMATHCONSTANTS_H
#endif /* GUCEF_MATH_CTMATHCONSTANTS_H ? */

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

/*
 *  The easing curves of Robert Penner: Linear and the families Quad, Cubic, Quart, Quint, Sine, Expo, Circ, Back,
 *  Elastic and Bounce, each in an In, Out and InOut form. Every function maps the normalised time t in [0, 1] to
 *  the progress, with f( 0 ) = 0 and f( 1 ) = 1. The input is not clamped. The InOut forms are point symmetric
 *  around ( 0.5, 0.5 ). Back uses the overshoot 1.70158 and Elastic the period 0.3, as published.
 *
 *  The functions are templates over the floating point kind. They are realtime safe.
 *  The named function exists for every enumerator of EEasingFunction (EASING_QUAD_IN is EaseInQuad); Ease()
 *  dispatches on the enumerator.
 */

template< typename T >
GUCEF_MATH_FORCE_INLINE T
EaseLinear( const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    return t;
}

/*-------------------------------------------------------------------------*/

template< typename T >
GUCEF_MATH_FORCE_INLINE T
EaseInQuad( const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    return t * t;
}

/*-------------------------------------------------------------------------*/

template< typename T >
GUCEF_MATH_FORCE_INLINE T
EaseOutQuad( const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    const T u = t - T( 1 );
    return T( 1 ) - u * u;
}

/*-------------------------------------------------------------------------*/

template< typename T >
GUCEF_MATH_FORCE_INLINE T
EaseInOutQuad( const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    const T u = t - T( 1 );
    return ( t < T( 0.5 ) ) ? ( T( 2 ) * t * t ) : ( T( 1 ) - T( 2 ) * u * u );
}

/*-------------------------------------------------------------------------*/

template< typename T >
GUCEF_MATH_FORCE_INLINE T
EaseInCubic( const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    return t * t * t;
}

/*-------------------------------------------------------------------------*/

template< typename T >
GUCEF_MATH_FORCE_INLINE T
EaseOutCubic( const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    const T u = t - T( 1 );
    return T( 1 ) + u * u * u;
}

/*-------------------------------------------------------------------------*/

template< typename T >
GUCEF_MATH_FORCE_INLINE T
EaseInOutCubic( const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    const T u = t - T( 1 );
    return ( t < T( 0.5 ) ) ? ( T( 4 ) * t * t * t ) : ( T( 1 ) + T( 4 ) * u * u * u );
}

/*-------------------------------------------------------------------------*/

template< typename T >
GUCEF_MATH_FORCE_INLINE T
EaseInQuart( const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    return t * t * t * t;
}

/*-------------------------------------------------------------------------*/

template< typename T >
GUCEF_MATH_FORCE_INLINE T
EaseOutQuart( const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    const T u = t - T( 1 );
    return T( 1 ) - u * u * u * u;
}

/*-------------------------------------------------------------------------*/

template< typename T >
GUCEF_MATH_FORCE_INLINE T
EaseInOutQuart( const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    const T u = t - T( 1 );
    return ( t < T( 0.5 ) ) ? ( T( 8 ) * t * t * t * t ) : ( T( 1 ) - T( 8 ) * u * u * u * u );
}

/*-------------------------------------------------------------------------*/

template< typename T >
GUCEF_MATH_FORCE_INLINE T
EaseInQuint( const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    return t * t * t * t * t;
}

/*-------------------------------------------------------------------------*/

template< typename T >
GUCEF_MATH_FORCE_INLINE T
EaseOutQuint( const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    const T u = t - T( 1 );
    return T( 1 ) + u * u * u * u * u;
}

/*-------------------------------------------------------------------------*/

template< typename T >
GUCEF_MATH_FORCE_INLINE T
EaseInOutQuint( const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    const T u = t - T( 1 );
    return ( t < T( 0.5 ) ) ? ( T( 16 ) * t * t * t * t * t ) : ( T( 1 ) + T( 16 ) * u * u * u * u * u );
}

/*-------------------------------------------------------------------------*/

template< typename T >
GUCEF_MATH_FORCE_INLINE T
EaseInSine( const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    return T( 1 ) - std::cos( t * CTMathConstants< T >::Pi() * T( 0.5 ) );
}

/*-------------------------------------------------------------------------*/

template< typename T >
GUCEF_MATH_FORCE_INLINE T
EaseOutSine( const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    return std::sin( t * CTMathConstants< T >::Pi() * T( 0.5 ) );
}

/*-------------------------------------------------------------------------*/

template< typename T >
GUCEF_MATH_FORCE_INLINE T
EaseInOutSine( const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    return T( -0.5 ) * ( std::cos( CTMathConstants< T >::Pi() * t ) - T( 1 ) );
}

/*-------------------------------------------------------------------------*/

template< typename T >
GUCEF_MATH_FORCE_INLINE T
EaseInExpo( const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    return ( t <= T( 0 ) ) ? T( 0 ) : std::pow( T( 2 ), T( 10 ) * t - T( 10 ) );
}

/*-------------------------------------------------------------------------*/

template< typename T >
GUCEF_MATH_FORCE_INLINE T
EaseOutExpo( const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    return ( t >= T( 1 ) ) ? T( 1 ) : T( 1 ) - std::pow( T( 2 ), T( -10 ) * t );
}

/*-------------------------------------------------------------------------*/

template< typename T >
GUCEF_MATH_FORCE_INLINE T
EaseInOutExpo( const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    return ( t <= T( 0 ) ) ? T( 0 ) : ( t >= T( 1 ) ) ? T( 1 ) :
           ( t < T( 0.5 ) ) ? std::pow( T( 2 ), T( 20 ) * t - T( 10 ) ) * T( 0.5 ) :
                              ( T( 2 ) - std::pow( T( 2 ), T( -20 ) * t + T( 10 ) ) ) * T( 0.5 );
}

/*-------------------------------------------------------------------------*/

template< typename T >
GUCEF_MATH_FORCE_INLINE T
EaseInCirc( const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    return T( 1 ) - std::sqrt( T( 1 ) - t * t );
}

/*-------------------------------------------------------------------------*/

template< typename T >
GUCEF_MATH_FORCE_INLINE T
EaseOutCirc( const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    return std::sqrt( T( 1 ) - ( t - T( 1 ) ) * ( t - T( 1 ) ) );
}

/*-------------------------------------------------------------------------*/

template< typename T >
GUCEF_MATH_FORCE_INLINE T
EaseInOutCirc( const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    return ( t < T( 0.5 ) ) ? ( T( 1 ) - std::sqrt( T( 1 ) - T( 4 ) * t * t ) ) * T( 0.5 ) :
                                   ( std::sqrt( T( 1 ) - ( T( 2 ) - T( 2 ) * t ) * ( T( 2 ) - T( 2 ) * t ) ) + T( 1 ) ) * T( 0.5 );
}

/*-------------------------------------------------------------------------*/

template< typename T >
GUCEF_MATH_FORCE_INLINE T
EaseInBack( const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    return ( T( 2.70158 ) * t - T( 1.70158 ) ) * t * t;
}

/*-------------------------------------------------------------------------*/

template< typename T >
GUCEF_MATH_FORCE_INLINE T
EaseOutBack( const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    const T u = t - T( 1 );
    return T( 1 ) + ( T( 2.70158 ) * u + T( 1.70158 ) ) * u * u;
}

/*-------------------------------------------------------------------------*/

template< typename T >
GUCEF_MATH_FORCE_INLINE T
EaseInOutBack( const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    const T c2 = T( 1.70158 ) * T( 1.525 );
    return ( t < T( 0.5 ) ) ?
           ( T( 4 ) * t * t * ( ( c2 + T( 1 ) ) * T( 2 ) * t - c2 ) ) * T( 0.5 ) :
           ( ( T( 2 ) * t - T( 2 ) ) * ( T( 2 ) * t - T( 2 ) ) * ( ( c2 + T( 1 ) ) * ( T( 2 ) * t - T( 2 ) ) + c2 ) + T( 2 ) ) * T( 0.5 );
}

/*-------------------------------------------------------------------------*/

template< typename T >
GUCEF_MATH_FORCE_INLINE T
EaseInElastic( const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    return ( t <= T( 0 ) ) ? T( 0 ) : ( t >= T( 1 ) ) ? T( 1 ) :
           -std::pow( T( 2 ), T( 10 ) * ( t - T( 1 ) ) ) * std::sin( ( t - T( 1 ) - T( 0.075 ) ) * T( 2 ) * CTMathConstants< T >::Pi() / T( 0.3 ) );
}

/*-------------------------------------------------------------------------*/

template< typename T >
GUCEF_MATH_FORCE_INLINE T
EaseOutElastic( const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    return ( t <= T( 0 ) ) ? T( 0 ) : ( t >= T( 1 ) ) ? T( 1 ) :
           std::pow( T( 2 ), T( -10 ) * t ) * std::sin( ( t - T( 0.075 ) ) * T( 2 ) * CTMathConstants< T >::Pi() / T( 0.3 ) ) + T( 1 );
}

/*-------------------------------------------------------------------------*/

template< typename T >
GUCEF_MATH_FORCE_INLINE T
EaseInOutElastic( const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    return ( t <= T( 0 ) ) ? T( 0 ) : ( t >= T( 1 ) ) ? T( 1 ) :
           ( t < T( 0.5 ) ) ?
           T( -0.5 ) * std::pow( T( 2 ), T( 10 ) * ( T( 2 ) * t - T( 1 ) ) ) * std::sin( ( T( 2 ) * t - T( 1 ) - T( 0.1125 ) ) * T( 2 ) * CTMathConstants< T >::Pi() / T( 0.45 ) ) :
           std::pow( T( 2 ), T( -10 ) * ( T( 2 ) * t - T( 1 ) ) ) * std::sin( ( T( 2 ) * t - T( 1 ) - T( 0.1125 ) ) * T( 2 ) * CTMathConstants< T >::Pi() / T( 0.45 ) ) * T( 0.5 ) + T( 1 );
}

/*-------------------------------------------------------------------------*/

template< typename T >
GUCEF_MATH_FORCE_INLINE T
EaseOutBounce( const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    return ( t < T( 1 ) / T( 2.75 ) ) ? T( 7.5625 ) * t * t :
           ( t < T( 2 ) / T( 2.75 ) ) ? T( 7.5625 ) * ( t - T( 1.5 ) / T( 2.75 ) ) * ( t - T( 1.5 ) / T( 2.75 ) ) + T( 0.75 ) :
           ( t < T( 2.5 ) / T( 2.75 ) ) ? T( 7.5625 ) * ( t - T( 2.25 ) / T( 2.75 ) ) * ( t - T( 2.25 ) / T( 2.75 ) ) + T( 0.9375 ) :
                                          T( 7.5625 ) * ( t - T( 2.625 ) / T( 2.75 ) ) * ( t - T( 2.625 ) / T( 2.75 ) ) + T( 0.984375 );
}

/*-------------------------------------------------------------------------*/

template< typename T >
GUCEF_MATH_FORCE_INLINE T
EaseInBounce( const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    return T( 1 ) - EaseOutBounce( T( 1 ) - t );
}

/*-------------------------------------------------------------------------*/

template< typename T >
GUCEF_MATH_FORCE_INLINE T
EaseInOutBounce( const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    return ( t < T( 0.5 ) ) ? ( T( 1 ) - EaseOutBounce( T( 1 ) - T( 2 ) * t ) ) * T( 0.5 ) :
                                   ( T( 1 ) + EaseOutBounce( T( 2 ) * t - T( 1 ) ) ) * T( 0.5 );
}

/*-------------------------------------------------------------------------*/

/** Evaluates the easing curve selected by the enumerator. An unknown enumerator evaluates as linear */
template< typename T >
GUCEF_MATH_FORCE_INLINE T
Ease( TEasingFunction function, const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    switch ( function )
    {
        case EASING_LINEAR:           return EaseLinear( t );
        case EASING_QUAD_IN:          return EaseInQuad( t );
        case EASING_QUAD_OUT:         return EaseOutQuad( t );
        case EASING_QUAD_INOUT:       return EaseInOutQuad( t );
        case EASING_CUBIC_IN:         return EaseInCubic( t );
        case EASING_CUBIC_OUT:        return EaseOutCubic( t );
        case EASING_CUBIC_INOUT:      return EaseInOutCubic( t );
        case EASING_QUART_IN:         return EaseInQuart( t );
        case EASING_QUART_OUT:        return EaseOutQuart( t );
        case EASING_QUART_INOUT:      return EaseInOutQuart( t );
        case EASING_QUINT_IN:         return EaseInQuint( t );
        case EASING_QUINT_OUT:        return EaseOutQuint( t );
        case EASING_QUINT_INOUT:      return EaseInOutQuint( t );
        case EASING_SINE_IN:          return EaseInSine( t );
        case EASING_SINE_OUT:         return EaseOutSine( t );
        case EASING_SINE_INOUT:       return EaseInOutSine( t );
        case EASING_EXPO_IN:          return EaseInExpo( t );
        case EASING_EXPO_OUT:         return EaseOutExpo( t );
        case EASING_EXPO_INOUT:       return EaseInOutExpo( t );
        case EASING_CIRC_IN:          return EaseInCirc( t );
        case EASING_CIRC_OUT:         return EaseOutCirc( t );
        case EASING_CIRC_INOUT:       return EaseInOutCirc( t );
        case EASING_BACK_IN:          return EaseInBack( t );
        case EASING_BACK_OUT:         return EaseOutBack( t );
        case EASING_BACK_INOUT:       return EaseInOutBack( t );
        case EASING_ELASTIC_IN:       return EaseInElastic( t );
        case EASING_ELASTIC_OUT:      return EaseOutElastic( t );
        case EASING_ELASTIC_INOUT:    return EaseInOutElastic( t );
        case EASING_BOUNCE_IN:        return EaseInBounce( t );
        case EASING_BOUNCE_OUT:       return EaseOutBounce( t );
        case EASING_BOUNCE_INOUT:     return EaseInOutBounce( t );
        default:                       return t;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_EASINGFUNCTIONS_H ? */
