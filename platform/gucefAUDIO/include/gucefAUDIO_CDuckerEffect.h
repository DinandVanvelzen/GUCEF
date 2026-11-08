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

#ifndef GUCEF_AUDIO_CDUCKEREFFECT_H
#define GUCEF_AUDIO_CDUCKEREFFECT_H

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

class CAudioBus;

/**
 *  Lowers the bus it is inserted on while something else is loud: music under an announcement, a call under a page.
 *  Registered as effect type "ducker".
 *
 *  The key is either the level meter of another bus (CAudioBus::GetMeter(), after that bus's fader), read with atomic
 *  loads once per block, so the key lags the audio by at most one block and no sidechain routing is needed, or the manual
 *  key SetManualKey(), which any thread may set (to duck before a chime starts, or from a half duplex controller).
 *  Ducking is wanted while the manual key is set or the key level, the largest of all channels of the meter, is above the
 *  threshold. The key level is the meter's peak of the last block or its running RMS (300 ms), as chosen by the parameter
 *  "keySource".
 *
 *  Ducking has a position between 0 (no ducking) and 1 (full depth) that moves on sub-blocks of
 *  GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES frames:
 *   - ducking wanted: the position rises by 1 per "attack" milliseconds, and the hold time is restarted
 *   - not wanted, hold time left: nothing moves, the hold time runs down
 *   - not wanted, no hold time left: the position falls by 1 per "release" milliseconds
 *  The gain in decibels is the depth times the position, so attack and release are the time that a full change takes and the
 *  change in decibels is linear in time. The gain is applied as a linear ramp over every sub-block, and a change of the depth
 *  is smoothed (20 ms one pole), so the audio never steps. Without ducking the audio passes bit for bit.
 *
 *  Construction option "keyBus": the name of the bus whose meter is the key. CAudioEngine resolves it through
 *  ResolveReferences() when the effect is inserted (buses are never removed, so the pointer stays valid); a name that no bus
 *  has refuses the insert. SetKeyBus() sets the bus directly, on the owner side before the effect is inserted. Without a
 *  key bus only the manual key ducks. No latency, no tail.
 */
class GUCEF_AUDIO_PUBLIC_CPP CDuckerEffect : public CAudioEffect
{
    public:

    enum EParameter
    {
        PARAMETER_DEPTH = 0 ,   /**< dB, -60 .. 0 */
        PARAMETER_THRESHOLD ,   /**< dBFS of the key level, -80 .. 0 */
        PARAMETER_ATTACK    ,   /**< ms, 1 .. 500 */
        PARAMETER_HOLD      ,   /**< ms, 0 .. 5000 */
        PARAMETER_RELEASE   ,   /**< ms, 10 .. 5000 */
        PARAMETER_KEY_SOURCE,   /**< enumeration: peak, rms (the key bus meter reading that is compared with the threshold) */

        PARAMETER_COUNT
    };

    enum EKeySource
    {
        KEYSOURCE_PEAK = 0  ,
        KEYSOURCE_RMS
    };

    static const CString& GetStaticEffectTypeName( void );

    static UInt32 GetStaticParameterCount( void );

    static const CAudioEffectParameterInfo* GetStaticParameterInfos( void );

    /** @param options  "keyBus": name of the bus whose meter is the key */
    explicit CDuckerEffect( const CORE::CValueList& options = CORE::CValueList() );

    virtual ~CDuckerEffect() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetEffectTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    void SetDepthInDecibels( Float32 decibels );
    void SetThresholdInDecibels( Float32 decibels );
    void SetAttackInMs( Float32 milliseconds );
    void SetHoldInMs( Float32 milliseconds );
    void SetReleaseInMs( Float32 milliseconds );
    void SetKeySource( EKeySource source );

    /** Owner side, before the effect is inserted: the bus whose meter is the key, null for none. Buses are never removed */
    void SetKeyBus( const CAudioBus* bus );

    /** The name of the key bus given in the options or taken from SetKeyBus(); empty without one */
    const CString& GetKeyBusName( void ) const;

    /** Any thread, real time safe (an atomic): ducks (true) or stops ducking (false) whatever the key bus does. Takes effect at the next block */
    void SetManualKey( bool active );

    bool IsManualKeyActive( void ) const;

    /** Any thread: the gain in dB (0 or below) applied at the end of the last block. For meters */
    Float32 GetGainInDecibels( void ) const;

    /** Adds "keyBus" when there is one */
    virtual void GetConstructionOptions( CORE::CValueList& options ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Finds the bus named by "keyBus" in the engine. False, with a log entry, when there is none */
    virtual bool ResolveReferences( CAudioEngine& engine ) GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual bool OnPrepare( const CAudioFormat& format, UInt32 maxBlockFrames ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnReset( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnProcess( Float32* inOut, UInt32 frames ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CDuckerEffect( const CDuckerEffect& src );              /**< not implemented */
    CDuckerEffect& operator=( const CDuckerEffect& src );   /**< not implemented */

    void ApplyParameters( bool jump );
    bool IsDuckingWanted( void ) const;
    void StepSubBlock( void );

    private:

    // owner side
    CString m_keyBusName;
    const CAudioBus* m_keyBus;

    // any thread
    MT::CTAtomic< bool > m_manualKey;
    CAtomicFloat m_gainDb;

    // audio thread
    UInt32 m_channels;
    Float64 m_sampleRate;
    CAudioParameterSmoother m_depth;
    Float32 m_thresholdLinear;
    bool m_useRms;
    Float32 m_attackStep;           /**< position change per sub-block while ducking is wanted */
    Float32 m_releaseStep;          /**< position change per sub-block while it is not, after the hold time */
    UInt32 m_holdFrames;
    bool m_keyWanted;               /**< the key as read at the start of the block */
    Float32 m_position;             /**< 0 .. 1 */
    UInt32 m_holdLeft;              /**< frames */
    Float32 m_gain;                 /**< linear, where the ramp is now */
    Float32 m_gainEnd;              /**< linear, where the ramp of the sub-block ends */
    Float32 m_gainStep;             /**< per frame */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CDUCKEREFFECT_H ? */
