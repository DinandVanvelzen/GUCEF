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

#ifndef GUCEF_MATH_CTCATMULLROMSPLINE_H
#define GUCEF_MATH_CTCATMULLROMSPLINE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <limits>

#ifndef GUCEF_MATH_CTHERMITESPLINE_H
#include "gucefMATH_CTHermiteSpline.h"
#define GUCEF_MATH_CTHERMITESPLINE_H
#endif /* GUCEF_MATH_CTHERMITESPLINE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      ENUMERATIONS                                                       //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  How the parameter advances between two control points of a Catmull-Rom spline (the knot spacing is distance^alpha).
 *  Centripetal (alpha 0.5) has neither cusps nor self-intersections inside a segment (Yuksel, Schaefer and Keyser,
 *  "Parameterization and applications of Catmull-Rom curves", 2011).
 */
enum ECatmullRomParametrisation
{
    CATMULLROM_UNIFORM = 0,     /**< alpha 0: every segment spans one unit of the parameter */
    CATMULLROM_CENTRIPETAL,     /**< alpha 0.5: the segment spans the square root of its chord length */
    CATMULLROM_CHORDAL          /**< alpha 1: the segment spans its chord length */
};

typedef enum ECatmullRomParametrisation TCatmullRomParametrisation;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  An open Catmull-Rom spline through its control points: the curve passes through every point, is C1 at the joints
 *  and each segment is the cubic Hermite segment whose tangents come from the neighbouring points (non-uniform
 *  formulation, equal to the Barry-Goldman pyramid). The end segments use reflected virtual end points.
 *
 *  - Parameter: the knot value. Point i sits at knot GetKnot( i ); knot 0 is 0 and the spacing of a segment follows the
 *    parametrisation, so for CATMULLROM_UNIFORM the parameter of point i is i. GetDerivative is the derivative
 *    with respect to this parameter and is continuous across the joints.
 *  - A knot spacing below the larger of 1e-6 and 8 epsilon times the knot (coincident points) is raised to it, so the
 *    tangents stay finite and the knot always advances, also in Float32 at large coordinates.
 *  - TPoint: see CTCurvePointTraits (Float32 / Float64, CTVector2D / 3D / 4D, CTPoint2D / 3D). No point of the spline
 *    is ever added to another point; tangents are built from displacements.
 *  - With fewer than two points the spline evaluates to its only point (or the default point) and has a zero derivative.
 *  - Queries are realtime-safe; changing the points allocates (not realtime-safe) and recomputes the knots.
 *  - DataNode: a child node 'point' per control point and the attribute 'parametrisation' (0, 1 or 2).
 *  - Curve interface for CTArcLengthTable and the curve queries: TTraits, TVector, TScalar, Evaluate,
 *    GetDerivative, GetMinParameter, GetMaxParameter.
 */
template < typename TPoint >
class GUCEF_MATH_TEMPLATE_PUBLIC CTCatmullRomSpline
{
    public:

    typedef CTCurvePointTraits< TPoint > TTraits;
    typedef typename TTraits::TScalar TScalar;
    typedef typename TTraits::TVector TVector;

    explicit CTCatmullRomSpline( TCatmullRomParametrisation parametrisation = CATMULLROM_CENTRIPETAL );

    void SetParametrisation( TCatmullRomParametrisation parametrisation );
    GUCEF_MATH_FORCE_INLINE TCatmullRomParametrisation GetParametrisation( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_parametrisation; }

    void AddPoint( const TPoint& point );

    /** Replaces all points with 'count' points from the array */
    void SetPoints( const TPoint* points, UInt32 count );

    /** Replaces point 'index'; false (nothing changed) when the index is out of range */
    bool SetPoint( UInt32 index, const TPoint& point );

    /** Removes point 'index'; false when the index is out of range */
    bool RemovePoint( UInt32 index );

    void Clear( void );

    GUCEF_MATH_FORCE_INLINE UInt32 GetPointCount( void ) const GUCEF_MATH_REALTIME_CONTEXT { return (UInt32) m_points.size(); }

    /** Point 'index'; the default point when out of range */
    TPoint GetPoint( UInt32 index ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Knot value of point 'index'; 0 when out of range */
    TScalar GetKnot( UInt32 index ) const GUCEF_MATH_REALTIME_CONTEXT;

    GUCEF_MATH_FORCE_INLINE UInt32 GetSegmentCount( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_points.size() > 1 ? (UInt32) m_points.size() - 1 : 0; }

    GUCEF_MATH_FORCE_INLINE TScalar GetMinParameter( void ) const GUCEF_MATH_REALTIME_CONTEXT { return TScalar( 0 ); }
    TScalar GetMaxParameter( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The point at the knot parameter t; t is clamped to the parameter range */
    TPoint Evaluate( const TScalar& t ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** dP/dt at the knot parameter t */
    TVector GetDerivative( const TScalar& t ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Appends the knots of the interior points, where the segments join, to 'breakpoints' (see CTArcLengthTable::BuildPiecewise) */
    void GetBreakpoints( GUCEF::vector< TScalar >& breakpoints ) const;

    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const;
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings );

    private:

    /** The Hermite form of segment 'segment' ( < GetSegmentCount() ): end points, tangents scaled to the unit parameter and the knot span */
    void GetSegmentData( UInt32 segment, TPoint& p1, TVector& m1, TPoint& p2, TVector& m2, TScalar& span ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Segment containing t and the parameter within it; false without segments */
    bool Locate( const TScalar& t, UInt32& segment, TScalar& u, TScalar& span ) const GUCEF_MATH_REALTIME_CONTEXT;

    TScalar GetSpacing( const TPoint& a, const TPoint& b, const TScalar& baseKnot ) const GUCEF_MATH_REALTIME_CONTEXT;
    void RebuildKnots( void );

    GUCEF::vector< TPoint > m_points;
    GUCEF::vector< TScalar > m_knots;
    TCatmullRomParametrisation m_parametrisation;
};

/*-------------------------------------------------------------------------*/

template < typename TPoint >
CTCatmullRomSpline< TPoint >::CTCatmullRomSpline( TCatmullRomParametrisation parametrisation )
    : m_points()
    , m_knots()
    , m_parametrisation( parametrisation )
{
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE typename CTCatmullRomSpline< TPoint >::TScalar
CTCatmullRomSpline< TPoint >::GetSpacing( const TPoint& a, const TPoint& b, const TScalar& baseKnot ) const GUCEF_MATH_REALTIME_CONTEXT
{
    TScalar spacing = TScalar( 1 );
    if ( CATMULLROM_UNIFORM != m_parametrisation )
    {
        const TScalar distance = TTraits::Length( TTraits::Sub( b, a ) );
        spacing = CATMULLROM_CENTRIPETAL == m_parametrisation ? std::sqrt( distance ) : distance;
    }
    // the floor grows with the knot so that adding it always changes the knot, whatever the kind
    const TScalar magnitude = baseKnot < TScalar( 0 ) ? -baseKnot : baseKnot;
    const TScalar floorValue = TScalar( 8 ) * std::numeric_limits< TScalar >::epsilon() * magnitude;
    const TScalar minimum = floorValue > TScalar( 1e-6 ) ? floorValue : TScalar( 1e-6 );
    return spacing > minimum ? spacing : minimum;
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE void
CTCatmullRomSpline< TPoint >::RebuildKnots( void )
{
    m_knots.clear();
    for ( size_t i=0; i<m_points.size(); ++i )
        m_knots.push_back( 0 == i ? TScalar( 0 ) : m_knots[ i - 1 ] + GetSpacing( m_points[ i - 1 ], m_points[ i ], m_knots[ i - 1 ] ) );
}

template < typename TPoint >
void
CTCatmullRomSpline< TPoint >::SetParametrisation( TCatmullRomParametrisation parametrisation )
{
    m_parametrisation = parametrisation;
    RebuildKnots();
}

template < typename TPoint >
void
CTCatmullRomSpline< TPoint >::AddPoint( const TPoint& point )
{
    const TScalar knot = m_points.empty() ? TScalar( 0 ) : m_knots.back() + GetSpacing( m_points.back(), point, m_knots.back() );
    m_points.push_back( point );
    m_knots.push_back( knot );
}

template < typename TPoint >
void
CTCatmullRomSpline< TPoint >::SetPoints( const TPoint* points, UInt32 count )
{
    m_points.clear();
    for ( UInt32 i=0; i<count; ++i )
        m_points.push_back( points[ i ] );
    RebuildKnots();
}

template < typename TPoint >
bool
CTCatmullRomSpline< TPoint >::SetPoint( UInt32 index, const TPoint& point )
{
    if ( index >= m_points.size() )
        return false;
    m_points[ index ] = point;
    RebuildKnots();
    return true;
}

template < typename TPoint >
bool
CTCatmullRomSpline< TPoint >::RemovePoint( UInt32 index )
{
    if ( index >= m_points.size() )
        return false;
    m_points.erase( m_points.begin() + index );
    RebuildKnots();
    return true;
}

template < typename TPoint >
void
CTCatmullRomSpline< TPoint >::Clear( void )
{
    m_points.clear();
    m_knots.clear();
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE TPoint
CTCatmullRomSpline< TPoint >::GetPoint( UInt32 index ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return index < m_points.size() ? m_points[ index ] : TPoint();
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE typename CTCatmullRomSpline< TPoint >::TScalar
CTCatmullRomSpline< TPoint >::GetKnot( UInt32 index ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return index < m_knots.size() ? m_knots[ index ] : TScalar( 0 );
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE typename CTCatmullRomSpline< TPoint >::TScalar
CTCatmullRomSpline< TPoint >::GetMaxParameter( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return m_knots.empty() ? TScalar( 0 ) : m_knots.back();
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE void
CTCatmullRomSpline< TPoint >::GetSegmentData( UInt32 segment, TPoint& p1, TVector& m1, TPoint& p2, TVector& m2, TScalar& span ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const size_t count = m_points.size();
    p1 = m_points[ segment ];
    p2 = m_points[ segment + 1 ];
    const TScalar t1 = m_knots[ segment ];
    const TScalar t2 = m_knots[ segment + 1 ];

    // the neighbours: reflected virtual points at the ends, with mirrored knot spacing
    TPoint p0;
    TScalar t0;
    if ( segment > 0 )
    {
        p0 = m_points[ segment - 1 ];
        t0 = m_knots[ segment - 1 ];
    }
    else
    {
        p0 = TTraits::Offset( p1, TTraits::Negate( TTraits::Sub( p2, p1 ) ) );
        t0 = t1 - ( t2 - t1 );
    }
    TPoint p3;
    TScalar t3;
    if ( segment + 2 < count )
    {
        p3 = m_points[ segment + 2 ];
        t3 = m_knots[ segment + 2 ];
    }
    else
    {
        p3 = TTraits::Offset( p2, TTraits::Sub( p2, p1 ) );
        t3 = t2 + ( t2 - t1 );
    }

    span = t2 - t1;
    const TVector v10 = TTraits::Sub( p1, p0 );
    const TVector v21 = TTraits::Sub( p2, p1 );
    const TVector v32 = TTraits::Sub( p3, p2 );
    const TVector v20 = TTraits::Sub( p2, p0 );
    const TVector v31 = TTraits::Sub( p3, p1 );

    // dP/dt at p1 and p2 of the non-uniform Catmull-Rom segment, scaled by the span to the unit segment parameter
    const TVector a = TTraits::Add( TTraits::Scale( v10, TScalar( 1 ) / ( t1 - t0 ) ), TTraits::Scale( v21, TScalar( 1 ) / span ) );
    m1 = TTraits::Scale( TTraits::Add( a, TTraits::Negate( TTraits::Scale( v20, TScalar( 1 ) / ( t2 - t0 ) ) ) ), span );
    const TVector b = TTraits::Add( TTraits::Scale( v21, TScalar( 1 ) / span ), TTraits::Scale( v32, TScalar( 1 ) / ( t3 - t2 ) ) );
    m2 = TTraits::Scale( TTraits::Add( b, TTraits::Negate( TTraits::Scale( v31, TScalar( 1 ) / ( t3 - t1 ) ) ) ), span );
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE bool
CTCatmullRomSpline< TPoint >::Locate( const TScalar& t, UInt32& segment, TScalar& u, TScalar& span ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const UInt32 segments = GetSegmentCount();
    if ( 0 == segments )
        return false;
    TScalar clamped = t;
    if ( !( clamped > m_knots[ 0 ] ) )
        clamped = m_knots[ 0 ];
    if ( clamped > m_knots[ segments ] )
        clamped = m_knots[ segments ];

    // largest segment index with knot <= t
    UInt32 low = 0;
    UInt32 high = segments;
    while ( high - low > 1 )
    {
        const UInt32 mid = low + ( high - low ) / 2;
        if ( m_knots[ mid ] <= clamped )
            low = mid;
        else
            high = mid;
    }
    segment = low;
    span = m_knots[ low + 1 ] - m_knots[ low ];
    u = ( clamped - m_knots[ low ] ) / span;
    return true;
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE TPoint
CTCatmullRomSpline< TPoint >::Evaluate( const TScalar& t ) const GUCEF_MATH_REALTIME_CONTEXT
{
    UInt32 segment = 0;
    TScalar u = TScalar( 0 );
    TScalar span = TScalar( 1 );
    if ( !Locate( t, segment, u, span ) )
        return m_points.empty() ? TPoint() : m_points[ 0 ];
    TPoint p1, p2;
    TVector m1 = TTraits::Zero();
    TVector m2 = TTraits::Zero();
    GetSegmentData( segment, p1, m1, p2, m2, span );
    return HermiteEvaluate( p1, m1, p2, m2, u );
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE typename CTCatmullRomSpline< TPoint >::TVector
CTCatmullRomSpline< TPoint >::GetDerivative( const TScalar& t ) const GUCEF_MATH_REALTIME_CONTEXT
{
    UInt32 segment = 0;
    TScalar u = TScalar( 0 );
    TScalar span = TScalar( 1 );
    if ( !Locate( t, segment, u, span ) )
        return TTraits::Zero();
    TPoint p1, p2;
    TVector m1 = TTraits::Zero();
    TVector m2 = TTraits::Zero();
    GetSegmentData( segment, p1, m1, p2, m2, span );
    return TTraits::Scale( HermiteDerivative( p1, m1, p2, m2, u ), TScalar( 1 ) / span );
}

template < typename TPoint >
void
CTCatmullRomSpline< TPoint >::GetBreakpoints( GUCEF::vector< TScalar >& breakpoints ) const
{
    for ( size_t i=1; i+1<m_knots.size(); ++i )
        breakpoints.push_back( m_knots[ i ] );
}

template < typename TPoint >
bool
CTCatmullRomSpline< TPoint >::Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const
{
    if ( !node.SetAttribute( CORE::CString( "parametrisation" ), (UInt32) m_parametrisation ) )
        return false;
    return CurveSerializeList( node, "point", m_points, settings );
}

template < typename TPoint >
bool
CTCatmullRomSpline< TPoint >::Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings )
{
    const CORE::CDataNode::TKeyValuePair* attribute = node.GetAttribute( CORE::CString( "parametrisation" ) );
    if ( GUCEF_NULL == attribute )
        return false;
    const UInt32 parametrisation = attribute->second.AsUInt32();
    if ( parametrisation > (UInt32) CATMULLROM_CHORDAL )
        return false;
    GUCEF::vector< TPoint > points;
    if ( !CurveDeserializeList( node, "point", points, settings ) )
        return false;
    m_parametrisation = (TCatmullRomParametrisation) parametrisation;
    m_points.swap( points );
    RebuildKnots();
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUCEF_MATH_CTCATMULLROMSPLINE_EXTERN( kind ) GUCEF_MATH_EXTERN_TEMPLATE_CLASS( CTCatmullRomSpline< kind > )
GUCEF_MATH_CURVE_PREDEFINED_POINTS( GUCEF_MATH_CTCATMULLROMSPLINE_EXTERN )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTCATMULLROMSPLINE_H ? */
