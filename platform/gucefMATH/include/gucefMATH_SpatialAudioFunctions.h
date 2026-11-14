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

#ifndef GUCEF_MATH_SPATIALAUDIOFUNCTIONS_H
#define GUCEF_MATH_SPATIALAUDIOFUNCTIONS_H

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

#ifndef GUCEF_MATH_CTRADIAN_H
#include "gucefMATH_CTRadian.h"
#define GUCEF_MATH_CTRADIAN_H
#endif /* GUCEF_MATH_CTRADIAN_H ? */

#ifndef GUCEF_MATH_SCALARFUNCTIONS_H
#include "gucefMATH_ScalarFunctions.h"
#define GUCEF_MATH_SCALARFUNCTIONS_H
#endif /* GUCEF_MATH_SCALARFUNCTIONS_H ? */


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
 *  Scalar functions of spatial audio: panning law, Doppler, decibel conversion and gain blending. They are the
 *  formulas of the spatial voice pipeline (OpenAL 1.1 models for distance and Doppler) as functions of plain
 *  numbers, so that engines, tests and tools evaluate them identically. T is a floating point kind. All functions
 *  are realtime safe: no allocation, no locks, no unbounded loops.
 */

/*-------------------------------------------------------------------------*/

/** 10 ^ ( dB / 20 ): the amplitude gain of a level in decibels */
template< typename T >
GUCEF_MATH_FORCE_INLINE T
DecibelsToGain( const T& decibels ) GUCEF_MATH_REALTIME_CONTEXT
{
    return std::pow( T( 10 ), decibels / T( 20 ) );
}

/*-------------------------------------------------------------------------*/

/** 20 log10( gain ) of an amplitude gain; a gain of 0 or less, or one whose level is under the floor, gives floorDb */
template< typename T >
GUCEF_MATH_FORCE_INLINE T
GainToDecibels( const T& gain, const T& floorDb ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( !( gain > T( 0 ) ) )
        return floorDb;
    const T decibels = T( 20 ) * std::log10( gain );
    return decibels < floorDb ? floorDb : decibels;
}

/*-------------------------------------------------------------------------*/

/**
 *  Constant power stereo pan: pan in [-1, 1] (-1 left, 0 centre, +1 right, clamped), angle ( pan + 1 ) pi / 4,
 *  gainLeft = scale cos( angle ), gainRight = scale sin( angle ). Both gains squared add to scale^2 for every pan.
 *  With scale = sqrt( 2 ) this is the stereo law of the spatial voices of gucefAUDIO: a centred source has the
 *  gain 1 on both channels, the same loudness as a non spatial mono voice at pan 0.
 */
template< typename T >
GUCEF_MATH_FORCE_INLINE void
ConstantPowerPan( const T& pan, const T& scale, T& gainLeft, T& gainRight ) GUCEF_MATH_REALTIME_CONTEXT
{
    const T angle = ( Clamp( pan, T( -1 ), T( 1 ) ) + T( 1 ) ) * CTMathConstants< T >::QuarterPi();
    gainLeft = scale * std::cos( angle );
    gainRight = scale * std::sin( angle );
}

/*-------------------------------------------------------------------------*/

/**
 *  Doppler pitch ratio of OpenAL 1.1 from the speeds along the axis from the source to the listener (SL = L - S):
 *  listenerSpeedAlongAxis = vL . SL / |SL| (positive when the listener moves away from the source), and
 *  sourceSpeedAlongAxis = vS . SL / |SL| (positive when the source moves toward the listener).
 *
 *  Both speeds are limited to below speedOfSound / dopplerFactor ( ( c - 1 ) / DF, as OpenAL Soft does, for
 *  c > 1 ) so the denominator never reaches zero; ratio = ( c - DF vls ) / ( c - DF vss ), clamped to
 *  [ minRatio, maxRatio ]. The ratio is 1 when dopplerFactor or speedOfSound is not positive.
 *  The vector form that derives the speeds from positions and velocities lives with the vector types.
 */
template< typename T >
GUCEF_MATH_FORCE_INLINE T
DopplerRatio( const T& listenerSpeedAlongAxis, const T& sourceSpeedAlongAxis, const T& speedOfSound, const T& dopplerFactor,
              const T& minRatio, const T& maxRatio ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( !( dopplerFactor > T( 0 ) ) || !( speedOfSound > T( 0 ) ) )
        return T( 1 );

    const T limit = speedOfSound > T( 1 ) ? ( speedOfSound - T( 1 ) ) / dopplerFactor : speedOfSound * T( 0.5 ) / dopplerFactor;
    const T vls = listenerSpeedAlongAxis < limit ? listenerSpeedAlongAxis : limit;
    const T vss = sourceSpeedAlongAxis < limit ? sourceSpeedAlongAxis : limit;
    const T ratio = ( speedOfSound - dopplerFactor * vls ) / ( speedOfSound - dopplerFactor * vss );
    return Clamp( ratio, minRatio, maxRatio );
}

/*-------------------------------------------------------------------------*/

/**
 *  Position of the angle between the emitter axis and the direction to the listener within the cone: 0 up to
 *  the half inner angle, 1 from the half outer angle, linear in the angle between ( cosAngle is clamped to
 *  [-1, 1] and converted with acos ). Use it to interpolate other properties, for example the cutoff with
 *  LogLerp( cutoffInside, cutoffOutside, factor ). An outer angle that does not exceed the inner angle gives a step
 *  at the half inner angle.
 */
template< typename T >
T
ConeInterpolationFactor( const T& cosAngle, const CTRadian< T >& innerAngle, const CTRadian< T >& outerAngle ) GUCEF_MATH_REALTIME_CONTEXT
{
    const T theta = std::acos( Clamp( cosAngle, T( -1 ), T( 1 ) ) );
    const T halfInner = innerAngle.GetValue() * T( 0.5 );
    const T halfOuter = outerAngle.GetValue() * T( 0.5 );
    if ( theta <= halfInner )
        return T( 0 );
    if ( !( halfOuter > halfInner ) || theta >= halfOuter )
        return T( 1 );
    return ( theta - halfInner ) / ( halfOuter - halfInner );
}

/*-------------------------------------------------------------------------*/

/** Cone gain: 1 inside half the inner angle, outerGain beyond half the outer angle, linear in the angle between */
template< typename T >
T
ConeGain( const T& cosAngle, const CTRadian< T >& innerAngle, const CTRadian< T >& outerAngle, const T& outerGain ) GUCEF_MATH_REALTIME_CONTEXT
{
    return Lerp( T( 1 ), outerGain, ConeInterpolationFactor( cosAngle, innerAngle, outerAngle ) );
}

/*-------------------------------------------------------------------------*/

/**
 *  Blends gains toward an even distribution over all channels and restores the total power: the gains become
 *  normalise( f g + ( 1 - f ) gUniform ) with the sum of the squares equal to targetPower, where gUniform is the same
 *  value on every channel. f is clamped to [0, 1]: 1 keeps the shape of the gains, 0 gives the uniform distribution
 *  (used near the listener, where a source has no direction). A blend of zero power falls back to the uniform
 *  distribution. The array is modified in place; count 0 does nothing.
 */
template< typename T >
void
BlendGainsTowardUniform( T* gains, UInt32 count, const T& f, const T& targetPower ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( 0 == count )
        return;

    const T keep = Clamp01( f );
    const T uniform = std::sqrt( targetPower / T( count ) );
    T power = T( 0 );
    for ( UInt32 i = 0; i < count; ++i )
    {
        gains[ i ] = keep * gains[ i ] + ( T( 1 ) - keep ) * uniform;
        power += gains[ i ] * gains[ i ];
    }

    if ( !( power > T( 0 ) ) )
    {
        for ( UInt32 i = 0; i < count; ++i )
            gains[ i ] = uniform;
        return;
    }

    const T normaliser = std::sqrt( targetPower / power );
    for ( UInt32 i = 0; i < count; ++i )
        gains[ i ] *= normaliser;
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_SPATIALAUDIOFUNCTIONS_H ? */
