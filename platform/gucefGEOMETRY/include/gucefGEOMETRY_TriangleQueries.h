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

#ifndef GUCEF_GEOMETRY_TRIANGLEQUERIES_H
#define GUCEF_GEOMETRY_TRIANGLEQUERIES_H

/*
 *  The triangle cells of the query matrix (section 7.5.1 of the design): a triangle against a triangle, sphere,
 *  capsule, axis aligned box and oriented box. The segment and plane cells are in their own headers. See
 *  gucefGEOMETRY_PointQueries.h for the conventions of the facade.
 *
 *  - Triangle against triangle: the nearest points are found from the six edge-against-triangle pairs (two triangles
 *    that overlap always have an edge of one touching the other); the overlap test counts triangles within the length
 *    tolerance as touching.
 *  - Triangle against a box: the separating axis test with the 13 axes of Akenine-Moeller (the box axes, the normal of
 *    the triangle and the nine cross products of an edge and a box axis), exact for touching boxes.
 *  - A degenerate triangle is treated as its edges.
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
 *  The separating axis test of a triangle against a box centred at the origin with the given half extents (not
 *  negative): the triangle is given relative to the centre of the box in the frame of the box. True when they share a
 *  point (touching counts). Building block of the triangle-box cells.
 */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
TriangleOverlapsCentredBox( const MATH::CTVector3D< T >& v0, const MATH::CTVector3D< T >& v1, const MATH::CTVector3D< T >& v2,
                            const MATH::CTVector3D< T >& halfExtents ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTVector3D< T > TVector;

    /* the three axes of the box */
    for ( UInt32 i=0; i<3; ++i )
    {
        const T h = halfExtents.GetComponent( i );
        const T a = v0.GetComponent( i );
        const T b = v1.GetComponent( i );
        const T c = v2.GetComponent( i );
        if ( TTraits::Min( TTraits::Min( a, b ), c ) > h || TTraits::Max( TTraits::Max( a, b ), c ) < MATH::ComponentNegate( h ) )
            return false;
    }

    const TVector edges[ 3 ] = { v1 - v0, v2 - v1, v0 - v2 };

    /* the normal of the triangle */
    {
        const TVector n = edges[ 0 ].Cross( edges[ 1 ] );
        const T radius = ( halfExtents.x * TTraits::Abs( n.x ) + halfExtents.y * TTraits::Abs( n.y ) ) + halfExtents.z * TTraits::Abs( n.z );
        if ( radius < TTraits::Abs( n.Dot( v0 ) ) )
            return false;
    }

    /* the nine cross products of an edge and a box axis */
    for ( UInt32 e=0; e<3; ++e )
    {
        for ( UInt32 i=0; i<3; ++i )
        {
            TVector unit;
            unit.SetComponent( i, TTraits::One() );
            const TVector axis = edges[ e ].Cross( unit );
            const T p0 = axis.Dot( v0 );
            const T p1 = axis.Dot( v1 );
            const T p2 = axis.Dot( v2 );
            const T radius = ( halfExtents.x * TTraits::Abs( axis.x ) + halfExtents.y * TTraits::Abs( axis.y ) ) + halfExtents.z * TTraits::Abs( axis.z );
            if ( TTraits::Min( TTraits::Min( p0, p1 ), p2 ) > radius || TTraits::Max( TTraits::Max( p0, p1 ), p2 ) < MATH::ComponentNegate( radius ) )
                return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** The nearest points of two triangles (solid, flat): from the six edge-against-triangle pairs */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
ClosestPoints( const CTTriangle3D< T >& a, const CTTriangle3D< T >& b, MATH::CTPoint3D< T >& onA, MATH::CTPoint3D< T >& onB ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    T best = TTraits::Max();
    for ( UInt32 i=0; i<6; ++i )
    {
        TPoint pe;
        TPoint pt;
        if ( i < 3 )
        {
            ClosestPoints( CTSegment3D< T >( a.GetVertex( i ), a.GetVertex( ( i + 1 ) % 3 ) ), b, pe, pt );
            const T d = ( pt - pe ).GetSquaredLength();
            if ( d < best )
            {
                best = d;
                onA = pe;
                onB = pt;
            }
        }
        else
        {
            ClosestPoints( CTSegment3D< T >( b.GetVertex( i - 3 ), b.GetVertex( ( i - 2 ) % 3 ) ), a, pe, pt );
            const T d = ( pt - pe ).GetSquaredLength();
            if ( d < best )
            {
                best = d;
                onA = pt;
                onB = pe;
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** The nearest points of a triangle and a solid sphere; false for a negative radius */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
ClosestPoints( const CTTriangle3D< T >& triangle, const CTSphere< T >& sphere, MATH::CTPoint3D< T >& onTriangle, MATH::CTPoint3D< T >& onSphere ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    if ( TTraits::IsNegative( sphere.radius ) )
        return false;
    ClosestPointsOfRoundCores( triangle.ClosestPoint( sphere.centre ), TTraits::Zero(), sphere.centre, sphere.radius, onTriangle, onSphere );
    return true;
}

/*-------------------------------------------------------------------------*/

/** The nearest points of a triangle and a solid capsule; false for a negative radius */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
ClosestPoints( const CTTriangle3D< T >& triangle, const CTCapsule< T >& capsule, MATH::CTPoint3D< T >& onTriangle, MATH::CTPoint3D< T >& onCapsule ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    if ( TTraits::IsNegative( capsule.radius ) )
        return false;
    MATH::CTPoint3D< T > ps;
    MATH::CTPoint3D< T > pt;
    ClosestPoints( capsule.GetSegment(), triangle, ps, pt );
    ClosestPointsOfRoundCores( pt, TTraits::Zero(), ps, capsule.radius, onTriangle, onCapsule );
    return true;
}

/*-------------------------------------------------------------------------*/

/** True when the triangles are within the tolerance of each other (touching, crossing and containment count) */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Intersects( const CTTriangle3D< T >& a, const CTTriangle3D< T >& b, const CTGeometryTolerance< T >& tolerance ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    MATH::CTPoint3D< T > pa;
    MATH::CTPoint3D< T > pb;
    ClosestPoints( a, b, pa, pb );
    const T limit = tolerance.GetLengthTolerance( MATH::CTNumericTraits< T >::Max( GetCoordinateScale( pa ), GetCoordinateScale( pb ) ) );
    return ( pb - pa ).GetSquaredLength() <= limit * limit;
}

template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool
Intersects( const CTTriangle3D< T >& a, const CTTriangle3D< T >& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return Intersects( a, b, CTGeometryTolerance< T >() );
}

/*-------------------------------------------------------------------------*/

/** True when the triangle shares a point with the solid sphere */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Intersects( const CTTriangle3D< T >& triangle, const CTSphere< T >& sphere ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( MATH::CTNumericTraits< T >::IsNegative( sphere.radius ) )
        return false;
    return triangle.GetSquaredDistance( sphere.centre ) <= sphere.radius * sphere.radius;
}

/*-------------------------------------------------------------------------*/

/** True when the triangle shares a point with the solid capsule */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Intersects( const CTTriangle3D< T >& triangle, const CTCapsule< T >& capsule ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( MATH::CTNumericTraits< T >::IsNegative( capsule.radius ) )
        return false;
    MATH::CTPoint3D< T > ps;
    MATH::CTPoint3D< T > pt;
    ClosestPoints( capsule.GetSegment(), triangle, ps, pt );
    return ( pt - ps ).GetSquaredLength() <= capsule.radius * capsule.radius;
}

/*-------------------------------------------------------------------------*/

/** True when the triangle shares a point with the box (separating axis test, 13 axes); false for an empty box */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Intersects( const CTTriangle3D< T >& triangle, const CTAxisAlignedBox3D< T >& box ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( box.IsEmpty() )
        return false;
    const MATH::CTPoint3D< T > centre = box.GetCentre();
    return TriangleOverlapsCentredBox( triangle.a - centre, triangle.b - centre, triangle.c - centre, box.GetHalfExtents() );
}

/*-------------------------------------------------------------------------*/

/** True when the triangle shares a point with the oriented box (the triangle in the frame of the box against the box); false for an invalid box */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Intersects( const CTTriangle3D< T >& triangle, const CTOrientedBox3D< T >& box ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( !box.IsValid() )
        return false;
    return TriangleOverlapsCentredBox( box.ToLocal( triangle.a ), box.ToLocal( triangle.b ), box.ToLocal( triangle.c ), box.halfExtents );
}

/*-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_SWAPPED_INTERSECTS( CTTriangle3D, CTSphere )
GUCEF_GEOMETRY_SWAPPED_INTERSECTS( CTTriangle3D, CTCapsule )
GUCEF_GEOMETRY_SWAPPED_INTERSECTS( CTTriangle3D, CTAxisAlignedBox3D )
GUCEF_GEOMETRY_SWAPPED_INTERSECTS( CTTriangle3D, CTOrientedBox3D )
GUCEF_GEOMETRY_SWAPPED_CLOSESTPOINTS( CTTriangle3D, CTSphere )
GUCEF_GEOMETRY_SWAPPED_CLOSESTPOINTS( CTTriangle3D, CTCapsule )

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/* For Float32 and Float64 the module's instantiations (gucefGEOMETRY_QueryInstantiations.cpp) are used, section 5.10 */

#define GUCEF_GEOMETRY_TRIANGLEQUERIES_KIND( prefix, kind ) \
    prefix bool TriangleOverlapsCentredBox< kind >( const MATH::CTVector3D< kind >&, const MATH::CTVector3D< kind >&, const MATH::CTVector3D< kind >&, const MATH::CTVector3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool ClosestPoints< kind >( const CTTriangle3D< kind >&, const CTTriangle3D< kind >&, MATH::CTPoint3D< kind >&, MATH::CTPoint3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool ClosestPoints< kind >( const CTTriangle3D< kind >&, const CTSphere< kind >&, MATH::CTPoint3D< kind >&, MATH::CTPoint3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool ClosestPoints< kind >( const CTTriangle3D< kind >&, const CTCapsule< kind >&, MATH::CTPoint3D< kind >&, MATH::CTPoint3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Intersects< kind >( const CTTriangle3D< kind >&, const CTTriangle3D< kind >&, const CTGeometryTolerance< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Intersects< kind >( const CTTriangle3D< kind >&, const CTSphere< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Intersects< kind >( const CTTriangle3D< kind >&, const CTCapsule< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Intersects< kind >( const CTTriangle3D< kind >&, const CTAxisAlignedBox3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Intersects< kind >( const CTTriangle3D< kind >&, const CTOrientedBox3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

#define GUCEF_GEOMETRY_TRIANGLEQUERIES_PREDEFINED_KINDS( prefix ) \
    GUCEF_GEOMETRY_TRIANGLEQUERIES_KIND( prefix, Float32 ) \
    GUCEF_GEOMETRY_TRIANGLEQUERIES_KIND( prefix, Float64 )

#ifdef GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED
GUCEF_GEOMETRY_TRIANGLEQUERIES_PREDEFINED_KINDS( GUCEF_GEOMETRY_EXTERN_TEMPLATE )
#endif /* GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_TRIANGLEQUERIES_H ? */
