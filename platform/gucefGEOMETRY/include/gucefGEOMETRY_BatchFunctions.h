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


#ifndef GUCEF_GEOMETRY_BATCHFUNCTIONS_H
#define GUCEF_GEOMETRY_BATCHFUNCTIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_MATH_MATHBATCHTYPES_H
#include "gucefMATH_MathBatchTypes.h"
#define GUCEF_MATH_MATHBATCHTYPES_H
#endif /* GUCEF_MATH_MATHBATCHTYPES_H ? */

#ifndef GUCEF_GEOMETRY_SHAPES_H
#include "gucefGEOMETRY_shapes.h"
#define GUCEF_GEOMETRY_SHAPES_H
#endif /* GUCEF_GEOMETRY_SHAPES_H ? */

#ifndef GUCEF_GEOMETRY_VOLUMESHAPES_H
#include "gucefGEOMETRY_volumeshapes.h"
#define GUCEF_GEOMETRY_VOLUMESHAPES_H
#endif /* GUCEF_GEOMETRY_VOLUMESHAPES_H ? */

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

/*
 *      The batch kernels of gucefGEOMETRY (section 5.6 and 7.8.5 of the design). They are registered in the dispatcher
 *      of gucefMATH (MATH::CMathBatchDispatcher) with RegisterExternalKernel in the range reserved for this module (256
 *      to 319), lazily from CGeometryGlobal::Initialize(), and are removed again by owner when CGeometryGlobal is
 *      destroyed. The rules of MathBatchTypes.h hold for both kernels: pointers need the alignment of Float32 only,
 *      byte strides are at least the element size and need not be multiples of 4, in place is permitted (the output
 *      array may be the input array with the same stride; partial overlap is not), count 0 touches nothing.
 */

/** The kernels of this module, ids in the range MATHBATCHKERNEL_GEOMETRY_FIRST to MATHBATCHKERNEL_GEOMETRY_LAST */
enum EGeometryBatchKernel
{
    GEOMETRYBATCHKERNEL_TRANSFORM_AABBS_F32 = MATH::MATHBATCHKERNEL_GEOMETRY_FIRST ,    /**< "GEOMETRY.TransformAabbs.F32", SGeometryTransformAabbsF32Args */
    GEOMETRYBATCHKERNEL_CULL_AABBS_FRUSTUM_F32 ,                                        /**< "GEOMETRY.CullAabbsFrustum.F32", SGeometryCullAabbsFrustumF32Args */

    GEOMETRYBATCHKERNEL_COUNT
};
typedef enum EGeometryBatchKernel TGeometryBatchKernel;

/*-------------------------------------------------------------------------*/

/**
 *  TransformAabbs: out[ i ] is CTAxisAlignedBox3D< Float32 >::Transformed( matrix ) of in[ i ], bit for bit: the box of
 *  the eight transformed corners by the per axis minimum and maximum of the products (Arvo), summed in the order of the
 *  matrix columns; the bottom row of the matrix is not used (an affine matrix). An empty box (min greater than max on an
 *  axis, or a NaN component) is copied unchanged. A box is six Float32, min x y z then max x y z (24 bytes).
 */
struct SGeometryTransformAabbsF32Args
{
    const MATH::TFloat32Matrix4D* matrix;
    const void* in;
    UInt32 inStrideBytes;
    void* out;
    UInt32 outStrideBytes;
    UInt32 count;
};

/**
 *  CullAabbsFrustum: for each box one UInt8 with the value of TFrustumClassification, the result of
 *  ClassifyBoxToPlaneSet< Float32 > for the planes with a tolerance of zero: OUTSIDE (0) when the box lies entirely
 *  behind one plane or a number is not finite, INSIDE (2) when it is in front of all planes, otherwise INTERSECTING (1).
 *  The planes are 'planeCount' CTPlane< Float32 > (normal x y z, offset: 16 bytes each) with the normals into the
 *  region; no planes classify every box as INSIDE. An empty box is OUTSIDE. 'out' receives one byte per box.
 */
struct SGeometryCullAabbsFrustumF32Args
{
    const TFloat32Plane* planes;
    UInt32 planeCount;
    const void* in;
    UInt32 inStrideBytes;
    UInt8* out;
    UInt32 outStrideBytes;
    UInt32 count;
};

typedef MATH::TMathStatus ( *TGeometryTransformAabbsF32Function )( void* context, const SGeometryTransformAabbsF32Args& args );
typedef MATH::TMathStatus ( *TGeometryCullAabbsFrustumF32Function )( void* context, const SGeometryCullAabbsFrustumF32Args& args );

/*-------------------------------------------------------------------------//
//                                                                         //
//      ARGUMENT CHECKS                                                    //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Found by the dispatcher (CMathBatchDispatcher::Execute) through argument dependent lookup, see MathBatchTypes.h.
 */

GUCEF_GEOMETRY_FORCE_INLINE size_t
MathBatchAlignmentBits( const SGeometryTransformAabbsF32Args& a ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return (size_t) a.in | (size_t) a.out | a.inStrideBytes | a.outStrideBytes;
}

GUCEF_GEOMETRY_FORCE_INLINE bool
MathBatchArgumentsValid( MATH::TMathBatchKernel /* kernel */, const SGeometryTransformAabbsF32Args& a ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return GUCEF_NULL != a.matrix && MATH::MathBatchStrideValid( a.count, a.inStrideBytes, 24 ) && MATH::MathBatchStrideValid( a.count, a.outStrideBytes, 24 );
}

GUCEF_GEOMETRY_FORCE_INLINE size_t
MathBatchAlignmentBits( const SGeometryCullAabbsFrustumF32Args& a ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return (size_t) a.planes | (size_t) a.in | a.inStrideBytes;
}

GUCEF_GEOMETRY_FORCE_INLINE bool
MathBatchArgumentsValid( MATH::TMathBatchKernel /* kernel */, const SGeometryCullAabbsFrustumF32Args& a ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return ( 0 == a.planeCount || GUCEF_NULL != a.planes ) && MATH::MathBatchStrideValid( a.count, a.inStrideBytes, 24 ) && MATH::MathBatchStrideValid( a.count, a.outStrideBytes, 1 );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The boxes of 'in' transformed by an affine matrix into 'out' (arrays of 'count' boxes, in place allowed) through the
 *  best provider: the dispatcher of MATH::CMathGlobal. 'flags' are MATH::TMathBatchFlags. Returns MATHSTATUS_OK, or
 *  MATHSTATUS_INVALID_OPERAND when no provider is eligible.
 */
GUCEF_GEOMETRY_PUBLIC_CPP MATH::TMathStatus
TransformAabbs( const MATH::TFloat32Matrix4D& matrix, const TFloat32AxisAlignedBox3D* in, TFloat32AxisAlignedBox3D* out, UInt32 count, UInt32 flags = 0 );

/** The classification of 'count' boxes against 'planeCount' planes, one UInt8 of TFrustumClassification per box into 'out' */
GUCEF_GEOMETRY_PUBLIC_CPP MATH::TMathStatus
CullAabbsFrustum( const TFloat32Plane* planes, UInt32 planeCount, const TFloat32AxisAlignedBox3D* boxes, UInt8* out, UInt32 count, UInt32 flags = 0 );

/** As above with the six planes of a frustum */
GUCEF_GEOMETRY_PUBLIC_CPP MATH::TMathStatus
CullAabbsFrustum( const TFloat32Frustum& frustum, const TFloat32AxisAlignedBox3D* boxes, UInt8* out, UInt32 count, UInt32 flags = 0 );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_BATCHFUNCTIONS_H ? */
