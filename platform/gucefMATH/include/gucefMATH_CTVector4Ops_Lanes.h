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

#ifndef GUCEF_MATH_CTVECTOR4OPS_LANES_H
#define GUCEF_MATH_CTVECTOR4OPS_LANES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
 *  The operations of CTVector4Ops for Float32 written once over a lane policy L (the members of CMathSse2; CMathNeon and
 *  CMathWasm128 have the same): the same results bit for bit as the primary template (canonical order of
 *  gucefMATH_CTVector4Ops.h). The vector is accessed as its 4 consecutive components (the layout of CTVector4D, 16
 *  bytes, alignment of Float32) with unaligned loads and stores. The specialisations of the NEON and WASM128 backends
 *  derive from it.
 */
template < class L >
class CTVector4OpsLanes
{
    public:

    typedef CTVector4D< Float32 > TVector;
    typedef CTVector4Ops< Float32, typename L::TScalarBackend > TScalarOps;
    typedef CTMathComponentTraits< Float32 >::TKernels TKernels;

    static GUCEF_MATH_FORCE_INLINE void Add( const TVector& a, const TVector& b, TVector& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        L::Store4( Data( out ), L::Add( L::Load4( Data( a ) ), L::Load4( Data( b ) ) ) );
    }

    static GUCEF_MATH_FORCE_INLINE void Subtract( const TVector& a, const TVector& b, TVector& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        L::Store4( Data( out ), L::Sub( L::Load4( Data( a ) ), L::Load4( Data( b ) ) ) );
    }

    static GUCEF_MATH_FORCE_INLINE void Negate( const TVector& a, TVector& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        L::Store4( Data( out ), L::Xor( L::Load4( Data( a ) ), L::Set1( -0.0f ) ) );
    }

    static GUCEF_MATH_FORCE_INLINE void Multiply( const TVector& a, const Float32& factor, TVector& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        L::Store4( Data( out ), L::Mul( L::Load4( Data( a ) ), L::Set1( factor ) ) );
    }

    static GUCEF_MATH_FORCE_INLINE Float32 Dot( const TVector& a, const TVector& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return L::Lane0( L::Dot4( L::Load4( Data( a ) ), L::Load4( Data( b ) ) ) );
    }

    /** b < a ? b : a per lane, as the scalar Min */
    static GUCEF_MATH_FORCE_INLINE void Min( const TVector& a, const TVector& b, TVector& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const typename L::TVec va = L::Load4( Data( a ) );
        const typename L::TVec vb = L::Load4( Data( b ) );
        L::Store4( Data( out ), L::Select( L::CmpLT( vb, va ), vb, va ) );
    }

    /** a < b ? b : a per lane, as the scalar Max */
    static GUCEF_MATH_FORCE_INLINE void Max( const TVector& a, const TVector& b, TVector& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const typename L::TVec va = L::Load4( Data( a ) );
        const typename L::TVec vb = L::Load4( Data( b ) );
        L::Store4( Data( out ), L::Select( L::CmpLT( va, vb ), vb, va ) );
    }

    /** The safe range in SIMD (division by the broadcast root), every other case through the primary template */
    static inline void Normalize( const TVector& a, TVector& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const typename L::TVec va = L::Load4( Data( a ) );
        const typename L::TVec squared = L::Dot4( va, va );
        if ( TKernels::IsSafeSquaredLength( L::Lane0( squared ) ) )
        {
            const typename L::TVec length = L::Sqrt( L::template Splat< 0 >( squared ) );
            L::Store4( Data( out ), L::Div( va, length ) );
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

#endif /* GUCEF_MATH_CTVECTOR4OPS_LANES_H ? */
