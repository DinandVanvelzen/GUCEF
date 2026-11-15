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

#ifndef GUCEF_MATH_CTMATRIX4OPS_SSE2_H
#define GUCEF_MATH_CTMATRIX4OPS_SSE2_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <emmintrin.h>

#ifndef GUCEF_MATH_CTMATRIX4OPS_H
#include "gucefMATH_CTMatrix4Ops.h"
#define GUCEF_MATH_CTMATRIX4OPS_H
#endif /* GUCEF_MATH_CTMATRIX4OPS_H ? */

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
 *  SSE2 implementation of CTMatrix4Ops for Float32: the same results bit for bit as the primary template. Column c
 *  of a * b is ( a0 b0c + a2 b2c ) + ( a1 b1c + a3 b3c ) over the columns a0..a3 of a, which is in every lane the
 *  canonical row times column dot product. All inputs are read before the output is written (in-place use).
 */
template <>
class GUCEF_MATH_TEMPLATE_PUBLIC CTMatrix4Ops< Float32, SMathSimdSse2 >
{
    public:

    static inline void Multiply( const Float32 ( &a )[ 16 ], const Float32 ( &b )[ 16 ], Float32 ( &out )[ 16 ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const __m128 a0 = CMathSse2::Load4( a );
        const __m128 a1 = CMathSse2::Load4( a + 4 );
        const __m128 a2 = CMathSse2::Load4( a + 8 );
        const __m128 a3 = CMathSse2::Load4( a + 12 );
        const __m128 b0 = CMathSse2::Load4( b );
        const __m128 b1 = CMathSse2::Load4( b + 4 );
        const __m128 b2 = CMathSse2::Load4( b + 8 );
        const __m128 b3 = CMathSse2::Load4( b + 12 );
        const __m128 r0 = Column( a0, a1, a2, a3, b0 );
        const __m128 r1 = Column( a0, a1, a2, a3, b1 );
        const __m128 r2 = Column( a0, a1, a2, a3, b2 );
        const __m128 r3 = Column( a0, a1, a2, a3, b3 );
        CMathSse2::Store4( out, r0 );
        CMathSse2::Store4( out + 4, r1 );
        CMathSse2::Store4( out + 8, r2 );
        CMathSse2::Store4( out + 12, r3 );
    }

    static inline void Transform( const Float32 ( &m )[ 16 ], const Float32 ( &v )[ 4 ], Float32 ( &out )[ 4 ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const __m128 result = Column( CMathSse2::Load4( m ), CMathSse2::Load4( m + 4 ), CMathSse2::Load4( m + 8 ), CMathSse2::Load4( m + 12 ), CMathSse2::Load4( v ) );
        CMathSse2::Store4( out, result );
    }

    static inline void Transpose( const Float32 ( &m )[ 16 ], Float32 ( &out )[ 16 ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        __m128 c0 = CMathSse2::Load4( m );
        __m128 c1 = CMathSse2::Load4( m + 4 );
        __m128 c2 = CMathSse2::Load4( m + 8 );
        __m128 c3 = CMathSse2::Load4( m + 12 );
        _MM_TRANSPOSE4_PS( c0, c1, c2, c3 );
        CMathSse2::Store4( out, c0 );
        CMathSse2::Store4( out + 4, c1 );
        CMathSse2::Store4( out + 8, c2 );
        CMathSse2::Store4( out + 12, c3 );
    }

    private:

    /** The columns a0..a3 times the vector v: per lane ( a0 v0 + a2 v2 ) + ( a1 v1 + a3 v3 ) */
    static GUCEF_MATH_FORCE_INLINE __m128 Column( __m128 a0, __m128 a1, __m128 a2, __m128 a3, __m128 v ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return CMathSse2::Combine4( a0, a1, a2, a3, CMathSse2::Splat< 0 >( v ), CMathSse2::Splat< 1 >( v ), CMathSse2::Splat< 2 >( v ), CMathSse2::Splat< 3 >( v ) );
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

#endif /* GUCEF_MATH_CTMATRIX4OPS_SSE2_H ? */
