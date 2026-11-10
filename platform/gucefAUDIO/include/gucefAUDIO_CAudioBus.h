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

#ifndef GUCEF_AUDIO_CAUDIOBUS_H
#define GUCEF_AUDIO_CAUDIOBUS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_MACROS_H
#include "gucefAUDIO_macros.h"
#define GUCEF_AUDIO_MACROS_H
#endif /* GUCEF_AUDIO_MACROS_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_AUDIO_CATOMICFLOAT_H
#include "gucefAUDIO_CAtomicFloat.h"
#define GUCEF_AUDIO_CATOMICFLOAT_H
#endif /* GUCEF_AUDIO_CATOMICFLOAT_H ? */

#ifndef GUCEF_AUDIO_CAUDIOEFFECTCHAIN_H
#include "gucefAUDIO_CAudioEffectChain.h"
#define GUCEF_AUDIO_CAUDIOEFFECTCHAIN_H
#endif /* GUCEF_AUDIO_CAUDIOEFFECTCHAIN_H ? */

#ifndef GUCEF_AUDIO_CAUDIOLEVELMETER_H
#include "gucefAUDIO_CAudioLevelMeter.h"
#define GUCEF_AUDIO_CAUDIOLEVELMETER_H
#endif /* GUCEF_AUDIO_CAUDIOLEVELMETER_H ? */

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
class CAudioVoice;

/*-------------------------------------------------------------------------*/

/**
 *  A group of sounds that is mixed and controlled together: effects, music, voices, the whole game.
 *  Buses form a tree. Sounds play into a bus, the bus adds up what plays into it and what its child
 *  buses deliver, applies its own volume and mute, and passes the result on to its parent. The top of the tree
 *  is the master bus, whose output is what the speakers play.
 *
 *  Volume is a linear factor (1.0 is unchanged, 0.5 is 6 dB quieter, 0.0 is silence). Changes are heard
 *  smoothly, over the next piece of audio, so they do not click.
 *
 *  What a bus does with its audio, in this order: the effects in its insert slots, the pre-fader sends, its volume
 *  and mute (the fader), the level meter, the post-fader sends, and on to the parent. Effects and sends are placed by
 *  CAudioEngine (InsertEffect(), AddSend()); their settings below can be changed from any thread at any time.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioBus
{
    public:

    typedef CORE::CTSharedPtr< CAudioBus, MT::CMutex > TAudioBusPtr;

    CAudioBus( const CString& name, CAudioBus* parent );

    ~CAudioBus();

    const CString& GetName( void ) const;

    /** The bus that this one feeds, null for the master bus */
    CAudioBus* GetParent( void ) const;

    /** Linear volume, any thread */
    void SetVolume( Float32 volume );
    Float32 GetVolume( void ) const;

    /** Convenience for the usual unit of volume: decibels, 0 dB is unchanged */
    void SetVolumeInDecibels( Float32 decibels );
    Float32 GetVolumeInDecibels( void ) const;

    void SetMuted( bool muted );
    bool IsMuted( void ) const;

    /** What the bus lets through: its volume, or nothing when muted */
    Float32 GetEffectiveVolume( void ) const;

    static Float32 DecibelsToLinear( Float32 decibels );
    static Float32 LinearToDecibels( Float32 linear );

    /** Number of insert effect slots (CAudioEngineParams::GetMaxEffectsPerBus()), 0 before the bus is prepared */
    UInt32 GetEffectSlotCount( void ) const;

    /** Any thread: switches the effect in a slot on or off, crossfaded. Slots are on by default */
    void SetEffectEnabled( UInt32 slot, bool enabled );
    bool IsEffectEnabled( UInt32 slot ) const;

    /** Any thread: wet and dry level of an effect slot, linear. Default 1 and 0 */
    void SetEffectMix( UInt32 slot, Float32 wet, Float32 dry );
    Float32 GetEffectWet( UInt32 slot ) const;
    Float32 GetEffectDry( UInt32 slot ) const;

    /** Number of send slots (CAudioEngineParams::GetMaxSendsPerBus()), 0 before the bus is prepared */
    UInt32 GetSendSlotCount( void ) const;

    /** Any thread: linear level of a send, changes are ramped over a block */
    void SetSendLevel( UInt32 sendIndex, Float32 level );
    Float32 GetSendLevel( UInt32 sendIndex ) const;

    /** Levels of the bus after its fader, any thread */
    const CAudioLevelMeter& GetMeter( void ) const;

    private:
    friend class CAudioMixer;
    friend class CAudioVoice;      /**< a voice adds itself to the buses of its sends */

    CAudioBus( const CAudioBus& src );              /**< not implemented */
    CAudioBus& operator=( const CAudioBus& src );   /**< not implemented */

    struct SSend
    {
        CAudioBus* target;          /**< null for a free slot. Set by the thread that mixes */
        bool preFader;
        CAtomicFloat level;
        Float32 lastLevel;          /**< mixing thread: level at the end of the previous block */
        bool levelKnown;

        SSend( void );
    };

    /** Owner thread, while the bus is not mixed: sizes the send slots (kept when the count does not change) */
    void PrepareSends( UInt32 sendCapacity );

    private:

    CString m_name;
    CAudioBus* m_parent;
    CAtomicFloat m_volume;
    MT::CTAtomic< bool > m_muted;
    CAudioEffectChain m_effects;
    SSend* m_sends;
    UInt32 m_sendCapacity;
    CAudioLevelMeter m_meter;

    // used by the mixer on the audio thread only
    GUCEF::vector< Float32 > m_buffer;
    Float32 m_lastGain;
    bool m_gainKnown;
};

typedef CAudioBus::TAudioBusPtr TAudioBusPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOBUS_H ? */
