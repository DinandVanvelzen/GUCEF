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

#ifndef GUCEF_MATH_COPENSIMPLEX2NOISE_H
#define GUCEF_MATH_COPENSIMPLEX2NOISE_H

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
 *  Simplex type gradient noise in two and three dimensions, implemented from the published description of the
 *  OpenSimplex2 family ( public domain algorithm description ): a lattice with few neighbours per point, a radial
 *  kernel ( r^2 - d^2 )^4 per lattice point that falls to zero before the neighbouring lattice points, and a pseudo
 *  random gradient per point. This is free of the axis aligned artefacts of lattice noise.
 *
 *  - 2D: the skewed triangular lattice, three points per sample ( kernel radius^2 1/2 ), 16 unit gradients.
 *  - 3D: the input is reflected about the main diagonal of the cube ( so that the XY plane is not a lattice plane ),
 *    and the body centred cubic lattice, two interleaved cubic lattices, is evaluated at the 8 corners of the cell of
 *    each lattice ( kernel radius^2 3/4, which is shorter than the cell, so no other point contributes ).
 *  - A value type without tables: gradients come from a hash of the seed and the lattice point; equal seeds give equal
 *    noise on every platform. No global state, concurrent const use is safe.
 *  - Results are in [-1, 1]: scaled so that the largest value is about 1 and clamped. Continuous ( C3 ).
 *  - Float32 and Float64 overloads, compiled in the module without floating-point contraction. Coordinates beyond
 *    +- 10^9 are clamped. Realtime-safe.
 */
class GUCEF_MATH_PUBLIC_CPP COpenSimplex2Noise
{
    public:

    explicit COpenSimplex2Noise( UInt32 seed = 0 ) GUCEF_MATH_REALTIME_CONTEXT;

    void Seed( UInt32 seed ) GUCEF_MATH_REALTIME_CONTEXT;

    GUCEF_MATH_FORCE_INLINE UInt32 GetSeed( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_seed; }

    Float32 Noise2D( Float32 x, Float32 y ) const GUCEF_MATH_REALTIME_CONTEXT;
    Float64 Noise2D( Float64 x, Float64 y ) const GUCEF_MATH_REALTIME_CONTEXT;
    Float32 Noise3D( Float32 x, Float32 y, Float32 z ) const GUCEF_MATH_REALTIME_CONTEXT;
    Float64 Noise3D( Float64 x, Float64 y, Float64 z ) const GUCEF_MATH_REALTIME_CONTEXT;

    bool operator==( const COpenSimplex2Noise& other ) const { return m_seed == other.m_seed; }
    bool operator!=( const COpenSimplex2Noise& other ) const { return m_seed != other.m_seed; }

    private:

    UInt32 m_seed;
    UInt32 m_preparedSeed;
    UInt32 m_preparedSeedB;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_COPENSIMPLEX2NOISE_H ? */
