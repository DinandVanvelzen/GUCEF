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

#ifndef GUCEF_AUDIO_CNULLAUDIODRIVER_H
#include "gucefAUDIO_CNullAudioDriver.h"
#define GUCEF_AUDIO_CNULLAUDIODRIVER_H
#endif /* GUCEF_AUDIO_CNULLAUDIODRIVER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAudioTestEffect.h"
#include "TestAudioEngineReopen.h"

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

bool
Near( Float32 a, Float32 b, Float32 tolerance )
{
    return fabsf( a - b ) <= tolerance;
}

AUDIO::CAudioEngineParams
NullParams( UInt32 rate, UInt32 channels, bool threaded = false )
{
    AUDIO::CAudioEngineParams params;
    params.SetDriverName( "null" );
    params.SetThreadingMode( threaded ? AUDIO::AUDIOTHREADING_THREADED : AUDIO::AUDIOTHREADING_COOPERATIVE );
    params.SetUpdateIntervalInMs( 1 );
    params.GetDeviceParams().SetBufferSizeInFrames( 256 );
    params.GetDeviceParams().SetPreferredFormat( AUDIO::CAudioFormat( rate, channels, AUDIO::SAMPLETYPE_FLOAT32 ) );
    params.GetDeviceParams().GetOptions().Set( "keepOutput", "true" );
    return params;
}

AUDIO::TAudioClipPtr
MakeTone( UInt32 frames, Float32 amplitude )
{
    AUDIO::CAudioBuffer audio( AUDIO::CAudioFormat( 48000, 1, AUDIO::SAMPLETYPE_FLOAT32 ), frames );
    for ( UInt32 f=0; f<frames; ++f )
        audio.SetSampleFromFloat( f, 0, amplitude * (Float32) sin( 2.0 * 3.14159265358979323846 * 440.0 * (Float64) f / 48000.0 ) );
    return AUDIO::TAudioClipPtr( GUCEF_NEW AUDIO::CAudioClip( audio ) );
}

/** Largest absolute sample the null device of the engine rendered since it was cleared */
Float32
RenderedPeak( AUDIO::CAudioEngine& engine )
{
    const AUDIO::CAudioBuffer& rendered = engine.GetDevice()->AsNullDevice()->GetRenderedAudio();
    const Float32* samples = static_cast< const Float32* >( rendered.GetDataPtr() );
    const size_t count = (size_t) rendered.GetFrameCount() * rendered.GetFormat().GetChannelCount();
    Float32 peak = 0.0f;
    for ( size_t i=0; i<count; ++i )
        peak = fabsf( samples[ i ] ) > peak ? fabsf( samples[ i ] ) : peak;
    return peak;
}

void
Render( AUDIO::CAudioEngine& engine, UInt32 frames )
{
    engine.GetDevice()->AsNullDevice()->Render( frames );
}

CORE::TTaskStatus
StatusOf( const CORE::CFutureResult& future )
{
    CORE::CTaskPtr task = future.GetResult( 1000 );
    return task.IsNULL() ? CORE::TTaskStatus::TASKSTATUS_UNDEFINED : task->GetTaskStatus();
}

/** Cooperative mode: Update() until the future is ready (the device is opened on a pool task meanwhile) */
bool
UpdateUntilReady( AUDIO::CAudioEngine& engine, const CORE::CFutureResult& future, UInt32 timeoutInMs = 10000 )
{
    for ( UInt32 waited=0; waited<timeoutInMs && !future.IsReady(); ++waited )
    {
        engine.Update();
        if ( !future.IsReady() )
            MT::ThreadDelay( 1 );
    }
    return future.IsReady();
}

/** What a scene of buses, a send, an effect and a looping tone looks like before a reopen */
struct SScene
{
    AUDIO::CAudioBus* music;
    AUDIO::CAudioBus* aux;
    AUDIO::TAudioVoicePtr tone;
    AUDIO::TAudioEffectPtr effect;
    SScene( void ) : music( GUCEF_NULL ), aux( GUCEF_NULL ), tone(), effect() {}
};

bool
BuildScene( AUDIO::CAudioEngine& engine, SScene& scene )
{
    scene.music = engine.CreateBus( "music" );
    scene.aux = engine.CreateBus( "aux" );
    if ( GUCEF_NULL == scene.music || GUCEF_NULL == scene.aux )
        return false;
    scene.effect = AUDIO::TAudioEffectPtr( GUCEF_NEW CTestAudioEffect() );
    if ( !scene.effect->Prepare( engine.GetMixFormat(), 4096 ) )
        return false;
    CORE::CFutureResult inserted = engine.InsertEffect( scene.music, 0, scene.effect );
    CORE::CFutureResult sent = engine.AddSend( scene.music, scene.aux, 0.5f, false );
    scene.tone = engine.CreateVoice( MakeTone( 4800, 0.3f ), scene.music );
    scene.tone->SetLoopCount( 0 );
    CORE::CFutureResult started = engine.Start( scene.tone );
    Render( engine, 1024 );
    engine.Update();
    return inserted.IsReady() && sent.IsReady() && started.IsReady() && !started.HasNoFuture();
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformReopenTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioEngineReopen: A synchronous reopen with another rate and channel count keeps buses, voices, sends and effects" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_FALSE( engine.ReopenDevice( NullParams( 44100, 6 ) ) );                     // not running
        ASSERT_TRUE( engine.Initialize( NullParams( 48000, 2 ) ) );
        SScene scene;
        ASSERT_TRUE( BuildScene( engine, scene ) );
        AUDIO::TAudioDevicePtr oldDevice = engine.GetDevice();
        const UInt64 positionBefore = scene.tone->GetPositionInFrames();
        ASSERT_TRUE( positionBefore > 0 && 1 == engine.GetActiveVoiceCount() );

        AUDIO::CAudioEngineParams wanted = NullParams( 44100, 6 );
        wanted.SetLimiterEnabled( false );
        wanted.SetMaxVoices( 3 );                                                           // ignored, with a log line
        ASSERT_TRUE( engine.ReopenDevice( wanted ) );

        ASSERT_TRUE( engine.GetDevice() != oldDevice );
        ASSERT_TRUE( 44100 == engine.GetMixFormat().GetSampleRate() && 6 == engine.GetMixFormat().GetChannelCount() );
        ASSERT_TRUE( 6 == engine.GetDeviceFormat().GetChannelCount() );
        ASSERT_TRUE( engine.FindBus( "music" ) == scene.music && engine.FindBus( "aux" ) == scene.aux );
        ASSERT_TRUE( 1 == engine.GetActiveVoiceCount() && AUDIO::AUDIOVOICESTATE_PLAYING == scene.tone->GetState() );
        ASSERT_TRUE( scene.effect->IsPreparedFor( engine.GetMixFormat(), 4096 ) );         // the effect follows the new format
        ASSERT_TRUE( 6 == scene.music->GetMeter().GetChannelCount() );

        // the tone plays on from where it was, through the effect and the send
        Render( engine, 2048 );
        engine.Update();
        ASSERT_TRUE( RenderedPeak( engine ) > 0.1f );
        ASSERT_TRUE( scene.tone->GetPositionInFrames() > positionBefore );
        Float32 auxPeak = 0.0f;
        for ( UInt32 c=0; c<scene.aux->GetMeter().GetChannelCount(); ++c )
            auxPeak = scene.aux->GetMeter().GetPeak( c ) > auxPeak ? scene.aux->GetMeter().GetPeak( c ) : auxPeak;
        ASSERT_TRUE( auxPeak > 0.05f );                                                     // the send still works (a mono tone is on the centre now)

        // and back, to the first format
        ASSERT_TRUE( engine.ReopenDevice( NullParams( 48000, 2 ) ) );
        Render( engine, 1024 );
        engine.Update();
        ASSERT_TRUE( 48000 == engine.GetMixFormat().GetSampleRate() && RenderedPeak( engine ) > 0.1f );
        ASSERT_TRUE( 1 == engine.GetActiveVoiceCount() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEngineReopen: An asynchronous reopen opens the device on a task and swaps it in Update(); one reopen at a time" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.ReopenDeviceAsync( NullParams( 44100, 6 ) ).HasNoFuture() );   // not running
        ASSERT_TRUE( engine.Initialize( NullParams( 48000, 2 ) ) );
        SScene scene;
        ASSERT_TRUE( BuildScene( engine, scene ) );

        CORE::CFutureResult reopened = engine.ReopenDeviceAsync( NullParams( 32000, 4 ) );
        ASSERT_FALSE( reopened.HasNoFuture() );
        ASSERT_FALSE( reopened.IsReady() );                                                 // the swap waits for the owner side

        // a second request while one is pending fails at once, synchronous or not
        CORE::CFutureResult second = engine.ReopenDeviceAsync( NullParams( 48000, 2 ) );
        ASSERT_TRUE( second.HasNoFuture() && CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED == StatusOf( second ) );
        ASSERT_FALSE( engine.ReopenDevice( NullParams( 48000, 2 ) ) );

        // the old device renders until the swap
        Render( engine, 256 );
        ASSERT_TRUE( UpdateUntilReady( engine, reopened ) );
        ASSERT_FALSE( reopened.HasNoFuture() );
        ASSERT_TRUE( CORE::TTaskStatus::TASKSTATUS_FINISHED == StatusOf( reopened ) );
        ASSERT_TRUE( 32000 == engine.GetMixFormat().GetSampleRate() && 4 == engine.GetMixFormat().GetChannelCount() );
        ASSERT_TRUE( 1 == engine.GetActiveVoiceCount() && engine.FindBus( "music" ) == scene.music );
        Render( engine, 2048 );
        engine.Update();
        ASSERT_TRUE( RenderedPeak( engine ) > 0.1f );

        // done: the next one is accepted again
        CORE::CFutureResult again = engine.ReopenDeviceAsync( NullParams( 48000, 2 ) );
        ASSERT_TRUE( UpdateUntilReady( engine, again ) && !again.HasNoFuture() );
        ASSERT_TRUE( 48000 == engine.GetMixFormat().GetSampleRate() );

        // a reopen still pending at shutdown is cancelled
        CORE::CFutureResult abandoned = engine.ReopenDeviceAsync( NullParams( 44100, 2 ) );
        engine.Shutdown();
        ASSERT_TRUE( abandoned.IsReady() && CORE::TTaskStatus::TASKSTATUS_CANCELLED == StatusOf( abandoned ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEngineReopen: A driver that does not exist fails the reopen, the old device keeps rendering" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams( 48000, 2 ) ) );
        SScene scene;
        ASSERT_TRUE( BuildScene( engine, scene ) );
        AUDIO::TAudioDevicePtr device = engine.GetDevice();

        AUDIO::CAudioEngineParams unknown = NullParams( 44100, 6 );
        unknown.SetDriverName( "nosuchdriver" );
        ASSERT_FALSE( engine.ReopenDevice( unknown ) );
        ASSERT_TRUE( engine.GetDevice() == device && 48000 == engine.GetMixFormat().GetSampleRate() );
        device->AsNullDevice()->ClearRenderedAudio();
        Render( engine, 1024 );
        ASSERT_TRUE( RenderedPeak( engine ) > 0.1f );

        CORE::CFutureResult failed = engine.ReopenDeviceAsync( unknown );
        ASSERT_FALSE( failed.HasNoFuture() );                                               // known only once the task has tried
        ASSERT_TRUE( UpdateUntilReady( engine, failed ) );
        ASSERT_TRUE( failed.HasNoFuture() && CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED == StatusOf( failed ) );
        ASSERT_TRUE( engine.GetDevice() == device );
        device->AsNullDevice()->ClearRenderedAudio();
        Render( engine, 1024 );
        ASSERT_TRUE( RenderedPeak( engine ) > 0.1f && 1 == engine.GetActiveVoiceCount() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEngineReopen: GetParams has the parameters of Initialize, those of a successful reopen, and keeps them after a failed one" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( AUDIO::CAudioEngineParams().GetMaxVoices() == engine.GetParams().GetMaxVoices() );                      // defaults before Initialize

        AUDIO::CAudioEngineParams first = NullParams( 48000, 2 );
        first.SetMaxVoices( 77 );
        first.GetDeviceParams().SetBufferSizeInFrames( 256 );
        ASSERT_TRUE( engine.Initialize( first ) );
        AUDIO::CAudioEngineParams inUse = engine.GetParams();
        ASSERT_TRUE( "null" == inUse.GetDriverName() && 77 == inUse.GetMaxVoices() && AUDIO::AUDIOTHREADING_COOPERATIVE == inUse.GetThreadingMode() );
        ASSERT_TRUE( 48000 == inUse.GetDeviceParams().GetPreferredFormat().GetSampleRate() && 2 == inUse.GetDeviceParams().GetPreferredFormat().GetChannelCount() );
        ASSERT_TRUE( 256 == inUse.GetDeviceParams().GetBufferSizeInFrames() && inUse.GetLimiterEnabled() && AUDIO::AUDIODIRECTION_PLAYBACK == inUse.GetDeviceParams().GetDirection() );

        // a copy: changing it does not change the engine
        inUse.SetMaxVoices( 5 );
        ASSERT_TRUE( 77 == engine.GetParams().GetMaxVoices() );

        // a successful reopen, synchronous and asynchronous: what is derived from the params in use keeps the rest
        AUDIO::CAudioEngineParams second = engine.GetParams();
        second.GetDeviceParams().SetPreferredFormat( AUDIO::CAudioFormat( 44100, 4, AUDIO::SAMPLETYPE_FLOAT32 ) );
        second.GetDeviceParams().SetBufferSizeInFrames( 512 );
        second.SetLimiterEnabled( false );
        ASSERT_TRUE( engine.ReopenDevice( second ) );
        AUDIO::CAudioEngineParams after = engine.GetParams();
        ASSERT_TRUE( 44100 == after.GetDeviceParams().GetPreferredFormat().GetSampleRate() && 4 == after.GetDeviceParams().GetPreferredFormat().GetChannelCount() );
        ASSERT_TRUE( 512 == after.GetDeviceParams().GetBufferSizeInFrames() && !after.GetLimiterEnabled() && 77 == after.GetMaxVoices() && "null" == after.GetDriverName() );

        AUDIO::CAudioEngineParams third = engine.GetParams();
        third.GetDeviceParams().SetPreferredFormat( AUDIO::CAudioFormat( 32000, 2, AUDIO::SAMPLETYPE_FLOAT32 ) );
        third.SetLimiterEnabled( true );
        CORE::CFutureResult reopened = engine.ReopenDeviceAsync( third );
        ASSERT_FALSE( reopened.HasNoFuture() );
        ASSERT_TRUE( 44100 == engine.GetParams().GetDeviceParams().GetPreferredFormat().GetSampleRate() );     // not yet swapped in
        ASSERT_TRUE( UpdateUntilReady( engine, reopened ) && !reopened.HasNoFuture() );
        after = engine.GetParams();
        ASSERT_TRUE( 32000 == after.GetDeviceParams().GetPreferredFormat().GetSampleRate() && after.GetLimiterEnabled() && 512 == after.GetDeviceParams().GetBufferSizeInFrames() );

        // failed ones, synchronous and asynchronous: the params in use do not move
        AUDIO::CAudioEngineParams unknown = engine.GetParams();
        unknown.SetDriverName( "nosuchdriver" );
        unknown.GetDeviceParams().SetPreferredFormat( AUDIO::CAudioFormat( 22050, 2, AUDIO::SAMPLETYPE_FLOAT32 ) );
        unknown.SetLimiterEnabled( false );
        ASSERT_FALSE( engine.ReopenDevice( unknown ) );
        after = engine.GetParams();
        ASSERT_TRUE( "null" == after.GetDriverName() && 32000 == after.GetDeviceParams().GetPreferredFormat().GetSampleRate() && after.GetLimiterEnabled() );
        CORE::CFutureResult failed = engine.ReopenDeviceAsync( unknown );
        ASSERT_TRUE( UpdateUntilReady( engine, failed ) && failed.HasNoFuture() );
        after = engine.GetParams();
        ASSERT_TRUE( "null" == after.GetDriverName() && 32000 == after.GetDeviceParams().GetPreferredFormat().GetSampleRate() && after.GetLimiterEnabled() );

        // the engine stops: the params stay readable
        engine.Shutdown();
        ASSERT_TRUE( 32000 == engine.GetParams().GetDeviceParams().GetPreferredFormat().GetSampleRate() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEngineReopen: Threaded mode: the engine thread completes the reopen, nobody calls Update()" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams( 48000, 2, true ) ) );
        ASSERT_TRUE( engine.IsThreaded() );
        AUDIO::CAudioBus* music = engine.CreateBus( "music" );
        AUDIO::TAudioVoicePtr tone = engine.CreateVoice( MakeTone( 4800, 0.3f ), music );
        tone->SetLoopCount( 0 );
        CORE::CFutureResult started = engine.Start( tone );
        for ( UInt32 i=0; i<5000 && !started.IsReady(); ++i )
        {
            Render( engine, 64 );
            MT::ThreadDelay( 1 );
        }
        ASSERT_TRUE( started.IsReady() );

        CORE::CFutureResult reopened = engine.ReopenDeviceAsync( NullParams( 44100, 6, true ) );
        ASSERT_FALSE( reopened.HasNoFuture() );
        for ( UInt32 i=0; i<10000 && !reopened.IsReady(); ++i )
            MT::ThreadDelay( 1 );                                                           // no Update() here: the engine thread does it
        ASSERT_TRUE( reopened.IsReady() && !reopened.HasNoFuture() );
        ASSERT_TRUE( 44100 == engine.GetMixFormat().GetSampleRate() && engine.FindBus( "music" ) == music );
        Render( engine, 2048 );
        ASSERT_TRUE( RenderedPeak( engine ) > 0.1f && 1 == engine.GetActiveVoiceCount() );
        engine.Shutdown();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEngineReopen: The limiter mode of a running engine changes with a reopen, synchronous or asynchronous; a failed reopen keeps it" )
    try
    {
        AUDIO::CAudioEngine engine;
        AUDIO::CAudioEngineParams first = NullParams( 48000, 2 );
        first.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_INSTANT );
        ASSERT_TRUE( engine.Initialize( first ) );
        ASSERT_TRUE( 0 == engine.GetMixLatencyInFrames() && AUDIO::AUDIOLIMITERMODE_INSTANT == engine.GetParams().GetLimiterMode() );

        // three loud tones that play on through every reopen
        AUDIO::TAudioVoicePtr tones[ 3 ];
        for ( UInt32 i=0; i<3; ++i )
        {
            tones[ i ] = engine.CreateVoice( MakeTone( 4800, 0.9f ) );
            tones[ i ]->SetLoopCount( 0 );
            engine.Start( tones[ i ] );
        }
        Render( engine, 4096 );
        engine.Update();
        ASSERT_TRUE( 3 == engine.GetActiveVoiceCount() );
        ASSERT_TRUE( Near( 0.98f, RenderedPeak( engine ), 1e-4f ) );                       // instant: 0.98

        // to lookahead, synchronously: the latency follows, the peak is the ceiling
        AUDIO::CAudioEngineParams wanted = engine.GetParams();
        wanted.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_LOOKAHEAD );
        ASSERT_TRUE( engine.ReopenDevice( wanted ) );
        ASSERT_TRUE( 72 == engine.GetMixLatencyInFrames() && AUDIO::AUDIOLIMITERMODE_LOOKAHEAD == engine.GetParams().GetLimiterMode() );
        Render( engine, 4096 );
        engine.Update();
        const Float64 ceiling = pow( 10.0, -0.3 / 20.0 );
        ASSERT_TRUE( (Float64) RenderedPeak( engine ) <= ceiling && (Float64) RenderedPeak( engine ) > 0.9 * ceiling );

        // to off, asynchronously: nothing limits, the clamp cuts off at full level
        wanted = engine.GetParams();
        wanted.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_OFF );
        CORE::CFutureResult reopened = engine.ReopenDeviceAsync( wanted );
        ASSERT_TRUE( UpdateUntilReady( engine, reopened ) && CORE::TTaskStatus::TASKSTATUS_FINISHED == StatusOf( reopened ) );
        ASSERT_TRUE( 0 == engine.GetMixLatencyInFrames() && AUDIO::AUDIOLIMITERMODE_OFF == engine.GetParams().GetLimiterMode() );
        Render( engine, 4096 );
        engine.Update();
        ASSERT_TRUE( 1.0f == RenderedPeak( engine ) );

        // switched on again it is the mode it had, here at another rate, lookahead and ceiling
        wanted = engine.GetParams();
        wanted.SetLimiterEnabled( true );
        wanted.SetLimiterLookaheadInMs( 4.0f );
        wanted.SetLimiterCeilingInDecibels( -2.0f );
        wanted.GetDeviceParams().SetPreferredFormat( AUDIO::CAudioFormat( 44100, 2, AUDIO::SAMPLETYPE_FLOAT32 ) );
        ASSERT_TRUE( engine.ReopenDevice( wanted ) );
        ASSERT_TRUE( 44100 == engine.GetMixFormat().GetSampleRate() );
        ASSERT_TRUE( 176 == engine.GetMixLatencyInFrames() && AUDIO::AUDIOLIMITERMODE_LOOKAHEAD == engine.GetParams().GetLimiterMode() );   // 4 ms at 44.1 kHz
        ASSERT_TRUE( Near( 4.0f, engine.GetParams().GetLimiterLookaheadInMs(), 1e-6f ) && Near( -2.0f, engine.GetParams().GetLimiterCeilingInDecibels(), 1e-6f ) );
        Render( engine, 4096 );
        engine.Update();
        ASSERT_TRUE( (Float64) RenderedPeak( engine ) <= pow( 10.0, -2.0 / 20.0 ) );

        // a reopen that fails changes nothing, the limiter included
        AUDIO::CAudioEngineParams unknown = engine.GetParams();
        unknown.SetDriverName( "nosuchdriver" );
        unknown.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_INSTANT );
        ASSERT_FALSE( engine.ReopenDevice( unknown ) );
        ASSERT_TRUE( 176 == engine.GetMixLatencyInFrames() && AUDIO::AUDIOLIMITERMODE_LOOKAHEAD == engine.GetParams().GetLimiterMode() );
        Render( engine, 4096 );
        engine.Update();
        ASSERT_TRUE( (Float64) RenderedPeak( engine ) <= pow( 10.0, -2.0 / 20.0 ) && 3 == engine.GetActiveVoiceCount() );
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
PerformAudioEngineReopenTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioEngineReopen TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.EngineReopen" );

    PerformReopenTests();
}

/*-------------------------------------------------------------------------*/
