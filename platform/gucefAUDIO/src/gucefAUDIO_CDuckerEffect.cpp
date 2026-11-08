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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_AUDIO_CAUDIOBUS_H
#include "gucefAUDIO_CAudioBus.h"
#define GUCEF_AUDIO_CAUDIOBUS_H
#endif /* GUCEF_AUDIO_CAUDIOBUS_H ? */

#ifndef GUCEF_AUDIO_CAUDIOENGINE_H
#include "gucefAUDIO_CAudioEngine.h"
#define GUCEF_AUDIO_CAUDIOENGINE_H
#endif /* GUCEF_AUDIO_CAUDIOENGINE_H ? */

#include "gucefAUDIO_CDuckerEffect.h"

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

/** log2( 10 ) / 20: the exponent of 2 per decibel */
const Float32 Log2PerDecibel = 0.16609640474436813f;

/** Smoothing time of a change of the depth */
const Float64 DepthSmoothingSeconds = 0.020;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString&
CDuckerEffect::GetStaticEffectTypeName( void )
{GUCEF_TRACE;

    static const CString typeName( "ducker" );
    return typeName;
}

/*-------------------------------------------------------------------------*/

UInt32
CDuckerEffect::GetStaticParameterCount( void )
{GUCEF_TRACE;

    return PARAMETER_COUNT;
}

/*-------------------------------------------------------------------------*/

const CAudioEffectParameterInfo*
CDuckerEffect::GetStaticParameterInfos( void )
{GUCEF_TRACE;

    static const char* const keySourceLabels[ 2 ] = { "peak", "rms" };
    static const CAudioEffectParameterInfo infos[ PARAMETER_COUNT ] =
    {
        CAudioEffectParameterInfo( PARAMETER_DEPTH, "depth", AUDIOPARAMUNIT_DECIBEL, AUDIOPARAMSCALE_LINEAR, -60.0f, 0.0f, -18.0f ),
        CAudioEffectParameterInfo( PARAMETER_THRESHOLD, "threshold", AUDIOPARAMUNIT_DECIBEL, AUDIOPARAMSCALE_LINEAR, -80.0f, 0.0f, -45.0f ),
        CAudioEffectParameterInfo( PARAMETER_ATTACK, "attack", AUDIOPARAMUNIT_MILLISECONDS, AUDIOPARAMSCALE_LOGARITHMIC, 1.0f, 500.0f, 20.0f ),
        CAudioEffectParameterInfo( PARAMETER_HOLD, "hold", AUDIOPARAMUNIT_MILLISECONDS, AUDIOPARAMSCALE_LINEAR, 0.0f, 5000.0f, 300.0f ),
        CAudioEffectParameterInfo( PARAMETER_RELEASE, "release", AUDIOPARAMUNIT_MILLISECONDS, AUDIOPARAMSCALE_LOGARITHMIC, 10.0f, 5000.0f, 500.0f ),
        CAudioEffectParameterInfo( PARAMETER_KEY_SOURCE, "keySource", keySourceLabels, 2, KEYSOURCE_PEAK )
    };
    return infos;
}

/*-------------------------------------------------------------------------*/

CDuckerEffect::CDuckerEffect( const CORE::CValueList& options )
    : CAudioEffect( GetStaticParameterInfos(), PARAMETER_COUNT )
    , m_keyBusName( options.GetValueAlways( "keyBus" ).AsString() )
    , m_keyBus( GUCEF_NULL )
    , m_manualKey( false )
    , m_gainDb( 0.0f )
    , m_channels( 0 )
    , m_sampleRate( 48000.0 )
    , m_depth()
    , m_thresholdLinear( 0.0f )
    , m_useRms( false )
    , m_attackStep( 1.0f )
    , m_releaseStep( 1.0f )
    , m_holdFrames( 0 )
    , m_keyWanted( false )
    , m_position( 0.0f )
    , m_holdLeft( 0 )
    , m_gain( 1.0f )
    , m_gainEnd( 1.0f )
    , m_gainStep( 0.0f )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDuckerEffect::~CDuckerEffect()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CString&
CDuckerEffect::GetEffectTypeName( void ) const
{GUCEF_TRACE;

    return GetStaticEffectTypeName();
}

/*-------------------------------------------------------------------------*/

void
CDuckerEffect::SetDepthInDecibels( Float32 decibels )
{GUCEF_TRACE;

    SetParameter( PARAMETER_DEPTH, decibels );
}

/*-------------------------------------------------------------------------*/

void
CDuckerEffect::SetThresholdInDecibels( Float32 decibels )
{GUCEF_TRACE;

    SetParameter( PARAMETER_THRESHOLD, decibels );
}

/*-------------------------------------------------------------------------*/

void
CDuckerEffect::SetAttackInMs( Float32 milliseconds )
{GUCEF_TRACE;

    SetParameter( PARAMETER_ATTACK, milliseconds );
}

/*-------------------------------------------------------------------------*/

void
CDuckerEffect::SetHoldInMs( Float32 milliseconds )
{GUCEF_TRACE;

    SetParameter( PARAMETER_HOLD, milliseconds );
}

/*-------------------------------------------------------------------------*/

void
CDuckerEffect::SetReleaseInMs( Float32 milliseconds )
{GUCEF_TRACE;

    SetParameter( PARAMETER_RELEASE, milliseconds );
}

/*-------------------------------------------------------------------------*/

void
CDuckerEffect::SetKeySource( EKeySource source )
{GUCEF_TRACE;

    SetParameter( PARAMETER_KEY_SOURCE, (Float32) source );
}

/*-------------------------------------------------------------------------*/

void
CDuckerEffect::SetKeyBus( const CAudioBus* bus )
{GUCEF_TRACE;

    m_keyBus = bus;
    m_keyBusName = GUCEF_NULL != bus ? bus->GetName() : CString::Empty;
}

/*-------------------------------------------------------------------------*/

const CString&
CDuckerEffect::GetKeyBusName( void ) const
{GUCEF_TRACE;

    return m_keyBusName;
}

/*-------------------------------------------------------------------------*/

void
CDuckerEffect::SetManualKey( bool active )
{

    m_manualKey.Store( active, MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

bool
CDuckerEffect::IsManualKeyActive( void ) const
{GUCEF_TRACE;

    return m_manualKey.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

Float32
CDuckerEffect::GetGainInDecibels( void ) const
{GUCEF_TRACE;

    return m_gainDb.Load();
}

/*-------------------------------------------------------------------------*/

void
CDuckerEffect::GetConstructionOptions( CORE::CValueList& options ) const
{GUCEF_TRACE;

    if ( !m_keyBusName.IsNULLOrEmpty() )
        options.Set( "keyBus", m_keyBusName );
}

/*-------------------------------------------------------------------------*/

bool
CDuckerEffect::ResolveReferences( CAudioEngine& engine )
{GUCEF_TRACE;

    if ( m_keyBusName.IsNULLOrEmpty() )
        return true;

    const CAudioBus* bus = engine.FindBus( m_keyBusName );
    if ( GUCEF_NULL == bus )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DuckerEffect: The key bus \"" << m_keyBusName << "\" does not exist in the engine" );
        return false;
    }
    m_keyBus = bus;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDuckerEffect::OnPrepare( const CAudioFormat& format, UInt32 )
{GUCEF_TRACE;

    m_channels = format.GetChannelCount();
    m_sampleRate = (Float64) format.GetSampleRate();
    m_depth.Prepare( AUDIOSMOOTHING_ONE_POLE, DepthSmoothingSeconds, m_sampleRate, false, GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDuckerEffect::OnReset( void )
{
    HaveParametersChanged();
    ApplyParameters( true );
    m_keyWanted = false;
    m_position = 0.0f;
    m_holdLeft = 0;
    m_gain = 1.0f;
    m_gainEnd = 1.0f;
    m_gainStep = 0.0f;
    m_gainDb.Store( 0.0f );
}

/*-------------------------------------------------------------------------*/

void
CDuckerEffect::ApplyParameters( bool jump )
{
    if ( jump )
        m_depth.Reset( ReadParameter( PARAMETER_DEPTH ) );
    else
        m_depth.SetTarget( ReadParameter( PARAMETER_DEPTH ) );

    m_thresholdLinear = (Float32) pow( 10.0, (Float64) ReadParameter( PARAMETER_THRESHOLD ) / 20.0 );
    m_useRms = KEYSOURCE_RMS == (EKeySource) (Int32) ReadParameter( PARAMETER_KEY_SOURCE );

    // position change per sub-block: a full change takes the attack or release time
    const Float64 subBlocksPerMs = m_sampleRate / ( 1000.0 * (Float64) GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES );
    Float64 attackSteps = (Float64) ReadParameter( PARAMETER_ATTACK ) * subBlocksPerMs;
    Float64 releaseSteps = (Float64) ReadParameter( PARAMETER_RELEASE ) * subBlocksPerMs;
    m_attackStep = attackSteps > 1.0 ? (Float32) ( 1.0 / attackSteps ) : 1.0f;
    m_releaseStep = releaseSteps > 1.0 ? (Float32) ( 1.0 / releaseSteps ) : 1.0f;
    m_holdFrames = (UInt32) ( (Float64) ReadParameter( PARAMETER_HOLD ) * m_sampleRate / 1000.0 + 0.5 );
}

/*-------------------------------------------------------------------------*/

bool
CDuckerEffect::IsDuckingWanted( void ) const
{
    if ( m_manualKey.Load( MT::MEMORDER_RELAXED ) )
        return true;
    if ( GUCEF_NULL == m_keyBus )
        return false;

    const CAudioLevelMeter& meter = m_keyBus->GetMeter();
    const UInt32 channels = meter.GetChannelCount();
    Float32 level = 0.0f;
    for ( UInt32 c=0; c<channels; ++c )
    {
        const Float32 channelLevel = m_useRms ? meter.GetRms( c ) : meter.GetPeak( c );
        if ( channelLevel > level )
            level = channelLevel;
    }
    return level > m_thresholdLinear;
}

/*-------------------------------------------------------------------------*/

void
CDuckerEffect::StepSubBlock( void )
{
    // the ramp of the previous sub-block has arrived
    m_gain = m_gainEnd;

    if ( m_keyWanted )
    {
        m_holdLeft = m_holdFrames;
        m_position += m_attackStep;
        if ( m_position > 1.0f )
            m_position = 1.0f;
    }
    else
    if ( m_holdLeft > 0 )
    {
        m_holdLeft = m_holdLeft > GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES ? m_holdLeft - GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES : 0;
    }
    else
    {
        m_position -= m_releaseStep;
        if ( m_position < 0.0f )
            m_position = 0.0f;
    }

    const Float32 depth = m_depth.IsSmoothing() ? m_depth.Step() : m_depth.GetValue();
    m_gainEnd = m_position > 0.0f ? exp2f( depth * m_position * Log2PerDecibel ) : 1.0f;
    m_gainStep = ( m_gainEnd - m_gain ) / (Float32) GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES;
}

/*-------------------------------------------------------------------------*/

void
CDuckerEffect::OnProcess( Float32* inOut, UInt32 frames )
{
    if ( HaveParametersChanged() )
        ApplyParameters( false );

    // the key is read once per block
    m_keyWanted = IsDuckingWanted();

    const UInt32 channels = m_channels;
    UInt64 position = GetProcessedFrameCount();
    UInt32 done = 0;
    while ( done < frames )
    {
        const UInt32 inSubBlock = (UInt32) ( position % GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES );
        if ( 0 == inSubBlock )
            StepSubBlock();
        UInt32 run = GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES - inSubBlock;
        if ( run > frames - done )
            run = frames - done;

        if ( 1.0f != m_gain || 1.0f != m_gainEnd )
        {
            Float32 gain = m_gain;
            const Float32 step = m_gainStep;
            Float32* frame = inOut + (size_t) done * channels;
            for ( UInt32 f=0; f<run; ++f )
            {
                gain += step;
                for ( UInt32 c=0; c<channels; ++c )
                    frame[ c ] *= gain;
                frame += channels;
            }
            m_gain = gain;
        }

        done += run;
        position += run;
    }

    // at the end of the block: the gain at the end of the ramp that is under way
    m_gainDb.Store( m_gainEnd < 1.0f ? 20.0f * log10f( m_gainEnd ) : 0.0f );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
