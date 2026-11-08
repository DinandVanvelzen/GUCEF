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

#ifndef GUCEF_AUDIO_CAUDIOBUSLAYOUT_H
#include "gucefAUDIO_CAudioBusLayout.h"
#define GUCEF_AUDIO_CAUDIOBUSLAYOUT_H
#endif /* GUCEF_AUDIO_CAUDIOBUSLAYOUT_H ? */

#ifndef GUCEF_AUDIO_CNOISEGATEEFFECT_H
#include "gucefAUDIO_CNoiseGateEffect.h"
#define GUCEF_AUDIO_CNOISEGATEEFFECT_H
#endif /* GUCEF_AUDIO_CNOISEGATEEFFECT_H ? */

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

#include "TestAudioNoiseGate.h"

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

typedef AUDIO::CNoiseGateEffect TGate;

const UInt32 SubBlock = GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES;

bool
PrepareGate( TGate& gate, UInt32 rate, UInt32 channels, UInt32 maxBlock = 4096 )
{
    return gate.Prepare( AUDIO::CAudioFormat( rate, channels, AUDIO::SAMPLETYPE_FLOAT32 ), maxBlock );
}

/** The gate for the level detector with the given settings, prepared */
bool
PrepareLevelGate( TGate& gate, UInt32 rate, UInt32 channels, Float32 threshold, Float32 hysteresis, Float32 range, Float32 attack, Float32 hold, Float32 release )
{
    gate.SetDetector( TGate::DETECTOR_LEVEL );
    gate.SetThresholdInDecibels( threshold );
    gate.SetHysteresisInDecibels( hysteresis );
    gate.SetRangeInDecibels( range );
    gate.SetAttackInMs( attack );
    gate.SetHoldInMs( hold );
    gate.SetReleaseInMs( release );
    return PrepareGate( gate, rate, channels );
}

Float64
Db( Float64 linear )
{
    return TVS::ToDecibels( linear );
}

/** Speech with white noise: 16 kHz, pauses of 1 to 1.5 s */
struct SMaterial
{
    TVS::SSpeech speech;
    TSamples mixed;
    SMaterial( void ) : speech(), mixed() {}
};

void
MakeMaterial( SMaterial& material, Float64 seconds, UInt64 seed, Float64 activeRmsInDecibels, Float64 noiseRmsInDecibels )
{
    TVS::SSpeechSpec spec;
    spec.sampleRate = 16000;
    spec.seconds = seconds;
    spec.seed = seed;
    spec.minPauseSeconds = 1.0;
    spec.maxPauseSeconds = 1.5;
    spec.activeRms = TVS::FromDecibels( activeRmsInDecibels );
    TVS::MakeSpeech( spec, material.speech );
    TSamples noise = TVS::MakeWhiteNoise( material.speech.samples.size(), TVS::FromDecibels( noiseRmsInDecibels ), seed + 100 );
    material.mixed = material.speech.samples;
    TVS::MixInto( material.mixed, noise );
}

/** The pauses, shortened: from 'after' seconds past the end of a talkspurt to 'before' seconds before the next, where that is at least 50 ms */
void
PauseWindows( const TVS::SSpeech& speech, Float64 after, Float64 before, TVS::TSpans& windows )
{
    const Float64 rate = (Float64) speech.sampleRate;
    UInt64 from = (UInt64) ( 0.3 * rate );
    for ( size_t i=0; i<=speech.talkspurts.size(); ++i )
    {
        const UInt64 next = i < speech.talkspurts.size() ? speech.talkspurts[ i ].start : (UInt64) speech.samples.size();
        const UInt64 start = i > 0 ? from : from;
        const UInt64 stop = next > (UInt64) ( before * rate ) ? next - (UInt64) ( before * rate ) : 0;
        if ( stop > start + (UInt64) ( 0.05 * rate ) )
            windows.push_back( TVS::SSpan( start, stop - start ) );
        if ( i < speech.talkspurts.size() )
            from = speech.talkspurts[ i ].End() + (UInt64) ( after * rate );
    }
}

Float64
EnergyIn( const TSamples& x, const TVS::TSpans& windows )
{
    Float64 sum = 0.0;
    for ( size_t w=0; w<windows.size(); ++w )
        for ( UInt64 i=windows[ w ].start; i<windows[ w ].End() && i<x.size(); ++i )
            sum += (Float64) x[ (size_t) i ] * (Float64) x[ (size_t) i ];
    return sum;
}

/** Runs the gate over mono audio in blocks of 'block' frames and records IsOpen() after each */
void
RunRecording( TGate& gate, TSamples& audio, UInt32 block, GUCEF::vector< UInt8 >& openAfterBlock )
{
    size_t done = 0;
    while ( done < audio.size() )
    {
        const size_t count = audio.size() - done < block ? audio.size() - done : block;
        ProcessRealtime( gate, &audio[ done ], (UInt32) count );
        openAfterBlock.push_back( gate.IsOpen() ? 1 : 0 );
        done += count;
    }
}

/** The number of changes of the open flag */
UInt32
CountTransitions( const GUCEF::vector< UInt8 >& flags, size_t from )
{
    UInt32 count = 0;
    for ( size_t i=from+1; i<flags.size(); ++i )
        if ( flags[ i ] != flags[ i - 1 ] )
            ++count;
    return count;
}

/** First frame at or after 'from' where the gain is above (or below) 'level' dB, -1 when never */
Int64
Crossing( const GUCEF::vector< Float64 >& gainDb, size_t from, Float64 level, bool above )
{
    for ( size_t f=from; f<gainDb.size(); ++f )
        if ( above ? gainDb[ f ] > level : gainDb[ f ] < level )
            return (Int64) f;
    return -1;
}

/**
 *  A DC signal that is 'quiet' until 'onFrame', 'loud' until 'offFrame' and 'quiet' after that, through a level gate; the gain
 *  per frame in dB is the output over the input
 */
void
RunStep( TGate& gate, Float32 quiet, Float32 loud, size_t onFrame, size_t offFrame, size_t total, GUCEF::vector< Float64 >& gainDb )
{
    TSamples in( total, quiet );
    for ( size_t f=onFrame; f<offFrame; ++f )
        in[ f ] = loud;
    TSamples out( in );
    Run( gate, out, 1, 256 );
    gainDb.assign( total, 0.0 );
    for ( size_t f=0; f<total; ++f )
        gainDb[ f ] = DecibelsOf( (Float64) out[ f ] / (Float64) in[ f ] );
}

// ---- the engine side -------------------------------------------------------------------------------------------------

AUDIO::CAudioEngineParams
NullParams( void )
{
    AUDIO::CAudioEngineParams params;
    params.SetDriverName( "null" );
    params.SetThreadingMode( AUDIO::AUDIOTHREADING_COOPERATIVE );
    params.SetLimiterEnabled( false );
    params.GetDeviceParams().SetBufferSizeInFrames( 256 );
    return params;
}

AUDIO::CNullAudioDevice*
NullDevice( AUDIO::CAudioEngine& engine )
{
    return engine.GetDevice()->AsNullDevice();
}

void
RenderBlocks( AUDIO::CAudioEngine& engine, UInt32 blocks, UInt32 blockFrames )
{
    for ( UInt32 i=0; i<blocks; ++i )
        NullDevice( engine )->Render( blockFrames );
}

bool
RenderUntilReady( AUDIO::CAudioEngine& engine, const CORE::CFutureResult& future, UInt32 maxBlocks = 20 )
{
    for ( UInt32 i=0; i<maxBlocks; ++i )
    {
        if ( future.IsReady() )
            return true;
        NullDevice( engine )->Render( 256 );
        engine.Update();
    }
    return future.IsReady();
}

AUDIO::TAudioClipPtr
MakeConstantClip( Float32 value )
{
    AUDIO::CAudioBuffer audio( AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 ), 480000 );
    for ( UInt32 f=0; f<480000; ++f )
    {
        audio.SetSampleFromFloat( f, 0, value );
        audio.SetSampleFromFloat( f, 1, value );
    }
    return AUDIO::TAudioClipPtr( GUCEF_NEW AUDIO::CAudioClip( audio ) );
}

Float32
PeakOfBus( const AUDIO::CAudioBus* bus )
{
    const Float32 left = bus->GetMeter().GetPeak( 0 );
    const Float32 right = bus->GetMeter().GetPeak( 1 );
    return left > right ? left : right;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformParameterTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioNoiseGate: Type name, parameter descriptors, defaults, limits, voice detector options, no latency or tail, registered as built-in type" )
    try
    {
        TGate gate;
        ASSERT_TRUE( gate.GetEffectTypeName() == "noiseGate" && TGate::GetStaticEffectTypeName() == "noiseGate" );
        ASSERT_TRUE( 8 == gate.GetParameterCount() && 8 == TGate::GetStaticParameterCount() );
        ASSERT_TRUE( (Float32) TGate::DETECTOR_VOICE == gate.GetParameter( TGate::PARAMETER_DETECTOR ) );
        ASSERT_TRUE( -50.0f == gate.GetParameter( TGate::PARAMETER_THRESHOLD ) );
        ASSERT_TRUE( 4.0f == gate.GetParameter( TGate::PARAMETER_HYSTERESIS ) );
        ASSERT_TRUE( -30.0f == gate.GetParameter( TGate::PARAMETER_RANGE ) );
        ASSERT_TRUE( 2.0f == gate.GetParameter( TGate::PARAMETER_ATTACK ) );
        ASSERT_TRUE( 150.0f == gate.GetParameter( TGate::PARAMETER_HOLD ) );
        ASSERT_TRUE( 150.0f == gate.GetParameter( TGate::PARAMETER_RELEASE ) );
        ASSERT_TRUE( (Float32) TGate::LEVELSOURCE_RMS == gate.GetParameter( TGate::PARAMETER_LEVEL_SOURCE ) );

        UInt32 id = 0;
        ASSERT_TRUE( gate.TryGetParameterId( "levelSource", id ) && TGate::PARAMETER_LEVEL_SOURCE == id );
        ASSERT_TRUE( gate.TryGetParameterId( "HOLD", id ) && TGate::PARAMETER_HOLD == id );
        ASSERT_FALSE( gate.TryGetParameterId( "vadOnsetDb", id ) );                 // an option, not a parameter
        const AUDIO::CAudioEffectParameterInfo* detector = gate.GetParameterInfo( TGate::PARAMETER_DETECTOR );
        ASSERT_TRUE( GUCEF_NULL != detector && AUDIO::AUDIOPARAMKIND_ENUMERATION == detector->GetKind() );
        ASSERT_TRUE( detector->GetEnumLabel( 0 ) == "level" && detector->GetEnumLabel( 1 ) == "voice" );

        gate.SetThresholdInDecibels( -120.0f );
        ASSERT_TRUE( -90.0f == gate.GetParameter( TGate::PARAMETER_THRESHOLD ) );
        gate.SetThresholdInDecibels( 9.0f );
        ASSERT_TRUE( 0.0f == gate.GetParameter( TGate::PARAMETER_THRESHOLD ) );
        gate.SetHysteresisInDecibels( -1.0f );
        ASSERT_TRUE( 0.0f == gate.GetParameter( TGate::PARAMETER_HYSTERESIS ) );
        gate.SetHysteresisInDecibels( 40.0f );
        ASSERT_TRUE( 12.0f == gate.GetParameter( TGate::PARAMETER_HYSTERESIS ) );
        gate.SetRangeInDecibels( -200.0f );
        ASSERT_TRUE( -80.0f == gate.GetParameter( TGate::PARAMETER_RANGE ) );
        gate.SetRangeInDecibels( 5.0f );
        ASSERT_TRUE( 0.0f == gate.GetParameter( TGate::PARAMETER_RANGE ) );
        gate.SetAttackInMs( 0.0f );
        ASSERT_TRUE( 0.1f == gate.GetParameter( TGate::PARAMETER_ATTACK ) );
        gate.SetAttackInMs( 900.0f );
        ASSERT_TRUE( 50.0f == gate.GetParameter( TGate::PARAMETER_ATTACK ) );
        gate.SetHoldInMs( -3.0f );
        ASSERT_TRUE( 0.0f == gate.GetParameter( TGate::PARAMETER_HOLD ) );
        gate.SetHoldInMs( 99999.0f );
        ASSERT_TRUE( 1000.0f == gate.GetParameter( TGate::PARAMETER_HOLD ) );
        gate.SetReleaseInMs( 1.0f );
        ASSERT_TRUE( 10.0f == gate.GetParameter( TGate::PARAMETER_RELEASE ) );
        gate.SetReleaseInMs( 99999.0f );
        ASSERT_TRUE( 2000.0f == gate.GetParameter( TGate::PARAMETER_RELEASE ) );
        gate.SetDetector( TGate::DETECTOR_LEVEL );
        ASSERT_TRUE( 0.0f == gate.GetParameter( TGate::PARAMETER_DETECTOR ) );
        gate.SetLevelSource( TGate::LEVELSOURCE_PEAK );
        ASSERT_TRUE( 0.0f == gate.GetParameter( TGate::PARAMETER_LEVEL_SOURCE ) );

        ASSERT_TRUE( 0 == gate.GetLatencyInFrames() && 0 == gate.GetTailInFrames() );

        // the voice detector settings are construction options, written where they differ from the defaults
        CORE::CValueList noOptions;
        gate.GetConstructionOptions( noOptions );
        ASSERT_TRUE( 0 == noOptions.GetKeyCount() );
        CORE::CValueList options;
        options.Set( "vadOnsetDb", 12.0f );
        options.Set( "vadHangoverMs", 300.0f );
        TGate tuned( options );
        ASSERT_TRUE( 12.0f == tuned.GetVoiceActivitySettings().GetOnsetInDecibels() );
        ASSERT_TRUE( 300.0f == tuned.GetVoiceActivitySettings().GetHangoverInMs() );
        ASSERT_TRUE( AUDIO::CVoiceActivitySettings::DefaultMinimumLevelInDecibels == tuned.GetVoiceActivitySettings().GetMinimumLevelInDecibels() );
        CORE::CValueList reported;
        tuned.GetConstructionOptions( reported );
        ASSERT_TRUE( 2 == reported.GetKeyCount() );
        ASSERT_TRUE( 12.0f == reported.GetValueAlways( "vadOnsetDb" ).AsFloat32( 0.0f ) );
        ASSERT_TRUE( 300.0f == reported.GetValueAlways( "vadHangoverMs" ).AsFloat32( 0.0f ) );
        AUDIO::CVoiceActivitySettings settings;
        settings.SetReleaseInDecibels( 3.0f );
        gate.SetVoiceActivitySettings( settings );
        ASSERT_TRUE( settings == gate.GetVoiceActivitySettings() );

        // built in, made by the registry (case does not matter) like the others
        AUDIO::TAudioEffectPtr made = AUDIO::CAudioGlobal::Instance()->GetAudioEffectRegistry().CreateEffect( "noisegate", options );
        ASSERT_FALSE( made.IsNULL() );
        ASSERT_TRUE( made->GetEffectTypeName() == "noiseGate" && 8 == made->GetParameterCount() );
        ASSERT_TRUE( 12.0f == made.StaticCast< TGate >()->GetVoiceActivitySettings().GetOnsetInDecibels() );

        // unprepared the audio passes
        TSamples audio = MakeNoise( 1000, 2, 3, 0.5f );
        const TSamples original( audio );
        TGate unprepared;
        unprepared.Process( &audio[ 0 ], 1000 );
        ASSERT_TRUE( IsSame( audio, original ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformLevelTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioNoiseGate: Level detector: above the threshold the audio passes bit for bit, below it the level is lowered by the range, a range of 0 dB does nothing, peak and RMS sources" )
    try
    {
        const Float64 rms[ 2 ] = { -20.0, -70.0 };
        const Float32 ranges[ 4 ] = { -30.0f, -12.0f, -60.0f, 0.0f };
        for ( UInt32 source=0; source<2; ++source )
        {
            for ( UInt32 r=0; r<4; ++r )
            {
                TGate gate;
                gate.SetLevelSource( 0 == source ? TGate::LEVELSOURCE_RMS : TGate::LEVELSOURCE_PEAK );
                ASSERT_TRUE( PrepareLevelGate( gate, 48000, 2, -50.0f, 4.0f, ranges[ r ], 2.0f, 150.0f, 150.0f ) );

                // loud: open after the first moments, from then on the audio is identical
                TSamples loud = MakeSine( 48000, 2, 997.0, (Float32) ( LinearOf( rms[ 0 ] ) * 1.4142135623731 ) );
                const TSamples loudIn( loud );
                Run( gate, loud, 2, 256 );
                ASSERT_TRUE( gate.IsOpen() );
                ASSERT_TRUE( 0.0f == gate.GetGainInDecibels() );
                ASSERT_TRUE( 0 == memcmp( &loud[ 24000 * 2 ], &loudIn[ 24000 * 2 ], 24000 * 2 * sizeof( Float32 ) ) );

                // quiet: closed, the level is lowered by the range
                TGate quietGate;
                quietGate.SetLevelSource( 0 == source ? TGate::LEVELSOURCE_RMS : TGate::LEVELSOURCE_PEAK );
                ASSERT_TRUE( PrepareLevelGate( quietGate, 48000, 2, -50.0f, 4.0f, ranges[ r ], 2.0f, 150.0f, 150.0f ) );
                TSamples quiet = MakeSine( 48000, 2, 997.0, (Float32) ( LinearOf( rms[ 1 ] ) * 1.4142135623731 ) );
                const TSamples quietIn( quiet );
                Run( quietGate, quiet, 2, 256 );
                ASSERT_FALSE( quietGate.IsOpen() );
                if ( 0.0f == ranges[ r ] )
                {
                    ASSERT_TRUE( IsSame( quiet, quietIn ) );
                }
                else
                {
                    const Float64 inLevel = DecibelsOf( RmsOfChannel( quietIn, 2, 1, 24000, 48000 ) );
                    const Float64 outLevel = DecibelsOf( RmsOfChannel( quiet, 2, 1, 24000, 48000 ) );
                    ASSERT_TRUE( Near( outLevel - inLevel, (Float64) ranges[ r ], 0.05 ) );
                    ASSERT_TRUE( Near( (Float64) quietGate.GetGainInDecibels(), (Float64) ranges[ r ], 0.01 ) );
                }
            }
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioNoiseGate: Threshold and hysteresis: a slowly rising level opens the gate at the threshold, a falling one closes it at the threshold minus the hysteresis" )
    try
    {
        const Float32 hysteresis[ 3 ] = { 4.0f, 8.0f, 1.0f };
        const Float32 thresholds[ 3 ] = { -50.0f, -40.0f, -60.0f };
        for ( UInt32 variant=0; variant<3; ++variant )
        {
            TGate gate;
            ASSERT_TRUE( PrepareLevelGate( gate, 48000, 1, thresholds[ variant ], hysteresis[ variant ], -30.0f, 2.0f, 0.0f, 100.0f ) );

            // 10 dB per second up from -80 to -20 dB and down again, a sine of 1 kHz
            const Float64 start = -80.0, peakLevel = -20.0, rate = 10.0;
            const size_t halfFrames = (size_t) ( ( peakLevel - start ) / rate * 48000.0 );
            TSamples audio( halfFrames * 2, 0.0f );
            for ( size_t f=0; f<audio.size(); ++f )
            {
                const Float64 t = (Float64) f / 48000.0;
                const Float64 level = f < halfFrames ? start + rate * t : peakLevel - rate * ( t - (Float64) halfFrames / 48000.0 );
                audio[ f ] = (Float32) ( LinearOf( level ) * 1.4142135623731 * sin( 2.0 * Pi * 1000.0 * t ) );
            }
            Float64 openedAt = 1000.0, closedAt = 1000.0;
            bool wasOpen = false;
            for ( size_t done=0; done+SubBlock<=audio.size(); done+=SubBlock )
            {
                ProcessRealtime( gate, &audio[ done ], SubBlock );
                const Float64 t = (Float64) ( done + SubBlock ) / 48000.0;
                const Float64 level = done < halfFrames ? start + rate * t : peakLevel - rate * ( t - (Float64) halfFrames / 48000.0 );
                if ( gate.IsOpen() && !wasOpen && done < halfFrames )
                    openedAt = level;
                if ( !gate.IsOpen() && wasOpen && done >= halfFrames )
                    closedAt = level;
                wasOpen = gate.IsOpen();
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioNoiseGate: threshold " << thresholds[ variant ] << " dB, hysteresis " << hysteresis[ variant ] << " dB: opens at " << openedAt << " dB, closes at " << closedAt << " dB" );
            ASSERT_TRUE( Near( openedAt, (Float64) thresholds[ variant ], 0.5 ) );
            ASSERT_TRUE( Near( closedAt, (Float64) ( thresholds[ variant ] - hysteresis[ variant ] ), 0.5 ) );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioNoiseGate: Noise at the threshold does not make the gate chatter with the hysteresis, and does without it" )
    try
    {
        UInt32 changes[ 2 ] = { 0, 0 };
        const Float32 hysteresis[ 2 ] = { 4.0f, 0.0f };
        for ( UInt32 v=0; v<2; ++v )
        {
            TGate gate;
            ASSERT_TRUE( PrepareLevelGate( gate, 48000, 1, -50.0f, hysteresis[ v ], -30.0f, 2.0f, 0.0f, 20.0f ) );
            TSamples noise = TVS::MakeWhiteNoise( 480000, TVS::FromDecibels( -50.0 ), 11 );
            GUCEF::vector< UInt8 > flags;
            RunRecording( gate, noise, SubBlock, flags );
            changes[ v ] = CountTransitions( flags, 100 );
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioNoiseGate: changes of state in 10 s of noise at the threshold: " << changes[ 0 ] << " with hysteresis 4 dB, " << changes[ 1 ] << " without" );
        ASSERT_TRUE( changes[ 0 ] <= 2 );
        ASSERT_TRUE( changes[ 1 ] > 20 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioNoiseGate: Attack, hold and release take the time of the settings within 10 %" )
    try
    {
        struct SCase { Float32 attack; Float32 release; Float32 hold; };
        const SCase cases[ 4 ] = { { 5.0f, 50.0f, 40.0f }, { 20.0f, 100.0f, 100.0f }, { 40.0f, 400.0f, 250.0f }, { 1.0f, 30.0f, 0.0f } };
        const size_t onFrame = 24000, offFrame = 48000, total = 144000;
        const Float32 quiet = 0.0001f, loud = 0.0316f;                                  // DC: -80 and -30 dB
        for ( UInt32 c=0; c<4; ++c )
        {
            // threshold -40, hysteresis 4: the level detector (10 ms) needs about 32 ms to fall from -30 to -44 dB
            TGate withHold, noHold;
            ASSERT_TRUE( PrepareLevelGate( withHold, 48000, 1, -40.0f, 4.0f, -30.0f, cases[ c ].attack, cases[ c ].hold, cases[ c ].release ) );
            ASSERT_TRUE( PrepareLevelGate( noHold, 48000, 1, -40.0f, 4.0f, -30.0f, cases[ c ].attack, 0.0f, cases[ c ].release ) );
            GUCEF::vector< Float64 > gainWith, gainNo;
            RunStep( withHold, quiet, loud, onFrame, offFrame, total, gainWith );
            RunStep( noHold, quiet, loud, onFrame, offFrame, total, gainNo );

            // closed before the loud part
            ASSERT_TRUE( Near( gainWith[ onFrame - 1 ], -30.0, 0.01 ) );

            // attack: 10 % to 90 % of the range takes 0.8 of the attack time
            const Int64 up10 = Crossing( gainWith, onFrame, -27.0, true );
            const Int64 up90 = Crossing( gainWith, onFrame, -3.0, true );
            ASSERT_TRUE( up10 > 0 && up90 > up10 );
            const Float64 attackMeasured = (Float64) ( up90 - up10 ) / 48.0 / 0.8;
            const Float64 openDelay = (Float64) ( up10 - (Int64) onFrame ) / 48.0;

            // release: 10 % to 90 % takes 0.8 of the release time
            const Int64 down10 = Crossing( gainWith, offFrame, -3.0, false );
            const Int64 down90 = Crossing( gainWith, offFrame, -27.0, false );
            ASSERT_TRUE( down10 > 0 && down90 > down10 );
            const Float64 releaseMeasured = (Float64) ( down90 - down10 ) / 48.0 / 0.8;

            // hold: the difference between the same gate with and without hold
            const Int64 noHold10 = Crossing( gainNo, offFrame, -3.0, false );
            ASSERT_TRUE( noHold10 > 0 );
            const Float64 holdMeasured = (Float64) ( down10 - noHold10 ) / 48.0;
            const Float64 detectionDelay = (Float64) ( noHold10 - (Int64) offFrame ) / 48.0 - 0.1 * (Float64) cases[ c ].release;

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioNoiseGate: attack " << cases[ c ].attack << " ms measured " << attackMeasured << " ms (opens after " << openDelay << " ms), release " <<
                       cases[ c ].release << " ms measured " << releaseMeasured << " ms, hold " << cases[ c ].hold << " ms measured " << holdMeasured << " ms, detector delay when closing " << detectionDelay << " ms" );
            const Float64 oneStep = 1.5;                                               // a sub-block is 0.67 ms, the 10 % and 90 % points each lose up to one
            ASSERT_TRUE( Near( attackMeasured, (Float64) cases[ c ].attack, 0.1 * cases[ c ].attack + oneStep ) );
            ASSERT_TRUE( Near( releaseMeasured, (Float64) cases[ c ].release, 0.1 * cases[ c ].release + oneStep ) );
            ASSERT_TRUE( Near( holdMeasured, (Float64) cases[ c ].hold, 0.1 * cases[ c ].hold + oneStep ) );
            ASSERT_TRUE( openDelay <= 0.1 * cases[ c ].attack + 2.0 );
            ASSERT_TRUE( Near( detectionDelay, 10.0 * log( 1.0e-3 / pow( 10.0, -4.4 ) ), 2.5 ) );   // 10 ms time constant from -30 to -44 dB
            ASSERT_TRUE( Near( gainWith[ total - 1 ], -30.0, 0.01 ) );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioNoiseGate: Gain changes are click free: the output of gated tone bursts never steps more than the tone itself" )
    try
    {
        TGate gate;
        ASSERT_TRUE( PrepareLevelGate( gate, 48000, 1, -50.0f, 4.0f, -30.0f, 2.0f, 50.0f, 150.0f ) );

        // 1 kHz bursts of -20 dB (0.2 s) alternating with -80 dB, switched at zero crossings so the input has no step
        const size_t burst = 9600;
        TSamples audio( burst * 12, 0.0f );
        for ( size_t f=0; f<audio.size(); ++f )
        {
            const Float64 amplitude = 0 == ( f / burst ) % 2 ? LinearOf( -80.0 ) : LinearOf( -17.0 );
            audio[ f ] = (Float32) ( amplitude * sin( 2.0 * Pi * 1000.0 * (Float64) f / 48000.0 ) );
        }
        const Float32 inputStep = MaxStep( audio, 1, 0 );
        Run( gate, audio, 1, 256 );
        const Float32 outputStep = MaxStep( audio, 1, 0 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioNoiseGate: largest step of the input " << inputStep << ", of the gated output " << outputStep );
        ASSERT_TRUE( outputStep <= 1.3f * inputStep );

        // a range that is changed on the fly does not step either
        TGate sweep;
        ASSERT_TRUE( PrepareLevelGate( sweep, 48000, 1, -10.0f, 4.0f, -6.0f, 2.0f, 50.0f, 150.0f ) );   // closed for a tone at -20 dB
        TSamples tone = MakeSine( 96000, 1, 1000.0, 0.1f );
        const Float32 toneStep = MaxStep( tone, 1, 0 );
        for ( UInt32 block=0; block<750; ++block )
        {
            sweep.SetRangeInDecibels( -6.0f - 60.0f * (Float32) ( 0 == ( block / 150 ) % 2 ? block % 150 : 150 - block % 150 ) / 150.0f );
            ProcessRealtime( sweep, &tone[ (size_t) block * 128 ], 128 );
        }
        ASSERT_TRUE( MaxStep( tone, 1, 0 ) <= 1.3f * toneStep );
        ASSERT_TRUE( AllFiniteAndBelow( tone, 0.11f ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformVoiceTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioNoiseGate: Voice detector mode: speech passes, the noise in the pauses is lowered by the range, noise alone keeps the gate closed whatever its level" )
    try
    {
        SMaterial material;
        MakeMaterial( material, 30.0, 21, -20.0, -50.0 );                                  // SNR 30 dB
        TGate gate;
        gate.SetHoldInMs( 50.0f );
        gate.SetReleaseInMs( 50.0f );
        ASSERT_TRUE( PrepareGate( gate, 16000, 1 ) );
        TSamples out( material.mixed );
        GUCEF::vector< UInt8 > flags;
        RunRecording( gate, out, SubBlock, flags );

        // the pauses, once the voice detector, the hold time and the release are over: lowered by the range
        TVS::TSpans windows;
        PauseWindows( material.speech, 0.45, 0.05, windows );
        ASSERT_TRUE( windows.size() >= 10 );
        const Float64 reduction = 10.0 * log10( EnergyIn( out, windows ) / EnergyIn( material.mixed, windows ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioNoiseGate: noise in the pauses lowered by " << -reduction << " dB (range 30 dB), " << windows.size() << " pause windows" );
        ASSERT_TRUE( Near( reduction, -30.0, 0.5 ) );

        // the speech inside the talkspurts passes unchanged (bit for bit) once the first 200 ms are over and until the last 200 ms
        size_t same = 0, total = 0;
        Float64 inEnergy = 0.0, outEnergy = 0.0;
        for ( size_t t=0; t<material.speech.talkspurts.size(); ++t )
        {
            const TVS::SSpan& spurt = material.speech.talkspurts[ t ];
            for ( UInt64 i=spurt.start+3200; i+3200<spurt.End(); ++i )
            {
                ++total;
                if ( out[ (size_t) i ] == material.mixed[ (size_t) i ] )
                    ++same;
            }
            for ( UInt64 i=spurt.start; i<spurt.End(); ++i )
            {
                if ( 0 != material.speech.active[ (size_t) i ] )
                {
                    inEnergy += (Float64) material.mixed[ (size_t) i ] * (Float64) material.mixed[ (size_t) i ];
                    outEnergy += (Float64) out[ (size_t) i ] * (Float64) out[ (size_t) i ];
                }
            }
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioNoiseGate: " << 100.0 * (Float64) same / (Float64) total << " % of the samples inside the talkspurts are unchanged, " << 100.0 * outEnergy / inEnergy << " % of the active speech energy is kept" );
        ASSERT_TRUE( (Float64) same >= 0.99 * (Float64) total );
        ASSERT_TRUE( outEnergy >= 0.95 * inEnergy );

        // noise alone, louder than the level threshold: the voice gate stays closed, the level gate opens
        TSamples noise = TVS::MakeWhiteNoise( 16000 * 20, TVS::FromDecibels( -40.0 ), 5 );
        TGate voiceGate;
        ASSERT_TRUE( PrepareGate( voiceGate, 16000, 1 ) );
        TSamples voiceOut( noise );
        GUCEF::vector< UInt8 > voiceFlags;
        RunRecording( voiceGate, voiceOut, SubBlock, voiceFlags );
        size_t opened = 0;
        for ( size_t i=voiceFlags.size()/20; i<voiceFlags.size(); ++i )
            opened += voiceFlags[ i ];
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioNoiseGate: voice mode on noise at -40 dB: open in " << 100.0 * (Float64) opened / (Float64) voiceFlags.size() << " % of the blocks" );
        ASSERT_TRUE( opened * 100 < voiceFlags.size() );
        ASSERT_TRUE( Near( (Float64) voiceGate.GetGainInDecibels(), -30.0, 0.01 ) );

        TGate levelGate;
        levelGate.SetDetector( TGate::DETECTOR_LEVEL );
        ASSERT_TRUE( PrepareGate( levelGate, 16000, 1 ) );
        TSamples levelOut( noise );
        Run( levelGate, levelOut, 1, 256 );
        ASSERT_TRUE( levelGate.IsOpen() );

        // the detector that cannot be made (rate below 8 kHz) falls back to the level detector
        TGate low;
        ASSERT_TRUE( PrepareGate( low, 4000, 1 ) );
        TSamples lowNoise = TVS::MakeWhiteNoise( 40000, TVS::FromDecibels( -40.0 ), 5 );
        Run( low, lowNoise, 1, 256 );
        ASSERT_TRUE( low.IsOpen() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformInvarianceAndRealtimeTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioNoiseGate: The output does not depend on the block size (bit for bit): voice detector on speech with noise, level detector with peak and RMS on stereo bursts" )
    try
    {
        SMaterial material;
        MakeMaterial( material, 20.0, 33, -25.0, -50.0 );

        // stereo bursts of noise at different levels
        TSamples stereo( 48000 * 6 * 2, 0.0f );
        CXorShift random( 41 );
        for ( size_t f=0; f<48000 * 6; ++f )
        {
            const Float32 level = 0 == ( f / 12000 ) % 2 ? 0.2f : 0.001f;
            stereo[ f * 2 ] = level * random.NextFloat();
            stereo[ f * 2 + 1 ] = 0.5f * level * random.NextFloat();
        }

        const UInt32 blocks[ 6 ] = { 1, 7, 33, 100, 1024, 4096 };
        for ( UInt32 variant=0; variant<3; ++variant )
        {
            const bool voice = 0 == variant;
            const UInt32 channels = voice ? 1 : 2;
            const UInt32 rate = voice ? 16000 : 48000;
            const TSamples& input = voice ? material.mixed : stereo;

            TGate reference;
            reference.SetDetector( voice ? TGate::DETECTOR_VOICE : TGate::DETECTOR_LEVEL );
            reference.SetLevelSource( 1 == variant ? TGate::LEVELSOURCE_PEAK : TGate::LEVELSOURCE_RMS );
            reference.SetThresholdInDecibels( -40.0f );
            ASSERT_TRUE( PrepareGate( reference, rate, channels ) );
            TSamples expected( input );
            Run( reference, expected, channels, SubBlock );
            ASSERT_FALSE( IsSame( expected, input ) );                                  // the gate did something

            for ( UInt32 b=0; b<6; ++b )
            {
                TGate gate;
                gate.SetDetector( voice ? TGate::DETECTOR_VOICE : TGate::DETECTOR_LEVEL );
                gate.SetLevelSource( 1 == variant ? TGate::LEVELSOURCE_PEAK : TGate::LEVELSOURCE_RMS );
                gate.SetThresholdInDecibels( -40.0f );
                ASSERT_TRUE( PrepareGate( gate, rate, channels ) );
                TSamples actual( input );
                Run( gate, actual, channels, blocks[ b ] );
                ASSERT_TRUE( IsSame( actual, expected ) );
            }
            TGate random1;
            random1.SetDetector( voice ? TGate::DETECTOR_VOICE : TGate::DETECTOR_LEVEL );
            random1.SetLevelSource( 1 == variant ? TGate::LEVELSOURCE_PEAK : TGate::LEVELSOURCE_RMS );
            random1.SetThresholdInDecibels( -40.0f );
            ASSERT_TRUE( PrepareGate( random1, rate, channels ) );
            TSamples randomOut( input );
            RunInRandomBlocks( random1, randomOut, channels, 500, 77 );
            ASSERT_TRUE( IsSame( randomOut, expected ) );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioNoiseGate: Reset returns to the closed state, long silence leaves no denormals, a block with not-a-number does not poison the state" )
    try
    {
        TGate gate;
        ASSERT_TRUE( PrepareLevelGate( gate, 48000, 1, -50.0f, 4.0f, -30.0f, 2.0f, 50.0f, 150.0f ) );
        TSamples loud = MakeSine( 24000, 1, 500.0, 0.3f );
        TSamples silence( 48000 * 20, 0.0f );
        Run( gate, loud, 1, 128 );
        ASSERT_TRUE( gate.IsOpen() );
        Run( gate, silence, 1, 128 );
        ASSERT_FALSE( gate.IsOpen() );
        ASSERT_TRUE( 0 == CountSubnormal( silence ) );
        ASSERT_TRUE( AllFiniteAndBelow( silence, 0.0f ) );

        // very quiet, nearly subnormal audio
        TSamples tiny( 48000, 1.0e-30f );
        Run( gate, tiny, 1, 128 );
        ASSERT_TRUE( 0 == CountSubnormal( tiny ) );

        TSamples again = MakeSine( 24000, 1, 500.0, 0.3f );
        Run( gate, again, 1, 128 );
        ASSERT_TRUE( gate.IsOpen() );
        ResetRealtime( gate );
        ASSERT_FALSE( gate.IsOpen() );
        ASSERT_TRUE( 0.0f == gate.GetGainInDecibels() );
        ProcessRealtime( gate, &again[ 0 ], 32 );
        ASSERT_TRUE( gate.GetGainInDecibels() <= 0.0f );

        // not a number in the input spoils the output of that block, not the next
        TSamples poisoned = MakeSine( 4800, 1, 500.0, 0.3f );
        poisoned[ 100 ] = nanf( "" );
        poisoned[ 101 ] = INFINITY;
        TGate victim;
        ASSERT_TRUE( PrepareLevelGate( victim, 48000, 1, -50.0f, 4.0f, -30.0f, 2.0f, 50.0f, 150.0f ) );
        Run( victim, poisoned, 1, 64 );
        TSamples clean = MakeSine( 24000, 1, 500.0, 0.3f );
        Run( victim, clean, 1, 64 );
        ASSERT_TRUE( victim.IsOpen() );
        ASSERT_TRUE( AllFiniteAndBelow( clean, 0.31f ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioNoiseGate: Settings saved and loaded: parameters round trip by name, the voice detector settings are construction options" )
    try
    {
        CORE::CValueList options;
        options.Set( "vadOnsetDb", 14.0f );
        TGate original( options );
        original.SetDetector( TGate::DETECTOR_LEVEL );
        original.SetThresholdInDecibels( -61.0f );
        original.SetHysteresisInDecibels( 7.0f );
        original.SetRangeInDecibels( -44.0f );
        original.SetAttackInMs( 3.5f );
        original.SetHoldInMs( 420.0f );
        original.SetReleaseInMs( 880.0f );
        original.SetLevelSource( TGate::LEVELSOURCE_PEAK );
        CORE::CDataNode config;
        ASSERT_TRUE( original.SaveConfig( config ) );
        const CORE::CDataNode* parameters = config.FindChild( "Parameters" );
        ASSERT_TRUE( GUCEF_NULL != parameters );
        ASSERT_TRUE( parameters->GetAttributeValue( "detector" ).AsString() == "level" );
        ASSERT_TRUE( parameters->GetAttributeValue( "levelSource" ).AsString() == "peak" );
        const CORE::CDataNode* optionsNode = config.FindChild( "Options" );
        ASSERT_TRUE( GUCEF_NULL != optionsNode && optionsNode->GetAttributeValue( "vadOnsetDb" ).AsFloat32( 0.0f ) == 14.0f );

        CORE::CValueList restored;
        original.GetConstructionOptions( restored );
        TGate loaded( restored );
        ASSERT_TRUE( loaded.LoadConfig( config ) );
        for ( UInt32 p=0; p<TGate::PARAMETER_COUNT; ++p )
            ASSERT_TRUE( loaded.GetParameter( p ) == original.GetParameter( p ) );
        ASSERT_TRUE( 14.0f == loaded.GetVoiceActivitySettings().GetOnsetInDecibels() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioNoiseGate: Real time safety: processing in both detector modes with parameter changes and a reset allocates, locks and blocks nowhere (RealtimeSanitizer builds)" )
    try
    {
        SMaterial material;
        MakeMaterial( material, 12.0, 44, -22.0, -48.0 );
        TGate gate;
        gate.SetAttackInMs( 3.0f );
        gate.SetHoldInMs( 20.0f );
        gate.SetReleaseInMs( 30.0f );
        ASSERT_TRUE( PrepareGate( gate, 16000, 1 ) );
        TSamples audio( material.mixed );
        size_t at = 0;
        Float32 deepest = 0.0f;
        for ( UInt32 round=0; round<2000 && at + SubBlock * 8 < audio.size(); ++round )
        {
            gate.SetParameter( TGate::PARAMETER_DETECTOR, (Float32) ( ( round / 100 ) % 2 ) );
            gate.SetParameter( TGate::PARAMETER_LEVEL_SOURCE, (Float32) ( round % 2 ) );
            gate.SetParameter( TGate::PARAMETER_RANGE, -6.0f - (Float32) ( round % 40 ) );
            gate.SetParameter( TGate::PARAMETER_THRESHOLD, -60.0f + (Float32) ( round % 20 ) );
            if ( 777 == round )
                ResetRealtime( gate );
            const UInt32 frames = SubBlock * ( 1 + round % 7 ) + round % 5;
            ProcessRealtime( gate, &audio[ at ], frames );
            at += frames;
            if ( gate.GetGainInDecibels() < deepest )
                deepest = gate.GetGainInDecibels();
        }
        ASSERT_TRUE( deepest < -5.0f );
        ASSERT_TRUE( AllFiniteAndBelow( audio, 2.0f ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformEngineTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioNoiseGate: In a bus layout: parameters and voice detector options round trip through the layout and the engine, the gate gates the bus" )
    try
    {
        AUDIO::CAudioBusLayout layout;
        AUDIO::CAudioBusLayout::SBus& gated = layout.AddBus( "gated" );
        CORE::CValueList options;
        options.Set( "vadOnsetDb", 12.0f );
        AUDIO::TAudioEffectPtr described = AUDIO::CAudioGlobal::Instance()->GetAudioEffectRegistry().CreateEffect( "noiseGate", options );
        ASSERT_FALSE( described.IsNULL() );
        described->SetParameter( TGate::PARAMETER_DETECTOR, (Float32) TGate::DETECTOR_LEVEL );
        described->SetParameter( TGate::PARAMETER_THRESHOLD, -40.0f );
        described->SetParameter( TGate::PARAMETER_RANGE, -20.0f );
        described->SetParameter( TGate::PARAMETER_HOLD, 80.0f );
        ASSERT_TRUE( described->SaveConfig( gated.AddEffect( 0, "noiseGate" ).config ) );

        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams() ) );
        CORE::CFutureResult applied = engine.ApplyBusLayout( layout );
        ASSERT_FALSE( applied.HasNoFuture() );
        ASSERT_TRUE( RenderUntilReady( engine, applied ) );
        AUDIO::CAudioBus* bus = engine.FindBus( "gated" );
        ASSERT_TRUE( GUCEF_NULL != bus );
        AUDIO::TAudioEffectPtr placed = engine.GetBusEffect( bus, 0 );
        ASSERT_FALSE( placed.IsNULL() );
        ASSERT_TRUE( -40.0f == placed->GetParameter( TGate::PARAMETER_THRESHOLD ) );
        ASSERT_TRUE( -20.0f == placed->GetParameter( TGate::PARAMETER_RANGE ) );
        ASSERT_TRUE( 80.0f == placed->GetParameter( TGate::PARAMETER_HOLD ) );
        ASSERT_TRUE( 12.0f == placed.StaticCast< TGate >()->GetVoiceActivitySettings().GetOnsetInDecibels() );

        // a loud constant passes (open), a quiet one is lowered by the range
        engine.Play( MakeConstantClip( 0.3f ), bus );
        RenderBlocks( engine, 100, 256 );
        ASSERT_TRUE( Near( (Float64) PeakOfBus( bus ), 0.3, 0.001 ) );
        ASSERT_TRUE( placed.StaticCast< TGate >()->IsOpen() );

        AUDIO::CAudioBusLayout captured;
        ASSERT_TRUE( engine.GetBusLayout( captured ) );
        engine.Shutdown();

        AUDIO::CAudioEngine fresh;
        ASSERT_TRUE( fresh.Initialize( NullParams() ) );
        CORE::CFutureResult appliedAgain = fresh.ApplyBusLayout( captured );
        ASSERT_FALSE( appliedAgain.HasNoFuture() );
        ASSERT_TRUE( RenderUntilReady( fresh, appliedAgain ) );
        AUDIO::CAudioBus* freshBus = fresh.FindBus( "gated" );
        ASSERT_TRUE( GUCEF_NULL != freshBus );
        AUDIO::TAudioEffectPtr rebuilt = fresh.GetBusEffect( freshBus, 0 );
        ASSERT_FALSE( rebuilt.IsNULL() );
        ASSERT_TRUE( rebuilt->GetEffectTypeName() == "noiseGate" );
        for ( UInt32 p=0; p<TGate::PARAMETER_COUNT; ++p )
            ASSERT_TRUE( rebuilt->GetParameter( p ) == placed->GetParameter( p ) );
        ASSERT_TRUE( 12.0f == rebuilt.StaticCast< TGate >()->GetVoiceActivitySettings().GetOnsetInDecibels() );
        fresh.Play( MakeConstantClip( 0.001f ), freshBus );
        RenderBlocks( fresh, 200, 256 );
        ASSERT_TRUE( Near( DecibelsOf( (Float64) PeakOfBus( freshBus ) / 0.001 ), -20.0, 0.1 ) );
        fresh.Shutdown();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformAudioNoiseGateTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioNoiseGate TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.NoiseGate" );

    PerformParameterTests();
    PerformLevelTests();
    PerformVoiceTests();
    PerformInvarianceAndRealtimeTests();
    PerformEngineTests();
}

/*-------------------------------------------------------------------------*/
