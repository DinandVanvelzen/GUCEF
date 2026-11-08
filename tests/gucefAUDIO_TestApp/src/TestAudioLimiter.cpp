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
#include <limits>

#ifndef GUCEFAUDIO_H
#include "gucefAUDIO.h"
#define GUCEFAUDIO_H
#endif /* GUCEFAUDIO_H ? */

#ifndef GUCEF_AUDIO_CAUDIOMIXER_H
#include "gucefAUDIO_CAudioMixer.h"
#define GUCEF_AUDIO_CAUDIOMIXER_H
#endif /* GUCEF_AUDIO_CAUDIOMIXER_H ? */

#ifndef GUCEF_AUDIO_CLIMITEREFFECT_H
#include "gucefAUDIO_CLimiterEffect.h"
#define GUCEF_AUDIO_CLIMITEREFFECT_H
#endif /* GUCEF_AUDIO_CLIMITEREFFECT_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAudioEffectSupport.h"
#include "TestAudioLimiter.h"

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

const AUDIO::CAudioFormat StereoFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 );

/** The master stage limiter of the mixer, written out: the arithmetic instant mode has to reproduce bit for bit */
void
ReferenceMasterLimiter( TSamples& audio, UInt32 channels, UInt32 rate, UInt64& limitedFrames )
{
    const Float32 ceiling = 0.98f;
    const Float32 release = 1.0f - (Float32) exp( -1.0 / ( 0.05 * rate ) );
    Float32 gain = 1.0f;
    limitedFrames = 0;
    const size_t frames = audio.size() / channels;
    for ( size_t f=0; f<frames; ++f )
    {
        Float32* frame = &audio[ f * channels ];
        Float32 peak = 0.0f;
        for ( UInt32 c=0; c<channels; ++c )
        {
            const Float32 level = frame[ c ] < 0.0f ? -frame[ c ] : frame[ c ];
            if ( level > peak )
                peak = level;
        }
        gain += ( 1.0f - gain ) * release;
        if ( peak * gain > ceiling )
            gain = ceiling / peak;
        if ( gain < 1.0f )
        {
            ++limitedFrames;
            for ( UInt32 c=0; c<channels; ++c )
                frame[ c ] *= gain;
        }
    }
}

/** What the mixer does with the master bus after the limiter: nothing outside -1 .. 1 leaves it */
void
ClampLikeMixer( TSamples& audio )
{
    for ( size_t i=0; i<audio.size(); ++i )
    {
        if ( audio[ i ] > 1.0f ) audio[ i ] = 1.0f;
        else if ( audio[ i ] < -1.0f ) audio[ i ] = -1.0f;
    }
}

/** Noise with loud passages in it: what makes a limiter work */
TSamples
MakeLoudNoise( UInt32 frames, UInt32 channels, UInt32 seed )
{
    TSamples audio = MakeNoise( frames, channels, seed, 0.6f );
    CXorShift random( seed + 7 );
    for ( UInt32 burst=0; burst<40; ++burst )
    {
        const UInt32 start = random.Next() % ( frames - 2000 );
        const UInt32 length = 1 + random.Next() % 1500;
        const Float32 boost = 1.0f + 2.0f * (Float32) random.NextUnit();
        for ( UInt32 f=start; f<start+length; ++f )
            for ( UInt32 c=0; c<channels; ++c )
                audio[ (size_t) f * channels + c ] *= boost;
    }
    return audio;
}

AUDIO::TAudioClipPtr
MakeClipOf( const TSamples& audio, UInt32 rate, UInt32 channels )
{
    const UInt32 frames = (UInt32) ( audio.size() / channels );
    AUDIO::CAudioBuffer buffer( AUDIO::CAudioFormat( rate, channels, AUDIO::SAMPLETYPE_FLOAT32 ), frames );
    for ( UInt32 f=0; f<frames; ++f )
        for ( UInt32 c=0; c<channels; ++c )
            buffer.SetSampleFromFloat( f, c, audio[ (size_t) f * channels + c ] );
    return AUDIO::TAudioClipPtr( GUCEF_NEW AUDIO::CAudioClip( buffer ) );
}

/** A limiter effect in a mode, prepared. The caller sets parameters before that, when it wants them in effect from the start */
bool
PrepareLimiter( AUDIO::CLimiterEffect& limiter, UInt32 rate, UInt32 channels, UInt32 maxBlock )
{
    return limiter.Prepare( AUDIO::CAudioFormat( rate, channels, AUDIO::SAMPLETYPE_FLOAT32 ), maxBlock );
}

/** The ceiling the effect works with, the way it computes it from its parameter */
Float64
CeilingOf( const AUDIO::CLimiterEffect& limiter )
{
    return pow( 10.0, (Float64) limiter.GetParameter( AUDIO::CLimiterEffect::PARAMETER_CEILING ) / 20.0 );
}

struct SCeilingResult
{
    Float64 peak;
    Float64 ceiling;
    bool exceeded;
};

/** Runs a signal through a lookahead limiter and measures the largest output sample */
SCeilingResult
RunAgainstCeiling( const TSamples& input, UInt32 channels, Float32 ceilingDb, Float32 lookaheadMs, Float32 inputGainDb, UInt32 blockFrames, UInt32 randomSeed )
{
    SCeilingResult result;
    result.peak = 0.0;
    result.ceiling = 0.0;
    result.exceeded = true;

    AUDIO::CLimiterEffect limiter;
    limiter.SetMode( AUDIO::AUDIOLIMITERMODE_LOOKAHEAD );
    limiter.SetCeilingInDecibels( ceilingDb );
    limiter.SetLookaheadInMs( lookaheadMs );
    limiter.SetInputGainInDecibels( inputGainDb );
    if ( !PrepareLimiter( limiter, 48000, channels, 4096 ) )
        return result;
    result.ceiling = CeilingOf( limiter );

    TSamples audio( input );
    if ( 0 == blockFrames )
        RunInRandomBlocks( limiter, audio, channels, 700, randomSeed );
    else
        Run( limiter, audio, channels, blockFrames );

    result.exceeded = false;
    for ( size_t i=0; i<audio.size(); ++i )
    {
        const Float64 magnitude = fabs( (Float64) audio[ i ] );
        if ( !( magnitude <= result.ceiling ) )
            result.exceeded = true;                         // also catches a sample that is not a number
        if ( magnitude > result.peak )
            result.peak = magnitude;
    }
    return result;
}

/** Index of the first sample at or after 'from' (frame count, channel 0) where the gain ratio output / input reaches the level */
Int32
FirstFrameAtOrAbove( const TSamples& output, UInt32 channels, size_t fromFrame, Float32 reference, Float32 level )
{
    const size_t frames = output.size() / channels;
    for ( size_t f=fromFrame; f<frames; ++f )
        if ( output[ f * channels ] / reference >= level )
            return (Int32) f;
    return -1;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformInstantTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioLimiter: Instant mode gives bit for bit what the limiter of the mixer master stage gives, with the real mixer as reference" )
    try
    {
        const UInt32 frames = 96000;
        const TSamples input = MakeLoudNoise( frames, 2, 11 );

        // the real mixer: one voice at unity into the master bus, limiter on
        AUDIO::CAudioMixer mixer( StereoFormat, 256, 8, 4 );
        mixer.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_INSTANT );
        AUDIO::TAudioVoicePtr voice( GUCEF_NEW AUDIO::CAudioVoice( MakeClipOf( input, 48000, 2 )->CreateSource() ) );
        ASSERT_TRUE( mixer.PrepareVoice( voice.GetPointerAlways() ) );
        ASSERT_TRUE( mixer.AddVoice( voice.GetPointerAlways() ) );
        TSamples fromMixer( (size_t) frames * 2, 123.0f );
        mixer.Mix( &fromMixer[ 0 ], frames );
        ASSERT_TRUE( mixer.GetLimitedFrameCount() > 1000 );         // the signal gave the limiter something to do

        // the effect, with the ceiling and release of the mixer
        AUDIO::CLimiterEffect limiter;
        limiter.SetMode( AUDIO::AUDIOLIMITERMODE_INSTANT );
        limiter.SetCeilingInDecibels( AUDIO::CLimiterEffect::GetMixerCeilingInDecibels() );
        limiter.SetReleaseInMs( 50.0f );
        ASSERT_TRUE( PrepareLimiter( limiter, 48000, 2, frames ) );
        ASSERT_TRUE( 0 == limiter.GetLatencyInFrames() );
        TSamples fromEffect( input );
        Run( limiter, fromEffect, 2, 480 );
        ClampLikeMixer( fromEffect );
        ASSERT_TRUE( IsSame( fromEffect, fromMixer ) );
        ASSERT_TRUE( mixer.GetLimitedFrameCount() == limiter.GetLimitedFrameCount() );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioLimiter: instant mode, " << mixer.GetLimitedFrameCount() << " of " << frames << " frames limited, identical to the mixer" );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioLimiter: Instant mode equals the written out master stage arithmetic for any block size, rate and channel count" )
    try
    {
        const UInt32 rates[] = { 44100, 48000, 96000 };
        const UInt32 channelCounts[] = { 1, 2, 6, 8 };
        const UInt32 blocks[] = { 1, 7, 256, 4096, 0 };
        for ( UInt32 r=0; r<3; ++r )
            for ( UInt32 cc=0; cc<4; ++cc )
            {
                const UInt32 channels = channelCounts[ cc ];
                const TSamples input = MakeLoudNoise( 30000, channels, 100 + r * 10 + cc );
                TSamples reference( input );
                UInt64 referenceLimited = 0;
                ReferenceMasterLimiter( reference, channels, rates[ r ], referenceLimited );

                for ( UInt32 b=0; b<5; ++b )
                {
                    AUDIO::CLimiterEffect limiter;
                    limiter.SetMode( AUDIO::AUDIOLIMITERMODE_INSTANT );
                    limiter.SetCeilingInDecibels( AUDIO::CLimiterEffect::GetMixerCeilingInDecibels() );
                    ASSERT_TRUE( PrepareLimiter( limiter, rates[ r ], channels, 30000 ) );
                    TSamples output( input );
                    if ( 0 == blocks[ b ] )
                        RunInRandomBlocks( limiter, output, channels, 900, 5 );
                    else
                        Run( limiter, output, channels, blocks[ b ] );
                    ASSERT_TRUE( IsSame( output, reference ) );
                    ASSERT_TRUE( referenceLimited == limiter.GetLimitedFrameCount() );
                }
            }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioLimiter: The ceiling of the mixer is what the effect makes of its dB parameter: the same Float32" )
    try
    {
        AUDIO::CLimiterEffect limiter;
        limiter.SetCeilingInDecibels( AUDIO::CLimiterEffect::GetMixerCeilingInDecibels() );
        ASSERT_TRUE( Near( AUDIO::CLimiterEffect::GetMixerCeilingInDecibels(), -0.17548, 1e-4 ) );
        ASSERT_TRUE( 0.98f == (Float32) CeilingOf( limiter ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformLookaheadTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioLimiter: Lookahead mode never lets a sample through above the ceiling: impulses, square waves, +20 dB noise, edges of blocks" )
    try
    {
        const UInt32 frames = 24000;
        const UInt32 channels = 2;

        GUCEF::vector< TSamples > signals;
        GUCEF::vector< CORE::CString > names;

        // impulses of several sizes, at the start, at and around block edges, on one channel only
        const UInt32 impulsePositions[] = { 0, 1, 255, 256, 257, 1000, 4095, 4096 };
        const Float32 impulseSizes[] = { 1.0f, 4.0f, 100.0f };
        for ( UInt32 p=0; p<8; ++p )
            for ( UInt32 s=0; s<3; ++s )
            {
                TSamples impulse( (size_t) frames * channels, 0.0f );
                impulse[ (size_t) impulsePositions[ p ] * channels + ( p & 1 ) ] = impulseSizes[ s ];
                signals.push_back( impulse );
                names.push_back( "impulse" );
            }

        // full scale square waves, fast ones with a period of a few samples
        const UInt32 periods[] = { 2, 3, 7, 48, 960 };
        for ( UInt32 p=0; p<5; ++p )
        {
            TSamples square( (size_t) frames * channels, 0.0f );
            for ( UInt32 f=0; f<frames; ++f )
                for ( UInt32 c=0; c<channels; ++c )
                    square[ (size_t) f * channels + c ] = ( ( f / ( periods[ p ] / 2 > 0 ? periods[ p ] / 2 : 1 ) ) & 1 ) ? -1.0f : 1.0f;
            signals.push_back( square );
            names.push_back( "square" );
        }

        // noise at +20 dB, and noise that jumps between quiet and +20 dB
        signals.push_back( MakeNoise( frames, channels, 21, 10.0f ) );
        names.push_back( "noise +20 dB" );
        TSamples jumping = MakeNoise( frames, channels, 22, 0.05f );
        for ( UInt32 f=0; f<frames; ++f )
            if ( ( f / 333 ) % 5 == 4 )
                for ( UInt32 c=0; c<channels; ++c )
                    jumping[ (size_t) f * channels + c ] *= 200.0f;
        signals.push_back( jumping );
        names.push_back( "noise jumping" );

        // transients that start or end exactly at the edge of a block of 256
        TSamples edges( (size_t) frames * channels, 0.0f );
        for ( UInt32 block=1; block<40; ++block )
            for ( UInt32 d=0; d<3; ++d )
                for ( UInt32 c=0; c<channels; ++c )
                    edges[ (size_t) ( block * 256 - 3 + d + ( block & 1 ? 3 : 0 ) ) * channels + c ] = ( block & 2 ) ? -8.0f : 8.0f;
        signals.push_back( edges );
        names.push_back( "block edges" );

        // a step from silence to loud, and one from loud to silence and back, and a DC offset
        TSamples step( (size_t) frames * channels, 0.0f );
        for ( UInt32 f=5000; f<frames; ++f )
            for ( UInt32 c=0; c<channels; ++c )
                step[ (size_t) f * channels + c ] = 3.0f;
        signals.push_back( step );
        names.push_back( "step" );

        // a sweep of an amplitude of 2, from 20 Hz up to the Nyquist frequency
        TSamples sweep( (size_t) frames * channels, 0.0f );
        {
            Float64 phase = 0.0;
            for ( UInt32 f=0; f<frames; ++f )
            {
                const Float64 t = (Float64) f / (Float64) frames;
                const Float64 frequency = 20.0 * pow( 1200.0, t );
                phase += 2.0 * Pi * frequency / 48000.0;
                for ( UInt32 c=0; c<channels; ++c )
                    sweep[ (size_t) f * channels + c ] = 2.0f * (Float32) sin( phase );
            }
        }
        signals.push_back( sweep );
        names.push_back( "sweep" );

        const Float32 ceilings[] = { -0.3f, 0.0f, -6.0f, -12.0f };
        const Float32 lookaheads[] = { 0.5f, 1.5f, 10.0f };
        Float64 worstRatio = 0.0;
        UInt32 runs = 0;
        for ( size_t s=0; s<signals.size(); ++s )
            for ( UInt32 ce=0; ce<4; ++ce )
                for ( UInt32 la=0; la<3; ++la )
                    for ( UInt32 b=0; b<2; ++b )
                    {
                        const SCeilingResult result = RunAgainstCeiling( signals[ s ], channels, ceilings[ ce ], lookaheads[ la ], 0.0f, b == 0 ? 256 : 0, (UInt32) s + 1 );
                        if ( result.exceeded )
                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioLimiter: " << names[ s ] << " exceeds the ceiling " << result.ceiling << " with " << result.peak << " (ceiling " << ceilings[ ce ] << " dB, lookahead " << lookaheads[ la ] << " ms)" );
                        ASSERT_FALSE( result.exceeded );
                        if ( result.peak / result.ceiling > worstRatio )
                            worstRatio = result.peak / result.ceiling;
                        ++runs;
                    }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioLimiter: " << runs << " lookahead runs, none above the ceiling, highest output at " << worstRatio << " of the ceiling" );
        ASSERT_TRUE( worstRatio <= 1.0 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioLimiter: A loud sample comes out at the ceiling, not below it: the limiter does not turn down more than it needs to" )
    try
    {
        const Float32 sizes[] = { 1.5f, 4.0f, 100.0f };
        for ( UInt32 s=0; s<3; ++s )
        {
            TSamples audio( 4800 * 2, 0.0f );
            audio[ 1000 * 2 ] = sizes[ s ];
            const SCeilingResult result = RunAgainstCeiling( audio, 2, -3.0f, 1.5f, 0.0f, 256, 1 );
            ASSERT_FALSE( result.exceeded );
            ASSERT_TRUE( result.peak > 0.99999 * result.ceiling );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioLimiter: A step to loud is held back by the lookahead and comes out at the ceiling at once, with the attack done before it" )
    try
    {
        AUDIO::CLimiterEffect limiter;
        limiter.SetMode( AUDIO::AUDIOLIMITERMODE_LOOKAHEAD );
        limiter.SetCeilingInDecibels( -6.0f );
        limiter.SetLookaheadInMs( 2.0f );
        ASSERT_TRUE( PrepareLimiter( limiter, 48000, 2, 4096 ) );
        const UInt32 lookahead = limiter.GetLatencyInFrames();
        ASSERT_TRUE( 96 == lookahead );

        TSamples audio( 4800 * 2, 0.0f );
        for ( UInt32 f=1000; f<4800; ++f )
            audio[ (size_t) f * 2 ] = audio[ (size_t) f * 2 + 1 ] = 3.0f;
        Run( limiter, audio, 2, 300 );
        const Float64 ceiling = CeilingOf( limiter );
        for ( UInt32 f=0; f<1000 + lookahead; ++f )
            ASSERT_TRUE( 0.0f == audio[ (size_t) f * 2 ] );                 // silence, delayed
        for ( UInt32 f=1000 + lookahead; f<4800; ++f )
        {
            ASSERT_TRUE( (Float64) audio[ (size_t) f * 2 ] <= ceiling );
            ASSERT_TRUE( (Float64) audio[ (size_t) f * 2 ] > 0.999999 * ceiling );   // at the ceiling from the first sample on
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioLimiter: A ceiling that is turned down, and signals that are not numbers, still leave nothing above the ceiling" )
    try
    {
        AUDIO::CLimiterEffect limiter;
        limiter.SetMode( AUDIO::AUDIOLIMITERMODE_LOOKAHEAD );
        limiter.SetCeilingInDecibels( -0.3f );
        ASSERT_TRUE( PrepareLimiter( limiter, 48000, 2, 4096 ) );
        TSamples audio = MakeNoise( 48000, 2, 77, 0.9f );
        TSamples first( audio.begin(), audio.begin() + 24000 * 2 );
        TSamples second( audio.begin() + 24000 * 2, audio.end() );
        Run( limiter, first, 2, 256 );
        limiter.SetCeilingInDecibels( -12.0f );
        const Float64 lowered = CeilingOf( limiter );
        Run( limiter, second, 2, 256 );
        for ( size_t i=0; i<second.size(); ++i )
            ASSERT_TRUE( (Float64) fabsf( second[ i ] ) <= lowered );

        // infinity and not a number in the input: the output stays a number below the ceiling
        TSamples bad = MakeNoise( 8000, 2, 78, 0.5f );
        bad[ 100 ] = std::numeric_limits< Float32 >::infinity();
        bad[ 1001 ] = -std::numeric_limits< Float32 >::infinity();
        bad[ 3000 ] = std::numeric_limits< Float32 >::quiet_NaN();
        Run( limiter, bad, 2, 256 );
        for ( size_t i=0; i<bad.size(); ++i )
            ASSERT_TRUE( (Float64) fabsf( bad[ i ] ) <= lowered );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioLimiter: Input gain raises the level into the limiter and keeps the output at or below the ceiling" )
    try
    {
        // a quiet sine, +24 dB at the input: it has to be limited. -12 dB: it passes at a quarter of the level
        const TSamples sine = MakeSine( 24000, 2, 1000.0, 0.2f );
        SCeilingResult loud = RunAgainstCeiling( sine, 2, -3.0f, 1.5f, 24.0f, 256, 1 );
        ASSERT_FALSE( loud.exceeded );
        ASSERT_TRUE( loud.peak > 0.9 * loud.ceiling );

        AUDIO::CLimiterEffect limiter;
        limiter.SetMode( AUDIO::AUDIOLIMITERMODE_LOOKAHEAD );
        limiter.SetInputGainInDecibels( -12.0f );
        ASSERT_TRUE( PrepareLimiter( limiter, 48000, 2, 4096 ) );
        TSamples quiet( sine );
        Run( limiter, quiet, 2, 256 );
        const Float64 expected = 0.2 * LinearOf( -12.0 );
        ASSERT_TRUE( Near( (Float64) PeakOf( quiet ), expected, 0.001 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformLatencyAndReleaseTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioLimiter: Below the threshold the output is the input delayed by the reported latency, bit for bit" )
    try
    {
        const UInt32 rates[] = { 44100, 48000, 96000 };
        const Float32 lookaheads[] = { 0.5f, 1.5f, 10.0f };
        const UInt32 channelCounts[] = { 1, 2, 6 };
        for ( UInt32 r=0; r<3; ++r )
            for ( UInt32 la=0; la<3; ++la )
                for ( UInt32 cc=0; cc<3; ++cc )
                {
                    const UInt32 channels = channelCounts[ cc ];
                    AUDIO::CLimiterEffect limiter;
                    limiter.SetMode( AUDIO::AUDIOLIMITERMODE_LOOKAHEAD );
                    limiter.SetLookaheadInMs( lookaheads[ la ] );
                    ASSERT_TRUE( PrepareLimiter( limiter, rates[ r ], channels, 8192 ) );
                    const UInt32 latency = limiter.GetLatencyInFrames();
                    ASSERT_TRUE( latency == (UInt32) floor( (Float64) lookaheads[ la ] * rates[ r ] / 1000.0 + 0.5 ) );

                    const TSamples input = MakeNoise( 20000, channels, 5 + cc, 0.5f );        // 0.5 is below the ceiling of -0.3 dB
                    TSamples output( input );
                    RunInRandomBlocks( limiter, output, channels, 3000, 3 );
                    for ( size_t f=0; f<20000; ++f )
                        for ( UInt32 c=0; c<channels; ++c )
                        {
                            const Float32 expected = f >= latency ? input[ ( f - latency ) * channels + c ] : 0.0f;
                            ASSERT_TRUE( expected == output[ f * channels + c ] );
                        }
                    ASSERT_TRUE( 0 == limiter.GetLimitedFrameCount() );
                }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioLimiter: The latency found by cross-correlating the output with the input equals GetLatencyInFrames()" )
    try
    {
        const Float32 lookaheads[] = { 0.5f, 1.5f, 3.0f, 10.0f };
        for ( UInt32 la=0; la<4; ++la )
        {
            AUDIO::CLimiterEffect limiter;
            limiter.SetMode( AUDIO::AUDIOLIMITERMODE_LOOKAHEAD );
            limiter.SetLookaheadInMs( lookaheads[ la ] );
            ASSERT_TRUE( PrepareLimiter( limiter, 48000, 2, 4096 ) );
            const UInt32 latency = limiter.GetLatencyInFrames();

            const TSamples input = MakeNoise( 16384, 2, 91, 0.4f );
            TSamples output( input );
            Run( limiter, output, 2, 512 );
            UInt32 bestLag = 0;
            Float64 best = -1.0e30;
            for ( UInt32 lag=0; lag<=600; ++lag )
            {
                Float64 correlation = 0.0;
                for ( UInt32 f=lag; f<16384; ++f )
                    correlation += (Float64) input[ (size_t) ( f - lag ) * 2 ] * (Float64) output[ (size_t) f * 2 ];
                if ( correlation > best )
                {
                    best = correlation;
                    bestLag = lag;
                }
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioLimiter: lookahead " << lookaheads[ la ] << " ms: latency " << latency << " frames at 48000 Hz, cross-correlation peak at " << bestLag );
            ASSERT_TRUE( bestLag == latency );
        }

        // the other modes add no delay, and the default is the 1.5 ms of the design: 72 frames at 48 kHz
        AUDIO::CLimiterEffect defaults;
        ASSERT_TRUE( AUDIO::AUDIOLIMITERMODE_LOOKAHEAD == defaults.GetMode() );
        ASSERT_TRUE( PrepareLimiter( defaults, 48000, 2, 256 ) );
        ASSERT_TRUE( 72 == defaults.GetLatencyInFrames() );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioLimiter: latency of the default lookahead limiter (1.5 ms): " << defaults.GetLatencyInFrames() << " frames at 48000 Hz" );
        defaults.SetMode( AUDIO::AUDIOLIMITERMODE_INSTANT );
        ASSERT_TRUE( 0 == defaults.GetLatencyInFrames() );
        defaults.SetMode( AUDIO::AUDIOLIMITERMODE_OFF );
        ASSERT_TRUE( 0 == defaults.GetLatencyInFrames() );
        ASSERT_TRUE( 0 == defaults.GetTailInFrames() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioLimiter: Lookahead mode comes back up with the release time constant, within 10 percent (instant mode as well)" )
    try
    {
        const Float32 releases[] = { 50.0f, 200.0f, 500.0f };
        for ( UInt32 mode=0; mode<2; ++mode )
            for ( UInt32 r=0; r<3; ++r )
            {
                AUDIO::CLimiterEffect limiter;
                limiter.SetMode( 0 == mode ? AUDIO::AUDIOLIMITERMODE_LOOKAHEAD : AUDIO::AUDIOLIMITERMODE_INSTANT );
                limiter.SetReleaseInMs( releases[ r ] );
                limiter.SetCeilingInDecibels( -6.0f );
                ASSERT_TRUE( PrepareLimiter( limiter, 48000, 2, 4096 ) );
                const UInt32 latency = limiter.GetLatencyInFrames();

                // loud at a constant 2.0 until frame 'switchAt', then quiet at a constant 0.1: the gain goes from the reduction (ceiling / 2) up to 1
                const UInt32 frames = 48000 * 3;
                const UInt32 switchAt = 24000;
                TSamples audio( (size_t) frames * 2, 0.1f );
                for ( UInt32 f=0; f<switchAt; ++f )
                    audio[ (size_t) f * 2 ] = audio[ (size_t) f * 2 + 1 ] = 2.0f;
                Run( limiter, audio, 2, 512 );

                // the output over the (delayed) input is the gain; the quiet input is at the output from 'switchAt' + latency on
                const size_t from = switchAt + latency;
                const Float32 gainBefore = audio[ ( from - 1 ) * 2 ] / 2.0f;
                const Float32 target = gainBefore + 0.6321206f * ( 1.0f - gainBefore );
                const Int32 reached = FirstFrameAtOrAbove( audio, 2, from, 0.1f, target );
                ASSERT_TRUE( reached > 0 );
                const Float64 measured = (Float64) ( reached - (Int32) from ) / 48.0;
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioLimiter: " << ( 0 == mode ? "lookahead" : "instant" ) << " release set to " << releases[ r ] << " ms, gain recovered 63.2 percent after " << measured << " ms" );
                ASSERT_TRUE( Near( measured, releases[ r ], 0.1 * releases[ r ] ) );
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
PerformParameterTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioLimiter: The parameters have the ranges and defaults of the design, the mode can be set by option and by call" )
    try
    {
        AUDIO::CLimiterEffect limiter;
        ASSERT_TRUE( limiter.GetEffectTypeName() == "limiter" && 5 == limiter.GetParameterCount() );
        ASSERT_TRUE( Near( limiter.GetParameter( AUDIO::CLimiterEffect::PARAMETER_CEILING ), -0.3, 1e-6 ) );
        ASSERT_TRUE( Near( limiter.GetParameter( AUDIO::CLimiterEffect::PARAMETER_LOOKAHEAD ), 1.5, 1e-6 ) );
        ASSERT_TRUE( Near( limiter.GetParameter( AUDIO::CLimiterEffect::PARAMETER_RELEASE ), 50.0, 1e-6 ) );
        ASSERT_TRUE( 0.0f == limiter.GetParameter( AUDIO::CLimiterEffect::PARAMETER_INPUT_GAIN ) );
        limiter.SetCeilingInDecibels( 5.0f );
        ASSERT_TRUE( 0.0f == limiter.GetParameter( AUDIO::CLimiterEffect::PARAMETER_CEILING ) );
        limiter.SetCeilingInDecibels( -40.0f );
        ASSERT_TRUE( -12.0f == limiter.GetParameter( AUDIO::CLimiterEffect::PARAMETER_CEILING ) );
        limiter.SetLookaheadInMs( 50.0f );
        ASSERT_TRUE( 10.0f == limiter.GetParameter( AUDIO::CLimiterEffect::PARAMETER_LOOKAHEAD ) );
        limiter.SetReleaseInMs( 1.0f );
        ASSERT_TRUE( 10.0f == limiter.GetParameter( AUDIO::CLimiterEffect::PARAMETER_RELEASE ) );
        limiter.SetInputGainInDecibels( 60.0f );
        ASSERT_TRUE( 24.0f == limiter.GetParameter( AUDIO::CLimiterEffect::PARAMETER_INPUT_GAIN ) );

        CORE::CValueList options;
        options.Set( "mode", "instant" );
        AUDIO::CLimiterEffect byOption( options );
        ASSERT_TRUE( AUDIO::AUDIOLIMITERMODE_INSTANT == byOption.GetMode() );
        CORE::CValueList offOptions;
        offOptions.Set( "mode", "OFF" );
        AUDIO::CLimiterEffect off( offOptions );
        ASSERT_TRUE( AUDIO::AUDIOLIMITERMODE_OFF == off.GetMode() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioLimiter: Off passes the audio unchanged, and the mode can be switched while processing" )
    try
    {
        AUDIO::CLimiterEffect limiter;
        limiter.SetMode( AUDIO::AUDIOLIMITERMODE_OFF );
        ASSERT_TRUE( PrepareLimiter( limiter, 48000, 2, 1024 ) );
        const TSamples input = MakeNoise( 4000, 2, 3, 3.0f );
        TSamples output( input );
        Run( limiter, output, 2, 1000 );
        ASSERT_TRUE( IsSame( input, output ) );
        ASSERT_TRUE( 0 == limiter.GetLimitedFrameCount() );

        // instant, lookahead, off, lookahead: always a result that is a number, in bounds
        const AUDIO::TAudioLimiterMode modes[] = { AUDIO::AUDIOLIMITERMODE_INSTANT, AUDIO::AUDIOLIMITERMODE_LOOKAHEAD, AUDIO::AUDIOLIMITERMODE_OFF, AUDIO::AUDIOLIMITERMODE_LOOKAHEAD };
        for ( UInt32 m=0; m<4; ++m )
        {
            limiter.SetMode( modes[ m ] );
            TSamples block( input );
            Run( limiter, block, 2, 512 );
            if ( AUDIO::AUDIOLIMITERMODE_OFF != modes[ m ] )
                ASSERT_TRUE( AllFiniteAndBelow( block, 1.0f ) );
            else
                ASSERT_TRUE( IsSame( input, block ) );
        }
        ASSERT_TRUE( limiter.GetLimitedFrameCount() > 0 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioLimiter: Reset clears the delay and the gain, as if no audio had been processed" )
    try
    {
        AUDIO::CLimiterEffect limiter;
        ASSERT_TRUE( PrepareLimiter( limiter, 48000, 2, 4096 ) );
        TSamples loud = MakeNoise( 4000, 2, 4, 5.0f );
        Run( limiter, loud, 2, 500 );
        ResetRealtime( limiter );

        // after the reset, quiet audio comes out delayed, with silence in front and no gain reduction left from the loud audio
        const TSamples quiet = MakeNoise( 2000, 2, 5, 0.3f );
        TSamples output( quiet );
        Run( limiter, output, 2, 500 );
        const UInt32 latency = limiter.GetLatencyInFrames();
        for ( size_t f=0; f<2000; ++f )
            ASSERT_TRUE( ( f >= latency ? quiet[ ( f - latency ) * 2 ] : 0.0f ) == output[ f * 2 ] );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformAudioLimiterTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioLimiter TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.Limiter" );

    PerformInstantTests();
    PerformLookaheadTests();
    PerformLatencyAndReleaseTests();
    PerformParameterTests();
}

/*-------------------------------------------------------------------------*/
