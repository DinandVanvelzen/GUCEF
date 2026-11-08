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

#include <math.h>

#include "gucefAUDIO_CAudioBus.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAudioBus::CAudioBus( const CString& name, CAudioBus* parent )
    : m_name( name )
    , m_parent( parent )
    , m_volume( 1.0f )
    , m_muted( false )
    , m_effects()
    , m_sends( GUCEF_NULL )
    , m_sendCapacity( 0 )
    , m_meter()
    , m_buffer()
    , m_lastGain( 1.0f )
    , m_gainKnown( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioBus::~CAudioBus()
{GUCEF_TRACE;

    GUCEF_DELETE[] m_sends;
    m_sends = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CAudioBus::SSend::SSend( void )
    : target( GUCEF_NULL )
    , preFader( false )
    , level( 1.0f )
    , lastLevel( 1.0f )
    , levelKnown( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAudioBus::PrepareSends( UInt32 sendCapacity )
{GUCEF_TRACE;

    if ( sendCapacity == m_sendCapacity )
        return;
    GUCEF_DELETE[] m_sends;
    m_sends = sendCapacity > 0 ? GUCEF_NEW SSend[ sendCapacity ] : GUCEF_NULL;
    m_sendCapacity = sendCapacity;
}

/*-------------------------------------------------------------------------*/

const CString&
CAudioBus::GetName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

CAudioBus*
CAudioBus::GetParent( void ) const
{
    return m_parent;
}

/*-------------------------------------------------------------------------*/

void
CAudioBus::SetVolume( Float32 volume )
{GUCEF_TRACE;

    m_volume.Store( volume < 0.0f ? 0.0f : volume );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioBus::GetVolume( void ) const
{GUCEF_TRACE;

    return m_volume.Load();
}

/*-------------------------------------------------------------------------*/

void
CAudioBus::SetVolumeInDecibels( Float32 decibels )
{GUCEF_TRACE;

    SetVolume( DecibelsToLinear( decibels ) );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioBus::GetVolumeInDecibels( void ) const
{GUCEF_TRACE;

    return LinearToDecibels( m_volume.Load() );
}

/*-------------------------------------------------------------------------*/

void
CAudioBus::SetMuted( bool muted )
{GUCEF_TRACE;

    m_muted.Store( muted, MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

bool
CAudioBus::IsMuted( void ) const
{
    return m_muted.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioBus::GetEffectiveVolume( void ) const
{
    return IsMuted() ? 0.0f : m_volume.Load();
}

/*-------------------------------------------------------------------------*/

Float32
CAudioBus::DecibelsToLinear( Float32 decibels )
{GUCEF_TRACE;

    return (Float32) pow( 10.0, (Float64) decibels / 20.0 );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioBus::LinearToDecibels( Float32 linear )
{GUCEF_TRACE;

    return linear <= 0.0f ? -1000.0f : (Float32) ( 20.0 * log10( (Float64) linear ) );
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioBus::GetEffectSlotCount( void ) const
{GUCEF_TRACE;

    return m_effects.GetSlotCapacity();
}

/*-------------------------------------------------------------------------*/

void
CAudioBus::SetEffectEnabled( UInt32 slot, bool enabled )
{GUCEF_TRACE;

    m_effects.SetEnabled( slot, enabled );
}

/*-------------------------------------------------------------------------*/

bool
CAudioBus::IsEffectEnabled( UInt32 slot ) const
{GUCEF_TRACE;

    return m_effects.IsEnabled( slot );
}

/*-------------------------------------------------------------------------*/

void
CAudioBus::SetEffectMix( UInt32 slot, Float32 wet, Float32 dry )
{GUCEF_TRACE;

    m_effects.SetMix( slot, wet, dry );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioBus::GetEffectWet( UInt32 slot ) const
{GUCEF_TRACE;

    return m_effects.GetWet( slot );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioBus::GetEffectDry( UInt32 slot ) const
{GUCEF_TRACE;

    return m_effects.GetDry( slot );
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioBus::GetSendSlotCount( void ) const
{GUCEF_TRACE;

    return m_sendCapacity;
}

/*-------------------------------------------------------------------------*/

void
CAudioBus::SetSendLevel( UInt32 sendIndex, Float32 level )
{GUCEF_TRACE;

    if ( sendIndex < m_sendCapacity )
        m_sends[ sendIndex ].level.Store( level < 0.0f ? 0.0f : level );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioBus::GetSendLevel( UInt32 sendIndex ) const
{GUCEF_TRACE;

    return sendIndex < m_sendCapacity ? m_sends[ sendIndex ].level.Load() : 0.0f;
}

/*-------------------------------------------------------------------------*/

const CAudioLevelMeter&
CAudioBus::GetMeter( void ) const
{GUCEF_TRACE;

    return m_meter;
}


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
