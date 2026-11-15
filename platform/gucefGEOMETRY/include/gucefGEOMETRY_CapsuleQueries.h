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

#ifndef GUCEF_GEOMETRY_CAPSULEQUERIES_H
#define GUCEF_GEOMETRY_CAPSULEQUERIES_H

/*
 *  The capsule cells of the query matrix (section 7.5.1 of the design): capsule against capsule, analytic through the
 *  nearest points of the two core segments. The cells of a capsule with a box are the GJK cells of the convex query
 *  package; those with a segment, triangle and sphere are in the headers of those shapes. See
 *  gucefGEOMETRY_PointQueries.h for the conventions of the facade.
 *
 *  A capsule with a negative radius has no points: it intersects nothing.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_SPHEREQUERIES_H
#include "gucefGEOMETRY_SphereQueries.h"
#define GUCEF_GEOMETRY_SPHEREQUERIES_H
#endif /* GUCEF_GEOMETRY_SPHEREQUERIES_H ? */

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

/** The nearest points of two solid capsules; false for a negative radius */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
ClosestPoints( const CTCapsule< T >& a, const CTCapsule< T >& b, MATH::CTPoint3D< T >& onA, MATH::CTPoint3D< T >& onB ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( MATH::CTNumericTraits< T >::IsNegative( a.radius ) || MATH::CTNumericTraits< T >::IsNegative( b.radius ) )
        return false;
    MATH::CTPoint3D< T > pa;
    MATH::CTPoint3D< T > pb;
    ClosestPoints( a.GetSegment(), b.GetSegment(), pa, pb );
    ClosestPointsOfRoundCores( pa, a.radius, pb, b.radius, onA, onB );
    return true;
}

/** True when the solid capsules share a point; false for a negative radius */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Intersects( const CTCapsule< T >& a, const CTCapsule< T >& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( MATH::CTNumericTraits< T >::IsNegative( a.radius ) || MATH::CTNumericTraits< T >::IsNegative( b.radius ) )
        return false;
    MATH::CTPoint3D< T > pa;
    MATH::CTPoint3D< T > pb;
    ClosestPoints( a.GetSegment(), b.GetSegment(), pa, pb );
    const T reach = a.radius + b.radius;
    return ( pb - pa ).GetSquaredLength() <= reach * reach;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/* For Float32 and Float64 the module's instantiations (gucefGEOMETRY_QueryInstantiations.cpp) are used, section 5.10 */

#define GUCEF_GEOMETRY_CAPSULEQUERIES_KIND( prefix, kind ) \
    prefix bool ClosestPoints< kind >( const CTCapsule< kind >&, const CTCapsule< kind >&, MATH::CTPoint3D< kind >&, MATH::CTPoint3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Intersects< kind >( const CTCapsule< kind >&, const CTCapsule< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

#define GUCEF_GEOMETRY_CAPSULEQUERIES_PREDEFINED_KINDS( prefix ) \
    GUCEF_GEOMETRY_CAPSULEQUERIES_KIND( prefix, Float32 ) \
    GUCEF_GEOMETRY_CAPSULEQUERIES_KIND( prefix, Float64 )

#ifdef GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED
GUCEF_GEOMETRY_CAPSULEQUERIES_PREDEFINED_KINDS( GUCEF_GEOMETRY_EXTERN_TEMPLATE )
#endif /* GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CAPSULEQUERIES_H ? */
