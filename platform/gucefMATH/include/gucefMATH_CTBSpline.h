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

#ifndef GUCEF_MATH_CTBSPLINE_H
#define GUCEF_MATH_CTBSPLINE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A B-spline of any degree from 1 to GUCEF_MATH_BSPLINE_MAX_DEGREE with an explicit knot vector, evaluated with de
 *  Boor's algorithm. n control points of degree d need n + d + 1 non-decreasing knots and n >= d + 1; the curve is
 *  defined on [ knot d, knot n ] (GetMinParameter(), GetMaxParameter()) and is C( d - 1 ) where knots are simple.
 *
 *  - Initialize() builds a clamped uniform knot vector (the curve starts and ends at the first and last control
 *    point, parameter range [ 0, n - d ] with one unit per span); InitializeUniform() builds the unclamped uniform
 *    vector 0, 1, 2, ... (range [ d, n ]; for d = 3 the segments equal the uniform cubic B-spline matrix form);
 *    SetKnots() takes any valid vector.
 *  - Evaluation: partition of unity holds (GetBasisFunctions), the curve lies in the convex hull of the control
 *    points, and a query allocates nothing (working storage of degree + 1 vectors on the stack).
 *  - TPoint: see CTCurvePointTraits (Float32 / Float64, CTVector2D / 3D / 4D, CTPoint2D / 3D).
 *  - Parameters outside the range are clamped. A spline that was not successfully initialised evaluates to the default
 *    point with a zero derivative.
 *  - Queries are realtime-safe; Initialize, InitializeUniform and SetKnots allocate (not realtime-safe).
 *  - DataNode: attribute 'degree', child nodes 'point' (control points) and 'knot' (attribute 'value').
 *  - Curve interface for CTArcLengthTable and the curve queries: TTraits, TVector, TScalar, Evaluate,
 *    GetDerivative, GetMinParameter, GetMaxParameter.
 */
template < typename TPoint >
class GUCEF_MATH_TEMPLATE_PUBLIC CTBSpline
{
    public:

    typedef CTCurvePointTraits< TPoint > TTraits;
    typedef typename TTraits::TScalar TScalar;
    typedef typename TTraits::TVector TVector;

    enum { MaxDegree = 15 };

    CTBSpline( void );

    /** Clamped uniform knots; false (nothing changed) when degree is outside 1 .. MaxDegree or count < degree + 1 */
    bool Initialize( UInt32 degree, const TPoint* points, UInt32 count );

    /** Unclamped uniform knots 0, 1, 2, ...; same conditions as Initialize() */
    bool InitializeUniform( UInt32 degree, const TPoint* points, UInt32 count );

    /** Explicit knots ( count + degree + 1 values, non-decreasing, first span non-empty ); false (nothing changed) otherwise */
    bool SetKnots( UInt32 degree, const TPoint* points, UInt32 count, const TScalar* knots, UInt32 knotCount );

    GUCEF_MATH_FORCE_INLINE bool IsValid( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_degree > 0; }
    GUCEF_MATH_FORCE_INLINE UInt32 GetDegree( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_degree; }
    GUCEF_MATH_FORCE_INLINE UInt32 GetControlPointCount( void ) const GUCEF_MATH_REALTIME_CONTEXT { return (UInt32) m_points.size(); }
    GUCEF_MATH_FORCE_INLINE UInt32 GetKnotCount( void ) const GUCEF_MATH_REALTIME_CONTEXT { return (UInt32) m_knots.size(); }

    /** Control point 'index'; the default point when out of range */
    TPoint GetControlPoint( UInt32 index ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Knot 'index'; 0 when out of range */
    TScalar GetKnot( UInt32 index ) const GUCEF_MATH_REALTIME_CONTEXT;

    TScalar GetMinParameter( void ) const GUCEF_MATH_REALTIME_CONTEXT;
    TScalar GetMaxParameter( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The point at parameter t (clamped to the range) */
    TPoint Evaluate( const TScalar& t ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** dP/dt at parameter t (clamped to the range); zero for degree 1 spans' constant slope is returned as is */
    TVector GetDerivative( const TScalar& t ) const GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The degree + 1 non-zero basis functions at t: they belong to the control points firstControlPoint ..
     *  firstControlPoint + degree, are non-negative and sum to one. 'basis' has room for degree + 1 values.
     *  False for an invalid spline.
     */
    bool GetBasisFunctions( const TScalar& t, UInt32& firstControlPoint, TScalar* basis ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Appends the distinct knot values strictly inside the parameter range, where the pieces join, to 'breakpoints' (see CTArcLengthTable::BuildPiecewise) */
    void GetBreakpoints( GUCEF::vector< TScalar >& breakpoints ) const;

    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const;
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings );

    private:

    /** The knot span index k with knot k <= t < knot k + 1 inside [ degree, n - 1 ]; the last non-empty span for t at the end */
    UInt32 FindSpan( const TScalar& t ) const GUCEF_MATH_REALTIME_CONTEXT;

    static bool CheckKnots( UInt32 degree, UInt32 count, const TScalar* knots, UInt32 knotCount );

    GUCEF::vector< TPoint > m_points;
    GUCEF::vector< TScalar > m_knots;
    UInt32 m_degree;
};

/*-------------------------------------------------------------------------*/

template < typename TPoint >
CTBSpline< TPoint >::CTBSpline( void )
    : m_points()
    , m_knots()
    , m_degree( 0 )
{
}

template < typename TPoint >
bool
CTBSpline< TPoint >::CheckKnots( UInt32 degree, UInt32 count, const TScalar* knots, UInt32 knotCount )
{
    if ( degree < 1 || degree > (UInt32) MaxDegree || count < degree + 1 || knotCount != count + degree + 1 )
        return false;
    for ( UInt32 i=1; i<knotCount; ++i )
        if ( !( knots[ i - 1 ] <= knots[ i ] ) )
            return false;
    return knots[ degree ] < knots[ count ];
}

template < typename TPoint >
bool
CTBSpline< TPoint >::SetKnots( UInt32 degree, const TPoint* points, UInt32 count, const TScalar* knots, UInt32 knotCount )
{
    if ( !CheckKnots( degree, count, knots, knotCount ) )
        return false;
    m_points.assign( points, points + count );
    m_knots.assign( knots, knots + knotCount );
    m_degree = degree;
    return true;
}

template < typename TPoint >
bool
CTBSpline< TPoint >::Initialize( UInt32 degree, const TPoint* points, UInt32 count )
{
    if ( degree < 1 || degree > (UInt32) MaxDegree || count < degree + 1 )
        return false;
    GUCEF::vector< TScalar > knots;
    for ( UInt32 i=0; i<count+degree+1; ++i )
    {
        if ( i <= degree )
            knots.push_back( TScalar( 0 ) );
        else if ( i >= count )
            knots.push_back( TScalar( count - degree ) );
        else
            knots.push_back( TScalar( i - degree ) );
    }
    return SetKnots( degree, points, count, &knots[ 0 ], (UInt32) knots.size() );
}

template < typename TPoint >
bool
CTBSpline< TPoint >::InitializeUniform( UInt32 degree, const TPoint* points, UInt32 count )
{
    if ( degree < 1 || degree > (UInt32) MaxDegree || count < degree + 1 )
        return false;
    GUCEF::vector< TScalar > knots;
    for ( UInt32 i=0; i<count+degree+1; ++i )
        knots.push_back( TScalar( i ) );
    return SetKnots( degree, points, count, &knots[ 0 ], (UInt32) knots.size() );
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE TPoint
CTBSpline< TPoint >::GetControlPoint( UInt32 index ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return index < m_points.size() ? m_points[ index ] : TPoint();
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE typename CTBSpline< TPoint >::TScalar
CTBSpline< TPoint >::GetKnot( UInt32 index ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return index < m_knots.size() ? m_knots[ index ] : TScalar( 0 );
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE typename CTBSpline< TPoint >::TScalar
CTBSpline< TPoint >::GetMinParameter( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return m_degree > 0 ? m_knots[ m_degree ] : TScalar( 0 );
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE typename CTBSpline< TPoint >::TScalar
CTBSpline< TPoint >::GetMaxParameter( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return m_degree > 0 ? m_knots[ m_points.size() ] : TScalar( 0 );
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE UInt32
CTBSpline< TPoint >::FindSpan( const TScalar& t ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const UInt32 n = (UInt32) m_points.size();
    UInt32 low = m_degree;
    UInt32 high = n;            // exclusive search bound: spans are [ degree, n - 1 ]
    // the largest k in [ degree, n - 1 ] with knot k <= t
    while ( high - low > 1 )
    {
        const UInt32 mid = low + ( high - low ) / 2;
        if ( m_knots[ mid ] <= t )
            low = mid;
        else
            high = mid;
    }
    // t at the end of the range: use the last span that has a non-zero length
    while ( low > m_degree && !( m_knots[ low ] < m_knots[ low + 1 ] ) )
        --low;
    return low;
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE TPoint
CTBSpline< TPoint >::Evaluate( const TScalar& t ) const GUCEF_MATH_REALTIME_CONTEXT
{
    if ( 0 == m_degree )
        return TPoint();
    TScalar x = t;
    if ( !( x > GetMinParameter() ) ) x = GetMinParameter();
    if ( x > GetMaxParameter() ) x = GetMaxParameter();

    const UInt32 d = m_degree;
    const UInt32 k = FindSpan( x );
    const TPoint& base = m_points[ k - d ];
    TVector work[ MaxDegree + 1 ];
    for ( UInt32 j=0; j<=d; ++j )
        work[ j ] = TTraits::Sub( m_points[ j + k - d ], base );
    for ( UInt32 r=1; r<=d; ++r )
    {
        for ( UInt32 j=d; j>=r; --j )
        {
            const TScalar left = m_knots[ j + k - d ];
            const TScalar denominator = m_knots[ j + 1 + k - r ] - left;
            const TScalar alpha = denominator > TScalar( 0 ) ? ( x - left ) / denominator : TScalar( 0 );
            work[ j ] = TTraits::Add( TTraits::Scale( work[ j - 1 ], TScalar( 1 ) - alpha ), TTraits::Scale( work[ j ], alpha ) );
        }
    }
    return TTraits::Offset( base, work[ d ] );
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE typename CTBSpline< TPoint >::TVector
CTBSpline< TPoint >::GetDerivative( const TScalar& t ) const GUCEF_MATH_REALTIME_CONTEXT
{
    if ( 0 == m_degree )
        return TTraits::Zero();
    TScalar x = t;
    if ( !( x > GetMinParameter() ) ) x = GetMinParameter();
    if ( x > GetMaxParameter() ) x = GetMaxParameter();

    // the derivative is the spline of degree d - 1 with the knots 1 .. n + d - 1 and the control vectors
    // Q_i = d ( P_i+1 - P_i ) / ( knot i+d+1 - knot i+1 ), evaluated with de Boor on the span k - 1
    const UInt32 d = m_degree;
    const UInt32 k = FindSpan( x );
    const UInt32 e = d - 1;
    TVector work[ MaxDegree + 1 ];
    for ( UInt32 j=0; j<=e; ++j )
    {
        const UInt32 i = k - d + j;
        const TScalar denominator = m_knots[ i + d + 1 ] - m_knots[ i + 1 ];
        const TScalar factor = denominator > TScalar( 0 ) ? TScalar( d ) / denominator : TScalar( 0 );
        work[ j ] = TTraits::Scale( TTraits::Sub( m_points[ i + 1 ], m_points[ i ] ), factor );
    }
    for ( UInt32 r=1; r<=e; ++r )
    {
        for ( UInt32 j=e; j>=r; --j )
        {
            // knots of the derivative spline are m_knots[ 1.. ], span index k - 1: knot index shifted by one
            const TScalar left = m_knots[ j + k - e ];
            const TScalar denominator = m_knots[ j + 1 + k - r ] - left;
            const TScalar alpha = denominator > TScalar( 0 ) ? ( x - left ) / denominator : TScalar( 0 );
            work[ j ] = TTraits::Add( TTraits::Scale( work[ j - 1 ], TScalar( 1 ) - alpha ), TTraits::Scale( work[ j ], alpha ) );
        }
    }
    return work[ e ];
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE bool
CTBSpline< TPoint >::GetBasisFunctions( const TScalar& t, UInt32& firstControlPoint, TScalar* basis ) const GUCEF_MATH_REALTIME_CONTEXT
{
    if ( 0 == m_degree )
        return false;
    TScalar x = t;
    if ( !( x > GetMinParameter() ) ) x = GetMinParameter();
    if ( x > GetMaxParameter() ) x = GetMaxParameter();

    const UInt32 d = m_degree;
    const UInt32 k = FindSpan( x );
    TScalar left[ MaxDegree + 1 ];
    TScalar right[ MaxDegree + 1 ];
    basis[ 0 ] = TScalar( 1 );
    for ( UInt32 j=1; j<=d; ++j )
    {
        left[ j ] = x - m_knots[ k + 1 - j ];
        right[ j ] = m_knots[ k + j ] - x;
        TScalar saved = TScalar( 0 );
        for ( UInt32 r=0; r<j; ++r )
        {
            const TScalar denominator = right[ r + 1 ] + left[ j - r ];
            const TScalar temp = denominator > TScalar( 0 ) ? basis[ r ] / denominator : TScalar( 0 );
            basis[ r ] = saved + right[ r + 1 ] * temp;
            saved = left[ j - r ] * temp;
        }
        basis[ j ] = saved;
    }
    firstControlPoint = k - d;
    return true;
}

template < typename TPoint >
void
CTBSpline< TPoint >::GetBreakpoints( GUCEF::vector< TScalar >& breakpoints ) const
{
    if ( 0 == m_degree )
        return;
    const TScalar low = GetMinParameter();
    const TScalar high = GetMaxParameter();
    TScalar last = low;
    for ( UInt32 i=m_degree; i<=m_points.size(); ++i )
    {
        if ( m_knots[ i ] > last && m_knots[ i ] < high )
        {
            breakpoints.push_back( m_knots[ i ] );
            last = m_knots[ i ];
        }
    }
}

template < typename TPoint >
bool
CTBSpline< TPoint >::Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const
{
    if ( 0 == m_degree || !node.SetAttribute( CORE::CString( "degree" ), m_degree ) )
        return false;
    if ( !CurveSerializeList( node, "point", m_points, settings ) )
        return false;
    return CurveSerializeList( node, "knot", m_knots, settings );
}

template < typename TPoint >
bool
CTBSpline< TPoint >::Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings )
{
    const CORE::CDataNode::TKeyValuePair* attribute = node.GetAttribute( CORE::CString( "degree" ) );
    if ( GUCEF_NULL == attribute )
        return false;
    const UInt32 degree = attribute->second.AsUInt32();
    GUCEF::vector< TPoint > points;
    GUCEF::vector< TScalar > knots;
    if ( !CurveDeserializeList( node, "point", points, settings ) || !CurveDeserializeList( node, "knot", knots, settings ) )
        return false;
    if ( points.empty() || knots.empty() )
        return false;
    return SetKnots( degree, &points[ 0 ], (UInt32) points.size(), &knots[ 0 ], (UInt32) knots.size() );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUCEF_MATH_CTBSPLINE_EXTERN( kind ) GUCEF_MATH_EXTERN_TEMPLATE_CLASS( CTBSpline< kind > )
GUCEF_MATH_CURVE_PREDEFINED_POINTS( GUCEF_MATH_CTBSPLINE_EXTERN )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTBSPLINE_H ? */
