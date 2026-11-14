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

#include <math.h>

#include "gucefMATH_FftKernels.h"

#include "gucefMATH_CRealFft.h"

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

template< typename T >
void
ForwardTransform( const T* input, T* spectrum, UInt32 size, const T* twiddles, T* work ) GUCEF_MATH_REALTIME_CONTEXT
{
    const UInt32 half = size / 2;

    // the even samples as real parts and the odd ones as imaginary parts of half as many complex numbers
    FftSplitRadix( input, 1, work, half, twiddles, size );

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
InverseTransform( const T* spectrum, T* output, UInt32 size, const T* twiddles, T* workIn, T* workOut ) GUCEF_MATH_REALTIME_CONTEXT
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

    FftSplitRadix( workIn, 1, workOut, half, twiddles, size );

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

    if ( !FftIsValidSize( size, (UInt32) MIN_SIZE, (UInt32) MAX_SIZE ) )
        return false;

    m_size = 0;
    m_twiddles32.clear(); m_workA32.clear(); m_workB32.clear();
    m_twiddles64.clear(); m_workA64.clear(); m_workB64.clear();

    if ( singlePrecision )
    {
        FftFillTwiddles( m_twiddles32, size );
        m_workA32.assign( size, 0.0f );
        m_workB32.assign( size, 0.0f );
    }
    if ( doublePrecision )
    {
        FftFillTwiddles( m_twiddles64, size );
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

}; /* namespace MATH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
