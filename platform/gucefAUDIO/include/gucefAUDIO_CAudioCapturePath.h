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

#ifndef GUCEF_AUDIO_CAUDIOCAPTUREPATH_H
#define GUCEF_AUDIO_CAUDIOCAPTUREPATH_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTSPSCQUEUE_H
#include "gucefMT_CTSpscQueue.h"
#define GUCEF_MT_CTSPSCQUEUE_H
#endif /* GUCEF_MT_CTSPSCQUEUE_H ? */

#ifndef GUCEF_AUDIO_CAUDIOEFFECTCHAIN_H
#include "gucefAUDIO_CAudioEffectChain.h"
#define GUCEF_AUDIO_CAUDIOEFFECTCHAIN_H
#endif /* GUCEF_AUDIO_CAUDIOEFFECTCHAIN_H ? */

#ifndef GUCEF_AUDIO_CAUDIOLEVELMETER_H
#include "gucefAUDIO_CAudioLevelMeter.h"
#define GUCEF_AUDIO_CAUDIOLEVELMETER_H
#endif /* GUCEF_AUDIO_CAUDIOLEVELMETER_H ? */

#ifndef GUCEF_AUDIO_CSINCRESAMPLER_H
#include "gucefAUDIO_CSincResampler.h"
#define GUCEF_AUDIO_CSINCRESAMPLER_H
#endif /* GUCEF_AUDIO_CSINCRESAMPLER_H ? */

#ifndef GUCEF_AUDIO_IAUDIOCAPTURECONSUMER_H
#include "gucefAUDIO_IAudioCaptureConsumer.h"
#define GUCEF_AUDIO_IAUDIOCAPTURECONSUMER_H
#endif /* GUCEF_AUDIO_IAUDIOCAPTURECONSUMER_H ? */

#ifndef GUCEF_AUDIO_CVOICEACTIVITYDETECTOR_H
#include "gucefAUDIO_CVoiceActivityDetector.h"
#define GUCEF_AUDIO_CVOICEACTIVITYDETECTOR_H
#endif /* GUCEF_AUDIO_CVOICEACTIVITYDETECTOR_H ? */

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

class CHalfDuplexController;
class CAcousticEchoCanceller;

/*-------------------------------------------------------------------------*/

/**
 *  The processing of captured audio: what the capture device records goes through these steps, period by period, on the
 *  capture thread:
 *   - conversion to float (CAudioConverter) and to mono: one channel of the device, or the mean of all channels
 *   - resampling to the processing rate with a CSincResampler of fixed ratio, when that rate differs from the device rate
 *   - the effect stage AUDIOCAPTURESTAGE_PRE_ECHO, the echo stage (a CAcousticEchoCanceller when one is set, which reads
 *     what the speaker plays from its render tap), the effect stage AUDIOCAPTURESTAGE_POST_ECHO: the effect stages are
 *     each a CAudioEffectChain in mono at the processing rate
 *   - the half-duplex controller, when one is set (CHalfDuplexController): it looks at the audio and asks for a gain;
 *     it is told every block whether the echo canceller converged and its echo return loss enhancement (the hand-over)
 *   - the microphone gain: mute and the gain of the half-duplex controller, ramped linearly (10 ms from full to off)
 *     frame by frame so the ramp does not depend on the block size
 *   - the voice activity detection (CVoiceActivityDetector, which looks at the processed audio and does not change it); its
 *     analysis frames are aligned to the absolute frame position of the blocks, so the decision does not depend on the block size
 *   - the level meter, then every consumer (IAudioCaptureConsumer) with the block and its SCaptureBlockInfo
 *  Blocks are at most the maximum block size of the path; a longer device period is processed in several blocks.
 *
 *  Like the mixer it follows the rules of the real time thread: the capture thread never allocates, locks, logs or touches
 *  a reference count. The structure (effects in slots, consumers) changes through commands that carry plain pointers, which
 *  the capture thread carries out at the start of a device period (ProcessCommands()); for every command it posts one event
 *  back. What it let go of (an effect, a consumer) is reported by an event and released by the owner. The queues are its own:
 *  the queues of the mixer have the render thread as their consumer. The owner keeps the number of commands on their way
 *  below the queue capacity, so no event is ever lost.
 *
 *  The mute switch, the slot switches, the meter, the voice activity decision and the overrun count can be used from any
 *  thread at any time once the path is prepared. Prepare() and Release() are for the owner thread while nothing processes.
 *  When the capture device does not run (it is stopped, or lost) the owner may carry out the commands itself with
 *  ProcessCommands(): it is then the only thread that uses the path.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioCapturePath
{
    public:

    /** A change of the structure, for the capture thread. Plain pointers only: copying a command never locks or allocates */
    struct SCommand
    {
        enum EType { COMMAND_NONE = 0, COMMAND_INSERT_EFFECT, COMMAND_REMOVE_EFFECT, COMMAND_ADD_CONSUMER, COMMAND_REMOVE_CONSUMER, COMMAND_SET_HALF_DUPLEX, COMMAND_SET_ECHO_CANCELLER };
        EType type;
        TAudioCaptureStage stage;
        UInt32 slot;
        CAudioEffect* effect;
        IAudioCaptureConsumer* consumer;
        CHalfDuplexController* halfDuplex;  /**< COMMAND_SET_HALF_DUPLEX: the controller from now on, null for none */
        CAcousticEchoCanceller* echoCanceller;  /**< COMMAND_SET_ECHO_CANCELLER: the canceller from now on, null for none */
        UInt32 token;               /**< the request on the owner side that waits for this command */
        SCommand( void ) : type( COMMAND_NONE ), stage( AUDIOCAPTURESTAGE_PRE_ECHO ), slot( 0 ), effect( GUCEF_NULL ), consumer( GUCEF_NULL ), halfDuplex( GUCEF_NULL ), echoCanceller( GUCEF_NULL ), token( 0 ) {}
    };

    /** What the capture thread reports back: one event per command */
    struct SEvent
    {
        enum EType
        {
            EVENT_NONE = 0          ,
            EVENT_COMMAND_DONE      ,   /**< an effect is in its slot, a consumer is in the table, a half-duplex controller or echo canceller is used (the one before it, in 'halfDuplex' or 'echoCanceller', is not any more) */
            EVENT_COMMAND_FAILED    ,   /**< the slot was taken or the table full: the effect or consumer is not used */
            EVENT_EFFECT_RELEASED   ,   /**< the effect is out of its slot and will not be used again */
            EVENT_CONSUMER_RELEASED     /**< the consumer is out of the table and will not be called again */
        };
        EType type;
        CAudioEffect* effect;
        IAudioCaptureConsumer* consumer;
        CHalfDuplexController* halfDuplex;  /**< the half-duplex controller let go of (COMMAND_SET_HALF_DUPLEX) */
        CAcousticEchoCanceller* echoCanceller;  /**< the echo canceller let go of (COMMAND_SET_ECHO_CANCELLER) */
        UInt32 token;
        SEvent( void ) : type( EVENT_NONE ), effect( GUCEF_NULL ), consumer( GUCEF_NULL ), halfDuplex( GUCEF_NULL ), echoCanceller( GUCEF_NULL ), token( 0 ) {}
    };

    /** The time over which the microphone gain moves from on to off (mute) and back */
    static const Float64 MuteRampInSeconds;

    CAudioCapturePath( void );

    ~CAudioCapturePath();

    /**
     *  Owner thread, while nothing processes: makes everything the capture thread will use. The path takes audio in
     *  'deviceFormat' (any sample type and channel count) and processes it in mono Float32 at 'processingRate' (0: the rate
     *  of the device), taking channel 'channel' of the device or the mean of all channels when it is negative. Effects
     *  are expected prepared for GetProcessingFormat() and 'maxBlockFrames'. A path that was prepared is released first.
     *  False, with the path released, for an invalid format, a channel the device does not have, a rate the resampler
     *  cannot reach, or a capacity of 0 where one is needed
     */
    bool Prepare( const CAudioFormat& deviceFormat     ,
                  UInt32 processingRate                ,
                  Int32 channel                        ,
                  UInt32 maxBlockFrames                ,
                  UInt32 maxConsumers                  ,
                  UInt32 maxEffectsPerStage            ,
                  UInt32 queueSize                     );

    /** Owner thread, while nothing processes: forgets the effects and consumers (it owns neither) and frees the memory */
    void Release( void );

    bool IsPrepared( void ) const;

    /** The format the device records in, as given to Prepare() */
    const CAudioFormat& GetDeviceFormat( void ) const;

    /** Mono Float32 at the processing rate: the format of the effects and of what the consumers get */
    const CAudioFormat& GetProcessingFormat( void ) const;

    UInt32 GetMaxBlockFrames( void ) const;
    UInt32 GetMaxConsumers( void ) const;
    UInt32 GetMaxEffectsPerStage( void ) const;

    /** Room in the command queue and in the event queue (the same): the most commands that may be on their way at once */
    UInt32 GetQueueCapacity( void ) const;

    /** Owner thread: hands a command to the capture thread. False when the queue is full */
    bool PushCommand( const SCommand& command );

    /** Owner thread: the next event of the capture thread. False when there is none */
    bool PopEvent( SEvent& event );

    /** Any thread: mutes or unmutes the microphone, ramped over MuteRampInSeconds */
    void SetMuted( bool muted );
    bool IsMuted( void ) const;

    /** Any thread: switches an effect slot of a stage on or off, crossfaded as on a bus */
    void SetEffectEnabled( TAudioCaptureStage stage, UInt32 slot, bool enabled );
    bool IsEffectEnabled( TAudioCaptureStage stage, UInt32 slot ) const;

    /** Any thread: the levels of the processed audio (one channel), valid as long as the path exists */
    const CAudioLevelMeter& GetMeter( void ) const;

    /** Any thread: the latest decision of the voice activity detection: that of the last analysis frame completed (up to 10 ms behind the audio) */
    bool IsVoiceActive( void ) const;

    /**
     *  The settings of the voice activity detection. Set them while nothing processes (before the device starts or while
     *  it is stopped); they stay when the path is prepared again. The defaults are those of CVoiceActivitySettings
     */
    void SetVoiceActivitySettings( const CVoiceActivitySettings& settings );
    const CVoiceActivitySettings& GetVoiceActivitySettings( void ) const;

    /** Any thread: the energy of the voice band of the last analysis frame and the noise floor, in dB (see CVoiceActivityDetector) */
    Float32 GetVoiceEnergyInDecibels( void ) const;
    Float32 GetVoiceNoiseFloorInDecibels( void ) const;

    /** Any thread: overruns of the device seen by the path (each one marked as a discontinuity) */
    UInt64 GetOverrunCount( void ) const;

    /** Any thread: frames delivered to the consumers so far at the processing rate, the frames lost to overruns included */
    UInt64 GetFramePosition( void ) const;

    /**
     *  Capture thread (or the owner while the device does not run): carries out the commands that are waiting and posts an
     *  event for each
     */
    void ProcessCommands( void );

    /**
     *  Capture thread: processes 'frameCount' frames in the device format. 'deviceOverrunCount' is the overrun count of the
     *  device as it is now (0 for a device that does not count), a rise marks the next block as a discontinuity.
     *  'tickCount' is MT::PrecisionTickCount() at the start of the device callback
     */
    void Process( const void* frames, UInt32 frameCount, UInt64 deviceOverrunCount, UInt64 tickCount );

    private:

    CAudioCapturePath( const CAudioCapturePath& src );              /**< not implemented */
    CAudioCapturePath& operator=( const CAudioCapturePath& src );   /**< not implemented */

    CAudioEffectChain& Chain( TAudioCaptureStage stage );
    const CAudioEffectChain* FindChain( TAudioCaptureStage stage ) const;
    void PostEvent( SEvent::EType type, const SCommand& command );
    void ToMono( const Float32* interleaved, UInt32 frames, Float32* mono ) const;
    void ProcessBlock( Float32* mono, UInt32 frames, UInt64 tickCount );
    void ApplyMicrophoneGain( Float32* mono, UInt32 frames, Float32 halfDuplexGain );
    bool DetectVoiceActivity( const Float32* mono, UInt32 frames, UInt64 firstFrame );

    private:

    bool m_prepared;
    CAudioFormat m_deviceFormat;
    CAudioFormat m_processingFormat;
    Int32 m_channel;
    UInt32 m_maxBlockFrames;
    UInt32 m_maxEffectsPerStage;
    bool m_resampling;
    Float64 m_ticksPerSecond;
    Float64 m_processingFramesPerDeviceFrame;

    MT::CTSpscQueue< SCommand >* m_commands;
    MT::CTSpscQueue< SEvent >* m_events;

    CAudioEffectChain m_preEcho;
    CAudioEffectChain m_postEcho;
    CAudioLevelMeter m_meter;
    CSincResampler m_resampler;
    CVoiceActivityDetector m_voiceDetector;

    MT::CTAtomic< bool > m_muted;
    MT::CTAtomic< bool > m_voiceActive;
    MT::CTAtomic< UInt64 > m_overruns;
    MT::CTAtomic< UInt64 > m_framePosition;

    // capture thread
    GUCEF::vector< IAudioCaptureConsumer* > m_consumers;
    CHalfDuplexController* m_halfDuplex;
    CAcousticEchoCanceller* m_echoCanceller;
    GUCEF::vector< Float32 > m_deviceFloat;     /**< a block of the device audio as float, when the device does not record float */
    GUCEF::vector< Float32 > m_mono;            /**< a block in mono at the device rate */
    GUCEF::vector< Float32 > m_resampled;       /**< a block in mono at the processing rate, when resampling */
    Float32 m_gain;                             /**< microphone gain applied to the last frame */
    Float32 m_gainStep;                         /**< gain change per frame while it ramps */
    UInt64 m_nextFrame;                         /**< absolute position of the next frame at the processing rate */
    UInt64 m_seenOverruns;                      /**< the overrun count of the device at the last callback */
    UInt64 m_lastTickCount;                     /**< tick count of the last callback, 0 before the first */
    bool m_discontinuityPending;                /**< the next block delivered follows an overrun */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOCAPTUREPATH_H ? */
