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

#ifndef GUCEF_GEOMETRY_CTIMAGESOURCEMODEL_H
#define GUCEF_GEOMETRY_CTIMAGESOURCEMODEL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_CTPLANE_H
#include "gucefGEOMETRY_CTPlane.h"
#define GUCEF_GEOMETRY_CTPLANE_H
#endif /* GUCEF_GEOMETRY_CTPLANE_H ? */

#ifndef GUCEF_GEOMETRY_CTPOLYGON2D_H
#include "gucefGEOMETRY_CTPolygon2D.h"
#define GUCEF_GEOMETRY_CTPOLYGON2D_H
#endif /* GUCEF_GEOMETRY_CTPOLYGON2D_H ? */

#ifndef GUCEF_GEOMETRY_CTGEOPLANE_H
#include "gucefGEOMETRY_CTGeoPlane.h"
#define GUCEF_GEOMETRY_CTGEOPLANE_H
#endif /* GUCEF_GEOMETRY_CTGEOPLANE_H ? */

#ifndef GUCEF_GEOMETRY_CTRAYCASTER_H
#include "gucefGEOMETRY_CTRayCaster.h"
#define GUCEF_GEOMETRY_CTRAYCASTER_H
#endif /* GUCEF_GEOMETRY_CTRAYCASTER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A reflecting wall of the image source model: a plane with a finite planar polygon in it (or no polygon: the whole
 *  plane). The front of the plane is the side its normal points to; for a room the normals point into the room.
 *
 *  - FromPolygon takes the corners of a planar polygon in 3D (convex or not, either winding): the plane normal is the
 *    Newell normal of the corners, so counter-clockwise seen from the front. False when the polygon has fewer than
 *    three corners or no area. The corners must lie in one plane (the reflection point test projects onto it).
 *  - FromGeoPlane takes a CTGeoPlane: bounded by its extents when it has them, else the whole plane; the front is the
 *    world +Z side of the plane object.
 *  - 'userId' is the owner's tag (a material, a surface index) and is reported with every path.
 *  - Kinds: Float32 and Float64 are exported.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTImageSourceWall
{
    public:

    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTPoint2D< T > TPoint2D;
    typedef MATH::CTVector3D< T > TVector;

    /** An unbounded wall: the plane y = 0 */
    CTImageSourceWall( void ) : m_plane(), m_origin(), m_axisU( TTraits::One(), TTraits::Zero(), TTraits::Zero() ), m_axisV( TTraits::Zero(), TTraits::Zero(), TTraits::One() ), m_outline(), m_bounded( false ), m_userId( 0 ) {}

    /** An unbounded wall in 'plane' */
    static CTImageSourceWall FromPlane( const CTPlane< T >& plane, UInt32 userId = 0 );

    /** A wall bounded by a planar polygon given by its corners; false (out unchanged) for fewer than three corners or no area */
    static bool FromPolygon( const TPoint* corners, UInt32 count, CTImageSourceWall& out, UInt32 userId = 0 );

    /** A wall from a linked plane, bounded by its extents when it has them */
    static bool FromGeoPlane( const CTGeoPlane< T >& plane, CTImageSourceWall& out, UInt32 userId = 0 );

    const CTPlane< T >& GetPlane( void ) const { return m_plane; }
    bool IsBounded( void ) const { return m_bounded; }
    UInt32 GetUserId( void ) const { return m_userId; }
    void SetUserId( UInt32 userId ) { m_userId = userId; }

    /** The 2D coordinates of the foot of the perpendicular of a world point in the frame of the wall polygon */
    TPoint2D ToWallCoordinates( const TPoint& point ) const;

    /** True when the point is within 'tolerance' of the wall plane and its foot lies in the polygon (boundary included, widened by the tolerance) */
    bool ContainsPoint( const TPoint& point, const T& tolerance ) const;

    private:

    CTPlane< T > m_plane;
    TPoint m_origin;                    /**< the origin of the polygon frame */
    TVector m_axisU;                    /**< unit axes in the plane, m_axisU x m_axisV = normal */
    TVector m_axisV;
    CTPolygon2D< T > m_outline;         /**< in the coordinates of the frame; empty when unbounded */
    bool m_bounded;
    UInt32 m_userId;
};

/*-------------------------------------------------------------------------*/

/** One path of an image source model: the virtual source and where the sound reflects (audio design: the early reflection stages) */
template < typename T >
struct GUCEF_GEOMETRY_TEMPLATE_PUBLIC SImageSourcePath
{
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;

    enum { MaxOrder = 2 };

    TPoint position;                    /**< the virtual source: the source mirrored in the reflecting walls, in order */
    UInt32 order;                       /**< the number of reflections: 0 (the direct path), 1 or 2 */
    UInt32 wallIndex[ 2 ];              /**< the walls in the order the sound meets them going from the source to the listener; unused entries are 0xFFFFFFFF */
    UInt32 wallUserId[ 2 ];             /**< the user ids of those walls */
    TPoint reflectionPoint[ 2 ];        /**< the reflection points, in the same order */
    T pathLength;                       /**< the length of the whole path, equal to the distance from the listener to 'position' */
    TVector arrivalDirection;           /**< the unit vector from the listener to where the sound arrives from (the last reflection point; the source for order 0) */

    SImageSourcePath( void ) : position(), order( 0 ), pathLength( T( 0 ) ), arrivalDirection()
    {
        wallIndex[ 0 ] = wallIndex[ 1 ] = 0xFFFFFFFFU;
        wallUserId[ 0 ] = wallUserId[ 1 ] = 0;
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  Image source reflections (section 7.11 of the design): the first and second order specular reflections of a source
 *  in a set of planar walls, with the validation of each path, for the early reflection and reverb stages of the audio
 *  side.
 *
 *  Method: the image of the source in a wall is its mirror image in the wall plane (CTPlane::Mirror); the second order
 *  image of the ordered pair ( a, b ) is the image of the first image of a in b ( a is met first going from the source,
 *  b last). The straight line from the listener to the image gives the reflection points (the last wall first); the
 *  path is kept when
 *   - the listener and the source are on the same side of a first order wall (for second order the line from the
 *     listener to the image crosses the last wall, and the line from that point to the first image crosses the first
 *     wall), so each reflection point lies on its segment strictly between its ends; a second reflection point that
 *     coincides with the first (within the point tolerance) is a path through the edge of two walls and is kept;
 *   - every reflection point lies inside the polygon of its wall (boundary included, within the point tolerance);
 *   - when the walls are one sided, every reflection arrives at the front of the wall it hits;
 *   - with a ray caster set, none of the segments of the path is blocked by an object of the caster (layer mask and
 *     filter as set); a segment is tested without the 'segment margin' at the ends that lie on a wall, so the walls
 *     themselves do not block their own reflection.
 *  Walls whose plane contains the source or the listener (within the point tolerance) give no reflection.
 *
 *  - A shoebox gives 6 first order paths and, for a source and listener inside it, 18 distinct second order paths: the
 *    30 ordered wall pairs give 18 distinct images (the 12 pairs of perpendicular walls give the same image in both
 *    orders, of which only the physical order passes validation; the 3 pairs of opposite walls give two images each).
 *  - Two second order paths with the same image (within four point tolerances) are one path.
 *  - Results are ordered by path length (shortest first), then by order, then by wall indices: deterministic.
 *  - Threading: owner thread; the model owns its walls, the ray caster is only read (queries need Update after the last
 *    change). Not realtime-safe. Results go into the caller's vector. Kinds: Float32 and Float64 are exported.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTImageSourceModel
{
    public:

    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    typedef CTImageSourceWall< T > TWall;
    typedef SImageSourcePath< T > TPath;
    typedef CTRayCaster< T > TRayCaster;
    typedef CTGeoRayFilter< T > TFilter;

    /** The reflection orders of the highest order that is supported */
    enum { MaxSupportedOrder = 2 };

    /** An empty model: order 2, two sided walls, no direct path in the results, no ray caster, tolerance 1e-4 of the kind scale, segment margin 1e-3 */
    CTImageSourceModel( void );

    /** Adds a wall; returns its index */
    UInt32 AddWall( const TWall& wall );

    /**
     *  Adds the six inner faces of the box between two corners ( min corner less than max corner on every axis ), normals
     *  pointing into the box, in the order -X, +X, -Y, +Y, -Z, +Z; returns the index of the first. 'firstUserId'
     *  and the five following numbers are their user ids. False (nothing added) for an empty box.
     */
    bool AddShoebox( const TPoint& minCorner, const TPoint& maxCorner, UInt32 firstUserId, UInt32& firstIndex );

    void Clear( void ) { m_walls.clear(); }
    UInt32 GetWallCount( void ) const { return (UInt32) m_walls.size(); }
    const TWall& GetWall( UInt32 index ) const { return m_walls[ index ]; }

    /** The highest reflection order that is computed, 1 or 2 (default 2) */
    void SetMaxOrder( UInt32 order ) { m_maxOrder = order < 1 ? 1 : ( order > (UInt32) MaxSupportedOrder ? (UInt32) MaxSupportedOrder : order ); }
    UInt32 GetMaxOrder( void ) const { return m_maxOrder; }

    /** Whether the direct path is part of the results as order 0 (default false) */
    void SetIncludeDirect( bool include ) { m_includeDirect = include; }
    bool GetIncludeDirect( void ) const { return m_includeDirect; }

    /** One sided walls reflect only on their front (default false: a wall reflects on both sides) */
    void SetOneSided( bool oneSided ) { m_oneSided = oneSided; }
    bool GetOneSided( void ) const { return m_oneSided; }

    /** The ray caster whose objects block the segments of a path; not owned, null skips the test (default) */
    void SetRayCaster( const TRayCaster* rayCaster ) { m_rayCaster = rayCaster; }
    void SetLayerMask( UInt32 layerMask ) { m_layerMask = layerMask; }
    void SetFilter( const TFilter* filter ) { m_filter = filter; }

    /** The distance in world units that decides whether a point is on a plane, and widens the wall polygons (default 1e-4) */
    void SetPointTolerance( const T& tolerance ) { if ( tolerance >= TTraits::Zero() ) m_pointTolerance = tolerance; }
    const T& GetPointTolerance( void ) const { return m_pointTolerance; }

    /** The length in world units that a segment is shortened by at its end on a wall for the occlusion test (default 1e-3) */
    void SetSegmentMargin( const T& margin ) { if ( margin >= TTraits::Zero() ) m_segmentMargin = margin; }
    const T& GetSegmentMargin( void ) const { return m_segmentMargin; }

    /**
     *  The images of 'source' without any validation: one per wall for order 1, one per ordered pair of different walls
     *  for order 2, appended in wall order (order 1 first). reflection points and path lengths are not set. Returns the number appended.
     */
    UInt32 BuildImages( const TPoint& source, UInt32 maxOrder, GUCEF::vector< TPath >& images ) const;

    /** The valid paths from 'source' to 'listener' (replaces the contents of 'paths'); returns their number */
    UInt32 FindPaths( const TPoint& source, const TPoint& listener, GUCEF::vector< TPath >& paths ) const;

    private:

    bool Open( const TPoint& from, const TPoint& to, bool fromOnWall, bool toOnWall ) const;

    bool TryFirstOrder( const TPoint& source, const TPoint& listener, UInt32 wall, TPath& path ) const;

    bool TrySecondOrder( const TPoint& source, const TPoint& listener, UInt32 first, UInt32 last, TPath& path ) const;

    /**
     *  The point where the segment a to b meets the plane; false when the ends are not strictly on opposite sides. With
     *  'startMayTouch' the start a may lie on the plane (within the point tolerance, as the reflection point of the wall
     *  before at a corner does) and is then the crossing point; the end b never may.
     */
    bool CrossPlane( const CTPlane< T >& plane, const TPoint& a, const TPoint& b, bool startMayTouch, TPoint& point ) const;

    static bool Before( const TPath& a, const TPath& b );

    private:

    GUCEF::vector< TWall > m_walls;
    UInt32 m_maxOrder;
    bool m_includeDirect;
    bool m_oneSided;
    const TRayCaster* m_rayCaster;
    const TFilter* m_filter;
    UInt32 m_layerMask;
    T m_pointTolerance;
    T m_segmentMargin;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
CTImageSourceWall< T >
CTImageSourceWall< T >::FromPlane( const CTPlane< T >& plane, UInt32 userId )
{
    CTImageSourceWall wall;
    wall.m_plane = plane;
    wall.m_userId = userId;
    wall.m_bounded = false;
    wall.m_origin = TPoint( TTraits::Zero(), TTraits::Zero(), TTraits::Zero() ) + plane.normal.AsVector() * plane.offset;
    MATH::CTDirectionVector3D< T > helper = TTraits::Abs( plane.normal.AsVector().x ) < TTraits::Half() ? MATH::CTDirectionVector3D< T >::UnitX() : MATH::CTDirectionVector3D< T >::UnitY();
    TVector u = helper.AsVector().Cross( plane.normal.AsVector() ).Normalized();
    wall.m_axisU = u;
    wall.m_axisV = plane.normal.AsVector().Cross( u );
    return wall;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTImageSourceWall< T >::FromPolygon( const TPoint* corners, UInt32 count, CTImageSourceWall& out, UInt32 userId )
{
    if ( GUCEF_NULL == corners || count < 3 )
        return false;

    // the Newell normal: sum over the edges, independent of the starting corner
    TVector sum;
    for ( UInt32 i=0; i<count; ++i )
    {
        const TPoint& a = corners[ i ];
        const TPoint& b = corners[ ( i + 1 ) % count ];
        sum = sum + TVector( ( a.y - b.y ) * ( a.z + b.z ), ( a.z - b.z ) * ( a.x + b.x ), ( a.x - b.x ) * ( a.y + b.y ) );
    }
    MATH::CTDirectionVector3D< T > normal;
    if ( !MATH::CTDirectionVector3D< T >::TryCreate( sum, normal ) )
        return false;

    // the first edge that has a length gives the first axis
    TVector u;
    bool haveU = false;
    for ( UInt32 i=1; i<count && !haveU; ++i )
    {
        TVector edge = corners[ i ] - corners[ 0 ];
        edge = edge - normal.AsVector() * edge.Dot( normal.AsVector() );
        haveU = edge.TryNormalize( u, TTraits::Epsilon() );
    }
    if ( !haveU )
        return false;

    CTImageSourceWall wall;
    wall.m_userId = userId;
    wall.m_bounded = true;
    wall.m_plane = CTPlane< T >::FromPointNormal( corners[ 0 ], normal );
    wall.m_origin = corners[ 0 ];
    wall.m_axisU = u;
    wall.m_axisV = normal.AsVector().Cross( u );
    for ( UInt32 i=0; i<count; ++i )
        wall.m_outline.AddPoint( wall.ToWallCoordinates( corners[ i ] ) );
    out = wall;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTImageSourceWall< T >::FromGeoPlane( const CTGeoPlane< T >& plane, CTImageSourceWall& out, UInt32 userId )
{
    const CTPlane< T > worldPlane = plane.GetPlane();
    if ( !plane.HasExtents() )
    {
        out = FromPlane( worldPlane, userId );
        return true;
    }
    const CTRectangle< T >& extents = plane.GetExtents();
    if ( extents.IsEmpty() )
        return false;
    TPoint corners[ 4 ];
    corners[ 0 ] = plane.FromPlane( TPoint2D( extents.min.x, extents.min.y ) );
    corners[ 1 ] = plane.FromPlane( TPoint2D( extents.max.x, extents.min.y ) );
    corners[ 2 ] = plane.FromPlane( TPoint2D( extents.max.x, extents.max.y ) );
    corners[ 3 ] = plane.FromPlane( TPoint2D( extents.min.x, extents.max.y ) );
    CTImageSourceWall wall;
    if ( !FromPolygon( corners, 4, wall, userId ) )
        return false;
    if ( wall.m_plane.normal.AsVector().Dot( worldPlane.normal.AsVector() ) < TTraits::Zero() )
    {
        // a mirroring placement: keep the front of the plane object
        wall.m_plane = wall.m_plane.Flipped();
        wall.m_axisV = wall.m_plane.normal.AsVector().Cross( wall.m_axisU );
        wall.m_outline.Clear();
        for ( UInt32 i=0; i<4; ++i )
            wall.m_outline.AddPoint( wall.ToWallCoordinates( corners[ i ] ) );
    }
    out = wall;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
typename CTImageSourceWall< T >::TPoint2D
CTImageSourceWall< T >::ToWallCoordinates( const TPoint& point ) const
{
    const TVector d = point - m_origin;
    return TPoint2D( d.Dot( m_axisU ), d.Dot( m_axisV ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTImageSourceWall< T >::ContainsPoint( const TPoint& point, const T& tolerance ) const
{
    if ( TTraits::Abs( m_plane.GetSignedDistance( point ) ) > tolerance )
        return false;
    if ( !m_bounded )
        return true;
    const TPoint2D p = ToWallCoordinates( point );
    return m_outline.Contains( p ) || m_outline.GetDistance( p ) <= tolerance;
}

/*-------------------------------------------------------------------------*/

template < typename T >
CTImageSourceModel< T >::CTImageSourceModel( void )
    : m_walls(), m_maxOrder( 2 ), m_includeDirect( false ), m_oneSided( false ), m_rayCaster( GUCEF_NULL ), m_filter( GUCEF_NULL ),
      m_layerMask( 0xFFFFFFFFU ), m_pointTolerance( T( 1e-4 ) ), m_segmentMargin( T( 1e-3 ) )
{
}

/*-------------------------------------------------------------------------*/

template < typename T >
UInt32
CTImageSourceModel< T >::AddWall( const TWall& wall )
{
    m_walls.push_back( wall );
    return (UInt32) m_walls.size() - 1;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTImageSourceModel< T >::AddShoebox( const TPoint& minCorner, const TPoint& maxCorner, UInt32 firstUserId, UInt32& firstIndex )
{
    if ( !( minCorner.x < maxCorner.x ) || !( minCorner.y < maxCorner.y ) || !( minCorner.z < maxCorner.z ) )
        return false;

    const T x0 = minCorner.x; const T x1 = maxCorner.x;
    const T y0 = minCorner.y; const T y1 = maxCorner.y;
    const T z0 = minCorner.z; const T z1 = maxCorner.z;
    firstIndex = (UInt32) m_walls.size();

    // each face counter-clockwise seen from inside the box
    TPoint c[ 4 ];
    TWall wall;
    bool ok = true;

    c[ 0 ] = TPoint( x0, y0, z0 ); c[ 1 ] = TPoint( x0, y0, z1 ); c[ 2 ] = TPoint( x0, y1, z1 ); c[ 3 ] = TPoint( x0, y1, z0 );     // -X, normal +X
    ok = TWall::FromPolygon( c, 4, wall, firstUserId ) && ok; AddWall( wall );
    c[ 0 ] = TPoint( x1, y0, z0 ); c[ 1 ] = TPoint( x1, y1, z0 ); c[ 2 ] = TPoint( x1, y1, z1 ); c[ 3 ] = TPoint( x1, y0, z1 );     // +X, normal -X
    ok = TWall::FromPolygon( c, 4, wall, firstUserId + 1 ) && ok; AddWall( wall );
    c[ 0 ] = TPoint( x0, y0, z0 ); c[ 1 ] = TPoint( x1, y0, z0 ); c[ 2 ] = TPoint( x1, y0, z1 ); c[ 3 ] = TPoint( x0, y0, z1 );     // -Y, normal +Y
    ok = TWall::FromPolygon( c, 4, wall, firstUserId + 2 ) && ok; AddWall( wall );
    c[ 0 ] = TPoint( x0, y1, z0 ); c[ 1 ] = TPoint( x0, y1, z1 ); c[ 2 ] = TPoint( x1, y1, z1 ); c[ 3 ] = TPoint( x1, y1, z0 );     // +Y, normal -Y
    ok = TWall::FromPolygon( c, 4, wall, firstUserId + 3 ) && ok; AddWall( wall );
    c[ 0 ] = TPoint( x0, y0, z0 ); c[ 1 ] = TPoint( x0, y1, z0 ); c[ 2 ] = TPoint( x1, y1, z0 ); c[ 3 ] = TPoint( x1, y0, z0 );     // -Z, normal +Z
    ok = TWall::FromPolygon( c, 4, wall, firstUserId + 4 ) && ok; AddWall( wall );
    c[ 0 ] = TPoint( x0, y0, z1 ); c[ 1 ] = TPoint( x1, y0, z1 ); c[ 2 ] = TPoint( x1, y1, z1 ); c[ 3 ] = TPoint( x0, y1, z1 );     // +Z, normal -Z
    ok = TWall::FromPolygon( c, 4, wall, firstUserId + 5 ) && ok; AddWall( wall );
    return ok;
}

/*-------------------------------------------------------------------------*/

template < typename T >
UInt32
CTImageSourceModel< T >::BuildImages( const TPoint& source, UInt32 maxOrder, GUCEF::vector< TPath >& images ) const
{
    const UInt32 wallCount = (UInt32) m_walls.size();
    const UInt32 before = (UInt32) images.size();
    for ( UInt32 a=0; a<wallCount; ++a )
    {
        TPath path;
        path.order = 1;
        path.wallIndex[ 0 ] = a;
        path.wallUserId[ 0 ] = m_walls[ a ].GetUserId();
        path.position = m_walls[ a ].GetPlane().Mirror( source );
        images.push_back( path );
    }
    if ( maxOrder >= 2 )
    {
        for ( UInt32 a=0; a<wallCount; ++a )
        {
            const TPoint first = m_walls[ a ].GetPlane().Mirror( source );
            for ( UInt32 b=0; b<wallCount; ++b )
            {
                if ( a == b )
                    continue;
                TPath path;
                path.order = 2;
                path.wallIndex[ 0 ] = a;
                path.wallIndex[ 1 ] = b;
                path.wallUserId[ 0 ] = m_walls[ a ].GetUserId();
                path.wallUserId[ 1 ] = m_walls[ b ].GetUserId();
                path.position = m_walls[ b ].GetPlane().Mirror( first );
                images.push_back( path );
            }
        }
    }
    return (UInt32) images.size() - before;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTImageSourceModel< T >::Open( const TPoint& from, const TPoint& to, bool fromOnWall, bool toOnWall ) const
{
    if ( GUCEF_NULL == m_rayCaster )
        return true;
    const TVector delta = to - from;
    const T length = delta.GetLength();
    const T cut = ( fromOnWall ? m_segmentMargin : TTraits::Zero() ) + ( toOnWall ? m_segmentMargin : TTraits::Zero() );
    if ( !( length > cut ) )
        return true;
    const TVector unit = delta * ( TTraits::One() / length );
    const TPoint a = fromOnWall ? from + unit * m_segmentMargin : from;
    const TPoint b = toOnWall ? to - unit * m_segmentMargin : to;
    return m_rayCaster->HasLineOfSight( a, b, m_layerMask, m_filter );
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTImageSourceModel< T >::CrossPlane( const CTPlane< T >& plane, const TPoint& a, const TPoint& b, bool startMayTouch, TPoint& point ) const
{
    const T da = plane.GetSignedDistance( a );
    const T db = plane.GetSignedDistance( b );
    if ( startMayTouch && TTraits::Abs( da ) <= m_pointTolerance && TTraits::Abs( db ) > m_pointTolerance )
    {
        point = plane.Project( a );
        return true;
    }
    if ( !( da > m_pointTolerance && db < -m_pointTolerance ) && !( da < -m_pointTolerance && db > m_pointTolerance ) )
        return false;
    const T t = da / ( da - db );
    point = a + ( b - a ) * t;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTImageSourceModel< T >::TryFirstOrder( const TPoint& source, const TPoint& listener, UInt32 wallIndex, TPath& path ) const
{
    const TWall& wall = m_walls[ wallIndex ];
    const CTPlane< T >& plane = wall.GetPlane();
    if ( m_oneSided && ( plane.GetSignedDistance( source ) <= m_pointTolerance || plane.GetSignedDistance( listener ) <= m_pointTolerance ) )
        return false;

    const TPoint image = plane.Mirror( source );
    TPoint p;
    if ( !CrossPlane( plane, listener, image, false, p ) || !wall.ContainsPoint( p, m_pointTolerance ) )
        return false;
    if ( !Open( listener, p, false, true ) || !Open( p, source, true, false ) )
        return false;

    path.order = 1;
    path.wallIndex[ 0 ] = wallIndex;
    path.wallUserId[ 0 ] = wall.GetUserId();
    path.reflectionPoint[ 0 ] = p;
    path.position = image;
    path.pathLength = ( image - listener ).GetLength();
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTImageSourceModel< T >::TrySecondOrder( const TPoint& source, const TPoint& listener, UInt32 first, UInt32 last, TPath& path ) const
{
    const TWall& wallA = m_walls[ first ];
    const TWall& wallB = m_walls[ last ];
    const TPoint image1 = wallA.GetPlane().Mirror( source );
    const TPoint image2 = wallB.GetPlane().Mirror( image1 );

    // from the listener the last wall is met first
    TPoint pb;
    if ( !CrossPlane( wallB.GetPlane(), listener, image2, false, pb ) || !wallB.ContainsPoint( pb, m_pointTolerance ) )
        return false;
    TPoint pa;
    if ( !CrossPlane( wallA.GetPlane(), pb, image1, true, pa ) || !wallA.ContainsPoint( pa, m_pointTolerance ) )
        return false;

    if ( m_oneSided )
    {
        // each reflection arrives at the front: the listener side of B, the side of A that the point on B and the source are on
        if ( wallB.GetPlane().GetSignedDistance( listener ) <= m_pointTolerance || wallA.GetPlane().GetSignedDistance( pb ) <= m_pointTolerance ||
             wallA.GetPlane().GetSignedDistance( source ) <= m_pointTolerance || wallB.GetPlane().GetSignedDistance( pa ) <= m_pointTolerance )
            return false;
    }

    if ( !Open( listener, pb, false, true ) || !Open( pb, pa, true, true ) || !Open( pa, source, true, false ) )
        return false;

    path.order = 2;
    path.wallIndex[ 0 ] = first;
    path.wallIndex[ 1 ] = last;
    path.wallUserId[ 0 ] = wallA.GetUserId();
    path.wallUserId[ 1 ] = wallB.GetUserId();
    path.reflectionPoint[ 0 ] = pa;
    path.reflectionPoint[ 1 ] = pb;
    path.position = image2;
    path.pathLength = ( image2 - listener ).GetLength();
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTImageSourceModel< T >::Before( const TPath& a, const TPath& b )
{
    if ( a.pathLength < b.pathLength )
        return true;
    if ( b.pathLength < a.pathLength )
        return false;
    if ( a.order != b.order )
        return a.order < b.order;
    if ( a.wallIndex[ 0 ] != b.wallIndex[ 0 ] )
        return a.wallIndex[ 0 ] < b.wallIndex[ 0 ];
    return a.wallIndex[ 1 ] < b.wallIndex[ 1 ];
}

/*-------------------------------------------------------------------------*/

template < typename T >
UInt32
CTImageSourceModel< T >::FindPaths( const TPoint& source, const TPoint& listener, GUCEF::vector< TPath >& paths ) const
{
    paths.clear();
    const UInt32 wallCount = (UInt32) m_walls.size();

    if ( m_includeDirect && Open( listener, source, false, false ) )
    {
        TPath direct;
        direct.order = 0;
        direct.position = source;
        direct.pathLength = ( source - listener ).GetLength();
        paths.push_back( direct );
    }

    for ( UInt32 a=0; a<wallCount; ++a )
    {
        TPath path;
        if ( TryFirstOrder( source, listener, a, path ) )
            paths.push_back( path );
    }
    if ( m_maxOrder >= 2 )
    {
        for ( UInt32 a=0; a<wallCount; ++a )
        {
            for ( UInt32 b=0; b<wallCount; ++b )
            {
                TPath path;
                if ( a != b && TrySecondOrder( source, listener, a, b, path ) )
                    paths.push_back( path );
            }
        }
    }

    // two orders of perpendicular walls give one image; when a path passes the edge of the two walls within the tolerance both
    // orders are accepted: the first of them stays
    const T same = m_pointTolerance * TTraits::FromInt32( 4 );
    for ( size_t i=0; i<paths.size(); ++i )
    {
        if ( 2 != paths[ i ].order )
            continue;
        for ( size_t j=0; j<i; ++j )
        {
            if ( 2 == paths[ j ].order && ( paths[ i ].position - paths[ j ].position ).GetLength() <= same )
            {
                paths.erase( paths.begin() + i );
                --i;
                break;
            }
        }
    }

    // arrival directions, then a stable insertion sort by length (the lists are short)
    for ( size_t i=0; i<paths.size(); ++i )
    {
        TPath& path = paths[ i ];
        const TPoint& last = 0 == path.order ? source : path.reflectionPoint[ path.order - 1 ];
        TVector direction;
        if ( !( last - listener ).TryNormalize( direction, TTraits::Zero() ) )
            direction = TVector();
        path.arrivalDirection = direction;
    }
    for ( size_t i=1; i<paths.size(); ++i )
    {
        TPath key = paths[ i ];
        size_t j = i;
        while ( j > 0 && Before( key, paths[ j - 1 ] ) )
        {
            paths[ j ] = paths[ j - 1 ];
            --j;
        }
        paths[ j ] = key;
    }
    return (UInt32) paths.size();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTImageSourceWall< Float32 > CImageSourceWall;
typedef CTImageSourceModel< Float32 > CImageSourceModel;
typedef CTImageSourceModel< Float64 > CImageSourceModel64;
typedef SImageSourcePath< Float32 > TFloat32ImageSourcePath;
typedef SImageSourcePath< Float64 > TFloat64ImageSourcePath;

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTImageSourceWall< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTImageSourceWall< Float64 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( SImageSourcePath< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( SImageSourcePath< Float64 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTImageSourceModel< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTImageSourceModel< Float64 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTIMAGESOURCEMODEL_H ? */
