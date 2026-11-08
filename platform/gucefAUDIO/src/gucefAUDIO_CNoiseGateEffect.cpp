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

#include "gucefAUDIO_CNoiseGateEffect.h"

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

/** Smoothing time of a change of the range */
const Float64 RangeSmoothingSeconds = 0.020;

/** Time constant of the peak envelope fall and of the mean square average of the level detector */
const Float64 LevelDetectorSeconds = 0.010;

/** The level of silence for the level detector, in dB */
const Float32 SilenceInDecibels = -200.0f;

const char* const VadOnsetOption = "vadOnsetDb";
const char* const VadReleaseOption = "vadReleaseDb";
const char* const VadHangoverOption = "vadHangoverMs";
const char* const VadMinimumLevelOption = "vadMinimumLevelDb";
const char* const VadFloorRiseOption = "vadFloorRiseDbPerSecond";

/** The voice detector settings of the options; what they do not name keeps the default */
void
ReadVoiceSettings( const CORE::CValueList& options, CVoiceActivitySettings& settings )
{
    // the release margin is limited by the onset margin, so the onset goes first
    settings.SetOnsetInDecibels( options.GetValueAlways( VadOnsetOption ).AsFloat32( settings.GetOnsetInDecibels() ) );
    settings.SetReleaseInDecibels( options.GetValueAlways( VadReleaseOption ).AsFloat32( settings.GetReleaseInDecibels() ) );
    settings.SetHangoverInMs( options.GetValueAlways( VadHangoverOption ).AsFloat32( settings.GetHangoverInMs() ) );
    settings.SetMinimumLevelInDecibels( options.GetValueAlways( VadMinimumLevelOption ).AsFloat32( settings.GetMinimumLevelInDecibels() ) );
    settings.SetFloorRiseInDecibelsPerSecond( options.GetValueAlways( VadFloorRiseOption ).AsFloat32( settings.GetFloorRiseInDecibelsPerSecond() ) );
}

/** The settings that differ from the defaults, as options */
void
WriteVoiceSettings( const CVoiceActivitySettings& settings, CORE::CValueList& options )
{
    const CVoiceActivitySettings defaults;
    if ( settings.GetOnsetInDecibels() != defaults.GetOnsetInDecibels() )
        options.Set( VadOnsetOption, settings.GetOnsetInDecibels() );
    if ( settings.GetReleaseInDecibels() != defaults.GetReleaseInDecibels() )
        options.Set( VadReleaseOption, settings.GetReleaseInDecibels() );
    if ( settings.GetHangoverInMs() != defaults.GetHangoverInMs() )
        options.Set( VadHangoverOption, settings.GetHangoverInMs() );
    if ( settings.GetMinimumLevelInDecibels() != defaults.GetMinimumLevelInDecibels() )
        options.Set( VadMinimumLevelOption, settings.GetMinimumLevelInDecibels() );
    if ( settings.GetFloorRiseInDecibelsPerSecond() != defaults.GetFloorRiseInDecibelsPerSecond() )
        options.Set( VadFloorRiseOption, settings.GetFloorRiseInDecibelsPerSecond() );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString&
CNoiseGateEffect::GetStaticEffectTypeName( void )
{GUCEF_TRACE;

    static const CString typeName( "noiseGate" );
    return typeName;
}

/*-------------------------------------------------------------------------*/

UInt32
CNoiseGateEffect::GetStaticParameterCount( void )
{GUCEF_TRACE;

    return PARAMETER_COUNT;
}

/*-------------------------------------------------------------------------*/

const CAudioEffectParameterInfo*
CNoiseGateEffect::GetStaticParameterInfos( void )
{GUCEF_TRACE;

    static const char* const detectorLabels[ 2 ] = { "level", "voice" };
    static const char* const levelSourceLabels[ 2 ] = { "peak", "rms" };
    static const CAudioEffectParameterInfo infos[ PARAMETER_COUNT ] =
    {
        CAudioEffectParameterInfo( PARAMETER_DETECTOR, "detector", detectorLabels, 2, DETECTOR_VOICE ),
        CAudioEffectParameterInfo( PARAMETER_THRESHOLD, "threshold", AUDIOPARAMUNIT_DECIBEL, AUDIOPARAMSCALE_LINEAR, -90.0f, 0.0f, -50.0f ),
        CAudioEffectParameterInfo( PARAMETER_HYSTERESIS, "hysteresis", AUDIOPARAMUNIT_DECIBEL, AUDIOPARAMSCALE_LINEAR, 0.0f, 12.0f, 4.0f ),
        CAudioEffectParameterInfo( PARAMETER_RANGE, "range", AUDIOPARAMUNIT_DECIBEL, AUDIOPARAMSCALE_LINEAR, -80.0f, 0.0f, -30.0f ),
        CAudioEffectParameterInfo( PARAMETER_ATTACK, "attack", AUDIOPARAMUNIT_MILLISECONDS, AUDIOPARAMSCALE_LOGARITHMIC, 0.1f, 50.0f, 2.0f ),
        CAudioEffectParameterInfo( PARAMETER_HOLD, "hold", AUDIOPARAMUNIT_MILLISECONDS, AUDIOPARAMSCALE_LINEAR, 0.0f, 1000.0f, 150.0f ),
        CAudioEffectParameterInfo( PARAMETER_RELEASE, "release", AUDIOPARAMUNIT_MILLISECONDS, AUDIOPARAMSCALE_LOGARITHMIC, 10.0f, 2000.0f, 150.0f ),
        CAudioEffectParameterInfo( PARAMETER_LEVEL_SOURCE, "levelSource", levelSourceLabels, 2, LEVELSOURCE_RMS )
    };
    return infos;
}

/*-------------------------------------------------------------------------*/

CNoiseGateEffect::CNoiseGateEffect( const CORE::CValueList& options )
    : CAudioEffect( GetStaticParameterInfos(), PARAMETER_COUNT )
    , m_voiceSettings()
    , m_publishedOpen( false )
    , m_publishedGainDb( 0.0f )
    , m_channels( 0 )
    , m_sampleRate( 48000.0 )
    , m_voice()
    , m_voiceReady( false )
    , m_useVoice( false )
    , m_useRms( true )
    , m_range()
    , m_thresholdInDecibels( -50.0f )
    , m_closeInDecibels( -54.0f )
    , m_attackStep( 1.0f )
    , m_releaseStep( 1.0f )
    , m_holdFrames( 0 )
    , m_peakFall( 0.0 )
    , m_squareCoefficient( 0.0 )
    , m_peakEnvelope( 0.0 )
    , m_meanSquare( 0.0 )
    , m_open( false )
    , m_holdLeft( 0 )
    , m_position( 0.0f )
    , m_gain( 1.0f )
    , m_gainEnd( 1.0f )
    , m_gainStep( 0.0f )
{GUCEF_TRACE;

    ReadVoiceSettings( options, m_voiceSettings );
}

/*-------------------------------------------------------------------------*/

CNoiseGateEffect::~CNoiseGateEffect()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CString&
CNoiseGateEffect::GetEffectTypeName( void ) const
{GUCEF_TRACE;

    return GetStaticEffectTypeName();
}

/*-------------------------------------------------------------------------*/

void
CNoiseGateEffect::SetDetector( EDetector detector )
{GUCEF_TRACE;

    SetParameter( PARAMETER_DETECTOR, (Float32) detector );
}

/*-------------------------------------------------------------------------*/

void
CNoiseGateEffect::SetThresholdInDecibels( Float32 decibels )
{GUCEF_TRACE;

    SetParameter( PARAMETER_THRESHOLD, decibels );
}

/*-------------------------------------------------------------------------*/

void
CNoiseGateEffect::SetHysteresisInDecibels( Float32 decibels )
{GUCEF_TRACE;

    SetParameter( PARAMETER_HYSTERESIS, decibels );
}

/*-------------------------------------------------------------------------*/

void
CNoiseGateEffect::SetRangeInDecibels( Float32 decibels )
{GUCEF_TRACE;

    SetParameter( PARAMETER_RANGE, decibels );
}

/*-------------------------------------------------------------------------*/

void
CNoiseGateEffect::SetAttackInMs( Float32 milliseconds )
{GUCEF_TRACE;

    SetParameter( PARAMETER_ATTACK, milliseconds );
}

/*-------------------------------------------------------------------------*/

void
CNoiseGateEffect::SetHoldInMs( Float32 milliseconds )
{GUCEF_TRACE;

    SetParameter( PARAMETER_HOLD, milliseconds );
}

/*-------------------------------------------------------------------------*/

void
CNoiseGateEffect::SetReleaseInMs( Float32 milliseconds )
{GUCEF_TRACE;

    SetParameter( PARAMETER_RELEASE, milliseconds );
}

/*-------------------------------------------------------------------------*/

void
CNoiseGateEffect::SetLevelSource( ELevelSource source )
{GUCEF_TRACE;

    SetParameter( PARAMETER_LEVEL_SOURCE, (Float32) source );
}

/*-------------------------------------------------------------------------*/

void
CNoiseGateEffect::SetVoiceActivitySettings( const CVoiceActivitySettings& settings )
{GUCEF_TRACE;

    m_voiceSettings = settings;
}

/*-------------------------------------------------------------------------*/

const CVoiceActivitySettings&
CNoiseGateEffect::GetVoiceActivitySettings( void ) const
{GUCEF_TRACE;

    return m_voiceSettings;
}

/*-------------------------------------------------------------------------*/

bool
CNoiseGateEffect::IsOpen( void ) const
{GUCEF_TRACE;

    return m_publishedOpen.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

Float32
CNoiseGateEffect::GetGainInDecibels( void ) const
{GUCEF_TRACE;

    return m_publishedGainDb.Load();
}

/*-------------------------------------------------------------------------*/

void
CNoiseGateEffect::GetConstructionOptions( CORE::CValueList& options ) const
{GUCEF_TRACE;

    WriteVoiceSettings( m_voiceSettings, options );
}

/*-------------------------------------------------------------------------*/

bool
CNoiseGateEffect::OnPrepare( const CAudioFormat& format, UInt32 )
{GUCEF_TRACE;

    m_channels = format.GetChannelCount();
    m_sampleRate = (Float64) format.GetSampleRate();
    m_range.Prepare( AUDIOSMOOTHING_ONE_POLE, RangeSmoothingSeconds, m_sampleRate, false, GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES );
    m_peakFall = exp( -1.0 / ( LevelDetectorSeconds * m_sampleRate ) );
    m_squareCoefficient = m_peakFall;

    m_voiceReady = m_voice.Prepare( format.GetSampleRate() );
    if ( m_voiceReady )
        m_voice.SetSettings( m_voiceSettings );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CNoiseGateEffect::OnReset( void )
{
    HaveParametersChanged();
    ApplyParameters( true );
    ClearState();
    if ( m_voiceReady )
    {
        m_voice.SetSettings( m_voiceSettings );
        m_voice.Reset();
    }
}

/*-------------------------------------------------------------------------*/

void
CNoiseGateEffect::ClearState( void )
{
    m_peakEnvelope = 0.0;
    m_meanSquare = 0.0;
    m_open = false;
    m_holdLeft = 0;
    m_position = 0.0f;
    m_gain = 1.0f;
    m_gainEnd = 1.0f;
    m_gainStep = 0.0f;
    m_publishedOpen.Store( false, MT::MEMORDER_RELAXED );
    m_publishedGainDb.Store( 0.0f );
}

/*-------------------------------------------------------------------------*/

void
CNoiseGateEffect::ApplyParameters( bool jump )
{
    if ( jump )
        m_range.Reset( ReadParameter( PARAMETER_RANGE ) );
    else
        m_range.SetTarget( ReadParameter( PARAMETER_RANGE ) );

    m_useVoice = m_voiceReady && DETECTOR_VOICE == (EDetector) (Int32) ReadParameter( PARAMETER_DETECTOR );
    m_useRms = LEVELSOURCE_RMS == (ELevelSource) (Int32) ReadParameter( PARAMETER_LEVEL_SOURCE );
    m_thresholdInDecibels = ReadParameter( PARAMETER_THRESHOLD );
    m_closeInDecibels = m_thresholdInDecibels - ReadParameter( PARAMETER_HYSTERESIS );

    // position change per sub-block: a complete change takes the attack or release time
    const Float64 subBlocksPerMs = m_sampleRate / ( 1000.0 * (Float64) GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES );
    const Float64 attackSteps = (Float64) ReadParameter( PARAMETER_ATTACK ) * subBlocksPerMs;
    const Float64 releaseSteps = (Float64) ReadParameter( PARAMETER_RELEASE ) * subBlocksPerMs;
    m_attackStep = attackSteps > 1.0 ? (Float32) ( 1.0 / attackSteps ) : 1.0f;
    m_releaseStep = releaseSteps > 1.0 ? (Float32) ( 1.0 / releaseSteps ) : 1.0f;
    m_holdFrames = (UInt32) ( (Float64) ReadParameter( PARAMETER_HOLD ) * m_sampleRate / 1000.0 + 0.5 );
}

/*-------------------------------------------------------------------------*/

void
CNoiseGateEffect::DetectLevel( const Float32* frame, UInt32 frames )
{
    const UInt32 channels = m_channels;
    if ( m_useRms )
    {
        Float64 average = m_meanSquare;
        const Float64 coefficient = m_squareCoefficient;
        const Float64 scale = 1.0 / (Float64) channels;
        for ( UInt32 f=0; f<frames; ++f )
        {
            Float64 sum = 0.0;
            for ( UInt32 c=0; c<channels; ++c )
            {
                const Float64 x = (Float64) frame[ c ];
                sum += x * x;
            }
            frame += channels;
            const Float64 square = sum * scale;
            if ( square - square != 0.0 )       // not a number or infinite
                continue;
            average = square + coefficient * ( average - square );
        }
        m_meanSquare = average;
    }
    else
    {
        Float64 envelope = m_peakEnvelope;
        const Float64 fall = m_peakFall;
        for ( UInt32 f=0; f<frames; ++f )
        {
            Float32 peak = 0.0f;
            for ( UInt32 c=0; c<channels; ++c )
            {
                const Float32 magnitude = fabsf( frame[ c ] );
                if ( magnitude > peak )
                    peak = magnitude;
            }
            frame += channels;
            if ( peak - peak != 0.0f )          // not a number or infinite
                continue;
            envelope = (Float64) peak > envelope ? (Float64) peak : (Float64) peak + fall * ( envelope - (Float64) peak );
        }
        m_peakEnvelope = envelope;
    }
}

/*-------------------------------------------------------------------------*/

void
CNoiseGateEffect::FeedVoiceDetector( const Float32* frame, UInt32 frames, UInt64 position )
{
    Float32 mono[ GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES ];
    const UInt32 channels = m_channels;
    if ( 1 == channels )
    {
        m_voice.Process( frame, frames, position );
        return;
    }

    const Float32 scale = 1.0f / (Float32) channels;
    for ( UInt32 f=0; f<frames; ++f )
    {
        Float32 sum = 0.0f;
        for ( UInt32 c=0; c<channels; ++c )
            sum += frame[ c ];
        frame += channels;
        mono[ f ] = sum * scale;
    }
    m_voice.Process( mono, frames, position );
}

/*-------------------------------------------------------------------------*/

void
CNoiseGateEffect::StepSubBlock( void )
{
    // the ramp of the previous sub-block has arrived
    m_gain = m_gainEnd;

    bool openNow = false;
    bool keepOpen = false;
    if ( m_useVoice )
    {
        openNow = keepOpen = m_voice.IsActive();
    }
    else
    {
        // keep decayed state out of the denormal range
        if ( m_peakEnvelope < 1.0e-20 )
            m_peakEnvelope = 0.0;
        if ( m_meanSquare < 1.0e-40 )
            m_meanSquare = 0.0;

        Float32 level = SilenceInDecibels;
        if ( m_useRms )
        {
            if ( m_meanSquare > 0.0 )
                level = 10.0f * (Float32) log10( m_meanSquare );
        }
        else
        if ( m_peakEnvelope > 0.0 )
        {
            level = 20.0f * (Float32) log10( m_peakEnvelope );
        }
        openNow = level > m_thresholdInDecibels;
        keepOpen = level >= m_closeInDecibels;
    }

    if ( !m_open )
    {
        if ( openNow )
        {
            m_open = true;
            m_holdLeft = m_holdFrames;
        }
    }
    else
    if ( keepOpen )
    {
        m_holdLeft = m_holdFrames;
    }
    else
    if ( m_holdLeft > 0 )
    {
        m_holdLeft = m_holdLeft > GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES ? m_holdLeft - GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES : 0;
    }
    else
    {
        m_open = false;
    }

    if ( m_open )
    {
        m_position += m_attackStep;
        if ( m_position > 1.0f )
            m_position = 1.0f;
    }
    else
    {
        m_position -= m_releaseStep;
        if ( m_position < 0.0f )
            m_position = 0.0f;
    }

    const Float32 range = m_range.IsSmoothing() ? m_range.Step() : m_range.GetValue();
    m_gainEnd = m_position < 1.0f ? exp2f( range * ( 1.0f - m_position ) * Log2PerDecibel ) : 1.0f;
    m_gainStep = ( m_gainEnd - m_gain ) / (Float32) GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES;
}

/*-------------------------------------------------------------------------*/

void
CNoiseGateEffect::OnProcess( Float32* inOut, UInt32 frames )
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
            StepSubBlock();
        UInt32 run = GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES - inSubBlock;
        if ( run > frames - done )
            run = frames - done;

        Float32* frame = inOut + (size_t) done * channels;

        // the detectors see the audio as it came in, the decision they feed is taken at the next sub-block
        if ( m_useVoice )
            FeedVoiceDetector( frame, run, position );
        else
            DetectLevel( frame, run );

        if ( 1.0f != m_gain || 1.0f != m_gainEnd )
        {
            Float32 gain = m_gain;
            const Float32 step = m_gainStep;
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

    m_publishedOpen.Store( m_open, MT::MEMORDER_RELAXED );
    m_publishedGainDb.Store( m_gainEnd < 1.0f ? 20.0f * log10f( m_gainEnd ) : 0.0f );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
