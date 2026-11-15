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

#ifndef GUCEF_MATH_CTVECTOR4OPS_SSE2_H
#define GUCEF_MATH_CTVECTOR4OPS_SSE2_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <emmintrin.h>

#ifndef GUCEF_MATH_CTVECTOR4OPS_H
#include "gucefMATH_CTVector4Ops.h"
#define GUCEF_MATH_CTVECTOR4OPS_H
#endif /* GUCEF_MATH_CTVECTOR4OPS_H ? */

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
 *  SSE2 helpers shared by the SSE2 specialisations of the operation policies: unaligned loads and stores, the
 *  canonical 4 term dot product ( p0 + p2 ) + ( p1 + p3 ) and sign masks. Only compiled when SSE2 is configured.
 *
 *  The class is also the lane policy of the batch kernels (src/gucefMATH_BatchKernelsSimd.h): CMathNeon and
 *  CMathWasm128 have the same members, so the kernels are written once. A lane policy offers: TVec, TMask, Load4,
 *  Store4, Load3, Store3, Set1, Zero, Set4, Add, Sub, Mul, Div, Sqrt (correctly rounded, as the scalar operations),
 *  Xor, Splat< lane >, Shuffle< a, b, c, d > (result lane i is source lane a, b, c, d), SignMask, Dot4 (result in
 *  lane 0), Lane0, Combine4, the comparisons CmpLT, CmpLE, CmpGE, CmpGT (a mask of all ones or all zeros per lane;
 *  false for NaN as the scalar comparisons), And, Select( mask, a, b ) (a where the mask is set), MoveMask (bit i for
 *  lane i) and Transpose4. No fused multiply-add and no estimate instructions.
 */
class CMathSse2
{
    public:

    typedef __m128 TVec;
    typedef __m128 TMask;
    typedef SMathSimdScalar TScalarBackend;

    static GUCEF_MATH_FORCE_INLINE __m128 Load4( const Float32* p ) GUCEF_MATH_REALTIME_CONTEXT { return _mm_loadu_ps( p ); }

    static GUCEF_MATH_FORCE_INLINE void Store4( Float32* p, __m128 v ) GUCEF_MATH_REALTIME_CONTEXT { _mm_storeu_ps( p, v ); }

    /** ( x, y, z, 0 ) from 3 components */
    static GUCEF_MATH_FORCE_INLINE __m128 Load3( const Float32* p ) GUCEF_MATH_REALTIME_CONTEXT { return _mm_set_ps( 0.0f, p[ 2 ], p[ 1 ], p[ 0 ] ); }

    static GUCEF_MATH_FORCE_INLINE void Store3( Float32* p, __m128 v ) GUCEF_MATH_REALTIME_CONTEXT
    {
        Float32 lanes[ 4 ];
        _mm_storeu_ps( lanes, v );
        p[ 0 ] = lanes[ 0 ]; p[ 1 ] = lanes[ 1 ]; p[ 2 ] = lanes[ 2 ];
    }

    /** Lane i of v in every lane */
    template < int I >
    static GUCEF_MATH_FORCE_INLINE __m128 Splat( __m128 v ) GUCEF_MATH_REALTIME_CONTEXT { return _mm_shuffle_ps( v, v, _MM_SHUFFLE( I, I, I, I ) ); }

    /** -0.0f in the lanes given as 1, +0.0f elsewhere: xor with it negates those lanes exactly as unary minus does */
    static GUCEF_MATH_FORCE_INLINE __m128 SignMask( int l0, int l1, int l2, int l3 ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return _mm_set_ps( l3 ? -0.0f : 0.0f, l2 ? -0.0f : 0.0f, l1 ? -0.0f : 0.0f, l0 ? -0.0f : 0.0f );
    }

    /** The canonical dot product of the 4 lanes in lane 0: ( a0 b0 + a2 b2 ) + ( a1 b1 + a3 b3 ) */
    static GUCEF_MATH_FORCE_INLINE __m128 Dot4( __m128 a, __m128 b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_FP_CONTRACT_OFF
        const __m128 p = _mm_mul_ps( a, b );
        const __m128 s = _mm_add_ps( p, _mm_movehl_ps( p, p ) );                     // p0 + p2, p1 + p3
        return _mm_add_ss( s, _mm_shuffle_ps( s, s, _MM_SHUFFLE( 1, 1, 1, 1 ) ) );
    }

    static GUCEF_MATH_FORCE_INLINE __m128 Set1( Float32 v ) GUCEF_MATH_REALTIME_CONTEXT { return _mm_set1_ps( v ); }
    static GUCEF_MATH_FORCE_INLINE __m128 Zero( void ) GUCEF_MATH_REALTIME_CONTEXT { return _mm_setzero_ps(); }
    static GUCEF_MATH_FORCE_INLINE __m128 Set4( Float32 l0, Float32 l1, Float32 l2, Float32 l3 ) GUCEF_MATH_REALTIME_CONTEXT { return _mm_set_ps( l3, l2, l1, l0 ); }
    static GUCEF_MATH_FORCE_INLINE __m128 Add( __m128 a, __m128 b ) GUCEF_MATH_REALTIME_CONTEXT { return _mm_add_ps( a, b ); }
    static GUCEF_MATH_FORCE_INLINE __m128 Sub( __m128 a, __m128 b ) GUCEF_MATH_REALTIME_CONTEXT { return _mm_sub_ps( a, b ); }
    static GUCEF_MATH_FORCE_INLINE __m128 Mul( __m128 a, __m128 b ) GUCEF_MATH_REALTIME_CONTEXT { return _mm_mul_ps( a, b ); }
    static GUCEF_MATH_FORCE_INLINE __m128 Div( __m128 a, __m128 b ) GUCEF_MATH_REALTIME_CONTEXT { return _mm_div_ps( a, b ); }
    static GUCEF_MATH_FORCE_INLINE __m128 Sqrt( __m128 a ) GUCEF_MATH_REALTIME_CONTEXT { return _mm_sqrt_ps( a ); }
    static GUCEF_MATH_FORCE_INLINE __m128 Xor( __m128 a, __m128 b ) GUCEF_MATH_REALTIME_CONTEXT { return _mm_xor_ps( a, b ); }
    static GUCEF_MATH_FORCE_INLINE Float32 Lane0( __m128 v ) GUCEF_MATH_REALTIME_CONTEXT { return _mm_cvtss_f32( v ); }

    template < int A, int B, int C, int D >
    static GUCEF_MATH_FORCE_INLINE __m128 Shuffle( __m128 v ) GUCEF_MATH_REALTIME_CONTEXT { return _mm_shuffle_ps( v, v, _MM_SHUFFLE( D, C, B, A ) ); }

    static GUCEF_MATH_FORCE_INLINE __m128 CmpLT( __m128 a, __m128 b ) GUCEF_MATH_REALTIME_CONTEXT { return _mm_cmplt_ps( a, b ); }
    static GUCEF_MATH_FORCE_INLINE __m128 CmpLE( __m128 a, __m128 b ) GUCEF_MATH_REALTIME_CONTEXT { return _mm_cmple_ps( a, b ); }
    static GUCEF_MATH_FORCE_INLINE __m128 CmpGE( __m128 a, __m128 b ) GUCEF_MATH_REALTIME_CONTEXT { return _mm_cmpge_ps( a, b ); }
    static GUCEF_MATH_FORCE_INLINE __m128 CmpGT( __m128 a, __m128 b ) GUCEF_MATH_REALTIME_CONTEXT { return _mm_cmpgt_ps( a, b ); }
    static GUCEF_MATH_FORCE_INLINE __m128 And( __m128 a, __m128 b ) GUCEF_MATH_REALTIME_CONTEXT { return _mm_and_ps( a, b ); }
    static GUCEF_MATH_FORCE_INLINE __m128 Select( __m128 mask, __m128 a, __m128 b ) GUCEF_MATH_REALTIME_CONTEXT { return _mm_or_ps( _mm_and_ps( mask, a ), _mm_andnot_ps( mask, b ) ); }
    static GUCEF_MATH_FORCE_INLINE int MoveMask( __m128 mask ) GUCEF_MATH_REALTIME_CONTEXT { return _mm_movemask_ps( mask ); }

    /** v0..v3 are 4 vectors of 4 lanes (rows): afterwards v0 holds lane 0 of each, v1 lane 1 of each and so on */
    static GUCEF_MATH_FORCE_INLINE void Transpose4( __m128& v0, __m128& v1, __m128& v2, __m128& v3 ) GUCEF_MATH_REALTIME_CONTEXT { _MM_TRANSPOSE4_PS( v0, v1, v2, v3 ); }

    /** Per lane ( c0 x + c2 z ) + ( c1 y + c3 w ): the canonical matrix times vector over the columns c */
    static GUCEF_MATH_FORCE_INLINE __m128 Combine4( __m128 c0, __m128 c1, __m128 c2, __m128 c3, __m128 x, __m128 y, __m128 z, __m128 w ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_FP_CONTRACT_OFF
        return _mm_add_ps( _mm_add_ps( _mm_mul_ps( c0, x ), _mm_mul_ps( c2, z ) ), _mm_add_ps( _mm_mul_ps( c1, y ), _mm_mul_ps( c3, w ) ) );
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  SSE2 implementation of CTVector4Ops for Float32: the same results bit for bit as the primary template (canonical
 *  order of gucefMATH_CTVector4Ops.h). The vector is accessed as its 4 consecutive components (the layout of
 *  CTVector4D, 16 bytes, alignment of Float32) with unaligned loads and stores.
 */
template <>
class GUCEF_MATH_TEMPLATE_PUBLIC CTVector4Ops< Float32, SMathSimdSse2 >
{
    public:

    typedef CTVector4D< Float32 > TVector;
    typedef CTVector4Ops< Float32, SMathSimdScalar > TScalarOps;
    typedef CTMathComponentTraits< Float32 >::TKernels TKernels;

    static GUCEF_MATH_FORCE_INLINE void Add( const TVector& a, const TVector& b, TVector& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CMathSse2::Store4( Data( out ), _mm_add_ps( CMathSse2::Load4( Data( a ) ), CMathSse2::Load4( Data( b ) ) ) );
    }

    static GUCEF_MATH_FORCE_INLINE void Subtract( const TVector& a, const TVector& b, TVector& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CMathSse2::Store4( Data( out ), _mm_sub_ps( CMathSse2::Load4( Data( a ) ), CMathSse2::Load4( Data( b ) ) ) );
    }

    static GUCEF_MATH_FORCE_INLINE void Negate( const TVector& a, TVector& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CMathSse2::Store4( Data( out ), _mm_xor_ps( CMathSse2::Load4( Data( a ) ), _mm_set1_ps( -0.0f ) ) );
    }

    static GUCEF_MATH_FORCE_INLINE void Multiply( const TVector& a, const Float32& factor, TVector& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CMathSse2::Store4( Data( out ), _mm_mul_ps( CMathSse2::Load4( Data( a ) ), _mm_set1_ps( factor ) ) );
    }

    static GUCEF_MATH_FORCE_INLINE Float32 Dot( const TVector& a, const TVector& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return _mm_cvtss_f32( CMathSse2::Dot4( CMathSse2::Load4( Data( a ) ), CMathSse2::Load4( Data( b ) ) ) );
    }

    /** minps( b, a ) is b < a ? b : a per lane, as the scalar Min */
    static GUCEF_MATH_FORCE_INLINE void Min( const TVector& a, const TVector& b, TVector& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CMathSse2::Store4( Data( out ), _mm_min_ps( CMathSse2::Load4( Data( b ) ), CMathSse2::Load4( Data( a ) ) ) );
    }

    /** maxps( b, a ) is b > a ? b : a per lane, as the scalar Max ( a < b ? b : a ) */
    static GUCEF_MATH_FORCE_INLINE void Max( const TVector& a, const TVector& b, TVector& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CMathSse2::Store4( Data( out ), _mm_max_ps( CMathSse2::Load4( Data( b ) ), CMathSse2::Load4( Data( a ) ) ) );
    }

    /** The safe range in SIMD (division by the broadcast root), every other case through the primary template */
    static inline void Normalize( const TVector& a, TVector& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const __m128 va = CMathSse2::Load4( Data( a ) );
        const __m128 squared = CMathSse2::Dot4( va, va );
        if ( TKernels::IsSafeSquaredLength( _mm_cvtss_f32( squared ) ) )
        {
            const __m128 length = _mm_sqrt_ss( squared );
            CMathSse2::Store4( Data( out ), _mm_div_ps( va, CMathSse2::Splat< 0 >( length ) ) );
            return;
        }
        TScalarOps::Normalize( a, out );
    }

    private:

    /** The 4 components of a vector; only the address is taken, so CTVector4D may still be incomplete here */
    static GUCEF_MATH_FORCE_INLINE const Float32* Data( const TVector& v ) GUCEF_MATH_REALTIME_CONTEXT { return reinterpret_cast< const Float32* >( &v ); }
    static GUCEF_MATH_FORCE_INLINE Float32* Data( TVector& v ) GUCEF_MATH_REALTIME_CONTEXT { return reinterpret_cast< Float32* >( &v ); }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTVECTOR4OPS_SSE2_H ? */
