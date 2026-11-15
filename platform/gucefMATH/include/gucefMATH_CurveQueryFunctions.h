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

#ifndef GUCEF_MATH_CURVEQUERYFUNCTIONS_H
#define GUCEF_MATH_CURVEQUERYFUNCTIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <cmath>
#include <limits>

#ifndef GUCEF_MATH_CTCURVEPOINTTRAITS_H
#include "gucefMATH_CTCurvePointTraits.h"
#define GUCEF_MATH_CTCURVEPOINTTRAITS_H
#endif /* GUCEF_MATH_CTCURVEPOINTTRAITS_H ? */

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
 *  Closest point on a parametric curve. The curve is sampled at 'sampleCount' + 1 parameters into candidate intervals
 *  (every sample that is not further from the query point than both neighbours, with the ends included); each
 *  candidate is refined on its interval with Newton iterations on f( t ) = ( C( t ) - p ) . C'( t ), which is zero at
 *  a stationary distance. No second derivative is needed: the slope of f is C'( t ) . C'( t ) (Gauss-Newton) for the
 *  first step and the secant through the last two iterates after that; a step that leaves the bracket, which the sign
 *  of f maintains, is replaced by bisection. The
 *  nearest refined candidate wins. A minimum at the end of the parameter range is found as such.
 *
 *  The curve is any type with TTraits (a CTCurvePointTraits), TScalar, TVector, Evaluate, GetDerivative, GetMinParameter and GetMaxParameter
 *  (all curves of gucefMATH). 'sampleCount' should cover the curve's wiggles: a few samples per segment of a spline;
 *  the default of 64 suits a single Bezier curve. A curve that comes back close to the query point several times is
 *  resolved only if the samples separate the passes. No allocation (candidates are processed one by one): realtime-safe.
 *
 *  Returns false when the curve has an empty parameter range or the point is not finite; the outputs are then unchanged.
 */
template < typename TCurve >
bool
ClosestPointOnCurve( const TCurve& curve, const typename TCurve::TTraits::TPoint& point, typename TCurve::TScalar& parameter, typename TCurve::TTraits::TPoint& closest,
                     UInt32 sampleCount = 64 ) GUCEF_MATH_REALTIME_CONTEXT
{
    typedef typename TCurve::TTraits TTraits;
    typedef typename TCurve::TScalar TScalar;
    typedef typename TCurve::TTraits::TPoint TPoint;
    typedef typename TCurve::TVector TVector;

    const TScalar start = curve.GetMinParameter();
    const TScalar end = curve.GetMaxParameter();
    if ( !( end > start ) )
        return false;
    const UInt32 samples = sampleCount < 2 ? 2 : sampleCount;
    const TScalar step = ( end - start ) / TScalar( samples );

    TScalar bestParameter = start;
    TPoint bestPoint = curve.Evaluate( start );
    TVector offset = TTraits::Sub( bestPoint, point );
    TScalar bestSquared = TTraits::Dot( offset, offset );
    if ( !( bestSquared == bestSquared ) || bestSquared > std::numeric_limits< TScalar >::max() )
        return false;

    // squared distances of the previous, current and next sample; samples are evaluated once
    TScalar previousSquared = std::numeric_limits< TScalar >::max();
    TScalar currentSquared = bestSquared;
    for ( UInt32 i=0; i<=samples; ++i )
    {
        TScalar nextSquared = std::numeric_limits< TScalar >::max();
        if ( i < samples )
        {
            const TScalar nextT = i + 1 == samples ? end : start + step * TScalar( i + 1 );
            const TVector nextOffset = TTraits::Sub( curve.Evaluate( nextT ), point );
            nextSquared = TTraits::Dot( nextOffset, nextOffset );
        }
        if ( currentSquared <= previousSquared && currentSquared <= nextSquared )
        {
            // refine on [ t_i-1, t_i+1 ] clamped to the range
            const TScalar centre = i == samples ? end : start + step * TScalar( i );
            TScalar lower = i == 0 ? start : centre - step;
            TScalar upper = i == samples ? end : ( i + 1 == samples ? end : centre + step );
            TScalar t = centre;

            const TVector lowOffset = TTraits::Sub( curve.Evaluate( lower ), point );
            const TVector highOffset = TTraits::Sub( curve.Evaluate( upper ), point );
            const TScalar fLow = TTraits::Dot( lowOffset, curve.GetDerivative( lower ) );
            const TScalar fHigh = TTraits::Dot( highOffset, curve.GetDerivative( upper ) );
            if ( !( fLow < TScalar( 0 ) ) )
                t = lower;                      // distance does not decrease at the lower end: minimum there
            else if ( !( fHigh > TScalar( 0 ) ) )
                t = upper;                      // distance still decreases at the upper end: minimum there
            else
            {
                const TScalar tolerance = std::numeric_limits< TScalar >::epsilon() * TScalar( 4 ) * ( upper - lower );
                bool havePrevious = false;
                TScalar previousT = t;
                TScalar previousF = TScalar( 0 );
                for ( UInt32 iteration=0; iteration<100; ++iteration )
                {
                    const TVector d = curve.GetDerivative( t );
                    const TVector o = TTraits::Sub( curve.Evaluate( t ), point );
                    const TScalar f = TTraits::Dot( o, d );
                    if ( f < TScalar( 0 ) ) lower = t; else upper = t;
                    // slope of f: the secant through the last two iterates once there are two (superlinear), the
                    // Gauss-Newton slope C' . C' otherwise or when the secant is not positive
                    TScalar slope = TTraits::Dot( d, d );
                    if ( havePrevious && t != previousT )
                    {
                        const TScalar secant = ( f - previousF ) / ( t - previousT );
                        if ( secant > TScalar( 0 ) )
                            slope = secant;
                    }
                    previousT = t;
                    previousF = f;
                    havePrevious = true;
                    TScalar next = slope > TScalar( 0 ) ? t - f / slope : TScalar( 0.5 ) * ( lower + upper );
                    if ( !( next > lower ) || !( next < upper ) )
                        next = TScalar( 0.5 ) * ( lower + upper );
                    const TScalar moved = next > t ? next - t : t - next;
                    t = next;
                    if ( moved <= tolerance || ( upper - lower ) <= tolerance )
                        break;
                }
            }
            const TPoint candidate = curve.Evaluate( t );
            offset = TTraits::Sub( candidate, point );
            const TScalar squared = TTraits::Dot( offset, offset );
            if ( squared < bestSquared )
            {
                bestSquared = squared;
                bestParameter = t;
                bestPoint = candidate;
            }
        }
        previousSquared = currentSquared;
        currentSquared = nextSquared;
    }
    parameter = bestParameter;
    closest = bestPoint;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CURVEQUERYFUNCTIONS_H ? */
