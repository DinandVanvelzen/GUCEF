/*
 *  gucefGEOMETRY: GUCEF module providing geometry: shapes, spaces and spatial queries
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

#include <math.h>
#include <algorithm>
#include <limits>

#ifndef GUCEF_MATH_DETERMINISTICTRIGONOMETRY_H
#include "gucefMATH_DeterministicTrigonometry.h"
#define GUCEF_MATH_DETERMINISTICTRIGONOMETRY_H
#endif /* GUCEF_MATH_DETERMINISTICTRIGONOMETRY_H ? */

#ifndef GUCEF_GEOMETRY_CTPOLYGONOFFSETTER_H
#include "gucefGEOMETRY_CTPolygonOffsetter.h"
#define GUCEF_GEOMETRY_CTPOLYGONOFFSETTER_H
#endif /* GUCEF_GEOMETRY_CTPOLYGONOFFSETTER_H ? */

#ifndef GUCEF_GEOMETRY_CTPREDICATES_H
#include "gucefGEOMETRY_CTPredicates.h"
#define GUCEF_GEOMETRY_CTPREDICATES_H
#endif /* GUCEF_GEOMETRY_CTPREDICATES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const Float64 OffsetGridLimit = 4503599627370496.0;     /* 2^52 */

/** The grid conversions of a kind (as CTPolygonClipper): 0 floats, 1 integers, 2 fixed point */
template < typename T, int Strategy = MATH::CTNumericTraits< T >::IsFloatingPoint ? 0 : ( MATH::CTNumericTraits< T >::IsFixedPoint ? 2 : 1 ) >
struct CTOffsetGrid;

template < typename T >
struct CTOffsetGrid< T, 0 >
{
    static bool IsFinite( const T& v ) { return MATH::CTNumericTraits< T >::IsFinite( v ); }
    static Float64 ToFloat64( const T& v ) { return (Float64) v; }
    static Float64 FixedScale( void ) { return 0.0; }
    static T FromGrid( Int64 g, Float64 scale ) { return (T) ( (Float64) g / scale ); }
    /** True when the grid value maps to a finite value of the kind */
    static bool InRange( Int64 g, Float64 scale ) { return fabs( (Float64) g / scale ) <= (Float64) std::numeric_limits< T >::max(); }
    /** The coordinate in grid units (exact for a power of two scale) */
    static Float64 ToGrid( const T& v, Float64 scale ) { return (Float64) v * scale; }
};

template < typename T >
struct CTOffsetGrid< T, 1 >
{
    static bool IsFinite( const T& ) { return true; }
    static Float64 ToFloat64( const T& v ) { return (Float64) v; }
    static Float64 FixedScale( void ) { return 1.0; }
    static T FromGrid( Int64 g, Float64 ) { return (T) g; }
    static bool InRange( Int64 g, Float64 ) { return g >= (Int64) std::numeric_limits< T >::min() && g <= (Int64) std::numeric_limits< T >::max(); }
    static Float64 ToGrid( const T& v, Float64 ) { return (Float64) v; }
};

template < typename T >
struct CTOffsetGrid< T, 2 >
{
    static bool IsFinite( const T& ) { return true; }
    static Float64 ToFloat64( const T& v ) { return MATH::CTNumericTraits< T >::ToFloat64( v ); }
    static Float64 FixedScale( void ) { return ldexp( 1.0, (int) T::FRACTION_BITS ); }
    static T FromGrid( Int64 g, Float64 ) { return T::FromRaw( (typename T::TStorageType) g ); }
    static bool InRange( Int64 g, Float64 ) { return g >= (Int64) std::numeric_limits< typename T::TStorageType >::min() && g <= (Int64) std::numeric_limits< typename T::TStorageType >::max(); }
    static Float64 ToGrid( const T& v, Float64 ) { return (Float64) v.GetRaw(); }
};

/*-------------------------------------------------------------------------*/

/**
 *  Builds offset rings in grid units (Float64): the join and cap geometry of CTPolygonOffsetter.
 *
 *  Round arcs use no C library function, so the points are bit identical on every platform with IEEE 754 binary64
 *  arithmetic (section 5.5 of the design): a full circle has 'segments' steps, the smallest count in
 *  [ 4, 100000 ] whose chord stays within the arc tolerance ( cos( pi / segments ) >= 1 - tolerance / |d| ), found by a
 *  bisection over MATH::SinCosOfTurnFraction( 1, 2 * segments ); the rotation by one step comes from
 *  MATH::SinCosOfTurnFraction( 1, segments ) and is applied as the recurrence x' = x c - y s, y' = x s + y c, every
 *  product in its own statement. A join arc starts at the first edge normal and stops before the point where the
 *  rotated normal passes the second edge normal (a sign test of the cross product: no angle is computed); the end
 *  points of an arc are added by the caller from the known normals, so no drift reaches the end.
 */
class COffsetBuilder
{
    public:

    /** 'arcRatio' is 1 - tolerance / |d| (0 when d is 0) */
    COffsetBuilder( GUCEF::vector< Float64 >& out, TPolygonJoinType join, Float64 miterLimit, Float64 arcRatio )
        : m_out( out ), m_join( join ), m_miterLimit( miterLimit ), m_segments( SegmentsPerTurn( arcRatio ) ), m_stepSine( 0.0 ), m_stepCosine( 1.0 )
    {
        MATH::SinCosOfTurnFraction( (Int64) 1, (Int64) m_segments, m_stepSine, m_stepCosine );
    }

    /** The smallest number of segments per turn, in [ 4, 100000 ], with cos( pi / segments ) >= ratio */
    static UInt32 SegmentsPerTurn( Float64 ratio )
    {
        UInt32 low = 4;
        UInt32 high = 100000;
        while ( low < high )
        {
            const UInt32 mid = low + ( high - low ) / 2;
            Float64 sine = 0.0;
            Float64 cosine = 1.0;
            MATH::SinCosOfTurnFraction( (Int64) 1, (Int64) 2 * (Int64) mid, sine, cosine );
            if ( ratio <= cosine )
                high = mid;
            else
                low = mid + 1;
        }
        return low;
    }

    UInt32 GetSegmentsPerTurn( void ) const { return m_segments; }

    void Add( Float64 x, Float64 y )
    {
        m_out.push_back( x );
        m_out.push_back( y );
    }

    /** The step rotation by the sine 's' (negated for a clockwise sweep): the canonical operation order of the recurrence */
    void Rotate( Float64& x, Float64& y, Float64 s ) const
    {
        const Float64 xc = x * m_stepCosine;
        const Float64 ys = y * s;
        const Float64 xs = x * s;
        const Float64 yc = y * m_stepCosine;
        x = xc - ys;
        y = xs + yc;
    }

    /**
     *  The points of the arc around ( px, py ), radius 'd', from the normal ( nx, ny ) turning towards 'steps' steps (the
     *  first point and the point after the last step excluded): a full turn is 'segments' steps, a half turn
     *  ( segments + 1 ) / 2. 'counterClockwise' selects the sense of the turn.
     */
    void ArcSteps( Float64 px, Float64 py, Float64 nx, Float64 ny, UInt32 steps, bool counterClockwise, Float64 d )
    {
        const Float64 s = counterClockwise ? m_stepSine : -m_stepSine;
        Float64 x = nx, y = ny;
        for ( UInt32 i=1; i<steps; ++i )
        {
            Rotate( x, y, s );
            Add( px + d * x, py + d * y );
        }
    }

    /**
     *  The points of the arc around ( px, py ), radius 'd', from the unit normal ( nx, ny ) to the unit normal ( ex, ey ) in
     *  the given sense (a sweep of less than a half turn), the end points excluded: the rotated normals are taken
     *  while the end normal is still ahead of them (sign of the cross product).
     */
    void ArcToNormal( Float64 px, Float64 py, Float64 nx, Float64 ny, Float64 ex, Float64 ey, bool counterClockwise, Float64 d )
    {
        const Float64 s = counterClockwise ? m_stepSine : -m_stepSine;
        Float64 x = nx, y = ny;
        for ( UInt32 i=1; i<m_segments; ++i )
        {
            Rotate( x, y, s );
            const Float64 xe = x * ey;
            const Float64 ye = y * ex;
            const Float64 cross = xe - ye;
            if ( counterClockwise ? !( 0.0 < cross ) : !( cross < 0.0 ) )
                break;
            Add( px + d * x, py + d * y );
        }
    }

    /** The vertex ( px, py ) between edges with unit right normals n1 (incoming) and n2 (outgoing), offset by d */
    void Join( Float64 px, Float64 py, Float64 n1x, Float64 n1y, Float64 n2x, Float64 n2y, Float64 d )
    {
        const Float64 sine = n1x * n2y - n1y * n2x;
        const Float64 cosine = n1x * n2x + n1y * n2y;
        if ( fabs( sine ) < 1e-12 && cosine > 0.0 )
        {
            // straight on
            Add( px + d * n2x, py + d * n2y );
            return;
        }
        if ( sine * d < 0.0 )
        {
            // the offset edges overlap: through the vertex, a loop of negative winding that the union removes
            Add( px + d * n1x, py + d * n1y );
            Add( px, py );
            Add( px + d * n2x, py + d * n2y );
            return;
        }
        const bool spike = 1.0 + cosine < 1e-12;
        switch ( m_join )
        {
            case POLYGONJOIN_ROUND:
            {
                Add( px + d * n1x, py + d * n1y );
                if ( spike )
                    ArcSteps( px, py, n1x, n1y, ( m_segments + 1 ) / 2, d > 0.0, d );
                else
                    ArcToNormal( px, py, n1x, n1y, n2x, n2y, sine > 0.0, d );
                Add( px + d * n2x, py + d * n2y );
                return;
            }
            case POLYGONJOIN_MITER:
            {
                if ( !spike && 1.0 + cosine >= 2.0 / ( m_miterLimit * m_miterLimit ) )
                {
                    const Float64 f = d / ( 1.0 + cosine );
                    Add( px + f * ( n1x + n2x ), py + f * ( n1y + n2y ) );
                }
                else
                {
                    Add( px + d * n1x, py + d * n1y );
                    Add( px + d * n2x, py + d * n2y );
                }
                return;
            }
            default:
            {
                // the tangents: t = ( -n.y, n.x ) for a right normal n
                const Float64 t1x = -n1y, t1y = n1x, t2x = -n2y, t2y = n2x;
                if ( spike )
                {
                    const Float64 e = fabs( d );
                    Add( px + d * n1x + e * t1x, py + d * n1y + e * t1y );
                    Add( px + d * n2x + e * t1x, py + d * n2y + e * t1y );
                    return;
                }
                // the cut across the bisector b at distance d from the vertex: ( x - p ) . b = d
                Float64 bx = n1x + n2x, by = n1y + n2y;
                const Float64 length = sqrt( bx * bx + by * by );
                bx /= length;
                by /= length;
                const Float64 l1 = d * ( 1.0 - ( n1x * bx + n1y * by ) ) / ( t1x * bx + t1y * by );
                const Float64 l2 = d * ( 1.0 - ( n2x * bx + n2y * by ) ) / ( t2x * bx + t2y * by );
                Add( px + d * n1x + l1 * t1x, py + d * n1y + l1 * t1y );
                Add( px + d * n2x + l2 * t2x, py + d * n2y + l2 * t2y );
                return;
            }
        }
    }

    /** The cap at the end ( px, py ) of a stroke whose last edge has the direction ( tx, ty ) and right normal ( nx, ny ), half width d */
    void Cap( Float64 px, Float64 py, Float64 tx, Float64 ty, Float64 nx, Float64 ny, Float64 d, TPolylineEndType ends )
    {
        Add( px + d * nx, py + d * ny );
        if ( POLYLINEEND_SQUARE == ends )
        {
            Add( px + d * nx + d * tx, py + d * ny + d * ty );
            Add( px - d * nx + d * tx, py - d * ny + d * ty );
        }
        else
        if ( POLYLINEEND_ROUND == ends )
            ArcSteps( px, py, nx, ny, ( m_segments + 1 ) / 2, true, d );
        Add( px - d * nx, py - d * ny );
    }

    private:

    GUCEF::vector< Float64 >& m_out;
    TPolygonJoinType m_join;
    Float64 m_miterLimit;
    UInt32 m_segments;
    Float64 m_stepSine;
    Float64 m_stepCosine;
};

/*-------------------------------------------------------------------------*/

/** Unit right normals of the edges of 'count' points (x, y interleaved); closed adds the closing edge */
void
EdgeNormals( const Float64* xy, UInt32 count, bool closed, GUCEF::vector< Float64 >& normals )
{
    normals.clear();
    const UInt32 edges = closed ? count : count - 1;
    for ( UInt32 i=0; i<edges; ++i )
    {
        const UInt32 j = ( i + 1 ) % count;
        const Float64 dx = xy[ 2 * j ] - xy[ 2 * i ], dy = xy[ 2 * j + 1 ] - xy[ 2 * i + 1 ];
        const Float64 length = sqrt( dx * dx + dy * dy );
        normals.push_back( dy / length );
        normals.push_back( -dx / length );
    }
}

}  /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
CTPolygonOffsetter< T >::CTPolygonOffsetter( void )
    : m_points()
    , m_inputs()
    , m_work()
    , m_ring()
    , m_normals()
    , m_gridRing()
    , m_gridRings()
    , m_gridPolygons()
    , m_clipper( new CTPolygonClipper< Int64 >() )
    , m_miterLimit( 2.0 )
    , m_arcTolerance( 0.0 )
    , m_requestedScale( 0.0 )
    , m_scale( 1.0 )
{
}

/*-------------------------------------------------------------------------*/

template < typename T >
CTPolygonOffsetter< T >::~CTPolygonOffsetter()
{
    delete m_clipper;
    m_clipper = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

template < typename T >
void CTPolygonOffsetter< T >::SetMiterLimit( Float64 limit ) { m_miterLimit = limit > 1.0 ? limit : 1.0; }

template < typename T >
void CTPolygonOffsetter< T >::SetArcTolerance( Float64 tolerance ) { m_arcTolerance = tolerance > 0.0 ? tolerance : 0.0; }

template < typename T >
Float64 CTPolygonOffsetter< T >::GetGridScale( void ) const { return m_scale; }

template < typename T >
void
CTPolygonOffsetter< T >::SetGridScale( Float64 scale )
{
    if ( scale > 0.0 && scale == scale )
    {
        int exponent = 0;
        frexp( scale, &exponent );
        m_requestedScale = ldexp( 1.0, exponent - 1 );
    }
    else
        m_requestedScale = 0.0;
}

template < typename T >
void
CTPolygonOffsetter< T >::Clear( void )
{
    m_points.clear();
    m_inputs.clear();
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTPolygonOffsetter< T >::AddInput( const TPoint* points, UInt32 count, Int32 kind )
{
    if ( GUCEF_NULL == points || 0 == count )
        return;
    SInput input;
    input.first = (UInt32) m_points.size();
    input.count = count;
    input.kind = kind;
    m_points.insert( m_points.end(), points, points + count );
    m_inputs.push_back( input );
}

template < typename T >
void CTPolygonOffsetter< T >::AddPolygon( const TPoint* ring, UInt32 count ) { AddInput( ring, count, 1 ); }

template < typename T >
void CTPolygonOffsetter< T >::AddPolygon( const TPath& ring ) { AddInput( ring.empty() ? GUCEF_NULL : &ring[ 0 ], (UInt32) ring.size(), 1 ); }

template < typename T >
void CTPolygonOffsetter< T >::AddPolygon( const CTPolygon2D< T >& polygon ) { AddPolygon( polygon.points ); }

template < typename T >
void
CTPolygonOffsetter< T >::AddPolygon( const TPolygon& polygon )
{
    AddPolygon( polygon.outer.points );
    for ( size_t h=0; h<polygon.holes.size(); ++h )
        AddInput( polygon.holes[ h ].points.empty() ? GUCEF_NULL : &polygon.holes[ h ].points[ 0 ], (UInt32) polygon.holes[ h ].points.size(), -1 );
}

template < typename T >
void CTPolygonOffsetter< T >::AddPolyline( const TPoint* points, UInt32 count ) { AddInput( points, count, 0 ); }

template < typename T >
void CTPolygonOffsetter< T >::AddPolyline( const TPath& points ) { AddInput( points.empty() ? GUCEF_NULL : &points[ 0 ], (UInt32) points.size(), 0 ); }

template < typename T >
void CTPolygonOffsetter< T >::AddPolyline( const CTPolyline2D< T >& polyline ) { AddPolyline( polyline.points ); }

/*-------------------------------------------------------------------------*/

template < typename T >
TPolygonClipperStatus
CTPolygonOffsetter< T >::Build( const T& delta, TPolygonJoinType join, TPolylineEndType ends )
{
    typedef CTOffsetGrid< T > TGrid;
    if ( !TGrid::IsFinite( delta ) || (UInt32) join > (UInt32) POLYGONJOIN_MITER || (UInt32) ends > (UInt32) POLYLINEEND_ROUND )
        return POLYGONCLIPPER_INVALID_INPUT;
    const Float64 deltaValue = TGrid::ToFloat64( delta );
    Float64 largest = 0.0;
    for ( size_t i=0; i<m_points.size(); ++i )
    {
        if ( !TGrid::IsFinite( m_points[ i ].x ) || !TGrid::IsFinite( m_points[ i ].y ) )
            return POLYGONCLIPPER_INVALID_INPUT;
        largest = std::max( largest, std::max( fabs( TGrid::ToFloat64( m_points[ i ].x ) ), fabs( TGrid::ToFloat64( m_points[ i ].y ) ) ) );
    }
    // the extent grows by the miter (or square) reach
    const Float64 reach = largest + fabs( deltaValue ) * std::max( m_miterLimit, 2.0 );
    m_scale = TGrid::FixedScale();
    if ( 0.0 == m_scale )
    {
        if ( m_requestedScale > 0.0 )
            m_scale = m_requestedScale;
        else
        if ( reach > 0.0 )
        {
            // the reach below 2^40; at most 2^1023, the largest finite power of two (tiny inputs)
            int exponent = 0;
            frexp( reach, &exponent );
            m_scale = ldexp( 1.0, std::min( 40 - exponent, 1023 ) );
        }
        else
            m_scale = 1.0;
    }
    if ( reach * m_scale > OffsetGridLimit )
        return POLYGONCLIPPER_OUT_OF_RANGE;

    const Float64 d = deltaValue * m_scale;
    Float64 tolerance = m_arcTolerance > 0.0 ? m_arcTolerance * m_scale : fabs( d ) / 400.0;
    tolerance = std::max( std::min( tolerance, fabs( d ) ), 0.25 );
    const Float64 ratio = fabs( d ) > 0.0 ? 1.0 - tolerance / fabs( d ) : 0.0;
    COffsetBuilder builder( m_ring, join, m_miterLimit, ratio );
    m_clipper->Clear();
    GUCEF::vector< Float64 >& xy = m_work;
    GUCEF::vector< Float64 >& normals = m_normals;
    for ( size_t r=0; r<m_inputs.size(); ++r )
    {
        const SInput& input = m_inputs[ r ];
        // grid coordinates without consecutive duplicates
        xy.clear();
        for ( UInt32 i=0; i<input.count; ++i )
        {
            const Float64 x = TGrid::ToGrid( m_points[ input.first + i ].x, m_scale );
            const Float64 y = TGrid::ToGrid( m_points[ input.first + i ].y, m_scale );
            if ( !xy.empty() && x == xy[ xy.size() - 2 ] && y == xy[ xy.size() - 1 ] )
                continue;
            xy.push_back( x );
            xy.push_back( y );
        }
        UInt32 n = (UInt32) xy.size() / 2;
        if ( 0 != input.kind )
            while ( n > 1 && xy[ 0 ] == xy[ 2 * n - 2 ] && xy[ 1 ] == xy[ 2 * n - 1 ] )
                --n;
        m_ring.clear();
        if ( 0 != input.kind )
        {
            if ( n < 3 )
                continue;
            // a ring whose points all lie on one line encloses nothing: ignored like a ring of two points (exact test)
            bool flat = true;
            for ( UInt32 i=2; i<n && flat; ++i )
                flat = 0 == CTPredicates< Float64 >::Orient2D( MATH::CTPoint2D< Float64 >( xy[ 0 ], xy[ 1 ] ), MATH::CTPoint2D< Float64 >( xy[ 2 ], xy[ 3 ] ),
                                                              MATH::CTPoint2D< Float64 >( xy[ 2 * i ], xy[ 2 * i + 1 ] ) );
            if ( flat )
                continue;
            // the requested orientation
            Float64 area = 0.0;
            for ( UInt32 i=1; i+1<n; ++i )
                area += ( xy[ 2 * i ] - xy[ 0 ] ) * ( xy[ 2 * i + 3 ] - xy[ 1 ] ) - ( xy[ 2 * i + 1 ] - xy[ 1 ] ) * ( xy[ 2 * i + 2 ] - xy[ 0 ] );
            if ( ( area < 0.0 && input.kind > 0 ) || ( area > 0.0 && input.kind < 0 ) )
            {
                for ( UInt32 i=0; i<n/2; ++i )
                {
                    std::swap( xy[ 2 * i ], xy[ 2 * ( n - 1 - i ) ] );
                    std::swap( xy[ 2 * i + 1 ], xy[ 2 * ( n - 1 - i ) + 1 ] );
                }
            }
            if ( 0.0 == d )
            {
                for ( UInt32 i=0; i<n; ++i )
                    builder.Add( xy[ 2 * i ], xy[ 2 * i + 1 ] );
            }
            else
            {
                EdgeNormals( &xy[ 0 ], n, true, normals );
                for ( UInt32 k=0; k<n; ++k )
                {
                    const UInt32 in = ( k + n - 1 ) % n;
                    builder.Join( xy[ 2 * k ], xy[ 2 * k + 1 ], normals[ 2 * in ], normals[ 2 * in + 1 ], normals[ 2 * k ], normals[ 2 * k + 1 ], d );
                }
            }
        }
        else
        {
            const Float64 w = fabs( d );
            if ( 0.0 == w || 0 == n )
                continue;
            if ( 1 == n )
            {
                const Float64 px = xy[ 0 ], py = xy[ 1 ];
                if ( POLYLINEEND_ROUND == ends )
                {
                    builder.Add( px + w, py );
                    builder.ArcSteps( px, py, 1.0, 0.0, builder.GetSegmentsPerTurn(), true, w );
                }
                else
                if ( POLYLINEEND_SQUARE == ends )
                {
                    builder.Add( px - w, py - w );
                    builder.Add( px + w, py - w );
                    builder.Add( px + w, py + w );
                    builder.Add( px - w, py + w );
                }
            }
            else
            {
                EdgeNormals( &xy[ 0 ], n, false, normals );
                const UInt32 last = n - 2;
                // the right side forwards, the end cap, the left side backwards, the start cap
                builder.Add( xy[ 0 ] + w * normals[ 0 ], xy[ 1 ] + w * normals[ 1 ] );
                for ( UInt32 k=1; k+1<n; ++k )
                    builder.Join( xy[ 2 * k ], xy[ 2 * k + 1 ], normals[ 2 * ( k - 1 ) ], normals[ 2 * ( k - 1 ) + 1 ], normals[ 2 * k ], normals[ 2 * k + 1 ], w );
                builder.Cap( xy[ 2 * n - 2 ], xy[ 2 * n - 1 ], -normals[ 2 * last + 1 ], normals[ 2 * last ], normals[ 2 * last ], normals[ 2 * last + 1 ], w, ends );
                for ( UInt32 k=n-2; k>=1; --k )
                    builder.Join( xy[ 2 * k ], xy[ 2 * k + 1 ], -normals[ 2 * k ], -normals[ 2 * k + 1 ], -normals[ 2 * ( k - 1 ) ], -normals[ 2 * ( k - 1 ) + 1 ], w );
                builder.Cap( xy[ 0 ], xy[ 1 ], normals[ 1 ], -normals[ 0 ], -normals[ 0 ], -normals[ 1 ], w, ends );
            }
        }
        // to the grid
        m_gridRing.clear();
        for ( size_t i=0; i+1<m_ring.size(); i+=2 )
        {
            const Float64 gx = floor( m_ring[ i ] + 0.5 ), gy = floor( m_ring[ i + 1 ] + 0.5 );
            if ( !( fabs( gx ) <= OffsetGridLimit ) || !( fabs( gy ) <= OffsetGridLimit ) )
                return POLYGONCLIPPER_OUT_OF_RANGE;
            m_gridRing.push_back( MATH::CTPoint2D< Int64 >( (Int64) gx, (Int64) gy ) );
        }
        m_clipper->AddSubject( m_gridRing );
    }
    return POLYGONCLIPPER_OK;
}

/*-------------------------------------------------------------------------*/

template < typename T >
TPolygonClipperStatus
CTPolygonOffsetter< T >::Execute( const T& delta, TPolygonJoinType join, TPolylineEndType ends, TPaths& rings )
{
    typedef CTOffsetGrid< T > TGrid;
    rings.clear();
    const TPolygonClipperStatus status = Build( delta, join, ends );
    if ( POLYGONCLIPPER_OK != status )
        return status;
    GUCEF::vector< GUCEF::vector< MATH::CTPoint2D< Int64 > > >& gridRings = m_gridRings;
    const TPolygonClipperStatus unionStatus = m_clipper->Execute( POLYGONBOOLEAN_UNION, POLYGONFILL_POSITIVE, POLYGONFILL_POSITIVE, gridRings );
    if ( POLYGONCLIPPER_OK != unionStatus )
        return unionStatus;
    // the result must fit the kind (an integer result beyond its range would wrap)
    for ( size_t r=0; r<gridRings.size(); ++r )
        for ( size_t i=0; i<gridRings[ r ].size(); ++i )
            if ( !TGrid::InRange( gridRings[ r ][ i ].x, m_scale ) || !TGrid::InRange( gridRings[ r ][ i ].y, m_scale ) )
                return POLYGONCLIPPER_OUT_OF_RANGE;
    rings.resize( gridRings.size() );
    for ( size_t r=0; r<gridRings.size(); ++r )
    {
        rings[ r ].resize( gridRings[ r ].size() );
        for ( size_t i=0; i<gridRings[ r ].size(); ++i )
            rings[ r ][ i ] = TPoint( TGrid::FromGrid( gridRings[ r ][ i ].x, m_scale ), TGrid::FromGrid( gridRings[ r ][ i ].y, m_scale ) );
    }
    return POLYGONCLIPPER_OK;
}

/*-------------------------------------------------------------------------*/

template < typename T >
TPolygonClipperStatus
CTPolygonOffsetter< T >::Execute( const T& delta, TPolygonJoinType join, TPolylineEndType ends, TPolygons& polygons )
{
    typedef CTOffsetGrid< T > TGrid;
    polygons.clear();
    const TPolygonClipperStatus status = Build( delta, join, ends );
    if ( POLYGONCLIPPER_OK != status )
        return status;
    GUCEF::vector< CTPolygonWithHoles2D< Int64 > >& gridPolygons = m_gridPolygons;
    const TPolygonClipperStatus unionStatus = m_clipper->Execute( POLYGONBOOLEAN_UNION, POLYGONFILL_POSITIVE, POLYGONFILL_POSITIVE, gridPolygons );
    if ( POLYGONCLIPPER_OK != unionStatus )
        return unionStatus;
    // the result must fit the kind (an integer result beyond its range would wrap)
    for ( size_t p=0; p<gridPolygons.size(); ++p )
    {
        const CTPolygonWithHoles2D< Int64 >& source = gridPolygons[ p ];
        for ( size_t i=0; i<source.outer.points.size(); ++i )
            if ( !TGrid::InRange( source.outer.points[ i ].x, m_scale ) || !TGrid::InRange( source.outer.points[ i ].y, m_scale ) )
                return POLYGONCLIPPER_OUT_OF_RANGE;
    }
    polygons.resize( gridPolygons.size() );
    for ( size_t p=0; p<gridPolygons.size(); ++p )
    {
        const CTPolygonWithHoles2D< Int64 >& source = gridPolygons[ p ];
        TPolygon& target = polygons[ p ];
        for ( size_t i=0; i<source.outer.points.size(); ++i )
            target.outer.points.push_back( TPoint( TGrid::FromGrid( source.outer.points[ i ].x, m_scale ), TGrid::FromGrid( source.outer.points[ i ].y, m_scale ) ) );
        target.holes.resize( source.holes.size() );
        for ( size_t h=0; h<source.holes.size(); ++h )
            for ( size_t i=0; i<source.holes[ h ].points.size(); ++i )
                target.holes[ h ].points.push_back( TPoint( TGrid::FromGrid( source.holes[ h ].points[ i ].x, m_scale ), TGrid::FromGrid( source.holes[ h ].points[ i ].y, m_scale ) ) );
    }
    return POLYGONCLIPPER_OK;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      INSTANTIATIONS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTPolygonOffsetter< Float32 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTPolygonOffsetter< Float64 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTPolygonOffsetter< Int32 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTPolygonOffsetter< Int64 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTPolygonOffsetter< MATH::TFixed32_32 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
