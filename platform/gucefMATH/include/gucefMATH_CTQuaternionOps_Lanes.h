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

#ifndef GUCEF_MATH_CTQUATERNIONOPS_LANES_H
#define GUCEF_MATH_CTQUATERNIONOPS_LANES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTQUATERNIONOPS_H
#include "gucefMATH_CTQuaternionOps.h"
#define GUCEF_MATH_CTQUATERNIONOPS_H
#endif /* GUCEF_MATH_CTQUATERNIONOPS_H ? */

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
 *  The operations of CTQuaternionOps for Float32 written once over a lane policy L (see gucefMATH_CTVector4Ops_Lanes.h):
 *  the same results bit for bit as the primary template (canonical order of gucefMATH_CTQuaternionOps.h). Each lane
 *  computes one result component as the same 4 term dot product, negations are sign flips of the left operands as in
 *  the primary template.
 */
template < class L >
class CTQuaternionOpsLanes
{
    public:

    typedef CTQuaternionOps< Float32, typename L::TScalarBackend > TScalarOps;
    typedef CTMathComponentTraits< Float32 >::TKernels TKernels;

    /** Lanes x, y, z, w: ( l0 r0 + l2 r2 ) + ( l1 r1 + l3 r3 ) with the operand rows of the primary template */
    static inline void Multiply( const Float32 ( &a )[ 4 ], const Float32 ( &b )[ 4 ], Float32 ( &out )[ 4 ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_FP_CONTRACT_OFF
        const typename L::TVec va = L::Load4( a );
        const typename L::TVec vb = L::Load4( b );
        const typename L::TVec l0 = L::template Splat< 3 >( va );                                                  // aw aw aw aw
        const typename L::TVec l1 = L::Xor( L::template Splat< 0 >( va ), L::SignMask( 0, 1, 0, 1 ) );           // ax -ax ax -ax
        const typename L::TVec l2 = L::Xor( L::template Splat< 1 >( va ), L::SignMask( 0, 0, 1, 1 ) );           // ay ay -ay -ay
        const typename L::TVec l3 = L::Xor( L::template Splat< 2 >( va ), L::SignMask( 1, 0, 0, 1 ) );           // -az az az -az
        const typename L::TVec r1 = L::template Shuffle< 3, 2, 1, 0 >( vb );                                       // bw bz by bx
        const typename L::TVec r2 = L::template Shuffle< 2, 3, 0, 1 >( vb );                                       // bz bw bx by
        const typename L::TVec r3 = L::template Shuffle< 1, 0, 3, 2 >( vb );                                       // by bx bw bz
        const typename L::TVec result = L::Add( L::Add( L::Mul( l0, vb ), L::Mul( l2, r2 ) ),
                                                L::Add( L::Mul( l1, r1 ), L::Mul( l3, r3 ) ) );
        L::Store4( out, result );
    }

    /**
     *  t = 2 ( q.xyz x v ) with cross lane i = q[ j ] v[ k ] + ( -q[ k ] ) v[ j ], then lane i of the result
     *  ( 1 v[ i ] + q[ j ] t[ k ] ) + ( w t[ i ] + ( -q[ k ] ) t[ j ] ), j = i + 1, k = i + 2 (mod 3)
     */
    static inline void Rotate( const Float32 ( &q )[ 4 ], const Float32 ( &v )[ 3 ], Float32 ( &out )[ 3 ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_FP_CONTRACT_OFF
        const typename L::TVec vq = L::Load4( q );
        const typename L::TVec vv = L::Load3( v );
        const typename L::TVec qj = L::template Shuffle< 1, 2, 0, 3 >( vq );                                       // qy qz qx
        const typename L::TVec negQk = L::Xor( L::template Shuffle< 2, 0, 1, 3 >( vq ), L::Set1( -0.0f ) );       // -qz -qx -qy
        const typename L::TVec vk = L::template Shuffle< 2, 0, 1, 3 >( vv );                                       // vz vx vy
        const typename L::TVec vj = L::template Shuffle< 1, 2, 0, 3 >( vv );                                       // vy vz vx
        const typename L::TVec cross = L::Add( L::Mul( qj, vk ), L::Mul( negQk, vj ) );
        const typename L::TVec t = L::Add( cross, cross );
        const typename L::TVec tk = L::template Shuffle< 2, 0, 1, 3 >( t );
        const typename L::TVec tj = L::template Shuffle< 1, 2, 0, 3 >( t );
        const typename L::TVec result = L::Add( L::Add( L::Mul( L::Set1( 1.0f ), vv ), L::Mul( qj, tk ) ),
                                                L::Add( L::Mul( L::template Splat< 3 >( vq ), t ), L::Mul( negQk, tj ) ) );
        L::Store3( out, result );
    }

    static GUCEF_MATH_FORCE_INLINE Float32 Dot( const Float32 ( &a )[ 4 ], const Float32 ( &b )[ 4 ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return L::Lane0( L::Dot4( L::Load4( a ), L::Load4( b ) ) );
    }

    /** The safe range in SIMD, every other case through the primary template (false leaves out unchanged) */
    static inline bool Normalize( const Float32 ( &a )[ 4 ], Float32 ( &out )[ 4 ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const typename L::TVec va = L::Load4( a );
        const typename L::TVec squared = L::Dot4( va, va );
        if ( TKernels::IsSafeSquaredLength( L::Lane0( squared ) ) )
        {
            const typename L::TVec length = L::Sqrt( L::template Splat< 0 >( squared ) );
            L::Store4( out, L::Div( va, length ) );
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

#endif /* GUCEF_MATH_CTQUATERNIONOPS_LANES_H ? */
