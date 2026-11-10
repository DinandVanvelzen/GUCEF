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

#ifndef GUCEF_AUDIO_CCOMPRESSOREFFECT_H
#include "gucefAUDIO_CCompressorEffect.h"
#define GUCEF_AUDIO_CCOMPRESSOREFFECT_H
#endif /* GUCEF_AUDIO_CCOMPRESSOREFFECT_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAudioEffectSupport.h"
#include "TestAudioCompressor.h"

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

/** Gain reduction in dB (positive) of the gain computer of the design for a level x, threshold T, ratio R and knee W: written independently of the effect */
Float64
ReferenceReduction( Float64 x, Float64 T, Float64 R, Float64 W )
{
    const Float64 over = x - T;
    if ( 2.0 * over < -W )
        return 0.0;
    if ( W > 0.0 && 2.0 * fabs( over ) <= W )
    {
        const Float64 inKnee = over + W / 2.0;
        return ( 1.0 - 1.0 / R ) * inKnee * inKnee / ( 2.0 * W );
    }
    return ( 1.0 - 1.0 / R ) * over;
}

bool
PrepareCompressor( AUDIO::CCompressorEffect& compressor, UInt32 channels, UInt32 maxBlock = 4096 )
{
    return compressor.Prepare( AUDIO::CAudioFormat( 48000, channels, AUDIO::SAMPLETYPE_FLOAT32 ), maxBlock );
}

/** Gain in dB of one channel over a range of frames, as output over input in RMS */
Float64
GainInDecibels( const TSamples& input, const TSamples& output, UInt32 channels, UInt32 channel, size_t fromFrame, size_t toFrame )
{
    return DecibelsOf( RmsOfChannel( output, channels, channel, fromFrame, toFrame ) / RmsOfChannel( input, channels, channel, fromFrame, toFrame ) );
}

/** First frame at or after 'fromFrame' where the reduction (-gain in dB of a DC signal) satisfies the test, -1 when never */
Int32
FirstFrameWhere( const TSamples& input, const TSamples& output, UInt32 channels, size_t fromFrame, Float64 threshold, bool atOrAbove )
{
    const size_t frames = output.size() / channels;
    for ( size_t f=fromFrame; f<frames; ++f )
    {
        const Float64 reduction = -DecibelsOf( (Float64) output[ f * channels ] / (Float64) input[ f * channels ] );
        if ( atOrAbove ? reduction >= threshold : reduction <= threshold )
            return (Int32) f;
    }
    return -1;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformCurveTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioCompressor: The static curve of steady sines from -60 to 0 dBFS in 3 dB steps matches the gain computer, hard and soft knee" )
    try
    {
        struct SCurve { Float32 threshold; Float32 ratio; Float32 knee; };
        const SCurve curves[] = { { -18.0f, 4.0f, 0.0f }, { -18.0f, 4.0f, 6.0f }, { -30.0f, 10.0f, 12.0f }, { -12.0f, 2.0f, 24.0f }, { -24.0f, 20.0f, 3.0f }, { -6.0f, 1.5f, 0.0f } };
        Float64 worst = 0.0;
        for ( UInt32 curve=0; curve<6; ++curve )
            for ( UInt32 step=0; step<=20; ++step )
            {
                const Float64 level = -60.0 + 3.0 * step;
                AUDIO::CCompressorEffect compressor;
                compressor.SetThresholdInDecibels( curves[ curve ].threshold );
                compressor.SetRatio( curves[ curve ].ratio );
                compressor.SetKneeInDecibels( curves[ curve ].knee );
                compressor.SetAttackInMs( 0.1f );
                compressor.SetReleaseInMs( 3000.0f );
                ASSERT_TRUE( PrepareCompressor( compressor, 2 ) );

                const TSamples input = MakeSine( 24000, 2, 1000.0, (Float32) LinearOf( level ) );
                TSamples output( input );
                Run( compressor, output, 2, 256 );

                // peak detector: a sine of this peak level is seen at that level
                const Float64 expected = -ReferenceReduction( level, curves[ curve ].threshold, curves[ curve ].ratio, curves[ curve ].knee );
                const Float64 measured = GainInDecibels( input, output, 2, 0, 19200, 24000 );
                const Float64 deviation = fabs( measured - expected );
                if ( deviation > worst ) worst = deviation;
                if ( deviation > 0.2 )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioCompressor: curve " << curve << " at " << level << " dBFS: gain " << measured << " dB, expected " << expected << " dB" );
                ASSERT_TRUE( deviation <= 0.2 );
                ASSERT_TRUE( Near( GainInDecibels( input, output, 2, 0, 19200, 24000 ), GainInDecibels( input, output, 2, 1, 19200, 24000 ), 1e-6 ) );     // linked: both channels alike
            }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioCompressor: static curves, worst deviation from the gain computer " << worst << " dB" );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioCompressor: Below the threshold minus half the knee the gain is exactly 0 dB: the audio comes out bit for bit" )
    try
    {
        const Float32 knees[] = { 0.0f, 6.0f, 24.0f };
        const Float32 thresholds[] = { -18.0f, -40.0f, -3.0f };
        for ( UInt32 k=0; k<3; ++k )
            for ( UInt32 t=0; t<3; ++t )
            {
                AUDIO::CCompressorEffect compressor;
                compressor.SetThresholdInDecibels( thresholds[ t ] );
                compressor.SetKneeInDecibels( knees[ k ] );
                compressor.SetRatio( 20.0f );
                ASSERT_TRUE( PrepareCompressor( compressor, 2 ) );

                // noise whose peak is 0.3 dB below the start of the knee
                const Float64 floorLevel = thresholds[ t ] - 0.5 * knees[ k ] - 0.3;
                const TSamples input = MakeNoise( 48000, 2, 8 + k + t, (Float32) LinearOf( floorLevel ) );
                TSamples output( input );
                Run( compressor, output, 2, 333 );
                ASSERT_TRUE( IsSame( input, output ) );
                ASSERT_TRUE( 0.0f == compressor.GetGainReductionInDecibels() );
            }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioCompressor: A ratio of 1 is transparent, whatever the level; the gain comes back to exactly 0 dB after a loud passage" )
    try
    {
        AUDIO::CCompressorEffect compressor;
        compressor.SetRatio( 1.0f );
        compressor.SetThresholdInDecibels( -40.0f );
        ASSERT_TRUE( PrepareCompressor( compressor, 2 ) );
        const TSamples loud = MakeNoise( 20000, 2, 31, 0.9f );
        TSamples output( loud );
        Run( compressor, output, 2, 256 );
        ASSERT_TRUE( IsSame( loud, output ) );

        // loud then quiet again: once the release has run out, the quiet audio is untouched
        AUDIO::CCompressorEffect recovering;
        recovering.SetThresholdInDecibels( -20.0f );
        recovering.SetReleaseInMs( 50.0f );
        ASSERT_TRUE( PrepareCompressor( recovering, 2 ) );
        TSamples burst = MakeNoise( 4800, 2, 32, 0.9f );
        Run( recovering, burst, 2, 256 );
        ASSERT_TRUE( recovering.GetGainReductionInDecibels() > 3.0f );
        const TSamples quiet = MakeNoise( 96000, 2, 33, 0.01f );
        TSamples quietOutput( quiet );
        Run( recovering, quietOutput, 2, 256 );
        for ( size_t i=quiet.size()-2000; i<quiet.size(); ++i )                   // 2 s after the burst, 40 release times
            ASSERT_TRUE( quiet[ i ] == quietOutput[ i ] );
        ASSERT_TRUE( 0.0f == recovering.GetGainReductionInDecibels() );
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

    GUCEF_TESTFW_TESTCASE( "AudioCompressor: Attack and release reach 63 percent of the way in the set time, within 10 percent" )
    try
    {
        const Float32 attacks[] = { 5.0f, 20.0f, 80.0f };
        const Float32 releases[] = { 30.0f, 100.0f, 400.0f };
        for ( UInt32 i=0; i<3; ++i )
        {
            AUDIO::CCompressorEffect compressor;
            compressor.SetThresholdInDecibels( -20.0f );
            compressor.SetRatio( 4.0f );
            compressor.SetKneeInDecibels( 0.0f );
            compressor.SetAttackInMs( attacks[ i ] );
            compressor.SetReleaseInMs( releases[ i ] );
            ASSERT_TRUE( PrepareCompressor( compressor, 2 ) );

            // a constant 0.5 (-6.02 dBFS), then a constant 0.001 (-60 dBFS)
            const UInt32 switchAt = 48000;
            TSamples input( (size_t) 48000 * 4 * 2, 0.001f );
            for ( UInt32 f=0; f<switchAt; ++f )
                input[ (size_t) f * 2 ] = input[ (size_t) f * 2 + 1 ] = 0.5f;
            TSamples output( input );
            Run( compressor, output, 2, 256 );

            const Float64 finalReduction = ReferenceReduction( DecibelsOf( 0.5 ), -20.0, 4.0, 0.0 );
            const Int32 attacked = FirstFrameWhere( input, output, 2, 0, 0.6321206 * finalReduction, true );
            ASSERT_TRUE( attacked > 0 );
            const Float64 attackMs = (Float64) ( attacked + 1 ) / 48.0;
            ASSERT_TRUE( Near( -DecibelsOf( (Float64) output[ (size_t) ( switchAt - 1 ) * 2 ] / 0.5 ), finalReduction, 0.01 ) );      // it did get there

            const Int32 released = FirstFrameWhere( input, output, 2, switchAt, 0.3678794 * finalReduction, false );
            ASSERT_TRUE( released > 0 );
            const Float64 releaseMs = (Float64) ( released + 1 - (Int32) switchAt ) / 48.0;
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioCompressor: attack " << attacks[ i ] << " ms measured " << attackMs << " ms, release " << releases[ i ] << " ms measured " << releaseMs << " ms" );
            ASSERT_TRUE( Near( attackMs, attacks[ i ], 0.1 * attacks[ i ] ) );
            ASSERT_TRUE( Near( releaseMs, releases[ i ], 0.1 * releases[ i ] ) );
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
PerformGainAndLinkTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioCompressor: Makeup gain is added to the gain reduction, and applies to audio below the threshold as well" )
    try
    {
        // below the threshold: the audio is just made louder
        AUDIO::CCompressorEffect quiet;
        quiet.SetMakeupInDecibels( 6.0f );
        ASSERT_TRUE( PrepareCompressor( quiet, 2 ) );
        const TSamples input = MakeNoise( 8000, 2, 41, 0.01f );
        TSamples output( input );
        Run( quiet, output, 2, 256 );
        const Float32 gain = (Float32) pow( 10.0, 6.0 / 20.0 );
        for ( size_t i=0; i<input.size(); ++i )
            ASSERT_TRUE( Near( output[ i ], input[ i ] * gain, 1e-6 * fabs( input[ i ] * gain ) + 1e-12 ) );

        // above it: reduction and makeup together
        AUDIO::CCompressorEffect loud;
        loud.SetThresholdInDecibels( -20.0f );
        loud.SetKneeInDecibels( 0.0f );
        loud.SetRatio( 4.0f );
        loud.SetAttackInMs( 0.1f );
        loud.SetMakeupInDecibels( 6.0f );
        ASSERT_TRUE( PrepareCompressor( loud, 2 ) );
        TSamples dc = MakeConstant( 24000, 2, 0.5f );
        TSamples dcOutput( dc );
        Run( loud, dcOutput, 2, 256 );
        const Float64 expected = 6.0 - ReferenceReduction( DecibelsOf( 0.5 ), -20.0, 4.0, 0.0 );
        ASSERT_TRUE( Near( DecibelsOf( (Float64) dcOutput[ 23999 * 2 ] / 0.5 ), expected, 0.01 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioCompressor: Stereo link makes both channels take the strongest reduction; unlinked channels do not influence each other" )
    try
    {
        TSamples input( (size_t) 24000 * 2, 0.0f );
        const TSamples loud = MakeSine( 24000, 1, 1000.0, 0.8f );
        const TSamples soft = MakeSine( 24000, 1, 1000.0, 0.05f );
        for ( UInt32 f=0; f<24000; ++f )
        {
            input[ (size_t) f * 2 ] = loud[ f ];
            input[ (size_t) f * 2 + 1 ] = soft[ f ];
        }

        Float64 reductionOfLeft = 0.0;
        const Float32 links[] = { 1.0f, 0.5f, 0.0f };
        for ( UInt32 l=0; l<3; ++l )
        {
            AUDIO::CCompressorEffect compressor;
            compressor.SetThresholdInDecibels( -18.0f );
            compressor.SetKneeInDecibels( 0.0f );
            compressor.SetAttackInMs( 0.1f );
            compressor.SetReleaseInMs( 3000.0f );
            compressor.SetStereoLink( links[ l ] );
            ASSERT_TRUE( PrepareCompressor( compressor, 2 ) );
            TSamples output( input );
            Run( compressor, output, 2, 256 );

            const Float64 left = GainInDecibels( input, output, 2, 0, 19200, 24000 );
            const Float64 right = GainInDecibels( input, output, 2, 1, 19200, 24000 );
            ASSERT_TRUE( left < -5.0 );
            if ( 0 == l )
            {
                reductionOfLeft = left;
                ASSERT_TRUE( Near( left, right, 1e-3 ) );
            }
            else
            if ( 1 == l )
                ASSERT_TRUE( Near( right, 0.5 * reductionOfLeft, 0.05 ) );
            else
            {
                // the right channel is below the threshold and alone: untouched, bit for bit
                for ( size_t f=0; f<24000; ++f )
                    ASSERT_TRUE( output[ f * 2 + 1 ] == input[ f * 2 + 1 ] );
            }
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioCompressor: A low frequency effects channel takes part in the detection of the linked channels" )
    try
    {
        // 5.1: channel 3 is the LFE channel. Loud bass there, a moderately quiet sine in channel 0
        const UInt32 channels = 6;
        TSamples input( (size_t) 24000 * channels, 0.0f );
        const TSamples bass = MakeSine( 24000, 1, 80.0, 0.9f );
        const TSamples tone = MakeSine( 24000, 1, 1000.0, 0.2f );
        for ( UInt32 f=0; f<24000; ++f )
        {
            input[ (size_t) f * channels + 3 ] = bass[ f ];
            input[ (size_t) f * channels + 0 ] = tone[ f ];
        }
        for ( UInt32 link=0; link<2; ++link )
        {
            AUDIO::CCompressorEffect compressor;
            compressor.SetThresholdInDecibels( -6.0f );
            compressor.SetKneeInDecibels( 0.0f );
            compressor.SetAttackInMs( 0.1f );
            compressor.SetReleaseInMs( 3000.0f );
            compressor.SetStereoLink( (Float32) link );
            AUDIO::CAudioFormat format( 48000, channels, AUDIO::SAMPLETYPE_FLOAT32 );
            ASSERT_TRUE( 0 != ( format.GetChannelLayout() & AUDIO::AUDIOSPEAKER_LOW_FREQUENCY ) );
            ASSERT_TRUE( compressor.Prepare( format, 1024 ) );
            TSamples output( input );
            Run( compressor, output, channels, 256 );
            const Float64 toneGain = GainInDecibels( input, output, channels, 0, 19200, 24000 );
            if ( 1 == link )
                ASSERT_TRUE( toneGain < -3.0 );         // the bass pulled the tone down
            else
                ASSERT_TRUE( Near( toneGain, 0.0, 1e-6 ) );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioCompressor: The RMS detector reads a sine 3 dB below its peak, and the window sets how fast it follows" )
    try
    {
        AUDIO::CCompressorEffect compressor;
        compressor.SetDetector( AUDIO::ENVELOPEMODE_RMS );
        compressor.SetRmsWindowInMs( 10.0f );
        compressor.SetThresholdInDecibels( -20.0f );
        compressor.SetKneeInDecibels( 0.0f );
        compressor.SetRatio( 4.0f );
        compressor.SetAttackInMs( 5.0f );
        compressor.SetReleaseInMs( 500.0f );
        ASSERT_TRUE( PrepareCompressor( compressor, 2 ) );
        const Float64 peakLevel = -4.0;
        const TSamples input = MakeSine( 48000, 2, 1000.0, (Float32) LinearOf( peakLevel ) );
        TSamples output( input );
        Run( compressor, output, 2, 256 );
        const Float64 rmsLevel = peakLevel - 3.0103;
        const Float64 expected = -ReferenceReduction( rmsLevel, -20.0, 4.0, 0.0 );
        const Float64 measured = GainInDecibels( input, output, 2, 0, 38400, 48000 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioCompressor: RMS detector, gain " << measured << " dB, expected " << expected << " dB" );
        ASSERT_TRUE( Near( measured, expected, 0.3 ) );
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

    GUCEF_TESTFW_TESTCASE( "AudioCompressor: The parameters have the ranges and defaults of the design" )
    try
    {
        AUDIO::CCompressorEffect compressor;
        ASSERT_TRUE( compressor.GetEffectTypeName() == "compressor" && 9 == compressor.GetParameterCount() );
        ASSERT_TRUE( -18.0f == compressor.GetParameter( AUDIO::CCompressorEffect::PARAMETER_THRESHOLD ) );
        ASSERT_TRUE( 4.0f == compressor.GetParameter( AUDIO::CCompressorEffect::PARAMETER_RATIO ) );
        ASSERT_TRUE( 6.0f == compressor.GetParameter( AUDIO::CCompressorEffect::PARAMETER_KNEE ) );
        ASSERT_TRUE( 10.0f == compressor.GetParameter( AUDIO::CCompressorEffect::PARAMETER_ATTACK ) );
        ASSERT_TRUE( 150.0f == compressor.GetParameter( AUDIO::CCompressorEffect::PARAMETER_RELEASE ) );
        ASSERT_TRUE( 0.0f == compressor.GetParameter( AUDIO::CCompressorEffect::PARAMETER_MAKEUP ) );
        ASSERT_TRUE( 0.0f == compressor.GetParameter( AUDIO::CCompressorEffect::PARAMETER_DETECTOR ) );
        ASSERT_TRUE( 10.0f == compressor.GetParameter( AUDIO::CCompressorEffect::PARAMETER_RMS_WINDOW ) );
        ASSERT_TRUE( 1.0f == compressor.GetParameter( AUDIO::CCompressorEffect::PARAMETER_STEREO_LINK ) );

        compressor.SetThresholdInDecibels( -90.0f );
        ASSERT_TRUE( -60.0f == compressor.GetParameter( AUDIO::CCompressorEffect::PARAMETER_THRESHOLD ) );
        compressor.SetRatio( 50.0f );
        ASSERT_TRUE( 20.0f == compressor.GetParameter( AUDIO::CCompressorEffect::PARAMETER_RATIO ) );
        compressor.SetRatio( 0.5f );
        ASSERT_TRUE( 1.0f == compressor.GetParameter( AUDIO::CCompressorEffect::PARAMETER_RATIO ) );
        compressor.SetKneeInDecibels( 100.0f );
        ASSERT_TRUE( 24.0f == compressor.GetParameter( AUDIO::CCompressorEffect::PARAMETER_KNEE ) );
        compressor.SetAttackInMs( 0.0f );
        ASSERT_TRUE( Near( compressor.GetParameter( AUDIO::CCompressorEffect::PARAMETER_ATTACK ), 0.1, 1e-6 ) );
        compressor.SetReleaseInMs( 10000.0f );
        ASSERT_TRUE( 3000.0f == compressor.GetParameter( AUDIO::CCompressorEffect::PARAMETER_RELEASE ) );
        compressor.SetMakeupInDecibels( 40.0f );
        ASSERT_TRUE( 24.0f == compressor.GetParameter( AUDIO::CCompressorEffect::PARAMETER_MAKEUP ) );
        compressor.SetStereoLink( 3.0f );
        ASSERT_TRUE( 1.0f == compressor.GetParameter( AUDIO::CCompressorEffect::PARAMETER_STEREO_LINK ) );
        ASSERT_TRUE( 0 == compressor.GetLatencyInFrames() && 0 == compressor.GetTailInFrames() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioCompressor: Changing threshold, ratio, knee and makeup while processing makes no steps in the audio" )
    try
    {
        // threshold, ratio and knee jump around every 0.5 s: the audio never steps more than the sine itself does (the gain only goes down)
        AUDIO::CCompressorEffect compressor;
        compressor.SetAttackInMs( 2.0f );
        ASSERT_TRUE( PrepareCompressor( compressor, 2 ) );
        const TSamples sine = MakeSine( 120000, 2, 1000.0, 0.7f );
        const Float32 inputStep = MaxStep( sine, 2, 0 );
        const Float32 settings[][3] = { { -6.0f, 2.0f, 0.0f }, { -40.0f, 10.0f, 12.0f }, { -12.0f, 4.0f, 6.0f }, { -50.0f, 20.0f, 0.0f }, { 0.0f, 1.0f, 24.0f } };
        TSamples output;
        for ( UInt32 round=0; round<5; ++round )
        {
            compressor.SetThresholdInDecibels( settings[ round ][ 0 ] );
            compressor.SetRatio( settings[ round ][ 1 ] );
            compressor.SetKneeInDecibels( settings[ round ][ 2 ] );
            TSamples part( sine.begin() + (size_t) round * 24000 * 2, sine.begin() + (size_t) ( round + 1 ) * 24000 * 2 );
            Run( compressor, part, 2, 256 );
            output.insert( output.end(), part.begin(), part.end() );
        }
        const Float32 outputStep = MaxStep( output, 2, 0 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioCompressor: largest step with jumping settings " << outputStep << ", input sine " << inputStep );
        ASSERT_TRUE( outputStep <= 1.05f * inputStep );

        // the makeup gain moves in a ramp over the block in which it changes: the step stays that of the new level
        AUDIO::CCompressorEffect makeup;
        makeup.SetThresholdInDecibels( 0.0f );
        ASSERT_TRUE( PrepareCompressor( makeup, 2 ) );
        TSamples quiet = MakeSine( 24000, 2, 1000.0, 0.1f );
        const Float32 quietStep = MaxStep( quiet, 2, 0 );
        TSamples first( quiet.begin(), quiet.begin() + 12000 * 2 );
        TSamples second( quiet.begin() + 12000 * 2, quiet.end() );
        Run( makeup, first, 2, 256 );
        makeup.SetMakeupInDecibels( 12.0f );
        Run( makeup, second, 2, 256 );
        first.insert( first.end(), second.begin(), second.end() );
        ASSERT_TRUE( MaxStep( first, 2, 0 ) <= 1.05f * (Float32) LinearOf( 12.0 ) * quietStep );
        ASSERT_TRUE( Near( RmsOfChannel( first, 2, 0, 20000, 24000 ) / RmsOfChannel( quiet, 2, 0, 20000, 24000 ), LinearOf( 12.0 ), 1e-4 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformAudioCompressorTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioCompressor TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.Compressor" );

    PerformCurveTests();
    PerformTimingTests();
    PerformGainAndLinkTests();
    PerformParameterTests();
}

/*-------------------------------------------------------------------------*/
