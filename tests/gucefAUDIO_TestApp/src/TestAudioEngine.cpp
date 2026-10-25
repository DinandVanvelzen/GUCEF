/*
 *  gucefAUDIO_TestApp: GUCEF test application for the AUDIO module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

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

#include "TestAudioEngine.h"

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
Near( Float32 a, Float32 b, Float32 tolerance = 1e-5f )
{
    return fabsf( a - b ) <= tolerance;
}

AUDIO::TAudioClipPtr
MakeConstant( UInt32 rate, UInt32 channels, UInt32 frames, Float32 value )
{
    AUDIO::CAudioBuffer audio( AUDIO::CAudioFormat( rate, channels, AUDIO::SAMPLETYPE_FLOAT32 ), frames );
    for ( UInt32 f=0; f<frames; ++f )
        for ( UInt32 c=0; c<channels; ++c )
            audio.SetSampleFromFloat( f, c, value );
    return AUDIO::TAudioClipPtr( GUCEF_NEW AUDIO::CAudioClip( audio ) );
}

/** Parameters for an engine on the null driver that keeps what it plays so the test can look at it */
AUDIO::CAudioEngineParams
NullParams( AUDIO::TAudioThreadingMode mode = AUDIO::AUDIOTHREADING_COOPERATIVE )
{
    AUDIO::CAudioEngineParams params;
    params.SetDriverName( "null" );
    params.SetThreadingMode( mode );
    params.GetDeviceParams().SetBufferSizeInFrames( 256 );
    params.GetDeviceParams().GetOptions().Set( "keepOutput", "true" );
    return params;
}

AUDIO::CNullAudioDevice*
Null( AUDIO::CAudioEngine& engine )
{
    AUDIO::TAudioDevicePtr device = engine.GetDevice();
    return ( device.IsNULL() ? GUCEF_NULL : device->AsNullDevice() );
}

/** The audio thread does its work (the null device is driven by hand) and then the application side catches up */
void
Run( AUDIO::CAudioEngine& engine, UInt32 frames )
{
    Null( engine )->Render( frames );
    engine.Update();

    // events of the engine are passed on to the thread of the application's pulse generator, which is this one: let it deliver
    CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
}

/** A refused future is in an error status, which is what HasNoFuture() tells (ready or not: a throttle stays pending) */
bool
IsFailed( const CORE::CFutureResult& future )
{
    return future.HasNoFuture();
}

/** The status of a future that has ended */
CORE::TTaskStatus
StatusOf( const CORE::CFutureResult& future )
{
    CORE::CTaskPtr task = future.GetResult( 1000 );
    return task.IsNULL() ? CORE::TTaskStatus::TASKSTATUS_UNDEFINED : task->GetTaskStatus();
}

class CEngineObserver : public CORE::CObserver
{
    public:

    CEngineObserver( void ) : CORE::CObserver(), ended(), lost( 0 ) {}

    GUCEF::vector< AUDIO::TAudioVoicePtr > ended;
    UInt32 lost;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( AUDIO::CAudioEngine::VoiceEndedEvent == eventid )
            ended.push_back( static_cast< AUDIO::CAudioEngine::TVoiceEventData* >( eventdata )->GetDataCopy() );
        else if ( AUDIO::CAudioEngine::DeviceLostEvent == eventid )
            ++lost;
    }
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformLifecycleTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioEngine: Starts on a driver and device, reports its formats, shuts down and starts again" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_FALSE( engine.IsInitialized() );
        ASSERT_TRUE( GUCEF_NULL == engine.GetMasterBus() );
        ASSERT_TRUE( 0 == engine.Update() );

        ASSERT_TRUE( engine.Initialize( NullParams() ) );
        ASSERT_TRUE( engine.IsInitialized() && !engine.IsThreaded() );
        ASSERT_TRUE( engine.GetDriverName() == "null" );
        ASSERT_TRUE( engine.GetDeviceFormat() == AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 ) );
        ASSERT_TRUE( engine.GetMixFormat() == engine.GetDeviceFormat() );
        ASSERT_TRUE( GUCEF_NULL != engine.GetMasterBus() && engine.GetMasterBus()->GetName() == "master" );
        ASSERT_TRUE( engine.GetDevice()->IsStarted() );
        ASSERT_FALSE( engine.Initialize( NullParams() ) );                      // already running

        engine.Shutdown();
        ASSERT_FALSE( engine.IsInitialized() );
        ASSERT_TRUE( engine.GetDriverName().IsNULLOrEmpty() );
        engine.Shutdown();                                                       // harmless again
        ASSERT_TRUE( engine.Initialize( NullParams() ) );

        // the device is not started when told not to
        AUDIO::CAudioEngine idle;
        AUDIO::CAudioEngineParams params = NullParams();
        params.SetStartDevice( false );
        ASSERT_TRUE( idle.Initialize( params ) );
        ASSERT_FALSE( idle.GetDevice()->IsStarted() );
        ASSERT_TRUE( idle.StartDevice() && idle.GetDevice()->IsStarted() );
        ASSERT_TRUE( idle.StopDevice() && !idle.GetDevice()->IsStarted() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEngine: An unknown driver, and no driver at all, are handled" )
    try
    {
        AUDIO::CAudioEngine engine;
        AUDIO::CAudioEngineParams params = NullParams();
        params.SetDriverName( "nosuchdriver" );
        ASSERT_FALSE( engine.Initialize( params ) );
        ASSERT_FALSE( engine.IsInitialized() );

        // with no driver named the best one is found: the null driver is the one that is always there
        AUDIO::CAudioEngine any;
        AUDIO::CAudioEngineParams open;
        open.SetThreadingMode( AUDIO::AUDIOTHREADING_COOPERATIVE );
        ASSERT_TRUE( any.Initialize( open ) );
        ASSERT_FALSE( any.GetDriverName().IsNULLOrEmpty() );

        // starting things on an engine that is not running is refused
        AUDIO::CAudioEngine off;
        AUDIO::TAudioVoicePtr voice( GUCEF_NEW AUDIO::CAudioVoice( MakeConstant( 48000, 1, 10, 0.1f )->CreateSource() ) );
        ASSERT_TRUE( IsFailed( off.Start( voice ) ) );
        ASSERT_TRUE( GUCEF_NULL == off.CreateBus( "x" ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformPlaybackTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioEngine: Play gives a future that is ready once the audio thread has the sound, and the sound is heard" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams() ) );

        AUDIO::TAudioVoicePtr voice;
        CORE::CFutureResult started = engine.Play( MakeConstant( 48000, 1, 1000, 0.5f ), GUCEF_NULL, &voice );
        ASSERT_FALSE( voice.IsNULL() );
        ASSERT_FALSE( started.IsReady() );                                      // the audio thread has not looked yet
        ASSERT_TRUE( AUDIO::AUDIOVOICESTATE_IDLE == voice->GetState() );
        ASSERT_TRUE( 1 == engine.GetActiveVoiceCount() );

        Null( engine )->Render( 256 );                                          // the audio thread picks the command up and mixes
        ASSERT_TRUE( AUDIO::AUDIOVOICESTATE_PLAYING == voice->GetState() );
        ASSERT_FALSE( started.IsReady() );                                      // not until the application side has heard of it
        ASSERT_TRUE( engine.WaitFor( started, 2000 ) );                         // WaitFor pumps Update() in cooperative mode
        ASSERT_TRUE( CORE::TTaskStatus::TASKSTATUS_FINISHED == StatusOf( started ) );

        const AUDIO::CAudioBuffer& heard = Null( engine )->GetRenderedAudio();
        ASSERT_TRUE( 256 == heard.GetFrameCount() );
        ASSERT_TRUE( Near( 0.5f, heard.GetSampleAsFloat( 0, 0 ) ) && Near( 0.5f, heard.GetSampleAsFloat( 255, 1 ) ) );

        // the rest of the sound and then silence
        Null( engine )->Render( 1000 );
        const AUDIO::CAudioBuffer& all = Null( engine )->GetRenderedAudio();
        ASSERT_TRUE( Near( 0.5f, all.GetSampleAsFloat( 999, 0 ) ) && Near( 0.0f, all.GetSampleAsFloat( 1000, 0 ) ) && Near( 0.0f, all.GetSampleAsFloat( 1255, 1 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEngine: A voice that ends reports it: ended future, notifier event, and it is let go of" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams() ) );
        CEngineObserver observer;
        observer.SubscribeTo( &engine );

        AUDIO::TAudioVoicePtr voice;
        engine.Play( MakeConstant( 48000, 1, 300, 0.2f ), GUCEF_NULL, &voice );
        CORE::CFutureResult ended = engine.GetEndedResult( voice );
        ASSERT_FALSE( ended.IsReady() );
        Run( engine, 256 );
        ASSERT_FALSE( ended.IsReady() );                                         // 300 frames, so one block is not enough
        ASSERT_TRUE( observer.ended.empty() );
        Run( engine, 256 );
        ASSERT_TRUE( ended.IsReady() );
        ASSERT_TRUE( AUDIO::AUDIOVOICESTATE_ENDED == voice->GetState() && AUDIO::AUDIOVOICEEND_FINISHED == voice->GetEndReason() );
        ASSERT_TRUE( 1 == observer.ended.size() && observer.ended[ 0 ].GetPointerAlways() == voice.GetPointerAlways() );
        ASSERT_TRUE( 0 == engine.GetActiveVoiceCount() );
        ASSERT_TRUE( engine.GetEndedResult( voice ).IsReady() );                  // asking after the end gives a ready future
        observer.UnsubscribeAllFromObserver();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEngine: Stop, pause and resume are futures that complete when the audio thread has done it" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams() ) );

        AUDIO::TAudioVoicePtr voice;
        engine.Play( MakeConstant( 48000, 1, 100000, 0.25f ), GUCEF_NULL, &voice );
        Run( engine, 256 );

        CORE::CFutureResult paused = engine.Pause( voice );
        ASSERT_FALSE( paused.IsReady() );
        Run( engine, 256 );
        ASSERT_TRUE( paused.IsReady() && AUDIO::AUDIOVOICESTATE_PAUSED == voice->GetState() );
        const AUDIO::CAudioBuffer& heard = Null( engine )->GetRenderedAudio();
        ASSERT_TRUE( Near( 0.0f, heard.GetSampleAsFloat( 256 + 100, 0 ) ) );     // silent while paused

        CORE::CFutureResult resumed = engine.Resume( voice );
        Run( engine, 256 );
        ASSERT_TRUE( resumed.IsReady() && AUDIO::AUDIOVOICESTATE_PLAYING == voice->GetState() );
        ASSERT_TRUE( Near( 0.25f, Null( engine )->GetRenderedAudio().GetSampleAsFloat( 256 + 256 + 100, 0 ) ) );

        CORE::CFutureResult stopped = engine.Stop( voice );
        ASSERT_FALSE( stopped.IsReady() );
        Run( engine, 256 );
        ASSERT_TRUE( stopped.IsReady() && AUDIO::AUDIOVOICEEND_STOPPED == voice->GetEndReason() );
        ASSERT_TRUE( 0 == engine.GetActiveVoiceCount() );

        // a voice that is not playing: stop is ready at once, pause and the ended result are refused
        ASSERT_TRUE( engine.Stop( voice ).IsReady() );
        ASSERT_TRUE( IsFailed( engine.Pause( voice ) ) );
        AUDIO::TAudioVoicePtr never = engine.CreateVoice( MakeConstant( 48000, 1, 10, 0.1f ) );
        ASSERT_TRUE( engine.Stop( never ).IsReady() );
        ASSERT_TRUE( IsFailed( engine.GetEndedResult( never ) ) );
        ASSERT_TRUE( IsFailed( engine.Stop( AUDIO::TAudioVoicePtr() ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEngine: A voice plays once: starting it twice, or a stopped one, is refused" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams() ) );
        AUDIO::TAudioVoicePtr voice = engine.CreateVoice( MakeConstant( 48000, 1, 100000, 0.1f ) );
        CORE::CFutureResult first = engine.Start( voice );
        ASSERT_FALSE( first.IsReady() );
        ASSERT_TRUE( IsFailed( engine.Start( voice ) ) );                        // already started
        Run( engine, 256 );
        ASSERT_TRUE( first.IsReady() && !IsFailed( first ) );
        ASSERT_TRUE( IsFailed( engine.Start( voice ) ) );                        // playing
        engine.Stop( voice );
        Run( engine, 256 );
        ASSERT_TRUE( IsFailed( engine.Start( voice ) ) );                        // over
        ASSERT_TRUE( IsFailed( engine.Start( AUDIO::TAudioVoicePtr() ) ) );
        ASSERT_TRUE( IsFailed( engine.Play( AUDIO::TAudioClipPtr() ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformBusAndVolumeTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioEngine: Buses made through the engine are in the tree by the time a voice on them plays" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams() ) );
        AUDIO::CAudioBus* music = engine.CreateBus( "music" );
        AUDIO::CAudioBus* effects = engine.CreateBus( "effects" );
        AUDIO::CAudioBus* ui = engine.CreateBus( "ui", effects );
        ASSERT_TRUE( GUCEF_NULL != music && GUCEF_NULL != effects && GUCEF_NULL != ui && ui->GetParent() == effects );
        ASSERT_TRUE( engine.FindBus( "ui" ) == ui && engine.FindBus( "master" ) == engine.GetMasterBus() && GUCEF_NULL == engine.FindBus( "missing" ) );

        AUDIO::CAudioEngine other;
        ASSERT_TRUE( other.Initialize( NullParams() ) );
        ASSERT_TRUE( GUCEF_NULL == engine.CreateBus( "stranger", other.GetMasterBus() ) );

        // all in one go, no render in between: the order of commands makes it work
        AUDIO::TAudioVoicePtr song = engine.CreateVoice( MakeConstant( 48000, 1, 100000, 0.2f ), music );
        AUDIO::TAudioVoicePtr click = engine.CreateVoice( MakeConstant( 48000, 1, 100000, 0.1f ), ui );
        CORE::CFutureResult a = engine.Start( song ), b = engine.Start( click );
        Run( engine, 512 );
        ASSERT_TRUE( a.IsReady() && b.IsReady() && !IsFailed( a ) && !IsFailed( b ) );
        ASSERT_TRUE( Near( 0.3f, Null( engine )->GetRenderedAudio().GetSampleAsFloat( 100, 0 ) ) );

        music->SetVolume( 0.5f );
        effects->SetMuted( true );
        Run( engine, 512 );
        Null( engine )->ClearRenderedAudio();
        Run( engine, 512 );
        ASSERT_TRUE( Near( 0.1f, Null( engine )->GetRenderedAudio().GetSampleAsFloat( 300, 1 ) ) );

        engine.SetMasterVolume( 0.5f );
        ASSERT_TRUE( Near( 0.5f, engine.GetMasterVolume() ) );
        Run( engine, 512 );
        Null( engine )->ClearRenderedAudio();
        Run( engine, 512 );
        ASSERT_TRUE( Near( 0.05f, Null( engine )->GetRenderedAudio().GetSampleAsFloat( 300, 1 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEngine: Bus limit and the order of buses are respected" )
    try
    {
        AUDIO::CAudioEngine engine;
        AUDIO::CAudioEngineParams params = NullParams();
        params.SetMaxBuses( 3 );
        ASSERT_TRUE( engine.Initialize( params ) );
        ASSERT_TRUE( GUCEF_NULL != engine.CreateBus( "one" ) );
        ASSERT_TRUE( GUCEF_NULL != engine.CreateBus( "two" ) );
        ASSERT_TRUE( GUCEF_NULL == engine.CreateBus( "three" ) );               // master, one, two: that is the three
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformLimitTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioEngine: When the mixer has no free voice the start future fails and the voice is let go of" )
    try
    {
        AUDIO::CAudioEngine engine;
        AUDIO::CAudioEngineParams params = NullParams();
        params.SetMaxVoices( 2 );
        ASSERT_TRUE( engine.Initialize( params ) );

        GUCEF::vector< CORE::CFutureResult > futures;
        GUCEF::vector< AUDIO::TAudioVoicePtr > voices;
        for ( UInt32 i=0; i<4; ++i )
        {
            AUDIO::TAudioVoicePtr voice;
            futures.push_back( engine.Play( MakeConstant( 48000, 1, 100000, 0.1f ), GUCEF_NULL, &voice ) );
            voices.push_back( voice );
        }
        Run( engine, 256 );
        for ( UInt32 i=0; i<4; ++i )
            ASSERT_TRUE( futures[ i ].IsReady() );
        ASSERT_FALSE( IsFailed( futures[ 0 ] ) || IsFailed( futures[ 1 ] ) );
        ASSERT_TRUE( IsFailed( futures[ 2 ] ) && IsFailed( futures[ 3 ] ) );
        ASSERT_TRUE( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED == StatusOf( futures[ 2 ] ) );     // the mixer had no room: an end state
        ASSERT_TRUE( 2 == engine.GetActiveVoiceCount() );
        ASSERT_TRUE( Near( 0.2f, Null( engine )->GetRenderedAudio().GetSampleAsFloat( 10, 0 ) ) );     // the two that got in play
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEngine: A full command queue and too many outstanding voices are refused at once, nothing is lost" )
    try
    {
        AUDIO::CAudioEngine engine;
        AUDIO::CAudioEngineParams params = NullParams();
        params.SetCommandQueueSize( 4 );
        params.SetEventQueueSize( 16 );          // 5 voices may be outstanding
        ASSERT_TRUE( engine.Initialize( params ) );

        UInt32 accepted = 0, refused = 0;
        GUCEF::vector< CORE::CFutureResult > futures;
        for ( UInt32 i=0; i<10; ++i )
        {
            CORE::CFutureResult f = engine.Play( MakeConstant( 48000, 1, 100000, 0.01f ) );
            futures.push_back( f );
            if ( IsFailed( f ) )
                ++refused;
            else
                ++accepted;
        }
        ASSERT_TRUE( 4 == accepted );            // the queue holds four commands until the audio thread comes by
        ASSERT_TRUE( 6 == refused );
        ASSERT_TRUE( CORE::TTaskStatus::TASKSTATUS_QUEUEING_FAILED == StatusOf( futures[ 9 ] ) );

        Run( engine, 256 );
        for ( UInt32 i=0; i<4; ++i )
            ASSERT_TRUE( futures[ i ].IsReady() && !IsFailed( futures[ i ] ) );
        ASSERT_TRUE( 0 == engine.GetDroppedEventCount() );

        // outstanding limit: at most 5 voices that were started and not reported ended
        CORE::CFutureResult fifth = engine.Play( MakeConstant( 48000, 1, 100000, 0.01f ) );
        Run( engine, 256 );
        ASSERT_TRUE( fifth.IsReady() && !IsFailed( fifth ) );
        CORE::CFutureResult sixth = engine.Play( MakeConstant( 48000, 1, 100000, 0.01f ) );
        ASSERT_TRUE( IsFailed( sixth ) && !sixth.IsReady() );                                  // a throttle: pending, in an error status
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEngine: Many voices at once all play and all end, and add up" )
    try
    {
        AUDIO::CAudioEngine engine;
        AUDIO::CAudioEngineParams params = NullParams();
        params.SetMaxVoices( 128 );
        ASSERT_TRUE( engine.Initialize( params ) );
        CEngineObserver observer;
        observer.SubscribeTo( &engine );

        const UInt32 count = 100;
        GUCEF::vector< CORE::CFutureResult > futures;
        for ( UInt32 i=0; i<count; ++i )
            futures.push_back( engine.Play( MakeConstant( 48000, 1, 300 + i, 0.005f ) ) );
        Run( engine, 256 );
        for ( UInt32 i=0; i<count; ++i )
            ASSERT_TRUE( futures[ i ].IsReady() && !IsFailed( futures[ i ] ) );
        ASSERT_TRUE( Near( 0.5f, Null( engine )->GetRenderedAudio().GetSampleAsFloat( 10, 0 ), 1e-4f ) );
        Run( engine, 1024 );
        ASSERT_TRUE( 0 == engine.GetActiveVoiceCount() );
        ASSERT_TRUE( count == observer.ended.size() );
        observer.UnsubscribeAllFromObserver();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformDeviceFormatTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioEngine: Sound is mixed in the format of the device: 16 bit mono at another rate" )
    try
    {
        AUDIO::CAudioEngine engine;
        AUDIO::CAudioEngineParams params = NullParams();
        params.GetDeviceParams().SetPreferredFormat( AUDIO::CAudioFormat( 22050, 1, AUDIO::SAMPLETYPE_INT16 ) );
        ASSERT_TRUE( engine.Initialize( params ) );
        ASSERT_TRUE( engine.GetDeviceFormat() == AUDIO::CAudioFormat( 22050, 1, AUDIO::SAMPLETYPE_INT16 ) );
        ASSERT_TRUE( engine.GetMixFormat() == AUDIO::CAudioFormat( 22050, 1, AUDIO::SAMPLETYPE_FLOAT32 ) );

        // a stereo sound at 44100 Hz to a mono 22050 Hz 16 bit device
        AUDIO::TAudioVoicePtr voice;
        CORE::CFutureResult started = engine.Play( MakeConstant( 44100, 2, 4000, 0.5f ), GUCEF_NULL, &voice );
        Run( engine, 512 );
        ASSERT_TRUE( started.IsReady() );
        const AUDIO::CAudioBuffer& heard = Null( engine )->GetRenderedAudio();
        ASSERT_TRUE( AUDIO::SAMPLETYPE_INT16 == heard.GetFormat().GetSampleType() && 1 == heard.GetFormat().GetChannelCount() );
        ASSERT_TRUE( 512 == heard.GetFrameCount() );
        const Int16* samples = static_cast< const Int16* >( heard.GetDataPtr() );
        ASSERT_TRUE( 16384 == samples[ 100 ] && 16384 == samples[ 400 ] );       // 0.5 as a 16 bit number
        ASSERT_TRUE( Near( 1024.0f, (Float32) voice->GetPositionInFrames(), 8.0f ) );        // 512 device frames at 22050 Hz are 1024 source frames at 44100 Hz
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEngine: Eight bit unsigned and 24 bit devices get their samples right too" )
    try
    {
        const AUDIO::TSampleType types[ 2 ] = { AUDIO::SAMPLETYPE_UINT8, AUDIO::SAMPLETYPE_INT24 };
        for ( UInt32 t=0; t<2; ++t )
        {
            AUDIO::CAudioEngine engine;
            AUDIO::CAudioEngineParams params = NullParams();
            params.GetDeviceParams().SetPreferredFormat( AUDIO::CAudioFormat( 48000, 2, types[ t ] ) );
            ASSERT_TRUE( engine.Initialize( params ) );
            CORE::CFutureResult started = engine.Play( MakeConstant( 48000, 2, 4000, -0.5f ) );
            Run( engine, 256 );
            ASSERT_TRUE( started.IsReady() );
            ASSERT_TRUE( Near( -0.5f, Null( engine )->GetRenderedAudio().GetSampleAsFloat( 100, 1 ), 1e-3f ) );
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
PerformShutdownAndLossTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioEngine: Shutdown cancels what was not done yet" )
    try
    {
        GUCEF::vector< CORE::CFutureResult > pending;       // started, ended, pause
        {
            AUDIO::CAudioEngine engine;
            ASSERT_TRUE( engine.Initialize( NullParams() ) );
            AUDIO::TAudioVoicePtr running, waiting;
            engine.Play( MakeConstant( 48000, 1, 100000, 0.1f ), GUCEF_NULL, &running );
            Run( engine, 256 );                                                  // this one is playing
            pending.push_back( engine.Play( MakeConstant( 48000, 1, 100000, 0.1f ), GUCEF_NULL, &waiting ) );   // this one the audio thread has not seen
            pending.push_back( engine.GetEndedResult( running ) );
            pending.push_back( engine.Pause( running ) );
            ASSERT_FALSE( pending[ 0 ].IsReady() || pending[ 1 ].IsReady() || pending[ 2 ].IsReady() );
            engine.Shutdown();
            ASSERT_TRUE( pending[ 0 ].IsReady() && pending[ 1 ].IsReady() && pending[ 2 ].IsReady() );
        }
        ASSERT_TRUE( CORE::TTaskStatus::TASKSTATUS_CANCELLED == StatusOf( pending[ 0 ] ) );
        ASSERT_TRUE( CORE::TTaskStatus::TASKSTATUS_CANCELLED == StatusOf( pending[ 1 ] ) );
        ASSERT_TRUE( CORE::TTaskStatus::TASKSTATUS_CANCELLED == StatusOf( pending[ 2 ] ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEngine: Loss of the device is raised as an event, once" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams() ) );
        CEngineObserver observer;
        observer.SubscribeTo( &engine );
        engine.Update();
        CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        ASSERT_TRUE( 0 == observer.lost );
        Null( engine )->SimulateDeviceLoss();
        ASSERT_TRUE( 1 == engine.Update() );
        CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        ASSERT_TRUE( 1 == observer.lost );
        ASSERT_TRUE( 0 == engine.Update() );
        CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        ASSERT_TRUE( 1 == observer.lost );
        observer.UnsubscribeAllFromObserver();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformThreadedTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioEngine: In threaded mode the engine thread completes futures without anyone calling Update, and the task manager knows the thread" )
    try
    {
        CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
        AUDIO::CAudioEngine engine;
        AUDIO::CAudioEngineParams params = NullParams( AUDIO::AUDIOTHREADING_THREADED );
        params.SetThreadPoolName( "AudioEngineTest" );
        params.SetUpdateIntervalInMs( 2 );
        ASSERT_TRUE( engine.Initialize( params ) );
        ASSERT_TRUE( engine.IsThreaded() );

        CORE::CThreadPoolInfo poolInfo;
        ASSERT_TRUE( taskManager.GetThreadPoolInfo( "AudioEngineTest", poolInfo ) );
        ASSERT_TRUE( 1 == poolInfo.GetActiveNrOfDedicatedThreads() || poolInfo.GetActiveNrOfWorkerThreads() >= 1 );

        AUDIO::TAudioVoicePtr voice;
        CORE::CFutureResult started = engine.Play( MakeConstant( 48000, 1, 600, 0.3f ), GUCEF_NULL, &voice );
        GUCEF::vector< CORE::CFutureResult > ended;                              // made once the voice has started
        UInt32 waited = 0;
        while ( !( started.IsReady() && !ended.empty() && ended[ 0 ].IsReady() ) && waited < 5000 )
        {
            Null( engine )->Render( 128 );                                       // only the audio thread is driven here, never Update()
            if ( started.IsReady() && ended.empty() )
                ended.push_back( engine.GetEndedResult( voice ) );
            MT::PrecisionDelay( 1 );
            ++waited;
        }
        ASSERT_TRUE( started.IsReady() && !IsFailed( started ) );
        ASSERT_TRUE( !ended.empty() && ended[ 0 ].IsReady() );
        ASSERT_TRUE( AUDIO::AUDIOVOICEEND_FINISHED == voice->GetEndReason() );
        ASSERT_TRUE( engine.WaitFor( started, 100 ) );

        engine.Shutdown();
        ASSERT_FALSE( engine.IsThreaded() );
        // the thread has finished (the pool itself keeps listing a finished dedicated thread for a while, so ask it whether its threads are done)
        CORE::ThreadPoolPtr pool = taskManager.GetThreadPool( "AudioEngineTest" );
        ASSERT_FALSE( pool.IsNULL() );
        ASSERT_TRUE( pool->WaitForAllThreadsToFinish( 3000 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformAudioEngineTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioEngine TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.Engine" );

    PerformLifecycleTests();
    PerformPlaybackTests();
    PerformBusAndVolumeTests();
    PerformLimitTests();
    PerformDeviceFormatTests();
    PerformShutdownAndLossTests();
    PerformThreadedTests();
}

/*-------------------------------------------------------------------------*/
