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

#ifndef GUCEF_GEOMETRY_SPHEREQUERIES_H
#define GUCEF_GEOMETRY_SPHEREQUERIES_H

/*
 *  The sphere cells of the query matrix (sections 7.5.1 and 7.5.5 of the design): a sphere against a sphere,
 *  capsule, axis aligned box and oriented box (distance, nearest points and overlap), and the containment of shapes in
 *  a sphere. The sphere against a box is the clamp test of Arvo: the sphere reaches the box when the nearest point of the
 *  box to the centre is within the radius. See gucefGEOMETRY_PointQueries.h for the conventions of the facade.
 *
 *  A sphere with a negative radius has no points: it intersects nothing and contains nothing.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_TRIANGLEQUERIES_H
#include "gucefGEOMETRY_TriangleQueries.h"
#define GUCEF_GEOMETRY_TRIANGLEQUERIES_H
#endif /* GUCEF_GEOMETRY_TRIANGLEQUERIES_H ? */

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

/** The nearest points of two solid spheres; false for a negative radius */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
ClosestPoints( const CTSphere< T >& a, const CTSphere< T >& b, MATH::CTPoint3D< T >& onA, MATH::CTPoint3D< T >& onB ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    if ( TTraits::IsNegative( a.radius ) || TTraits::IsNegative( b.radius ) )
        return false;
    ClosestPointsOfRoundCores( a.centre, a.radius, b.centre, b.radius, onA, onB );
    return true;
}

/*-------------------------------------------------------------------------*/

/** The nearest points of a solid sphere and a solid capsule; false for a negative radius */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
ClosestPoints( const CTSphere< T >& sphere, const CTCapsule< T >& capsule, MATH::CTPoint3D< T >& onSphere, MATH::CTPoint3D< T >& onCapsule ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    if ( TTraits::IsNegative( sphere.radius ) || TTraits::IsNegative( capsule.radius ) )
        return false;
    ClosestPointsOfRoundCores( sphere.centre, sphere.radius, capsule.GetSegment().ClosestPoint( sphere.centre ), capsule.radius, onSphere, onCapsule );
    return true;
}

/*-------------------------------------------------------------------------*/

/** The nearest points of a solid sphere and a solid box; false for a negative radius or an empty box */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
ClosestPoints( const CTSphere< T >& sphere, const CTAxisAlignedBox3D< T >& box, MATH::CTPoint3D< T >& onSphere, MATH::CTPoint3D< T >& onBox ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    if ( TTraits::IsNegative( sphere.radius ) || box.IsEmpty() )
        return false;
    ClosestPointsOfRoundCores( sphere.centre, sphere.radius, box.ClosestPoint( sphere.centre ), TTraits::Zero(), onSphere, onBox );
    return true;
}

/*-------------------------------------------------------------------------*/

/** The nearest points of a solid sphere and a solid oriented box; false for a negative radius or an invalid box */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
ClosestPoints( const CTSphere< T >& sphere, const CTOrientedBox3D< T >& box, MATH::CTPoint3D< T >& onSphere, MATH::CTPoint3D< T >& onBox ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    if ( TTraits::IsNegative( sphere.radius ) || !box.IsValid() )
        return false;
    ClosestPointsOfRoundCores( sphere.centre, sphere.radius, box.ClosestPoint( sphere.centre ), TTraits::Zero(), onSphere, onBox );
    return true;
}

/*-------------------------------------------------------------------------*/

/** True when the solid spheres share a point; false for a negative radius */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Intersects( const CTSphere< T >& a, const CTSphere< T >& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( MATH::CTNumericTraits< T >::IsNegative( a.radius ) || MATH::CTNumericTraits< T >::IsNegative( b.radius ) )
        return false;
    const T reach = a.radius + b.radius;
    return ( b.centre - a.centre ).GetSquaredLength() <= reach * reach;
}

/** True when the solid sphere and the solid capsule share a point; false for a negative radius */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Intersects( const CTSphere< T >& sphere, const CTCapsule< T >& capsule ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( MATH::CTNumericTraits< T >::IsNegative( sphere.radius ) || MATH::CTNumericTraits< T >::IsNegative( capsule.radius ) )
        return false;
    const T reach = sphere.radius + capsule.radius;
    return capsule.GetSegment().GetSquaredDistance( sphere.centre ) <= reach * reach;
}

/** True when the solid sphere and the box share a point (the clamp test of Arvo); false for a negative radius or an empty box */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Intersects( const CTSphere< T >& sphere, const CTAxisAlignedBox3D< T >& box ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( MATH::CTNumericTraits< T >::IsNegative( sphere.radius ) || box.IsEmpty() )
        return false;
    return box.GetSquaredDistance( sphere.centre ) <= sphere.radius * sphere.radius;
}

/** True when the solid sphere and the oriented box share a point (the clamp test in the frame of the box); false for a negative radius or an invalid box */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Intersects( const CTSphere< T >& sphere, const CTOrientedBox3D< T >& box ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( MATH::CTNumericTraits< T >::IsNegative( sphere.radius ) || !box.IsValid() )
        return false;
    return box.GetSquaredDistance( sphere.centre ) <= sphere.radius * sphere.radius;
}

/*-------------------------------------------------------------------------*/

/** True when the sphere contains the other sphere entirely (equal spheres contain each other) */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Contains( const CTSphere< T >& outer, const CTSphere< T >& inner ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( MATH::CTNumericTraits< T >::IsNegative( outer.radius ) || MATH::CTNumericTraits< T >::IsNegative( inner.radius ) )
        return false;
    return ( inner.centre - outer.centre ).GetLength() + inner.radius <= outer.radius;
}

/** True when the sphere contains the box entirely: its farthest corner is within the radius; false for an empty box */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Contains( const CTSphere< T >& outer, const CTAxisAlignedBox3D< T >& box ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    if ( TTraits::IsNegative( outer.radius ) || box.IsEmpty() )
        return false;
    T squared = TTraits::Zero();
    for ( UInt32 axis=0; axis<3; ++axis )
    {
        const T c = outer.centre.GetComponent( axis );
        const T farthest = TTraits::Max( TTraits::Abs( c - box.min.GetComponent( axis ) ), TTraits::Abs( c - box.max.GetComponent( axis ) ) );
        squared = squared + farthest * farthest;
    }
    return squared <= outer.radius * outer.radius;
}

/** True when the sphere contains the oriented box entirely: its farthest corner is within the radius; false for an invalid box */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Contains( const CTSphere< T >& outer, const CTOrientedBox3D< T >& box ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    if ( TTraits::IsNegative( outer.radius ) || !box.IsValid() )
        return false;
    MATH::CTPoint3D< T > corners[ 8 ];
    box.GetCorners( corners );
    const T limit = outer.radius * outer.radius;
    for ( UInt32 i=0; i<8; ++i )
    {
        if ( limit < ( corners[ i ] - outer.centre ).GetSquaredLength() )
            return false;
    }
    return true;
}

/** True when the sphere contains all 'count' points (true for none) */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Contains( const CTSphere< T >& outer, const MATH::CTPoint3D< T >* points, UInt32 count ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    for ( UInt32 i=0; i<count; ++i )
    {
        if ( !outer.Contains( points[ i ] ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_SWAPPED_INTERSECTS( CTSphere, CTCapsule )
GUCEF_GEOMETRY_SWAPPED_INTERSECTS( CTSphere, CTAxisAlignedBox3D )
GUCEF_GEOMETRY_SWAPPED_INTERSECTS( CTSphere, CTOrientedBox3D )
GUCEF_GEOMETRY_SWAPPED_CLOSESTPOINTS( CTSphere, CTCapsule )
GUCEF_GEOMETRY_SWAPPED_CLOSESTPOINTS( CTSphere, CTAxisAlignedBox3D )
GUCEF_GEOMETRY_SWAPPED_CLOSESTPOINTS( CTSphere, CTOrientedBox3D )

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/* For Float32 and Float64 the module's instantiations (gucefGEOMETRY_QueryInstantiations.cpp) are used, section 5.10 */

#define GUCEF_GEOMETRY_SPHEREQUERIES_KIND( prefix, kind ) \
    prefix bool ClosestPoints< kind >( const CTSphere< kind >&, const CTSphere< kind >&, MATH::CTPoint3D< kind >&, MATH::CTPoint3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool ClosestPoints< kind >( const CTSphere< kind >&, const CTCapsule< kind >&, MATH::CTPoint3D< kind >&, MATH::CTPoint3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool ClosestPoints< kind >( const CTSphere< kind >&, const CTAxisAlignedBox3D< kind >&, MATH::CTPoint3D< kind >&, MATH::CTPoint3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool ClosestPoints< kind >( const CTSphere< kind >&, const CTOrientedBox3D< kind >&, MATH::CTPoint3D< kind >&, MATH::CTPoint3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Intersects< kind >( const CTSphere< kind >&, const CTSphere< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Intersects< kind >( const CTSphere< kind >&, const CTCapsule< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Intersects< kind >( const CTSphere< kind >&, const CTAxisAlignedBox3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Intersects< kind >( const CTSphere< kind >&, const CTOrientedBox3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Contains< kind >( const CTSphere< kind >&, const CTSphere< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Contains< kind >( const CTSphere< kind >&, const CTAxisAlignedBox3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Contains< kind >( const CTSphere< kind >&, const CTOrientedBox3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Contains< kind >( const CTSphere< kind >&, const MATH::CTPoint3D< kind >*, UInt32 ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

#define GUCEF_GEOMETRY_SPHEREQUERIES_PREDEFINED_KINDS( prefix ) \
    GUCEF_GEOMETRY_SPHEREQUERIES_KIND( prefix, Float32 ) \
    GUCEF_GEOMETRY_SPHEREQUERIES_KIND( prefix, Float64 )

#ifdef GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED
GUCEF_GEOMETRY_SPHEREQUERIES_PREDEFINED_KINDS( GUCEF_GEOMETRY_EXTERN_TEMPLATE )
#endif /* GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_SPHEREQUERIES_H ? */
