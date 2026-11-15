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

#ifndef GUCEF_GEOMETRY_SWEEPQUERIES_H
#define GUCEF_GEOMETRY_SWEEPQUERIES_H

/*
 *  Sweeps (shape casts) of section 7.5.4 of the design:
 *
 *  - bool SphereCast( ray, radius, shape, maxDistance, result ): a sphere of 'radius' whose centre moves along the ray
 *    by at most maxDistance; analytic (the ray against the shape grown by the radius, a Minkowski sum) for a plane, a
 *    sphere, a capsule, an AABB (the rounded box: three slabs and twelve edge capsules) and a triangle (the two caps of
 *    the slab and three edge capsules); the GJK ray cast for every other bounded convex shape.
 *  - bool ShapeCast( a, direction, maxDistance, b, result ): a bounded convex shape A moved along the unit direction by
 *    at most maxDistance against a bounded convex shape B, by the GJK ray cast on the Minkowski difference
 *    (conservative and exact for linear motion).
 *
 *  The result (SRayCastResult) holds the distance travelled, the contact point on the target shape, the unit normal
 *  of the target at the contact facing the moving shape, and RAYHIT_STARTED_INSIDE when the shapes overlap at the
 *  start (distance 0, the normal opposite to the motion; the point is the start of the ray for the analytic sphere
 *  casts and a point common to both shapes for the GJK casts). A sphere with a negative radius and shapes without
 *  points are never hit. Accuracy: the analytic casts are exact up to rounding; the GJK cast stops within the length
 *  tolerance of the surface, refined on the gap of the cores for shapes with margins; for shapes without margins
 *  Float32 and fixed point may stop slightly early where their rounding stalls the cast (never after the contact).
 *  Float32, Float64 and TFixed32_32 are exported instantiations; every query is realtime-safe and allocation-free.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_CONVEXQUERIES_H
#include "gucefGEOMETRY_ConvexQueries.h"
#define GUCEF_GEOMETRY_CONVEXQUERIES_H
#endif /* GUCEF_GEOMETRY_CONVEXQUERIES_H ? */

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

/** The result of a cast that starts in contact: distance 0, normal opposite to the motion, the start point */
template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool
SetCastStartedInside( const CTRay3D< T >& ray, SRayCastResult< T >& result ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    SRayCastResult< T > hit;
    hit.distance = MATH::CTNumericTraits< T >::Zero();
    hit.point = ray.origin;
    hit.normal = -ray.direction.AsVector();
    hit.flags = RAYHIT_STARTED_INSIDE;
    result = hit;
    return true;
}

/*-------------------------------------------------------------------------*/

/** Keeps the nearer of two hits in 'best'; 'found' tells whether 'best' holds one */
template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE void
KeepNearerHit( const SRayCastResult< T >& candidate, bool& found, SRayCastResult< T >& best ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( !found || candidate.distance < best.distance )
    {
        best = candidate;
        found = true;
    }
}

/*-------------------------------------------------------------------------*/

/** Turns the hit of the sphere centre with the grown shape into the cast result: the contact point on the shape */
template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool
FinishSphereCast( const T& radius, SRayCastResult< T >& hit, SRayCastResult< T >& result ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    hit.point = hit.point - hit.normal * radius;
    hit.flags = RAYHIT_NONE;
    result = hit;
    return true;
}

/*-------------------------------------------------------------------------*/

/** A bounded convex shape A moved along 'direction' by at most maxDistance against B (GJK ray cast) */
template < typename TA, typename TB >
GUCEF_GEOMETRY_PRECISE_INLINE bool
ShapeCast( const TA& a, const MATH::CTDirectionVector3D< typename TA::TValue >& direction, const typename TA::TValue& maxDistance, const TB& b,
           SRayCastResult< typename TA::TValue >& result ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef typename TA::TValue T;
    typedef MATH::CTNumericTraits< T > TTraits;
    if ( !IsQueryableConvex( a ) || !IsQueryableConvex( b ) || TTraits::IsNegative( maxDistance ) )
        return false;
    const typename CTSupportMapping< TA >::TSupport sa = CTSupportMapping< TA >::Make( a );
    SGjkCastResult< T > cast;
    CTGjk< T >::Cast( sa, direction.AsVector() * maxDistance, TTraits::One(), CTSupportMapping< TB >::Make( b ), CTGeometryTolerance< T >(), cast );
    if ( !cast.hit )
        return false;
    SRayCastResult< T > hit;
    hit.distance = cast.fraction * maxDistance;
    hit.point = cast.pointOnB;
    if ( TTraits::Zero() == cast.fraction || cast.normal.IsZero() )
    {
        hit.distance = TTraits::Zero();
        hit.normal = -direction.AsVector();
        hit.flags = RAYHIT_STARTED_INSIDE;
    }
    else
    {
        hit.normal = cast.normal;
    }
    result = hit;
    return true;
}

/*-------------------------------------------------------------------------*/

/** Sphere cast against any bounded convex shape: the GJK ray cast of the sphere (a point core with the radius as margin) */
template < typename T, typename TShape >
GUCEF_GEOMETRY_PRECISE_INLINE bool
SphereCast( const CTRay3D< T >& ray, const T& radius, const TShape& shape, const T& maxDistance, SRayCastResult< T >& result ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    if ( TTraits::IsNegative( radius ) )
        return false;
    return ShapeCast( CTSphere< T >( ray.origin, radius ), ray.direction, maxDistance, shape, result );
}

/*-------------------------------------------------------------------------*/

/** Sphere cast against a plane (the plane surface, both sides): the ray against the plane moved by the radius towards the sphere */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
SphereCast( const CTRay3D< T >& ray, const T& radius, const CTPlane< T >& plane, const T& maxDistance, SRayCastResult< T >& result ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    if ( TTraits::IsNegative( radius ) || TTraits::IsNegative( maxDistance ) )
        return false;
    const T start = plane.GetSignedDistance( ray.origin );
    if ( !( radius < TTraits::Abs( start ) ) )
        return SetCastStartedInside( ray, result );
    const bool front = TTraits::Zero() < start;
    const T approach = front ? MATH::ComponentNegate( plane.normal.Dot( ray.direction.AsVector() ) ) : plane.normal.Dot( ray.direction.AsVector() );
    if ( !( TTraits::Zero() < approach ) )
        return false;
    const T distance = ( TTraits::Abs( start ) - radius ) / approach;
    if ( maxDistance < distance )
        return false;
    SRayCastResult< T > hit;
    hit.distance = distance;
    hit.normal = front ? plane.normal.AsVector() : -plane.normal.AsVector();
    hit.point = ray.GetPoint( distance ) - hit.normal * radius;
    result = hit;
    return true;
}

/*-------------------------------------------------------------------------*/

/** Sphere cast against a sphere: the ray against the sphere grown by the radius */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
SphereCast( const CTRay3D< T >& ray, const T& radius, const CTSphere< T >& sphere, const T& maxDistance, SRayCastResult< T >& result ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    if ( TTraits::IsNegative( radius ) || TTraits::IsNegative( sphere.radius ) )
        return false;
    if ( !( sphere.radius + radius < sphere.centre.GetDistance( ray.origin ) ) )
        return SetCastStartedInside( ray, result );
    SRayCastResult< T > hit;
    if ( !CTSphere< T >( sphere.centre, sphere.radius + radius ).RayCast( ray, maxDistance, hit ) )
        return false;
    return FinishSphereCast( radius, hit, result );
}

/*-------------------------------------------------------------------------*/

/** Sphere cast against a capsule: the ray against the capsule grown by the radius */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
SphereCast( const CTRay3D< T >& ray, const T& radius, const CTCapsule< T >& capsule, const T& maxDistance, SRayCastResult< T >& result ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    if ( TTraits::IsNegative( radius ) || TTraits::IsNegative( capsule.radius ) )
        return false;
    const CTCapsule< T > grown( capsule.start, capsule.end, capsule.radius + radius );
    if ( grown.Contains( ray.origin ) )
        return SetCastStartedInside( ray, result );
    SRayCastResult< T > hit;
    if ( !grown.RayCast( ray, maxDistance, hit ) )
        return false;
    return FinishSphereCast( radius, hit, result );
}

/*-------------------------------------------------------------------------*/

/** Sphere cast against an axis aligned box: the ray against the rounded box (three slabs and twelve edge capsules) */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
SphereCast( const CTRay3D< T >& ray, const T& radius, const CTAxisAlignedBox3D< T >& box, const T& maxDistance, SRayCastResult< T >& result ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    if ( TTraits::IsNegative( radius ) || box.IsEmpty() || TTraits::IsNegative( maxDistance ) )
        return false;
    if ( !( radius < box.GetDistance( ray.origin ) ) )
        return SetCastStartedInside( ray, result );
    bool found = false;
    SRayCastResult< T > best;
    const T zero = TTraits::Zero();
    const TVector grow[ 3 ] = { TVector( radius, zero, zero ), TVector( zero, radius, zero ), TVector( zero, zero, radius ) };
    for ( UInt32 axis=0; axis<3; ++axis )
    {
        SRayCastResult< T > hit;
        if ( CTAxisAlignedBox3D< T >( box.min - grow[ axis ], box.max + grow[ axis ] ).RayCast( ray, maxDistance, hit ) )
            KeepNearerHit( hit, found, best );
    }
    for ( UInt32 corner=0; corner<8; ++corner )
    {
        const TPoint p( 0 != ( corner & 1 ) ? box.max.x : box.min.x, 0 != ( corner & 2 ) ? box.max.y : box.min.y, 0 != ( corner & 4 ) ? box.max.z : box.min.z );
        for ( UInt32 axis=0; axis<3; ++axis )
        {
            /* the edge from this corner in the + direction of the axis, once per edge */
            if ( 0 != ( corner & ( 1u << axis ) ) )
                continue;
            const TPoint q( 0 == axis ? box.max.x : p.x, 1 == axis ? box.max.y : p.y, 2 == axis ? box.max.z : p.z );
            SRayCastResult< T > hit;
            if ( CTCapsule< T >( p, q, radius ).RayCast( ray, maxDistance, hit ) )
                KeepNearerHit( hit, found, best );
        }
    }
    if ( !found )
        return false;
    return FinishSphereCast( radius, best, result );
}

/*-------------------------------------------------------------------------*/

/** Sphere cast against a triangle: the ray against the two caps of the slab around the triangle and three edge capsules */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
SphereCast( const CTRay3D< T >& ray, const T& radius, const CTTriangle3D< T >& triangle, const T& maxDistance, SRayCastResult< T >& result ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTVector3D< T > TVector;
    if ( TTraits::IsNegative( radius ) || TTraits::IsNegative( maxDistance ) )
        return false;
    if ( !( radius < triangle.GetDistance( ray.origin ) ) )
        return SetCastStartedInside( ray, result );
    bool found = false;
    SRayCastResult< T > best;
    MATH::CTDirectionVector3D< T > normal;
    if ( triangle.TryGetNormal( normal ) )
    {
        const TVector offset = normal * radius;
        SRayCastResult< T > hit;
        if ( CTTriangle3D< T >( triangle.a + offset, triangle.b + offset, triangle.c + offset ).RayCast( ray, maxDistance, hit ) )
            KeepNearerHit( hit, found, best );
        if ( CTTriangle3D< T >( triangle.a - offset, triangle.b - offset, triangle.c - offset ).RayCast( ray, maxDistance, hit ) )
            KeepNearerHit( hit, found, best );
    }
    const CTCapsule< T > edges[ 3 ] = { CTCapsule< T >( triangle.a, triangle.b, radius ), CTCapsule< T >( triangle.b, triangle.c, radius ), CTCapsule< T >( triangle.c, triangle.a, radius ) };
    for ( UInt32 e=0; e<3; ++e )
    {
        SRayCastResult< T > hit;
        if ( edges[ e ].RayCast( ray, maxDistance, hit ) )
            KeepNearerHit( hit, found, best );
    }
    if ( !found )
        return false;
    best.u = TTraits::Zero();
    best.v = TTraits::Zero();
    best.featureIndex = -1;
    return FinishSphereCast( radius, best, result );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/* For Float32, Float64 and TFixed32_32 the module's instantiations (gucefGEOMETRY_ConvexQueryInstantiations.cpp) are used */

#define GUCEF_GEOMETRY_SWEEPQUERIES_SPHERE( prefix, kind, TShapeClass ) \
    prefix bool SphereCast< kind >( const CTRay3D< kind >&, const kind&, const TShapeClass< kind >&, const kind&, SRayCastResult< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

#define GUCEF_GEOMETRY_SWEEPQUERIES_GJK_SPHERE( prefix, kind, TShapeClass ) \
    prefix bool SphereCast< kind, TShapeClass< kind > >( const CTRay3D< kind >&, const kind&, const TShapeClass< kind >&, const kind&, SRayCastResult< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

#define GUCEF_GEOMETRY_SWEEPQUERIES_SHAPECAST( prefix, kind, A, B ) \
    prefix bool ShapeCast< A< kind >, B< kind > >( const A< kind >&, const MATH::CTDirectionVector3D< kind >&, const kind&, const B< kind >&, SRayCastResult< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

#define GUCEF_GEOMETRY_SWEEPQUERIES_KIND( prefix, kind ) \
    GUCEF_GEOMETRY_SWEEPQUERIES_SPHERE( prefix, kind, CTPlane ) \
    GUCEF_GEOMETRY_SWEEPQUERIES_SPHERE( prefix, kind, CTSphere ) \
    GUCEF_GEOMETRY_SWEEPQUERIES_SPHERE( prefix, kind, CTCapsule ) \
    GUCEF_GEOMETRY_SWEEPQUERIES_SPHERE( prefix, kind, CTAxisAlignedBox3D ) \
    GUCEF_GEOMETRY_SWEEPQUERIES_SPHERE( prefix, kind, CTTriangle3D ) \
    GUCEF_GEOMETRY_SWEEPQUERIES_GJK_SPHERE( prefix, kind, CTOrientedBox3D ) \
    GUCEF_GEOMETRY_SWEEPQUERIES_GJK_SPHERE( prefix, kind, CTCylinder ) \
    GUCEF_GEOMETRY_SWEEPQUERIES_GJK_SPHERE( prefix, kind, CTCone ) \
    GUCEF_GEOMETRY_SWEEPQUERIES_GJK_SPHERE( prefix, kind, CTConvexHull3D ) \
    GUCEF_GEOMETRY_SWEEPQUERIES_SHAPECAST( prefix, kind, CTSphere, CTOrientedBox3D ) \
    GUCEF_GEOMETRY_SWEEPQUERIES_SHAPECAST( prefix, kind, CTCapsule, CTOrientedBox3D ) \
    GUCEF_GEOMETRY_SWEEPQUERIES_SHAPECAST( prefix, kind, CTOrientedBox3D, CTOrientedBox3D ) \
    GUCEF_GEOMETRY_SWEEPQUERIES_SHAPECAST( prefix, kind, CTAxisAlignedBox3D, CTAxisAlignedBox3D ) \
    GUCEF_GEOMETRY_SWEEPQUERIES_SHAPECAST( prefix, kind, CTOrientedBox3D, CTConvexHull3D ) \
    GUCEF_GEOMETRY_SWEEPQUERIES_SHAPECAST( prefix, kind, CTConvexHull3D, CTConvexHull3D )

#define GUCEF_GEOMETRY_SWEEPQUERIES_PREDEFINED_KINDS( prefix ) \
    GUCEF_GEOMETRY_SWEEPQUERIES_KIND( prefix, Float32 ) \
    GUCEF_GEOMETRY_SWEEPQUERIES_KIND( prefix, Float64 ) \
    GUCEF_GEOMETRY_SWEEPQUERIES_KIND( prefix, MATH::TFixed32_32 )

#ifdef GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED
GUCEF_GEOMETRY_SWEEPQUERIES_PREDEFINED_KINDS( GUCEF_GEOMETRY_EXTERN_TEMPLATE )
#endif /* GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_SWEEPQUERIES_H ? */
