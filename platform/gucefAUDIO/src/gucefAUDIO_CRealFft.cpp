/*
 *  gucefAUDIO: GUCEF module providing audio playback, mixing and decoding
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

#include <math.h>

#include "gucefAUDIO_CRealFft.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const Float64 Pi = 3.14159265358979323846;

/**
 *  Complex transform of 'count' points (a power of two) with the split-radix decimation in time algorithm.
 *  'input' holds interleaved complex numbers, 'stride' apart in complex units; the result is written to 'output'
 *  (not the same memory as the input). 'twiddles' is the table of the real transform of size 'fullSize':
 *  entry j is exp( -2 pi i j / fullSize ), so a factor exp( -2 pi i t / count ) is entry t * fullSize / count.
 *
 *  A transform of n points is made of one of n/2 points (even input) and two of n/4 points (inputs 4m+1 and 4m+3),
 *  which are written to the first half, third quarter and last quarter of the output, then butterflies combine them
 *  in place: the four values of a butterfly are at k, k + n/4, k + n/2 and k + 3n/4.
 */
template< typename T >
void
SplitRadix( const T* input, UInt32 stride, T* output, UInt32 count, const T* twiddles, UInt32 fullSize )
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
    SplitRadix( input, stride * 2, output, half, twiddles, fullSize );
    SplitRadix( input + 2 * (size_t) stride, stride * 4, output + 2 * (size_t) half, quarter, twiddles, fullSize );
    SplitRadix( input + 6 * (size_t) stride, stride * 4, output + 2 * (size_t) ( half + quarter ), quarter, twiddles, fullSize );

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
            const T* w3 = twiddles + 2 * (size_t) ( 3 * k * twiddleStep );
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

/** Fills the table with exp( -2 pi i j / size ) for j below size */
template< typename T >
void
FillTwiddles( GUCEF::vector< T >& table, UInt32 size )
{
    table.assign( 2 * (size_t) size, (T) 0 );
    for ( UInt32 j=0; j<size; ++j )
    {
        const Float64 angle = -2.0 * Pi * (Float64) j / (Float64) size;
        table[ 2 * (size_t) j ] = (T) cos( angle );
        table[ 2 * (size_t) j + 1 ] = (T) sin( angle );
    }
}

/*-------------------------------------------------------------------------*/

template< typename T >
void
ForwardTransform( const T* input, T* spectrum, UInt32 size, const T* twiddles, T* work )
{
    const UInt32 half = size / 2;

    // the even samples as real parts and the odd ones as imaginary parts of half as many complex numbers
    SplitRadix( input, 1, work, half, twiddles, size );

    // separate the spectra of the even and the odd samples and combine them: X[k] = E[k] + exp( -2 pi i k / N ) * O[k]
    for ( UInt32 k=0; k<=half; ++k )
    {
        const T* a = work + 2 * (size_t) ( k < half ? k : 0 );
        const T* b = work + 2 * (size_t) ( k > 0 ? half - k : 0 );
        const T zr = a[ 0 ];
        const T zi = a[ 1 ];
        const T cr = b[ 0 ];        // conj( Z[half-k] ) is ( b[0], -b[1] )
        const T ci = -b[ 1 ];
        const T er = (T) 0.5 * ( zr + cr );
        const T ei = (T) 0.5 * ( zi + ci );
        const T dr = zr - cr;
        const T di = zi - ci;
        const T orr = (T) 0.5 * di;             // O = ( Z - conj( Z' ) ) / 2i
        const T oi = (T) -0.5 * dr;
        const T* w = twiddles + 2 * (size_t) k;
        T* out = spectrum + 2 * (size_t) k;
        const T xr = er + ( orr * w[ 0 ] - oi * w[ 1 ] );
        const T xi = ei + ( orr * w[ 1 ] + oi * w[ 0 ] );
        out[ 0 ] = xr;
        out[ 1 ] = ( 0 == k || half == k ) ? (T) 0 : xi;
    }
}

/*-------------------------------------------------------------------------*/

template< typename T >
void
InverseTransform( const T* spectrum, T* output, UInt32 size, const T* twiddles, T* workIn, T* workOut )
{
    const UInt32 half = size / 2;

    // E[k] = ( X[k] + conj( X[half-k] ) ) / 2, O[k] = ( X[k] - conj( X[half-k] ) ) / 2 * exp( +2 pi i k / N ), Z[k] = E[k] + i O[k]
    // The complex inverse transform is done as the conjugate of the forward transform of the conjugate, so the conjugate of Z goes in.
    for ( UInt32 k=0; k<half; ++k )
    {
        const T* a = spectrum + 2 * (size_t) k;
        const T* b = spectrum + 2 * (size_t) ( half - k );
        const T xr = a[ 0 ];
        const T xi = ( 0 == k ) ? (T) 0 : a[ 1 ];
        const T yr = b[ 0 ];
        const T yi = ( 0 == k ) ? (T) 0 : -b[ 1 ];      // conj( X[half-k] ); the imaginary part of bin half is ignored (k = 0)
        const T er = (T) 0.5 * ( xr + yr );
        const T ei = (T) 0.5 * ( xi + yi );
        const T dr = (T) 0.5 * ( xr - yr );
        const T di = (T) 0.5 * ( xi - yi );
        const T* w = twiddles + 2 * (size_t) k;         // exp( -i angle ): its conjugate is the factor needed
        const T orr = dr * w[ 0 ] + di * w[ 1 ];
        const T oi = di * w[ 0 ] - dr * w[ 1 ];
        // Z = E + i * O
        T* in = workIn + 2 * (size_t) k;
        in[ 0 ] = er - oi;
        in[ 1 ] = -( ei + orr );
    }

    SplitRadix( workIn, 1, workOut, half, twiddles, size );

    const T scale = (T) 1 / (T) half;
    for ( UInt32 m=0; m<half; ++m )
    {
        output[ 2 * (size_t) m ] = workOut[ 2 * (size_t) m ] * scale;
        output[ 2 * (size_t) m + 1 ] = -workOut[ 2 * (size_t) m + 1 ] * scale;
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRealFft::CRealFft( void )
    : m_size( 0 )
    , m_twiddles32()
    , m_workA32()
    , m_workB32()
    , m_twiddles64()
    , m_workA64()
    , m_workB64()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRealFft::~CRealFft()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CRealFft::Prepare( UInt32 size, bool singlePrecision, bool doublePrecision )
{GUCEF_TRACE;

    if ( size < MIN_SIZE || size > MAX_SIZE || 0 != ( size & ( size - 1 ) ) )
        return false;

    m_size = 0;
    m_twiddles32.clear(); m_workA32.clear(); m_workB32.clear();
    m_twiddles64.clear(); m_workA64.clear(); m_workB64.clear();

    if ( singlePrecision )
    {
        FillTwiddles( m_twiddles32, size );
        m_workA32.assign( size, 0.0f );
        m_workB32.assign( size, 0.0f );
    }
    if ( doublePrecision )
    {
        FillTwiddles( m_twiddles64, size );
        m_workA64.assign( size, 0.0 );
        m_workB64.assign( size, 0.0 );
    }
    m_size = size;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CRealFft::GetSize( void ) const
{
    // no trace macro: this runs where the audio is mixed

    return m_size;
}

/*-------------------------------------------------------------------------*/

UInt32
CRealFft::GetBinCount( void ) const
{
    // no trace macro: this runs where the audio is mixed

    return 0 == m_size ? 0 : m_size / 2 + 1;
}

/*-------------------------------------------------------------------------*/

UInt32
CRealFft::GetSpectrumLength( void ) const
{
    // no trace macro: this runs where the audio is mixed

    return 0 == m_size ? 0 : m_size + 2;
}

/*-------------------------------------------------------------------------*/

bool
CRealFft::Forward( const Float32* input, Float32* spectrum )
{
    // no trace macro: this runs where the audio is mixed

    if ( m_twiddles32.empty() || GUCEF_NULL == input || GUCEF_NULL == spectrum )
        return false;
    ForwardTransform( input, spectrum, m_size, &m_twiddles32[ 0 ], &m_workA32[ 0 ] );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRealFft::Forward( const Float64* input, Float64* spectrum )
{
    // no trace macro: this runs where the audio is mixed

    if ( m_twiddles64.empty() || GUCEF_NULL == input || GUCEF_NULL == spectrum )
        return false;
    ForwardTransform( input, spectrum, m_size, &m_twiddles64[ 0 ], &m_workA64[ 0 ] );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRealFft::Inverse( const Float32* spectrum, Float32* output )
{
    // no trace macro: this runs where the audio is mixed

    if ( m_twiddles32.empty() || GUCEF_NULL == spectrum || GUCEF_NULL == output )
        return false;
    InverseTransform( spectrum, output, m_size, &m_twiddles32[ 0 ], &m_workA32[ 0 ], &m_workB32[ 0 ] );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRealFft::Inverse( const Float64* spectrum, Float64* output )
{
    // no trace macro: this runs where the audio is mixed

    if ( m_twiddles64.empty() || GUCEF_NULL == spectrum || GUCEF_NULL == output )
        return false;
    InverseTransform( spectrum, output, m_size, &m_twiddles64[ 0 ], &m_workA64[ 0 ], &m_workB64[ 0 ] );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
