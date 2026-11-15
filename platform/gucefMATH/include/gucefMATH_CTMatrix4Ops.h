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

#ifndef GUCEF_MATH_CTMATRIX4OPS_H
#define GUCEF_MATH_CTMATRIX4OPS_H

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
 *  The operations of CTMatrix4D that SIMD backends may specialise (section 5.3 of the design), as static functions over
 *  the column-major element arrays (element (r, c) at c * 4 + r). This primary template is the scalar implementation
 *  for every kind and backend and defines the canonical evaluation order that every specialisation must reproduce bit
 *  for bit:
 *  - Multiply: element (r, c) of a * b is the 4 term dot product of row r of a and column c of b in the order of
 *    CTVector4DotOrder (floats ( p0 + p2 ) + ( p1 + p3 ); fixed point exact and rounded once; integers wrapping);
 *  - Transform: element r of m * v is the same dot product of row r of m and v;
 *  - Transpose: element moves only.
 *  Specialisations exist for Float32 only and change implementations, never results, layouts or signatures
 *  (gucefMATH_CTMatrix4Ops_Sse2.h, ..., included at the end of this header for the configured instruction set); the
 *  default backend is the one of the build (TMathSimdDefaultBackend). Lane r of the SIMD form of Transform is
 *  ( c0 vx + c2 vz ) + ( c1 vy + c3 vw ) over the columns c, which is the same dot product per row.
 *  NaN payloads with more than one NaN operand are not part of the bit identity.
 *  All functions are realtime-safe. In-place use (out aliasing an input) is allowed.
 */
template < typename T, typename TBackend = TMathSimdDefaultBackend >
class GUCEF_MATH_TEMPLATE_PUBLIC CTMatrix4Ops
{
    public:

    typedef CTNumericTraits< T > TTraits;
    typedef CTVector4DotOrder< T, TTraits::IsFloatingPoint > TDotOrder;

    static inline void Multiply( const T ( &a )[ 16 ], const T ( &b )[ 16 ], T ( &out )[ 16 ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        T result[ 16 ];
        for ( UInt32 r=0; r<4; ++r )
        {
            const T row[ 4 ] = { a[ r ], a[ 4 + r ], a[ 8 + r ], a[ 12 + r ] };
            for ( UInt32 c=0; c<4; ++c )
            {
                const T column[ 4 ] = { b[ c * 4 ], b[ c * 4 + 1 ], b[ c * 4 + 2 ], b[ c * 4 + 3 ] };
                result[ c * 4 + r ] = TDotOrder::Dot( row, column );
            }
        }
        for ( UInt32 i=0; i<16; ++i )
            out[ i ] = result[ i ];
    }

    static inline void Transform( const T ( &m )[ 16 ], const T ( &v )[ 4 ], T ( &out )[ 4 ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        T result[ 4 ];
        for ( UInt32 r=0; r<4; ++r )
        {
            const T row[ 4 ] = { m[ r ], m[ 4 + r ], m[ 8 + r ], m[ 12 + r ] };
            result[ r ] = TDotOrder::Dot( row, v );
        }
        for ( UInt32 i=0; i<4; ++i )
            out[ i ] = result[ i ];
    }

    static inline void Transpose( const T ( &m )[ 16 ], T ( &out )[ 16 ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        T result[ 16 ];
        for ( UInt32 r=0; r<4; ++r )
            for ( UInt32 c=0; c<4; ++c )
                result[ r * 4 + c ] = m[ c * 4 + r ];
        for ( UInt32 i=0; i<16; ++i )
            out[ i ] = result[ i ];
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
  #ifndef GUCEF_MATH_CTMATRIX4OPS_SSE2_H
  #include "gucefMATH_CTMatrix4Ops_Sse2.h"
  #define GUCEF_MATH_CTMATRIX4OPS_SSE2_H
  #endif /* GUCEF_MATH_CTMATRIX4OPS_SSE2_H ? */
#endif

#if ( GUCEF_MATH_SIMD_HAVE_NEON == 1 )
  #ifndef GUCEF_MATH_CTMATRIX4OPS_NEON_H
  #include "gucefMATH_CTMatrix4Ops_Neon.h"
  #define GUCEF_MATH_CTMATRIX4OPS_NEON_H
  #endif /* GUCEF_MATH_CTMATRIX4OPS_NEON_H ? */
#endif

#if ( GUCEF_MATH_SIMD_HAVE_WASM128 == 1 )
  #ifndef GUCEF_MATH_CTMATRIX4OPS_WASM128_H
  #include "gucefMATH_CTMatrix4Ops_Wasm128.h"
  #define GUCEF_MATH_CTMATRIX4OPS_WASM128_H
  #endif /* GUCEF_MATH_CTMATRIX4OPS_WASM128_H ? */
#endif

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTMATRIX4OPS_H ? */
