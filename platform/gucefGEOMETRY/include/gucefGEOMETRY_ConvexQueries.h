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

#ifndef GUCEF_GEOMETRY_CONVEXQUERIES_H
#define GUCEF_GEOMETRY_CONVEXQUERIES_H

/*
 *  The G/E cells of the query matrix (section 7.5.1 of the design) as further overloads of the query facade of
 *  gucefGEOMETRY_PointQueries.h, and the contact query of every pair of bounded convex shapes:
 *
 *  - ClosestPoints( a, b, onA, onB ) and Intersects( a, b ) by GJK for the pairs the table gives to GJK: a segment or a
 *    triangle with a cylinder, cone or hull (and the nearest points with a box), a sphere with a cylinder, cone or hull,
 *    a capsule with a box, cylinder, cone or hull (core segment plus margin), an AABB with an OBB (nearest points), a
 *    cylinder, cone or hull, an OBB with an OBB (nearest points), cylinder, cone or hull, and cylinders, cones and
 *    hulls among themselves; Distance( a, b ) follows from ClosestPoints. Both orders of the arguments are offered.
 *  - The plane, frustum and plane-bounded volume cells of the hull (the support based cells of those headers).
 *  - ComputeContact( a, b, manifold ): the contact of two bounded convex shapes: the boxes (AABB, OBB in any
 *    combination) by SAT with face clipping (CTSatBoxes, up to 4 points), every other pair by GJK on the cores and,
 *    when the cores overlap, EPA on the cores, with the margins added (one point). True when the shapes touch or
 *    overlap; for separated shapes the manifold holds the nearest points with the separation as negative depth.
 *
 *  Float32, Float64 and TFixed32_32 are exported instantiations (section 5.10, amendment A-3 of section 11.2): the
 *  fixed-point queries use no floating point and are bit-exact on every platform. Every query is realtime-safe and
 *  allocation-free (EPA places its fixed-capacity work arrays on the stack: about 30 KB).
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_QUERIES_H
#include "gucefGEOMETRY_queries.h"
#define GUCEF_GEOMETRY_QUERIES_H
#endif /* GUCEF_GEOMETRY_QUERIES_H ? */

#ifndef GUCEF_GEOMETRY_CTCONVEXHULL3D_H
#include "gucefGEOMETRY_CTConvexHull3D.h"
#define GUCEF_GEOMETRY_CTCONVEXHULL3D_H
#endif /* GUCEF_GEOMETRY_CTCONVEXHULL3D_H ? */

#ifndef GUCEF_GEOMETRY_CTEPA_H
#include "gucefGEOMETRY_CTEpa.h"
#define GUCEF_GEOMETRY_CTEPA_H
#endif /* GUCEF_GEOMETRY_CTEPA_H ? */

#ifndef GUCEF_GEOMETRY_CTSATBOXES_H
#include "gucefGEOMETRY_CTSatBoxes.h"
#define GUCEF_GEOMETRY_CTSATBOXES_H
#endif /* GUCEF_GEOMETRY_CTSATBOXES_H ? */

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

/** False for a shape without points (an empty box, an invalid box, cylinder or cone, a negative radius, an empty hull) */
template < typename TShape >
GUCEF_GEOMETRY_FORCE_INLINE bool IsQueryableConvex( const TShape& /*shape*/ ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return true; }

template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool IsQueryableConvex( const CTSphere< T >& s ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return !MATH::CTNumericTraits< T >::IsNegative( s.radius ); }
template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool IsQueryableConvex( const CTCapsule< T >& c ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return !MATH::CTNumericTraits< T >::IsNegative( c.radius ); }
template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool IsQueryableConvex( const CTAxisAlignedBox3D< T >& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return !b.IsEmpty(); }
template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool IsQueryableConvex( const CTOrientedBox3D< T >& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return b.IsValid(); }
template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool IsQueryableConvex( const CTCylinder< T >& c ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return c.IsValid(); }
template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool IsQueryableConvex( const CTCone< T >& c ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return c.IsValid(); }
template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool IsQueryableConvex( const CTConvexHull3D< T >& h ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return !h.IsEmpty(); }

/*-------------------------------------------------------------------------*/

/** The nearest points of two bounded convex shapes by GJK on their support mappings (cores plus margins) */
template < typename TA, typename TB >
GUCEF_GEOMETRY_PRECISE_INLINE bool
GjkClosestPoints( const TA& a, const TB& b, MATH::CTPoint3D< typename TA::TValue >& onA, MATH::CTPoint3D< typename TA::TValue >& onB ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef typename TA::TValue T;
    if ( !IsQueryableConvex( a ) || !IsQueryableConvex( b ) )
        return false;
    SGjkResult< T > result;
    CTGjk< T >::ComputeDistance( CTSupportMapping< TA >::Make( a ), CTSupportMapping< TB >::Make( b ), CTGeometryTolerance< T >(), GUCEF_NULL, result );
    onA = result.pointOnA;
    onB = result.pointOnB;
    return true;
}

/*-------------------------------------------------------------------------*/

/** True when two bounded convex shapes touch or overlap, by GJK */
template < typename TA, typename TB >
GUCEF_GEOMETRY_PRECISE_INLINE bool
GjkIntersects( const TA& a, const TB& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef typename TA::TValue T;
    if ( !IsQueryableConvex( a ) || !IsQueryableConvex( b ) )
        return false;
    return CTGjk< T >::Intersects( CTSupportMapping< TA >::Make( a ), CTSupportMapping< TB >::Make( b ), CTGeometryTolerance< T >() );
}

/*-------------------------------------------------------------------------*/

/**
 *  The contact of two support mappings (cores plus margins) with an optional EPA workspace (GUCEF_NULL: EPA runs in a
 *  work object on the stack); see ComputeSupportContact.
 */
template < typename TSA, typename TSB >
GUCEF_GEOMETRY_PRECISE_INLINE MATH::TMathStatus
ComputeSupportContactIn( const TSA& a, const TSB& b, const CTGeometryTolerance< typename TSA::TValue >& tolerance, CTEpa< typename TSA::TValue >* workspace, CTContactManifold< typename TSA::TValue >& manifold ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef typename TSA::TValue T;
    manifold.Clear();
    SGjkResult< T > gjk;
    MATH::TMathStatus status = CTGjk< T >::ComputeDistance( a, b, tolerance, GUCEF_NULL, gjk );
    const T marginA = a.GetMargin();
    const T marginB = b.GetMargin();
    if ( !gjk.coresOverlapping )
    {
        const MATH::CTVector3D< T > n = gjk.normal;
        manifold.normal = n;
        const MATH::CTPoint3D< T > onA = gjk.corePointOnA + n * marginA;
        const MATH::CTPoint3D< T > onB = gjk.corePointOnB - n * marginB;
        manifold.AddPoint( onA, onB, ( marginA + marginB ) - gjk.coreDistance, 0 );
        manifold.status = status;
        return status;
    }
    SEpaResult< T > epa;
    if ( GUCEF_NULL != workspace )
        status |= CTEpa< T >::Penetration( a, b, gjk.simplex, tolerance, *workspace, epa );
    else
        status |= CTEpa< T >::Penetration( a, b, gjk.simplex, tolerance, epa );
    const MATH::CTVector3D< T > n = epa.normal;
    manifold.normal = n;
    manifold.AddPoint( epa.pointOnA + n * marginA, epa.pointOnB - n * marginB, epa.depth + marginA + marginB, 0 );
    manifold.status = status;
    return status;
}

/*-------------------------------------------------------------------------*/

/**
 *  The contact of two support mappings (cores plus margins): GJK on the cores; separated wholes give their nearest
 *  points with a negative depth, cores apart within the margins give the depth margins - core distance, overlapping
 *  cores give the EPA depth of the cores plus the margins. One point. Returns the status of GJK and EPA, also stored in
 *  manifold.status: MATHSTATUS_NOT_CONVERGED when either stopped at a cap (the point is an estimate),
 *  MATHSTATUS_DEGENERATE when the Minkowski difference of the cores is flat (the cores touch: the point is exact, the
 *  normal is one of the valid directions). EPA runs in a work object on the stack (see CTEpa::Penetration).
 */
template < typename TSA, typename TSB >
GUCEF_GEOMETRY_PRECISE_INLINE MATH::TMathStatus
ComputeSupportContact( const TSA& a, const TSB& b, const CTGeometryTolerance< typename TSA::TValue >& tolerance, CTContactManifold< typename TSA::TValue >& manifold ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return ComputeSupportContactIn( a, b, tolerance, (CTEpa< typename TSA::TValue >*) GUCEF_NULL, manifold );
}

/** ComputeSupportContact with EPA in a caller-provided work object */
template < typename TSA, typename TSB >
GUCEF_GEOMETRY_PRECISE_INLINE MATH::TMathStatus
ComputeSupportContact( const TSA& a, const TSB& b, const CTGeometryTolerance< typename TSA::TValue >& tolerance, CTEpa< typename TSA::TValue >& workspace, CTContactManifold< typename TSA::TValue >& manifold ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return ComputeSupportContactIn( a, b, tolerance, &workspace, manifold );
}

/*-------------------------------------------------------------------------*/

/**
 *  The contact of two bounded convex shapes (see the header comment): true when they touch or overlap. False for a
 *  shape without points (no manifold points) and for separated shapes (one point, negative depth). The status of the
 *  query is in manifold.status (see ComputeSupportContact).
 */
template < typename TA, typename TB >
GUCEF_GEOMETRY_PRECISE_INLINE bool
ComputeContact( const TA& a, const TB& b, CTContactManifold< typename TA::TValue >& manifold ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef typename TA::TValue T;
    manifold.Clear();
    if ( !IsQueryableConvex( a ) || !IsQueryableConvex( b ) )
        return false;
    ComputeSupportContact( CTSupportMapping< TA >::Make( a ), CTSupportMapping< TB >::Make( b ), CTGeometryTolerance< T >(), manifold );
    return manifold.IsTouching();
}

/*-------------------------------------------------------------------------*/

/** The contact of two oriented boxes: SAT with face clipping, at most 4 points */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
ComputeContact( const CTOrientedBox3D< T >& a, const CTOrientedBox3D< T >& b, CTContactManifold< T >& manifold ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return CTSatBoxes< T >::ComputeContact( a, b, CTGeometryTolerance< T >(), 4, manifold );
}

/** The contact of two axis aligned boxes: SAT with face clipping, at most 4 points */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
ComputeContact( const CTAxisAlignedBox3D< T >& a, const CTAxisAlignedBox3D< T >& b, CTContactManifold< T >& manifold ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    manifold.Clear();
    if ( a.IsEmpty() || b.IsEmpty() )
        return false;
    return CTSatBoxes< T >::ComputeContact( CTOrientedBox3D< T >::FromAxisAlignedBox( a ), CTOrientedBox3D< T >::FromAxisAlignedBox( b ), CTGeometryTolerance< T >(), 4, manifold );
}

/** The contact of an axis aligned and an oriented box: SAT with face clipping, at most 4 points */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
ComputeContact( const CTAxisAlignedBox3D< T >& a, const CTOrientedBox3D< T >& b, CTContactManifold< T >& manifold ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    manifold.Clear();
    if ( a.IsEmpty() )
        return false;
    return CTSatBoxes< T >::ComputeContact( CTOrientedBox3D< T >::FromAxisAlignedBox( a ), b, CTGeometryTolerance< T >(), 4, manifold );
}

/** The contact of an oriented and an axis aligned box: SAT with face clipping, at most 4 points */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
ComputeContact( const CTOrientedBox3D< T >& a, const CTAxisAlignedBox3D< T >& b, CTContactManifold< T >& manifold ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    manifold.Clear();
    if ( b.IsEmpty() )
        return false;
    return CTSatBoxes< T >::ComputeContact( a, CTOrientedBox3D< T >::FromAxisAlignedBox( b ), CTGeometryTolerance< T >(), 4, manifold );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Defines ClosestPoints of a pair through GJK, in both orders */
#define GUCEF_GEOMETRY_GJK_CLOSESTPOINTS( A, B )                                                                          \
    template < typename T >                                                                                               \
    GUCEF_GEOMETRY_PRECISE_INLINE bool                                                                                    \
    ClosestPoints( const A< T >& a, const B< T >& b, MATH::CTPoint3D< T >& onA, MATH::CTPoint3D< T >& onB ) GUCEF_GEOMETRY_REALTIME_CONTEXT \
    {                                                                                                                     \
        return GjkClosestPoints( a, b, onA, onB );                                                                        \
    }                                                                                                                     \
    GUCEF_GEOMETRY_SWAPPED_CLOSESTPOINTS( A, B )

/** Defines ClosestPoints and Intersects of a pair through GJK, in both orders */
#define GUCEF_GEOMETRY_GJK_PAIR( A, B )                                                                                   \
    GUCEF_GEOMETRY_GJK_CLOSESTPOINTS( A, B )                                                                              \
    template < typename T >                                                                                               \
    GUCEF_GEOMETRY_PRECISE_INLINE bool                                                                                    \
    Intersects( const A< T >& a, const B< T >& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT                                        \
    {                                                                                                                     \
        return GjkIntersects( a, b );                                                                                     \
    }                                                                                                                     \
    GUCEF_GEOMETRY_SWAPPED_INTERSECTS( A, B )

/** As GUCEF_GEOMETRY_GJK_PAIR for a shape paired with itself (one overload each) */
#define GUCEF_GEOMETRY_GJK_SELF_PAIR( A )                                                                                 \
    template < typename T >                                                                                               \
    GUCEF_GEOMETRY_PRECISE_INLINE bool                                                                                    \
    ClosestPoints( const A< T >& a, const A< T >& b, MATH::CTPoint3D< T >& onA, MATH::CTPoint3D< T >& onB ) GUCEF_GEOMETRY_REALTIME_CONTEXT \
    {                                                                                                                     \
        return GjkClosestPoints( a, b, onA, onB );                                                                        \
    }                                                                                                                     \
    template < typename T >                                                                                               \
    GUCEF_GEOMETRY_PRECISE_INLINE bool                                                                                    \
    Intersects( const A< T >& a, const A< T >& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT                                        \
    {                                                                                                                     \
        return GjkIntersects( a, b );                                                                                     \
    }

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/* segment and triangle: the boxes have an analytic or SAT overlap test, GJK gives the nearest points */
GUCEF_GEOMETRY_GJK_CLOSESTPOINTS( CTSegment3D, CTAxisAlignedBox3D )
GUCEF_GEOMETRY_GJK_CLOSESTPOINTS( CTSegment3D, CTOrientedBox3D )
GUCEF_GEOMETRY_GJK_PAIR( CTSegment3D, CTCylinder )
GUCEF_GEOMETRY_GJK_PAIR( CTSegment3D, CTCone )
GUCEF_GEOMETRY_GJK_PAIR( CTSegment3D, CTConvexHull3D )
GUCEF_GEOMETRY_GJK_CLOSESTPOINTS( CTTriangle3D, CTAxisAlignedBox3D )
GUCEF_GEOMETRY_GJK_CLOSESTPOINTS( CTTriangle3D, CTOrientedBox3D )
GUCEF_GEOMETRY_GJK_PAIR( CTTriangle3D, CTCylinder )
GUCEF_GEOMETRY_GJK_PAIR( CTTriangle3D, CTCone )
GUCEF_GEOMETRY_GJK_PAIR( CTTriangle3D, CTConvexHull3D )

/* sphere (point core plus margin) */
GUCEF_GEOMETRY_GJK_PAIR( CTSphere, CTCylinder )
GUCEF_GEOMETRY_GJK_PAIR( CTSphere, CTCone )
GUCEF_GEOMETRY_GJK_PAIR( CTSphere, CTConvexHull3D )

/* capsule (segment core plus margin) */
GUCEF_GEOMETRY_GJK_PAIR( CTCapsule, CTAxisAlignedBox3D )
GUCEF_GEOMETRY_GJK_PAIR( CTCapsule, CTOrientedBox3D )
GUCEF_GEOMETRY_GJK_PAIR( CTCapsule, CTCylinder )
GUCEF_GEOMETRY_GJK_PAIR( CTCapsule, CTCone )
GUCEF_GEOMETRY_GJK_PAIR( CTCapsule, CTConvexHull3D )

/* boxes: SAT decides the overlap of box pairs, GJK gives the nearest points */
GUCEF_GEOMETRY_GJK_CLOSESTPOINTS( CTAxisAlignedBox3D, CTOrientedBox3D )
GUCEF_GEOMETRY_GJK_PAIR( CTAxisAlignedBox3D, CTCylinder )
GUCEF_GEOMETRY_GJK_PAIR( CTAxisAlignedBox3D, CTCone )
GUCEF_GEOMETRY_GJK_PAIR( CTAxisAlignedBox3D, CTConvexHull3D )

/** The nearest points of two oriented boxes (GJK) */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
ClosestPoints( const CTOrientedBox3D< T >& a, const CTOrientedBox3D< T >& b, MATH::CTPoint3D< T >& onA, MATH::CTPoint3D< T >& onB ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return GjkClosestPoints( a, b, onA, onB );
}

GUCEF_GEOMETRY_GJK_PAIR( CTOrientedBox3D, CTCylinder )
GUCEF_GEOMETRY_GJK_PAIR( CTOrientedBox3D, CTCone )
GUCEF_GEOMETRY_GJK_PAIR( CTOrientedBox3D, CTConvexHull3D )

/* cylinder, cone and hull */
GUCEF_GEOMETRY_GJK_SELF_PAIR( CTCylinder )
GUCEF_GEOMETRY_GJK_PAIR( CTCylinder, CTCone )
GUCEF_GEOMETRY_GJK_PAIR( CTCylinder, CTConvexHull3D )
GUCEF_GEOMETRY_GJK_SELF_PAIR( CTCone )
GUCEF_GEOMETRY_GJK_PAIR( CTCone, CTConvexHull3D )
GUCEF_GEOMETRY_GJK_SELF_PAIR( CTConvexHull3D )

/* the support based cells of the plane, the frustum and the plane-bounded volume for the hull */
GUCEF_GEOMETRY_PLANE_QUERIES( CTConvexHull3D )
GUCEF_GEOMETRY_VOLUME_CONVEX_QUERIES( CTFrustum, CTConvexHull3D )
GUCEF_GEOMETRY_VOLUME_CONVEX_QUERIES( CTPlaneBoundedVolume, CTConvexHull3D )
GUCEF_GEOMETRY_VOLUME_INTERSECTS_BY_CLASSIFY( CTFrustum, CTConvexHull3D )
GUCEF_GEOMETRY_VOLUME_INTERSECTS_BY_CLASSIFY( CTPlaneBoundedVolume, CTConvexHull3D )

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/* For Float32, Float64 and TFixed32_32 the module's instantiations (gucefGEOMETRY_ConvexQueryInstantiations.cpp) are used */

#define GUCEF_GEOMETRY_CONVEXQUERIES_CLOSESTPOINTS( prefix, kind, A, B ) \
    prefix bool ClosestPoints< kind >( const A< kind >&, const B< kind >&, MATH::CTPoint3D< kind >&, MATH::CTPoint3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

#define GUCEF_GEOMETRY_CONVEXQUERIES_PAIR( prefix, kind, A, B ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_CLOSESTPOINTS( prefix, kind, A, B ) \
    prefix bool Intersects< kind >( const A< kind >&, const B< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

#define GUCEF_GEOMETRY_CONVEXQUERIES_CONTACT( prefix, kind, A, B ) \
    prefix bool ComputeContact< A< kind >, B< kind > >( const A< kind >&, const B< kind >&, CTContactManifold< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

#define GUCEF_GEOMETRY_CONVEXQUERIES_KIND( prefix, kind ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_CLOSESTPOINTS( prefix, kind, CTSegment3D, CTAxisAlignedBox3D ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_CLOSESTPOINTS( prefix, kind, CTSegment3D, CTOrientedBox3D ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_PAIR( prefix, kind, CTSegment3D, CTCylinder ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_PAIR( prefix, kind, CTSegment3D, CTCone ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_PAIR( prefix, kind, CTSegment3D, CTConvexHull3D ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_CLOSESTPOINTS( prefix, kind, CTTriangle3D, CTAxisAlignedBox3D ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_CLOSESTPOINTS( prefix, kind, CTTriangle3D, CTOrientedBox3D ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_PAIR( prefix, kind, CTTriangle3D, CTCylinder ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_PAIR( prefix, kind, CTTriangle3D, CTCone ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_PAIR( prefix, kind, CTTriangle3D, CTConvexHull3D ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_PAIR( prefix, kind, CTSphere, CTCylinder ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_PAIR( prefix, kind, CTSphere, CTCone ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_PAIR( prefix, kind, CTSphere, CTConvexHull3D ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_PAIR( prefix, kind, CTCapsule, CTAxisAlignedBox3D ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_PAIR( prefix, kind, CTCapsule, CTOrientedBox3D ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_PAIR( prefix, kind, CTCapsule, CTCylinder ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_PAIR( prefix, kind, CTCapsule, CTCone ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_PAIR( prefix, kind, CTCapsule, CTConvexHull3D ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_CLOSESTPOINTS( prefix, kind, CTAxisAlignedBox3D, CTOrientedBox3D ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_PAIR( prefix, kind, CTAxisAlignedBox3D, CTCylinder ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_PAIR( prefix, kind, CTAxisAlignedBox3D, CTCone ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_PAIR( prefix, kind, CTAxisAlignedBox3D, CTConvexHull3D ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_CLOSESTPOINTS( prefix, kind, CTOrientedBox3D, CTOrientedBox3D ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_PAIR( prefix, kind, CTOrientedBox3D, CTCylinder ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_PAIR( prefix, kind, CTOrientedBox3D, CTCone ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_PAIR( prefix, kind, CTOrientedBox3D, CTConvexHull3D ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_PAIR( prefix, kind, CTCylinder, CTCylinder ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_PAIR( prefix, kind, CTCylinder, CTCone ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_PAIR( prefix, kind, CTCylinder, CTConvexHull3D ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_PAIR( prefix, kind, CTCone, CTCone ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_PAIR( prefix, kind, CTCone, CTConvexHull3D ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_PAIR( prefix, kind, CTConvexHull3D, CTConvexHull3D ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_CONTACT( prefix, kind, CTSphere, CTSphere ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_CONTACT( prefix, kind, CTSphere, CTCapsule ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_CONTACT( prefix, kind, CTSphere, CTOrientedBox3D ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_CONTACT( prefix, kind, CTSphere, CTCylinder ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_CONTACT( prefix, kind, CTSphere, CTConvexHull3D ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_CONTACT( prefix, kind, CTCapsule, CTCapsule ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_CONTACT( prefix, kind, CTCapsule, CTOrientedBox3D ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_CONTACT( prefix, kind, CTCapsule, CTCylinder ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_CONTACT( prefix, kind, CTCapsule, CTConvexHull3D ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_CONTACT( prefix, kind, CTOrientedBox3D, CTCylinder ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_CONTACT( prefix, kind, CTOrientedBox3D, CTConvexHull3D ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_CONTACT( prefix, kind, CTCylinder, CTCylinder ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_CONTACT( prefix, kind, CTCylinder, CTConvexHull3D ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_CONTACT( prefix, kind, CTConvexHull3D, CTConvexHull3D ) \
    prefix bool ComputeContact< kind >( const CTOrientedBox3D< kind >&, const CTOrientedBox3D< kind >&, CTContactManifold< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool ComputeContact< kind >( const CTAxisAlignedBox3D< kind >&, const CTAxisAlignedBox3D< kind >&, CTContactManifold< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool ComputeContact< kind >( const CTAxisAlignedBox3D< kind >&, const CTOrientedBox3D< kind >&, CTContactManifold< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool ComputeContact< kind >( const CTOrientedBox3D< kind >&, const CTAxisAlignedBox3D< kind >&, CTContactManifold< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

/* the plane and volume cells of the hull, for the float kinds of those headers */
#define GUCEF_GEOMETRY_CONVEXQUERIES_FLOAT_KIND( prefix, kind ) \
    GUCEF_GEOMETRY_PLANEQUERIES_SHAPE( prefix, kind, CTConvexHull3D ) \
    GUCEF_GEOMETRY_VOLUMEQUERIES_SHAPE( prefix, kind, CTConvexHull3D )

#define GUCEF_GEOMETRY_CONVEXQUERIES_PREDEFINED_KINDS( prefix ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_KIND( prefix, Float32 ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_KIND( prefix, Float64 ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_KIND( prefix, MATH::TFixed32_32 ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_FLOAT_KIND( prefix, Float32 ) \
    GUCEF_GEOMETRY_CONVEXQUERIES_FLOAT_KIND( prefix, Float64 )

#ifdef GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED
GUCEF_GEOMETRY_CONVEXQUERIES_PREDEFINED_KINDS( GUCEF_GEOMETRY_EXTERN_TEMPLATE )
#endif /* GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CONVEXQUERIES_H ? */
