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

#include "gucefMATH_COpenSimplex2Noise.h"

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

/** Factors that bring the largest value to about 1 ( the maximum over the plane or space of the kernel weights times the best gradient of each lattice point: 0.010080 for 2D and 0.110546 for 3D ) */
const Float64 g_scale2D = 99.2;
const Float64 g_scale3D = 9.045;

template < typename T >
T
Simplex2D( UInt32 seed, T x, T y )
{
    const T skew = T( 0.36602540378443864676 );         // ( sqrt( 3 ) - 1 ) / 2
    const T unskew = T( 0.21132486540518711775 );       // ( 3 - sqrt( 3 ) ) / 6
    const T radiusSquared = T( 0.5 );

    const T s = ( x + y ) * skew;
    const Int32 i = FloorToLattice( x + s );
    const Int32 j = FloorToLattice( y + s );
    const T t = (T) ( i + j ) * unskew;
    const T x0 = x - ( (T) i - t );
    const T y0 = y - ( (T) j - t );

    // the middle vertex of the simplex depends on which triangle of the skewed cell the point is in
    const Int32 i1 = x0 > y0 ? 1 : 0;
    const Int32 j1 = x0 > y0 ? 0 : 1;
    const T x1 = x0 - (T) i1 + unskew;
    const T y1 = y0 - (T) j1 + unskew;
    const T x2 = x0 - T( 1 ) + T( 2 ) * unskew;
    const T y2 = y0 - T( 1 ) + T( 2 ) * unskew;

    T result = T( 0 );
    const T dx[ 3 ] = { x0, x1, x2 };
    const T dy[ 3 ] = { y0, y1, y2 };
    const Int32 vi[ 3 ] = { i, i + i1, i + 1 };
    const Int32 vj[ 3 ] = { j, j + j1, j + 1 };
    for ( UInt32 k=0; k<3; ++k )
    {
        const T a = radiusSquared - dx[ k ] * dx[ k ] - dy[ k ] * dy[ k ];
        if ( a > T( 0 ) )
        {
            const Float64* g = GetGradients2D( Hash2( seed, vi[ k ], vj[ k ] ) );
            const T a2 = a * a;
            result += a2 * a2 * ( T( g[ 0 ] ) * dx[ k ] + T( g[ 1 ] ) * dy[ k ] );
        }
    }
    return Clamp1( (T) g_scale2D * result );
}

/** The contribution of the 8 corners of the cell of one cubic lattice of the body centred lattice */
template < typename T >
T
SimplexLattice3D( UInt32 seed, T x, T y, T z )
{
    const Int32 xi = FloorToLattice( x );
    const Int32 yi = FloorToLattice( y );
    const Int32 zi = FloorToLattice( z );
    const T xf = x - (T) xi;
    const T yf = y - (T) yi;
    const T zf = z - (T) zi;
    T result = T( 0 );
    for ( Int32 c=0; c<8; ++c )
    {
        const Int32 ox = c & 1, oy = ( c >> 1 ) & 1, oz = ( c >> 2 ) & 1;
        const T dx = xf - (T) ox;
        const T dy = yf - (T) oy;
        const T dz = zf - (T) oz;
        const T a = T( 0.75 ) - dx * dx - dy * dy - dz * dz;
        if ( a > T( 0 ) )
        {
            const Int8* g = GetGradients3D( Hash3( seed, xi + ox, yi + oy, zi + oz ) );
            const T a2 = a * a;
            result += a2 * a2 * ( T( g[ 0 ] ) * dx + T( g[ 1 ] ) * dy + T( g[ 2 ] ) * dz );
        }
    }
    return result;
}

template < typename T >
T
Simplex3D( UInt32 seedA, UInt32 seedB, T x, T y, T z )
{
    // reflection about the diagonal ( x + y + z ) / 3: an orthogonal map, so distances are kept
    const T r = T( 2.0 / 3.0 ) * ( x + y + z );
    const T rx = r - x;
    const T ry = r - y;
    const T rz = r - z;
    const T half = T( 0.5 );
    const T result = SimplexLattice3D( seedA, rx, ry, rz ) + SimplexLattice3D( seedB, rx - half, ry - half, rz - half );
    return Clamp1( (T) g_scale3D * result );
}

}

/*-------------------------------------------------------------------------*/

COpenSimplex2Noise::COpenSimplex2Noise( UInt32 seed ) GUCEF_MATH_REALTIME_CONTEXT
    : m_seed( seed ) ,
      m_preparedSeed( 0 ) ,
      m_preparedSeedB( 0 )
{
    Seed( seed );
}

void
COpenSimplex2Noise::Seed( UInt32 seed ) GUCEF_MATH_REALTIME_CONTEXT
{
    m_seed = seed;
    m_preparedSeed = NoiseKernels::PrepareSeed( seed );
    m_preparedSeedB = NoiseKernels::Mix( m_preparedSeed ^ 0xA511E9B3U );
}

Float32 COpenSimplex2Noise::Noise2D( Float32 x, Float32 y ) const GUCEF_MATH_REALTIME_CONTEXT { return Simplex2D< Float32 >( m_preparedSeed, x, y ); }
Float64 COpenSimplex2Noise::Noise2D( Float64 x, Float64 y ) const GUCEF_MATH_REALTIME_CONTEXT { return Simplex2D< Float64 >( m_preparedSeed, x, y ); }
Float32 COpenSimplex2Noise::Noise3D( Float32 x, Float32 y, Float32 z ) const GUCEF_MATH_REALTIME_CONTEXT { return Simplex3D< Float32 >( m_preparedSeed, m_preparedSeedB, x, y, z ); }
Float64 COpenSimplex2Noise::Noise3D( Float64 x, Float64 y, Float64 z ) const GUCEF_MATH_REALTIME_CONTEXT { return Simplex3D< Float64 >( m_preparedSeed, m_preparedSeedB, x, y, z ); }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
