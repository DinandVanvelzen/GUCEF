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

#include "gucefAUDIO_CAutomaticGainEffect.h"

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

/** Release time constant of the ceiling stage */
const Float64 CeilingReleaseSeconds = 0.100;

/** Length of the analysis frames in seconds, the weight of one frame in the level average is based on it */
const Float64 AnalysisFrameSeconds = 0.010;

/**
 *  An active frame counts towards the speech level when its mean square is at least this fraction of the level (12 dB below it,
 *  after the active speech level of ITU-T P.56, which uses 15.9 dB): the quiet ends of syllables are not speech level
 */
const Float64 ActiveFrameFraction = 0.063095734448019;

/** Active frames in a row that are all below the fraction above: the speech became quieter, the level starts over from them */
const UInt32 ReseedFrames = 100;

/** The value shown for a speech level that is not known */
const Float32 NoLevelInDecibels = -120.0f;

/** The ceiling stage aims this much below the ceiling, so rounding in the product cannot put a sample above it */
const Float32 CeilingMargin = 0.99999f;

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
CAutomaticGainEffect::GetStaticEffectTypeName( void )
{GUCEF_TRACE;

    static const CString typeName( "automaticGain" );
    return typeName;
}

/*-------------------------------------------------------------------------*/

UInt32
CAutomaticGainEffect::GetStaticParameterCount( void )
{GUCEF_TRACE;

    return PARAMETER_COUNT;
}

/*-------------------------------------------------------------------------*/

const CAudioEffectParameterInfo*
CAutomaticGainEffect::GetStaticParameterInfos( void )
{GUCEF_TRACE;

    static const char* const freezeLabels[ 2 ] = { "off", "on" };
    static const CAudioEffectParameterInfo infos[ PARAMETER_COUNT ] =
    {
        CAudioEffectParameterInfo( PARAMETER_TARGET, "target", AUDIOPARAMUNIT_DECIBEL, AUDIOPARAMSCALE_LINEAR, -40.0f, -6.0f, -20.0f ),
        CAudioEffectParameterInfo( PARAMETER_MAX_GAIN, "maxGain", AUDIOPARAMUNIT_DECIBEL, AUDIOPARAMSCALE_LINEAR, 0.0f, 40.0f, 24.0f ),
        CAudioEffectParameterInfo( PARAMETER_MIN_GAIN, "minGain", AUDIOPARAMUNIT_DECIBEL, AUDIOPARAMSCALE_LINEAR, -20.0f, 0.0f, -12.0f ),
        CAudioEffectParameterInfo( PARAMETER_RISE_RATE, "riseRate", AUDIOPARAMUNIT_NONE, AUDIOPARAMSCALE_LOGARITHMIC, 0.5f, 30.0f, 6.0f ),
        CAudioEffectParameterInfo( PARAMETER_FALL_RATE, "fallRate", AUDIOPARAMUNIT_NONE, AUDIOPARAMSCALE_LOGARITHMIC, 1.0f, 60.0f, 20.0f ),
        CAudioEffectParameterInfo( PARAMETER_LEVEL_TIME, "levelTime", AUDIOPARAMUNIT_MILLISECONDS, AUDIOPARAMSCALE_LOGARITHMIC, 100.0f, 3000.0f, 500.0f ),
        CAudioEffectParameterInfo( PARAMETER_FREEZE, "freeze", freezeLabels, 2, FREEZE_ON ),
        CAudioEffectParameterInfo( PARAMETER_CEILING, "ceiling", AUDIOPARAMUNIT_DECIBEL, AUDIOPARAMSCALE_LINEAR, -12.0f, 0.0f, -1.0f )
    };
    return infos;
}

/*-------------------------------------------------------------------------*/

CAutomaticGainEffect::CAutomaticGainEffect( const CORE::CValueList& options )
    : CAudioEffect( GetStaticParameterInfos(), PARAMETER_COUNT )
    , m_voiceSettings()
    , m_publishedGainDb( 0.0f )
    , m_publishedLevelDb( NoLevelInDecibels )
    , m_channels( 0 )
    , m_sampleRate( 48000 )
    , m_voice()
    , m_releaseMargin( CVoiceActivitySettings::DefaultReleaseInDecibels )
    , m_targetInDecibels( -20.0f )
    , m_maxGainInDecibels( 24.0f )
    , m_minGainInDecibels( -12.0f )
    , m_riseStep( 0.0f )
    , m_fallStep( 0.0f )
    , m_levelCoefficient( 0.0 )
    , m_freeze( true )
    , m_ceiling( 1.0f )
    , m_limitRelease( 0.0f )
    , m_frameIndex( 0 )
    , m_frameStart( 0 )
    , m_frameEnd( 0 )
    , m_frameSum( 0.0 )
    , m_haveLevel( false )
    , m_speechMeanSquare( 0.0 )
    , m_rejectedFrames( 0 )
    , m_rejectedSum( 0.0 )
    , m_gainInDecibels( 0.0f )
    , m_gain( 1.0f )
    , m_gainEnd( 1.0f )
    , m_gainStep( 0.0f )
    , m_limit( 1.0f )
{GUCEF_TRACE;

    ReadVoiceSettings( options, m_voiceSettings );
}

/*-------------------------------------------------------------------------*/

CAutomaticGainEffect::~CAutomaticGainEffect()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CString&
CAutomaticGainEffect::GetEffectTypeName( void ) const
{GUCEF_TRACE;

    return GetStaticEffectTypeName();
}

/*-------------------------------------------------------------------------*/

void
CAutomaticGainEffect::SetTargetInDecibels( Float32 decibels )
{GUCEF_TRACE;

    SetParameter( PARAMETER_TARGET, decibels );
}

/*-------------------------------------------------------------------------*/

void
CAutomaticGainEffect::SetMaxGainInDecibels( Float32 decibels )
{GUCEF_TRACE;

    SetParameter( PARAMETER_MAX_GAIN, decibels );
}

/*-------------------------------------------------------------------------*/

void
CAutomaticGainEffect::SetMinGainInDecibels( Float32 decibels )
{GUCEF_TRACE;

    SetParameter( PARAMETER_MIN_GAIN, decibels );
}

/*-------------------------------------------------------------------------*/

void
CAutomaticGainEffect::SetRiseRateInDecibelsPerSecond( Float32 decibelsPerSecond )
{GUCEF_TRACE;

    SetParameter( PARAMETER_RISE_RATE, decibelsPerSecond );
}

/*-------------------------------------------------------------------------*/

void
CAutomaticGainEffect::SetFallRateInDecibelsPerSecond( Float32 decibelsPerSecond )
{GUCEF_TRACE;

    SetParameter( PARAMETER_FALL_RATE, decibelsPerSecond );
}

/*-------------------------------------------------------------------------*/

void
CAutomaticGainEffect::SetLevelTimeInMs( Float32 milliseconds )
{GUCEF_TRACE;

    SetParameter( PARAMETER_LEVEL_TIME, milliseconds );
}

/*-------------------------------------------------------------------------*/

void
CAutomaticGainEffect::SetFreeze( bool freeze )
{GUCEF_TRACE;

    SetParameter( PARAMETER_FREEZE, freeze ? (Float32) FREEZE_ON : (Float32) FREEZE_OFF );
}

/*-------------------------------------------------------------------------*/

void
CAutomaticGainEffect::SetCeilingInDecibels( Float32 decibels )
{GUCEF_TRACE;

    SetParameter( PARAMETER_CEILING, decibels );
}

/*-------------------------------------------------------------------------*/

void
CAutomaticGainEffect::SetVoiceActivitySettings( const CVoiceActivitySettings& settings )
{GUCEF_TRACE;

    m_voiceSettings = settings;
}

/*-------------------------------------------------------------------------*/

const CVoiceActivitySettings&
CAutomaticGainEffect::GetVoiceActivitySettings( void ) const
{GUCEF_TRACE;

    return m_voiceSettings;
}

/*-------------------------------------------------------------------------*/

Float32
CAutomaticGainEffect::GetGainInDecibels( void ) const
{GUCEF_TRACE;

    return m_publishedGainDb.Load();
}

/*-------------------------------------------------------------------------*/

Float32
CAutomaticGainEffect::GetSpeechLevelInDecibels( void ) const
{GUCEF_TRACE;

    return m_publishedLevelDb.Load();
}

/*-------------------------------------------------------------------------*/

void
CAutomaticGainEffect::GetConstructionOptions( CORE::CValueList& options ) const
{GUCEF_TRACE;

    WriteVoiceSettings( m_voiceSettings, options );
}

/*-------------------------------------------------------------------------*/

UInt64
CAutomaticGainEffect::FrameBoundary( UInt64 frameIndex ) const
{
    return ( frameIndex * (UInt64) m_sampleRate ) / 100;
}

/*-------------------------------------------------------------------------*/

bool
CAutomaticGainEffect::OnPrepare( const CAudioFormat& format, UInt32 )
{GUCEF_TRACE;

    m_channels = format.GetChannelCount();
    m_sampleRate = format.GetSampleRate();
    if ( !m_voice.Prepare( m_sampleRate ) )
        return false;
    m_voice.SetSettings( m_voiceSettings );
    m_limitRelease = (Float32) ( 1.0 - exp( -1.0 / ( CeilingReleaseSeconds * (Float64) m_sampleRate ) ) );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAutomaticGainEffect::OnReset( void )
{
    HaveParametersChanged();
    ApplyParameters();
    ClearState();
    m_voice.SetSettings( m_voiceSettings );
    m_voice.Reset();
    m_releaseMargin = m_voiceSettings.GetReleaseInDecibels();
}

/*-------------------------------------------------------------------------*/

void
CAutomaticGainEffect::ClearState( void )
{
    m_frameIndex = 0;
    m_frameStart = 0;
    m_frameEnd = FrameBoundary( 1 );
    m_frameSum = 0.0;
    m_haveLevel = false;
    m_speechMeanSquare = 0.0;
    m_rejectedFrames = 0;
    m_rejectedSum = 0.0;
    m_gainInDecibels = 0.0f;
    m_gain = 1.0f;
    m_gainEnd = 1.0f;
    m_gainStep = 0.0f;
    m_limit = 1.0f;
    m_publishedGainDb.Store( 0.0f );
    m_publishedLevelDb.Store( NoLevelInDecibels );
}

/*-------------------------------------------------------------------------*/

void
CAutomaticGainEffect::ApplyParameters( void )
{
    m_targetInDecibels = ReadParameter( PARAMETER_TARGET );
    m_maxGainInDecibels = ReadParameter( PARAMETER_MAX_GAIN );
    m_minGainInDecibels = ReadParameter( PARAMETER_MIN_GAIN );
    m_freeze = FREEZE_ON == (EFreeze) (Int32) ReadParameter( PARAMETER_FREEZE );
    m_ceiling = (Float32) pow( 10.0, (Float64) ReadParameter( PARAMETER_CEILING ) / 20.0 );

    const Float64 subBlockSeconds = (Float64) GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES / (Float64) m_sampleRate;
    m_riseStep = (Float32) ( (Float64) ReadParameter( PARAMETER_RISE_RATE ) * subBlockSeconds );
    m_fallStep = (Float32) ( (Float64) ReadParameter( PARAMETER_FALL_RATE ) * subBlockSeconds );

    const Float64 levelSeconds = (Float64) ReadParameter( PARAMETER_LEVEL_TIME ) / 1000.0;
    m_levelCoefficient = 1.0 - exp( -AnalysisFrameSeconds / levelSeconds );
}

/*-------------------------------------------------------------------------*/

void
CAutomaticGainEffect::FinishAnalysisFrame( void )
{
    // the voice detector has just completed this very frame
    const Float64 length = (Float64) ( m_frameEnd - m_frameStart );
    if ( m_voice.IsActive() && m_voice.GetEnergyInDecibels() > m_voice.GetNoiseFloorInDecibels() + m_releaseMargin )
    {
        const Float64 meanSquare = m_frameSum / length;
        if ( !m_haveLevel )
        {
            m_speechMeanSquare = meanSquare;
            m_haveLevel = true;
        }
        else
        if ( meanSquare >= ActiveFrameFraction * m_speechMeanSquare )
        {
            m_speechMeanSquare += m_levelCoefficient * ( meanSquare - m_speechMeanSquare );
            m_rejectedFrames = 0;
            m_rejectedSum = 0.0;
        }
        else
        {
            // far below the level: the end of a syllable, or the speech became much quieter
            m_rejectedSum += meanSquare;
            if ( ++m_rejectedFrames >= ReseedFrames )
            {
                m_speechMeanSquare = m_rejectedSum / (Float64) m_rejectedFrames;
                m_rejectedFrames = 0;
                m_rejectedSum = 0.0;
            }
        }
    }

    ++m_frameIndex;
    m_frameStart = m_frameEnd;
    m_frameEnd = FrameBoundary( m_frameIndex + 1 );
    m_frameSum = 0.0;
}

/*-------------------------------------------------------------------------*/

void
CAutomaticGainEffect::StepSubBlock( void )
{
    // the ramp of the previous sub-block has arrived
    m_gain = m_gainEnd;

    Float32 desired = 0.0f;
    if ( m_haveLevel )
    {
        const Float64 level = m_speechMeanSquare > 1.0e-12 ? 10.0 * log10( m_speechMeanSquare ) : -120.0;
        desired = m_targetInDecibels - (Float32) level;
    }
    if ( desired > m_maxGainInDecibels )
        desired = m_maxGainInDecibels;
    if ( desired < m_minGainInDecibels )
        desired = m_minGainInDecibels;

    const bool outOfRange = m_gainInDecibels > m_maxGainInDecibels || m_gainInDecibels < m_minGainInDecibels;
    if ( !m_freeze || outOfRange || m_voice.IsActive() )
    {
        const Float32 delta = desired - m_gainInDecibels;
        if ( delta > m_riseStep )
            m_gainInDecibels += m_riseStep;
        else
        if ( delta < -m_fallStep )
            m_gainInDecibels -= m_fallStep;
        else
            m_gainInDecibels = desired;
    }

    m_gainEnd = 0.0f != m_gainInDecibels ? exp2f( m_gainInDecibels * Log2PerDecibel ) : 1.0f;
    m_gainStep = ( m_gainEnd - m_gain ) / (Float32) GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES;
}

/*-------------------------------------------------------------------------*/

void
CAutomaticGainEffect::OnProcess( Float32* inOut, UInt32 frames )
{
    if ( HaveParametersChanged() )
    {
        ApplyParameters();
    }

    const UInt32 channels = m_channels;
    const Float32 scale = 1.0f / (Float32) channels;
    UInt64 position = GetProcessedFrameCount();
    UInt32 done = 0;
    Float32 mono[ GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES ];
    while ( done < frames )
    {
        const UInt32 inSubBlock = (UInt32) ( position % GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES );
        if ( 0 == inSubBlock )
            StepSubBlock();

        // a piece ends at the end of the sub-block, of the analysis frame or of the block
        UInt32 run = GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES - inSubBlock;
        if ( run > frames - done )
            run = frames - done;
        const UInt64 untilFrameEnd = m_frameEnd - position;
        if ( untilFrameEnd < (UInt64) run )
            run = (UInt32) untilFrameEnd;

        Float32* frame = inOut + (size_t) done * channels;

        // the detector and the level see the audio as it came in
        Float64 sum = m_frameSum;
        for ( UInt32 f=0; f<run; ++f )
        {
            Float32 mix = 0.0f;
            for ( UInt32 c=0; c<channels; ++c )
                mix += frame[ (size_t) f * channels + c ];
            mix *= scale;
            if ( mix - mix != 0.0f )            // not a number or infinite
                mix = 0.0f;
            mono[ f ] = mix;
            sum += (Float64) mix * (Float64) mix;
        }
        m_frameSum = sum;
        m_voice.Process( mono, run, position );

        // the gain and the ceiling
        const bool ramp = 1.0f != m_gain || 1.0f != m_gainEnd;
        Float32 gain = m_gain;
        const Float32 step = m_gainStep;
        Float32 limit = m_limit;
        const Float32 ceiling = m_ceiling;
        for ( UInt32 f=0; f<run; ++f )
        {
            if ( ramp )
                gain += step;

            Float32 peak = 0.0f;
            for ( UInt32 c=0; c<channels; ++c )
            {
                const Float32 magnitude = fabsf( frame[ c ] );
                if ( magnitude > peak )
                    peak = magnitude;
            }
            peak *= gain;

            if ( limit < 1.0f )
            {
                limit += ( 1.0f - limit ) * m_limitRelease;
                if ( 1.0f - limit < 1.0e-6f )
                    limit = 1.0f;
            }
            if ( peak * limit > ceiling )
                limit = ceiling * CeilingMargin / peak;

            const Float32 total = gain * limit;
            if ( 1.0f != total )
            {
                for ( UInt32 c=0; c<channels; ++c )
                    frame[ c ] *= total;
            }
            frame += channels;
        }
        m_gain = gain;
        m_limit = limit;

        done += run;
        position += run;
        if ( position >= m_frameEnd )
            FinishAnalysisFrame();
    }

    m_publishedGainDb.Store( m_gainInDecibels );
    m_publishedLevelDb.Store( m_haveLevel && m_speechMeanSquare > 1.0e-12 ? (Float32) ( 10.0 * log10( m_speechMeanSquare ) ) : NoLevelInDecibels );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
