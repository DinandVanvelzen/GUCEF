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

#ifndef GUCEF_AUDIO_TESTAPP_TESTAUDIOVOICESIGNALS_H
#define GUCEF_AUDIO_TESTAPP_TESTAUDIOVOICESIGNALS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>

#ifndef GUCEF_AUDIO_MACROS_H
#include "gucefAUDIO_macros.h"
#define GUCEF_AUDIO_MACROS_H
#endif /* GUCEF_AUDIO_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Deterministic voice test signals for the tests of the voice path (detector, gate, automatic gain, sender): speech-like
 *  signals with known talkspurts, noise at a given signal to noise ratio, hum, clicks and silence with realistic timing.
 *  The same arguments always give the same samples on every platform that has IEEE floating point (the random numbers
 *  come from a generator of our own, not from the standard library). All levels are RMS values in the linear scale of the
 *  samples (1.0 is full scale), the helpers ToDecibels() and FromDecibels() convert.
 *
 *  The speech-like signal is not speech and has no pretence of being: a voiced burst is a harmonic complex with a gliding
 *  fundamental (100 to 250 Hz) whose harmonics are weighted by three formant resonances and a tilt of 6 dB per octave,
 *  with the phases of the harmonics spread so the crest factor is that of a voice and not of a pulse train. Its amplitude
 *  follows a syllable envelope (sin squared, 4 syllables per second by default, falling to zero between syllables) inside
 *  talkspurts of 0.8 to 2.5 s that are separated by pauses of 0.5 to 1.5 s. The known truth is a mask of the samples
 *  that are active: those inside a talkspurt where the syllable envelope is at least 0.3 of its peak (-10 dB).
 */
namespace TestVoiceSignals {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef GUCEF::vector< GUCEF::Float32 > TSamples;
typedef GUCEF::vector< GUCEF::UInt8 > TMask;

/** A stretch of samples */
struct SSpan
{
    GUCEF::UInt64 start;
    GUCEF::UInt64 length;
    SSpan( void ) : start( 0 ), length( 0 ) {}
    SSpan( GUCEF::UInt64 start, GUCEF::UInt64 length ) : start( start ), length( length ) {}
    GUCEF::UInt64 End( void ) const { return start + length; }
};
typedef GUCEF::vector< SSpan > TSpans;

/** What MakeSpeech() makes */
struct SSpeechSpec
{
    GUCEF::UInt32 sampleRate;
    GUCEF::Float64 seconds;                 /**< length of the signal */
    GUCEF::UInt64 seed;
    GUCEF::Float64 leadingSilenceSeconds;   /**< no talkspurt starts before this */
    GUCEF::Float64 minSpurtSeconds;
    GUCEF::Float64 maxSpurtSeconds;
    GUCEF::Float64 minPauseSeconds;
    GUCEF::Float64 maxPauseSeconds;
    GUCEF::Float64 syllablesPerSecond;
    GUCEF::Float64 minFundamental;
    GUCEF::Float64 maxFundamental;
    GUCEF::Float64 activeRms;               /**< RMS over the active samples */

    SSpeechSpec( void )
        : sampleRate( 16000 ), seconds( 20.0 ), seed( 1 ), leadingSilenceSeconds( 1.0 ), minSpurtSeconds( 0.8 ), maxSpurtSeconds( 2.5 )
        , minPauseSeconds( 0.5 ), maxPauseSeconds( 1.5 ), syllablesPerSecond( 4.0 ), minFundamental( 100.0 ), maxFundamental( 250.0 ), activeRms( 0.1 )
    {}
};

/** A speech-like signal with its truth */
struct SSpeech
{
    TSamples samples;
    TMask active;               /**< 1 for the samples that are active */
    TSpans talkspurts;          /**< where the talkspurts are, the pauses in the syllables included */
    GUCEF::UInt32 sampleRate;
    SSpeech( void ) : sampleRate( 0 ) {}
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const GUCEF::Float64 Pi = 3.14159265358979323846;

/** The ratio of the syllable envelope (relative to its peak) from which on a sample counts as active */
const GUCEF::Float64 ActiveEnvelopeThreshold = 0.3;

/** A small, fast pseudo random generator (xorshift64*) that gives the same numbers everywhere */
class CRandom
{
    public:

    explicit CRandom( GUCEF::UInt64 seed ) : m_state( seed * 2685821657736338717ULL + 88172645463325252ULL ), m_spare( 0.0 ), m_haveSpare( false )
    {
        if ( 0 == m_state )
            m_state = 88172645463325252ULL;
        for ( int i=0; i<4; ++i )
            NextUInt64();
    }

    GUCEF::UInt64 NextUInt64( void )
    {
        m_state ^= m_state >> 12;
        m_state ^= m_state << 25;
        m_state ^= m_state >> 27;
        return m_state * 2685821657736338717ULL;
    }

    /** In [0,1) */
    GUCEF::Float64 NextUnit( void )
    {
        return (GUCEF::Float64) ( NextUInt64() >> 11 ) * ( 1.0 / 9007199254740992.0 );
    }

    /** In [low,high) */
    GUCEF::Float64 NextInRange( GUCEF::Float64 low, GUCEF::Float64 high )
    {
        return low + ( high - low ) * NextUnit();
    }

    /** Normal distribution, mean 0, deviation 1 (Box-Muller) */
    GUCEF::Float64 NextGaussian( void )
    {
        if ( m_haveSpare )
        {
            m_haveSpare = false;
            return m_spare;
        }
        GUCEF::Float64 u = NextUnit();
        if ( u < 1.0e-12 )
            u = 1.0e-12;
        const GUCEF::Float64 v = NextUnit();
        const GUCEF::Float64 radius = sqrt( -2.0 * log( u ) );
        m_spare = radius * sin( 2.0 * Pi * v );
        m_haveSpare = true;
        return radius * cos( 2.0 * Pi * v );
    }

    private:

    GUCEF::UInt64 m_state;
    GUCEF::Float64 m_spare;
    bool m_haveSpare;
};

inline GUCEF::Float64
ToDecibels( GUCEF::Float64 linear )
{
    return 20.0 * log10( linear > 1.0e-12 ? linear : 1.0e-12 );
}

inline GUCEF::Float64
FromDecibels( GUCEF::Float64 decibels )
{
    return pow( 10.0, decibels / 20.0 );
}

/** RMS of 'count' samples from 'start' (the end of the signal limits the count) */
inline GUCEF::Float64
Rms( const TSamples& x, size_t start, size_t count )
{
    if ( start >= x.size() )
        return 0.0;
    if ( start + count > x.size() )
        count = x.size() - start;
    if ( 0 == count )
        return 0.0;
    GUCEF::Float64 sum = 0.0;
    for ( size_t i=start; i<start+count; ++i )
        sum += (GUCEF::Float64) x[ i ] * (GUCEF::Float64) x[ i ];
    return sqrt( sum / (GUCEF::Float64) count );
}

/** RMS of the samples whose mask entry is set, 0 when there are none */
inline GUCEF::Float64
MaskedRms( const TSamples& x, const TMask& mask )
{
    GUCEF::Float64 sum = 0.0;
    size_t count = 0;
    for ( size_t i=0; i<x.size() && i<mask.size(); ++i )
    {
        if ( 0 != mask[ i ] )
        {
            sum += (GUCEF::Float64) x[ i ] * (GUCEF::Float64) x[ i ];
            ++count;
        }
    }
    return ( count > 0 ) ? sqrt( sum / (GUCEF::Float64) count ) : 0.0;
}

/** Multiplies the samples by 'gain' */
inline void
Scale( TSamples& x, GUCEF::Float64 gain )
{
    for ( size_t i=0; i<x.size(); ++i )
        x[ i ] = (GUCEF::Float32) ( (GUCEF::Float64) x[ i ] * gain );
}

/** Scales the samples so their RMS is 'rms' */
inline void
NormalizeRms( TSamples& x, GUCEF::Float64 rms )
{
    const GUCEF::Float64 current = Rms( x, 0, x.size() );
    if ( current > 0.0 )
        Scale( x, rms / current );
}

/** dst[ offset + i ] += gain * src[ i ], as far as dst reaches */
inline void
MixInto( TSamples& dst, const TSamples& src, GUCEF::Float64 gain = 1.0, size_t offset = 0 )
{
    for ( size_t i=0; i<src.size() && offset+i<dst.size(); ++i )
        dst[ offset + i ] = (GUCEF::Float32) ( (GUCEF::Float64) dst[ offset + i ] + gain * (GUCEF::Float64) src[ i ] );
}

/** Puts 'tail' behind 'head' */
inline void
Append( TSamples& head, const TSamples& tail )
{
    head.insert( head.end(), tail.begin(), tail.end() );
}

/** Digital silence */
inline TSamples
MakeSilence( size_t count )
{
    return TSamples( count, 0.0f );
}

/** The RMS of noise that is 'snrInDecibels' below a signal of RMS 'signalRms' */
inline GUCEF::Float64
NoiseRmsForSnr( GUCEF::Float64 signalRms, GUCEF::Float64 snrInDecibels )
{
    return signalRms * FromDecibels( -snrInDecibels );
}

/** White Gaussian noise with exactly the RMS 'rms' */
inline TSamples
MakeWhiteNoise( size_t count, GUCEF::Float64 rms, GUCEF::UInt64 seed )
{
    CRandom random( seed );
    TSamples x( count, 0.0f );
    for ( size_t i=0; i<count; ++i )
        x[ i ] = (GUCEF::Float32) random.NextGaussian();
    NormalizeRms( x, rms );
    return x;
}

/** Pink noise (power falling 3 dB per octave: three pole approximation of P. Kellet, within 0.05 dB from 9 Hz up to the Nyquist frequency) with exactly the RMS 'rms' */
inline TSamples
MakePinkNoise( size_t count, GUCEF::Float64 rms, GUCEF::UInt64 seed )
{
    CRandom random( seed );
    TSamples x( count, 0.0f );
    GUCEF::Float64 b0 = 0.0, b1 = 0.0, b2 = 0.0;
    for ( size_t i=0; i<count; ++i )
    {
        const GUCEF::Float64 white = random.NextGaussian();
        b0 = 0.99765 * b0 + white * 0.0990460;
        b1 = 0.96300 * b1 + white * 0.2965164;
        b2 = 0.57000 * b2 + white * 1.0526913;
        x[ i ] = (GUCEF::Float32) ( b0 + b1 + b2 + white * 0.1848 );
    }
    NormalizeRms( x, rms );
    return x;
}

/** A sine of the RMS 'rms' */
inline TSamples
MakeTone( size_t count, GUCEF::UInt32 sampleRate, GUCEF::Float64 frequency, GUCEF::Float64 rms, GUCEF::Float64 startPhase = 0.0 )
{
    TSamples x( count, 0.0f );
    const GUCEF::Float64 amplitude = rms * sqrt( 2.0 );
    for ( size_t i=0; i<count; ++i )
        x[ i ] = (GUCEF::Float32) ( amplitude * sin( startPhase + 2.0 * Pi * frequency * (GUCEF::Float64) i / (GUCEF::Float64) sampleRate ) );
    return x;
}

/** Mains hum: 'fundamental' (50 or 60 Hz) and its harmonics up to 'harmonics' with amplitudes of 1 / h and fixed phases, exactly the RMS 'rms' */
inline TSamples
MakeHum( size_t count, GUCEF::UInt32 sampleRate, GUCEF::Float64 fundamental, GUCEF::UInt32 harmonics, GUCEF::Float64 rms )
{
    TSamples x( count, 0.0f );
    for ( GUCEF::UInt32 h=1; h<=harmonics; ++h )
    {
        const GUCEF::Float64 frequency = fundamental * (GUCEF::Float64) h;
        if ( frequency >= 0.5 * (GUCEF::Float64) sampleRate )
            break;
        const GUCEF::Float64 amplitude = 1.0 / (GUCEF::Float64) h;
        const GUCEF::Float64 phase = 0.9 * (GUCEF::Float64) h * (GUCEF::Float64) h;
        for ( size_t i=0; i<count; ++i )
            x[ i ] = (GUCEF::Float32) ( (GUCEF::Float64) x[ i ] + amplitude * sin( phase + 2.0 * Pi * frequency * (GUCEF::Float64) i / (GUCEF::Float64) sampleRate ) );
    }
    NormalizeRms( x, rms );
    return x;
}

/**
 *  Clicks: every 'periodSeconds' (jittered by up to 25 %) a pulse of 'widthSamples' samples (1 is an impulse, more is a
 *  short exponentially decaying tick) with the peak value 'peak', alternating in sign. The positions are appended to 'positions' when given
 */
inline TSamples
MakeClicks( size_t count, GUCEF::UInt32 sampleRate, GUCEF::Float64 periodSeconds, GUCEF::Float64 peak, GUCEF::UInt32 widthSamples, GUCEF::UInt64 seed, TSpans* positions = GUCEF_NULL )
{
    CRandom random( seed );
    TSamples x( count, 0.0f );
    GUCEF::Float64 at = periodSeconds * (GUCEF::Float64) sampleRate * 0.5;
    GUCEF::Float64 sign = 1.0;
    while ( at < (GUCEF::Float64) count )
    {
        const size_t position = (size_t) at;
        for ( GUCEF::UInt32 w=0; w<widthSamples && position+w<count; ++w )
            x[ position + w ] = (GUCEF::Float32) ( sign * peak * exp( -3.0 * (GUCEF::Float64) w / (GUCEF::Float64) ( widthSamples > 0 ? widthSamples : 1 ) ) );
        if ( GUCEF_NULL != positions )
            positions->push_back( SSpan( position, widthSamples ) );
        sign = -sign;
        at += periodSeconds * (GUCEF::Float64) sampleRate * random.NextInRange( 0.75, 1.25 );
    }
    return x;
}

/**
 *  Adds a voiced burst to 'x' from sample 'start' for 'length' samples: a harmonic complex with a fundamental gliding from
 *  'f0Start' to 'f0End' (half cosine), formants scaled by 'formantShift' (1 is a mid vowel), and an amplitude that follows the
 *  syllable envelope (sin squared) when 'syllablesPerSecond' is above 0, flat otherwise, with a raised cosine attack and
 *  release of 'rampSamples'. The peak of the harmonic sum is not normalised: scale the result. Where the envelope (attack and
 *  syllables together) is at least ActiveEnvelopeThreshold the entry of 'active' (when given) is set to 1
 */
inline void
AddVoicedBurst( TSamples& x, TMask* active, GUCEF::UInt32 sampleRate, size_t start, size_t length, GUCEF::Float64 f0Start, GUCEF::Float64 f0End,
                GUCEF::Float64 formantShift, GUCEF::Float64 syllablesPerSecond, size_t rampSamples, GUCEF::UInt64 seed )
{
    const GUCEF::Float64 rate = (GUCEF::Float64) sampleRate;
    const GUCEF::Float64 maximumFrequency = ( 4500.0 < 0.45 * rate ) ? 4500.0 : 0.45 * rate;
    const GUCEF::Float64 formants[ 3 ] = { 600.0 * formantShift, 1500.0 * formantShift, 2600.0 * formantShift };
    const GUCEF::Float64 widths[ 3 ] = { 120.0, 180.0, 260.0 };
    const GUCEF::Float64 weights[ 3 ] = { 1.0, 0.6, 0.3 };

    CRandom random( seed );
    GUCEF::Float64 syllableLength = ( syllablesPerSecond > 0.0 ) ? rate / syllablesPerSecond : 0.0;
    GUCEF::Float64 syllableStart = 0.0;
    GUCEF::Float64 syllableTotal = syllableLength * random.NextInRange( 0.85, 1.15 );

    GUCEF::Float64 phase = random.NextInRange( 0.0, 2.0 * Pi );
    GUCEF::Float64 gains[ 64 ];
    GUCEF::UInt32 harmonicCount = 0;
    for ( size_t i=0; i<length && start+i<x.size(); ++i )
    {
        const GUCEF::Float64 u = ( length > 1 ) ? (GUCEF::Float64) i / (GUCEF::Float64) ( length - 1 ) : 0.0;
        const GUCEF::Float64 f0 = f0Start + ( f0End - f0Start ) * 0.5 * ( 1.0 - cos( Pi * u ) );
        phase += 2.0 * Pi * f0 / rate;
        if ( phase > 2.0 * Pi )
            phase -= 2.0 * Pi;

        // the gains of the harmonics follow the fundamental, updated every 32 samples
        if ( 0 == ( i % 32 ) )
        {
            harmonicCount = (GUCEF::UInt32) ( maximumFrequency / f0 );
            if ( harmonicCount > 64 )
                harmonicCount = 64;
            for ( GUCEF::UInt32 h=1; h<=harmonicCount; ++h )
            {
                const GUCEF::Float64 f = f0 * (GUCEF::Float64) h;
                GUCEF::Float64 shape = 0.1;
                for ( int k=0; k<3; ++k )
                {
                    const GUCEF::Float64 d = ( f - formants[ k ] ) / widths[ k ];
                    shape += weights[ k ] * exp( -0.5 * d * d );
                }
                gains[ h - 1 ] = shape / ( 1.0 + f / 500.0 );
            }
        }

        GUCEF::Float64 sample = 0.0;
        for ( GUCEF::UInt32 h=1; h<=harmonicCount; ++h )
            sample += gains[ h - 1 ] * sin( (GUCEF::Float64) h * phase + 0.7 * (GUCEF::Float64) ( h * h ) );

        // the envelope: syllables inside the attack and release ramps
        GUCEF::Float64 envelope = 1.0;
        if ( syllableLength > 0.0 )
        {
            while ( (GUCEF::Float64) i >= syllableStart + syllableTotal )
            {
                syllableStart += syllableTotal;
                syllableTotal = syllableLength * random.NextInRange( 0.85, 1.15 );
            }
            const GUCEF::Float64 s = sin( Pi * ( (GUCEF::Float64) i - syllableStart ) / syllableTotal );
            envelope = s * s;
        }
        GUCEF::Float64 ramp = 1.0;
        if ( rampSamples > 0 )
        {
            if ( i < rampSamples )
                ramp = 0.5 * ( 1.0 - cos( Pi * (GUCEF::Float64) i / (GUCEF::Float64) rampSamples ) );
            else
            if ( length - i <= rampSamples )
                ramp = 0.5 * ( 1.0 - cos( Pi * (GUCEF::Float64) ( length - i ) / (GUCEF::Float64) rampSamples ) );
        }
        envelope *= ramp;
        x[ start + i ] = (GUCEF::Float32) ( (GUCEF::Float64) x[ start + i ] + envelope * sample );
        if ( GUCEF_NULL != active && envelope >= ActiveEnvelopeThreshold && start + i < active->size() )
            ( *active )[ start + i ] = 1;
    }
}

/** A speech-like signal as described at the top of this file, with its truth. The active RMS is exactly 'spec.activeRms' */
inline void
MakeSpeech( const SSpeechSpec& spec, SSpeech& speech )
{
    const size_t count = (size_t) ( spec.seconds * (GUCEF::Float64) spec.sampleRate );
    speech.sampleRate = spec.sampleRate;
    speech.samples.assign( count, 0.0f );
    speech.active.assign( count, 0 );
    speech.talkspurts.clear();

    CRandom random( spec.seed );
    GUCEF::Float64 at = spec.leadingSilenceSeconds * (GUCEF::Float64) spec.sampleRate;
    while ( true )
    {
        const size_t length = (size_t) ( random.NextInRange( spec.minSpurtSeconds, spec.maxSpurtSeconds ) * (GUCEF::Float64) spec.sampleRate );
        const size_t start = (size_t) at;
        if ( start + length >= count )
            break;
        const GUCEF::Float64 f0Start = random.NextInRange( spec.minFundamental, spec.maxFundamental );
        const GUCEF::Float64 f0End = random.NextInRange( spec.minFundamental, spec.maxFundamental );
        const GUCEF::Float64 formantShift = random.NextInRange( 0.8, 1.25 );
        AddVoicedBurst( speech.samples, &speech.active, spec.sampleRate, start, length, f0Start, f0End, formantShift, spec.syllablesPerSecond,
                        (size_t) ( 0.008 * (GUCEF::Float64) spec.sampleRate ), random.NextUInt64() );
        speech.talkspurts.push_back( SSpan( start, length ) );
        at = (GUCEF::Float64) ( start + length ) + random.NextInRange( spec.minPauseSeconds, spec.maxPauseSeconds ) * (GUCEF::Float64) spec.sampleRate;
    }

    const GUCEF::Float64 current = MaskedRms( speech.samples, speech.active );
    if ( current > 0.0 )
        Scale( speech.samples, spec.activeRms / current );
}

/**
 *  'speech' with noise of the given ratio added: the signal to noise ratio is the RMS over the active samples of the speech
 *  against the RMS of the noise over the whole signal, both before the filtering of any detector. 'noise' is scaled to fit
 *  (its own RMS does not matter) and must be as long as the speech
 */
inline TSamples
MixAtSnr( const SSpeech& speech, const TSamples& noise, GUCEF::Float64 snrInDecibels )
{
    TSamples mixed = speech.samples;
    const GUCEF::Float64 activeRms = MaskedRms( speech.samples, speech.active );
    const GUCEF::Float64 noiseRms = Rms( noise, 0, noise.size() );
    if ( noiseRms > 0.0 )
        MixInto( mixed, noise, NoiseRmsForSnr( activeRms, snrInDecibels ) / noiseRms );
    return mixed;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace TestVoiceSignals */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_TESTAPP_TESTAUDIOVOICESIGNALS_H ? */
