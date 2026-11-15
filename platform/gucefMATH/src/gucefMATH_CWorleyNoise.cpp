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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <limits>

#include "gucefMATH_NoiseKernels.h"

#include "gucefMATH_CWorleyNoise.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

using namespace NoiseKernels;

/** Distance from the sample, whose position inside its cell is 'f', to the nearest edge of the cell at offset 'o' along one axis; 0 for the cell itself */
template < typename T >
GUCEF_MATH_FORCE_INLINE T
CellGap( Int32 o, T f )
{
    return o == 0 ? T( 0 ) : ( o > 0 ? (T) o - f : f - (T) ( o + 1 ) );
}

/** The ordering in which offsets are visited: the own cell first, the neighbours next, so the bounds prune the rest early */
GUCEF_MATH_FORCE_INLINE Int32
OffsetAt( UInt32 index )
{
    static const Int32 offsets[ 5 ] = { 0, -1, 1, -2, 2 };
    return offsets[ index ];
}

/** Adds the per axis distances in the metric: squared for Euclidean, summed for Manhattan, maximum for Chebyshev */
template < typename T >
GUCEF_MATH_FORCE_INLINE T
Combine( TWorleyMetric metric, T accumulated, T axisDistance )
{
    const T absolute = axisDistance < T( 0 ) ? -axisDistance : axisDistance;
    switch ( metric )
    {
        case WORLEYMETRIC_MANHATTAN: return accumulated + absolute;
        case WORLEYMETRIC_CHEBYSHEV: return absolute > accumulated ? absolute : accumulated;
        default: return accumulated + axisDistance * axisDistance;
    }
}

template < typename T >
GUCEF_MATH_FORCE_INLINE T
Finish( TWorleyMetric metric, T accumulated )
{
    return metric == WORLEYMETRIC_EUCLIDEAN ? (T) sqrt( (Float64) accumulated ) : accumulated;
}

/** Keeps the two smallest values */
template < typename T >
GUCEF_MATH_FORCE_INLINE void
Insert( T value, T& smallest, T& second )
{
    if ( value < smallest )
    {
        second = smallest;
        smallest = value;
    }
    else if ( value < second )
    {
        second = value;
    }
}

template < typename T >
void
Worley2D( UInt32 seed, TWorleyMetric metric, T jitter, T x, T y, T& f1, T& f2 )
{
    const Int32 xi = FloorToLattice( x );
    const Int32 yi = FloorToLattice( y );
    const T xf = x - (T) xi;
    const T yf = y - (T) yi;
    const T half = T( 0.5 );
    T smallest = std::numeric_limits< T >::max();
    T second = std::numeric_limits< T >::max();
    for ( UInt32 iy=0; iy<5; ++iy )
    {
        const Int32 oy = OffsetAt( iy );
        const T gapY = CellGap< T >( oy, yf );
        const T boundY = Combine< T >( metric, T( 0 ), gapY );
        if ( boundY >= second )
            continue;
        for ( UInt32 ix=0; ix<5; ++ix )
        {
            const Int32 ox = OffsetAt( ix );
            if ( Combine< T >( metric, boundY, CellGap< T >( ox, xf ) ) >= second )
                continue;
            const UInt32 hash = Hash2( seed, xi + ox, yi + oy );
            const T px = (T) ox + half + jitter * ( HashToUnit< T >( hash ) - half ) - xf;
            const T py = (T) oy + half + jitter * ( HashToUnit< T >( Mix( hash ^ 0x68E31DA4U ) ) - half ) - yf;
            Insert( Combine< T >( metric, Combine< T >( metric, T( 0 ), px ), py ), smallest, second );
        }
    }
    f1 = Finish( metric, smallest );
    f2 = Finish( metric, second );
}

template < typename T >
void
Worley3D( UInt32 seed, TWorleyMetric metric, T jitter, T x, T y, T z, T& f1, T& f2 )
{
    const Int32 xi = FloorToLattice( x );
    const Int32 yi = FloorToLattice( y );
    const Int32 zi = FloorToLattice( z );
    const T xf = x - (T) xi;
    const T yf = y - (T) yi;
    const T zf = z - (T) zi;
    const T half = T( 0.5 );
    T smallest = std::numeric_limits< T >::max();
    T second = std::numeric_limits< T >::max();
    for ( UInt32 iz=0; iz<5; ++iz )
    {
        const Int32 oz = OffsetAt( iz );
        const T boundZ = Combine< T >( metric, T( 0 ), CellGap< T >( oz, zf ) );
        if ( boundZ >= second )
            continue;
        for ( UInt32 iy=0; iy<5; ++iy )
        {
            const Int32 oy = OffsetAt( iy );
            const T boundY = Combine< T >( metric, boundZ, CellGap< T >( oy, yf ) );
            if ( boundY >= second )
                continue;
            for ( UInt32 ix=0; ix<5; ++ix )
            {
                const Int32 ox = OffsetAt( ix );
                if ( Combine< T >( metric, boundY, CellGap< T >( ox, xf ) ) >= second )
                    continue;
                const UInt32 hash = Hash3( seed, xi + ox, yi + oy, zi + oz );
                const UInt32 hashY = Mix( hash ^ 0x68E31DA4U );
                const UInt32 hashZ = Mix( hash ^ 0xB5297A4DU );
                const T px = (T) ox + half + jitter * ( HashToUnit< T >( hash ) - half ) - xf;
                const T py = (T) oy + half + jitter * ( HashToUnit< T >( hashY ) - half ) - yf;
                const T pz = (T) oz + half + jitter * ( HashToUnit< T >( hashZ ) - half ) - zf;
                Insert( Combine< T >( metric, Combine< T >( metric, Combine< T >( metric, T( 0 ), px ), py ), pz ), smallest, second );
            }
        }
    }
    f1 = Finish( metric, smallest );
    f2 = Finish( metric, second );
}

/** The selected function mapped to [-1, 1] */
template < typename T >
T
MapFeature( TWorleyFunction function, TWorleyMetric metric, T f1, T f2 )
{
    T range = T( 1 );
    T value = f1;
    if ( function == WORLEYFUNCTION_F2 )
    {
        value = f2;
        range = T( 1.5 );
    }
    else if ( function == WORLEYFUNCTION_F2_MINUS_F1 )
    {
        value = f2 - f1;
    }
    if ( metric == WORLEYMETRIC_MANHATTAN )
        range = range * T( 1.25 );
    else if ( metric == WORLEYMETRIC_CHEBYSHEV )
        range = range * T( 0.9 );
    T unit = value / range;
    if ( unit > T( 1 ) )
        unit = T( 1 );
    if ( !( unit > T( 0 ) ) )
        unit = T( 0 );
    return T( 2 ) * unit - T( 1 );
}

}

/*-------------------------------------------------------------------------*/

CWorleyNoise::CWorleyNoise( UInt32 seed, TWorleyMetric metric, TWorleyFunction function, Float32 jitter ) GUCEF_MATH_REALTIME_CONTEXT
    : m_seed( seed ) ,
      m_preparedSeed( NoiseKernels::PrepareSeed( seed ) ) ,
      m_metric( metric ) ,
      m_function( function ) ,
      m_jitter( 1.0f )
{
    SetJitter( jitter );
}

void
CWorleyNoise::Seed( UInt32 seed ) GUCEF_MATH_REALTIME_CONTEXT
{
    m_seed = seed;
    m_preparedSeed = NoiseKernels::PrepareSeed( seed );
}

void
CWorleyNoise::SetJitter( Float32 jitter ) GUCEF_MATH_REALTIME_CONTEXT
{
    m_jitter = jitter > 0.0f ? ( jitter < 1.0f ? jitter : 1.0f ) : 0.0f;
}

void CWorleyNoise::Evaluate2D( Float32 x, Float32 y, Float32& f1, Float32& f2 ) const GUCEF_MATH_REALTIME_CONTEXT { Worley2D< Float32 >( m_preparedSeed, m_metric, m_jitter, x, y, f1, f2 ); }
void CWorleyNoise::Evaluate2D( Float64 x, Float64 y, Float64& f1, Float64& f2 ) const GUCEF_MATH_REALTIME_CONTEXT { Worley2D< Float64 >( m_preparedSeed, m_metric, (Float64) m_jitter, x, y, f1, f2 ); }
void CWorleyNoise::Evaluate3D( Float32 x, Float32 y, Float32 z, Float32& f1, Float32& f2 ) const GUCEF_MATH_REALTIME_CONTEXT { Worley3D< Float32 >( m_preparedSeed, m_metric, m_jitter, x, y, z, f1, f2 ); }
void CWorleyNoise::Evaluate3D( Float64 x, Float64 y, Float64 z, Float64& f1, Float64& f2 ) const GUCEF_MATH_REALTIME_CONTEXT { Worley3D< Float64 >( m_preparedSeed, m_metric, (Float64) m_jitter, x, y, z, f1, f2 ); }

Float32
CWorleyNoise::Noise2D( Float32 x, Float32 y ) const GUCEF_MATH_REALTIME_CONTEXT
{
    Float32 f1, f2;
    Evaluate2D( x, y, f1, f2 );
    return MapFeature< Float32 >( m_function, m_metric, f1, f2 );
}

Float64
CWorleyNoise::Noise2D( Float64 x, Float64 y ) const GUCEF_MATH_REALTIME_CONTEXT
{
    Float64 f1, f2;
    Evaluate2D( x, y, f1, f2 );
    return MapFeature< Float64 >( m_function, m_metric, f1, f2 );
}

Float32
CWorleyNoise::Noise3D( Float32 x, Float32 y, Float32 z ) const GUCEF_MATH_REALTIME_CONTEXT
{
    Float32 f1, f2;
    Evaluate3D( x, y, z, f1, f2 );
    return MapFeature< Float32 >( m_function, m_metric, f1, f2 );
}

Float64
CWorleyNoise::Noise3D( Float64 x, Float64 y, Float64 z ) const GUCEF_MATH_REALTIME_CONTEXT
{
    Float64 f1, f2;
    Evaluate3D( x, y, z, f1, f2 );
    return MapFeature< Float64 >( m_function, m_metric, f1, f2 );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
