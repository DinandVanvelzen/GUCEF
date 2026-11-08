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

#ifndef GUCEF_AUDIO_CHALFDUPLEXCONTROLLER_H
#define GUCEF_AUDIO_CHALFDUPLEXCONTROLLER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_AUDIO_MACROS_H
#include "gucefAUDIO_macros.h"
#define GUCEF_AUDIO_MACROS_H
#endif /* GUCEF_AUDIO_MACROS_H ? */

#ifndef GUCEF_AUDIO_ETYPES_H
#include "gucefAUDIO_ETypes.h"
#define GUCEF_AUDIO_ETYPES_H
#endif /* GUCEF_AUDIO_ETYPES_H ? */

#ifndef GUCEF_AUDIO_CATOMICFLOAT_H
#include "gucefAUDIO_CAtomicFloat.h"
#define GUCEF_AUDIO_CATOMICFLOAT_H
#endif /* GUCEF_AUDIO_CATOMICFLOAT_H ? */

#ifndef GUCEF_AUDIO_CAUDIOEFFECT_H
#include "gucefAUDIO_CAudioEffect.h"
#define GUCEF_AUDIO_CAUDIOEFFECT_H
#endif /* GUCEF_AUDIO_CAUDIOEFFECT_H ? */

#ifndef GUCEF_AUDIO_CVOICEACTIVITYDETECTOR_H
#include "gucefAUDIO_CVoiceActivityDetector.h"
#define GUCEF_AUDIO_CVOICEACTIVITYDETECTOR_H
#endif /* GUCEF_AUDIO_CVOICEACTIVITYDETECTOR_H ? */

#ifndef GUCEF_AUDIO_CHALFDUPLEXSETTINGS_H
#include "gucefAUDIO_CHalfDuplexSettings.h"
#define GUCEF_AUDIO_CHALFDUPLEXSETTINGS_H
#endif /* GUCEF_AUDIO_CHALFDUPLEXSETTINGS_H ? */

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
class CAudioLevelMeter;
class CDuckerEffect;

/*-------------------------------------------------------------------------*/

/**
 *  Voice-switched half duplex for a unit whose speaker the microphone hears (section 4.13 of the intercom design,
 *  stage 1 of the echo handling): while the far end talks the microphone is attenuated, so the far talker does not
 *  hear their own voice back; while the near end talks the far end is ducked on the speaker. A stage of the capture
 *  path (CAudioEngine::SetCaptureHalfDuplexController()): it runs once per capture block on the processed audio before
 *  the microphone gain, and the gain it asks for goes into the ramp of that gain (10 ms from full to off).
 *
 *  What it looks at, per block:
 *   - the near end: the power of the block (smoothed over 10 ms) and its own CVoiceActivityDetector, both before the
 *     microphone gain (an attenuated microphone must still be heard to break in)
 *   - the far end: the meter of the far-end bus (the bus the receivers play on), read once per block: its peak gives
 *     the activity and, held with the release 'farReleaseMs', the envelope the echo is estimated from
 *   - the acoustic coupling: while only the far end is active (state FAR, no break in on its way) the mean power of
 *     the microphone and of the far-end envelope are averaged over 'couplingTimeMs'; their ratio C turns the far-end
 *     envelope into the echo estimate E. GetCouplingInDecibels() tells the coupling against the mean far-end power
 *     (the meter's RMS), which is the attenuation of the acoustic path
 *  The near end talks when its detector hears a voice and its power is above E by 'breakInMarginDb'.
 *
 *  Hand-over to an echo canceller (stage 2): the capture path tells the controller every block whether its
 *  CAcousticEchoCanceller is converged and its echo return loss enhancement (SetEchoCancellation()). While the canceller
 *  is converged with at least 'handOverErleDb' (and 'echoCancellerHandOver' is on) the controller stands aside: the
 *  microphone gain is 1 and the ducker's key is released, so the call is full duplex; the states go on being followed
 *  (and the coupling measured, now behind the canceller). It takes over again, at once, as soon as the canceller is no
 *  longer converged or its enhancement fell 6 dB below the hand-over level. Half duplex stays the fallback and the start
 *  of every call, until the canceller has converged.
 *
 *  States (THalfDuplexState):
 *   - IDLE: microphone open, speaker as it is. Near end talks -> NEAR; else far end active -> FAR
 *   - FAR: microphone at 'micAttenuationDb'. Near end talks for 'breakInMs' -> NEAR (break in); far end quiet for
 *     'hangoverMs' -> IDLE
 *   - NEAR: microphone open, the far end ducked (the ducker's manual key, when 'duckFarEnd' and a ducker is set).
 *     Near end quiet for 'hangoverMs' -> IDLE
 *
 *  Threading: the setup (SetSettings(), SetFarEndBus(), SetFarEndMeter(), SetDucker(), ResolveReferences(), Prepare(),
 *  Reset()) belongs to the owner while no capture path uses the controller. Process() is the capture thread's: it
 *  allocates nothing, takes no lock, logs nothing. SetEnabled() and the getters work from any thread (atomics).
 *  The far-end bus (buses are never removed) and the ducker (the controller holds a reference) stay valid as long as
 *  the controller is used.
 */
class GUCEF_AUDIO_PUBLIC_CPP CHalfDuplexController
{
    public:

    explicit CHalfDuplexController( const CHalfDuplexSettings& settings = CHalfDuplexSettings() );

    ~CHalfDuplexController();

    /** Owner: the settings; the enabled switch of the settings becomes the current one. Takes effect at Prepare() */
    void SetSettings( const CHalfDuplexSettings& settings );
    const CHalfDuplexSettings& GetSettings( void ) const;

    /** Owner: the far end is the meter of this bus (null: none, ResolveReferences() finds one by name) */
    void SetFarEndBus( const CAudioBus* bus );

    /** Owner: the far end is this meter (of a bus, or of anything else the test or the application meters). Null: none */
    void SetFarEndMeter( const CAudioLevelMeter* meter );
    const CAudioLevelMeter* GetFarEndMeter( void ) const;

    /** Owner: the ducker whose manual key lowers the far end in state NEAR. False, and nothing changed, for another effect type */
    bool SetDucker( const TAudioEffectPtr& ducker );
    const TAudioEffectPtr& GetDucker( void ) const;

    /**
     *  Owner: when no far-end meter is set, the bus named in the settings (the master bus when the name is empty)
     *  becomes the far end. False when the name is not a bus of the engine
     */
    bool ResolveReferences( CAudioEngine& engine );

    /** Owner: prepares for the processing rate of the capture path (8000 to 192000 Hz) and starts in IDLE. Allocates */
    bool Prepare( UInt32 sampleRate );
    bool IsPrepared( void ) const;
    UInt32 GetSampleRate( void ) const;

    /** Owner, while nothing processes: IDLE, no coupling measured, detector reset; the ducker's key is released */
    void Reset( void );

    /**
     *  Capture thread: one block of the processed microphone signal (mono, before the microphone gain) at the absolute
     *  frame position 'firstFrame' of the processing rate. Returns the microphone gain the controller wants (linear):
     *  1 or the attenuation of FAR; 1 when disabled or not prepared
     */
    Float32 Process( const Float32* mono, UInt32 frameCount, UInt64 firstFrame ) GUCEF_AUDIO_REALTIME_CONTEXT;

    /**
     *  Capture thread, before Process(): whether the echo canceller of the capture path is converged, and its echo return
     *  loss enhancement in dB (see the hand-over in the class description). Without a call the controller acts alone
     */
    void SetEchoCancellation( bool converged, Float32 erleInDecibels ) GUCEF_AUDIO_REALTIME_CONTEXT;

    /** Any thread: the controller stands aside for the echo canceller (hand-over) */
    bool IsHandedOver( void ) const;

    /** Any thread: hand-overs to the echo canceller since Prepare() or Reset() */
    UInt64 GetHandOverCount( void ) const;

    /** Any thread: on or off. Off, the microphone gain is 1, the ducker's key is released and the state is IDLE */
    void SetEnabled( bool enabled );
    bool IsEnabled( void ) const;

    /** Any thread: the state after the last block */
    THalfDuplexState GetState( void ) const;

    /** Any thread: the microphone gain asked for after the last block, linear */
    Float32 GetMicGain( void ) const;

    /** Any thread: the acoustic coupling measured (microphone power against the mean far-end power), dB; the initial coupling until one was measured */
    Float32 GetCouplingInDecibels( void ) const;
    bool HasMeasuredCoupling( void ) const;

    /** Any thread: the near-end power (10 ms), the far-end envelope and the echo estimate of the last block, dBFS */
    Float32 GetNearLevelInDecibels( void ) const;
    Float32 GetFarLevelInDecibels( void ) const;
    Float32 GetEchoEstimateInDecibels( void ) const;

    /** Any thread: the decision of the controller's own near-end voice activity detector */
    bool IsNearVoiceActive( void ) const;

    /** Any thread: state changes since Prepare() or Reset() */
    UInt64 GetTransitionCount( void ) const;

    private:

    CHalfDuplexController( const CHalfDuplexController& src );              /**< not implemented */
    CHalfDuplexController& operator=( const CHalfDuplexController& src );   /**< not implemented */

    void EnterState( THalfDuplexState state );
    void SetDuckKey( bool active );

    private:

    CHalfDuplexSettings m_settings;
    const CAudioLevelMeter* m_farMeter;
    bool m_farMeterSet;                     /**< set by SetFarEndBus() / SetFarEndMeter(): ResolveReferences() leaves it */
    TAudioEffectPtr m_duckerRef;
    CDuckerEffect* m_ducker;
    CVoiceActivityDetector m_detector;
    UInt32 m_sampleRate;

    // derived from the settings at Prepare()
    Float64 m_micGain;
    Float64 m_farThreshold;                 /**< linear peak */
    Float64 m_breakInFactor;                /**< power ratio */
    Float64 m_initialCoupling;              /**< power ratio */

    // capture thread
    THalfDuplexState m_state;
    bool m_wasEnabled;
    bool m_duckKey;
    Float64 m_nearPower;
    Float64 m_farEnvelope;
    Float64 m_nearAverage;                  /**< of the microphone power while only the far end talked */
    Float64 m_farEnvelopeAverage;           /**< of the far-end envelope at the same time */
    Float64 m_farMeanAverage;               /**< of the far-end mean power (the meter's RMS squared) at the same time */
    Float64 m_couplingWeight;               /**< how much of the averages is measured (0 to 1) */
    Float64 m_breakInTime;
    Float64 m_quietTime;
    bool m_cancellerConverged;
    Float32 m_cancellerErleDb;
    bool m_handedOver;

    // any thread
    MT::CTAtomic< bool > m_enabled;
    MT::CTAtomic< UInt32 > m_publishedState;
    MT::CTAtomic< UInt64 > m_transitions;
    MT::CTAtomic< bool > m_nearVoice;
    MT::CTAtomic< bool > m_couplingMeasured;
    CAtomicFloat m_publishedGain;
    CAtomicFloat m_couplingDb;
    CAtomicFloat m_nearDb;
    CAtomicFloat m_farDb;
    CAtomicFloat m_echoDb;
    MT::CTAtomic< bool > m_publishedHandedOver;
    MT::CTAtomic< UInt64 > m_handOvers;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CHALFDUPLEXCONTROLLER_H ? */
