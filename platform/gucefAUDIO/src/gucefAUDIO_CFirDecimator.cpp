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
#include <string.h>

#include "gucefAUDIO_CFirDecimator.h"

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

#if defined( __GNUC__ ) || defined( __clang__ ) || defined( _MSC_VER )
  #define GUCEF_AUDIO_FIR_RESTRICT __restrict
#else
  #define GUCEF_AUDIO_FIR_RESTRICT
#endif

/**
 *  Dot product with eight partial sums kept in a small array: the compiler may not reorder a single float sum, the
 *  eight independent ones it turns into vector operations (also with the cheap cost model of -O2)
 */
Float32
Dot( const Float32* GUCEF_AUDIO_FIR_RESTRICT a, const Float32* GUCEF_AUDIO_FIR_RESTRICT b, UInt32 count )
{
    Float32 sums[ 8 ] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    UInt32 t = 0;
    for ( ; t + 8 <= count; t += 8 )
    {
        sums[ 0 ] += a[ t ] * b[ t ];
        sums[ 1 ] += a[ t + 1 ] * b[ t + 1 ];
        sums[ 2 ] += a[ t + 2 ] * b[ t + 2 ];
        sums[ 3 ] += a[ t + 3 ] * b[ t + 3 ];
        sums[ 4 ] += a[ t + 4 ] * b[ t + 4 ];
        sums[ 5 ] += a[ t + 5 ] * b[ t + 5 ];
        sums[ 6 ] += a[ t + 6 ] * b[ t + 6 ];
        sums[ 7 ] += a[ t + 7 ] * b[ t + 7 ];
    }
    Float32 tail = 0.0f;
    for ( ; t<count; ++t )
        tail += a[ t ] * b[ t ];
    return ( ( sums[ 0 ] + sums[ 4 ] ) + ( sums[ 1 ] + sums[ 5 ] ) ) + ( ( sums[ 2 ] + sums[ 6 ] ) + ( sums[ 3 ] + sums[ 7 ] ) ) + tail;
}

/** Modified Bessel function of the first kind, order 0 (series), for the Kaiser window */
Float64
BesselI0( Float64 x )
{
    Float64 sum = 1.0, term = 1.0;
    const Float64 half = x * 0.5;
    for ( UInt32 k=1; k<64; ++k )
    {
        term *= ( half / (Float64) k ) * ( half / (Float64) k );
        sum += term;
        if ( term < sum * 1.0e-17 )
            break;
    }
    return sum;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CFirDecimator::CFirDecimator( void )
    : m_factor( 0 )
    , m_taps( 0 )
    , m_coefficients( GUCEF_NULL )
    , m_history( GUCEF_NULL )
    , m_write( 0 )
    , m_phase( 0 )
    , m_zeroRun( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFirDecimator::~CFirDecimator()
{GUCEF_TRACE;

    Release();
}

/*-------------------------------------------------------------------------*/

bool
CFirDecimator::Prepare( UInt32 factor, UInt32 tapsPerPhase, Float64 cutoff, Float64 beta )
{GUCEF_TRACE;

    Release();
    if ( factor < 1 || factor > 16 || tapsPerPhase < 4 || tapsPerPhase > 64 || !( cutoff >= 0.1 && cutoff <= 0.5 ) || !( beta >= 0.0 && beta <= 20.0 ) )
        return false;

    m_factor = factor;
    if ( 1 == factor )
        return true;

    const UInt32 taps = factor * tapsPerPhase;
    m_taps = taps;
    m_coefficients = GUCEF_NEW Float32[ taps ];
    m_history = GUCEF_NEW Float32[ 2 * taps ];

    // windowed sinc around the centre of the taps, cut-off relative to the input rate, normalised to a gain of 1 at DC
    const Float64 fc = cutoff / (Float64) factor;
    const Float64 centre = 0.5 * (Float64) ( taps - 1 );
    const Float64 i0Beta = BesselI0( beta );
    Float64 sum = 0.0;
    for ( UInt32 n=0; n<taps; ++n )
    {
        const Float64 t = (Float64) n - centre;
        const Float64 x = 2.0 * 3.14159265358979323846 * fc * t;
        const Float64 sinc = ( fabs( t ) < 1.0e-12 ) ? 2.0 * fc : sin( x ) / ( 3.14159265358979323846 * t );
        const Float64 r = t / ( centre + 0.5 );
        const Float64 window = BesselI0( beta * sqrt( r * r < 1.0 ? 1.0 - r * r : 0.0 ) ) / i0Beta;
        const Float64 value = sinc * window;
        m_coefficients[ n ] = (Float32) value;
        sum += value;
    }
    for ( UInt32 n=0; n<taps; ++n )
        m_coefficients[ n ] = (Float32) ( (Float64) m_coefficients[ n ] / sum );

    Reset();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CFirDecimator::Release( void )
{GUCEF_TRACE;

    GUCEF_DELETE[] m_coefficients;
    m_coefficients = GUCEF_NULL;
    GUCEF_DELETE[] m_history;
    m_history = GUCEF_NULL;
    m_factor = 0;
    m_taps = 0;
    m_write = 0;
    m_phase = 0;
    m_zeroRun = 0;
}

/*-------------------------------------------------------------------------*/

bool
CFirDecimator::IsPrepared( void ) const
{
    // no trace macro: the real time threads use it
    return 0 != m_factor;
}

/*-------------------------------------------------------------------------*/

UInt32
CFirDecimator::GetFactor( void ) const
{
    // no trace macro: the real time threads use it
    return m_factor;
}

/*-------------------------------------------------------------------------*/

UInt32
CFirDecimator::GetTapCount( void ) const
{
    // no trace macro: the real time threads use it
    return m_taps;
}

/*-------------------------------------------------------------------------*/

UInt32
CFirDecimator::GetLatencyInFrames( void ) const
{
    // no trace macro: the real time threads use it
    if ( m_factor <= 1 || 0 == m_taps )
        return 0;
    return ( m_taps - 1 ) / ( 2 * m_factor );
}

/*-------------------------------------------------------------------------*/

void
CFirDecimator::Reset( void )
{
    // no trace macro: the audio thread may reset
    if ( GUCEF_NULL != m_history )
        memset( m_history, 0, (size_t) 2 * m_taps * sizeof( Float32 ) );
    m_write = 0;
    m_phase = 0;
    m_zeroRun = m_taps;
}

/*-------------------------------------------------------------------------*/

UInt32
CFirDecimator::GetOutputCount( UInt32 inputFrames ) const
{
    if ( 0 == m_factor )
        return 0;
    if ( 1 == m_factor )
        return inputFrames;
    return ( m_phase + inputFrames ) / m_factor;
}

/*-------------------------------------------------------------------------*/

UInt32
CFirDecimator::Process( const Float32* input, UInt32 frames, Float32* output )
{
    // no trace macro: this runs on the audio thread
    if ( 0 == m_factor || GUCEF_NULL == input || GUCEF_NULL == output )
        return 0;
    if ( 1 == m_factor )
    {
        memcpy( output, input, (size_t) frames * sizeof( Float32 ) );
        return frames;
    }

    const UInt32 taps = m_taps;
    const UInt32 factor = m_factor;

    // silence on silence: no arithmetic, only the positions move on
    if ( m_zeroRun >= taps )
    {
        bool zero = true;
        for ( UInt32 i=0; i<frames && zero; ++i )
            zero = ( 0.0f == input[ i ] );
        if ( zero )
        {
            const UInt32 produced = ( m_phase + frames ) / factor;
            if ( produced > 0 )
                memset( output, 0, (size_t) produced * sizeof( Float32 ) );
            m_phase = ( m_phase + frames ) % factor;
            m_write = (UInt32) ( ( (UInt64) m_write + frames ) % taps );
            return produced;
        }
    }

    const Float32* coefficients = m_coefficients;
    Float32* history = m_history;
    UInt32 write = m_write;
    UInt32 phase = m_phase;
    UInt32 zeroRun = m_zeroRun;
    UInt32 produced = 0;
    UInt32 done = 0;
    while ( done < frames )
    {
        // a run that does not wrap the history. The second copy is written first: the windows of the outputs in the run
        // read the newer inputs from it and the older ones from the first copy, which is written after them
        UInt32 run = taps - write;
        if ( run > frames - done )
            run = frames - done;
        const Float32* in = input + done;
        memcpy( history + write + taps, in, (size_t) run * sizeof( Float32 ) );

        // the outputs fall on every factor-th input: after input i the window from the oldest input to the newest starts
        // at write + i + 1; the filter is symmetric, so the order of the taps does not matter
        for ( UInt32 i=factor - 1 - phase; i<run; i += factor )
            output[ produced++ ] = Dot( coefficients, history + write + i + 1, taps );
        memcpy( history + write, in, (size_t) run * sizeof( Float32 ) );

        // the zeros at the end of the run continue the zero run when the whole run is zero
        UInt32 trailing = 0;
        while ( trailing < run && 0.0f == in[ run - 1 - trailing ] )
            ++trailing;
        zeroRun = ( trailing == run ) ? ( zeroRun + run < taps ? zeroRun + run : taps ) : trailing;

        phase = ( phase + run ) % factor;
        write += run;
        if ( write == taps )
            write = 0;
        done += run;
    }
    m_write = write;
    m_phase = phase;
    m_zeroRun = zeroRun;
    return produced;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
