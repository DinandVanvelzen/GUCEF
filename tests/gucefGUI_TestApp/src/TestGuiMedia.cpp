/*
 *  gucefGUI_TestApp: Application for testing gucefGUI
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

#include <string.h>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_AUDIO_CTONESEQUENCESOURCE_H
#include "gucefAUDIO_CToneSequenceSource.h"
#define GUCEF_AUDIO_CTONESEQUENCESOURCE_H
#endif /* GUCEF_AUDIO_CTONESEQUENCESOURCE_H ? */

#ifndef GUCEF_AUDIO_CNULLAUDIODRIVER_H
#include "gucefAUDIO_CNullAudioDriver.h"
#define GUCEF_AUDIO_CNULLAUDIODRIVER_H
#endif /* GUCEF_AUDIO_CNULLAUDIODRIVER_H ? */

#ifndef GUCEF_AUDIO_CAUDIOENGINEPARAMS_H
#include "gucefAUDIO_CAudioEngineParams.h"
#define GUCEF_AUDIO_CAUDIOENGINEPARAMS_H
#endif /* GUCEF_AUDIO_CAUDIOENGINEPARAMS_H ? */

#ifndef GUCEF_VIDEO_IVIDEOSOURCE_H
#include "gucefVIDEO_IVideoSource.h"
#define GUCEF_VIDEO_IVIDEOSOURCE_H
#endif /* GUCEF_VIDEO_IVIDEOSOURCE_H ? */

#include "gucefGUI_TestCommon.h"

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

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

/** A movie made up by the test: 4x2 pixels in palette colours, 20 frames per second, no sound; frame n uses colour index n */
class CScriptedVideo : public VIDEO::IVideoSource
{
    public:

    CScriptedVideo( UInt32 frameCount )
        : VIDEO::IVideoSource()
        , m_format( 4, 2, IMAGE::PSF_PALETTE_INDICES, 20, 1, frameCount )
        , m_frameCount( frameCount )
        , m_position( 0 )
        , m_ended( false )
    {
    }

    virtual const VIDEO::CVideoFormat& GetFormat( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_format; }
    virtual UInt32 GetAudioTrackCount( void ) const GUCEF_VIRTUAL_OVERRIDE { return 0; }
    virtual bool GetAudioTrackFormat( UInt32 track, AUDIO::CAudioFormat& format ) const GUCEF_VIRTUAL_OVERRIDE { return false; }

    virtual bool DecodeNextFrame( VIDEO::CVideoFrame& frame, VIDEO::IVideoAudioSink* audioSink ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( m_ended || m_position >= m_frameCount )
        {
            m_ended = true;
            return false;
        }
        frame.Prepare( m_format );
        memset( frame.GetPicture().GetDataPtr(), (int) m_position, 8 );
        // palette entry n is (n*10, 0, 0)
        for ( UInt32 c=0; c<256; ++c )
            frame.GetPalette()[ c*3 ] = (UInt8) ( c * 10 );
        frame.SetPaletteChanged( 0 == m_position );
        frame.SetFrameIndex( m_position );
        frame.SetTimeInMicroSeconds( m_format.GetFrameTimeInMicroSeconds( m_position ) );
        ++m_position;
        return true;
    }

    virtual bool IsEndOfStream( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_ended; }
    virtual bool HasFailed( void ) const GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual UInt64 GetPositionInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_position; }

    private:

    VIDEO::CVideoFormat m_format;
    UInt32 m_frameCount;
    UInt32 m_position;
    bool m_ended;
};

class CEventCount : public CORE::CObserver
{
    public:

    CORE::CEvent watched;
    UInt32 count;

    CEventCount( CORE::CNotifier* notifier, const CORE::CEvent& eventId ) : CORE::CObserver(), watched( eventId ), count( 0 ) { SubscribeTo( notifier, eventId ); }

    virtual ~CEventCount() { UnsubscribeAllFromObserver(); }

    // a notifier also tells its observers about the subscription itself: only the watched event counts
    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( eventId == watched )
            ++count;
    }
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiMediaTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.Media" );

    GUCEF_TESTFW_TESTCASE( "Media: A video view shows the frames of a movie as its image and reports the end" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CWidget* screen = context->CreateWidget( "Panel", "Screen", GUCEF_NULL );
        screen->SetProperty( "Size", "1,0 1,0" );
        context->ShowRoot( screen, false );
        GUI::CVideoView* view = static_cast< GUI::CVideoView* >( context->CreateWidget( "VideoView", "Movie", screen ) );
        CEventCount ended( view, GUI::CVideoView::VideoEndedEvent );
        CEventCount skipped( view, GUI::CVideoView::VideoSkippedEvent );

        ASSERT_TRUE( view->Play( VIDEO::TVideoSourcePtr( GUCEF_NEW CScriptedVideo( 6 ) ) ) );
        ASSERT_TRUE( view->IsPlaying() && view->GetBoolProperty( "Playing" ) );
        context->FlushChanges();
        // the first frame is there at once: 4x2, colour index 0 is black
        IMAGE::TPixelMapPtr pixels;
        UInt32 revision = 0;
        ASSERT_TRUE( context->ResolveImage( view->GetImageName(), pixels, revision ) );
        ASSERT_TRUE( 4 == pixels->GetWidthInPixels() && 2 == pixels->GetHeightInPixels() && 4 == view->GetRect().width );

        // 6 frames at 20 per second: done well within two seconds of real time
        UInt32 lastColour = 0;
        bool sawLaterFrame = false;
        for ( UInt32 i=0; i<200 && 0 == ended.count; ++i )
        {
            MT::PrecisionDelay( 10 );
            context->RenderFrame( 10 );
            context->ResolveImage( view->GetImageName(), pixels, revision );
            lastColour = static_cast< const UInt8* >( pixels->GetDataPtr() )[ 0 ];
            if ( lastColour > 0 )
                sawLaterFrame = true;
        }
        ASSERT_TRUE( 1 == ended.count && 0 == skipped.count && sawLaterFrame );
        ASSERT_TRUE( 50 == lastColour );
        ASSERT_FALSE( view->IsPlaying() );

        // a click ends a playing movie early
        ASSERT_TRUE( view->Play( VIDEO::TVideoSourcePtr( GUCEF_NEW CScriptedVideo( 100 ) ) ) );
        context->FlushChanges();
        context->InjectMouseMove( 1, 1 );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        ASSERT_TRUE( 1 == skipped.count && !view->IsPlaying() );
        // with SkipOnInput off a key does not
        view->SetBoolProperty( "SkipOnInput", false );
        ASSERT_TRUE( view->Play( VIDEO::TVideoSourcePtr( GUCEF_NEW CScriptedVideo( 100 ) ) ) );
        context->InjectKey( INPUT::KEYCODE_SPACE, true, 0 );
        ASSERT_TRUE( 1 == skipped.count && view->IsPlaying() );
        view->Stop();
        context->DestroyWidget( screen );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Media: An audio player plays through the audio engine, pauses on activation and reports the end" )
    try
    {
        AUDIO::CAudioEngine engine;
        AUDIO::CAudioEngineParams params;
        params.SetDriverName( "null" );
        params.SetThreadingMode( AUDIO::AUDIOTHREADING_COOPERATIVE );
        ASSERT_TRUE( engine.Initialize( params ) );

        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CWidget* screen = context->CreateWidget( "Panel", "Screen", GUCEF_NULL );
        screen->SetProperty( "Size", "1,0 1,0" );
        context->ShowRoot( screen, false );
        GUI::CAudioPlayer* player = static_cast< GUI::CAudioPlayer* >( context->CreateWidget( "AudioPlayer", "Music", screen ) );
        player->SetProperty( "Size", "0,100 0,20" );
        CEventCount ended( player, GUI::CAudioPlayer::AudioEndedEvent );

        // without an engine nothing plays
        AUDIO::CToneSequenceSource::TSegmentVector segments;
        segments.push_back( AUDIO::CToneSequenceSource::SSegment( AUDIO::CToneSequenceSource::WAVEFORM_SINE, 440.0, 300 ) );
        ASSERT_FALSE( player->Play( AUDIO::TAudioSourcePtr( GUCEF_NEW AUDIO::CToneSequenceSource( 22050, 1, segments ) ) ) );

        player->SetAudioOutput( &engine, GUCEF_NULL );
        ASSERT_TRUE( player->Play( AUDIO::TAudioSourcePtr( GUCEF_NEW AUDIO::CToneSequenceSource( 22050, 1, segments ) ) ) );
        ASSERT_TRUE( player->IsPlaying() && !player->IsPaused() );

        // activation (a click) pauses and resumes
        context->FlushChanges();
        context->InjectMouseMove( 5, 5 );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        ASSERT_TRUE( player->IsPaused() );
        context->RenderFrame( 1000 );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        ASSERT_FALSE( player->IsPaused() );

        // the null device plays only what it is asked to: 10 ms per step, 300 ms of sound
        AUDIO::CNullAudioDevice* device = ( engine.GetDevice().IsNULL() ? GUCEF_NULL : engine.GetDevice()->AsNullDevice() );
        ASSERT_TRUE( GUCEF_NULL != device );
        Int32 position = 0;
        for ( UInt32 i=0; i<400 && 0 == ended.count && GUCEF_NULL != device; ++i )
        {
            device->Render( 480 );
            engine.Update();
            context->RenderFrame( 10 );
            if ( player->GetIntProperty( "PositionInMs" ) > position )
                position = player->GetIntProperty( "PositionInMs" );
        }
        ASSERT_TRUE( 1 == ended.count && !player->IsPlaying() );
        ASSERT_TRUE( position > 0 );

        context->DestroyWidget( screen );
        engine.Shutdown();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
