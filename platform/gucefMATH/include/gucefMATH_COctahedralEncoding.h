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

#ifndef GUCEF_MATH_COCTAHEDRALENCODING_H
#define GUCEF_MATH_COCTAHEDRALENCODING_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_MACROS_H
#include "gucefMATH_macros.h"
#define GUCEF_MATH_MACROS_H
#endif /* GUCEF_MATH_MACROS_H ? */

#ifndef GUCEF_MATH_VECTORS_H
#include "gucefMATH_vectors.h"
#define GUCEF_MATH_VECTORS_H
#endif /* GUCEF_MATH_VECTORS_H ? */

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
 *  Octahedral encoding of unit vectors (Cigolle, Donow, Evangelakos, Mara, McGuire and Meyer, "A Survey of Efficient
 *  Representations for Independent Unit Vectors", JCGT 2014): the sphere is mapped onto an octahedron, the octahedron
 *  folded onto the square [-1, 1] x [-1, 1] and the two coordinates quantised. The building block for replicating
 *  directions ( normals, velocities ); the wire format is decided by the replication layer.
 *
 *  - Project / Unproject are the continuous map between a vector and two components in [-1, 1] ( the lower hemisphere
 *    is folded into the corners ). The vector need not be normalised ( it is scaled to the octahedron ); the zero or
 *    a non finite vector gives +Z ( the point 0, 0 ).
 *  - Encode16 packs the two components as signed normalised 8 bit values ( round( x * 127 ), ties to even ) into a
 *    UInt16: the first component in the upper byte, two's complement. Encode32 does the same with 16 bit values
 *    ( round( x * 32767 ) ) into a UInt32. The six axis directions are exact in both. The largest angular error
 *    over 10^6 seeded directions is 0.95 degrees for 16 bits and 0.0037 degrees for 32 bits.
 *  - EncodePrecise16 / EncodePrecise32 examine the four codes around the exact point and return the one whose decoded
 *    vector is angularly nearest to the input; never worse than Encode; the largest error over the same directions
 *    is 0.63 degrees for 16 bits and 0.0025 degrees for 32 bits.
 *  - Decode16 / Decode32 return unit vectors ( normalised after unfolding ).
 *  - Float32 and Float64 overloads, compiled in the module without floating-point contraction ( the arithmetic is done
 *    in Float64, so the codes of a Float32 and a Float64 input of the same value agree ). Realtime-safe.
 */
class GUCEF_MATH_PUBLIC_CPP COctahedralEncoding
{
    public:

    /** The two octahedral components in [-1, 1] of the direction of 'v' */
    static void Project( const CTVector3D< Float32 >& v, Float32& u, Float32& w ) GUCEF_MATH_REALTIME_CONTEXT;
    static void Project( const CTVector3D< Float64 >& v, Float64& u, Float64& w ) GUCEF_MATH_REALTIME_CONTEXT;

    /** The unit vector of two octahedral components ( limited to [-1, 1] ) */
    static void Unproject( Float32 u, Float32 w, CTVector3D< Float32 >& out ) GUCEF_MATH_REALTIME_CONTEXT;
    static void Unproject( Float64 u, Float64 w, CTVector3D< Float64 >& out ) GUCEF_MATH_REALTIME_CONTEXT;

    static UInt16 Encode16( const CTVector3D< Float32 >& v ) GUCEF_MATH_REALTIME_CONTEXT;
    static UInt16 Encode16( const CTVector3D< Float64 >& v ) GUCEF_MATH_REALTIME_CONTEXT;
    static UInt32 Encode32( const CTVector3D< Float32 >& v ) GUCEF_MATH_REALTIME_CONTEXT;
    static UInt32 Encode32( const CTVector3D< Float64 >& v ) GUCEF_MATH_REALTIME_CONTEXT;

    static UInt16 EncodePrecise16( const CTVector3D< Float32 >& v ) GUCEF_MATH_REALTIME_CONTEXT;
    static UInt16 EncodePrecise16( const CTVector3D< Float64 >& v ) GUCEF_MATH_REALTIME_CONTEXT;
    static UInt32 EncodePrecise32( const CTVector3D< Float32 >& v ) GUCEF_MATH_REALTIME_CONTEXT;
    static UInt32 EncodePrecise32( const CTVector3D< Float64 >& v ) GUCEF_MATH_REALTIME_CONTEXT;

    static void Decode16( UInt16 code, CTVector3D< Float32 >& out ) GUCEF_MATH_REALTIME_CONTEXT;
    static void Decode16( UInt16 code, CTVector3D< Float64 >& out ) GUCEF_MATH_REALTIME_CONTEXT;
    static void Decode32( UInt32 code, CTVector3D< Float32 >& out ) GUCEF_MATH_REALTIME_CONTEXT;
    static void Decode32( UInt32 code, CTVector3D< Float64 >& out ) GUCEF_MATH_REALTIME_CONTEXT;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_COCTAHEDRALENCODING_H ? */
