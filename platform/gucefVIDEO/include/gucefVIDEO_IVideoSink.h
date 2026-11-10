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

#ifndef GUCEF_VIDEO_IVIDEOSINK_H
#define GUCEF_VIDEO_IVIDEOSINK_H

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
 *  A video being encoded, frame by frame: the pictures in order, the sound of the audio tracks as it comes with
 *  them. Finish() completes the output; a sink that is destroyed without it leaves an incomplete output.
 *  A sink is used from one thread at a time.
 */
class GUCEF_VIDEO_PUBLIC_CPP IVideoSink
{
    public:

    typedef CORE::CTSharedPtr< IVideoSink, MT::CMutex > TVideoSinkPtr;

    virtual ~IVideoSink();

    /** Size and frame rate the sink was opened with. The pixel format is what the pictures are encoded from */
    virtual const CVideoFormat& GetFormat( void ) const = 0;

    virtual UInt32 GetAudioTrackCount( void ) const = 0;

    /** The format WriteAudio() takes for a track: the one the sink was opened with */
    virtual bool GetAudioTrackFormat( UInt32 track, AUDIO::CAudioFormat& format ) const = 0;

    /** Encodes the next picture. The frame can have any pixel format, of the size of the sink. False on an error */
    virtual bool WriteFrame( const CVideoFrame& frame ) = 0;

    /** Encodes sound of a track: interleaved frames in the format of the track. False on an error */
    virtual bool WriteAudio( UInt32 track, const void* frames, UInt32 frameCount ) = 0;

    /** Writes what the output needs at its end. After it nothing more can be written. False on an error */
    virtual bool Finish( void ) = 0;

    /** True when a write or Finish() failed */
    virtual bool HasFailed( void ) const = 0;

    /** The number of pictures written so far */
    virtual UInt64 GetFramesWritten( void ) const = 0;
};

typedef IVideoSink::TVideoSinkPtr TVideoSinkPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VIDEO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VIDEO_IVIDEOSINK_H ? */
