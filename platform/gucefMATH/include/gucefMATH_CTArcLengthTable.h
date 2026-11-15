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

#ifndef GUCEF_MATH_CTARCLENGTHTABLE_H
#define GUCEF_MATH_CTARCLENGTHTABLE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <cmath>
#include <limits>

#ifndef GUCEF_MATH_MACROS_H
#include "gucefMATH_macros.h"
#define GUCEF_MATH_MACROS_H
#endif /* GUCEF_MATH_MACROS_H ? */

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
 *  The arc length of a parametric curve and the inverse question, which parameter lies at a given distance along it.
 *
 *  - Works with every curve of gucefMATH (CTBezierCurve2 / 3, CTHermiteSpline, CTCatmullRomSpline, CTBSpline) and any
 *    user type with the same members: TTraits, TScalar, TVector, GetDerivative( t ), GetMinParameter() and
 *    GetMaxParameter(). The curve is a template parameter of Build() and of the queries (the table does not keep a
 *    pointer, so it can not dangle; the same curve object, unchanged since Build(), must be passed to the queries).
 *  - Splines are made of pieces that join with a kink in the speed (the derivative is continuous, the speed |C'| is not
 *    smooth where it passes through a joint); an adaptive rule can miss a kink close to the edge of an interval.
 *    BuildPiecewise() therefore integrates every piece separately; it uses curve.GetBreakpoints() (provided by
 *    CTHermiteSpline, CTCatmullRomSpline and CTBSpline). Build() is for smooth single piece curves (Bezier).
 *  - Build() integrates the speed |C'( t )| with 5 point Gauss-Legendre quadrature on adaptively bisected parameter
 *    intervals (an interval is accepted when its two halves agree with the whole within 'tolerance', relatively) and
 *    stores the parameter and the accumulated length at every interval boundary.
 *  - GetParameterAtDistance() finds the interval by binary search and refines inside it with Newton iterations on the
 *    interval integral (bisection safeguarded, a few iterations).
 *  - T: Float32 or Float64. The quadrature code is a template compiled with the flags of the caller; the table itself
 *    (data, GetLength, boundary access) is an exported instantiation.
 *  - Build() allocates (not realtime-safe); every query is realtime-safe. Concurrent const use is safe.
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTArcLengthTable
{
    public:

    typedef T TScalar;

    CTArcLengthTable( void );

    /**
     *  Measures the whole parameter range of 'curve'. tolerance <= 0 selects the default of the kind (0.01 sqrt( epsilon ),
     *  about 1.5e-10 for Float64); minSegments (at least 1) is the number of equal parameter intervals the adaptive
     *  bisection starts from, a curve built from several pieces (splines) should pass at least its piece count.
     *  Returns false and leaves an empty table when the parameter range is empty or the speed is not finite.
     */
    template < typename TCurve >
    bool Build( const TCurve& curve, const T& tolerance = T( 0 ), UInt32 minSegments = 8 );

    /**
     *  As Build() for a curve made of pieces: the parameter range is divided at the breakpoints that
     *  curve.GetBreakpoints( GUCEF::vector< T >& ) reports (joints of the pieces; values outside the range and
     *  duplicates are ignored), and each piece is integrated adaptively on its own.
     */
    template < typename TCurve >
    bool BuildPiecewise( const TCurve& curve, const T& tolerance = T( 0 ) );

    void Clear( void );

    GUCEF_MATH_FORCE_INLINE bool IsValid( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_parameters.size() > 1; }

    /** Total arc length over the parameter range; 0 for an empty table */
    T GetLength( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Number of intervals the adaptive bisection produced */
    GUCEF_MATH_FORCE_INLINE UInt32 GetIntervalCount( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_parameters.size() > 1 ? (UInt32) m_parameters.size() - 1 : 0; }

    /** Parameter at interval boundary 'index' ( 0 .. GetIntervalCount() ); 0 when out of range */
    T GetBoundaryParameter( UInt32 index ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Arc length from the start up to interval boundary 'index'; 0 when out of range */
    T GetBoundaryDistance( UInt32 index ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The parameter at which the arc length from the start equals 'distance' (clamped to [ 0, GetLength() ]); the first parameter for an empty table */
    template < typename TCurve >
    T GetParameterAtDistance( const TCurve& curve, const T& distance ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The arc length from the start of the range up to parameter t (clamped to the range) */
    template < typename TCurve >
    T GetDistanceAtParameter( const TCurve& curve, const T& t ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The 5 point Gauss-Legendre integral of the speed of 'curve' over [ a, b ] */
    template < typename TCurve >
    static T IntegrateSpeed( const TCurve& curve, const T& a, const T& b ) GUCEF_MATH_REALTIME_CONTEXT;

    private:

    /** Integrates every piece [ edges[ i ], edges[ i + 1 ] ] adaptively; edges are increasing and span the range */
    template < typename TCurve >
    bool BuildFromEdges( const TCurve& curve, const GUCEF::vector< T >& edges, const T& tolerance );

    /** Index i of the interval with distance i <= d < distance i + 1 */
    UInt32 FindInterval( const T& distance ) const GUCEF_MATH_REALTIME_CONTEXT;

    GUCEF::vector< T > m_parameters;    /**< interval boundaries, first is the start of the range */
    GUCEF::vector< T > m_distances;     /**< accumulated arc length at every boundary */
};

/*-------------------------------------------------------------------------*/

template < typename T >
template < typename TCurve >
GUCEF_MATH_PRECISE_INLINE T
CTArcLengthTable< T >::IntegrateSpeed( const TCurve& curve, const T& a, const T& b ) GUCEF_MATH_REALTIME_CONTEXT
{
    // nodes and weights of the 5 point rule on [ -1, 1 ] (constant data)
    static const T nodes[ 5 ] = { T( 0 ), T( 0.5384693101056831 ), T( -0.5384693101056831 ), T( 0.9061798459386640 ), T( -0.9061798459386640 ) };
    static const T weights[ 5 ] = { T( 0.5688888888888889 ), T( 0.4786286704993665 ), T( 0.4786286704993665 ), T( 0.2369268850561891 ), T( 0.2369268850561891 ) };
    const T middle = T( 0.5 ) * ( a + b );
    const T half = T( 0.5 ) * ( b - a );
    T sum = T( 0 );
    for ( UInt32 i=0; i<5; ++i )
        sum += weights[ i ] * TCurve::TTraits::Length( curve.GetDerivative( middle + half * nodes[ i ] ) );
    return half * sum;
}

template < typename T >
CTArcLengthTable< T >::CTArcLengthTable( void )
    : m_parameters()
    , m_distances()
{
}

template < typename T >
void
CTArcLengthTable< T >::Clear( void )
{
    m_parameters.clear();
    m_distances.clear();
}

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTArcLengthTable< T >::GetLength( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return m_distances.empty() ? T( 0 ) : m_distances.back();
}

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTArcLengthTable< T >::GetBoundaryParameter( UInt32 index ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return index < m_parameters.size() ? m_parameters[ index ] : T( 0 );
}

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTArcLengthTable< T >::GetBoundaryDistance( UInt32 index ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return index < m_distances.size() ? m_distances[ index ] : T( 0 );
}

template < typename T >
GUCEF_MATH_PRECISE_INLINE UInt32
CTArcLengthTable< T >::FindInterval( const T& distance ) const GUCEF_MATH_REALTIME_CONTEXT
{
    UInt32 low = 0;
    UInt32 high = (UInt32) m_distances.size() - 1;
    while ( high - low > 1 )
    {
        const UInt32 mid = low + ( high - low ) / 2;
        if ( m_distances[ mid ] <= distance )
            low = mid;
        else
            high = mid;
    }
    return low;
}

template < typename T >
template < typename TCurve >
bool
CTArcLengthTable< T >::Build( const TCurve& curve, const T& tolerance, UInt32 minSegments )
{
    Clear();
    const T start = curve.GetMinParameter();
    const T end = curve.GetMaxParameter();
    if ( !( end > start ) )
        return false;
    const UInt32 pieces = minSegments > 0 ? minSegments : 1;
    GUCEF::vector< T > edges;
    for ( UInt32 i=0; i<=pieces; ++i )
        edges.push_back( i == 0 ? start : ( i == pieces ? end : start + ( end - start ) * ( T( i ) / T( pieces ) ) ) );
    return BuildFromEdges( curve, edges, tolerance );
}

template < typename T >
template < typename TCurve >
bool
CTArcLengthTable< T >::BuildPiecewise( const TCurve& curve, const T& tolerance )
{
    Clear();
    const T start = curve.GetMinParameter();
    const T end = curve.GetMaxParameter();
    if ( !( end > start ) )
        return false;
    GUCEF::vector< T > breakpoints;
    curve.GetBreakpoints( breakpoints );
    GUCEF::vector< T > edges;
    edges.push_back( start );
    for ( size_t i=0; i<breakpoints.size(); ++i )
        if ( breakpoints[ i ] > edges.back() && breakpoints[ i ] < end )
            edges.push_back( breakpoints[ i ] );
    edges.push_back( end );
    return BuildFromEdges( curve, edges, tolerance );
}

template < typename T >
template < typename TCurve >
bool
CTArcLengthTable< T >::BuildFromEdges( const TCurve& curve, const GUCEF::vector< T >& edges, const T& tolerance )
{
    struct SFrame { T a; T b; T whole; UInt32 depth; };
    enum { MaxDepth = 30, MaxIntervals = 65536 };

    Clear();
    const T tol = tolerance > T( 0 ) ? tolerance : T( 0.01 ) * std::sqrt( std::numeric_limits< T >::epsilon() );

    m_parameters.push_back( edges[ 0 ] );
    m_distances.push_back( T( 0 ) );
    T total = T( 0 );
    for ( size_t piece=0; piece+1<edges.size(); ++piece )
    {
        const T pieceStart = edges[ piece ];
        const T pieceEnd = edges[ piece + 1 ];
        SFrame stack[ MaxDepth + 4 ];
        UInt32 top = 0;
        stack[ 0 ].a = pieceStart;
        stack[ 0 ].b = pieceEnd;
        stack[ 0 ].whole = IntegrateSpeed( curve, pieceStart, pieceEnd );
        stack[ 0 ].depth = 0;
        top = 1;
        while ( top > 0 )
        {
            const SFrame frame = stack[ --top ];
            const T middle = T( 0.5 ) * ( frame.a + frame.b );
            const T left = IntegrateSpeed( curve, frame.a, middle );
            const T right = IntegrateSpeed( curve, middle, frame.b );
            const T sum = left + right;
            if ( !( sum == sum ) || sum > std::numeric_limits< T >::max() )
            {
                Clear();
                return false;
            }
            const T difference = frame.whole > sum ? frame.whole - sum : sum - frame.whole;
            if ( frame.depth >= (UInt32) MaxDepth || m_parameters.size() >= (size_t) MaxIntervals || difference <= tol * sum )
            {
                total += sum;
                m_parameters.push_back( frame.b );
                m_distances.push_back( total );
            }
            else
            {
                // right first so that the left half is processed first
                stack[ top ].a = middle; stack[ top ].b = frame.b; stack[ top ].whole = right; stack[ top ].depth = frame.depth + 1; ++top;
                stack[ top ].a = frame.a; stack[ top ].b = middle; stack[ top ].whole = left; stack[ top ].depth = frame.depth + 1; ++top;
            }
        }
    }
    return true;
}

template < typename T >
template < typename TCurve >
GUCEF_MATH_PRECISE_INLINE T
CTArcLengthTable< T >::GetParameterAtDistance( const TCurve& curve, const T& distance ) const GUCEF_MATH_REALTIME_CONTEXT
{
    if ( !IsValid() )
        return curve.GetMinParameter();
    const T length = GetLength();
    if ( !( distance > T( 0 ) ) )
        return m_parameters[ 0 ];
    if ( !( length > T( 0 ) ) )
        return m_parameters[ 0 ];                   // a curve without length is one point: the start
    if ( distance >= length )
        return m_parameters.back();

    const UInt32 i = FindInterval( distance );
    const T a = m_parameters[ i ];
    const T b = m_parameters[ i + 1 ];
    const T segment = m_distances[ i + 1 ] - m_distances[ i ];
    const T wanted = distance - m_distances[ i ];
    if ( !( segment > T( 0 ) ) )
        return a;

    T lower = a;
    T upper = b;
    T t = a + ( b - a ) * ( wanted / segment );
    const T stop = std::numeric_limits< T >::epsilon() * T( 4 ) * ( b - a );
    for ( UInt32 iteration=0; iteration<40; ++iteration )
    {
        const T f = IntegrateSpeed( curve, a, t ) - wanted;
        if ( f < T( 0 ) ) lower = t; else upper = t;
        const T speed = TCurve::TTraits::Length( curve.GetDerivative( t ) );
        T next = speed > T( 0 ) ? t - f / speed : T( 0.5 ) * ( lower + upper );
        if ( !( next > lower ) || !( next < upper ) )
            next = T( 0.5 ) * ( lower + upper );
        const T step = next > t ? next - t : t - next;
        t = next;
        if ( step <= stop )
            break;
    }
    return t;
}

template < typename T >
template < typename TCurve >
GUCEF_MATH_PRECISE_INLINE T
CTArcLengthTable< T >::GetDistanceAtParameter( const TCurve& curve, const T& t ) const GUCEF_MATH_REALTIME_CONTEXT
{
    if ( !IsValid() )
        return T( 0 );
    if ( !( t > m_parameters[ 0 ] ) )
        return T( 0 );
    if ( t >= m_parameters.back() )
        return GetLength();
    UInt32 low = 0;
    UInt32 high = (UInt32) m_parameters.size() - 1;
    while ( high - low > 1 )
    {
        const UInt32 mid = low + ( high - low ) / 2;
        if ( m_parameters[ mid ] <= t )
            low = mid;
        else
            high = mid;
    }
    return m_distances[ low ] + IntegrateSpeed( curve, m_parameters[ low ], t );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_MATH_EXTERN_TEMPLATE_CLASS( CTArcLengthTable< Float32 > )
GUCEF_MATH_EXTERN_TEMPLATE_CLASS( CTArcLengthTable< Float64 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTARCLENGTHTABLE_H ? */
