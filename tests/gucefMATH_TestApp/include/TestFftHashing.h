/*
 *  gucefMATH_TestApp: GUCEF test application for the Math module
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

#ifndef GUCEF_MATH_TESTAPP_TESTFFTHASHING_H
#define GUCEF_MATH_TESTAPP_TESTFFTHASHING_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_MATH_MACROS_H
#include "gucefMATH_macros.h"
#define GUCEF_MATH_MACROS_H
#endif /* GUCEF_MATH_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  Hashing of the outputs of a real FFT object, for the check that the transform stays bit identical. The functions
 *  are templates over the FFT class, so that the same code hashes MATH::CRealFft and a class with the same interface.
 *  For every size from 64 to 4096, in Float32 and Float64, with a seeded input signal, four hashes are produced:
 *  Forward out of place, Inverse out of place (of that spectrum), Forward in place and Inverse in place. The hash
 *  is FNV-1a over the bytes of the output values, 64 bit.
 */
namespace FftHashing {

const GUCEF::UInt32 SizeCount = 7;
const GUCEF::UInt32 Sizes[ SizeCount ] = { 64, 128, 256, 512, 1024, 2048, 4096 };

/** Index of a hash in the pinned table: [ size index ][ precision 0 = Float32, 1 = Float64 ][ 0 forward, 1 inverse, 2 forward in place, 3 inverse in place ] */
inline GUCEF::UInt32 HashIndex( GUCEF::UInt32 sizeIndex, GUCEF::UInt32 precision, GUCEF::UInt32 mode )
{
    return ( sizeIndex * 2 + precision ) * 4 + mode;
}

inline GUCEF::UInt64
Fnv1a( const void* data, size_t byteCount, GUCEF::UInt64 hash )
{
    const unsigned char* bytes = static_cast< const unsigned char* >( data );
    for ( size_t i=0; i<byteCount; ++i )
    {
        hash ^= (GUCEF::UInt64) bytes[ i ];
        hash *= (GUCEF::UInt64) 0x100000001b3ULL;
    }
    return hash;
}

/** xorshift64* generator, seeded signal values in [ -1, 1 ) */
inline GUCEF::Float64
NextSignalValue( GUCEF::UInt64& state )
{
    state ^= state >> 12;
    state ^= state << 25;
    state ^= state >> 27;
    const GUCEF::UInt64 r = state * (GUCEF::UInt64) 0x2545F4914F6CDD1DULL;
    return ( (GUCEF::Float64) ( r >> 11 ) / 9007199254740992.0 ) * 2.0 - 1.0;
}

template< typename TFft, typename T >
void
HashPrecision( TFft& fft, GUCEF::UInt32 size, GUCEF::UInt32 seed, GUCEF::UInt64* hashes )
{
    GUCEF::UInt64 state = 0x9E3779B97F4A7C15ULL ^ ( (GUCEF::UInt64) seed * 0xBF58476D1CE4E5B9ULL );
    if ( 0 == state )
        state = 1;
    const size_t spectrumLength = (size_t) size + 2;
    T* input = new T[ size ];
    T* spectrum = new T[ spectrumLength ];
    T* output = new T[ size ];
    T* inPlace = new T[ spectrumLength ];
    for ( GUCEF::UInt32 n=0; n<size; ++n )
        input[ n ] = (T) NextSignalValue( state );
    const GUCEF::UInt64 offset = (GUCEF::UInt64) 0xcbf29ce484222325ULL;

    memset( spectrum, 0x55, spectrumLength * sizeof( T ) );
    fft.Forward( input, spectrum );
    hashes[ 0 ] = Fnv1a( spectrum, spectrumLength * sizeof( T ), offset );

    fft.Inverse( spectrum, output );
    hashes[ 1 ] = Fnv1a( output, size * sizeof( T ), offset );

    memset( inPlace, 0x55, spectrumLength * sizeof( T ) );
    memcpy( inPlace, input, size * sizeof( T ) );
    fft.Forward( inPlace, inPlace );
    hashes[ 2 ] = Fnv1a( inPlace, spectrumLength * sizeof( T ), offset );

    fft.Inverse( inPlace, inPlace );
    hashes[ 3 ] = Fnv1a( inPlace, size * sizeof( T ), offset );

    delete[] input;
    delete[] spectrum;
    delete[] output;
    delete[] inPlace;
}

/** Fills 'hashes' ( SizeCount * 2 * 4 values, indexed with HashIndex ) with the hashes of the given FFT class, which needs Prepare( size, true, true ), Forward and Inverse for both precisions */
template< typename TFft >
void
ComputeAllHashes( GUCEF::UInt64* hashes )
{
    for ( GUCEF::UInt32 s=0; s<SizeCount; ++s )
    {
        TFft fft;
        fft.Prepare( Sizes[ s ], true, true );
        HashPrecision< TFft, GUCEF::Float32 >( fft, Sizes[ s ], 1000 + Sizes[ s ], hashes + HashIndex( s, 0, 0 ) );
        HashPrecision< TFft, GUCEF::Float64 >( fft, Sizes[ s ], 2000 + Sizes[ s ], hashes + HashIndex( s, 1, 0 ) );
    }
}

}; /* namespace FftHashing */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_TESTAPP_TESTFFTHASHING_H ? */
