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

#ifndef GUCEF_GEOMETRY_PLANEQUERIES_H
#define GUCEF_GEOMETRY_PLANEQUERIES_H

/*
 *  The plane cells of the query matrix: a plane against a triangle, sphere, capsule, axis aligned box, oriented box,
 *  cylinder, cone and frustum (the segment is in gucefGEOMETRY_SegmentQueries.h and the ray in the ray queries). All are
 *  analytic: a convex shape lies between the signed distances of its two support points along the plane normal, so
 *  the range of signed distances answers every question (side, overlap, distance, nearest points) without a case per
 *  shape. See gucefGEOMETRY_PointQueries.h for the conventions of the facade.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_SEGMENTQUERIES_H
#include "gucefGEOMETRY_SegmentQueries.h"
#define GUCEF_GEOMETRY_SEGMENTQUERIES_H
#endif /* GUCEF_GEOMETRY_SEGMENTQUERIES_H ? */

#ifndef GUCEF_GEOMETRY_CTCYLINDER_H
#include "gucefGEOMETRY_CTCylinder.h"
#define GUCEF_GEOMETRY_CTCYLINDER_H
#endif /* GUCEF_GEOMETRY_CTCYLINDER_H ? */

#ifndef GUCEF_GEOMETRY_CTCONE_H
#include "gucefGEOMETRY_CTCone.h"
#define GUCEF_GEOMETRY_CTCONE_H
#endif /* GUCEF_GEOMETRY_CTCONE_H ? */

#ifndef GUCEF_GEOMETRY_CTFRUSTUM_H
#include "gucefGEOMETRY_CTFrustum.h"
#define GUCEF_GEOMETRY_CTFRUSTUM_H
#endif /* GUCEF_GEOMETRY_CTFRUSTUM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The side of a plane that a shape lies on */
enum EPlaneSide
{
    PLANESIDE_BEHIND = 0 ,      /**< entirely behind the plane (negative signed distances), touching it not counted */
    PLANESIDE_STRADDLING ,      /**< the plane touches or cuts the shape */
    PLANESIDE_FRONT             /**< entirely in front of the plane (positive signed distances), touching it not counted */
};
typedef enum EPlaneSide TPlaneSide;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The range of signed distances to the plane over a convex shape that has GetSupport: the support points along the
 *  negative and positive normal. Building block of the plane cells; the overloads below are the public form.
 */
template < typename T, typename TShape >
GUCEF_GEOMETRY_PRECISE_INLINE void
PlaneConvexRange( const CTPlane< T >& plane, const TShape& shape, T& lowest, T& highest ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const MATH::CTVector3D< T > n = plane.normal.AsVector();
    lowest = plane.GetSignedDistance( shape.GetSupport( -n ) );
    highest = plane.GetSignedDistance( shape.GetSupport( n ) );
}

/*-------------------------------------------------------------------------*/

/**
 *  The nearest points of a plane and a convex shape that has GetSupport: when the plane cuts the shape both points are
 *  the point of the shape on the plane that lies on the segment between the two support points; otherwise the support
 *  point on the near side and its projection on the plane.
 */
template < typename T, typename TShape >
GUCEF_GEOMETRY_PRECISE_INLINE bool
PlaneConvexClosestPoints( const CTPlane< T >& plane, const TShape& shape, MATH::CTPoint3D< T >& onPlane, MATH::CTPoint3D< T >& onShape ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    const MATH::CTVector3D< T > n = plane.normal.AsVector();
    const MATH::CTPoint3D< T > pLow = shape.GetSupport( -n );
    const MATH::CTPoint3D< T > pHigh = shape.GetSupport( n );
    const T low = plane.GetSignedDistance( pLow );
    const T high = plane.GetSignedDistance( pHigh );
    if ( low <= TTraits::Zero() && TTraits::Zero() <= high )
    {
        const T span = high - low;
        onShape = TTraits::Zero() < span ? pLow + ( pHigh - pLow ) * ( MATH::ComponentNegate( low ) / span ) : pLow;
        onPlane = onShape;
        return true;
    }
    onShape = TTraits::Zero() < low ? pLow : pHigh;
    onPlane = plane.Project( onShape );
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  Defines the plane cells of one shape class: GetSignedDistanceRange, Classify, Intersects and ClosestPoints for
 *  ( plane, shape ) and Intersects and ClosestPoints for ( shape, plane ).
 */
#define GUCEF_GEOMETRY_PLANE_QUERIES( TShapeClass )                                                                       \
    /** The lowest and highest signed distance to the plane over the shape */                                             \
    template < typename T >                                                                                               \
    GUCEF_GEOMETRY_PRECISE_INLINE void                                                                                    \
    GetSignedDistanceRange( const CTPlane< T >& plane, const TShapeClass< T >& shape, T& lowest, T& highest ) GUCEF_GEOMETRY_REALTIME_CONTEXT \
    {                                                                                                                     \
        PlaneConvexRange( plane, shape, lowest, highest );                                                                \
    }                                                                                                                     \
    /** The side of the plane that the shape lies on */                                                                   \
    template < typename T >                                                                                               \
    GUCEF_GEOMETRY_FORCE_INLINE TPlaneSide                                                                                \
    Classify( const CTPlane< T >& plane, const TShapeClass< T >& shape ) GUCEF_GEOMETRY_REALTIME_CONTEXT                  \
    {                                                                                                                     \
        T lowest = MATH::CTNumericTraits< T >::Zero();                                                                    \
        T highest = lowest;                                                                                               \
        GetSignedDistanceRange( plane, shape, lowest, highest );                                                          \
        return MATH::CTNumericTraits< T >::Zero() < lowest ? PLANESIDE_FRONT : ( highest < MATH::CTNumericTraits< T >::Zero() ? PLANESIDE_BEHIND : PLANESIDE_STRADDLING ); \
    }                                                                                                                     \
    /** True when the plane touches or cuts the solid shape */                                                            \
    template < typename T >                                                                                               \
    GUCEF_GEOMETRY_FORCE_INLINE bool                                                                                      \
    Intersects( const CTPlane< T >& plane, const TShapeClass< T >& shape ) GUCEF_GEOMETRY_REALTIME_CONTEXT                \
    {                                                                                                                     \
        return PLANESIDE_STRADDLING == Classify( plane, shape );                                                          \
    }                                                                                                                     \
    /** The nearest points of the plane and the solid shape */                                                            \
    template < typename T >                                                                                               \
    GUCEF_GEOMETRY_PRECISE_INLINE bool                                                                                    \
    ClosestPoints( const CTPlane< T >& plane, const TShapeClass< T >& shape, MATH::CTPoint3D< T >& onPlane, MATH::CTPoint3D< T >& onShape ) GUCEF_GEOMETRY_REALTIME_CONTEXT \
    {                                                                                                                     \
        return PlaneConvexClosestPoints( plane, shape, onPlane, onShape );                                                \
    }                                                                                                                     \
    GUCEF_GEOMETRY_SWAPPED_INTERSECTS( CTPlane, TShapeClass )                                                             \
    GUCEF_GEOMETRY_SWAPPED_CLOSESTPOINTS( CTPlane, TShapeClass )

GUCEF_GEOMETRY_PLANE_QUERIES( CTTriangle3D )
GUCEF_GEOMETRY_PLANE_QUERIES( CTSphere )
GUCEF_GEOMETRY_PLANE_QUERIES( CTCapsule )
GUCEF_GEOMETRY_PLANE_QUERIES( CTAxisAlignedBox3D )
GUCEF_GEOMETRY_PLANE_QUERIES( CTOrientedBox3D )
GUCEF_GEOMETRY_PLANE_QUERIES( CTCylinder )
GUCEF_GEOMETRY_PLANE_QUERIES( CTCone )
GUCEF_GEOMETRY_PLANE_QUERIES( CTFrustum )

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/* For Float32 and Float64 the module's instantiations (gucefGEOMETRY_QueryInstantiations.cpp) are used, section 5.10 */

#define GUCEF_GEOMETRY_PLANEQUERIES_SHAPE( prefix, kind, TShapeClass ) \
    prefix void GetSignedDistanceRange< kind >( const CTPlane< kind >&, const TShapeClass< kind >&, kind&, kind& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool ClosestPoints< kind >( const CTPlane< kind >&, const TShapeClass< kind >&, MATH::CTPoint3D< kind >&, MATH::CTPoint3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

#define GUCEF_GEOMETRY_PLANEQUERIES_KIND( prefix, kind ) \
    GUCEF_GEOMETRY_PLANEQUERIES_SHAPE( prefix, kind, CTTriangle3D ) \
    GUCEF_GEOMETRY_PLANEQUERIES_SHAPE( prefix, kind, CTSphere ) \
    GUCEF_GEOMETRY_PLANEQUERIES_SHAPE( prefix, kind, CTCapsule ) \
    GUCEF_GEOMETRY_PLANEQUERIES_SHAPE( prefix, kind, CTAxisAlignedBox3D ) \
    GUCEF_GEOMETRY_PLANEQUERIES_SHAPE( prefix, kind, CTOrientedBox3D ) \
    GUCEF_GEOMETRY_PLANEQUERIES_SHAPE( prefix, kind, CTCylinder ) \
    GUCEF_GEOMETRY_PLANEQUERIES_SHAPE( prefix, kind, CTCone ) \
    GUCEF_GEOMETRY_PLANEQUERIES_SHAPE( prefix, kind, CTFrustum )

#define GUCEF_GEOMETRY_PLANEQUERIES_PREDEFINED_KINDS( prefix ) \
    GUCEF_GEOMETRY_PLANEQUERIES_KIND( prefix, Float32 ) \
    GUCEF_GEOMETRY_PLANEQUERIES_KIND( prefix, Float64 )

#ifdef GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED
GUCEF_GEOMETRY_PLANEQUERIES_PREDEFINED_KINDS( GUCEF_GEOMETRY_EXTERN_TEMPLATE )
#endif /* GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_PLANEQUERIES_H ? */
