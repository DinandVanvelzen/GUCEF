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

#ifndef GUCEF_AUDIO_CAUDIOEFFECTCHAIN_H
#define GUCEF_AUDIO_CAUDIOEFFECTCHAIN_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_CAUDIOEFFECT_H
#include "gucefAUDIO_CAudioEffect.h"
#define GUCEF_AUDIO_CAUDIOEFFECT_H
#endif /* GUCEF_AUDIO_CAUDIOEFFECT_H ? */

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
 *  A fixed number of effect slots that audio passes through in order, as on a bus (its inserts) or a voice.
 *
 *  Each slot holds an effect (a plain pointer: the owner keeps the effect alive, see CAudioEngine) and its settings:
 *   - enabled: switching it crossfades over 10 ms between the processed and the unprocessed signal. After a fade out the
 *     effect is not processed at all, and it is Reset() before it fades in again
 *   - wet and dry levels (default 1 and 0): out = wet x processed + dry x input, both ramped over the block on a change
 *  The settings belong to the slot, not to the effect: they stay when the effect in the slot is replaced. To place an
 *  effect without a click in audio that is playing, disable the slot first and enable it once the effect is in.
 *
 *  An empty or disabled slot costs one branch; a chain without effects leaves the audio exactly as it is.
 *
 *  Threads: the settings can be set from any thread. Prepare() is for the owner thread while the chain is not processed.
 *  SetSlotEffect() and Process() are for the thread that mixes (the audio thread; the owner thread while nothing is mixed).
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioEffectChain
{
    public:

    CAudioEffectChain( void );

    ~CAudioEffectChain();

    /**
     *  Allocates the slots and the scratch memory for a format and block size. Slots and their settings are kept
     *  when the capacity does not change (a new format for the same chain); the effects in them are not prepared here
     */
    bool Prepare( UInt32 slotCapacity, const CAudioFormat& format, UInt32 maxBlockFrames );

    /**
     *  Allocates the slots and their settings before the format is known, so that the settings can be set from any thread
     *  from then on: Prepare() with the same capacity keeps them (a voice does this when it is made). Owner thread, while the
     *  chain is not processed. Nothing is processed before Prepare()
     */
    void SetSlotCapacity( UInt32 slotCapacity );

    UInt32 GetSlotCapacity( void ) const;

    /** Mixing thread: puts an effect in a slot (null empties it). The effect must be prepared for the chain's format */
    bool SetSlotEffect( UInt32 slot, CAudioEffect* effect );

    /** Mixing thread: the effect in a slot */
    CAudioEffect* GetSlotEffect( UInt32 slot ) const;

    /** Mixing thread: number of slots holding an effect */
    UInt32 GetEffectCount( void ) const;

    /** Any thread */
    void SetEnabled( UInt32 slot, bool enabled );
    bool IsEnabled( UInt32 slot ) const;

    /** Any thread: wet and dry levels of a slot, linear */
    void SetMix( UInt32 slot, Float32 wet, Float32 dry );
    Float32 GetWet( UInt32 slot ) const;
    Float32 GetDry( UInt32 slot ) const;

    /** Mixing thread: passes interleaved float audio through the slots in order, in place */
    void Process( Float32* inOut, UInt32 frames );

    /** Mixing thread: clears the state of every effect and every fade */
    void Reset( void );

    private:

    CAudioEffectChain( const CAudioEffectChain& src );              /**< not implemented */
    CAudioEffectChain& operator=( const CAudioEffectChain& src );   /**< not implemented */

    struct SSlot
    {
        CAudioEffect* effect;
        MT::CTAtomic< bool > enabled;
        CAtomicFloat wet;
        CAtomicFloat dry;

        // mixing thread
        Float32 fade;               /**< how much of the processed signal is heard: 0 bypassed .. 1 fully */
        Float32 lastWet;
        Float32 lastDry;
        bool needsReset;

        SSlot( void );
    };

    void ProcessSlot( SSlot& slot, Float32* inOut, UInt32 frames );

    private:

    SSlot* m_slots;
    UInt32 m_slotCapacity;
    UInt32 m_effectCount;
    UInt32 m_channels;
    UInt32 m_maxBlockFrames;
    Float32 m_fadeStep;             /**< fade change per frame, 10 ms for a full crossfade */
    GUCEF::vector< Float32 > m_input;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOEFFECTCHAIN_H ? */
