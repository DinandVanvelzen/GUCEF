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

#ifndef GUCEF_MATH_CTQUATERNIONOPS_SSE2_H
#define GUCEF_MATH_CTQUATERNIONOPS_SSE2_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <emmintrin.h>

#ifndef GUCEF_MATH_CTQUATERNIONOPS_H
#include "gucefMATH_CTQuaternionOps.h"
#define GUCEF_MATH_CTQUATERNIONOPS_H
#endif /* GUCEF_MATH_CTQUATERNIONOPS_H ? */

#ifndef GUCEF_MATH_CTVECTOR4OPS_SSE2_H
#include "gucefMATH_CTVector4Ops_Sse2.h"
#define GUCEF_MATH_CTVECTOR4OPS_SSE2_H
#endif /* GUCEF_MATH_CTVECTOR4OPS_SSE2_H ? */

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
 *  SSE2 implementation of CTQuaternionOps for Float32: the same results bit for bit as the primary template (canonical
 *  order of gucefMATH_CTQuaternionOps.h). Each lane computes one result component as the same 4 term dot product,
 *  negations are sign flips of the left operands as in the primary template.
 */
template <>
class GUCEF_MATH_TEMPLATE_PUBLIC CTQuaternionOps< Float32, SMathSimdSse2 >
{
    public:

    typedef CTQuaternionOps< Float32, SMathSimdScalar > TScalarOps;
    typedef CTMathComponentTraits< Float32 >::TKernels TKernels;

    /** Lanes x, y, z, w: ( l0 r0 + l2 r2 ) + ( l1 r1 + l3 r3 ) with the operand rows of the primary template */
    static inline void Multiply( const Float32 ( &a )[ 4 ], const Float32 ( &b )[ 4 ], Float32 ( &out )[ 4 ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_FP_CONTRACT_OFF
        const __m128 va = CMathSse2::Load4( a );
        const __m128 vb = CMathSse2::Load4( b );
        const __m128 l0 = CMathSse2::Splat< 3 >( va );                                                // aw aw aw aw
        const __m128 l1 = _mm_xor_ps( CMathSse2::Splat< 0 >( va ), CMathSse2::SignMask( 0, 1, 0, 1 ) );   // ax -ax ax -ax
        const __m128 l2 = _mm_xor_ps( CMathSse2::Splat< 1 >( va ), CMathSse2::SignMask( 0, 0, 1, 1 ) );   // ay ay -ay -ay
        const __m128 l3 = _mm_xor_ps( CMathSse2::Splat< 2 >( va ), CMathSse2::SignMask( 1, 0, 0, 1 ) );   // -az az az -az
        const __m128 r1 = _mm_shuffle_ps( vb, vb, _MM_SHUFFLE( 0, 1, 2, 3 ) );                         // bw bz by bx
        const __m128 r2 = _mm_shuffle_ps( vb, vb, _MM_SHUFFLE( 1, 0, 3, 2 ) );                         // bz bw bx by
        const __m128 r3 = _mm_shuffle_ps( vb, vb, _MM_SHUFFLE( 2, 3, 0, 1 ) );                         // by bx bw bz
        const __m128 result = _mm_add_ps( _mm_add_ps( _mm_mul_ps( l0, vb ), _mm_mul_ps( l2, r2 ) ),
                                          _mm_add_ps( _mm_mul_ps( l1, r1 ), _mm_mul_ps( l3, r3 ) ) );
        CMathSse2::Store4( out, result );
    }

    /**
     *  t = 2 ( q.xyz x v ) with cross lane i = q[ j ] v[ k ] + ( -q[ k ] ) v[ j ], then lane i of the result
     *  ( 1 v[ i ] + q[ j ] t[ k ] ) + ( w t[ i ] + ( -q[ k ] ) t[ j ] ), j = i + 1, k = i + 2 (mod 3)
     */
    static inline void Rotate( const Float32 ( &q )[ 4 ], const Float32 ( &v )[ 3 ], Float32 ( &out )[ 3 ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_FP_CONTRACT_OFF
        const __m128 vq = CMathSse2::Load4( q );
        const __m128 vv = CMathSse2::Load3( v );
        const __m128 qj = _mm_shuffle_ps( vq, vq, _MM_SHUFFLE( 3, 0, 2, 1 ) );                        // qy qz qx
        const __m128 negQk = _mm_xor_ps( _mm_shuffle_ps( vq, vq, _MM_SHUFFLE( 3, 1, 0, 2 ) ), _mm_set1_ps( -0.0f ) );    // -qz -qx -qy
        const __m128 vk = _mm_shuffle_ps( vv, vv, _MM_SHUFFLE( 3, 1, 0, 2 ) );                        // vz vx vy
        const __m128 vj = _mm_shuffle_ps( vv, vv, _MM_SHUFFLE( 3, 0, 2, 1 ) );                        // vy vz vx
        const __m128 cross = _mm_add_ps( _mm_mul_ps( qj, vk ), _mm_mul_ps( negQk, vj ) );
        const __m128 t = _mm_add_ps( cross, cross );
        const __m128 tk = _mm_shuffle_ps( t, t, _MM_SHUFFLE( 3, 1, 0, 2 ) );
        const __m128 tj = _mm_shuffle_ps( t, t, _MM_SHUFFLE( 3, 0, 2, 1 ) );
        const __m128 result = _mm_add_ps( _mm_add_ps( _mm_mul_ps( _mm_set1_ps( 1.0f ), vv ), _mm_mul_ps( qj, tk ) ),
                                          _mm_add_ps( _mm_mul_ps( CMathSse2::Splat< 3 >( vq ), t ), _mm_mul_ps( negQk, tj ) ) );
        CMathSse2::Store3( out, result );
    }

    static GUCEF_MATH_FORCE_INLINE Float32 Dot( const Float32 ( &a )[ 4 ], const Float32 ( &b )[ 4 ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return _mm_cvtss_f32( CMathSse2::Dot4( CMathSse2::Load4( a ), CMathSse2::Load4( b ) ) );
    }

    /** The safe range in SIMD, every other case through the primary template (false leaves out unchanged) */
    static inline bool Normalize( const Float32 ( &a )[ 4 ], Float32 ( &out )[ 4 ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const __m128 va = CMathSse2::Load4( a );
        const __m128 squared = CMathSse2::Dot4( va, va );
        if ( TKernels::IsSafeSquaredLength( _mm_cvtss_f32( squared ) ) )
        {
            const __m128 length = _mm_sqrt_ss( squared );
            CMathSse2::Store4( out, _mm_div_ps( va, CMathSse2::Splat< 0 >( length ) ) );
            return true;
        }
        return TScalarOps::Normalize( a, out );
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTQUATERNIONOPS_SSE2_H ? */
