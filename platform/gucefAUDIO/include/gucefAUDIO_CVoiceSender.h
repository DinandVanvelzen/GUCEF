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

#ifndef GUCEF_AUDIO_CVOICESENDER_H
#define GUCEF_AUDIO_CVOICESENDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_MT_CTSLOTDATAEXCHANGE_H
#include "gucefMT_CTSlotDataExchange.h"
#define GUCEF_MT_CTSLOTDATAEXCHANGE_H
#endif /* GUCEF_MT_CTSLOTDATAEXCHANGE_H ? */

#ifndef GUCEF_AUDIO_IAUDIOCAPTURECONSUMER_H
#include "gucefAUDIO_IAudioCaptureConsumer.h"
#define GUCEF_AUDIO_IAUDIOCAPTURECONSUMER_H
#endif /* GUCEF_AUDIO_IAUDIOCAPTURECONSUMER_H ? */

#ifndef GUCEF_AUDIO_CVOICEENCODER_H
#include "gucefAUDIO_CVoiceEncoder.h"
#define GUCEF_AUDIO_CVOICEENCODER_H
#endif /* GUCEF_AUDIO_CVOICEENCODER_H ? */

#ifndef GUCEF_AUDIO_CVOICEFRAME_H
#include "gucefAUDIO_CVoiceFrame.h"
#define GUCEF_AUDIO_CVOICEFRAME_H
#endif /* GUCEF_AUDIO_CVOICEFRAME_H ? */

#ifndef GUCEF_AUDIO_CVOICESENDERCONFIG_H
#include "gucefAUDIO_CVoiceSenderConfig.h"
#define GUCEF_AUDIO_CVOICESENDERCONFIG_H
#endif /* GUCEF_AUDIO_CVOICESENDERCONFIG_H ? */

#ifndef GUCEF_AUDIO_CVOICESENDERSTATS_H
#include "gucefAUDIO_CVoiceSenderStats.h"
#define GUCEF_AUDIO_CVOICESENDERSTATS_H
#endif /* GUCEF_AUDIO_CVOICESENDERSTATS_H ? */

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
 *  The sending side of a voice stream: takes the processed microphone audio of the capture path (a capture consumer,
 *  CAudioEngine::AddCaptureConsumer()), cuts it into frames, decides per frame whether it is sent, encodes it and puts
 *  it, as a CVoiceFrame with its RTP fields, into an outbound queue that the network side empties with PopFrame().
 *  It owns no socket: CRtpVoicePacketCodec turns a frame into a packet.
 *
 *  Per frame on the capture thread:
 *   - the audio is accumulated into frames of the codec's frame duration (the codec rate is the processing rate of the
 *     capture path, nothing is resampled here)
 *   - transmit gating: continuous mode sends every frame; voice activated mode sends while the capture path's voice
 *     activity detector hears a voice and for the hangover after it, and at an onset first sends the pre-roll (the last
 *     encoded frames before it, which the detector needed to decide); push to talk sends while the talk key is down and
 *     the frame that was in progress when it was let go. Muted sends nothing
 *   - RTP fields: the sequence number counts sent frames; the timestamp follows the position of the frame's first sample
 *     in the captured audio (SCaptureBlockInfo::firstFrame), so silence and overruns are gaps in the timestamps; the
 *     marker is set on the first frame after a gap; the capture time is the tick count of the block the frame started in
 *   - with redundancy the previous frame travels along as an RFC 2198 block, as long as the packet stays within
 *     MaxPacketSize (otherwise the block is left out and counted)
 *   - a full outbound queue drops the frame (counted); its sequence number is used, so the receiver sees a loss
 *   - after an overrun (a discontinuity) the frame in progress is dropped and the next one starts where the audio resumes
 *
 *  Threads:
 *   - owner: construction, Prepare(), PrepareForCapture() (called by the engine, with the engine locked), SetCodecConfig()
 *     and destruction, while the engine does not use the sender (before AddCaptureConsumer(), after the future of
 *     RemoveCaptureConsumer() is ready) except for SetCodecConfig(), which hands the new encoder over to the capture
 *     thread
 *   - capture thread: OnCapturedAudio(): allocates nothing, takes no lock, does no IO, logs nothing, posts no semaphore
 *   - one network thread at a time: PopFrame(). The capture thread cannot wake it without a system call that locks, so
 *     the network side polls PopFrame() at least once per millisecond
 *   - any thread: the transmit mode, the talk key, mute, bit rate, expected loss, IsTransmitting(), GetStats()
 *
 *  Recommended capture chain of an intercom (the application sets it up in the engine, at a processing rate of 16 kHz on
 *  small units): pre-echo stage "filter" high pass 100 Hz 24 dB per octave; the echo stage a CAcousticEchoCanceller for
 *  calls (CAudioEngine::SetEchoCanceller(), with the half-duplex controller as its fallback); post-echo stage
 *  "noiseSuppressor" (or the canceller's residual suppressor with noise suppression, which saves a transform),
 *  "noiseGate", "automaticGain", "limiter" in instant mode at -1 dBFS.
 */
class GUCEF_AUDIO_PUBLIC_CPP CVoiceSender : public IAudioCaptureConsumer
{
    public:

    /** The largest RTP packet a frame may make: the UDP payload of a 1500 byte Ethernet MTU (1500 - 20 IP - 8 UDP) */
    static const UInt32 MaxPacketSize = 1472;

    explicit CVoiceSender( const CVoiceSenderConfig& config );

    virtual ~CVoiceSender() GUCEF_VIRTUAL_OVERRIDE;

    /** The configuration the sender was made with (the codec part is the one of construction, see GetCodecConfig()) */
    const CVoiceSenderConfig& GetConfig( void ) const;

    /**
     *  Owner: makes the encoder from the voice codec registry. PrepareForCapture() calls it when it was not done.
     *  False when the codec is unknown or does not support the configuration (logged)
     */
    bool Prepare( void );

    bool IsPrepared( void ) const;

    /** Any thread: when frames are sent, from the next frame on */
    void SetTransmitMode( TVoiceTransmitMode mode );
    TVoiceTransmitMode GetTransmitMode( void ) const;

    /** Any thread: the push to talk key. Releasing it still sends the frame that was in progress */
    void SetTransmitting( bool transmitting );
    bool IsTalkKeyDown( void ) const;

    /** Any thread: while muted nothing is encoded or sent, in every mode (the next frame after it starts a talkspurt) */
    void SetMuted( bool muted );
    bool IsMuted( void ) const;

    /** Any thread: whether the last completed frame was sent */
    bool IsTransmitting( void ) const;

    /** Any thread: passed on to the encoder at the next frame */
    void SetBitrate( UInt32 bitsPerSecond );
    void SetExpectedLossPercentage( UInt32 percentage );

    /**
     *  Owner: changes the codec while the stream runs. The new encoder is made here; the capture thread takes it at the
     *  start of the next frame. The first frame of the new codec is a new talkspurt (marker) with the payload type of
     *  the new codec, the timestamps go on from the position of that frame in the new timestamp rate. The sample rate
     *  must stay the processing rate. False when a change made before has not been taken yet (try again later), when the
     *  codec does not support the configuration, or when the sample rate differs from the processing rate
     */
    bool SetCodecConfig( const CVoiceCodecConfig& codecConfig );

    /** Owner: a change of SetCodecConfig() waits for the capture thread */
    bool IsCodecChangePending( void ) const;

    /** Owner: the codec configuration of the last successful Prepare() or SetCodecConfig() */
    const CVoiceCodecConfig& GetCodecConfig( void ) const;

    /** The RTP SSRC of the stream: the configured one, or the random one picked at construction */
    UInt32 GetStreamId( void ) const;

    /** Network thread (one at a time): takes the oldest frame. False when there is none. Never blocks */
    bool PopFrame( CVoiceFrame& frame );

    /** Any thread: frames waiting in the outbound queue (an estimate while the other threads work) */
    UInt32 GetPendingFrameCount( void ) const;

    /** Any thread: the latest statistics the capture thread published */
    void GetStats( CVoiceSenderStats& stats ) const;

    /**
     *  Owner, called by CAudioEngine::AddCaptureConsumer() with the engine locked. Needs mono Float32 at the sample rate
     *  of the codec. Prepares the sender when that was not done and starts a new stream section: the next frame is a
     *  talkspurt start, the sequence numbers and timestamps go on from where they were
     */
    virtual bool PrepareForCapture( const CAudioFormat& processingFormat, UInt32 maxBlockFrames ) GUCEF_VIRTUAL_OVERRIDE;

    /** Capture thread */
    virtual void OnCapturedAudio( const Float32* mono, UInt32 frameCount, const SCaptureBlockInfo& info ) GUCEF_VIRTUAL_OVERRIDE GUCEF_AUDIO_REALTIME_CONTEXT;

    private:

    /** An encoder with what the capture thread needs of it. Two of them, so a codec change can be handed over */
    struct SCodecSlot
    {
        TVoiceEncoderPtr encoderRef;            /**< owner side: keeps the encoder alive */
        CVoiceEncoder* encoder;                 /**< what the capture thread uses */
        CVoiceCodecConfig config;               /**< owner side */
        UInt32 sampleRate;
        UInt32 frameSize;                       /**< samples per frame */
        UInt32 timestampRate;
        UInt32 timestampUnitsPerFrame;
        UInt32 hangoverFrames;                  /**< the VAD hangover in frames */
        UInt8 payloadType;
        GUCEF::vector< Float32 > accumulator;   /**< one frame of audio */

        SCodecSlot( void );
    };

    typedef MT::CTSlotDataExchange< CVoiceFrame, MT::SSlotDeliveryAllInOrder, MT::SSlotFullFail > TFrameQueue;
    typedef MT::CTSlotDataExchange< CVoiceSenderStats, MT::SSlotDeliveryLatestOnly, MT::SSlotFullFail > TStatsExchange;

    CVoiceSender( const CVoiceSender& src );                /**< not implemented */
    CVoiceSender& operator=( const CVoiceSender& src );     /**< not implemented */

    bool PrepareSlot( SCodecSlot& slot, const CVoiceCodecConfig& codecConfig );
    void ResetCaptureState( void );
    void ApplyPendingChanges( void ) GUCEF_AUDIO_REALTIME_CONTEXT;
    UInt32 TimestampAt( UInt64 position ) const GUCEF_AUDIO_REALTIME_CONTEXT;
    void CompleteFrame( bool voiceActive ) GUCEF_AUDIO_REALTIME_CONTEXT;
    void SendFrame( CVoiceFrame& frame, bool preRoll ) GUCEF_AUDIO_REALTIME_CONTEXT;
    void ClearPreRoll( void ) GUCEF_AUDIO_REALTIME_CONTEXT;
    void FrameNotSent( void ) GUCEF_AUDIO_REALTIME_CONTEXT;

    private:

    CVoiceSenderConfig m_config;
    UInt32 m_streamId;
    UInt16 m_firstSequenceNumber;
    UInt32 m_firstTimestamp;
    UInt32 m_queueLimit;
    UInt32 m_preRollCapacity;
    UInt32 m_redundancy;

    // owner side
    bool m_prepared;
    UInt32 m_ownerSlot;                             /**< the slot the capture thread uses, or will once it took the pending change */
    UInt32 m_processingRate;                        /**< of PrepareForCapture(), 0 before */

    // any thread
    MT::CTAtomic< bool > m_ready;                   /**< prepared for capture: the capture thread may work */
    MT::CTAtomic< Int32 > m_pendingSlot;            /**< a slot the capture thread is to switch to, -1 for none */
    MT::CTAtomic< UInt32 > m_transmitMode;
    MT::CTAtomic< bool > m_talk;
    MT::CTAtomic< bool > m_releaseLatch;            /**< the key was let go: the frame in progress still goes */
    MT::CTAtomic< bool > m_muted;
    MT::CTAtomic< bool > m_transmitting;
    MT::CTAtomic< UInt32 > m_requestedBitrate;
    MT::CTAtomic< UInt32 > m_bitrateGeneration;
    MT::CTAtomic< UInt32 > m_requestedLoss;
    MT::CTAtomic< UInt32 > m_lossGeneration;

    SCodecSlot m_slots[ 2 ];

    // capture thread
    UInt32 m_activeSlot;
    UInt32 m_appliedBitrateGeneration;
    UInt32 m_appliedLossGeneration;
    UInt32 m_fill;                                  /**< samples in the frame being accumulated */
    bool m_hasPosition;                             /**< a block arrived since PrepareForCapture() */
    UInt64 m_nextPosition;                          /**< the position the next block should start at */
    UInt64 m_frameStartPosition;                    /**< position of the first sample of the frame being accumulated */
    UInt64 m_frameTickCount;                        /**< tick count of the block it started in */
    bool m_hasTimeline;                             /**< the timestamp mapping was set once */
    UInt64 m_positionBase;                          /**< the position that has the timestamp m_timestampBase */
    UInt32 m_timestampBase;
    UInt32 m_lastFrameTimestamp;                    /**< of the last completed frame */
    UInt16 m_nextSequenceNumber;
    bool m_previousFrameSent;                       /**< the frame before the current one was sent: no marker, redundancy possible */
    UInt32 m_hangoverLeft;
    CVoiceFrame m_work;                             /**< the frame being encoded */
    UInt8 m_previousPayload[ CVoiceFrame::MaxPayloadSize ];
    UInt32 m_previousPayloadSize;
    UInt8 m_previousPayloadType;
    GUCEF::vector< CVoiceFrame > m_preRoll;         /**< ring of encoded frames not sent (voice activated mode) */
    UInt32 m_preRollHead;
    UInt32 m_preRollCount;
    Float64 m_encodeTimeSum;
    CVoiceSenderStats m_stats;

    TFrameQueue m_queue;
    mutable TStatsExchange m_statsExchange;         /**< mutable: reading it moves the reader's slot */
    mutable MT::CMutex m_statsReadLock;             /**< readers of the statistics, never the capture thread */
    mutable CVoiceSenderStats m_statsRead;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CVOICESENDER_H ? */
