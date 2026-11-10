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

#ifndef GUCEF_VIDEO_CVIDEODEMUXER_H
#define GUCEF_VIDEO_CVIDEODEMUXER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_AUDIO_IAUDIOSOURCE_H
#include "gucefAUDIO_IAudioSource.h"
#define GUCEF_AUDIO_IAUDIOSOURCE_H
#endif /* GUCEF_AUDIO_IAUDIOSOURCE_H ? */

#ifndef GUCEF_VIDEO_IVIDEOSOURCE_H
#include "gucefVIDEO_IVideoSource.h"
#define GUCEF_VIDEO_IVIDEOSOURCE_H
#endif /* GUCEF_VIDEO_IVIDEOSOURCE_H ? */

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
 *  Shares one video source between whoever shows the pictures and whoever plays the sound. Both are fed from the
 *  same sequential decoder: what one of them makes the decoder produce ahead of the other is queued for the
 *  other. The picture side calls ReadFrame(); the sound of a track is an AUDIO::IAudioSource made with
 *  CreateAudioTrackSource(), to be played as a stream (AUDIO::CAudioStream reads it on its own task).
 *
 *  The queue of pictures is bounded (SetMaxQueuedFrames): when the sound is that far ahead of the pictures it
 *  gets nothing more until pictures are taken. Only tracks that have a source are queued, and pictures are only
 *  queued while a picture consumer is set (SetPictureConsumer, on by default).
 *
 *  All member functions can be called from any thread.
 */
class GUCEF_VIDEO_PUBLIC_CPP CVideoDemuxer : private IVideoAudioSink
{
    public:

    typedef CORE::CTSharedPtr< CVideoDemuxer, MT::CMutex > TVideoDemuxerPtr;

    static const UInt32 DefaultMaxQueuedFrames = 64;

    explicit CVideoDemuxer( const TVideoSourcePtr& source );

    virtual ~CVideoDemuxer() GUCEF_VIRTUAL_OVERRIDE;

    const CVideoFormat& GetFormat( void ) const;

    UInt32 GetAudioTrackCount( void ) const;

    bool GetAudioTrackFormat( UInt32 track, AUDIO::CAudioFormat& format ) const;

    /**
     *  The next picture: the oldest one that was queued, or else the next one decoded. False when there is no
     *  next picture (the end, an error: see IsEndOfStream() and HasFailed()).
     */
    bool ReadFrame( CVideoFrame& frame );

    /** As ReadFrame() but without handing the picture out, for frames that are late and will not be shown. 'paletteChanged' tells whether the skipped frame changed the palette */
    bool SkipFrame( bool* paletteChanged = GUCEF_NULL );

    /**
     *  The sound of a track as a source to play. The source keeps the demuxer alive. Its length is not known
     *  up front and it is not seekable. NULL for a track the video does not have.
     */
    static AUDIO::TAudioSourcePtr CreateAudioTrackSource( const TVideoDemuxerPtr& demuxer, UInt32 track );

    /** False: nobody takes the pictures, so they are not queued (playing only the sound of a video) */
    void SetPictureConsumer( bool consumesPictures );

    void SetMaxQueuedFrames( UInt32 maxFrames );
    UInt32 GetMaxQueuedFrames( void ) const;

    UInt32 GetQueuedFrameCount( void ) const;

    /** The source has ended and every queued picture has been read */
    bool IsEndOfStream( void ) const;

    /** The source failed to decode */
    bool HasFailed( void ) const;

    private:
    friend class CVideoAudioTrackSource;

    /** For the audio track sources: up to 'frameCapacity' frames of the track. 'ended' is set when there will be no more */
    UInt32 ReadAudio( UInt32 track, void* destination, UInt32 frameCapacity, bool& ended );

    void SubscribeTrack( UInt32 track, bool subscribe );

    /** Decodes one frame into m_decodeFrame, queueing the sound for the tracks and (when 'queuePicture') a copy of the picture. Lock held. False when nothing could be decoded */
    bool DecodeOne( bool queuePicture );

    /** A queued frame from the pool or a new one. Lock held */
    CVideoFrame* AcquireQueueFrame( void );

    virtual void OnVideoAudio( UInt32 track, const void* frames, UInt32 frameCount ) GUCEF_VIRTUAL_OVERRIDE;

    CVideoDemuxer( const CVideoDemuxer& src );              /**< not implemented */
    CVideoDemuxer& operator=( const CVideoDemuxer& src );   /**< not implemented */

    private:

    struct SAudioQueue
    {
        CORE::CDynamicBuffer data;
        UInt32 readOffset;          /**< bytes of 'data' that were read already */
        UInt32 frameSize;           /**< bytes per frame of the track */
        UInt32 subscribers;

        SAudioQueue( void );
    };
    typedef GUCEF::vector< SAudioQueue > TAudioQueueVector;
    typedef GUCEF::vector< CVideoFrame* > TFrameList;     /**< a short queue: taken from the front */
    typedef GUCEF::vector< CVideoFrame* > TFrameVector;

    MT::CMutex m_lock;
    TVideoSourcePtr m_source;
    CVideoFormat m_format;
    CVideoFrame m_decodeFrame;          /**< the frame the decoder draws into, so delta frames build on the previous picture */
    TFrameList m_queuedFrames;
    TFrameVector m_freeFrames;
    TAudioQueueVector m_audioQueues;
    bool m_consumesPictures;
    UInt32 m_maxQueuedFrames;
};

typedef CVideoDemuxer::TVideoDemuxerPtr TVideoDemuxerPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VIDEO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VIDEO_CVIDEODEMUXER_H ? */
