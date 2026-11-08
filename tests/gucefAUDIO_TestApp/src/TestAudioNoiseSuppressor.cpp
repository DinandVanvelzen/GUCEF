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

#ifndef GUCEF_AUDIO_CAUDIOGLOBAL_H
#include "gucefAUDIO_CAudioGlobal.h"
#define GUCEF_AUDIO_CAUDIOGLOBAL_H
#endif /* GUCEF_AUDIO_CAUDIOGLOBAL_H ? */

#ifndef GUCEF_AUDIO_CAUDIOEFFECTREGISTRY_H
#include "gucefAUDIO_CAudioEffectRegistry.h"
#define GUCEF_AUDIO_CAUDIOEFFECTREGISTRY_H
#endif /* GUCEF_AUDIO_CAUDIOEFFECTREGISTRY_H ? */

#ifndef GUCEF_AUDIO_CNOISESUPPRESSOREFFECT_H
#include "gucefAUDIO_CNoiseSuppressorEffect.h"
#define GUCEF_AUDIO_CNOISESUPPRESSOREFFECT_H
#endif /* GUCEF_AUDIO_CNOISESUPPRESSOREFFECT_H ? */

#ifndef GUCEF_AUDIO_CFILTEREFFECT_H
#include "gucefAUDIO_CFilterEffect.h"
#define GUCEF_AUDIO_CFILTEREFFECT_H
#endif /* GUCEF_AUDIO_CFILTEREFFECT_H ? */

#ifndef GUCEF_AUDIO_CREALFFT_H
#include "gucefAUDIO_CRealFft.h"
#define GUCEF_AUDIO_CREALFFT_H
#endif /* GUCEF_AUDIO_CREALFFT_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAudioEffectSupport.h"
#include "TestAudioVoiceSignals.h"

#include "TestAudioNoiseSuppressor.h"

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

namespace TVS = TestVoiceSignals;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

#if defined( __has_feature )
  #if __has_feature( realtime_sanitizer )
    #define GUCEF_AUDIO_TEST_NOISESUPPRESSOR_RTSAN_BUILD
  #endif
#endif

#ifdef GUCEF_AUDIO_TEST_NOISESUPPRESSOR_RTSAN_BUILD
const bool IsRtsanBuild = true;
#else
const bool IsRtsanBuild = false;
#endif

typedef AUDIO::CNoiseSuppressorEffect TSuppressor;

const UInt32 Rate = 16000;
const UInt32 Latency = 128;         /**< the frame of the transform at 16 kHz */

bool
PrepareSuppressor( TSuppressor& suppressor, UInt32 rate = Rate, UInt32 channels = 1, UInt32 maxBlock = 4096 )
{
    return suppressor.Prepare( AUDIO::CAudioFormat( rate, channels, AUDIO::SAMPLETYPE_FLOAT32 ), maxBlock );
}

/** The high pass of the pre-echo stage of a unit (100 Hz, 24 dB per octave), in place: what reaches the suppressor */
void
HighPass( TSamples& x )
{
    AUDIO::CFilterEffect filter;
    filter.SetFilterType( AUDIO::FILTERBANDTYPE_HIGHPASS );
    filter.SetFrequency( 100.0f );
    filter.SetSlopeInDecibelsPerOctave( 24 );
    filter.Prepare( AUDIO::CAudioFormat( Rate, 1, AUDIO::SAMPLETYPE_FLOAT32 ), 4096 );
    Run( filter, x, 1, 256 );
}

/** Speech-like signal at 16 kHz with its truth; pauses of 0.6 to 1.2 s */
void
MakeTalk( TVS::SSpeech& speech, Float64 seconds, UInt64 seed, Float64 activeRmsDb )
{
    TVS::SSpeechSpec spec;
    spec.sampleRate = Rate;
    spec.seconds = seconds;
    spec.seed = seed;
    spec.leadingSilenceSeconds = 2.0;
    spec.minPauseSeconds = 0.6;
    spec.maxPauseSeconds = 1.2;
    spec.activeRms = TVS::FromDecibels( activeRmsDb );
    TVS::MakeSpeech( spec, speech );
}

/** The noise of a kind (white or pink) for a signal to noise ratio against the active speech */
TSamples
MakeNoiseFor( const TVS::SSpeech& speech, bool pink, Float64 snrDb, UInt64 seed )
{
    const Float64 rms = TVS::NoiseRmsForSnr( TVS::MaskedRms( speech.samples, speech.active ), snrDb );
    return pink ? TVS::MakePinkNoise( speech.samples.size(), rms, seed ) : TVS::MakeWhiteNoise( speech.samples.size(), rms, seed );
}

/** Runs the effect over a copy of 'input' in blocks; the output is moved back by the latency so it lines up with the input */
TSamples
Suppress( TSuppressor& suppressor, const TSamples& input, UInt32 blockFrames, UInt32 latency )
{
    TSamples work = input;
    work.resize( input.size() + latency, 0.0f );
    Run( suppressor, work, 1, blockFrames );
    return TSamples( work.begin() + latency, work.end() );
}

/**
 *  Segmental SNR of 'estimate' against 'clean' over the 10 ms frames whose samples are mostly active speech, each frame
 *  limited to -10 .. 35 dB (the usual definition), from 'from' on
 */
Float64
SegmentalSnr( const TSamples& clean, const TSamples& estimate, const TVS::TMask& active, size_t from )
{
    const size_t frame = Rate / 100;
    Float64 sum = 0.0;
    UInt32 count = 0;
    for ( size_t start=from; start + frame <= clean.size() && start + frame <= estimate.size(); start += frame )
    {
        UInt32 activeCount = 0;
        Float64 s = 0.0, n = 0.0;
        for ( size_t i=start; i<start + frame; ++i )
        {
            activeCount += active[ i ] ? 1 : 0;
            const Float64 c = (Float64) clean[ i ];
            const Float64 d = (Float64) estimate[ i ] - c;
            s += c * c;
            n += d * d;
        }
        if ( activeCount * 2 < frame || s <= 0.0 )
            continue;
        Float64 snr = n > 0.0 ? 10.0 * log10( s / n ) : 35.0;
        snr = snr < -10.0 ? -10.0 : ( snr > 35.0 ? 35.0 : snr );
        sum += snr;
        ++count;
    }
    return count > 0 ? sum / (Float64) count : 0.0;
}

/** The RMS in dB of the samples that are (or are not) active, from 'from' on, keeping 'guard' samples away from the edges of talkspurts */
Float64
RmsWhere( const TSamples& x, const TVS::TMask& active, bool wantActive, size_t from, size_t guard )
{
    Float64 sum = 0.0;
    UInt64 count = 0;
    for ( size_t i=from + guard; i + guard < x.size() && i < active.size(); ++i )
    {
        bool all = true;
        for ( size_t g=i - guard; g<=i + guard && all; g += ( guard > 0 ? guard : 1 ) )
            all = ( ( 0 != active[ g < active.size() ? g : active.size() - 1 ] ) == wantActive );
        if ( !all )
            continue;
        sum += (Float64) x[ i ] * (Float64) x[ i ];
        ++count;
    }
    return count > 0 ? TVS::ToDecibels( sqrt( sum / (Float64) count ) ) : -200.0;
}

/** Gain of 'output' along 'reference' (projection) over [from, to) */
Float64
ProjectionGainDb( const TSamples& output, const TSamples& reference, size_t from, size_t to )
{
    Float64 xy = 0.0, xx = 0.0;
    for ( size_t i=from; i<to && i<output.size() && i<reference.size(); ++i )
    {
        xy += (Float64) output[ i ] * (Float64) reference[ i ];
        xx += (Float64) reference[ i ] * (Float64) reference[ i ];
    }
    return ( xx > 0.0 && xy > 0.0 ) ? 10.0 * log10( xy / xx ) : -200.0;
}

/**
 *  Shadow filtering: runs the suppressor over 'noisy' hop by hop, takes the gains it applied to every frame, and applies
 *  the same gains to the speech and to the noise in a short-time Fourier transform of their own (the same frames and
 *  windows, Float64), so the speech and the noise after the suppressor can be measured apart. Outputs are not delayed
 */
void
ShadowFilter( TSuppressor& suppressor, const TSamples& noisy, const TSamples& speech, const TSamples& noise, TSamples& speechOut, TSamples& noiseOut )
{
    const AUDIO::CSpectralSuppressor* core = suppressor.GetSuppressor( 0 );
    const UInt32 n = core->GetFrameSize();
    const UInt32 hop = core->GetHopSize();
    const UInt32 bins = core->GetBinCount();
    AUDIO::CRealFft fft;
    fft.Prepare( n, false, true );
    GUCEF::vector< Float64 > window( n ), frame( n + 2 ), accS( noisy.size() + 2 * n, 0.0 ), accN( noisy.size() + 2 * n, 0.0 ), gains( bins );
    for ( UInt32 i=0; i<n; ++i )
        window[ i ] = sin( Pi * (Float64) i / (Float64) n );
    TSamples work = noisy;
    for ( size_t at=0; at + hop <= work.size(); at += hop )
    {
        ProcessRealtime( suppressor, &work[ at ], hop );
        for ( UInt32 k=0; k<bins; ++k )
            gains[ k ] = (Float64) core->GetGain( k );
        // the frame just processed covers the input from one hop before 'at' to the end of this hop
        const Int64 start = (Int64) at - (Int64) hop;
        for ( UInt32 which=0; which<2; ++which )
        {
            const TSamples& x = 0 == which ? speech : noise;
            GUCEF::vector< Float64 >& acc = 0 == which ? accS : accN;
            for ( UInt32 i=0; i<n; ++i )
            {
                const Int64 t = start + (Int64) i;
                frame[ i ] = ( t >= 0 && (size_t) t < x.size() ) ? (Float64) x[ (size_t) t ] * window[ i ] : 0.0;
            }
            fft.Forward( &frame[ 0 ], &frame[ 0 ] );
            for ( UInt32 k=0; k<bins; ++k )
            {
                frame[ 2 * k ] *= gains[ k ];
                frame[ 2 * k + 1 ] *= gains[ k ];
            }
            fft.Inverse( &frame[ 0 ], &frame[ 0 ] );
            for ( UInt32 i=0; i<n; ++i )
            {
                const Int64 t = start + (Int64) i;
                if ( t >= 0 && (size_t) t < acc.size() )
                    acc[ (size_t) t ] += frame[ i ] * window[ i ];
            }
        }
    }
    speechOut.assign( noisy.size(), 0.0f );
    noiseOut.assign( noisy.size(), 0.0f );
    for ( size_t i=0; i<noisy.size(); ++i )
    {
        speechOut[ i ] = (Float32) accS[ i ];
        noiseOut[ i ] = (Float32) accN[ i ];
    }
}

/** Segmental SNR of speech against noise over the 10 ms frames that are mostly active speech, each limited to -10 .. 35 dB */
Float64
SegmentalSnrApart( const TSamples& speech, const TSamples& noise, const TVS::TMask& active, size_t from, size_t to )
{
    const size_t frame = Rate / 100;
    Float64 sum = 0.0;
    UInt32 count = 0;
    for ( size_t start=from; start + frame <= to; start += frame )
    {
        UInt32 activeCount = 0;
        Float64 s = 0.0, n = 0.0;
        for ( size_t i=start; i<start + frame; ++i )
        {
            activeCount += active[ i ] ? 1 : 0;
            s += (Float64) speech[ i ] * (Float64) speech[ i ];
            n += (Float64) noise[ i ] * (Float64) noise[ i ];
        }
        if ( activeCount * 2 < frame || s <= 0.0 )
            continue;
        Float64 snr = n > 0.0 ? 10.0 * log10( s / n ) : 35.0;
        snr = snr < -10.0 ? -10.0 : ( snr > 35.0 ? 35.0 : snr );
        sum += snr;
        ++count;
    }
    return count > 0 ? sum / (Float64) count : 0.0;
}

/** Spectral flatness (geometric over arithmetic mean of a Welch power spectrum, 256 points, bins 2 .. 120) of a stretch */
Float64
SpectralFlatness( const TSamples& x, size_t from, size_t to )
{
    const UInt32 n = 256;
    AUDIO::CRealFft fft;
    fft.Prepare( n, false, true );
    GUCEF::vector< Float64 > frame( n + 2, 0.0 ), power( n / 2 + 1, 0.0 );
    UInt32 frames = 0;
    for ( size_t start=from; start + n <= to && start + n <= x.size(); start += n / 2 )
    {
        for ( UInt32 i=0; i<n; ++i )
            frame[ i ] = (Float64) x[ start + i ] * ( 0.5 - 0.5 * cos( 2.0 * Pi * (Float64) i / (Float64) n ) );
        fft.Forward( &frame[ 0 ], &frame[ 0 ] );
        for ( UInt32 k=0; k<=n / 2; ++k )
            power[ k ] += frame[ 2 * k ] * frame[ 2 * k ] + frame[ 2 * k + 1 ] * frame[ 2 * k + 1 ];
        ++frames;
    }
    Float64 logSum = 0.0, sum = 0.0;
    UInt32 bins = 0;
    for ( UInt32 k=2; k<=120; ++k )
    {
        logSum += log( power[ k ] + 1.0e-30 );
        sum += power[ k ];
        ++bins;
    }
    return ( frames > 0 && sum > 0.0 ) ? exp( logSum / bins ) / ( sum / bins ) : 0.0;
}

/*-------------------------------------------------------------------------*/

void
PerformParameterTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioNoiseSuppressor: Parameters, registry, latency per rate, saving and loading" )
    try
    {
        ASSERT_TRUE( TSuppressor::GetStaticEffectTypeName() == "noiseSuppressor" );
        ASSERT_TRUE( TSuppressor::GetStaticParameterCount() == 3 );
        const AUDIO::CAudioEffectParameterInfo* infos = TSuppressor::GetStaticParameterInfos();
        ASSERT_TRUE( infos[ TSuppressor::PARAMETER_AMOUNT ].GetName() == "amount" && infos[ TSuppressor::PARAMETER_AMOUNT ].GetDefaultValue() == -15.0f );
        ASSERT_TRUE( infos[ TSuppressor::PARAMETER_AMOUNT ].GetMinimum() == -40.0f && infos[ TSuppressor::PARAMETER_AMOUNT ].GetMaximum() == 0.0f );
        ASSERT_TRUE( infos[ TSuppressor::PARAMETER_STRENGTH ].GetName() == "strength" && infos[ TSuppressor::PARAMETER_STRENGTH ].GetDefaultValue() == 1.0f );
        ASSERT_TRUE( infos[ TSuppressor::PARAMETER_BYPASS_BELOW_HZ ].GetName() == "bypassBelowHz" && infos[ TSuppressor::PARAMETER_BYPASS_BELOW_HZ ].GetDefaultValue() == 100.0f );

        // built in: the registry makes it, without regard for case
        AUDIO::TAudioEffectPtr made = AUDIO::CAudioGlobal::Instance()->GetAudioEffectRegistry().CreateEffect( "noisesuppressor" );
        ASSERT_TRUE( !made.IsNULL() && made->GetEffectTypeName() == "noiseSuppressor" );

        // the latency is the frame of the transform, about 5.3 ms or the next power of two
        const UInt32 rates[] = { 8000, 16000, 22050, 24000, 32000, 44100, 48000 };
        const UInt32 latencies[] = { 64, 128, 128, 128, 256, 256, 256 };
        for ( UInt32 r=0; r<7; ++r )
        {
            TSuppressor suppressor;
            ASSERT_TRUE( 0 == suppressor.GetLatencyInFrames() );
            ASSERT_TRUE( PrepareSuppressor( suppressor, rates[ r ] ) );
            ASSERT_TRUE( suppressor.GetLatencyInFrames() == latencies[ r ] );
        }
        // a rate it does not support: no latency, the audio passes
        TSuppressor odd;
        ASSERT_TRUE( PrepareSuppressor( odd, 4000 ) );
        ASSERT_TRUE( 0 == odd.GetLatencyInFrames() );
        TSamples noise = MakeNoise( 1000, 1, 7, 0.1f );
        TSamples passed = noise;
        Run( odd, passed, 1, 100 );
        ASSERT_TRUE( IsSame( noise, passed ) );

        // limits and configuration round trip
        TSuppressor original;
        original.SetAmountInDecibels( -60.0f );
        ASSERT_TRUE( original.GetParameter( TSuppressor::PARAMETER_AMOUNT ) == -40.0f );
        original.SetAmountInDecibels( -22.0f );
        original.SetStrength( 1.5f );
        original.SetBypassBelowHz( 200.0f );
        CORE::CDataNode config;
        ASSERT_TRUE( original.SaveConfig( config ) );
        TSuppressor loaded;
        ASSERT_TRUE( loaded.LoadConfig( config ) );
        ASSERT_TRUE( loaded.GetParameter( TSuppressor::PARAMETER_AMOUNT ) == -22.0f );
        ASSERT_TRUE( loaded.GetParameter( TSuppressor::PARAMETER_STRENGTH ) == 1.5f );
        ASSERT_TRUE( loaded.GetParameter( TSuppressor::PARAMETER_BYPASS_BELOW_HZ ) == 200.0f );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioNoiseSuppressor: The reported latency is the delay: an amount of 0 dB delays exactly by it without a transform, and suppressing keeps a loud tone at that delay" )
    try
    {
        const UInt32 rates[] = { 8000, 16000, 48000 };
        for ( UInt32 r=0; r<3; ++r )
        {
            TSuppressor suppressor;
            suppressor.SetAmountInDecibels( 0.0f );
            ASSERT_TRUE( PrepareSuppressor( suppressor, rates[ r ] ) );
            const UInt32 latency = suppressor.GetLatencyInFrames();
            TSamples x = MakeNoise( rates[ r ], 1, 11 + r, 0.3f );
            TSamples y = x;
            Run( suppressor, y, 1, 77 );
            Float64 worst = 0.0;
            for ( size_t i=latency; i<y.size(); ++i )
            {
                const Float64 error = fabs( (Float64) y[ i ] - (Float64) x[ i - latency ] );
                worst = error > worst ? error : worst;
            }
            for ( size_t i=0; i<latency; ++i )
                ASSERT_TRUE( 0.0f == y[ i ] );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioNoiseSuppressor: at " << rates[ r ] << " Hz, amount 0 dB: delay " << latency << " frames, largest difference from the delayed input " << worst );
            ASSERT_TRUE( worst < 1.0e-6 );
            ASSERT_TRUE( 0 == suppressor.GetTransformedFrameCount() );
        }

        // with suppression: a loud tone in noise comes out at the same delay (cross-correlation peak)
        TSuppressor suppressor;
        ASSERT_TRUE( PrepareSuppressor( suppressor ) );
        TSamples tone = TVS::MakeTone( 3 * Rate, Rate, 700.0, 0.1 );
        TSamples x = tone;
        TVS::MixInto( x, TVS::MakeWhiteNoise( x.size(), 0.01, 5 ) );
        TSamples y = x;
        Run( suppressor, y, 1, 160 );
        Int32 bestLag = -1;
        Float64 best = -1.0e30;
        for ( Int32 lag=0; lag<400; ++lag )
        {
            Float64 c = 0.0;
            for ( size_t i=2 * Rate; i<3 * Rate - 400; ++i )
                c += (Float64) y[ i + lag ] * (Float64) x[ i ];
            if ( c > best )
            {
                best = c;
                bestLag = lag;
            }
        }
        // a tone repeats every period: the peak nearest the latency within half a period of 700 Hz
        ASSERT_TRUE( bestLag >= (Int32) Latency - 11 && bestLag <= (Int32) Latency + 11 );
        ASSERT_TRUE( suppressor.GetTransformedFrameCount() > 0 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformQualityTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioNoiseSuppressor: Speech-like signal in white and pink noise at 10 and 5 dB SNR: SNR improved (speech and noise measured apart by shadow filtering), noise in pauses lowered to near the floor, the speech kept within 2 dB" )
    try
    {
        TVS::SSpeech speech;
        MakeTalk( speech, 30.0, 3, -26.0 );
        const size_t from = 3 * Rate;   // after the noise estimate has settled
        // what the suppressor gets in a unit has passed the high pass of the pre-echo stage; so has the reference
        TSamples clean = speech.samples;
        HighPass( clean );
        const bool kinds[] = { false, true };
        const Float64 snrs[] = { 10.0, 5.0 };
        for ( UInt32 k=0; k<2; ++k )
        {
            for ( UInt32 s=0; s<2; ++s )
            {
                TSamples noise = MakeNoiseFor( speech, kinds[ k ], snrs[ s ], 17 + k );
                HighPass( noise );
                TSamples noisy = clean;
                TVS::MixInto( noisy, noise );

                TSuppressor suppressor;
                ASSERT_TRUE( PrepareSuppressor( suppressor ) );
                const TSamples out = Suppress( suppressor, noisy, 160, suppressor.GetLatencyInFrames() );

                const Float64 before = SegmentalSnr( clean, noisy, speech.active, from );
                const Float64 after = SegmentalSnr( clean, out, speech.active, from );
                const Float64 noiseIn = RmsWhere( noisy, speech.active, false, from, Rate / 5 );
                const Float64 noiseOut = RmsWhere( out, speech.active, false, from, Rate / 5 );
                const Float64 speechGain = ProjectionGainDb( out, clean, from, out.size() );

                // the same gains on the speech and on the noise apart
                TSuppressor shadow;
                ASSERT_TRUE( PrepareSuppressor( shadow ) );
                TSamples speechOut, noiseOut2;
                ShadowFilter( shadow, noisy, clean, noise, speechOut, noiseOut2 );
                const size_t to = clean.size() - Rate / 10;
                const Float64 segIn = SegmentalSnrApart( clean, noise, speech.active, from, to );
                const Float64 segOut = SegmentalSnrApart( speechOut, noiseOut2, speech.active, from, to );
                const Float64 globalIn = TVS::ToDecibels( TVS::Rms( clean, from, to - from ) / TVS::Rms( noise, from, to - from ) );
                const Float64 globalOut = TVS::ToDecibels( TVS::Rms( speechOut, from, to - from ) / TVS::Rms( noiseOut2, from, to - from ) );
                const Float64 distortion = TVS::ToDecibels( TVS::MaskedRms( speechOut, speech.active ) / TVS::MaskedRms( clean, speech.active ) );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioNoiseSuppressor: " << ( kinds[ k ] ? "pink" : "white" ) << " noise at " << snrs[ s ] << " dB SNR: segmental SNR of speech and noise apart (shadow filtering) " <<
                           segIn << " -> " << segOut << " dB (improved by " << segOut - segIn << " dB), whole signal " << globalIn << " -> " << globalOut << " dB (improved by " << globalOut - globalIn <<
                           " dB), active speech level " << distortion << " dB; segmental SNR of the output against the clean speech " << before << " -> " << after <<
                           " dB (" << after - before << " dB, speech distortion counted as noise); noise in the pauses " << noiseIn << " -> " << noiseOut << " dB (lowered by " << noiseIn - noiseOut << " dB); speech along itself " << speechGain << " dB" );
                // white noise: the design's 6 dB; pink noise lies under the speech's own spectrum, less can be taken out
                ASSERT_TRUE( segOut - segIn >= ( kinds[ k ] ? 3.0 : 6.0 ) );
                ASSERT_TRUE( globalOut - globalIn >= ( kinds[ k ] ? 6.0 : 9.0 ) );
                ASSERT_TRUE( distortion >= -2.0 );
                ASSERT_TRUE( after - before >= 0.0 );
                ASSERT_TRUE( noiseIn - noiseOut >= 10.0 && noiseIn - noiseOut <= 16.5 );
                ASSERT_TRUE( speechGain >= -1.5 );
            }
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioNoiseSuppressor: Clean speech (with a microphone noise 50 dB below it) changes by at most 1 dB in level" )
    try
    {
        TVS::SSpeech speech;
        MakeTalk( speech, 20.0, 5, -26.0 );
        TSamples input = speech.samples;
        TVS::MixInto( input, TVS::MakeWhiteNoise( input.size(), TVS::FromDecibels( -76.0 ), 3 ) );
        TSuppressor suppressor;
        ASSERT_TRUE( PrepareSuppressor( suppressor ) );
        const TSamples out = Suppress( suppressor, input, 64, Latency );
        const Float64 levelIn = TVS::ToDecibels( TVS::MaskedRms( TSamples( input.begin(), input.end() ), speech.active ) );
        const Float64 levelOut = TVS::ToDecibels( TVS::MaskedRms( out, speech.active ) );
        const Float64 along = ProjectionGainDb( out, speech.samples, 2 * Rate, out.size() );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioNoiseSuppressor: clean speech " << levelIn << " dB in, " << levelOut << " dB out (" << levelOut - levelIn << " dB), " << along << " dB along the input" );
        ASSERT_TRUE( fabs( levelOut - levelIn ) <= 1.0 );
        ASSERT_TRUE( along >= -1.0 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioNoiseSuppressor: Noise alone: lowered to the floor with steady gains (no musical noise: small spread of the gains, few isolated peaks, the residual as flat as the input)" )
    try
    {
        const bool kinds[] = { false, true };
        const Float32 amounts[] = { -15.0f, -25.0f };
        for ( UInt32 k=0; k<2; ++k )
        {
            for ( UInt32 a=0; a<2; ++a )
            {
                TSamples noise = kinds[ k ] ? TVS::MakePinkNoise( 12 * Rate, 0.01, 31 ) : TVS::MakeWhiteNoise( 12 * Rate, 0.01, 31 );
                HighPass( noise );
                TSuppressor suppressor;
                suppressor.SetAmountInDecibels( amounts[ a ] );
                suppressor.SetBypassBelowHz( 0.0f );
                ASSERT_TRUE( PrepareSuppressor( suppressor ) );
                const AUDIO::CSpectralSuppressor* core = suppressor.GetSuppressor( 0 );
                ASSERT_TRUE( GUCEF_NULL != core );
                TSamples work = noise;
                const UInt32 hop = core->GetHopSize();
                const UInt32 bins = core->GetBinCount();
                Float64 sum = 0.0, sumSquares = 0.0;
                UInt64 count = 0, peaks = 0;
                for ( size_t at=0; at + hop <= work.size(); at += hop )
                {
                    ProcessRealtime( suppressor, &work[ at ], hop );
                    if ( at < 3 * Rate )
                        continue;
                    // the gains of the bins above the bypass frequency, in dB above the floor
                    for ( UInt32 b=1; b<bins; ++b )
                    {
                        const Float64 g = TVS::ToDecibels( core->GetGain( b ) );
                        sum += g;
                        sumSquares += g * g;
                        ++count;
                        if ( g > (Float64) amounts[ a ] + 6.0 )
                            ++peaks;
                    }
                }
                const Float64 mean = sum / (Float64) count;
                const Float64 spread = sqrt( sumSquares / (Float64) count - mean * mean );
                const Float64 peakShare = 100.0 * (Float64) peaks / (Float64) count;
                const Float64 levelIn = TVS::ToDecibels( TVS::Rms( noise, 3 * Rate, 8 * Rate ) );
                const Float64 levelOut = TVS::ToDecibels( TVS::Rms( work, 3 * Rate + Latency, 8 * Rate ) );
                const Float64 flatIn = SpectralFlatness( noise, 3 * Rate, 11 * Rate );
                const Float64 flatOut = SpectralFlatness( work, 3 * Rate + Latency, 11 * Rate + Latency );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioNoiseSuppressor: " << ( kinds[ k ] ? "pink" : "white" ) << " noise alone, amount " << amounts[ a ] << " dB: lowered by " << levelIn - levelOut <<
                           " dB, gain mean " << mean << " dB, spread " << spread << " dB, " << peakShare << " % of the gains 6 dB above the floor, flatness " << flatIn << " in, " << flatOut << " out" );
                ASSERT_TRUE( levelIn - levelOut >= -(Float64) amounts[ a ] - 4.0 && levelIn - levelOut <= -(Float64) amounts[ a ] + 1.0 );
                ASSERT_TRUE( spread <= 2.5 );
                ASSERT_TRUE( peakShare <= 2.0 );
                ASSERT_TRUE( flatOut >= 0.75 * flatIn );
            }
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioNoiseSuppressor: Transients: a tone burst 20 dB over the noise is at full level within 16 ms, a click keeps its energy, a step up of the noise by 10 dB is lowered again within 2.5 s" )
    try
    {
        // a burst after 3 s of noise
        TSamples x = TVS::MakeWhiteNoise( 5 * Rate, 0.003, 41 );
        const TSamples tone = TVS::MakeTone( Rate, Rate, 600.0, 0.03 );
        TVS::MixInto( x, tone, 1.0, 3 * Rate );
        TSuppressor suppressor;
        ASSERT_TRUE( PrepareSuppressor( suppressor ) );
        const TSamples y = Suppress( suppressor, x, 80, Latency );
        Float64 reachedMs = 1.0e9;
        for ( size_t w=3 * Rate; w<4 * Rate; w += Rate / 1000 )
        {
            if ( TVS::ToDecibels( TVS::Rms( y, w, Rate / 500 ) ) >= TVS::ToDecibels( 0.03 ) - 3.0 )
            {
                reachedMs = (Float64) ( w - 3 * Rate ) * 1000.0 / Rate;
                break;
            }
        }

        // a click of 2 ms in noise
        TSamples c = TVS::MakeWhiteNoise( 4 * Rate, 0.003, 43 );
        for ( UInt32 i=0; i<32; ++i )
            c[ 3 * Rate + i ] += (Float32) ( 0.5 * sin( Pi * (Float64) i / 32.0 ) );
        TSuppressor clickSuppressor;
        ASSERT_TRUE( PrepareSuppressor( clickSuppressor ) );
        const TSamples d = Suppress( clickSuppressor, c, 80, Latency );
        const Float64 clickIn = TVS::Rms( c, 3 * Rate - 64, 160 );
        const Float64 clickOut = TVS::Rms( d, 3 * Rate - 64, 160 );

        // the noise steps up by 10 dB at 4 s
        TSamples n = TVS::MakeWhiteNoise( 9 * Rate, 0.002, 47 );
        for ( size_t i=4 * Rate; i<n.size(); ++i )
            n[ i ] *= (Float32) TVS::FromDecibels( 10.0 );
        TSuppressor stepSuppressor;
        ASSERT_TRUE( PrepareSuppressor( stepSuppressor ) );
        const TSamples m = Suppress( stepSuppressor, n, 80, Latency );
        Float64 settledS = 1.0e9;
        for ( size_t w=4 * Rate; w + Rate / 10 < m.size(); w += Rate / 20 )
        {
            if ( TVS::ToDecibels( TVS::Rms( m, w, Rate / 10 ) ) <= TVS::ToDecibels( TVS::Rms( n, w, Rate / 10 ) ) - 12.0 )
            {
                settledS = (Float64) ( w - 4 * Rate ) / Rate;
                break;
            }
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioNoiseSuppressor: transients: a burst 20 dB over the noise at full level (within 3 dB) after " << reachedMs << " ms; a click " <<
                   TVS::ToDecibels( clickOut / clickIn ) << " dB; a step up of the noise by 10 dB lowered by 12 dB again after " << settledS << " s" );
        ASSERT_TRUE( reachedMs <= 16.0 );
        ASSERT_TRUE( fabs( TVS::ToDecibels( clickOut / clickIn ) ) <= 3.0 );
        ASSERT_TRUE( settledS <= 2.5 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformRobustnessTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioNoiseSuppressor: Block size invariance (blocks of 1, 7, 33, 100, 1024 and random) bit for bit, at 16 and 48 kHz, and each channel on its own" )
    try
    {
        const UInt32 rates[] = { 16000, 48000 };
        for ( UInt32 r=0; r<2; ++r )
        {
            TVS::SSpeech speech;
            TVS::SSpeechSpec spec;
            spec.sampleRate = rates[ r ];
            spec.seconds = 4.0;
            spec.seed = 9;
            spec.leadingSilenceSeconds = 1.0;
            TVS::MakeSpeech( spec, speech );
            TSamples input = speech.samples;
            TVS::MixInto( input, TVS::MakePinkNoise( input.size(), 0.01, 3 ) );

            TSuppressor reference;
            ASSERT_TRUE( PrepareSuppressor( reference, rates[ r ] ) );
            TSamples expected = input;
            Run( reference, expected, 1, 160 );

            const UInt32 blocks[] = { 1, 7, 33, 100, 1024 };
            for ( UInt32 b=0; b<5; ++b )
            {
                TSuppressor suppressor;
                ASSERT_TRUE( PrepareSuppressor( suppressor, rates[ r ] ) );
                TSamples out = input;
                Run( suppressor, out, 1, blocks[ b ] );
                ASSERT_TRUE( IsSame( expected, out ) );
            }
            TSuppressor random;
            ASSERT_TRUE( PrepareSuppressor( random, rates[ r ] ) );
            TSamples out = input;
            RunInRandomBlocks( random, out, 1, 700, 5 );
            ASSERT_TRUE( IsSame( expected, out ) );

            // stereo: the same signal in both channels gives the mono result in both
            TSamples stereo( input.size() * 2 );
            for ( size_t i=0; i<input.size(); ++i )
                stereo[ 2 * i ] = stereo[ 2 * i + 1 ] = input[ i ];
            TSuppressor two;
            ASSERT_TRUE( PrepareSuppressor( two, rates[ r ], 2 ) );
            Run( two, stereo, 2, 333 );
            bool same = true;
            for ( size_t i=0; i<input.size() && same; ++i )
                same = ( stereo[ 2 * i ] == expected[ i ] && stereo[ 2 * i + 1 ] == expected[ i ] );
            ASSERT_TRUE( same );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioNoiseSuppressor: Silence passes as silence without a transform; denormal, not-a-number and infinite input give finite output without denormals" )
    try
    {
        TSuppressor suppressor;
        ASSERT_TRUE( PrepareSuppressor( suppressor ) );
        TSamples silence( 2 * Rate, 0.0f );
        Run( suppressor, silence, 1, 160 );
        ASSERT_TRUE( PeakOf( silence ) == 0.0f );
        ASSERT_TRUE( 0 == suppressor.GetTransformedFrameCount() && suppressor.GetBypassedFrameCount() > 0 );

        // speech, then denormal values, then speech with a few not-a-number and infinite samples
        TVS::SSpeech speech;
        MakeTalk( speech, 4.0, 13, -26.0 );
        TSamples x = speech.samples;
        TVS::MixInto( x, TVS::MakeWhiteNoise( x.size(), 0.002, 9 ) );
        for ( size_t i=Rate; i<2 * Rate; ++i )
            x[ i ] = ( i & 1 ) ? 1.0e-39f : -1.0e-40f;
        const Float32 zero = 0.0f;
        x[ 3 * Rate ] = zero / zero;
        x[ 3 * Rate + 100 ] = 1.0f / zero;
        x[ 3 * Rate + 200 ] = -1.0f / zero;
        Run( suppressor, x, 1, 160 );
        ASSERT_TRUE( AllFiniteAndBelow( x, 10.0f ) );
        ASSERT_TRUE( 0 == CountSubnormal( x ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioNoiseSuppressor: Switched off in its slot it does not run and the audio passes bit for bit; switched on it is heard after the crossfade, delayed" )
    try
    {
        const AUDIO::CAudioFormat format( Rate, 1, AUDIO::SAMPLETYPE_FLOAT32 );
        AUDIO::CAudioEffectChain chain;
        ASSERT_TRUE( chain.Prepare( 2, format, 512 ) );
        TSuppressor suppressor;
        ASSERT_TRUE( PrepareSuppressor( suppressor, Rate, 1, 512 ) );
        ASSERT_TRUE( chain.SetSlotEffect( 0, &suppressor ) );
        chain.SetEnabled( 0, false );

        // a slot starts on: switched off, it fades out over 10 ms, from then on it is not run
        TSamples input = TVS::MakeWhiteNoise( 2 * Rate, 0.02, 3 );
        TSamples fade = input;
        for ( size_t at=0; at<Rate / 10; at += 160 )
            chain.Process( &fade[ at ], 160 );
        const UInt64 transformed = suppressor.GetTransformedFrameCount();
        const UInt64 bypassed = suppressor.GetBypassedFrameCount();
        TSamples out = input;
        for ( size_t at=0; at<out.size(); at += 160 )
            chain.Process( &out[ at ], 160 );
        ASSERT_TRUE( IsSame( input, out ) );
        ASSERT_TRUE( transformed == suppressor.GetTransformedFrameCount() && bypassed == suppressor.GetBypassedFrameCount() );

        chain.SetEnabled( 0, true );
        out = input;
        for ( size_t at=0; at<out.size(); at += 160 )
            chain.Process( &out[ at ], 160 );
        ASSERT_TRUE( suppressor.GetTransformedFrameCount() > 0 );
        // after the crossfade the output is the suppressed noise
        ASSERT_TRUE( TVS::ToDecibels( TVS::Rms( out, (size_t) ( 1.8 * Rate ), Rate / 10 ) ) < TVS::ToDecibels( 0.02 ) - 8.0 );
        chain.SetSlotEffect( 0, GUCEF_NULL );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioNoiseSuppressor: Real time safety: processing with parameter changes and resets allocates, locks and blocks nowhere (RealtimeSanitizer builds)" )
    if ( !IsRtsanBuild )
    {
        GUCEF_TESTFW_SKIP( "not a RealtimeSanitizer build (-fsanitize=realtime)" );
    }
    try
    {
        TSuppressor suppressor;
        ASSERT_TRUE( PrepareSuppressor( suppressor, Rate, 2, 512 ) );
        TSamples audio = MakeNoise( 3 * Rate, 2, 5, 0.1f );
        UInt32 at = 0, round = 0;
        while ( at < 3 * Rate )
        {
            const UInt32 frames = 1 + ( round * 37 ) % 400;
            if ( at + frames > 3 * Rate )
                break;
            suppressor.SetAmountInDecibels( -10.0f - (Float32) ( round % 20 ) );
            if ( 0 == round % 50 )
                ResetRealtime( suppressor );
            ProcessRealtime( suppressor, &audio[ (size_t) at * 2 ], frames );
            at += frames;
            ++round;
        }
        ASSERT_TRUE( suppressor.GetTransformedFrameCount() > 0 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------*/

void
PerformAudioNoiseSuppressorTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioNoiseSuppressor TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.NoiseSuppressor" );

    PerformParameterTests();
    PerformQualityTests();
    PerformRobustnessTests();
}

/*-------------------------------------------------------------------------*/
