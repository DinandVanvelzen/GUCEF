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

#include "gucefAUDIO_ETypes.h"

#include "gucefAUDIO_CDelayEffect.h"

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

const Float32 DefaultMaxDelayMs = 2000.0f;
const Float32 LongestMaxDelayMs = 10000.0f;
const Float32 LongestStereoOffsetMs = 50.0f;

const Float64 TimeSmoothingSeconds = 0.100;
const Float64 FilterSmoothingSeconds = 0.020;

/** The filters in the feedback path are Butterworth sections */
const Float64 FilterQ = 0.7071067811865476;

/** Feedback below this is silence; flushing it keeps the loop out of the denormal range where nothing flushes to zero */
const Float32 DenormalFlushThreshold = 1.0e-15f;

/** Cut frequencies at these values switch the filter off */
const Float32 LowCutOff = 20.001f;
const Float32 HighCutOff = 19999.0f;

Float32
MaxDelayOf( const CORE::CValueList& options )
{
    Float32 maxDelay = options.GetValueAlways( "maxDelay" ).AsFloat32( DefaultMaxDelayMs );
    if ( !( maxDelay >= 1.0f ) )
        maxDelay = 1.0f;
    if ( maxDelay > LongestMaxDelayMs )
        maxDelay = LongestMaxDelayMs;
    return maxDelay;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString&
CDelayEffect::GetStaticEffectTypeName( void )
{GUCEF_TRACE;

    static const CString typeName( "delay" );
    return typeName;
}

/*-------------------------------------------------------------------------*/

UInt32
CDelayEffect::GetStaticParameterCount( void )
{GUCEF_TRACE;

    return PARAMETER_COUNT;
}

/*-------------------------------------------------------------------------*/

const CAudioEffectParameterInfo*
CDelayEffect::GetStaticParameterInfos( void )
{GUCEF_TRACE;

    static const CAudioEffectParameterInfo infos[ PARAMETER_COUNT ] =
    {
        CAudioEffectParameterInfo( PARAMETER_TIME, "time", AUDIOPARAMUNIT_MILLISECONDS, AUDIOPARAMSCALE_LOGARITHMIC, 1.0f, LongestMaxDelayMs, 375.0f ),
        CAudioEffectParameterInfo( PARAMETER_FEEDBACK, "feedback", AUDIOPARAMUNIT_NONE, AUDIOPARAMSCALE_LINEAR, -0.98f, 0.98f, 0.35f ),
        CAudioEffectParameterInfo( PARAMETER_PING_PONG, "pingPong", AUDIOPARAMUNIT_NONE, AUDIOPARAMSCALE_LINEAR, 0.0f, 1.0f, 0.0f, AUDIOPARAMKIND_BOOLEAN ),
        CAudioEffectParameterInfo( PARAMETER_LOW_CUT, "lowCut", AUDIOPARAMUNIT_HERTZ, AUDIOPARAMSCALE_LOGARITHMIC, 20.0f, 2000.0f, 80.0f ),
        CAudioEffectParameterInfo( PARAMETER_HIGH_CUT, "highCut", AUDIOPARAMUNIT_HERTZ, AUDIOPARAMSCALE_LOGARITHMIC, 1000.0f, 20000.0f, 8000.0f ),
        CAudioEffectParameterInfo( PARAMETER_STEREO_OFFSET, "stereoOffset", AUDIOPARAMUNIT_MILLISECONDS, AUDIOPARAMSCALE_LINEAR, 0.0f, LongestStereoOffsetMs, 0.0f )
    };
    return infos;
}

/*-------------------------------------------------------------------------*/

CDelayEffect::CDelayEffect( const CORE::CValueList& options )
    : CAudioEffect( GetStaticParameterInfos(), PARAMETER_COUNT )
    , m_maxDelayMs( MaxDelayOf( options ) )
    , m_channels( 0 )
    , m_passMask( 0 )
    , m_sampleRate( 48000.0 )
    , m_delay()
    , m_lowCutFilter()
    , m_highCutFilter()
    , m_lowCutActive( false )
    , m_highCutActive( false )
    , m_pingPong( false )
    , m_time()
    , m_offset()
    , m_feedback()
    , m_lowCut()
    , m_highCut()
    , m_taps()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDelayEffect::~CDelayEffect()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CString&
CDelayEffect::GetEffectTypeName( void ) const
{GUCEF_TRACE;

    return GetStaticEffectTypeName();
}

/*-------------------------------------------------------------------------*/

Float32
CDelayEffect::GetMaxDelayInMs( void ) const
{GUCEF_TRACE;

    return m_maxDelayMs;
}

/*-------------------------------------------------------------------------*/

void
CDelayEffect::GetConstructionOptions( CORE::CValueList& options ) const
{GUCEF_TRACE;

    options.Set( "maxDelay", m_maxDelayMs );
}

/*-------------------------------------------------------------------------*/

Float32
CDelayEffect::TimeInFrames( Float32 milliseconds ) const
{
    if ( milliseconds > m_maxDelayMs )
        milliseconds = m_maxDelayMs;
    return (Float32) ( (Float64) milliseconds * m_sampleRate / 1000.0 );
}

/*-------------------------------------------------------------------------*/

UInt32
CDelayEffect::GetTailInFrames( void ) const
{GUCEF_TRACE;

    if ( 0.0f != GetParameter( PARAMETER_FEEDBACK ) )
        return GUCEF_AUDIO_TAIL_INFINITE;
    return (UInt32) ( TimeInFrames( GetParameter( PARAMETER_TIME ) ) + TimeInFrames( GetParameter( PARAMETER_STEREO_OFFSET ) ) + 0.5f );
}

/*-------------------------------------------------------------------------*/

void
CDelayEffect::SetTimeInMs( Float32 milliseconds )
{GUCEF_TRACE;

    SetParameter( PARAMETER_TIME, milliseconds );
}

/*-------------------------------------------------------------------------*/

void
CDelayEffect::SetFeedback( Float32 feedback )
{GUCEF_TRACE;

    SetParameter( PARAMETER_FEEDBACK, feedback );
}

/*-------------------------------------------------------------------------*/

void
CDelayEffect::SetPingPong( bool pingPong )
{GUCEF_TRACE;

    SetParameter( PARAMETER_PING_PONG, pingPong ? 1.0f : 0.0f );
}

/*-------------------------------------------------------------------------*/

void
CDelayEffect::SetLowCutInHertz( Float32 hertz )
{GUCEF_TRACE;

    SetParameter( PARAMETER_LOW_CUT, hertz );
}

/*-------------------------------------------------------------------------*/

void
CDelayEffect::SetHighCutInHertz( Float32 hertz )
{GUCEF_TRACE;

    SetParameter( PARAMETER_HIGH_CUT, hertz );
}

/*-------------------------------------------------------------------------*/

void
CDelayEffect::SetStereoOffsetInMs( Float32 milliseconds )
{GUCEF_TRACE;

    SetParameter( PARAMETER_STEREO_OFFSET, milliseconds );
}

/*-------------------------------------------------------------------------*/

bool
CDelayEffect::OnPrepare( const CAudioFormat& format, UInt32 maxBlockFrames )
{GUCEF_TRACE;

    m_channels = format.GetChannelCount();
    m_sampleRate = (Float64) format.GetSampleRate();

    m_passMask = 0;
    const UInt32 layout = format.GetChannelLayout();
    if ( ( layout & AUDIOSPEAKER_LOW_FREQUENCY ) != 0 && m_channels <= 32 )
        m_passMask = 1U << CAudioFormat::GetChannelCountOfLayout( layout & ( AUDIOSPEAKER_LOW_FREQUENCY - 1 ) );

    // room for the longest delay time and the longest stereo offset on top of it, and the samples a cubic read uses
    const Float64 longest = ( (Float64) m_maxDelayMs + (Float64) LongestStereoOffsetMs ) * m_sampleRate / 1000.0;
    if ( !m_delay.Prepare( m_channels, (UInt32) ceil( longest ) + 2 ) ||
         !m_lowCutFilter.Prepare( m_channels ) || !m_highCutFilter.Prepare( m_channels ) )
        return false;
    m_taps.assign( m_channels, 0.0f );

    m_time.Prepare( AUDIOSMOOTHING_ONE_POLE, TimeSmoothingSeconds, m_sampleRate, false, 1 );
    m_offset.Prepare( AUDIOSMOOTHING_ONE_POLE, TimeSmoothingSeconds, m_sampleRate, false, 1 );
    m_feedback.Prepare( AUDIOSMOOTHING_ONE_POLE, FilterSmoothingSeconds, m_sampleRate, false, GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES );
    m_lowCut.Prepare( AUDIOSMOOTHING_ONE_POLE, FilterSmoothingSeconds, m_sampleRate, true, GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES );
    m_highCut.Prepare( AUDIOSMOOTHING_ONE_POLE, FilterSmoothingSeconds, m_sampleRate, true, GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDelayEffect::UpdateFilters( void )
{
    const Float32 lowCut = m_lowCut.GetValue();
    const Float32 highCut = m_highCut.GetValue();
    m_lowCutActive = lowCut > LowCutOff;
    m_highCutActive = highCut < HighCutOff;
    if ( m_lowCutActive )
        m_lowCutFilter.SetResponse( FILTERRESPONSE_HIGHPASS, m_sampleRate, (Float64) lowCut, FilterQ );
    if ( m_highCutActive )
        m_highCutFilter.SetResponse( FILTERRESPONSE_LOWPASS, m_sampleRate, (Float64) highCut, FilterQ );
}

/*-------------------------------------------------------------------------*/

void
CDelayEffect::ApplyParameters( bool jump )
{
    const Float32 time = TimeInFrames( ReadParameter( PARAMETER_TIME ) );
    const Float32 offset = TimeInFrames( ReadParameter( PARAMETER_STEREO_OFFSET ) );
    if ( jump )
    {
        m_time.Reset( time );
        m_offset.Reset( offset );
        m_feedback.Reset( ReadParameter( PARAMETER_FEEDBACK ) );
        m_lowCut.Reset( ReadParameter( PARAMETER_LOW_CUT ) );
        m_highCut.Reset( ReadParameter( PARAMETER_HIGH_CUT ) );
    }
    else
    {
        m_time.SetTarget( time );
        m_offset.SetTarget( offset );
        m_feedback.SetTarget( ReadParameter( PARAMETER_FEEDBACK ) );
        m_lowCut.SetTarget( ReadParameter( PARAMETER_LOW_CUT ) );
        m_highCut.SetTarget( ReadParameter( PARAMETER_HIGH_CUT ) );
    }
    m_pingPong = ReadParameter( PARAMETER_PING_PONG ) != 0.0f;
    if ( jump )
        UpdateFilters();
}

/*-------------------------------------------------------------------------*/

void
CDelayEffect::OnReset( void )
{
    HaveParametersChanged();
    m_delay.Reset();
    m_lowCutFilter.Reset();
    m_highCutFilter.Reset();
    ApplyParameters( true );
}

/*-------------------------------------------------------------------------*/

void
CDelayEffect::StepSubBlock( void )
{
    if ( m_feedback.IsSmoothing() )
        m_feedback.Step();
    if ( m_lowCut.IsSmoothing() || m_highCut.IsSmoothing() )
    {
        m_lowCut.Step();
        m_highCut.Step();
        UpdateFilters();
    }
    m_lowCutFilter.FlushDenormals();
    m_highCutFilter.FlushDenormals();
}

/*-------------------------------------------------------------------------*/

void
CDelayEffect::OnProcess( Float32* inOut, UInt32 frames )
{
    if ( HaveParametersChanged() )
        ApplyParameters( false );

    const UInt32 channels = m_channels;
    const UInt32 passMask = m_passMask;
    const bool pingPong = m_pingPong && channels >= 2 && 0 == ( passMask & 3 );
    Float32* taps = &m_taps[ 0 ];

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

        const Float32 feedback = m_feedback.GetValue();
        const bool lowCut = m_lowCutActive;
        const bool highCut = m_highCutActive;

        for ( UInt32 f=0; f<run; ++f )
        {
            Float32* frame = inOut + (size_t) ( done + f ) * channels;

            // the echoes: the delay time counts back from the sample that is about to be written, which is one more than a read
            const Float32 time = m_time.IsSmoothing() ? m_time.Step() : m_time.GetValue();
            const Float32 offset = m_offset.IsSmoothing() ? m_offset.Step() : m_offset.GetValue();
            for ( UInt32 c=0; c<channels; ++c )
            {
                if ( ( passMask >> c ) & 1 )
                    continue;
                const Float32 delay = ( c & 1 ) ? time + offset : time;
                taps[ c ] = m_delay.ReadCubic( c, delay - 1.0f );
            }

            for ( UInt32 c=0; c<channels; ++c )
            {
                if ( ( passMask >> c ) & 1 )
                    continue;

                // the echo that comes back: from the other channel when ping-ponging
                Float32 returning = ( pingPong && c < 2 ) ? taps[ 1 - c ] : taps[ c ];
                if ( lowCut )
                    returning = m_lowCutFilter.Process( c, returning );
                if ( highCut )
                    returning = m_highCutFilter.Process( c, returning );
                returning *= feedback;
                if ( returning < DenormalFlushThreshold && returning > -DenormalFlushThreshold )
                    returning = 0.0f;

                m_delay.Push( c, frame[ c ] + returning );
                frame[ c ] = taps[ c ];
            }
        }

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
