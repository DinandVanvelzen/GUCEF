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

#ifndef GUCEF_AUDIO_CAUDIOMIXER_H
#define GUCEF_AUDIO_CAUDIOMIXER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_CAUDIOVOICE_H
#include "gucefAUDIO_CAudioVoice.h"
#define GUCEF_AUDIO_CAUDIOVOICE_H
#endif /* GUCEF_AUDIO_CAUDIOVOICE_H ? */

#ifndef GUCEF_AUDIO_CLIMITEREFFECT_H
#include "gucefAUDIO_CLimiterEffect.h"
#define GUCEF_AUDIO_CLIMITEREFFECT_H
#endif /* GUCEF_AUDIO_CLIMITEREFFECT_H ? */

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

/** Told by the mixer when a voice has ended. Called on the thread that mixes, so the implementation has to be quick and must not wait */
class GUCEF_AUDIO_PUBLIC_CPP IAudioMixerListener
{
    public:

    virtual ~IAudioMixerListener() {}

    virtual void OnVoiceEnded( CAudioVoice* voice, TAudioVoiceEndReason reason ) = 0;
};

/*-------------------------------------------------------------------------*/

/**
 *  Adds up all playing voices through the bus tree into one stream of float audio. This is the engine's
 *  inner loop: no locks, no allocation and no IO in Mix(), only the arithmetic. Everything that needs memory is
 *  set up beforehand (voices are prepared when they are added, buses when they are attached, and room for the
 *  maximum number of voices and buses is reserved), so Mix() is safe to call from a real time audio thread.
 *
 *  For the same voices and settings Mix() always makes the same audio, whatever size the pieces are that it is
 *  asked for (up to rounding in the last bit), as long as no setting changes in between: a change in volume or
 *  pan is spread over the piece of audio that is mixed next, so how the audio is cut up decides the shape of
 *  the fade. The tests rely on this.
 *
 *  Mixing is single threaded: AddVoice(), AttachBus(), the structural calls (SetBusOrder(), SetBusEffect(), SetBusSend())
 *  and Mix() are called by one thread at a time. The engine puts the thread rules around it. The voices, buses and
 *  effects are owned by the caller and must outlive their time in the mixer.
 *
 *  Buses are processed in an order in which every bus comes after all buses that feed it (children and buses that send to
 *  it): the order given to SetBusOrder(), computed off the audio thread by CAudioBusRouting. Without one, the buses are
 *  processed in reverse order of attachment, which suits a plain tree; that is also the order CAudioBusRouting computes
 *  for a tree, so both give the same audio.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioMixer
{
    public:

    /**
     *  @param mixFormat        what Mix() produces: float32 and the speaker layout of the output. The rate is the rate of the output
     *  @param maxBlockFrames   largest piece mixed in one go (larger requests are mixed in pieces)
     *  @param maxVoices        number of voices that can play at the same time
     *  @param maxBuses         number of buses, the master bus included
     *  @param maxEffectsPerBus insert effect slots per bus
     *  @param maxSendsPerBus   send slots per bus
     */
    CAudioMixer( const CAudioFormat& mixFormat       ,
                 UInt32 maxBlockFrames = 4096        ,
                 UInt32 maxVoices = 256              ,
                 UInt32 maxBuses = 64                ,
                 UInt32 maxEffectsPerBus = 8         ,
                 UInt32 maxSendsPerBus = 4           );

    ~CAudioMixer();

    const CAudioFormat& GetMixFormat( void ) const;

    UInt32 GetMaxBlockFrames( void ) const;

    UInt32 GetMaxBuses( void ) const;

    UInt32 GetMaxEffectsPerBus( void ) const;

    UInt32 GetMaxSendsPerBus( void ) const;

    /**
     *  Changes the speakers and rate of the mix. Owner thread, only while Mix() is not running (the device is stopped).
     *  Every attached bus is prepared again (buffers, effect slots, meter) and keeps its effects, sends and settings;
     *  voices that are being mixed are prepared again and keep their position, and prepare their own effects for the new
     *  format. The effects in bus slots are not prepared here: their owner prepares them for the new format (CAudioEngine
     *  does). Allocates.
     */
    bool SetMixFormat( const CAudioFormat& mixFormat );

    /** The top of the bus tree, always there */
    CAudioBus* GetMasterBus( void );

    /** Makes a bus below 'parent' (the master bus when null). Owner thread, not while Mix() runs. Null when the bus limit is reached */
    CAudioBus* CreateBus( const CString& name, CAudioBus* parent = GUCEF_NULL );

    /** Allocates what a bus needs to be mixed. Done on the owner thread, before AttachBus() */
    bool PrepareBus( CAudioBus* bus );

    /**
     *  Adds a prepared bus to the tree below its parent, which has to be in the tree already. Allocates nothing and
     *  touches no reference count, so it is safe on the audio thread. The caller keeps the bus alive. False when the
     *  bus is not prepared, the parent is unknown or the limit is reached.
     */
    bool AttachBus( CAudioBus* bus );

    CAudioBus* FindBus( const CString& name );

    UInt32 GetBusCount( void ) const;

    /**
     *  Mixing thread: the order to process the buses in, feeders first and the master bus last, as computed by
     *  CAudioBusRouting. It must list every attached bus once; the table is not copied and must stay unchanged while it
     *  is in use. Null goes back to reverse order of attachment
     */
    void SetBusOrder( CAudioBus* const* order, UInt32 count );

    /** Mixing thread: puts an effect in an insert slot of a bus (null empties it). The effect must be prepared for the mix format */
    bool SetBusEffect( CAudioBus* bus, UInt32 slot, CAudioEffect* effect );

    /** Mixing thread: the effect in an insert slot of a bus */
    CAudioEffect* GetBusEffect( CAudioBus* bus, UInt32 slot ) const;

    /**
     *  Mixing thread: points a send slot of a bus at a target bus (null frees the slot), taken before or after the fader.
     *  The bus order must have the target after the bus; set it in the same go
     */
    bool SetBusSend( CAudioBus* bus, UInt32 sendIndex, CAudioBus* target, bool preFader );

    /**
     *  Prepares a voice for this mixer, its effects included (CAudioVoice::AddEffect()). Done on the owner thread, before
     *  AddVoice(). False when the source cannot be mixed in the mix format, an effect cannot be prepared, or a send goes to
     *  a bus that is not prepared for this mixer
     */
    bool PrepareVoice( CAudioVoice* voice );

    /** Prepares a voice again for the current mix format, keeping its position. Owner thread, while Mix() is not running */
    bool ReprepareVoice( CAudioVoice* voice );

    /** Starts mixing a prepared voice. False when the maximum number of voices play, or the voice was not prepared */
    bool AddVoice( CAudioVoice* voice );

    UInt32 GetVoiceCount( void ) const;

    void SetListener( IAudioMixerListener* listener );

    void SetPanLaw( TAudioPanLaw panLaw );

    /**
     *  The master limiter stage: the last stage of the master bus, after its inserts, fader and meter and before the
     *  final clamp to -1.0 .. 1.0. It is a CLimiterEffect owned by the mixer, prepared for the mix format (again on
     *  SetMixFormat()), and not an insert slot, so no effect can be placed after it. Modes:
     *   - off: the master output passes unchanged; what goes over full level is cut off by the clamp and counted
     *     (GetClippedSampleCount())
     *   - instant: no delay. The output is turned down at once, for the frames that need it, to 0.98 of full level
     *     (always that ceiling, SetLimiterCeilingInDecibels() is not used) and comes back up with the release time.
     *     With the default release of 50 ms this is, bit for bit, the limiter the mixer had before it had modes
     *   - lookahead: a brickwall limiter that turns down smoothly ahead of a peak, to the ceiling. The output is
     *     delayed by the lookahead (GetLatencyInFrames()) and the clamp after it has nothing left to do
     *  Off by default here; the engine switches it on by default, in lookahead mode (CAudioEngineParams).
     *
     *  The setters are for the owner thread. A change reaches the mixing thread at the start of the next block, through
     *  the parameters of the effect, without a lock; a change of the mode or of the lookahead restarts the limiter, so
     *  it is best made while Mix() is not running (CAudioEngine changes them with a device reopen)
     */
    void SetLimiterMode( TAudioLimiterMode mode );

    /** The mode the limiter stage works in: off when it is disabled */
    TAudioLimiterMode GetLimiterMode( void ) const;

    /**
     *  True switches the limiter on in the mode last given to SetLimiterMode() other than off (lookahead when none was
     *  given), false switches it off
     */
    void SetLimiterEnabled( bool enabled );
    bool IsLimiterEnabled( void ) const;

    /** Ceiling of the lookahead mode in dBFS, -12 .. 0 (default -0.3) */
    void SetLimiterCeilingInDecibels( Float32 decibels );
    Float32 GetLimiterCeilingInDecibels( void ) const;

    /** Lookahead of the lookahead mode in milliseconds, 0.5 .. 10 (default 1.5) */
    void SetLimiterLookaheadInMs( Float32 milliseconds );
    Float32 GetLimiterLookaheadInMs( void ) const;

    /** Release time constant in milliseconds, 10 .. 1000 (default 50), of both modes */
    void SetLimiterReleaseInMs( Float32 milliseconds );
    Float32 GetLimiterReleaseInMs( void ) const;

    /** Frames by which the mix comes out later than it went in: the lookahead in lookahead mode, 0 otherwise */
    UInt32 GetLatencyInFrames( void ) const;

    /** Mixes 'frames' frames of interleaved float audio into 'output' (replaces what is there). Samples are limited to -1.0 .. 1.0 */
    void Mix( Float32* output, UInt32 frames );

    /** Total frames mixed so far */
    UInt64 GetTotalFramesMixed( void ) const;

    /** Number of samples of the master output that had to be cut off because they went over full level */
    UInt64 GetClippedSampleCount( void ) const;

    /** Number of frames the limiter turned down */
    UInt64 GetLimitedFrameCount( void ) const;

    private:

    CAudioMixer( const CAudioMixer& src );              /**< not implemented */
    CAudioMixer& operator=( const CAudioMixer& src );   /**< not implemented */

    void MixBlock( Float32* output, UInt32 frames );

    void ProcessBus( CAudioBus* bus, UInt32 frames );

    static void ApplySends( CAudioBus* bus, bool preFader, UInt32 frames, UInt32 channels );

    void PrepareBusFor( CAudioBus* bus );

    void ApplyLimiterSettings( void );

    private:

    CAudioFormat m_mixFormat;
    UInt32 m_maxBlock;
    UInt32 m_maxVoices;
    UInt32 m_maxBuses;
    UInt32 m_maxEffectsPerBus;
    UInt32 m_maxSendsPerBus;
    GUCEF::vector< TAudioBusPtr > m_ownedBuses; /**< the master bus and the buses made by CreateBus(), owner side */
    GUCEF::vector< CAudioBus* > m_buses;        /**< every attached bus, parents come before their children */
    CAudioBus* const* m_order;                  /**< processing order, null for reverse attachment order */
    UInt32 m_orderCount;
    GUCEF::vector< CAudioVoice* > m_voices;
    IAudioMixerListener* m_listener;
    TAudioPanLaw m_panLaw;
    UInt64 m_totalFrames;
    UInt64 m_clipped;
    bool m_limiterEnabled;
    TAudioLimiterMode m_limiterOnMode;          /**< the mode of the limiter when it is enabled: instant or lookahead */
    Float32 m_limiterCeilingInDecibels;
    Float32 m_limiterLookaheadInMs;
    Float32 m_limiterReleaseInMs;
    CLimiterEffect m_limiter;                   /**< the master limiter stage */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOMIXER_H ? */
