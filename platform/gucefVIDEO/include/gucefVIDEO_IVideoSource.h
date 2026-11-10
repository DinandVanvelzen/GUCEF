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

#ifndef GUCEF_VIDEO_IVIDEOSOURCE_H
#define GUCEF_VIDEO_IVIDEOSOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_AUDIO_CAUDIOFORMAT_H
#include "gucefAUDIO_CAudioFormat.h"
#define GUCEF_AUDIO_CAUDIOFORMAT_H
#endif /* GUCEF_AUDIO_CAUDIOFORMAT_H ? */

#ifndef GUCEF_VIDEO_CVIDEOFRAME_H
#include "gucefVIDEO_CVideoFrame.h"
#define GUCEF_VIDEO_CVIDEOFRAME_H
#endif /* GUCEF_VIDEO_CVIDEOFRAME_H ? */

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
 *  Receives the sound that is stored with a frame, while the frame is decoded
 */
class GUCEF_VIDEO_PUBLIC_CPP IVideoAudioSink
{
    public:

    virtual ~IVideoAudioSink();

    /** Interleaved frames of audio in the format of the track (IVideoSource::GetAudioTrackFormat). Copy them, they are only valid during the call */
    virtual void OnVideoAudio( UInt32 track, const void* frames, UInt32 frameCount ) = 0;
};

/*-------------------------------------------------------------------------*/

/**
 *  A video being decoded, frame by frame. Movie files interleave pictures and sound and can mostly only be read
 *  in order, so each DecodeNextFrame() gives one picture and hands the sound stored with it to a sink.
 *  A source is used from one thread at a time (CVideoDemuxer shares one between a picture and a sound consumer).
 */
class GUCEF_VIDEO_PUBLIC_CPP IVideoSource
{
    public:

    typedef CORE::CTSharedPtr< IVideoSource, MT::CMutex > TVideoSourcePtr;

    virtual ~IVideoSource();

    /** Size, pixel format, frame rate and (when known) the number of frames. Does not change during the life of the source */
    virtual const CVideoFormat& GetFormat( void ) const = 0;

    virtual UInt32 GetAudioTrackCount( void ) const = 0;

    /** The format the sound of a track is handed to the sink in */
    virtual bool GetAudioTrackFormat( UInt32 track, AUDIO::CAudioFormat& format ) const = 0;

    /**
     *  Decodes the next frame: the picture into 'frame' (which is prepared for the format when it is not yet, and
     *  otherwise drawn over, so give the same frame each time for formats with delta frames), the sound to
     *  'audioSink' (NULL drops it). False at the end of the video or on an error: IsEndOfStream() tells which.
     */
    virtual bool DecodeNextFrame( CVideoFrame& frame, IVideoAudioSink* audioSink ) = 0;

    /** True when there will be no more frames, unless the source is sought */
    virtual bool IsEndOfStream( void ) const = 0;

    /** True when the last DecodeNextFrame() failed because the data could not be decoded */
    virtual bool HasFailed( void ) const = 0;

    /** The number of the frame the next DecodeNextFrame() gives */
    virtual UInt64 GetPositionInFrames( void ) const = 0;

    /** True when SeekToFrame() works. The default is false */
    virtual bool IsSeekable( void ) const;

    /** Moves to the given frame, after which DecodeNextFrame() gives it completely drawn. The default fails */
    virtual bool SeekToFrame( UInt64 frame );
};

typedef IVideoSource::TVideoSourcePtr TVideoSourcePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VIDEO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VIDEO_IVIDEOSOURCE_H ? */
