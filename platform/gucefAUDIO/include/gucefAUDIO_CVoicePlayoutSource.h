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

#ifndef GUCEF_AUDIO_CVOICEPLAYOUTSOURCE_H
#define GUCEF_AUDIO_CVOICEPLAYOUTSOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTSLOTDATAEXCHANGE_H
#include "gucefMT_CTSlotDataExchange.h"
#define GUCEF_MT_CTSLOTDATAEXCHANGE_H
#endif /* GUCEF_MT_CTSLOTDATAEXCHANGE_H ? */

#ifndef GUCEF_AUDIO_IAUDIOSOURCE_H
#include "gucefAUDIO_IAudioSource.h"
#define GUCEF_AUDIO_IAUDIOSOURCE_H
#endif /* GUCEF_AUDIO_IAUDIOSOURCE_H ? */

#ifndef GUCEF_AUDIO_CAUDIOLEVELMETER_H
#include "gucefAUDIO_CAudioLevelMeter.h"
#define GUCEF_AUDIO_CAUDIOLEVELMETER_H
#endif /* GUCEF_AUDIO_CAUDIOLEVELMETER_H ? */

#ifndef GUCEF_AUDIO_CSINCRESAMPLER_H
#include "gucefAUDIO_CSincResampler.h"
#define GUCEF_AUDIO_CSINCRESAMPLER_H
#endif /* GUCEF_AUDIO_CSINCRESAMPLER_H ? */

#ifndef GUCEF_AUDIO_CVOICEDECODER_H
#include "gucefAUDIO_CVoiceDecoder.h"
#define GUCEF_AUDIO_CVOICEDECODER_H
#endif /* GUCEF_AUDIO_CVOICEDECODER_H ? */

#ifndef GUCEF_AUDIO_CVOICEFRAME_H
#include "gucefAUDIO_CVoiceFrame.h"
#define GUCEF_AUDIO_CVOICEFRAME_H
#endif /* GUCEF_AUDIO_CVOICEFRAME_H ? */

#ifndef GUCEF_AUDIO_CVOICEJITTERBUFFER_H
#include "gucefAUDIO_CVoiceJitterBuffer.h"
#define GUCEF_AUDIO_CVOICEJITTERBUFFER_H
#endif /* GUCEF_AUDIO_CVOICEJITTERBUFFER_H ? */

#ifndef GUCEF_AUDIO_CVOICEPLAYOUTCONTROLLER_H
#include "gucefAUDIO_CVoicePlayoutController.h"
#define GUCEF_AUDIO_CVOICEPLAYOUTCONTROLLER_H
#endif /* GUCEF_AUDIO_CVOICEPLAYOUTCONTROLLER_H ? */

#ifndef GUCEF_AUDIO_CVOICERECEIVERCONFIG_H
#include "gucefAUDIO_CVoiceReceiverConfig.h"
#define GUCEF_AUDIO_CVOICERECEIVERCONFIG_H
#endif /* GUCEF_AUDIO_CVOICERECEIVERCONFIG_H ? */

#ifndef GUCEF_AUDIO_CVOICERECEIVERSTATS_H
#include "gucefAUDIO_CVoiceReceiverStats.h"
#define GUCEF_AUDIO_CVOICERECEIVERSTATS_H
#endif /* GUCEF_AUDIO_CVOICERECEIVERSTATS_H ? */

#ifndef GUCEF_AUDIO_CVOICECLOCK_H
#include "gucefAUDIO_CVoiceClock.h"
#define GUCEF_AUDIO_CVOICECLOCK_H
#endif /* GUCEF_AUDIO_CVOICECLOCK_H ? */

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
 *  The audio of a received voice stream as a live IAudioSource for an engine voice: mono Float32 at the output rate
 *  of the decoder. Made and owned by a CVoiceReceiver; usable on its own (a test, another mixer) as well.
 *
 *  Every ReadFrames() is a render request of the playout (section 4.8 of the design), in pieces of at most
 *  MaxRequestFrames:
 *   1. the time of the request from the clock (CVoiceClock), the clock of the arrival times
 *   2. a reset asked for since the last request is carried out; the frames that came from the network thread since
 *      then are taken from the inbound queue into the jitter buffer (CVoicePlayoutController::Insert())
 *   3. the control step gives the playout ratio
 *   4. the controller decodes, recovers (forward error correction) or conceals frames into a FIFO until it holds the
 *      request plus the lookahead of the resampler
 *   5. the request is filled from the FIFO: directly, bit for bit, while the ratio has been exactly 1 since the
 *      playout started; through a CSincResampler at the ratio from the first request on whose ratio is not 1. The
 *      switch continues the stream at the same sample (the resampler is primed with the audio before it), so it cannot
 *      be heard. The resampler then stays in use until the playout is reset (RequestReset(), a receiver's Start()):
 *      the controller's ratio does not come back to exactly 1 while its drift estimate is not 0
 *  It always returns exactly the frames asked for (silence while nothing plays) and never reaches an end.
 *
 *  Threads: Prepare() on the owner thread while nothing reads the source. PushFrame() on one producer thread at a time
 *  (the network thread). ReadFrames() on the audio thread: it allocates nothing, takes no lock, does no IO and logs
 *  nothing (IsRealtimeSafe() is true). RequestReset(), the state, the level and GetStats() from any thread.
 */
class GUCEF_AUDIO_PUBLIC_CPP CVoicePlayoutSource : public IAudioSource
{
    public:

    /** The longest piece of a render request handled at once; longer requests are made of several */
    static const UInt32 MaxRequestFrames = 1024;

    CVoicePlayoutSource( void );

    virtual ~CVoicePlayoutSource() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Owner, while nothing reads the source or pushes to it: sets up the inbound queue, the jitter buffer, the playout
     *  controller and the resampler for the decoder (prepared, made for config.GetCodecConfig()). The clock must outlive
     *  the source. False when the decoder or the configuration cannot be used
     */
    bool Prepare( const CVoiceReceiverConfig& config, const TVoiceDecoderPtr& decoder, const CVoiceClock& clock );

    bool IsPrepared( void ) const;

    /**
     *  Producer thread (one at a time): queues a frame for the audio thread with its arrival time in the clock of the
     *  source. False when the inbound queue is full (counted) or the source is not prepared. Never blocks
     */
    bool PushFrame( const CVoiceFrame& frame, UInt64 arrivalTimeInUs );

    /**
     *  Any thread: the audio thread forgets the stream at its next request (jitter buffer, decoder, playout state,
     *  resampler, statistics of the playout). Frames pushed before this call are dropped, frames pushed after it are
     *  the new stream
     */
    void RequestReset( void );

    /** Any thread: the state of the playout at the last request */
    TVoicePlayoutState GetPlayoutState( void ) const;

    /** Any thread: frames are being played (state PLAYING) */
    bool IsPlaying( void ) const;

    /** Any thread: RMS of the output (300 ms) in dBFS, CAudioLevelMeter::MinimumDecibels for silence */
    Float32 GetLevelInDecibels( void ) const;

    /** Any thread: frames waiting in the inbound queue */
    UInt32 GetPendingFrameCount( void ) const;

    /** Any thread: the latest statistics of the audio thread plus the counters of PushFrame() (the receiver adds its own) */
    void GetStats( CVoiceReceiverStats& stats ) const;

    /** The clock rate of the RTP timestamps of the stream */
    UInt32 GetTimestampRate( void ) const;

    virtual const CAudioFormat& GetFormat( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Audio thread: one render request. Always fills 'frameCapacity' frames */
    virtual UInt32 ReadFrames( void* destination, UInt32 frameCapacity ) GUCEF_VIRTUAL_OVERRIDE GUCEF_AUDIO_REALTIME_CONTEXT;

    /** False: a live stream has no end, the voice plays until it is stopped */
    virtual bool IsEndOfStream( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** True */
    virtual bool IsRealtimeSafe( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Frames output so far */
    virtual UInt64 GetPositionInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    /** A frame on its way from the network thread to the audio thread */
    struct SInboundFrame
    {
        CVoiceFrame frame;
        UInt64 arrivalTimeInUs;
        UInt32 generation;          /**< the reset generation it was pushed in */

        SInboundFrame( void );
    };

    typedef MT::CTSlotDataExchange< SInboundFrame, MT::SSlotDeliveryAllInOrder, MT::SSlotFullFail > TInboundQueue;
    typedef MT::CTSlotDataExchange< CVoiceReceiverStats, MT::SSlotDeliveryLatestOnly, MT::SSlotFullFail > TStatsExchange;

    CVoicePlayoutSource( const CVoicePlayoutSource& src );              /**< not implemented */
    CVoicePlayoutSource& operator=( const CVoicePlayoutSource& src );   /**< not implemented */

    void ResetPlayout( void ) GUCEF_AUDIO_REALTIME_CONTEXT;
    void DrainInbound( void ) GUCEF_AUDIO_REALTIME_CONTEXT;
    UInt64 RequestTime( UInt32 frames ) GUCEF_AUDIO_REALTIME_CONTEXT;
    void RenderPiece( Float32* output, UInt32 frames, UInt32 remaining, bool first ) GUCEF_AUDIO_REALTIME_CONTEXT;
    bool ProduceMore( UInt32 deficit, UInt64 nowInUs, Float64 ratio ) GUCEF_AUDIO_REALTIME_CONTEXT;
    void PrimeResampler( void ) GUCEF_AUDIO_REALTIME_CONTEXT;
    void CompactFifo( void ) GUCEF_AUDIO_REALTIME_CONTEXT;
    void PublishStats( void ) GUCEF_AUDIO_REALTIME_CONTEXT;

    private:

    bool m_prepared;
    CAudioFormat m_format;
    UInt32 m_rate;
    UInt32 m_frameSize;
    UInt32 m_lookahead;                         /**< input the resampler needs beyond its output at the largest ratio */
    UInt32 m_timestampRate;
    const CVoiceClock* m_clock;
    TVoiceDecoderPtr m_decoderRef;              /**< owner side: keeps the decoder alive */
    TInboundQueue* m_inbound;
    UInt32 m_inboundLimit;                      /**< the configured size of the inbound queue (its capacity is a power of two) */

    // any thread
    MT::CTAtomic< UInt32 > m_generation;        /**< raised by RequestReset() */
    MT::CTAtomic< UInt32 > m_state;
    MT::CTAtomic< UInt64 > m_position;
    MT::CTAtomic< UInt64 > m_framesQueued;      /**< written by the producer */
    MT::CTAtomic< UInt64 > m_inboundQueueFull;  /**< written by the producer */
    CAudioLevelMeter m_meter;

    // producer thread
    SInboundFrame m_pushItem;

    // audio thread
    CVoiceJitterBuffer m_buffer;
    CVoicePlayoutController m_controller;
    CSincResampler m_resampler;
    GUCEF::vector< Float32 > m_fifo;            /**< decoded audio: resampler history, then what is not output yet */
    UInt32 m_fifoCount;
    Float64 m_readOffset;                       /**< the next output sample, as a position in the FIFO (whole while not resampling) */
    UInt32 m_resamplerOffset;                   /**< the next FIFO frame the resampler takes */
    bool m_resampling;
    Float32 m_primeScratch[ 2 * CSincResampler::HALF_WIDTH ];
    UInt32 m_appliedGeneration;
    SInboundFrame m_popItem;
    bool m_hasRequest;
    UInt64 m_lastClockUs;
    Float64 m_lastNowUs;
    Float64 m_lastDurationUs;
    UInt64 m_requestNowUs;                      /**< the request being made */
    UInt32 m_requestFrames;
    Float64 m_oneWayDelaySum;
    CVoiceReceiverStats m_stats;
    CVoicePlayoutController::SStatistics m_controllerStats;
    CVoiceJitterBuffer::SStatistics m_bufferStats;
    mutable TStatsExchange m_statsExchange;     /**< mutable: reading it moves the reader's slot */

    // readers of the statistics, never the audio thread
    mutable MT::CMutex m_statsReadLock;
    mutable CVoiceReceiverStats m_statsRead;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CVOICEPLAYOUTSOURCE_H ? */
