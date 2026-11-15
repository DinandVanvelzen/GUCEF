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

#ifndef GUCEF_MATH_CTQUATERNIONOPS_H
#define GUCEF_MATH_CTQUATERNIONOPS_H

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
 *  The operations of CTQuaternion that SIMD backends may specialise (section 5.3 of the design), as static functions
 *  over the components in storage order x, y, z, w. This primary template is the scalar implementation for every kind
 *  and backend and defines the canonical evaluation order that every specialisation must reproduce bit for bit:
 *  - Multiply (Hamilton product a b, b applied first): each component is one 4 term dot product in the order of
 *    CTVector4DotOrder over the operands
 *      x: ( aw, ax, ay, -az ) . ( bx, bw, bz, by )      y: ( aw, -ax, ay, az ) . ( by, bz, bw, bx )
 *      z: ( aw, ax, -ay, az ) . ( bz, by, bx, bw )      w: ( aw, -ax, -ay, -az ) . ( bw, bx, by, bz );
 *  - Rotate (v' = q v q*): t = 2 ( q.xyz x v ) with each cross component a 2 term dot product, then each component of
 *    v + w t + q.xyz x t as one 4 term dot product ( 1, w, qj, -qk ) . ( v, t, tk, tj );
 *  - Dot: CTVector4DotOrder; Normalize: CTVector4Normalization (the zero quaternion gives false).
 *  Fixed point rounds once per dot product (exact 128 bit sums), integers wrap.
 *  Specialisations exist for Float32 only (gucefMATH_CTQuaternionOps_Sse2.h, ..., included at the end of this header
 *  for the configured instruction set) and change implementations, never results; the default backend is the one of
 *  the build (TMathSimdDefaultBackend). NaN payloads with more than one NaN operand are not part of the bit identity.
 *  All functions are realtime-safe. In-place use (out aliasing an input) is allowed.
 */
template < typename T, typename TBackend = TMathSimdDefaultBackend >
class GUCEF_MATH_TEMPLATE_PUBLIC CTQuaternionOps
{
    public:

    typedef CTNumericTraits< T > TTraits;
    typedef typename CTMathComponentTraits< T >::TKernels TKernels;
    typedef CTVector4DotOrder< T, TTraits::IsFloatingPoint > TDotOrder;

    static inline void Multiply( const T ( &a )[ 4 ], const T ( &b )[ 4 ], T ( &out )[ 4 ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const T ax = a[ 0 ], ay = a[ 1 ], az = a[ 2 ], aw = a[ 3 ];
        const T bx = b[ 0 ], by = b[ 1 ], bz = b[ 2 ], bw = b[ 3 ];
        const T lx[ 4 ] = { aw, ax, ay, TKernels::Negate( az ) };
        const T rx[ 4 ] = { bx, bw, bz, by };
        const T ly[ 4 ] = { aw, TKernels::Negate( ax ), ay, az };
        const T ry[ 4 ] = { by, bz, bw, bx };
        const T lz[ 4 ] = { aw, ax, TKernels::Negate( ay ), az };
        const T rz[ 4 ] = { bz, by, bx, bw };
        const T lw[ 4 ] = { aw, TKernels::Negate( ax ), TKernels::Negate( ay ), TKernels::Negate( az ) };
        const T rw[ 4 ] = { bw, bx, by, bz };
        T x = TDotOrder::Dot( lx, rx );
        T y = TDotOrder::Dot( ly, ry );
        T z = TDotOrder::Dot( lz, rz );
        T w = TDotOrder::Dot( lw, rw );
        out[ 0 ] = x; out[ 1 ] = y; out[ 2 ] = z; out[ 3 ] = w;
    }

    static inline void Rotate( const T ( &q )[ 4 ], const T ( &v )[ 3 ], T ( &out )[ 3 ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        T t[ 3 ];
        Cross( q, v, t );
        for ( UInt32 i=0; i<3; ++i )
            t[ i ] = TKernels::Add( t[ i ], t[ i ] );
        T result[ 3 ];
        for ( UInt32 i=0; i<3; ++i )
        {
            const UInt32 j = ( i + 1 ) % 3;
            const UInt32 k = ( i + 2 ) % 3;
            const T l[ 4 ] = { TTraits::One(), q[ 3 ], q[ j ], TKernels::Negate( q[ k ] ) };
            const T r[ 4 ] = { v[ i ], t[ i ], t[ k ], t[ j ] };
            result[ i ] = TDotOrder::Dot( l, r );
        }
        for ( UInt32 i=0; i<3; ++i )
            out[ i ] = result[ i ];
    }

    static GUCEF_MATH_FORCE_INLINE T Dot( const T ( &a )[ 4 ], const T ( &b )[ 4 ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return TDotOrder::Dot( a, b );
    }

    static GUCEF_MATH_FORCE_INLINE bool Normalize( const T ( &a )[ 4 ], T ( &out )[ 4 ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTVector4Normalization< T, TTraits::IsFloatingPoint >::Normalize( a, out );
    }

    private:

    /** q.xyz x v, each component a 2 term dot product */
    static GUCEF_MATH_FORCE_INLINE void Cross( const T ( &q )[ 4 ], const T ( &v )[ 3 ], T ( &out )[ 3 ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        for ( UInt32 i=0; i<3; ++i )
        {
            const UInt32 j = ( i + 1 ) % 3;
            const UInt32 k = ( i + 2 ) % 3;
            const T l[ 2 ] = { q[ j ], TKernels::Negate( q[ k ] ) };
            const T r[ 2 ] = { v[ k ], v[ j ] };
            out[ i ] = TKernels::Dot( l, r );
        }
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------//
//                                                                         //
//      SPECIALISATIONS                                                    //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( GUCEF_MATH_SIMD_HAVE_SSE2 == 1 )
  #ifndef GUCEF_MATH_CTQUATERNIONOPS_SSE2_H
  #include "gucefMATH_CTQuaternionOps_Sse2.h"
  #define GUCEF_MATH_CTQUATERNIONOPS_SSE2_H
  #endif /* GUCEF_MATH_CTQUATERNIONOPS_SSE2_H ? */
#endif

#if ( GUCEF_MATH_SIMD_HAVE_NEON == 1 )
  #ifndef GUCEF_MATH_CTQUATERNIONOPS_NEON_H
  #include "gucefMATH_CTQuaternionOps_Neon.h"
  #define GUCEF_MATH_CTQUATERNIONOPS_NEON_H
  #endif /* GUCEF_MATH_CTQUATERNIONOPS_NEON_H ? */
#endif

#if ( GUCEF_MATH_SIMD_HAVE_WASM128 == 1 )
  #ifndef GUCEF_MATH_CTQUATERNIONOPS_WASM128_H
  #include "gucefMATH_CTQuaternionOps_Wasm128.h"
  #define GUCEF_MATH_CTQUATERNIONOPS_WASM128_H
  #endif /* GUCEF_MATH_CTQUATERNIONOPS_WASM128_H ? */
#endif

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTQUATERNIONOPS_H ? */
