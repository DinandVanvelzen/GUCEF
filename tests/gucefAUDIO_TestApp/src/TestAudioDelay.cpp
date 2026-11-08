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

#ifndef GUCEF_AUDIO_CBIQUADDESIGN_H
#include "gucefAUDIO_CBiquadDesign.h"
#define GUCEF_AUDIO_CBIQUADDESIGN_H
#endif /* GUCEF_AUDIO_CBIQUADDESIGN_H ? */

#ifndef GUCEF_AUDIO_CDELAYEFFECT_H
#include "gucefAUDIO_CDelayEffect.h"
#define GUCEF_AUDIO_CDELAYEFFECT_H
#endif /* GUCEF_AUDIO_CDELAYEFFECT_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAudioEffectSupport.h"
#include "TestAudioDelay.h"

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

bool
PrepareDelay( AUDIO::CDelayEffect& delay, UInt32 channels, UInt32 maxBlock = 4096 )
{
    return delay.Prepare( AUDIO::CAudioFormat( 48000, channels, AUDIO::SAMPLETYPE_FLOAT32 ), maxBlock );
}

/** Feedback filters out of the way: low cut at its lowest, high cut at its highest */
void
FiltersOff( AUDIO::CDelayEffect& delay )
{
    delay.SetLowCutInHertz( 20.0f );
    delay.SetHighCutInHertz( 20000.0f );
}

/** A unit impulse at 'frame' on the given channels */
TSamples
MakeImpulse( UInt32 frames, UInt32 channels, UInt32 frame, UInt32 channelMask )
{
    TSamples audio( (size_t) frames * channels, 0.0f );
    for ( UInt32 c=0; c<channels; ++c )
        if ( ( channelMask >> c ) & 1 )
            audio[ (size_t) frame * channels + c ] = 1.0f;
    return audio;
}

/** True when channel 'channel' holds the expected amplitude at the given frames and (almost) nothing anywhere else */
bool
HasEchoes( const TSamples& audio, UInt32 channels, UInt32 channel, const GUCEF::vector< UInt32 >& frames, const GUCEF::vector< Float64 >& amplitudes, Float64 tolerance )
{
    const size_t total = audio.size() / channels;
    size_t next = 0;
    for ( size_t f=0; f<total; ++f )
    {
        Float64 expected = 0.0;
        if ( next < frames.size() && frames[ next ] == f )
            expected = amplitudes[ next++ ];
        if ( fabs( (Float64) audio[ f * channels + channel ] - expected ) > tolerance )
            return false;
    }
    return next == frames.size();
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformEchoTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioDelay: An impulse comes back at exact sample positions, with the amplitude feedback to the power n, positive and negative feedback" )
    try
    {
        const Float32 times[] = { 1.0f, 50.0f, 100.0f, 375.0f };
        const Float32 feedbacks[] = { 0.5f, -0.6f, 0.0f, 0.9f };
        for ( UInt32 t=0; t<4; ++t )
            for ( UInt32 fb=0; fb<4; ++fb )
            {
                AUDIO::CDelayEffect delay;
                delay.SetTimeInMs( times[ t ] );
                delay.SetFeedback( feedbacks[ fb ] );
                FiltersOff( delay );
                ASSERT_TRUE( PrepareDelay( delay, 2 ) );

                const UInt32 echoFrames = (UInt32) ( times[ t ] * 48.0f + 0.5f );
                const UInt32 total = 4 * echoFrames > 60000 ? 60000 : 4 * echoFrames + 100;
                TSamples audio = MakeImpulse( total + 200, 2, 100, 3 );
                Run( delay, audio, 2, 256 );

                GUCEF::vector< UInt32 > frames;
                GUCEF::vector< Float64 > amplitudes;
                Float64 amplitude = 1.0;
                for ( UInt32 echo=1; 100 + echo * echoFrames < total + 200; ++echo )
                {
                    frames.push_back( 100 + echo * echoFrames );
                    amplitudes.push_back( amplitude );
                    amplitude *= feedbacks[ fb ];
                    if ( 0.0f == feedbacks[ fb ] )
                        break;                                          // no feedback: one echo
                }
                for ( UInt32 c=0; c<2; ++c )
                    ASSERT_TRUE( HasEchoes( audio, 2, c, frames, amplitudes, 1.0e-6 ) );
            }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioDelay: Echoes come at the same positions however the audio is split into blocks; a long time works as well" )
    try
    {
        AUDIO::CDelayEffect delay;
        delay.SetTimeInMs( 1500.0f );
        delay.SetFeedback( 0.5f );
        FiltersOff( delay );
        ASSERT_TRUE( PrepareDelay( delay, 1, 100000 ) );
        TSamples audio = MakeImpulse( 200000, 1, 7, 1 );
        RunInRandomBlocks( delay, audio, 1, 5000, 12 );
        GUCEF::vector< UInt32 > frames;
        GUCEF::vector< Float64 > amplitudes;
        frames.push_back( 7 + 72000 );
        amplitudes.push_back( 1.0 );
        frames.push_back( 7 + 144000 );
        amplitudes.push_back( 0.5 );
        ASSERT_TRUE( HasEchoes( audio, 1, 0, frames, amplitudes, 1.0e-6 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioDelay: The maximum delay is an option, the time is limited to it" )
    try
    {
        CORE::CValueList options;
        options.Set( "maxDelay", "500" );
        AUDIO::CDelayEffect delay( options );
        ASSERT_TRUE( Near( delay.GetMaxDelayInMs(), 500.0, 1e-6 ) );
        delay.SetTimeInMs( 2000.0f );
        delay.SetFeedback( 0.0f );
        FiltersOff( delay );
        ASSERT_TRUE( PrepareDelay( delay, 1, 40000 ) );
        TSamples audio = MakeImpulse( 30000, 1, 0, 1 );
        Run( delay, audio, 1, 1000 );
        ASSERT_TRUE( 1.0f == audio[ 24000 ] );              // 500 ms

        CORE::CValueList huge;
        huge.Set( "maxDelay", "1000000" );
        AUDIO::CDelayEffect limited( huge );
        ASSERT_TRUE( Near( limited.GetMaxDelayInMs(), 10000.0, 1e-6 ) );
        AUDIO::CDelayEffect byDefault;
        ASSERT_TRUE( Near( byDefault.GetMaxDelayInMs(), 2000.0, 1e-6 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformFeedbackPathTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioDelay: The filters in the feedback path shape every repeat: high cut takes the highs, low cut the lows" )
    try
    {
        const UInt32 delayFrames = 14400;                               // 300 ms
        const Float64 butterworth = 0.7071067811865476;

        for ( UInt32 mode=0; mode<2; ++mode )
        {
            AUDIO::CDelayEffect delay;
            delay.SetTimeInMs( 300.0f );
            delay.SetFeedback( 0.8f );
            const Float64 toneFrequency = 0 == mode ? 8000.0 : 100.0;
            if ( 0 == mode )
            {
                delay.SetLowCutInHertz( 20.0f );
                delay.SetHighCutInHertz( 2000.0f );
            }
            else
            {
                delay.SetLowCutInHertz( 1000.0f );
                delay.SetHighCutInHertz( 20000.0f );
            }
            ASSERT_TRUE( PrepareDelay( delay, 1 ) );

            TSamples audio( 4 * delayFrames, 0.0f );
            const TSamples tone = MakeSine( 9600, 1, toneFrequency, 0.5f );
            for ( UInt32 f=0; f<9600; ++f )
                audio[ f ] = tone[ f ];
            Run( delay, audio, 1, 256 );

            // second echo against the first, in the second half of the bursts where the filters have settled
            const Float64 first = RmsOfChannel( audio, 1, 0, delayFrames + 4800, delayFrames + 9600 );
            const Float64 second = RmsOfChannel( audio, 1, 0, 2 * delayFrames + 4800, 2 * delayFrames + 9600 );
            const Float64 filter = AUDIO::CBiquadDesign::GetMagnitudeInDecibels( 0 == mode ? AUDIO::FILTERRESPONSE_LOWPASS : AUDIO::FILTERRESPONSE_HIGHPASS,
                                                                                 48000.0, 0 == mode ? 2000.0 : 1000.0, butterworth, 0.0, toneFrequency );
            const Float64 expected = DecibelsOf( 0.8 ) + filter;
            const Float64 measured = DecibelsOf( second / first );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioDelay: " << toneFrequency << " Hz through the feedback " << ( 0 == mode ? "high cut" : "low cut" ) << ": second echo " << measured << " dB below the first, expected " << expected << " dB" );
            ASSERT_TRUE( Near( measured, expected, 0.5 ) );
            ASSERT_TRUE( Near( DecibelsOf( first / RmsOfChannel( tone, 1, 0, 4800, 9600 ) ), 0.0, 0.01 ) );      // the first echo is the audio, unfiltered
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioDelay: Feedback near one stays bounded for ten seconds of noise with the filters at their defaults" )
    try
    {
        const Float32 feedbacks[] = { 0.98f, -0.98f };
        for ( UInt32 i=0; i<2; ++i )
        {
            AUDIO::CDelayEffect delay;
            delay.SetTimeInMs( 37.0f );
            delay.SetFeedback( feedbacks[ i ] );
            ASSERT_TRUE( PrepareDelay( delay, 2 ) );
            TSamples audio = MakeNoise( 480000, 2, 71 + i, 0.3f );
            Run( delay, audio, 2, 256 );
            ASSERT_TRUE( AllFiniteAndBelow( audio, 100.0f ) );
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
PerformStereoTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioDelay: Ping-pong sends the echoes from one channel to the other" )
    try
    {
        // left only in; with ping-pong the left echo comes back on the right and the other way round
        AUDIO::CDelayEffect delay;
        delay.SetTimeInMs( 50.0f );
        delay.SetFeedback( 0.5f );
        delay.SetPingPong( true );
        FiltersOff( delay );
        ASSERT_TRUE( PrepareDelay( delay, 2 ) );
        TSamples audio = MakeImpulse( 16000, 2, 0, 1 );
        Run( delay, audio, 2, 256 );

        GUCEF::vector< UInt32 > leftFrames, rightFrames;
        GUCEF::vector< Float64 > leftAmplitudes, rightAmplitudes;
        for ( UInt32 echo=1; echo<=6; ++echo )
        {
            if ( echo & 1 )
            {
                leftFrames.push_back( echo * 2400 );
                leftAmplitudes.push_back( pow( 0.5, echo - 1 ) );
            }
            else
            {
                rightFrames.push_back( echo * 2400 );
                rightAmplitudes.push_back( pow( 0.5, echo - 1 ) );
            }
        }
        ASSERT_TRUE( HasEchoes( audio, 2, 0, leftFrames, leftAmplitudes, 1.0e-6 ) );
        ASSERT_TRUE( HasEchoes( audio, 2, 1, rightFrames, rightAmplitudes, 1.0e-6 ) );

        // without it the echoes stay on their channel
        AUDIO::CDelayEffect plain;
        plain.SetTimeInMs( 50.0f );
        plain.SetFeedback( 0.5f );
        FiltersOff( plain );
        ASSERT_TRUE( PrepareDelay( plain, 2 ) );
        TSamples straight = MakeImpulse( 16000, 2, 0, 1 );
        Run( plain, straight, 2, 256 );
        for ( size_t f=0; f<16000; ++f )
            ASSERT_TRUE( 0.0f == straight[ f * 2 + 1 ] );
        ASSERT_TRUE( Near( straight[ 4800 * 2 ], 0.5, 1e-6 ) );

        // and a mono signal has nothing to ping-pong with: it echoes as usual
        AUDIO::CDelayEffect mono;
        mono.SetTimeInMs( 50.0f );
        mono.SetPingPong( true );
        FiltersOff( mono );
        ASSERT_TRUE( PrepareDelay( mono, 1 ) );
        TSamples single = MakeImpulse( 4000, 1, 0, 1 );
        Run( mono, single, 1, 256 );
        ASSERT_TRUE( 1.0f == single[ 2400 ] );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioDelay: Stereo offset delays the right channel by the offset" )
    try
    {
        AUDIO::CDelayEffect delay;
        delay.SetTimeInMs( 50.0f );
        delay.SetStereoOffsetInMs( 10.0f );
        delay.SetFeedback( 0.5f );
        FiltersOff( delay );
        ASSERT_TRUE( PrepareDelay( delay, 2 ) );
        TSamples audio = MakeImpulse( 9000, 2, 0, 3 );
        Run( delay, audio, 2, 256 );
        GUCEF::vector< UInt32 > leftFrames, rightFrames;
        GUCEF::vector< Float64 > amplitudes;
        leftFrames.push_back( 2400 ); leftFrames.push_back( 4800 ); leftFrames.push_back( 7200 );
        rightFrames.push_back( 2880 ); rightFrames.push_back( 5760 ); rightFrames.push_back( 8640 );
        amplitudes.push_back( 1.0 ); amplitudes.push_back( 0.5 ); amplitudes.push_back( 0.25 );
        ASSERT_TRUE( HasEchoes( audio, 2, 0, leftFrames, amplitudes, 1.0e-6 ) );
        ASSERT_TRUE( HasEchoes( audio, 2, 1, rightFrames, amplitudes, 1.0e-6 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioDelay: A low frequency effects channel passes through unchanged, the others are delayed" )
    try
    {
        const UInt32 channels = 6;
        AUDIO::CDelayEffect delay;
        delay.SetTimeInMs( 20.0f );
        delay.SetFeedback( 0.5f );
        delay.SetPingPong( true );
        ASSERT_TRUE( delay.Prepare( AUDIO::CAudioFormat( 48000, channels, AUDIO::SAMPLETYPE_FLOAT32 ), 1024 ) );
        const TSamples input = MakeNoise( 12000, channels, 17, 0.5f );
        TSamples output( input );
        Run( delay, output, channels, 256 );
        for ( size_t f=0; f<12000; ++f )
            ASSERT_TRUE( output[ f * channels + 3 ] == input[ f * channels + 3 ] );
        for ( size_t f=0; f<960; ++f )
            ASSERT_TRUE( 0.0f == output[ f * channels + 0 ] );                       // nothing before the first echo
        ASSERT_TRUE( PeakOf( output, 960 * channels, 1200 * channels ) > 0.0f );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformTimeAndTailTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioDelay: A change of the delay time glides, with steps in the audio no larger than the signal makes plus the speed of the glide" )
    try
    {
        const Float32 targets[] = { 120.0f, 80.0f, 600.0f, 5.0f };
        for ( UInt32 i=0; i<4; ++i )
        {
            AUDIO::CDelayEffect delay;
            delay.SetTimeInMs( 100.0f );
            delay.SetFeedback( 0.0f );
            FiltersOff( delay );
            ASSERT_TRUE( PrepareDelay( delay, 2 ) );
            const TSamples input = MakeSine( 144000, 2, 1000.0, 0.5f );
            const Float32 inputStep = MaxStep( input, 2, 0 );

            TSamples output;
            {
                TSamples first( input.begin(), input.begin() + 24000 * 2 );
                Run( delay, first, 2, 256 );
                output = first;
            }
            delay.SetTimeInMs( targets[ i ] );
            {
                TSamples second( input.begin() + 24000 * 2, input.end() );
                Run( delay, second, 2, 256 );
                output.insert( output.end(), second.begin(), second.end() );
            }

            // the glide speed is at most (change in frames) / (time constant in frames) of the position: a pitch change by that factor
            const Float64 changeFrames = fabs( (Float64) targets[ i ] - 100.0 ) * 48.0;
            const Float64 speed = changeFrames / ( 0.1 * 48000.0 );
            const Float32 bound = inputStep * (Float32) ( 1.0 + speed ) * 1.02f + 1.0e-4f;
            const Float32 largest = MaxStep( output, 2, 0, 5000 );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioDelay: time 100 ms to " << targets[ i ] << " ms: largest step " << largest << ", bound " << bound << " (signal " << inputStep << ")" );
            ASSERT_TRUE( largest <= bound );
            ASSERT_TRUE( AllFiniteAndBelow( output, 1.0f ) );

            // it arrives: after a few time constants the delay is the new one
            const UInt32 settled = 24000 + 96000;                    // 20 time constants of the glide
            const UInt32 newDelay = (UInt32) ( targets[ i ] * 48.0f + 0.5f );
            Float64 worstError = 0.0;
            for ( UInt32 f=settled; f<settled+200; ++f )
            {
                const Float64 error = fabs( (Float64) output[ (size_t) f * 2 ] - (Float64) input[ (size_t) ( f - newDelay ) * 2 ] );
                if ( error > worstError ) worstError = error;
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioDelay: time 100 ms to " << targets[ i ] << " ms: after 2 s the audio is off by " << worstError << " from the audio delayed by the new time" );
            ASSERT_TRUE( worstError <= 2.0e-3 );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioDelay: The tail is infinite while there is feedback and the delay time otherwise; the delay has no latency" )
    try
    {
        AUDIO::CDelayEffect delay;
        ASSERT_TRUE( delay.GetEffectTypeName() == "delay" && 6 == delay.GetParameterCount() );
        ASSERT_TRUE( PrepareDelay( delay, 2 ) );
        ASSERT_TRUE( 0 == delay.GetLatencyInFrames() );
        ASSERT_TRUE( Near( delay.GetParameter( AUDIO::CDelayEffect::PARAMETER_TIME ), 375.0, 1e-6 ) );
        ASSERT_TRUE( Near( delay.GetParameter( AUDIO::CDelayEffect::PARAMETER_FEEDBACK ), 0.35, 1e-6 ) );
        ASSERT_TRUE( 0.0f == delay.GetParameter( AUDIO::CDelayEffect::PARAMETER_PING_PONG ) );
        ASSERT_TRUE( 80.0f == delay.GetParameter( AUDIO::CDelayEffect::PARAMETER_LOW_CUT ) );
        ASSERT_TRUE( 8000.0f == delay.GetParameter( AUDIO::CDelayEffect::PARAMETER_HIGH_CUT ) );
        ASSERT_TRUE( GUCEF_AUDIO_TAIL_INFINITE == delay.GetTailInFrames() );
        delay.SetFeedback( -0.2f );
        ASSERT_TRUE( GUCEF_AUDIO_TAIL_INFINITE == delay.GetTailInFrames() );
        delay.SetFeedback( 0.0f );
        ASSERT_TRUE( 18000 == delay.GetTailInFrames() );
        delay.SetStereoOffsetInMs( 10.0f );
        ASSERT_TRUE( 18480 == delay.GetTailInFrames() );
        delay.SetTimeInMs( 10000.0f );                       // more than the effect holds: the tail is what it can hold
        ASSERT_TRUE( 96000 + 480 == delay.GetTailInFrames() );

        // parameter ranges
        delay.SetFeedback( 2.0f );
        ASSERT_TRUE( Near( delay.GetParameter( AUDIO::CDelayEffect::PARAMETER_FEEDBACK ), 0.98, 1e-6 ) );
        delay.SetFeedback( -2.0f );
        ASSERT_TRUE( Near( delay.GetParameter( AUDIO::CDelayEffect::PARAMETER_FEEDBACK ), -0.98, 1e-6 ) );
        delay.SetLowCutInHertz( 5000.0f );
        ASSERT_TRUE( 2000.0f == delay.GetParameter( AUDIO::CDelayEffect::PARAMETER_LOW_CUT ) );
        delay.SetHighCutInHertz( 100.0f );
        ASSERT_TRUE( 1000.0f == delay.GetParameter( AUDIO::CDelayEffect::PARAMETER_HIGH_CUT ) );
        delay.SetStereoOffsetInMs( 500.0f );
        ASSERT_TRUE( 50.0f == delay.GetParameter( AUDIO::CDelayEffect::PARAMETER_STEREO_OFFSET ) );
        delay.SetTimeInMs( 0.0f );
        ASSERT_TRUE( 1.0f == delay.GetParameter( AUDIO::CDelayEffect::PARAMETER_TIME ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformAudioDelayTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioDelay TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.Delay" );

    PerformEchoTests();
    PerformFeedbackPathTests();
    PerformStereoTests();
    PerformTimeAndTailTests();
}

/*-------------------------------------------------------------------------*/
