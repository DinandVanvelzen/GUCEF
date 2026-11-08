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

#include "gucefAUDIO_CVoiceActivityDetector.h"

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

const Float64 BandLowFrequency = 200.0;
const Float64 BandHighFrequency = 4000.0;
const Float64 ButterworthQ = 0.70710678118654752;
const Float64 SmoothingInMs = 50.0;

/** The smallest mean square that is kept apart from silence: -120 dB */
const Float64 SilenceMeanSquare = 1.0e-12;
const Float32 SilenceInDecibels = -120.0f;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const Float32 CVoiceActivityDetector::AnalysisFrameInMs = 10.0f;
const Float32 CVoiceActivityDetector::FloorWindowInMs = 1500.0f;

/*-------------------------------------------------------------------------*/

CVoiceActivityDetector::CVoiceActivityDetector( void )
    : m_prepared( false )
    , m_sampleRate( 0 )
    , m_settings()
    , m_onsetInDecibels( 9.0f )
    , m_releaseInDecibels( 6.0f )
    , m_minimumLevelInDecibels( -65.0f )
    , m_risePerFrameInDecibels( 0.03 )
    , m_hangoverFrames( 20 )
    , m_highPass()
    , m_lowPass()
    , m_minimum()
    , m_smoothing( 0.0 )
    , m_position( 0 )
    , m_frameIndex( 0 )
    , m_frameStart( 0 )
    , m_frameEnd( 0 )
    , m_sum( 0.0 )
    , m_dropFrame( false )
    , m_haveFrame( false )
    , m_smoothed( 0.0 )
    , m_floorInDecibels( SilenceInDecibels )
    , m_onsetCount( 0 )
    , m_hangoverLeft( 0 )
    , m_active( false )
    , m_publishedActive( false )
    , m_publishedEnergy( SilenceInDecibels )
    , m_publishedFloor( SilenceInDecibels )
    , m_publishedFrames( 0 )
{GUCEF_TRACE;

    ApplySettings();
}

/*-------------------------------------------------------------------------*/

CVoiceActivityDetector::~CVoiceActivityDetector()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CVoiceActivityDetector::Prepare( UInt32 sampleRate )
{GUCEF_TRACE;

    Release();
    if ( sampleRate < 8000 || sampleRate > 192000 )
        return false;

    const Float64 rate = (Float64) sampleRate;
    Float64 highCut = BandHighFrequency;
    if ( highCut > 0.45 * rate )
        highCut = 0.45 * rate;
    if ( !m_highPass.Prepare( 1 ) || !m_lowPass.Prepare( 1 ) ||
         !m_highPass.SetResponse( FILTERRESPONSE_HIGHPASS, rate, BandLowFrequency, ButterworthQ ) ||
         !m_lowPass.SetResponse( FILTERRESPONSE_LOWPASS, rate, highCut, ButterworthQ ) )
        return false;

    const UInt32 windowFrames = (UInt32) ( FloorWindowInMs / AnalysisFrameInMs + 0.5f );
    if ( !m_minimum.Prepare( windowFrames ) )
        return false;

    m_sampleRate = sampleRate;
    m_smoothing = exp( -(Float64) AnalysisFrameInMs / SmoothingInMs );
    m_prepared = true;
    ApplySettings();
    ClearState();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CVoiceActivityDetector::Release( void )
{GUCEF_TRACE;

    m_prepared = false;
    m_sampleRate = 0;
    ClearState();
}

/*-------------------------------------------------------------------------*/

bool
CVoiceActivityDetector::IsPrepared( void ) const
{GUCEF_TRACE;

    return m_prepared;
}

/*-------------------------------------------------------------------------*/

UInt32
CVoiceActivityDetector::GetSampleRate( void ) const
{GUCEF_TRACE;

    return m_sampleRate;
}

/*-------------------------------------------------------------------------*/

void
CVoiceActivityDetector::SetSettings( const CVoiceActivitySettings& settings )
{GUCEF_TRACE;

    m_settings = settings;
    ApplySettings();
}

/*-------------------------------------------------------------------------*/

const CVoiceActivitySettings&
CVoiceActivityDetector::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

void
CVoiceActivityDetector::ApplySettings( void )
{
    m_onsetInDecibels = m_settings.GetOnsetInDecibels();
    m_releaseInDecibels = m_settings.GetReleaseInDecibels();
    m_minimumLevelInDecibels = m_settings.GetMinimumLevelInDecibels();
    m_risePerFrameInDecibels = (Float64) m_settings.GetFloorRiseInDecibelsPerSecond() * (Float64) AnalysisFrameInMs / 1000.0;
    m_hangoverFrames = (UInt32) ( m_settings.GetHangoverInMs() / AnalysisFrameInMs + 0.5f );
    if ( m_hangoverLeft > m_hangoverFrames )
        m_hangoverLeft = m_hangoverFrames;
}

/*-------------------------------------------------------------------------*/

void
CVoiceActivityDetector::ClearState( void )
{
    m_highPass.Reset();
    m_lowPass.Reset();
    m_minimum.Reset();
    m_position = 0;
    m_haveFrame = false;
    m_smoothed = 0.0;
    m_floorInDecibels = SilenceInDecibels;
    m_onsetCount = 0;
    m_hangoverLeft = 0;
    m_active = false;
    m_publishedActive.Store( false, MT::MEMORDER_RELAXED );
    m_publishedEnergy.Store( SilenceInDecibels );
    m_publishedFloor.Store( SilenceInDecibels );
    m_publishedFrames.Store( 0, MT::MEMORDER_RELAXED );
    if ( m_prepared )
        BeginFrameAt( 0 );
    else
    {
        m_frameIndex = 0;
        m_frameStart = 0;
        m_frameEnd = 0;
        m_sum = 0.0;
        m_dropFrame = false;
    }
}

/*-------------------------------------------------------------------------*/

void
CVoiceActivityDetector::Reset( void )
{
    ClearState();
}

/*-------------------------------------------------------------------------*/

UInt64
CVoiceActivityDetector::FrameBoundary( UInt64 frameIndex ) const
{
    return ( frameIndex * (UInt64) m_sampleRate ) / 100;
}

/*-------------------------------------------------------------------------*/

void
CVoiceActivityDetector::BeginFrameAt( UInt64 position )
{
    // the analysis frame that holds the position: the first frame whose end lies beyond it
    m_frameIndex = ( position * 100 ) / (UInt64) m_sampleRate;
    while ( FrameBoundary( m_frameIndex + 1 ) <= position )
        ++m_frameIndex;
    while ( m_frameIndex > 0 && FrameBoundary( m_frameIndex ) > position )
        --m_frameIndex;
    m_frameStart = FrameBoundary( m_frameIndex );
    m_frameEnd = FrameBoundary( m_frameIndex + 1 );
    m_position = position;
    m_sum = 0.0;
    m_dropFrame = ( position != m_frameStart );
}

/*-------------------------------------------------------------------------*/

bool
CVoiceActivityDetector::Process( const Float32* mono, UInt32 frameCount, UInt64 firstFrame ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    if ( !m_prepared )
        return false;

    if ( firstFrame != m_position )
    {
        // a gap: what the filters hold belongs to audio that is not contiguous with this
        m_highPass.Reset();
        m_lowPass.Reset();
        m_onsetCount = 0;
        BeginFrameAt( firstFrame );
    }
    return Process( mono, frameCount );
}

/*-------------------------------------------------------------------------*/

bool
CVoiceActivityDetector::Process( const Float32* mono, UInt32 frameCount ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    if ( !m_prepared || GUCEF_NULL == mono )
        return m_active;

    UInt32 done = 0;
    while ( done < frameCount )
    {
        const UInt64 untilEnd = m_frameEnd - m_position;
        const UInt32 left = frameCount - done;
        const UInt32 count = ( untilEnd < (UInt64) left ) ? (UInt32) untilEnd : left;

        Float64 sum = m_sum;
        for ( UInt32 i=0; i<count; ++i )
        {
            Float32 x = mono[ done + i ];
            if ( x - x != 0.0f )        // not a number or infinite
                x = 0.0f;
            const Float32 y = m_lowPass.Process( 0, m_highPass.Process( 0, x ) );
            sum += (Float64) y * (Float64) y;
        }
        m_sum = sum;
        m_position += count;
        done += count;

        if ( m_position >= m_frameEnd )
            FinishFrame();
    }
    return m_active;
}

/*-------------------------------------------------------------------------*/

void
CVoiceActivityDetector::FinishFrame( void )
{
    if ( !m_dropFrame )
    {
        const Float64 length = (Float64) ( m_frameEnd - m_frameStart );
        Analyze( m_sum / length );
    }

    // the state decays towards zero in silence: keep it out of the denormal range, at a point that does not depend on the block size
    m_highPass.FlushDenormals();
    m_lowPass.FlushDenormals();

    ++m_frameIndex;
    m_frameStart = m_frameEnd;
    m_frameEnd = FrameBoundary( m_frameIndex + 1 );
    m_sum = 0.0;
    m_dropFrame = false;
}

/*-------------------------------------------------------------------------*/

void
CVoiceActivityDetector::Analyze( Float64 meanSquare )
{
    if ( !( meanSquare >= SilenceMeanSquare ) )
        meanSquare = SilenceMeanSquare;
    const Float64 energy = 10.0 * log10( meanSquare );

    // smoothed over 50 ms, in power
    if ( !m_haveFrame )
        m_smoothed = meanSquare;
    else
        m_smoothed = m_smoothing * m_smoothed + ( 1.0 - m_smoothing ) * meanSquare;
    const Float64 smoothedInDecibels = 10.0 * log10( m_smoothed );

    // the floor: the minimum of the smoothed energy over the window, falling at once and rising at a limited rate. It does not go below the minimum level
    Float64 minimum = (Float64) m_minimum.Push( (Float32) smoothedInDecibels );
    if ( minimum < (Float64) m_minimumLevelInDecibels )
        minimum = (Float64) m_minimumLevelInDecibels;
    if ( !m_haveFrame || minimum <= m_floorInDecibels )
        m_floorInDecibels = minimum;
    else
    {
        m_floorInDecibels += m_risePerFrameInDecibels;
        if ( m_floorInDecibels > minimum )
            m_floorInDecibels = minimum;
    }
    m_haveFrame = true;

    // the decision
    if ( energy < (Float64) m_minimumLevelInDecibels )
    {
        m_onsetCount = 0;
        m_hangoverLeft = 0;
        m_active = false;
    }
    else
    if ( !m_active )
    {
        if ( energy > m_floorInDecibels + (Float64) m_onsetInDecibels )
        {
            if ( ++m_onsetCount >= 2 )
            {
                m_active = true;
                m_onsetCount = 0;
                m_hangoverLeft = m_hangoverFrames;
            }
        }
        else
            m_onsetCount = 0;
    }
    else
    {
        if ( energy > m_floorInDecibels + (Float64) m_releaseInDecibels )
            m_hangoverLeft = m_hangoverFrames;
        else
        {
            if ( m_hangoverLeft > 0 )
                --m_hangoverLeft;
            if ( 0 == m_hangoverLeft )
                m_active = false;
        }
    }

    m_publishedEnergy.Store( (Float32) energy );
    m_publishedFloor.Store( (Float32) m_floorInDecibels );
    m_publishedActive.Store( m_active, MT::MEMORDER_RELAXED );
    m_publishedFrames.FetchAdd( 1, MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

bool
CVoiceActivityDetector::IsActive( void ) const
{
    return m_publishedActive.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

Float32
CVoiceActivityDetector::GetEnergyInDecibels( void ) const
{
    return m_publishedEnergy.Load();
}

/*-------------------------------------------------------------------------*/

Float32
CVoiceActivityDetector::GetNoiseFloorInDecibels( void ) const
{
    return m_publishedFloor.Load();
}

/*-------------------------------------------------------------------------*/

UInt64
CVoiceActivityDetector::GetAnalyzedFrameCount( void ) const
{
    return m_publishedFrames.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

UInt64
CVoiceActivityDetector::GetPosition( void ) const
{
    return m_position;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
