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

#ifndef GUCEF_GEOMETRY_SEGMENTQUERIES_H
#define GUCEF_GEOMETRY_SEGMENTQUERIES_H

/*
 *  The segment cells of the query matrix (section 7.5.1 of the design): a segment against a segment, plane, triangle,
 *  sphere, capsule, axis aligned box and oriented box, all analytic, plus the building block for shapes with a
 *  radius. See gucefGEOMETRY_PointQueries.h for the conventions of the facade.
 *
 *  - Tolerance: the tests that decide by a distance (segment-segment, segment-triangle) count shapes that are within
 *    the length tolerance as touching, so that configurations that touch up to rounding are reported as touching.
 *    The default tolerance of the kind is used unless one is given.
 *  - Degenerate input: a zero-length segment is a point; a degenerate triangle is treated as its edges.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_POINTQUERIES_H
#include "gucefGEOMETRY_PointQueries.h"
#define GUCEF_GEOMETRY_POINTQUERIES_H
#endif /* GUCEF_GEOMETRY_POINTQUERIES_H ? */

#ifndef GUCEF_GEOMETRY_SHAPES_H
#include "gucefGEOMETRY_shapes.h"
#define GUCEF_GEOMETRY_SHAPES_H
#endif /* GUCEF_GEOMETRY_SHAPES_H ? */

#ifndef GUCEF_GEOMETRY_CTORIENTEDBOX3D_H
#include "gucefGEOMETRY_CTOrientedBox3D.h"
#define GUCEF_GEOMETRY_CTORIENTEDBOX3D_H
#endif /* GUCEF_GEOMETRY_CTORIENTEDBOX3D_H ? */

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

/**
 *  The nearest points of two shapes that are a core (a point) inflated by a radius: spheres (the centre), capsules
 *  (the nearest point of the core segments), and with radius 0 any shape whose nearest point to the other is given
 *  (a box, a triangle, a segment). 'coreA' and 'coreB' are the nearest points of the cores. The result is the distance
 *  between the shapes, 0 when they touch or overlap; then both outputs are the same point that lies in both shapes (the
 *  core of a shape of radius 0, else the middle of the overlap along the line of the cores).
 *  Radii must not be negative.
 */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
ClosestPointsOfRoundCores( const MATH::CTPoint3D< T >& coreA, const T& radiusA, const MATH::CTPoint3D< T >& coreB, const T& radiusB,
                           MATH::CTPoint3D< T >& onA, MATH::CTPoint3D< T >& onB ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTVector3D< T > TVector;
    const TVector between = coreB - coreA;
    const T distance = between.GetLength();
    const T gap = distance - ( radiusA + radiusB );
    if ( TTraits::Zero() < gap )
    {
        const TVector direction = between / distance;
        onA = coreA + direction * radiusA;
        onB = coreB - direction * radiusB;
        return gap;
    }
    MATH::CTPoint3D< T > witness = coreA;
    if ( !( TTraits::Zero() < radiusB ) )
        witness = coreB;
    else if ( TTraits::Zero() < radiusA && TTraits::Zero() < distance )
    {
        /* along the line of the cores the first shape covers [ -rA, rA ] and the second [ d - rB, d + rB ]: the middle of the overlap */
        const T low = TTraits::Max( MATH::ComponentNegate( radiusA ), distance - radiusB );
        const T high = TTraits::Min( radiusA, distance + radiusB );
        witness = coreA + ( between / distance ) * ( ( low + high ) * TTraits::Half() );
    }
    onA = witness;
    onB = witness;
    return TTraits::Zero();
}

/*-------------------------------------------------------------------------*/

/** The nearest points of two segments (Ericson 5.1.9), also for zero-length and parallel segments; 'true' always */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
ClosestPoints( const CTSegment3D< T >& a, const CTSegment3D< T >& b, MATH::CTPoint3D< T >& onA, MATH::CTPoint3D< T >& onB ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTVector3D< T > TVector;
    const T zero = TTraits::Zero();
    const T one = TTraits::One();
    const TVector d1 = a.end - a.start;
    const TVector d2 = b.end - b.start;
    const TVector r = a.start - b.start;
    const T aa = d1.Dot( d1 );
    const T e = d2.Dot( d2 );
    const T f = d2.Dot( r );
    T s = zero;
    T t = zero;
    if ( !( zero < aa ) && !( zero < e ) )
    {
        /* both are points */
    }
    else if ( !( zero < aa ) )
    {
        t = f / e;
        t = t < zero ? zero : ( one < t ? one : t );
    }
    else
    {
        const T c = d1.Dot( r );
        if ( !( zero < e ) )
        {
            s = MATH::ComponentNegate( c ) / aa;
            s = s < zero ? zero : ( one < s ? one : s );
        }
        else
        {
            const T bb = d1.Dot( d2 );
            const T denominator = aa * e - bb * bb;
            if ( zero < denominator )
            {
                s = ( bb * f - c * e ) / denominator;
                s = s < zero ? zero : ( one < s ? one : s );
            }
            t = ( bb * s + f ) / e;
            if ( t < zero )
            {
                t = zero;
                s = MATH::ComponentNegate( c ) / aa;
                s = s < zero ? zero : ( one < s ? one : s );
            }
            else if ( one < t )
            {
                t = one;
                s = ( bb - c ) / aa;
                s = s < zero ? zero : ( one < s ? one : s );
            }
        }
    }
    onA = a.start + d1 * s;
    onB = b.start + d2 * t;
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  The parameter t of the point where the segment meets the plane ( start + t * ( end - start ) ), the first one for a
 *  segment that lies in the plane (t = 0). False when the segment is strictly on one side.
 */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
IntersectSegmentPlane( const CTSegment3D< T >& segment, const CTPlane< T >& plane, T& t ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    const T da = plane.GetSignedDistance( segment.start );
    const T db = plane.GetSignedDistance( segment.end );
    if ( ( da < TTraits::Zero() && TTraits::Zero() < db ) || ( db < TTraits::Zero() && TTraits::Zero() < da ) )
    {
        t = da / ( da - db );
        return true;
    }
    if ( TTraits::Zero() == da )
    {
        t = TTraits::Zero();
        return true;
    }
    if ( TTraits::Zero() == db )
    {
        t = TTraits::One();
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/** The nearest points of a segment and a plane: the crossing point (on both) or the end point nearer to the plane and its projection */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
ClosestPoints( const CTSegment3D< T >& segment, const CTPlane< T >& plane, MATH::CTPoint3D< T >& onSegment, MATH::CTPoint3D< T >& onPlane ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    T t = TTraits::Zero();
    if ( IntersectSegmentPlane( segment, plane, t ) )
    {
        onSegment = segment.GetPoint( t );
        onPlane = onSegment;
        return true;
    }
    const bool startNearer = TTraits::Abs( plane.GetSignedDistance( segment.start ) ) < TTraits::Abs( plane.GetSignedDistance( segment.end ) );
    onSegment = startNearer ? segment.start : segment.end;
    onPlane = plane.Project( onSegment );
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  The nearest points of a segment and a (solid, flat) triangle: where the segment crosses the triangle (on both), else
 *  the nearest of the end points against the triangle and the segment against the three edges (Ericson 5.1.5, 5.1.9).
 */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
ClosestPoints( const CTSegment3D< T >& segment, const CTTriangle3D< T >& triangle, MATH::CTPoint3D< T >& onSegment, MATH::CTPoint3D< T >& onTriangle ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    const T zero = TTraits::Zero();

    if ( !triangle.IsDegenerate() )
    {
        const TVector n = triangle.GetNormalVector();
        const T da = ( segment.start - triangle.a ).Dot( n );
        const T db = ( segment.end - triangle.a ).Dot( n );
        if ( ( da <= zero && zero <= db ) || ( db <= zero && zero <= da ) )
        {
            if ( da != db )
            {
                const TPoint p = segment.GetPoint( da / ( da - db ) );
                if ( !TTraits::IsNegative( n.Dot( ( triangle.b - triangle.a ).Cross( p - triangle.a ) ) ) &&
                     !TTraits::IsNegative( n.Dot( ( triangle.c - triangle.b ).Cross( p - triangle.b ) ) ) &&
                     !TTraits::IsNegative( n.Dot( ( triangle.a - triangle.c ).Cross( p - triangle.c ) ) ) )
                {
                    onSegment = p;
                    onTriangle = p;
                    return true;
                }
            }
        }
    }

    TPoint bestSegment = segment.start;
    TPoint bestTriangle = triangle.ClosestPoint( segment.start );
    T best = ( bestTriangle - bestSegment ).GetSquaredLength();

    const TPoint endClosest = triangle.ClosestPoint( segment.end );
    T candidate = ( endClosest - segment.end ).GetSquaredLength();
    if ( candidate < best )
    {
        best = candidate;
        bestSegment = segment.end;
        bestTriangle = endClosest;
    }
    for ( UInt32 i=0; i<3; ++i )
    {
        TPoint pa;
        TPoint pb;
        ClosestPoints( segment, CTSegment3D< T >( triangle.GetVertex( i ), triangle.GetVertex( ( i + 1 ) % 3 ) ), pa, pb );
        candidate = ( pb - pa ).GetSquaredLength();
        if ( candidate < best )
        {
            best = candidate;
            bestSegment = pa;
            bestTriangle = pb;
        }
    }
    onSegment = bestSegment;
    onTriangle = bestTriangle;
    return true;
}

/*-------------------------------------------------------------------------*/

/** The nearest points of a segment and a solid sphere; false for a negative radius */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
ClosestPoints( const CTSegment3D< T >& segment, const CTSphere< T >& sphere, MATH::CTPoint3D< T >& onSegment, MATH::CTPoint3D< T >& onSphere ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    if ( TTraits::IsNegative( sphere.radius ) )
        return false;
    ClosestPointsOfRoundCores( segment.ClosestPoint( sphere.centre ), TTraits::Zero(), sphere.centre, sphere.radius, onSegment, onSphere );
    return true;
}

/*-------------------------------------------------------------------------*/

/** The nearest points of a segment and a solid capsule; false for a negative radius */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
ClosestPoints( const CTSegment3D< T >& segment, const CTCapsule< T >& capsule, MATH::CTPoint3D< T >& onSegment, MATH::CTPoint3D< T >& onCapsule ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    if ( TTraits::IsNegative( capsule.radius ) )
        return false;
    MATH::CTPoint3D< T > pa;
    MATH::CTPoint3D< T > pb;
    ClosestPoints( segment, capsule.GetSegment(), pa, pb );
    ClosestPointsOfRoundCores( pa, TTraits::Zero(), pb, capsule.radius, onSegment, onCapsule );
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  The parameter interval [ tEnter, tExit ] of the segment ( start + t * ( end - start ), t in [ 0, 1 ] ) inside the
 *  box (slab test); false when the segment misses the box or the box is empty. A segment parallel to a face on the
 *  boundary of the box is inside; a zero length segment is a point.
 */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
ClipSegmentToBox( const CTSegment3D< T >& segment, const CTAxisAlignedBox3D< T >& box, T& tEnter, T& tExit ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    if ( box.IsEmpty() )
        return false;
    T enter = TTraits::Zero();
    T leave = TTraits::One();
    for ( UInt32 axis=0; axis<3; ++axis )
    {
        const T o = segment.start.GetComponent( axis );
        const T d = segment.end.GetComponent( axis ) - o;
        const T lo = box.min.GetComponent( axis );
        const T hi = box.max.GetComponent( axis );
        if ( TTraits::Zero() == d )
        {
            if ( o < lo || hi < o )
                return false;
            continue;
        }
        T t1 = ( lo - o ) / d;
        T t2 = ( hi - o ) / d;
        if ( t2 < t1 )
        {
            const T swap = t1;
            t1 = t2;
            t2 = swap;
        }
        if ( enter < t1 )
            enter = t1;
        if ( t2 < leave )
            leave = t2;
        if ( leave < enter )
            return false;
    }
    if ( !( enter <= leave ) )
        return false;
    tEnter = enter;
    tExit = leave;
    return true;
}

/*-------------------------------------------------------------------------*/

/** True when the segments are within the tolerance of each other (touching or crossing counts) */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Intersects( const CTSegment3D< T >& a, const CTSegment3D< T >& b, const CTGeometryTolerance< T >& tolerance ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    MATH::CTPoint3D< T > pa;
    MATH::CTPoint3D< T > pb;
    ClosestPoints( a, b, pa, pb );
    const T limit = tolerance.GetLengthTolerance( MATH::CTNumericTraits< T >::Max( GetCoordinateScale( pa ), GetCoordinateScale( pb ) ) );
    return ( pb - pa ).GetSquaredLength() <= limit * limit;
}

template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool
Intersects( const CTSegment3D< T >& a, const CTSegment3D< T >& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return Intersects( a, b, CTGeometryTolerance< T >() );
}

/*-------------------------------------------------------------------------*/

/** True when the segment touches or crosses the plane */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Intersects( const CTSegment3D< T >& segment, const CTPlane< T >& plane ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    T t = MATH::CTNumericTraits< T >::Zero();
    return IntersectSegmentPlane( segment, plane, t );
}

/*-------------------------------------------------------------------------*/

/** True when the segment is within the tolerance of the triangle (touching or crossing counts) */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Intersects( const CTSegment3D< T >& segment, const CTTriangle3D< T >& triangle, const CTGeometryTolerance< T >& tolerance ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    MATH::CTPoint3D< T > ps;
    MATH::CTPoint3D< T > pt;
    ClosestPoints( segment, triangle, ps, pt );
    const T limit = tolerance.GetLengthTolerance( MATH::CTNumericTraits< T >::Max( GetCoordinateScale( ps ), GetCoordinateScale( pt ) ) );
    return ( pt - ps ).GetSquaredLength() <= limit * limit;
}

template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool
Intersects( const CTSegment3D< T >& segment, const CTTriangle3D< T >& triangle ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return Intersects( segment, triangle, CTGeometryTolerance< T >() );
}

/*-------------------------------------------------------------------------*/

/** True when the segment shares a point with the solid sphere */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Intersects( const CTSegment3D< T >& segment, const CTSphere< T >& sphere ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( MATH::CTNumericTraits< T >::IsNegative( sphere.radius ) )
        return false;
    return segment.GetSquaredDistance( sphere.centre ) <= sphere.radius * sphere.radius;
}

/*-------------------------------------------------------------------------*/

/** True when the segment shares a point with the solid capsule */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Intersects( const CTSegment3D< T >& segment, const CTCapsule< T >& capsule ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( MATH::CTNumericTraits< T >::IsNegative( capsule.radius ) )
        return false;
    MATH::CTPoint3D< T > pa;
    MATH::CTPoint3D< T > pb;
    ClosestPoints( segment, capsule.GetSegment(), pa, pb );
    return ( pb - pa ).GetSquaredLength() <= capsule.radius * capsule.radius;
}

/*-------------------------------------------------------------------------*/

/** True when the segment shares a point with the box (slab test); false for an empty box */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Intersects( const CTSegment3D< T >& segment, const CTAxisAlignedBox3D< T >& box ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    T enter = MATH::CTNumericTraits< T >::Zero();
    T leave = enter;
    return ClipSegmentToBox( segment, box, enter, leave );
}

/*-------------------------------------------------------------------------*/

/** True when the segment shares a point with the oriented box: the segment in the frame of the box against the box (slab test); false for an invalid box */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Intersects( const CTSegment3D< T >& segment, const CTOrientedBox3D< T >& box ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    if ( !box.IsValid() )
        return false;
    const CTSegment3D< T > local( MATH::CTPoint3D< T >( box.ToLocal( segment.start ) ), MATH::CTPoint3D< T >( box.ToLocal( segment.end ) ) );
    const MATH::CTVector3D< T >& e = box.halfExtents;
    T enter = TTraits::Zero();
    T leave = enter;
    return ClipSegmentToBox( local, CTAxisAlignedBox3D< T >( MATH::CTPoint3D< T >( MATH::ComponentNegate( e.x ), MATH::ComponentNegate( e.y ), MATH::ComponentNegate( e.z ) ),
                                                              MATH::CTPoint3D< T >( e.x, e.y, e.z ) ), enter, leave );
}

/*-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_SWAPPED_INTERSECTS( CTSegment3D, CTPlane )
GUCEF_GEOMETRY_SWAPPED_INTERSECTS( CTSegment3D, CTTriangle3D )
GUCEF_GEOMETRY_SWAPPED_INTERSECTS( CTSegment3D, CTSphere )
GUCEF_GEOMETRY_SWAPPED_INTERSECTS( CTSegment3D, CTCapsule )
GUCEF_GEOMETRY_SWAPPED_INTERSECTS( CTSegment3D, CTAxisAlignedBox3D )
GUCEF_GEOMETRY_SWAPPED_INTERSECTS( CTSegment3D, CTOrientedBox3D )
GUCEF_GEOMETRY_SWAPPED_CLOSESTPOINTS( CTSegment3D, CTPlane )
GUCEF_GEOMETRY_SWAPPED_CLOSESTPOINTS( CTSegment3D, CTTriangle3D )
GUCEF_GEOMETRY_SWAPPED_CLOSESTPOINTS( CTSegment3D, CTSphere )
GUCEF_GEOMETRY_SWAPPED_CLOSESTPOINTS( CTSegment3D, CTCapsule )

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/* For Float32 and Float64 the module's instantiations (gucefGEOMETRY_QueryInstantiations.cpp) are used, section 5.10 */

#define GUCEF_GEOMETRY_SEGMENTQUERIES_KIND( prefix, kind ) \
    prefix kind ClosestPointsOfRoundCores< kind >( const MATH::CTPoint3D< kind >&, const kind&, const MATH::CTPoint3D< kind >&, const kind&, MATH::CTPoint3D< kind >&, MATH::CTPoint3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool ClosestPoints< kind >( const CTSegment3D< kind >&, const CTSegment3D< kind >&, MATH::CTPoint3D< kind >&, MATH::CTPoint3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool IntersectSegmentPlane< kind >( const CTSegment3D< kind >&, const CTPlane< kind >&, kind& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool ClosestPoints< kind >( const CTSegment3D< kind >&, const CTPlane< kind >&, MATH::CTPoint3D< kind >&, MATH::CTPoint3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool ClosestPoints< kind >( const CTSegment3D< kind >&, const CTTriangle3D< kind >&, MATH::CTPoint3D< kind >&, MATH::CTPoint3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool ClosestPoints< kind >( const CTSegment3D< kind >&, const CTSphere< kind >&, MATH::CTPoint3D< kind >&, MATH::CTPoint3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool ClosestPoints< kind >( const CTSegment3D< kind >&, const CTCapsule< kind >&, MATH::CTPoint3D< kind >&, MATH::CTPoint3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool ClipSegmentToBox< kind >( const CTSegment3D< kind >&, const CTAxisAlignedBox3D< kind >&, kind&, kind& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Intersects< kind >( const CTSegment3D< kind >&, const CTSegment3D< kind >&, const CTGeometryTolerance< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Intersects< kind >( const CTSegment3D< kind >&, const CTPlane< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Intersects< kind >( const CTSegment3D< kind >&, const CTTriangle3D< kind >&, const CTGeometryTolerance< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Intersects< kind >( const CTSegment3D< kind >&, const CTSphere< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Intersects< kind >( const CTSegment3D< kind >&, const CTCapsule< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Intersects< kind >( const CTSegment3D< kind >&, const CTAxisAlignedBox3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Intersects< kind >( const CTSegment3D< kind >&, const CTOrientedBox3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

#define GUCEF_GEOMETRY_SEGMENTQUERIES_PREDEFINED_KINDS( prefix ) \
    GUCEF_GEOMETRY_SEGMENTQUERIES_KIND( prefix, Float32 ) \
    GUCEF_GEOMETRY_SEGMENTQUERIES_KIND( prefix, Float64 )

#ifdef GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED
GUCEF_GEOMETRY_SEGMENTQUERIES_PREDEFINED_KINDS( GUCEF_GEOMETRY_EXTERN_TEMPLATE )
#endif /* GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_SEGMENTQUERIES_H ? */
