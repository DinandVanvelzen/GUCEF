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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_AUDIO_CAUDIOENGINE_H
#include "gucefAUDIO_CAudioEngine.h"
#define GUCEF_AUDIO_CAUDIOENGINE_H
#endif /* GUCEF_AUDIO_CAUDIOENGINE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAudioTestEffect.h"
#include "TestAudioReferenceScenarios.h"
#include "TestAudioVoiceEffects.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

#if defined( __has_feature )
  #if __has_feature( realtime_sanitizer )
    #define GUCEF_AUDIO_TEST_RTSAN_BUILD
  #endif
#endif

const AUDIO::CAudioFormat StereoFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 );
const Float64 Pi = 3.14159265358979323846;

typedef GUCEF::vector< Float32 > TSamples;
typedef GUCEF::vector< Float64 > TTimes;

bool
Near( Float64 a, Float64 b, Float64 tolerance )
{
    return fabs( a - b ) <= tolerance;
}

/** Engine on the null driver that keeps what it renders, limiter off so the output is the plain sum */
AUDIO::CAudioEngineParams
NullParams( bool threaded = false )
{
    AUDIO::CAudioEngineParams params;
    params.SetDriverName( "null" );
    params.SetThreadingMode( threaded ? AUDIO::AUDIOTHREADING_THREADED : AUDIO::AUDIOTHREADING_COOPERATIVE );
    params.SetUpdateIntervalInMs( 1 );
    params.SetLimiterEnabled( false );
    params.GetDeviceParams().SetBufferSizeInFrames( 256 );
    params.GetDeviceParams().SetPreferredFormat( StereoFormat );
    params.GetDeviceParams().GetOptions().Set( "keepOutput", "true" );
    return params;
}

AUDIO::TAudioClipPtr
MakeConstant( UInt32 frames, Float32 value )
{
    AUDIO::CAudioBuffer audio( AUDIO::CAudioFormat( 48000, 1, AUDIO::SAMPLETYPE_FLOAT32 ), frames );
    for ( UInt32 f=0; f<frames; ++f )
        audio.SetSampleFromFloat( f, 0, value );
    return AUDIO::TAudioClipPtr( GUCEF_NEW AUDIO::CAudioClip( audio ) );
}

AUDIO::TAudioClipPtr
MakeImpulse( UInt32 frames, Float32 value )
{
    AUDIO::CAudioBuffer audio( AUDIO::CAudioFormat( 48000, 1, AUDIO::SAMPLETYPE_FLOAT32 ), frames );
    for ( UInt32 f=0; f<frames; ++f )
        audio.SetSampleFromFloat( f, 0, 0 == f ? value : 0.0f );
    return AUDIO::TAudioClipPtr( GUCEF_NEW AUDIO::CAudioClip( audio ) );
}

AUDIO::TAudioClipPtr
MakeSine( UInt32 frames, Float64 frequency, Float32 amplitude )
{
    AUDIO::CAudioBuffer audio( AUDIO::CAudioFormat( 48000, 1, AUDIO::SAMPLETYPE_FLOAT32 ), frames );
    for ( UInt32 f=0; f<frames; ++f )
        audio.SetSampleFromFloat( f, 0, amplitude * (Float32) sin( 2.0 * Pi * frequency * (Float64) f / 48000.0 ) );
    return AUDIO::TAudioClipPtr( GUCEF_NEW AUDIO::CAudioClip( audio ) );
}

AUDIO::CNullAudioDevice*
NullDevice( AUDIO::CAudioEngine& engine )
{
    return engine.GetDevice()->AsNullDevice();
}

/** Renders and updates until the future is ready. Cooperative mode needs both; in threaded mode Update() does no harm */
bool
RenderUntilReady( AUDIO::CAudioEngine& engine, const CORE::CFutureResult& future, UInt32 maxBlocks = 50 )
{
    for ( UInt32 i=0; i<maxBlocks; ++i )
    {
        if ( future.IsReady() )
            return true;
        NullDevice( engine )->Render( 256 );
        engine.Update();
        if ( engine.IsThreaded() )
            MT::ThreadDelay( 1 );
    }
    return future.IsReady();
}

bool
IsFailedWith( const CORE::CFutureResult& future, CORE::TTaskStatus status )
{
    if ( !future.HasNoFuture() )
        return false;
    CORE::CTaskPtr task = future.GetResult( 1000 );
    return !task.IsNULL() && status == task->GetTaskStatus();
}

/** Sample of the rendered output, -999 beyond what was rendered */
Float32
Rendered( AUDIO::CAudioEngine& engine, UInt32 frame, UInt32 channel = 0 )
{
    const AUDIO::CAudioBuffer& rendered = NullDevice( engine )->GetRenderedAudio();
    const UInt32 channels = rendered.GetFormat().GetChannelCount();
    if ( frame >= rendered.GetFrameCount() || channel >= channels )
        return -999.0f;
    return static_cast< const Float32* >( rendered.GetDataPtr() )[ (size_t) frame * channels + channel ];
}

/** True when every rendered sample of one channel from 'first' up to 'last' is 'value' within 'tolerance' */
bool
RenderedIs( AUDIO::CAudioEngine& engine, UInt32 first, UInt32 last, UInt32 channel, Float32 value, Float32 tolerance )
{
    for ( UInt32 f=first; f<=last; ++f )
        if ( !Near( Rendered( engine, f, channel ), value, tolerance ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceEffects: frame " << f << " channel " << channel << " is " << Rendered( engine, f, channel ) << ", expected " << value );
            return false;
        }
    return true;
}

/** Largest absolute sample of one channel of what the engine rendered, from 'first' up to 'last' */
Float32
RenderedPeak( AUDIO::CAudioEngine& engine, UInt32 first, UInt32 last, UInt32 channel )
{
    Float32 peak = 0.0f;
    for ( UInt32 f=first; f<=last; ++f )
    {
        const Float32 x = (Float32) fabs( Rendered( engine, f, channel ) );
        if ( x > peak )
            peak = x;
    }
    return peak;
}

/** Largest difference between neighbouring frames of one channel of interleaved audio */
Float32
LargestStep( const TSamples& audio, UInt32 channels, UInt32 channel, UInt32 firstFrame, UInt32 lastFrame )
{
    Float32 largest = 0.0f;
    for ( UInt32 f=firstFrame+1; f<=lastFrame; ++f )
    {
        const Float32 step = (Float32) fabs( audio[ (size_t) f * channels + channel ] - audio[ (size_t) ( f - 1 ) * channels + channel ] );
        if ( step > largest )
            largest = step;
    }
    return largest;
}

/** Where one channel goes from below zero to zero or above, in frames, placed between the samples by linear interpolation */
void
UpwardCrossings( const TSamples& audio, UInt32 channels, UInt32 channel, TTimes& times )
{
    const size_t frames = audio.size() / channels;
    for ( size_t f=1; f<frames; ++f )
    {
        const Float64 a = audio[ ( f - 1 ) * channels + channel ];
        const Float64 b = audio[ f * channels + channel ];
        if ( a < 0.0 && b >= 0.0 )
            times.push_back( (Float64) ( f - 1 ) + a / ( a - b ) );
    }
}

/** Average frequency of the crossings between two frame positions */
Float64
FrequencyBetween( const TTimes& crossings, Float64 from, Float64 to )
{
    Float64 first = -1.0, last = -1.0;
    UInt32 periods = 0;
    for ( size_t i=0; i<crossings.size(); ++i )
    {
        if ( crossings[ i ] < from || crossings[ i ] > to )
            continue;
        if ( first < 0.0 )
            first = crossings[ i ];
        else
            ++periods;
        last = crossings[ i ];
    }
    if ( 0 == periods )
        return 0.0;
    return (Float64) periods * 48000.0 / ( last - first );
}

/** Mixes 'frames' stereo frames into 'out' in pieces of a device period (256 frames), the last piece what is left */
void
MixInPieces( AUDIO::CAudioMixer& mixer, TSamples& out, UInt32 frames )
{
    out.assign( (size_t) frames * 2, 0.0f );
    for ( UInt32 f=0; f<frames; f+=256 )
        mixer.Mix( &out[ (size_t) f * 2 ], frames - f < 256 ? frames - f : 256 );
}

bool
SetNamedParameter( AUDIO::TAudioEffectPtr& effect, const char* name, Float32 value )
{
    UInt32 id = 0;
    if ( effect.IsNULL() || !effect->TryGetParameterId( name, id ) )
        return false;
    effect->SetParameter( id, value );
    return true;
}

/** A delay on its own: no feedback filters, the echo at full level */
AUDIO::TAudioEffectPtr
MakeDelay( AUDIO::CAudioEngine& engine, Float32 timeInMs, Float32 feedback )
{
    AUDIO::TAudioEffectPtr delay = engine.CreateEffect( "delay" );
    if ( !SetNamedParameter( delay, "time", timeInMs ) || !SetNamedParameter( delay, "feedback", feedback ) ||
         !SetNamedParameter( delay, "lowCut", 20.0f ) || !SetNamedParameter( delay, "highCut", 20000.0f ) )
        return AUDIO::TAudioEffectPtr();
    return delay;
}

/**
 *  Mixer scenario with voices of other rates and channel counts, a looping voice, a voice at another playback rate, pans
 *  and volume changes while it plays. 'decorated' gives every voice a transparent effect (the test effect at its
 *  defaults passes the audio unchanged), a send at level 0 to an aux bus and a pitch of exactly 1
 */
UInt64
RunVoiceScenario( bool decorated )
{
    AUDIO::CAudioMixer mixer( StereoFormat, 128, 8, 6 );
    mixer.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_INSTANT );
    AUDIO::CAudioBus* music = mixer.CreateBus( "music" );
    AUDIO::CAudioBus* aux = mixer.CreateBus( "aux" );
    music->SetVolume( 0.8f );

    const UInt32 rates[ 4 ] = { 22050, 48000, 48000, 44100 };
    const UInt32 channels[ 4 ] = { 1, 2, 1, 1 };
    const UInt32 lengths[ 4 ] = { 3000, 900, 20000, 8000 };
    AUDIO::TAudioVoicePtr voices[ 4 ];
    for ( UInt32 v=0; v<4; ++v )
    {
        AUDIO::TAudioClipPtr clip = TestAudioReferenceScenarios::MakeRampClip( rates[ v ], channels[ v ], lengths[ v ], -0.6f + 0.3f * (Float32) v, 0.7f - 0.2f * (Float32) v );
        voices[ v ] = AUDIO::TAudioVoicePtr( GUCEF_NEW AUDIO::CAudioVoice( clip->CreateSource() ) );
        voices[ v ]->SetBus( 0 == v % 2 ? music : GUCEF_NULL );
        if ( decorated )
        {
            if ( !voices[ v ]->AddEffect( AUDIO::TAudioEffectPtr( GUCEF_NEW CTestAudioEffect() ) ) || !voices[ v ]->SetSend( 0, aux, 0.0f ) )
                return 0;
            voices[ v ]->SetPitch( 1.0f );
        }
    }
    voices[ 0 ]->SetPan( -0.3f );
    voices[ 0 ]->SetPlaybackRate( 1.25f );
    voices[ 1 ]->SetLoopCount( 4 );
    voices[ 3 ]->SetPan( 0.5f );
    for ( UInt32 v=0; v<4; ++v )
        if ( !mixer.PrepareVoice( voices[ v ].GetPointerAlways() ) || !mixer.AddVoice( voices[ v ].GetPointerAlways() ) )
            return 0;

    const UInt32 total = 6000;
    const UInt32 piece = 100;
    TSamples out( (size_t) total * 2, 0.0f );
    for ( UInt32 p=0; p*piece<total; ++p )
    {
        if ( 10 == p ) music->SetVolume( 0.3f );
        if ( 20 == p ) voices[ 2 ]->SetPan( 0.8f );
        if ( 30 == p ) voices[ 3 ]->SetVolume( 0.4f );
        mixer.Mix( &out[ (size_t) p * piece * 2 ], piece );
    }
    return TestAudioReferenceScenarios::HashFloats( &out[ 0 ], out.size() );
}

/** Changes the pitch, the send level and the effect slot of a voice as fast as it can, from its own thread */
struct SVoiceWriter
{
    AUDIO::CAudioVoice* voice;
    UInt32 changes;
    MT::CTAtomic< bool > done;
    SVoiceWriter( void ) : voice( GUCEF_NULL ), changes( 0 ), done( false ) {}
};

UInt32 GUCEF_CALLSPEC_PREFIX
VoiceWriterMain( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    SVoiceWriter* writer = static_cast< SVoiceWriter* >( data );
    for ( UInt32 i=0; i<writer->changes; ++i )
    {
        writer->voice->SetPitch( 0.5f + 0.25f * (Float32) ( i % 7 ) );
        writer->voice->SetSendLevel( 0, 0.25f * (Float32) ( i % 5 ) );
        writer->voice->SetEffectMix( 0, 1.0f, 0.5f * (Float32) ( i % 2 ) );
        writer->voice->SetEffectEnabled( 0, 0 != i % 3 );
        if ( 0 == i % 64 )
            MT::ThreadDelay( 0 );
    }
    writer->done.Store( true, MT::MEMORDER_RELEASE );
    return 0;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformPlaybackRateTests( void )
{GUCEF_TRACE;

    // A voice of a source at the mix rate resamples only while its speed is not 1. Going back to 1 must continue where the
    // resampler was: the source audio that was read for it and not yet used is part of the sound
    GUCEF_TESTFW_TESTCASE( "AudioVoiceEffects: Back to playback rate 1 after resampling continues the sound without a jump" )
    try
    {
        AUDIO::CAudioMixer mixer( StereoFormat, 256, 4, 2 );
        const UInt32 frames = 20000;
        AUDIO::TAudioClipPtr ramp = TestAudioReferenceScenarios::MakeRampClip( 48000, 1, frames, 0.0f, (Float32) ( frames - 1 ) / 32768.0f );
        AUDIO::TAudioVoicePtr voice = TestAudioReferenceScenarios::MakeMixerVoice( mixer, ramp, GUCEF_NULL );
        ASSERT_TRUE( mixer.AddVoice( voice.GetPointerAlways() ) );

        TSamples out( (size_t) 256 * 3 * 2, 0.0f );
        mixer.Mix( &out[ 0 ], 256 );
        voice->SetPlaybackRate( 1.5f );
        mixer.Mix( &out[ 256 * 2 ], 256 );
        voice->SetPlaybackRate( 1.0f );
        mixer.Mix( &out[ 512 * 2 ], 256 );

        // one source frame is 1/32768 of the ramp: a step of at most 1.5 source frames is the speed, anything more a jump
        const Float32 step = LargestStep( out, 2, 0, 0, 767 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceEffects: largest step " << step * 32768.0f << " source frames" );
        ASSERT_TRUE( step <= 1.5001f / 32768.0f );
        ASSERT_TRUE( fabs( out[ 767 * 2 ] * 32768.0f - (Float32) ( 256 + 384 + 255 ) ) < 1.01f );
        ASSERT_TRUE( 256 + 384 + 256 == voice->GetPositionInFrames() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformPlainVoiceTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceEffects: Plain voices sound bit for bit as before: the reference scenarios of the mixer and the engine" )
    try
    {
        ASSERT_TRUE( TestAudioReferenceScenarios::ReferenceMixerLimiterStereo == TestAudioReferenceScenarios::RunMixerTreeScenario( true, 2 ) );
        ASSERT_TRUE( TestAudioReferenceScenarios::ReferenceMixerClippingStereo == TestAudioReferenceScenarios::RunMixerTreeScenario( false, 2 ) );
        ASSERT_TRUE( TestAudioReferenceScenarios::ReferenceMixerLimiterSurround == TestAudioReferenceScenarios::RunMixerTreeScenario( true, 6 ) );
        ASSERT_TRUE( TestAudioReferenceScenarios::ReferenceEngine == TestAudioReferenceScenarios::RunEngineScenario() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceEffects: A transparent effect, a send at level 0 and a pitch of 1 leave the output of voices bit for bit the same" )
    try
    {
        const UInt64 plain = RunVoiceScenario( false );
        const UInt64 decorated = RunVoiceScenario( true );
        ASSERT_TRUE( 0 != plain );
        ASSERT_TRUE( plain == decorated );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformChainTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceEffects: A voice's effects process that voice only; slot switch and wet and dry work as on a bus" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams() ) );

        // an effect that is not prepared yet: Start() prepares it for the mix format
        AUDIO::TAudioEffectPtr half( GUCEF_NEW CTestAudioEffect() );
        half->SetParameter( CTestAudioEffect::PARAMETER_GAIN, 0.5f );
        ASSERT_FALSE( half->IsPrepared() );

        AUDIO::TAudioVoicePtr withEffect = engine.CreateVoice( MakeConstant( 48000, 0.1f ) );
        AUDIO::TAudioVoicePtr plain = engine.CreateVoice( MakeConstant( 48000, 0.2f ) );
        ASSERT_TRUE( withEffect->AddEffect( half ) );
        ASSERT_TRUE( 1 == withEffect->GetEffectCount() && withEffect->GetEffect( 0 ) == half );
        CORE::CFutureResult a = engine.Start( withEffect );
        CORE::CFutureResult b = engine.Start( plain );
        ASSERT_TRUE( half->IsPreparedFor( engine.GetMixFormat(), engine.GetParams().GetMaxBlockFrames() ) );
        ASSERT_TRUE( RenderUntilReady( engine, a ) && RenderUntilReady( engine, b ) );

        // 0.1 halved plus 0.2 untouched, on both speakers (mono goes to both at gain 1)
        ASSERT_TRUE( RenderedIs( engine, 0, 255, 0, 0.25f, 1.0e-6f ) );
        ASSERT_TRUE( RenderedIs( engine, 0, 255, 1, 0.25f, 1.0e-6f ) );

        // switched off it crossfades in 10 ms to the voice as it is, on again with dry 1 it is processed plus unprocessed
        NullDevice( engine )->ClearRenderedAudio();
        withEffect->SetEffectEnabled( 0, false );
        ASSERT_FALSE( withEffect->IsEffectEnabled( 0 ) );
        NullDevice( engine )->Render( 1024 );
        ASSERT_TRUE( RenderedIs( engine, 600, 1023, 0, 0.3f, 1.0e-6f ) );
        NullDevice( engine )->ClearRenderedAudio();
        withEffect->SetEffectMix( 0, 1.0f, 1.0f );
        withEffect->SetEffectEnabled( 0, true );
        ASSERT_TRUE( 1.0f == withEffect->GetEffectWet( 0 ) && 1.0f == withEffect->GetEffectDry( 0 ) );
        NullDevice( engine )->Render( 1024 );
        ASSERT_TRUE( RenderedIs( engine, 600, 1023, 0, 0.35f, 1.0e-6f ) );
        engine.Shutdown();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceEffects: After the source the effects ring out: a delay's echo is heard, then the voice ends" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams() ) );

        // 50 ms is 2400 frames; the source is 480 frames long, so the echo comes after its end
        AUDIO::TAudioEffectPtr delay = MakeDelay( engine, 50.0f, 0.0f );
        ASSERT_FALSE( delay.IsNULL() );
        AUDIO::TAudioVoicePtr voice = engine.CreateVoice( MakeImpulse( 480, 0.5f ) );
        ASSERT_TRUE( voice->AddEffect( delay ) );
        voice->SetEffectMix( 0, 1.0f, 1.0f );
        CORE::CFutureResult started = engine.Start( voice );
        CORE::CFutureResult ended = engine.GetEndedResult( voice );
        ASSERT_FALSE( started.HasNoFuture() || ended.HasNoFuture() );

        NullDevice( engine )->Render( 2560 );
        engine.Update();
        ASSERT_FALSE( ended.IsReady() );                                         // the source is over, the echo is not
        NullDevice( engine )->Render( 1024 );
        engine.Update();
        ASSERT_TRUE( ended.IsReady() && AUDIO::AUDIOVOICEEND_FINISHED == voice->GetEndReason() );
        ASSERT_TRUE( Near( Rendered( engine, 0 ), 0.5, 1.0e-6 ) );
        ASSERT_TRUE( Near( Rendered( engine, 2400 ), 0.5, 1.0e-3 ) );
        ASSERT_TRUE( RenderedPeak( engine, 1, 2390, 0 ) < 1.0e-3f && RenderedPeak( engine, 2410, 3583, 0 ) < 1.0e-3f );
        engine.Shutdown();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceEffects: An effect that can sound forever rings out until the voice was silent for a second" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams() ) );

        // echoes every 20 ms at half the level of the one before: below -100 dBFS after 16 of them (about 15400 frames)
        AUDIO::TAudioEffectPtr delay = MakeDelay( engine, 20.0f, 0.5f );
        ASSERT_TRUE( GUCEF_AUDIO_TAIL_INFINITE == delay->GetTailInFrames() );
        AUDIO::TAudioVoicePtr voice = engine.CreateVoice( MakeImpulse( 480, 0.5f ) );
        ASSERT_TRUE( voice->AddEffect( delay ) );
        voice->SetEffectMix( 0, 1.0f, 1.0f );
        engine.Start( voice );
        CORE::CFutureResult ended = engine.GetEndedResult( voice );

        NullDevice( engine )->Render( 48000 );
        engine.Update();
        ASSERT_FALSE( ended.IsReady() );
        ASSERT_TRUE( RenderedPeak( engine, 960 * 10 - 4, 960 * 10 + 4, 0 ) > 0.0003f );    // the tenth echo: 0.5^11
        NullDevice( engine )->Render( 24000 );
        engine.Update();
        ASSERT_TRUE( ended.IsReady() );
        const UInt64 rendered = NullDevice( engine )->GetTotalFramesRendered();
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceEffects: voice with an unbounded tail ended within " << rendered << " frames" );
        engine.Shutdown();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformSendTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceEffects: Sends take the voice after its volume and pan, at their level, into aux buses fed by sends alone" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams() ) );
        AUDIO::CAudioBus* dry = engine.CreateBus( "dry" );
        AUDIO::CAudioBus* aux1 = engine.CreateBus( "aux1" );
        AUDIO::CAudioBus* aux2 = engine.CreateBus( "aux2" );
        ASSERT_TRUE( GUCEF_NULL != dry && GUCEF_NULL != aux1 && GUCEF_NULL != aux2 );
        dry->SetMuted( true );                                                  // what is heard comes through the sends

        AUDIO::TAudioVoicePtr voice = engine.CreateVoice( MakeConstant( 48000, 0.4f ), dry );
        ASSERT_TRUE( 2 == voice->GetMaxSends() );
        voice->SetVolume( 0.5f );
        ASSERT_TRUE( voice->SetSend( 0, aux1, 0.5f ) && voice->SetSend( 1, aux2, 0.25f ) );
        ASSERT_TRUE( aux1 == voice->GetSendBus( 0 ) && 0.25f == voice->GetSendLevel( 1 ) );
        ASSERT_TRUE( RenderUntilReady( engine, engine.Start( voice ) ) );
        NullDevice( engine )->ClearRenderedAudio();
        NullDevice( engine )->Render( 512 );

        // 0.4 at volume 0.5 is 0.2: 0.1 through the first send and 0.05 through the second, after the volume
        ASSERT_TRUE( RenderedIs( engine, 0, 511, 0, 0.15f, 1.0e-6f ) );
        ASSERT_TRUE( RenderedIs( engine, 0, 511, 1, 0.15f, 1.0e-6f ) );
        ASSERT_TRUE( Near( aux1->GetMeter().GetPeak( 0 ), 0.1, 1.0e-6 ) && Near( aux2->GetMeter().GetPeak( 0 ), 0.05, 1.0e-6 ) );
        ASSERT_TRUE( 0.0f == dry->GetMeter().GetPeak( 0 ) );

        // a level change is a ramp over the next block: from 0.5 to 1.0 on the first send
        NullDevice( engine )->ClearRenderedAudio();
        voice->SetSendLevel( 0, 1.0f );
        NullDevice( engine )->Render( 256 );
        ASSERT_TRUE( Near( Rendered( engine, 0 ), 0.2 * ( 0.5 + 0.5 / 256.0 ) + 0.05, 1.0e-6 ) );
        ASSERT_TRUE( Near( Rendered( engine, 127 ), 0.2 * ( 0.5 + 0.5 * 128.0 / 256.0 ) + 0.05, 1.0e-6 ) );
        ASSERT_TRUE( Near( Rendered( engine, 255 ), 0.25, 1.0e-6 ) );
        engine.Stop( voice );

        // the pan is part of what is sent: a mono voice all the way left (constant power) reaches the left speaker only
        AUDIO::TAudioVoicePtr left = engine.CreateVoice( MakeConstant( 48000, 0.4f ), dry );
        left->SetVolume( 0.5f );
        left->SetPan( -1.0f );
        ASSERT_TRUE( left->SetSend( 0, aux1, 0.5f ) );
        ASSERT_TRUE( RenderUntilReady( engine, engine.Start( left ) ) );
        NullDevice( engine )->ClearRenderedAudio();
        NullDevice( engine )->Render( 256 );
        ASSERT_TRUE( RenderedIs( engine, 0, 255, 0, 0.2f * 1.41421356f * 0.5f, 1.0e-6f ) );
        ASSERT_TRUE( RenderedIs( engine, 0, 255, 1, 0.0f, 1.0e-6f ) );
        engine.Shutdown();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformPitchTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceEffects: A pitch set before the start is there at once, a change glides smoothly to the right frequency" )
    try
    {
        AUDIO::CAudioMixer mixer( StereoFormat, 256, 4, 2 );
        AUDIO::TAudioClipPtr sine = MakeSine( 5 * 48000, 1000.0, 0.5f );
        AUDIO::TAudioVoicePtr voice( GUCEF_NEW AUDIO::CAudioVoice( sine->CreateSource() ) );
        voice->SetPitch( 1.5f );
        ASSERT_TRUE( 1.5f == voice->GetPitch() );
        ASSERT_TRUE( mixer.PrepareVoice( voice.GetPointerAlways() ) && mixer.AddVoice( voice.GetPointerAlways() ) );

        // half a second at 1.5: 1500 Hz from the first period on
        const UInt32 half = 24000;
        TSamples first;
        MixInPieces( mixer, first, half );
        TTimes crossings;
        UpwardCrossings( first, 2, 0, crossings );
        const Float64 start = FrequencyBetween( crossings, 0.0, 2400.0 );
        const Float64 steady = FrequencyBetween( crossings, 0.0, (Float64) half );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceEffects: pitch 1.5 from the start: " << start << " Hz in the first 50 ms, " << steady << " Hz over 500 ms" );
        ASSERT_TRUE( Near( start, 1500.0, 1.5 ) && Near( steady, 1500.0, 0.3 ) );

        // to 3.0, an octave up: a glide of about 20 ms (one pole), never a step
        voice->SetPitch( 3.0f );
        TSamples second;
        MixInPieces( mixer, second, half );
        crossings.clear();
        UpwardCrossings( second, 2, 0, crossings );
        ASSERT_TRUE( crossings.size() > 100 );
        const Float64 arrived = FrequencyBetween( crossings, 14400.0, (Float64) half );
        ASSERT_TRUE( Near( arrived, 3000.0, 0.6 ) );

        // frequency per period: it only goes up, starts near 1500, and passes 10 % and 90 % of the octave about 44 ms apart
        Float64 previous = 0.0, at10 = -1.0, at90 = -1.0;
        bool rising = true;
        for ( size_t i=1; i<crossings.size(); ++i )
        {
            const Float64 frequency = 48000.0 / ( crossings[ i ] - crossings[ i - 1 ] );
            const Float64 progress = log( frequency / 1500.0 ) / log( 2.0 );
            if ( 1 == i )
                ASSERT_TRUE( frequency < 1650.0 );
            if ( frequency < previous * 0.995 )
                rising = false;
            if ( at10 < 0.0 && progress >= 0.1 ) at10 = crossings[ i ];
            if ( at90 < 0.0 && progress >= 0.9 ) at90 = crossings[ i ];
            previous = frequency;
        }
        const Float64 riseInMs = ( at90 - at10 ) * 1000.0 / 48000.0;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceEffects: pitch 1.5 to 3.0: " << arrived << " Hz after 300 ms, 10 to 90 percent of the octave in " << riseInMs << " ms (one pole of 20 ms: 43.9 ms)" );
        ASSERT_TRUE( rising );
        ASSERT_TRUE( at10 > 0.0 && at90 > at10 && riseInMs > 38.0 && riseInMs < 50.0 );

        // no click: no step between samples larger than that of the 3000 Hz sine itself
        const Float32 sineStep = (Float32) ( 2.0 * Pi * 3000.0 / 48000.0 * 0.5 );
        ASSERT_TRUE( LargestStep( second, 2, 0, 0, half - 1 ) <= sineStep * 1.01f );

        // the playback rate multiplies with the pitch and changes at once: half speed gives 1500 Hz in the next block
        voice->SetPlaybackRate( 0.5f );
        TSamples third;
        MixInPieces( mixer, third, 4800 );
        crossings.clear();
        UpwardCrossings( third, 2, 0, crossings );
        ASSERT_TRUE( Near( FrequencyBetween( crossings, 256.0, 4800.0 ), 1500.0, 0.5 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformThreadTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceEffects: Pitch, send level and effect slot settings set from another thread while the audio thread renders" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams( true ) ) );
        AUDIO::CAudioBus* aux = engine.CreateBus( "aux" );
        AUDIO::TAudioVoicePtr voice = engine.CreateVoice( MakeSine( 48000, 440.0, 0.25f ) );
        voice->SetLoopCount( 0 );
        ASSERT_TRUE( voice->AddEffect( AUDIO::TAudioEffectPtr( GUCEF_NEW CTestAudioEffect() ) ) );
        ASSERT_TRUE( voice->SetSend( 0, aux, 0.5f ) );
        ASSERT_TRUE( RenderUntilReady( engine, engine.Start( voice ), 200 ) );

        SVoiceWriter writer;
        writer.voice = voice.GetPointerAlways();
        writer.changes = 20000;
        MT::SThreadData* thread = MT::ThreadDataReserve();
        ASSERT_TRUE( 0 != MT::ThreadCreate( thread, &VoiceWriterMain, &writer ) );

        // the test thread is the audio thread, the engine thread does the owner side
        UInt32 blocks = 0;
        bool finite = true;
        while ( !writer.done.Load( MT::MEMORDER_ACQUIRE ) )
        {
            NullDevice( engine )->Render( 64 );
            ++blocks;
            if ( 0 == blocks % 64 )
            {
                const Float32 peak = RenderedPeak( engine, 0, (UInt32) NullDevice( engine )->GetRenderedAudio().GetFrameCount() - 1, 0 );
                finite = ( peak == peak && peak <= 1.0f ) && finite;
                NullDevice( engine )->ClearRenderedAudio();
            }
        }
        MT::ThreadWait( thread, 60000 );
        MT::ThreadDataCleanup( thread );
        NullDevice( engine )->Render( 256 );

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceEffects: " << blocks << " blocks rendered while another thread made " << writer.changes << " rounds of changes" );
        ASSERT_TRUE( finite );
        ASSERT_TRUE( AUDIO::AUDIOVOICESTATE_PLAYING == voice->GetState() );
        engine.Shutdown();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformReleaseTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceEffects: The effects of a voice are let go of on the engine thread once it has ended, never on the audio thread" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams( true ) ) );

        AUDIO::TAudioVoicePtr voice = engine.CreateVoice( MakeConstant( 1000, 0.1f ) );
        {
            AUDIO::TAudioEffectPtr first( GUCEF_NEW CTestAudioEffect() );
            AUDIO::TAudioEffectPtr second( GUCEF_NEW CTestAudioEffect() );
            ASSERT_TRUE( voice->AddEffect( first ) && voice->AddEffect( second ) );
        }                                                                       // the voice holds the only references now
        const UInt32 destroyedBefore = CTestAudioEffect::DestroyedCount().Load();
        CTestAudioEffect::LastDestructorThread().Store( 0 );
        CORE::CFutureResult started = engine.Start( voice );
        CORE::CFutureResult ended = engine.GetEndedResult( voice );
        ASSERT_FALSE( started.HasNoFuture() || ended.HasNoFuture() );

        // the test thread is the audio thread: it renders, the engine thread completes the futures and lets go of the effects
        for ( UInt32 i=0; i<5000 && !ended.IsReady(); ++i )
        {
            NullDevice( engine )->Render( 64 );
            MT::ThreadDelay( 0 );
        }
        ASSERT_TRUE( ended.IsReady() );
        ASSERT_TRUE( destroyedBefore + 2 == CTestAudioEffect::DestroyedCount().Load() );
        ASSERT_TRUE( 0 != CTestAudioEffect::LastDestructorThread().Load() );
        ASSERT_TRUE( MT::GetCurrentThreadID() != CTestAudioEffect::LastDestructorThread().Load() );   // not on the audio thread
        ASSERT_TRUE( 0 == voice->GetEffectCount() && voice->GetEffect( 0 ).IsNULL() );
        engine.Shutdown();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceEffects: Shutdown lets go of the effects of voices that still play, a voice that never started keeps its own" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams() ) );
        AUDIO::TAudioVoicePtr playing = engine.CreateVoice( MakeConstant( 48000, 0.1f ) );
        AUDIO::TAudioVoicePtr idle = engine.CreateVoice( MakeConstant( 48000, 0.1f ) );
        ASSERT_TRUE( playing->AddEffect( AUDIO::TAudioEffectPtr( GUCEF_NEW CTestAudioEffect() ) ) );
        ASSERT_TRUE( idle->AddEffect( AUDIO::TAudioEffectPtr( GUCEF_NEW CTestAudioEffect() ) ) );
        ASSERT_TRUE( RenderUntilReady( engine, engine.Start( playing ) ) );

        const UInt32 destroyedBefore = CTestAudioEffect::DestroyedCount().Load();
        engine.Shutdown();
        ASSERT_TRUE( destroyedBefore + 1 == CTestAudioEffect::DestroyedCount().Load() );
        ASSERT_TRUE( 0 == playing->GetEffectCount() && 1 == idle->GetEffectCount() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformReopenTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceEffects: A device reopen with another format keeps voices with effects and sends playing, the effects prepared again" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams() ) );
        AUDIO::CAudioBus* aux = engine.CreateBus( "aux" );
        AUDIO::TAudioEffectPtr half( GUCEF_NEW CTestAudioEffect() );
        half->SetParameter( CTestAudioEffect::PARAMETER_GAIN, 0.5f );
        AUDIO::TAudioVoicePtr voice = engine.CreateVoice( MakeSine( 48000, 1000.0, 0.5f ) );
        voice->SetLoopCount( 0 );
        ASSERT_TRUE( voice->AddEffect( half ) && voice->SetSend( 0, aux, 0.5f ) );
        ASSERT_TRUE( RenderUntilReady( engine, engine.Start( voice ) ) );

        // 0.5 halved by the effect is 0.25, plus half of that through the send: 0.375
        NullDevice( engine )->Render( 4800 );
        ASSERT_TRUE( Near( RenderedPeak( engine, 1000, 4799, 0 ), 0.375, 1.0e-3 ) );

        // four speakers at 44.1 kHz: the voice goes on, its effect is prepared for the new format and still halves
        AUDIO::CAudioEngineParams next = engine.GetParams();
        next.GetDeviceParams().SetPreferredFormat( AUDIO::CAudioFormat( 44100, 4, AUDIO::SAMPLETYPE_FLOAT32 ) );
        ASSERT_TRUE( engine.ReopenDevice( next ) );
        ASSERT_TRUE( 4 == engine.GetMixFormat().GetChannelCount() && 44100 == engine.GetMixFormat().GetSampleRate() );
        ASSERT_TRUE( half->IsPreparedFor( engine.GetMixFormat(), engine.GetParams().GetMaxBlockFrames() ) );
        ASSERT_TRUE( voice->GetEffect( 0 ) == half );
        NullDevice( engine )->Render( 4410 );
        engine.Update();
        ASSERT_TRUE( AUDIO::AUDIOVOICESTATE_PLAYING == voice->GetState() );
        const Float32 peak = RenderedPeak( engine, 1000, 4409, 0 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceEffects: peak after the reopen " << peak );
        ASSERT_TRUE( Near( peak, 0.375, 0.01 ) );
        ASSERT_TRUE( Near( aux->GetMeter().GetPeak( 0 ), 0.125, 0.01 ) );
        engine.Shutdown();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformCapacityTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceEffects: Capacities, the one owner of an effect and the time to set effects and sends up are enforced" )
    try
    {
        AUDIO::CAudioEngineParams params = NullParams();
        params.SetMaxVoiceEffects( 2 );
        params.SetMaxVoiceSends( 1 );
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( params ) );
        AUDIO::CAudioBus* aux = engine.CreateBus( "aux" );
        AUDIO::TAudioClipPtr clip = MakeConstant( 48000, 0.1f );

        AUDIO::TAudioEffectPtr effects[ 5 ];
        for ( UInt32 e=0; e<5; ++e )
            effects[ e ] = AUDIO::TAudioEffectPtr( GUCEF_NEW CTestAudioEffect() );

        // a voice of the engine has the engine's capacities
        AUDIO::TAudioVoicePtr voice = engine.CreateVoice( clip );
        ASSERT_TRUE( 2 == voice->GetMaxEffects() && 1 == voice->GetMaxSends() );
        ASSERT_FALSE( voice->AddEffect( AUDIO::TAudioEffectPtr() ) );
        ASSERT_TRUE( voice->AddEffect( effects[ 0 ] ) );
        ASSERT_FALSE( voice->AddEffect( effects[ 0 ] ) );                       // in the chain already
        ASSERT_TRUE( voice->AddEffect( effects[ 1 ] ) );
        ASSERT_FALSE( voice->AddEffect( effects[ 2 ] ) );                       // full
        ASSERT_FALSE( voice->SetSend( 1, aux, 1.0f ) );
        ASSERT_TRUE( voice->SetSend( 0, aux, 1.0f ) );
        ASSERT_TRUE( RenderUntilReady( engine, engine.Start( voice ) ) );

        // once started effects and sends are fixed; levels and slot settings are not
        ASSERT_FALSE( voice->AddEffect( effects[ 2 ] ) );
        ASSERT_FALSE( voice->SetSend( 0, GUCEF_NULL, 0.0f ) );
        voice->SetSendLevel( 0, 0.5f );
        ASSERT_TRUE( 0.5f == voice->GetSendLevel( 0 ) );

        // a voice made without the engine has the default capacities, more than this engine allows
        AUDIO::TAudioVoicePtr direct( GUCEF_NEW AUDIO::CAudioVoice( clip->CreateSource() ) );
        ASSERT_TRUE( AUDIO::CAudioVoice::DefaultMaxEffects == direct->GetMaxEffects() && AUDIO::CAudioVoice::DefaultMaxSends == direct->GetMaxSends() );
        ASSERT_TRUE( direct->AddEffect( effects[ 2 ] ) && direct->AddEffect( effects[ 3 ] ) && direct->AddEffect( effects[ 4 ] ) );
        ASSERT_TRUE( IsFailedWith( engine.Start( direct ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );
        AUDIO::TAudioVoicePtr secondSend( GUCEF_NEW AUDIO::CAudioVoice( clip->CreateSource() ) );
        ASSERT_TRUE( secondSend->SetSend( 1, aux, 1.0f ) );
        ASSERT_TRUE( IsFailedWith( engine.Start( secondSend ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );

        // an effect has one owner: not a second voice, not a bus slot, while its voice plays
        AUDIO::TAudioVoicePtr sharing = engine.CreateVoice( clip );
        ASSERT_TRUE( sharing->AddEffect( effects[ 0 ] ) );
        ASSERT_TRUE( IsFailedWith( engine.Start( sharing ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );
        ASSERT_TRUE( IsFailedWith( engine.InsertEffect( aux, 0, effects[ 1 ] ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );

        // and a bus slot effect cannot go on a voice
        AUDIO::TAudioEffectPtr busEffect = engine.CreateEffect( "testeffect" );
        if ( busEffect.IsNULL() )
            busEffect = AUDIO::TAudioEffectPtr( GUCEF_NEW CTestAudioEffect() );
        ASSERT_TRUE( busEffect->Prepare( engine.GetMixFormat(), engine.GetParams().GetMaxBlockFrames() ) );
        ASSERT_TRUE( RenderUntilReady( engine, engine.InsertEffect( aux, 0, busEffect ) ) );
        AUDIO::TAudioVoicePtr onBusEffect = engine.CreateVoice( clip );
        ASSERT_TRUE( onBusEffect->AddEffect( busEffect ) );
        ASSERT_TRUE( IsFailedWith( engine.Start( onBusEffect ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );

        // a send to a bus of another engine
        AUDIO::CAudioEngine other;
        ASSERT_TRUE( other.Initialize( NullParams() ) );
        AUDIO::TAudioVoicePtr elsewhere = engine.CreateVoice( clip );
        ASSERT_TRUE( elsewhere->SetSend( 0, other.CreateBus( "x" ), 1.0f ) );
        ASSERT_TRUE( IsFailedWith( engine.Start( elsewhere ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );
        other.Shutdown();

        // the refused voices did not start and the playing one still plays
        ASSERT_TRUE( AUDIO::AUDIOVOICESTATE_IDLE == direct->GetState() && AUDIO::AUDIOVOICESTATE_IDLE == sharing->GetState() );
        NullDevice( engine )->Render( 256 );
        ASSERT_TRUE( AUDIO::AUDIOVOICESTATE_PLAYING == voice->GetState() );
        engine.Shutdown();

        // a mixer refuses a send to a bus that was never prepared for it
        AUDIO::CAudioMixer mixer( StereoFormat, 256, 4, 2 );
        AUDIO::CAudioBus loose( "loose", GUCEF_NULL );
        AUDIO::TAudioVoicePtr toLoose( GUCEF_NEW AUDIO::CAudioVoice( clip->CreateSource() ) );
        ASSERT_TRUE( toLoose->SetSend( 0, &loose, 1.0f ) );
        ASSERT_FALSE( mixer.PrepareVoice( toLoose.GetPointerAlways() ) );
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

    // The render callback is a real time context in a RealtimeSanitizer build: any allocation, lock or blocking call while
    // rendering aborts the run. Voices with effects (prepared at the start), sends, a pitch glide, slot changes and an
    // unbounded ring out; the voice ends and its effects are let go of in Update(), outside the render
    GUCEF_TESTFW_TESTCASE( "AudioVoiceEffects: Voice effects, sends and pitch render without anything a real time thread must not do (RealtimeSanitizer build)" )
    #ifndef GUCEF_AUDIO_TEST_RTSAN_BUILD
    GUCEF_TESTFW_SKIP( "not a RealtimeSanitizer build (-fsanitize=realtime)" );
    #endif
    GUCEF_TESTFW_SKIP_IF( GUCEF_TESTFW_GETENV( "GUCEF_TEST_AUDIO_RTSAN_CLEAN" ) == "0", "switched off with GUCEF_TEST_AUDIO_RTSAN_CLEAN=0" );
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams() ) );
        AUDIO::CAudioBus* aux = engine.CreateBus( "aux" );
        AUDIO::TAudioEffectPtr filter = engine.CreateEffect( "filter" );
        AUDIO::TAudioEffectPtr delay = MakeDelay( engine, 30.0f, 0.4f );
        ASSERT_FALSE( filter.IsNULL() || delay.IsNULL() );

        AUDIO::TAudioVoicePtr voice = engine.CreateVoice( MakeSine( 9600, 440.0, 0.3f ) );
        ASSERT_TRUE( voice->AddEffect( AUDIO::TAudioEffectPtr( GUCEF_NEW CTestAudioEffect() ) ) );
        ASSERT_TRUE( voice->AddEffect( filter ) && voice->AddEffect( delay ) );
        voice->SetEffectMix( 2, 1.0f, 1.0f );
        ASSERT_TRUE( voice->SetSend( 0, aux, 0.5f ) );
        voice->SetPitch( 1.5f );
        AUDIO::TAudioVoicePtr plain = engine.CreateVoice( MakeSine( 9600, 220.0, 0.3f ) );
        engine.Start( plain );
        CORE::CFutureResult started = engine.Start( voice );
        CORE::CFutureResult ended = engine.GetEndedResult( voice );
        filter.Unlink();
        delay.Unlink();

        // under the sanitizer the first unsafe call aborts the run in one of these renders
        NullDevice( engine )->Render( 1024 );
        engine.Update();
        ASSERT_TRUE( started.IsReady() );
        voice->SetPitch( 0.8f );
        voice->SetSendLevel( 0, 0.1f );
        voice->SetEffectEnabled( 0, false );
        NullDevice( engine )->Render( 4096 );
        voice->SetEffectEnabled( 0, true );
        for ( UInt32 i=0; i<40 && !ended.IsReady(); ++i )
        {
            NullDevice( engine )->Render( 4096 );
            engine.Update();
        }
        ASSERT_TRUE( ended.IsReady() );
        ASSERT_TRUE( 0 == voice->GetEffectCount() );
        engine.Shutdown();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformAudioVoiceEffectsTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioVoiceEffects TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.VoiceEffects" );

    PerformPlaybackRateTests();
    PerformPlainVoiceTests();
    PerformChainTests();
    PerformSendTests();
    PerformPitchTests();
    PerformThreadTests();
    PerformReleaseTests();
    PerformReopenTests();
    PerformCapacityTests();
    PerformRealtimeTests();
}

/*-------------------------------------------------------------------------*/
