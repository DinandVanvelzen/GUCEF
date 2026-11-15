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

#ifndef GUCEF_GEOMETRY_POINTQUERIES_H
#define GUCEF_GEOMETRY_POINTQUERIES_H

/*
 *  The query facade of the 3D shapes (sections 2.1 and 7.5 of the design) and the point cells of the query matrix.
 *
 *  Every query is a free function template in namespace GEOMETRY, overloaded over the pairs of shapes of section 7.5,
 *  so that the call states the question and never the algorithm:
 *
 *  - T Distance( a, b )                        the distance between the solids, 0 when they touch or overlap;
 *  - bool ClosestPoints( a, b, onA, onB )      the nearest points; when the shapes touch or overlap both are the same
 *                                              witness point that lies in both shapes; false (outputs unchanged) for a
 *                                              shape without points (an empty box, a negative radius);
 *  - bool Intersects( a, b )                   true when the solids share a point;
 *  - bool Contains( outer, inner )             true when the inner shape lies entirely in the outer one (section 7.5.5).
 *
 *  Each pair of shapes is answered by one overload in the header of the first shape of the canonical order (point,
 *  segment, ray, plane, triangle, sphere, capsule, AABB, OBB, cylinder, cone, frustum, plane-bounded volume), and the
 *  other order of the arguments is forwarded to it. The pairs of section 7.5.1 that the table gives to GJK and EPA
 *  (capsule with a box, any pair with a cylinder, cone or hull) are added by the convex query header of the GJK
 *  package as further overloads of the same functions; nothing here has to change for them. Distance of a shape
 *  without points is the largest number of the kind (the distance to the empty set).
 *
 *  Float32 and Float64 queries are exported instantiations (section 5.10), compiled without floating-point
 *  contraction. Every query is realtime-safe and allocation-free unless its documentation says otherwise.
 *
 *  This header holds the point cells: a point against any shape that has the uniform members of section 2.1.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_GEOMETRY_CTGEOMETRYTOLERANCE_H
#include "gucefGEOMETRY_CTGeometryTolerance.h"
#define GUCEF_GEOMETRY_CTGEOMETRYTOLERANCE_H
#endif /* GUCEF_GEOMETRY_CTGEOMETRYTOLERANCE_H ? */

#ifndef GUCEF_MATH_POINTS_H
#include "gucefMATH_points.h"
#define GUCEF_MATH_POINTS_H
#endif /* GUCEF_MATH_POINTS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Defines the overload Intersects( B, A ) that forwards to Intersects( A, B ): the other order of the arguments of a
 *  pair that is answered once. A and B are class template names with one parameter of the kind.
 */
#define GUCEF_GEOMETRY_SWAPPED_INTERSECTS( A, B )                                                                         \
    template < typename T >                                                                                               \
    GUCEF_GEOMETRY_FORCE_INLINE bool Intersects( const B< T >& b, const A< T >& a ) GUCEF_GEOMETRY_REALTIME_CONTEXT       \
    {                                                                                                                     \
        return Intersects( a, b );                                                                                        \
    }

/** As GUCEF_GEOMETRY_SWAPPED_INTERSECTS for ClosestPoints: the points are returned for the swapped roles */
#define GUCEF_GEOMETRY_SWAPPED_CLOSESTPOINTS( A, B )                                                                      \
    template < typename T >                                                                                               \
    GUCEF_GEOMETRY_FORCE_INLINE bool ClosestPoints( const B< T >& b, const A< T >& a, MATH::CTPoint3D< T >& onB,          \
                                                    MATH::CTPoint3D< T >& onA ) GUCEF_GEOMETRY_REALTIME_CONTEXT           \
    {                                                                                                                     \
        return ClosestPoints( a, b, onA, onB );                                                                           \
    }

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

/** The largest absolute coordinate of the points: the magnitude of the operands for the relative tolerance */
template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE T
GetCoordinateScale( const MATH::CTPoint3D< T >& p ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    return TTraits::Max( TTraits::Max( TTraits::Abs( p.x ), TTraits::Abs( p.y ) ), TTraits::Abs( p.z ) );
}

/*-------------------------------------------------------------------------*/

/** The point against a shape: true when the shape contains the point */
template < typename T, typename TShape >
GUCEF_GEOMETRY_FORCE_INLINE bool
Intersects( const MATH::CTPoint3D< T >& point, const TShape& shape ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return shape.Contains( point );
}

/** The point against a shape (the other order of the arguments) */
template < typename TShape, typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool
Intersects( const TShape& shape, const MATH::CTPoint3D< T >& point ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return shape.Contains( point );
}

/** The distance from the point to the solid shape, 0 inside */
template < typename T, typename TShape >
GUCEF_GEOMETRY_FORCE_INLINE T
Distance( const MATH::CTPoint3D< T >& point, const TShape& shape ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return shape.GetDistance( point );
}

/** The distance from the solid shape to the point, 0 inside */
template < typename TShape, typename T >
GUCEF_GEOMETRY_FORCE_INLINE T
Distance( const TShape& shape, const MATH::CTPoint3D< T >& point ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return shape.GetDistance( point );
}

/** The nearest point of the shape to the point; 'onPoint' is the point itself (also inside the shape: then both are the point) */
template < typename T, typename TShape >
GUCEF_GEOMETRY_FORCE_INLINE bool
ClosestPoints( const MATH::CTPoint3D< T >& point, const TShape& shape, MATH::CTPoint3D< T >& onPoint, MATH::CTPoint3D< T >& onShape ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    onPoint = point;
    onShape = shape.ClosestPoint( point );
    return true;
}

/** The nearest point of the shape to the point (the other order of the arguments) */
template < typename TShape, typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool
ClosestPoints( const TShape& shape, const MATH::CTPoint3D< T >& point, MATH::CTPoint3D< T >& onShape, MATH::CTPoint3D< T >& onPoint ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    onPoint = point;
    onShape = shape.ClosestPoint( point );
    return true;
}

/** The distance between the solids of two shapes (3D or 2D) that have ClosestPoints: 0 when they touch or overlap, the largest number of the kind when a shape has no points */
template < typename TA, typename TB >
GUCEF_GEOMETRY_FORCE_INLINE typename TA::TValue
Distance( const TA& a, const TB& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef typename TA::TValue T;
    typename TA::TPoint onA;
    typename TA::TPoint onB;
    if ( !ClosestPoints( a, b, onA, onB ) )
        return MATH::CTNumericTraits< T >::Max();
    return ( onB - onA ).GetLength();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_POINTQUERIES_H ? */
