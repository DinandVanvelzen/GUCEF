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

#include <string.h>
#include <math.h>

#include "gucefAUDIO_CPacketLossConcealer.h"

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

const UInt32 SaturatedCount = 0x7FFFFFFF;

/** Normalised cross-correlation of two segments of 'count' samples, 0 when one of them is silent */
inline Float32
NormalisedCorrelation( const Float32* a, const Float32* b, UInt32 count )
{
    Float32 dot = 0.0f, energyA = 0.0f, energyB = 0.0f;
    for ( UInt32 i=0; i<count; ++i )
    {
        dot += a[ i ] * b[ i ];
        energyA += a[ i ] * a[ i ];
        energyB += b[ i ] * b[ i ];
    }
    const Float32 denominator = energyA * energyB;
    return denominator > 1.0e-18f ? dot / sqrtf( denominator ) : 0.0f;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPacketLossConcealer::CPacketLossConcealer( void )
    : m_sampleRate( 0 )
    , m_historySize( 0 )
    , m_valid( 0 )
    , m_decimation( 1 )
    , m_window( 0 )
    , m_pitchMin( 0 )
    , m_pitchMax( 0 )
    , m_tenMs( 0 )
    , m_history()
    , m_decimated()
    , m_correlation()
    , m_olaScratch()
    , m_inLoss( false )
    , m_lost( 0 )
    , m_pitch( 0 )
    , m_cycleLength( 1 )
    , m_cyclePos( 0 )
    , m_seamOffset( 0.0f )
    , m_seamLength( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPacketLossConcealer::~CPacketLossConcealer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CPacketLossConcealer::Prepare( UInt32 sampleRate )
{GUCEF_TRACE;

    m_sampleRate = 0;
    if ( sampleRate < 8000 || sampleRate > 192000 )
        return false;

    m_decimation = ( sampleRate + 4000 ) / 8000;
    m_window = sampleRate / 50;
    m_pitchMin = sampleRate / 200;
    m_pitchMax = ( sampleRate * 15 ) / 1000;
    m_tenMs = sampleRate / 100;
    m_historySize = (UInt32) ( ( (UInt64) sampleRate * 4875ULL + 99999ULL ) / 100000ULL );

    m_history.assign( m_historySize, 0.0f );
    m_decimated.assign( ( m_window + m_pitchMax ) / m_decimation + 2, 0.0f );
    m_correlation.assign( m_pitchMax / m_decimation + 2, 0.0f );
    m_olaScratch.assign( m_tenMs + 1, 0.0f );
    m_sampleRate = sampleRate;
    Reset();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPacketLossConcealer::IsPrepared( void ) const
{
    return 0 != m_sampleRate;
}

/*-------------------------------------------------------------------------*/

UInt32
CPacketLossConcealer::GetSampleRate( void ) const
{
    return m_sampleRate;
}

/*-------------------------------------------------------------------------*/

void
CPacketLossConcealer::Reset( void )
{
    if ( 0 != m_historySize )
        memset( &m_history[ 0 ], 0, (size_t) m_historySize * sizeof( Float32 ) );
    m_valid = 0;
    m_inLoss = false;
    m_lost = 0;
    m_pitch = 0;
    m_cycleLength = 1;
    m_cyclePos = 0;
    m_seamOffset = 0.0f;
    m_seamLength = 0;
}

/*-------------------------------------------------------------------------*/

bool
CPacketLossConcealer::IsConcealing( void ) const
{
    return m_inLoss;
}

/*-------------------------------------------------------------------------*/

UInt32
CPacketLossConcealer::GetConcealedSampleCount( void ) const
{
    return m_lost;
}

/*-------------------------------------------------------------------------*/

UInt32
CPacketLossConcealer::GetPitchPeriod( void ) const
{
    return m_pitch;
}

/*-------------------------------------------------------------------------*/

void
CPacketLossConcealer::AppendToHistory( const Float32* pcm, UInt32 frames )
{
    if ( frames >= m_historySize )
    {
        memcpy( &m_history[ 0 ], pcm + ( frames - m_historySize ), (size_t) m_historySize * sizeof( Float32 ) );
        m_valid = m_historySize;
        return;
    }
    Float32* history = &m_history[ 0 ];
    memmove( history, history + frames, (size_t) ( m_historySize - frames ) * sizeof( Float32 ) );
    memcpy( history + ( m_historySize - frames ), pcm, (size_t) frames * sizeof( Float32 ) );
    m_valid = m_valid + frames < m_historySize ? m_valid + frames : m_historySize;
}

/*-------------------------------------------------------------------------*/

UInt32
CPacketLossConcealer::EstimatePitch( void )
{
    const UInt32 window = m_window;
    const UInt32 needed = window + m_pitchMax;
    if ( m_valid < needed )
    {
        // too little history to search: a period of 10 ms, or what there is
        const UInt32 fallback = m_tenMs < m_valid ? m_tenMs : m_valid;
        return fallback > 0 ? fallback : 1;
    }

    const Float32* history = &m_history[ 0 ];
    const UInt32 decimation = m_decimation;

    // the signal of the last 'needed' samples at the decimated rate (the mean of 'decimation' samples), aligned to the end
    const UInt32 decimatedCount = needed / decimation;
    Float32* decimated = &m_decimated[ 0 ];
    const Float32* start = history + ( m_historySize - decimatedCount * decimation );
    const Float32 scale = 1.0f / (Float32) decimation;
    for ( UInt32 i=0; i<decimatedCount; ++i )
    {
        Float32 sum = 0.0f;
        for ( UInt32 d=0; d<decimation; ++d )
            sum += start[ i * decimation + d ];
        decimated[ i ] = sum * scale;
    }

    // coarse search over the periods
    const UInt32 decimatedWindow = window / decimation;
    UInt32 coarseMin = ( m_pitchMin + decimation - 1 ) / decimation;
    UInt32 coarseMax = m_pitchMax / decimation;
    if ( coarseMin < 1 ) coarseMin = 1;
    if ( coarseMax + decimatedWindow > decimatedCount ) coarseMax = decimatedCount - decimatedWindow;
    if ( coarseMax < coarseMin ) coarseMax = coarseMin;

    const Float32* segment = decimated + ( decimatedCount - decimatedWindow );
    Float32* correlation = &m_correlation[ 0 ];
    Float32 best = -2.0f;
    for ( UInt32 p=coarseMin; p<=coarseMax; ++p )
    {
        correlation[ p ] = NormalisedCorrelation( segment, segment - p, decimatedWindow );
        if ( correlation[ p ] > best )
            best = correlation[ p ];
    }

    // the shortest local peak that is nearly as good as the best: the multiples of the period correlate as well
    UInt32 coarse = coarseMin;
    for ( UInt32 p=coarseMin; p<=coarseMax; ++p )
    {
        const bool isPeak = ( p == coarseMin || correlation[ p ] >= correlation[ p-1 ] ) && ( p == coarseMax || correlation[ p ] >= correlation[ p+1 ] );
        if ( isPeak && correlation[ p ] >= best - 0.02f )
        {
            coarse = p;
            break;
        }
    }

    // refine at the full rate within one decimation step of the coarse period
    const Float32* tail = history + ( m_historySize - window );
    UInt32 lowest = coarse * decimation > decimation ? coarse * decimation - decimation : 1;
    UInt32 highest = coarse * decimation + decimation;
    if ( lowest < m_pitchMin ) lowest = m_pitchMin;
    if ( highest > m_pitchMax ) highest = m_pitchMax;
    if ( highest < lowest ) highest = lowest;
    UInt32 pitch = lowest;
    Float32 bestFine = -2.0f;
    for ( UInt32 q=lowest; q<=highest; ++q )
    {
        const Float32 c = NormalisedCorrelation( tail, tail - q, window );
        if ( c > bestFine )
        {
            bestFine = c;
            pitch = q;
        }
    }
    return pitch;
}

/*-------------------------------------------------------------------------*/

void
CPacketLossConcealer::StartLoss( void )
{
    m_inLoss = true;
    m_lost = 0;
    m_pitch = EstimatePitch();
    if ( m_pitch > m_valid ) m_pitch = m_valid;
    if ( m_pitch < 1 ) m_pitch = 1;

    // matches the start of the synthetic signal to the last decoded sample: what is left of the step from the last
    // decoded sample to the one before it a period earlier, fading out over a quarter of a period
    m_seamOffset = 0.0f;
    m_seamLength = 0;
    if ( m_valid > m_pitch )
    {
        const Float32* history = &m_history[ 0 ];
        m_seamOffset = history[ m_historySize - 1 ] - history[ m_historySize - 1 - m_pitch ];
        m_seamLength = m_pitch / 4 > 0 ? m_pitch / 4 : 1;
    }
    m_cycleLength = m_pitch;
    m_cyclePos = 0;
}

/*-------------------------------------------------------------------------*/

void
CPacketLossConcealer::Synthesize( Float32* out, UInt32 count )
{
    const Float32* history = &m_history[ 0 ];
    const UInt32 tenMs = m_tenMs;
    const UInt32 silentFrom = 6 * tenMs;
    const Float32 gainStep = 0.2f / (Float32) tenMs;
    const UInt32 firstIndex = m_lost;

    UInt32 done = 0;
    while ( done < count )
    {
        const UInt32 index = m_lost;
        if ( index >= silentFrom )
        {
            memset( out + done, 0, (size_t) ( count - done ) * sizeof( Float32 ) );
            if ( m_lost < SaturatedCount - ( count - done ) )
                m_lost += count - done;
            else
                m_lost = SaturatedCount;
            done = count;
            break;
        }

        // the number of periods repeated: 1, then 2 after 10 ms and 3 after 20 ms, as many as the history holds
        UInt32 periods = index < tenMs ? 1 : ( index < 2 * tenMs ? 2 : 3 );
        while ( periods > 1 && periods * m_pitch > m_valid )
            --periods;
        const UInt32 cycle = periods * m_pitch;
        if ( cycle != m_cycleLength )
        {
            m_cycleLength = cycle;
            m_cyclePos = index % cycle;
        }

        const UInt32 boundary = index < tenMs ? tenMs : ( index < 2 * tenMs ? 2 * tenMs : silentFrom );
        UInt32 run = boundary - index;
        if ( run > count - done )
            run = count - done;

        const Float32* source = history + ( m_historySize - cycle );
        UInt32 position = m_cyclePos;
        for ( UInt32 i=0; i<run; ++i )
        {
            // the level from the sample number, so how the samples are asked for (one call or many) makes no difference
            const UInt32 at = index + i;
            const Float32 gain = at < tenMs ? 1.0f : 1.0f - gainStep * (Float32) ( at - tenMs );
            out[ done + i ] = source[ position ] * gain;
            if ( ++position >= cycle )
                position = 0;
        }
        m_cyclePos = position;
        m_lost += run;
        done += run;
    }

    // the match to the last decoded sample
    if ( firstIndex < m_seamLength )
    {
        const UInt32 seamCount = m_seamLength - firstIndex < count ? m_seamLength - firstIndex : count;
        for ( UInt32 i=0; i<seamCount; ++i )
            out[ i ] += m_seamOffset * ( 1.0f - (Float32) ( firstIndex + i + 1 ) / (Float32) m_seamLength );
    }
}

/*-------------------------------------------------------------------------*/

void
CPacketLossConcealer::Conceal( Float32* pcm, UInt32 frames )
{
    if ( 0 == m_sampleRate )
    {
        memset( pcm, 0, (size_t) frames * sizeof( Float32 ) );
        return;
    }
    if ( !m_inLoss )
        StartLoss();
    Synthesize( pcm, frames );
}

/*-------------------------------------------------------------------------*/

void
CPacketLossConcealer::OnGoodFrame( Float32* pcm, UInt32 frames )
{
    if ( 0 == m_sampleRate || 0 == frames )
        return;

    if ( m_inLoss )
    {
        // the cross-fade from the signal as it would have gone on into the decoded one
        UInt32 length = m_lost <= m_tenMs ? m_pitch / 4 : ( m_lost <= 2 * m_tenMs ? m_pitch / 2 : m_tenMs );
        if ( length < 1 )               length = 1;
        if ( length > frames )          length = frames;
        if ( length > m_tenMs )         length = m_tenMs;
        if ( length > 0 )
        {
            Float32* synthetic = &m_olaScratch[ 0 ];
            Synthesize( synthetic, length );
            for ( UInt32 i=0; i<length; ++i )
            {
                const Float32 weight = (Float32) ( i + 1 ) / (Float32) ( length + 1 );
                pcm[ i ] = pcm[ i ] * weight + synthetic[ i ] * ( 1.0f - weight );
            }
        }
        m_inLoss = false;
        m_lost = 0;
    }
    AppendToHistory( pcm, frames );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
