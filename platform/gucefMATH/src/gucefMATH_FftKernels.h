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

#ifndef GUCEF_MATH_FFTKERNELS_H
#define GUCEF_MATH_FFTKERNELS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_MACROS_H
#include "gucefMATH_macros.h"
#define GUCEF_MATH_MACROS_H
#endif /* GUCEF_MATH_MACROS_H ? */

#ifndef GUCEF_MATH_DETERMINISTICTRIGONOMETRY_H
#include "gucefMATH_DeterministicTrigonometry.h"
#define GUCEF_MATH_DETERMINISTICTRIGONOMETRY_H
#endif /* GUCEF_MATH_DETERMINISTICTRIGONOMETRY_H ? */

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

/*
 *  Private kernels shared by CRealFft and CComplexFft: not part of the public surface of the module.
 *  The arithmetic and its order of evaluation are those of the FFT of gucefAUDIO that moved here, so that the
 *  results are bit identical with it. No tracing, no allocation, no locks: realtime safe.
 */

/*-------------------------------------------------------------------------*/

/**
 *  Complex transform of 'count' points (a power of two) with the split-radix decimation in time algorithm.
 *  'input' holds interleaved complex numbers, 'stride' apart in complex units; the result is written to 'output'
 *  (not the same memory as the input). 'twiddles' is a table of 'fullSize' entries:
 *  entry j is exp( -2 pi i j / fullSize ), so a factor exp( -2 pi i t / count ) is entry t * fullSize / count.
 *
 *  A transform of n points is made of one of n/2 points (even input) and two of n/4 points (inputs 4m+1 and 4m+3),
 *  which are written to the first half, third quarter and last quarter of the output, then butterflies combine them
 *  in place: the four values of a butterfly are at k, k + n/4, k + n/2 and k + 3n/4.
 */
template< typename T >
void
FftSplitRadix( const T* input, UInt32 stride, T* output, UInt32 count, const T* twiddles, UInt32 fullSize ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( 1 == count )
    {
        output[ 0 ] = input[ 0 ];
        output[ 1 ] = input[ 1 ];
        return;
    }
    if ( 2 == count )
    {
        const T* second = input + 2 * (size_t) stride;
        output[ 0 ] = input[ 0 ] + second[ 0 ];
        output[ 1 ] = input[ 1 ] + second[ 1 ];
        output[ 2 ] = input[ 0 ] - second[ 0 ];
        output[ 3 ] = input[ 1 ] - second[ 1 ];
        return;
    }
    if ( 4 == count )
    {
        // a four point transform directly: the leaves are most of the calls of the recursion
        const T* x0 = input;
        const T* x1 = input + 2 * (size_t) stride;
        const T* x2 = input + 4 * (size_t) stride;
        const T* x3 = input + 6 * (size_t) stride;
        const T ar = x0[ 0 ] + x2[ 0 ], ai = x0[ 1 ] + x2[ 1 ];
        const T br = x0[ 0 ] - x2[ 0 ], bi = x0[ 1 ] - x2[ 1 ];
        const T cr = x1[ 0 ] + x3[ 0 ], ci = x1[ 1 ] + x3[ 1 ];
        const T dr = x1[ 0 ] - x3[ 0 ], di = x1[ 1 ] - x3[ 1 ];
        output[ 0 ] = ar + cr;          // X0 = a + c
        output[ 1 ] = ai + ci;
        output[ 2 ] = br + di;          // X1 = b - i d
        output[ 3 ] = bi - dr;
        output[ 4 ] = ar - cr;          // X2 = a - c
        output[ 5 ] = ai - ci;
        output[ 6 ] = br - di;          // X3 = b + i d
        output[ 7 ] = bi + dr;
        return;
    }

    const UInt32 half = count / 2;
    const UInt32 quarter = count / 4;
    FftSplitRadix( input, stride * 2, output, half, twiddles, fullSize );
    FftSplitRadix( input + 2 * (size_t) stride, stride * 4, output + 2 * (size_t) half, quarter, twiddles, fullSize );
    FftSplitRadix( input + 6 * (size_t) stride, stride * 4, output + 2 * (size_t) ( half + quarter ), quarter, twiddles, fullSize );

    const UInt32 twiddleStep = fullSize / count;
    for ( UInt32 k=0; k<quarter; ++k )
    {
        T* u0 = output + 2 * (size_t) k;
        T* u1 = output + 2 * (size_t) ( k + quarter );
        T* z1 = output + 2 * (size_t) ( k + half );
        T* z3 = output + 2 * (size_t) ( k + half + quarter );

        T t1r = z1[ 0 ];
        T t1i = z1[ 1 ];
        T t3r = z3[ 0 ];
        T t3i = z3[ 1 ];
        if ( k > 0 )
        {
            const T* w1 = twiddles + 2 * (size_t) ( k * twiddleStep );
            const T* w3 = twiddles + 2 * (size_t) ( 3 * (size_t) k * twiddleStep );
            t1r = z1[ 0 ] * w1[ 0 ] - z1[ 1 ] * w1[ 1 ];
            t1i = z1[ 0 ] * w1[ 1 ] + z1[ 1 ] * w1[ 0 ];
            t3r = z3[ 0 ] * w3[ 0 ] - z3[ 1 ] * w3[ 1 ];
            t3i = z3[ 0 ] * w3[ 1 ] + z3[ 1 ] * w3[ 0 ];
        }
        const T sumR = t1r + t3r;
        const T sumI = t1i + t3i;
        const T diffR = t1r - t3r;
        const T diffI = t1i - t3i;

        const T a0r = u0[ 0 ];
        const T a0i = u0[ 1 ];
        const T a1r = u1[ 0 ];
        const T a1i = u1[ 1 ];

        u0[ 0 ] = a0r + sumR;
        u0[ 1 ] = a0i + sumI;
        z1[ 0 ] = a0r - sumR;
        z1[ 1 ] = a0i - sumI;
        u1[ 0 ] = a1r + diffI;      // a1 - i * diff
        u1[ 1 ] = a1i - diffR;
        z3[ 0 ] = a1r - diffI;      // a1 + i * diff
        z3[ 1 ] = a1i + diffR;
    }
}

/*-------------------------------------------------------------------------*/

/**
 *  Fills the table with exp( -2 pi i j / size ) for j below size, real and imaginary part alternating.
 *  The values come from the deterministic sine and cosine of gucefMATH ( the C library gives other last bits on other
 *  platforms ), and only the first octant is computed: the rest follows by the exact symmetries of the circle (swapping
 *  sine and cosine, flipping signs), so the table is exactly symmetric and identical on every platform.
 */
template< typename T >
void
FftFillTwiddles( GUCEF::vector< T >& table, UInt32 size )
{
    table.assign( 2 * (size_t) size, (T) 0 );

    // first octant, including its end: j from 0 to size / 8 ( only j = 0 for the sizes below 8 )
    const UInt32 octantEnd = size / 8;
    for ( UInt32 j=0; j<=octantEnd && j<size; ++j )
    {
        Float64 s = 0.0, c = 0.0;
        SinCosOfTurnFraction( (Int64) j, (Int64) size, s, c );
        table[ 2 * (size_t) j ] = (T) c;
        table[ 2 * (size_t) j + 1 ] = (T) ( 0.0 - s );
    }

    // the other octants: the angle is j / size of a turn, in octant o it is o / 8 turn plus 'within' / ( 8 size ),
    // measured back from the end of the octant for an odd one; the table holds ( cos, -sin ) of the first octant
    for ( UInt32 j=octantEnd+1; j<size; ++j )
    {
        const UInt64 eighths = 8 * (UInt64) j;
        const UInt32 octant = (UInt32) ( eighths / size );
        UInt64 within = eighths - (UInt64) octant * size;       // a multiple of 8 for sizes from 8 up, zero below
        if ( 0 != ( octant & 1 ) )
            within = size - within;
        const T* base = &table[ 0 ] + 2 * (size_t) ( within / 8 );
        const T c = base[ 0 ];
        const T s = (T) 0 - base[ 1 ];
        T sine, cosine;
        switch ( octant )
        {
            case 1: sine = c;           cosine = s;         break;
            case 2: sine = c;           cosine = (T) 0 - s; break;
            case 3: sine = s;           cosine = (T) 0 - c; break;
            case 4: sine = (T) 0 - s;   cosine = (T) 0 - c; break;
            case 5: sine = (T) 0 - c;   cosine = (T) 0 - s; break;
            case 6: sine = (T) 0 - c;   cosine = s;         break;
            default: sine = (T) 0 - s;  cosine = c;         break;
        }
        table[ 2 * (size_t) j ] = cosine;
        table[ 2 * (size_t) j + 1 ] = (T) 0 - sine;
    }
}

/*-------------------------------------------------------------------------*/

/** True for a power of two that is at least 'minimum' and at most 'maximum' */
inline bool
FftIsValidSize( UInt32 size, UInt32 minimum, UInt32 maximum )
{
    return size >= minimum && size <= maximum && 0 == ( size & ( size - 1 ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MATH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_FFTKERNELS_H ? */
