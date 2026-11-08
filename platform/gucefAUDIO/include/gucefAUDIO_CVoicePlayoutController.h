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

#ifndef GUCEF_AUDIO_CVOICEPLAYOUTCONTROLLER_H
#define GUCEF_AUDIO_CVOICEPLAYOUTCONTROLLER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_MACROS_H
#include "gucefAUDIO_macros.h"
#define GUCEF_AUDIO_MACROS_H
#endif /* GUCEF_AUDIO_MACROS_H ? */

#ifndef GUCEF_AUDIO_ETYPES_H
#include "gucefAUDIO_ETypes.h"
#define GUCEF_AUDIO_ETYPES_H
#endif /* GUCEF_AUDIO_ETYPES_H ? */

#ifndef GUCEF_AUDIO_CSLIDINGQUANTILE_H
#include "gucefAUDIO_CSlidingQuantile.h"
#define GUCEF_AUDIO_CSLIDINGQUANTILE_H
#endif /* GUCEF_AUDIO_CSLIDINGQUANTILE_H ? */

#ifndef GUCEF_AUDIO_CVOICEJITTERBUFFER_H
#include "gucefAUDIO_CVoiceJitterBuffer.h"
#define GUCEF_AUDIO_CVOICEJITTERBUFFER_H
#endif /* GUCEF_AUDIO_CVOICEJITTERBUFFER_H ? */

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

class CVoiceDecoder;

/*-------------------------------------------------------------------------*/

/**
 *  Playout control of one voice stream: decides when a talkspurt starts to play, which frame is decoded, concealed
 *  or recovered with forward error correction, and how fast the decoded audio is consumed so the buffer stays at
 *  its margin while the clocks of sender and receiver drift apart.
 *
 *  It drives a CVoiceJitterBuffer and a CVoiceDecoder; the caller (the playout source of a receiver) owns those, a
 *  FIFO of decoded audio and a resampler that consumes the audio at GetRatio() input frames per output frame.
 *  Per render request of the device the caller does:
 *  1. BeginRequest( now, frames of the request, decoded frames not yet output )
 *  2. Insert() for every frame that arrived since the last request (the inbound queue drained)
 *  3. UpdateRatio(): the control step; the result goes to the resampler
 *  4. Produce() until the FIFO holds what the resampler needs for the request
 *
 *  States: IDLE (nothing buffered, silence), PREBUFFER (a talkspurt is buffered, playout waits until the oldest
 *  frame has waited the margin M plus one render block), PLAYING. The playout of a talkspurt starts with silence
 *  of the part of a block that puts the first sample exactly at its arrival time plus M plus one block.
 *
 *  While playing every Produce() makes one frame: the frame due (decoded), a missing frame while a newer one of the
 *  talkspurt is there (forward error correction from the next frame when the decoder supports it, else
 *  concealment), or concealment when nothing is buffered. A frame of the talkspurt that arrives while concealment
 *  stands in for it is played after the concealment (the playout stretches, the margin grows by the stretch);
 *  frames older than the last decoded one are late and dropped. After 'concealLimit' concealed frames with nothing
 *  newer the talkspurt is over (IDLE); a frame whose timestamp is further ahead than its sequence number accounts for
 *  starts a new talkspurt (PREBUFFER).
 *
 *  Level and margin: every arriving frame gives a slack: the time from its arrival to the first render request that
 *  can need it. Lmin is the 'lateQuantile' quantile of the slack of the last 'window' seconds (CSlidingQuantile,
 *  corrected for the frames the playout stretched or dropped since); with a quantile of 0 (the wired profile) it is
 *  the smallest slack. The margin M is the target of Lmin. It starts at its initial value and shrinks by 1 ms after
 *  every decay interval without a late frame, not below the minimum, 'jitterMarginFactor' times the RFC 3550 jitter,
 *  nor, while the frames carry RFC 2198 redundancy (and a window after), the timestamp offset of the redundant copy,
 *  which comes that much later, nor, when the decoder's configuration has in-band forward error correction, one
 *  frame (the frame after a lost one carries its recovery data and must be there when the lost one is due). A frame the playout stretched for (the buffer ran dry) grows M by its lateness
 *  rounded up to a frame, at most to the maximum. A late frame that was dropped (a newer one was played) does the
 *  same with a quantile of 0; with a quantile above 0 it does not grow M: its slack is in the window and lowers
 *  Lmin, so the ratio adds the delay it asks for. So with a quantile q about a fraction q of the frames come too late
 *  for their first render request (they are concealed), and in return the delay follows the bulk of the delay
 *  distribution instead of its worst value: a heavy-tailed Wi-Fi delay does not pin M at its maximum, while stalls
 *  that empty the buffer are still waited for.
 *
 *  Ratio: e = Lmin - M (0 within one sample: the dead band), I += Ki e dt (clamped), r = 1 + clamp( Kp e + I ) with
 *  the bound of +-0.6 %, slewed by at most 1e-5 per request. r > 1 consumes faster. A decoded frame of silence (RMS
 *  below -60 dBFS) or a concealed frame is dropped while Lmin exceeds M by more than two frames.
 *
 *  Threading: Prepare() allocates and belongs to the owner thread. Everything else allocates nothing, takes no lock
 *  and is bounded in time (at most two decoder calls per Produce()), for the render thread. One thread at a time.
 */
class GUCEF_AUDIO_PUBLIC_CPP CVoicePlayoutController
{
    public:

    /** The tuning of the playout control. The defaults are the wireless profile */
    struct GUCEF_AUDIO_PUBLIC_CPP SSettings
    {
        Float64 initialMarginInMs;          /**< margin M at the start: 20 (wireless), 4 (wired) */
        Float64 minMarginInMs;              /**< M never decays below: 6, 2 */
        Float64 maxMarginInMs;              /**< M never grows above: 200, 60 */
        Float64 windowInS;                  /**< Lmin is the quantile of the slack of this many seconds: 4, 2 */
        Float64 decayIntervalInS;           /**< M shrinks after this long without a late frame: 1 (wireless), 10 (wired) */
        Float64 decayStepInMs;              /**< by this much: 1 */
        Float64 jitterMarginFactor;         /**< M does not decay below this many times the interarrival jitter: 0 (wireless: the quantile covers the jitter), 3 (wired) */
        UInt32 concealLimit;                /**< concealed frames with nothing newer that end a talkspurt: 6 */
        Float64 proportionalGain;           /**< Kp in 1 / s: 0.2 */
        Float64 integralTimeInS;            /**< Ki = Kp / this: 30 */
        Float64 maxRatioDeviation;          /**< bound of r - 1: 0.006 (drift up to 0.5 % is corrected with headroom left) */
        Float64 integralLimit;              /**< bound of the integral: 0.006 */
        Float64 maxRatioStepPerRequest;     /**< slew limit of r: 1e-5 */
        Float64 silenceThresholdInDb;       /**< frames below this RMS (dBFS) may be dropped: -60 */
        Float64 excessFramesForDrop;        /**< frames may be dropped while Lmin > M + this many frames: 2 */
        Float64 lateQuantile;               /**< Lmin is this quantile of the slack, 0 to below 0.5: 0.01 (wireless), 0 (wired: the minimum, and late frames grow M) */

        SSettings( void );

        /** Sets the margins, the window, the decay interval, the jitter factor and the quantile of a profile (section 4.8 of the design), leaves the rest */
        void SetProfile( TVoicePlayoutProfile profile );
    };

    enum EProduced
    {
        PRODUCED_SILENCE = 0 ,  /**< nothing plays (idle, or waiting for the start of a talkspurt) */
        PRODUCED_DECODED ,      /**< a frame was decoded */
        PRODUCED_RECOVERED ,    /**< a lost frame was recovered from the next one (forward error correction) */
        PRODUCED_CONCEALED      /**< concealment stood in for a missing frame */
    };

    /** What one Produce() made */
    struct GUCEF_AUDIO_PUBLIC_CPP SProduceInfo
    {
        EProduced kind;
        UInt64 extendedTimestamp;           /**< of the frame made (0 for silence) */
        UInt64 extendedSequenceNumber;      /**< of a decoded frame */
        UInt64 arrivalTimeInUs;             /**< of a decoded frame */
        UInt64 captureTickCount;            /**< of a decoded frame, as the sender set it (latency statistics where the clocks compare) */

        SProduceInfo( void );
    };

    /** Counters since Prepare() or Reset(), and the current control values */
    struct GUCEF_AUDIO_PUBLIC_CPP SStatistics
    {
        TVoicePlayoutState state;
        UInt64 framesDecoded;               /**< frames decoded and played */
        UInt64 framesConcealed;             /**< frames made by concealment */
        UInt64 framesRecovered;             /**< frames made by forward error correction from the next frame */
        UInt64 framesAccelerated;           /**< silent or concealed frames dropped to lower the delay */
        UInt64 framesStretched;             /**< frames of a talkspurt played after concealment had stood in for them */
        UInt64 underruns;                   /**< times the buffer ran dry while a talkspurt went on (resolved by a stretch) */
        UInt64 lateFrames;                  /**< frames dropped because their time had passed while playing */
        UInt64 decodeErrors;                /**< frames the decoder refused (concealed instead) */
        UInt64 talkspurts;                  /**< playout starts */
        Float64 ratio;                      /**< current playout ratio, input frames per output frame */
        Float64 integral;                   /**< current integral term (the estimate of the clock drift) */
        Float64 marginInS;                  /**< M */
        Float64 minimumLevelInS;            /**< Lmin (the quantile of the slack the margin applies to), 0 when there is none yet */
        bool hasMinimumLevel;
        Float64 levelInS;                   /**< the audio buffered beyond the current render block */
        Float64 lastPlayoutDelayInS;        /**< from the arrival of the last decoded frame to the output of its first sample */
        Float64 meanPlayoutDelayInS;        /**< mean of the same since the last playout start */
        Float64 maxPlayoutDelayInS;
        Float64 jitterInS;                  /**< RFC 3550 interarrival jitter */

        SStatistics( void );
    };

    CVoicePlayoutController( void );

    ~CVoicePlayoutController();

    /**
     *  Links the buffer and the decoder (both prepared, they must outlive this object or the next Prepare()) and
     *  allocates the minimum window. Owner thread only. False when the buffer is not prepared or the decoder's frame
     *  is not a whole number of samples.
     */
    bool Prepare( const SSettings& settings, CVoiceJitterBuffer& buffer, CVoiceDecoder& decoder );

    bool IsPrepared( void ) const;

    const SSettings& GetSettings( void ) const;

    /** Forgets the stream: buffer, decoder, state, margin, integral and statistics. Real time safe */
    void Reset( void );

    /**
     *  Starts a render request at 'nowInUs' (the clock of the arrival times). 'requestFrames' is the decoded audio
     *  (at the decoder rate) the request needs beyond its output position: the frames it outputs plus the lookahead
     *  of the resampler; it is the render block of the margin rules. 'pendingFrames' is the decoded audio produced
     *  earlier and not output yet (the FIFO plus what the resampler consumed beyond the output position). Real time
     *  safe.
     */
    void BeginRequest( UInt64 nowInUs, UInt32 requestFrames, UInt32 pendingFrames );

    /** Hands an arrived frame to the jitter buffer and measures its slack. Real time safe */
    CVoiceJitterBuffer::EInsertResult Insert( const CVoiceFrame& frame, UInt64 arrivalTimeInUs );

    /** The control step of the request: margin decay, Lmin, integral, ratio. Returns the new ratio. Real time safe */
    Float64 UpdateRatio( void );

    /**
     *  Makes audio at the decoder rate into 'pcm': while playing exactly one frame (GetFrameSizeInFrames()), else
     *  silence up to 'maxFrames' (less when the playout starts within them). Returns the frames written; 0 when a
     *  frame is to be made and 'maxFrames' is less than a frame (offer at least a frame then), or when nothing is
     *  prepared. 'info', when given, tells what was made. Real time safe.
     */
    UInt32 Produce( Float32* pcm, UInt32 maxFrames, SProduceInfo* info = GUCEF_NULL );

    TVoicePlayoutState GetState( void ) const;

    /** Input frames per output frame for the resampler; 1 exactly while nothing moves the playout */
    Float64 GetRatio( void ) const;

    Float64 GetMarginInS( void ) const;

    /** Lmin (the quantile of the slack) in seconds; false when there is no slack measured since the playout started */
    bool GetMinimumLevel( Float64& levelInS ) const;

    /** Samples of one frame at the decoder rate */
    UInt32 GetFrameSizeInFrames( void ) const;

    UInt32 GetOutputRate( void ) const;

    void GetStatistics( SStatistics& stats ) const;

    private:

    CVoicePlayoutController( const CVoicePlayoutController& src );              /**< not implemented */
    CVoicePlayoutController& operator=( const CVoicePlayoutController& src );   /**< not implemented */

    Float64 GetOutputPositionTimestamp( void ) const;
    Float64 GetSlack( UInt64 extendedTimestamp, UInt64 arrivalTimeInUs ) const;
    void PushSlack( Float64 slackInS );
    void ShiftTimeline( Float64 seconds );
    void GrowMargin( Float64 latenessInS, bool stretched );
    UInt32 WriteSilence( Float32* pcm, UInt32 frames );
    UInt32 ProduceWhileWaiting( Float32* pcm, UInt32 maxFrames, SProduceInfo& produced );
    bool StartTalkspurt( void );
    UInt32 ProducePlaying( Float32* pcm, UInt32 maxFrames, SProduceInfo& produced );
    bool Conceal( Float32* pcm );
    bool IsSilent( const Float32* pcm ) const;
    UInt64 GetPlayoutTimestamp( void ) const;
    void StopPlayout( TVoicePlayoutState nextState );

    private:

    SSettings m_settings;
    CVoiceJitterBuffer* m_buffer;
    CVoiceDecoder* m_decoder;
    UInt32 m_outputRate;
    UInt32 m_frameSize;
    UInt32 m_timestampUnitsPerFrame;
    UInt32 m_timestampRate;
    Float64 m_frameDurationInS;
    Float64 m_kp;
    Float64 m_ki;
    Float64 m_silenceThreshold;             /**< RMS, linear */
    CSlidingQuantile m_slackWindow;

    TVoicePlayoutState m_state;
    UInt32 m_concealed;                     /**< frames concealed since the last decoded frame */
    bool m_concealedForEmpty;               /**< part of the current concealment was made with nothing buffered */
    UInt32 m_emptySince;                    /**< the concealed frame from which on nothing was buffered */
    bool m_hasSlack;
    Float64 m_timelineShift;                /**< seconds the playout stretched (+) or dropped (-) since the window's reference */
    Float64 m_margin;
    Float64 m_integral;
    Float64 m_ratio;
    Float64 m_sinceLateInS;
    Float64 m_redundancyFloor;              /**< M is at least this while frames carry RFC 2198 redundancy: the offset of the redundant copy */
    Float64 m_sinceRedundancyInS;
    Float64 m_fecFloor;                     /**< M is at least a frame when the decoder recovers frames with in-band FEC (the next frame must be there) */
    Float64 m_level;

    // the current request
    bool m_hasRequest;
    UInt64 m_nowInUs;
    Float64 m_blockInS;
    UInt32 m_pending;
    UInt32 m_produced;
    Float64 m_elapsedInS;                   /**< since the previous request */

    Float64 m_delaySum;
    UInt64 m_delayCount;
    SStatistics m_stats;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CVOICEPLAYOUTCONTROLLER_H ? */
