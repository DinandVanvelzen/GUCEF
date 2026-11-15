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

#ifndef GUCEF_GEOMETRY_SRAYCASTRESULT_H
#define GUCEF_GEOMETRY_SRAYCASTRESULT_H

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

/** Bits of SRayCastResult::flags */
enum ERayCastFlags
{
    RAYHIT_NONE             = 0 ,
    RAYHIT_STARTED_INSIDE   = 1   /**< the ray started inside a solid shape: hit at distance 0, normal opposite to the ray direction */
};

/*-------------------------------------------------------------------------*/

/**
 *  The result of a ray cast (section 7.5.3 of the design). Valid only when the cast returned true.
 *  - distance: along the ray direction (the direction is a unit vector, so this is a length);
 *  - point: the hit position, origin + direction * distance;
 *  - normal: the unit surface normal at the hit, facing against the ray for planes and triangles (two-sided);
 *  - u, v: barycentric coordinates of the hit for triangles (the weights of the second and third vertex), 0 otherwise;
 *  - featureIndex: the shape-specific feature that was hit (axis of the box slab, 0 otherwise), -1 when unspecified;
 *  - flags: ERayCastFlags.
 *  A plain value type with public members, the default value is all zeros with featureIndex -1.
 */
template < typename T >
struct SRayCastResult
{
    typedef MATH::CTNumericTraits< T > TTraits;

    T distance;
    MATH::CTPoint3D< T > point;
    MATH::CTVector3D< T > normal;
    T u;
    T v;
    Int32 featureIndex;
    UInt32 flags;

    SRayCastResult( void )
        : distance( TTraits::Zero() ), point(), normal(), u( TTraits::Zero() ), v( TTraits::Zero() ), featureIndex( -1 ), flags( RAYHIT_NONE )
    {
    }

    /** True when the ray started inside the shape */
    bool StartedInside( void ) const { return 0 != ( flags & RAYHIT_STARTED_INSIDE ); }
};

typedef SRayCastResult< Float32 > TFloat32RayCastResult;
typedef SRayCastResult< Float64 > TFloat64RayCastResult;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_SRAYCASTRESULT_H ? */
