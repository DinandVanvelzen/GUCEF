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

#ifndef GUCEF_GEOMETRY_RAYQUERIES_H
#define GUCEF_GEOMETRY_RAYQUERIES_H

/*
 *  The ray cells of the query matrix (section 7.5.3 of the design): the ray cast facade over every shape that has a
 *  RayCast member (plane, sphere, AABB, OBB, triangle, capsule, cylinder, cone, frustum, plane-bounded volume, and the
 *  hull and the others of later packages), and the watertight ray triangle test. See gucefGEOMETRY_PointQueries.h for
 *  the conventions of the facade.
 *
 *  A ray that starts inside a solid hits at distance 0 with RAYHIT_STARTED_INSIDE and the normal opposite to the ray;
 *  a hit beyond the maximum distance is a miss.
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

#ifndef GUCEF_GEOMETRY_SRAYCASTRESULT_H
#include "gucefGEOMETRY_SRayCastResult.h"
#define GUCEF_GEOMETRY_SRAYCASTRESULT_H
#endif /* GUCEF_GEOMETRY_SRAYCASTRESULT_H ? */

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

/** The first hit of the ray with the shape within 'maxDistance' (the RayCast member of the shape) */
template < typename T, typename TShape >
GUCEF_GEOMETRY_FORCE_INLINE bool
RayCast( const CTRay3D< T >& ray, const TShape& shape, const T& maxDistance, SRayCastResult< T >& result ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return shape.RayCast( ray, maxDistance, result );
}

/** True when the ray hits the shape anywhere along its length */
template < typename T, typename TShape >
GUCEF_GEOMETRY_FORCE_INLINE bool
Intersects( const CTRay3D< T >& ray, const TShape& shape ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    SRayCastResult< T > hit;
    return shape.RayCast( ray, MATH::CTNumericTraits< T >::Max(), hit );
}

/** True when the ray hits the shape anywhere along its length (the other order of the arguments) */
template < typename TShape, typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool
Intersects( const TShape& shape, const CTRay3D< T >& ray ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    SRayCastResult< T > hit;
    return shape.RayCast( ray, MATH::CTNumericTraits< T >::Max(), hit );
}

/*-------------------------------------------------------------------------*/

/**
 *  Ray against a triangle, two-sided, watertight (Woop, Benthin and Wald 2013): the ray is sheared into a frame in which
 *  it runs along +Z, and the hit is decided by the three edge functions of the sheared vertices. Each edge function is
 *  computed from the two vertices of the edge only, as a difference of two products whose sign flips exactly when the
 *  edge is walked the other way, so a ray through a shared edge or vertex of two triangles gets opposite signs for
 *  them and at least one of the triangles reports a hit (an edge function of exactly zero counts as a hit for both).
 *  This holds only when the products are not fused: the functions of the module are compiled with floating-point
 *  contraction off, and callers that inline them must do the same.
 *
 *  A ray parallel to the plane of the triangle and a degenerate triangle miss. The result has the distance, the point, the
 *  normal facing the ray, and u and v, the barycentric weights of the second and third vertex.
 */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
RayCastTriangleWatertight( const CTRay3D< T >& ray, const CTTriangle3D< T >& triangle, const T& maxDistance, SRayCastResult< T >& result ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTVector3D< T > TVector;
    const T zero = TTraits::Zero();
    const TVector d = ray.direction.AsVector();

    /* the dimension in which the direction is largest becomes the shear axis, the others keep the winding of the triangle */
    UInt32 kz = 0;
    if ( TTraits::Abs( d.GetComponent( 0 ) ) < TTraits::Abs( d.GetComponent( 1 ) ) )
        kz = 1;
    if ( TTraits::Abs( d.GetComponent( kz ) ) < TTraits::Abs( d.GetComponent( 2 ) ) )
        kz = 2;
    UInt32 kx = ( kz + 1 ) % 3;
    UInt32 ky = ( kx + 1 ) % 3;
    if ( TTraits::IsNegative( d.GetComponent( kz ) ) )
    {
        const UInt32 swap = kx;
        kx = ky;
        ky = swap;
    }
    const T dz = d.GetComponent( kz );
    if ( !( zero < TTraits::Abs( dz ) ) )
        return false;
    const T sx = d.GetComponent( kx ) / dz;
    const T sy = d.GetComponent( ky ) / dz;
    const T sz = TTraits::One() / dz;

    /* the vertices relative to the origin of the ray, sheared */
    const TVector a = triangle.a - ray.origin;
    const TVector b = triangle.b - ray.origin;
    const TVector c = triangle.c - ray.origin;
    const T ax = a.GetComponent( kx ) - sx * a.GetComponent( kz );
    const T ay = a.GetComponent( ky ) - sy * a.GetComponent( kz );
    const T bx = b.GetComponent( kx ) - sx * b.GetComponent( kz );
    const T by = b.GetComponent( ky ) - sy * b.GetComponent( kz );
    const T cx = c.GetComponent( kx ) - sx * c.GetComponent( kz );
    const T cy = c.GetComponent( ky ) - sy * c.GetComponent( kz );

    const T u = cx * by - cy * bx;
    const T v = ax * cy - ay * cx;
    const T w = bx * ay - by * ax;
    if ( ( u < zero || v < zero || w < zero ) && ( zero < u || zero < v || zero < w ) )
        return false;
    const T determinant = ( u + v ) + w;
    if ( zero == determinant || !TTraits::IsFinite( determinant ) )
        return false;

    const T az = sz * a.GetComponent( kz );
    const T bz = sz * b.GetComponent( kz );
    const T cz = sz * c.GetComponent( kz );
    const T scaledT = ( u * az + v * bz ) + w * cz;
    if ( zero < determinant )
    {
        if ( scaledT < zero || maxDistance * determinant < scaledT )
            return false;
    }
    else
    {
        if ( zero < scaledT || scaledT < maxDistance * determinant )
            return false;
    }
    const T inverse = TTraits::One() / determinant;
    SRayCastResult< T > hit;
    hit.distance = scaledT * inverse;
    hit.point = ray.origin + d * hit.distance;
    hit.u = v * inverse;
    hit.v = w * inverse;
    MATH::CTDirectionVector3D< T > normal;
    if ( !triangle.TryGetNormal( normal ) )
        return false;
    hit.normal = TTraits::Zero() < normal.Dot( d ) ? -normal.AsVector() : normal.AsVector();
    result = hit;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/* For Float32 and Float64 the module's instantiations (gucefGEOMETRY_QueryInstantiations.cpp) are used, section 5.10 */

#define GUCEF_GEOMETRY_RAYQUERIES_KIND( prefix, kind ) \
    prefix bool RayCastTriangleWatertight< kind >( const CTRay3D< kind >&, const CTTriangle3D< kind >&, const kind&, SRayCastResult< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

#define GUCEF_GEOMETRY_RAYQUERIES_PREDEFINED_KINDS( prefix ) \
    GUCEF_GEOMETRY_RAYQUERIES_KIND( prefix, Float32 ) \
    GUCEF_GEOMETRY_RAYQUERIES_KIND( prefix, Float64 )

#ifdef GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED
GUCEF_GEOMETRY_RAYQUERIES_PREDEFINED_KINDS( GUCEF_GEOMETRY_EXTERN_TEMPLATE )
#endif /* GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_RAYQUERIES_H ? */
