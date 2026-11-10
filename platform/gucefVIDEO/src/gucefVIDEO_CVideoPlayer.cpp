/*
 *  gucefVIDEO: GUCEF module providing video decoding and playback
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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_VIDEO_CVIDEOGLOBAL_H
#include "gucefVIDEO_CVideoGlobal.h"
#define GUCEF_VIDEO_CVIDEOGLOBAL_H
#endif /* GUCEF_VIDEO_CVIDEOGLOBAL_H ? */

#ifndef GUCEF_VIDEO_CVIDEOCODECREGISTRY_H
#include "gucefVIDEO_CVideoCodecRegistry.h"
#define GUCEF_VIDEO_CVIDEOCODECREGISTRY_H
#endif /* GUCEF_VIDEO_CVIDEOCODECREGISTRY_H ? */

#include "gucefVIDEO_CVideoPlayer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VIDEO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CVideoPlayer::VideoEndedEvent = "GUCEF::VIDEO::CVideoPlayer::VideoEndedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CVideoPlayer::RegisterEvents( void )
{GUCEF_TRACE;

    VideoEndedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CVideoPlayer::CVideoPlayer( void )
    : CORE::CNotifier()
    , m_demuxer()
    , m_currentFrame()
    , m_hasFrame( false )
    , m_playing( false )
    , m_paused( false )
    , m_sourceEnded( false )
    , m_endReason( VIDEOEND_NONE )
    , m_skippedFrames( 0 )
    , m_nextFrameIndex( 0 )
    , m_engine( GUCEF_NULL )
    , m_voice()
    , m_stream()
    , m_audioSampleRate( 0 )
    , m_audioClock( false )
    , m_lastAudioPosition( 0 )
    , m_lastAudioMove( 0 )
    , m_timerStart( 0 )
    , m_timerBase( 0 )
    , m_pausedAt( 0 )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CVideoPlayer::~CVideoPlayer()
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

bool
CVideoPlayer::Open( const TVideoSourcePtr& source )
{GUCEF_TRACE;

    Close();
    if ( source.IsNULL() || !source->GetFormat().IsValid() )
        return false;

    m_demuxer = TVideoDemuxerPtr( GUCEF_NEW CVideoDemuxer( source ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVideoPlayer::Open( const CString& codecType, const CORE::IOAccessPtr& input, const CORE::CValueList& options )
{GUCEF_TRACE;

    Close();

    TVideoCodecPtr codec;
    if ( !CVideoGlobal::Instance()->GetVideoCodecRegistry().TryFindCodec( codecType, codec ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VideoPlayer: No video codec of type \"" << codecType << "\" is loaded" );
        return false;
    }

    TVideoSourcePtr source;
    if ( !codec->OpenSource( input, source, options ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VideoPlayer: The \"" << codecType << "\" codec could not open the video" );
        return false;
    }
    return Open( source );
}

/*-------------------------------------------------------------------------*/

void
CVideoPlayer::Close( void )
{GUCEF_TRACE;

    StopAudio();
    m_demuxer.Unlink();
    m_hasFrame = false;
    m_playing = false;
    m_paused = false;
    m_sourceEnded = false;
    m_endReason = VIDEOEND_NONE;
    m_skippedFrames = 0;
    m_nextFrameIndex = 0;
    m_engine = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CVideoPlayer::IsOpen( void ) const
{GUCEF_TRACE;

    return !m_demuxer.IsNULL();
}

/*-------------------------------------------------------------------------*/

const CVideoFormat&
CVideoPlayer::GetFormat( void ) const
{GUCEF_TRACE;

    static const CVideoFormat noFormat;
    return m_demuxer.IsNULL() ? noFormat : m_demuxer->GetFormat();
}

/*-------------------------------------------------------------------------*/

UInt32
CVideoPlayer::GetAudioTrackCount( void ) const
{GUCEF_TRACE;

    return m_demuxer.IsNULL() ? 0 : m_demuxer->GetAudioTrackCount();
}

/*-------------------------------------------------------------------------*/

bool
CVideoPlayer::Start( AUDIO::CAudioEngine* engine, AUDIO::CAudioBus* bus, UInt32 audioTrack )
{GUCEF_TRACE;

    // a video plays once per Open(): the decoders read forward only
    if ( m_demuxer.IsNULL() || m_playing || VIDEOEND_NONE != m_endReason || m_nextFrameIndex > 0 )
        return false;

    m_engine = engine;
    m_audioClock = false;
    if ( GUCEF_NULL != engine && engine->IsInitialized() && audioTrack < m_demuxer->GetAudioTrackCount() )
    {
        AUDIO::TAudioSourcePtr trackSource = CVideoDemuxer::CreateAudioTrackSource( m_demuxer, audioTrack );
        if ( !trackSource.IsNULL() )
        {
            m_audioSampleRate = trackSource->GetFormat().GetSampleRate();
            m_stream = CORE::CTSharedPtr< AUDIO::CAudioStream, MT::CMutex >( GUCEF_NEW AUDIO::CAudioStream( trackSource, AUDIO::CAudioStream::DefaultBufferDurationInMs, 1, engine->IsThreaded() ) );
            m_voice = engine->CreateVoice( m_stream.StaticCast< AUDIO::IAudioSource >(), bus );
            if ( !m_voice.IsNULL() && m_audioSampleRate > 0 )
            {
                engine->Start( m_voice );
                m_audioClock = true;
            }
            else
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "VideoPlayer: The sound of the video cannot be played, the video plays without it" );
                StopAudio();
            }
        }
    }

    m_timerStart = GetTimerInMicroSeconds();
    m_timerBase = 0;
    m_pausedAt = 0;
    m_lastAudioPosition = 0;
    m_lastAudioMove = m_timerStart;
    m_paused = false;
    m_playing = true;

    // the first frame is shown at once
    Update();
    if ( !m_hasFrame )
    {
        End( m_demuxer->HasFailed() ? VIDEOEND_FAILED : VIDEOEND_FINISHED );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CVideoPlayer::Pause( void )
{GUCEF_TRACE;

    if ( !m_playing || m_paused )
        return;

    m_pausedAt = GetClock();
    m_paused = true;
    if ( !m_voice.IsNULL() && GUCEF_NULL != m_engine )
        m_engine->Pause( m_voice );
}

/*-------------------------------------------------------------------------*/

void
CVideoPlayer::Resume( void )
{GUCEF_TRACE;

    if ( !m_playing || !m_paused )
        return;

    m_paused = false;
    m_timerBase = m_pausedAt;
    m_timerStart = GetTimerInMicroSeconds();
    m_lastAudioMove = m_timerStart;
    if ( !m_voice.IsNULL() && GUCEF_NULL != m_engine )
        m_engine->Resume( m_voice );
}

/*-------------------------------------------------------------------------*/

bool
CVideoPlayer::IsPaused( void ) const
{GUCEF_TRACE;

    return m_paused;
}

/*-------------------------------------------------------------------------*/

void
CVideoPlayer::Stop( void )
{GUCEF_TRACE;

    End( VIDEOEND_STOPPED );
}

/*-------------------------------------------------------------------------*/

void
CVideoPlayer::StopAudio( void )
{GUCEF_TRACE;

    if ( !m_voice.IsNULL() && GUCEF_NULL != m_engine )
        m_engine->Stop( m_voice );
    m_voice.Unlink();
    m_stream.Unlink();
    m_audioClock = false;
}

/*-------------------------------------------------------------------------*/

void
CVideoPlayer::End( TVideoEndReason reason )
{GUCEF_TRACE;

    if ( !m_playing )
        return;

    m_playing = false;
    m_paused = false;
    m_endReason = reason;
    StopAudio();

    TVideoEndedEventData data( reason );
    NotifyObservers( VideoEndedEvent, &data );
}

/*-------------------------------------------------------------------------*/

UInt64
CVideoPlayer::GetTimerInMicroSeconds( void ) const
{GUCEF_TRACE;

    const UInt64 ticks = MT::PrecisionTickCount();
    const UInt64 resolution = MT::PrecisionTimerResolution();
    if ( 0 == resolution )
        return 0;
    // split so a high resolution does not overflow
    return ( ticks / resolution ) * 1000000 + ( ticks % resolution ) * 1000000 / resolution;
}

/*-------------------------------------------------------------------------*/

UInt64
CVideoPlayer::GetClock( void ) const
{GUCEF_TRACE;

    if ( m_paused )
        return m_pausedAt;

    if ( m_audioClock && !m_voice.IsNULL() )
        return m_voice->GetPositionInFrames() * 1000000 / m_audioSampleRate;

    const UInt64 now = GetTimerInMicroSeconds();
    return m_timerBase + ( now > m_timerStart ? now - m_timerStart : 0 );
}

/*-------------------------------------------------------------------------*/

bool
CVideoPlayer::Update( void )
{GUCEF_TRACE;

    if ( !m_playing )
        return false;

    // without a thread for the sound it is read here
    if ( !m_stream.IsNULL() && GUCEF_NULL != m_engine && !m_engine->IsThreaded() )
        m_stream->Fill();

    if ( m_paused )
        return false;

    // the sound stopped before the pictures (shorter, failed, or its device does not play): the timer takes over where it was
    if ( m_audioClock && !m_voice.IsNULL() )
    {
        const UInt64 now = GetTimerInMicroSeconds();
        const UInt64 position = m_voice->GetPositionInFrames();
        bool stalled = false;
        if ( position != m_lastAudioPosition )
        {
            m_lastAudioPosition = position;
            m_lastAudioMove = now;
        }
        else
        if ( now > m_lastAudioMove && now - m_lastAudioMove > AudioStallTimeoutInMicroSeconds )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "VideoPlayer: The sound does not play on, the pictures follow the timer from now on" );
            stalled = true;
        }
        if ( stalled || AUDIO::AUDIOVOICESTATE_ENDED == m_voice->GetState() )
        {
            m_timerBase = position * 1000000 / m_audioSampleRate;
            m_timerStart = now;
            m_audioClock = false;
        }
    }

    // the frame that is due; with a known length never past the last one, which is shown for its full time
    const CVideoFormat& format = m_demuxer->GetFormat();
    const UInt64 clock = GetClock();
    const UInt64 frameCount = format.GetFrameCount();
    UInt64 due = format.GetFrameAtTime( clock );
    if ( GUCEF_VIDEO_FRAMECOUNT_UNKNOWN != frameCount && frameCount > 0 && due >= frameCount )
        due = frameCount - 1;

    // late frames are decoded into the current frame too, so the last one that decodes is the one shown
    bool changed = false;
    bool paletteChanged = false;
    while ( !m_sourceEnded && m_nextFrameIndex <= due )
    {
        if ( !m_demuxer->ReadFrame( m_currentFrame ) )
        {
            m_sourceEnded = true;
            break;
        }
        if ( changed )
            ++m_skippedFrames;
        paletteChanged = paletteChanged || m_currentFrame.IsPaletteChanged();
        m_hasFrame = true;
        changed = true;
        ++m_nextFrameIndex;
    }
    if ( changed )
        m_currentFrame.SetPaletteChanged( paletteChanged );

    if ( GUCEF_VIDEO_FRAMECOUNT_UNKNOWN != frameCount && m_nextFrameIndex >= frameCount && clock >= format.GetFrameTimeInMicroSeconds( frameCount ) )
        m_sourceEnded = true;

    // the end: the pictures are done and so is the sound (unless the sound no longer is the clock)
    if ( m_sourceEnded )
    {
        if ( m_demuxer->HasFailed() )
            End( VIDEOEND_FAILED );
        else
        if ( !m_audioClock || m_voice.IsNULL() || AUDIO::AUDIOVOICESTATE_ENDED == m_voice->GetState() )
            End( VIDEOEND_FINISHED );
    }
    return changed;
}

/*-------------------------------------------------------------------------*/

bool
CVideoPlayer::IsPlaying( void ) const
{GUCEF_TRACE;

    return m_playing;
}

/*-------------------------------------------------------------------------*/

bool
CVideoPlayer::IsEnded( void ) const
{GUCEF_TRACE;

    return VIDEOEND_NONE != m_endReason;
}

/*-------------------------------------------------------------------------*/

TVideoEndReason
CVideoPlayer::GetEndReason( void ) const
{GUCEF_TRACE;

    return m_endReason;
}

/*-------------------------------------------------------------------------*/

bool
CVideoPlayer::HasFrame( void ) const
{GUCEF_TRACE;

    return m_hasFrame;
}

/*-------------------------------------------------------------------------*/

const CVideoFrame&
CVideoPlayer::GetCurrentFrame( void ) const
{GUCEF_TRACE;

    return m_currentFrame;
}

/*-------------------------------------------------------------------------*/

UInt64
CVideoPlayer::GetSkippedFrameCount( void ) const
{GUCEF_TRACE;

    return m_skippedFrames;
}

/*-------------------------------------------------------------------------*/

UInt64
CVideoPlayer::GetPlaybackTimeInMicroSeconds( void ) const
{GUCEF_TRACE;

    if ( m_playing )
        return GetClock();
    return m_hasFrame ? m_currentFrame.GetTimeInMicroSeconds() : 0;
}

/*-------------------------------------------------------------------------*/

bool
CVideoPlayer::IsAudioClock( void ) const
{GUCEF_TRACE;

    return m_audioClock;
}

/*-------------------------------------------------------------------------*/

AUDIO::TAudioVoicePtr
CVideoPlayer::GetAudioVoice( void ) const
{GUCEF_TRACE;

    return m_voice;
}

/*-------------------------------------------------------------------------*/

const CString&
CVideoPlayer::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString typeName = "GUCEF::VIDEO::CVideoPlayer";
    return typeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VIDEO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
