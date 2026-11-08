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

#ifndef GUCEF_AUDIO_CNOISEGATEEFFECT_H
#define GUCEF_AUDIO_CNOISEGATEEFFECT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

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

#ifndef GUCEF_AUDIO_CAUDIOPARAMETERSMOOTHER_H
#include "gucefAUDIO_CAudioParameterSmoother.h"
#define GUCEF_AUDIO_CAUDIOPARAMETERSMOOTHER_H
#endif /* GUCEF_AUDIO_CAUDIOPARAMETERSMOOTHER_H ? */

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
 *  Noise gate: lowers the audio while there is no wanted signal, to a floor of "range" decibels rather than to silence,
 *  which sounds natural on an intercom and keeps the concealment of a receiver out of play. Registered as effect type
 *  "noiseGate" (the registry ignores case).
 *
 *  Detector (parameter "detector"):
 *   - level: the level of the audio decides. The level is the largest over all channels (linked) of the peak ("peak": an
 *     envelope that rises at once and falls with a time constant of 10 ms) or of the RMS ("rms": the mean square over all
 *     channels, one pole average of 10 ms), in dBFS. The gate opens when the level exceeds "threshold" and starts to close
 *     when it is below "threshold" minus "hysteresis", so a level between the two does not make the gate chatter
 *   - voice: a CVoiceActivityDetector of its own (energy of the band 200 to 4000 Hz, see there) decides on the mono mix of
 *     the channels: the gate opens when the voice is active and starts to close when it is not. "threshold",
 *     "hysteresis" and "levelSource" are not used. The detector needs a rate of 8000 to 192000 Hz; at another rate the gate
 *     uses the level detector whatever "detector" says. The detector only runs while this detector is selected
 *  Either way "hold" keeps the gate open for that long after the closing condition began, and the decision is taken once
 *  per sub-block of GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES frames from the audio before that sub-block, so the gate follows the
 *  audio by one sub-block (0.7 ms at 48 kHz) and, in voice mode, by the delay of the voice detector as well (10 to 30 ms
 *  after the start of a burst): the first part of a word is passed at the floor level. There is no lookahead.
 *
 *  Gain: the gate has a position between 0 (closed) and 1 (open) that moves by 1 per "attack" milliseconds while the gate is
 *  open and falls by 1 per "release" milliseconds when it is closing. The gain in decibels is "range" times ( 1 - position ),
 *  so attack and release are the time a complete change of the gain takes, and the change in decibels is linear in time.
 *  The gain is applied as a linear ramp over every sub-block, and a change of the range is smoothed (20 ms one pole), so
 *  the audio never steps. While the gate is open the audio passes bit for bit. The state is cleared of denormals; the output
 *  does not depend on how the audio is divided into blocks. A "range" of 0 dB gives a gate that does nothing.
 *
 *  The settings of the voice detector are construction options (so a saved layout restores them): "vadOnsetDb",
 *  "vadReleaseDb", "vadHangoverMs", "vadMinimumLevelDb", "vadFloorRiseDbPerSecond", written by GetConstructionOptions()
 *  only where they differ from the defaults of CVoiceActivitySettings. SetVoiceActivitySettings() sets them on the owner
 *  side before the effect is inserted. No latency, no tail.
 */
class GUCEF_AUDIO_PUBLIC_CPP CNoiseGateEffect : public CAudioEffect
{
    public:

    enum EParameter
    {
        PARAMETER_DETECTOR = 0  ,   /**< enumeration: level, voice (the voice activity detector decides) */
        PARAMETER_THRESHOLD     ,   /**< dBFS, -90 .. 0, level detector only: the gate opens above it */
        PARAMETER_HYSTERESIS    ,   /**< dB, 0 .. 12, level detector only: the gate starts to close this far below the threshold */
        PARAMETER_RANGE         ,   /**< dB, -80 .. 0: the gain while the gate is closed */
        PARAMETER_ATTACK        ,   /**< ms, 0.1 .. 50: the time a complete opening takes */
        PARAMETER_HOLD          ,   /**< ms, 0 .. 1000: the gate stays open this long after the closing condition began */
        PARAMETER_RELEASE       ,   /**< ms, 10 .. 2000: the time a complete closing takes */
        PARAMETER_LEVEL_SOURCE  ,   /**< enumeration: peak, rms (what the level detector measures) */

        PARAMETER_COUNT
    };

    enum EDetector
    {
        DETECTOR_LEVEL = 0  ,
        DETECTOR_VOICE
    };

    enum ELevelSource
    {
        LEVELSOURCE_PEAK = 0    ,
        LEVELSOURCE_RMS
    };

    static const CString& GetStaticEffectTypeName( void );

    static UInt32 GetStaticParameterCount( void );

    static const CAudioEffectParameterInfo* GetStaticParameterInfos( void );

    /** @param options  the settings of the voice detector, see the class description */
    explicit CNoiseGateEffect( const CORE::CValueList& options = CORE::CValueList() );

    virtual ~CNoiseGateEffect() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetEffectTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    void SetDetector( EDetector detector );
    void SetThresholdInDecibels( Float32 decibels );
    void SetHysteresisInDecibels( Float32 decibels );
    void SetRangeInDecibels( Float32 decibels );
    void SetAttackInMs( Float32 milliseconds );
    void SetHoldInMs( Float32 milliseconds );
    void SetReleaseInMs( Float32 milliseconds );
    void SetLevelSource( ELevelSource source );

    /** Owner side, while the effect is not processed: the settings of the voice detector, in force from the next Prepare() or Reset() */
    void SetVoiceActivitySettings( const CVoiceActivitySettings& settings );

    const CVoiceActivitySettings& GetVoiceActivitySettings( void ) const;

    /** Any thread: true when the gate was open (including its hold time) at the end of the last block */
    bool IsOpen( void ) const;

    /** Any thread: the gain in dB (the range or above, 0 or below) at the end of the last block. For meters */
    Float32 GetGainInDecibels( void ) const;

    /** Adds the settings of the voice detector that differ from the defaults */
    virtual void GetConstructionOptions( CORE::CValueList& options ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual bool OnPrepare( const CAudioFormat& format, UInt32 maxBlockFrames ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnReset( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnProcess( Float32* inOut, UInt32 frames ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CNoiseGateEffect( const CNoiseGateEffect& src );              /**< not implemented */
    CNoiseGateEffect& operator=( const CNoiseGateEffect& src );   /**< not implemented */

    void ApplyParameters( bool jump );
    void ClearState( void );
    void StepSubBlock( void );
    void DetectLevel( const Float32* frame, UInt32 frames );
    void FeedVoiceDetector( const Float32* frame, UInt32 frames, UInt64 position );

    private:

    // owner side
    CVoiceActivitySettings m_voiceSettings;

    // any thread
    MT::CTAtomic< bool > m_publishedOpen;
    CAtomicFloat m_publishedGainDb;

    // audio thread
    UInt32 m_channels;
    Float64 m_sampleRate;
    CVoiceActivityDetector m_voice;
    bool m_voiceReady;                  /**< the voice detector could be prepared for the rate */
    bool m_useVoice;
    bool m_useRms;
    CAudioParameterSmoother m_range;
    Float32 m_thresholdInDecibels;
    Float32 m_closeInDecibels;          /**< threshold minus hysteresis */
    Float32 m_attackStep;               /**< position change per sub-block while open */
    Float32 m_releaseStep;              /**< position change per sub-block while closing */
    UInt32 m_holdFrames;
    Float64 m_peakFall;                 /**< per frame factor of the peak envelope */
    Float64 m_squareCoefficient;        /**< per frame coefficient of the mean square average */
    Float64 m_peakEnvelope;
    Float64 m_meanSquare;
    bool m_open;                        /**< the decision, hold time included */
    UInt32 m_holdLeft;                  /**< frames */
    Float32 m_position;                 /**< 0 (closed) .. 1 (open) */
    Float32 m_gain;                     /**< linear, where the ramp is now */
    Float32 m_gainEnd;                  /**< linear, where the ramp of the sub-block ends */
    Float32 m_gainStep;                 /**< per frame */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CNOISEGATEEFFECT_H ? */
