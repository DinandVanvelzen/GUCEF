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

#ifndef GUCEF_AUDIO_CSINCRESAMPLER_H
#define GUCEF_AUDIO_CSINCRESAMPLER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_IAUDIORESAMPLER_H
#include "gucefAUDIO_IAudioResampler.h"
#define GUCEF_AUDIO_IAUDIORESAMPLER_H
#endif /* GUCEF_AUDIO_IAUDIORESAMPLER_H ? */

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
 *  High quality resampler: windowed sinc interpolation (polyphase) with a ratio that can change continuously while
 *  the stream plays. Meant for sample rate conversion of voice and music, and for the small ratio corrections
 *  (a few hundred ppm to 0.5 %) that keep a playout in step with a sender whose clock runs at a slightly
 *  different speed.
 *
 *  Quality and cost:
 *  - the kernel is a Kaiser windowed sinc (beta 8, about 80 dB of stop band rejection) of 32 taps: 16 input frames to
 *    each side of the output position, cut off at 0.45 of the lower of the two sample rates
 *  - the kernel is tabulated at 256 phases per input frame and linearly interpolated between neighbouring phases
 *    (the interpolation error is far below the window's 80 dB)
 *  - when the output rate is the lower one (the input rate is more than the output rate) the kernel is stretched by
 *    the ratio so the cut off follows the output rate and nothing above it aliases. The tap count grows in step:
 *    32 taps at ratio 1 or less, 96 at 3 to 1 (48 kHz to 16 kHz), 1024 at the limit of 32 to 1. Cost per output
 *    frame is the tap count in multiply-adds, plus the same amount for computing the coefficients once per output
 *    frame (shared by all channels)
 *  - passband: flat within 0.1 dB up to about 0.38 of the lower rate, the transition band is centred on 0.45
 *  - the gain at DC is exactly 1 at every phase and ratio, so a ratio change never changes the level
 *  - ratio 1 is not a bit exact copy (the kernel has the cut off); callers that need a copy should not resample
 *
 *  A ratio change takes effect with the next output frame; the output position is continuous so nothing clicks. The
 *  stretching of the kernel is continuous in the ratio as well (it starts at ratio 1).
 *
 *  Latency: the first output corresponds to the first input frame and is made once 16 input frames beyond it are
 *  there; GetLatencyInFrames() reports 16, and 16 times the ratio (rounded up) when downsampling.
 *
 *  Nothing allocates or locks after Configure(): Process() and SetRates() are fit for the audio thread. The
 *  filter has no feedback; the output never holds denormal numbers (tiny values are flushed to zero). Not thread
 *  safe: one thread at a time uses one resampler. Process() uses only the frames of the stream it was given: it
 *  may consume part of the input (the frames no longer needed) when the output capacity ran out.
 */
class GUCEF_AUDIO_PUBLIC_CPP CSincResampler : public IAudioResampler
{
    public:

    enum
    {
        HALF_WIDTH      = 16 ,      /**< input frames to each side of the output position at ratio 1 and below */
        PHASE_COUNT     = 256 ,     /**< table entries per input frame */
        MAX_STEP        = 32        /**< largest input rate / output rate, downsampling stronger than this is refused */
    };

    CSincResampler( void );

    virtual ~CSincResampler() GUCEF_VIRTUAL_OVERRIDE;

    /** Allocates (tables, history, scratch space), not for the real time thread. Rates are limited to a ratio up to MAX_STEP */
    virtual bool Configure( UInt32 channelCount, Float64 inputRate, Float64 outputRate ) GUCEF_VIRTUAL_OVERRIDE;

    /** Real time safe. False (and no change) when the ratio is not above 0 or higher than MAX_STEP, or when not configured */
    virtual bool SetRates( Float64 inputRate, Float64 outputRate ) GUCEF_VIRTUAL_OVERRIDE;

    /** Clears the history and starts the stream over: the next output corresponds to the next input frame */
    virtual void Reset( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Process( const Float32* input          ,
                          UInt32 inputFrames            ,
                          UInt32& inputFramesConsumed   ,
                          Float32* output               ,
                          UInt32 outputFrameCapacity    ,
                          UInt32& outputFramesProduced  ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetLatencyInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual IAudioResampler* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The current ratio: input frames per output frame */
    Float64 GetStep( void ) const;

    private:

    CSincResampler( const CSincResampler& src );            /**< not implemented, use Clone() */
    CSincResampler& operator=( const CSincResampler& src ); /**< not implemented */

    void BuildTables( void );

    /** Input frames to each side of the output position for the current ratio */
    UInt32 CurrentHalfWidth( void ) const;

    void ComputeCoefficients( Float64 position, UInt32 halfWidth );

    void PushHistory( const Float32* input, UInt32 frames );

    private:

    UInt32 m_channelCount;
    Float64 m_step;                         /**< input frames per output frame */
    Float64 m_position;                     /**< position of the next output in input frames, 0 is the first frame of the chunk offered next */
    GUCEF::vector< Float32 > m_phaseTable;  /**< ( PHASE_COUNT + 1 ) rows of 2 * HALF_WIDTH taps, every row adds up to 1: the kernel for ratios up to 1 */
    GUCEF::vector< Float32 > m_kernel;      /**< the kernel at 1 / PHASE_COUNT spacing from the centre outwards, for stretching */
    GUCEF::vector< Float32 > m_history;     /**< ring of the most recent frames that were consumed */
    GUCEF::vector< Float32 > m_coefficients;
    GUCEF::vector< Float32 > m_window;      /**< gathered frames when the taps reach into the history */
    UInt32 m_historyWrite;                  /**< frames written to the ring so far, wraps harmlessly with the power of two size */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CSINCRESAMPLER_H ? */
