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

#ifndef GUCEF_MATH_CTMATRIX4OPS_LANES_H
#define GUCEF_MATH_CTMATRIX4OPS_LANES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTMATRIX4OPS_H
#include "gucefMATH_CTMatrix4Ops.h"
#define GUCEF_MATH_CTMATRIX4OPS_H
#endif /* GUCEF_MATH_CTMATRIX4OPS_H ? */

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
 *  The operations of CTMatrix4Ops for Float32 written once over a lane policy L (see gucefMATH_CTVector4Ops_Lanes.h):
 *  the same results bit for bit as the primary template. Column c of a * b is ( a0 b0c + a2 b2c ) + ( a1 b1c + a3 b3c )
 *  over the columns a0..a3 of a, which is in every lane the canonical row times column dot product. All inputs are read
 *  before the output is written (in-place use).
 */
template < class L >
class CTMatrix4OpsLanes
{
    public:

    static inline void Multiply( const Float32 ( &a )[ 16 ], const Float32 ( &b )[ 16 ], Float32 ( &out )[ 16 ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const typename L::TVec a0 = L::Load4( a );
        const typename L::TVec a1 = L::Load4( a + 4 );
        const typename L::TVec a2 = L::Load4( a + 8 );
        const typename L::TVec a3 = L::Load4( a + 12 );
        const typename L::TVec b0 = L::Load4( b );
        const typename L::TVec b1 = L::Load4( b + 4 );
        const typename L::TVec b2 = L::Load4( b + 8 );
        const typename L::TVec b3 = L::Load4( b + 12 );
        const typename L::TVec r0 = Column( a0, a1, a2, a3, b0 );
        const typename L::TVec r1 = Column( a0, a1, a2, a3, b1 );
        const typename L::TVec r2 = Column( a0, a1, a2, a3, b2 );
        const typename L::TVec r3 = Column( a0, a1, a2, a3, b3 );
        L::Store4( out, r0 );
        L::Store4( out + 4, r1 );
        L::Store4( out + 8, r2 );
        L::Store4( out + 12, r3 );
    }

    static inline void Transform( const Float32 ( &m )[ 16 ], const Float32 ( &v )[ 4 ], Float32 ( &out )[ 4 ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const typename L::TVec result = Column( L::Load4( m ), L::Load4( m + 4 ), L::Load4( m + 8 ), L::Load4( m + 12 ), L::Load4( v ) );
        L::Store4( out, result );
    }

    static inline void Transpose( const Float32 ( &m )[ 16 ], Float32 ( &out )[ 16 ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        typename L::TVec c0 = L::Load4( m );
        typename L::TVec c1 = L::Load4( m + 4 );
        typename L::TVec c2 = L::Load4( m + 8 );
        typename L::TVec c3 = L::Load4( m + 12 );
        L::Transpose4( c0, c1, c2, c3 );
        L::Store4( out, c0 );
        L::Store4( out + 4, c1 );
        L::Store4( out + 8, c2 );
        L::Store4( out + 12, c3 );
    }

    private:

    /** The columns a0..a3 times the vector v: per lane ( a0 v0 + a2 v2 ) + ( a1 v1 + a3 v3 ) */
    static GUCEF_MATH_FORCE_INLINE typename L::TVec Column( typename L::TVec a0, typename L::TVec a1, typename L::TVec a2, typename L::TVec a3, typename L::TVec v ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return L::Combine4( a0, a1, a2, a3, L::template Splat< 0 >( v ), L::template Splat< 1 >( v ), L::template Splat< 2 >( v ), L::template Splat< 3 >( v ) );
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

#endif /* GUCEF_MATH_CTMATRIX4OPS_LANES_H ? */
