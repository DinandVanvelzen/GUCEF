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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefAUDIO_ETypes.h"

#include "gucefAUDIO_CEqualizerEffect.h"

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

struct SDefaultBand
{
    bool enabled;
    TFilterBandType type;
    Float32 frequency;
    Float32 q;
};

const SDefaultBand DefaultBandTable[ CEqualizerEffect::DefaultBands ] =
{
    { true  , FILTERBANDTYPE_LOWSHELF  ,    80.0f , 0.7071f },
    { true  , FILTERBANDTYPE_PEAK      ,   200.0f , 1.0f    },
    { true  , FILTERBANDTYPE_PEAK      ,   500.0f , 1.0f    },
    { true  , FILTERBANDTYPE_PEAK      ,  1000.0f , 1.0f    },
    { true  , FILTERBANDTYPE_PEAK      ,  2000.0f , 1.0f    },
    { true  , FILTERBANDTYPE_PEAK      ,  5000.0f , 1.0f    },
    { true  , FILTERBANDTYPE_HIGHSHELF , 10000.0f , 0.7071f },
    { false , FILTERBANDTYPE_HIGHPASS  ,    20.0f , 0.7071f }
};

const Float32 Graphic6[ 6 ] = { 32.0f, 100.0f, 320.0f, 1000.0f, 3200.0f, 10000.0f };
const Float32 Graphic10[ 10 ] = { 31.25f, 62.5f, 125.0f, 250.0f, 500.0f, 1000.0f, 2000.0f, 4000.0f, 8000.0f, 16000.0f };
const Float32 Graphic21[ 21 ] = { 22.0f, 32.0f, 44.0f, 63.0f, 90.0f, 125.0f, 175.0f, 250.0f, 350.0f, 500.0f, 700.0f, 1000.0f,
                                  1400.0f, 2000.0f, 2800.0f, 4000.0f, 5600.0f, 8000.0f, 11000.0f, 16000.0f, 20000.0f };

/** The center frequencies of a graphic preset, null for a name that is no preset */
const Float32*
GetPresetFrequencies( const CString& preset, UInt32& count )
{
    const CString name = preset.Lowercase();
    if ( name == "graphic6" )  { count = 6;  return Graphic6; }
    if ( name == "graphic10" ) { count = 10; return Graphic10; }
    if ( name == "graphic21" ) { count = 21; return Graphic21; }
    count = 0;
    return GUCEF_NULL;
}

UInt32
BandCountOf( const CORE::CValueList& options )
{
    UInt32 presetCount = 0;
    if ( GUCEF_NULL != GetPresetFrequencies( options.GetValueAlways( "preset" ).AsString(), presetCount ) )
        return presetCount;

    const Int32 bands = options.GetValueAlways( "bands" ).AsInt32( (Int32) CEqualizerEffect::DefaultBands );
    if ( bands < 1 )
        return 1;
    if ( bands > (Int32) CEqualizerEffect::MaxBands )
        return CEqualizerEffect::MaxBands;
    return (UInt32) bands;
}

/** Defaults of a band of the largest equalizer */
SDefaultBand
DefaultBandOf( UInt32 band )
{
    if ( band < CEqualizerEffect::DefaultBands )
        return DefaultBandTable[ band ];
    SDefaultBand extra;
    extra.enabled = false;
    extra.type = FILTERBANDTYPE_PEAK;
    extra.frequency = (Float32) ( 20.0 * pow( 1000.0, (Float64) ( band - CEqualizerEffect::DefaultBands + 1 ) / (Float64) ( CEqualizerEffect::MaxBandsOfPreset - CEqualizerEffect::DefaultBands + 1 ) ) );
    extra.q = 1.0f;
    return extra;
}

/** The Q at which peaking bands 'octaves' apart overlap evenly: 2^( bw / 2 ) / ( 2^bw - 1 ) */
Float32
QForSpacing( Float64 octaves )
{
    return (Float32) ( pow( 2.0, octaves / 2.0 ) / ( pow( 2.0, octaves ) - 1.0 ) );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString&
CEqualizerEffect::GetStaticEffectTypeName( void )
{GUCEF_TRACE;

    static const CString typeName( "equalizer" );
    return typeName;
}

/*-------------------------------------------------------------------------*/

UInt32
CEqualizerEffect::GetStaticParameterCount( void )
{GUCEF_TRACE;

    return DefaultBands * BANDFIELD_COUNT;
}

/*-------------------------------------------------------------------------*/

UInt32
CEqualizerEffect::GetParameterId( UInt32 band, EBandField field )
{
    return band * BANDFIELD_COUNT + (UInt32) field;
}

/*-------------------------------------------------------------------------*/

const CAudioEffectParameterInfo*
CEqualizerEffect::GetStaticParameterInfos( void )
{GUCEF_TRACE;

    static GUCEF::vector< CAudioEffectParameterInfo > infos;
    if ( infos.empty() )
    {
        for ( UInt32 band=0; band<MaxBandsOfPreset; ++band )
        {
            const CString name = "band" + CORE::UInt32ToString( band );
            const SDefaultBand defaults = DefaultBandOf( band );
            infos.push_back( CAudioEffectParameterInfo( GetParameterId( band, BANDFIELD_ENABLED ), name + "Enabled", AUDIOPARAMUNIT_NONE, AUDIOPARAMSCALE_LINEAR, 0.0f, 1.0f, defaults.enabled ? 1.0f : 0.0f, AUDIOPARAMKIND_BOOLEAN ) );
            infos.push_back( CAudioEffectParameterInfo( GetParameterId( band, BANDFIELD_TYPE ), name + "Type", CFilterBand::GetTypeLabels(), FILTERBANDTYPE_COUNT, (UInt32) defaults.type ) );
            infos.push_back( CAudioEffectParameterInfo( GetParameterId( band, BANDFIELD_FREQUENCY ), name + "Frequency", AUDIOPARAMUNIT_HERTZ, AUDIOPARAMSCALE_LOGARITHMIC, 20.0f, 20000.0f, defaults.frequency ) );
            infos.push_back( CAudioEffectParameterInfo( GetParameterId( band, BANDFIELD_GAIN ), name + "Gain", AUDIOPARAMUNIT_DECIBEL, AUDIOPARAMSCALE_LINEAR, -24.0f, 24.0f, 0.0f ) );
            infos.push_back( CAudioEffectParameterInfo( GetParameterId( band, BANDFIELD_Q ), name + "Q", AUDIOPARAMUNIT_NONE, AUDIOPARAMSCALE_LOGARITHMIC, 0.1f, 18.0f, defaults.q ) );
        }
    }
    return &infos[ 0 ];
}

/*-------------------------------------------------------------------------*/

CEqualizerEffect::CEqualizerEffect( const CORE::CValueList& options )
    : CAudioEffect( GetStaticParameterInfos(), BandCountOf( options ) * BANDFIELD_COUNT )
    , m_bandCount( BandCountOf( options ) )
    , m_channels( 0 )
    , m_passMask( 0 )
    , m_bands()
{GUCEF_TRACE;

    UInt32 presetCount = 0;
    const Float32* presetFrequencies = GetPresetFrequencies( options.GetValueAlways( "preset" ).AsString(), presetCount );
    if ( GUCEF_NULL != presetFrequencies )
    {
        // a graphic equalizer: every band a peak on its own center frequency, with a Q that fits the spacing
        const Float32 q = QForSpacing( log( (Float64) presetFrequencies[ presetCount - 1 ] / (Float64) presetFrequencies[ 0 ] ) / log( 2.0 ) / (Float64) ( presetCount - 1 ) );
        for ( UInt32 band=0; band<presetCount; ++band )
        {
            const UInt32 ids[ 5 ] = { GetParameterId( band, BANDFIELD_ENABLED ), GetParameterId( band, BANDFIELD_TYPE ), GetParameterId( band, BANDFIELD_FREQUENCY ), GetParameterId( band, BANDFIELD_GAIN ), GetParameterId( band, BANDFIELD_Q ) };
            const Float32 values[ 5 ] = { 1.0f, (Float32) FILTERBANDTYPE_PEAK, presetFrequencies[ band ], 0.0f, q };
            SetParameters( ids, values, 5 );
        }
    }
}

/*-------------------------------------------------------------------------*/

CEqualizerEffect::~CEqualizerEffect()
{GUCEF_TRACE;

    ReleaseBands();
}

/*-------------------------------------------------------------------------*/

const CString&
CEqualizerEffect::GetEffectTypeName( void ) const
{GUCEF_TRACE;

    return GetStaticEffectTypeName();
}

/*-------------------------------------------------------------------------*/

UInt32
CEqualizerEffect::GetBandCount( void ) const
{GUCEF_TRACE;

    return m_bandCount;
}

/*-------------------------------------------------------------------------*/

void
CEqualizerEffect::GetConstructionOptions( CORE::CValueList& options ) const
{GUCEF_TRACE;

    options.Set( "bands", (Int32) m_bandCount );
}

/*-------------------------------------------------------------------------*/

void
CEqualizerEffect::SetBand( UInt32 band, bool enabled, TFilterBandType type, Float32 frequency, Float32 gainInDecibels, Float32 q )
{GUCEF_TRACE;

    if ( band >= m_bandCount )
        return;
    const UInt32 ids[ 5 ] = { GetParameterId( band, BANDFIELD_ENABLED ), GetParameterId( band, BANDFIELD_TYPE ), GetParameterId( band, BANDFIELD_FREQUENCY ), GetParameterId( band, BANDFIELD_GAIN ), GetParameterId( band, BANDFIELD_Q ) };
    const Float32 values[ 5 ] = { enabled ? 1.0f : 0.0f, (Float32) type, frequency, gainInDecibels, q };
    SetParameters( ids, values, 5 );
}

/*-------------------------------------------------------------------------*/

void
CEqualizerEffect::ReleaseBands( void )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_bands.size(); ++i )
        GUCEF_DELETE m_bands[ i ];
    m_bands.clear();
}

/*-------------------------------------------------------------------------*/

bool
CEqualizerEffect::OnPrepare( const CAudioFormat& format, UInt32 maxBlockFrames )
{GUCEF_TRACE;

    m_channels = format.GetChannelCount();

    // the low frequency effects channel is the channel at the place of its speaker in the layout
    m_passMask = 0;
    const UInt32 layout = format.GetChannelLayout();
    if ( ( layout & AUDIOSPEAKER_LOW_FREQUENCY ) != 0 && m_channels <= 32 )
        m_passMask = 1U << CAudioFormat::GetChannelCountOfLayout( layout & ( AUDIOSPEAKER_LOW_FREQUENCY - 1 ) );

    ReleaseBands();
    for ( UInt32 band=0; band<m_bandCount; ++band )
    {
        CFilterBand* filterBand = GUCEF_NEW CFilterBand();
        m_bands.push_back( filterBand );
        if ( !filterBand->Prepare( m_channels, (Float64) format.GetSampleRate() ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CEqualizerEffect::ApplyParameters( bool jump )
{
    for ( UInt32 band=0; band<m_bandCount; ++band )
    {
        const bool enabled = ReadParameter( GetParameterId( band, BANDFIELD_ENABLED ) ) != 0.0f;
        const TFilterBandType type = (TFilterBandType) (Int32) ReadParameter( GetParameterId( band, BANDFIELD_TYPE ) );
        const Float32 frequency = ReadParameter( GetParameterId( band, BANDFIELD_FREQUENCY ) );
        const Float32 gain = ReadParameter( GetParameterId( band, BANDFIELD_GAIN ) );
        const Float32 q = ReadParameter( GetParameterId( band, BANDFIELD_Q ) );
        if ( jump )
            m_bands[ band ]->Reset( enabled, type, false, frequency, q, gain );
        else
            m_bands[ band ]->SetTarget( enabled, type, false, frequency, q, gain );
    }
}

/*-------------------------------------------------------------------------*/

void
CEqualizerEffect::OnReset( void )
{
    HaveParametersChanged();
    ApplyParameters( true );
}

/*-------------------------------------------------------------------------*/

void
CEqualizerEffect::OnProcess( Float32* inOut, UInt32 frames )
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
        {
            for ( UInt32 band=0; band<m_bandCount; ++band )
                m_bands[ band ]->StepSubBlock();
        }
        UInt32 run = GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES - inSubBlock;
        if ( run > frames - done )
            run = frames - done;

        Float32* start = inOut + (size_t) done * channels;
        for ( UInt32 band=0; band<m_bandCount; ++band )
            m_bands[ band ]->ProcessRun( start, run, m_passMask );

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
