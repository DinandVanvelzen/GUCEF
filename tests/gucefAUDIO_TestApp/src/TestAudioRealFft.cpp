/*
 *  gucefAUDIO_TestApp: GUCEF test application for the AUDIO module
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
#include <string.h>

#ifndef GUCEFAUDIO_H
#include "gucefAUDIO.h"
#define GUCEFAUDIO_H
#endif /* GUCEFAUDIO_H ? */

#ifndef GUCEF_AUDIO_CREALFFT_H
#include "gucefAUDIO_CRealFft.h"
#define GUCEF_AUDIO_CREALFFT_H
#endif /* GUCEF_AUDIO_CREALFFT_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAudioEffectSupport.h"
#include "TestAudioRealFft.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace AudioEffectTest;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef GUCEF::vector< Float64 > TDoubles;

const UInt32 Sizes[] = { 64, 128, 256, 512, 1024, 2048, 4096 };
const UInt32 SizeCount = 7;

/** The calls marked as a real time context in a RealtimeSanitizer build */
bool
ForwardRealtime( AUDIO::CRealFft& fft, const Float32* input, Float32* spectrum ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    return fft.Forward( input, spectrum );
}

bool
InverseRealtime( AUDIO::CRealFft& fft, const Float32* spectrum, Float32* output ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    return fft.Inverse( spectrum, output );
}

bool
ForwardRealtime( AUDIO::CRealFft& fft, const Float64* input, Float64* spectrum ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    return fft.Forward( input, spectrum );
}

bool
InverseRealtime( AUDIO::CRealFft& fft, const Float64* spectrum, Float64* output ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    return fft.Inverse( spectrum, output );
}

/*-------------------------------------------------------------------------*/

/** The definition: X[k] = sum of x[n] exp( -2 pi i k n / N ) for the bins 0 to N/2, real and imaginary part alternating */
TDoubles
DirectDft( const TDoubles& x )
{
    const size_t size = x.size();
    TDoubles spectrum( size + 2 );
    for ( size_t k=0; k<=size/2; ++k )
    {
        Float64 re = 0.0;
        Float64 im = 0.0;
        for ( size_t n=0; n<size; ++n )
        {
            const Float64 angle = 2.0 * Pi * (Float64) ( ( k * n ) % size ) / (Float64) size;
            re += x[ n ] * cos( angle );
            im -= x[ n ] * sin( angle );
        }
        spectrum[ 2 * k ] = re;
        spectrum[ 2 * k + 1 ] = im;
    }
    return spectrum;
}

/** Signal with a DC part, from the generator */
TDoubles
MakeSignal( UInt32 size, UInt32 seed, Float64 dc = 0.3 )
{
    CXorShift random( seed );
    TDoubles x( size );
    for ( UInt32 n=0; n<size; ++n )
        x[ n ] = dc + (Float64) random.NextFloat();
    return x;
}

template< typename T >
GUCEF::vector< T >
Convert( const TDoubles& values, size_t length = 0 )
{
    GUCEF::vector< T > result( length > values.size() ? length : values.size(), (T) 0 );
    for ( size_t i=0; i<values.size(); ++i )
        result[ i ] = (T) values[ i ];
    return result;
}

template< typename T >
Float64
LargestMagnitude( const GUCEF::vector< T >& values )
{
    Float64 largest = 0.0;
    for ( size_t i=0; i<values.size(); ++i )
        if ( fabs( (Float64) values[ i ] ) > largest )
            largest = fabs( (Float64) values[ i ] );
    return largest;
}

/** Largest difference between a computed and a reference vector of the same length, relative to the reference's largest value */
template< typename T >
Float64
RelativeError( const GUCEF::vector< T >& computed, const TDoubles& reference, size_t length = 0 )
{
    if ( 0 == length )
        length = reference.size();
    Float64 largestError = 0.0;
    Float64 largest = 0.0;
    for ( size_t i=0; i<length; ++i )
    {
        const Float64 e = fabs( (Float64) computed[ i ] - reference[ i ] );
        if ( e > largestError )
            largestError = e;
        if ( fabs( reference[ i ] ) > largest )
            largest = fabs( reference[ i ] );
    }
    return largestError / largest;
}

/** Sum of the squares of the signal, and of the bins by Parseval's relation for the half spectrum: ( |X0|^2 + |X(N/2)|^2 + 2 sum of the others ) / N */
template< typename T >
Float64
SpectrumEnergy( const GUCEF::vector< T >& spectrum, UInt32 size )
{
    Float64 sum = 0.0;
    for ( UInt32 k=0; k<=size/2; ++k )
    {
        const Float64 power = (Float64) spectrum[ 2 * k ] * (Float64) spectrum[ 2 * k ] + (Float64) spectrum[ 2 * k + 1 ] * (Float64) spectrum[ 2 * k + 1 ];
        sum += ( 0 == k || size/2 == k ) ? power : 2.0 * power;
    }
    return sum / (Float64) size;
}

Float64
SignalEnergy( const TDoubles& x )
{
    Float64 sum = 0.0;
    for ( size_t n=0; n<x.size(); ++n )
        sum += x[ n ] * x[ n ];
    return sum;
}

Float64
BinMagnitude( const TDoubles& spectrum, UInt32 k )
{
    return sqrt( spectrum[ 2 * k ] * spectrum[ 2 * k ] + spectrum[ 2 * k + 1 ] * spectrum[ 2 * k + 1 ] );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformTransformTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "RealFft: Forward equals the direct DFT for sizes 64 to 4096 in Float64 (1e-12) and Float32 (5e-6 of the largest bin)" )
    try
    {
        Float64 worst32 = 0.0;
        for ( UInt32 s=0; s<SizeCount; ++s )
        {
            const UInt32 size = Sizes[ s ];
            AUDIO::CRealFft fft;
            ASSERT_TRUE( fft.Prepare( size ) );
            ASSERT_TRUE( size == fft.GetSize() && size/2 + 1 == fft.GetBinCount() && size + 2 == fft.GetSpectrumLength() );

            for ( UInt32 seed=1; seed<=2; ++seed )
            {
                const TDoubles x = MakeSignal( size, 1000 + size + seed );
                const TDoubles reference = DirectDft( x );

                GUCEF::vector< Float64 > spectrum64( size + 2, 7.0 );
                ASSERT_TRUE( ForwardRealtime( fft, &x[ 0 ], &spectrum64[ 0 ] ) );
                ASSERT_TRUE( RelativeError( spectrum64, reference ) < 1.0e-12 );
                ASSERT_TRUE( 0.0 == spectrum64[ 1 ] && 0.0 == spectrum64[ size + 1 ] );

                const GUCEF::vector< Float32 > x32 = Convert< Float32 >( x );
                GUCEF::vector< Float32 > spectrum32( size + 2, 7.0f );
                ASSERT_TRUE( ForwardRealtime( fft, &x32[ 0 ], &spectrum32[ 0 ] ) );
                // the reference of the Float32 path is the DFT of the rounded input
                TDoubles rounded( size );
                for ( UInt32 n=0; n<size; ++n )
                    rounded[ n ] = x32[ n ];
                const Float64 error = RelativeError( spectrum32, DirectDft( rounded ) );
                if ( error > worst32 )
                    worst32 = error;
                ASSERT_TRUE( error < 5.0e-6 );
                ASSERT_TRUE( 0.0f == spectrum32[ 1 ] && 0.0f == spectrum32[ size + 1 ] );
            }
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RealFft: worst Float32 error against the DFT of its input, relative to the largest bin: " << CORE::Int64ToString( (Int64) ( worst32 * 1.0e9 ) ) << " parts per billion" );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RealFft: Inverse of Forward gives the input back, in Float64 (1e-13) and Float32 (2e-6), and the inverse is scaled by 1/N" )
    try
    {
        for ( UInt32 s=0; s<SizeCount; ++s )
        {
            const UInt32 size = Sizes[ s ];
            AUDIO::CRealFft fft;
            ASSERT_TRUE( fft.Prepare( size ) );
            const TDoubles x = MakeSignal( size, 31 + size );

            GUCEF::vector< Float64 > spectrum( size + 2 );
            GUCEF::vector< Float64 > back( size );
            ASSERT_TRUE( ForwardRealtime( fft, &x[ 0 ], &spectrum[ 0 ] ) );
            ASSERT_TRUE( InverseRealtime( fft, &spectrum[ 0 ], &back[ 0 ] ) );
            ASSERT_TRUE( RelativeError( back, x ) < 1.0e-13 );

            const GUCEF::vector< Float32 > x32 = Convert< Float32 >( x );
            TDoubles rounded( size );
            for ( UInt32 n=0; n<size; ++n )
                rounded[ n ] = x32[ n ];
            GUCEF::vector< Float32 > spectrum32( size + 2 );
            GUCEF::vector< Float32 > back32( size );
            ASSERT_TRUE( ForwardRealtime( fft, &x32[ 0 ], &spectrum32[ 0 ] ) );
            ASSERT_TRUE( InverseRealtime( fft, &spectrum32[ 0 ], &back32[ 0 ] ) );
            ASSERT_TRUE( RelativeError( back32, rounded ) < 2.0e-6 );

            // the inverse of a single bin is a cosine of amplitude 2/N per unit of bin magnitude: bin k of magnitude N/2 at phase 0.7 is cos( 2 pi k n / N + 0.7 )
            const UInt32 bin = size / 8 + 1;
            GUCEF::vector< Float64 > one( size + 2, 0.0 );
            one[ 2 * bin ] = 0.5 * size * cos( 0.7 );
            one[ 2 * bin + 1 ] = 0.5 * size * sin( 0.7 );
            ASSERT_TRUE( InverseRealtime( fft, &one[ 0 ], &back[ 0 ] ) );
            TDoubles cosine( size );
            for ( UInt32 n=0; n<size; ++n )
                cosine[ n ] = cos( 2.0 * Pi * (Float64) ( ( bin * n ) % size ) / (Float64) size + 0.7 );
            ASSERT_TRUE( RelativeError( back, cosine ) < 1.0e-13 );

            // DC and Nyquist bins: imaginary parts are ignored
            GUCEF::vector< Float64 > edges( size + 2, 0.0 );
            edges[ 0 ] = 3.0 * size;
            edges[ 1 ] = 99.0;
            edges[ size ] = 2.0 * size;
            edges[ size + 1 ] = -99.0;
            ASSERT_TRUE( InverseRealtime( fft, &edges[ 0 ], &back[ 0 ] ) );
            for ( UInt32 n=0; n<size; ++n )
                ASSERT_TRUE( fabs( back[ n ] - ( 3.0 + ( 0 == n % 2 ? 2.0 : -2.0 ) ) ) < 1.0e-12 );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RealFft: Parseval holds (energy of the signal = energy of the spectrum / N) and the transform is linear" )
    try
    {
        for ( UInt32 s=0; s<SizeCount; ++s )
        {
            const UInt32 size = Sizes[ s ];
            AUDIO::CRealFft fft;
            ASSERT_TRUE( fft.Prepare( size ) );
            const TDoubles x = MakeSignal( size, 5 + size );
            const TDoubles y = MakeSignal( size, 9 + size, -0.1 );

            GUCEF::vector< Float64 > fx( size + 2 );
            GUCEF::vector< Float64 > fy( size + 2 );
            ASSERT_TRUE( fft.Forward( &x[ 0 ], &fx[ 0 ] ) );
            ASSERT_TRUE( fft.Forward( &y[ 0 ], &fy[ 0 ] ) );
            ASSERT_TRUE( fabs( SpectrumEnergy( fx, size ) / SignalEnergy( x ) - 1.0 ) < 1.0e-12 );

            const Float64 a = 1.7;
            const Float64 b = -0.45;
            TDoubles mix( size );
            for ( UInt32 n=0; n<size; ++n )
                mix[ n ] = a * x[ n ] + b * y[ n ];
            GUCEF::vector< Float64 > fmix( size + 2 );
            ASSERT_TRUE( fft.Forward( &mix[ 0 ], &fmix[ 0 ] ) );
            TDoubles expected( size + 2 );
            for ( UInt32 i=0; i<size+2; ++i )
                expected[ i ] = a * fx[ i ] + b * fy[ i ];
            ASSERT_TRUE( RelativeError( fmix, expected ) < 1.0e-13 );

            // Float32
            const GUCEF::vector< Float32 > x32 = Convert< Float32 >( x );
            const GUCEF::vector< Float32 > y32 = Convert< Float32 >( y );
            GUCEF::vector< Float32 > fx32( size + 2 );
            GUCEF::vector< Float32 > fy32( size + 2 );
            ASSERT_TRUE( fft.Forward( &x32[ 0 ], &fx32[ 0 ] ) );
            ASSERT_TRUE( fft.Forward( &y32[ 0 ], &fy32[ 0 ] ) );
            TDoubles rounded( size );
            for ( UInt32 n=0; n<size; ++n )
                rounded[ n ] = x32[ n ];
            ASSERT_TRUE( fabs( SpectrumEnergy( fx32, size ) / SignalEnergy( rounded ) - 1.0 ) < 1.0e-5 );

            GUCEF::vector< Float32 > mix32( size );
            for ( UInt32 n=0; n<size; ++n )
                mix32[ n ] = (Float32) a * x32[ n ] + (Float32) b * y32[ n ];
            GUCEF::vector< Float32 > fmix32( size + 2 );
            ASSERT_TRUE( fft.Forward( &mix32[ 0 ], &fmix32[ 0 ] ) );
            TDoubles expected32( size + 2 );
            for ( UInt32 i=0; i<size+2; ++i )
                expected32[ i ] = a * fx32[ i ] + b * fy32[ i ];
            ASSERT_TRUE( RelativeError( fmix32, expected32 ) < 5.0e-6 );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RealFft: Scaling convention: forward unscaled (DC of c is c*N, a sine of amplitude A on a bin is A*N/2, an impulse is flat 1), shifts rotate the phase" )
    try
    {
        for ( UInt32 s=0; s<SizeCount; ++s )
        {
            const UInt32 size = Sizes[ s ];
            AUDIO::CRealFft fft;
            ASSERT_TRUE( fft.Prepare( size ) );
            GUCEF::vector< Float64 > spectrum( size + 2 );

            TDoubles dc( size, 0.25 );
            ASSERT_TRUE( fft.Forward( &dc[ 0 ], &spectrum[ 0 ] ) );
            ASSERT_TRUE( fabs( spectrum[ 0 ] - 0.25 * size ) < 1.0e-10 );
            for ( UInt32 k=1; k<=size/2; ++k )
                ASSERT_TRUE( BinMagnitude( spectrum, k ) < 1.0e-10 );

            const UInt32 bin = 3 + size / 16;
            TDoubles sine( size );
            for ( UInt32 n=0; n<size; ++n )
                sine[ n ] = 0.8 * sin( 2.0 * Pi * (Float64) bin * (Float64) n / (Float64) size );
            ASSERT_TRUE( fft.Forward( &sine[ 0 ], &spectrum[ 0 ] ) );
            ASSERT_TRUE( fabs( BinMagnitude( spectrum, bin ) - 0.8 * size / 2.0 ) < 1.0e-9 );
            ASSERT_TRUE( fabs( spectrum[ 2 * bin ] ) < 1.0e-9 && spectrum[ 2 * bin + 1 ] < 0.0 );       // sin = -i * ( e^{ix} - e^{-ix} ) / 2: the bin is -i * A N / 2
            ASSERT_TRUE( BinMagnitude( spectrum, bin + 1 ) < 1.0e-9 && BinMagnitude( spectrum, bin - 1 ) < 1.0e-9 );

            TDoubles impulse( size, 0.0 );
            impulse[ 5 ] = 1.0;
            ASSERT_TRUE( fft.Forward( &impulse[ 0 ], &spectrum[ 0 ] ) );
            for ( UInt32 k=0; k<=size/2; ++k )
            {
                const Float64 angle = -2.0 * Pi * (Float64) k * 5.0 / (Float64) size;
                ASSERT_TRUE( fabs( spectrum[ 2 * k ] - cos( angle ) ) < 1.0e-12 && fabs( spectrum[ 2 * k + 1 ] - sin( angle ) ) < 1.0e-12 );
            }

            // Nyquist: alternating signs
            TDoubles nyquist( size );
            for ( UInt32 n=0; n<size; ++n )
                nyquist[ n ] = 0 == n % 2 ? 1.0 : -1.0;
            ASSERT_TRUE( fft.Forward( &nyquist[ 0 ], &spectrum[ 0 ] ) );
            ASSERT_TRUE( fabs( spectrum[ size ] - size ) < 1.0e-10 && 0.0 == spectrum[ size + 1 ] );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformUseTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "RealFft: In place and out of place give the same bits, forward and inverse, both precisions" )
    try
    {
        for ( UInt32 s=0; s<SizeCount; ++s )
        {
            const UInt32 size = Sizes[ s ];
            AUDIO::CRealFft fft;
            ASSERT_TRUE( fft.Prepare( size ) );
            const TDoubles x = MakeSignal( size, 77 + size );

            GUCEF::vector< Float64 > separate( size + 2 );
            GUCEF::vector< Float64 > inPlace = Convert< Float64 >( x, size + 2 );
            ASSERT_TRUE( fft.Forward( &x[ 0 ], &separate[ 0 ] ) );
            ASSERT_TRUE( fft.Forward( &inPlace[ 0 ], &inPlace[ 0 ] ) );
            ASSERT_TRUE( separate == inPlace );

            GUCEF::vector< Float64 > backSeparate( size );
            ASSERT_TRUE( fft.Inverse( &separate[ 0 ], &backSeparate[ 0 ] ) );
            ASSERT_TRUE( fft.Inverse( &inPlace[ 0 ], &inPlace[ 0 ] ) );
            ASSERT_TRUE( 0 == ::memcmp( &backSeparate[ 0 ], &inPlace[ 0 ], size * sizeof( Float64 ) ) );

            const GUCEF::vector< Float32 > x32 = Convert< Float32 >( x );
            GUCEF::vector< Float32 > separate32( size + 2 );
            GUCEF::vector< Float32 > inPlace32 = Convert< Float32 >( x, size + 2 );
            ASSERT_TRUE( fft.Forward( &x32[ 0 ], &separate32[ 0 ] ) );
            ASSERT_TRUE( fft.Forward( &inPlace32[ 0 ], &inPlace32[ 0 ] ) );
            ASSERT_TRUE( separate32 == inPlace32 );
            GUCEF::vector< Float32 > backSeparate32( size );
            ASSERT_TRUE( fft.Inverse( &separate32[ 0 ], &backSeparate32[ 0 ] ) );
            ASSERT_TRUE( fft.Inverse( &inPlace32[ 0 ], &inPlace32[ 0 ] ) );
            ASSERT_TRUE( 0 == ::memcmp( &backSeparate32[ 0 ], &inPlace32[ 0 ], size * sizeof( Float32 ) ) );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RealFft: Prepare accepts powers of two from 64 to 4096 only, builds the precisions asked for, and can be done again" )
    try
    {
        AUDIO::CRealFft fft;
        TDoubles x( 256, 1.0 );
        GUCEF::vector< Float64 > spectrum( 258 );
        ASSERT_TRUE( 0 == fft.GetSize() && 0 == fft.GetBinCount() && 0 == fft.GetSpectrumLength() );
        ASSERT_FALSE( fft.Forward( &x[ 0 ], &spectrum[ 0 ] ) );             // not prepared

        const UInt32 refused[] = { 0, 1, 16, 32, 100, 192, 8192, 65536 };
        for ( UInt32 i=0; i<8; ++i )
            ASSERT_FALSE( fft.Prepare( refused[ i ] ) );
        ASSERT_TRUE( 0 == fft.GetSize() );
        ASSERT_TRUE( fft.Prepare( 64 ) && 64 == fft.GetSize() );
        ASSERT_TRUE( fft.Prepare( 4096 ) && 4096 == fft.GetSize() );

        ASSERT_TRUE( fft.Prepare( 256, true, false ) );
        const GUCEF::vector< Float32 > x32 = Convert< Float32 >( x );
        GUCEF::vector< Float32 > spectrum32( 258 );
        ASSERT_TRUE( fft.Forward( &x32[ 0 ], &spectrum32[ 0 ] ) );
        ASSERT_FALSE( fft.Forward( &x[ 0 ], &spectrum[ 0 ] ) );             // the Float64 path was not asked for
        ASSERT_FALSE( fft.Inverse( &spectrum[ 0 ], &x[ 0 ] ) );

        ASSERT_TRUE( fft.Prepare( 256, false, true ) );
        ASSERT_TRUE( fft.Forward( &x[ 0 ], &spectrum[ 0 ] ) );
        ASSERT_FALSE( fft.Forward( &x32[ 0 ], &spectrum32[ 0 ] ) );
        ASSERT_TRUE( fabs( spectrum[ 0 ] - 256.0 ) < 1.0e-10 );
        ASSERT_FALSE( fft.Forward( static_cast< const Float64* >( GUCEF_NULL ), &spectrum[ 0 ] ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformAudioRealFftTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioRealFft TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.RealFft" );

    PerformTransformTests();
    PerformUseTests();
}

/*-------------------------------------------------------------------------*/
