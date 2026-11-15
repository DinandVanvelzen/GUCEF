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

#ifndef GUCEF_MATH_CTRANDOMSTREAM_H
#include "gucefMATH_CTRandomStream.h"
#define GUCEF_MATH_CTRANDOMSTREAM_H
#endif /* GUCEF_MATH_CTRANDOMSTREAM_H ? */

#include "gucefMATH_NoiseKernels.h"

#include "gucefMATH_CPerlinNoise.h"

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

/** Factors that bring the largest value of each dimension to 1 ( the extremes of the gradient sets: the maximum over the cell of the interpolation weights times the best gradient of each corner, 0.70711, 1.03635 and 1.53651 for 2D, 3D and 4D ) */
const Float64 g_scale1D = 2.0;
const Float64 g_scale2D = 1.41421356237309504880;
const Float64 g_scale3D = 0.9649;
const Float64 g_scale4D = 0.6508;

template < typename T >
GUCEF_MATH_FORCE_INLINE T
Dot2( UInt32 hash, T dx, T dy )
{
    const Float64* g = GetGradients2D( hash );
    return T( g[ 0 ] ) * dx + T( g[ 1 ] ) * dy;
}

template < typename T >
GUCEF_MATH_FORCE_INLINE T
Dot3( UInt32 hash, T dx, T dy, T dz )
{
    const Int8* g = GetGradients3D( hash );
    return T( g[ 0 ] ) * dx + T( g[ 1 ] ) * dy + T( g[ 2 ] ) * dz;
}

template < typename T >
GUCEF_MATH_FORCE_INLINE T
Dot4( UInt32 hash, T dx, T dy, T dz, T dw )
{
    const Int8* g = GetGradients4D( hash );
    return T( g[ 0 ] ) * dx + T( g[ 1 ] ) * dy + T( g[ 2 ] ) * dz + T( g[ 3 ] ) * dw;
}

/*-------------------------------------------------------------------------*/

template < typename T >
T
Perlin1D( const UInt8* p, T x )
{
    const Int32 xi = FloorToLattice( x );
    const T xf = x - (T) xi;
    const UInt32 x0 = (UInt32) xi & 255U;
    const UInt32 x1 = ( x0 + 1U ) & 255U;
    const T n0 = ( p[ x0 ] & 1 ) ? -xf : xf;
    const T n1 = ( p[ x1 ] & 1 ) ? -( xf - T( 1 ) ) : ( xf - T( 1 ) );
    return Clamp1( (T) g_scale1D * Lerp( n0, n1, Fade( xf ) ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
T
Perlin2D( const UInt8* p, T x, T y )
{
    const Int32 xi = FloorToLattice( x );
    const Int32 yi = FloorToLattice( y );
    const T xf = x - (T) xi;
    const T yf = y - (T) yi;
    const UInt32 x0 = (UInt32) xi & 255U;
    const UInt32 y0 = (UInt32) yi & 255U;
    const UInt32 x1 = ( x0 + 1U ) & 255U;
    const UInt32 y1 = ( y0 + 1U ) & 255U;
    const T one = T( 1 );

    const T n00 = Dot2( p[ p[ x0 ] + y0 ], xf, yf );
    const T n10 = Dot2( p[ p[ x1 ] + y0 ], xf - one, yf );
    const T n01 = Dot2( p[ p[ x0 ] + y1 ], xf, yf - one );
    const T n11 = Dot2( p[ p[ x1 ] + y1 ], xf - one, yf - one );
    const T u = Fade( xf );
    const T v = Fade( yf );
    return Clamp1( (T) g_scale2D * Lerp( Lerp( n00, n10, u ), Lerp( n01, n11, u ), v ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
T
Perlin3D( const UInt8* p, T x, T y, T z )
{
    const Int32 xi = FloorToLattice( x );
    const Int32 yi = FloorToLattice( y );
    const Int32 zi = FloorToLattice( z );
    const T xf = x - (T) xi;
    const T yf = y - (T) yi;
    const T zf = z - (T) zi;
    const UInt32 x0 = (UInt32) xi & 255U;
    const UInt32 y0 = (UInt32) yi & 255U;
    const UInt32 z0 = (UInt32) zi & 255U;
    const UInt32 x1 = ( x0 + 1U ) & 255U;
    const UInt32 y1 = ( y0 + 1U ) & 255U;
    const UInt32 z1 = ( z0 + 1U ) & 255U;
    const T one = T( 1 );

    const UInt32 a0 = p[ x0 ];
    const UInt32 a1 = p[ x1 ];
    const UInt32 a00 = p[ a0 + y0 ], a01 = p[ a0 + y1 ], a10 = p[ a1 + y0 ], a11 = p[ a1 + y1 ];

    const T n000 = Dot3( p[ a00 + z0 ], xf, yf, zf );
    const T n100 = Dot3( p[ a10 + z0 ], xf - one, yf, zf );
    const T n010 = Dot3( p[ a01 + z0 ], xf, yf - one, zf );
    const T n110 = Dot3( p[ a11 + z0 ], xf - one, yf - one, zf );
    const T n001 = Dot3( p[ a00 + z1 ], xf, yf, zf - one );
    const T n101 = Dot3( p[ a10 + z1 ], xf - one, yf, zf - one );
    const T n011 = Dot3( p[ a01 + z1 ], xf, yf - one, zf - one );
    const T n111 = Dot3( p[ a11 + z1 ], xf - one, yf - one, zf - one );
    const T u = Fade( xf );
    const T v = Fade( yf );
    const T w = Fade( zf );
    const T lower = Lerp( Lerp( n000, n100, u ), Lerp( n010, n110, u ), v );
    const T upper = Lerp( Lerp( n001, n101, u ), Lerp( n011, n111, u ), v );
    return Clamp1( (T) g_scale3D * Lerp( lower, upper, w ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
T
Perlin4D( const UInt8* p, T x, T y, T z, T w )
{
    const Int32 xi = FloorToLattice( x );
    const Int32 yi = FloorToLattice( y );
    const Int32 zi = FloorToLattice( z );
    const Int32 wi = FloorToLattice( w );
    const T xf = x - (T) xi;
    const T yf = y - (T) yi;
    const T zf = z - (T) zi;
    const T wf = w - (T) wi;
    const UInt32 xs[ 2 ] = { (UInt32) xi & 255U, ( (UInt32) xi + 1U ) & 255U };
    const UInt32 ys[ 2 ] = { (UInt32) yi & 255U, ( (UInt32) yi + 1U ) & 255U };
    const UInt32 zs[ 2 ] = { (UInt32) zi & 255U, ( (UInt32) zi + 1U ) & 255U };
    const UInt32 ws[ 2 ] = { (UInt32) wi & 255U, ( (UInt32) wi + 1U ) & 255U };
    const T one = T( 1 );

    T corners[ 16 ];
    for ( UInt32 i=0; i<16; ++i )
    {
        const UInt32 ix = i & 1U, iy = ( i >> 1 ) & 1U, iz = ( i >> 2 ) & 1U, iw = ( i >> 3 ) & 1U;
        const UInt32 hash = p[ p[ p[ p[ xs[ ix ] ] + ys[ iy ] ] + zs[ iz ] ] + ws[ iw ] ];
        corners[ i ] = Dot4( hash, ix ? xf - one : xf, iy ? yf - one : yf, iz ? zf - one : zf, iw ? wf - one : wf );
    }
    const T u = Fade( xf );
    const T v = Fade( yf );
    const T s = Fade( zf );
    const T t = Fade( wf );
    T reduced[ 8 ];
    for ( UInt32 i=0; i<8; ++i )
        reduced[ i ] = Lerp( corners[ 2 * i ], corners[ 2 * i + 1 ], u );
    T reduced2[ 4 ];
    for ( UInt32 i=0; i<4; ++i )
        reduced2[ i ] = Lerp( reduced[ 2 * i ], reduced[ 2 * i + 1 ], v );
    const T lower = Lerp( reduced2[ 0 ], reduced2[ 1 ], s );
    const T upper = Lerp( reduced2[ 2 ], reduced2[ 3 ], s );
    return Clamp1( (T) g_scale4D * Lerp( lower, upper, t ) );
}

}

/*-------------------------------------------------------------------------*/

CPerlinNoise::CPerlinNoise( UInt32 seed ) GUCEF_MATH_REALTIME_CONTEXT
    : m_seed( seed )
{
    Seed( seed );
}

/*-------------------------------------------------------------------------*/

void
CPerlinNoise::Seed( UInt32 seed ) GUCEF_MATH_REALTIME_CONTEXT
{
    m_seed = seed;
    for ( UInt32 i=0; i<(UInt32) TABLE_SIZE; ++i )
        m_permutation[ i ] = (UInt8) i;
    CSplitMixRandomStream stream( CSplitMix64( (UInt64) seed ) );
    stream.Shuffle( m_permutation, m_permutation + TABLE_SIZE );
    for ( UInt32 i=0; i<(UInt32) TABLE_SIZE; ++i )
        m_permutation[ TABLE_SIZE + i ] = m_permutation[ i ];
}

/*-------------------------------------------------------------------------*/

Float32 CPerlinNoise::Noise1D( Float32 x ) const GUCEF_MATH_REALTIME_CONTEXT { return Perlin1D< Float32 >( m_permutation, x ); }
Float64 CPerlinNoise::Noise1D( Float64 x ) const GUCEF_MATH_REALTIME_CONTEXT { return Perlin1D< Float64 >( m_permutation, x ); }
Float32 CPerlinNoise::Noise2D( Float32 x, Float32 y ) const GUCEF_MATH_REALTIME_CONTEXT { return Perlin2D< Float32 >( m_permutation, x, y ); }
Float64 CPerlinNoise::Noise2D( Float64 x, Float64 y ) const GUCEF_MATH_REALTIME_CONTEXT { return Perlin2D< Float64 >( m_permutation, x, y ); }
Float32 CPerlinNoise::Noise3D( Float32 x, Float32 y, Float32 z ) const GUCEF_MATH_REALTIME_CONTEXT { return Perlin3D< Float32 >( m_permutation, x, y, z ); }
Float64 CPerlinNoise::Noise3D( Float64 x, Float64 y, Float64 z ) const GUCEF_MATH_REALTIME_CONTEXT { return Perlin3D< Float64 >( m_permutation, x, y, z ); }
Float32 CPerlinNoise::Noise4D( Float32 x, Float32 y, Float32 z, Float32 w ) const GUCEF_MATH_REALTIME_CONTEXT { return Perlin4D< Float32 >( m_permutation, x, y, z, w ); }
Float64 CPerlinNoise::Noise4D( Float64 x, Float64 y, Float64 z, Float64 w ) const GUCEF_MATH_REALTIME_CONTEXT { return Perlin4D< Float64 >( m_permutation, x, y, z, w ); }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
