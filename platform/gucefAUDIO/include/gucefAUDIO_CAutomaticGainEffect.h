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

#ifndef GUCEF_AUDIO_CAUTOMATICGAINEFFECT_H
#define GUCEF_AUDIO_CAUTOMATICGAINEFFECT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

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

#ifndef GUCEF_AUDIO_CVOICEACTIVITYSETTINGS_H
#include "gucefAUDIO_CVoiceActivitySettings.h"
#define GUCEF_AUDIO_CVOICEACTIVITYSETTINGS_H
#endif /* GUCEF_AUDIO_CVOICEACTIVITYSETTINGS_H ? */

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
 *  Automatic gain control for speech: a slow normalisation of the level of the talker towards a target. Registered as
 *  effect type "automaticGain" (the registry ignores case).
 *
 *  The speech level S is the RMS of the audio (all channels, mean square) in dBFS over the analysis frames of 10 ms
 *  (aligned to the absolute frame count, as those of the voice detector) in which the voice is active, averaged with a
 *  one pole filter of the time "levelTime" in the power domain and updated in those frames only. The effect has a
 *  CVoiceActivityDetector of its own on the mono mix of the channels. Only frames that exceed the noise floor of that
 *  detector by its release margin count: the frames of the hangover time after speech, which are pauses, do not pull the
 *  level down, and of those only the ones that are no more than 12 dB below the level so far (the active speech level
 *  of ITU-T P.56 works the same way): the quiet ends of syllables would pull the level 1 to 3 dB under what a listener
 *  hears as the level of the talker. When 100 active frames in a row are that far below, the talker became much
 *  quieter and the level starts over from them. The first frame sets the level, so there is no start-up from zero.
 *
 *  Desired gain: G* = clamp( target - S, minGain, maxGain ). The applied gain moves towards G* by no more than "riseRate"
 *  (G* above the gain) or "fallRate" (below) decibels per second, in steps on the sub-blocks of
 *  GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES frames (aligned to the absolute frame count), and is applied as a linear ramp over each
 *  sub-block. With "freeze" on the gain does not move while the voice is not active, so the noise floor of the pauses is not
 *  raised between sentences (no pumping); the one exception is a gain outside minGain and maxGain after one of them was
 *  changed, which moves in at the rates above. Without any speech the gain stays at 0 dB and the audio passes bit for bit.
 *  The voice detector decides one sub-block late (and 10 to 30 ms after the start of a burst), so the gain of a pause is
 *  held while the first part of a burst after a pause is passed at the gain of the earlier speech.
 *
 *  Ceiling: the output never exceeds "ceiling" dBFS in any channel. A gain reduction with instant attack that is
 *  released with a time constant of 100 ms is applied wherever the gained audio would be above it (the largest of the
 *  channels decides, so the stereo image is kept). With the audio below the ceiling this stage does nothing and the audio
 *  is not changed by it; it is the safety net of the effect, not a limiter in the sense of CLimiterEffect: a design that
 *  follows with a CLimiterEffect can set the ceiling to 0 dB.
 *
 *  The output does not depend on how the audio is divided into blocks (the detector is fed in pieces that never cross a
 *  sub-block or an analysis frame). Needs a rate of 8000 to 192000 Hz (Prepare() fails otherwise). The settings of the voice
 *  detector are construction options, see CNoiseGateEffect ("vadOnsetDb", "vadReleaseDb", "vadHangoverMs",
 *  "vadMinimumLevelDb", "vadFloorRiseDbPerSecond"); SetVoiceActivitySettings() sets them on the owner side before the
 *  effect is inserted. No latency, no tail.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAutomaticGainEffect : public CAudioEffect
{
    public:

    enum EParameter
    {
        PARAMETER_TARGET = 0    ,   /**< dBFS, -40 .. -6: the RMS level of active speech to aim for */
        PARAMETER_MAX_GAIN      ,   /**< dB, 0 .. 40 */
        PARAMETER_MIN_GAIN      ,   /**< dB, -20 .. 0 */
        PARAMETER_RISE_RATE     ,   /**< dB per second, 0.5 .. 30: the fastest the gain goes up */
        PARAMETER_FALL_RATE     ,   /**< dB per second, 1 .. 60: the fastest the gain goes down */
        PARAMETER_LEVEL_TIME    ,   /**< ms, 100 .. 3000: the time constant of the speech level average */
        PARAMETER_FREEZE        ,   /**< enumeration: off, on (the gain does not move while the voice is not active) */
        PARAMETER_CEILING       ,   /**< dBFS, -12 .. 0: the output peak never exceeds it */

        PARAMETER_COUNT
    };

    enum EFreeze
    {
        FREEZE_OFF = 0  ,
        FREEZE_ON
    };

    static const CString& GetStaticEffectTypeName( void );

    static UInt32 GetStaticParameterCount( void );

    static const CAudioEffectParameterInfo* GetStaticParameterInfos( void );

    /** @param options  the settings of the voice detector, see the class description */
    explicit CAutomaticGainEffect( const CORE::CValueList& options = CORE::CValueList() );

    virtual ~CAutomaticGainEffect() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetEffectTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    void SetTargetInDecibels( Float32 decibels );
    void SetMaxGainInDecibels( Float32 decibels );
    void SetMinGainInDecibels( Float32 decibels );
    void SetRiseRateInDecibelsPerSecond( Float32 decibelsPerSecond );
    void SetFallRateInDecibelsPerSecond( Float32 decibelsPerSecond );
    void SetLevelTimeInMs( Float32 milliseconds );
    void SetFreeze( bool freeze );
    void SetCeilingInDecibels( Float32 decibels );

    /** Owner side, while the effect is not processed: the settings of the voice detector, in force from the next Prepare() or Reset() */
    void SetVoiceActivitySettings( const CVoiceActivitySettings& settings );

    const CVoiceActivitySettings& GetVoiceActivitySettings( void ) const;

    /** Any thread: the gain in dB applied at the end of the last block (not counting the ceiling). For meters */
    Float32 GetGainInDecibels( void ) const;

    /** Any thread: the speech level in dBFS as of the last block, -120 while no speech was seen */
    Float32 GetSpeechLevelInDecibels( void ) const;

    /** Adds the settings of the voice detector that differ from the defaults */
    virtual void GetConstructionOptions( CORE::CValueList& options ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual bool OnPrepare( const CAudioFormat& format, UInt32 maxBlockFrames ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnReset( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnProcess( Float32* inOut, UInt32 frames ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CAutomaticGainEffect( const CAutomaticGainEffect& src );              /**< not implemented */
    CAutomaticGainEffect& operator=( const CAutomaticGainEffect& src );   /**< not implemented */

    void ApplyParameters( void );
    void ClearState( void );
    void StepSubBlock( void );
    void FinishAnalysisFrame( void );
    UInt64 FrameBoundary( UInt64 frameIndex ) const;

    private:

    // owner side
    CVoiceActivitySettings m_voiceSettings;

    // any thread
    CAtomicFloat m_publishedGainDb;
    CAtomicFloat m_publishedLevelDb;

    // audio thread
    UInt32 m_channels;
    UInt32 m_sampleRate;
    CVoiceActivityDetector m_voice;
    Float32 m_releaseMargin;            /**< the release margin of the voice detector settings, dB */
    Float32 m_targetInDecibels;
    Float32 m_maxGainInDecibels;
    Float32 m_minGainInDecibels;
    Float32 m_riseStep;                 /**< dB per sub-block */
    Float32 m_fallStep;                 /**< dB per sub-block */
    Float64 m_levelCoefficient;         /**< weight of an active analysis frame in the speech level */
    bool m_freeze;
    Float32 m_ceiling;                  /**< linear */
    Float32 m_limitRelease;             /**< per frame coefficient of the release of the ceiling stage */

    // analysis frames of 10 ms, as those of the voice detector
    UInt64 m_frameIndex;
    UInt64 m_frameStart;
    UInt64 m_frameEnd;
    Float64 m_frameSum;                 /**< sum of squares of the mono mix in the frame so far */

    // level and gain
    bool m_haveLevel;
    Float64 m_speechMeanSquare;
    UInt32 m_rejectedFrames;            /**< active frames in a row far below the level */
    Float64 m_rejectedSum;              /**< their mean squares added */
    Float32 m_gainInDecibels;           /**< where the gain control is now */
    Float32 m_gain;                     /**< linear, where the ramp is now */
    Float32 m_gainEnd;                  /**< linear, where the ramp of the sub-block ends */
    Float32 m_gainStep;                 /**< per frame */
    Float32 m_limit;                    /**< factor of the ceiling stage, 1 when it does nothing */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUTOMATICGAINEFFECT_H ? */
