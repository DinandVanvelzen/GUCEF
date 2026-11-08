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

#ifndef GUCEF_AUDIO_TESTAPP_TESTAUDIOVOICECODECSUPPORT_H
#define GUCEF_AUDIO_TESTAPP_TESTAUDIOVOICECODECSUPPORT_H

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

#ifndef GUCEF_AUDIO_CAUDIOGLOBAL_H
#include "gucefAUDIO_CAudioGlobal.h"
#define GUCEF_AUDIO_CAUDIOGLOBAL_H
#endif /* GUCEF_AUDIO_CAUDIOGLOBAL_H ? */

#ifndef GUCEF_AUDIO_CVOICECODECREGISTRY_H
#include "gucefAUDIO_CVoiceCodecRegistry.h"
#define GUCEF_AUDIO_CVOICECODECREGISTRY_H
#endif /* GUCEF_AUDIO_CVOICECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#include "TestAudioEffectSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#if defined( __has_feature )
  #if __has_feature( realtime_sanitizer )
    #define GUCEF_AUDIO_VOICETEST_RTSAN_BUILD
  #endif
#endif

/**
 *  What the tests of the voice codecs share: configurations, reproducible voice-like signals made in code (no stored
 *  files), measurements and wrappers that run the codec calls as a real time context in a RealtimeSanitizer build.
 *  Header only, inline.
 */
namespace VoiceCodecTest {

using namespace GUCEF;

typedef AudioEffectTest::TSamples TSamples;

/*-------------------------------------------------------------------------*/

inline AUDIO::CVoiceCodecConfig
MakeConfig( const char* codec, UInt32 sampleRate, UInt32 frameDurationInUs )
{
    AUDIO::CVoiceCodecConfig config;
    config.SetCodec( codec );
    config.SetSampleRate( sampleRate );
    config.SetFrameDurationInUs( frameDurationInUs );
    return config;
}

/*-------------------------------------------------------------------------*/

/** A harmonic series (1/k amplitudes up to 3.4 kHz, at most 6 harmonics) at 'f0', peak amplitude 'amplitude': a clear, stationary pitch */
inline TSamples
MakeHarmonic( UInt32 frames, Float64 f0, UInt32 sampleRate, Float32 amplitude )
{
    TSamples signal( frames, 0.0f );
    UInt32 harmonics = 0;
    while ( harmonics < 6 && f0 * (Float64) ( harmonics + 1 ) < 3400.0 )
        ++harmonics;

    Float64 peak = 1.0e-9;
    GUCEF::vector< Float64 > raw( frames, 0.0 );
    for ( UInt32 n=0; n<frames; ++n )
    {
        Float64 sum = 0.0;
        for ( UInt32 k=1; k<=harmonics; ++k )
            sum += sin( 2.0 * AudioEffectTest::Pi * f0 * (Float64) k * (Float64) n / (Float64) sampleRate ) / (Float64) k;
        raw[ n ] = sum;
        if ( fabs( sum ) > peak )
            peak = fabs( sum );
    }
    for ( UInt32 n=0; n<frames; ++n )
        signal[ n ] = (Float32) ( raw[ n ] * (Float64) amplitude / peak );
    return signal;
}

/*-------------------------------------------------------------------------*/

/**
 *  A speech-like signal: a pitch gliding between 100 and 180 Hz at 2.3 Hz, harmonics with 1/k amplitudes up to 3.4 kHz,
 *  an amplitude that rises and falls like syllables at 3 Hz, and noise 40 dB down. Peak about 'amplitude'. Fixed seed.
 */
inline TSamples
MakeSpeechLike( UInt32 frames, UInt32 sampleRate, Float32 amplitude, UInt32 seed = 12345 )
{
    AudioEffectTest::CXorShift random( seed );
    TSamples signal( frames, 0.0f );
    Float64 phase = 0.0;
    Float64 peak = 1.0e-9;
    GUCEF::vector< Float64 > raw( frames, 0.0 );
    for ( UInt32 n=0; n<frames; ++n )
    {
        const Float64 t = (Float64) n / (Float64) sampleRate;
        const Float64 f0 = 140.0 + 40.0 * sin( 2.0 * AudioEffectTest::Pi * 2.3 * t );
        phase += 2.0 * AudioEffectTest::Pi * f0 / (Float64) sampleRate;
        Float64 sum = 0.0;
        for ( UInt32 k=1; (Float64) k * 180.0 < 3400.0 && (Float64) k * f0 < 0.45 * (Float64) sampleRate; ++k )
            sum += sin( (Float64) k * phase ) / (Float64) k;
        const Float64 envelope = 0.55 + 0.45 * sin( 2.0 * AudioEffectTest::Pi * 3.0 * t );
        raw[ n ] = sum * envelope + 0.01 * (Float64) random.NextFloat();
        if ( fabs( raw[ n ] ) > peak )
            peak = fabs( raw[ n ] );
    }
    for ( UInt32 n=0; n<frames; ++n )
        signal[ n ] = (Float32) ( raw[ n ] * (Float64) amplitude / peak );
    return signal;
}

/*-------------------------------------------------------------------------*/

/**
 *  Loads a voice codec plugin from the directory of the test application. True when it is there and the codec 'codec' is
 *  in the voice codec registry afterwards
 */
inline bool
LoadVoiceCodecPlugin( const char* pluginName, const char* group, const char* codec )
{
    AUDIO::CVoiceCodecRegistry::TRegisteredFactoryPtr factory;
    if ( AUDIO::CAudioGlobal::Instance()->GetVoiceCodecRegistry().TryFindFactory( codec, factory ) )
        return true;

    #ifdef GUCEF_DEBUG_MODE
    const CORE::CString first = CORE::CString( "$MODULEDIR$/" ) + pluginName + "_d";
    const CORE::CString second = CORE::CString( "$MODULEDIR$/" ) + pluginName;
    #else
    const CORE::CString first = CORE::CString( "$MODULEDIR$/" ) + pluginName;
    const CORE::CString second = CORE::CString( "$MODULEDIR$/" ) + pluginName + "_d";
    #endif

    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    return ( pluginControl.AddPluginFromDir( first, group, true ) || pluginControl.AddPluginFromDir( second, group, true ) ) &&
           AUDIO::CAudioGlobal::Instance()->GetVoiceCodecRegistry().TryFindFactory( codec, factory );
}

/*-------------------------------------------------------------------------*/

/** The root mean square of a range */
inline Float64
Rms( const TSamples& signal, size_t from, size_t to )
{
    Float64 sum = 0.0;
    for ( size_t n=from; n<to; ++n )
        sum += (Float64) signal[ n ] * (Float64) signal[ n ];
    return to > from ? sqrt( sum / (Float64) ( to - from ) ) : 0.0;
}

/** The largest absolute value of a range */
inline Float32
PeakOfRange( const TSamples& signal, size_t from, size_t to )
{
    Float32 peak = 0.0f;
    for ( size_t n=from; n<to; ++n )
        if ( fabsf( signal[ n ] ) > peak )
            peak = fabsf( signal[ n ] );
    return peak;
}

/** The largest absolute second difference x[n+1] - 2 x[n] + x[n-1] over a range of centre positions */
inline Float32
MaxSecondDifference( const TSamples& signal, size_t from, size_t to )
{
    Float32 largest = 0.0f;
    for ( size_t n=( from < 1 ? 1 : from ); n<to && n+1<signal.size(); ++n )
    {
        const Float32 d = fabsf( signal[ n + 1 ] - 2.0f * signal[ n ] + signal[ n - 1 ] );
        if ( d > largest )
            largest = d;
    }
    return largest;
}

/** The signal to noise ratio in dB of 'test' against 'reference' with 'test' delayed by 'delay' samples, over the reference range */
inline Float64
SnrAtDelay( const TSamples& reference, const TSamples& test, size_t delay, size_t from, size_t to )
{
    Float64 signalEnergy = 0.0, errorEnergy = 1.0e-30;
    for ( size_t n=from; n<to && n+delay<test.size(); ++n )
    {
        const Float64 r = reference[ n ];
        const Float64 e = (Float64) test[ n + delay ] - r;
        signalEnergy += r * r;
        errorEnergy += e * e;
    }
    return 10.0 * log10( signalEnergy / errorEnergy + 1.0e-30 );
}

/** The delay in 0 to 'maxDelay' samples with the best SNR, which is returned in 'snr' */
inline size_t
BestDelay( const TSamples& reference, const TSamples& test, size_t maxDelay, size_t from, size_t to, Float64& snr )
{
    size_t best = 0;
    snr = -1000.0;
    for ( size_t d=0; d<=maxDelay; ++d )
    {
        const Float64 s = SnrAtDelay( reference, test, d, from, to );
        if ( s > snr )
        {
            snr = s;
            best = d;
        }
    }
    return best;
}

/** The normalised correlation of two ranges of the same length */
inline Float64
Correlation( const TSamples& a, size_t fromA, const TSamples& b, size_t fromB, size_t count )
{
    Float64 dot = 0.0, energyA = 1.0e-30, energyB = 1.0e-30;
    for ( size_t n=0; n<count; ++n )
    {
        dot += (Float64) a[ fromA + n ] * (Float64) b[ fromB + n ];
        energyA += (Float64) a[ fromA + n ] * (Float64) a[ fromA + n ];
        energyB += (Float64) b[ fromB + n ] * (Float64) b[ fromB + n ];
    }
    return dot / sqrt( energyA * energyB );
}

/**
 *  The positive-going zero crossings in a range. 'first' and 'last' get the sample index of the first and last
 *  crossing (the sample after the sign change), so the mean period is ( last - first ) / ( count - 1 )
 */
inline UInt32
CountZeroCrossings( const TSamples& signal, size_t from, size_t to, size_t& first, size_t& last )
{
    UInt32 count = 0;
    first = last = 0;
    for ( size_t n=( from < 1 ? 1 : from ); n<to; ++n )
    {
        if ( signal[ n - 1 ] < 0.0f && signal[ n ] >= 0.0f )
        {
            if ( 0 == count )
                first = n;
            last = n;
            ++count;
        }
    }
    return count;
}

/*-------------------------------------------------------------------------*/

/** Whether every sample is finite */
inline bool
AllFinite( const TSamples& signal )
{
    for ( size_t n=0; n<signal.size(); ++n )
        if ( !( fabsf( signal[ n ] ) < 1.0e6f ) )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

/*
 *  Calls of the codecs as the audio threads make them: a real time context in a RealtimeSanitizer build, so any
 *  allocation, lock or blocking call fails the run
 */

inline Int32
EncodeRealtime( AUDIO::CVoiceEncoder& encoder, const Float32* frame, UInt8* payload, UInt32 capacity ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    return encoder.Encode( frame, payload, capacity );
}

inline Int32
DecodeRealtime( AUDIO::CVoiceDecoder& decoder, const UInt8* payload, UInt32 size, Float32* pcm, UInt32 capacity ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    return decoder.Decode( payload, size, pcm, capacity );
}

inline Int32
ConcealRealtime( AUDIO::CVoiceDecoder& decoder, Float32* pcm, UInt32 frames ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    return decoder.Conceal( pcm, frames );
}

inline Int32
DecodeFecRealtime( AUDIO::CVoiceDecoder& decoder, const UInt8* payload, UInt32 size, Float32* pcm, UInt32 frames ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    return decoder.DecodeFec( payload, size, pcm, frames );
}

inline void
ResetRealtime( AUDIO::CVoiceEncoder& encoder, AUDIO::CVoiceDecoder& decoder ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    encoder.Reset();
    decoder.Reset();
}

/*-------------------------------------------------------------------------*/

}; /* namespace VoiceCodecTest */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_TESTAPP_TESTAUDIOVOICECODECSUPPORT_H ? */
