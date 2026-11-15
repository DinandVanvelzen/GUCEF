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

#ifndef GUCEF_GEOMETRY_SGEORAYHIT_H
#define GUCEF_GEOMETRY_SGEORAYHIT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_SRAYCASTRESULT_H
#include "gucefGEOMETRY_SRayCastResult.h"
#define GUCEF_GEOMETRY_SRAYCASTRESULT_H
#endif /* GUCEF_GEOMETRY_SRAYCASTRESULT_H ? */

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

template < typename T > class CTGeoObjectBase;

/** Bits of the ray hit flags beyond ERayCastFlags (the values continue the bits of ERayCastFlags) */
enum EGeoRayHitFlags
{
    RAYHIT_BACKFACE         = 2   /**< a triangle of a mesh was hit from behind (the ray runs along its counter-clockwise normal) */
};

/*-------------------------------------------------------------------------*/

/**
 *  The result of a ray cast against linked objects (section 7.7 of the design): the fields of SRayCastResult in world
 *  coordinates plus the object that was hit.
 *  - distance: world distance along the unit ray direction;
 *  - point: the world hit position; normal: the world unit normal facing the ray origin (opposite to the ray for a
 *    ray that started inside), held as a CTVector3D (unit length but not a CTDirectionVector3D, as in SRayCastResult);
 *  - u, v: barycentric weights of the second and third vertex for triangle hits, 0 otherwise;
 *  - featureIndex: an Int32 (as in SRayCastResult): the triangle index for meshes, the shape feature for other
 *    shapes, -1 when unspecified;
 *  - flags: ERayCastFlags and EGeoRayHitFlags bits (RAYHIT_STARTED_INSIDE, RAYHIT_BACKFACE);
 *  - object: the object hit, GUCEF_NULL in the default value.
 *  A plain value type with public members.
 */
template < typename T >
struct SGeoRayHit
{
    typedef MATH::CTNumericTraits< T > TTraits;

    T distance;
    MATH::CTPoint3D< T > point;
    MATH::CTVector3D< T > normal;
    T u;
    T v;
    Int32 featureIndex;
    UInt32 flags;
    CTGeoObjectBase< T >* object;

    SGeoRayHit( void )
        : distance( TTraits::Zero() ), point(), normal(), u( TTraits::Zero() ), v( TTraits::Zero() ), featureIndex( -1 ), flags( RAYHIT_NONE ), object( GUCEF_NULL )
    {
    }

    /** The hit of a shape ray cast (world coordinates) on 'hitObject' */
    SGeoRayHit( const SRayCastResult< T >& hit, CTGeoObjectBase< T >* hitObject )
        : distance( hit.distance ), point( hit.point ), normal( hit.normal ), u( hit.u ), v( hit.v ), featureIndex( hit.featureIndex ), flags( hit.flags ), object( hitObject )
    {
    }

    /** True when the ray started inside the shape */
    bool StartedInside( void ) const { return 0 != ( flags & RAYHIT_STARTED_INSIDE ); }

    /** True when a mesh triangle was hit from behind */
    bool IsBackFace( void ) const { return 0 != ( flags & RAYHIT_BACKFACE ); }
};

typedef SGeoRayHit< Float32 > TFloat32GeoRayHit;
typedef SGeoRayHit< Float64 > TFloat64GeoRayHit;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_SGEORAYHIT_H ? */
