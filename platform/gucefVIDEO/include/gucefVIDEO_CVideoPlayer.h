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

#ifndef GUCEF_VIDEO_CVIDEOPLAYER_H
#define GUCEF_VIDEO_CVIDEOPLAYER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CNOTIFIER_H
#include "CNotifier.h"
#define GUCEF_CORE_CNOTIFIER_H
#endif /* GUCEF_CORE_CNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_AUDIO_CAUDIOENGINE_H
#include "gucefAUDIO_CAudioEngine.h"
#define GUCEF_AUDIO_CAUDIOENGINE_H
#endif /* GUCEF_AUDIO_CAUDIOENGINE_H ? */

#ifndef GUCEF_AUDIO_CAUDIOSTREAM_H
#include "gucefAUDIO_CAudioStream.h"
#define GUCEF_AUDIO_CAUDIOSTREAM_H
#endif /* GUCEF_AUDIO_CAUDIOSTREAM_H ? */

#ifndef GUCEF_VIDEO_CVIDEODEMUXER_H
#include "gucefVIDEO_CVideoDemuxer.h"
#define GUCEF_VIDEO_CVIDEODEMUXER_H
#endif /* GUCEF_VIDEO_CVIDEODEMUXER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VIDEO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Plays a video: decodes the picture that is due, plays the sound of one track through an audio engine and keeps
 *  the two together. With sound, the position of the sound is the clock the pictures follow, so they stay in sync
 *  whatever the audio device does; without sound (no engine, no track, or the sound could not be played) the
 *  high resolution timer is the clock.
 *
 *  The application calls Update() regularly (every frame it draws) and shows GetCurrentFrame(). Frames that are
 *  late are decoded (the next ones need them) but only the last of them is shown; its palette change flag also
 *  covers the palette changes of the frames that were passed over. A video of known length ends after its last
 *  frame has been shown for its full time and (with sound) the sound has played. The player has no thread of its own: the pictures are
 *  decoded in Update(), the sound by the read ahead task of the audio stream.
 *
 *  Use a player from one thread.
 */
class GUCEF_VIDEO_PUBLIC_CPP CVideoPlayer : public CORE::CNotifier
{
    public:

    /** Raised from Update() or Stop() when the video has ended. Data: a TVideoEndedEventData with the reason */
    static const CORE::CEvent VideoEndedEvent;

    typedef CORE::CTCloneableObj< TVideoEndReason > TVideoEndedEventData;

    /** No sound for this video */
    static const UInt32 NoAudioTrack = GUCEF_UINT32MAX;

    /** When the sound does not move on for this long while playing (a device that stopped), the timer becomes the clock */
    static const UInt64 AudioStallTimeoutInMicroSeconds = 1000000;

    static void RegisterEvents( void );

    CVideoPlayer( void );

    virtual ~CVideoPlayer() GUCEF_VIRTUAL_OVERRIDE;

    /** Takes a source to play. Closes what was open */
    bool Open( const TVideoSourcePtr& source );

    /** Opens the data with the codec registered for the type ("smk"). The source keeps the input alive */
    bool Open( const CString& codecType, const CORE::IOAccessPtr& input, const CORE::CValueList& options = CORE::CValueList() );

    /** Stops playing (without the ended event) and lets go of the video */
    void Close( void );

    bool IsOpen( void ) const;

    const CVideoFormat& GetFormat( void ) const;

    UInt32 GetAudioTrackCount( void ) const;

    /**
     *  Starts playing from the first frame, which is shown at once. With an engine the sound of 'audioTrack' plays
     *  on 'bus' (NULL: the master bus) and is the clock. Without an engine, or with NoAudioTrack, or for a video
     *  without sound, the timer is the clock. False when nothing is open or the first frame cannot be decoded.
     */
    bool Start( AUDIO::CAudioEngine* engine = GUCEF_NULL, AUDIO::CAudioBus* bus = GUCEF_NULL, UInt32 audioTrack = 0 );

    void Pause( void );
    void Resume( void );
    bool IsPaused( void ) const;

    /** Ends playback now (skipping the rest), with the ended event and VIDEOEND_STOPPED as the reason */
    void Stop( void );

    /**
     *  Brings the picture up to the clock, and ends the video when everything has played.
     *  Returns true when GetCurrentFrame() changed.
     */
    bool Update( void );

    bool IsPlaying( void ) const;
    bool IsEnded( void ) const;
    TVideoEndReason GetEndReason( void ) const;

    /** True once a frame was decoded */
    bool HasFrame( void ) const;

    /** The frame to show now */
    const CVideoFrame& GetCurrentFrame( void ) const;

    /** Frames decoded but not shown because they were late (passed over within one Update()) */
    UInt64 GetSkippedFrameCount( void ) const;

    /** Where playback is, in microseconds from the start */
    UInt64 GetPlaybackTimeInMicroSeconds( void ) const;

    /** True when the sound is the clock */
    bool IsAudioClock( void ) const;

    /** The voice the sound plays on, NULL without sound */
    AUDIO::TAudioVoicePtr GetAudioVoice( void ) const;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    /** The timer clock in microseconds. Tests make time pass by overriding it */
    virtual UInt64 GetTimerInMicroSeconds( void ) const;

    private:

    void End( TVideoEndReason reason );

    void StopAudio( void );

    /** The clock, in microseconds of video time */
    UInt64 GetClock( void ) const;

    CVideoPlayer( const CVideoPlayer& src );              /**< not implemented */
    CVideoPlayer& operator=( const CVideoPlayer& src );   /**< not implemented */

    private:

    TVideoDemuxerPtr m_demuxer;
    CVideoFrame m_currentFrame;
    bool m_hasFrame;
    bool m_playing;
    bool m_paused;
    bool m_sourceEnded;
    TVideoEndReason m_endReason;
    UInt64 m_skippedFrames;
    UInt64 m_nextFrameIndex;        /**< the number of the frame the demuxer gives next */

    AUDIO::CAudioEngine* m_engine;
    AUDIO::TAudioVoicePtr m_voice;
    CORE::CTSharedPtr< AUDIO::CAudioStream, MT::CMutex > m_stream;
    UInt32 m_audioSampleRate;
    bool m_audioClock;
    UInt64 m_lastAudioPosition;     /**< sound position at the last Update() */
    UInt64 m_lastAudioMove;         /**< timer value when the sound position last changed */

    UInt64 m_timerStart;            /**< timer value at video time m_timerBase */
    UInt64 m_timerBase;             /**< video time when the timer clock was (re)started */
    UInt64 m_pausedAt;              /**< video time when paused */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VIDEO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VIDEO_CVIDEOPLAYER_H ? */
