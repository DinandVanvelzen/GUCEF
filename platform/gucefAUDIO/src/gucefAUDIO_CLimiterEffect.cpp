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

#include "gucefAUDIO_CLimiterEffect.h"

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

/** Longest lookahead the parameter offers, in seconds */
const Float64 MaxLookaheadSeconds = 0.010;

/** Samples beyond this are not audio: they are brought back into range so the gain computation stays finite */
const Float32 MaxSampleMagnitude = 1.0e30f;

/** The largest Float32 that is not above 'value' (positive values only) */
inline Float32
FloatNotAbove( Float64 value )
{
    Float32 result = (Float32) value;
    if ( (Float64) result > value )
    {
        UInt32 bits = 0;
        memcpy( &bits, &result, sizeof( bits ) );
        --bits;
        memcpy( &result, &bits, sizeof( result ) );
    }
    return result;
}

inline Float32
Sanitize( Float32 sample )
{
    if ( !( fabsf( sample ) <= MaxSampleMagnitude ) )
        return sample != sample ? 0.0f : ( sample > 0.0f ? MaxSampleMagnitude : -MaxSampleMagnitude );
    return sample;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString&
CLimiterEffect::GetStaticEffectTypeName( void )
{GUCEF_TRACE;

    static const CString typeName( "limiter" );
    return typeName;
}

/*-------------------------------------------------------------------------*/

UInt32
CLimiterEffect::GetStaticParameterCount( void )
{GUCEF_TRACE;

    return PARAMETER_COUNT;
}

/*-------------------------------------------------------------------------*/

const CAudioEffectParameterInfo*
CLimiterEffect::GetStaticParameterInfos( void )
{GUCEF_TRACE;

    static const char* const modeLabels[ 3 ] = { "off", "instant", "lookahead" };
    static const CAudioEffectParameterInfo infos[ PARAMETER_COUNT ] =
    {
        CAudioEffectParameterInfo( PARAMETER_MODE, "mode", modeLabels, 3, (UInt32) AUDIOLIMITERMODE_LOOKAHEAD ),
        CAudioEffectParameterInfo( PARAMETER_CEILING, "ceiling", AUDIOPARAMUNIT_DECIBEL, AUDIOPARAMSCALE_LINEAR, -12.0f, 0.0f, -0.3f ),
        CAudioEffectParameterInfo( PARAMETER_LOOKAHEAD, "lookahead", AUDIOPARAMUNIT_MILLISECONDS, AUDIOPARAMSCALE_LINEAR, 0.5f, 10.0f, 1.5f ),
        CAudioEffectParameterInfo( PARAMETER_RELEASE, "release", AUDIOPARAMUNIT_MILLISECONDS, AUDIOPARAMSCALE_LOGARITHMIC, 10.0f, 1000.0f, 50.0f ),
        CAudioEffectParameterInfo( PARAMETER_INPUT_GAIN, "inputGain", AUDIOPARAMUNIT_DECIBEL, AUDIOPARAMSCALE_LINEAR, -12.0f, 24.0f, 0.0f )
    };
    return infos;
}

/*-------------------------------------------------------------------------*/

Float32
CLimiterEffect::GetMixerCeilingInDecibels( void )
{GUCEF_TRACE;

    return (Float32) ( 20.0 * log10( (Float64) 0.98f ) );
}

/*-------------------------------------------------------------------------*/

CLimiterEffect::CLimiterEffect( const CORE::CValueList& options )
    : CAudioEffect( GetStaticParameterInfos(), PARAMETER_COUNT )
    , m_channels( 0 )
    , m_sampleRate( 48000.0 )
    , m_maxLookahead( 480 )
    , m_mode( AUDIOLIMITERMODE_LOOKAHEAD )
    , m_lookahead( 72 )
    , m_ceilingLinear( 1.0 )
    , m_ceilingInstant( 1.0f )
    , m_ceilingBelow( 1.0f )
    , m_releaseInstant( 1.0f )
    , m_releaseLookahead( 1.0 )
    , m_inputGain( 1.0f )
    , m_inputGainTarget( 1.0f )
    , m_instantGain( 1.0f )
    , m_delay()
    , m_minimum()
    , m_averageRing()
    , m_averagePosition( 0 )
    , m_averageSum( 0.0 )
    , m_belowUnity( 0 )
    , m_gain( 1.0 )
    , m_limitedFrames( 0 )
{GUCEF_TRACE;

    const CString modeName = options.GetValueAlways( "mode" ).AsString();
    if ( !modeName.IsNULLOrEmpty() )
    {
        Float32 mode = 0.0f;
        if ( GetStaticParameterInfos()[ PARAMETER_MODE ].TryGetEnumValue( modeName, mode ) )
            SetParameter( PARAMETER_MODE, mode );
    }
}

/*-------------------------------------------------------------------------*/

CLimiterEffect::~CLimiterEffect()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CString&
CLimiterEffect::GetEffectTypeName( void ) const
{GUCEF_TRACE;

    return GetStaticEffectTypeName();
}

/*-------------------------------------------------------------------------*/

UInt32
CLimiterEffect::LookaheadFramesFor( Float32 milliseconds ) const
{
    Float64 frames = floor( (Float64) milliseconds * m_sampleRate / 1000.0 + 0.5 );
    if ( frames < 1.0 ) frames = 1.0;
    if ( frames > (Float64) m_maxLookahead ) frames = (Float64) m_maxLookahead;
    return (UInt32) frames;
}

/*-------------------------------------------------------------------------*/

UInt32
CLimiterEffect::GetLatencyInFrames( void ) const
{GUCEF_TRACE;

    if ( (Float32) AUDIOLIMITERMODE_LOOKAHEAD != GetParameter( PARAMETER_MODE ) )
        return 0;
    return LookaheadFramesFor( GetParameter( PARAMETER_LOOKAHEAD ) );
}

/*-------------------------------------------------------------------------*/

void
CLimiterEffect::SetMode( TAudioLimiterMode mode )
{GUCEF_TRACE;

    SetParameter( PARAMETER_MODE, (Float32) mode );
}

/*-------------------------------------------------------------------------*/

TAudioLimiterMode
CLimiterEffect::GetMode( void ) const
{GUCEF_TRACE;

    return (TAudioLimiterMode) (Int32) GetParameter( PARAMETER_MODE );
}

/*-------------------------------------------------------------------------*/

void
CLimiterEffect::SetCeilingInDecibels( Float32 decibels )
{GUCEF_TRACE;

    SetParameter( PARAMETER_CEILING, decibels );
}

/*-------------------------------------------------------------------------*/

void
CLimiterEffect::SetLookaheadInMs( Float32 milliseconds )
{GUCEF_TRACE;

    SetParameter( PARAMETER_LOOKAHEAD, milliseconds );
}

/*-------------------------------------------------------------------------*/

void
CLimiterEffect::SetReleaseInMs( Float32 milliseconds )
{GUCEF_TRACE;

    SetParameter( PARAMETER_RELEASE, milliseconds );
}

/*-------------------------------------------------------------------------*/

void
CLimiterEffect::SetInputGainInDecibels( Float32 decibels )
{GUCEF_TRACE;

    SetParameter( PARAMETER_INPUT_GAIN, decibels );
}

/*-------------------------------------------------------------------------*/

UInt64
CLimiterEffect::GetLimitedFrameCount( void ) const
{GUCEF_TRACE;

    return m_limitedFrames.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

bool
CLimiterEffect::OnPrepare( const CAudioFormat& format, UInt32 maxBlockFrames )
{GUCEF_TRACE;

    m_channels = format.GetChannelCount();
    m_sampleRate = (Float64) format.GetSampleRate();
    m_maxLookahead = (UInt32) ceil( MaxLookaheadSeconds * m_sampleRate );
    if ( m_maxLookahead < 1 )
        m_maxLookahead = 1;

    // the longest lookahead is reserved now, so a change of the lookahead later on allocates nothing
    if ( !m_delay.Prepare( m_channels, m_maxLookahead ) || !m_minimum.Prepare( m_maxLookahead + 1 ) )
        return false;
    m_averageRing.assign( (size_t) m_maxLookahead + 1, 1.0f );
    m_mode = AUDIOLIMITERMODE_OFF;                  // the first ApplyParameters() sees a change and starts the state
    return true;
}

/*-------------------------------------------------------------------------*/

void
CLimiterEffect::OnReset( void )
{
    HaveParametersChanged();
    m_mode = AUDIOLIMITERMODE_OFF;
    ApplyParameters();
    ClearState();
    m_inputGain = m_inputGainTarget;
}

/*-------------------------------------------------------------------------*/

void
CLimiterEffect::ClearState( void )
{
    m_instantGain = 1.0f;
    m_gain = 1.0;
    m_delay.Reset();
    if ( m_lookahead + 1 <= m_averageRing.size() )
    {
        m_minimum.SetWindowSize( m_lookahead + 1 );     // inside the reserved memory: nothing is allocated, nothing traced
        for ( UInt32 i=0; i<=m_lookahead; ++i )
            m_averageRing[ i ] = 1.0f;
    }
    m_averagePosition = 0;
    m_averageSum = (Float64) ( m_lookahead + 1 );
    m_belowUnity = 0;
}

/*-------------------------------------------------------------------------*/

void
CLimiterEffect::ApplyParameters( void )
{
    const TAudioLimiterMode mode = (TAudioLimiterMode) (Int32) ReadParameter( PARAMETER_MODE );
    const UInt32 lookahead = LookaheadFramesFor( ReadParameter( PARAMETER_LOOKAHEAD ) );

    m_ceilingLinear = pow( 10.0, (Float64) ReadParameter( PARAMETER_CEILING ) / 20.0 );
    m_ceilingInstant = (Float32) m_ceilingLinear;
    m_ceilingBelow = FloatNotAbove( m_ceilingLinear );

    // the time constant of the release, the way the mixer limiter computes it
    const Float64 releaseSeconds = (Float64) ReadParameter( PARAMETER_RELEASE ) / 1000.0;
    m_releaseInstant = 1.0f - (Float32) exp( -1.0 / ( releaseSeconds * m_sampleRate ) );
    m_releaseLookahead = 1.0 - exp( -1.0 / ( releaseSeconds * m_sampleRate ) );

    m_inputGainTarget = (Float32) pow( 10.0, (Float64) ReadParameter( PARAMETER_INPUT_GAIN ) / 20.0 );

    const bool restart = mode != m_mode || ( AUDIOLIMITERMODE_LOOKAHEAD == mode && lookahead != m_lookahead );
    m_mode = mode;
    m_lookahead = lookahead;
    if ( restart )
        ClearState();
}

/*-------------------------------------------------------------------------*/

void
CLimiterEffect::OnProcess( Float32* inOut, UInt32 frames )
{
    if ( HaveParametersChanged() )
        ApplyParameters();

    switch ( m_mode )
    {
        case AUDIOLIMITERMODE_INSTANT :
        {
            ProcessInstant( inOut, frames );
            break;
        }
        case AUDIOLIMITERMODE_LOOKAHEAD :
        {
            ProcessLookahead( inOut, frames );
            break;
        }
        default :
        {
            m_inputGain = m_inputGainTarget;        // off: nothing happens to the audio
            break;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CLimiterEffect::ProcessInstant( Float32* inOut, UInt32 frames )
{
    const UInt32 channels = m_channels;
    const Float32 ceiling = m_ceilingInstant;
    const Float32 release = m_releaseInstant;
    Float32 limiterGain = m_instantGain;
    UInt64 limited = 0;

    Float32 inputGain = m_inputGain;
    const Float32 inputTarget = m_inputGainTarget;
    const Float32 inputStep = inputGain != inputTarget ? ( inputTarget - inputGain ) / (Float32) frames : 0.0f;
    const bool applyInputGain = inputGain != 1.0f || inputTarget != 1.0f;

    for ( UInt32 f=0; f<frames; ++f )
    {
        Float32* frame = inOut + (size_t) f * channels;

        if ( applyInputGain )
        {
            inputGain = f + 1 == frames ? inputTarget : inputGain + inputStep;
            for ( UInt32 c=0; c<channels; ++c )
                frame[ c ] *= inputGain;
        }

        Float32 peak = 0.0f;
        for ( UInt32 c=0; c<channels; ++c )
        {
            const Float32 level = frame[ c ] < 0.0f ? -frame[ c ] : frame[ c ];
            if ( level > peak )
                peak = level;
        }

        // down at once as far as this frame needs, back up gradually (the same arithmetic as the mixer master stage)
        limiterGain += ( 1.0f - limiterGain ) * release;
        if ( peak * limiterGain > ceiling )
            limiterGain = ceiling / peak;
        if ( limiterGain < 1.0f )
        {
            ++limited;
            for ( UInt32 c=0; c<channels; ++c )
                frame[ c ] *= limiterGain;
        }
    }

    m_instantGain = limiterGain;
    m_inputGain = inputTarget;
    if ( limited > 0 )
        m_limitedFrames.FetchAdd( limited, MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

void
CLimiterEffect::ProcessLookahead( Float32* inOut, UInt32 frames )
{
    const UInt32 channels = m_channels;
    const UInt32 lookahead = m_lookahead;
    const UInt32 window = lookahead + 1;
    const Float64 ceilingLinear = m_ceilingLinear;
    const Float32 ceilingBelow = m_ceilingBelow;
    const Float64 release = m_releaseLookahead;
    Float64 gain = m_gain;
    UInt64 limited = 0;

    Float32 inputGain = m_inputGain;
    const Float32 inputTarget = m_inputGainTarget;
    const Float32 inputStep = inputGain != inputTarget ? ( inputTarget - inputGain ) / (Float32) frames : 0.0f;
    const bool applyInputGain = inputGain != 1.0f || inputTarget != 1.0f;

    Float32* ring = &m_averageRing[ 0 ];
    UInt32 position = m_averagePosition;
    Float64 sum = m_averageSum;
    UInt32 belowUnity = m_belowUnity;

    for ( UInt32 f=0; f<frames; ++f )
    {
        Float32* frame = inOut + (size_t) f * channels;

        if ( applyInputGain )
            inputGain = f + 1 == frames ? inputTarget : inputGain + inputStep;

        // the loudest channel decides; the audio goes into the delay line, to come out a lookahead later
        Float32 peak = 0.0f;
        for ( UInt32 c=0; c<channels; ++c )
        {
            Float32 sample = Sanitize( frame[ c ] );
            if ( applyInputGain )
                sample *= inputGain;
            m_delay.Push( c, sample );
            const Float32 level = sample < 0.0f ? -sample : sample;
            if ( level > peak )
                peak = level;
        }

        // the gain this frame asks for, never rounded up: a reduction that is too small would let a sample through
        Float32 target = 1.0f;
        if ( (Float64) peak > ceilingLinear )
            target = FloatNotAbove( ceilingLinear / (Float64) peak );

        // strongest reduction of the window, then the average of those over the window: a smooth attack that is complete in time
        const Float32 lowest = m_minimum.Push( target );
        const Float32 leaving = ring[ position ];
        ring[ position ] = lowest;
        if ( ++position >= window )
            position = 0;
        if ( leaving < 1.0f )
            --belowUnity;
        if ( lowest < 1.0f )
            ++belowUnity;
        if ( 0 == belowUnity )
            sum = (Float64) window;                 // all ones: no rounding drift is carried along
        else
            sum += (Float64) lowest - (Float64) leaving;
        Float64 average = sum / (Float64) window;
        if ( average > 1.0 )
            average = 1.0;

        // down as soon as the average is, up with the release
        if ( average < gain )
            gain = average;
        else
            gain += ( average - gain ) * release;

        if ( gain < 1.0 )
        {
            ++limited;
            for ( UInt32 c=0; c<channels; ++c )
            {
                Float32 out = (Float32) ( (Float64) m_delay.Read( c, lookahead ) * gain );
                if ( out > ceilingBelow )           // only ever acts on a rounding step, or on a ceiling that was just turned down
                    out = ceilingBelow;
                else
                if ( out < -ceilingBelow )
                    out = -ceilingBelow;
                frame[ c ] = out;
            }
        }
        else
        {
            for ( UInt32 c=0; c<channels; ++c )
            {
                Float32 out = m_delay.Read( c, lookahead );
                if ( out > ceilingBelow )
                    out = ceilingBelow;
                else
                if ( out < -ceilingBelow )
                    out = -ceilingBelow;
                frame[ c ] = out;
            }
        }
    }

    m_gain = gain;
    m_averagePosition = position;
    m_averageSum = sum;
    m_belowUnity = belowUnity;
    m_inputGain = inputTarget;
    if ( limited > 0 )
        m_limitedFrames.FetchAdd( limited, MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
