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

#ifndef GUCEF_AUDIO_TESTAPP_TESTAUDIOECHOSIMULATION_H
#define GUCEF_AUDIO_TESTAPP_TESTAUDIOECHOSIMULATION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>
#include <string.h>

#ifndef GUCEF_AUDIO_CACOUSTICECHOCANCELLER_H
#include "gucefAUDIO_CAcousticEchoCanceller.h"
#define GUCEF_AUDIO_CACOUSTICECHOCANCELLER_H
#endif /* GUCEF_AUDIO_CACOUSTICECHOCANCELLER_H ? */

#ifndef GUCEF_AUDIO_CREALFFT_H
#include "gucefAUDIO_CRealFft.h"
#define GUCEF_AUDIO_CREALFFT_H
#endif /* GUCEF_AUDIO_CREALFFT_H ? */

#include "TestAudioVoiceSignals.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Simulation of the echo of a unit for the tests of the echo canceller: room impulse responses (direct path with early
 *  reflections, or a reverberant tail of exponentially decaying noise), FFT convolution, the playback of the reference
 *  at the capture clock (the playback buffer and a drift between the two clocks), a far talker with voiced and unvoiced
 *  parts, and a rig that drives a CAcousticEchoCanceller as the engine does: per period the render side writes the
 *  reference, then the capture side processes the microphone. Everything from fixed seeds, Float64 where it is reference
 *  computation. Header only, inline; specific to the gucefAUDIO tests.
 */
namespace EchoTest {

typedef GUCEF::vector< GUCEF::Float32 > TSamples;
typedef GUCEF::vector< GUCEF::Float64 > TResponse;

/*-------------------------------------------------------------------------*/

inline GUCEF::Float64
EnergyOf( const TResponse& h )
{
    GUCEF::Float64 e = 0.0;
    for ( size_t i=0; i<h.size(); ++i )
        e += h[ i ] * h[ i ];
    return e;
}

/** Scales a response to an energy gain of 'gainDb' (the attenuation of the echo path for white noise) */
inline void
NormaliseResponse( TResponse& h, GUCEF::Float64 gainDb )
{
    const GUCEF::Float64 e = EnergyOf( h );
    if ( e <= 0.0 )
        return;
    const GUCEF::Float64 scale = sqrt( pow( 10.0, gainDb / 10.0 ) / e );
    for ( size_t i=0; i<h.size(); ++i )
        h[ i ] *= scale;
}

/**
 *  A small enclosure: the direct path after 'delayMs' and five early reflections 1.5 to 18 ms after it, 4 to 16 dB below
 *  the direct path with alternating signs, each spread over three taps (a reflection is never a single sample); energy
 *  gain 'gainDb'
 */
inline TResponse
MakeEarlyReflections( GUCEF::UInt32 rate, GUCEF::Float64 delayMs, GUCEF::Float64 gainDb, GUCEF::UInt64 seed )
{
    TestVoiceSignals::CRandom random( seed );
    const size_t delay = (size_t) ( delayMs * (GUCEF::Float64) rate / 1000.0 );
    TResponse h( delay + (size_t) ( 0.025 * rate ) + 4, 0.0 );
    const GUCEF::Float64 afterMs[] = { 0.0, 1.5, 4.0, 7.0, 11.0, 18.0 };
    const GUCEF::Float64 levelDb[] = { 0.0, -4.0, -7.0, -10.0, -13.0, -16.0 };
    for ( GUCEF::UInt32 r=0; r<6; ++r )
    {
        const size_t at = delay + (size_t) ( ( afterMs[ r ] + ( r > 0 ? random.NextInRange( -0.3, 0.3 ) : 0.0 ) ) * rate / 1000.0 );
        const GUCEF::Float64 sign = ( r % 2 ) ? -1.0 : 1.0;
        const GUCEF::Float64 a = sign * pow( 10.0, levelDb[ r ] / 20.0 );
        h[ at ] += 0.25 * a;
        h[ at + 1 ] += 0.5 * a;
        h[ at + 2 ] += 0.25 * a;
    }
    NormaliseResponse( h, gainDb );
    return h;
}

/** A reverberant room: the direct path after 'delayMs', then noise decaying by 60 dB in 't60Ms' (whose energy is the direct path's), energy gain 'gainDb' */
inline TResponse
MakeReverberantRoom( GUCEF::UInt32 rate, GUCEF::Float64 delayMs, GUCEF::Float64 gainDb, GUCEF::Float64 t60Ms, GUCEF::UInt64 seed )
{
    TestVoiceSignals::CRandom random( seed );
    const size_t delay = (size_t) ( delayMs * (GUCEF::Float64) rate / 1000.0 );
    const size_t tail = (size_t) ( t60Ms * (GUCEF::Float64) rate / 1000.0 );
    TResponse h( delay + tail + 3, 0.0 );
    h[ delay ] = 0.25;
    h[ delay + 1 ] = 0.5;
    h[ delay + 2 ] = 0.25;
    const GUCEF::Float64 direct = EnergyOf( h );
    TResponse reverb( tail, 0.0 );
    const GUCEF::Float64 decayPerSample = log( 1000.0 ) / (GUCEF::Float64) tail;      // amplitude: -60 dB at the end
    for ( size_t i=0; i<tail; ++i )
        reverb[ i ] = random.NextGaussian() * exp( -decayPerSample * (GUCEF::Float64) i );
    const GUCEF::Float64 scale = sqrt( direct / EnergyOf( reverb ) );
    for ( size_t i=0; i<tail; ++i )
        h[ delay + 3 + i ] += scale * reverb[ i ];
    NormaliseResponse( h, gainDb );
    return h;
}

/** y = x convolved with h, as long as x: uniformly partitioned FFT convolution (partitions of 2048, FFT of 4096, Float64) */
inline TSamples
Convolve( const TSamples& x, const TResponse& h )
{
    const GUCEF::UInt32 part = 2048;
    const GUCEF::UInt32 n = 2 * part;
    GUCEF::AUDIO::CRealFft fft;
    fft.Prepare( n, false, true );
    GUCEF::vector< GUCEF::Float64 > out( x.size() + n, 0.0 );
    GUCEF::vector< GUCEF::Float64 > hs( n + 2 ), xs( n + 2 ), prod( n + 2 );
    for ( size_t h0=0; h0<h.size(); h0 += part )
    {
        // the spectrum of this partition of the response
        for ( GUCEF::UInt32 i=0; i<n; ++i )
            hs[ i ] = ( i < part && h0 + i < h.size() ) ? h[ h0 + i ] : 0.0;
        fft.Forward( &hs[ 0 ], &hs[ 0 ] );
        for ( size_t x0=0; x0<x.size(); x0 += part )
        {
            for ( GUCEF::UInt32 i=0; i<n; ++i )
                xs[ i ] = ( i < part && x0 + i < x.size() ) ? (GUCEF::Float64) x[ x0 + i ] : 0.0;
            fft.Forward( &xs[ 0 ], &xs[ 0 ] );
            for ( GUCEF::UInt32 k=0; k<=n / 2; ++k )
            {
                const GUCEF::Float64 ar = hs[ 2 * k ], ai = hs[ 2 * k + 1 ], br = xs[ 2 * k ], bi = xs[ 2 * k + 1 ];
                prod[ 2 * k ] = ar * br - ai * bi;
                prod[ 2 * k + 1 ] = ar * bi + ai * br;
            }
            fft.Inverse( &prod[ 0 ], &prod[ 0 ] );
            for ( GUCEF::UInt32 i=0; i<n; ++i )
            {
                const size_t t = h0 + x0 + i;
                if ( t < out.size() )
                    out[ t ] += prod[ i ];
            }
        }
    }
    TSamples y( x.size(), 0.0f );
    for ( size_t i=0; i<x.size(); ++i )
        y[ i ] = (GUCEF::Float32) out[ i ];
    return y;
}

/** The taps of the interpolator of Interpolate(): 4096 phases of a Kaiser windowed sinc of 64 taps (beta 8, cut-off 0.95), Float64, made at the first use */
inline const GUCEF::vector< GUCEF::Float64 >&
InterpolationTable( void )
{
    static GUCEF::vector< GUCEF::Float64 > table;
    if ( table.empty() )
    {
        const GUCEF::Int32 half = 32;
        table.assign( (size_t) 4097 * 64, 0.0 );
        for ( GUCEF::UInt32 phase=0; phase<=4096; ++phase )
        {
            const GUCEF::Float64 fraction = (GUCEF::Float64) phase / 4096.0;
            GUCEF::Float64 sum = 0.0;
            for ( GUCEF::Int32 k=-half + 1; k<=half; ++k )
            {
                const GUCEF::Float64 d = (GUCEF::Float64) k - fraction;
                const GUCEF::Float64 r = d / (GUCEF::Float64) half;
                const GUCEF::Float64 arg = 1.0 - r * r;
                const GUCEF::Float64 z = 4.0 * sqrt( arg > 0.0 ? arg : 0.0 );
                GUCEF::Float64 term = 1.0, i0 = 1.0;
                for ( GUCEF::UInt32 m=1; m<30; ++m )
                {
                    term *= ( z / m ) * ( z / m );
                    i0 += term;
                }
                const GUCEF::Float64 sinc = fabs( d ) < 1.0e-12 ? 0.95 : sin( TestVoiceSignals::Pi * 0.95 * d ) / ( TestVoiceSignals::Pi * d );
                const GUCEF::Float64 value = sinc * i0;
                table[ (size_t) phase * 64 + (size_t) ( k + half - 1 ) ] = value;
                sum += value;
            }
            for ( GUCEF::UInt32 t=0; t<64; ++t )
                table[ (size_t) phase * 64 + t ] /= sum;
        }
    }
    return table;
}

/** The value of x at the fractional position 't' by a Kaiser windowed sinc of 64 taps (Float64, phases of 1/4096), zero outside */
inline GUCEF::Float64
Interpolate( const TSamples& x, GUCEF::Float64 t )
{
    const GUCEF::vector< GUCEF::Float64 >& table = InterpolationTable();
    GUCEF::Int64 whole = (GUCEF::Int64) floor( t );
    GUCEF::UInt32 phase = (GUCEF::UInt32) floor( ( t - (GUCEF::Float64) whole ) * 4096.0 + 0.5 );
    if ( phase >= 4096 )
    {
        phase = 0;
        ++whole;
    }
    if ( 0 == phase )
        return ( whole >= 0 && (size_t) whole < x.size() ) ? (GUCEF::Float64) x[ (size_t) whole ] : 0.0;
    const GUCEF::Float64* c = &table[ (size_t) phase * 64 ];
    GUCEF::Float64 sum = 0.0;
    for ( GUCEF::Int32 k=-31; k<=32; ++k )
    {
        const GUCEF::Int64 i = whole + k;
        if ( i >= 0 && (size_t) i < x.size() )
            sum += (GUCEF::Float64) x[ (size_t) i ] * c[ k + 31 ];
    }
    return sum;
}

/*-------------------------------------------------------------------------*/

/** How the rig runs the canceller */
struct SEchoRigConfig
{
    GUCEF::UInt32 rate;                 /**< processing rate of capture and reference */
    GUCEF::UInt32 period;               /**< frames per period of both devices */
    GUCEF::UInt32 playbackPeriods;      /**< the reference written plays this many periods later */
    GUCEF::Float64 driftPpm;            /**< the playback clock runs this much faster than the capture clock */
    GUCEF::UInt32 captureChunk;         /**< frames per Process() call, at most (0: the period); a period is always processed whole */

    SEchoRigConfig( void ) : rate( 16000 ), period( 80 ), playbackPeriods( 3 ), driftPpm( 0.0 ), captureChunk( 0 ) {}
};

/**
 *  What the speaker plays, sampled by the capture clock: the reference written for render frame r plays at capture
 *  frame ( r + playbackPeriods * period ) / ( 1 + drift ); without drift it is the reference delayed by the playback
 *  buffer
 */
inline TSamples
Played( const TSamples& far, const SEchoRigConfig& config, size_t length )
{
    const size_t buffer = (size_t) config.playbackPeriods * config.period;
    TSamples played( length, 0.0f );
    const GUCEF::Float64 ratio = 1.0 + config.driftPpm * 1.0e-6;
    for ( size_t n=buffer; n<length; ++n )
    {
        if ( 0.0 == config.driftPpm )
        {
            const size_t r = n - buffer;
            played[ n ] = r < far.size() ? far[ r ] : 0.0f;
        }
        else
            played[ n ] = (GUCEF::Float32) Interpolate( far, (GUCEF::Float64) ( n - buffer ) * ratio );
    }
    return played;
}

/** Told after every period of the rig (to look at the canceller as the run goes) */
class IEchoRigObserver
{
    public:

    virtual ~IEchoRigObserver() {}

    /** After the capture side processed up to frame 'captured' */
    virtual void OnPeriod( GUCEF::UInt64 captured ) = 0;
};

/** Drives a canceller as the engine does: per period the render side writes the reference, then the capture side processes */
class CEchoRig
{
    public:

    CEchoRig( GUCEF::AUDIO::CAcousticEchoCanceller& canceller, const SEchoRigConfig& config, IEchoRigObserver* observer = GUCEF_NULL )
        : m_canceller( canceller ), m_config( config ), m_observer( observer ), m_renderCarry( 0.0 ), m_rendered( 0 ), m_captured( 0 )
    {}

    bool Prepare( void )
    {
        m_renderCarry = 0.0;
        m_rendered = 0;
        m_captured = 0;
        return m_canceller.Prepare( m_config.rate, 4096, m_config.rate, 1, 4096 );
    }

    /** Runs over all of 'mic' (output in 'out', the same length); 'far' is the reference at the render clock */
    void Run( const TSamples& far, const TSamples& mic, TSamples& out )
    {
        out = mic;
        const GUCEF::Float64 ratio = 1.0 + m_config.driftPpm * 1.0e-6;
        const GUCEF::UInt32 chunk = 0 != m_config.captureChunk ? m_config.captureChunk : m_config.period;
        GUCEF::vector< GUCEF::Float32 > reference( 2 * m_config.period + 2, 0.0f );
        while ( m_captured < mic.size() )
        {
            // the render side: one period of its clock
            m_renderCarry += (GUCEF::Float64) m_config.period * ratio;
            const GUCEF::UInt32 frames = (GUCEF::UInt32) floor( m_renderCarry );
            m_renderCarry -= (GUCEF::Float64) frames;
            for ( GUCEF::UInt32 i=0; i<frames; ++i )
            {
                const size_t r = (size_t) m_rendered + i;
                reference[ i ] = r < far.size() ? far[ r ] : 0.0f;
            }
            m_canceller.WriteReference( &reference[ 0 ], frames );
            m_rendered += frames;

            // the capture side: one period, in calls of at most 'chunk' frames (the last one of the period shorter)
            GUCEF::UInt32 left = m_config.period;
            while ( left > 0 && m_captured < mic.size() )
            {
                GUCEF::UInt32 n = chunk < left ? chunk : left;
                if ( m_captured + n > mic.size() )
                    n = (GUCEF::UInt32) ( mic.size() - m_captured );
                m_canceller.Process( &out[ (size_t) m_captured ], n, m_captured );
                m_captured += n;
                left -= n;
            }
            if ( GUCEF_NULL != m_observer )
                m_observer->OnPeriod( m_captured );
        }
    }

    private:

    GUCEF::AUDIO::CAcousticEchoCanceller& m_canceller;
    SEchoRigConfig m_config;
    IEchoRigObserver* m_observer;
    GUCEF::Float64 m_renderCarry;
    GUCEF::UInt64 m_rendered;
    GUCEF::UInt64 m_captured;
};

/*-------------------------------------------------------------------------*/

/**
 *  A far talker for the echo tests: the speech-like signal of TestVoiceSignals (voiced bursts) with an unvoiced part
 *  (white noise at 15 dB below the voiced level inside the active samples), active RMS 'rmsDb'
 */
inline TSamples
MakeFarTalker( GUCEF::UInt32 rate, GUCEF::Float64 seconds, GUCEF::UInt64 seed, GUCEF::Float64 rmsDb, GUCEF::Float64 leadingSeconds = 0.5, GUCEF::Float64 maxPauseSeconds = 0.8 )
{
    TestVoiceSignals::SSpeechSpec spec;
    spec.sampleRate = rate;
    spec.seconds = seconds;
    spec.seed = seed;
    spec.leadingSilenceSeconds = leadingSeconds;
    spec.minPauseSeconds = 0.3;
    spec.maxPauseSeconds = maxPauseSeconds;
    spec.minSpurtSeconds = 1.5;
    spec.maxSpurtSeconds = 4.0;
    spec.activeRms = pow( 10.0, rmsDb / 20.0 );
    TestVoiceSignals::SSpeech speech;
    TestVoiceSignals::MakeSpeech( spec, speech );
    TSamples noise = TestVoiceSignals::MakeWhiteNoise( speech.samples.size(), spec.activeRms * pow( 10.0, -15.0 / 20.0 ), seed + 7 );
    for ( size_t i=0; i<noise.size(); ++i )
        speech.samples[ i ] += speech.active[ i ] ? noise[ i ] : 0.0f;
    return speech.samples;
}

/** A loudspeaker that compresses its peaks: 0.5 tanh( 2 x ), the nonlinear variant of the echo path */
inline TSamples
SoftClip( const TSamples& x )
{
    TSamples y( x.size(), 0.0f );
    for ( size_t i=0; i<x.size(); ++i )
        y[ i ] = (GUCEF::Float32) ( 0.5 * tanh( 2.0 * (GUCEF::Float64) x[ i ] ) );
    return y;
}

/** Echo return loss enhancement in dB over [from, to): the echo against what of it is left */
inline GUCEF::Float64
ErleDb( const TSamples& echo, const TSamples& residual, size_t from, size_t to )
{
    GUCEF::Float64 e = 0.0, r = 0.0;
    for ( size_t i=from; i<to && i<echo.size() && i<residual.size(); ++i )
    {
        e += (GUCEF::Float64) echo[ i ] * (GUCEF::Float64) echo[ i ];
        r += (GUCEF::Float64) residual[ i ] * (GUCEF::Float64) residual[ i ];
    }
    if ( e <= 0.0 )
        return 0.0;
    return r > 0.0 ? 10.0 * log10( e / r ) : 200.0;
}

/** a - b */
inline TSamples
Difference( const TSamples& a, const TSamples& b )
{
    TSamples d( a.size() < b.size() ? a.size() : b.size(), 0.0f );
    for ( size_t i=0; i<d.size(); ++i )
        d[ i ] = a[ i ] - b[ i ];
    return d;
}

/** a + b, as long as a */
inline TSamples
Sum( const TSamples& a, const TSamples& b )
{
    TSamples s = a;
    for ( size_t i=0; i<s.size() && i<b.size(); ++i )
        s[ i ] += b[ i ];
    return s;
}

/*-------------------------------------------------------------------------*/

}; /* namespace EchoTest */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_TESTAPP_TESTAUDIOECHOSIMULATION_H ? */
