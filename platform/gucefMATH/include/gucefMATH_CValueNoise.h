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

#ifndef GUCEF_MATH_CVALUENOISE_H
#define GUCEF_MATH_CVALUENOISE_H

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
 *  Value noise in one to three dimensions: pseudo random values on the integer lattice, blended with the quintic
 *  weight 6 t^5 - 15 t^4 + 10 t^3. Cheaper and blockier than gradient noise ( CPerlinNoise ).
 *
 *  - A value type without tables: the values are a hash of the seed and the lattice coordinates; equal seeds give
 *    equal noise on every platform. There is no global state, concurrent const use is safe.
 *  - Results are in [-1, 1]: the lattice values are uniform in [-1, 1) and the blend is a convex combination, so the
 *    range holds without scaling; the noise equals the lattice value at integer coordinates.
 *  - Float32 and Float64 overloads, compiled in the module without floating-point contraction. Coordinates beyond
 *    +- 10^9 are clamped. Realtime-safe.
 */
class GUCEF_MATH_PUBLIC_CPP CValueNoise
{
    public:

    explicit CValueNoise( UInt32 seed = 0 ) GUCEF_MATH_REALTIME_CONTEXT;

    void Seed( UInt32 seed ) GUCEF_MATH_REALTIME_CONTEXT;

    GUCEF_MATH_FORCE_INLINE UInt32 GetSeed( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_seed; }

    Float32 Noise1D( Float32 x ) const GUCEF_MATH_REALTIME_CONTEXT;
    Float64 Noise1D( Float64 x ) const GUCEF_MATH_REALTIME_CONTEXT;
    Float32 Noise2D( Float32 x, Float32 y ) const GUCEF_MATH_REALTIME_CONTEXT;
    Float64 Noise2D( Float64 x, Float64 y ) const GUCEF_MATH_REALTIME_CONTEXT;
    Float32 Noise3D( Float32 x, Float32 y, Float32 z ) const GUCEF_MATH_REALTIME_CONTEXT;
    Float64 Noise3D( Float64 x, Float64 y, Float64 z ) const GUCEF_MATH_REALTIME_CONTEXT;

    bool operator==( const CValueNoise& other ) const { return m_seed == other.m_seed; }
    bool operator!=( const CValueNoise& other ) const { return m_seed != other.m_seed; }

    private:

    UInt32 m_seed;
    UInt32 m_preparedSeed;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CVALUENOISE_H ? */
