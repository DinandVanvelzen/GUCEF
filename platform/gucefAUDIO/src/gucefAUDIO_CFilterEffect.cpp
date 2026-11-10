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

#include "gucefAUDIO_CFilterEffect.h"

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
CFilterEffect::GetStaticEffectTypeName( void )
{GUCEF_TRACE;

    static const CString typeName( "filter" );
    return typeName;
}

/*-------------------------------------------------------------------------*/

UInt32
CFilterEffect::GetStaticParameterCount( void )
{GUCEF_TRACE;

    return PARAMETER_COUNT;
}

/*-------------------------------------------------------------------------*/

const CAudioEffectParameterInfo*
CFilterEffect::GetStaticParameterInfos( void )
{GUCEF_TRACE;

    static const CAudioEffectParameterInfo infos[ PARAMETER_COUNT ] =
    {
        CAudioEffectParameterInfo( PARAMETER_TYPE, "type", CFilterBand::GetTypeLabels(), FILTERBANDTYPE_COUNT, (UInt32) FILTERBANDTYPE_LOWPASS ),
        CAudioEffectParameterInfo( PARAMETER_FREQUENCY, "frequency", AUDIOPARAMUNIT_HERTZ, AUDIOPARAMSCALE_LOGARITHMIC, 20.0f, 20000.0f, 1000.0f ),
        CAudioEffectParameterInfo( PARAMETER_Q, "q", AUDIOPARAMUNIT_NONE, AUDIOPARAMSCALE_LOGARITHMIC, 0.1f, 18.0f, 0.7071f ),
        CAudioEffectParameterInfo( PARAMETER_GAIN, "gain", AUDIOPARAMUNIT_DECIBEL, AUDIOPARAMSCALE_LINEAR, -24.0f, 24.0f, 0.0f ),
        CAudioEffectParameterInfo( PARAMETER_SLOPE, "slope", AUDIOPARAMUNIT_NONE, AUDIOPARAMSCALE_LINEAR, 12.0f, 24.0f, 12.0f, AUDIOPARAMKIND_INTEGER )
    };
    return infos;
}

/*-------------------------------------------------------------------------*/

CFilterEffect::CFilterEffect( const CORE::CValueList& options )
    : CAudioEffect( GetStaticParameterInfos(), PARAMETER_COUNT )
    , m_channels( 0 )
    , m_band()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFilterEffect::~CFilterEffect()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CString&
CFilterEffect::GetEffectTypeName( void ) const
{GUCEF_TRACE;

    return GetStaticEffectTypeName();
}

/*-------------------------------------------------------------------------*/

void
CFilterEffect::SetFilterType( TFilterBandType type )
{GUCEF_TRACE;

    SetParameter( PARAMETER_TYPE, (Float32) type );
}

/*-------------------------------------------------------------------------*/

void
CFilterEffect::SetFrequency( Float32 hertz )
{GUCEF_TRACE;

    SetParameter( PARAMETER_FREQUENCY, hertz );
}

/*-------------------------------------------------------------------------*/

void
CFilterEffect::SetQ( Float32 q )
{GUCEF_TRACE;

    SetParameter( PARAMETER_Q, q );
}

/*-------------------------------------------------------------------------*/

void
CFilterEffect::SetGainInDecibels( Float32 decibels )
{GUCEF_TRACE;

    SetParameter( PARAMETER_GAIN, decibels );
}

/*-------------------------------------------------------------------------*/

void
CFilterEffect::SetSlopeInDecibelsPerOctave( UInt32 slope )
{GUCEF_TRACE;

    SetParameter( PARAMETER_SLOPE, (Float32) slope );
}

/*-------------------------------------------------------------------------*/

bool
CFilterEffect::OnPrepare( const CAudioFormat& format, UInt32 maxBlockFrames )
{GUCEF_TRACE;

    m_channels = format.GetChannelCount();
    return m_band.Prepare( m_channels, (Float64) format.GetSampleRate() );
}

/*-------------------------------------------------------------------------*/

void
CFilterEffect::ApplyParameters( bool jump )
{
    const TFilterBandType type = (TFilterBandType) (Int32) ReadParameter( PARAMETER_TYPE );
    const bool steep = ReadParameter( PARAMETER_SLOPE ) >= 18.0f;
    const Float32 frequency = ReadParameter( PARAMETER_FREQUENCY );
    const Float32 q = ReadParameter( PARAMETER_Q );
    const Float32 gain = ReadParameter( PARAMETER_GAIN );
    if ( jump )
        m_band.Reset( true, type, steep, frequency, q, gain );
    else
        m_band.SetTarget( true, type, steep, frequency, q, gain );
}

/*-------------------------------------------------------------------------*/

void
CFilterEffect::OnReset( void )
{
    HaveParametersChanged();
    ApplyParameters( true );
}

/*-------------------------------------------------------------------------*/

void
CFilterEffect::OnProcess( Float32* inOut, UInt32 frames )
{
    if ( HaveParametersChanged() )
        ApplyParameters( false );

    const UInt32 channels = m_channels;
    UInt64 position = GetProcessedFrameCount();
    UInt32 done = 0;
    while ( done < frames )
    {
        const UInt32 inSubBlock = (UInt32) ( position % GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES );
        if ( 0 == inSubBlock )
            m_band.StepSubBlock();
        UInt32 run = GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES - inSubBlock;
        if ( run > frames - done )
            run = frames - done;

        m_band.ProcessRun( inOut + (size_t) done * channels, run, 0 );

        done += run;
        position += run;
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
