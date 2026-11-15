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

#ifndef GUCEF_GEOMETRY_FRUSTUMCULLING_H
#define GUCEF_GEOMETRY_FRUSTUMCULLING_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_GEOMETRY_ETYPES_H
#include "gucefGEOMETRY_ETypes.h"
#define GUCEF_GEOMETRY_ETYPES_H
#endif /* GUCEF_GEOMETRY_ETYPES_H ? */

#ifndef GUCEF_GEOMETRY_CTPLANE_H
#include "gucefGEOMETRY_CTPlane.h"
#define GUCEF_GEOMETRY_CTPLANE_H
#endif /* GUCEF_GEOMETRY_CTPLANE_H ? */

#ifndef GUCEF_GEOMETRY_CTFRUSTUM_H
#include "gucefGEOMETRY_CTFrustum.h"
#define GUCEF_GEOMETRY_CTFRUSTUM_H
#endif /* GUCEF_GEOMETRY_CTFRUSTUM_H ? */

#ifndef GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H
#include "gucefGEOMETRY_CTAxisAlignedBox3D.h"
#define GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H
#endif /* GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H ? */

#ifndef GUCEF_GEOMETRY_PLANESETFUNCTIONS_H
#include "gucefGEOMETRY_PlaneSetFunctions.h"
#define GUCEF_GEOMETRY_PLANESETFUNCTIONS_H
#endif /* GUCEF_GEOMETRY_PLANESETFUNCTIONS_H ? */

#ifndef GUCEF_GEOMETRY_BATCHFUNCTIONS_H
#include "gucefGEOMETRY_BatchFunctions.h"
#define GUCEF_GEOMETRY_BATCHFUNCTIONS_H
#endif /* GUCEF_GEOMETRY_BATCHFUNCTIONS_H ? */

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

/*
 *  Frustum culling (section 7.8.5 of the design): box against a set of planes with the p-vertex / n-vertex test of
 *  ClassifyBoxToPlaneSet (the radius of the box along the plane normal against the distance of its centre) and plane
 *  mask coherence for hierarchies: a node that lies entirely in front of a plane has all its children in front of it,
 *  so the plane is dropped from the mask handed to the children.
 */

/**
 *  OUTSIDE when the closed box 'lo' to 'hi' lies entirely behind one of the planes whose bit is set in 'mask' (or its
 *  numbers are not finite), INSIDE when it is in front of every one of them, otherwise INTERSECTING. 'mask' holds the
 *  planes still to be tested (bit i for plane i, 32 planes at most): the bit of every plane the box lies entirely in
 *  front of is cleared, so a node's mask is the mask to pass for its children; a mask that is 0 on return means INSIDE.
 *  'slack' widens the region by that length (0 for the exact classification of ClassifyBoxToPlaneSet; a positive value
 *  never reports OUTSIDE for a box that the exact test accepts).
 */
template < typename T >
TFrustumClassification
ClassifyBoxLowHighWithMask( const CTPlane< T >* planes, UInt32 planeCount, const T* lo, const T* hi, UInt32& mask, const T& slack ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    T centre[ 3 ];
    T half[ 3 ];
    for ( UInt32 a=0; a<3; ++a )
    {
        if ( !TTraits::IsFinite( lo[ a ] ) || !TTraits::IsFinite( hi[ a ] ) || !( lo[ a ] <= hi[ a ] ) )
            return FRUSTUMCLASSIFICATION_OUTSIDE;
        const T extent = hi[ a ] - lo[ a ];
        centre[ a ] = lo[ a ] + extent * TTraits::Half();
        half[ a ] = extent * TTraits::Half();
        if ( !TTraits::IsFinite( centre[ a ] ) || !TTraits::IsFinite( half[ a ] ) )
            return FRUSTUMCLASSIFICATION_OUTSIDE;
    }
    bool straddles = false;
    UInt32 remaining = mask;
    for ( UInt32 i=0; i<planeCount && i<32; ++i )
    {
        if ( 0 == ( mask & ( (UInt32) 1 << i ) ) )
            continue;
        const T nx = planes[ i ].normal.AsVector().x;
        const T ny = planes[ i ].normal.AsVector().y;
        const T nz = planes[ i ].normal.AsVector().z;
        const T radius = ( TTraits::Abs( nx ) * half[ 0 ] + TTraits::Abs( ny ) * half[ 1 ] ) + TTraits::Abs( nz ) * half[ 2 ];
        const T distance = ( ( nx * centre[ 0 ] + ny * centre[ 1 ] ) + nz * centre[ 2 ] ) - planes[ i ].offset;
        if ( !TTraits::IsFinite( distance ) || !TTraits::IsFinite( radius ) )
            return FRUSTUMCLASSIFICATION_OUTSIDE;
        if ( distance < MATH::ComponentNegate( radius ) - slack )
            return FRUSTUMCLASSIFICATION_OUTSIDE;
        if ( distance < radius - slack )
            straddles = true;
        else
            remaining &= ~( (UInt32) 1 << i );
    }
    mask = remaining;
    return straddles ? FRUSTUMCLASSIFICATION_INTERSECTING : FRUSTUMCLASSIFICATION_INSIDE;
}

/*-------------------------------------------------------------------------*/

/** The classification of a box against the six planes of a frustum with plane mask coherence: start with mask 0x3F, pass the mask of a node on to its children */
template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE TFrustumClassification
ClassifyBoxWithMask( const CTFrustum< T >& frustum, const CTAxisAlignedBox3D< T >& box, UInt32& mask ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const T lo[ 3 ] = { box.min.x, box.min.y, box.min.z };
    const T hi[ 3 ] = { box.max.x, box.max.y, box.max.z };
    return ClassifyBoxLowHighWithMask( frustum.GetPlanes(), 6, lo, hi, mask, MATH::CTNumericTraits< T >::Zero() );
}

/*-------------------------------------------------------------------------*/

/**
 *  The classification (a TFrustumClassification per box into 'out') of 'count' boxes against a frustum. For Float32 the
 *  batch kernel "GEOMETRY.CullAabbsFrustum.F32" of the dispatcher of gucefMATH runs it (SIMD where the CPU has it),
 *  with a result equal to Classify of the frustum for each box; an empty box is OUTSIDE. Not realtime-safe unless the
 *  flags select realtime providers (MATH::MATHBATCH_REALTIME).
 */
GUCEF_GEOMETRY_FORCE_INLINE void
CullBoxes( const TFloat32Frustum& frustum, const TFloat32AxisAlignedBox3D* boxes, UInt32 count, UInt8* out, UInt32 flags = 0 )
{
    if ( MATH::MATHSTATUS_OK == CullAabbsFrustum( frustum, boxes, out, count, flags ) )
        return;
    for ( UInt32 i=0; i<count; ++i )
        out[ i ] = (UInt8) ( boxes[ i ].IsEmpty() ? FRUSTUMCLASSIFICATION_OUTSIDE : frustum.Classify( boxes[ i ] ) );
}

/** The same for Float64 boxes, one at a time */
GUCEF_GEOMETRY_FORCE_INLINE void
CullBoxes( const TFloat64Frustum& frustum, const TFloat64AxisAlignedBox3D* boxes, UInt32 count, UInt8* out, UInt32 /* flags */ = 0 )
{
    for ( UInt32 i=0; i<count; ++i )
        out[ i ] = (UInt8) ( boxes[ i ].IsEmpty() ? FRUSTUMCLASSIFICATION_OUTSIDE : frustum.Classify( boxes[ i ] ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_FRUSTUMCULLING_H ? */
