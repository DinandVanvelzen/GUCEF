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

#ifndef GUCEF_GEOMETRY_AXISALIGNEDBOXQUERIES_H
#define GUCEF_GEOMETRY_AXISALIGNEDBOXQUERIES_H

/*
 *  The axis aligned box cells of the query matrix (sections 7.5.1 and 7.5.5 of the design): box against box (distance,
 *  nearest points, overlap), box against oriented box (separating axis test with 15 axes, Gottschalk) and the
 *  containment of shapes in a box (a convex shape lies in a box when its support points along the box axes do). See
 *  gucefGEOMETRY_PointQueries.h for the conventions of the facade.
 *
 *  An empty box has no points: it intersects nothing, contains nothing and has no nearest points.
 *  The separating axis test treats the axes that come from the cross product of two nearly parallel edges as
 *  separating only when the gap exceeds a margin of 16 machine epsilons of the box extents, so that a nearly parallel
 *  pair of edges cannot produce a wrong separation; this errs towards reporting an overlap, by less than 1e-5 of the extent
 *  for Float32.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_CAPSULEQUERIES_H
#include "gucefGEOMETRY_CapsuleQueries.h"
#define GUCEF_GEOMETRY_CAPSULEQUERIES_H
#endif /* GUCEF_GEOMETRY_CAPSULEQUERIES_H ? */

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
 *  The separating axis test of two oriented boxes given by centre, three unit axes and half extents (not negative):
 *  true when they share a point (touching counts). The 15 axes are the three axes of each box and the nine cross
 *  products of one axis of each (Gottschalk, as in Ericson 4.4.1). Building block of the box-box cells. The tolerance
 *  gives the gap below which boxes count as touching (its length tolerance for the magnitude of the boxes) and, as its
 *  angular tolerance, the margin for edge axes of nearly parallel edges.
 */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
OrientedBoxesOverlap( const MATH::CTPoint3D< T >& centreA, const MATH::CTVector3D< T > ( &axesA )[ 3 ], const MATH::CTVector3D< T >& halfA,
                      const MATH::CTPoint3D< T >& centreB, const MATH::CTVector3D< T > ( &axesB )[ 3 ], const MATH::CTVector3D< T >& halfB, const CTGeometryTolerance< T >& tolerance ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTVector3D< T > TVector;
    const T epsilon = tolerance.angular;                /* the margin for edges that are nearly parallel: their cross product axis is not reliable */
    const T ea[ 3 ] = { halfA.x, halfA.y, halfA.z };
    const T eb[ 3 ] = { halfB.x, halfB.y, halfB.z };

    /* the rotation of B in the frame of A, and its absolute value with the margin for parallel edges */
    T r[ 3 ][ 3 ];
    T absR[ 3 ][ 3 ];
    for ( UInt32 i=0; i<3; ++i )
    {
        for ( UInt32 j=0; j<3; ++j )
        {
            r[ i ][ j ] = axesA[ i ].Dot( axesB[ j ] );
            absR[ i ][ j ] = TTraits::Abs( r[ i ][ j ] ) + epsilon;
        }
    }
    const TVector between = centreB - centreA;
    const T t[ 3 ] = { between.Dot( axesA[ 0 ] ), between.Dot( axesA[ 1 ] ), between.Dot( axesA[ 2 ] ) };

    const T slack = tolerance.GetLengthTolerance( TTraits::Max( TTraits::Max( halfA.GetChebyshevLength(), halfB.GetChebyshevLength() ), between.GetChebyshevLength() ) );

    /* the axes of A */
    for ( UInt32 i=0; i<3; ++i )
    {
        const T ra = ea[ i ];
        const T rb = ( eb[ 0 ] * absR[ i ][ 0 ] + eb[ 1 ] * absR[ i ][ 1 ] ) + eb[ 2 ] * absR[ i ][ 2 ];
        if ( ra + rb + slack < TTraits::Abs( t[ i ] ) )
            return false;
    }
    /* the axes of B */
    for ( UInt32 j=0; j<3; ++j )
    {
        const T ra = ( ea[ 0 ] * absR[ 0 ][ j ] + ea[ 1 ] * absR[ 1 ][ j ] ) + ea[ 2 ] * absR[ 2 ][ j ];
        const T rb = eb[ j ];
        if ( ra + rb + slack < TTraits::Abs( ( t[ 0 ] * r[ 0 ][ j ] + t[ 1 ] * r[ 1 ][ j ] ) + t[ 2 ] * r[ 2 ][ j ] ) )
            return false;
    }
    /* the nine cross products A_i x B_j */
    for ( UInt32 i=0; i<3; ++i )
    {
        const UInt32 i1 = ( i + 1 ) % 3;
        const UInt32 i2 = ( i + 2 ) % 3;
        for ( UInt32 j=0; j<3; ++j )
        {
            const UInt32 j1 = ( j + 1 ) % 3;
            const UInt32 j2 = ( j + 2 ) % 3;
            const T ra = ea[ i1 ] * absR[ i2 ][ j ] + ea[ i2 ] * absR[ i1 ][ j ];
            const T rb = eb[ j1 ] * absR[ i ][ j2 ] + eb[ j2 ] * absR[ i ][ j1 ];
            const T distance = TTraits::Abs( t[ i2 ] * r[ i1 ][ j ] - t[ i1 ] * r[ i2 ][ j ] );
            if ( ra + rb + slack < distance )
                return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  The exact form: no length tolerance, and the margin for nearly parallel edges is 16 machine epsilons (the form that
 *  the Intersects overloads without a tolerance use)
 */
template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool
OrientedBoxesOverlap( const MATH::CTPoint3D< T >& centreA, const MATH::CTVector3D< T > ( &axesA )[ 3 ], const MATH::CTVector3D< T >& halfA,
                      const MATH::CTPoint3D< T >& centreB, const MATH::CTVector3D< T > ( &axesB )[ 3 ], const MATH::CTVector3D< T >& halfB ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    return OrientedBoxesOverlap( centreA, axesA, halfA, centreB, axesB, halfB,
                                 CTGeometryTolerance< T >( TTraits::Zero(), TTraits::Zero(), TTraits::Epsilon() * TTraits::FromInt32( 16 ) ) );
}

/*-------------------------------------------------------------------------*/

/** The nearest points of two solid boxes: per axis the facing faces, or the middle of the overlap; false when a box is empty */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
ClosestPoints( const CTAxisAlignedBox3D< T >& a, const CTAxisAlignedBox3D< T >& b, MATH::CTPoint3D< T >& onA, MATH::CTPoint3D< T >& onB ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    if ( a.IsEmpty() || b.IsEmpty() )
        return false;
    MATH::CTPoint3D< T > pa;
    MATH::CTPoint3D< T > pb;
    for ( UInt32 axis=0; axis<3; ++axis )
    {
        const T alo = a.min.GetComponent( axis );
        const T ahi = a.max.GetComponent( axis );
        const T blo = b.min.GetComponent( axis );
        const T bhi = b.max.GetComponent( axis );
        T ca;
        T cb;
        if ( ahi < blo )
        {
            ca = ahi;
            cb = blo;
        }
        else if ( bhi < alo )
        {
            ca = alo;
            cb = bhi;
        }
        else
        {
            ca = ( TTraits::Max( alo, blo ) + TTraits::Min( ahi, bhi ) ) * TTraits::Half();
            cb = ca;
        }
        pa.SetComponent( axis, ca );
        pb.SetComponent( axis, cb );
    }
    onA = pa;
    onB = pb;
    return true;
}

/*-------------------------------------------------------------------------*/

/** True when the closed boxes share a point; false when a box is empty */
template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool
Intersects( const CTAxisAlignedBox3D< T >& a, const CTAxisAlignedBox3D< T >& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return a.Intersects( b );
}

/** True when the box and the oriented box share a point (separating axis test with 15 axes), boxes within the tolerance of each other counting as touching; false when a box is empty or invalid */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Intersects( const CTAxisAlignedBox3D< T >& a, const CTOrientedBox3D< T >& b, const CTGeometryTolerance< T >& tolerance ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTVector3D< T > TVector;
    if ( a.IsEmpty() || !b.IsValid() )
        return false;
    const TVector identity[ 3 ] = { TVector::UnitX(), TVector::UnitY(), TVector::UnitZ() };
    TVector axes[ 3 ];
    b.GetAxes( axes );
    return OrientedBoxesOverlap( a.GetCentre(), identity, a.GetHalfExtents(), b.centre, axes, b.halfExtents, tolerance );
}

/** The exact form of the test: a tolerance of zero (and the margin of 16 machine epsilons for parallel edges) */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Intersects( const CTAxisAlignedBox3D< T >& a, const CTOrientedBox3D< T >& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    return Intersects( a, b, CTGeometryTolerance< T >( TTraits::Zero(), TTraits::Zero(), TTraits::Epsilon() * TTraits::FromInt32( 16 ) ) );
}

/*-------------------------------------------------------------------------*/

/**
 *  True when a convex shape that has GetSupport lies entirely in the box: its support points along the six axis
 *  directions lie inside (the building block of the containment cells below). False for an empty box.
 */
template < typename T, typename TShape >
GUCEF_GEOMETRY_PRECISE_INLINE bool
ContainsConvex( const CTAxisAlignedBox3D< T >& box, const TShape& shape ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTVector3D< T > TVector;
    if ( box.IsEmpty() )
        return false;
    for ( UInt32 axis=0; axis<3; ++axis )
    {
        TVector unit;
        unit.SetComponent( axis, MATH::CTNumericTraits< T >::One() );
        if ( box.max.GetComponent( axis ) < shape.GetSupport( unit ).GetComponent( axis ) ||
             shape.GetSupport( -unit ).GetComponent( axis ) < box.min.GetComponent( axis ) )
            return false;
    }
    return true;
}

/** True when the box contains the other box entirely (the member of the box); false when a box is empty */
template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool
Contains( const CTAxisAlignedBox3D< T >& box, const CTAxisAlignedBox3D< T >& inner ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return !box.IsEmpty() && !inner.IsEmpty() && box.Contains( inner );
}

/** True when the box contains the sphere entirely; false for an empty box or a negative radius */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Contains( const CTAxisAlignedBox3D< T >& box, const CTSphere< T >& sphere ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( box.IsEmpty() || MATH::CTNumericTraits< T >::IsNegative( sphere.radius ) )
        return false;
    const MATH::CTVector3D< T > r( sphere.radius, sphere.radius, sphere.radius );
    return box.Contains( sphere.centre - r ) && box.Contains( sphere.centre + r );
}

/** True when the box contains all 'count' points (true for none, also for an empty box) */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Contains( const CTAxisAlignedBox3D< T >& box, const MATH::CTPoint3D< T >* points, UInt32 count ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    for ( UInt32 i=0; i<count; ++i )
    {
        if ( !box.Contains( points[ i ] ) )
            return false;
    }
    return true;
}

/** True when the box contains the oriented box entirely (the corners lie inside); false for an empty or invalid box */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Contains( const CTAxisAlignedBox3D< T >& box, const CTOrientedBox3D< T >& inner ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( box.IsEmpty() || !inner.IsValid() )
        return false;
    return ContainsConvex( box, inner );
}

/** True when the box contains the capsule entirely; false for an empty box or a negative radius */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Contains( const CTAxisAlignedBox3D< T >& box, const CTCapsule< T >& inner ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( MATH::CTNumericTraits< T >::IsNegative( inner.radius ) )
        return false;
    return ContainsConvex( box, inner );
}

/** True when the box contains the triangle entirely */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Contains( const CTAxisAlignedBox3D< T >& box, const CTTriangle3D< T >& inner ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return ContainsConvex( box, inner );
}

/** True when the box contains the segment entirely */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Contains( const CTAxisAlignedBox3D< T >& box, const CTSegment3D< T >& inner ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return ContainsConvex( box, inner );
}

/*-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_SWAPPED_INTERSECTS( CTAxisAlignedBox3D, CTOrientedBox3D )

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/* For Float32 and Float64 the module's instantiations (gucefGEOMETRY_QueryInstantiations.cpp) are used, section 5.10 */

#define GUCEF_GEOMETRY_AXISALIGNEDBOXQUERIES_KIND( prefix, kind ) \
    prefix bool OrientedBoxesOverlap< kind >( const MATH::CTPoint3D< kind >&, const MATH::CTVector3D< kind > ( & )[ 3 ], const MATH::CTVector3D< kind >&, const MATH::CTPoint3D< kind >&, const MATH::CTVector3D< kind > ( & )[ 3 ], const MATH::CTVector3D< kind >&, const CTGeometryTolerance< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool ClosestPoints< kind >( const CTAxisAlignedBox3D< kind >&, const CTAxisAlignedBox3D< kind >&, MATH::CTPoint3D< kind >&, MATH::CTPoint3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Intersects< kind >( const CTAxisAlignedBox3D< kind >&, const CTOrientedBox3D< kind >&, const CTGeometryTolerance< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Contains< kind >( const CTAxisAlignedBox3D< kind >&, const CTSphere< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Contains< kind >( const CTAxisAlignedBox3D< kind >&, const MATH::CTPoint3D< kind >*, UInt32 ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Contains< kind >( const CTAxisAlignedBox3D< kind >&, const CTOrientedBox3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Contains< kind >( const CTAxisAlignedBox3D< kind >&, const CTCapsule< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Contains< kind >( const CTAxisAlignedBox3D< kind >&, const CTTriangle3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Contains< kind >( const CTAxisAlignedBox3D< kind >&, const CTSegment3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

#define GUCEF_GEOMETRY_AXISALIGNEDBOXQUERIES_PREDEFINED_KINDS( prefix ) \
    GUCEF_GEOMETRY_AXISALIGNEDBOXQUERIES_KIND( prefix, Float32 ) \
    GUCEF_GEOMETRY_AXISALIGNEDBOXQUERIES_KIND( prefix, Float64 )

#ifdef GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED
GUCEF_GEOMETRY_AXISALIGNEDBOXQUERIES_PREDEFINED_KINDS( GUCEF_GEOMETRY_EXTERN_TEMPLATE )
#endif /* GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_AXISALIGNEDBOXQUERIES_H ? */
