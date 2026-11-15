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

#ifndef GUCEF_MATH_CWORLEYNOISE_H
#define GUCEF_MATH_CWORLEYNOISE_H

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

/** The distance measure of CWorleyNoise */
enum EWorleyMetric
{
    WORLEYMETRIC_EUCLIDEAN = 0,     /**< sqrt( dx^2 + dy^2 ) */
    WORLEYMETRIC_MANHATTAN,         /**< |dx| + |dy| */
    WORLEYMETRIC_CHEBYSHEV          /**< max( |dx|, |dy| ) */
};

typedef enum EWorleyMetric TWorleyMetric;

/** The feature distance that CWorleyNoise::Noise2D / Noise3D return */
enum EWorleyFunction
{
    WORLEYFUNCTION_F1 = 0,          /**< distance to the nearest feature point */
    WORLEYFUNCTION_F2,              /**< distance to the second nearest feature point */
    WORLEYFUNCTION_F2_MINUS_F1      /**< F2 - F1: bright cell borders */
};

typedef enum EWorleyFunction TWorleyFunction;

/**
 *  Cellular noise (Worley 1996) in two and three dimensions: one feature point per unit cell, at a pseudo random
 *  position; the noise is a function of the distances F1 <= F2 to the nearest and second nearest feature point.
 *
 *  - Evaluate2D / Evaluate3D give the exact F1 and F2 in cell units ( the cells within 2 of the sample cell are
 *    searched, cells that cannot hold a nearer point are pruned by their distance bound ). 'jitter' in [0, 1] scales the
 *    offset of a point from its cell centre ( 0 = a regular grid of cell centres, 1 = anywhere in the cell ).
 *  - Noise2D / Noise3D return the selected function mapped to [-1, 1]: 2 min( F / range, 1 ) - 1 with the range 1 for
 *    F1 and F2 - F1 and 1.5 for F2, times 1.25 for the Manhattan and 0.9 for the Chebyshev metric. So F1 is -1 at a
 *    feature point and rises towards the cell borders.
 *  - A value type without tables: feature points are a hash of the seed and the cell; equal seeds give equal noise on
 *    every platform ( square root is the only operation beyond +, -, * and comparison ). No global state, concurrent
 *    const use is safe.
 *  - Float32 and Float64 overloads, compiled in the module without floating-point contraction. Coordinates beyond
 *    +- 10^9 are clamped. Realtime-safe.
 */
class GUCEF_MATH_PUBLIC_CPP CWorleyNoise
{
    public:

    explicit CWorleyNoise( UInt32 seed = 0, TWorleyMetric metric = WORLEYMETRIC_EUCLIDEAN, TWorleyFunction function = WORLEYFUNCTION_F1, Float32 jitter = 1.0f ) GUCEF_MATH_REALTIME_CONTEXT;

    void Seed( UInt32 seed ) GUCEF_MATH_REALTIME_CONTEXT;
    GUCEF_MATH_FORCE_INLINE UInt32 GetSeed( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_seed; }

    GUCEF_MATH_FORCE_INLINE void SetMetric( TWorleyMetric metric ) GUCEF_MATH_REALTIME_CONTEXT { m_metric = metric; }
    GUCEF_MATH_FORCE_INLINE TWorleyMetric GetMetric( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_metric; }

    GUCEF_MATH_FORCE_INLINE void SetFunction( TWorleyFunction function ) GUCEF_MATH_REALTIME_CONTEXT { m_function = function; }
    GUCEF_MATH_FORCE_INLINE TWorleyFunction GetFunction( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_function; }

    /** Limited to [0, 1] */
    void SetJitter( Float32 jitter ) GUCEF_MATH_REALTIME_CONTEXT;
    GUCEF_MATH_FORCE_INLINE Float32 GetJitter( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_jitter; }

    /** The distances to the nearest and second nearest feature point, f1 <= f2, in the metric of this noise */
    void Evaluate2D( Float32 x, Float32 y, Float32& f1, Float32& f2 ) const GUCEF_MATH_REALTIME_CONTEXT;
    void Evaluate2D( Float64 x, Float64 y, Float64& f1, Float64& f2 ) const GUCEF_MATH_REALTIME_CONTEXT;
    void Evaluate3D( Float32 x, Float32 y, Float32 z, Float32& f1, Float32& f2 ) const GUCEF_MATH_REALTIME_CONTEXT;
    void Evaluate3D( Float64 x, Float64 y, Float64 z, Float64& f1, Float64& f2 ) const GUCEF_MATH_REALTIME_CONTEXT;

    Float32 Noise2D( Float32 x, Float32 y ) const GUCEF_MATH_REALTIME_CONTEXT;
    Float64 Noise2D( Float64 x, Float64 y ) const GUCEF_MATH_REALTIME_CONTEXT;
    Float32 Noise3D( Float32 x, Float32 y, Float32 z ) const GUCEF_MATH_REALTIME_CONTEXT;
    Float64 Noise3D( Float64 x, Float64 y, Float64 z ) const GUCEF_MATH_REALTIME_CONTEXT;

    bool operator==( const CWorleyNoise& other ) const
    {
        return m_seed == other.m_seed && m_metric == other.m_metric && m_function == other.m_function && m_jitter == other.m_jitter;
    }
    bool operator!=( const CWorleyNoise& other ) const { return !( *this == other ); }

    private:

    UInt32 m_seed;
    UInt32 m_preparedSeed;
    TWorleyMetric m_metric;
    TWorleyFunction m_function;
    Float32 m_jitter;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CWORLEYNOISE_H ? */
