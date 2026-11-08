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

#ifndef GUCEF_AUDIO_CAUDIOLEVELMETER_H
#include "gucefAUDIO_CAudioLevelMeter.h"
#define GUCEF_AUDIO_CAUDIOLEVELMETER_H
#endif /* GUCEF_AUDIO_CAUDIOLEVELMETER_H ? */

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

const Float32 CAudioLevelMeter::MinimumDecibels = -200.0f;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const Float64 RmsTimeInSeconds = 0.300;
const Float64 PeakHoldTimeConstantInSeconds = 1.5;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAudioLevelMeter::CAudioLevelMeter( void )
    : m_channels( 0 )
    , m_rmsCoefficient( 0.0 )
    , m_holdDecayPerFrame( 1.0 )
    , m_sampleRate( 0 )
{GUCEF_TRACE;

    Prepare( 0, 48000 );
}

/*-------------------------------------------------------------------------*/

CAudioLevelMeter::~CAudioLevelMeter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAudioLevelMeter::Prepare( UInt32 channels, UInt32 sampleRate )
{GUCEF_TRACE;

    m_sampleRate = sampleRate > 0 ? sampleRate : 48000;
    m_rmsCoefficient = 1.0 - exp( -1.0 / ( RmsTimeInSeconds * (Float64) m_sampleRate ) );
    m_holdDecayPerFrame = exp( -1.0 / ( PeakHoldTimeConstantInSeconds * (Float64) m_sampleRate ) );
    m_channels.Store( channels < (UInt32) MaxChannels ? channels : (UInt32) MaxChannels, MT::MEMORDER_RELAXED );
    Reset();
}

/*-------------------------------------------------------------------------*/

void
CAudioLevelMeter::Reset( void )
{
    for ( UInt32 c=0; c<(UInt32) MaxChannels; ++c )
    {
        m_meanSquare[ c ] = 0.0;
        m_hold[ c ] = 0.0f;
        m_peak[ c ].Store( 0.0f );
        m_rms[ c ].Store( 0.0f );
        m_peakHold[ c ].Store( 0.0f );
    }
}

/*-------------------------------------------------------------------------*/

void
CAudioLevelMeter::Update( const Float32* interleaved, UInt32 frames )
{
    const UInt32 channels = m_channels.Load( MT::MEMORDER_RELAXED );
    if ( 0 == frames || 0 == channels || GUCEF_NULL == interleaved )
        return;

    const Float64 a = m_rmsCoefficient;
    const Float32 holdDecay = (Float32) pow( m_holdDecayPerFrame, (Float64) frames );
    for ( UInt32 c=0; c<channels; ++c )
    {
        Float32 peak = 0.0f;
        Float64 meanSquare = m_meanSquare[ c ];
        const Float32* sample = interleaved + c;
        for ( UInt32 f=0; f<frames; ++f, sample += channels )
        {
            Float32 x = *sample;
            Float32 level = x < 0.0f ? -x : x;
            if ( !( level <= 1e30f ) )
            {
                x = 0.0f;               // not a number or infinite: broken audio must not stick in the running mean
                level = 0.0f;
            }
            if ( level > peak )
                peak = level;
            meanSquare += a * ( (Float64) x * (Float64) x - meanSquare );
        }
        if ( meanSquare < 1e-30 )
            meanSquare = 0.0;          // keeps the state out of the denormal range in silence
        m_meanSquare[ c ] = meanSquare;

        Float32 hold = m_hold[ c ] * holdDecay;
        if ( peak > hold )
            hold = peak;
        m_hold[ c ] = hold;

        m_peak[ c ].Store( peak );
        m_rms[ c ].Store( (Float32) sqrt( meanSquare ) );
        m_peakHold[ c ].Store( hold );
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioLevelMeter::GetChannelCount( void ) const
{

    return m_channels.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioLevelMeter::GetPeak( UInt32 channel ) const
{

    return channel < (UInt32) MaxChannels ? m_peak[ channel ].Load() : 0.0f;
}

/*-------------------------------------------------------------------------*/

Float32
CAudioLevelMeter::GetRms( UInt32 channel ) const
{

    return channel < (UInt32) MaxChannels ? m_rms[ channel ].Load() : 0.0f;
}

/*-------------------------------------------------------------------------*/

Float32
CAudioLevelMeter::GetPeakHold( UInt32 channel ) const
{GUCEF_TRACE;

    return channel < (UInt32) MaxChannels ? m_peakHold[ channel ].Load() : 0.0f;
}

/*-------------------------------------------------------------------------*/

Float32
CAudioLevelMeter::ToDecibels( Float32 linear )
{GUCEF_TRACE;

    if ( linear <= 0.0f )
        return MinimumDecibels;
    const Float32 decibels = (Float32) ( 20.0 * log10( (Float64) linear ) );
    return decibels < MinimumDecibels ? MinimumDecibels : decibels;
}

/*-------------------------------------------------------------------------*/

Float32
CAudioLevelMeter::GetPeakInDecibels( UInt32 channel ) const
{GUCEF_TRACE;

    return ToDecibels( GetPeak( channel ) );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioLevelMeter::GetRmsInDecibels( UInt32 channel ) const
{GUCEF_TRACE;

    return ToDecibels( GetRms( channel ) );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioLevelMeter::GetPeakHoldInDecibels( UInt32 channel ) const
{GUCEF_TRACE;

    return ToDecibels( GetPeakHold( channel ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
