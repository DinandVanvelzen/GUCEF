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

#ifndef GUCEF_AUDIO_IAUDIORESAMPLER_H
#define GUCEF_AUDIO_IAUDIORESAMPLER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_MACROS_H
#include "gucefAUDIO_macros.h"
#define GUCEF_AUDIO_MACROS_H
#endif /* GUCEF_AUDIO_MACROS_H ? */

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
 *  Changes the sample rate of a stream of float audio, in pieces: the audio is handed over in chunks of any size
 *  and the output is the same as if it had been handed over at once.
 *
 *  Rates are floating point so the same interface does playback speed changes (pitch) as well as plain rate
 *  conversion, and the ratio can be changed while the stream plays. Better quality implementations (windowed sinc,
 *  polyphase) derive from this class, they can be provided by plugins and take the place of the default one.
 */
class GUCEF_AUDIO_PUBLIC_CPP IAudioResampler
{
    public:

    virtual ~IAudioResampler() {}

    /**
     *  Sets up the channel count and the ratio. The ratio is inputRate / outputRate:
     *  2.0 makes 1 output frame for every 2 input frames. Keeps the position in the stream when called again.
     */
    virtual bool Configure( UInt32 channelCount, Float64 inputRate, Float64 outputRate ) = 0;

    /** Changes the rates without losing the position in the stream (pitch changes while playing) */
    virtual bool SetRates( Float64 inputRate, Float64 outputRate ) = 0;

    /** Forgets everything about the stream so far */
    virtual void Reset( void ) = 0;

    /**
     *  Converts as much as fits. Both 'inputFrames' and 'outputFrameCapacity' are in frames of interleaved floats.
     *  The input that was used is reported in 'inputFramesConsumed', the caller offers the rest again next time.
     *  A resampler may keep a few frames inside (see GetLatencyInFrames) which come out with later input.
     */
    virtual void Process( const Float32* input          ,
                          UInt32 inputFrames            ,
                          UInt32& inputFramesConsumed   ,
                          Float32* output               ,
                          UInt32 outputFrameCapacity    ,
                          UInt32& outputFramesProduced  ) = 0;

    /** Frames of input that are needed before the first output corresponds to the first input */
    virtual UInt32 GetLatencyInFrames( void ) const = 0;

    /** Makes an independent resampler of the same kind, with the same setup and no stream history */
    virtual IAudioResampler* Clone( void ) const = 0;

    virtual const char* GetName( void ) const = 0;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_IAUDIORESAMPLER_H ? */
