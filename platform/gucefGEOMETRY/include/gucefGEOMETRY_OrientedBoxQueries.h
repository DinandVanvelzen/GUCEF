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

#ifndef GUCEF_GEOMETRY_ORIENTEDBOXQUERIES_H
#define GUCEF_GEOMETRY_ORIENTEDBOXQUERIES_H

/*
 *  The oriented box cells of the query matrix (sections 7.5.1 and 7.5.5 of the design): box against box (separating
 *  axis test with 15 axes) and the containment of shapes in an oriented box. The cells with a segment, triangle, sphere
 *  and axis aligned box are in the headers of those shapes. See gucefGEOMETRY_PointQueries.h for the conventions of
 *  the facade, and gucefGEOMETRY_AxisAlignedBoxQueries.h for the margin of the separating axis test.
 *
 *  An invalid box (a negative half extent) has no points: it intersects nothing and contains nothing.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_AXISALIGNEDBOXQUERIES_H
#include "gucefGEOMETRY_AxisAlignedBoxQueries.h"
#define GUCEF_GEOMETRY_AXISALIGNEDBOXQUERIES_H
#endif /* GUCEF_GEOMETRY_AXISALIGNEDBOXQUERIES_H ? */

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

/** True when the oriented boxes share a point (separating axis test with 15 axes), boxes within the tolerance of each other counting as touching; false when a box is invalid */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Intersects( const CTOrientedBox3D< T >& a, const CTOrientedBox3D< T >& b, const CTGeometryTolerance< T >& tolerance ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTVector3D< T > TVector;
    if ( !a.IsValid() || !b.IsValid() )
        return false;
    TVector axesA[ 3 ];
    TVector axesB[ 3 ];
    a.GetAxes( axesA );
    b.GetAxes( axesB );
    return OrientedBoxesOverlap( a.centre, axesA, a.halfExtents, b.centre, axesB, b.halfExtents, tolerance );
}

/** The exact form of the test: a tolerance of zero (and the margin of 16 machine epsilons for parallel edges) */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Intersects( const CTOrientedBox3D< T >& a, const CTOrientedBox3D< T >& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    return Intersects( a, b, CTGeometryTolerance< T >( TTraits::Zero(), TTraits::Zero(), TTraits::Epsilon() * TTraits::FromInt32( 16 ) ) );
}

/*-------------------------------------------------------------------------*/

/**
 *  True when a convex shape that has GetSupport lies entirely in the oriented box: its support points along the six
 *  axis directions of the box lie inside. False for an invalid box.
 */
template < typename T, typename TShape >
GUCEF_GEOMETRY_PRECISE_INLINE bool
ContainsConvex( const CTOrientedBox3D< T >& box, const TShape& shape ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTVector3D< T > TVector;
    if ( !box.IsValid() )
        return false;
    TVector axes[ 3 ];
    box.GetAxes( axes );
    for ( UInt32 axis=0; axis<3; ++axis )
    {
        const T centre = box.centre.ToPositionVector().Dot( axes[ axis ] );
        const T half = box.halfExtents.GetComponent( axis );
        if ( centre + half < shape.GetSupport( axes[ axis ] ).ToPositionVector().Dot( axes[ axis ] ) ||
             shape.GetSupport( -axes[ axis ] ).ToPositionVector().Dot( axes[ axis ] ) < centre - half )
            return false;
    }
    return true;
}

/** True when the oriented box contains the sphere entirely; false for an invalid box or a negative radius */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Contains( const CTOrientedBox3D< T >& box, const CTSphere< T >& sphere ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    if ( !box.IsValid() || TTraits::IsNegative( sphere.radius ) )
        return false;
    const MATH::CTVector3D< T > local = box.ToLocal( sphere.centre );
    return sphere.radius <= box.halfExtents.x - TTraits::Abs( local.x ) && sphere.radius <= box.halfExtents.y - TTraits::Abs( local.y ) &&
           sphere.radius <= box.halfExtents.z - TTraits::Abs( local.z );
}

/** True when the oriented box contains the axis aligned box entirely (its corners lie inside); false for an invalid or empty box */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Contains( const CTOrientedBox3D< T >& box, const CTAxisAlignedBox3D< T >& inner ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( !box.IsValid() || inner.IsEmpty() )
        return false;
    for ( UInt32 i=0; i<8; ++i )
    {
        if ( !box.Contains( inner.GetCorner( i ) ) )
            return false;
    }
    return true;
}

/** True when the oriented box contains the other oriented box entirely; false when a box is invalid */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Contains( const CTOrientedBox3D< T >& box, const CTOrientedBox3D< T >& inner ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( !box.IsValid() || !inner.IsValid() )
        return false;
    return ContainsConvex( box, inner );
}

/** True when the oriented box contains the capsule entirely; false for an invalid box or a negative radius */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Contains( const CTOrientedBox3D< T >& box, const CTCapsule< T >& inner ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( MATH::CTNumericTraits< T >::IsNegative( inner.radius ) )
        return false;
    return ContainsConvex( box, inner );
}

/** True when the oriented box contains the triangle entirely */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Contains( const CTOrientedBox3D< T >& box, const CTTriangle3D< T >& inner ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return ContainsConvex( box, inner );
}

/** True when the oriented box contains the segment entirely */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Contains( const CTOrientedBox3D< T >& box, const CTSegment3D< T >& inner ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return ContainsConvex( box, inner );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/* For Float32 and Float64 the module's instantiations (gucefGEOMETRY_QueryInstantiations.cpp) are used, section 5.10 */

#define GUCEF_GEOMETRY_ORIENTEDBOXQUERIES_KIND( prefix, kind ) \
    prefix bool Intersects< kind >( const CTOrientedBox3D< kind >&, const CTOrientedBox3D< kind >&, const CTGeometryTolerance< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Contains< kind >( const CTOrientedBox3D< kind >&, const CTSphere< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Contains< kind >( const CTOrientedBox3D< kind >&, const CTAxisAlignedBox3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Contains< kind >( const CTOrientedBox3D< kind >&, const CTOrientedBox3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Contains< kind >( const CTOrientedBox3D< kind >&, const CTCapsule< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Contains< kind >( const CTOrientedBox3D< kind >&, const CTTriangle3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Contains< kind >( const CTOrientedBox3D< kind >&, const CTSegment3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

#define GUCEF_GEOMETRY_ORIENTEDBOXQUERIES_PREDEFINED_KINDS( prefix ) \
    GUCEF_GEOMETRY_ORIENTEDBOXQUERIES_KIND( prefix, Float32 ) \
    GUCEF_GEOMETRY_ORIENTEDBOXQUERIES_KIND( prefix, Float64 )

#ifdef GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED
GUCEF_GEOMETRY_ORIENTEDBOXQUERIES_PREDEFINED_KINDS( GUCEF_GEOMETRY_EXTERN_TEMPLATE )
#endif /* GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_ORIENTEDBOXQUERIES_H ? */
