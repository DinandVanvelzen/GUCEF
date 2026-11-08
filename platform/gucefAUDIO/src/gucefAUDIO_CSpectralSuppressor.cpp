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

#include "gucefAUDIO_CSpectralSuppressor.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const Float64 CSpectralSuppressor::SmoothingSeconds = 0.030;
const Float64 CSpectralSuppressor::GainSmoothingSeconds = 0.004;
const Float64 CSpectralSuppressor::MinimumWindowSeconds = 1.5;
const Float32 CSpectralSuppressor::MinimumBias = 1.7f;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The smallest power the state holds: far below any signal, far above the denormal range */
const Float32 PowerFloor = 1.0e-20f;

/** Alpha of the decision-directed estimate (Ephraim and Malah) */
const Float32 DecisionDirectedAlpha = 0.98f;

const Float32 LargePower = 3.0e38f;

/** Gives the memory of a vector back, which clear() does not */
template< typename T >
void
FreeVector( GUCEF::vector< T >& v )
{
    GUCEF::vector< T > empty;
    v.swap( empty );
}

Float32
Clamp( Float32 value, Float32 low, Float32 high )
{
    if ( !( value >= low ) )        // not a number goes to the low end
        return low;
    return value > high ? high : value;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSpectralSuppressor::CSpectralSuppressor( void )
    : m_sampleRate( 0 )
    , m_size( 0 )
    , m_hop( 0 )
    , m_bins( 0 )
    , m_fft()
    , m_suppressNoise( true )
    , m_noiseFloorGain( 0.17782794f )   // -15 dB
    , m_echoFloorGain( 0.01f )          // -40 dB
    , m_noiseStrength( 1.0f )
    , m_echoStrength( 1.0f )
    , m_bypassBelowHz( 0.0f )
    , m_bypassBins( 0 )
    , m_interference( GUCEF_NULL )
    , m_smoothing( 0.0f )
    , m_gainSmoothing( 0.0f )
    , m_subwindowFrames( 1 )
    , m_window()
    , m_hann()
    , m_input()
    , m_overlap()
    , m_output()
    , m_work()
    , m_power()
    , m_smoothed()
    , m_noise()
    , m_subwindowMin()
    , m_minima()
    , m_storedMinimum()
    , m_previousClean()
    , m_rawGain()
    , m_gain()
    , m_fill( 0 )
    , m_subwindowCount( 0 )
    , m_subwindowIndex( 0 )
    , m_noiseStarted( false )
    , m_transformedBefore( false )
    , m_meanGainDb( 0.0f )
    , m_transformed( 0 )
    , m_bypassed( 0 )
    , m_processed( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSpectralSuppressor::~CSpectralSuppressor()
{GUCEF_TRACE;

    Release();
}

/*-------------------------------------------------------------------------*/

UInt32
CSpectralSuppressor::FrameSizeForRate( UInt32 sampleRate )
{GUCEF_TRACE;

    if ( sampleRate < 8000 || sampleRate > 192000 )
        return 0;

    // the smallest power of two that covers 5.3 ms (256 frames at 48 kHz)
    const Float64 wanted = (Float64) sampleRate * 256.0 / 48000.0;
    UInt32 size = CRealFft::MIN_SIZE;
    while ( (Float64) size < wanted - 1.0e-9 && size < 1024 )
        size *= 2;
    return size;
}

/*-------------------------------------------------------------------------*/

bool
CSpectralSuppressor::Prepare( UInt32 sampleRate )
{GUCEF_TRACE;

    Release();
    const UInt32 size = FrameSizeForRate( sampleRate );
    if ( 0 == size || !m_fft.Prepare( size, true, false ) )
        return false;

    m_sampleRate = sampleRate;
    m_size = size;
    m_hop = size / 2;
    m_bins = size / 2 + 1;

    m_window.assign( size, 0.0f );
    m_hann.assign( size, 0.0f );
    for ( UInt32 n=0; n<size; ++n )
    {
        // periodic: the squares of two windows half a frame apart add up to 1
        const Float64 w = sin( 3.14159265358979323846 * (Float64) n / (Float64) size );
        m_window[ n ] = (Float32) w;
        m_hann[ n ] = (Float32) ( w * w );
    }
    m_input.assign( size, 0.0f );
    m_overlap.assign( size, 0.0f );
    m_output.assign( m_hop, 0.0f );
    m_work.assign( size + 2, 0.0f );
    m_power.assign( m_bins, 0.0f );
    m_smoothed.assign( m_bins, 0.0f );
    m_noise.assign( m_bins, PowerFloor );
    m_subwindowMin.assign( m_bins, LargePower );
    m_minima.assign( (size_t) m_bins * MINIMUM_SUBWINDOWS, LargePower );
    m_storedMinimum.assign( m_bins, LargePower );
    m_previousClean.assign( m_bins, 0.0f );
    m_rawGain.assign( m_bins, 1.0f );
    m_gain.assign( m_bins, 1.0f );

    const Float64 hopSeconds = (Float64) m_hop / (Float64) sampleRate;
    m_smoothing = (Float32) exp( -hopSeconds / SmoothingSeconds );
    m_gainSmoothing = (Float32) exp( -hopSeconds / GainSmoothingSeconds );
    m_subwindowFrames = (UInt32) floor( MinimumWindowSeconds / (Float64) MINIMUM_SUBWINDOWS / hopSeconds + 0.5 );
    if ( m_subwindowFrames < 1 )
        m_subwindowFrames = 1;
    SetBypassBelowHz( m_bypassBelowHz );

    Reset();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CSpectralSuppressor::Release( void )
{GUCEF_TRACE;

    m_sampleRate = 0;
    m_size = 0;
    m_hop = 0;
    m_bins = 0;
    FreeVector( m_window );
    FreeVector( m_hann );
    FreeVector( m_input );
    FreeVector( m_overlap );
    FreeVector( m_output );
    FreeVector( m_work );
    FreeVector( m_power );
    FreeVector( m_smoothed );
    FreeVector( m_noise );
    FreeVector( m_subwindowMin );
    FreeVector( m_minima );
    FreeVector( m_storedMinimum );
    FreeVector( m_previousClean );
    FreeVector( m_rawGain );
    FreeVector( m_gain );
}

/*-------------------------------------------------------------------------*/

bool
CSpectralSuppressor::IsPrepared( void ) const
{
    // no trace macro: the real time threads use it
    return 0 != m_size;
}

/*-------------------------------------------------------------------------*/

UInt32
CSpectralSuppressor::GetSampleRate( void ) const
{
    // no trace macro: the real time threads use it
    return m_sampleRate;
}

/*-------------------------------------------------------------------------*/

UInt32
CSpectralSuppressor::GetFrameSize( void ) const
{
    // no trace macro: the real time threads use it
    return m_size;
}

/*-------------------------------------------------------------------------*/

UInt32
CSpectralSuppressor::GetHopSize( void ) const
{
    // no trace macro: the real time threads use it
    return m_hop;
}

/*-------------------------------------------------------------------------*/

UInt32
CSpectralSuppressor::GetBinCount( void ) const
{
    // no trace macro: the real time threads use it
    return m_bins;
}

/*-------------------------------------------------------------------------*/

UInt32
CSpectralSuppressor::GetLatencyInFrames( void ) const
{
    // no trace macro: the real time threads use it
    return m_size;
}

/*-------------------------------------------------------------------------*/

void
CSpectralSuppressor::Reset( void )
{
    // no trace macro: the audio and capture threads may reset
    if ( 0 == m_size )
        return;

    memset( &m_input[ 0 ], 0, m_input.size() * sizeof( Float32 ) );
    memset( &m_overlap[ 0 ], 0, m_overlap.size() * sizeof( Float32 ) );
    memset( &m_output[ 0 ], 0, m_output.size() * sizeof( Float32 ) );
    for ( UInt32 k=0; k<m_bins; ++k )
    {
        m_power[ k ] = 0.0f;
        m_smoothed[ k ] = 0.0f;
        m_noise[ k ] = PowerFloor;
        m_subwindowMin[ k ] = LargePower;
        m_storedMinimum[ k ] = LargePower;
        m_previousClean[ k ] = 0.0f;
        m_rawGain[ k ] = 1.0f;
        m_gain[ k ] = 1.0f;
    }
    for ( size_t i=0; i<m_minima.size(); ++i )
        m_minima[ i ] = LargePower;
    m_fill = 0;
    m_subwindowCount = 0;
    m_subwindowIndex = 0;
    m_noiseStarted = false;
    m_transformedBefore = false;
    m_meanGainDb = 0.0f;
    m_transformed = 0;
    m_bypassed = 0;
    m_processed = 0;
}

/*-------------------------------------------------------------------------*/

void
CSpectralSuppressor::SetNoiseSuppression( bool suppressNoise )
{
    m_suppressNoise = suppressNoise;
}

/*-------------------------------------------------------------------------*/

bool
CSpectralSuppressor::GetNoiseSuppression( void ) const
{
    return m_suppressNoise;
}

/*-------------------------------------------------------------------------*/

void
CSpectralSuppressor::SetNoiseFloorGain( Float32 gain )
{
    m_noiseFloorGain = Clamp( gain, 0.0f, 1.0f );
}

/*-------------------------------------------------------------------------*/

Float32
CSpectralSuppressor::GetNoiseFloorGain( void ) const
{
    return m_noiseFloorGain;
}

/*-------------------------------------------------------------------------*/

void
CSpectralSuppressor::SetEchoFloorGain( Float32 gain )
{
    m_echoFloorGain = Clamp( gain, 0.0f, 1.0f );
}

/*-------------------------------------------------------------------------*/

Float32
CSpectralSuppressor::GetEchoFloorGain( void ) const
{
    return m_echoFloorGain;
}

/*-------------------------------------------------------------------------*/

void
CSpectralSuppressor::SetNoiseStrength( Float32 strength )
{
    m_noiseStrength = Clamp( strength, 0.25f, 8.0f );
}

/*-------------------------------------------------------------------------*/

Float32
CSpectralSuppressor::GetNoiseStrength( void ) const
{
    return m_noiseStrength;
}

/*-------------------------------------------------------------------------*/

void
CSpectralSuppressor::SetEchoStrength( Float32 strength )
{
    m_echoStrength = Clamp( strength, 0.25f, 8.0f );
}

/*-------------------------------------------------------------------------*/

Float32
CSpectralSuppressor::GetEchoStrength( void ) const
{
    return m_echoStrength;
}

/*-------------------------------------------------------------------------*/

void
CSpectralSuppressor::SetBypassBelowHz( Float32 frequency )
{
    m_bypassBelowHz = Clamp( frequency, 0.0f, 24000.0f );
    m_bypassBins = 0;
    if ( 0 != m_size )
    {
        // the bins whose centre lies below the frequency
        const Float64 binHz = (Float64) m_sampleRate / (Float64) m_size;
        UInt32 bins = (UInt32) ceil( (Float64) m_bypassBelowHz / binHz - 1.0e-9 );
        m_bypassBins = bins > m_bins ? m_bins : bins;
    }
}

/*-------------------------------------------------------------------------*/

Float32
CSpectralSuppressor::GetBypassBelowHz( void ) const
{
    return m_bypassBelowHz;
}

/*-------------------------------------------------------------------------*/

void
CSpectralSuppressor::SetInterference( const Float32* powerPerBin )
{
    m_interference = powerPerBin;
}

/*-------------------------------------------------------------------------*/

Float32
CSpectralSuppressor::GetNoisePower( UInt32 bin ) const
{
    return bin < m_bins ? m_noise[ bin ] : 0.0f;
}

/*-------------------------------------------------------------------------*/

Float32
CSpectralSuppressor::GetGain( UInt32 bin ) const
{
    return bin < m_bins ? m_gain[ bin ] : 1.0f;
}

/*-------------------------------------------------------------------------*/

Float32
CSpectralSuppressor::GetMeanGainInDecibels( void ) const
{
    return m_meanGainDb;
}

/*-------------------------------------------------------------------------*/

UInt64
CSpectralSuppressor::GetTransformedFrameCount( void ) const
{
    return m_transformed;
}

/*-------------------------------------------------------------------------*/

UInt64
CSpectralSuppressor::GetBypassedFrameCount( void ) const
{
    return m_bypassed;
}

/*-------------------------------------------------------------------------*/

UInt64
CSpectralSuppressor::GetProcessedFrameCount( void ) const
{
    return m_processed;
}

/*-------------------------------------------------------------------------*/

void
CSpectralSuppressor::Process( Float32* inOut, UInt32 frames )
{
    // no trace macro: this runs on the audio and capture threads
    if ( 0 == m_size || GUCEF_NULL == inOut )
        return;

    const UInt32 hop = m_hop;
    while ( frames > 0 )
    {
        UInt32 run = hop - m_fill;
        if ( run > frames )
            run = frames;

        // the input goes into the second half of the frame (not a number and infinite values as 0), the output of the
        // hop comes out in its place
        Float32* in = &m_input[ hop + m_fill ];
        for ( UInt32 i=0; i<run; ++i )
        {
            // not a number, infinite and values in the denormal range count as silence
            const Float32 x = inOut[ i ];
            in[ i ] = ( x - x == 0.0f && fabsf( x ) >= 1.0e-30f ) ? x : 0.0f;
        }
        memcpy( inOut, &m_output[ m_fill ], (size_t) run * sizeof( Float32 ) );

        m_fill += run;
        inOut += run;
        frames -= run;
        m_processed += run;
        if ( m_fill == hop )
        {
            ProcessFrame();
            m_fill = 0;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CSpectralSuppressor::NeedsTransform( bool silent ) const
{
    if ( silent )
        return false;
    if ( m_suppressNoise && m_noiseFloorGain < 1.0f )
        return true;
    if ( GUCEF_NULL != m_interference )
    {
        const Float32* interference = m_interference;
        for ( UInt32 k=m_bypassBins; k<m_bins; ++k )
            if ( interference[ k ] > 0.0f )
                return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CSpectralSuppressor::ProcessFrame( void )
{
    const UInt32 size = m_size;
    const UInt32 hop = m_hop;
    const Float32* input = &m_input[ 0 ];

    bool silent = true;
    for ( UInt32 n=hop; n<size && silent; ++n )
        silent = ( 0.0f == input[ n ] );
    if ( silent )
    {
        for ( UInt32 n=0; n<hop && silent; ++n )
            silent = ( 0.0f == input[ n ] );
    }

    Float32* overlap = &m_overlap[ 0 ];
    if ( NeedsTransform( silent ) )
    {
        TransformFrame();
        ++m_transformed;
        m_transformedBefore = true;
    }
    else
    {
        // a gain of 1 without the transform: analysis and synthesis window in one
        if ( !silent )
        {
            const Float32* hann = &m_hann[ 0 ];
            for ( UInt32 n=0; n<size; ++n )
                overlap[ n ] += input[ n ] * hann[ n ];
        }
        if ( m_transformedBefore )
        {
            for ( UInt32 k=0; k<m_bins; ++k )
                m_gain[ k ] = 1.0f;
        }
        ++m_bypassed;
        m_transformedBefore = false;
    }

    // the first half of the accumulator is complete: it is the output of the next hop
    memcpy( &m_output[ 0 ], overlap, (size_t) hop * sizeof( Float32 ) );
    memmove( overlap, overlap + hop, (size_t) ( size - hop ) * sizeof( Float32 ) );
    memset( overlap + ( size - hop ), 0, (size_t) hop * sizeof( Float32 ) );
    memmove( &m_input[ 0 ], &m_input[ hop ], (size_t) hop * sizeof( Float32 ) );
}

/*-------------------------------------------------------------------------*/

void
CSpectralSuppressor::UpdateNoise( void )
{
    const UInt32 bins = m_bins;
    const Float32* power = &m_power[ 0 ];
    Float32* smoothed = &m_smoothed[ 0 ];
    Float32* subMin = &m_subwindowMin[ 0 ];
    const Float32* stored = &m_storedMinimum[ 0 ];
    Float32* noise = &m_noise[ 0 ];

    // minimum statistics: the minimum of the smoothed power over the current sub-window and the ones before
    const Float32 a = m_noiseStarted ? m_smoothing : 0.0f;
    const Float32 b = 1.0f - a;
    for ( UInt32 k=0; k<bins; ++k )
    {
        Float32 s = a * smoothed[ k ] + b * power[ k ];
        s = s > PowerFloor ? s : PowerFloor;
        smoothed[ k ] = s;
        const Float32 m = s < subMin[ k ] ? s : subMin[ k ];
        subMin[ k ] = m;
        const Float32 lowest = m < stored[ k ] ? m : stored[ k ];
        noise[ k ] = MinimumBias * lowest;
    }
    m_noiseStarted = true;

    if ( ++m_subwindowCount < m_subwindowFrames )
        return;

    // the sub-window is complete: its minima replace the oldest row, the minimum over the rows is kept per bin
    m_subwindowCount = 0;
    Float32* row = &m_minima[ (size_t) m_subwindowIndex * bins ];
    memcpy( row, subMin, (size_t) bins * sizeof( Float32 ) );
    m_subwindowIndex = ( m_subwindowIndex + 1 ) % MINIMUM_SUBWINDOWS;
    Float32* storedMin = &m_storedMinimum[ 0 ];
    for ( UInt32 k=0; k<bins; ++k )
    {
        storedMin[ k ] = LargePower;
        subMin[ k ] = LargePower;
    }
    for ( UInt32 r=0; r<MINIMUM_SUBWINDOWS; ++r )
    {
        const Float32* values = &m_minima[ (size_t) r * bins ];
        for ( UInt32 k=0; k<bins; ++k )
            storedMin[ k ] = values[ k ] < storedMin[ k ] ? values[ k ] : storedMin[ k ];
    }
}

/*-------------------------------------------------------------------------*/

void
CSpectralSuppressor::ComputeGains( void )
{
    const UInt32 bins = m_bins;
    const Float32* power = &m_power[ 0 ];
    const Float32* noise = &m_noise[ 0 ];
    const Float32* interference = m_interference;
    Float32* previousClean = &m_previousClean[ 0 ];
    Float32* raw = &m_rawGain[ 0 ];

    const bool suppressNoise = m_suppressNoise && m_noiseFloorGain < 1.0f;
    const Float32 noiseStrength = suppressNoise ? m_noiseStrength : 0.0f;
    const Float32 echoStrength = GUCEF_NULL != interference ? m_echoStrength : 0.0f;
    const Float32 noiseFloor2 = m_noiseFloorGain * m_noiseFloorGain;
    const Float32 echoFloor2 = m_echoFloorGain * m_echoFloorGain;
    const Float32 alpha = m_transformedBefore ? DecisionDirectedAlpha : 0.0f;
    const Float32 beta = 1.0f - alpha;

    for ( UInt32 k=0; k<bins; ++k )
    {
        const Float32 p = power[ k ];
        const Float32 in = noiseStrength * noise[ k ];
        const Float32 ie = GUCEF_NULL != interference ? echoStrength * interference[ k ] : 0.0f;
        const Float32 total = in + ie;
        Float32 g = 1.0f;
        if ( total > PowerFloor )
        {
            const Float32 inverse = 1.0f / total;
            const Float32 posteriori = p * inverse;
            const Float32 ml = posteriori > 1.0f ? posteriori - 1.0f : 0.0f;
            const Float32 xi = alpha * previousClean[ k ] * inverse + beta * ml;
            g = xi / ( 1.0f + xi );

            // the floor of what is there, as a power ratio
            Float32 floor2 = ( noiseFloor2 * in + echoFloor2 * ie ) * inverse;
            if ( !suppressNoise && ie > 0.0f && p > PowerFloor )
            {
                // comfort floor: the background noise of the near end stays
                const Float32 comfort = noise[ k ] / p;
                if ( comfort > floor2 )
                    floor2 = comfort < 1.0f ? comfort : 1.0f;
            }
            if ( g * g < floor2 )
                g = sqrtf( floor2 );
        }
        raw[ k ] = g;
        previousClean[ k ] = g * g * p;
    }

    // smoothing over three bins, then over time; the bins below the bypass frequency stay at 1
    Float32* gain = &m_gain[ 0 ];
    const Float32 c = m_gainSmoothing;
    const Float32 d = 1.0f - c;
    Float32 before = raw[ bins > 1 ? 1 : 0 ];
    for ( UInt32 k=0; k<bins; ++k )
    {
        const Float32 after = ( k + 1 < bins ) ? raw[ k + 1 ] : raw[ k > 0 ? k - 1 : 0 ];
        Float32 smoothed = 0.25f * before + 0.5f * raw[ k ] + 0.25f * after;
        if ( k < m_bypassBins )
            smoothed = 1.0f;
        before = raw[ k ];
        gain[ k ] = c * gain[ k ] + d * smoothed;
    }
}

/*-------------------------------------------------------------------------*/

void
CSpectralSuppressor::TransformFrame( void )
{
    const UInt32 size = m_size;
    const UInt32 bins = m_bins;
    Float32* work = &m_work[ 0 ];
    const Float32* window = &m_window[ 0 ];
    const Float32* input = &m_input[ 0 ];

    for ( UInt32 n=0; n<size; ++n )
        work[ n ] = input[ n ] * window[ n ];
    m_fft.Forward( work, work );

    Float32* power = &m_power[ 0 ];
    for ( UInt32 k=0; k<bins; ++k )
    {
        const Float32 re = work[ 2 * k ];
        const Float32 im = work[ 2 * k + 1 ];
        power[ k ] = re * re + im * im;
    }

    UpdateNoise();
    ComputeGains();

    const Float32* gain = &m_gain[ 0 ];
    Float64 before = 0.0, after = 0.0;
    for ( UInt32 k=0; k<bins; ++k )
    {
        const Float32 g = gain[ k ];
        work[ 2 * k ] *= g;
        work[ 2 * k + 1 ] *= g;
        before += (Float64) power[ k ];
        after += (Float64) ( g * g * power[ k ] );
    }
    m_meanGainDb = ( before > 0.0 && after > 0.0 ) ? (Float32) ( 10.0 * log10( after / before ) ) : 0.0f;

    m_fft.Inverse( work, work );
    Float32* overlap = &m_overlap[ 0 ];
    for ( UInt32 n=0; n<size; ++n )
        overlap[ n ] += work[ n ] * window[ n ];
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
