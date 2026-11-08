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

#include "gucefAUDIO_CSincResampler.h"

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

const Float64 Pi = 3.14159265358979323846;

/** Cut off of the kernel relative to the lower of the two sample rates */
const Float64 CutOff = 0.45;

/** Kaiser window shape: about 80 dB of side lobe rejection */
const Float64 KaiserBeta = 8.0;

/** Taps at a ratio of 1 and below */
const UInt32 BaseTaps = 2 * CSincResampler::HALF_WIDTH;

/** Frames in the history ring: enough for the longest kernel (at MAX_STEP) to either side, a power of two */
const UInt32 HistoryFrames = 1024;

/** Most taps there can be: the kernel at the largest ratio */
const UInt32 MaxTaps = 2 * CSincResampler::HALF_WIDTH * CSincResampler::MAX_STEP;

/** Entries of the stretching kernel table: the kernel's half width times the phases, plus room for the rounding up of the kernel's half width at stretched ratios, plus the interpolation partner */
const UInt32 KernelTableSize = ( CSincResampler::HALF_WIDTH + 1 ) * CSincResampler::PHASE_COUNT + 2;

/** Results smaller than this are zero: keeps denormal numbers out of the output */
const Float32 DenormalFlushThreshold = 1.0e-30f;

/*-------------------------------------------------------------------------*/

/** Modified Bessel function of the first kind, order 0, by its power series */
Float64
BesselI0( Float64 x )
{
    const Float64 quarter = x * x * 0.25;
    Float64 term = 1.0;
    Float64 sum = 1.0;
    for ( UInt32 k=1; k<200; ++k )
    {
        term *= quarter / ( (Float64) k * (Float64) k );
        sum += term;
        if ( term < sum * 1.0e-17 )
            break;
    }
    return sum;
}

/*-------------------------------------------------------------------------*/

/** The kernel at 'x' input frames from its centre, unnormalised: a windowed low pass of cut off CutOff */
Float64
KernelValue( Float64 x )
{
    const Float64 halfWidth = (Float64) CSincResampler::HALF_WIDTH;
    if ( fabs( x ) >= halfWidth )
        return 0.0;
    const Float64 sinc = ( fabs( x ) < 1.0e-12 ) ? 2.0 * CutOff : sin( 2.0 * Pi * CutOff * x ) / ( Pi * x );
    const Float64 r = x / halfWidth;
    const Float64 window = BesselI0( KaiserBeta * sqrt( 1.0 - r * r ) ) / BesselI0( KaiserBeta );
    return sinc * window;
}

/*-------------------------------------------------------------------------*/

/** Sum of products with four independent running sums, which the processor can overlap */
inline Float32
Dot( const Float32* coefficients, const Float32* samples, UInt32 stride, UInt32 count )
{
    Float32 s0 = 0.0f;
    Float32 s1 = 0.0f;
    Float32 s2 = 0.0f;
    Float32 s3 = 0.0f;
    UInt32 k = 0;
    if ( 1 == stride )
    {
        for ( ; k+4<=count; k+=4 )
        {
            s0 += coefficients[ k ] * samples[ k ];
            s1 += coefficients[ k+1 ] * samples[ k+1 ];
            s2 += coefficients[ k+2 ] * samples[ k+2 ];
            s3 += coefficients[ k+3 ] * samples[ k+3 ];
        }
    }
    else
    {
        for ( ; k+4<=count; k+=4 )
        {
            s0 += coefficients[ k ] * samples[ (size_t) k * stride ];
            s1 += coefficients[ k+1 ] * samples[ (size_t) ( k+1 ) * stride ];
            s2 += coefficients[ k+2 ] * samples[ (size_t) ( k+2 ) * stride ];
            s3 += coefficients[ k+3 ] * samples[ (size_t) ( k+3 ) * stride ];
        }
    }
    for ( ; k<count; ++k )
        s0 += coefficients[ k ] * samples[ (size_t) k * stride ];
    return ( s0 + s1 ) + ( s2 + s3 );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSincResampler::CSincResampler( void )
    : IAudioResampler()
    , m_channelCount( 0 )
    , m_step( 1.0 )
    , m_position( 0.0 )
    , m_phaseTable()
    , m_kernel()
    , m_history()
    , m_coefficients()
    , m_window()
    , m_historyWrite( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSincResampler::~CSincResampler()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CSincResampler::BuildTables( void )
{GUCEF_TRACE;

    // the kernel from the centre outwards, for the stretched kernels
    m_kernel.assign( KernelTableSize, 0.0f );
    for ( UInt32 i=0; i<=HALF_WIDTH*PHASE_COUNT; ++i )
        m_kernel[ i ] = (Float32) KernelValue( (Float64) i / (Float64) PHASE_COUNT );

    // one row per phase, tap k of the row for an output at 'phase' past the frame that is tap HALF_WIDTH-1
    m_phaseTable.assign( (size_t) ( PHASE_COUNT + 1 ) * BaseTaps, 0.0f );
    for ( UInt32 p=0; p<=PHASE_COUNT; ++p )
    {
        Float64 row[ BaseTaps ];
        Float64 sum = 0.0;
        for ( UInt32 k=0; k<BaseTaps; ++k )
        {
            row[ k ] = KernelValue( (Float64) p / (Float64) PHASE_COUNT + (Float64) ( HALF_WIDTH - 1 ) - (Float64) k );
            sum += row[ k ];
        }
        for ( UInt32 k=0; k<BaseTaps; ++k )
            m_phaseTable[ (size_t) p * BaseTaps + k ] = (Float32) ( row[ k ] / sum );
    }
}

/*-------------------------------------------------------------------------*/

bool
CSincResampler::Configure( UInt32 channelCount, Float64 inputRate, Float64 outputRate )
{GUCEF_TRACE;

    if ( 0 == channelCount || channelCount > 256 || !( inputRate > 0.0 ) || !( outputRate > 0.0 ) )
        return false;
    const Float64 step = inputRate / outputRate;
    if ( step > (Float64) MAX_STEP )
        return false;

    if ( channelCount != m_channelCount || m_phaseTable.empty() )
    {
        if ( m_phaseTable.empty() )
            BuildTables();
        m_history.assign( (size_t) HistoryFrames * channelCount, 0.0f );
        m_window.assign( (size_t) MaxTaps * channelCount, 0.0f );
        m_coefficients.assign( MaxTaps + 4, 0.0f );
        m_channelCount = channelCount;
        Reset();
    }
    return SetRates( inputRate, outputRate );
}

/*-------------------------------------------------------------------------*/

bool
CSincResampler::SetRates( Float64 inputRate, Float64 outputRate )
{
    // no trace macro: this runs where the audio is mixed

    if ( 0 == m_channelCount || !( inputRate > 0.0 ) || !( outputRate > 0.0 ) )
        return false;
    const Float64 step = inputRate / outputRate;
    if ( !( step > 0.0 ) || step > (Float64) MAX_STEP )
        return false;
    m_step = step;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CSincResampler::Reset( void )
{
    // no trace macro: this runs where the audio is mixed

    m_position = 0.0;
    m_historyWrite = 0;
    if ( !m_history.empty() )
        memset( &m_history[ 0 ], 0, m_history.size() * sizeof( Float32 ) );
}

/*-------------------------------------------------------------------------*/

UInt32
CSincResampler::CurrentHalfWidth( void ) const
{
    // no trace macro: this runs where the audio is mixed

    if ( m_step <= 1.0 )
        return HALF_WIDTH;
    return (UInt32) ceil( (Float64) HALF_WIDTH * m_step );
}

/*-------------------------------------------------------------------------*/

void
CSincResampler::ComputeCoefficients( Float64 position, UInt32 halfWidth )
{
    // no trace macro: this runs where the audio is mixed

    const Float64 whole = floor( position );
    const Float64 fraction = position - whole;
    Float32* coefficients = &m_coefficients[ 0 ];

    if ( m_step <= 1.0 )
    {
        // the table rows for the phases around the position, with a linear blend between them
        const Float64 phasePosition = fraction * (Float64) PHASE_COUNT;
        const UInt32 phase = (UInt32) phasePosition;
        const Float32 blend = (Float32) ( phasePosition - (Float64) phase );
        const Float32* a = &m_phaseTable[ (size_t) phase * BaseTaps ];
        const Float32* b = a + BaseTaps;
        for ( UInt32 k=0; k<BaseTaps; ++k )
            coefficients[ k ] = a[ k ] + ( b[ k ] - a[ k ] ) * blend;
        return;
    }

    // Stretched kernel: the table lookup runs at 1 / m_step of the table spacing. Tap k is at distance
    // fraction + halfWidth - 1 - k from the position, in input frames.
    const UInt32 taps = 2 * halfWidth;
    const Float64 tableScale = (Float64) PHASE_COUNT / m_step;
    const Float32* kernel = &m_kernel[ 0 ];
    Float32 sum = 0.0f;
    for ( UInt32 k=0; k<taps; ++k )
    {
        const Float64 distance = fabs( fraction + (Float64) ( halfWidth - 1 ) - (Float64) k );
        const Float64 index = distance * tableScale;
        const UInt32 i = (UInt32) index;
        const Float32 blend = (Float32) ( index - (Float64) i );
        const Float32 value = kernel[ i ] + ( kernel[ i+1 ] - kernel[ i ] ) * blend;
        coefficients[ k ] = value;
        sum += value;
    }
    const Float32 norm = 1.0f / sum;
    for ( UInt32 k=0; k<taps; ++k )
        coefficients[ k ] *= norm;
}

/*-------------------------------------------------------------------------*/

void
CSincResampler::PushHistory( const Float32* input, UInt32 frames )
{
    // no trace macro: this runs where the audio is mixed

    const UInt32 channels = m_channelCount;
    if ( frames > HistoryFrames )
    {
        // only the last frames can matter, but the count of frames written stays right
        const UInt32 skip = frames - HistoryFrames;
        input += (size_t) skip * channels;
        m_historyWrite += skip;
        frames = HistoryFrames;
    }
    const UInt32 start = m_historyWrite & ( HistoryFrames - 1 );
    const UInt32 firstPart = ( HistoryFrames - start < frames ) ? HistoryFrames - start : frames;
    memcpy( &m_history[ (size_t) start * channels ], input, (size_t) firstPart * channels * sizeof( Float32 ) );
    if ( firstPart < frames )
        memcpy( &m_history[ 0 ], input + (size_t) firstPart * channels, (size_t) ( frames - firstPart ) * channels * sizeof( Float32 ) );
    m_historyWrite += frames;
}

/*-------------------------------------------------------------------------*/

void
CSincResampler::Process( const Float32* input          ,
                         UInt32 inputFrames            ,
                         UInt32& inputFramesConsumed   ,
                         Float32* output               ,
                         UInt32 outputFrameCapacity    ,
                         UInt32& outputFramesProduced  )
{
    // no trace macro: this runs where the audio is mixed

    inputFramesConsumed = 0;
    outputFramesProduced = 0;
    if ( 0 == m_channelCount || GUCEF_NULL == input || GUCEF_NULL == output || 0 == inputFrames )
        return;

    const UInt32 channels = m_channelCount;
    const Float64 step = m_step;
    const UInt32 halfWidth = CurrentHalfWidth();
    const UInt32 taps = 2 * halfWidth;
    const Float32* coefficients = &m_coefficients[ 0 ];
    const Int64 lastFrame = (Int64) inputFrames - 1;
    Float64 position = m_position;
    UInt32 produced = 0;
    bool starved = false;

    // An output at 'position' needs the frames from floor( position ) - halfWidth + 1 up to floor( position ) + halfWidth.
    // Frames before the offered chunk are in the history. The outputs whose last frame has not arrived wait for more input.
    while ( produced < outputFrameCapacity )
    {
        const Int64 whole = (Int64) floor( position );
        if ( whole + (Int64) halfWidth > lastFrame )
        {
            starved = true;
            break;
        }

        ComputeCoefficients( position, halfWidth );

        const Int64 first = whole - (Int64) halfWidth + 1;
        const Float32* frames;
        if ( first >= 0 )
        {
            frames = input + (size_t) first * channels;
        }
        else
        {
            // The taps reach back into the consumed frames: gather the history and the start of the chunk in one run.
            // After a change to a smaller ratio the whole window can lie in the history.
            const UInt32 fromHistory = ( -first < (Int64) taps ) ? (UInt32) ( -first ) : taps;
            for ( UInt32 h=0; h<fromHistory; ++h )
            {
                const UInt32 slot = ( m_historyWrite + (UInt32) first + h ) & ( HistoryFrames - 1 );
                memcpy( &m_window[ (size_t) h * channels ], &m_history[ (size_t) slot * channels ], (size_t) channels * sizeof( Float32 ) );
            }
            if ( fromHistory < taps )
                memcpy( &m_window[ (size_t) fromHistory * channels ], input, (size_t) ( taps - fromHistory ) * channels * sizeof( Float32 ) );
            frames = &m_window[ 0 ];
        }

        Float32* out = output + (size_t) produced * channels;
        for ( UInt32 c=0; c<channels; ++c )
        {
            Float32 value = Dot( coefficients, frames + c, channels, taps );
            if ( fabsf( value ) < DenormalFlushThreshold )
                value = 0.0f;
            out[ c ] = value;
        }
        ++produced;
        position += step;
    }

    if ( starved )
    {
        // all of the chunk is used up, the position moves into the next chunk
        PushHistory( input, inputFrames );
        m_position = position - (Float64) inputFrames;
        inputFramesConsumed = inputFrames;
    }
    else
    {
        // The output space ran out: the frames before the next output position are no longer needed, they stay in the history
        Int64 done = (Int64) floor( position );
        if ( done < 0 )
            done = 0;
        if ( done > (Int64) inputFrames )
            done = inputFrames;
        if ( done > 0 )
            PushHistory( input, (UInt32) done );
        m_position = position - (Float64) done;
        inputFramesConsumed = (UInt32) done;
    }
    outputFramesProduced = produced;
}

/*-------------------------------------------------------------------------*/

UInt32
CSincResampler::GetLatencyInFrames( void ) const
{
    // no trace macro: this runs where the audio is mixed

    return CurrentHalfWidth();
}

/*-------------------------------------------------------------------------*/

IAudioResampler*
CSincResampler::Clone( void ) const
{GUCEF_TRACE;

    CSincResampler* clone = GUCEF_NEW CSincResampler();
    if ( m_channelCount > 0 )
        clone->Configure( m_channelCount, m_step, 1.0 );
    return clone;
}

/*-------------------------------------------------------------------------*/

const char*
CSincResampler::GetName( void ) const
{GUCEF_TRACE;

    return "sinc";
}

/*-------------------------------------------------------------------------*/

Float64
CSincResampler::GetStep( void ) const
{
    // no trace macro: this runs where the audio is mixed

    return m_step;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
