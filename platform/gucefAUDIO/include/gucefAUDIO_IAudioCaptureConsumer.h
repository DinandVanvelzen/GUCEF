/*
 *  gucefAUDIO: GUCEF module providing audio playback, mixing and decoding
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

#ifndef GUCEF_AUDIO_IAUDIOCAPTURECONSUMER_H
#define GUCEF_AUDIO_IAUDIOCAPTURECONSUMER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_CAUDIOFORMAT_H
#include "gucefAUDIO_CAudioFormat.h"
#define GUCEF_AUDIO_CAUDIOFORMAT_H
#endif /* GUCEF_AUDIO_CAUDIOFORMAT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What takes the processed audio of the capture path (CAudioCapturePath, through CAudioEngine::AddCaptureConsumer()):
 *  mono float blocks at the processing rate of the path, after the effect stages, the microphone gain and the meter.
 *  A voice sender is the typical consumer.
 *
 *  OnCapturedAudio() is called on the capture thread, which is a real time thread: it must not allocate, lock, log,
 *  notify, do IO or post a semaphore. Everything it needs is made in PrepareForCapture(), on the owner thread.
 *
 *  The engine keeps a plain pointer to the consumer from AddCaptureConsumer() until the future of RemoveCaptureConsumer()
 *  is ready (or the engine was shut down): the consumer must live at least that long.
 */
class GUCEF_AUDIO_PUBLIC_CPP IAudioCaptureConsumer
{
    public:

    /** What comes with a block of captured audio */
    struct SCaptureBlockInfo
    {
        /**
         *  Absolute position of the first frame of the block at the processing rate. Continuous from block to block; after
         *  an overrun it jumps forward by the frames that were lost (an estimate from the monotonic clock: devices do not
         *  report how many frames an overrun cost)
         */
        UInt64 firstFrame;

        /** MT::PrecisionTickCount() at the start of the device callback that delivered the audio of this block */
        UInt64 tickCount;

        /** The device overran (audio was lost) before this block */
        bool discontinuity;

        /** The decision of the voice activity detector for the last completed analysis frame */
        bool voiceActive;

        /** RMS level of the block after all processing, in dBFS (CAudioLevelMeter::MinimumDecibels for silence) */
        Float32 levelInDecibels;

        SCaptureBlockInfo( void )
            : firstFrame( 0 ), tickCount( 0 ), discontinuity( false ), voiceActive( false ), levelInDecibels( 0.0f )
        {}
    };

    virtual ~IAudioCaptureConsumer() {}

    /**
     *  Owner thread, called by CAudioEngine::AddCaptureConsumer() before the consumer is handed to the capture thread, with
     *  the engine locked (do not wait here for another thread that uses the engine). 'processingFormat' is mono Float32 at
     *  the processing rate; no block is longer than 'maxBlockFrames'. False refuses the registration
     */
    virtual bool PrepareForCapture( const CAudioFormat& processingFormat, UInt32 maxBlockFrames ) = 0;

    /**
     *  Capture thread: a block of processed mono audio. 'mono' is only valid during the call. It runs inside the real time
     *  context of the capture path (GUCEF_AUDIO_REALTIME_CONTEXT), so the RealtimeSanitizer build checks it as well
     */
    virtual void OnCapturedAudio( const Float32* mono, UInt32 frameCount, const SCaptureBlockInfo& info ) = 0;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_IAUDIOCAPTURECONSUMER_H ? */
