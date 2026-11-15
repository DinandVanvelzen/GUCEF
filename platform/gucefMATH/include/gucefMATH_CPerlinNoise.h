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

#ifndef GUCEF_MATH_CPERLINNOISE_H
#define GUCEF_MATH_CPERLINNOISE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_MACROS_H
#include "gucefMATH_macros.h"
#define GUCEF_MATH_MACROS_H
#endif /* GUCEF_MATH_MACROS_H ? */

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
 *  Gradient noise after the improved noise of Ken Perlin (2002) in one to four dimensions: a lattice of random
 *  gradients, a quintic interpolation weight ( 6 t^5 - 15 t^4 + 10 t^3 ) and the gradient sets of the edges of the
 *  cube and hypercube; the 1D and 2D forms use unit gradients ( two, and sixteen directions 22.5 degrees apart ).
 *
 *  - A value type: the permutation table of 256 entries belongs to the object ( built from the seed, so equal seeds
 *    give equal noise; copy it to keep a noise ), there is no global state, concurrent const use is safe.
 *  - Results are in [-1, 1]: scaled so that the largest possible value is 1 and clamped to guard the last rounding.
 *    The function is 0 at every integer lattice point, continuous ( and C2 inside the cell by the weight ), and
 *    periodic with period 256 per axis.
 *  - Float32 and Float64 overloads, compiled in the module without floating-point contraction: only +, -, * , floor
 *    and integer hashing are used, so the same seed gives the same values on every platform. Coordinates beyond
 *    +- 10^9 are clamped. Sampling is realtime-safe; construction and Seed() are cheap and allocate nothing.
 */
class GUCEF_MATH_PUBLIC_CPP CPerlinNoise
{
    public:

    /** Number of entries of the permutation table */
    enum { TABLE_SIZE = 256 };

    explicit CPerlinNoise( UInt32 seed = 0 ) GUCEF_MATH_REALTIME_CONTEXT;

    /** Rebuilds the permutation table from the seed: a shuffle by CSplitMix64( seed ) */
    void Seed( UInt32 seed ) GUCEF_MATH_REALTIME_CONTEXT;

    GUCEF_MATH_FORCE_INLINE UInt32 GetSeed( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_seed; }

    /** The permutation of 0 .. 255 of this noise ( entries 256 .. 511 repeat it ) */
    GUCEF_MATH_FORCE_INLINE const UInt8* GetPermutationTable( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_permutation; }

    Float32 Noise1D( Float32 x ) const GUCEF_MATH_REALTIME_CONTEXT;
    Float64 Noise1D( Float64 x ) const GUCEF_MATH_REALTIME_CONTEXT;
    Float32 Noise2D( Float32 x, Float32 y ) const GUCEF_MATH_REALTIME_CONTEXT;
    Float64 Noise2D( Float64 x, Float64 y ) const GUCEF_MATH_REALTIME_CONTEXT;
    Float32 Noise3D( Float32 x, Float32 y, Float32 z ) const GUCEF_MATH_REALTIME_CONTEXT;
    Float64 Noise3D( Float64 x, Float64 y, Float64 z ) const GUCEF_MATH_REALTIME_CONTEXT;
    Float32 Noise4D( Float32 x, Float32 y, Float32 z, Float32 w ) const GUCEF_MATH_REALTIME_CONTEXT;
    Float64 Noise4D( Float64 x, Float64 y, Float64 z, Float64 w ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Noises with the same seed are equal */
    bool operator==( const CPerlinNoise& other ) const { return m_seed == other.m_seed; }
    bool operator!=( const CPerlinNoise& other ) const { return m_seed != other.m_seed; }

    private:

    UInt32 m_seed;
    UInt8 m_permutation[ 2 * TABLE_SIZE ];
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CPERLINNOISE_H ? */
