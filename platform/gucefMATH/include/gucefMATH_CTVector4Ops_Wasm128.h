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

#ifndef GUCEF_MATH_CTVECTOR4OPS_WASM128_H
#define GUCEF_MATH_CTVECTOR4OPS_WASM128_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <wasm_simd128.h>
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
 *  WebAssembly SIMD128 lane policy, the counterpart of CMathSse2: unaligned loads and stores, the canonical 4 term dot
 *  product ( p0 + p2 ) + ( p1 + p3 ) and the operations of the batch kernels. Only compiled when SIMD128 is configured.
 *  The arithmetic is correctly rounded as the scalar operations; no relaxed (fused) instructions are used.
 */
class CMathWasm128
{
    public:

    typedef v128_t TVec;
    typedef v128_t TMask;
    typedef SMathSimdScalar TScalarBackend;

    static GUCEF_MATH_FORCE_INLINE v128_t Load4( const Float32* p ) GUCEF_MATH_REALTIME_CONTEXT { return wasm_v128_load( p ); }

    static GUCEF_MATH_FORCE_INLINE void Store4( Float32* p, v128_t v ) GUCEF_MATH_REALTIME_CONTEXT { wasm_v128_store( p, v ); }

    /** ( x, y, z, 0 ) from 3 components */
    static GUCEF_MATH_FORCE_INLINE v128_t Load3( const Float32* p ) GUCEF_MATH_REALTIME_CONTEXT { return wasm_f32x4_make( p[ 0 ], p[ 1 ], p[ 2 ], 0.0f ); }

    static GUCEF_MATH_FORCE_INLINE void Store3( Float32* p, v128_t v ) GUCEF_MATH_REALTIME_CONTEXT
    {
        Float32 lanes[ 4 ];
        wasm_v128_store( lanes, v );
        p[ 0 ] = lanes[ 0 ]; p[ 1 ] = lanes[ 1 ]; p[ 2 ] = lanes[ 2 ];
    }

    static GUCEF_MATH_FORCE_INLINE v128_t Set1( Float32 v ) GUCEF_MATH_REALTIME_CONTEXT { return wasm_f32x4_splat( v ); }
    static GUCEF_MATH_FORCE_INLINE v128_t Zero( void ) GUCEF_MATH_REALTIME_CONTEXT { return wasm_f32x4_splat( 0.0f ); }
    static GUCEF_MATH_FORCE_INLINE v128_t Set4( Float32 l0, Float32 l1, Float32 l2, Float32 l3 ) GUCEF_MATH_REALTIME_CONTEXT { return wasm_f32x4_make( l0, l1, l2, l3 ); }

    static GUCEF_MATH_FORCE_INLINE v128_t Add( v128_t a, v128_t b ) GUCEF_MATH_REALTIME_CONTEXT { return wasm_f32x4_add( a, b ); }
    static GUCEF_MATH_FORCE_INLINE v128_t Sub( v128_t a, v128_t b ) GUCEF_MATH_REALTIME_CONTEXT { return wasm_f32x4_sub( a, b ); }
    static GUCEF_MATH_FORCE_INLINE v128_t Mul( v128_t a, v128_t b ) GUCEF_MATH_REALTIME_CONTEXT { return wasm_f32x4_mul( a, b ); }
    static GUCEF_MATH_FORCE_INLINE v128_t Div( v128_t a, v128_t b ) GUCEF_MATH_REALTIME_CONTEXT { return wasm_f32x4_div( a, b ); }
    static GUCEF_MATH_FORCE_INLINE v128_t Sqrt( v128_t a ) GUCEF_MATH_REALTIME_CONTEXT { return wasm_f32x4_sqrt( a ); }
    static GUCEF_MATH_FORCE_INLINE v128_t Xor( v128_t a, v128_t b ) GUCEF_MATH_REALTIME_CONTEXT { return wasm_v128_xor( a, b ); }
    static GUCEF_MATH_FORCE_INLINE Float32 Lane0( v128_t v ) GUCEF_MATH_REALTIME_CONTEXT { return wasm_f32x4_extract_lane( v, 0 ); }

    /** Lane i of v in every lane */
    template < int I >
    static GUCEF_MATH_FORCE_INLINE v128_t Splat( v128_t v ) GUCEF_MATH_REALTIME_CONTEXT { return wasm_i32x4_shuffle( v, v, I, I, I, I ); }

    template < int A, int B, int C, int D >
    static GUCEF_MATH_FORCE_INLINE v128_t Shuffle( v128_t v ) GUCEF_MATH_REALTIME_CONTEXT { return wasm_i32x4_shuffle( v, v, A, B, C, D ); }

    /** -0.0f in the lanes given as 1, +0.0f elsewhere: xor with it negates those lanes exactly as unary minus does */
    static GUCEF_MATH_FORCE_INLINE v128_t SignMask( int l0, int l1, int l2, int l3 ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return wasm_u32x4_make( l0 ? 0x80000000U : 0U, l1 ? 0x80000000U : 0U, l2 ? 0x80000000U : 0U, l3 ? 0x80000000U : 0U );
    }

    /** The canonical dot product of the 4 lanes in lane 0: ( a0 b0 + a2 b2 ) + ( a1 b1 + a3 b3 ) */
    static GUCEF_MATH_FORCE_INLINE v128_t Dot4( v128_t a, v128_t b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const v128_t p = wasm_f32x4_mul( a, b );
        const v128_t s = wasm_f32x4_add( p, wasm_i32x4_shuffle( p, p, 2, 3, 2, 3 ) );     // p0 + p2, p1 + p3
        return wasm_f32x4_add( s, wasm_i32x4_shuffle( s, s, 1, 1, 1, 1 ) );
    }

    /** Per lane ( c0 x + c2 z ) + ( c1 y + c3 w ): the canonical matrix times vector over the columns c */
    static GUCEF_MATH_FORCE_INLINE v128_t Combine4( v128_t c0, v128_t c1, v128_t c2, v128_t c3, v128_t x, v128_t y, v128_t z, v128_t w ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return wasm_f32x4_add( wasm_f32x4_add( wasm_f32x4_mul( c0, x ), wasm_f32x4_mul( c2, z ) ), wasm_f32x4_add( wasm_f32x4_mul( c1, y ), wasm_f32x4_mul( c3, w ) ) );
    }

    static GUCEF_MATH_FORCE_INLINE v128_t CmpLT( v128_t a, v128_t b ) GUCEF_MATH_REALTIME_CONTEXT { return wasm_f32x4_lt( a, b ); }
    static GUCEF_MATH_FORCE_INLINE v128_t CmpLE( v128_t a, v128_t b ) GUCEF_MATH_REALTIME_CONTEXT { return wasm_f32x4_le( a, b ); }
    static GUCEF_MATH_FORCE_INLINE v128_t CmpGE( v128_t a, v128_t b ) GUCEF_MATH_REALTIME_CONTEXT { return wasm_f32x4_ge( a, b ); }
    static GUCEF_MATH_FORCE_INLINE v128_t CmpGT( v128_t a, v128_t b ) GUCEF_MATH_REALTIME_CONTEXT { return wasm_f32x4_gt( a, b ); }
    static GUCEF_MATH_FORCE_INLINE v128_t And( v128_t a, v128_t b ) GUCEF_MATH_REALTIME_CONTEXT { return wasm_v128_and( a, b ); }
    static GUCEF_MATH_FORCE_INLINE v128_t Select( v128_t mask, v128_t a, v128_t b ) GUCEF_MATH_REALTIME_CONTEXT { return wasm_v128_bitselect( a, b, mask ); }
    static GUCEF_MATH_FORCE_INLINE int MoveMask( v128_t mask ) GUCEF_MATH_REALTIME_CONTEXT { return (int) wasm_i32x4_bitmask( mask ); }

    /** v0..v3 are 4 vectors of 4 lanes (rows): afterwards v0 holds lane 0 of each, v1 lane 1 of each and so on */
    static GUCEF_MATH_FORCE_INLINE void Transpose4( v128_t& v0, v128_t& v1, v128_t& v2, v128_t& v3 ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const v128_t t0 = wasm_i32x4_shuffle( v0, v1, 0, 4, 1, 5 );      // v00 v10 v01 v11
        const v128_t t1 = wasm_i32x4_shuffle( v2, v3, 0, 4, 1, 5 );      // v20 v30 v21 v31
        const v128_t t2 = wasm_i32x4_shuffle( v0, v1, 2, 6, 3, 7 );      // v02 v12 v03 v13
        const v128_t t3 = wasm_i32x4_shuffle( v2, v3, 2, 6, 3, 7 );
        v0 = wasm_i32x4_shuffle( t0, t1, 0, 1, 4, 5 );
        v1 = wasm_i32x4_shuffle( t0, t1, 2, 3, 6, 7 );
        v2 = wasm_i32x4_shuffle( t2, t3, 0, 1, 4, 5 );
        v3 = wasm_i32x4_shuffle( t2, t3, 2, 3, 6, 7 );
    }
};

/*-------------------------------------------------------------------------*/

/** WebAssembly SIMD128 implementation of CTVector4Ops for Float32: the same results bit for bit as the primary template */
template <>
class GUCEF_MATH_TEMPLATE_PUBLIC CTVector4Ops< Float32, SMathSimdWasm128 > : public CTVector4OpsLanes< CMathWasm128 >
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

#endif /* GUCEF_MATH_CTVECTOR4OPS_WASM128_H ? */
