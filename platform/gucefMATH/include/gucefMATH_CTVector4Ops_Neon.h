/*
 *  gucefMATH: GUCEF module providing math: scalar, vector, matrix and numeric classes
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

#ifndef GUCEF_MATH_CTVECTOR4OPS_NEON_H
#define GUCEF_MATH_CTVECTOR4OPS_NEON_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <arm_neon.h>
#ifndef GUCEF_MATH_CTVECTOR4OPS_H
#include "gucefMATH_CTVector4Ops.h"
#define GUCEF_MATH_CTVECTOR4OPS_H
#endif /* GUCEF_MATH_CTVECTOR4OPS_H ? */

#ifndef GUCEF_MATH_CTVECTOR4OPS_LANES_H
#include "gucefMATH_CTVector4Ops_Lanes.h"
#define GUCEF_MATH_CTVECTOR4OPS_LANES_H
#endif /* GUCEF_MATH_CTVECTOR4OPS_LANES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  NEON (AArch64 Advanced SIMD) lane policy, the counterpart of CMathSse2: unaligned loads and stores, the canonical
 *  4 term dot product ( p0 + p2 ) + ( p1 + p3 ) and the operations of the batch kernels. Only compiled when NEON is
 *  configured. AArch64 gives the correctly rounded divide and square root of the scalar path and no flush of denormal
 *  numbers; no fused multiply-add and no estimate instructions are used.
 */
class CMathNeon
{
    public:

    typedef float32x4_t TVec;
    typedef uint32x4_t TMask;
    typedef SMathSimdScalar TScalarBackend;

    static GUCEF_MATH_FORCE_INLINE float32x4_t Load4( const Float32* p ) GUCEF_MATH_REALTIME_CONTEXT { return vld1q_f32( p ); }

    static GUCEF_MATH_FORCE_INLINE void Store4( Float32* p, float32x4_t v ) GUCEF_MATH_REALTIME_CONTEXT { vst1q_f32( p, v ); }

    /** ( x, y, z, 0 ) from 3 components */
    static GUCEF_MATH_FORCE_INLINE float32x4_t Load3( const Float32* p ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const Float32 lanes[ 4 ] = { p[ 0 ], p[ 1 ], p[ 2 ], 0.0f };
        return vld1q_f32( lanes );
    }

    static GUCEF_MATH_FORCE_INLINE void Store3( Float32* p, float32x4_t v ) GUCEF_MATH_REALTIME_CONTEXT
    {
        Float32 lanes[ 4 ];
        vst1q_f32( lanes, v );
        p[ 0 ] = lanes[ 0 ]; p[ 1 ] = lanes[ 1 ]; p[ 2 ] = lanes[ 2 ];
    }

    static GUCEF_MATH_FORCE_INLINE float32x4_t Set1( Float32 v ) GUCEF_MATH_REALTIME_CONTEXT { return vdupq_n_f32( v ); }
    static GUCEF_MATH_FORCE_INLINE float32x4_t Zero( void ) GUCEF_MATH_REALTIME_CONTEXT { return vdupq_n_f32( 0.0f ); }

    static GUCEF_MATH_FORCE_INLINE float32x4_t Set4( Float32 l0, Float32 l1, Float32 l2, Float32 l3 ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const Float32 lanes[ 4 ] = { l0, l1, l2, l3 };
        return vld1q_f32( lanes );
    }

    static GUCEF_MATH_FORCE_INLINE float32x4_t Add( float32x4_t a, float32x4_t b ) GUCEF_MATH_REALTIME_CONTEXT { return vaddq_f32( a, b ); }
    static GUCEF_MATH_FORCE_INLINE float32x4_t Sub( float32x4_t a, float32x4_t b ) GUCEF_MATH_REALTIME_CONTEXT { return vsubq_f32( a, b ); }
    static GUCEF_MATH_FORCE_INLINE float32x4_t Mul( float32x4_t a, float32x4_t b ) GUCEF_MATH_REALTIME_CONTEXT { return vmulq_f32( a, b ); }
    static GUCEF_MATH_FORCE_INLINE float32x4_t Div( float32x4_t a, float32x4_t b ) GUCEF_MATH_REALTIME_CONTEXT { return vdivq_f32( a, b ); }
    static GUCEF_MATH_FORCE_INLINE float32x4_t Sqrt( float32x4_t a ) GUCEF_MATH_REALTIME_CONTEXT { return vsqrtq_f32( a ); }

    static GUCEF_MATH_FORCE_INLINE float32x4_t Xor( float32x4_t a, float32x4_t b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return vreinterpretq_f32_u32( veorq_u32( vreinterpretq_u32_f32( a ), vreinterpretq_u32_f32( b ) ) );
    }

    static GUCEF_MATH_FORCE_INLINE Float32 Lane0( float32x4_t v ) GUCEF_MATH_REALTIME_CONTEXT { return vgetq_lane_f32( v, 0 ); }

    /** Lane i of v in every lane */
    template < int I >
    static GUCEF_MATH_FORCE_INLINE float32x4_t Splat( float32x4_t v ) GUCEF_MATH_REALTIME_CONTEXT { return vdupq_laneq_f32( v, I ); }

    template < int A, int B, int C, int D >
    static GUCEF_MATH_FORCE_INLINE float32x4_t Shuffle( float32x4_t v ) GUCEF_MATH_REALTIME_CONTEXT
    {
        float32x4_t r = vdupq_laneq_f32( v, A );
        r = vsetq_lane_f32( vgetq_lane_f32( v, B ), r, 1 );
        r = vsetq_lane_f32( vgetq_lane_f32( v, C ), r, 2 );
        r = vsetq_lane_f32( vgetq_lane_f32( v, D ), r, 3 );
        return r;
    }

    /** -0.0f in the lanes given as 1, +0.0f elsewhere: xor with it negates those lanes exactly as unary minus does */
    static GUCEF_MATH_FORCE_INLINE float32x4_t SignMask( int l0, int l1, int l2, int l3 ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const UInt32 lanes[ 4 ] = { l0 ? 0x80000000U : 0U, l1 ? 0x80000000U : 0U, l2 ? 0x80000000U : 0U, l3 ? 0x80000000U : 0U };
        return vreinterpretq_f32_u32( vld1q_u32( lanes ) );
    }

    /** The canonical dot product of the 4 lanes, in every lane: ( a0 b0 + a2 b2 ) + ( a1 b1 + a3 b3 ) */
    static GUCEF_MATH_FORCE_INLINE float32x4_t Dot4( float32x4_t a, float32x4_t b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_FP_CONTRACT_OFF
        const float32x4_t p = vmulq_f32( a, b );
        const float32x4_t s = vaddq_f32( p, vcombine_f32( vget_high_f32( p ), vget_high_f32( p ) ) );     // p0 + p2, p1 + p3
        return vaddq_f32( vdupq_laneq_f32( s, 0 ), vdupq_laneq_f32( s, 1 ) );
    }

    /** Per lane ( c0 x + c2 z ) + ( c1 y + c3 w ): the canonical matrix times vector over the columns c */
    static GUCEF_MATH_FORCE_INLINE float32x4_t Combine4( float32x4_t c0, float32x4_t c1, float32x4_t c2, float32x4_t c3, float32x4_t x, float32x4_t y, float32x4_t z, float32x4_t w ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_FP_CONTRACT_OFF
        return vaddq_f32( vaddq_f32( vmulq_f32( c0, x ), vmulq_f32( c2, z ) ), vaddq_f32( vmulq_f32( c1, y ), vmulq_f32( c3, w ) ) );
    }

    static GUCEF_MATH_FORCE_INLINE uint32x4_t CmpLT( float32x4_t a, float32x4_t b ) GUCEF_MATH_REALTIME_CONTEXT { return vcltq_f32( a, b ); }
    static GUCEF_MATH_FORCE_INLINE uint32x4_t CmpLE( float32x4_t a, float32x4_t b ) GUCEF_MATH_REALTIME_CONTEXT { return vcleq_f32( a, b ); }
    static GUCEF_MATH_FORCE_INLINE uint32x4_t CmpGE( float32x4_t a, float32x4_t b ) GUCEF_MATH_REALTIME_CONTEXT { return vcgeq_f32( a, b ); }
    static GUCEF_MATH_FORCE_INLINE uint32x4_t CmpGT( float32x4_t a, float32x4_t b ) GUCEF_MATH_REALTIME_CONTEXT { return vcgtq_f32( a, b ); }
    static GUCEF_MATH_FORCE_INLINE uint32x4_t And( uint32x4_t a, uint32x4_t b ) GUCEF_MATH_REALTIME_CONTEXT { return vandq_u32( a, b ); }
    static GUCEF_MATH_FORCE_INLINE float32x4_t Select( uint32x4_t mask, float32x4_t a, float32x4_t b ) GUCEF_MATH_REALTIME_CONTEXT { return vbslq_f32( mask, a, b ); }

    static GUCEF_MATH_FORCE_INLINE int MoveMask( uint32x4_t mask ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const UInt32 weights[ 4 ] = { 1U, 2U, 4U, 8U };
        return (int) vaddvq_u32( vandq_u32( vshrq_n_u32( mask, 31 ), vld1q_u32( weights ) ) );
    }

    /** v0..v3 are 4 vectors of 4 lanes (rows): afterwards v0 holds lane 0 of each, v1 lane 1 of each and so on */
    static GUCEF_MATH_FORCE_INLINE void Transpose4( float32x4_t& v0, float32x4_t& v1, float32x4_t& v2, float32x4_t& v3 ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const float32x4x2_t t0 = vtrnq_f32( v0, v1 );       // v00 v10 v02 v12 | v01 v11 v03 v13
        const float32x4x2_t t1 = vtrnq_f32( v2, v3 );
        v0 = vcombine_f32( vget_low_f32( t0.val[ 0 ] ), vget_low_f32( t1.val[ 0 ] ) );
        v1 = vcombine_f32( vget_low_f32( t0.val[ 1 ] ), vget_low_f32( t1.val[ 1 ] ) );
        v2 = vcombine_f32( vget_high_f32( t0.val[ 0 ] ), vget_high_f32( t1.val[ 0 ] ) );
        v3 = vcombine_f32( vget_high_f32( t0.val[ 1 ] ), vget_high_f32( t1.val[ 1 ] ) );
    }
};

/*-------------------------------------------------------------------------*/

/** NEON (AArch64) implementation of CTVector4Ops for Float32: the same results bit for bit as the primary template */
template <>
class GUCEF_MATH_TEMPLATE_PUBLIC CTVector4Ops< Float32, SMathSimdNeon > : public CTVector4OpsLanes< CMathNeon >
{
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTVECTOR4OPS_NEON_H ? */
