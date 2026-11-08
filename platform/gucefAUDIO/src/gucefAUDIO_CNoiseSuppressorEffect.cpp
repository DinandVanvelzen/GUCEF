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
#include <string.h>

#include "gucefAUDIO_CNoiseSuppressorEffect.h"

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

const CString&
CNoiseSuppressorEffect::GetStaticEffectTypeName( void )
{GUCEF_TRACE;

    static const CString typeName( "noiseSuppressor" );
    return typeName;
}

/*-------------------------------------------------------------------------*/

UInt32
CNoiseSuppressorEffect::GetStaticParameterCount( void )
{GUCEF_TRACE;

    return PARAMETER_COUNT;
}

/*-------------------------------------------------------------------------*/

const CAudioEffectParameterInfo*
CNoiseSuppressorEffect::GetStaticParameterInfos( void )
{GUCEF_TRACE;

    static const CAudioEffectParameterInfo infos[ PARAMETER_COUNT ] =
    {
        CAudioEffectParameterInfo( PARAMETER_AMOUNT, "amount", AUDIOPARAMUNIT_DECIBEL, AUDIOPARAMSCALE_LINEAR, -40.0f, 0.0f, -15.0f ),
        CAudioEffectParameterInfo( PARAMETER_STRENGTH, "strength", AUDIOPARAMUNIT_RATIO, AUDIOPARAMSCALE_LOGARITHMIC, 0.5f, 4.0f, 1.0f ),
        CAudioEffectParameterInfo( PARAMETER_BYPASS_BELOW_HZ, "bypassBelowHz", AUDIOPARAMUNIT_HERTZ, AUDIOPARAMSCALE_LINEAR, 0.0f, 1000.0f, 100.0f )
    };
    return infos;
}

/*-------------------------------------------------------------------------*/

CNoiseSuppressorEffect::CNoiseSuppressorEffect( const CORE::CValueList& )
    : CAudioEffect( GetStaticParameterInfos(), PARAMETER_COUNT )
    , m_suppressors()
    , m_channel()
    , m_channels( 0 )
    , m_latency( 0 )
    , m_publishedGainDb( 0.0f )
    , m_publishedTransformed( 0 )
    , m_publishedBypassed( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CNoiseSuppressorEffect::~CNoiseSuppressorEffect()
{GUCEF_TRACE;

    ReleaseSuppressors();
}

/*-------------------------------------------------------------------------*/

const CString&
CNoiseSuppressorEffect::GetEffectTypeName( void ) const
{GUCEF_TRACE;

    return GetStaticEffectTypeName();
}

/*-------------------------------------------------------------------------*/

void
CNoiseSuppressorEffect::SetAmountInDecibels( Float32 decibels )
{GUCEF_TRACE;

    SetParameter( PARAMETER_AMOUNT, decibels );
}

/*-------------------------------------------------------------------------*/

void
CNoiseSuppressorEffect::SetStrength( Float32 strength )
{GUCEF_TRACE;

    SetParameter( PARAMETER_STRENGTH, strength );
}

/*-------------------------------------------------------------------------*/

void
CNoiseSuppressorEffect::SetBypassBelowHz( Float32 frequency )
{GUCEF_TRACE;

    SetParameter( PARAMETER_BYPASS_BELOW_HZ, frequency );
}

/*-------------------------------------------------------------------------*/

UInt32
CNoiseSuppressorEffect::GetLatencyInFrames( void ) const
{GUCEF_TRACE;

    return m_latency;
}

/*-------------------------------------------------------------------------*/

Float32
CNoiseSuppressorEffect::GetGainInDecibels( void ) const
{GUCEF_TRACE;

    return m_publishedGainDb.Load();
}

/*-------------------------------------------------------------------------*/

UInt64
CNoiseSuppressorEffect::GetTransformedFrameCount( void ) const
{GUCEF_TRACE;

    return m_publishedTransformed.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

UInt64
CNoiseSuppressorEffect::GetBypassedFrameCount( void ) const
{GUCEF_TRACE;

    return m_publishedBypassed.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

const CSpectralSuppressor*
CNoiseSuppressorEffect::GetSuppressor( UInt32 channel ) const
{GUCEF_TRACE;

    return channel < m_suppressors.size() ? m_suppressors[ channel ] : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CNoiseSuppressorEffect::ReleaseSuppressors( void )
{GUCEF_TRACE;

    for ( size_t c=0; c<m_suppressors.size(); ++c )
        GUCEF_DELETE m_suppressors[ c ];
    m_suppressors.clear();
    GUCEF::vector< Float32 > empty;
    m_channel.swap( empty );
    m_latency = 0;
}

/*-------------------------------------------------------------------------*/

bool
CNoiseSuppressorEffect::OnPrepare( const CAudioFormat& format, UInt32 maxBlockFrames )
{GUCEF_TRACE;

    ReleaseSuppressors();
    m_channels = format.GetChannelCount();
    m_publishedTransformed.Store( 0, MT::MEMORDER_RELAXED );
    m_publishedBypassed.Store( 0, MT::MEMORDER_RELAXED );
    m_publishedGainDb.Store( 0.0f );

    // a rate the suppressor does not support leaves the effect without suppressors: it passes the audio unchanged
    if ( 0 == CSpectralSuppressor::FrameSizeForRate( format.GetSampleRate() ) )
        return true;
    for ( UInt32 c=0; c<m_channels; ++c )
    {
        CSpectralSuppressor* suppressor = GUCEF_NEW CSpectralSuppressor();
        suppressor->Prepare( format.GetSampleRate() );
        m_suppressors.push_back( suppressor );
    }
    if ( m_channels > 1 )
        m_channel.assign( maxBlockFrames, 0.0f );
    m_latency = m_suppressors.empty() ? 0 : m_suppressors[ 0 ]->GetLatencyInFrames();
    ApplyParameters();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CNoiseSuppressorEffect::ApplyParameters( void )
{
    const Float32 amount = ReadParameter( PARAMETER_AMOUNT );
    const Float32 floorGain = amount >= 0.0f ? 1.0f : (Float32) pow( 10.0, (Float64) amount / 20.0 );
    const Float32 strength = ReadParameter( PARAMETER_STRENGTH );
    const Float32 bypassBelow = ReadParameter( PARAMETER_BYPASS_BELOW_HZ );
    for ( size_t c=0; c<m_suppressors.size(); ++c )
    {
        CSpectralSuppressor* suppressor = m_suppressors[ c ];
        suppressor->SetNoiseSuppression( true );
        suppressor->SetNoiseFloorGain( floorGain );
        suppressor->SetNoiseStrength( strength );
        suppressor->SetBypassBelowHz( bypassBelow );
    }
}

/*-------------------------------------------------------------------------*/

void
CNoiseSuppressorEffect::OnReset( void )
{
    HaveParametersChanged();
    ApplyParameters();
    for ( size_t c=0; c<m_suppressors.size(); ++c )
        m_suppressors[ c ]->Reset();
    Publish();
}

/*-------------------------------------------------------------------------*/

void
CNoiseSuppressorEffect::Publish( void )
{
    UInt64 transformed = 0, bypassed = 0;
    for ( size_t c=0; c<m_suppressors.size(); ++c )
    {
        transformed += m_suppressors[ c ]->GetTransformedFrameCount();
        bypassed += m_suppressors[ c ]->GetBypassedFrameCount();
    }
    m_publishedTransformed.Store( transformed, MT::MEMORDER_RELAXED );
    m_publishedBypassed.Store( bypassed, MT::MEMORDER_RELAXED );
    m_publishedGainDb.Store( m_suppressors.empty() ? 0.0f : m_suppressors[ 0 ]->GetMeanGainInDecibels() );
}

/*-------------------------------------------------------------------------*/

void
CNoiseSuppressorEffect::OnProcess( Float32* inOut, UInt32 frames )
{
    if ( m_suppressors.empty() )
        return;
    if ( HaveParametersChanged() )
        ApplyParameters();

    const UInt32 channels = m_channels;
    if ( 1 == channels )
    {
        m_suppressors[ 0 ]->Process( inOut, frames );
    }
    else
    {
        // each channel on its own, through a scratch block
        Float32* channel = &m_channel[ 0 ];
        for ( UInt32 c=0; c<channels; ++c )
        {
            for ( UInt32 f=0; f<frames; ++f )
                channel[ f ] = inOut[ (size_t) f * channels + c ];
            m_suppressors[ c ]->Process( channel, frames );
            for ( UInt32 f=0; f<frames; ++f )
                inOut[ (size_t) f * channels + c ] = channel[ f ];
        }
    }
    Publish();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
