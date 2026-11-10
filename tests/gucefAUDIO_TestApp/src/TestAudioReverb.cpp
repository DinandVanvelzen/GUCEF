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

#ifndef GUCEF_AUDIO_CREVERBEFFECT_H
#include "gucefAUDIO_CReverbEffect.h"
#define GUCEF_AUDIO_CREVERBEFFECT_H
#endif /* GUCEF_AUDIO_CREVERBEFFECT_H ? */

#ifndef GUCEF_AUDIO_CAUDIOEFFECTREGISTRY_H
#include "gucefAUDIO_CAudioEffectRegistry.h"
#define GUCEF_AUDIO_CAUDIOEFFECTREGISTRY_H
#endif /* GUCEF_AUDIO_CAUDIOEFFECTREGISTRY_H ? */

#ifndef GUCEF_AUDIO_CAUDIOGLOBAL_H
#include "gucefAUDIO_CAudioGlobal.h"
#define GUCEF_AUDIO_CAUDIOGLOBAL_H
#endif /* GUCEF_AUDIO_CAUDIOGLOBAL_H ? */

#ifndef GUCEF_AUDIO_CAUDIOBUSLAYOUT_H
#include "gucefAUDIO_CAudioBusLayout.h"
#define GUCEF_AUDIO_CAUDIOBUSLAYOUT_H
#endif /* GUCEF_AUDIO_CAUDIOBUSLAYOUT_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_AUDIO_CSVFFILTER_H
#include "gucefAUDIO_CSvfFilter.h"
#define GUCEF_AUDIO_CSVFFILTER_H
#endif /* GUCEF_AUDIO_CSVFFILTER_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAudioEffectSupport.h"
#include "TestAudioReverb.h"

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

typedef AUDIO::CReverbEffect CReverb;
typedef GUCEF::vector< Float64 > TSignal;

const UInt32 Rate = 48000;

/**
 *  Tolerances of the measured decay time: those of the design (10.2.3) down to a high frequency ratio of 0.35. Below that the
 *  one pole absorption filter is too gentle to hold the decay time at 500 Hz while the decay time at 4 kHz is a quarter of it,
 *  and the allowed deviations are a little wider
 */
const Float64 DecayTolerance = 0.10;
const Float64 RatioTolerance = 0.15;
const Float64 DecayToleranceLowRatio = 0.12;
const Float64 RatioToleranceLowRatio = 0.20;

CORE::CValueList
QualityOptions( bool low )
{
    CORE::CValueList options;
    options.Set( "quality", low ? "low" : "normal" );
    return options;
}

bool
PrepareReverb( CReverb& reverb, UInt32 channels, UInt32 rate = Rate, UInt32 maxBlock = 4096 )
{
    return reverb.Prepare( AUDIO::CAudioFormat( rate, channels, AUDIO::SAMPLETYPE_FLOAT32 ), maxBlock );
}

/** A unit impulse at 'frame' on the channels of the mask */
TSamples
MakeImpulse( UInt32 frames, UInt32 channels, UInt32 frame, UInt32 channelMask )
{
    TSamples audio( (size_t) frames * channels, 0.0f );
    for ( UInt32 c=0; c<channels; ++c )
        if ( ( channelMask >> c ) & 1 )
            audio[ (size_t) frame * channels + c ] = 1.0f;
    return audio;
}

/** The response to a unit impulse on all channels at frame 0, in blocks of 256 */
TSamples
ImpulseResponse( CReverb& reverb, UInt32 channels, UInt32 frames, UInt32 channelMask = 0xFFFFFFFF )
{
    TSamples audio = MakeImpulse( frames, channels, 0, channelMask );
    Run( reverb, audio, channels, 256 );
    return audio;
}

TSignal
ChannelOf( const TSamples& audio, UInt32 channels, UInt32 channel, size_t fromFrame = 0, size_t toFrame = 0 )
{
    const size_t frames = audio.size() / channels;
    if ( 0 == toFrame || toFrame > frames )
        toFrame = frames;
    TSignal signal;
    for ( size_t f=fromFrame; f<toFrame; ++f )
        signal.push_back( (Float64) audio[ f * channels + channel ] );
    return signal;
}

/** One third of an octave around 'centre': two band pass sections, 24 dB per octave */
TSignal
BandPass( const TSignal& input, Float64 centre, Float64 rate )
{
    AUDIO::CSvfFilter first;
    AUDIO::CSvfFilter second;
    first.Prepare( 1 );
    second.Prepare( 1 );
    first.SetResponse( AUDIO::FILTERRESPONSE_BANDPASS_PEAK0DB, rate, centre, 4.32 );
    second.SetResponse( AUDIO::FILTERRESPONSE_BANDPASS_PEAK0DB, rate, centre, 4.32 );
    TSignal output( input.size() );
    for ( size_t i=0; i<input.size(); ++i )
        output[ i ] = (Float64) second.Process( 0, first.Process( 0, (Float32) input[ i ] ) );
    return output;
}

/**
 *  Decay time (T60) from the energy decay curve: the backward integral of the squared impulse response after
 *  M. R. Schroeder, in decibels, with a straight line fitted by least squares between 'fromDb' and 'toDb', and the
 *  slope extrapolated to -60 dB. Negative when the curve does not span the range.
 */
Float64
DecayTimeOf( const TSignal& response, Float64 rate, Float64 fromDb = -5.0, Float64 toDb = -35.0 )
{
    const size_t n = response.size();
    TSignal energy( n );
    Float64 sum = 0.0;
    for ( size_t i=n; i-->0; )
    {
        sum += response[ i ] * response[ i ];
        energy[ i ] = sum;
    }
    if ( !( energy[ 0 ] > 0.0 ) )
        return -1.0;
    Float64 sx = 0.0, sy = 0.0, sxx = 0.0, sxy = 0.0;
    size_t count = 0;
    for ( size_t i=0; i<n; ++i )
    {
        const Float64 db = 10.0 * log10( energy[ i ] / energy[ 0 ] + 1.0e-300 );
        if ( db <= fromDb && db >= toDb )
        {
            const Float64 x = (Float64) i / rate;
            sx += x;
            sy += db;
            sxx += x * x;
            sxy += x * db;
            ++count;
        }
    }
    if ( count < 20 )
        return -1.0;
    const Float64 slope = ( (Float64) count * sxy - sx * sy ) / ( (Float64) count * sxx - sx * sx );
    return slope < 0.0 ? -60.0 / slope : -1.0;
}

/** Correlation coefficient of two channels over a range of frames */
Float64
CorrelationOf( const TSamples& audio, UInt32 channels, UInt32 a, UInt32 b, size_t fromFrame, size_t toFrame )
{
    Float64 sab = 0.0, saa = 0.0, sbb = 0.0;
    for ( size_t f=fromFrame; f<toFrame; ++f )
    {
        const Float64 x = (Float64) audio[ f * channels + a ];
        const Float64 y = (Float64) audio[ f * channels + b ];
        sab += x * y;
        saa += x * x;
        sbb += y * y;
    }
    return ( saa > 0.0 && sbb > 0.0 ) ? sab / sqrt( saa * sbb ) : 0.0;
}

/** Index of the first frame in which the channel holds more than 'threshold' */
size_t
FirstAbove( const TSamples& audio, UInt32 channels, UInt32 channel, Float32 threshold )
{
    const size_t frames = audio.size() / channels;
    for ( size_t f=0; f<frames; ++f )
        if ( fabsf( audio[ f * channels + channel ] ) > threshold )
            return f;
    return frames;
}

Float64
EnergyOf( const TSamples& audio )
{
    Float64 sum = 0.0;
    for ( size_t i=0; i<audio.size(); ++i )
        sum += (Float64) audio[ i ] * (Float64) audio[ i ];
    return sum;
}

/** All eleven parameters at random values within their ranges, published together */
void
RandomizeAll( CReverb& reverb, CXorShift& random )
{
    UInt32 ids[ CReverb::PARAMETER_COUNT ];
    Float32 values[ CReverb::PARAMETER_COUNT ];
    for ( UInt32 p=0; p<CReverb::PARAMETER_COUNT; ++p )
    {
        const AUDIO::CAudioEffectParameterInfo& info = *reverb.GetParameterInfo( p );
        ids[ p ] = p;
        values[ p ] = info.GetMinimum() + ( info.GetMaximum() - info.GetMinimum() ) * (Float32) random.NextUnit();
    }
    reverb.SetParameters( ids, values, CReverb::PARAMETER_COUNT );
}

/** Milliseconds for the fastest of a few runs of a function: a quiet measure on a machine that does other things as well */
template< class TWork >
Float64
BestTimeInMs( TWork& work, UInt32 runs )
{
    Float64 best = 1.0e30;
    for ( UInt32 r=0; r<runs; ++r )
    {
        const UInt64 start = MT::PrecisionTickCount();
        work();
        const Float64 ms = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start );
        if ( ms < best )
            best = ms;
    }
    return best;
}

/** The middle one of 'count' values (the list is sorted) */
Float64
MedianOf( Float64* values, UInt32 count )
{
    for ( UInt32 i=1; i<count; ++i )
        for ( UInt32 j=i; j>0 && values[ j ] < values[ j - 1 ]; --j )
        {
            const Float64 swap = values[ j ];
            values[ j ] = values[ j - 1 ];
            values[ j - 1 ] = swap;
        }
    return values[ count / 2 ];
}

/** Runs a reverb over a buffer of noise in blocks of 256 frames, for BestTimeInMs() */
class CProcessNoise
{
    public:

    CProcessNoise( CReverb& reverb, UInt32 frames ) : m_reverb( reverb ), m_input( MakeNoise( frames, 2, 31, 0.3f ) ), m_work( m_input ) {}

    void operator()( void )
    {
        m_work = m_input;
        Run( m_reverb, m_work, 2, 256 );
    }

    private:

    CReverb& m_reverb;
    TSamples m_input;
    TSamples m_work;
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformIdentityTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioReverb: Type name, parameter table, defaults and ranges, quality option, no latency" )
    try
    {
        CReverb reverb;
        ASSERT_TRUE( reverb.GetEffectTypeName() == "reverb" && CReverb::PARAMETER_COUNT == reverb.GetParameterCount() );
        ASSERT_TRUE( CReverb::QUALITY_NORMAL == reverb.GetQuality() && 8 == reverb.GetLineCount() );
        ASSERT_TRUE( PrepareReverb( reverb, 2 ) );
        ASSERT_TRUE( 0 == reverb.GetLatencyInFrames() );

        const char* names[ CReverb::PARAMETER_COUNT ] = { "preDelay", "size", "decayTime", "hfRatio", "diffusion", "earlyLevel", "lateLevel", "lowCut", "highCut", "width", "modulationDepth" };
        const Float32 defaults[ CReverb::PARAMETER_COUNT ] = { 20.0f, 0.6f, 1.8f, 0.5f, 0.6f, -6.0f, -3.0f, 80.0f, 9000.0f, 1.0f, 0.3f };
        const Float32 minimums[ CReverb::PARAMETER_COUNT ] = { 0.0f, 0.1f, 0.1f, 0.1f, 0.0f, -60.0f, -60.0f, 20.0f, 1000.0f, 0.0f, 0.0f };
        const Float32 maximums[ CReverb::PARAMETER_COUNT ] = { 500.0f, 1.0f, 20.0f, 1.0f, 0.75f, 0.0f, 0.0f, 1000.0f, 20000.0f, 1.0f, 1.0f };
        for ( UInt32 p=0; p<CReverb::PARAMETER_COUNT; ++p )
        {
            const AUDIO::CAudioEffectParameterInfo* info = reverb.GetParameterInfo( p );
            ASSERT_TRUE( GUCEF_NULL != info && info->GetName() == names[ p ] );
            ASSERT_TRUE( defaults[ p ] == info->GetDefaultValue() && defaults[ p ] == reverb.GetParameter( p ) );
            ASSERT_TRUE( minimums[ p ] == info->GetMinimum() && maximums[ p ] == info->GetMaximum() );
            UInt32 id = 99;
            ASSERT_TRUE( reverb.TryGetParameterId( names[ p ], id ) && id == p );

            // values outside the range are brought into it
            reverb.SetParameter( p, maximums[ p ] + 100.0f );
            ASSERT_TRUE( maximums[ p ] == reverb.GetParameter( p ) );
            reverb.SetParameter( p, minimums[ p ] - 100.0f );
            ASSERT_TRUE( minimums[ p ] == reverb.GetParameter( p ) );
        }

        // the typed setters reach the same parameters
        reverb.SetPreDelayInMs( 33.0f );
        reverb.SetSize( 0.8f );
        reverb.SetDecayTimeInSeconds( 4.0f );
        reverb.SetHfRatio( 0.7f );
        reverb.SetDiffusion( 0.4f );
        reverb.SetEarlyLevelInDecibels( -12.0f );
        reverb.SetLateLevelInDecibels( -9.0f );
        reverb.SetLowCutInHertz( 120.0f );
        reverb.SetHighCutInHertz( 6000.0f );
        reverb.SetWidth( 0.25f );
        reverb.SetModulationDepth( 0.9f );
        const Float32 set[ CReverb::PARAMETER_COUNT ] = { 33.0f, 0.8f, 4.0f, 0.7f, 0.4f, -12.0f, -9.0f, 120.0f, 6000.0f, 0.25f, 0.9f };
        for ( UInt32 p=0; p<CReverb::PARAMETER_COUNT; ++p )
            ASSERT_TRUE( set[ p ] == reverb.GetParameter( p ) );

        // quality
        CReverb low( QualityOptions( true ) );
        ASSERT_TRUE( CReverb::QUALITY_LOW == low.GetQuality() && 4 == low.GetLineCount() );
        CORE::CValueList shouting;
        shouting.Set( "quality", "LOW" );
        ASSERT_TRUE( CReverb::QUALITY_LOW == CReverb( shouting ).GetQuality() );
        CORE::CValueList unknown;
        unknown.Set( "quality", "ultra" );
        ASSERT_TRUE( CReverb::QUALITY_NORMAL == CReverb( unknown ).GetQuality() );
        ASSERT_TRUE( CReverb::QUALITY_NORMAL == CReverb( QualityOptions( false ) ).GetQuality() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformDecayTimeTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioReverb: Measured T60 (Schroeder backward integral, one third octave band) is the decay time at 500 Hz and decay time x high frequency ratio at 4 kHz, for both qualities, late reverberation alone and with the early reflections" )
    try
    {
        struct SCase { Float32 size; Float32 decay; Float32 hfRatio; };
        const SCase cases[] =
        {
            { 0.6f, 1.0f, 1.0f }, { 0.6f, 1.0f, 0.5f }, { 0.6f, 1.0f, 0.25f },
            { 0.6f, 1.8f, 1.0f }, { 0.6f, 1.8f, 0.5f }, { 0.6f, 1.8f, 0.25f },
            { 0.6f, 5.0f, 1.0f }, { 0.6f, 5.0f, 0.5f }, { 0.6f, 5.0f, 0.25f },
            { 1.0f, 1.8f, 0.5f }, { 0.2f, 1.8f, 0.5f }, { 1.0f, 3.0f, 0.35f }, { 0.3f, 2.5f, 0.7f }, { 0.1f, 1.5f, 0.6f }
        };
        const UInt32 caseCount = sizeof( cases ) / sizeof( cases[ 0 ] );

        for ( UInt32 variant=0; variant<3; ++variant )
        {
            // normal quality late only, low quality (no early reflections), normal quality with the early reflections
            const UInt32 quality = 1 == variant ? 1 : 0;
            const bool withEarly = 2 == variant;
            Float64 worstLow = 0.0, worstHigh = 0.0, worstRatio = 0.0;
            UInt32 measured = 0;
            for ( UInt32 i=0; i<caseCount; ++i )
            {
                // the early reflections are not shaped by the decay time: with them on, a decay time at 4 kHz of a few tenths of a second is dominated by them
                if ( withEarly && cases[ i ].decay * cases[ i ].hfRatio < 0.6f )
                    continue;
                CReverb reverb( QualityOptions( 1 == quality ) );
                reverb.SetSize( cases[ i ].size );
                reverb.SetDecayTimeInSeconds( cases[ i ].decay );
                reverb.SetHfRatio( cases[ i ].hfRatio );
                if ( !withEarly )
                    reverb.SetEarlyLevelInDecibels( -60.0f );
                ASSERT_TRUE( PrepareReverb( reverb, 2 ) );
                const UInt32 frames = (UInt32) ( ( 1.5 * cases[ i ].decay + 0.5 ) * Rate );
                const TSamples response = ImpulseResponse( reverb, 2, frames );
                const TSignal left = ChannelOf( response, 2, 0 );

                const Float64 low = DecayTimeOf( BandPass( left, 500.0, Rate ), Rate );
                const Float64 high = DecayTimeOf( BandPass( left, 4000.0, Rate ), Rate );
                ASSERT_TRUE( low > 0.0 && high > 0.0 );
                const Float64 lowError = low / cases[ i ].decay - 1.0;
                const Float64 highError = high / ( cases[ i ].decay * cases[ i ].hfRatio ) - 1.0;
                const Float64 ratioError = ( high / low ) / cases[ i ].hfRatio - 1.0;
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioReverb: " << ( 0 == quality ? "normal" : "low" ) << ( withEarly ? " with early reflections" : " late only" ) << ", size " << cases[ i ].size << ", decay time " << cases[ i ].decay <<
                                                  " s, hf ratio " << cases[ i ].hfRatio << ": T60 at 500 Hz " << low << " s (" << 100.0 * lowError << " %), at 4 kHz " << high <<
                                                  " s (" << 100.0 * highError << " %), ratio error " << 100.0 * ratioError << " %" );
                if ( fabs( lowError ) > worstLow ) worstLow = fabs( lowError );
                if ( fabs( highError ) > worstHigh ) worstHigh = fabs( highError );
                if ( fabs( ratioError ) > worstRatio ) worstRatio = fabs( ratioError );
                ++measured;
                const bool lowRatio = cases[ i ].hfRatio < 0.35f;
                ASSERT_TRUE( fabs( lowError ) <= ( lowRatio ? DecayToleranceLowRatio : DecayTolerance ) );
                ASSERT_TRUE( fabs( highError ) <= ( lowRatio ? DecayToleranceLowRatio : DecayTolerance ) );
                ASSERT_TRUE( fabs( ratioError ) <= ( lowRatio ? RatioToleranceLowRatio : RatioTolerance ) );
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioReverb: " << ( 0 == quality ? "normal" : "low" ) << ( withEarly ? " with early reflections" : " late only" ) << ", worst deviation over " << measured << " settings: 500 Hz " <<
                                              100.0 * worstLow << " %, 4 kHz " << 100.0 * worstHigh << " %, ratio " << 100.0 * worstRatio << " %" );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioReverb: The decay time holds at other sample rates and without modulation; the decay is the same on the right channel" )
    try
    {
        const UInt32 rates[] = { 22050, 44100, 96000 };
        for ( UInt32 r=0; r<3; ++r )
        {
            CReverb reverb;
            reverb.SetDecayTimeInSeconds( 2.0f );
            reverb.SetHfRatio( 0.5f );
            reverb.SetModulationDepth( 0.0f );
            ASSERT_TRUE( PrepareReverb( reverb, 2, rates[ r ] ) );
            const TSamples response = ImpulseResponse( reverb, 2, 3 * rates[ r ] + rates[ r ] / 2 );
            const Float64 sampleRate = (Float64) rates[ r ];
            for ( UInt32 channel=0; channel<2; ++channel )
            {
                const TSignal signal = ChannelOf( response, 2, channel );
                const Float64 low = DecayTimeOf( BandPass( signal, 500.0, sampleRate ), sampleRate );
                const Float64 high = DecayTimeOf( BandPass( signal, 4000.0, sampleRate ), sampleRate );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioReverb: " << rates[ r ] << " Hz, channel " << channel << ": T60 at 500 Hz " << low << " s, at 4 kHz " << high << " s (expected 2 and 1)" );
                ASSERT_TRUE( fabs( low / 2.0 - 1.0 ) <= DecayTolerance );
                ASSERT_TRUE( fabs( high / 1.0 - 1.0 ) <= DecayTolerance );
            }
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioReverb: A change of the decay time while running glides to the new decay time" )
    try
    {
        CReverb reverb;
        reverb.SetDecayTimeInSeconds( 4.0f );
        reverb.SetHfRatio( 1.0f );
        ASSERT_TRUE( PrepareReverb( reverb, 2 ) );
        TSamples audio = MakeImpulse( 20 * Rate, 2, 0, 3 );
        // one second at 4 s, then 1 s
        TSamples first( audio.begin(), audio.begin() + Rate * 2 );
        Run( reverb, first, 2, 256 );
        reverb.SetDecayTimeInSeconds( 1.0f );
        TSamples second( audio.begin() + Rate * 2, audio.begin() + 7 * Rate * 2 );
        Run( reverb, second, 2, 256 );
        // the part after the change decays with the new decay time: 60 dB in 1 s, 10 dB in 1/6 s
        const Float64 a = RmsOfChannel( second, 2, 0, 2 * Rate, 2 * Rate + 4800 );
        const Float64 b = RmsOfChannel( second, 2, 0, 3 * Rate, 3 * Rate + 4800 );
        ASSERT_TRUE( a > 0.0 && b > 0.0 );
        const Float64 dropDb = DecibelsOf( b / a );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioReverb: after a change from 4 s to 1 s the level drops " << dropDb << " dB in a second (expected about -60)" );
        ASSERT_TRUE( dropDb < -45.0 && dropDb > -75.0 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformTimingTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioReverb: The pre-delay moves the response by exactly that many frames, early reflections alone and late reverberation alone, both qualities" )
    try
    {
        const Float32 preDelays[] = { 0.0f, 1.0f, 7.3f, 20.0f, 100.0f, 250.0f, 500.0f };
        for ( UInt32 mode=0; mode<3; ++mode )
        {
            // mode 0: early reflections alone, 1: late reverberation alone, 2: late reverberation alone in low quality
            size_t onsetAtZero[ 2 ] = { 0, 0 };
            for ( UInt32 i=0; i<7; ++i )
            {
                CReverb reverb( QualityOptions( 2 == mode ) );
                reverb.SetPreDelayInMs( preDelays[ i ] );
                reverb.SetEarlyLevelInDecibels( 0 == mode ? 0.0f : -60.0f );
                reverb.SetLateLevelInDecibels( 0 == mode ? -60.0f : 0.0f );
                reverb.SetLowCutInHertz( 20.0f );
                reverb.SetHighCutInHertz( 20000.0f );
                ASSERT_TRUE( PrepareReverb( reverb, 2 ) );
                const TSamples response = ImpulseResponse( reverb, 2, 40000 );
                const size_t expected = (size_t) floor( (Float64) preDelays[ i ] * 48.0 + 0.5 );
                for ( UInt32 channel=0; channel<2; ++channel )
                {
                    const size_t onset = FirstAbove( response, 2, channel, 1.0e-4f );
                    ASSERT_TRUE( onset < 40000 );
                    if ( 0 == i )
                        onsetAtZero[ channel ] = onset;
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioReverb: mode " << mode << ", pre-delay " << preDelays[ i ] << " ms, channel " << channel << ": first sound at frame " << onset << ", " << ( onset - onsetAtZero[ channel ] ) << " frames after the one without, expected " << expected );
                    ASSERT_TRUE( onset - onsetAtZero[ channel ] == expected );
                }
            }
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioReverb: Early reflections come at the 12 tap times multiplied by the size, 6 per side, with decaying gains of unit energy per side" )
    try
    {
        const Float32 tapMs[ 12 ] = { 5.0f, 7.9f, 11.6f, 15.3f, 20.2f, 25.7f, 32.1f, 38.9f, 47.3f, 57.1f, 67.9f, 80.0f };
        const Float32 tapSign[ 12 ] = { 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f };
        const Float32 sizes[] = { 0.6f, 1.0f, 0.1f };
        for ( UInt32 s=0; s<3; ++s )
        {
            CReverb reverb;
            reverb.SetSize( sizes[ s ] );
            reverb.SetPreDelayInMs( 20.0f );
            reverb.SetEarlyLevelInDecibels( 0.0f );
            reverb.SetLateLevelInDecibels( -60.0f );
            reverb.SetLowCutInHertz( 20.0f );
            reverb.SetHighCutInHertz( 20000.0f );
            ASSERT_TRUE( PrepareReverb( reverb, 2 ) );
            TSamples response = MakeImpulse( 12000, 2, 100, 3 );
            Run( reverb, response, 2, 256 );

            for ( UInt32 side=0; side<2; ++side )
            {
                Float64 energy = 0.0;
                for ( UInt32 k=side; k<12; k+=2 )
                    energy += pow( 0.85, 2.0 * k );
                size_t nonZero = 0;
                for ( UInt32 k=side; k<12; k+=2 )
                {
                    const size_t frame = 100 + 960 + (size_t) floor( (Float64) tapMs[ k ] * 48.0 * (Float64) sizes[ s ] + 0.5 );
                    const Float32 expected = tapSign[ k ] * (Float32) ( pow( 0.85, (Float64) k ) / sqrt( energy ) );
                    ASSERT_TRUE( Near( response[ frame * 2 + side ], expected, 2.0e-5 ) );
                    ++nonZero;
                }
                // and nothing else
                size_t others = 0;
                for ( size_t f=0; f<12000; ++f )
                    if ( fabsf( response[ f * 2 + side ] ) > 1.0e-9f )
                        ++others;
                ASSERT_TRUE( others == nonZero );
            }
        }

        // low quality has no early reflections: the level has no effect on it
        CReverb low( QualityOptions( true ) );
        low.SetEarlyLevelInDecibels( 0.0f );
        CReverb lowQuiet( QualityOptions( true ) );
        lowQuiet.SetEarlyLevelInDecibels( -60.0f );
        ASSERT_TRUE( PrepareReverb( low, 2 ) && PrepareReverb( lowQuiet, 2 ) );
        ASSERT_TRUE( IsSame( ImpulseResponse( low, 2, 20000 ), ImpulseResponse( lowQuiet, 2, 20000 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioReverb: Silence in is silence out, the levels at -60 dB are silence, and the level setting scales the response" )
    try
    {
        CReverb reverb;
        ASSERT_TRUE( PrepareReverb( reverb, 2 ) );
        TSamples silence( 20000 * 2, 0.0f );
        Run( reverb, silence, 2, 256 );
        for ( size_t i=0; i<silence.size(); ++i )
            ASSERT_TRUE( 0.0f == silence[ i ] );

        CReverb off;
        off.SetEarlyLevelInDecibels( -60.0f );
        off.SetLateLevelInDecibels( -60.0f );
        ASSERT_TRUE( PrepareReverb( off, 2 ) );
        const TSamples response = ImpulseResponse( off, 2, 20000 );
        for ( size_t i=0; i<response.size(); ++i )
            ASSERT_TRUE( 0.0f == response[ i ] );

        // late level -3 against -9 dB: 6 dB in energy
        CReverb loud;
        loud.SetEarlyLevelInDecibels( -60.0f );
        loud.SetLateLevelInDecibels( -3.0f );
        CReverb quiet;
        quiet.SetEarlyLevelInDecibels( -60.0f );
        quiet.SetLateLevelInDecibels( -9.0f );
        ASSERT_TRUE( PrepareReverb( loud, 2 ) && PrepareReverb( quiet, 2 ) );
        const Float64 difference = 10.0 * log10( EnergyOf( ImpulseResponse( loud, 2, 4 * Rate ) ) / EnergyOf( ImpulseResponse( quiet, 2, 4 * Rate ) ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioReverb: late level 6 dB higher gives " << difference << " dB more energy" );
        ASSERT_TRUE( Near( difference, 6.0, 0.01 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformChannelTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioReverb: The channels of the tail are decorrelated, in stereo and in 5.1, whatever channels the input is on" )
    try
    {
        for ( UInt32 quality=0; quality<2; ++quality )
        {
            const Float64 limit = 0 == quality ? 0.3 : 0.3;
            for ( UInt32 layout=0; layout<2; ++layout )
            {
                const UInt32 channels = 0 == layout ? 2 : 6;
                const UInt32 masks[] = { 0xFFFFFFFF, 1, 2 };
                for ( UInt32 m=0; m<3; ++m )
                {
                    CReverb reverb( QualityOptions( 1 == quality ) );
                    ASSERT_TRUE( PrepareReverb( reverb, channels ) );
                    const TSamples response = ImpulseResponse( reverb, channels, 3 * Rate, masks[ m ] );
                    Float64 worst = 0.0;
                    for ( UInt32 a=0; a<channels; ++a )
                        for ( UInt32 b=a+1; b<channels; ++b )
                        {
                            if ( 6 == channels && ( 3 == a || 3 == b ) )
                                continue;                                   // the low frequency effects channel is not reverberated
                            const Float64 correlation = fabs( CorrelationOf( response, channels, a, b, Rate / 4, 3 * Rate ) );
                            if ( correlation > worst )
                                worst = correlation;
                        }
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioReverb: " << ( 0 == quality ? "normal" : "low" ) << " quality, " << channels << " channels, input mask " << masks[ m ] << ": largest correlation between channels of the tail " << worst );
                    // four lines have four orthogonal output rows: a fifth channel and further ones cannot be uncorrelated with them
                    ASSERT_TRUE( worst < ( 1 == quality && channels > 4 ? 0.75 : limit ) );
                }
            }
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioReverb: Width 0 gives the same signal on all channels with the mono sum of width 1, width 1 decorrelated channels, width 0.5 in between" )
    try
    {
        const Float32 widths[] = { 0.0f, 0.5f, 1.0f };
        TSamples responses[ 3 ];
        for ( UInt32 w=0; w<3; ++w )
        {
            CReverb reverb;
            reverb.SetWidth( widths[ w ] );
            ASSERT_TRUE( PrepareReverb( reverb, 2 ) );
            responses[ w ] = ImpulseResponse( reverb, 2, 2 * Rate );
        }
        // width 0: identical channels, bit for bit
        for ( size_t f=0; f<2 * Rate; ++f )
            ASSERT_TRUE( responses[ 0 ][ f * 2 ] == responses[ 0 ][ f * 2 + 1 ] );
        // and the sum of the two channels is that of width 1
        Float32 worst = 0.0f;
        for ( size_t f=0; f<2 * Rate; ++f )
        {
            const Float32 sum0 = responses[ 0 ][ f * 2 ] + responses[ 0 ][ f * 2 + 1 ];
            const Float32 sum1 = responses[ 2 ][ f * 2 ] + responses[ 2 ][ f * 2 + 1 ];
            if ( fabsf( sum0 - sum1 ) > worst )
                worst = fabsf( sum0 - sum1 );
        }
        ASSERT_TRUE( worst < 1.0e-5f );

        const Float64 wide = CorrelationOf( responses[ 2 ], 2, 0, 1, Rate / 4, 2 * Rate );
        const Float64 half = CorrelationOf( responses[ 1 ], 2, 0, 1, Rate / 4, 2 * Rate );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioReverb: correlation of the channels at width 1: " << wide << ", at width 0.5: " << half << " (0.6 for uncorrelated channels of equal level), mono sum difference " << worst );
        ASSERT_TRUE( fabs( wide ) < 0.3 );
        ASSERT_TRUE( half > 0.4 && half < 0.8 );

        // the same in 5.1: width 0 gives the same signal on every channel except the low frequency effects channel
        CReverb surround;
        surround.SetWidth( 0.0f );
        ASSERT_TRUE( PrepareReverb( surround, 6 ) );
        const TSamples sixChannels = ImpulseResponse( surround, 6, Rate, 0x37 );
        for ( size_t f=0; f<Rate; ++f )
            for ( UInt32 c=1; c<6; ++c )
                if ( 3 != c )
                    ASSERT_TRUE( sixChannels[ f * 6 + c ] == sixChannels[ f * 6 ] );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioReverb: The low frequency effects channel passes through unchanged, mono, stereo, 5.1 and 7.1 work, and a channel on one side stays on its side early" )
    try
    {
        const UInt32 counts[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
        for ( UInt32 n=0; n<8; ++n )
        {
            const UInt32 channels = counts[ n ];
            CReverb reverb;
            ASSERT_TRUE( PrepareReverb( reverb, channels ) );
            const TSamples input = MakeNoise( 20000, channels, 11 + n, 0.4f );
            TSamples output( input );
            Run( reverb, output, channels, 300 );
            ASSERT_TRUE( AllFiniteAndBelow( output, 10.0f ) );
            const AUDIO::CAudioFormat format( Rate, channels, AUDIO::SAMPLETYPE_FLOAT32 );
            const bool hasLfe = ( format.GetChannelLayout() & AUDIO::AUDIOSPEAKER_LOW_FREQUENCY ) != 0;
            const UInt32 lfeIndex = hasLfe ? AUDIO::CAudioFormat::GetChannelCountOfLayout( format.GetChannelLayout() & ( AUDIO::AUDIOSPEAKER_LOW_FREQUENCY - 1 ) ) : 0;
            for ( UInt32 c=0; c<channels; ++c )
            {
                bool same = true;
                for ( size_t f=0; f<20000 && same; ++f )
                    same = output[ f * channels + c ] == input[ f * channels + c ];
                ASSERT_TRUE( same == ( hasLfe && c == lfeIndex ) );
                if ( !( hasLfe && c == lfeIndex ) )
                    ASSERT_TRUE( RmsOfChannel( output, channels, c, 10000, 20000 ) > 0.0 );
            }
        }

        // left side channels only reverberate to the left: a hard left impulse of 5.1 has no early sound on the right
        CReverb reverb;
        reverb.SetLateLevelInDecibels( -60.0f );
        ASSERT_TRUE( PrepareReverb( reverb, 6 ) );
        const TSamples response = ImpulseResponse( reverb, 6, 6000, 1 );
        ASSERT_TRUE( PeakOf( response, 0, 6000 * 6 ) > 0.0f );
        for ( size_t f=0; f<6000; ++f )
            ASSERT_TRUE( 0.0f == response[ f * 6 + 1 ] && 0.0f == response[ f * 6 + 5 ] );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformStabilityTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioReverb: At the longest decay time, largest size, full diffusion and modulation, loud noise stays bounded and the tail dies away after it, both qualities" )
    try
    {
        for ( UInt32 quality=0; quality<2; ++quality )
        {
            CReverb reverb( QualityOptions( 1 == quality ) );
            reverb.SetDecayTimeInSeconds( 20.0f );
            reverb.SetSize( 1.0f );
            reverb.SetDiffusion( 0.75f );
            reverb.SetHfRatio( 1.0f );
            reverb.SetModulationDepth( 1.0f );
            reverb.SetEarlyLevelInDecibels( 0.0f );
            reverb.SetLateLevelInDecibels( 0.0f );
            reverb.SetLowCutInHertz( 20.0f );
            reverb.SetHighCutInHertz( 20000.0f );
            ASSERT_TRUE( PrepareReverb( reverb, 2 ) );

            const UInt32 noiseSeconds = 0 == quality ? 20 : 10;
            const UInt32 silentSeconds = 0 == quality ? 40 : 20;
            Float32 peak = 0.0f;
            Float64 lastLoud = 0.0;
            for ( UInt32 s=0; s<noiseSeconds; ++s )
            {
                TSamples second = MakeNoise( Rate, 2, 100 + s, 0.9f );
                Run( reverb, second, 2, 256 );
                ASSERT_TRUE( AllFiniteAndBelow( second, 100.0f ) );
                const Float32 p = PeakOf( second );
                if ( p > peak )
                    peak = p;
                lastLoud = RmsOfChannel( second, 2, 0, 0, Rate );
            }
            Float64 lastQuiet = 0.0;
            Float32 silentPeak = 0.0f;
            Float64 rmsAfterTenSeconds = 0.0;
            for ( UInt32 s=0; s<silentSeconds; ++s )
            {
                TSamples second( (size_t) Rate * 2, 0.0f );
                Run( reverb, second, 2, 256 );
                ASSERT_TRUE( AllFiniteAndBelow( second, 100.0f ) );
                lastQuiet = RmsOfChannel( second, 2, 0, 0, Rate );
                if ( 9 == s )
                    rmsAfterTenSeconds = lastQuiet;
                const Float32 p = PeakOf( second );
                if ( 0 == s )
                    silentPeak = p;
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioReverb: " << ( 0 == quality ? "normal" : "low" ) << " quality at the extreme settings: peak " << peak << " with noise of 0.9 (rms " << lastLoud << "), first second of silence peak " << silentPeak <<
                                              ", rms after 10 s of silence " << rmsAfterTenSeconds << ", after " << silentSeconds << " s " << lastQuiet );
            ASSERT_TRUE( peak < 40.0f );
            ASSERT_TRUE( silentPeak < 2.0f * peak );
            // 20 s decay time: 10 s is -30 dB, the end -60 dB x silentSeconds / 20
            ASSERT_TRUE( DecibelsOf( rmsAfterTenSeconds / lastLoud ) < -20.0 );
            ASSERT_TRUE( DecibelsOf( ( lastQuiet + 1.0e-30 ) / lastLoud ) < -50.0 );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioReverb: Random parameters in every block (size included) keep the output finite and bounded for 30 s, both qualities, stereo and 5.1" )
    try
    {
        for ( UInt32 variant=0; variant<3; ++variant )
        {
            const UInt32 channels = 2 == variant ? 6 : 2;
            CReverb reverb( QualityOptions( 1 == variant ) );
            ASSERT_TRUE( PrepareReverb( reverb, channels ) );
            CXorShift random( 5000 + variant );
            const UInt32 blocks = 5625;                                     // 30 s of blocks of 256 frames
            Float32 peak = 0.0f;
            for ( UInt32 b=0; b<blocks; ++b )
            {
                RandomizeAll( reverb, random );
                TSamples block = MakeNoise( 256, channels, 7 + b, 0.7f );
                Run( reverb, block, channels, 256 );
                ASSERT_TRUE( AllFiniteAndBelow( block, 1000.0f ) );
                const Float32 p = PeakOf( block );
                if ( p > peak )
                    peak = p;
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioReverb: random parameters every block, variant " << variant << ": peak " << peak << " with noise of 0.7" );
            ASSERT_TRUE( peak < 50.0f );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioReverb: A change of the size makes no clicks: the largest step from sample to sample during the change stays near that of the steady output before and after it" )
    try
    {
        const Float32 changes[][2] = { { 0.6f, 1.0f }, { 1.0f, 0.1f }, { 0.2f, 0.9f }, { 0.5f, 0.55f }, { 0.1f, 0.6f } };
        for ( UInt32 quality=0; quality<2; ++quality )
            for ( UInt32 i=0; i<5; ++i )
            {
                CReverb reverb( QualityOptions( 1 == quality ) );
                reverb.SetSize( changes[ i ][ 0 ] );
                ASSERT_TRUE( PrepareReverb( reverb, 2 ) );
                TSamples warm = MakeNoise( 3 * Rate, 2, 41, 0.3f );
                Run( reverb, warm, 2, 256 );
                TSamples steady = MakeNoise( 2 * Rate, 2, 42, 0.3f );
                Run( reverb, steady, 2, 256 );
                const Float32 stepBefore = MaxStep( steady, 2, 0 );

                // the glide has 200 ms as time constant: the first two seconds hold the change, the last two the new steady state
                reverb.SetSize( changes[ i ][ 1 ] );
                TSamples after = MakeNoise( 6 * Rate, 2, 43, 0.3f );
                Run( reverb, after, 2, 256 );
                const Float32 stepDuring = MaxStep( after, 2, 0, 0, 2 * Rate );
                const Float32 stepAfter = MaxStep( after, 2, 0, 4 * Rate, 6 * Rate );
                const Float32 reference = stepBefore > stepAfter ? stepBefore : stepAfter;
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioReverb: " << ( 0 == quality ? "normal" : "low" ) << " size " << changes[ i ][ 0 ] << " to " << changes[ i ][ 1 ] << ": largest step " << stepDuring << " in the two seconds of the change, " <<
                                                  stepBefore << " steady before, " << stepAfter << " steady after" );
                ASSERT_TRUE( stepDuring <= 1.5f * reference );
                ASSERT_TRUE( AllFiniteAndBelow( after, 10.0f ) );
            }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioReverb: Changes of the pre-delay, the decay time and the levels make no clicks either" )
    try
    {
        CReverb reverb;
        ASSERT_TRUE( PrepareReverb( reverb, 2 ) );
        TSamples warm = MakeNoise( 3 * Rate, 2, 51, 0.3f );
        Run( reverb, warm, 2, 256 );
        TSamples steady = MakeNoise( 2 * Rate, 2, 52, 0.3f );
        Run( reverb, steady, 2, 256 );
        const Float32 steadyStep = MaxStep( steady, 2, 0 );
        for ( UInt32 i=0; i<4; ++i )
        {
            switch ( i )
            {
                case 0 : reverb.SetPreDelayInMs( 300.0f ); break;
                case 1 : reverb.SetDecayTimeInSeconds( 8.0f ); reverb.SetHfRatio( 1.0f ); break;
                case 2 : reverb.SetLateLevelInDecibels( -30.0f ); reverb.SetEarlyLevelInDecibels( -60.0f ); break;
                default : reverb.SetLateLevelInDecibels( 0.0f ); reverb.SetEarlyLevelInDecibels( 0.0f ); reverb.SetPreDelayInMs( 0.0f ); break;
            }
            TSamples changing = MakeNoise( Rate, 2, 53 + i, 0.3f );
            Run( reverb, changing, 2, 256 );
            // a louder setting gives a larger signal and so larger steps: compare the step with the level
            const Float64 level = RmsOfChannel( changing, 2, 0, Rate / 2, Rate );
            const Float64 steadyLevel = RmsOfChannel( steady, 2, 0, 0, 2 * Rate );
            const Float32 step = MaxStep( changing, 2, 0 );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioReverb: change " << i << ": largest step " << step << " (level " << level << ", level before " << steadyLevel << ", steady step " << steadyStep << ")" );
            ASSERT_TRUE( step <= 1.6f * steadyStep * (Float32) ( level > steadyLevel ? level / steadyLevel : 1.0 ) );
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
PerformTailAndQualityTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioReverb: The tail is the pre-delay plus 1.5 times the decay time (more where the input diffusion rings longer), and the response is silent after it" )
    try
    {
        struct SCase { Float32 decay; Float32 preDelay; Float32 size; Float32 hfRatio; bool decayDominates; };
        const SCase cases[] = { { 1.8f, 20.0f, 0.6f, 0.5f, true }, { 0.1f, 0.0f, 0.1f, 1.0f, false }, { 4.0f, 250.0f, 1.0f, 0.2f, true }, { 0.5f, 500.0f, 0.3f, 1.0f, true }, { 10.0f, 0.0f, 1.0f, 1.0f, true } };
        for ( UInt32 quality=0; quality<2; ++quality )
            for ( UInt32 i=0; i<5; ++i )
            {
                CReverb reverb( QualityOptions( 1 == quality ) );
                reverb.SetDecayTimeInSeconds( cases[ i ].decay );
                reverb.SetPreDelayInMs( cases[ i ].preDelay );
                reverb.SetSize( cases[ i ].size );
                reverb.SetHfRatio( cases[ i ].hfRatio );
                ASSERT_TRUE( PrepareReverb( reverb, 2 ) );
                const UInt32 expected = (UInt32) ceil( 1.5 * (Float64) cases[ i ].decay * Rate ) + (UInt32) floor( (Float64) cases[ i ].preDelay * 48.0 + 0.5 );
                if ( cases[ i ].decayDominates )
                    ASSERT_TRUE( reverb.GetTailInFrames() == expected );
                else
                    ASSERT_TRUE( reverb.GetTailInFrames() > expected );

                // a diffusion of 0 leaves only the decay time
                reverb.SetDiffusion( 0.0f );
                ASSERT_TRUE( reverb.GetTailInFrames() == expected );
                reverb.SetDiffusion( 0.6f );

                const UInt32 tail = reverb.GetTailInFrames();
                const TSamples response = ImpulseResponse( reverb, 2, tail + Rate );
                const Float32 afterTail = PeakOf( response, (size_t) tail * 2 );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioReverb: " << ( 0 == quality ? "normal" : "low" ) << ", decay " << cases[ i ].decay << " s, pre-delay " << cases[ i ].preDelay << " ms: tail " << tail << " frames, peak after the tail " << afterTail <<
                                                  " (" << DecibelsOf( afterTail ) << " dB re the impulse)" );
                // 90 dB below the impulse that went in
                ASSERT_TRUE( afterTail <= (Float32) LinearOf( -90.0 ) );
            }

        // a reverb that is not prepared still reports a tail, from the sample rate it assumes
        CReverb fresh;
        ASSERT_TRUE( fresh.GetTailInFrames() > 0 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioReverb: Low quality works (four lines, no early reflections, other output) and costs less than normal quality" )
    try
    {
        CReverb normal;
        CReverb low( QualityOptions( true ) );
        ASSERT_TRUE( PrepareReverb( normal, 2 ) && PrepareReverb( low, 2 ) );
        ASSERT_TRUE( 8 == normal.GetLineCount() && 4 == low.GetLineCount() );
        const TSamples input = MakeNoise( Rate, 2, 61, 0.5f );
        TSamples a( input );
        TSamples b( input );
        Run( normal, a, 2, 256 );
        Run( low, b, 2, 256 );
        ASSERT_TRUE( AllFiniteAndBelow( a, 10.0f ) && AllFiniteAndBelow( b, 10.0f ) );
        ASSERT_FALSE( IsSame( a, b ) );
        ASSERT_TRUE( RmsOfChannel( b, 2, 0, Rate / 2, Rate ) > 0.01 );
        // the level of the two is about the same: the reverberation of both is of unit energy
        const Float64 difference = DecibelsOf( RmsOfChannel( a, 2, 0, Rate / 2, Rate ) / RmsOfChannel( b, 2, 0, Rate / 2, Rate ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioReverb: level of normal quality against low quality: " << difference << " dB" );
        ASSERT_TRUE( fabs( difference ) < 4.0 );

        CProcessNoise normalWork( normal, 5 * Rate );
        CProcessNoise lowWork( low, 5 * Rate );
        const Float64 normalMs = BestTimeInMs( normalWork, 5 );
        const Float64 lowMs = BestTimeInMs( lowWork, 5 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioReverb: 5 s of stereo noise takes " << normalMs << " ms in normal quality and " << lowMs << " ms in low quality (in this build)" );
        ASSERT_TRUE( lowMs < normalMs );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioReverb: The size scales the response in time: a larger size gives later first reflections and longer line delays" )
    try
    {
        size_t previousOnset = 0;
        const Float32 sizes[] = { 0.1f, 0.3f, 0.6f, 1.0f };
        for ( UInt32 i=0; i<4; ++i )
        {
            CReverb reverb;
            reverb.SetSize( sizes[ i ] );
            reverb.SetPreDelayInMs( 0.0f );
            reverb.SetEarlyLevelInDecibels( 0.0f );
            reverb.SetLateLevelInDecibels( -60.0f );
            ASSERT_TRUE( PrepareReverb( reverb, 2 ) );
            const TSamples response = ImpulseResponse( reverb, 2, 10000 );
            const size_t onset = FirstAbove( response, 2, 0, 1.0e-4f );
            ASSERT_TRUE( onset > previousOnset );
            ASSERT_TRUE( onset == (size_t) floor( 5.0 * 48.0 * sizes[ i ] + 0.5 ) );
            previousOnset = onset;
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
PerformConfigurationTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioReverb: The registry makes the reverb with its quality option, the quality is saved with the construction options and the parameters by name" )
    try
    {
        AUDIO::CAudioEffectRegistry& registry = AUDIO::CAudioGlobal::Instance()->GetAudioEffectRegistry();
        AUDIO::CAudioEffectRegistry::TRegisteredFactoryPtr factory;
        ASSERT_TRUE( registry.TryFindFactory( "reverb", factory ) );
        ASSERT_TRUE( CReverb::PARAMETER_COUNT == factory->GetParameterCount() );

        AUDIO::TAudioEffectPtr normal = registry.CreateEffect( "reverb" );
        AUDIO::TAudioEffectPtr low = registry.CreateEffect( "REVERB", QualityOptions( true ) );
        ASSERT_TRUE( !normal.IsNULL() && !low.IsNULL() );
        ASSERT_TRUE( normal->GetEffectTypeName() == "reverb" && low->GetFactory() == factory.GetPointerAlways() );
        ASSERT_TRUE( CReverb::QUALITY_NORMAL == static_cast< CReverb* >( normal.GetPointerAlways() )->GetQuality() );
        ASSERT_TRUE( CReverb::QUALITY_LOW == static_cast< CReverb* >( low.GetPointerAlways() )->GetQuality() );

        CORE::CValueList options;
        low->GetConstructionOptions( options );
        ASSERT_TRUE( options.GetValueAlways( "quality" ).AsString() == "low" );
        CORE::CValueList normalOptions;
        normal->GetConstructionOptions( normalOptions );
        ASSERT_TRUE( normalOptions.GetValueAlways( "quality" ).AsString() == "normal" );

        // save: type, options, parameters; load into an effect made from the saved options
        for ( UInt32 quality=0; quality<2; ++quality )
        {
            AUDIO::TAudioEffectPtr source = registry.CreateEffect( "reverb", QualityOptions( 1 == quality ) );
            CXorShift random( 70 + quality );
            for ( UInt32 p=0; p<source->GetParameterCount(); ++p )
            {
                const AUDIO::CAudioEffectParameterInfo& info = *source->GetParameterInfo( p );
                source->SetParameter( p, info.GetMinimum() + ( info.GetMaximum() - info.GetMinimum() ) * (Float32) ( 1 + random.Next() % 255 ) / 256.0f );
            }
            CORE::CDataNode config( "Effect" );
            ASSERT_TRUE( source->SaveConfig( config ) );
            ASSERT_TRUE( config.GetAttributeValue( "type" ).AsString() == "reverb" );
            const CORE::CDataNode* optionsNode = config.FindChild( "Options" );
            ASSERT_TRUE( GUCEF_NULL != optionsNode );
            ASSERT_TRUE( optionsNode->GetAttributeValue( "quality" ).AsString() == ( 0 == quality ? "normal" : "low" ) );
            const CORE::CDataNode* parameters = config.FindChild( "Parameters" );
            ASSERT_TRUE( GUCEF_NULL != parameters );
            for ( UInt32 p=0; p<source->GetParameterCount(); ++p )
                ASSERT_TRUE( parameters->IsAttribute( source->GetParameterInfo( p )->GetName() ) );

            CORE::CValueList saved;
            saved.Set( "quality", optionsNode->GetAttributeValue( "quality" ).AsString() );
            AUDIO::TAudioEffectPtr target = registry.CreateEffect( "reverb", saved );
            ASSERT_TRUE( !target.IsNULL() && target->LoadConfig( config ) );
            ASSERT_TRUE( static_cast< CReverb* >( target.GetPointerAlways() )->GetQuality() == static_cast< CReverb* >( source.GetPointerAlways() )->GetQuality() );
            for ( UInt32 p=0; p<source->GetParameterCount(); ++p )
            {
                const AUDIO::CAudioEffectParameterInfo& info = *source->GetParameterInfo( p );
                const Float64 range = (Float64) info.GetMaximum() - (Float64) info.GetMinimum();
                ASSERT_TRUE( Near( source->GetParameter( p ), target->GetParameter( p ), 1.0e-5 * ( range > 1.0 ? range : 1.0 ) ) );
            }

            // and they sound the same
            ASSERT_TRUE( source->Prepare( AUDIO::CAudioFormat( Rate, 2, AUDIO::SAMPLETYPE_FLOAT32 ), 512 ) );
            ASSERT_TRUE( target->Prepare( AUDIO::CAudioFormat( Rate, 2, AUDIO::SAMPLETYPE_FLOAT32 ), 512 ) );
            TSamples a = MakeNoise( 30000, 2, 80, 0.4f );
            TSamples b( a );
            Run( *source, a, 2, 512 );
            Run( *target, b, 2, 512 );
            Float32 worst = 0.0f;
            for ( size_t i=0; i<a.size(); ++i )
                if ( fabsf( a[ i ] - b[ i ] ) > worst )
                    worst = fabsf( a[ i ] - b[ i ] );
            ASSERT_TRUE( worst <= 1.0e-4f );
        }

        // the engine makes it prepared for its mix format
        AUDIO::CAudioEngineParams params;
        params.SetDriverName( "null" );
        params.SetThreadingMode( AUDIO::AUDIOTHREADING_COOPERATIVE );
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( params ) );
        AUDIO::TAudioEffectPtr fromEngine = engine.CreateEffect( "reverb", QualityOptions( true ) );
        ASSERT_TRUE( !fromEngine.IsNULL() && fromEngine->IsPrepared() );
        ASSERT_TRUE( CReverb::QUALITY_LOW == static_cast< CReverb* >( fromEngine.GetPointerAlways() )->GetQuality() );

        // and a bus layout keeps the quality
        AUDIO::CAudioBusLayout layout;
        layout.AddBus( "master" );
        AUDIO::CAudioBusLayout::SEffect& effect = layout.AddBus( "room" ).AddEffect( 0, "reverb" );
        effect.SetOption( "quality", "low" );
        effect.SetParameter( "decayTime", 3.5f );
        CORE::CFutureResult applied = engine.ApplyBusLayout( layout );
        ASSERT_FALSE( applied.HasNoFuture() );
        AUDIO::CNullAudioDevice* null = engine.GetDevice()->AsNullDevice();
        ASSERT_TRUE( GUCEF_NULL != null );
        for ( UInt32 i=0; i<100 && !applied.IsReady(); ++i )
        {
            null->Render( 256 );
            engine.Update();
        }
        ASSERT_TRUE( applied.IsReady() );
        AUDIO::TAudioEffectPtr inBus = engine.GetBusEffect( engine.FindBus( "room" ), 0 );
        ASSERT_TRUE( !inBus.IsNULL() && inBus->GetEffectTypeName() == "reverb" );
        ASSERT_TRUE( CReverb::QUALITY_LOW == static_cast< CReverb* >( inBus.GetPointerAlways() )->GetQuality() );
        ASSERT_TRUE( Near( inBus->GetParameter( CReverb::PARAMETER_DECAY_TIME ), 3.5, 1.0e-6 ) );
        AUDIO::CAudioBusLayout back;
        ASSERT_TRUE( engine.GetBusLayout( back ) );
        const AUDIO::CAudioBusLayout::SBus* room = GUCEF_NULL;
        for ( size_t i=0; i<back.GetBuses().size(); ++i )
            if ( "room" == back.GetBuses()[ i ].name )
                room = &back.GetBuses()[ i ];
        ASSERT_TRUE( GUCEF_NULL != room && 1 == room->effects.size() );
        const CORE::CDataNode* savedOptions = room->effects[ 0 ].config.FindChild( "Options" );
        ASSERT_TRUE( GUCEF_NULL != savedOptions && savedOptions->GetAttributeValue( "quality" ).AsString() == "low" );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformRepeatabilityTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioReverb: The output is bit for bit the same however the audio is split into blocks, 10 s of noise, both qualities, stereo and 5.1, defaults and extreme settings" )
    try
    {
        for ( UInt32 variant=0; variant<4; ++variant )
        {
            const UInt32 channels = ( variant & 1 ) ? 6 : 2;
            const bool low = variant >= 2;
            CReverb whole( QualityOptions( low ) );
            CReverb split( QualityOptions( low ) );
            CReverb random( QualityOptions( low ) );
            CReverb* all[ 3 ] = { &whole, &split, &random };
            for ( UInt32 e=0; e<3; ++e )
            {
                if ( variant >= 2 )
                {
                    all[ e ]->SetSize( 0.9f );
                    all[ e ]->SetDecayTimeInSeconds( 7.0f );
                    all[ e ]->SetHfRatio( 0.3f );
                    all[ e ]->SetPreDelayInMs( 37.3f );
                    all[ e ]->SetWidth( 0.4f );
                    all[ e ]->SetModulationDepth( 1.0f );
                    all[ e ]->SetLowCutInHertz( 300.0f );
                    all[ e ]->SetHighCutInHertz( 5000.0f );
                }
            }
            ASSERT_TRUE( PrepareReverb( whole, channels, Rate, 10 * Rate ) );
            ASSERT_TRUE( PrepareReverb( split, channels, Rate, 4096 ) );
            ASSERT_TRUE( PrepareReverb( random, channels, Rate, 4096 ) );

            const TSamples input = MakeNoise( 10 * Rate, channels, 90 + variant, 0.5f );
            TSamples a( input );
            TSamples b( input );
            TSamples c( input );
            Run( whole, a, channels, 10 * Rate );
            Run( split, b, channels, 64 );
            RunInRandomBlocks( random, c, channels, 1000, 17 + variant );
            ASSERT_TRUE( IsSame( a, b ) );
            ASSERT_TRUE( IsSame( a, c ) );
            ASSERT_TRUE( RmsOfChannel( a, channels, 0, 5 * Rate, 10 * Rate ) > 0.0 );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioReverb: Reset gives the same output as a new effect, also after parameter changes and loud input" )
    try
    {
        for ( UInt32 quality=0; quality<2; ++quality )
        {
            CReverb used( QualityOptions( 1 == quality ) );
            ASSERT_TRUE( PrepareReverb( used, 2 ) );
            TSamples junk = MakeNoise( 3 * Rate, 2, 3, 0.9f );
            Run( used, junk, 2, 256 );
            used.SetSize( 0.95f );
            used.SetPreDelayInMs( 311.0f );
            TSamples more = MakeNoise( Rate, 2, 4, 0.9f );
            Run( used, more, 2, 256 );

            // the parameters of the new effect are those the used one has now
            CReverb fresh( QualityOptions( 1 == quality ) );
            for ( UInt32 p=0; p<CReverb::PARAMETER_COUNT; ++p )
                fresh.SetParameter( p, used.GetParameter( p ) );
            ASSERT_TRUE( PrepareReverb( fresh, 2 ) );

            ResetRealtime( used );
            const TSamples input = MakeNoise( 2 * Rate, 2, 5, 0.4f );
            TSamples a( input );
            TSamples b( input );
            Run( used, a, 2, 300 );
            Run( fresh, b, 2, 300 );
            ASSERT_TRUE( IsSame( a, b ) );

            // a second reset in the middle of processing, without the audio of before
            ResetRealtime( used );
            ResetRealtime( fresh );
            TSamples c( input );
            TSamples d( input );
            Run( used, c, 2, 77 );
            Run( fresh, d, 2, 1000 );
            ASSERT_TRUE( IsSame( c, d ) );
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
PerformDenormalTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioReverb: After the tail the output is exactly silent: no denormal numbers in the output, for short and long decay times" )
    try
    {
        const Float32 decays[] = { 0.1f, 0.5f, 2.0f };
        for ( UInt32 quality=0; quality<2; ++quality )
            for ( UInt32 i=0; i<3; ++i )
            {
                CReverb reverb( QualityOptions( 1 == quality ) );
                reverb.SetDecayTimeInSeconds( decays[ i ] );
                reverb.SetHfRatio( 0.3f );
                reverb.SetLateLevelInDecibels( 0.0f );
                reverb.SetEarlyLevelInDecibels( 0.0f );
                ASSERT_TRUE( PrepareReverb( reverb, 2 ) );
                // noise first so that the state is full, then silence
                TSamples noise = MakeNoise( 2 * Rate, 2, 21, 0.8f );
                Run( reverb, noise, 2, 256 );
                const UInt32 seconds = 14;
                TSamples silence( (size_t) seconds * Rate * 2, 0.0f );
                Run( reverb, silence, 2, 256 );
                ASSERT_TRUE( 0 == CountSubnormal( silence ) );
                // 300 dB below the start is flushed: the end of the run is silent to the last bit once the decay got there
                const Float64 secondsToFlush = decays[ i ] * 300.0 / 60.0 * 1.2 + 1.0;
                if ( secondsToFlush < seconds )
                {
                    const size_t from = (size_t) ( secondsToFlush * Rate );
                    size_t firstSound = (size_t) seconds * Rate;
                    for ( size_t f=from; f<(size_t) seconds * Rate; ++f )
                        if ( 0.0f != silence[ f * 2 ] || 0.0f != silence[ f * 2 + 1 ] )
                        {
                            firstSound = f;
                            break;
                        }
                    if ( firstSound < (size_t) seconds * Rate )
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioReverb: not silent at frame " << firstSound << " (" << (Float64) firstSound / Rate << " s): " << silence[ firstSound * 2 ] );
                    ASSERT_TRUE( firstSound == (size_t) seconds * Rate );
                }
            }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioReverb: A decaying tail costs no more than an active block (the denormal check: the median second of the tail takes not above twice as long as the median second of noise)" )
    try
    {
        CReverb reverb;
        reverb.SetDecayTimeInSeconds( 0.5f );
        ASSERT_TRUE( PrepareReverb( reverb, 2 ) );
        TSamples noise = MakeNoise( 2 * Rate, 2, 22, 0.5f );
        Run( reverb, noise, 2, 256 );

        // seconds of active audio, then seconds of the tail after it, timed one by one
        const UInt32 seconds = 9;
        Float64 activeMs[ seconds ];
        Float64 tailMs[ seconds ];
        TSamples input = MakeNoise( Rate, 2, 23, 0.5f );
        for ( UInt32 i=0; i<seconds; ++i )
        {
            TSamples work( input );
            const UInt64 start = MT::PrecisionTickCount();
            Run( reverb, work, 2, 256 );
            activeMs[ i ] = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start );
        }
        TSamples tail( (size_t) Rate * 2, 0.0f );
        for ( UInt32 i=0; i<seconds; ++i )
        {
            const UInt64 start = MT::PrecisionTickCount();
            Run( reverb, tail, 2, 256 );
            tailMs[ i ] = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start );
        }
        const Float64 activeMedian = MedianOf( activeMs, seconds );
        const Float64 tailMedian = MedianOf( tailMs, seconds );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioReverb: 1 s of active stereo audio takes " << activeMedian << " ms (median), 1 s of the decaying tail " << tailMedian << " ms (median)" );
        ASSERT_TRUE( tailMedian <= 2.0 * activeMedian );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformRealtimeTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioReverb: Processing, reset and the application of parameter changes (size, pre-delay, decay time, all parameters) run in a real time context without allocation or locks" )
    try
    {
        for ( UInt32 quality=0; quality<2; ++quality )
        {
            CReverb reverb( QualityOptions( 1 == quality ) );
            ASSERT_TRUE( PrepareReverb( reverb, 6, Rate, 512 ) );
            CXorShift random( 300 + quality );
            TSamples block = MakeNoise( 512, 6, 9, 0.6f );
            for ( UInt32 round=0; round<400; ++round )
            {
                switch ( round % 4 )
                {
                    case 0 : RandomizeAll( reverb, random ); break;
                    case 1 :
                        // size and decay time: the audio thread sees them at the start of the next block, and finds new prime line lengths
                        reverb.SetSize( 0.1f + 0.9f * (Float32) random.NextUnit() );
                        reverb.SetDecayTimeInSeconds( 0.1f + 19.0f * (Float32) random.NextUnit() );
                        break;
                    case 2 : reverb.SetPreDelayInMs( 500.0f * (Float32) random.NextUnit() ); break;
                    default : break;
                }
                if ( 0 == round % 37 )
                    ResetRealtime( reverb );
                TSamples data = MakeNoise( 512, 6, 10 + round, 0.6f );
                ProcessRealtime( reverb, &data[ 0 ], 1 + random.Next() % 512 );
                ASSERT_TRUE( AllFiniteAndBelow( data, 1000.0f ) );
            }
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformAudioReverbTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioReverb TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.Reverb" );

    PerformIdentityTests();
    PerformDecayTimeTests();
    PerformTimingTests();
    PerformChannelTests();
    PerformStabilityTests();
    PerformTailAndQualityTests();
    PerformConfigurationTests();
    PerformRepeatabilityTests();
    PerformDenormalTests();
    PerformRealtimeTests();
}

/*-------------------------------------------------------------------------*/
