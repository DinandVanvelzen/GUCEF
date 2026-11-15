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

#ifndef GUCEF_GEOMETRY_SUPPORTFUNCTIONS_H
#define GUCEF_GEOMETRY_SUPPORTFUNCTIONS_H

/*
 *  The one implementation of the support points of the cylinder and the cone: the GetSupport members of the shapes and
 *  the support mappings of the convex algorithms (gucefGEOMETRY_SupportMappings.h) both call these functions, so that
 *  the point stays on the surface and maximises the dot product also for directions nearly parallel to the axis.
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

#ifndef GUCEF_MATH_POINTS_H
#include "gucefMATH_points.h"
#define GUCEF_MATH_POINTS_H
#endif /* GUCEF_MATH_POINTS_H ? */

#ifndef GUCEF_MATH_VECTORS_H
#include "gucefMATH_vectors.h"
#define GUCEF_MATH_VECTORS_H
#endif /* GUCEF_MATH_VECTORS_H ? */

#ifndef GUCEF_MATH_DIRECTIONS_H
#include "gucefMATH_directions.h"
#define GUCEF_MATH_DIRECTIONS_H
#endif /* GUCEF_MATH_DIRECTIONS_H ? */

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
 *  The unit vector of the part of 'direction' perpendicular to the unit 'axis': the direction is first scaled to its
 *  largest component, then projected twice (the second projection removes the rounding of the first). False when that
 *  part is not above the larger of the relative and absolute tolerance of the kind (the rounding noise of a direction
 *  parallel to the axis, whose normalisation would point anywhere): the direction is then treated as parallel.
 */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
GetPerpendicularUnit( const MATH::CTVector3D< T >& direction, const MATH::CTVector3D< T >& axis, MATH::CTDirectionVector3D< T >& unit ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    MATH::CTVector3D< T > scaled;
    if ( !direction.ScaleToUnitChebyshev( scaled ) )
        return false;
    MATH::CTVector3D< T > lateral = scaled - axis * scaled.Dot( axis );
    lateral = lateral - axis * lateral.Dot( axis );
    const T threshold = TTraits::Max( TTraits::DefaultRelativeTolerance(), TTraits::DefaultAbsoluteTolerance() );
    if ( !( threshold < lateral.GetChebyshevLength() ) )
        return false;
    return MATH::CTDirectionVector3D< T >::TryCreate( lateral, unit );
}

/*-------------------------------------------------------------------------*/

/** The rim point of a circle around 'centre' (radius, in the plane perpendicular to the unit axis) furthest along 'direction'; the centre when the direction is parallel to the axis */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE MATH::CTPoint3D< T >
GetRimSupport( const MATH::CTPoint3D< T >& centre, const MATH::CTVector3D< T >& axis, const T& radius, const MATH::CTVector3D< T >& direction ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    MATH::CTDirectionVector3D< T > unit;
    if ( !GetPerpendicularUnit( direction, axis, unit ) )
        return centre;
    /* the rim offset made perpendicular once more at full length, so the point stays in the plane of the circle */
    const MATH::CTVector3D< T > offset = unit * radius;
    return centre + ( offset - axis * offset.Dot( axis ) );
}

/*-------------------------------------------------------------------------*/

/** The point of the cylinder with the axis from 'start' to 'end' (unit axis given) furthest along 'direction': the cap on the side of the direction, then the rim */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE MATH::CTPoint3D< T >
GetCylinderSupport( const MATH::CTPoint3D< T >& start, const MATH::CTPoint3D< T >& end, const MATH::CTVector3D< T >& axis, const T& radius, const MATH::CTVector3D< T >& direction ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const MATH::CTPoint3D< T > cap = MATH::CTNumericTraits< T >::IsNegative( direction.Dot( axis ) ) ? start : end;
    return GetRimSupport( cap, axis, radius, direction );
}

/*-------------------------------------------------------------------------*/

/** The point of the cone (apex, base centre, unit axis, base radius) furthest along 'direction': the apex or the point of the base rim */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE MATH::CTPoint3D< T >
GetConeSupport( const MATH::CTPoint3D< T >& apex, const MATH::CTPoint3D< T >& base, const MATH::CTVector3D< T >& axis, const T& baseRadius, const MATH::CTVector3D< T >& direction ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const MATH::CTPoint3D< T > rim = GetRimSupport( base, axis, baseRadius, direction );
    return MATH::CTNumericTraits< T >::Zero() < direction.Dot( rim - apex ) ? rim : apex;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_SUPPORTFUNCTIONS_H ? */
