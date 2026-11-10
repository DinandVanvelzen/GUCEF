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

#ifndef GUCEF_AUDIO_CAUDIOVOICE_H
#define GUCEF_AUDIO_CAUDIOVOICE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_AUDIO_CAUDIOBUS_H
#include "gucefAUDIO_CAudioBus.h"
#define GUCEF_AUDIO_CAUDIOBUS_H
#endif /* GUCEF_AUDIO_CAUDIOBUS_H ? */

#ifndef GUCEF_AUDIO_IAUDIOSOURCE_H
#include "gucefAUDIO_IAudioSource.h"
#define GUCEF_AUDIO_IAUDIOSOURCE_H
#endif /* GUCEF_AUDIO_IAUDIOSOURCE_H ? */

#ifndef GUCEF_AUDIO_CLINEARRESAMPLER_H
#include "gucefAUDIO_CLinearResampler.h"
#define GUCEF_AUDIO_CLINEARRESAMPLER_H
#endif /* GUCEF_AUDIO_CLINEARRESAMPLER_H ? */

#ifndef GUCEF_AUDIO_CAUDIOEFFECTCHAIN_H
#include "gucefAUDIO_CAudioEffectChain.h"
#define GUCEF_AUDIO_CAUDIOEFFECTCHAIN_H
#endif /* GUCEF_AUDIO_CAUDIOEFFECTCHAIN_H ? */

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

class CAudioMixer;

/*-------------------------------------------------------------------------*/

/** Where a voice is in its life */
enum EAudioVoiceState
{
    AUDIOVOICESTATE_IDLE = 0 ,      /**< made but not playing yet */
    AUDIOVOICESTATE_PLAYING  ,
    AUDIOVOICESTATE_PAUSED   ,
    AUDIOVOICESTATE_ENDED           /**< finished, stopped or lost: it will not play again */
};
typedef enum EAudioVoiceState TAudioVoiceState;

/** Why a voice ended */
enum EAudioVoiceEndReason
{
    AUDIOVOICEEND_NONE = 0 ,
    AUDIOVOICEEND_FINISHED ,        /**< the sound played to its end (all repeats included) */
    AUDIOVOICEEND_STOPPED  ,        /**< somebody stopped it */
    AUDIOVOICEEND_FAILED            /**< it could not be played (the source gave nothing at all, or does not fit the mixer) */
};
typedef enum EAudioVoiceEndReason TAudioVoiceEndReason;

/** How a sound is placed between the left and the right speaker */
enum EAudioPanLaw
{
    AUDIOPANLAW_CONSTANT_POWER = 0 ,    /**< a mono sound keeps its loudness as it moves, centred it is the same as without panning. The usual choice */
    AUDIOPANLAW_BALANCE                 /**< the side you move away from is turned down, the other side is left alone */
};
typedef enum EAudioPanLaw TAudioPanLaw;

/*-------------------------------------------------------------------------*/

/**
 *  One sound that is playing (or about to): a source, the bus it plays into and its settings.
 *  The settings can be changed from any thread at any time and are heard smoothly. What is going on inside
 *  (reading, converting, resampling) belongs to the mixer on the audio thread.
 *
 *  What happens to the audio of a voice, in this order, once per block on the audio thread:
 *   1. the source is read and converted to float, and remixed from the channels of the source to the working channels of
 *      the voice (the speakers of the mix)
 *   2. it is resampled to the mix rate, at the playback rate times the pitch
 *   3. the voice's own effects process it, in the working channels (AddEffect())
 *   4. the output stage: volume and pan, ramped over the block, take it to the speakers of the mix
 *   5. it is added to the bus of the voice, and to the aux buses of its sends at their levels (SetSend()), after the
 *      volume and pan of step 4
 *  A voice without effects and sends costs what it did before they existed, and sounds bit for bit the same.
 *
 *  Effects and sends are set up before the voice starts, as its bus is; the engine prepares the effects for the mix
 *  format when the voice is started (CAudioEngine::Start()) and lets go of them on the owner side once the voice has ended
 *  and that has been reported, never on the audio thread. An effect belongs to one chain at a time: one voice or one bus
 *  slot. When the source is over the effects ring out (a delay, a reverb) before the voice ends: for as many frames as
 *  their latency and tail add up to, and for an effect that can sound forever (feedback), until the voice has given
 *  nothing above -100 dBFS for one second. A voice that is stopped ends at once.
 *  The number of effects and sends a voice can have is fixed when it is made (CAudioEngine::CreateVoice() takes the
 *  maxVoiceEffects and maxVoiceSends of the engine parameters).
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioVoice
{
    public:

    typedef CORE::CTSharedPtr< CAudioVoice, MT::CMutex > TAudioVoicePtr;

    /** Effects and sends a voice made with the one argument constructor can have: the defaults of CAudioEngineParams */
    static const UInt32 DefaultMaxEffects = 4;
    static const UInt32 DefaultMaxSends = 2;

    explicit CAudioVoice( const TAudioSourcePtr& source );

    /** A voice that can have up to 'maxEffects' effects and 'maxSends' sends */
    CAudioVoice( const TAudioSourcePtr& source, UInt32 maxEffects, UInt32 maxSends );

    ~CAudioVoice();

    const TAudioSourcePtr& GetSource( void ) const;

    /** The bus the voice plays into, the master bus when none is set. Set before the voice starts */
    void SetBus( CAudioBus* bus );
    CAudioBus* GetBus( void ) const;

    /** Linear volume, 1.0 is unchanged */
    void SetVolume( Float32 volume );
    Float32 GetVolume( void ) const;

    /** Position between the speakers, -1.0 all the way left, 0.0 centre, 1.0 all the way right */
    void SetPan( Float32 pan );
    Float32 GetPan( void ) const;

    /** Speed of playback as a factor: 1.0 normal, 2.0 twice as fast and an octave higher, 0.5 half speed. Limited to 0.05 .. 16 */
    void SetPlaybackRate( Float32 rate );
    Float32 GetPlaybackRate( void ) const;

    /**
     *  Pitch as a factor that multiplies the playback rate: 2.0 an octave higher (and twice as fast), 0.5 an octave lower.
     *  Any thread, any time. A change glides (a one pole of 20 ms on the logarithm of the factor), so a game can follow an
     *  engine's revolutions or bend a sound without steps; the playback rate itself changes at once. Limited to 0.05 .. 16,
     *  and the product of playback rate and pitch to the same range
     */
    void SetPitch( Float32 factor );
    Float32 GetPitch( void ) const;

    /** Most effects and sends the voice can have */
    UInt32 GetMaxEffects( void ) const;
    UInt32 GetMaxSends( void ) const;

    /**
     *  Adds an effect at the end of the voice's chain. Owner thread, before the voice starts (like SetBus()). The engine
     *  prepares it for the mix format at Start() when it is not prepared for it already. False when the voice was started,
     *  the effect is null or in this chain already, or the chain is full (GetMaxEffects())
     */
    bool AddEffect( const TAudioEffectPtr& effect );

    /** Any thread. 0 once the voice has ended and the engine let go of its effects */
    UInt32 GetEffectCount( void ) const;

    /** Any thread. Null when there is no effect at 'index', and once the voice has ended and the engine let go of its effects */
    TAudioEffectPtr GetEffect( UInt32 index ) const;

    /**
     *  Any thread, any time, also before the voice starts: switches the effect at 'index' on or off, crossfaded over 10 ms
     *  as in a bus slot (CAudioEffectChain). On by default
     */
    void SetEffectEnabled( UInt32 index, bool enabled );
    bool IsEffectEnabled( UInt32 index ) const;

    /**
     *  Any thread, any time: wet and dry level of the effect at 'index', linear, ramped over a block. Default 1 and 0, the
     *  effect alone; an effect that gives only what it adds (a delay, a reverb) wants dry 1 on a voice
     */
    void SetEffectMix( UInt32 index, Float32 wet, Float32 dry );
    Float32 GetEffectWet( UInt32 index ) const;
    Float32 GetEffectDry( UInt32 index ) const;

    /**
     *  Sends the voice to an aux bus as well, at a linear level, after its volume and pan: a send slot from 0 to
     *  GetMaxSends() - 1, null for 'bus' frees the slot. Owner thread, before the voice starts (like SetBus()). A send to
     *  the bus the voice plays into adds the voice there a second time. False when the voice was started or 'index' is
     *  beyond the slots
     */
    bool SetSend( UInt32 index, CAudioBus* bus, Float32 level );
    CAudioBus* GetSendBus( UInt32 index ) const;

    /** Linear level of a send, any thread, any time; a change is ramped over the next block */
    void SetSendLevel( UInt32 index, Float32 level );
    Float32 GetSendLevel( UInt32 index ) const;

    /** How many times the sound plays: 1 once (default), 3 three times in a row, 0 until stopped. Needs a source that can seek */
    void SetLoopCount( UInt32 plays );
    UInt32 GetLoopCount( void ) const;

    TAudioVoiceState GetState( void ) const;

    TAudioVoiceEndReason GetEndReason( void ) const;

    /** How far the sound has played, in frames of the source, counting from the start of the current repeat. Updated every piece of audio that is mixed */
    UInt64 GetPositionInFrames( void ) const;

    /** Asks for the voice to stop. The mixer ends it at its next piece of audio */
    void RequestStop( void );

    /** Asks for the voice to pause or to go on again */
    void RequestPause( bool paused );

    bool IsPauseRequested( void ) const;

    bool IsStopRequested( void ) const;

    private:
    friend class CAudioMixer;
    friend class CAudioEngine;

    CAudioVoice( const CAudioVoice& src );              /**< not implemented */
    CAudioVoice& operator=( const CAudioVoice& src );   /**< not implemented */

    /** A send of the voice to an aux bus */
    struct SSend
    {
        CAudioBus* bus;             /**< null for a free slot. Set before the voice starts */
        CAtomicFloat level;
        Float32 lastLevel;          /**< audio thread: level at the end of the previous block */
        bool levelKnown;

        SSend( void );
    };

    /** Sets up what the voice needs to be mixed in the given format, its effects included: owner thread, allocates. False if the source cannot be mixed in it */
    bool Prepare( const CAudioFormat& mixFormat, UInt32 maxBlockFrames );

    /**
     *  Prepares again for another mix format, keeping where the voice is (repeats done, position, audio read from the
     *  source and not yet mixed). Its effects are prepared for the new format. Owner thread, while the voice is not mixed. Allocates
     */
    bool Reprepare( const CAudioFormat& mixFormat, UInt32 maxBlockFrames );

    /**
     *  Owner thread: prepares the effects that are not prepared for the working format yet, places them in the chain and
     *  works out how long they ring out. False when one cannot be prepared
     */
    bool PrepareEffects( UInt32 maxBlockFrames );

    /** The effect at 'index' as a plain pointer, for the engine's bookkeeping. Null beyond the effects */
    CAudioEffect* GetEffectPointer( UInt32 index ) const;

    /** Owner thread: true when every send goes to a bus that is prepared for the mix format, notes whether there are sends */
    bool CheckSends( UInt32 maxBlockFrames );

    /**
     *  Owner side, once the voice will not be mixed again (it has ended and that was reported): empties the chain and drops
     *  the references to the effects, so their last reference is never let go of on the audio thread
     */
    void ReleaseEffects( void );

    /** Mixes the next 'frames' frames of the voice into 'busBuffer' (adds), and into the buses of its sends. Audio thread only, never allocates */
    void RenderBlock( Float32* busBuffer, UInt32 frames, TAudioPanLaw panLaw );

    void End( TAudioVoiceEndReason reason );

    /** Fills m_blockBuffer with 'frames' frames in the working channels, as far as the source gives them. Returns the number of frames made */
    UInt32 GatherBlock( UInt32 frames );

    /** Reads up to 'frames' frames from the source into m_convertedBuffer as floats in the working channels. Returns the number read */
    UInt32 ReadAndConvert( UInt32 frames, bool sameChannels );

    /**
     *  Called when the source gave nothing: starts the next repeat or notes that the sound is over.
     *  Returns true when there is new audio to read, false when the block has to be left as it is.
     */
    bool HandleSourceEnd( void );

    /** Audio thread: the source rate times playback rate times pitch, for the resampler */
    void ApplyResamplerRate( Float64 playbackRate );

    /** Audio thread: one step of the pitch glide (a sub-block), with the new rate for the resampler. False once it has arrived */
    bool StepPitchGlide( Float64 playbackRate );

    /**
     *  Audio thread, after the source has ended: counts the frames of the block in which the effects rang out. True when they
     *  have rung out and the voice can end. Looks at the output of the effects, so it is called before the output stage
     */
    bool IsRingOutOver( UInt32 produced, UInt32 frames );

    /**
     *  The output stage, audio thread: takes the working signal in m_blockBuffer to the speakers of the mix (volume and pan,
     *  ramped over the block) and adds it to the bus buffer and to the buses of the sends. With sends the gains are applied
     *  to m_blockBuffer in place, so that the bus and the sends get the same signal.
     *  A spatial voice (steps 13 to 15 of the design) replaces this stage with its spatializer: the working signal is mono
     *  then, and the spatializer writes the speaker signal into a buffer of the mix layout, which goes to the bus and the sends
     *  in the same way
     */
    void RenderOutput( Float32* busBuffer, UInt32 frames, TAudioPanLaw panLaw );

    /** Audio thread: adds 'block' (mix layout) to the buses of the sends at their levels, a change ramped over the block */
    void DeliverSends( const Float32* block, UInt32 frames, UInt32 channels );

    private:

    TAudioSourcePtr m_source;
    IAudioSource* m_sourceRaw;          /**< the source for the audio thread: reaching it through the shared pointer is not real time safe */
    CAudioBus* m_bus;
    CAtomicFloat m_volume;
    CAtomicFloat m_pan;
    CAtomicFloat m_playbackRate;
    MT::CTAtomic< UInt32 > m_loopCount;
    MT::CTAtomic< UInt32 > m_state;
    MT::CTAtomic< UInt32 > m_endReason;
    MT::CTAtomic< UInt64 > m_position;
    MT::CTAtomic< bool > m_stopRequested;
    MT::CTAtomic< bool > m_pauseRequested;
    CAtomicFloat m_pitch;                           /**< the pitch asked for, the audio thread glides to it */

    // owner side: set up before the voice starts
    mutable MT::CMutex m_effectLock;                /**< guards m_effectRefs: the engine lets go of the effects on its own thread. Never taken on the audio thread */
    GUCEF::vector< TAudioEffectPtr > m_effectRefs;  /**< the voice's effects, in chain order. The references that keep them alive */
    UInt32 m_maxEffects;
    SSend* m_sends;
    UInt32 m_maxSends;

    // mixer side, audio thread only after Prepare()
    CAudioFormat m_sourceFormat;
    CAudioFormat m_mixFormat;
    CAudioFormat m_workFormat;                      /**< the channels the voice works in up to its output stage: those of the mix (a spatial voice will work in mono) */
    GUCEF::vector< Float32 > m_matrix;              /**< from the source channels to the working channels */
    GUCEF::vector< UInt8 > m_rawBuffer;
    GUCEF::vector< Float32 > m_rawFloatBuffer;
    GUCEF::vector< Float32 > m_convertedBuffer;     /**< source frames converted to float and remixed to the working channels */
    GUCEF::vector< Float32 > m_blockBuffer;         /**< the block that is mixed, in the working channels */
    CAudioEffectChain m_effects;                    /**< a slot per effect the voice can have, in the working channels. The slots exist from the start, so their settings can be set from any thread */
    bool m_hasEffects;
    bool m_hasSends;
    UInt32 m_tailFrames;                            /**< frames the effects ring out after the source ended, GUCEF_AUDIO_TAIL_INFINITE until they are silent */
    UInt32 m_tailLeft;
    UInt32 m_silentFrames;                          /**< frames in a row the effects gave nothing audible, while an unbounded tail rings out */
    UInt32 m_silenceToEnd;                          /**< silent frames after which an unbounded tail is over: one second */
    CLinearResampler m_resampler;
    bool m_resampling;                              /**< the resampler is in use. Once it is, it stays: it holds read-ahead source audio and a position between frames */
    Float32 m_pitchTarget;                          /**< the pitch the glide goes to */
    Float64 m_pitchTargetLog2;
    Float64 m_pitchLog2;                            /**< the pitch now, log2 */
    Float64 m_pitchNow;                             /**< the pitch now, as a factor: exactly the target once it arrived */
    Float64 m_pitchCoefficient;                     /**< one pole coefficient of the glide per sub-block */
    UInt32 m_pitchFramesLeft;                       /**< frames left in the sub-block of the glide */
    UInt32 m_maxBlock;
    UInt32 m_pendingInput;                          /**< frames waiting in m_convertedBuffer for the resampler */
    UInt32 m_playsDone;
    UInt64 m_framesThisPlay;                        /**< frames read from the source since this repeat started */
    Float64 m_playedPosition;                       /**< where in the source the sound is, in source frames, the part that has actually been mixed */
    GUCEF::vector< Float32 > m_lastGains;           /**< gain per mix channel at the end of the previous block, for smooth changes */
    bool m_gainsKnown;
    bool m_prepared;
    bool m_sourceEnded;
    Float64 m_resamplerRatio;
};

typedef CAudioVoice::TAudioVoicePtr TAudioVoicePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOVOICE_H ? */
