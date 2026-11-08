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

#ifndef GUCEF_AUDIO_CVOICERECEIVER_H
#define GUCEF_AUDIO_CVOICERECEIVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_AUDIO_CAUDIOVOICE_H
#include "gucefAUDIO_CAudioVoice.h"
#define GUCEF_AUDIO_CAUDIOVOICE_H
#endif /* GUCEF_AUDIO_CAUDIOVOICE_H ? */

#ifndef GUCEF_AUDIO_CVOICEPLAYOUTSOURCE_H
#include "gucefAUDIO_CVoicePlayoutSource.h"
#define GUCEF_AUDIO_CVOICEPLAYOUTSOURCE_H
#endif /* GUCEF_AUDIO_CVOICEPLAYOUTSOURCE_H ? */

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

class CAudioEngine;
class CAudioBus;

/*-------------------------------------------------------------------------*/

/**
 *  The receiving side of one voice stream: takes the frames the network side read from its packets (PushFrame()) and
 *  plays them through a voice of the engine on the bus the application chooses. Several incoming streams are several
 *  receivers, each with its own voice; the mixer of the engine adds them up.
 *
 *  It owns the decoder and a CVoicePlayoutSource (inbound queue, jitter buffer, playout controller, drift resampler),
 *  which the voice reads on the audio thread.
 *
 *  Frames of another stream id than the accepted one (the configured one, or the first that arrived since Prepare()
 *  or Reset()) and of another payload type than the codec's are counted and dropped: the network layer demultiplexes
 *  by stream id. Frames read from RFC 2198 packets carry the payload type of their primary block, so the RED payload
 *  type never reaches a receiver.
 *
 *  Threads:
 *   - owner: construction, SetClock(), Prepare(), Start(), Stop(), destruction. The engine must outlive the receiver;
 *     the playout source lives on with the voice until the engine let go of it
 *   - one network thread at a time: PushFrame(), with arrival times in the receiver's clock (CVoiceClock: by default
 *     MT::PrecisionTickCount() in microseconds, CVoiceClock::GetSystemTimeInUs())
 *   - audio thread: the playout source, through the voice
 *   - any thread: Reset(), SetVolume(), IsPlaying(), GetLevelInDecibels(), GetStats()
 */
class GUCEF_AUDIO_PUBLIC_CPP CVoiceReceiver
{
    public:

    CVoiceReceiver( CAudioEngine& engine, const CVoiceReceiverConfig& config );

    /** Stops the voice when it plays (without waiting for it) */
    ~CVoiceReceiver();

    const CVoiceReceiverConfig& GetConfig( void ) const;

    /** Owner, before Prepare(): the clock of the arrival times and of the playout (the system clock by default). It must outlive the receiver */
    void SetClock( const CVoiceClock& clock );
    const CVoiceClock& GetClock( void ) const;

    /** Owner, while the receiver does not play: makes the decoder and prepares the playout source. False (logged) when the codec cannot be made */
    bool Prepare( void );

    bool IsPrepared( void ) const;

    /**
     *  Owner: makes a voice for the playout source on 'bus' (the master bus when null) and starts it; the playout starts
     *  afresh (frames pushed before are dropped). The future is that of CAudioEngine::Start(). Failed at once
     *  (TASKSTATUS_SETUP_FAILED) when the receiver is not prepared or its previous voice has not ended yet
     */
    CORE::CFutureResult Start( CAudioBus* bus = GUCEF_NULL );

    /** Owner: stops the voice; the future is ready when it has ended (at once when there is none) */
    CORE::CFutureResult Stop( void );

    /** Owner: the voice of the last Start(), null before */
    TAudioVoicePtr GetVoice( void ) const;

    /**
     *  Network thread (one at a time): a frame and its arrival time. Never blocks. False when it is dropped: not
     *  prepared, another stream, another payload type, or the inbound queue is full (all counted)
     */
    bool PushFrame( const CVoiceFrame& frame, UInt64 arrivalTimeInMicroseconds );

    /** Any thread: forgets the stream (the accepted stream id too); the audio thread resets the playout at its next request */
    void Reset( void );

    /** Any thread: the volume of the voice, linear (1.0 unchanged); kept for the voices of later Start() calls */
    void SetVolume( Float32 volume );
    Float32 GetVolume( void ) const;

    /** Any thread: frames are being played (playout state PLAYING) */
    bool IsPlaying( void ) const;

    /** Any thread: RMS of the output (300 ms) in dBFS, for a "who is talking" indicator */
    Float32 GetLevelInDecibels( void ) const;

    /** Any thread: the statistics; the fraction lost is over the interval since the previous call */
    void GetStats( CVoiceReceiverStats& stats ) const;

    /** The playout source (owner side, for tests and other mixers): valid as long as the receiver lives */
    CVoicePlayoutSource& GetPlayoutSource( void );
    const TAudioSourcePtr& GetSource( void ) const;

    private:

    CVoiceReceiver( const CVoiceReceiver& src );                /**< not implemented */
    CVoiceReceiver& operator=( const CVoiceReceiver& src );     /**< not implemented */

    private:

    CAudioEngine* m_engine;
    CVoiceReceiverConfig m_config;
    const CVoiceClock* m_clock;
    CVoicePlayoutSource* m_playout;             /**< the object of m_source */
    TAudioSourcePtr m_source;
    TVoiceDecoderPtr m_decoder;
    bool m_prepared;

    mutable MT::CMutex m_lock;                  /**< the voice and the volume, owner and control threads, never the audio thread */
    TAudioVoicePtr m_voice;
    Float32 m_volume;

    // network thread
    MT::CTAtomic< bool > m_ready;
    UInt32 m_acceptedStreamId;
    UInt32 m_seenResetGeneration;

    // any thread
    MT::CTAtomic< UInt32 > m_resetGeneration;
    MT::CTAtomic< UInt32 > m_currentStreamId;
    MT::CTAtomic< UInt64 > m_framesPushed;
    MT::CTAtomic< UInt64 > m_wrongStream;
    MT::CTAtomic< UInt64 > m_wrongPayloadType;

    // the interval of the fraction lost (readers of the statistics)
    mutable MT::CMutex m_statsLock;
    mutable UInt64 m_priorExpected;
    mutable UInt64 m_priorReceived;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CVOICERECEIVER_H ? */
