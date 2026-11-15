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

#ifndef GUCEF_MATH_NOISEKERNELS_H
#define GUCEF_MATH_NOISEKERNELS_H

/*
 *  Private helpers shared by the noise classes of the module ( not installed ): the integer hash that turns a seed and
 *  lattice coordinates into random bits, the lattice floor, the gradient tables and the interpolation weight. Only
 *  integer operations, additions, subtractions and multiplications are used, so results are identical on every
 *  platform ( the module is compiled without floating-point contraction ).
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>

#ifndef GUCEF_MATH_MACROS_H
#include "gucefMATH_macros.h"
#define GUCEF_MATH_MACROS_H
#endif /* GUCEF_MATH_MACROS_H ? */

#ifndef GUCEF_MATH_SCALARFUNCTIONS_H
#include "gucefMATH_ScalarFunctions.h"
#define GUCEF_MATH_SCALARFUNCTIONS_H
#endif /* GUCEF_MATH_SCALARFUNCTIONS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {
namespace NoiseKernels {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Largest lattice coordinate magnitude; coordinates beyond it are clamped to it */
enum { LATTICE_LIMIT = 1000000000 };

/** Avalanche step ( the finaliser of the 32 bit Murmur3 hash, Austin Appleby, public domain ) */
GUCEF_MATH_FORCE_INLINE UInt32
Mix( UInt32 h ) GUCEF_MATH_REALTIME_CONTEXT
{
    h ^= h >> 16;
    h *= 0x85EBCA6BU;
    h ^= h >> 13;
    h *= 0xC2B2AE35U;
    h ^= h >> 16;
    return h;
}

/** The seed prepared for hashing: seeds that differ in one bit give unrelated hashes */
GUCEF_MATH_FORCE_INLINE UInt32
PrepareSeed( UInt32 seed ) GUCEF_MATH_REALTIME_CONTEXT
{
    return Mix( seed ^ 0x9E3779B9U ) + 0x7F4A7C15U;
}

GUCEF_MATH_FORCE_INLINE UInt32
Hash1( UInt32 preparedSeed, Int32 x ) GUCEF_MATH_REALTIME_CONTEXT
{
    return Mix( preparedSeed ^ ( (UInt32) x * 0x27D4EB2DU ) );
}

GUCEF_MATH_FORCE_INLINE UInt32
Hash2( UInt32 preparedSeed, Int32 x, Int32 y ) GUCEF_MATH_REALTIME_CONTEXT
{
    return Mix( Hash1( preparedSeed, x ) ^ ( (UInt32) y * 0x165667B1U ) );
}

GUCEF_MATH_FORCE_INLINE UInt32
Hash3( UInt32 preparedSeed, Int32 x, Int32 y, Int32 z ) GUCEF_MATH_REALTIME_CONTEXT
{
    return Mix( Hash2( preparedSeed, x, y ) ^ ( (UInt32) z * 0x9E3779B1U ) );
}

/** [0, 1) from the upper 24 bits of a hash */
template < typename T >
GUCEF_MATH_FORCE_INLINE T
HashToUnit( UInt32 hash ) GUCEF_MATH_REALTIME_CONTEXT
{
    return (T) ( hash >> 8 ) * T( 5.9604644775390625e-8 );
}

/** The largest integer not above v, with v limited to the lattice range; a NaN gives -LATTICE_LIMIT */
template < typename T >
GUCEF_MATH_FORCE_INLINE Int32
FloorToLattice( T v ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( !( v > T( -(Int32) LATTICE_LIMIT ) ) )
        return -(Int32) LATTICE_LIMIT;
    if ( v >= T( (Int32) LATTICE_LIMIT ) )
        return (Int32) LATTICE_LIMIT;
    const Int32 i = (Int32) v;
    return (T) i > v ? i - 1 : i;
}

/** Interpolation weight of the improved noise: 6 t^5 - 15 t^4 + 10 t^3 */
template < typename T >
GUCEF_MATH_FORCE_INLINE T
Fade( const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    return SmootherStep( T( 0 ), T( 1 ), t );
}

template < typename T >
GUCEF_MATH_FORCE_INLINE T
Clamp1( const T& v ) GUCEF_MATH_REALTIME_CONTEXT
{
    return v > T( 1 ) ? T( 1 ) : ( v < T( -1 ) ? T( -1 ) : v );
}

/*-------------------------------------------------------------------------*/

/** 16 unit gradient directions in the plane, 22.5 degrees apart; every 45 degrees entry is a diagonal */
GUCEF_MATH_FORCE_INLINE const Float64*
GetGradients2D( UInt32 index ) GUCEF_MATH_REALTIME_CONTEXT
{
    static const Float64 gradients[ 16 ][ 2 ] =
    {
        {  1.0,                   0.0                   }, {  0.92387953251128675613,  0.38268343236508977173 },
        {  0.70710678118654752440,  0.70710678118654752440 }, {  0.38268343236508977173,  0.92387953251128675613 },
        {  0.0,                   1.0                   }, { -0.38268343236508977173,  0.92387953251128675613 },
        { -0.70710678118654752440,  0.70710678118654752440 }, { -0.92387953251128675613,  0.38268343236508977173 },
        { -1.0,                   0.0                   }, { -0.92387953251128675613, -0.38268343236508977173 },
        { -0.70710678118654752440, -0.70710678118654752440 }, { -0.38268343236508977173, -0.92387953251128675613 },
        {  0.0,                  -1.0                   }, {  0.38268343236508977173, -0.92387953251128675613 },
        {  0.70710678118654752440, -0.70710678118654752440 }, {  0.92387953251128675613, -0.38268343236508977173 }
    };
    return gradients[ index & 15 ];
}

/** The 12 edge directions of a cube ( length sqrt( 2 ) ), 4 of them repeated to fill 16 entries (Perlin 2002) */
GUCEF_MATH_FORCE_INLINE const Int8*
GetGradients3D( UInt32 index ) GUCEF_MATH_REALTIME_CONTEXT
{
    static const Int8 gradients[ 16 ][ 3 ] =
    {
        {  1,  1,  0 }, { -1,  1,  0 }, {  1, -1,  0 }, { -1, -1,  0 },
        {  1,  0,  1 }, { -1,  0,  1 }, {  1,  0, -1 }, { -1,  0, -1 },
        {  0,  1,  1 }, {  0, -1,  1 }, {  0,  1, -1 }, {  0, -1, -1 },
        {  1,  1,  0 }, { -1,  1,  0 }, {  0, -1,  1 }, {  0, -1, -1 }
    };
    return gradients[ index & 15 ];
}

/** The 32 edge directions of a hypercube with one zero component ( length sqrt( 3 ) ) */
GUCEF_MATH_FORCE_INLINE const Int8*
GetGradients4D( UInt32 index ) GUCEF_MATH_REALTIME_CONTEXT
{
    static const Int8 gradients[ 32 ][ 4 ] =
    {
        {  0,  1,  1,  1 }, {  0,  1,  1, -1 }, {  0,  1, -1,  1 }, {  0,  1, -1, -1 },
        {  0, -1,  1,  1 }, {  0, -1,  1, -1 }, {  0, -1, -1,  1 }, {  0, -1, -1, -1 },
        {  1,  0,  1,  1 }, {  1,  0,  1, -1 }, {  1,  0, -1,  1 }, {  1,  0, -1, -1 },
        { -1,  0,  1,  1 }, { -1,  0,  1, -1 }, { -1,  0, -1,  1 }, { -1,  0, -1, -1 },
        {  1,  1,  0,  1 }, {  1,  1,  0, -1 }, {  1, -1,  0,  1 }, {  1, -1,  0, -1 },
        { -1,  1,  0,  1 }, { -1,  1,  0, -1 }, { -1, -1,  0,  1 }, { -1, -1,  0, -1 },
        {  1,  1,  1,  0 }, {  1,  1, -1,  0 }, {  1, -1,  1,  0 }, {  1, -1, -1,  0 },
        { -1,  1,  1,  0 }, { -1,  1, -1,  0 }, { -1, -1,  1,  0 }, { -1, -1, -1,  0 }
    };
    return gradients[ index & 31 ];
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace NoiseKernels */
}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_NOISEKERNELS_H ? */
