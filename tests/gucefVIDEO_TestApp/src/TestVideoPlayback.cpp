/*
 *  gucefVIDEO_TestApp: GUCEF test application for the VIDEO module
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

#include <string.h>

#ifndef GUCEFVIDEO_H
#include "gucefVIDEO.h"
#define GUCEFVIDEO_H
#endif /* GUCEFVIDEO_H ? */

#ifndef GUCEF_AUDIO_CNULLAUDIODRIVER_H
#include "gucefAUDIO_CNullAudioDriver.h"
#define GUCEF_AUDIO_CNULLAUDIODRIVER_H
#endif /* GUCEF_AUDIO_CNULLAUDIODRIVER_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestVideoPlayback.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Audio frames per video frame of the scripted source: 10 frames per second, sound at 1000 Hz */
const UInt32 AudioPerFrame = 100;

/** The sample of the scripted sound at an audio frame */
UInt8
ScriptedSample( UInt64 audioFrame )
{
    return (UInt8) ( ( audioFrame * 7 ) & 0xFF );
}

/**
 *  A video made up by the test: 4x2 pixels, 10 frames per second, frame n is filled with the colour n and changes the
 *  palette when n is a multiple of 3; one 8 bit mono track at 1000 Hz, 100 frames of sound per picture, handed out
 *  in two pieces. It can fail at a chosen frame.
 */
class CScriptedSource : public VIDEO::IVideoSource
{
    public:

    CScriptedSource( UInt32 frameCount, UInt32 failAtFrame = GUCEF_UINT32MAX )
        : VIDEO::IVideoSource()
        , m_format( 4, 2, IMAGE::PSF_PALETTE_INDICES, 10, 1, frameCount )
        , m_frameCount( frameCount )
        , m_failAt( failAtFrame )
        , m_position( 0 )
        , m_ended( false )
        , m_failed( false )
        , decodeCount( 0 )
    {
    }

    virtual const VIDEO::CVideoFormat& GetFormat( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_format; }

    virtual UInt32 GetAudioTrackCount( void ) const GUCEF_VIRTUAL_OVERRIDE { return 1; }

    virtual bool GetAudioTrackFormat( UInt32 track, AUDIO::CAudioFormat& format ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( 0 != track )
            return false;
        format = AUDIO::CAudioFormat( 1000, 1, AUDIO::SAMPLETYPE_UINT8 );
        return true;
    }

    virtual bool DecodeNextFrame( VIDEO::CVideoFrame& frame, VIDEO::IVideoAudioSink* audioSink ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( m_ended )
            return false;
        if ( m_position >= m_frameCount || m_position == m_failAt )
        {
            m_ended = true;
            m_failed = m_position == m_failAt;
            return false;
        }
        ++decodeCount;
        frame.Prepare( m_format );
        memset( frame.GetPicture().GetDataPtr(), (int) ( m_position & 0xFF ), 8 );
        const bool paletteChange = 0 == m_position % 3;
        if ( paletteChange )
            frame.GetPalette()[ 0 ] = (UInt8) m_position;
        frame.SetPaletteChanged( paletteChange );
        frame.SetFrameIndex( m_position );
        frame.SetTimeInMicroSeconds( m_format.GetFrameTimeInMicroSeconds( m_position ) );

        if ( GUCEF_NULL != audioSink )
        {
            UInt8 sound[ AudioPerFrame ];
            for ( UInt32 i=0; i<AudioPerFrame; ++i )
                sound[ i ] = ScriptedSample( (UInt64) m_position * AudioPerFrame + i );
            audioSink->OnVideoAudio( 0, sound, 40 );
            audioSink->OnVideoAudio( 0, sound + 40, AudioPerFrame - 40 );
        }
        ++m_position;
        return true;
    }

    virtual bool IsEndOfStream( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_ended; }

    virtual bool HasFailed( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_failed; }

    virtual UInt64 GetPositionInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_position; }

    private:

    VIDEO::CVideoFormat m_format;
    UInt32 m_frameCount;
    UInt32 m_failAt;
    UInt32 m_position;
    bool m_ended;
    bool m_failed;

    public:

    UInt32 decodeCount;
};

/*-------------------------------------------------------------------------*/

/** First pixel of a frame: the frame number for the scripted source */
UInt32
Colour( const VIDEO::CVideoFrame& frame )
{
    return static_cast< const UInt8* >( frame.GetPicture().GetDataPtr() )[ 0 ];
}

/*-------------------------------------------------------------------------*/

/** Reads sound from a source until it has 'frames' or nothing comes; true when every sample is the scripted one from 'first' on */
bool
ReadScriptedSound( AUDIO::IAudioSource& source, UInt64 first, UInt32 frames, UInt32& got )
{
    UInt8 buffer[ 4096 ];
    got = 0;
    bool ok = true;
    while ( got < frames )
    {
        const UInt32 wanted = frames - got < sizeof( buffer ) ? frames - got : (UInt32) sizeof( buffer );
        const UInt32 read = source.ReadFrames( buffer, wanted );
        for ( UInt32 i=0; i<read; ++i )
            ok = ok && buffer[ i ] == ScriptedSample( first + got + i );
        got += read;
        if ( 0 == read )
            break;
    }
    return ok;
}

/*-------------------------------------------------------------------------*/

/** A player whose timer is set by the test */
class CTestPlayer : public VIDEO::CVideoPlayer
{
    public:

    CTestPlayer( void ) : VIDEO::CVideoPlayer(), now( 1000000 ) {}

    UInt64 now;

    protected:

    virtual UInt64 GetTimerInMicroSeconds( void ) const GUCEF_VIRTUAL_OVERRIDE { return now; }
};

/*-------------------------------------------------------------------------*/

class CPlayerObserver : public CORE::CObserver
{
    public:

    CPlayerObserver( void ) : CORE::CObserver(), endings( 0 ), lastReason( VIDEO::VIDEOEND_NONE ) {}

    UInt32 endings;
    VIDEO::TVideoEndReason lastReason;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( VIDEO::CVideoPlayer::VideoEndedEvent == eventid )
        {
            ++endings;
            lastReason = static_cast< VIDEO::CVideoPlayer::TVideoEndedEventData* >( eventdata )->GetData();
        }
    }
};

/*-------------------------------------------------------------------------*/

/** An engine on the null driver, driven by hand */
AUDIO::CAudioEngineParams
NullParams( void )
{
    AUDIO::CAudioEngineParams params;
    params.SetDriverName( "null" );
    params.SetThreadingMode( AUDIO::AUDIOTHREADING_COOPERATIVE );
    params.GetDeviceParams().SetBufferSizeInFrames( 256 );
    return params;
}

AUDIO::CNullAudioDevice*
Null( AUDIO::CAudioEngine& engine )
{
    AUDIO::TAudioDevicePtr device = engine.GetDevice();
    return ( device.IsNULL() ? GUCEF_NULL : device->AsNullDevice() );
}

/*-------------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformDemuxerTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Demuxer: Pictures come in order to the end, and sound nobody reads is not kept" )
    try
    {
        CScriptedSource* scripted = GUCEF_NEW CScriptedSource( 6 );
        VIDEO::TVideoDemuxerPtr demuxer( GUCEF_NEW VIDEO::CVideoDemuxer( VIDEO::TVideoSourcePtr( scripted ) ) );
        ASSERT_TRUE( 1 == demuxer->GetAudioTrackCount() && 6 == demuxer->GetFormat().GetFrameCount() );

        VIDEO::CVideoFrame frame;
        for ( UInt32 n=0; n<3; ++n )
        {
            ASSERT_TRUE( demuxer->ReadFrame( frame ) );
            ASSERT_TRUE( n == frame.GetFrameIndex() && n == Colour( frame ) && 100000 * n == frame.GetTimeInMicroSeconds() );
        }

        // the sound of the frames before there was a reader is gone: the track starts at frame 3
        AUDIO::TAudioSourcePtr sound = VIDEO::CVideoDemuxer::CreateAudioTrackSource( demuxer, 0 );
        ASSERT_FALSE( sound.IsNULL() );
        ASSERT_TRUE( 1000 == sound->GetFormat().GetSampleRate() && AUDIO::SAMPLETYPE_UINT8 == sound->GetFormat().GetSampleType() );
        UInt32 got = 0;
        ASSERT_TRUE( ReadScriptedSound( *sound, 3 * AudioPerFrame, 150, got ) && 150 == got );

        // the rest of the pictures: two were queued for the sound, one is decoded now
        ASSERT_TRUE( 2 == demuxer->GetQueuedFrameCount() );
        for ( UInt32 n=3; n<6; ++n )
        {
            ASSERT_TRUE( demuxer->ReadFrame( frame ) );
            ASSERT_TRUE( n == frame.GetFrameIndex() && n == Colour( frame ) );
        }
        ASSERT_FALSE( demuxer->ReadFrame( frame ) );
        ASSERT_TRUE( demuxer->IsEndOfStream() && !demuxer->HasFailed() );
        ASSERT_TRUE( 6 == scripted->decodeCount );                                    // every frame decoded once

        // a track the video does not have
        ASSERT_TRUE( VIDEO::CVideoDemuxer::CreateAudioTrackSource( demuxer, 1 ).IsNULL() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Demuxer: Picture and sound consumers share one decoder, the sound is complete and in order" )
    try
    {
        CScriptedSource* scripted = GUCEF_NEW CScriptedSource( 10 );
        VIDEO::TVideoDemuxerPtr demuxer( GUCEF_NEW VIDEO::CVideoDemuxer( VIDEO::TVideoSourcePtr( scripted ) ) );
        AUDIO::TAudioSourcePtr sound = VIDEO::CVideoDemuxer::CreateAudioTrackSource( demuxer, 0 );

        // the sound runs ahead: 2.5 frames worth makes three frames decoded, all three pictures wait in the queue
        UInt32 got = 0;
        ASSERT_TRUE( ReadScriptedSound( *sound, 0, 250, got ) && 250 == got );
        ASSERT_TRUE( 3 == scripted->decodeCount && 3 == demuxer->GetQueuedFrameCount() );

        // the pictures catch up and pass the sound
        VIDEO::CVideoFrame frame;
        for ( UInt32 n=0; n<5; ++n )
        {
            ASSERT_TRUE( demuxer->ReadFrame( frame ) && n == frame.GetFrameIndex() );
        }
        ASSERT_TRUE( 5 == scripted->decodeCount && 0 == demuxer->GetQueuedFrameCount() );

        // the sound of frames decoded for the pictures was kept: it goes on without a gap to the end
        ASSERT_TRUE( ReadScriptedSound( *sound, 250, 10000, got ) && 750 == got );
        ASSERT_TRUE( sound->IsEndOfStream() );
        ASSERT_TRUE( 1000 == sound->GetPositionInFrames() );

        // the pictures the sound made the decoder produce are still there for the picture side
        ASSERT_TRUE( 5 == demuxer->GetQueuedFrameCount() );
        for ( UInt32 n=5; n<10; ++n )
        {
            ASSERT_TRUE( demuxer->ReadFrame( frame ) && n == frame.GetFrameIndex() && n == Colour( frame ) );
        }
        ASSERT_FALSE( demuxer->ReadFrame( frame ) );
        ASSERT_TRUE( demuxer->IsEndOfStream() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Demuxer: The queue of pictures is bounded, the sound waits for the pictures to be taken" )
    try
    {
        CScriptedSource* scripted = GUCEF_NEW CScriptedSource( 10 );
        VIDEO::TVideoDemuxerPtr demuxer( GUCEF_NEW VIDEO::CVideoDemuxer( VIDEO::TVideoSourcePtr( scripted ) ) );
        demuxer->SetMaxQueuedFrames( 2 );
        ASSERT_TRUE( 2 == demuxer->GetMaxQueuedFrames() );
        AUDIO::TAudioSourcePtr sound = VIDEO::CVideoDemuxer::CreateAudioTrackSource( demuxer, 0 );

        UInt32 got = 0;
        ASSERT_TRUE( ReadScriptedSound( *sound, 0, 1000, got ) && 200 == got );        // two frames, then the queue is full
        ASSERT_FALSE( sound->IsEndOfStream() );                                        // not the end: nothing more right now
        ASSERT_TRUE( 2 == scripted->decodeCount && 2 == demuxer->GetQueuedFrameCount() );

        VIDEO::CVideoFrame frame;
        ASSERT_TRUE( demuxer->ReadFrame( frame ) && 0 == frame.GetFrameIndex() );
        ASSERT_TRUE( ReadScriptedSound( *sound, 200, 1000, got ) && 100 == got );       // room for one more frame
        ASSERT_TRUE( 3 == scripted->decodeCount );

        // nobody takes the pictures: the sound plays to the end and nothing is queued
        demuxer->SetPictureConsumer( false );
        ASSERT_TRUE( 0 == demuxer->GetQueuedFrameCount() );
        ASSERT_TRUE( ReadScriptedSound( *sound, 300, 1000, got ) && 700 == got );
        ASSERT_TRUE( sound->IsEndOfStream() && 0 == demuxer->GetQueuedFrameCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Demuxer: Skipped frames tell their palette changes, a failing source is reported" )
    try
    {
        VIDEO::TVideoDemuxerPtr demuxer( GUCEF_NEW VIDEO::CVideoDemuxer( VIDEO::TVideoSourcePtr( GUCEF_NEW CScriptedSource( 10, 4 ) ) ) );
        bool paletteChanged = false;
        ASSERT_TRUE( demuxer->SkipFrame( &paletteChanged ) && paletteChanged );        // frame 0
        ASSERT_TRUE( demuxer->SkipFrame( &paletteChanged ) && !paletteChanged );       // frame 1
        ASSERT_TRUE( demuxer->SkipFrame() );                                           // frame 2
        ASSERT_TRUE( demuxer->SkipFrame( &paletteChanged ) && paletteChanged );        // frame 3
        ASSERT_FALSE( demuxer->HasFailed() );
        VIDEO::CVideoFrame frame;
        ASSERT_FALSE( demuxer->ReadFrame( frame ) );                                   // frame 4 fails
        ASSERT_TRUE( demuxer->HasFailed() && demuxer->IsEndOfStream() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformPlayerTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Player: Without sound the timer is the clock: due frames are shown, late ones skipped" )
    try
    {
        CTestPlayer player;
        CPlayerObserver observer;
        observer.SubscribeTo( &player, VIDEO::CVideoPlayer::VideoEndedEvent );

        ASSERT_FALSE( player.Start() );                                                // nothing open
        ASSERT_TRUE( player.Open( VIDEO::TVideoSourcePtr( GUCEF_NEW CScriptedSource( 10 ) ) ) );
        ASSERT_TRUE( player.IsOpen() && 1 == player.GetAudioTrackCount() && 4 == player.GetFormat().GetWidth() );

        ASSERT_TRUE( player.Start() );
        ASSERT_FALSE( player.IsAudioClock() );
        ASSERT_TRUE( player.IsPlaying() && player.HasFrame() && 0 == player.GetCurrentFrame().GetFrameIndex() );
        ASSERT_FALSE( player.Start() );                                                // once per Open()
        ASSERT_FALSE( player.Update() );                                               // no time passed

        player.now += 99999;
        ASSERT_FALSE( player.Update() );
        player.now += 1;
        ASSERT_TRUE( player.Update() && 1 == player.GetCurrentFrame().GetFrameIndex() && 1 == Colour( player.GetCurrentFrame() ) );

        // a slow application: frame 2 is late and is skipped, frame 3 is shown
        player.now += 250000;
        ASSERT_TRUE( player.Update() && 3 == player.GetCurrentFrame().GetFrameIndex() );
        ASSERT_TRUE( 1 == player.GetSkippedFrameCount() );
        ASSERT_TRUE( 350000 == player.GetPlaybackTimeInMicroSeconds() );

        // frame 6 changed the palette but is skipped: frame 7, which is shown, says so
        player.now += 200000;
        ASSERT_TRUE( player.Update() && 5 == player.GetCurrentFrame().GetFrameIndex() && !player.GetCurrentFrame().IsPaletteChanged() );
        player.now += 200000;
        ASSERT_TRUE( player.Update() && 7 == player.GetCurrentFrame().GetFrameIndex() && player.GetCurrentFrame().IsPaletteChanged() );

        // pausing stops the clock
        player.Pause();
        ASSERT_TRUE( player.IsPaused() );
        player.now += 5000000;
        ASSERT_FALSE( player.Update() );
        ASSERT_TRUE( 7 == player.GetCurrentFrame().GetFrameIndex() && 750000 == player.GetPlaybackTimeInMicroSeconds() );
        player.Resume();
        ASSERT_FALSE( player.IsPaused() );
        player.now += 50000;
        ASSERT_TRUE( player.Update() && 8 == player.GetCurrentFrame().GetFrameIndex() );

        // the end: the last frame stays, the event comes once
        ASSERT_TRUE( 0 == observer.endings );
        player.now += 10000000;
        player.Update();
        ASSERT_TRUE( 9 == player.GetCurrentFrame().GetFrameIndex() );
        ASSERT_TRUE( player.IsEnded() && !player.IsPlaying() && VIDEO::VIDEOEND_FINISHED == player.GetEndReason() );
        ASSERT_TRUE( 1 == observer.endings && VIDEO::VIDEOEND_FINISHED == observer.lastReason );
        player.Update();
        player.Stop();
        ASSERT_TRUE( 1 == observer.endings );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Player: Stop ends with its own reason, a failing video ends as failed" )
    try
    {
        CTestPlayer player;
        CPlayerObserver observer;
        observer.SubscribeTo( &player, VIDEO::CVideoPlayer::VideoEndedEvent );
        ASSERT_TRUE( player.Open( VIDEO::TVideoSourcePtr( GUCEF_NEW CScriptedSource( 10 ) ) ) );
        ASSERT_TRUE( player.Start() );
        player.Stop();
        ASSERT_TRUE( player.IsEnded() && VIDEO::VIDEOEND_STOPPED == player.GetEndReason() );
        ASSERT_TRUE( 1 == observer.endings && VIDEO::VIDEOEND_STOPPED == observer.lastReason );
        player.now += 1000000;
        ASSERT_FALSE( player.Update() );

        ASSERT_TRUE( player.Open( VIDEO::TVideoSourcePtr( GUCEF_NEW CScriptedSource( 10, 3 ) ) ) );
        ASSERT_FALSE( player.IsEnded() );                                              // opening starts over
        ASSERT_TRUE( player.Start() );
        player.now += 1000000;
        player.Update();
        ASSERT_TRUE( 2 == player.GetCurrentFrame().GetFrameIndex() );
        ASSERT_TRUE( VIDEO::VIDEOEND_FAILED == player.GetEndReason() && VIDEO::VIDEOEND_FAILED == observer.lastReason && 2 == observer.endings );

        // a video whose first frame does not decode does not start
        ASSERT_TRUE( player.Open( VIDEO::TVideoSourcePtr( GUCEF_NEW CScriptedSource( 10, 0 ) ) ) );
        ASSERT_FALSE( player.Start() );
        ASSERT_TRUE( VIDEO::VIDEOEND_FAILED == player.GetEndReason() );

        player.Close();
        ASSERT_FALSE( player.IsOpen() || player.HasFrame() || player.IsEnded() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Player: With sound the position of the sound is the clock, a sound that stops playing hands over to the timer" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams() ) );
        const UInt32 deviceRate = engine.GetDeviceFormat().GetSampleRate();
        ASSERT_TRUE( deviceRate > 0 );

        CTestPlayer player;
        CPlayerObserver observer;
        observer.SubscribeTo( &player, VIDEO::CVideoPlayer::VideoEndedEvent );
        ASSERT_TRUE( player.Open( VIDEO::TVideoSourcePtr( GUCEF_NEW CScriptedSource( 20 ) ) ) );
        ASSERT_TRUE( player.Start( &engine ) );
        ASSERT_TRUE( player.IsAudioClock() && !player.GetAudioVoice().IsNULL() );
        ASSERT_TRUE( 0 == player.GetCurrentFrame().GetFrameIndex() );

        // time passes but no sound is played: the picture waits for the sound
        player.now += 500000;
        player.Update();
        ASSERT_TRUE( 0 == player.GetCurrentFrame().GetFrameIndex() );

        // 0.35 s of sound plays: frame 3 is due
        Null( engine )->Render( (UInt64) deviceRate * 35 / 100 );
        engine.Update();
        player.Update();
        const UInt64 shown = player.GetCurrentFrame().GetFrameIndex();
        ASSERT_TRUE( shown >= 2 && shown <= 3 );                                       // the resampler may hold back a few frames of sound
        ASSERT_TRUE( player.IsAudioClock() );

        // pausing pauses the sound as well
        player.Pause();
        Null( engine )->Render( deviceRate / 2 );
        engine.Update();
        ASSERT_FALSE( player.Update() );
        ASSERT_TRUE( shown == player.GetCurrentFrame().GetFrameIndex() );
        player.Resume();
        Null( engine )->Render( deviceRate / 10 );
        engine.Update();
        player.Update();
        ASSERT_TRUE( player.GetCurrentFrame().GetFrameIndex() > shown );

        // the device stops: after a while without sound the timer takes over and the video plays to its end
        const UInt64 before = player.GetCurrentFrame().GetFrameIndex();
        player.now += VIDEO::CVideoPlayer::AudioStallTimeoutInMicroSeconds + 1;
        player.Update();
        ASSERT_FALSE( player.IsAudioClock() );
        ASSERT_TRUE( before == player.GetCurrentFrame().GetFrameIndex() );             // the clock continues from where the sound was
        player.now += 3000000;
        player.Update();
        ASSERT_TRUE( 19 == player.GetCurrentFrame().GetFrameIndex() );
        ASSERT_TRUE( VIDEO::VIDEOEND_FINISHED == player.GetEndReason() && 1 == observer.endings );
        ASSERT_TRUE( player.GetAudioVoice().IsNULL() );                                // the sound is let go at the end
        engine.Shutdown();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Player: With sound the video ends when the sound has played" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams() ) );
        const UInt32 deviceRate = engine.GetDeviceFormat().GetSampleRate();

        CTestPlayer player;
        ASSERT_TRUE( player.Open( VIDEO::TVideoSourcePtr( GUCEF_NEW CScriptedSource( 5 ) ) ) );
        ASSERT_TRUE( player.Start( &engine ) );
        for ( UInt32 i=0; i<20 && player.IsPlaying(); ++i )
        {
            Null( engine )->Render( deviceRate / 10 );
            engine.Update();
            player.now += 100000;
            player.Update();
        }
        ASSERT_TRUE( 4 == player.GetCurrentFrame().GetFrameIndex() );
        ASSERT_TRUE( VIDEO::VIDEOEND_FINISHED == player.GetEndReason() );
        engine.Shutdown();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformVideoPlaybackTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING video playback TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefVIDEO.Playback" );

    PerformDemuxerTests();
    PerformPlayerTests();
}

/*-------------------------------------------------------------------------*/
