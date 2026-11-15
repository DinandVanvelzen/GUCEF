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

#include "gucefMATH_NoiseKernels.h"

#include "gucefMATH_CValueNoise.h"

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

/** The lattice value of a hash: uniform in [-1, 1) */
template < typename T >
GUCEF_MATH_FORCE_INLINE T
LatticeValue( UInt32 hash )
{
    return T( 2 ) * HashToUnit< T >( hash ) - T( 1 );
}

template < typename T >
T
Value1D( UInt32 seed, T x )
{
    const Int32 xi = FloorToLattice( x );
    const T xf = x - (T) xi;
    return Lerp( LatticeValue< T >( Hash1( seed, xi ) ), LatticeValue< T >( Hash1( seed, xi + 1 ) ), Fade( xf ) );
}

template < typename T >
T
Value2D( UInt32 seed, T x, T y )
{
    const Int32 xi = FloorToLattice( x );
    const Int32 yi = FloorToLattice( y );
    const T xf = x - (T) xi;
    const T yf = y - (T) yi;
    const T u = Fade( xf );
    const T v = Fade( yf );
    const T bottom = Lerp( LatticeValue< T >( Hash2( seed, xi, yi ) ), LatticeValue< T >( Hash2( seed, xi + 1, yi ) ), u );
    const T top = Lerp( LatticeValue< T >( Hash2( seed, xi, yi + 1 ) ), LatticeValue< T >( Hash2( seed, xi + 1, yi + 1 ) ), u );
    return Lerp( bottom, top, v );
}

template < typename T >
T
Value3D( UInt32 seed, T x, T y, T z )
{
    const Int32 xi = FloorToLattice( x );
    const Int32 yi = FloorToLattice( y );
    const Int32 zi = FloorToLattice( z );
    const T xf = x - (T) xi;
    const T yf = y - (T) yi;
    const T zf = z - (T) zi;
    const T u = Fade( xf );
    const T v = Fade( yf );
    const T w = Fade( zf );
    T layer[ 2 ];
    for ( Int32 k=0; k<2; ++k )
    {
        const T bottom = Lerp( LatticeValue< T >( Hash3( seed, xi, yi, zi + k ) ), LatticeValue< T >( Hash3( seed, xi + 1, yi, zi + k ) ), u );
        const T top = Lerp( LatticeValue< T >( Hash3( seed, xi, yi + 1, zi + k ) ), LatticeValue< T >( Hash3( seed, xi + 1, yi + 1, zi + k ) ), u );
        layer[ k ] = Lerp( bottom, top, v );
    }
    return Lerp( layer[ 0 ], layer[ 1 ], w );
}

}

/*-------------------------------------------------------------------------*/

CValueNoise::CValueNoise( UInt32 seed ) GUCEF_MATH_REALTIME_CONTEXT
    : m_seed( seed ) ,
      m_preparedSeed( NoiseKernels::PrepareSeed( seed ) )
{
}

void
CValueNoise::Seed( UInt32 seed ) GUCEF_MATH_REALTIME_CONTEXT
{
    m_seed = seed;
    m_preparedSeed = NoiseKernels::PrepareSeed( seed );
}

Float32 CValueNoise::Noise1D( Float32 x ) const GUCEF_MATH_REALTIME_CONTEXT { return Clamp1( Value1D< Float32 >( m_preparedSeed, x ) ); }
Float64 CValueNoise::Noise1D( Float64 x ) const GUCEF_MATH_REALTIME_CONTEXT { return Clamp1( Value1D< Float64 >( m_preparedSeed, x ) ); }
Float32 CValueNoise::Noise2D( Float32 x, Float32 y ) const GUCEF_MATH_REALTIME_CONTEXT { return Clamp1( Value2D< Float32 >( m_preparedSeed, x, y ) ); }
Float64 CValueNoise::Noise2D( Float64 x, Float64 y ) const GUCEF_MATH_REALTIME_CONTEXT { return Clamp1( Value2D< Float64 >( m_preparedSeed, x, y ) ); }
Float32 CValueNoise::Noise3D( Float32 x, Float32 y, Float32 z ) const GUCEF_MATH_REALTIME_CONTEXT { return Clamp1( Value3D< Float32 >( m_preparedSeed, x, y, z ) ); }
Float64 CValueNoise::Noise3D( Float64 x, Float64 y, Float64 z ) const GUCEF_MATH_REALTIME_CONTEXT { return Clamp1( Value3D< Float64 >( m_preparedSeed, x, y, z ) ); }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
