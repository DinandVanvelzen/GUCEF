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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Time of a full crossfade when a slot is enabled or disabled */
const Float64 EnableCrossfadeInSeconds = 0.010;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAudioEffectChain::SSlot::SSlot( void )
    : effect( GUCEF_NULL )
    , enabled( true )
    , wet( 1.0f )
    , dry( 0.0f )
    , fade( 1.0f )
    , lastWet( 1.0f )
    , lastDry( 0.0f )
    , needsReset( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioEffectChain::CAudioEffectChain( void )
    : m_slots( GUCEF_NULL )
    , m_slotCapacity( 0 )
    , m_effectCount( 0 )
    , m_channels( 0 )
    , m_maxBlockFrames( 0 )
    , m_fadeStep( 1.0f )
    , m_input()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioEffectChain::~CAudioEffectChain()
{GUCEF_TRACE;

    GUCEF_DELETE[] m_slots;
    m_slots = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEffectChain::Prepare( UInt32 slotCapacity, const CAudioFormat& format, UInt32 maxBlockFrames )
{GUCEF_TRACE;

    if ( !format.IsValid() || 0 == maxBlockFrames )
        return false;

    if ( slotCapacity != m_slotCapacity )
    {
        GUCEF_DELETE[] m_slots;
        m_slots = slotCapacity > 0 ? GUCEF_NEW SSlot[ slotCapacity ] : GUCEF_NULL;
        m_slotCapacity = slotCapacity;
        m_effectCount = 0;
    }

    m_channels = format.GetChannelCount();
    m_maxBlockFrames = maxBlockFrames;
    const Float64 rate = format.GetSampleRate() > 0 ? (Float64) format.GetSampleRate() : 48000.0;
    m_fadeStep = (Float32) ( 1.0 / ( EnableCrossfadeInSeconds * rate ) );
    if ( m_slotCapacity > 0 )
        m_input.assign( (size_t) maxBlockFrames * m_channels, 0.0f );
    else
        m_input.clear();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAudioEffectChain::SetSlotCapacity( UInt32 slotCapacity )
{GUCEF_TRACE;

    if ( slotCapacity == m_slotCapacity )
        return;
    GUCEF_DELETE[] m_slots;
    m_slots = slotCapacity > 0 ? GUCEF_NEW SSlot[ slotCapacity ] : GUCEF_NULL;
    m_slotCapacity = slotCapacity;
    m_effectCount = 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioEffectChain::GetSlotCapacity( void ) const
{GUCEF_TRACE;

    return m_slotCapacity;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEffectChain::SetSlotEffect( UInt32 slot, CAudioEffect* effect )
{
    if ( slot >= m_slotCapacity )
        return false;

    SSlot& s = m_slots[ slot ];
    if ( GUCEF_NULL != s.effect )
        --m_effectCount;
    if ( GUCEF_NULL != effect )
        ++m_effectCount;
    s.effect = effect;

    // a new effect starts from a clean state, heard at once when the slot is enabled
    s.fade = s.enabled.Load( MT::MEMORDER_RELAXED ) ? 1.0f : 0.0f;
    s.lastWet = s.wet.Load();
    s.lastDry = s.dry.Load();
    s.needsReset = true;
    return true;
}

/*-------------------------------------------------------------------------*/

CAudioEffect*
CAudioEffectChain::GetSlotEffect( UInt32 slot ) const
{
    return slot < m_slotCapacity ? m_slots[ slot ].effect : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioEffectChain::GetEffectCount( void ) const
{
    return m_effectCount;
}

/*-------------------------------------------------------------------------*/

void
CAudioEffectChain::SetEnabled( UInt32 slot, bool enabled )
{GUCEF_TRACE;

    if ( slot < m_slotCapacity )
        m_slots[ slot ].enabled.Store( enabled, MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

bool
CAudioEffectChain::IsEnabled( UInt32 slot ) const
{GUCEF_TRACE;

    return slot < m_slotCapacity && m_slots[ slot ].enabled.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

void
CAudioEffectChain::SetMix( UInt32 slot, Float32 wet, Float32 dry )
{GUCEF_TRACE;

    if ( slot >= m_slotCapacity )
        return;
    m_slots[ slot ].wet.Store( wet );
    m_slots[ slot ].dry.Store( dry );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioEffectChain::GetWet( UInt32 slot ) const
{GUCEF_TRACE;

    return slot < m_slotCapacity ? m_slots[ slot ].wet.Load() : 0.0f;
}

/*-------------------------------------------------------------------------*/

Float32
CAudioEffectChain::GetDry( UInt32 slot ) const
{GUCEF_TRACE;

    return slot < m_slotCapacity ? m_slots[ slot ].dry.Load() : 0.0f;
}

/*-------------------------------------------------------------------------*/

void
CAudioEffectChain::Process( Float32* inOut, UInt32 frames )
{
    if ( 0 == m_effectCount || 0 == frames || frames > m_maxBlockFrames )
        return;

    for ( UInt32 s=0; s<m_slotCapacity; ++s )
        if ( GUCEF_NULL != m_slots[ s ].effect )
            ProcessSlot( m_slots[ s ], inOut, frames );
}

/*-------------------------------------------------------------------------*/

void
CAudioEffectChain::ProcessSlot( SSlot& slot, Float32* inOut, UInt32 frames )
{
    const bool on = slot.enabled.Load( MT::MEMORDER_RELAXED );
    if ( !on && slot.fade <= 0.0f )
    {
        // bypassed: not processed, and cleared before it is heard again
        slot.needsReset = true;
        return;
    }
    if ( slot.needsReset )
    {
        slot.effect->Reset();
        slot.needsReset = false;
    }

    const Float32 wet = slot.wet.Load();
    const Float32 dry = slot.dry.Load();
    const Float32 fromWet = slot.lastWet;
    const Float32 fromDry = slot.lastDry;
    const bool fading = !on || slot.fade < 1.0f;
    const bool mixChanges = ( wet != fromWet || dry != fromDry );

    // the usual case: fully on, all wet, nothing changing. The effect works on the audio directly
    if ( !fading && !mixChanges && 1.0f == wet && 0.0f == dry )
    {
        slot.effect->Process( inOut, frames );
        return;
    }

    const UInt32 channels = m_channels;
    const size_t samples = (size_t) frames * channels;
    Float32* input = &m_input[ 0 ];
    memcpy( input, inOut, samples * sizeof( Float32 ) );
    slot.effect->Process( inOut, frames );

    const Float32 inverseFrames = 1.0f / (Float32) frames;
    if ( !fading )
    {
        if ( !mixChanges )
        {
            for ( size_t i=0; i<samples; ++i )
                inOut[ i ] = wet * inOut[ i ] + dry * input[ i ];
        }
        else
        {
            for ( UInt32 f=0; f<frames; ++f )
            {
                const Float32 t = (Float32) ( f + 1 ) * inverseFrames;
                const Float32 w = fromWet + ( wet - fromWet ) * t;
                const Float32 d = fromDry + ( dry - fromDry ) * t;
                for ( UInt32 c=0; c<channels; ++c )
                {
                    const size_t i = (size_t) f * channels + c;
                    inOut[ i ] = w * inOut[ i ] + d * input[ i ];
                }
            }
        }
    }
    else
    {
        // crossfade between the unprocessed and the processed signal, frame by frame
        const Float32 step = on ? m_fadeStep : -m_fadeStep;
        Float32 fade = slot.fade;
        for ( UInt32 f=0; f<frames; ++f )
        {
            fade += step;
            if ( fade > 1.0f ) fade = 1.0f;
            if ( fade < 0.0f ) fade = 0.0f;
            const Float32 t = (Float32) ( f + 1 ) * inverseFrames;
            const Float32 w = fromWet + ( wet - fromWet ) * t;
            const Float32 d = fromDry + ( dry - fromDry ) * t;
            for ( UInt32 c=0; c<channels; ++c )
            {
                const size_t i = (size_t) f * channels + c;
                const Float32 x = input[ i ];
                const Float32 mixed = w * inOut[ i ] + d * x;
                inOut[ i ] = x + fade * ( mixed - x );
            }
        }
        slot.fade = fade;
        if ( fade <= 0.0f )
            slot.needsReset = true;
    }
    slot.lastWet = wet;
    slot.lastDry = dry;
}

/*-------------------------------------------------------------------------*/

void
CAudioEffectChain::Reset( void )
{
    for ( UInt32 s=0; s<m_slotCapacity; ++s )
    {
        SSlot& slot = m_slots[ s ];
        slot.fade = slot.enabled.Load( MT::MEMORDER_RELAXED ) ? 1.0f : 0.0f;
        slot.lastWet = slot.wet.Load();
        slot.lastDry = slot.dry.Load();
        slot.needsReset = GUCEF_NULL != slot.effect;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
