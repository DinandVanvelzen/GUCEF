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

#ifndef GUCEF_GEOMETRY_CTGEOSPACESNAPSHOT_H
#define GUCEF_GEOMETRY_CTGEOSPACESNAPSHOT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_MATH_MATRICES_H
#include "gucefMATH_matrices.h"
#define GUCEF_MATH_MATRICES_H
#endif /* GUCEF_MATH_MATRICES_H ? */

#ifndef GUCEF_MATH_QUATERNIONS_H
#include "gucefMATH_quaternions.h"
#define GUCEF_MATH_QUATERNIONS_H
#endif /* GUCEF_MATH_QUATERNIONS_H ? */

#ifndef GUCEF_MATH_POINTS_H
#include "gucefMATH_points.h"
#define GUCEF_MATH_POINTS_H
#endif /* GUCEF_MATH_POINTS_H ? */

#ifndef GUCEF_MATH_DIRECTIONS_H
#include "gucefMATH_directions.h"
#define GUCEF_MATH_DIRECTIONS_H
#endif /* GUCEF_MATH_DIRECTIONS_H ? */

#ifndef GUCEF_MATH_VECTORS_H
#include "gucefMATH_vectors.h"
#define GUCEF_MATH_VECTORS_H
#endif /* GUCEF_MATH_VECTORS_H ? */

#ifndef GUCEF_MATH_FIXEDPOINT_H
#include "gucefMATH_fixedpoint.h"
#define GUCEF_MATH_FIXEDPOINT_H
#endif /* GUCEF_MATH_FIXEDPOINT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The world placement of one CTGeoSpace node, captured on the owner thread for readers on other threads (section
 *  7.3.6 of the design): plain data, copyable by assignment, no heap, no virtual functions, so it can be published
 *  through MT::CTSlotDataExchange< CTGeoSpaceSnapshot< T >, MT::SSlotDeliveryLatestOnly > (the exchange belongs to the
 *  consumer, which chooses its delivery policy).
 *
 *  - Every member comes from one state of the node: CTGeoSpace::CaptureSnapshot fills them together.
 *  - forward, up and right are the axes of the coordinate system given to the capture, rotated by worldRotation:
 *    orthonormal, and exactly the node's axes when its world matrix has no shear.
 *  - velocity is set by an owner that tracks it (the capture overload with a velocity), zero otherwise.
 *  - Kinds: Float32, Float64 and TFixed32_32 are exported; CaptureSnapshotRelativeTo gives a snapshot of another kind
 *    relative to an origin (Float32 snapshots of a Float64 tree for camera-relative rendering and audio).
 *  - The default snapshot is the identity placement with the GUCEF default axes (right +X, up +Y, forward -Z).
 */
template < typename T >
struct GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTGeoSpaceSnapshot
{
    MATH::CTMatrix4D< T > worldMatrix;
    MATH::CTPoint3D< T > worldPosition;
    MATH::CTQuaternion< T > worldRotation;
    MATH::CTDirectionVector3D< T > forward;
    MATH::CTDirectionVector3D< T > up;
    MATH::CTDirectionVector3D< T > right;
    MATH::CTVector3D< T > velocity;

    CTGeoSpaceSnapshot( void )
        : worldMatrix(), worldPosition(), worldRotation(), forward( MATH::CTDirectionVector3D< T >::Forward() ), up( MATH::CTDirectionVector3D< T >::Up() ),
          right( MATH::CTDirectionVector3D< T >::Right() ), velocity()
    {
    }

    /** True when every member equals the other's bit for bit (the comparison of the published value) */
    bool operator==( const CTGeoSpaceSnapshot& other ) const
    {
        return worldMatrix == other.worldMatrix && worldPosition == other.worldPosition && worldRotation == other.worldRotation && forward == other.forward &&
               up == other.up && right == other.right && velocity == other.velocity;
    }

    bool operator!=( const CTGeoSpaceSnapshot& other ) const { return !( *this == other ); }
};

typedef CTGeoSpaceSnapshot< Float32 > TFloat32GeoSpaceSnapshot;
typedef CTGeoSpaceSnapshot< Float64 > TFloat64GeoSpaceSnapshot;
typedef CTGeoSpaceSnapshot< MATH::TFixed32_32 > TFixed32_32GeoSpaceSnapshot;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTGeoSpaceSnapshot< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTGeoSpaceSnapshot< Float64 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTGeoSpaceSnapshot< MATH::TFixed32_32 > )

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTGEOSPACESNAPSHOT_H ? */
