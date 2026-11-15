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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_MATH_SIMD_CONFIG_H
#include "gucefMATH_simd_config.h"
#define GUCEF_MATH_SIMD_CONFIG_H
#endif /* GUCEF_MATH_SIMD_CONFIG_H ? */

#if ( GUCEF_MATH_SIMD_HAVE_SSE2 == 1 )
  #include "gucefMATH_CTVector4Ops_Sse2.h"
#endif
#if ( GUCEF_MATH_SIMD_HAVE_NEON == 1 )
  #include "gucefMATH_CTVector4Ops_Neon.h"
#endif
#if ( GUCEF_MATH_SIMD_HAVE_WASM128 == 1 )
  #include "gucefMATH_CTVector4Ops_Wasm128.h"
#endif

#ifndef GUCEF_MATH_CMATHBATCHDISPATCHER_H
#include "gucefMATH_CMathBatchDispatcher.h"
#define GUCEF_MATH_CMATHBATCHDISPATCHER_H
#endif /* GUCEF_MATH_CMATHBATCHDISPATCHER_H ? */

#ifndef GUCEF_MATH_CMATHGLOBAL_H
#include "gucefMATH_CMathGlobal.h"
#define GUCEF_MATH_CMATHGLOBAL_H
#endif /* GUCEF_MATH_CMATHGLOBAL_H ? */

#ifndef GUCEF_GEOMETRY_CGEOMETRYGLOBAL_H
#include "gucefGEOMETRY_CGeometryGlobal.h"
#define GUCEF_GEOMETRY_CGEOMETRYGLOBAL_H
#endif /* GUCEF_GEOMETRY_CGEOMETRYGLOBAL_H ? */

#include "gucefGEOMETRY_BatchFunctions.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The planes of a call are copied into lane order (structure of arrays, four planes per lane group) by the SIMD
 *  provider: at most this many planes (a CTPlaneBoundedVolume holds 32); beyond it the provider reports a failure
 *  and the dispatcher continues with the scalar provider.
 */
static const UInt32 GeometrySimdMaxPlanes = 32;

/*-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_FORCE_INLINE static bool
IsFiniteFloat32( Float32 x ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return x == x && x - x == 0.0f;
}

/*-------------------------------------------------------------------------*/

/** The element operation of TransformAabbs: the order of CTAxisAlignedBox3D::Transformed. 'in' and 'out' may be the same. */
static void
TransformAabbElement( const Float32* m, const Float32* in, Float32* out ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const Float32 mn[ 3 ] = { in[ 0 ], in[ 1 ], in[ 2 ] };
    const Float32 mx[ 3 ] = { in[ 3 ], in[ 4 ], in[ 5 ] };
    if ( !( mn[ 0 ] <= mx[ 0 ] && mn[ 1 ] <= mx[ 1 ] && mn[ 2 ] <= mx[ 2 ] ) )
    {
        if ( in != out )
            memcpy( out, in, 6 * sizeof( Float32 ) );
        return;
    }
    Float32 low[ 3 ];
    Float32 high[ 3 ];
    for ( UInt32 i=0; i<3; ++i )
    {
        Float32 l = m[ 12 + i ];
        Float32 h = l;
        for ( UInt32 j=0; j<3; ++j )
        {
            const Float32 e = m[ 4 * j + i ];
            const Float32 a = e * mn[ j ];
            const Float32 b = e * mx[ j ];
            l = l + ( b < a ? b : a );
            h = h + ( a < b ? b : a );
        }
        low[ i ] = l;
        high[ i ] = h;
    }
    out[ 0 ] = low[ 0 ]; out[ 1 ] = low[ 1 ]; out[ 2 ] = low[ 2 ];
    out[ 3 ] = high[ 0 ]; out[ 4 ] = high[ 1 ]; out[ 5 ] = high[ 2 ];
}

/*-------------------------------------------------------------------------*/

/**
 *  The classification of one box (its six Float32 read as bytes) against planes: the arithmetic of
 *  ClassifyBoxToPlaneSet< Float32 > with a tolerance of zero; the empty box is OUTSIDE.
 */
static UInt8
CullAabbElement( const TFloat32Plane* planes, UInt32 planeCount, const Float32* box ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( !( box[ 0 ] <= box[ 3 ] && box[ 1 ] <= box[ 4 ] && box[ 2 ] <= box[ 5 ] ) )
        return (UInt8) FRUSTUMCLASSIFICATION_OUTSIDE;
    const Float32 ex = box[ 3 ] - box[ 0 ];
    const Float32 ey = box[ 4 ] - box[ 1 ];
    const Float32 ez = box[ 5 ] - box[ 2 ];
    const Float32 cx = box[ 0 ] + ex * 0.5f;
    const Float32 cy = box[ 1 ] + ey * 0.5f;
    const Float32 cz = box[ 2 ] + ez * 0.5f;
    const Float32 hx = ex * 0.5f;
    const Float32 hy = ey * 0.5f;
    const Float32 hz = ez * 0.5f;
    if ( !IsFiniteFloat32( cx ) || !IsFiniteFloat32( cy ) || !IsFiniteFloat32( cz ) || !IsFiniteFloat32( hx ) || !IsFiniteFloat32( hy ) || !IsFiniteFloat32( hz ) )
        return (UInt8) FRUSTUMCLASSIFICATION_OUTSIDE;
    bool straddles = false;
    for ( UInt32 i=0; i<planeCount; ++i )
    {
        const Float32* p = reinterpret_cast< const Float32* >( planes + i );
        const Float32 nx = p[ 0 ], ny = p[ 1 ], nz = p[ 2 ], d = p[ 3 ];
        const Float32 ax = nx < 0.0f ? -nx : nx;
        const Float32 ay = ny < 0.0f ? -ny : ny;
        const Float32 az = nz < 0.0f ? -nz : nz;
        const Float32 radius = ( ax * hx + ay * hy ) + az * hz;
        const Float32 distance = ( ( nx * cx + ny * cy ) + nz * cz ) - d;
        if ( !IsFiniteFloat32( distance ) || !IsFiniteFloat32( radius ) )
            return (UInt8) FRUSTUMCLASSIFICATION_OUTSIDE;
        if ( distance < -radius )
            return (UInt8) FRUSTUMCLASSIFICATION_OUTSIDE;
        if ( distance < radius )
            straddles = true;
    }
    return (UInt8) ( straddles ? FRUSTUMCLASSIFICATION_INTERSECTING : FRUSTUMCLASSIFICATION_INSIDE );
}

/*-------------------------------------------------------------------------*/

static MATH::TMathStatus
TransformAabbsScalar( void* /* context */, const SGeometryTransformAabbsF32Args& args ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const Float32* m = args.matrix->GetData();
    const UInt8* in = static_cast< const UInt8* >( args.in );
    UInt8* out = static_cast< UInt8* >( args.out );
    for ( UInt32 i=0; i<args.count; ++i )
    {
        Float32 box[ 6 ];
        memcpy( box, in + (size_t) i * args.inStrideBytes, sizeof( box ) );
        TransformAabbElement( m, box, box );
        memcpy( out + (size_t) i * args.outStrideBytes, box, sizeof( box ) );
    }
    return MATH::MATHSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

static MATH::TMathStatus
CullAabbsScalar( void* /* context */, const SGeometryCullAabbsFrustumF32Args& args ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const UInt8* in = static_cast< const UInt8* >( args.in );
    for ( UInt32 i=0; i<args.count; ++i )
    {
        Float32 box[ 6 ];
        memcpy( box, in + (size_t) i * args.inStrideBytes, sizeof( box ) );
        args.out[ (size_t) i * args.outStrideBytes ] = CullAabbElement( args.planes, args.planeCount, box );
    }
    return MATH::MATHSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

/**
 *  The SIMD providers over a lane policy L (the lane policy of the gucefMATH batch kernels, see gucefMATH_CTVector4Ops_Sse2.h):
 *  every lane computes with the operations and the order of the scalar element operation, so the results are bit
 *  identical (no fused multiply-add: the module is compiled with contraction off).
 */
template < class L >
class CGeometryBatchSimd
{
    public:

    typedef typename L::TVec TVec;

    /** Lane i of the result is row i of the transformed box: the columns of the matrix are the lane vectors */
    static MATH::TMathStatus
    TransformAabbs( void* /* context */, const SGeometryTransformAabbsF32Args& args ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        GUCEF_MATH_FP_CONTRACT_OFF
        const Float32* m = args.matrix->GetData();
        const TVec c0 = L::Load4( m );
        const TVec c1 = L::Load4( m + 4 );
        const TVec c2 = L::Load4( m + 8 );
        const TVec c3 = L::Load4( m + 12 );
        const UInt8* in = static_cast< const UInt8* >( args.in );
        UInt8* out = static_cast< UInt8* >( args.out );
        for ( UInt32 i=0; i<args.count; ++i )
        {
            Float32 box[ 6 ];
            memcpy( box, in + (size_t) i * args.inStrideBytes, sizeof( box ) );
            UInt8* o = out + (size_t) i * args.outStrideBytes;
            if ( !( box[ 0 ] <= box[ 3 ] && box[ 1 ] <= box[ 4 ] && box[ 2 ] <= box[ 5 ] ) )
            {
                memcpy( o, box, sizeof( box ) );
                continue;
            }
            TVec low = c3;
            TVec high = c3;
            const TVec columns[ 3 ] = { c0, c1, c2 };
            for ( UInt32 j=0; j<3; ++j )
            {
                const TVec a = L::Mul( columns[ j ], L::Set1( box[ j ] ) );
                const TVec b = L::Mul( columns[ j ], L::Set1( box[ 3 + j ] ) );
                low = L::Add( low, L::Select( L::CmpLT( b, a ), b, a ) );
                high = L::Add( high, L::Select( L::CmpLT( a, b ), b, a ) );
            }
            Float32 tl[ 4 ];
            Float32 th[ 4 ];
            L::Store4( tl, low );
            L::Store4( th, high );
            memcpy( o, tl, 3 * sizeof( Float32 ) );
            memcpy( o + 3 * sizeof( Float32 ), th, 3 * sizeof( Float32 ) );
        }
        return MATH::MATHSTATUS_OK;
    }

    /** The lanes are four planes at a time; padding lanes are the plane 0 . p = 0, which is inert */
    static MATH::TMathStatus
    CullAabbs( void* /* context */, const SGeometryCullAabbsFrustumF32Args& args ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        GUCEF_MATH_FP_CONTRACT_OFF
        if ( args.planeCount > GeometrySimdMaxPlanes )
            return MATH::MATHSTATUS_INVALID_OPERAND;
        const UInt32 groups = ( args.planeCount + 3 ) / 4;
        // per group: nx, ny, nz, d, |nx|, |ny|, |nz| as four lanes each
        Float32 lanes[ GeometrySimdMaxPlanes / 4 ][ 7 ][ 4 ];
        for ( UInt32 g=0; g<groups; ++g )
            for ( UInt32 k=0; k<4; ++k )
            {
                const UInt32 index = g * 4 + k;
                Float32 n[ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f };
                if ( index < args.planeCount )
                    memcpy( n, args.planes + index, sizeof( n ) );
                lanes[ g ][ 0 ][ k ] = n[ 0 ]; lanes[ g ][ 1 ][ k ] = n[ 1 ]; lanes[ g ][ 2 ][ k ] = n[ 2 ]; lanes[ g ][ 3 ][ k ] = n[ 3 ];
                lanes[ g ][ 4 ][ k ] = n[ 0 ] < 0.0f ? -n[ 0 ] : n[ 0 ];
                lanes[ g ][ 5 ][ k ] = n[ 1 ] < 0.0f ? -n[ 1 ] : n[ 1 ];
                lanes[ g ][ 6 ][ k ] = n[ 2 ] < 0.0f ? -n[ 2 ] : n[ 2 ];
            }
        const UInt8* in = static_cast< const UInt8* >( args.in );
        const TVec zero = L::Zero();
        const TVec ones = L::CmpLE( zero, zero );
        for ( UInt32 i=0; i<args.count; ++i )
        {
            Float32 box[ 6 ];
            memcpy( box, in + (size_t) i * args.inStrideBytes, sizeof( box ) );
            UInt8& result = args.out[ (size_t) i * args.outStrideBytes ];
            if ( !( box[ 0 ] <= box[ 3 ] && box[ 1 ] <= box[ 4 ] && box[ 2 ] <= box[ 5 ] ) )
            {
                result = (UInt8) FRUSTUMCLASSIFICATION_OUTSIDE;
                continue;
            }
            const Float32 ex = box[ 3 ] - box[ 0 ];
            const Float32 ey = box[ 4 ] - box[ 1 ];
            const Float32 ez = box[ 5 ] - box[ 2 ];
            const Float32 cx = box[ 0 ] + ex * 0.5f;
            const Float32 cy = box[ 1 ] + ey * 0.5f;
            const Float32 cz = box[ 2 ] + ez * 0.5f;
            const Float32 hx = ex * 0.5f;
            const Float32 hy = ey * 0.5f;
            const Float32 hz = ez * 0.5f;
            if ( !IsFiniteFloat32( cx ) || !IsFiniteFloat32( cy ) || !IsFiniteFloat32( cz ) || !IsFiniteFloat32( hx ) || !IsFiniteFloat32( hy ) || !IsFiniteFloat32( hz ) )
            {
                result = (UInt8) FRUSTUMCLASSIFICATION_OUTSIDE;
                continue;
            }
            const TVec vcx = L::Set1( cx ), vcy = L::Set1( cy ), vcz = L::Set1( cz );
            const TVec vhx = L::Set1( hx ), vhy = L::Set1( hy ), vhz = L::Set1( hz );
            TVec outside = zero;
            TVec straddle = zero;
            TVec bad = zero;
            for ( UInt32 g=0; g<groups; ++g )
            {
                const TVec distance = L::Sub( L::Add( L::Add( L::Mul( L::Load4( lanes[ g ][ 0 ] ), vcx ), L::Mul( L::Load4( lanes[ g ][ 1 ] ), vcy ) ), L::Mul( L::Load4( lanes[ g ][ 2 ] ), vcz ) ), L::Load4( lanes[ g ][ 3 ] ) );
                const TVec radius = L::Add( L::Add( L::Mul( L::Load4( lanes[ g ][ 4 ] ), vhx ), L::Mul( L::Load4( lanes[ g ][ 5 ] ), vhy ) ), L::Mul( L::Load4( lanes[ g ][ 6 ] ), vhz ) );
                // x - x is 0 for a finite x, NaN otherwise: both comparisons are false for NaN
                const TVec dd = L::Sub( distance, distance );
                const TVec rd = L::Sub( radius, radius );
                const TVec finite = L::And( L::And( L::CmpLE( dd, zero ), L::CmpGE( dd, zero ) ), L::And( L::CmpLE( rd, zero ), L::CmpGE( rd, zero ) ) );
                bad = L::Select( finite, bad, ones );
                outside = L::Select( L::CmpLT( distance, L::Sub( zero, radius ) ), ones, outside );
                straddle = L::Select( L::CmpLT( distance, radius ), ones, straddle );
            }
            if ( 0 != L::MoveMask( bad ) || 0 != L::MoveMask( outside ) )
                result = (UInt8) FRUSTUMCLASSIFICATION_OUTSIDE;
            else
                result = (UInt8) ( 0 != L::MoveMask( straddle ) ? FRUSTUMCLASSIFICATION_INTERSECTING : FRUSTUMCLASSIFICATION_INSIDE );
        }
        return MATH::MATHSTATUS_OK;
    }
};

/*-------------------------------------------------------------------------*/

struct SGeometryKernelSet
{
    const char* name;
    MATH::TMathBatchTier tier;
    UInt64 requiredCpuFeatures;
    MATH::TMathBatchGenericFunction transformAabbs;
    MATH::TMathBatchGenericFunction cullAabbs;
    bool cullMayFail;
};

/*-------------------------------------------------------------------------*/

static bool
RegisterKernelSet( MATH::CMathBatchDispatcher& dispatcher, const SGeometryKernelSet& set, const void* owner )
{
    bool ok = true;
    for ( UInt32 k=0; k<2; ++k )
    {
        MATH::SMathBatchProvider provider;
        provider.tier = set.tier;
        provider.name = set.name;
        provider.function = 0 == k ? set.transformAabbs : set.cullAabbs;
        provider.minCount = 0;
        provider.maxCount = 0xFFFFFFFF;
        provider.flags = MATH::MATHPROVIDER_BITEXACT;
        provider.alignment = 0;
        provider.requiredCpuFeatures = set.requiredCpuFeatures;
        provider.context = GUCEF_NULL;
        if ( 1 == k && set.cullMayFail )
            provider.flags |= MATH::MATHPROVIDER_MAY_FAIL;
        #if ( GUCEF_CPU_REQUIRES_ALIGNED_ACCESS == 1 )
        if ( MATH::MATHBATCHTIER_SIMD == set.tier )
        {
            provider.flags |= MATH::MATHPROVIDER_REQUIRES_ALIGNED_INPUT;
            provider.alignment = 16;
        }
        #endif
        ok = dispatcher.RegisterProvider( (MATH::TMathBatchKernel) ( GEOMETRYBATCHKERNEL_TRANSFORM_AABBS_F32 + k ), provider, owner ) && ok;
    }
    return ok;
}

/*-------------------------------------------------------------------------*/

/**
 *  Registers the two kernels and their providers with 'dispatcher' under 'owner': the scalar providers and, when the
 *  build targets an instruction set family with a lane policy and the CPU has the features, the SIMD providers. Not
 *  realtime-safe. Returns false when a registration was refused.
 */
bool
GeometryRegisterBatchKernels( MATH::CMathBatchDispatcher& dispatcher, const void* owner )
{GUCEF_TRACE;

    bool ok = dispatcher.RegisterExternalKernel( (MATH::TMathBatchKernel) GEOMETRYBATCHKERNEL_TRANSFORM_AABBS_F32, "GEOMETRY.TransformAabbs.F32", false, owner );
    ok = dispatcher.RegisterExternalKernel( (MATH::TMathBatchKernel) GEOMETRYBATCHKERNEL_CULL_AABBS_FRUSTUM_F32, "GEOMETRY.CullAabbsFrustum.F32", false, owner ) && ok;

    SGeometryKernelSet scalar;
    scalar.name = "Scalar";
    scalar.tier = MATH::MATHBATCHTIER_SCALAR;
    scalar.requiredCpuFeatures = 0;
    scalar.transformAabbs = reinterpret_cast< MATH::TMathBatchGenericFunction >( &TransformAabbsScalar );
    scalar.cullAabbs = reinterpret_cast< MATH::TMathBatchGenericFunction >( &CullAabbsScalar );
    scalar.cullMayFail = false;
    ok = RegisterKernelSet( dispatcher, scalar, owner ) && ok;

    #if ( GUCEF_MATH_SIMD_HAVE_SSE2 == 1 )
    SGeometryKernelSet simd;
    simd.name = "SSE2";
    simd.tier = MATH::MATHBATCHTIER_SIMD;
    simd.requiredCpuFeatures = GUCEF_CPU_ISA_SSE2;
    simd.transformAabbs = reinterpret_cast< MATH::TMathBatchGenericFunction >( &CGeometryBatchSimd< MATH::CMathSse2 >::TransformAabbs );
    simd.cullAabbs = reinterpret_cast< MATH::TMathBatchGenericFunction >( &CGeometryBatchSimd< MATH::CMathSse2 >::CullAabbs );
    simd.cullMayFail = true;
    if ( 0 != ( dispatcher.GetCpuIsaFeatures() & simd.requiredCpuFeatures ) )
        ok = RegisterKernelSet( dispatcher, simd, owner ) && ok;
    #elif ( GUCEF_MATH_SIMD_HAVE_NEON == 1 )
    SGeometryKernelSet simd;
    simd.name = "NEON";
    simd.tier = MATH::MATHBATCHTIER_SIMD;
    simd.requiredCpuFeatures = GUCEF_CPU_ISA_NEON;
    simd.transformAabbs = reinterpret_cast< MATH::TMathBatchGenericFunction >( &CGeometryBatchSimd< MATH::CMathNeon >::TransformAabbs );
    simd.cullAabbs = reinterpret_cast< MATH::TMathBatchGenericFunction >( &CGeometryBatchSimd< MATH::CMathNeon >::CullAabbs );
    simd.cullMayFail = true;
    if ( 0 != ( dispatcher.GetCpuIsaFeatures() & simd.requiredCpuFeatures ) )
        ok = RegisterKernelSet( dispatcher, simd, owner ) && ok;
    #elif ( GUCEF_MATH_SIMD_HAVE_WASM128 == 1 )
    SGeometryKernelSet simd;
    simd.name = "WASM128";
    simd.tier = MATH::MATHBATCHTIER_SIMD;
    simd.requiredCpuFeatures = GUCEF_CPU_ISA_WASM_SIMD128;
    simd.transformAabbs = reinterpret_cast< MATH::TMathBatchGenericFunction >( &CGeometryBatchSimd< MATH::CMathWasm128 >::TransformAabbs );
    simd.cullAabbs = reinterpret_cast< MATH::TMathBatchGenericFunction >( &CGeometryBatchSimd< MATH::CMathWasm128 >::CullAabbs );
    simd.cullMayFail = true;
    if ( 0 != ( dispatcher.GetCpuIsaFeatures() & simd.requiredCpuFeatures ) )
        ok = RegisterKernelSet( dispatcher, simd, owner ) && ok;
    #endif
    return ok;
}

/*-------------------------------------------------------------------------*/

MATH::TMathStatus
TransformAabbs( const MATH::TFloat32Matrix4D& matrix, const TFloat32AxisAlignedBox3D* in, TFloat32AxisAlignedBox3D* out, UInt32 count, UInt32 flags )
{
    GUCEF_GEOMETRY_STATIC_ASSERT( sizeof( TFloat32AxisAlignedBox3D ) == 24, Float32_box_is_24_bytes );
    SGeometryTransformAabbsF32Args args;
    args.matrix = &matrix;
    args.in = in;
    args.inStrideBytes = sizeof( TFloat32AxisAlignedBox3D );
    args.out = out;
    args.outStrideBytes = sizeof( TFloat32AxisAlignedBox3D );
    args.count = count;
    // the first call creates the module global, which registers the kernels
    CGeometryGlobal::Instance();
    return MATH::CMathGlobal::Instance()->GetBatchDispatcher().Execute( (MATH::TMathBatchKernel) GEOMETRYBATCHKERNEL_TRANSFORM_AABBS_F32, args, count, flags, in, out );
}

/*-------------------------------------------------------------------------*/

MATH::TMathStatus
CullAabbsFrustum( const TFloat32Plane* planes, UInt32 planeCount, const TFloat32AxisAlignedBox3D* boxes, UInt8* out, UInt32 count, UInt32 flags )
{
    GUCEF_GEOMETRY_STATIC_ASSERT( sizeof( TFloat32Plane ) == 16, Float32_plane_is_16_bytes );
    SGeometryCullAabbsFrustumF32Args args;
    args.planes = planes;
    args.planeCount = planeCount;
    args.in = boxes;
    args.inStrideBytes = sizeof( TFloat32AxisAlignedBox3D );
    args.out = out;
    args.outStrideBytes = 1;
    args.count = count;
    CGeometryGlobal::Instance();
    return MATH::CMathGlobal::Instance()->GetBatchDispatcher().Execute( (MATH::TMathBatchKernel) GEOMETRYBATCHKERNEL_CULL_AABBS_FRUSTUM_F32, args, count, flags, boxes, out );
}

/*-------------------------------------------------------------------------*/

MATH::TMathStatus
CullAabbsFrustum( const TFloat32Frustum& frustum, const TFloat32AxisAlignedBox3D* boxes, UInt8* out, UInt32 count, UInt32 flags )
{
    return CullAabbsFrustum( frustum.GetPlanes(), 6, boxes, out, count, flags );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
